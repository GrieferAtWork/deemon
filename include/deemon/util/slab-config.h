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
/*!export **/
#ifndef GUARD_DEEMON_UTIL_SLAB_CONFIG_H
#define GUARD_DEEMON_UTIL_SLAB_CONFIG_H 1 /*!export-*/

#include "../api.h"

#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR

#include <hybrid/typecore.h> /* __SIZEOF_POINTER__ */

#if __SIZEOF_POINTER__ == 4
#define Dee_SLAB_CHUNKSIZE_MIN 12
#define Dee_SLAB_CHUNKSIZE_MAX 40
#define Dee_SLAB_CHUNKSIZE_FOREACH(cb, _)    cb(12, _) cb(16, _) cb(20, _) cb(24, _) cb(32, _) cb(40, _)
#define Dee_SLAB_CHUNKSIZE_GC_FOREACH(cb, _) cb(12/*20*/, _) cb(16/*24*/, _) cb(24/*32*/, _) cb(32/*40*/, _)
#elif __SIZEOF_POINTER__ == 8
#define Dee_SLAB_CHUNKSIZE_MIN 24
#define Dee_SLAB_CHUNKSIZE_MAX 80
#define Dee_SLAB_CHUNKSIZE_FOREACH(cb, _)    cb(24, _) cb(32, _) cb(40, _) cb(48, _) cb(64, _) cb(80, _)
#define Dee_SLAB_CHUNKSIZE_GC_FOREACH(cb, _) cb(24/*40*/, _) cb(32/*48*/, _) cb(48/*64*/, _) cb(64/*80*/, _)
#else /* __SIZEOF_POINTER__ == ... */
#define Dee_SLAB_CHUNKSIZE_FOREACH(cb, _)    /* nothing */
#define Dee_SLAB_CHUNKSIZE_GC_FOREACH(cb, _) /* nothing */
#endif /* __SIZEOF_POINTER__ != ... */

#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

#endif /* !GUARD_DEEMON_UTIL_SLAB_CONFIG_H */
