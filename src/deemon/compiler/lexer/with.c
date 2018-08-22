/* Copyright (c) 2018 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_COMPILER_LEXER_WITH_C
#define GUARD_DEEMON_COMPILER_LEXER_WITH_C 1

#include <deemon/api.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>

DECL_BEGIN


/* The with-statement:
 * >> with (<foo>) {
 * >>     <bar>
 * >> }
 * `with' doesn't have its own AST, but instead is equivalent
 * (and actually encoded as) the following replacement:
 * >> {
 * >>     __stack local __hidden = <foo>;
 * >>     __hidden.operator enter();
 * >>     try {
 * >>         <bar>
 * >>     } finally {
 * >>         __hidden.operator leave();
 * >>     }
 * >> }
 * It's main purpose is to be useful for automatic, scope-based
 * resource management, specifically synchronization primitives
 * such as mutexes, or read/write locks, but it can also be used
 * in other places, such as to automatically close files:
 * >> with (local fp = file.open("foo")) {
 * >>     print fp.read();
 * >>     // `fp.operator leave()' here will invoke `fp.close()'
 * >> }
 */
INTERN DREF struct ast *FCALL
ast_parse_with(bool is_statement, bool allow_nonblock) {
 struct ast_loc loc;
 struct symbol *expression_sym;
 DREF struct ast *result,*other,*merge;
 DREF struct ast **result_v;
 uint32_t old_flags;
 ASSERT(tok == KWD_with);
 loc_here(&loc);
 if unlikely(yield() < 0) goto err;
 if (scope_push()) goto err;
 old_flags = TPPLexer_Current->l_flags;
 TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
 if unlikely(likely(tok == '(') ? (yield() < 0) :
             WARN(W_EXPECTED_LPARENT_AFTER_WITH))
    goto err_scope_flags;
 /* Parse the expression for the with.
  * NOTE: We always allow the user to declare variables in here,
  *       so-as to make it easier to make use of with-statements
  *       where the with-expression is re-used inside the block. */
 result = ast_parse_comma(AST_COMMA_NORMAL|AST_COMMA_ALLOWVARDECLS,
                          AST_FMULTIPLE_TUPLE,
                          NULL);
 if unlikely(!result) goto err_scope_flags;
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
 if unlikely(likely(tok == ')') ? (yield() < 0) :
             WARN(W_EXPECTED_RPARENT_AFTER_WITH))
    goto err_scope_r;
 /* Create the symbol that's going to contain the with-expression. */
 expression_sym = new_unnamed_symbol();
 if unlikely(!expression_sym) goto err_scope_r;
 /* Use s stack variable. */
 expression_sym->s_type = SYMBOL_TYPE_STACK;
 /* Generate the store expression. */
 other = ast_setddi(ast_sym(expression_sym),&loc);
 if unlikely(!other) goto err_scope_r;
 merge = ast_setddi(ast_action2(AST_FACTION_STORE,other,result),&loc);
 ast_decref(other);
 ast_decref(result);
 if unlikely(!merge) goto err_scope;
 result = merge;
 /* At this point, we've written the expression into a
  * symbol, which we can access normally from now on. */
 /* Create a vector that's going to be used for the AST_MULTIPLE:
  * [0] -- __hidden_symbol = with_expression;
  * [1] -- __hidden_symbol.operator enter();
  * [2] -- try ... finally { __hidden_symbol.operator leave(); } */
 result_v = (DREF struct ast **)Dee_Malloc(3*sizeof(DREF struct ast *));
 if unlikely(!result_v) goto err_scope_r;
 result_v[0] = result; /* Inherit */
 result = ast_setddi(ast_sym(expression_sym),&loc);
 if unlikely(!result) goto err_result_v_0;
 merge = ast_operator1(OPERATOR_ENTER,AST_OPERATOR_FNORMAL,result);
 ast_decref(result);
 if unlikely(!merge) goto err_result_v_0;
 result_v[1] = merge; /* Inherit. */

 /* Finally, parse the content of the wrapped try-statement. */
 result = is_statement ? ast_parse_statement(allow_nonblock)
                       : ast_parse_expr(LOOKUP_SYM_NORMAL);
 if unlikely(!result) goto err_result_v_1;

 /* Create the leave-expression for the finally block. */
 merge = ast_setddi(ast_sym(expression_sym),&loc);
 if unlikely(!merge) goto err_result_v_1_r;
 /* Invoke the leave operator on the symbol. */
 other = ast_operator1(OPERATOR_LEAVE,AST_OPERATOR_FNORMAL,merge);
 ast_decref(merge);
 if unlikely(!other) goto err_result_v_1_r;

 /* Wrap the with-block in a try-finally AST with the leave-statement. */
 merge = ast_setddi(ast_tryfinally(result,
                                   other),
                   &loc);
 ast_decref(other);
 ast_decref(result);
 if unlikely(!merge) goto err_result_v_1;
 result_v[2] = merge; /* Inherit */

 /* At this point, we've created all the necessary expression and it
  * is time to pack everything together in a multiple-ast.
  * HINT: as it happens, the try-statement is the last expression in
  *       this ast, which is exactly what we want the with-statement
  *       to evaluate to when used in an expression (aka. whatever
  *       the user writes as the last expression of the try-block). */
 result = ast_setddi(ast_multiple(AST_FMULTIPLE_KEEPLAST,3,result_v),&loc);
 if unlikely(!result) goto err_result_v_2;
 scope_pop();
 return result;
err_result_v_2:
 ast_decref(result_v[2]);
err_result_v_1:
 ast_decref(result_v[1]);
err_result_v_0:
 ast_decref(result_v[0]);
 Dee_Free(result_v);
err_scope:
 scope_pop();
err:
 return NULL;
err_scope_flags:
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
 goto err_scope;
err_result_v_1_r:
 ast_decref(result);
 goto err_result_v_1;
err_scope_r:
 ast_decref(result);
 goto err_scope;
}


