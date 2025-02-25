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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_HINT_ERRORS_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_HINT_ERRORS_C 1

#include <deemon/api.h>
#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/alloc.h>
#include <deemon/operator-hints.h>

/**/
#include "method-hint-defaults.h"
#include "operator-hint-errors.h"
#include "runtime_error.h"

DECL_BEGIN

/* Custom impls, as referenced by:
 * >> [[custom_unsupported_impl_name(default__hash__unsupported)]] */
INTERN Dee_hash_t DCALL
default__hash__unsupported(DeeObject *__restrict self) {
	return DeeObject_HashGeneric(self);
}

/* clang-format off */
/*[[[deemon (printNativeOperatorHintErrorImpls from "..method-hints.method-hints")(decls: false);]]]*/
INTERN int DCALL default__assign__unsupported(DeeObject*self, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ASSIGN);
}
INTERN int DCALL default__move_assign__unsupported(DeeObject*self, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_MOVEASSIGN);
}
INTERN void*DCALL default__str__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_STR);
	return NULL;
}
INTERN Dee_ssize_t DCALL default__print__unsupported(DeeObject*self, Dee_formatprinter_t, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_STR);
}
INTERN void*DCALL default__repr__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_REPR);
	return NULL;
}
INTERN Dee_ssize_t DCALL default__printrepr__unsupported(DeeObject*self, Dee_formatprinter_t, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_REPR);
}
INTERN int DCALL default__bool__unsupported(DeeObject*self) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_BOOL);
}
INTERN void*DCALL default__call__unsupported(DeeObject*self, void*, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_CALL);
	return NULL;
}
INTERN void*DCALL default__call_kw__unsupported(DeeObject*self, void*, void*, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_CALL);
	return NULL;
}
INTERN void*DCALL default__iter_next__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITERNEXT);
	return NULL;
}
INTERN int DCALL default__nextpair__badalloc(void*UNUSED(self), void*) {
	return Dee_BadAlloc(sizeof(struct type_iterator));
}
INTERN int DCALL default__nextpair__unsupported(DeeObject*self, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITERNEXT);
}
INTERN void*DCALL default__nextkey__badalloc(void*UNUSED(self)) {
	Dee_BadAlloc(sizeof(struct type_iterator));
	return NULL;
}
INTERN Dee_ssize_t DCALL default__advance__badalloc(void*UNUSED(self), void*) {
	return Dee_BadAlloc(sizeof(struct type_iterator));
}
INTERN Dee_ssize_t DCALL default__advance__unsupported(DeeObject*self, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITERNEXT);
}
INTERN void*DCALL default__int__badalloc(void*UNUSED(self)) {
	Dee_BadAlloc(sizeof(struct type_math));
	return NULL;
}
INTERN void*DCALL default__int__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_INT);
	return NULL;
}
INTERN int DCALL default__int32__badalloc(void*UNUSED(self), void*) {
	return Dee_BadAlloc(sizeof(struct type_math));
}
INTERN int DCALL default__int32__unsupported(DeeObject*self, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_INT);
}
INTERN int DCALL default__double__unsupported(DeeObject*self, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_FLOAT);
}
INTERN Dee_ssize_t DCALL default__hash__badalloc(void*UNUSED(self)) {
	return Dee_BadAlloc(sizeof(struct type_cmp));
}
INTERN int DCALL default__compare_eq__badalloc(void*UNUSED(self), void*) {
	return Dee_BadAlloc(sizeof(struct type_cmp));
}
INTERN int DCALL default__compare_eq__unsupported(DeeObject*self, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_EQ);
}
INTERN int DCALL default__compare__unsupported(DeeObject*self, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_LO);
}
INTERN void*DCALL default__eq__badalloc(void*UNUSED(self), void*) {
	Dee_BadAlloc(sizeof(struct type_cmp));
	return NULL;
}
INTERN void*DCALL default__eq__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_EQ);
	return NULL;
}
INTERN void*DCALL default__ne__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_NE);
	return NULL;
}
INTERN void*DCALL default__lo__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_LO);
	return NULL;
}
INTERN void*DCALL default__le__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_LE);
	return NULL;
}
INTERN void*DCALL default__gr__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GR);
	return NULL;
}
INTERN void*DCALL default__ge__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GE);
	return NULL;
}
INTERN void*DCALL default__iter__badalloc(void*UNUSED(self)) {
	Dee_BadAlloc(sizeof(struct type_seq));
	return NULL;
}
INTERN void*DCALL default__iter__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
	return NULL;
}
INTERN Dee_ssize_t DCALL default__foreach__badalloc(void*UNUSED(self), void*, void*) {
	return Dee_BadAlloc(sizeof(struct type_seq));
}
INTERN Dee_ssize_t DCALL default__foreach__unsupported(DeeObject*self, void*, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
}
INTERN void*DCALL default__sizeob__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SIZE);
	return NULL;
}
INTERN Dee_ssize_t DCALL default__size__badalloc(void*UNUSED(self)) {
	return Dee_BadAlloc(sizeof(struct type_seq));
}
INTERN Dee_ssize_t DCALL default__size__unsupported(DeeObject*self) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SIZE);
}
INTERN void*DCALL default__contains__badalloc(void*UNUSED(self), void*) {
	Dee_BadAlloc(sizeof(struct type_seq));
	return NULL;
}
INTERN void*DCALL default__contains__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_CONTAINS);
	return NULL;
}
INTERN void*DCALL default__getitem__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
	return NULL;
}
INTERN void*DCALL default__getitem_string_hash__badalloc(void*UNUSED(self), void*, void*) {
	Dee_BadAlloc(sizeof(struct type_seq));
	return NULL;
}
INTERN void*DCALL default__getitem_string_hash__unsupported(DeeObject*self, void*, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
	return NULL;
}
INTERN void*DCALL default__getitem_string_len_hash__badalloc(void*UNUSED(self), void*, void*, void*) {
	Dee_BadAlloc(sizeof(struct type_seq));
	return NULL;
}
INTERN void*DCALL default__getitem_string_len_hash__unsupported(DeeObject*self, void*, void*, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
	return NULL;
}
INTERN int DCALL default__bounditem__badalloc(void*UNUSED(self), void*) {
	return Dee_BadAlloc(sizeof(struct type_seq));
}
INTERN int DCALL default__bounditem__unsupported(DeeObject*self, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
}
INTERN int DCALL default__bounditem_string_hash__badalloc(void*UNUSED(self), void*, void*) {
	return Dee_BadAlloc(sizeof(struct type_seq));
}
INTERN int DCALL default__bounditem_string_hash__unsupported(DeeObject*self, void*, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
}
INTERN int DCALL default__bounditem_string_len_hash__badalloc(void*UNUSED(self), void*, void*, void*) {
	return Dee_BadAlloc(sizeof(struct type_seq));
}
INTERN int DCALL default__bounditem_string_len_hash__unsupported(DeeObject*self, void*, void*, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
}
INTERN int DCALL default__delitem__unsupported(DeeObject*self, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_DELITEM);
}
INTERN int DCALL default__delitem_string_hash__unsupported(DeeObject*self, void*, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_DELITEM);
}
INTERN int DCALL default__delitem_string_len_hash__unsupported(DeeObject*self, void*, void*, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_DELITEM);
}
INTERN int DCALL default__setitem__unsupported(DeeObject*self, void*, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SETITEM);
}
INTERN int DCALL default__setitem_string_hash__unsupported(DeeObject*self, void*, void*, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SETITEM);
}
INTERN int DCALL default__setitem_string_len_hash__badalloc(void*UNUSED(self), void*, void*, void*, void*) {
	return Dee_BadAlloc(sizeof(struct type_seq));
}
INTERN int DCALL default__setitem_string_len_hash__unsupported(DeeObject*self, void*, void*, void*, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SETITEM);
}
INTERN void*DCALL default__getrange__unsupported(DeeObject*self, void*, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETRANGE);
	return NULL;
}
INTERN void*DCALL default__getrange_index_n__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETRANGE);
	return NULL;
}
INTERN int DCALL default__delrange__unsupported(DeeObject*self, void*, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_DELRANGE);
}
INTERN int DCALL default__delrange_index_n__unsupported(DeeObject*self, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_DELRANGE);
}
INTERN int DCALL default__setrange__unsupported(DeeObject*self, void*, void*, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SETRANGE);
}
INTERN int DCALL default__setrange_index_n__unsupported(DeeObject*self, void*, void*) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SETRANGE);
}
INTERN void*DCALL default__inv__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_INV);
	return NULL;
}
INTERN void*DCALL default__pos__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_POS);
	return NULL;
}
INTERN void*DCALL default__neg__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_NEG);
	return NULL;
}
INTERN void*DCALL default__add__badalloc(void*UNUSED(self), void*) {
	Dee_BadAlloc(sizeof(struct type_math));
	return NULL;
}
INTERN void*DCALL default__add__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ADD);
	return NULL;
}
INTERN int DCALL default__inplace_add__unsupported(DREF DeeObject**self, void*) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_ADD);
}
INTERN void*DCALL default__sub__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SUB);
	return NULL;
}
INTERN int DCALL default__inplace_sub__unsupported(DREF DeeObject**self, void*) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_SUB);
}
INTERN void*DCALL default__mul__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_MUL);
	return NULL;
}
INTERN int DCALL default__inplace_mul__unsupported(DREF DeeObject**self, void*) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_MUL);
}
INTERN void*DCALL default__div__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_DIV);
	return NULL;
}
INTERN int DCALL default__inplace_div__unsupported(DREF DeeObject**self, void*) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_DIV);
}
INTERN void*DCALL default__mod__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_MOD);
	return NULL;
}
INTERN int DCALL default__inplace_mod__unsupported(DREF DeeObject**self, void*) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_MOD);
}
INTERN void*DCALL default__shl__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SHL);
	return NULL;
}
INTERN int DCALL default__inplace_shl__unsupported(DREF DeeObject**self, void*) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_SHL);
}
INTERN void*DCALL default__shr__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SHR);
	return NULL;
}
INTERN int DCALL default__inplace_shr__unsupported(DREF DeeObject**self, void*) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_SHR);
}
INTERN void*DCALL default__and__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_AND);
	return NULL;
}
INTERN int DCALL default__inplace_and__unsupported(DREF DeeObject**self, void*) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_AND);
}
INTERN void*DCALL default__or__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_OR);
	return NULL;
}
INTERN int DCALL default__inplace_or__unsupported(DREF DeeObject**self, void*) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_OR);
}
INTERN void*DCALL default__xor__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_XOR);
	return NULL;
}
INTERN int DCALL default__inplace_xor__unsupported(DREF DeeObject**self, void*) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_XOR);
}
INTERN void*DCALL default__pow__unsupported(DeeObject*self, void*) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_POW);
	return NULL;
}
INTERN int DCALL default__inplace_pow__unsupported(DREF DeeObject**self, void*) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_POW);
}
INTERN int DCALL default__inc__badalloc(void*UNUSED(self)) {
	return Dee_BadAlloc(sizeof(struct type_math));
}
INTERN int DCALL default__inc__unsupported(DREF DeeObject**self) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INC);
}
INTERN int DCALL default__dec__unsupported(DREF DeeObject**self) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_DEC);
}
INTERN int DCALL default__enter__badalloc(void*UNUSED(self)) {
	return Dee_BadAlloc(sizeof(struct type_with));
}
INTERN int DCALL default__enter__unsupported(DeeObject*self) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ENTER);
}
INTERN int DCALL default__leave__unsupported(DeeObject*self) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_LEAVE);
}
INTERN void*DCALL default__getattr__badalloc(void*UNUSED(self), void*) {
	Dee_BadAlloc(sizeof(struct type_attr));
	return NULL;
}
INTERN void*DCALL default__getattr_string_hash__badalloc(void*UNUSED(self), void*, void*) {
	Dee_BadAlloc(sizeof(struct type_attr));
	return NULL;
}
INTERN void*DCALL default__getattr_string_len_hash__badalloc(void*UNUSED(self), void*, void*, void*) {
	Dee_BadAlloc(sizeof(struct type_attr));
	return NULL;
}
INTERN int DCALL default__boundattr__badalloc(void*UNUSED(self), void*) {
	return Dee_BadAlloc(sizeof(struct type_attr));
}
INTERN int DCALL default__boundattr_string_hash__badalloc(void*UNUSED(self), void*, void*) {
	return Dee_BadAlloc(sizeof(struct type_attr));
}
INTERN int DCALL default__boundattr_string_len_hash__badalloc(void*UNUSED(self), void*, void*, void*) {
	return Dee_BadAlloc(sizeof(struct type_attr));
}
INTERN int DCALL default__setattr_string_len_hash__badalloc(void*UNUSED(self), void*, void*, void*, void*) {
	return Dee_BadAlloc(sizeof(struct type_attr));
}
/*[[[end]]]*/
/* clang-format on */


