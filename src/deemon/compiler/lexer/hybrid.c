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
#ifndef GUARD_DEEMON_COMPILER_LEXER_HYBRID_C
#define GUARD_DEEMON_COMPILER_LEXER_HYBRID_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_Free, Dee_Mallocc, Dee_Reallocc */
#include <deemon/compiler/ast.h>    /* AST_*, ast, ast_* */
#include <deemon/compiler/lexer.h>  /* AST_COMMA_*, AST_PARSE_WASEXPR_*, ast_parse_*, current_tags */
#include <deemon/compiler/symbol.h> /* LOOKUP_SYM_NORMAL, current_scope, scope_pop, scope_push */
#include <deemon/compiler/tpp.h>
#include <deemon/system-features.h> /* memmoveupc, remainder */
#include <deemon/types.h>           /* DREF */

#include <stdbool.h> /* bool, false */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint16_t */

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_do_parse_brace_items(DeeLexer *self) {
	DREF struct ast *result;
	DeeLexer_NoLf_Push(self);
	if (DeeLexer_GetTok(self) == '\n') {
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
/*err_flags:*/
			DeeLexer_NoLf_Break(self);
			goto err;
		}
	}
	result = ast_parse_brace_items(self);
	DeeLexer_NoLf_Pop(self);
	return result;
err:
	return NULL;
}


/* @param: mode: Set of `AST_COMMA_*` - What is allowed and when should we pack values. */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_statement_or_expression(DeeLexer *self, unsigned int *p_was_expression) {
	DREF struct ast *result;
	unsigned int was_expression;
	switch (DeeLexer_GetTok(self)) {

	case '{':
		result = ast_parse_statement_or_braces(self, &was_expression);
		if unlikely(!result)
			goto err;
		if (was_expression != AST_PARSE_WASEXPR_NO) {
			/* Try to parse a suffix expression.
			 * If there was one, then we know that it actually was an expression. */
			tpp_token_num token_num = tpp_lexer_gettokennum(&self->dl_lexer);
			result = ast_parse_postexpr(self, result);
			if (token_num != tpp_lexer_gettokennum(&self->dl_lexer))
				was_expression = AST_PARSE_WASEXPR_YES;
		}
		if (p_was_expression)
			*p_was_expression = was_expression;
		break;

	case TPP_KWD_try:
		result = ast_parse_try_hybrid(self, p_was_expression);
		break;

	case TPP_KWD_if:
		result = ast_parse_if_hybrid(self, p_was_expression);
		break;

	case TPP_KWD_with:
		result = ast_parse_with_hybrid(self, p_was_expression);
		break;

	case TPP_KWD_assert:
		result = ast_parse_assert_hybrid(self, p_was_expression);
		break;

	case TPP_KWD_import:
		result = ast_parse_import_hybrid(self, p_was_expression);
		break;

	case TPP_KWD_for:
	case TPP_KWD_foreach:
	case TPP_KWD_do:
	case TPP_KWD_while:
		result = ast_parse_loopexpr_hybrid(self, p_was_expression);
		break;

	case TPP_KWD_from:
	case TPP_KWD_del: /* TODO: This can also appear in expressions! */
	case TPP_KWD_return:
	case TPP_KWD_yield:
	case TPP_KWD_throw:
	case TPP_KWD_print:
	case TPP_KWD_break:
	case TPP_KWD_continue:
	case TPP_KWD___asm:
	case TPP_KWD___asm__:
	case TPP_KWD_goto:
	case TPP_KWD_switch:
	case TPP_KWD_case:
	case TPP_KWD_default:
	case '@':
	case ';':
		result = ast_parse_statement(self, false);
		if (p_was_expression)
			*p_was_expression = AST_PARSE_WASEXPR_NO;
		break;

	default: {
		uint16_t comma_mode;
		size_t old_varc;
		old_varc   = current_scope->s_mapc;
		comma_mode = 0;
		result = ast_parse_comma(self,
		                         AST_COMMA_PARSESINGLE |
		                         AST_COMMA_NOSUFFIXKWD |
		                         AST_COMMA_ALLOWVARDECLS |
		                         AST_COMMA_PARSESEMI,
		                         AST_FMULTIPLE_GENERIC,
		                         &comma_mode);
		if unlikely(!result)
			goto done;
		if (DeeLexer_GetTok(self) == ';' && (comma_mode & AST_COMMA_OUT_FNEEDSEMI)) {
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
at_semi_after_expression:
#endif /* CONFIG_EXPERIMENTAL_USE_TPP3 */
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
			if (p_was_expression)
				*p_was_expression = AST_PARSE_WASEXPR_NO;
		} else if (old_varc != current_scope->s_mapc) {
			if (comma_mode & AST_COMMA_OUT_FNEEDSEMI) {
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
				tpp_token_id got = DeeLexer_Require(self, TPP_TOK_OFCHAR(';'));
				if (TPP_TOK_ISERR(got))
					goto err;
				if (got == ';')
					goto at_semi_after_expression;
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
				if (DeeLexer_Warnf(self, TPP_W_EXPECTED_SEMICOLON_AFTER_EXPRESSION))
					goto err;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
			}
			if (p_was_expression)
				*p_was_expression = AST_PARSE_WASEXPR_NO;
		} else {
			if (p_was_expression)
				*p_was_expression = AST_PARSE_WASEXPR_YES;
		}
	}	break;

	}
done:
	return result;
err_r:
	ast_decref(result);
err:
	return NULL;
}



