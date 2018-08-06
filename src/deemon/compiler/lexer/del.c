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
#ifndef GUARD_DEEMON_COMPILER_LEXER_DEL_C
#define GUARD_DEEMON_COMPILER_LEXER_DEL_C 1

#include <deemon/api.h>
#include <deemon/tuple.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>

DECL_BEGIN

PRIVATE DREF DeeAstObject *DCALL
ast_parse_del_single(unsigned int lookup_mode) {
 DREF DeeAstObject *result;
 DREF DeeAstObject *new_result;
 result = ast_parse_unary(lookup_mode);
 if unlikely(!result) goto err;
 switch (result->ast_type) {
 case AST_SYM:
  /* Create an unbind AST. */
  new_result = ast_setddi(ast_unbind(result->ast_sym),
                         &result->ast_ddi);
  if unlikely(!new_result) goto err_r;
  Dee_Decref(result);
  result = new_result;
  if ((lookup_mode&LOOKUP_SYM_ALLOWDECL) &&
       result->ast_unbind->sym_scope == current_scope) {
   /* Delete the actual symbol (don't just unbind it). */
   del_local_symbol(result->ast_unbind);
  }
  break;

 {
  uint16_t new_operator;
 case AST_OPERATOR:
  switch (result->ast_flag) {
  case OPERATOR_GETATTR:  new_operator = OPERATOR_DELATTR; break;
  case OPERATOR_GETITEM:  new_operator = OPERATOR_DELITEM; break;
  case OPERATOR_GETRANGE: new_operator = OPERATOR_DELRANGE; break;
  default: goto default_case;
  }
  /* Create a new operator branch. */
  if unlikely(result->ast_operator.ast_exflag&AST_OPERATOR_FVARARGS)
     goto create_2;
  if unlikely(!result->ast_operator.ast_opb) {
   new_result = ast_operator1(new_operator,
                              result->ast_operator.ast_exflag,
                              result->ast_operator.ast_opa);
  } else if unlikely(result->ast_operator.ast_opd) {
   new_result = ast_operator4(new_operator,
                              result->ast_operator.ast_exflag,
                              result->ast_operator.ast_opa,
                              result->ast_operator.ast_opb,
                              result->ast_operator.ast_opc,
                              result->ast_operator.ast_opd);
  } else if (result->ast_operator.ast_opc) {
   new_result = ast_operator3(new_operator,
                              result->ast_operator.ast_exflag,
                              result->ast_operator.ast_opa,
                              result->ast_operator.ast_opb,
                              result->ast_operator.ast_opc);
  } else {
create_2:
   new_result = ast_operator2(new_operator,
                              result->ast_operator.ast_exflag,
                              result->ast_operator.ast_opa,
                              result->ast_operator.ast_opb);
  }
  if unlikely(!new_result) goto err_r;
  ast_setddi(new_result,&result->ast_ddi);
  Dee_Decref(result);
  result = new_result;
 } break;

 default:
default_case:
  if (WARN(W_UNEXPECTED_EXPRESSION_FOR_DEL))
      goto err_r;
  break;
 }
 return result;
err_r:
 Dee_Decref(result);
err:
 return NULL;
}


INTERN DREF DeeAstObject *DCALL
ast_parse_del(unsigned int lookup_mode) {
 DREF DeeAstObject *result;
 size_t delc,dela; DREF DeeAstObject **delv;
 /* Parse additional lookup modifiers. */
 if (ast_parse_lookup_mode(&lookup_mode))
     goto err;
 result = ast_parse_del_single(lookup_mode);
 if unlikely(!result) goto err;
 if (tok == ',') {
  /* Delete-multiple expression. */
  if unlikely(yield() < 0) goto err_r;
  /* Check for relaxed comma-rules. */
  if (!maybe_expression_begin()) goto done;
  delv = (DREF DeeAstObject **)Dee_Malloc(2*sizeof(DREF DeeAstObject *));
  if unlikely(!delv) goto err_r;
  dela = 2,delc = 1;
  delv[0] = result; /* Inherit */
  do {
   result = ast_parse_del_single(lookup_mode);
   if unlikely(!result) goto err_delv;
   if (delc == dela) {
    DREF DeeAstObject **new_delv;
    size_t new_dela = dela*2;
do_realloc_delv:
    new_delv = (DREF DeeAstObject **)Dee_TryRealloc(delv,new_dela*
                                                    sizeof(DREF DeeAstObject *));
    if unlikely(!new_delv) {
     if (new_dela != delc+1) { new_dela = delc+1; goto do_realloc_delv; }
     if (Dee_CollectMemory(new_dela*sizeof(DREF DeeAstObject *))) goto do_realloc_delv;
     goto err_delv_r;
    }
    delv = new_delv;
    dela = new_dela;
   }
   delv[delc++] = result; /* Inherit */
   if (tok != ',') break;
   if unlikely(yield() < 0) goto err_delv;
  } while (maybe_expression_begin());
  /* Pack all delete expression together into a multiple-branch. */
  if (delc != dela) {
   DREF DeeAstObject **new_delv;
   new_delv = (DREF DeeAstObject **)Dee_TryRealloc(delv,delc*
                                                   sizeof(DREF DeeAstObject *));
   if likely(new_delv) delv = new_delv;
  }
  result = ast_multiple(AST_FMULTIPLE_KEEPLAST,delc,delv);
  if unlikely(!result) goto err_delv;
  /* Upon success, the multiple-branch inherited all `delv' expressions. */
 }
done:
 return result;
err_delv_r:
 Dee_Decref(result);
err_delv:
 while (delc--) Dee_Decref(delv[delc]);
 Dee_Free(delv);
 goto err;
err_r:
 Dee_Decref(result);
err:
 return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_DEL_C */
