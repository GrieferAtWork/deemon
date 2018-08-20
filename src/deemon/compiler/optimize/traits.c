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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_TRAITS_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_TRAITS_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/list.h>
#include <deemon/tuple.h>
#include <deemon/string.h>
#include <deemon/hashset.h>
#include <deemon/cell.h>
#include <deemon/dict.h>
#include <deemon/roset.h>
#include <deemon/rodict.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/map.h>
#include <deemon/bool.h>
#include <deemon/module.h>
#include <deemon/int.h>
#include <deemon/super.h>
#include <deemon/error.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>

DECL_BEGIN

PRIVATE DeeTypeObject *generic_sequence_types[] = {
    &DeeList_Type,
    &DeeTuple_Type,
    &DeeString_Type,
    &DeeHashSet_Type,
    &DeeDict_Type,
    &DeeRoSet_Type,
    &DeeRoDict_Type,
};

PRIVATE bool DCALL
is_generic_sequence_type(DeeTypeObject *self) {
 size_t i;
 for (i = 0; i < COMPILER_LENOF(generic_sequence_types); ++i)
     if (generic_sequence_types[i] == self)
         return true;
 return false;
}


INTERN DeeTypeObject *DCALL
ast_predict_type(DeeAstObject *__restrict self) {
 ASSERT_OBJECT_TYPE(self,&DeeAst_Type);
 /* When AST type prediction is disabled, always indicate unpredictable ASTs. */
 if (optimizer_flags&OPTIMIZE_FNOPREDICT)
     return NULL;
 switch (self->ast_type) {
 case AST_CONSTEXPR:
  return Dee_TYPE(self->ast_constexpr);
 case AST_MULTIPLE:
  if (self->ast_flag == AST_FMULTIPLE_KEEPLAST) {
   if (!self->ast_multiple.ast_exprc)
        return &DeeNone_Type;
   return ast_predict_type(self->ast_multiple.ast_exprv[
                           self->ast_multiple.ast_exprc-1]);
  }
  if (self->ast_flag == AST_FMULTIPLE_TUPLE)
      return &DeeTuple_Type;
  if (self->ast_flag == AST_FMULTIPLE_LIST)
      return &DeeList_Type;
  if (self->ast_flag == AST_FMULTIPLE_SET)
      return &DeeHashSet_Type;
  if (self->ast_flag == AST_FMULTIPLE_DICT)
      return &DeeDict_Type;
  if (AST_FMULTIPLE_ISDICT(self->ast_flag))
      return &DeeMapping_Type;
  return &DeeSeq_Type; /* That's all we can guaranty. */
 case AST_LOOP:
 case AST_RETURN:
 case AST_YIELD:
 case AST_THROW:
 case AST_LOOPCTL:
 case AST_SWITCH:
 case AST_ASSEMBLY:
  return &DeeNone_Type;
#if 0
 case AST_TRY:
  /* TODO: Predictable, but only when the guard and all
   *       catch-handles share the same return type. */
  return ast_predict_type(self->ast_try.ast_guard);
#endif
 {
  DeeTypeObject *tt_type;
  DeeTypeObject *ff_type;
 case AST_CONDITIONAL:
  if (self->ast_flag&AST_FCOND_BOOL)
      return &DeeBool_Type;
  tt_type = self->ast_conditional.ast_tt ? ast_predict_type(self->ast_conditional.ast_tt) : &DeeNone_Type;
  ff_type = self->ast_conditional.ast_ff ? ast_predict_type(self->ast_conditional.ast_ff) : &DeeNone_Type;
  if (tt_type == ff_type) return tt_type;
 } break;

 case AST_SYM:
  /* Certain symbol classes always refer to specific object types. */
  SYMBOL_INPLACE_UNWIND_ALIAS(self->ast_sym);
  switch (SYMBOL_TYPE(self->ast_sym)) {
  case SYMBOL_TYPE_MODULE:
  case SYMBOL_TYPE_MYMOD:
   return &DeeModule_Type;
  case SYMBOL_TYPE_MYFUNC:
   return &DeeFunction_Type;
  {
   DeeBaseScopeObject *bscope;
  case SYMBOL_TYPE_ARG:
   /* The type of the varargs-argument is always a tuple!
    * This deduction is required to optimize:
    * >> local x = (...);
    * ASM:
    * >>     push varargs
    * >>     cast top, tuple
    * >>     pop  local @x
    * Into:
    * >>     push varargs
    * >>     pop  local @x
    */
   bscope = self->ast_scope->s_base;
   if (bscope->bs_flags & CODE_FVARARGS &&
       DeeBaseScope_IsArgVarArgs(bscope,self->ast_sym->s_symid))
       return &DeeTuple_Type;
  } break;
  default: break;
  }
  break;

 case AST_BOOL:
  return &DeeBool_Type;

 case AST_FUNCTION:
  return &DeeFunction_Type;

 case AST_OPERATOR:
  /* If the self-operator gets re-returned, it's type is the result type. */
  if (self->ast_operator.ast_exflag & AST_OPERATOR_FPOSTOP)
      return ast_predict_type(self->ast_operator.ast_opa);
  if (self->ast_operator.ast_exflag & AST_OPERATOR_FVARARGS)
      break; /* XXX: Special handling? */
  switch (self->ast_flag) {

  case OPERATOR_STR:
  case OPERATOR_REPR:
   return &DeeString_Type;

  case OPERATOR_COPY:
  case OPERATOR_DEEPCOPY:
   return ast_predict_type(self->ast_operator.ast_opa);

  case OPERATOR_DELITEM:
  case OPERATOR_DELATTR:
  case OPERATOR_DELRANGE:
   return &DeeNone_Type;

  {
   DeeTypeObject *predict;
  case OPERATOR_SIZE:
   predict = ast_predict_type(self->ast_operator.ast_opa);
   if (is_generic_sequence_type(predict))
       return &DeeInt_Type;
   if (predict == &DeeNone_Type)
       return &DeeNone_Type;
  } break;

  {
   DeeTypeObject *predict;
  case OPERATOR_INV:
  case OPERATOR_POS:
  case OPERATOR_NEG:
  case OPERATOR_ADD:
  case OPERATOR_SUB:
  case OPERATOR_MUL:
  case OPERATOR_DIV:
  case OPERATOR_MOD:
  case OPERATOR_SHL:
  case OPERATOR_SHR:
  case OPERATOR_AND:
  case OPERATOR_OR :
  case OPERATOR_XOR:
//case OPERATOR_POW:
   predict = ast_predict_type(self->ast_operator.ast_opa);
   if (predict == &DeeInt_Type)
       return &DeeInt_Type;
   if (predict == &DeeNone_Type)
       return &DeeNone_Type;
  } break;

  case OPERATOR_ASSIGN:
  case OPERATOR_MOVEASSIGN:
   return ast_predict_type(self->ast_operator.ast_opb);

   /* AST_FOP_GETATTR? */
   /* AST_FOP_CALL? */

  {
   DeeTypeObject *predict;
  case OPERATOR_EQ:
  case OPERATOR_NE:
  case OPERATOR_LO:
  case OPERATOR_LE:
  case OPERATOR_GR:
  case OPERATOR_GE:
   predict = ast_predict_type(self->ast_operator.ast_opa);
   if (predict == &DeeString_Type ||
       predict == &DeeTuple_Type ||
       predict == &DeeInt_Type ||
       predict == &DeeBool_Type ||
       predict == &DeeList_Type ||
       predict == &DeeRoSet_Type ||
       predict == &DeeHashSet_Type ||
       predict == &DeeCell_Type ||
       predict == &DeeRoDict_Type ||
       predict == &DeeDict_Type)
       return &DeeBool_Type;
   if (predict == &DeeNone_Type) {
    if (self->ast_flag == OPERATOR_EQ ||
        self->ast_flag == OPERATOR_NE)
        return &DeeBool_Type;
    return &DeeNone_Type;
   }
  } break;

  {
   DeeTypeObject *sequence_type;
  case OPERATOR_CONTAINS:
   sequence_type = ast_predict_type(self->ast_operator.ast_opa);
   if (is_generic_sequence_type(sequence_type))
       return &DeeBool_Type;
  } break;

  case OPERATOR_SETITEM:
  case OPERATOR_SETATTR:
   return ast_predict_type(self->ast_operator.ast_opc);

  case OPERATOR_SETRANGE:
   return ast_predict_type(self->ast_operator.ast_opd);

  {
   DeeTypeObject *sequence_type;
  case OPERATOR_GETITEM:
  case OPERATOR_GETRANGE:
   sequence_type = ast_predict_type(self->ast_operator.ast_opa);
   if (sequence_type == &DeeString_Type)
       return &DeeString_Type;
  } break;

  default: break;
  }
  break;

 case AST_ACTION:
  switch (self->ast_flag&AST_FACTION_KINDMASK) {
#define ACTION(x) case x & AST_FACTION_KINDMASK:

  ACTION(AST_FACTION_STORE)
   return ast_predict_type(self->ast_action.ast_act1);

  {
   DeeTypeObject *sequence_type;
  ACTION(AST_FACTION_IN)
   sequence_type = ast_predict_type(self->ast_action.ast_act1);
   if (is_generic_sequence_type(sequence_type))
       return &DeeBool_Type;
  } break;

  ACTION(AST_FACTION_AS)
  ACTION(AST_FACTION_SUPEROF)
   return &DeeSuper_Type;

  ACTION(AST_FACTION_RANGE)
   return &DeeSeq_Type;

  ACTION(AST_FACTION_PRINT)
  ACTION(AST_FACTION_PRINTLN)
   return &DeeNone_Type;

  ACTION(AST_FACTION_FPRINT)
  ACTION(AST_FACTION_FPRINTLN)
   return ast_predict_type(self->ast_action.ast_act0);

  ACTION(AST_FACTION_CELL0)
  ACTION(AST_FACTION_CELL1)
   return &DeeCell_Type;

  ACTION(AST_FACTION_TYPEOF)
  ACTION(AST_FACTION_CLASSOF)
   return &DeeType_Type;

  ACTION(AST_FACTION_IS)
  ACTION(AST_FACTION_ANY)
  ACTION(AST_FACTION_ALL)
  ACTION(AST_FACTION_BOUNDATTR)
  ACTION(AST_FACTION_SAMEOBJ)
  ACTION(AST_FACTION_DIFFOBJ)
   return &DeeBool_Type;

  ACTION(AST_FACTION_ASSERT)
  ACTION(AST_FACTION_ASSERT_M)
  return ast_predict_type(self->ast_action.ast_act0);


  default: break;
#undef ACTION
  }


 default:
  break;
 }
 return NULL;
}



