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
#ifndef GUARD_DEEMON_COMPILER_ASM_GENASM_C
#define GUARD_DEEMON_COMPILER_ASM_GENASM_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/code.h>
#include <deemon/module.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/asm.h>
#include <deemon/string.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/tuple.h>
#include <deemon/dict.h>
#include <deemon/hashset.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/optimize.h>
#include <deemon/util/string.h>

#include <string.h>

#include "../../runtime/builtin.h"

DECL_BEGIN

STATIC_ASSERT(ASM_NOT&1);
STATIC_ASSERT(!(ASM_BOOL&1));
STATIC_ASSERT(ASM_BOOL == (ASM_NOT^1));
STATIC_ASSERT((ASM_INCPOST & 0xff) == ASM_INC);
STATIC_ASSERT((ASM_DECPOST & 0xff) == ASM_DEC);
STATIC_ASSERT((ASM_INCPOST & 0xff00) == (ASM_DECPOST & 0xff00));

/* Quick translation table for most basic operator instruction codes. */
#ifdef _MSC_VER
extern
#endif
INTERN instruction_t const operator_instr_table[] = {
   /* [OPERATOR_CONSTRUCTOR] = */0,
   /* [OPERATOR_COPY]        = */ASM_COPY,
   /* [OPERATOR_DEEPCOPY]    = */ASM_DEEPCOPY,
   /* [OPERATOR_DESTRUCTOR]  = */0,
   /* [OPERATOR_ASSIGN]      = */ASM_ASSIGN,
   /* [OPERATOR_MOVEASSIGN]  = */ASM_MOVE_ASSIGN,
   /* [OPERATOR_STR]         = */ASM_STR,
   /* [OPERATOR_REPR]        = */ASM_REPR,
   /* [OPERATOR_BOOL]        = */ASM_BOOL,
   /* [OPERATOR_ITERNEXT]    = */0, /* ASM_ITERNEXT - Extended opcodes can't be used here. */
   /* [OPERATOR_CALL]        = */ASM_CALL_TUPLE,
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

#ifdef _MSC_VER
extern
#endif
INTERN uint8_t const operator_opcount_table[OPERATOR_USERCOUNT] = {
#define OPCOUNT_OPCOUNTMASK 0x0f
#define OPCOUNT_RESULTMASK  0xf0
#define OPCOUNT_INSTRIN     0x00 /* The instruction intrinsically pushes a result. */
#define OPCOUNT_PUSHFIRST   0x10 /* You must re-return the first operand. */
#define OPCOUNT_PUSHSECOND  0x20 /* You must re-return the second operand. */
#define OPCOUNT_PUSHTHIRD   0x30 /* You must re-return the third operand. */
#define OPCOUNT_PUSHFOURTH  0x40 /* You must re-return the fourth operand. */
#define OPCOUNT_POPPUSHNONE 0x70 /* You must pop one object, the push `none'. */
#define OPCOUNT_PUSHNONE    0x80 /* You must re-return none. */
#define ENTRY(push_mode,opcount) (push_mode|opcount)
   /* [OPERATOR_CONSTRUCTOR] = */0,
   /* [OPERATOR_COPY]        = */ENTRY(OPCOUNT_INSTRIN,1),
   /* [OPERATOR_DEEPCOPY]    = */ENTRY(OPCOUNT_INSTRIN,1),
   /* [OPERATOR_DESTRUCTOR]  = */0,
   /* [OPERATOR_ASSIGN]      = */ENTRY(OPCOUNT_PUSHFIRST,2),
   /* [OPERATOR_MOVEASSIGN]  = */ENTRY(OPCOUNT_PUSHFIRST,2),
   /* [OPERATOR_STR]         = */ENTRY(OPCOUNT_INSTRIN,1),
   /* [OPERATOR_REPR]        = */ENTRY(OPCOUNT_INSTRIN,1),
   /* [OPERATOR_BOOL]        = */ENTRY(OPCOUNT_INSTRIN,1),
   /* [OPERATOR_ITERNEXT]    = */0, /* ASM_ITERNEXT - Extended opcodes can't be used here. */
   /* [OPERATOR_CALL]        = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_INT]         = */ENTRY(OPCOUNT_INSTRIN,1),
   /* [OPERATOR_FLOAT]       = */0,
   /* [OPERATOR_INV]         = */ENTRY(OPCOUNT_INSTRIN,1),
   /* [OPERATOR_POS]         = */ENTRY(OPCOUNT_INSTRIN,1),
   /* [OPERATOR_NEG]         = */ENTRY(OPCOUNT_INSTRIN,1),
   /* [OPERATOR_ADD]         = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_SUB]         = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_MUL]         = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_DIV]         = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_MOD]         = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_SHL]         = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_SHR]         = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_AND]         = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_OR]          = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_XOR]         = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_POW]         = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_INC]         = */ENTRY(OPCOUNT_PUSHFIRST,1), /* Inplace! */
   /* [OPERATOR_DEC]         = */ENTRY(OPCOUNT_PUSHFIRST,1), /* Inplace! */
   /* [OPERATOR_INPLACE_ADD] = */ENTRY(OPCOUNT_PUSHFIRST,2), /* Inplace! */
   /* [OPERATOR_INPLACE_SUB] = */ENTRY(OPCOUNT_PUSHFIRST,2), /* Inplace! */
   /* [OPERATOR_INPLACE_MUL] = */ENTRY(OPCOUNT_PUSHFIRST,2), /* Inplace! */
   /* [OPERATOR_INPLACE_DIV] = */ENTRY(OPCOUNT_PUSHFIRST,2), /* Inplace! */
   /* [OPERATOR_INPLACE_MOD] = */ENTRY(OPCOUNT_PUSHFIRST,2), /* Inplace! */
   /* [OPERATOR_INPLACE_SHL] = */ENTRY(OPCOUNT_PUSHFIRST,2), /* Inplace! */
   /* [OPERATOR_INPLACE_SHR] = */ENTRY(OPCOUNT_PUSHFIRST,2), /* Inplace! */
   /* [OPERATOR_INPLACE_AND] = */ENTRY(OPCOUNT_PUSHFIRST,2), /* Inplace! */
   /* [OPERATOR_INPLACE_OR]  = */ENTRY(OPCOUNT_PUSHFIRST,2),  /* Inplace! */
   /* [OPERATOR_INPLACE_XOR] = */ENTRY(OPCOUNT_PUSHFIRST,2), /* Inplace! */
   /* [OPERATOR_INPLACE_POW] = */ENTRY(OPCOUNT_PUSHFIRST,2), /* Inplace! */
   /* [OPERATOR_HASH]        = */0,
   /* [OPERATOR_EQ]          = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_NE]          = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_LO]          = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_LE]          = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_GR]          = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_GE]          = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_ITERSELF]    = */ENTRY(OPCOUNT_INSTRIN,1),
   /* [OPERATOR_SIZE]        = */ENTRY(OPCOUNT_INSTRIN,1),
   /* [OPERATOR_CONTAINS]    = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_GETITEM]     = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_DELITEM]     = */ENTRY(OPCOUNT_PUSHNONE,2),
   /* [OPERATOR_SETITEM]     = */ENTRY(OPCOUNT_PUSHTHIRD,3),
   /* [OPERATOR_GETRANGE]    = */ENTRY(OPCOUNT_INSTRIN,3),
   /* [OPERATOR_DELRANGE]    = */ENTRY(OPCOUNT_PUSHNONE,3),
   /* [OPERATOR_SETRANGE]    = */ENTRY(OPCOUNT_PUSHFOURTH,4),
   /* [OPERATOR_GETATTR]     = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_DELATTR]     = */ENTRY(OPCOUNT_PUSHNONE,2),
   /* [OPERATOR_SETATTR]     = */ENTRY(OPCOUNT_PUSHTHIRD,3),
   /* [OPERATOR_ENUMATTR]    = */0,
   /* [OPERATOR_ENTER]       = */ENTRY(OPCOUNT_POPPUSHNONE,1),
   /* [OPERATOR_LEAVE]       = */ENTRY(OPCOUNT_PUSHNONE,1)
};


/* Given an AST_MULTIPLE:AST_FMULTIPLE_KEEPLAST, recursively unwrap
 * it and generate assembly for all unused operands before returning
 * the actually effective AST (without generating assembly for _it_) */
PRIVATE struct ast *DCALL
ast_unwrap_effective(struct ast *__restrict self) {
 while (self->a_type == AST_MULTIPLE &&
        self->a_flag == AST_FMULTIPLE_KEEPLAST &&
        self->a_multiple.m_astc != 0 &&
        self->a_scope == current_assembler.a_scope) {
  struct ast **iter,**end;
  end = (iter = self->a_multiple.m_astv)+
                self->a_multiple.m_astc-1;
  for (; iter != end; ++iter) {
   if (ast_genasm(*iter,ASM_G_FNORMAL))
       return NULL;
  }
  self = *end;
 }
 return self;
}


#define PRINT_MODE_NORMAL   (ASM_PRINT-ASM_PRINT)    /* Mode: print. */
#define PRINT_MODE_SP       (ASM_PRINT_SP-ASM_PRINT) /* Mode: print, followed by a space. */
#define PRINT_MODE_NL       (ASM_PRINT_NL-ASM_PRINT) /* Mode: print, followed by a new-line. */
#define PRINT_MODE_FILE     (ASM_FPRINT-ASM_PRINT)   /* FLAG: Print to a file. */
#define PRINT_MODE_ALL      (ASM_PRINTALL-ASM_PRINT) /* FLAG: Print elements from a sequence. */

/* Ensure that we can directly encode print modes in instructions. */
STATIC_ASSERT(ASM_PRINT+(PRINT_MODE_NORMAL)                                == ASM_PRINT);
STATIC_ASSERT(ASM_PRINT+(PRINT_MODE_SP)                                    == ASM_PRINT_SP);
STATIC_ASSERT(ASM_PRINT+(PRINT_MODE_NL)                                    == ASM_PRINT_NL);
STATIC_ASSERT(ASM_PRINT+(PRINT_MODE_NORMAL|PRINT_MODE_FILE)                == ASM_FPRINT);
STATIC_ASSERT(ASM_PRINT+(PRINT_MODE_SP|PRINT_MODE_FILE)                    == ASM_FPRINT_SP);
STATIC_ASSERT(ASM_PRINT+(PRINT_MODE_NL|PRINT_MODE_FILE)                    == ASM_FPRINT_NL);
STATIC_ASSERT(ASM_PRINT+(PRINT_MODE_NORMAL|PRINT_MODE_ALL)                 == ASM_PRINTALL);
STATIC_ASSERT(ASM_PRINT+(PRINT_MODE_SP|PRINT_MODE_ALL)                     == ASM_PRINTALL_SP);
STATIC_ASSERT(ASM_PRINT+(PRINT_MODE_NL|PRINT_MODE_ALL)                     == ASM_PRINTALL_NL);
STATIC_ASSERT(ASM_PRINT+(PRINT_MODE_NORMAL|PRINT_MODE_FILE|PRINT_MODE_ALL) == ASM_FPRINTALL);
STATIC_ASSERT(ASM_PRINT+(PRINT_MODE_SP|PRINT_MODE_FILE|PRINT_MODE_ALL)     == ASM_FPRINTALL_SP);
STATIC_ASSERT(ASM_PRINT+(PRINT_MODE_NL|PRINT_MODE_FILE|PRINT_MODE_ALL)     == ASM_FPRINTALL_NL);
STATIC_ASSERT(ASM_PRINTNL+(PRINT_MODE_FILE)                                == ASM_FPRINTNL);
STATIC_ASSERT(ASM_PRINT_C+(PRINT_MODE_NORMAL)                              == ASM_PRINT_C);
STATIC_ASSERT(ASM_PRINT_C+(PRINT_MODE_SP)                                  == ASM_PRINT_C_SP);
STATIC_ASSERT(ASM_PRINT_C+(PRINT_MODE_NL)                                  == ASM_PRINT_C_NL);
STATIC_ASSERT(ASM_PRINT_C+(PRINT_MODE_NORMAL|PRINT_MODE_FILE)              == ASM_FPRINT_C);
STATIC_ASSERT(ASM_PRINT_C+(PRINT_MODE_SP|PRINT_MODE_FILE)                  == ASM_FPRINT_C_SP);
STATIC_ASSERT(ASM_PRINT_C+(PRINT_MODE_NL|PRINT_MODE_FILE)                  == ASM_FPRINT_C_NL);
STATIC_ASSERT(ASM16_PRINT_C+(PRINT_MODE_NORMAL)                            == ASM16_PRINT_C);
STATIC_ASSERT(ASM16_PRINT_C+(PRINT_MODE_SP)                                == ASM16_PRINT_C_SP);
STATIC_ASSERT(ASM16_PRINT_C+(PRINT_MODE_NL)                                == ASM16_PRINT_C_NL);
STATIC_ASSERT(ASM16_PRINT_C+(PRINT_MODE_NORMAL|PRINT_MODE_FILE)            == ASM16_FPRINT_C);
STATIC_ASSERT(ASM16_PRINT_C+(PRINT_MODE_SP|PRINT_MODE_FILE)                == ASM16_FPRINT_C_SP);
STATIC_ASSERT(ASM16_PRINT_C+(PRINT_MODE_NL|PRINT_MODE_FILE)                == ASM16_FPRINT_C_NL);

PRIVATE DEFINE_STRING(space_string," ");

/* Generate code for the expression `print print_expression...;'
 * @param: mode: The print mode. NOTE: When `PRINT_MODE_FILE' is set,
 *               then the caller must first push the file to print to. */
PRIVATE int DCALL
ast_genprint(instruction_t mode,
             struct ast *__restrict print_expression,
             struct ast *__restrict ddi_ast) {
 print_expression = ast_unwrap_effective(print_expression);
 if (mode&PRINT_MODE_ALL) {
  if (print_expression->a_type == AST_MULTIPLE &&
      print_expression->a_flag != AST_FMULTIPLE_KEEPLAST) {
   struct ast **iter,**end;
   /* Special optimization for printing an expanded, multi-branch.
    * This is actually the most likely case, because something like
    * `print "Hello World";' is actually encoded as `print pack("Hello World")...;' */
   end = (iter = print_expression->a_multiple.m_astv)+
                 print_expression->a_multiple.m_astc;
   if (iter == end) {
    int32_t space_cid;
empty_operand:
    /* Just print an empty line. */
    if ((mode&~(PRINT_MODE_FILE|PRINT_MODE_ALL)) == PRINT_MODE_NL) {
     if (asm_putddi(ddi_ast)) goto err;
     return asm_put((instruction_t)(ASM_PRINTNL+(mode&PRINT_MODE_FILE)));
    }
    /* Nothing needs to be printed _at_ _all_. */
    if ((mode&~(PRINT_MODE_FILE|PRINT_MODE_ALL)) == PRINT_MODE_NORMAL)
        return 0;
    /* Print whitespace. */
    space_cid = asm_newconst((DeeObject *)&space_string);
    if unlikely(space_cid < 0) goto err;
    if (asm_putddi(ddi_ast)) goto err;
    return asm_gprint_const((uint16_t)space_cid);
   }
   /* Print each expression individually. */
   for (; iter != end; ++iter) {
    instruction_t item_mode;
    item_mode = PRINT_MODE_SP|(mode&PRINT_MODE_FILE);
    if (iter == end-1)
        item_mode = (mode&~PRINT_MODE_ALL);
    if unlikely(ast_genprint(item_mode,*iter,ddi_ast)) goto err;
   }
   return 0;
#if 1
  } else if (print_expression->a_type == AST_CONSTEXPR) {
   DREF DeeObject *items,**iter,**end;
   items = DeeTuple_FromSequence(print_expression->a_constexpr);
   if unlikely(!items) { DeeError_Handled(ERROR_HANDLED_RESTORE); goto fallback; }
   if (DeeTuple_IsEmpty(items)) { Dee_Decref(items); goto empty_operand; }
   /* Print each expression individually. */
   end = (iter = DeeTuple_ELEM(items))+DeeTuple_SIZE(items);
   for (; iter != end; ++iter) {
    instruction_t item_mode; int32_t const_cid;
    item_mode = PRINT_MODE_SP|(mode&PRINT_MODE_FILE);
    if (iter == end-1)
        item_mode = (mode&~PRINT_MODE_ALL);
    /* Check if the operand is allowed to appear in constants. */
    if (!asm_allowconst(*iter)) {
     if (asm_gpush_constexpr(*iter)) goto err_items;
     if (asm_putddi(ddi_ast)) goto err;
     if unlikely(asm_put(ASM_PRINT+item_mode)) goto err_items;
     asm_decsp();
    } else {
     const_cid = asm_newconst(*iter);
     if unlikely(const_cid < 0) goto err_items;
     if (asm_putddi(ddi_ast)) goto err;
     if (asm_put816(ASM_PRINT_C+item_mode,(uint16_t)const_cid)) goto err_items;
    }
   }
   Dee_Decref(items);
   return 0;
err_items:
   Dee_Decref(items);
   goto err;
#endif
  }
 } else if (print_expression->a_type == AST_CONSTEXPR &&
            asm_allowconst(print_expression->a_constexpr)) {
  /* Special instructions exist for direct printing of constants. */
  int32_t const_cid = asm_newconst(print_expression->a_constexpr);
  if unlikely(const_cid < 0) goto err;
  if (asm_putddi(ddi_ast)) goto err;
  return asm_put816(ASM_PRINT_C+mode,(uint16_t)const_cid);
 }
fallback:
 /* Fallback: Compile the print expression, then print it as an expanded sequence. */
 if (print_expression->a_type == AST_EXPAND && !(mode&PRINT_MODE_ALL)) {
  if (ast_genasm(print_expression->a_expand,ASM_G_FPUSHRES)) goto err;
  if (asm_putddi(ddi_ast)) goto err;
  if (asm_put(ASM_PRINTALL+mode)) goto err;
 } else {
  if (ast_genasm(print_expression,ASM_G_FPUSHRES)) goto err;
  if (asm_putddi(ddi_ast)) goto err;
  if (asm_put(ASM_PRINT+mode)) goto err;
 }
 asm_decsp(); /* Consume the print expression. */
 return 0;
err:
 return -1;
}


INTERN struct module_symbol *DCALL
get_module_symbol(DeeModuleObject *__restrict module,
                  DeeStringObject *__restrict name) {
 dhash_t i,perturb;
 dhash_t hash = DeeString_Hash((DeeObject *)name);
 perturb = i = MODULE_HASHST(module,hash);
 for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
  struct module_symbol *item = MODULE_HASHIT(module,i);
  if (!item->ss_name) break; /* Not found */
  if (item->ss_hash != hash) continue; /* Non-matching hash */
  if (!MODULE_SYMBOL_EQUALS(item,DeeString_STR(name),DeeString_SIZE(name)))
      continue; /* Differing strings. */
  return item; /* Found it! */
 }
 return NULL;
}



