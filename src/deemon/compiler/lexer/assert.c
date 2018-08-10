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
#ifndef GUARD_DEEMON_COMPILER_LEXER_ASSERT_C
#define GUARD_DEEMON_COMPILER_LEXER_ASSERT_C 1

#include <deemon/api.h>
#include <deemon/compiler/tpp.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>

DECL_BEGIN

#undef CONFIG_ASSERT_DDI_USES_EXPRESSION
#define CONFIG_ASSERT_DDI_USES_EXPRESSION 1

INTERN DREF DeeAstObject *DCALL
ast_parse_assert(bool needs_parenthesis) {
 DREF DeeAstObject *result,*message,*merge;
#ifndef CONFIG_ASSERT_DDI_USES_EXPRESSION
 struct ast_loc loc;
#endif /* !CONFIG_ASSERT_DDI_USES_EXPRESSION */
 ASSERT(tok == KWD_assert);
#ifndef CONFIG_ASSERT_DDI_USES_EXPRESSION
 loc_here(&loc);
#endif /* !CONFIG_ASSERT_DDI_USES_EXPRESSION */
 if unlikely(yield() < 0) goto err;
 message = NULL;
 if (tok == '(') {
  /* Special case: We must be able to handle both of these:
   * >> assert (foo == bar), "Error";
   * >> ASSERT(foo == bar, "Error");
   */
  result = ast_parse_expression(LOOKUP_SYM_NORMAL);
  if unlikely(!result) goto err;
  if (!needs_parenthesis && tok == ',') {
   /* The message was passed individually. */
   if unlikely(yield() < 0) goto err_r;
   message = ast_parse_expression(LOOKUP_SYM_NORMAL);
   if unlikely(!message) goto err_r;
  } else if (result->ast_type == AST_MULTIPLE &&
             result->ast_flag == AST_FMULTIPLE_TUPLE &&
             result->ast_multiple.ast_exprc >= 2) {
   /* Steal the last expression and use it as message. */
   message = result->ast_multiple.ast_exprv[--result->ast_multiple.ast_exprc];
   if (result->ast_multiple.ast_exprc == 1) {
    merge = result->ast_multiple.ast_exprv[0];
    Dee_Incref(merge);
    Dee_Decref(result);
    result = merge;
   }
  }
 } else {
  if (needs_parenthesis &&
      WARN(W_EXPECTED_LPAREN_AFTER_ASSERT_IN_EXPRESSION))
      goto err;
  result = ast_parse_expression(LOOKUP_SYM_NORMAL);
  if unlikely(!result) goto err;
  if (tok == ',') {
   if unlikely(yield() < 0) goto err_r;
   message = ast_parse_expression(LOOKUP_SYM_NORMAL);
   if unlikely(!message) goto err_r;
  }
 }
 /* Create the assert branch. */
 merge = ast_setddi(message ? ast_action2(AST_FACTION_ASSERT_M,result,message)
                            : ast_action1(AST_FACTION_ASSERT,result),
#ifdef CONFIG_ASSERT_DDI_USES_EXPRESSION
                   &result->ast_ddi
#else /* CONFIG_ASSERT_DDI_USES_EXPRESSION */
                   &loc
#endif /* !CONFIG_ASSERT_DDI_USES_EXPRESSION */
                   );
 Dee_XDecref(message);
 Dee_Decref(result);
 return merge;
err_r: Dee_Decref(result);
err:   return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_ASSERT_C */