INTERN bool DCALL
ast_has_sideeffects(DeeAstObject *__restrict self) {
 if (optimizer_flags&OPTIMIZE_FNOPREDICT)
     return true;
 switch (self->ast_type) {
 case AST_CONSTEXPR:
 case AST_SYM:       /* `UnboundLocal' errors don't count as valid side-effects. */
 case AST_BOUND:
  return false;

 {
  DeeAstObject **iter,**end;
 case AST_MULTIPLE:
  /* No side-effects when no branch has any.
   * NOTE: This is true for all types of multiple-asts. */
  end = (iter = self->ast_multiple.ast_exprv)+
                self->ast_multiple.ast_exprc;
  for (; iter != end; ++iter) {
   if (ast_has_sideeffects(*iter)) return true;
  }
  return false;
 } break;

 case AST_CONDITIONAL:
  /* AST_CONDITIONAL (with none of the branches having side-effects) */
  return (ast_has_sideeffects(self->ast_conditional.ast_cond) ||
         (self->ast_conditional.ast_tt &&
          self->ast_conditional.ast_tt != self->ast_conditional.ast_cond &&
          ast_has_sideeffects(self->ast_conditional.ast_tt)) ||
         (self->ast_conditional.ast_ff &&
          self->ast_conditional.ast_ff != self->ast_conditional.ast_cond &&
          ast_has_sideeffects(self->ast_conditional.ast_ff)));

 /* TODO: `function' without references that could cause side-effects (aka. property refs) */

 case AST_ACTION:
  switch (self->ast_flag) {

  case AST_FACTION_CELL0:
   return false;

  case AST_FACTION_CELL1:
  case AST_FACTION_TYPEOF:
  case AST_FACTION_CLASSOF:
  case AST_FACTION_SUPEROF:
   return ast_has_sideeffects(self->ast_action.ast_act0);

  case AST_FACTION_IS:
  case AST_FACTION_AS:
  case AST_FACTION_BOUNDATTR:
  case AST_FACTION_SAMEOBJ:
  case AST_FACTION_DIFFOBJ:
   return (ast_has_sideeffects(self->ast_action.ast_act0) ||
           ast_has_sideeffects(self->ast_action.ast_act1));

  case AST_FACTION_RANGE:
   return (ast_has_sideeffects(self->ast_action.ast_act0) ||
           ast_has_sideeffects(self->ast_action.ast_act1) ||
           ast_has_sideeffects(self->ast_action.ast_act2));

  default: break;
  }
  break;

 default: break;
 }
 /* Fallback: anything we don't recognize explicitly has side-effects. */
 return true;
}


