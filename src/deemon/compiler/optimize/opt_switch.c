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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_TRY_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_TRY_C 1

#include <deemon/api.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/symbol.h>
#include <deemon/object.h>

DECL_BEGIN

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL ast_optimize_switch)(struct ast_optimize_stack *__restrict stack,
                            struct ast *__restrict self, bool result_used) {
	ASSERT(self->a_type == AST_SWITCH);
	(void)result_used;
	if (ast_optimize(stack, self->a_switch.s_expr, true))
		goto err;
#ifdef OPTIMIZE_FASSUME
	if (optimizer_flags & OPTIMIZE_FASSUME) {
		/* Since execution may enter the switch block at any location,
		 * any assumption made within must implicitly be matched by an
		 * empty branch.
		 * XXX: Technically, we should optimize this as follows:
		 * >> switch (foo) {
		 * >> case 1:
		 * >>        a();
		 * >>        break;
		 * >> case 2:
		 * >>        b();
		 * >> case 3:
		 * >>        c();
		 * >>        break;
		 * >> }
		 * OPT:
		 * >> if (...) {
		 * >>     a();
		 * >> }
		 * >> if (...) {
		 * >>     if (...) {
		 * >>         b();
		 * >>     }
		 * >>     c();
		 * >> }
		 */
		struct ast_assumes child_assumes;
		struct ast_optimize_stack child_stack;
		if (ast_assumes_initcond(&child_assumes, stack->os_assume))
			goto err;
		child_stack.os_prev   = stack;
		child_stack.os_ast    = self->a_switch.s_block;
		child_stack.os_assume = &child_assumes;
		child_stack.os_used   = false;
		if (ast_optimize(&child_stack, child_stack.os_ast, false)) {
err_child_assumes:
			ast_assumes_fini(&child_assumes);
			goto err;
		}
		/* Merge assumptions made by the child-branch. */
		if (ast_assumes_mergecond(&child_assumes, NULL))
			goto err_child_assumes;
		if (ast_assumes_merge(stack->os_assume, &child_assumes))
			goto err_child_assumes;
		ast_assumes_fini(&child_assumes);
	} else
#endif /* OPTIMIZE_FASSUME */
	{
		if (ast_optimize(stack, self->a_switch.s_block, false))
			goto err;
	}
	/* TODO: Delete constant cases shared with the default-case. */
	/* TODO: Looking at the type of the switch-expression, check if we can delete
	 *       some impossible cases. (e.g. integer cases with string-expression) */
	return 0;
err:
	return -1;
/*
did_optimize:
	++optimizer_count;
	return 0;
*/
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_TRY_C */