/* Same as `ast_parse_try_hybrid` but for if statements / expressions. */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_if_hybrid(DeeLexer *self, unsigned int *p_was_expression) {
	DREF struct ast *tt_branch;
	DREF struct ast *ff_branch;
	DREF struct ast *result, *merge;
	uint16_t expect;
	struct ast_loc loc;
	unsigned int was_expression;
	bool has_paren;
	expect = current_tags.at_expect;
	if (DeeLexer_GetLoc(self, &loc))
		goto err;
	DeeLexer_NoLf_Push(self);
	if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_flags:
		DeeLexer_NoLf_Break(self);
		goto err;
	}
	if (DeeLexer_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_IF))
		goto err_flags;
	result = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
	DeeLexer_NoLf_Pop(self);
	if unlikely(!result)
		goto err;
	if (DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_IF))
		goto err;
	tt_branch      = NULL;
	was_expression = AST_PARSE_WASEXPR_MAYBE;
	if (DeeLexer_GetTok(self) != TPP_KWD_else &&
	    DeeLexer_GetTok(self) != TPP_KWD_elif) {
		tt_branch = ast_parse_hybrid_primary(self, &was_expression);
		if unlikely(!tt_branch)
			goto err_r;
	}
	ff_branch = NULL;
	if (DeeLexer_GetTok(self) == TPP_KWD_elif) {
		DeeLexer_SetTokenId(self, TPP_KWD_if); /* Cheat a bit... */
		goto do_else_branch;
	}
	if (DeeLexer_GetTok(self) == TPP_KWD_else) {
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r_tt;
do_else_branch:
		ff_branch = ast_parse_hybrid_secondary(self, &was_expression);
		if unlikely(!ff_branch)
			goto err_r_tt;
	}
	merge = ast_conditional(AST_FCOND_EVAL | expect, result, tt_branch, ff_branch);
	merge = ast_setddi(merge, &loc);
	ast_xdecref(ff_branch);
	ast_xdecref(tt_branch);
	ast_xdecref(result);
	if (p_was_expression)
		*p_was_expression = was_expression;
	return merge;
err_r_tt:
	ast_xdecref(tt_branch);
err_r:
	ast_decref(result);
err:
	return NULL;
}



