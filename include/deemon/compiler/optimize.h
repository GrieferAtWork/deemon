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

#define OPTIMIZE_FNORMAL    0x0000 /* Default optimizer flags. */
#define OPTIMIZE_FDISABLED  0x0000 /* VALUE: AST-Optimization is disabled. */
#define OPTIMIZE_FENABLED   0x0001 /* FLAG: AST-Optimization is enabled (May be or'd with other options below).
                                    *       This is the main optimizer feature flag that enables constant propagation,
                                    *       and is required to enable all other optimizations.
                                    * WARNING: AST optimizations may break DDI consistency by deleting/skewing
                                    *          branches that would otherwise remain available, thus changing the
                                    *          positions of checkpoints, as well as their number.
                                    *          It is therefor recommended not to set this flag unless also
                                    *          passing the `ASM_FNODDI' flag to the assembler during the last
                                    *          phase of code generation. */
#define OPTIMIZE_FONEPASS   0x0008 /* FLAG: Only perform a single optimization pass. */
#define OPTIMIZE_FCSE       0x0100 /* FLAG: Perform common-subexpression-elimination. (i.e. moving stuff out of conditional expressions) */
#define OPTIMIZE_FCONSTSYMS 0x0200 /* FLAG: Allow local, stack & static variables that are only written once to be turned into constants. */
#define OPTIMIZE_FNOUSESYMS 0x0400 /* FLAG: Allow local, stack & static variables that were written, but never read from to be removed. */
#define OPTIMIZE_FNOCOMPARE 0x4000 /* FLAG: Comparing ASTs always returns `false'. */
#define OPTIMIZE_FNOPREDICT 0x8000 /* FLAG: Disable type prediction of ASTs.
                                    *       AST type prediction is able to affect code beyond the optimization
                                    *       pass, as certain ASTs need to generate less assembly when the type
                                    *       of an operand is already known at compile-time (or rather assembly-time).
                                    *       For example: `x = !!y;' When it is known that `y' already is a boolean,
                                    *       then no intermediate code must be generated before assigning its value
                                    *       to `x'. However when `OPTIMIZE_FNOPREDICT' is set, code to cast `y' to
                                    *       a boolean is generated in all cases, regardless of what the actual type
                                    *       of `y' might be if it was even known at all.
                                    * NOTE: Unlike other optimization flags, this one is unaffected by `OPTIMIZE_FENABLED'.
                                    * NOTE: This also causes `ast_has_sideeffects()' to always return `true',
                                    *       `ast_get_boolean()' to always return `-1', as well as disable other
                                    *       internal ast analyzers that are only used for automatic optimizations
                                    *       during assembly to allow for smaller code generation. */



#ifdef CONFIG_BUILDING_DEEMON

struct ast_optimize_stack {
    struct ast_optimize_stack *os_prev; /* [0..1] The ast from which the optimization originates. */
    DeeAstObject              *os_ast;  /* [1..1] The ast being optimized. */
    bool                       os_used; /* True if this stack-branch is being used. */
};


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
INTDEF int (DCALL ast_optimize)(struct ast_optimize_stack *parent, DeeAstObject *__restrict self, bool result_used);

/* Ast optimization sub-functions.
 * @param: self:        == stack->os_ast
 * @param: result_used: == stack->os_used */
INTDEF int (DCALL ast_optimize_operator)(struct ast_optimize_stack *__restrict stack, DeeAstObject *__restrict self, bool result_used);
INTDEF int (DCALL ast_optimize_action)(struct ast_optimize_stack *__restrict stack, DeeAstObject *__restrict self, bool result_used);
INTDEF int (DCALL ast_optimize_multiple)(struct ast_optimize_stack *__restrict stack, DeeAstObject *__restrict self, bool result_used);
INTDEF int (DCALL ast_optimize_symbol)(struct ast_optimize_stack *__restrict stack, DeeAstObject *__restrict self, bool result_used);
INTDEF int (DCALL ast_optimize_conditional)(struct ast_optimize_stack *__restrict stack, DeeAstObject *__restrict self, bool result_used);

INTDEF uint16_t optimizer_flags;        /* Set of `OPTIMIZE_F*' */
INTDEF uint16_t optimizer_unwind_limit; /* The max amount of times that a loop may be unwound. */
INTDEF unsigned int optimizer_count;    /* Incremented each time `ast_optimize' performs an optimization */

/* Similar to `ast_optimize()', but keeps on doing it's thing while `optimizer_count' changes.
 * NOTE: When the `OPTIMIZE_FONEPASS' flag is set, this function behaves identical to `ast_optimize()' */
INTDEF int (DCALL ast_optimize_all)(DeeAstObject *__restrict self, bool result_used);

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

/* Check if a given ast `self' is, or contains a `goto' branch,
 * or a `break' / `continue' branch when `consider_loopctl' is set. */
INTDEF bool DCALL ast_contains_goto(DeeAstObject *__restrict self, uint16_t consider_loopctl);
#define AST_CONTAINS_GOTO_CONSIDER_NONE     0x00
#define AST_CONTAINS_GOTO_CONSIDER_CONTINUE 0x01
#define AST_CONTAINS_GOTO_CONSIDER_BREAK    0x02
#define AST_CONTAINS_GOTO_CONSIDER_ALL      0xff


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
