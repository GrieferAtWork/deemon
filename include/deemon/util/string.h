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
#ifndef GUARD_DEEMON_UTIL_STRING_H
#define GUARD_DEEMON_UTIL_STRING_H 1
#ifndef _KOS_SOURCE
#define _KOS_SOURCE 1
#endif

#include "../api.h"
#include <string.h>

#ifdef __USE_KOS
#include <hybrid/typecore.h>
#if __SIZEOF_POINTER__ == 4
#define MEMFIL_PTR(dst, ptr, n)  memsetl(dst, (__UINT32_TYPE__)(ptr), n)
#define MEMSET_PTR(dst, byte, n) memsetl(dst, (__UINT32_TYPE__)(byte)*__UINT32_C(0x01010101), n)
#define MEMCPY_PTR(dst, src, n)  memcpyl(dst, src, n)
#define MEMMOVE_PTR(dst, src, n) memmovel(dst, src, n)
#elif __SIZEOF_POINTER__ == 8
#define MEMFIL_PTR(dst, ptr, n)  memsetq(dst, (__UINT64_TYPE__)(ptr), n)
#define MEMSET_PTR(dst, byte, n) memsetq(dst, (__UINT64_TYPE__)(byte)*__UINT64_C(0x0101010101010101), n)
#define MEMCPY_PTR(dst, src, n)  memcpyq(dst, src, n)
#define MEMMOVE_PTR(dst, src, n) memmoveq(dst, src, n)
#elif __SIZEOF_POINTER__ == 2
#define MEMFIL_PTR(dst, ptr, n)  memsetw(dst, (__UINT16_TYPE__)(ptr), n)
#define MEMSET_PTR(dst, byte, n) memsetw(dst, (__UINT16_TYPE__)(byte)*__UINT16_C(0x0101), n)
#define MEMCPY_PTR(dst, src, n)  memcpyw(dst, src, n)
#define MEMMOVE_PTR(dst, src, n) memmovew(dst, src, n)
#elif __SIZEOF_POINTER__ == 1
#define MEMFIL_PTR(dst, ptr, n)  memsetb(dst, (__UINT8_TYPE__)(ptr), n)
#define MEMSET_PTR(dst, byte, n) memsetb(dst, byte, n)
#define MEMCPY_PTR(dst, src, n)  memcpyb(dst, src, n)
#define MEMMOVE_PTR(dst, src, n) memmoveb(dst, src, n)
#endif
#endif /* __USE_KOS */



#ifndef MEMFIL_PTR
#if __SIZEOF_POINTER__ == 1
#define MEMFIL_PTR(dst, ptr, n) memset(dst, (__UINT8_TYPE__)(ptr), (n) * sizeof(void *))
#elif defined(_MSC_VER) && defined(__x86_64__)
DECL_BEGIN
extern void __stosq(unsigned __int64 *, unsigned __int64, unsigned __int64);
DECL_END
#pragma intrinsic(__stosq)
#define MEMFIL_PTR(dst, ptr, n) __stosq((unsigned __int64 *)(dst), (unsigned __int64)(ptr), n)
#elif defined(_MSC_VER) && defined(__i386__)
DECL_BEGIN
extern void __stosd(unsigned long *, unsigned long, unsigned int);
DECL_END
#pragma intrinsic(__stosd)
#define MEMFIL_PTR(dst, ptr, n) __stosd((unsigned long *)(dst), (unsigned long)(ptr), n)
#else
#include <hybrid/typecore.h>
DECL_BEGIN
FORCELOCAL void DCALL
dee_memfil_ptr(void **__restrict dst, void *ptr, __SIZE_TYPE__ n) {
	while (n--)
		*dst++ = ptr;
}
DECL_END
#define MEMFIL_PTR(dst, ptr, n) dee_memfil_ptr((void **)(dst), (void *)(ptr), n)
#endif
#endif /* !MEMFIL_PTR */

#ifndef MEMSET_PTR
#define MEMSET_PTR(dst, byte, n) memset(dst, byte, (n) * sizeof(void *))
#endif /* !MEMSET_PTR */
#ifndef MEMCPY_PTR
#define MEMCPY_PTR(dst, src, n) memcpy(dst, src, (n) * sizeof(void *))
#endif /* !MEMCPY_PTR */
#ifndef MEMMOVE_PTR
#define MEMMOVE_PTR(dst, src, n) memmove(dst, src, (n) * sizeof(void *))
#endif /* !MEMMOVE_PTR */


#endif /* !GUARD_DEEMON_UTIL_STRING_H */