struct seqops {
    /* Opcodes are encoded in big-endian.
     * When the mask 0xff00 is ZERO, the opcode is a single byte long. */
    DeeTypeObject *so_typ;    /* The deemon type for this sequence. */
    uint16_t       so_pck[2]; /* Pack - [0]: 8-bit; [1]: 16-bit; */
    uint16_t       so_cas;    /* Cast */
};


/* Make sure that generic sequences are encoded as either
 * tuples or lists with used in non-specific contexts. */
STATIC_ASSERT((AST_FMULTIPLE_GENERIC&3) <= 1);
STATIC_ASSERT((AST_FMULTIPLE_GENERIC_KEYS&3) == (AST_FMULTIPLE_DICT&3));

PRIVATE struct seqops seqops_info[4] = {
    /* [AST_FMULTIPLE_TUPLE   & 3] = */{ &DeeTuple_Type,   { ASM_PACK_TUPLE,   ASM16_PACK_TUPLE },   ASM_CAST_TUPLE },
    /* [AST_FMULTIPLE_LIST    & 3] = */{ &DeeList_Type,    { ASM_PACK_LIST,    ASM16_PACK_LIST },    ASM_CAST_LIST },
    /* [AST_FMULTIPLE_HASHSET & 3] = */{ &DeeHashSet_Type, { ASM_PACK_HASHSET, ASM16_PACK_HASHSET }, ASM_CAST_HASHSET },
    /* [AST_FMULTIPLE_DICT    & 3] = */{ &DeeDict_Type,    { ASM_PACK_DICT,    ASM16_PACK_DICT },    ASM_CAST_DICT }
};

/* @param: type: One of `AST_FMULTIPLE_*' */
PRIVATE int DCALL pack_sequence(uint16_t type, uint16_t num_args) {
 uint16_t (*popcode)[2],op;
 ASSERT(type != AST_FMULTIPLE_KEEPLAST);
 if (AST_FMULTIPLE_ISDICT(type)) {
  /* Special case: dict. */
  if unlikely((num_args&1) && asm_gpop())
     goto err; /* Discard superfluous element. */
  num_args /= 2;
  asm_subsp(num_args); /* Adjust for the second half. */
 }
 popcode = &seqops_info[type & 3].so_pck;
 if (num_args > UINT8_MAX) {
  op = (*popcode)[1];
  if (op & 0xff00 && asm_put((op & 0xff00) >> 8)) goto err;
  if (asm_putimm16(op & 0xff,num_args)) goto err;
 } else {
  op = (*popcode)[0];
  if (op & 0xff00 && asm_put((op & 0xff00) >> 8)) goto err;
  if (asm_putimm8(op & 0xff,(uint8_t)num_args)) goto err;
 }
 /* Adjust the stack. */
 asm_subsp(num_args);
 asm_incsp();

 return 0;
err:
 return -1;
}
PRIVATE int DCALL cast_sequence(uint16_t type) {
 uint16_t op = seqops_info[type & 3].so_cas;
 ASSERT(type != AST_FMULTIPLE_KEEPLAST);
 if (op & 0xff00 && asm_put((op & 0xff00) >> 8)) return -1;
 return asm_put(op & 0xff);
}




