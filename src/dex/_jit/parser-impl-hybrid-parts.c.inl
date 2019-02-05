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
#include "parser-impl-hybrid.c.inl"
#define JIT_HYBRID 1
#endif

#include <deemon/alloc.h>
#include <deemon/thread.h>

#ifdef JIT_HYBRID
#define JIT_ARGS       unsigned int *pwas_expression
#ifdef JIT_EVAL
#define EVAL_PRIMARY   JITLexer_EvalHybrid
#define EVAL_SECONDARY JITLexer_EvalHybridSecondary
#define H_FUNC(x)      JITLexer_Eval##x##Hybrid
#else
#define EVAL_PRIMARY   JITLexer_SkipHybrid
#define EVAL_SECONDARY JITLexer_SkipHybridSecondary
#define H_FUNC(x)      JITLexer_Skip##x##Hybrid
#endif
#define SKIP_PRIMARY   JITLexer_SkipHybrid
#define SKIP_SECONDARY JITLexer_SkipHybridSecondary
#define IF_HYBRID(...)  __VA_ARGS__
#define IF_NHYBRID(...) /* nothing */
#else
#define JIT_ARGS     bool is_statement
#ifdef JIT_EVAL
#define EVAL_PRIMARY(self,pwas_expression)   (is_statement ? JITLexer_EvalStatement(self) : JITLexer_EvalExpression(self,JITLEXER_EVAL_FNORMAL))
#define EVAL_SECONDARY(self,pwas_expression) (is_statement ? JITLexer_EvalStatement(self) : JITLexer_EvalExpression(self,JITLEXER_EVAL_FNORMAL))
#define H_FUNC(x)                             JITLexer_Eval##x
#else
#define EVAL_PRIMARY(self,pwas_expression)   (is_statement ? JITLexer_SkipStatement(self) : JITLexer_SkipExpression(self,JITLEXER_EVAL_FNORMAL))
#define EVAL_SECONDARY(self,pwas_expression) (is_statement ? JITLexer_SkipStatement(self) : JITLexer_SkipExpression(self,JITLEXER_EVAL_FNORMAL))
#define H_FUNC(x) JITLexer_Skip##x
#endif
#define SKIP_PRIMARY(self,pwas_expression)   (is_statement ? JITLexer_SkipStatement(self) : JITLexer_SkipExpression(self,JITLEXER_EVAL_FNORMAL))
#define SKIP_SECONDARY(self,pwas_expression) (is_statement ? JITLexer_SkipStatement(self) : JITLexer_SkipExpression(self,JITLEXER_EVAL_FNORMAL))
#define IF_HYBRID(...)  /* nothing */
#define IF_NHYBRID(...) __VA_ARGS__
#endif

DECL_BEGIN


