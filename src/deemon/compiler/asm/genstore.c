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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENSTORE_C
#define GUARD_DEEMON_COMPILER_ASM_GENSTORE_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/module.h>
#include <deemon/string.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/assembler.h>

DECL_BEGIN

#define PUSH_RESULT  (gflags & ASM_G_FPUSHRES)

PRIVATE int DCALL
asm_gpush2_duplast(DeeAstObject *__restrict a,
                   DeeAstObject *__restrict b,
                   DeeAstObject *__restrict ddi_ast,
                   unsigned int gflags) {
 if (PUSH_RESULT) {
  if (ast_can_exchange(b,a)) {
   if (ast_genasm(b,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(a,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup_n(2-2)) goto err;
  } else if (current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) {
   if (ast_genasm(a,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(b,ASM_G_FPUSHRES)) goto err; /* a, b */
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup()) goto err;         /* a, b, b */
   if (asm_grrot(3)) goto err;       /* b, a, b */
  } else {
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gpush_none()) goto err;
   if (ast_genasm(a,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(b,ASM_G_FPUSHRES)) goto err; /* none, a, b */
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup()) goto err;         /* none, a, b, b */
   if (asm_gpop_n(4-2)) goto err;    /* b,    a, b */
  }
 } else {
  if (ast_genasm(a,ASM_G_FPUSHRES)) goto err;
  if (ast_genasm(b,ASM_G_FPUSHRES)) goto err;
  if (asm_putddi(ddi_ast)) goto err;
 }
 return 0;
err:
 return -1;
}

PRIVATE int DCALL
asm_gpush3_duplast(DeeAstObject *__restrict a,
                   DeeAstObject *__restrict b,
                   DeeAstObject *__restrict c,
                   DeeAstObject *__restrict ddi_ast,
                   unsigned int gflags) {
 if (PUSH_RESULT) {
  if (ast_can_exchange(c,b) && ast_can_exchange(c,a)) {
   if (ast_genasm(c,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(a,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(b,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup_n(3-2)) goto err;
  } else if (current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) {
   if (ast_genasm(a,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(b,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(c,ASM_G_FPUSHRES)) goto err; /* a, b, c */
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup()) goto err;         /* a, b, c, c */
   if (asm_grrot(4)) goto err;       /* c, a, b, c */
  } else {
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gpush_none()) goto err;
   if (ast_genasm(a,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(b,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(c,ASM_G_FPUSHRES)) goto err; /* none, a, b, c */
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup()) goto err;         /* none, a, b, c, c */
   if (asm_gpop_n(5-2)) goto err;    /* c,    a, b, c */
  }
 } else {
  if (ast_genasm(a,ASM_G_FPUSHRES)) goto err;
  if (ast_genasm(b,ASM_G_FPUSHRES)) goto err;
  if (ast_genasm(c,ASM_G_FPUSHRES)) goto err;
  if (asm_putddi(ddi_ast)) goto err;
 }
 return 0;
err:
 return -1;
}

PRIVATE int DCALL
asm_gpush4_duplast(DeeAstObject *__restrict a,
                   DeeAstObject *__restrict b,
                   DeeAstObject *__restrict c,
                   DeeAstObject *__restrict d,
                   DeeAstObject *__restrict ddi_ast,
                   unsigned int gflags) {
 if (PUSH_RESULT) {
  if (ast_can_exchange(d,c) &&
      ast_can_exchange(d,b) &&
      ast_can_exchange(d,a)) {
   if (ast_genasm(d,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(a,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(b,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(c,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup_n(4-2)) goto err;
  } else if (current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) {
   if (ast_genasm(a,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(b,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(c,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(d,ASM_G_FPUSHRES)) goto err; /* a, b, c, d */
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup()) goto err;         /* a, b, c, d, d */
   if (asm_grrot(5)) goto err;       /* d, a, b, c, d */
  } else {
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gpush_none()) goto err;
   if (ast_genasm(a,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(b,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(c,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(d,ASM_G_FPUSHRES)) goto err; /* none, a, b, c, d */
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup()) goto err;         /* none, a, b, c, d, d */
   if (asm_gpop_n(6-2)) goto err;    /* d,    a, b, c, d */
  }
 } else {
  if (ast_genasm(a,ASM_G_FPUSHRES)) goto err;
  if (ast_genasm(b,ASM_G_FPUSHRES)) goto err;
  if (ast_genasm(c,ASM_G_FPUSHRES)) goto err;
  if (ast_genasm(d,ASM_G_FPUSHRES)) goto err;
  if (asm_putddi(ddi_ast)) goto err;
 }
 return 0;
err:
 return -1;
}

INTDEF struct module_symbol *DCALL
get_module_symbol(DeeModuleObject *__restrict module,
                  DeeStringObject *__restrict name);

INTERN int DCALL
ast_gen_setattr(DeeAstObject *__restrict base,
                DeeAstObject *__restrict name,
                DeeAstObject *__restrict value,
                DeeAstObject *__restrict ddi_ast,
                unsigned int gflags) {
 if (name->ast_type == AST_CONSTEXPR &&
     DeeString_Check(name->ast_constexpr)) {
  int32_t cid;
  if (base->ast_type == AST_SYM) {
   struct symbol *sym = base->ast_sym;
check_base_symbol_class:
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
   if (!SYMBOL_MUST_REFERENCE(sym))
#endif
   {
    switch (SYMBOL_TYPE(sym)) {

    case SYM_CLASS_ALIAS:
     ASSERT(SYMBOL_TYPE(SYMBOL_ALIAS(sym)) != SYM_CLASS_ALIAS);
     sym = SYMBOL_ALIAS(sym);
     goto check_base_symbol_class;

    case SYM_CLASS_THIS:
     cid = asm_newconst(name->ast_constexpr);
     if unlikely(cid < 0) goto err;
     if (ast_genasm(value,ASM_G_FPUSHRES)) goto err;
     if (PUSH_RESULT && (asm_putddi(ddi_ast) || asm_gdup())) goto err;
     if (asm_gsetattr_this_const((uint16_t)cid)) goto err;
     goto done;
    {
     struct module_symbol *modsym; int32_t module_id;
    case SYM_CLASS_MODULE: /* module.attr --> pop extern ... */
     modsym = get_module_symbol(SYMBOL_MODULE_MODULE(sym),
                               (DeeStringObject *)name->ast_constexpr);
     if (!modsym) break;
     module_id = asm_msymid(sym);
     if unlikely(module_id < 0) goto err;
     /* Push an external symbol accessed through its module. */
     if (ast_genasm(value,ASM_G_FPUSHRES)) goto err;
     if (asm_putddi(ddi_ast)) goto err;
     if (PUSH_RESULT && asm_gdup()) goto err;
     if (asm_gpop_extern((uint16_t)module_id,modsym->ss_index)) goto err;
     goto done;
    } break;
    default:
     break;
    }
   }
  }
  if unlikely(asm_gpush2_duplast(base,value,ddi_ast,gflags))
     goto err;
  cid = asm_newconst(name->ast_constexpr);
  if unlikely(cid < 0) goto err;
  if (asm_gsetattr_const((uint16_t)cid)) goto err;
  goto done;
 }
 if unlikely(asm_gpush3_duplast(base,name,value,ddi_ast,gflags))
    goto err;
 if (asm_gsetattr()) goto err;
done:
 return 0;
err:
 return -1;
}

INTERN int DCALL
ast_gen_setitem(DeeAstObject *__restrict sequence,
                DeeAstObject *__restrict index,
                DeeAstObject *__restrict value,
                DeeAstObject *__restrict ddi_ast,
                unsigned int gflags) {
 if (index->ast_type == AST_CONSTEXPR) {
  int32_t int_index;
  /* Special optimizations for constant indices. */
  if (DeeInt_Check(index->ast_constexpr) &&
      DeeInt_TryAsS32(index->ast_constexpr,&int_index) &&
      int_index >= INT16_MIN && int_index <= INT16_MAX) {
   if unlikely(asm_gpush2_duplast(sequence,value,ddi_ast,gflags))
      goto err;
   if (asm_gsetitem_index((int16_t)int_index)) goto err;
   goto done;
  }
  if (asm_allowconst(index->ast_constexpr)) {
   int_index = asm_newconst(index->ast_constexpr);
   if unlikely(int_index < 0) goto err;
   if unlikely(asm_gpush2_duplast(sequence,value,ddi_ast,gflags))
      goto err;
   if (asm_gsetitem_const((uint16_t)int_index)) goto err;
   goto done;
  }
 }
 if unlikely(asm_gpush3_duplast(sequence,index,value,ddi_ast,gflags))
    goto err;
 if (asm_gsetitem()) goto err;
done:
 return 0;
err:
 return -1;
}

INTERN int DCALL
ast_gen_setrange(DeeAstObject *__restrict sequence,
                 DeeAstObject *__restrict begin,
                 DeeAstObject *__restrict end,
                 DeeAstObject *__restrict value,
                 DeeAstObject *__restrict ddi_ast,
                 unsigned int gflags) {
 int32_t index;
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
    if unlikely(asm_gpush2_duplast(sequence,value,ddi_ast,gflags))
       goto err;
    if (asm_gsetrange_ni((int16_t)index)) goto err;
    goto done;
   }
   /* `setrange pop, none, pop, pop' */
   if unlikely(asm_gpush3_duplast(sequence,end,value,ddi_ast,gflags))
      goto err;
   if (asm_gsetrange_np()) goto err;
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
     if unlikely(asm_gpush2_duplast(sequence,value,ddi_ast,gflags))
        goto err;
     if (asm_gsetrange_in((int16_t)index)) goto err;
     goto done;
    }
    if (DeeInt_Check(end_index) &&
        DeeInt_TryAsS32(end_index,&index2) &&
        index2 >= INT16_MIN && index2 <= INT16_MAX) {
     /* `setrange pop, $<Simm16>, $<Simm16>, pop' */
     if unlikely(asm_gpush2_duplast(sequence,value,ddi_ast,gflags))
        goto err;
     if (asm_gsetrange_ii((int16_t)index,(int16_t)index2)) goto err;
     goto done;
    }
   }
   if unlikely(asm_gpush3_duplast(sequence,end,value,ddi_ast,gflags))
      goto err;
   if (asm_gsetrange_ip((int16_t)index)) goto err;
   goto done;
  }
 } else if (end->ast_type == AST_CONSTEXPR) {
  /* Optimization: `setrange pop, pop, [none | $<Simm16>], pop' */
  DeeObject *end_index = end->ast_constexpr;
  int32_t index;
  if (DeeNone_Check(end_index)) {
   /* `setrange pop, pop, none, pop' */
   if unlikely(asm_gpush3_duplast(sequence,begin,value,ddi_ast,gflags))
      goto err;
   if (asm_gsetrange_pn()) goto err;
   goto done;
  }
  if (DeeInt_Check(end_index) &&
      DeeInt_TryAsS32(end_index,&index) &&
      index >= INT16_MIN && index <= INT16_MAX) {
   /* `setrange pop, pop, $<Simm16>, pop' */
   if unlikely(asm_gpush3_duplast(sequence,begin,value,ddi_ast,gflags))
      goto err;
   if (asm_gsetrange_pi((int16_t)index)) goto err;
   goto done;
  }
 }
 if unlikely(asm_gpush4_duplast(sequence,begin,end,value,ddi_ast,gflags))
    goto err;
 if (asm_gsetrange()) goto err;
done:
 return 0;
err:
 return -1;
}

INTERN int DCALL
asm_gunpack_expr(DeeAstObject *__restrict src,
                 uint16_t num_targets,
                 DeeAstObject *__restrict ddi_ast) {
 /* Unwind inner sequence-expand expressions.
  * This optimizes:
  * >> `(x,y,z) = [(10,20,30)...];'
  * Into:
  * >> `(x,y,z) = (10,20,30);'
  */
 for (;;) {
  DeeAstObject *inner;
  if (src->ast_type != AST_MULTIPLE) break;
  if (src->ast_flag == AST_FMULTIPLE_KEEPLAST) break;
  if (src->ast_multiple.ast_exprc != 1) break;
  inner = src->ast_multiple.ast_exprv[0];
  if (inner->ast_type != AST_EXPAND) break;
  src = inner->ast_expandexpr;
 }
 if (src->ast_type == AST_SYM) {
  struct symbol *sym = src->ast_sym;
  while (SYMBOL_TYPE(sym) == SYM_CLASS_ALIAS)
      sym = SYMBOL_ALIAS(sym);
  if (SYMBOL_TYPE(sym) == SYM_CLASS_ARG &&
     (current_basescope->bs_flags & CODE_FVARARGS) &&
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
     !SYMBOL_MUST_REFERENCE_TYPEMAY(sym) &&
#endif
     !DeeBaseScope_HasOptional(current_basescope) &&
      DeeBaseScope_IsArgVarArgs(current_basescope,SYMBOL_ARG_INDEX(sym))) {
   /* Unpack the varargs argument. */
   if (asm_putddi(ddi_ast)) goto err;
   return asm_gvarargs_unpack(num_targets);
  }
  if (asm_can_prefix_symbol_for_read(sym)) {
   /* The unpack instructions can make use of an object prefix. */
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gprefix_symbol(sym,src)) goto err;
   return asm_gunpack_p(num_targets);
  }
 }

 /* Fallback: generate regular assembly for `src' and use `ASM_UNPACK' */
 if (ast_genasm(src,ASM_G_FPUSHRES)) goto err;
 if (asm_putddi(ddi_ast)) goto err;
 return asm_gunpack(num_targets);
err:
 return -1;
}


INTERN int
(DCALL asm_gmov_sym_sym)(struct symbol *__restrict dst_sym,
                        struct symbol *__restrict src_sym,
                        DeeAstObject *__restrict dst_ast,
                        DeeAstObject *__restrict src_ast) {
 int32_t symid;
 ASSERT(asm_can_prefix_symbol(dst_sym));
check_src_sym_class:
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
 if (SYMBOL_MUST_REFERENCE(src_sym)) {
  /* mov PREFIX, ref <imm8/16> */
  symid = asm_rsymid(src_sym);
  if unlikely(symid < 0) goto err;
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_ref_p((uint16_t)symid);
 }
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */
 switch (SYMBOL_TYPE(src_sym)) {

 case SYM_CLASS_ALIAS:
  ASSERT(src_sym != SYMBOL_ALIAS(src_sym));
  src_sym = SYMBOL_ALIAS(src_sym);
  goto check_src_sym_class;

 case SYM_CLASS_STACK:
  /* mov PREFIX, #... */
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
  if (!(src_sym->s_flag & SYMBOL_FALLOC)) break;
#else
  if (!(src_sym->sym_flag & SYM_FSTK_ALLOC)) break;
#endif
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  if (SYMBOL_STACK_OFFSET(src_sym) == current_assembler.a_stackcur-1) {
   return asm_gdup_p();
  } else {
   uint16_t offset;
   offset = (uint16_t)((current_assembler.a_stackcur -
                        SYMBOL_STACK_OFFSET(src_sym))-1);
   return asm_gdup_n_p(offset);
  }
  break;

 case SYM_CLASS_EXCEPT:
  /* mov PREFIX, except */
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_except_p();

 case SYM_CLASS_MODULE:
  /* mov PREFIX, module <imm8/16> */
  symid = asm_msymid(src_sym);
  if unlikely(symid < 0) goto err;
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_module_p((uint16_t)symid);

 case SYM_CLASS_THIS:
  /* mov PREFIX, this */
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_this_p();

 case SYM_CLASS_THIS_MODULE:
  /* mov PREFIX, this_module */
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_this_module_p();

 case SYM_CLASS_THIS_FUNCTION:
  /* mov PREFIX, this_function */
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_this_function_p();

#ifndef CONFIG_USE_NEW_SYMBOL_TYPE
 case SYM_CLASS_REF:
  /* mov PREFIX, ref <imm8/16> */
  symid = asm_rsymid(src_sym);
  if unlikely(symid < 0) goto err;
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_ref_p((uint16_t)symid);
#endif /* !CONFIG_USE_NEW_SYMBOL_TYPE */

 case SYM_CLASS_ARG:
  /* mov PREFIX, arg <imm8/16> */
  if (!DeeBaseScope_IsArgReqOrDefl(current_basescope,SYMBOL_ARG_INDEX(src_sym)))
       break; /* Can only be used for required, or default arguments. */
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_arg_p(SYMBOL_ARG_INDEX(src_sym));

#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
 case SYMBOL_TYPE_GLOBAL:
  /* mov PREFIX, global <imm8/16> */
  if (!(src_sym->s_flag & SYMBOL_FALLOC)) break;
  symid = asm_gsymid_for_read(src_sym,dst_ast);
  if unlikely(symid < 0) goto err;
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_global_p((uint16_t)symid);

 case SYMBOL_TYPE_LOCAL:
  /* mov PREFIX, local <imm8/16> */
  if (!(src_sym->s_flag & SYMBOL_FALLOC)) break;
  symid = asm_lsymid_for_read(src_sym,dst_ast);
  if unlikely(symid < 0) goto err;
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_local_p((uint16_t)symid);

 case SYMBOL_TYPE_STATIC:
  /* mov PREFIX, static <imm8/16> */
  if (!(src_sym->s_flag & SYMBOL_FALLOC)) break;
  symid = asm_ssymid_for_read(src_sym,dst_ast);
  if unlikely(symid < 0) goto err;
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_static_p((uint16_t)symid);
#else
 case SYM_CLASS_VAR:
  if (!(src_sym->sym_flag & SYM_FVAR_ALLOC))
        break;
  switch (src_sym->sym_flag & SYM_FVAR_MASK) {
  case SYM_FVAR_GLOBAL:
   /* mov PREFIX, global <imm8/16> */
   symid = asm_gsymid_for_read(src_sym,dst_ast);
   if unlikely(symid < 0) goto err;
   if (asm_putddi(dst_ast)) goto err;
   if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
   return asm_gpush_global_p((uint16_t)symid);
  case SYM_FVAR_LOCAL:
   /* mov PREFIX, local <imm8/16> */
   symid = asm_lsymid_for_read(src_sym,dst_ast);
   if unlikely(symid < 0) goto err;
   if (asm_putddi(dst_ast)) goto err;
   if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
   return asm_gpush_local_p((uint16_t)symid);
  case SYM_FVAR_STATIC:
   /* mov PREFIX, static <imm8/16> */
   symid = asm_ssymid_for_read(src_sym,dst_ast);
   if unlikely(symid < 0) goto err;
   if (asm_putddi(dst_ast)) goto err;
   if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
   return asm_gpush_static_p((uint16_t)symid);
  default: break;
  }
  break;
#endif

 case SYM_CLASS_EXTERN:
  /* mov PREFIX, extern <imm8/16>:<imm8/16> */
  if (SYMBOL_EXTERN_SYMBOL(src_sym)->ss_flags & MODSYM_FPROPERTY)
      break; /* Cannot be used for external properties. */
  symid = asm_esymid(src_sym);
  if unlikely(symid < 0) goto err;
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_extern_p((uint16_t)symid,SYMBOL_EXTERN_SYMBOL(src_sym)->ss_index);

 default: break;
 }
 if (asm_putddi(dst_ast)) goto err;
 if (asm_gpush_symbol(src_sym,dst_ast)) goto err;
 if (src_ast != dst_ast && asm_putddi(src_ast)) goto err;
 return asm_gpop_symbol(dst_sym,src_ast);
err:
 return -1;
}


INTERN int
(DCALL asm_gmov_sym_ast)(struct symbol *__restrict dst_sym,
                         DeeAstObject *__restrict src,
                         DeeAstObject *__restrict dst_ast) {
 ASSERT(asm_can_prefix_symbol(dst_sym));
 switch (src->ast_type) {

  /* The ASM_FUNCTION* instructions can be used with a prefix to
   * construct + store the function using a single instruction. */
 case AST_FUNCTION:
  return asm_gmov_function(dst_sym,src,dst_ast);

 case AST_SYM:
  return asm_gmov_sym_sym(dst_sym,src->ast_sym,dst_ast,src);

 {
  DeeObject *constval;
 case AST_CONSTEXPR:
  constval = src->ast_constexpr;
  if (DeeNone_Check(constval)) {
   /* mov PREFIX, none */
   if (asm_putddi(dst_ast)) goto err;
   if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
   return asm_gpush_none_p();
  } else if (constval == Dee_True) {
   /* mov PREFIX, true */
   if (asm_putddi(dst_ast)) goto err;
   if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
   return asm_gpush_true_p();
  } else if (constval == Dee_False) {
   /* mov PREFIX, false */
   if (asm_putddi(dst_ast)) goto err;
   if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
   return asm_gpush_false_p();
  } else if (asm_allowconst(constval)) {
   int32_t cid;
   /* mov PREFIX, const <imm8/16> */
   cid = asm_newconst(constval);
   if unlikely(cid < 0) goto err;
   if (asm_putddi(dst_ast)) goto err;
   if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
   return asm_gpush_const_p((uint16_t)cid);
  }
 } break;

 default:
  break;
 }
 if (ast_genasm(src,ASM_G_FPUSHRES)) goto err;
 if (asm_putddi(dst_ast)) goto err;
 return asm_gpop_symbol(dst_sym,dst_ast);
err:
 return -1;
}

INTERN int
(DCALL asm_gmov_ast_sym)(DeeAstObject *__restrict dst,
                         struct symbol *__restrict src_sym,
                         DeeAstObject *__restrict src_ast) {
 switch (dst->ast_type) {
 case AST_SYM:
  if (asm_can_prefix_symbol(dst->ast_sym))
      return asm_gmov_sym_sym(dst->ast_sym,src_sym,dst,src_ast);
  break;
 default: break;
 }
 if (asm_putddi(src_ast)) goto err;
 if (asm_gpop_expr_enter(dst)) goto err;
 if (asm_gpush_symbol(src_sym,src_ast)) goto err;
 if (asm_gpop_expr_leave(dst,ASM_G_FNORMAL)) goto err;
 return asm_gpop_expr(dst);
err:
 return -1;
}

INTDEF int (DCALL asm_gmov_ast_constexpr)(DeeAstObject *__restrict dst,
                                          DeeObject *__restrict src,
                                          DeeAstObject *__restrict src_ast);


INTERN int
(DCALL asm_gstore)(DeeAstObject *__restrict dst,
                   DeeAstObject *__restrict src,
                   DeeAstObject *__restrict ddi_ast,
                   unsigned int gflags) {
again:
 switch (dst->ast_type) {

 {
  struct symbol *dst_sym;
 case AST_SYM:
  dst_sym = dst->ast_sym;

  /* Special instructions that allow a symbol prefix to specify the target. */
  if (!PUSH_RESULT && asm_can_prefix_symbol(dst_sym))
       return asm_gmov_sym_ast(dst_sym,src,dst);

check_dst_sym_class:
  switch (SYMBOL_TYPE(dst_sym)) {

  case SYM_CLASS_ALIAS:
   ASSERT(dst_sym != SYMBOL_ALIAS(dst_sym));
   dst_sym = SYMBOL_ALIAS(dst_sym);
   goto check_dst_sym_class;

#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
  case SYMBOL_TYPE_GLOBAL:
  case SYMBOL_TYPE_LOCAL:
  case SYMBOL_TYPE_STATIC:
   if (dst_sym->s_type == SYMBOL_TYPE_STATIC &&
     !(dst_sym->s_flag & SYMBOL_FALLOC))
#else
  case SYM_CLASS_VAR:
   if (dst_sym->sym_flag == (SYM_FVAR_STATIC & ~SYM_FVAR_ALLOC))
#endif
   {
    int32_t sid;
    /* Special case: Unallocated static variable
     * > The first assignment is used as the static initializer */
    if (src->ast_type == AST_CONSTEXPR &&
        asm_allowconst(src->ast_constexpr)) {
     /* The simple case: the initializer itself is a constant expression
      * -> In this case, we can simply encode the initializer as the raw
      *    static value and not have to generate any runtime code! */
     sid = asm_newstatic(src->ast_constexpr);
     if unlikely(sid < 0) goto err;
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
     dst_sym->s_symid = (uint16_t)sid;
     dst_sym->s_flag |= SYMBOL_FALLOC;
#else
     dst_sym->sym_var.sym_index = (uint16_t)sid;
     dst_sym->sym_flag |= SYM_FVAR_ALLOC;
#endif
     if (PUSH_RESULT &&
        (asm_putddi(ddi_ast) ||
         asm_gpush_static((uint16_t)sid)))
         goto err;
     goto done;
    }
    /* The source-expression cannot be evaluated at compile-time.
     * Instead, we must generate runtime wrapper code:
     * >>     .static my_static       = @none
     * >>     .static is_initialized  = @false
     * >>     .static is_initializing = @false
     * >>.Lretry:
     * >>     push  true
     * >>     swap  @is_initializing, top // Guarantied to be atomic for static variables
     * >>.Lexcept_start: // Protect against exceptions, return & yield the initializer
     * >>     jt    pop, .Lwaitfor
     * >>
     * >>     // Do the actual init
     * >>     push  <...> // The initializer expression
     * >>#if USING_RESULT
     * >>     dup
     * >>#endif
     * >>     pop   @my_static
     * >>
     * >>     mov   @is_initialized, true // Indicate that initialization has been completed
     * >>.Lexcept_end:
     * >>     jmp   .Ldone2
     * >>.Lwaitfor:
     * >>     push  @is_initialized
     * >>     jt    .Ldone // Check if initialization has been completed
     * >>     push  extern @deemon:@thread
     * >>     callattr top, @"yield", #0 // Yield to get the thread performing the init to run
     * >>     pop
     * >>#if INIT_MAY_THROW_EXCEPTIONS
     * >>     jmp   .Lretry   // If the other thread failed, we'll re-attemt the init
     * >>#else
     * >>     jmp   .Lwaitfor // The other thread can't fail, so we just have to wait
     * >>#endif
     * >>.Ldone:
     * >>#if USING_RESULT
     * >>     push  @my_static
     * >>#endif
     * >>.Ldone2:
     * >>     ...
     * >>
     * >>.section .cold
     * >>.Lexcept_entry:
     * >>.except .Lexcept_start, .Lexcept_end, .Lexcept_entry, @finally:
     * >>     // Indicate initialization failure
     * >>     // NOTE: We use a finally-handler so we also indicate initialization
     * >>     //       failure when the init contains a yield or return statement
     * >>     //       that ended up never returning.
     * >>     mov   @is_initializing, false
     * >>     end   finally
     * >>     throw except // Shouldn't get here, but satisfies peephole and code integrity checks
     */
    /* TODO */
   }
   if (ast_genasm(src,ASM_G_FPUSHRES)) goto err;
   if (PUSH_RESULT && (asm_putddi(ddi_ast) || asm_gdup())) goto err;
   if (asm_putddi(dst)) goto err;
   return asm_gpop_symbol(dst_sym,dst);

  default: break;
  }
  if (src->ast_type == AST_SYM && !PUSH_RESULT &&
      asm_can_prefix_symbol_for_read(src->ast_sym)) {
   int32_t symid;
check_dst_sym_class_hybrid:
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
   if (!SYMBOL_MUST_REFERENCE(dst_sym))
#endif
   {
    switch (SYMBOL_TYPE(dst_sym)) {

    case SYM_CLASS_ALIAS:
     ASSERT(dst_sym != SYMBOL_ALIAS(dst_sym));
     dst_sym = SYMBOL_ALIAS(dst_sym);
     goto check_dst_sym_class_hybrid;

#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
    case SYMBOL_TYPE_GLOBAL:
     /* mov global <imm8/16>, PREFIX */
     symid = asm_gsymid(dst_sym);
     if unlikely(symid < 0) goto err;
     if (asm_putddi(dst)) goto err;
     if (asm_gprefix_symbol(src->ast_sym,src)) goto err;
     return asm_gpop_global_p((uint16_t)symid);

    case SYMBOL_TYPE_LOCAL:
     /* mov local <imm8/16>, PREFIX */
     symid = asm_lsymid(dst_sym);
     if unlikely(symid < 0) goto err;
     if (asm_putddi(dst)) goto err;
     if (asm_gprefix_symbol(src->ast_sym,src)) goto err;
     return asm_gpop_local_p((uint16_t)symid);

    case SYMBOL_TYPE_STATIC:
     /* mov static <imm8/16>, PREFIX */
     symid = asm_ssymid(dst_sym);
     if unlikely(symid < 0) goto err;
     if (asm_putddi(dst)) goto err;
     if (asm_gprefix_symbol(src->ast_sym,src)) goto err;
     return asm_gpop_static_p((uint16_t)symid);
#else
    case SYM_CLASS_VAR:
     switch (dst_sym->sym_flag & SYM_FVAR_MASK) {
     case SYM_FVAR_GLOBAL:
      /* mov global <imm8/16>, PREFIX */
      symid = asm_gsymid(dst_sym);
      if unlikely(symid < 0) goto err;
      if (asm_putddi(dst)) goto err;
      if (asm_gprefix_symbol(src->ast_sym,src)) goto err;
      return asm_gpop_global_p((uint16_t)symid);
     case SYM_FVAR_LOCAL:
      /* mov local <imm8/16>, PREFIX */
      symid = asm_lsymid(dst_sym);
      if unlikely(symid < 0) goto err;
      if (asm_putddi(dst)) goto err;
      if (asm_gprefix_symbol(src->ast_sym,src)) goto err;
      return asm_gpop_local_p((uint16_t)symid);
     case SYM_FVAR_STATIC:
      /* mov static <imm8/16>, PREFIX */
      symid = asm_ssymid(dst_sym);
      if unlikely(symid < 0) goto err;
      if (asm_putddi(dst)) goto err;
      if (asm_gprefix_symbol(src->ast_sym,src)) goto err;
      return asm_gpop_static_p((uint16_t)symid);
     default: break;
     }
     break;
#endif

    case SYM_CLASS_EXTERN:
     /* mov extern <imm8/16>, PREFIX */
     if (SYMBOL_EXTERN_SYMBOL(dst_sym)->ss_flags &
        (MODSYM_FREADONLY|MODSYM_FPROPERTY)) break;
     symid = asm_esymid(dst_sym);
     if unlikely(symid < 0) goto err;
     if (asm_putddi(dst)) goto err;
     if (asm_gprefix_symbol(src->ast_sym,src)) goto err;
     return asm_gpop_extern_p((uint16_t)symid,SYMBOL_EXTERN_SYMBOL(dst_sym)->ss_index);

    case SYM_CLASS_STACK:
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
     if (!(dst_sym->s_flag & SYMBOL_FALLOC))
           break;
#else
     if (!(dst_sym->sym_flag & SYM_FSTK_ALLOC))
           break;
#endif
     if (asm_putddi(ddi_ast)) goto err;
     if (asm_gprefix_symbol(src->ast_sym,src)) goto err;
     if (SYMBOL_STACK_OFFSET(dst_sym) == current_assembler.a_stackcur-1) {
      /* mov top, PREFIX */
      return asm_gpop_p();
     } else {
      /* mov #<imm8/16>, PREFIX */
      uint16_t offset;
      offset = (uint16_t)((current_assembler.a_stackcur -
                           SYMBOL_STACK_OFFSET(dst_sym))-1);
      return asm_gpop_n_p(offset);
     }
     break;

    default: break;
    }
   }
  }
 } break;


 case AST_CONSTEXPR:
  /* Check for special case: store into a constant
   * expression `none' is the same as `pop' */
  if (!DeeNone_Check(dst->ast_constexpr))
       break;
  /* Store-to-none is a no-op (so just assembly the source-expression). */
  return ast_genasm(src,gflags);

 case AST_OPERATOR:
  switch (dst->ast_flag) {

  case OPERATOR_GETATTR:
   if unlikely(!dst->ast_operator.ast_opb) break;
   return ast_gen_setattr(dst->ast_operator.ast_opa,
                          dst->ast_operator.ast_opb,
                          src,ddi_ast,gflags);

  case OPERATOR_GETITEM:
   if unlikely(!dst->ast_operator.ast_opb) break;
   return ast_gen_setitem(dst->ast_operator.ast_opa,
                          dst->ast_operator.ast_opb,
                          src,ddi_ast,gflags);

  case OPERATOR_GETRANGE:
   if unlikely(!dst->ast_operator.ast_opc) break;
   return ast_gen_setrange(dst->ast_operator.ast_opa,
                           dst->ast_operator.ast_opb,
                           dst->ast_operator.ast_opc,
                           src,ddi_ast,gflags);

  default: break;
  }
  break;

 case AST_MULTIPLE:
  /* Special handling for unpack expressions (i.e. `(a,b,c) = foo()'). */
  if (dst->ast_flag == AST_FMULTIPLE_KEEPLAST) {
   size_t i;
   if (dst->ast_multiple.ast_exprc == 0)
       return ast_genasm(src,gflags);
   /* Compile all branches except for the last one normally. */
   for (i = 0; i < dst->ast_multiple.ast_exprc-1; ++i)
      if (ast_genasm(dst->ast_multiple.ast_exprv[i],ASM_G_FNORMAL))
          goto err;
   /* The last branch is the one we're going to write to. */
   dst = dst->ast_multiple.ast_exprv[i];
   goto again;
  }
  /* Optimization for special case: (a,b,c) = (d,e,f); */
  if (src->ast_type == AST_MULTIPLE &&
      src->ast_flag != AST_FMULTIPLE_KEEPLAST &&
      src->ast_multiple.ast_exprc == dst->ast_multiple.ast_exprc &&
     !ast_multiple_hasexpand(src) && !PUSH_RESULT) {
   size_t i;
   for (i = 0; i < src->ast_multiple.ast_exprc; ++i) {
    if (asm_gstore(dst->ast_multiple.ast_exprv[i],
                   src->ast_multiple.ast_exprv[i],
                   ddi_ast,gflags))
        goto err;
   }
   goto done;
  }
  if (src->ob_type == AST_CONSTEXPR) {
   /* TODO: Optimizations for `none'. */
   /* TODO: Optimizations for sequence constants. */
  }
  break;

 default: break;
 }
 /* TODO: Special handling when the stored value uses the target:
  *  >> local my_list = [10,20,my_list]; // Should create a self-referencing list.
  * ASM:
  *  >>    pack list 0
  *  >>    pop  local @my_list
  *  >>    push local @my_list  // May be optimized to a dup
  *  >>    push $10
  *  >>    push $20
  *  >>    push local @my_list
  *  >>    pack list 3
  *  >>    move assign top, pop // Move-assign the second list.
  * Essentially, this would look like this:
  *  >> local my_list = [];
  *  >> my_list := [10,20,my_list]; */
 if (asm_gpop_expr_enter(dst)) goto err;
 if (ast_genasm(src,ASM_G_FPUSHRES)) goto err;
 if (asm_gpop_expr_leave(dst,gflags)) goto err;
done:
 return 0;
err:
 return -1;
}

#undef PUSH_RESULT

INTERN int DCALL
asm_gpop_expr_multiple(size_t astc, DeeAstObject **__restrict astv) {
 size_t i,j;
 /* Optimization: Trailing asts with no side-effects can be handled in reverse order */
 while (astc && !ast_has_sideeffects(astv[astc-1])) {
  --astc; /* This way we don't have to rotate stack-elements. */
  if (asm_gpop_expr(astv[astc])) goto err;
 }
 if (!astc) goto done;
 i = 0;
 /* Find the first AST that does actually have side-effects. */
 while (i < astc && !ast_has_sideeffects(astv[i])) ++i;
 for (j = i; j < astc; ++j) {
  if (asm_glrot((uint16_t)(astc-j))) goto err;
  if (asm_gpop_expr(astv[j])) goto err;
 }
 /* Handle leading asts without side-effects in reverse order. */
 while (i--) if (asm_gpop_expr(astv[i])) goto err;

done:
 return 0;
err:
 return -1;
}

INTERN int DCALL
asm_gpop_expr(DeeAstObject *__restrict ast) {
 switch (ast->ast_type) {

 case AST_SYM:
  if (asm_putddi(ast)) goto err;
  return asm_gpop_symbol(ast->ast_sym,ast);

 case AST_MULTIPLE:
  if (ast->ast_flag == AST_FMULTIPLE_KEEPLAST) {
   size_t i;
   /* Special case: Store to KEEP-last multiple AST:
    *               Evaluate all branches but the last without using them.
    *               Then simply write the value to the last expression.
    */
   if (ast->ast_multiple.ast_exprc == 0)
       return asm_gpop();
   for (i = 0; i != ast->ast_multiple.ast_exprc-1; ++i)
      if (ast_genasm(ast->ast_multiple.ast_exprv[i],ASM_G_FNORMAL)) goto err;
   ASSERT(i == ast->ast_multiple.ast_exprc-1);
   return asm_gpop_expr(ast->ast_multiple.ast_exprv[i]);
  }
  if (asm_putddi(ast)) goto err;
  if (asm_gunpack((uint16_t)ast->ast_multiple.ast_exprc)) goto err;
  if (asm_gpop_expr_multiple(ast->ast_multiple.ast_exprc,
                             ast->ast_multiple.ast_exprv))
      goto err;
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
     while (SYMBOL_TYPE(base->ast_sym) == SYM_CLASS_ALIAS)
         base->ast_sym = SYMBOL_ALIAS(base->ast_sym);
     if (SYMBOL_TYPE(base->ast_sym) == SYM_CLASS_THIS
#ifdef CONFIG_USE_NEW_SYMBOL_TYPE
         && !SYMBOL_MUST_REFERENCE_TYPEMAY(base->ast_sym)
#endif
         ) {
      if (asm_putddi(ast)) goto err;
      if (asm_gsetattr_this_const((uint16_t)cid)) goto err;
      goto done;
     }
    }
    if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ast)) goto err;
    if (asm_gswap()) goto err;
    if (asm_gsetattr_const((uint16_t)cid)) goto err;
    goto done;
   }
   if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(ast->ast_operator.ast_opb,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast)) goto err;
   if (asm_glrot(3)) goto err;
   if (asm_gsetattr()) goto err;
   goto done;
  }


  {
   DeeAstObject *index;
  case OPERATOR_GETITEM:
   if unlikely((index = ast->ast_operator.ast_opb) == NULL) break;
   if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
   if (index->ast_type == AST_CONSTEXPR) {
    int32_t int_index;
    /* Special optimizations for constant indices. */
    if (DeeInt_Check(index->ast_constexpr) &&
        DeeInt_TryAsS32(index->ast_constexpr,&int_index) &&
        int_index >= INT16_MIN && int_index <= INT16_MAX) {
     if (asm_putddi(ast)) goto err;
     if (asm_gswap()) goto err;
     if (asm_gsetitem_index((int16_t)int_index)) goto err;
     goto done;
    }
    if (asm_allowconst(index->ast_constexpr)) {
     int_index = asm_newconst(index->ast_constexpr);
     if unlikely(int_index < 0) goto err;
     if (asm_putddi(ast)) goto err;
     if (asm_gswap()) goto err;
     if (asm_gsetitem_const((uint16_t)int_index)) goto err;
     goto done;
    }
   }
   if (ast_genasm(index,ASM_G_FPUSHRES)) goto err; /* STACK: item, base, index */
   if (asm_putddi(ast)) goto err;
   if (asm_glrot(3)) goto err;           /* STACK: base, index, item */
   if (asm_gsetitem()) goto err;         /* STACK: - */
   goto done;
  }

  {
   DeeAstObject *begin,*end;
   int32_t index;
  case OPERATOR_GETRANGE:
   if unlikely(!ast->ast_operator.ast_opc) break;
   if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
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
      if (asm_putddi(ast)) goto err;
      if (asm_gswap()) goto err; /* STACK: base, item */
      if (asm_gsetrange_ni((int16_t)index)) goto err;
      goto done;
     }
     /* `setrange pop, none, pop, pop' */
     if (ast_genasm(end,ASM_G_FPUSHRES)) goto err; /* STACK: item, base, end */
     if (asm_putddi(ast)) goto err;
     if (asm_glrot(3)) goto err;         /* STACK: base, end, item */
     if (asm_gsetrange_np()) goto err;   /* STACK: - */
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
       if (asm_putddi(ast)) goto err;
       if (asm_gswap()) goto err;                      /* STACK: base, item */
       if (asm_gsetrange_in((int16_t)index)) goto err; /* STACK: - */
       goto done;
      }
      if (DeeInt_Check(end_index) &&
          DeeInt_TryAsS32(end_index,&index2) &&
          index2 >= INT16_MIN && index2 <= INT16_MAX) {
       /* `setrange pop, $<Simm16>, $<Simm16>, pop' */
       if (asm_putddi(ast)) goto err;
       if (asm_gswap()) goto err;                                      /* STACK: base, item */
       if (asm_gsetrange_ii((int16_t)index,(int16_t)index2)) goto err; /* STACK: - */
       goto done;
      }
     }
     if (ast_genasm(end,ASM_G_FPUSHRES)) goto err; /* STACK: item, base, end */
     if (asm_putddi(ast)) goto err;
     if (asm_glrot(3)) goto err;         /* STACK: base, end, item */
     if (asm_gsetrange_ip((int16_t)index)) goto err; /* STACK: - */
     goto done;
    }
   } else if (end->ast_type == AST_CONSTEXPR) {
    /* Optimization: `setrange pop, pop, [none | $<Simm16>], pop' */
    DeeObject *end_index = end->ast_constexpr;
    int32_t index;
    if (DeeNone_Check(end_index)) {
     /* `setrange pop, pop, none, pop' */
     if (ast_genasm(begin,ASM_G_FPUSHRES)) goto err; /* STACK: item, base, begin */
     if (asm_putddi(ast)) goto err;
     if (asm_glrot(3)) goto err;           /* STACK: base, begin, item */
     if (asm_gsetrange_pn()) goto err;     /* STACK: - */
     goto done;
    }
    if (DeeInt_Check(end_index) &&
        DeeInt_TryAsS32(end_index,&index) &&
        index >= INT16_MIN && index <= INT16_MAX) {
     /* `setrange pop, pop, $<Simm16>, pop' */
     if (ast_genasm(begin,ASM_G_FPUSHRES)) goto err;           /* STACK: item, base, begin */
     if (asm_putddi(ast)) goto err;
     if (asm_glrot(3)) goto err;                     /* STACK: base, begin, item */
     if (asm_gsetrange_pi((int16_t)index)) goto err; /* STACK: - */
     goto done;
    }
   }
   if (ast_genasm(begin,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(end,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast)) goto err;
                                  /* STACK: item, base, begin, end */
   if (asm_glrot(4)) goto err;    /* STACK: base, begin, end, item */
   if (asm_gsetrange()) goto err; /* STACK: - */
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
  if (asm_gpop()) goto err;
  break;

 default:
default_case:
  /* Emit a warning about an r-value store. */
  if (WARNAST(ast,W_ASM_STORE_TO_RVALUE))
      goto err;
  /* Fallback: Generate the ast and store it directly. */
  if (ast_genasm(ast,ASM_G_FPUSHRES)) goto err;
  if (asm_putddi(ast)) goto err;
  if (asm_gswap()) goto err;
  if (asm_gassign()) goto err;
  break;
 }
done:
 return 0;
err:
 return -1;
}

DECL_END

#ifndef __INTELLISENSE__
#undef ENTER
#undef LEAVE
#define ENTER 1
#include "genstore-setup.c.inl"
#define LEAVE 1
#include "genstore-setup.c.inl"
#endif

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENSTORE_C */
