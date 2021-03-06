/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_FLOAT_H
#define GUARD_DEEMON_CXX_FLOAT_H 1

#include "api.h"

#include "../float.h"
#include "object.h"

DEE_CXX_BEGIN

class float_: public Object {
public:
	static DeeTypeObject *classtype() DEE_CXX_NOTHROW {
		return &DeeFloat_Type;
	}
	static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeFloat_Check(ob);
	}
	static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeFloat_CheckExact(ob);
	}

public:
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(float_, Object)
	float_()
	    : Object(inherit(DeeFloat_New(0.0))) {}
	float_(float value)
	    : Object(inherit(DeeFloat_New((double)value))) {}
	float_(double value)
	    : Object(inherit(DeeFloat_New(value))) {}
#ifdef __COMPILER_HAVE_LONGDOUBLE
	float_(long double value)
	    : Object(inherit(DeeFloat_New((double)value))) {}
#endif /* __COMPILER_HAVE_LONGDOUBLE */
#ifndef __OPTIMIZE_SIZE__
	using Object::getval;
	float_ const &getval(float &value) const {
		if likely(DeeFloat_CheckExact(this->ptr()))
			value = (float)DeeFloat_VALUE(this->ptr());
		else Object::getval(value);
		return *this;
	}
	float_ const &getval(double &value) const {
		if likely(DeeFloat_CheckExact(this->ptr()))
			value = (double)DeeFloat_VALUE(this->ptr());
		else Object::getval(value);
		return *this;
	}
#ifdef __COMPILER_HAVE_LONGDOUBLE
	float_ const &getval(long double &value) const {
		if likely(DeeFloat_CheckExact(this->ptr()))
			value = (long double)DeeFloat_VALUE(this->ptr());
		else Object::getval(value);
		return *this;
	}
#endif /* __COMPILER_HAVE_LONGDOUBLE */
	WUNUSED float asfloat() const {
		float result;
		getval(result);
		return result;
	}
	WUNUSED double asdouble() const {
		double result;
		getval(result);
		return result;
	}
#ifdef __COMPILER_HAVE_LONGDOUBLE
	WUNUSED long double asldouble() const {
		long double result;
		getval(result);
		return result;
	}
#endif /* __COMPILER_HAVE_LONGDOUBLE */
	explicit WUNUSED operator float() const {
		float result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator double() const {
		double result;
		getval(result);
		return result;
	}
#ifdef __COMPILER_HAVE_LONGDOUBLE
	explicit WUNUSED operator long double() const {
		long double result;
		getval(result);
		return result;
	}
#endif /* __COMPILER_HAVE_LONGDOUBLE */
#endif /* !__OPTIMIZE_SIZE__ */
};



DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_FLOAT_H */
