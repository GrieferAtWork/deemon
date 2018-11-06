/* Copyright (c) 2018 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_CXX_OBJECT_H
#define GUARD_DEEMON_CXX_OBJECT_H 1

#include "api.h"
#include "../object.h"
#include "../bool.h"
#include "../tuple.h"
#include "../none.h"
#include "../error.h"
#include "../int.h"
#include "../float.h"
#include "../string.h"

#include <exception>
#include <type_traits>
#include <initializer_list>
#include <string.h>
#include <stddef.h>

DEE_CXX_BEGIN

FORCELOCAL DREF DeeObject *(DCALL incref)(DeeObject *__restrict obj) { Dee_Incref(obj); return obj; }
FORCELOCAL DREF DeeObject *(DCALL xincref)(DeeObject *obj) { Dee_XIncref(obj); return obj; }
FORCELOCAL void (DCALL decref)(DREF DeeObject *__restrict obj) { Dee_Decref(obj); }
FORCELOCAL void (DCALL decref_likely)(DREF DeeObject *__restrict obj) { Dee_Decref_likely(obj); }
FORCELOCAL void (DCALL decref_unlikely)(DREF DeeObject *__restrict obj) { Dee_Decref_unlikely(obj); }
FORCELOCAL void (DCALL decref_nokill)(DREF DeeObject *__restrict obj) { Dee_DecrefNokill(obj); }
FORCELOCAL void (DCALL decref_dokill)(DREF DeeObject *__restrict obj) { Dee_DecrefDokill(obj); }
FORCELOCAL bool (DCALL decref_ifone)(DREF DeeObject *__restrict obj) { return Dee_DecrefIfOne(obj); }
FORCELOCAL bool (DCALL decref_ifnotone)(DREF DeeObject *__restrict obj) { return Dee_DecrefIfNotOne(obj); }
FORCELOCAL void (DCALL xdecref)(DREF DeeObject *obj) { Dee_XDecref(obj); }
FORCELOCAL void (DCALL xdecref_nokill)(DREF DeeObject *obj) { Dee_XDecrefNokill(obj); }
#ifdef CONFIG_TRACE_REFCHANGES
FORCELOCAL DREF DeeObject *(DCALL incref_traced)(DeeObject *__restrict obj, char const *file, int line) { Dee_Incref_traced(obj,file,line); return obj; }
FORCELOCAL DREF DeeObject *(DCALL xincref_traced)(DeeObject *obj, char const *file, int line) { if (obj) Dee_Incref_traced(obj,file,line); return obj; }
FORCELOCAL DeeObject *(DCALL decref_traced)(DREF DeeObject *__restrict obj, char const *file, int line) { Dee_Decref_traced(obj,file,line); return obj; }
FORCELOCAL DeeObject *(DCALL decref_likely_traced)(DREF DeeObject *__restrict obj, char const *file, int line) { Dee_Decref_likely_traced(obj,file,line); return obj; }
FORCELOCAL DeeObject *(DCALL decref_unlikely_traced)(DREF DeeObject *__restrict obj, char const *file, int line) { Dee_Decref_unlikely_traced(obj,file,line); return obj; }
FORCELOCAL DeeObject *(DCALL decref_nokill_traced)(DREF DeeObject *__restrict obj, char const *file, int line) { Dee_DecrefNokill_traced(obj,file,line); return obj; }
FORCELOCAL void (DCALL decref_dokill_traced)(DREF DeeObject *__restrict obj, char const *file, int line) { Dee_DecrefDokill_traced(obj,file,line); }
FORCELOCAL bool (DCALL decref_ifone_traced)(DREF DeeObject *__restrict obj, char const *file, int line) { return Dee_DecrefIfOne_traced(obj,file,line); }
FORCELOCAL bool (DCALL decref_ifnotone_traced)(DREF DeeObject *__restrict obj, char const *file, int line) { return Dee_DecrefIfNotOne_traced(obj,file,line); }
FORCELOCAL void (DCALL xdecref_traced)(DREF DeeObject *obj, char const *file, int line) { if (obj) Dee_Decref_traced(obj,file,line); }
FORCELOCAL void (DCALL xdecref_nokill_traced)(DREF DeeObject *obj, char const *file, int line) { if (obj) Dee_DecrefNokill_traced(obj,file,line); }
#define incref(obj)          incref_traced(obj,__FILE__,__LINE__)
#define xincref(obj)         xincref_traced(obj,__FILE__,__LINE__)
#define decref(obj)          decref_traced(obj,__FILE__,__LINE__)
#define decref_nokill(obj)   decref_nokill_traced(obj,__FILE__,__LINE__)
#define decref_dokill(obj)   decref_dokill_traced(obj,__FILE__,__LINE__)
#define decref_ifone(obj)    decref_ifone_traced(obj,__FILE__,__LINE__)
#define decref_ifnotone(obj) decref_ifnotone_traced(obj,__FILE__,__LINE__)
#define xdecref(obj)         xdecref_traced(obj,__FILE__,__LINE__)
#define xdecref_nokill(obj)  xdecrefnokill_traced(obj,__FILE__,__LINE__)
#endif

/* Throws the latest deemon exception as a C++ error. */
inline ATTR_NORETURN ATTR_COLD void DCALL throw_last_deemon_exception(void);
inline ATTR_RETNONNULL void *DCALL throw_if_null(void *ptr) { if unlikely(!ptr) throw_last_deemon_exception(); return ptr; }
inline ATTR_RETNONNULL DeeObject *DCALL throw_if_null(DeeObject *obj) { if unlikely(!obj) throw_last_deemon_exception(); return obj; }
inline unsigned int DCALL throw_if_negative(int x) { if unlikely(x < 0) throw_last_deemon_exception(); return (unsigned int)x; }
inline unsigned long DCALL throw_if_negative(long x) { if unlikely(x < 0l) throw_last_deemon_exception(); return (unsigned long)x; }
#ifdef __COMPILER_HAVE_LONGLONG
inline unsigned long long DCALL throw_if_negative(long long x) { if unlikely(x < 0ll) throw_last_deemon_exception(); return (unsigned long long)x; }
#endif /* __COMPILER_HAVE_LONGLONG */
inline unsigned int DCALL throw_if_nonzero(int x) { if unlikely(x != 0) throw_last_deemon_exception(); return (unsigned int)x; }
inline unsigned int DCALL throw_if_minusone(unsigned int x) { if unlikely(x == (unsigned int)-1) throw_last_deemon_exception(); return x; }
inline unsigned long DCALL throw_if_minusone(unsigned long x) { if unlikely(x == (unsigned long)-1l) throw_last_deemon_exception(); return x; }
#ifdef __COMPILER_HAVE_LONGLONG
inline unsigned long long DCALL throw_if_minusone(unsigned long long x) { if unlikely(x == (unsigned long long)-1ll) throw_last_deemon_exception(); return x; }
#endif /* __COMPILER_HAVE_LONGLONG */

#define DEFINE_OBJECT_WRAPPER(name) \
class name { \
    DeeObject *m_ptr; \
public: \
    operator DeeObject *() const { return m_ptr; } \
    explicit name(DeeObject *ptr) DEE_CXX_NOTHROW: m_ptr(ptr) { } \
    name(name const &ptr) DEE_CXX_NOTHROW: m_ptr(ptr.m_ptr) { } \
}; \
/**/
DEFINE_OBJECT_WRAPPER(obj_nonnull)
DEFINE_OBJECT_WRAPPER(obj_maybenull)
DEFINE_OBJECT_WRAPPER(obj_inherited)
DEFINE_OBJECT_WRAPPER(obj_nonnull_inherited)
DEFINE_OBJECT_WRAPPER(obj_maybenull_inherited)
DEFINE_OBJECT_WRAPPER(obj_string)
DEFINE_OBJECT_WRAPPER(obj_tuple)
DEFINE_OBJECT_WRAPPER(obj_sequence)
#undef DEFINE_OBJECT_WRAPPER


class exception: public std::exception {
public:
	virtual const char *what() const DEE_CXX_NOTHROW {
        DeeObject *current = DeeError_Current();
        return current ? Dee_TYPE(current)->tp_name : "No exception";
    }
};



/* Indicate that is-NULL checks to throw an exception can
 * be omitted, because an object pointer is never NULL. */
inline WUNUSED NONNULL((1)) obj_nonnull DCALL nonnull(DeeObject *__restrict ptr) { return obj_nonnull(ptr); }
inline WUNUSED obj_nonnull DCALL nonnull(obj_nonnull ptr) { return obj_nonnull((DeeObject *)ptr); }
inline WUNUSED obj_nonnull_inherited DCALL nonnull(obj_inherited ptr) { return obj_nonnull_inherited((DeeObject *)ptr); }
inline WUNUSED obj_nonnull_inherited DCALL nonnull(obj_nonnull_inherited ptr) { return obj_nonnull_inherited((DeeObject *)ptr); }

/* Indicate that is-NULL checks to throw an exception can
 * be omitted, because a NULL-object is allowed. */
inline WUNUSED obj_maybenull DCALL maybenull(DeeObject *ptr) { return obj_maybenull(ptr); }
inline WUNUSED obj_maybenull DCALL maybenull(obj_maybenull ptr) { return obj_maybenull((DeeObject *)ptr); }
inline WUNUSED obj_maybenull_inherited DCALL maybenull(obj_inherited ptr) { return obj_maybenull_inherited((DeeObject *)ptr); }
inline WUNUSED obj_maybenull_inherited DCALL maybenull(obj_maybenull_inherited ptr) { return obj_maybenull_inherited((DeeObject *)ptr); }

/* Indicate that an object reference should be inherited. */
inline WUNUSED NONNULL((1)) obj_inherited DCALL inherit(DeeObject *__restrict ptr) { return obj_inherited(ptr); }
inline WUNUSED obj_inherited DCALL inherit(obj_inherited ptr) { return obj_inherited((DeeObject *)ptr); }
inline WUNUSED obj_nonnull_inherited DCALL inherit(obj_nonnull ptr) { return obj_nonnull_inherited((DeeObject *)ptr); }
inline WUNUSED obj_nonnull_inherited DCALL inherit(obj_nonnull_inherited ptr) { return obj_nonnull_inherited((DeeObject *)ptr); }

