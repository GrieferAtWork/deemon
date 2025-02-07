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
#ifndef GUARD_DEEMON_OPERATOR_HINTS_H
#define GUARD_DEEMON_OPERATOR_HINTS_H 1

#include "api.h"
#include "object.h"

#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include "none-operator.h"

DECL_BEGIN

/* Equivalence callbacks for native operators.
 *
 * These equivalences are not related to method hints,
 * and are applicable to *any* type of object, but only
 * within an object itself. */

#if defined(CONFIG_BUILDING_DEEMON) || defined(__DEEMON__)

/* Dee_TypeNativeOperator_ID */
enum Dee_tno_id {
	/* clang-format off */
/*[[[deemon (printNativeOperatorIds from "...src.deemon.method-hints.method-hints")();]]]*/
	Dee_TNO_str,
	Dee_TNO_print,
	Dee_TNO_repr,
	Dee_TNO_printrepr,
	Dee_TNO_bool,
	Dee_TNO_call,
	Dee_TNO_call_kw,
	Dee_TNO_iter_next,
	Dee_TNO_nextpair,
	Dee_TNO_nextkey,
	Dee_TNO_nextvalue,
	Dee_TNO_advance,
	Dee_TNO_int,
	Dee_TNO_int32,
	Dee_TNO_int64,
	Dee_TNO_double,
	Dee_TNO_iter,
	Dee_TNO_foreach,
	Dee_TNO_foreach_pair,
	Dee_TNO_inv,
	Dee_TNO_pos,
	Dee_TNO_neg,
	Dee_TNO_add,
	Dee_TNO_inplace_add,
	Dee_TNO_sub,
	Dee_TNO_inplace_sub,
	Dee_TNO_mul,
	Dee_TNO_inplace_mul,
	Dee_TNO_div,
	Dee_TNO_inplace_div,
	Dee_TNO_mod,
	Dee_TNO_inplace_mod,
	Dee_TNO_shl,
	Dee_TNO_inplace_shl,
	Dee_TNO_shr,
	Dee_TNO_inplace_shr,
	Dee_TNO_and,
	Dee_TNO_inplace_and,
	Dee_TNO_or,
	Dee_TNO_inplace_or,
	Dee_TNO_xor,
	Dee_TNO_inplace_xor,
	Dee_TNO_pow,
	Dee_TNO_inplace_pow,
	Dee_TNO_inc,
	Dee_TNO_dec,
	Dee_TNO_enter,
	Dee_TNO_leave,
/*[[[end]]]*/
	/* clang-format on */
	Dee_TNO_COUNT
};

/* Looking at related operators that actually *are* present,
 * and assuming that `id' isn't implemented, return the most
 * applicable default implementation for the operator.
 *
 * e.g. Given a type that defines `tp_iter' and `id=Dee_TNO_seq_foreach',
 *      this function would return `&default__seq_foreach__with__seq_iter'
 *
 * When no related operators are present (or `id' doesn't
 * have *any* related operators), return `NULL' instead. */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_SelectMissingNativeOperator)(DeeTypeObject const *__restrict self, enum Dee_tno_id id);

/* Same as `DeeType_SelectMissingNativeOperator', but first
 * checks if the slot of `id' is already assigned, and if so:
 * returns whatever is written to that slot. */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperator)(DeeTypeObject const *__restrict self, enum Dee_tno_id id);

/* Similar to `DeeType_GetNativeOperator', but checks if one
 * of the direct bases of `self' implements the same operator
 * the same way as `self' (iow: the operator was inherited
 * into set), and if so: returns `NULL' instead.
 *
 * This function also handles the transitive case where the
 * function pointer returned by `DeeType_GetNativeOperator'
 * is something like `default__seq_iter__with__seq_foreach',
 * in which case the function calls back on itself to verify
 * that at least one dependency (here: Dee_TNO_seq_foreach)
 * is provided by the type "self" privately. */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetPrivateNativeOperator)(DeeTypeObject const *__restrict self, enum Dee_tno_id id);



