/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_LEXER_STATEMENT_C
#define GUARD_DEEMON_COMPILER_LEXER_STATEMENT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/tpp.h>
#include <deemon/none.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>

DECL_BEGIN

#define is_semicollon() (tok == ';' || tok == '\n')

PRIVATE tok_t DCALL yield_semicollonnbif(bool allow_nonblock) {
	tok_t result = yieldnbif(allow_nonblock);
	if (result == '\n') {
		uint32_t old_flags;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		result = yieldnbif(allow_nonblock);
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
	}
	return result;
}

INTERN int DCALL skip_lf(void) {
	if (tok == '\n') {
		uint32_t old_flags;
		tok_t error;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		error = yield();
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if unlikely(error < 0)
			goto err;
	}
	return 0;
err:
	return -1;
}



/* Parse the head header of a for-statement, returning the appropriate
 * AST flags for creating the loop (usually `AST_FLOOP_NORMAL' or `AST_FLOOP_FOREACH'),
 * as well as filling in the given pointers to used asts.
 * NOTE: The caller is responsible for wrapping this function in its own
 *       scope, should they choose to with initializers/loop element symbols
 *       to be placed in their own scope.
 * NOTE: Any of the given pointers may be filled with NULL if that AST is not present,
 *       unless the loop is actually a foreach-loop, in which case they _must_ always
 *       be present.
 * WARNING: The caller is responsible for wrapping `*piter_or_next' in an `__iterself__()'
 *          operator call when `AST_FLOOP_FOREACH' is part of the return mask, unless they wish
 *          to enumerate an iterator itself (which is possible using the `__foreach' statement). */
INTERN WUNUSED NONNULL((1, 2, 3)) int32_t DCALL
ast_parse_for_head(DREF struct ast **__restrict pinit,
                   DREF struct ast **__restrict pelem_or_cond,
                   DREF struct ast **__restrict piter_or_next) {
	int32_t result                = AST_FLOOP_NORMAL;
	DREF struct ast *init         = NULL;
	DREF struct ast *elem_or_cond = NULL;
	DREF struct ast *iter_or_next = NULL;
	if (tok != ';') {
		init = ast_parse_comma(AST_COMMA_ALLOWVARDECLS,
		                       AST_FMULTIPLE_TUPLE,
		                       NULL);
		if unlikely(!init)
			goto err;
		if (tok == ':') {
			if unlikely(yield() < 0)
				goto err;
			/* foreach-style loop. */
			elem_or_cond = init;
			init         = NULL;
			result |= AST_FLOOP_FOREACH;
			iter_or_next = ast_parse_expr(LOOKUP_SYM_NORMAL);
			if unlikely(!iter_or_next)
				goto err;
			goto done;
		}
	}
	if (skip(';', W_EXPECTED_SEMICOLLON1_AFTER_FOR))
		goto err;
	if (tok != ';') {
		elem_or_cond = ast_parse_expr(LOOKUP_SYM_NORMAL);
		if unlikely(!elem_or_cond)
			goto err;
	}
	if (skip(';', W_EXPECTED_SEMICOLLON2_AFTER_FOR))
		goto err;
	if (tok == ')') {
		iter_or_next = NULL;
	} else {
		iter_or_next = ast_parse_expr(LOOKUP_SYM_NORMAL);
		if unlikely(!iter_or_next)
			goto err;
	}
done:
	*pinit         = init;
	*pelem_or_cond = elem_or_cond;
	*piter_or_next = iter_or_next;
	return result;
err:
	ast_xdecref(init);
	ast_xdecref(elem_or_cond);
	ast_xdecref(iter_or_next);
	return -1;
}



/* Parse a sequence of statements until `end_token' is
 * encountered at the start of a statement, or until
 * the end of the current input-file-stack is reached.
 * NOTE: The returned ast is usually an `AST_MULTIPLE',
 *       which will have the given `flags' assigned.
 * WARNING: If only a single AST would be contained
 *          and `flags' is `AST_FMULTIPLE_KEEPLAST',
 *          the inner expression is automatically
 *          returned instead.
 * NOTE: If desired, the caller is responsible to setup
 *       or teardown a new scope before/after this function. */
