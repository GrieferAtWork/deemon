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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENINPLACE_C
#define GUARD_DEEMON_COMPILER_ASM_GENINPLACE_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/module.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/lexer.h>

#include "../../runtime/strings.h"
#include "../../runtime/builtin.h"

DECL_BEGIN

/* Quick translation table for most basic operator instruction codes. */
PRIVATE instruction_t const operator_instr_table[] = {
   /* [OPERATOR_CONSTRUCTOR] = */0,
   /* [OPERATOR_COPY]        = */ASM_COPY,
   /* [OPERATOR_DEEPCOPY]    = */ASM_DEEPCOPY,
   /* [OPERATOR_DESTRUCTOR]  = */0,
   /* [OPERATOR_ASSIGN]      = */ASM_ASSIGN,
   /* [OPERATOR_MOVEASSIGN]  = */ASM_MOVE_ASSIGN,
   /* [OPERATOR_STR]         = */ASM_STR,
   /* [OPERATOR_REPR]        = */ASM_REPR,
   /* [OPERATOR_BOOL]        = */ASM_BOOL,
   /* [OPERATOR_CALL]        = */ASM_CALL_TUPLE,
   /* [OPERATOR_ITERNEXT]    = */0, /* ASM_ITERNEXT - Extended opcodes can't be used here. */
   /* [OPERATOR_INT]         = */ASM_CAST_INT,
   /* [OPERATOR_FLOAT]       = */0,
   /* [OPERATOR_INV]         = */ASM_INV,
   /* [OPERATOR_POS]         = */ASM_POS,
   /* [OPERATOR_NEG]         = */ASM_NEG,
   /* [OPERATOR_ADD]         = */ASM_ADD,
   /* [OPERATOR_SUB]         = */ASM_SUB,
   /* [OPERATOR_MUL]         = */ASM_MUL,
   /* [OPERATOR_DIV]         = */ASM_DIV,
   /* [OPERATOR_MOD]         = */ASM_MOD,
   /* [OPERATOR_SHL]         = */ASM_SHL,
   /* [OPERATOR_SHR]         = */ASM_SHR,
   /* [OPERATOR_AND]         = */ASM_AND,
   /* [OPERATOR_OR]          = */ASM_OR,
   /* [OPERATOR_XOR]         = */ASM_XOR,
   /* [OPERATOR_POW]         = */ASM_POW,
   /* [OPERATOR_INC]         = */ASM_INC, /* Inplace! */
   /* [OPERATOR_DEC]         = */ASM_DEC, /* Inplace! */
   /* [OPERATOR_INPLACE_ADD] = */ASM_ADD, /* Inplace! */
   /* [OPERATOR_INPLACE_SUB] = */ASM_SUB, /* Inplace! */
   /* [OPERATOR_INPLACE_MUL] = */ASM_MUL, /* Inplace! */
   /* [OPERATOR_INPLACE_DIV] = */ASM_DIV, /* Inplace! */
   /* [OPERATOR_INPLACE_MOD] = */ASM_MOD, /* Inplace! */
   /* [OPERATOR_INPLACE_SHL] = */ASM_SHL, /* Inplace! */
   /* [OPERATOR_INPLACE_SHR] = */ASM_SHR, /* Inplace! */
   /* [OPERATOR_INPLACE_AND] = */ASM_AND, /* Inplace! */
   /* [OPERATOR_INPLACE_OR]  = */ASM_OR,  /* Inplace! */
   /* [OPERATOR_INPLACE_XOR] = */ASM_XOR, /* Inplace! */
   /* [OPERATOR_INPLACE_POW] = */ASM_POW, /* Inplace! */
   /* [OPERATOR_HASH]        = */0,
   /* [OPERATOR_EQ]          = */ASM_CMP_EQ,
   /* [OPERATOR_NE]          = */ASM_CMP_NE,
   /* [OPERATOR_LO]          = */ASM_CMP_LO,
   /* [OPERATOR_LE]          = */ASM_CMP_LE,
   /* [OPERATOR_GR]          = */ASM_CMP_GR,
   /* [OPERATOR_GE]          = */ASM_CMP_GE,
   /* [OPERATOR_ITERSELF]    = */ASM_ITERSELF,
   /* [OPERATOR_SIZE]        = */ASM_GETSIZE,
   /* [OPERATOR_CONTAINS]    = */ASM_CONTAINS,
   /* [OPERATOR_GETITEM]     = */ASM_GETITEM,
   /* [OPERATOR_DELITEM]     = */ASM_DELITEM,
   /* [OPERATOR_SETITEM]     = */ASM_SETITEM,
   /* [OPERATOR_GETRANGE]    = */ASM_GETRANGE,
   /* [OPERATOR_DELRANGE]    = */ASM_DELRANGE,
   /* [OPERATOR_SETRANGE]    = */ASM_SETRANGE,
   /* [OPERATOR_GETATTR]     = */ASM_GETATTR,
   /* [OPERATOR_DELATTR]     = */ASM_DELATTR,
   /* [OPERATOR_SETATTR]     = */ASM_SETATTR,
   /* [OPERATOR_ENUMATTR]    = */0,
   /* [OPERATOR_ENTER]       = */ASM_ENTER,
   /* [OPERATOR_LEAVE]       = */ASM_LEAVE
};