/* clang-format off */
/*[[[deemon (printNativeOperatorHintDecls from "...src.deemon.method-hints.method-hints")();]]]*/
/* tp_cast.tp_str */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__str(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__str__by_print(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__str__with__print(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL tdefault__str(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__str(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__str__by_print(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__str__with__print(DeeObject *__restrict self);
#define isusrtype__str(tp_str) ((tp_str) == &usrtype__str || (tp_str) == &usrtype__str__by_print)
#define isdefault__str(tp_str) ((tp_str) == &default__str__with__print)
#define maketyped__str(tp_str) ((tp_str) == &usrtype__str ? &tusrtype__str : (tp_str) == &usrtype__str__by_print ? &tusrtype__str__by_print : (tp_str) == &default__str__with__print ? &tdefault__str__with__print : &tdefault__str)

/* tp_cast.tp_print */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tusrtype__print(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tusrtype__print__by_print(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__print__with__str(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL tdefault__print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL usrtype__print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL usrtype__print__by_print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__print__with__str(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
#define isusrtype__print(tp_print) ((tp_print) == &usrtype__print || (tp_print) == &usrtype__print__by_print)
#define isdefault__print(tp_print) ((tp_print) == &default__print__with__str)
#define maketyped__print(tp_print) ((tp_print) == &usrtype__print ? &tusrtype__print : (tp_print) == &usrtype__print__by_print ? &tusrtype__print__by_print : (tp_print) == &default__print__with__str ? &tdefault__print__with__str : &tdefault__print)

/* tp_cast.tp_repr */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__repr(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__repr__by_printrepr(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__repr__with__printrepr(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL tdefault__repr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__repr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__repr__by_printrepr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__repr__with__printrepr(DeeObject *__restrict self);
#define isusrtype__repr(tp_repr) ((tp_repr) == &usrtype__repr || (tp_repr) == &usrtype__repr__by_printrepr)
#define isdefault__repr(tp_repr) ((tp_repr) == &default__repr__with__printrepr)
#define maketyped__repr(tp_repr) ((tp_repr) == &usrtype__repr ? &tusrtype__repr : (tp_repr) == &usrtype__repr__by_printrepr ? &tusrtype__repr__by_printrepr : (tp_repr) == &default__repr__with__printrepr ? &tdefault__repr__with__printrepr : &tdefault__repr)

/* tp_cast.tp_printrepr */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tusrtype__printrepr(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tusrtype__printrepr__by_print(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__printrepr__with__repr(DeeTypeObject *tp_self, DeeObject *self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL tdefault__printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL usrtype__printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL usrtype__printrepr__by_print(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__printrepr__with__repr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg);
#define isusrtype__printrepr(tp_printrepr) ((tp_printrepr) == &usrtype__printrepr || (tp_printrepr) == &usrtype__printrepr__by_print)
#define isdefault__printrepr(tp_printrepr) ((tp_printrepr) == &default__printrepr__with__repr)
#define maketyped__printrepr(tp_printrepr) ((tp_printrepr) == &usrtype__printrepr ? &tusrtype__printrepr : (tp_printrepr) == &usrtype__printrepr__by_print ? &tusrtype__printrepr__by_print : (tp_printrepr) == &default__printrepr__with__repr ? &tdefault__printrepr__with__repr : &tdefault__printrepr)

/* tp_cast.tp_bool */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__bool(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL tdefault__bool(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__bool(DeeObject *__restrict self);
#define isusrtype__bool(tp_bool) ((tp_bool) == &usrtype__bool)
#define maketyped__bool(tp_bool) ((tp_bool) == &usrtype__bool ? &tusrtype__bool : &tdefault__bool)

/* tp_call */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__call(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__call__with__call_kw(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL tdefault__call(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__call(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__call__with__call_kw(DeeObject *self, size_t argc, DeeObject *const *argv);
#define isusrtype__call(tp_call) ((tp_call) == &usrtype__call)
#define isdefault__call(tp_call) ((tp_call) == &default__call__with__call_kw)
#define maketyped__call(tp_call) ((tp_call) == &usrtype__call ? &tusrtype__call : (tp_call) == &default__call__with__call_kw ? &tdefault__call__with__call_kw : &tdefault__call)

/* tp_call_kw */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__call_kw(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__call_kw__with__call(DeeTypeObject *tp_self, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL tdefault__call_kw(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__call_kw(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__call_kw__with__call(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define isusrtype__call_kw(tp_call_kw) ((tp_call_kw) == &usrtype__call_kw)
#define isdefault__call_kw(tp_call_kw) ((tp_call_kw) == &default__call_kw__with__call)
#define maketyped__call_kw(tp_call_kw) ((tp_call_kw) == &usrtype__call_kw ? &tusrtype__call_kw : (tp_call_kw) == &default__call_kw__with__call ? &tdefault__call_kw__with__call : &tdefault__call_kw)

/* tp_iter_next */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__iter_next(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter_next__with__nextpair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL tdefault__iter_next(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__iter_next(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter_next__with__nextpair(DeeObject *__restrict self);
#define isusrtype__iter_next(tp_iter_next) ((tp_iter_next) == &usrtype__iter_next)
#define isdefault__iter_next(tp_iter_next) ((tp_iter_next) == &default__iter_next__with__nextpair)
#define maketyped__iter_next(tp_iter_next) ((tp_iter_next) == &usrtype__iter_next ? &tusrtype__iter_next : (tp_iter_next) == &default__iter_next__with__nextpair ? &tdefault__iter_next__with__nextpair : &tdefault__iter_next)

/* tp_iterator->tp_nextpair */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__nextpair__with__iter_next(DeeTypeObject *tp_self, DeeObject *self, DREF DeeObject *key_and_value[2]);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__nextpair(DeeObject *__restrict self, DREF DeeObject *__restrict key_and_value[2]);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__nextpair__with__iter_next(DeeObject *__restrict self, DREF DeeObject *__restrict key_and_value[2]);
#define isdefault__nextpair(tp_nextpair) ((tp_nextpair) == &default__nextpair__with__iter_next)
#define maketyped__nextpair(tp_nextpair) ((tp_nextpair) == &default__nextpair__with__iter_next ? &tdefault__nextpair__with__iter_next : &tdefault__nextpair)

/* tp_iterator->tp_nextkey */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__nextkey__with__iter_next(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__nextkey__with__nextpair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL tdefault__nextkey(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__nextkey__with__iter_next(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__nextkey__with__nextpair(DeeObject *__restrict self);
#define isdefault__nextkey(tp_nextkey) ((tp_nextkey) == &default__nextkey__with__iter_next || (tp_nextkey) == &default__nextkey__with__nextpair)
#define maketyped__nextkey(tp_nextkey) ((tp_nextkey) == &default__nextkey__with__iter_next ? &tdefault__nextkey__with__iter_next : (tp_nextkey) == &default__nextkey__with__nextpair ? &tdefault__nextkey__with__nextpair : &tdefault__nextkey)

/* tp_iterator->tp_nextvalue */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__nextvalue__with__iter_next(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__nextvalue__with__nextpair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL tdefault__nextvalue(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__nextvalue__with__iter_next(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__nextvalue__with__nextpair(DeeObject *__restrict self);
#define isdefault__nextvalue(tp_nextvalue) ((tp_nextvalue) == &default__nextvalue__with__iter_next || (tp_nextvalue) == &default__nextvalue__with__nextpair)
#define maketyped__nextvalue(tp_nextvalue) ((tp_nextvalue) == &default__nextvalue__with__iter_next ? &tdefault__nextvalue__with__iter_next : (tp_nextvalue) == &default__nextvalue__with__nextpair ? &tdefault__nextvalue__with__nextpair : &tdefault__nextvalue)

/* tp_iterator->tp_advance */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__advance__with__nextkey(DeeTypeObject *tp_self, DeeObject *self, size_t step);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__advance__with__nextvalue(DeeTypeObject *tp_self, DeeObject *self, size_t step);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__advance__with__nextpair(DeeTypeObject *tp_self, DeeObject *self, size_t step);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL tdefault__advance__with__iter_next(DeeTypeObject *tp_self, DeeObject *self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL tdefault__advance(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__advance__with__nextkey(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__advance__with__nextvalue(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__advance__with__nextpair(DeeObject *__restrict self, size_t step);
INTDEF WUNUSED NONNULL((1)) size_t DCALL default__advance__with__iter_next(DeeObject *__restrict self, size_t step);
#define isdefault__advance(tp_advance) ((tp_advance) == &default__advance__with__nextkey || (tp_advance) == &default__advance__with__nextvalue || (tp_advance) == &default__advance__with__nextpair || (tp_advance) == &default__advance__with__iter_next)

/* tp_math->tp_int */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__int(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__int__with__int64(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__int__with__int32(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__int__with__double(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL tdefault__int(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__int(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__int__with__int64(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__int__with__int32(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__int__with__double(DeeObject *__restrict self);
#define isusrtype__int(tp_int) ((tp_int) == &usrtype__int)
#define isdefault__int(tp_int) ((tp_int) == &default__int__with__int64 || (tp_int) == &default__int__with__int32 || (tp_int) == &default__int__with__double)
#define maketyped__int(tp_int) ((tp_int) == &usrtype__int ? &tusrtype__int : (tp_int) == &default__int__with__int64 ? &tdefault__int__with__int64 : (tp_int) == &default__int__with__int32 ? &tdefault__int__with__int32 : (tp_int) == &default__int__with__double ? &tdefault__int__with__double : &tdefault__int)

/* tp_math->tp_int32 */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__int32__with__int64(DeeTypeObject *tp_self, DeeObject *self, int32_t *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__int32__with__int(DeeTypeObject *tp_self, DeeObject *self, int32_t *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__int32__with__double(DeeTypeObject *tp_self, DeeObject *self, int32_t *p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__int32(DeeObject *__restrict self, int32_t *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__int32__with__int64(DeeObject *__restrict self, int32_t *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__int32__with__int(DeeObject *__restrict self, int32_t *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__int32__with__double(DeeObject *__restrict self, int32_t *__restrict p_result);
#define isdefault__int32(tp_int32) ((tp_int32) == &default__int32__with__int64 || (tp_int32) == &default__int32__with__int || (tp_int32) == &default__int32__with__double)
#define maketyped__int32(tp_int32) ((tp_int32) == &default__int32__with__int64 ? &tdefault__int32__with__int64 : (tp_int32) == &default__int32__with__int ? &tdefault__int32__with__int : (tp_int32) == &default__int32__with__double ? &tdefault__int32__with__double : &tdefault__int32)

/* tp_math->tp_int64 */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__int64__with__int32(DeeTypeObject *tp_self, DeeObject *self, int64_t *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__int64__with__int(DeeTypeObject *tp_self, DeeObject *self, int64_t *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__int64__with__double(DeeTypeObject *tp_self, DeeObject *self, int64_t *p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__int64(DeeObject *__restrict self, int64_t *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__int64__with__int32(DeeObject *__restrict self, int64_t *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__int64__with__int(DeeObject *__restrict self, int64_t *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__int64__with__double(DeeObject *__restrict self, int64_t *__restrict p_result);
#define isdefault__int64(tp_int64) ((tp_int64) == &default__int64__with__int32 || (tp_int64) == &default__int64__with__int || (tp_int64) == &default__int64__with__double)
#define maketyped__int64(tp_int64) ((tp_int64) == &default__int64__with__int32 ? &tdefault__int64__with__int32 : (tp_int64) == &default__int64__with__int ? &tdefault__int64__with__int : (tp_int64) == &default__int64__with__double ? &tdefault__int64__with__double : &tdefault__int64)

/* tp_math->tp_double */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__double(DeeTypeObject *tp_self, DeeObject *self, double *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__double__with__int(DeeTypeObject *tp_self, DeeObject *self, double *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__double__with__int64(DeeTypeObject *tp_self, DeeObject *self, double *p_result);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__double__with__int32(DeeTypeObject *tp_self, DeeObject *self, double *p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__double(DeeObject *__restrict self, double *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__double(DeeObject *__restrict self, double *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__double__with__int(DeeObject *__restrict self, double *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__double__with__int64(DeeObject *__restrict self, double *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__double__with__int32(DeeObject *__restrict self, double *__restrict p_result);
#define isusrtype__double(tp_double) ((tp_double) == &usrtype__double)
#define isdefault__double(tp_double) ((tp_double) == &default__double__with__int || (tp_double) == &default__double__with__int64 || (tp_double) == &default__double__with__int32)
#define maketyped__double(tp_double) ((tp_double) == &usrtype__double ? &tusrtype__double : (tp_double) == &default__double__with__int ? &tdefault__double__with__int : (tp_double) == &default__double__with__int64 ? &tdefault__double__with__int64 : (tp_double) == &default__double__with__int32 ? &tdefault__double__with__int32 : &tdefault__double)

/* tp_seq->tp_iter */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__iter(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter__with__foreach(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__iter__with__foreach_pair(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL tdefault__iter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__iter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter__with__foreach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__iter__with__foreach_pair(DeeObject *__restrict self);
#define isusrtype__iter(tp_iter) ((tp_iter) == &usrtype__iter)
#define isdefault__iter(tp_iter) ((tp_iter) == &default__iter__with__foreach || (tp_iter) == &default__iter__with__foreach_pair)
#define maketyped__iter(tp_iter) ((tp_iter) == &usrtype__iter ? &tusrtype__iter : &tdefault__iter)

/* tp_seq->tp_foreach */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__foreach__with__iter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__foreach__with__foreach_pair(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL tdefault__foreach(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__foreach__with__iter(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__foreach__with__foreach_pair(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
#define isdefault__foreach(tp_foreach) ((tp_foreach) == &default__foreach__with__iter || (tp_foreach) == &default__foreach__with__foreach_pair)
#define maketyped__foreach(tp_foreach) ((tp_foreach) == &default__foreach__with__iter ? &tdefault__foreach__with__iter : (tp_foreach) == &default__foreach__with__foreach_pair ? &tdefault__foreach__with__foreach_pair : &tdefault__foreach)

/* tp_seq->tp_foreach_pair */
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__foreach_pair__with__iter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__foreach_pair__with__foreach(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL tdefault__foreach_pair(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__foreach_pair__with__iter(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__foreach_pair__with__foreach(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
#define isdefault__foreach_pair(tp_foreach_pair) ((tp_foreach_pair) == &default__foreach_pair__with__iter || (tp_foreach_pair) == &default__foreach_pair__with__foreach)
#define maketyped__foreach_pair(tp_foreach_pair) ((tp_foreach_pair) == &default__foreach_pair__with__iter ? &tdefault__foreach_pair__with__iter : (tp_foreach_pair) == &default__foreach_pair__with__foreach ? &tdefault__foreach_pair__with__foreach : &tdefault__foreach_pair)

/* tp_math->tp_inv */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__inv(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL tdefault__inv(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__inv(DeeObject *self);
#define isusrtype__inv(tp_inv) ((tp_inv) == &usrtype__inv)
#define maketyped__inv(tp_inv) ((tp_inv) == &usrtype__inv ? &tusrtype__inv : &tdefault__inv)

/* tp_math->tp_pos */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__pos(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL tdefault__pos(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__pos(DeeObject *self);
#define isusrtype__pos(tp_pos) ((tp_pos) == &usrtype__pos)
#define maketyped__pos(tp_pos) ((tp_pos) == &usrtype__pos ? &tusrtype__pos : &tdefault__pos)

/* tp_math->tp_neg */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__neg(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL tdefault__neg(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__neg(DeeObject *self);
#define isusrtype__neg(tp_neg) ((tp_neg) == &usrtype__neg)
#define maketyped__neg(tp_neg) ((tp_neg) == &usrtype__neg ? &tusrtype__neg : &tdefault__neg)

/* tp_math->tp_add */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__add(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__add(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__add(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__add(tp_add) ((tp_add) == &usrtype__add)
#define maketyped__add(tp_add) ((tp_add) == &usrtype__add ? &tusrtype__add : &tdefault__add)

/* tp_math->tp_inplace_add */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_add(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_add__with__add(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inplace_add(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_add(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_add__with__add(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_add(tp_inplace_add) ((tp_inplace_add) == &usrtype__inplace_add)
#define isdefault__inplace_add(tp_inplace_add) ((tp_inplace_add) == &default__inplace_add__with__add)
#define maketyped__inplace_add(tp_inplace_add) ((tp_inplace_add) == &usrtype__inplace_add ? &tusrtype__inplace_add : (tp_inplace_add) == &default__inplace_add__with__add ? &tdefault__inplace_add__with__add : &tdefault__inplace_add)

/* tp_math->tp_sub */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__sub(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__sub(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__sub(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__sub(tp_sub) ((tp_sub) == &usrtype__sub)
#define maketyped__sub(tp_sub) ((tp_sub) == &usrtype__sub ? &tusrtype__sub : &tdefault__sub)

/* tp_math->tp_inplace_sub */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_sub(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_sub__with__sub(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inplace_sub(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_sub(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_sub__with__sub(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_sub(tp_inplace_sub) ((tp_inplace_sub) == &usrtype__inplace_sub)
#define isdefault__inplace_sub(tp_inplace_sub) ((tp_inplace_sub) == &default__inplace_sub__with__sub)
#define maketyped__inplace_sub(tp_inplace_sub) ((tp_inplace_sub) == &usrtype__inplace_sub ? &tusrtype__inplace_sub : (tp_inplace_sub) == &default__inplace_sub__with__sub ? &tdefault__inplace_sub__with__sub : &tdefault__inplace_sub)

/* tp_math->tp_mul */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__mul(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__mul(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__mul(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__mul(tp_mul) ((tp_mul) == &usrtype__mul)
#define maketyped__mul(tp_mul) ((tp_mul) == &usrtype__mul ? &tusrtype__mul : &tdefault__mul)

/* tp_math->tp_inplace_mul */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_mul(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_mul__with__mul(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inplace_mul(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_mul(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_mul__with__mul(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_mul(tp_inplace_mul) ((tp_inplace_mul) == &usrtype__inplace_mul)
#define isdefault__inplace_mul(tp_inplace_mul) ((tp_inplace_mul) == &default__inplace_mul__with__mul)
#define maketyped__inplace_mul(tp_inplace_mul) ((tp_inplace_mul) == &usrtype__inplace_mul ? &tusrtype__inplace_mul : (tp_inplace_mul) == &default__inplace_mul__with__mul ? &tdefault__inplace_mul__with__mul : &tdefault__inplace_mul)

/* tp_math->tp_div */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__div(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__div(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__div(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__div(tp_div) ((tp_div) == &usrtype__div)
#define maketyped__div(tp_div) ((tp_div) == &usrtype__div ? &tusrtype__div : &tdefault__div)

/* tp_math->tp_inplace_div */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_div(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_div__with__div(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inplace_div(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_div(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_div__with__div(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_div(tp_inplace_div) ((tp_inplace_div) == &usrtype__inplace_div)
#define isdefault__inplace_div(tp_inplace_div) ((tp_inplace_div) == &default__inplace_div__with__div)
#define maketyped__inplace_div(tp_inplace_div) ((tp_inplace_div) == &usrtype__inplace_div ? &tusrtype__inplace_div : (tp_inplace_div) == &default__inplace_div__with__div ? &tdefault__inplace_div__with__div : &tdefault__inplace_div)

/* tp_math->tp_mod */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__mod(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__mod(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__mod(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__mod(tp_mod) ((tp_mod) == &usrtype__mod)
#define maketyped__mod(tp_mod) ((tp_mod) == &usrtype__mod ? &tusrtype__mod : &tdefault__mod)

/* tp_math->tp_inplace_mod */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_mod(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_mod__with__mod(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inplace_mod(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_mod(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_mod__with__mod(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_mod(tp_inplace_mod) ((tp_inplace_mod) == &usrtype__inplace_mod)
#define isdefault__inplace_mod(tp_inplace_mod) ((tp_inplace_mod) == &default__inplace_mod__with__mod)
#define maketyped__inplace_mod(tp_inplace_mod) ((tp_inplace_mod) == &usrtype__inplace_mod ? &tusrtype__inplace_mod : (tp_inplace_mod) == &default__inplace_mod__with__mod ? &tdefault__inplace_mod__with__mod : &tdefault__inplace_mod)

/* tp_math->tp_shl */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__shl(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__shl(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__shl(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__shl(tp_shl) ((tp_shl) == &usrtype__shl)
#define maketyped__shl(tp_shl) ((tp_shl) == &usrtype__shl ? &tusrtype__shl : &tdefault__shl)

/* tp_math->tp_inplace_shl */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_shl(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_shl__with__shl(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inplace_shl(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_shl(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_shl__with__shl(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_shl(tp_inplace_shl) ((tp_inplace_shl) == &usrtype__inplace_shl)
#define isdefault__inplace_shl(tp_inplace_shl) ((tp_inplace_shl) == &default__inplace_shl__with__shl)
#define maketyped__inplace_shl(tp_inplace_shl) ((tp_inplace_shl) == &usrtype__inplace_shl ? &tusrtype__inplace_shl : (tp_inplace_shl) == &default__inplace_shl__with__shl ? &tdefault__inplace_shl__with__shl : &tdefault__inplace_shl)

/* tp_math->tp_shr */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__shr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__shr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__shr(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__shr(tp_shr) ((tp_shr) == &usrtype__shr)
#define maketyped__shr(tp_shr) ((tp_shr) == &usrtype__shr ? &tusrtype__shr : &tdefault__shr)

/* tp_math->tp_inplace_shr */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_shr(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_shr__with__shr(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inplace_shr(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_shr(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_shr__with__shr(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_shr(tp_inplace_shr) ((tp_inplace_shr) == &usrtype__inplace_shr)
#define isdefault__inplace_shr(tp_inplace_shr) ((tp_inplace_shr) == &default__inplace_shr__with__shr)
#define maketyped__inplace_shr(tp_inplace_shr) ((tp_inplace_shr) == &usrtype__inplace_shr ? &tusrtype__inplace_shr : (tp_inplace_shr) == &default__inplace_shr__with__shr ? &tdefault__inplace_shr__with__shr : &tdefault__inplace_shr)

/* tp_math->tp_and */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__and(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__and(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__and(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__and(tp_and) ((tp_and) == &usrtype__and)
#define maketyped__and(tp_and) ((tp_and) == &usrtype__and ? &tusrtype__and : &tdefault__and)

/* tp_math->tp_inplace_and */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_and(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_and__with__and(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inplace_and(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_and(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_and__with__and(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_and(tp_inplace_and) ((tp_inplace_and) == &usrtype__inplace_and)
#define isdefault__inplace_and(tp_inplace_and) ((tp_inplace_and) == &default__inplace_and__with__and)
#define maketyped__inplace_and(tp_inplace_and) ((tp_inplace_and) == &usrtype__inplace_and ? &tusrtype__inplace_and : (tp_inplace_and) == &default__inplace_and__with__and ? &tdefault__inplace_and__with__and : &tdefault__inplace_and)

/* tp_math->tp_or */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__or(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__or(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__or(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__or(tp_or) ((tp_or) == &usrtype__or)
#define maketyped__or(tp_or) ((tp_or) == &usrtype__or ? &tusrtype__or : &tdefault__or)

/* tp_math->tp_inplace_or */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_or(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_or__with__or(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inplace_or(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_or(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_or__with__or(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_or(tp_inplace_or) ((tp_inplace_or) == &usrtype__inplace_or)
#define isdefault__inplace_or(tp_inplace_or) ((tp_inplace_or) == &default__inplace_or__with__or)
#define maketyped__inplace_or(tp_inplace_or) ((tp_inplace_or) == &usrtype__inplace_or ? &tusrtype__inplace_or : (tp_inplace_or) == &default__inplace_or__with__or ? &tdefault__inplace_or__with__or : &tdefault__inplace_or)

/* tp_math->tp_xor */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__xor(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__xor(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__xor(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__xor(tp_xor) ((tp_xor) == &usrtype__xor)
#define maketyped__xor(tp_xor) ((tp_xor) == &usrtype__xor ? &tusrtype__xor : &tdefault__xor)

/* tp_math->tp_inplace_xor */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_xor(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_xor__with__xor(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inplace_xor(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_xor(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_xor__with__xor(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_xor(tp_inplace_xor) ((tp_inplace_xor) == &usrtype__inplace_xor)
#define isdefault__inplace_xor(tp_inplace_xor) ((tp_inplace_xor) == &default__inplace_xor__with__xor)
#define maketyped__inplace_xor(tp_inplace_xor) ((tp_inplace_xor) == &usrtype__inplace_xor ? &tusrtype__inplace_xor : (tp_inplace_xor) == &default__inplace_xor__with__xor ? &tdefault__inplace_xor__with__xor : &tdefault__inplace_xor)

/* tp_math->tp_pow */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__pow(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__pow(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__pow(DeeObject *lhs, DeeObject *rhs);
#define isusrtype__pow(tp_pow) ((tp_pow) == &usrtype__pow)
#define maketyped__pow(tp_pow) ((tp_pow) == &usrtype__pow ? &tusrtype__pow : &tdefault__pow)

/* tp_math->tp_inplace_pow */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__inplace_pow(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__inplace_pow__with__pow(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inplace_pow(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__inplace_pow(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__inplace_pow__with__pow(DeeObject **__restrict p_lhs, DeeObject *rhs);
#define isusrtype__inplace_pow(tp_inplace_pow) ((tp_inplace_pow) == &usrtype__inplace_pow)
#define isdefault__inplace_pow(tp_inplace_pow) ((tp_inplace_pow) == &default__inplace_pow__with__pow)
#define maketyped__inplace_pow(tp_inplace_pow) ((tp_inplace_pow) == &usrtype__inplace_pow ? &tusrtype__inplace_pow : (tp_inplace_pow) == &default__inplace_pow__with__pow ? &tdefault__inplace_pow__with__pow : &tdefault__inplace_pow)

/* tp_math->tp_inc */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__inc(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inc__with__inplace_add(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__inc__with__add(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL tdefault__inc(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__inc(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__inc__with__inplace_add(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__inc__with__add(DeeObject **__restrict p_self);
#define isusrtype__inc(tp_inc) ((tp_inc) == &usrtype__inc)
#define isdefault__inc(tp_inc) ((tp_inc) == &default__inc__with__inplace_add || (tp_inc) == &default__inc__with__add)
#define maketyped__inc(tp_inc) ((tp_inc) == &usrtype__inc ? &tusrtype__inc : (tp_inc) == &default__inc__with__inplace_add ? &tdefault__inc__with__inplace_add : (tp_inc) == &default__inc__with__add ? &tdefault__inc__with__add : &tdefault__inc)

/* tp_math->tp_dec */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__dec(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__dec__with__inplace_sub(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__dec__with__sub(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL tdefault__dec(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__dec(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__dec__with__inplace_sub(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__dec__with__sub(DeeObject **__restrict p_self);
#define isusrtype__dec(tp_dec) ((tp_dec) == &usrtype__dec)
#define isdefault__dec(tp_dec) ((tp_dec) == &default__dec__with__inplace_sub || (tp_dec) == &default__dec__with__sub)
#define maketyped__dec(tp_dec) ((tp_dec) == &usrtype__dec ? &tusrtype__dec : (tp_dec) == &default__dec__with__inplace_sub ? &tdefault__dec__with__inplace_sub : (tp_dec) == &default__dec__with__sub ? &tdefault__dec__with__sub : &tdefault__dec)

/* tp_with->tp_enter */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__enter(DeeTypeObject *tp_self, DeeObject *self);
#define tdefault__enter__with__leave (*(int (DCALL *)(DeeTypeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1)) int DCALL tdefault__enter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__enter(DeeObject *__restrict self);
#define default__enter__with__leave (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
#define isusrtype__enter(tp_enter) ((tp_enter) == &usrtype__enter)
#define isdefault__enter(tp_enter) ((tp_enter) == &default__enter__with__leave)
#define maketyped__enter(tp_enter) ((tp_enter) == &usrtype__enter ? &tusrtype__enter : &tdefault__enter)

/* tp_with->tp_leave */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__leave(DeeTypeObject *tp_self, DeeObject *self);
#define tdefault__leave__with__enter (*(int (DCALL *)(DeeTypeObject *, DeeObject *))&_DeeNone_reti0_2)
INTDEF WUNUSED NONNULL((1)) int DCALL tdefault__leave(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__leave(DeeObject *__restrict self);
#define default__leave__with__enter (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
#define isusrtype__leave(tp_leave) ((tp_leave) == &usrtype__leave)
#define isdefault__leave(tp_leave) ((tp_leave) == &default__leave__with__enter)
#define maketyped__leave(tp_leave) ((tp_leave) == &usrtype__leave ? &tusrtype__leave : &tdefault__leave)
/*[[[end]]]*/
/* clang-format on */

#endif /* CONFIG_BUILDING_DEEMON */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_OPERATOR_HINTS_H */
