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

INTERN uint16_t optimizer_flags     = OPTIMIZE_FDISABLED;
INTERN uint16_t unwind_limit        = 0;
INTERN unsigned int optimizer_count = 0;

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




#if 1
#define is_builtin_object is_builtin_object
PRIVATE bool DCALL
is_builtin_object(DeeObject *__restrict ob) {
 if (Dec_BuiltinID(ob) != DEC_BUILTINID_UNKNOWN)
     return true;
 /* XXX: What about all the other builtin objects? */
 return false;
}
PRIVATE int DCALL
warn_idcompare_nonbuiltin(DeeAstObject *__restrict warn_ast) {
 return WARNAST(warn_ast,W_COMPARE_SODO_NONBUILTIN_UNDEFINED);
}
#endif


INTERN int (DCALL ast_optimize)(DeeAstObject *__restrict self, bool result_used) {
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

 {
  struct symbol *sym;
 case AST_SYM:
  /* If the symbol is being written to, then we can't optimize for external constants. */
  if (self->ast_flag)
      break;
  sym = self->ast_sym;
  SYMBOL_INPLACE_UNWIND_ALIAS(sym);
  ASSERT(sym->s_nread);
  /* Optimize constant, extern symbols. */
  if (SYMBOL_TYPE(sym) == SYMBOL_TYPE_EXTERN &&
     (SYMBOL_EXTERN_SYMBOL(sym)->ss_flags & MODSYM_FCONSTEXPR)) {
   /* The symbol is allowed to be expanded at compile-time. */
   DREF DeeObject *symval; int error;
   DeeModuleObject *symmod;
   symmod = SYMBOL_EXTERN_MODULE(sym);
   error  = DeeModule_RunInit((DeeObject *)symmod);
   if unlikely(error < 0) goto err;
   if (error == 0) {
    /* The module is not initialized. */
    ASSERT(SYMBOL_EXTERN_SYMBOL(sym)->ss_index <
           symmod->mo_globalc);
#ifndef CONFIG_NO_THREADS
    rwlock_read(&symmod->mo_lock);
#endif
    symval = symmod->mo_globalv[SYMBOL_EXTERN_SYMBOL(sym)->ss_index];
    Dee_XIncref(symval);
#ifndef CONFIG_NO_THREADS
    rwlock_endread(&symmod->mo_lock);
#endif
    /* Make sure that the symbol value is allowed
     * to be expanded in constant expression. */
    if likely(symval) {
     int allowed = allow_constexpr(symval);
     if (allowed == CONSTEXPR_USECOPY) {
      if (DeeObject_InplaceDeepCopy(&symval)) {
       DeeError_Handled(ERROR_HANDLED_RESTORE);
       goto done_set_constexpr;
      }
     } else if (allowed != CONSTEXPR_ALLOWED) {
      goto done_set_constexpr;
     }
     /* Set the value as a constant expression. */
     self->ast_constexpr = symval; /* Inherit */
     self->ast_type      = AST_CONSTEXPR;
     SYMBOL_DEC_NREAD(sym); /* Trace read references. */
     goto did_optimize;
    }
done_set_constexpr:
    Dee_Decref(symval);
   }
  }

 } break;

 {
  DeeAstObject **iter,**end;
  bool is_unreachable,only_constexpr;
 case AST_MULTIPLE:
  if (!result_used &&
       self->ast_flag != AST_FMULTIPLE_KEEPLAST) {
   self->ast_flag = AST_FMULTIPLE_KEEPLAST;
   OPTIMIZE_VERBOSE("Replace unused sequence expression with keep-last multi-branch");
   ++optimizer_count;
  }

  /* Optimize all asts within.
   * In the event of a keep-last AST, delete a branches
   * without side-effects except for the last. */
  end = (iter = self->ast_multiple.ast_exprv)+
                self->ast_multiple.ast_exprc;
  is_unreachable = false;
  only_constexpr = true;
multiple_continue_at_iter:
  for (; iter != end; ++iter) {
   bool using_value = result_used; int temp;
   if (self->ast_flag == AST_FMULTIPLE_KEEPLAST && iter != end-1)
       using_value = false;
   if (ast_optimize(*iter,using_value)) goto err;
   /* TODO: Optimize something like this:
    *       `[10,20,[30,40]...]' --> [10,20,30,40]'
    * NOTE: This shouldn't even be exclusive to constant expressions!
    *       Any time an ast_expand(ast_multiple()) is encounted
    *       here, we can insert all the elements from the
    *       underlying AST here!
    */
   if ((*iter)->ast_type != AST_CONSTEXPR) only_constexpr = false;
   temp = ast_doesnt_return(*iter,AST_DOESNT_RETURN_FNORMAL);
   if (temp < 0) is_unreachable = temp == -2;
   else if (is_unreachable ||
            /* Delete branches that are unreachable or have no side-effects. */
           (self->ast_flag == AST_FMULTIPLE_KEEPLAST &&
           (iter != end-1 || !result_used) && !ast_has_sideeffects(*iter))) {
    /* Get rid of this one. */
    OPTIMIZE_VERBOSEAT(*iter,is_unreachable ? "Delete unreachable AST\n"
                                            : "Delete AST without side-effects\n");
    Dee_Decref(*iter);
    --end;
    --self->ast_multiple.ast_exprc;
    MEMMOVE_PTR(iter,iter+1,(size_t)(end-iter));
    ++optimizer_count;
    goto multiple_continue_at_iter;
   } else if (temp > 0) {
    /* If the AST doesn't return, everything that follows is no-return. */
    is_unreachable = true;
   }
  }
  if (only_constexpr) {
   if (self->ast_multiple.ast_exprc == 1 &&
      !self->ast_ddi.l_file) {
    memcpy(&self->ast_ddi,
           &self->ast_multiple.ast_exprv[0]->ast_ddi,
           sizeof(struct ast_loc));
    if (self->ast_ddi.l_file)
        TPPFile_Incref(self->ast_ddi.l_file);
   }
   if (self->ast_multiple.ast_exprc == 0) {
    DREF DeeObject *cexpr;
    if (self->ast_flag == AST_FMULTIPLE_KEEPLAST) {
     cexpr = Dee_None;
     Dee_Incref(cexpr);
    } else if (self->ast_flag == AST_FMULTIPLE_TUPLE ||
               self->ast_flag == AST_FMULTIPLE_GENERIC) {
     cexpr = Dee_EmptyTuple;
     Dee_Incref(cexpr);
    } else if (self->ast_flag == AST_FMULTIPLE_LIST) {
     cexpr = DeeList_New();
     if unlikely(!cexpr) goto err;
    } else if (self->ast_flag == AST_FMULTIPLE_SET) {
     cexpr = DeeHashSet_New();
     if unlikely(!cexpr) goto err;
    } else if (self->ast_flag == AST_FMULTIPLE_DICT) {
     cexpr = DeeDict_New();
     if unlikely(!cexpr) goto err;
    } else if (self->ast_flag == AST_FMULTIPLE_GENERIC_KEYS) {
     cexpr = DeeRoDict_New();
     if unlikely(!cexpr) goto err;
    } else {
     goto after_multiple_constexpr;
    }
    Dee_Free(self->ast_multiple.ast_exprv);
    self->ast_constexpr = cexpr; /* Inherit reference */
   } else if (self->ast_flag == AST_FMULTIPLE_KEEPLAST) {
    if unlikely(ast_graft_onto(self,self->ast_multiple.ast_exprv[
                                    self->ast_multiple.ast_exprc-1]))
       goto err;
   } else if (self->ast_flag == AST_FMULTIPLE_TUPLE ||
              self->ast_flag == AST_FMULTIPLE_GENERIC) {
    DREF DeeObject *new_tuple; size_t i;
    new_tuple = DeeTuple_NewUninitialized(self->ast_multiple.ast_exprc);
    if unlikely(!new_tuple) goto err;
    for (i = 0; i < self->ast_multiple.ast_exprc; ++i) {
     DeeAstObject *ast = self->ast_multiple.ast_exprv[i];
     DeeObject *value = ast->ast_constexpr;
     DeeTuple_SET(new_tuple,i,value);
     Dee_Incref(value);
     Dee_Decref(ast);
    }
    Dee_Free(self->ast_multiple.ast_exprv);
    self->ast_constexpr = new_tuple; /* Inherit reference. */
   } else if (self->ast_flag == AST_FMULTIPLE_LIST) {
    DREF DeeObject *new_list; size_t i;
    new_list = DeeList_NewUninitialized(self->ast_multiple.ast_exprc);
    if unlikely(!new_list) goto err;
    for (i = 0; i < self->ast_multiple.ast_exprc; ++i) {
     DeeAstObject *ast = self->ast_multiple.ast_exprv[i];
     DeeObject *value = ast->ast_constexpr;
     DeeList_SET(new_list,i,value);
     Dee_Incref(value);
     Dee_Decref(ast);
    }
    DeeGC_Track((DeeObject *)new_list);
    Dee_Free(self->ast_multiple.ast_exprv);
    self->ast_constexpr = new_list; /* Inherit reference. */
   } else if (self->ast_flag == AST_FMULTIPLE_SET) {
    DREF DeeObject *new_set; size_t i;
    new_set = DeeHashSet_New();
    if unlikely(!new_set) goto err;
    for (i = 0; i < self->ast_multiple.ast_exprc; ++i) {
     if (DeeHashSet_Insert(new_set,self->ast_multiple.ast_exprv[i]->ast_constexpr) < 0) {
      Dee_Decref(new_set);
      goto err;
     }
    }
    for (i = 0; i < self->ast_multiple.ast_exprc; ++i)
         Dee_Decref(self->ast_multiple.ast_exprv[i]);
    Dee_Free(self->ast_multiple.ast_exprv);
    self->ast_constexpr = new_set;
   } else if (self->ast_flag == AST_FMULTIPLE_DICT) {
    DREF DeeObject *new_dict; size_t i;
    new_dict = DeeDict_New();
    if unlikely(!new_dict) goto err;
    for (i = 0; i < self->ast_multiple.ast_exprc/2; ++i) {
     DeeObject *key  = self->ast_multiple.ast_exprv[(i*2)+0]->ast_constexpr;
     DeeObject *item = self->ast_multiple.ast_exprv[(i*2)+1]->ast_constexpr;
     if (DeeObject_SetItem(new_dict,key,item)) {
      Dee_Decref(new_dict);
      goto err;
     }
    }
    for (i = 0; i < self->ast_multiple.ast_exprc; ++i)
         Dee_Decref(self->ast_multiple.ast_exprv[i]);
    Dee_Free(self->ast_multiple.ast_exprv);
    self->ast_constexpr = new_dict; /* Inherit reference. */
   } else if (self->ast_flag == AST_FMULTIPLE_GENERIC_KEYS) {
    DREF DeeObject *new_dict; size_t i,length;
    length   = self->ast_multiple.ast_exprc/2;
    new_dict = DeeRoDict_NewWithHint(length);
    if unlikely(!new_dict) goto err;
    for (i = 0; i < length; ++i) {
     DeeObject *key  = self->ast_multiple.ast_exprv[(i*2)+0]->ast_constexpr;
     DeeObject *item = self->ast_multiple.ast_exprv[(i*2)+1]->ast_constexpr;
     if (DeeRoDict_Insert(&new_dict,key,item)) {
      Dee_Decref(new_dict);
      goto err;
     }
    }
    for (i = 0; i < self->ast_multiple.ast_exprc; ++i)
         Dee_Decref(self->ast_multiple.ast_exprv[i]);
    Dee_Free(self->ast_multiple.ast_exprv);
    self->ast_constexpr = new_dict; /* Inherit reference. */
   } else {
    goto after_multiple_constexpr;
   }
   self->ast_flag = AST_FNORMAL;
   self->ast_type = AST_CONSTEXPR;
   OPTIMIZE_VERBOSE("Reduce constant-expression multi-ast\n");
   goto did_optimize;
  }
after_multiple_constexpr:
  if (self->ast_flag == AST_FMULTIPLE_KEEPLAST) {
   switch (self->ast_multiple.ast_exprc) {
   case 1:
    if (self->ast_scope == self->ast_multiple.ast_exprv[0]->ast_scope) {
     /* We can simply inherit this single branch, simplifying the entire AST. */
     if (ast_assign(self,self->ast_multiple.ast_exprv[0])) goto err;
     OPTIMIZE_VERBOSE("Collapse single-branch multi-ast\n");
     goto did_optimize;
    }
    break;
   case 0:
    /* Convert this branch into `none' */
    Dee_Free(self->ast_multiple.ast_exprv);
    self->ast_type = AST_CONSTEXPR;
    self->ast_flag = AST_FNORMAL;
    self->ast_constexpr = Dee_None;
    Dee_Incref(Dee_None);
    OPTIMIZE_VERBOSE("Replace empty multi-ast with `none'\n");
    goto did_optimize;
   default: break;
   }
  }
 } break;

 case AST_RETURN:
 case AST_YIELD:
 case AST_THROW:
  if (self->ast_returnexpr &&
      ast_optimize(self->ast_returnexpr,true))
      goto err;
  break;

 {
  struct catch_expr *iter,*end;
 case AST_TRY:
  if (ast_optimize(self->ast_try.ast_guard,result_used))
      goto err;
  if (ast_is_nothrow(self->ast_try.ast_guard,result_used)) {
   /* TODO: `try { foo; } finally { bar; }' -> `({ __stack local _r = foo; bar; _r; })'
    * TODO: `try { foo; } catch (...) { bar; }' -> `(foo)' */
  }
  end = (iter = self->ast_try.ast_catchv)+self->ast_try.ast_catchc;
  for (; iter != end; ++iter) {
   if (ast_optimize(iter->ce_code,false)) goto err;
   if (iter->ce_mask &&
       ast_optimize(iter->ce_mask,false)) goto err;
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
      ast_optimize(self->ast_loop.ast_loop,false)) goto err;
  if (self->ast_flag&AST_FLOOP_FOREACH) {
   /* foreach-style loop. */
   if (ast_optimize(self->ast_loop.ast_iter,true)) goto err;
   if (unwind_limit != 0) {
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
       ast_optimize(self->ast_loop.ast_cond,true)) goto err;
   if (self->ast_loop.ast_next &&
       ast_optimize(self->ast_loop.ast_next,false)) goto err;
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

 {
  int constant_condition;
  int tt_value,ff_value;
 case AST_CONDITIONAL:
  /* Obviously: optimize sub-branches and do dead-branch elimination. */
  /* TODO: Optimize the inner expressions in regards to
   *       them only being used as a boolean expression. */
  if (ast_optimize(self->ast_conditional.ast_cond,true)) goto err;
  if (self->ast_conditional.ast_tt &&
      self->ast_conditional.ast_tt != self->ast_conditional.ast_cond &&
      ast_optimize(self->ast_conditional.ast_tt,result_used)) goto err;
  if (self->ast_conditional.ast_ff &&
      self->ast_conditional.ast_ff != self->ast_conditional.ast_cond &&
      ast_optimize(self->ast_conditional.ast_ff,result_used)) goto err;
  if (self->ast_conditional.ast_tt == self->ast_conditional.ast_cond &&
      self->ast_conditional.ast_ff == self->ast_conditional.ast_cond) {
   /* ??? Why though? */
   if (result_used) {
    Dee_DecrefNokill(self->ast_conditional.ast_tt);
    Dee_DecrefNokill(self->ast_conditional.ast_ff);
    __STATIC_IF(COMPILER_OFFSETOF(DeeAstObject,ast_boolexpr) !=
                COMPILER_OFFSETOF(DeeAstObject,ast_conditional.ast_cond)) {
     self->ast_boolexpr = self->ast_conditional.ast_cond;
    }
    self->ast_type = AST_BOOL;
    self->ast_flag = AST_FBOOL_NORMAL;
   } else {
    if (ast_graft_onto(self,self->ast_conditional.ast_cond))
        goto err;
   }
   OPTIMIZE_VERBOSE("Remove no-op conditional branch\n");
   goto did_optimize;
  }

  constant_condition = ast_get_boolean(self);
  if (constant_condition >= 0) {
   DeeAstObject *eval_branch,*other_branch;
   /* Only evaluate the tt/ff-branch. */
   if (constant_condition) {
    eval_branch  = self->ast_conditional.ast_tt;
    other_branch = self->ast_conditional.ast_ff;
   } else {
    eval_branch  = self->ast_conditional.ast_ff;
    other_branch = self->ast_conditional.ast_tt;
   }
   /* We can't optimize away the dead branch when it contains a label. */
   if (other_branch && ast_doesnt_return(other_branch,AST_DOESNT_RETURN_FNORMAL) < 0)
       break;
   if (eval_branch == self->ast_conditional.ast_cond) {
    /* Special case: The branch that is getting evaluated
     *               is referencing the condition. */
    if (self->ast_flag&AST_FCOND_BOOL) {
     /* Must also convert `eval_branch' to a boolean. */
     DREF DeeAstObject *graft; int temp;
     graft = ast_setscope_and_ddi(ast_bool(AST_FBOOL_NORMAL,eval_branch),self);
     if unlikely(!graft) goto err;
     temp = ast_graft_onto(self,graft);
     Dee_Decref(graft);
     if unlikely(temp) goto err;
    } else {
     if (ast_graft_onto(self,eval_branch)) goto err;
    }
   } else {
    DREF DeeAstObject **elemv;
    /* Merge the eval-branch and the condition:
     * >> if (true) {
     * >>     print "Here";
     * >> }
     * Optimize to:
     * >> {
     * >>     true;
     * >>     print "Here";
     * >> }
     */
    elemv = (DREF DeeAstObject **)Dee_Malloc(2*sizeof(DREF DeeAstObject *));
    if unlikely(!elemv) goto err;
    elemv[0] = self->ast_conditional.ast_cond;
    /* Cast the branch being evaluated to a boolean if required. */
    if (self->ast_flag&AST_FCOND_BOOL) {
     DREF DeeAstObject *merge;
     merge = ast_setscope_and_ddi(ast_bool(AST_FBOOL_NORMAL,eval_branch),self);
     if unlikely(!merge) { Dee_Free(elemv); goto err; }
     Dee_Decref(eval_branch);
     eval_branch = merge;
    }
    elemv[1] = eval_branch; /* Inherit */
    /* Override (and inherit) this AST. */
    self->ast_type               = AST_MULTIPLE;
    self->ast_flag               = AST_FMULTIPLE_KEEPLAST;
    self->ast_multiple.ast_exprc = 2;
    self->ast_multiple.ast_exprv = elemv;
   }
   OPTIMIZE_VERBOSE("Expanding conditional branch with constant condition\n");
   goto did_optimize;
  }
  if (self->ast_flag & AST_FCOND_BOOL) {
   tt_value = -2;
   ff_value = -2;
   if (self->ast_conditional.ast_tt == self->ast_conditional.ast_cond) {
    ff_value = self->ast_conditional.ast_ff
             ? ast_get_boolean_noeffect(self->ast_conditional.ast_ff)
             : 0;
    if (ff_value >= 0) {
     if (ff_value && result_used) {
      /* Optimize: `!!foo() ? : true' --> `({ foo(); true; })' */
      DREF DeeAstObject **elemv;
      ASSERT(self->ast_conditional.ast_ff);
      elemv = (DREF DeeAstObject **)Dee_Malloc(2*sizeof(DREF DeeAstObject *));
      if unlikely(!elemv) goto err;
      elemv[0] = self->ast_conditional.ast_cond; /* Inherit reference. */
      elemv[1] = self->ast_conditional.ast_ff;   /* Inherit reference. */
      Dee_DecrefNokill(self->ast_conditional.ast_tt);
      self->ast_multiple.ast_exprc = 2;
      self->ast_multiple.ast_exprv = elemv;
      self->ast_type = AST_MULTIPLE;
      self->ast_flag = AST_FMULTIPLE_KEEPLAST;
     } else if (result_used) {
      /* Optimize: `!!foo() ? : false' --> `!!foo();' */
      Dee_DecrefNokill(self->ast_conditional.ast_tt);
      Dee_XDecref(self->ast_conditional.ast_ff);
      __STATIC_IF(COMPILER_OFFSETOF(DeeAstObject,ast_boolexpr) !=
                  COMPILER_OFFSETOF(DeeAstObject,ast_conditional.ast_cond)) {
       self->ast_boolexpr = self->ast_conditional.ast_cond;
      }
      self->ast_type = AST_BOOL;
      self->ast_flag = AST_FBOOL_NORMAL;
     } else {
      /* Optimize: `!!foo() ? : false' --> `foo();' */
      if (ast_graft_onto(self,self->ast_conditional.ast_cond))
          goto err;
     }
     OPTIMIZE_VERBOSE("Flatten constant false-branch of boolean conditional tt-is-cond expression\n");
     goto did_optimize;
    }
   }
   if (self->ast_conditional.ast_ff == self->ast_conditional.ast_cond) {
    tt_value = self->ast_conditional.ast_tt
             ? ast_get_boolean_noeffect(self->ast_conditional.ast_tt)
             : 0;
    if (tt_value >= 0) {
     if (!tt_value && result_used) {
      /* Optimize: `!!foo() ? false : ' --> `({ foo(); false; })' */
      DREF DeeAstObject **elemv;
      ASSERT(self->ast_conditional.ast_tt);
      elemv = (DREF DeeAstObject **)Dee_Malloc(2*sizeof(DREF DeeAstObject *));
      if unlikely(!elemv) goto err;
      elemv[0] = self->ast_conditional.ast_cond; /* Inherit reference. */
      elemv[1] = self->ast_conditional.ast_tt;   /* Inherit reference. */
      Dee_DecrefNokill(self->ast_conditional.ast_ff);
      self->ast_multiple.ast_exprc = 2;
      self->ast_multiple.ast_exprv = elemv;
      self->ast_type = AST_MULTIPLE;
      self->ast_flag = AST_FMULTIPLE_KEEPLAST;
     } else if (result_used) {
      /* Optimize: `!!foo() ? true : ' --> `!!foo();' */
      Dee_DecrefNokill(self->ast_conditional.ast_ff);
      Dee_XDecref(self->ast_conditional.ast_tt);
      __STATIC_IF(COMPILER_OFFSETOF(DeeAstObject,ast_boolexpr) !=
                  COMPILER_OFFSETOF(DeeAstObject,ast_conditional.ast_cond)) {
       self->ast_boolexpr = self->ast_conditional.ast_cond;
      }
      self->ast_type = AST_BOOL;
      self->ast_flag = AST_FBOOL_NORMAL;
     } else {
      /* Optimize: `!!foo() ? true : ' --> `foo();' */
      if (ast_graft_onto(self,self->ast_conditional.ast_cond))
          goto err;
     }
     OPTIMIZE_VERBOSE("Flatten constant true-branch of boolean conditional ff-is-cond expression\n");
     goto did_optimize;
    }
   }
   if (tt_value == -2) {
    tt_value = self->ast_conditional.ast_tt
             ? ast_get_boolean_noeffect(self->ast_conditional.ast_tt)
             : 0;
   }
   if (ff_value == -2) {
    ff_value = self->ast_conditional.ast_ff
             ? ast_get_boolean_noeffect(self->ast_conditional.ast_ff)
             : 0;
   }
   if (tt_value >= 0 && ff_value >= 0) {
    /* In a boolean context, both branches can be predicted at compile-time,
     * meaning we can optimize using a matrix:
     * >>  cond ? false : false --> ({ cond; false; })
     * >>  cond ? false : true  --> !cond
     * >>  cond ? true : false  --> !!cond
     * >>  cond ? true : true   --> ({ cond; true; })
     */
apply_bool_matrix_transformation:
    if (tt_value) {
     if (ff_value) {
      /* cond ? true : true  --> ({ cond; true; }) */
      DREF DeeAstObject **elemv;
optimize_conditional_bool_predictable_inherit_multiple:
      elemv = (DREF DeeAstObject **)Dee_Malloc(2*sizeof(DREF DeeAstObject *));
      if unlikely(!elemv) goto err;
      elemv[0] = self->ast_conditional.ast_cond; /* Inherit reference. */
      elemv[1] = self->ast_conditional.ast_tt;   /* Inherit reference. */
      Dee_Decref(self->ast_conditional.ast_ff);
      self->ast_multiple.ast_exprc = 2;
      self->ast_multiple.ast_exprv = elemv;
      self->ast_type = AST_MULTIPLE;
      self->ast_flag = AST_FMULTIPLE_KEEPLAST;
     } else {
      /* cond ? true : false  --> !!cond */
      Dee_Decref(self->ast_conditional.ast_tt);
      Dee_Decref(self->ast_conditional.ast_ff);
      __STATIC_IF(COMPILER_OFFSETOF(DeeAstObject,ast_boolexpr) !=
                  COMPILER_OFFSETOF(DeeAstObject,ast_conditional.ast_cond)) {
       self->ast_boolexpr = self->ast_conditional.ast_cond;
      }
      self->ast_type = AST_BOOL;
      self->ast_flag = AST_FBOOL_NORMAL;
     }
    } else {
     if (!ff_value) {
      /* cond ? false : false --> ({ cond; false; }) */
      goto optimize_conditional_bool_predictable_inherit_multiple;
     } else {
      /* cond ? false : true  --> !cond */
      Dee_Decref(self->ast_conditional.ast_tt);
      Dee_Decref(self->ast_conditional.ast_ff);
      __STATIC_IF(COMPILER_OFFSETOF(DeeAstObject,ast_boolexpr) !=
                  COMPILER_OFFSETOF(DeeAstObject,ast_conditional.ast_cond)) {
       self->ast_boolexpr = self->ast_conditional.ast_cond;
      }
      self->ast_type = AST_BOOL;
      self->ast_flag = AST_FBOOL_NEGATE;
     }
    }
    OPTIMIZE_VERBOSE("Apply matrix to constant tt/ff branches of boolean conditional expression\n");
    goto did_optimize;
   }
  }
  if (!result_used) {
   /* Remove branches without any side-effects. */
   if (self->ast_conditional.ast_tt &&
       self->ast_conditional.ast_tt != self->ast_conditional.ast_cond &&
      !ast_has_sideeffects(self->ast_conditional.ast_tt)) {
    if (self->ast_conditional.ast_ff) {
     Dee_Clear(self->ast_conditional.ast_tt);
    } else {
     if (ast_graft_onto(self,self->ast_conditional.ast_cond))
         goto err;
    }
    OPTIMIZE_VERBOSE("Remove conditional tt-branch without any side-effects\n");
    goto did_optimize;
   }
   if (self->ast_conditional.ast_ff &&
       self->ast_conditional.ast_ff != self->ast_conditional.ast_cond &&
      !ast_has_sideeffects(self->ast_conditional.ast_ff)) {
    if (self->ast_conditional.ast_tt) {
     Dee_Clear(self->ast_conditional.ast_ff);
    } else {
     if (ast_graft_onto(self,self->ast_conditional.ast_cond))
         goto err;
    }
    OPTIMIZE_VERBOSE("Remove conditional ff-branch without any side-effects\n");
    goto did_optimize;
   }
  }

  if (self->ast_conditional.ast_tt && self->ast_conditional.ast_ff) {
   DeeAstObject *tt = self->ast_conditional.ast_tt;
   DeeAstObject *ff = self->ast_conditional.ast_ff;
   /* TODO: if the current context only needs a boolean value,
    *       optimize constant expressions into Dee_True / Dee_False */
   if (tt->ast_type == AST_CONSTEXPR && ff->ast_type == AST_CONSTEXPR) {
    /* Optimize when both branches are in use and both are constant expressions:
     * >> get_cond() ? 10 : 20;
     * Optimize to:
     * >> pack(10,20)[!!get_cond()]; */
    DREF DeeObject *argument_packet;
    /* Special case: If both values are boolean, we can apply a bool-matrix. */
    if (DeeBool_Check(tt->ast_constexpr) &&
        DeeBool_Check(ff->ast_constexpr)) {
     /* TODO: Even without this special case, the optimization
      *       for `operator []' should be also be able to get
      *       rid of this! */
     tt_value = DeeBool_IsTrue(tt->ast_constexpr);
     ff_value = DeeBool_IsTrue(ff->ast_constexpr);
     goto apply_bool_matrix_transformation;
    }
    argument_packet = DeeTuple_Pack(2,
                                    ff->ast_constexpr,
                                    tt->ast_constexpr);
    if unlikely(!argument_packet) goto err;
    Dee_Decref(tt->ast_constexpr);
    Dee_Decref(ff->ast_constexpr);
    tt->ast_constexpr = argument_packet; /* Inherit reference. */
    ff->ast_type      = AST_BOOL;
    ff->ast_flag      = AST_FBOOL_NORMAL;
    ff->ast_boolexpr  = self->ast_conditional.ast_cond; /* Inherit reference. */
    self->ast_operator.ast_opa    = tt; /* Inherit reference. */
    self->ast_operator.ast_opb    = ast_setddi(ff,&self->ast_ddi); /* Inherit reference. */
    self->ast_operator.ast_opc    = NULL;
    self->ast_operator.ast_opd    = NULL;
    self->ast_operator.ast_exflag = AST_OPERATOR_FNORMAL;
    self->ast_flag = OPERATOR_GETITEM;
    self->ast_type = AST_OPERATOR;
    OPTIMIZE_VERBOSE("Use result-vectoring to reduce constant tt/ff branches of conditional expression\n");
    goto did_optimize;
   }
   if (optimizer_flags & OPTIMIZE_FCSE) {
    if (ast_equal(tt,ff)) {
     /* The true and false branches are identical. */
     DREF DeeAstObject **elemv;
if_statement_branches_identical:
     elemv = (DREF DeeAstObject **)Dee_Malloc(2*sizeof(DREF DeeAstObject *));
     if unlikely(!elemv) goto err;
     ast_fini_contents(tt);
     tt->ast_type = AST_BOOL;
     tt->ast_flag = AST_FBOOL_NORMAL;
     tt->ast_boolexpr = self->ast_conditional.ast_cond; /* Inherit reference. */
     elemv[0] = tt; /* Inherit reference. */
     elemv[1] = ff; /* Inherit reference. */
     self->ast_multiple.ast_exprc = 2;
     self->ast_multiple.ast_exprv = elemv;
     self->ast_type = AST_MULTIPLE;
     self->ast_flag = AST_FMULTIPLE_KEEPLAST;
     OPTIMIZE_VERBOSE("Flatten identical true/false branches\n");
     goto did_optimize;
    }
    /* Optimize stuff like this:
     * >> if (foo()) {
     * >>     print "Begin";
     * >>     DoTheThing();
     * >>     print "End";
     * >> } else {
     * >>     print "Begin";
     * >>     DoTheOtherThing();
     * >>     print "End";
     * >> }
     * Into:
     * >> print "Begin";
     * >> if (foo()) {
     * >>     DoTheThing();
     * >> } else {
     * >>     DoTheOtherThing();
     * >> }
     * >> print "End";
     */
    {
     DeeAstObject **tt_astv; size_t tt_astc;
     DeeAstObject **ff_astv; size_t ff_astc;
     if (tt->ast_type == AST_MULTIPLE &&
         tt->ast_flag == AST_FMULTIPLE_KEEPLAST)
      tt_astv = tt->ast_multiple.ast_exprv,
      tt_astc = tt->ast_multiple.ast_exprc;
     else {
      tt_astv = &tt;
      tt_astc = 1;
     }
     if (ff->ast_type == AST_MULTIPLE &&
         ff->ast_flag == AST_FMULTIPLE_KEEPLAST)
      ff_astv = ff->ast_multiple.ast_exprv,
      ff_astc = ff->ast_multiple.ast_exprc;
     else {
      ff_astv = &ff;
      ff_astc = 1;
     }
     size_t move_count = 0;
     while (move_count < tt_astc &&
            move_count < ff_astc) {
      DeeAstObject *tt_last = tt_astv[tt_astc-(move_count+1)];
      DeeAstObject *ff_last = ff_astv[ff_astc-(move_count+1)];
      if (!ast_equal(tt_last,ff_last)) break;
      ++move_count;
     }
     if (move_count) {
      if (!tt_astc && !ff_astc)
           goto if_statement_branches_identical;
      if (!tt_astc) {
       /* Only the false-branch remains. */


      }
      if (!ff_astc) {
      }
     }
    }
   }
  }
 } break;

 {
  int ast_value;
  DREF DeeAstObject **elemv;
 case AST_BOOL:
  /* TODO: Optimize the inner expression in regards to
   *       it only being used as a boolean expression */
  if (ast_optimize(self->ast_boolexpr,result_used)) goto err;
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
  if (ast_optimize(self->ast_expandexpr,result_used))
      goto err;
  break;
 case AST_FUNCTION:
  if (ast_optimize(self->ast_function.ast_code,false))
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
  return ast_optimize_operator(self,result_used);

 {
  DREF DeeObject *expr_result;
 case AST_ACTION:
  /* TODO: The result-used parameter depends on what kind of action it is...
   * TODO: Optimize AST order of `AST_FACTION_IN' and `AST_FACTION_AS'.
   * TODO: Do constant propagation when branches are known at compile-time. */
  switch (self->ast_flag) {

  case AST_FACTION_STORE:
   /* Check for store-to-none (which is a no-op that translates to the stored expression) */
   if (ast_optimize(self->ast_action.ast_act0,true)) goto err;
   if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR &&
       DeeNone_Check(self->ast_action.ast_act0->ast_constexpr)) {
    /* Optimize the source-expression with propagated result-used settings. */
    if (ast_optimize(self->ast_action.ast_act1,result_used)) goto err;
    OPTIMIZE_VERBOSE("Discarding store-to-none\n");
    if (ast_graft_onto(self,self->ast_action.ast_act1)) goto err;
    goto did_optimize;
   }
   if (ast_optimize(self->ast_action.ast_act1,true)) goto err;
   break;

  case AST_FACTION_IN:
   if (ast_optimize(self->ast_action.ast_act0,true)) goto err;
   if (ast_optimize(self->ast_action.ast_act1,true)) goto err;
   if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR &&
       self->ast_action.ast_act1->ast_type == AST_CONSTEXPR) {
    /* Propagate constants. */
    expr_result = DeeObject_ContainsObject(self->ast_action.ast_act1->ast_constexpr,
                                           self->ast_action.ast_act0->ast_constexpr);
action_set_expr_result:
    if (!expr_result)
     DeeError_Handled(ERROR_HANDLED_RESTORE);
    else {
     int temp = allow_constexpr(expr_result);
     if (temp == CONSTEXPR_ILLEGAL)
         Dee_Decref(expr_result);
     else {
      if (temp == CONSTEXPR_USECOPY &&
          DeeObject_InplaceDeepCopy(&expr_result)) {
       Dee_Decref(expr_result);
       DeeError_Handled(ERROR_HANDLED_RESTORE);
      } else {
       Dee_XDecref(self->ast_action.ast_act2);
       Dee_XDecref(self->ast_action.ast_act1);
       Dee_XDecref(self->ast_action.ast_act0);
       self->ast_constexpr = expr_result; /* Inherit reference. */
       self->ast_type      = AST_CONSTEXPR;
       OPTIMIZE_VERBOSE("Propagate constant result of action-expression\n");
       goto did_optimize;
      }
     }
    }
   }
   break;

  case AST_FACTION_AS:
   if (ast_optimize(self->ast_action.ast_act0,true)) goto err;
   if (ast_optimize(self->ast_action.ast_act1,true)) goto err;
   if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR &&
       self->ast_action.ast_act1->ast_type == AST_CONSTEXPR &&
       DeeType_Check(self->ast_action.ast_act1->ast_constexpr)) {
    /* Propagate constants. */
    expr_result = DeeSuper_New((DeeTypeObject *)self->ast_action.ast_act1->ast_constexpr,
                                self->ast_action.ast_act0->ast_constexpr);
    goto action_set_expr_result;
   }
   break;

  case AST_FACTION_IS:
   if (ast_optimize(self->ast_action.ast_act0,true)) goto err;
   if (ast_optimize(self->ast_action.ast_act1,true)) goto err;
   if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR &&
       self->ast_action.ast_act1->ast_type == AST_CONSTEXPR) {
    if (DeeType_Check(self->ast_action.ast_act1->ast_constexpr)) {
     /* Propagate constants. */
     expr_result = DeeBool_For(DeeObject_InstanceOf(self->ast_action.ast_act0->ast_constexpr,
                                   (DeeTypeObject *)self->ast_action.ast_act1->ast_constexpr));
     Dee_Incref(expr_result);
     goto action_set_expr_result;
    }
    if (DeeNone_Check(self->ast_action.ast_act1->ast_constexpr)) {
     /* Propagate constants. */
     expr_result = DeeBool_For(DeeNone_Check(self->ast_action.ast_act0->ast_constexpr));
     Dee_Incref(expr_result);
     goto action_set_expr_result;
    }
   }
   break;

  case AST_FACTION_MIN:
   if (ast_optimize(self->ast_action.ast_act0,true)) goto err;
   if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR) {
    expr_result = DeeSeq_Min(self->ast_action.ast_act0->ast_constexpr,NULL);
    goto action_set_expr_result;
   }
   break;
  case AST_FACTION_MAX:
   if (ast_optimize(self->ast_action.ast_act0,true)) goto err;
   if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR) {
    expr_result = DeeSeq_Max(self->ast_action.ast_act0->ast_constexpr,NULL);
    goto action_set_expr_result;
   }
   break;
  case AST_FACTION_SUM:
   if (ast_optimize(self->ast_action.ast_act0,true)) goto err;
   if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR) {
    expr_result = DeeSeq_Sum(self->ast_action.ast_act0->ast_constexpr);
    goto action_set_expr_result;
   }
   break;
  case AST_FACTION_ANY:
   if (ast_optimize(self->ast_action.ast_act0,true)) goto err;
   if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR) {
    int temp = DeeSeq_Any(self->ast_action.ast_act0->ast_constexpr);
    if (temp < 0)
     expr_result = NULL;
    else {
     expr_result = DeeBool_For(temp);
     Dee_Incref(expr_result);
    }
    goto action_set_expr_result;
   }
   break;
  case AST_FACTION_ALL:
   if (ast_optimize(self->ast_action.ast_act0,true)) goto err;
   if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR) {
    int temp = DeeSeq_All(self->ast_action.ast_act0->ast_constexpr);
    if (temp < 0)
     expr_result = NULL;
    else {
     expr_result = DeeBool_For(temp);
     Dee_Incref(expr_result);
    }
    goto action_set_expr_result;
   }
   break;

  case AST_FACTION_BOUNDATTR:
   if (ast_optimize(self->ast_action.ast_act0,true)) goto err;
   if (ast_optimize(self->ast_action.ast_act1,true)) goto err;
   if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR &&
       self->ast_action.ast_act1->ast_type == AST_CONSTEXPR &&
       DeeString_Check(self->ast_action.ast_act1->ast_constexpr)) {
    int temp = DeeObject_HasAttr(self->ast_action.ast_act0->ast_constexpr,
                                 self->ast_action.ast_act1->ast_constexpr);
    if (temp < 0)
     expr_result = NULL;
    else {
     expr_result = DeeBool_For(temp);
     Dee_Incref(expr_result);
    }
    goto action_set_expr_result;
   }
   break;

  case AST_FACTION_SAMEOBJ:
   if (ast_optimize(self->ast_action.ast_act0,true)) goto err;
   if (ast_optimize(self->ast_action.ast_act1,true)) goto err;
   if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR &&
       self->ast_action.ast_act1->ast_type == AST_CONSTEXPR) {
#ifdef is_builtin_object
    /* Warn if the constant of either branch isn't a builtin object */
    if ((!is_builtin_object(self->ast_action.ast_act0->ast_constexpr) ||
         !is_builtin_object(self->ast_action.ast_act1->ast_constexpr)) &&
         warn_idcompare_nonbuiltin(self)) goto err;
#endif
    expr_result = DeeBool_For(self->ast_action.ast_act0->ast_constexpr ==
                              self->ast_action.ast_act1->ast_constexpr);
    Dee_Incref(expr_result);
    goto action_set_expr_result;
   } else if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR) {
#ifdef is_builtin_object
    /* Warn if the constant of `act0' isn't a builtin object */
    if (!is_builtin_object(self->ast_action.ast_act0->ast_constexpr) &&
         warn_idcompare_nonbuiltin(self)) goto err;
#endif
    if (DeeNone_Check(self->ast_action.ast_act0->ast_constexpr)) {
     /* Optimize: `none === x' -> `x is none' */
     DeeAstObject *temp;
     temp = self->ast_action.ast_act1;
     self->ast_action.ast_act1 = self->ast_action.ast_act0;
     self->ast_action.ast_act0 = temp;
     self->ast_flag = AST_FACTION_IS;
     OPTIMIZE_VERBOSE("Optimize `none === x' -> `x is none'\n");
     goto did_optimize;
    }
   } else if (self->ast_action.ast_act1->ast_type == AST_CONSTEXPR) {
#ifdef is_builtin_object
    /* Warn if the constant of `act1' isn't a builtin object */
    if (!is_builtin_object(self->ast_action.ast_act1->ast_constexpr) &&
         warn_idcompare_nonbuiltin(self)) goto err;
#endif
    if (DeeNone_Check(self->ast_action.ast_act0->ast_constexpr)) {
     /* Optimize: `x === none' -> `x is none' */
     self->ast_flag = AST_FACTION_IS;
     OPTIMIZE_VERBOSE("Optimize `x === none' -> `x is none'\n");
     goto did_optimize;
    }
   }
   break;

  case AST_FACTION_DIFFOBJ:
   if (ast_optimize(self->ast_action.ast_act0,true)) goto err;
   if (ast_optimize(self->ast_action.ast_act1,true)) goto err;
   if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR &&
       self->ast_action.ast_act1->ast_type == AST_CONSTEXPR) {
#ifdef is_builtin_object
    /* Warn if the constant of either branch isn't a builtin object */
    if ((!is_builtin_object(self->ast_action.ast_act0->ast_constexpr) ||
         !is_builtin_object(self->ast_action.ast_act1->ast_constexpr)) &&
         warn_idcompare_nonbuiltin(self)) goto err;
#endif
    expr_result = DeeBool_For(self->ast_action.ast_act0->ast_constexpr !=
                              self->ast_action.ast_act1->ast_constexpr);
    Dee_Incref(expr_result);
    goto action_set_expr_result;
   } else if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR) {
#ifdef is_builtin_object
    /* Warn if the constant of `act0' isn't a builtin object */
    if (!is_builtin_object(self->ast_action.ast_act0->ast_constexpr) &&
         warn_idcompare_nonbuiltin(self)) goto err;
#endif
    if (DeeNone_Check(self->ast_action.ast_act0->ast_constexpr)) {
     /* Optimize: `none !== x' -> `x !is none' */
     DREF DeeAstObject *temp;
     temp = ast_setscope_and_ddi(ast_action2(AST_FACTION_IS,
                                             self->ast_action.ast_act1,
                                             self->ast_action.ast_act0),
                                 self);
     if unlikely(!temp) goto err;
     Dee_DecrefNokill(self->ast_action.ast_act1);
     Dee_DecrefNokill(self->ast_action.ast_act0);
     self->ast_boolexpr = temp; /* Inherit reference. */
     self->ast_type     = AST_BOOL;
     self->ast_flag     = AST_FBOOL_NEGATE;
     OPTIMIZE_VERBOSE("Optimize `none !== x' -> `x !is none'\n");
     goto did_optimize;
    }
   } else if (self->ast_action.ast_act1->ast_type == AST_CONSTEXPR) {
#ifdef is_builtin_object
    /* Warn if the constant of `act1' isn't a builtin object */
    if (!is_builtin_object(self->ast_action.ast_act1->ast_constexpr) &&
         warn_idcompare_nonbuiltin(self)) goto err;
#endif
    if (DeeNone_Check(self->ast_action.ast_act0->ast_constexpr)) {
     /* Optimize: `x !== none' -> `x !is none' */
     DREF DeeAstObject *temp;
     temp = ast_setscope_and_ddi(ast_action2(AST_FACTION_IS,
                                             self->ast_action.ast_act0,
                                             self->ast_action.ast_act1),
                                 self);
     if unlikely(!temp) goto err;
     Dee_DecrefNokill(self->ast_action.ast_act0);
     Dee_DecrefNokill(self->ast_action.ast_act1);
     self->ast_boolexpr = temp; /* Inherit reference. */
     self->ast_type     = AST_BOOL;
     self->ast_flag     = AST_FBOOL_NEGATE;
     OPTIMIZE_VERBOSE("Optimize `x !== none' -> `x !is none'\n");
     goto did_optimize;
    }
   }
   break;


  default:
   /* Default case: optimize individual action branches. */
   switch (AST_FACTION_ARGC_GT(self->ast_flag)) {
   case 3:  if (ast_optimize(self->ast_action.ast_act2,true)) goto err; ATTR_FALLTHROUGH
   case 2:  if (ast_optimize(self->ast_action.ast_act1,true)) goto err; ATTR_FALLTHROUGH
   case 1:  if (ast_optimize(self->ast_action.ast_act0,true)) goto err; ATTR_FALLTHROUGH
   default: break;
   }
   break;
  }
 } break;

 case AST_SWITCH:
  if (ast_optimize(self->ast_switch.ast_expr,true)) goto err;
  if (ast_optimize(self->ast_switch.ast_block,false)) goto err;
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
   if (ast_optimize(iter->ao_expr,true))
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
  result = ast_optimize(self,result_used);
  if unlikely(result) break;
  /* Stop after the first pass if the `OPTIMIZE_FONEPASS' flag is set. */
  if (optimizer_flags&OPTIMIZE_FONEPASS) break;
  /* Keep optimizing the branch while stuff happens. */
 } while (old_value != optimizer_count);
 return result;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPTIMIZE_C */
