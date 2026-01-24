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
#ifndef GUARD_DEEMON_CXX_OBJECT_H
#define GUARD_DEEMON_CXX_OBJECT_H 1

#include "../api.h"
#include "api.h"

#include <__stdcxx.h>

#include <hybrid/compiler.h>

#include "../error.h"
#include "../none.h"
#include "../object.h"
#include "../super.h"
#include "../system-features.h" /* strlen(), wcslen() */
#include "../tuple.h"           /* DeeObject_CallAttrStringTuple -> DeeTuple_ELEM */

#include <exception>
#include <initializer_list>
#include <stdbool.h>        /* bool, false, true */
#include <stddef.h>         /* NULL, size_t */
#include <stdint.h>         /* intN_t, uintN_t */
#include <type_traits>

DEE_CXX_BEGIN

/* C++ wrappers for deemon types */
class Object;
class string;
class Bytes;
class bool_;
class int_;
class float_;
class Type;
template<class T = Object> class Iterator;
template<class T = Object> class Sequence;
template<class ...Types> class _AbstractTuple;
template<class ...Types> class Tuple;

/* C++ wrappers for deemon object holders */
template<class T = Object> class Ref;
template<class T = Object> class WeakRef;



FORCELOCAL WUNUSED DREF DeeObject *(incref)(DeeObject *__restrict obj) DEE_CXX_NOTHROW {
	Dee_Incref(obj);
	return obj;
}

FORCELOCAL WUNUSED DREF DeeObject *(xincref)(DeeObject *obj) DEE_CXX_NOTHROW {
	Dee_XIncref(obj);
	return obj;
}

FORCELOCAL void (decref)(DREF DeeObject *__restrict obj) DEE_CXX_NOTHROW {
	Dee_Decref(obj);
}

FORCELOCAL void (decref_likely)(DREF DeeObject *__restrict obj) DEE_CXX_NOTHROW {
	Dee_Decref_likely(obj);
}

FORCELOCAL void (decref_unlikely)(DREF DeeObject *__restrict obj) DEE_CXX_NOTHROW {
	Dee_Decref_unlikely(obj);
}

FORCELOCAL void (decref_nokill)(DREF DeeObject *__restrict obj) DEE_CXX_NOTHROW {
	Dee_DecrefNokill(obj);
}

FORCELOCAL void (decref_dokill)(DREF DeeObject *__restrict obj) DEE_CXX_NOTHROW {
	Dee_DecrefDokill(obj);
}

FORCELOCAL bool (decref_ifone)(DREF DeeObject *__restrict obj) DEE_CXX_NOTHROW {
	return Dee_DecrefIfOne(obj);
}

FORCELOCAL bool (decref_ifnotone)(DREF DeeObject *__restrict obj) DEE_CXX_NOTHROW {
	return Dee_DecrefIfNotOne(obj);
}

FORCELOCAL void (xdecref)(DREF DeeObject *obj) DEE_CXX_NOTHROW {
	Dee_XDecref(obj);
}

FORCELOCAL void (xdecref_nokill)(DREF DeeObject *obj) DEE_CXX_NOTHROW {
	Dee_XDecrefNokill(obj);
}
#ifdef CONFIG_TRACE_REFCHANGES
FORCELOCAL WUNUSED DREF DeeObject *(incref_traced)(DeeObject *__restrict obj, char const *file, int line) DEE_CXX_NOTHROW {
	Dee_Incref_traced(obj, file, line);
	return obj;
}

FORCELOCAL WUNUSED DREF DeeObject *(xincref_traced)(DeeObject *obj, char const *file, int line) DEE_CXX_NOTHROW {
	if (obj)
		Dee_Incref_traced(obj, file, line);
	return obj;
}

FORCELOCAL DeeObject *(decref_traced)(DREF DeeObject *__restrict obj, char const *file, int line) DEE_CXX_NOTHROW {
	Dee_Decref_traced(obj, file, line);
	return obj;
}

FORCELOCAL DeeObject *(decref_likely_traced)(DREF DeeObject *__restrict obj, char const *file, int line) DEE_CXX_NOTHROW {
	Dee_Decref_likely_traced(obj, file, line);
	return obj;
}

FORCELOCAL DeeObject *(decref_unlikely_traced)(DREF DeeObject *__restrict obj, char const *file, int line) DEE_CXX_NOTHROW {
	Dee_Decref_unlikely_traced(obj, file, line);
	return obj;
}

FORCELOCAL DeeObject *(decref_nokill_traced)(DREF DeeObject *__restrict obj, char const *file, int line) DEE_CXX_NOTHROW {
	Dee_DecrefNokill_traced(obj, file, line);
	return obj;
}

FORCELOCAL void (decref_dokill_traced)(DREF DeeObject *__restrict obj, char const *file, int line) DEE_CXX_NOTHROW {
	Dee_DecrefDokill_traced(obj, file, line);
}

FORCELOCAL bool (decref_ifone_traced)(DREF DeeObject *__restrict obj, char const *file, int line) DEE_CXX_NOTHROW {
	return Dee_DecrefIfOne_traced(obj, file, line);
}

FORCELOCAL bool (decref_ifnotone_traced)(DREF DeeObject *__restrict obj, char const *file, int line) DEE_CXX_NOTHROW {
	return Dee_DecrefIfNotOne_traced(obj, file, line);
}

FORCELOCAL void (xdecref_traced)(DREF DeeObject *obj, char const *file, int line) DEE_CXX_NOTHROW {
	if (obj)
		Dee_Decref_traced(obj, file, line);
}