/* Parse a statement or a brace-expression, with the current token being a `{` */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_statement_or_braces(DeeLexer *self, unsigned int *p_was_expression) {
	DREF struct ast *result, **new_elemv;
	DREF struct ast *remainder;
	struct ast_loc loc;
	unsigned int was_expression;
	ASSERT(DeeLexer_GetTok(self) == '{');
	if (DeeLexer_GetLoc(self, &loc))
		goto err;
	if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
		goto err;
	switch (DeeLexer_GetTok(self)) {

	case '}':
		/* Special case: empty sequence. */
		result = ast_multiple(AST_FMULTIPLE_GENERIC, 0, NULL);
		result = ast_setddi(result, &loc);
		if unlikely(!result)
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		if (p_was_expression)
			*p_was_expression = AST_PARSE_WASEXPR_MAYBE;
		break;

	case '.':
		result = ast_do_parse_brace_items(self);
		result = ast_setddi(result, &loc);
		if unlikely(!result)
			goto err;
		if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR('}'), W_EXPECTED_RBRACE_AFTER_BRACEINIT))
			goto err_r;
		if (p_was_expression)
			*p_was_expression = AST_PARSE_WASEXPR_YES;
		break;

	case '{': /* Recursion! */
		if unlikely(scope_push() < 0)
			goto err;
		result = ast_parse_statement_or_braces(self, &was_expression);
		ASSERT(!result ||
		       result->a_type == AST_MULTIPLE ||
		       result->a_type == AST_CONSTEXPR);
parse_remainder_after_hybrid_popscope:
		if unlikely(!result)
			goto err;
parse_remainder_after_hybrid_popscope_resok:
		if (was_expression == AST_PARSE_WASEXPR_NO)
			goto parse_remainder_after_statement;
		if (was_expression == AST_PARSE_WASEXPR_YES) {
			result = ast_parse_postexpr(self, result);
			if unlikely(!result)
				goto err;
check_recursion_after_expression_suffix:
			if (DeeLexer_GetTok(self) == ';') {
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err_r;
				goto parse_remainder_after_statement;
			}
			if (DeeLexer_GetTok(self) == ':')
				goto parse_remainder_after_colon_popscope;
			if (DeeLexer_GetTok(self) == ',') {
parse_remainder_after_comma_popscope:
				scope_pop();
				remainder = ast_parse_brace_list(self, result);
				if unlikely(!remainder)
					goto err_r;
				ast_decref(result);
				result = ast_setddi(remainder, &loc);
				if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR('}'), W_EXPECTED_RBRACE_AFTER_BRACEINIT))
					goto err_r;
				if (p_was_expression)
					*p_was_expression = AST_PARSE_WASEXPR_YES;
				break;
			}
			if likely(DeeLexer_GetTok(self) == '}') {
parse_remainder_before_rbrace_popscope_wrap:
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err_r;
			} else {
				if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), TPP_W_EXPECTED_RBRACE_AFTER_BRACEINIT))
					goto err_r;
			}

			/* Wrap the result as a single sequence. */
			new_elemv = (DREF struct ast **)Dee_Mallocc(1, sizeof(DREF struct ast *));
			if unlikely(!new_elemv)
				goto err_r;
			new_elemv[0] = result; /* Inherit reference. */
			remainder    = ast_multiple(AST_FMULTIPLE_GENERIC, 1, new_elemv);
			if unlikely(!remainder) {
				Dee_Free(new_elemv);
				goto err_r;
			}

			/* `ast_multiple()` inherited `new_elemv` on success. */
			result = ast_setddi(remainder, &loc);
			goto parse_remainder_after_rbrace_popscope;
		}
		if (DeeLexer_GetTok(self) == ',')
			goto parse_remainder_after_comma_popscope;
		if (DeeLexer_GetTok(self) == ':')
			goto parse_remainder_after_colon_popscope;
		if (DeeLexer_GetTok(self) == '}')
			goto parse_remainder_before_rbrace_popscope_wrap;
		{
			tpp_token_num token_num = tpp_lexer_gettokennum(&self->dl_lexer);
			result = ast_parse_postexpr(self, result);
			if unlikely(!result)
				goto err;
			if (token_num != tpp_lexer_gettokennum(&self->dl_lexer))
				goto check_recursion_after_expression_suffix;
		}
#if 0
		if (result->a_type == AST_MULTIPLE)
			result->a_flag = AST_FMULTIPLE_KEEPLAST;
#endif
		goto parse_remainder_after_statement;

	case TPP_KWD_try:
		if unlikely(scope_push() < 0)
			goto err;
		result = ast_parse_try_hybrid(self, &was_expression);
		goto parse_remainder_after_hybrid_popscope;

	case TPP_KWD_if:
		if unlikely(scope_push() < 0)
			goto err;
		result = ast_parse_if_hybrid(self, &was_expression);
		goto parse_remainder_after_hybrid_popscope;

	case TPP_KWD_with:
		if unlikely(scope_push() < 0)
			goto err;
		result = ast_parse_with_hybrid(self, &was_expression);
		goto parse_remainder_after_hybrid_popscope;

	case TPP_KWD_assert:
		if unlikely(scope_push() < 0)
			goto err;
		result = ast_parse_assert_hybrid(self, &was_expression);
