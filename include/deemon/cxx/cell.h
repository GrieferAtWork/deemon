/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_CELL_H
#define GUARD_DEEMON_CXX_CELL_H 1

#include "api.h"

#include "../cell.h"
#include "object.h"

DEE_CXX_BEGIN

class Cell: public Object {
public:
	static DeeTypeObject *classtype() DEE_CXX_NOTHROW {
		return &DeeCell_Type;
	}
	static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeCell_Check(ob);
	}
	static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeCell_CheckExact(ob);
	}

private:
	Cell(DeeObject *__restrict ob, int)
	    : Object(inherit(DeeCell_New(ob))) {}

public:
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(Cell, Object)
	Cell()
	    : Object(inherit(DeeCell_NewEmpty())) {}
	static Cell with(DeeObject *__restrict ob) {
		return Cell(ob, 0);
	}
	WUNUSED DREF DeeObject *getref() const DEE_CXX_NOTHROW {
		return DeeCell_Get(m_ptr);
	}
	WUNUSED ATTR_RETNONNULL DREF DeeObject *getref(DeeObject *__restrict def) const DEE_CXX_NOTHROW {
		DREF DeeObject *result = DeeCell_TryGet(m_ptr);
		if (!result) {
			Dee_Incref(def);
			result = def;
		}
		return result;
	}
	WUNUSED DREF DeeObject *xchref(DeeObject *newval) const DEE_CXX_NOTHROW {
		return DeeCell_Xch(m_ptr, newval);
	}
	WUNUSED DREF DeeObject *xchnonnullref(DeeObject *newval) const DEE_CXX_NOTHROW {
		return DeeCell_XchNonNull(m_ptr, newval);
	}
	WUNUSED DREF DeeObject *cmpxchref(DeeObject *oldval, DeeObject *newval) const DEE_CXX_NOTHROW {
		return DeeCell_CmpXch(m_ptr, oldval, newval);
	}
	Object get() const {
		return inherit(getref());
	}
	Object get(DeeObject *__restrict def) const {
		return inherit(getref(def));
	}
	void del() const {
		throw_if_nonzero(DeeCell_Del(m_ptr));
	}
	void set(DeeObject *__restrict ob) const {
		throw_if_nonzero(DeeCell_Set(m_ptr, ob));
	}
	bool bound() const DEE_CXX_NOTHROW {
		return DeeCell_Bound(m_ptr);
	}
};


DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_CELL_H */
