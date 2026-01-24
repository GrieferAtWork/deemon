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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_HINT_ERRORS_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_HINT_ERRORS_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>          /* Dee_BadAlloc */
#include <deemon/object.h>
#include <deemon/operator-hints.h>
/**/

#include "operator-hint-errors.h"
#include "runtime_error.h"

#include <stddef.h> /* NULL */

DECL_BEGIN

/* Custom impls, as referenced by:
 * >> [[custom_unsupported_impl_name(default__hash__unsupported)]] */
INTERN Dee_hash_t DCALL
default__hash__unsupported(DeeObject *__restrict self) {
	return DeeObject_HashGeneric(self);
}

/* clang-format off */
/*[[[deemon (printNativeOperatorHintErrorImpls from "..method-hints.method-hints")(decls: false);]]]*/
INTERN int DCALL _default__assign__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ASSIGN);
}
INTERN int DCALL _default__move_assign__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_MOVEASSIGN);
}
INTERN void*DCALL _default__str__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_STR);
	return NULL;
}
INTERN Dee_ssize_t DCALL _default__print__unsupported(DeeObject*self, Dee_formatprinter_t UNUSED(arg1), void*UNUSED(arg2)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_STR);
}
INTERN void*DCALL _default__repr__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_REPR);
	return NULL;
}
INTERN Dee_ssize_t DCALL _default__printrepr__unsupported(DeeObject*self, Dee_formatprinter_t UNUSED(arg1), void*UNUSED(arg2)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_REPR);
}
INTERN int DCALL _default__bool__unsupported(DeeObject*self) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_BOOL);
}
INTERN void*DCALL _default__call__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_CALL);
	return NULL;
}
INTERN void*DCALL default__call_kw__badalloc(void*UNUSED(self), void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3)) {
	Dee_BadAlloc(sizeof(struct type_callable));
	return NULL;
}
INTERN void*DCALL _default__call_kw__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_CALL);
	return NULL;
}
INTERN void*DCALL default__thiscall_kw__badalloc(void*UNUSED(self), void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3), void*UNUSED(arg4)) {
	Dee_BadAlloc(sizeof(struct type_callable));
	return NULL;
}
INTERN void*DCALL _default__thiscall_kw__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3), void*UNUSED(arg4)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_CALL);
	return NULL;
}
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
INTERN void*DCALL default__call_tuple__badalloc(void*UNUSED(self), void*UNUSED(arg1)) {
	Dee_BadAlloc(sizeof(struct type_callable));
	return NULL;
}
INTERN void*DCALL _default__call_tuple__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_CALL);
	return NULL;
}
INTERN void*DCALL default__call_tuple_kw__badalloc(void*UNUSED(self), void*UNUSED(arg1), void*UNUSED(arg2)) {
	Dee_BadAlloc(sizeof(struct type_callable));
	return NULL;
}
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
INTERN void*DCALL _default__iter_next__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITERNEXT);
	return NULL;
}
INTERN int DCALL default__nextpair__badalloc(void*UNUSED(self), void*UNUSED(arg1)) {
	return Dee_BadAlloc(sizeof(struct type_iterator));
}
INTERN int DCALL _default__nextpair__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITERNEXT);
}
INTERN void*DCALL default__nextkey__badalloc(void*UNUSED(self)) {
	Dee_BadAlloc(sizeof(struct type_iterator));
	return NULL;
}
INTERN Dee_ssize_t DCALL default__advance__badalloc(void*UNUSED(self), void*UNUSED(arg1)) {
	return Dee_BadAlloc(sizeof(struct type_iterator));
}
INTERN Dee_ssize_t DCALL _default__advance__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITERNEXT);
}
INTERN void*DCALL default__int__badalloc(void*UNUSED(self)) {
	Dee_BadAlloc(sizeof(struct type_math));
	return NULL;
}
INTERN void*DCALL _default__int__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_INT);
	return NULL;
}
INTERN int DCALL default__int32__badalloc(void*UNUSED(self), void*UNUSED(arg1)) {
	return Dee_BadAlloc(sizeof(struct type_math));
}
INTERN int DCALL _default__int32__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_INT);
}
INTERN int DCALL _default__double__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_FLOAT);
}
INTERN Dee_ssize_t DCALL default__hash__badalloc(void*UNUSED(self)) {
	return Dee_BadAlloc(sizeof(struct type_cmp));
}
INTERN int DCALL default__compare_eq__badalloc(void*UNUSED(self), void*UNUSED(arg1)) {
	return Dee_BadAlloc(sizeof(struct type_cmp));
}
INTERN int DCALL _default__compare_eq__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_EQ);
}
INTERN int DCALL _default__compare__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_LO);
}
INTERN void*DCALL default__eq__badalloc(void*UNUSED(self), void*UNUSED(arg1)) {
	Dee_BadAlloc(sizeof(struct type_cmp));
	return NULL;
}
INTERN void*DCALL _default__eq__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_EQ);
	return NULL;
}
INTERN void*DCALL _default__ne__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_NE);
	return NULL;
}
INTERN void*DCALL _default__lo__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_LO);
	return NULL;
}
INTERN void*DCALL _default__le__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_LE);
	return NULL;
}
INTERN void*DCALL _default__gr__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GR);
	return NULL;
}
INTERN void*DCALL _default__ge__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GE);
	return NULL;
}
INTERN void*DCALL default__iter__badalloc(void*UNUSED(self)) {
	Dee_BadAlloc(sizeof(struct type_seq));
	return NULL;
}
INTERN void*DCALL _default__iter__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
	return NULL;
}
INTERN Dee_ssize_t DCALL default__foreach__badalloc(void*UNUSED(self), void*UNUSED(arg1), void*UNUSED(arg2)) {
	return Dee_BadAlloc(sizeof(struct type_seq));
}
INTERN Dee_ssize_t DCALL _default__foreach__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
}
INTERN void*DCALL _default__sizeob__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SIZE);
	return NULL;
}
INTERN Dee_ssize_t DCALL default__size__badalloc(void*UNUSED(self)) {
	return Dee_BadAlloc(sizeof(struct type_seq));
}
INTERN Dee_ssize_t DCALL _default__size__unsupported(DeeObject*self) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SIZE);
}
INTERN void*DCALL default__contains__badalloc(void*UNUSED(self), void*UNUSED(arg1)) {
	Dee_BadAlloc(sizeof(struct type_seq));
	return NULL;
}
INTERN void*DCALL _default__contains__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_CONTAINS);
	return NULL;
}
INTERN void*DCALL _default__getitem__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
	return NULL;
}
INTERN void*DCALL default__getitem_string_hash__badalloc(void*UNUSED(self), void*UNUSED(arg1), void*UNUSED(arg2)) {
	Dee_BadAlloc(sizeof(struct type_seq));
	return NULL;
}
INTERN void*DCALL _default__getitem_string_hash__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
	return NULL;
}
INTERN void*DCALL default__getitem_string_len_hash__badalloc(void*UNUSED(self), void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3)) {
	Dee_BadAlloc(sizeof(struct type_seq));
	return NULL;
}
INTERN void*DCALL _default__getitem_string_len_hash__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
	return NULL;
}
INTERN int DCALL default__bounditem__badalloc(void*UNUSED(self), void*UNUSED(arg1)) {
	return Dee_BadAlloc(sizeof(struct type_seq));
}
INTERN int DCALL _default__bounditem__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
}
INTERN int DCALL default__bounditem_string_hash__badalloc(void*UNUSED(self), void*UNUSED(arg1), void*UNUSED(arg2)) {
	return Dee_BadAlloc(sizeof(struct type_seq));
}
INTERN int DCALL _default__bounditem_string_hash__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
}
INTERN int DCALL default__bounditem_string_len_hash__badalloc(void*UNUSED(self), void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3)) {
	return Dee_BadAlloc(sizeof(struct type_seq));
}
INTERN int DCALL _default__bounditem_string_len_hash__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
}
INTERN int DCALL _default__delitem__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_DELITEM);
}
INTERN int DCALL _default__delitem_string_hash__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_DELITEM);
}
INTERN int DCALL _default__delitem_string_len_hash__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_DELITEM);
}
INTERN int DCALL _default__setitem__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SETITEM);
}
INTERN int DCALL _default__setitem_string_hash__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SETITEM);
}
INTERN int DCALL default__setitem_string_len_hash__badalloc(void*UNUSED(self), void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3), void*UNUSED(arg4)) {
	return Dee_BadAlloc(sizeof(struct type_seq));
}
INTERN int DCALL _default__setitem_string_len_hash__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3), void*UNUSED(arg4)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SETITEM);
}
INTERN void*DCALL _default__getrange__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETRANGE);
	return NULL;
}
INTERN void*DCALL _default__getrange_index_n__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETRANGE);
	return NULL;
}
INTERN int DCALL _default__delrange__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_DELRANGE);
}
INTERN int DCALL _default__delrange_index_n__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_DELRANGE);
}
INTERN int DCALL _default__setrange__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SETRANGE);
}
INTERN int DCALL _default__setrange_index_n__unsupported(DeeObject*self, void*UNUSED(arg1), void*UNUSED(arg2)) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SETRANGE);
}
INTERN void*DCALL _default__inv__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_INV);
	return NULL;
}
INTERN void*DCALL _default__pos__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_POS);
	return NULL;
}
INTERN void*DCALL _default__neg__unsupported(DeeObject*self) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_NEG);
	return NULL;
}
INTERN void*DCALL default__add__badalloc(void*UNUSED(self), void*UNUSED(arg1)) {
	Dee_BadAlloc(sizeof(struct type_math));
	return NULL;
}
INTERN void*DCALL _default__add__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ADD);
	return NULL;
}
INTERN int DCALL _default__inplace_add__unsupported(DREF DeeObject**self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_ADD);
}
INTERN void*DCALL _default__sub__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SUB);
	return NULL;
}
INTERN int DCALL _default__inplace_sub__unsupported(DREF DeeObject**self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_SUB);
}
INTERN void*DCALL _default__mul__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_MUL);
	return NULL;
}
INTERN int DCALL _default__inplace_mul__unsupported(DREF DeeObject**self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_MUL);
}
INTERN void*DCALL _default__div__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_DIV);
	return NULL;
}
INTERN int DCALL _default__inplace_div__unsupported(DREF DeeObject**self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_DIV);
}
INTERN void*DCALL _default__mod__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_MOD);
	return NULL;
}
INTERN int DCALL _default__inplace_mod__unsupported(DREF DeeObject**self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_MOD);
}
INTERN void*DCALL _default__shl__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SHL);
	return NULL;
}
INTERN int DCALL _default__inplace_shl__unsupported(DREF DeeObject**self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_SHL);
}
INTERN void*DCALL _default__shr__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SHR);
	return NULL;
}
INTERN int DCALL _default__inplace_shr__unsupported(DREF DeeObject**self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_SHR);
}
INTERN void*DCALL _default__and__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_AND);
	return NULL;
}
INTERN int DCALL _default__inplace_and__unsupported(DREF DeeObject**self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_AND);
}
INTERN void*DCALL _default__or__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_OR);
	return NULL;
}
INTERN int DCALL _default__inplace_or__unsupported(DREF DeeObject**self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_OR);
}
INTERN void*DCALL _default__xor__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_XOR);
	return NULL;
}
INTERN int DCALL _default__inplace_xor__unsupported(DREF DeeObject**self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_XOR);
}
INTERN void*DCALL _default__pow__unsupported(DeeObject*self, void*UNUSED(arg1)) {
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_POW);
	return NULL;
}
INTERN int DCALL _default__inplace_pow__unsupported(DREF DeeObject**self, void*UNUSED(arg1)) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INPLACE_POW);
}
INTERN int DCALL default__inc__badalloc(void*UNUSED(self)) {
	return Dee_BadAlloc(sizeof(struct type_math));
}
INTERN int DCALL _default__inc__unsupported(DREF DeeObject**self) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_INC);
}
INTERN int DCALL _default__dec__unsupported(DREF DeeObject**self) {
	return err_unimplemented_operator(Dee_TYPE(*self), OPERATOR_DEC);
}
INTERN int DCALL default__enter__badalloc(void*UNUSED(self)) {
	return Dee_BadAlloc(sizeof(struct type_with));
}
INTERN int DCALL _default__enter__unsupported(DeeObject*self) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ENTER);
}
INTERN int DCALL _default__leave__unsupported(DeeObject*self) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_LEAVE);
}
INTERN void*DCALL default__getattr__badalloc(void*UNUSED(self), void*UNUSED(arg1)) {
	Dee_BadAlloc(sizeof(struct type_attr));
	return NULL;
}
INTERN void*DCALL default__getattr_string_hash__badalloc(void*UNUSED(self), void*UNUSED(arg1), void*UNUSED(arg2)) {
	Dee_BadAlloc(sizeof(struct type_attr));
	return NULL;
}
INTERN void*DCALL default__getattr_string_len_hash__badalloc(void*UNUSED(self), void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3)) {
	Dee_BadAlloc(sizeof(struct type_attr));
	return NULL;
}
INTERN int DCALL default__boundattr__badalloc(void*UNUSED(self), void*UNUSED(arg1)) {
	return Dee_BadAlloc(sizeof(struct type_attr));
}
INTERN int DCALL default__boundattr_string_hash__badalloc(void*UNUSED(self), void*UNUSED(arg1), void*UNUSED(arg2)) {
	return Dee_BadAlloc(sizeof(struct type_attr));
}
INTERN int DCALL default__boundattr_string_len_hash__badalloc(void*UNUSED(self), void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3)) {
	return Dee_BadAlloc(sizeof(struct type_attr));
}
INTERN int DCALL default__setattr_string_len_hash__badalloc(void*UNUSED(self), void*UNUSED(arg1), void*UNUSED(arg2), void*UNUSED(arg3), void*UNUSED(arg4)) {
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
	/* [Dee_TNO_assign]                     = */ NULL,
	/* [Dee_TNO_move_assign]                = */ NULL,
	/* [Dee_TNO_str]                        = */ NULL,
	/* [Dee_TNO_print]                      = */ NULL,
	/* [Dee_TNO_repr]                       = */ NULL,
	/* [Dee_TNO_printrepr]                  = */ NULL,
	/* [Dee_TNO_bool]                       = */ NULL,
	/* [Dee_TNO_call]                       = */ NULL,
	/* [Dee_TNO_call_kw]                    = */ (Dee_funptr_t)&default__call_kw__badalloc,
	/* [Dee_TNO_thiscall]                   = */ (Dee_funptr_t)&default__thiscall__badalloc,
	/* [Dee_TNO_thiscall_kw]                = */ (Dee_funptr_t)&default__thiscall_kw__badalloc,
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* [Dee_TNO_call_tuple]                 = */ (Dee_funptr_t)&default__call_tuple__badalloc,
	/* [Dee_TNO_call_tuple_kw]              = */ (Dee_funptr_t)&default__call_tuple_kw__badalloc,
	/* [Dee_TNO_thiscall_tuple]             = */ (Dee_funptr_t)&default__thiscall_tuple__badalloc,
	/* [Dee_TNO_thiscall_tuple_kw]          = */ (Dee_funptr_t)&default__thiscall_tuple_kw__badalloc,
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
	/* [Dee_TNO_iter_next]                  = */ NULL,
	/* [Dee_TNO_nextpair]                   = */ (Dee_funptr_t)&default__nextpair__badalloc,
	/* [Dee_TNO_nextkey]                    = */ (Dee_funptr_t)&default__nextkey__badalloc,
	/* [Dee_TNO_nextvalue]                  = */ (Dee_funptr_t)&default__nextvalue__badalloc,
	/* [Dee_TNO_advance]                    = */ (Dee_funptr_t)&default__advance__badalloc,
	/* [Dee_TNO_int]                        = */ (Dee_funptr_t)&default__int__badalloc,
	/* [Dee_TNO_int32]                      = */ (Dee_funptr_t)&default__int32__badalloc,
	/* [Dee_TNO_int64]                      = */ (Dee_funptr_t)&default__int64__badalloc,
	/* [Dee_TNO_double]                     = */ (Dee_funptr_t)&default__double__badalloc,
	/* [Dee_TNO_hash]                       = */ (Dee_funptr_t)&default__hash__badalloc,
	/* [Dee_TNO_compare_eq]                 = */ (Dee_funptr_t)&default__compare_eq__badalloc,
	/* [Dee_TNO_compare]                    = */ (Dee_funptr_t)&default__compare__badalloc,
	/* [Dee_TNO_trycompare_eq]              = */ (Dee_funptr_t)&default__trycompare_eq__badalloc,
	/* [Dee_TNO_eq]                         = */ (Dee_funptr_t)&default__eq__badalloc,
	/* [Dee_TNO_ne]                         = */ (Dee_funptr_t)&default__ne__badalloc,
	/* [Dee_TNO_lo]                         = */ (Dee_funptr_t)&default__lo__badalloc,
	/* [Dee_TNO_le]                         = */ (Dee_funptr_t)&default__le__badalloc,
	/* [Dee_TNO_gr]                         = */ (Dee_funptr_t)&default__gr__badalloc,
	/* [Dee_TNO_ge]                         = */ (Dee_funptr_t)&default__ge__badalloc,
	/* [Dee_TNO_iter]                       = */ (Dee_funptr_t)&default__iter__badalloc,
	/* [Dee_TNO_foreach]                    = */ (Dee_funptr_t)&default__foreach__badalloc,
	/* [Dee_TNO_foreach_pair]               = */ (Dee_funptr_t)&default__foreach_pair__badalloc,
	/* [Dee_TNO_sizeob]                     = */ (Dee_funptr_t)&default__sizeob__badalloc,
	/* [Dee_TNO_size]                       = */ (Dee_funptr_t)&default__size__badalloc,
	/* [Dee_TNO_size_fast]                  = */ (Dee_funptr_t)&default__size_fast__with__,
	/* [Dee_TNO_contains]                   = */ (Dee_funptr_t)&default__contains__badalloc,
	/* [Dee_TNO_getitem_index_fast]         = */ (Dee_funptr_t)&default__getitem_index_fast__badalloc,
	/* [Dee_TNO_getitem]                    = */ (Dee_funptr_t)&default__getitem__badalloc,
	/* [Dee_TNO_getitem_index]              = */ (Dee_funptr_t)&default__getitem_index__badalloc,
	/* [Dee_TNO_getitem_string_hash]        = */ (Dee_funptr_t)&default__getitem_string_hash__badalloc,
	/* [Dee_TNO_getitem_string_len_hash]    = */ (Dee_funptr_t)&default__getitem_string_len_hash__badalloc,
	/* [Dee_TNO_trygetitem]                 = */ (Dee_funptr_t)&default__trygetitem__badalloc,
	/* [Dee_TNO_trygetitem_index]           = */ (Dee_funptr_t)&default__trygetitem_index__badalloc,
	/* [Dee_TNO_trygetitem_string_hash]     = */ (Dee_funptr_t)&default__trygetitem_string_hash__badalloc,
	/* [Dee_TNO_trygetitem_string_len_hash] = */ (Dee_funptr_t)&default__trygetitem_string_len_hash__badalloc,
	/* [Dee_TNO_bounditem]                  = */ (Dee_funptr_t)&default__bounditem__badalloc,
	/* [Dee_TNO_bounditem_index]            = */ (Dee_funptr_t)&default__bounditem_index__badalloc,
	/* [Dee_TNO_bounditem_string_hash]      = */ (Dee_funptr_t)&default__bounditem_string_hash__badalloc,
	/* [Dee_TNO_bounditem_string_len_hash]  = */ (Dee_funptr_t)&default__bounditem_string_len_hash__badalloc,
	/* [Dee_TNO_hasitem]                    = */ (Dee_funptr_t)&default__hasitem__badalloc,
	/* [Dee_TNO_hasitem_index]              = */ (Dee_funptr_t)&default__hasitem_index__badalloc,
	/* [Dee_TNO_hasitem_string_hash]        = */ (Dee_funptr_t)&default__hasitem_string_hash__badalloc,
	/* [Dee_TNO_hasitem_string_len_hash]    = */ (Dee_funptr_t)&default__hasitem_string_len_hash__badalloc,
	/* [Dee_TNO_delitem]                    = */ (Dee_funptr_t)&default__delitem__badalloc,
	/* [Dee_TNO_delitem_index]              = */ (Dee_funptr_t)&default__delitem_index__badalloc,
	/* [Dee_TNO_delitem_string_hash]        = */ (Dee_funptr_t)&default__delitem_string_hash__badalloc,
	/* [Dee_TNO_delitem_string_len_hash]    = */ (Dee_funptr_t)&default__delitem_string_len_hash__badalloc,
	/* [Dee_TNO_setitem]                    = */ (Dee_funptr_t)&default__setitem__badalloc,
	/* [Dee_TNO_setitem_index]              = */ (Dee_funptr_t)&default__setitem_index__badalloc,
	/* [Dee_TNO_setitem_string_hash]        = */ (Dee_funptr_t)&default__setitem_string_hash__badalloc,
	/* [Dee_TNO_setitem_string_len_hash]    = */ (Dee_funptr_t)&default__setitem_string_len_hash__badalloc,
	/* [Dee_TNO_getrange]                   = */ (Dee_funptr_t)&default__getrange__badalloc,
	/* [Dee_TNO_getrange_index]             = */ (Dee_funptr_t)&default__getrange_index__badalloc,
	/* [Dee_TNO_getrange_index_n]           = */ (Dee_funptr_t)&default__getrange_index_n__badalloc,
	/* [Dee_TNO_delrange]                   = */ (Dee_funptr_t)&default__delrange__badalloc,
	/* [Dee_TNO_delrange_index]             = */ (Dee_funptr_t)&default__delrange_index__badalloc,
	/* [Dee_TNO_delrange_index_n]           = */ (Dee_funptr_t)&default__delrange_index_n__badalloc,
	/* [Dee_TNO_setrange]                   = */ (Dee_funptr_t)&default__setrange__badalloc,
	/* [Dee_TNO_setrange_index]             = */ (Dee_funptr_t)&default__setrange_index__badalloc,
	/* [Dee_TNO_setrange_index_n]           = */ (Dee_funptr_t)&default__setrange_index_n__badalloc,
	/* [Dee_TNO_inv]                        = */ (Dee_funptr_t)&default__inv__badalloc,
	/* [Dee_TNO_pos]                        = */ (Dee_funptr_t)&default__pos__badalloc,
	/* [Dee_TNO_neg]                        = */ (Dee_funptr_t)&default__neg__badalloc,
	/* [Dee_TNO_add]                        = */ (Dee_funptr_t)&default__add__badalloc,
	/* [Dee_TNO_inplace_add]                = */ (Dee_funptr_t)&default__inplace_add__badalloc,
	/* [Dee_TNO_sub]                        = */ (Dee_funptr_t)&default__sub__badalloc,
	/* [Dee_TNO_inplace_sub]                = */ (Dee_funptr_t)&default__inplace_sub__badalloc,
	/* [Dee_TNO_mul]                        = */ (Dee_funptr_t)&default__mul__badalloc,
	/* [Dee_TNO_inplace_mul]                = */ (Dee_funptr_t)&default__inplace_mul__badalloc,
	/* [Dee_TNO_div]                        = */ (Dee_funptr_t)&default__div__badalloc,
	/* [Dee_TNO_inplace_div]                = */ (Dee_funptr_t)&default__inplace_div__badalloc,
	/* [Dee_TNO_mod]                        = */ (Dee_funptr_t)&default__mod__badalloc,
	/* [Dee_TNO_inplace_mod]                = */ (Dee_funptr_t)&default__inplace_mod__badalloc,
	/* [Dee_TNO_shl]                        = */ (Dee_funptr_t)&default__shl__badalloc,
	/* [Dee_TNO_inplace_shl]                = */ (Dee_funptr_t)&default__inplace_shl__badalloc,
	/* [Dee_TNO_shr]                        = */ (Dee_funptr_t)&default__shr__badalloc,
	/* [Dee_TNO_inplace_shr]                = */ (Dee_funptr_t)&default__inplace_shr__badalloc,
	/* [Dee_TNO_and]                        = */ (Dee_funptr_t)&default__and__badalloc,
	/* [Dee_TNO_inplace_and]                = */ (Dee_funptr_t)&default__inplace_and__badalloc,
	/* [Dee_TNO_or]                         = */ (Dee_funptr_t)&default__or__badalloc,
	/* [Dee_TNO_inplace_or]                 = */ (Dee_funptr_t)&default__inplace_or__badalloc,
	/* [Dee_TNO_xor]                        = */ (Dee_funptr_t)&default__xor__badalloc,
	/* [Dee_TNO_inplace_xor]                = */ (Dee_funptr_t)&default__inplace_xor__badalloc,
	/* [Dee_TNO_pow]                        = */ (Dee_funptr_t)&default__pow__badalloc,
	/* [Dee_TNO_inplace_pow]                = */ (Dee_funptr_t)&default__inplace_pow__badalloc,
	/* [Dee_TNO_inc]                        = */ (Dee_funptr_t)&default__inc__badalloc,
	/* [Dee_TNO_dec]                        = */ (Dee_funptr_t)&default__dec__badalloc,
	/* [Dee_TNO_enter]                      = */ (Dee_funptr_t)&default__enter__badalloc,
	/* [Dee_TNO_leave]                      = */ (Dee_funptr_t)&default__leave__badalloc,
	/* [Dee_TNO_getattr]                    = */ (Dee_funptr_t)&default__getattr__badalloc,
	/* [Dee_TNO_getattr_string_hash]        = */ (Dee_funptr_t)&default__getattr_string_hash__badalloc,
	/* [Dee_TNO_getattr_string_len_hash]    = */ (Dee_funptr_t)&default__getattr_string_len_hash__badalloc,
	/* [Dee_TNO_boundattr]                  = */ (Dee_funptr_t)&default__boundattr__badalloc,
	/* [Dee_TNO_boundattr_string_hash]      = */ (Dee_funptr_t)&default__boundattr_string_hash__badalloc,
	/* [Dee_TNO_boundattr_string_len_hash]  = */ (Dee_funptr_t)&default__boundattr_string_len_hash__badalloc,
	/* [Dee_TNO_hasattr]                    = */ (Dee_funptr_t)&default__hasattr__badalloc,
	/* [Dee_TNO_hasattr_string_hash]        = */ (Dee_funptr_t)&default__hasattr_string_hash__badalloc,
	/* [Dee_TNO_hasattr_string_len_hash]    = */ (Dee_funptr_t)&default__hasattr_string_len_hash__badalloc,
	/* [Dee_TNO_delattr]                    = */ (Dee_funptr_t)&default__delattr__badalloc,
	/* [Dee_TNO_delattr_string_hash]        = */ (Dee_funptr_t)&default__delattr_string_hash__badalloc,
	/* [Dee_TNO_delattr_string_len_hash]    = */ (Dee_funptr_t)&default__delattr_string_len_hash__badalloc,
	/* [Dee_TNO_setattr]                    = */ (Dee_funptr_t)&default__setattr__badalloc,
	/* [Dee_TNO_setattr_string_hash]        = */ (Dee_funptr_t)&default__setattr_string_hash__badalloc,
	/* [Dee_TNO_setattr_string_len_hash]    = */ (Dee_funptr_t)&default__setattr_string_len_hash__badalloc,
/*[[[end]]]*/
	/* clang-format on */
};