parse_remainder_after_semicolon_hybrid_popscope:
		if unlikely(!result)
			goto err;

		/* Special case: `assert` statements require a trailing `;` token.
		 *                If that token exists, we know for sure that this is a statement! */
		if (DeeLexer_GetTok(self) == ';') {
			was_expression = AST_PARSE_WASEXPR_NO;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
		}
		goto parse_remainder_after_hybrid_popscope_resok;

	case TPP_KWD_import:
		if unlikely(scope_push() < 0)
			goto err;
		result = ast_parse_import_hybrid(self, &was_expression);
		if unlikely(!result)
			goto err;
		/* Same as `assert`: `import` requires a trailing `;` */
		goto parse_remainder_after_semicolon_hybrid_popscope;

	case TPP_KWD_for:
	case TPP_KWD_foreach:
	case TPP_KWD_do:
	case TPP_KWD_while:
		if unlikely(scope_push() < 0)
			goto err;
		result = ast_parse_loopexpr_hybrid(self, &was_expression);
		goto parse_remainder_after_hybrid_popscope;

	case TPP_KWD_from:
	case TPP_KWD_del: /* TODO: This can also appear in expressions! */
	case TPP_KWD_return:
	case TPP_KWD_yield:
	case TPP_KWD_throw:
	case TPP_KWD_print:
	case TPP_KWD_break:
	case TPP_KWD_continue:
	case TPP_KWD___asm:
	case TPP_KWD___asm__:
	case TPP_KWD_goto:
	case TPP_KWD_switch:
	case TPP_KWD_case:
	case TPP_KWD_default:
	case '@':
	case ';':
is_a_statement:
		if unlikely(scope_push() < 0)
			goto err;
		/* Enter a new scope and parse expressions. */
		DeeLexer_EnableLf_Push(self);
		result = ast_parse_statements_until(self, AST_FMULTIPLE_KEEPLAST, TPP_TOK_OFCHAR('}'));
		result = ast_putddi(result, &loc);
		DeeLexer_EnableLf_Pop(self);
		if unlikely(!result)
			goto err;
		while (DeeLexer_GetTok(self) == '\n') {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
		}
		if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR('}'), W_EXPECTED_RBRACE_AFTER_LBRACE))
			goto err_r;
		scope_pop();
		if (p_was_expression)
			*p_was_expression = AST_PARSE_WASEXPR_NO;
		break;

	default: {
		uint16_t comma_mode;

		/* Check for a label definition. */
		if (DeeLexer_HasTokenKwd(self)) {
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
			tpp_token_id next_token;
			next_token = tpp_lexer_peek_raw(&self->dl_lexer,
			                                TPP_LEXER_PEEK_RAW_FLAG_NORMAL,
			                                NULL, NULL, NULL);
			if (TPP_TOK_ISERR(next_token))
				goto err;
			if (next_token == ':')
				goto is_a_statement; /* label */
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
			char const *next_token = peek_next_token(NULL);
			if unlikely(!next_token)
				goto err;
			if (*next_token == ':' &&
			    (next_token = advance_wraplf(next_token),
			     *next_token != ':' && *next_token != '='))
				goto is_a_statement; /* label */
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
		}

		/* Figure out what we're dealing with as we go. */
		if unlikely(scope_push() < 0)
			goto err;
		comma_mode = 0;
		result = ast_parse_comma(self,
		                         AST_COMMA_NORMAL |
		                         AST_COMMA_FORCEMULTIPLE |
		                         AST_COMMA_ALLOWVARDECLS |
		                         AST_COMMA_ALLOWTYPEDECL |
		                         AST_COMMA_PARSESEMI,
		                         AST_FMULTIPLE_GENERIC,
		                         &comma_mode);
		if unlikely(!result)
			goto err;
		ASSERT(result->a_type == AST_MULTIPLE);
		ASSERT(result->a_flag == AST_FMULTIPLE_GENERIC);
		if (!current_scope->s_mapc) {
			if (DeeLexer_GetTok(self) == '}') {
/*parse_remainder_before_rbrace_popscope:*/
				/* Sequence-like brace expression. */
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
parse_remainder_after_rbrace_popscope:
				scope_pop();
				if (p_was_expression)
					*p_was_expression = AST_PARSE_WASEXPR_YES;
				break;
			}
			if (DeeLexer_GetTok(self) == ':' && result->a_multiple.m_astc == 1) {
				/* Use the first expression from the multi-branch. */
				remainder = result->a_multiple.m_astv[0];
				ast_incref(remainder);
				ast_decref(result);
				result = remainder;
parse_remainder_after_colon_popscope:
				scope_pop();

				/* mapping-like brace expression. */
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err_r;
				remainder = ast_parse_mapping(self, result);
				ast_decref(result);
				if unlikely(!remainder)
					goto err;
				result = ast_setddi(remainder, &loc);
				if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR('}'), W_EXPECTED_RBRACE_AFTER_BRACEINIT))
					goto err_r;
				if (p_was_expression)
					*p_was_expression = AST_PARSE_WASEXPR_YES;
				break;
			}
		}

		/* Statement expression. */
		if (comma_mode & AST_COMMA_OUT_FNEEDSEMI) {
			/* Consume a `;` token as part of the expression. */
			if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), W_EXPECTED_SEMICOLON_AFTER_EXPRESSION))
				goto err_r;
		}
		if (result->a_multiple.m_astc == 1) {
			remainder = result->a_multiple.m_astv[0];
			ast_incref(remainder);
			ast_decref(result);
			result = remainder;
		} else {
			result->a_flag = AST_FMULTIPLE_KEEPLAST;
		}
