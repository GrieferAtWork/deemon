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
#ifndef __GUARD_HYBRID_LIMITS_H
#define __GUARD_HYBRID_LIMITS_H 1

#include "host.h"

#if defined(__i386__) || \
    defined(__x86_64__) || \
    defined(__arm__)
#   define __PAGESIZE        4096
#endif
#if defined(__i386__) || defined(__x86_64__)
#   define __CACHELINE       64
#elif 0
#   define __CACHELINE       64 /* Just guess... */
#endif

#ifdef __GUARD_HYBRID_COMPILER_H
#ifndef PAGESIZE
#ifdef __PAGESIZE
#   define PAGESIZE          __PAGESIZE
#endif
#endif /* !PAGESIZE */
#ifdef __CACHELINE
#   define CACHELINE         __CACHELINE
#   define CACHELINE_ALIGNED ATTR_ALIGNED(CACHELINE)
#else
#   define CACHELINE_ALIGNED /* Nothing */
#endif
#endif

#endif /* !__GUARD_HYBRID_LIMITS_H */