class object;
class string;
class super;
template<class T = object> class type;
template<class T = object> class iterator;
template<class T = object> class sequence;
template<class T = object> class tuple;
template<class Prototype> class function;
class int_;
class float_;

namespace detail {

class object_base {
protected:
    DREF DeeObject *m_ptr; /* [1..1][lock(CALLER)] The represented object. */
public:
#define DEFINE_OBJECT_CONSTRUCTORS(T,superT) \
    T(T &&right) DEE_CXX_NOTHROW: superT(std::move((superT &)right)) { } \
    T(T const &right) DEE_CXX_NOTHROW: superT(right) { } \
    T(DeeObject *obj) DEE_CXX_NOTHROW: superT(obj) { } \
    T(obj_nonnull obj) DEE_CXX_NOTHROW: superT(obj) { } \
    T(obj_maybenull obj) DEE_CXX_NOTHROW: superT(obj) { } \
    T(obj_inherited obj) DEE_CXX_NOTHROW: superT(obj) { } \
    T(obj_nonnull_inherited obj) DEE_CXX_NOTHROW: superT(obj) { } \
    T(obj_maybenull_inherited obj) DEE_CXX_NOTHROW: superT(obj) { } \
    T &operator = (T &&right) DEE_CXX_NOTHROW { superT::operator = (std::move((superT &)right)); return *this; } \
    T &operator = (T const &right) DEE_CXX_NOTHROW { superT::operator = (right); return *this; } \
    T copy() const { return inherit(DeeObject_Copy(*this)); } \
    T deepcopy() const { return inherit(DeeObject_DeepCopy(*this)); } \
    T &inplace_deepcopy() { throw_if_nonzero(DeeObject_InplaceDeepCopy(&this->m_ptr)); return *this; } \
/**/
    object_base() DEE_CXX_NOTHROW: m_ptr(NULL) { }
    object_base(object_base &&right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr) { right.m_ptr = NULL; }
    object_base(object_base const &right) DEE_CXX_NOTHROW: m_ptr(incref(right.m_ptr)) { }
    object_base(DeeObject *obj): m_ptr(incref(throw_if_null(obj))) { }
    object_base(obj_nonnull obj) DEE_CXX_NOTHROW: m_ptr(incref(obj)) { }
    object_base(obj_maybenull obj) DEE_CXX_NOTHROW: m_ptr(xincref(obj)) { }
    object_base(obj_inherited obj): m_ptr(throw_if_null(obj)) { }
    object_base(obj_nonnull_inherited obj) DEE_CXX_NOTHROW: m_ptr(obj) { }
    object_base(obj_maybenull_inherited obj) DEE_CXX_NOTHROW: m_ptr(obj) { }
    ~object_base() DEE_CXX_NOTHROW { Dee_XDecref(m_ptr); }