/* Returns the impl for "id" that calls `err_unimplemented_operator()'.
 * Returns "NULL" if "id" doesn't define a user-code ID */
INTERN_TPCONST Dee_funptr_t tpconst
_DeeType_GetNativeOperatorUnsupported[Dee_TNO_COUNT] = {
	/* clang-format off */
/*[[[deemon (printNativeOperatorHintUnsupportedTable from "..method-hints.method-hints")();]]]*/
	/* [Dee_TNO_assign]                     = */ (Dee_funptr_t)&default__assign__unsupported,
	/* [Dee_TNO_move_assign]                = */ (Dee_funptr_t)&default__move_assign__unsupported,
	/* [Dee_TNO_str]                        = */ (Dee_funptr_t)&default__str__unsupported,
	/* [Dee_TNO_print]                      = */ (Dee_funptr_t)&default__print__unsupported,
	/* [Dee_TNO_repr]                       = */ (Dee_funptr_t)&default__repr__unsupported,
	/* [Dee_TNO_printrepr]                  = */ (Dee_funptr_t)&default__printrepr__unsupported,
	/* [Dee_TNO_bool]                       = */ (Dee_funptr_t)&default__bool__unsupported,
	/* [Dee_TNO_call]                       = */ (Dee_funptr_t)&default__call__unsupported,
	/* [Dee_TNO_call_kw]                    = */ (Dee_funptr_t)&default__call_kw__unsupported,
	/* [Dee_TNO_thiscall]                   = */ (Dee_funptr_t)&default__thiscall__unsupported,
	/* [Dee_TNO_thiscall_kw]                = */ (Dee_funptr_t)&default__thiscall_kw__unsupported,
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* [Dee_TNO_call_tuple]                 = */ (Dee_funptr_t)&default__call_tuple__unsupported,
	/* [Dee_TNO_call_tuple_kw]              = */ (Dee_funptr_t)&default__call_tuple_kw__unsupported,
	/* [Dee_TNO_thiscall_tuple]             = */ (Dee_funptr_t)&default__thiscall_tuple__unsupported,
	/* [Dee_TNO_thiscall_tuple_kw]          = */ (Dee_funptr_t)&default__thiscall_tuple_kw__unsupported,
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
	/* [Dee_TNO_iter_next]                  = */ (Dee_funptr_t)&default__iter_next__unsupported,
	/* [Dee_TNO_nextpair]                   = */ (Dee_funptr_t)&default__nextpair__unsupported,
	/* [Dee_TNO_nextkey]                    = */ (Dee_funptr_t)&default__nextkey__unsupported,
	/* [Dee_TNO_nextvalue]                  = */ (Dee_funptr_t)&default__nextvalue__unsupported,
	/* [Dee_TNO_advance]                    = */ (Dee_funptr_t)&default__advance__unsupported,
	/* [Dee_TNO_int]                        = */ (Dee_funptr_t)&default__int__unsupported,
	/* [Dee_TNO_int32]                      = */ (Dee_funptr_t)&default__int32__unsupported,
	/* [Dee_TNO_int64]                      = */ (Dee_funptr_t)&default__int64__unsupported,
	/* [Dee_TNO_double]                     = */ (Dee_funptr_t)&default__double__unsupported,
	/* [Dee_TNO_hash]                       = */ (Dee_funptr_t)&default__hash__unsupported,
	/* [Dee_TNO_compare_eq]                 = */ (Dee_funptr_t)&default__compare_eq__unsupported,
	/* [Dee_TNO_compare]                    = */ (Dee_funptr_t)&default__compare__unsupported,
	/* [Dee_TNO_trycompare_eq]              = */ (Dee_funptr_t)&default__trycompare_eq__unsupported,
	/* [Dee_TNO_eq]                         = */ (Dee_funptr_t)&default__eq__unsupported,
	/* [Dee_TNO_ne]                         = */ (Dee_funptr_t)&default__ne__unsupported,
	/* [Dee_TNO_lo]                         = */ (Dee_funptr_t)&default__lo__unsupported,
	/* [Dee_TNO_le]                         = */ (Dee_funptr_t)&default__le__unsupported,
	/* [Dee_TNO_gr]                         = */ (Dee_funptr_t)&default__gr__unsupported,
	/* [Dee_TNO_ge]                         = */ (Dee_funptr_t)&default__ge__unsupported,
	/* [Dee_TNO_iter]                       = */ (Dee_funptr_t)&default__iter__unsupported,
	/* [Dee_TNO_foreach]                    = */ (Dee_funptr_t)&default__foreach__unsupported,
	/* [Dee_TNO_foreach_pair]               = */ (Dee_funptr_t)&default__foreach_pair__unsupported,
	/* [Dee_TNO_sizeob]                     = */ (Dee_funptr_t)&default__sizeob__unsupported,
	/* [Dee_TNO_size]                       = */ (Dee_funptr_t)&default__size__unsupported,
	/* [Dee_TNO_size_fast]                  = */ (Dee_funptr_t)&default__size_fast__with__,
	/* [Dee_TNO_contains]                   = */ (Dee_funptr_t)&default__contains__unsupported,
	/* [Dee_TNO_getitem_index_fast]         = */ NULL,
	/* [Dee_TNO_getitem]                    = */ (Dee_funptr_t)&default__getitem__unsupported,
	/* [Dee_TNO_getitem_index]              = */ (Dee_funptr_t)&default__getitem_index__unsupported,
	/* [Dee_TNO_getitem_string_hash]        = */ (Dee_funptr_t)&default__getitem_string_hash__unsupported,
	/* [Dee_TNO_getitem_string_len_hash]    = */ (Dee_funptr_t)&default__getitem_string_len_hash__unsupported,
	/* [Dee_TNO_trygetitem]                 = */ (Dee_funptr_t)&default__trygetitem__unsupported,
	/* [Dee_TNO_trygetitem_index]           = */ (Dee_funptr_t)&default__trygetitem_index__unsupported,
	/* [Dee_TNO_trygetitem_string_hash]     = */ (Dee_funptr_t)&default__trygetitem_string_hash__unsupported,
	/* [Dee_TNO_trygetitem_string_len_hash] = */ (Dee_funptr_t)&default__trygetitem_string_len_hash__unsupported,
	/* [Dee_TNO_bounditem]                  = */ (Dee_funptr_t)&default__bounditem__unsupported,
	/* [Dee_TNO_bounditem_index]            = */ (Dee_funptr_t)&default__bounditem_index__unsupported,
	/* [Dee_TNO_bounditem_string_hash]      = */ (Dee_funptr_t)&default__bounditem_string_hash__unsupported,
	/* [Dee_TNO_bounditem_string_len_hash]  = */ (Dee_funptr_t)&default__bounditem_string_len_hash__unsupported,
	/* [Dee_TNO_hasitem]                    = */ (Dee_funptr_t)&default__hasitem__unsupported,
	/* [Dee_TNO_hasitem_index]              = */ (Dee_funptr_t)&default__hasitem_index__unsupported,
	/* [Dee_TNO_hasitem_string_hash]        = */ (Dee_funptr_t)&default__hasitem_string_hash__unsupported,
	/* [Dee_TNO_hasitem_string_len_hash]    = */ (Dee_funptr_t)&default__hasitem_string_len_hash__unsupported,
	/* [Dee_TNO_delitem]                    = */ (Dee_funptr_t)&default__delitem__unsupported,
	/* [Dee_TNO_delitem_index]              = */ (Dee_funptr_t)&default__delitem_index__unsupported,
	/* [Dee_TNO_delitem_string_hash]        = */ (Dee_funptr_t)&default__delitem_string_hash__unsupported,
	/* [Dee_TNO_delitem_string_len_hash]    = */ (Dee_funptr_t)&default__delitem_string_len_hash__unsupported,
	/* [Dee_TNO_setitem]                    = */ (Dee_funptr_t)&default__setitem__unsupported,
	/* [Dee_TNO_setitem_index]              = */ (Dee_funptr_t)&default__setitem_index__unsupported,
	/* [Dee_TNO_setitem_string_hash]        = */ (Dee_funptr_t)&default__setitem_string_hash__unsupported,
	/* [Dee_TNO_setitem_string_len_hash]    = */ (Dee_funptr_t)&default__setitem_string_len_hash__unsupported,
	/* [Dee_TNO_getrange]                   = */ (Dee_funptr_t)&default__getrange__unsupported,
	/* [Dee_TNO_getrange_index]             = */ (Dee_funptr_t)&default__getrange_index__unsupported,
	/* [Dee_TNO_getrange_index_n]           = */ (Dee_funptr_t)&default__getrange_index_n__unsupported,
	/* [Dee_TNO_delrange]                   = */ (Dee_funptr_t)&default__delrange__unsupported,
	/* [Dee_TNO_delrange_index]             = */ (Dee_funptr_t)&default__delrange_index__unsupported,
	/* [Dee_TNO_delrange_index_n]           = */ (Dee_funptr_t)&default__delrange_index_n__unsupported,
	/* [Dee_TNO_setrange]                   = */ (Dee_funptr_t)&default__setrange__unsupported,
	/* [Dee_TNO_setrange_index]             = */ (Dee_funptr_t)&default__setrange_index__unsupported,
	/* [Dee_TNO_setrange_index_n]           = */ (Dee_funptr_t)&default__setrange_index_n__unsupported,
	/* [Dee_TNO_inv]                        = */ (Dee_funptr_t)&default__inv__unsupported,
	/* [Dee_TNO_pos]                        = */ (Dee_funptr_t)&default__pos__unsupported,
	/* [Dee_TNO_neg]                        = */ (Dee_funptr_t)&default__neg__unsupported,
	/* [Dee_TNO_add]                        = */ (Dee_funptr_t)&default__add__unsupported,
	/* [Dee_TNO_inplace_add]                = */ (Dee_funptr_t)&default__inplace_add__unsupported,
	/* [Dee_TNO_sub]                        = */ (Dee_funptr_t)&default__sub__unsupported,
	/* [Dee_TNO_inplace_sub]                = */ (Dee_funptr_t)&default__inplace_sub__unsupported,
	/* [Dee_TNO_mul]                        = */ (Dee_funptr_t)&default__mul__unsupported,
	/* [Dee_TNO_inplace_mul]                = */ (Dee_funptr_t)&default__inplace_mul__unsupported,
	/* [Dee_TNO_div]                        = */ (Dee_funptr_t)&default__div__unsupported,
	/* [Dee_TNO_inplace_div]                = */ (Dee_funptr_t)&default__inplace_div__unsupported,
	/* [Dee_TNO_mod]                        = */ (Dee_funptr_t)&default__mod__unsupported,
	/* [Dee_TNO_inplace_mod]                = */ (Dee_funptr_t)&default__inplace_mod__unsupported,
	/* [Dee_TNO_shl]                        = */ (Dee_funptr_t)&default__shl__unsupported,
	/* [Dee_TNO_inplace_shl]                = */ (Dee_funptr_t)&default__inplace_shl__unsupported,
	/* [Dee_TNO_shr]                        = */ (Dee_funptr_t)&default__shr__unsupported,
	/* [Dee_TNO_inplace_shr]                = */ (Dee_funptr_t)&default__inplace_shr__unsupported,
	/* [Dee_TNO_and]                        = */ (Dee_funptr_t)&default__and__unsupported,
	/* [Dee_TNO_inplace_and]                = */ (Dee_funptr_t)&default__inplace_and__unsupported,
	/* [Dee_TNO_or]                         = */ (Dee_funptr_t)&default__or__unsupported,
	/* [Dee_TNO_inplace_or]                 = */ (Dee_funptr_t)&default__inplace_or__unsupported,
	/* [Dee_TNO_xor]                        = */ (Dee_funptr_t)&default__xor__unsupported,
	/* [Dee_TNO_inplace_xor]                = */ (Dee_funptr_t)&default__inplace_xor__unsupported,
	/* [Dee_TNO_pow]                        = */ (Dee_funptr_t)&default__pow__unsupported,
	/* [Dee_TNO_inplace_pow]                = */ (Dee_funptr_t)&default__inplace_pow__unsupported,
	/* [Dee_TNO_inc]                        = */ (Dee_funptr_t)&default__inc__unsupported,
	/* [Dee_TNO_dec]                        = */ (Dee_funptr_t)&default__dec__unsupported,
	/* [Dee_TNO_enter]                      = */ (Dee_funptr_t)&default__enter__unsupported,
	/* [Dee_TNO_leave]                      = */ (Dee_funptr_t)&default__leave__unsupported,
	/* [Dee_TNO_getattr]                    = */ NULL,
	/* [Dee_TNO_getattr_string_hash]        = */ NULL,
	/* [Dee_TNO_getattr_string_len_hash]    = */ NULL,
	/* [Dee_TNO_boundattr]                  = */ NULL,
	/* [Dee_TNO_boundattr_string_hash]      = */ NULL,
	/* [Dee_TNO_boundattr_string_len_hash]  = */ NULL,
	/* [Dee_TNO_hasattr]                    = */ NULL,
	/* [Dee_TNO_hasattr_string_hash]        = */ NULL,
	/* [Dee_TNO_hasattr_string_len_hash]    = */ NULL,
	/* [Dee_TNO_delattr]                    = */ NULL,
	/* [Dee_TNO_delattr_string_hash]        = */ NULL,
	/* [Dee_TNO_delattr_string_len_hash]    = */ NULL,
	/* [Dee_TNO_setattr]                    = */ NULL,
	/* [Dee_TNO_setattr_string_hash]        = */ NULL,
	/* [Dee_TNO_setattr_string_len_hash]    = */ NULL,
/*[[[end]]]*/
	/* clang-format on */
};

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_HINT_ERRORS_C */