INTERN RETURN_TYPE FCALL
H_FUNC(Try)(JITLexer *__restrict self, JIT_ARGS) {
 RETURN_TYPE result;
 IF_HYBRID(unsigned int was_expression;)
#ifdef JIT_EVAL
 unsigned char *start;
 ASSERT(JITLexer_ISKWD(self,"try"));
 JITLexer_Yield(self);
 start = self->jl_tokstart;
 result = EVAL_PRIMARY(self,&was_expression);
 if (result == JIT_LVALUE)
     result = JITLexer_PackLValue(self);
 if (self->jl_context->jc_flags & JITCONTEXT_FSYNERR)
     goto err;
 JITLexer_YieldAt(self,start);
 if (SKIP_PRIMARY(self,&was_expression))
     goto err;
 while (self->jl_tok == JIT_KEYWORD) {
  bool allow_interrupts = false;
  /* XXX: Full tagging support? */
  if (self->jl_tok == '@') {
   JITLexer_Yield(self);
   if (self->jl_tok == ':') {
    JITLexer_Yield(self);
    if (JITLexer_ISKWD(self,"interrupt")) {
     JITLexer_Yield(self);
     allow_interrupts = true;
    }
   }
  }
  if (JITLexer_ISTOK(self,"finally")) {
   DREF DeeObject *finally_value;
   DREF DeeObject *old_return_expr;
   JITLexer_Yield(self);
   old_return_expr = self->jl_context->jc_retval;
   self->jl_context->jc_retval = JITCONTEXT_RETVAL_UNSET;
   start = self->jl_tokstart;
   finally_value = EVAL_SECONDARY(self,&was_expression);
   /* Discard the finally-branch expression value. */
   if (ISOK(finally_value)) {
    if (finally_value == JIT_LVALUE) {
     JITLValue_Fini(&self->jl_lvalue);
     JITLValue_Init(&self->jl_lvalue);
    } else {
     Dee_Decref(finally_value);
    }
   }
   if (ISERR(finally_value)) {
    JITLexer_YieldAt(self,start);
    if (SKIP_SECONDARY(self,&was_expression))
        goto err_r_popscope;
    Dee_XClear(result);
    if (self->jl_context->jc_flags & JITCONTEXT_FSYNERR)
        goto err_popscope;
    continue;
   }
   /* Restore the old special return branch, if it was set. */
   if (self->jl_context->jc_retval &&
       JITCONTEXT_RETVAL_ISSET(self->jl_context->jc_retval)) {
    Dee_Decref(self->jl_context->jc_retval);
    self->jl_context->jc_retval = old_return_expr; /* Inherit reference. */
   } else if (old_return_expr && JITCONTEXT_RETVAL_ISSET(old_return_expr)) {
    self->jl_context->jc_retval = old_return_expr; /* Inherit reference. */
   } else {
    /* Don't restore special return branches, which finally is allowed
     * to re-direct (such as `continue' being turned into `break' through
     * use of a finally statement) */
   }
  } else if (JITLexer_ISTOK(self,"catch")) {
   /* Skip catch statements. */
   JITLexer_Yield(self);
   if likely(self->jl_tok == '(') {
    JITLexer_Yield(self);
   } else {
    SYNTAXERROR("Expected `(' after `catch', but got `%$s'",
               (size_t)(self->jl_tokend - self->jl_tokstart),
                self->jl_tokstart);
    goto err_r;
   }
   if (!result &&
        self->jl_context->jc_retval == JITCONTEXT_RETVAL_UNSET) {
    DREF DeeTypeObject *mask = NULL;
    char const *symbol_name = NULL;
    size_t symbol_size = 0;
    uint16_t old_except; DeeObject *current;
    DeeThreadObject *ts = DeeThread_Self();
    unsigned char *start;
    old_except = ts->t_exceptsz;
    current = DeeError_Current();
    ASSERT(current != NULL);
    JITContext_PushScope(self->jl_context);
#if 0
    /* TODO: Parse and initialize:
     *   - mask
     *   - symbol_name
     *   - symbol_size
     */
#else
    if (JITLexer_SkipPair(self,'(',')'))
        goto err_r;
#endif
    /* Check if we're allowed to handle this exception! */
    if ((!mask || DeeObject_InstanceOf(current,mask)) &&
        (allow_interrupts || !DeeObject_IsInterrupt(current))) {
     Dee_XDecref(mask);
     /* Store the current exception into a local symbol. */
     if (symbol_size) {
      JITObjectTable *tab;
      tab = JITContext_GetRWLocals(self->jl_context);
      if unlikely(!tab) goto err_r_popscope;
      if (JITObjectTable_Update(tab,
                                symbol_name,
                                symbol_size,
                                Dee_HashUtf8(symbol_name,symbol_size),
                                current,
                                true) < 0)
          goto err_r_popscope;
     }
     /* Override the previous return expression with the new one. */
     Dee_XDecref(result);
     start = self->jl_tokstart;
     result = EVAL_SECONDARY(self,&was_expression);
     if (result == JIT_LVALUE)
         result = JITLexer_PackLValue(self);
     if unlikely(!result) {
      if (self->jl_context->jc_flags & JITCONTEXT_FSYNERR)
          goto err_popscope;
      goto err_handle_catch_except;
     }
     /* The exception was handled normally. */
     DeeError_Handled(ERROR_HANDLED_INTERRUPT);
     JITContext_PopScope(self->jl_context);
     continue;
err_handle_catch_except:
     JITContext_PopScope(self->jl_context);
     JITLexer_YieldAt(self,start);
     if (SKIP_SECONDARY(self,&was_expression))
         goto err;
     /* Must still handle the original exception. */
     ASSERT(ts->t_exceptsz >= old_except);
     if (ts->t_exceptsz == old_except) {
      ASSERT(ts->t_except);
      ASSERT(ts->t_except->ef_error == current);
      continue; /* The previous exception was re-thrown */
     }
     /* A new exception was thrown on-top of ours. (we must still handle our old one) */
     {
      uint16_t ind = ts->t_exceptsz - old_except;
      struct except_frame *exc,**pexc;
      exc = *(pexc = &ts->t_except);
      while (ind--) {
       ASSERT(exc);
       ASSERT(exc->ef_prev);
       exc = *(pexc = &exc->ef_prev);
      }
      *pexc = exc->ef_prev;
      --ts->t_exceptsz;
      /* Destroy the frame in question. */
      if (ITER_ISOK(exc->ef_trace))
          Dee_Decref((DeeObject *)exc->ef_trace);
      Dee_Decref(exc->ef_error);
      except_frame_free(exc);
     }
     continue;
    } else {
     /* Don't execute this handler. */
     JITContext_PopScope(self->jl_context);
     Dee_Decref(mask);
     if (SKIP_SECONDARY(self,&was_expression))
         goto err_r;
    }
   } else {
    if (JITLexer_SkipPair(self,'(',')'))
        goto err_r;
    if (SKIP_SECONDARY(self,&was_expression))
        goto err_r;
   }
  } else break;
 }
#else
 ASSERT(JITLexer_ISKWD(self,"try"));
 JITLexer_Yield(self);
 result = SKIP_PRIMARY(self,&was_expression);
 if (ISERR(result)) goto err;
 while (self->jl_tok == JIT_KEYWORD) {
  if (JITLexer_ISTOK(self,"finally")) {
   JITLexer_Yield(self);
   result = SKIP_SECONDARY(self,&was_expression);
   if (ISERR(result)) goto err;
  } else if (JITLexer_ISTOK(self,"catch")) {
   JITLexer_Yield(self);
   if likely(self->jl_tok == '(') {
    JITLexer_Yield(self);
   } else {
    SYNTAXERROR("Expected `(' after `catch', but got `%$s'",
               (size_t)(self->jl_tokend - self->jl_tokstart),
                self->jl_tokstart);
    goto err;
   }
   if (JITLexer_SkipPair(self,'(',')'))
       goto err;
   result = SKIP_SECONDARY(self,&was_expression);
   if (ISERR(result)) goto err;
  } else break;
 }
#endif
 IF_HYBRID(if (*pwas_expression) *pwas_expression = was_expression;)
 return result;
#ifdef JIT_EVAL
err_popscope:
 JITContext_PopScope(self->jl_context);
 goto err;
err_r_popscope:
 JITContext_PopScope(self->jl_context);
err_r:
 Dee_XDecref(result);
#endif
err:
 return ERROR;
}