INTDEF int DCALL
ast_gen_symbol_inplace(struct symbol *__restrict sym,
                       struct ast *operand,
                       struct ast *__restrict ddi_ast,
                       uint16_t inplace_operator_name,
                       bool is_post_operator,
                       unsigned int gflags) {
 if (asm_can_prefix_symbol(sym)) {
  if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gpush_symbol(sym,ddi_ast)) goto err;
   if (asm_gcopy()) goto err;
  }
  if (operand) {
   if (ast_genasm(operand,ASM_G_FPUSHRES))
       goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gprefix_symbol(sym,ddi_ast)) goto err;
   if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
   asm_decsp();
  } else {
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gprefix_symbol(sym,ddi_ast)) goto err;
   if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
  }
  /* Inplace operations return the symbol */
  if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
   if (asm_gpush_symbol(sym,ddi_ast)) goto err;
  }
  return 0;
 }

 if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
  if (asm_gpush_symbol(sym,ddi_ast)) goto err;
  if (asm_gcopy()) goto err;  /* sym, COPY(sym) */
 }
 if (asm_gpush_symbol(sym,ddi_ast)) goto err;
 if (operand) {
  if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* sym, operand */
  if (asm_putddi(ddi_ast)) goto err;
  if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
  if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
  asm_ddicsp(); /* sym += operand */
 } else {
  if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
  if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
  asm_dicsp(); /* ++sym */
 }
 if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
  if (asm_gdup()) goto err;
 }
 return asm_gpop_symbol(sym,ddi_ast);
err:
 return -1;
}


