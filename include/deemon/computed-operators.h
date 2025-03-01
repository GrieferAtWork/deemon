/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPUTED_OPERATORS_H
#define GUARD_DEEMON_COMPUTED_OPERATORS_H 1

#include "api.h"

#ifdef CONFIG_BUILDING_DEEMON

/* Figure out of computed operators should be used */
#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
#undef CONFIG_WITHOUT_COMPUTED_DEFAULT_OPERATORS
#undef CONFIG_WITH_COMPUTED_DEFAULT_OPERATORS
#define CONFIG_WITHOUT_COMPUTED_DEFAULT_OPERATORS
#elif defined(CONFIG_WITHOUT_COMPUTED_DEFAULT_OPERATORS) || 0 /*<<< change to "1" to quickly disable */
#undef CONFIG_WITH_COMPUTED_DEFAULT_OPERATORS
#undef CONFIG_WITHOUT_COMPUTED_DEFAULT_OPERATORS
#define CONFIG_WITHOUT_COMPUTED_DEFAULT_OPERATORS
#elif defined(CONFIG_WITH_COMPUTED_DEFAULT_OPERATORS) || 0
#undef CONFIG_WITHOUT_COMPUTED_DEFAULT_OPERATORS
#undef CONFIG_WITH_COMPUTED_DEFAULT_OPERATORS
#define CONFIG_WITH_COMPUTED_DEFAULT_OPERATORS
#else /* ... */
#include <hybrid/host.h> /* __pic__ */
/* Disable computed operators when the deemon core is built as position-independent code.
 * Reason is that position-independent code requires relocations being generated for every
 * function pointer that appears in program data, so computed operators would add a heap
 * of overhead to program startup time. */
#ifdef __pic__
#define CONFIG_WITHOUT_COMPUTED_DEFAULT_OPERATORS
#else /* __pic__ */
#define CONFIG_WITH_COMPUTED_DEFAULT_OPERATORS
#endif /* !__pic__ */
#endif /* !... */


#ifndef CONFIG_WITHOUT_COMPUTED_DEFAULT_OPERATORS

/* Have a deemon program that:
 * - scans the deemon source code for type declarations
 * - resolves those types to their respective deemon objects
 * - uses debug in as per "/util/test/deemon-operator-linkage.dee"
 *   to figure out all the default operators
 * - automatically completes those types by replacing "NULL" values
 *   in type initializers with "DEFIMPL(...)"
 * >> PRIVATE struct type_seq mytyp_seq = {
 * >>     / * .tp_iter     = * / &mytyp_iter,
 * >>     / * .tp_sizeob   = * / DEFIMPL(&default__sizeob__with__size),
 * >>     / * .tp_contains = * / ...,
 * >> };
 * >> PRIVATE DeeTypeObject MyObject_Type = {
 * >>     OBJECT_HEAD_INIT(&DeeType_Type),
 * >>     / * .tp_name * / "_MyObject", // Script should find this one as "rt.MyObject"
 * >>     ...
 * >>     / * .tp_seq  * / &mytyp_seq,
 * >> };
 *
 * Obviously, this magic script can only run when deemon was
 * built with `CONFIG_WITHOUT_COMPUTED_DEFAULT_OPERATORS'.
 *
 * To re-generate computed operators:
 * #1: Build deemon with CONFIG_WITHOUT_COMPUTED_DEFAULT_OPERATORS
 * #2: `$ make computed-operators`
 */
#define DEFIMPL(x) x
#include "../../src/deemon/runtime/method-hint-defaults.h" /*!KEEPME*/
#include "operator-hints.h"                                /*!KEEPME*/

#define DEFAULT_OPDEF INTDEF
#define DEFAULT_OPIMP INTERN

DECL_BEGIN

/* Reusable default operators (and operator callbacks that get inherited) */
/*[[[begin::computer-operator-decls]]]*/
/*[[[end::computer-operator-decls]]]*/

INTDEF Dee_hash_t DCALL default__hash__unsupported(DeeObject *__restrict self);

DECL_END
#else /* !CONFIG_WITHOUT_COMPUTED_DEFAULT_OPERATORS */
#define DEFIMPL(x) NULL

#define DEFAULT_OPDEF PRIVATE
#define DEFAULT_OPIMP PRIVATE
#endif /* CONFIG_WITHOUT_COMPUTED_DEFAULT_OPERATORS */
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !GUARD_DEEMON_COMPUTED_OPERATORS_H */
