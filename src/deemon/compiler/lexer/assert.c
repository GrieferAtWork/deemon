/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_LEXER_ASSERT_C
#define GUARD_DEEMON_COMPILER_LEXER_ASSERT_C 1

#include <deemon/api.h>

#include <deemon/compiler/ast.h>    /* AST_*, ast, ast_* */
#include <deemon/compiler/lexer.h>  /* AST_PARSE_WASEXPR_MAYBE, AST_PARSE_WASEXPR_NO, ast_parse_* */
#include <deemon/compiler/symbol.h> /* LOOKUP_SYM_NORMAL */
#include <deemon/compiler/tpp.h>
#include <deemon/types.h>           /* DREF */

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL */

DECL_BEGIN

#undef CONFIG_ASSERT_DDI_USES_EXPRESSION
#define CONFIG_ASSERT_DDI_USES_EXPRESSION

/* Parse an assertion statement. (must be started ontop of the `assert` keyword) */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_assert(DeeLexer *self, bool needs_parenthesis) {
	DREF struct ast *result, *message, *merge;
#ifndef CONFIG_ASSERT_DDI_USES_EXPRESSION
	struct ast_loc loc;
#endif /* !CONFIG_ASSERT_DDI_USES_EXPRESSION */
	ASSERT(DeeLexer_GetTok(self) == TPP_KWD_assert);
#ifndef CONFIG_ASSERT_DDI_USES_EXPRESSION
	if (DeeLexer_GetLoc(self, &loc))
		goto err;
#endif /* !CONFIG_ASSERT_DDI_USES_EXPRESSION */
	if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
		goto err;
	message = NULL;
	if (needs_parenthesis) {
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
		if (TPP_TOK_ISERR(DeeLexer_Require(self, TPP_TOK_OFCHAR('('))))
			goto err;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
		if (DeeLexer_GetTok(self) != '(') {
			if (DeeLexer_Warnf(self, TPP_W_EXPECTED_LPAREN_AFTER_ASSERT_IN_EXPRESSION))
				goto err;
		}
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
	}
	if (DeeLexer_GetTok(self) == '(') {
		/* Special case: We must be able to handle both of these:
		 * >> assert (foo == bar), "Error";
		 * >> ASSERT(foo == bar, "Error");
		 */
		result = ast_parse_unary(self, LOOKUP_SYM_NORMAL);
		if unlikely(!result)
			goto err;
		if (!needs_parenthesis) {
			result = ast_parse_postexpr(self, result);
			if unlikely(!result)
				goto err;
		}
		if (!needs_parenthesis && DeeLexer_GetTok(self) == ',') {
			/* The message was passed individually. */
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
			message = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
			if unlikely(!message)
				goto err_r;
		} else if (result->a_type == AST_MULTIPLE &&
		           result->a_flag == AST_FMULTIPLE_TUPLE &&
		           result->a_multiple.m_astc >= 2) {
			/* Steal the last expression and use it as message. */
			message = result->a_multiple.m_astv[--result->a_multiple.m_astc];
			if (result->a_multiple.m_astc == 1) {
				merge = result->a_multiple.m_astv[0];
				ast_incref(merge);
				ast_decref(result);
				result = merge;
			}
		}
	} else {
		result = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
		if unlikely(!result)
			goto err;
		if (DeeLexer_GetTok(self) == ',') {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
			message = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
			if unlikely(!message)
				goto err_r;
		}
	}

	/* Create the assert branch. */
	merge = ast_setddi(message ? ast_action2(AST_FACTION_ASSERT_M, result, message)
	                           : ast_action1(AST_FACTION_ASSERT, result),
#ifdef CONFIG_ASSERT_DDI_USES_EXPRESSION
	                   &result->a_ddi
#else /* CONFIG_ASSERT_DDI_USES_EXPRESSION */
	                   &loc
#endif /* !CONFIG_ASSERT_DDI_USES_EXPRESSION */
	                   );
	ast_xdecref(message);
	ast_decref(result);
	return merge;
err_r:
	ast_decref(result);
err:
	return NULL;
}


/* Same as `ast_parse_try_hybrid` but for assert statements / expressions. */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_assert_hybrid(DeeLexer *self, unsigned int *p_was_expression) {
	DREF struct ast *result, *message, *merge;
#ifndef CONFIG_ASSERT_DDI_USES_EXPRESSION
	struct ast_loc loc;
#endif /* !CONFIG_ASSERT_DDI_USES_EXPRESSION */
	ASSERT(DeeLexer_GetTok(self) == TPP_KWD_assert);
#ifndef CONFIG_ASSERT_DDI_USES_EXPRESSION
	if (DeeLexer_GetLoc(self, &loc))
		goto err;
#endif /* !CONFIG_ASSERT_DDI_USES_EXPRESSION */
	if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
		goto err;
	message = NULL;
	if (DeeLexer_GetTok(self) == '(') {
		/* Special case: We must be able to handle both of these:
		 * >> assert (foo == bar), "Error";
		 * >> ASSERT(foo == bar, "Error");
		 */
		result = ast_parse_unary(self, LOOKUP_SYM_NORMAL);
		if unlikely(!result)
			goto err;
		if (result->a_type == AST_MULTIPLE &&
		    result->a_flag == AST_FMULTIPLE_TUPLE &&
		    result->a_multiple.m_astc >= 2) {
			/* Steal the last expression and use it as message. */
			message = result->a_multiple.m_astv[--result->a_multiple.m_astc];
			if (result->a_multiple.m_astc == 1) {
				merge = result->a_multiple.m_astv[0];
				ast_incref(merge);
				ast_decref(result);
				result = merge;
			}
		}
		if (p_was_expression)
			*p_was_expression = AST_PARSE_WASEXPR_MAYBE;
	} else {
		result = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
		if unlikely(!result)
			goto err;
		if (DeeLexer_GetTok(self) == ',') {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
			message = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
			if unlikely(!message)
				goto err_r;
		}
		if (p_was_expression)
			*p_was_expression = AST_PARSE_WASEXPR_NO;
	}
	/* Create the assert branch. */
	merge = ast_setddi(message ? ast_action2(AST_FACTION_ASSERT_M, result, message)
	                           : ast_action1(AST_FACTION_ASSERT, result),
#ifdef CONFIG_ASSERT_DDI_USES_EXPRESSION
	                   &result->a_ddi
#else /* CONFIG_ASSERT_DDI_USES_EXPRESSION */
	                   &loc
#endif /* !CONFIG_ASSERT_DDI_USES_EXPRESSION */
	                   );
	ast_xdecref(message);
	ast_decref(result);
	return merge;
err_r:
	ast_decref(result);
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_ASSERT_C */
