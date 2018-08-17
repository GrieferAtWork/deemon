/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_H
#define GUARD_DEEMON_COMPILER_OPTIMIZE_H 1

#include "../api.h"
#include "ast.h"

DECL_BEGIN

#ifdef CONFIG_BUILDING_DEEMON

/* Perform optimizations, such as substituting constants and symbols, etc.
 * HINT: Constants have been implemented somewhat differently in deemon v200:
 *       Instead of requiring the user to explicitly state them as such,
 *       constants may appear to no longer exist at all.
 *       Yet in actuality, what can now be considered a constant is really
 *       a variable (symbol) that is initialized once and only ever read
 *       from then on out.
 * NOTE: The caller should test for the `OPTIMIZE_FENABLED' flag before
 *       calling this function, and don't call it when it is set.
 * @return:  0: The branch was potentially optimized.
 * @return: -1: An error occurred. */
INTDEF int (DCALL ast_optimize)(DeeAstObject *__restrict self, bool result_used);

/* Ast optimization sub-functions. */
INTDEF int (DCALL ast_optimize_operator)(DeeAstObject *__restrict self, bool result_used);

INTDEF uint16_t optimizer_flags; /* Set of `OPTIMIZE_F*' */
INTDEF uint16_t unwind_limit;    /* The max amount of times that a loop may be unwound. */
INTDEF unsigned int optimizer_count; /* Incremented each time `ast_optimize' performs an optimization */

/* Similar to `ast_optimize()', but keeps on doing it's thing while `optimizer_count' changes.
 * NOTE: When the `OPTIMIZE_FONEPASS' flag is set, this function behaves identical to `ast_optimize()' */
INTDEF int DCALL ast_optimize_all(DeeAstObject *__restrict self, bool result_used);

/* Check if `a' and `b' are semantically speaking the same AST.
 * When the `OPTIMIZE_FNOCOMPARE' flag is set, this always returns `false' */
INTDEF bool DCALL ast_equal(DeeAstObject *__restrict a, DeeAstObject *__restrict b);

/* Check if the 2 given ASTs can be exchanged in such a way that
 * the second is executed prior to the first within assembly.
 * This usually means that the first does not have any impact
 * on the latter, nor does it invoke any side-effects that could
 * have any influence on the other.
 * NOTE: When the `OPTIMIZE_FNOPREDICT' flag is set, the always returns `false'. */
INTDEF bool DCALL ast_can_exchange(DeeAstObject *__restrict first,
                                   DeeAstObject *__restrict second);

/* Check if the given ast `self' makes use of `sym' in any way.
 * NOTE: When the `OPTIMIZE_FNOPREDICT' flag is set, the always returns `true'. */
INTDEF bool DCALL ast_uses_symbol(DeeAstObject *__restrict self,
                                  struct symbol *__restrict sym);

/* Do a shallow assignment of `other' onto `self' */
INTDEF int DCALL ast_assign(DeeAstObject *__restrict self,
                            DeeAstObject *__restrict other);

/* Graft `other' onto `self', assigning it if both branches have the same scope,
 * or converting `self' into a single-expression multiple-ast containing `other.' */
INTDEF int DCALL ast_graft_onto(DeeAstObject *__restrict self,
                                DeeAstObject *__restrict other);

/* Finalize the contents of `self', but don't destroy the object itself. */
INTDEF void DCALL ast_fini_contents(DeeAstObject *__restrict self);

/* Copy scope and DDI information from `src' and assign them to `ast'.
 * When `ast' is NULL, don't do anything.
 * @return: * : Always re-returns `ast' */
INTDEF DeeAstObject *DCALL ast_setscope_and_ddi(DeeAstObject *ast,
                                                DeeAstObject *__restrict src);

/* Internal optimization helpers... */
INTDEF bool DCALL ast_has_sideeffects(DeeAstObject *__restrict self);
INTDEF bool DCALL ast_is_nothrow(DeeAstObject *__restrict self, bool result_used);

/* Checks if a branch _NEVER_ returns normally (e.g. `yield' can return normally; `return' can't)
 * Something like a label is unpredictable, as it can return even if the previous instruction can't.
 * @return  0: does return
 * @return  1: doesn't return
 * @return -1: always reachable / unpredictable
 * @return -2: always reachable / unpredictable & doesn't return */
INTDEF int DCALL ast_doesnt_return(DeeAstObject *__restrict self, unsigned int flags);
#define AST_DOESNT_RETURN_FNORMAL     0x0000 /*  */
#define AST_DOESNT_RETURN_FINLOOP     0x0001
#define AST_DOESNT_RETURN_FINCATCH    0x0002
#define AST_DOESNT_RETURN_FINCATCHALL 0x0004

/* 0: false, > 0: true, < 0: unpredictable. */
INTDEF int DCALL ast_get_boolean(DeeAstObject *__restrict self);
/* Same as `ast_get_boolean()', but return `-1' if the ast has side-effects. */
INTDEF int DCALL ast_get_boolean_noeffect(DeeAstObject *__restrict self);

/* Predict the typing of a given AST, or return NULL when unpredictable.
 * NOTE: When the `OPTIMIZE_FNOPREDICT' flag is set, this function always returns `NULL'. */
INTDEF DeeTypeObject *DCALL ast_predict_type(DeeAstObject *__restrict self);



#define CONSTEXPR_ILLEGAL 0 /* Use of this object is not allowed. */
#define CONSTEXPR_ALLOWED 1 /* Use of this object is allowed. */
#define CONSTEXPR_USECOPY 2 /* Use is allowed, but you must work with a deep copy. */

/* Return if the optimizer is allowed to perform
 * operations on/with a constant instance `self'.
 * @return: * : One of `CONSTEXPR_*' */
INTDEF int DCALL allow_constexpr(DeeObject *__restrict self);

/* Check if a given object `type' is a type that implements a cast-constructor.
 * When `type' isn't derived from `DeeType_Type', always return `false' */
INTDEF bool DCALL has_cast_constructor(DeeObject *__restrict type);


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define ast_optimize(self,result_used)     __builtin_expect(ast_optimize(self,result_used),0)
#define ast_optimize_all(self,result_used) __builtin_expect(ast_optimize_all(self,result_used),0)
#endif
#endif

#if !defined(NDEBUG) && 1
#define CONFIG_HAVE_OPTIMIZE_VERBOSE 1
#define OPTIMIZE_VERBOSE(...)       ast_optimize_verbose(self,__VA_ARGS__)
#define OPTIMIZE_VERBOSEAT(ast,...) ast_optimize_verbose(ast,__VA_ARGS__)
INTDEF void ast_optimize_verbose(DeeAstObject *__restrict self, char const *format, ...);
#else
#define OPTIMIZE_VERBOSE(...)       (void)0
#define OPTIMIZE_VERBOSEAT(ast,...) (void)0
#endif


#endif /* !CONFIG_BUILDING_DEEMON */


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_H */
