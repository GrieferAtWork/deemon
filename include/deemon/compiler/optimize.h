/* Copyright (c) 2019 Griefer@Work                                            *
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
#if 1
#define OPTIMIZE_FASSUME    0x0800 /* FLAG: Allow assumptions to be made about the value of variables. */
#endif
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


#ifdef OPTIMIZE_FASSUME
struct ast_symbol_assume {
	struct symbol      *sa_sym;   /* [0..1] The symbol on which assumptions are made. */
	DREF DeeObject     *sa_value; /* [0..1][valid_if(sa_sym)] The assumed value of `sa_sym', or NULL if unknown. */
};
#define AST_SYMBOL_ASSUME_HASH(x) ((x)->sa_sym->s_name->k_id)

struct ast_symbol_assumes {
	size_t                    sa_size; /* Number of symbol assumptions. */
	size_t                    sa_mask; /* Allocated hash-vector mask. */
	struct ast_symbol_assume *sa_elem; /* [0..sa_mask + 1][owned] Hash-vector of symbol assumes. */
};
#define AST_SYMBOL_ASSUMES_HASHST(self,hash)  ((hash) & (self)->sa_mask)
#define AST_SYMBOL_ASSUMES_HASHNX(hs,perturb) (((hs) << 2) + (hs) + (perturb) + 1)
#define AST_SYMBOL_ASSUMES_HASHPT(perturb)    ((perturb) >>= 5) /* This `5' is tunable. */
#define AST_SYMBOL_ASSUMES_HASHIT(self,i)     ((self)->sa_elem+((i) & (self)->sa_mask))

struct ast_assumes {
	struct ast_assumes const *aa_prev; /* [0..1] When inside of a conditional branch, this points
	                                    *        to the set of assumptions made before the conditional
	                                    *        portion. */
	struct ast_symbol_assumes aa_syms; /* Symbol assumptions. */
#define AST_ASSUMES_FNORMAL   0x0000   /* Normal assumption flags. */
#define AST_ASSUMES_FFUNCTION 0x0001   /* These assumptions represent the base of a function. */
	uint16_t                  aa_flag; /* Assumption flags (Set of `AST_ASSUMES_F*'). */
};


/* Add an assumption that the value of `sym' currently is set to `value'.
 * When `value' is `NULL', assume that the value of `sym' is now undefined.
 * NOTE: Depending on the type of `sym', no assumption may be made, such as
 *       in the case of external, or global variables, which may arbitrarily
 *       be modified by other threads running independently on the caller.
 * @return:  0: OK.
 * @return: -1: An error occurred. */
INTDEF int (DCALL ast_assumes_setsymval)(struct ast_assumes *__restrict self,
                                         struct symbol *__restrict sym,
                                         DeeObject *value);

/* Lookup the assumed value of a given symbol `sym', and return a reference to it.
 * NOTE: When no such assumption is available, or the symbol is assumed to be
 *       unknown, `NULL' is returned, but no error is thrown. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ast_assumes_getsymval(struct ast_assumes *__restrict self,
                      struct symbol *__restrict sym);


/* Update `self' to be the state of assumptions as it would be
 * if `branch' would have been optimized using those assumptions.
 * However, `branch' will not actually be optimized!
 * This is used in cases where it is necessary to determine changes
 * in assumptions made ahead of time, in order to determine which
 * assumptions will continue to hold, which have changed, etc, such
 * as in a loop:
 * >> local x = "foobar";
 * >> for (local y: get_items()) {
 * >>     // If we were to blindly optimize the loop with previous assumptions,
 * >>     // this use of `x' would get optimized into a constant `"foobar"',
 * >>     // despite the fact that `x' will be re-written further down below.
 * >>     print x;
 * >>     x = y;
 * >> }
 * In cases such as this, we will gather all assumptions made by a
 * loop ahead of time, before actually going ahead and performing
 * optimizations.
 * Then, knowing the assumptions at the start and end of the loop,
 * we can merge then using `ast_assumes_mergecond()' (the logic here
 * being that a loop is a conditional branch in that it will either
 * continue running, in which case the new state will be used, or will
 * exit, in which case the old state will be used), thus meaning that
 * during a generic iteration, the true state of assumptions is the
 * intersections of assumptions made before and after the loop.
 * @return:  0: OK.
 * @return: -1: An error occurred. */
