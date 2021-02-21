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
#ifndef GUARD_DEEMON_CXX_OBJECT_H
#define GUARD_DEEMON_CXX_OBJECT_H 1

#include "api.h"
/**/

#include <exception>
#include <initializer_list>
#include <stddef.h>
#include <type_traits>
#include <hybrid/typecore.h>

#include "../bool.h"
#include "../error.h"
#include "../float.h"
#include "../int.h"
#include "../none.h"
#include "../object.h"
#include "../string.h"
#include "../tuple.h"

#include "../system-features.h" /* strlen(), wcslen() */

DEE_CXX_BEGIN

FORCELOCAL WUNUSED DREF DeeObject *(DCALL incref)(DeeObject *__restrict obj) {
	Dee_Incref(obj);
	return obj;
}

FORCELOCAL WUNUSED DREF DeeObject *(DCALL xincref)(DeeObject *obj) {
	Dee_XIncref(obj);
	return obj;
}

FORCELOCAL void (DCALL decref)(DREF DeeObject *__restrict obj) {
	Dee_Decref(obj);
}

FORCELOCAL void (DCALL decref_likely)(DREF DeeObject *__restrict obj) {
	Dee_Decref_likely(obj);
}

FORCELOCAL void (DCALL decref_unlikely)(DREF DeeObject *__restrict obj) {
	Dee_Decref_unlikely(obj);
}

FORCELOCAL void (DCALL decref_nokill)(DREF DeeObject *__restrict obj) {
	Dee_DecrefNokill(obj);
}

FORCELOCAL void (DCALL decref_dokill)(DREF DeeObject *__restrict obj) {
	Dee_DecrefDokill(obj);
}

FORCELOCAL bool (DCALL decref_ifone)(DREF DeeObject *__restrict obj) {
	return Dee_DecrefIfOne(obj);
}

FORCELOCAL bool (DCALL decref_ifnotone)(DREF DeeObject *__restrict obj) {
	return Dee_DecrefIfNotOne(obj);
}

FORCELOCAL void (DCALL xdecref)(DREF DeeObject *obj) {
	Dee_XDecref(obj);
}

FORCELOCAL void (DCALL xdecref_nokill)(DREF DeeObject *obj) {
	Dee_XDecrefNokill(obj);
}
#ifdef CONFIG_TRACE_REFCHANGES
FORCELOCAL WUNUSED DREF DeeObject *(DCALL incref_traced)(DeeObject *__restrict obj, char const *file, int line) {
	Dee_Incref_traced(obj, file, line);
	return obj;
}

FORCELOCAL WUNUSED DREF DeeObject *(DCALL xincref_traced)(DeeObject *obj, char const *file, int line) {
	if (obj)
		Dee_Incref_traced(obj, file, line);
	return obj;
}

FORCELOCAL DeeObject *(DCALL decref_traced)(DREF DeeObject *__restrict obj, char const *file, int line) {
	Dee_Decref_traced(obj, file, line);
	return obj;
}

FORCELOCAL DeeObject *(DCALL decref_likely_traced)(DREF DeeObject *__restrict obj, char const *file, int line) {
	Dee_Decref_likely_traced(obj, file, line);
	return obj;
}

FORCELOCAL DeeObject *(DCALL decref_unlikely_traced)(DREF DeeObject *__restrict obj, char const *file, int line) {
	Dee_Decref_unlikely_traced(obj, file, line);
	return obj;
}

FORCELOCAL DeeObject *(DCALL decref_nokill_traced)(DREF DeeObject *__restrict obj, char const *file, int line) {
	Dee_DecrefNokill_traced(obj, file, line);
	return obj;
}

FORCELOCAL void (DCALL decref_dokill_traced)(DREF DeeObject *__restrict obj, char const *file, int line) {
	Dee_DecrefDokill_traced(obj, file, line);
}

FORCELOCAL bool (DCALL decref_ifone_traced)(DREF DeeObject *__restrict obj, char const *file, int line) {
	return Dee_DecrefIfOne_traced(obj, file, line);
}

FORCELOCAL bool (DCALL decref_ifnotone_traced)(DREF DeeObject *__restrict obj, char const *file, int line) {
	return Dee_DecrefIfNotOne_traced(obj, file, line);
}

FORCELOCAL void (DCALL xdecref_traced)(DREF DeeObject *obj, char const *file, int line) {
	if (obj)
		Dee_Decref_traced(obj, file, line);
}

FORCELOCAL void (DCALL xdecref_nokill_traced)(DREF DeeObject *obj, char const *file, int line) {
	if (obj)
		Dee_DecrefNokill_traced(obj, file, line);
}
#define incref(obj)          incref_traced(obj, __FILE__, __LINE__)
#define xincref(obj)         xincref_traced(obj, __FILE__, __LINE__)
#define decref(obj)          decref_traced(obj, __FILE__, __LINE__)
#define decref_nokill(obj)   decref_nokill_traced(obj, __FILE__, __LINE__)
#define decref_dokill(obj)   decref_dokill_traced(obj, __FILE__, __LINE__)
#define decref_ifone(obj)    decref_ifone_traced(obj, __FILE__, __LINE__)
#define decref_ifnotone(obj) decref_ifnotone_traced(obj, __FILE__, __LINE__)
#define xdecref(obj)         xdecref_traced(obj, __FILE__, __LINE__)
#define xdecref_nokill(obj)  xdecrefnokill_traced(obj, __FILE__, __LINE__)
#endif /* !CONFIG_TRACE_REFCHANGES */

/* Throws the latest deemon exception as a C++ error. */
inline ATTR_NORETURN ATTR_COLD void (DCALL throw_last_deemon_exception)(void);
inline ATTR_RETNONNULL void *(DCALL throw_if_null)(void *ptr) {
	if unlikely(!ptr)
		throw_last_deemon_exception();
	return ptr;
}
inline ATTR_RETNONNULL DeeObject *(DCALL throw_if_null)(DeeObject *obj) {
	if unlikely(!obj)
		throw_last_deemon_exception();
	return obj;
}
inline unsigned int (DCALL throw_if_negative)(int x) {
	if unlikely(x < 0)
		throw_last_deemon_exception();
	return (unsigned int)x;
}
inline unsigned long (DCALL throw_if_negative)(long x) {
	if unlikely(x < 0l)
		throw_last_deemon_exception();
	return (unsigned long)x;
}
#ifdef __COMPILER_HAVE_LONGLONG
inline __ULONGLONG (DCALL throw_if_negative)(__LONGLONG x) {
	if unlikely(x < 0ll)
		throw_last_deemon_exception();
	return (__ULONGLONG)x;
}
#endif /* __COMPILER_HAVE_LONGLONG */
inline unsigned int (DCALL throw_if_nonzero)(int x) {
	if unlikely(x != 0)
		throw_last_deemon_exception();
	return (unsigned int)x;
}
inline unsigned int (DCALL throw_if_minusone)(int x) {
	if unlikely(x == -1)
		throw_last_deemon_exception();
	return (unsigned int)x;
}
inline unsigned long (DCALL throw_if_minusone)(long x) {
	if unlikely(x == -1l)
		throw_last_deemon_exception();
	return (unsigned long)x;
}
#ifdef __COMPILER_HAVE_LONGLONG
inline __ULONGLONG (DCALL throw_if_minusone)(__LONGLONG x) {
	if unlikely(x == -1ll)
		throw_last_deemon_exception();
	return (__ULONGLONG)x;
}
#endif /* __COMPILER_HAVE_LONGLONG */
inline unsigned int (DCALL throw_if_minusone)(unsigned int x) {
	if unlikely(x == (unsigned int)-1)
		throw_last_deemon_exception();
	return x;
}
inline unsigned long (DCALL throw_if_minusone)(unsigned long x) {
	if unlikely(x == (unsigned long)-1l)
		throw_last_deemon_exception();
	return x;
}
#ifdef __COMPILER_HAVE_LONGLONG
inline __ULONGLONG (DCALL throw_if_minusone)(__ULONGLONG x) {
	if unlikely(x == (__ULONGLONG)-1ll)
		throw_last_deemon_exception();
	return x;
}
#endif /* __COMPILER_HAVE_LONGLONG */

#define DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(name)                  \
	class name {                                                     \
		DeeObject *m_ptr;                                            \
	public:                                                          \
		operator DeeObject *() const {                               \
			return m_ptr;                                            \
		}                                                            \
		explicit name(DeeObject *ptr) DEE_CXX_NOTHROW: m_ptr(ptr) {} \
		name(name const &ptr) DEE_CXX_NOTHROW: m_ptr(ptr.m_ptr) {}   \
	};
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(obj_nonnull)
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(obj_maybenull)
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(obj_inherited)
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(obj_nonnull_inherited)
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(obj_maybenull_inherited)
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(obj_string)
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(obj_file)
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(obj_tuple)
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(obj_sequence)
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(obj_mapping)
#undef DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER


class exception: public std::exception {
public:
	virtual const char *what() const DEE_CXX_NOTHROW {
		DeeObject *current = DeeError_Current();
		return current
		       ? Dee_TYPE(current)->tp_name
		       : "No exception";
	}
};



/* Indicate that is-NULL checks to throw an exception can
 * be omitted, because an object pointer is never NULL. */
inline WUNUSED NONNULL((1)) obj_nonnull DCALL nonnull(DeeObject *__restrict ptr) {
	return obj_nonnull(ptr);
}
inline WUNUSED obj_nonnull DCALL nonnull(obj_nonnull ptr) {
	return obj_nonnull((DeeObject *)ptr);
}
inline WUNUSED obj_nonnull_inherited DCALL nonnull(obj_inherited ptr) {
	return obj_nonnull_inherited((DeeObject *)ptr);
}
inline WUNUSED obj_nonnull_inherited DCALL nonnull(obj_nonnull_inherited ptr) {
	return obj_nonnull_inherited((DeeObject *)ptr);
}

/* Indicate that is-NULL checks to throw an exception can
 * be omitted, because a NULL-object is allowed. */
inline WUNUSED obj_maybenull DCALL maybenull(DeeObject *ptr) {
	return obj_maybenull(ptr);
}
inline WUNUSED obj_maybenull DCALL maybenull(obj_maybenull ptr) {
	return obj_maybenull((DeeObject *)ptr);
}
inline WUNUSED obj_maybenull_inherited DCALL maybenull(obj_inherited ptr) {
	return obj_maybenull_inherited((DeeObject *)ptr);
}
inline WUNUSED obj_maybenull_inherited DCALL maybenull(obj_maybenull_inherited ptr) {
	return obj_maybenull_inherited((DeeObject *)ptr);
}

/* Indicate that an object reference should be inherited. */
inline WUNUSED NONNULL((1)) obj_inherited DCALL inherit(DeeObject *__restrict ptr) {
	return obj_inherited(ptr);
}
inline WUNUSED obj_inherited DCALL inherit(obj_inherited ptr) {
	return obj_inherited((DeeObject *)ptr);
}
inline WUNUSED obj_nonnull_inherited DCALL inherit(obj_nonnull ptr) {
	return obj_nonnull_inherited((DeeObject *)ptr);
}
inline WUNUSED obj_nonnull_inherited DCALL inherit(obj_nonnull_inherited ptr) {
	return obj_nonnull_inherited((DeeObject *)ptr);
}

class Object;
class string;
class Bytes;
class Super;
template<class T = Object> class Type;
template<class T = Object> class Iterator;
template<class T = Object> class Sequence;
template<class T = Object> class Tuple;
template<class Prototype> class Function;
class int_;
class float_;

namespace detail {

class object_base {
protected:
	DREF DeeObject *m_ptr; /* [1..1][lock(CALLER)] The represented object. */
public:
#define DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(T, ...)                                 \
	T(T &&right)  DEE_CXX_NOTHROW: __VA_ARGS__(std::move((__VA_ARGS__ &)right)) {} \
	T(T const &right) DEE_CXX_NOTHROW: __VA_ARGS__(right) {}                       \
	T(DeeObject *obj) DEE_CXX_NOTHROW: __VA_ARGS__(obj) {}                         \
	T(obj_nonnull obj) DEE_CXX_NOTHROW: __VA_ARGS__(obj) {}                        \
	T(obj_maybenull obj) DEE_CXX_NOTHROW: __VA_ARGS__(obj) {}                      \
	T(obj_inherited obj) DEE_CXX_NOTHROW: __VA_ARGS__(obj) {}                      \
	T(obj_nonnull_inherited obj) DEE_CXX_NOTHROW: __VA_ARGS__(obj) {}              \
	T(obj_maybenull_inherited obj) DEE_CXX_NOTHROW: __VA_ARGS__(obj) {}            \
	T &operator=(T &&right) DEE_CXX_NOTHROW {                                      \
		__VA_ARGS__::operator=(std::move((__VA_ARGS__ &)right));                   \
		return *this;                                                              \
	}                                                                              \
	T &operator=(T const &right) DEE_CXX_NOTHROW {                                 \
		__VA_ARGS__::operator=(right);                                             \
		return *this;                                                              \
	}                                                                              \
	T copy() const {                                                               \
		return inherit(DeeObject_Copy(*this));                                     \
	}                                                                              \
	T deepcopy() const {                                                           \
		return inherit(DeeObject_DeepCopy(*this));                                 \
	}                                                                              \
	T &inplace_deepcopy() {                                                        \
		throw_if_nonzero(DeeObject_InplaceDeepCopy(&this->m_ptr));                 \
		return *this;                                                              \
	}
	object_base() DEE_CXX_NOTHROW: m_ptr(NULL) {}
	object_base(object_base &&right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr) {
		right.m_ptr = NULL;
	}
	object_base(object_base const &right) DEE_CXX_NOTHROW: m_ptr(incref(right.m_ptr)) {}
	object_base(DeeObject *obj)
	    : m_ptr(incref(throw_if_null(obj))) {}
	object_base(obj_nonnull obj) DEE_CXX_NOTHROW: m_ptr(incref(obj)) {}
	object_base(obj_maybenull obj) DEE_CXX_NOTHROW: m_ptr(xincref(obj)) {}
	object_base(obj_inherited obj)
	    : m_ptr(throw_if_null(obj)) {}
	object_base(obj_nonnull_inherited obj) DEE_CXX_NOTHROW: m_ptr(obj) {}
	object_base(obj_maybenull_inherited obj) DEE_CXX_NOTHROW: m_ptr(obj) {}
	~object_base() DEE_CXX_NOTHROW {
		Dee_XDecref(m_ptr);
	}

