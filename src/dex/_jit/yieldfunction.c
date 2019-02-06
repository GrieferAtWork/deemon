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
#ifndef GUARD_DEX_JIT_YIELDFUNCTION_C
#define GUARD_DEX_JIT_YIELDFUNCTION_C 1

#include "libjit.h"
#include <deemon/arg.h>
#include <deemon/seq.h>
#include <deemon/gc.h>
#include <deemon/none.h>
#include <deemon/alloc.h>
#include <deemon/error.h>
#include <deemon/thread.h>
#include <deemon/object.h>
#include <hybrid/unaligned.h>

DECL_BEGIN

typedef JITFunctionObject JITFunction;
typedef JITYieldFunctionObject JITYieldFunction;
typedef JITYieldFunctionIteratorObject JITYieldFunctionIterator;

#ifdef CONFIG_LITTLE_ENDIAN
#define ENCODE2(a,b)     ((b)<<8|(a))
#define ENCODE4(a,b,c,d) ((d)<<24|(c)<<16|(b)<<8|(a))
#else
#define ENCODE2(a,b)     ((b)|(a)<<8)
#define ENCODE4(a,b,c,d) ((d)|(c)<<8|(b)<<16|(a)<<24)
#endif


INTERN void DCALL
jit_state_fini(struct jit_state *__restrict self) {
 switch (self->js_kind) {
 case JIT_STATE_KIND_FOREACH:
  Dee_Decref(self->js_foreach.f_iter);
  JITLValue_Fini(&self->js_foreach.f_elem);
  break;
 case JIT_STATE_KIND_FOREACH2:
  Dee_Decref(self->js_foreach2.f_iter);
  JITLValueList_Fini(&self->js_foreach2.f_elem);
  break;
 default:
  break;
 }
}


PRIVATE void DCALL
jy_fini(JITYieldFunction *__restrict self) {
 size_t i;
 for (i = 0; i < self->jy_argc; ++i)
     Dee_Decref(self->jy_argv[i]);
 Dee_XDecref(self->jy_kw);
 Dee_Decref(self->jy_func);
}

PRIVATE void DCALL
jy_visit(JITYieldFunction *__restrict self, dvisit_t proc, void *arg) {
 size_t i;
 for (i = 0; i < self->jy_argc; ++i)
     Dee_Visit(self->jy_argv[i]);
 Dee_XVisit(self->jy_kw);
 Dee_Visit(self->jy_func);
}