INTDEF int (DCALL ast_assumes_gather)(struct ast_assumes *__restrict self,
                                      struct ast *__restrict branch,
                                      bool result_used);



/* Assume a fully undefined state, creating new negative
 * assumptions for all previously made positive ones.
 * This must be done when encountering a label, as the state
 * of symbols would not be known at this point. */
INTDEF int (DCALL ast_assumes_undefined)(struct ast_assumes *__restrict self);
/* Override _all_ assumptions made within the current function. */
INTDEF int (DCALL ast_assumes_undefined_all)(struct ast_assumes *__restrict self);

/* Initialize an empty set of ast assumptions. */
INTDEF void DCALL
ast_assumes_init(struct ast_assumes *__restrict self);

/* Finalize the given ast assumptions. */
INTDEF void DCALL
ast_assumes_fini(struct ast_assumes *__restrict self);

/* Setup AST assumption at the start of a conditional branch,
 * where the conditionally executed code is located in `child',
 * while assumptions already made until then are in `parent'
 * @return:  0: OK.
 * @return: -1: An error occurred. */
INTDEF int (DCALL ast_assumes_initcond)(struct ast_assumes *__restrict child,
                                        struct ast_assumes const *__restrict parent);

/* Initialize a set of assumptions for a child-function.
 * This also affects the limit of `ast_assumes_undefined_all()' */
INTDEF int (DCALL ast_assumes_initfunction)(struct ast_assumes *__restrict child,
                                            struct ast_assumes const *__restrict parent);

/* Merge assumptions made in `child' and `sibling', such that
 * only assumptions made in both places still hold true, saving
 * that intersection in `child'. Or in other words:
 *   -> Remove all of `child's assumptions, not also made by `sibling'
 *   -> child = child & sibling;
 * >> local foo = 7;
 * >> if (bar()) {
 * >>     foo = 14;
 * >>     print "bar() was true";
 * >> } else {
 * >>     foo = 14;
 * >> }
 * >> // ASSUME(foo == 14)
 * Note however that negative assumptions (i.e. assumptions made
 * that state that the value of a symbol currently is unknown),
 * are merged as a union, meaning that it suffices for either `child'
 * or `sibling' to explicitly not know the value of a symbol, which
 * is required in cases such as the following:
 * >> local foo = 7;
 * >> if (bar()) {
 * >>     foo = get_new_foo();
 * >> } else {
 * >>     foo = 14;
 * >> }
 * >> // ASSUME(foo == UNKNOWN) // Even though both branches made assumptions for `foo'
 * When `sibling' is `NULL', only keep negative assumptions.
 * WARNING: `sibling' (when non-NULL) may have its data stolen.
 * @return:  0: OK.
 * @return: -1: An error occurred. */
INTDEF int (DCALL ast_assumes_mergecond)(struct ast_assumes *__restrict child,
                                         struct ast_assumes *sibling);

/* Merge the assumptions made by `follower' with `self' in a situation
 * where `follower' is a piece of follow-up code to `self', resulting
 * in the same behavior as would have been caused by all assumptions
 * made by `follower' instead having been made in `self'
 * This is used to merge assumptions from conditional branches onto
 * those made by the parent-branch, after they had been merged with
 * each other.
 * WARNING: `follower' may have its data stolen. */
INTDEF int (DCALL ast_assumes_merge)(struct ast_assumes *__restrict self,
                                     struct ast_assumes *__restrict follower);
#endif /* OPTIMIZE_FASSUME */


