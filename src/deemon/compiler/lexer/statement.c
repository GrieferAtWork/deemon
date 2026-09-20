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
#ifndef GUARD_DEEMON_COMPILER_LEXER_STATEMENT_C
#define GUARD_DEEMON_COMPILER_LEXER_STATEMENT_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_*alloc*, Dee_CollectMemoryc, Dee_Free */
#include <deemon/code.h>            /* Dee_CODE_FYIELDING */
#include <deemon/compiler/ast.h>    /* AST_*, ast, ast_* */
#include <deemon/compiler/lexer.h>  /* AST_COMMA_*, ast_parse_*, ast_tags_clear, current_tags, parse_tags_block */
#include <deemon/compiler/symbol.h> /* BASESCOPE_FRETURN, BASESCOPE_FSWITCH, LOOKUP_SYM_ALLOWDECL, LOOKUP_SYM_NORMAL, current_basescope, lookup_label, new_case_label, new_default_label, scope_pop, scope_push, text_label */
#include <deemon/compiler/tpp.h>
#include <deemon/none.h>            /* Dee_None */
#include <deemon/object.h>          /* DREF */
#include <deemon/system-features.h> /* memmoveupc */
#include <deemon/tuple.h>           /* Dee_EmptyTuple */
#include <deemon/type.h>            /* OPERATOR_ITER */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* int32_t, uint16_t */

DECL_BEGIN

#define is_semicolon(tok) ((tok) == ';' || (tok) == '\n')

PRIVATE WUNUSED NONNULL((1)) tpp_token_id DFCALL
yield_semicolonnbif(DeeLexer *self, bool allow_nonblock) {
	tpp_token_id result = DeeLexer_YieldXNB(self, allow_nonblock);
	if (result == '\n') {
		DeeLexer_NoLf_Push(self);
		result = DeeLexer_YieldXNB(self, allow_nonblock);
		DeeLexer_NoLf_Pop(self);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DFCALL
skip_lf(DeeLexer *self) {
	if (DeeLexer_GetTok(self) == '\n') {
		tpp_token_id error;
		DeeLexer_NoLf_Push(self);
		error = DeeLexer_Yield(self);
		DeeLexer_NoLf_Pop(self);
		if (TPP_TOK_ISERR(error))
			goto err;
	}
	return 0;
err:
	return -1;
}



/* Parse the head header of a for-statement, returning the appropriate
 * AST flags for creating the loop (usually `AST_FLOOP_NORMAL` or `AST_FLOOP_FOREACH`),
 * as well as filling in the given pointers to used asts.
 * NOTE: The caller is responsible for wrapping this function in its own
 *       scope, should they choose to with initializers/loop element symbols
 *       to be placed in their own scope.
 * NOTE: Any of the given pointers may be filled with NULL if that AST is not present,
 *       unless the loop is actually a foreach-loop, in which case they _must_ always
 *       be present.
 * WARNING: The caller is responsible for wrapping `*p_iter_or_next` in an `__iterself__()`
 *          operator call when `AST_FLOOP_FOREACH` is part of the return mask, unless they wish
 *          to enumerate an iterator itself (which is possible using the `__foreach` statement). */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int32_t DFCALL
ast_parse_for_head(DeeLexer *self,
                   DREF struct ast **__restrict p_init,
                   DREF struct ast **__restrict p_elem_or_cond,
                   DREF struct ast **__restrict p_iter_or_next) {
	int32_t result = AST_FLOOP_NORMAL;
	DREF struct ast *init = NULL;
	DREF struct ast *elem_or_cond = NULL;
	DREF struct ast *iter_or_next = NULL;
	if (DeeLexer_GetTok(self) != ';') {
		init = ast_parse_comma(self,
		                       AST_COMMA_ALLOWVARDECLS,
		                       AST_FMULTIPLE_TUPLE,
		                       NULL);
		if unlikely(!init)
			goto err;
		if (DeeLexer_GetTok(self) == ':') {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
			/* foreach-style loop. */
			elem_or_cond = init;
			init         = NULL;
			result |= AST_FLOOP_FOREACH;
			iter_or_next = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
			if unlikely(!iter_or_next)
				goto err;
			goto done;
		}
	}
	if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), W_EXPECTED_SEMICOLON1_AFTER_FOR))
		goto err;
	if (DeeLexer_GetTok(self) != ';') {
		elem_or_cond = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
		if unlikely(!elem_or_cond)
			goto err;
	}
	if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), W_EXPECTED_SEMICOLON2_AFTER_FOR))
		goto err;
	if (DeeLexer_GetTok(self) == ')') {
		iter_or_next = NULL;
	} else {
		iter_or_next = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
		if unlikely(!iter_or_next)
			goto err;
	}
done:
	*p_init         = init;
	*p_elem_or_cond = elem_or_cond;
	*p_iter_or_next = iter_or_next;
	return result;
err:
	ast_xdecref(init);
	ast_xdecref(elem_or_cond);
	ast_xdecref(iter_or_next);
	return -1;
}