PRIVATE DREF JITYieldFunctionIterator *DCALL
jy_iter(JITYieldFunction *__restrict self) {
 DREF JITYieldFunctionIterator *result;
 JITFunction *jf = self->jy_func; size_t i;
 result = DeeGCObject_MALLOC(JITYieldFunctionIterator);
 if unlikely(!result)
    goto done;

 ASSERT(jf->jf_args.ot_prev.otp_ind >= 2);
 ASSERT(jf->jf_args.ot_prev.otp_tab == &jf->jf_refs);
 memcpy(&result->ji_loc,&jf->jf_args,sizeof(JITObjectTable));
 ASSERT(result->ji_loc.ot_prev.otp_tab != NULL);
 result->ji_loc.ot_list = (struct jit_object_entry *)Dee_Malloc((result->ji_loc.ot_mask + 1) *
                                                                 sizeof(struct jit_object_entry));
 if unlikely(!result->ji_loc.ot_list) goto err_r;
 memcpy(result->ji_loc.ot_list,jf->jf_args.ot_list,
       (result->ji_loc.ot_mask + 1) * sizeof(struct jit_object_entry));

 /* Define the self-argument.
  * NOTE: Do this before loading arguments, in case one of the arguments
  *       uses the same name as the function, in which case that argument
  *       must be loaded, rather than the function loading itself! */
 if (jf->jf_selfarg != (size_t)-1) {
  ASSERT(result->ji_loc.ot_list[jf->jf_selfarg].oe_value == NULL);
  result->ji_loc.ot_list[jf->jf_selfarg].oe_value = (DREF DeeObject *)self;
 }

 if (self->jy_kw) {
  /* TODO: Load arguments! */
  (void)self->jy_argc;
  (void)self->jy_argv;
  (void)self->jy_kw;
  /* Assign references to all objects from base-locals */
  for (i = 0; i <= result->ji_loc.ot_mask; ++i) {
   if (!ITER_ISOK(result->ji_loc.ot_list[i].oe_namestr))
        continue;
   Dee_XIncref(result->ji_loc.ot_list[i].oe_value);
  }
 } else {
  size_t i;
  /* Load arguments! */
  if (self->jy_argc < jf->jf_argc_min)
      goto err_argc;
  if (self->jy_argc > jf->jf_argc_max && jf->jf_varargs == (size_t)-1)
      goto err_argc;
  /* Load positional arguments. */
  for (i = 0; i < self->jy_argc; ++i)
      result->ji_loc.ot_list[jf->jf_argv[i]].oe_value = self->jy_argv[i];
  /* Assign references to all objects from base-locals */
  for (i = 0; i <= result->ji_loc.ot_mask; ++i) {
   if (!ITER_ISOK(result->ji_loc.ot_list[i].oe_namestr))
        continue;
   Dee_XIncref(result->ji_loc.ot_list[i].oe_value);
  }
 }

 result->ji_func = self;
 Dee_Incref(self);
 recursive_rwlock_init(&result->ji_lock);
 result->ji_lex.jl_text = jf->jf_source;
 result->ji_lex.jl_context = &result->ji_ctx;
 JITLValue_Init(&result->ji_lex.jl_lvalue);
 result->ji_ctx.jc_impbase        = jf->jf_impbase;
 result->ji_ctx.jc_globals        = jf->jf_globals;
 result->ji_ctx.jc_retval         = JITCONTEXT_RETVAL_UNSET;
 result->ji_ctx.jc_locals.otp_ind = 1;
 result->ji_ctx.jc_locals.otp_tab = &result->ji_loc;
 /* Initialize the base-compiler-context-state as a block-scope. */
 result->ji_state = &result->ji_bstat;
 result->ji_bstat.js_prev = NULL;
 result->ji_bstat.js_kind = JIT_STATE_KIND_SCOPE;
 result->ji_bstat.js_flag = JIT_STATE_FLAG_BLOCK;
 JITLexer_Start(&result->ji_lex,
               (unsigned char *)jf->jf_source_start,
               (unsigned char *)jf->jf_source_end);
 DeeObject_Init(result,&JITYieldFunctionIterator_Type);
 DeeGC_Track((DeeObject *)result);
done:
 return result;
err_r_loc:
 Dee_Free(result->ji_loc.ot_list);
err_r:
 DeeGCObject_FREE(result);
 return NULL;
err_argc:
 if (jf->jf_selfarg == (size_t)-1) {
  err_invalid_argc_len(NULL,
                       0,
                       self->jy_argc,
                       jf->jf_argc_min,
                       jf->jf_argc_max);
 } else {
  struct jit_object_entry *ent;
  ent = &jf->jf_args.ot_list[jf->jf_selfarg];
  err_invalid_argc_len((char const *)ent->oe_namestr,
                        ent->oe_namelen,
                        self->jy_argc,
                        jf->jf_argc_min,
                        jf->jf_argc_max);
 }
 goto err_r_loc;
}


PRIVATE struct type_seq jy_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jy_iter,
    /* .tp_size      = */NULL,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */NULL,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL
};


PRIVATE struct type_getset jy_getsets[] = {
    { NULL }
};

PRIVATE struct type_member jy_members[] = {
    TYPE_MEMBER_FIELD_DOC("__func__",STRUCT_OBJECT,offsetof(JITYieldFunction,jy_func),"->?GFunction"),
    TYPE_MEMBER_END
};

PRIVATE struct type_member jy_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&JITYieldFunctionIterator_Type),
    TYPE_MEMBER_END
};


INTERN DeeTypeObject JITYieldFunction_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_JitYieldFunction",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL | TP_FVARIABLE,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_var = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&jy_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void *))&jy_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&jy_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */jy_getsets,
    /* .tp_members       = */jy_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */jy_class_members,
    /* .tp_call_kw       = */NULL
};


/* @return:  1: The state wasn't removed, but the lexer position was updated
 *              such that execution should resume normally (this is used to
 *              implement loop statements)
 * @return:  0: The state was successfully removed
 * @return: -1: An error occurred. */