FORCELOCAL void (xdecref_nokill_traced)(DREF DeeObject *obj, char const *file, int line) DEE_CXX_NOTHROW {
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

/* Throws the latest deemon error as a C++ exception. */
LOCAL ATTR_NORETURN ATTR_COLD void throw_last_deemon_exception(void);
LOCAL ATTR_RETNONNULL DeeObject *throw_if_null(DeeObject *obj) {
	if unlikely(!obj)
		throw_last_deemon_exception();
	return obj;
}
LOCAL unsigned int throw_if_negative(int x) {
	if unlikely(x < 0)
		throw_last_deemon_exception();
	return (unsigned int)x;
}
LOCAL unsigned long throw_if_negative(long x) {
	if unlikely(x < 0l)
		throw_last_deemon_exception();
	return (unsigned long)x;
}
#ifdef __COMPILER_HAVE_LONGLONG
LOCAL __ULONGLONG throw_if_negative(__LONGLONG x) {
	if unlikely(x < 0ll)
		throw_last_deemon_exception();
	return (__ULONGLONG)x;
}
#endif /* __COMPILER_HAVE_LONGLONG */
LOCAL unsigned int throw_if_nonzero(int x) {
	if unlikely(x != 0)
		throw_last_deemon_exception();
	return (unsigned int)x;
}
LOCAL int throw_if_minusone(int x) {
	if unlikely(x == -1)
		throw_last_deemon_exception();
	return x;
}
LOCAL long throw_if_minusone(long x) {
	if unlikely(x == -1l)
		throw_last_deemon_exception();
	return x;
}
#ifdef __COMPILER_HAVE_LONGLONG
LOCAL __LONGLONG throw_if_minusone(__LONGLONG x) {
	if unlikely(x == -1ll)
		throw_last_deemon_exception();
	return x;
}
#endif /* __COMPILER_HAVE_LONGLONG */
LOCAL unsigned int throw_if_minusone(unsigned int x) {
	if unlikely(x == (unsigned int)-1)
		throw_last_deemon_exception();
	return x;
}
LOCAL unsigned long throw_if_minusone(unsigned long x) {
	if unlikely(x == (unsigned long)-1l)
		throw_last_deemon_exception();
	return x;
}
#ifdef __COMPILER_HAVE_LONGLONG
LOCAL __ULONGLONG throw_if_minusone(__ULONGLONG x) {
	if unlikely(x == (__ULONGLONG)-1ll)
		throw_last_deemon_exception();
	return x;
}
#endif /* __COMPILER_HAVE_LONGLONG */

class Exception: public std::exception {
public:
	virtual char const *what() const DEE_CXX_NOTHROW {
		DeeObject *current = DeeError_Current();
		if (!current)
			return "No exception";
		char const *result = Dee_TYPE(current)->tp_name;
		return result ? result : "<Unnamed exception>";
	}
};

/* Throws the latest deemon error as a C++ exception. */
LOCAL ATTR_COLD ATTR_NORETURN void throw_last_deemon_exception(void) {
	/* XXX: Exception sub-classes? */
	throw deemon::Exception();
}


namespace detail {

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

#define DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(name)                  \
	template<class T = ::DeeObject>                                  \
	class name {                                                     \
		DeeObject *m_ptr;                                            \
	public:                                                          \
		operator T *() const {                                       \
			return (T *)m_ptr;                                       \
		}                                                            \
		explicit name(DeeObject *ptr) DEE_CXX_NOTHROW: m_ptr(ptr) {} \
		name(name const &ptr) DEE_CXX_NOTHROW: m_ptr(ptr.m_ptr) {}   \
	};
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(ObjNonNull)
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(ObjMaybeNull)
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(ObjInherited)
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(ObjNonNullInherited)
DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER(ObjMaybeNullInherited)
#undef DEE_CXX_PRIVATE_DEFINE_OBJECT_WRAPPER


template<class To, class From> class is_object_assignable: public ::std::is_base_of<To, From> {};
template<class From> class is_object_assignable<Object, From>: public ::std::true_type {};
template<class From> class is_object_assignable<DeeObject, From>: public ::std::true_type {};
template<class To> class is_object_assignable<To, DeeObject>: public ::std::true_type {};
template<class To> class is_object_assignable<To, Object>: public ::std::true_type {};
template<> class is_object_assignable<DeeObject, DeeObject>: public ::std::true_type {};
template<> class is_object_assignable<DeeObject, Object>: public ::std::true_type {};
template<> class is_object_assignable<Object, DeeObject>: public ::std::true_type {};
template<> class is_object_assignable<Object, Object>: public ::std::true_type {};

template<class T> class is_object: public ::std::is_convertible<T, ::DeeObject> {};
template<class T> class is_object_ptr: public ::std::false_type {};
template<class T> class is_object_ptr<T *>: public is_object<T> {};
template<class T> class is_object_ptr<Ref<T> >: public ::std::true_type {};

#define DEE_ENABLE_IF_OBJECT(S) \
	typename ::std::enable_if< ::deemon::detail::is_object<S>::value, S>::type
#define DEE_ENABLE_IF_OBJECT_T(S, T) \
	typename ::std::enable_if< ::deemon::detail::is_object<S>::value, T>::type
#define DEE_ENABLE_IF_OBJECT_AND_PTR_T(Sob, Sptr, T) \
	typename ::std::enable_if< ::deemon::detail::is_object<Sob>::value && ::deemon::detail::is_object_ptr<Sptr>::value, T>::type
#define DEE_ENABLE_IF_OBJECT_PTR_T(S, T) \
	typename ::std::enable_if< ::deemon::detail::is_object_ptr<S>::value, T>::type
#define DEE_ENABLE_IF_OBJECT_PTR2_T(S1, S2, T) \
	typename ::std::enable_if< ::deemon::detail::is_object_ptr<S1>::value && ::deemon::detail::is_object_ptr<S2>::value, T>::type
#define DEE_ENABLE_IF_OBJECT_PTR3_T(S1, S2, S3, T) \
	typename ::std::enable_if< ::deemon::detail::is_object_ptr<S1>::value && ::deemon::detail::is_object_ptr<S2>::value && ::deemon::detail::is_object_ptr<S3>::value, T>::type
#define DEE_ENABLE_IF_OBJECT_PTR4_T(S1, S2, S3, S4, T) \
	typename ::std::enable_if< ::deemon::detail::is_object_ptr<S1>::value && ::deemon::detail::is_object_ptr<S2>::value && ::deemon::detail::is_object_ptr<S3>::value && ::deemon::detail::is_object_ptr<S4>::value, T>::type
#define DEE_ENABLE_IF_OBJECT_PTR5_T(S1, S2, S3, S4, S5, T) \
	typename ::std::enable_if< ::deemon::detail::is_object_ptr<S1>::value && ::deemon::detail::is_object_ptr<S2>::value && ::deemon::detail::is_object_ptr<S3>::value && ::deemon::detail::is_object_ptr<S4>::value && ::deemon::detail::is_object_ptr<S5>::value, T>::type
#define DEE_ENABLE_IF_OBJECT_PTR6_T(S1, S2, S3, S4, S5, S6, T) \
	typename ::std::enable_if< ::deemon::detail::is_object_ptr<S1>::value && ::deemon::detail::is_object_ptr<S2>::value && ::deemon::detail::is_object_ptr<S3>::value && ::deemon::detail::is_object_ptr<S4>::value && ::deemon::detail::is_object_ptr<S5>::value && ::deemon::detail::is_object_ptr<S6>::value, T>::type
#define DEE_ENABLE_IF_OBJECT_ASSIGNABLE(To, From) \
	typename ::std::enable_if< ::deemon::detail::is_object_assignable<To, From>::value, To>::type
#define DEE_ENABLE_IF_OBJECT_ASSIGNABLE_T(To, From, T) \
	typename ::std::enable_if< ::deemon::detail::is_object_assignable<To, From>::value, T>::type
#define DEE_ENABLE_IF_OBJECT_ASSIGNABLE2(To, From1, From2) \
	typename ::std::enable_if< ::deemon::detail::is_object_assignable<To, From1>::value && ::deemon::detail::is_object_assignable<To, From2>::value, To>::type


class RefBase {
protected:
	DREF DeeObject *m_ptr; /* [0..1] Referenced object. */
public:
	RefBase() DEE_CXX_NOTHROW
		: m_ptr(NULL) {}
	RefBase(RefBase &&right) DEE_CXX_NOTHROW
		: m_ptr(right.release()) {}
	RefBase(RefBase const &right) DEE_CXX_NOTHROW
		: m_ptr(incref(right.m_ptr)) {}
	RefBase(DeeObject *ptr)
		: m_ptr(incref(throw_if_null(ptr))) {}
	template<class S> RefBase(detail::ObjNonNull<S> ptr) DEE_CXX_NOTHROW
		: m_ptr(incref((DeeObject *)ptr)) {}
	template<class S> RefBase(detail::ObjMaybeNull<S> ptr) DEE_CXX_NOTHROW
		: m_ptr(xincref((DeeObject *)ptr)) {}
	template<class S> RefBase(detail::ObjInherited<S> ptr)
		: m_ptr(throw_if_null((DeeObject *)ptr)) {}
	template<class S> RefBase(detail::ObjNonNullInherited<S> ptr) DEE_CXX_NOTHROW
		: m_ptr((DeeObject *)ptr) {}
	template<class S> RefBase(detail::ObjMaybeNullInherited<S> ptr) DEE_CXX_NOTHROW
		: m_ptr((DeeObject *)ptr) {}

	~RefBase() DEE_CXX_NOTHROW {
		Dee_XDecref(m_ptr);
	}

	void set(RefBase &&right) {
		DREF DeeObject *oldval;
		oldval = m_ptr;
		m_ptr  = right.release();
		Dee_XDecref(oldval);
	}
	void set(RefBase const &right) {
		DREF DeeObject *oldval;
		oldval = m_ptr;
		m_ptr  = incref(right.m_ptr);
		Dee_XDecref(oldval);
	}
	void set(DeeObject *ptr) {
		DREF DeeObject *oldval;
		oldval = m_ptr;
		m_ptr  = incref(throw_if_null(ptr));
		Dee_XDecref(oldval);
	}
	template<class S> void set(detail::ObjNonNull<S> ptr) DEE_CXX_NOTHROW {
		DREF DeeObject *oldval;
		oldval = m_ptr;
		m_ptr  = incref((DeeObject *)ptr);
		Dee_XDecref(oldval);
	}
	template<class S> void set(detail::ObjMaybeNull<S> ptr) DEE_CXX_NOTHROW {
		DREF DeeObject *oldval;
		oldval = m_ptr;
		m_ptr  = xincref((DeeObject *)ptr);
		Dee_XDecref(oldval);
	}
	template<class S> void set(detail::ObjInherited<S> ptr) {
		DREF DeeObject *oldval;
		oldval = m_ptr;
		m_ptr  = throw_if_null((DeeObject *)ptr);
		Dee_XDecref(oldval);
	}
	template<class S> void set(detail::ObjNonNullInherited<S> ptr) DEE_CXX_NOTHROW {
		DREF DeeObject *oldval;
		oldval = m_ptr;
		m_ptr  = (DeeObject *)ptr;
		Dee_XDecref(oldval);
	}
	template<class S> void set(detail::ObjMaybeNullInherited<S> ptr) DEE_CXX_NOTHROW {
		DREF DeeObject *oldval;
		oldval = m_ptr;
		m_ptr  = (DeeObject *)ptr;
		Dee_XDecref(oldval);
	}

	WUNUSED bool isempty() const DEE_CXX_NOTHROW {
		return m_ptr == NULL;
	}
	WUNUSED bool ispresent() const DEE_CXX_NOTHROW {
		return m_ptr != NULL;
	}
	WUNUSED DeeObject *ptr() const DEE_CXX_NOTHROW {
		return m_ptr;
	}
	operator DeeObject *(void) const DEE_CXX_NOTHROW {
		return m_ptr;
	}
	DREF DeeObject *release(void) DEE_CXX_NOTHROW {
		DREF DeeObject *result;
		result = m_ptr;
		m_ptr  = NULL;
		return result;
	}
	bool isnone() const DEE_CXX_NOTHROW {
		return DeeNone_Check(m_ptr);
	}

	RefBase &inplace_add(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceAddInt8(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_add(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceAddUInt32(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_add(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceAdd(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_sub(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceSubInt8(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_sub(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceSubUInt32(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_sub(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceSub(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_mul(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceMulInt8(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_mul(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceMul(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_div(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceDivInt8(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_div(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceDiv(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_mod(int8_t right) {
		throw_if_nonzero(DeeObject_InplaceModInt8(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_mod(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceMod(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_shl(uint8_t right) {
		throw_if_nonzero(DeeObject_InplaceShlUInt8(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_shl(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceShl(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_shr(uint8_t right) {
		throw_if_nonzero(DeeObject_InplaceShrUInt8(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_shr(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceShr(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_and(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceAndUInt32(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_and(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceAnd(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_or(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceOrUInt32(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_or(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceOr(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_xor(uint32_t right) {
		throw_if_nonzero(DeeObject_InplaceXorUInt32(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_xor(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplaceXor(&m_ptr, right));
		return *this;
	}
	RefBase &inplace_pow(DeeObject *right) {
		throw_if_nonzero(DeeObject_InplacePow(&m_ptr, right));
		return *this;
	}

	RefBase &inplace_inc() {
		throw_if_nonzero(DeeObject_Inc(&m_ptr));
		return *this;
	}
	RefBase &inplace_dec() {
		throw_if_nonzero(DeeObject_Dec(&m_ptr));
		return *this;
	}
	WUNUSED DREF DeeObject *inplace_postinc() {
		DREF DeeObject *result;
		int error;
		result = throw_if_null(DeeObject_Copy(m_ptr));
		error  = DeeObject_Inc(&m_ptr);
		if unlikely(error) {
			Dee_Decref(result);
			throw_last_deemon_exception();
		}
		return result;
	}
	WUNUSED DREF DeeObject *inplace_postdec() {
		DREF DeeObject *result;
		int error;
		result = throw_if_null(DeeObject_Copy(m_ptr));
		error  = DeeObject_Dec(&m_ptr);
		if unlikely(error) {
			Dee_Decref(result);
			throw_last_deemon_exception();
		}
		return result;
	}
};

class WeakRefBase {
private:
	struct ::Dee_weakref w_ref; /* The underlying weak reference. */
public:
	WeakRefBase() DEE_CXX_NOTHROW {
		Dee_weakref_initempty(&w_ref);
	}
	WeakRefBase(std::nullptr_t) DEE_CXX_NOTHROW {
		Dee_weakref_initempty(&w_ref);
	}
	WeakRefBase(DeeObject *ptr) {
		if (ptr) {
			if (!Dee_weakref_init(&w_ref, ptr, NULL))
				detail::err_cannot_weak_reference(ptr);
		} else {
			Dee_weakref_initempty(&w_ref);
		}
	}
	template<class S> WeakRefBase(ObjNonNull<S> ptr) {
		if (!Dee_weakref_init(&w_ref, ptr, NULL))
			err_cannot_weak_reference(ptr);
	}
	template<class S> WeakRefBase(ObjMaybeNull<S> ptr) {
		if (ptr) {
			if (!Dee_weakref_init(&w_ref, ptr, NULL))
				err_cannot_weak_reference(ptr);
		} else {
			Dee_weakref_initempty(&w_ref);
		}
	}
	WeakRefBase(WeakRefBase const &other) DEE_CXX_NOTHROW {
		Dee_weakref_copy(&w_ref, &other.w_ref);
	}
	WeakRefBase(WeakRefBase &&other) DEE_CXX_NOTHROW {
		Dee_weakref_move(&w_ref, &other.w_ref);
	}
	~WeakRefBase() DEE_CXX_NOTHROW {
		Dee_weakref_fini(&w_ref);
	}
	WeakRefBase &operator=(WeakRefBase &&other) DEE_CXX_NOTHROW {
		Dee_weakref_moveassign(&w_ref, &other.w_ref);
		return *this;
	}
	WeakRefBase &operator=(WeakRefBase const &other) DEE_CXX_NOTHROW {
		Dee_weakref_copyassign(&w_ref, &other.w_ref);
		return *this;
	}
	WUNUSED DREF DeeObject *trylock() const DEE_CXX_NOTHROW {
		return Dee_weakref_lock(&w_ref);
	}
	ATTR_RETNONNULL WUNUSED DREF DeeObject *lock() const {
		DREF DeeObject *result = Dee_weakref_lock(&w_ref);
		if (!result)
			detail::err_cannot_lock_weakref();
		return result;
	}
	ATTR_RETNONNULL WUNUSED NONNULL_CXX((1)) DREF DeeObject *lock(DeeObject *def) const DEE_CXX_NOTHROW {
		DREF DeeObject *result = Dee_weakref_lock(&w_ref);
		if (!result) {
			result = def;
			Dee_Incref(def);
		}
		return result;
	}
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
	DREF DeeObject *cmpxch(DeeObject *old_ob, DeeObject *new_ob) DEE_CXX_NOTHROW {
		return Dee_weakref_cmpxch(&w_ref, old_ob, new_ob);
	}
	void set(std::nullptr_t) {
		Dee_weakref_clear(&w_ref);
	}
	void set(DeeObject *ob) {
		if (ob != NULL) {
			set(ObjNonNull<>(ob));
		} else {
			Dee_weakref_clear(&w_ref);
		}
	}
	template<class S> void set(ObjNonNull<S> ob) {
		if (!Dee_weakref_set(&w_ref, (DeeObject *)ob))
			err_cannot_weak_reference(ob);
	}
	template<class S> void set(ObjMaybeNull<S> ob) {
		set((DeeObject *)ob);
	}
};

} /* namespace detail */


/* Indicate that is-NULL checks to throw an exception can
 * be omitted, because an object pointer is never NULL. */
inline WUNUSED NONNULL_CXX((1)) detail::ObjNonNull<> nonnull(DeeObject *__restrict ptr) {
	return detail::ObjNonNull<>(ptr);
}
template<class T> inline WUNUSED NONNULL_CXX((1))
detail::ObjNonNull<DEE_ENABLE_IF_OBJECT(T)> nonnull(T *__restrict ptr) {
	return detail::ObjNonNull<T>(ptr);
}
template<class T> inline WUNUSED
detail::ObjNonNull<T> nonnull(detail::ObjNonNull<T> ptr) {
	return detail::ObjNonNull<T>((DeeObject *)ptr);
}
template<class T> inline WUNUSED
detail::ObjNonNullInherited<T> nonnull(detail::ObjInherited<T> ptr) {
	return detail::ObjNonNullInherited<T>((DeeObject *)ptr);
}
template<class T> inline WUNUSED
detail::ObjNonNullInherited<T> nonnull(detail::ObjNonNullInherited<T> ptr) {
	return detail::ObjNonNullInherited<T>((DeeObject *)ptr);
}

/* Indicate that is-NULL checks to throw an exception can
 * be omitted, because a NULL-object is allowed. */
inline WUNUSED detail::ObjMaybeNull<> maybenull(DeeObject *ptr) {
	return detail::ObjMaybeNull<>(ptr);
}
template<class T> inline WUNUSED
detail::ObjMaybeNull<DEE_ENABLE_IF_OBJECT(T)> maybenull(T *ptr) {
	return detail::ObjMaybeNull<T>(ptr);
}
template<class T> inline WUNUSED
detail::ObjMaybeNull<T> maybenull(detail::ObjMaybeNull<T> ptr) {
	return detail::ObjMaybeNull<T>((DeeObject *)ptr);
}
template<class T> inline WUNUSED
detail::ObjMaybeNullInherited<T> maybenull(detail::ObjInherited<T> ptr) {
	return detail::ObjMaybeNullInherited<T>((DeeObject *)ptr);
}
template<class T> inline WUNUSED
detail::ObjMaybeNullInherited<T> maybenull(detail::ObjMaybeNullInherited<T> ptr) {
	return detail::ObjMaybeNullInherited<T>((DeeObject *)ptr);
}

/* Indicate that an object reference should be inherited. */
inline WUNUSED detail::ObjInherited<> inherit(DeeObject *__restrict ptr) {
	return detail::ObjInherited<>(ptr);
}
template<class T> inline WUNUSED
detail::ObjInherited<DEE_ENABLE_IF_OBJECT(T)> inherit(T *__restrict ptr) {
	return detail::ObjInherited<T>(ptr);
}
template<class T> inline WUNUSED
detail::ObjInherited<T> inherit(detail::ObjInherited<T> ptr) {
	return detail::ObjInherited<T>((DeeObject *)ptr);
}
template<class T> inline WUNUSED
detail::ObjNonNullInherited<T> inherit(detail::ObjNonNull<T> ptr) {
	return detail::ObjNonNullInherited<T>((DeeObject *)ptr);
}
template<class T> inline WUNUSED
detail::ObjNonNullInherited<T> inherit(detail::ObjNonNullInherited<T> ptr) {
	return detail::ObjNonNullInherited<T>((DeeObject *)ptr);
}

template<class T>
class Ref: protected detail::RefBase {
	template<class S> friend class Ref;
public:
	Ref() DEE_CXX_NOTHROW
		: RefBase() {}
	Ref(Ref<T> &&right) DEE_CXX_NOTHROW
		: RefBase(static_cast<RefBase &&>(right)) {}
	Ref(Ref<T> const &right) DEE_CXX_NOTHROW
		: RefBase(static_cast<RefBase const &>(right)) {}
	template<class S> Ref(Ref<S> &&right, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S) * = 0) DEE_CXX_NOTHROW
		: RefBase(static_cast<RefBase &&>(right)) {}
	template<class S> Ref(Ref<S> const &right, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S) * = 0) DEE_CXX_NOTHROW
		: RefBase(static_cast<RefBase const &>(right)) {}
	template<class S> Ref(S *ptr, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S) * = 0)
		: RefBase(static_cast<DeeObject *>(ptr)) {}
	template<class S> Ref(detail::ObjNonNull<S> ptr, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S) * = 0) DEE_CXX_NOTHROW
		: RefBase(ptr) {}
	template<class S> Ref(detail::ObjMaybeNull<S> ptr, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S) * = 0) DEE_CXX_NOTHROW
		: RefBase(ptr) {}
	template<class S> Ref(detail::ObjInherited<S> ptr, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S) * = 0)
		: RefBase(ptr) {}
	template<class S> Ref(detail::ObjNonNullInherited<S> ptr, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S) * = 0) DEE_CXX_NOTHROW
		: RefBase(ptr) {}
	template<class S> Ref(detail::ObjMaybeNullInherited<S> ptr, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S) * = 0) DEE_CXX_NOTHROW
		: RefBase(ptr) {}

	Ref<T> &operator=(Ref<T> &&right) DEE_CXX_NOTHROW {
		RefBase::set(static_cast<RefBase &&>(right));
		return *this;
	}
	Ref<T> &operator=(Ref<T> const &right) DEE_CXX_NOTHROW {
		RefBase::set(static_cast<RefBase const &>(right));
		return *this;
	}
	template<class S> Ref<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S)> &operator=(Ref<S> &&right) DEE_CXX_NOTHROW {
		RefBase::set(static_cast<RefBase &&>(right));
		return *this;
	}
	template<class S> Ref<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S)> &operator=(Ref<S> const &right) DEE_CXX_NOTHROW {
		RefBase::set(static_cast<RefBase const &>(right));
		return *this;
	}
	template<class S> Ref<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S)> &operator=(S *ptr) {
		RefBase::set(static_cast<DeeObject *>(ptr));
		return *this;
	}
	template<class S> Ref<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S)> &operator=(detail::ObjNonNull<S> ptr) DEE_CXX_NOTHROW {
		RefBase::set(ptr);
		return *this;
	}
	template<class S> Ref<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S)> &operator=(detail::ObjMaybeNull<S> ptr) DEE_CXX_NOTHROW {
		RefBase::set(ptr);
		return *this;
	}
	template<class S> Ref<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S)> &operator=(detail::ObjInherited<S> ptr) {
		RefBase::set(ptr);
		return *this;
	}
	template<class S> Ref<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S)> &operator=(detail::ObjNonNullInherited<S> ptr) DEE_CXX_NOTHROW {
		RefBase::set(ptr);
		return *this;
	}
	template<class S> Ref<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S)> &operator=(detail::ObjMaybeNullInherited<S> ptr) DEE_CXX_NOTHROW {
		RefBase::set(ptr);
		return *this;
	}

	WUNUSED DREF T *release() DEE_CXX_NOTHROW {
		return (DREF T *)RefBase::release();
	}
	WUNUSED operator T *(void) const DEE_CXX_NOTHROW {
		return (T *)m_ptr;
	}
	WUNUSED T *ptr() const DEE_CXX_NOTHROW {
		return (T *)m_ptr;
	}
	WUNUSED T *operator->() const DEE_CXX_NOTHROW {
		return (T *)m_ptr;
	}
	WUNUSED T &operator*() const DEE_CXX_NOTHROW {
		return *(T *)m_ptr;
	}
	WUNUSED T &get() const DEE_CXX_NOTHROW {
		return *(T *)m_ptr;
	}
	using RefBase::isnone;
	using RefBase::isempty;
	using RefBase::ispresent;
	using RefBase::operator DeeObject *;

	/* Generic operator invocation. */
	Ref<Object> invoke_inplace_operator(uint16_t name) {
		return inherit(DeeObject_PInvokeOperator(&m_ptr, name, 0, NULL));
	}
	template<class ...ArgTypes>
	Ref<Object> invoke_inplace_operator(uint16_t name, Tuple<ArgTypes...> const *args) {
		return inherit(DeeObject_PInvokeOperatorTuple(&m_ptr, name, (DeeObject *)args));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<Object>) invoke_inplace_operator(uint16_t name, size_t argc, ArgType *const *argv) {
		return inherit(DeeObject_PInvokeOperator(&m_ptr, name, argc, (DeeObject *const *)argv));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<Object>) invoke_inplace_operator(uint16_t name, size_t argc, ArgType **argv) {
		return inherit(DeeObject_PInvokeOperator(&m_ptr, name, argc, (DeeObject *const *)argv));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_PTR_T(ArgType, Ref<Object>) invoke_inplace_operator(uint16_t name, std::initializer_list<ArgType> const &args) {
		return inherit(DeeObject_PInvokeOperator(&m_ptr, name, args.size(), (DeeObject *const *)args.begin()));
	}

	Ref<T> &inplace_add(int8_t right) {
		return (Ref<T> &)RefBase::inplace_add(right);
	}
	Ref<T> &inplace_add(uint32_t right) {
		return (Ref<T> &)RefBase::inplace_add(right);
	}
	Ref<T> &inplace_add(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_add(right);
	}
	Ref<T> &inplace_sub(int8_t right) {
		return (Ref<T> &)RefBase::inplace_sub(right);
	}
	Ref<T> &inplace_sub(uint32_t right) {
		return (Ref<T> &)RefBase::inplace_sub(right);
	}
	Ref<T> &inplace_sub(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_sub(right);
	}
	Ref<T> &inplace_mul(int8_t right) {
		return (Ref<T> &)RefBase::inplace_mul(right);
	}
	Ref<T> &inplace_mul(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_mul(right);
	}
	Ref<T> &inplace_div(int8_t right) {
		return (Ref<T> &)RefBase::inplace_div(right);
	}
	Ref<T> &inplace_div(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_div(right);
	}
	Ref<T> &inplace_mod(int8_t right) {
		return (Ref<T> &)RefBase::inplace_mod(right);
	}
	Ref<T> &inplace_mod(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_mod(right);
	}
	Ref<T> &inplace_shl(uint8_t right) {
		return (Ref<T> &)RefBase::inplace_shl(right);
	}
	Ref<T> &inplace_shl(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_shl(right);
	}
	Ref<T> &inplace_shr(uint8_t right) {
		return (Ref<T> &)RefBase::inplace_shr(right);
	}
	Ref<T> &inplace_shr(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_shr(right);
	}
	Ref<T> &inplace_and(uint32_t right) {
		return (Ref<T> &)RefBase::inplace_and(right);
	}
	Ref<T> &inplace_and(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_and(right);
	}
	Ref<T> &inplace_or(uint32_t right) {
		return (Ref<T> &)RefBase::inplace_or(right);
	}
	Ref<T> &inplace_or(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_or(right);
	}
	Ref<T> &inplace_xor(uint32_t right) {
		return (Ref<T> &)RefBase::inplace_xor(right);
	}
	Ref<T> &inplace_xor(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_xor(right);
	}
	Ref<T> &inplace_pow(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_pow(right);
	}
	Ref<T> &inplace_inc() {
		return (Ref<T> &)RefBase::inplace_inc();
	}
	Ref<T> &inplace_dec() {
		return (Ref<T> &)RefBase::inplace_dec();
	}
	Ref<T> inplace_postinc() {
		return inherit(nonnull(RefBase::inplace_postinc()));
	}
	Ref<T> inplace_postdec() {
		return inherit(nonnull(RefBase::inplace_postdec()));
	}

	Ref<T> &operator+=(int8_t right) {
		return (Ref<T> &)RefBase::inplace_add(right);
	}
	Ref<T> &operator+=(uint32_t right) {
		return (Ref<T> &)RefBase::inplace_add(right);
	}
	Ref<T> &operator+=(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_add(right);
	}
	Ref<T> &operator-=(int8_t right) {
		return (Ref<T> &)RefBase::inplace_sub(right);
	}
	Ref<T> &operator-=(uint32_t right) {
		return (Ref<T> &)RefBase::inplace_sub(right);
	}
	Ref<T> &operator-=(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_sub(right);
	}
	Ref<T> &operator*=(int8_t right) {
		return (Ref<T> &)RefBase::inplace_mul(right);
	}
	Ref<T> &operator*=(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_mul(right);
	}
	Ref<T> &operator/=(int8_t right) {
		return (Ref<T> &)RefBase::inplace_div(right);
	}
	Ref<T> &operator/=(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_div(right);
	}
	Ref<T> &operator%=(int8_t right) {
		return (Ref<T> &)RefBase::inplace_mod(right);
	}
	Ref<T> &operator%=(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_mod(right);
	}
	Ref<T> &operator<<=(uint8_t right) {
		return (Ref<T> &)RefBase::inplace_shl(right);
	}
	Ref<T> &operator<<=(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_shl(right);
	}
	Ref<T> &operator>>=(uint8_t right) {
		return (Ref<T> &)RefBase::inplace_shr(right);
	}
	Ref<T> &operator>>=(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_shr(right);
	}
	Ref<T> &operator&=(uint32_t right) {
		return (Ref<T> &)RefBase::inplace_and(right);
	}
	Ref<T> &operator&=(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_and(right);
	}
	Ref<T> &operator|=(uint32_t right) {
		return (Ref<T> &)RefBase::inplace_or(right);
	}
	Ref<T> &operator|=(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_or(right);
	}
	Ref<T> &operator^=(uint32_t right) {
		return (Ref<T> &)RefBase::inplace_xor(right);
	}
	Ref<T> &operator^=(DeeObject *right) {
		return (Ref<T> &)RefBase::inplace_xor(right);
	}
	Ref<T> &operator++() {
		return (Ref<T> &)RefBase::inplace_inc();
	}
	Ref<T> &operator--() {
		return (Ref<T> &)RefBase::inplace_dec();
	}
	Ref<T> operator++(int) {
		return inherit(nonnull(RefBase::inplace_postinc()));
	}
	Ref<T> operator--(int) {
		return inherit(nonnull(RefBase::inplace_postdec()));
	}
};

inline WUNUSED Ref<> ref(DeeObject *ptr) { return ptr; }
template<class T> inline WUNUSED Ref<T> ref(Ref<T> const &ptr) { return ptr; }
template<class T> inline WUNUSED Ref<DEE_ENABLE_IF_OBJECT(T)> ref(T &obj) { return obj; }
template<class T> inline WUNUSED Ref<DEE_ENABLE_IF_OBJECT(T)> ref(T *ptr) { return ptr; }
template<class T> inline WUNUSED Ref<T> ref(detail::ObjNonNull<T> ptr) DEE_CXX_NOTHROW { return ptr; }
template<class T> inline WUNUSED Ref<T> ref(detail::ObjMaybeNull<T> ptr) DEE_CXX_NOTHROW { return ptr; }
template<class T> inline WUNUSED Ref<T> ref(detail::ObjInherited<T> ptr) { return ptr; }
template<class T> inline WUNUSED Ref<T> ref(detail::ObjNonNullInherited<T> ptr) DEE_CXX_NOTHROW { return ptr; }
template<class T> inline WUNUSED Ref<T> ref(detail::ObjMaybeNullInherited<T> ptr) DEE_CXX_NOTHROW { return ptr; }


template<class T>
class WeakRef: public detail::WeakRefBase {
public:
	WeakRef() DEE_CXX_NOTHROW
		: WeakRefBase() {}
	WeakRef(WeakRef<T> const &other) DEE_CXX_NOTHROW
		: WeakRefBase(static_cast<WeakRefBase const &>(other)) {}
	WeakRef(WeakRef<T> &&other) DEE_CXX_NOTHROW
		: WeakRefBase(static_cast<WeakRefBase &&>(other)) {}
	template<class S> WeakRef(S *ob, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S) * = 0)
		: WeakRefBase((DeeObject *)ob) {}
	template<class S> WeakRef(Ref<S> const &ob, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S) * = 0)
		: WeakRefBase((DeeObject *)ob.ptr()) {}
	template<class S> WeakRef(detail::ObjNonNull<S> ptr, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S) * = 0)
		: WeakRefBase(ptr) {}
	template<class S> WeakRef(detail::ObjMaybeNull<S> ptr, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S) * = 0)
		: WeakRefBase(ptr) {}
	template<class S> WeakRef(WeakRef<S> const &other, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S) * = 0) DEE_CXX_NOTHROW
		: WeakRefBase(static_cast<WeakRefBase const &>(other)) {}
	template<class S> WeakRef(WeakRef<S> &&other, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S) * = 0) DEE_CXX_NOTHROW
		: WeakRefBase(static_cast<WeakRefBase &&>(other)) {}

	WeakRef<T> &operator=(std::nullptr_t) DEE_CXX_NOTHROW {
		WeakRefBase::set(std::nullptr_t());
		return *this;
	}
	WeakRef<T> &operator=(WeakRef<T> const &other) DEE_CXX_NOTHROW {
		WeakRefBase::operator=(static_cast<WeakRefBase const &>(other));
		return *this;
	}
	WeakRef<T> &operator=(WeakRef<T> &&other) DEE_CXX_NOTHROW {
		WeakRefBase::operator=(static_cast<WeakRefBase &&>(other));
		return *this;
	}
	template<class S> WeakRef<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S)> &operator=(S *ob) {
		WeakRefBase::set(static_cast<DeeObject *>(ob));
		return *this;
	}
	template<class S> WeakRef<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S)> &operator=(Ref<S> const &ob) {
		WeakRefBase::set(static_cast<DeeObject *>(ob.ptr()));
		return *this;
	}
	template<class S> WeakRef<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S)> &operator=(detail::ObjNonNull<S> ptr) {
		WeakRefBase::set(ptr);
		return *this;
	}
	template<class S> WeakRef<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S)> &operator=(detail::ObjMaybeNull<S> ptr) {
		WeakRefBase::set(ptr);
		return *this;
	}
	template<class S> WeakRef<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S)> &operator=(WeakRef<S> const &other) DEE_CXX_NOTHROW {
		WeakRefBase::operator=(static_cast<WeakRefBase const &>(other));
		return *this;
	}
	template<class S> WeakRef<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(T, S)> &operator=(WeakRef<S> &&other) DEE_CXX_NOTHROW {
		WeakRefBase::operator=(static_cast<WeakRefBase &&>(other));
		return *this;
	}

	WUNUSED Ref<T> trylock() const DEE_CXX_NOTHROW {
		return detail::ObjMaybeNullInherited<T>(WeakRefBase::trylock());
	}
	WUNUSED Ref<T> lock() const {
		return detail::ObjNonNullInherited<T>(WeakRefBase::lock());
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> lock(DeeObject *def) const DEE_CXX_NOTHROW {
		return detail::ObjNonNullInherited<T>(WeakRefBase::lock(def));
	}
	using WeakRefBase::alive;
	using WeakRefBase::operator bool;
	using WeakRefBase::operator!;
	using WeakRefBase::clear;
	Ref<T> cmpxch(DeeObject *old_ob, DeeObject *new_ob) DEE_CXX_NOTHROW {
		return WeakRefBase::cmpxch(old_ob, new_ob);
	}
	void set(std::nullptr_t) {
		WeakRefBase::set(std::nullptr_t());
	}
	NONNULL_CXX((1)) void set(DeeObject *ptr) {
		WeakRefBase::set(ptr);
	}
	template<class S> DEE_ENABLE_IF_OBJECT_ASSIGNABLE_T(T, S, void) set(detail::ObjNonNull<S> ptr) {
		WeakRefBase::set(ptr);
	}
	template<class S> DEE_ENABLE_IF_OBJECT_ASSIGNABLE_T(T, S, void) set(detail::ObjMaybeNull<S> ptr) {
		WeakRefBase::set(ptr);
	}
};

inline WeakRef<> weakref(DeeObject *ob) { return ob; }
template<class T> inline WeakRef<DEE_ENABLE_IF_OBJECT(T)> weakref(T *ob) { return ob; }
template<class T> inline WeakRef<DEE_ENABLE_IF_OBJECT(T)> weakref(Ref<T> const &ob) { return ob; }
template<class T> inline WeakRef<DEE_ENABLE_IF_OBJECT(T)> weakref(detail::ObjNonNull<T> ptr) { return ptr; }
template<class T> inline WeakRef<DEE_ENABLE_IF_OBJECT(T)> weakref(detail::ObjMaybeNull<T> ptr) { return ptr; }



namespace detail {

template<class ProxyType,
         class GetReturnType = Object>
class ConstGetRefProxy {
public:
	Ref<GetReturnType> get() const {
		return inherit(((ProxyType const *)this)->_getref());
	}
	operator Ref<GetReturnType>() const {
		return inherit(((ProxyType const *)this)->_getref());
	}
	Ref<GetReturnType> operator->() const {
		return inherit(((ProxyType const *)this)->_getref());
	}
};

template<class ProxyType,
         class SetValueType = Object>
class ConstSetRefProxy {
public:
	template<class S>
	void set(S *value, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(SetValueType, S) * = 0) const {
		throw_if_nonzero(((ProxyType const *)this)->_setref(value));
	}
	template<class S>
	void set(Ref<S> const &value, DEE_ENABLE_IF_OBJECT_ASSIGNABLE(SetValueType, S) * = 0) const {
		throw_if_nonzero(((ProxyType const *)this)->_setref(value));
	}
	template<class S>
	DEE_ENABLE_IF_OBJECT_ASSIGNABLE_T(SetValueType, S, ProxyType) const &operator=(S *value) const {
		set(value);
		return *(ProxyType const *)this;
	}
	template<class S>
	DEE_ENABLE_IF_OBJECT_ASSIGNABLE_T(SetValueType, S, ProxyType) const &operator=(Ref<S> const &value) const {
		set(value);
		return *(ProxyType const *)this;
	}
};

template<class ProxyType,
         class GetReturnType = Object>
class ConstGetAndSetRefProxy
	: public ConstGetRefProxy<ProxyType, GetReturnType>
	, public ConstSetRefProxy<ProxyType, GetReturnType>
{
public:
	using ConstSetRefProxy<ProxyType, GetReturnType>::operator=;
};

template<class ProxyType, class GetReturnType = Object>
class ConstGetAndSetRefProxyWithDefault
	: public ConstGetAndSetRefProxy<ProxyType, GetReturnType>
{
public:
	using ConstGetAndSetRefProxy<ProxyType, GetReturnType>::get;
	using ConstGetAndSetRefProxy<ProxyType, GetReturnType>::operator=;
	template<class S>
	Ref<DEE_ENABLE_IF_OBJECT_ASSIGNABLE(GetReturnType, S)> get(S *def) const {
		DREF DeeObject *result = ((ProxyType const *)this)->_trygetref();
		if (result == ITER_DONE) {
			result = (DREF DeeObject *)def;
			Dee_Incref(result);
		}
		return inherit(result);
	}
};

template<class ProxyType,
         class CallReturnType = Object>
class CallRefProxy {
public:
	Ref<CallReturnType> call() {
		return inherit(((ProxyType *)this)->_callref(0, NULL));
	}
	template<class ...ArgTypes>
	Ref<CallReturnType> call(Tuple<ArgTypes...> const *args) {
		return inherit(((ProxyType *)this)->_callref((DeeObject *)args));
	}
	template<class KwTypePtr, class ...ArgTypes>
	DEE_ENABLE_IF_OBJECT_PTR_T(KwTypePtr, Ref<CallReturnType>) call(Tuple<ArgTypes...> const *args, KwTypePtr kw) {
		return inherit(((ProxyType *)this)->_callref((DeeObject *)args, (DeeObject *)kw));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<CallReturnType>) call(size_t argc, ArgType *const *argv) {
		return inherit(((ProxyType *)this)->_callref(argc, (DeeObject *const *)argv));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<CallReturnType>) call(size_t argc, ArgType **argv) {
		return inherit(((ProxyType *)this)->_callref(argc, (DeeObject *const *)argv));
	}
	template<class ArgType, class KwTypePtr>
	DEE_ENABLE_IF_OBJECT_AND_PTR_T(ArgType, KwTypePtr, Ref<CallReturnType>) call(size_t argc, ArgType *const *argv, KwTypePtr kw) {
		return inherit(((ProxyType *)this)->_callref(argc, (DeeObject *const *)argv, (DeeObject *)kw));
	}
	template<class ArgType, class KwTypePtr>
	DEE_ENABLE_IF_OBJECT_AND_PTR_T(ArgType, KwTypePtr, Ref<CallReturnType>) call(size_t argc, ArgType **argv, KwTypePtr kw) {
		return inherit(((ProxyType *)this)->_callref(argc, (DeeObject *const *)argv, (DeeObject *)kw));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_PTR_T(ArgType, Ref<CallReturnType>) call(std::initializer_list<ArgType> const &args) {
		return inherit(((ProxyType *)this)->_callref(args.size(), (DeeObject *const *)args.begin()));
	}
	template<class ArgType, class KwTypePtr>
	DEE_ENABLE_IF_OBJECT_PTR2_T(ArgType, KwTypePtr, Ref<CallReturnType>) call(std::initializer_list<ArgType> const &args, KwTypePtr kw) {
		return inherit(((ProxyType *)this)->_callref(args.size(), (DeeObject *const *)args.begin(), kw));
	}

	Ref<CallReturnType> operator()() const {
		return inherit(((ProxyType *)this)->_callref(0, NULL));
	}
/*[[[deemon
for (local n: [1:7]) {
	print("	template<", ", ".join(for (local i: [:n]) f"class A{i}"), ">");
	print("	DEE_ENABLE_IF_OBJECT_PTR", n > 1 ? n : "", "_T(", ", ".join(for (local i: [:n]) f"A{i}"), ", Ref<CallReturnType>)");
	print("	operator()(", ", ".join(for (local i: [:n]) f"A{i} a{i}"), ") {");
	print("		DeeObject *argv[", n, "];");
	for (local i: [:n]) {
		print("		argv[", i, "] = static_cast<DeeObject *>(a", i, ");");
	}
	print("		return inherit(((ProxyType *)this)->_callref(", n, ", argv));");
	print("	}");
}
]]]*/
	template<class A0>
	DEE_ENABLE_IF_OBJECT_PTR_T(A0, Ref<CallReturnType>)
	operator()(A0 a0) {
		DeeObject *argv[1];
		argv[0] = static_cast<DeeObject *>(a0);
		return inherit(((ProxyType *)this)->_callref(1, argv));
	}
	template<class A0, class A1>
	DEE_ENABLE_IF_OBJECT_PTR2_T(A0, A1, Ref<CallReturnType>)
	operator()(A0 a0, A1 a1) {
		DeeObject *argv[2];
		argv[0] = static_cast<DeeObject *>(a0);
		argv[1] = static_cast<DeeObject *>(a1);
		return inherit(((ProxyType *)this)->_callref(2, argv));
	}
	template<class A0, class A1, class A2>
	DEE_ENABLE_IF_OBJECT_PTR3_T(A0, A1, A2, Ref<CallReturnType>)
	operator()(A0 a0, A1 a1, A2 a2) {
		DeeObject *argv[3];
		argv[0] = static_cast<DeeObject *>(a0);
		argv[1] = static_cast<DeeObject *>(a1);
		argv[2] = static_cast<DeeObject *>(a2);
		return inherit(((ProxyType *)this)->_callref(3, argv));
	}
	template<class A0, class A1, class A2, class A3>
	DEE_ENABLE_IF_OBJECT_PTR4_T(A0, A1, A2, A3, Ref<CallReturnType>)
	operator()(A0 a0, A1 a1, A2 a2, A3 a3) {
		DeeObject *argv[4];
		argv[0] = static_cast<DeeObject *>(a0);
		argv[1] = static_cast<DeeObject *>(a1);
		argv[2] = static_cast<DeeObject *>(a2);
		argv[3] = static_cast<DeeObject *>(a3);
		return inherit(((ProxyType *)this)->_callref(4, argv));
	}
	template<class A0, class A1, class A2, class A3, class A4>
	DEE_ENABLE_IF_OBJECT_PTR5_T(A0, A1, A2, A3, A4, Ref<CallReturnType>)
	operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) {
		DeeObject *argv[5];
		argv[0] = static_cast<DeeObject *>(a0);
		argv[1] = static_cast<DeeObject *>(a1);
		argv[2] = static_cast<DeeObject *>(a2);
		argv[3] = static_cast<DeeObject *>(a3);
		argv[4] = static_cast<DeeObject *>(a4);
		return inherit(((ProxyType *)this)->_callref(5, argv));
	}
	template<class A0, class A1, class A2, class A3, class A4, class A5>
	DEE_ENABLE_IF_OBJECT_PTR6_T(A0, A1, A2, A3, A4, A5, Ref<CallReturnType>)
	operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
		DeeObject *argv[6];
		argv[0] = static_cast<DeeObject *>(a0);
		argv[1] = static_cast<DeeObject *>(a1);
		argv[2] = static_cast<DeeObject *>(a2);
		argv[3] = static_cast<DeeObject *>(a3);
		argv[4] = static_cast<DeeObject *>(a4);
		argv[5] = static_cast<DeeObject *>(a5);
		return inherit(((ProxyType *)this)->_callref(6, argv));
	}
/*[[[end]]]*/
};

template<class ProxyType,
         class CallReturnType = Object>
class ThisCallRefProxy {
public:
	Ref<CallReturnType> thiscall(DeeObject *this_arg) {
		return inherit(((ProxyType *)this)->_thiscallref(this_arg, 0, NULL));
	}
	template<class ...ArgTypes>
	Ref<CallReturnType> thiscall(DeeObject *this_arg, Tuple<ArgTypes...> const *args) {
		return inherit(((ProxyType *)this)->_thiscallref(this_arg, (DeeObject *)args));
	}
	template<class KwTypePtr, class ...ArgTypes>
	DEE_ENABLE_IF_OBJECT_PTR_T(KwTypePtr, Ref<CallReturnType>) thiscall(DeeObject *this_arg, Tuple<ArgTypes...> const *args, KwTypePtr kw) {
		return inherit(((ProxyType *)this)->_thiscallref(this_arg, (DeeObject *)args, (DeeObject *)kw));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<CallReturnType>) thiscall(DeeObject *this_arg, size_t argc, ArgType *const *argv) {
		return inherit(((ProxyType *)this)->_thiscallref(this_arg, argc, (DeeObject *const *)argv));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<CallReturnType>) thiscall(DeeObject *this_arg, size_t argc, ArgType **argv) {
		return inherit(((ProxyType *)this)->_thiscallref(this_arg, argc, (DeeObject *const *)argv));
	}
	template<class ArgType, class KwTypePtr>
	DEE_ENABLE_IF_OBJECT_AND_PTR_T(ArgType, KwTypePtr, Ref<CallReturnType>) thiscall(DeeObject *this_arg, size_t argc, ArgType *const *argv, KwTypePtr kw) {
		return inherit(((ProxyType *)this)->_thiscallref(this_arg, argc, (DeeObject *const *)argv, (DeeObject *)kw));
	}
	template<class ArgType, class KwTypePtr>
	DEE_ENABLE_IF_OBJECT_AND_PTR_T(ArgType, KwTypePtr, Ref<CallReturnType>) thiscall(DeeObject *this_arg, size_t argc, ArgType **argv, KwTypePtr kw) {
		return inherit(((ProxyType *)this)->_thiscallref(this_arg, argc, (DeeObject *const *)argv, (DeeObject *)kw));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_PTR_T(ArgType, Ref<CallReturnType>) thiscall(DeeObject *this_arg, std::initializer_list<ArgType> const &args) {
		return inherit(((ProxyType *)this)->_thiscallref(this_arg, args.size(), (DeeObject *const *)args.begin()));
	}
	template<class ArgType, class KwTypePtr>
	DEE_ENABLE_IF_OBJECT_PTR2_T(ArgType, KwTypePtr, Ref<CallReturnType>) thiscall(DeeObject *this_arg, std::initializer_list<ArgType> const &args, KwTypePtr kw) {
		return inherit(((ProxyType *)this)->_thiscallref(this_arg, args.size(), (DeeObject *const *)args.begin(), kw));
	}
};

template<class ProxyType,
         class CallReturnType = Object>
class ConstCallRefProxy {
public:
	Ref<CallReturnType> call() const {
		return inherit(((ProxyType const *)this)->_callref(0, NULL));
	}
	template<class ...ArgTypes>
	Ref<CallReturnType> call(Tuple<ArgTypes...> const *args) const {
		return inherit(((ProxyType const *)this)->_callref((DeeObject *)args));
	}
	template<class KwTypePtr, class ...ArgTypes>
	DEE_ENABLE_IF_OBJECT_PTR_T(KwTypePtr, Ref<CallReturnType>) call(Tuple<ArgTypes...> const *args, KwTypePtr kw) const {
		return inherit(((ProxyType const *)this)->_callref((DeeObject *)args));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<CallReturnType>) call(size_t argc, ArgType *const *argv) const {
		return inherit(((ProxyType const *)this)->_callref(argc, (DeeObject *const *)argv));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<CallReturnType>) call(size_t argc, ArgType **argv) const {
		return inherit(((ProxyType const *)this)->_callref(argc, (DeeObject *const *)argv));
	}
	template<class ArgType, class KwTypePtr>
	DEE_ENABLE_IF_OBJECT_AND_PTR_T(ArgType, KwTypePtr, Ref<CallReturnType>) call(size_t argc, ArgType *const *argv, KwTypePtr kw) const {
		return inherit(((ProxyType const *)this)->_callref(argc, (DeeObject *const *)argv, (DeeObject *)kw));
	}
	template<class ArgType, class KwTypePtr>
	DEE_ENABLE_IF_OBJECT_AND_PTR_T(ArgType, KwTypePtr, Ref<CallReturnType>) call(size_t argc, ArgType **argv, KwTypePtr kw) const {
		return inherit(((ProxyType const *)this)->_callref(argc, (DeeObject *const *)argv, (DeeObject *)kw));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_PTR_T(ArgType, Ref<CallReturnType>) call(std::initializer_list<ArgType> const &args) const {
		return inherit(((ProxyType const *)this)->_callref(args.size(), (DeeObject *const *)args.begin()));
	}
	template<class ArgType, class KwTypePtr>
	DEE_ENABLE_IF_OBJECT_PTR2_T(ArgType, KwTypePtr, Ref<CallReturnType>) call(std::initializer_list<ArgType> const &args, KwTypePtr kw) const {
		return inherit(((ProxyType const *)this)->_callref(args.size(), (DeeObject *const *)args.begin(), kw));
	}

	Ref<CallReturnType> operator()() const {
		return inherit(((ProxyType const *)this)->_callref(0, NULL));
	}
/*[[[deemon
for (local n: [1:7]) {
	print("	template<", ", ".join(for (local i: [:n]) f"class A{i}"), ">");
	print("	DEE_ENABLE_IF_OBJECT_PTR", n > 1 ? n : "", "_T(", ", ".join(for (local i: [:n]) f"A{i}"), ", Ref<CallReturnType>)");
	print("	operator()(", ", ".join(for (local i: [:n]) f"A{i} a{i}"), ") const {");
	print("		DeeObject *argv[", n, "];");
	for (local i: [:n]) {
		print("		argv[", i, "] = static_cast<DeeObject *>(a", i, ");");
	}
	print("		return inherit(((ProxyType const *)this)->_callref(", n, ", argv));");
	print("	}");
}
]]]*/
	template<class A0>
	DEE_ENABLE_IF_OBJECT_PTR_T(A0, Ref<CallReturnType>)
	operator()(A0 a0) const {
		DeeObject *argv[1];
		argv[0] = static_cast<DeeObject *>(a0);
		return inherit(((ProxyType const *)this)->_callref(1, argv));
	}
	template<class A0, class A1>
	DEE_ENABLE_IF_OBJECT_PTR2_T(A0, A1, Ref<CallReturnType>)
	operator()(A0 a0, A1 a1) const {
		DeeObject *argv[2];
		argv[0] = static_cast<DeeObject *>(a0);
		argv[1] = static_cast<DeeObject *>(a1);
		return inherit(((ProxyType const *)this)->_callref(2, argv));
	}
	template<class A0, class A1, class A2>
	DEE_ENABLE_IF_OBJECT_PTR3_T(A0, A1, A2, Ref<CallReturnType>)
	operator()(A0 a0, A1 a1, A2 a2) const {
		DeeObject *argv[3];
		argv[0] = static_cast<DeeObject *>(a0);
		argv[1] = static_cast<DeeObject *>(a1);
		argv[2] = static_cast<DeeObject *>(a2);
		return inherit(((ProxyType const *)this)->_callref(3, argv));
	}
	template<class A0, class A1, class A2, class A3>
	DEE_ENABLE_IF_OBJECT_PTR4_T(A0, A1, A2, A3, Ref<CallReturnType>)
	operator()(A0 a0, A1 a1, A2 a2, A3 a3) const {
		DeeObject *argv[4];
		argv[0] = static_cast<DeeObject *>(a0);
		argv[1] = static_cast<DeeObject *>(a1);
		argv[2] = static_cast<DeeObject *>(a2);
		argv[3] = static_cast<DeeObject *>(a3);
		return inherit(((ProxyType const *)this)->_callref(4, argv));
	}
	template<class A0, class A1, class A2, class A3, class A4>
	DEE_ENABLE_IF_OBJECT_PTR5_T(A0, A1, A2, A3, A4, Ref<CallReturnType>)
	operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4) const {
		DeeObject *argv[5];
		argv[0] = static_cast<DeeObject *>(a0);
		argv[1] = static_cast<DeeObject *>(a1);
		argv[2] = static_cast<DeeObject *>(a2);
		argv[3] = static_cast<DeeObject *>(a3);
		argv[4] = static_cast<DeeObject *>(a4);
		return inherit(((ProxyType const *)this)->_callref(5, argv));
	}
	template<class A0, class A1, class A2, class A3, class A4, class A5>
	DEE_ENABLE_IF_OBJECT_PTR6_T(A0, A1, A2, A3, A4, A5, Ref<CallReturnType>)
	operator()(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const {
		DeeObject *argv[6];
		argv[0] = static_cast<DeeObject *>(a0);
		argv[1] = static_cast<DeeObject *>(a1);
		argv[2] = static_cast<DeeObject *>(a2);
		argv[3] = static_cast<DeeObject *>(a3);
		argv[4] = static_cast<DeeObject *>(a4);
		argv[5] = static_cast<DeeObject *>(a5);
		return inherit(((ProxyType const *)this)->_callref(6, argv));
	}
/*[[[end]]]*/
};

class AttrProxyObjBase {
private:
	DeeObject *m_ptr;
	DeeObject *m_str;
public:
	AttrProxyObjBase(DeeObject *ptr, DeeObject *str) DEE_CXX_NOTHROW
		: m_ptr(ptr)
		, m_str(str) {}
	AttrProxyObjBase(AttrProxyObjBase const &right) DEE_CXX_NOTHROW
		: m_ptr(right.m_ptr)
		, m_str(right.m_str) {}
	WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
		return DeeObject_GetAttr(m_ptr, m_str);
	}
	WUNUSED DREF DeeObject *_callref(DeeObject *args) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrTuple(m_ptr, m_str, args);
	}
	WUNUSED DREF DeeObject *_callref(DeeObject *args, DeeObject *kw) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrTupleKw(m_ptr, m_str, args, kw);
	}
	WUNUSED DREF DeeObject *_callref(size_t argc, DeeObject *const *argv) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttr(m_ptr, m_str, argc, argv);
	}
	WUNUSED DREF DeeObject *_callref(size_t argc, DeeObject *const *argv, DeeObject *kw) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrKw(m_ptr, m_str, argc, argv, kw);
	}
	int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
		return DeeObject_SetAttr(m_ptr, m_str, value);
	}
	bool has() const {
		return throw_if_negative(DeeObject_HasAttr(m_ptr, m_str)) != 0;
	}
	bool bound() const {
		return throw_if_minusone(DeeObject_BoundAttr(m_ptr, m_str));
	}
	void del() const {
		throw_if_nonzero(DeeObject_DelAttr(m_ptr, m_str));
	}
};

class AttrProxyStrBase {
private:
	DeeObject *m_ptr;
	char const *m_str;
public:
	AttrProxyStrBase(DeeObject *ptr, char const *str) DEE_CXX_NOTHROW
		: m_ptr(ptr)
		, m_str(str) {}
	AttrProxyStrBase(AttrProxyStrBase const &right) DEE_CXX_NOTHROW
		: m_ptr(right.m_ptr)
		, m_str(right.m_str) {}
	WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
		return DeeObject_GetAttrString(m_ptr, m_str);
	}
	WUNUSED DREF DeeObject *_callref(DeeObject *args) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringTuple(m_ptr, m_str, args);
	}
	WUNUSED DREF DeeObject *_callref(DeeObject *args, DeeObject *kw) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringTupleKw(m_ptr, m_str, args, kw);
	}
	WUNUSED DREF DeeObject *_callref(size_t argc, DeeObject *const *argv) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrString(m_ptr, m_str, argc, argv);
	}
	WUNUSED DREF DeeObject *_callref(size_t argc, DeeObject *const *argv, DeeObject *kw) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringKw(m_ptr, m_str, argc, argv, kw);
	}
	int _setref(DeeObject *value) const {
		return DeeObject_SetAttrString(m_ptr, m_str, value);
	}
	bool has() const {
		return throw_if_negative(DeeObject_HasAttrString(m_ptr, m_str)) != 0;
	}
	bool bound() const {
		return throw_if_minusone(DeeObject_BoundAttrString(m_ptr, m_str));
	}
	void del() const {
		throw_if_nonzero(DeeObject_DelAttrString(m_ptr, m_str));
	}
};


class AttrProxySthBase {
private:
	DeeObject *m_ptr;
	char const *m_str;
	Dee_hash_t m_hsh;
public:
	AttrProxySthBase(DeeObject *ptr, char const *str, Dee_hash_t hsh) DEE_CXX_NOTHROW
		: m_ptr(ptr)
		, m_str(str)
		, m_hsh(hsh) {}
	AttrProxySthBase(AttrProxySthBase const &right) DEE_CXX_NOTHROW
		: m_ptr(right.m_ptr)
		, m_str(right.m_str)
		, m_hsh(right.m_hsh) {}
	WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
		return DeeObject_GetAttrStringHash(m_ptr, m_str, m_hsh);
	}
	WUNUSED DREF DeeObject *_callref(DeeObject *args) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringHashTuple(m_ptr, m_str, m_hsh, args);
	}
	WUNUSED DREF DeeObject *_callref(DeeObject *args, DeeObject *kw) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringHashTupleKw(m_ptr, m_str, m_hsh, args, kw);
	}
	WUNUSED DREF DeeObject *_callref(size_t argc, DeeObject *const *argv) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringHash(m_ptr, m_str, m_hsh, argc, argv);
	}
	WUNUSED DREF DeeObject *_callref(size_t argc, DeeObject *const *argv, DeeObject *kw) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringHashKw(m_ptr, m_str, m_hsh, argc, argv, kw);
	}
	int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
		return DeeObject_SetAttrStringHash(m_ptr, m_str, m_hsh, value);
	}
	bool has() const {
		return throw_if_negative(DeeObject_HasAttrStringHash(m_ptr, m_str, m_hsh)) != 0;
	}
	bool bound() const {
		return throw_if_minusone(DeeObject_BoundAttrStringHash(m_ptr, m_str, m_hsh));
	}
	void del() const {
		throw_if_nonzero(DeeObject_DelAttrStringHash(m_ptr, m_str, m_hsh));
	}
};

class AttrProxySnhBase {
private:
	DeeObject *m_ptr;
	char const *m_str;
	size_t m_len;
	Dee_hash_t m_hsh;
public:
	AttrProxySnhBase(DeeObject *ptr, char const *str, size_t len, Dee_hash_t hsh) DEE_CXX_NOTHROW
		: m_ptr(ptr)
		, m_str(str)
		, m_len(len)
		, m_hsh(hsh) {}
	AttrProxySnhBase(AttrProxySnhBase const &right) DEE_CXX_NOTHROW
		: m_ptr(right.m_ptr)
		, m_str(right.m_str)
		, m_len(right.m_len)
		, m_hsh(right.m_hsh) {}
	WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
		return DeeObject_GetAttrStringLenHash(m_ptr, m_str, m_len, m_hsh);
	}
	WUNUSED DREF DeeObject *_callref(DeeObject *args) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringLenHashTuple(m_ptr, m_str, m_len, m_hsh, args);
	}
	WUNUSED DREF DeeObject *_callref(DeeObject *args, DeeObject *kw) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringLenHashTupleKw(m_ptr, m_str, m_len, m_hsh, args, kw);
	}
	WUNUSED DREF DeeObject *_callref(size_t argc, DeeObject *const *argv) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringLenHash(m_ptr, m_str, m_len, m_hsh, argc, argv);
	}
	WUNUSED DREF DeeObject *_callref(size_t argc, DeeObject *const *argv, DeeObject *kw) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringLenHashKw(m_ptr, m_str, m_len, m_hsh, argc, argv, kw);
	}
	int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
		return DeeObject_SetAttrStringLenHash(m_ptr, m_str, m_len, m_hsh, value);
	}
	bool has() const {
		return throw_if_negative(DeeObject_HasAttrStringLenHash(m_ptr, m_str, m_len, m_hsh)) != 0;
	}
	bool bound() const {
		return throw_if_minusone(DeeObject_BoundAttrStringLenHash(m_ptr, m_str, m_len, m_hsh));
	}
	void del() const {
		throw_if_nonzero(DeeObject_DelAttrStringLenHash(m_ptr, m_str, m_len, m_hsh));
	}
};

