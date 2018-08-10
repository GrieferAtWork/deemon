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
#ifndef GUARD_DEEMON_COMPILER_LEXER_CAST_C
#define GUARD_DEEMON_COMPILER_LEXER_CAST_C 1

#include <deemon/api.h>
#include <deemon/tuple.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>

DECL_BEGIN

INTERN DREF DeeAstObject *FCALL
ast_parse_cast(DeeAstObject *__restrict typeexpr) {
 uint32_t old_flags;
 DREF DeeAstObject *result,*merge,**exprv;
 ASSERT_OBJECT_TYPE(typeexpr,&DeeAst_Type);
 switch (tok) {

 {
  char *next_tok;
  struct TPPFile *next_file;
 case '!':
  /* Special handling required:
   * >> (int)!!!42;         // This...
   * >> (int)!!!in my_list; // ... vs. this
   * After parsing any number of additional `!' tokens, if the token
   * thereafter is the keyword `is' or `in', then this isn't a cast
   * expression. However if it isn't, then it is a cast expression. */
  next_tok = peek_next_token(&next_file);
  for (;;) {
   if unlikely(!next_tok) goto err;
   if (*next_tok != '!') break;
   next_tok = peek_next_advance(next_tok+1,&next_file);
  }
  if (*next_tok++ != 'i') goto do_a_cast;
  while (SKIP_WRAPLF(next_tok,token.t_file->f_end));
  if (*next_tok != 's' && *next_tok != 'n') goto do_a_cast;
  /* This isn't a cast expression. */
  goto not_a_cast;
 }

 case '+': /* `(typexpr).operator add(castexpr)' vs. `(typexpr)castexpr.operator pos()' */
 case '-': /* `(typexpr).operator sub(castexpr)' vs. `(typexpr)castexpr.operator neg()' */
 case '<': /* `(typexpr).operator lo(castexpr)' vs. `(typexpr)(cell(castexpr))' */
 case '[': /* `(typexpr).operator [](castexpr)' vs. `(typexpr)(list(castexpr))' */
  if (WARN(W_UNCLEAR_CAST_INTENT))
      goto err;
not_a_cast:
  /* Not a cast expression. */
  result = typeexpr;
  Dee_Incref(result);
  break;

#if 1
 {
  struct ast_loc loc;
  bool second_paren;
 case '(':
  loc_here(&loc);
  /* Special handling for the following cases:
   * >> (float)();              // Call with 0 arguments
   * >> (float)(42);            // Call with 1 argument `42'
   * >> (float)((42),);         // Call with 1 argument `42'
   * >> (float)(10,20,30);      // Call with 3 arguments `10,20,30'
   * >> (float)(pack 10,20,30); // Call with 1 argument `(10,20,30)'
   * Without this handling, the 4th line would be compiled as
   * `float(pack(10,20,30))', when we want it to be `float(10,20,30)' */
  old_flags = TPPLexer_Current->l_flags;
  TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
  if unlikely(yield() < 0) goto err_flags;
  if (tok == ')') {
   /* Handle case #0 */
   TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
   merge = ast_setddi(ast_constexpr(Dee_EmptyTuple),&loc);
   if unlikely(!merge) goto err;
   result = ast_setddi(ast_operator2(OPERATOR_CALL,AST_OPERATOR_FNORMAL,
                                     typeexpr,merge),
                      &typeexpr->ast_ddi);
   Dee_Decref(merge);
   if unlikely(!result) goto err;
   if unlikely(yield() < 0) goto err_r;
   goto done;
  }
  second_paren = tok == '(';
  /* Parse the cast-expression / argument list. */
  merge = ast_parse_comma(AST_COMMA_FORCEMULTIPLE,
                          AST_FMULTIPLE_TUPLE,
                          NULL);
  if unlikely(!merge) goto err_flags;
  ASSERT(merge->ast_type == AST_MULTIPLE);
  ASSERT(merge->ast_flag == AST_FMULTIPLE_TUPLE);
  TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
  if unlikely(likely(tok == ')') ? (yield() < 0) :
              WARN(W_EXPECTED_RPAREN_AFTER_LPAREN))
     goto err_merge;
  if (!second_paren &&
       merge->ast_multiple.ast_exprc == 1) {
   /* Recursively parse cast suffix expressions:
    * >> (int)(float)get_value(); 
    * Parse as:
    * >> int(float(get_value()));
    * Without this, it would be parsed as:
    * >> int(float)(get_value());
    */
   result = merge->ast_multiple.ast_exprv[0];
   result = ast_parse_cast(result);
   if unlikely(!result) goto err_merge;
   if (result == merge->ast_multiple.ast_exprv[0]) {
    /* Same argument expression. */
    Dee_Decref(result);
   } else {
    /* New argument expression. */
    if likely(!DeeObject_IsShared(merge)) {
     Dee_Decref(merge->ast_multiple.ast_exprv[0]);
     merge->ast_multiple.ast_exprv[0] = result; /* Inherit reference. */
    } else {
     DREF DeeAstObject *other;
     exprv = (DREF DeeAstObject **)Dee_Malloc(1*sizeof(DREF DeeAstObject *));
     if unlikely(!exprv) goto err_merge_r;
     exprv[0] = result; /* Inherit */
     other = ast_multiple(AST_FMULTIPLE_TUPLE,1,exprv);
     if unlikely(!other) { Dee_Free(exprv); goto err_merge_r; }
     Dee_Decref(merge);
     merge = other; /* Inherit */
    }
   }
  }

  result = ast_setddi(ast_operator2(OPERATOR_CALL,AST_OPERATOR_FNORMAL,
                                    typeexpr,merge),
                     &typeexpr->ast_ddi);
  Dee_Decref(merge);
  if unlikely(!result) goto err;
 } break;
#endif


 default:
  /* If what follows still isn't the start of
   * an expression, then this isn't a cast. */
  if (!maybe_expression_begin())
       goto not_a_cast;
do_a_cast:
  /* Actually do a cast. */
  result = ast_parse_unary(LOOKUP_SYM_NORMAL);
  if unlikely(!result) return NULL;
  /* Use the parsed branch in a single-argument call-operator invocation. */
  exprv = (DREF DeeAstObject **)Dee_Malloc(1*sizeof(DREF DeeAstObject *));
  if unlikely(!exprv) goto err_r;
  exprv[0] = result; /* Inherit */
  /* Pack together the argument list branch. */
  merge = ast_setddi(ast_multiple(AST_FMULTIPLE_TUPLE,1,exprv),
                    &typeexpr->ast_ddi);
  if unlikely(!merge) goto err_r_exprv;
  /* Create the call expression branch. */
  result = ast_setddi(ast_operator2(OPERATOR_CALL,AST_OPERATOR_FNORMAL,
                                    typeexpr,merge),
                     &typeexpr->ast_ddi);
  Dee_Decref(merge);
  break;
 }
done:
 return result;
err_flags:   
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
 goto err;
err_merge_r: Dee_Decref(result);
err_merge:   Dee_Decref(merge); goto err;
err_r_exprv: Dee_Free(exprv);
err_r:       Dee_Decref(result);
err:         return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_CAST_C */