/* Parse a sequence of statements until `end_token` is
 * encountered at the start of a statement, or until
 * the end of the current input-file-stack is reached.
 * NOTE: The returned ast is usually an `AST_MULTIPLE`,
 *       which will have the given `flags` assigned.
 * WARNING: If only a single AST would be contained
 *          and `flags` is `AST_FMULTIPLE_KEEPLAST`,
 *          the inner expression is automatically
 *          returned instead.
 * NOTE: If desired, the caller is responsible to setup
 *       or teardown a new scope before/after this function. */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_statements_until(DeeLexer *self, uint16_t flags, tpp_token_id end_token) {
	size_t exprc, expra;
	DREF struct ast **exprv;
	DREF struct ast *new_expression;
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
	unsigned long token_num;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
	exprc = expra = 0, exprv = NULL;
	for (;;) {
		if unlikely(skip_lf(self))
			goto err;
		if (DeeLexer_GetTok(self) == TPP_TOK_EOF ||
		    DeeLexer_GetTok(self) == end_token)
			break;
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
		token_num = TPPLexer_Current->l_token.t_num;
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
		new_expression = ast_parse_statement(self, false);
		if unlikely(!new_expression)
			goto err;
		ASSERT(exprc <= expra);
		if (exprc == expra) {
			DREF struct ast **new_exprv;
			size_t new_expra = expra * 2;
			if (!new_expra)
				new_expra = 8;
do_realloc:
			new_exprv = (DREF struct ast **)Dee_TryReallocc(exprv, new_expra,
			                                                sizeof(DREF struct ast *));
			if unlikely(!new_exprv) {
				if (new_expra != exprc + 1) {
					new_expra = exprc + 1;
					goto do_realloc;
				}
				if (Dee_CollectMemoryc(new_expra, sizeof(DREF struct ast *)))
					goto do_realloc;
				goto err;
			}
			exprv = new_exprv;
			expra = new_expra;
		}
		exprv[exprc++] = new_expression; /* Inherit reference. */
#ifndef CONFIG_EXPERIMENTAL_USE_TPP3
		if (token_num == TPPLexer_Current->l_token.t_num) {
			if (DeeLexer_Warnf(self, TPP_W_FAILED_TO_PARSE_STATEMENT))
				goto err;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
		}
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
	}
	/* Truncate the expression buffer to what is actually being used. */
	if (exprc != expra) {
		DREF struct ast **new_exprv;
		new_exprv = (DREF struct ast **)Dee_TryReallocc(exprv, exprc, sizeof(DREF struct ast *));
		if (new_exprv)
			exprv = new_exprv;
	}
	/* Pack all parsed statements into a multi-ast.
	 * NOTE: This automatically unwinds keep-last-single ASTs. */
	new_expression = ast_multiple(flags, exprc, exprv);
	if unlikely(!new_expression)
		goto err;
	return new_expression;
err:
	/* Cleanup. */
	ast_decrefv(exprv, exprc);
	Dee_Free(exprv);
	return NULL;
}


INTDEF void DCALL
cleanup_switch_cases(struct text_label *switch_cases,
                     struct text_label *switch_default);