    DeeObject *(ptr)() const DEE_CXX_NOTHROW { return m_ptr; }
    operator DeeObject *(void) const DEE_CXX_NOTHROW { return m_ptr; }
    bool (isnull)() const DEE_CXX_NOTHROW { return m_ptr == NULL; }
    bool (isnone)() const DEE_CXX_NOTHROW { return DeeNone_Check(m_ptr); }
    bool (is)(DeeTypeObject *__restrict tp) const DEE_CXX_NOTHROW { return DeeObject_InstanceOf(m_ptr,tp); }
    bool (isexact)(DeeTypeObject *__restrict tp) const DEE_CXX_NOTHROW { return DeeObject_InstanceOfExact(m_ptr,tp); }
    template<class T> bool (is)() const DEE_CXX_NOTHROW { return T::check(this->ptr()); }
    template<class T> bool (isexact)() const DEE_CXX_NOTHROW { return T::checkexact(this->ptr()); }
    object_base &operator = (object_base &&right) DEE_CXX_NOTHROW { Dee_XDecref(this->m_ptr); this->m_ptr = right.m_ptr; right.m_ptr = NULL; return *this; }
    object_base &operator = (object_base const &right) DEE_CXX_NOTHROW { Dee_XDecref(this->m_ptr); this->m_ptr = xincref(right.m_ptr); return *this; }
    object_base &operator = (DeeObject *obj) { throw_if_null(obj); xdecref(m_ptr); m_ptr = incref(obj); return *this; }
    object_base &operator = (obj_nonnull obj) DEE_CXX_NOTHROW { xdecref(m_ptr); m_ptr = incref(obj); return *this; }
    object_base &operator = (obj_maybenull obj) DEE_CXX_NOTHROW { xdecref(m_ptr); m_ptr = xincref(obj); return *this; }
    object_base &operator = (obj_inherited obj) { throw_if_null(obj); xdecref(m_ptr); m_ptr = incref(obj); return *this; }
    object_base &operator = (obj_nonnull_inherited obj) DEE_CXX_NOTHROW { xdecref(m_ptr); m_ptr = obj; return *this; }
    object_base &operator = (obj_maybenull_inherited obj) DEE_CXX_NOTHROW { xdecref(m_ptr); m_ptr = obj; return *this; }
};

template<class T>
class cxx_iterator {
private:
    DREF DeeObject *it_iter; /* [0..1] Underlying iterator. */
    DREF DeeObject *it_next; /* [0..1] Next element (NULL if not queried). */
public:
    ~cxx_iterator() DEE_CXX_NOTHROW { Dee_XDecref(it_iter); if (ITER_ISOK(it_next)) Dee_Decref(it_next); }
    cxx_iterator() DEE_CXX_NOTHROW: it_iter(NULL), it_next(NULL) { }
    cxx_iterator(cxx_iterator &&right) DEE_CXX_NOTHROW: it_iter(right.it_iter), it_next(right.it_next) { right.it_iter = NULL; right.it_next = NULL; }
    cxx_iterator(cxx_iterator const &right) DEE_CXX_NOTHROW: it_iter(xincref(right.it_iter)), it_next(right.it_next) { if (ITER_ISOK(it_next)) Dee_Incref(it_next); }
    cxx_iterator(DeeObject *iter): it_iter(throw_if_null(iter)), it_next(throw_if_null(DeeObject_IterNext(iter))) { Dee_Incref(it_iter); }
    cxx_iterator(obj_inherited iter): it_iter(throw_if_null(iter)), it_next(DeeObject_IterNext(iter)) { if unlikely(!it_next) { Dee_Decref((DeeObject *)iter); throw_last_deemon_exception(); } }
    cxx_iterator(obj_nonnull iter) DEE_CXX_NOTHROW: it_iter(iter), it_next(throw_if_null(DeeObject_IterNext(iter))) { Dee_Incref((DeeObject *)iter); }
    cxx_iterator(obj_nonnull_inherited iter) DEE_CXX_NOTHROW: it_iter(iter), it_next(DeeObject_IterNext(iter)) { if unlikely(!it_next) { Dee_Decref((DeeObject *)iter); throw_last_deemon_exception(); } }
    bool operator == (cxx_iterator const &right) const { return !right.it_iter ? it_next == ITER_DONE : throw_if_negative(DeeObject_CompareEq(it_iter,right.it_iter)) != 0; }
    bool operator != (cxx_iterator const &right) const { return !right.it_iter ? it_next != ITER_DONE : throw_if_negative(DeeObject_CompareNe(it_iter,right.it_iter)) != 0; }
    bool operator < (cxx_iterator const &right) const { return !right.it_iter ? it_next != ITER_DONE : throw_if_negative(DeeObject_CompareLo(it_iter,right.it_iter)) != 0; }
    bool operator <= (cxx_iterator const &right) const { return !right.it_iter ? true : throw_if_negative(DeeObject_CompareLe(it_iter,right.it_iter)) != 0; }
    bool operator > (cxx_iterator const &right) const { return !right.it_iter ? false : throw_if_negative(DeeObject_CompareGr(it_iter,right.it_iter)) != 0; }
    bool operator >= (cxx_iterator const &right) const { return !right.it_iter ? it_next == ITER_DONE : throw_if_negative(DeeObject_CompareGe(it_iter,right.it_iter)) != 0; }
    cxx_iterator &operator = (cxx_iterator &&right) DEE_CXX_NOTHROW {
        Dee_XDecref(it_iter);
        if (ITER_ISOK(it_next))
            Dee_Decref(it_next);
        it_iter = right.it_iter;
        it_next = right.it_next;
        right.it_iter = NULL;
        right.it_next = NULL;
        return *this;
    }
    cxx_iterator &operator = (cxx_iterator const &right) {
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
    cxx_iterator copy() const { return inherit(DeeObject_Copy(this->it_iter)); }
    cxx_iterator deepcopy() const { return inherit(DeeObject_DeepCopy(this->it_iter)); }
    cxx_iterator &inplace_deepcopy() { throw_if_nonzero(DeeObject_InplaceDeepCopy(&this->it_iter)); return this->it_iter; }
    T operator * () { return it_next; }
    cxx_iterator &operator ++ () {
        if (ITER_ISOK(it_next))
            Dee_Decref(it_next);
        it_next = throw_if_null(DeeObject_IterNext(this->it_iter));
        return *this;
    }
    cxx_iterator operator ++ (int) {
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
    enum{exists = true};
};
template<class T, bool C> class any_convertible_cc { enum{exists = false}; };
template<class T> class any_convertible_cc<T,true>: public any_convertible_base {
public:
    static DREF DeeObject *convert(T const &value) {
        DREF DeeObject *result;
        result = (DREF DeeObject *)value;
        Dee_Incref(result);
        return result;
    }
};
template<class T> class any_convertible: public any_convertible_cc<T,std::is_convertible<T,DeeObject *>::value> { };

template<> class any_convertible<char>: public any_convertible_base { public: static DREF DeeObject *convert(char value) { return DeeInt_NewChar(value); } };
template<> class any_convertible<signed char>: public any_convertible_base { public: static DREF DeeObject *convert(signed char value) { return DeeInt_NewSChar(value); } };
template<> class any_convertible<unsigned char>: public any_convertible_base { public: static DREF DeeObject *convert(unsigned char value) { return DeeInt_NewUChar(value); } };
template<> class any_convertible<short>: public any_convertible_base { public: static DREF DeeObject *convert(short value) { return DeeInt_NewShort(value); } };
template<> class any_convertible<unsigned short>: public any_convertible_base { public: static DREF DeeObject *convert(unsigned short value) { return DeeInt_NewUShort(value); } };
template<> class any_convertible<int>: public any_convertible_base { public: static DREF DeeObject *convert(int value) { return DeeInt_NewInt(value); } };
template<> class any_convertible<unsigned int>: public any_convertible_base { public: static DREF DeeObject *convert(unsigned int value) { return DeeInt_NewUInt(value); } };
template<> class any_convertible<long>: public any_convertible_base { public: static DREF DeeObject *convert(long value) { return DeeInt_NewLong(value); } };
template<> class any_convertible<unsigned long>: public any_convertible_base { public: static DREF DeeObject *convert(unsigned long value) { return DeeInt_NewULong(value); } };
#ifdef __COMPILER_HAVE_LONGLONG
template<> class any_convertible<long long>: public any_convertible_base { public: static DREF DeeObject *convert(long value) { return DeeInt_NewLLong(value); } };
template<> class any_convertible<unsigned long long>: public any_convertible_base { public: static DREF DeeObject *convert(unsigned long value) { return DeeInt_NewULLong(value); } };
#endif /* __COMPILER_HAVE_LONGLONG */                                                                            
template<> class any_convertible<dint128_t>: public any_convertible_base { public: static DREF DeeObject *convert(dint128_t value) { return DeeInt_NewS128(value); } };
template<> class any_convertible<duint128_t>: public any_convertible_base { public: static DREF DeeObject *convert(duint128_t value) { return DeeInt_NewU128(value); } };
template<> class any_convertible<float>: public any_convertible_base { public: static DREF DeeObject *convert(float value) { return DeeFloat_New((double)value); } };
template<> class any_convertible<double>: public any_convertible_base { public: static DREF DeeObject *convert(double value) { return DeeFloat_New(value); } };
template<> class any_convertible<long double>: public any_convertible_base { public: static DREF DeeObject *convert(long double value) { return DeeFloat_New((double)value); } };
template<> class any_convertible</*utf-8*/char const *>: public any_convertible_base { public: static DREF DeeObject *convert(/*utf-8*/char const *__restrict value) { return DeeString_NewUtf8(value,strlen(value),STRING_ERROR_FSTRICT); } };
#ifdef CONFIG_DEEMON_HAVE_NATIVE_WCHAR_T
template<> class any_convertible</*wide*/dwchar_t const *>: public any_convertible_base { public: static DREF DeeObject *convert(/*wide*/dwchar_t const *__restrict value) { return DeeString_NewWide(value,wcslen(value),STRING_ERROR_FSTRICT); } };
#endif
#undef DEFINE_CONVERTIBLE
template<size_t sz>
class any_convertible<char const[sz]>: public any_convertible_base {
public:
    static DREF DeeObject *convert(/*utf-8*/char const (&value)[sz]) {
        return DeeString_NewUtf8(value,sz - 1,STRING_ERROR_FSTRICT);
    }
};
template<size_t sz> class any_convertible<char[sz]>: public any_convertible<char const[sz]> { };
#ifdef CONFIG_DEEMON_HAVE_NATIVE_WCHAR_T
template<size_t sz>
class any_convertible<dwchar_t const[sz]>: public any_convertible_base {
public:
    static DREF DeeObject *convert(/*utf-8*/dwchar_t const (&value)[sz]) {
        return DeeString_NewWide(value,sz - 1,STRING_ERROR_FSTRICT);
    }
};
template<size_t sz> class any_convertible<dwchar_t[sz]>: public any_convertible<dwchar_t const[sz]> { };
#endif

template<class T>
class any_convertible<std::initializer_list<T> > {
public:
    typedef typename any_convertible<T>::available available;
    static DREF DeeObject *convert(std::initializer_list<T> const &values) {
        DREF DeeObject *result;
        T const *elem = values.begin();
        size_t i = 0,size = values.size();
        result = throw_if_null(DeeTuple_NewUninitialized(size));
        try {
            for (; i < size; ++i) {
                DREF DeeObject *temp;
                temp = any_convertible<T>::convert(elem[i]);
                DeeTuple_SET(result,i,temp);
            }
        } catch (...) {
            while (i--)
                Dee_Decref(DeeTuple_GET(result,i));
            DeeTuple_FreeUninitialized(result);
            throw;
        }
        return result;
    }
};

} /* namespace intern... */



class object: public detail::object_base {
    template<class T> class proxy_base {
    public:
        object get() const { return inherit(((T const *)this)->getref()); }
        operator object() const { return inherit(((T const *)this)->getref()); }
        void set(DeeObject *__restrict value) const { *((T const *)this) = value; }
    };
    template<class T> class call_proxy_base: public proxy_base<T> {
    public:
        object call() const { return inherit(((T const *)this)->callref()); }
        object call(obj_tuple args) const { return inherit(((T const *)this)->callref(args)); }
        object call(size_t argc, DeeObject **__restrict argv) const { return inherit(((T const *)this)->callref(argc,argv)); }
        object call(size_t argc, DeeObject *const *__restrict argv) const { return inherit(((T const *)this)->callref(argc,(DeeObject **)argv)); }
        object call(size_t argc, object **__restrict argv) const { return inherit(((T const *)this)->callref(argc,(DeeObject **)argv)); }
        object call(size_t argc, object *const *__restrict argv) const { return inherit(((T const *)this)->callref(argc,(DeeObject **)argv)); }
        object call(std::initializer_list<DeeObject *> const &args) const { return inherit(((T const *)this)->callref(args.size(),(DeeObject **)args.begin())); }
        WUNUSED object operator ()(obj_tuple args) const { return inherit(((T const *)this)->callref(args)); }
        WUNUSED object operator ()(std::initializer_list<DeeObject *> const &args) const { return inherit(((T const *)this)->callref(args.size(),(DeeObject **)args.begin())); }
    };
    class attr_proxy_obj: public call_proxy_base<attr_proxy_obj> {
    private:
        DeeObject *m_ptr;
        DeeObject *m_str;
    public:
        attr_proxy_obj(DeeObject *ptr, DeeObject *str) DEE_CXX_NOTHROW: m_ptr(ptr), m_str(str) {}
        attr_proxy_obj(attr_proxy_obj const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr), m_str(right.m_str) {}
        WUNUSED string doc() const;
        WUNUSED DREF DeeObject *getref() const { return DeeObject_GetAttr(m_ptr,m_str); }
        WUNUSED DREF DeeObject *callref() const { return DeeObject_CallAttr(m_ptr,m_str,0,NULL); }
        WUNUSED DREF DeeObject *callref(obj_tuple args) const { return DeeObject_CallAttrTuple(m_ptr,m_str,args); }
        WUNUSED DREF DeeObject *callref(size_t argc, DeeObject **__restrict argv) const { return DeeObject_CallAttr(m_ptr,m_str,argc,argv); }
        bool has() const { return throw_if_negative(DeeObject_HasAttr(m_ptr,m_str)) != 0; }
        bool bound() const { int result = DeeObject_BoundAttr(m_ptr,m_str); if (result == -1) throw_last_deemon_exception(); return result > 0; }
        void del() const { throw_if_nonzero(DeeObject_DelAttr(m_ptr,m_str)); }
        attr_proxy_obj const &operator = (DeeObject *__restrict value) const { throw_if_nonzero(DeeObject_SetAttr(m_ptr,m_str,value)); return *this; }
    };
    class attr_proxy_str: public call_proxy_base<attr_proxy_str> {
    private:
        DeeObject  *m_ptr;
        char const *m_str;
    public:
        attr_proxy_str(DeeObject *ptr, char const *str) DEE_CXX_NOTHROW: m_ptr(ptr), m_str(str) {}
        attr_proxy_str(attr_proxy_str const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr), m_str(right.m_str) {}
        DREF DeeObject *getref() const { return DeeObject_GetAttrString(m_ptr,m_str); }
        string doc() const;
        WUNUSED DREF DeeObject *callref() const { return DeeObject_CallAttrString(m_ptr,m_str,0,NULL); }
        WUNUSED DREF DeeObject *callref(obj_tuple args) const { return DeeObject_CallAttrString(m_ptr,m_str,DeeTuple_SIZE((DeeObject *)args),DeeTuple_ELEM((DeeObject *)args)); }
        WUNUSED DREF DeeObject *callref(size_t argc, DeeObject **__restrict argv) const { return DeeObject_CallAttrString(m_ptr,m_str,argc,argv); }
        bool has() const { return throw_if_negative(DeeObject_HasAttrString(m_ptr,m_str)) != 0; }
        bool bound() const { int result = DeeObject_BoundAttrString(m_ptr,m_str); if (result == -1) throw_last_deemon_exception(); return result > 0; }
        void del() const { throw_if_nonzero(DeeObject_DelAttrString(m_ptr,m_str)); }
        attr_proxy_str const &operator = (DeeObject *__restrict value) const { throw_if_nonzero(DeeObject_SetAttrString(m_ptr,m_str,value)); return *this; }
    };
    class attr_proxy_sth: public call_proxy_base<attr_proxy_sth> {
    private:
        DeeObject  *m_ptr;
        char const *m_str;
        dhash_t     m_hsh;
    public:
        attr_proxy_sth(DeeObject *ptr, char const *str, dhash_t hsh) DEE_CXX_NOTHROW: m_ptr(ptr), m_str(str), m_hsh(hsh) {}
        attr_proxy_sth(attr_proxy_sth const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr), m_str(right.m_str), m_hsh(right.m_hsh) {}
        DREF DeeObject *getref() const { return DeeObject_GetAttrStringHash(m_ptr,m_str,m_hsh); }
        string doc() const;
        WUNUSED DREF DeeObject *callref() const { return DeeObject_CallAttrStringHash(m_ptr,m_str,m_hsh,0,NULL); }
        WUNUSED DREF DeeObject *callref(obj_tuple args) const { return DeeObject_CallAttrStringHash(m_ptr,m_str,m_hsh,DeeTuple_SIZE((DeeObject *)args),DeeTuple_ELEM((DeeObject *)args)); }
        WUNUSED DREF DeeObject *callref(size_t argc, DeeObject **__restrict argv) const { return DeeObject_CallAttrStringHash(m_ptr,m_str,m_hsh,argc,argv); }
        bool has() const { return throw_if_negative(DeeObject_HasAttrStringHash(m_ptr,m_str,m_hsh)) != 0; }
        bool bound() const { int result = DeeObject_BoundAttrStringHash(m_ptr,m_str,m_hsh); if (result == -1) throw_last_deemon_exception(); return result > 0; }
        void del() const { throw_if_nonzero(DeeObject_DelAttrStringHash(m_ptr,m_str,m_hsh)); }
        attr_proxy_sth const &operator = (DeeObject *__restrict value) const { throw_if_nonzero(DeeObject_SetAttrStringHash(m_ptr,m_str,m_hsh,value)); return *this; }
    };
    class item_proxy_obj: public proxy_base<item_proxy_obj> {
    private:
        DeeObject *m_ptr;
        DeeObject *m_str;
    public:
        item_proxy_obj(DeeObject *ptr, DeeObject *str) DEE_CXX_NOTHROW: m_ptr(ptr), m_str(str) {}
        item_proxy_obj(item_proxy_obj const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr), m_str(right.m_str) {}
        DREF DeeObject *getref() const { return DeeObject_GetItem(m_ptr,m_str); }
        DREF DeeObject *getref_def(DeeObject *__restrict def) const { return DeeObject_GetItemDef(m_ptr,m_str,def); }
        object getdef(DeeObject *__restrict def) const { return inherit(getref_def(def)); }
        void del() const { throw_if_nonzero(DeeObject_DelItem(m_ptr,m_str)); }
        item_proxy_obj const &operator = (DeeObject *__restrict value) const { throw_if_nonzero(DeeObject_SetItem(m_ptr,m_str,value)); return *this; }
    };
    class item_proxy_idx: public proxy_base<item_proxy_idx> {
    private:
        DeeObject *m_ptr;
        size_t     m_idx;
    public:
        item_proxy_idx(DeeObject *ptr, size_t idx) DEE_CXX_NOTHROW: m_ptr(ptr), m_idx(idx) {}
        item_proxy_idx(item_proxy_idx const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr), m_idx(right.m_idx) {}
        DREF DeeObject *getref() const { return DeeObject_GetItemIndex(m_ptr,m_idx); }
        DREF DeeObject *getref_def(DeeObject *__restrict def) const { DREF DeeObject *result,*index_ob = throw_if_null(DeeInt_NewSize(m_idx)); result = DeeObject_GetItemDef(m_ptr,index_ob,def); Dee_Decref(index_ob); return inherit(result); }
        object getdef(DeeObject *__restrict def) const { return inherit(getref_def(def)); }
        void del() const { throw_if_nonzero(DeeObject_DelItemIndex(m_ptr,m_idx)); }
        item_proxy_idx const &operator = (DeeObject *__restrict value) const { throw_if_nonzero(DeeObject_SetItemIndex(m_ptr,m_idx,value)); return *this; }
    };
    class item_proxy_sth: public proxy_base<item_proxy_sth> {
    private:
        DeeObject  *m_ptr;
        char const *m_str;
        dhash_t     m_hsh;
    public:
        item_proxy_sth(DeeObject *ptr, char const *str) DEE_CXX_NOTHROW: m_ptr(ptr), m_str(str), m_hsh(hash_str(str)) {}
        item_proxy_sth(DeeObject *ptr, char const *str, dhash_t hsh) DEE_CXX_NOTHROW: m_ptr(ptr), m_str(str), m_hsh(hsh) {}
        item_proxy_sth(item_proxy_sth const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr), m_str(right.m_str), m_hsh(right.m_hsh) {}
        DREF DeeObject *getref() const { return DeeObject_GetItemString(m_ptr,m_str,m_hsh); }
        DREF DeeObject *getref_def(DeeObject *__restrict def) const { return DeeObject_GetItemStringDef(m_ptr,m_str,m_hsh,def); }
        object getdef(DeeObject *__restrict def) const { return inherit(getref_def(def)); }
        void del() const { throw_if_nonzero(DeeObject_DelItemString(m_ptr,m_str,m_hsh)); }
        item_proxy_sth const &operator = (DeeObject *__restrict value) const { throw_if_nonzero(DeeObject_SetItemString(m_ptr,m_str,m_hsh,value)); return *this; }
    };
    class range_proxy_oo: public proxy_base<range_proxy_oo> {
    private:
        DeeObject  *m_ptr;
        DeeObject  *m_bgn;
        DeeObject  *m_end;
    public:
        range_proxy_oo(DeeObject *ptr, DeeObject *bgn, DeeObject *end) DEE_CXX_NOTHROW: m_ptr(ptr), m_bgn(bgn), m_end(end) {}
        range_proxy_oo(range_proxy_oo const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr), m_bgn(right.m_bgn), m_end(right.m_end) {}
        DREF DeeObject *getref() const { return DeeObject_GetRange(m_ptr,m_bgn,m_end); }
        void del() const { throw_if_nonzero(DeeObject_DelRange(m_ptr,m_bgn,m_end)); }
        range_proxy_oo const &operator = (DeeObject *__restrict value) const { throw_if_nonzero(DeeObject_SetRange(m_ptr,m_bgn,m_end,value)); return *this; }
    };
    class range_proxy_io: public proxy_base<range_proxy_io> {
    private:
        DeeObject  *m_ptr;
        size_t      m_bgn;
        DeeObject  *m_end;
    public:
        range_proxy_io(DeeObject *ptr, size_t bgn, DeeObject *end) DEE_CXX_NOTHROW: m_ptr(ptr), m_bgn(bgn), m_end(end) {}
        range_proxy_io(range_proxy_io const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr), m_bgn(right.m_bgn), m_end(right.m_end) {}
        DREF DeeObject *getref() const { return DeeObject_GetRangeBeginIndex(m_ptr,m_bgn,m_end); }
        void del() const { DREF DeeObject *begin_ob = throw_if_null(DeeInt_NewSize(m_bgn)); int error = DeeObject_DelRange(m_ptr,begin_ob,m_end); Dee_Decref(begin_ob); throw_if_nonzero(error); }
        range_proxy_io const &operator = (DeeObject *__restrict value) const { throw_if_nonzero(DeeObject_SetRangeBeginIndex(m_ptr,m_bgn,m_end,value)); return *this; }
    };
    class range_proxy_oi: public proxy_base<range_proxy_oi> {
    private:
        DeeObject  *m_ptr;
        DeeObject  *m_bgn;
        size_t      m_end;
    public:
        range_proxy_oi(DeeObject *ptr, DeeObject *bgn, size_t end) DEE_CXX_NOTHROW: m_ptr(ptr), m_bgn(bgn), m_end(end) {}
        range_proxy_oi(range_proxy_oi const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr), m_bgn(right.m_bgn), m_end(right.m_end) {}
        DREF DeeObject *getref() const { return DeeObject_GetRangeEndIndex(m_ptr,m_bgn,m_end); }
        void del() const { DREF DeeObject *end_ob = throw_if_null(DeeInt_NewSize(m_end)); int error = DeeObject_DelRange(m_ptr,m_bgn,end_ob); Dee_Decref(end_ob); throw_if_nonzero(error); }
        range_proxy_oi const &operator = (DeeObject *__restrict value) const { throw_if_nonzero(DeeObject_SetRangeEndIndex(m_ptr,m_bgn,m_end,value)); return *this; }
    };
    class range_proxy_ii: public proxy_base<range_proxy_ii> {
    private:
        DeeObject  *m_ptr;
        size_t      m_bgn;
        size_t      m_end;
    public:
        range_proxy_ii(DeeObject *ptr, size_t bgn, size_t end) DEE_CXX_NOTHROW: m_ptr(ptr), m_bgn(bgn), m_end(end) {}
        range_proxy_ii(range_proxy_ii const &right) DEE_CXX_NOTHROW: m_ptr(right.m_ptr), m_bgn(right.m_bgn), m_end(right.m_end) {}
        DREF DeeObject *getref() const { return DeeObject_GetRangeIndex(m_ptr,m_bgn,m_end); }
        void del() const { DREF DeeObject *begin_ob = throw_if_null(DeeInt_NewSize(m_bgn)),*end_ob; end_ob = DeeInt_NewSize(m_end); if unlikely(!end_ob) { Dee_Decref(begin_ob); throw_last_deemon_exception(); } int error = DeeObject_DelRange(m_ptr,begin_ob,end_ob); Dee_Decref(end_ob); Dee_Decref(begin_ob); throw_if_nonzero(error); }
        range_proxy_ii const &operator = (DeeObject *__restrict value) const { throw_if_nonzero(DeeObject_SetRangeIndex(m_ptr,m_bgn,m_end,value)); return *this; }
    };
public:
    static DeeTypeObject *classtype() DEE_CXX_NOTHROW { return &DeeObject_Type; }
    static bool check(DeeObject *__restrict UNUSED(ob)) DEE_CXX_NOTHROW { return true; }
    static bool checkexact(DeeObject *__restrict UNUSED(ob)) DEE_CXX_NOTHROW { return true; }
public:
    template<class T> object(T const &init, typename detail::any_convertible<T>::available* =0)
        : object_base(inherit(detail::any_convertible<T>::convert(init))) { }
    template<class T> object(std::initializer_list<T> const &init, typename detail::any_convertible<std::initializer_list<T> >::available* =0)
        : object_base(inherit(detail::any_convertible<std::initializer_list<T> >::convert(init))) { }
    DEFINE_OBJECT_CONSTRUCTORS(object,object_base)
    attr_proxy_obj attr(obj_string name) const { return attr_proxy_obj(*this,name); }
    attr_proxy_str attr(char const *__restrict name) const { return attr_proxy_str(*this,name); }
    attr_proxy_sth attr(char const *__restrict name, dhash_t hash) const { return attr_proxy_sth(*this,name,hash); }
    object getattr(obj_string name) const { return inherit(DeeObject_GetAttr(*this,name)); }
    object getattr(char const *__restrict name) const { return inherit(DeeObject_GetAttrString(*this,name)); }
    object getattr(char const *__restrict name, dhash_t hash) const { return inherit(DeeObject_GetAttrStringHash(*this,name,hash)); }
    string docattr(obj_string name) const;
    string docattr(char const *__restrict name) const;
    string docattr(char const *__restrict name, dhash_t hash) const;
    bool hasattr(obj_string name) const { return throw_if_negative(DeeObject_HasAttr(*this,name)) != 0; }
    bool hasattr(char const *__restrict name) const { return throw_if_negative(DeeObject_HasAttrString(*this,name)) != 0; }
    bool hasattr(char const *__restrict name, dhash_t hash) const { return throw_if_negative(DeeObject_HasAttrStringHash(*this,name,hash)) != 0; }
    bool boundattr(obj_string name) const { int result = DeeObject_BoundAttr(*this,name); if (result == -1) throw_last_deemon_exception(); return result > 0; }
    bool boundattr(char const *__restrict name) const { int result = DeeObject_BoundAttrString(*this,name); if (result == -1) throw_last_deemon_exception(); return result > 0; }
    bool boundattr(char const *__restrict name, dhash_t hash) const { int result = DeeObject_BoundAttrStringHash(*this,name,hash); if (result == -1) throw_last_deemon_exception(); return result > 0; }
    void delattr(obj_string name) const { throw_if_negative(DeeObject_DelAttr(*this,name)); }
    void delattr(char const *__restrict name) const { throw_if_negative(DeeObject_DelAttrString(*this,name)); }
    void delattr(char const *__restrict name, dhash_t hash) const { throw_if_negative(DeeObject_DelAttrStringHash(*this,name,hash)); }
    void setattr(obj_string name, DeeObject *__restrict value) const { throw_if_negative(DeeObject_SetAttr(*this,name,value)); }
    void setattr(char const *__restrict name, DeeObject *__restrict value) const { throw_if_negative(DeeObject_SetAttrString(*this,name,value)); }
    void setattr(char const *__restrict name, dhash_t hash, DeeObject *__restrict value) const { throw_if_negative(DeeObject_SetAttrStringHash(*this,name,hash,value)); }
    object callattr(obj_string name, obj_tuple args) const { return inherit(DeeObject_CallAttrTuple(*this,name,args)); }
    object callattr(obj_string name, size_t argc, DeeObject **__restrict argv) const { return inherit(DeeObject_CallAttr(*this,name,argc,argv)); }
    object callattr(obj_string name, size_t argc, DeeObject *const *__restrict argv) const { return inherit(DeeObject_CallAttr(*this,name,argc,(DeeObject **)argv)); }
    object callattr(obj_string name, size_t argc, object **__restrict argv) const { return inherit(DeeObject_CallAttr(*this,name,argc,(DeeObject **)argv)); }
    object callattr(obj_string name, size_t argc, object *const *__restrict argv) const { return inherit(DeeObject_CallAttr(*this,name,argc,(DeeObject **)argv)); }
    object callattr(obj_string name, std::initializer_list<DeeObject *> const &args) const { return inherit(DeeObject_CallAttr(*this,name,args.size(),(DeeObject **)args.begin())); }
    object callattr(char const *__restrict name, obj_tuple args) const { return inherit(DeeObject_CallAttrString(*this,name,DeeTuple_SIZE((DeeObject *)args),DeeTuple_ELEM((DeeObject *)args))); }
    object callattr(char const *__restrict name, size_t argc, DeeObject **__restrict argv) const { return inherit(DeeObject_CallAttrString(*this,name,argc,argv)); }
    object callattr(char const *__restrict name, size_t argc, DeeObject *const *__restrict argv) const { return inherit(DeeObject_CallAttrString(*this,name,argc,(DeeObject **)argv)); }
    object callattr(char const *__restrict name, size_t argc, object **__restrict argv) const { return inherit(DeeObject_CallAttrString(*this,name,argc,(DeeObject **)argv)); }
    object callattr(char const *__restrict name, size_t argc, object *const *__restrict argv) const { return inherit(DeeObject_CallAttrString(*this,name,argc,(DeeObject **)argv)); }
    object callattr(char const *__restrict name, std::initializer_list<DeeObject *> const &args) const { return inherit(DeeObject_CallAttrString(*this,name,args.size(),(DeeObject **)args.begin())); }
    object callattr(char const *__restrict name, dhash_t hash, obj_tuple args) const { return inherit(DeeObject_CallAttrStringHash(*this,name,hash,DeeTuple_SIZE((DeeObject *)args),DeeTuple_ELEM((DeeObject *)args))); }
    object callattr(char const *__restrict name, dhash_t hash, size_t argc, DeeObject **__restrict argv) const { return inherit(DeeObject_CallAttrStringHash(*this,name,hash,argc,argv)); }
    object callattr(char const *__restrict name, dhash_t hash, size_t argc, DeeObject *const *__restrict argv) const { return inherit(DeeObject_CallAttrStringHash(*this,name,hash,argc,(DeeObject **)argv)); }
    object callattr(char const *__restrict name, dhash_t hash, size_t argc, object **__restrict argv) const { return inherit(DeeObject_CallAttrStringHash(*this,name,hash,argc,(DeeObject **)argv)); }
    object callattr(char const *__restrict name, dhash_t hash, size_t argc, object *const *__restrict argv) const { return inherit(DeeObject_CallAttrStringHash(*this,name,hash,argc,(DeeObject **)argv)); }
    object callattr(char const *__restrict name, dhash_t hash, std::initializer_list<DeeObject *> const &args) const { return inherit(DeeObject_CallAttrStringHash(*this,name,hash,args.size(),(DeeObject **)args.begin())); }
    item_proxy_obj item(DeeObject *__restrict index) const { return item_proxy_obj(*this,index); }
    item_proxy_idx item(size_t index) const { return item_proxy_idx(*this,index); }
    item_proxy_sth item(char const *__restrict name) const { return item_proxy_sth(*this,name); }
    item_proxy_sth item(char const *__restrict name, dhash_t hash) const { return item_proxy_sth(*this,name,hash); }
    object getitem(DeeObject *__restrict index) const { return inherit(DeeObject_GetItem(*this,index)); }
    object getitem(size_t index) const { return inherit(DeeObject_GetItemIndex(*this,index)); }
    object getitem(char const *__restrict name) const { return inherit(DeeObject_GetItemString(*this,name,hash_str(name))); }
    object getitem(char const *__restrict name, dhash_t hash) const { return inherit(DeeObject_GetItemString(*this,name,hash)); }
    object getitem_def(DeeObject *__restrict index, DeeObject *__restrict def) const { return inherit(DeeObject_GetItemDef(*this,index,def)); }
    object getitem_def(size_t index, DeeObject *__restrict def) const { DREF DeeObject *result,*index_ob = throw_if_null(DeeInt_NewSize(index)); result = DeeObject_GetItemDef(*this,index_ob,def); Dee_Decref(index_ob); return inherit(result); }
    object getitem_def(char const *__restrict name, DeeObject *__restrict def) const { return inherit(DeeObject_GetItemStringDef(*this,name,hash_str(name),def)); }
    object getitem_def(char const *__restrict name, dhash_t hash, DeeObject *__restrict def) const { return inherit(DeeObject_GetItemStringDef(*this,name,hash,def)); }
    void delitem(DeeObject *__restrict index) const { throw_if_negative(DeeObject_DelItem(*this,index)); }
    void delitem(size_t index) const { throw_if_negative(DeeObject_DelItemIndex(*this,index)); }
    void delitem(char const *__restrict name) const { throw_if_negative(DeeObject_DelItemString(*this,name,hash_str(name))); }
    void delitem(char const *__restrict name, dhash_t hash) const { throw_if_negative(DeeObject_DelItemString(*this,name,hash)); }
    void setitem(DeeObject *__restrict index, DeeObject *__restrict value) const { throw_if_negative(DeeObject_SetItem(*this,index,value)); }
    void setitem(size_t index, DeeObject *__restrict value) const { throw_if_negative(DeeObject_SetItemIndex(*this,index,value)); }
    void setitem(char const *__restrict name, DeeObject *__restrict value) const { throw_if_negative(DeeObject_SetItemString(*this,name,hash_str(name),value)); }
    void setitem(char const *__restrict name, dhash_t hash, DeeObject *__restrict value) const { throw_if_negative(DeeObject_SetItemString(*this,name,hash,value)); }
    range_proxy_oo range(DeeObject *__restrict begin, DeeObject *__restrict end) const { return range_proxy_oo(*this,begin,end); }
    range_proxy_io range(size_t begin, DeeObject *__restrict end) const { return range_proxy_io(*this,begin,end); }
    range_proxy_oi range(DeeObject *__restrict begin, size_t end) const { return range_proxy_oi(*this,begin,end); }
    range_proxy_ii range(size_t begin, size_t end) const { return range_proxy_ii(*this,begin,end); }
    object getrange(DeeObject *__restrict begin, DeeObject *__restrict end) const { return inherit(DeeObject_GetRange(*this,begin,end)); }
    object getrange(size_t begin, DeeObject *__restrict end) const { return inherit(DeeObject_GetRangeBeginIndex(*this,begin,end)); }
    object getrange(DeeObject *__restrict begin, size_t end) const { return inherit(DeeObject_GetRangeEndIndex(*this,begin,end)); }
    object getrange(size_t begin, size_t end) const { return inherit(DeeObject_GetRangeIndex(*this,begin,end)); }
    void delrange(DeeObject *__restrict begin, DeeObject *__restrict end) const { throw_if_negative(DeeObject_DelRange(*this,begin,end)); }
    void delrange(size_t begin, DeeObject *__restrict end) const { DREF DeeObject *begin_ob = throw_if_null(DeeInt_NewSize(begin)); int error = DeeObject_DelRange(*this,begin_ob,end); Dee_Decref(begin_ob); throw_if_nonzero(error); }
    void delrange(DeeObject *__restrict begin, size_t end) const { DREF DeeObject *end_ob = throw_if_null(DeeInt_NewSize(end)); int error = DeeObject_DelRange(*this,begin,end_ob); Dee_Decref(end_ob); throw_if_nonzero(error); }
    void delrange(size_t begin, size_t end) const { DREF DeeObject *begin_ob = throw_if_null(DeeInt_NewSize(begin)); DREF DeeObject *end_ob = DeeInt_NewSize(end); int error; if unlikely(!end_ob) { Dee_Decref(begin_ob); throw_last_deemon_exception(); } error = DeeObject_DelRange(*this,begin_ob,end_ob); Dee_Decref(end_ob); Dee_Decref(begin_ob); throw_if_nonzero(error); }
    void setrange(DeeObject *__restrict begin, DeeObject *__restrict end, DeeObject *__restrict values) const { throw_if_nonzero(DeeObject_SetRange(*this,begin,end,values)); }
    void setrange(size_t begin, DeeObject *__restrict end, DeeObject *__restrict values) const { throw_if_nonzero(DeeObject_SetRangeBeginIndex(*this,begin,end,values)); }
    void setrange(DeeObject *__restrict begin, size_t end, DeeObject *__restrict values) const { throw_if_nonzero(DeeObject_SetRangeEndIndex(*this,begin,end,values)); }
    void setrange(size_t begin, size_t end, DeeObject *__restrict values) const { throw_if_nonzero(DeeObject_SetRangeIndex(*this,begin,end,values)); }
    object call() const { return inherit(DeeObject_Call(*this,0,NULL)); }
    object call(obj_tuple args) const { return inherit(DeeObject_CallTuple(*this,args)); }
    object call(size_t argc, DeeObject **__restrict argv) const { return inherit(DeeObject_Call(*this,argc,argv)); }
    object call(size_t argc, DeeObject *const *__restrict argv) const { return inherit(DeeObject_Call(*this,argc,(DeeObject **)argv)); }
    object call(size_t argc, object **__restrict argv) const { return inherit(DeeObject_Call(*this,argc,(DeeObject **)argv)); }
    object call(size_t argc, object *const *__restrict argv) const { return inherit(DeeObject_Call(*this,argc,(DeeObject **)argv)); }
    object call(std::initializer_list<DeeObject *> const &args) const { return inherit(DeeObject_Call(*this,args.size(),(DeeObject **)args.begin())); }
    object call(obj_tuple args, DeeObject *kw) const { return inherit(DeeObject_CallTupleKw(*this,args,kw)); }
    object call(size_t argc, DeeObject **__restrict argv, DeeObject *kw) const { return inherit(DeeObject_CallKw(*this,argc,argv,kw)); }
    object call(size_t argc, DeeObject *const *__restrict argv, DeeObject *kw) const { return inherit(DeeObject_CallKw(*this,argc,(DeeObject **)argv,kw)); }
    object call(size_t argc, object **__restrict argv, DeeObject *kw) const { return inherit(DeeObject_CallKw(*this,argc,(DeeObject **)argv,kw)); }
    object call(size_t argc, object *const *__restrict argv, DeeObject *kw) const { return inherit(DeeObject_CallKw(*this,argc,(DeeObject **)argv,kw)); }
    object call(std::initializer_list<DeeObject *> const &args, DeeObject *kw) const { return inherit(DeeObject_CallKw(*this,args.size(),(DeeObject **)args.begin(),kw)); }
    object thiscall(DeeObject *__restrict this_arg) const { return inherit(DeeObject_ThisCall(*this,this_arg,0,NULL)); }
    object thiscall(DeeObject *__restrict this_arg, obj_tuple args) const { return inherit(DeeObject_ThisCallTuple(*this,this_arg,args)); }
    object thiscall(DeeObject *__restrict this_arg, size_t argc, DeeObject **__restrict argv) const { return inherit(DeeObject_ThisCall(*this,this_arg,argc,argv)); }
    object thiscall(DeeObject *__restrict this_arg, size_t argc, DeeObject *const *__restrict argv) const { return inherit(DeeObject_ThisCall(*this,this_arg,argc,(DeeObject **)argv)); }
    object thiscall(DeeObject *__restrict this_arg, size_t argc, object **__restrict argv) const { return inherit(DeeObject_ThisCall(*this,this_arg,argc,(DeeObject **)argv)); }
    object thiscall(DeeObject *__restrict this_arg, size_t argc, object *const *__restrict argv) const { return inherit(DeeObject_ThisCall(*this,this_arg,argc,(DeeObject **)argv)); }
    object thiscall(DeeObject *__restrict this_arg, std::initializer_list<DeeObject *> const &args) const { return inherit(DeeObject_ThisCall(*this,this_arg,args.size(),(DeeObject **)args.begin())); }
    object thiscall(DeeObject *__restrict this_arg, obj_tuple args, DeeObject *kw) const { return inherit(DeeObject_ThisCallTupleKw(*this,this_arg,args,kw)); }
    object thiscall(DeeObject *__restrict this_arg, size_t argc, DeeObject **__restrict argv, DeeObject *kw) const { return inherit(DeeObject_ThisCallKw(*this,this_arg,argc,argv,kw)); }
    object thiscall(DeeObject *__restrict this_arg, size_t argc, DeeObject *const *__restrict argv, DeeObject *kw) const { return inherit(DeeObject_ThisCallKw(*this,this_arg,argc,(DeeObject **)argv,kw)); }
    object thiscall(DeeObject *__restrict this_arg, size_t argc, object **__restrict argv, DeeObject *kw) const { return inherit(DeeObject_ThisCallKw(*this,this_arg,argc,(DeeObject **)argv,kw)); }
    object thiscall(DeeObject *__restrict this_arg, size_t argc, object *const *__restrict argv, DeeObject *kw) const { return inherit(DeeObject_ThisCallKw(*this,this_arg,argc,(DeeObject **)argv,kw)); }
    object thiscall(DeeObject *__restrict this_arg, std::initializer_list<DeeObject *> const &args, DeeObject *kw) const { return inherit(DeeObject_ThisCallKw(*this,this_arg,args.size(),(DeeObject **)args.begin(),kw)); }
    string doc() const;
    string str() const;
    string repr() const;
    deemon::super super() const;
    deemon::super super(DeeTypeObject *__restrict super_type) const;
    template<class T = object> deemon::super super() const;
    deemon::type<object> type() const;
    deemon::type<object> class_() const;
    int_ int_() const;
    bool bool_() const { return throw_if_negative(DeeObject_Bool(*this)) != 0; }
    WUNUSED dhash_t hash() const DEE_CXX_NOTHROW { return DeeObject_Hash(*this); }
    bool equal(DeeObject *__restrict other) const { return throw_if_negative(DeeObject_CompareEq(*this,other)) != 0; }
    bool equal(DeeObject *__restrict other, DeeObject *key) const { return throw_if_negative(DeeObject_CompareKeyEq(*this,other,key)) != 0; }
    bool nonequal(DeeObject *__restrict other) const { return throw_if_negative(DeeObject_CompareNe(*this,other)) != 0; }
    bool nonequal(DeeObject *__restrict other, DeeObject *key) const { return throw_if_negative(DeeObject_CompareKeyEq(*this,other,key)) == 0; }
    size_t size() const { return throw_if_minusone(DeeObject_Size(*this)); }
    bool contains(DeeObject *__restrict elem) const { return throw_if_negative(DeeObject_Contains(*this,elem)) != 0; }
    object sizeobj() const { return inherit(DeeObject_SizeObject(*this)); }
    object containsobj(DeeObject *__restrict elem) const { return inherit(DeeObject_ContainsObject(*this,elem)); }
    object const &assign(DeeObject *__restrict some_object) const { throw_if_nonzero(DeeObject_Assign(*this,some_object)); return *this; }
    object const &moveassign(DeeObject *__restrict some_object) const { throw_if_nonzero(DeeObject_MoveAssign(*this,some_object)); return *this; }
    void enter() const { throw_if_nonzero(DeeObject_Enter(*this)); }
    void leave() const { throw_if_nonzero(DeeObject_Leave(*this)); }
    ATTR_NORETURN void throw_() const { DeeError_Throw(*this); throw_last_deemon_exception(); }
    void unpack(size_t objc, DREF DeeObject **__restrict objv) const { throw_if_nonzero(DeeObject_Unpack(*this,objc,objv)); }
//  void unpack(size_t objc, object **__restrict objv) const { throw_if_nonzero(DeeObject_Unpack(*this,objc,(DeeObject **)objv)); }
    typedef detail::cxx_iterator<object> iterator;
    WUNUSED iterator begin() const { return inherit(DeeObject_IterSelf(*this)); }
    WUNUSED iterator end() const { return iterator(); }
    deemon::iterator<object> iter() const;
    template<class T = object> deemon::iterator<T> iter() const;