/* Returns a special impl for "id" returned by:
 * - DeeType_GetNativeOperatorWithoutHints()
 * - DeeType_GetNativeOperatorWithoutInherit()
 * - DeeType_GetNativeOperatorWithoutUnsupported()
 * when it failed to allocate a necessary operator table. These impls behave
 * similar to `DeeType_GetNativeOperatorUnsupported()', except that rather
 * than calling `err_unimplemented_operator()', these call:
 * >> Dee_BadAlloc(type_tno_sizeof_table(oh_init_specs[id].ohis_table));
 * where "id" is the same as the given "id"
 *
 * Returns "NULL" if "id" isn't part of a sub-table. */
INTERN_TPCONST Dee_funptr_t tpconst
_DeeType_GetNativeOperatorOOM[Dee_TNO_COUNT] = {
	/* clang-format off */
/*[[[deemon (printNativeOperatorHintBadAllocTable from "..method-hints.method-hints")();]]]*/
	/* assign                     */ NULL,
	/* move_assign                */ NULL,
	/* str                        */ NULL,
	/* print                      */ NULL,
	/* repr                       */ NULL,
	/* printrepr                  */ NULL,
	/* bool                       */ NULL,
	/* call                       */ NULL,
	/* call_kw                    */ NULL,
	/* iter_next                  */ NULL,
	/* nextpair                   */ (Dee_funptr_t)&default__nextpair__badalloc,
	/* nextkey                    */ (Dee_funptr_t)&default__nextkey__badalloc,
	/* nextvalue                  */ (Dee_funptr_t)&default__nextvalue__badalloc,
	/* advance                    */ (Dee_funptr_t)&default__advance__badalloc,
	/* int                        */ (Dee_funptr_t)&default__int__badalloc,
	/* int32                      */ (Dee_funptr_t)&default__int32__badalloc,
	/* int64                      */ (Dee_funptr_t)&default__int64__badalloc,
	/* double                     */ (Dee_funptr_t)&default__double__badalloc,
	/* hash                       */ (Dee_funptr_t)&default__hash__badalloc,
	/* compare_eq                 */ (Dee_funptr_t)&default__compare_eq__badalloc,
	/* compare                    */ (Dee_funptr_t)&default__compare__badalloc,
	/* trycompare_eq              */ (Dee_funptr_t)&default__trycompare_eq__badalloc,
	/* eq                         */ (Dee_funptr_t)&default__eq__badalloc,
	/* ne                         */ (Dee_funptr_t)&default__ne__badalloc,
	/* lo                         */ (Dee_funptr_t)&default__lo__badalloc,
	/* le                         */ (Dee_funptr_t)&default__le__badalloc,
	/* gr                         */ (Dee_funptr_t)&default__gr__badalloc,
	/* ge                         */ (Dee_funptr_t)&default__ge__badalloc,
	/* iter                       */ (Dee_funptr_t)&default__iter__badalloc,
	/* foreach                    */ (Dee_funptr_t)&default__foreach__badalloc,
	/* foreach_pair               */ (Dee_funptr_t)&default__foreach_pair__badalloc,
	/* sizeob                     */ (Dee_funptr_t)&default__sizeob__badalloc,
	/* size                       */ (Dee_funptr_t)&default__size__badalloc,
	/* size_fast                  */ (Dee_funptr_t)&default__size_fast__with__,
	/* contains                   */ (Dee_funptr_t)&default__contains__badalloc,
	/* getitem_index_fast         */ (Dee_funptr_t)&default__getitem_index_fast__badalloc,
	/* getitem                    */ (Dee_funptr_t)&default__getitem__badalloc,
	/* getitem_index              */ (Dee_funptr_t)&default__getitem_index__badalloc,
	/* getitem_string_hash        */ (Dee_funptr_t)&default__getitem_string_hash__badalloc,
	/* getitem_string_len_hash    */ (Dee_funptr_t)&default__getitem_string_len_hash__badalloc,
	/* trygetitem                 */ (Dee_funptr_t)&default__trygetitem__badalloc,
	/* trygetitem_index           */ (Dee_funptr_t)&default__trygetitem_index__badalloc,
	/* trygetitem_string_hash     */ (Dee_funptr_t)&default__trygetitem_string_hash__badalloc,
	/* trygetitem_string_len_hash */ (Dee_funptr_t)&default__trygetitem_string_len_hash__badalloc,
	/* bounditem                  */ (Dee_funptr_t)&default__bounditem__badalloc,
	/* bounditem_index            */ (Dee_funptr_t)&default__bounditem_index__badalloc,
	/* bounditem_string_hash      */ (Dee_funptr_t)&default__bounditem_string_hash__badalloc,
	/* bounditem_string_len_hash  */ (Dee_funptr_t)&default__bounditem_string_len_hash__badalloc,
	/* hasitem                    */ (Dee_funptr_t)&default__hasitem__badalloc,
	/* hasitem_index              */ (Dee_funptr_t)&default__hasitem_index__badalloc,
	/* hasitem_string_hash        */ (Dee_funptr_t)&default__hasitem_string_hash__badalloc,
	/* hasitem_string_len_hash    */ (Dee_funptr_t)&default__hasitem_string_len_hash__badalloc,
	/* delitem                    */ (Dee_funptr_t)&default__delitem__badalloc,
	/* delitem_index              */ (Dee_funptr_t)&default__delitem_index__badalloc,
	/* delitem_string_hash        */ (Dee_funptr_t)&default__delitem_string_hash__badalloc,
	/* delitem_string_len_hash    */ (Dee_funptr_t)&default__delitem_string_len_hash__badalloc,
	/* setitem                    */ (Dee_funptr_t)&default__setitem__badalloc,
	/* setitem_index              */ (Dee_funptr_t)&default__setitem_index__badalloc,
	/* setitem_string_hash        */ (Dee_funptr_t)&default__setitem_string_hash__badalloc,
	/* setitem_string_len_hash    */ (Dee_funptr_t)&default__setitem_string_len_hash__badalloc,
	/* getrange                   */ (Dee_funptr_t)&default__getrange__badalloc,
	/* getrange_index             */ (Dee_funptr_t)&default__getrange_index__badalloc,
	/* getrange_index_n           */ (Dee_funptr_t)&default__getrange_index_n__badalloc,
	/* delrange                   */ (Dee_funptr_t)&default__delrange__badalloc,
	/* delrange_index             */ (Dee_funptr_t)&default__delrange_index__badalloc,
	/* delrange_index_n           */ (Dee_funptr_t)&default__delrange_index_n__badalloc,
	/* setrange                   */ (Dee_funptr_t)&default__setrange__badalloc,
	/* setrange_index             */ (Dee_funptr_t)&default__setrange_index__badalloc,
	/* setrange_index_n           */ (Dee_funptr_t)&default__setrange_index_n__badalloc,
	/* inv                        */ (Dee_funptr_t)&default__inv__badalloc,
	/* pos                        */ (Dee_funptr_t)&default__pos__badalloc,
	/* neg                        */ (Dee_funptr_t)&default__neg__badalloc,
	/* add                        */ (Dee_funptr_t)&default__add__badalloc,
	/* inplace_add                */ (Dee_funptr_t)&default__inplace_add__badalloc,
	/* sub                        */ (Dee_funptr_t)&default__sub__badalloc,
	/* inplace_sub                */ (Dee_funptr_t)&default__inplace_sub__badalloc,
	/* mul                        */ (Dee_funptr_t)&default__mul__badalloc,
	/* inplace_mul                */ (Dee_funptr_t)&default__inplace_mul__badalloc,
	/* div                        */ (Dee_funptr_t)&default__div__badalloc,
	/* inplace_div                */ (Dee_funptr_t)&default__inplace_div__badalloc,
	/* mod                        */ (Dee_funptr_t)&default__mod__badalloc,
	/* inplace_mod                */ (Dee_funptr_t)&default__inplace_mod__badalloc,
	/* shl                        */ (Dee_funptr_t)&default__shl__badalloc,
	/* inplace_shl                */ (Dee_funptr_t)&default__inplace_shl__badalloc,
	/* shr                        */ (Dee_funptr_t)&default__shr__badalloc,
	/* inplace_shr                */ (Dee_funptr_t)&default__inplace_shr__badalloc,
	/* and                        */ (Dee_funptr_t)&default__and__badalloc,
	/* inplace_and                */ (Dee_funptr_t)&default__inplace_and__badalloc,
	/* or                         */ (Dee_funptr_t)&default__or__badalloc,
	/* inplace_or                 */ (Dee_funptr_t)&default__inplace_or__badalloc,
	/* xor                        */ (Dee_funptr_t)&default__xor__badalloc,
	/* inplace_xor                */ (Dee_funptr_t)&default__inplace_xor__badalloc,
	/* pow                        */ (Dee_funptr_t)&default__pow__badalloc,
	/* inplace_pow                */ (Dee_funptr_t)&default__inplace_pow__badalloc,
	/* inc                        */ (Dee_funptr_t)&default__inc__badalloc,
	/* dec                        */ (Dee_funptr_t)&default__dec__badalloc,
	/* enter                      */ (Dee_funptr_t)&default__enter__badalloc,
	/* leave                      */ (Dee_funptr_t)&default__leave__badalloc,
	/* getattr                    */ (Dee_funptr_t)&default__getattr__badalloc,
	/* getattr_string_hash        */ (Dee_funptr_t)&default__getattr_string_hash__badalloc,
	/* getattr_string_len_hash    */ (Dee_funptr_t)&default__getattr_string_len_hash__badalloc,
	/* boundattr                  */ (Dee_funptr_t)&default__boundattr__badalloc,
	/* boundattr_string_hash      */ (Dee_funptr_t)&default__boundattr_string_hash__badalloc,
	/* boundattr_string_len_hash  */ (Dee_funptr_t)&default__boundattr_string_len_hash__badalloc,
	/* hasattr                    */ (Dee_funptr_t)&default__hasattr__badalloc,
	/* hasattr_string_hash        */ (Dee_funptr_t)&default__hasattr_string_hash__badalloc,
	/* hasattr_string_len_hash    */ (Dee_funptr_t)&default__hasattr_string_len_hash__badalloc,
	/* delattr                    */ (Dee_funptr_t)&default__delattr__badalloc,
	/* delattr_string_hash        */ (Dee_funptr_t)&default__delattr_string_hash__badalloc,
	/* delattr_string_len_hash    */ (Dee_funptr_t)&default__delattr_string_len_hash__badalloc,
	/* setattr                    */ (Dee_funptr_t)&default__setattr__badalloc,
	/* setattr_string_hash        */ (Dee_funptr_t)&default__setattr_string_hash__badalloc,
	/* setattr_string_len_hash    */ (Dee_funptr_t)&default__setattr_string_len_hash__badalloc,
/*[[[end]]]*/
	/* clang-format on */
};


