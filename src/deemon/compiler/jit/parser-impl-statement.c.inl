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
#ifdef __INTELLISENSE__
#include "parser-impl.c.inl"
#endif

DECL_BEGIN

#ifdef JIT_EVAL
INTERN DREF DeeObject *FCALL
JITLexer_EvalStatement(JITLexer *__restrict self)
#else
INTERN int FCALL
JITLexer_SkipStatement(JITLexer *__restrict self)
#endif
{
 RETURN_TYPE result;
 switch (self->jl_tok) {

 case ';':
  JITLexer_Yield(self);
#ifdef JIT_EVAL
  result = Dee_None;
  Dee_Incref(Dee_None);
#else
  result = 0;
#endif
  break;

 case '{':
  JITLexer_Yield(self);
#ifndef JIT_EVAL
  /* Special case optimization:
   *  -> Since we're supposed to skip the statement, we can optimize for
   *     this case by scanning ahead for the next matching `}' token! */
  {
   unsigned int recursion = 1;
   for (;;) {
    switch (self->jl_tok) {
    case '{':
     ++recursion;
     break;
    case '}':
     JITLexer_Yield(self);
     --recursion;
     if (!recursion)
         goto done_skip;
     break;
    case TOK_EOF:
     SYNTAXERROR("Missing `}' following statement beginning with `{'");
     goto err;
    default: break;
    }
   }
done_skip:
   result = 0;
  }
#else
  IF_EVAL(JITContext_PushScope(self->jl_context));
  for (;;) {
#ifdef JIT_EVAL
   result = JITLexer_EvalStatement(self);
#else
   result = JITLexer_SkipStatement(self);
#endif
   if (ISERR(result)) goto err_popscope;
    if (self->jl_tok == '}') break;
#ifdef JIT_EVAL
   /* Discard the intermediate statement result. */
   if (result == JIT_LVALUE) {
    JITLValue_Fini(&self->jl_lvalue);
    JITLValue_Init(&self->jl_lvalue);
   } else {
    Dee_Decref(result);
   }
#endif
  }
  /* Make sure to unwind l-values to prevent corrupt access
   * to local variables from the scope we're about to pop. */
  LOAD_LVALUE(result,err_popscope);
  IF_EVAL(JITContext_PopScope(self->jl_context));
  JITLexer_Yield(self);
#endif
  break;

 default:
  /* Fallback: parse a regular, old expression. */
#ifdef JIT_EVAL
  result = JITLexer_EvalComma(self,
                              AST_COMMA_NORMAL |
                              AST_COMMA_ALLOWVARDECLS |
                              AST_COMMA_ALLOWTYPEDECL |
                              AST_COMMA_PARSESEMI,
                              NULL,
                              NULL);
#else
  result = JITLexer_SkipComma(self,
                              AST_COMMA_NORMAL |
                              AST_COMMA_ALLOWVARDECLS |
                              AST_COMMA_ALLOWTYPEDECL |
                              AST_COMMA_PARSESEMI,
                              NULL);
#endif
  break;
 }
 return result;
#ifdef JIT_EVAL
err_popscope:
 JITContext_PopScope(self->jl_context);
#endif
IF_SKIP(err:)
 return ERROR;
}

DECL_END