    object next() const { DREF DeeObject *result = throw_if_null(DeeObject_IterNext(*this)); if (result == ITER_DONE) result = NULL; return inherit(maybenull(result)); }
    size_t print(dformatprinter printer, void *arg) const { return throw_if_negative(DeeObject_Print(*this,printer,arg)); }
    size_t print(dformatprinter printer, void *arg, DeeObject *__restrict format_str) const { return throw_if_negative(DeeObject_PrintFormat(*this,printer,arg,format_str)); }
    size_t print(dformatprinter printer, void *arg, /*utf-8*/char const *__restrict format_str) const { return throw_if_negative(DeeObject_PrintFormatString(*this,printer,arg,format_str,strlen(format_str))); }
    size_t print(dformatprinter printer, void *arg, /*utf-8*/char const *__restrict format_str, size_t format_len) const { return throw_if_negative(DeeObject_PrintFormatString(*this,printer,arg,format_str,format_len)); }
    size_t printrepr(dformatprinter printer, void *arg) const { return throw_if_negative(DeeObject_PrintRepr(*this,printer,arg)); }
    object inv() const { return inherit(DeeObject_Inv(*this)); }
    object pos() const { return inherit(DeeObject_Pos(*this)); }
    object neg() const { return inherit(DeeObject_Neg(*this)); }
    object add(int8_t right) const { return inherit(DeeObject_AddS8(*this,right)); }
    object add(uint32_t right) const { return inherit(DeeObject_AddInt(*this,right)); }
    object add(DeeObject *__restrict right) const { return inherit(DeeObject_Add(*this,right)); }
    object sub(int8_t right) const { return inherit(DeeObject_SubS8(*this,right)); }
    object sub(uint32_t right) const { return inherit(DeeObject_SubInt(*this,right)); }
    object sub(DeeObject *__restrict right) const { return inherit(DeeObject_Sub(*this,right)); }
    object mul(int8_t right) const { return inherit(DeeObject_MulInt(*this,right)); }
    object mul(DeeObject *__restrict right) const { return inherit(DeeObject_Mul(*this,right)); }
    object div(int8_t right) const { return inherit(DeeObject_DivInt(*this,right)); }
    object div(DeeObject *__restrict right) const { return inherit(DeeObject_Div(*this,right)); }
    object mod(int8_t right) const { return inherit(DeeObject_ModInt(*this,right)); }
    object mod(DeeObject *__restrict right) const { return inherit(DeeObject_Mod(*this,right)); }
    object shl(uint8_t right) const { return inherit(DeeObject_ShlInt(*this,right)); }
    object shl(DeeObject *__restrict right) const { return inherit(DeeObject_Shl(*this,right)); }
    object shr(uint8_t right) const { return inherit(DeeObject_ShrInt(*this,right)); }
    object shr(DeeObject *__restrict right) const { return inherit(DeeObject_Shr(*this,right)); }
    object and_(uint32_t right) const { return inherit(DeeObject_AndInt(*this,right)); }
    object and_(DeeObject *__restrict right) const { return inherit(DeeObject_And(*this,right)); }
    object or_(uint32_t right) const { return inherit(DeeObject_OrInt(*this,right)); }
    object or_(DeeObject *__restrict right) const { return inherit(DeeObject_Or(*this,right)); }
    object xor_(uint32_t right) const { return inherit(DeeObject_XorInt(*this,right)); }
    object xor_(DeeObject *__restrict right) const { return inherit(DeeObject_Xor(*this,right)); }
    object pow(DeeObject *__restrict right) const { return inherit(DeeObject_Pow(*this,right)); }
    object &inplace_add(int8_t right) { throw_if_nonzero(DeeObject_InplaceAddS8(&this->m_ptr,right)); return *this; }
    object &inplace_add(uint32_t right) { throw_if_nonzero(DeeObject_InplaceAddInt(&this->m_ptr,right)); return *this; }
    object &inplace_add(DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceAdd(&this->m_ptr,right)); return *this; }
    object &inplace_sub(int8_t right) { throw_if_nonzero(DeeObject_InplaceSubS8(&this->m_ptr,right)); return *this; }
    object &inplace_sub(uint32_t right) { throw_if_nonzero(DeeObject_InplaceSubInt(&this->m_ptr,right)); return *this; }
    object &inplace_sub(DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceSub(&this->m_ptr,right)); return *this; }
    object &inplace_mul(int8_t right) { throw_if_nonzero(DeeObject_InplaceMulInt(&this->m_ptr,right)); return *this; }
    object &inplace_mul(DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceMul(&this->m_ptr,right)); return *this; }
    object &inplace_div(int8_t right) { throw_if_nonzero(DeeObject_InplaceDivInt(&this->m_ptr,right)); return *this; }
    object &inplace_div(DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceDiv(&this->m_ptr,right)); return *this; }
    object &inplace_mod(int8_t right) { throw_if_nonzero(DeeObject_InplaceModInt(&this->m_ptr,right)); return *this; }
    object &inplace_mod(DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceMod(&this->m_ptr,right)); return *this; }
    object &inplace_shl(uint8_t right) { throw_if_nonzero(DeeObject_InplaceShlInt(&this->m_ptr,right)); return *this; }
    object &inplace_shl(DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceShl(&this->m_ptr,right)); return *this; }
    object &inplace_shr(uint8_t right) { throw_if_nonzero(DeeObject_InplaceShrInt(&this->m_ptr,right)); return *this; }
    object &inplace_shr(DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceShr(&this->m_ptr,right)); return *this; }
    object &inplace_and(uint32_t right) { throw_if_nonzero(DeeObject_InplaceAndInt(&this->m_ptr,right)); return *this; }
    object &inplace_and(DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceAnd(&this->m_ptr,right)); return *this; }
    object &inplace_or(uint32_t right) { throw_if_nonzero(DeeObject_InplaceOrInt(&this->m_ptr,right)); return *this; }
    object &inplace_or(DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceOr(&this->m_ptr,right)); return *this; }
    object &inplace_xor(uint32_t right) { throw_if_nonzero(DeeObject_InplaceXorInt(&this->m_ptr,right)); return *this; }
    object &inplace_xor(DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceXor(&this->m_ptr,right)); return *this; }
    object &inplace_pow(DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplacePow(&this->m_ptr,right)); return *this; }

    /* Generic operator invocation. */
    object invoke_operator(uint16_t name, obj_tuple args) const { return inherit(DeeObject_InvokeOperator(*this,name,DeeTuple_SIZE((DeeObject *)args),DeeTuple_ELEM((DeeObject *)args))); }
    object invoke_operator(uint16_t name, size_t argc, DeeObject **__restrict argv) const { return inherit(DeeObject_InvokeOperator(*this,name,argc,argv)); }
    object invoke_operator(uint16_t name, size_t argc, object *__restrict argv) const { return inherit(DeeObject_InvokeOperator(*this,name,argc,(DeeObject **)argv)); }
    object invoke_operator(uint16_t name, std::initializer_list<DeeObject *> const &args) const { return inherit(DeeObject_InvokeOperator(*this,name,args.size(),(DeeObject **)args.begin())); }
    object invoke_inplace_operator(uint16_t name, obj_tuple args) { return inherit(DeeObject_PInvokeOperator(&this->m_ptr,name,DeeTuple_SIZE((DeeObject *)args),DeeTuple_ELEM((DeeObject *)args))); }
    object invoke_inplace_operator(uint16_t name, size_t argc, DeeObject **__restrict argv) { return inherit(DeeObject_PInvokeOperator(&this->m_ptr,name,argc,argv)); }
    object invoke_inplace_operator(uint16_t name, size_t argc, object *__restrict argv) { return inherit(DeeObject_PInvokeOperator(&this->m_ptr,name,argc,(DeeObject **)argv)); }
    object invoke_inplace_operator(uint16_t name, std::initializer_list<DeeObject *> const &args) { return inherit(DeeObject_PInvokeOperator(&this->m_ptr,name,args.size(),(DeeObject **)args.begin())); }

    /* Operator integration */
    WUNUSED item_proxy_obj operator [](DeeObject *__restrict index) const { return item_proxy_obj(*this,index); }
    WUNUSED item_proxy_idx operator [](size_t index) const { return item_proxy_idx(*this,index); }
    WUNUSED item_proxy_sth operator [](char const *__restrict name) const { return item_proxy_sth(*this,name); }
    WUNUSED object operator ()(obj_tuple args) const { return inherit(DeeObject_CallTuple(*this,args)); }
    WUNUSED object operator ()(std::initializer_list<DeeObject *> const &args) const { return inherit(DeeObject_Call(*this,args.size(),(DeeObject **)args.begin())); }
    WUNUSED operator bool() const { return throw_if_negative(DeeObject_Bool(*this)) != 0; }
    WUNUSED bool operator ! () const { return throw_if_negative(DeeObject_Bool(*this)) == 0; }
    WUNUSED object operator == (DeeObject *__restrict other) const { return DeeObject_CompareEqObject(*this,other); }
    WUNUSED object operator != (DeeObject *__restrict other) const { return DeeObject_CompareNeObject(*this,other); }
    WUNUSED object operator < (DeeObject *__restrict other) const { return DeeObject_CompareLoObject(*this,other); }
    WUNUSED object operator <= (DeeObject *__restrict other) const { return DeeObject_CompareLeObject(*this,other); }
    WUNUSED object operator > (DeeObject *__restrict other) const { return DeeObject_CompareGrObject(*this,other); }
    WUNUSED object operator >= (DeeObject *__restrict other) const { return DeeObject_CompareGeObject(*this,other); }
    WUNUSED object operator ~ () const { return inherit(DeeObject_Inv(*this)); }
    WUNUSED object operator + () const { return inherit(DeeObject_Pos(*this)); }
    WUNUSED object operator - () const { return inherit(DeeObject_Neg(*this)); }
    WUNUSED object operator + (int8_t right) const { return inherit(DeeObject_AddS8(*this,right)); }
    WUNUSED object operator + (uint32_t right) const { return inherit(DeeObject_AddInt(*this,right)); }
    WUNUSED object operator + (DeeObject *__restrict right) const { return inherit(DeeObject_Add(*this,right)); }
    WUNUSED object operator - (int8_t right) const { return inherit(DeeObject_SubS8(*this,right)); }
    WUNUSED object operator - (uint32_t right) const { return inherit(DeeObject_SubInt(*this,right)); }
    WUNUSED object operator - (DeeObject *__restrict right) const { return inherit(DeeObject_Sub(*this,right)); }
    WUNUSED object operator * (int8_t right) const { return inherit(DeeObject_MulInt(*this,right)); }
    WUNUSED object operator * (DeeObject *__restrict right) const { return inherit(DeeObject_Mul(*this,right)); }
    WUNUSED object operator / (int8_t right) const { return inherit(DeeObject_DivInt(*this,right)); }
    WUNUSED object operator / (DeeObject *__restrict right) const { return inherit(DeeObject_Div(*this,right)); }
    WUNUSED object operator % (int8_t right) const { return inherit(DeeObject_ModInt(*this,right)); }
    WUNUSED object operator % (DeeObject *__restrict right) const { return inherit(DeeObject_Mod(*this,right)); }
    WUNUSED object operator << (uint8_t right) const { return inherit(DeeObject_ShlInt(*this,right)); }
    WUNUSED object operator << (DeeObject *__restrict right) const { return inherit(DeeObject_Shl(*this,right)); }
    WUNUSED object operator >> (uint8_t right) const { return inherit(DeeObject_ShrInt(*this,right)); }
    WUNUSED object operator >> (DeeObject *__restrict right) const { return inherit(DeeObject_Shr(*this,right)); }
    WUNUSED object operator & (uint32_t right) const { return inherit(DeeObject_AndInt(*this,right)); }
    WUNUSED object operator & (DeeObject *__restrict right) const { return inherit(DeeObject_And(*this,right)); }
    WUNUSED object operator | (uint32_t right) const { return inherit(DeeObject_OrInt(*this,right)); }
    WUNUSED object operator | (DeeObject *__restrict right) const { return inherit(DeeObject_Or(*this,right)); }
    WUNUSED object operator ^ (uint32_t right) const { return inherit(DeeObject_XorInt(*this,right)); }
    WUNUSED object operator ^ (DeeObject *__restrict right) const { return inherit(DeeObject_Xor(*this,right)); }
    object &operator ++ () { throw_if_nonzero(DeeObject_Inc(&this->m_ptr)); return *this; }
    object &operator -- () { throw_if_nonzero(DeeObject_Dec(&this->m_ptr)); return *this; }
    WUNUSED object operator ++ (int) { DREF DeeObject *result = throw_if_null(DeeObject_Copy(*this)); int error = DeeObject_Inc(&this->m_ptr); if unlikely(error) { Dee_Decref(result); throw_last_deemon_exception(); } return inherit(nonnull(result)); }
    WUNUSED object operator -- (int) { DREF DeeObject *result = throw_if_null(DeeObject_Copy(*this)); int error = DeeObject_Dec(&this->m_ptr); if unlikely(error) { Dee_Decref(result); throw_last_deemon_exception(); } return inherit(nonnull(result)); }
    object &operator += (int8_t right) { throw_if_nonzero(DeeObject_InplaceAddS8(&this->m_ptr,right)); return *this; }
    object &operator += (uint32_t right) { throw_if_nonzero(DeeObject_InplaceAddInt(&this->m_ptr,right)); return *this; }
    object &operator += (DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceAdd(&this->m_ptr,right)); return *this; }
    object &operator -= (int8_t right) { throw_if_nonzero(DeeObject_InplaceSubS8(&this->m_ptr,right)); return *this; }
    object &operator -= (uint32_t right) { throw_if_nonzero(DeeObject_InplaceSubInt(&this->m_ptr,right)); return *this; }
    object &operator -= (DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceSub(&this->m_ptr,right)); return *this; }
    object &operator *= (int8_t right) { throw_if_nonzero(DeeObject_InplaceMulInt(&this->m_ptr,right)); return *this; }
    object &operator *= (DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceMul(&this->m_ptr,right)); return *this; }
    object &operator /= (int8_t right) { throw_if_nonzero(DeeObject_InplaceDivInt(&this->m_ptr,right)); return *this; }
    object &operator /= (DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceDiv(&this->m_ptr,right)); return *this; }
    object &operator %= (int8_t right) { throw_if_nonzero(DeeObject_InplaceModInt(&this->m_ptr,right)); return *this; }
    object &operator %= (DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceMod(&this->m_ptr,right)); return *this; }
    object &operator <<= (uint8_t right) { throw_if_nonzero(DeeObject_InplaceShlInt(&this->m_ptr,right)); return *this; }
    object &operator <<= (DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceShl(&this->m_ptr,right)); return *this; }
    object &operator >>= (uint8_t right) { throw_if_nonzero(DeeObject_InplaceShrInt(&this->m_ptr,right)); return *this; }
    object &operator >>= (DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceShr(&this->m_ptr,right)); return *this; }
    object &operator &= (uint32_t right) { throw_if_nonzero(DeeObject_InplaceAndInt(&this->m_ptr,right)); return *this; }
    object &operator &= (DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceAnd(&this->m_ptr,right)); return *this; }
    object &operator |= (uint32_t right) { throw_if_nonzero(DeeObject_InplaceOrInt(&this->m_ptr,right)); return *this; }
    object &operator |= (DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceOr(&this->m_ptr,right)); return *this; }
    object &operator ^= (uint32_t right) { throw_if_nonzero(DeeObject_InplaceXorInt(&this->m_ptr,right)); return *this; }
    object &operator ^= (DeeObject *__restrict right) { throw_if_nonzero(DeeObject_InplaceXor(&this->m_ptr,right)); return *this; }

    /* High-level interface for up-casting object types. */
    template<class T> WUNUSED T &as(typename std::enable_if<std::is_base_of<object,T>::value,int>::type* =0) { return *(T *)this; }
    template<class T> WUNUSED T const &as(typename std::enable_if<std::is_base_of<object,T>::value,int>::type* =0) const { return *(T *)this; }

    /* Integer conversion */
    object const &getval(char &value) const { throw_if_nonzero(DeeObject_AsChar(*this,&value)); return *this; }
    object const &getval(signed char &value) const { throw_if_nonzero(DeeObject_AsSChar(*this,&value)); return *this; }
    object const &getval(unsigned char &value) const { throw_if_nonzero(DeeObject_AsUChar(*this,&value)); return *this; }
    object const &getval(short &value) const { throw_if_nonzero(DeeObject_AsShort(*this,&value)); return *this; }
    object const &getval(unsigned short &value) const { throw_if_nonzero(DeeObject_AsUShort(*this,&value)); return *this; }
    object const &getval(int &value) const { throw_if_nonzero(DeeObject_AsInt(*this,&value)); return *this; }
    object const &getval(unsigned int &value) const { throw_if_nonzero(DeeObject_AsUInt(*this,&value)); return *this; }
    object const &getval(long &value) const { throw_if_nonzero(DeeObject_AsLong(*this,&value)); return *this; }
    object const &getval(unsigned long &value) const { throw_if_nonzero(DeeObject_AsULong(*this,&value)); return *this; }
