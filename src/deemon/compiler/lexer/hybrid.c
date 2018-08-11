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
#ifndef GUARD_DEEMON_COMPILER_LEXER_HYBRID_C
#define GUARD_DEEMON_COMPILER_LEXER_HYBRID_C 1

#include <deemon/api.h>
#include <deemon/none.h>
#include <deemon/tuple.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/util/string.h>

DECL_BEGIN

PRIVATE DREF DeeAstObject *FCALL ast_do_parse_brace_items(void) {
 DREF DeeAstObject *result;
 uint32_t old_flags = TPPLexer_Current->l_flags;
 TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
 if (tok == '\n' && yield() < 0) goto err_flags;
 result = ast_parse_brace_items();
 if unlikely(!result) goto err_flags;
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
 return result;
err_flags:
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
 return NULL;
}


/* @param: mode: Set of `AST_COMMA_*' - What is allowed and when should we pack values. */
INTERN DREF DeeAstObject *FCALL
ast_parse_statement_or_expression(uint16_t mode,
                                  unsigned int *pwas_expression) {
 DREF DeeAstObject *result;
 unsigned int was_expression;
 switch (tok) {


 case '{':
  result = ast_parse_statement_or_braces(&was_expression);
  if unlikely(!result) goto err;
  if (was_expression != AST_PARSE_WASEXPR_NO) {
   /* Try to parse a suffix expression.
    * If there was one, then we know that it actually was an expression. */
   unsigned long token_num = token.t_num;
   result = ast_parse_unary_postexpr(result);
   if (token_num != token.t_num)
       was_expression = AST_PARSE_WASEXPR_YES;
  }
  if (pwas_expression)
     *pwas_expression = was_expression;
  break;

 case KWD_try:
  result = ast_parse_try_hybrid(pwas_expression);
  break;

 case KWD_if:
  result = ast_parse_if_hybrid(pwas_expression);
  break;

 case KWD_with:
  result = ast_parse_with_hybrid(pwas_expression);
  break;

 case KWD_assert:
  result = ast_parse_assert_hybrid(pwas_expression);
  break;

 case KWD_import:
  result = ast_parse_import_hybrid(pwas_expression);
  break;

 case KWD_for:      /* TODO: generator expressions? */
 case KWD_foreach:  /* TODO: generator expressions? */
 case KWD_do:       /* TODO: generator expressions? */
 case KWD_while:    /* TODO: generator expressions? */

 case KWD_from:
 case KWD_del:
 case KWD_return:
 case KWD_yield:
 case KWD_throw:
 case KWD_print:
 case KWD_break:
 case KWD_continue:
#ifndef CONFIG_LANGUAGE_NO_ASM
 case KWD___asm:
 case KWD___asm__:
#endif /* !CONFIG_LANGUAGE_NO_ASM */
 case KWD_goto:
 case KWD_switch:
 case KWD_case:
 case KWD_default:
 case '@':
 case ';':
  result = ast_parse_statement(false);
  if (pwas_expression)
     *pwas_expression = AST_PARSE_WASEXPR_NO;
  break;

 {
  uint16_t comma_mode;
  size_t old_varc;
 default:
  old_varc = current_scope->s_mapc;
  comma_mode = 0;
  result = ast_parse_comma(mode,AST_FMULTIPLE_GENERIC,&comma_mode);
  if unlikely(!result) goto done;
  if (tok == ';' && (comma_mode & AST_COMMA_OUT_FNEEDSEMI)) {
   if unlikely(yield() < 0) goto err_r;
   if (pwas_expression)
      *pwas_expression = AST_PARSE_WASEXPR_NO;
  } else if (old_varc != current_scope->s_mapc) {
   if ((comma_mode & AST_COMMA_OUT_FNEEDSEMI) &&
        WARN(W_EXPECTED_SEMICOLLON_AFTER_EXPRESSION))
        goto err;
   if (pwas_expression)
      *pwas_expression = AST_PARSE_WASEXPR_NO;
  } else {
   if (pwas_expression)
      *pwas_expression = AST_PARSE_WASEXPR_YES;
  }
 } break;
 }
done:
 return result;
err_r:
 Dee_Decref(result);
err:
 return NULL;
}



