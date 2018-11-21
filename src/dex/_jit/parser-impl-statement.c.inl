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

#include <deemon/thread.h>

DECL_BEGIN

INTERN RETURN_TYPE FCALL
FUNC(StatementBlock)(JITLexer *__restrict self) {
 RETURN_TYPE result;
#ifndef JIT_EVAL
 /* Special case optimization:
  *  -> Since we're supposed to skip the statement, we can optimize for
  *     this case by scanning ahead for the next matching `}' token! */
 if (JITLexer_SkipPair(self,'{','}'))
     goto err;
 result = 0;
#else
 for (;;) {
#ifdef JIT_EVAL
  result = JITLexer_EvalStatement(self);
#else
  result = JITLexer_SkipStatement(self);
#endif
  if (ISERR(result)) goto err;
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
 JITLexer_Yield(self);
#endif
 return result;
err:
 return ERROR;
}


INTERN RETURN_TYPE FCALL
FUNC(Statement)(JITLexer *__restrict self) {
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
  IF_EVAL(JITContext_PushScope(self->jl_context));
  result = FUNC(StatementBlock)(self);
  LOAD_LVALUE(result,err_popscope);
  IF_EVAL(JITContext_PopScope(self->jl_context));
  break;

 {
  char const *tok_begin;
  size_t tok_length;
  uint32_t name;
 case JIT_KEYWORD:
  tok_begin  = (char *)self->jl_tokstart;
  tok_length = (size_t)((char *)self->jl_tokend - tok_begin);
  switch (tok_length) {

  case 2:
   if (self->jl_tokstart[0] == 'i' &&
       self->jl_tokstart[1] == 'f') {
    IF_EVAL(JITContext_PushScope(self->jl_context));
    result = FUNC(If)(self,true);
    LOAD_LVALUE(result,err_popscope);
    IF_EVAL(JITContext_PopScope(self->jl_context));
    goto done;
   }
   if (self->jl_tokstart[0] == 'd' &&
       self->jl_tokstart[1] == 'o') {
    IF_EVAL(JITContext_PushScope(self->jl_context));
    result = FUNC(Do)(self,true);
    LOAD_LVALUE(result,err_popscope);
    IF_EVAL(JITContext_PopScope(self->jl_context));
    goto done;
   }
   break;

  case 3:
   if (self->jl_tokstart[0] == 't' &&
       self->jl_tokstart[1] == 'r' &&
       self->jl_tokstart[2] == 'y') {
    result = FUNC(Try)(self,true);
    goto done;
   }
   if (self->jl_tokstart[0] == 'f' &&
       self->jl_tokstart[1] == 'o' &&
       self->jl_tokstart[2] == 'r') {
    IF_EVAL(JITContext_PushScope(self->jl_context));
    result = FUNC(For)(self,true);
    LOAD_LVALUE(result,err_popscope);
    IF_EVAL(JITContext_PopScope(self->jl_context));
    goto done;
   }
   if (self->jl_tokstart[0] == 'd' &&
       self->jl_tokstart[1] == 'e' &&
       self->jl_tokstart[2] == 'l') {
    result = FUNC(Del)(self,true);
    goto done;
   }
   break;

  case 4:
   name = UNALIGNED_GET32((uint32_t *)tok_begin);
   if (name == ENCODE4('w','i','t','h')) {
    IF_EVAL(JITContext_PushScope(self->jl_context));
    result = FUNC(With)(self,true);
    LOAD_LVALUE(result,err_popscope);
    IF_EVAL(JITContext_PopScope(self->jl_context));
    goto done;
   }
   if (name == ENCODE4('f','r','o','m')) {
    /* TODO */
   }
   break;

  case 5:
   name = UNALIGNED_GET32((uint32_t *)tok_begin);
   if (name == ENCODE4('t','h','r','o') &&
       *(uint8_t *)(tok_begin + 4) == 'w') {
    JITLexer_Yield(self);
    if (self->jl_tok == ';') {
     JITLexer_Yield(self);
#ifdef JIT_EVAL
     /* Rethrow the last active exception. */
     Dee_XClear(self->jl_context->jc_retval);
     if (DeeThread_Self()->t_exceptsz <= self->jl_context->jc_except)
         err_no_active_exception();
     result = NULL;
#else
     result = 0;
#endif
    } else {
     result = FUNC(Comma)(self,AST_COMMA_NORMAL,IF_EVAL(&DeeTuple_Type,)NULL);
     if (ISERR(result)) goto err;
     LOAD_LVALUE(result,err);
     if likely(self->jl_tok == ';') {
      JITLexer_Yield(self);
     } else {
      SYNTAXERROR("Expected `;' after `throw', but got `%$s'",
                 (size_t)(self->jl_tokend - self->jl_tokstart),
                  self->jl_tokstart);
      goto err_r;
     }
#ifdef JIT_EVAL
     Dee_XClear(self->jl_context->jc_retval);
     /* Throw the given object. */
     DeeError_Throw(result);
     Dee_Clear(result);
#endif
    }
    goto done;
   }
   if (name == ENCODE4('w','h','i','l') &&
       *(uint8_t *)(tok_begin + 4) == 'e') {
    IF_EVAL(JITContext_PushScope(self->jl_context));
    result = FUNC(While)(self,true);
    LOAD_LVALUE(result,err_popscope);
    IF_EVAL(JITContext_PopScope(self->jl_context));
    goto done;
   }
   if (name == ENCODE4('y','i','e','l') &&
       *(uint8_t *)(tok_begin + 4) == 'd') {
    /* TODO */
   }
   if (name == ENCODE4('p','r','i','n') &&
       *(uint8_t *)(tok_begin + 4) == 't') {
    /* TODO */
   }
   if (name == ENCODE4('b','r','e','a') &&
       *(uint8_t *)(tok_begin + 4) == 'k') {
    /* TODO */
   }
   if (name == ENCODE4('_','_','a','s') &&
       *(uint8_t *)(tok_begin + 4) == 'm')
       goto do_asm;
   break;

  case 6:
   name = UNALIGNED_GET32((uint32_t *)tok_begin);
   if (name == ENCODE4('r','e','t','u') &&
       UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE2('r','n')) {
    JITLexer_Yield(self);
    if (self->jl_tok == ';') {
     JITLexer_Yield(self);
#ifdef JIT_EVAL
     Dee_XDecref(self->jl_context->jc_retval);
     self->jl_context->jc_retval = Dee_None;
     Dee_Incref(Dee_None);
     result = NULL;
#else
     result = 0;
#endif
    } else {
     result = FUNC(Comma)(self,AST_COMMA_NORMAL,IF_EVAL(&DeeTuple_Type,)NULL);
     if (ISERR(result)) goto err;
     LOAD_LVALUE(result,err);
     if likely(self->jl_tok == ';') {
      JITLexer_Yield(self);
     } else {
      SYNTAXERROR("Expected `;' after `return', but got `%$s'",
                 (size_t)(self->jl_tokend - self->jl_tokstart),
                  self->jl_tokstart);
      goto err_r;
     }
#ifdef JIT_EVAL
     Dee_XDecref(self->jl_context->jc_retval);
     self->jl_context->jc_retval = result; /* Inherit reference. */
     result = NULL; /* Propagate the return value. */
#endif
    }
    goto done;
   }
   if (name == ENCODE4('a','s','s','e') &&
       UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE2('r','t')) {
    IF_EVAL(JITContext_PushScope(self->jl_context));
    result = FUNC(Assert)(self,true);
    LOAD_LVALUE(result,err_popscope);
    IF_EVAL(JITContext_PopScope(self->jl_context));
    goto done;
   }
   if (name == ENCODE4('i','m','p','o') &&
       UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE2('r','t')) {
    result = FUNC(Import)(self,false);
    goto done;
   }
   if (name == ENCODE4('s','w','i','t') &&
       UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE2('c','h')) {
    /* TODO */
   }
   break;

  case 7:
   name = UNALIGNED_GET32((uint32_t *)tok_begin);
   if (name == ENCODE4('f','o','r','e') &&
       UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE2('a','c') &&
       *(uint8_t *)(tok_begin + 6) == 'h') {
    IF_EVAL(JITContext_PushScope(self->jl_context));
    result = FUNC(Foreach)(self,true);
    LOAD_LVALUE(result,err_popscope);
    IF_EVAL(JITContext_PopScope(self->jl_context));
    goto done;
   }
   if (name == ENCODE4('_','_','a','s') &&
       UNALIGNED_GET16((uint16_t *)(tok_begin + 4)) == ENCODE2('m','_') &&
       *(uint8_t *)(tok_begin + 6) == '_') {
do_asm:
    /* TODO */
    ;
   }
   break;

  {
   uint32_t nam2;
  case 8:
   name = UNALIGNED_GET32((uint32_t *)tok_begin);
   nam2 = UNALIGNED_GET32((uint32_t *)(tok_begin + 4));
   if (name == ENCODE4('c','o','n','t') &&
       nam2 == ENCODE4('i','n','u','e')) {
    /* TODO */
   }
  } break;


  default: break;
  }
  goto parse_expr;
 } break;

 default:
parse_expr:
  /* Fallback: parse a regular, old expression. */
  result = FUNC(Comma)(self,
                       AST_COMMA_NORMAL |
                       AST_COMMA_ALLOWVARDECLS |
                       AST_COMMA_ALLOWTYPEDECL |
                       AST_COMMA_PARSESEMI,
                       IF_EVAL(NULL,)
                       NULL);
  break;
 }
done:
 return result;
#ifdef JIT_EVAL
err_popscope:
 IF_EVAL(JITContext_PopScope(self->jl_context));
 goto err;
#endif /* JIT_EVAL */
err_r:
#ifdef JIT_EVAL
 Dee_XDecref(result);
#endif /* JIT_EVAL */
err:
 return ERROR;
}

DECL_END

