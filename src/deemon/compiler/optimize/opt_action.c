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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_ACTION_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_ACTION_C 1

#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/dec.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/super.h>

DECL_BEGIN


#if 1
#define is_builtin_object is_builtin_object
PRIVATE WUNUSED NONNULL((1)) bool DCALL
is_builtin_object(DeeObject *__restrict ob) {
	if (Dec_BuiltinID(ob) != DEC_BUILTINID_UNKNOWN)
		return true;
	/* XXX: What about all the other builtin objects? */
	return false;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
warn_idcompare_nonbuiltin(struct ast *__restrict warn_ast) {
	return WARNAST(warn_ast, W_COMPARE_SODO_NONBUILTIN_UNDEFINED);
}
#endif


PRIVATE WUNUSED NONNULL((1)) bool DCALL
ast_sequence_is_nonempty(struct ast *__restrict self) {
	(void)self;
	/* TODO: Check if `self' is a non-empty sequence expression. */
	return false;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
ast_iterator_is_nonempty(struct ast *__restrict self) {
	/* Simple (and most likely) case: Check if `self' invokes `operator iter'. */
	if (self->a_type == AST_OPERATOR &&
	    self->a_flag == OPERATOR_ITERSELF &&
	    !(self->a_operator.o_exflag & (AST_OPERATOR_FPOSTOP | AST_OPERATOR_FVARARGS)) &&
	    self->a_operator.o_op0)
		return ast_sequence_is_nonempty(self->a_operator.o_op0);
	/* TODO: Check if `self' is an iterator that is non-empty. */
	return false;
}


INTERN int
(DCALL ast_optimize_action)(struct ast_optimize_stack *__restrict stack,
                            struct ast *__restrict self, bool result_used) {
	DREF DeeObject *expr_result;
	/* TODO: The result-used parameter depends on what kind of action it is...
	 * TODO: Optimize AST order of `AST_FACTION_IN' and `AST_FACTION_AS'.
	 * TODO: Do constant propagation when branches are known at compile-time. */
	switch (self->a_flag) {

	case AST_FACTION_STORE:
		if (ast_optimize(stack, self->a_action.a_act0, true))
			goto err;
		/* Check for store-to-none (which is a no-op that translates to the stored expression) */
		if (self->a_action.a_act0->a_type == AST_CONSTEXPR &&
		    DeeNone_Check(self->a_action.a_act0->a_constexpr)) {
			/* Optimize the source-expression with propagated result-used settings. */
			if (ast_optimize(stack, self->a_action.a_act1, result_used))
				goto err;
			OPTIMIZE_VERBOSE("Discarding store-to-none\n");
			if unlikely(ast_graft_onto(self, self->a_action.a_act1))
				goto err;
			goto did_optimize;
		}
		if (ast_optimize(stack, self->a_action.a_act1, true))
			goto err;
		if (self->a_action.a_act0->a_type == AST_MULTIPLE) {
			/* TODO: Unwind store-to-multiple, if doing so doesn't cause any
			 *       undesired side-effects, such as a change in execution
			 *       order. */
		}
		if (self->a_action.a_act0->a_type == AST_SYM) {
			struct symbol *target_sym;
			ASSERTF(self->a_action.a_act0->a_flag != 0,
			        "But the symbol of this ast is written to...");
			/* Store constant expression to symbol.
			 * Check if we can turn this symbol into a constant expression! */
			target_sym = self->a_action.a_act0->a_sym;
			SYMBOL_INPLACE_UNWIND_ALIAS(target_sym);
			/* If writing to this symbol has side-effects,
			 * don't accidentally optimize away the write. */
			if (symbol_set_haseffect(target_sym, self->a_scope))
				goto after_target_symbol_optimization;
			if (!target_sym->s_nread && !target_sym->s_nbound &&
			    (optimizer_flags & OPTIMIZE_FNOUSESYMS)) {
				/* Special case: the symbol is never read from, or checked for being bound.
				 * -> We can simply get rid of this symbol all-together! */
				OPTIMIZE_VERBOSE("Removing store to symbol `%s' that is never "
				                 "read from, or checking for being bound\n",
				                 SYMBOL_NAME(target_sym));
				if unlikely(ast_graft_onto(self, self->a_action.a_act1))
					goto err;
				goto did_optimize;
			}
			/* Check if what's being assigned is a constant expression. */
			if (self->a_action.a_act1->a_type == AST_CONSTEXPR) {
				if (optimizer_flags & OPTIMIZE_FCONSTSYMS) {
					if (target_sym->s_nwrite == 1 &&
					    !(target_sym->s_flag & (SYMBOL_FALLOC | SYMBOL_FALLOCREF)) &&
					    (target_sym->s_type == SYMBOL_TYPE_LOCAL ||
					     target_sym->s_type == SYMBOL_TYPE_STACK ||
					     target_sym->s_type == SYMBOL_TYPE_STATIC) &&
					    allow_constexpr(self->a_action.a_act1->a_constexpr) == CONSTEXPR_ALLOWED) {
						/* Everything seems to check out so far.
						 * However, we must still be careful to make sure that the symbol's scope is
						 * reachable from our current scope, as well as that between our own ast, and
						 * the first ast apart of the symbol's scope, no conditional AST or loop that
						 * doesn't guaranty that it's block is executed at least once exists:
						 * Example #1:
						 * >> local x;
						 * >> if (foo()) {
						 * >>     x = "foobar"; // This can't be propagated
						 * >> }
						 * >> print x;
						 * Example #2:
						 * >> local x;
						 * >> {
						 * >>     x = "foobar"; // This be propagated
						 * >> }
						 * >> print x;
						 * Example #3:
						 * >> local x;
						 * >> for (local y: foo()) {
						 * >>     x = "foobar"; // This can't be propagated
						 * >> }
						 * >> print x;
						 * Example #4:
						 * >> local x;
						 * >> local i = 1;
						 * >> do {
						 * >>     x = "foobar"; // This can be propagated
						 * >> } while (--i != 0);
						 * >> print x;
						 * NOTE: Checks for `goto' are required to ensure that no side-effects
						 *       are broken by inlining constants assigned to variables:
						 * >>     local x;
						 * >>     goto foo;     // Because of this `goto', our branch becomes conditional,
						 * >>                   // and just as with regular conditional branches, constant
						 * >>                   // symbols cannot be propagated outside of them!
						 * >>     x = "foobar";
						 * >> foo:
						 * >>     print x;
						 */
						struct ast_optimize_stack *iter       = stack;
						struct ast_optimize_stack *prev_stack = NULL;
						DeeScopeObject *symbol_scope          = target_sym->s_scope;
						bool found_scope                      = false;
						for (;;) {
							struct ast *iter_ast = iter->os_ast;
							if (iter_ast == self)
								goto stack_next;
							if (iter_ast->a_scope != symbol_scope && found_scope)
								break; /* We've exited the declaration scope successfully. */
							/* Check what kind of AST we're dealing with here. */
							if (iter_ast->a_type == AST_CONDITIONAL) {
								/* If we're originating from the condition-branch itself, then
								 * we know that we aren't actually a conditional branch ourself,
								 * but rather get executed unconditionally! */
								if (prev_stack->os_ast != iter_ast->a_conditional.c_cond)
									goto done_symbol_store;
							} else if (iter_ast->a_type == AST_LOOP) {
								/* Make sure that our expression is run at least once by the loop. */
								struct ast *prev_ast = prev_stack->os_ast;
								if (iter_ast->a_flag & AST_FLOOP_FOREACH) {
									if (iter_ast->a_loop.l_iter == prev_ast)
										goto stack_next; /* It's OK. - The iterator is always executed. */
									if (!ast_iterator_is_nonempty(iter_ast->a_loop.l_iter))
										goto done_symbol_store; /* The iterator may be empty, meaning that `elem' and `loop' may not be executed! */
									if (ast_contains_goto(iter_ast->a_loop.l_iter, AST_CONTAINS_GOTO_CONSIDER_NONE))
										goto done_symbol_store;
									if (ast_uses_symbol(iter_ast->a_loop.l_iter, target_sym))
										goto done_symbol_store; /* The iterator uses our symbol. */
									if (iter_ast->a_loop.l_elem) {
										if (iter_ast->a_loop.l_elem == prev_ast)
											goto stack_next;
										if (ast_contains_goto(iter_ast->a_loop.l_elem, AST_CONTAINS_GOTO_CONSIDER_NONE))
											goto done_symbol_store;
										if (ast_uses_symbol(iter_ast->a_loop.l_elem, target_sym))
											goto done_symbol_store; /* The element-expression uses our symbol. */
									}
								} else if (iter_ast->a_flag & AST_FLOOP_POSTCOND) {
									if (iter_ast->a_loop.l_loop) {
										if (iter_ast->a_loop.l_loop == prev_ast)
											goto stack_next;
										if (ast_contains_goto(iter_ast->a_loop.l_loop, AST_CONTAINS_GOTO_CONSIDER_NONE))
											goto done_symbol_store;
										if (ast_uses_symbol(iter_ast->a_loop.l_loop, target_sym))
											goto done_symbol_store;
									}
									if (iter_ast->a_loop.l_next) {
										if (iter_ast->a_loop.l_next == prev_ast)
											goto stack_next;
										if (ast_contains_goto(iter_ast->a_loop.l_next, AST_CONTAINS_GOTO_CONSIDER_NONE))
											goto done_symbol_store;
										if (ast_uses_symbol(iter_ast->a_loop.l_next, target_sym))
											goto done_symbol_store;
									}
								} else {
									if (!iter_ast->a_loop.l_cond)
										goto stack_next; /* Without a condition, the loop is guarantied to execute. */
									/* Since the condition may be false, we might not get executed. */
									goto done_symbol_store;
								}
							} else if (iter_ast->a_type == AST_MULTIPLE) {
								/* Check if there is a `goto' before the matching sub-ast,
								 * which might be used to skip our expression. */
								size_t i;
								struct ast *prev_ast = prev_stack->os_ast;
								for (i = 0; i < iter_ast->a_multiple.m_astc; ++i) {
									struct ast *sub_ast = iter_ast->a_multiple.m_astv[i];
									if (prev_ast == sub_ast)
										break;
									if (ast_contains_goto(sub_ast, AST_CONTAINS_GOTO_CONSIDER_ALL))
										goto done_symbol_store; /* There's a goto between declaration & initialization */
									if (ast_uses_symbol(sub_ast, target_sym))
										goto done_symbol_store; /* Nope! - The symbol is used prior to it being assigned. */
								}
							} else {
								/* Don't run any risks. - Use a whitelist of allowed branches
								 * between the symbol being declared, and the symbol being
								 * initialized. */
								goto done_symbol_store;
							}
stack_next:
							if (iter_ast->a_scope == symbol_scope)
								found_scope = true;
							prev_stack = iter;
							iter       = iter->os_prev;
							if (!iter) {
								if (found_scope)
									break;
								goto done_symbol_store;
							}
						}

						/* Everything checks out. - We can use this symbol for constant propagation. */
						OPTIMIZE_VERBOSE("Defining symbol `%s' as a constant evaluating to `%r'\n",
						                 SYMBOL_NAME(target_sym), self->a_action.a_act1->a_constexpr);
						target_sym->s_type  = SYMBOL_TYPE_CONST;
						target_sym->s_const = self->a_action.a_act1->a_constexpr;
						Dee_Incref(target_sym->s_const);
						/* Replace the store-branch with a  */
						if unlikely(ast_graft_onto(self, self->a_action.a_act1))
							goto err;
						goto did_optimize;
					}
					if (!target_sym->s_nread && target_sym->s_nbound &&
					    !DeeNone_Check(self->a_action.a_act1->a_constexpr)) {
						/* Special case: the symbol is never read from, but checking for being bound.
						 *  - In this case, we can instead assign a quicker value (`Dee_None'),
						 *    that the constant currently being used. */
						OPTIMIZE_VERBOSE("Store `none' in symbol `%s' only ever checked for being bound, rather than `%r'\n",
						                 SYMBOL_NAME(target_sym), self->a_action.a_act1->a_constexpr);
						Dee_Incref(Dee_None);
						Dee_Decref(self->a_action.a_act1->a_constexpr);
						self->a_action.a_act1->a_constexpr = Dee_None;
						goto did_optimize;
					}
				}
done_symbol_store:
#ifdef OPTIMIZE_FASSUME
				if (optimizer_flags & OPTIMIZE_FASSUME) {
					if (allow_constexpr(self->a_action.a_act1->a_constexpr) != CONSTEXPR_ALLOWED)
						goto unset_symbol_assumption;
					if (ast_assumes_setsymval(stack->os_assume,
					                          target_sym,
					                          self->a_action.a_act1->a_constexpr))
						goto err;
				}
#endif /* OPTIMIZE_FASSUME */
				;
			}
#ifdef OPTIMIZE_FASSUME
			else if (optimizer_flags & OPTIMIZE_FASSUME) {
unset_symbol_assumption:
				if (ast_assumes_setsymval(stack->os_assume,
				                          target_sym,
				                          NULL))
					goto err;
			}
#endif /* OPTIMIZE_FASSUME */
		}
after_target_symbol_optimization:
		break;

	case AST_FACTION_IN:
		if (ast_optimize(stack, self->a_action.a_act0, true))
			goto err;
		if (ast_optimize(stack, self->a_action.a_act1, true))
			goto err;
		if (self->a_action.a_act0->a_type == AST_CONSTEXPR &&
		    self->a_action.a_act1->a_type == AST_CONSTEXPR) {
			/* Propagate constants. */
			expr_result = DeeObject_ContainsObject(self->a_action.a_act1->a_constexpr,
			                                       self->a_action.a_act0->a_constexpr);
action_set_expr_result:
			if (!expr_result)
				DeeError_Handled(ERROR_HANDLED_RESTORE);
			else {
				int temp = allow_constexpr(expr_result);
				if (temp == CONSTEXPR_ILLEGAL)
					Dee_Decref(expr_result);
				else {
					if (temp == CONSTEXPR_USECOPY &&
					    DeeObject_InplaceDeepCopy(&expr_result)) {
						Dee_Decref(expr_result);
						DeeError_Handled(ERROR_HANDLED_RESTORE);
					} else {
						if (AST_FACTION_ARGC_GT(self->a_flag) >= 3)
							ast_decref(self->a_action.a_act2);
						if (AST_FACTION_ARGC_GT(self->a_flag) >= 2)
							ast_decref(self->a_action.a_act1);
						if (AST_FACTION_ARGC_GT(self->a_flag) >= 1)
							ast_decref(self->a_action.a_act0);
						self->a_constexpr = expr_result; /* Inherit reference. */
						self->a_type      = AST_CONSTEXPR;
						OPTIMIZE_VERBOSE("Propagate constant result of action-expression: %r\n", expr_result);
						goto did_optimize;
					}
				}
			}
		}
		break;

	case AST_FACTION_AS:
		if (ast_optimize(stack, self->a_action.a_act0, true))
			goto err;
		if (ast_optimize(stack, self->a_action.a_act1, true))
			goto err;
		if (self->a_action.a_act0->a_type == AST_CONSTEXPR &&
		    self->a_action.a_act1->a_type == AST_CONSTEXPR &&
		    DeeType_Check(self->a_action.a_act1->a_constexpr)) {
			/* Propagate constants. */
			expr_result = DeeSuper_New((DeeTypeObject *)self->a_action.a_act1->a_constexpr,
			                           self->a_action.a_act0->a_constexpr);
			goto action_set_expr_result;
		}
		break;

	case AST_FACTION_IS:
		if (ast_optimize(stack, self->a_action.a_act0, true))
			goto err;
		if (ast_optimize(stack, self->a_action.a_act1, true))
			goto err;
		if (self->a_action.a_act0->a_type == AST_CONSTEXPR &&
		    self->a_action.a_act1->a_type == AST_CONSTEXPR) {
			if (DeeType_Check(self->a_action.a_act1->a_constexpr)) {
				/* Propagate constants. */
				expr_result = DeeBool_For(DeeObject_InstanceOf(self->a_action.a_act0->a_constexpr,
				                                               (DeeTypeObject *)self->a_action.a_act1->a_constexpr));
				Dee_Incref(expr_result);
				goto action_set_expr_result;
			}
			if (DeeNone_Check(self->a_action.a_act1->a_constexpr)) {
				/* Propagate constants. */
				expr_result = DeeBool_For(DeeNone_Check(self->a_action.a_act0->a_constexpr));
				Dee_Incref(expr_result);
				goto action_set_expr_result;
			}
		}
		break;

	case AST_FACTION_MIN:
		if (ast_optimize(stack, self->a_action.a_act0, true))
			goto err;
		if (self->a_action.a_act0->a_type == AST_CONSTEXPR) {
			expr_result = DeeSeq_Min(self->a_action.a_act0->a_constexpr, NULL);
			goto action_set_expr_result;
		}
		break;

	case AST_FACTION_MAX:
		if (ast_optimize(stack, self->a_action.a_act0, true))
			goto err;
		if (self->a_action.a_act0->a_type == AST_CONSTEXPR) {
			expr_result = DeeSeq_Max(self->a_action.a_act0->a_constexpr, NULL);
			goto action_set_expr_result;
		}
		break;

	case AST_FACTION_SUM:
		if (ast_optimize(stack, self->a_action.a_act0, true))
			goto err;
		if (self->a_action.a_act0->a_type == AST_CONSTEXPR) {
			expr_result = DeeSeq_Sum(self->a_action.a_act0->a_constexpr);
			goto action_set_expr_result;
		}
		break;

	case AST_FACTION_ANY:
		if (ast_optimize(stack, self->a_action.a_act0, true))
			goto err;
		if (self->a_action.a_act0->a_type == AST_CONSTEXPR) {
			int temp = DeeSeq_Any(self->a_action.a_act0->a_constexpr);
			if (temp < 0)
				expr_result = NULL;
			else {
				expr_result = DeeBool_For(temp);
				Dee_Incref(expr_result);
			}
			goto action_set_expr_result;
		}
		break;

	case AST_FACTION_ALL:
		if (ast_optimize(stack, self->a_action.a_act0, true))
			goto err;
		if (self->a_action.a_act0->a_type == AST_CONSTEXPR) {
			int temp = DeeSeq_All(self->a_action.a_act0->a_constexpr);
			if (temp < 0)
				expr_result = NULL;
			else {
				expr_result = DeeBool_For(temp);
				Dee_Incref(expr_result);
			}
			goto action_set_expr_result;
		}
		break;

	case AST_FACTION_BOUNDATTR:
		if (ast_optimize(stack, self->a_action.a_act0, true))
			goto err;
		if (ast_optimize(stack, self->a_action.a_act1, true))
			goto err;
		if (self->a_action.a_act0->a_type == AST_CONSTEXPR &&
		    self->a_action.a_act1->a_type == AST_CONSTEXPR &&
		    DeeString_Check(self->a_action.a_act1->a_constexpr)) {
			int temp = DeeObject_BoundAttr(self->a_action.a_act0->a_constexpr,
			                               self->a_action.a_act1->a_constexpr);
			if (temp == -1)
				expr_result = NULL;
			else {
				expr_result = DeeBool_For(temp > 0);
				Dee_Incref(expr_result);
			}
			goto action_set_expr_result;
		}
		break;

	case AST_FACTION_BOUNDITEM:
		if (ast_optimize(stack, self->a_action.a_act0, true))
			goto err;
		if (ast_optimize(stack, self->a_action.a_act1, true))
			goto err;
		if (self->a_action.a_act0->a_type == AST_CONSTEXPR &&
		    self->a_action.a_act1->a_type == AST_CONSTEXPR &&
		    DeeString_Check(self->a_action.a_act1->a_constexpr)) {
			int temp = DeeObject_BoundItem(self->a_action.a_act0->a_constexpr,
			                               self->a_action.a_act1->a_constexpr,
			                               true);
			if (temp == -1)
				expr_result = NULL;
			else {
				expr_result = DeeBool_For(temp > 0);
				Dee_Incref(expr_result);
			}
			goto action_set_expr_result;
		}
		break;

	case AST_FACTION_SAMEOBJ:
		if (ast_optimize(stack, self->a_action.a_act0, true))
			goto err;
		if (ast_optimize(stack, self->a_action.a_act1, true))
			goto err;
		if (self->a_action.a_act0->a_type == AST_CONSTEXPR &&
		    self->a_action.a_act1->a_type == AST_CONSTEXPR) {
#ifdef is_builtin_object
			/* Warn if the constant of either branch isn't a builtin object */
			if ((!is_builtin_object(self->a_action.a_act0->a_constexpr) ||
			     !is_builtin_object(self->a_action.a_act1->a_constexpr)) &&
			    warn_idcompare_nonbuiltin(self))
				goto err;
#endif /* is_builtin_object */
			expr_result = DeeBool_For(self->a_action.a_act0->a_constexpr ==
			                          self->a_action.a_act1->a_constexpr);
			Dee_Incref(expr_result);
			goto action_set_expr_result;
		} else if (self->a_action.a_act0->a_type == AST_CONSTEXPR) {
#ifdef is_builtin_object
			/* Warn if the constant of `act0' isn't a builtin object */
			if (!is_builtin_object(self->a_action.a_act0->a_constexpr) &&
			    warn_idcompare_nonbuiltin(self))
				goto err;
#endif /* is_builtin_object */
			if (DeeNone_Check(self->a_action.a_act0->a_constexpr)) {
				/* Optimize: `none === x' -> `x is none' */
				struct ast *temp;
				temp                  = self->a_action.a_act1;
				self->a_action.a_act1 = self->a_action.a_act0;
				self->a_action.a_act0 = temp;
				self->a_flag          = AST_FACTION_IS;
				OPTIMIZE_VERBOSE("Optimize `none === x' -> `x is none'\n");
				goto did_optimize;
			}
		} else if (self->a_action.a_act1->a_type == AST_CONSTEXPR) {
#ifdef is_builtin_object
			/* Warn if the constant of `act1' isn't a builtin object */
			if (!is_builtin_object(self->a_action.a_act1->a_constexpr) &&
			    warn_idcompare_nonbuiltin(self))
				goto err;
#endif /* is_builtin_object */
			if (DeeNone_Check(self->a_action.a_act0->a_constexpr)) {
				/* Optimize: `x === none' -> `x is none' */
				self->a_flag = AST_FACTION_IS;
				OPTIMIZE_VERBOSE("Optimize `x === none' -> `x is none'\n");
				goto did_optimize;
			}
		}
		break;

	case AST_FACTION_DIFFOBJ:
		if (ast_optimize(stack, self->a_action.a_act0, true))
			goto err;
		if (ast_optimize(stack, self->a_action.a_act1, true))
			goto err;
		if (self->a_action.a_act0->a_type == AST_CONSTEXPR &&
		    self->a_action.a_act1->a_type == AST_CONSTEXPR) {
#ifdef is_builtin_object
			/* Warn if the constant of either branch isn't a builtin object */
			if ((!is_builtin_object(self->a_action.a_act0->a_constexpr) ||
			     !is_builtin_object(self->a_action.a_act1->a_constexpr)) &&
			    warn_idcompare_nonbuiltin(self))
				goto err;
#endif /* is_builtin_object */
			expr_result = DeeBool_For(self->a_action.a_act0->a_constexpr !=
			                          self->a_action.a_act1->a_constexpr);
			Dee_Incref(expr_result);
			goto action_set_expr_result;
		} else if (self->a_action.a_act0->a_type == AST_CONSTEXPR) {
#ifdef is_builtin_object
			/* Warn if the constant of `act0' isn't a builtin object */
			if (!is_builtin_object(self->a_action.a_act0->a_constexpr) &&
			    warn_idcompare_nonbuiltin(self))
				goto err;
#endif /* is_builtin_object */
			if (DeeNone_Check(self->a_action.a_act0->a_constexpr)) {
				/* Optimize: `none !== x' -> `x !is none' */
				DREF struct ast *temp;
				temp = ast_setscope_and_ddi(ast_action2(AST_FACTION_IS,
				                                        self->a_action.a_act1,
				                                        self->a_action.a_act0),
				                            self);
				if unlikely(!temp)
					goto err;
				ast_decref_nokill(self->a_action.a_act1);
				ast_decref_nokill(self->a_action.a_act0);
				self->a_bool = temp; /* Inherit reference. */
				self->a_type = AST_BOOL;
				self->a_flag = AST_FBOOL_NEGATE;
				OPTIMIZE_VERBOSE("Optimize `none !== x' -> `x !is none'\n");
				goto did_optimize;
			}
		} else if (self->a_action.a_act1->a_type == AST_CONSTEXPR) {
#ifdef is_builtin_object
			/* Warn if the constant of `act1' isn't a builtin object */
			if (!is_builtin_object(self->a_action.a_act1->a_constexpr) &&
			    warn_idcompare_nonbuiltin(self))
				goto err;
#endif /* is_builtin_object */
			if (DeeNone_Check(self->a_action.a_act0->a_constexpr)) {
				/* Optimize: `x !== none' -> `x !is none' */
				DREF struct ast *temp;
				temp = ast_setscope_and_ddi(ast_action2(AST_FACTION_IS,
				                                        self->a_action.a_act0,
				                                        self->a_action.a_act1),
				                            self);
				if unlikely(!temp)
					goto err;
				ast_decref_nokill(self->a_action.a_act0);
				ast_decref_nokill(self->a_action.a_act1);
				self->a_bool = temp; /* Inherit reference. */
				self->a_type = AST_BOOL;
				self->a_flag = AST_FBOOL_NEGATE;
				OPTIMIZE_VERBOSE("Optimize `x !== none' -> `x !is none'\n");
				goto did_optimize;
			}
		}
		break;

	default:
		/* Default case: optimize individual action branches. */
		switch (AST_FACTION_ARGC_GT(self->a_flag)) {
		case 3:
			if (ast_optimize(stack, self->a_action.a_act2, true))
				goto err;
			ATTR_FALLTHROUGH
		case 2:
			if (ast_optimize(stack, self->a_action.a_act1, true))
				goto err;
			ATTR_FALLTHROUGH
		case 1:
			if (ast_optimize(stack, self->a_action.a_act0, true))
				goto err;
			ATTR_FALLTHROUGH
		default: break;
		}
		break;
	}
/*done:*/
	return 0;
did_optimize:
	++optimizer_count;
	return 0;
err:
	return -1;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_ACTION_C */
