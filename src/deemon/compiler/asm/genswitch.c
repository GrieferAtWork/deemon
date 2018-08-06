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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENSWITCH_C
#define GUARD_DEEMON_COMPILER_ASM_GENSWITCH_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/rodict.h>
#include <deemon/tuple.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/assembler.h>

#include "../../runtime/strings.h"

DECL_BEGIN


/* Assuming that the switch-expression is located ontop of the stack,
 * generate code to jump to `target' after popping said expression
 * when if equals `case_expr'. However if it doesn't, leave the stack
 * as it was upon entry. */
PRIVATE int DCALL
emit_runtime_check(DeeAstObject *__restrict ddi_ast,
                   DeeAstObject *__restrict case_expr,
                   struct asm_sym *__restrict target) {
 struct asm_sym *temp,*guard_begin,*guard_end;
 struct asm_exc *exc_hand;
 if unlikely((temp = asm_newsym()) == NULL) goto err;
 if unlikely((guard_begin = asm_newsym()) == NULL) goto err;
 if unlikely((guard_end = asm_newsym()) == NULL) goto err;
 if (asm_putddi(ddi_ast)) goto err;
 if (asm_gdup()) goto err;                           /* expr, expr */
 if (ast_genasm(case_expr,ASM_G_FPUSHRES)) goto err; /* expr, expr, case */
 /* This instruction must be protected by an exception
  * handler for NOT_IMPLEMENTED and TYPE errors that
  * are handled by jumping to the next check. */
 if (asm_putddi(ddi_ast)) goto err;
 asm_defsym(guard_begin);
 if (asm_gcmp_eq()) goto err;              /* expr, expr==case */
 asm_defsym(guard_end);
 if (asm_gjmp(ASM_JF,temp)) goto err;      /* if (!(expr == case)) goto temp; */
 asm_decsp(); /* Adjust for ASM_JF */
 if (asm_gpop()) goto err; /* Pop `expr' before jumping to `target' */
 if (asm_gjmp(ASM_JMP,target)) goto err;
 asm_incsp(); /* Revert the popping of `expr' after the jump. */
 asm_defsym(temp); /* Jump here when the expressions didn't match. */

 /* Create a primary exception handler for NOT_IMPLEMENTED errors. */
 if unlikely((exc_hand = asm_newexc()) == NULL) goto err;
 Dee_Incref(&DeeError_NotImplemented);
 exc_hand->ex_mask  = &DeeError_NotImplemented;
 exc_hand->ex_start = guard_begin;
 exc_hand->ex_end   = guard_end;
 exc_hand->ex_addr  = temp;
#if 0
 exc_hand->ex_stack = current_assembler.a_stackcur;
#endif
 exc_hand->ex_flags = EXCEPTION_HANDLER_FHANDLED;
 ++guard_begin->as_used;
 ++guard_end->as_used;
 ++temp->as_used;

 /* Create a secondary exception handler for TYPE-errors. */
 if unlikely((exc_hand = asm_newexc()) == NULL) goto err;
 Dee_Incref(&DeeError_TypeError);
 exc_hand->ex_mask  = &DeeError_TypeError;
 exc_hand->ex_start = guard_begin;
 exc_hand->ex_end   = guard_end;
 exc_hand->ex_addr  = temp;
#if 0
 exc_hand->ex_stack = current_assembler.a_stackcur;
#endif
 exc_hand->ex_flags = EXCEPTION_HANDLER_FHANDLED;
 ++guard_begin->as_used;
 ++guard_end->as_used;
 ++temp->as_used;

 return 0;
err:
 return -1;
}


/* Construct a tuple `(sym.IP,sym.SP)' for the given `sym' */
PRIVATE DREF DeeObject *DCALL
pack_target_tuple(struct asm_sym *__restrict sym) {
 DREF DeeObject *ri_ip;
 DREF DeeObject *ri_sp;
 DREF DeeObject *result;
 ri_ip = DeeRelInt_New(sym,0,RELINT_MODE_FADDR);
 if unlikely(!ri_ip) goto err;
 ri_sp = DeeRelInt_New(sym,0,RELINT_MODE_FSTCK);
 if unlikely(!ri_sp) goto err_ip;
 result = DeeTuple_NewUninitialized(2);
 if unlikely(!result) goto err_sp;
 DeeTuple_SET(result,0,ri_ip); /* Inherit reference. */
 DeeTuple_SET(result,1,ri_sp); /* Inherit reference. */
 return result;
err_sp:
 Dee_DecrefDokill(ri_sp);
err_ip:
 Dee_DecrefDokill(ri_ip);
err:
 return NULL;
}