#ifdef __COMPILER_HAVE_LONGLONG
    object const &getval(long long &value) const { throw_if_nonzero(DeeObject_AsLLong(*this,&value)); return *this; }
    object const &getval(unsigned long long &value) const { throw_if_nonzero(DeeObject_AsULLong(*this,&value)); return *this; }
#endif
    object const &getval(dint128_t &value) const { throw_if_nonzero(DeeObject_AsInt128(*this,&value)); return *this; }
    object const &getval(duint128_t &value) const { throw_if_nonzero(DeeObject_AsUInt128(*this,&value)); return *this; }
    object const &getval(float &value) const { double temp; getval(temp); value = (float)temp; return *this; }
    object const &getval(double &value) const { throw_if_nonzero(DeeObject_AsDouble(*this,&value)); return *this; }
#ifdef __COMPILER_HAVE_LONGDOUBLE
    object const &getval(long double &value) const { double temp; getval(temp); value = (long double)temp; return *this; }
#endif

    /* Helper functions to explicitly convert an object to an integral value. */
    WUNUSED int8_t ass8() const { int8_t result; getval(result); return result; }
    WUNUSED int16_t ass16() const { int16_t result; getval(result); return result; }
    WUNUSED int32_t ass32() const { int32_t result; getval(result); return result; }
    WUNUSED int64_t ass64() const { int64_t result; getval(result); return result; }
    WUNUSED dint128_t ass128() const { dint128_t result; getval(result); return result; }
    WUNUSED uint8_t asu8() const { uint8_t result; getval(result); return result; }
    WUNUSED uint16_t asu16() const { uint16_t result; getval(result); return result; }
    WUNUSED uint32_t asu32() const { uint32_t result; getval(result); return result; }
    WUNUSED uint64_t asu64() const { uint64_t result; getval(result); return result; }
    WUNUSED duint128_t asu128() const { duint128_t result; getval(result); return result; }
    WUNUSED size_t assize() const { size_t result; getval(result); return result; }
    WUNUSED dssize_t asssize() const { dssize_t result; getval(result); return result; }
    WUNUSED float asfloat() const { float result; getval(result); return result; }
    WUNUSED double asdouble() const { double result; getval(result); return result; }
