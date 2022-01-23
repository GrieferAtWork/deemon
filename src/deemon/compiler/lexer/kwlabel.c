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
#ifndef GUARD_DEEMON_COMPILER_LEXER_KWLABEL_C
#define GUARD_DEEMON_COMPILER_LEXER_KWLABEL_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/tpp.h>
#include <deemon/tuple.h>

DECL_BEGIN


INTERN WUNUSED NONNULL((2)) DREF struct ast *DCALL
ast_parse_argument_list(uint16_t mode,
                        DREF struct ast **__restrict pkeyword_labels) {
	DREF struct ast *result;
	DREF struct ast *kwdlist_ast;
	DeeObject *kwdlist;
	*pkeyword_labels = NULL;
	ASSERT(mode & AST_COMMA_FORCEMULTIPLE);
	result = ast_parse_comma(mode |
	                         AST_COMMA_ALLOWKWDLIST,
	                         AST_FMULTIPLE_TUPLE,
	                         NULL);
	if unlikely(!result)
		goto err;
	if (tok == TOK_POW) {
		/* XXX: I really don't like using `**' for this.
		 *      I realize that I _have_ to provide some way
		 *      of passing arbitrary mappings through keywords,
		 *      however this just feel too python-sque to me...
		 */
		if unlikely(yield() < 0)
			goto err_r;
		/* Parse the keyword invocation AST. */
		*pkeyword_labels = ast_parse_expr(LOOKUP_SYM_NORMAL);
		if unlikely(!*pkeyword_labels)
			goto err_r;
	} else if (TPP_ISKEYWORD(tok)) {
		char *next = peek_next_token(NULL);
		if unlikely(!next)
			goto err_r;
		if (*next == ':') {
			size_t multiple_a;
			if (result->a_type == AST_CONSTEXPR) {
				ASSERT(result->a_constexpr == Dee_EmptyTuple);
				Dee_Decref(Dee_EmptyTuple);
				result->a_type            = AST_MULTIPLE;
				result->a_flag            = AST_FMULTIPLE_TUPLE;
				result->a_multiple.m_astc = 0;
				result->a_multiple.m_astv = NULL;
			}
			kwdlist = DeeKwds_NewWithHint(1);
			if unlikely(!kwdlist)
				goto err_r;
			kwdlist_ast = ast_sethere(ast_constexpr(kwdlist));
			Dee_Decref_unlikely(kwdlist);
			if unlikely(!kwdlist_ast)
				goto err_r;
			*pkeyword_labels = kwdlist_ast; /* Inherit reference (on success) */
			multiple_a       = result->a_multiple.m_astc;
			/* Parse a keyword label list! */
			for (;;) {
				DREF struct ast *argument_value;
				/* Append the argument label. */
				if (DeeKwds_Append(&kwdlist_ast->a_constexpr,
				                   token.t_kwd->k_name,
				                   token.t_kwd->k_size,
				                   Dee_HashStr(token.t_kwd->k_name)))
					goto err_r_kwdlist;
				if unlikely(yield() < 0)
					goto err_r_kwdlist;
				if (skip(':', W_EXPECTED_COLLON_AFTER_KEYWORD_LABEL))
					goto err_r_kwdlist;
				/* Make sure that we have allocated sufficient memory for the keyword list. */
				ASSERT(result->a_multiple.m_astc <= multiple_a);
				if (result->a_multiple.m_astc >= multiple_a) {
					DREF struct ast **new_astv;
					size_t new_alloc = multiple_a * 2;
					if unlikely(!new_alloc)
						new_alloc = 2;
					new_astv = (DREF struct ast **)Dee_TryRealloc(result->a_multiple.m_astv,
					                                              new_alloc * sizeof(DREF struct ast *));
					if unlikely(!new_astv) {
						new_alloc = result->a_multiple.m_astc + 1;
						new_astv = (DREF struct ast **)Dee_Realloc(result->a_multiple.m_astv,
						                                           new_alloc * sizeof(DREF struct ast *));
						if unlikely(!new_astv)
							goto err_r_kwdlist;
					}
					result->a_multiple.m_astv = new_astv;
					multiple_a                = new_alloc;
				}
				/* Parse the expression that is bound to the keyword. */
				argument_value = ast_parse_expr(LOOKUP_SYM_NORMAL);
				if unlikely(!argument_value)
					goto err_r_kwdlist;
				result->a_multiple.m_astv[result->a_multiple.m_astc++] = argument_value; /* Inherit reference. */
				if (tok != ',')
					break;
				if (mode & AST_COMMA_STRICTCOMMA) {
					char *next_token = peek_next_token(NULL);
					if unlikely(!next_token)
						goto err_r_kwdlist;
					if (!DeeUni_IsSymCont(*next_token)) /* Can't be a label. */
						break;
				}
				if unlikely(yield() < 0)
					goto err_r_kwdlist;
				if (!TPP_ISKEYWORD(tok))
					break; /* End of argument list. */
			}
			ASSERT(result->a_multiple.m_astc <= multiple_a);
			/* Flush unused memory from keyword-bound arguments. */
			if (multiple_a > result->a_multiple.m_astc) {
				DREF struct ast **new_astv;
				new_astv = (DREF struct ast **)Dee_TryRealloc(result->a_multiple.m_astv,
				                                              result->a_multiple.m_astc *
				                                              sizeof(DREF struct ast *));
				if likely(new_astv)
					result->a_multiple.m_astv = new_astv;
			}
			/* The constant-branch for the keyword-list was already
			 * constructed above, so we're already finished here! */
		}
	}
	return result;
err_r_kwdlist:
	ast_decref(kwdlist_ast);
err_r:
	ast_decref(result);
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_KWLABEL_C */
