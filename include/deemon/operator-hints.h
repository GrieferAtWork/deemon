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
#include <hybrid/limitcore.h>
#include <hybrid/typecore.h>

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
	Dee_TNO_seq_iter,
	Dee_TNO_seq_foreach,
	Dee_TNO_seq_foreach_pair,
	Dee_TNO_math_add,
	Dee_TNO_math_inplace_add,
	Dee_TNO_math_sub,
	Dee_TNO_math_inplace_sub,
	Dee_TNO_math_mul,
	Dee_TNO_math_inplace_mul,
	Dee_TNO_math_div,
	Dee_TNO_math_inplace_div,
	Dee_TNO_math_mod,
	Dee_TNO_math_inplace_mod,
	Dee_TNO_math_shl,
	Dee_TNO_math_inplace_shl,
	Dee_TNO_math_shr,
	Dee_TNO_math_inplace_shr,
	Dee_TNO_math_and,
	Dee_TNO_math_inplace_and,
	Dee_TNO_math_or,
	Dee_TNO_math_inplace_or,
	Dee_TNO_math_xor,
	Dee_TNO_math_inplace_xor,
	Dee_TNO_math_pow,
	Dee_TNO_math_inplace_pow,
	Dee_TNO_math_inc,
	Dee_TNO_math_dec,
	Dee_TNO_with_enter,
	Dee_TNO_with_leave,
/*[[[end]]]*/
	/* clang-format on */
	Dee_TNO_COUNT
};



struct Dee_tno_impl {
#define DEE_TNO_IMPL_DEPS_CONT ((enum Dee_tno_id)__INT_MIN__)
	Dee_funptr_t                             tnoi_impl;  /* [1..1] Default impl (e.g. `default__seq_iter__with__seq_foreach') */
	COMPILER_FLEXIBLE_ARRAY(enum Dee_tno_id, tnoi_deps); /* Vector of dependencies (all but the last dependency are or'd with DEE_TNO_IMPL_DEPS_CONT) */
};

struct Dee_tno_spec {
	__UINTPTR_HALF_TYPE__ tnos_table; /* Offset into `DeeTypeObject' for the sub-table containing this operator, or `0' when part of the primary table. */
	__UINTPTR_HALF_TYPE__ tnos_field; /* Offset into the sub-table (or root) where the operator's function pointer is located. */
	struct Dee_tno_impl  *tnos_impls; /* [0..n] Default impls of this  */
};

