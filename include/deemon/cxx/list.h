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
#ifndef GUARD_DEEMON_CXX_LIST_H
#define GUARD_DEEMON_CXX_LIST_H 1

#include "api.h"
/**/

#include "object.h"
/**/

#include "../format.h"
#include "../list.h"

DEE_CXX_BEGIN

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
	NONNULL_CXX((1)) void (extend)(DeeObject *items) {
		DeeObject *args[1];
		args[0] = items;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "extend", _Dee_HashSelectC(0x960b75e7, 0xba076858e3adb055), 1, args)));
	}
	NONNULL_CXX((1)) void (resize)(DeeObject *newsize) {
		DeeObject *args[1];
		args[0] = newsize;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (resize)(DeeObject *newsize, DeeObject *filler) {
		DeeObject *args[2];
		args[0] = newsize;
		args[1] = filler;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34), 2, args)));
	}
	void (resize)(Dee_ssize_t newsize) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34),  DEE_PCKdSIZ, newsize)));
	}
	NONNULL_CXX((2)) void (resize)(Dee_ssize_t newsize, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34),  DEE_PCKdSIZ "o", newsize, filler)));
	}
	void (resize)(size_t newsize) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34),  DEE_PCKuSIZ, newsize)));
	}
	NONNULL_CXX((2)) void (resize)(size_t newsize, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34),  DEE_PCKuSIZ "o", newsize, filler)));
	}
	NONNULL_CXX((1, 2)) void (insert)(DeeObject *index, DeeObject *item) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insert", _Dee_HashSelectC(0x71d74a66, 0x5e168c86241590d7), 2, args)));
	}
	NONNULL_CXX((2)) void (insert)(Dee_ssize_t index, DeeObject *item) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert", _Dee_HashSelectC(0x71d74a66, 0x5e168c86241590d7),  DEE_PCKdSIZ "o", index, item)));
	}
	NONNULL_CXX((2)) void (insert)(size_t index, DeeObject *item) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert", _Dee_HashSelectC(0x71d74a66, 0x5e168c86241590d7),  DEE_PCKuSIZ "o", index, item)));
	}
	NONNULL_CXX((1, 2)) void (insertall)(DeeObject *index, DeeObject *seq) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = seq;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insertall", _Dee_HashSelectC(0xbf9bc3a9, 0x4f85971d093a27f2), 2, args)));
	}
	NONNULL_CXX((2)) void (insertall)(Dee_ssize_t index, DeeObject *seq) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insertall", _Dee_HashSelectC(0xbf9bc3a9, 0x4f85971d093a27f2),  DEE_PCKdSIZ "o", index, seq)));
	}
	NONNULL_CXX((2)) void (insertall)(size_t index, DeeObject *seq) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insertall", _Dee_HashSelectC(0xbf9bc3a9, 0x4f85971d093a27f2),  DEE_PCKuSIZ "o", index, seq)));
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
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), "o" DEE_PCKdSIZ, index, count)));
	}
	NONNULL_CXX((1)) void (erase)(DeeObject *index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), "o" DEE_PCKuSIZ, index, count)));
	}
	void (erase)(Dee_ssize_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKdSIZ, index)));
	}
	NONNULL_CXX((2)) void (erase)(Dee_ssize_t index, DeeObject *count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKdSIZ "o", index, count)));
	}
	void (erase)(Dee_ssize_t index, Dee_ssize_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKdSIZ DEE_PCKdSIZ, index, count)));
	}
	void (erase)(Dee_ssize_t index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKdSIZ DEE_PCKuSIZ, index, count)));
	}
	void (erase)(size_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKuSIZ, index)));
	}
	NONNULL_CXX((2)) void (erase)(size_t index, DeeObject *count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKuSIZ "o", index, count)));
	}
	void (erase)(size_t index, Dee_ssize_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKuSIZ DEE_PCKdSIZ, index, count)));
	}
	void (erase)(size_t index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKuSIZ DEE_PCKuSIZ, index, count)));
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
		return inherit(DeeObject_CallAttrStringHashf(this, "pop", _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb),  DEE_PCKdSIZ, index));
	}
	WUNUSED Ref<T> (pop)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "pop", _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb),  DEE_PCKuSIZ, index));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (xchitem)(DeeObject *index, DeeObject *value) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "xchitem", _Dee_HashSelectC(0xc89decce, 0x16e81f00d8d95d57), 2, args));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (xchitem)(Dee_ssize_t index, DeeObject *value) {
		return inherit(DeeObject_CallAttrStringHashf(this, "xchitem", _Dee_HashSelectC(0xc89decce, 0x16e81f00d8d95d57),  DEE_PCKdSIZ "o", index, value));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (xchitem)(size_t index, DeeObject *value) {
		return inherit(DeeObject_CallAttrStringHashf(this, "xchitem", _Dee_HashSelectC(0xc89decce, 0x16e81f00d8d95d57),  DEE_PCKuSIZ "o", index, value));
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
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
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
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (index)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (index)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (index)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (index)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (index)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (index)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (index)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (index)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (index)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (index)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (index)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (index)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (index)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (index)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (index)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rindex)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (rindex)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rindex)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rindex)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rindex)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rindex)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
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
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
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
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
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
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "ooo" DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "ooo" DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "ooo" DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "ooo" DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKdSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKdSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKuSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKuSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ "oo", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ "ooo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ "o" DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ "o" DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ "o" DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ "o" DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKdSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKuSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ "oo", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ "ooo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ "o" DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ "o" DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ "o" DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ "o" DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKdSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKuSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, max, key));
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
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), "o" DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((1, 3)) void (fill)(DeeObject *start, Dee_ssize_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), "o" DEE_PCKdSIZ "o", start, end, filler)));
	}
	NONNULL_CXX((1)) void (fill)(DeeObject *start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), "o" DEE_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((1, 3)) void (fill)(DeeObject *start, size_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), "o" DEE_PCKuSIZ "o", start, end, filler)));
	}
	void (fill)(Dee_ssize_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKdSIZ, start)));
	}
	NONNULL_CXX((2)) void (fill)(Dee_ssize_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKdSIZ "o", start, end)));
	}
	NONNULL_CXX((2, 3)) void (fill)(Dee_ssize_t start, DeeObject *end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKdSIZ "oo", start, end, filler)));
	}
	void (fill)(Dee_ssize_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (fill)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKdSIZ DEE_PCKdSIZ "o", start, end, filler)));
	}
	void (fill)(Dee_ssize_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (fill)(Dee_ssize_t start, size_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKdSIZ DEE_PCKuSIZ "o", start, end, filler)));
	}
	void (fill)(size_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKuSIZ, start)));
	}
	NONNULL_CXX((2)) void (fill)(size_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKuSIZ "o", start, end)));
	}
	NONNULL_CXX((2, 3)) void (fill)(size_t start, DeeObject *end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKuSIZ "oo", start, end, filler)));
	}
	void (fill)(size_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (fill)(size_t start, Dee_ssize_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKuSIZ DEE_PCKdSIZ "o", start, end, filler)));
	}
	void (fill)(size_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (fill)(size_t start, size_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKuSIZ DEE_PCKuSIZ "o", start, end, filler)));
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
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "ooo" DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "ooo" DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKdSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKdSIZ DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKdSIZ DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKuSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKuSIZ DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKuSIZ DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ, should, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ "o", should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, DeeObject *end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ "oo", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ "o" DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ "o" DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKdSIZ DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKdSIZ DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKuSIZ DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKuSIZ DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ, should, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ "o", should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, DeeObject *end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ "oo", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ "o" DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ "o" DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKdSIZ DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKdSIZ DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKuSIZ DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKuSIZ DEE_PCKuSIZ, should, start, end, max));
	}
	NONNULL_CXX((1)) void (pushfront)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "pushfront", _Dee_HashSelectC(0xc682cfdf, 0x5933eb9a387ff882), 1, args)));
	}
	NONNULL_CXX((1)) void (pushback)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "pushback", _Dee_HashSelectC(0xad1e1509, 0x4cfafd84a12923bd), 1, args)));
	}
	WUNUSED Ref<T> (popfront)() {
		return inherit(DeeObject_CallAttrStringHash(this, "popfront", _Dee_HashSelectC(0x46523911, 0x22a469cc52318bba), 0, NULL));
	}
	WUNUSED Ref<T> (popback)() {
		return inherit(DeeObject_CallAttrStringHash(this, "popback", _Dee_HashSelectC(0xd84577aa, 0xb77f74a49a9cc289), 0, NULL));
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
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), "o" DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((1)) void (reverse)(DeeObject *start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), "o" DEE_PCKuSIZ, start, end)));
	}
	void (reverse)(Dee_ssize_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKdSIZ, start)));
	}
	NONNULL_CXX((2)) void (reverse)(Dee_ssize_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKdSIZ "o", start, end)));
	}
	void (reverse)(Dee_ssize_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end)));
	}
	void (reverse)(Dee_ssize_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end)));
	}
	void (reverse)(size_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKuSIZ, start)));
	}
	NONNULL_CXX((2)) void (reverse)(size_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKuSIZ "o", start, end)));
	}
	void (reverse)(size_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end)));
	}
	void (reverse)(size_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end)));
	}
	void (sort)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), 0, NULL)));
	}
	NONNULL_CXX((1)) void (sort)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (sort)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), 2, args)));
	}
	NONNULL_CXX((1, 2, 3)) void (sort)(DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = key;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), 3, args)));
	}
	NONNULL_CXX((1)) void (sort)(DeeObject *start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), "o" DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((1, 3)) void (sort)(DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), "o" DEE_PCKdSIZ "o", start, end, key)));
	}
	NONNULL_CXX((1)) void (sort)(DeeObject *start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), "o" DEE_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((1, 3)) void (sort)(DeeObject *start, size_t end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), "o" DEE_PCKuSIZ "o", start, end, key)));
	}
	void (sort)(Dee_ssize_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKdSIZ, start)));
	}
	NONNULL_CXX((2)) void (sort)(Dee_ssize_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKdSIZ "o", start, end)));
	}
	NONNULL_CXX((2, 3)) void (sort)(Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKdSIZ "oo", start, end, key)));
	}
	void (sort)(Dee_ssize_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (sort)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKdSIZ DEE_PCKdSIZ "o", start, end, key)));
	}
	void (sort)(Dee_ssize_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (sort)(Dee_ssize_t start, size_t end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKdSIZ DEE_PCKuSIZ "o", start, end, key)));
	}
	void (sort)(size_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKuSIZ, start)));
	}
	NONNULL_CXX((2)) void (sort)(size_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKuSIZ "o", start, end)));
	}
	NONNULL_CXX((2, 3)) void (sort)(size_t start, DeeObject *end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKuSIZ "oo", start, end, key)));
	}
	void (sort)(size_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (sort)(size_t start, Dee_ssize_t end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKuSIZ DEE_PCKdSIZ "o", start, end, key)));
	}
	void (sort)(size_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (sort)(size_t start, size_t end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKuSIZ DEE_PCKuSIZ "o", start, end, key)));
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
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Sequence<T> > (sorted)(DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (sorted)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Sequence<T> > (sorted)(DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<T> > (sorted)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKdSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Sequence<T> > (sorted)(Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKdSIZ "oo", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<T> > (sorted)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKdSIZ DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<T> > (sorted)(Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKdSIZ DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<T> > (sorted)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKuSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Sequence<T> > (sorted)(size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKuSIZ "oo", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<T> > (sorted)(size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKuSIZ DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<T> > (sorted)(size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKuSIZ DEE_PCKuSIZ "o", start, end, key));
	}
	NONNULL_CXX((1)) void (reserve)(DeeObject *size) {
		DeeObject *args[1];
		args[0] = size;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9), 1, args)));
	}
	void (reserve)(Dee_ssize_t size) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9),  DEE_PCKdSIZ, size)));
	}
	void (reserve)(size_t size) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reserve", _Dee_HashSelectC(0x62fb4af5, 0x3a9a799f304cf8c9),  DEE_PCKuSIZ, size)));
	}
	void (shrink)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "shrink", _Dee_HashSelectC(0xd8afa32b, 0xf4785bc46214dc5), 0, NULL)));
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
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "oo" DEE_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (remove_if)(DeeObject *should, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "oo" DEE_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (remove_if)(DeeObject *should, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" DEE_PCKdSIZ, should, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (remove_if)(DeeObject *should, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" DEE_PCKdSIZ "o", should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (remove_if)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" DEE_PCKdSIZ DEE_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (remove_if)(DeeObject *should, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" DEE_PCKdSIZ DEE_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (remove_if)(DeeObject *should, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" DEE_PCKuSIZ, should, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (remove_if)(DeeObject *should, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" DEE_PCKuSIZ "o", should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (remove_if)(DeeObject *should, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" DEE_PCKuSIZ DEE_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (remove_if)(DeeObject *should, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove_if", _Dee_HashSelectC(0x58aa20e7, 0x10d750b49732d559), "o" DEE_PCKuSIZ DEE_PCKuSIZ, should, start, end));
	}
	NONNULL_CXX((1, 2)) void (insert_list)(DeeObject *index, DeeObject *items) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = items;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insert_list", _Dee_HashSelectC(0xc36250ae, 0xef11d73d4bfae0f3), 2, args)));
	}
	NONNULL_CXX((2)) void (insert_list)(Dee_ssize_t index, DeeObject *items) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert_list", _Dee_HashSelectC(0xc36250ae, 0xef11d73d4bfae0f3),  DEE_PCKdSIZ "o", index, items)));
	}
	NONNULL_CXX((2)) void (insert_list)(size_t index, DeeObject *items) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert_list", _Dee_HashSelectC(0xc36250ae, 0xef11d73d4bfae0f3),  DEE_PCKuSIZ "o", index, items)));
	}
	NONNULL_CXX((1, 2)) void (insert_iter)(DeeObject *index, DeeObject *iter) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = iter;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insert_iter", _Dee_HashSelectC(0xb8d6666d, 0x6d19ee143161783f), 2, args)));
	}
	NONNULL_CXX((2)) void (insert_iter)(Dee_ssize_t index, DeeObject *iter) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert_iter", _Dee_HashSelectC(0xb8d6666d, 0x6d19ee143161783f),  DEE_PCKdSIZ "o", index, iter)));
	}
	NONNULL_CXX((2)) void (insert_iter)(size_t index, DeeObject *iter) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert_iter", _Dee_HashSelectC(0xb8d6666d, 0x6d19ee143161783f),  DEE_PCKuSIZ "o", index, iter)));
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