class AttrProxyStnBase {
private:
	DeeObject *m_ptr;
	char const *m_str;
	size_t m_len;
public:
	AttrProxyStnBase(DeeObject *ptr, char const *str, size_t len) DEE_CXX_NOTHROW
		: m_ptr(ptr)
		, m_str(str)
		, m_len(len) {}
	AttrProxyStnBase(AttrProxyStnBase const &right) DEE_CXX_NOTHROW
		: m_ptr(right.m_ptr)
		, m_str(right.m_str)
		, m_len(right.m_len) {}
	WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
		return DeeObject_GetAttrStringLen(m_ptr, m_str, m_len);
	}
	WUNUSED DREF DeeObject *_callref(DeeObject *args) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringLenTuple(m_ptr, m_str, m_len, args);
	}
	WUNUSED DREF DeeObject *_callref(DeeObject *args, DeeObject *kw) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringLenTupleKw(m_ptr, m_str, m_len, args, kw);
	}
	WUNUSED DREF DeeObject *_callref(size_t argc, DeeObject *const *argv) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringLen(m_ptr, m_str, m_len, argc, argv);
	}
	WUNUSED DREF DeeObject *_callref(size_t argc, DeeObject *const *argv, DeeObject *kw) const DEE_CXX_NOTHROW {
		return DeeObject_CallAttrStringLenKw(m_ptr, m_str, m_len, argc, argv, kw);
	}
	int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
		return DeeObject_SetAttrStringLen(m_ptr, m_str, m_len, value);
	}
	bool has() const {
		return throw_if_negative(DeeObject_HasAttrStringLen(m_ptr, m_str, m_len)) != 0;
	}
	bool bound() const {
		return throw_if_minusone(DeeObject_BoundAttrStringLen(m_ptr, m_str, m_len));
	}
	void del() const {
		throw_if_nonzero(DeeObject_DelAttrStringLen(m_ptr, m_str, m_len));
	}
};


