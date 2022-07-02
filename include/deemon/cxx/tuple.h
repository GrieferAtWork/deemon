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
#ifndef GUARD_DEEMON_CXX_TUPLE_H
#define GUARD_DEEMON_CXX_TUPLE_H 1

#include "api.h"

#include "../seq.h"
#include "../tuple.h"
#include "object.h"
#include "sequence.h"

DEE_CXX_BEGIN

template<class T>
class Tuple: public Sequence<T> {
public:
	static DeeTypeObject *classtype() DEE_CXX_NOTHROW {
		return &DeeTuple_Type;
	}
	static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeTuple_Check(ob);
	}
	static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeTuple_CheckExact(ob);
	}

public:
	Tuple() DEE_CXX_NOTHROW
	    : Sequence<T>(nonnull(Dee_EmptyTuple)) {}
	Tuple(std::initializer_list<T> const &items)
	    : Sequence<T>(inherit(DeeTuple_NewVector(items.size(), (DeeObject **)items.begin()))) {}
	Tuple(std::initializer_list<DeeObject *> const &items)
	    : Sequence<T>(inherit(DeeTuple_NewVector(items.size(), items.begin()))) {}
	Tuple(size_t objc, DeeObject **__restrict objv)
	    : Sequence<T>(inherit(DeeTuple_NewVector(objc, objv))) {}
	Tuple(size_t objc, DeeObject *const *__restrict objv)
	    : Sequence<T>(inherit(DeeTuple_NewVector(objc, objv))) {}
	Tuple(size_t objc, T **__restrict objv)
	    : Sequence<T>(inherit(DeeTuple_NewVector(objc, (DeeObject **)objv))) {}
	Tuple(size_t objc, T *const *__restrict objv)
	    : Sequence<T>(inherit(DeeTuple_NewVector(objc, (DeeObject **)objv))) {}
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(Tuple, Sequence<T>)
	operator obj_tuple() const DEE_CXX_NOTHROW {
		return obj_tuple(this->ptr());
	}
	Tuple types() const {
		return inherit(DeeTuple_Types(this->ptr()));
	}
	void(append)(T const &ob) {
		if likely(DeeTuple_Check(this->ptr())) {
			this->m_ptr = throw_if_null(DeeTuple_Append(this->ptr(), ob));
		} else {
			Sequence<T>::append(ob);
		}
	}
	void(append)(DeeObject *__restrict ob) {
		if likely(DeeTuple_Check(this->ptr())) {
			this->m_ptr = throw_if_null(DeeTuple_Append(this->ptr(), ob));
		} else {
			Sequence<T>::append(ob);
		}
	}
	void(appenditer)(DeeObject *__restrict iter) {
		if likely(DeeTuple_Check(this->ptr())) {
			this->m_ptr = throw_if_null(DeeTuple_AppendIterator(this->ptr(), iter));
		} else {
			Sequence<T>::appenditer(iter);
		}
	}
};


DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_TUPLE_H */
