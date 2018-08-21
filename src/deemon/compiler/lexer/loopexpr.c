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
#ifndef GUARD_DEEMON_COMPILER_LEXER_LOOPEXPR_C
#define GUARD_DEEMON_COMPILER_LEXER_LOOPEXPR_C 1

#include <deemon/api.h>
#include <deemon/tuple.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>


/* Loop statements in expressions are compiled as yield-function lambda expressions:
 * >> local text = get_text();
 * >> print (for (local x: text.splitlines(false)) x.strip())...;
 * Actually parsed as:
 * >> local text = get_text();
 * >> print []{
 * >>     for (local x: text.splitlines(false))
 * >>         yield x.strip();
 * >> }()...;
 * The functions in this file implement this abstraction.
 */
DECL_BEGIN

PRIVATE DREF struct ast *FCALL
wrap_yield(DREF struct ast *ast, struct ast_loc *__restrict loc) {
 DREF struct ast *result;
 result = ast_setddi(ast_yield(ast_setddi(ast,loc)),loc);
 if likely(result) ast_decref(ast);
 return result;
}

PRIVATE DREF struct ast *FCALL
parse_generator_loop(struct ast_loc *__restrict ddi_loc) {
 struct ast_loc loc; uint32_t old_flags;
 DREF struct ast *result,*other,*merge;
 /* Special handling for recursive loops and conditional statements:
  * >> print (for (local x: items) if (x > 10) x)...; // Print all items > 10
  * >> print (for (local x: items) for (local y: x) y)...; // Print all items or each item of `items'
  */
 switch (tok) {

 {
  DREF struct ast *ff_branch;
 case KWD_if:
  loc_here(&loc);
  if unlikely(yield() < 0) goto err;
  old_flags = TPPLexer_Current->l_flags;
  TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == '(') ? (yield() < 0) :
              WARN(W_EXPECTED_LPAREN_AFTER_IF))
     goto err_flags;
  /* NOTE: Allow variable declarations within the condition. */
  result = ast_parse_comma(LOOKUP_SYM_NORMAL|
                           LOOKUP_SYM_ALLOWDECL,
                           AST_FMULTIPLE_KEEPLAST,
                           NULL);
  if unlikely(!result) goto err_flags;
  TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == ')') ? (yield() < 0) :
              WARN(W_EXPECTED_RPAREN_AFTER_IF))
     goto err_r;

  /* Parse the conditional expression. */
  other = parse_generator_loop(ddi_loc);
  if unlikely(!other) goto err_r;

  /* Parse an optional false-branch. */
  ff_branch = NULL;
  if (tok == KWD_else) {
   if unlikely(yield() < 0) goto err_r_other;
   ff_branch = parse_generator_loop(ddi_loc);
   if unlikely(!ff_branch) goto err_r_other;
  }

  /* Create the conditional branch. */
  merge = ast_setddi(ast_conditional(AST_FCOND_EVAL,
                                     result,other,
                                     ff_branch),
                    &loc);
  ast_xdecref(ff_branch);
  ast_decref(other);
  ast_decref(result);
  result = merge;
 } break;

 case KWD_do:
  loc_here(&loc);
  if unlikely(yield() < 0) goto err;
  result = parse_generator_loop(&loc);
  if unlikely(!result) goto err;
  if unlikely(likely(tok == KWD_while) ? (yield() < 0) :
              WARN(W_EXPECTED_WHILE_AFTER_DO))
     goto err_r;
  old_flags = TPPLexer_Current->l_flags;
  TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == '(') ? (yield() < 0) :
              WARN(W_EXPECTED_LPAREN_AFTER_WHILE))
     goto err_r_flags;
  other = ast_parse_expression(LOOKUP_SYM_NORMAL);
  if unlikely(!other)
     goto err_r_flags;
  TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == ')') ? (yield() < 0) :
              WARN(W_EXPECTED_RPAREN_AFTER_WHILE))
     goto err_r_other;
  /* Pack together the loop expression. */
  merge = ast_setddi(ast_loop(AST_FLOOP_POSTCOND,other,NULL,result),&loc);
  if unlikely(!merge) goto err_r_other;
  ast_decref(other);
  ast_decref(result);
  result = merge;
  break;

 case KWD_while:
  loc_here(&loc);
  if unlikely(yield() < 0) goto err;
  /* Parse the while-condition. */
  old_flags = TPPLexer_Current->l_flags;
  TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == '(') ? (yield() < 0) :
              WARN(W_EXPECTED_LPAREN_AFTER_WHILE))
     goto err_flags;
  /* NOTE: Allow variable declarations within the condition. */
  result = ast_parse_comma(LOOKUP_SYM_NORMAL|
                           LOOKUP_SYM_ALLOWDECL,
                           AST_FMULTIPLE_KEEPLAST,
                           NULL);
  if unlikely(!result)
     goto err_flags;
  TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == ')') ? (yield() < 0) :
              WARN(W_EXPECTED_RPAREN_AFTER_WHILE))
     goto err_r;
  /* Parse the generator loop. */
  other = parse_generator_loop(&loc);
  if unlikely(!other) goto err_r;
  merge = ast_loop(AST_FLOOP_NORMAL,result,NULL,other);
  if unlikely(!merge) goto err_r_other;
  ast_decref(result);
  ast_decref(other);
  result = merge;
  break;

 {
  DREF struct ast *init;
  DREF struct ast *elem_or_cond;
  DREF struct ast *iter_or_next;
  int32_t type;
 case KWD_for:
  loc_here(&loc);
  if unlikely(yield() < 0) goto err;
  old_flags = TPPLexer_Current->l_flags;
  TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == '(') ? (yield() < 0) :
              WARN(W_EXPECTED_LPAREN_AFTER_FOR))
     goto err_flags;
  /* Parse the for-header. */
  type = ast_parse_for_head(&init,&elem_or_cond,&iter_or_next);
  if unlikely(type < 0)
     goto err_flags;
  TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
  if (type&AST_FLOOP_FOREACH && iter_or_next) {
   /* Wrap the iterator of a foreach-loop with an __iterself__ operator. */
   merge = ast_setddi(ast_operator1(OPERATOR_ITERSELF,
                                    AST_OPERATOR_FNORMAL,
                                    iter_or_next),
                     &loc);
   if unlikely(!merge) goto err_for_loop;
   ast_decref(iter_or_next);
   iter_or_next = merge;
  }
  if unlikely(likely(tok == ')') ? (yield() < 0) :
              WARN(W_EXPECTED_RPAREN_AFTER_FOR))
     goto err_for_loop;
  /* Parse the loop expression. */
  result = parse_generator_loop(&loc);
  if unlikely(!result) goto err_for_loop;
  /* Pack together the loop ast. */
  merge = ast_loop((uint16_t)type,elem_or_cond,iter_or_next,result);
  ast_decref(result);
  if unlikely(!merge) goto err_for_loop;
  ast_decref(iter_or_next);
  ast_decref(elem_or_cond);
  result = merge;
  /* Check if a loop initializer was parsed.
   * If one was, then simply wrap everything in a multi-branch AST. */
  if (init) {
   DREF struct ast **exprv = (DREF struct ast **)Dee_Malloc(2*sizeof(DREF struct ast *));
   if unlikely(!exprv) { err_loop_init: ast_decref(init); goto err_r; }
   /* A loop initializer was given. - Pack it into the resulting AST. */
   exprv[0] = init;   /* Inherit reference. */
   exprv[1] = result; /* Inherit reference. */
   merge    = ast_multiple(AST_FMULTIPLE_KEEPLAST,2,exprv);
   if unlikely(!merge) { Dee_Free(exprv); goto err_loop_init; }
   result = ast_setddi(merge,&loc);
  }
  break;
