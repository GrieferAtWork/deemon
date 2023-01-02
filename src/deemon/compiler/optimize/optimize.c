/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_OPTIMIZE_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_OPTIMIZE_C 1

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/callable.h>
#include <deemon/cell.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/dec.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/float.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/numeric.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/tuple.h>

#include <stdarg.h>

DECL_BEGIN

INTERN uint16_t optimizer_flags        = OPTIMIZE_FDISABLED;
INTERN uint16_t optimizer_unwind_limit = 0;
INTERN unsigned int optimizer_count    = 0;

#ifdef CONFIG_HAVE_OPTIMIZE_VERBOSE
INTERN NONNULL((1, 2)) void
ast_optimize_verbose(struct ast *__restrict self, char const *format, ...) {
	va_list args;
	va_start(args, format);
	if (self->a_ddi.l_file) {
		Dee_DPRINTF("%s(%d,%d) : ",
		            TPPFile_Filename(self->a_ddi.l_file, NULL),
		            self->a_ddi.l_line + 1,
		            self->a_ddi.l_col + 1);
	}
	Dee_VDPRINTF(format, args);
	va_end(args);
}
#endif /* CONFIG_HAVE_OPTIMIZE_VERBOSE */




INTERN WUNUSED NONNULL((1, 2)) int
(DCALL ast_optimize)(struct ast_optimize_stack *__restrict parent,
                     struct ast *__restrict self, bool result_used) {
	struct ast_optimize_stack stack;
	stack.os_prev = parent;
	stack.os_ast  = self;
	stack.os_used = result_used;
#ifdef OPTIMIZE_FASSUME
	stack.os_assume = parent->os_assume;
#endif /* OPTIMIZE_FASSUME */
	return ast_dooptimize(&stack, self, result_used);
}