template<class GetAttrType = Object, class CallReturnType = Object>
class AttrProxyObj
	: public AttrProxyObjBase
	, public ConstGetAndSetRefProxy<AttrProxyObj<GetAttrType, CallReturnType>, GetAttrType>
	, public ConstCallRefProxy<AttrProxyObj<GetAttrType, CallReturnType>, CallReturnType> {
public:
	using ConstGetAndSetRefProxy<AttrProxyObj<GetAttrType, CallReturnType>, GetAttrType>::operator=;
	AttrProxyObj(DeeObject *ptr, DeeObject *str) DEE_CXX_NOTHROW
		: AttrProxyObjBase(ptr, str) {}
	AttrProxyObj(AttrProxyObj const &right) DEE_CXX_NOTHROW
		: AttrProxyObjBase(static_cast<AttrProxyObjBase const &>(right)) {}
};

template<class GetAttrType = Object, class CallReturnType = Object>
class AttrProxyStr
	: public AttrProxyStrBase
	, public ConstGetAndSetRefProxy<AttrProxyStr<GetAttrType, CallReturnType>, GetAttrType>
	, public ConstCallRefProxy<AttrProxyStr<GetAttrType, CallReturnType>, CallReturnType> {
public:
	using ConstGetAndSetRefProxy<AttrProxyStr<GetAttrType, CallReturnType>, GetAttrType>::operator=;
	AttrProxyStr(DeeObject *ptr, char const *str) DEE_CXX_NOTHROW
		: AttrProxyStrBase(ptr, str) {}
	AttrProxyStr(AttrProxyStr const &right) DEE_CXX_NOTHROW
		: AttrProxyStrBase(static_cast<AttrProxyStrBase const &>(right)) {}
};

template<class GetAttrType = Object, class CallReturnType = Object>
class AttrProxySth
	: public AttrProxySthBase
	, public ConstGetAndSetRefProxy<AttrProxySth<GetAttrType, CallReturnType>, GetAttrType>
	, public ConstCallRefProxy<AttrProxySth<GetAttrType, CallReturnType>, CallReturnType> {
public:
	using ConstGetAndSetRefProxy<AttrProxySth<GetAttrType, CallReturnType>, GetAttrType>::operator=;
	AttrProxySth(DeeObject *ptr, char const *str, Dee_hash_t hsh) DEE_CXX_NOTHROW
		: AttrProxySthBase(ptr, str, hsh) {}
	AttrProxySth(AttrProxySth const &right) DEE_CXX_NOTHROW
		: AttrProxySthBase(static_cast<AttrProxySthBase const &>(right)) {}
};

template<class GetAttrType = Object, class CallReturnType = Object>
class AttrProxySnh
	: public AttrProxySnhBase
	, public ConstGetAndSetRefProxy<AttrProxySnh<GetAttrType, CallReturnType>, GetAttrType>
	, public ConstCallRefProxy<AttrProxySnh<GetAttrType, CallReturnType>, CallReturnType> {
public:
	using ConstGetAndSetRefProxy<AttrProxySnh<GetAttrType, CallReturnType>, GetAttrType>::operator=;
	AttrProxySnh(DeeObject *ptr, char const *str, size_t len, Dee_hash_t hsh) DEE_CXX_NOTHROW
		: AttrProxySnhBase(ptr, str, len, hsh) {}
	AttrProxySnh(AttrProxySnh const &right) DEE_CXX_NOTHROW
		: AttrProxySnhBase(static_cast<AttrProxySnhBase const &>(right)) {}
};

template<class GetAttrType = Object, class CallReturnType = Object>
class AttrProxyStn
	: public AttrProxyStnBase
	, public ConstGetAndSetRefProxy<AttrProxyStn<GetAttrType, CallReturnType>, GetAttrType>
	, public ConstCallRefProxy<AttrProxyStn<GetAttrType, CallReturnType>, CallReturnType> {
public:
	using ConstGetAndSetRefProxy<AttrProxyStn<GetAttrType, CallReturnType>, GetAttrType>::operator=;
	AttrProxyStn(DeeObject *ptr, char const *str, size_t len) DEE_CXX_NOTHROW
		: AttrProxyStnBase(ptr, str, len) {}
	AttrProxyStn(AttrProxyStn const &right) DEE_CXX_NOTHROW
		: AttrProxyStnBase(static_cast<AttrProxyStnBase const &>(right)) {}
};


class CxxIteratorBase {
protected:
	DREF DeeObject *it_iter; /* [0..1] Underlying iterator. */
	DREF DeeObject *it_next; /* [0..1] Next element (NULL if not queried). */
	~CxxIteratorBase() DEE_CXX_NOTHROW {
		Dee_XDecref(it_iter);
		if (ITER_ISOK(it_next))
			Dee_Decref(it_next);
	}
	CxxIteratorBase() DEE_CXX_NOTHROW
		: it_iter(NULL)
		, it_next(NULL) {}
	CxxIteratorBase(CxxIteratorBase &&right) DEE_CXX_NOTHROW
		: it_iter(right.it_iter)
		, it_next(right.it_next) {
		right.it_iter = NULL;
		right.it_next = NULL;
	}
	CxxIteratorBase(CxxIteratorBase const &right) DEE_CXX_NOTHROW
		: it_iter(xincref(right.it_iter))
		, it_next(right.it_next) {
		if (ITER_ISOK(it_next))
			Dee_Incref(it_next);
	}
	CxxIteratorBase(DeeObject *iter)
		: it_iter(throw_if_null(iter))
		, it_next(throw_if_null(DeeObject_IterNext(iter))) {
		Dee_Incref(it_iter);
	}
	template<class S> CxxIteratorBase(ObjInherited<S> iter)
		: it_iter(throw_if_null(iter))
		, it_next(DeeObject_IterNext(iter)) {
		if unlikely(!it_next) {
			Dee_Decref(it_iter);
			throw_last_deemon_exception();
		}
	}
	template<class S> CxxIteratorBase(ObjNonNull<S> iter)
		: it_iter(iter)
		, it_next(throw_if_null(DeeObject_IterNext(iter))) {
		Dee_Incref(it_iter);
	}
	template<class S> CxxIteratorBase(ObjNonNullInherited<S> iter)
		: it_iter(iter)
		, it_next(DeeObject_IterNext(iter)) {
		if unlikely(!it_next) {
			Dee_Decref((DeeObject *)it_iter);
			throw_last_deemon_exception();
		}
	}

	WUNUSED DREF DeeObject *_copy() const {
		return DeeObject_Copy(this->it_iter);
	}
	WUNUSED DREF DeeObject *_deepcopy() const {
		return DeeObject_DeepCopy(this->it_iter);
	}
	CxxIteratorBase &inplace_deepcopy() {
		throw_if_nonzero(DeeObject_InplaceDeepCopy(&this->it_iter));
		return *this;
	}
	CxxIteratorBase &inc() {
		if (ITER_ISOK(it_next))
			Dee_Decref(it_next);
		it_next = throw_if_null(DeeObject_IterNext(this->it_iter));
		return *this;
	}
	ATTR_RETNONNULL WUNUSED DREF DeeObject *postinc() {
		DREF DeeObject *result;
		result = throw_if_null(DeeObject_Copy(this->it_iter));
		if (ITER_ISOK(it_next))
			Dee_Decref(it_next);
		it_next = DeeObject_IterNext(this->it_iter);
		if unlikely(!it_next) {
			Dee_Decref(result);
			throw_last_deemon_exception();
		}
		return result;
	}
public:
	bool operator==(CxxIteratorBase const &right) const {
		return !right.it_iter
		       ? it_next == ITER_DONE
		       : throw_if_negative(DeeObject_CmpEqAsBool(it_iter, right.it_iter)) != 0;
	}
	bool operator!=(CxxIteratorBase const &right) const {
		return !right.it_iter
		       ? it_next != ITER_DONE
		       : throw_if_negative(DeeObject_CmpNeAsBool(it_iter, right.it_iter)) != 0;
	}
	bool operator<(CxxIteratorBase const &right) const {
		return !right.it_iter
		       ? it_next != ITER_DONE
		       : throw_if_negative(DeeObject_CmpLoAsBool(it_iter, right.it_iter)) != 0;
	}
	bool operator<=(CxxIteratorBase const &right) const {
		return !right.it_iter
		       ? true
		       : throw_if_negative(DeeObject_CmpLeAsBool(it_iter, right.it_iter)) != 0;
	}
	bool operator>(CxxIteratorBase const &right) const {
		return !right.it_iter
		       ? false
		       : throw_if_negative(DeeObject_CmpGrAsBool(it_iter, right.it_iter)) != 0;
	}
	bool operator>=(CxxIteratorBase const &right) const {
		return !right.it_iter
		       ? it_next == ITER_DONE
		       : throw_if_negative(DeeObject_CmpGeAsBool(it_iter, right.it_iter)) != 0;
	}
	CxxIteratorBase &operator=(CxxIteratorBase &&right) DEE_CXX_NOTHROW {
		Dee_XDecref(it_iter);
		if (ITER_ISOK(it_next))
			Dee_Decref(it_next);
		it_iter       = right.it_iter;
		it_next       = right.it_next;
		right.it_iter = NULL;
		right.it_next = NULL;
		return *this;
	}
	CxxIteratorBase &operator=(CxxIteratorBase const &right) {
		Dee_XIncref(right.it_iter);
		if (ITER_ISOK(right.it_next))
			Dee_Incref(right.it_next);
		Dee_XDecref(it_iter);
		if (ITER_ISOK(it_next))
			Dee_Decref(it_next);
		it_iter = right.it_iter;
		it_next = right.it_next;
		return *this;
	}
	WUNUSED operator DeeObject *() const DEE_CXX_NOTHROW {
		return it_next;
	}
};

template<class GetItemType = Object>
class CxxIterator: public CxxIteratorBase {
public:
	CxxIterator() DEE_CXX_NOTHROW
		: CxxIteratorBase() {}
	CxxIterator(CxxIterator<GetItemType> &&right) DEE_CXX_NOTHROW
		: CxxIteratorBase(static_cast<CxxIteratorBase &&>(right)) {}
	CxxIterator(CxxIterator const &right) DEE_CXX_NOTHROW
		: CxxIteratorBase(static_cast<CxxIteratorBase const &>(right)) {}
	CxxIterator(DeeObject *iter)
		: CxxIteratorBase(iter) {}
	template<class S> CxxIterator(ObjInherited<S> iter)
		: CxxIteratorBase(iter) {}
	template<class S> CxxIterator(ObjNonNull<S> iter)
		: CxxIteratorBase(iter) {}
	template<class S> CxxIterator(ObjNonNullInherited<S> iter)
		: CxxIteratorBase(iter) {}

	CxxIterator<GetItemType> &operator=(CxxIterator<GetItemType> &&right) DEE_CXX_NOTHROW {
		return (CxxIterator<GetItemType> &)CxxIteratorBase::operator=(static_cast<CxxIteratorBase &&>(right));
	}
	CxxIterator<GetItemType> &operator=(CxxIterator<GetItemType> const &right) DEE_CXX_NOTHROW {
		return (CxxIterator<GetItemType> &)CxxIteratorBase::operator=(static_cast<CxxIteratorBase const &>(right));
	}
	CxxIterator<GetItemType> copy() const {
		return inherit(CxxIteratorBase::_copy());
	}
	CxxIterator<GetItemType> deepcopy() const {
		return inherit(CxxIteratorBase::_deepcopy());
	}
	CxxIterator<GetItemType> &inplace_deepcopy() {
		return (CxxIterator<GetItemType> &)CxxIteratorBase::inplace_deepcopy();
	}
	WUNUSED operator GetItemType *() const DEE_CXX_NOTHROW {
		return (GetItemType *)it_next;
	}
	WUNUSED GetItemType *operator->() const DEE_CXX_NOTHROW {
		return (GetItemType *)it_next;
	}
	WUNUSED GetItemType &operator*() const DEE_CXX_NOTHROW {
		return *(GetItemType *)it_next;
	}
	WUNUSED GetItemType &get() const DEE_CXX_NOTHROW {
		return *(GetItemType *)it_next;
	}
	CxxIterator<GetItemType> &operator++() {
		return (CxxIterator<GetItemType> &)CxxIteratorBase::inc();
	}
	CxxIterator<GetItemType> operator++(int) {
		return inherit(nonnull(CxxIteratorBase::postinc()));
	}
	Iterator<GetItemType> &iter() const DEE_CXX_NOTHROW {
		return *(Iterator<GetItemType> *)it_iter;
	}
};