INTERN int DCALL
ast_doesnt_return(DeeAstObject *__restrict self,
                  unsigned int flags) {
 int temp;
 switch (self->ast_type) {

  /* Some normal branches that always return normally. */
 case AST_CONSTEXPR:
 case AST_SYM:
 case AST_UNBIND:
 case AST_BOUND:
 case AST_FUNCTION: /* Function definitions always return normally. */
  goto does_return;

 case AST_OPERATOR_FUNC:
  if (!self->ast_operator_func.ast_binding)
       goto does_return;
  ATTR_FALLTHROUGH
 case AST_YIELD:
 case AST_BOOL:
 case AST_EXPAND:
  /* Simple single-branch wrappers that can return normally. */
  return ast_doesnt_return(self->ast_yieldexpr,flags);

 {
  size_t i;
  bool has_noreturn;
 case AST_CLASS:
  has_noreturn = false;
  for (i = 0; i < 5; ++i) {
   if (!(&self->ast_class.ast_base)[i]) continue;
   temp = ast_doesnt_return(self->ast_operator_ops[i],flags);
   if (temp != 0) { /* doesn't return, or is unpredictable. */
    if (temp < 0) return temp;
    has_noreturn = true;
   }
  }
  /* Check class members. */
  for (i = 0; i < self->ast_class.ast_memberc; ++i) {
   temp = ast_doesnt_return(self->ast_class.ast_memberv[i].cm_ast,flags);
   if (temp != 0) { /* doesn't return, or is unpredictable. */
    if (temp < 0) return temp;
    has_noreturn = true;
   }
  }
  if (has_noreturn)
      goto doesnt_return;
  goto does_return;
 } break;

 {
  size_t i;
  bool has_noreturn;
 case AST_OPERATOR:
  /* Assume simple, ordered operator execution (a,[b,[c,[d]]]),
   * which is guarantied for any operator invocation. */
  has_noreturn = false;
  for (i = 0; i < 4; ++i) {
   if (!self->ast_operator_ops[i]) break;
   temp = ast_doesnt_return(self->ast_operator_ops[i],flags);
   if (temp != 0) { /* doesn't return, or is unpredictable. */
    if (temp < 0) return temp;
    has_noreturn = true;
   }
  }
  if (has_noreturn)
      goto doesnt_return;
  goto does_return;
 } break;

 case AST_ACTION:
  /* Actions behave similar to operators, but musn't necessarily follow
   * regular operator invocation behavior. That is why we keep of whitelist
   * of known action behavior here to determine if they can actually return. */
  switch (self->ast_flag & AST_FACTION_KINDMASK) {
#define ACTION(x) case x & AST_FACTION_KINDMASK:

  {
   size_t i;
   bool has_noreturn;
  ACTION(AST_FACTION_CELL0)
  ACTION(AST_FACTION_CELL1)
  ACTION(AST_FACTION_TYPEOF)
  ACTION(AST_FACTION_CLASSOF)
  ACTION(AST_FACTION_SUPEROF)
  ACTION(AST_FACTION_PRINT)
  ACTION(AST_FACTION_PRINTLN)
  ACTION(AST_FACTION_FPRINT)
  ACTION(AST_FACTION_FPRINTLN)
  ACTION(AST_FACTION_RANGE)
  ACTION(AST_FACTION_IS)
  ACTION(AST_FACTION_IN)
  ACTION(AST_FACTION_AS)
  ACTION(AST_FACTION_MIN)
  ACTION(AST_FACTION_MAX)
  ACTION(AST_FACTION_SUM)
  ACTION(AST_FACTION_ANY)
  ACTION(AST_FACTION_ALL)
  ACTION(AST_FACTION_STORE)    /* XXX: Should be fine for all cases, but technically doesn't follow normal rules, either... */
  ACTION(AST_FACTION_ASSERT)
//ACTION(AST_FACTION_ASSERT_M) /* Assertion-with-message doesn't follow normal rules. */
  ACTION(AST_FACTION_BOUNDATTR)
  ACTION(AST_FACTION_SAMEOBJ)
  ACTION(AST_FACTION_DIFFOBJ)
   /* Actions which follow regular operator execution rules. */
   has_noreturn = false;
   for (i = 0; i < (size_t)AST_FACTION_ARGC_GT(self->ast_flag); ++i) {
    temp = ast_doesnt_return(self->ast_operator_ops[i],flags);
    if (temp != 0) { /* doesn't return, or is unpredictable. */
     if (temp < 0) return temp;
     has_noreturn = true;
    }
   }
   if (has_noreturn)
       goto doesnt_return;
   goto does_return;
  } break;

  default: break;
#undef ACTION
  }
  break;


 {
  size_t i;
  bool has_noreturn;
 case AST_MULTIPLE:
  /* We never return if we contain another branch that doesn't
   * return, with no unpredictable branches anywhere. */
  has_noreturn = false;
  for (i = 0; i < self->ast_multiple.ast_exprc; ++i) {
   temp = ast_doesnt_return(self->ast_multiple.ast_exprv[i],flags);
   if (temp != 0) { /* doesn't return, or is unpredictable. */
    if (temp < 0) return temp;
    has_noreturn = true;
   }
  }
  if (has_noreturn)
      goto doesnt_return;
  goto does_return;
 } break;

 case AST_THROW:
  /* In a catch-all statement, a throw expression always returns. */
  if (self->ast_throwexpr) {
   temp = ast_doesnt_return(self->ast_throwexpr,flags);
   if (temp != 0) return temp; /* Unpredictable, or doesn't return */
  }
  if (flags & AST_DOESNT_RETURN_FINCATCHALL)
      goto does_return;
  /* If there are catch-expression, but not a catch-all one,
   * then we can't predict if this throw will return. */
  if (flags & AST_DOESNT_RETURN_FINCATCH)
      goto unpredictable;
  goto doesnt_return;

 case AST_LOOPCTL:
  /* When looking a a loop as a whole,
   * loop-control statements do actually return. */
  if (flags & AST_DOESNT_RETURN_FINLOOP)
      goto does_return;
  goto doesnt_return;

 case AST_RETURN:
  /* Check if the return-expression is predictable. */
  if (self->ast_returnexpr) {
   if (flags & AST_DOESNT_RETURN_FINCATCH &&
      !ast_is_nothrow(self->ast_returnexpr,true))
       goto does_return; /* The return-expression may throw an exception... */
   temp = ast_doesnt_return(self->ast_returnexpr,flags);
   if (temp != 0) {
    if (temp < 0) return temp;
    return temp; /* Unpredictable, or doesn't return */
   }
  }
  goto doesnt_return;

 {
  bool has_returning_handler;
  size_t i; struct catch_expr *vec;
 case AST_TRY:
  if unlikely(!self->ast_try.ast_catchc)
     return ast_doesnt_return(self->ast_try.ast_guard,flags);
  has_returning_handler = false;
  vec = self->ast_try.ast_catchv;
  flags &= ~(AST_DOESNT_RETURN_FINCATCH|
             AST_DOESNT_RETURN_FINCATCHALL);
  for (i = 0; i < self->ast_try.ast_catchc; ++i) {
   bool is_returning_handler = true;
   if (vec[i].ce_mask) {
    temp = ast_doesnt_return(vec[i].ce_mask,flags);
    if (temp < 0) return temp;
    if (temp) is_returning_handler = false;
   }
   /* Check if this handler ever returns. */
   temp = ast_doesnt_return(vec[i].ce_code,flags);
   if (temp < 0) return temp;
   if (temp) is_returning_handler = false;
   if (is_returning_handler)
       has_returning_handler = true;
   if (!(vec[i].ce_flags & EXCEPTION_HANDLER_FFINALLY)) {
    flags |= AST_DOESNT_RETURN_FINCATCH;
    if (!vec[i].ce_mask)
         flags |= AST_DOESNT_RETURN_FINCATCHALL;
   }
  }
  /* test the guarded expression. */
  temp = ast_doesnt_return(self->ast_try.ast_guard,flags);
  if (temp < 0) return temp;
  if (temp) {
   /* The guarded expression doesn't return.
    * If there are no handlers that ever return, the try-block doesn't either */
   if (!has_returning_handler)
        goto doesnt_return;
  }
  goto does_return;
 } break;

 {
  bool has_noreturn;
  struct text_label *iter;
 case AST_SWITCH:
  has_noreturn = false;
  temp = ast_doesnt_return(self->ast_switch.ast_expr,flags);
  if (temp < 0) return temp;
  if (temp) has_noreturn = true;
  flags |= AST_DOESNT_RETURN_FINLOOP; /* Switch overrides `break' */
  temp = ast_doesnt_return(self->ast_switch.ast_block,flags);
  if (temp < 0) return temp;
  if (temp) has_noreturn = true;
  /* Enumerate switch cases. */
  iter = self->ast_switch.ast_cases;
  for (; iter; iter = iter->tl_next) {
   temp = ast_doesnt_return(iter->tl_expr,flags);
   if (temp < 0) return temp;
   if (temp) has_noreturn = true;
  }
  if (has_noreturn)
      goto doesnt_return;
  goto does_return;
 } break;

 case AST_GOTO:
  /* The simple never-return-branches */
  goto doesnt_return;

 {
  size_t i;
  bool has_noreturn;
 case AST_ASSEMBLY:
  /* Inspect user-assembly operands. */
  has_noreturn = false;
  for (i = 0; i < self->ast_assembly.ast_num_i+
                  self->ast_assembly.ast_num_o; ++i) {
   temp = ast_doesnt_return(self->ast_assembly.ast_opv[i].ao_expr,flags);
   if (temp < 0) return temp;
   if (temp) has_noreturn = true;
  }
  if (has_noreturn) goto doesnt_return;
  if (self->ast_flag & AST_FASSEMBLY_REACH) {
   if (self->ast_flag & AST_FASSEMBLY_NORETURN)
       goto unpredictable_noreturn; /* doesn't return, but is reachable. */
   goto unpredictable; /* Reachable user-assembly is unpredictable. */
  }
  if (self->ast_flag & AST_FASSEMBLY_NORETURN)
      goto doesnt_return; /* If the user-assembly states that it doesn't return, then this ast doesn't either! */
  goto does_return;
 } break;

 case AST_CONDITIONAL:
  /* Simple case: If the condition doesn't return, neither
   *              do we if both conditions are predictable. */
  temp = ast_doesnt_return(self->ast_conditional.ast_cond,flags);
  if (temp) {
   if (temp < 0) return temp;
   if (self->ast_conditional.ast_tt) {
    temp = ast_doesnt_return(self->ast_conditional.ast_tt,flags);
    if (temp < 0) return temp;
   }
   if (self->ast_conditional.ast_ff) {
    temp = ast_doesnt_return(self->ast_conditional.ast_ff,flags);
    if (temp < 0) return temp;
   }
   goto doesnt_return;
  }
  /* Extended case: With both conditional branches existing,
   *                if neither returns, we don't either. */
  if (self->ast_conditional.ast_tt) {
   int temp2;
   temp = ast_doesnt_return(self->ast_conditional.ast_tt,flags);
   if (temp < 0) return temp;
   if (!self->ast_conditional.ast_ff)
        goto does_return; /* Without a false-branch, the AST can potentially return. */
   temp2 = ast_doesnt_return(self->ast_conditional.ast_ff,flags);
   if (temp2 < 0) return temp2; /* Check if the false-branch is unpredictable. */
   /* If both the true- and false-branches don't return, but the
    * condition does, then the entire AST won't return, either! */
   if (temp && temp2) goto doesnt_return;
   goto does_return;
  }
  if (self->ast_conditional.ast_ff) {
   temp = ast_doesnt_return(self->ast_conditional.ast_ff,flags);
   if (temp < 0) return temp;
   /* Without a true-branch, the AST can potentially return. */
   goto does_return;
  }
  break;

 default: break;
 }

 /* Default case: Any branch we don't recognize is unpredictable. */
unpredictable:
 return -1;
does_return:
 return 0;
doesnt_return:
 return 1;
unpredictable_noreturn:
 return -2;
}