INTERN DREF DeeAstObject *FCALL
ast_parse_if_hybrid(unsigned int *pwas_expression) {
 DREF DeeAstObject *tt_branch;
 DREF DeeAstObject *ff_branch;
 DREF DeeAstObject *result,*merge;
 uint16_t expect; struct ast_loc loc;
 uint32_t old_flags; unsigned int was_expression;
 expect = current_tags.at_expect;
 loc_here(&loc);
 if unlikely(yield() < 0) goto err;
 old_flags = TPPLexer_Current->l_flags;
 TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
 if unlikely(likely(tok == '(') ? (yield() < 0) :
             WARN(W_EXPECTED_LPAREN_AFTER_IF)) goto err_flags;
 result = ast_parse_expression(LOOKUP_SYM_NORMAL);
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
 if unlikely(!result) goto err;
 if unlikely(likely(tok == ')') ? (yield() < 0) :
             WARN(W_EXPECTED_RPAREN_AFTER_IF)) goto err;
 tt_branch = NULL;
 was_expression = AST_PARSE_WASEXPR_MAYBE;
 if (tok != KWD_else && tok != KWD_elif) {
  tt_branch = ast_parse_hybrid_primary(&was_expression);
  if unlikely(!tt_branch) goto err_r;
 }
 ff_branch = NULL;
 if (tok == KWD_elif) {
  token.t_id = KWD_if; /* Cheat a bit... */
  goto do_else_branch;
 }
 if (tok == KWD_else) {
  if unlikely(yield() < 0)
     goto err_tt;
do_else_branch:
  ff_branch = ast_parse_hybrid_secondary(&was_expression);
  if unlikely(!ff_branch)
     goto err_tt;
 }
 merge = ast_setddi(ast_conditional(AST_FCOND_EVAL|expect,
                                    result,
                                    tt_branch,
                                    ff_branch),
                   &loc);
 Dee_XDecref(ff_branch);
 Dee_XDecref(tt_branch);
 Dee_XDecref(result);
 if (pwas_expression)
    *pwas_expression = was_expression;
 return merge;
err_tt:
 Dee_XDecref(tt_branch);
err_r:
 Dee_Decref(result);
 goto err;
err_flags:
 TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
err:
 return NULL;
}