class ItemProxyObjBase {
private:
	DeeObject *m_ptr;
	DeeObject *m_idx;
public:
	ItemProxyObjBase(DeeObject *ptr, DeeObject *idx) DEE_CXX_NOTHROW
		: m_ptr(ptr)
		, m_idx(idx) {}
	ItemProxyObjBase(ItemProxyObjBase const &right) DEE_CXX_NOTHROW
		: m_ptr(right.m_ptr)
		, m_idx(right.m_idx) {}
	WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
		return DeeObject_GetItem(m_ptr, m_idx);
	}
	WUNUSED DREF DeeObject *_trygetref() const DEE_CXX_NOTHROW {
		return DeeObject_TryGetItem(m_ptr, m_idx);
	}
	WUNUSED int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
		return DeeObject_SetItem(m_ptr, m_idx, value);
	}
	bool has() const {
		return throw_if_negative(DeeObject_HasItem(m_ptr, m_idx)) != 0;
	}
	bool bound() const {
		return throw_if_minusone(DeeObject_BoundItem(m_ptr, m_idx)) > 0;
	}
	void del() const {
		throw_if_nonzero(DeeObject_DelItem(m_ptr, m_idx));
	}
};

class ItemProxyIdxBase {
private:
	DeeObject *m_ptr;
	size_t m_idx;
public:
	ItemProxyIdxBase(DeeObject *ptr, size_t idx) DEE_CXX_NOTHROW
		: m_ptr(ptr)
		, m_idx(idx) {}
	ItemProxyIdxBase(ItemProxyIdxBase const &right) DEE_CXX_NOTHROW
		: m_ptr(right.m_ptr)
		, m_idx(right.m_idx) {}
	WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
		return DeeObject_GetItemIndex(m_ptr, m_idx);
	}
	WUNUSED DREF DeeObject *_trygetref() const DEE_CXX_NOTHROW {
		return DeeObject_TryGetItemIndex(m_ptr, m_idx);
	}
	WUNUSED int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
		return DeeObject_SetItemIndex(m_ptr, m_idx, value);
	}
	bool has() const {
		return throw_if_negative(DeeObject_HasItemIndex(m_ptr, m_idx)) != 0;
	}
	bool bound() const {
		return throw_if_minusone(DeeObject_BoundItemIndex(m_ptr, m_idx)) > 0;
	}
	void del() const {
		throw_if_nonzero(DeeObject_DelItemIndex(m_ptr, m_idx));
	}
};

class ItemProxySthBase {
private:
	DeeObject *m_ptr;
	char const *m_key;
	Dee_hash_t m_hsh;
public:
	ItemProxySthBase(DeeObject *ptr, char const *key, Dee_hash_t hsh) DEE_CXX_NOTHROW
		: m_ptr(ptr)
		, m_key(key)
		, m_hsh(hsh) {}
	ItemProxySthBase(ItemProxySthBase const &right) DEE_CXX_NOTHROW
		: m_ptr(right.m_ptr)
		, m_key(right.m_key)
		, m_hsh(right.m_hsh) {}
	WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
		return DeeObject_GetItemStringHash(m_ptr, m_key, m_hsh);
	}
	WUNUSED DREF DeeObject *_trygetref() const DEE_CXX_NOTHROW {
		return DeeObject_TryGetItemStringHash(m_ptr, m_key, m_hsh);
	}
	WUNUSED int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
		return DeeObject_SetItemStringHash(m_ptr, m_key, m_hsh, value);
	}
	bool has() const {
		return throw_if_negative(DeeObject_HasItemStringHash(m_ptr, m_key, m_hsh)) != 0;
	}
	bool bound() const {
		return throw_if_minusone(DeeObject_BoundItemStringHash(m_ptr, m_key, m_hsh)) > 0;
	}
	void del() const {
		throw_if_nonzero(DeeObject_DelItemStringHash(m_ptr, m_key, m_hsh));
	}
};

class ItemProxySnhBase {
private:
	DeeObject *m_ptr;
	char const *m_key;
	size_t m_len;
	Dee_hash_t m_hsh;
public:
	ItemProxySnhBase(DeeObject *ptr, char const *key, size_t len, Dee_hash_t hsh) DEE_CXX_NOTHROW
		: m_ptr(ptr)
		, m_key(key)
		, m_len(len)
		, m_hsh(hsh) {}
	ItemProxySnhBase(ItemProxySnhBase const &right) DEE_CXX_NOTHROW
		: m_ptr(right.m_ptr)
		, m_key(right.m_key)
		, m_len(right.m_len)
		, m_hsh(right.m_hsh) {}
	WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
		return DeeObject_GetItemStringLenHash(m_ptr, m_key, m_len, m_hsh);
	}
	WUNUSED DREF DeeObject *_trygetref() const DEE_CXX_NOTHROW {
		return DeeObject_TryGetItemStringLenHash(m_ptr, m_key, m_len, m_hsh);
	}
	WUNUSED int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
		return DeeObject_SetItemStringLenHash(m_ptr, m_key, m_len, m_hsh, value);
	}
	bool has() const {
		return throw_if_negative(DeeObject_HasItemStringLenHash(m_ptr, m_key, m_len, m_hsh)) != 0;
	}
	bool bound() const {
		return throw_if_minusone(DeeObject_BoundItemStringLenHash(m_ptr, m_key, m_len, m_hsh)) > 0;
	}
	void del() const {
		throw_if_nonzero(DeeObject_DelItemStringLenHash(m_ptr, m_key, m_len, m_hsh));
	}
};

template<class GetItemType = Object>
class ItemProxyObj
	: public ItemProxyObjBase
	, public ConstGetAndSetRefProxyWithDefault<ItemProxyObj<GetItemType>, GetItemType>
{
public:
	using ConstGetAndSetRefProxyWithDefault<ItemProxyObj<GetItemType>, GetItemType>::operator=;
	ItemProxyObj(DeeObject *ptr, DeeObject *idx) DEE_CXX_NOTHROW
		: ItemProxyObjBase(ptr, idx) {}
	ItemProxyObj(ItemProxyObj const &right) DEE_CXX_NOTHROW
		: ItemProxyObjBase(static_cast<ItemProxyObjBase const &>(right)) {}
};

template<class GetItemType = Object>
class ItemProxyIdx
	: public ItemProxyIdxBase
	, public ConstGetAndSetRefProxyWithDefault<ItemProxyIdx<GetItemType>, GetItemType>
{
public:
	using ConstGetAndSetRefProxyWithDefault<ItemProxyIdx<GetItemType>, GetItemType>::operator=;
	ItemProxyIdx(DeeObject *ptr, size_t idx) DEE_CXX_NOTHROW
		: ItemProxyIdxBase(ptr, idx) {}
	ItemProxyIdx(ItemProxyIdx const &right) DEE_CXX_NOTHROW
		: ItemProxyIdxBase(static_cast<ItemProxyIdxBase const &>(right)) {}
};

template<class GetItemType = Object>
class ItemProxySth
	: public ItemProxySthBase
	, public ConstGetAndSetRefProxyWithDefault<ItemProxySth<GetItemType>, GetItemType>
{
public:
	using ConstGetAndSetRefProxyWithDefault<ItemProxySth<GetItemType>, GetItemType>::operator=;
	ItemProxySth(DeeObject *ptr, char const *key, Dee_hash_t hsh) DEE_CXX_NOTHROW
		: ItemProxySthBase(ptr, key, hsh) {}
	ItemProxySth(ItemProxySth const &right) DEE_CXX_NOTHROW
		: ItemProxySthBase(static_cast<ItemProxySthBase const &>(right)) {}
};

template<class GetItemType = Object>
class ItemProxySnh
	: public ItemProxySnhBase
	, public ConstGetAndSetRefProxyWithDefault<ItemProxySnh<GetItemType>, GetItemType>
{
public:
	using ConstGetAndSetRefProxyWithDefault<ItemProxySnh<GetItemType>, GetItemType>::operator=;
	ItemProxySnh(DeeObject *ptr, char const *key, size_t len, Dee_hash_t hsh) DEE_CXX_NOTHROW
		: ItemProxySnhBase(ptr, key, len, hsh) {}
	ItemProxySnh(ItemProxySnh const &right) DEE_CXX_NOTHROW
		: ItemProxySnhBase(static_cast<ItemProxySnhBase const &>(right)) {}
};

class RangeProxyObjBase {
private:
	DeeObject *m_ptr;
	DeeObject *m_bgn;
	DeeObject *m_end;
public:
	RangeProxyObjBase(DeeObject *ptr, DeeObject *bgn, DeeObject *end) DEE_CXX_NOTHROW
		: m_ptr(ptr)
		, m_bgn(bgn)
		, m_end(end) {}
	RangeProxyObjBase(RangeProxyObjBase const &right) DEE_CXX_NOTHROW
		: m_ptr(right.m_ptr)
		, m_bgn(right.m_bgn)
		, m_end(right.m_end) {}
	WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
		return DeeObject_GetRange(m_ptr, m_bgn, m_end);
	}
	WUNUSED int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
		return DeeObject_SetRange(m_ptr, m_bgn, m_end, value);
	}
	void del() const {
		throw_if_nonzero(DeeObject_DelRange(m_ptr, m_bgn, m_end));
	}
};

class RangeProxyIdxObjBase {
private:
	DeeObject *m_ptr;
	Dee_ssize_t m_bgn;
	DeeObject *m_end;
public:
	RangeProxyIdxObjBase(DeeObject *ptr, Dee_ssize_t bgn, DeeObject *end) DEE_CXX_NOTHROW
		: m_ptr(ptr)
		, m_bgn(bgn)
		, m_end(end) {}
	RangeProxyIdxObjBase(RangeProxyIdxObjBase const &right) DEE_CXX_NOTHROW
		: m_ptr(right.m_ptr)
		, m_bgn(right.m_bgn)
		, m_end(right.m_end) {}
	WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
		return DeeObject_GetRangeBeginIndex(m_ptr, m_bgn, m_end);
	}
	WUNUSED int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
		return DeeObject_SetRangeBeginIndex(m_ptr, m_bgn, m_end, value);
	}
	void del() const {
		throw_if_nonzero(DeeObject_DelRangeBeginIndex(m_ptr, m_bgn, m_end));
	}
};

class RangeProxyObjIdxBase {
private:
	DeeObject *m_ptr;
	DeeObject *m_bgn;
	Dee_ssize_t m_end;
public:
	RangeProxyObjIdxBase(DeeObject *ptr, DeeObject *bgn, Dee_ssize_t end) DEE_CXX_NOTHROW
		: m_ptr(ptr)
		, m_bgn(bgn)
		, m_end(end) {}
	RangeProxyObjIdxBase(RangeProxyObjIdxBase const &right) DEE_CXX_NOTHROW
		: m_ptr(right.m_ptr)
		, m_bgn(right.m_bgn)
		, m_end(right.m_end) {}
	WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
		return DeeObject_GetRangeEndIndex(m_ptr, m_bgn, m_end);
	}
	WUNUSED int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
		return DeeObject_SetRangeEndIndex(m_ptr, m_bgn, m_end, value);
	}
	void del() const {
		throw_if_nonzero(DeeObject_DelRangeEndIndex(m_ptr, m_bgn, m_end));
	}
};

class RangeProxyIdxBase {
private:
	DeeObject *m_ptr;
	Dee_ssize_t m_bgn;
	Dee_ssize_t m_end;
public:
	RangeProxyIdxBase(DeeObject *ptr, Dee_ssize_t bgn, Dee_ssize_t end) DEE_CXX_NOTHROW
		: m_ptr(ptr)
		, m_bgn(bgn)
		, m_end(end) {}
	RangeProxyIdxBase(RangeProxyIdxBase const &right) DEE_CXX_NOTHROW
		: m_ptr(right.m_ptr)
		, m_bgn(right.m_bgn)
		, m_end(right.m_end) {}
	WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
		return DeeObject_GetRangeIndex(m_ptr, m_bgn, m_end);
	}
	WUNUSED int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
		return DeeObject_SetRangeIndex(m_ptr, m_bgn, m_end, value);
	}
	void del() const {
		throw_if_nonzero(DeeObject_DelRangeIndex(m_ptr, m_bgn, m_end));
	}
};

template<class RangeSeqType = Object>
class RangeProxyObj
	: public RangeProxyObjBase
	, public ConstGetAndSetRefProxyWithDefault<RangeProxyObj<RangeSeqType>, RangeSeqType>
{
public:
	using ConstGetAndSetRefProxyWithDefault<RangeProxyObj<RangeSeqType>, RangeSeqType>::operator=;
	RangeProxyObj(DeeObject *ptr, DeeObject *bgn, DeeObject *end) DEE_CXX_NOTHROW
		: RangeProxyObjBase(ptr, bgn, end) {}
	RangeProxyObj(RangeProxyObj const &right) DEE_CXX_NOTHROW
		: RangeProxyObjBase(static_cast<RangeProxyObjBase const &>(right)) {}
};

template<class RangeSeqType = Object>
class RangeProxyIdxObj
	: public RangeProxyIdxObjBase
	, public ConstGetAndSetRefProxyWithDefault<RangeProxyIdxObj<RangeSeqType>, RangeSeqType>
{
public:
	using ConstGetAndSetRefProxyWithDefault<RangeProxyIdxObj<RangeSeqType>, RangeSeqType>::operator=;
	RangeProxyIdxObj(DeeObject *ptr, Dee_ssize_t bgn, DeeObject *end) DEE_CXX_NOTHROW
		: RangeProxyIdxObjBase(ptr, bgn, end) {}
	RangeProxyIdxObj(RangeProxyIdxObj const &right) DEE_CXX_NOTHROW
		: RangeProxyIdxObjBase(static_cast<RangeProxyIdxObjBase const &>(right)) {}
};

template<class RangeSeqType = Object>
class RangeProxyObjIdx
	: public RangeProxyObjIdxBase
	, public ConstGetAndSetRefProxyWithDefault<RangeProxyObjIdx<RangeSeqType>, RangeSeqType>
{
public:
	using ConstGetAndSetRefProxyWithDefault<RangeProxyObjIdx<RangeSeqType>, RangeSeqType>::operator=;
	RangeProxyObjIdx(DeeObject *ptr, DeeObject *bgn, Dee_ssize_t end) DEE_CXX_NOTHROW
		: RangeProxyObjIdxBase(ptr, bgn, end) {}
	RangeProxyObjIdx(RangeProxyObjIdx const &right) DEE_CXX_NOTHROW
		: RangeProxyObjIdxBase(static_cast<RangeProxyObjIdxBase const &>(right)) {}
};

template<class RangeSeqType = Object>
class RangeProxyIdx
	: public RangeProxyIdxBase
	, public ConstGetAndSetRefProxyWithDefault<RangeProxyIdx<RangeSeqType>, RangeSeqType>
{
public:
	using ConstGetAndSetRefProxyWithDefault<RangeProxyIdx<RangeSeqType>, RangeSeqType>::operator=;
	RangeProxyIdx(DeeObject *ptr, Dee_ssize_t bgn, Dee_ssize_t end) DEE_CXX_NOTHROW
		: RangeProxyIdxBase(ptr, bgn, end) {}
	RangeProxyIdx(RangeProxyIdx const &right) DEE_CXX_NOTHROW
		: RangeProxyIdxBase(static_cast<RangeProxyIdxBase const &>(right)) {}
};

template<class ProxyType,
         class GetAttrType = Object,
         class CallReturnType = Object>
class AttrProxyAccessor {
public:
	NONNULL_CXX((1)) AttrProxyObj<GetAttrType, CallReturnType> attr(string *name) {
		return AttrProxyObj<GetAttrType, CallReturnType>(((ProxyType *)this)->ptr(), (DeeObject *)name);
	}
	NONNULL_CXX((1)) AttrProxyStr<GetAttrType, CallReturnType> attr(char const *name) {
		return AttrProxyStr<GetAttrType, CallReturnType>(((ProxyType *)this)->ptr(), name);
	}
	NONNULL_CXX((1)) AttrProxySth<GetAttrType, CallReturnType> attr(char const *name, Dee_hash_t hash) {
		return AttrProxySth<GetAttrType, CallReturnType>(((ProxyType *)this)->ptr(), name, hash);
	}
	NONNULL_CXX((1)) AttrProxySnh<GetAttrType, CallReturnType> attr(char const *name, size_t namelen, Dee_hash_t hash) {
		return AttrProxySnh<GetAttrType, CallReturnType>(((ProxyType *)this)->ptr(), name, namelen, hash);
	}
	NONNULL_CXX((1)) Ref<GetAttrType> getattr(string *name) {
		return inherit(DeeObject_GetAttr(((ProxyType *)this)->ptr(), (DeeObject *)name));
	}
	NONNULL_CXX((1)) Ref<GetAttrType> getattr(char const *name) {
		return inherit(DeeObject_GetAttrString(((ProxyType *)this)->ptr(), name));
	}
	NONNULL_CXX((1)) Ref<GetAttrType> getattr(char const *name, Dee_hash_t hash) {
		return inherit(DeeObject_GetAttrStringHash(((ProxyType *)this)->ptr(), name, hash));
	}
	NONNULL_CXX((1)) Ref<GetAttrType> getattr(char const *name, size_t namelen, Dee_hash_t hash) {
		return inherit(DeeObject_GetAttrStringLenHash(((ProxyType *)this)->ptr(), name, namelen, hash));
	}
	NONNULL_CXX((1)) bool hasattr(string *name) {
		return throw_if_negative(DeeObject_HasAttr(((ProxyType *)this)->ptr(), name)) != 0;
	}
	NONNULL_CXX((1)) bool hasattr(char const *__restrict name) {
		return throw_if_negative(DeeObject_HasAttrString(((ProxyType *)this)->ptr(), name)) != 0;
	}
	NONNULL_CXX((1)) bool hasattr(char const *__restrict name, Dee_hash_t hash) {
		return throw_if_negative(DeeObject_HasAttrStringHash(((ProxyType *)this)->ptr(), name, hash)) != 0;
	}
	NONNULL_CXX((1)) bool hasattr(char const *__restrict name, size_t namelen, Dee_hash_t hash) {
		return throw_if_negative(DeeObject_HasAttrStringLenHash(((ProxyType *)this)->ptr(), name, namelen, hash)) != 0;
	}
	NONNULL_CXX((1)) bool boundattr(string *name) {
		return throw_if_minusone(DeeObject_BoundAttr(((ProxyType *)this)->ptr(), name));
	}
	NONNULL_CXX((1)) bool boundattr(char const *__restrict name) {
		return throw_if_minusone(DeeObject_BoundAttrString(((ProxyType *)this)->ptr(), name));
	}
	NONNULL_CXX((1)) bool boundattr(char const *__restrict name, Dee_hash_t hash) {
		return throw_if_minusone(DeeObject_BoundAttrStringHash(((ProxyType *)this)->ptr(), name, hash));
	}
	NONNULL_CXX((1)) bool boundattr(char const *__restrict name, size_t namelen, Dee_hash_t hash) {
		return throw_if_minusone(DeeObject_BoundAttrStringLenHash(((ProxyType *)this)->ptr(), name, namelen, hash));
	}
	NONNULL_CXX((1)) void delattr(string *name) {
		throw_if_negative(DeeObject_DelAttr(((ProxyType *)this)->ptr(), name));
	}
	NONNULL_CXX((1)) void delattr(char const *__restrict name) {
		throw_if_negative(DeeObject_DelAttrString(((ProxyType *)this)->ptr(), name));
	}
	NONNULL_CXX((1)) void delattr(char const *__restrict name, Dee_hash_t hash) {
		throw_if_negative(DeeObject_DelAttrStringHash(((ProxyType *)this)->ptr(), name, hash));
	}
	NONNULL_CXX((1)) void delattr(char const *__restrict name, size_t namelen, Dee_hash_t hash) {
		throw_if_negative(DeeObject_DelAttrStringLenHash(((ProxyType *)this)->ptr(), name, namelen, hash));
	}
	NONNULL_CXX((1)) void setattr(string *name, DeeObject *value) {
		throw_if_negative(DeeObject_SetAttr(((ProxyType *)this)->ptr(), name, value));
	}
	NONNULL_CXX((1)) void setattr(char const *__restrict name, DeeObject *value) {
		throw_if_negative(DeeObject_SetAttrString(((ProxyType *)this)->ptr(), name, value));
	}
	NONNULL_CXX((1)) void setattr(char const *__restrict name, Dee_hash_t hash, DeeObject *value) {
		throw_if_negative(DeeObject_SetAttrStringHash(((ProxyType *)this)->ptr(), name, hash, value));
	}
	NONNULL_CXX((1)) void setattr(char const *__restrict name, size_t namelen, Dee_hash_t hash, DeeObject *value) {
		throw_if_negative(DeeObject_SetAttrStringLenHash(((ProxyType *)this)->ptr(), name, namelen, hash, value));
	}