INTERN RETURN_TYPE FCALL
H_FUNC(If)(JITLexer *__restrict self, JIT_ARGS) {
 RETURN_TYPE result;
 ASSERT(JITLexer_ISKWD(self,"if"));
do_if_statement:
 JITLexer_Yield(self);
 if likely(self->jl_tok == '(') {
  JITLexer_Yield(self);
 } else {
  SYNTAXERROR("Expected `(' after `if', but got `%$s'",
             (size_t)(self->jl_tokend - self->jl_tokstart),
              self->jl_tokstart);
  goto err;
 }
#ifdef JIT_EVAL
 result = JITLexer_EvalComma(self,
                             AST_COMMA_NORMAL|
                             AST_COMMA_ALLOWVARDECLS,
                             NULL,
                             NULL);
 if (ISERR(result)) goto err;
 LOAD_LVALUE(result,err);
 if likely(self->jl_tok == ')') {
  JITLexer_Yield(self);
 } else {
  SYNTAXERROR("Expected `)' after `if', but got `%$s'",
             (size_t)(self->jl_tokend - self->jl_tokstart),
              self->jl_tokstart);
  goto err_r;
 }
#else
 if (JITLexer_SkipPair(self,'(',')'))
     goto err;
 result = 0;
#endif
 {
#ifdef JIT_EVAL
  int b = DeeObject_Bool(result);
  if unlikely(b < 0) goto err_r;
  Dee_Decref(result);
  if (b) {
   result = EVAL_PRIMARY(self,pwas_expression);
   if unlikely(!result) goto err;
   if (self->jl_tok == JIT_KEYWORD) {
    if (JITLexer_ISTOK(self,"elif")) {
     self->jl_tokstart += 2;
     goto do_else_branch;
    }
    if (JITLexer_ISTOK(self,"else")) {
     JITLexer_Yield(self);
do_else_branch:
     if (SKIP_SECONDARY(self,pwas_expression))
         goto err_r;
    }
   }
  } else
#endif
  {
   if (SKIP_SECONDARY(self,pwas_expression))
       goto err;
   if (self->jl_tok == JIT_KEYWORD) {
    if (JITLexer_ISTOK(self,"elif"))
        goto do_if_statement;
    if (JITLexer_ISTOK(self,"else")) {
     JITLexer_Yield(self);
     result = EVAL_SECONDARY(self,pwas_expression);
     /*if (ISERR(result)) goto err;*/
    }
   }
  }
 }
 return result;
#ifdef JIT_EVAL
err_r:
 DECREF(result);
#endif /* JIT_EVAL */
err:
 return ERROR;
}