INTERN DREF DeeAstObject *FCALL
ast_parse_statement_or_braces(unsigned int *pwas_expression) {
 DREF DeeAstObject *result,**new_elemv;
 DREF DeeAstObject *remainder;
 struct ast_loc loc;
 unsigned int was_expression;
 ASSERT(tok == '{');
 loc_here(&loc);
 if unlikely(yield() < 0) goto err;
 switch (tok) {

 case '}':
  /* Special case: empty sequence. */
  result = ast_setddi(ast_multiple(AST_FMULTIPLE_GENERIC,0,NULL),&loc);
  if unlikely(!result) goto err;
  if unlikely(yield() < 0) goto err_r;
  if (pwas_expression)
     *pwas_expression = AST_PARSE_WASEXPR_MAYBE;
  break;

 case '.':
  result = ast_setddi(ast_do_parse_brace_items(),&loc);
  if unlikely(!result) goto err;
  if unlikely(likely(tok == '}') ? (yield() < 0) : 
              WARN(W_EXPECTED_RBRACE_AFTER_BRACEINIT))
     goto err_r;
  if (pwas_expression)
     *pwas_expression = AST_PARSE_WASEXPR_YES;
  break;


 case '{': /* Recursion! */
  if unlikely(scope_push() < 0) goto err;
  result = ast_parse_statement_or_braces(&was_expression);
  ASSERT(!result ||
          result->ast_type == AST_MULTIPLE ||
          result->ast_type == AST_CONSTEXPR);
parse_remainder_after_hybrid_popscope:
  if unlikely(!result) goto err;
parse_remainder_after_hybrid_popscope_resok:
  if (was_expression == AST_PARSE_WASEXPR_NO)
      goto parse_remainder_after_statement;
  if (was_expression == AST_PARSE_WASEXPR_YES) {
   result = ast_parse_unary_postexpr(result);
   if unlikely(!result) goto err;
check_recursion_after_expression_suffix:
   if (tok == ';') {
    if unlikely(yield() < 0) goto err_r;
    goto parse_remainder_after_statement;
   }
   if (tok == ':')
       goto parse_remainder_after_colon_popscope;
   if (tok == ',') {
parse_remainder_after_comma_popscope:
    scope_pop();
    remainder = ast_parse_brace_list(result);
    if unlikely(!remainder) goto err_r;
    Dee_Decref(result);
    result = ast_setddi(remainder,&loc);
    if unlikely(likely(tok == '}') ? (yield() < 0) : 
                WARN(W_EXPECTED_RBRACE_AFTER_BRACEINIT))
       goto err_r;
    if (pwas_expression)
       *pwas_expression = AST_PARSE_WASEXPR_YES;
    break;
   }
   if likely(tok == '}') {
parse_remainder_before_rbrace_popscope_wrap:
    if unlikely(yield() < 0)
       goto err_r;
   } else {
    if unlikely(WARN(W_EXPECTED_RBRACE_AFTER_BRACEINIT))
       goto err_r;
   }
   /* Wrap the result as a single sequence. */
   new_elemv = (DREF DeeAstObject **)Dee_Malloc(1 * sizeof(DREF DeeAstObject *));
   if unlikely(!new_elemv) goto err_r;
   new_elemv[0] = result; /* Inherit reference. */
   remainder = ast_multiple(AST_FMULTIPLE_GENERIC,1,new_elemv);
   if unlikely(!remainder) { Dee_Free(new_elemv); goto err_r; }
   /* `ast_multiple()' inherited `new_elemv' on success. */
   result = ast_setddi(remainder,&loc);
   goto parse_remainder_after_rbrace_popscope;
  }
  if (tok == ',')
      goto parse_remainder_after_comma_popscope;
  if (tok == ':')
      goto parse_remainder_after_colon_popscope;
  if (tok == '}')
      goto parse_remainder_before_rbrace_popscope_wrap;
  {
   unsigned long token_num = token.t_num;
   result = ast_parse_unary_postexpr(result);
   if unlikely(!result) goto err;
   if (token_num != token.t_num)
       goto check_recursion_after_expression_suffix;
  }
#if 0
  if (result->ast_type == AST_MULTIPLE)
      result->ast_flag = AST_FMULTIPLE_KEEPLAST;
#endif
  goto parse_remainder_after_statement;

 case KWD_try:
  if unlikely(scope_push() < 0) goto err;
  result = ast_parse_try_hybrid(&was_expression);
  goto parse_remainder_after_hybrid_popscope;

 case KWD_if:
  if unlikely(scope_push() < 0) goto err;
  result = ast_parse_if_hybrid(&was_expression);
  goto parse_remainder_after_hybrid_popscope;

 case KWD_with:
  if unlikely(scope_push() < 0) goto err;
  result = ast_parse_with_hybrid(&was_expression);
  goto parse_remainder_after_hybrid_popscope;

 case KWD_assert:
  if unlikely(scope_push() < 0) goto err;
  result = ast_parse_assert_hybrid(&was_expression);
parse_remainder_after_semicolon_hybrid_popscope:
  if unlikely(!result) goto err;

  /* Special case: `assert' statements require a trailing `;' token.
   *                If that token exists, we know for sure that this is a statement! */
  if (tok == ';') {
   was_expression = AST_PARSE_WASEXPR_NO;
   if unlikely(yield() < 0) goto err_r;
  }
  goto parse_remainder_after_hybrid_popscope_resok;

 case KWD_import:
  if unlikely(scope_push() < 0) goto err;
  result = ast_parse_import_hybrid(&was_expression);
  if unlikely(!result) goto err;
  /* Same as `assert': `import' requires a trailing `;' */
  goto parse_remainder_after_semicolon_hybrid_popscope;


 case KWD_for:      /* TODO: generator expressions? */
 case KWD_foreach:  /* TODO: generator expressions? */
 case KWD_do:       /* TODO: generator expressions? */
 case KWD_while:    /* TODO: generator expressions? */


 case KWD_from:
 case KWD_del:
 case KWD_return:
 case KWD_yield:
 case KWD_throw:
 case KWD_print:
 case KWD_break:
 case KWD_continue:
#ifndef CONFIG_LANGUAGE_NO_ASM
 case KWD___asm:
 case KWD___asm__:
#endif /* !CONFIG_LANGUAGE_NO_ASM */
 case KWD_goto:
 case KWD_switch:
 case KWD_case:
 case KWD_default:
 case '@':
 case ';':
is_a_statement:
  if unlikely(scope_push() < 0) goto err;
  if unlikely(yield() < 0) goto err;
  /* Enter a new scope and parse expressions. */
  if (parser_flags & PARSE_FLFSTMT)
      TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;
  result = ast_setddi(ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST,'}'),&loc);
  TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
  if unlikely(!result) goto err;
  while (tok == '\n') if unlikely(yield() < 0) goto err_r;
  if unlikely(likely(tok == '}') ? (yield() < 0) :
              WARN(W_EXPECTED_RBRACE_AFTER_LBRACE))
     goto err_r;
  scope_pop();
  if (pwas_expression)
     *pwas_expression = AST_PARSE_WASEXPR_NO;
  break;

 {
  uint16_t comma_mode;
 default:
  /* Check for a label definition. */
  if (TPP_ISKEYWORD(tok)) {
   char *next_token = peek_next_token(NULL);
   if unlikely(!next_token) goto err;
   if (*next_token == ':' &&
       (next_token = advance_wraplf(next_token),
       *next_token != ':' && *next_token != '='))
        goto is_a_statement; /* label */
  }
  /* Figure out what we're dealing with as we go. */
  if unlikely(scope_push() < 0) goto err;
  comma_mode = 0;
  result = ast_parse_comma(AST_COMMA_NORMAL|
                           AST_COMMA_FORCEMULTIPLE|
                           AST_COMMA_ALLOWVARDECLS|
                           AST_COMMA_PARSESEMI,
                           AST_FMULTIPLE_GENERIC,
                          &comma_mode);
  if unlikely(!result) goto err;
  ASSERT(result->ast_type == AST_MULTIPLE);
  ASSERT(result->ast_flag == AST_FMULTIPLE_GENERIC);
  if (!current_scope->s_mapc) {
   if (tok == '}') {
/*parse_remainder_before_rbrace_popscope:*/
    /* Sequence-like brace expression. */
    if unlikely(yield() < 0) goto err;
parse_remainder_after_rbrace_popscope:
    scope_pop();
    if (pwas_expression)
       *pwas_expression = AST_PARSE_WASEXPR_YES;
    break;
   }
   if (tok == ':' && result->ast_multiple.ast_exprc == 1) {
    /* Use the first expression from the multi-branch. */
    remainder = result->ast_multiple.ast_exprv[0];
    Dee_Incref(remainder);
    Dee_Decref(result);
    result = remainder;
parse_remainder_after_colon_popscope:
    scope_pop();
    /* mapping-like brace expression. */
    remainder = ast_parse_mapping(result);
    Dee_Decref(result);
    if unlikely(!remainder) goto err;
    result = ast_setddi(remainder,&loc);
    if unlikely(likely(tok == '}') ? (yield() < 0) : 
                WARN(W_EXPECTED_RBRACE_AFTER_BRACEINIT))
       goto err_r;
    if (pwas_expression)
       *pwas_expression = AST_PARSE_WASEXPR_YES;
    break;
   }
  }
  /* Statement expression. */
  if (comma_mode & AST_COMMA_OUT_FNEEDSEMI) {
   /* Consume a `;' token as part of the expression. */
   if unlikely(likely(tok == ';') ? (yield() < 0) : 
               WARN(W_EXPECTED_SEMICOLLON_AFTER_EXPRESSION))
      goto err_r;
  }
  if (result->ast_multiple.ast_exprc == 1) {
   remainder = result->ast_multiple.ast_exprv[0];
   Dee_Incref(remainder);
   Dee_Decref(result);
   result = remainder;
  } else {
   result->ast_flag = AST_FMULTIPLE_KEEPLAST;
  }
parse_remainder_after_statement:
  if (tok == '}') {
   ast_setddi(result,&loc);
   if unlikely(yield() < 0)
      goto err_r;
  } else {
   remainder = ast_parse_statements_until(AST_FMULTIPLE_KEEPLAST,'}');
   if unlikely(!remainder) goto err_r;
   if (remainder->ast_type == AST_MULTIPLE &&
       remainder->ast_flag == AST_FMULTIPLE_KEEPLAST &&
       remainder->ast_scope == current_scope) {
    new_elemv = (DREF DeeAstObject **)Dee_Realloc(remainder->ast_multiple.ast_exprv,
                                                 (remainder->ast_multiple.ast_exprc+1)*
                                                  sizeof(DREF DeeAstObject *));
    if unlikely(!new_elemv) goto err_r_remainder;
    MEMMOVE_PTR(new_elemv+1,new_elemv,remainder->ast_multiple.ast_exprc);
    remainder->ast_multiple.ast_exprv = new_elemv;
    new_elemv[0] = result; /* Inherit reference. */
    ++remainder->ast_multiple.ast_exprc;
   } else {
    new_elemv = (DREF DeeAstObject **)Dee_Malloc(2*sizeof(DREF DeeAstObject *));
    if unlikely(!new_elemv) goto err_r_remainder;
    new_elemv[0] = result;    /* Inherit reference. */
    new_elemv[1] = remainder; /* Inherit reference. */
    remainder = ast_multiple(AST_FMULTIPLE_KEEPLAST,2,new_elemv);
    if unlikely(!remainder) {
     Dee_Decref(new_elemv[1]);
     Dee_Decref(new_elemv[0]);
     Dee_Free(new_elemv);
     goto err;
    }
    /* `ast_multiple()' inherited `new_elemv' on success. */
   }
   result = ast_setddi(remainder,&loc);
   if unlikely(likely(tok == '}') ? (yield() < 0) :
               WARN(W_EXPECTED_RBRACE_AFTER_LBRACE))
      goto err_r;
  }
  scope_pop();
  if (pwas_expression)
     *pwas_expression = AST_PARSE_WASEXPR_NO;
 } break;

 }
 return result;
err_r_remainder:
 Dee_Decref(remainder);
err_r:
 Dee_Decref(result);
err:
 return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_HYBRID_C */
