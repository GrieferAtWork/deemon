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
#ifndef GUARD_DEEMON_COMPILER_LEXER_TRY_C
#define GUARD_DEEMON_COMPILER_LEXER_TRY_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_CollectMemoryc, Dee_Free, Dee_Mallocc, Dee_TryReallocc */
#include <deemon/code.h>            /* Dee_EXCEPTION_HANDLER_F* */
#include <deemon/compiler/ast.h>    /* AST_FMULTIPLE_TUPLE, CATCH_EXPR_FNORMAL, ast, ast_*, catch_expr */
#include <deemon/compiler/lexer.h>  /* ast_parse_*, ast_tags_clear, current_tags, parse_tags_block */
#include <deemon/compiler/symbol.h> /* LOOKUP_SYM_NORMAL, SYMBOL_TYPE_EXCEPT, has_local_symbol, new_local_symbol, scope_pop, scope_push, symbol */
#include <deemon/compiler/tpp.h>
#include <deemon/object.h>          /* DREF */
#include <deemon/type.h>            /* TP_FINTERRUPT */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_catchmask(DeeLexer *self) {
	size_t exprc, expra;
	DREF struct ast **exprv, *result;
	result = ast_parse_unary(self, LOOKUP_SYM_NORMAL);
	if (DeeLexer_GetTok(self) == '|' && result) {
		struct ast_loc multi_loc;
		exprc = 1;
		expra = 2;
		exprv = (DREF struct ast **)Dee_Mallocc(expra, sizeof(DREF struct ast *));
		if unlikely(!exprv)
			goto err_r;
		exprv[0] = result; /* Inherit */

		/* Multiple masks.
		 * >> import Error from deemon;
		 * >> try {
		 * >>     do_the_danger();
		 * >> } catch (Error.RuntimeError |
		 * >>          Error.ValueError |
		 * >>          Error.TypeError
		 * >>          as err) {
		 * >>     print "So this happened:", err;
		 * >> }
		 * Multi-masks are actually just implemented as
		 * Tuple-expressions found in the mask ast, meaning
		 * that the above example could also be written as:
		 * >> import Error from deemon;
		 * >> try {
		 * >>     do_the_danger();
		 * >> } catch ((Error.RuntimeError,
		 * >>           Error.ValueError,
		 * >>           Error.TypeError)
		 * >>           as err) {
		 * >>     print "So this happened:", err;
		 * >> }
		 */
		if (DeeLexer_GetLoc(self, &multi_loc))
			goto err_exprv;
		while (DeeLexer_GetTok(self) == '|') {
			if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
				goto err_exprv;
			if (exprc == expra) {
				/* Must allocate more memory. */
				DREF struct ast **new_exprv;
				size_t new_expra = expra * 2;
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
					goto err_exprv;
				}
				expra = new_expra;
				exprv = new_exprv;
			}
			result = ast_parse_unary(self, LOOKUP_SYM_NORMAL);
			if unlikely(!result)
				goto err_exprv;
			exprv[exprc++] = result; /* Inherit */
		}

		/* Pack together the multi-expression. */
		result = ast_setddi(ast_multiple(AST_FMULTIPLE_TUPLE,
		                                 exprc, exprv),
		                    &multi_loc);
		if unlikely(!result)
			goto err_exprv;
		/* `ast_multiple()` inherited `exprv` and all contained asts upon success. */
	}
	return result;
err_exprv:
	while (exprc--)
		ast_decref(exprv[exprc]);
	Dee_Free(exprv);
	return NULL;
err_r:
	ast_decref(result);
	return NULL;
}