	DeeObject *(ptr)() const DEE_CXX_NOTHROW {
		return m_ptr;
	}
	operator DeeObject *(void)const DEE_CXX_NOTHROW {
		return m_ptr;
	}
	bool(isnull)() const DEE_CXX_NOTHROW {
		return m_ptr == NULL;
	}
	bool(isnone)() const DEE_CXX_NOTHROW {
		return DeeNone_Check(m_ptr);
	}
	bool(is)(DeeTypeObject *tp) const DEE_CXX_NOTHROW {
		return DeeObject_InstanceOf(m_ptr, tp);
	}
	bool(isexact)(DeeTypeObject *tp) const DEE_CXX_NOTHROW {
		return DeeObject_InstanceOfExact(m_ptr, tp);
	}
	template<class T> bool(is)() const DEE_CXX_NOTHROW {
		return T::check(this->ptr());
	}
	template<class T> bool(isexact)() const DEE_CXX_NOTHROW {
		return T::checkexact(this->ptr());
	}
	object_base &operator=(object_base &&right) DEE_CXX_NOTHROW {
		Dee_XDecref(this->m_ptr);
		this->m_ptr = right.m_ptr;
		right.m_ptr = NULL;
		return *this;
	}
	object_base &operator=(object_base const &right) DEE_CXX_NOTHROW {
		Dee_XDecref(this->m_ptr);
		this->m_ptr = xincref(right.m_ptr);
		return *this;
	}
	object_base &operator=(DeeObject *obj) {
		throw_if_null(obj);
		xdecref(m_ptr);
		m_ptr = incref(obj);
		return *this;
	}
	object_base &operator=(obj_nonnull obj) DEE_CXX_NOTHROW {
		xdecref(m_ptr);
		m_ptr = incref(obj);
		return *this;
	}
	object_base &operator=(obj_maybenull obj) DEE_CXX_NOTHROW {
		xdecref(m_ptr);
		m_ptr = xincref(obj);
		return *this;
	}
	object_base &operator=(obj_inherited obj) {
		throw_if_null(obj);
		xdecref(m_ptr);
		m_ptr = incref(obj);
		return *this;
	}
	object_base &operator=(obj_nonnull_inherited obj) DEE_CXX_NOTHROW {
		xdecref(m_ptr);
		m_ptr = obj;
		return *this;
	}
	object_base &operator=(obj_maybenull_inherited obj) DEE_CXX_NOTHROW {
		xdecref(m_ptr);
		m_ptr = obj;
		return *this;
	}
};

template<class T>
class cxx_iterator {
private:
	DREF DeeObject *it_iter; /* [0..1] Underlying iterator. */
	DREF DeeObject *it_next; /* [0..1] Next element (NULL if not queried). */
public:
	~cxx_iterator() DEE_CXX_NOTHROW {
		Dee_XDecref(it_iter);
		if (ITER_ISOK(it_next))
			Dee_Decref(it_next);
	}
	cxx_iterator() DEE_CXX_NOTHROW
	    : it_iter(NULL)
	    , it_next(NULL) {}
	cxx_iterator(cxx_iterator &&right) DEE_CXX_NOTHROW
	    : it_iter(right.it_iter)
	    , it_next(right.it_next) {
		right.it_iter = NULL;
		right.it_next = NULL;
	}
	cxx_iterator(cxx_iterator const &right) DEE_CXX_NOTHROW
	    : it_iter(xincref(right.it_iter))
	    , it_next(right.it_next) {
		if (ITER_ISOK(it_next))
			Dee_Incref(it_next);
	}
	cxx_iterator(DeeObject *iter)
	    : it_iter(throw_if_null(iter))
	    , it_next(throw_if_null(DeeObject_IterNext(iter))) {
		Dee_Incref(it_iter);
	}
	cxx_iterator(obj_inherited iter)
	    : it_iter(throw_if_null(iter))
	    , it_next(DeeObject_IterNext(iter)) {
		if unlikely(!it_next) {
			Dee_Decref((DeeObject *)iter);
			throw_last_deemon_exception();
		}
	}
	cxx_iterator(obj_nonnull iter) DEE_CXX_NOTHROW
	    : it_iter(iter)
	    , it_next(throw_if_null(DeeObject_IterNext(iter))) {
		Dee_Incref((DeeObject *)iter);
	}
	cxx_iterator(obj_nonnull_inherited iter) DEE_CXX_NOTHROW
	    : it_iter(iter)
	    , it_next(DeeObject_IterNext(iter)) {
		if unlikely(!it_next) {
			Dee_Decref((DeeObject *)iter);
			throw_last_deemon_exception();
		}
	}
	bool operator==(cxx_iterator const &right) const {
		return !right.it_iter
		       ? it_next == ITER_DONE
		       : throw_if_negative(DeeObject_CompareEq(it_iter, right.it_iter)) != 0;
	}
	bool operator!=(cxx_iterator const &right) const {
		return !right.it_iter
		       ? it_next != ITER_DONE
		       : throw_if_negative(DeeObject_CompareNe(it_iter, right.it_iter)) != 0;
	}
	bool operator<(cxx_iterator const &right) const {
		return !right.it_iter
		       ? it_next != ITER_DONE
		       : throw_if_negative(DeeObject_CompareLo(it_iter, right.it_iter)) != 0;
	}
	bool operator<=(cxx_iterator const &right) const {
		return !right.it_iter
		       ? true
		       : throw_if_negative(DeeObject_CompareLe(it_iter, right.it_iter)) != 0;
	}
	bool operator>(cxx_iterator const &right) const {
		return !right.it_iter
		       ? false
		       : throw_if_negative(DeeObject_CompareGr(it_iter, right.it_iter)) != 0;
	}
	bool operator>=(cxx_iterator const &right) const {
		return !right.it_iter
		       ? it_next == ITER_DONE
		       : throw_if_negative(DeeObject_CompareGe(it_iter, right.it_iter)) != 0;
	}
	cxx_iterator &operator=(cxx_iterator &&right) DEE_CXX_NOTHROW {
		Dee_XDecref(it_iter);
		if (ITER_ISOK(it_next))
			Dee_Decref(it_next);
		it_iter       = right.it_iter;
		it_next       = right.it_next;
		right.it_iter = NULL;
		right.it_next = NULL;
		return *this;
	}
	cxx_iterator &operator=(cxx_iterator const &right) {
		Dee_XDecref(it_iter);
		if (ITER_ISOK(it_next))
			Dee_Decref(it_next);
		it_iter = right.it_iter;
		it_next = right.it_next;
		Dee_XIncref(it_iter);
		if (ITER_ISOK(it_next))
			Dee_Incref(it_next);
		return *this;
	}
	cxx_iterator copy() const {
		return inherit(DeeObject_Copy(this->it_iter));
	}
	cxx_iterator deepcopy() const {
		return inherit(DeeObject_DeepCopy(this->it_iter));
	}
	cxx_iterator &inplace_deepcopy() {
		throw_if_nonzero(DeeObject_InplaceDeepCopy(&this->it_iter));
		return this->it_iter;
	}
	T operator*() {
		return it_next;
	}
	cxx_iterator &operator++() {
		if (ITER_ISOK(it_next))
			Dee_Decref(it_next);
		it_next = throw_if_null(DeeObject_IterNext(this->it_iter));
		return *this;
	}
	cxx_iterator operator++(int) {
		DREF DeeObject *result;
		result = throw_if_null(DeeObject_Copy(this->it_iter));
		if (ITER_ISOK(it_next))
			Dee_Decref(it_next);
		it_next = DeeObject_IterNext(this->it_iter);
		if unlikely(!it_next) {
			Dee_Decref(result);
			throw_last_deemon_exception();
		}
		return inherit(nonnull(result));
	}
};


class any_convertible_base {
public:
	typedef int available;
	enum { exists = true };
};
template<class T, bool C> class any_convertible_cc {
	enum { exists = false };
};
template<class T> class any_convertible_cc<T, true>: public any_convertible_base {
public:
	static DREF DeeObject *convert(T const &value) {
		DREF DeeObject *result;
		result = (DREF DeeObject *)value;
		Dee_Incref(result);
		return result;
	}
};
template<class T> class any_convertible
    : public any_convertible_cc<T, std::is_convertible<T, DeeObject *>::value> {};

template<> class any_convertible<bool>: public any_convertible_base {
public:
	static DREF DeeObject *convert(char value) {
		return_bool_(value);
	}
};
template<> class any_convertible<char>: public any_convertible_base {
public:
	static DREF DeeObject *convert(char value) {
		return DeeInt_NewChar(value);
	}
};
template<> class any_convertible<signed char>: public any_convertible_base {
public:
	static DREF DeeObject *convert(signed char value) {
		return DeeInt_NewSChar(value);
	}
};
template<> class any_convertible<unsigned char>: public any_convertible_base {
public:
	static DREF DeeObject *convert(unsigned char value) {
		return DeeInt_NewUChar(value);
	}
};
template<> class any_convertible<short>: public any_convertible_base {
public:
	static DREF DeeObject *convert(short value) {
		return DeeInt_NewShort(value);
	}
};
template<> class any_convertible<unsigned short>: public any_convertible_base {
public:
	static DREF DeeObject *convert(unsigned short value) {
		return DeeInt_NewUShort(value);
	}
};
template<> class any_convertible<int>: public any_convertible_base {
public:
	static DREF DeeObject *convert(int value) {
		return DeeInt_NewInt(value);
	}
};
template<> class any_convertible<unsigned int>: public any_convertible_base {
public:
	static DREF DeeObject *convert(unsigned int value) {
		return DeeInt_NewUInt(value);
	}
};
template<> class any_convertible<long>: public any_convertible_base {
public:
	static DREF DeeObject *convert(long value) {
		return DeeInt_NewLong(value);
	}
};
template<> class any_convertible<unsigned long>: public any_convertible_base {
public:
	static DREF DeeObject *convert(unsigned long value) {
		return DeeInt_NewULong(value);
	}
};
#ifdef __COMPILER_HAVE_LONGLONG
template<> class any_convertible<__LONGLONG>: public any_convertible_base {
public:
	static DREF DeeObject *convert(long value) {
		return DeeInt_NewLLong(value);
	}
};
template<> class any_convertible<__ULONGLONG>: public any_convertible_base {
public:
	static DREF DeeObject *convert(unsigned long value) {
		return DeeInt_NewULLong(value);
	}
};
#endif /* __COMPILER_HAVE_LONGLONG */
template<> class any_convertible<Dee_int128_t>: public any_convertible_base {
public:
	static DREF DeeObject *convert(Dee_int128_t value) {
		return DeeInt_NewS128(value);
	}
};
template<> class any_convertible<Dee_uint128_t>: public any_convertible_base {
public:
	static DREF DeeObject *convert(Dee_uint128_t value) {
		return DeeInt_NewU128(value);
	}
};
template<> class any_convertible<float>: public any_convertible_base {
public:
	static DREF DeeObject *convert(float value) {
		return DeeFloat_New((double)value);
	}
};
template<> class any_convertible<double>: public any_convertible_base {
public:
	static DREF DeeObject *convert(double value) {
		return DeeFloat_New(value);
	}
};
template<> class any_convertible<long double>: public any_convertible_base {
public:
	static DREF DeeObject *convert(long double value) {
		return DeeFloat_New((double)value);
	}
};
template<> class any_convertible</*utf-8*/ char const *>: public any_convertible_base {
public:
	static DREF DeeObject *convert(/*utf-8*/ char const *__restrict value) {
		return DeeString_NewUtf8(value, strlen(value), Dee_STRING_ERROR_FSTRICT);
	}
};
#ifdef __native_wchar_t_defined
#ifndef _dee_wcslen
#ifdef CONFIG_HAVE_wcslen
#define _dee_wcslen(str) ::wcslen(str)
#else /* CONFIG_HAVE_wcslen */
#define _dee_wcslen(str) (::deemon::detail::_dee_wcslen)(str)
DeeSystem_DEFINE_wcslen(_dee_wcslen)
#endif /* !CONFIG_HAVE_wcslen */
#endif /* !_dee_wcslen */
template<> class any_convertible</*wide*/ wchar_t const *>: public any_convertible_base {
public:
	static DREF DeeObject *convert(/*wide*/ wchar_t const *__restrict value) {
		return DeeString_NewWide(value, _dee_wcslen(value), Dee_STRING_ERROR_FSTRICT);
	}
};
#endif /* __native_wchar_t_defined */

#ifdef __native_char16_t_defined
#ifndef _dee_c16len
DeeSystem_DEFINE_XSTRLEN(_dee_c16len, uint16_t)
#define _dee_c16len(str) _dee_c16len(str)
#endif /* !_dee_c16len */
#ifndef _dee_c32len
DeeSystem_DEFINE_XSTRLEN(_dee_c32len, uint32_t)
#define _dee_c32len(str) _dee_c32len(str)
#endif /* !_dee_c32len */
template<> class any_convertible<char16_t const *>: public any_convertible_base {
public:
	static DREF DeeObject *convert(char16_t const *__restrict value) {
		return DeeString_NewUtf16((uint16_t const *)value, _dee_c16len((uint16_t const *)value), Dee_STRING_ERROR_FSTRICT);
	}
};
template<> class any_convertible<char32_t const *>: public any_convertible_base {
public:
	static DREF DeeObject *convert(char32_t const *__restrict value) {
		return DeeString_NewUtf32((uint32_t const *)value, _dee_c32len((uint32_t const *)value), Dee_STRING_ERROR_FSTRICT);
	}
};
#endif /* __native_char16_t_defined */

#undef DEFINE_CONVERTIBLE
template<size_t sz>
class any_convertible<char const[sz]>: public any_convertible_base {
public:
	static DREF DeeObject *convert(/*utf-8*/ char const (&value)[sz]) {
		return DeeString_NewUtf8(value, sz - 1, Dee_STRING_ERROR_FSTRICT);
	}
};
template<size_t sz> class any_convertible<char[sz]>: public any_convertible<char const[sz]> {};
#ifdef __native_wchar_t_defined
template<size_t sz>
class any_convertible<wchar_t const[sz]>: public any_convertible_base {
public:
	static DREF DeeObject *convert(/*utf-8*/ wchar_t const (&value)[sz]) {
		return DeeString_NewWide(value, sz - 1, Dee_STRING_ERROR_FSTRICT);
	}
};
template<size_t sz> class any_convertible<wchar_t[sz]>
    : public any_convertible<wchar_t const[sz]> {};
#endif /* __native_wchar_t_defined */
#ifdef __native_char16_t_defined
template<size_t sz>
class any_convertible<char16_t const[sz]>: public any_convertible_base {
public:
	static DREF DeeObject *convert(/*utf-8*/ char16_t const (&value)[sz]) {
		return DeeString_NewUtf16(value, sz - 1, Dee_STRING_ERROR_FSTRICT);
	}
};
template<size_t sz> class any_convertible<char16_t[sz]>
    : public any_convertible<char16_t const[sz]> {};
template<size_t sz>
class any_convertible<char32_t const[sz]>: public any_convertible_base {
public:
	static DREF DeeObject *convert(/*utf-8*/ char32_t const (&value)[sz]) {
		return DeeString_NewUtf32(value, sz - 1, Dee_STRING_ERROR_FSTRICT);
	}
};
template<size_t sz> class any_convertible<char32_t[sz]>
    : public any_convertible<char32_t const[sz]> {};
#endif /* __native_char16_t_defined */

template<class T>
class any_convertible<std::initializer_list<T> > {
public:
	typedef typename any_convertible<T>::available available;
	static DREF DeeObject *convert(std::initializer_list<T> const &values) {
		DREF DeeObject *result;
		T const *elem = values.begin();
		size_t i = 0, size = values.size();
		result = throw_if_null(DeeTuple_NewUninitialized(size));
		try {
			for (; i < size; ++i) {
				DREF DeeObject *temp;
				temp = any_convertible<T>::convert(elem[i]);
				DeeTuple_SET(result, i, temp);
			}
		} catch (...) {
			while (i--)
				Dee_Decref(DeeTuple_GET(result, i));
			DeeTuple_FreeUninitialized(result);
			throw;
		}
		return result;
	}
};

LOCAL ATTR_COLD ATTR_NORETURN void DCALL
err_cannot_weak_reference(DeeObject *__restrict ob) {
	ASSERT_OBJECT(ob);
	DeeError_Throwf(&DeeError_TypeError,
	                "Cannot create weak reference for instances of type `%k'",
	                Dee_TYPE(ob));
	throw_last_deemon_exception();
}

LOCAL ATTR_COLD ATTR_NORETURN void DCALL
err_cannot_lock_weakref(void) {
	DeeError_Throwf(&DeeError_ReferenceError,
	                "Cannot lock weak reference");
	throw_last_deemon_exception();
}


} /* namespace detail... */