INTERN RETURN_TYPE FCALL
H_FUNC(Del)(JITLexer *__restrict self, JIT_ARGS) {
 ASSERT(JITLexer_ISKWD(self,"del"));
#ifdef JIT_HYBRID
 (void)pwas_expression;
#else
 (void)is_statement;
#endif
 DERROR_NOTIMPLEMENTED();
 return ERROR;
}

INTERN RETURN_TYPE FCALL
H_FUNC(For)(JITLexer *__restrict self, JIT_ARGS) {
 ASSERT(JITLexer_ISKWD(self,"for"));
#ifdef JIT_HYBRID
 (void)pwas_expression;
#else
 (void)is_statement;
#endif
 DERROR_NOTIMPLEMENTED();
 return ERROR;
}

INTERN RETURN_TYPE FCALL
H_FUNC(Foreach)(JITLexer *__restrict self, JIT_ARGS) {
 ASSERT(JITLexer_ISKWD(self,"foreach"));
#ifdef JIT_HYBRID
 (void)pwas_expression;
#else
 (void)is_statement;
#endif
 DERROR_NOTIMPLEMENTED();
 return ERROR;
}

INTERN RETURN_TYPE FCALL
H_FUNC(While)(JITLexer *__restrict self, JIT_ARGS) {
 ASSERT(JITLexer_ISKWD(self,"while"));
#ifdef JIT_HYBRID
 (void)pwas_expression;
#else
 (void)is_statement;
#endif
 DERROR_NOTIMPLEMENTED();
 return ERROR;
}

INTERN RETURN_TYPE FCALL
H_FUNC(Do)(JITLexer *__restrict self, JIT_ARGS) {
 ASSERT(JITLexer_ISKWD(self,"do"));
#ifdef JIT_HYBRID
 (void)pwas_expression;
#else
 (void)is_statement;
#endif
 DERROR_NOTIMPLEMENTED();
 return ERROR;
}

INTERN RETURN_TYPE FCALL
H_FUNC(With)(JITLexer *__restrict self, JIT_ARGS) {
 ASSERT(JITLexer_ISKWD(self,"with"));
#ifdef JIT_HYBRID
 (void)pwas_expression;
#else
 (void)is_statement;
#endif
 DERROR_NOTIMPLEMENTED();
 return ERROR;
}

INTERN RETURN_TYPE FCALL
H_FUNC(Assert)(JITLexer *__restrict self, JIT_ARGS) {
 ASSERT(JITLexer_ISKWD(self,"assert"));
#ifdef JIT_HYBRID
 (void)pwas_expression;
#else
 (void)is_statement;
#endif
 DERROR_NOTIMPLEMENTED();
 return ERROR;
}

#ifdef JIT_HYBRID
INTERN RETURN_TYPE FCALL
H_FUNC(Import)(JITLexer *__restrict self, bool is_from_import, JIT_ARGS)
#else
INTERN RETURN_TYPE FCALL
H_FUNC(Import)(JITLexer *__restrict self, bool is_from_import)
#endif
{
 RETURN_TYPE result;
 ASSERT(is_from_import
      ? JITLexer_ISKWD(self,"from")
      : JITLexer_ISKWD(self,"import"));
#ifdef JIT_HYBRID
 if (!is_from_import) {
  *pwas_expression = AST_PARSE_WASEXPR_YES;
#ifdef JIT_EVAL
  result = DeeObject_GetAttrString((DeeObject *)DeeModule_GetDeemon(),"import");
#else /* JIT_EVAL */
  result = 0;
#endif /* !JIT_EVAL */
  return result;
 }
#endif

 (void)is_from_import;
#ifdef JIT_HYBRID
 (void)pwas_expression;
#endif
 DERROR_NOTIMPLEMENTED();
 result = ERROR;
 return result;
}


DECL_END

#undef JIT_ARGS
#undef EVAL_PRIMARY
#undef EVAL_SECONDARY
#undef SKIP_PRIMARY
#undef SKIP_SECONDARY
#undef IF_HYBRID
#undef IF_NHYBRID
#undef H_FUNC
#undef JIT_HYBRID