/* Parse a try-statement/expression. */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_try(DeeLexer *self, bool is_statement) {
	DREF struct ast *result, *merge;
	struct ast_loc loc;
	size_t catcha, catchc;
	struct catch_expr *catchv, *handler;
	ASSERT(DeeLexer_GetTok(self) == TPP_KWD_try);
	if (DeeLexer_GetLoc(self, &loc))
		goto err;
	if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
		goto err;
	result = is_statement
	         ? ast_parse_statement(self, false)
	         : ast_parse_expr(self, LOOKUP_SYM_NORMAL);
	if unlikely(!result)
		goto err;
	catcha = 0;
	catchc = 0;
	catchv = NULL;
	for (;;) {
		tpp_token_id mode;
		if unlikely(ast_tags_clear(self))
			goto err_try;
		if unlikely(parse_tags_block(self))
			goto err_try;
		mode = DeeLexer_GetTok(self);
		if (mode != TPP_KWD_finally &&
		    mode != TPP_KWD_catch)
			break;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_try;
		ASSERT(catchc <= catcha);

		/* Most of the time a try-statement only has a single handler,
		 * meaning that the following check will succeed most of the time. */
		handler = catchv;
		if likely(catchc == catcha) {
			size_t new_catcha = unlikely(catcha)
			                    ? catcha + ((catcha + 2) / 3)
			                    : 1;
do_realloc_catchv:
			handler = (struct catch_expr *)Dee_TryReallocc(catchv, new_catcha,
			                                               sizeof(struct catch_expr));
			if unlikely(!handler) {
				if (new_catcha != catchc + 1) {
					new_catcha = catchc + 1;
					goto do_realloc_catchv;
				}
				if (Dee_CollectMemoryc(new_catcha, sizeof(struct catch_expr)))
					goto do_realloc_catchv;
				goto err_try;
			}
			catchv = handler;
			catcha = new_catcha;
		}
		handler += catchc;
		handler->ce_mask  = NULL;
		handler->ce_flags = Dee_EXCEPTION_HANDLER_FNORMAL;
		handler->ce_mode  = CATCH_EXPR_FNORMAL;

		/* Set the interrupt-flag when an @[interrupt] tag was used. */
		if (current_tags.at_class_flags & TP_FINTERRUPT)
			handler->ce_flags |= Dee_EXCEPTION_HANDLER_FINTERPT;
		if (mode == TPP_KWD_finally) {
			handler->ce_flags |= Dee_EXCEPTION_HANDLER_FFINALLY;
			handler->ce_code = is_statement
			                   ? ast_parse_statement(self, false)
			                   : ast_parse_expr(self, LOOKUP_SYM_NORMAL);
			if unlikely(!handler->ce_code)
				goto err_try;
		} else {
			bool has_paren;
			bool is_new_scope = false;
			struct symbol *guard_symbol;
			DeeLexer_NoLf_Push(self);
			if (DeeLexer_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_CATCH)) {
err_try_flags:
				DeeLexer_NoLf_Break(self);
				goto err_try;
			}
			if (DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT) {
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err_try_flags;
				if (DeeLexer_HasTokenKwd(self)) {
					/* Alternative catch-all spelling for backwards
					 * compatibility: `catch (...error)` */
					if unlikely(scope_push() < 0)
						goto err_try_flags;
					is_new_scope = true;
					ASSERT(!has_local_symbol(DeeLexer_GetTokenKwd(self)));
					guard_symbol = new_local_symbol(DeeLexer_GetTokenKwd(self), NULL);
					if unlikely(!guard_symbol)
						goto err_try_flags;
					guard_symbol->s_type = SYMBOL_TYPE_EXCEPT;
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_try_flags;
				}
			} else if (DeeLexer_HasTokenKwd(self)) {
				/* Exception guard name: `try { ... } catch (err...) {}` */
#ifdef CONFIG_EXPERIMENTAL_USE_TPP3
				tpp_token_id next_token;
				next_token = tpp_lexer_peek_raw(&self->dl_lexer,
				                                TPP_LEXER_PEEK_RAW_FLAG_NORMAL,
				                                NULL, NULL, NULL);
				if (TPP_TOK_ISERR(next_token))
					goto err_try_flags;
				if (next_token == TPP_TOK_DOT_DOT_DOT)
#else /* CONFIG_EXPERIMENTAL_USE_TPP3 */
				char const *next_token = peek_next_token(NULL);
				if unlikely(!next_token)
					goto err_try_flags;
				if (*next_token == '.' && /* Check for `...` */
				    (next_token = advance_wraplf(next_token), *next_token == '.') &&
				    (next_token = advance_wraplf(next_token), *next_token == '.'))
#endif /* !CONFIG_EXPERIMENTAL_USE_TPP3 */
				{
					if unlikely(scope_push() < 0)
						goto err_try_flags;
					is_new_scope = true;
					ASSERT(!has_local_symbol(DeeLexer_GetTokenKwd(self)));
					guard_symbol = new_local_symbol(DeeLexer_GetTokenKwd(self), NULL);
					if unlikely(!guard_symbol)
						goto err_try;
					guard_symbol->s_type = SYMBOL_TYPE_EXCEPT;
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_try_flags; /* Yield the exception addressing keyword. */
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_try_flags; /* Yield `...`. */
				} else {
					goto parse_catch_mask;
				}
			} else {
parse_catch_mask:
				/* Explicit catch mask: `try { ... } catch (get_mask())` */
				handler->ce_mask = ast_parse_catchmask(self);
				/* NOTE: For some reason I though it would be a good idea to use
				 *       the arrow token here in the old deemon (like wtf?).
				 *       But since using `as` in its place is literally a 1-on-1
				 *       transition, it doesn't hurt if we continue to allow arrows. */
				if unlikely(DeeLexer_GetTok(self) == TPP_TOK_MINUS_RANGLE ||
				            DeeLexer_GetTok(self) == TPP_KWD_as) {
					if unlikely(DeeLexer_GetTok(self) == TPP_TOK_MINUS_RANGLE) {
						if (DeeLexer_Warnf(self, TPP_W_DEPRECATED_ARROW_IN_CATCH_EXPRESSION))
							goto err_try_flags;
					}
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_try_flags;
					if unlikely(!DeeLexer_HasTokenKwd(self)) {
						if (DeeLexer_Warnf(self, TPP_W_EXPECTED_KEYWORD_AFTER_CATCH_AS))
							goto err_try_flags;
						goto end_catch_handler;
					}
					goto parse_catch_symbol;
				}
				if (DeeLexer_HasTokenKwd(self)) {
					/* Exception guard name: `try { ... } catch (Error err) {}` */
parse_catch_symbol:
					if unlikely(scope_push() < 0)
						goto err_try_flags;
					ASSERT(!is_new_scope);
					is_new_scope = true;
					ASSERT(!has_local_symbol(DeeLexer_GetTokenKwd(self)));
					guard_symbol = new_local_symbol(DeeLexer_GetTokenKwd(self), NULL);
					if unlikely(!guard_symbol)
						goto err_try_flags;
					guard_symbol->s_type = SYMBOL_TYPE_EXCEPT;
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_try_flags;
				}
			}
end_catch_handler:
			DeeLexer_NoLf_Pop(self);
			if (DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_CATCH))
				goto err_try;
			handler->ce_code = is_statement
			                   ? ast_parse_statement(self, false)
			                   : ast_parse_expr(self, LOOKUP_SYM_NORMAL);
			if unlikely(!handler->ce_code)
				goto err_try;
			if (is_new_scope)
				scope_pop();
		}
		++catchc;
	}

	/* Clear unused buffer memory. */
	if (catchc != catcha) {
		handler = (struct catch_expr *)Dee_TryReallocc(catchv, catchc,
		                                               sizeof(struct catch_expr));
		if likely(handler)
			catchv = handler;
	}

	/* Create the new try-AST. */
	merge = ast_setddi(ast_try(result, catchc, catchv), &loc);
	if unlikely(!merge)
		goto err_try;
	ast_decref(result);
	result = merge;

	/* Warn if we didn't parse any handlers. */
	if unlikely(!catchc) {
		if (DeeLexer_Warnf(self, TPP_W_EXPECTED_CATCH_OR_FINALLY_AFTER_TRY))
			goto err_r;
	}
	return result;
