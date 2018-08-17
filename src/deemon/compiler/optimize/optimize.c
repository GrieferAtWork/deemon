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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_OPTIMIZE_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_OPTIMIZE_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/none.h>
#include <deemon/arg.h>
#include <deemon/bytes.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/roset.h>
#include <deemon/rodict.h>
#include <deemon/bool.h>
#include <deemon/seq.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/string.h>
#include <deemon/cell.h>
#include <deemon/dict.h>
#include <deemon/dec.h>
#include <deemon/super.h>
#include <deemon/hashset.h>
#include <deemon/tuple.h>
#include <deemon/map.h>
#include <deemon/list.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/optimize.h>
#include <deemon/util/string.h>
#include <deemon/objmethod.h>
#include <deemon/module.h>
#include <deemon/numeric.h>
#include <deemon/callable.h>

#include <stdarg.h>
#include <string.h>

DECL_BEGIN

INTERN uint16_t optimizer_flags        = OPTIMIZE_FDISABLED;
INTERN uint16_t optimizer_unwind_limit = 0;
INTERN unsigned int optimizer_count    = 0;

#ifdef CONFIG_HAVE_OPTIMIZE_VERBOSE
INTERN void ast_optimize_verbose(DeeAstObject *__restrict self, char const *format, ...) {
 va_list args; va_start(args,format);
 if (self->ast_ddi.l_file) {
  DEE_DPRINTF("%s(%d,%d) : ",
              TPPFile_Filename(self->ast_ddi.l_file,NULL),
              self->ast_ddi.l_line+1,
              self->ast_ddi.l_col+1);
 }
 DEE_VDPRINTF(format,args);
 va_end(args);
}
#endif