/* Parse a regular, old statement. */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_statement(DeeLexer *self, bool allow_nonblock) {
	DREF struct ast *result, *merge;
	struct ast_loc loc;
again:
	switch (DeeLexer_GetTok(self)) {

	case '@':
		/* Parse tags. */
		if (parse_tags_block(self))
			goto err;
		goto again;

	case '{':
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if unlikely(scope_push() < 0)
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		/* Enter a new scope and parse expressions. */
		result = ast_parse_statements_until(self, AST_FMULTIPLE_KEEPLAST, TPP_TOK_OFCHAR('}'));
		result = ast_putddi(result, &loc);
		if unlikely(!result)
			goto err;
		while (DeeLexer_GetTok(self) == '\n')
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
		goto err_r;
		if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR('}'), W_EXPECTED_RBRACE_AFTER_LBRACE))
			goto err_r;
		scope_pop();
		break;

	case '\n':
		/* Skip empty, leading lines in statements. */
		if unlikely(skip_lf(self))
			goto err;
		goto again;

	case ';':
		result = ast_constexpr(Dee_None);
		result = ast_sethere(self, result);
		if (TPP_TOK_ISERR(DeeLexer_YieldXNB(self, allow_nonblock)))
			goto err;
		break;

	case TPP_KWD_if: {
		DREF struct ast *tt_branch;
		DREF struct ast *ff_branch;
		uint16_t expect;
		bool has_paren;

		/* If-statement. */
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		expect = current_tags.at_expect;
		if unlikely(scope_push() < 0)
			goto err;
		DeeLexer_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_if_flags:
			DeeLexer_NoLf_Break(self);
			goto err;
		}
		if (DeeLexer_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_IF))
			goto err_if_flags;
		result = ast_parse_comma(self,
		                         AST_COMMA_NORMAL |
		                         AST_COMMA_ALLOWVARDECLS,
		                         AST_FMULTIPLE_KEEPLAST,
		                         NULL);
		DeeLexer_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		if (DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_IF))
			goto err_r;
		tt_branch = ast_parse_statement(self, false);
		if unlikely(!tt_branch)
			goto err_r;
		ff_branch = NULL;

		/* Allow tags before the `else` keyword (forward-compatibility...) */
		if unlikely(ast_tags_clear(self))
			goto err_tt_branch;
		if unlikely(skip_lf(self))
			goto err_tt_branch;
		if unlikely(parse_tags_block(self)) {
err_tt_branch:
			ast_decref(tt_branch);
			goto err_r;
		}
		if unlikely(skip_lf(self))
			goto err_tt_branch;
		if (DeeLexer_GetTok(self) == TPP_KWD_elif) {
			DeeLexer_SetTokenId(self, TPP_KWD_if); /* Cheat a bit... */
			goto do_else_branch;
		}
		if (DeeLexer_GetTok(self) == TPP_KWD_else) {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_tt_branch;
do_else_branch:
			ff_branch = ast_parse_statement(self, allow_nonblock);
			if unlikely(!ff_branch)
				goto err_tt_branch;
		}
		merge = ast_conditional(AST_FCOND_EVAL | expect, result, tt_branch, ff_branch);
		merge = ast_setddi(merge, &loc);
		scope_pop();
		ast_xdecref(ff_branch);
		ast_decref(tt_branch);
		ast_decref(result);
		result = merge;
		/* We've already parsed tags, so don't reset them before parsing the next statement. */
		goto done_no_tag_reset;
	}

	case TPP_KWD_return:
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if (is_semicolon(DeeLexer_GetTok(self))) {
			/* Special case: A return without an operator is allowed
			 *               in both return and yield functions. */
			result = ast_return(NULL);
		} else {
			if ((current_basescope->bs_flags & Dee_CODE_FYIELDING)) {
				if (DeeLexer_Warnf(self, TPP_W_RETURN_IN_YIELD_FUNCTION))
					goto err;
			}
			current_basescope->bs_cflags |= BASESCOPE_FRETURN;
			result = ast_parse_comma(self,
			                         AST_COMMA_NORMAL,
			                         AST_FMULTIPLE_TUPLE,
			                         NULL);
			if unlikely(!result)
				goto err;
			merge = ast_return(result);
			ast_decref(result);
			result = merge;
		}
		ast_setddi(result, &loc);
		if unlikely(!result)
			goto err;
		if likely(is_semicolon(DeeLexer_GetTok(self))) {
			if unlikely(yield_semicolonnbif(self, allow_nonblock) < 0)
				goto err_r;
		} else {
			if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), TPP_W_EXPECTED_SEMICOLON_AFTER_RETURN))
				goto err_r;
		}
		break;

	case TPP_KWD_yield:
		/* TODO: Warn about use of non-portable extension if this yield
		 *       statement appears inside of a statement-expression, or
		 *       a finally/catch block. */
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		result = ast_parse_comma(self,
		                         AST_COMMA_NORMAL |
		                         AST_COMMA_FORCEMULTIPLE,
		                         AST_FMULTIPLE_TUPLE,
		                         NULL);
		if unlikely(!result)
			goto err;
		merge = ast_expand(result);
		merge = ast_setddi(merge, &loc);
		ast_decref(result);
		if unlikely(!merge)
			goto err;
		result = ast_yield(merge);
		result = ast_setddi(result, &loc);
		ast_decref(merge);
		if unlikely(!result)
			goto err;
		current_basescope->bs_flags |= Dee_CODE_FYIELDING;
		if (current_basescope->bs_cflags & BASESCOPE_FRETURN) {
			if (DeeLexer_Warnf(self, TPP_W_YIELD_AFTER_RETURN))
				goto err;
		}
		if likely(is_semicolon(DeeLexer_GetTok(self))) {
			if unlikely(yield_semicolonnbif(self, allow_nonblock) < 0)
				goto err_r;
		} else {
			if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), TPP_W_EXPECTED_SEMICOLON_AFTER_YIELD))
				goto err_r;
		}
		break;

	case TPP_KWD_from:
	case TPP_KWD_import:
		result = ast_parse_import(self);
		if unlikely(!result)
			goto err;
		if likely(is_semicolon(DeeLexer_GetTok(self))) {
			if unlikely(yield_semicolonnbif(self, allow_nonblock) < 0)
				goto err_r;
		} else {
			if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), TPP_W_EXPECTED_SEMICOLON_AFTER_IMPORT))
				goto err_r;
		}
		break;

	case TPP_KWD_throw:
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if (is_semicolon(DeeLexer_GetTok(self))) {
			result = ast_throw(NULL);
		} else {
			result = ast_parse_comma(self,
			                         AST_COMMA_NORMAL,
			                         AST_FMULTIPLE_TUPLE,
			                         NULL);
			if unlikely(!result)
				goto err;
			merge = ast_throw(result);
			ast_decref(result);
			result = merge;
		}
		ast_setddi(result, &loc);
		if unlikely(!result)
			goto err;
		if likely(is_semicolon(DeeLexer_GetTok(self))) {
			if unlikely(yield_semicolonnbif(self, allow_nonblock) < 0)
				goto err_r;
		} else {
			if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), TPP_W_EXPECTED_SEMICOLON_AFTER_THROW))
				goto err_r;
		}
		break;

	case TPP_KWD_print:
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if (DeeLexer_GetTok(self) == ',') {
			/* `print,;` --> `none` */
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
			result = ast_constexpr(Dee_None);
			if unlikely(!result)
				goto err;
		} else if (is_semicolon(DeeLexer_GetTok(self))) {
			/* `print;` --> `print pack()...;` */
			result = ast_constexpr(Dee_EmptyTuple);
			if unlikely(!result)
				goto err;
			merge = ast_action1(AST_FACTION_PRINTLN, result);
			ast_decref(result);
			if unlikely(!merge)
				goto err;
			result = merge;
		} else {
			/* NOTE: Use strict comma-rules, because a trailing comma in a print statement
			 *       causes the generated assembly to omit a terminating linefeed. */
			result = ast_parse_comma(self,
			                         AST_COMMA_FORCEMULTIPLE |
			                         AST_COMMA_STRICTCOMMA,
			                         AST_FMULTIPLE_TUPLE,
			                         NULL);
			if unlikely(!result)
				goto err;
			if (DeeLexer_GetTok(self) == ':') {
				DREF struct ast *text;
				/* This is actually an fprint-style statement: `print foo: "bar";' */
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err_r;
				/* Since we've forced a multiple-parse above, we must now extract the target file. */
				if (result->a_type == AST_MULTIPLE &&
				    result->a_multiple.m_astc == 1) {
					merge = result->a_multiple.m_astv[0];
					ast_incref(merge);
					ast_decref(result);
					result = merge;
				}
				if (is_semicolon(DeeLexer_GetTok(self))) {
					text = ast_constexpr(Dee_EmptyTuple);
					text = ast_sethere(self, text);
				} else {
					text = ast_parse_comma(self,
					                       AST_COMMA_FORCEMULTIPLE |
					                       AST_COMMA_STRICTCOMMA,
					                       AST_FMULTIPLE_TUPLE,
					                       NULL);
				}
				if unlikely(!text)
					goto err_r;
				merge = ast_action2(DeeLexer_GetTok(self) == ','
				                    ? AST_FACTION_FPRINT
				                    : AST_FACTION_FPRINTLN,
				                    result, text);
				ast_decref(text);
				ast_decref(result);
				if unlikely(!merge)
					goto err;
				result = merge;
				if (DeeLexer_GetTok(self) == ',') {
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_r;
				}
			} else {
				/* Print-to-stdout statement. */
				merge = ast_action1(DeeLexer_GetTok(self) == ','
				                    ? AST_FACTION_PRINT
				                    : AST_FACTION_PRINTLN,
				                    result);
				ast_decref(result);
				if unlikely(!merge)
					goto err;
				result = merge;
				if (DeeLexer_GetTok(self) == ',') {
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_r;
				}
			}
		}
		ast_setddi(result, &loc);
		if likely(is_semicolon(DeeLexer_GetTok(self))) {
			if unlikely(yield_semicolonnbif(self, allow_nonblock) < 0)
				goto err_r;
		} else {
			if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), TPP_W_EXPECTED_SEMICOLON_AFTER_PRINT))
				goto err_r;
		}
		break;

	case TPP_KWD_for: {
		DREF struct ast *init;
		DREF struct ast *elem_or_cond;
		DREF struct ast *iter_or_next;
		DREF struct ast *loop;
		int32_t type;
		bool has_scope;
		bool has_paren;
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		DeeLexer_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_for_flags:
			DeeLexer_NoLf_Break(self);
			goto err;
		}
		if (DeeLexer_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_FOR))
			goto err_for_flags;
		has_scope = false;
		if (DeeLexer_GetTok(self) != ';') {
			/* To save on space, only create a loop scope when an initializer was given. */
			if unlikely(scope_push())
				goto err_for_flags;
			has_scope = true;
		}
		type = ast_parse_for_head(self, &init, &elem_or_cond, &iter_or_next);
		if unlikely(type < 0)
			goto err_for_flags;
		DeeLexer_NoLf_Pop(self);
		if (type & AST_FLOOP_FOREACH && iter_or_next) {
			/* Wrap the iterator of a foreach-loop with an __iterself__ operator. */
			merge = ast_setddi(ast_operator1(OPERATOR_ITER,
			                                 AST_OPERATOR_FNORMAL,
			                                 iter_or_next),
			                   &loc);
			if unlikely(!merge)
				goto err_loop;
			ast_decref(iter_or_next);
			iter_or_next = merge;
		}
		if (DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_FOR))
			goto err_loop;

		loop = ast_parse_statement(self, allow_nonblock);
		if unlikely(!loop)
			goto err_loop;
		/* Create the loop branch. */
		result = ast_loop((uint16_t)type, elem_or_cond, iter_or_next, loop);
		result = ast_setddi(result, &loc);
		if unlikely(!result)
			goto err_loop2;
		ast_decref(loop);
		ast_xdecref(iter_or_next);
		ast_xdecref(elem_or_cond);
		if (init) {
			DREF struct ast **exprv = (DREF struct ast **)Dee_Mallocc(2, sizeof(DREF struct ast *));
			if unlikely(!exprv) {
err_loop_init:
				ast_decref(init);
				goto err_r;
			}
			/* A loop initializer was given. - Pack it into the resulting AST. */
			exprv[0] = init;   /* Inherit reference. */
			exprv[1] = result; /* Inherit reference. */
			merge    = ast_multiple(AST_FMULTIPLE_KEEPLAST, 2, exprv);
			if unlikely(!merge) {
				Dee_Free(exprv);
				goto err_loop_init;
			}
			result = ast_setddi(merge, &loc);
		}
		/* Pop the scope for the loop initializer, if one was used. */
		if (has_scope)
			scope_pop();
		break;
