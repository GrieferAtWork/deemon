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
#ifndef GUARD_DEEMON_COMPILER_LEXER_TRY_C
#define GUARD_DEEMON_COMPILER_LEXER_TRY_C 1

#include <deemon/api.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>

DECL_BEGIN

INTERN DREF DeeAstObject *DCALL
ast_parse_catchmask(void) {
 size_t exprc,expra;
 DREF DeeAstObject **exprv,*result;
 result = ast_parse_unary(LOOKUP_SYM_NORMAL);
 if (tok == '|' && result) {
  struct ast_loc multi_loc;
  exprc = 1,expra = 2;
  exprv = (DREF DeeAstObject **)Dee_Malloc(2*sizeof(DREF DeeAstObject *));
  if unlikely(!exprv) goto err_r;
  exprv[0] = result; /* Inherit */
  /* Multiple masks.
   * >> import Error from deemon;
   * >> try {
   * >>     do_the_danger();
   * >> } catch (Error.RuntimeError |
   * >>          Error.ValueError |
   * >>          Error.TypeError
   * >>          as err) {
   * >>     print "So this happened:",err;
   * >> }
   * Multi-masks are actually just implemented as
   * tuple-expressions found in the mask ast, meaning
   * that the above example could also be written as:
   * >> import Error from deemon;
   * >> try {
   * >>     do_the_danger();
   * >> } catch ((Error.RuntimeError,
   * >>           Error.ValueError,
   * >>           Error.TypeError)
   * >>           as err) {
   * >>     print "So this happened:",err;
   * >> }
   */
  loc_here(&multi_loc);
  while (tok == '|') {
   if unlikely(yield() < 0) goto err_exprv;
   if (exprc == expra) {
    /* Must allocate more memory. */
    DREF DeeAstObject **new_exprv;
    size_t new_expra = expra*2;
do_realloc:
    new_exprv = (DREF DeeAstObject **)Dee_TryRealloc(exprv,new_expra*
                                                     sizeof(DREF DeeAstObject *));
    if unlikely(!new_exprv) {
     if (new_expra != exprc+1) { new_expra = exprc+1; goto do_realloc; }
     if (Dee_CollectMemory(new_expra*sizeof(DREF DeeAstObject *))) goto do_realloc;
     goto err_exprv;
    }
    expra = new_expra;
    exprv = new_exprv;
   }
   result = ast_parse_unary(LOOKUP_SYM_NORMAL);
   if unlikely(!result) goto err_exprv;
   exprv[exprc++] = result; /* Inherit */
  }
  /* Pack together the multi-expression. */
  result = ast_setddi(ast_multiple(AST_FMULTIPLE_TUPLE,
                                   exprc,exprv),
                     &multi_loc);
  if unlikely(!result) goto err_exprv;
  /* `ast_multiple()' inherited `exprv' and all contained asts upon success. */
 }
 return result;
err_exprv:
 while (exprc--) Dee_Decref(exprv[exprc]);
 Dee_Free(exprv);
 return NULL;
err_r:
 Dee_Decref(result);
 return NULL;
}


