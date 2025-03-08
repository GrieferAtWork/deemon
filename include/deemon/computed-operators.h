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
#if defined(CONFIG_WITHOUT_COMPUTED_DEFAULT_OPERATORS) || 0 /*<<< change to "1" to quickly disable */
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
#include "object.h"

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
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Concat(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default_seq_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default_set_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_object_compare_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_object_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL generic_object_ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_object_trycompare_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL iterator_add(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1)) int DCALL iterator_bool(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL iterator_compare(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1)) int DCALL iterator_dec(DREF DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL iterator_inc(DREF DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL iterator_inplace_add(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL iterator_inplace_sub(DREF DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL iterator_next(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL iterator_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL iterator_sub(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL map_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL module_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL module_str(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL object_repr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL object_str(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL type_compare_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_eq(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_ne(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL type_print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL type_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL type_repr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL type_str(DeeObject *__restrict self);
INTDEF struct type_cmp default__tp_cmp__247219960F1E745D;
INTDEF struct type_cmp default__tp_cmp__26B2EC529683DE3C;
INTDEF struct type_cmp default__tp_cmp__287C06B7236F06BE;
INTDEF struct type_cmp default__tp_cmp__2B5761B4075B51D3;
INTDEF struct type_cmp default__tp_cmp__3C4D336761465F8A;
INTDEF struct type_cmp default__tp_cmp__40D3D60A1F18CAE2;
INTDEF struct type_cmp default__tp_cmp__50A436E90E5A2AF0;
INTDEF struct type_cmp default__tp_cmp__5819FE7E0C5EF426;
INTDEF struct type_cmp default__tp_cmp__6F3C9C45873AB01F;
INTDEF struct type_cmp default__tp_cmp__7188129899C2A8D6;
INTDEF struct type_cmp default__tp_cmp__7EA181D4706D1525;
INTDEF struct type_cmp default__tp_cmp__ABC6920EC80A6EC1;
INTDEF struct type_cmp default__tp_cmp__B8EC3298B952DF3A;
INTDEF struct type_cmp default__tp_cmp__C2B62E6BCA44673D;
INTDEF struct type_cmp default__tp_cmp__C6AA9DC8372C283F;
INTDEF struct type_cmp default__tp_cmp__CE2E4B8E19554701;
INTDEF struct type_cmp default__tp_cmp__DC202CECA797EF15;
INTDEF struct type_iterator default__tp_iterator__863AC70046E4B6B0;
INTDEF struct type_math default__tp_math__385A9235483A0324;
INTDEF struct type_math default__tp_math__3959C0D1502AC76A;
INTDEF struct type_math default__tp_math__56685E2B01B76756;
INTDEF struct type_math default__tp_math__667432E5904B49F8;
INTDEF struct type_math default__tp_math__7C9B3D263E47878C;
INTDEF struct type_math default__tp_math__9211580AA9433079;
INTDEF struct type_math default__tp_math__AFC6A8FA89E9F0A6;
INTDEF struct type_math default__tp_math__BA555DDFFD44D1A5;
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