err_loop2:
		ast_decref(loop);
err_loop:
		ast_xdecref(init);
		ast_xdecref(elem_or_cond);
		ast_xdecref(iter_or_next);
		goto err;
	}	break;

	case TPP_KWD_foreach: {
		DREF struct ast *foreach_elem;
		DREF struct ast *foreach_iter;
		DREF struct ast *foreach_loop;
		bool has_paren;
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		DeeLexer_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_foreach_flags:
			DeeLexer_NoLf_Break(self);
			goto err;
		}
		if (DeeLexer_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_FOR))
			goto err_foreach_flags;
		if unlikely(scope_push())
			goto err_foreach_flags;
		foreach_elem = ast_parse_comma(self,
		                               AST_COMMA_ALLOWVARDECLS,
		                               AST_FMULTIPLE_TUPLE,
		                               NULL);
		if unlikely(!foreach_elem)
			goto err_foreach_flags;
		if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(':'), W_EXPECTED_COLON_AFTER_FOREACH)) {
err_foreach_flags_elem:
			DeeLexer_NoLf_Break(self);
			goto err_foreach_flags;
		}
		foreach_iter = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
		if unlikely(!foreach_iter)
			goto err_foreach_flags_elem;
		DeeLexer_NoLf_Pop(self);
		if (DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_FOR))
			goto err_foreach_iter;
		foreach_loop = ast_parse_statement(self, allow_nonblock);
		if unlikely(!foreach_loop)
			goto err_foreach_iter;
		result = ast_loop(AST_FLOOP_FOREACH, foreach_elem, foreach_iter, foreach_loop);
		result = ast_setddi(result, &loc);
		ast_decref(foreach_loop);
		ast_decref(foreach_iter);
		ast_decref(foreach_elem);
		scope_pop();
		break;