INTERN DREF DeeAstObject *DCALL
ast_parse_try(bool is_statement) {
 DREF DeeAstObject *result,*merge;
 struct ast_loc loc; size_t catcha,catchc;
 struct catch_expr *catchv,*handler;
 uint32_t old_flags;
 ASSERT(tok == KWD_try);
 loc_here(&loc);
 if unlikely(yield() < 0) goto err;
 result = is_statement
        ? ast_parse_statement(false)
        : ast_parse_expression(LOOKUP_SYM_NORMAL);
 if unlikely(!result) goto err;
 catcha = catchc = 0,catchv = NULL;
 for (;;) {
  tok_t mode = tok;
  clear_current_tags();
  if unlikely(parse_tags_block()) goto err_try;
  if (tok != KWD_finally && tok != KWD_catch) break;
  if unlikely(yield() < 0) goto err_try;
  ASSERT(catchc <= catcha);
  /* Most of the time a try-statement only has a single handler,
   * meaning that the following check will succeed most of the time. */
  handler = catchv;
  if likely(catchc == catcha) {
   size_t new_catcha = unlikely(catcha) ? catcha+((catcha+2)/3) : 1;
do_realloc_catchv:
   handler = (struct catch_expr *)Dee_TryRealloc(catchv,new_catcha*
                                                 sizeof(struct catch_expr));
   if unlikely(!handler) {
    if (new_catcha != catchc+1) { new_catcha = catchc+1; goto do_realloc_catchv; }
    if (Dee_CollectMemory(new_catcha*sizeof(struct catch_expr))) goto do_realloc_catchv;
    goto err_try;
   }
   catchv = handler;
   catcha = new_catcha;
  }
  handler          += catchc;
  handler->ce_mask  = NULL;
  handler->ce_flags = EXCEPTION_HANDLER_FNORMAL;
  handler->ce_mode  = CATCH_EXPR_FNORMAL;
  /* Set the interrupt-flag when an @interrupt tag was used. */
  if (current_tags.at_class_flags&TP_FINTERRUPT)
      handler->ce_flags |= EXCEPTION_HANDLER_FINTERPT;
  if (mode == KWD_finally) {
   handler->ce_flags |= EXCEPTION_HANDLER_FFINALLY;
   handler->ce_code = is_statement
                    ? ast_parse_statement(false)
                    : ast_parse_expression(LOOKUP_SYM_NORMAL);
   if unlikely(!handler->ce_code) goto err_try;
  } else {
   bool is_new_scope = false;
   struct symbol *guard_symbol;
   old_flags = TPPLexer_Current->l_flags;
   TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
   if unlikely(likely(tok == '(') ? (yield() < 0) :
               WARN(W_EXPECTED_LPAREN_AFTER_CATCH))
      goto err_try_flags;
   if (tok == TOK_DOTS) {
    if unlikely(yield() < 0) goto err_try_flags;
    if (TPP_ISKEYWORD(tok)) {
     /* Alternative catch-all spelling for backwards
      * compatibility: `catch (...error)' */
     if unlikely(scope_push() < 0) goto err_try_flags;
     is_new_scope = true;
     ASSERT(!has_local_symbol(token.t_kwd));
     guard_symbol = new_local_symbol(token.t_kwd);
     if unlikely(!guard_symbol) goto err_try_flags;
     guard_symbol->sym_class = SYM_CLASS_EXCEPT;
     if unlikely(yield() < 0) goto err_try_flags;
    }
   } else if (TPP_ISKEYWORD(tok)) {
    /* Exception guard name: `try { ... } catch (err...) {}' */
    char *next_token = peek_next_token(NULL);
    if unlikely(!next_token) goto err_try_flags;
    if (*next_token == '.' && /* Check for `...' */
        (next_token = advance_wraplf(next_token),*next_token == '.') &&
        (next_token = advance_wraplf(next_token),*next_token == '.')) {
     if unlikely(scope_push() < 0) goto err_try_flags;
     is_new_scope = true;
     ASSERT(!has_local_symbol(token.t_kwd));
     guard_symbol = new_local_symbol(token.t_kwd);
     if unlikely(!guard_symbol) goto err_try;
     guard_symbol->sym_class = SYM_CLASS_EXCEPT;
     if unlikely(yield() < 0) goto err_try_flags; /* Yield the exception addressing keyword. */
     if unlikely(yield() < 0) goto err_try_flags; /* Yield `...'. */
    } else {
     goto parse_catch_mask;
    }
   } else {
parse_catch_mask:
    /* Explicit catch mask: `try { ... } catch (get_mask())' */
    handler->ce_mask = ast_parse_catchmask();
    /* NOTE: For some reason I though it would be a good idea to use
     *       the arrow token here in the old deemon (like wtf?).
     *       But since using `as' in its place is literally a 1-on-1
     *       transition, it doesn't hurt if we continue to allow arrows. */
    if unlikely(tok == TOK_ARROW || tok == KWD_as) {
     if unlikely(tok == TOK_ARROW &&
                 WARN(W_DEPRECATED_ARROW_IN_CATCH_EXPRESSION))
        goto err_try_flags;
     if unlikely(yield() < 0) goto err_try_flags;
     if unlikely(!TPP_ISKEYWORD(tok)) {
      if (WARN(W_EXPECTED_KEYWORD_AFTER_CATCH_AS))
          goto err_try_flags;
      goto end_catch_handler;
     }
     goto parse_catch_symbol;
    }
    if (TPP_ISKEYWORD(tok)) {
     /* Exception guard name: `try { ... } catch (Error err) {}' */
parse_catch_symbol:
     if unlikely(scope_push() < 0) goto err_try_flags;
     ASSERT(!is_new_scope);
     is_new_scope = true;
     ASSERT(!has_local_symbol(token.t_kwd));
     guard_symbol = new_local_symbol(token.t_kwd);
     if unlikely(!guard_symbol) goto err_try_flags;
     guard_symbol->sym_class = SYM_CLASS_EXCEPT;
     if unlikely(yield() < 0) goto err_try_flags;
    }
   }
end_catch_handler:
   TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
   if unlikely(likely(tok == ')') ? (yield() < 0) :
               WARN(W_EXPECTED_RPAREN_AFTER_CATCH))
      goto err_try;
   handler->ce_code = is_statement
                    ? ast_parse_statement(false)
                    : ast_parse_expression(LOOKUP_SYM_NORMAL);
   if unlikely(!handler->ce_code) goto err_try;
   if (is_new_scope) scope_pop();
  }
  ++catchc;
 }
 /* Clear unused buffer memory. */
 if (catchc != catcha) {
  handler = (struct catch_expr *)Dee_TryRealloc(catchv,catchc*
                                                sizeof(struct catch_expr));
  if likely(handler) catchv = handler;
 }
 /* Create the new try-AST. */
 merge = ast_setddi(ast_try(result,catchc,catchv),&loc);
 if unlikely(!merge) goto err_try;
 Dee_Decref(result);
 result = merge;
 /* Warn if we didn't parse any handlers. */
 if unlikely(unlikely(!catchc) &&
             WARN(W_EXPECTED_CATCH_OR_FINALLY_AFTER_TRY))
    goto err_r;
 return result;
err_try_flags:
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
err_try:
 while (catchc--) {
  Dee_XDecref(catchv[catchc].ce_mask);
  Dee_Decref(catchv[catchc].ce_code);
 }
 Dee_Free(catchv);
 /*goto err_r;*/
err_r:
 Dee_Decref(result);
err:
 return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_TRY_C */
