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
#include "genstore.c"
#define ENTER 1
#endif

#ifndef ENTER
#define LEAVE 1
#else
#undef  LEAVE
#endif

DECL_BEGIN

#ifdef ENTER
INTERN int (DCALL asm_gpop_expr_enter)(DeeAstObject *__restrict ast)
#else
INTERN int (DCALL asm_gpop_expr_leave)(DeeAstObject *__restrict ast, unsigned int gflags)
#define PUSH_RESULT  (gflags & ASM_G_FPUSHRES)
#endif
{
 switch (ast->ast_type) {

 case AST_SYM:
#ifdef LEAVE
  if (asm_putddi(ast)) goto err;
  if (PUSH_RESULT && asm_gdup()) goto err;
  return asm_gpop_symbol(ast->ast_sym,ast);
#endif
  break;

 case AST_MULTIPLE:
  if (ast->ast_flag == AST_FMULTIPLE_KEEPLAST) {
   /* Special case: Store to KEEP-last multiple AST:
    *               Evaluate all branches but the last without using them.
    *               Then simply write the value to the last expression. */
#ifdef ENTER
   size_t i = 0;
   if (ast->ast_multiple.ast_exprc == 0)
       goto done;
   if (ast->ast_multiple.ast_exprc > 1) {
    ASM_PUSH_SCOPE(ast->ast_scope,err);
    for (; i < ast->ast_multiple.ast_exprc-1; ++i) {
     if (ast_genasm(ast->ast_multiple.ast_exprv[i],ASM_G_FNORMAL))
         goto err;
    }
    ASM_POP_SCOPE(0,err);
   }
   ASSERT(i == ast->ast_multiple.ast_exprc-1);
   if (asm_gpop_expr_enter(ast->ast_multiple.ast_exprv[i])) goto err;
#else
   if (!ast->ast_multiple.ast_exprc) {
    if (PUSH_RESULT) goto done;
    return asm_gpop();
   }
   return asm_gpop_expr_leave(ast->ast_multiple.ast_exprv[
                              ast->ast_multiple.ast_exprc-1],
                              gflags);
#endif
  }
  {
   size_t i;
#ifdef ENTER
   for (i = 0; i < ast->ast_multiple.ast_exprc; ++i) {
    uint16_t old_stacksz = current_assembler.a_stackcur;
    DeeAstObject *inner = ast->ast_multiple.ast_exprv[i];
    if (asm_gpop_expr_enter(inner))
        goto err;
    ASSERT(current_assembler.a_stackcur >= old_stacksz);
    /* Remember how many stack-temporaries were produced by this entry. */
    inner->ast_temp = current_assembler.a_stackcur - old_stacksz;
   }
#else
   {
    uint16_t total_diff = 0;
    size_t count;
    /* Move the result below the block of stack-temporaries used by target-expressions. */
    count = ast->ast_multiple.ast_exprc;
    if (asm_putddi(ast)) goto err;
    if (asm_gunpack((uint16_t)count)) goto err;
    i = count;
    while (i--) total_diff += ast->ast_multiple.ast_exprv[i]->ast_temp;
    if (PUSH_RESULT) {
     if (asm_gdup()) goto err; /* <temp...>, result, result */
     if (total_diff != 0 && asm_grrot(total_diff + 2)) goto err; /* result, <temp...>, result */
    }
    /* Directly leave expressions that didn't use any stack-temporaries. */
    while (count && ast->ast_multiple.ast_exprv[count-1]->ast_temp == 0) {
     if (asm_gpop_expr_leave(ast->ast_multiple.ast_exprv[count-1],ASM_G_FNORMAL))
         goto err;
     --count;
    }
    if (count) {
     size_t j;
     /* Right now, the stack looks like this:
      * [result], T0a, T0b, T1a, T1b, T2a, T2b, V0, V1, V2
      * However, we want it to look like this:
      * [result], T0a, T0b, V0, T1a, T1b, V1, T2a, T2b, V2
      * Because of this, we must adjust the stack. */
     if (asm_grrot((uint16_t)count))
         goto err; /* T0a, T0b, T1a, T1b, T2a, T2b, V2, V0, V1 */
     /* -> asm_grrot(5); // T0a, T0b, T1a, T1b, V1, T2a, T2b, V2, V0 */
     /* -> asm_grrot(7); // T0a, T0b, V0, T1a, T1b, V1, T2a, T2b, V2 */
     i = count;
     while (i-- > 1) {
      uint16_t total = (uint16_t)count;
      for (j = i; j < count; ++j)
          total += ast->ast_multiple.ast_exprv[j]->ast_temp;
      if (asm_grrot(total)) goto err;
     }
     /* The leave-stack is unwound in reverse order! */
     i = count;
     while (i--) {
      if (asm_gpop_expr_leave(ast->ast_multiple.ast_exprv[i],ASM_G_FNORMAL))
          goto err;
     }
    }
   }
#endif
  }
  break;

 case AST_OPERATOR:
  switch (ast->ast_flag) {
  {
   DeeAstObject *attr;
  case OPERATOR_GETATTR:
   if unlikely((attr = ast->ast_operator.ast_opb) == NULL) break;
   if (attr->ast_type == AST_CONSTEXPR &&
       DeeString_Check(attr->ast_constexpr)) {
    int32_t cid = asm_newconst(attr->ast_constexpr);
    DeeAstObject *base = ast->ast_operator.ast_opa;
    if unlikely(cid < 0) goto err;
    if (base->ast_type == AST_SYM) {
     while (SYMBOL_TYPE(base->ast_sym) == SYMBOL_TYPE_ALIAS)
         base->ast_sym = SYMBOL_ALIAS(base->ast_sym);
     if (SYMBOL_TYPE(base->ast_sym) == SYMBOL_TYPE_THIS &&
        !SYMBOL_MUST_REFERENCE_TYPEMAY(base->ast_sym)) {
#ifdef LEAVE
      if (asm_putddi(ast)) goto err;
      if (PUSH_RESULT && asm_gdup()) goto err;
      if (asm_gsetattr_this_const((uint16_t)cid)) goto err;
#endif
      goto done;
     }
    }
#ifdef ENTER
    if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;
#else
    if (asm_putddi(ast)) goto err; /* base, value */
    if (PUSH_RESULT) {
     if (asm_gdup()) goto err;     /* base, value, value */
     if (asm_grrot(3)) goto err;   /* value, base, value */
    }
    if (asm_gsetattr_const((uint16_t)cid)) goto err;
#endif
    goto done;
   }
#ifdef ENTER
   if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(ast->ast_operator.ast_opb,ASM_G_FPUSHRES)) goto err;
#else
   if (asm_putddi(ast)) goto err;
   if (PUSH_RESULT) {
    if (asm_gdup()) goto err;     /* base, attr, value, value */
    if (asm_grrot(4)) goto err;   /* value, base, attr, value */
   }
   if (asm_gsetattr()) goto err;
#endif
   goto done;
  }


  {
   DeeAstObject *index;
  case OPERATOR_GETITEM:
   if unlikely((index = ast->ast_operator.ast_opb) == NULL) break;
#ifdef ENTER
   if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
#endif
   if (index->ast_type == AST_CONSTEXPR) {
    int32_t int_index;
    /* Special optimizations for constant indices. */
    if (DeeInt_Check(index->ast_constexpr) &&
        DeeInt_TryAsS32(index->ast_constexpr,&int_index) &&
        int_index >= INT16_MIN && int_index <= INT16_MAX) {
#ifdef LEAVE
     if (asm_putddi(ast)) goto err;
     if (PUSH_RESULT) {
      if (asm_gdup()) goto err;     /* base, value, value */
      if (asm_grrot(3)) goto err;   /* value, base, value */
     }
     if (asm_gsetitem_index((int16_t)int_index)) goto err;
#endif
     goto done;
    }
    if (asm_allowconst(index->ast_constexpr)) {
     int_index = asm_newconst(index->ast_constexpr);
     if unlikely(int_index < 0) goto err;
#ifdef LEAVE
     if (asm_putddi(ast)) goto err;
     if (PUSH_RESULT) {
      if (asm_gdup()) goto err;     /* base, value, value */
      if (asm_grrot(3)) goto err;   /* value, base, value */
     }
     if (asm_gsetitem_const((uint16_t)int_index)) goto err;
#endif
     goto done;
    }
   }
#ifdef ENTER
   if (ast_genasm(index,ASM_G_FPUSHRES)) goto err; /* STACK: base, index */
#else
   if (asm_putddi(ast)) goto err;
   if (PUSH_RESULT) {
    if (asm_gdup()) goto err;     /* base, index, value, value */
    if (asm_grrot(4)) goto err;   /* value, base, index, value */
   }
   if (asm_gsetitem()) goto err;
#endif
   goto done;
  }

  {
   DeeAstObject *begin,*end;
   int32_t index;
  case OPERATOR_GETRANGE:
   if unlikely(!ast->ast_operator.ast_opc) break;
#ifdef ENTER
   if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
#endif
   begin = ast->ast_operator.ast_opb;
   end   = ast->ast_operator.ast_opc;
   /* Special optimizations for certain ranges. */
   if (begin->ast_type == AST_CONSTEXPR) {
    DeeObject *begin_index = begin->ast_constexpr;
    if (DeeNone_Check(begin_index)) {
     /* Optimization: `setrange pop, none, [pop | $<Simm16>], pop' */
     if (end->ast_type == AST_CONSTEXPR &&
         DeeInt_Check(end->ast_constexpr) &&
         DeeInt_TryAsS32(end->ast_constexpr,&index) &&
         index >= INT16_MIN && index <= INT16_MAX) {
      /* `setrange pop, none, $<Simm16>, pop' */
#ifdef LEAVE
      if (asm_putddi(ast)) goto err;
      if (PUSH_RESULT) {
       if (asm_gdup()) goto err;     /* base, value, value */
       if (asm_grrot(3)) goto err;   /* value, base, value */
      }
      if (asm_gsetrange_ni((int16_t)index)) goto err;
#endif
      goto done;
     }
     /* `setrange pop, none, pop, pop' */
#ifdef ENTER
     if (ast_genasm(end,ASM_G_FPUSHRES)) goto err; /* STACK: base, end */
#else
     if (asm_putddi(ast)) goto err;
     if (PUSH_RESULT) {
      if (asm_gdup()) goto err;     /* base, end, value, value */
      if (asm_grrot(4)) goto err;   /* value, base, end, value */
     }
     if (asm_gsetrange_np()) goto err;
#endif
     goto done;
    }
    if (DeeInt_Check(begin_index) &&
        DeeInt_TryAsS32(begin_index,&index) &&
        index >= INT16_MIN && index <= INT16_MAX) {
     if (end->ast_type == AST_CONSTEXPR) {
      int32_t index2;
      DeeObject *end_index = end->ast_constexpr;
      if (DeeNone_Check(end_index)) {
       /* `setrange pop, $<Simm16>, none, pop' */
#ifdef LEAVE
       if (asm_putddi(ast)) goto err;
       if (PUSH_RESULT) {
        if (asm_gdup()) goto err;     /* base, value, value */
        if (asm_grrot(3)) goto err;   /* value, base, value */
       }
       if (asm_gsetrange_in((int16_t)index)) goto err;
#endif
       goto done;
      }
      if (DeeInt_Check(end_index) &&
          DeeInt_TryAsS32(end_index,&index2) &&
          index2 >= INT16_MIN && index2 <= INT16_MAX) {
       /* `setrange pop, $<Simm16>, $<Simm16>, pop' */
#ifdef LEAVE
       if (asm_putddi(ast)) goto err;
       if (PUSH_RESULT) {
        if (asm_gdup()) goto err;     /* base, value, value */
        if (asm_grrot(3)) goto err;   /* value, base, value */
       }
       if (asm_gsetrange_ii((int16_t)index,(int16_t)index2)) goto err;
#endif
       goto done;
      }
     }
#ifdef ENTER
     if (ast_genasm(end,ASM_G_FPUSHRES)) goto err; /* STACK: base, end */
#else
     if (asm_putddi(ast)) goto err;
     if (PUSH_RESULT) {
      if (asm_gdup()) goto err;     /* base, end, value, value */
      if (asm_grrot(4)) goto err;   /* value, base, end, value */
     }
     if (asm_gsetrange_ip((int16_t)index)) goto err;
#endif
     goto done;
    }
   } else if (end->ast_type == AST_CONSTEXPR) {
    /* Optimization: `setrange pop, pop, [none | $<Simm16>], pop' */
    DeeObject *end_index = end->ast_constexpr;
    int32_t index;
    if (DeeNone_Check(end_index)) {
     /* `setrange pop, pop, none, pop' */
#ifdef ENTER
     if (ast_genasm(begin,ASM_G_FPUSHRES)) goto err; /* STACK: base, begin */
#else
     if (asm_putddi(ast)) goto err;
     if (PUSH_RESULT) {
      if (asm_gdup()) goto err;     /* base, begin, value, value */
      if (asm_grrot(4)) goto err;   /* value, base, begin, value */
     }
     if (asm_gsetrange_pn()) goto err;
#endif
     goto done;
    }
    if (DeeInt_Check(end_index) &&
        DeeInt_TryAsS32(end_index,&index) &&
        index >= INT16_MIN && index <= INT16_MAX) {
     /* `setrange pop, pop, $<Simm16>, pop' */
#ifdef ENTER
     if (ast_genasm(begin,ASM_G_FPUSHRES)) goto err; /* STACK: base, begin */
#else
     if (asm_putddi(ast)) goto err;
     if (PUSH_RESULT) {
      if (asm_gdup()) goto err;     /* base, begin, value, value */
      if (asm_grrot(4)) goto err;   /* value, base, begin, value */
     }
     if (asm_gsetrange_pi((int16_t)index)) goto err;
#endif
     goto done;
    }
   }
#ifdef ENTER
   if (ast_genasm(begin,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(end,ASM_G_FPUSHRES)) goto err;
#else
   if (asm_putddi(ast)) goto err;
   if (PUSH_RESULT) {
    if (asm_gdup()) goto err;     /* base, begin, end, value, value */
    if (asm_grrot(5)) goto err;   /* value, base, begin, end, value */
   }
   if (asm_gsetrange()) goto err;
#endif
   goto done;
  }

  default: break;
  }
  goto default_case;

 case AST_CONSTEXPR:
  /* Check for special case: store into a constant
   * expression `none' is the same as `pop' */
  if (!DeeNone_Check(ast->ast_constexpr))
       goto default_case;
#ifdef LEAVE
  if (!PUSH_RESULT && asm_gpop()) goto err;
#endif
  break;

 default:
default_case:
  /* Fallback: Generate the ast and store it directly. */
#ifdef ENTER
  if (ast_genasm(ast,ASM_G_FPUSHRES)) goto err;
#else
  /* Emit a warning about an r-value store. */
  if (WARNAST(ast,W_ASM_STORE_TO_RVALUE))
      goto err;
  if (asm_putddi(ast)) goto err;
  if (PUSH_RESULT) {
   if (asm_gdup_n(0)) goto err; /* dst, value, dst */
   if (asm_gswap()) goto err;   /* dst, dst, value */
  }
  if (asm_gassign()) goto err;
#endif
  break;
 }
done:
 return 0;
err:
 return -1;
}

DECL_END

#undef PUSH_RESULT
#undef LEAVE
#undef ENTER

