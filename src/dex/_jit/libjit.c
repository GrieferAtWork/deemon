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
#ifndef GUARD_DEX_JIT_LIBJIT_C
#define GUARD_DEX_JIT_LIBJIT_C 1

#include "libjit.h"
#include <deemon/arg.h>
#include <deemon/objmethod.h>
#include <deemon/bytes.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/error.h>
#include <deemon/file.h>

DECL_BEGIN

/* !!! THIS MODULE IS NON-STANDARD AND DRIVES THE BUILTIN `exec' FUNCTION FOR !!!
 * !!! THE GATW IMPLEMENTATION OF DEEMON                                      !!!
 * --------------------------------------------------------------------------------
 * Because this module is non-portable between deemon implementations, it's name
 * starts with a leading underscore, indicative of this fact. */


PRIVATE DREF DeeObject *DCALL
libjit_exec_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 DREF DeeObject *result; DeeObject *globals;
 JITContext context; JITLexer lexer;
 char const *usertext; size_t usersize;
 DeeThreadObject *ts;
 PRIVATE struct keyword kwlist[] = { K(expr), K(globals), K(base), KEND };
 context.jc_impbase = NULL;
 if (DeeArg_UnpackKw(argc,argv,kw,kwlist,"o|oo:exec",
                    &lexer.jl_text,
                    &globals,
                    &context.jc_impbase))
     goto err;
 if (DeeString_Check(lexer.jl_text)) {
  usertext = DeeString_AsUtf8(lexer.jl_text);
  if unlikely(!usertext)
     goto err;
  usersize = WSTR_LENGTH(usertext);
  Dee_Incref(lexer.jl_text);
 } else if (DeeBytes_Check(lexer.jl_text)) {
  usertext = (char *)DeeBytes_DATA(lexer.jl_text);
  usersize = DeeBytes_SIZE(lexer.jl_text);
  Dee_Incref(lexer.jl_text);
 } else {
  lexer.jl_text = DeeFile_ReadText(lexer.jl_text,(size_t)-1,true);
  if unlikely(!lexer.jl_text) goto err;
  if (DeeString_Check(lexer.jl_text)) {
   usertext = DeeString_AsUtf8(lexer.jl_text);
   if unlikely(!usertext)
      goto err_expr;
   usersize = WSTR_LENGTH(usertext);
  } else {
   usertext = (char *)DeeBytes_DATA(lexer.jl_text);
   usersize = DeeBytes_SIZE(lexer.jl_text);
  }
 }
 ts = DeeThread_Self();
 context.jc_locals.otp_tab = NULL;
 context.jc_locals.otp_ind = 0;
 context.jc_globals        = globals;
 context.jc_retval         = NULL;
 context.jc_except         = ts->t_exceptsz;
 context.jc_flags          = 0;
 lexer.jl_context          = &context;
 lexer.jl_errpos           = NULL;
 JITLValue_Init(&lexer.jl_lvalue);
 JITLexer_Start(&lexer,
               (unsigned char *)usertext,
               (unsigned char *)usertext + usersize);
 result = JITLexer_EvalComma(&lexer,
                             AST_COMMA_NORMAL |
                             AST_COMMA_STRICTCOMMA |
                             AST_COMMA_NOSUFFIXKWD |
                             AST_COMMA_PARSESINGLE,
                             NULL,
                             NULL);
 ASSERT(!result || (result == JIT_LVALUE) ==
       (lexer.jl_lvalue.lv_kind != JIT_LVALUE_NONE));
 /* Check if the resulting expression evaluates to an L-Value
  * If so, unpack that l-value to access the pointed-to object. */
 if (result == JIT_LVALUE) {
  result = JITLValue_GetValue(&lexer.jl_lvalue,
                              &context);
  JITLValue_Fini(&lexer.jl_lvalue);
 }
 ASSERT(ts->t_exceptsz >= context.jc_except);
 /* Check for non-propagated exceptions. */
 if (ts->t_exceptsz > context.jc_except) {
  if (context.jc_retval != JITCONTEXT_RETVAL_UNSET) {
   if (JITCONTEXT_RETVAL_ISSET(context.jc_retval))
       Dee_Decref(context.jc_retval);
   context.jc_retval = JITCONTEXT_RETVAL_UNSET;
  }
  Dee_XClear(result);
  while (ts->t_exceptsz > context.jc_except + 1) {
   DeeError_Print("Discarding secondary error\n",
                  ERROR_PRINT_DOHANDLE);
  }
 }
 if likely(result) {
  ASSERT(context.jc_retval == JITCONTEXT_RETVAL_UNSET);
  if unlikely(lexer.jl_tok != TOK_EOF) {
   DeeError_Throwf(&DeeError_SyntaxError,
                   "Expected EOF but got `%$s'",
                  (size_t)(lexer.jl_end - lexer.jl_tokstart),
                   lexer.jl_tokstart);
   lexer.jl_errpos = lexer.jl_tokstart;
   Dee_Clear(result);
   goto handle_error;
  }
 } else if (context.jc_retval != JITCONTEXT_RETVAL_UNSET) {
  if (JITCONTEXT_RETVAL_ISSET(context.jc_retval)) {
   result = context.jc_retval;
  } else {
   /* Exited code via unconventional means, such as `break' or `continue' */
   DeeError_Throwf(&DeeError_SyntaxError,
                   "Attempted to use `break' or `continue' used outside of a loop");
   lexer.jl_errpos = lexer.jl_tokstart;
   goto handle_error;
  }
 } else {
  if (!lexer.jl_errpos)
       lexer.jl_errpos = lexer.jl_tokstart;
handle_error:
  JITLValue_Fini(&lexer.jl_lvalue);
  /* TODO: Somehow remember that the error happened at `lexer.jl_errpos' */
  ;
 }
 ASSERT(!globals || context.jc_globals == globals);
 if (context.jc_globals != globals)
     Dee_Decref(context.jc_globals);
 Dee_Decref_unlikely(lexer.jl_text);
 return result;
err_expr:
 Dee_Decref(lexer.jl_text);
err:
 return NULL;
}

PRIVATE DEFINE_KWCMETHOD(libjit_exec,&libjit_exec_f);




PRIVATE struct dex_symbol symbols[] = {
    { "exec", (DeeObject *)&libjit_exec, MODSYM_FNORMAL,
      DOC("(expr:?X3?Dstring?Dbytes?Dfile,globals?:?S?T2?Dstring?O,base?:?Dmodule)->\n"
          "Execute a given expression @expr and return the result\n"
          "This function is used to implement the builtin :deemon.exec function") },
    { "function_", (DeeObject *)&JITFunction_Type },
    { NULL }
};

PRIVATE void DCALL
libjit_fini(DeeDexObject *__restrict UNUSED(self)) {
 jit_object_table_clear((size_t)-1);
}

PUBLIC struct dex DEX = {
    /* .d_symbols = */symbols,
    /* .d_init    = */NULL,
    /* .d_fini    = */&libjit_fini,
};

DECL_END

#endif /* !GUARD_DEX_JIT_LIBJIT_C */