INTERN int DCALL
ast_gen_setattr_inplace(struct ast *__restrict base,
                        struct ast *__restrict name,
                        struct ast *operand,
                        struct ast *__restrict ddi_ast,
                        uint16_t inplace_operator_name,
                        bool is_post_operator,
                        unsigned int gflags) {
 /* <base>.<name> += operand; */
 ASSERT(OPERATOR_ISINPLACE(inplace_operator_name));
 if (name->a_type == AST_CONSTEXPR &&
     DeeString_Check(name->a_constexpr)) {
  /* Special case: Name is a constant string. */
  int32_t cid = asm_newconst(name->a_constexpr);
  if unlikely(cid < 0) goto err;
  if (base->a_type == AST_SYM) {
   SYMBOL_INPLACE_UNWIND_ALIAS(base->a_sym);
   if (SYMBOL_TYPE(base->a_sym) == SYMBOL_TYPE_THIS &&
      !SYMBOL_MUST_REFERENCE_TYPEMAY(base->a_sym)) {
    if (asm_ggetattr_this_const((uint16_t)cid)) goto err; /* this.name */
    if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
     if (asm_gdup()) goto err;   /* this.name, this.name */
     if (asm_gcopy()) goto err;  /* this.name, COPY(this.name) */
     if (asm_gswap()) goto err;  /* COPY(this.name), this.name */
    }
    if (operand) {
     if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* this.name, operand */
     if (asm_putddi(ddi_ast)) goto err;
     if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
     if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
     asm_ddicsp(); /* this.name += operand */
    } else {
     if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
     if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
     asm_dicsp(); /* ++this.name */
    }
    if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
     if (asm_gdup()) goto err;    /* this.name += operand, this.name += operand */
    }
    if (asm_gsetattr_this_const((uint16_t)cid)) goto err; /* . */
    goto done;
   }
  }
  if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;   /* base */
  if (asm_putddi(ddi_ast)) goto err;
  if (asm_gdup()) goto err;                        /* base, base */
  if (asm_ggetattr_const((uint16_t)cid)) goto err; /* base, base.name */
  if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
   if (asm_gdup()) goto err;   /* base, base.name, base.name */
   if (asm_gcopy()) goto err;  /* base, base.name, COPY(base.name) */
   if (asm_grrot(3)) goto err; /* COPY(base.name), base, base.name */
  }
  if (operand) {
   if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* base, base.name, operand */
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
   if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
   asm_ddicsp(); /* base, base.name += operand */
  } else {
   if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
   if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
   asm_dicsp(); /* base, ++base.name */
  }
  if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
   if (asm_gdup()) goto err;    /* base, base.name += operand, base.name += operand */
   if (asm_grrot(3)) goto err;  /* base.name += operand, base, base.name += operand */
  }
  if (asm_gsetattr_const((uint16_t)cid)) goto err; /* . */
  goto done;
 }
 if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;
 if (asm_putddi(ddi_ast)) goto err;
 if (asm_gdup()) goto err;
 if (ast_genasm(name,ASM_G_FPUSHRES)) goto err;
 if (asm_putddi(ddi_ast)) goto err;
 if (asm_gdup()) goto err;                         /* base, base, name, name */
 if (asm_grrot(3)) goto err;                       /* base, name, base, name */
 if (asm_ggetattr()) goto err;                     /* base, name, base.name */
 if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
  if (asm_gdup()) goto err;   /* base, name, base.name, base.name */
  if (asm_gcopy()) goto err;  /* base, name, base.name, COPY(base.name) */
  if (asm_grrot(4)) goto err; /* COPY(base.name), base, name, base.name */
 }
 if (operand) {
  if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* base, name, base.name, operand */
  if (asm_putddi(ddi_ast)) goto err;
  if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
  if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
  asm_ddicsp(); /* base, name, base.name += operand */
 } else {
  if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
  if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
  asm_dicsp(); /* base, name, ++base.name */
 }
 if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
  if (asm_gdup()) goto err;    /* base, name, base.name += operand, base.name += operand */
  if (asm_grrot(4)) goto err;  /* base.name += operand, base, name, base.name += operand */
 }
 if (asm_gsetattr()) goto err; /* . */
done:
 return 0;
err:
 return -1;
}

