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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_HINT_ERRORS_H
#define GUARD_DEEMON_RUNTIME_OPERATOR_HINT_ERRORS_H 1

#include <deemon/api.h>
#include <deemon/object.h>

/**/
#include "method-hint-defaults.h"

DECL_BEGIN

/* Custom impls, as referenced by:
 * >> [[custom_unsupported_impl_name(default__hash__unsupported)]] */
INTDEF Dee_hash_t DCALL default__hash__unsupported(DeeObject *__restrict self);
#define default__trycompare_eq__unsupported \
	default__seq_operator_trycompare_eq__unsupported

/* clang-format off */
/*[[[deemon (printNativeOperatorHintErrorImpls from "..method-hints.method-hints")(decls: true);]]]*/
INTDEF int DCALL _default__assign__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__move_assign__unsupported(DeeObject*, void*);
INTDEF void*DCALL _default__str__unsupported(DeeObject*);
INTDEF Dee_ssize_t DCALL _default__print__unsupported(DeeObject*, Dee_formatprinter_t, void*);
INTDEF void*DCALL _default__repr__unsupported(DeeObject*);
INTDEF Dee_ssize_t DCALL _default__printrepr__unsupported(DeeObject*, Dee_formatprinter_t, void*);
INTDEF int DCALL _default__bool__unsupported(DeeObject*);
INTDEF void*DCALL _default__call__unsupported(DeeObject*, void*, void*);
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define _default__call_tuple_kw__unsupported _default__call__unsupported
#define _default__thiscall_tuple__unsupported _default__call__unsupported
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
INTDEF void*DCALL default__call_kw__badalloc(void*, void*, void*, void*);
#define default__thiscall__badalloc default__call_kw__badalloc
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define default__thiscall_tuple_kw__badalloc default__call_kw__badalloc
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
INTDEF void*DCALL _default__call_kw__unsupported(DeeObject*, void*, void*, void*);
#define _default__thiscall__unsupported _default__call_kw__unsupported
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define _default__thiscall_tuple_kw__unsupported _default__call_kw__unsupported
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
INTDEF void*DCALL default__thiscall_kw__badalloc(void*, void*, void*, void*, void*);
INTDEF void*DCALL _default__thiscall_kw__unsupported(DeeObject*, void*, void*, void*, void*);
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
INTDEF void*DCALL default__call_tuple__badalloc(void*, void*);
INTDEF void*DCALL _default__call_tuple__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__call_tuple_kw__badalloc(void*, void*, void*);
#define default__thiscall_tuple__badalloc default__call_tuple_kw__badalloc
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
INTDEF void*DCALL _default__iter_next__unsupported(DeeObject*);
#define _default__nextkey__unsupported _default__iter_next__unsupported
#define _default__nextvalue__unsupported _default__iter_next__unsupported
INTDEF int DCALL default__nextpair__badalloc(void*, void*);
INTDEF int DCALL _default__nextpair__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__nextkey__badalloc(void*);
#define default__nextvalue__badalloc default__nextkey__badalloc
INTDEF Dee_ssize_t DCALL default__advance__badalloc(void*, void*);
INTDEF Dee_ssize_t DCALL _default__advance__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__int__badalloc(void*);
#define default__inv__badalloc default__int__badalloc
#define default__pos__badalloc default__int__badalloc
#define default__neg__badalloc default__int__badalloc
INTDEF void*DCALL _default__int__unsupported(DeeObject*);
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
INTDEF int DCALL _default__int32__unsupported(DeeObject*, void*);
#define _default__int64__unsupported _default__int32__unsupported
INTDEF int DCALL _default__double__unsupported(DeeObject*, void*);
INTDEF Dee_ssize_t DCALL default__hash__badalloc(void*);
INTDEF int DCALL default__compare_eq__badalloc(void*, void*);
#define default__compare__badalloc default__compare_eq__badalloc
#define default__trycompare_eq__badalloc default__compare_eq__badalloc
INTDEF int DCALL _default__compare_eq__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__compare__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__eq__badalloc(void*, void*);
#define default__ne__badalloc default__eq__badalloc
#define default__lo__badalloc default__eq__badalloc
#define default__le__badalloc default__eq__badalloc
#define default__gr__badalloc default__eq__badalloc
#define default__ge__badalloc default__eq__badalloc
INTDEF void*DCALL _default__eq__unsupported(DeeObject*, void*);
INTDEF void*DCALL _default__ne__unsupported(DeeObject*, void*);
INTDEF void*DCALL _default__lo__unsupported(DeeObject*, void*);
INTDEF void*DCALL _default__le__unsupported(DeeObject*, void*);
INTDEF void*DCALL _default__gr__unsupported(DeeObject*, void*);
INTDEF void*DCALL _default__ge__unsupported(DeeObject*, void*);
INTDEF void*DCALL default__iter__badalloc(void*);
#define default__sizeob__badalloc default__iter__badalloc
INTDEF void*DCALL _default__iter__unsupported(DeeObject*);
INTDEF Dee_ssize_t DCALL default__foreach__badalloc(void*, void*, void*);
#define default__foreach_pair__badalloc default__foreach__badalloc
INTDEF Dee_ssize_t DCALL _default__foreach__unsupported(DeeObject*, void*, void*);
#define _default__foreach_pair__unsupported _default__foreach__unsupported
INTDEF void*DCALL _default__sizeob__unsupported(DeeObject*);
INTDEF Dee_ssize_t DCALL default__size__badalloc(void*);
INTDEF Dee_ssize_t DCALL _default__size__unsupported(DeeObject*);
INTDEF void*DCALL default__contains__badalloc(void*, void*);
#define default__getitem_index_fast__badalloc default__contains__badalloc
#define default__getitem__badalloc default__contains__badalloc
#define default__getitem_index__badalloc default__contains__badalloc
#define default__trygetitem__badalloc default__contains__badalloc
#define default__trygetitem_index__badalloc default__contains__badalloc
#define default__getrange_index_n__badalloc default__contains__badalloc
INTDEF void*DCALL _default__contains__unsupported(DeeObject*, void*);
INTDEF void*DCALL _default__getitem__unsupported(DeeObject*, void*);
#define _default__getitem_index__unsupported _default__getitem__unsupported
#define _default__trygetitem__unsupported _default__getitem__unsupported
#define _default__trygetitem_index__unsupported _default__getitem__unsupported
INTDEF void*DCALL default__getitem_string_hash__badalloc(void*, void*, void*);
#define default__trygetitem_string_hash__badalloc default__getitem_string_hash__badalloc
#define default__getrange__badalloc default__getitem_string_hash__badalloc
#define default__getrange_index__badalloc default__getitem_string_hash__badalloc
INTDEF void*DCALL _default__getitem_string_hash__unsupported(DeeObject*, void*, void*);
#define _default__trygetitem_string_hash__unsupported _default__getitem_string_hash__unsupported
INTDEF void*DCALL default__getitem_string_len_hash__badalloc(void*, void*, void*, void*);
#define default__trygetitem_string_len_hash__badalloc default__getitem_string_len_hash__badalloc
INTDEF void*DCALL _default__getitem_string_len_hash__unsupported(DeeObject*, void*, void*, void*);
#define _default__trygetitem_string_len_hash__unsupported _default__getitem_string_len_hash__unsupported
INTDEF int DCALL default__bounditem__badalloc(void*, void*);
#define default__bounditem_index__badalloc default__bounditem__badalloc
#define default__hasitem__badalloc default__bounditem__badalloc
#define default__hasitem_index__badalloc default__bounditem__badalloc
#define default__delitem__badalloc default__bounditem__badalloc
#define default__delitem_index__badalloc default__bounditem__badalloc
#define default__delrange_index_n__badalloc default__bounditem__badalloc
INTDEF int DCALL _default__bounditem__unsupported(DeeObject*, void*);
#define _default__bounditem_index__unsupported _default__bounditem__unsupported
#define _default__hasitem__unsupported _default__bounditem__unsupported
#define _default__hasitem_index__unsupported _default__bounditem__unsupported
INTDEF int DCALL default__bounditem_string_hash__badalloc(void*, void*, void*);
#define default__hasitem_string_hash__badalloc default__bounditem_string_hash__badalloc
#define default__delitem_string_hash__badalloc default__bounditem_string_hash__badalloc
#define default__setitem__badalloc default__bounditem_string_hash__badalloc
#define default__setitem_index__badalloc default__bounditem_string_hash__badalloc
#define default__delrange__badalloc default__bounditem_string_hash__badalloc
#define default__delrange_index__badalloc default__bounditem_string_hash__badalloc
#define default__setrange_index_n__badalloc default__bounditem_string_hash__badalloc
INTDEF int DCALL _default__bounditem_string_hash__unsupported(DeeObject*, void*, void*);
#define _default__hasitem_string_hash__unsupported _default__bounditem_string_hash__unsupported
INTDEF int DCALL default__bounditem_string_len_hash__badalloc(void*, void*, void*, void*);
#define default__hasitem_string_len_hash__badalloc default__bounditem_string_len_hash__badalloc
#define default__delitem_string_len_hash__badalloc default__bounditem_string_len_hash__badalloc
#define default__setitem_string_hash__badalloc default__bounditem_string_len_hash__badalloc
#define default__setrange__badalloc default__bounditem_string_len_hash__badalloc
#define default__setrange_index__badalloc default__bounditem_string_len_hash__badalloc
INTDEF int DCALL _default__bounditem_string_len_hash__unsupported(DeeObject*, void*, void*, void*);
#define _default__hasitem_string_len_hash__unsupported _default__bounditem_string_len_hash__unsupported
INTDEF int DCALL _default__delitem__unsupported(DeeObject*, void*);
#define _default__delitem_index__unsupported _default__delitem__unsupported
INTDEF int DCALL _default__delitem_string_hash__unsupported(DeeObject*, void*, void*);
INTDEF int DCALL _default__delitem_string_len_hash__unsupported(DeeObject*, void*, void*, void*);
INTDEF int DCALL _default__setitem__unsupported(DeeObject*, void*, void*);
#define _default__setitem_index__unsupported _default__setitem__unsupported
INTDEF int DCALL _default__setitem_string_hash__unsupported(DeeObject*, void*, void*, void*);
INTDEF int DCALL default__setitem_string_len_hash__badalloc(void*, void*, void*, void*, void*);
INTDEF int DCALL _default__setitem_string_len_hash__unsupported(DeeObject*, void*, void*, void*, void*);
INTDEF void*DCALL _default__getrange__unsupported(DeeObject*, void*, void*);
#define _default__getrange_index__unsupported _default__getrange__unsupported
INTDEF void*DCALL _default__getrange_index_n__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__delrange__unsupported(DeeObject*, void*, void*);
#define _default__delrange_index__unsupported _default__delrange__unsupported
INTDEF int DCALL _default__delrange_index_n__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__setrange__unsupported(DeeObject*, void*, void*, void*);
#define _default__setrange_index__unsupported _default__setrange__unsupported
INTDEF int DCALL _default__setrange_index_n__unsupported(DeeObject*, void*, void*);
INTDEF void*DCALL _default__inv__unsupported(DeeObject*);
INTDEF void*DCALL _default__pos__unsupported(DeeObject*);
INTDEF void*DCALL _default__neg__unsupported(DeeObject*);
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
INTDEF void*DCALL _default__add__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__inplace_add__unsupported(DREF DeeObject**, void*);
INTDEF void*DCALL _default__sub__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__inplace_sub__unsupported(DREF DeeObject**, void*);
INTDEF void*DCALL _default__mul__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__inplace_mul__unsupported(DREF DeeObject**, void*);
INTDEF void*DCALL _default__div__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__inplace_div__unsupported(DREF DeeObject**, void*);
INTDEF void*DCALL _default__mod__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__inplace_mod__unsupported(DREF DeeObject**, void*);
INTDEF void*DCALL _default__shl__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__inplace_shl__unsupported(DREF DeeObject**, void*);
INTDEF void*DCALL _default__shr__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__inplace_shr__unsupported(DREF DeeObject**, void*);
INTDEF void*DCALL _default__and__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__inplace_and__unsupported(DREF DeeObject**, void*);
INTDEF void*DCALL _default__or__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__inplace_or__unsupported(DREF DeeObject**, void*);
INTDEF void*DCALL _default__xor__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__inplace_xor__unsupported(DREF DeeObject**, void*);
INTDEF void*DCALL _default__pow__unsupported(DeeObject*, void*);
INTDEF int DCALL _default__inplace_pow__unsupported(DREF DeeObject**, void*);
INTDEF int DCALL default__inc__badalloc(void*);
#define default__dec__badalloc default__inc__badalloc
INTDEF int DCALL _default__inc__unsupported(DREF DeeObject**);
INTDEF int DCALL _default__dec__unsupported(DREF DeeObject**);
INTDEF int DCALL default__enter__badalloc(void*);
#define default__leave__badalloc default__enter__badalloc
INTDEF int DCALL _default__enter__unsupported(DeeObject*);
INTDEF int DCALL _default__leave__unsupported(DeeObject*);
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
INTDEF int DCALL default__setattr_string_len_hash__badalloc(void*, void*, void*, void*, void*);
#define default__assign__unsupported                     (*(DeeNO_assign_t)&_default__assign__unsupported)
#define default__move_assign__unsupported                (*(DeeNO_move_assign_t)&_default__move_assign__unsupported)
#define default__str__unsupported                        (*(DeeNO_str_t)&_default__str__unsupported)
#define default__print__unsupported                      (*(DeeNO_print_t)&_default__print__unsupported)
#define default__repr__unsupported                       (*(DeeNO_repr_t)&_default__repr__unsupported)
#define default__printrepr__unsupported                  (*(DeeNO_printrepr_t)&_default__printrepr__unsupported)
#define default__bool__unsupported                       (*(DeeNO_bool_t)&_default__bool__unsupported)
#define default__call__unsupported                       (*(DeeNO_call_t)&_default__call__unsupported)
#define default__call_kw__unsupported                    (*(DeeNO_call_kw_t)&_default__call_kw__unsupported)
#define default__thiscall__unsupported                   (*(DeeNO_thiscall_t)&_default__thiscall__unsupported)
#define default__thiscall_kw__unsupported                (*(DeeNO_thiscall_kw_t)&_default__thiscall_kw__unsupported)
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define default__call_tuple__unsupported                 (*(DeeNO_call_tuple_t)&_default__call_tuple__unsupported)
#define default__call_tuple_kw__unsupported              (*(DeeNO_call_tuple_kw_t)&_default__call_tuple_kw__unsupported)
#define default__thiscall_tuple__unsupported             (*(DeeNO_thiscall_tuple_t)&_default__thiscall_tuple__unsupported)
#define default__thiscall_tuple_kw__unsupported          (*(DeeNO_thiscall_tuple_kw_t)&_default__thiscall_tuple_kw__unsupported)
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#define default__iter_next__unsupported                  (*(DeeNO_iter_next_t)&_default__iter_next__unsupported)
#define default__nextpair__unsupported                   (*(DeeNO_nextpair_t)&_default__nextpair__unsupported)
#define default__nextkey__unsupported                    (*(DeeNO_nextkey_t)&_default__nextkey__unsupported)
#define default__nextvalue__unsupported                  (*(DeeNO_nextvalue_t)&_default__nextvalue__unsupported)
#define default__advance__unsupported                    (*(DeeNO_advance_t)&_default__advance__unsupported)
#define default__int__unsupported                        (*(DeeNO_int_t)&_default__int__unsupported)
#define default__int32__unsupported                      (*(DeeNO_int32_t)&_default__int32__unsupported)
#define default__int64__unsupported                      (*(DeeNO_int64_t)&_default__int64__unsupported)
#define default__double__unsupported                     (*(DeeNO_double_t)&_default__double__unsupported)
#define default__compare_eq__unsupported                 (*(DeeNO_compare_eq_t)&_default__compare_eq__unsupported)
#define default__compare__unsupported                    (*(DeeNO_compare_t)&_default__compare__unsupported)
#define default__eq__unsupported                         (*(DeeNO_eq_t)&_default__eq__unsupported)
#define default__ne__unsupported                         (*(DeeNO_ne_t)&_default__ne__unsupported)
#define default__lo__unsupported                         (*(DeeNO_lo_t)&_default__lo__unsupported)
#define default__le__unsupported                         (*(DeeNO_le_t)&_default__le__unsupported)
#define default__gr__unsupported                         (*(DeeNO_gr_t)&_default__gr__unsupported)
#define default__ge__unsupported                         (*(DeeNO_ge_t)&_default__ge__unsupported)
#define default__iter__unsupported                       (*(DeeNO_iter_t)&_default__iter__unsupported)
#define default__foreach__unsupported                    (*(DeeNO_foreach_t)&_default__foreach__unsupported)
#define default__foreach_pair__unsupported               (*(DeeNO_foreach_pair_t)&_default__foreach_pair__unsupported)
#define default__sizeob__unsupported                     (*(DeeNO_sizeob_t)&_default__sizeob__unsupported)
#define default__size__unsupported                       (*(DeeNO_size_t)&_default__size__unsupported)
#define default__contains__unsupported                   (*(DeeNO_contains_t)&_default__contains__unsupported)
#define default__getitem__unsupported                    (*(DeeNO_getitem_t)&_default__getitem__unsupported)
#define default__getitem_index__unsupported              (*(DeeNO_getitem_index_t)&_default__getitem_index__unsupported)
#define default__getitem_string_hash__unsupported        (*(DeeNO_getitem_string_hash_t)&_default__getitem_string_hash__unsupported)
#define default__getitem_string_len_hash__unsupported    (*(DeeNO_getitem_string_len_hash_t)&_default__getitem_string_len_hash__unsupported)
#define default__trygetitem__unsupported                 (*(DeeNO_trygetitem_t)&_default__trygetitem__unsupported)
#define default__trygetitem_index__unsupported           (*(DeeNO_trygetitem_index_t)&_default__trygetitem_index__unsupported)
#define default__trygetitem_string_hash__unsupported     (*(DeeNO_trygetitem_string_hash_t)&_default__trygetitem_string_hash__unsupported)
#define default__trygetitem_string_len_hash__unsupported (*(DeeNO_trygetitem_string_len_hash_t)&_default__trygetitem_string_len_hash__unsupported)
#define default__bounditem__unsupported                  (*(DeeNO_bounditem_t)&_default__bounditem__unsupported)
#define default__bounditem_index__unsupported            (*(DeeNO_bounditem_index_t)&_default__bounditem_index__unsupported)
#define default__bounditem_string_hash__unsupported      (*(DeeNO_bounditem_string_hash_t)&_default__bounditem_string_hash__unsupported)
#define default__bounditem_string_len_hash__unsupported  (*(DeeNO_bounditem_string_len_hash_t)&_default__bounditem_string_len_hash__unsupported)
#define default__hasitem__unsupported                    (*(DeeNO_hasitem_t)&_default__hasitem__unsupported)
#define default__hasitem_index__unsupported              (*(DeeNO_hasitem_index_t)&_default__hasitem_index__unsupported)
#define default__hasitem_string_hash__unsupported        (*(DeeNO_hasitem_string_hash_t)&_default__hasitem_string_hash__unsupported)
#define default__hasitem_string_len_hash__unsupported    (*(DeeNO_hasitem_string_len_hash_t)&_default__hasitem_string_len_hash__unsupported)
#define default__delitem__unsupported                    (*(DeeNO_delitem_t)&_default__delitem__unsupported)
#define default__delitem_index__unsupported              (*(DeeNO_delitem_index_t)&_default__delitem_index__unsupported)
#define default__delitem_string_hash__unsupported        (*(DeeNO_delitem_string_hash_t)&_default__delitem_string_hash__unsupported)
#define default__delitem_string_len_hash__unsupported    (*(DeeNO_delitem_string_len_hash_t)&_default__delitem_string_len_hash__unsupported)
#define default__setitem__unsupported                    (*(DeeNO_setitem_t)&_default__setitem__unsupported)
#define default__setitem_index__unsupported              (*(DeeNO_setitem_index_t)&_default__setitem_index__unsupported)
#define default__setitem_string_hash__unsupported        (*(DeeNO_setitem_string_hash_t)&_default__setitem_string_hash__unsupported)
#define default__setitem_string_len_hash__unsupported    (*(DeeNO_setitem_string_len_hash_t)&_default__setitem_string_len_hash__unsupported)
#define default__getrange__unsupported                   (*(DeeNO_getrange_t)&_default__getrange__unsupported)
#define default__getrange_index__unsupported             (*(DeeNO_getrange_index_t)&_default__getrange_index__unsupported)
#define default__getrange_index_n__unsupported           (*(DeeNO_getrange_index_n_t)&_default__getrange_index_n__unsupported)
#define default__delrange__unsupported                   (*(DeeNO_delrange_t)&_default__delrange__unsupported)
#define default__delrange_index__unsupported             (*(DeeNO_delrange_index_t)&_default__delrange_index__unsupported)
#define default__delrange_index_n__unsupported           (*(DeeNO_delrange_index_n_t)&_default__delrange_index_n__unsupported)
#define default__setrange__unsupported                   (*(DeeNO_setrange_t)&_default__setrange__unsupported)
#define default__setrange_index__unsupported             (*(DeeNO_setrange_index_t)&_default__setrange_index__unsupported)
#define default__setrange_index_n__unsupported           (*(DeeNO_setrange_index_n_t)&_default__setrange_index_n__unsupported)
#define default__inv__unsupported                        (*(DeeNO_inv_t)&_default__inv__unsupported)
#define default__pos__unsupported                        (*(DeeNO_pos_t)&_default__pos__unsupported)
#define default__neg__unsupported                        (*(DeeNO_neg_t)&_default__neg__unsupported)
#define default__add__unsupported                        (*(DeeNO_add_t)&_default__add__unsupported)
#define default__inplace_add__unsupported                (*(DeeNO_inplace_add_t)&_default__inplace_add__unsupported)
#define default__sub__unsupported                        (*(DeeNO_sub_t)&_default__sub__unsupported)
#define default__inplace_sub__unsupported                (*(DeeNO_inplace_sub_t)&_default__inplace_sub__unsupported)
#define default__mul__unsupported                        (*(DeeNO_mul_t)&_default__mul__unsupported)
#define default__inplace_mul__unsupported                (*(DeeNO_inplace_mul_t)&_default__inplace_mul__unsupported)
#define default__div__unsupported                        (*(DeeNO_div_t)&_default__div__unsupported)
#define default__inplace_div__unsupported                (*(DeeNO_inplace_div_t)&_default__inplace_div__unsupported)
#define default__mod__unsupported                        (*(DeeNO_mod_t)&_default__mod__unsupported)
#define default__inplace_mod__unsupported                (*(DeeNO_inplace_mod_t)&_default__inplace_mod__unsupported)
#define default__shl__unsupported                        (*(DeeNO_shl_t)&_default__shl__unsupported)
#define default__inplace_shl__unsupported                (*(DeeNO_inplace_shl_t)&_default__inplace_shl__unsupported)
#define default__shr__unsupported                        (*(DeeNO_shr_t)&_default__shr__unsupported)
#define default__inplace_shr__unsupported                (*(DeeNO_inplace_shr_t)&_default__inplace_shr__unsupported)
#define default__and__unsupported                        (*(DeeNO_and_t)&_default__and__unsupported)
#define default__inplace_and__unsupported                (*(DeeNO_inplace_and_t)&_default__inplace_and__unsupported)
#define default__or__unsupported                         (*(DeeNO_or_t)&_default__or__unsupported)
#define default__inplace_or__unsupported                 (*(DeeNO_inplace_or_t)&_default__inplace_or__unsupported)
#define default__xor__unsupported                        (*(DeeNO_xor_t)&_default__xor__unsupported)
#define default__inplace_xor__unsupported                (*(DeeNO_inplace_xor_t)&_default__inplace_xor__unsupported)
#define default__pow__unsupported                        (*(DeeNO_pow_t)&_default__pow__unsupported)
#define default__inplace_pow__unsupported                (*(DeeNO_inplace_pow_t)&_default__inplace_pow__unsupported)
#define default__inc__unsupported                        (*(DeeNO_inc_t)&_default__inc__unsupported)
#define default__dec__unsupported                        (*(DeeNO_dec_t)&_default__dec__unsupported)
#define default__enter__unsupported                      (*(DeeNO_enter_t)&_default__enter__unsupported)
#define default__leave__unsupported                      (*(DeeNO_leave_t)&_default__leave__unsupported)
/*[[[end]]]*/
/* clang-format on */

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_HINT_ERRORS_H */