struct ast_optimize_stack {
	struct ast_optimize_stack *os_prev;   /* [0..1] The ast from which the optimization originates. */
	struct ast                *os_ast;    /* [1..1] The ast being optimized. */
#ifdef OPTIMIZE_FASSUME
	struct ast_assumes        *os_assume; /* [1..1] Valid assumptions within the current branch. */
#endif /* OPTIMIZE_FASSUME */
	bool                       os_used;   /* True if this stack-branch is being used. */
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
INTDEF int (DCALL ast_optimize)(struct ast_optimize_stack *__restrict parent, struct ast *__restrict self, bool result_used);
INTDEF int (DCALL ast_dooptimize)(struct ast_optimize_stack *__restrict stack, struct ast *__restrict self, bool result_used);
INTDEF int (DCALL ast_startoptimize)(struct ast *__restrict self, bool result_used);

/* Ast optimization sub-functions.
 * @param: self:        == stack->os_ast
 * @param: result_used: == stack->os_used */
INTDEF int (DCALL ast_optimize_operator)(struct ast_optimize_stack *__restrict stack, struct ast *__restrict self, bool result_used);
INTDEF int (DCALL ast_optimize_action)(struct ast_optimize_stack *__restrict stack, struct ast *__restrict self, bool result_used);
INTDEF int (DCALL ast_optimize_multiple)(struct ast_optimize_stack *__restrict stack, struct ast *__restrict self, bool result_used);
INTDEF int (DCALL ast_optimize_symbol)(struct ast_optimize_stack *__restrict stack, struct ast *__restrict self, bool result_used);
INTDEF int (DCALL ast_optimize_conditional)(struct ast_optimize_stack *__restrict stack, struct ast *__restrict self, bool result_used);
INTDEF int (DCALL ast_optimize_loop)(struct ast_optimize_stack *__restrict stack, struct ast *__restrict self, bool result_used);
INTDEF int (DCALL ast_optimize_try)(struct ast_optimize_stack *__restrict stack, struct ast *__restrict self, bool result_used);
INTDEF int (DCALL ast_optimize_switch)(struct ast_optimize_stack *__restrict stack, struct ast *__restrict self, bool result_used);

INTDEF uint16_t optimizer_flags;        /* Set of `OPTIMIZE_F*' */
INTDEF uint16_t optimizer_unwind_limit; /* The max amount of times that a loop may be unwound. */
INTDEF unsigned int optimizer_count;    /* Incremented each time `ast_optimize' performs an optimization */

/* Similar to `ast_optimize()', but keeps on doing it's thing while `optimizer_count' changes.
 * NOTE: When the `OPTIMIZE_FONEPASS' flag is set, this function behaves identical to `ast_optimize()' */
INTDEF int (DCALL ast_optimize_all)(struct ast *__restrict self, bool result_used);

/* Check if `a' and `b' are semantically speaking the same AST.
 * When the `OPTIMIZE_FNOCOMPARE' flag is set, this always returns `false' */
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL ast_equal(struct ast *__restrict a, struct ast *__restrict b);

/* Check if the 2 given ASTs can be exchanged in such a way that
 * the second is executed prior to the first within assembly.
 * This usually means that the first does not have any impact
 * on the latter, nor does it invoke any side-effects that could
 * have any influence on the other.
 * NOTE: When the `OPTIMIZE_FNOPREDICT' flag is set, the always returns `false'. */
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL ast_can_exchange(struct ast *__restrict first,
                                   struct ast *__restrict second);

/* Check if the given ast `self' makes use of `sym' in any way.
 * NOTE: When the `OPTIMIZE_FNOPREDICT' flag is set, the always returns `true'. */
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL ast_uses_symbol(struct ast *__restrict self,
                                  struct symbol *__restrict sym);

/* Do a shallow assignment of `other' onto `self' */
INTDEF int (DCALL ast_assign)(struct ast *__restrict self,
                              struct ast *__restrict other);

/* Graft `other' onto `self', assigning it if both branches have the same scope,
 * or converting `self' into a single-expression multiple-ast containing `other.' */
INTDEF int (DCALL ast_graft_onto)(struct ast *__restrict self,
                                  struct ast *__restrict other);

/* Finalize the contents of `self', but don't destroy the object itself. */
INTDEF void DCALL ast_fini_contents(struct ast *__restrict self);

/* Copy scope and DDI information from `src' and assign them to `ast'.
 * When `ast' is NULL, don't do anything.
 * @return: * : Always re-returns `ast' */
INTDEF struct ast *DCALL ast_setscope_and_ddi(struct ast *self,
                                              struct ast *__restrict src);

/* Internal optimization helpers... */
INTDEF WUNUSED NONNULL((1)) bool DCALL ast_has_sideeffects(struct ast *__restrict self);
INTDEF WUNUSED NONNULL((1)) bool DCALL ast_is_nothrow(struct ast *__restrict self, bool result_used);

/* Check if a given ast `self' is, or contains a `goto' branch,
 * or a `break' / `continue' branch when `consider_loopctl' is set. */
INTDEF WUNUSED NONNULL((1)) bool DCALL ast_contains_goto(struct ast *__restrict self, uint16_t consider_loopctl);
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
INTDEF int (DCALL ast_doesnt_return)(struct ast *__restrict self, unsigned int flags);
#define AST_DOESNT_RETURN_FNORMAL     0x0000
#define AST_DOESNT_RETURN_FINLOOP     0x0001
#define AST_DOESNT_RETURN_FINCATCH    0x0002
#define AST_DOESNT_RETURN_FINCATCHALL 0x0004

/* 0: false, > 0: true, < 0: unpredictable. */
INTDEF int (DCALL ast_get_boolean)(struct ast *__restrict self);

/* Same as `ast_get_boolean()', but return `-1' if the ast has side-effects. */
INTDEF int (DCALL ast_get_boolean_noeffect)(struct ast *__restrict self);

/* Predict the typing of a given AST, or return NULL when unpredictable.
 * NOTE: When the `OPTIMIZE_FNOPREDICT' flag is set, this function always returns `NULL'. */
INTDEF WUNUSED NONNULL((1)) DeeTypeObject *DCALL ast_predict_type(struct ast *__restrict self);



#define CONSTEXPR_ILLEGAL 0 /* Use of this object is not allowed. */
#define CONSTEXPR_ALLOWED 1 /* Use of this object is allowed. */
#define CONSTEXPR_USECOPY 2 /* Use is allowed, but you must work with a deep copy. */

/* Return if the optimizer is allowed to perform
 * operations on/with a constant instance `self'.
 * @return: * : One of `CONSTEXPR_*' */
INTDEF int (DCALL allow_constexpr)(DeeObject *__restrict self);

/* Check if a given object `type' is a type that implements a cast-constructor.
 * When `type' isn't derived from `DeeType_Type', always return `false' */
INTDEF WUNUSED NONNULL((1)) bool DCALL has_cast_constructor(DeeObject *__restrict type);


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#ifdef OPTIMIZE_FASSUME
#define ast_assumes_setsymval(self,sym,value)    __builtin_expect(ast_assumes_setsymval(self,sym,value),0)
#define ast_assumes_gather(self,ast,result_used) __builtin_expect(ast_assumes_gather(self,ast,result_used),0)
#define ast_assumes_undefined(self)              __builtin_expect(ast_assumes_undefined(self),0)
#define ast_assumes_undefined_all(self)          __builtin_expect(ast_assumes_undefined_all(self),0)
#define ast_assumes_initcond(child,parent)       __builtin_expect(ast_assumes_initcond(child,parent),0)
#define ast_assumes_initfunction(child,parent)   __builtin_expect(ast_assumes_initfunction(child,parent),0)
#define ast_assumes_mergecond(child,sibling)     __builtin_expect(ast_assumes_mergecond(child,sibling),0)
#define ast_assumes_merge(self,follower)         __builtin_expect(ast_assumes_merge(self,follower),0)
#endif /* OPTIMIZE_FASSUME */
#define ast_optimize(parent,self,result_used)    __builtin_expect(ast_optimize(parent,self,result_used),0)
#define ast_dooptimize(stack,self,result_used)   __builtin_expect(ast_dooptimize(stack,self,result_used),0)
#define ast_startoptimize(self,result_used)      __builtin_expect(ast_startoptimize(self,result_used),0)
#define ast_optimize_all(self,result_used)       __builtin_expect(ast_optimize_all(self,result_used),0)
#define ast_assign(self,other)                   __builtin_expect(ast_assign(self,other),0)
#define ast_graft_onto(self,other)               __builtin_expect(ast_graft_onto(self,other),0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

#if !defined(NDEBUG) && 1
#define CONFIG_HAVE_OPTIMIZE_VERBOSE 1
#define OPTIMIZE_VERBOSE(...)       ast_optimize_verbose(self,__VA_ARGS__)
#define OPTIMIZE_VERBOSEAT(ast, ...) ast_optimize_verbose(ast,__VA_ARGS__)
INTDEF void ast_optimize_verbose(struct ast *__restrict self, char const *format, ...);
#else /* !NDEBUG */
#define OPTIMIZE_VERBOSE(...)       (void)0
#define OPTIMIZE_VERBOSEAT(ast, ...) (void)0
#endif /* NDEBUG */

#endif /* !CONFIG_BUILDING_DEEMON */


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_H */
