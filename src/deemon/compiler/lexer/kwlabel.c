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
#ifndef GUARD_DEEMON_COMPILER_LEXER_KWLABEL_C
#define GUARD_DEEMON_COMPILER_LEXER_KWLABEL_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/tuple.h>

DECL_BEGIN


INTERN DREF DeeAstObject *DCALL
ast_parse_argument_list(uint16_t mode,
                        DREF DeeAstObject **__restrict pkeyword_labels) {
 DREF DeeAstObject *result;
 DREF DeeAstObject *kwdlist_ast;
 DeeObject *kwdlist;
 *pkeyword_labels = NULL;
 ASSERT(mode & AST_COMMA_FORCEMULTIPLE);
 result = ast_parse_comma(mode|AST_COMMA_ALLOWKWDLIST,
                          AST_FMULTIPLE_TUPLE,
                          NULL);
 if unlikely(!result) goto err;
 if (tok == TOK_POW) {
  /* XXX: I really don't like using `**' for this.
   *      I realize that I _have_ to provide some way
   *      of passing arbitrary mappings through keywords,
   *      however this just feel too python-�sque to me...
   */
  if unlikely(yield() < 0) goto err_r;
  /* Parse the keyword invocation AST. */
  *pkeyword_labels = ast_parse_expression(LOOKUP_SYM_NORMAL);
  if unlikely(!*pkeyword_labels) goto err_r;
 } else if (TPP_ISKEYWORD(tok)) {
  char *next = peek_next_token(NULL);
  if unlikely(!next) goto err_r;
  if (*next == ':') {
   size_t multiple_a;
   if (result->ast_type == AST_CONSTEXPR) {
    ASSERT(result->ast_constexpr == Dee_EmptyTuple);
    Dee_Decref(Dee_EmptyTuple);
    result->ast_type = AST_MULTIPLE;
    result->ast_flag = AST_FMULTIPLE_TUPLE;
    result->ast_multiple.ast_exprc = 0;
    result->ast_multiple.ast_exprv = NULL;
   }
   kwdlist = DeeKwds_NewWithHint(1);
   if unlikely(!kwdlist) goto err_r;
   kwdlist_ast = ast_sethere(ast_constexpr(kwdlist));
   Dee_Decref(kwdlist);
   if unlikely(!kwdlist_ast) goto err_r;
   *pkeyword_labels = kwdlist_ast; /* Inherit reference (on success) */
   multiple_a = result->ast_multiple.ast_exprc;
   /* Parse a keyword label list! */
   for (;;) {
    DREF DeeAstObject *argument_value;
    /* Append the argument label. */
    if (DeeKwds_Append(&kwdlist,
                        token.t_kwd->k_name,
                        token.t_kwd->k_size,
                        hash_str(token.t_kwd->k_name)))
        goto err_r_kwdlist;
    if unlikely(yield() < 0) goto err_r_kwdlist;
    if unlikely(likely(tok == ':') ? (yield() < 0) :
                WARN(W_EXPECTED_COLLON_AFTER_KEYWORD_LABEL))
       goto err_r_kwdlist;
    /* Make sure that we have allocated sufficient memory for the keyword list. */
    ASSERT(result->ast_multiple.ast_exprc <= multiple_a);
    if (result->ast_multiple.ast_exprc >= multiple_a) {
     DREF DeeAstObject **new_astv;
     size_t new_alloc = multiple_a * 2;
     if unlikely(!new_alloc) new_alloc = 2;
     new_astv = (DREF DeeAstObject **)Dee_TryRealloc(result->ast_multiple.ast_exprv,
                                                     new_alloc*sizeof(DREF DeeAstObject *));
     if unlikely(!new_astv) {
      new_alloc = result->ast_multiple.ast_exprc + 1;
      new_astv = (DREF DeeAstObject **)Dee_Realloc(result->ast_multiple.ast_exprv,
                                                   new_alloc*sizeof(DREF DeeAstObject *));
      if unlikely(!new_astv) goto err_r_kwdlist;
     }
     result->ast_multiple.ast_exprv = new_astv;
     multiple_a = new_alloc;
    }
    /* Parse the expression that is bound to the keyword. */
    argument_value = ast_parse_expression(LOOKUP_SYM_NORMAL);
    if unlikely(!argument_value) goto err_r_kwdlist;
    result->ast_multiple.ast_exprv[result->ast_multiple.ast_exprc++] = argument_value; /* Inherit reference. */
    if (tok != ',') break;
    if (mode & AST_COMMA_STRICTCOMMA) {
     char *next_token = peek_next_token(NULL);
     if unlikely(!next_token) goto err_r_kwdlist;
     if (!DeeUni_IsSymCont(*next_token)) /* Can't be a label. */
          break;
    }
    if unlikely(yield() < 0) goto err_r_kwdlist;
    if (!TPP_ISKEYWORD(tok))
         break; /* End of argument list. */
   }
   ASSERT(result->ast_multiple.ast_exprc <= multiple_a);
   /* Flush unused memory from keyword-bound arguments. */
   if (multiple_a > result->ast_multiple.ast_exprc) {
    DREF DeeAstObject **new_astv;
    new_astv = (DREF DeeAstObject **)Dee_TryRealloc(result->ast_multiple.ast_exprv,
                                                    result->ast_multiple.ast_exprc*
                                                    sizeof(DREF DeeAstObject *));
    if likely(new_astv)
       result->ast_multiple.ast_exprv = new_astv;
   }
   /* The constant-branch for the keyword-list was already
    * constructed above, so we're already finished here! */
  }
 }
 return result;
err_r_kwdlist:
 Dee_Decref(kwdlist_ast);
err_r:
 Dee_Decref(result);
err:
 return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_KWLABEL_C */
