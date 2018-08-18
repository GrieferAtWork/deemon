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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_CONDITIONAL_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_CONDITIONAL_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/bool.h>
#include <deemon/tuple.h>
#include <deemon/none.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>

DECL_BEGIN

INTERN int (DCALL ast_optimize_conditional)(struct ast_optimize_stack *__restrict stack,
                                            DeeAstObject *__restrict self, bool result_used) {
 int constant_condition;
 int tt_value,ff_value;
 ASSERT(self->ast_type == AST_CONDITIONAL);
 /* Obviously: optimize sub-branches and do dead-branch elimination. */
 /* TODO: Optimize the inner expressions in regards to
  *       them only being used as a boolean expression. */
 if (ast_optimize(stack,self->ast_conditional.ast_cond,true)) goto err;
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
#ifdef OPTIMIZE_FASSUME
 if (optimizer_flags & OPTIMIZE_FASSUME) {
  /* we're supposed to be making assumptions. */
  bool has_tt = self->ast_conditional.ast_tt && self->ast_conditional.ast_tt != self->ast_conditional.ast_cond;
  bool has_ff = self->ast_conditional.ast_ff && self->ast_conditional.ast_ff != self->ast_conditional.ast_cond;
  struct ast_assumes tt_assumes;
  struct ast_assumes ff_assumes;
  struct ast_optimize_stack child_stack;
  if (has_tt && has_ff) {
   if unlikely(ast_assumes_initcond(&tt_assumes,stack->os_assume))
      goto err;
   child_stack.os_prev   = stack;
   child_stack.os_assume = &tt_assumes;
   child_stack.os_ast    = self->ast_conditional.ast_tt;
   child_stack.os_used   = result_used;
   if unlikely(ast_optimize(&child_stack,child_stack.os_ast,result_used)) {
err_tt_assumes:
    ast_assumes_fini(&tt_assumes);
    goto err;
   }
   if unlikely(ast_assumes_initcond(&ff_assumes,stack->os_assume))
      goto err_tt_assumes;
   child_stack.os_prev   = stack;
   child_stack.os_assume = &ff_assumes;
   child_stack.os_ast    = self->ast_conditional.ast_ff;
   child_stack.os_used   = result_used;
   if unlikely(ast_optimize(&child_stack,child_stack.os_ast,result_used)) {
err_ff_tt_assumes:
    ast_assumes_fini(&ff_assumes);
    goto err_tt_assumes;
   }
   /* With true-branch and false-branch assumptions now made, merge them! */
   if unlikely(ast_assumes_mergecond(&tt_assumes,&ff_assumes))
      goto err_ff_tt_assumes;
   ast_assumes_fini(&ff_assumes);
  } else {
   ASSERT(has_tt || has_ff);
   if unlikely(ast_assumes_initcond(&tt_assumes,stack->os_assume))
      goto err;
   child_stack.os_prev   = stack;
   child_stack.os_assume = &tt_assumes;
   child_stack.os_ast    = has_tt
                         ? self->ast_conditional.ast_tt
                         : self->ast_conditional.ast_ff
                         ;
   child_stack.os_used   = result_used;
   if unlikely(ast_optimize(&child_stack,child_stack.os_ast,result_used))
      goto err_tt_assumes;
   /* Merge made assumptions with a NULL-branch, behaving
    * the same as a merge with an empty set of assumptions. */
   if unlikely(ast_assumes_mergecond(&tt_assumes,NULL))
      goto err_tt_assumes;
  }
  /* Finally, merge newly made assumptions onto those made by the caller. */
  if unlikely(ast_assumes_merge(stack->os_assume,&tt_assumes))
     goto err_tt_assumes;
  ast_assumes_fini(&tt_assumes);
 } else
#endif
 {
  if (self->ast_conditional.ast_tt &&
      self->ast_conditional.ast_tt != self->ast_conditional.ast_cond &&
      ast_optimize(stack,self->ast_conditional.ast_tt,result_used))
      goto err;
  if (self->ast_conditional.ast_ff &&
      self->ast_conditional.ast_ff != self->ast_conditional.ast_cond &&
      ast_optimize(stack,self->ast_conditional.ast_ff,result_used))
      goto err;
 }
 /* Load the constant value of the condition as a boolean. */
 constant_condition = ast_get_boolean(self->ast_conditional.ast_cond);
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
  /* We can't optimize away the dead branch when it contains a label.
   * TODO: That's not true. - Just could do an unreachable-pass on the
   *       other branch that deletes all branches before the first label! */
  if (other_branch && ast_doesnt_return(other_branch,AST_DOESNT_RETURN_FNORMAL) < 0)
      goto after_constant_condition;
  if (!eval_branch) {
   /* No branch is being evaluated. - Just replace the branch with `none' */
   /* TODO: In relation to the `TODO: That's not true...': we'd have to assign
    *       `{ <everything_after_label_in(other_branch)>; none; }' instead,
    *       if `other_branch' contains a `label'. */
   ast_fini_contents(self);
   self->ast_type = AST_CONSTEXPR;
   self->ast_constexpr = Dee_None;
   Dee_Incref(Dee_None);
  } else if (eval_branch == self->ast_conditional.ast_cond) {
   /* Special case: The branch that is getting evaluated
    *               is referencing the condition. */
   /* TODO: In relation to the `TODO: That's not true...': we'd have to assign
    *       `{ __stack local _temp = <eval_branch>; <everything_after_label_in(other_branch)>; _temp; }'
    *       instead, if `other_branch' contains a `label'. */
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
   /* TODO: In relation to the `TODO: That's not true...': we'd have to assign
    *       `{ <ast_cond>; __stack local _temp = <eval_branch>;
    *          <everything_after_label_in(other_branch)>; _temp; }'
    *       instead, if `other_branch' contains a `label'. */
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
   elemv[1] = eval_branch; /* Inherit reference */
   /* Override (and inherit) this AST. */
   self->ast_type               = AST_MULTIPLE;
   self->ast_flag               = AST_FMULTIPLE_KEEPLAST;
   self->ast_multiple.ast_exprc = 2;
   self->ast_multiple.ast_exprv = elemv;
   Dee_Decref(other_branch);
  }
  OPTIMIZE_VERBOSE("Expanding conditional branch with constant condition\n");
  goto did_optimize;
 }
after_constant_condition:
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
     if unlikely(!tt_astc && !ff_astc)
        goto if_statement_branches_identical;
     if (!tt_astc) {
      /* Only the false-branches remain. */
      /* TODO */
     }
     if (!ff_astc) {
      /* Only the true-branches remain. */
      /* TODO */
     }
    }
   }
  }
 }
 return 0;
err:
 return -1;
did_optimize:
 ++optimizer_count;
 return 0;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_CONDITIONAL_C */
