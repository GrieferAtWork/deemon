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
#ifndef __GUARD_HYBRID_BYTEORDER_H
#define __GUARD_HYBRID_BYTEORDER_H 1

#include <__stdinc.h>
#include "__byteorder.h"

#define BYTE_ORDER          __BYTE_ORDER__
#define ORDER_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define ORDER_BIG_ENDIAN    __ORDER_BIG_ENDIAN__   
#define ORDER_PDP_ENDIAN    __ORDER_PDP_ENDIAN__   

#ifdef __GUARD_HYBRID_BYTESWAP_H
#define LESWAP16(x)    __hybrid_leswap16(x)
#define BESWAP16(x)    __hybrid_beswap16(x)
#define LESWAP16_C(x)  __hybrid_leswap16_c(x)
#define BESWAP16_C(x)  __hybrid_beswap16_c(x)
#define LESWAP32(x)    __hybrid_leswap32(x)
#define BESWAP32(x)    __hybrid_beswap32(x)
#define LESWAP32_C(x)  __hybrid_leswap32_c(x)
#define BESWAP32_C(x)  __hybrid_beswap32_c(x)
#ifdef __UINT64_TYPE__
#define LESWAP64(x)    __hybrid_leswap64(x)
#define BESWAP64(x)    __hybrid_beswap64(x)
#define LESWAP64_C(x)  __hybrid_leswap64_c(x)
#define BESWAP64_C(x)  __hybrid_beswap64_c(x)
#ifdef __UINT128_TYPE__
#define LESWAP128(x)   __hybrid_leswap128(x)
#define BESWAP128(x)   __hybrid_beswap128(x)
#define LESWAP128_C(x) __hybrid_leswap128_c(x)
#define BESWAP128_C(x) __hybrid_beswap128_c(x)
#endif /* __UINT128_TYPE__ */
#endif /* __UINT64_TYPE__ */
#endif /* __GUARD_HYBRID_BYTESWAP_H */

#if defined(__GUARD_HYBRID_UNALIGNED_H) && \
    defined(__GUARD_HYBRID_BYTESWAP_H)
#define UNALIGNED_GETLE(ptr)      __hybrid_unaligned_getle(ptr)
#define UNALIGNED_SETLE(ptr,v)    __hybrid_unaligned_setle(ptr,v)
#define UNALIGNED_GETBE(ptr)      __hybrid_unaligned_getbe(ptr)
#define UNALIGNED_SETBE(ptr,v)    __hybrid_unaligned_setbe(ptr,v)
#define UNALIGNED_GETLE16(ptr)    __hybrid_unaligned_getle16(ptr)
#define UNALIGNED_SETLE16(ptr,v)  __hybrid_unaligned_setle16(ptr,v)
#define UNALIGNED_GETBE16(ptr)    __hybrid_unaligned_getbe16(ptr)
#define UNALIGNED_SETBE16(ptr,v)  __hybrid_unaligned_setbe16(ptr,v)
#define UNALIGNED_GETLE32(ptr)    __hybrid_unaligned_getle32(ptr)
#define UNALIGNED_SETLE32(ptr,v)  __hybrid_unaligned_setle32(ptr,v)
#define UNALIGNED_GETBE32(ptr)    __hybrid_unaligned_getbe32(ptr)
#define UNALIGNED_SETBE32(ptr,v)  __hybrid_unaligned_setbe32(ptr,v)
#ifdef __UINT64_TYPE__
#define UNALIGNED_GETLE64(ptr)    __hybrid_unaligned_getle64(ptr)
#define UNALIGNED_SETLE64(ptr,v)  __hybrid_unaligned_setle64(ptr,v)
#define UNALIGNED_GETBE64(ptr)    __hybrid_unaligned_getbe64(ptr)
#define UNALIGNED_SETBE64(ptr,v)  __hybrid_unaligned_setbe64(ptr,v)
#ifdef __UINT128_TYPE__
#define UNALIGNED_GETLE128(ptr)   __hybrid_unaligned_getle128(ptr)
#define UNALIGNED_SETLE128(ptr,v) __hybrid_unaligned_setle128(ptr,v)
#define UNALIGNED_GETBE128(ptr)   __hybrid_unaligned_getbe128(ptr)
#define UNALIGNED_SETBE128(ptr,v) __hybrid_unaligned_setbe128(ptr,v)
#endif /* __UINT128_TYPE__ */
#endif /* __UINT64_TYPE__ */
#endif

#endif /* !__GUARD_HYBRID_BYTEORDER_H */