#ifdef __COMPILER_HAVE_LONGDOUBLE
    WUNUSED long double asldouble() const { long double result; getval(result); return result; }
#endif

    /* Integer conversion operators */
    explicit WUNUSED operator char() const { char result; getval(result); return result; }
    explicit WUNUSED operator signed char() const { signed char result; getval(result); return result; }
    explicit WUNUSED operator unsigned char() const { unsigned char result; getval(result); return result; }
    explicit WUNUSED operator short() const { short result; getval(result); return result; }
    explicit WUNUSED operator unsigned short() const { unsigned short result; getval(result); return result; }
    explicit WUNUSED operator int() const { int result; getval(result); return result; }
    explicit WUNUSED operator unsigned int() const { unsigned int result; getval(result); return result; }
    explicit WUNUSED operator long() const { long result; getval(result); return result; }
    explicit WUNUSED operator unsigned long() const { unsigned long result; getval(result); return result; }
#ifdef __COMPILER_HAVE_LONGLONG
    explicit WUNUSED operator long long() const { long long result; getval(result); return result; }
    explicit WUNUSED operator unsigned long long() const { unsigned long long result; getval(result); return result; }
#endif
    explicit WUNUSED operator dint128_t() const { dint128_t result; getval(result); return result; }
    explicit WUNUSED operator duint128_t() const { duint128_t result; getval(result); return result; }
    explicit WUNUSED operator float() const { float result; getval(result); return result; }
    explicit WUNUSED operator double() const { double result; getval(result); return result; }
