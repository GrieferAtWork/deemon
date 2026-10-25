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
#ifndef GUARD_DEEMON_COMPILER_CONST_H
#define GUARD_DEEMON_COMPILER_CONST_H 1 /*!export-*/

#include "../api.h"

#include "../types.h" /* DeeObject */

#include <stdbool.h> /* bool */

#ifdef CONFIG_BUILDING_DEEMON
DECL_BEGIN

/* Do a compile-time, constant check that "a" and "b" are *deeply* equal.
 * This means that:
 * - Both must have the same type
 * - The type must be immutable and known to the compiler
 * - In case of sequence types, element counts must match, and all
 *   elements must be equal according to this function also.
 *
 * NOTE:
 * - This function is mainly needed because of stuff like `0 == none`,
 *   which is the case because `int(none) == 0`, which would otherwise
 *   mean that `{0}` and `{none}` might (sometimes) compare equal. */
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
DeeObject_ConstEquals(DeeObject const *a,
                      DeeObject const *b);
INTDEF ATTR_PURE WUNUSED bool DCALL
DeeObject_XConstEquals(DeeObject const *a,
                       DeeObject const *b);

DECL_END
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !GUARD_DEEMON_COMPILER_CONST_H */