INTERN int DCALL
ast_gen_setitem_inplace(struct ast *__restrict base,
                        struct ast *__restrict index,
                        struct ast *operand,
                        struct ast *__restrict ddi_ast,
                        uint16_t inplace_operator_name,
                        bool is_post_operator,
                        unsigned int gflags) {
 /* <base>[<index>] += operand; */
 ASSERT(OPERATOR_ISINPLACE(inplace_operator_name));
 if (index->a_type == AST_CONSTEXPR) {
  /* Special case: The index is constant. */
  if (DeeInt_Check(index->a_constexpr)) {
   int32_t int_index;
   if (DeeInt_TryAsS32(index->a_constexpr,&int_index) &&
       int_index >= INT16_MIN && int_index <= INT16_MAX) {
    if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;   /* base */
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_gdup()) goto err;                        /* base, base */
    if (asm_ggetitem_index((int16_t)int_index)) goto err; /* base, base[index] */
    if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
     if (asm_gdup()) goto err;   /* base, base[index], base[index] */
     if (asm_gcopy()) goto err;  /* base, base[index], COPY(base[index]) */
     if (asm_grrot(3)) goto err; /* COPY(base[index]), base, base[index] */
    }
    if (operand) {
     if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* base, base[index], operand */
     if (asm_putddi(ddi_ast)) goto err;
     if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
     if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
     asm_ddicsp(); /* base, base[index] += operand */
    } else {
     if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
     if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
     asm_dicsp(); /* base, ++base[index] */
    }
    if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
     if (asm_gdup()) goto err;    /* base, base[index] += operand, base[index] += operand */
     if (asm_grrot(3)) goto err;  /* base[index] += operand, base, base[index] += operand */
    }
    if (asm_gsetitem_index((int16_t)int_index)) goto err; /* . */
    goto done;
   }
  }
  if (asm_allowconst(index->a_constexpr)) {
   int32_t cid = asm_newconst(index->a_constexpr);
   if unlikely(cid < 0) goto err;
   if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;   /* base */
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup()) goto err;                        /* base, base */
   if (asm_ggetitem_const((uint16_t)cid)) goto err; /* base, base[index] */
   if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
    if (asm_gdup()) goto err;   /* base, base[index], base[index] */
    if (asm_gcopy()) goto err;  /* base, base[index], COPY(base[index]) */
    if (asm_grrot(3)) goto err; /* COPY(base[index]), base, base[index] */
   }
   if (operand) {
    if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* base, base[index], operand */
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
    if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
    asm_ddicsp(); /* base, base[index] += operand */
   } else {
    if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
    if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
    asm_dicsp(); /* base, ++base[index] */
   }
   if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
    if (asm_gdup()) goto err;    /* base, base[index] += operand, base[index] += operand */
    if (asm_grrot(3)) goto err;  /* base[index] += operand, base, base[index] += operand */
   }
   if (asm_gsetitem_const((uint16_t)cid)) goto err; /* . */
   goto done;
  }
 }
 if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;
 if (asm_putddi(ddi_ast)) goto err;
 if (asm_gdup()) goto err;
 if (ast_genasm(index,ASM_G_FPUSHRES)) goto err;
 if (asm_putddi(ddi_ast)) goto err;
 if (asm_gdup()) goto err;                         /* base, base, index, index */
 if (asm_grrot(3)) goto err;                       /* base, index, base, index */
 if (asm_ggetitem()) goto err;                     /* base, index, base[index] */
 if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
  if (asm_gdup()) goto err;   /* base, index, base[index], base[index] */
  if (asm_gcopy()) goto err;  /* base, index, base[index], COPY(base[index]) */
  if (asm_grrot(4)) goto err; /* COPY(base[index]), base, index, base[index] */
 }
 if (operand) {
  if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* base, index, base[index], operand */
  if (asm_putddi(ddi_ast)) goto err;
  if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
  if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
  asm_ddicsp(); /* base, index, base[index] += operand */
 } else {
  if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
  if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
  asm_dicsp(); /* base, index, ++base[index] */
 }
 if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
  if (asm_gdup()) goto err;    /* base, index, base[index] += operand, base[index] += operand */
  if (asm_grrot(4)) goto err;  /* base[index] += operand, base, index, base[index] += operand */
 }
 if (asm_gsetitem()) goto err; /* . */
done:
 return 0;