class WeakRef {
private:
	struct ::Dee_weakref w_ref; /* The underlying weak reference. */
public:
	WeakRef() DEE_CXX_NOTHROW {
		Dee_weakref_null(&w_ref);
	}
	WeakRef(DeeObject *ob) {
		if (ob) {
			if (!Dee_weakref_init(&w_ref, ob, NULL))
				detail::err_cannot_weak_reference(ob);
		} else {
			Dee_weakref_null(&w_ref);
		}
	}
	WeakRef(obj_nonnull ob) {
		if (!Dee_weakref_init(&w_ref, ob, NULL))
			detail::err_cannot_weak_reference(ob);
	}
	WeakRef(obj_maybenull ob) {
		if (ob) {
			if (!Dee_weakref_init(&w_ref, ob, NULL))
				detail::err_cannot_weak_reference(ob);
		} else {
			Dee_weakref_null(&w_ref);
		}
	}
	WeakRef(WeakRef const &other) DEE_CXX_NOTHROW {
		Dee_weakref_copy(&w_ref, &other.w_ref);
	}
	WeakRef(WeakRef &&other) DEE_CXX_NOTHROW {
		Dee_weakref_move(&w_ref, &other.w_ref);
	}
	~WeakRef() DEE_CXX_NOTHROW {
		Dee_weakref_fini(&w_ref);
	}
	WeakRef &operator=(DeeObject *ob) {
		if (ob) {
			if (!ob) {
				Dee_weakref_clear(&w_ref);
			} else if (!Dee_weakref_set(&w_ref, ob)) {
				detail::err_cannot_weak_reference(ob);
			}
		} else {
			Dee_weakref_clear(&w_ref);
		}
		return *this;
	}
	WeakRef &operator=(obj_nonnull ob) {
		if (!Dee_weakref_set(&w_ref, ob))
			detail::err_cannot_weak_reference(ob);
		return *this;
	}
	WeakRef &operator=(obj_maybenull ob) {
		if (ob) {
			if (!ob) {
				Dee_weakref_clear(&w_ref);
			} else if (!Dee_weakref_set(&w_ref, ob)) {
				detail::err_cannot_weak_reference(ob);
			}
		} else {
			Dee_weakref_clear(&w_ref);
		}
		return *this;
	}
	WeakRef &operator=(WeakRef &&other) DEE_CXX_NOTHROW {
		Dee_weakref_moveassign(&w_ref, &other.w_ref);
		return *this;
	}
	WeakRef &operator=(WeakRef const &other) DEE_CXX_NOTHROW {
		Dee_weakref_copyassign(&w_ref, &other.w_ref);
		return *this;
	}
	DREF DeeObject *trylockref() const DEE_CXX_NOTHROW {
		return Dee_weakref_lock(&w_ref);
	}
	ATTR_RETNONNULL DREF DeeObject *lockref() const {
		DREF DeeObject *result = Dee_weakref_lock(&w_ref);
		if (!result)
			detail::err_cannot_lock_weakref();
		return result;
	}
	ATTR_RETNONNULL DREF DeeObject *lockref(DeeObject *defl) const DEE_CXX_NOTHROW {
		DREF DeeObject *result = Dee_weakref_lock(&w_ref);
		if (!result) {
			result = defl;
			Dee_Incref(defl);
		}
		return result;
	}
	inline WUNUSED Object trylock() const DEE_CXX_NOTHROW;
	inline WUNUSED Object lock() const;
	inline WUNUSED Object lock(DeeObject *defl) const DEE_CXX_NOTHROW;
	bool alive() const DEE_CXX_NOTHROW {
		return Dee_weakref_bound(&w_ref);
	}
	operator bool() const DEE_CXX_NOTHROW {
		return Dee_weakref_bound(&w_ref);
	}
	bool operator!() const DEE_CXX_NOTHROW {
		return !Dee_weakref_bound(&w_ref);
	}
	void clear() DEE_CXX_NOTHROW {
		Dee_weakref_clear(&w_ref);
	}
	DREF DeeObject *cmpxchref(DeeObject *old_ob, DeeObject *new_ob) DEE_CXX_NOTHROW {
		return Dee_weakref_cmpxch(&w_ref, old_ob, new_ob);
	}
	void set(DeeObject *ob) {
		if (ob) {
			if (!Dee_weakref_set(&w_ref, ob))
				detail::err_cannot_weak_reference(ob);
		} else {
			Dee_weakref_clear(&w_ref);
		}
	}
	void set(obj_nonnull ob) {
		if (!Dee_weakref_set(&w_ref, ob))
			detail::err_cannot_weak_reference(ob);
	}
	void set(obj_maybenull ob) {
		if (ob) {
			if (!Dee_weakref_init(&w_ref, ob, NULL))
				detail::err_cannot_weak_reference(ob);
		} else {
			Dee_weakref_clear(&w_ref);
		}
	}
};

class Object: public detail::object_base {
	template<class T> class proxy_base {
	public:
		Object get() const {
			return inherit(((T const *)this)->getref());
		}
		operator Object() const {
			return inherit(((T const *)this)->getref());
		}
		void set(DeeObject *value) const {
			*((T const *)this) = value;
		}
	};
	template<class T> class call_proxy_base: public proxy_base<T> {
	public:
		Object(call)() const {
			return inherit(((T const *)this)->callref());
		}
		Object(call)(obj_tuple args) const {
			return inherit(((T const *)this)->callref(args));
		}
		Object(call)(obj_tuple args, DeeObject *kw) const {
			return inherit(((T const *)this)->callref(args, kw));
		}
		Object(call)(size_t argc, DeeObject *const *argv) const {
			return inherit(((T const *)this)->callref(argc, argv));
		}
		Object(call)(size_t argc, DeeObject *const *argv, DeeObject *kw) const {
			return inherit(((T const *)this)->callref(argc, argv, kw));
		}
		Object(call)(size_t argc, DeeObject *const *argv) const {
			return inherit(((T const *)this)->callref(argc, (DeeObject **)argv));
		}
		Object(call)(size_t argc, DeeObject *const *argv, DeeObject *kw) const {
			return inherit(((T const *)this)->callref(argc, (DeeObject **)argv, kw));
		}
		Object(call)(size_t argc, Object **argv) const {
			return inherit(((T const *)this)->callref(argc, (DeeObject **)argv));
		}
		Object(call)(size_t argc, Object **argv, DeeObject *kw) const {
			return inherit(((T const *)this)->callref(argc, (DeeObject **)argv, kw));
		}
		Object(call)(size_t argc, Object *const *argv) const {
			return inherit(((T const *)this)->callref(argc, (DeeObject **)argv));
		}
		Object(call)(size_t argc, Object *const *argv, DeeObject *kw) const {
			return inherit(((T const *)this)->callref(argc, (DeeObject **)argv, kw));
		}
		Object(call)(std::initializer_list<DeeObject *> const &args) const {
			return inherit(((T const *)this)->callref(args.size(), (DeeObject **)args.begin()));
		}
		Object(call)(std::initializer_list<DeeObject *> const &args, DeeObject *kw) const {
			return inherit(((T const *)this)->callref(args.size(), (DeeObject **)args.begin(), kw));
		}
		WUNUSED Object operator()(obj_tuple args) const {
			return inherit(((T const *)this)->callref(args));
		}
		WUNUSED Object operator()(std::initializer_list<DeeObject *> const &args) const {
			return inherit(((T const *)this)->callref(args.size(), (DeeObject **)args.begin()));
		}
	};
	class attr_proxy_obj: public call_proxy_base<attr_proxy_obj> {
	private:
		DeeObject *m_ptr;
		DeeObject *m_str;