INTERN bool DCALL
ast_is_nothrow(DeeAstObject *__restrict self, bool result_used) {
 switch (self->ast_type) {
 case AST_CONSTEXPR:
  goto is_nothrow;
 {
  struct symbol *sym;
 case AST_SYM:
  /* Access to some symbols is nothrow. */
  sym = self->ast_sym;
  SYMBOL_INPLACE_UNWIND_ALIAS(sym);
  /* Ref vars are static and never cause exceptions. */
  if (SYMBOL_MUST_REFERENCE(sym))
      goto is_nothrow;
  switch (SYMBOL_TYPE(sym)) {

  case SYMBOL_TYPE_STATIC:
   /* Since static variables can never be unbound,
    * accessing them can never cause any exceptions. */
   goto is_nothrow;

  case SYMBOL_TYPE_STACK:
   /* Stack-based symbols can never be unbound, so
    * accessing them is always a nothrow operation. */
   goto is_nothrow;

  case SYMBOL_TYPE_ARG:
   /* Accessing non-variadic & non-optional argument symbols is nothrow.
    * Only varargs themself can potentially cause exceptions
    * when accessed at runtime.
    * Additionally, the symbol must never be written to, because
    * if it is, it gets turned into a local variable, which _can_
    * cause exceptions when accessed. */
   if (SYMBOL_NWRITE(sym) == 0 &&
       DeeBaseScope_IsArgReqOrDefl(current_basescope,sym->s_symid))
       goto is_nothrow;
   break;

  case SYMBOL_TYPE_MODULE:
   /* Imported modules are static and never cause exceptions. */
   goto is_nothrow;

  case SYMBOL_TYPE_EXCEPT:
  case SYMBOL_TYPE_MYMOD:
  case SYMBOL_TYPE_MYFUNC:
  case SYMBOL_TYPE_THIS:
   /* These never cause exceptions, either. */
   goto is_nothrow;

  default: break;
  }
 } break;

 case AST_BOUND:
  /* Checking if a symbol is bound is nothrow. */
  goto is_nothrow;

 {
  size_t i;
 case AST_MULTIPLE:
  /* If the result is being used, and the AST is something other than
   * a keep-last-expression AST, then the sequence pack operation may
   * cause an exception. */
  if (result_used &&
      self->ast_flag != AST_FMULTIPLE_KEEPLAST)
      goto is_not_nothrow;
  /* If this is a keep-last expression, or the result isn't being used,
   * make sure that none of the contained expressions can cause exceptions. */
  for (i = 0; i < self->ast_multiple.ast_exprc; ++i) {
   if (!ast_is_nothrow(self->ast_multiple.ast_exprv[i],
                       result_used && i == self->ast_multiple.ast_exprc-1))
        goto is_not_nothrow;
  }
  goto is_nothrow;
 } break;

 case AST_RETURN:
 case AST_YIELD:
  return ast_is_nothrow(self->ast_returnexpr,true);

#if 0
 {
  size_t i;
 case AST_TRY:
  /* A try-block is nothrow if: the guarded block is nothrow,
   * or if a catch-all guard exists, who's handler is nothrow,
   * and all . */
  if (ast_is_nothrow(self->ast_try.ast_guard,result_used))
      goto is_nothrow;
  for (i = 0; i < self->ast_try.ast_catchc; ++i) {
   self->ast_try.ast_catchv[i].ce_code;
  }
 } break;
#endif

 default: break;
 }
is_not_nothrow:
 return false;
is_nothrow:
 return true;
}