err_foreach_iter:
		ast_decref(foreach_iter);
/*err_foreach_elem:*/
		ast_decref(foreach_elem);
		goto err;
	}	break;

	case TPP_KWD_assert:
		result = ast_parse_assert(self, false);
		if unlikely(!result)
			goto err;
		if likely(is_semicolon(DeeLexer_GetTok(self))) {
			if unlikely(yield_semicolonnbif(self, allow_nonblock) < 0)
				goto err_r;
		} else {
			if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), TPP_W_EXPECTED_SEMICOLON_AFTER_ASSERT))
				goto err_r;
		}
		break;

	case TPP_KWD_do: {
		DREF struct ast *cond;
		bool has_paren;
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		result = ast_parse_statement(self, false);
		if unlikely(!result)
			goto err;

		/* Allow tags before the `while` keyword (forward-compatibility...) */
		if unlikely(ast_tags_clear(self))
			goto err_r;
		if unlikely(skip_lf(self))
			goto err_r;
		if unlikely(parse_tags_block(self))
			goto err_r;
		DeeLexer_NoLf_Push(self);
		if unlikely(skip_lf(self)) {
err_r_do_flags:
			DeeLexer_NoLf_Break(self);
			goto err_r;
		}
		if (DeeLexer_Skip2(self, TPP_KWD_while, W_EXPECTED_WHILE_AFTER_DO))
			goto err_r_do_flags;
		if (DeeLexer_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_WHILE))
			goto err_r_do_flags;
		cond = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
		DeeLexer_NoLf_Pop(self);
		if (DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_WHILE))
			goto err_r;
		merge = ast_loop(AST_FLOOP_POSTCOND, cond, NULL, result);
		merge = ast_setddi(merge, &loc);
		ast_decref(result);
		ast_decref(cond);
		if unlikely(!merge)
			goto err;
		result = merge;
		if likely(is_semicolon(DeeLexer_GetTok(self))) {
			if unlikely(yield_semicolonnbif(self, allow_nonblock) < 0)
				goto err_r;
		} else {
			if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), TPP_W_EXPECTED_SEMICOLON_AFTER_DOWHILE))
				goto err_r;
		}
	}	break;

	case TPP_KWD_while: {
		DREF struct ast *loop;
		bool has_paren;
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if unlikely(scope_push() < 0)
			goto err;
		DeeLexer_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_while_flags:
			DeeLexer_NoLf_Break(self);
			goto err;
		}
		if (DeeLexer_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_WHILE))
			goto err_while_flags;
		result = ast_parse_comma(self,
		                         AST_COMMA_NORMAL |
		                         AST_COMMA_ALLOWVARDECLS,
		                         AST_FMULTIPLE_KEEPLAST,
		                         NULL);
		DeeLexer_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		if (DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_WHILE))
			goto err_r;
		loop = ast_parse_statement(self, allow_nonblock);
		if unlikely(!loop)
			goto err_r;
		merge = ast_loop(AST_FNORMAL, result, NULL, loop);
		merge = ast_setddi(merge, &loc);
		ast_decref(loop);
		ast_decref(result);
		if unlikely(!merge)
			goto err;
		scope_pop();
		result = merge;
	}	break;

	case TPP_KWD_break:
	case TPP_KWD_continue: {
#if AST_FLOOPCTL_BRK + 1 == AST_FLOOPCTL_CON
		STATIC_ASSERT(AST_FLOOPCTL_BRK == (TPP_KWD_break - TPP_KWD_break));
		STATIC_ASSERT(AST_FLOOPCTL_CON == (TPP_KWD_continue - TPP_KWD_break));
		result = ast_loopctl((uint16_t)(DeeLexer_GetTok(self) - TPP_KWD_break));
#else /* AST_FLOOPCTL_BRK + 1 == AST_FLOOPCTL_CON */
		result = ast_loopctl(DeeLexer_GetTok(self) == TPP_KWD_break ? AST_FLOOPCTL_BRK : AST_FLOOPCTL_CON);
#endif /* AST_FLOOPCTL_BRK + 1 != AST_FLOOPCTL_CON */
		result = ast_sethere(self, result);
		if unlikely(!result)
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_r;
		if likely(is_semicolon(DeeLexer_GetTok(self))) {
			if unlikely(yield_semicolonnbif(self, allow_nonblock) < 0)
				goto err_r;
		} else {
			if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), TPP_W_EXPECTED_SEMICOLON_AFTER_BREAK))
				goto err_r;
		}
	}	break;

	case TPP_KWD_with:
		result = ast_parse_with(self, true, allow_nonblock);
		break;

	case TPP_KWD_try:
		result = ast_parse_try(self, true);
		/* Don't reset tags after a try-statement,
		 * because we've already handled tags for the next statement
		 * before noticing that they in fact weren't designated for
		 * yet another catch/finally block that never came:
		 * >> try {
		 * >>     print "Hello";
		 * >> } @[interrupt] catch (...) {
		 * >>     print "Error";
		 * >> }
		 * >>
		 * >> @@Documentation text // We've already parsed this due to the chance of this being followed by `catch` or `finally`
		 * >> global foo = 42;
		 */
		goto done_no_tag_reset;

	case TPP_KWD_del:
		/* Delete statement. */
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if (DeeLexer_GetTok(self) == '(' || DeeLexer_GetTok(self) == TPP_KWD_pack) {
			bool has_paren;
			/* Del with parenthesis (like in expressions)
			 * For that reason, don't allow allow the actual symbols being removed, either. */
			DeeLexer_NoLf_Push(self);
			has_paren = DeeLexer_GetTok(self) == '(';
			if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_del_flags:
				DeeLexer_NoLf_Break(self);
				goto err;
			}
			if (!has_paren && DeeLexer_GetTok(self) == '(') {
				has_paren = true;
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err_del_flags;
			}
			result = ast_parse_del(self, LOOKUP_SYM_NORMAL);
			DeeLexer_NoLf_Pop(self);
			if unlikely(!result)
				goto err;
			if (has_paren) {
				if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(')'), W_EXPECTED_RPAREN_AFTER_DEL))
					goto err_r;
			}
		} else {
			result = ast_parse_del(self, LOOKUP_SYM_ALLOWDECL);
			if unlikely(!result)
				goto err;
		}
		result = ast_putddi(result, &loc);
		if likely(is_semicolon(DeeLexer_GetTok(self))) {
			if unlikely(yield_semicolonnbif(self, allow_nonblock) < 0)
				goto err_r;
		} else {
			if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), TPP_W_EXPECTED_SEMICOLON_AFTER_DEL))
				goto err_r;
		}
		break;

	case TPP_KWD___asm:
	case TPP_KWD___asm__:
		result = ast_parse_asm(self);
		if likely(is_semicolon(DeeLexer_GetTok(self))) {
			if unlikely(yield_semicolonnbif(self, allow_nonblock) < 0)
				goto err_r;
		} else {
			if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), TPP_W_EXPECTED_SEMICOLON_AFTER_ASM))
				goto err_r;
		}
		break;

	case TPP_KWD_goto: {
		struct text_label *goto_label;
		/* Create a new goto-branch. */
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err;
		if (DeeLexer_HasTokenKwd(self)) {
			goto_label = lookup_label(DeeLexer_GetTokenKwd(self));
			if unlikely(!goto_label)
				goto err;
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err;
		} else {
			if (DeeLexer_Warnf(self, TPP_W_EXPECTED_KEYWORD_AFTER_GOTO))
				goto err;
			goto_label = lookup_label(tpp_builtin_getkeyword_empty());
			if unlikely(!goto_label)
				goto err;
		}
		result = ast_goto(goto_label, current_basescope);
		result = ast_setddi(result, &loc);
		if unlikely(!result)
			goto err;
		if likely(is_semicolon(DeeLexer_GetTok(self))) {
			if unlikely(yield_semicolonnbif(self, allow_nonblock) < 0)
				goto err_r;
		} else {
			if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(';'), TPP_W_EXPECTED_SEMICOLON_AFTER_GOTO))
				goto err_r;
		}
	}	break;

	case TPP_KWD_switch: {
		uint16_t old_scope_flags;
		struct text_label *old_cases;
		struct text_label *old_default;
		struct text_label *switch_cases;
		struct text_label *switch_default;
		DREF struct ast *switch_block;
		bool has_paren;

		/* Switch statement. */
		if (DeeLexer_GetLoc(self, &loc))
			goto err;
		DeeLexer_NoLf_Push(self);
		if (TPP_TOK_ISERR(DeeLexer_Yield(self))) {
err_switch_flags:
			DeeLexer_NoLf_Break(self);
			goto err;
		}
		if (DeeLexer_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_SWITCH))
			goto err_switch_flags;

		/* Parse the switch-expression (NOTE: Allow variable declarations). */
		result = ast_parse_comma(self,
		                         AST_COMMA_NORMAL |
		                         AST_COMMA_ALLOWVARDECLS,
		                         AST_FMULTIPLE_KEEPLAST,
		                         NULL);
		DeeLexer_NoLf_Pop(self);
		if unlikely(!result)
			goto err;
		if (DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_SWITCH))
			goto err_r;

		/* Setup + activate switch-mode. */
		old_scope_flags = current_basescope->bs_cflags;
		current_basescope->bs_cflags |= BASESCOPE_FSWITCH;
		old_cases                    = current_basescope->bs_swcase;
		old_default                  = current_basescope->bs_swdefl;
		current_basescope->bs_swcase = NULL;
		current_basescope->bs_swdefl = NULL;

		switch_block = ast_parse_statement(self, allow_nonblock);

		/* Extract switch cases. */
		switch_cases   = current_basescope->bs_swcase;
		switch_default = current_basescope->bs_swdefl;

		/* Restore the old state of the FSWITCH-flag. */
		current_basescope->bs_swdefl = old_default;
		current_basescope->bs_swcase = old_cases;
		current_basescope->bs_cflags &= ~BASESCOPE_FSWITCH;
		current_basescope->bs_cflags |= old_scope_flags & BASESCOPE_FSWITCH;

		if unlikely(!switch_block)
			goto err_r_switch;

		/* Since cases are currently in order of
		 * last -> first, we must reverse that order. */
		{
			struct text_label *chain;
			struct text_label *next;
			chain        = switch_cases;
			switch_cases = NULL;
			while (chain) {
				next           = chain->tl_next;
				chain->tl_next = switch_cases;
				switch_cases   = chain;
				chain          = next;
			}
		}

		/* With the switch and associated block parsed,
		 * pack them together into a SWITCH-ast. */
		merge = ast_setddi(ast_switch(AST_FSWITCH_NORMAL,
		                              result, switch_block,
		                              switch_cases, switch_default),
		                   &loc);
		if unlikely(!merge)
			goto err_r_switch_block;

		/* Cleanup + assign the new switch statement to the return value. */
		ast_decref(switch_block);
		ast_decref(result);
		result = merge;
		break;
