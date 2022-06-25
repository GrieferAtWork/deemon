/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_LIST_H
#define GUARD_DEEMON_CXX_LIST_H 1

#include "api.h"

#include "../format.h" /* DEE_PCK* */
#include "../list.h"
#include "../seq.h"
#include "object.h"
#include "sequence.h"

DEE_CXX_BEGIN

template<class T = Object> class List;

template<class T>
class List: public Sequence<T> {
public:
	static DeeTypeObject *classtype() DEE_CXX_NOTHROW {
		return &DeeList_Type;
	}
	static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeList_Check(ob);
	}
	static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeList_CheckExact(ob);
	}

public:
	List()
	    : Sequence<T>(nonnull(DeeList_New())) {}
	List(std::initializer_list<T> const &items)
	    : Sequence<T>(inherit(DeeList_NewVector(items.size(), (DeeObject **)items.begin()))) {}
	List(std::initializer_list<DeeObject *> const &items)
	    : Sequence<T>(inherit(DeeList_NewVector(items.size(), items.begin()))) {}
	List(size_t objc, DeeObject **__restrict objv)
	    : Sequence<T>(inherit(DeeList_NewVector(objc, objv))) {}
	List(size_t objc, DeeObject *const *__restrict objv)
	    : Sequence<T>(inherit(DeeList_NewVector(objc, objv))) {}
	List(size_t objc, T **__restrict objv)
	    : Sequence<T>(inherit(DeeList_NewVector(objc, (DeeObject **)objv))) {}
	List(size_t objc, T *const *__restrict objv)
	    : Sequence<T>(inherit(DeeList_NewVector(objc, (DeeObject **)objv))) {}
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(List, Sequence<T>)
#ifndef __OPTIMIZE_SIZE__
	void append(T const &ob) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_Append(this->ptr(), ob));
		else {
			Sequence<T>::append(ob);
		}
	}
	void append(DeeObject *__restrict ob) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_Append(this->ptr(), ob));
		else {
			Sequence<T>::append(ob);
		}
	}
	void extend(DeeObject *__restrict items) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_AppendSequence(this->ptr(), items));
		else {
			Sequence<T>::extend(items);
		}
	}
	void extend(std::initializer_list<T> const &items) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_AppendVector(this->ptr(), items.size(), (DeeObject **)items.begin()));
		else {
			Sequence<T>::extend(items);
		}
	}
	void extend(std::initializer_list<DeeObject *> const &items) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_AppendVector(this->ptr(), items.size(), items.begin()));
		else {
			Sequence<T>::extend(items);
		}
	}
	void extend(size_t objc, DeeObject **__restrict objv) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_AppendVector(this->ptr(), objc, objv));
		else {
			Sequence<T>::extend(objc, objv);
		}
	}
	void extend(size_t objc, DeeObject *const *__restrict objv) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_AppendVector(this->ptr(), objc, objv));
		else {
			Sequence<T>::extend(objc, objv);
		}
	}
	void extend(size_t objc, T **__restrict objv) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_AppendVector(this->ptr(), objc, (DeeObject **)objv));
		else {
			Sequence<T>::extend(objc, objv);
		}
	}
	void extend(size_t objc, T *const *__restrict objv) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_AppendVector(this->ptr(), objc, (DeeObject **)objv));
		else {
			Sequence<T>::extend(objc, objv);
		}
	}
	void appenditer(DeeObject *__restrict iter) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_AppendIterator(this->ptr(), iter));
		else {
			Sequence<T>::appenditer(iter);
		}
	}
	using Sequence<T>::erase;
	size_t erase(size_t index) const {
		if likely(DeeList_CheckExact(this->ptr()))
			return DeeList_Erase(this->ptr(), index, 1);
		return Object(inherit(DeeObject_CallAttrStringf(this->ptr(), "erase", DEE_PCKuSIZ, index))).assize();
	}
	size_t erase(size_t index, size_t count) const {
		if likely(DeeList_CheckExact(this->ptr()))
			return DeeList_Erase(this->ptr(), index, count);
		return Object(inherit(DeeObject_CallAttrStringf(this->ptr(), "erase", DEE_PCKuSIZ DEE_PCKuSIZ, index, count))).assize();
	}
	using Sequence<T>::pop;
	WUNUSED T pop() const {
		return inherit(likely(DeeList_CheckExact(this->ptr()))
		               ? DeeList_Pop(this->ptr(), -1)
		               : DeeObject_CallAttrString(this->ptr(), "pop", 0, NULL));
	}
	WUNUSED T pop(Dee_ssize_t index) const {
		return inherit(likely(DeeList_CheckExact(this->ptr()))
		               ? DeeList_Pop(this->ptr(), index)
		               : DeeObject_CallAttrStringf(this->ptr(), "pop", DEE_PCKdSIZ, index));
	}
	void clear() const {
		if likely(DeeList_CheckExact(this->ptr()))
			DeeList_Clear(this->ptr());
		else {
			Sequence<T>::clear();
		}
	}
	void reverse() const {
		if likely(DeeList_CheckExact(this->ptr()))
			DeeList_Reverse(this->ptr());
		else {
			Sequence<T>::reverse();
		}
	}
	void sort() const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_Sort(this->ptr(), NULL));
		else {
			Sequence<T>::sort();
		}
	}
	void sort(DeeObject *__restrict key) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_Sort(this->ptr(), key));
		else {
			Sequence<T>::sort(key);
		}
	}
	using Sequence<T>::insert;
	using Sequence<T>::insertall;
	using Sequence<T>::insertiter;
	void insert(size_t index, DeeObject *__restrict item) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_Insert(this->ptr(), index, item));
		else {
			Sequence<T>::insert(index, item);
		}
	}
	void insertall(size_t index, DeeObject *__restrict items) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_InsertSequence(this->ptr(), index, items));
		else {
			Sequence<T>::insertall(index, items);
		}
	}
	void insertall(size_t index, size_t objc, DeeObject *const *__restrict objv) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_InsertVector(this->ptr(), index, objc, objv));
		else {
			Sequence<T>::insertall(index, objc, objv);
		}
	}
	void insertall(size_t index, size_t objc, DeeObject **__restrict objv) const {
		insertall(index, objc, (DeeObject *const *)objv);
	}
	void insertall(size_t index, std::initializer_list<DeeObject *> const &items) const {
		insertall(index, items.size(), items.begin());
	}
	void insertiter(size_t index, DeeObject *__restrict iter) const {
		if likely(DeeList_CheckExact(this->ptr()))
			throw_if_nonzero(DeeList_InsertIterator(this->ptr(), index, iter));
		else {
			Sequence<T>::insertiter(index, iter);
		}
	}
#endif /* !__OPTIMIZE_SIZE__ */
};


DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_LIST_H */
