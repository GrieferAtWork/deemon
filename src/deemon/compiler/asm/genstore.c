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
#include <deemon/compiler/optimize.h>

DECL_BEGIN

#define PUSH_RESULT  (gflags & ASM_G_FPUSHRES)

PRIVATE int DCALL
asm_gpush2_duplast(struct ast *__restrict a,
                   struct ast *__restrict b,
                   struct ast *__restrict ddi_ast,
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
asm_gpush3_duplast(struct ast *__restrict a,
                   struct ast *__restrict b,
                   struct ast *__restrict c,
                   struct ast *__restrict ddi_ast,
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
asm_gpush4_duplast(struct ast *__restrict a,
                   struct ast *__restrict b,
                   struct ast *__restrict c,
                   struct ast *__restrict d,
                   struct ast *__restrict ddi_ast,
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
ast_gen_setattr(struct ast *__restrict base,
                struct ast *__restrict name,
                struct ast *__restrict value,
                struct ast *__restrict ddi_ast,
                unsigned int gflags) {
 if (name->a_type == AST_CONSTEXPR &&
     DeeString_Check(name->a_constexpr)) {
  int32_t cid;
  if (base->a_type == AST_SYM) {
   struct symbol *sym = base->a_sym;
check_base_symbol_class:
   if (!SYMBOL_MUST_REFERENCE(sym)) {
    switch (SYMBOL_TYPE(sym)) {

    case SYMBOL_TYPE_ALIAS:
     sym = SYMBOL_ALIAS(sym);
     goto check_base_symbol_class;

    case SYMBOL_TYPE_THIS:
     cid = asm_newconst(name->a_constexpr);
     if unlikely(cid < 0) goto err;
     if (ast_genasm(value,ASM_G_FPUSHRES)) goto err;
     if (PUSH_RESULT && (asm_putddi(ddi_ast) || asm_gdup())) goto err;
     if (asm_gsetattr_this_const((uint16_t)cid)) goto err;
     goto done;
    {
     struct module_symbol *modsym; int32_t module_id;
    case SYMBOL_TYPE_MODULE: /* module.attr --> pop extern ... */
     modsym = get_module_symbol(SYMBOL_MODULE_MODULE(sym),
                               (DeeStringObject *)name->a_constexpr);
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
  cid = asm_newconst(name->a_constexpr);
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
ast_gen_setitem(struct ast *__restrict sequence,
                struct ast *__restrict index,
                struct ast *__restrict value,
                struct ast *__restrict ddi_ast,
                unsigned int gflags) {
 if (index->a_type == AST_CONSTEXPR) {
  int32_t int_index;
  /* Special optimizations for constant indices. */
  if (DeeInt_Check(index->a_constexpr) &&
      DeeInt_TryAsS32(index->a_constexpr,&int_index) &&
      int_index >= INT16_MIN && int_index <= INT16_MAX) {
   if unlikely(asm_gpush2_duplast(sequence,value,ddi_ast,gflags))
      goto err;
   if (asm_gsetitem_index((int16_t)int_index)) goto err;
   goto done;
  }
  if (asm_allowconst(index->a_constexpr)) {
   int_index = asm_newconst(index->a_constexpr);
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
ast_gen_setrange(struct ast *__restrict sequence,
                 struct ast *__restrict begin,
                 struct ast *__restrict end,
                 struct ast *__restrict value,
                 struct ast *__restrict ddi_ast,
                 unsigned int gflags) {
 int32_t index;
 /* Special optimizations for certain ranges. */
 if (begin->a_type == AST_CONSTEXPR) {
  DeeObject *begin_index = begin->a_constexpr;
  if (DeeNone_Check(begin_index)) {
   /* Optimization: `setrange pop, none, [pop | $<Simm16>], pop' */
   if (end->a_type == AST_CONSTEXPR &&
       DeeInt_Check(end->a_constexpr) &&
       DeeInt_TryAsS32(end->a_constexpr,&index) &&
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
   if (end->a_type == AST_CONSTEXPR) {
    int32_t index2;
    DeeObject *end_index = end->a_constexpr;
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
 } else if (end->a_type == AST_CONSTEXPR) {
  /* Optimization: `setrange pop, pop, [none | $<Simm16>], pop' */
  DeeObject *end_index = end->a_constexpr;
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
asm_gunpack_expr(struct ast *__restrict src,
                 uint16_t num_targets,
                 struct ast *__restrict ddi_ast) {
 /* Unwind inner sequence-expand expressions.
  * This optimizes:
  * >> `(x,y,z) = [(10,20,30)...];'
  * Into:
  * >> `(x,y,z) = (10,20,30);'
  */
 for (;;) {
  struct ast *inner;
  if (src->a_type != AST_MULTIPLE) break;
  if (src->a_flag == AST_FMULTIPLE_KEEPLAST) break;
  if (src->a_multiple.m_astc != 1) break;
  inner = src->a_multiple.m_astv[0];
  if (inner->a_type != AST_EXPAND) break;
  src = inner->a_expand;
 }
 if (src->a_type == AST_SYM) {
  struct symbol *sym = src->a_sym;
  SYMBOL_INPLACE_UNWIND_ALIAS(sym);
  if (SYMBOL_TYPE(sym) == SYMBOL_TYPE_ARG &&
     (current_basescope->bs_flags & CODE_FVARARGS) &&
     !SYMBOL_MUST_REFERENCE_TYPEMAY(sym) &&
     !DeeBaseScope_HasOptional(current_basescope) &&
      DeeBaseScope_IsArgVarArgs(current_basescope,sym->s_symid)) {
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
                        struct ast *__restrict dst_ast,
                        struct ast *__restrict src_ast) {
 int32_t symid;
 ASSERT(asm_can_prefix_symbol(dst_sym));
check_src_sym_class:
 if (SYMBOL_MUST_REFERENCE(src_sym)) {
  /* mov PREFIX, ref <imm8/16> */
  symid = asm_rsymid(src_sym);
  if unlikely(symid < 0) goto err;
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_ref_p((uint16_t)symid);
 }
 switch (SYMBOL_TYPE(src_sym)) {

 case SYMBOL_TYPE_ALIAS:
  src_sym = SYMBOL_ALIAS(src_sym);
  goto check_src_sym_class;

 case SYMBOL_TYPE_STACK:
  /* mov PREFIX, #... */
  if (!(src_sym->s_flag & SYMBOL_FALLOC)) break;
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

 case SYMBOL_TYPE_EXCEPT:
  /* mov PREFIX, except */
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_except_p();

 case SYMBOL_TYPE_MODULE:
  /* mov PREFIX, module <imm8/16> */
  symid = asm_msymid(src_sym);
  if unlikely(symid < 0) goto err;
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_module_p((uint16_t)symid);

 case SYMBOL_TYPE_THIS:
  /* mov PREFIX, this */
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_this_p();

 case SYMBOL_TYPE_MYMOD:
  /* mov PREFIX, this_module */
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_this_module_p();

 case SYMBOL_TYPE_MYFUNC:
  /* mov PREFIX, this_function */
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_this_function_p();

 case SYMBOL_TYPE_ARG:
  /* mov PREFIX, arg <imm8/16> */
  if (!DeeBaseScope_IsArgReqOrDefl(current_basescope,src_sym->s_symid))
       break; /* Can only be used for required, or default arguments. */
  if (asm_putddi(dst_ast)) goto err;
  if (asm_gprefix_symbol(dst_sym,dst_ast)) goto err;
  return asm_gpush_arg_p(src_sym->s_symid);

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

 case SYMBOL_TYPE_EXTERN:
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
                         struct ast *__restrict src,
                         struct ast *__restrict dst_ast) {
 ASSERT(asm_can_prefix_symbol(dst_sym));
 switch (src->a_type) {

  /* The ASM_FUNCTION* instructions can be used with a prefix to
   * construct + store the function using a single instruction. */
 case AST_FUNCTION:
  return asm_gmov_function(dst_sym,src,dst_ast);

 case AST_SYM:
  return asm_gmov_sym_sym(dst_sym,src->a_sym,dst_ast,src);

 {
  DeeObject *constval;
 case AST_CONSTEXPR:
  constval = src->a_constexpr;
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
(DCALL asm_gmov_ast_sym)(struct ast *__restrict dst,
                         struct symbol *__restrict src_sym,
                         struct ast *__restrict src_ast) {
 switch (dst->a_type) {
 case AST_SYM:
  if (asm_can_prefix_symbol(dst->a_sym))
      return asm_gmov_sym_sym(dst->a_sym,src_sym,dst,src_ast);
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

INTDEF int (DCALL asm_gmov_ast_constexpr)(struct ast *__restrict dst,
                                          DeeObject *__restrict src,
                                          struct ast *__restrict src_ast);


INTERN int
(DCALL asm_gstore)(struct ast *__restrict dst,
                   struct ast *__restrict src,
                   struct ast *__restrict ddi_ast,
                   unsigned int gflags) {
again:
 switch (dst->a_type) {

 {
  struct symbol *dst_sym;
 case AST_SYM:
  dst_sym = dst->a_sym;

  /* Special instructions that allow a symbol prefix to specify the target. */
  if (!PUSH_RESULT && asm_can_prefix_symbol(dst_sym))
       return asm_gmov_sym_ast(dst_sym,src,dst);

check_dst_sym_class:
  switch (SYMBOL_TYPE(dst_sym)) {

  case SYMBOL_TYPE_ALIAS:
   dst_sym = SYMBOL_ALIAS(dst_sym);
   goto check_dst_sym_class;

  case SYMBOL_TYPE_GLOBAL:
  case SYMBOL_TYPE_LOCAL:
  case SYMBOL_TYPE_STATIC:
   if (dst_sym->s_type == SYMBOL_TYPE_STATIC &&
     !(dst_sym->s_flag & SYMBOL_FALLOC)) {
    int32_t sid;
    /* Special case: Unallocated static variable
     * > The first assignment is used as the static initializer */
    if (src->a_type == AST_CONSTEXPR &&
        asm_allowconst(src->a_constexpr)) {
     /* The simple case: the initializer itself is a constant expression
      * -> In this case, we can simply encode the initializer as the raw
      *    static value and not have to generate any runtime code! */
     sid = asm_newstatic(src->a_constexpr);
     if unlikely(sid < 0) goto err;
     dst_sym->s_symid = (uint16_t)sid;
     dst_sym->s_flag |= SYMBOL_FALLOC;
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
  if (src->a_type == AST_SYM && !PUSH_RESULT &&
      asm_can_prefix_symbol_for_read(src->a_sym)) {
   int32_t symid;
check_dst_sym_class_hybrid:
   if (!SYMBOL_MUST_REFERENCE(dst_sym)) {
    switch (SYMBOL_TYPE(dst_sym)) {

    case SYMBOL_TYPE_ALIAS:
     dst_sym = SYMBOL_ALIAS(dst_sym);
     goto check_dst_sym_class_hybrid;

    case SYMBOL_TYPE_GLOBAL:
     /* mov global <imm8/16>, PREFIX */
     symid = asm_gsymid(dst_sym);
     if unlikely(symid < 0) goto err;
     if (asm_putddi(dst)) goto err;
     if (asm_gprefix_symbol(src->a_sym,src)) goto err;
     return asm_gpop_global_p((uint16_t)symid);

    case SYMBOL_TYPE_LOCAL:
     /* mov local <imm8/16>, PREFIX */
     symid = asm_lsymid(dst_sym);
     if unlikely(symid < 0) goto err;
     if (asm_putddi(dst)) goto err;
     if (asm_gprefix_symbol(src->a_sym,src)) goto err;
     return asm_gpop_local_p((uint16_t)symid);

    case SYMBOL_TYPE_STATIC:
     /* mov static <imm8/16>, PREFIX */
     symid = asm_ssymid(dst_sym);
     if unlikely(symid < 0) goto err;
     if (asm_putddi(dst)) goto err;
     if (asm_gprefix_symbol(src->a_sym,src)) goto err;
     return asm_gpop_static_p((uint16_t)symid);

    case SYMBOL_TYPE_EXTERN:
     /* mov extern <imm8/16>, PREFIX */
     if (SYMBOL_EXTERN_SYMBOL(dst_sym)->ss_flags &
        (MODSYM_FREADONLY|MODSYM_FPROPERTY)) break;
     symid = asm_esymid(dst_sym);
     if unlikely(symid < 0) goto err;
     if (asm_putddi(dst)) goto err;
     if (asm_gprefix_symbol(src->a_sym,src)) goto err;
     return asm_gpop_extern_p((uint16_t)symid,SYMBOL_EXTERN_SYMBOL(dst_sym)->ss_index);

    case SYMBOL_TYPE_STACK:
     if (!(dst_sym->s_flag & SYMBOL_FALLOC))
           break;
     if (asm_putddi(ddi_ast)) goto err;
     if (asm_gprefix_symbol(src->a_sym,src)) goto err;
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
  if (!DeeNone_Check(dst->a_constexpr))
       break;
  /* Store-to-none is a no-op (so just assembly the source-expression). */
  return ast_genasm(src,gflags);

 case AST_OPERATOR:
  switch (dst->a_flag) {

  case OPERATOR_GETATTR:
   if unlikely(!dst->a_operator.o_op1) break;
   return ast_gen_setattr(dst->a_operator.o_op0,
                          dst->a_operator.o_op1,
                          src,ddi_ast,gflags);

  case OPERATOR_GETITEM:
   if unlikely(!dst->a_operator.o_op1) break;
   return ast_gen_setitem(dst->a_operator.o_op0,
                          dst->a_operator.o_op1,
                          src,ddi_ast,gflags);

  case OPERATOR_GETRANGE:
   if unlikely(!dst->a_operator.o_op2) break;
   return ast_gen_setrange(dst->a_operator.o_op0,
                           dst->a_operator.o_op1,
                           dst->a_operator.o_op2,
                           src,ddi_ast,gflags);

  default: break;
  }
  break;

 case AST_MULTIPLE:
  /* Special handling for unpack expressions (i.e. `(a,b,c) = foo()'). */
  if (dst->a_flag == AST_FMULTIPLE_KEEPLAST) {
   size_t i = 0;
   if (dst->a_multiple.m_astc == 0)
       return ast_genasm(src,gflags);
   /* Compile all branches except for the last one normally. */
   if (dst->a_multiple.m_astc > 1) {
    ASM_PUSH_SCOPE(dst->a_scope,err);
    for (; i < dst->a_multiple.m_astc-1; ++i) {
     if (ast_genasm(dst->a_multiple.m_astv[i],ASM_G_FNORMAL))
         goto err;
    }
    ASM_POP_SCOPE(0,err);
   }
   /* The last branch is the one we're going to write to. */
   dst = dst->a_multiple.m_astv[i];
   goto again;
  }
  /* Optimization for special case: (a,b,c) = (d,e,f); */
  if (src->a_type == AST_MULTIPLE &&
      src->a_flag != AST_FMULTIPLE_KEEPLAST &&
      src->a_multiple.m_astc == dst->a_multiple.m_astc &&
     !ast_multiple_hasexpand(src) && !PUSH_RESULT) {
   size_t i;
   for (i = 0; i < src->a_multiple.m_astc; ++i) {
    if (asm_gstore(dst->a_multiple.m_astv[i],
                   src->a_multiple.m_astv[i],
                   ddi_ast,gflags))
        goto err;
   }
   goto done;
  }
  if (src->a_type == AST_CONSTEXPR) {
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
asm_gpop_expr_multiple(size_t astc, struct ast **__restrict astv) {
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
asm_gpop_expr(struct ast *__restrict ast) {
 switch (ast->a_type) {

 case AST_SYM:
  if (asm_putddi(ast)) goto err;
  return asm_gpop_symbol(ast->a_sym,ast);

 case AST_MULTIPLE:
  if (ast->a_flag == AST_FMULTIPLE_KEEPLAST) {
   size_t i = 0;
   /* Special case: Store to KEEP-last multiple AST:
    *               Evaluate all branches but the last without using them.
    *               Then simply write the value to the last expression.
    */
   if (ast->a_multiple.m_astc == 0)
       return asm_gpop();
   if (ast->a_multiple.m_astc > 1)  {
    ASM_PUSH_SCOPE(ast->a_scope,err);
    for (; i != ast->a_multiple.m_astc-1; ++i) {
     if (ast_genasm(ast->a_multiple.m_astv[i],ASM_G_FNORMAL))
         goto err;
    }
    ASM_POP_SCOPE(0,err);
   }
   ASSERT(i == ast->a_multiple.m_astc-1);
   return asm_gpop_expr(ast->a_multiple.m_astv[i]);
  }
  if (asm_putddi(ast)) goto err;
  if (asm_gunpack((uint16_t)ast->a_multiple.m_astc)) goto err;
  if (asm_gpop_expr_multiple(ast->a_multiple.m_astc,
                             ast->a_multiple.m_astv))
      goto err;
  break;

 case AST_OPERATOR:
  switch (ast->a_flag) {
  {
   struct ast *attr;
  case OPERATOR_GETATTR:
   if unlikely((attr = ast->a_operator.o_op1) == NULL) break;
   if (attr->a_type == AST_CONSTEXPR &&
       DeeString_Check(attr->a_constexpr)) {
    int32_t cid = asm_newconst(attr->a_constexpr);
    struct ast *base = ast->a_operator.o_op0;
    if unlikely(cid < 0) goto err;
    if (base->a_type == AST_SYM) {
     SYMBOL_INPLACE_UNWIND_ALIAS(base->a_sym);
     if (SYMBOL_TYPE(base->a_sym) == SYMBOL_TYPE_THIS &&
        !SYMBOL_MUST_REFERENCE_TYPEMAY(base->a_sym)) {
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
   if (ast_genasm(ast->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(ast->a_operator.o_op1,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast)) goto err;
   if (asm_glrot(3)) goto err;
   if (asm_gsetattr()) goto err;
   goto done;
  }


  {
   struct ast *index;
  case OPERATOR_GETITEM:
   if unlikely((index = ast->a_operator.o_op1) == NULL) break;
   if (ast_genasm(ast->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
   if (index->a_type == AST_CONSTEXPR) {
    int32_t int_index;
    /* Special optimizations for constant indices. */
    if (DeeInt_Check(index->a_constexpr) &&
        DeeInt_TryAsS32(index->a_constexpr,&int_index) &&
        int_index >= INT16_MIN && int_index <= INT16_MAX) {
     if (asm_putddi(ast)) goto err;
     if (asm_gswap()) goto err;
     if (asm_gsetitem_index((int16_t)int_index)) goto err;
     goto done;
    }
    if (asm_allowconst(index->a_constexpr)) {
     int_index = asm_newconst(index->a_constexpr);
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
   struct ast *begin,*end;
   int32_t index;
  case OPERATOR_GETRANGE:
   if unlikely(!ast->a_operator.o_op2) break;
   if (ast_genasm(ast->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
   begin = ast->a_operator.o_op1;
   end   = ast->a_operator.o_op2;
   /* Special optimizations for certain ranges. */
   if (begin->a_type == AST_CONSTEXPR) {
    DeeObject *begin_index = begin->a_constexpr;
    if (DeeNone_Check(begin_index)) {
     /* Optimization: `setrange pop, none, [pop | $<Simm16>], pop' */
     if (end->a_type == AST_CONSTEXPR &&
         DeeInt_Check(end->a_constexpr) &&
         DeeInt_TryAsS32(end->a_constexpr,&index) &&
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
     if (end->a_type == AST_CONSTEXPR) {
      int32_t index2;
      DeeObject *end_index = end->a_constexpr;
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
   } else if (end->a_type == AST_CONSTEXPR) {
    /* Optimization: `setrange pop, pop, [none | $<Simm16>], pop' */
    DeeObject *end_index = end->a_constexpr;
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
  if (!DeeNone_Check(ast->a_constexpr))
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