err_r_switch_block:
		ast_decref(switch_block);
err_r_switch:
		/* Cleanup switch cases + default. */
		cleanup_switch_cases(switch_cases,
		                     switch_default);
		goto err_r;
	}	break;

	default:
		if (DeeLexer_HasTokenKwd(self)) {
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
			tpp_token_id next_token;
			next_token = tpp_lexer_peek_raw(&self->dl_lexer,
			                                TPP_LEXER_PEEK_RAW_FLAG_NORMAL,
			                                NULL, NULL, NULL);
			if (TPP_TOK_ISERR(next_token))
				goto err;
			if (next_token == ':')
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
			char const *next_token;
			next_token = peek_next_token(NULL);
			if unlikely(!next_token)
				goto err;
			if (*next_token == ':' &&
			    (next_token = advance_wraplf(next_token),
			     *next_token != ':' && *next_token != '='))
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
			{
				/* Define a label. */
				struct text_label *def_label;
				DREF struct ast *label_ast;
				uint16_t label_flags;
				if (DeeLexer_GetLoc(self, &loc))
					goto err;
				def_label   = lookup_label(DeeLexer_GetTokenKwd(self));
				label_flags = AST_FLABEL_NORMAL;
				if unlikely(!def_label)
					goto err;
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err; /* Label name */
				if unlikely(skip_lf(self))
					goto err;
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err; /* `:` token. */
handle_post_label:
				if unlikely(skip_lf(self))
					goto err;
				if unlikely(DeeLexer_GetTok(self) == '}') {
					/* Emit a warning and when the next token is a `}` */
					if (DeeLexer_Warnf(self, TPP_W_MISSING_STATEMENT_AFTER_LABEL))
						goto err;
					result = ast_constexpr(Dee_None);
					if unlikely(!result)
						goto err;
					label_ast = ast_setddi(ast_label(label_flags, def_label, current_basescope), &loc);
					if unlikely(!label_ast)
						goto err_r;
				} else {
					label_ast = ast_setddi(ast_label(label_flags, def_label, current_basescope), &loc);
					if unlikely(!label_ast)
						goto err;

					/* Parse the statement that is prefixed by the label. */
					result = ast_parse_statement(self, allow_nonblock);
					if unlikely(!result) {
						ast_decref(label_ast);
						goto err;
					}
				}
				if (!ast_shared(result) &&
				    result->a_type == AST_MULTIPLE &&
				    result->a_flag == AST_FMULTIPLE_KEEPLAST) {
					/* Prepend the label AST before all the others in the MULTIPLE-ast. */
					DREF struct ast **elemv;
					elemv = (DREF struct ast **)Dee_Reallocc(result->a_multiple.m_astv,
					                                         result->a_multiple.m_astc + 1,
					                                         sizeof(DREF struct ast *));
					if unlikely(!elemv)
						goto err_label_ast;
					memmoveupc(elemv + 1,
					           elemv,
					           result->a_multiple.m_astc,
					           sizeof(DREF struct ast *));
					result->a_multiple.m_astc += 1;
					result->a_multiple.m_astv = elemv;
					elemv[0]                  = label_ast; /* Inherit reference. */
				} else {
					/* Create a new MULTIPLE-ast */
					DREF struct ast **elemv;
					elemv = (DREF struct ast **)Dee_Mallocc(2, sizeof(DREF struct ast *));
					if unlikely(!elemv) {
err_label_ast:
						ast_decref(label_ast);
						goto err_r;
					}
					elemv[0] = label_ast; /* Inherit reference. */
					elemv[1] = result;    /* Inherit reference. */
					merge    = ast_multiple(AST_FMULTIPLE_KEEPLAST, 2, elemv);
					if unlikely(!merge) {
						Dee_Free(elemv);
						goto err_label_ast;
					}
					result = merge;
				}
				break;
	case TPP_KWD_case:
				if unlikely(!(current_basescope->bs_cflags & BASESCOPE_FSWITCH)) {
					if (DeeLexer_Warnf(self, TPP_W_NOT_INSIDE_A_SWITCH_STATEMENT))
						goto err;
				}
				if (DeeLexer_GetLoc(self, &loc))
					goto err;
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;

				/* Parse the case expression. */
				result = ast_parse_expr(self, LOOKUP_SYM_NORMAL);
				if unlikely(!result)
					goto err;
				if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(':'), W_EXPECTED_COLON_AFTER_CASE))
					goto err_r;
				if unlikely(!(current_basescope->bs_cflags & BASESCOPE_FSWITCH)) {
					ast_decref(result);
					goto again;
				}

				/* Create the new text label that will be used as case. */
				def_label = new_case_label(result);
				ast_decref(result);
				if unlikely(!def_label)
					goto err;
				label_flags = AST_FLABEL_CASE;
				goto handle_post_label;
	case TPP_KWD_default:
				if unlikely(!(current_basescope->bs_cflags & BASESCOPE_FSWITCH)) {
					if (DeeLexer_Warnf(self, TPP_W_NOT_INSIDE_A_SWITCH_STATEMENT))
						goto err;
				} else if unlikely(current_basescope->bs_swdefl) {
					/* Warn if another default label had already been defined. */
					if (DeeLexer_Warnf(self, TPP_W_DEFAULT_LABEL_HAD_ALREADY_BEEN_DEFINED))
						goto err;
				}
				if (DeeLexer_GetLoc(self, &loc))
					goto err;
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err;
				if (DeeLexer_Skip2(self, TPP_TOK_OFCHAR(':'), W_EXPECTED_COLON_AFTER_DEFAULT))
					goto err;
				if unlikely(!(current_basescope->bs_cflags & BASESCOPE_FSWITCH))
					goto again;

				/* Ensure existence of the default label. */
				def_label = new_default_label();
				if unlikely(!def_label)
					goto err;
				label_flags = AST_FLABEL_CASE;
				goto handle_post_label;
			}
		}

		/* Parse a comma-separated expression in style of keep-last. */
		result = ast_parse_comma(self,
		                         allow_nonblock
		                         ? (AST_COMMA_NORMAL | AST_COMMA_ALLOWVARDECLS |
		                            AST_COMMA_ALLOWTYPEDECL | AST_COMMA_PARSESEMI |
		                            AST_COMMA_ALLOWNONBLOCK)
		                         : (AST_COMMA_NORMAL | AST_COMMA_ALLOWVARDECLS |
		                            AST_COMMA_ALLOWTYPEDECL | AST_COMMA_PARSESEMI),
		                         AST_FMULTIPLE_KEEPLAST,
		                         NULL);
		/*if unlikely(!result)
			goto err;*/
		break;
	}
	/* Clear tags at the end of each statement. */
	if unlikely(ast_tags_clear(self))
		goto err_r;
done_no_tag_reset:
	return result;
err_r:
	ast_decref(result);
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_STATEMENT_C */