INTERN DREF struct ast *FCALL
ast_parse_with_hybrid(unsigned int *pwas_expression) {
 struct ast_loc loc;
 struct symbol *expression_sym;
 DREF struct ast *result,*other,*merge;
 DREF struct ast **result_v;
 uint32_t old_flags;
 ASSERT(tok == KWD_with);
 loc_here(&loc);
 if unlikely(yield() < 0) goto err;
 if (scope_push()) goto err;
 old_flags = TPPLexer_Current->l_flags;
 TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
 if unlikely(likely(tok == '(') ? (yield() < 0) :
             WARN(W_EXPECTED_LPARENT_AFTER_WITH))
    goto err_scope_flags;
 /* Parse the expression for the with.
  * NOTE: We always allow the user to declare variables in here,
  *       so-as to make it easier to make use of with-statements
  *       where the with-expression is re-used inside the block. */
 result = ast_parse_comma(AST_COMMA_NORMAL|AST_COMMA_ALLOWVARDECLS,
                          AST_FMULTIPLE_TUPLE,
                          NULL);
 if unlikely(!result) goto err_scope_flags;
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
 if unlikely(likely(tok == ')') ? (yield() < 0) :
             WARN(W_EXPECTED_RPARENT_AFTER_WITH))
    goto err_scope_r;
 /* Create the symbol that's going to contain the with-expression. */
 expression_sym = new_unnamed_symbol();
 if unlikely(!expression_sym) goto err_scope_r;
 /* Use s stack variable. */
 expression_sym->s_type = SYMBOL_TYPE_STACK;
 /* Generate the store expression. */
 other = ast_setddi(ast_sym(expression_sym),&loc);
 if unlikely(!other) goto err_scope_r;
 merge = ast_setddi(ast_action2(AST_FACTION_STORE,other,result),&loc);
 ast_decref(other);
 ast_decref(result);
 if unlikely(!merge) goto err_scope;
 result = merge;
 /* At this point, we've written the expression into a
  * symbol, which we can access normally from now on. */
 /* Create a vector that's going to be used for the AST_MULTIPLE:
  * [0] -- __hidden_symbol = with_expression;
  * [1] -- __hidden_symbol.operator enter();
  * [2] -- try ... finally { __hidden_symbol.operator leave(); } */
 result_v = (DREF struct ast **)Dee_Malloc(3*sizeof(DREF struct ast *));
 if unlikely(!result_v) goto err_scope_r;
 result_v[0] = result; /* Inherit */
 result = ast_setddi(ast_sym(expression_sym),&loc);
 if unlikely(!result) goto err_result_v_0;
 merge = ast_operator1(OPERATOR_ENTER,AST_OPERATOR_FNORMAL,result);
 ast_decref(result);
 if unlikely(!merge) goto err_result_v_0;
 result_v[1] = merge; /* Inherit. */

 /* Finally, parse the content of the wrapped try-statement. */
 result = ast_parse_statement_or_expression(AST_COMMA_MODE_HYBRID_SINGLE,
                                            pwas_expression);
 if unlikely(!result) goto err_result_v_1;

 /* Create the leave-expression for the finally block. */
 merge = ast_setddi(ast_sym(expression_sym),&loc);
 if unlikely(!merge) goto err_result_v_1_r;
 /* Invoke the leave operator on the symbol. */
 other = ast_operator1(OPERATOR_LEAVE,AST_OPERATOR_FNORMAL,merge);
 ast_decref(merge);
 if unlikely(!other) goto err_result_v_1_r;

 /* Wrap the with-block in a try-finally AST with the leave-statement. */
 merge = ast_setddi(ast_tryfinally(result,
                                   other),
                   &loc);
 ast_decref(other);
 ast_decref(result);
 if unlikely(!merge) goto err_result_v_1;
 result_v[2] = merge; /* Inherit */

 /* At this point, we've created all the necessary expression and it
  * is time to pack everything together in a multiple-ast.
  * HINT: as it happens, the try-statement is the last expression in
  *       this ast, which is exactly what we want the with-statement
  *       to evaluate to when used in an expression (aka. whatever
  *       the user writes as the last expression of the try-block). */
 result = ast_setddi(ast_multiple(AST_FMULTIPLE_KEEPLAST,3,result_v),&loc);
 if unlikely(!result) goto err_result_v_2;
 scope_pop();
 return result;
err_result_v_2:
 ast_decref(result_v[2]);
err_result_v_1:
 ast_decref(result_v[1]);
err_result_v_0:
 ast_decref(result_v[0]);
 Dee_Free(result_v);
err_scope:
 scope_pop();
err:
 return NULL;
err_scope_flags:
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
 goto err_scope;
err_result_v_1_r:
 ast_decref(result);
 goto err_result_v_1;
err_scope_r:
 ast_decref(result);
 goto err_scope;
}



DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_WITH_C */
