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
#ifndef GUARD_DEX_RT_LIBRT_H
#define GUARD_DEX_RT_LIBRT_H 1

#include <deemon/api.h>

#include <deemon/alloc.h>  /* DeeSlabInfo, DeeSlabStat */
#include <deemon/object.h> /* DREF, DeeTypeObject, OBJECT_HEAD */

DECL_BEGIN

#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
/* TODO: New introspection API for:
 * - The reworked slab allocator (does not even have a C API for this, yet)
 * - The new "CONFIG_EXPERIMENTAL_CUSTOM_HEAP" heap: DeeHeap_MallInfo() */
#else /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
/* Types for accessing slab allocator statistics. */

typedef struct {
	OBJECT_HEAD
	DeeSlabStat st_stat; /* [const] Slab statistics. */
} SlabStatObject;

typedef struct {
	OBJECT_HEAD
	DREF SlabStatObject *si_stat; /* [1..1][const] Slab statistics. */
	DeeSlabInfo         *si_info; /* [1..1][const] Slab information. */
} SlabInfoObject;

INTDEF DeeTypeObject SlabStat_Type;
INTDEF DeeTypeObject SlabInfo_Type;
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

INTDEF DeeTypeObject StringFiniHook_Type;

DECL_END

#endif /* !GUARD_DEX_RT_LIBRT_H */
