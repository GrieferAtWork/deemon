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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_CONDITIONAL_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_CONDITIONAL_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/tuple.h>

DECL_BEGIN

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL ast_optimize_conditional)(struct ast_optimize_stack *__restrict stack,
                                 struct ast *__restrict self, bool result_used) {
	int constant_condition;
	int tt_value, ff_value;
	ASSERT(self->a_type == AST_CONDITIONAL);
	/* Obviously: optimize sub-branches and do dead-branch elimination. */
	/* TODO: Optimize the inner expressions in regards to
	 *       them only being used as a boolean expression. */
	if (ast_optimize(stack, self->a_conditional.c_cond, true))
		goto err;
	if (self->a_conditional.c_tt == self->a_conditional.c_cond &&
	    self->a_conditional.c_ff == self->a_conditional.c_cond) {
		/* ??? Why though? */
		if (result_used) {
			ast_decref_nokill(self->a_conditional.c_tt);
			ast_decref_nokill(self->a_conditional.c_ff);
			__STATIC_IF(offsetof(struct ast, a_bool) !=
			            offsetof(struct ast, a_conditional.c_cond)) {
				self->a_bool = self->a_conditional.c_cond;
			}
			self->a_type = AST_BOOL;
			self->a_flag = AST_FBOOL_NORMAL;
		} else {
			if (ast_graft_onto(self, self->a_conditional.c_cond))
				goto err;
		}
		OPTIMIZE_VERBOSE("Remove no-op conditional branch\n");
		goto did_optimize;
	}
#ifdef OPTIMIZE_FASSUME
	if (optimizer_flags & OPTIMIZE_FASSUME) {
		/* we're supposed to be making assumptions. */
		bool has_tt = self->a_conditional.c_tt && self->a_conditional.c_tt != self->a_conditional.c_cond;
		bool has_ff = self->a_conditional.c_ff && self->a_conditional.c_ff != self->a_conditional.c_cond;
		struct ast_assumes tt_assumes;
		struct ast_assumes ff_assumes;
		struct ast_optimize_stack child_stack;
		if (has_tt && has_ff) {
			if unlikely(ast_assumes_initcond(&tt_assumes, stack->os_assume))
				goto err;
			child_stack.os_prev   = stack;
			child_stack.os_assume = &tt_assumes;
			child_stack.os_ast    = self->a_conditional.c_tt;
			child_stack.os_used   = result_used;
			if unlikely(ast_optimize(&child_stack, child_stack.os_ast, result_used)) {
err_tt_assumes:
				ast_assumes_fini(&tt_assumes);
				goto err;
			}
			if unlikely(ast_assumes_initcond(&ff_assumes, stack->os_assume))
				goto err_tt_assumes;
			child_stack.os_prev   = stack;
			child_stack.os_assume = &ff_assumes;
			child_stack.os_ast    = self->a_conditional.c_ff;
			child_stack.os_used   = result_used;
			if unlikely(ast_optimize(&child_stack, child_stack.os_ast, result_used)) {
err_ff_tt_assumes:
				ast_assumes_fini(&ff_assumes);
				goto err_tt_assumes;
			}
			/* With true-branch and false-branch assumptions now made, merge them! */
			if unlikely(ast_assumes_mergecond(&tt_assumes, &ff_assumes))
				goto err_ff_tt_assumes;
			ast_assumes_fini(&ff_assumes);
		} else {
			ASSERT(has_tt || has_ff);
			if unlikely(ast_assumes_initcond(&tt_assumes, stack->os_assume))
				goto err;
			child_stack.os_prev   = stack;
			child_stack.os_assume = &tt_assumes;
			child_stack.os_ast    = has_tt
			                     ? self->a_conditional.c_tt
			                     : self->a_conditional.c_ff;
			child_stack.os_used = result_used;
			if unlikely(ast_optimize(&child_stack, child_stack.os_ast, result_used))
				goto err_tt_assumes;
			/* Merge made assumptions with a NULL-branch, behaving
			 * the same as a merge with an empty set of assumptions. */
			if unlikely(ast_assumes_mergecond(&tt_assumes, NULL))
				goto err_tt_assumes;
		}
		/* Finally, merge newly made assumptions onto those made by the caller. */
		if unlikely(ast_assumes_merge(stack->os_assume, &tt_assumes))
			goto err_tt_assumes;
		ast_assumes_fini(&tt_assumes);
	} else
#endif /*  OPTIMIZE_FASSUME*/
	{
		if (self->a_conditional.c_tt &&
		    self->a_conditional.c_tt != self->a_conditional.c_cond &&
		    ast_optimize(stack, self->a_conditional.c_tt, result_used))
			goto err;
		if (self->a_conditional.c_ff &&
		    self->a_conditional.c_ff != self->a_conditional.c_cond &&
		    ast_optimize(stack, self->a_conditional.c_ff, result_used))
			goto err;
	}
	/* Load the constant value of the condition as a boolean. */
	constant_condition = ast_get_boolean(self->a_conditional.c_cond);
	if (constant_condition >= 0) {
		struct ast *eval_branch, *other_branch;
		/* Only evaluate the tt/ff-branch. */
		if (constant_condition) {
			eval_branch  = self->a_conditional.c_tt;
			other_branch = self->a_conditional.c_ff;
		} else {
			eval_branch  = self->a_conditional.c_ff;
			other_branch = self->a_conditional.c_tt;
		}
		/* We can't optimize away the dead branch when it contains a label.
		 * TODO: That's not true. - Just could do an unreachable-pass on the
		 *       other branch that deletes all branches before the first label! */
		if (other_branch && ast_doesnt_return(other_branch, AST_DOESNT_RETURN_FNORMAL) < 0)
			goto after_constant_condition;
		if (!eval_branch) {
			/* No branch is being evaluated. - Just replace the branch with `none' */
			/* TODO: In relation to the `TODO: That's not true...': we'd have to assign
			 *       `{ <everything_after_label_in(other_branch)>; none; }' instead,
			 *       if `other_branch' contains a `label'. */
			ast_fini_contents(self);
			self->a_type      = AST_CONSTEXPR;
			self->a_constexpr = Dee_None;
			Dee_Incref(Dee_None);
		} else if (eval_branch == self->a_conditional.c_cond) {
			/* Special case: The branch that is getting evaluated
			 *               is referencing the condition. */
			/* TODO: In relation to the `TODO: That's not true...': we'd have to assign
			 *       `{ __stack local _temp = <eval_branch>; <everything_after_label_in(other_branch)>; _temp; }'
			 *       instead, if `other_branch' contains a `label'. */
			if (self->a_flag & AST_FCOND_BOOL) {
				/* Must also convert `eval_branch' to a boolean. */
				DREF struct ast *graft;
				int temp;
				graft = ast_setscope_and_ddi(ast_bool(AST_FBOOL_NORMAL, eval_branch), self);
				if unlikely(!graft)
					goto err;
				temp = ast_graft_onto(self, graft);
				ast_decref(graft);
				if unlikely(temp)
					goto err;
			} else {
				if (ast_graft_onto(self, eval_branch))
					goto err;
			}
		} else {
			DREF struct ast **elemv;
			/* Merge the eval-branch and the condition:
			 * >> if (true) {
			 * >>     print "Here";
			 * >> }
			 * Optimize to:
			 * >> {
			 * >>     true;
			 * >>     print "Here";
			 * >> }
			 */
			/* TODO: In relation to the `TODO: That's not true...': we'd have to assign
			 *       `{ <l_cond>; __stack local _temp = <eval_branch>;
			 *          <everything_after_label_in(other_branch)>; _temp; }'
			 *       instead, if `other_branch' contains a `label'. */
			elemv = (DREF struct ast **)Dee_Mallocc(2, sizeof(DREF struct ast *));
			if unlikely(!elemv)
				goto err;
			elemv[0] = self->a_conditional.c_cond;
			/* Cast the branch being evaluated to a boolean if required. */
			if (self->a_flag & AST_FCOND_BOOL) {
				DREF struct ast *merge;
				merge = ast_setscope_and_ddi(ast_bool(AST_FBOOL_NORMAL, eval_branch), self);
				if unlikely(!merge) {
					Dee_Free(elemv);
					goto err;
				}
				ast_decref(eval_branch);
				eval_branch = merge;
			}
			elemv[1] = eval_branch; /* Inherit reference */
			/* Override (and inherit) this AST. */
			self->a_type            = AST_MULTIPLE;
			self->a_flag            = AST_FMULTIPLE_KEEPLAST;
			self->a_multiple.m_astc = 2;
			self->a_multiple.m_astv = elemv;
			ast_xdecref(other_branch);
		}
		OPTIMIZE_VERBOSE("Expanding conditional branch with constant condition\n");
		goto did_optimize;
	}
after_constant_condition:
	if (self->a_flag & AST_FCOND_BOOL) {
		tt_value = -2;
		ff_value = -2;
		if (self->a_conditional.c_tt == self->a_conditional.c_cond) {
			ff_value = self->a_conditional.c_ff
			           ? ast_get_boolean_noeffect(self->a_conditional.c_ff)
			           : 0;
			if (ff_value >= 0) {
				if (ff_value && result_used) {
					/* Optimize: `!!foo() ? : true' --> `({ foo(); true; })' */
					DREF struct ast **elemv;
					ASSERT(self->a_conditional.c_ff);
					elemv = (DREF struct ast **)Dee_Mallocc(2, sizeof(DREF struct ast *));
					if unlikely(!elemv)
						goto err;
					elemv[0] = self->a_conditional.c_cond; /* Inherit reference. */
					elemv[1] = self->a_conditional.c_ff;   /* Inherit reference. */
					ast_decref_nokill(self->a_conditional.c_tt);
					self->a_multiple.m_astc = 2;
					self->a_multiple.m_astv = elemv;
					self->a_type            = AST_MULTIPLE;
					self->a_flag            = AST_FMULTIPLE_KEEPLAST;
				} else if (result_used) {
					/* Optimize: `!!foo() ? : false' --> `!!foo();' */
					ast_decref_nokill(self->a_conditional.c_tt);
					ast_xdecref(self->a_conditional.c_ff);
					__STATIC_IF(offsetof(struct ast, a_bool) !=
					            offsetof(struct ast, a_conditional.c_cond)) {
						self->a_bool = self->a_conditional.c_cond;
					}
					self->a_type = AST_BOOL;
					self->a_flag = AST_FBOOL_NORMAL;
				} else {
					/* Optimize: `!!foo() ? : false' --> `foo();' */
					if (ast_graft_onto(self, self->a_conditional.c_cond))
						goto err;
				}
				OPTIMIZE_VERBOSE("Flatten constant false-branch of boolean conditional tt-is-cond expression\n");
				goto did_optimize;
			}
		}
		if (self->a_conditional.c_ff == self->a_conditional.c_cond) {
			tt_value = self->a_conditional.c_tt
			           ? ast_get_boolean_noeffect(self->a_conditional.c_tt)
			           : 0;
			if (tt_value >= 0) {
				if (!tt_value && result_used) {
					/* Optimize: `!!foo() ? false : ' --> `({ foo(); false; })' */
					DREF struct ast **elemv;
					ASSERT(self->a_conditional.c_tt);
					elemv = (DREF struct ast **)Dee_Mallocc(2, sizeof(DREF struct ast *));
					if unlikely(!elemv)
						goto err;
					elemv[0] = self->a_conditional.c_cond; /* Inherit reference. */
					elemv[1] = self->a_conditional.c_tt;   /* Inherit reference. */
					ast_decref_nokill(self->a_conditional.c_ff);
					self->a_multiple.m_astc = 2;
					self->a_multiple.m_astv = elemv;
					self->a_type            = AST_MULTIPLE;
					self->a_flag            = AST_FMULTIPLE_KEEPLAST;
				} else if (result_used) {
					/* Optimize: `!!foo() ? true : ' --> `!!foo();' */
					ast_decref_nokill(self->a_conditional.c_ff);
					ast_xdecref(self->a_conditional.c_tt);
					__STATIC_IF(offsetof(struct ast, a_bool) !=
					            offsetof(struct ast, a_conditional.c_cond)) {
						self->a_bool = self->a_conditional.c_cond;
					}
					self->a_type = AST_BOOL;
					self->a_flag = AST_FBOOL_NORMAL;
				} else {
					/* Optimize: `!!foo() ? true : ' --> `foo();' */
					if (ast_graft_onto(self, self->a_conditional.c_cond))
						goto err;
				}
				OPTIMIZE_VERBOSE("Flatten constant true-branch of boolean conditional ff-is-cond expression\n");
				goto did_optimize;
			}
		}
		if (tt_value == -2) {
			tt_value = self->a_conditional.c_tt
			           ? ast_get_boolean_noeffect(self->a_conditional.c_tt)
			           : 0;
		}
		if (ff_value == -2) {
			ff_value = self->a_conditional.c_ff
			           ? ast_get_boolean_noeffect(self->a_conditional.c_ff)
			           : 0;
		}
		if (tt_value >= 0 && ff_value >= 0) {
			/* In a boolean context, both branches can be predicted at compile-time,
			 * meaning we can optimize using a matrix:
			 * >>  cond ? false : false --> ({ cond; false; })
			 * >>  cond ? false : true  --> !cond
			 * >>  cond ? true : false  --> !!cond
			 * >>  cond ? true : true   --> ({ cond; true; })
			 */
apply_bool_matrix_transformation:
			if (tt_value) {
				if (ff_value) {
					/* cond ? true : true  --> ({ cond; true; }) */
					DREF struct ast **elemv;
optimize_conditional_bool_predictable_inherit_multiple:
					elemv = (DREF struct ast **)Dee_Mallocc(2, sizeof(DREF struct ast *));
					if unlikely(!elemv)
						goto err;
					elemv[0] = self->a_conditional.c_cond; /* Inherit reference. */
					elemv[1] = self->a_conditional.c_tt;   /* Inherit reference. */
					ast_decref(self->a_conditional.c_ff);
					self->a_multiple.m_astc = 2;
					self->a_multiple.m_astv = elemv;
					self->a_type            = AST_MULTIPLE;
					self->a_flag            = AST_FMULTIPLE_KEEPLAST;
				} else {
					/* cond ? true : false  --> !!cond */
					ast_decref(self->a_conditional.c_tt);
					ast_decref(self->a_conditional.c_ff);
					__STATIC_IF(offsetof(struct ast, a_bool) !=
					            offsetof(struct ast, a_conditional.c_cond)) {
						self->a_bool = self->a_conditional.c_cond;
					}
					self->a_type = AST_BOOL;
					self->a_flag = AST_FBOOL_NORMAL;
				}
			} else {
				if (!ff_value) {
					/* cond ? false : false --> ({ cond; false; }) */
					goto optimize_conditional_bool_predictable_inherit_multiple;
				} else {
					/* cond ? false : true  --> !cond */
					ast_decref(self->a_conditional.c_tt);
					ast_decref(self->a_conditional.c_ff);
					__STATIC_IF(offsetof(struct ast, a_bool) !=
					            offsetof(struct ast, a_conditional.c_cond)) {
						self->a_bool = self->a_conditional.c_cond;
					}
					self->a_type = AST_BOOL;
					self->a_flag = AST_FBOOL_NEGATE;
				}
			}
			OPTIMIZE_VERBOSE("Apply matrix to constant tt/ff branches of boolean conditional expression\n");
			goto did_optimize;
		}
	}
	if (!result_used) {
		/* Remove branches without any side-effects. */
		if (self->a_conditional.c_tt &&
		    self->a_conditional.c_tt != self->a_conditional.c_cond &&
		    !ast_has_sideeffects(self->a_conditional.c_tt)) {
			if (self->a_conditional.c_ff) {
				ast_decref(self->a_conditional.c_tt);
				self->a_conditional.c_tt = NULL;
			} else {
				if (ast_graft_onto(self, self->a_conditional.c_cond))
					goto err;
			}
			OPTIMIZE_VERBOSE("Remove conditional tt-branch without any side-effects\n");
			goto did_optimize;
		}
		if (self->a_conditional.c_ff &&
		    self->a_conditional.c_ff != self->a_conditional.c_cond &&
		    !ast_has_sideeffects(self->a_conditional.c_ff)) {
			if (self->a_conditional.c_tt) {
				ast_decref(self->a_conditional.c_ff);
				self->a_conditional.c_ff = NULL;
			} else {
				if (ast_graft_onto(self, self->a_conditional.c_cond))
					goto err;
			}
			OPTIMIZE_VERBOSE("Remove conditional ff-branch without any side-effects\n");
			goto did_optimize;
		}
	}

	if (self->a_conditional.c_tt && self->a_conditional.c_ff) {
		struct ast *tt = self->a_conditional.c_tt;
		struct ast *ff = self->a_conditional.c_ff;
		/* TODO: if the current context only needs a boolean value,
		 *       optimize constant expressions into Dee_True / Dee_False */
		if (tt->a_type == AST_CONSTEXPR && ff->a_type == AST_CONSTEXPR) {
			/* Optimize when both branches are in use and both are constant expressions:
			 * >> get_cond() ? 10 : 20;
			 * Optimize to:
			 * >> pack(10, 20)[!!get_cond()]; */
			DREF DeeObject *argument_packet;
			/* Special case: If both values are boolean, we can apply a bool-matrix. */
			if (DeeBool_Check(tt->a_constexpr) &&
			    DeeBool_Check(ff->a_constexpr)) {
				/* TODO: Even without this special case, the optimization
				 *       for `operator []' should also be able to get
				 *       rid of this! */
				tt_value = DeeBool_IsTrue(tt->a_constexpr);
				ff_value = DeeBool_IsTrue(ff->a_constexpr);
				goto apply_bool_matrix_transformation;
			}
			argument_packet = DeeTuple_Pack(2,
			                                ff->a_constexpr,
			                                tt->a_constexpr);
			if unlikely(!argument_packet)
				goto err;
			Dee_Decref(tt->a_constexpr);
			Dee_Decref(ff->a_constexpr);
			tt->a_constexpr           = argument_packet; /* Inherit reference. */
			ff->a_type                = AST_BOOL;
			ff->a_flag                = AST_FBOOL_NORMAL;
			ff->a_bool                = self->a_conditional.c_cond;   /* Inherit reference. */
			self->a_operator.o_op0    = tt;                           /* Inherit reference. */
			self->a_operator.o_op1    = ast_setddi(ff, &self->a_ddi); /* Inherit reference. */
			self->a_operator.o_op2    = NULL;
			self->a_operator.o_op3    = NULL;
			self->a_operator.o_exflag = AST_OPERATOR_FNORMAL;
			self->a_flag              = OPERATOR_GETITEM;
			self->a_type              = AST_OPERATOR;
			OPTIMIZE_VERBOSE("Use result-vectoring to reduce constant tt/ff branches of conditional expression\n");
			goto did_optimize;
		}
		if (optimizer_flags & OPTIMIZE_FCSE) {
			if (ast_equal(tt, ff)) {
				/* The true and false branches are identical. */
				DREF struct ast **elemv;
if_statement_branches_identical:
				elemv = (DREF struct ast **)Dee_Mallocc(2, sizeof(DREF struct ast *));
				if unlikely(!elemv)
					goto err;
				ast_fini_contents(tt);
				tt->a_type              = AST_BOOL;
				tt->a_flag              = AST_FBOOL_NORMAL;
				tt->a_bool              = self->a_conditional.c_cond; /* Inherit reference. */
				elemv[0]                = tt;                         /* Inherit reference. */
				elemv[1]                = ff;                         /* Inherit reference. */
				self->a_multiple.m_astc = 2;
				self->a_multiple.m_astv = elemv;
				self->a_type            = AST_MULTIPLE;
				self->a_flag            = AST_FMULTIPLE_KEEPLAST;
				OPTIMIZE_VERBOSE("Flatten identical true/false branches\n");
				goto did_optimize;
			}
			/* Optimize stuff like this:
			 * >> if (foo()) {
			 * >>     print "Begin";
			 * >>     DoTheThing();
			 * >>     print "End";
			 * >> } else {
			 * >>     print "Begin";
			 * >>     DoTheOtherThing();
			 * >>     print "End";
			 * >> }
			 * Into:
			 * >> print "Begin";
			 * >> if (foo()) {
			 * >>     DoTheThing();
			 * >> } else {
			 * >>     DoTheOtherThing();
			 * >> }
			 * >> print "End";
			 */
			{
				struct ast **tt_astv;
				size_t tt_astc;
				struct ast **ff_astv;
				size_t ff_astc;
				if (tt->a_type == AST_MULTIPLE &&
				    tt->a_flag == AST_FMULTIPLE_KEEPLAST) {
					tt_astv = tt->a_multiple.m_astv;
					tt_astc = tt->a_multiple.m_astc;
				} else {
					tt_astv = &tt;
					tt_astc = 1;
				}
				if (ff->a_type == AST_MULTIPLE &&
				    ff->a_flag == AST_FMULTIPLE_KEEPLAST) {
					ff_astv = ff->a_multiple.m_astv;
					ff_astc = ff->a_multiple.m_astc;
				} else {
					ff_astv = &ff;
					ff_astc = 1;
				}
				size_t move_count = 0;
				while (move_count < tt_astc &&
				       move_count < ff_astc) {
					struct ast *tt_last = tt_astv[tt_astc - (move_count + 1)];
					struct ast *ff_last = ff_astv[ff_astc - (move_count + 1)];
					if (!ast_equal(tt_last, ff_last))
						break;
					++move_count;
				}
				if (move_count) {
					if unlikely(!tt_astc && !ff_astc)
						goto if_statement_branches_identical;
					if (!tt_astc) {
						/* Only the false-branches remain. */
						/* TODO */
					}
					if (!ff_astc) {
						/* Only the true-branches remain. */
						/* TODO */
					}
				}
			}
		}
	}
	return 0;
err:
	return -1;
did_optimize:
	++optimizer_count;
	return 0;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_CONDITIONAL_C */
