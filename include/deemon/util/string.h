/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_UTIL_STRING_H
#define GUARD_DEEMON_UTIL_STRING_H 1

#include "../api.h"
/**/

#include <hybrid/typecore.h>

#include "../system-features.h"

#if __SIZEOF_POINTER__ == 4
#ifndef CONFIG_HAVE_memsetl
#define CONFIG_HAVE_memsetl 1
DECL_BEGIN
#undef memsetl
#define memsetl dee_memsetl
DeeSystem_DEFINE_memsetl(dee_memsetl)
DECL_END
#endif /* !CONFIG_HAVE_memsetl */

#define MEMFIL_PTR(dst, ptr, n)  memsetl(dst, (__UINT32_TYPE__)(ptr), n)
#define MEMSET_PTR(dst, byte, n) memsetl(dst, (__UINT32_TYPE__)(byte)*__UINT32_C(0x01010101), n)
#define MEMCPY_PTR(dst, src, n)  memcpyl(dst, src, n)
#define MEMMOVE_PTR(dst, src, n) memmovel(dst, src, n)

#elif __SIZEOF_POINTER__ == 8
#ifndef CONFIG_HAVE_memsetq
#define CONFIG_HAVE_memsetq 1
DECL_BEGIN
#undef memsetq
#define memsetq dee_memsetq
DeeSystem_DEFINE_memsetq(dee_memsetq)
DECL_END
#endif /* !CONFIG_HAVE_memsetq */

#define MEMFIL_PTR(dst, ptr, n)  memsetq(dst, (__UINT64_TYPE__)(ptr), n)
#define MEMSET_PTR(dst, byte, n) memsetq(dst, (__UINT64_TYPE__)(byte)*__UINT64_C(0x0101010101010101), n)
#define MEMCPY_PTR(dst, src, n)  memcpyq(dst, src, n)
#define MEMMOVE_PTR(dst, src, n) memmoveq(dst, src, n)

#elif __SIZEOF_POINTER__ == 2
#ifndef CONFIG_HAVE_memsetw
#define CONFIG_HAVE_memsetw 1
DECL_BEGIN
#undef memsetw
#define memsetw dee_memsetw
DeeSystem_DEFINE_memsetw(dee_memsetw)
DECL_END
#endif /* !CONFIG_HAVE_memsetw */

#define MEMFIL_PTR(dst, ptr, n)  memsetw(dst, (__UINT16_TYPE__)(ptr), n)
#define MEMSET_PTR(dst, byte, n) memsetw(dst, (__UINT16_TYPE__)(byte)*__UINT16_C(0x0101), n)
#define MEMCPY_PTR(dst, src, n)  memcpyw(dst, src, n)
#define MEMMOVE_PTR(dst, src, n) memmovew(dst, src, n)

#elif __SIZEOF_POINTER__ == 1
#define MEMFIL_PTR(dst, ptr, n)  memsetb(dst, (__UINT8_TYPE__)(ptr), n)
#define MEMSET_PTR(dst, byte, n) memsetb(dst, byte, n)
#define MEMCPY_PTR(dst, src, n)  memcpyb(dst, src, n)
#define MEMMOVE_PTR(dst, src, n) memmoveb(dst, src, n)
#else
#error "Unsupported __SIZEOF_POINTER__"
#endif


#endif /* !GUARD_DEEMON_UTIL_STRING_H */