	NONNULL_CXX((1)) Ref<CallReturnType> callattr(string *name) {
		return inherit(DeeObject_CallAttr(((ProxyType *)this)->ptr(), (DeeObject *)name, 0, NULL));
	}
	template<class ...ArgTypes> NONNULL_CXX((1))
	Ref<CallReturnType> callattr(string *name, Tuple<ArgTypes...> const *args) {
		return inherit(DeeObject_CallAttrTuple(((ProxyType *)this)->ptr(), (DeeObject *)name, (DeeObject *)args));
	}
	template<class KwTypePtr, class ...ArgTypes> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_PTR_T(KwTypePtr, Ref<CallReturnType>) callattr(string *name, Tuple<ArgTypes...> const *args, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrTupleKw(((ProxyType *)this)->ptr(), (DeeObject *)name, (DeeObject *)args, (DeeObject *)kw));
	}
	template<class ArgType> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<CallReturnType>) callattr(string *name, size_t argc, ArgType *const *argv) {
		return inherit(DeeObject_CallAttr(((ProxyType *)this)->ptr(), (DeeObject *)name, argc, (DeeObject *const *)argv));
	}
	template<class ArgType> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<CallReturnType>) callattr(string *name, size_t argc, ArgType **argv) {
		return inherit(DeeObject_CallAttr(((ProxyType *)this)->ptr(), (DeeObject *)name, argc, (DeeObject *const *)argv));
	}
	template<class ArgType, class KwTypePtr> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_AND_PTR_T(ArgType, KwTypePtr, Ref<CallReturnType>) callattr(string *name, size_t argc, ArgType *const *argv, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrKw(((ProxyType *)this)->ptr(), (DeeObject *)name, argc, (DeeObject *const *)argv, (DeeObject *)kw));
	}
	template<class ArgType, class KwTypePtr> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_AND_PTR_T(ArgType, KwTypePtr, Ref<CallReturnType>) callattr(string *name, size_t argc, ArgType **argv, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrKw(((ProxyType *)this)->ptr(), (DeeObject *)name, argc, (DeeObject *const *)argv, (DeeObject *)kw));
	}
	template<class ArgType> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_PTR_T(ArgType, Ref<CallReturnType>) callattr(string *name, std::initializer_list<ArgType> const &args) {
		return inherit(DeeObject_CallAttr(((ProxyType *)this)->ptr(), (DeeObject *)name, args.size(), (DeeObject *const *)args.begin()));
	}
	template<class ArgType, class KwTypePtr> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_PTR2_T(ArgType, KwTypePtr, Ref<CallReturnType>) callattr(string *name, std::initializer_list<ArgType> const &args, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrKw(((ProxyType *)this)->ptr(), (DeeObject *)name, args.size(), (DeeObject *const *)args.begin(), (DeeObject *)kw));
	}

	NONNULL_CXX((1)) Ref<CallReturnType> callattr(char const *__restrict name) {
		return inherit(DeeObject_CallAttrString(((ProxyType *)this)->ptr(), name, 0, NULL));
	}
	template<class ...ArgTypes> NONNULL_CXX((1))
	Ref<CallReturnType> callattr(char const *__restrict name, Tuple<ArgTypes...> const *args) {
		return inherit(DeeObject_CallAttrStringTuple(((ProxyType *)this)->ptr(), name, (DeeObject *)args));
	}
	template<class KwTypePtr, class ...ArgTypes> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_PTR_T(KwTypePtr, Ref<CallReturnType>) callattr(char const *__restrict name, Tuple<ArgTypes...> const *args, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrStringTupleKw(((ProxyType *)this)->ptr(), name, (DeeObject *)args, (DeeObject *)kw));
	}
	template<class ArgType> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<CallReturnType>) callattr(char const *__restrict name, size_t argc, ArgType *const *argv) {
		return inherit(DeeObject_CallAttrString(((ProxyType *)this)->ptr(), name, argc, (DeeObject *const *)argv));
	}
	template<class ArgType> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<CallReturnType>) callattr(char const *__restrict name, size_t argc, ArgType **argv) {
		return inherit(DeeObject_CallAttrString(((ProxyType *)this)->ptr(), name, argc, (DeeObject *const *)argv));
	}
	template<class ArgType, class KwTypePtr> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_AND_PTR_T(ArgType, KwTypePtr, Ref<CallReturnType>) callattr(char const *__restrict name, size_t argc, ArgType *const *argv, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrStringKw(((ProxyType *)this)->ptr(), name, argc, (DeeObject *const *)argv, (DeeObject *)kw));
	}
	template<class ArgType, class KwTypePtr> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_AND_PTR_T(ArgType, KwTypePtr, Ref<CallReturnType>) callattr(char const *__restrict name, size_t argc, ArgType **argv, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrStringKw(((ProxyType *)this)->ptr(), name, argc, (DeeObject *const *)argv, (DeeObject *)kw));
	}
	template<class ArgType> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_PTR_T(ArgType, Ref<CallReturnType>) callattr(char const *__restrict name, std::initializer_list<ArgType> const &args) {
		return inherit(DeeObject_CallAttrString(((ProxyType *)this)->ptr(), name, args.size(), (DeeObject *const *)args.begin()));
	}
	template<class ArgType, class KwTypePtr> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_PTR2_T(ArgType, KwTypePtr, Ref<CallReturnType>) callattr(char const *__restrict name, std::initializer_list<ArgType> const &args, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrStringKw(((ProxyType *)this)->ptr(), name, args.size(), (DeeObject *const *)args.begin(), (DeeObject *)kw));
	}

	NONNULL_CXX((1)) Ref<CallReturnType> callattr(char const *__restrict name, Dee_hash_t hash) {
		return inherit(DeeObject_CallAttrStringHash(((ProxyType *)this)->ptr(), name, hash, 0, NULL));
	}
	template<class ...ArgTypes> NONNULL_CXX((1))
	Ref<CallReturnType> callattr(char const *__restrict name, Dee_hash_t hash, Tuple<ArgTypes...> const *args) {
		return inherit(DeeObject_CallAttrStringHashTuple(((ProxyType *)this)->ptr(), name, hash, (DeeObject *)args));
	}
	template<class KwTypePtr, class ...ArgTypes> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_PTR_T(KwTypePtr, Ref<CallReturnType>) callattr(char const *__restrict name, Dee_hash_t hash, Tuple<ArgTypes...> const *args, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrStringHashTupleKw(((ProxyType *)this)->ptr(), name, hash, (DeeObject *)args, (DeeObject *)kw));
	}
	template<class ArgType> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<CallReturnType>) callattr(char const *__restrict name, Dee_hash_t hash, size_t argc, ArgType *const *argv) {
		return inherit(DeeObject_CallAttrStringHash(((ProxyType *)this)->ptr(), name, hash, argc, (DeeObject *const *)argv));
	}
	template<class ArgType> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<CallReturnType>) callattr(char const *__restrict name, Dee_hash_t hash, size_t argc, ArgType **argv) {
		return inherit(DeeObject_CallAttrStringHash(((ProxyType *)this)->ptr(), name, hash, argc, (DeeObject *const *)argv));
	}
	template<class ArgType, class KwTypePtr> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_AND_PTR_T(ArgType, KwTypePtr, Ref<CallReturnType>) callattr(char const *__restrict name, Dee_hash_t hash, size_t argc, ArgType *const *argv, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrStringHashKw(((ProxyType *)this)->ptr(), name, hash, argc, (DeeObject *const *)argv, (DeeObject *)kw));
	}
	template<class ArgType, class KwTypePtr> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_AND_PTR_T(ArgType, KwTypePtr, Ref<CallReturnType>) callattr(char const *__restrict name, Dee_hash_t hash, size_t argc, ArgType **argv, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrStringHashKw(((ProxyType *)this)->ptr(), name, hash, argc, (DeeObject *const *)argv, (DeeObject *)kw));
	}
	template<class ArgType> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_PTR_T(ArgType, Ref<CallReturnType>) callattr(char const *__restrict name, Dee_hash_t hash, std::initializer_list<ArgType> const &args) {
		return inherit(DeeObject_CallAttrStringHash(((ProxyType *)this)->ptr(), name, hash, args.size(), (DeeObject *const *)args.begin()));
	}
	template<class ArgType, class KwTypePtr> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_PTR2_T(ArgType, KwTypePtr, Ref<CallReturnType>) callattr(char const *__restrict name, Dee_hash_t hash, std::initializer_list<ArgType> const &args, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrStringHashKw(((ProxyType *)this)->ptr(), name, hash, args.size(), (DeeObject *const *)args.begin(), (DeeObject *)kw));
	}

	NONNULL_CXX((1)) Ref<CallReturnType> callattr(char const *__restrict name, size_t namelen, Dee_hash_t hash) {
		return inherit(DeeObject_CallAttrStringLenHash(((ProxyType *)this)->ptr(), name, namelen, hash, 0, NULL));
	}
	template<class ...ArgTypes> NONNULL_CXX((1))
	Ref<CallReturnType> callattr(char const *__restrict name, size_t namelen, Dee_hash_t hash, Tuple<ArgTypes...> const *args) {
		return inherit(DeeObject_CallAttrStringLenHashTuple(((ProxyType *)this)->ptr(), name, namelen, hash, (DeeObject *)args));
	}
	template<class KwTypePtr, class ...ArgTypes> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_PTR_T(KwTypePtr, Ref<CallReturnType>) callattr(char const *__restrict name, size_t namelen, Dee_hash_t hash, Tuple<ArgTypes...> const *args, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrStringLenHashTupleKw(((ProxyType *)this)->ptr(), name, namelen, hash, (DeeObject *)args, (DeeObject *)kw));
	}
	template<class ArgType> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<CallReturnType>) callattr(char const *__restrict name, size_t namelen, Dee_hash_t hash, size_t argc, ArgType *const *argv) {
		return inherit(DeeObject_CallAttrStringLenHash(((ProxyType *)this)->ptr(), name, namelen, hash, argc, (DeeObject *const *)argv));
	}
	template<class ArgType> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<CallReturnType>) callattr(char const *__restrict name, size_t namelen, Dee_hash_t hash, size_t argc, ArgType **argv) {
		return inherit(DeeObject_CallAttrStringLenHash(((ProxyType *)this)->ptr(), name, namelen, hash, argc, (DeeObject *const *)argv));
	}
	template<class ArgType, class KwTypePtr> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_AND_PTR_T(ArgType, KwTypePtr, Ref<CallReturnType>) callattr(char const *__restrict name, size_t namelen, Dee_hash_t hash, size_t argc, ArgType *const *argv, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrStringLenHashKw(((ProxyType *)this)->ptr(), name, namelen, hash, argc, (DeeObject *const *)argv, (DeeObject *)kw));
	}
	template<class ArgType, class KwTypePtr> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_AND_PTR_T(ArgType, KwTypePtr, Ref<CallReturnType>) callattr(char const *__restrict name, size_t namelen, Dee_hash_t hash, size_t argc, ArgType **argv, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrStringLenHashKw(((ProxyType *)this)->ptr(), name, namelen, hash, argc, (DeeObject *const *)argv, (DeeObject *)kw));
	}
	template<class ArgType> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_PTR_T(ArgType, Ref<CallReturnType>) callattr(char const *__restrict name, size_t namelen, Dee_hash_t hash, std::initializer_list<ArgType> const &args) {
		return inherit(DeeObject_CallAttrStringLenHash(((ProxyType *)this)->ptr(), name, namelen, hash, args.size(), (DeeObject *const *)args.begin()));
	}
	template<class ArgType, class KwTypePtr> NONNULL_CXX((1))
	DEE_ENABLE_IF_OBJECT_PTR2_T(ArgType, KwTypePtr, Ref<CallReturnType>) callattr(char const *__restrict name, size_t namelen, Dee_hash_t hash, std::initializer_list<ArgType> const &args, KwTypePtr kw) {
		return inherit(DeeObject_CallAttrStringLenHashKw(((ProxyType *)this)->ptr(), name, namelen, hash, args.size(), (DeeObject *const *)args.begin(), (DeeObject *)kw));
	}

};

template<class ProxyType,
         class GetItemType = Object>
class ItemProxyAccessor {
public:
	typedef CxxIterator<GetItemType> iterator;
	Ref<Iterator<GetItemType> > iter() DEE_CXX_NOTHROW {
		return inherit(DeeObject_Iter(((ProxyType *)this)->ptr()));
	}
	WUNUSED CxxIterator<GetItemType> begin() const {
		return inherit(DeeObject_Iter(((ProxyType *)this)->ptr()));
	}
	WUNUSED CxxIterator<GetItemType> end() const {
		return CxxIterator<GetItemType>();
	}

	NONNULL_CXX((1)) ItemProxyObj<GetItemType> operator[](DeeObject *index_or_key) {
		return ItemProxyObj<GetItemType>(((ProxyType *)this)->ptr(), index_or_key);
	}
	ItemProxyIdx<GetItemType> operator[](size_t index) {
		return ItemProxyIdx<GetItemType>(((ProxyType *)this)->ptr(), index);
	}
	NONNULL_CXX((1)) ItemProxySth<GetItemType> operator[](char const *key) {
		return ItemProxySth<GetItemType>(((ProxyType *)this)->ptr(), key, Dee_HashStr(key));
	}

	NONNULL_CXX((1)) ItemProxyObj<GetItemType> item(DeeObject *index_or_key) {
		return ItemProxyObj<GetItemType>(((ProxyType *)this)->ptr(), index_or_key);
	}
	ItemProxyIdx<GetItemType> item(size_t index) {
		return ItemProxyIdx<GetItemType>(((ProxyType *)this)->ptr(), index);
	}
	NONNULL_CXX((1)) ItemProxySth<GetItemType> item(char const *key) {
		return ItemProxySth<GetItemType>(((ProxyType *)this)->ptr(), key, Dee_HashStr(key));
	}
	NONNULL_CXX((1)) ItemProxySth<GetItemType> item(char const *key, Dee_hash_t hash) {
		return ItemProxySth<GetItemType>(((ProxyType *)this)->ptr(), key, hash);
	}
	NONNULL_CXX((1)) ItemProxySnh<GetItemType> item(char const *key, size_t keylen, Dee_hash_t hash) {
		return ItemProxySnh<GetItemType>(((ProxyType *)this)->ptr(), key, keylen, hash);
	}

	NONNULL_CXX((1)) DREF DeeObject *_getitem(DeeObject *index_or_key) {
		return DeeObject_GetItem(((ProxyType *)this)->ptr(), index_or_key);
	}
	NONNULL_CXX((1)) DREF DeeObject *_getitem(size_t index) {
		return DeeObject_GetItemIndex(((ProxyType *)this)->ptr(), index);
	}
	NONNULL_CXX((1)) DREF DeeObject *_getitem(char const *key) {
		return DeeObject_GetItemString(((ProxyType *)this)->ptr(), key);
	}
	NONNULL_CXX((1)) DREF DeeObject *_getitem(char const *key, Dee_hash_t hash) {
		return DeeObject_GetItemStringHash(((ProxyType *)this)->ptr(), key, hash);
	}
	NONNULL_CXX((1)) DREF DeeObject *_getitem(char const *key, size_t keylen, Dee_hash_t hash) {
		return DeeObject_GetItemStringLenHash(((ProxyType *)this)->ptr(), key, keylen, hash);
	}

	NONNULL_CXX((1)) DREF DeeObject *_trygetitem(DeeObject *index_or_key) {
		return DeeObject_TryGetItem(((ProxyType *)this)->ptr(), index_or_key);
	}
	NONNULL_CXX((1)) DREF DeeObject *_trygetitem(size_t index) {
		return DeeObject_TryGetItemIndex(((ProxyType *)this)->ptr(), index);
	}
	NONNULL_CXX((1)) DREF DeeObject *_trygetitem(char const *key) {
		return DeeObject_TryGetItemString(((ProxyType *)this)->ptr(), key);
	}
	NONNULL_CXX((1)) DREF DeeObject *_trygetitem(char const *key, Dee_hash_t hash) {
		return DeeObject_TryGetItemStringHash(((ProxyType *)this)->ptr(), key, hash);
	}
	NONNULL_CXX((1)) DREF DeeObject *_trygetitem(char const *key, size_t keylen, Dee_hash_t hash) {
		return DeeObject_TryGetItemStringLenHash(((ProxyType *)this)->ptr(), key, keylen, hash);
	}

	NONNULL_CXX((1)) int _bounditem(DeeObject *index_or_key) {
		return DeeObject_BoundItem(((ProxyType *)this)->ptr(), index_or_key);
	}
	int _bounditem(size_t index) {
		return DeeObject_BoundItemIndex(((ProxyType *)this)->ptr(), index);
	}
	NONNULL_CXX((1)) int _bounditem(char const *key) {
		return DeeObject_BoundItemString(((ProxyType *)this)->ptr(), key);
	}
	NONNULL_CXX((1)) int _bounditem(char const *key, Dee_hash_t hash) {
		return DeeObject_BoundItemStringHash(((ProxyType *)this)->ptr(), key, hash);
	}
	NONNULL_CXX((1)) int _bounditem(char const *key, size_t keylen, Dee_hash_t hash) {
		return DeeObject_BoundItemStringLenHash(((ProxyType *)this)->ptr(), key, keylen, hash);
	}

	NONNULL_CXX((1)) int _hasitem(DeeObject *index_or_key) {
		return DeeObject_HasItem(((ProxyType *)this)->ptr(), index_or_key);
	}
	int _hasitem(size_t index) {
		return DeeObject_HasItemIndex(((ProxyType *)this)->ptr(), index);
	}
	NONNULL_CXX((1)) int _hasitem(char const *key) {
		return DeeObject_HasItemString(((ProxyType *)this)->ptr(), key);
	}
	NONNULL_CXX((1)) int _hasitem(char const *key, Dee_hash_t hash) {
		return DeeObject_HasItemStringHash(((ProxyType *)this)->ptr(), key, hash);
	}
	NONNULL_CXX((1)) int _hasitem(char const *key, size_t keylen, Dee_hash_t hash) {
		return DeeObject_HasItemStringLenHash(((ProxyType *)this)->ptr(), key, keylen, hash);
	}

	NONNULL_CXX((1)) int _delitem(DeeObject *index_or_key) {
		return DeeObject_DelItem(((ProxyType *)this)->ptr(), index_or_key);
	}
	int _delitem(size_t index) {
		return DeeObject_DelItemIndex(((ProxyType *)this)->ptr(), index);
	}
	NONNULL_CXX((1)) int _delitem(char const *key) {
		return DeeObject_DelItemString(((ProxyType *)this)->ptr(), key);
	}
	NONNULL_CXX((1)) int _delitem(char const *key, Dee_hash_t hash) {
		return DeeObject_DelItemStringHash(((ProxyType *)this)->ptr(), key, hash);
	}
	NONNULL_CXX((1)) int _delitem(char const *key, size_t keylen, Dee_hash_t hash) {
		return DeeObject_DelItemStringLenHash(((ProxyType *)this)->ptr(), key, keylen, hash);
	}

	NONNULL_CXX((1, 2)) int _setitem(DeeObject *index_or_key, DeeObject *value) {
		return DeeObject_SetItem(((ProxyType *)this)->ptr(), index_or_key, value);
	}
	NONNULL_CXX((2)) int _setitem(size_t index, DeeObject *value) {
		return DeeObject_SetItemIndex(((ProxyType *)this)->ptr(), index, value);
	}
	NONNULL_CXX((1, 2)) int _setitem(char const *key, DeeObject *value) {
		return DeeObject_SetItemString(((ProxyType *)this)->ptr(), key, value);
	}
	NONNULL_CXX((1, 3)) int _setitem(char const *key, Dee_hash_t hash, DeeObject *value) {
		return DeeObject_SetItemStringHash(((ProxyType *)this)->ptr(), key, hash, value);
	}
	NONNULL_CXX((1, 4)) int _setitem(char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
		return DeeObject_SetItemStringLenHash(((ProxyType *)this)->ptr(), key, keylen, hash, value);
	}


