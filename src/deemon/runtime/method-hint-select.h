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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINT_SELECT_H
#define GUARD_DEEMON_RUNTIME_METHOD_HINT_SELECT_H 1

#include <deemon/api.h>

#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/alloc.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>

DECL_BEGIN

/* clang-format off */
/*[[[deemon (printMhInitSelectDecls from "..method-hints.method-hints")();]]]*/
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_bool_t DCALL mh_select_seq_operator_bool(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_sizeob_t DCALL mh_select_seq_operator_sizeob(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_size_t DCALL mh_select_seq_operator_size(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_iter_t DCALL mh_select_seq_operator_iter(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_foreach_t DCALL mh_select_seq_operator_foreach(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_foreach_pair_t DCALL mh_select_seq_operator_foreach_pair(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_iterkeys_t DCALL mh_select_seq_operator_iterkeys(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_enumerate_t DCALL mh_select_seq_operator_enumerate(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_enumerate_index_t DCALL mh_select_seq_operator_enumerate_index(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_t DCALL mh_select_seq_any(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_with_key_t DCALL mh_select_seq_any_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_with_range_t DCALL mh_select_seq_any_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_with_range_and_key_t DCALL mh_select_seq_any_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_all_t DCALL mh_select_seq_all(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_all_with_key_t DCALL mh_select_seq_all_with_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_all_with_range_t DCALL mh_select_seq_all_with_range(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_all_with_range_and_key_t DCALL mh_select_seq_all_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_trygetfirst_t DCALL mh_select_seq_trygetfirst(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_getfirst_t DCALL mh_select_seq_getfirst(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_boundfirst_t DCALL mh_select_seq_boundfirst(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_delfirst_t DCALL mh_select_seq_delfirst(DeeTypeObject *self, DeeTypeObject *orig_type);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_setfirst_t DCALL mh_select_seq_setfirst(DeeTypeObject *self, DeeTypeObject *orig_type);
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_SELECT_H */