INTDEF struct Dee_tno_spec tpconst Dee_tno_specs[Dee_TNO_COUNT];


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
/* tp_seq->tp_iter */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL usrtype__seq_iter(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_iter__with__seq_foreach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default__seq_iter__with__seq_foreach_pair(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__seq_iter(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tusrtype__seq_iter(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__seq_iter__with__seq_foreach(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL tdefault__seq_iter__with__seq_foreach_pair(DeeTypeObject *tp_self, DeeObject *self);
#define default__seq_iter__check(tp_iter) ((tp_iter) == &default__seq_iter__with__seq_foreach || (tp_iter) == &default__seq_iter__with__seq_foreach_pair)
#define typed__seq_iter(tp_iter) ((tp_iter) == &usrtype__seq_iter ? &tusrtype__seq_iter : (tp_iter) == &default__seq_iter__with__seq_foreach ? &tdefault__seq_iter__with__seq_foreach : (tp_iter) == &default__seq_iter__with__seq_foreach_pair ? &tdefault__seq_iter__with__seq_foreach_pair : &tdefault__seq_iter)

/* tp_seq->tp_foreach */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_foreach__with__seq_iter(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_foreach__with__seq_foreach_pair(DeeObject *__restrict self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__seq_foreach(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__seq_foreach__with__seq_iter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__seq_foreach__with__seq_foreach_pair(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_t cb, void *arg);
#define default__seq_foreach__check(tp_foreach) ((tp_foreach) == &default__seq_foreach__with__seq_iter || (tp_foreach) == &default__seq_foreach__with__seq_foreach_pair)
#define typed__seq_foreach(tp_foreach) ((tp_foreach) == &default__seq_foreach__with__seq_iter ? &tdefault__seq_foreach__with__seq_iter : (tp_foreach) == &default__seq_foreach__with__seq_foreach_pair ? &tdefault__seq_foreach__with__seq_foreach_pair : &tdefault__seq_foreach)

/* tp_seq->tp_foreach_pair */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_foreach_pair__with__seq_iter(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL default__seq_foreach_pair__with__seq_foreach(DeeObject *__restrict self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__seq_foreach_pair(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__seq_foreach_pair__with__seq_iter(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL tdefault__seq_foreach_pair__with__seq_foreach(DeeTypeObject *tp_self, DeeObject *self, Dee_foreach_pair_t cb, void *arg);
#define default__seq_foreach_pair__check(tp_foreach_pair) ((tp_foreach_pair) == &default__seq_foreach_pair__with__seq_iter || (tp_foreach_pair) == &default__seq_foreach_pair__with__seq_foreach)
#define typed__seq_foreach_pair(tp_foreach_pair) ((tp_foreach_pair) == &default__seq_foreach_pair__with__seq_iter ? &tdefault__seq_foreach_pair__with__seq_iter : (tp_foreach_pair) == &default__seq_foreach_pair__with__seq_foreach ? &tdefault__seq_foreach_pair__with__seq_foreach : &tdefault__seq_foreach_pair)

/* tp_math->tp_add */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__math_add(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__math_add(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__math_add(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
#define default__math_add__check(tp_add) ()
#define typed__math_add(tp_add) ((tp_add) == &usrtype__math_add ? &tusrtype__math_add : &tdefault__math_add)

/* tp_math->tp_inplace_add */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__math_inplace_add(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__math_inplace_add__with__math_add(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_add(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__math_inplace_add(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_add__with__math_add(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
#define default__math_inplace_add__check(tp_inplace_add) ((tp_inplace_add) == &default__math_inplace_add__with__math_add)
#define typed__math_inplace_add(tp_inplace_add) ((tp_inplace_add) == &usrtype__math_inplace_add ? &tusrtype__math_inplace_add : (tp_inplace_add) == &default__math_inplace_add__with__math_add ? &tdefault__math_inplace_add__with__math_add : &tdefault__math_inplace_add)

/* tp_math->tp_sub */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__math_sub(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__math_sub(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__math_sub(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
#define default__math_sub__check(tp_sub) ()
#define typed__math_sub(tp_sub) ((tp_sub) == &usrtype__math_sub ? &tusrtype__math_sub : &tdefault__math_sub)

/* tp_math->tp_inplace_sub */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__math_inplace_sub(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__math_inplace_sub__with__math_sub(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_sub(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__math_inplace_sub(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_sub__with__math_sub(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
#define default__math_inplace_sub__check(tp_inplace_sub) ((tp_inplace_sub) == &default__math_inplace_sub__with__math_sub)
#define typed__math_inplace_sub(tp_inplace_sub) ((tp_inplace_sub) == &usrtype__math_inplace_sub ? &tusrtype__math_inplace_sub : (tp_inplace_sub) == &default__math_inplace_sub__with__math_sub ? &tdefault__math_inplace_sub__with__math_sub : &tdefault__math_inplace_sub)

/* tp_math->tp_mul */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__math_mul(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__math_mul(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__math_mul(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
#define default__math_mul__check(tp_mul) ()
#define typed__math_mul(tp_mul) ((tp_mul) == &usrtype__math_mul ? &tusrtype__math_mul : &tdefault__math_mul)

/* tp_math->tp_inplace_mul */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__math_inplace_mul(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__math_inplace_mul__with__math_mul(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_mul(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__math_inplace_mul(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_mul__with__math_mul(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
#define default__math_inplace_mul__check(tp_inplace_mul) ((tp_inplace_mul) == &default__math_inplace_mul__with__math_mul)
#define typed__math_inplace_mul(tp_inplace_mul) ((tp_inplace_mul) == &usrtype__math_inplace_mul ? &tusrtype__math_inplace_mul : (tp_inplace_mul) == &default__math_inplace_mul__with__math_mul ? &tdefault__math_inplace_mul__with__math_mul : &tdefault__math_inplace_mul)

/* tp_math->tp_div */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__math_div(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__math_div(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__math_div(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
#define default__math_div__check(tp_div) ()
#define typed__math_div(tp_div) ((tp_div) == &usrtype__math_div ? &tusrtype__math_div : &tdefault__math_div)

/* tp_math->tp_inplace_div */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__math_inplace_div(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__math_inplace_div__with__math_div(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_div(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__math_inplace_div(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_div__with__math_div(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
#define default__math_inplace_div__check(tp_inplace_div) ((tp_inplace_div) == &default__math_inplace_div__with__math_div)
#define typed__math_inplace_div(tp_inplace_div) ((tp_inplace_div) == &usrtype__math_inplace_div ? &tusrtype__math_inplace_div : (tp_inplace_div) == &default__math_inplace_div__with__math_div ? &tdefault__math_inplace_div__with__math_div : &tdefault__math_inplace_div)

/* tp_math->tp_mod */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__math_mod(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__math_mod(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__math_mod(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
#define default__math_mod__check(tp_mod) ()
#define typed__math_mod(tp_mod) ((tp_mod) == &usrtype__math_mod ? &tusrtype__math_mod : &tdefault__math_mod)

/* tp_math->tp_inplace_mod */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__math_inplace_mod(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__math_inplace_mod__with__math_mod(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_mod(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__math_inplace_mod(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_mod__with__math_mod(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
#define default__math_inplace_mod__check(tp_inplace_mod) ((tp_inplace_mod) == &default__math_inplace_mod__with__math_mod)
#define typed__math_inplace_mod(tp_inplace_mod) ((tp_inplace_mod) == &usrtype__math_inplace_mod ? &tusrtype__math_inplace_mod : (tp_inplace_mod) == &default__math_inplace_mod__with__math_mod ? &tdefault__math_inplace_mod__with__math_mod : &tdefault__math_inplace_mod)

/* tp_math->tp_shl */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__math_shl(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__math_shl(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__math_shl(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
#define default__math_shl__check(tp_shl) ()
#define typed__math_shl(tp_shl) ((tp_shl) == &usrtype__math_shl ? &tusrtype__math_shl : &tdefault__math_shl)

/* tp_math->tp_inplace_shl */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__math_inplace_shl(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__math_inplace_shl__with__math_shl(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_shl(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__math_inplace_shl(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_shl__with__math_shl(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
#define default__math_inplace_shl__check(tp_inplace_shl) ((tp_inplace_shl) == &default__math_inplace_shl__with__math_shl)
#define typed__math_inplace_shl(tp_inplace_shl) ((tp_inplace_shl) == &usrtype__math_inplace_shl ? &tusrtype__math_inplace_shl : (tp_inplace_shl) == &default__math_inplace_shl__with__math_shl ? &tdefault__math_inplace_shl__with__math_shl : &tdefault__math_inplace_shl)

/* tp_math->tp_shr */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__math_shr(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__math_shr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__math_shr(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
#define default__math_shr__check(tp_shr) ()
#define typed__math_shr(tp_shr) ((tp_shr) == &usrtype__math_shr ? &tusrtype__math_shr : &tdefault__math_shr)

/* tp_math->tp_inplace_shr */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__math_inplace_shr(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__math_inplace_shr__with__math_shr(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_shr(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__math_inplace_shr(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_shr__with__math_shr(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
#define default__math_inplace_shr__check(tp_inplace_shr) ((tp_inplace_shr) == &default__math_inplace_shr__with__math_shr)
#define typed__math_inplace_shr(tp_inplace_shr) ((tp_inplace_shr) == &usrtype__math_inplace_shr ? &tusrtype__math_inplace_shr : (tp_inplace_shr) == &default__math_inplace_shr__with__math_shr ? &tdefault__math_inplace_shr__with__math_shr : &tdefault__math_inplace_shr)

/* tp_math->tp_and */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__math_and(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__math_and(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__math_and(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
#define default__math_and__check(tp_and) ()
#define typed__math_and(tp_and) ((tp_and) == &usrtype__math_and ? &tusrtype__math_and : &tdefault__math_and)

/* tp_math->tp_inplace_and */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__math_inplace_and(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__math_inplace_and__with__math_and(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_and(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__math_inplace_and(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_and__with__math_and(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
#define default__math_inplace_and__check(tp_inplace_and) ((tp_inplace_and) == &default__math_inplace_and__with__math_and)
#define typed__math_inplace_and(tp_inplace_and) ((tp_inplace_and) == &usrtype__math_inplace_and ? &tusrtype__math_inplace_and : (tp_inplace_and) == &default__math_inplace_and__with__math_and ? &tdefault__math_inplace_and__with__math_and : &tdefault__math_inplace_and)

/* tp_math->tp_or */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__math_or(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__math_or(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__math_or(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
#define default__math_or__check(tp_or) ()
#define typed__math_or(tp_or) ((tp_or) == &usrtype__math_or ? &tusrtype__math_or : &tdefault__math_or)

/* tp_math->tp_inplace_or */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__math_inplace_or(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__math_inplace_or__with__math_or(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_or(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__math_inplace_or(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_or__with__math_or(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
#define default__math_inplace_or__check(tp_inplace_or) ((tp_inplace_or) == &default__math_inplace_or__with__math_or)
#define typed__math_inplace_or(tp_inplace_or) ((tp_inplace_or) == &usrtype__math_inplace_or ? &tusrtype__math_inplace_or : (tp_inplace_or) == &default__math_inplace_or__with__math_or ? &tdefault__math_inplace_or__with__math_or : &tdefault__math_inplace_or)

/* tp_math->tp_xor */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__math_xor(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__math_xor(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__math_xor(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
#define default__math_xor__check(tp_xor) ()
#define typed__math_xor(tp_xor) ((tp_xor) == &usrtype__math_xor ? &tusrtype__math_xor : &tdefault__math_xor)

/* tp_math->tp_inplace_xor */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__math_inplace_xor(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__math_inplace_xor__with__math_xor(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_xor(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__math_inplace_xor(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_xor__with__math_xor(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
#define default__math_inplace_xor__check(tp_inplace_xor) ((tp_inplace_xor) == &default__math_inplace_xor__with__math_xor)
#define typed__math_inplace_xor(tp_inplace_xor) ((tp_inplace_xor) == &usrtype__math_inplace_xor ? &tusrtype__math_inplace_xor : (tp_inplace_xor) == &default__math_inplace_xor__with__math_xor ? &tdefault__math_inplace_xor__with__math_xor : &tdefault__math_inplace_xor)

/* tp_math->tp_pow */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL usrtype__math_pow(DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tdefault__math_pow(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL tusrtype__math_pow(DeeTypeObject *tp_self, DeeObject *lhs, DeeObject *rhs);
#define default__math_pow__check(tp_pow) ()
#define typed__math_pow(tp_pow) ((tp_pow) == &usrtype__math_pow ? &tusrtype__math_pow : &tdefault__math_pow)

/* tp_math->tp_inplace_pow */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usrtype__math_inplace_pow(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default__math_inplace_pow__with__math_pow(DeeObject **__restrict p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_pow(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tusrtype__math_inplace_pow(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL tdefault__math_inplace_pow__with__math_pow(DeeTypeObject *tp_self, DeeObject **p_lhs, DeeObject *rhs);
#define default__math_inplace_pow__check(tp_inplace_pow) ((tp_inplace_pow) == &default__math_inplace_pow__with__math_pow)
#define typed__math_inplace_pow(tp_inplace_pow) ((tp_inplace_pow) == &usrtype__math_inplace_pow ? &tusrtype__math_inplace_pow : (tp_inplace_pow) == &default__math_inplace_pow__with__math_pow ? &tdefault__math_inplace_pow__with__math_pow : &tdefault__math_inplace_pow)

/* tp_math->tp_inc */
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__math_inc(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__math_inc__with__math_inplace_add(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__math_inc__with__math_add(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__math_inc(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__math_inc(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__math_inc__with__math_inplace_add(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__math_inc__with__math_add(DeeTypeObject *tp_self, DeeObject **p_self);
#define default__math_inc__check(tp_inc) ((tp_inc) == &default__math_inc__with__math_inplace_add || (tp_inc) == &default__math_inc__with__math_add)
#define typed__math_inc(tp_inc) ((tp_inc) == &usrtype__math_inc ? &tusrtype__math_inc : (tp_inc) == &default__math_inc__with__math_inplace_add ? &tdefault__math_inc__with__math_inplace_add : (tp_inc) == &default__math_inc__with__math_add ? &tdefault__math_inc__with__math_add : &tdefault__math_inc)

/* tp_math->tp_dec */
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__math_dec(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__math_dec__with__math_inplace_sub(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1)) int DCALL default__math_dec__with__math_sub(DeeObject **__restrict p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__math_dec(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__math_dec(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__math_dec__with__math_inplace_sub(DeeTypeObject *tp_self, DeeObject **p_self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__math_dec__with__math_sub(DeeTypeObject *tp_self, DeeObject **p_self);
#define default__math_dec__check(tp_dec) ((tp_dec) == &default__math_dec__with__math_inplace_sub || (tp_dec) == &default__math_dec__with__math_sub)
#define typed__math_dec(tp_dec) ((tp_dec) == &usrtype__math_dec ? &tusrtype__math_dec : (tp_dec) == &default__math_dec__with__math_inplace_sub ? &tdefault__math_dec__with__math_inplace_sub : (tp_dec) == &default__math_dec__with__math_sub ? &tdefault__math_dec__with__math_sub : &tdefault__math_dec)

/* tp_with->tp_enter */
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__with_enter(DeeObject *__restrict self);
#define default__with_enter__with__with_leave (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__with_enter(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__with_enter(DeeTypeObject *tp_self, DeeObject *self);
#define ttdefault__with_enter__with__with_leave (*(int (DCALL *)(DeeTypeObject *, DeeObject *))&t_DeeNone_reti0_1)
#define default__with_enter__check(tp_enter) ((tp_enter) == &default__with_enter__with__with_leave)
#define typed__with_enter(tp_enter) ((tp_enter) == &usrtype__with_enter ? &tusrtype__with_enter : (tp_enter) == &default__with_enter__with__with_leave ? &tdefault__with_enter__with__with_leave : &tdefault__with_enter)

/* tp_with->tp_leave */
INTDEF WUNUSED NONNULL((1)) int DCALL usrtype__with_leave(DeeObject *__restrict self);
#define default__with_leave__with__with_enter (*(int (DCALL *)(DeeObject *__restrict))&_DeeNone_reti0_1)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tdefault__with_leave(DeeTypeObject *tp_self, DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL tusrtype__with_leave(DeeTypeObject *tp_self, DeeObject *self);
#define ttdefault__with_leave__with__with_enter (*(int (DCALL *)(DeeTypeObject *, DeeObject *))&t_DeeNone_reti0_1)
#define default__with_leave__check(tp_leave) ((tp_leave) == &default__with_leave__with__with_enter)
#define typed__with_leave(tp_leave) ((tp_leave) == &usrtype__with_leave ? &tusrtype__with_leave : (tp_leave) == &default__with_leave__with__with_enter ? &tdefault__with_leave__with__with_enter : &tdefault__with_leave)
/*[[[end]]]*/
/* clang-format on */

#endif /* CONFIG_BUILDING_DEEMON */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_OPERATOR_HINTS_H */
