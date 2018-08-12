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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_C 1
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

PRIVATE DeeTypeObject *generic_sequence_types[] = {
    &DeeList_Type,
    &DeeTuple_Type,
    &DeeString_Type,
    &DeeHashSet_Type,
    &DeeCell_Type,
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
#ifndef CONFIG_LANGUAGE_NO_ASM
 case AST_ASSEMBLY:
#endif
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
  /* Certain symbol classes alawys refer to specific object types. */
  switch (self->ast_sym->sym_class) {
  case SYM_CLASS_MODULE:
  case SYM_CLASS_THIS_MODULE:
   return &DeeModule_Type;
  case SYM_CLASS_THIS_FUNCTION:
   return &DeeFunction_Type;
  {
   DeeBaseScopeObject *bscope;
  case SYM_CLASS_ARG:
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
       DeeBaseScope_IsArgVarArgs(bscope,self->ast_sym->sym_arg.sym_index))
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
 case AST_BNDSYM:
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
 case AST_BNDSYM:
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

#ifndef CONFIG_LANGUAGE_NO_ASM
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
#endif

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
  switch (sym->sym_class) {

  case SYM_CLASS_VAR:
   /* Since static variables can never be unbound,
    * accessing them can never cause any exceptions. */
   if ((sym->sym_flag & SYM_FVAR_MASK) == SYM_FVAR_STATIC)
        goto is_nothrow;
   /* NOTE: Due to debugger interference, other var-symbols
    *       _can_ actually cause exceptions. */
   break;

  case SYM_CLASS_STACK:
   /* Stack-based symbols can never be unbound, so
    * accessing them is always a nothrow operation. */
   goto is_nothrow;

  case SYM_CLASS_ARG:
   /* Accessing non-variadic & non-optional argument symbols is nothrow.
    * Only varargs themself can potentially cause exceptions
    * when accessed at runtime.
    * Additionally, the symbol must never be written to, because
    * if it is, it gets turned into a local variable, which _can_
    * cause exceptions when accessed. */
   if (sym->sym_write == 0 &&
       DeeBaseScope_IsArgReqOrDefl(current_basescope,sym->sym_arg.sym_index))
       goto is_nothrow;
   break;

  case SYM_CLASS_REF:
   /* Ref vars are static and never cause exceptions. */
   goto is_nothrow;

  case SYM_CLASS_MODULE:
   /* Imported modules are static and never cause exceptions. */
   goto is_nothrow;

  case SYM_CLASS_EXCEPT:
  case SYM_CLASS_THIS_MODULE:
  case SYM_CLASS_THIS_FUNCTION:
  case SYM_CLASS_THIS:
   /* These never cause exceptions, either. */
   goto is_nothrow;

  default: break;
  }
 } break;

 case AST_BNDSYM:
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
 case AST_BNDSYM:
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
  if (self->ast_class.ast_classsym == sym ||
      self->ast_class.ast_supersym == sym)
      goto yup;
  if (sym->sym_class == SYM_CLASS_MEMBER &&
      sym->sym_member.sym_class == self->ast_class.ast_classsym)
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

#ifndef CONFIG_LANGUAGE_NO_ASM
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
#endif /* !CONFIG_LANGUAGE_NO_ASM */

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
 case AST_BNDSYM:
 case AST_UNBIND:
  if (!ast_uses_symbol(second,first->ast_sym))
       goto yup;
  break;
 default: break;
 }
 switch (second->ast_type) {
 case AST_SYM:
 case AST_BNDSYM:
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
 case AST_BNDSYM:
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



INTDEF void DCALL ast_incwrite(DeeAstObject *__restrict self);
INTDEF void DCALL ast_decwrite(DeeAstObject *__restrict self);

INTDEF void DCALL ast_fini_contents(DeeAstObject *__restrict self);
INTERN int DCALL
ast_assign(DeeAstObject *__restrict self,
           DeeAstObject *__restrict other) {
 uint8_t buffer[(sizeof(DeeAstObject)-
                 COMPILER_OFFSETOF(DeeAstObject,ast_type))];
 /* Use a temporary buffer for the variable portion of the AST.
  * Until we actually assign `other' to `self', we initialize it as a shallow copy of `other'. */
#if defined(__INTELLISENSE__) || 1
 DeeAstObject *temp = COMPILER_CONTAINER_OF(buffer,DeeAstObject,ast_type);
#else
#define temp   COMPILER_CONTAINER_OF(buffer,DeeAstObject,ast_type)
#endif
 temp->ast_type = other->ast_type;
 temp->ast_flag = other->ast_flag;
 switch (other->ast_type) {
 case AST_LOOP:
  if (other->ast_flag&AST_FLOOP_FOREACH &&
      other->ast_loop.ast_elem) {
   /* For an explanation, see AST_ACTION:AST_FACTION_STORE */
   ast_incwrite(other->ast_loop.ast_elem);
  }
  temp->ast_loop.ast_cond = other->ast_loop.ast_cond;
  temp->ast_loop.ast_iter = other->ast_loop.ast_iter;
  temp->ast_loop.ast_loop = other->ast_loop.ast_loop;
  Dee_XIncref(other->ast_loop.ast_cond);
  Dee_XIncref(other->ast_loop.ast_iter);
  Dee_XIncref(other->ast_loop.ast_loop);
  break;

 case AST_OPERATOR:
  temp->ast_operator.ast_exflag = other->ast_operator.ast_exflag;
  goto do_subast_x4;

 {
  struct class_member *iter,*end,*dst;
 case AST_CLASS:
  end = (iter = other->ast_class.ast_memberv)+
                other->ast_class.ast_memberc;
  temp->ast_class.ast_memberc = other->ast_class.ast_memberc;
  dst = (struct class_member *)Dee_Malloc(temp->ast_class.ast_memberc*
                                          sizeof(struct class_member));
  if unlikely(!dst) goto err;
  temp->ast_class.ast_memberv = dst;
  /* Copy the member descriptor table. */
  for (; iter != end; ++iter,++dst) {
   *dst = *iter;
   Dee_Incref(dst->cm_ast);
  }
 }
do_subast_x4:
  temp->ast_operator.ast_opd = other->ast_operator.ast_opd;
  Dee_XIncref(temp->ast_operator.ast_opd);
  ATTR_FALLTHROUGH
 case AST_CONDITIONAL:
do_xcopy_3:
  temp->ast_operator.ast_opc = other->ast_operator.ast_opc;
  Dee_XIncref(temp->ast_operator.ast_opc);
  ATTR_FALLTHROUGH
 case AST_FUNCTION:
  temp->ast_operator.ast_opb = other->ast_operator.ast_opb;
  Dee_XIncref(temp->ast_operator.ast_opb);
  ATTR_FALLTHROUGH
 case AST_CONSTEXPR:
 case AST_RETURN:
 case AST_YIELD:
 case AST_THROW:
 case AST_BOOL:
 case AST_EXPAND:
  temp->ast_constexpr = other->ast_constexpr;
  Dee_XIncref(temp->ast_constexpr);
  break;

 case AST_ACTION:
  /* NOTE: Must fix the symbol effect counters, but ignore any reference underflow
   *       that will be fixed as soon as the given branch is destroyed.
   *       This _MUST_ always happen unless the caller uses this function
   *       improperly in order to duplicate a branch without the intent of
   *       eventually destroying the source branch. */
  if ((other->ast_flag&AST_FACTION_KINDMASK) ==
      (AST_FACTION_STORE&AST_FACTION_KINDMASK)) {
   ast_incwrite(other->ast_action.ast_act0);
  }
  goto do_xcopy_3;

 {
  DREF DeeAstObject **iter,**end,**dst;
 case AST_MULTIPLE:
  temp->ast_multiple.ast_exprc = other->ast_multiple.ast_exprc;
  end = (iter = other->ast_multiple.ast_exprv)+
                other->ast_multiple.ast_exprc;
  dst = (DREF DeeAstObject **)Dee_Malloc(temp->ast_multiple.ast_exprc*
                                         sizeof(DREF DeeAstObject *));
  if unlikely(!dst) goto err;
  temp->ast_multiple.ast_exprv = dst;
  for (; iter != end; ++iter,++dst) {
   *dst = *iter;
   Dee_Incref(*dst);
  }
 } break;

 {
  struct catch_expr *iter,*end,*dst;
 case AST_TRY:
  temp->ast_try.ast_guard = other->ast_try.ast_guard;
  temp->ast_try.ast_catchc = other->ast_try.ast_catchc;
  Dee_Incref(temp->ast_try.ast_guard);
  end = (iter = other->ast_try.ast_catchv)+
                other->ast_try.ast_catchc;
  dst = (struct catch_expr *)Dee_Malloc(temp->ast_try.ast_catchc*
                                        sizeof(struct catch_expr));
  if unlikely(!dst) goto err;
  temp->ast_try.ast_catchv = dst;
  for (; iter != end; ++iter,++dst) {
   *dst = *iter;
   Dee_XIncref(dst->ce_mask);
   Dee_Incref(dst->ce_code);
  }
 } break;

 case AST_SYM:
  ASSERT(other->ast_sym);
  temp->ast_sym = other->ast_sym;
  if (temp->ast_flag) {
   ASSERT(other->ast_sym->sym_write);
   ++temp->ast_sym->sym_write;
  } else {
   ASSERT(other->ast_sym->sym_read);
   ++temp->ast_sym->sym_read;
  }
  break;

 case AST_BNDSYM:
  ASSERT(other->ast_sym);
  ASSERT(other->ast_sym->sym_read);
  temp->ast_sym = other->ast_sym;
  ++temp->ast_sym->sym_read;
  break;

 case AST_GOTO:
  ASSERT(other->ast_goto.ast_label);
  ASSERT(other->ast_goto.ast_label->tl_goto);
  temp->ast_goto.ast_label = other->ast_goto.ast_label;
  ++other->ast_goto.ast_label->tl_goto;
  temp->ast_goto.ast_base = other->ast_goto.ast_base;
  Dee_Incref((DeeObject *)temp->ast_goto.ast_base);
  break;
 case AST_LABEL:
  ASSERT(other->ast_label.ast_label);
  temp->ast_label.ast_label = other->ast_label.ast_label;
  temp->ast_label.ast_base  = other->ast_label.ast_base;
  Dee_Incref((DeeObject *)temp->ast_label.ast_base);
  break;

// case AST_SWITCH: /* Use the default case... */
//  break;

 default:
  /* XXX: Couldn't we must always do a move-construction like this? */
  memcpy(buffer,&other->ast_type,sizeof(buffer));
  memset(&other->ast_type,0,sizeof(buffer));
  break;
 }
 if (self != other) {
  /* Copy over DDI information. */
  if (other->ast_ddi.l_file)
      TPPFile_Incref(other->ast_ddi.l_file);
  if (self->ast_ddi.l_file)
      TPPFile_Decref(self->ast_ddi.l_file);
  memcpy(&self->ast_ddi,&other->ast_ddi,sizeof(struct ast_loc));
 }
 /* Finalize the contents of the AST that's being assigned to. */
 ast_fini_contents(self);
 /* Copy our temporary buffer into the actual, new AST. */
 memcpy(&self->ast_type,buffer,sizeof(buffer));
 return 0;
err:
 return -1;
#undef temp
}
INTERN int DCALL
ast_graft_onto(DeeAstObject *__restrict self,
               DeeAstObject *__restrict other) {
 DREF DeeAstObject **elemv;
 if (self->ast_scope == other->ast_scope)
     return ast_assign(self,other);
 elemv = (DeeAstObject **)Dee_Malloc(1*sizeof(DREF DeeAstObject *));
 if unlikely(!elemv) return -1;
 elemv[0] = other;
 Dee_Incref(other);
 if (self != other) {
  /* Copy over DDI information. */
  if (other->ast_ddi.l_file)
      TPPFile_Incref(other->ast_ddi.l_file);
  if (self->ast_ddi.l_file)
      TPPFile_Decref(self->ast_ddi.l_file);
  memcpy(&self->ast_ddi,&other->ast_ddi,sizeof(struct ast_loc));
 }
 ast_fini_contents(self);
 /* Override the (currently) invalid ast `self'. */
 self->ast_type               = AST_MULTIPLE;
 self->ast_flag               = AST_FMULTIPLE_KEEPLAST;
 self->ast_multiple.ast_exprc = 1;
 self->ast_multiple.ast_exprv = elemv;
 return 0;
}


PRIVATE DeeAstObject *DCALL
ast_setscope_and_ddi(DeeAstObject *ast,
                     DeeAstObject *__restrict src) {
 if unlikely(!ast) return NULL;
 Dee_Incref(src->ast_scope);
 Dee_Decref(ast->ast_scope);
 /* Override the effective scope of the AST. */
 ast->ast_scope = src->ast_scope;
 /* Override the DDI information. */
 if (src->ast_ddi.l_file) TPPFile_Incref(src->ast_ddi.l_file);
 if (ast->ast_ddi.l_file) TPPFile_Decref(ast->ast_ddi.l_file);
 ast->ast_ddi = src->ast_ddi;
 return ast;
}

#if !defined(NDEBUG) && 1
#define HAVE_VERBOSE 1
#define V(...)       optimize_verbose(self,__VA_ARGS__)
#define VAT(ast,...) optimize_verbose(ast,__VA_ARGS__)
PRIVATE void
optimize_verbose(DeeAstObject *__restrict self, char const *format, ...) {
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
#else
#define V(...)       (void)0
#define VAT(ast,...) (void)0
#endif



PRIVATE DeeTypeObject *constant_types[] = {
    /* Non-object-sequence types that can be encoded using DEC type codes. */
    &DeeInt_Type,
    &DeeFloat_Type,
    &DeeString_Type,
    &DeeNone_Type,
    &DeeBool_Type,
    &DeeMemberTable_Type, /* Required for class declarations. */
    &DeeCode_Type,        /* Not really, but must count because code objects live in constant slots. */
    &DeeRelInt_Type       /* Required so-as to support constant relocations.
                           * Objects of this type don't actually show up */
};

#define CONSTEXPR_ILLEGAL 0 /* Use of this object is not allowed. */
#define CONSTEXPR_ALLOWED 1 /* Use of this object is allowed. */
#define CONSTEXPR_USECOPY 2 /* Use is allowed, but you must work with a deep copy. */


struct constexpr_frame {
    struct constexpr_frame *cf_prev; /* [0..1] Previous frame. */
    DeeObject              *cf_obj;  /* [1..1] The object being checked. */
};

#define CONSTEXPR_FRAME_BEGIN(obj) \
 do{ struct constexpr_frame _frame; \
     _frame.cf_prev   = constexpr_frames; \
     _frame.cf_obj    = (obj); \
     constexpr_frames = &_frame; \
 do
#define CONSTEXPR_FRAME_BREAK  constexpr_frames = _frame.cf_prev
#define CONSTEXPR_FRAME_END \
     __WHILE0; \
     constexpr_frames = _frame.cf_prev; \
 }__WHILE0

/* [lock(DeeCompiler_Lock)] */
PRIVATE struct constexpr_frame *constexpr_frames = NULL;
LOCAL bool DCALL
constexpr_onstack(DeeObject *__restrict self) {
 struct constexpr_frame *iter;
 for (iter = constexpr_frames; iter;
      iter = iter->cf_prev) {
  if (iter->cf_obj == self)
      return true;
 }
 return false;
}


INTERN bool DCALL
asm_allowconst(DeeObject *__restrict self) {
 size_t i; DeeTypeObject *type;
 type = Dee_TYPE(self);
 /* Allow some basic types. */
 for (i = 0; i < COMPILER_LENOF(constant_types); ++i)
     if (type == constant_types[i]) goto allowed;
 if (Dec_BuiltinID(self) != DEC_BUILTINID_UNKNOWN)
     goto allowed;
 if (type == &DeeTuple_Type) {
  /* Special case: Only allow tuples of constant expressions. */
  for (i = 0; i < DeeTuple_SIZE(self); ++i) {
   if (!asm_allowconst(DeeTuple_GET(self,i)))
        goto illegal;
  }
  goto allowed;
 }
 if (type == &DeeRoSet_Type) {
  /* Special case: Only allow read-only sets of constant expressions. */
  struct roset_item *iter,*end;
  iter = ((DeeRoSetObject *)self)->rs_elem;
  end  = iter + ((DeeRoSetObject *)self)->rs_mask+1;
  for (; iter < end; ++iter) {
   if (!iter->si_key) continue;
   if (!asm_allowconst(iter->si_key))
        goto illegal;
  }
  goto allowed;
 }
 if (type == &DeeRoDict_Type) {
  /* Special case: Only allow read-only dicts of constant expressions. */
  struct rodict_item *iter,*end;
  iter = ((DeeRoDictObject *)self)->rd_elem;
  end  = iter + ((DeeRoDictObject *)self)->rd_mask+1;
  for (; iter < end; ++iter) {
   if (!iter->di_key) continue;
   if (!asm_allowconst(iter->di_key))
        goto illegal;
   if (!asm_allowconst(iter->di_value))
        goto illegal;
  }
  goto allowed;
 }
illegal: return false;
allowed: return true;
}

/* Return true if the optimizer is allowed to perform
 * operations on/with a constant instance `self'.
 * @return: * : One of `CONSTEXPR_*'
 */
PRIVATE int DCALL
allow_constexpr(DeeObject *__restrict self) {
 size_t i; DeeTypeObject *type;
again0:
 type = Dee_TYPE(self);
 /* Whitelist! */
 /* Allow some basic types. */
 for (i = 0; i < COMPILER_LENOF(constant_types); ++i)
     if (type == constant_types[i]) goto allowed;
 /* Check for special wrapper objects. */
 if (type == &DeeObjMethod_Type) {
  /* ObjMethod objects cannot be encoded in in DEC files. */
  if (!DeeCompiler_Current->cp_options ||
     !(DeeCompiler_Current->cp_options->co_assembler & ASM_FNODEC))
       goto illegal;
  self = ((DeeObjMethodObject *)self)->om_self;
  goto again0;
 }
 if (type == &DeeSuper_Type) {
  int result = CONSTEXPR_ALLOWED,temp;
  temp = allow_constexpr((DeeObject *)DeeSuper_TYPE(self));
  if (temp == CONSTEXPR_ILLEGAL) goto illegal;
  if (temp == CONSTEXPR_USECOPY)
      result = CONSTEXPR_USECOPY;
  temp = allow_constexpr((DeeObject *)DeeSuper_SELF(self));
  if (temp == CONSTEXPR_ILLEGAL) goto illegal;
  if (temp == CONSTEXPR_USECOPY)
      result = CONSTEXPR_USECOPY;
  return result;
 }
 if (type == &DeeTuple_Type) {
  /* Allow tuples consisting only of other allowed types. */
  DeeObject **iter,**end;
  int result = CONSTEXPR_ALLOWED;
  end = (iter = DeeTuple_ELEM(self))+DeeTuple_SIZE(self);
  for (; iter != end; ++iter) {
   int temp = allow_constexpr(*iter);
   if (temp == CONSTEXPR_ILLEGAL) goto illegal;
   if (temp == CONSTEXPR_USECOPY)
       result = CONSTEXPR_USECOPY;
  }
  return result;
 }
 /* Allow list/dict/set. */
 if (type == &DeeList_Type) {
  bool must_copy = DeeObject_IsShared(self);
  /* Recursive GC-type. */
  if (constexpr_onstack(self)) goto usecopy;
  CONSTEXPR_FRAME_BEGIN(self) {
   DeeList_LockRead(self);
   for (i = 0; i < DeeList_SIZE(self); ++i) {
    int temp = allow_constexpr(DeeList_GET(self,i));
    if (temp == CONSTEXPR_ILLEGAL) {
     DeeList_LockEndRead(self);
     CONSTEXPR_FRAME_BREAK;
     goto illegal;
    }
    if (temp == CONSTEXPR_USECOPY)
        must_copy = true;
   }
   DeeList_LockEndRead(self);
  }
  CONSTEXPR_FRAME_END;
  if (must_copy)
      goto usecopy;
  goto allowed;
 }
 if (type == &DeeHashSet_Type) {
  bool must_copy = DeeObject_IsShared(self);
  /* Recursive GC-type. */
  if (constexpr_onstack(self)) goto usecopy;
  CONSTEXPR_FRAME_BEGIN(self) {
   DeeHashSetObject *me = (DeeHashSetObject *)self;
   DeeHashSet_LockRead(self);
   for (i = 0; i <= me->s_mask; ++i) {
    int temp;
    DeeObject *key = me->s_elem[i].si_key;
    if (!key) continue;
    temp = allow_constexpr(key);
    if (temp == CONSTEXPR_ILLEGAL) {
     DeeHashSet_LockEndRead(self);
     CONSTEXPR_FRAME_BREAK;
     goto illegal;
    }
    if (temp == CONSTEXPR_USECOPY)
        must_copy = true;
   }
   DeeHashSet_LockEndRead(self);
  }
  CONSTEXPR_FRAME_END;
  if (must_copy)
      goto usecopy;
  goto allowed;
 }
 if (type == &DeeDict_Type) {
  bool must_copy = DeeObject_IsShared(self);
  /* Recursive GC-type. */
  if (constexpr_onstack(self)) goto usecopy;
  CONSTEXPR_FRAME_BEGIN(self) {
   DeeDictObject *me = (DeeDictObject *)self;
   DeeDict_LockRead(self);
   for (i = 0; i <= me->d_mask; ++i) {
    int temp;
    DeeObject *key = me->d_elem[i].di_key;
    if (!key) continue;
    temp = allow_constexpr(key);
    if (temp == CONSTEXPR_USECOPY) must_copy = true;
    if (temp == CONSTEXPR_ILLEGAL ||
       (temp = allow_constexpr(me->d_elem[i].di_value)) == CONSTEXPR_ILLEGAL) {
     DeeDict_LockEndRead(self);
     CONSTEXPR_FRAME_BREAK;
     goto illegal;
    }
    if (temp == CONSTEXPR_USECOPY) must_copy = true;
   }
   DeeDict_LockEndRead(self);
  }
  CONSTEXPR_FRAME_END;
  if (must_copy) goto usecopy;
  goto allowed;
 }
 if (type == &DeeRoDict_Type) {
  /* Allow read-only dicts consisting only of other allowed types. */
  int temp,result = CONSTEXPR_ALLOWED; size_t i;
  DeeRoDictObject *me = (DeeRoDictObject *)self;
  for (i = 0; i <= me->rd_mask; ++i) {
   if (!me->rd_elem[i].di_key) continue;
   temp = allow_constexpr(me->rd_elem[i].di_key);
   if (temp == CONSTEXPR_ILLEGAL) goto illegal;
   if (temp == CONSTEXPR_USECOPY) result = CONSTEXPR_USECOPY;
   temp = allow_constexpr(me->rd_elem[i].di_value);
   if (temp == CONSTEXPR_ILLEGAL) goto illegal;
   if (temp == CONSTEXPR_USECOPY) result = CONSTEXPR_USECOPY;
  }
  return result;
 }
 if (type == &DeeRoSet_Type) {
  /* Allow read-only sets consisting only of other allowed types. */
  int temp,result = CONSTEXPR_ALLOWED; size_t i;
  DeeRoSetObject *me = (DeeRoSetObject *)self;
  for (i = 0; i <= me->rs_mask; ++i) {
   if (!me->rs_elem[i].si_key) continue;
   temp = allow_constexpr(me->rs_elem[i].si_key);
   if (temp == CONSTEXPR_ILLEGAL) goto illegal;
   if (temp == CONSTEXPR_USECOPY) result = CONSTEXPR_USECOPY;
  }
  return result;
 }
 /* Last check: There is a small hand full of constant objects that are always allowed. */
 if (Dec_BuiltinID(self) != DEC_BUILTINID_UNKNOWN)
     goto allowed;
illegal: return CONSTEXPR_ILLEGAL;
allowed: return CONSTEXPR_ALLOWED;
usecopy: return CONSTEXPR_USECOPY;
}

/* Check if a given object `type' is a type that implements a cast-constructor. */
PRIVATE bool DCALL
has_cast_constructor(DeeObject *__restrict type) {
 if (type == (DeeObject *)&DeeTuple_Type) goto yes;
 if (type == (DeeObject *)&DeeList_Type) goto yes;
 if (type == (DeeObject *)&DeeHashSet_Type) goto yes;
 if (type == (DeeObject *)&DeeDict_Type) goto yes;
 if (type == (DeeObject *)&DeeRoDict_Type) goto yes;
 if (type == (DeeObject *)&DeeRoSet_Type) goto yes;
 if (type == (DeeObject *)&DeeInt_Type) goto yes;
 if (type == (DeeObject *)&DeeString_Type) goto yes;
 return false;
yes:
 return true;
}



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


INTDEF DREF DeeObject *DCALL string_decode(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL string_encode(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);

INTDEF DREF DeeObject *DCALL DeeCodec_NormalizeName(DeeObject *__restrict name);
INTDEF unsigned int DCALL DeeCodec_GetErrorMode(char const *__restrict errors);
INTDEF DREF DeeObject *DCALL DeeCodec_DecodeIntern(DeeObject *__restrict self, DeeObject *__restrict name, unsigned int error_mode);
INTDEF DREF DeeObject *DCALL DeeCodec_EncodeIntern(DeeObject *__restrict self, DeeObject *__restrict name, unsigned int error_mode);

PRIVATE DREF DeeObject *DCALL
emulate_object_decode(DeeObject *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 /* Something like `"foo".encode("UTF-8")' can still be
  * optimized at compile-time, however `"foo".encode("hex")'
  * mustn't, because the codec is implemented externally */
 DeeObject *name; char *errors = NULL;
 unsigned int error_mode = STRING_ERROR_FSTRICT;
 if (DeeArg_Unpack(argc,argv,"o|s:decode",&name,&errors) ||
     DeeObject_AssertTypeExact(name,&DeeString_Type))
     goto err;
 if (errors) {
  error_mode = DeeCodec_GetErrorMode(errors);
  if unlikely(error_mode == (unsigned int)-1) goto err;
 }
 return DeeCodec_DecodeIntern(self,name,error_mode);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
emulate_object_encode(DeeObject *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 DeeObject *name; char *errors = NULL;
 unsigned int error_mode = STRING_ERROR_FSTRICT;
 if (DeeArg_Unpack(argc,argv,"o|s:encode",&name,&errors) ||
     DeeObject_AssertTypeExact(name,&DeeString_Type))
     goto err;
 if (errors) {
  error_mode = DeeCodec_GetErrorMode(errors);
  if unlikely(error_mode == (unsigned int)-1) goto err;
 }
 return DeeCodec_EncodeIntern(self,name,error_mode);
err:
 return NULL;
}


/* Returns `ITER_DONE' if the call isn't allowed. */
PRIVATE DREF DeeObject *DCALL
emulate_method_call(DeeObject *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 if (DeeObjMethod_Check(self)) {
  /* Must emulate encode() and decode() functions, so they don't
   * call into libcodecs, which should only be loaded at runtime!
   * However, builtin codecs are still allowed!
   * NOTE: Both `string' and `bytes' use the same underlying
   *       function in order to implement `encode' and `decode'! */
  dobjmethod_t method;
  method = ((DeeObjMethodObject *)self)->om_func;
  if (method == &string_encode)
      return emulate_object_encode(((DeeObjMethodObject *)self)->om_self,argc,argv);
  if (method == &string_decode)
      return emulate_object_decode(((DeeObjMethodObject *)self)->om_self,argc,argv);
 }
 return DeeObject_Call(self,argc,argv);
}

/* Returns `ITER_DONE' if the call isn't allowed. */
INTERN DREF DeeObject *DCALL
emulate_member_call(DeeObject *__restrict base,
                    DeeObject *__restrict name,
                    size_t argc, DeeObject **__restrict argv) {
#define NAME_EQ(x) \
       (DeeString_SIZE(name) == COMPILER_STRLEN(x) && \
        memcmp(DeeString_STR(name),x,sizeof(x)-sizeof(char)) == 0)
 if (DeeString_Check(base) || DeeBytes_Check(base)) {
  /* Same as the other call emulator: special
   * handling for (string|bytes).(encode|decode) */
  if (NAME_EQ("encode"))
      return emulate_object_encode(base,argc,argv);
  if (NAME_EQ("decode"))
      return emulate_object_decode(base,argc,argv);
 }
 return DeeObject_CallAttr(base,name,argc,argv);
}


INTERN int (DCALL ast_optimize)(DeeAstObject *__restrict self, bool result_used) {
 ASSERT_OBJECT_TYPE(self,&DeeAst_Type);
again:
 /* Check for interrupts to allow the user to stop optimization */
 if (DeeThread_CheckInterrupt()) goto err;
 while (self->ast_scope->s_prev &&          /* Has parent scope */
        self->ast_scope->s_prev->ob_type == /* Parent scope has the same type */
        self->ast_scope->ob_type &&         /* ... */
       !self->ast_scope->s_mapc &&          /* Current scope has no symbols */
       !self->ast_scope->s_del &&           /* Current scope has no deleted symbol */
       !self->ast_scope->s_stk) {           /* Current scope has no stack symbol */
  /* Use the parent's scope. */
  DeeScopeObject *new_scope;
  new_scope = self->ast_scope->s_prev;
  Dee_Incref(new_scope);
  Dee_Decref(self->ast_scope);
  self->ast_scope = new_scope;
  ++optimizer_count;
#if 0 /* This happens so often, logging it would just be spam... */
  VAT(self,"Inherit parent scope above empty child scope\n");
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
  ASSERT(sym->sym_read);
  /* Optimize constant, extern symbols. */
  if (sym->sym_class == SYM_CLASS_EXTERN &&
     (sym->sym_extern.sym_modsym->ss_flags & MODSYM_FCONSTEXPR)) {
   /* The symbol is allowed to be expanded at compile-time. */
   DREF DeeObject *symval; int error;
   DeeModuleObject *symmod;
   symmod = sym->sym_extern.sym_module;
   error  = DeeModule_RunInit((DeeObject *)symmod);
   if unlikely(error < 0) goto err;
   if (error == 0) {
    /* The module is not initialized. */
    ASSERT(sym->sym_extern.sym_modsym->ss_index <
           symmod->mo_globalc);
#ifndef CONFIG_NO_THREADS
    rwlock_read(&symmod->mo_lock);
#endif
    symval = symmod->mo_globalv[sym->sym_extern.sym_modsym->ss_index];
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
     ASSERT(sym->sym_read);
     --sym->sym_read; /* Trace read references. */
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
    VAT(*iter,is_unreachable ? "Delete unreachable AST\n"
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
   V("Reduce constant-expression multi-ast\n");
   goto did_optimize;
  }
after_multiple_constexpr:
  if (self->ast_flag == AST_FMULTIPLE_KEEPLAST) {
   switch (self->ast_multiple.ast_exprc) {
   case 1:
    if (self->ast_scope == self->ast_multiple.ast_exprv[0]->ast_scope) {
     /* We can simply inherit this single branch, simplifying the entire AST. */
     if (ast_assign(self,self->ast_multiple.ast_exprv[0])) goto err;
     V("Collapse single-branch multi-ast\n");
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
    V("Replace empty multi-ast with `none'\n");
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
     V("Optimize object-mask to catch-all\n");
     ++optimizer_count;
     Dee_Clear(iter->ce_mask);
    }
    /* Set the `CATCH_EXPR_FSECOND' flag for all noexcept catch-handlers. */
    if (!(iter->ce_mode & CATCH_EXPR_FSECOND) &&
          ast_is_nothrow(iter->ce_code,result_used) &&
       (!iter->ce_mask || ast_is_nothrow(iter->ce_mask,true))) {
     V("Optimize nothrow catch-handler\n");
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
     V("Removing constant-true loop condition\n");
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
      V("Unwinding loop only iterated once\n");
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
      V("Deleting loop never executed\n");
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
   V("Remove no-op conditional branch\n");
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
   V("Expanding conditional branch with constant condition\n");
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
     V("Flatten constant false-branch of boolean conditional tt-is-cond expression\n");
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
     V("Flatten constant true-branch of boolean conditional ff-is-cond expression\n");
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
    V("Apply matrix to constant tt/ff branches of boolean conditional expression\n");
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
    V("Remove conditional tt-branch without any side-effects\n");
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
    V("Remove conditional ff-branch without any side-effects\n");
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
    V("Use result-vectoring to reduce constant tt/ff branches of conditional expression\n");
    goto did_optimize;
   }
   if (optimizer_flags & OPTIMIZE_FCSE) {
    if (ast_equal(tt,ff)) {
     /* The true and false branches are identical. */
     DREF DeeAstObject **elemv;
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
     V("Flatten identical true/false branches\n");
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
#if 0 /* TODO: How should we deal with scopes? */
    if (tt->ast_type == AST_MULTIPLE && tt->ast_flag == AST_FMULTIPLE_KEEPLAST &&
        ff->ast_type == AST_MULTIPLE && ff->ast_flag == AST_FMULTIPLE_KEEPLAST) {
     size_t move_count = 0;
     while (move_count < tt->ast_multiple.ast_exprc &&
            move_count < ff->ast_multiple.ast_exprc) {
      DeeAstObject *tt_last = tt->ast_multiple.ast_exprv[tt->ast_multiple.ast_exprc-(move_count+1)];
      DeeAstObject *ff_last = ff->ast_multiple.ast_exprv[ff->ast_multiple.ast_exprc-(move_count+1)];
      if (!ast_equal(tt_last,ff_last)) break;
      ++move_count;
     }
     if (move_count) {
      /* TODO: Move the last `move_count' asts from both branches outside. */
     }
    }
#endif
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
   V("Propagating constant boolean expression\n");
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
    V("Remove unused bool cast\n");
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
   V("Removing unused label\n");
   goto did_optimize;
  }
  break;

 {
  unsigned int opcount; int temp;
  DREF DeeObject *operator_result;
 case AST_OPERATOR:
  /* Only optimize sub-branches, but don't propagate constants
   * if the branch has already been optimized before. */
  if (self->ast_operator.ast_exflag & AST_OPERATOR_FDONTOPT) {
   if (ast_optimize(self->ast_operator.ast_opa,true)) goto err;
   if (self->ast_operator.ast_opb) {
    if (ast_optimize(self->ast_operator.ast_opb,true)) goto err;
    if (self->ast_operator.ast_opc) {
     if (ast_optimize(self->ast_operator.ast_opc,true)) goto err;
     if (self->ast_operator.ast_opd &&
         ast_optimize(self->ast_operator.ast_opd,true)) goto err;
    }
   }
   break;
  }
  self->ast_operator.ast_exflag |= AST_OPERATOR_FDONTOPT;
  /* If the result isn't used, then we can delete the postop flag. */
  if (!result_used)
       self->ast_operator.ast_exflag &= ~(AST_OPERATOR_FPOSTOP);
  if (self->ast_operator.ast_exflag & AST_OPERATOR_FVARARGS) {
   /* TODO: Unknown varargs when their number can now be predicted. */
   break;
  }
  /* Since `objmethod' isn't allowed in constant expressions, but
   * since it is the gateway to all kinds of compiler optimizations,
   * such as `"foo".upper()' --> `"FOO"', as a special case we try
   * to bridge across the GETATTR operator invocation and try to
   * directly invoke the function when possible. */
  if (self->ast_flag == OPERATOR_CALL &&
      self->ast_operator.ast_opa &&  self->ast_operator.ast_opb &&
     !self->ast_operator.ast_opc && !self->ast_operator.ast_opd &&
      self->ast_operator.ast_opa->ast_type == AST_OPERATOR &&
      self->ast_operator.ast_opa->ast_flag == OPERATOR_GETATTR &&
      self->ast_operator.ast_opa->ast_operator.ast_opa &&
      self->ast_operator.ast_opa->ast_operator.ast_opb &&
     !self->ast_operator.ast_opa->ast_operator.ast_opc &&
     !self->ast_operator.ast_opa->ast_operator.ast_opd) {
   DeeAstObject *base = self->ast_operator.ast_opa->ast_operator.ast_opa;
   DeeAstObject *name = self->ast_operator.ast_opa->ast_operator.ast_opb;
   DeeAstObject *args = self->ast_operator.ast_opb;
   /* Optimize the attribute name and make sure it's a constant string. */
   if (ast_optimize(name,true)) goto err;
   if (name->ast_type == AST_CONSTEXPR && DeeString_Check(name->ast_constexpr)) {
    /* Optimize the base-expression and make sure it's constant. */
    if (ast_optimize(base,true)) goto err;
    if (base->ast_type == AST_CONSTEXPR) {
     /* Optimize the argument list and make sure it's a constant tuple. */
     if (ast_optimize(args,true)) goto err;
     if (args->ast_type == AST_CONSTEXPR && DeeTuple_Check(args->ast_constexpr)) {
      /* All right! everything has fallen into place, and this is
       * a valid candidate for <getattr> -> <call> optimization. */
      operator_result = emulate_member_call(base->ast_constexpr,
                                            name->ast_constexpr,
                                            DeeTuple_SIZE(args->ast_constexpr),
                                            DeeTuple_ELEM(args->ast_constexpr));
      if (operator_result == ITER_DONE)
          goto done; /* Call wasn't allowed. */
#ifdef HAVE_VERBOSE
      if (operator_result &&
          allow_constexpr(operator_result) != CONSTEXPR_ILLEGAL) {
       V("Reduce constant expression `%r.%k%r -> %r'\n",
          base->ast_constexpr,name->ast_constexpr,
          args->ast_constexpr,operator_result);
      }
#endif
      opcount = 2;
      goto set_operator_result;
     }
    }
   }
  }

  opcount = 1;
  if (ast_optimize(self->ast_operator.ast_opa,true)) goto err;
  if (self->ast_operator.ast_opb) {
   ++opcount;
   if (ast_optimize(self->ast_operator.ast_opb,true)) goto err;
   if (self->ast_operator.ast_opc) {
    ++opcount;
    if (ast_optimize(self->ast_operator.ast_opc,true)) goto err;
    if (self->ast_operator.ast_opd) {
     ++opcount;
     if (ast_optimize(self->ast_operator.ast_opd,true)) goto err;
    }
   }
  }
  /* Invoke the specified operator. */
  /* XXX: `AST_FOPERATOR_POSTOP'? */
  {
   DREF DeeObject *argv[4];
   unsigned int i = opcount;
   /* Check if we can do some constant propagation. */
   while (i--) {
    DeeObject *operand;
    if (self->ast_operator_ops[i]->ast_type != AST_CONSTEXPR)
        goto cleanup_operands;
    operand = self->ast_operator_ops[i]->ast_constexpr;
    /* Check if the operand can appear in constant expression. */
    temp = allow_constexpr(operand);
    if (temp == CONSTEXPR_ILLEGAL) {
cleanup_operands:
     for (++i; i < opcount; ++i) Dee_Decref(argv[i]);
     goto generic_operator_optimizations;
    }
    if (temp == CONSTEXPR_USECOPY) {
     operand = DeeObject_DeepCopy(operand);
     if unlikely(!operand) {
      DeeError_Handled(ERROR_HANDLED_RESTORE);
      goto cleanup_operands;
     }
    } else {
     Dee_Incref(operand);
    }
    argv[i] = operand;
   }
   /* Special handling when performing a call operation. */
   if (self->ast_flag == OPERATOR_CALL) {
    if (opcount != 2) goto not_allowed;
    if (!DeeTuple_Check(argv[1])) goto not_allowed;
    if unlikely(self->ast_operator.ast_exflag & AST_OPERATOR_FPOSTOP) {
     operator_result = DeeObject_Copy(argv[0]);
     if likely(operator_result) {
      DREF DeeObject *real_result;
      real_result = emulate_method_call(argv[0],
                                        DeeTuple_SIZE(argv[1]),
                                        DeeTuple_ELEM(argv[1]));
      if likely(real_result)
         Dee_Decref(real_result);
      else {
       Dee_Clear(operator_result);
      }
     }
    } else {
     operator_result = emulate_method_call(argv[0],
                                           DeeTuple_SIZE(argv[1]),
                                           DeeTuple_ELEM(argv[1]));
    }
    if (operator_result == ITER_DONE) {
not_allowed:
     for (i = 0; i < opcount; ++i)
          Dee_Decref(argv[i]);
     goto done;
    }
   } else if (self->ast_operator.ast_exflag & AST_OPERATOR_FPOSTOP) {
    /* Return a copy of the original operand. */
    operator_result = DeeObject_Copy(argv[0]);
    if likely(operator_result) {
     DREF DeeObject *real_result;
     real_result = DeeObject_InvokeOperator(argv[0],self->ast_flag,opcount-1,argv+1);
     if likely(real_result)
        Dee_Decref(real_result);
     else {
      Dee_Clear(operator_result);
     }
    }
   } else {
    operator_result = DeeObject_InvokeOperator(argv[0],self->ast_flag,opcount-1,argv+1);
   }
#ifdef HAVE_VERBOSE
   if (operator_result &&
       allow_constexpr(operator_result) != CONSTEXPR_ILLEGAL) {
    struct opinfo *info;
    info = Dee_OperatorInfo(Dee_TYPE(argv[0]),self->ast_flag);
    V("Reduce constant expression `%r.operator %s %R -> %r'\n",
      argv[0],info ? info->oi_uname : "?",
      self->ast_flag == OPERATOR_CALL && opcount == 2
    ? DeeObject_NewRef(argv[1])
    : DeeTuple_NewVector(opcount-1,argv+1),
      operator_result);
   }
#endif
   for (i = 0; i < opcount; ++i)
        Dee_Decref(argv[i]);
  }
  /* If the operator failed, don't do any propagation. */
set_operator_result:
  if unlikely(!operator_result) {
   DeeError_Handled(ERROR_HANDLED_RESTORE);
   goto generic_operator_optimizations;
  }
  /* Check result is allowed in constant expressions. */
  temp = allow_constexpr(operator_result);
  if (temp != CONSTEXPR_ALLOWED) {
   if (temp == CONSTEXPR_ILLEGAL) {
dont_optimize_operator:
    Dee_Decref(operator_result);
    goto generic_operator_optimizations;
   }
   /* Replace with a deep copy (if shared) */
   if (DeeObject_InplaceDeepCopy(&operator_result)) {
    DeeError_Handled(ERROR_HANDLED_RESTORE);
    goto dont_optimize_operator;
   }
  }

  /* Override this branch with a constant expression `operator_result' */
  while (opcount--) Dee_Decref(self->ast_operator_ops[opcount]);
  self->ast_type      = AST_CONSTEXPR;
  self->ast_flag      = AST_FNORMAL;
  self->ast_constexpr = operator_result;
  goto did_optimize;
generic_operator_optimizations:
  if (self->ast_flag == OPERATOR_CALL &&
      self->ast_operator.ast_opb &&
      self->ast_operator.ast_opb->ast_type == AST_MULTIPLE &&
      self->ast_operator.ast_opb->ast_multiple.ast_exprc == 1 &&
      self->ast_operator.ast_opa->ast_type == AST_CONSTEXPR) {
   /* Certain types of calls can be optimized away:
    * >> local x = list([10,20,30]); // Optimize to `x = [10,20,30]' */
   DeeObject *function = self->ast_operator.ast_opa->ast_constexpr;
   DeeAstObject *cast_expr = self->ast_operator.ast_opb->ast_multiple.ast_exprv[0];
   if (has_cast_constructor(function) &&
       ast_predict_type(cast_expr) == (DeeTypeObject *)function) {
    V("Discard no-op cast-style function call to %k\n",function);
    /* We can simply get rid of this function call! */
    if (ast_assign(self,cast_expr)) goto err;
    goto did_optimize;
   }
   /* TODO: Propagate explicit cast calls to underlying sequence types:
    * >> tuple([10,20,30]); // Optimize to `pack(10,20,30)' */
  }
  goto done;
 } break;

 {
  DREF DeeObject *expr_result;
 case AST_ACTION:
  /* TODO: The result-used parameter depends on what kind of action it is...
   * TODO: Optimize AST order of `AST_FACTION_IN' and `AST_FACTION_AS'.
   * TOOD: Do constant propagation when branches are known at compile-time. */
  switch (self->ast_flag) {

  case AST_FACTION_STORE:
   /* Check for store-to-none (which is a no-op that translates to the stored expression) */
   if (ast_optimize(self->ast_action.ast_act0,true)) goto err;
   if (self->ast_action.ast_act0->ast_type == AST_CONSTEXPR &&
       DeeNone_Check(self->ast_action.ast_act0->ast_constexpr)) {
    /* Optimize the source-expression with propagated result-used settings. */
    if (ast_optimize(self->ast_action.ast_act1,result_used)) goto err;
    V("Discarding store-to-none\n");
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
       V("Propagate constant result of action-expression\n");
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
     V("Optimize `none === x' -> `x is none'\n");
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
     V("Optimize `x === none' -> `x is none'\n");
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
     V("Optimize `none !== x' -> `x !is none'\n");
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
     V("Optimize `x !== none' -> `x !is none'\n");
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
#ifndef CONFIG_LANGUAGE_NO_ASM
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
#endif

 default: break;
 }
done:
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

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_C */