err_for_loop:
  ast_xdecref(iter_or_next);
  ast_xdecref(elem_or_cond);
  ast_xdecref(init);
  goto err;
 } break;

 {
  DREF struct ast *foreach_elem;
  DREF struct ast *foreach_iter;
  DREF struct ast *foreach_loop;
 case KWD_foreach:
  loc_here(&loc);
  if unlikely(yield() < 0) goto err;
  old_flags = TPPLexer_Current->l_flags;
  TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == '(') ? (yield() < 0) :
              WARN(W_EXPECTED_LPAREN_AFTER_FOR))
     goto err_flags;
  foreach_elem = ast_parse_comma(AST_COMMA_ALLOWVARDECLS,
                                 AST_FMULTIPLE_TUPLE,
                                 NULL);
  if unlikely(!foreach_elem) goto err_flags;
  if unlikely(likely(tok == ':') ? (yield() < 0) :
              WARN(W_EXPECTED_COLLON_AFTER_FOREACH))
     goto err_foreach_elem_flags;
  foreach_iter = ast_parse_expression(LOOKUP_SYM_NORMAL);
  if unlikely(!foreach_iter)
     goto err_foreach_elem_flags;
  TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == ')') ? (yield() < 0) :
              WARN(W_EXPECTED_RPAREN_AFTER_FOR))
     goto err_foreach_iter;
  /* Parse the generator loop expression. */
  foreach_loop = parse_generator_loop(&loc);
  if unlikely(!foreach_loop) goto err_foreach_iter;
  result = ast_setddi(ast_loop(AST_FLOOP_FOREACH,
                               foreach_elem,
                               foreach_iter,
                               foreach_loop),
                     &loc);
  ast_decref(foreach_loop);
  ast_decref(foreach_iter);
  ast_decref(foreach_elem);
  break;