INTERN int DCALL
ast_genasm_switch(DeeAstObject *__restrict ast) {
 struct asm_sym *old_break,*switch_break;
 struct text_label *constant_cases,**pcases,*cases;
 struct asm_sym *default_sym; size_t i,num_constants;
 uint16_t old_finflag; int temp;
 int32_t default_cid = -1,jumptable_cid = -1;
 ASSERT_OBJECT_TYPE(ast,&DeeAst_Type);
 ASSERT(ast->ast_type == AST_SWITCH);

 /* Allocate a new symbol for `break' being used
  * inside of a switch and set up contextual flags. */
 switch_break = asm_newsym();
 if unlikely(!switch_break) goto err;
 old_break = current_assembler.a_loopctl[ASM_LOOPCTL_BRK];
 current_assembler.a_loopctl[ASM_LOOPCTL_BRK] = switch_break;
 old_finflag = current_assembler.a_finflag;
 current_assembler.a_finflag |= ASM_FINFLAG_NOLOOP;

 /* Assemble text for the switch expression. */
 if (ast_genasm(ast->ast_switch.ast_expr,ASM_G_FPUSHRES))
     goto err;

 if (ast->ast_switch.ast_default) {
  /* The default label shouldn't be allocated yet, but
   * the specs don't state that a goto-AST can't jump
   * to a case label, meaning that we must allow a
   * pre-allocated symbol. */
  default_sym = ast->ast_switch.ast_default->tl_asym;
  if likely(!default_sym) {
   default_sym = asm_newsym();
   /* Save the default symbol to that the switch-block may initialize it. */
   ast->ast_switch.ast_default->tl_asym = default_sym;
  }
 } else {
  default_sym = asm_newsym();
 }
 if unlikely(!default_sym)
    goto err;

 /* Go through all the cases and collect those than can be considered
  * suitable candidates for constant expressions, while generating
  * text to compare cases that cannot be determined at compile-time
  * at runtime. */
 constant_cases = NULL,num_constants = 0;
 pcases = &ast->ast_switch.ast_cases;
 temp = 0;
 while ((cases = *pcases) != NULL) {
  struct asm_sym *case_sym;
  ASSERT_OBJECT_TYPE(cases->tl_expr,&DeeAst_Type);
  /* Allocate the symbol for every case (We'll need them all
   * eventually and this is a good spot to allocate them). */
  case_sym = cases->tl_asym;
  if likely(!case_sym) {
   case_sym = asm_newsym();
   if unlikely(!case_sym) goto err_cases;
   cases->tl_asym = case_sym;
  }

  /* NOTE: When the no-jmptab flag is set, act as though
   *       no constants at all were being allowed. */
  if (cases->tl_expr->ast_type == AST_CONSTEXPR &&
    !(ast->ast_flag&AST_FSWITCH_NOJMPTAB) &&
      asm_allowconst(cases->tl_expr->ast_constexpr)) {
   /* Constant case (collect these and handle them individually).
    * NOTE: Technically, we're reversing the order of constant expressions
    *       here, but that's OK, considering they don't actually have any
    *       side-effects. */
   *pcases = cases->tl_next;
   ++num_constants;
   cases->tl_next = constant_cases;
   constant_cases = cases;
   continue;
  }
  /* Generate a runtime check for this case. */
  if likely(!temp) {
   if unlikely(emit_runtime_check(ast,cases->tl_expr,case_sym))
      goto err_cases;
  }
continue_next_case:
  pcases = &cases->tl_next;
  continue;
err_cases:
  temp = -1;
  goto continue_next_case;
 }
 /* Re-append all of the constant cases at the end of the linked list of cases. */
 *pcases = constant_cases;
 ASSERT((constant_cases != NULL) ==
        (num_constants != 0));
 /* Check if something went wrong during creation of runtime cases. */
 if unlikely(temp) goto err;

 /* Now that all of the non-constant cases are out-of-the-way, we
  * can simply move on to all of the actually constant cases. */

 /* Special case: If there are only 1 or less constants, generate runtime checks for them, too.
  *               The additional overhead caused by generating a jump-table here would be
  *               greater than the benefits it would give us. */
 if (num_constants <= 1) {
/*use_runtime_checks:*/
  for (i = 0; i < num_constants; ++i) {
   if unlikely(emit_runtime_check(ast,constant_cases->tl_expr,
                                      constant_cases->tl_asym))
      goto err;
   constant_cases = constant_cases->tl_next;
  }
  ASSERT(!constant_cases);
  /* With all cases now in check, pop the expression and jump ahead to the default case. */
  if (asm_putddi(ast)) goto err;
  if (asm_gpop()) goto err; /* Pop the switch-expression. */
  /* Jump to the default case when no other was matched. */
  if (asm_gjmp(ASM_JMP,default_sym)) goto err;
  goto do_generate_block;
 }

#if 1
 /* Generate a jump table! */
 /* NOTE: Stack: ..., expr */

 {
  DREF DeeObject *jump_table;
  DREF DeeObject *default_target;
  jump_table = DeeRoDict_NewWithHint(num_constants);
  if unlikely(!jump_table) goto err;
  for (i = 0; i < num_constants; ++i) {
   DREF DeeObject *case_target;
   ASSERT_OBJECT_TYPE(constant_cases->tl_expr,&DeeAst_Type);
   ASSERT(constant_cases->tl_expr->ast_type == AST_CONSTEXPR);
   ASSERT(constant_cases->tl_asym);
   case_target = pack_target_tuple(constant_cases->tl_asym);
   if unlikely(!case_target) {
err_jump_table:
    Dee_Decref(jump_table);
    goto err;
   }
   temp = DeeRoDict_Insert(&jump_table,
                            constant_cases->tl_expr->ast_constexpr,
                            case_target);
   Dee_Decref_unlikely(case_target);
   if unlikely(temp) goto err_jump_table;
   constant_cases = constant_cases->tl_next;
  }
  /* With the jump table now generated, generate code like this:
   * >>    push     const @<jump_table>            // expr, jump_table
   * >>    swap                                    // jump_table, expr
   * >>    push     const @(default.IP,default.SP) // jump_table, expr, default
   * >>    callattr top, @"get", #2                // target
   * >>    unpack   pop, #2                        // target.IP, target.SP
   * >>    jmp      pop, #pop */                   // ...
  if (asm_allowconst(jump_table)) {
   jumptable_cid = asm_newconst(jump_table);
   if unlikely(jumptable_cid < 0) goto err_jump_table;
   if (asm_gpush_const((uint16_t)jumptable_cid)) goto err_jump_table;
  } else {
   if (asm_gpush_constexpr(jump_table))
       goto err_jump_table;
  }
  Dee_Decref_unlikely(jump_table);                      /* expr, jump_table */
  if (asm_gswap()) goto err;                            /* jump_table, expr */
  default_target = pack_target_tuple(default_sym);
  if unlikely(!default_target) goto err;
  default_cid = asm_newconst(default_target);
  Dee_Decref_unlikely(default_target);
  if unlikely(default_cid < 0) goto err;
  if (asm_gpush_const((uint16_t)default_cid)) goto err; /* jump_table, expr, default */
  default_cid = asm_newconst(&str_get);
  if unlikely(default_cid < 0) goto err;
  if (asm_gcallattr_const((uint16_t)default_cid,2)) goto err; /* target */
  if (asm_gunpack(2)) goto err; /* target.IP, target.SP */
  if (asm_gjmp_pop_pop()) goto err;
  /* XXX: Some way of optimizing for the case of all
   *      targets sharing the same stack indirection. */
 }
#else
 goto use_runtime_checks;
#endif

do_generate_block:
 /* With all the jump-code out of the way, generate the switch block. */
 if (ast_genasm(ast->ast_switch.ast_block,ASM_G_FNORMAL))
     goto err;

 /* If the user didn't define a default symbol,
  * it will jump where `break' is located at. */
 if (!ASM_SYM_DEFINED(default_sym))
      asm_defsym(default_sym);
 /* Define the switch break symbol and restore the assembler state. */
 asm_defsym(switch_break);
 ASSERT(current_assembler.a_finflag&ASM_FINFLAG_NOLOOP);
 current_assembler.a_finflag = old_finflag;
 current_assembler.a_loopctl[ASM_LOOPCTL_BRK] = old_break;

 if (jumptable_cid >= 0) {
  /* Check if all jump targets share the same stack depth,
   * and if they do, optimize the generated assembly:
   * OLD:
   * >>    push     const @<jump_table>            // expr, jump_table
   * >>    swap                                    // jump_table, expr
   * >>    push     const @(default.IP,default.SP) // jump_table, expr, default
   * >>    callattr top, @"get", #2                // target
   * >>    unpack   pop, #2                        // target.IP, target.SP
   * >>    jmp      pop, #pop                      // ...
   * NEW:
   * >>    push     const @<jump_table>            // expr, jump_table
   * >>    swap                                    // jump_table, expr
   * >>    push     const @default.IP              // jump_table, expr, default
   * >>    callattr top, @"get", #2                // target
   * >>    jmp      pop                            // ...
   */
  /* TODO */

 }

 return 0;
err:
 return -1;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENSWITCH_C */