/* The heart of the compiler: The AST --> Assembly generator. */
INTERN int (DCALL ast_genasm)(struct ast *__restrict self,
                              unsigned int gflags) {
#define PUSH_RESULT   (gflags & ASM_G_FPUSHRES)
 ASSERT_AST(self);
 /* Set the given AST as location information for error messages. */
 ASM_PUSH_LOC(&self->a_ddi);
 ASM_PUSH_SCOPE(self->a_scope,err);
 switch (self->a_type) {

 case AST_CONSTEXPR:
  if (!PUSH_RESULT) break;
  if (asm_putddi(self)) goto err;
  if (asm_gpush_constexpr(self->a_constexpr)) goto err;
  break;

 {
  struct symbol *sym;
 case AST_SYM:
  sym = self->a_sym;
  SYMBOL_INPLACE_UNWIND_ALIAS(sym);
  /* Make sure to still generate code if the symbol access could have side-effects. */
  if (!PUSH_RESULT &&
      !symbol_get_haseffect(sym,self->a_scope))
       break;
  if (asm_putddi(self)) goto err;
  if ((gflags & ASM_G_FLAZYBOOL) &&
       SYMBOL_TYPE(sym) == SYMBOL_TYPE_ARG &&
      !SYMBOL_MUST_REFERENCE(sym) &&
      (current_basescope->bs_flags & CODE_FVARARGS) &&
       DeeBaseScope_IsArgVarArgs(current_basescope,sym->s_symid)) {
   /* Special case: If the caller accesses the varargs-symbol in a boolean-context,
    *               then we can simply check if the number of varargs is non-zero,
    *               emulating the behavior of tuple's `operator bool()'.
    * NOTE: Here we check if the number of varargs is greater than the
    *       number if optional arguments, in which case more arguments
    *       were given than optional were required. */
   if (asm_gcmp_gr_varargs_sz(current_basescope->bs_argc_opt)) goto err;
   goto done;
  }
  if (asm_gpush_symbol(sym,self)) goto err;
  goto pop_unused;
 } break;

 {
  struct symbol *sym;
 case AST_UNBIND:
  if (asm_putddi(self)) goto err;
  sym = SYMBOL_UNWIND_ALIAS(self->a_unbind);
  if (asm_gdel_symbol(sym,self)) goto err;
  /* NOTE: Only make the symbol ID be available again, when
   *       the symbol itself was declared in the same scope,
   *       as the one it is being deleted from. */
  if (sym->s_type == SYMBOL_TYPE_LOCAL &&
      sym->s_scope == self->a_scope) {
   asm_dellocal(sym->s_symid);
   sym->s_flag &= ~SYMBOL_FALLOC;
  }
done_push_none:
  if (PUSH_RESULT && asm_gpush_none()) goto err;
 } break;

 {
  struct symbol *sym;
 case AST_BOUND:
  sym = self->a_sym;
  SYMBOL_INPLACE_UNWIND_ALIAS(sym);
  if (!PUSH_RESULT &&
      !symbol_bnd_haseffect(sym,self->a_scope))
      break;
  if (asm_putddi(self)) goto err;
  if (asm_gpush_bnd_symbol(sym,self)) goto err;
  goto pop_unused;
 }

 {
  unsigned int need_all;
  bool expand_encountered;
  struct ast **iter,**end;
  uint16_t active_size; int error;
 case AST_MULTIPLE:
  end = (iter = self->a_multiple.m_astv)+
                self->a_multiple.m_astc;
  if unlikely(iter == end) {
   /* Special case: empty multiple list. */
   if (!PUSH_RESULT) goto done;
   if (asm_putddi(self)) goto err;
   if (self->a_flag == AST_FMULTIPLE_KEEPLAST) {
    /* Simply push `none' */
    if (asm_gpush_none()) goto err;
   } else {
    /* Must push an empty sequence. */
    if (AST_FMULTIPLE_ISDICT(self->a_flag)) {
     error = asm_gpack_dict(0);
    } else if (self->a_flag == AST_FMULTIPLE_HASHSET) {
     error = asm_gpack_hashset(0);
    } else if (self->a_flag == AST_FMULTIPLE_LIST) {
     error = asm_gpack_list(0);
    } else {
     error = asm_gpack_tuple(0);
    }
    if (error) goto err;
   }
   goto done;
  }

  /* When `need_all' is true, we must push the results of all elements onto the stack. */
  need_all    = (self->a_flag != AST_FMULTIPLE_KEEPLAST) ? PUSH_RESULT : ASM_G_FNORMAL;
  active_size = 0,expand_encountered = false;
  for (; iter != end; ++iter) {
   struct ast *elem = *iter;
   /* Only need to push the last element when _we_ are supposed to push our result. */
   unsigned int need_this;
   need_this = need_all ? need_all : (iter == end-1 ? gflags : ASM_G_FNORMAL);
   if (elem->a_type == AST_EXPAND && need_all) {
    if (active_size) {
     if (asm_putddi(self)) goto err;
     if (expand_encountered) {
      if unlikely(asm_gextend(active_size)) goto err;
     } else {
      if unlikely(pack_sequence(self->a_flag,active_size)) goto err;
     }
    }
    error = ast_genasm(elem->a_expand,ASM_G_FPUSHRES);
    if unlikely(error) goto err;
    if (active_size || expand_encountered) {
     /* Concat the old an new parts. */
     if (asm_putddi(self)) goto err;
     if unlikely(asm_gconcat()) goto err;
    } else {
     /* The AST starts with an expand expression.
      * Because of that, we have to make sure that the entire
      * branch gets the correct type by casting now. */
     if (ast_predict_type(elem->a_expand) !=
         seqops_info[self->a_flag&3].so_typ) {
      if (asm_putddi(self)) goto err;
      if unlikely(cast_sequence(self->a_flag)) goto err;
     }
    }
    expand_encountered = true;
    active_size        = 0;
   } else {
    if (need_all) {
     if unlikely(active_size == UINT16_MAX) {
      PERRAST(self,W_ASM_SEQUENCE_TOO_LONG);
      goto err;
     }
     ++active_size;
    }
    error = ast_genasm(elem,need_this);
   }
   if unlikely(error) goto err;
  }
  if (need_all) {
   if (active_size) {
    /* Pack together a sequence, as requested by the caller. */
    if (asm_putddi(self)) goto err;
    /* In case we were packing an sequence containing expand expressions,
     * we must still concat this part with the one before that. */
    if (expand_encountered) {
     if unlikely(asm_gextend(active_size)) goto err;
    } else {
     if unlikely(pack_sequence(self->a_flag,active_size)) goto err;
    }
   }
  }
 } break;

 case AST_RETURN:
  if (!self->a_return ||
        /* NOTE: Don't optimize `return none' --> `return' in yield functions.
         *       When yielding, the `ASM_RET_NONE' instruction behaves differently
         *       from what a regular `ASM_RET' for `Dee_None' does! */
     (!(current_basescope->bs_flags & CODE_FYIELDING) &&
        self->a_return->a_type == AST_CONSTEXPR &&
        DeeNone_Check(self->a_return->a_constexpr))) {
   if (asm_putddi(self)) goto err;
   if (asm_gunwind()) goto err;
   if (asm_gret_none()) goto err;
  } else if (self->a_return->a_type == AST_SYM &&
             asm_can_prefix_symbol_for_read(self->a_return->a_sym)) {
   if (asm_putddi(self)) goto err;
   if (asm_gunwind()) goto err;
   if (asm_gprefix_symbol(self->a_return->a_sym,self->a_return)) goto err;
   if (asm_gret_p()) goto err;
  } else {
   if (ast_genasm(self->a_return,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self)) goto err;
   if (asm_gunwind()) goto err;
   if (asm_gret()) goto err;
  }
  /* NOTE: We're not responsible to perform dead-code elimination.
   *       The only thing we're responsible for, is to prevent
   *       generation of trailing code still associated with
   *       instructions that are known to never return.
   *       As is the case now, when we only fake pushing a when
   *       a `return' expression is supposed to yield something. */
done_fake_none:
  if (PUSH_RESULT) asm_incsp();
  break;

 case AST_YIELD:
  if (self->a_yield->a_type == AST_EXPAND) {
   /* Special case: Must do a YIELDALL when the
    *               expression is an expand-expression. */
   if (ast_genasm(self->a_yield->a_expand,ASM_G_FPUSHRES) ||
       asm_putddi(self) || asm_giterself() || asm_gyieldall()) goto err;
  } else if (self->a_yield->a_type == AST_SYM &&
             asm_can_prefix_symbol_for_read(self->a_yield->a_sym)) {
   if (asm_putddi(self)) goto err;
   if (asm_gprefix_symbol(self->a_yield->a_sym,self->a_yield)) goto err;
   if (asm_gyield_p()) goto err;
  } else {
   if (ast_genasm(self->a_yield,ASM_G_FPUSHRES) ||
       asm_putddi(self) ||
       asm_gyield()) goto err;
  }
  goto done_push_none;

 case AST_THROW:
  if (!self->a_throw) {
   if (asm_putddi(self)) goto err;
   if (asm_grethrow()) goto err;
  } else if (self->a_throw->a_type == AST_SYM &&
             asm_can_prefix_symbol_for_read(self->a_throw->a_sym)) {
   if (asm_putddi(self)) goto err;
   if (asm_gprefix_symbol(self->a_throw->a_sym,self->a_throw)) goto err;
   if (asm_gthrow_p()) goto err;
  } else {
   if (ast_genasm(self->a_throw,ASM_G_FPUSHRES) ||
       asm_putddi(self) ||
       asm_gthrow()) goto err;
  }
  goto done_fake_none;

 case AST_TRY:
  if unlikely(asm_gentry(self,gflags))
     goto err;
  break;

 {
  struct asm_sym *loop_break;
 case AST_LOOP:
  loop_break = asm_genloop(self->a_flag,
                           self->a_loop.l_elem,
                           self->a_loop.l_iter,
                           self->a_loop.l_loop,
                           self);
  if unlikely(!loop_break) goto err;

  /* Reset the stack depth to its original value.
   * NOTE: The current value may be distorted and invalid due to
   *       stack variables having been initialized inside the loop. */
  ASM_BREAK_SCOPE(0,err);

  /* This is where `break' jumps to. (After the re-aligned stack) */
  asm_defsym(loop_break);

  /* Loop expressions simply return `none'. */
  /* Because we've already cleaned up after stack variables initialized
   * within the loop's scope, we must not allow the code below to do so again! */
  if (PUSH_RESULT && asm_gpush_none()) goto err;
  goto done_noalign;
 } break;

 {
  struct asm_sym *loopsym;
  uint16_t old_stack;
 case AST_LOOPCTL:
  ASSERT(self->a_flag == AST_FLOOPCTL_BRK ||
         self->a_flag == AST_FLOOPCTL_CON);
#if AST_FLOOPCTL_BRK == ASM_LOOPCTL_BRK && \
    AST_FLOOPCTL_CON == ASM_LOOPCTL_CON
  loopsym = current_assembler.a_loopctl[self->a_flag];
#else
  loopsym = current_assembler.a_loopctl[self->a_flag == AST_FLOOPCTL_BRK ?
                                        ASM_LOOPCTL_BRK : ASM_LOOPCTL_CON];
#endif
  if unlikely(!loopsym) {
   if (WARNAST(self,W_ASM_BREAK_OR_CONTINUE_NOT_ALLOWED))
       goto err;
   goto done_push_none;
  }
  /* NOTE: Execute intermediate finally blocks:
   *    >> for (;;) {
   *    >>     try {
   *    >>         if (should_stop())
   *    >>             break;
   *    >>     } finally {
   *    >>         print "In finally";
   *    >>     }
   *    >>     print "next";
   *    >> }
   *    >> print "done";
   * Solution: Add an instruction to perform absolute jumps using a pop-value:
   *    >> jmp pop
   *    Using this, a finally block in a place such as that above
   *    must be entered after pushing the address of the instruction
   *    it should return to once done, meaning that `break' will
   *    push the address of `print "done";' and before entering the
   *    finally block during normal code-flow, the address of
   *    `print "next";' is pushed instead.
   *    HINT: The implementation of this is further documented in `AST_TRY' above.
   */
  old_stack = current_assembler.a_stackcur;
  /* Adjust the stack and jump to the proper symbol. */
  if (asm_putddi(self)) goto err;
  if (current_assembler.a_finsym &&
    !(current_assembler.a_finflag&ASM_FINFLAG_NOLOOP)) {
   /* Special case: Must push `ls_sym', but jump to `ls_fsym'. */
   current_assembler.a_finflag |= ASM_FINFLAG_USED;
   if (asm_gpush_abs(loopsym) ||
       asm_gpush_stk(loopsym) ||
       asm_gjmps(current_assembler.a_finsym))
       goto err;
  } else {
   if (asm_gjmps(loopsym)) goto err;
  }
  current_assembler.a_stackcur = old_stack;
  goto done_fake_none;
 } break;

 {
  bool invert_condition;
  struct ast *condition;
 case AST_CONDITIONAL:
  ASSERT_AST(self->a_conditional.c_cond);
  ASSERTF(self->a_conditional.c_tt ||
          self->a_conditional.c_ff,
          "At least one branch must exist");
  ASSERTF((self->a_conditional.c_tt != self->a_conditional.c_cond) ||
          (self->a_conditional.c_ff != self->a_conditional.c_cond),
          "At most one branch can equal the conditional branch");
  invert_condition = false;

  /* Special handling for boolean conditions.
   * NOTE: Be careful when condition re-use for this one! */
  if (self->a_conditional.c_cond->a_type == AST_BOOL &&
      self->a_conditional.c_tt != self->a_conditional.c_cond &&
      self->a_conditional.c_ff != self->a_conditional.c_cond) {
   invert_condition = (self->a_conditional.c_cond->a_flag & AST_FBOOL_NEGATE);
   condition = self->a_conditional.c_cond->a_bool;
  } else {
   condition = self->a_conditional.c_cond;
  }

  if (self->a_conditional.c_tt &&
      self->a_conditional.c_ff) {
   unsigned int cond_flags = ASM_G_FPUSHRES|ASM_G_FLAZYBOOL;
   /* If the condition expression is re-used, we can't
    * have it auto-optimize itself into a boolean value
    * if the caller expects the real expression value. */
   if (!(gflags & ASM_G_FLAZYBOOL) &&
        (self->a_conditional.c_tt == self->a_conditional.c_cond ||
         self->a_conditional.c_ff == self->a_conditional.c_cond))
         cond_flags &= ~ASM_G_FLAZYBOOL;
   if (ast_genasm(condition,cond_flags)) goto err;
   /* Branch with specific code for both paths. */
   if (self->a_conditional.c_tt == self->a_conditional.c_cond ||
       self->a_conditional.c_ff == self->a_conditional.c_cond) {
    struct asm_sym *cond_end;
    /* Special case: re-use the condition as true or false branch. */

    /* If the condition will be re-used as result, and `AST_FCOND_BOOL' is set, we
     * must first convert the conditional into a boolean if it's not already one. */
    if (asm_putddi(self)) goto err;
    if (PUSH_RESULT &&
       (self->a_flag&AST_FCOND_BOOL) &&
        ast_predict_type(condition) != &DeeBool_Type) {
     /* Force the condition to become a boolean. */
     if (asm_gbool(invert_condition)) goto err;
     invert_condition = false;
    }
    /*     push <cond>
     *    [dup]
     *     jt   1f  # Inverted by `invert_condition ^ (ast->a_conditional.c_ff == ast->a_conditional.c_cond)'
     *    [pop]
     *    [push] <false-branch> / <true-branch>
     *1:   */
    cond_end = asm_newsym();
    if unlikely(!cond_end) goto err;
    if (PUSH_RESULT && asm_gdup()) goto err;
    if (self->a_conditional.c_ff == self->a_conditional.c_cond)
        invert_condition = !invert_condition;
    if (self->a_flag&(AST_FCOND_LIKELY|AST_FCOND_UNLIKELY) &&
        current_assembler.a_curr != &current_assembler.a_sect[SECTION_COLD]) {
     /*     push <cond>
      *    [dup]
      *     jf   .cold.1f  # Inverted by `invert_condition ^ (ast->a_conditional.c_ff == ast->a_conditional.c_cond)'
      *2:
      *
      *.cold.1:
      *     pop
      *    [push] <false-branch> / <true-branch>
      *     jmp   2b */
     struct asm_sym *cold_entry = asm_newsym();
     struct asm_sec *prev_section; struct ast *genast;
     if unlikely(!cold_entry) goto err;
     if (asm_putddi(self)) goto err;
     if (asm_gjmp(invert_condition ? ASM_JT : ASM_JF,cold_entry)) goto err;
     asm_decsp();
     prev_section = current_assembler.a_curr;
     current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
     if (asm_gpop()) goto err;
     genast = self->a_conditional.c_tt != self->a_conditional.c_cond
            ? self->a_conditional.c_tt
            : self->a_conditional.c_ff;
     if (ast_genasm(genast,(self->a_flag&AST_FCOND_BOOL) ? (gflags|ASM_G_FLAZYBOOL) : gflags)) goto err;
     if (PUSH_RESULT && (self->a_flag&AST_FCOND_BOOL) &&
         ast_predict_type(genast) != &DeeBool_Type &&
         asm_gbool(false)) goto err;
     current_assembler.a_curr = prev_section;
    } else {
     struct ast *genast;
     if (asm_gjmp(invert_condition ? ASM_JF : ASM_JT,cond_end)) goto err;
     asm_decsp();
     if (PUSH_RESULT && asm_gpop()) goto err;
     genast = self->a_conditional.c_tt != self->a_conditional.c_cond
            ? self->a_conditional.c_tt
            : self->a_conditional.c_ff;
     if (ast_genasm(genast,(self->a_flag&AST_FCOND_BOOL) ? (gflags|ASM_G_FLAZYBOOL) : gflags)) goto err;
     if (PUSH_RESULT && (self->a_flag&AST_FCOND_BOOL) &&
         ast_predict_type(genast) != &DeeBool_Type &&
         asm_gbool(false)) goto err;
    }
    asm_defsym(cond_end);
   } else if (self->a_flag&(AST_FCOND_LIKELY|AST_FCOND_UNLIKELY) &&
              current_assembler.a_curr != &current_assembler.a_sect[SECTION_COLD]) {
    /* Special case where one of the branches is placed in cold text. */
    /*     push <cond>
     *     jf   .cold.1f  # Inverted by `invert_condition ^ <likely-branch == false-branch>'
     *    [push] <likely-branch>
     *2:
     *
     *.cold.1:
     *    [push] <unlikely-branch>
     *     jmp   2b */
    struct ast *likely_branch,*unlikely_branch;
    struct asm_sym *cold_entry = asm_newsym();
    struct asm_sym *text_return = asm_newsym();
    struct asm_sec *prev_section;
    bool likely_is_bool,unlikely_is_bool;
    if unlikely(!cold_entry || !text_return) goto err;
    if (self->a_flag&AST_FCOND_LIKELY) {
     likely_branch    = self->a_conditional.c_tt;
     unlikely_branch  = self->a_conditional.c_ff;
    } else {
     unlikely_branch  = self->a_conditional.c_tt;
     likely_branch    = self->a_conditional.c_ff;
     invert_condition = !invert_condition;
    }
    likely_is_bool = unlikely_is_bool = !PUSH_RESULT || !(self->a_flag&AST_FCOND_BOOL);
    if (!likely_is_bool)   likely_is_bool   = ast_predict_type(likely_branch) == &DeeBool_Type;
    if (!unlikely_is_bool) unlikely_is_bool = ast_predict_type(unlikely_branch) == &DeeBool_Type;

    if (asm_putddi(self)) goto err;
    if (asm_gjmp(invert_condition ? ASM_JT : ASM_JF,cold_entry)) goto err;
    asm_decsp();
    if (ast_genasm(likely_branch,gflags)) goto err;
    if (!likely_is_bool && unlikely_is_bool && asm_gbool(false)) goto err;
    /* Pre-adjust the stack for anything the other branch may do. */
    if (current_assembler.a_flag&ASM_FSTACKDISP &&
        asm_gsetstack_s(text_return)) goto err;

    /* Compile the unlikely code. */
    prev_section = current_assembler.a_curr;
    current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
    if (PUSH_RESULT) asm_decsp(); /* Adjust stack depth to before the likely-branch. */
    asm_defsym(cold_entry);
    if (ast_genasm(unlikely_branch,gflags)) goto err;
    if (asm_putddi(self)) goto err;
    if (likely_is_bool && !unlikely_is_bool && asm_gbool(false)) goto err;
    /* Return to regular text. */
    if (asm_gjmp(ASM_JMP,text_return)) goto err;
    current_assembler.a_curr = prev_section;

    asm_defsym(text_return);
    if (!likely_is_bool && !unlikely_is_bool && asm_gbool(false)) goto err;
   } else {
    /*     push <cond>
     *     jf   1f  # Inverted by `invert_condition'
     *    [push] <true-branch>
     *     jmp  2f
     *1:  [push] <false-branch>
     *2:   */
    struct asm_sym *ff_enter = asm_newsym();
    struct asm_sym *ff_leave = asm_newsym();
    bool tt_is_bool,ff_is_bool;
    if unlikely(!ff_enter || !ff_leave) goto err;
    tt_is_bool = ff_is_bool = !PUSH_RESULT || !(self->a_flag&AST_FCOND_BOOL);
    if (!tt_is_bool) tt_is_bool = ast_predict_type(self->a_conditional.c_tt) == &DeeBool_Type;
    if (!ff_is_bool) ff_is_bool = ast_predict_type(self->a_conditional.c_ff) == &DeeBool_Type;
    if (asm_putddi(self)) goto err;
    if (asm_gjmp(invert_condition ? ASM_JT : ASM_JF,ff_enter)) goto err;
    asm_decsp(); /* Popped by `ASM_JT' / `ASM_JF' */
    if (ast_genasm(self->a_conditional.c_tt,gflags)) goto err;
    if (asm_putddi(self)) goto err;
    if (!tt_is_bool && ff_is_bool && asm_gbool(false)) goto err;
    if (asm_gjmp(ASM_JMP,ff_leave)) goto err;
    if (PUSH_RESULT) asm_decsp(); /* Adjust to before `tt' was executed. */
    asm_defsym(ff_enter);
    if (ast_genasm(self->a_conditional.c_ff,gflags)) goto err;
    if (asm_putddi(self)) goto err;
    if (tt_is_bool && !ff_is_bool && asm_gbool(false)) goto err;
    asm_defsym(ff_leave);
    if (!tt_is_bool && !ff_is_bool && asm_gbool(false)) goto err;
   }
  } else {
   /* Only the one of the branches exists. - the other should return `none'. */
   struct ast *existing_branch;
   bool invert_boolean = invert_condition;
   ASSERT( self->a_conditional.c_tt ||  self->a_conditional.c_ff);
   ASSERT(!self->a_conditional.c_tt || !self->a_conditional.c_ff);
   existing_branch = self->a_conditional.c_tt;
   if (!existing_branch) {
    existing_branch  = self->a_conditional.c_ff;
    invert_condition = !invert_condition;
   }
   if (existing_branch == self->a_conditional.c_cond) {
    if (!PUSH_RESULT) {
     if (ast_genasm(condition,ASM_G_FNORMAL)) goto err;
     goto done;
    }
    /* Special case: The only existing branch is a duplicate of the condition. */
    /*    push <cond>
     *   [bool]
     *    dup
     *    jf   1f  # Inverted by `existing_branch == ast->a_conditional.c_ff'
     *    pop
     *    push none
     *1: */
    if (ast_genasm(condition,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(self)) goto err;
    if (PUSH_RESULT && (self->a_flag&AST_FCOND_BOOL) &&
        ast_predict_type(condition) != &DeeBool_Type) {
     /* Force the condition to become a boolean. */
     if (asm_gbool(invert_boolean)) goto err;
     invert_condition ^= invert_boolean;
    }

    if (asm_gdup()) goto err;
    /* Do a bit of hacking to prevent creation of a pointless relocation. */
    if (asm_putimm8(invert_condition ? ASM_JT : ASM_JF,sizeof(instruction_t)*2) ||
        asm_put(ASM_POP) || asm_put(ASM_PUSH_NONE)) goto err;
    asm_decsp();
   } else {
    /*   [push none|false]
     *    push <cond>
     *   [bool]
     *    jf   1f    # Inverted by `existing_branch == ast->a_conditional.c_ff'
     *   [pop]
     *   [push] <existing-branch>
     *1: */
    struct asm_sym *after_existing = asm_newsym();
    if unlikely(!after_existing) goto err;
    if (PUSH_RESULT) {
     /* Due to stack displacement, the conditional may leave more
      * than just its return value on the stack, meaning that
      * we cannot rely on our `none' remaining immediately below
      * when stack displacement is enabled.
      * Instead, we must rely on peephole optimization to then optimize
      * text like this:
      * >> push <cond>
      * >> push none
      * >> swap
      * ... into this:
      * >> push none
      * >> push <cond>
      * HINT: This uses the same facility that optimizes
      *      `a in b' --> `b.operator contains(a)'.
      */
     if (current_assembler.a_flag&ASM_FSTACKDISP &&
         ast_genasm(condition,ASM_G_FPUSHRES)) goto err;
     if (asm_putddi(self)) goto err;
     if (self->a_flag&AST_FCOND_BOOL) {
      if (asm_gpush_constexpr(Dee_False)) goto err;
     } else {
      if (asm_gpush_none()) goto err;
     }
     if (current_assembler.a_flag&ASM_FSTACKDISP) {
      if (asm_gswap()) goto err;
     } else {
      if (ast_genasm(condition,ASM_G_FPUSHRES)) goto err;
     }
     if (asm_putddi(self)) goto err;
     if (self->a_flag&AST_FCOND_BOOL &&
         ast_predict_type(condition) != &DeeBool_Type &&
         asm_gbool(false)) goto err;
     if (asm_gjmp(invert_condition ? ASM_JT : ASM_JF,after_existing)) goto err;
     asm_decsp(); /* Adjust for the value popped by `ASM_JT' / `ASM_JF' */
    } else {
     if (asm_gjcc(condition,
                  invert_condition ? ASM_JT : ASM_JF,
                  after_existing,
                  self))
         goto err;
    }
    if (PUSH_RESULT && asm_gpop()) goto err;
    if (ast_genasm(existing_branch,gflags)) goto err;
    asm_defsym(after_existing);
   }
  }
 } break;

 {
  instruction_t instr;
 case AST_BOOL:
#if AST_FBOOL_NEGATE == 1
  instr = ASM_BOOL ^ (self->a_flag & AST_FBOOL_NEGATE);
#else
  instr = ASM_BOOL ^ (instruction_t)!!(self->a_flag & AST_FBOOL_NEGATE);
#endif
  if (ast_genasm(self->a_bool,
                 ASM_G_FLAZYBOOL|
                 ASM_G_FPUSHRES))
      goto err;
  /* If the result won't be used, no need to do any inversion.
   * However, we must still invoke the bool operator of the
   * top-object, just in case doing so has any side-effects. */
  if (!PUSH_RESULT) instr = ASM_BOOL;
  if (instr == ASM_BOOL) {
   if (gflags & ASM_G_FLAZYBOOL) break;
   if (ast_predict_type(self->a_bool) == &DeeBool_Type) break;
  }
  if (asm_putddi(self)) goto err;
  if (asm_put(instr)) goto err;
  goto pop_unused;
 } break;

 case AST_EXPAND:
  if (PUSH_RESULT) {
   /* Expand to a single object by default. */
   if (asm_gunpack_expr(self->a_expand,1,self))
       goto err;
  } else {
   /* If the result isn't being used, follow the regular comma-rule
    * to generate expected assembly for code like this:
    * >> foo()...; // the results of foo() can be as comma-separated,
    * >>           // but doing so doesn't have any special meaning.
    */
   if (ast_genasm(self->a_expand,gflags))
       goto err;
  }
  break;

 case AST_FUNCTION:
  if (!PUSH_RESULT) break;
  if (asm_gpush_function(self))
      goto err;
  break;

 case AST_OPERATOR_FUNC:
  if (PUSH_RESULT) {
   /* Only need to generate the operator function binding if
    * the result of the expression is actually being used! */
   if unlikely(ast_gen_operator_func(self->a_operator_func.of_binding,
                                     self,self->a_flag))
      goto err;
  } else if (self->a_operator_func.of_binding) {
   /* Still generate code the binding-expression (in case it has side-effects) */
   if (ast_genasm(self->a_operator_func.of_binding,ASM_G_FNORMAL))
       goto err;
  }
  break;

 {
  uint16_t operator_name;
 case AST_OPERATOR:
  /* Probably one of the most important AST types: The operator AST. */
  operator_name = self->a_flag;
  /* Special case: The arguments of the operator are variadic. */
  if unlikely(self->a_operator.o_exflag&AST_OPERATOR_FVARARGS) {
   struct symbol *prefix_symbol;
   struct opinfo *info;
   info = Dee_OperatorInfo(NULL,operator_name);
   if (self->a_operator.o_op0->a_type == AST_SYM &&
      (!info || (info->oi_type&OPTYPE_INPLACE))) {
    /* Generate a prefixed instruction. */
    prefix_symbol = self->a_operator.o_op0->a_sym;
    if ((self->a_operator.o_exflag&AST_OPERATOR_FMAYBEPFX) &&
        !asm_can_prefix_symbol(prefix_symbol))
         goto varop_without_prefix;
   } else {
varop_without_prefix:
    if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
    prefix_symbol = NULL;
   }
   /* Compile operand tuple or push an empty one. */
   if (self->a_operator.o_op1) {
    if (ast_genasm(self->a_operator.o_op1,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(self)) goto err;
   } else {
    if (asm_putddi(self)) goto err;
    if (asm_gpack_tuple(0)) goto err;
   }
   if (prefix_symbol) {
    if (asm_gprefix_symbol(prefix_symbol,self->a_operator.o_op0)) goto err;
    if (asm_ginplace_operator_tuple(operator_name)) goto err;
   } else {
    if (asm_goperator_tuple(operator_name)) goto err;
   }
pop_unused:
   if (!PUSH_RESULT && asm_gpop()) goto err;
   break;
  }
  switch (operator_name) {

   /* Special instruction encoding for call operations. */
  case OPERATOR_CALL:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   if (asm_gcall_expr(self->a_operator.o_op0,
                      self->a_operator.o_op1,
                      self,gflags)) goto err;
   goto done;

  {
   DeeObject *index; int32_t temp;
   struct ast *sequence;
  case OPERATOR_GETITEM:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   sequence = self->a_operator.o_op0;
   for (;;) {
    struct ast *inner;
    if (sequence->a_type != AST_MULTIPLE) break;
    if (sequence->a_flag == AST_FMULTIPLE_KEEPLAST) break;
    if (sequence->a_multiple.m_astc != 1) break;
    inner = sequence->a_multiple.m_astv[0];
    if (inner->a_type != AST_EXPAND) break;
    sequence = inner->a_expand;
   }
   if (sequence->a_type == AST_SYM) {
    struct symbol *sym = sequence->a_sym;
    SYMBOL_INPLACE_UNWIND_ALIAS(sym);
    if (SYMBOL_TYPE(sym) == SYMBOL_TYPE_ARG &&
       !SYMBOL_MUST_REFERENCE_TYPEMAY(sym) &&
       (current_basescope->bs_flags & CODE_FVARARGS) &&
        DeeBaseScope_IsArgVarArgs(current_basescope,sym->s_symid)) {
     uint32_t va_index;
     if (!PUSH_RESULT) {
      if (ast_genasm(self->a_operator.o_op1,ASM_G_FNORMAL))
          goto err;
      goto done;
     }
     /* Lookup a varargs-argument by index. */
     if (self->a_operator.o_op1->a_type == AST_CONSTEXPR &&
         DeeInt_Check(self->a_operator.o_op1->a_constexpr) &&
         DeeInt_TryAsU32(self->a_operator.o_op1->a_constexpr,&va_index)) {
      /* Optional arguments are actually apart of varargs, so
       * we need to adjust the user-given index to have its base
       * be located just after the varargs. */
      va_index += current_basescope->bs_argc_opt;
      if (va_index <= UINT8_MAX) {
       if (asm_putddi(self)) goto err;
       if (asm_gvarargs_getitem_i((uint8_t)va_index)) goto err;
       goto done;
      }
     }
     if (ast_genasm(self->a_operator.o_op1,ASM_G_FPUSHRES)) goto err;
     if (asm_putddi(self)) goto err;
     if (DeeBaseScope_HasOptional(current_basescope)) {
      /* Must adjust for optional arguments by adding their amount to the index. */
      if (asm_gadd_imm32(current_basescope->bs_argc_opt))
          goto err;
     }
     if (asm_gvarargs_getitem()) goto err;
     goto done;
    }
   }

   if (self->a_operator.o_op1->a_type != AST_CONSTEXPR)
       break;
   index = self->a_operator.o_op1->a_constexpr;
   /* Special optimizations for integer indices. */
   if (DeeInt_Check(index) &&
       DeeInt_TryAsS32(index,&temp) &&
       temp >= INT16_MIN && temp <= INT16_MAX) {
    if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(self) || asm_ggetitem_index((int16_t)temp)) goto err;
    goto pop_unused;
   }
   /* Special optimizations for constant indices. */
   if (asm_allowconst(index)) {
    temp = asm_newconst(index);
    if unlikely(temp < 0) goto err;
    if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(self) || asm_ggetitem_const((uint16_t)temp)) goto err;
    goto pop_unused;
   }
  } break;

  {
   struct ast *begin,*end; int32_t intval;
  case OPERATOR_GETRANGE:
   if unlikely(!self->a_operator.o_op2) goto generic_operator;
   if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
   begin = self->a_operator.o_op1;
   end = self->a_operator.o_op2;
   if (begin->a_type == AST_CONSTEXPR) {
    DeeObject *const_begin = begin->a_constexpr;
    if (end->a_type == AST_CONSTEXPR) {
     DeeObject *const_end = end->a_constexpr;
     if (DeeInt_Check(const_begin) &&
         DeeInt_TryAsS32(const_begin,&intval) &&
         intval >= INT16_MIN && intval <= INT16_MAX) {
      int32_t endval;
      if (DeeNone_Check(const_end)) {
       if (asm_putddi(self)) goto err;
       if (asm_ggetrange_in((int16_t)intval)) goto err;
       goto pop_unused;
      }
      if (DeeInt_Check(const_end) &&
          DeeInt_TryAsS32(const_end,&endval) &&
          endval >= INT16_MIN && endval <= INT16_MAX) {
       if (asm_putddi(self)) goto err;
       if (asm_ggetrange_ii((int16_t)intval,(int16_t)endval)) goto err;
       goto pop_unused;
      }
     } else if (DeeNone_Check(const_begin) &&
                DeeInt_Check(const_end) &&
                DeeInt_TryAsS32(const_end,&intval) &&
                intval >= INT16_MIN && intval <= INT16_MAX) {
      if (asm_putddi(self)) goto err;
      if (asm_ggetrange_ni((int16_t)intval)) goto err;
      goto pop_unused;
     }
    }
    if (DeeNone_Check(const_begin)) {
     if (ast_genasm(end,ASM_G_FPUSHRES)) goto err;
     if (asm_putddi(self)) goto err;
     if (asm_ggetrange_np()) goto err;
     goto pop_unused;
    }
    if (DeeInt_Check(const_begin) &&
        DeeInt_TryAsS32(const_begin,&intval) &&
        intval >= INT16_MIN && intval <= INT16_MAX) {
     if (ast_genasm(end,ASM_G_FPUSHRES)) goto err;
     if (asm_putddi(self)) goto err;
     if (asm_ggetrange_ip((int16_t)intval)) goto err;
     goto pop_unused;
    }
   } else if (end->a_type == AST_CONSTEXPR) {
    DeeObject *const_end = end->a_constexpr;
    if (DeeNone_Check(const_end)) {
     if (ast_genasm(begin,ASM_G_FPUSHRES)) goto err;
     if (asm_putddi(self)) goto err;
     if (asm_ggetrange_pn()) goto err;
     goto pop_unused;
    }
    if (DeeInt_Check(const_end) &&
        DeeInt_TryAsS32(const_end,&intval) &&
        intval >= INT16_MIN && intval <= INT16_MAX) {
     if (ast_genasm(begin,ASM_G_FPUSHRES)) goto err;
     if (asm_putddi(self)) goto err;
     if (asm_ggetrange_pi((int16_t)intval)) goto err;
     goto pop_unused;
    }
   }
   if (ast_genasm(begin,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(end,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self)) goto err;
   if (asm_ggetrange()) goto err;
   goto pop_unused;
  }


  case OPERATOR_SETITEM:
   if unlikely(!self->a_operator.o_op2) goto generic_operator;
   if (ast_gen_setitem(self->a_operator.o_op0,
                       self->a_operator.o_op1,
                       self->a_operator.o_op2,
                       self,gflags)) goto err;
   goto done;

  case OPERATOR_GETATTR:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   if (ast_gen_getattr(self->a_operator.o_op0,
                       self->a_operator.o_op1,
                       self,gflags)) goto err;
   goto done;
  case OPERATOR_DELATTR:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   if (ast_gen_delattr(self->a_operator.o_op0,
                       self->a_operator.o_op1,
                       self)) goto err;
   goto done_push_none;
  case OPERATOR_SETATTR:
   if unlikely(!self->a_operator.o_op2) goto generic_operator;
   if (ast_gen_setattr(self->a_operator.o_op0,
                       self->a_operator.o_op1,
                       self->a_operator.o_op2,
                       self,gflags)) goto err;
   goto done;

   /* Arithmetic-with-constant-operand optimizations. */
  {
   int32_t intval;
  case OPERATOR_ADD:
  case OPERATOR_SUB:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   if (self->a_operator.o_op1->a_type != AST_CONSTEXPR) break;
   if (!DeeInt_Check(self->a_operator.o_op1->a_constexpr)) break;
   if (DeeInt_TryAsS32(self->a_operator.o_op1->a_constexpr,&intval) &&
      (intval >= INT8_MIN && intval <= INT8_MAX)) {
    if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(self)) goto err;
    if (operator_name == OPERATOR_ADD ? asm_gadd_simm8((int8_t)intval)
                                      : asm_gsub_simm8((int8_t)intval))
        goto err;
    goto pop_unused;
   }
   if (DeeInt_TryAsU32(self->a_operator.o_op1->a_constexpr,
                       (uint32_t *)&intval)) {
    if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(self)) goto err;
    if (operator_name == OPERATOR_ADD ? asm_gadd_imm32(*(uint32_t *)&intval)
                                      : asm_gsub_imm32(*(uint32_t *)&intval))
        goto err;
    goto pop_unused;
   }
  } break;

  {
   int32_t intval;
  case OPERATOR_INPLACE_ADD:
  case OPERATOR_INPLACE_SUB:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   if (self->a_operator.o_op1->a_type != AST_CONSTEXPR) break;
   if (self->a_operator.o_op0->a_type != AST_SYM) break;
   if (!asm_can_prefix_symbol(self->a_operator.o_op0->a_sym)) break;
   if (!DeeInt_Check(self->a_operator.o_op1->a_constexpr)) break;
   if (DeeInt_TryAsS32(self->a_operator.o_op1->a_constexpr,&intval) &&
      (intval >= INT8_MIN && intval <= INT8_MAX)) {
    if (asm_gprefix_symbol(self->a_operator.o_op0->a_sym,
                           self->a_operator.o_op0)) goto err;
    if (asm_putddi(self)) goto err;
    if (operator_name == OPERATOR_INPLACE_ADD ? asm_gadd_inplace_simm8((int8_t)intval)
                                              : asm_gsub_inplace_simm8((int8_t)intval))
        goto err;
push_a_if_used:
    if (PUSH_RESULT && ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES))
        goto err;
    goto done;
   }
   if (DeeInt_TryAsU32(self->a_operator.o_op1->a_constexpr,
                       (uint32_t *)&intval)) {
    if (asm_gprefix_symbol(self->a_operator.o_op0->a_sym,
                           self->a_operator.o_op0)) goto err;
    if (asm_putddi(self)) goto err;
    if (operator_name == OPERATOR_INPLACE_ADD ? asm_gadd_inplace_imm32(*(uint32_t *)&intval)
                                              : asm_gsub_inplace_imm32(*(uint32_t *)&intval))
        goto err;
    goto push_a_if_used;
   }
  } break;

  {
   int32_t intval;
  case OPERATOR_MUL:
  case OPERATOR_DIV:
  case OPERATOR_MOD:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   if (self->a_operator.o_op1->a_type != AST_CONSTEXPR) break;
   if (!DeeInt_Check(self->a_operator.o_op1->a_constexpr)) break;
   if (DeeInt_TryAsS32(self->a_operator.o_op1->a_constexpr,&intval) &&
      (intval >= INT8_MIN && intval <= INT8_MAX)) {
    if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(self)) goto err;
    if (operator_name == OPERATOR_MUL ? asm_gmul_simm8((int8_t)intval) :
        operator_name == OPERATOR_DIV ? asm_gdiv_simm8((int8_t)intval) :
                                        asm_gmod_simm8((int8_t)intval))
        goto err;
    goto pop_unused;
   }
  } break;

  {
   int32_t intval;
  case OPERATOR_INPLACE_MUL:
  case OPERATOR_INPLACE_DIV:
  case OPERATOR_INPLACE_MOD:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   if (self->a_operator.o_op1->a_type != AST_CONSTEXPR) break;
   if (self->a_operator.o_op0->a_type != AST_SYM) break;
   if (!asm_can_prefix_symbol(self->a_operator.o_op0->a_sym)) break;
   if (!DeeInt_Check(self->a_operator.o_op1->a_constexpr)) break;
   if (DeeInt_TryAsS32(self->a_operator.o_op1->a_constexpr,&intval) &&
      (intval >= INT8_MIN && intval <= INT8_MAX)) {
    if (asm_putddi(self)) goto err;
    if (asm_gprefix_symbol(self->a_operator.o_op0->a_sym,
                           self->a_operator.o_op0)) goto err;
    if (operator_name == OPERATOR_INPLACE_MUL ? asm_gmul_inplace_simm8((int8_t)intval) :
        operator_name == OPERATOR_INPLACE_DIV ? asm_gdiv_inplace_simm8((int8_t)intval) :
                                                asm_gmod_inplace_simm8((int8_t)intval))
        goto err;
    goto push_a_if_used;
   }
  } break;

  {
   uint32_t intval;
  case OPERATOR_AND:
  case OPERATOR_OR:
  case OPERATOR_XOR:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   if (self->a_operator.o_op1->a_type != AST_CONSTEXPR) break;
   if (!DeeInt_Check(self->a_operator.o_op1->a_constexpr)) break;
   if (DeeInt_TryAsU32(self->a_operator.o_op1->a_constexpr,&intval)) {
    if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(self)) goto err;
    if (operator_name == OPERATOR_AND ? asm_gand_imm32(intval) :
        operator_name == OPERATOR_OR  ? asm_gor_imm32(intval) :
                                        asm_gxor_imm32(intval))
        goto err;
    goto pop_unused;
   }
  } break;

  {
   uint32_t intval;
  case OPERATOR_INPLACE_AND:
  case OPERATOR_INPLACE_OR:
  case OPERATOR_INPLACE_XOR:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   if (self->a_operator.o_op1->a_type != AST_CONSTEXPR) break;
   if (self->a_operator.o_op0->a_type != AST_SYM) break;
   if (!asm_can_prefix_symbol(self->a_operator.o_op0->a_sym)) break;
   if (!DeeInt_Check(self->a_operator.o_op1->a_constexpr)) break;
   if (DeeInt_TryAsU32(self->a_operator.o_op1->a_constexpr,&intval)) {
    if (asm_putddi(self)) goto err;
    if (asm_gprefix_symbol(self->a_operator.o_op0->a_sym,
                           self->a_operator.o_op0)) goto err;
    if (operator_name == OPERATOR_INPLACE_AND ? asm_gand_inplace_imm32(intval) :
        operator_name == OPERATOR_INPLACE_OR  ? asm_gor_inplace_imm32(intval) :
                                                asm_gxor_inplace_imm32(intval))
        goto err;
    goto push_a_if_used;
   }
  } break;

  {
   struct ast *enter_expr;
  case OPERATOR_ENTER:
   enter_expr = self->a_operator.o_op0;
   if (enter_expr->a_type == AST_SYM) {
    struct symbol *sym = SYMBOL_UNWIND_ALIAS(enter_expr->a_sym);
    if (SYMBOL_TYPE(sym) == SYMBOL_TYPE_STACK &&
       !SYMBOL_MUST_REFERENCE_TYPEMAY(sym) &&
       (sym->s_flag & SYMBOL_FALLOC) &&
        SYMBOL_STACK_OFFSET(sym) == current_assembler.a_stackcur - 1) {
     /* Special optimization: Since `ASM_ENTER' doesn't modify the top stack item,
      * if the operand _is_ the top stack item, then we can simply generate the enter
      * instruction without the need of any kludge. */
     if (asm_putddi(self) || asm_genter()) goto err;
     goto done_push_none;
    }
   }
   if (ast_genasm(enter_expr,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self) || asm_genter() || asm_gpop()) goto err;
   goto done_push_none;
  }

  case OPERATOR_LEAVE:
   /* NOTE: The case of the operand being stack-top, in which
    *       case `dup; leave pop; pop;' is generated, will later
    *       be optimized away by the peephole optimizer. */
   if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self) || asm_gleave()) goto err;
   goto done_push_none;

  case OPERATOR_ITERNEXT:
   if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self) || asm_giternext()) goto err; /* This one's an extended instruction... */
   goto pop_unused;

  case OPERATOR_SIZE:
   if unlikely(!self->a_operator.o_op0) goto generic_operator;
   if ((current_basescope->bs_flags & CODE_FVARARGS) &&
        self->a_operator.o_op0->a_type == AST_SYM) {
    struct symbol *sym = self->a_operator.o_op0->a_sym;
    SYMBOL_INPLACE_UNWIND_ALIAS(sym);
    if (SYMBOL_TYPE(sym) == SYMBOL_TYPE_ARG &&
       !SYMBOL_MUST_REFERENCE_TYPEMAY(sym) &&
        DeeBaseScope_IsArgVarArgs(current_basescope,
                                  sym->s_symid)) {
     /* Special case: Get the size of varargs. */
     if (!PUSH_RESULT) goto done;
     if (asm_putddi(self)) goto err;
     if ((gflags & ASM_G_FLAZYBOOL) &&
        !(current_assembler.a_flag & ASM_FOPTIMIZE_SIZE)) {
      /* When not optimizing for size, we can compare the number
       * of varargs against 1 and push the inverted result. */
      if (asm_gcmp_gr_varargs_sz(current_basescope->bs_argc_opt)) goto err;
     } else if (!DeeBaseScope_HasOptional(current_basescope)) {
      /* NOTE: If optional arguments are being used, don't go this
       *       route because we'd need a lot of overhead to get the
       *       number of variable arguments minus the number of optional
       *       arguments. */
      if (asm_ggetsize_varargs()) goto err;
     }
     goto done;
    }
   }
   break;

  {
   struct symbol *sym;
   struct ast *sizeast;
   DeeObject *sizeval;
   uint32_t va_size_val;
  case OPERATOR_EQ:
  case OPERATOR_NE:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   /* Check for special case:
    * >> if (#varargs == 42) ...
    * >> if (42 == #varargs) ...
    * There is a dedicated instruction for comparing the size of varargs.
    */
   if unlikely(!(current_basescope->bs_flags & CODE_FVARARGS)) break;
   if (self->a_operator.o_op0->a_type == AST_OPERATOR) {
    if (self->a_operator.o_op0->a_flag != OPERATOR_SIZE) break;
    if (self->a_operator.o_op0->a_operator.o_exflag&
       (AST_OPERATOR_FPOSTOP|AST_OPERATOR_FVARARGS)) break;
    if (!self->a_operator.o_op0->a_operator.o_op0) break;
    if (self->a_operator.o_op0->a_operator.o_op0->a_type != AST_SYM) break;
    sym     = self->a_operator.o_op0->a_operator.o_op0->a_sym;
    sizeast = self->a_operator.o_op1;
   } else {
    if (self->a_operator.o_op1->a_type != AST_OPERATOR) break;
    if (self->a_operator.o_op1->a_flag != OPERATOR_SIZE) break;
    if (self->a_operator.o_op1->a_operator.o_exflag&
       (AST_OPERATOR_FPOSTOP|AST_OPERATOR_FVARARGS)) break;
    if (!self->a_operator.o_op1->a_operator.o_op0) break;
    if (self->a_operator.o_op1->a_operator.o_op0->a_type != AST_SYM) break;
    sym     = self->a_operator.o_op1->a_operator.o_op0->a_sym;
    sizeast = self->a_operator.o_op0;
   }
   SYMBOL_INPLACE_UNWIND_ALIAS(sym);
   if (SYMBOL_TYPE(sym) != SYMBOL_TYPE_ARG) break;
   if (SYMBOL_MUST_REFERENCE_TYPEMAY(sym)) break;
   if (!DeeBaseScope_IsArgVarArgs(current_basescope,sym->s_symid)) break;
   if (sizeast->a_type != AST_CONSTEXPR) break;
   sizeval = sizeast->a_constexpr;
   if (!DeeInt_Check(sizeval)) break;
   /* If the expression result isn't being used,
    * then there is no need to do anything! */
   if (!PUSH_RESULT) goto done;
   if (!DeeInt_TryAsU32(sizeval,&va_size_val)) break;
   /* Adjust for the number of optional arguments. */
   va_size_val += current_basescope->bs_argc_opt;
   if (va_size_val > UINT8_MAX) break;
   /* All right! we can encode this one as `cmp eq, #varargs, $<va_size_val>' */
   if (asm_putddi(self)) goto err;
   if (_asm_gcmp_eq_varargs_sz((uint8_t)va_size_val)) goto err;
   /* If the expression is checking for inequality, invert the result. */
   if (self->a_flag == OPERATOR_NE && asm_gbool(true)) goto err;
   goto done;
  } break;

  case OPERATOR_CONTAINS:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   if (ast_genasm_set(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(self->a_operator.o_op1,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self) || asm_gcontains()) goto err;
   goto pop_unused;

  default: break;
  }

  /* Use dedicated instructions for most operators. */
  switch (operator_name) {
  case OPERATOR_COPY:
  case OPERATOR_DEEPCOPY:
  case OPERATOR_STR:
  case OPERATOR_REPR:
  case OPERATOR_BOOL:
  case OPERATOR_INT:
  //case OPERATOR_FLOAT: /* Doesn't have its own instruction (yet?). */
  case OPERATOR_INV:
  case OPERATOR_POS:
  case OPERATOR_NEG:
  case OPERATOR_ITERSELF:
  case OPERATOR_SIZE:
   if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self)) goto err;
   ASSERT(operator_instr_table[operator_name] != 0);
   if ((asm_dicsp(),asm_put(operator_instr_table[operator_name]))) goto err;
   goto pop_unused;

  case OPERATOR_CALL:
  case OPERATOR_ADD:
  case OPERATOR_SUB:
  case OPERATOR_MUL:
  case OPERATOR_DIV:
  case OPERATOR_MOD:
  case OPERATOR_SHL:
  case OPERATOR_SHR:
  case OPERATOR_AND:
  case OPERATOR_OR:
  case OPERATOR_XOR:
  case OPERATOR_POW:
  case OPERATOR_EQ:
  case OPERATOR_NE:
  case OPERATOR_LO:
  case OPERATOR_LE:
  case OPERATOR_GR:
  case OPERATOR_GE:
  case OPERATOR_CONTAINS:
  case OPERATOR_GETITEM:
  case OPERATOR_GETATTR:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(self->a_operator.o_op1,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self)) goto err;
   ASSERT(operator_instr_table[operator_name] != 0);
   if ((asm_ddicsp(),asm_put(operator_instr_table[operator_name]))) goto err;
   goto pop_unused;

  case OPERATOR_DELITEM:
  case OPERATOR_DELATTR:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(self->a_operator.o_op1,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self)) goto err;
   ASSERT(operator_instr_table[operator_name] != 0);
   if ((asm_ddcsp(),asm_put(operator_instr_table[operator_name]))) goto err;
   goto done_push_none;

  case OPERATOR_ASSIGN:
  case OPERATOR_MOVEASSIGN:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   ASSERT(operator_instr_table[operator_name] != 0);
   if (PUSH_RESULT &&
       ast_can_exchange(self->a_operator.o_op0,
                        self->a_operator.o_op1)) {
    /* Optimization when A and B can be exchanged. */
    if (ast_genasm(self->a_operator.o_op1,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(self) || asm_gdup()) goto err;
    if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(self) || asm_gswap()) goto err;
   } else {
    if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
    if (ast_genasm(self->a_operator.o_op1,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(self)) goto err;
    if (PUSH_RESULT && (asm_gdup() || asm_grrot(3))) goto err;
   }
   if ((asm_ddcsp(),asm_put(operator_instr_table[operator_name]))) goto err;
   goto done;

  case OPERATOR_DELRANGE:
   if unlikely(!self->a_operator.o_op1) goto generic_operator;
   if unlikely(!self->a_operator.o_op2) goto generic_operator;
   if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(self->a_operator.o_op1,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(self->a_operator.o_op2,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self)) goto err;
   ASSERT(operator_instr_table[operator_name] != 0);
   if ((asm_dddcsp(),asm_put(operator_instr_table[operator_name]))) goto err;
   goto done_push_none;

  default: break;
  }

  if (OPERATOR_ISINPLACE(operator_name)) {
   bool is_unary = operator_name <= OPERATOR_DEC;
   struct ast *opa;

   /* Inplace operators with dedicated prefix-instructions. */
   ASSERT(operator_instr_table[operator_name] != 0);
   opa = self->a_operator.o_op0;
   switch (opa->a_type) {

   case AST_SYM:
    if (ast_gen_symbol_inplace(opa->a_sym,
                               is_unary ? NULL : self->a_operator.o_op1,
                               self,
                               operator_name,
                              (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) != 0,
                               gflags))
        goto err;
    goto done;

    /* >> foo.bar += 2;
     * Compile as (looks more complicated that it is):
     * >> __stack local _x = foo;
     * >> __stack local _y = _x.bar;
     * >> _y += 2;
     * >> _x.bar = _y;
     * ASM:
     * >> push    @foo
     * >> dup
     * >> getattr top, @bar
     * >> stack #SP - 1: add $2
     * >> setattr pop, @bar
     * Same goes for GETITEM & GETRANGE */
   case AST_OPERATOR:
    if (!opa->a_operator.o_op1)
        goto generic_operator;
    switch (opa->a_flag) {

    case OPERATOR_GETATTR:
     if (ast_gen_setattr_inplace(opa->a_operator.o_op0,
                                 opa->a_operator.o_op1,
                                 is_unary ? NULL : self->a_operator.o_op1,
                                 self,
                                 operator_name,
                                (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) != 0,
                                 gflags))
         goto err;
     goto done;

    case OPERATOR_GETITEM:
     if (ast_gen_setitem_inplace(opa->a_operator.o_op0,
                                 opa->a_operator.o_op1,
                                 is_unary ? NULL : self->a_operator.o_op1,
                                 self,
                                 operator_name,
                                (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) != 0,
                                 gflags))
         goto err;
     goto done;

    case OPERATOR_GETRANGE:
     if (!opa->a_operator.o_op2) goto generic_operator;
     if (ast_gen_setrange_inplace(opa->a_operator.o_op0,
                                  opa->a_operator.o_op1,
                                  opa->a_operator.o_op2,
                                  is_unary ? NULL : self->a_operator.o_op1,
                                  self,
                                  operator_name,
                                 (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) != 0,
                                  gflags))
         goto err;
     goto done;

    default: break;
    }
    goto generic_operator;


   default:
    /* TODO: Warning: Inplace operation used without prefix. */
    goto generic_operator;
   }
  }