INTERN int DCALL
ast_get_boolean(DeeAstObject *__restrict self) {
 /* NOTE: Assume that other operations on constant
  *       expressions have already been propagated. */
 if (self->ast_type == AST_CONSTEXPR &&
   !(optimizer_flags&OPTIMIZE_FNOPREDICT)) {
  int result = DeeObject_Bool(self->ast_constexpr);
  if unlikely(result < 0) DeeError_Handled(ERROR_HANDLED_RESTORE);
  return result;
 }
 return -1;
}
INTERN int DCALL
ast_get_boolean_noeffect(DeeAstObject *__restrict self) {
 int result;
 result = ast_get_boolean(self);
 if (result >= 0 && ast_has_sideeffects(self))
     result = -1;
 return result;
}


INTERN bool DCALL
ast_uses_symbol(DeeAstObject *__restrict self,
                struct symbol *__restrict sym) {
 if (optimizer_flags&OPTIMIZE_FNOPREDICT)
     goto yup;
 switch (self->ast_type) {
 case AST_SYM:
 case AST_UNBIND:
 case AST_BOUND:
  return self->ast_sym == sym;
 {
  DREF DeeAstObject **iter,**end;
 case AST_MULTIPLE:
  end = (iter = self->ast_multiple.ast_exprv)+
                self->ast_multiple.ast_exprc;
  for (; iter != end; ++iter) {
   if (ast_uses_symbol(*iter,sym))
       goto yup;
  }
 } break;

 {
  struct catch_expr *iter,*end;
 case AST_TRY:
  end = (iter = self->ast_try.ast_catchv)+
                self->ast_try.ast_catchc;
  for (; iter != end; ++iter) {
   if (iter->ce_mask &&
       ast_uses_symbol(iter->ce_mask,sym))
       goto yup;
   if (ast_uses_symbol(iter->ce_code,sym))
       goto yup;
  }
 }
  ATTR_FALLTHROUGH
 case AST_RETURN:
 case AST_YIELD:
 case AST_THROW:
 case AST_BOOL:
 case AST_EXPAND:
 case AST_FUNCTION:
  if (self->ast_returnexpr &&
      ast_uses_symbol(self->ast_returnexpr,sym))
      goto yup;
  break;
 {
  struct class_member *iter,*end;
 case AST_CLASS:
  SYMBOL_INPLACE_UNWIND_ALIAS(self->ast_class.ast_classsym);
  SYMBOL_INPLACE_UNWIND_ALIAS(self->ast_class.ast_supersym);
  if (self->ast_class.ast_classsym == sym ||
      self->ast_class.ast_supersym == sym)
      goto yup;
  if ((SYMBOL_TYPE(sym) == SYMBOL_TYPE_IFIELD ||
       SYMBOL_TYPE(sym) == SYMBOL_TYPE_CFIELD) &&
       SYMBOL_FIELD_CLASS(sym) == self->ast_class.ast_classsym)
       goto yup;
  end = (iter = self->ast_class.ast_memberv)+
                self->ast_class.ast_memberc;
  for (; iter != end; ++iter) {
   if (ast_uses_symbol(iter->cm_ast,sym))
       goto yup;
  }
  if (self->ast_class.ast_cmem &&
      ast_uses_symbol(self->ast_class.ast_cmem,sym)) goto yup;
 }
  ATTR_FALLTHROUGH
 case AST_OPERATOR:
  if (self->ast_operator.ast_opd &&
      ast_uses_symbol(self->ast_operator.ast_opd,sym))
      goto yup;
  ATTR_FALLTHROUGH
 case AST_LOOP:
 case AST_CONDITIONAL:
 case AST_ACTION:
  if (self->ast_loop.ast_elem && ast_uses_symbol(self->ast_loop.ast_elem,sym)) goto yup;
  ATTR_FALLTHROUGH
 case AST_SWITCH:
  if (self->ast_loop.ast_iter && ast_uses_symbol(self->ast_loop.ast_iter,sym)) goto yup;
  if (self->ast_loop.ast_loop && ast_uses_symbol(self->ast_loop.ast_loop,sym)) goto yup;
  break;

 {
  struct asm_operand *iter,*end;
 case AST_ASSEMBLY:
  /* Assembly branches with the `AST_FASSEMBLY_MEMORY'
   * flag set are assumed to use _any_ symbol. */
  if (self->ast_flag&AST_FASSEMBLY_MEMORY)
      goto yup;
  end = (iter = self->ast_assembly.ast_opv)+
               (self->ast_assembly.ast_num_i+
                self->ast_assembly.ast_num_o);
  for (; iter != end; ++iter) {
   if (ast_uses_symbol(iter->ao_expr,sym))
       goto yup;
  }
 } break;

 default: break;
 }
/*nope:*/
 return false;
yup:
 return true;
}