INTERN WUNUSED NONNULL((1)) int
(DCALL ast_startoptimize)(struct ast *__restrict self, bool result_used) {
	struct ast_optimize_stack stack;
	int result;
#ifdef OPTIMIZE_FASSUME
	struct ast_assumes assumes;
#endif /* OPTIMIZE_FASSUME */
	stack.os_prev = NULL;
	stack.os_ast  = self;
	stack.os_used = result_used;
#ifdef OPTIMIZE_FASSUME
	stack.os_assume = &assumes;
	ast_assumes_init(&assumes);
#endif /* OPTIMIZE_FASSUME */
	result = ast_dooptimize(&stack, self, result_used);
#ifdef OPTIMIZE_FASSUME
	ast_assumes_fini(&assumes);
#endif /* OPTIMIZE_FASSUME */
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL ast_dooptimize)(struct ast_optimize_stack *__restrict stack,
                       struct ast *__restrict self, bool result_used) {
	ASSERT_AST(self);
	ASSERT(self->a_scope->s_del != (struct symbol *)1);
again:
	ASSERT(self->a_scope->s_del != (struct symbol *)1);

	/* Check for interrupts to allow the user to stop optimization */
	if (DeeThread_CheckInterrupt())
		goto err;
	while (self->a_scope->s_prev &&                    /* Has parent scope */
	       self->a_scope->s_prev->ob_type ==           /* Parent scope has the same type */
	       self->a_scope->ob_type &&                   /* ... */
	       self->a_scope->ob_type == &DeeScope_Type && /* It's a regular scope */
	       !self->a_scope->s_mapc &&                   /* Current scope has no symbols */
	       !self->a_scope->s_del) {                    /* Current scope has no deleted symbol */
		/* Use the parent's scope. */
		DeeScopeObject *new_scope;
		new_scope = self->a_scope->s_prev;
		Dee_Incref(new_scope);
		Dee_Decref(self->a_scope);
		self->a_scope = new_scope;
		++optimizer_count;
#if 0 /* This happens so often, logging it would just be spam... */
		OPTIMIZE_VERBOSEAT(self, "Inherit parent scope above empty child scope\n");
#endif
	}
	/* TODO: Remove unused symbols from scopes (s.a. `s_nread == 0 && s_nwrite == 0 && s_nbound == 0') */

	/* TODO: Move variables declared in outer scopes, but only used in inner ones
	 *       into those inner scopes, thus improving local-variable-reuse optimizations
	 *       later done the line. */
	switch (self->a_type) {

	case AST_SYM:
		return ast_optimize_symbol(stack, self, result_used);

	case AST_MULTIPLE:
		return ast_optimize_multiple(stack, self, result_used);

	case AST_UNBIND:
#ifdef OPTIMIZE_FASSUME
		/* Delete assumptions made about the symbol. */
		if (optimizer_flags & OPTIMIZE_FASSUME)
			return ast_assumes_setsymval(stack->os_assume, self->a_unbind, NULL);
#endif /* OPTIMIZE_FASSUME */
		/* TODO: Remove this branch if the symbol is never read from, or checking for being bound
		 *       NOTE: Only do so when `symbol_del_haseffect()' is false */
		break;

	case AST_BOUND:
		/* TODO: Always `false' for `local-symbols' with `s_nwrite == 0' */
		/* TODO: Using assumptions, we could also track if a symbol if a symbol is bound? */
		break;

	case AST_YIELD:
		/* TODO: `yield { foo }...' -> `yield foo' */
	case AST_RETURN:
	case AST_THROW:
		if (self->a_return &&
		    ast_optimize(stack, self->a_return, true))
			goto err;
		break;

	case AST_TRY:
		return ast_optimize_try(stack, self, result_used);

	case AST_LOOP:
		return ast_optimize_loop(stack, self, result_used);

	case AST_CONDITIONAL:
		return ast_optimize_conditional(stack, self, result_used);

	case AST_BOOL: {
		int ast_value;
		DREF struct ast **elemv;
		/* TODO: Optimize the inner expression in regards to
		 *       it only being used as a boolean expression */
		if (ast_optimize(stack, self->a_bool, result_used))
			goto err;
		/* If the result doesn't matter, don't perform a negation. */
		if (!result_used)
			self->a_flag &= ~AST_FBOOL_NEGATE;

		/* Figure out if the branch's value is known at compile-time. */
		ast_value = ast_get_boolean(self->a_bool);
		if (ast_value >= 0) {
			if (self->a_flag & AST_FBOOL_NEGATE)
				ast_value = !ast_value;
			if (ast_has_sideeffects(self->a_bool)) {
				/* Replace this branch with `{ ...; true/false; }' */
				elemv = (DREF struct ast **)Dee_Malloc(2 * sizeof(DREF struct ast *));
				if unlikely(!elemv)
					goto err;
				elemv[0] = self->a_bool;
				elemv[1] = ast_setscope_and_ddi(ast_constexpr(DeeBool_For(ast_value)), self);
				if unlikely(!elemv[1]) {
					Dee_Free(elemv);
					goto err;
				}
				self->a_type            = AST_MULTIPLE;
				self->a_flag            = AST_FMULTIPLE_KEEPLAST;
				self->a_multiple.m_astc = 2;
				self->a_multiple.m_astv = elemv;
			} else {
				/* Without side-effects, there is no need to keep the expression around. */
				ast_decref_likely(self->a_bool);
				self->a_constexpr = DeeBool_For(ast_value);
				Dee_Incref(self->a_constexpr);
				self->a_type = AST_CONSTEXPR;
				self->a_flag = AST_FNORMAL;
			}
			OPTIMIZE_VERBOSE("Propagating constant boolean expression\n");
			goto did_optimize;
		}
		if (ast_predict_type(self->a_bool) == &DeeBool_Type) {
			if (self->a_flag & AST_FBOOL_NEGATE) {
				/* TODO: Search for what is making the inner expression
				 *       a boolean and try to apply our negation to it:
				 * >> x = !({ !foo(); }); // Apply our negation to the inner expression.
				 */
			} else {
				/* Optimize away the unnecessary bool-cast */
				if (ast_graft_onto(self, self->a_bool))
					goto err;
				OPTIMIZE_VERBOSE("Remove unused bool cast\n");
				goto did_optimize;
			}
		}
	}	break;

	case AST_EXPAND:
		if (ast_optimize(stack, self->a_expand, result_used))
			goto err;
		break;

	case AST_FUNCTION:
#ifdef OPTIMIZE_FASSUME
		if (optimizer_flags & OPTIMIZE_FASSUME) {
			struct ast_assumes child_assumes;
			struct ast_optimize_stack child_stack;
			/* Assumptions made in functions shouldn't bleed into the outside,
			 * so we use a sub-set of assumption within the function. */
			if (ast_assumes_initfunction(&child_assumes, stack->os_assume))
				goto err;
			child_stack.os_assume = &child_assumes;
			child_stack.os_ast    = self->a_function.f_code;
			child_stack.os_prev   = stack;
			child_stack.os_used   = false;
			if (ast_optimize(&child_stack, child_stack.os_ast, false)) {
				ast_assumes_fini(&child_assumes);
				goto err;
			}
			/* Don't let child assumptions bleed to the outside. - Assumptions
			 * made in the function are discarded once we're back on the outside. */
			ast_assumes_fini(&child_assumes);
		} else
#endif /* OPTIMIZE_FASSUME */
		{
			if (ast_optimize(stack, self->a_function.f_code, false))
				goto err;
		}
		if (!DeeCompiler_Current->cp_options ||
		    !(DeeCompiler_Current->cp_options->co_assembler & ASM_FNODEC)) {
			/* TODO: Replace initializers of function default-arguments that
			 *       are never actually used by the function with `none'
			 *    -> That way, we can reduce the size of a produced DEC file.
			 * NOTE: Only enable this optimization when a DEC file is being
			 *       generated, as it doesn't affect runtime performance in
			 *       direct source->assembly execution mode, and only slows
			 *       down compilation then. */
		}
		break;

	case AST_LABEL:
		/* Special label-branch. */
		ASSERT(self->a_label.l_label);
		if (!self->a_label.l_label->tl_goto) {
			/* The label was never used. - Ergo: it should not exist.
			 * To signify this, we simply convert this branch to `none'. */
			if (WARNAST(self, W_ASM_LABEL_NEVER_USED,
			            self->a_flag & AST_FLABEL_CASE ? (self->a_label.l_label->tl_expr ? "case" : "default") : self->a_label.l_label->tl_name->k_name))
				goto err;
			Dee_Decref((DeeObject *)self->a_label.l_base);
			self->a_type      = AST_CONSTEXPR;
			self->a_flag      = AST_FNORMAL;
			self->a_constexpr = Dee_None;
			Dee_Incref(Dee_None);
			OPTIMIZE_VERBOSE("Removing unused label\n");
			goto did_optimize;
		}
#ifdef OPTIMIZE_FASSUME
		if (optimizer_flags & OPTIMIZE_FASSUME &&
		    ast_assumes_undefined_all(stack->os_assume))
			goto err;
#endif /* OPTIMIZE_FASSUME */
		break;

	case AST_OPERATOR:
		return ast_optimize_operator(stack, self, result_used);

	case AST_ACTION:
		return ast_optimize_action(stack, self, result_used);

	case AST_SWITCH:
		return ast_optimize_switch(stack, self, result_used);

	case AST_CLASS: {
		size_t i;
		if (self->a_class.c_base &&
		    ast_optimize(stack, self->a_class.c_base, true))
			goto err;
		if (ast_optimize(stack, self->a_class.c_desc, true))
			goto err;
		for (i = 0; i < self->a_class.c_memberc; ++i) {
			if (ast_optimize(stack, self->a_class.c_memberv[i].cm_ast, true))
				goto err;
		}
	}	break;

	case AST_ASSEMBLY: {
		struct asm_operand *iter, *end;
#ifdef OPTIMIZE_FASSUME
		if (optimizer_flags & OPTIMIZE_FASSUME) {
			end = (iter = self->a_assembly.as_opv + self->a_assembly.as_num_o) +
			      self->a_assembly.as_num_i;
			for (; iter < end; ++iter) {
				if (ast_optimize(stack, iter->ao_expr, true))
					goto err;
			}
			/* If the assembly branch is marked as reachable from the outside,
			 * then we must act the same as we do for label assumptions, and
			 * delete all assumptions already made. */
			if ((self->a_flag & AST_FASSEMBLY_REACH) &&
			    ast_assumes_undefined_all(stack->os_assume))
				goto err;
			/* Optimize output operands in respect to new assumptions,
			 * as those are executed _after_ input operands. */
			end = (iter = self->a_assembly.as_opv) + self->a_assembly.as_num_o;
			for (; iter < end; ++iter) {
				if (ast_optimize(stack, iter->ao_expr, true))
					goto err;
			}
		} else
#endif /* OPTIMIZE_FASSUME */
		{
			end = (iter = self->a_assembly.as_opv) +
			      (self->a_assembly.as_num_i +
			       self->a_assembly.as_num_o);
			for (; iter < end; ++iter) {
				if (ast_optimize(stack, iter->ao_expr, true))
					goto err;
			}
		}
	}	break;

	default: break;
	}
	return 0;
did_optimize:
	++optimizer_count;
	goto again;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int
(DCALL ast_optimize_all)(struct ast *__restrict self, bool result_used) {
	int result;
	unsigned int old_value;
	do {
		old_value = optimizer_count;
		result    = ast_startoptimize(self, result_used);
		if unlikely(result)
			break;
		/* Stop after the first pass if the `OPTIMIZE_FONEPASS' flag is set. */
		if (optimizer_flags & OPTIMIZE_FONEPASS)
			break;
		/* Keep optimizing the branch while stuff happens. */
	} while (old_value != optimizer_count);
	return result;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPTIMIZE_C */
