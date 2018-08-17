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

PRIVATE uint8_t const operator_opcount_table[OPERATOR_USERCOUNT] = {
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
   /* [OPERATOR_CALL]        = */ENTRY(OPCOUNT_INSTRIN,2),
   /* [OPERATOR_ITERNEXT]    = */0, /* ASM_ITERNEXT - Extended opcodes can't be used here. */
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
PRIVATE DeeAstObject *DCALL
ast_unwrap_effective(DeeAstObject *__restrict self) {
 while (self->ast_type == AST_MULTIPLE &&
        self->ast_flag == AST_FMULTIPLE_KEEPLAST &&
        self->ast_multiple.ast_exprc != 0 &&
        self->ast_scope == current_assembler.a_scope) {
  DeeAstObject **iter,**end;
  end = (iter = self->ast_multiple.ast_exprv)+
                self->ast_multiple.ast_exprc-1;
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
             DeeAstObject *__restrict print_expression,
             DeeAstObject *__restrict ddi_ast) {
 print_expression = ast_unwrap_effective(print_expression);
 if (mode&PRINT_MODE_ALL) {
  if (print_expression->ast_type == AST_MULTIPLE &&
      print_expression->ast_flag != AST_FMULTIPLE_KEEPLAST) {
   DeeAstObject **iter,**end;
   /* Special optimization for printing an expanded, multi-branch.
    * This is actually the most likely case, because something like
    * `print "Hello World";' is actually encoded as `print pack("Hello World")...;' */
   end = (iter = print_expression->ast_multiple.ast_exprv)+
                 print_expression->ast_multiple.ast_exprc;
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
  } else if (print_expression->ast_type == AST_CONSTEXPR) {
   DREF DeeObject *items,**iter,**end;
   items = DeeTuple_FromSequence(print_expression->ast_constexpr);
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
 } else if (print_expression->ast_type == AST_CONSTEXPR &&
            asm_allowconst(print_expression->ast_constexpr)) {
  /* Special instructions exist for direct printing of constants. */
  int32_t const_cid = asm_newconst(print_expression->ast_constexpr);
  if unlikely(const_cid < 0) goto err;
  if (asm_putddi(ddi_ast)) goto err;
  return asm_put816(ASM_PRINT_C+mode,(uint16_t)const_cid);
 }
fallback:
 /* Fallback: Compile the print expression, then print it as an expanded sequence. */
 if (print_expression->ast_type == AST_EXPAND && !(mode&PRINT_MODE_ALL)) {
  if (ast_genasm(print_expression->ast_expandexpr,ASM_G_FPUSHRES)) goto err;
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
  if (DeeString_SIZE(item->ss_name) != DeeString_SIZE(name)) continue; /* Non-matching length */
  if (memcmp(DeeString_STR(item->ss_name), /* Differing strings. */
             DeeString_STR(name),
             DeeString_SIZE(name)*sizeof(char)) != 0)
      continue;
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
    /* [AST_FMULTIPLE_TUPLE & 3] = */{ &DeeTuple_Type,   { ASM_PACK_TUPLE,   ASM16_PACK_TUPLE },   ASM_CAST_TUPLE },
    /* [AST_FMULTIPLE_LIST  & 3] = */{ &DeeList_Type,    { ASM_PACK_LIST,    ASM16_PACK_LIST },    ASM_CAST_LIST },
    /* [AST_FMULTIPLE_SET   & 3] = */{ &DeeHashSet_Type, { ASM_PACK_HASHSET, ASM16_PACK_HASHSET }, ASM_CAST_HASHSET },
    /* [AST_FMULTIPLE_DICT  & 3] = */{ &DeeDict_Type,    { ASM_PACK_DICT,    ASM16_PACK_DICT },    ASM_CAST_DICT }
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
INTERN int (DCALL ast_genasm)(DeeAstObject *__restrict ast,
                              unsigned int gflags) {
#define PUSH_RESULT   (gflags & ASM_G_FPUSHRES)
 struct ast_loc *old_loc;
 ASSERT_OBJECT_TYPE(ast,&DeeAst_Type);
 /* Set the given AST as location information for error messages. */
 old_loc = current_assembler.a_error;
 current_assembler.a_error = &ast->ast_ddi;
 ASM_PUSH_SCOPE(ast->ast_scope,err);
 switch (ast->ast_type) {

 case AST_CONSTEXPR:
  if (!PUSH_RESULT) break;
  if (asm_putddi(ast)) goto err;
  if (asm_gpush_constexpr(ast->ast_constexpr)) goto err;
  break;

 {
  struct symbol *sym;
 case AST_SYM:
  if (!PUSH_RESULT) break;
  if (asm_putddi(ast)) goto err;
  sym = ast->ast_sym;
  SYMBOL_INPLACE_UNWIND_ALIAS(sym);
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
  if (asm_gpush_symbol(sym,ast)) goto err;
 } break;

 case AST_UNBIND:
  if (asm_putddi(ast)) goto err;
  SYMBOL_INPLACE_UNWIND_ALIAS(ast->ast_unbind);
  if (asm_gdel_symbol(ast->ast_unbind,ast)) goto err;
  if (ast->ast_unbind->s_type == SYMBOL_TYPE_LOCAL &&
      ast->ast_unbind->s_scope->s_base == current_basescope) {
   asm_dellocal(ast->ast_unbind->s_symid);
   ast->ast_unbind->s_flag &= ~SYMBOL_FALLOC;
  }
done_push_none:
  if (PUSH_RESULT && asm_gpush_none()) goto err;
  break;

 case AST_BNDSYM:
  if (!PUSH_RESULT) break;
  if (asm_putddi(ast)) goto err;
  if (asm_gpush_bnd_symbol(ast->ast_sym,ast)) goto err;
  break;

 {
  unsigned int need_all;
  bool expand_encountered;
  DeeAstObject **iter,**end;
  uint16_t active_size; int error;
 case AST_MULTIPLE:
  end = (iter = ast->ast_multiple.ast_exprv)+
                ast->ast_multiple.ast_exprc;
  if unlikely(iter == end) {
   /* Special case: empty multiple list. */
   if (!PUSH_RESULT) goto done;
   if (asm_putddi(ast)) goto err;
   if (ast->ast_flag == AST_FMULTIPLE_KEEPLAST) {
    /* Simply push `none' */
    if (asm_gpush_none()) goto err;
   } else {
    /* Must push an empty sequence. */
    if (AST_FMULTIPLE_ISDICT(ast->ast_flag)) {
     error = asm_gpack_dict(0);
    } else if (ast->ast_flag == AST_FMULTIPLE_SET) {
     error = asm_gpack_hashset(0);
    } else if (ast->ast_flag == AST_FMULTIPLE_LIST) {
     error = asm_gpack_list(0);
    } else {
     error = asm_gpack_tuple(0);
    }
    if (error) goto err;
   }
   goto done;
  }

  /* When `need_all' is true, we must push the results of all elements onto the stack. */
  need_all    = (ast->ast_flag != AST_FMULTIPLE_KEEPLAST) ? PUSH_RESULT : ASM_G_FNORMAL;
  active_size = 0,expand_encountered = false;
  for (; iter != end; ++iter) {
   DeeAstObject *elem = *iter;
   /* Only need to push the last element when _we_ are supposed to push our result. */
   unsigned int need_this;
   need_this = need_all ? need_all : (iter == end-1 ? gflags : ASM_G_FNORMAL);
   if (elem->ast_type == AST_EXPAND && need_all) {
    if (active_size) {
     if (asm_putddi(ast)) goto err;
     if (expand_encountered) {
      if unlikely(asm_gextend(active_size)) goto err;
     } else {
      if unlikely(pack_sequence(ast->ast_flag,active_size)) goto err;
     }
    }
    error = ast_genasm(elem->ast_expandexpr,ASM_G_FPUSHRES);
    if unlikely(error) goto err;
    if (active_size || expand_encountered) {
     /* Concat the old an new parts. */
     if (asm_putddi(ast)) goto err;
     if unlikely(asm_gconcat()) goto err;
    } else {
     /* The AST starts with an expand expression.
      * Because of that, we have to make sure that the entire
      * branch gets the correct type by casting now. */
     if (ast_predict_type(elem->ast_expandexpr) !=
         seqops_info[ast->ast_flag&3].so_typ) {
      if (asm_putddi(ast)) goto err;
      if unlikely(cast_sequence(ast->ast_flag)) goto err;
     }
    }
    expand_encountered = true;
    active_size        = 0;
   } else {
    if (need_all) {
     if unlikely(active_size == UINT16_MAX) {
      PERRAST(ast,W_ASM_SEQUENCE_TOO_LONG);
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
    if (asm_putddi(ast)) goto err;
    /* In case we were packing an sequence containing expand expressions,
     * we must still concat this part with the one before that. */
    if (expand_encountered) {
     if unlikely(asm_gextend(active_size)) goto err;
    } else {
     if unlikely(pack_sequence(ast->ast_flag,active_size)) goto err;
    }
   }
  }
 } break;

 case AST_RETURN:
  if (!ast->ast_returnexpr ||
        /* NOTE: Don't optimize `return none' --> `return' in yield functions.
         *       When yielding, the `ASM_RET_NONE' instruction behaves differently
         *       from what a regular `ASM_RET' for `Dee_None' does! */
     (!(current_basescope->bs_flags&CODE_FYIELDING) &&
        ast->ast_returnexpr->ast_type == AST_CONSTEXPR &&
        DeeNone_Check(ast->ast_returnexpr->ast_constexpr))) {
   if (asm_putddi(ast)) goto err;
   if (asm_gunwind()) goto err;
   if (asm_gret_none()) goto err;
  } else if (ast->ast_returnexpr->ast_type == AST_SYM &&
             asm_can_prefix_symbol_for_read(ast->ast_returnexpr->ast_sym)) {
   if (asm_putddi(ast)) goto err;
   if (asm_gunwind()) goto err;
   if (asm_gprefix_symbol(ast->ast_returnexpr->ast_sym,ast->ast_returnexpr)) goto err;
   if (asm_gret_p()) goto err;
  } else {
   if (ast_genasm(ast->ast_returnexpr,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast)) goto err;
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
  if (ast->ast_yieldexpr->ast_type == AST_EXPAND) {
   /* Special case: Must do a YIELDALL when the
    *               expression is an expand-expression. */
   if (ast_genasm(ast->ast_yieldexpr->ast_expandexpr,ASM_G_FPUSHRES) ||
       asm_putddi(ast) || asm_giterself() || asm_gyieldall()) goto err;
  } else if (ast->ast_yieldexpr->ast_type == AST_SYM &&
             asm_can_prefix_symbol_for_read(ast->ast_yieldexpr->ast_sym)) {
   if (asm_putddi(ast)) goto err;
   if (asm_gprefix_symbol(ast->ast_yieldexpr->ast_sym,ast->ast_yieldexpr)) goto err;
   if (asm_gyield_p()) goto err;
  } else {
   if (ast_genasm(ast->ast_yieldexpr,ASM_G_FPUSHRES) ||
       asm_putddi(ast) ||
       asm_gyield()) goto err;
  }
  goto done_push_none;

 case AST_THROW:
  if (!ast->ast_throwexpr) {
   if (asm_putddi(ast)) goto err;
   if (asm_grethrow()) goto err;
  } else if (ast->ast_throwexpr->ast_type == AST_SYM &&
             asm_can_prefix_symbol_for_read(ast->ast_throwexpr->ast_sym)) {
   if (asm_putddi(ast)) goto err;
   if (asm_gprefix_symbol(ast->ast_throwexpr->ast_sym,ast->ast_throwexpr)) goto err;
   if (asm_gthrow_p()) goto err;
  } else {
   if (ast_genasm(ast->ast_throwexpr,ASM_G_FPUSHRES) ||
       asm_putddi(ast) ||
       asm_gthrow()) goto err;
  }
  goto done_fake_none;

 { /* Guarding addresses of all sections. */
  code_addr_t guard_begin[SECTION_TEXTCOUNT];
  code_addr_t guard_end[SECTION_TEXTCOUNT];
  /* [0..1][(b != NULL) == (e != NULL)] begin/end of the guarded section
   *  NOTE: When NULL, the protected code did not write to the section. */
  struct { struct asm_sym *b,*e; } guard[SECTION_TEXTCOUNT];
  struct catch_expr *iter,*end; uint16_t i,old_finflag;
  bool is_guarding; struct asm_sym *next_handler;
  struct asm_sym *my_first_finally,*existing_finally;
  uint16_t guard_finflags,except_index; struct asm_sym *after_catch;
  struct handler_frame hand_frame; size_t catch_mask_c;
  DREF DeeTypeObject *catch_mask,**catch_mask_v; /* [owned_if(!= catch_mask)] */
 case AST_TRY:
  after_catch = NULL;

  /* Keep track of where different sections are currently at,
   * so we can safely determine what changed afterwards and
   * therewith generate appropriate exception handlers for
   * everything that is located within.
   * >> We must do it this way because the guarded code
   *    may be writing text to more than one section, in
   *    which case we must still guard everything it wrote! */
  for (i = 0; i < SECTION_TEXTCOUNT; ++i)
       guard_begin[i] = asm_secip(i);

  /* Save the priority index of the handlers that we're going to generate.
   * This is the vector index where we're going to insert our handlers below. */
  except_index = current_assembler.a_exceptc;

  my_first_finally = NULL;
  existing_finally = current_assembler.a_finsym;
  /* If this we're currently inside of a loop, we much check
   * for finally-handlers, because if there are some, then
   * any break/continue statements inside must first jump
   * to the nearest finally-block, which must then be executed
   * before continuing on its path to execute more handler,
   * until eventually jumping to where the break was meant to go. */
  end = (iter = ast->ast_try.ast_catchv)+ast->ast_try.ast_catchc;
  old_finflag = current_assembler.a_finflag;
  for (; iter != end; ++iter) {
   if (!(iter->ce_flags&EXCEPTION_HANDLER_FFINALLY)) continue;
   my_first_finally = asm_newsym();
   if unlikely(!my_first_finally) goto err;
   current_assembler.a_finsym  = my_first_finally;
   current_assembler.a_finflag = ASM_FINFLAG_NORMAL;
   goto gen_guard;
  }

gen_guard:
  /* HINT: We propagate the expression result of the guarded expression outwards.
   *       I'm not quite sure, and I'm too lazy to look it up right now, but if
   *       I remember correctly, this is something I wanted to do in the old
   *       deemon, but never could because of how convoluted its assembly
   *       generator was...
   *       I mean: I didn't even understood how symbols, or relocations are
   *               meant to work, or knew what they were at all.
   *               Looking back, it's amazing that I managed to create something
   *               that worked, knowing so little about how it's done correctly. */
  if (ast_genasm(ast->ast_try.ast_guard,gflags))
      goto err;

  /* Check if a loop control statement was used within the guarded block.
   * Because if one was, then we must do some special handling to compile
   * all of our finally-handlers. */
  guard_finflags = current_assembler.a_finflag;
  current_assembler.a_finsym = existing_finally;
  current_assembler.a_finflag = old_finflag;

  /* This is where the guarded section ends. */
  for (i = 0; i < SECTION_TEXTCOUNT; ++i)
       guard_end[i] = asm_secip(i);

  /* Figure out what has changed and generate appropriate symbols. */
  memset(guard,0,sizeof(guard)),is_guarding = false;
  for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
   if (guard_begin[i] != guard_end[i]) {
    guard[i].b = asm_newsym();
    if unlikely(!guard[i].b) goto err;
    guard[i].e = asm_newsym();
    if unlikely(!guard[i].e) goto err;
    guard[i].b->as_sect = i;
    guard[i].e->as_sect = i;
#if 1
    /* All right. This is kind-of lazy.
     * The problem (that could only be fixed properly by
     * introducing some mechanism to define symbols at the
     * current location when the current section is changed)
     * originates from code like this:
     * >> try {
     * >>     __stack local inner_stack = 42;
     * >>     try {
     * >>         print "Try",inner_stack;
     * >>     } catch (...) {
     * >>         print "Inner",inner_stack;
     * >>     }
     * >> } catch (...) {
     * >>     print "Outer";
     * >> }
     * When the Outer exception handler gets assembled, it will
     * create a total of 4 symbols for the begin/end of its
     * protection are in all text sections covered by the
     * try-block. (.text and .cold)
     * It then simply defines all the begin-symbols at the
     * associated text positions of the affected section,
     * later to check if the section's text point changed,
     * in which case an exception handler entry is generated
     * to cover the affected area.
     * However, these symbols (like all others) also include
     * tracking information for the respective stack-depth.
     * And while that information isn't actually required by
     * the exception handler (it only needs to know the stack-depth
     * at the exception handler entry point, but not at the coverage
     * entry/exit points in all sections), those symbols still
     * include that information.
     * However, looking back at the code, the inner catch-handler
     * will be placed in the .cold section, yet the outer handler
     * would have otherwise created a symbol at its entry point,
     * not realizing that the stack address at that location is
     * invalid.
     * So to go the easy route, simply define the symbols as using
     * an invalid stack address which makes them illegal for use
     * as jump targets, but still possible for use as exception
     * handler begin/end addresses. */
    guard[i].b->as_stck = ASM_SYM_STCK_INVALID;
    guard[i].e->as_stck = ASM_SYM_STCK_INVALID;
#else
    guard[i].b->as_stck = current_assembler.a_stackcur;
    guard[i].e->as_stck = current_assembler.a_stackcur;
    if (PUSH_RESULT) --guard[i].b->as_stck;
#endif
    guard[i].b->as_hand = current_assembler.a_handlerc;
    guard[i].e->as_hand = current_assembler.a_handlerc;
    guard[i].b->as_addr = guard_begin[i];
    guard[i].e->as_addr = guard_end[i];
    is_guarding = true;
   }
  }

  /* Now that we know exactly what is being protected,
   * let's generate the actual exception handlers. */
  end = (iter = ast->ast_try.ast_catchv)+ast->ast_try.ast_catchc;
  next_handler = NULL;
  hand_frame.hf_prev = current_assembler.a_handler;
  current_assembler.a_handler = &hand_frame;
  ++current_assembler.a_handlerc;
  for (; iter != end; ++iter) {
#define IS_LAST_HANDLER() (iter == end-1)
   struct asm_sym *handler_entry;
   struct asm_exc *descriptor;
   uint16_t handler_stack_exit = current_assembler.a_stackcur;
   uint16_t handler_stack = handler_stack_exit;
   catch_mask_v = &catch_mask;
   catch_mask_c = 1;
   catch_mask = NULL;
   /* Keep track of what kind of handler this is. */
   hand_frame.hf_flags = iter->ce_flags;
   if (hand_frame.hf_flags&EXCEPTION_HANDLER_FFINALLY) {
    struct asm_sym *finally_exit = NULL;
    /* Finally handlers are written in-line directly after the protected code.
     * Additionally, their stack alignment matches that of the guarded code,
     * so that we can easily hide the result value of the protected part in the stack. */
    if (next_handler) handler_entry = next_handler,next_handler = NULL;
    else if unlikely((handler_entry = asm_newsym()) == NULL) goto err_hand_frame;
    if (guard_finflags&ASM_FINFLAG_USED) {
     if unlikely((finally_exit = asm_newsym()) == NULL) goto err_hand_frame;
     /* Push the stack & address where finally is meant to return to normally. */
     if unlikely(asm_gpush_abs(finally_exit)) goto err_hand_frame;
     if unlikely(asm_gpush_stk(finally_exit)) goto err_hand_frame;
     handler_stack += 2;
    }
    if (my_first_finally) {
     asm_defsym(my_first_finally);
     my_first_finally = NULL;
    }
    /* Note how we didn't switch sections. - Finally handlers are written in-line. */
    asm_defsym(handler_entry);
    /* If this is the first finally, define the entry label as such. */
    if (iter->ce_mask && /* We never defined what should happen to a mask here.
                          * So to not cause any problems and stick to the documentation
                          * stating that this code is executed before the handler itself,
                          * simple evaluate it as an expression and move on. */
        ast_genasm(iter->ce_mask,ASM_G_FNORMAL)) goto err_hand_frame;
    if (ast_genasm(iter->ce_code,ASM_G_FNORMAL)) goto err_hand_frame;
    if (is_guarding && asm_gendfinally()) goto err_hand_frame;
    if (guard_finflags&ASM_FINFLAG_USED) {
     /* Search for the next finally-handler. */
     struct catch_expr *iter2 = iter+1;
     struct asm_sym *next_finally = existing_finally;
     for (; iter2 != end; ++iter2) {
      if (!(iter2->ce_flags&EXCEPTION_HANDLER_FFINALLY)) continue;
      next_finally = asm_newsym();
      if unlikely(!next_finally) goto err_hand_frame;
      /* Define as the entry point of the next finally-block. */
      my_first_finally = next_finally;
      break;
     }
     if (next_finally) {
      /* This is where it gets a bit convoluted,
       * because we need to check something at runtime:
       * We already know that the finally-handler was not executed
       * following a return instruction (because in that case
       * execution would not have passed `asm_gendfinally'), but
       * what we don't know is how the finally block was entered.
       * Was it:
       *   - Entered through normal code flow
       *   - Or entered because a loop control expression, or goto-statement
       *     was executed that needed to jump across the finally handler.
       * Considering the fact that we know of more finally handlers
       * that need to be executed before the loop control expression
       * can be served, we must either jump to the next finally-handler
       * in case we got here because of a loop-control expression, or
       * we must continue execution after the finally block itself, in
       * case it was entered through normal means:
       * >> for (;;) {
       * >>     try {
       * >>         try {
       * >>             if (should_break())
       * >>                 break; // push addrof(Loop_end); jmp addrof(Inner_finally)
       * >>             
       * >>             print "Entering inner finally normally";
       * >>         } finally { // push addrof(Leaving_inner_finally);
       * >>             print "Inner_finally";
       * >>             // This is where we are right now.
       * >>             //   - If we got here from `break', we must hold
       * >>             //     off from jumping to `Loop_end' because of
       * >>             //     the outter finally block, but we can not
       * >>             //     blindly jump there all the time, because
       * >>             //     then we'd always skip `Leaving_inner_finally'
       * >>             //   - To fix this, we must compare the address that is
       * >>             //     currently located ontop of the stack and contains the
       * >>             //     finally-return-address (either `Leaving_inner_finally' or `Loop_end'),
       * >>             //     and only jump to `Outter_finally' when it isn't equal
       * >>             //     to `Leaving_inner_finally'.
       * >>             // ASM:
       * >>             //  >> # TOP == finally_return_address
       * >>             //  >>     dup
       * >>             //  >>     push $addrof(Leaving_inner_finally)
       * >>             //  >>     cmp  eq
       * >>             //  >>     jf   addrof(Outter_finally)
       * >>             //  >>     jmp  pop
       * >>             // HINT: `Leaving_inner_finally' is named `finally_exit'
       * >>             // HINT: `Outter_finally' is named `next_finally'
       * >>         }
       * >>         print "Leaving_inner_finally";
       * >>         print "Entering outter finally normally";
       * >>     } finally { // push addrof(Loop_last)
       * >>         print "Outter_finally";
       * >>         // jmp pop (Can be served blindly in this case)
       * >>     }
       * >>     print "Loop_last";
       * >> }
       * >> print "Loop_end";
       */
      /* Generate the assembly documented above. */
      if (asm_gdup()) goto err_hand_frame;
      if (asm_gpush_abs(finally_exit)) goto err_hand_frame;
      if (asm_gcmp_eq()) goto err_hand_frame;
      /* TODO: Must clean up catch-handlers between here and the next finally! */
      if (asm_gjmp(ASM_JF,next_finally)) goto err_hand_frame;
      asm_decsp(); /* Popped by `ASM_JF' */
     }
     ASSERT((current_assembler.a_flag&ASM_FSTACKDISP) ||
            (current_assembler.a_stackcur == handler_stack));
     /* Ensure that the stack is properly adjusted to where
      * the handler return address and stack depth are stored.
      * This must be done because in STACKDISP mode, the stack
      * may need to be re-aligned after stack variables were
      * initialized. */
     if (asm_gsetstack(handler_stack)) goto err_hand_frame;
     /* When there are no further finally-blocks left,
      * simply pop the loop-exit address and jump there. */
     if (asm_gjmp_pop_pop()) goto err_hand_frame;
    }
    /* Define the address that is pushed to cause the
     * `jmp pop' above to become a no-op when the handler
     * is entered through regular code-flow. */
    if (finally_exit) asm_defsym(finally_exit);
    /* Set the FFINALLY flag in the resulting code object, thus
     * ensuring that finally-blocks are executed after return. */
    if (is_guarding)
        current_basescope->bs_flags |= CODE_FFINALLY;
   } else {
    struct asm_sec *prev_section;
    struct asm_sym *jump_across = NULL;
    code_addr_t cleanup_begin[SECTION_TEXTCOUNT];
    code_addr_t cleanup_end[SECTION_TEXTCOUNT];
    bool needs_cleanup,is_empty_handler;
    struct { struct asm_sym *b,*e; } cleanup[SECTION_TEXTCOUNT];
    /* Don't generate catch-blocks if they're not guarding anything.
     * This can happen when the guarded code is a no-op that might
     * have been optimized away during the AST optimization pass. */
    if (!is_guarding) continue;
    /* Switch the current section to the cold one.
     * >> Exception handlers are assumed to be executed only
     *    rarely, so instead of generating jumps around them
     *    in regular text, we later put them at the end, so-as
     *    to prevent them from influencing code performance
     *    all-together! */
    prev_section             = current_assembler.a_curr;
    current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
    if (prev_section == current_assembler.a_curr) {
     jump_across = asm_newsym();
     if unlikely(!jump_across) goto err_hand_frame;
     if (asm_gjmp(ASM_JMP,jump_across)) goto err_hand_frame;
    }
    /* Allow catch-handlers to define their own return value. */
    if (PUSH_RESULT) { --handler_stack; asm_decsp(); }

    /* This is where the handler's entry point is located at! */
    if (next_handler) handler_entry = next_handler,next_handler = NULL;
    else if unlikely((handler_entry = asm_newsym()) == NULL) goto err_hand_frame;
    asm_defsym(handler_entry);

    /* Must include the catch-mask expression in the cleanup guard! */
    needs_cleanup = false;
    for (i = 0; i < SECTION_TEXTCOUNT; ++i)
         cleanup_begin[i] = asm_secip(i);
    if (iter->ce_mask) {
     DeeAstObject *mask_ast = iter->ce_mask;
     struct asm_sym *enter_handler;
     /* Deal with an explicit exception handling mask.
      * NOTE: The runtime has special hooks in place to quickly deal
      *       with a mask that was already known at compile-time.
      *       Though when the catch-expression wasn't known then,
      *       we must generate some work-around code:
      *       >> try {
      *       >>     ....
      *       >> } catch (get_mask()) {
      *       >>     print "Handler";
      *       >> }
      *       Compile as:
      *       >> try {
      *       >>     ....
      *       >> } catch (<except>...) {
      *       >>     if (<except> !is get_mask()) {
      *       >>         if (<is_last_handler>) {
      *       >>             throw;
      *       >>         } else {
      *       >>             <goto_next_handler>
      *       >>         }
      *       >>     }
      *       >>     print "Handler";
      *       >> }
      */
handle_mask_ast:
     if (mask_ast->ast_type == AST_CONSTEXPR &&
         DeeType_Check(mask_ast->ast_constexpr) &&
         asm_allowconst(mask_ast->ast_constexpr)) {
      catch_mask = (DREF DeeTypeObject *)mask_ast->ast_constexpr;
      Dee_Incref(catch_mask);
     } else if (mask_ast->ast_type == AST_CONSTEXPR &&
                DeeTuple_Check(mask_ast->ast_constexpr)) {
      /* More than one exception mask. */
      DeeObject **maskv;
      maskv = DeeTuple_ELEM(mask_ast->ast_constexpr);
      catch_mask_c = DeeTuple_SIZE(mask_ast->ast_constexpr);
      enter_handler = NULL;
      if unlikely(!catch_mask_c) /* No mask? */
       catch_mask_v = NULL;
      else {
       size_t catch_mask_i;
       catch_mask_v = (DREF DeeTypeObject **)Dee_Calloc(catch_mask_c*
                                                        sizeof(DREF DeeTypeObject *));
       if unlikely(!catch_mask_v) { catch_mask_c = 0; goto err_hand_frame; }
       for (catch_mask_i = 0; catch_mask_i < catch_mask_c; ++catch_mask_i) {
        DeeObject *mask_object;
        mask_object = maskv[catch_mask_i];
        ASSERT_OBJECT(mask_object);
        if (DeeType_Check(mask_object) &&
            asm_allowconst(mask_object)) {
         catch_mask_v[catch_mask_i] = (DREF DeeTypeObject *)mask_object;
         Dee_Incref(catch_mask_v[catch_mask_i]);
        } else {
         /* Runtime-evaluated sub-mask. */
         if (!enter_handler && (enter_handler = asm_newsym()) == NULL)
              goto err_hand_frame;
         /* Generate code of this mask. */
         if (asm_gpush_except()) goto err_hand_frame;               /* except */
         if (asm_gpush_constexpr(mask_object)) goto err_hand_frame; /* except, mask */
         if (asm_ginstanceof()) goto err_hand_frame;                /* except instanceof mask */
         /* Jump to the handler entry point if the mask matches. */
         if (asm_gjmp(ASM_JT,enter_handler)) goto err_hand_frame;
         asm_decsp(); /* Popped by `ASM_JF' */
        }
       }
      }
      goto do_multimask_rethrow;
     } else if (mask_ast->ast_type == AST_MULTIPLE &&
                mask_ast->ast_flag != AST_FMULTIPLE_KEEPLAST) {
      /* More than one exception mask. */
      DeeAstObject **maskv;
      enter_handler = NULL;
      maskv = mask_ast->ast_multiple.ast_exprv;
      if (mask_ast->ast_multiple.ast_exprc == 1) {
       /* Special handling when only a single type is being masked. */
       mask_ast = maskv[0];
       goto handle_mask_ast;
      }
      catch_mask_c = mask_ast->ast_multiple.ast_exprc;
      if unlikely(!catch_mask_c) /* No mask? */
       catch_mask_v = NULL;
      else {
       size_t catch_mask_i;
       catch_mask_v = (DREF DeeTypeObject **)Dee_Calloc(catch_mask_c*
                                                        sizeof(DREF DeeTypeObject *));
       if unlikely(!catch_mask_v) { catch_mask_c = 0; goto err_hand_frame; }
       for (catch_mask_i = 0; catch_mask_i < catch_mask_c; ++catch_mask_i) {
        mask_ast = maskv[catch_mask_i];
        ASSERT_OBJECT_TYPE_EXACT(mask_ast,&DeeAst_Type);
        if (mask_ast->ast_type == AST_CONSTEXPR &&
            DeeType_Check(mask_ast->ast_constexpr) &&
            asm_allowconst(mask_ast->ast_constexpr)) {
         catch_mask_v[catch_mask_i] = (DREF DeeTypeObject *)mask_ast->ast_constexpr;
         Dee_Incref(catch_mask_v[catch_mask_i]);
        } else {
         /* Runtime-evaluated sub-mask. */
         if (!enter_handler && (enter_handler = asm_newsym()) == NULL)
              goto err_hand_frame;
         /* Generate code of this mask. */
         if (asm_gpush_except()) goto err_hand_frame;                  /* except */
         if (ast_genasm(mask_ast,ASM_G_FPUSHRES)) goto err_hand_frame; /* except, mask */
         if (asm_ginstanceof()) goto err_hand_frame;                   /* except instanceof mask */
         /* Jump to the handler entry point if the mask matches. */
         if (asm_gjmp(ASM_JT,enter_handler)) goto err_hand_frame;
         asm_decsp(); /* Popped by `ASM_JF' */
        }
       }
      }
do_multimask_rethrow:
      if (enter_handler) {
       /* If the enter-handler symbol was allocated, that means
        * that at least one of the catch-masks has to be evaluated
        * at runtime, meaning we must generate a bit more code now. */
       if (!IS_LAST_HANDLER()) {
        next_handler = asm_newsym();
        if unlikely(!next_handler) goto err_hand_frame;
        if (asm_gjmp(ASM_JMP,next_handler)) goto err_hand_frame;
       } else {
        if (asm_grethrow()) goto err_hand_frame;
       }
       asm_defsym(enter_handler);
      }
     } else {
      /* Fallback: generate a check at runtime. */
      if (asm_gpush_except()) goto err_hand_frame;                  /* except */
      if (ast_genasm(mask_ast,ASM_G_FPUSHRES)) goto err_hand_frame; /* except, mask */
      if (asm_ginstanceof()) goto err_hand_frame;                   /* except instanceof mask */

      if (!IS_LAST_HANDLER()) {
       next_handler = asm_newsym();
       if unlikely(!next_handler) goto err_hand_frame;
       /* Jump to the next handler when it's not a match. */
       if (asm_gjmp(ASM_JF,next_handler)) goto err_hand_frame;
       asm_decsp(); /* Popped by `ASM_JF' */
      } else {
       struct asm_sym *is_a_match;
       is_a_match = asm_newsym();
       if unlikely(!is_a_match) goto err_hand_frame;
       /* Execute the handler when it's a match. */
       if (asm_gjmp(ASM_JT,is_a_match)) goto err_hand_frame;
       asm_decsp(); /* Popped by `ASM_JT' */

       /* Since there is no next handler, it's up to us to re-throw the exception. */
       if (asm_grethrow()) goto err_hand_frame;
       /* This is where our match-check jumps to when there was no match. */
       asm_defsym(is_a_match);
      }
     }
    }
    /* Allow the handler to save a result onto the stack. */
    if (ast_genasm(iter->ce_code,gflags)) goto err_hand_frame;
    for (i = 0; i < SECTION_TEXTCOUNT; ++i)
         cleanup_end[i] = asm_secip(i);
    /* If the FSECOND flag is set, the caller is OK with secondary
     * exceptions not being discarded in the event of a new primary
     * exception. */
    if (!(iter->ce_mode&CATCH_EXPR_FSECOND)) {
     /* Must generate cleanup code to use always make use of the primary exception. */
     /* Check which sections need to be protected by cleanup code. */
     memset(cleanup,0,sizeof(cleanup));
     for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
      if (cleanup_begin[i] == cleanup_end[i]) continue;
      needs_cleanup = true;
      cleanup[i].b = asm_newsym();
      if unlikely(!cleanup[i].b) goto err_hand_frame;
      cleanup[i].e = asm_newsym();
      if unlikely(!cleanup[i].e) goto err_hand_frame;
      cleanup[i].b->as_stck = ASM_SYM_STCK_INVALID;
      cleanup[i].e->as_stck = ASM_SYM_STCK_INVALID;
      cleanup[i].b->as_sect = i;
      cleanup[i].e->as_sect = i;
      cleanup[i].b->as_hand = current_assembler.a_handlerc;
      cleanup[i].e->as_hand = current_assembler.a_handlerc;
      cleanup[i].b->as_addr = cleanup_begin[i];
      cleanup[i].e->as_addr = cleanup_end[i];
     }
    }
    /* Check if the handler is a so-called ~empty~ handler, that is a
     * handler that would not contain any code other than `throw except'
     * to re-throw the last error. (or rather continue handling it)
     * Such a handler can be optimized by making use of the
     * `EXCEPTION_HANDLER_FHANDLED' flag to let the runtime handle
     * its associated exception before using the handler's guard
     * end address as its entry point, continuing execution after
     * the error has been discarded.
     * XXX: Why don't we just set the flag for any handler that doesn't
     *      make use of the active exception? Then we wouldn't have to
     *      generate another cleanup handler if the handler itself causes
     *      another error to be thrown?
     *      For this purpose, we should add a new __asm__-clobber
     *      field `"except"', that should be specified to disable
     *      this optimization when user-assembly appears in exception
     *      handlers.
     *      Other than that: Check if the handler contains any use
     *      of `SYMBOL_TYPE_EXCEPT' symbols that don't originate from
     *      other catch-handlers that may be reachable from inside.
     */
    is_empty_handler = !needs_cleanup && !PUSH_RESULT;
    if (is_empty_handler) {
     for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
      if (cleanup_begin[i] == cleanup_end[i])
          continue;
      is_empty_handler = false;
      break;
     }
    }
    if (is_empty_handler) {
     /* Cheat a bit by re-defining the handle-entry
      * symbol to point to its coverage exit address.
      * This is to optimize empty catch-handlers:
      * >> try {
      * >>     return get_value_1();
      * >> } catch (...) {
      * >> }
      * >> return get_value_2();
      * ASM:
      * >>.begin except_1
      * >>    call global @get_value_1, #0
      * >>    ret  pop
      * >>.end except_1
      * >>.entry except_1, @except, @handled
      * >>    call global @get_value_2, #0
      * >>    ret  pop
      */
     handler_entry->as_sect = (uint16_t)(prev_section - current_assembler.a_sect);
     handler_entry->as_addr = (code_addr_t)(prev_section->sec_iter - prev_section->sec_begin);
     /* Use the FHANDLED flag to discard the error,
      * rather than generating text to do the same. */
     hand_frame.hf_flags |= EXCEPTION_HANDLER_FHANDLED;
    } else {
     /* Before regular exit from a catch-block, handle the current exception. */
     if (asm_gendcatch()) goto err_hand_frame;
     if (!IS_LAST_HANDLER() || current_assembler.a_curr != prev_section) {
      /* Just generate a jmp after all catch handlers. */
      if (!after_catch && (after_catch = asm_newsym()) == NULL) goto err_hand_frame;
      if (asm_gjmp(ASM_JMP,after_catch)) goto err_hand_frame;
     }
     /* If there are more handlers and this one just pushed its result,
      * then we must re-adjust the stack to discard the value. */
     if (PUSH_RESULT && !IS_LAST_HANDLER()) asm_decsp();
     if (needs_cleanup) {
      /* So do _do_ need to generate cleanup code! */
      struct asm_sym *cleanup_entry;
      if (IS_LAST_HANDLER() && current_assembler.a_curr == prev_section) {
       /* But generate code to jump across the cleanup text. */
       if (!jump_across && (jump_across = asm_newsym()) == NULL) goto err_hand_frame;
       if (asm_gjmp(ASM_JMP,jump_across)) goto err_hand_frame;
      }
      /* Determine the distance to the highest-order exception handler. */
      cleanup_entry = asm_newsym();
      if unlikely(!cleanup_entry) goto err_hand_frame;

      /* Create an exception descriptor for the cleanup. */
      for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
       if (!cleanup[i].b) continue;
       /* NOTE: Increase the priority for following handlers, so that
        *       cleanup handlers are considered after the real handlers. */
       descriptor = asm_newexc_at(except_index++);
       if unlikely(!descriptor) goto err_hand_frame;
       /* Track usage of exception handler symbols. */
       ++cleanup[i].b->as_used;
       ++cleanup[i].e->as_used;
       ++cleanup_entry->as_used;
       descriptor->ex_mask  = NULL;
       descriptor->ex_start = cleanup[i].b;
       descriptor->ex_end   = cleanup[i].e;
       descriptor->ex_addr  = cleanup_entry;
#if 0
       descriptor->ex_stack = handler_stack;
#endif
       descriptor->ex_flags = EXCEPTION_HANDLER_FNORMAL;
      }

      /* This is the cleanup code. */
      asm_defsym(cleanup_entry);
      if (asm_gendcatch_n(1)) goto err_hand_frame; /* Discard the first secondary exception */
      if (asm_grethrow()) goto err_hand_frame;
     }
    }
    if (jump_across) asm_defsym(jump_across);
    current_assembler.a_curr = prev_section;
   }

   /* Get rid of duplicate exception masks that can occur when multi-catch expressions are used. */
   if (catch_mask_c > 1) {
    size_t mask_i,mask_j;
    for (mask_i = 0; mask_i < catch_mask_c-1; ++mask_i) {
     DeeTypeObject *my_mask = catch_mask_v[mask_i];
     for (mask_j = mask_i+1; mask_j < catch_mask_c;) {
      if (catch_mask_v[mask_j] == my_mask) {
       /* Get rid of this mask (which is already in use) */
       --catch_mask_c;
       MEMMOVE_PTR(catch_mask_v+mask_i,
                   catch_mask_v+mask_i+1,
                  (catch_mask_c-mask_i));
       Dee_XDecref(my_mask);
       continue;
      }
      ++mask_j;
     }
    }
   }

   /* Create an exception descriptor for every affected section. */
   for (i = 0; i < SECTION_TEXTCOUNT; ++i) {
    size_t mask_i;
    if (!guard[i].b) continue;
    /* Create new descriptors at the effective priority index.
     * Since we're not incrementing `except_index' following this,
     * the effective priority remains the same, meaning that later
     * exception handlers have a lower priority than previous ones,
     * just as is intended by the deemon specs. */
    for (mask_i = 0; mask_i < catch_mask_c; ++mask_i) {
     descriptor = asm_newexc_at(except_index);
     if unlikely(!descriptor) goto err_hand_frame;
     descriptor->ex_mask  = catch_mask_v[mask_i];
     Dee_XIncref(catch_mask_v[mask_i]);
     /* Track usage of exception handler symbols. */
     ++guard[i].b->as_used;
     ++guard[i].e->as_used;
     ++handler_entry->as_used;
     descriptor->ex_start = guard[i].b;
     descriptor->ex_end   = guard[i].e;
     descriptor->ex_addr  = handler_entry;
#if 0
     /* No special magic here. - Simply use the stack depth as it is after the guarded expression.
      * With that in mind, the result value of a catch()-expression is the same object that the
      * runtime uses when aligning the stack. */
     descriptor->ex_stack = handler_stack;
#endif
     descriptor->ex_flags = hand_frame.hf_flags;
    }
   }

   /* Reset the stack depth to its original value.
    * NOTE: The current value may be distorted and invalid due to
    *       stack variables having been initialized inside the loop. */
   ASSERT(current_assembler.a_flag&ASM_FSTACKDISP ||
          current_assembler.a_stackcur == handler_stack_exit);
   current_assembler.a_stackcur = handler_stack_exit;

   /* Drop the reference to a compile-time catch mask. */
   if (catch_mask_v != &catch_mask) {
    while (catch_mask_c--)
        Dee_XDecref(catch_mask_v[catch_mask_c]);
    Dee_Free(catch_mask_v);
   }
   Dee_XDecref(catch_mask);
#undef IS_LAST_HANDLER
  }
  current_assembler.a_handler = hand_frame.hf_prev;
  --current_assembler.a_handlerc;
  /* This is where catch-handles jump to. */
  if (after_catch)
      asm_defsym(after_catch);
  break;
err_hand_frame:
  if (catch_mask_v != &catch_mask) {
   while (catch_mask_c--)
       Dee_Decref(catch_mask_v[catch_mask_c]);
   Dee_Free(catch_mask_v);
  }
  Dee_XDecref(catch_mask);
  current_assembler.a_handler = hand_frame.hf_prev;
  --current_assembler.a_handlerc;
  goto err;
 } break;


 {
  struct asm_sym *old_break;
  struct asm_sym *old_continue;
  struct asm_sym *loop_break,*loop_continue;
  uint16_t old_finflag;
 case AST_LOOP:
  /* Save old loop control symbols. */
  old_break     = current_assembler.a_loopctl[ASM_LOOPCTL_BRK];
  old_continue  = current_assembler.a_loopctl[ASM_LOOPCTL_CON];
  /* Create new symbols for loop control. */
  loop_break    = asm_newsym();
  if unlikely(!loop_break) goto err;
  loop_continue = asm_newsym();
  if unlikely(!loop_continue) goto err;
  /* Setup the new loop control symbols. */
  current_assembler.a_loopctl[ASM_LOOPCTL_BRK] = loop_break;
  current_assembler.a_loopctl[ASM_LOOPCTL_CON] = loop_continue;
  /* Set the out-of-loop flag for execution of existing finally handler. */
  old_finflag = current_assembler.a_finflag;
  current_assembler.a_finflag |= ASM_FINFLAG_NOLOOP;

  /* Now let's get to work! */
  if (ast->ast_flag&AST_FLOOP_FOREACH) {
   struct asm_sec *prev_section;
   /* >> foreach()-style loop. */
   /* Start out by evaluating the loop iterator. */
   if (ast->ast_loop.ast_iter->ast_type == AST_OPERATOR &&
       ast->ast_loop.ast_iter->ast_flag == OPERATOR_ITERSELF &&
     !(ast->ast_loop.ast_iter->ast_operator.ast_exflag&(AST_OPERATOR_FPOSTOP|AST_OPERATOR_FVARARGS)) &&
       ast->ast_loop.ast_iter->ast_operator.ast_opa) {
    /* Generate a sequence as an ASP, thus optimizing away unnecessary casts. */
    if (ast_genasm_asp(ast->ast_loop.ast_iter->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ast->ast_loop.ast_iter)) goto err;
    if (asm_giterself()) goto err;
   } else {
    if (ast_genasm(ast->ast_loop.ast_iter,ASM_G_FPUSHRES)) goto err;
   }
   /* This is where the loop starts! (and where `continue' jump to) */
   asm_defsym(loop_continue);
   /* The foreach instruction will jump to the break-address
    * when the iterator has been exhausted. */
   if (asm_putddi(ast)) goto err;
   if (asm_gjmp(ASM_FOREACH,loop_break)) goto err;
   asm_diicsp(); /* -2, +1: loop-branch of a foreach instruction. */
   /* HINT: Right now, the stack looks like this:
    *       ..., iterator, elem */
   prev_section = current_assembler.a_curr;
   /* Put the loop block into the cold section if it's unlikely to be executed. */
   if (ast->ast_flag&AST_FLOOP_UNLIKELY &&
       prev_section != &current_assembler.a_sect[SECTION_COLD]) {
    struct asm_sym *loop_begin;
    loop_begin = asm_newsym();
    if unlikely(!loop_begin) goto err;
    if (asm_putddi(ast)) goto err;
    if (asm_gjmp(ASM_JMP,loop_begin)) goto err; /* Jump into cold text. */
    current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
   }

   /* Store the foreach iterator into the loop variable(s) */
   if (ast->ast_loop.ast_elem) {
    if (asm_gpop_expr(ast->ast_loop.ast_elem))
        goto err;
   } else {
    if (asm_gpop())
        goto err;
   }

   /* Generate the loop block itself. */
   if (ast->ast_loop.ast_loop &&
       ast_genasm(ast->ast_loop.ast_loop,ASM_G_FNORMAL))
       goto err;

   if (asm_putddi(ast)) goto err;
   if (asm_gjmp(ASM_JMP,loop_continue)) goto err; /* Jump back to yield the next item. */
   current_assembler.a_curr = prev_section;

   /* -1: Adjust for the `ASM_FOREACH' instruction popping the iterator once it's empty. */
   asm_decsp();
  } else if (ast->ast_flag&AST_FLOOP_POSTCOND) {
   struct asm_sym *loop_block = asm_newsym();
   if unlikely(!loop_block) goto err;
   asm_defsym(loop_block);
   /* NOTE: There's no point in trying to put some if this stuff into
    *       the cold section when the `AST_FLOOP_UNLIKELY' flag is set.
    *       Since the loop-block is always executed, we'd always have
    *       to jump into cold text, which would kind-of defeat the purpose
    *       considering that it's meant to contain code that's unlikely
    *       to be called. */
   /* Generate the loop itself. */
   if (ast->ast_loop.ast_loop &&
       ast_genasm(ast->ast_loop.ast_loop,ASM_G_FNORMAL)) goto err;

   /* Evaluate the condition after the loop (and after the continue-symbol). */
   asm_defsym(loop_continue);
   if (ast->ast_loop.ast_next &&
       ast_genasm(ast->ast_loop.ast_next,ASM_G_FNORMAL)) goto err;
   if (ast->ast_loop.ast_cond) {
    if (asm_gjcc(ast->ast_loop.ast_cond,ASM_JT,loop_block,ast))
        goto err; /* if (cond) goto loop_block; */
   } else {
    if (asm_putddi(ast)) goto err;
    if (asm_gjmp(ASM_JMP,loop_block)) goto err; /* if (cond) goto loop_block; */
   }
  } else {
   struct asm_sym *loop_block;
   if (!ast->ast_loop.ast_next) {
    loop_block = loop_continue;
   } else {
    loop_block = asm_newsym();
    if unlikely(!loop_block) goto err;
   }
   asm_defsym(loop_block);
   /* Evaluate the condition before the loop. */
   if (ast->ast_flag&AST_FLOOP_UNLIKELY &&
       current_assembler.a_curr != &current_assembler.a_sect[SECTION_COLD]) {
    struct asm_sec *prev_section;
    struct asm_sym *loop_enter;
    loop_enter = asm_newsym();
    if unlikely(!loop_enter) goto err;
    if (asm_gjcc(ast->ast_loop.ast_cond,ASM_JT,loop_enter,ast))
        goto err; /* if (cond) goto enter_loop; */

    prev_section = current_assembler.a_curr;
    current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
    asm_defsym(loop_enter);

    /* Generate the loop itself. */
    if (ast->ast_loop.ast_loop &&
        ast_genasm(ast->ast_loop.ast_loop,ASM_G_FNORMAL)) goto err;

    if (ast->ast_loop.ast_next) {
     asm_defsym(loop_continue);
     if (ast_genasm(ast->ast_loop.ast_next,ASM_G_FNORMAL)) goto err;
    }
    /* Jump back to re-evaluate the condition. */
    if (asm_putddi(ast)) goto err;
    if (asm_gjmp(ASM_JMP,loop_block)) goto err;
    current_assembler.a_curr = prev_section;
   } else {
    if (ast->ast_loop.ast_cond &&
        asm_gjcc(ast->ast_loop.ast_cond,ASM_JF,loop_break,ast))
        goto err; /* if (!(cond)) break; */

    /* Generate the loop itself. */
    if (ast->ast_loop.ast_loop &&
        ast_genasm(ast->ast_loop.ast_loop,ASM_G_FNORMAL)) goto err;

    if (ast->ast_loop.ast_next) {
     asm_defsym(loop_continue);
     if (ast_genasm(ast->ast_loop.ast_next,ASM_G_FNORMAL)) goto err;
    }
    /* Jump back to re-evaluate the condition. */
    if (asm_putddi(ast)) goto err;
    if (asm_gjmp(ASM_JMP,loop_block)) goto err;
   }
  }
  /* Reset the stack depth to its original value.
   * NOTE: The current value may be distorted and invalid due to
   *       stack variables having been initialized inside the loop. */
  ASM_BREAK_SCOPE(PUSH_RESULT ? 1 : 0,err);

  /* This is where `break' jumps to. (After the re-aligned stack) */
  asm_defsym(loop_break);

  ASSERT(current_assembler.a_finflag&ASM_FINFLAG_NOLOOP);
  current_assembler.a_finflag = old_finflag;

  /* Restore old loop control symbols. */
  current_assembler.a_loopctl[ASM_LOOPCTL_CON] = old_continue;
  current_assembler.a_loopctl[ASM_LOOPCTL_BRK] = old_break;
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
  ASSERT(ast->ast_flag == AST_FLOOPCTL_BRK ||
         ast->ast_flag == AST_FLOOPCTL_CON);
#if AST_FLOOPCTL_BRK == ASM_LOOPCTL_BRK && \
    AST_FLOOPCTL_CON == ASM_LOOPCTL_CON
  loopsym = current_assembler.a_loopctl[ast->ast_flag];
#else
  loopsym = current_assembler.a_loopctl[ast->ast_flag == AST_FLOOPCTL_BRK ?
                                        ASM_LOOPCTL_BRK : ASM_LOOPCTL_CON];
#endif
  if unlikely(!loopsym) {
   if (WARNAST(ast,W_ASM_BREAK_OR_CONTINUE_NOT_ALLOWED))
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
  if (asm_putddi(ast)) goto err;
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
  DeeAstObject *condition;
 case AST_CONDITIONAL:
  ASSERT_OBJECT_TYPE(ast->ast_conditional.ast_cond,&DeeAst_Type);
  ASSERTF(ast->ast_conditional.ast_tt ||
          ast->ast_conditional.ast_ff,
          "At least one branch must exist");
  ASSERTF((ast->ast_conditional.ast_tt != ast->ast_conditional.ast_cond) ||
          (ast->ast_conditional.ast_ff != ast->ast_conditional.ast_cond),
          "At most one branch can equal the conditional branch");
  invert_condition = false;

  /* Special handling for boolean conditions.
   * NOTE: Be careful when condition re-use for this one! */
  if (ast->ast_conditional.ast_cond->ast_type == AST_BOOL &&
      ast->ast_conditional.ast_tt != ast->ast_conditional.ast_cond &&
      ast->ast_conditional.ast_ff != ast->ast_conditional.ast_cond) {
   invert_condition = (ast->ast_conditional.ast_cond->ast_flag & AST_FBOOL_NEGATE);
   condition = ast->ast_conditional.ast_cond->ast_boolexpr;
  } else {
   condition = ast->ast_conditional.ast_cond;
  }

  if (ast->ast_conditional.ast_tt &&
      ast->ast_conditional.ast_ff) {
   unsigned int cond_flags = ASM_G_FPUSHRES|ASM_G_FLAZYBOOL;
   /* If the condition expression is re-used, we can't
    * have it auto-optimize itself into a boolean value
    * if the caller expects the real expression value. */
   if (!(gflags & ASM_G_FLAZYBOOL) &&
        (ast->ast_conditional.ast_tt == ast->ast_conditional.ast_cond ||
         ast->ast_conditional.ast_ff == ast->ast_conditional.ast_cond))
         cond_flags &= ~ASM_G_FLAZYBOOL;
   if (ast_genasm(condition,cond_flags)) goto err;
   /* Branch with specific code for both paths. */
   if (ast->ast_conditional.ast_tt == ast->ast_conditional.ast_cond ||
       ast->ast_conditional.ast_ff == ast->ast_conditional.ast_cond) {
    struct asm_sym *cond_end;
    /* Special case: re-use the condition as true or false branch. */

    /* If the condition will be re-used as result, and `AST_FCOND_BOOL' is set, we
     * must first convert the conditional into a boolean if it's not already one. */
    if (asm_putddi(ast)) goto err;
    if (PUSH_RESULT &&
       (ast->ast_flag&AST_FCOND_BOOL) &&
        ast_predict_type(condition) != &DeeBool_Type) {
     /* Force the condition to become a boolean. */
     if (asm_gbool(invert_condition)) goto err;
     invert_condition = false;
    }
    /*     push <cond>
     *    [dup]
     *     jt   1f  # Inverted by `invert_condition ^ (ast->ast_conditional.ast_ff == ast->ast_conditional.ast_cond)'
     *    [pop]
     *    [push] <false-branch> / <true-branch>
     *1:   */
    cond_end = asm_newsym();
    if unlikely(!cond_end) goto err;
    if (PUSH_RESULT && asm_gdup()) goto err;
    if (ast->ast_conditional.ast_ff == ast->ast_conditional.ast_cond)
        invert_condition = !invert_condition;
    if (ast->ast_flag&(AST_FCOND_LIKELY|AST_FCOND_UNLIKELY) &&
        current_assembler.a_curr != &current_assembler.a_sect[SECTION_COLD]) {
     /*     push <cond>
      *    [dup]
      *     jf   .cold.1f  # Inverted by `invert_condition ^ (ast->ast_conditional.ast_ff == ast->ast_conditional.ast_cond)'
      *2:
      *
      *.cold.1:
      *     pop
      *    [push] <false-branch> / <true-branch>
      *     jmp   2b */
     struct asm_sym *cold_entry = asm_newsym();
     struct asm_sec *prev_section; DeeAstObject *genast;
     if unlikely(!cold_entry) goto err;
     if (asm_putddi(ast)) goto err;
     if (asm_gjmp(invert_condition ? ASM_JT : ASM_JF,cold_entry)) goto err;
     asm_decsp();
     prev_section = current_assembler.a_curr;
     current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
     if (asm_gpop()) goto err;
     genast = ast->ast_conditional.ast_tt != ast->ast_conditional.ast_cond
            ? ast->ast_conditional.ast_tt
            : ast->ast_conditional.ast_ff;
     if (ast_genasm(genast,(ast->ast_flag&AST_FCOND_BOOL) ? (gflags|ASM_G_FLAZYBOOL) : gflags)) goto err;
     if (PUSH_RESULT && (ast->ast_flag&AST_FCOND_BOOL) &&
         ast_predict_type(genast) != &DeeBool_Type &&
         asm_gbool(false)) goto err;
     current_assembler.a_curr = prev_section;
    } else {
     DeeAstObject *genast;
     if (asm_gjmp(invert_condition ? ASM_JF : ASM_JT,cond_end)) goto err;
     asm_decsp();
     if (PUSH_RESULT && asm_gpop()) goto err;
     genast = ast->ast_conditional.ast_tt != ast->ast_conditional.ast_cond
            ? ast->ast_conditional.ast_tt
            : ast->ast_conditional.ast_ff;
     if (ast_genasm(genast,(ast->ast_flag&AST_FCOND_BOOL) ? (gflags|ASM_G_FLAZYBOOL) : gflags)) goto err;
     if (PUSH_RESULT && (ast->ast_flag&AST_FCOND_BOOL) &&
         ast_predict_type(genast) != &DeeBool_Type &&
         asm_gbool(false)) goto err;
    }
    asm_defsym(cond_end);
   } else if (ast->ast_flag&(AST_FCOND_LIKELY|AST_FCOND_UNLIKELY) &&
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
    DeeAstObject *likely_branch,*unlikely_branch;
    struct asm_sym *cold_entry = asm_newsym();
    struct asm_sym *text_return = asm_newsym();
    struct asm_sec *prev_section;
    bool likely_is_bool,unlikely_is_bool;
    if unlikely(!cold_entry || !text_return) goto err;
    if (ast->ast_flag&AST_FCOND_LIKELY) {
     likely_branch    = ast->ast_conditional.ast_tt;
     unlikely_branch  = ast->ast_conditional.ast_ff;
    } else {
     unlikely_branch  = ast->ast_conditional.ast_tt;
     likely_branch    = ast->ast_conditional.ast_ff;
     invert_condition = !invert_condition;
    }
    likely_is_bool = unlikely_is_bool = !PUSH_RESULT || !(ast->ast_flag&AST_FCOND_BOOL);
    if (!likely_is_bool)   likely_is_bool   = ast_predict_type(likely_branch) == &DeeBool_Type;
    if (!unlikely_is_bool) unlikely_is_bool = ast_predict_type(unlikely_branch) == &DeeBool_Type;

    if (asm_putddi(ast)) goto err;
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
    if (asm_putddi(ast)) goto err;
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
    tt_is_bool = ff_is_bool = !PUSH_RESULT || !(ast->ast_flag&AST_FCOND_BOOL);
    if (!tt_is_bool) tt_is_bool = ast_predict_type(ast->ast_conditional.ast_tt) == &DeeBool_Type;
    if (!ff_is_bool) ff_is_bool = ast_predict_type(ast->ast_conditional.ast_ff) == &DeeBool_Type;
    if (asm_putddi(ast)) goto err;
    if (asm_gjmp(invert_condition ? ASM_JT : ASM_JF,ff_enter)) goto err;
    asm_decsp(); /* Popped by `ASM_JT' / `ASM_JF' */
    if (ast_genasm(ast->ast_conditional.ast_tt,gflags)) goto err;
    if (asm_putddi(ast)) goto err;
    if (!tt_is_bool && ff_is_bool && asm_gbool(false)) goto err;
    if (asm_gjmp(ASM_JMP,ff_leave)) goto err;
    if (PUSH_RESULT) asm_decsp(); /* Adjust to before `tt' was executed. */
    asm_defsym(ff_enter);
    if (ast_genasm(ast->ast_conditional.ast_ff,gflags)) goto err;
    if (asm_putddi(ast)) goto err;
    if (tt_is_bool && !ff_is_bool && asm_gbool(false)) goto err;
    asm_defsym(ff_leave);
    if (!tt_is_bool && !ff_is_bool && asm_gbool(false)) goto err;
   }
  } else {
   /* Only the one of the branches exists. - the other should return `none'. */
   DeeAstObject *existing_branch;
   bool invert_boolean = invert_condition;
   ASSERT( ast->ast_conditional.ast_tt ||  ast->ast_conditional.ast_ff);
   ASSERT(!ast->ast_conditional.ast_tt || !ast->ast_conditional.ast_ff);
   existing_branch = ast->ast_conditional.ast_tt;
   if (!existing_branch) {
    existing_branch  = ast->ast_conditional.ast_ff;
    invert_condition = !invert_condition;
   }
   if (existing_branch == ast->ast_conditional.ast_cond) {
    if (!PUSH_RESULT) {
     if (ast_genasm(condition,ASM_G_FNORMAL)) goto err;
     goto done;
    }
    /* Special case: The only existing branch is a duplicate of the condition. */
    /*    push <cond>
     *   [bool]
     *    dup
     *    jf   1f  # Inverted by `existing_branch == ast->ast_conditional.ast_ff'
     *    pop
     *    push none
     *1: */
    if (ast_genasm(condition,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ast)) goto err;
    if (PUSH_RESULT && (ast->ast_flag&AST_FCOND_BOOL) &&
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
     *    jf   1f    # Inverted by `existing_branch == ast->ast_conditional.ast_ff'
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
     if (asm_putddi(ast)) goto err;
     if (ast->ast_flag&AST_FCOND_BOOL) {
      if (asm_gpush_constexpr(Dee_False)) goto err;
     } else {
      if (asm_gpush_none()) goto err;
     }
     if (current_assembler.a_flag&ASM_FSTACKDISP) {
      if (asm_gswap()) goto err;
     } else {
      if (ast_genasm(condition,ASM_G_FPUSHRES)) goto err;
     }
     if (asm_putddi(ast)) goto err;
     if (ast->ast_flag&AST_FCOND_BOOL &&
         ast_predict_type(condition) != &DeeBool_Type &&
         asm_gbool(false)) goto err;
     if (asm_gjmp(invert_condition ? ASM_JT : ASM_JF,after_existing)) goto err;
     asm_decsp(); /* Adjust for the value popped by `ASM_JT' / `ASM_JF' */
    } else {
     if (asm_gjcc(condition,
                  invert_condition ? ASM_JT : ASM_JF,
                  after_existing,
                  ast))
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
  instr = ASM_BOOL ^ (ast->ast_flag & AST_FBOOL_NEGATE);
#else
  instr = ASM_BOOL ^ (instruction_t)!!(ast->ast_flag & AST_FBOOL_NEGATE);
#endif
  if (ast_genasm(ast->ast_boolexpr,
                 ASM_G_FLAZYBOOL|
                 ASM_G_FPUSHRES))
      goto err;
  /* If the result won't be used, no need to do any inversion.
   * However, we must still invoke the bool operator of the
   * top-object, just in case doing so has any side-effects. */
  if (!PUSH_RESULT) instr = ASM_BOOL;
  if (instr == ASM_BOOL) {
   if (gflags & ASM_G_FLAZYBOOL) break;
   if (ast_predict_type(ast->ast_boolexpr) == &DeeBool_Type) break;
  }
  if (asm_putddi(ast)) goto err;
  if (asm_put(instr)) goto err;
  goto pop_unused;
 } break;

 case AST_EXPAND:
  if (PUSH_RESULT) {
   /* Expand to a single object by default. */
   if (asm_gunpack_expr(ast->ast_expandexpr,1,ast))
       goto err;
  } else {
   /* If the result isn't being used, follow the regular comma-rule
    * to generate expected assembly for code like this:
    * >> foo()...; // the results of foo() can be as comma-separated,
    * >>           // but doing so doesn't have any special meaning.
    */
   if (ast_genasm(ast->ast_expandexpr,gflags))
       goto err;
  }
  break;

 case AST_FUNCTION:
  if (!PUSH_RESULT) break;
  if (asm_gpush_function(ast))
      goto err;
  break;

 case AST_OPERATOR_FUNC:
  if (PUSH_RESULT) {
   /* Only need to generate the operator function binding if
    * the result of the expression is actually being used! */
   if unlikely(ast_gen_operator_func(ast->ast_operator_func.ast_binding,
                                     ast,ast->ast_flag))
      goto err;
  } else if (ast->ast_operator_func.ast_binding) {
   /* Still generate code the binding-expression (in case it has side-effects) */
   if (ast_genasm(ast->ast_operator_func.ast_binding,ASM_G_FNORMAL))
       goto err;
  }
  break;

 {
  uint16_t operator_name;
 case AST_OPERATOR:
  /* Probably one of the most important AST types: The operator AST. */
  operator_name = ast->ast_flag;
  /* Special case: The arguments of the operator are variadic. */
  if unlikely(ast->ast_operator.ast_exflag&AST_OPERATOR_FVARARGS) {
   struct symbol *prefix_symbol;
   struct opinfo *info = Dee_OperatorInfo(NULL,operator_name);
   BREAKPOINT();
   if (ast->ast_operator.ast_opa->ast_type == AST_SYM &&
      (!info || (info->oi_type&OPTYPE_INPLACE))) {
    /* Generate a prefixed instruction. */
    prefix_symbol = ast->ast_operator.ast_opa->ast_sym;
    if ((ast->ast_operator.ast_exflag&AST_OPERATOR_FMAYBEPFX) &&
        !asm_can_prefix_symbol(prefix_symbol))
         goto varop_without_prefix;
   } else {
varop_without_prefix:
    if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
    prefix_symbol = NULL;
   }
   /* Compile operand tuple or push an empty one. */
   if (ast->ast_operator.ast_opb) {
    if (ast_genasm(ast->ast_operator.ast_opb,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ast)) goto err;
   } else {
    if (asm_putddi(ast)) goto err;
    if (asm_gpack_tuple(0)) goto err;
   }
   if (prefix_symbol) {
    if (asm_gprefix_symbol(prefix_symbol,ast->ast_operator.ast_opa)) goto err;
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
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   if (asm_gcall_expr(ast->ast_operator.ast_opa,
                      ast->ast_operator.ast_opb,
                      ast,gflags)) goto err;
   goto done;

  {
   DeeObject *index; int32_t temp;
   DeeAstObject *sequence;
  case OPERATOR_GETITEM:
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   sequence = ast->ast_operator.ast_opa;
   for (;;) {
    DeeAstObject *inner;
    if (sequence->ast_type != AST_MULTIPLE) break;
    if (sequence->ast_flag == AST_FMULTIPLE_KEEPLAST) break;
    if (sequence->ast_multiple.ast_exprc != 1) break;
    inner = sequence->ast_multiple.ast_exprv[0];
    if (inner->ast_type != AST_EXPAND) break;
    sequence = inner->ast_expandexpr;
   }
   if (sequence->ast_type == AST_SYM) {
    struct symbol *sym = sequence->ast_sym;
    SYMBOL_INPLACE_UNWIND_ALIAS(sym);
    if (SYMBOL_TYPE(sym) == SYMBOL_TYPE_ARG &&
       !SYMBOL_MUST_REFERENCE_TYPEMAY(sym) &&
       (current_basescope->bs_flags & CODE_FVARARGS) &&
        DeeBaseScope_IsArgVarArgs(current_basescope,sym->s_symid)) {
     uint32_t va_index;
     if (!PUSH_RESULT) {
      if (ast_genasm(ast->ast_operator.ast_opb,ASM_G_FNORMAL))
          goto err;
      goto done;
     }
     /* Lookup a varargs-argument by index. */
     if (ast->ast_operator.ast_opb->ast_type == AST_CONSTEXPR &&
         DeeInt_Check(ast->ast_operator.ast_opb->ast_constexpr) &&
         DeeInt_TryAsU32(ast->ast_operator.ast_opb->ast_constexpr,&va_index)) {
      /* Optional arguments are actually apart of varargs, so
       * we need to adjust the user-given index to have its base
       * be located just after the varargs. */
      va_index += current_basescope->bs_argc_opt;
      if (va_index <= UINT8_MAX) {
       if (asm_putddi(ast)) goto err;
       if (asm_gvarargs_getitem_i((uint8_t)va_index)) goto err;
       goto done;
      }
     }
     if (ast_genasm(ast->ast_operator.ast_opb,ASM_G_FPUSHRES)) goto err;
     if (asm_putddi(ast)) goto err;
     if (DeeBaseScope_HasOptional(current_basescope)) {
      /* Must adjust for optional arguments by adding their amount to the index. */
      if (asm_gadd_imm32(current_basescope->bs_argc_opt))
          goto err;
     }
     if (asm_gvarargs_getitem()) goto err;
     goto done;
    }
   }

   if (ast->ast_operator.ast_opb->ast_type != AST_CONSTEXPR)
       break;
   index = ast->ast_operator.ast_opb->ast_constexpr;
   /* Special optimizations for integer indices. */
   if (DeeInt_Check(index) &&
       DeeInt_TryAsS32(index,&temp) &&
       temp >= INT16_MIN && temp <= INT16_MAX) {
    if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ast) || asm_ggetitem_index((int16_t)temp)) goto err;
    goto pop_unused;
   }
   /* Special optimizations for constant indices. */
   if (asm_allowconst(index)) {
    temp = asm_newconst(index);
    if unlikely(temp < 0) goto err;
    if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ast) || asm_ggetitem_const((uint16_t)temp)) goto err;
    goto pop_unused;
   }
  } break;

  case OPERATOR_GETATTR:
   /* Special optimizations when the attribute name is known at compile-time. */
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   if (ast->ast_operator.ast_opb->ast_type == AST_CONSTEXPR &&
       DeeString_Check(ast->ast_operator.ast_opb->ast_constexpr)) {
    DeeStringObject *name; int32_t attrid;
    name = (DeeStringObject *)ast->ast_operator.ast_opb->ast_constexpr;
    if (ast->ast_operator.ast_opa->ast_type == AST_SYM) {
     struct symbol *sym = ast->ast_operator.ast_opa->ast_sym;
check_getattr_sym:
     switch (SYMBOL_TYPE(sym)) {
     case SYMBOL_TYPE_ALIAS:
      ASSERT(SYMBOL_TYPE(SYMBOL_ALIAS(sym)) != SYMBOL_TYPE_ALIAS);
      sym = SYMBOL_ALIAS(sym);
      goto check_getattr_sym;

     case SYMBOL_TYPE_THIS:
      attrid = asm_newconst((DeeObject *)name);
      if unlikely(attrid < 0) goto err;
      if (asm_putddi(ast)) goto err;
      if (asm_ggetattr_this_const((uint16_t)attrid)) goto err;
      goto pop_unused;
     {
      struct module_symbol *modsym; int32_t module_id;
     case SYMBOL_TYPE_MODULE: /* module.attr --> push extern ... */
      modsym = get_module_symbol(SYMBOL_MODULE_MODULE(sym),name);
      if (!modsym) break;
      if (!PUSH_RESULT) goto done;
      module_id = asm_msymid(sym);
      if unlikely(module_id < 0) goto err;
      /* Push an external symbol accessed through its module. */
      if (asm_putddi(ast)) goto err;
      if (asm_gpush_extern((uint16_t)module_id,modsym->ss_index)) goto err;
      goto done;
     } break;
     default: break;
     }
    }
    if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
    attrid = asm_newconst((DeeObject *)name);
    if unlikely(attrid < 0) goto err;
    if (asm_putddi(ast)) goto err;
    if (asm_ggetattr_const((uint16_t)attrid)) goto err;
    goto pop_unused;
   }
   break;

  {
   DeeAstObject *begin,*end; int32_t intval;
  case OPERATOR_GETRANGE:
   if unlikely(!ast->ast_operator.ast_opc) goto generic_operator;
   if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
   begin = ast->ast_operator.ast_opb;
   end = ast->ast_operator.ast_opc;
   if (begin->ast_type == AST_CONSTEXPR) {
    DeeObject *const_begin = begin->ast_constexpr;
    if (end->ast_type == AST_CONSTEXPR) {
     DeeObject *const_end = end->ast_constexpr;
     if (DeeInt_Check(const_begin) &&
         DeeInt_TryAsS32(const_begin,&intval) &&
         intval >= INT16_MIN && intval <= INT16_MAX) {
      int32_t endval;
      if (DeeNone_Check(const_end)) {
       if (asm_putddi(ast)) goto err;
       if (asm_ggetrange_in((int16_t)intval)) goto err;
       goto pop_unused;
      }
      if (DeeInt_Check(const_end) &&
          DeeInt_TryAsS32(const_end,&endval) &&
          endval >= INT16_MIN && endval <= INT16_MAX) {
       if (asm_putddi(ast)) goto err;
       if (asm_ggetrange_ii((int16_t)intval,(int16_t)endval)) goto err;
       goto pop_unused;
      }
     } else if (DeeNone_Check(const_begin) &&
                DeeInt_Check(const_end) &&
                DeeInt_TryAsS32(const_end,&intval) &&
                intval >= INT16_MIN && intval <= INT16_MAX) {
      if (asm_putddi(ast)) goto err;
      if (asm_ggetrange_ni((int16_t)intval)) goto err;
      goto pop_unused;
     }
    }
    if (DeeNone_Check(const_begin)) {
     if (ast_genasm(end,ASM_G_FPUSHRES)) goto err;
     if (asm_putddi(ast)) goto err;
     if (asm_ggetrange_np()) goto err;
     goto pop_unused;
    }
    if (DeeInt_Check(const_begin) &&
        DeeInt_TryAsS32(const_begin,&intval) &&
        intval >= INT16_MIN && intval <= INT16_MAX) {
     if (ast_genasm(end,ASM_G_FPUSHRES)) goto err;
     if (asm_putddi(ast)) goto err;
     if (asm_ggetrange_ip((int16_t)intval)) goto err;
     goto pop_unused;
    }
   } else if (end->ast_type == AST_CONSTEXPR) {
    DeeObject *const_end = end->ast_constexpr;
    if (DeeNone_Check(const_end)) {
     if (ast_genasm(begin,ASM_G_FPUSHRES)) goto err;
     if (asm_putddi(ast)) goto err;
     if (asm_ggetrange_pn()) goto err;
     goto pop_unused;
    }
    if (DeeInt_Check(const_end) &&
        DeeInt_TryAsS32(const_end,&intval) &&
        intval >= INT16_MIN && intval <= INT16_MAX) {
     if (ast_genasm(begin,ASM_G_FPUSHRES)) goto err;
     if (asm_putddi(ast)) goto err;
     if (asm_ggetrange_pi((int16_t)intval)) goto err;
     goto pop_unused;
    }
   }
   if (ast_genasm(begin,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(end,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast)) goto err;
   if (asm_ggetrange()) goto err;
   goto pop_unused;
  }


  case OPERATOR_DELATTR:
   /* Special optimizations when the attribute name is known at compile-time. */
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   if (ast->ast_operator.ast_opb->ast_type == AST_CONSTEXPR &&
       DeeString_Check(ast->ast_operator.ast_opb->ast_constexpr)) {
    int32_t attrid = asm_newconst(ast->ast_operator.ast_opb->ast_constexpr);
    if unlikely(attrid < 0) goto err;
    if (ast->ast_operator.ast_opa->ast_type == AST_SYM) {
     struct symbol *sym = ast->ast_operator.ast_opa->ast_sym;
     SYMBOL_INPLACE_UNWIND_ALIAS(sym);
     if (SYMBOL_TYPE(sym) == SYMBOL_TYPE_THIS &&
        !SYMBOL_MUST_REFERENCE_TYPEMAY(sym)) {
      if (asm_gdelattr_this_const(attrid)) goto err;
      goto done_push_none;
     }
    }
    if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
    if (asm_gdelattr_const(attrid)) goto err;
    goto done_push_none;
   }
   break;

  case OPERATOR_SETITEM:
   if unlikely(!ast->ast_operator.ast_opc) goto generic_operator;
   if (ast_gen_setitem(ast->ast_operator.ast_opa,
                       ast->ast_operator.ast_opb,
                       ast->ast_operator.ast_opc,
                       ast,gflags)) goto err;
   goto done;
  case OPERATOR_SETATTR:
   if unlikely(!ast->ast_operator.ast_opc) goto generic_operator;
   if (ast_gen_setattr(ast->ast_operator.ast_opa,
                       ast->ast_operator.ast_opb,
                       ast->ast_operator.ast_opc,
                       ast,gflags)) goto err;
   goto done;

   /* Arithmetic-with-constant-operand optimizations. */
  {
   int32_t intval;
  case OPERATOR_ADD:
  case OPERATOR_SUB:
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   if (ast->ast_operator.ast_opb->ast_type != AST_CONSTEXPR) break;
   if (!DeeInt_Check(ast->ast_operator.ast_opb->ast_constexpr)) break;
   if (DeeInt_TryAsS32(ast->ast_operator.ast_opb->ast_constexpr,&intval) &&
      (intval >= INT8_MIN && intval <= INT8_MAX)) {
    if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
    if (operator_name == OPERATOR_ADD ? asm_gadd_simm8((int8_t)intval)
                                      : asm_gsub_simm8((int8_t)intval))
        goto err;
    goto pop_unused;
   }
   if (DeeInt_TryAsU32(ast->ast_operator.ast_opb->ast_constexpr,
                       (uint32_t *)&intval)) {
    if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
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
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   if (ast->ast_operator.ast_opb->ast_type != AST_CONSTEXPR) break;
   if (ast->ast_operator.ast_opa->ast_type != AST_SYM) break;
   if (!asm_can_prefix_symbol(ast->ast_operator.ast_opa->ast_sym)) break;
   if (!DeeInt_Check(ast->ast_operator.ast_opb->ast_constexpr)) break;
   if (DeeInt_TryAsS32(ast->ast_operator.ast_opb->ast_constexpr,&intval) &&
      (intval >= INT8_MIN && intval <= INT8_MAX)) {
    if (asm_gprefix_symbol(ast->ast_operator.ast_opa->ast_sym,
                           ast->ast_operator.ast_opa)) goto err;
    if (operator_name == OPERATOR_INPLACE_ADD ? asm_gadd_inplace_simm8((int8_t)intval)
                                              : asm_gsub_inplace_simm8((int8_t)intval))
        goto err;
push_a_if_used:
    if (PUSH_RESULT && ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES))
        goto err;
    goto done;
   }
   if (DeeInt_TryAsU32(ast->ast_operator.ast_opb->ast_constexpr,
                       (uint32_t *)&intval)) {
    if (asm_gprefix_symbol(ast->ast_operator.ast_opa->ast_sym,
                           ast->ast_operator.ast_opa)) goto err;
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
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   if (ast->ast_operator.ast_opb->ast_type != AST_CONSTEXPR) break;
   if (!DeeInt_Check(ast->ast_operator.ast_opb->ast_constexpr)) break;
   if (DeeInt_TryAsS32(ast->ast_operator.ast_opb->ast_constexpr,&intval) &&
      (intval >= INT8_MIN && intval <= INT8_MAX)) {
    if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
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
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   if (ast->ast_operator.ast_opb->ast_type != AST_CONSTEXPR) break;
   if (ast->ast_operator.ast_opa->ast_type != AST_SYM) break;
   if (!asm_can_prefix_symbol(ast->ast_operator.ast_opa->ast_sym)) break;
   if (!DeeInt_Check(ast->ast_operator.ast_opb->ast_constexpr)) break;
   if (DeeInt_TryAsS32(ast->ast_operator.ast_opb->ast_constexpr,&intval) &&
      (intval >= INT8_MIN && intval <= INT8_MAX)) {
    if (asm_gprefix_symbol(ast->ast_operator.ast_opa->ast_sym,
                           ast->ast_operator.ast_opa)) goto err;
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
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   if (ast->ast_operator.ast_opb->ast_type != AST_CONSTEXPR) break;
   if (!DeeInt_Check(ast->ast_operator.ast_opb->ast_constexpr)) break;
   if (DeeInt_TryAsU32(ast->ast_operator.ast_opb->ast_constexpr,&intval)) {
    if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
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
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   if (ast->ast_operator.ast_opb->ast_type != AST_CONSTEXPR) break;
   if (ast->ast_operator.ast_opa->ast_type != AST_SYM) break;
   if (!asm_can_prefix_symbol(ast->ast_operator.ast_opa->ast_sym)) break;
   if (!DeeInt_Check(ast->ast_operator.ast_opb->ast_constexpr)) break;
   if (DeeInt_TryAsU32(ast->ast_operator.ast_opb->ast_constexpr,&intval)) {
    if (asm_gprefix_symbol(ast->ast_operator.ast_opa->ast_sym,
                           ast->ast_operator.ast_opa)) goto err;
    if (operator_name == OPERATOR_INPLACE_AND ? asm_gand_inplace_imm32(intval) :
        operator_name == OPERATOR_INPLACE_OR  ? asm_gor_inplace_imm32(intval) :
                                                asm_gxor_inplace_imm32(intval))
        goto err;
    goto push_a_if_used;
   }
  } break;

  {
   DeeAstObject *enter_expr;
  case OPERATOR_ENTER:
   enter_expr = ast->ast_operator.ast_opa;
   if (enter_expr->ast_type == AST_SYM) {
    SYMBOL_INPLACE_UNWIND_ALIAS(enter_expr->ast_sym);
    if (SYMBOL_TYPE(enter_expr->ast_sym) == SYMBOL_TYPE_STACK &&
       !SYMBOL_MUST_REFERENCE_TYPEMAY(enter_expr->ast_sym) &&
       (enter_expr->ast_sym->s_flag & SYMBOL_FALLOC) &&
        SYMBOL_STACK_OFFSET(enter_expr->ast_sym) == current_assembler.a_stackcur - 1) {
     /* Special optimization: Since `ASM_ENTER' doesn't modify the top stack item,
      * if the operand _is_ the top stack item, then we can simply generate the enter
      * instruction without the need of any kludge. */
     if (asm_putddi(ast) || asm_genter()) goto err;
     goto done_push_none;
    }
   }
   if (ast_genasm(enter_expr,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast) || asm_genter() || asm_gpop()) goto err;
   goto done_push_none;
  }

  case OPERATOR_LEAVE:
   /* NOTE: The case of the operand being stack-top, in which
    *       case `dup; leave pop; pop;' is generated, will later
    *       be optimized away by the peephole optimizer. */
   if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast) || asm_gleave()) goto err;
   goto done_push_none;

  case OPERATOR_ITERNEXT:
   if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast) || asm_giternext()) goto err; /* This one's an extended instruction... */
   goto pop_unused;

  case OPERATOR_SIZE:
   if unlikely(!ast->ast_operator.ast_opa) goto generic_operator;
   if ((current_basescope->bs_flags & CODE_FVARARGS) &&
        ast->ast_operator.ast_opa->ast_type == AST_SYM) {
    struct symbol *sym = ast->ast_operator.ast_opa->ast_sym;
    SYMBOL_INPLACE_UNWIND_ALIAS(sym);
    if (SYMBOL_TYPE(sym) == SYMBOL_TYPE_ARG &&
       !SYMBOL_MUST_REFERENCE_TYPEMAY(sym) &&
        DeeBaseScope_IsArgVarArgs(current_basescope,
                                  sym->s_symid)) {
     /* Special case: Get the size of varargs. */
     if (!PUSH_RESULT) goto done;
     if (asm_putddi(ast)) goto err;
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
   DeeAstObject *sizeast;
   DeeObject *sizeval;
   uint32_t va_size_val;
  case OPERATOR_EQ:
  case OPERATOR_NE:
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   /* Check for special case:
    * >> if (#varargs == 42) ...
    * >> if (42 == #varargs) ...
    * There is a dedicated instruction for comparing the size of varargs.
    */
   if unlikely(!(current_basescope->bs_flags & CODE_FVARARGS)) break;
   if (ast->ast_operator.ast_opa->ast_type == AST_OPERATOR) {
    if (ast->ast_operator.ast_opa->ast_flag != OPERATOR_SIZE) break;
    if (ast->ast_operator.ast_opa->ast_operator.ast_exflag&
       (AST_OPERATOR_FPOSTOP|AST_OPERATOR_FVARARGS)) break;
    if (!ast->ast_operator.ast_opa->ast_operator.ast_opa) break;
    if (ast->ast_operator.ast_opa->ast_operator.ast_opa->ast_type != AST_SYM) break;
    sym     = ast->ast_operator.ast_opa->ast_operator.ast_opa->ast_sym;
    sizeast = ast->ast_operator.ast_opb;
   } else {
    if (ast->ast_operator.ast_opb->ast_type != AST_OPERATOR) break;
    if (ast->ast_operator.ast_opb->ast_flag != OPERATOR_SIZE) break;
    if (ast->ast_operator.ast_opb->ast_operator.ast_exflag&
       (AST_OPERATOR_FPOSTOP|AST_OPERATOR_FVARARGS)) break;
    if (!ast->ast_operator.ast_opb->ast_operator.ast_opa) break;
    if (ast->ast_operator.ast_opb->ast_operator.ast_opa->ast_type != AST_SYM) break;
    sym     = ast->ast_operator.ast_opb->ast_operator.ast_opa->ast_sym;
    sizeast = ast->ast_operator.ast_opa;
   }
   SYMBOL_INPLACE_UNWIND_ALIAS(sym);
   if (SYMBOL_TYPE(sym) != SYMBOL_TYPE_ARG) break;
   if (SYMBOL_MUST_REFERENCE_TYPEMAY(sym)) break;
   if (!DeeBaseScope_IsArgVarArgs(current_basescope,sym->s_symid)) break;
   if (sizeast->ast_type != AST_CONSTEXPR) break;
   sizeval = sizeast->ast_constexpr;
   if (!DeeInt_Check(sizeval)) break;
   /* If the expression result isn't being used,
    * then there is no need to do anything! */
   if (!PUSH_RESULT) goto done;
   if (!DeeInt_TryAsU32(sizeval,&va_size_val)) break;
   /* Adjust for the number of optional arguments. */
   va_size_val += current_basescope->bs_argc_opt;
   if (va_size_val > UINT8_MAX) break;
   /* All right! we can encode this one as `cmp eq, #varargs, $<va_size_val>' */
   if (asm_putddi(ast)) goto err;
   if (_asm_gcmp_eq_varargs_sz((uint8_t)va_size_val)) goto err;
   /* If the expression is checking for inequality, invert the result. */
   if (ast->ast_flag == OPERATOR_NE && asm_gbool(true)) goto err;
   goto done;
  } break;

  case OPERATOR_CONTAINS:
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   if (ast_genasm_set(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(ast->ast_operator.ast_opb,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast) || asm_gcontains()) goto err;
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
   if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast)) goto err;
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
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(ast->ast_operator.ast_opb,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast)) goto err;
   ASSERT(operator_instr_table[operator_name] != 0);
   if ((asm_ddicsp(),asm_put(operator_instr_table[operator_name]))) goto err;
   goto pop_unused;

  case OPERATOR_DELITEM:
  case OPERATOR_DELATTR:
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(ast->ast_operator.ast_opb,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast)) goto err;
   ASSERT(operator_instr_table[operator_name] != 0);
   if ((asm_ddcsp(),asm_put(operator_instr_table[operator_name]))) goto err;
   goto done_push_none;

  case OPERATOR_ASSIGN:
  case OPERATOR_MOVEASSIGN:
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   ASSERT(operator_instr_table[operator_name] != 0);
   if (PUSH_RESULT &&
       ast_can_exchange(ast->ast_operator.ast_opa,
                        ast->ast_operator.ast_opb)) {
    /* Optimization when A and B can be exchanged. */
    if (ast_genasm(ast->ast_operator.ast_opb,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ast) || asm_gdup()) goto err;
    if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ast) || asm_gswap()) goto err;
   } else {
    if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
    if (ast_genasm(ast->ast_operator.ast_opb,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ast)) goto err;
    if (PUSH_RESULT && (asm_gdup() || asm_grrot(3))) goto err;
   }
   if ((asm_ddcsp(),asm_put(operator_instr_table[operator_name]))) goto err;
   goto done;

  case OPERATOR_DELRANGE:
   if unlikely(!ast->ast_operator.ast_opb) goto generic_operator;
   if unlikely(!ast->ast_operator.ast_opc) goto generic_operator;
   if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(ast->ast_operator.ast_opb,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(ast->ast_operator.ast_opc,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast)) goto err;
   ASSERT(operator_instr_table[operator_name] != 0);
   if ((asm_dddcsp(),asm_put(operator_instr_table[operator_name]))) goto err;
   goto done_push_none;

  default: break;
  }

  if (OPERATOR_ISINPLACE(operator_name)) {
   bool is_unary = operator_name <= OPERATOR_DEC;
   DeeAstObject *opa;

   /* Inplace operators with dedicated prefix-instructions. */
   ASSERT(operator_instr_table[operator_name] != 0);
   opa = ast->ast_operator.ast_opa;
   switch (opa->ast_type) {

   case AST_SYM:
    if (ast_gen_symbol_inplace(opa->ast_sym,
                               is_unary ? NULL : ast->ast_operator.ast_opb,
                               ast,
                               operator_name,
                              (ast->ast_operator.ast_exflag & AST_OPERATOR_FPOSTOP) != 0,
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
    if (!opa->ast_operator.ast_opb)
        goto generic_operator;
    switch (opa->ast_flag) {

    case OPERATOR_GETATTR:
     if (ast_gen_setattr_inplace(opa->ast_operator.ast_opa,
                                 opa->ast_operator.ast_opb,
                                 is_unary ? NULL : ast->ast_operator.ast_opb,
                                 ast,
                                 operator_name,
                                (ast->ast_operator.ast_exflag & AST_OPERATOR_FPOSTOP) != 0,
                                 gflags))
         goto err;
     goto done;

    case OPERATOR_GETITEM:
     if (ast_gen_setitem_inplace(opa->ast_operator.ast_opa,
                                 opa->ast_operator.ast_opb,
                                 is_unary ? NULL : ast->ast_operator.ast_opb,
                                 ast,
                                 operator_name,
                                (ast->ast_operator.ast_exflag & AST_OPERATOR_FPOSTOP) != 0,
                                 gflags))
         goto err;
     goto done;

    case OPERATOR_GETRANGE:
     if (!opa->ast_operator.ast_opc) goto generic_operator;
     if (ast_gen_setrange_inplace(opa->ast_operator.ast_opa,
                                  opa->ast_operator.ast_opb,
                                  opa->ast_operator.ast_opc,
                                  is_unary ? NULL : ast->ast_operator.ast_opb,
                                  ast,
                                  operator_name,
                                 (ast->ast_operator.ast_exflag & AST_OPERATOR_FPOSTOP) != 0,
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
   if (ast->ast_operator.ast_opa->ast_type == AST_SYM &&
      (!info || (info->oi_type&OPTYPE_INPLACE))) {
    /* Generate a prefixed instruction. */
    prefix_symbol = ast->ast_operator.ast_opa->ast_sym;
    /* Make sure that the symbol can actually be used as a prefix. */
    if ((ast->ast_operator.ast_exflag&AST_OPERATOR_FMAYBEPFX) &&
        !asm_can_prefix_symbol(prefix_symbol))
         goto operator_without_prefix;
   } else {
operator_without_prefix:
    if (ast_genasm(ast->ast_operator.ast_opa,ASM_G_FPUSHRES)) goto err;
    prefix_symbol = NULL;
   }
   /* Compile operands. */
   if (ast->ast_operator.ast_opb &&
      (++argc,ast_genasm(ast->ast_operator.ast_opb,ASM_G_FPUSHRES))) goto err;
   if (ast->ast_operator.ast_opc &&
      (++argc,ast_genasm(ast->ast_operator.ast_opc,ASM_G_FPUSHRES))) goto err;
   if (ast->ast_operator.ast_opd &&
      (++argc,ast_genasm(ast->ast_operator.ast_opd,ASM_G_FPUSHRES))) goto err;
   if (asm_putddi(ast)) goto err;
   if (prefix_symbol) {
    if (asm_gprefix_symbol(prefix_symbol,ast->ast_operator.ast_opa)) goto err;
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
  action_type = ast->ast_flag & AST_FACTION_KINDMASK;
  ASSERT(AST_FACTION_ARGC_GT(ast->ast_flag) <= 3);
  ASSERT((AST_FACTION_ARGC_GT(ast->ast_flag) >= 3) == (ast->ast_action.ast_act2 != NULL));
  ASSERT((AST_FACTION_ARGC_GT(ast->ast_flag) >= 2) == (ast->ast_action.ast_act1 != NULL));
  ASSERT((AST_FACTION_ARGC_GT(ast->ast_flag) >= 1) == (ast->ast_action.ast_act0 != NULL));
  switch (action_type) {
#define ACTION(x) case x & AST_FACTION_KINDMASK:
  {
   int32_t deemon_modid;
  ACTION(AST_FACTION_CELL0)
   if (!PUSH_RESULT) goto done;
   deemon_modid = asm_newmodule(get_deemon_module());
   if unlikely(deemon_modid < 0) goto err;
   if (asm_gcall_extern((uint16_t)deemon_modid,id_cell,0)) goto err;
  } break;
  {
   int32_t deemon_modid;
  ACTION(AST_FACTION_CELL1)
   if (ast_genasm(ast->ast_action.ast_act0,PUSH_RESULT)) goto err;
   if (!PUSH_RESULT) goto done;
   deemon_modid = asm_newmodule(get_deemon_module());
   if unlikely(deemon_modid < 0) goto err;
   if (asm_gcall_extern((uint16_t)deemon_modid,id_cell,1)) goto err;
  } break;
  ACTION(AST_FACTION_TYPEOF)
   if (ast_genasm(ast->ast_action.ast_act0,PUSH_RESULT)) goto err;
   if (PUSH_RESULT && (asm_putddi(ast) || asm_gtypeof())) goto err;
   break;
  ACTION(AST_FACTION_CLASSOF)
   if (ast_genasm(ast->ast_action.ast_act0,PUSH_RESULT)) goto err;
   if (PUSH_RESULT && (asm_putddi(ast) || asm_gclassof())) goto err;
   break;
  ACTION(AST_FACTION_SUPEROF)
   if (ast_genasm(ast->ast_action.ast_act0,PUSH_RESULT)) goto err;
   if (PUSH_RESULT && (asm_putddi(ast) || asm_gsuperof())) goto err;
   break;
  ACTION(AST_FACTION_AS)
   if (PUSH_RESULT &&
       ast->ast_action.ast_act0->ast_type == AST_SYM) {
    struct symbol *this_sym = ast->ast_action.ast_act0->ast_sym;
    SYMBOL_INPLACE_UNWIND_ALIAS(this_sym);
    if (SYMBOL_TYPE(this_sym) == SYMBOL_TYPE_THIS &&
        ast->ast_action.ast_act1->ast_type == AST_SYM) {
     /* Special optimizations for `this as ...' */
     int32_t symid;
     struct symbol *typesym = ast->ast_action.ast_act1->ast_sym;
cast_this_as_symbol:
     if (SYMBOL_MUST_REFERENCE(typesym)) {
      symid = asm_rsymid(typesym);
      if unlikely(symid < 0) goto err;
      if (asm_gsuper_this_r(symid)) goto err;
      goto done;
     }
     switch (SYMBOL_TYPE(typesym)) {
     case SYMBOL_TYPE_ALIAS:
      ASSERT(SYMBOL_TYPE(SYMBOL_ALIAS(typesym)) != SYMBOL_TYPE_ALIAS);
      typesym = SYMBOL_ALIAS(typesym);
      goto cast_this_as_symbol;

     case SYMBOL_TYPE_GLOBAL:
      symid = asm_gsymid_for_read(typesym,ast->ast_action.ast_act1);
      if unlikely(symid < 0) goto err;
      if (asm_gsuper_this_g(symid)) goto err;
      goto done;

     case SYMBOL_TYPE_EXTERN:
      if (SYMBOL_EXTERN_SYMBOL(typesym)->ss_flags & MODSYM_FPROPERTY)
          break;
      symid = asm_esymid(typesym);
      if unlikely(symid < 0) goto err;
      if (asm_gsuper_this_e(symid,SYMBOL_EXTERN_SYMBOL(typesym)->ss_index)) goto err;
      goto done;

     default: break;
     }
    }
   }
   if (PUSH_RESULT &&
       ast_can_exchange(ast->ast_action.ast_act0,
                        ast->ast_action.ast_act1)) {
    /* Optimization when `ACT0' and `ACT1' can be exchanged. */
    if (ast_genasm(ast->ast_action.ast_act1,ASM_G_FPUSHRES)) goto err;
    if (ast_genasm(ast->ast_action.ast_act0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ast) || asm_gsuper()) goto err;
   } else {
    if (ast_genasm(ast->ast_action.ast_act0,PUSH_RESULT)) goto err;
    if (ast_genasm(ast->ast_action.ast_act1,PUSH_RESULT)) goto err;
    if (PUSH_RESULT && (asm_putddi(ast) || asm_gswap() || asm_gsuper())) goto err;
   }
   break;
  ACTION(AST_FACTION_PRINT)
   if unlikely(ast_genprint(PRINT_MODE_NORMAL+PRINT_MODE_ALL,
                            ast->ast_action.ast_act0,ast))
      goto err;
   goto done_push_none;
  ACTION(AST_FACTION_PRINTLN)
   if unlikely(ast_genprint(PRINT_MODE_NL+PRINT_MODE_ALL,
                            ast->ast_action.ast_act0,ast))
      goto err;
   goto done_push_none;
  ACTION(AST_FACTION_FPRINT)
   if (ast_genasm(ast->ast_action.ast_act0,ASM_G_FPUSHRES)) goto err;
   if unlikely(ast_genprint(PRINT_MODE_NORMAL+PRINT_MODE_FILE+PRINT_MODE_ALL,
                            ast->ast_action.ast_act1,ast))
      goto err;
   goto pop_unused;
  ACTION(AST_FACTION_FPRINTLN)
   if (ast_genasm(ast->ast_action.ast_act0,ASM_G_FPUSHRES)) goto err;
   if unlikely(ast_genprint(PRINT_MODE_NL+PRINT_MODE_FILE+PRINT_MODE_ALL,
                            ast->ast_action.ast_act1,ast))
      goto err;
   goto pop_unused;
  ACTION(AST_FACTION_RANGE)
   if (ast->ast_action.ast_act0->ast_type == AST_CONSTEXPR) {
    DeeObject *index_object = ast->ast_action.ast_act0->ast_constexpr;
    uint32_t intval;
    if (DeeNone_Check(index_object) ||
       (DeeInt_Check(index_object) &&
       (DeeInt_TryAsU32(index_object,&intval) && intval == 0))) {
     /* Special optimization: The begin index is not set, or equal to int(0). */
     if (AST_ISNONE(ast->ast_action.ast_act2) &&
        (ast->ast_action.ast_act1->ast_type == AST_CONSTEXPR &&
        (index_object = ast->ast_action.ast_act1->ast_constexpr,
         DeeInt_Check(index_object)) &&
         DeeInt_TryAsU32(index_object,&intval))) {
      /* Special optimization: No step is given and the
       * end index is known to be a constant integer. */
      if (PUSH_RESULT && (asm_putddi(ast) || asm_grange_0_i(intval))) goto err;
      break;
     }
     if (ast_genasm(ast->ast_action.ast_act1,PUSH_RESULT)) goto err;
     if (AST_ISNONE(ast->ast_action.ast_act2)) {
      if (PUSH_RESULT && (asm_putddi(ast) || asm_grange_0())) goto err;
     } else {
      if (ast_genasm(ast->ast_action.ast_act2,PUSH_RESULT)) goto err;
      if (PUSH_RESULT && (asm_putddi(ast) || asm_grange_step_0())) goto err;
     }
     break;
    }
   }
   if (ast_genasm(ast->ast_action.ast_act0,PUSH_RESULT)) goto err;
   if (ast_genasm(ast->ast_action.ast_act1,PUSH_RESULT)) goto err;
   if (AST_ISNONE(ast->ast_action.ast_act2)) {
    if (PUSH_RESULT && (asm_putddi(ast) || asm_grange())) goto err;
   } else {
    if (ast_genasm(ast->ast_action.ast_act2,PUSH_RESULT)) goto err;
    if (PUSH_RESULT && (asm_putddi(ast) || asm_grange_step())) goto err;
   }
   break;
  ACTION(AST_FACTION_IS)
   if (ast_genasm(ast->ast_action.ast_act0,PUSH_RESULT)) goto err;
   if (PUSH_RESULT &&
      (ast->ast_action.ast_act1->ast_type == AST_CONSTEXPR &&
      (ast->ast_action.ast_act1->ast_constexpr == Dee_None ||
       ast->ast_action.ast_act1->ast_constexpr == (DeeObject *)&DeeNone_Type))) {
    /* Optimization for code like this: `foo is none'.
     * A special opcode exists for this case because a lot of code uses
     * `none' as placeholder in default arguments, relying on the `is'
     * operator to check if the argument has a meaningful value.
     * In these types of situations, `operator ==' can't be used
     * because using it may invoke arbitrary code, while `is' only
     * performs a shallow check that never fails, or invokes other code. */
    if (asm_putddi(ast)) goto err;
    if (asm_gisnone()) goto err;
    break;
   }
   if (ast_genasm(ast->ast_action.ast_act1,PUSH_RESULT)) goto err;
   if (PUSH_RESULT && (asm_putddi(ast) || asm_ginstanceof())) goto err;
   break;
  ACTION(AST_FACTION_IN)
   if (ast_can_exchange(ast->ast_action.ast_act0,
                        ast->ast_action.ast_act1)) {
    if (ast_genasm_set(ast->ast_action.ast_act1,ASM_G_FPUSHRES)) goto err;
    if (ast_genasm(ast->ast_action.ast_act0,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ast) || asm_gcontains()) goto err;
   } else {
    if (ast_genasm(ast->ast_action.ast_act0,ASM_G_FPUSHRES)) goto err;
    if (ast_genasm_set(ast->ast_action.ast_act1,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ast) || asm_gswap() || asm_gcontains()) goto err;
   }
   goto pop_unused;
  ACTION(AST_FACTION_MIN)
   if (ast_genasm(ast->ast_action.ast_act0,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast) || asm_greduce_min()) goto err;
   goto pop_unused;
  ACTION(AST_FACTION_MAX)
   if (ast_genasm(ast->ast_action.ast_act0,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast) || asm_greduce_max()) goto err;
   goto pop_unused;
  ACTION(AST_FACTION_SUM)
   if (ast_genasm(ast->ast_action.ast_act0,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast) || asm_greduce_sum()) goto err;
   goto pop_unused;
  ACTION(AST_FACTION_ANY)
   if (ast_genasm(ast->ast_action.ast_act0,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast) || asm_greduce_any()) goto err;
   goto pop_unused;
  ACTION(AST_FACTION_ALL)
   if (ast_genasm(ast->ast_action.ast_act0,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast) || asm_greduce_all()) goto err;
   goto pop_unused;
  ACTION(AST_FACTION_STORE)
   if unlikely(asm_gstore(ast->ast_action.ast_act0,
                          ast->ast_action.ast_act1,
                          ast,PUSH_RESULT))
               goto err;
   break;

  ACTION(AST_FACTION_BOUNDATTR)
   if (ast_genasm(ast->ast_action.ast_act0,ASM_G_FPUSHRES)) goto err;
   if (ast_genasm(ast->ast_action.ast_act1,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast) || asm_gboundattr()) goto err;
   goto pop_unused;

  ACTION(AST_FACTION_CALL_KW)
   /* Call with keyword list. */
   if (asm_gcall_kw_expr(ast->ast_action.ast_act0,
                         ast->ast_action.ast_act1,
                         ast->ast_action.ast_act2,
                         ast,gflags))
       goto err;
   goto done;

  {
   DeeAstObject *expr,*message;
   struct asm_sym *assert_enter;
   struct asm_sym *assert_leave;
   struct asm_sec *old_section;
   uint8_t argc; int32_t deemon_modid;
   uint16_t operator_name;
  ACTION(AST_FACTION_ASSERT)
  ACTION(AST_FACTION_ASSERT_M)
   expr = ast->ast_action.ast_act0;
   if (current_assembler.a_flag&ASM_FNOASSERT) {
    /* Discard the assert-expression and message and emit a constant true. */
    if (PUSH_RESULT) {
     if (ast_genasm(expr,gflags))
         goto err;
    }
    goto done;
   }
   message = NULL;
   if (action_type == (AST_FACTION_ASSERT_M&AST_FACTION_KINDMASK))
       message = ast->ast_action.ast_act1;
   /* Assertions have their own action due to their very special encoding:
    * >> assert foo() == bar(), "This is bad";
    * This generates the following assembly:
    * >>    push  <foo>
    * >>    call  top, #0
    * >>    push  <bar>
    * >>    call  top, #0
    * >>    dup   #1             // Copy <foo>
    * >>    dup   #1             // Copy <bar>
    * >>    cmp   eq top, pop    // Do the comparison
    * >>   [dup]                 // When the result is used
    * >>    jf    .cold.1f
    * >>   [rrot  #3]            // When the result is used
    * >>    pop
    * >>    pop
    * >>2:
    * >>
    * >>.cold.1:
    * >>   [pop]                 // When the result is used
    * >>                         // Stack: foo, bar
    * >>    push  @"This is bad" // Stack: foo, bar, message  --- When no message is given, push `none' instead
    * >>    rrot  #3             // Stack: message, foo, bar
    * >>    push  $__eq__        // Stack: message, foo, bar, id  --- Push the operator id as an integer onto the stack.
    * >>    rrot  #3             // Stack: message, id, foo, bar
    * >>    call  @deemon:@__assert, #4 // Call the assertion handler (implementation-specific)
    * >>   [pop]                 // When the result isn't used.
    * >>    jmp   2b             // This may seem redundant, but without this, peephole breaks.
    * >>                         // Also: this is required to no confuse debuggers, and in the
    * >>                         //       event that the assertion-failure function was overwritten,
    * >>                         //       it may actually return once again!
    */
   if unlikely((assert_enter = asm_newsym()) == NULL) goto err; /* .cold.1: */
   if unlikely((assert_leave = asm_newsym()) == NULL) goto err; /* 2: */
   if (expr->ast_type == AST_OPERATOR &&
       /* NOTE: Don't handle varargs operators. */
     !(expr->ast_operator.ast_exflag&AST_OPERATOR_FVARARGS) &&
       /* NOTE: Don't handle invalid operators. */
      (operator_name = expr->ast_flag,
       operator_name < OPERATOR_INC || operator_name > OPERATOR_INPLACE_POW)) {
    instruction_t op_instr; uint8_t operand_mode;
    /* Figure out how many operands there are while generating code for the operator itself. */
    argc = 1;
    if (ast_genasm(expr->ast_operator.ast_opa,ASM_G_FPUSHRES))
        goto err;
    if (!expr->ast_operator.ast_opb)
         goto emit_instruction;
    ++argc;
    if (ast_genasm(expr->ast_operator.ast_opb,ASM_G_FPUSHRES))
        goto err;
    if (!expr->ast_operator.ast_opc)
         goto emit_instruction;
    ++argc;
    if (ast_genasm(expr->ast_operator.ast_opc,ASM_G_FPUSHRES))
        goto err;
    if (!expr->ast_operator.ast_opd)
         goto emit_instruction;
    ++argc;
    if (ast_genasm(expr->ast_operator.ast_opd,ASM_G_FPUSHRES))
        goto err;
emit_instruction:
    /* Duplicate all operands. */
    if (asm_putddi(ast)) goto err;
    switch (argc) {
    case 4:  if (asm_gdup_n(2)) goto err;
             if (asm_gdup_n(2)) goto err;
             if (asm_gdup_n(2)) goto err;
             if (asm_gdup_n(2)) goto err;
             break;
    case 3:  if (asm_gdup_n(1)) goto err;
             if (asm_gdup_n(1)) goto err;
             if (asm_gdup_n(1)) goto err;
             break;
    case 2:  if (asm_gdup_n(0)) goto err;
             if (asm_gdup_n(0)) goto err;
             break;
    case 1:  if (asm_gdup()) goto err; break;
    default: break;
    }

    if (asm_putddi(expr)) goto err;
    /* With all the operands on-stack, as well as duplicated,
     * it's time to perform the operation that's to-be asserted. */
    if (operator_name > OPERATOR_USERCOUNT ||
       (op_instr = operator_instr_table[operator_name]) == 0 ||
       (operand_mode = operator_opcount_table[operator_name],
       (operand_mode&OPCOUNT_OPCOUNTMASK) != argc)) {
     /* Invoke the operator using a general-purpose instruction. */
     if (asm_goperator(operator_name,argc-1)) goto err;
    } else {
     /* The operator has its own dedicated instruction, which we can use. */
     if (asm_put(op_instr)) goto err;
     asm_subsp(argc);
     /* STACK: a, [b, [c, [d]]], [check_cond] (remember the duplicates we created above) */
     /* Must unify the instruction behavior by fixing the
      * stack (`check_cond' must always be present). */
     switch (operand_mode&OPCOUNT_RESULTMASK) {
     case OPCOUNT_PUSHFIRST:
      if (argc > 1 ? asm_gdup_n(argc-2) : asm_gdup()) goto err; /* `dup #argc-1' */
      break;
     case OPCOUNT_PUSHSECOND:
      if (argc > 2 ? asm_gdup_n(argc-3) : asm_gdup()) goto err; /* `dup #argc-2' */
      break;
     case OPCOUNT_PUSHTHIRD:
      if (argc > 3 ? asm_gdup_n(argc-4) : asm_gdup()) goto err; /* `dup #argc-3' */
      break;
     case OPCOUNT_PUSHFOURTH:
      ASSERT(!(argc > 4));
      if (/*argc > 4 ? asm_gdup_n(argc-5) : */asm_gdup()) goto err; /* `dup #argc-4' */
      break;
     case OPCOUNT_POPPUSHNONE:
      asm_incsp();
      if (asm_gpop()) goto err;
      ATTR_FALLTHROUGH
     case OPCOUNT_PUSHNONE:
      if (asm_gpush_none()) goto err; /* Kind-of pointless, but still a case that can happen... */
      break;
     default: asm_incsp(); /* The instruction leaves behind the result. */
     }
    }
    if (asm_putddi(ast)) goto err;
    /* Duplicate `condition' to-be re-used as result of the assert expression. */
    if (PUSH_RESULT && asm_gdup()) goto err;
    /* STACK: a, [b, [c, [d]]] condition [condition] */
#define assert_cleanup  assert_enter
    old_section = current_assembler.a_curr;
    if (old_section == &current_assembler.a_sect[SECTION_COLD]) {
     if (asm_gjmp(ASM_JT,assert_cleanup)) goto err;
     asm_decsp(); /* Adjust for `ASM_JT' */
    } else {
     if (asm_gjmp(ASM_JF,assert_enter)) goto err;
     asm_decsp(); /* Adjust for `ASM_JF' */
     current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
     if (asm_putddi(ast)) goto err;
     asm_defsym(assert_enter);
    }
    /* STACK: a, [b, [c, [d]]], [condition] */
    if (PUSH_RESULT && asm_gpop()) goto err; /* Pop the duplicated `condition' */
    /* STACK: a, [b, [c, [d]]] */
    if (message ? (ast_genasm(message,ASM_G_FPUSHRES) || asm_putddi(ast))
                :  asm_gpush_none())
        goto err;
    /* STACK: a, [b, [c, [d]]], message */
    if (asm_grrot(argc+1)) goto err;
    /* STACK: message, a, [b, [c, [d]]] */
    if (asm_gpush_u16(operator_name)) goto err; /* Push the instruction id */
    /* STACK: message, a, [b, [c, [d]]], operator_name */
    if (asm_grrot(argc+1)) goto err;
    /* STACK: message, operator_name, a, [b, [c, [d]]] */
    /* Add `deemon' to the import list. */
    deemon_modid = asm_newmodule(get_deemon_module());
    if unlikely(deemon_modid < 0) goto err;
    /* Generate the call to the builtin assertion function. */
    if (asm_gcall_extern((uint16_t)deemon_modid,id___assert,argc+2)) goto err;
    /* Pop the result when it isn't being used. */
    if (!PUSH_RESULT && asm_gpop()) goto err;
    /* Jump back to regular code. */
    if (asm_gjmp(ASM_JMP,assert_leave)) goto err;
    current_assembler.a_curr = old_section;
    /* Generate the stack-cleanup for when the assertion didn't fail. */
    asm_addsp(argc);
    if (old_section == &current_assembler.a_sect[SECTION_COLD])
        asm_defsym(assert_cleanup);
    else if (asm_putddi(ast)) goto err;
    /* STACK: a, [b, [c, [d]]] [condition] */
    if (PUSH_RESULT && asm_grrot(argc+1)) goto err;
    /* STACK: [condition] a, [b, [c, [d]]] */
    if (asm_gadjstack(-(int16_t)argc)) goto err;
    /* STACK: [condition] */
    asm_defsym(assert_leave);
    goto done;
#undef assert_cleanup
   } 

   if (expr->ast_type == AST_ACTION &&
       expr->ast_flag == AST_FACTION_IN) {
    /* Special case: in-expressions. */
    operator_name = OPERATOR_CONTAINS;
    argc          = 2;
    if (ast_genasm(expr->ast_action.ast_act0,ASM_G_FPUSHRES)) goto err;
    if (ast_genasm(expr->ast_action.ast_act1,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ast)) goto err;
    if (asm_gswap()) goto err;
    goto emit_instruction;
   }
#if 0
   if (expr->ast_type == AST_ACTION &&
       expr->ast_flag == AST_FACTION_IS) {
    /* TODO: Special case: is-expressions. */
    goto emit_instruction;
   }
   if (expr->ast_type == AST_ACTION &&
      (expr->ast_flag == AST_FACTION_SAMEOBJ ||
       expr->ast_flag == AST_FACTION_DIFFOBJ)) {
    /* TODO: Special case: same/diff-object expressions. */
    goto emit_instruction;
   }
#endif
   /* Fallback: Do not include extended information in the failure.
    * In this kind of message, information about operands is omit. */
   if (ast_genasm(expr,ASM_G_FPUSHRES)) goto err;
   if (asm_putddi(ast)) goto err;
   if (PUSH_RESULT && asm_gdup()) goto err;
   old_section = current_assembler.a_curr;
   if (old_section == &current_assembler.a_sect[SECTION_COLD]) {
    if (asm_gjmp(ASM_JT,assert_leave)) goto err;
    asm_decsp(); /* Adjust for `ASM_JT' / `ASM_JF' */
   } else {
    if (asm_gjmp(ASM_JF,assert_enter)) goto err;
    asm_decsp(); /* Adjust for `ASM_JT' / `ASM_JF' */
    current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
    if (asm_putddi(ast)) goto err;
   }
   asm_defsym(assert_enter);
   /* Now we're inside the assertion handler. */
   if (PUSH_RESULT && asm_gpop()) goto err; /* The asserted expression. */
   argc = 0;
   /* Generate code for the assertion message. */
   if (message) {
    if (ast_genasm(message,ASM_G_FPUSHRES)) goto err;
    if (asm_putddi(ast)) goto err;
    ++argc;
   }
   /* Add `deemon' to the import list. */
   deemon_modid = asm_newmodule(get_deemon_module());
   if unlikely(deemon_modid < 0) goto err;
   /* Generate the call to the builtin assertion function. */
   if (asm_gcall_extern((uint16_t)deemon_modid,id___assert,argc)) goto err;
   /* Pop the result when it isn't being used. */
   if (!PUSH_RESULT && asm_gpop()) goto err;
   /* Jump back to regular code if we went into the cold section. */
   if (old_section != &current_assembler.a_sect[SECTION_COLD] &&
       asm_gjmp(ASM_JMP,assert_leave)) goto err;
   current_assembler.a_curr = old_section;
   asm_defsym(assert_leave);
  } break;

  ACTION(AST_FACTION_SAMEOBJ)
   if (ast_genasm(ast->ast_action.ast_act0,PUSH_RESULT)) goto err;
   if (ast_genasm(ast->ast_action.ast_act1,PUSH_RESULT)) goto err;
   if (PUSH_RESULT && (asm_putddi(ast) || asm_gsameobj())) goto err;
   goto pop_unused;
  ACTION(AST_FACTION_DIFFOBJ)
   if (ast_genasm(ast->ast_action.ast_act0,PUSH_RESULT)) goto err;
   if (ast_genasm(ast->ast_action.ast_act1,PUSH_RESULT)) goto err;
   if (PUSH_RESULT && (asm_putddi(ast) || asm_gdiffobj())) goto err;
   goto pop_unused;

#undef ACTION
  default:
   ASSERTF(0,"Invalid action type: %x",(unsigned int)action_type);
  }
 } break;

 {
  uint8_t genflags,opcount;
  uint16_t class_addr;
  struct class_member *iter,*end;
  struct symbol *super_sym;
 case AST_CLASS:
  /* Figure out the flags accompanying the opcode. */
  genflags = ast->ast_flag & 0xf,opcount = 0;
  if (ast->ast_class.ast_base) genflags |= CLASSGEN_FHASBASE,++opcount;
  if (ast->ast_class.ast_name) genflags |= CLASSGEN_FHASNAME,++opcount;
  if (ast->ast_class.ast_imem) genflags |= CLASSGEN_FHASIMEM,++opcount;
  if (ast->ast_class.ast_cmem) genflags |= CLASSGEN_FHASCMEM,++opcount;
  super_sym = ast->ast_class.ast_supersym;
  if (super_sym) {
   SYMBOL_INPLACE_UNWIND_ALIAS(super_sym);
   if (!super_sym->s_nread)
        super_sym = NULL;
  }

  /* Now to actually create the class. */
  if ((!ast->ast_class.ast_name ||
       (ast->ast_class.ast_name->ast_type == AST_CONSTEXPR &&
        DeeString_CheckExact(ast->ast_class.ast_name->ast_constexpr))) &&
      (!ast->ast_class.ast_imem ||
       (ast->ast_class.ast_imem->ast_type == AST_CONSTEXPR &&
        DeeMemberTable_CheckExact(ast->ast_class.ast_imem->ast_constexpr))) &&
      (!ast->ast_class.ast_cmem ||
       (ast->ast_class.ast_cmem->ast_type == AST_CONSTEXPR &&
        DeeMemberTable_CheckExact(ast->ast_class.ast_cmem->ast_constexpr)))) {
   /* Special optimizations using operator-optimized instructions. */
   int32_t name_cid,imem_cid,cmem_cid; instruction_t *text;
   DeeAstObject *base = ast->ast_class.ast_base;
   /* Allocate the constant for operands.
    * NOTE: Even when we can't optimize the base expression, the assembly generators
    *       used when the operands are pushed individually will automatically 
    *       re-use indices that we've already allocated here.
    */
   if unlikely((name_cid = (ast->ast_class.ast_name ?
                            asm_newconst(ast->ast_class.ast_name->ast_constexpr) :
                            0)) < 0 ||
               (imem_cid = (ast->ast_class.ast_imem ?
                            asm_newconst(ast->ast_class.ast_imem->ast_constexpr) :
                            0)) < 0 ||
               (cmem_cid = (ast->ast_class.ast_cmem ?
                            asm_newconst(ast->ast_class.ast_cmem->ast_constexpr) :
                            0)) < 0)
       goto err;
   /* The optimized class instructions can only be used
    * when all constant operands have an index that can
    * fit into the 8-bit range. */
   if ((uint16_t)name_cid > UINT8_MAX) goto class_fallback;
   if ((uint16_t)imem_cid > UINT8_MAX) goto class_fallback;
   if ((uint16_t)cmem_cid > UINT8_MAX) goto class_fallback;
   if (!base) {
    /* Without a base type defined, encode the operation using `ASM_CLASS_C' */
    if unlikely((text = asm_alloc(6)) == NULL) goto err;
    *           (text + 0) = ASM_CLASS_C;
    *(uint8_t *)(text + 1) = genflags;
    *(uint8_t *)(text + 2) = 0;
    *(uint8_t *)(text + 3) = (uint8_t)name_cid;
    *(uint8_t *)(text + 4) = (uint8_t)imem_cid;
    *(uint8_t *)(text + 5) = (uint8_t)cmem_cid;
    goto got_class_incsp;
   }
   if (base->ast_type == AST_CONSTEXPR &&
       asm_allowconst(base->ast_constexpr)) {
    int32_t base_cid;
    base_cid = asm_newconst(base->ast_constexpr);
    if unlikely(base_cid < 0) goto err;
    if (base_cid > UINT8_MAX) goto class_fallback;
    if (super_sym) {
     /* Make sure to save the base expression in the super-symbol. */
     if (asm_gpush_const8((uint8_t)base_cid)) goto err;
     if (asm_gpop_symbol(super_sym,ast)) goto err;
    }
    /* Encode a constant expression base-type. */
    if unlikely((text = asm_alloc(6)) == NULL) goto err;
    *           (text + 0) = ASM_CLASS_C;
    *(uint8_t *)(text + 1) = genflags;
    *(uint8_t *)(text + 2) = (uint8_t)base_cid;
    *(uint8_t *)(text + 3) = (uint8_t)name_cid;
    *(uint8_t *)(text + 4) = (uint8_t)imem_cid;
    *(uint8_t *)(text + 5) = (uint8_t)cmem_cid;
    goto got_class_incsp;
   }
   if (base->ast_type == AST_SYM) {
    struct symbol *base_sym = base->ast_sym;
    SYMBOL_INPLACE_UNWIND_ALIAS(base_sym);
    if (base_sym->s_type == SYMBOL_TYPE_GLOBAL ||
       (base_sym->s_type == SYMBOL_TYPE_LOCAL &&
       !SYMBOL_MUST_REFERENCE_TYPEMAY(base_sym))) {
     int32_t varid; bool is_global;
     is_global = base_sym->s_type == SYMBOL_TYPE_GLOBAL;
     varid = is_global ? asm_gsymid_for_read(base_sym,base)
                       : asm_lsymid_for_read(base_sym,base);
     if unlikely(varid < 0) goto err;
     if (varid > UINT8_MAX) goto class_fallback;
     if (super_sym) {
      /* Make sure to save the base expression in the super-symbol. */
      if (is_global ? asm_gpush_global8((uint8_t)varid)
                    : asm_gpush_local8((uint8_t)varid)) goto err;
      if (asm_gpop_symbol(super_sym,ast)) goto err;
     }
     /* Encode a constant expression base-type. */
     if unlikely((text = asm_alloc(6)) == NULL) goto err;
     *           (text + 0) = is_global ? ASM_CLASS_CBG : ASM_CLASS_CBL;
     *(uint8_t *)(text + 1) = genflags;
     *(uint8_t *)(text + 2) = (uint8_t)varid;
     *(uint8_t *)(text + 3) = (uint8_t)name_cid;
     *(uint8_t *)(text + 4) = (uint8_t)imem_cid;
     *(uint8_t *)(text + 5) = (uint8_t)cmem_cid;
     goto got_class_incsp;
    }
   }
  }
class_fallback:
  if (ast->ast_class.ast_base) {
   if (ast_genasm(ast->ast_class.ast_base,ASM_G_FPUSHRES))
       goto err;
   if (super_sym) {
    /* Write the class base to the super-symbol. */
    SYMBOL_INPLACE_UNWIND_ALIAS(super_sym);
    if (super_sym->s_type == SYMBOL_TYPE_STACK &&
      !(super_sym->s_flag & SYMBOL_FALLOC) &&
       !SYMBOL_MUST_REFERENCE_TYPEMAY(super_sym)) {
     super_sym->s_symid = current_assembler.a_stackcur-1;
     super_sym->s_flag |= SYMBOL_FALLOC;
     if (asm_gdup()) goto err;
    } else {
     if (asm_gdup()) goto err;
     if (asm_gpop_symbol(super_sym,ast)) goto err;
    }
   }
  }
  if (ast->ast_class.ast_name && ast_genasm(ast->ast_class.ast_name,ASM_G_FPUSHRES)) goto err;
  if (ast->ast_class.ast_imem && ast_genasm(ast->ast_class.ast_imem,ASM_G_FPUSHRES)) goto err;
  if (ast->ast_class.ast_cmem && ast_genasm(ast->ast_class.ast_cmem,ASM_G_FPUSHRES)) goto err;

  /* Create a regular, old class. */
  if (asm_putddi(ast)) goto err;
  if (asm_putimm8(ASM_CLASS,genflags)) goto err;
  asm_subsp(opcount);
got_class_incsp:
  asm_incsp();

  if (ast->ast_class.ast_classsym) {
   /* Write the class itself to the class-symbol. */
   if (asm_gdup()) goto err;
   if (asm_gpop_symbol(ast->ast_class.ast_classsym,ast)) goto err;
  }
  class_addr = current_assembler.a_stackcur-1;
  /* Go through all class member descriptors and generate their code. */
  end = (iter = ast->ast_class.ast_memberv)+
                ast->ast_class.ast_memberc;
  for (; iter != end; ++iter) {
   PRIVATE instruction_t member_modes[2] = {
       /* [CLASS_MEMBER_MEMBER]   = */ASM_DEFMEMBER,
       /* [CLASS_MEMBER_OPERATOR] = */ASM_DEFOP
   };
   uint16_t member_id = iter->cm_index;
   instruction_t def_instr;
   ASSERT(iter->cm_type < COMPILER_LENOF(member_modes));
   def_instr = member_modes[iter->cm_type];
   /* Compile the expression (In the event of it using the class
    * type, it will be able to access it as a stack variable). */
   if (ast_genasm(iter->cm_ast,ASM_G_FPUSHRES))
       goto err;
   if (asm_putddi(ast)) goto err;
   if (class_addr == current_assembler.a_stackcur-2) {
    /* Simple (and most likely) case:
     * >> ..., class, member-value */
    if (member_id > UINT8_MAX) {
     if (asm_put(ASM_EXTENDED1)) goto err;
     if (asm_putimm16(def_instr,member_id)) goto err;
    } else {
     if (asm_putimm8(def_instr,(uint8_t)member_id)) goto err;
    }
    asm_ddicsp(); /* All member-define instructions behave as `-2,+1' */
   } else {
    uint16_t displacement;
    ASSERT(current_assembler.a_flag&ASM_FSTACKDISP);
    ASSERT(class_addr < current_assembler.a_stackcur-2);
    /* Complicated case (can happen in displacement-mode):
     * >> ..., class, ..., member-value
     * Here, we must generate code to do this:
     * >> dup  #sizeof(1+...)   // ..., class, ..., member-value, class
     * >> swap                  // ..., class, ..., class, member-value
     * >> defop / defmember     // ..., class, ..., class
     * >> pop                   // ..., class, ...
     */
    displacement = (current_assembler.a_stackcur-2)-class_addr;
    if (asm_gdup_n(displacement)) goto err;
    if (asm_gswap()) goto err;
    if (member_id > UINT8_MAX) {
     if (asm_put(ASM_EXTENDED1)) goto err;
     if (asm_putimm16(def_instr,member_id)) goto err;
    } else {
     if (asm_putimm8(def_instr,(uint8_t)member_id)) goto err;
    }
    asm_ddicsp(); /* All member-define instructions behave as `-2,+1' */
    if (asm_gpop()) goto err;
   }
  }
  goto pop_unused;
 } break;

 {
  struct text_label *label;
  struct asm_sym *sym;
 case AST_LABEL:
  label = ast->ast_label.ast_label;
  ASSERT(label);
  sym = label->tl_asym;
  if (!sym) {
   sym = asm_newsym();
   if unlikely(!sym) goto err;
   label->tl_asym = sym;
  }
  if unlikely(ASM_SYM_DEFINED(sym)) {
   /* Warn if the label had already been defined. */
   if (WARNAST(ast,W_ASM_LABEL_ALREADY_DEFINED,
               ast->ast_flag&AST_FLABEL_CASE ?
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
  label = ast->ast_goto.ast_label;
  ASSERT(label);
  sym = label->tl_asym;
  if (!sym) {
   sym = asm_newsym();
   if unlikely(!sym) goto err;
   label->tl_asym = sym;
  }
  old_stack = current_assembler.a_stackcur;
  /* Adjust the stack and jump to the proper symbol. */
  if (asm_putddi(ast)) goto err;
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
  if unlikely(ast_genasm_switch(ast)) goto err;
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

#ifndef CONFIG_LANGUAGE_NO_ASM
 case AST_ASSEMBLY:
  if unlikely(ast_genasm_userasm(ast))
     goto err;
  goto done_push_none;
#endif /* !CONFIG_LANGUAGE_NO_ASM */

 default:
  ASSERTF(0,"Invalid AST type: %x",(unsigned int)ast->ast_type);
  break;
 }
done:
 ASM_POP_SCOPE(PUSH_RESULT ? 1 : 0,err);
done_noalign:
 current_assembler.a_error = old_loc;
 return 0;
err:
 current_assembler.a_error = old_loc;
 return -1;
#undef PUSH_RESULT
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_GENASM_C */