INTERN int (DCALL ast_optimize)(struct ast_optimize_stack *parent,
                                DeeAstObject *__restrict self, bool result_used) {
 struct ast_optimize_stack stack;
 stack.os_prev = parent;
 stack.os_ast  = self;
 stack.os_used = result_used;
 ASSERT_OBJECT_TYPE(self,&DeeAst_Type);
again:
 /* Check for interrupts to allow the user to stop optimization */
 if (DeeThread_CheckInterrupt()) goto err;
 while (self->ast_scope->s_prev &&                    /* Has parent scope */
        self->ast_scope->s_prev->ob_type ==           /* Parent scope has the same type */
        self->ast_scope->ob_type &&                   /* ... */
        self->ast_scope->ob_type == &DeeScope_Type && /* It's a regular scope */
       !self->ast_scope->s_mapc &&                    /* Current scope has no symbols */
       !self->ast_scope->s_del) {                     /* Current scope has no deleted symbol */
  /* Use the parent's scope. */
  DeeScopeObject *new_scope;
  new_scope = self->ast_scope->s_prev;
  Dee_Incref(new_scope);
  Dee_Decref(self->ast_scope);
  self->ast_scope = new_scope;
  ++optimizer_count;
#if 0 /* This happens so often, logging it would just be spam... */
  OPTIMIZE_VERBOSEAT(self,"Inherit parent scope above empty child scope\n");
#endif
 }

 /* TODO: Move variables declared in outer scopes, but only used in inner ones
  *       into those inner scopes, thus improving local-variable-reuse optimizations
  *       later done the line. */
 switch (self->ast_type) {

 case AST_SYM:
  return ast_optimize_symbol(&stack,self,result_used);

 case AST_MULTIPLE:
  return ast_optimize_multiple(&stack,self,result_used);

 case AST_RETURN:
 case AST_YIELD:
 case AST_THROW:
  if (self->ast_returnexpr &&
      ast_optimize(&stack,self->ast_returnexpr,true))
      goto err;
  break;

 {
  struct catch_expr *iter,*end;
 case AST_TRY:
  if (ast_optimize(&stack,self->ast_try.ast_guard,result_used))
      goto err;
  if (ast_is_nothrow(self->ast_try.ast_guard,result_used)) {
   /* TODO: `try { foo; } finally { bar; }' -> `({ __stack local _r = foo; bar; _r; })'
    * TODO: `try { foo; } catch (...) { bar; }' -> `(foo)' */
  }
  end = (iter = self->ast_try.ast_catchv)+self->ast_try.ast_catchc;
  for (; iter != end; ++iter) {
   if (ast_optimize(&stack,iter->ce_code,result_used)) goto err;
   if (iter->ce_mask &&
       ast_optimize(&stack,iter->ce_mask,!(iter->ce_flags & EXCEPTION_HANDLER_FFINALLY))) goto err;
   if (!(iter->ce_flags & EXCEPTION_HANDLER_FFINALLY)) {
    /* `catch (object)' --> `catch (...)' */
    if (iter->ce_mask &&
        iter->ce_mask->ast_type == AST_CONSTEXPR &&
        iter->ce_mask->ast_constexpr == (DeeObject *)&DeeObject_Type) {
     OPTIMIZE_VERBOSE("Optimize object-mask to catch-all\n");
     ++optimizer_count;
     Dee_Clear(iter->ce_mask);
    }
    /* Set the `CATCH_EXPR_FSECOND' flag for all noexcept catch-handlers. */
    if (!(iter->ce_mode & CATCH_EXPR_FSECOND) &&
          ast_is_nothrow(iter->ce_code,result_used) &&
       (!iter->ce_mask || ast_is_nothrow(iter->ce_mask,true))) {
     OPTIMIZE_VERBOSE("Optimize nothrow catch-handler\n");
     iter->ce_mode |= CATCH_EXPR_FSECOND;
    }
   }
  }
 } break;

 case AST_LOOP:
  if (self->ast_loop.ast_loop &&
      ast_optimize(&stack,self->ast_loop.ast_loop,false)) goto err;
  if (self->ast_flag&AST_FLOOP_FOREACH) {
   /* foreach-style loop. */
   if (ast_optimize(&stack,self->ast_loop.ast_iter,true)) goto err;
   if (optimizer_unwind_limit != 0) {
    /* TODO: Loop unwinding when `self->ast_loop.ast_iter'
     *       evaluates to a constant expression.
     * >> for (local x: [:3])
     * >>      print x;
     * Optimize to:
     * >> print 0;
     * >> print 1;
     * >> print 2;
     * Optimize to:
     * >> print "0";
     * >> print "1";
     * >> print "2";
     * Optimize to:
     * >> print "0\n",;
     * >> print "1\n",;
     * >> print "2\n",;
     * Optimize to:
     * >> print "0\n1\n2\n",;
     */
   }
  } else {
   if (self->ast_loop.ast_cond &&
       ast_optimize(&stack,self->ast_loop.ast_cond,true)) goto err;
   if (self->ast_loop.ast_next &&
       ast_optimize(&stack,self->ast_loop.ast_next,false)) goto err;
   if (self->ast_loop.ast_cond) {
    /* TODO: Do this optimization in regards to the current, known state of variables. */
    int condition_value;
    /* Optimize constant conditions. */
    condition_value = ast_get_boolean(self->ast_loop.ast_cond);
    if (condition_value > 0) {
     /* Forever-loop (Discard the condition). */
     Dee_Clear(self->ast_loop.ast_cond);
     OPTIMIZE_VERBOSE("Removing constant-true loop condition\n");
     ++optimizer_count;
    } else if (condition_value == 0) {
#if 0 /* TODO: Can only be done when no loop control statements are used. */
     /* Unused loop:
      * >> while (0) { ... }     // optimize to `none'
      * >> do { ... } while (0); // optimize to `{ ...; none; }' */
     if (self->ast_flag&AST_FLOOP_POSTCOND) {
      /* Convert to `{ <loop>; <next>; <cond>; none; }' */
      DREF DeeAstObject **elemv,*none_ast,**iter;
      elemv = (DREF DeeAstObject **)Dee_Malloc(4*sizeof(DREF DeeAstObject *));
      if unlikely(!elemv) goto err;
      none_ast = ast_setscope_and_ddi(ast_constexpr(Dee_None),self);
      if unlikely(!none_ast) { Dee_Free(elemv); goto err; }
      iter = elemv;
      *iter++ = self->ast_loop.ast_loop; /* Inherit */
      if (self->ast_loop.ast_next)
         *iter++ = self->ast_loop.ast_next; /* Inherit */
      *iter++ = self->ast_loop.ast_cond; /* Inherit */
      *iter++ = none_ast; /* Inherit */
      /* Convert into a multiple-ast. */
      self->ast_multiple.ast_exprc = (size_t)(iter-elemv);
      self->ast_type               = AST_MULTIPLE;
      self->ast_flag               = AST_FMULTIPLE_KEEPLAST;
      self->ast_multiple.ast_exprv = elemv; /* Inherit */
      OPTIMIZE_VERBOSE("Unwinding loop only iterated once\n");
     } else {
      /* Convert to `{ <cond>; none; }' */
      DREF DeeAstObject **elemv,*none_ast;
      elemv = (DREF DeeAstObject **)Dee_Malloc(2*sizeof(DREF DeeAstObject *));
      if unlikely(!elemv) goto err;
      none_ast = ast_setscope_and_ddi(ast_constexpr(Dee_None),self);
      if unlikely(!none_ast) { Dee_Free(elemv); goto err; }
      elemv[0] = self->ast_loop.ast_cond; /* Inherit */
      elemv[1] = none_ast; /* Inherit */
      /* Convert into a multiple-ast. */
      self->ast_multiple.ast_exprc = 2;
      self->ast_type               = AST_MULTIPLE;
      self->ast_flag               = AST_FMULTIPLE_KEEPLAST;
      self->ast_multiple.ast_exprv = elemv; /* Inherit */
      OPTIMIZE_VERBOSE("Deleting loop never executed\n");
     }
     goto did_optimize;
#endif
    }
   }
   if (!(self->ast_flag&AST_FLOOP_POSTCOND)) {
    /* TODO: If the condition is known to be true during the first
     *       pass, convert the loop to a post-conditional loop:
     * >> {
     * >>     local i = 0;
     * >>     for (; i < 10; ++i)
     * >>         print i;
     * >> }
     * Optimize to:
     * >> {
     * >>     local i = 0;
     * >>     do {
     * >>         print i;
     * >>         ++i;
     * >>     } while (i < 10);
     * >> }
     */
   }
   /* TODO: Convert to a foreach-style loop (potentially allowing
    *       for further optimization through loop unwinding):
    * >> {
    * >>     local i = 0;
    * >>     do {
    * >>         print i;
    * >>         ++i;
    * >>     } while (i < 10);
    * >> }
    * Optimize to:
    * >> for (local i: [0:10,1])
    * >>     print i;
    */
  }
  break;

 case AST_CONDITIONAL:
  return ast_optimize_conditional(&stack,self,result_used);

 {
  int ast_value;
  DREF DeeAstObject **elemv;
 case AST_BOOL:
  /* TODO: Optimize the inner expression in regards to
   *       it only being used as a boolean expression */
  if (ast_optimize(&stack,self->ast_boolexpr,result_used)) goto err;
  /* If the result doesn't matter, don't perform a negation. */
  if (!result_used) self->ast_flag &= ~AST_FBOOL_NEGATE;

  /* Figure out if the branch's value is known at compile-time. */
  ast_value = ast_get_boolean(self->ast_boolexpr);
  if (ast_value >= 0) {
   if (self->ast_flag&AST_FBOOL_NEGATE)
       ast_value = !ast_value;
   if (ast_has_sideeffects(self->ast_boolexpr)) {
    /* Replace this branch with `{ ...; true/false; }' */
    elemv = (DREF DeeAstObject **)Dee_Malloc(2*sizeof(DREF DeeAstObject *));
    if unlikely(!elemv) goto err;
    elemv[0] = self->ast_boolexpr;
    elemv[1] = ast_setscope_and_ddi(ast_constexpr(DeeBool_For(ast_value)),self);
    if unlikely(!elemv[1]) { Dee_Free(elemv); goto err; }
    self->ast_type               = AST_MULTIPLE;
    self->ast_flag               = AST_FMULTIPLE_KEEPLAST;
    self->ast_multiple.ast_exprc = 2;
    self->ast_multiple.ast_exprv = elemv;
   } else {
    /* Without side-effects, there is no need to keep the expression around. */
    Dee_Decref_likely(self->ast_boolexpr);
    self->ast_constexpr = DeeBool_For(ast_value);
    Dee_Incref(self->ast_constexpr);
    self->ast_type = AST_CONSTEXPR;
    self->ast_flag = AST_FNORMAL;
   }
   OPTIMIZE_VERBOSE("Propagating constant boolean expression\n");
   goto did_optimize;
  }
  if (ast_predict_type(self->ast_boolexpr) == &DeeBool_Type) {
   if (self->ast_flag & AST_FBOOL_NEGATE) {
    /* TODO: Search for what is making the inner expression
     *       a boolean and try to apply our negation to it:
     * >> x = !({ !foo(); }); // Apply our negation to the inner expression.
     */
   } else {
    /* Optimize away the unnecessary bool-cast */
    if (ast_graft_onto(self,self->ast_boolexpr))
        goto err;
    OPTIMIZE_VERBOSE("Remove unused bool cast\n");
    goto did_optimize;
   }
  }
 } break;

 case AST_EXPAND:
  if (ast_optimize(&stack,self->ast_expandexpr,result_used))
      goto err;
  break;
 case AST_FUNCTION:
  if (ast_optimize(&stack,self->ast_function.ast_code,false))
      goto err;
  if (!DeeCompiler_Current->cp_options ||
     !(DeeCompiler_Current->cp_options->co_assembler & ASM_FNODEC)) {
   /* TODO: Replace initializers of function default-arguments that
    *       are never actually used by the function with `none'
    *    -> That way, we can reduce the size of a produced DEC file.
    * NOTE: Only enable this optimization when a DEC file is being
    *       generated, as it doesn't affect runtime performance in
    *       direct source->assembly execution mode, and only slows
    *       down compilation then. */
  }
  break;

 case AST_LABEL:
  /* Special label-branch. */
  ASSERT(self->ast_label.ast_label);
  if (!self->ast_label.ast_label->tl_goto) {
   /* The label was never used. - Ergo: it should not exist.
    * To signify this, we simply convert this branch to `none'. */
   Dee_Decref((DeeObject *)self->ast_label.ast_base);
   self->ast_type      = AST_CONSTEXPR;
   self->ast_flag      = AST_FNORMAL;
   self->ast_constexpr = Dee_None;
   Dee_Incref(Dee_None);
   OPTIMIZE_VERBOSE("Removing unused label\n");
   goto did_optimize;
  }
  break;

 case AST_OPERATOR:
  return ast_optimize_operator(&stack,self,result_used);

 case AST_ACTION:
  return ast_optimize_action(&stack,self,result_used);

 case AST_SWITCH:
  if (ast_optimize(&stack,self->ast_switch.ast_expr,true)) goto err;
  if (ast_optimize(&stack,self->ast_switch.ast_block,false)) goto err;
  /* TODO: Delete constant cases shared with the default-case. */
  /* TODO: Looking at the type of the switch-expression, check if we can delete
   *       some impossible cases. (e.g. integer cases with string-expression) */
  break;

 {
  struct asm_operand *iter,*end;
 case AST_ASSEMBLY:
  end = (iter = self->ast_assembly.ast_opv)+
               (self->ast_assembly.ast_num_i+
                self->ast_assembly.ast_num_o);
  for (; iter != end; ++iter) {
   if (ast_optimize(&stack,iter->ao_expr,true))
       goto err;
  }
 } break;

 default: break;
 }
 return 0;
did_optimize:
 ++optimizer_count;
 goto again;
err:
 return -1;
}

INTERN int (DCALL ast_optimize_all)(DeeAstObject *__restrict self, bool result_used) {
 int result;
 unsigned int old_value;
 do {
  old_value = optimizer_count;
  result = ast_optimize(NULL,self,result_used);
  if unlikely(result) break;
  /* Stop after the first pass if the `OPTIMIZE_FONEPASS' flag is set. */
  if (optimizer_flags&OPTIMIZE_FONEPASS) break;
  /* Keep optimizing the branch while stuff happens. */
 } while (old_value != optimizer_count);
 return result;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPTIMIZE_C */