INTERN bool DCALL
ast_can_exchange(DeeAstObject *__restrict first,
                 DeeAstObject *__restrict second) {
 if (optimizer_flags&OPTIMIZE_FNOPREDICT)
     goto nope;
 /* Check if simple cases: When one of the branches
  * has absolutely no side-effects, then it never
  * matters in what order they appear. */
 if (first->ast_type == AST_CONSTEXPR ||
     second->ast_type == AST_CONSTEXPR)
     goto yup;
 /* If one of the asts is a symbol and the other branch
  * doesn't affect that symbol, then they can be exchanged
  * as well.
  * TODO: If both asts only read from the symbol, they could
  *       still be exchanged!
  */
 switch (first->ast_type) {
 case AST_SYM:
 case AST_BOUND:
 case AST_UNBIND:
  if (!ast_uses_symbol(second,first->ast_sym))
       goto yup;
  break;
 default: break;
 }
 switch (second->ast_type) {
 case AST_SYM:
 case AST_BOUND:
 case AST_UNBIND:
  if (!ast_uses_symbol(first,second->ast_sym))
       goto yup;
  break;
 default: break;
 }

nope:
 return false;
yup:
 return true;
}

PRIVATE bool DCALL
ast_equal_impl(DeeAstObject *__restrict a,
               DeeAstObject *__restrict b) {
 if (a->ast_type != b->ast_type) goto ne;
 if (a->ast_scope != b->ast_scope) {
  /* TODO: If the scopes aren't identical, the branches may still
   *       have the same meaning, dependent on the context:
   *    >> if (foo()) {
   *    >>     print x;
   *    >> } else {
   *    >>     // Different scope, but same meaning...
   *    >>     print x;
   *    >> }
   */
  goto ne;
 }
 switch (a->ast_type) {

 {
  int temp;
 case AST_CONSTEXPR:
  if (a->ast_constexpr == b->ast_constexpr) goto eq;
  if (Dee_TYPE(a->ast_constexpr) != Dee_TYPE(b->ast_constexpr)) goto ne;
  temp = DeeObject_CompareEq(a->ast_constexpr,b->ast_constexpr);
  if unlikely(temp < 0) DeeError_Handled(ERROR_HANDLED_RESTORE);
  return temp > 0;
 } break;

 case AST_SYM:
 case AST_UNBIND:
 case AST_BOUND:
  /* TODO: If the symbols aren't identical, the branches may still
   *       have the same meaning, dependent on the context:
   *    >> if (foo()) {
   *    >>     local x = 20;
   *    >>     print x;
   *    >> } else {
   *    >>     local x = 20; // Different symbol, but same meaning...
   *    >>     print x;
   *    >> }
   */
  return a->ast_sym == b->ast_sym;

 {
  size_t i;
 case AST_MULTIPLE:
  if (a->ast_flag != b->ast_flag) goto ne;
  if (a->ast_multiple.ast_exprc != b->ast_multiple.ast_exprc) goto ne;
  for (i = 0; i < a->ast_multiple.ast_exprc; ++i) {
   if (!ast_equal_impl(a->ast_multiple.ast_exprv[i],
                       b->ast_multiple.ast_exprv[i]))
        goto ne;
  }
  goto eq;
 } break;

 case AST_RETURN:
 case AST_THROW:
  if (!a->ast_returnexpr)
       return b->ast_returnexpr == NULL;
  if (!b->ast_returnexpr) goto ne;
  ATTR_FALLTHROUGH;
 case AST_YIELD:
  return ast_equal_impl(a->ast_returnexpr,
                        b->ast_returnexpr);

 {
  size_t i;
 case AST_TRY:
  if (a->ast_try.ast_catchc != b->ast_try.ast_catchc)
      goto ne;
  if (!ast_equal_impl(a->ast_try.ast_guard,
                      b->ast_try.ast_guard))
       goto ne;
  for (i = 0; i < a->ast_try.ast_catchc; ++i) {
   struct catch_expr *ahand = &a->ast_try.ast_catchv[i];
   struct catch_expr *bhand = &b->ast_try.ast_catchv[i];
   if (ahand->ce_flags != bhand->ce_flags) goto ne;
   if (ahand->ce_mask) {
    if (!bhand->ce_mask) goto ne;
    if (!ast_equal_impl(ahand->ce_mask,
                        bhand->ce_mask))
         goto ne;
   } else {
    if (bhand->ce_mask) goto ne;
   }

  }
 } break;

 case AST_LOOPCTL:
  if (a->ast_flag != b->ast_flag) goto ne;
  break;

 default: goto ne;
 }
eq:
 return true;
ne:
 return false;
}
INTERN bool DCALL
ast_equal(DeeAstObject *__restrict a,
          DeeAstObject *__restrict b) {
 if (optimizer_flags & OPTIMIZE_FNOCOMPARE) return false;
 if (a == b) return true;
 return ast_equal_impl(a,b);
}