#ifdef __COMPILER_HAVE_LONGDOUBLE
    explicit WUNUSED operator long double() const { long double result; getval(result); return result; }
#endif
};



class bool_: public object {
public:
    static DeeTypeObject *classtype() DEE_CXX_NOTHROW { return &DeeBool_Type; }
    static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW { return DeeBool_Check(ob); }
    static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW { return DeeBool_CheckExact(ob); }
public:
    DEFINE_OBJECT_CONSTRUCTORS(bool_,object)
    bool_() DEE_CXX_NOTHROW: object(nonnull(Dee_False)) { }
    bool_(bool value) DEE_CXX_NOTHROW: object(nonnull(DeeBool_For(value))) { }
    static WUNUSED bool_ true_() DEE_CXX_NOTHROW { return nonnull(Dee_True); }
    static WUNUSED bool_ false_() DEE_CXX_NOTHROW { return nonnull(Dee_False); }
#ifndef __OPTIMIZE_SIZE__
    //bool bool_() const DEE_CXX_NOTHROW { return DeeBool_IsTrue(this->ptr()); }
    WUNUSED operator bool () const { return likely(DeeBool_Check(this->ptr())) ? DeeBool_IsTrue(this->ptr()) : object::operator bool(); }
    WUNUSED bool operator ! () const { return likely(DeeBool_Check(this->ptr())) ? !DeeBool_IsTrue(this->ptr()) : object::operator !(); }
#endif /* !__OPTIMIZE_SIZE__ */
};

inline ATTR_NORETURN ATTR_COLD void DCALL throw_last_deemon_exception(void) {
    /* XXX: Exception sub-classes? */
    throw deemon::exception();
}


DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_OBJECT_H */