	public:
		attr_proxy_obj(DeeObject *ptr, DeeObject *str) DEE_CXX_NOTHROW: m_ptr(ptr)
		    , m_str(str) {}
		attr_proxy_obj(attr_proxy_obj const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr)
		    , m_str(right.m_str) {}
		WUNUSED DREF DeeObject *getref() const {
			return DeeObject_GetAttr(m_ptr, m_str);
		}
		WUNUSED DREF DeeObject *callref() const {
			return DeeObject_CallAttr(m_ptr, m_str, 0, NULL);
		}
		WUNUSED DREF DeeObject *callref(obj_tuple args) const {
			return DeeObject_CallAttrTuple(m_ptr, m_str, args);
		}
		WUNUSED DREF DeeObject *callref(obj_tuple args, DeeObject *kw) const {
			return DeeObject_CallAttrTupleKw(m_ptr, m_str, args, kw);
		}
		WUNUSED DREF DeeObject *callref(size_t argc, DeeObject *const *argv) const {
			return DeeObject_CallAttr(m_ptr, m_str, argc, argv);
		}
		WUNUSED DREF DeeObject *callref(size_t argc, DeeObject *const *argv, DeeObject *kw) const {
			return DeeObject_CallAttrKw(m_ptr, m_str, argc, argv, kw);
		}
		bool has() const {
			return throw_if_negative(DeeObject_HasAttr(m_ptr, m_str)) != 0;
		}
		bool bound() const {
			int result = DeeObject_BoundAttr(m_ptr, m_str);
			if (result == -1)
				throw_last_deemon_exception();
			return result > 0;
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttr(m_ptr, m_str));
		}
		attr_proxy_obj const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetAttr(m_ptr, m_str, value));
			return *this;
		}
	};
	class attr_proxy_str: public call_proxy_base<attr_proxy_str> {
	private:
		DeeObject *m_ptr;
		char const *m_str;

	public:
		attr_proxy_str(DeeObject *ptr, char const *str) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_str(str) {}
		attr_proxy_str(attr_proxy_str const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_str(right.m_str) {}
		DREF DeeObject *getref() const {
			return DeeObject_GetAttrString(m_ptr, m_str);
		}
		WUNUSED DREF DeeObject *(callref)() const {
			return DeeObject_CallAttrString(m_ptr, m_str, 0, NULL);
		}
		WUNUSED DREF DeeObject *(callref)(obj_tuple args) const {
			return DeeObject_CallAttrStringTuple(m_ptr, m_str, (DeeObject *)args);
		}
		WUNUSED DREF DeeObject *(callref)(obj_tuple args, DeeObject *kw) const {
			return DeeObject_CallAttrStringTupleKw(m_ptr, m_str, (DeeObject *)args, kw);
		}
		WUNUSED DREF DeeObject *(callref)(size_t argc, DeeObject *const *argv) const {
			return DeeObject_CallAttrString(m_ptr, m_str, argc, argv);
		}
		WUNUSED DREF DeeObject *(callref)(size_t argc, DeeObject *const *argv, DeeObject *kw) const {
			return DeeObject_CallAttrStringKw(m_ptr, m_str, argc, argv, kw);
		}
		bool(has)() const {
			return throw_if_negative(DeeObject_HasAttrString(m_ptr, m_str)) != 0;
		}
		bool(bound)() const {
			int result = DeeObject_BoundAttrString(m_ptr, m_str);
			if (result == -1)
				throw_last_deemon_exception();
			return result > 0;
		}
		void(del)() const {
			throw_if_nonzero(DeeObject_DelAttrString(m_ptr, m_str));
		}
		attr_proxy_str const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetAttrString(m_ptr, m_str, value));
			return *this;
		}
	};
	class attr_proxy_sth: public call_proxy_base<attr_proxy_sth> {
	private:
		DeeObject *m_ptr;
		char const *m_str;
		Dee_hash_t m_hsh;

	public:
		attr_proxy_sth(DeeObject *ptr, char const *str, Dee_hash_t hsh) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_str(str)
		    , m_hsh(hsh) {}
		attr_proxy_sth(attr_proxy_sth const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_str(right.m_str)
		    , m_hsh(right.m_hsh) {}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetAttrStringHash(m_ptr, m_str, m_hsh);
		}
		WUNUSED DREF DeeObject *(callref)() const {
			return DeeObject_CallAttrStringHash(m_ptr, m_str, m_hsh, 0, NULL);
		}
		WUNUSED DREF DeeObject *(callref)(obj_tuple args) const {
			return DeeObject_CallAttrStringHashTuple(m_ptr, m_str, m_hsh, (DeeObject *)args);
		}
		WUNUSED DREF DeeObject *(callref)(obj_tuple args, DeeObject *kw) const {
			return DeeObject_CallAttrStringHashTupleKw(m_ptr, m_str, m_hsh, (DeeObject *)args, kw);
		}
		WUNUSED DREF DeeObject *(callref)(size_t argc, DeeObject *const *argv) const {
			return DeeObject_CallAttrStringHash(m_ptr, m_str, m_hsh, argc, argv);
		}
		WUNUSED DREF DeeObject *(callref)(size_t argc, DeeObject *const *argv, DeeObject *kw) const {
			return DeeObject_CallAttrStringHashKw(m_ptr, m_str, m_hsh, argc, argv, kw);
		}
		bool(has)() const {
			return throw_if_negative(DeeObject_HasAttrStringHash(m_ptr, m_str, m_hsh)) != 0;
		}
		bool(bound)() const {
			int result = DeeObject_BoundAttrStringHash(m_ptr, m_str, m_hsh);
			if (result == -1)
				throw_last_deemon_exception();
			return result > 0;
		}
		void(del)() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_ptr, m_str, m_hsh));
		}
		attr_proxy_sth const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetAttrStringHash(m_ptr, m_str, m_hsh, value));
			return *this;
		}
	};
	class attr_proxy_snh: public call_proxy_base<attr_proxy_snh> {
	private:
		DeeObject *m_ptr;
		char const *m_str;
		size_t m_len;
		Dee_hash_t m_hsh;

	public:
		attr_proxy_snh(DeeObject *ptr, char const *str, size_t len, Dee_hash_t hsh) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_str(str)
		    , m_len(len)
		    , m_hsh(hsh) {}
		attr_proxy_snh(attr_proxy_snh const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_str(right.m_str)
		    , m_len(right.m_len)
		    , m_hsh(right.m_hsh) {}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetAttrStringHash(m_ptr, m_str, m_hsh);
		}
		WUNUSED DREF DeeObject *(callref)() const {
			return DeeObject_CallAttrStringLenHash(m_ptr, m_str, m_len, m_hsh, 0, NULL);
		}
		WUNUSED DREF DeeObject *(callref)(obj_tuple args) const {
			return DeeObject_CallAttrStringLenHashTuple(m_ptr, m_str, m_len, m_hsh, (DeeObject *)args);
		}
		WUNUSED DREF DeeObject *(callref)(obj_tuple args, DeeObject *kw) const {
			return DeeObject_CallAttrStringLenHashTupleKw(m_ptr, m_str, m_len, m_hsh, (DeeObject *)args, kw);
		}
		WUNUSED DREF DeeObject *(callref)(size_t argc, DeeObject *const *argv) const {
			return DeeObject_CallAttrStringLenHash(m_ptr, m_str, m_len, m_hsh, argc, argv);
		}
		WUNUSED DREF DeeObject *(callref)(size_t argc, DeeObject *const *argv, DeeObject *kw) const {
			return DeeObject_CallAttrStringLenHashKw(m_ptr, m_str, m_len, m_hsh, argc, argv, kw);
		}
		bool(has)() const {
			return throw_if_negative(DeeObject_HasAttrStringLenHash(m_ptr, m_str, m_len, m_hsh)) != 0;
		}
		bool(bound)() const {
			int result = DeeObject_BoundAttrStringLenHash(m_ptr, m_str, m_len, m_hsh);
			if (result == -1)
				throw_last_deemon_exception();
			return result > 0;
		}
		void(del)() const {
			throw_if_nonzero(DeeObject_DelAttrStringLenHash(m_ptr, m_str, m_len, m_hsh));
		}
		attr_proxy_snh const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetAttrStringLenHash(m_ptr, m_str, m_len, m_hsh, value));
			return *this;
		}
	};
	class item_proxy_obj: public proxy_base<item_proxy_obj> {
	private:
		DeeObject *m_ptr;
		DeeObject *m_str;

	public:
		item_proxy_obj(DeeObject *ptr, DeeObject *str) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_str(str) {}
		item_proxy_obj(item_proxy_obj const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_str(right.m_str) {}
		bool(has)() const {
			int result = DeeObject_HasItem(m_ptr, m_str);
			if (result < 0)
				throw_last_deemon_exception();
			return result != 0;
		}
		bool(bound)(bool allow_missing = true) const {
			int result = DeeObject_BoundItem(m_ptr, m_str, allow_missing);
			if (result == -1)
				throw_last_deemon_exception();
			return result > 0;
		}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetItem(m_ptr, m_str);
		}
		DREF DeeObject *(getref)(DeeObject *def) const {
			return DeeObject_GetItemDef(m_ptr, m_str, def);
		}
		using proxy_base::get;
		Object (get)(DeeObject *def) const {
			return inherit(getref(def));
		}
		void (del)() const {
			throw_if_nonzero(DeeObject_DelItem(m_ptr, m_str));
		}
		item_proxy_obj const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetItem(m_ptr, m_str, value));
			return *this;
		}
	};
	class item_proxy_idx: public proxy_base<item_proxy_idx> {
	private:
		DeeObject *m_ptr;
		size_t m_idx;

	public:
		item_proxy_idx(DeeObject *ptr, size_t idx) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_idx(idx) {}
		item_proxy_idx(item_proxy_idx const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_idx(right.m_idx) {}
		bool(has)() const {
			int result = DeeObject_HasItemIndex(m_ptr, m_idx);
			if (result < 0)
				throw_last_deemon_exception();
			return result != 0;
		}
		bool(bound)(bool allow_missing = true) const {
			int result = DeeObject_BoundItemIndex(m_ptr, m_idx, allow_missing);
			if (result == -1)
				throw_last_deemon_exception();
			return result > 0;
		}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetItemIndex(m_ptr, m_idx);
		}
		DREF DeeObject *(getref)(DeeObject *def) const {
			DREF DeeObject *result, *index_ob = throw_if_null(DeeInt_NewSize(m_idx));
			result = DeeObject_GetItemDef(m_ptr, index_ob, def);
			Dee_Decref(index_ob);
			return inherit(result);
		}
		using proxy_base::get;
		Object get(DeeObject *def) const {
			return inherit(getref(def));
		}
		void(del)() const {
			throw_if_nonzero(DeeObject_DelItemIndex(m_ptr, m_idx));
		}
		item_proxy_idx const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetItemIndex(m_ptr, m_idx, value));
			return *this;
		}
	};
	class item_proxy_sth: public proxy_base<item_proxy_sth> {
	private:
		DeeObject *m_ptr;
		char const *m_str;
		Dee_hash_t m_hsh;

	public:
		item_proxy_sth(DeeObject *ptr, char const *str) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_str(str)
		    , m_hsh(Dee_HashStr(str)) {}
		item_proxy_sth(DeeObject *ptr, char const *str, Dee_hash_t hsh) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_str(str)
		    , m_hsh(hsh) {}
		item_proxy_sth(item_proxy_sth const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_str(right.m_str)
		    , m_hsh(right.m_hsh) {}
		bool(has)() const {
			int result = DeeObject_HasItemString(m_ptr, m_str, m_hsh);
			if (result < 0)
				throw_last_deemon_exception();
			return result != 0;
		}
		bool(bound)(bool allow_missing = true) const {
			int result = DeeObject_BoundItemString(m_ptr, m_str, m_hsh, allow_missing);
			if (result == -1)
				throw_last_deemon_exception();
			return result > 0;
		}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetItemString(m_ptr, m_str, m_hsh);
		}
		DREF DeeObject *(getref)(DeeObject *def) const {
			return DeeObject_GetItemStringDef(m_ptr, m_str, m_hsh, def);
		}
		using proxy_base::get;
		Object(get)(DeeObject *def) const {
			return inherit(getref(def));
		}
		void(del)() const {
			throw_if_nonzero(DeeObject_DelItemString(m_ptr, m_str, m_hsh));
		}
		item_proxy_sth const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetItemString(m_ptr, m_str, m_hsh, value));
			return *this;
		}
	};
	class item_proxy_snh: public proxy_base<item_proxy_snh> {
	private:
		DeeObject *m_ptr;
		char const *m_str;
		size_t m_len;
		Dee_hash_t m_hsh;

	public:
		item_proxy_snh(DeeObject *ptr, char const *str, size_t len, Dee_hash_t hsh) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_str(str)
		    , m_len(len)
		    , m_hsh(hsh) {}
		item_proxy_snh(item_proxy_snh const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_str(right.m_str)
		    , m_len(right.m_len)
		    , m_hsh(right.m_hsh) {}
		bool(has)() const {
			int result = DeeObject_HasItemStringLen(m_ptr, m_str, m_len, m_hsh);
			if (result < 0)
				throw_last_deemon_exception();
			return result != 0;
		}
		bool(bound)(bool allow_missing = true) const {
			int result = DeeObject_BoundItemStringLen(m_ptr, m_str, m_len, m_hsh, allow_missing);
			if (result == -1)
				throw_last_deemon_exception();
			return result > 0;
		}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetItemStringLen(m_ptr, m_str, m_len, m_hsh);
		}
		DREF DeeObject *(getref)(DeeObject *def) const {
			return DeeObject_GetItemStringLenDef(m_ptr, m_str, m_len, m_hsh, def);
		}
		Object(get)(DeeObject *def) const {
			return inherit(getref(def));
		}
		void(del)() const {
			throw_if_nonzero(DeeObject_DelItemStringLen(m_ptr, m_str, m_len, m_hsh));
		}
		item_proxy_snh const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetItemStringLen(m_ptr, m_str, m_len, m_hsh, value));
			return *this;
		}
	};
	class range_proxy_oo: public proxy_base<range_proxy_oo> {
	private:
		DeeObject *m_ptr;
		DeeObject *m_bgn;
		DeeObject *m_end;

	public:
		range_proxy_oo(DeeObject *ptr, DeeObject *bgn, DeeObject *end) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_bgn(bgn)
		    , m_end(end) {}
		range_proxy_oo(range_proxy_oo const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_bgn(right.m_bgn)
		    , m_end(right.m_end) {}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetRange(m_ptr, m_bgn, m_end);
		}
		void(del)() const {
			throw_if_nonzero(DeeObject_DelRange(m_ptr, m_bgn, m_end));
		}
		range_proxy_oo const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetRange(m_ptr, m_bgn, m_end, value));
			return *this;
		}
	};
	class range_proxy_io: public proxy_base<range_proxy_io> {
	private:
		DeeObject *m_ptr;
		size_t m_bgn;
		DeeObject *m_end;

	public:
		range_proxy_io(DeeObject *ptr, size_t bgn, DeeObject *end) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_bgn(bgn)
		    , m_end(end) {}
		range_proxy_io(range_proxy_io const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_bgn(right.m_bgn)
		    , m_end(right.m_end) {}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetRangeBeginIndex(m_ptr, m_bgn, m_end);
		}
		void(del)() const {
			DREF DeeObject *begin_ob = throw_if_null(DeeInt_NewSize(m_bgn));
			int error                = DeeObject_DelRange(m_ptr, begin_ob, m_end);
			Dee_Decref(begin_ob);
			throw_if_nonzero(error);
		}
		range_proxy_io const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetRangeBeginIndex(m_ptr, m_bgn, m_end, value));
			return *this;
		}
	};
	class range_proxy_oi: public proxy_base<range_proxy_oi> {
	private:
		DeeObject *m_ptr;
		DeeObject *m_bgn;
		size_t     m_end;

	public:
		range_proxy_oi(DeeObject *ptr, DeeObject *bgn, size_t end) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_bgn(bgn)
		    , m_end(end) {}
		range_proxy_oi(range_proxy_oi const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_bgn(right.m_bgn)
		    , m_end(right.m_end) {}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetRangeEndIndex(m_ptr, m_bgn, m_end);
		}
		void(del)() const {
			DREF DeeObject *end_ob = throw_if_null(DeeInt_NewSize(m_end));
			int error              = DeeObject_DelRange(m_ptr, m_bgn, end_ob);
			Dee_Decref(end_ob);
			throw_if_nonzero(error);
		}
		range_proxy_oi const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetRangeEndIndex(m_ptr, m_bgn, m_end, value));
			return *this;
		}
	};
	class range_proxy_ii: public proxy_base<range_proxy_ii> {
	private:
		DeeObject *m_ptr;
		size_t m_bgn;
		size_t m_end;

	public:
		range_proxy_ii(DeeObject *ptr, size_t bgn, size_t end) DEE_CXX_NOTHROW
		    : m_ptr(ptr)
		    , m_bgn(bgn)
		    , m_end(end) {}
		range_proxy_ii(range_proxy_ii const &right) DEE_CXX_NOTHROW
		    : m_ptr(right.m_ptr)
		    , m_bgn(right.m_bgn)
		    , m_end(right.m_end) {}
		DREF DeeObject *(getref)() const {
			return DeeObject_GetRangeIndex(m_ptr, m_bgn, m_end);
		}
		void(del)() const {
			DREF DeeObject *begin_ob = throw_if_null(DeeInt_NewSize(m_bgn)), *end_ob;
			end_ob                   = DeeInt_NewSize(m_end);
			if unlikely(!end_ob) {
				Dee_Decref(begin_ob);
				throw_last_deemon_exception();
			}
			int error = DeeObject_DelRange(m_ptr, begin_ob, end_ob);
			Dee_Decref(end_ob);
			Dee_Decref(begin_ob);
			throw_if_nonzero(error);
		}
		range_proxy_ii const &operator=(DeeObject *value) const {
			throw_if_nonzero(DeeObject_SetRangeIndex(m_ptr, m_bgn, m_end, value));
			return *this;
		}
	};

