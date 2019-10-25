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
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/object.h>

DECL_BEGIN


INTERN int
(DCALL ast_optimize_loop)(struct ast_optimize_stack *__restrict stack,
                          struct ast *__restrict self, bool result_used) {
	(void)result_used;
	if (optimizer_unwind_limit != 0) {
		/* TODO: Loop unwinding when `self->a_loop.l_iter'
		 *       evaluates to a constant expression.
		 * >> for (local x: [:3])
		 * >>      print x;
		 * Optimize to:
		 * >> print 0;
		 * >> print 1;
		 * >> print 2;
		 * Optimize to:
		 * >> print "0";
		 * >> print "1";
		 * >> print "2";
		 * Optimize to:
		 * >> print "0\n",;
		 * >> print "1\n",;
		 * >> print "2\n",;
		 * Optimize to:
		 * >> print "0\n1\n2\n",;
		 */
	}

	/* Optimize the inner branches. */
#ifdef OPTIMIZE_FASSUME
	if (optimizer_flags & OPTIMIZE_FASSUME) {
		struct ast_assumes lookahead_assumptions;
		struct ast_assumes entry_assumptions;
		struct ast_optimize_stack child_stack;
		if unlikely(ast_assumes_initcond(&lookahead_assumptions, stack->os_assume))
			goto err;
		/* Build up assumptions as they will appear after the loop. */
		if (self->a_flag & AST_FLOOP_FOREACH) {
			/* foreach-style loop. */
			if (ast_assumes_gather(&lookahead_assumptions, self->a_loop.l_iter, true)) {
err_lookahead_assumptions:
				ast_assumes_fini(&lookahead_assumptions);
				goto err;
			}
			if (self->a_loop.l_elem &&
			    ast_assumes_gather(&lookahead_assumptions, self->a_loop.l_elem, true))
				goto err_lookahead_assumptions;
			if (self->a_loop.l_loop &&
			    ast_assumes_gather(&lookahead_assumptions, self->a_loop.l_loop, false))
				goto err_lookahead_assumptions;
		} else {
			if (!(self->a_flag & AST_FLOOP_POSTCOND)) {
				if (self->a_loop.l_cond &&
				    ast_assumes_gather(&lookahead_assumptions, self->a_loop.l_cond, true))
					goto err_lookahead_assumptions;
			}
			if (self->a_loop.l_loop &&
			    ast_assumes_gather(&lookahead_assumptions, self->a_loop.l_loop, false))
				goto err_lookahead_assumptions;
			if (self->a_loop.l_next &&
			    ast_assumes_gather(&lookahead_assumptions, self->a_loop.l_next, false))
				goto err_lookahead_assumptions;
			if (self->a_flag & AST_FLOOP_POSTCOND) {
				if (self->a_loop.l_cond &&
				    ast_assumes_gather(&lookahead_assumptions, self->a_loop.l_cond, true))
					goto err_lookahead_assumptions;
			}
		}
		/* Delete any assumptions that wouldn't have been made if the loop isn't run at all. */
		if ((self->a_flag & (AST_FLOOP_FOREACH | AST_FLOOP_POSTCOND)) != AST_FLOOP_POSTCOND &&
		    ast_assumes_mergecond(&lookahead_assumptions, NULL))
			goto err_lookahead_assumptions;
		if unlikely(ast_assumes_initcond(&entry_assumptions, stack->os_assume))
			goto err_lookahead_assumptions;
		/* Merge assumptions upon entry (0 iterations) with those made up exit (>= 1 iterations) */
		if unlikely(ast_assumes_mergecond(&entry_assumptions, &lookahead_assumptions)) {
			ast_assumes_fini(&entry_assumptions);
			goto err_lookahead_assumptions;
		}
		ast_assumes_fini(&lookahead_assumptions);
		/* Now, using the common set of assumptions found at the start and
		 * end of the loop, perform optimizations within the loop itself! */

		child_stack.os_prev   = stack;
		child_stack.os_assume = &entry_assumptions;
		if (self->a_flag & AST_FLOOP_FOREACH) {
			child_stack.os_ast  = self->a_loop.l_iter;
			child_stack.os_used = true;
			if (ast_optimize(&child_stack, self->a_loop.l_iter, true)) {
err_entry_assumptions:
				ast_assumes_fini(&entry_assumptions);
				goto err;
			}
			if (self->a_loop.l_elem &&
			    ast_optimize(&child_stack, self->a_loop.l_elem, true))
				goto err_entry_assumptions;
			if (self->a_loop.l_loop &&
			    ast_optimize(&child_stack, self->a_loop.l_loop, false))
				goto err_entry_assumptions;
		} else {
			if (!(self->a_flag & AST_FLOOP_POSTCOND)) {
				if (self->a_loop.l_cond &&
				    ast_optimize(&child_stack, self->a_loop.l_cond, true))
					goto err_entry_assumptions;
			}
			if (self->a_loop.l_loop &&
			    ast_optimize(&child_stack, self->a_loop.l_loop, false))
				goto err_entry_assumptions;
			if (self->a_loop.l_next &&
			    ast_optimize(&child_stack, self->a_loop.l_next, false))
				goto err_entry_assumptions;
			if (self->a_flag & AST_FLOOP_POSTCOND) {
				if (self->a_loop.l_cond &&
				    ast_optimize(&child_stack, self->a_loop.l_cond, true))
					goto err_entry_assumptions;
			}
		}
		/* Merge the final set of assumptions with those done by the caller. */
		if unlikely(ast_assumes_merge(stack->os_assume, &entry_assumptions))
			goto err_entry_assumptions;
		ast_assumes_fini(&entry_assumptions);
	} else
#endif /* OPTIMIZE_FASSUME */
	{
		if (self->a_loop.l_cond &&
		    ast_optimize(stack, self->a_loop.l_cond, true))
			goto err;
		if (self->a_loop.l_next &&
		    ast_optimize(stack, self->a_loop.l_next, (self->a_flag & AST_FLOOP_FOREACH) != 0))
			goto err;
		if (self->a_loop.l_loop &&
		    ast_optimize(stack, self->a_loop.l_loop, false))
			goto err;
	}


	if (self->a_flag & AST_FLOOP_FOREACH) {
		/* foreach-style loop. */
	} else if (!(self->a_flag & AST_FLOOP_POSTCOND)) {
		if (self->a_loop.l_cond) {
			/* TODO: Do this optimization in regards to the current, known state of variables. */
			int condition_value;
			/* Optimize constant conditions. */
			condition_value = ast_get_boolean(self->a_loop.l_cond);
			if (condition_value > 0) {
				/* Forever-loop (Discard the condition). */
				ast_decref(self->a_loop.l_cond);
				self->a_loop.l_cond = NULL;
				OPTIMIZE_VERBOSE("Removing constant-true loop condition\n");
				++optimizer_count;
			} else if (condition_value == 0) {
#if 0 /* TODO: This doesn't properly handle labels! */
				/* Unused loop:
				 * >> while (0) { ... } */
				DREF struct ast **elemv, *none_ast;
				elemv = (DREF struct ast **)Dee_Malloc(2 * sizeof(DREF struct ast *));
				if unlikely(!elemv)
					goto err;
				none_ast = ast_setscope_and_ddi(ast_constexpr(Dee_None), self);
				if unlikely(!none_ast) {
					Dee_Free(elemv);
					goto err;
				}
				elemv[0] = self->a_loop.l_cond; /* Inherit */
				elemv[1] = none_ast;            /* Inherit */
				/* Convert into a multiple-ast. */
				self->ast_multiple.m_astc = 2;
				self->a_type              = AST_MULTIPLE;
				self->a_flag              = AST_FMULTIPLE_KEEPLAST;
				self->ast_multiple.m_astv = elemv; /* Inherit */
				OPTIMIZE_VERBOSE("Deleting loop never executed\n");
				goto did_optimize;
#endif
			}
		}
		/* TODO: If the condition is known to be true during the first
		 *       pass, convert the loop to a post-conditional loop:
		 * >> {
		 * >>     local i = 0;
		 * >>     for (; i < 10; ++i)
		 * >>         print i;
		 * >> }
		 * Optimize to:
		 * >> {
		 * >>     local i = 0;
		 * >>     do {
		 * >>         print i;
		 * >>         ++i;
		 * >>     } while (i < 10);
		 * >> }
		 */
		/* TODO: Convert to a foreach-style loop (potentially allowing
		 *       for further optimization through loop unwinding):
		 * >> {
		 * >>     local i = 0;
		 * >>     do {
		 * >>         print i;
		 * >>         ++i;
		 * >>     } while (i < 10);
		 * >> }
		 * Optimize to:
		 * >> for (local i: [0:10,1])
		 * >>     print i;
		 */
	} else {
		if (self->a_loop.l_cond) {
			/* TODO: Do this optimization in regards to the current, known state of variables. */
			int condition_value;
			/* Optimize constant conditions. */
			condition_value = ast_get_boolean(self->a_loop.l_cond);
			if (condition_value > 0) {
				/* Forever-loop (Discard the condition). */
				ast_decref(self->a_loop.l_cond);
				self->a_loop.l_cond = NULL;
				OPTIMIZE_VERBOSE("Removing constant-true loop condition\n");
				++optimizer_count;
			} else if (condition_value == 0) {
#if 0 /* TODO: This doesn't properly handle labels! \
       * TODO: This can only be done when no loop control statements were used. */
				/* Unused loop:
				 * >> do { ... } while (0); */
				/* Convert to `{ <loop>; <next>; <cond>; none; }' */
				DREF struct ast **elemv, *none_ast, **iter;
				elemv = (DREF struct ast **)Dee_Malloc(4 * sizeof(DREF struct ast *));
				if unlikely(!elemv)
					goto err;
				none_ast = ast_setscope_and_ddi(ast_constexpr(Dee_None), self);
				if unlikely(!none_ast) {
					Dee_Free(elemv);
					goto err;
				}
				iter    = elemv;
				*iter++ = self->a_loop.l_loop; /* Inherit */
				if (self->a_loop.l_next)
					*iter++ = self->a_loop.l_next; /* Inherit */
				*iter++ = self->a_loop.l_cond;     /* Inherit */
				*iter++ = none_ast;                /* Inherit */
				/* Convert into a multiple-ast. */
				self->ast_multiple.m_astc = (size_t)(iter - elemv);
				self->a_type              = AST_MULTIPLE;
				self->a_flag              = AST_FMULTIPLE_KEEPLAST;
				self->ast_multiple.m_astv = elemv; /* Inherit */
				OPTIMIZE_VERBOSE("Unwinding loop only iterated once\n");
				goto did_optimize;
#endif
			}
		}
	}
/*done:*/
	return 0;
/*
did_optimize:
 ++optimizer_count;
 return 0;
*/
err:
	return -1;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_ACTION_C */
