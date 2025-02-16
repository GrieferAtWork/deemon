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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_HINT_ERRORS_H
#define GUARD_DEEMON_RUNTIME_OPERATOR_HINT_ERRORS_H 1

#include <deemon/api.h>
#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/object.h>

/**/
#include "method-hint-defaults.h"

DECL_BEGIN

/* Custom impls, as referenced by:
 * >> [[custom_unsupported_impl_name(default__hash__unsupported)]] */
INTDEF Dee_hash_t DCALL default__hash__unsupported(DeeObject *__restrict self);
#define default__trycompare_eq__unsupported \
	default__seq_operator_trycompare_eq__unsupported

/* TODO: Types should be able to "delete" operators by assigning the `default__*__unsupported'
 *       implementation to the relevant slot, and the operator inheritance engine should just
 *       notice those assignment and stop trying to find the operator in that sub-tree of types */

/* clang-format off */
/*[[[deemon (printNativeOperatorHintErrorImpls from "..method-hints.method-hints")(decls: true);]]]*/
INTDEF int DCALL default__assign__unsupported(DeeObject*, void*);
INTDEF int DCALL default__move_assign__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__str__unsupported(DeeObject*);
INTDEF Dee_ssize_t DCALL default__print__unsupported(DeeObject*, Dee_formatprinter_t, void*);
INTDEF void*DCALL default__repr__unsupported(DeeObject*);
INTDEF Dee_ssize_t DCALL default__printrepr__unsupported(DeeObject*, Dee_formatprinter_t, void*);
INTDEF int DCALL default__bool__unsupported(DeeObject*);
INTDEF void*DCALL default__call__unsupported(DeeObject*, void*, void*);
INTDEF void*DCALL default__call_kw__unsupported(DeeObject*, void*, void*, void*);
INTDEF void*DCALL default__iter_next__unsupported(DeeObject*);
#define default__nextkey__unsupported default__iter_next__unsupported
#define default__nextvalue__unsupported default__iter_next__unsupported
INTDEF int DCALL default__nextpair__badalloc(void*, void*);
INTDEF int DCALL default__nextpair__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__nextkey__badalloc(void*);
#define default__nextvalue__badalloc default__nextkey__badalloc
INTDEF Dee_ssize_t DCALL default__advance__badalloc(void*, void*);
INTDEF Dee_ssize_t DCALL default__advance__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__int__badalloc(void*);
#define default__inv__badalloc default__int__badalloc
#define default__pos__badalloc default__int__badalloc
#define default__neg__badalloc default__int__badalloc
INTDEF void*DCALL default__int__unsupported(DeeObject*);
INTDEF int DCALL default__int32__badalloc(void*, void*);
#define default__int64__badalloc default__int32__badalloc
#define default__double__badalloc default__int32__badalloc
#define default__inplace_add__badalloc default__int32__badalloc
#define default__inplace_sub__badalloc default__int32__badalloc
#define default__inplace_mul__badalloc default__int32__badalloc
#define default__inplace_div__badalloc default__int32__badalloc
#define default__inplace_mod__badalloc default__int32__badalloc
#define default__inplace_shl__badalloc default__int32__badalloc
#define default__inplace_shr__badalloc default__int32__badalloc
#define default__inplace_and__badalloc default__int32__badalloc
#define default__inplace_or__badalloc default__int32__badalloc
#define default__inplace_xor__badalloc default__int32__badalloc
#define default__inplace_pow__badalloc default__int32__badalloc
INTDEF int DCALL default__int32__unsupported(DeeObject*, void*);
#define default__int64__unsupported default__int32__unsupported
INTDEF int DCALL default__double__unsupported(DeeObject*, void*);
INTDEF Dee_ssize_t DCALL default__hash__badalloc(void*);
INTDEF int DCALL default__compare_eq__badalloc(void*, void*);
#define default__compare__badalloc default__compare_eq__badalloc
#define default__trycompare_eq__badalloc default__compare_eq__badalloc
INTDEF int DCALL default__compare_eq__unsupported(DeeObject*, void*);
INTDEF int DCALL default__compare__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__eq__badalloc(void*, void*);
#define default__ne__badalloc default__eq__badalloc
#define default__lo__badalloc default__eq__badalloc
#define default__le__badalloc default__eq__badalloc
#define default__gr__badalloc default__eq__badalloc
#define default__ge__badalloc default__eq__badalloc
INTDEF void*DCALL default__eq__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__ne__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__lo__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__le__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__gr__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__ge__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__iter__badalloc(void*);
#define default__sizeob__badalloc default__iter__badalloc
INTDEF void*DCALL default__iter__unsupported(DeeObject*);
INTDEF Dee_ssize_t DCALL default__foreach__badalloc(void*, void*, void*);
#define default__foreach_pair__badalloc default__foreach__badalloc
INTDEF Dee_ssize_t DCALL default__foreach__unsupported(DeeObject*, void*, void*);
#define default__foreach_pair__unsupported default__foreach__unsupported
INTDEF void*DCALL default__sizeob__unsupported(DeeObject*);
INTDEF Dee_ssize_t DCALL default__size__badalloc(void*);
INTDEF Dee_ssize_t DCALL default__size__unsupported(DeeObject*);
INTDEF void*DCALL default__contains__badalloc(void*, void*);
#define default__getitem__badalloc default__contains__badalloc
#define default__trygetitem__badalloc default__contains__badalloc
#define default__getitem_index_fast__badalloc default__contains__badalloc
#define default__getitem_index__badalloc default__contains__badalloc
#define default__trygetitem_index__badalloc default__contains__badalloc
#define default__getrange_index_n__badalloc default__contains__badalloc
INTDEF void*DCALL default__contains__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__getitem__unsupported(DeeObject*, void*);
#define default__trygetitem__unsupported default__getitem__unsupported
#define default__getitem_index__unsupported default__getitem__unsupported
#define default__trygetitem_index__unsupported default__getitem__unsupported
INTDEF void*DCALL default__getitem_string_hash__badalloc(void*, void*, void*);
#define default__trygetitem_string_hash__badalloc default__getitem_string_hash__badalloc
#define default__getrange__badalloc default__getitem_string_hash__badalloc
#define default__getrange_index__badalloc default__getitem_string_hash__badalloc
INTDEF void*DCALL default__getitem_string_hash__unsupported(DeeObject*, void*, void*);
#define default__trygetitem_string_hash__unsupported default__getitem_string_hash__unsupported
INTDEF void*DCALL default__getitem_string_len_hash__badalloc(void*, void*, void*, void*);
#define default__trygetitem_string_len_hash__badalloc default__getitem_string_len_hash__badalloc
INTDEF void*DCALL default__getitem_string_len_hash__unsupported(DeeObject*, void*, void*, void*);
#define default__trygetitem_string_len_hash__unsupported default__getitem_string_len_hash__unsupported
INTDEF int DCALL default__bounditem__badalloc(void*, void*);
#define default__bounditem_index__badalloc default__bounditem__badalloc
#define default__hasitem__badalloc default__bounditem__badalloc
#define default__hasitem_index__badalloc default__bounditem__badalloc
#define default__delitem__badalloc default__bounditem__badalloc
#define default__delitem_index__badalloc default__bounditem__badalloc
#define default__delrange_index_n__badalloc default__bounditem__badalloc
INTDEF int DCALL default__bounditem__unsupported(DeeObject*, void*);
#define default__bounditem_index__unsupported default__bounditem__unsupported
#define default__hasitem__unsupported default__bounditem__unsupported
#define default__hasitem_index__unsupported default__bounditem__unsupported
INTDEF int DCALL default__bounditem_string_hash__badalloc(void*, void*, void*);
#define default__hasitem_string_hash__badalloc default__bounditem_string_hash__badalloc
#define default__delitem_string_hash__badalloc default__bounditem_string_hash__badalloc
#define default__setitem__badalloc default__bounditem_string_hash__badalloc
#define default__setitem_index__badalloc default__bounditem_string_hash__badalloc
#define default__delrange__badalloc default__bounditem_string_hash__badalloc
#define default__delrange_index__badalloc default__bounditem_string_hash__badalloc
#define default__setrange_index_n__badalloc default__bounditem_string_hash__badalloc
INTDEF int DCALL default__bounditem_string_hash__unsupported(DeeObject*, void*, void*);
#define default__hasitem_string_hash__unsupported default__bounditem_string_hash__unsupported
INTDEF int DCALL default__bounditem_string_len_hash__badalloc(void*, void*, void*, void*);
#define default__hasitem_string_len_hash__badalloc default__bounditem_string_len_hash__badalloc
#define default__delitem_string_len_hash__badalloc default__bounditem_string_len_hash__badalloc
#define default__setitem_string_hash__badalloc default__bounditem_string_len_hash__badalloc
#define default__setrange__badalloc default__bounditem_string_len_hash__badalloc
#define default__setrange_index__badalloc default__bounditem_string_len_hash__badalloc
INTDEF int DCALL default__bounditem_string_len_hash__unsupported(DeeObject*, void*, void*, void*);
#define default__hasitem_string_len_hash__unsupported default__bounditem_string_len_hash__unsupported
INTDEF int DCALL default__delitem__unsupported(DeeObject*, void*);
#define default__delitem_index__unsupported default__delitem__unsupported
INTDEF int DCALL default__delitem_string_hash__unsupported(DeeObject*, void*, void*);
INTDEF int DCALL default__delitem_string_len_hash__unsupported(DeeObject*, void*, void*, void*);
INTDEF int DCALL default__setitem__unsupported(DeeObject*, void*, void*);
#define default__setitem_index__unsupported default__setitem__unsupported
INTDEF int DCALL default__setitem_string_hash__unsupported(DeeObject*, void*, void*, void*);
INTDEF int DCALL default__setitem_string_len_hash__badalloc(void*, void*, void*, void*, void*);
INTDEF int DCALL default__setitem_string_len_hash__unsupported(DeeObject*, void*, void*, void*, void*);
INTDEF void*DCALL default__getrange__unsupported(DeeObject*, void*, void*);
#define default__getrange_index__unsupported default__getrange__unsupported
INTDEF void*DCALL default__getrange_index_n__unsupported(DeeObject*, void*);
INTDEF int DCALL default__delrange__unsupported(DeeObject*, void*, void*);
#define default__delrange_index__unsupported default__delrange__unsupported
INTDEF int DCALL default__delrange_index_n__unsupported(DeeObject*, void*);
INTDEF int DCALL default__setrange__unsupported(DeeObject*, void*, void*, void*);
#define default__setrange_index__unsupported default__setrange__unsupported
INTDEF int DCALL default__setrange_index_n__unsupported(DeeObject*, void*, void*);
INTDEF void*DCALL default__inv__unsupported(DeeObject*);
INTDEF void*DCALL default__pos__unsupported(DeeObject*);
INTDEF void*DCALL default__neg__unsupported(DeeObject*);
INTDEF void*DCALL default__add__badalloc(void*, void*);
#define default__sub__badalloc default__add__badalloc
#define default__mul__badalloc default__add__badalloc
#define default__div__badalloc default__add__badalloc
#define default__mod__badalloc default__add__badalloc
#define default__shl__badalloc default__add__badalloc
#define default__shr__badalloc default__add__badalloc
#define default__and__badalloc default__add__badalloc
#define default__or__badalloc default__add__badalloc
#define default__xor__badalloc default__add__badalloc
#define default__pow__badalloc default__add__badalloc
INTDEF void*DCALL default__add__unsupported(DeeObject*, void*);
INTDEF int DCALL default__inplace_add__unsupported(DeeObject**, void*);
INTDEF void*DCALL default__sub__unsupported(DeeObject*, void*);
INTDEF int DCALL default__inplace_sub__unsupported(DeeObject**, void*);
INTDEF void*DCALL default__mul__unsupported(DeeObject*, void*);
INTDEF int DCALL default__inplace_mul__unsupported(DeeObject**, void*);
INTDEF void*DCALL default__div__unsupported(DeeObject*, void*);
INTDEF int DCALL default__inplace_div__unsupported(DeeObject**, void*);
INTDEF void*DCALL default__mod__unsupported(DeeObject*, void*);
INTDEF int DCALL default__inplace_mod__unsupported(DeeObject**, void*);
INTDEF void*DCALL default__shl__unsupported(DeeObject*, void*);
INTDEF int DCALL default__inplace_shl__unsupported(DeeObject**, void*);
INTDEF void*DCALL default__shr__unsupported(DeeObject*, void*);
INTDEF int DCALL default__inplace_shr__unsupported(DeeObject**, void*);
INTDEF void*DCALL default__and__unsupported(DeeObject*, void*);
INTDEF int DCALL default__inplace_and__unsupported(DeeObject**, void*);
INTDEF void*DCALL default__or__unsupported(DeeObject*, void*);
INTDEF int DCALL default__inplace_or__unsupported(DeeObject**, void*);
INTDEF void*DCALL default__xor__unsupported(DeeObject*, void*);
INTDEF int DCALL default__inplace_xor__unsupported(DeeObject**, void*);
INTDEF void*DCALL default__pow__unsupported(DeeObject*, void*);
INTDEF int DCALL default__inplace_pow__unsupported(DeeObject**, void*);
INTDEF int DCALL default__inc__badalloc(void*);
#define default__dec__badalloc default__inc__badalloc
INTDEF int DCALL default__inc__unsupported(DeeObject**);
INTDEF int DCALL default__dec__unsupported(DeeObject**);
INTDEF int DCALL default__enter__badalloc(void*);
#define default__leave__badalloc default__enter__badalloc
INTDEF int DCALL default__enter__unsupported(DeeObject*);
INTDEF int DCALL default__leave__unsupported(DeeObject*);
INTDEF void*DCALL default__getattr__badalloc(void*, void*);
INTDEF void*DCALL default__getattr_string_hash__badalloc(void*, void*, void*);
INTDEF void*DCALL default__getattr_string_len_hash__badalloc(void*, void*, void*, void*);
INTDEF int DCALL default__boundattr__badalloc(void*, void*);
#define default__hasattr__badalloc default__boundattr__badalloc
#define default__delattr__badalloc default__boundattr__badalloc
INTDEF int DCALL default__boundattr_string_hash__badalloc(void*, void*, void*);
#define default__hasattr_string_hash__badalloc default__boundattr_string_hash__badalloc
#define default__delattr_string_hash__badalloc default__boundattr_string_hash__badalloc
#define default__setattr__badalloc default__boundattr_string_hash__badalloc
INTDEF int DCALL default__boundattr_string_len_hash__badalloc(void*, void*, void*, void*);
#define default__hasattr_string_len_hash__badalloc default__boundattr_string_len_hash__badalloc
#define default__delattr_string_len_hash__badalloc default__boundattr_string_len_hash__badalloc
#define default__setattr_string_hash__badalloc default__boundattr_string_len_hash__badalloc
INTDEF int DCALL default__hasattr__unsupported(DeeObject*, void*);
INTDEF int DCALL default__setattr_string_len_hash__badalloc(void*, void*, void*, void*, void*);
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_HINT_ERRORS_H */