err_try:
	while (catchc--) {
		ast_xdecref(catchv[catchc].ce_mask);
		ast_decref(catchv[catchc].ce_code);
	}
	Dee_Free(catchv);
	/*goto err_r;*/
err_r:
	ast_decref(result);
err:
	return NULL;
}


/* With the current token being `try`, parse the construct and
 * try to figure out if it's a statement or an expression. */
INTERN WUNUSED NONNULL((1)) DREF struct ast *DFCALL
ast_parse_try_hybrid(DeeLexer *self, unsigned int *p_was_expression) {
	DREF struct ast *result, *merge;
	struct ast_loc loc;
	size_t catcha, catchc;
	struct catch_expr *catchv, *handler;
	unsigned int was_expression;
	ASSERT(DeeLexer_GetTok(self) == TPP_KWD_try);
	if (DeeLexer_GetLoc(self, &loc))
		goto err;
	if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
		goto err;
	result = ast_parse_hybrid_primary(self, &was_expression);
	if unlikely(!result)
		goto err;
	catcha = 0;
	catchc = 0;
	catchv = NULL;
	for (;;) {
		tok_t mode;
		if unlikely(ast_tags_clear(self))
			goto err_try;
		if unlikely(parse_tags_block(self))
			goto err_try;
		mode = DeeLexer_GetTok(self);
		if (mode != TPP_KWD_finally &&
		    mode != TPP_KWD_catch)
			break;
		if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
			goto err_try;
		ASSERT(catchc <= catcha);
		/* Most of the time a try-statement only has a single handler,
		 * meaning that the following check will succeed most of the time. */
		handler = catchv;
		if likely(catchc == catcha) {
			size_t new_catcha = unlikely(catcha)
			                    ? catcha + ((catcha + 2) / 3)
			                    : 1;
do_realloc_catchv:
			handler = (struct catch_expr *)Dee_TryReallocc(catchv, new_catcha,
			                                               sizeof(struct catch_expr));
			if unlikely(!handler) {
				if (new_catcha != catchc + 1) {
					new_catcha = catchc + 1;
					goto do_realloc_catchv;
				}
				if (Dee_CollectMemoryc(new_catcha, sizeof(struct catch_expr)))
					goto do_realloc_catchv;
				goto err_try;
			}
			catchv = handler;
			catcha = new_catcha;
		}
		handler += catchc;
		handler->ce_mask  = NULL;
		handler->ce_flags = Dee_EXCEPTION_HANDLER_FNORMAL;
		handler->ce_mode  = CATCH_EXPR_FNORMAL;

		/* Set the interrupt-flag when an @[interrupt] tag was used. */
		if (current_tags.at_class_flags & TP_FINTERRUPT)
			handler->ce_flags |= Dee_EXCEPTION_HANDLER_FINTERPT;
		if (mode == TPP_KWD_finally) {
			handler->ce_flags |= Dee_EXCEPTION_HANDLER_FFINALLY;
			handler->ce_code = ast_parse_hybrid_secondary(self, &was_expression);
			if unlikely(!handler->ce_code)
				goto err_try;
		} else {
			bool has_paren;
			bool is_new_scope = false;
			struct symbol *guard_symbol;
			DeeLexer_NoLf_Push(self);
			if (DeeLexer_ParenBegin2(self, &has_paren, W_EXPECTED_LPAREN_AFTER_CATCH)) {
err_try_flags:
				DeeLexer_NoLf_Break(self);
				goto err_try;
			}
			if (DeeLexer_GetTok(self) == TPP_TOK_DOT_DOT_DOT) {
				if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
					goto err_try_flags;
				if (DeeLexer_HasTokenKwd(self)) {
					/* Alternative catch-all spelling for backwards
					 * compatibility: `catch (...error)` */
					if unlikely(scope_push() < 0)
						goto err_try_flags;
					is_new_scope = true;
					ASSERT(!has_local_symbol(DeeLexer_GetTokenKwd(self)));
					guard_symbol = new_local_symbol(DeeLexer_GetTokenKwd(self), NULL);
					if unlikely(!guard_symbol)
						goto err_try_flags;
					guard_symbol->s_type = SYMBOL_TYPE_EXCEPT;
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_try_flags;
				}
			} else if (DeeLexer_HasTokenKwd(self)) {
				/* Exception guard name: `try { ... } catch (err...) {}` */
				char const *next_token = peek_next_token(NULL);
				if unlikely(!next_token)
					goto err_try_flags;
				if (*next_token == '.' && /* Check for `...` */
				    (next_token = advance_wraplf(next_token), *next_token == '.') &&
				    (next_token = advance_wraplf(next_token), *next_token == '.')) {
					if unlikely(scope_push() < 0)
						goto err_try_flags;
					is_new_scope = true;
					ASSERT(!has_local_symbol(DeeLexer_GetTokenKwd(self)));
					guard_symbol = new_local_symbol(DeeLexer_GetTokenKwd(self), NULL);
					if unlikely(!guard_symbol)
						goto err_try;
					guard_symbol->s_type = SYMBOL_TYPE_EXCEPT;
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_try_flags; /* Yield the exception addressing keyword. */
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_try_flags; /* Yield `...`. */
				} else {
					goto parse_catch_mask;
				}
			} else {
parse_catch_mask:
				/* Explicit catch mask: `try { ... } catch (get_mask())` */
				handler->ce_mask = ast_parse_catchmask(self);

				/* NOTE: For some reason I though it would be a good idea to use
				 *       the arrow token here in the old deemon (like wtf?).
				 *       But since using `as` in its place is literally a 1-on-1
				 *       transition, it doesn't hurt if we continue to allow arrows. */
				if unlikely(DeeLexer_GetTok(self) == TPP_TOK_MINUS_RANGLE || DeeLexer_GetTok(self) == TPP_KWD_as) {
					if unlikely(DeeLexer_GetTok(self) == TPP_TOK_MINUS_RANGLE) {
						if (DeeLexer_Warnf(self, TPP_W_DEPRECATED_ARROW_IN_CATCH_EXPRESSION))
							goto err_try_flags;
					}
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_try_flags;
					if unlikely(!DeeLexer_HasTokenKwd(self)) {
						if (DeeLexer_Warnf(self, TPP_W_EXPECTED_KEYWORD_AFTER_CATCH_AS))
							goto err_try_flags;
						goto end_catch_handler;
					}
					goto parse_catch_symbol;
				}
				if (DeeLexer_HasTokenKwd(self)) {
					/* Exception guard name: `try { ... } catch (Error err) {}` */
parse_catch_symbol:
					if unlikely(scope_push() < 0)
						goto err_try_flags;
					ASSERT(!is_new_scope);
					is_new_scope = true;
					ASSERT(!has_local_symbol(DeeLexer_GetTokenKwd(self)));
					guard_symbol = new_local_symbol(DeeLexer_GetTokenKwd(self), NULL);
					if unlikely(!guard_symbol)
						goto err_try_flags;
					guard_symbol->s_type = SYMBOL_TYPE_EXCEPT;
					if (TPP_TOK_ISERR(DeeLexer_Yield(self)))
						goto err_try_flags;
				}
			}
end_catch_handler:
			DeeLexer_NoLf_Pop(self);
			if (DeeLexer_ParenEnd2(self, has_paren, W_EXPECTED_RPAREN_AFTER_CATCH))
				goto err_try;
			handler->ce_code = ast_parse_hybrid_secondary(self, &was_expression);
			if unlikely(!handler->ce_code)
				goto err_try;
			if (is_new_scope)
				scope_pop();
		}
		++catchc;
	}

	/* Clear unused buffer memory. */
	if (catchc != catcha) {
		handler = (struct catch_expr *)Dee_TryReallocc(catchv, catchc,
		                                               sizeof(struct catch_expr));
		if likely(handler)
			catchv = handler;
	}

	/* Create the new try-AST. */
	merge = ast_try(result, catchc, catchv);
	merge = ast_setddi(merge, &loc);
	if unlikely(!merge)
		goto err_try;
	ast_decref(result);
	result = merge;

	/* Warn if we didn't parse any handlers. */
	if unlikely(!catchc) {
		if (DeeLexer_Warnf(self, TPP_W_EXPECTED_CATCH_OR_FINALLY_AFTER_TRY))
			goto err_r;
	}
	if (p_was_expression)
		*p_was_expression = was_expression;
	return result;
err_try:
	while (catchc--) {
		ast_xdecref(catchv[catchc].ce_mask);
		ast_decref(catchv[catchc].ce_code);
	}
	Dee_Free(catchv);
	/*goto err_r;*/
err_r:
	ast_decref(result);
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_TRY_C */