PRIVATE int DCALL
JITYieldFunctionIterator_PopState(JITYieldFunctionIterator *__restrict self) {
 struct jit_state *st = self->ji_state;
 unsigned char *old_pos;
 assert(st);
 assert(st != &self->ji_bstat);
 switch (st->js_kind) {

 case JIT_STATE_KIND_SCOPE2:
  JITContext_PopScope(&self->ji_ctx);
  break;

 case JIT_STATE_KIND_FOR:
  old_pos = self->ji_lex.jl_tokstart;
  if (st->js_for.f_next) {
   DREF DeeObject *temp;
   JITLexer_YieldAt(&self->ji_lex,st->js_for.f_next);
   /* Invoke the next-expression. */
   temp = JITLexer_EvalExpression(&self->ji_lex,
                                    JITLEXER_EVAL_FNORMAL);
   if unlikely(!temp)
      goto err;
   if (temp == JIT_LVALUE) {
    JITLValue_Fini(&self->ji_lex.jl_lvalue);
    JITLValue_Init(&self->ji_lex.jl_lvalue);
   } else {
    Dee_Decref(temp);
   }
  }
  if (st->js_for.f_cond) {
   DREF DeeObject *value; int temp;
   JITLexer_YieldAt(&self->ji_lex,st->js_for.f_cond);
   /* Invoke the cond-expression. */
   value = JITLexer_EvalRValue(&self->ji_lex);
   if unlikely(!value)
      goto err;
   /* Check if loop iteration should continue. */
   temp = DeeObject_Bool(value);
   Dee_Decref(value);
   if unlikely(temp < 0)
      goto err;
   if (!temp) {
    /* Restore the old position (which is presumably just after the loop block) */
    JITLexer_YieldAt(&self->ji_lex,old_pos);
    /* Don't jump back, but break out of the loop. */
    goto do_pop_state;
   }
  }
  /* Check for interrupts before jumping backwards */
  if (DeeThread_CheckInterrupt())
      goto err;
  /* Resume execution of the loop. */
  JITLexer_YieldAt(&self->ji_lex,st->js_for.f_loop);
  return 1;

 case JIT_STATE_KIND_FOREACH:
  /* TODO */
  DERROR_NOTIMPLEMENTED();
  goto err;

 case JIT_STATE_KIND_FOREACH2:
  /* TODO */
  DERROR_NOTIMPLEMENTED();
  goto err;

 case JIT_STATE_KIND_SKIPELSE:
  /* Check for `else' and `elif' */
  if (self->ji_lex.jl_tok == JIT_KEYWORD &&
      self->ji_lex.jl_tokend == self->ji_lex.jl_tokstart + 4) {
   uint32_t name;
   name = UNALIGNED_GET32((uint32_t *)self->ji_lex.jl_tokstart);
   if (name == ENCODE4('e','l','s','e')) {
    JITLexer_Yield(&self->ji_lex);
do_skip_else:
    /* Skip the accompanying block. */
    if unlikely(JITLexer_SkipStatement(&self->ji_lex))
       goto err;
   } else if (name == ENCODE4('e','l','i','f')) {
    self->ji_lex.jl_tokstart += 2; /* Transform into an `if' */
    goto do_skip_else;
   }
  }
  break;

 default: break;
 }
do_pop_state:
 /* Default: Remove the state. */
 self->ji_state = st->js_prev;
 jit_state_destroy(st);
 return 0;
err:
 return -1;
}


/* Handle a break/continue loop control command by searching for
 * the nearest JIT state that behaves as a loop controller.
 * @return:  1: The lexer state was updated, the state was unwound,
 *              and the loop control command was handled successfully.
 * @return:  0: Failed to locate a valid receiver for the loop control command.
 * @return: -1: An error occurred. */
PRIVATE int DCALL
JITYieldFunctionIterator_HandleLoopctl(JITYieldFunctionIterator *__restrict self,
                                       bool ctl_is_break) {
 /* TODO: Search for the nearest state that can handle the loop control command,
  *       then proceed to unwind the state stack up to that state and load the
  *       lexer position to which the jump should be performed.
  *       Once this is done, return `1' (but remember that in the case of `break',
  *       the loop state entry itself must also be removed, as the target location
  *       exists after the loop itself, meaning that the loop context will have
  *       ended at that point)
  */
 (void)self;
 (void)ctl_is_break;
 DERROR_NOTIMPLEMENTED();
 return -1;
}