/* Returns the impl for "id" that calls `err_unimplemented_operator()'.
 * Returns "NULL" if "id" doesn't define a user-code ID */
INTERN_TPCONST Dee_funptr_t tpconst
_DeeType_GetNativeOperatorUnsupported[Dee_TNO_COUNT] = {
	/* clang-format off */
/*[[[deemon (printNativeOperatorHintUnsupportedTable from "..method-hints.method-hints")();]]]*/
	/* assign                     */ (Dee_funptr_t)&default__assign__unsupported,
	/* move_assign                */ (Dee_funptr_t)&default__move_assign__unsupported,
	/* str                        */ (Dee_funptr_t)&default__str__unsupported,
	/* print                      */ (Dee_funptr_t)&default__print__unsupported,
	/* repr                       */ (Dee_funptr_t)&default__repr__unsupported,
	/* printrepr                  */ (Dee_funptr_t)&default__printrepr__unsupported,
	/* bool                       */ (Dee_funptr_t)&default__bool__unsupported,
	/* call                       */ (Dee_funptr_t)&default__call__unsupported,
	/* call_kw                    */ (Dee_funptr_t)&default__call_kw__unsupported,
	/* iter_next                  */ (Dee_funptr_t)&default__iter_next__unsupported,
	/* nextpair                   */ (Dee_funptr_t)&default__nextpair__unsupported,
	/* nextkey                    */ (Dee_funptr_t)&default__nextkey__unsupported,
	/* nextvalue                  */ (Dee_funptr_t)&default__nextvalue__unsupported,
	/* advance                    */ (Dee_funptr_t)&default__advance__unsupported,
	/* int                        */ (Dee_funptr_t)&default__int__unsupported,
	/* int32                      */ (Dee_funptr_t)&default__int32__unsupported,
	/* int64                      */ (Dee_funptr_t)&default__int64__unsupported,
	/* double                     */ (Dee_funptr_t)&default__double__unsupported,
	/* hash                       */ (Dee_funptr_t)&default__hash__unsupported,
	/* compare_eq                 */ (Dee_funptr_t)&default__compare_eq__unsupported,
	/* compare                    */ (Dee_funptr_t)&default__compare__unsupported,
	/* trycompare_eq              */ (Dee_funptr_t)&default__trycompare_eq__unsupported,
	/* eq                         */ (Dee_funptr_t)&default__eq__unsupported,
	/* ne                         */ (Dee_funptr_t)&default__ne__unsupported,
	/* lo                         */ (Dee_funptr_t)&default__lo__unsupported,
	/* le                         */ (Dee_funptr_t)&default__le__unsupported,
	/* gr                         */ (Dee_funptr_t)&default__gr__unsupported,
	/* ge                         */ (Dee_funptr_t)&default__ge__unsupported,
	/* iter                       */ (Dee_funptr_t)&default__iter__unsupported,
	/* foreach                    */ (Dee_funptr_t)&default__foreach__unsupported,
	/* foreach_pair               */ (Dee_funptr_t)&default__foreach_pair__unsupported,
	/* sizeob                     */ (Dee_funptr_t)&default__sizeob__unsupported,
	/* size                       */ (Dee_funptr_t)&default__size__unsupported,
	/* size_fast                  */ (Dee_funptr_t)&default__size_fast__with__,
	/* contains                   */ (Dee_funptr_t)&default__contains__unsupported,
	/* getitem_index_fast         */ NULL,
	/* getitem                    */ (Dee_funptr_t)&default__getitem__unsupported,
	/* getitem_index              */ (Dee_funptr_t)&default__getitem_index__unsupported,
	/* getitem_string_hash        */ (Dee_funptr_t)&default__getitem_string_hash__unsupported,
	/* getitem_string_len_hash    */ (Dee_funptr_t)&default__getitem_string_len_hash__unsupported,
	/* trygetitem                 */ (Dee_funptr_t)&default__trygetitem__unsupported,
	/* trygetitem_index           */ (Dee_funptr_t)&default__trygetitem_index__unsupported,
	/* trygetitem_string_hash     */ (Dee_funptr_t)&default__trygetitem_string_hash__unsupported,
	/* trygetitem_string_len_hash */ (Dee_funptr_t)&default__trygetitem_string_len_hash__unsupported,
	/* bounditem                  */ (Dee_funptr_t)&default__bounditem__unsupported,
	/* bounditem_index            */ (Dee_funptr_t)&default__bounditem_index__unsupported,
	/* bounditem_string_hash      */ (Dee_funptr_t)&default__bounditem_string_hash__unsupported,
	/* bounditem_string_len_hash  */ (Dee_funptr_t)&default__bounditem_string_len_hash__unsupported,
	/* hasitem                    */ (Dee_funptr_t)&default__hasitem__unsupported,
	/* hasitem_index              */ (Dee_funptr_t)&default__hasitem_index__unsupported,
	/* hasitem_string_hash        */ (Dee_funptr_t)&default__hasitem_string_hash__unsupported,
	/* hasitem_string_len_hash    */ (Dee_funptr_t)&default__hasitem_string_len_hash__unsupported,
	/* delitem                    */ (Dee_funptr_t)&default__delitem__unsupported,
	/* delitem_index              */ (Dee_funptr_t)&default__delitem_index__unsupported,
	/* delitem_string_hash        */ (Dee_funptr_t)&default__delitem_string_hash__unsupported,
	/* delitem_string_len_hash    */ (Dee_funptr_t)&default__delitem_string_len_hash__unsupported,
	/* setitem                    */ (Dee_funptr_t)&default__setitem__unsupported,
	/* setitem_index              */ (Dee_funptr_t)&default__setitem_index__unsupported,
	/* setitem_string_hash        */ (Dee_funptr_t)&default__setitem_string_hash__unsupported,
	/* setitem_string_len_hash    */ (Dee_funptr_t)&default__setitem_string_len_hash__unsupported,
	/* getrange                   */ (Dee_funptr_t)&default__getrange__unsupported,
	/* getrange_index             */ (Dee_funptr_t)&default__getrange_index__unsupported,
	/* getrange_index_n           */ (Dee_funptr_t)&default__getrange_index_n__unsupported,
	/* delrange                   */ (Dee_funptr_t)&default__delrange__unsupported,
	/* delrange_index             */ (Dee_funptr_t)&default__delrange_index__unsupported,
	/* delrange_index_n           */ (Dee_funptr_t)&default__delrange_index_n__unsupported,
	/* setrange                   */ (Dee_funptr_t)&default__setrange__unsupported,
	/* setrange_index             */ (Dee_funptr_t)&default__setrange_index__unsupported,
	/* setrange_index_n           */ (Dee_funptr_t)&default__setrange_index_n__unsupported,
	/* inv                        */ (Dee_funptr_t)&default__inv__unsupported,
	/* pos                        */ (Dee_funptr_t)&default__pos__unsupported,
	/* neg                        */ (Dee_funptr_t)&default__neg__unsupported,
	/* add                        */ (Dee_funptr_t)&default__add__unsupported,
	/* inplace_add                */ (Dee_funptr_t)&default__inplace_add__unsupported,
	/* sub                        */ (Dee_funptr_t)&default__sub__unsupported,
	/* inplace_sub                */ (Dee_funptr_t)&default__inplace_sub__unsupported,
	/* mul                        */ (Dee_funptr_t)&default__mul__unsupported,
	/* inplace_mul                */ (Dee_funptr_t)&default__inplace_mul__unsupported,
	/* div                        */ (Dee_funptr_t)&default__div__unsupported,
	/* inplace_div                */ (Dee_funptr_t)&default__inplace_div__unsupported,
	/* mod                        */ (Dee_funptr_t)&default__mod__unsupported,
	/* inplace_mod                */ (Dee_funptr_t)&default__inplace_mod__unsupported,
	/* shl                        */ (Dee_funptr_t)&default__shl__unsupported,
	/* inplace_shl                */ (Dee_funptr_t)&default__inplace_shl__unsupported,
	/* shr                        */ (Dee_funptr_t)&default__shr__unsupported,
	/* inplace_shr                */ (Dee_funptr_t)&default__inplace_shr__unsupported,
	/* and                        */ (Dee_funptr_t)&default__and__unsupported,
	/* inplace_and                */ (Dee_funptr_t)&default__inplace_and__unsupported,
	/* or                         */ (Dee_funptr_t)&default__or__unsupported,
	/* inplace_or                 */ (Dee_funptr_t)&default__inplace_or__unsupported,
	/* xor                        */ (Dee_funptr_t)&default__xor__unsupported,
	/* inplace_xor                */ (Dee_funptr_t)&default__inplace_xor__unsupported,
	/* pow                        */ (Dee_funptr_t)&default__pow__unsupported,
	/* inplace_pow                */ (Dee_funptr_t)&default__inplace_pow__unsupported,
	/* inc                        */ (Dee_funptr_t)&default__inc__unsupported,
	/* dec                        */ (Dee_funptr_t)&default__dec__unsupported,
	/* enter                      */ (Dee_funptr_t)&default__enter__unsupported,
	/* leave                      */ (Dee_funptr_t)&default__leave__unsupported,
	/* getattr                    */ NULL,
	/* getattr_string_hash        */ NULL,
	/* getattr_string_len_hash    */ NULL,
	/* boundattr                  */ NULL,
	/* boundattr_string_hash      */ NULL,
	/* boundattr_string_len_hash  */ NULL,
	/* hasattr                    */ NULL,
	/* hasattr_string_hash        */ NULL,
	/* hasattr_string_len_hash    */ NULL,
	/* delattr                    */ NULL,
	/* delattr_string_hash        */ NULL,
	/* delattr_string_len_hash    */ NULL,
	/* setattr                    */ NULL,
	/* setattr_string_hash        */ NULL,
	/* setattr_string_len_hash    */ NULL,
/*[[[end]]]*/
	/* clang-format on */
};

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_HINT_ERRORS_C */
