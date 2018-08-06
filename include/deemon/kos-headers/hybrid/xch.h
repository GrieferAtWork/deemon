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
#ifndef __GUARD_HYBRID_XCH_H
#define __GUARD_HYBRID_XCH_H 1

#include <__stdinc.h>
#include "host.h"

__SYSDECL_BEGIN

#if !defined(__GNUC__) && !defined(__DCC_VERSION__) && !defined(__TINYC__)
#ifdef __cplusplus
extern "C++" { template<class __T> __FORCELOCAL __T (__cxx_xch)(__T &__x, __T __y) { __T __res = __x; __x = __y; return __res; } }
#define __XCH(x,y) ::__cxx_xch(x,y)
#else
__FORCELOCAL __UINT8_TYPE__ __xch8(void *__x, __UINT8_TYPE__ __y) { __UINT8_TYPE__ __res = *(__UINT8_TYPE__ *)__x; *(__UINT8_TYPE__ *)__x = __y; return __res; }
__FORCELOCAL __UINT16_TYPE__ __xch16(void *__x, __UINT16_TYPE__ __y) { __UINT16_TYPE__ __res = *(__UINT16_TYPE__ *)__x; *(__UINT16_TYPE__ *)__x = __y; return __res; }
__FORCELOCAL __UINT32_TYPE__ __xch32(void *__x, __UINT32_TYPE__ __y) { __UINT32_TYPE__ __res = *(__UINT32_TYPE__ *)__x; *(__UINT32_TYPE__ *)__x = __y; return __res; }
__FORCELOCAL __UINT64_TYPE__ __xch64(void *__x, __UINT64_TYPE__ __y) { __UINT64_TYPE__ __res = *(__UINT64_TYPE__ *)__x; *(__UINT64_TYPE__ *)__x = __y; return __res; }
#ifdef __COMPILER_HAVE_TYPEOF
#define __XCH(x,y) \
  ((__typeof__(x))(sizeof(x) == 1 ? __xch8(&(x),(__UINT8_TYPE__)(y)) : \
                   sizeof(x) == 2 ? __xch16(&(x),(__UINT16_TYPE__)(y)) : \
                   sizeof(x) == 4 ? __xch32(&(x),(__UINT32_TYPE__)(y)) : \
                                    __xch64(&(x),(__UINT64_TYPE__)(y))))
#else /* __COMPILER_HAVE_TYPEOF */
#define __XCH(x,y) \
  (sizeof(x) == 1 ? __xch8(&(x),(__UINT8_TYPE__)(y)) : \
   sizeof(x) == 2 ? __xch16(&(x),(__UINT16_TYPE__)(y)) : \
   sizeof(x) == 4 ? __xch32(&(x),(__UINT32_TYPE__)(y)) : \
                    __xch64(&(x),(__UINT64_TYPE__)(y)))
#endif /* !__COMPILER_HAVE_TYPEOF */
#endif
#elif defined(__x86_64__) && 0
#define __XCH(x,y) \
 __XBLOCK({ __typeof__(x) __oldx; \
            if (sizeof(__oldx) == 1) { \
              __oldx = (y); \
              __asm__("xchgb %0, %1\n" : "+g" (x), "+r" (__oldx)); \
            } else if (sizeof(__oldx) == 2) { \
              __oldx = (y); \
              __asm__("xchgw %0, %1\n" : "+g" (x), "+r" (__oldx)); \
            } else if (sizeof(__oldx) == 4) { \
              __oldx = (y); \
              __asm__("xchgl %0, %1\n" : "+g" (x), "+r" (__oldx)); \
            } else if (sizeof(__oldx) == 8) { \
              __oldx = (y); \
              __asm__("xchgq %0, %1\n" : "+g" (x), "+r" (__oldx)); \
            } else {\
              __oldx = (x); \
              (x) = (y); \
            } \
            __XRETURN __oldx; \
 })
#elif defined(__i386__) && 0
#define __XCH(x,y) \
 __XBLOCK({ __typeof__(x) __oldx; \
            if (sizeof(__oldx) == 1) { \
              __oldx = (y); \
              __asm__("xchgb %0, %1\n" : "+g" (x), "+r" (__oldx)); \
            } else if (sizeof(__oldx) == 2) { \
              __oldx = (y); \
              __asm__("xchgw %0, %1\n" : "+g" (x), "+r" (__oldx)); \
            } else if (sizeof(__oldx) == 4) { \
              __oldx = (y); \
              __asm__("xchgl %0, %1\n" : "+g" (x), "+r" (__oldx)); \
            } else {\
              __oldx = (x); \
              (x) = (y); \
            } \
            __XRETURN __oldx; \
 })
#else
#define __XCH(x,y) \
 __XBLOCK({ __typeof__(x) __oldx = (x); \
            (x) = (y); \
            __XRETURN __oldx; \
 })
#endif


#ifdef __GUARD_HYBRID_COMPILER_H
#define XCH(x,y) __XCH(x,y)
#endif

__SYSDECL_END

#endif /* !__GUARD_HYBRID_XCH_H */