INTERN WUNUSED DREF struct ast *DCALL
ast_parse_statements_until(uint16_t flags, tok_t end_token) {
	size_t exprc, expra;
	DREF struct ast **exprv;
	DREF struct ast *new_expression;
	unsigned long token_num;
	exprc = expra = 0, exprv = NULL;
	for (;;) {
		if unlikely(skip_lf())
			goto err;
		if (!tok || tok == end_token)
			break;
		token_num      = token.t_num;
		new_expression = ast_parse_statement(false);
		if unlikely(!new_expression)
			goto err;
		ASSERT(exprc <= expra);
		if (exprc == expra) {
			DREF struct ast **new_exprv;
			size_t new_expra = expra * 2;
			if (!new_expra)
				new_expra = 8;
do_realloc:
			new_exprv = (DREF struct ast **)Dee_TryRealloc(exprv, new_expra *
			                                                      sizeof(DREF struct ast *));
			if unlikely(!new_exprv) {
				if (new_expra != exprc + 1) {
					new_expra = exprc + 1;
					goto do_realloc;
				}
				if (Dee_CollectMemory(new_expra * sizeof(DREF struct ast *)))
					goto do_realloc;
				goto err;
			}
			exprv = new_exprv;
			expra = new_expra;
		}
		exprv[exprc++] = new_expression; /* Inherit reference. */
		if (token_num == token.t_num) {
			if unlikely(WARN(W_FAILED_TO_PARSE_STATEMENT))
				goto err;
			if unlikely(yield() < 0)
				goto err;
		}
	}
	/* Truncate the expression buffer to what is actually being used. */
	if (exprc != expra) {
		DREF struct ast **new_exprv;
		new_exprv = (DREF struct ast **)Dee_TryRealloc(exprv, exprc * sizeof(DREF struct ast *));
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
	while (exprc--)
		ast_decref(exprv[exprc]);
	Dee_Free(exprv);
	return NULL;
}


INTDEF void DCALL
cleanup_switch_cases(struct text_label *switch_cases,
                     struct text_label *switch_default);

/* Parse a regular, old statement. */
INTERN WUNUSED DREF struct ast *DCALL
ast_parse_statement(bool allow_nonblock) {
	DREF struct ast *result, *merge;
	struct ast_loc loc;
	uint32_t old_flags;
again:
	switch (tok) {

	case '@':
		/* Parse tags. */
		if unlikely(yield() < 0)
			goto err;
		if (parse_tags())
			goto err;
		goto again;

	case '{':
		loc_here(&loc);
		if unlikely(scope_push() < 0)
			goto err;
		if unlikely(yield() < 0)
			goto err;
		/* Enter a new scope and parse expressions. */
		result = ast_putddi(ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST, '}'), &loc);
		if unlikely(!result)
			goto err;
		while (tok == '\n')
			if unlikely(yield() < 0)
		goto err_r;
		if (skip('}', W_EXPECTED_RBRACE_AFTER_LBRACE))
			goto err_r;
		scope_pop();
		break;

	case '\n':
		/* Skip empty, leading lines in statements. */
		if unlikely(skip_lf())
			goto err;
		goto again;

	case ';':
		result = ast_sethere(ast_constexpr(Dee_None));
		if unlikely(yieldnbif(allow_nonblock) < 0)
			goto err;
		break;

	case KWD_if: {
		DREF struct ast *tt_branch;
		DREF struct ast *ff_branch;
		uint16_t expect;
		/* If-statement. */
		loc_here(&loc);
		expect = current_tags.at_expect;
		if unlikely(scope_push() < 0)
			goto err;
		if unlikely(yield() < 0)
			goto err;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if (skip('(', W_EXPECTED_LPAREN_AFTER_IF))
			goto err_flags;
		result = ast_parse_comma(AST_COMMA_NORMAL |
		                         AST_COMMA_ALLOWVARDECLS,
		                         AST_FMULTIPLE_KEEPLAST,
		                         NULL);
		if unlikely(!result)
			goto err_flags;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (skip(')', W_EXPECTED_RPAREN_AFTER_IF))
			goto err_r;
		tt_branch = ast_parse_statement(false);
		if unlikely(!tt_branch)
			goto err_r;
		ff_branch = NULL;
		/* Allow tags before the `else' keyword (forward-compatibility...) */
		if unlikely(ast_tags_clear())
			goto err_tt_branch;
		if unlikely(skip_lf())
			goto err_tt_branch;
		if unlikely(parse_tags_block()) {
err_tt_branch:
			ast_decref(tt_branch);
			goto err_r;
		}
		if unlikely(skip_lf())
			goto err_tt_branch;
		if (tok == KWD_elif) {
			token.t_id = KWD_if; /* Cheat a bit... */
			goto do_else_branch;
		}
		if (tok == KWD_else) {
			if unlikely(yield() < 0)
				goto err_tt_branch;
do_else_branch:
			ff_branch = ast_parse_statement(allow_nonblock);
			if unlikely(!ff_branch)
				goto err_tt_branch;
		}
		merge = ast_setddi(ast_conditional(AST_FCOND_EVAL | expect,
		                                   result,
		                                   tt_branch,
		                                   ff_branch),
		                   &loc);
		scope_pop();
		ast_xdecref(ff_branch);
		ast_decref(tt_branch);
		ast_decref(result);
		result = merge;
		/* We've already parsed tags, so don't reset them before parsing the next statement. */
		goto done_no_tag_reset;
	}

	case KWD_return:
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		if (is_semicollon()) {
			/* Special case: A return without an operator is allowed
			 *               in both return and yield functions. */
			result = ast_return(NULL);
		} else {
			if ((current_basescope->bs_flags & CODE_FYIELDING) &&
			    WARN(W_RETURN_IN_YIELD_FUNCTION))
				goto err;
			current_basescope->bs_cflags |= BASESCOPE_FRETURN;
			result = ast_parse_comma(AST_COMMA_NORMAL,
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
		if unlikely(likely(is_semicollon())
		            ? (yield_semicollonnbif(allow_nonblock) < 0)
		            : WARN(W_EXPECTED_SEMICOLLON_AFTER_RETURN))
			goto err_r;
		break;

	case KWD_yield:
		/* TODO: Warn about use of non-portable extension if this yield
		 *       statement appears inside of a statement-expression, or
		 *       a finally/catch block. */
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		result = ast_parse_comma(AST_COMMA_NORMAL |
		                         AST_COMMA_FORCEMULTIPLE,
		                         AST_FMULTIPLE_TUPLE,
		                         NULL);
		if unlikely(!result)
			goto err;
		merge = ast_setddi(ast_expand(result), &loc);
		ast_decref(result);
		if unlikely(!merge)
			goto err;
		result = ast_setddi(ast_yield(merge), &loc);
		ast_decref(merge);
		if unlikely(!result)
			goto err;
		current_basescope->bs_flags |= CODE_FYIELDING;
		if (current_basescope->bs_cflags & BASESCOPE_FRETURN &&
		    WARN(W_YIELD_AFTER_RETURN))
			goto err;
		if unlikely(likely(is_semicollon())
		            ? (yield_semicollonnbif(allow_nonblock) < 0)
		            : WARN(W_EXPECTED_SEMICOLLON_AFTER_YIELD))
			goto err_r;
		break;

	case KWD_from:
	case KWD_import:
		result = ast_parse_import();
		if unlikely(!result)
			goto err;
		if unlikely(likely(is_semicollon())
		            ? (yield_semicollonnbif(allow_nonblock) < 0)
		            : WARN(W_EXPECTED_SEMICOLLON_AFTER_IMPORT))
			goto err_r;
		break;

	case KWD_throw:
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		if (is_semicollon()) {
			result = ast_throw(NULL);
		} else {
			result = ast_parse_comma(AST_COMMA_NORMAL,
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
		if unlikely(likely(is_semicollon())
		            ? (yield_semicollonnbif(allow_nonblock) < 0)
		            : WARN(W_EXPECTED_SEMICOLLON_AFTER_THROW))
			goto err_r;
		break;

	case KWD_print:
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		if (tok == ',') {
			/* `print,;' --> `none' */
			if unlikely(yield() < 0)
				goto err;
			result = ast_constexpr(Dee_None);
			if unlikely(!result)
				goto err;
		} else if (is_semicollon()) {
			/* `print;' --> `print pack()...;' */
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
			result = ast_parse_comma(AST_COMMA_FORCEMULTIPLE |
			                         AST_COMMA_STRICTCOMMA,
			                         AST_FMULTIPLE_TUPLE,
			                         NULL);
			if unlikely(!result)
				goto err;
			if (tok == ':') {
				DREF struct ast *text;
				/* This is actually an fprint-style statement: `print foo: "bar";' */
				if unlikely(yield() < 0)
					goto err_r;
				/* Since we've forced a multiple-parse above, we must now extract the target file. */
				if (result->a_type == AST_MULTIPLE &&
				    result->a_multiple.m_astc == 1) {
					merge = result->a_multiple.m_astv[0];
					ast_incref(merge);
					ast_decref(result);
					result = merge;
				}
				if (is_semicollon()) {
					text = ast_sethere(ast_constexpr(Dee_EmptyTuple));
				} else {
					text = ast_parse_comma(AST_COMMA_FORCEMULTIPLE |
					                       AST_COMMA_STRICTCOMMA,
					                       AST_FMULTIPLE_TUPLE,
					                       NULL);
				}
				if unlikely(!text)
					goto err_r;
				merge = ast_action2(tok == ','
				                    ? AST_FACTION_FPRINT
				                    : AST_FACTION_FPRINTLN,
				                    result, text);
				ast_decref(text);
				ast_decref(result);
				if unlikely(!merge)
					goto err;
				result = merge;
				if (tok == ',' && yield() < 0)
					goto err_r;
			} else {
				/* Print-to-stdout statement. */
				merge = ast_action1(tok == ','
				                    ? AST_FACTION_PRINT
				                    : AST_FACTION_PRINTLN,
				                    result);
				ast_decref(result);
				if unlikely(!merge)
					goto err;
				result = merge;
				if (tok == ',' && yield() < 0)
					goto err_r;
			}
		}
		ast_setddi(result, &loc);
		if unlikely(likely(is_semicollon())
		            ? (yield_semicollonnbif(allow_nonblock) < 0)
		            : WARN(W_EXPECTED_SEMICOLLON_AFTER_PRINT))
			goto err_r;
		break;

	case KWD_for: {
		DREF struct ast *init;
		DREF struct ast *elem_or_cond;
		DREF struct ast *iter_or_next;
		DREF struct ast *loop;
		int32_t type;
		bool has_scope;
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if (skip('(', W_EXPECTED_LPAREN_AFTER_FOR))
			goto err_flags;
		has_scope = false;
		if (tok != ';') {
			/* To save on space, only create a loop scope when an initializer was given. */
			if unlikely(scope_push())
				goto err_flags;
			has_scope = true;
		}
		type = ast_parse_for_head(&init, &elem_or_cond, &iter_or_next);
		if unlikely(type < 0)
			goto err_flags;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (type & AST_FLOOP_FOREACH && iter_or_next) {
			/* Wrap the iterator of a foreach-loop with an __iterself__ operator. */
			merge = ast_setddi(ast_operator1(OPERATOR_ITERSELF,
			                                 AST_OPERATOR_FNORMAL,
			                                 iter_or_next),
			                   &loc);
			if unlikely(!merge)
				goto err_loop;
			ast_decref(iter_or_next);
			iter_or_next = merge;
		}
		if (skip(')', W_EXPECTED_RPAREN_AFTER_FOR))
			goto err_loop;

		loop = ast_parse_statement(allow_nonblock);
		if unlikely(!loop)
			goto err_loop;
		/* Create the loop branch. */
		result = ast_setddi(ast_loop((uint16_t)type,
		                             elem_or_cond,
		                             iter_or_next,
		                             loop),
		                    &loc);
		if unlikely(!result)
			goto err_loop2;
		ast_decref(loop);
		ast_xdecref(iter_or_next);
		ast_xdecref(elem_or_cond);
		if (init) {
			DREF struct ast **exprv = (DREF struct ast **)Dee_Malloc(2 * sizeof(DREF struct ast *));
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

	case KWD_foreach: {
		DREF struct ast *foreach_elem;
		DREF struct ast *foreach_iter;
		DREF struct ast *foreach_loop;
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if (skip('(', W_EXPECTED_LPAREN_AFTER_FOR))
			goto err_flags;
		if unlikely(scope_push())
			goto err_flags;
		foreach_elem = ast_parse_comma(AST_COMMA_ALLOWVARDECLS,
		                               AST_FMULTIPLE_TUPLE,
		                               NULL);
		if unlikely(!foreach_elem)
			goto err_flags;
		if (skip(':', W_EXPECTED_COLLON_AFTER_FOREACH))
			goto err_foreach_elem;
		foreach_iter = ast_parse_expr(LOOKUP_SYM_NORMAL);
		if unlikely(!foreach_iter)
			goto err_foreach_elem;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (skip(')', W_EXPECTED_RPAREN_AFTER_FOR))
			goto err_foreach_iter;
		foreach_loop = ast_parse_statement(allow_nonblock);
		if unlikely(!foreach_loop)
			goto err_foreach_iter;
		result = ast_setddi(ast_loop(AST_FLOOP_FOREACH,
		                             foreach_elem,
		                             foreach_iter,
		                             foreach_loop),
		                    &loc);
		ast_decref(foreach_loop);
		ast_decref(foreach_iter);
		ast_decref(foreach_elem);
		scope_pop();
		break;
err_foreach_iter:
		ast_decref(foreach_iter);
err_foreach_elem:
		ast_decref(foreach_elem);
		goto err_flags;
	}	break;

	case KWD_assert:
		result = ast_parse_assert(false);
		if unlikely(!result)
			goto err;
		if unlikely(likely(is_semicollon())
		            ? (yield_semicollonnbif(allow_nonblock) < 0)
		            : WARN(W_EXPECTED_SEMICOLLON_AFTER_ASSERT))
			goto err_r;
		break;

	case KWD_do: {
		DREF struct ast *cond;
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		result = ast_parse_statement(false);
		if unlikely(!result)
			goto err;
		/* Allow tags before the `while' keyword (forward-compatibility...) */
		if unlikely(ast_tags_clear())
			goto err_r;
		if unlikely(skip_lf())
			goto err_r;
		if unlikely(parse_tags_block())
			goto err_r;
		if unlikely(skip_lf())
			goto err_r;
		if (skip(KWD_while, W_EXPECTED_WHILE_AFTER_DO))
			goto err_r;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if (skip('(', W_EXPECTED_LPAREN_AFTER_WHILE))
			goto err_r_flags;
		cond = ast_parse_expr(LOOKUP_SYM_NORMAL);
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (skip(')', W_EXPECTED_RPAREN_AFTER_WHILE))
			goto err_r;
		merge = ast_setddi(ast_loop(AST_FLOOP_POSTCOND, cond, NULL, result), &loc);
		ast_decref(result);
		ast_decref(cond);
		if unlikely(!merge)
			goto err;
		result = merge;
		if unlikely(likely(is_semicollon())
		            ? (yield_semicollonnbif(allow_nonblock) < 0)
		            : WARN(W_EXPECTED_SEMICOLLON_AFTER_DOWHILE))
			goto err_r;
	}	break;

	case KWD_while: {
		DREF struct ast *loop;
		loc_here(&loc);
		if unlikely(scope_push() < 0)
			goto err;
		if unlikely(yield() < 0)
			goto err;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if (skip('(', W_EXPECTED_LPAREN_AFTER_WHILE))
			goto err_flags;
		result = ast_parse_comma(AST_COMMA_NORMAL |
		                         AST_COMMA_ALLOWVARDECLS,
		                         AST_FMULTIPLE_KEEPLAST,
		                         NULL);
		if unlikely(!result)
			goto err_flags;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (skip(')', W_EXPECTED_RPAREN_AFTER_WHILE))
			goto err_r;
		loop = ast_parse_statement(allow_nonblock);
		if unlikely(!loop)
			goto err_r;
		merge = ast_setddi(ast_loop(AST_FNORMAL, result, NULL, loop), &loc);
		ast_decref(loop);
		ast_decref(result);
		if unlikely(!merge)
			goto err;
		scope_pop();
		result = merge;
	}	break;

	case KWD_break:
	case KWD_continue:
#if AST_FLOOPCTL_BRK + 1 == AST_FLOOPCTL_CON
		result = ast_loopctl((uint16_t)(tok - KWD_break));
#else
		result = ast_loopctl(tok == KWD_break ? AST_FLOOPCTL_BRK : AST_FLOOPCTL_CON);
#endif
		ast_sethere(result);
		if unlikely(!result)
			goto err;
		if unlikely(yield() < 0)
			goto err_r;
		if unlikely(likely(is_semicollon())
		            ? (yield_semicollonnbif(allow_nonblock) < 0)
		            : WARN(W_EXPECTED_SEMICOLLON_AFTER_BREAK))
			goto err_r;
		break;

	case KWD_with:
		result = ast_parse_with(true, allow_nonblock);
		break;

	case KWD_try:
		result = ast_parse_try(true);
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
		 * >> @"Documentation text" // We've already parsed this due to the chance of this being followed by `catch' or `finally'
		 * >> global foo = 42;
		 */
		goto done_no_tag_reset;

	case KWD_del:
		/* Delete statement. */
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		if (tok == '(') {
			/* Del with parenthesis (like in expressions)
			 * For that reason, don't allow allow the actual symbols being removed, either. */
			old_flags = TPPLexer_Current->l_flags;
			TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
			if unlikely(yield() < 0)
				goto err_flags;
			result = ast_parse_del(LOOKUP_SYM_NORMAL);
			if unlikely(!result)
				goto err_flags;
			TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
			if (skip(')', W_EXPECTED_RPAREN_AFTER_DEL))
				goto err_r;
		} else {
			result = ast_parse_del(LOOKUP_SYM_ALLOWDECL);
			if unlikely(!result)
				goto err;
		}
		ast_putddi(result, &loc);
		if unlikely(likely(is_semicollon())
		            ? (yield_semicollonnbif(allow_nonblock) < 0)
		            : WARN(W_EXPECTED_SEMICOLLON_AFTER_DEL))
			goto err_r;
		break;

	case KWD___asm:
	case KWD___asm__:
		result = ast_parse_asm();
		if unlikely(likely(is_semicollon())
		            ? (yield_semicollonnbif(allow_nonblock) < 0)
		            : WARN(W_EXPECTED_SEMICOLLON_AFTER_ASM))
			goto err_r;
		break;

	case KWD_goto: {
		struct text_label *goto_label;
		/* Create a new goto-branch. */
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		if (TPP_ISKEYWORD(tok)) {
			goto_label = lookup_label(token.t_kwd);
			if unlikely(!goto_label)
				goto err;
			if unlikely(yield() < 0)
				goto err;
		} else {
			if (WARN(W_EXPECTED_KEYWORD_AFTER_GOTO))
				goto err;
			goto_label = lookup_label(&TPPKeyword_Empty);
			if unlikely(!goto_label)
				goto err;
		}
		result = ast_goto(goto_label, current_basescope);
		if unlikely(!result)
			goto err;
		if unlikely(likely(is_semicollon())
		            ? (yield_semicollonnbif(allow_nonblock) < 0)
		            : WARN(W_EXPECTED_SEMICOLLON_AFTER_GOTO))
			goto err_r;
	}	break;

	case KWD_switch: {
		uint16_t old_scope_flags;
		struct text_label *old_cases;
		struct text_label *old_default;
		struct text_label *switch_cases;
		struct text_label *switch_default;
		DREF struct ast *switch_block;
		/* Switch statement. */
		loc_here(&loc);
		if unlikely(yield() < 0)
			goto err;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if (skip('(', W_EXPECTED_LPAREN_AFTER_SWITCH))
			goto err_flags;
		/* Parse the switch-expression (NOTE: Allow variable declarations). */
		result = ast_parse_comma(AST_COMMA_NORMAL |
		                         AST_COMMA_ALLOWVARDECLS,
		                         AST_FMULTIPLE_KEEPLAST,
		                         NULL);
		if unlikely(!result)
			goto err_flags;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (skip(')', W_EXPECTED_RPAREN_AFTER_SWITCH))
			goto err_r;

		/* Setup + activate switch-mode. */
		old_scope_flags = current_basescope->bs_cflags;
		current_basescope->bs_cflags |= BASESCOPE_FSWITCH;
		old_cases                    = current_basescope->bs_swcase;
		old_default                  = current_basescope->bs_swdefl;
		current_basescope->bs_swcase = NULL;
		current_basescope->bs_swdefl = NULL;

		switch_block = ast_parse_statement(allow_nonblock);

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
		if (TPP_ISKEYWORD(tok)) {
			char *next_token;
			next_token = peek_next_token(NULL);
			if unlikely(!next_token)
				goto err;
			if (*next_token == ':' &&
			    (next_token = advance_wraplf(next_token),
			     *next_token != ':' && *next_token != '=')) {
				/* Define a label. */
				struct text_label *def_label;
				DREF struct ast *label_ast;
				uint16_t label_flags;
				loc_here(&loc);
				def_label   = lookup_label(token.t_kwd);
				label_flags = AST_FLABEL_NORMAL;
				if unlikely(!def_label)
					goto err;
				if unlikely(yield() < 0)
					goto err; /* Label name */
				if unlikely(skip_lf())
					goto err;
				if unlikely(yield() < 0)
					goto err; /* `:' token. */
handle_post_label:
				if unlikely(skip_lf())
					goto err;
				if unlikely(tok == '}') {
					/* Emit a warning and when the next token is a `}' */
					if unlikely(WARN(W_MISSING_STATEMENT_AFTER_LABEL))
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
					result = ast_parse_statement(allow_nonblock);
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
					elemv = (DREF struct ast **)Dee_Realloc(result->a_multiple.m_astv,
					                                        (result->a_multiple.m_astc + 1) *
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
					elemv = (DREF struct ast **)Dee_Malloc(2 * sizeof(DREF struct ast *));
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
	case KWD_case:
				if unlikely(!(current_basescope->bs_cflags & BASESCOPE_FSWITCH)) {
					if (WARN(W_NOT_INSIDE_A_SWITCH_STATEMENT))
						goto err;
				}
				loc_here(&loc);
				if unlikely(yield() < 0)
					goto err;
				/* Parse the case expression. */
				result = ast_parse_expr(LOOKUP_SYM_NORMAL);
				if unlikely(!result)
					goto err;
				if (skip(':', W_EXPECTED_COLLON_AFTER_CASE))
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
	case KWD_default:
				if unlikely(!(current_basescope->bs_cflags & BASESCOPE_FSWITCH)) {
					if (WARN(W_NOT_INSIDE_A_SWITCH_STATEMENT))
						goto err;
				} else if unlikely(current_basescope->bs_swdefl) {
					/* Warn if another default label had already been defined. */
					if (WARN(W_DEFAULT_LABEL_HAD_ALREADY_BEEN_DEFINED))
						goto err;
				}
				loc_here(&loc);
				if unlikely(yield() < 0)
					goto err;
				if (skip(':', W_EXPECTED_COLLON_AFTER_DEFAULT))
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
		result = ast_parse_comma(allow_nonblock
		                         ? (AST_COMMA_NORMAL | AST_COMMA_ALLOWVARDECLS |
		                            AST_COMMA_ALLOWTYPEDECL | AST_COMMA_PARSESEMI |
		                            AST_COMMA_ALLOWNONBLOCK)
		                         : (AST_COMMA_NORMAL | AST_COMMA_ALLOWVARDECLS |
		                            AST_COMMA_ALLOWTYPEDECL | AST_COMMA_PARSESEMI),
		                         AST_FMULTIPLE_KEEPLAST,
		                         NULL);
		/*if unlikely(!result) goto err;*/
		break;
	}
	/* Clear tags at the end of each statement. */
	if unlikely(ast_tags_clear())
		goto err_r;
done_no_tag_reset:
	return result;
err_flags:
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
	goto err;
err_r_flags:
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
err_r:
	ast_decref(result);
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_STATEMENT_C */