public:
	static DeeTypeObject *(classtype)() DEE_CXX_NOTHROW {
		return &DeeObject_Type;
	}
	static bool(check)(DeeObject *__restrict UNUSED(ob)) DEE_CXX_NOTHROW {
		return true;
	}
	static bool(checkexact)(DeeObject *__restrict UNUSED(ob)) DEE_CXX_NOTHROW {
		return true;
	}

public:
	template<class T> Object(T const &init, typename detail::any_convertible<T>::available * = 0)
	    : object_base(inherit(detail::any_convertible<T>::convert(init))) {}
	template<class T> Object(std::initializer_list<T> const &init, typename detail::any_convertible<std::initializer_list<T> >::available * = 0)
	    : object_base(inherit(detail::any_convertible<std::initializer_list<T> >::convert(init))) {}
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(Object, object_base)
	attr_proxy_obj(attr)(obj_string name) const {
		return attr_proxy_obj(*this, name);
	}
	attr_proxy_str(attr)(char const *__restrict name) const {
		return attr_proxy_str(*this, name);
	}
	attr_proxy_sth(attr)(char const *__restrict name, Dee_hash_t hash) const {
		return attr_proxy_sth(*this, name, hash);
	}
	attr_proxy_snh(attr)(char const *__restrict name, size_t attrlen, Dee_hash_t hash) const {
		return attr_proxy_snh(*this, name, attrlen, hash);
	}
	Object(getattr)(obj_string name) const {
		return inherit(DeeObject_GetAttr(*this, name));
	}
	Object(getattr)(char const *__restrict name) const {
		return inherit(DeeObject_GetAttrString(*this, name));
	}
	Object(getattr)(char const *__restrict name, Dee_hash_t hash) const {
		return inherit(DeeObject_GetAttrStringHash(*this, name, hash));
	}
	Object(getattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash) const {
		return inherit(DeeObject_GetAttrStringLenHash(*this, name, namelen, hash));
	}
	bool(hasattr)(obj_string name) const {
		return throw_if_negative(DeeObject_HasAttr(*this, name)) != 0;
	}
	bool(hasattr)(char const *__restrict name) const {
		return throw_if_negative(DeeObject_HasAttrString(*this, name)) != 0;
	}
	bool(hasattr)(char const *__restrict name, Dee_hash_t hash) const {
		return throw_if_negative(DeeObject_HasAttrStringHash(*this, name, hash)) != 0;
	}
	bool(hasattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash) const {
		return throw_if_negative(DeeObject_HasAttrStringLenHash(*this, name, namelen, hash)) != 0;
	}
	bool(boundattr)(obj_string name) const {
		int result = DeeObject_BoundAttr(*this, name);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	bool(boundattr)(char const *__restrict name) const {
		int result = DeeObject_BoundAttrString(*this, name);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	bool(boundattr)(char const *__restrict name, Dee_hash_t hash) const {
		int result = DeeObject_BoundAttrStringHash(*this, name, hash);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	bool(boundattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash) const {
		int result = DeeObject_BoundAttrStringLenHash(*this, name, namelen, hash);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	void(delattr)(obj_string name) const {
		throw_if_negative(DeeObject_DelAttr(*this, name));
	}
	void(delattr)(char const *__restrict name) const {
		throw_if_negative(DeeObject_DelAttrString(*this, name));
	}
	void(delattr)(char const *__restrict name, Dee_hash_t hash) const {
		throw_if_negative(DeeObject_DelAttrStringHash(*this, name, hash));
	}
	void(delattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash) const {
		throw_if_negative(DeeObject_DelAttrStringLenHash(*this, name, namelen, hash));
	}
	void(setattr)(obj_string name, DeeObject *value) const {
		throw_if_negative(DeeObject_SetAttr(*this, name, value));
	}
	void(setattr)(char const *__restrict name, DeeObject *value) const {
		throw_if_negative(DeeObject_SetAttrString(*this, name, value));
	}
	void(setattr)(char const *__restrict name, Dee_hash_t hash, DeeObject *value) const {
		throw_if_negative(DeeObject_SetAttrStringHash(*this, name, hash, value));
	}
	void(setattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash, DeeObject *value) const {
		throw_if_negative(DeeObject_SetAttrStringLenHash(*this, name, namelen, hash, value));
	}
	Object(callattr)(obj_string name, obj_tuple args) const {
		return inherit(DeeObject_CallAttrTuple(*this, name, args));
	}
	Object(callattr)(obj_string name, obj_tuple args, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrTupleKw(*this, name, args, kw));
	}
	Object(callattr)(obj_string name, size_t argc, DeeObject *const *argv) const {
		return inherit(DeeObject_CallAttr(*this, name, argc, argv));
	}
	Object(callattr)(obj_string name, size_t argc, DeeObject *const *argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrKw(*this, name, argc, argv, kw));
	}
	Object(callattr)(obj_string name, size_t argc, DeeObject *const *__restrict argv) const {
		return inherit(DeeObject_CallAttr(*this, name, argc, (DeeObject **)argv));
	}
	Object(callattr)(obj_string name, size_t argc, DeeObject *const *__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrKw(*this, name, argc, (DeeObject **)argv, kw));
	}
	Object(callattr)(obj_string name, size_t argc, Object **__restrict argv) const {
		return inherit(DeeObject_CallAttr(*this, name, argc, (DeeObject **)argv));
	}
	Object(callattr)(obj_string name, size_t argc, Object **__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrKw(*this, name, argc, (DeeObject **)argv, kw));
	}
	Object(callattr)(obj_string name, size_t argc, Object *const *__restrict argv) const {
		return inherit(DeeObject_CallAttr(*this, name, argc, (DeeObject **)argv));
	}
	Object(callattr)(obj_string name, size_t argc, Object *const *__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrKw(*this, name, argc, (DeeObject **)argv, kw));
	}
	Object(callattr)(obj_string name, std::initializer_list<DeeObject *> const &args) const {
		return inherit(DeeObject_CallAttr(*this, name, args.size(), (DeeObject **)args.begin()));
	}
	Object(callattr)(obj_string name, std::initializer_list<DeeObject *> const &args, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrKw(*this, name, args.size(), (DeeObject **)args.begin(), kw));
	}
	Object(callattr)(char const *__restrict name, obj_tuple args) const {
		return inherit(DeeObject_CallAttrStringTuple(*this, name, (DeeObject *)args));
	}
	Object(callattr)(char const *__restrict name, obj_tuple args, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringTupleKw(*this, name, (DeeObject *)args, kw));
	}
	Object(callattr)(char const *__restrict name, size_t argc, DeeObject *const *argv) const {
		return inherit(DeeObject_CallAttrString(*this, name, argc, argv));
	}
	Object(callattr)(char const *__restrict name, size_t argc, DeeObject *const *argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringKw(*this, name, argc, argv, kw));
	}
	Object(callattr)(char const *__restrict name, size_t argc, DeeObject *const *__restrict argv) const {
		return inherit(DeeObject_CallAttrString(*this, name, argc, (DeeObject **)argv));
	}
	Object(callattr)(char const *__restrict name, size_t argc, DeeObject *const *__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringKw(*this, name, argc, (DeeObject **)argv, kw));
	}
	Object(callattr)(char const *__restrict name, size_t argc, Object **__restrict argv) const {
		return inherit(DeeObject_CallAttrString(*this, name, argc, (DeeObject **)argv));
	}
	Object(callattr)(char const *__restrict name, size_t argc, Object **__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringKw(*this, name, argc, (DeeObject **)argv, kw));
	}
	Object(callattr)(char const *__restrict name, size_t argc, Object *const *__restrict argv) const {
		return inherit(DeeObject_CallAttrString(*this, name, argc, (DeeObject **)argv));
	}
	Object(callattr)(char const *__restrict name, size_t argc, Object *const *__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringKw(*this, name, argc, (DeeObject **)argv, kw));
	}
	Object(callattr)(char const *__restrict name, std::initializer_list<DeeObject *> const &args) const {
		return inherit(DeeObject_CallAttrString(*this, name, args.size(), (DeeObject **)args.begin()));
	}
	Object(callattr)(char const *__restrict name, std::initializer_list<DeeObject *> const &args, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringKw(*this, name, args.size(), (DeeObject **)args.begin(), kw));
	}
	Object(callattr)(char const *__restrict name, Dee_hash_t hash, obj_tuple args) const {
		return inherit(DeeObject_CallAttrStringHashTuple(*this, name, hash, (DeeObject *)args));
	}
	Object(callattr)(char const *__restrict name, Dee_hash_t hash, obj_tuple args, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringHashTupleKw(*this, name, hash, (DeeObject *)args, kw));
	}
	Object(callattr)(char const *__restrict name, Dee_hash_t hash, size_t argc, DeeObject *const *argv) const {
		return inherit(DeeObject_CallAttrStringHash(*this, name, hash, argc, argv));
	}
	Object(callattr)(char const *__restrict name, Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringHashKw(*this, name, hash, argc, argv, kw));
	}
	Object(callattr)(char const *__restrict name, Dee_hash_t hash, size_t argc, DeeObject *const *__restrict argv) const {
		return inherit(DeeObject_CallAttrStringHash(*this, name, hash, argc, (DeeObject **)argv));
	}
	Object(callattr)(char const *__restrict name, Dee_hash_t hash, size_t argc, DeeObject *const *__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringHashKw(*this, name, hash, argc, (DeeObject **)argv, kw));
	}
	Object(callattr)(char const *__restrict name, Dee_hash_t hash, size_t argc, Object **__restrict argv) const {
		return inherit(DeeObject_CallAttrStringHash(*this, name, hash, argc, (DeeObject **)argv));
	}
	Object(callattr)(char const *__restrict name, Dee_hash_t hash, size_t argc, Object **__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringHashKw(*this, name, hash, argc, (DeeObject **)argv, kw));
	}
	Object(callattr)(char const *__restrict name, Dee_hash_t hash, size_t argc, Object *const *__restrict argv) const {
		return inherit(DeeObject_CallAttrStringHash(*this, name, hash, argc, (DeeObject **)argv));
	}
	Object(callattr)(char const *__restrict name, Dee_hash_t hash, size_t argc, Object *const *__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringHashKw(*this, name, hash, argc, (DeeObject **)argv, kw));
	}
	Object(callattr)(char const *__restrict name, Dee_hash_t hash, std::initializer_list<DeeObject *> const &args) const {
		return inherit(DeeObject_CallAttrStringHash(*this, name, hash, args.size(), (DeeObject **)args.begin()));
	}
	Object(callattr)(char const *__restrict name, Dee_hash_t hash, std::initializer_list<DeeObject *> const &args, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringHashKw(*this, name, hash, args.size(), (DeeObject **)args.begin(), kw));
	}
	Object(callattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash, obj_tuple args) const {
		return inherit(DeeObject_CallAttrStringLenHashTuple(*this, name, namelen, hash, (DeeObject *)args));
	}
	Object(callattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash, obj_tuple args, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringLenHashTupleKw(*this, name, namelen, hash, (DeeObject *)args, kw));
	}
	Object(callattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash, size_t argc, DeeObject *const *argv) const {
		return inherit(DeeObject_CallAttrStringLenHash(*this, name, namelen, hash, argc, argv));
	}
	Object(callattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringLenHashKw(*this, name, namelen, hash, argc, argv, kw));
	}
	Object(callattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash, size_t argc, DeeObject *const *__restrict argv) const {
		return inherit(DeeObject_CallAttrStringLenHash(*this, name, namelen, hash, argc, (DeeObject **)argv));
	}
	Object(callattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash, size_t argc, DeeObject *const *__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringLenHashKw(*this, name, namelen, hash, argc, (DeeObject **)argv, kw));
	}
	Object(callattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash, size_t argc, Object **__restrict argv) const {
		return inherit(DeeObject_CallAttrStringLenHash(*this, name, namelen, hash, argc, (DeeObject **)argv));
	}
	Object(callattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash, size_t argc, Object **__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringLenHashKw(*this, name, namelen, hash, argc, (DeeObject **)argv, kw));
	}
	Object(callattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash, size_t argc, Object *const *__restrict argv) const {
		return inherit(DeeObject_CallAttrStringLenHash(*this, name, namelen, hash, argc, (DeeObject **)argv));
	}
	Object(callattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash, size_t argc, Object *const *__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringLenHashKw(*this, name, namelen, hash, argc, (DeeObject **)argv, kw));
	}
	Object(callattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash, std::initializer_list<DeeObject *> const &args) const {
		return inherit(DeeObject_CallAttrStringLenHash(*this, name, namelen, hash, args.size(), (DeeObject **)args.begin()));
	}
	Object(callattr)(char const *__restrict name, size_t namelen, Dee_hash_t hash, std::initializer_list<DeeObject *> const &args, DeeObject *kw) const {
		return inherit(DeeObject_CallAttrStringLenHashKw(*this, name, namelen, hash, args.size(), (DeeObject **)args.begin(), kw));
	}
	item_proxy_obj(item)(DeeObject *index) const {
		return item_proxy_obj(*this, index);
	}
	item_proxy_idx(item)(size_t index) const {
		return item_proxy_idx(*this, index);
	}
	item_proxy_sth(item)(char const *__restrict name) const {
		return item_proxy_sth(*this, name);
	}
	item_proxy_sth(item)(char const *__restrict name, Dee_hash_t hash) const {
		return item_proxy_sth(*this, name, hash);
	}
	item_proxy_snh(item)(char const *__restrict name, size_t len, Dee_hash_t hash) const {
		return item_proxy_snh(*this, name, len, hash);
	}
	Object(getitem)(DeeObject *index) const {
		return inherit(DeeObject_GetItem(*this, index));
	}
	Object(getitem)(DeeObject *index, DeeObject *def) const {
		return inherit(DeeObject_GetItemDef(*this, index, def));
	}
	Object(getitem)(size_t index) const {
		return inherit(DeeObject_GetItemIndex(*this, index));
	}
	Object(getitem)(size_t index, DeeObject *def) const {
		DREF DeeObject *result, *index_ob = throw_if_null(DeeInt_NewSize(index));
		result = DeeObject_GetItemDef(*this, index_ob, def);
		Dee_Decref(index_ob);
		return inherit(result);
	}
	Object(getitem)(char const *__restrict name) const {
		return inherit(DeeObject_GetItemString(*this, name, Dee_HashStr(name)));
	}
	Object(getitem)(char const *__restrict name, DeeObject *def) const {
		return inherit(DeeObject_GetItemStringDef(*this, name, Dee_HashStr(name), def));
	}
	Object(getitem)(char const *__restrict name, Dee_hash_t hash) const {
		return inherit(DeeObject_GetItemString(*this, name, hash));
	}
	Object(getitem)(char const *__restrict name, Dee_hash_t hash, DeeObject *def) const {
		return inherit(DeeObject_GetItemStringDef(*this, name, hash, def));
	}
	Object(getitem)(char const *__restrict name, size_t len, Dee_hash_t hash) const {
		return inherit(DeeObject_GetItemStringLen(*this, name, len, hash));
	}
	Object(getitem)(char const *__restrict name, size_t len, Dee_hash_t hash, DeeObject *def) const {
		return inherit(DeeObject_GetItemStringLenDef(*this, name, len, hash, def));
	}
	bool(bounditem)(DeeObject *key_or_index, bool allow_missing = true) const {
		int result = DeeObject_BoundItem(*this, key_or_index, allow_missing);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	bool(bounditem)(size_t index, bool allow_missing = true) const {
		int result = DeeObject_BoundItemIndex(*this, index, allow_missing);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	bool(bounditem)(char const *__restrict key, bool allow_missing = true) const {
		int result = DeeObject_BoundItemString(*this, key, Dee_HashStr(key), allow_missing);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	bool(bounditem)(char const *__restrict key, Dee_hash_t hash, bool allow_missing = true) const {
		int result = DeeObject_BoundItemString(*this, key, hash, allow_missing);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	bool(bounditem)(char const *__restrict key, size_t len, Dee_hash_t hash, bool allow_missing = true) const {
		int result = DeeObject_BoundItemStringLen(*this, key, len, hash, allow_missing);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	bool(hasitem)(DeeObject *key_or_index) const {
		int result = DeeObject_HasItem(*this, key_or_index);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	bool(hasitem)(size_t index) const {
		int result = DeeObject_HasItemIndex(*this, index);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	bool(hasitem)(char const *__restrict key) const {
		int result = DeeObject_HasItemString(*this, key, Dee_HashStr(key));
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	bool(hasitem)(char const *__restrict key, Dee_hash_t hash) const {
		int result = DeeObject_HasItemString(*this, key, hash);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	bool(hasitem)(char const *__restrict key, size_t len, Dee_hash_t hash) const {
		int result = DeeObject_HasItemStringLen(*this, key, len, hash);
		if (result == -1)
			throw_last_deemon_exception();
		return result > 0;
	}
	void(delitem)(DeeObject *index) const {
		throw_if_negative(DeeObject_DelItem(*this, index));
	}
	void(delitem)(size_t index) const {
		throw_if_negative(DeeObject_DelItemIndex(*this, index));
	}
	void(delitem)(char const *__restrict name) const {
		throw_if_negative(DeeObject_DelItemString(*this, name, Dee_HashStr(name)));
	}
	void(delitem)(char const *__restrict name, Dee_hash_t hash) const {
		throw_if_negative(DeeObject_DelItemString(*this, name, hash));
	}
	void(delitem)(char const *__restrict name, size_t len, Dee_hash_t hash) const {
		throw_if_negative(DeeObject_DelItemStringLen(*this, name, len, hash));
	}
	void(setitem)(DeeObject *index, DeeObject *value) const {
		throw_if_negative(DeeObject_SetItem(*this, index, value));
	}
	void(setitem)(size_t index, DeeObject *value) const {
		throw_if_negative(DeeObject_SetItemIndex(*this, index, value));
	}
	void(setitem)(char const *__restrict name, DeeObject *value) const {
		throw_if_negative(DeeObject_SetItemString(*this, name, Dee_HashStr(name), value));
	}
	void(setitem)(char const *__restrict name, Dee_hash_t hash, DeeObject *value) const {
		throw_if_negative(DeeObject_SetItemString(*this, name, hash, value));
	}
	void(setitem)(char const *__restrict name, size_t len, Dee_hash_t hash, DeeObject *value) const {
		throw_if_negative(DeeObject_SetItemStringLen(*this, name, len, hash, value));
	}
	range_proxy_oo(range)(DeeObject *begin, DeeObject *end) const {
		return range_proxy_oo(*this, begin, end);
	}
	range_proxy_io(range)(size_t begin, DeeObject *end) const {
		return range_proxy_io(*this, begin, end);
	}
	range_proxy_oi(range)(DeeObject *begin, size_t end) const {
		return range_proxy_oi(*this, begin, end);
	}
	range_proxy_ii(range)(size_t begin, size_t end) const {
		return range_proxy_ii(*this, begin, end);
	}
	Object(getrange)(DeeObject *begin, DeeObject *end) const {
		return inherit(DeeObject_GetRange(*this, begin, end));
	}
	Object(getrange)(size_t begin, DeeObject *end) const {
		return inherit(DeeObject_GetRangeBeginIndex(*this, begin, end));
	}
	Object(getrange)(DeeObject *begin, size_t end) const {
		return inherit(DeeObject_GetRangeEndIndex(*this, begin, end));
	}
	Object(getrange)(size_t begin, size_t end) const {
		return inherit(DeeObject_GetRangeIndex(*this, begin, end));
	}
	void(delrange)(DeeObject *begin, DeeObject *end) const {
		throw_if_negative(DeeObject_DelRange(*this, begin, end));
	}
	void(delrange)(size_t begin, DeeObject *end) const {
		DREF DeeObject *begin_ob = throw_if_null(DeeInt_NewSize(begin));
		int error                = DeeObject_DelRange(*this, begin_ob, end);
		Dee_Decref(begin_ob);
		throw_if_nonzero(error);
	}
	void(delrange)(DeeObject *begin, size_t end) const {
		DREF DeeObject *end_ob = throw_if_null(DeeInt_NewSize(end));
		int error              = DeeObject_DelRange(*this, begin, end_ob);
		Dee_Decref(end_ob);
		throw_if_nonzero(error);
	}
	void(delrange)(size_t begin, size_t end) const {
		DREF DeeObject *begin_ob = throw_if_null(DeeInt_NewSize(begin));
		DREF DeeObject *end_ob   = DeeInt_NewSize(end);
		int error;
		if unlikely(!end_ob) {
			Dee_Decref(begin_ob);
			throw_last_deemon_exception();
		}
		error = DeeObject_DelRange(*this, begin_ob, end_ob);
		Dee_Decref(end_ob);
		Dee_Decref(begin_ob);
		throw_if_nonzero(error);
	}
	void(setrange)(DeeObject *begin, DeeObject *end, DeeObject *values) const {
		throw_if_nonzero(DeeObject_SetRange(*this, begin, end, values));
	}
	void(setrange)(size_t begin, DeeObject *end, DeeObject *values) const {
		throw_if_nonzero(DeeObject_SetRangeBeginIndex(*this, begin, end, values));
	}
	void(setrange)(DeeObject *begin, size_t end, DeeObject *values) const {
		throw_if_nonzero(DeeObject_SetRangeEndIndex(*this, begin, end, values));
	}
	void(setrange)(size_t begin, size_t end, DeeObject *values) const {
		throw_if_nonzero(DeeObject_SetRangeIndex(*this, begin, end, values));
	}
	Object(call)() const {
		return inherit(DeeObject_Call(*this, 0, NULL));
	}
	Object(call)(obj_tuple args) const {
		return inherit(DeeObject_CallTuple(*this, args));
	}
	Object(call)(obj_tuple args, DeeObject *kw) const {
		return inherit(DeeObject_CallTupleKw(*this, args, kw));
	}
	Object(call)(size_t argc, DeeObject *const *argv) const {
		return inherit(DeeObject_Call(*this, argc, argv));
	}
	Object(call)(size_t argc, DeeObject *const *argv, DeeObject *kw) const {
		return inherit(DeeObject_CallKw(*this, argc, argv, kw));
	}
	Object(call)(size_t argc, DeeObject *const *argv) const {
		return inherit(DeeObject_Call(*this, argc, (DeeObject **)argv));
	}
	Object(call)(size_t argc, DeeObject *const *argv, DeeObject *kw) const {
		return inherit(DeeObject_CallKw(*this, argc, (DeeObject **)argv, kw));
	}
	Object(call)(size_t argc, Object **argv) const {
		return inherit(DeeObject_Call(*this, argc, (DeeObject **)argv));
	}
	Object(call)(size_t argc, Object **argv, DeeObject *kw) const {
		return inherit(DeeObject_CallKw(*this, argc, (DeeObject **)argv, kw));
	}
	Object(call)(size_t argc, Object *const *argv) const {
		return inherit(DeeObject_Call(*this, argc, (DeeObject **)argv));
	}
	Object(call)(size_t argc, Object *const *argv, DeeObject *kw) const {
		return inherit(DeeObject_CallKw(*this, argc, (DeeObject **)argv, kw));
	}
	Object(call)(std::initializer_list<DeeObject *> const &args) const {
		return inherit(DeeObject_Call(*this, args.size(), (DeeObject **)args.begin()));
	}
	Object(call)(std::initializer_list<DeeObject *> const &args, DeeObject *kw) const {
		return inherit(DeeObject_CallKw(*this, args.size(), (DeeObject **)args.begin(), kw));
	}
	Object(thiscall)(DeeObject *this_arg) const {
		return inherit(DeeObject_ThisCall(*this, this_arg, 0, NULL));
	}
	Object(thiscall)(DeeObject *this_arg, obj_tuple args) const {
		return inherit(DeeObject_ThisCallTuple(*this, this_arg, args));
	}
	Object(thiscall)(DeeObject *this_arg, obj_tuple args, DeeObject *kw) const {
		return inherit(DeeObject_ThisCallTupleKw(*this, this_arg, args, kw));
	}
	Object(thiscall)(DeeObject *this_arg, size_t argc, DeeObject *const *argv) const {
		return inherit(DeeObject_ThisCall(*this, this_arg, argc, argv));
	}
	Object(thiscall)(DeeObject *this_arg, size_t argc, DeeObject *const *argv, DeeObject *kw) const {
		return inherit(DeeObject_ThisCallKw(*this, this_arg, argc, argv, kw));
	}
	Object(thiscall)(DeeObject *this_arg, size_t argc, DeeObject *const *__restrict argv) const {
		return inherit(DeeObject_ThisCall(*this, this_arg, argc, (DeeObject **)argv));
	}
	Object(thiscall)(DeeObject *this_arg, size_t argc, DeeObject *const *__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_ThisCallKw(*this, this_arg, argc, (DeeObject **)argv, kw));
	}
	Object(thiscall)(DeeObject *this_arg, size_t argc, Object **__restrict argv) const {
		return inherit(DeeObject_ThisCall(*this, this_arg, argc, (DeeObject **)argv));
	}
	Object(thiscall)(DeeObject *this_arg, size_t argc, Object **__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_ThisCallKw(*this, this_arg, argc, (DeeObject **)argv, kw));
	}
	Object(thiscall)(DeeObject *this_arg, size_t argc, Object *const *__restrict argv) const {
		return inherit(DeeObject_ThisCall(*this, this_arg, argc, (DeeObject **)argv));
	}
	Object(thiscall)(DeeObject *this_arg, size_t argc, Object *const *__restrict argv, DeeObject *kw) const {
		return inherit(DeeObject_ThisCallKw(*this, this_arg, argc, (DeeObject **)argv, kw));
	}
	Object(thiscall)(DeeObject *this_arg, std::initializer_list<DeeObject *> const &args) const {
		return inherit(DeeObject_ThisCall(*this, this_arg, args.size(), (DeeObject **)args.begin()));
	}
	Object(thiscall)(DeeObject *this_arg, std::initializer_list<DeeObject *> const &args, DeeObject *kw) const {
		return inherit(DeeObject_ThisCallKw(*this, this_arg, args.size(), (DeeObject **)args.begin(), kw));
	}
	string(str)() const;
	string(repr)() const;
	deemon::Super(super)() const;
	deemon::Super(super)(DeeTypeObject *__restrict super_type) const;
	template<class T = Object> deemon::Super(super)() const;
	deemon::Type<Object>(type)() const;
	deemon::Type<Object>(class_)() const;
	int_(int_)() const;
	bool(bool_)() const {
		return throw_if_negative(DeeObject_Bool(*this)) != 0;
	}
	WUNUSED Dee_hash_t(hash)() const DEE_CXX_NOTHROW {
		return DeeObject_Hash(*this);
	}
	bool(equal)(DeeObject *other) const {
		return throw_if_negative(DeeObject_CompareEq(*this, other)) != 0;
	}
	bool(equal)(DeeObject *other, DeeObject *key) const {
		return throw_if_negative(DeeObject_CompareKeyEq(*this, other, key)) != 0;
	}
	bool(nonequal)(DeeObject *other) const {
		return throw_if_negative(DeeObject_CompareNe(*this, other)) != 0;
	}
	bool(nonequal)(DeeObject *other, DeeObject *key) const {
		return throw_if_negative(DeeObject_CompareKeyEq(*this, other, key)) == 0;
	}
	size_t(size)() const {
		return throw_if_minusone(DeeObject_Size(*this));
	}
	bool(contains)(DeeObject *elem) const {
		return throw_if_negative(DeeObject_Contains(*this, elem)) != 0;
	}
	Object(sizeobj)() const {
		return inherit(DeeObject_SizeObject(*this));
	}
	Object(containsobj)(DeeObject *elem) const {
		return inherit(DeeObject_ContainsObject(*this, elem));
	}
	Object const &(assign)(DeeObject *some_object) const {
		throw_if_nonzero(DeeObject_Assign(*this, some_object));
		return *this;
	}
	Object const &(moveassign)(DeeObject *some_object) const {
		throw_if_nonzero(DeeObject_MoveAssign(*this, some_object));
		return *this;
	}
	void(enter)() const {
		throw_if_nonzero(DeeObject_Enter(*this));
	}
	void(leave)() const {
		throw_if_nonzero(DeeObject_Leave(*this));
	}
	ATTR_NORETURN void(throw_)() const {
		DeeError_Throw(*this);
		throw_last_deemon_exception();
	}
	void(unpack)(size_t objc, DREF DeeObject **__restrict objv) const {
		throw_if_nonzero(DeeObject_Unpack(*this, objc, objv));
	}
	//  void unpack(size_t objc, object **__restrict objv) const { throw_if_nonzero(DeeObject_Unpack(*this, objc, (DeeObject **)objv)); }
	typedef detail::cxx_iterator<Object> iterator;
	WUNUSED iterator(begin)() const {
		return inherit(DeeObject_IterSelf(*this));
	}
	WUNUSED iterator(end)() const {
		return iterator();
	}
	inline deemon::Iterator<Object>(iter)() const;
	template<class T = Object> inline deemon::Iterator<T>(iter)() const;

	Object(next)() const {
		DREF DeeObject *result = throw_if_null(DeeObject_IterNext(*this));
		if (result == ITER_DONE)
			result = NULL;
		return inherit(maybenull(result));
	}
#undef print
	size_t(print)(Dee_formatprinter_t printer, void *arg) const {
		return throw_if_negative(DeeObject_Print(*this, printer, arg));
	}
	size_t(print)(Dee_formatprinter_t printer, void *arg, DeeObject *format_str) const {
		return throw_if_negative(DeeObject_PrintFormat(*this, printer, arg, format_str));
	}
	size_t(print)(Dee_formatprinter_t printer, void *arg, /*utf-8*/ char const *__restrict format_str) const {
		return throw_if_negative(DeeObject_PrintFormatString(*this, printer, arg, format_str, strlen(format_str)));
	}
	size_t(print)(Dee_formatprinter_t printer, void *arg, /*utf-8*/ char const *__restrict format_str, size_t format_len) const {
		return throw_if_negative(DeeObject_PrintFormatString(*this, printer, arg, format_str, format_len));
	}
	size_t(printrepr)(Dee_formatprinter_t printer, void *arg) const {
		return throw_if_negative(DeeObject_PrintRepr(*this, printer, arg));
	}
	Object(inv)() const {
		return inherit(DeeObject_Inv(*this));
	}
	Object(pos)() const {
		return inherit(DeeObject_Pos(*this));
	}
	Object(neg)() const {
		return inherit(DeeObject_Neg(*this));
	}
	Object(add)(int8_t right) const {
		return inherit(DeeObject_AddS8(*this, right));
	}
	Object(add)(uint32_t right) const {
		return inherit(DeeObject_AddInt(*this, right));
	}
	Object(add)(DeeObject *right) const {
		return inherit(DeeObject_Add(*this, right));
	}
	Object(sub)(int8_t right) const {
		return inherit(DeeObject_SubS8(*this, right));
	}
	Object(sub)(uint32_t right) const {
		return inherit(DeeObject_SubInt(*this, right));
	}
	Object(sub)(DeeObject *right) const {
		return inherit(DeeObject_Sub(*this, right));
	}
	Object(mul)(int8_t right) const {
		return inherit(DeeObject_MulInt(*this, right));
	}
	Object(mul)(DeeObject *right) const {
		return inherit(DeeObject_Mul(*this, right));
	}
	Object(div)(int8_t right) const {
		return inherit(DeeObject_DivInt(*this, right));
	}
	Object(div)(DeeObject *right) const {
		return inherit(DeeObject_Div(*this, right));
	}
	Object(mod)(int8_t right) const {
		return inherit(DeeObject_ModInt(*this, right));
	}
	Object(mod)(DeeObject *right) const {
		return inherit(DeeObject_Mod(*this, right));
	}
	Object(shl)(uint8_t right) const {
		return inherit(DeeObject_ShlInt(*this, right));
	}
	Object(shl)(DeeObject *right) const {
		return inherit(DeeObject_Shl(*this, right));
	}
	Object(shr)(uint8_t right) const {
		return inherit(DeeObject_ShrInt(*this, right));
	}
	Object(shr)(DeeObject *right) const {
		return inherit(DeeObject_Shr(*this, right));
	}
	Object(and_)(uint32_t right) const {
		return inherit(DeeObject_AndInt(*this, right));
	}
	Object(and_)(DeeObject *right) const {
		return inherit(DeeObject_And(*this, right));
	}
	Object(or_)(uint32_t right) const {
		return inherit(DeeObject_OrInt(*this, right));
	}
	Object(or_)(DeeObject *right) const {
		return inherit(DeeObject_Or(*this, right));
	}
	Object(xor_)(uint32_t right) const {
		return inherit(DeeObject_XorInt(*this, right));
	}
	Object(xor_)(DeeObject *right) const {
		return inherit(DeeObject_Xor(*this, right));
	}
	Object(pow)(DeeObject *right) const {
		return inherit(DeeObject_Pow(*this, right));
	}
	Object &(inplace_add)(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceAddS8(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_add)(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceAddInt(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_add)(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceAdd(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_sub)(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceSubS8(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_sub)(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceSubInt(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_sub)(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceSub(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_mul)(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceMulInt(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_mul)(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceMul(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_div)(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceDivInt(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_div)(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceDiv(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_mod)(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceModInt(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_mod)(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceMod(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_shl)(uint8_t right) {
		throw_if_nonzero(DeeObject_InplaceShlInt(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_shl)(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceShl(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_shr)(uint8_t right) {
		throw_if_nonzero(DeeObject_InplaceShrInt(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_shr)(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceShr(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_and)(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceAndInt(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_and)(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceAnd(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_or)(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceOrInt(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_or)(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceOr(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_xor)(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceXorInt(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_xor)(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceXor(&this->m_ptr, right));
		return *this;
	}
	Object &(inplace_pow)(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplacePow(&this->m_ptr, right));
		return *this;
	}

	/* Generic operator invocation. */
	Object(invoke_operator)(uint16_t name, obj_tuple args) const {
		return inherit(DeeObject_InvokeOperatorTuple(*this, name, (DeeObject *)args));
	}
	Object(invoke_operator)(uint16_t name, size_t argc, DeeObject *const *argv) const {
		return inherit(DeeObject_InvokeOperator(*this, name, argc, argv));
	}
	Object(invoke_operator)(uint16_t name, size_t argc, Object *__restrict argv) const {
		return inherit(DeeObject_InvokeOperator(*this, name, argc, (DeeObject **)argv));
	}
	Object(invoke_operator)(uint16_t name, std::initializer_list<DeeObject *> const &args) const {
		return inherit(DeeObject_InvokeOperator(*this, name, args.size(), (DeeObject **)args.begin()));
	}
	Object(invoke_inplace_operator)(uint16_t name, obj_tuple args) {
		return inherit(DeeObject_PInvokeOperatorTuple(&this->m_ptr, name, (DeeObject *)args));
	}
	Object(invoke_inplace_operator)(uint16_t name, size_t argc, DeeObject *const *argv) {
		return inherit(DeeObject_PInvokeOperator(&this->m_ptr, name, argc, argv));
	}
	Object(invoke_inplace_operator)(uint16_t name, size_t argc, Object *__restrict argv) {
		return inherit(DeeObject_PInvokeOperator(&this->m_ptr, name, argc, (DeeObject **)argv));
	}
	Object(invoke_inplace_operator)(uint16_t name, std::initializer_list<DeeObject *> const &args) {
		return inherit(DeeObject_PInvokeOperator(&this->m_ptr, name, args.size(), (DeeObject **)args.begin()));
	}

	/* Operator integration */
	WUNUSED item_proxy_obj operator[](DeeObject *index) const {
		return item_proxy_obj(*this, index);
	}
	WUNUSED item_proxy_idx operator[](int index) const {
		return item_proxy_idx(*this, (size_t)(unsigned int)index);
	}
	WUNUSED item_proxy_idx operator[](unsigned int index) const {
		return item_proxy_idx(*this, (size_t)index);
	}
	WUNUSED item_proxy_idx operator[](long index) const {
		return item_proxy_idx(*this, (size_t)(unsigned long)index);
	}
	WUNUSED item_proxy_idx operator[](unsigned long index) const {
		return item_proxy_idx(*this, (size_t)index);
	}
	WUNUSED item_proxy_sth operator[](char const *__restrict name) const {
		return item_proxy_sth(*this, name);
	}
	WUNUSED Object operator()(obj_tuple args) const {
		return inherit(DeeObject_CallTuple(*this, args));
	}
	WUNUSED Object operator()(std::initializer_list<DeeObject *> const &args) const {
		return inherit(DeeObject_Call(*this, args.size(), (DeeObject **)args.begin()));
	}
	WUNUSED operator bool() const {
		return throw_if_negative(DeeObject_Bool(*this)) != 0;
	}
	WUNUSED bool operator!() const {
		return throw_if_negative(DeeObject_Bool(*this)) == 0;
	}
	WUNUSED Object operator==(DeeObject *other) const {
		return DeeObject_CompareEqObject(*this, other);
	}
	WUNUSED Object operator!=(DeeObject *other) const {
		return DeeObject_CompareNeObject(*this, other);
	}
	WUNUSED Object operator<(DeeObject *other) const {
		return DeeObject_CompareLoObject(*this, other);
	}
	WUNUSED Object operator<=(DeeObject *other) const {
		return DeeObject_CompareLeObject(*this, other);
	}
	WUNUSED Object operator>(DeeObject *other) const {
		return DeeObject_CompareGrObject(*this, other);
	}
	WUNUSED Object operator>=(DeeObject *other) const {
		return DeeObject_CompareGeObject(*this, other);
	}
	WUNUSED Object operator~() const {
		return inherit(DeeObject_Inv(*this));
	}
	WUNUSED Object operator+() const {
		return inherit(DeeObject_Pos(*this));
	}
	WUNUSED Object operator-() const {
		return inherit(DeeObject_Neg(*this));
	}
	WUNUSED Object operator+(int8_t right) const {
		return inherit(DeeObject_AddS8(*this, right));
	}
	WUNUSED Object operator+(uint32_t right) const {
		return inherit(DeeObject_AddInt(*this, right));
	}
	WUNUSED Object operator+(DeeObject *right) const {
		return inherit(DeeObject_Add(*this, right));
	}
	WUNUSED Object operator-(int8_t right) const {
		return inherit(DeeObject_SubS8(*this, right));
	}
	WUNUSED Object operator-(uint32_t right) const {
		return inherit(DeeObject_SubInt(*this, right));
	}
	WUNUSED Object operator-(DeeObject *right) const {
		return inherit(DeeObject_Sub(*this, right));
	}
	WUNUSED Object operator*(int8_t right) const {
		return inherit(DeeObject_MulInt(*this, right));
	}
	WUNUSED Object operator*(DeeObject *right) const {
		return inherit(DeeObject_Mul(*this, right));
	}
	WUNUSED Object operator/(int8_t right) const {
		return inherit(DeeObject_DivInt(*this, right));
	}
	WUNUSED Object operator/(DeeObject *right) const {
		return inherit(DeeObject_Div(*this, right));
	}
	WUNUSED Object operator%(int8_t right) const {
		return inherit(DeeObject_ModInt(*this, right));
	}
	WUNUSED Object operator%(DeeObject *right) const {
		return inherit(DeeObject_Mod(*this, right));
	}
	WUNUSED Object operator<<(uint8_t right) const {
		return inherit(DeeObject_ShlInt(*this, right));
	}
	WUNUSED Object operator<<(DeeObject *right) const {
		return inherit(DeeObject_Shl(*this, right));
	}
	WUNUSED Object operator>>(uint8_t right) const {
		return inherit(DeeObject_ShrInt(*this, right));
	}
	WUNUSED Object operator>>(DeeObject *right) const {
		return inherit(DeeObject_Shr(*this, right));
	}
	WUNUSED Object operator&(uint32_t right) const {
		return inherit(DeeObject_AndInt(*this, right));
	}
	WUNUSED Object operator&(DeeObject *right) const {
		return inherit(DeeObject_And(*this, right));
	}
	WUNUSED Object operator|(uint32_t right) const {
		return inherit(DeeObject_OrInt(*this, right));
	}
	WUNUSED Object operator|(DeeObject *right) const {
		return inherit(DeeObject_Or(*this, right));
	}
	WUNUSED Object operator^(uint32_t right) const {
		return inherit(DeeObject_XorInt(*this, right));
	}
	WUNUSED Object operator^(DeeObject *right) const {
		return inherit(DeeObject_Xor(*this, right));
	}
	Object &operator++() {
		throw_if_nonzero(DeeObject_Inc(&this->m_ptr));
		return *this;
	}
	Object &operator--() {
		throw_if_nonzero(DeeObject_Dec(&this->m_ptr));
		return *this;
	}
	WUNUSED Object operator++(int) {
		DREF DeeObject *result = throw_if_null(DeeObject_Copy(*this));
		int error              = DeeObject_Inc(&this->m_ptr);
		if unlikely(error) {
			Dee_Decref(result);
			throw_last_deemon_exception();
		}
		return inherit(nonnull(result));
	}
	WUNUSED Object operator--(int) {
		DREF DeeObject *result = throw_if_null(DeeObject_Copy(*this));
		int error              = DeeObject_Dec(&this->m_ptr);
		if unlikely(error) {
			Dee_Decref(result);
			throw_last_deemon_exception();
		}
		return inherit(nonnull(result));
	}
	Object &operator+=(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceAddS8(&this->m_ptr, right));
		return *this;
	}
	Object &operator+=(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceAddInt(&this->m_ptr, right));
		return *this;
	}
	Object &operator+=(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceAdd(&this->m_ptr, right));
		return *this;
	}
	Object &operator-=(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceSubS8(&this->m_ptr, right));
		return *this;
	}
	Object &operator-=(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceSubInt(&this->m_ptr, right));
		return *this;
	}
	Object &operator-=(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceSub(&this->m_ptr, right));
		return *this;
	}
	Object &operator*=(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceMulInt(&this->m_ptr, right));
		return *this;
	}
	Object &operator*=(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceMul(&this->m_ptr, right));
		return *this;
	}
	Object &operator/=(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceDivInt(&this->m_ptr, right));
		return *this;
	}
	Object &operator/=(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceDiv(&this->m_ptr, right));
		return *this;
	}
	Object &operator%=(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceModInt(&this->m_ptr, right));
		return *this;
	}
	Object &operator%=(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceMod(&this->m_ptr, right));
		return *this;
	}
	Object &operator<<=(uint8_t right) {
		throw_if_nonzero(DeeObject_InplaceShlInt(&this->m_ptr, right));
		return *this;
	}
	Object &operator<<=(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceShl(&this->m_ptr, right));
		return *this;
	}
	Object &operator>>=(uint8_t right) {
		throw_if_nonzero(DeeObject_InplaceShrInt(&this->m_ptr, right));
		return *this;
	}
	Object &operator>>=(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceShr(&this->m_ptr, right));
		return *this;
	}
	Object &operator&=(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceAndInt(&this->m_ptr, right));
		return *this;
	}
	Object &operator&=(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceAnd(&this->m_ptr, right));
		return *this;
	}
	Object &operator|=(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceOrInt(&this->m_ptr, right));
		return *this;
	}
	Object &operator|=(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceOr(&this->m_ptr, right));
		return *this;
	}
	Object &operator^=(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceXorInt(&this->m_ptr, right));
		return *this;
	}
	Object &operator^=(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceXor(&this->m_ptr, right));
		return *this;
	}

	/* High-level interface for up-casting object types. */
	template<class T> WUNUSED T &as(typename std::enable_if<std::is_base_of<Object, T>::value, int>::type * = 0) DEE_CXX_NOTHROW {
		return *(T *)this;
	}
	template<class T> WUNUSED T const &as(typename std::enable_if<std::is_base_of<Object, T>::value, int>::type * = 0) const DEE_CXX_NOTHROW {
		return *(T *)this;
	}
	/* Same as `as()', but throw an exception if the cast cannot be performed. */
	template<class T> WUNUSED T &as_chk(typename std::enable_if<std::is_base_of<Object, T>::value, int>::type * = 0) {
		if (!T::check(m_ptr)) {
			DeeObject_TypeAssertFailed(m_ptr, T::classtype());
			throw_last_deemon_exception();
		}
		return *(T *)this;
	}
	template<class T> WUNUSED T const &as_chk(typename std::enable_if<std::is_base_of<Object, T>::value, int>::type * = 0) const {
		if (!T::check(m_ptr)) {
			DeeObject_TypeAssertFailed(m_ptr, T::classtype());
			throw_last_deemon_exception();
		}
		return *(T *)this;
	}

	/* Integer conversion */
	Object const &(getval)(char &value) const {
		throw_if_nonzero(DeeObject_AsChar(*this, &value));
		return *this;
	}
	Object const &(getval)(signed char &value) const {
		throw_if_nonzero(DeeObject_AsSChar(*this, &value));
		return *this;
	}
	Object const &(getval)(unsigned char &value) const {
		throw_if_nonzero(DeeObject_AsUChar(*this, &value));
		return *this;
	}
	Object const &(getval)(short &value) const {
		throw_if_nonzero(DeeObject_AsShort(*this, &value));
		return *this;
	}
	Object const &(getval)(unsigned short &value) const {
		throw_if_nonzero(DeeObject_AsUShort(*this, &value));
		return *this;
	}
	Object const &(getval)(int &value) const {
		throw_if_nonzero(DeeObject_AsInt(*this, &value));
		return *this;
	}
	Object const &(getval)(unsigned int &value) const {
		throw_if_nonzero(DeeObject_AsUInt(*this, &value));
		return *this;
	}
	Object const &(getval)(long &value) const {
		throw_if_nonzero(DeeObject_AsLong(*this, &value));
		return *this;
	}
	Object const &(getval)(unsigned long &value) const {
		throw_if_nonzero(DeeObject_AsULong(*this, &value));
		return *this;
	}
#ifdef __COMPILER_HAVE_LONGLONG
	Object const &(getval)(__LONGLONG &value) const {
		throw_if_nonzero(DeeObject_AsLLong(*this, &value));
		return *this;
	}
	Object const &(getval)(__ULONGLONG &value) const {
		throw_if_nonzero(DeeObject_AsULLong(*this, &value));
		return *this;
	}
#endif /* __COMPILER_HAVE_LONGLONG */
	Object const &(getval)(Dee_int128_t &value) const {
		throw_if_nonzero(DeeObject_AsInt128(*this, &value));
		return *this;
	}
	Object const &(getval)(Dee_uint128_t &value) const {
		throw_if_nonzero(DeeObject_AsUInt128(*this, &value));
		return *this;
	}
	Object const &(getval)(float &value) const {
		double temp;
		getval(temp);
		value = (float)temp;
		return *this;
	}
	Object const &(getval)(double &value) const {
		throw_if_nonzero(DeeObject_AsDouble(*this, &value));
		return *this;
	}
#ifdef __COMPILER_HAVE_LONGDOUBLE
	Object const &(getval)(long double &value) const {
		double temp;
		getval(temp);
		value = (long double)temp;
		return *this;
	}
#endif /* __COMPILER_HAVE_LONGDOUBLE */

	/* Helper functions to explicitly convert an object to an integral value. */
	WUNUSED short(asshort)() const {
		short result;
		getval(result);
		return result;
	}
	WUNUSED unsigned short(asushort)() const {
		unsigned short result;
		getval(result);
		return result;
	}
	WUNUSED int(asint)() const {
		int result;
		getval(result);
		return result;
	}
	WUNUSED unsigned int(asuint)() const {
		unsigned int result;
		getval(result);
		return result;
	}
	WUNUSED long(aslong)() const {
		long result;
		getval(result);
		return result;
	}
	WUNUSED unsigned long(asulong)() const {
		unsigned long result;
		getval(result);
		return result;
	}
#ifdef __COMPILER_HAVE_LONGLONG
	WUNUSED __LONGLONG(asllong)() const {
		__LONGLONG result;
		getval(result);
		return result;
	}
	WUNUSED __ULONGLONG(asullong)() const {
		__ULONGLONG result;
		getval(result);
		return result;
	}
#endif /* __COMPILER_HAVE_LONGLONG */

	WUNUSED int8_t(ass8)() const {
		int8_t result;
		getval(result);
		return result;
	}
	WUNUSED int16_t(ass16)() const {
		int16_t result;
		getval(result);
		return result;
	}
	WUNUSED int32_t(ass32)() const {
		int32_t result;
		getval(result);
		return result;
	}
	WUNUSED int64_t(ass64)() const {
		int64_t result;
		getval(result);
		return result;
	}
	WUNUSED Dee_int128_t(ass128)() const {
		Dee_int128_t result;
		getval(result);
		return result;
	}
	WUNUSED uint8_t(asu8)() const {
		uint8_t result;
		getval(result);
		return result;
	}
	WUNUSED uint16_t(asu16)() const {
		uint16_t result;
		getval(result);
		return result;
	}
	WUNUSED uint32_t(asu32)() const {
		uint32_t result;
		getval(result);
		return result;
	}
	WUNUSED uint64_t(asu64)() const {
		uint64_t result;
		getval(result);
		return result;
	}
	WUNUSED Dee_uint128_t(asu128)() const {
		Dee_uint128_t result;
		getval(result);
		return result;
	}
	WUNUSED size_t(assize)() const {
		size_t result;
		getval(result);
		return result;
	}
	WUNUSED Dee_ssize_t(asssize)() const {
		Dee_ssize_t result;
		getval(result);
		return result;
	}
	WUNUSED float(asfloat)() const {
		float result;
		getval(result);
		return result;
	}
	WUNUSED double(asdouble)() const {
		double result;
		getval(result);
		return result;
	}
#ifdef __COMPILER_HAVE_LONGDOUBLE
	WUNUSED long double(asldouble)() const {
		long double result;
		getval(result);
		return result;
	}
#endif /* __COMPILER_HAVE_LONGDOUBLE */

	/* Integer conversion operators */
	explicit WUNUSED operator char() const {
		char result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator signed char() const {
		signed char result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator unsigned char() const {
		unsigned char result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator short() const {
		short result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator unsigned short() const {
		unsigned short result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator int() const {
		int result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator unsigned int() const {
		unsigned int result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator long() const {
		long result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator unsigned long() const {
		unsigned long result;
		getval(result);
		return result;
	}
#ifdef __COMPILER_HAVE_LONGLONG
	explicit WUNUSED operator __LONGLONG() const {
		__LONGLONG result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator __ULONGLONG() const {
		__ULONGLONG result;
		getval(result);
		return result;
	}
#endif /* __COMPILER_HAVE_LONGLONG */
	explicit WUNUSED operator Dee_int128_t() const {
		Dee_int128_t result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator Dee_uint128_t() const {
		Dee_uint128_t result;
		getval(result);
		return result;
	}
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
#endif /* __COMPILER_HAVE_LONGLONG */
};


WUNUSED Object WeakRef::trylock() const DEE_CXX_NOTHROW {
	return inherit(maybenull(trylockref()));
}
WUNUSED Object WeakRef::lock() const {
	return inherit(nonnull(lockref()));
}
WUNUSED Object WeakRef::lock(DeeObject *defl) const DEE_CXX_NOTHROW {
	return inherit(nonnull(lockref(defl)));
}


class none: public Object {
public:
	static DeeTypeObject *(classtype)() DEE_CXX_NOTHROW {
		return &DeeNone_Type;
	}
	static bool(check)(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeNone_Check(ob);
	}
	static bool(checkexact)(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeNone_CheckExact(ob);
	}

public:
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(none, Object)
	none() DEE_CXX_NOTHROW: Object(nonnull(Dee_None)) {}
};


class bool_: public Object {
public:
	static DeeTypeObject *(classtype)() DEE_CXX_NOTHROW {
		return &DeeBool_Type;
	}
	static bool(check)(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeBool_Check(ob);
	}
	static bool(checkexact)(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeBool_CheckExact(ob);
	}

public:
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(bool_, Object)
	bool_() DEE_CXX_NOTHROW: Object(nonnull(Dee_False)) {}
	bool_(bool value) DEE_CXX_NOTHROW: Object(nonnull(DeeBool_For(value))) {}
	static WUNUSED bool_(true_)() DEE_CXX_NOTHROW {
		return nonnull(Dee_True);
	}
	static WUNUSED bool_(false_)() DEE_CXX_NOTHROW {
		return nonnull(Dee_False);
	}
#ifndef __OPTIMIZE_SIZE__
	//bool bool_() const DEE_CXX_NOTHROW { return DeeBool_IsTrue(this->ptr()); }
	WUNUSED operator bool() const {
		return (likely(DeeBool_Check(this->ptr())))
		       ? DeeBool_IsTrue(this->ptr())
		       : Object::operator bool();
	}
	WUNUSED bool operator!() const {
		return (likely(DeeBool_Check(this->ptr())))
		       ? !DeeBool_IsTrue(this->ptr())
		       : Object::operator!();
	}
#endif /* !__OPTIMIZE_SIZE__ */
};

inline ATTR_NORETURN ATTR_COLD void (DCALL throw_last_deemon_exception)(void) {
	/* XXX: Exception sub-classes? */
	throw deemon::exception();
}


DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_OBJECT_H */