PRIVATE DREF DeeObject *DCALL
ji_next(JITYieldFunctionIterator *__restrict self) {
 DREF DeeObject *result; int error;
 DeeThreadObject *ts = DeeThread_Self();
 recursive_rwlock_write(&self->ji_lock);
 self->ji_ctx.jc_except = ts->t_exceptsz;
 self->ji_ctx.jc_flags  = JITCONTEXT_FNORMAL;

 /* TODO: Just parser and context stack for yield function invocation.
  *    -> The context stack here must be allocated on the heap and be
  *       representative of how and when scopes need to be torn down. */
parse_again:
 /* Pop all single-statement states, allowing for loop blocks to re-evaluate
  * their context and potentially jump back to continue execution at a prior
  * point of execution. */
 while (self->ji_state->js_flag & JIT_STATE_FLAG_SINGLE) {
  error = JITYieldFunctionIterator_PopState(self);
  if unlikely(error < 0)
     goto err;
  if (error)
      break;
 }
parse_again_same_statement:
 switch (self->ji_lex.jl_tok) {

 case TOK_EOF:
  /* The iterator has been exhausted! */
  result = ITER_DONE;
  goto done;

 {
  struct jit_state *st;
 case '{':
  st = self->ji_state;
  /* Transform the current state to become block-scopes. */
  if (st->js_flag & JIT_STATE_FLAG_SINGLE)
   st->js_flag &= ~JIT_STATE_FLAG_SINGLE;
  else {
   /* Recursively defined block scope. */
   st = jit_state_alloc();
   if unlikely(!st)
      goto err;
   st->js_prev = self->ji_state;
   st->js_kind = JIT_STATE_KIND_SCOPE;
   st->js_flag = JIT_STATE_FLAG_BLOCK;
   self->ji_state = st;
  }
  JITLexer_Yield(&self->ji_lex);
  /* Push an additional block-scope */
  JITContext_PushScope(&self->ji_ctx);
  goto parse_again_same_statement;
 }

 case '}':
  if (self->ji_state->js_flag & JIT_STATE_FLAG_SINGLE) {
   DeeError_Throwf(&DeeError_SyntaxError,
                   "Expected statement before `}' within `yield'-function");
   self->ji_lex.jl_errpos = self->ji_lex.jl_tokstart;
   goto err;
  }
  if (self->ji_state == &self->ji_bstat) {
   DeeError_Throwf(&DeeError_SyntaxError,
                   "Unmatched `}' encountered within `yield'-function");
   self->ji_lex.jl_errpos = self->ji_lex.jl_tokstart;
   goto err;
  }
  /* Pop the additional block scope. */
  JITContext_PopScope(&self->ji_ctx);
  error = JITYieldFunctionIterator_PopState(self);
  if unlikely(error < 0)
     goto err;
  if (error)
      goto parse_again_same_statement;
  JITLexer_Yield(&self->ji_lex);
  goto parse_again;

 {
  char const *tok_begin;
  size_t tok_length;
  uint32_t name;
 case JIT_KEYWORD:
  tok_begin  = (char *)self->ji_lex.jl_tokstart;
  tok_length = (size_t)((char *)self->ji_lex.jl_tokend - tok_begin);
  switch (tok_length) {

  case 2:
   if (tok_begin[0] == 'i' &&
       tok_begin[1] == 'f') {
    DREF DeeObject *value; int temp;
    JITLexer_Yield(&self->ji_lex);
    if likely(self->ji_lex.jl_tok == '(') {
     JITLexer_Yield(&self->ji_lex);
    } else {
     DeeError_Throwf(&DeeError_SyntaxError,
                     "Expected `(' after `if', but got `%$s'",
                     (size_t)(self->ji_lex.jl_tokend - self->ji_lex.jl_tokstart),
                      self->ji_lex.jl_tokstart);
     goto err;
    }
    value = JITLexer_EvalRValue(&self->ji_lex);
    if unlikely(!value)
       goto err;
    temp = DeeObject_Bool(value);
    Dee_Decref(value);
    if unlikely(temp < 0)
       goto err;
    if likely(self->ji_lex.jl_tok == ')') {
     JITLexer_Yield(&self->ji_lex);
    } else {
     DeeError_Throwf(&DeeError_SyntaxError,
                     "Expected `)' after `if', but got `%$s'",
                     (size_t)(self->ji_lex.jl_tokend - self->ji_lex.jl_tokstart),
                      self->ji_lex.jl_tokstart);
     goto err;
    }
    if (temp) {
     /* Choose the true-branch (push a skip-else state). */
     struct jit_state *st;
     st = jit_state_alloc();
     if unlikely(!st)
        goto err;
     st->js_kind = JIT_STATE_KIND_SKIPELSE;
     st->js_flag = JIT_STATE_FLAG_SINGLE;
     st->js_prev = self->ji_state;
     self->ji_state = st;
     goto parse_again_same_statement;
    }
    /* Choose the false-branch (should it exist) */
    if unlikely(JITLexer_SkipStatement(&self->ji_lex))
       goto err;
    if (self->ji_lex.jl_tok == JIT_KEYWORD &&
        self->ji_lex.jl_tokend == self->ji_lex.jl_tokstart + 4) {
     uint32_t name;
     name = UNALIGNED_GET32((uint32_t *)self->ji_lex.jl_tokstart);
     if (name == ENCODE4('e','l','s','e')) {
      JITLexer_Yield(&self->ji_lex);
      goto parse_again_same_statement;
     } else if (name == ENCODE4('e','l','i','f')) {
      self->ji_lex.jl_tokstart += 2; /* Transform into an `if' */
      goto parse_again_same_statement;
     }
    }
    /* No live branch existed within the if-statement (move on to execute the next statement) */
    goto parse_again;
   }
   break;

  case 5:
   name = UNALIGNED_GET32((uint32_t *)tok_begin);
   if (name == ENCODE4('y','i','e','l') &&
       *(uint8_t *)(tok_begin + 4) == 'd') {
    /* The thing that we're actually after: `yield' statements! */
    JITLexer_Yield(&self->ji_lex);
    result = JITLexer_EvalRValue(&self->ji_lex);
    /* Consume the trailing `;' that is required for yield statements. */
    if likely(self->ji_lex.jl_tok == ';') {
     JITLexer_Yield(&self->ji_lex);
    } else {
     DeeError_Throwf(&DeeError_SyntaxError,
                     "Expected `;' after `throw', but got `%$s'",
                    (size_t)(self->ji_lex.jl_tokend - self->ji_lex.jl_tokstart),
                     self->ji_lex.jl_tokstart);
     goto err_r;
    }
    goto got_yield_value;
   }
   break;

  default: break;
  }
  goto parse_generic_statement;
 }

 default:
parse_generic_statement:
  /* Fallback: Parse a regular statement. */
  result = JITLexer_EvalStatement(&self->ji_lex);
  if unlikely(!result)
     goto err;
  if (result == JIT_LVALUE) {
   JITLValue_Fini(&self->ji_lex.jl_lvalue);
   self->ji_lex.jl_lvalue.lv_kind = JIT_LVALUE_NONE;
  } else {
   Dee_Decref(result);
  }
  goto parse_again;
 }
got_yield_value:

 ASSERT(result != JIT_LVALUE);
 if (!result) {
err:
  if (self->ji_ctx.jc_retval != JITCONTEXT_RETVAL_UNSET) {
   if (self->ji_ctx.jc_retval == JITCONTEXT_RETVAL_BREAK ||
       self->ji_ctx.jc_retval == JITCONTEXT_RETVAL_CONTINUE) {
    bool is_break = self->ji_ctx.jc_retval == JITCONTEXT_RETVAL_BREAK;
    /* Try to service the break/continue control command using
     * the currently active compiler context state stack. */
    error = JITYieldFunctionIterator_HandleLoopctl(self,is_break);
    if unlikely(error < 0)
       goto err;
    if (error) {/* Resume execution at the location that was unwound */
     self->ji_ctx.jc_retval = JITCONTEXT_RETVAL_UNSET;
     goto parse_again_same_statement;
    }
   }
   if (JITCONTEXT_RETVAL_ISSET(self->ji_ctx.jc_retval)) {
    /* Deal with use of the return statement. */
    result = self->ji_ctx.jc_retval;
    self->ji_ctx.jc_retval = JITCONTEXT_RETVAL_UNSET;
    if (DeeNone_Check(result)) {
     /* `return;' on its own causes `return none',
      * which we must interpret as stop-iteration. */
     Dee_Decref(result);
     result = ITER_DONE;
    } else {
     Dee_Decref(result);
     DeeError_Throwf(&DeeError_SyntaxError,
                     "`return' statement encountered within `yield'-function");
     self->ji_lex.jl_errpos = self->ji_lex.jl_tokstart;
     goto handle_error;
    }
   } else {
    /* Exited code via unconventional means, such as `break' or `continue' */
    DeeError_Throwf(&DeeError_SyntaxError,
                    "Attempted to use `break' or `continue' outside of a loop");
    self->ji_lex.jl_errpos = self->ji_lex.jl_tokstart;
    goto handle_error;
   }
  } else {
   if (!self->ji_lex.jl_errpos)
        self->ji_lex.jl_errpos = self->ji_lex.jl_tokstart;
handle_error:
   JITLValue_Fini(&self->ji_lex.jl_lvalue);
   self->ji_lex.jl_lvalue.lv_kind = JIT_LVALUE_NONE;
   result = NULL;
   /* TODO: Somehow remember that the error happened at `lexer.jl_errpos' */
   ;
  }
  self->ji_lex.jl_tok = TOK_EOF; /* Don't iterate again. */
 }
 if (ts->t_exceptsz > self->ji_ctx.jc_except) {
  if (self->ji_ctx.jc_retval != JITCONTEXT_RETVAL_UNSET) {
   if (JITCONTEXT_RETVAL_ISSET(self->ji_ctx.jc_retval))
       Dee_Decref(self->ji_ctx.jc_retval);
   self->ji_ctx.jc_retval = JITCONTEXT_RETVAL_UNSET;
  }
  if (ITER_ISOK(result))
      Dee_Decref(result);
  result = NULL;
  while (ts->t_exceptsz > self->ji_ctx.jc_except + 1) {
   DeeError_Print("Discarding secondary error\n",
                  ERROR_PRINT_DOHANDLE);
  }
 }
done:
 recursive_rwlock_endwrite(&self->ji_lock);
 return result;
err_r:
 Dee_Decref(result);
 goto err;
}