err:
 return -1;
}
INTERN int DCALL
ast_gen_setrange_inplace(struct ast *__restrict base,
                         struct ast *__restrict start,
                         struct ast *__restrict end,
                         struct ast *operand,
                         struct ast *__restrict ddi_ast,
                         uint16_t inplace_operator_name,
                         bool is_post_operator,
                         unsigned int gflags) {
 /* <base>[<start>:<end>] += operand; */
 ASSERT(OPERATOR_ISINPLACE(inplace_operator_name));
 /* Special case: The start or end index is constant. */
 if (start->a_type == AST_CONSTEXPR) {
  if (DeeNone_Check(start->a_constexpr)) {
   if (end->a_type == AST_CONSTEXPR) {
    int32_t end_index;
    if (DeeInt_Check(end->a_constexpr) &&
        DeeInt_TryAsS32(end->a_constexpr,&end_index) &&
        end_index >= INT16_MIN && end_index <= INT16_MAX) {
     /* <base>[none:43] += operand; (Uses `asm_ggetrange_ni') */
     if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;   /* base */
     if (asm_putddi(ddi_ast)) goto err;
     if (asm_gdup()) goto err;                        /* base, base */
     if (asm_ggetrange_ni((int16_t)end_index)) goto err; /* base, base[none:43] */
     if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
      if (asm_gdup()) goto err;   /* base, base[none:43], base[none:43] */
      if (asm_gcopy()) goto err;  /* base, base[none:43], COPY(base[none:43]) */
      if (asm_grrot(3)) goto err; /* COPY(base[none:43]), base, base[none:43] */
     }
     if (operand) {
      if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* base, base[none:43], operand */
      if (asm_putddi(ddi_ast)) goto err;
      if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
      if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
      asm_ddicsp(); /* base, base[none:43] += operand */
     } else {
      if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
      if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
      asm_dicsp(); /* base, ++base[none:43] */
     }
     if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
      if (asm_gdup()) goto err;    /* base, base[none:43] += operand, base[none:43] += operand */
      if (asm_grrot(3)) goto err;  /* base[none:43] += operand, base, base[none:43] += operand */
     }
     if (asm_gsetrange_ni((int16_t)end_index)) goto err; /* . */
     goto done;
    }
   }
   /* <base>[none:<end>] += operand; (Uses `asm_ggetrange_ip') */
   if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup()) goto err;
   if (ast_genasm(end,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup()) goto err;         /* base, base, end, end */
   if (asm_grrot(3)) goto err;       /* base, end, base, end */
   if (asm_ggetrange_np()) goto err; /* base, end, base[none:end] */
   if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
    if (asm_gdup()) goto err;   /* base, end, base[none:end], base[none:end] */
    if (asm_gcopy()) goto err;  /* base, end, base[none:end], COPY(base[none:end]) */
    if (asm_grrot(4)) goto err; /* COPY(base[none:end]), base, end, base[none:end] */
   }
   if (operand) {
    if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* base, end, base[none:end], operand */
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
    if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
    asm_ddicsp(); /* base, end, base[none:end] += operand */
   } else {
    if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
    if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
    asm_dicsp(); /* base, end, ++base[none:end] */
   }
   if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
    if (asm_gdup()) goto err;    /* base, end, base[none:end] += operand, base[none:end] += operand */
    if (asm_grrot(4)) goto err;  /* base[none:end] += operand, base, end, base[none:end] += operand */
   }
   if (asm_gsetrange_np()) goto err; /* . */
   goto done;
  } else if (DeeInt_Check(start->a_constexpr)) {
   int32_t start_index;
   if (DeeInt_TryAsS32(start->a_constexpr,&start_index) &&
       start_index >= INT16_MIN && start_index <= INT16_MAX) {
    if (end->a_type == AST_CONSTEXPR) {
     int32_t end_index;
     if (DeeNone_Check(end->a_constexpr)) {
      /* <base>[42:none] += operand; (Uses `asm_ggetrange_in') */
      if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;   /* base */
      if (asm_putddi(ddi_ast)) goto err;
      if (asm_gdup()) goto err;                        /* base, base */
      if (asm_ggetrange_in((int16_t)start_index)) goto err; /* base, base[42:none] */
      if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
       if (asm_gdup()) goto err;   /* base, base[42:none], base[42:none] */
       if (asm_gcopy()) goto err;  /* base, base[42:none], COPY(base[42:none]) */
       if (asm_grrot(3)) goto err; /* COPY(base[42:none]), base, base[42:none] */
      }
      if (operand) {
       if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* base, base[42:none], operand */
       if (asm_putddi(ddi_ast)) goto err;
       if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
       if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
       asm_ddicsp(); /* base, base[42:none] += operand */
      } else {
       if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
       if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
       asm_dicsp(); /* base, ++base[42:none] */
      }
      if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
       if (asm_gdup()) goto err;    /* base, base[42:none] += operand, base[42:none] += operand */
       if (asm_grrot(3)) goto err;  /* base[42:none] += operand, base, base[42:none] += operand */
      }
      if (asm_gsetrange_in((int16_t)start_index)) goto err; /* . */
      goto done;
     } else if (DeeInt_Check(end->a_constexpr) &&
                DeeInt_TryAsS32(end->a_constexpr,&end_index) &&
                end_index >= INT16_MIN && end_index <= INT16_MAX) {
      /* <base>[42:43] += operand; (Uses `asm_ggetrange_ii') */
      if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;   /* base */
      if (asm_putddi(ddi_ast)) goto err;
      if (asm_gdup()) goto err;                        /* base, base */
      if (asm_ggetrange_ii((int16_t)start_index,(int16_t)end_index)) goto err; /* base, base[42:43] */
      if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
       if (asm_gdup()) goto err;   /* base, base[42:43], base[42:43] */
       if (asm_gcopy()) goto err;  /* base, base[42:43], COPY(base[42:43]) */
       if (asm_grrot(3)) goto err; /* COPY(base[42:43]), base, base[42:43] */
      }
      if (operand) {
       if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* base, base[42:43], operand */
       if (asm_putddi(ddi_ast)) goto err;
       if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
       if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
       asm_ddicsp(); /* base, base[42:43] += operand */
      } else {
       if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
       if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
       asm_dicsp(); /* base, ++base[42:43] */
      }
      if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
       if (asm_gdup()) goto err;    /* base, base[42:43] += operand, base[42:43] += operand */
       if (asm_grrot(3)) goto err;  /* base[42:43] += operand, base, base[42:43] += operand */
      }
      if (asm_gsetrange_ii((int16_t)start_index,(int16_t)end_index)) goto err; /* . */
      goto done;
     }
    }
    /* <base>[42:<end>] += operand; (Uses `asm_ggetrange_ip') */
    if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_gdup()) goto err;
    if (ast_genasm(end,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_gdup()) goto err;                             /* base, base, end, end */
    if (asm_grrot(3)) goto err;                           /* base, end, base, end */
    if (asm_ggetrange_ip((int16_t)start_index)) goto err; /* base, end, base[42:end] */
    if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
     if (asm_gdup()) goto err;   /* base, end, base[42:end], base[42:end] */
     if (asm_gcopy()) goto err;  /* base, end, base[42:end], COPY(base[42:end]) */
     if (asm_grrot(4)) goto err; /* COPY(base[42:end]), base, end, base[42:end] */
    }
    if (operand) {
     if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* base, end, base[42:end], operand */
     if (asm_putddi(ddi_ast)) goto err;
     if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
     if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
     asm_ddicsp(); /* base, end, base[42:end] += operand */
    } else {
     if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
     if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
     asm_dicsp(); /* base, end, ++base[42:end] */
    }
    if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
     if (asm_gdup()) goto err;    /* base, end, base[42:end] += operand, base[42:end] += operand */
     if (asm_grrot(4)) goto err;  /* base[42:end] += operand, base, end, base[42:end] += operand */
    }
    if (asm_gsetrange_ip((int16_t)start_index)) goto err; /* . */
    goto done;
   }
  }
 }
 if (end->a_type == AST_CONSTEXPR) {
  int32_t end_index;
  if (DeeNone_Check(end->a_constexpr)) {
   /* <base>[start:none] += operand; (Uses `asm_ggetrange_pn') */
   if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup()) goto err;
   if (ast_genasm(start,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup()) goto err;         /* base, base, start, start */
   if (asm_grrot(3)) goto err;       /* base, start, base, start */
   if (asm_ggetrange_pn()) goto err; /* base, start, base[start:none] */
   if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
    if (asm_gdup()) goto err;   /* base, start, base[start:none], base[start:none] */
    if (asm_gcopy()) goto err;  /* base, start, base[start:none], COPY(base[start:none]) */
    if (asm_grrot(4)) goto err; /* COPY(base[start:none]), base, start, base[start:none] */
   }
   if (operand) {
    if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* base, start, base[start:none], operand */
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
    if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
    asm_ddicsp(); /* base, start, base[start:none] += operand */
   } else {
    if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
    if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
    asm_dicsp(); /* base, start, ++base[start:none] */
   }
   if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
    if (asm_gdup()) goto err;    /* base, start, base[start:none] += operand, base[start:none] += operand */
    if (asm_grrot(4)) goto err;  /* base[start:none] += operand, base, start, base[start:none] += operand */
   }
   if (asm_gsetrange_pn()) goto err; /* . */
   goto done;
  } else if (DeeInt_Check(end->a_constexpr) &&
             DeeInt_TryAsS32(end->a_constexpr,&end_index) &&
             end_index >= INT16_MIN && end_index <= INT16_MAX) {
   /* <base>[start:42] += operand; (Uses `asm_ggetrange_pi') */
   if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup()) goto err;
   if (ast_genasm(start,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ddi_ast)) goto err;
   if (asm_gdup()) goto err;                           /* base, base, start, start */
   if (asm_grrot(3)) goto err;                         /* base, start, base, start */
   if (asm_ggetrange_pi((int16_t)end_index)) goto err; /* base, start, base[start:42] */
   if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
    if (asm_gdup()) goto err;   /* base, start, base[start:42], base[start:42] */
    if (asm_gcopy()) goto err;  /* base, start, base[start:42], COPY(base[start:42]) */
    if (asm_grrot(4)) goto err; /* COPY(base[start:42]), base, start, base[start:42] */
   }
   if (operand) {
    if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* base, start, base[start:42], operand */
    if (asm_putddi(ddi_ast)) goto err;
    if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
    if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
    asm_ddicsp(); /* base, start, base[start:42] += operand */
   } else {
    if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
    if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
    asm_dicsp(); /* base, start, ++base[start:42] */
   }
   if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
    if (asm_gdup()) goto err;    /* base, start, base[start:42] += operand, base[start:42] += operand */
    if (asm_grrot(4)) goto err;  /* base[start:42] += operand, base, start, base[start:42] += operand */
   }
   if (asm_gsetrange_pi((int16_t)end_index)) goto err; /* . */
   goto done;
  }
 }
 if (ast_genasm(base,ASM_G_FPUSHRES)) goto err;   /* base */
 if (asm_putddi(ddi_ast)) goto err;
 if (asm_gdup()) goto err;                        /* base, base */
 if (ast_genasm(start,ASM_G_FPUSHRES)) goto err;  /* base, base, start */
 if (asm_putddi(ddi_ast)) goto err;
 if (asm_gdup()) goto err;                        /* base, base, start, start */
 if (asm_grrot(3)) goto err;                      /* base, start, base, start */
 if (ast_genasm(end,ASM_G_FPUSHRES)) goto err;    /* base, start, base, start, end */
 if (asm_putddi(ddi_ast)) goto err;
 if (asm_gdup()) goto err;                        /* base, start, base, start, end, end */
 if (asm_grrot(4)) goto err;                      /* base, start, end, base, start, end */
 if (asm_ggetrange()) goto err;                   /* base, start, end, base[start:end] */
 if ((gflags & ASM_G_FPUSHRES) && is_post_operator) {
  if (asm_gdup()) goto err;   /* base, start, end, base[start:end], base[start:end] */
  if (asm_gcopy()) goto err;  /* base, start, end, base[start:end], COPY(base[start:end]) */
  if (asm_grrot(5)) goto err; /* COPY(base[start:end]), base, start, end, base[start:end] */
 }
 if (operand) {
  if (ast_genasm(operand,ASM_G_FPUSHRES)) goto err; /* base, start, end, base[start:end], operand */
  if (asm_putddi(ddi_ast)) goto err;
  if (asm_pstack(current_assembler.a_stackcur-2)) goto err;
  if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
  asm_ddicsp(); /* base, start, end, base[start:end] += operand */
 } else {
  if (asm_pstack(current_assembler.a_stackcur-1)) goto err;
  if (asm_put(operator_instr_table[inplace_operator_name])) goto err;
  asm_dicsp(); /* base, start, end, ++base[start:end] */
 }
 if ((gflags & ASM_G_FPUSHRES) && !is_post_operator) {
  if (asm_gdup()) goto err;    /* base, start, end, base[start:end] += operand, base[start:end] += operand */
  if (asm_grrot(5)) goto err;  /* base[start:end] += operand, base, start, end, base[start:end] += operand */
 }
 if (asm_gsetrange()) goto err; /* . */
done:
 return 0;
err:
 return -1;
}



DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENINPLACE_C */