	NONNULL_CXX((1)) Ref<GetItemType> getitem(DeeObject *index_or_key) {
		return inherit(DeeObject_GetItem(((ProxyType *)this)->ptr(), index_or_key));
	}
	NONNULL_CXX((1, 2)) Ref<GetItemType> getitem(DeeObject *index_or_key, DeeObject *def) {
		DREF DeeObject *result = DeeObject_TryGetItem(((ProxyType *)this)->ptr(), index_or_key);
		if (result == ITER_DONE) {
			Dee_Incref(def);
			result = def;
		}
		return inherit(result);
	}
	Ref<GetItemType> getitem(size_t index) {
		return inherit(DeeObject_GetItemIndex(((ProxyType *)this)->ptr(), index));
	}
	NONNULL_CXX((2)) Ref<GetItemType> getitem(size_t index, DeeObject *def) {
		DREF DeeObject *result = DeeObject_TryGetItemIndex(((ProxyType *)this)->ptr(), index);
		if (result == ITER_DONE) {
			Dee_Incref(def);
			result = def;
		}
		return inherit(result);
	}
	NONNULL_CXX((1)) Ref<GetItemType> getitem(char const *key) {
		return inherit(DeeObject_GetItemString(((ProxyType *)this)->ptr(), key));
	}
	NONNULL_CXX((1, 2)) Ref<GetItemType> getitem(char const *key, DeeObject *def) {
		DREF DeeObject *result = DeeObject_TryGetItemString(((ProxyType *)this)->ptr(), key);
		if (result == ITER_DONE) {
			Dee_Incref(def);
			result = def;
		}
		return inherit(result);
	}
	NONNULL_CXX((1)) Ref<GetItemType> getitem(char const *key, Dee_hash_t hash) {
		return inherit(DeeObject_GetItemStringHash(((ProxyType *)this)->ptr(), key, hash));
	}
	NONNULL_CXX((1, 3)) Ref<GetItemType> getitem(char const *key, Dee_hash_t hash, DeeObject *def) {
		DREF DeeObject *result = DeeObject_TryGetItemStringHash(((ProxyType *)this)->ptr(), key, hash);
		if (result == ITER_DONE) {
			Dee_Incref(def);
			result = def;
		}
		return inherit(result);
	}
	NONNULL_CXX((1)) Ref<GetItemType> getitem(char const *key, size_t keylen, Dee_hash_t hash) {
		return inherit(DeeObject_GetItemStringLenHash(((ProxyType *)this)->ptr(), key, keylen, hash));
	}
	NONNULL_CXX((1, 4)) Ref<GetItemType> getitem(char const *key, size_t keylen, Dee_hash_t hash, DeeObject *def) {
		DREF DeeObject *result = DeeObject_TryGetItemStringLenHash(((ProxyType *)this)->ptr(), key, keylen, hash);
		if (result == ITER_DONE) {
			Dee_Incref(def);
			result = def;
		}
		return inherit(result);
	}

	NONNULL_CXX((1)) bool bounditem(DeeObject *index_or_key) {
		return throw_if_minusone(DeeObject_BoundItem(((ProxyType *)this)->ptr(), index_or_key)) > 0;
	}
	bool bounditem(size_t index) {
		return throw_if_minusone(DeeObject_BoundItemIndex(((ProxyType *)this)->ptr(), index)) > 0;
	}
	NONNULL_CXX((1)) bool bounditem(char const *key) {
		return throw_if_minusone(DeeObject_BoundItemString(((ProxyType *)this)->ptr(), key)) > 0;
	}
	NONNULL_CXX((1)) bool bounditem(char const *key, Dee_hash_t hash) {
		return throw_if_minusone(DeeObject_BoundItemStringHash(((ProxyType *)this)->ptr(), key, hash)) > 0;
	}
	NONNULL_CXX((1)) bool bounditem(char const *key, size_t keylen, Dee_hash_t hash) {
		return throw_if_minusone(DeeObject_BoundItemStringLenHash(((ProxyType *)this)->ptr(), key, keylen, hash)) > 0;
	}

	NONNULL_CXX((1)) bool hasitem(DeeObject *index_or_key) {
		return throw_if_negative(DeeObject_HasItem(((ProxyType *)this)->ptr(), index_or_key)) != 0;
	}
	bool hasitem(size_t index) {
		return throw_if_negative(DeeObject_HasItemIndex(((ProxyType *)this)->ptr(), index)) != 0;
	}
	NONNULL_CXX((1)) bool hasitem(char const *key) {
		return throw_if_negative(DeeObject_HasItemString(((ProxyType *)this)->ptr(), key)) != 0;
	}
	NONNULL_CXX((1)) bool hasitem(char const *key, Dee_hash_t hash) {
		return throw_if_negative(DeeObject_HasItemStringHash(((ProxyType *)this)->ptr(), key, hash)) != 0;
	}
	NONNULL_CXX((1)) bool hasitem(char const *key, size_t keylen, Dee_hash_t hash) {
		return throw_if_negative(DeeObject_HasItemStringLenHash(((ProxyType *)this)->ptr(), key, keylen, hash)) != 0;
	}

	NONNULL_CXX((1)) void delitem(DeeObject *index_or_key) {
		throw_if_nonzero(DeeObject_DelItem(((ProxyType *)this)->ptr(), index_or_key));
	}
	void delitem(size_t index) {
		throw_if_nonzero(DeeObject_DelItemIndex(((ProxyType *)this)->ptr(), index));
	}
	NONNULL_CXX((1)) void delitem(char const *key) {
		throw_if_nonzero(DeeObject_DelItemString(((ProxyType *)this)->ptr(), key));
	}
	NONNULL_CXX((1)) void delitem(char const *key, Dee_hash_t hash) {
		throw_if_nonzero(DeeObject_DelItemStringHash(((ProxyType *)this)->ptr(), key, hash));
	}
	NONNULL_CXX((1)) void delitem(char const *key, size_t keylen, Dee_hash_t hash) {
		throw_if_nonzero(DeeObject_DelItemStringLenHash(((ProxyType *)this)->ptr(), key, keylen, hash));
	}

	NONNULL_CXX((1, 2)) void setitem(DeeObject *index_or_key, DeeObject *value) {
		throw_if_nonzero(DeeObject_SetItem(((ProxyType *)this)->ptr(), index_or_key, value));
	}
	NONNULL_CXX((2)) void setitem(size_t index, DeeObject *value) {
		throw_if_nonzero(DeeObject_SetItemIndex(((ProxyType *)this)->ptr(), index, value));
	}
	NONNULL_CXX((1, 2)) void setitem(char const *key, DeeObject *value) {
		throw_if_nonzero(DeeObject_SetItemString(((ProxyType *)this)->ptr(), key, value));
	}
	NONNULL_CXX((1, 3)) void setitem(char const *key, Dee_hash_t hash, DeeObject *value) {
		throw_if_nonzero(DeeObject_SetItemStringHash(((ProxyType *)this)->ptr(), key, hash, value));
	}
	NONNULL_CXX((1, 4)) void setitem(char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
		throw_if_nonzero(DeeObject_SetItemStringLenHash(((ProxyType *)this)->ptr(), key, keylen, hash, value));
	}
};

template<class ProxyType,
         class RangeSeqType = Sequence<> >
class RangeProxyAccessor {
public:
	NONNULL_CXX((1, 2)) RangeProxyObj<RangeSeqType> range(DeeObject *begin, DeeObject *end) {
		return RangeProxyObj<RangeSeqType>(((ProxyType *)this)->ptr(), begin, end);
	}
	NONNULL_CXX((1)) RangeProxyObjIdx<RangeSeqType> range(DeeObject *begin, Dee_ssize_t end) {
		return RangeProxyObjIdx<RangeSeqType>(((ProxyType *)this)->ptr(), begin, end);
	}
	NONNULL_CXX((2)) RangeProxyIdxObj<RangeSeqType> range(Dee_ssize_t begin, DeeObject *end) {
		return RangeProxyIdxObj<RangeSeqType>(((ProxyType *)this)->ptr(), begin, end);
	}
	RangeProxyIdx<RangeSeqType> range(Dee_ssize_t begin, Dee_ssize_t end) {
		return RangeProxyIdx<RangeSeqType>(((ProxyType *)this)->ptr(), begin, end);
	}

	NONNULL_CXX((1, 2)) Ref<RangeSeqType> getrange(DeeObject *begin, DeeObject *end) {
		return inherit(DeeObject_GetRange(((ProxyType *)this)->ptr(), begin, end));
	}
	NONNULL_CXX((1)) Ref<RangeSeqType> getrange(DeeObject *begin, Dee_ssize_t end) {
		return inherit(DeeObject_GetRangeEndIndex(((ProxyType *)this)->ptr(), begin, end));
	}
	NONNULL_CXX((2)) Ref<RangeSeqType> getrange(Dee_ssize_t begin, DeeObject *end) {
		return inherit(DeeObject_GetRangeBeginIndex(((ProxyType *)this)->ptr(), begin, end));
	}
	Ref<RangeSeqType> getrange(Dee_ssize_t begin, Dee_ssize_t end) {
		return inherit(DeeObject_GetRangeIndex(((ProxyType *)this)->ptr(), begin, end));
	}

	NONNULL_CXX((1, 2)) void delrange(DeeObject *begin, DeeObject *end) {
		throw_if_nonzero(DeeObject_DelRange(((ProxyType *)this)->ptr(), begin, end));
	}
	NONNULL_CXX((1)) void delrange(DeeObject *begin, Dee_ssize_t end) {
		throw_if_nonzero(DeeObject_DelRangeEndIndex(((ProxyType *)this)->ptr(), begin, end));
	}
	NONNULL_CXX((2)) void delrange(Dee_ssize_t begin, DeeObject *end) {
		throw_if_nonzero(DeeObject_DelRangeBeginIndex(((ProxyType *)this)->ptr(), begin, end));
	}
	void delrange(Dee_ssize_t begin, Dee_ssize_t end) {
		throw_if_nonzero(DeeObject_DelRangeIndex(((ProxyType *)this)->ptr(), begin, end));
	}

	NONNULL_CXX((1, 2, 3)) void setrange(DeeObject *begin, DeeObject *end, DeeObject *value) {
		return inherit(DeeObject_SetRange(((ProxyType *)this)->ptr(), begin, end, value));
	}
	NONNULL_CXX((1, 3)) void setrange(DeeObject *begin, Dee_ssize_t end, DeeObject *value) {
		return inherit(DeeObject_SetRangeEndIndex(((ProxyType *)this)->ptr(), begin, end, value));
	}
	NONNULL_CXX((2, 3)) void setrange(Dee_ssize_t begin, DeeObject *end, DeeObject *value) {
		return inherit(DeeObject_SetRangeBeginIndex(((ProxyType *)this)->ptr(), begin, end, value));
	}
	NONNULL_CXX((3)) void setrange(Dee_ssize_t begin, Dee_ssize_t end, DeeObject *value) {
		return inherit(DeeObject_SetRangeIndex(((ProxyType *)this)->ptr(), begin, end, value));
	}
};

template<class ProxyType,
         class MathType = Object>
