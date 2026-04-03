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
#ifndef GUARD_DEEMON_RUNTIME_STATIC_REFCNT_CHECKS_H
#define GUARD_DEEMON_RUNTIME_STATIC_REFCNT_CHECKS_H 1

#include <deemon/api.h>

#undef HAVE_DEBUG_STATIC_REFS
#if defined(CONFIG_EXPERIMENTAL_NO_TP_FHEAP_IS_NOREF_OB_TYPE) && !defined(NDEBUG)
#define HAVE_DEBUG_STATIC_REFS
#endif /* CONFIG_EXPERIMENTAL_NO_TP_FHEAP_IS_NOREF_OB_TYPE && !NDEBUG */

DECL_BEGIN

#ifdef HAVE_DEBUG_STATIC_REFS
/* Dump unexpected changes to reference counters of static objects. */
INTDEF void DCALL DeeDbg_DumpStaticRefChanges(void);
#endif /* HAVE_DEBUG_STATIC_REFS */

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_STATIC_REFCNT_CHECKS_H */