generic_operator:

  {
   /* Generic operator assembler. */
   uint8_t argc = 0;
   struct symbol *prefix_symbol;
   struct opinfo *info = Dee_OperatorInfo(NULL,operator_name);
   if (self->a_operator.o_op0->a_type == AST_SYM &&
      (!info || (info->oi_type&OPTYPE_INPLACE))) {
    /* Generate a prefixed instruction. */
    prefix_symbol = self->a_operator.o_op0->a_sym;
    /* Make sure that the symbol can actually be used as a prefix. */
    if ((self->a_operator.o_exflag&AST_OPERATOR_FMAYBEPFX) &&
        !asm_can_prefix_symbol(prefix_symbol))
         goto operator_without_prefix;
   } else {
operator_without_prefix:
    if (ast_genasm(self->a_operator.o_op0,ASM_G_FPUSHRES)) goto err;
    prefix_symbol = NULL;
   }
   /* Compile operands. */
   if (self->a_operator.o_op1 &&
      (++argc,ast_genasm(self->a_operator.o_op1,ASM_G_FPUSHRES))) goto err;
   if (self->a_operator.o_op2 &&
      (++argc,ast_genasm(self->a_operator.o_op2,ASM_G_FPUSHRES))) goto err;
   if (self->a_operator.o_op3 &&
      (++argc,ast_genasm(self->a_operator.o_op3,ASM_G_FPUSHRES))) goto err;
   if (asm_putddi(self)) goto err;
   if (prefix_symbol) {
    if (asm_gprefix_symbol(prefix_symbol,self->a_operator.o_op0)) goto err;
    if (asm_ginplace_operator(operator_name,argc)) goto err;
   } else {
    if (asm_goperator(operator_name,argc)) goto err;
   }
   goto pop_unused;
  }
 } break;

 {
  uint16_t action_type;
 case AST_ACTION:
  /* Special action operation. */
  action_type = self->a_flag & AST_FACTION_KINDMASK;
  ASSERT(AST_FACTION_ARGC_GT(self->a_flag) <= 3);
  ASSERT((AST_FACTION_ARGC_GT(self->a_flag) >= 3) == (self->a_action.a_act2 != NULL));
  ASSERT((AST_FACTION_ARGC_GT(self->a_flag) >= 2) == (self->a_action.a_act1 != NULL));
  ASSERT((AST_FACTION_ARGC_GT(self->a_flag) >= 1) == (self->a_action.a_act0 != NULL));
  switch (action_type) {
#define ACTION(x) case x & AST_FACTION_KINDMASK:
  {
   int32_t deemon_modid;
  ACTION(AST_FACTION_CELL0)
   if (!PUSH_RESULT) goto done;
   deemon_modid = asm_newmodule(DeeModule_GetDeemon());
   if unlikely(deemon_modid < 0) goto err;
   if (asm_gcall_extern((uint16_t)deemon_modid,id_cell,0)) goto err;
  } break;
  {
   int32_t deemon_modid;
  ACTION(AST_FACTION_CELL1)
   if (ast_genasm(self->a_action.a_act0,PUSH_RESULT)) goto err;
   if (!PUSH_RESULT) goto done;
   deemon_modid = asm_newmodule(DeeModule_GetDeemon());
   if unlikely(deemon_modid < 0) goto err;
   if (asm_gcall_extern((uint16_t)deemon_modid,id_cell,1)) goto err;
  } break;
  ACTION(AST_FACTION_TYPEOF)
   if (ast_genasm(self->a_action.a_act0,PUSH_RESULT)) goto err;
   if (PUSH_RESULT && (asm_putddi(self) || asm_gtypeof())) goto err;
   break;
  ACTION(AST_FACTION_CLASSOF)
   if (ast_genasm(self->a_action.a_act0,PUSH_RESULT)) goto err;
   if (PUSH_RESULT && (asm_putddi(self) || asm_gclassof())) goto err;
   break;
  ACTION(AST_FACTION_SUPEROF)
   if (ast_genasm(self->a_action.a_act0,PUSH_RESULT)) goto err;
   if (PUSH_RESULT && (asm_putddi(self) || asm_gsuperof())) goto err;
   break;
  ACTION(AST_FACTION_AS)
   if (PUSH_RESULT &&
       self->a_action.a_act0->a_type == AST_SYM) {
    struct symbol *this_sym = self->a_action.a_act0->a_sym;
    SYMBOL_INPLACE_UNWIND_ALIAS(this_sym);
    if (this_sym->s_type == SYMBOL_TYPE_THIS &&
       !SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
     struct symbol *typesym;
     if (self->a_action.a_act1->a_type == AST_SYM) {
      /* Special optimizations for `this as ...' */
      int32_t symid;
      typesym = self->a_action.a_act1->a_sym;
      SYMBOL_INPLACE_UNWIND_ALIAS(typesym);
      if (ASM_SYMBOL_MAY_REFERENCE(typesym)) {
do_this_as_typesym_ref:
       symid = asm_rsymid(typesym);
       if unlikely(symid < 0) goto err;
       if (asm_gsuper_this_r(symid)) goto err;
       goto done;
      }
     }
     if (self->a_action.a_act1->a_type == AST_CONSTEXPR &&
         current_basescope != (DeeBaseScopeObject *)current_rootscope &&
       !(current_assembler.a_flag & ASM_FREDUCEREFS)) {
      typesym = asm_bind_deemon_export(self->a_action.a_act1->a_constexpr);
      if unlikely(!typesym) goto err;
      if (typesym != ASM_BIND_DEEMON_EXPORT_NOTFOUND)
          goto do_this_as_typesym_ref;
     }
    }
   }
   if (ast_genasm(self->a_action.a_act0,PUSH_RESULT)) goto err;
   if (ast_genasm(self->a_action.a_act1,PUSH_RESULT)) goto err;
   if (PUSH_RESULT && (asm_putddi(self) || asm_gsuper())) goto err;
   break;
  ACTION(AST_FACTION_PRINT)
   if unlikely(ast_genprint(PRINT_MODE_NORMAL+PRINT_MODE_ALL,
                            self->a_action.a_act0,self))
      goto err;
   goto done_push_none;
  ACTION(AST_FACTION_PRINTLN)
   if unlikely(ast_genprint(PRINT_MODE_NL+PRINT_MODE_ALL,
                            self->a_action.a_act0,self))
      goto err;
   goto done_push_none;
  ACTION(AST_FACTION_FPRINT)
   if (ast_genasm(self->a_action.a_act0,ASM_G_FPUSHRES)) goto err;
   if unlikely(ast_genprint(PRINT_MODE_NORMAL+PRINT_MODE_FILE+PRINT_MODE_ALL,
                            self->a_action.a_act1,self))
      goto err;
   goto pop_unused;
  ACTION(AST_FACTION_FPRINTLN)
   if (ast_genasm(self->a_action.a_act0,ASM_G_FPUSHRES)) goto err;
   if unlikely(ast_genprint(PRINT_MODE_NL+PRINT_MODE_FILE+PRINT_MODE_ALL,
                            self->a_action.a_act1,self))
      goto err;
   goto pop_unused;
  ACTION(AST_FACTION_RANGE)
   if (self->a_action.a_act0->a_type == AST_CONSTEXPR) {
    DeeObject *index_object = self->a_action.a_act0->a_constexpr;
    uint32_t intval;
    if (DeeNone_Check(index_object) ||
       (DeeInt_Check(index_object) &&
       (DeeInt_TryAsU32(index_object,&intval) && intval == 0))) {
     /* Special optimization: The begin index is not set, or equal to int(0). */
     if (AST_ISNONE(self->a_action.a_act2) &&
        (self->a_action.a_act1->a_type == AST_CONSTEXPR &&
        (index_object = self->a_action.a_act1->a_constexpr,
         DeeInt_Check(index_object)) &&
         DeeInt_TryAsU32(index_object,&intval))) {
      /* Special optimization: No step is given and the
       * end index is known to be a constant integer. */
      if (PUSH_RESULT && (asm_putddi(self) || asm_grange_0_i(intval))) goto err;
      break;
     }
     if (ast_genasm(self->a_action.a_act1,PUSH_RESULT)) goto err;
     if (AST_ISNONE(self->a_action.a_act2)) {
      if (PUSH_RESULT && (asm_putddi(self) || asm_grange_0())) goto err;
     } else {
      if (ast_genasm(self->a_action.a_act2,PUSH_RESULT)) goto err;
      if (PUSH_RESULT && (asm_putddi(self) || asm_grange_step_0())) goto err;
     }
     break;
    }
   }
   if (ast_genasm(self->a_action.a_act0,PUSH_RESULT)) goto err;
   if (ast_genasm(self->a_action.a_act1,PUSH_RESULT)) goto err;
   if (AST_ISNONE(self->a_action.a_act2)) {
    if (PUSH_RESULT && (asm_putddi(self) || asm_grange())) goto err;
   } else {
    if (ast_genasm(self->a_action.a_act2,PUSH_RESULT)) goto err;
    if (PUSH_RESULT && (asm_putddi(self) || asm_grange_step())) goto err;
   }
   break;
  ACTION(AST_FACTION_IS)
   if (ast_genasm(self->a_action.a_act0,PUSH_RESULT)) goto err;
   if (PUSH_RESULT &&
      (self->a_action.a_act1->a_type == AST_CONSTEXPR &&
      (self->a_action.a_act1->a_constexpr == Dee_None ||
       self->a_action.a_act1->a_constexpr == (DeeObject *)&DeeNone_Type))) {
    /* Optimization for code like this: `foo is none'.
     * A special opcode exists for this case because a lot of code uses
     * `none' as placeholder in default arguments, relying on the `is'
     * operator to check if the argument has a meaningful value.
     * In these types of situations, `operator ==' can't be used
     * because using it may invoke arbitrary code, while `is' only
     * performs a shallow check that never fails, or invokes other code. */
    if (asm_putddi(self)) goto err;
    if (asm_gisnone()) goto err;
    break;
   }
   if (ast_genasm(self->a_action.a_act1,PUSH_RESULT)) goto err;
   if (PUSH_RESULT && (asm_putddi(self) || asm_ginstanceof())) goto err;
   break;
  ACTION(AST_FACTION_IN)
   if (ast_can_exchange(self->a_action.a_act0,
                        self->a_action.a_act1)) {
    if (ast_genasm_set(self->a_action.a_act1,ASM_G_FPUSHRES)) goto err;
    if (ast_genasm(self->a_action.a_act0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(self) || asm_gcontains()) goto err;
   } else {
    if (ast_genasm(self->a_action.a_act0,ASM_G_FPUSHRES)) goto err;
    if (ast_genasm_set(self->a_action.a_act1,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(self) || asm_gswap() || asm_gcontains()) goto err;
   }
   goto pop_unused;
  ACTION(AST_FACTION_MIN)
   if (ast_genasm(self->a_action.a_act0,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self) || asm_greduce_min()) goto err;
   goto pop_unused;
  ACTION(AST_FACTION_MAX)
   if (ast_genasm(self->a_action.a_act0,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self) || asm_greduce_max()) goto err;
   goto pop_unused;
  ACTION(AST_FACTION_SUM)
   if (ast_genasm(self->a_action.a_act0,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self) || asm_greduce_sum()) goto err;
   goto pop_unused;
  ACTION(AST_FACTION_ANY)
   if (ast_genasm(self->a_action.a_act0,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self) || asm_greduce_any()) goto err;
   goto pop_unused;
  ACTION(AST_FACTION_ALL)
   if (ast_genasm(self->a_action.a_act0,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self) || asm_greduce_all()) goto err;
   goto pop_unused;
  ACTION(AST_FACTION_STORE)
   if unlikely(asm_gstore(self->a_action.a_act0,
                          self->a_action.a_act1,
                          self,PUSH_RESULT))
      goto err;
   break;

  ACTION(AST_FACTION_BOUNDATTR)
   if unlikely(ast_gen_boundattr(self->a_action.a_act0,
                                 self->a_action.a_act1,
                                 self,PUSH_RESULT))
      goto err;
   break;

  ACTION(AST_FACTION_BOUNDITEM)
   if (ast_genasm(self->a_action.a_act0,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(self->a_action.a_act1,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(self) || asm_gbounditem()) goto err;
   goto pop_unused;

  ACTION(AST_FACTION_CALL_KW)
   /* Call with keyword list. */
   if (asm_gcall_kw_expr(self->a_action.a_act0,
                         self->a_action.a_act1,
                         self->a_action.a_act2,
                         self,gflags))
       goto err;
   break;

  ACTION(AST_FACTION_ASSERT)
   if (asm_genassert(self->a_action.a_act0,
                     NULL,self,gflags))
       goto err;
   break;
  ACTION(AST_FACTION_ASSERT_M)
   if (asm_genassert(self->a_action.a_act0,
                     self->a_action.a_act1,
                     self,gflags))
       goto err;
   break;

  ACTION(AST_FACTION_SAMEOBJ)
   if (ast_genasm(self->a_action.a_act0,PUSH_RESULT)) goto err;
   if (ast_genasm(self->a_action.a_act1,PUSH_RESULT)) goto err;
   if (PUSH_RESULT && (asm_putddi(self) || asm_gsameobj())) goto err;
   goto pop_unused;
  ACTION(AST_FACTION_DIFFOBJ)
   if (ast_genasm(self->a_action.a_act0,PUSH_RESULT)) goto err;
   if (ast_genasm(self->a_action.a_act1,PUSH_RESULT)) goto err;
   if (PUSH_RESULT && (asm_putddi(self) || asm_gdiffobj())) goto err;
   goto pop_unused;

#undef ACTION
  default:
   ASSERTF(0,"Invalid action type: %x",(unsigned int)action_type);
  }
 } break;

 case AST_CLASS:
  if unlikely(asm_genclass(self,gflags))
     goto err;
  break;

 {
  struct text_label *label;
  struct asm_sym *sym;
 case AST_LABEL:
  label = self->a_label.l_label;
  ASSERT(label);
  if (!label->tl_goto) {
   if (WARNAST(self,W_ASM_LABEL_NEVER_USED,
               self->a_flag&AST_FLABEL_CASE ?
              (label->tl_expr ? "case" : "default") :
               label->tl_name->k_name))
       goto err;
  }
  sym = label->tl_asym;
  if (!sym) {
   sym = asm_newsym();
   if unlikely(!sym) goto err;
   label->tl_asym = sym;
  }
  if unlikely(ASM_SYM_DEFINED(sym)) {
   /* Warn if the label had already been defined. */
   if (WARNAST(self,W_ASM_LABEL_ALREADY_DEFINED,
               self->a_flag&AST_FLABEL_CASE ?
              (label->tl_expr ? "case" : "default") :
               label->tl_name->k_name))
       goto err;
   goto done_push_none;
  }
  asm_defsym(sym);
  goto done_push_none;
 }

 {
  struct text_label *label;
  struct asm_sym *sym;
  uint16_t old_stack;
 case AST_GOTO:
  /* Just to a specified symbol. */
  label = self->a_goto.g_label;
  ASSERT(label);
  sym = label->tl_asym;
  if (!sym) {
   sym = asm_newsym();
   if unlikely(!sym) goto err;
   label->tl_asym = sym;
  }
  old_stack = current_assembler.a_stackcur;
  /* Adjust the stack and jump to the proper symbol. */
  if (asm_putddi(self)) goto err;
  if (current_assembler.a_finsym) {
   /* NOTE: Must only go through the finally-block
    *       if the jump target is located outside. */
   current_assembler.a_finflag |= ASM_FINFLAG_USED;
   if (asm_gpush_abs(sym) ||
       asm_gpush_stk(sym) ||
       asm_gjmps(current_assembler.a_finsym))
       goto err;
  } else {
   if (asm_gjmps(sym)) goto err;
  }
  current_assembler.a_stackcur = old_stack;
  goto done_fake_none;
 }

 case AST_SWITCH:
  if unlikely(ast_genasm_switch(self)) goto err;
  /* Switch statements simply return `none' by default.
   * When a switch appears in an expressions, it is actually parsed as follows:
   * >> local x = switch (y) { case 7: "foo"; case 8: 42; default: 11; }
   * Parsed like this:
   * >> local x = ({
   * >>     __stack local _temp;
   * >>     switch (y) {
   * >>     case 7:
   * >>         _temp = "foo";
   * >>         break;
   * >>     case 8:
   * >>         _temp = 42;
   * >>         break;
   * >>     default:
   * >>         _temp = 11;
   * >>         break;
   * >>     }
   * >>     _temp;
   * >> });
   * For that reason, there is no need for a second AST type for expression-switches.
   */
  goto done_push_none;

 case AST_ASSEMBLY:
  if unlikely(ast_genasm_userasm(self))
     goto err;
  goto done_push_none;

 default:
  ASSERTF(0,"Invalid AST type: %x",(unsigned int)self->a_type);
  break;
 }
done:
 ASM_POP_SCOPE(PUSH_RESULT ? 1 : 0,err);
done_noalign:
 ASM_BREAK_LOC();
 return 0;
err:
 ASM_POP_LOC();
 return -1;
#undef PUSH_RESULT
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENASM_C */
