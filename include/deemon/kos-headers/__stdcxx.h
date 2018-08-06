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
#ifndef ___STDCXX_H
#define ___STDCXX_H 1

#include "__stdinc.h"

#ifndef __cplusplus
#error "A C++ compiler is required"
#endif

/* TODO: Determine support for these */
#define __COMPILER_HAVE_CXX_RVALUE_REFERENCE 1
#define __COMPILER_HAVE_CXX_PARTIAL_TPL_SPEC 1
#define __COMPILER_HAVE_CXX_DECLTYPE 1
#define __COMPILER_HAVE_CXX_NULLPTR 1
#define __CXX_DEDUCE_TYPENAME typename

#if __has_feature(defaulted_functions) || \
   (defined(__cplusplus) && defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 180020827) || \
   (defined(__GNUC__) /* TODO: Which version? */)
#   define __CXX_HAVE_DEFAULT_FUNCTIONS 1
#   define __CXX_DEFAULT_CTOR(T)                 T() = default
#   define __CXX_DEFAULT_DTOR(T)                 ~T() = default
#ifdef __COMPILER_HAVE_CXX11_NOEXCEPT
#   define __CXX_DEFAULT_CTOR_NOEXCEPT(T)        T() __CXX_NOEXCEPT = default
#   define __CXX_DEFAULT_DTOR_NOEXCEPT(T)        ~T() __CXX_NOEXCEPT = default
#else /* __COMPILER_HAVE_CXX11_NOEXCEPT */
#   define __CXX_DEFAULT_CTOR_NOEXCEPT(T)        T() = default
#   define __CXX_DEFAULT_DTOR_NOEXCEPT(T)        ~T() = default
#endif /* !__COMPILER_HAVE_CXX11_NOEXCEPT */
#else
#   define __CXX_DEFAULT_CTOR(T)                 T() {}
#   define __CXX_DEFAULT_DTOR(T)                 ~T() {}
#   define __CXX_DEFAULT_CTOR_NOEXCEPT(T)        T() __CXX_NOEXCEPT {}
#   define __CXX_DEFAULT_DTOR_NOEXCEPT(T)        ~T() __CXX_NOEXCEPT {}
#endif

#if __has_feature(deleted_functions) || \
   (defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 180020827) || \
   (defined(__GNUC__) /* TODO: Which version? */)
#   define __CXX_HAVE_DELETE_FUNCTIONS 1
#   define __CXX_DELETE_CTOR(T)                 T() = delete
#   define __CXX_DELETE_DTOR(T)                 ~T() = delete
#   define __CXX_DELETE_COPY(T)                 T(T const&) = delete
#   define __CXX_DELETE_COPY_ASSIGN(T)          T &operator = (T const&) = delete
#ifdef _MSC_VER
#   define __CXX_DELETE_VOLATILE_COPY_ASSIGN(T) /* Nothing */
#else
#   define __CXX_DELETE_VOLATILE_COPY_ASSIGN(T) T &operator = (T const&) volatile = delete
#endif
#else
#   define __CXX_DELETE_CTOR(T)                 private: T()
#   define __CXX_DELETE_DTOR(T)                 private: ~T()
#   define __CXX_DELETE_COPY(T)                 private: T(T const&)
#   define __CXX_DELETE_COPY_ASSIGN(T)          private: T &operator = (T const&)
#   define __CXX_DELETE_VOLATILE_COPY_ASSIGN(T) private: T &operator = (T const&) volatile
#endif

#ifdef _MSC_VER
/* 4522: Incorrect warning about multiple assignment
 *       operators after `__CXX_DELETE_COPY_ASSIGN()' */
#define __CXXDECL_BEGIN __pragma(warning(push)) \
                        __pragma(warning(disable: 4522))
#define __CXXDECL_END   __pragma(warning(pop))
#endif

#ifdef __COMPILER_HAVE_CXX11_NOEXCEPT
#   define __CXX_THROWS(...) /* Nothing */
#elif defined(_MSC_VER) && !defined(__INTELLISENSE__)
#   define __CXX_THROWS(...) /* Nothing */
#else
#   define __CXX_THROWS(...) throw(__VA_ARGS__)
#endif

#define __CXX_ADDROF(x) (&(x)) /* TODO: It's more complicated that this... */

#ifndef __CXXDECL_BEGIN
#define __CXXDECL_BEGIN
#define __CXXDECL_END
#endif

#include <features.h>
#undef __USE_GLIBCXX
#ifdef __CRT_GLC
#define __USE_GLIBCXX 1
#endif

#endif /* !___STDCXX_H */
