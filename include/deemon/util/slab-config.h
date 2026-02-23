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
/*!export Dee_SLAB_**/
#ifndef GUARD_DEEMON_UTIL_SLAB_CONFIG_H
#define GUARD_DEEMON_UTIL_SLAB_CONFIG_H 1 /*!export-*/

#include "../api.h"

#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR

#include <hybrid/typecore.h> /* __SIZEOF_POINTER__ */

/* Host-specific slab configuration:
 *
 * >> #define Dee_SLAB_CHUNKSIZE_MIN               <Smallest slab size from Dee_SLAB_CHUNKSIZE_FOREACH()>
 * >> #define Dee_SLAB_CHUNKSIZE_MAX               <Greatest slab size from Dee_SLAB_CHUNKSIZE_FOREACH()>
 * >> #define Dee_SLAB_CHUNKSIZE_FOREACH(cb, _)    <Invoke 'cb(n, _)' for every supported slab size>
 * >> #define Dee_SLAB_CHUNKSIZE_GC_FOREACH(cb, _) <Invoke 'cb(n, _)' for every supported GC slab size>
 *
 * Notes on GC slabs:
 * - GC slabs aren't actually distinct from regular slabs, they only need
 *   a different enumeration and name because they allocate slabs that have
 *   some extra space for a leading `struct Dee_gc_head'
 * - As such (with 32-bit pointers), `DeeGCSlab_Malloc16()' would just be a
 *   wrapper around `DeeSlab_Malloc24()' that adjusts the returned pointer
 *   by an offset of `8' bytes (on success).
 * - Also as a consequence, the macro `Dee_SLAB_CHUNKSIZE_GC_FOREACH()'
 *   doesn't actually add anything new (the set of supported GC slabs is
 *   already defined by `Dee_SLAB_CHUNKSIZE_FOREACH()'), only that another
 *   macro is needed because the preprocessor can't evaluate the logic:
 *   >> SLABS    = {<some set of integers>}
 *   >> GC_SLABS = SOME_SUBSET_OF((
 *   >>     for (local n: SLABS)
 *   >>         if (n > sizeof(struct Dee_gc_head))
 *   >>             n - sizeof(struct Dee_gc_head)
 *   >> ).asset)
 *   iow: every slab (can) have a GC sibling after subtracting
 *        `sizeof(struct Dee_gc_head)' from the slab's size.
 */

#undef Dee_SLAB_CHUNKSIZE_MIN
#undef Dee_SLAB_CHUNKSIZE_MAX
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