err_foreach_iter:
  ast_decref(foreach_iter);
err_foreach_elem:
  ast_decref(foreach_elem);
  goto err;
err_foreach_elem_flags:
  TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
  goto err_foreach_elem;
 } break;

 default:
  /* Fallback: parse a brace expression and wrap it inside a yield-statement. */
  result = wrap_yield(ast_parse_expression(LOOKUP_SYM_NORMAL),ddi_loc);
  break;
 }
 return result;
err_flags:
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
 goto err;
err_r_flags:
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
 goto err_r;
err_r_other:
 ast_decref(other);
err_r:
 ast_decref(result);
err:
 return NULL;
}

INTERN DREF struct ast *FCALL ast_parse_loopexpr(void) {
 struct ast_loc loc; tok_t mode = tok;
 DREF struct ast *result,*other,*merge;
 ASSERT(mode == KWD_do || mode == KWD_while ||
        mode == KWD_for || mode == KWD_foreach);
 loc_here(&loc);
 if (basescope_push()) goto err;
 /* Generate expressions always create yield-functions. */
 current_basescope->bs_flags |= CODE_FYIELDING;
 /* Parse the generator loop. */
 result = parse_generator_loop(&loc);
 if unlikely(!result) goto err_scope;
 /* Wrap the generator loop in a function ast. */
 merge = ast_setddi(ast_function(result,current_basescope),&loc);
 ast_decref(result);
 basescope_pop();
 if unlikely(!merge) goto err;
 result = merge;
 /* Hack: The function AST itself must be located in the caller's scope. */
 ASSERT(current_scope);
 ASSERT(result->a_scope);
 ASSERT(result->a_scope != current_scope);
 Dee_Incref(current_scope);
 Dee_Decref(result->a_scope);
 result->a_scope = current_scope;
 /* With the lambda function now created, we must still wrap it in a call-expression. */
 other = ast_setddi(ast_constexpr(Dee_EmptyTuple),&loc);
 if unlikely(!other) goto err_r;
 merge = ast_setddi(ast_operator2(OPERATOR_CALL,AST_OPERATOR_FNORMAL,result,other),&loc);
 ast_decref(other);
 ast_decref(result);
 /* if unlikely(!merge) goto err; */
 return merge;
err_scope:
 basescope_pop();
 goto err;
err_r:
 ast_decref(result);
err:
 return NULL;
}


INTERN DREF struct ast *FCALL
ast_parse_loopexpr_hybrid(unsigned int *pwas_expression) {
 /* TODO */
 *pwas_expression = AST_PARSE_WASEXPR_NO;
 return ast_parse_statement(false);
}



DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_LOOPEXPR_C */