/* Check if a given ast `self' is, or contains a `goto' branch,
 * or a `break' / `continue' branch when `consider_loopctl' is true.
 * NOTE: `goto' branches found in inner functions are not considered here! */
INTERN bool DCALL
ast_contains_goto(DeeAstObject *__restrict self,
                  uint16_t consider_loopctl) {
 switch (self->ast_type) {

 case AST_CONSTEXPR:
 case AST_SYM:
 case AST_UNBIND:
 case AST_BOUND:
 case AST_FUNCTION:
no:
  return false;

 {
  size_t i;
 case AST_MULTIPLE:
  for (i = 0; i < self->ast_multiple.ast_exprc; ++i) {
   if (ast_contains_goto(self->ast_multiple.ast_exprv[i],consider_loopctl))
       goto yes;
  }
  goto no;
 }

 case AST_RETURN:
 case AST_YIELD:
 case AST_THROW:
 case AST_BOOL:
 case AST_EXPAND:
 case AST_OPERATOR_FUNC:
  if (!self->ast_returnexpr) goto no;
  return ast_contains_goto(self->ast_returnexpr,consider_loopctl);

 {
  size_t i;
 case AST_TRY:
  if (ast_contains_goto(self->ast_try.ast_guard,consider_loopctl))
      goto yes;
  for (i = 0; i < self->ast_try.ast_catchc; ++i) {
   if (self->ast_try.ast_catchv[i].ce_mask &&
       ast_contains_goto(self->ast_try.ast_catchv[i].ce_mask,consider_loopctl))
       goto yes;
   if (ast_contains_goto(self->ast_try.ast_catchv[i].ce_code,consider_loopctl))
       goto yes;
  }
  goto no;
 } break;

 case AST_LOOP:
  if (self->ast_loop.ast_cond &&
      ast_contains_goto(self->ast_loop.ast_cond,AST_CONTAINS_GOTO_CONSIDER_NONE))
      goto yes;
  if (self->ast_loop.ast_next &&
      ast_contains_goto(self->ast_loop.ast_next,AST_CONTAINS_GOTO_CONSIDER_NONE))
      goto yes;
  if (self->ast_loop.ast_loop &&
      ast_contains_goto(self->ast_loop.ast_loop,AST_CONTAINS_GOTO_CONSIDER_NONE))
      goto yes;
  goto no;

 case AST_LOOPCTL:
  if (self->ast_flag == AST_FLOOPCTL_BRK
      ? (consider_loopctl & AST_CONTAINS_GOTO_CONSIDER_BREAK)
      : (consider_loopctl & AST_CONTAINS_GOTO_CONSIDER_CONTINUE))
      goto yes;
  goto no;

 case AST_CONDITIONAL:
  if (ast_contains_goto(self->ast_conditional.ast_cond,consider_loopctl))
      goto yes;
  if (self->ast_conditional.ast_tt &&
      self->ast_conditional.ast_tt != self->ast_conditional.ast_cond &&
      ast_contains_goto(self->ast_conditional.ast_tt,consider_loopctl))
      goto yes;
  if (self->ast_conditional.ast_ff &&
      self->ast_conditional.ast_ff != self->ast_conditional.ast_cond &&
      ast_contains_goto(self->ast_conditional.ast_ff,consider_loopctl))
      goto yes;
  goto no;

 case AST_OPERATOR:
  if (self->ast_operator.ast_opa) {
   if (ast_contains_goto(self->ast_operator.ast_opa,consider_loopctl))
       goto yes;
   if (self->ast_operator.ast_opb) {
    if (ast_contains_goto(self->ast_operator.ast_opb,consider_loopctl))
        goto yes;
    if (self->ast_operator.ast_opc) {
     if (ast_contains_goto(self->ast_operator.ast_opc,consider_loopctl))
         goto yes;
     if (self->ast_operator.ast_opd) {
      if (ast_contains_goto(self->ast_operator.ast_opd,consider_loopctl))
          goto yes;
     }
    }
   }
  }
  goto no;

 case AST_ACTION:
  switch (AST_FACTION_ARGC_GT(self->ast_flag)) {
  case 3: if (ast_contains_goto(self->ast_action.ast_act0,consider_loopctl)) goto yes;
  case 2: if (ast_contains_goto(self->ast_action.ast_act0,consider_loopctl)) goto yes;
  case 1: if (ast_contains_goto(self->ast_action.ast_act0,consider_loopctl)) goto yes;
  default: break;
  }
  goto no;

 {
  size_t i;
 case AST_CLASS:
  if (self->ast_class.ast_base &&
      ast_contains_goto(self->ast_class.ast_base,consider_loopctl))
      goto yes;
  if (self->ast_class.ast_name &&
      ast_contains_goto(self->ast_class.ast_name,consider_loopctl))
      goto yes;
  if (self->ast_class.ast_doc &&
      ast_contains_goto(self->ast_class.ast_doc,consider_loopctl))
      goto yes;
  if (self->ast_class.ast_imem &&
      ast_contains_goto(self->ast_class.ast_imem,consider_loopctl))
      goto yes;
  if (self->ast_class.ast_cmem &&
      ast_contains_goto(self->ast_class.ast_cmem,consider_loopctl))
      goto yes;
  for (i = 0; i < self->ast_class.ast_memberc; ++i) {
   if (ast_contains_goto(self->ast_class.ast_memberv[i].cm_ast,consider_loopctl))
       goto yes;
  }
  goto no;
 }

 {
  struct text_label *iter;
 case AST_SWITCH:
  /* Don't consider `break', which appears as part of the switch-branch! */
  consider_loopctl &= ~AST_CONTAINS_GOTO_CONSIDER_BREAK;
  if (ast_contains_goto(self->ast_switch.ast_expr,consider_loopctl))
      goto yes;
  if (ast_contains_goto(self->ast_switch.ast_block,consider_loopctl))
      goto yes;
  /* Don't forget to check the case expressions, too. */
  iter = self->ast_switch.ast_cases;
  for (; iter; iter = iter->tl_next) {
   if (ast_contains_goto(iter->tl_expr,consider_loopctl))
       goto yes;
  }
  goto no;
 }

 {
  size_t i;
 case AST_ASSEMBLY:
#if 0 /* Nope! - User-assembly should mark variables it uses manually!
       * >> local x = "foobar";
       * >> __asm__("" : "+x" (x)); // This prevents `x' from being optimized away! */
  if (self->ast_flag & AST_FASSEMBLY_NORETURN)
      goto yes;
#endif
  /* Check user-assembly operands. */
  for (i = 0; i <
       self->ast_assembly.ast_num_i +
       self->ast_assembly.ast_num_i; ++i) {
   if (ast_contains_goto(self->ast_assembly.ast_opv[i].ao_expr,consider_loopctl))
       goto yes;
  }
  goto no;
 }

 default: break;
 }
 /* fallback: Any branch we don't recognize may point to a GOTO branch.
  *        -> While we should be able to recognize everything, we still
  *           got to be as future-proof as possible! */
yes:
 return true;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_TRAITS_C */