parse_remainder_after_statement:
		if (DeeLexer_GetTok(self) == '}') {
			ast_setddi(result, &loc);
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_r;
		} else {
			remainder = ast_parse_statements_until(self, AST_FMULTIPLE_KEEPLAST, TPP_TOK_OFCHAR('}'));
			remainder = ast_putddi(remainder, &loc);
			if unlikely(!remainder)
				goto err_r;
			if (remainder->a_type == AST_MULTIPLE &&
			    remainder->a_flag == AST_FMULTIPLE_KEEPLAST &&
			    remainder->a_scope == current_scope) {
				new_elemv = (DREF struct ast **)Dee_Reallocc(remainder->a_multiple.m_astv,
				                                             remainder->a_multiple.m_astc + 1,
				                                             sizeof(DREF struct ast *));
				if unlikely(!new_elemv)
					goto err_r_remainder;
				memmoveupc(new_elemv + 1,
				           new_elemv,
				           remainder->a_multiple.m_astc,
				           sizeof(DREF struct ast *));
				remainder->a_multiple.m_astv = new_elemv;
				new_elemv[0]                 = result; /* Inherit reference. */
				++remainder->a_multiple.m_astc;
			} else {
				new_elemv = (DREF struct ast **)Dee_Mallocc(2, sizeof(DREF struct ast *));
				if unlikely(!new_elemv)
					goto err_r_remainder;
				new_elemv[0] = result;    /* Inherit reference. */
				new_elemv[1] = remainder; /* Inherit reference. */
				remainder    = ast_multiple(AST_FMULTIPLE_KEEPLAST, 2, new_elemv);
				if unlikely(!remainder) {
					ast_decref(new_elemv[1]);
					ast_decref(new_elemv[0]);
					Dee_Free(new_elemv);
					goto err;
				}
				/* `ast_multiple()` inherited `new_elemv` on success. */
			}
			result = ast_setddi(remainder, &loc);
			if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR('}'), W_EXPECTED_RBRACE_AFTER_LBRACE))
				goto err_r;
		}
		scope_pop();
		if (p_was_expression)
			*p_was_expression = AST_PARSE_WASEXPR_NO;
	}	break;
	}
	return result;
err_r_remainder:
	ast_decref(remainder);
err_r:
	ast_decref(result);
err:
	return NULL;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_HYBRID_C */