PRIVATE void DCALL
ji_fini(JITYieldFunctionIterator *__restrict self) {
 JITFunctionObject *jf = self->ji_func->jy_func;
 ASSERT(!jf->jf_globals || self->ji_ctx.jc_globals == jf->jf_globals);
 while (self->ji_state != &self->ji_bstat)
     jit_state_destroy(self->ji_state);
 /* Destroy any remaining L-value expressions. */
 JITLValue_Fini(&self->ji_lex.jl_lvalue);
 /* Pop remaining scopes. */
 while (self->ji_ctx.jc_locals.otp_tab != &self->ji_loc) {
  ASSERT(self->ji_ctx.jc_locals.otp_tab != NULL);
  self->ji_ctx.jc_locals.otp_ind = 0;
  _JITContext_PopLocals(&self->ji_ctx);
 }
 /* Cleanup custom globals (if those had been accessed at one point) */
 if (self->ji_ctx.jc_globals != jf->jf_globals)
     Dee_Decref(self->ji_ctx.jc_globals);
 /* Destroy the function's base-scope. */
 JITObjectTable_Fini(&self->ji_loc);
 /* Drop our reference to the associated function. */
 Dee_Decref(self->ji_func);
}

PRIVATE void DCALL
ji_visit(JITYieldFunctionIterator *__restrict self, dvisit_t proc, void *arg) {
 JITFunctionObject *jf = self->ji_func->jy_func;
 JITObjectTable *tab;
 ASSERT(!jf->jf_globals || self->ji_ctx.jc_globals == jf->jf_globals);
 /* Destroy any remaining L-value expressions. */
 JITLValue_Visit(&self->ji_lex.jl_lvalue,proc,arg);
 /* Pop remaining scopes. */
 tab = self->ji_ctx.jc_locals.otp_tab;
 for (;;) {
  ASSERT(tab);
  JITObjectTable_Visit(tab,proc,arg);
  if (tab == &self->ji_loc)
      break;
  tab = tab->ot_prev.otp_tab;
 }
 if (self->ji_ctx.jc_globals != jf->jf_globals)
     Dee_Visit(self->ji_ctx.jc_globals);
 Dee_Visit(self->ji_func);
}

PRIVATE struct type_getset ji_getsets[] = {
    { NULL }
};

PRIVATE struct type_member ji_members[] = {
    TYPE_MEMBER_FIELD_DOC("seq",STRUCT_OBJECT,offsetof(JITYieldFunctionIterator,ji_func),"->?GYieldFunction"),
    TYPE_MEMBER_END
};


INTERN DeeTypeObject JITYieldFunctionIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_JitYieldFunctionIterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL | TP_FGC,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                TYPE_FIXED_ALLOCATOR_GC(JITYieldFunctionIterator)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&ji_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void *))&ji_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ji_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */ji_getsets,
    /* .tp_members       = */ji_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL,
    /* .tp_call_kw       = */NULL
};


DECL_END

#endif /* !GUARD_DEX_JIT_YIELDFUNCTION_C */