class MathProxyAccessor {
public:
	Ref<MathType> inv() {
		return inherit(DeeObject_Inv(((ProxyType *)this)->ptr()));
	}
	Ref<MathType> pos() {
		return inherit(DeeObject_Pos(((ProxyType *)this)->ptr()));
	}
	Ref<MathType> neg() {
		return inherit(DeeObject_Neg(((ProxyType *)this)->ptr()));
	}
	Ref<MathType> add(int8_t right) {
		return inherit(DeeObject_AddInt8(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> add(uint32_t right) {
		return inherit(DeeObject_AddUInt32(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> add(DeeObject *right) {
		return inherit(DeeObject_Add(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> sub(int8_t right) {
		return inherit(DeeObject_SubInt8(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> sub(uint32_t right) {
		return inherit(DeeObject_SubUInt32(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> sub(DeeObject *right) {
		return inherit(DeeObject_Sub(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> mul(int8_t right) {
		return inherit(DeeObject_MulInt8(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> mul(DeeObject *right) {
		return inherit(DeeObject_Mul(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> div(int8_t right) {
		return inherit(DeeObject_DivInt8(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> div(DeeObject *right) {
		return inherit(DeeObject_Div(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> mod(int8_t right) {
		return inherit(DeeObject_ModInt8(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> mod(DeeObject *right) {
		return inherit(DeeObject_Mod(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> shl(uint8_t right) {
		return inherit(DeeObject_ShlUInt8(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> shl(DeeObject *right) {
		return inherit(DeeObject_Shl(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> shr(uint8_t right) {
		return inherit(DeeObject_ShrUInt8(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> shr(DeeObject *right) {
		return inherit(DeeObject_Shr(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> and_(uint32_t right) {
		return inherit(DeeObject_AndUInt32(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> and_(DeeObject *right) {
		return inherit(DeeObject_And(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> or_(uint32_t right) {
		return inherit(DeeObject_OrUInt32(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> or_(DeeObject *right) {
		return inherit(DeeObject_Or(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> xor_(uint32_t right) {
		return inherit(DeeObject_XorUInt32(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> xor_(DeeObject *right) {
		return inherit(DeeObject_Xor(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> pow(DeeObject *right) {
		return inherit(DeeObject_Pow(((ProxyType *)this)->ptr(), right));
	}

	Ref<MathType> operator~() {
		return inherit(DeeObject_Inv(((ProxyType *)this)->ptr()));
	}
	Ref<MathType> operator+() {
		return inherit(DeeObject_Pos(((ProxyType *)this)->ptr()));
	}
	Ref<MathType> operator-() {
		return inherit(DeeObject_Neg(((ProxyType *)this)->ptr()));
	}
	Ref<MathType> operator+(int8_t right) {
		return inherit(DeeObject_AddInt8(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator+(uint32_t right) {
		return inherit(DeeObject_AddUInt32(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator+(DeeObject *right) {
		return inherit(DeeObject_Add(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator-(int8_t right) {
		return inherit(DeeObject_SubInt8(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator-(uint32_t right) {
		return inherit(DeeObject_SubUInt32(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator-(DeeObject *right) {
		return inherit(DeeObject_Sub(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator*(int8_t right) {
		return inherit(DeeObject_MulInt8(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator*(DeeObject *right) {
		return inherit(DeeObject_Mul(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator/(int8_t right) {
		return inherit(DeeObject_DivInt8(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator/(DeeObject *right) {
		return inherit(DeeObject_Div(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator%(int8_t right) {
		return inherit(DeeObject_ModInt8(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator%(DeeObject *right) {
		return inherit(DeeObject_Mod(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator<<(uint8_t right) {
		return inherit(DeeObject_ShlUInt8(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator<<(DeeObject *right) {
		return inherit(DeeObject_Shl(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator>>(uint8_t right) {
		return inherit(DeeObject_ShrUInt8(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator>>(DeeObject *right) {
		return inherit(DeeObject_Shr(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator&(uint32_t right) {
		return inherit(DeeObject_AndUInt32(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator&(DeeObject *right) {
		return inherit(DeeObject_And(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator|(uint32_t right) {
		return inherit(DeeObject_OrUInt32(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator|(DeeObject *right) {
		return inherit(DeeObject_Or(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator^(uint32_t right) {
		return inherit(DeeObject_XorUInt32(((ProxyType *)this)->ptr(), right));
	}
	Ref<MathType> operator^(DeeObject *right) {
		return inherit(DeeObject_Xor(((ProxyType *)this)->ptr(), right));
	}
};

} /* namespace detail */


/************************************************************************/
/* Base type for all deemon objects                                     */
/************************************************************************/
class Object
	: public ::DeeObject
	, public detail::CallRefProxy<Object, Object>
	, public detail::ThisCallRefProxy<Object, Object>
	, public detail::AttrProxyAccessor<Object, Object, Object>
	, public detail::ItemProxyAccessor<Object, Object>
	, public detail::RangeProxyAccessor<Object, Object>
	, public detail::MathProxyAccessor<Object, Object>
{
	__CXX_DELETE_CTOR(Object);
	__CXX_DELETE_COPY(Object);
	__CXX_DELETE_COPY_ASSIGN(Object);
	__CXX_DELETE_VOLATILE_COPY_ASSIGN(Object);
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeObject_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *__restrict UNUSED(ob)) DEE_CXX_NOTHROW {
		return true;
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *__restrict UNUSED(ob)) DEE_CXX_NOTHROW {
		return true;
	}


	ATTR_RETNONNULL WUNUSED DeeObject *ptr() DEE_CXX_NOTHROW {
		return this;
	}
	WUNUSED NONNULL_CXX((1)) DREF DeeObject *_callref(DeeObject *args) DEE_CXX_NOTHROW {
		return DeeObject_CallTuple(this, args);
	}
	WUNUSED NONNULL_CXX((1)) DREF DeeObject *_callref(DeeObject *args, DeeObject *kw) DEE_CXX_NOTHROW {
		return DeeObject_CallTupleKw(this, args, kw);
	}
	WUNUSED DREF DeeObject *_callref(size_t argc, DeeObject *const *argv) DEE_CXX_NOTHROW {
		return DeeObject_Call(this, argc, argv);
	}
	WUNUSED DREF DeeObject *_callref(size_t argc, DeeObject *const *argv, DeeObject *kw) DEE_CXX_NOTHROW {
		return DeeObject_CallKw(this, argc, argv, kw);
	}

	WUNUSED NONNULL_CXX((1, 2)) DREF DeeObject *_thiscallref(DeeObject *this_arg, DeeObject *args) DEE_CXX_NOTHROW {
		return DeeObject_ThisCallTuple(this, this_arg, args);
	}
	WUNUSED NONNULL_CXX((1, 2)) DREF DeeObject *_thiscallref(DeeObject *this_arg, DeeObject *args, DeeObject *kw) DEE_CXX_NOTHROW {
		return DeeObject_ThisCallTupleKw(this, this_arg, args, kw);
	}
	WUNUSED NONNULL_CXX((1)) DREF DeeObject *_thiscallref(DeeObject *this_arg, size_t argc, DeeObject *const *argv) DEE_CXX_NOTHROW {
		return DeeObject_ThisCall(this, this_arg, argc, argv);
	}
	WUNUSED NONNULL_CXX((1)) DREF DeeObject *_thiscallref(DeeObject *this_arg, size_t argc, DeeObject *const *argv, DeeObject *kw) DEE_CXX_NOTHROW {
		return DeeObject_ThisCallKw(this, this_arg, argc, argv, kw);
	}

	Ref<string> str() {
		return inherit(DeeObject_Str(this));
	}
	Ref<string> repr() {
		return inherit(DeeObject_Repr(this));
	}
	Ref<Object> super() {
		return inherit(DeeSuper_Of(this));
	}
	Ref<Object> super(DeeTypeObject *__restrict super_type) {
		return inherit(DeeSuper_New(super_type, this));
	}
	Ref<Object> next() {
		DREF DeeObject *result;
		result = throw_if_null(DeeObject_IterNext(this));
		if (result == ITER_DONE)
			result = NULL;
		return inherit(maybenull(result));
	}

	bool contains(DeeObject *value) {
		return throw_if_negative(DeeObject_ContainsAsBool(this, value)) > 0;
	}
	size_t size() {
		return throw_if_minusone(DeeObject_Size(this));
	}

#undef print
	size_t print(Dee_formatprinter_t printer, void *arg) {
		return throw_if_negative(DeeObject_Print(this, printer, arg));
	}
	size_t print(Dee_formatprinter_t printer, void *arg, DeeObject *format_str) {
		return throw_if_negative(DeeObject_PrintFormat(this, printer, arg, format_str));
	}
	size_t print(Dee_formatprinter_t printer, void *arg, /*utf-8*/ char const *__restrict format_str) {
		return throw_if_negative(DeeObject_PrintFormatString(this, printer, arg, format_str, strlen(format_str)));
	}
	size_t print(Dee_formatprinter_t printer, void *arg, /*utf-8*/ char const *__restrict format_str, size_t format_len) {
		return throw_if_negative(DeeObject_PrintFormatString(this, printer, arg, format_str, format_len));
	}
	size_t printrepr(Dee_formatprinter_t printer, void *arg) {
		return throw_if_negative(DeeObject_PrintRepr(this, printer, arg));
	}

	/* Generic operator invocation. */
	Ref<Object> invoke_operator(uint16_t name) {
		return inherit(DeeObject_InvokeOperator(this, name, 0, NULL));
	}
	template<class ...ArgTypes>
	Ref<Object> invoke_operator(uint16_t name, Tuple<ArgTypes...> const *args) {
		return inherit(DeeObject_InvokeOperatorTuple(this, name, (DeeObject *)args));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<Object>) invoke_operator(uint16_t name, size_t argc, ArgType *const *argv) {
		return inherit(DeeObject_InvokeOperator(this, name, argc, (DeeObject *const *)argv));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_T(ArgType, Ref<Object>) invoke_operator(uint16_t name, size_t argc, ArgType **argv) {
		return inherit(DeeObject_InvokeOperator(this, name, argc, (DeeObject *const *)argv));
	}
	template<class ArgType>
	DEE_ENABLE_IF_OBJECT_PTR_T(ArgType, Ref<Object>) invoke_operator(uint16_t name, std::initializer_list<ArgType> const &args) {
		return inherit(DeeObject_InvokeOperator(this, name, args.size(), (DeeObject *const *)args.begin()));
	}

	WUNUSED bool istrue() {
		return throw_if_negative(DeeObject_Bool(this)) != 0;
	}
	WUNUSED bool isfalse() {
		return !istrue();
	}
	WUNUSED operator bool() {
		return istrue();
	}
	WUNUSED bool operator!() {
		return isfalse();
	}

	WUNUSED Dee_hash_t hash() DEE_CXX_NOTHROW {
		return DeeObject_Hash(this);
	}

	WUNUSED Ref<Object> operator==(DeeObject *other) {
		return inherit(DeeObject_CmpEq(this, other));
	}
	WUNUSED Ref<Object> operator!=(DeeObject *other) {
		return inherit(DeeObject_CmpNe(this, other));
	}
	WUNUSED Ref<Object> operator<(DeeObject *other) {
		return inherit(DeeObject_CmpLo(this, other));
	}
	WUNUSED Ref<Object> operator<=(DeeObject *other) {
		return inherit(DeeObject_CmpLe(this, other));
	}
	WUNUSED Ref<Object> operator>(DeeObject *other) {
		return inherit(DeeObject_CmpGr(this, other));
	}
	WUNUSED Ref<Object> operator>=(DeeObject *other) {
		return inherit(DeeObject_CmpGe(this, other));
	}

	/* Integer conversion */
	Object &getval(char &value) {
		throw_if_nonzero(DeeObject_AsChar(this, &value));
		return *this;
	}
	Object &getval(signed char &value) {
		throw_if_nonzero(DeeObject_AsSChar(this, &value));
		return *this;
	}
	Object &getval(unsigned char &value) {
		throw_if_nonzero(DeeObject_AsUChar(this, &value));
		return *this;
	}
	Object &getval(short &value) {
		throw_if_nonzero(DeeObject_AsShort(this, &value));
		return *this;
	}
	Object &getval(unsigned short &value) {
		throw_if_nonzero(DeeObject_AsUShort(this, &value));
		return *this;
	}
	Object &getval(int &value) {
		throw_if_nonzero(DeeObject_AsInt(this, &value));
		return *this;
	}
	Object &getval(unsigned int &value) {
		throw_if_nonzero(DeeObject_AsUInt(this, &value));
		return *this;
	}
	Object &getval(long &value) {
		throw_if_nonzero(DeeObject_AsLong(this, &value));
		return *this;
	}
	Object &getval(unsigned long &value) {
		throw_if_nonzero(DeeObject_AsULong(this, &value));
		return *this;
	}
#ifdef __COMPILER_HAVE_LONGLONG
	Object &getval(__LONGLONG &value) {
		throw_if_nonzero(DeeObject_AsLLong(this, &value));
		return *this;
	}
	Object &getval(__ULONGLONG &value) {
		throw_if_nonzero(DeeObject_AsULLong(this, &value));
		return *this;
	}
#endif /* __COMPILER_HAVE_LONGLONG */
	Object &getval(Dee_int128_t &value) {
		throw_if_nonzero(DeeObject_AsInt128(this, &value));
		return *this;
	}
	Object &getval(Dee_uint128_t &value) {
		throw_if_nonzero(DeeObject_AsUInt128(this, &value));
		return *this;
	}
	Object &getval(float &value) {
		double temp;
		getval(temp);
		value = (float)temp;
		return *this;
	}
	Object &getval(double &value) {
		throw_if_nonzero(DeeObject_AsDouble(this, &value));
		return *this;
	}
#ifdef __COMPILER_HAVE_LONGDOUBLE
	Object &getval(__LONGDOUBLE &value) {
		double temp;
		getval(temp);
		value = (__LONGDOUBLE)temp;
		return *this;
	}
#endif /* __COMPILER_HAVE_LONGDOUBLE */

	/* Helper functions to explicitly convert an object to an integral value. */
	template<class T> WUNUSED T asval() {
		T result;
		getval(result);
		return result;
	}
	WUNUSED short asshort() {
		return asval<short>();
	}
	WUNUSED unsigned short asushort() {
		return asval<unsigned short>();
	}
	WUNUSED int asint() {
		return asval<int>();
	}
	WUNUSED unsigned int asuint() {
		return asval<unsigned int>();
	}
	WUNUSED long aslong() {
		return asval<long>();
	}
	WUNUSED unsigned long asulong() {
		return asval<unsigned long>();
	}
#ifdef __COMPILER_HAVE_LONGLONG
	WUNUSED __LONGLONG asllong() {
		return asval<__LONGLONG>();
	}
	WUNUSED __ULONGLONG asullong() {
		return asval<__ULONGLONG>();
	}
#endif /* __COMPILER_HAVE_LONGLONG */
	WUNUSED int8_t ass8() {
		return asval<int8_t>();
	}
	WUNUSED int16_t ass16() {
		return asval<int16_t>();
	}
	WUNUSED int32_t ass32() {
		return asval<int32_t>();
	}
	WUNUSED int64_t ass64() {
		return asval<int64_t>();
	}
	WUNUSED Dee_int128_t ass128() {
		return asval<Dee_int128_t>();
	}
	WUNUSED uint8_t asu8() {
		return asval<uint8_t>();
	}
	WUNUSED uint16_t asu16() {
		return asval<uint16_t>();
	}
	WUNUSED uint32_t asu32() {
		return asval<uint32_t>();
	}
	WUNUSED uint64_t asu64() {
		return asval<uint64_t>();
	}
	WUNUSED Dee_uint128_t asu128() {
		return asval<Dee_uint128_t>();
	}
	WUNUSED size_t assize() {
		return asval<size_t>();
	}
	WUNUSED Dee_ssize_t asssize() {
		return asval<Dee_ssize_t>();
	}
	WUNUSED float asfloat() {
		return asval<float>();
	}
	WUNUSED double asdouble() {
		return asval<double>();
	}
#ifdef __COMPILER_HAVE_LONGDOUBLE
	WUNUSED __LONGDOUBLE asldouble() {
		return asval<__LONGDOUBLE>();
	}
#endif /* __COMPILER_HAVE_LONGDOUBLE */

	/* Integer conversion operators */
	explicit WUNUSED operator char() {
		return asval<char>();
	}
	explicit WUNUSED operator signed char() {
		return asval<signed char>();
	}
	explicit WUNUSED operator unsigned char() {
		return asval<unsigned char>();
	}
	explicit WUNUSED operator short() {
		return asval<short>();
	}
	explicit WUNUSED operator unsigned short() {
		return asval<unsigned short>();
	}
	explicit WUNUSED operator int() {
		return asval<int>();
	}
	explicit WUNUSED operator unsigned int() {
		return asval<unsigned int>();
	}
	explicit WUNUSED operator long() {
		return asval<long>();
	}
	explicit WUNUSED operator unsigned long() {
		return asval<unsigned long>();
	}
#ifdef __COMPILER_HAVE_LONGLONG
	explicit WUNUSED operator __LONGLONG() {
		return asval<__LONGLONG>();
	}
	explicit WUNUSED operator __ULONGLONG() {
		return asval<__ULONGLONG>();
	}
#endif /* __COMPILER_HAVE_LONGLONG */
	explicit WUNUSED operator Dee_int128_t() {
		return asval<Dee_int128_t>();
	}
	explicit WUNUSED operator Dee_uint128_t() {
		return asval<Dee_uint128_t>();
	}
	explicit WUNUSED operator float() {
		return asval<float>();
	}
	explicit WUNUSED operator double() {
		return asval<double>();
	}
#ifdef __COMPILER_HAVE_LONGDOUBLE
	explicit WUNUSED operator __LONGDOUBLE() {
		return asval<__LONGDOUBLE>();
	}
#endif /* __COMPILER_HAVE_LONGLONG */

	template<class T> DEE_ENABLE_IF_OBJECT(T) &as() DEE_CXX_NOTHROW {
		return *(T *)this;
	}
	template<class T> DEE_ENABLE_IF_OBJECT(T) const &as() const DEE_CXX_NOTHROW {
		return *(T const *)this;
	}

/*[[[deemon (CxxType from rt.gen.cxxapi)(Object from deemon).printCxxApi(exclude: { "this", "class", "type", "super" });]]]*/
	WUNUSED Ref<Object> (__copy__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__copy__", _Dee_HashSelectC(0xdd5d91cd, 0x3b45880f436d8335), 0, NULL));
	}
	WUNUSED Ref<Object> (__deepcopy__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__deepcopy__", _Dee_HashSelectC(0xedd98409, 0x2ee5773152024f39), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__assign__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__assign__", _Dee_HashSelectC(0x1fe4d78d, 0x874637496b0b0a2d), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__moveassign__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__moveassign__", _Dee_HashSelectC(0x45785caa, 0x3ef73258381466b4), 1, args));
	}
	WUNUSED Ref<string> (__str__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__str__", _Dee_HashSelectC(0xbcdfcf00, 0xc0caa980438eff23), 0, NULL));
	}
	WUNUSED Ref<string> (__repr__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__repr__", _Dee_HashSelectC(0x5c5d08ce, 0xfdc7945465f9ede7), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (__bool__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__bool__", _Dee_HashSelectC(0x1d6e29c8, 0x7d5655cb5b8aa88b), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__call__)(DeeObject *args) {
		DeeObject *args_[1];
		args_[0] = args;
		return inherit(DeeObject_CallAttrStringHash(this, "__call__", _Dee_HashSelectC(0xbf1484cd, 0x98fac865489cd2e0), 1, args_));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Object> (__thiscall__)(DeeObject *this_arg, DeeObject *args) {
		DeeObject *args_[2];
		args_[0] = this_arg;
		args_[1] = args;
		return inherit(DeeObject_CallAttrStringHash(this, "__thiscall__", _Dee_HashSelectC(0xb621cbeb, 0x1f66fa1159253519), 2, args_));
	}
	WUNUSED Ref<deemon::int_> (__hash__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__hash__", _Dee_HashSelectC(0xc088645e, 0xbc5b5b1504b9d2d8), 0, NULL));
	}
	WUNUSED Ref<deemon::int_> (__int__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__int__", _Dee_HashSelectC(0x5400020e, 0xf79c54cf51ffa1f8), 0, NULL));
	}
	WUNUSED Ref<Object> (__inv__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__inv__", _Dee_HashSelectC(0x63d81258, 0xef0370cd96d93e7e), 0, NULL));
	}
	WUNUSED Ref<Object> (__pos__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__pos__", _Dee_HashSelectC(0x108c10b3, 0xad88142ae7345f59), 0, NULL));
	}
	WUNUSED Ref<Object> (__neg__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__neg__", _Dee_HashSelectC(0x2b34a59d, 0xbbdf80da78d5b0ee), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__add__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__add__", _Dee_HashSelectC(0x5cb4d11a, 0x6f33a2bc44b51c54), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__sub__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__sub__", _Dee_HashSelectC(0xc2239a1e, 0xd91dc2370225ae2f), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__mul__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__mul__", _Dee_HashSelectC(0x51c62b13, 0x7e793c85424d924c), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__div__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__div__", _Dee_HashSelectC(0x5b814977, 0x4a50b3e0b859e051), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__mod__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__mod__", _Dee_HashSelectC(0x481c8a3, 0x4f56cb923e40dd8), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__shl__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__shl__", _Dee_HashSelectC(0xca15bfa1, 0x8eb668f22579acf1), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__shr__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__shr__", _Dee_HashSelectC(0xb066ed7b, 0xc3d4cf88459979b3), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__and__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__and__", _Dee_HashSelectC(0xac39cb48, 0x2b28cb619a45a71e), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__or__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__or__", _Dee_HashSelectC(0xf95e054c, 0x2bc6caacde6c129e), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__xor__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__xor__", _Dee_HashSelectC(0x7378854c, 0x4a8410f65a74106f), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__pow__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__pow__", _Dee_HashSelectC(0xe40938b1, 0xefb08d20fe1ec58), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__eq__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__eq__", _Dee_HashSelectC(0x2e15aa28, 0x20311e6561792a00), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__ne__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__ne__", _Dee_HashSelectC(0x485d961, 0xe9453f35f2aef187), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__lo__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__lo__", _Dee_HashSelectC(0xbd689eba, 0xf2a5e28053b056c9), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__le__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__le__", _Dee_HashSelectC(0xd4e31410, 0xe371879105557498), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__gr__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__gr__", _Dee_HashSelectC(0x8af205e9, 0x3fe2793f689055e5), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__ge__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__ge__", _Dee_HashSelectC(0xe467e452, 0xe5ad3ef5f6f17572), 1, args));
	}
	WUNUSED Ref<Object> (__size__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__size__", _Dee_HashSelectC(0x543ba3b5, 0xd416117435cce357), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__contains__)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "__contains__", _Dee_HashSelectC(0x769af591, 0x80f9234f8000b556), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__getitem__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "__getitem__", _Dee_HashSelectC(0x2796c7b1, 0x326672bfc335fb3d), 1, args));
	}
	NONNULL_CXX((1)) void (__delitem__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__delitem__", _Dee_HashSelectC(0x20ba3d50, 0x477c6001247177f), 1, args)));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Object> (__setitem__)(DeeObject *index, DeeObject *value) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "__setitem__", _Dee_HashSelectC(0xa12b6584, 0x4f2c202e4a8ee77a), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Object> (__getrange__)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "__getrange__", _Dee_HashSelectC(0x7f22541, 0x53d4d4259a06a055), 2, args));
	}
	NONNULL_CXX((1, 2)) void (__delrange__)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__delrange__", _Dee_HashSelectC(0x685a6ec8, 0xbd7df74412129f9d), 2, args)));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<Object> (__setrange__)(DeeObject *start, DeeObject *end, DeeObject *value) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "__setrange__", _Dee_HashSelectC(0x7f9874f9, 0x9da9fdb9a7e37ce8), 3, args));
	}
	WUNUSED Ref<Object> (__iterself__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__iterself__", _Dee_HashSelectC(0x4daaaed1, 0x5517f97b342fe2c1), 0, NULL));
	}
	WUNUSED Ref<Object> (__iternext__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__iternext__", _Dee_HashSelectC(0x29de3af1, 0xa4832d636bd81d42), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__getattr__)(DeeObject *name) {
		DeeObject *args[1];
		args[0] = name;
		return inherit(DeeObject_CallAttrStringHash(this, "__getattr__", _Dee_HashSelectC(0x59b442c5, 0xdef2290969a1663b), 1, args));
	}
	WUNUSED Ref<Object> (__getattr__)(char const *name) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getattr__", _Dee_HashSelectC(0x59b442c5, 0xdef2290969a1663b), "s", name));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Object> (__callattr__)(DeeObject *name, DeeObject *args) {
		DeeObject *args_[2];
		args_[0] = name;
		args_[1] = args;
		return inherit(DeeObject_CallAttrStringHash(this, "__callattr__", _Dee_HashSelectC(0xdb8ad67, 0x55d365851b01ce77), 2, args_));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Object> (__callattr__)(char const *name, DeeObject *args) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__callattr__", _Dee_HashSelectC(0xdb8ad67, 0x55d365851b01ce77), "so", name, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__hasattr__)(DeeObject *name) {
		DeeObject *args[1];
		args[0] = name;
		return inherit(DeeObject_CallAttrStringHash(this, "__hasattr__", _Dee_HashSelectC(0xca559b90, 0x8e9c1065fc0f38a5), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (__hasattr__)(char const *name) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__hasattr__", _Dee_HashSelectC(0xca559b90, 0x8e9c1065fc0f38a5), "s", name));
	}
	NONNULL_CXX((1)) void (__delattr__)(DeeObject *name) {
		DeeObject *args[1];
		args[0] = name;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__delattr__", _Dee_HashSelectC(0x4f3fb870, 0x15b0036f4684d4a7), 1, args)));
	}
	void (__delattr__)(char const *name) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delattr__", _Dee_HashSelectC(0x4f3fb870, 0x15b0036f4684d4a7), "s", name)));
	}
	NONNULL_CXX((1, 2)) void (__setattr__)(DeeObject *name, DeeObject *value) {
		DeeObject *args[2];
		args[0] = name;
		args[1] = value;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__setattr__", _Dee_HashSelectC(0x8a57f5f3, 0x8f1d3bac859d769d), 2, args)));
	}
	NONNULL_CXX((2)) void (__setattr__)(char const *name, DeeObject *value) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setattr__", _Dee_HashSelectC(0x8a57f5f3, 0x8f1d3bac859d769d), "so", name, value)));
	}
	WUNUSED Ref<Sequence<Object> > (__enumattr__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__enumattr__", _Dee_HashSelectC(0xcd281d49, 0x761caa648b619b9e), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<string> (__format__)(DeeObject *format) {
		DeeObject *args[1];
		args[0] = format;
		return inherit(DeeObject_CallAttrStringHash(this, "__format__", _Dee_HashSelectC(0xb59a1ae2, 0xdf14ed3788cde344), 1, args));
	}
	WUNUSED Ref<string> (__format__)(char const *format) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__format__", _Dee_HashSelectC(0xb59a1ae2, 0xdf14ed3788cde344), "s", format));
	}
	WUNUSED Ref<Object> (__move__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__move__", _Dee_HashSelectC(0xa04fc316, 0x9e97bf092c9c90d2), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__lt__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__lt__", _Dee_HashSelectC(0x121ff95f, 0x3a902a00b1c281dc), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__gt__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__gt__", _Dee_HashSelectC(0x17d0ec4e, 0x27b68eacce43548b), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (__not__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__not__", _Dee_HashSelectC(0x649d0c36, 0x2c7779d2eeb10015), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__is__)(DeeObject *tp) {
		DeeObject *args[1];
		args[0] = tp;
		return inherit(DeeObject_CallAttrStringHash(this, "__is__", _Dee_HashSelectC(0x2b1a70e0, 0xad244331897a16d8), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__deepequals__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__deepequals__", _Dee_HashSelectC(0x3313c27d, 0xe49a1925c263f351), 1, args));
	}
	WUNUSED Ref<Object> (__inc__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__inc__", _Dee_HashSelectC(0xd5dda33a, 0x79decd04e006c348), 0, NULL));
	}
	WUNUSED Ref<Object> (__dec__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__dec__", _Dee_HashSelectC(0xfaefcbae, 0x2b760c2bd9183e47), 0, NULL));
	}
	WUNUSED Ref<Object> (__incpost__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__incpost__", _Dee_HashSelectC(0x883cc053, 0xbea592bfd47b7365), 0, NULL));
	}
	WUNUSED Ref<Object> (__decpost__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__decpost__", _Dee_HashSelectC(0x4948ab96, 0x6ef9e5fc09db4cec), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__iadd__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__iadd__", _Dee_HashSelectC(0xa098243d, 0x285a2423f4071a26), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__isub__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__isub__", _Dee_HashSelectC(0x8ab9b3ac, 0xaeba4543ae375177), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__imul__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__imul__", _Dee_HashSelectC(0x147f6149, 0xfe507ea03c334483), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__idiv__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__idiv__", _Dee_HashSelectC(0xe6ec70cc, 0x25a04ba2340cfea4), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__imod__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__imod__", _Dee_HashSelectC(0xceed3226, 0x77990987221a348e), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__ishl__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__ishl__", _Dee_HashSelectC(0xe34046a4, 0xfb3c3eaf13c7ebf5), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__ishr__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__ishr__", _Dee_HashSelectC(0xa03408b4, 0x12df0bba41454989), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__iand__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__iand__", _Dee_HashSelectC(0x303cee11, 0x7247beb98b3e97fa), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__ior__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__ior__", _Dee_HashSelectC(0xcd0bcc8c, 0xeacfdc77e1d50bef), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__ixor__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__ixor__", _Dee_HashSelectC(0x4a21400c, 0x29dec6e135bac526), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__ipow__)(DeeObject *other) {
		DeeObject *args[1];
		args[0] = other;
		return inherit(DeeObject_CallAttrStringHash(this, "__ipow__", _Dee_HashSelectC(0xf77b6ecf, 0x401aa0d3bfcf83df), 1, args));
	}
	class _Wrap___itable__
		: public deemon::detail::ConstGetRefProxy<_Wrap___itable__, Sequence<> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___itable__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__itable__", _Dee_HashSelectC(0xb7ec355a, 0xbcabfd5c3d01dac0));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__itable__", _Dee_HashSelectC(0xb7ec355a, 0xbcabfd5c3d01dac0)));
		}
	};
	WUNUSED _Wrap___itable__ (__itable__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___module__
		: public deemon::detail::ConstGetRefProxy<_Wrap___module__, Object> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___module__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__module__", _Dee_HashSelectC(0x3bea6c9f, 0x183a20d7d6c28dbb));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__module__", _Dee_HashSelectC(0x3bea6c9f, 0x183a20d7d6c28dbb)));
		}
	};
	WUNUSED _Wrap___module__ (__module__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap___true_module__
		: public deemon::detail::ConstGetRefProxy<_Wrap___true_module__, Object> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___true_module__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__true_module__", _Dee_HashSelectC(0xca437364, 0x7a345f9bcf4328b6));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__true_module__", _Dee_HashSelectC(0xca437364, 0x7a345f9bcf4328b6)));
		}
	};
	WUNUSED _Wrap___true_module__ (__true_module__)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_id
		: public deemon::detail::ConstGetRefProxy<_Wrap_id, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_id(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "id", _Dee_HashSelectC(0x98768be1, 0x828b9fe0c4522be2));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "id", _Dee_HashSelectC(0x98768be1, 0x828b9fe0c4522be2)));
		}
	};
	WUNUSED _Wrap_id (id)() DEE_CXX_NOTHROW {
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

#endif /* !GUARD_DEEMON_CXX_OBJECT_H */
