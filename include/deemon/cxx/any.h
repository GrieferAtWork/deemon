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
#ifndef GUARD_DEEMON_CXX_ANY_H
#define GUARD_DEEMON_CXX_ANY_H 1

#include "api.h"
#include "object.h"
#include "../int.h"
#include "../string.h"
#include "../bool.h"
#include "../float.h"

#include <type_traits>
#include <initializer_list>
#include <string.h>
#ifdef CONFIG_DEEMON_HAVE_NATIVE_WCHAR_T
#include <wchar.h>
#endif /* CONFIG_DEEMON_HAVE_NATIVE_WCHAR_T */

DEE_CXX_BEGIN

namespace detail {
class any_convertible_base {
public:
    typedef int available;
};
template<class T, bool C> class any_convertible_cc;
template<class T> class any_convertible_cc<T,true>: public any_convertible_base {
public:
    static DREF DeeObject *convert(T const &value) {
        return (DREF DeeObject *)value;
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


/* Helper object type with a custom constructor capable of implicitly
 * casting pretty much anything that can directly be represented in deemon */
class any: public object {
public:
    DEFINE_OBJECT_CONSTRUCTORS(any,object)
    template<class T> any(T const &init, typename detail::any_convertible<T>::available* =0)
        : object(inherit(detail::any_convertible<T>::convert(init))) { }
    template<class T> any(std::initializer_list<T> const &init, typename detail::any_convertible<std::initializer_list<T> >::available* =0)
        : object(inherit(detail::any_convertible<std::initializer_list<T> >::convert(init))) { }
};




DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_ANY_H */
