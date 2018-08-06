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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_INFO_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_INFO_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/file.h>
#include <deemon/tuple.h>
#include <deemon/arg.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/super.h>
#include <deemon/string.h>

#include "runtime_error.h"

DECL_BEGIN

typedef DeeTypeObject Type;


INTERN struct opinfo basic_opinfo[OPERATOR_USERCOUNT] = {
    /* [OPERATOR_CONSTRUCTOR]  = */{ OPTYPE_SPECIAL,                               OPCLASS_TYPE, 0, offsetof(Type,tp_init.tp_alloc.tp_any_ctor),  "this",         "constructor", "tp_any_ctor" },
    /* [OPERATOR_COPY]         = */{ OPTYPE_SPECIAL,                               OPCLASS_TYPE, 0, offsetof(Type,tp_init.tp_alloc.tp_copy_ctor), "copy",         "copy",        "tp_copy_ctor" },
    /* [OPERATOR_DEEPCOPY]     = */{ OPTYPE_SPECIAL,                               OPCLASS_TYPE, 0, offsetof(Type,tp_init.tp_alloc.tp_deep_ctor), "deepcopy",     "deepcopy",    "tp_deep_ctor" },
    /* [OPERATOR_DESTRUCTOR]   = */{ OPTYPE_RVOID|OPTYPE_UNARY,                    OPCLASS_TYPE, 1, offsetof(Type,tp_init.tp_dtor),               "~this",        "destructor",  "tp_dtor" },
    /* [OPERATOR_ASSIGN]       = */{ OPTYPE_RINT|OPTYPE_BINARY,                    OPCLASS_TYPE, 0, offsetof(Type,tp_init.tp_assign),             "=",            "assign",      "tp_assign" },
    /* [OPERATOR_MOVEASSIGN]   = */{ OPTYPE_RINT|OPTYPE_BINARY,                    OPCLASS_TYPE, 0, offsetof(Type,tp_init.tp_move_assign),        "move =",       "moveassign",  "tp_move_assign" },
    /* [OPERATOR_STR]          = */{ OPTYPE_ROBJECT|OPTYPE_UNARY,                  OPCLASS_TYPE, 0, offsetof(Type,tp_cast.tp_str),                "str",          "str",         "tp_str" },
    /* [OPERATOR_REPR]         = */{ OPTYPE_ROBJECT|OPTYPE_UNARY,                  OPCLASS_TYPE, 0, offsetof(Type,tp_cast.tp_repr),               "repr",         "repr",        "tp_repr" },
    /* [OPERATOR_BOOL]         = */{ OPTYPE_RINT|OPTYPE_UNARY,                     OPCLASS_TYPE, 0, offsetof(Type,tp_cast.tp_bool),               "bool",         "bool",        "tp_bool" },
    /* [OPERATOR_CALL]         = */{ OPTYPE_ROBJECT|OPTYPE_BINARY|OPTYPE_VARIABLE, OPCLASS_TYPE, 0, offsetof(Type,tp_call),                       "()",           "call",        "tp_call" },
    /* [OPERATOR_ITERNEXT]     = */{ OPTYPE_ROBJECT|OPTYPE_UNARY,                  OPCLASS_TYPE, 0, offsetof(Type,tp_iter_next),                  "next",         "next",        "tp_iter_next" },
    /* [OPERATOR_INT]          = */{ OPTYPE_SPECIAL,                               OPCLASS_MATH, 0, offsetof(struct type_math,tp_int),            "int",          "int",         "tp_int" },
    /* [OPERATOR_FLOAT]        = */{ OPTYPE_DOUBLE,                                OPCLASS_MATH, 0, offsetof(struct type_math,tp_double),         "float",        "float",       "tp_double" },
    /* [OPERATOR_INV]          = */{ OPTYPE_ROBJECT|OPTYPE_UNARY,                  OPCLASS_MATH, 0, offsetof(struct type_math,tp_inv),            "~",            "inv",         "tp_inv" },
    /* [OPERATOR_POS]          = */{ OPTYPE_ROBJECT|OPTYPE_UNARY,                  OPCLASS_MATH, 0, offsetof(struct type_math,tp_pos),            "+",            "pos",         "tp_pos" },
    /* [OPERATOR_NEG]          = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_neg),            "-",            "neg",         "tp_neg" },
    /* [OPERATOR_ADD]          = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_add),            "+",            "add",         "tp_add" },
    /* [OPERATOR_SUB]          = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_sub),            "-",            "sub",         "tp_sub" },
    /* [OPERATOR_MUL]          = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_mul),            "*",            "mul",         "tp_mul" },
    /* [OPERATOR_DIV]          = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_div),            "/",            "div",         "tp_div" },
    /* [OPERATOR_MOD]          = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_mod),            "%",            "mod",         "tp_mod" },
    /* [OPERATOR_SHL]          = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_shl),            "<<",           "shl",         "tp_shl" },
    /* [OPERATOR_SHR]          = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_shr),            ">>",           "shr",         "tp_shr" },
    /* [OPERATOR_AND]          = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_and),            "&",            "and",         "tp_and" },
    /* [OPERATOR_OR]           = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_or),             "|",            "or",          "tp_or" },
    /* [OPERATOR_XOR]          = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_xor),            "^",            "xor",         "tp_xor" },
    /* [OPERATOR_POW]          = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_pow),            "**",           "pow",         "tp_pow" },
    /* [OPERATOR_INC]          = */{ OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_UNARY,      OPCLASS_MATH, 0, offsetof(struct type_math,tp_inc),            "++",           "inc",         "tp_inc" },
    /* [OPERATOR_DEC]          = */{ OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_UNARY,      OPCLASS_MATH, 0, offsetof(struct type_math,tp_dec),            ",,",           "dec",         "tp_dec" },
    /* [OPERATOR_INPLACE_ADD]  = */{ OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_add),    "+=",           "iadd",        "tp_inplace_add" },
    /* [OPERATOR_INPLACE_SUB]  = */{ OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_sub),    ",=",           "isub",        "tp_inplace_sub" },
    /* [OPERATOR_INPLACE_MUL]  = */{ OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_mul),    "*=",           "imul",        "tp_inplace_mul" },
    /* [OPERATOR_INPLACE_DIV]  = */{ OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_div),    "/=",           "idiv",        "tp_inplace_div" },
    /* [OPERATOR_INPLACE_MOD]  = */{ OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_mod),    "%=",           "imod",        "tp_inplace_mod" },
    /* [OPERATOR_INPLACE_SHL]  = */{ OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_shl),    "<<=",          "ishl",        "tp_inplace_shl" },
    /* [OPERATOR_INPLACE_SHR]  = */{ OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_shr),    ">>=",          "ishr",        "tp_inplace_shr" },
    /* [OPERATOR_INPLACE_AND]  = */{ OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_and),    "&=",           "iand",        "tp_inplace_and" },
    /* [OPERATOR_INPLACE_OR]   = */{ OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_or),     "|=",           "ior",         "tp_inplace_or" },
    /* [OPERATOR_INPLACE_XOR]  = */{ OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_xor),    "^=",           "ixor",        "tp_inplace_xor" },
    /* [OPERATOR_INPLACE_POW]  = */{ OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_pow),    "**=",          "ipow",        "tp_inplace_pow" },
    /* [OPERATOR_HASH]         = */{ OPTYPE_RUINTPTR|OPTYPE_UNARY,                 OPCLASS_CMP,  0, offsetof(struct type_cmp,tp_hash),            "hash",         "hash",        "tp_hash" },
    /* [OPERATOR_EQ]           = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_CMP,  0, offsetof(struct type_cmp,tp_eq),              "==",           "eq",          "tp_eq" },
    /* [OPERATOR_NE]           = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_CMP,  0, offsetof(struct type_cmp,tp_ne),              "!=",           "ne",          "tp_ne" },
    /* [OPERATOR_LO]           = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_CMP,  0, offsetof(struct type_cmp,tp_lo),              "<",            "lo",          "tp_lo" },
    /* [OPERATOR_LE]           = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_CMP,  0, offsetof(struct type_cmp,tp_le),              "<=",           "le",          "tp_le" },
    /* [OPERATOR_GR]           = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_CMP,  0, offsetof(struct type_cmp,tp_gr),              ">",            "gr",          "tp_gr" },
    /* [OPERATOR_GE]           = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_CMP,  0, offsetof(struct type_cmp,tp_ge),              ">=",           "ge",          "tp_ge" },
    /* [OPERATOR_ITERSELF]     = */{ OPTYPE_ROBJECT|OPTYPE_UNARY,                  OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_iter_self),       "iter",         "iter",        "tp_iter_self" },
    /* [OPERATOR_SIZE]         = */{ OPTYPE_ROBJECT|OPTYPE_UNARY,                  OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_size),            "#",            "size",        "tp_size" },
    /* [OPERATOR_CONTAINS]     = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_contains),        "contains",     "contains",    "tp_contains" },
    /* [OPERATOR_GETITEM]      = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_get),             "[]",           "getitem",     "tp_get" },
    /* [OPERATOR_DELITEM]      = */{ OPTYPE_RINT|OPTYPE_BINARY,                    OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_del),             "del []",       "delitem",     "tp_del" },
    /* [OPERATOR_SETITEM]      = */{ OPTYPE_RINT|OPTYPE_TRINARY,                   OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_set),             "[] =",         "setitem",     "tp_set" },
    /* [OPERATOR_GETRANGE]     = */{ OPTYPE_ROBJECT|OPTYPE_TRINARY,                OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_range_get),       "[:]",          "getrange",    "tp_range_get" },
    /* [OPERATOR_DELRANGE]     = */{ OPTYPE_RINT|OPTYPE_TRINARY,                   OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_range_del),       "del [:]",      "delrange",    "tp_range_del" },
    /* [OPERATOR_SETRANGE]     = */{ OPTYPE_RINT|OPTYPE_QUAD,                      OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_range_set),       "[:] =",        "setrange",    "tp_range_set" },
    /* [OPERATOR_GETATTR]      = */{ OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_ATTR, 0, offsetof(struct type_attr,tp_getattr),        ".",            "getattr",     "tp_getattr" },
    /* [OPERATOR_DELATTR]      = */{ OPTYPE_RINT|OPTYPE_BINARY,                    OPCLASS_ATTR, 0, offsetof(struct type_attr,tp_delattr),        "del .",        "delattr",     "tp_delattr" },
    /* [OPERATOR_SETATTR]      = */{ OPTYPE_RINT|OPTYPE_TRINARY,                   OPCLASS_ATTR, 0, offsetof(struct type_attr,tp_setattr),        ". =",          "setattr",     "tp_setattr" },
    /* [OPERATOR_ENUMATTR]     = */{ OPTYPE_ENUMATTR,                              OPCLASS_ATTR, 0, offsetof(struct type_attr,tp_enumattr),       "...",          "enumattr",    "tp_enumattr" },
    /* [OPERATOR_ENTER]        = */{ OPTYPE_RINT|OPTYPE_UNARY,                     OPCLASS_WITH, 0, offsetof(struct type_with,tp_enter),          "enter",        "enter",       "tp_enter" },
    /* [OPERATOR_LEAVE]        = */{ OPTYPE_RINT|OPTYPE_UNARY,                     OPCLASS_WITH, 0, offsetof(struct type_with,tp_leave),          "leave",        "leave",       "tp_leave" }
};
PRIVATE struct opinfo private_opinfo[(OPERATOR_PRIVMAX-OPERATOR_PRIVMIN)+1] = {
    /* [OPERATOR_VISIT  - OPERATOR_PRIVMIN] = */{ OPTYPE_VISIT,              OPCLASS_TYPE,   1, offsetof(Type,tp_visit),               "", "", "tp_visit" },
    /* [OPERATOR_CLEAR  - OPERATOR_PRIVMIN] = */{ OPTYPE_RVOID|OPTYPE_UNARY, OPCLASS_GC,     1, offsetof(struct type_gc,tp_clear),     "", "", "tp_clear" },
    /* [OPERATOR_PCLEAR - OPERATOR_PRIVMIN] = */{ OPTYPE_PCLEAR,             OPCLASS_GC,     1, offsetof(struct type_gc,tp_pclear),    "", "", "tp_pclear" },
    /* [OPERATOR_GETBUF - OPERATOR_PRIVMIN] = */{ OPTYPE_GETBUF,             OPCLASS_BUFFER, 1, offsetof(struct type_buffer,tp_getbuf),"", "", "tp_getbuf" }
};

INTERN struct opinfo file_opinfo[FILE_OPERATOR_COUNT] = {
    /* [FILE_OPERATOR_READ   - OPERATOR_EXTENDED(0)] = */{ OPTYPE_READWRITE,         OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_read),   "read",   "read",   "ft_read",   },
    /* [FILE_OPERATOR_WRITE  - OPERATOR_EXTENDED(0)] = */{ OPTYPE_READWRITE,         OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_write),  "write",  "write",  "ft_write",  },
    /* [FILE_OPERATOR_SEEK   - OPERATOR_EXTENDED(0)] = */{ OPTYPE_SEEK,              OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_seek),   "seek",   "seek",   "ft_seek",   },
    /* [FILE_OPERATOR_SYNC   - OPERATOR_EXTENDED(0)] = */{ OPTYPE_RINT|OPTYPE_UNARY, OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_sync),   "sync",   "sync",   "ft_sync",   },
    /* [FILE_OPERATOR_TRUNC  - OPERATOR_EXTENDED(0)] = */{ OPTYPE_TRUNC,             OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_trunc),  "trunc",  "trunc",  "ft_trunc",  },
    /* [FILE_OPERATOR_CLOSE  - OPERATOR_EXTENDED(0)] = */{ OPTYPE_RINT|OPTYPE_UNARY, OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_close),  "close",  "close",  "ft_close",  },
    /* [FILE_OPERATOR_PREAD  - OPERATOR_EXTENDED(0)] = */{ OPTYPE_PREADWRITE,        OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_pread),  "pread",  "pread",  "ft_pread",  },
    /* [FILE_OPERATOR_PWRITE - OPERATOR_EXTENDED(0)] = */{ OPTYPE_PREADWRITE,        OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_pwrite), "pwrite", "pwrite", "ft_pwrite", },
    /* [FILE_OPERATOR_GETC   - OPERATOR_EXTENDED(0)] = */{ OPTYPE_RINT|OPTYPE_UNARY, OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_getc),   "getc",   "getc",   "ft_getc",   },
    /* [FILE_OPERATOR_UNGETC - OPERATOR_EXTENDED(0)] = */{ OPTYPE_UNGETPUT,          OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_ungetc), "ungetc", "ungetc", "ft_ungetc", },
    /* [FILE_OPERATOR_PUTC   - OPERATOR_EXTENDED(0)] = */{ OPTYPE_UNGETPUT,          OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_putc),   "putc",   "putc",   "ft_putc",   }
};

PUBLIC struct opinfo *DCALL
Dee_OperatorInfo(DeeTypeObject *typetype,
                 uint16_t id) {
 ASSERT_OBJECT_TYPE_OPT(typetype,&DeeType_Type);
 if (id < OPERATOR_USERCOUNT)
     return &basic_opinfo[id];
 if (id >= OPERATOR_PRIVMIN && id <= OPERATOR_PRIVMAX)
     return &private_opinfo[id - OPERATOR_PRIVMIN];
 if (typetype && id >= OPERATOR_EXTENDED(0)) {
  /* Extended operator codes. */
  ASSERT(DeeType_IsTypeType(typetype));
  if (typetype == &DeeFileType_Type &&
      id < OPERATOR_EXTENDED(FILE_OPERATOR_COUNT))
      return &file_opinfo[id - OPERATOR_EXTENDED(0)];
 }
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
invoke_operator(DeeObject *__restrict self,
                DeeObject **pself, uint16_t name,
                size_t argc, DeeObject **__restrict argv) {
 ASSERT_OBJECT(self);
 /* Fast-pass for known operators. */
 switch (name) {

 case OPERATOR_CONSTRUCTOR:
  /* Special wrapper: invoke the construction operator. */
  if (DeeObject_AssertType(self,&DeeType_Type))
      return NULL;
  return DeeObject_New((DeeTypeObject *)self,argc,argv);

 case OPERATOR_COPY:
  /* Special wrapper: invoke the copy constructor. */
  if (DeeArg_Unpack(argc,argv,":__copy__"))
      return NULL;
  return DeeObject_Copy(self);

 case OPERATOR_DEEPCOPY:
  /* Special wrapper: invoke the deepcopy constructor. */
  if (DeeArg_Unpack(argc,argv,":__deepcopy__"))
      return NULL;
  return DeeObject_DeepCopy(self);

 {
  DeeObject *other;
 case OPERATOR_ASSIGN:
  if (DeeArg_Unpack(argc,argv,"o:__assign__",&other))
      return NULL;
  if (DeeObject_Assign(self,other))
      return NULL;
  goto return_self;
 }

 {
  DeeObject *other;
 case OPERATOR_MOVEASSIGN:
  if (DeeArg_Unpack(argc,argv,"o:__moveassign__",&other))
      return NULL;
  if (DeeObject_MoveAssign(self,other))
      return NULL;
  goto return_self;
 }

 case OPERATOR_STR:
  if (DeeArg_Unpack(argc,argv,":__str__"))
      return NULL;
  return DeeObject_Str(self);

 case OPERATOR_REPR:
  if (DeeArg_Unpack(argc,argv,":__repr__"))
      return NULL;
  return DeeObject_Repr(self);

 {
  int result;
 case OPERATOR_BOOL:
  if (DeeArg_Unpack(argc,argv,":__bool__"))
      return NULL;
  result = DeeObject_Bool(self);
  if unlikely(result < 0) return NULL;
  return_bool_(result);
 }

 {
  DREF DeeObject *args;
 case OPERATOR_CALL:
  if (DeeArg_Unpack(argc,argv,"o:__call__",&args) ||
      DeeObject_AssertTypeExact(args,&DeeTuple_Type))
      return NULL;
  return DeeObject_Call(self,
                        DeeTuple_SIZE(args),
                        DeeTuple_ELEM(args));
 } break;

 {
  dhash_t result;
 case OPERATOR_HASH:
  if (DeeArg_Unpack(argc,argv,":__hash__"))
      return NULL;
  result = DeeObject_Hash(self);
  return DeeInt_NewHash(result);
 }

 {
  DREF DeeObject *result;
 case OPERATOR_ITERNEXT:
  if (DeeArg_Unpack(argc,argv,":__next__"))
      return NULL;
  result = DeeObject_IterNext(self);
  if (result == ITER_DONE) {
   DeeError_Throw(&DeeError_StopIteration_instance);
   result = NULL;
  }
  return result;
 }

 case OPERATOR_INT:
  if (DeeArg_Unpack(argc,argv,":__int__"))
      return NULL;
  return DeeObject_Int(self);

 {
  double value;
 case OPERATOR_FLOAT:
  if (DeeArg_Unpack(argc,argv,":__float__"))
      return NULL;
  if (DeeObject_AsDouble(self,&value))
      return NULL;
  return DeeFloat_New(value);
 }

 case OPERATOR_INV:
  if (DeeArg_Unpack(argc,argv,":__inv__"))
      return NULL;
  return DeeObject_Inv(self);
 case OPERATOR_POS:
  if (DeeArg_Unpack(argc,argv,":__pos__"))
      return NULL;
  return DeeObject_Pos(self);
 case OPERATOR_NEG:
  if (DeeArg_Unpack(argc,argv,":__neg__"))
      return NULL;
  return DeeObject_Neg(self);
 {
  DeeObject *other;
 case OPERATOR_ADD:
  if (DeeArg_Unpack(argc,argv,"o:__add__",&other))
      return NULL;
  return DeeObject_Add(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_SUB:
  if (DeeArg_Unpack(argc,argv,"o:__sub__",&other))
      return NULL;
  return DeeObject_Sub(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_MUL:
  if (DeeArg_Unpack(argc,argv,"o:__mul__",&other))
      return NULL;
  return DeeObject_Mul(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_DIV:
  if (DeeArg_Unpack(argc,argv,"o:__div__",&other))
      return NULL;
  return DeeObject_Div(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_MOD:
  if (DeeArg_Unpack(argc,argv,"o:__mod__",&other))
      return NULL;
  return DeeObject_Mod(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_SHL:
  if (DeeArg_Unpack(argc,argv,"o:__shl__",&other))
      return NULL;
  return DeeObject_Shl(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_SHR:
  if (DeeArg_Unpack(argc,argv,"o:__shr__",&other))
      return NULL;
  return DeeObject_Shr(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_AND:
  if (DeeArg_Unpack(argc,argv,"o:__and__",&other))
      return NULL;
  return DeeObject_And(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_OR:
  if (DeeArg_Unpack(argc,argv,"o:__or__",&other))
      return NULL;
  return DeeObject_Or(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_XOR:
  if (DeeArg_Unpack(argc,argv,"o:__xor__",&other))
      return NULL;
  return DeeObject_Xor(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_POW:
  if (DeeArg_Unpack(argc,argv,"o:__pow__",&other))
      return NULL;
  return DeeObject_Pow(self,other);
 }
 case OPERATOR_INC:
  if unlikely(!pself) goto requires_inplace;
  if (DeeArg_Unpack(argc,argv,":__inc__"))
      return NULL;
  if (DeeObject_Inc(pself))
      return NULL;
  goto return_self;
 case OPERATOR_DEC:
  if unlikely(!pself) goto requires_inplace;
  if (DeeArg_Unpack(argc,argv,":__dec__"))
      return NULL;
  if (DeeObject_Dec(pself))
      return NULL;
  goto return_self;
 {
  DeeObject *other;
 case OPERATOR_INPLACE_ADD:
  if unlikely(!pself) goto requires_inplace;
  if (DeeArg_Unpack(argc,argv,"o:__iadd__",&other))
      return NULL;
  if (DeeObject_InplaceAdd(pself,other))
      return NULL;
  goto return_self;
 }
 {
  DeeObject *other;
 case OPERATOR_INPLACE_SUB:
  if unlikely(!pself) goto requires_inplace;
  if (DeeArg_Unpack(argc,argv,"o:__isub__",&other))
      return NULL;
  if (DeeObject_InplaceSub(pself,other))
      return NULL;
  goto return_self;
 }
 {
  DeeObject *other;
 case OPERATOR_INPLACE_MUL:
  if unlikely(!pself) goto requires_inplace;
  if (DeeArg_Unpack(argc,argv,"o:__imul__",&other))
      return NULL;
  if (DeeObject_InplaceMul(pself,other))
      return NULL;
  goto return_self;
 }
 {
  DeeObject *other;
 case OPERATOR_INPLACE_DIV:
  if unlikely(!pself) goto requires_inplace;
  if (DeeArg_Unpack(argc,argv,"o:__idiv__",&other))
      return NULL;
  if (DeeObject_InplaceDiv(pself,other))
      return NULL;
  goto return_self;
 }
 {
  DeeObject *other;
 case OPERATOR_INPLACE_MOD:
  if unlikely(!pself) goto requires_inplace;
  if (DeeArg_Unpack(argc,argv,"o:__imod__",&other))
      return NULL;
  if (DeeObject_InplaceMod(pself,other))
      return NULL;
  goto return_self;
 }
 {
  DeeObject *other;
 case OPERATOR_INPLACE_SHL:
  if unlikely(!pself) goto requires_inplace;
  if (DeeArg_Unpack(argc,argv,"o:__ishl__",&other))
      return NULL;
  if (DeeObject_InplaceShl(pself,other))
      return NULL;
  goto return_self;
 }
 {
  DeeObject *other;
 case OPERATOR_INPLACE_SHR:
  if unlikely(!pself) goto requires_inplace;
  if (DeeArg_Unpack(argc,argv,"o:__ishr__",&other))
      return NULL;
  if (DeeObject_InplaceShr(pself,other))
      return NULL;
  goto return_self;
 }
 {
  DeeObject *other;
 case OPERATOR_INPLACE_AND:
  if unlikely(!pself) goto requires_inplace;
  if (DeeArg_Unpack(argc,argv,"o:__iand__",&other))
      return NULL;
  if (DeeObject_InplaceAnd(pself,other))
      return NULL;
  goto return_self;
 }
 {
  DeeObject *other;
 case OPERATOR_INPLACE_OR:
  if unlikely(!pself) goto requires_inplace;
  if (DeeArg_Unpack(argc,argv,"o:__ior__",&other))
      return NULL;
  if (DeeObject_InplaceOr(pself,other))
      return NULL;
  goto return_self;
 }
 {
  DeeObject *other;
 case OPERATOR_INPLACE_XOR:
  if unlikely(!pself) goto requires_inplace;
  if (DeeArg_Unpack(argc,argv,"o:__ixor__",&other))
      return NULL;
  if (DeeObject_InplaceXor(pself,other))
      return NULL;
  goto return_self;
 }
 {
  DeeObject *other;
 case OPERATOR_INPLACE_POW:
  if unlikely(!pself) goto requires_inplace;
  if (DeeArg_Unpack(argc,argv,"o:__ipow__",&other))
      return NULL;
  if (DeeObject_InplacePow(pself,other))
      return NULL;
  goto return_self;
 }
 {
  DeeObject *other;
 case OPERATOR_EQ:
  if (DeeArg_Unpack(argc,argv,"o:__eq__",&other))
      return NULL;
  return DeeObject_CompareEqObject(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_NE:
  if (DeeArg_Unpack(argc,argv,"o:__ne__",&other))
      return NULL;
  return DeeObject_CompareNeObject(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_LO:
  if (DeeArg_Unpack(argc,argv,"o:__lo__",&other))
      return NULL;
  return DeeObject_CompareLoObject(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_LE:
  if (DeeArg_Unpack(argc,argv,"o:__le__",&other))
      return NULL;
  return DeeObject_CompareLeObject(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_GR:
  if (DeeArg_Unpack(argc,argv,"o:__gr__",&other))
      return NULL;
  return DeeObject_CompareGrObject(self,other);
 }
 {
  DeeObject *other;
 case OPERATOR_GE:
  if (DeeArg_Unpack(argc,argv,"o:__ge__",&other))
      return NULL;
  return DeeObject_CompareGeObject(self,other);
 }
 case OPERATOR_ITERSELF:
  if (DeeArg_Unpack(argc,argv,":__iter__"))
      return NULL;
  return DeeObject_IterSelf(self);
 case OPERATOR_SIZE:
  if (DeeArg_Unpack(argc,argv,":__size__"))
      return NULL;
  return DeeObject_SizeObject(self);
 {
  DeeObject *item;
 case OPERATOR_CONTAINS:
  if (DeeArg_Unpack(argc,argv,"o:__contains__",&item))
      return NULL;
  return DeeObject_ContainsObject(self,item);
 }
 {
  DeeObject *index;
 case OPERATOR_GETITEM:
  if (DeeArg_Unpack(argc,argv,"o:__getitem__",&index))
      return NULL;
  return DeeObject_GetItem(self,index);
 }
 {
  DeeObject *index;
 case OPERATOR_DELITEM:
  if (DeeArg_Unpack(argc,argv,"o:__delitem__",&index))
      return NULL;
  if (DeeObject_DelItem(self,index))
      return NULL;
  goto return_none_;
 }
 {
  DeeObject *index,*value;
 case OPERATOR_SETITEM:
  if (DeeArg_Unpack(argc,argv,"oo:__setitem__",&index,&value))
      return NULL;
  if (DeeObject_SetItem(self,index,value))
      return NULL;
  return_reference_(value);
 }
 {
  DeeObject *begin,*end;
 case OPERATOR_GETRANGE:
  if (DeeArg_Unpack(argc,argv,"oo:__getrange__",&begin,&end))
      return NULL;
  return DeeObject_GetRange(self,begin,end);
 }
 {
  DeeObject *begin,*end;
 case OPERATOR_DELRANGE:
  if (DeeArg_Unpack(argc,argv,"oo:__delrange__",&begin,&end))
      return NULL;
  if (DeeObject_DelRange(self,begin,end))
      return NULL;
  goto return_none_;
 }
 {
  DeeObject *begin,*end,*value;
 case OPERATOR_SETRANGE:
  if (DeeArg_Unpack(argc,argv,"ooo:__setrange__",&begin,&end,&value))
      return NULL;
  if (DeeObject_SetRange(self,begin,end,value))
      return NULL;
  return_reference_(value);
 }
 {
  DeeObject *attr;
 case OPERATOR_GETATTR:
  if (DeeArg_Unpack(argc,argv,"o:__getattr__",&attr) ||
      DeeObject_AssertTypeExact(attr,&DeeString_Type))
      return NULL;
  return DeeObject_GetAttr(self,attr);
 }
 {
  DeeObject *attr;
 case OPERATOR_DELATTR:
  if (DeeArg_Unpack(argc,argv,"o:__delattr__",&attr) ||
      DeeObject_AssertTypeExact(attr,&DeeString_Type))
      return NULL;
  if (DeeObject_DelAttr(self,attr))
      return NULL;
  goto return_none_;
 }
 {
  DeeObject *attr,*value;
 case OPERATOR_SETATTR:
  if (DeeArg_Unpack(argc,argv,"oo:__setattr__",&attr,&value) ||
      DeeObject_AssertTypeExact(attr,&DeeString_Type))
      return NULL;
  if (DeeObject_SetAttr(self,attr,value))
      return NULL;
  return_reference_(value);
 }
 case OPERATOR_ENUMATTR:
  /* TODO */
  break;

 case OPERATOR_ENTER:
  if (DeeArg_Unpack(argc,argv,"__enter__") ||
      DeeObject_Enter(self))
      return NULL;
  goto return_none_;

 case OPERATOR_LEAVE:
  if (DeeArg_Unpack(argc,argv,"__leave__") ||
      DeeObject_Leave(self))
      return NULL;
  goto return_none_;

 default: break;
 }
 {
  /* NOTE: We must query operator information on the
   *       type-type of `self'. Not the regular type! */
  struct opinfo *info;
  DeeTypeObject *typetype;
  typetype = Dee_TYPE(self);
  /* Special handling for unwrapping super objects. */
  if (typetype == &DeeSuper_Type)
      typetype = DeeSuper_TYPE(self);
  typetype = Dee_TYPE(typetype);
  info = Dee_OperatorInfo(typetype,name);
  if (info) {
   if (typetype == &DeeFileType_Type) {
    /* Invoke file operators. */
    switch (name) {

    {
     size_t max_bytes;
    case FILE_OPERATOR_READ:
     max_bytes = (size_t)-1;
     if (DeeArg_Unpack(argc,argv,"|Iu:__read__",&max_bytes))
         return NULL;
     return DeeFile_ReadText(self,max_bytes,false);
    }

    {
     DeeObject *data,*begin,*end;
     dssize_t result;
    case FILE_OPERATOR_WRITE:
     begin = end = NULL;
     if (DeeArg_Unpack(argc,argv,"o|oo:__write__",&data,&begin,&end) ||
         DeeObject_AssertTypeExact(data,&DeeString_Type))
         return NULL;
     if (end) {
      dssize_t ibegin,iend;
      if (DeeObject_AsSSize(begin,&ibegin) ||
          DeeObject_AsSSize(end,&iend))
          return NULL;
      if ((size_t)iend > DeeString_SIZE(data)*sizeof(char))
          iend = (dssize_t)DeeString_SIZE(data)*sizeof(char);
      if ((size_t)ibegin >= (size_t)iend)
          return_reference_(&DeeInt_Zero);
      result = DeeFile_Write(self,DeeString_STR(data)+ibegin,(size_t)(iend-ibegin));
     } else if (begin) {
      size_t max_size;
      if (DeeObject_AsSize(begin,&max_size))
          return NULL;
      if (max_size > DeeString_SIZE(data)*sizeof(char))
          max_size = DeeString_SIZE(data)*sizeof(char);
      result = DeeFile_Write(self,DeeString_STR(data),max_size);
     } else {
      result = DeeFile_Write(self,DeeString_STR(data),
                             DeeString_SIZE(data)*sizeof(char));
     }
     if unlikely(result < 0) return NULL;
     return DeeInt_NewSize((size_t)result);
    }

    {
     doff_t off; int whence;
    case FILE_OPERATOR_SEEK:
     whence = SEEK_SET;
     if (DeeArg_Unpack(argc,argv,"I64d|d:seek",&off,&whence))
         return NULL;
     off = DeeFile_Seek(self,off,whence);
     if unlikely(off < 0) return NULL;
     return DeeInt_NewU64((uint64_t)off);
    } break;

    case FILE_OPERATOR_SYNC:
     if (DeeArg_Unpack(argc,argv,":sync"))
         return NULL;
     if (DeeFile_Sync(self))
         return NULL;
     goto return_none_;

    {
     dpos_t length;
    case FILE_OPERATOR_TRUNC:
     if (DeeArg_Unpack(argc,argv,"I64u:trunc",&length))
         return NULL;
     if (DeeFile_Trunc(self,length))
         return NULL;
     goto return_none_;
    }

    case FILE_OPERATOR_CLOSE:
     if (DeeArg_Unpack(argc,argv,":close"))
         return NULL;
     if (DeeFile_Close(self))
         return NULL;
     goto return_none_;

    {
     DeeObject *a,*b;
     dpos_t pos; size_t max_bytes;
    case FILE_OPERATOR_PREAD:
     b = NULL;
     if (DeeArg_Unpack(argc,argv,"o|o:__pread__",&a,&b))
         return NULL;
     if (b) {
      if (DeeObject_AsSize(a,&max_bytes) ||
          DeeObject_AsUInt64(b,&pos))
          return NULL;
     } else {
      max_bytes = (size_t)-1;
      if (DeeObject_AsUInt64(a,&pos))
          return NULL;
     }
     return DeeFile_PReadText(self,max_bytes,pos,false);
    }

    {
     DeeObject *data,*a,*b,*c;
     dssize_t result; dpos_t pos;
    case FILE_OPERATOR_PWRITE:
     b = c = NULL;
     if (DeeArg_Unpack(argc,argv,"oo|oo:__write__",&data,&a,&b,&c) ||
         DeeObject_AssertTypeExact(data,&DeeString_Type))
         return NULL;
     if (c) {
      dssize_t ibegin,iend;
      if (DeeObject_AsSSize(a,&ibegin) ||
          DeeObject_AsSSize(b,&iend) ||
          DeeObject_AsUInt64(c,&pos))
          return NULL;
      if ((size_t)iend > DeeString_SIZE(data)*sizeof(char))
          iend = (dssize_t)DeeString_SIZE(data)*sizeof(char);
      if ((size_t)ibegin >= (size_t)iend)
          return_reference_(&DeeInt_Zero);
      result = DeeFile_PWrite(self,DeeString_STR(data)+ibegin,(size_t)(iend-ibegin),pos);
     } else if (b) {
      size_t max_size;
      if (DeeObject_AsSize(a,&max_size) ||
          DeeObject_AsUInt64(b,&pos))
          return NULL;
      if (max_size > DeeString_SIZE(data)*sizeof(char))
          max_size = DeeString_SIZE(data)*sizeof(char);
      result = DeeFile_PWrite(self,DeeString_STR(data),max_size,pos);
     } else {
      if (DeeObject_AsUInt64(a,&pos))
          return NULL;
      result = DeeFile_PWrite(self,DeeString_STR(data),
                              DeeString_SIZE(data)*sizeof(char),
                              pos);
     }
     if unlikely(result < 0) return NULL;
     return DeeInt_NewSize((size_t)result);
    }

    {
     int result;
    case FILE_OPERATOR_GETC:
     if (DeeArg_Unpack(argc,argv,":__getc__"))
         return NULL;
     result = DeeFile_Getc(self);
     if unlikely(result == GETC_ERR) return NULL;
     return DeeInt_NewInt(result);
    }

    {
     int ch;
    case FILE_OPERATOR_UNGETC:
     if (DeeArg_Unpack(argc,argv,"d:__ungetc__",&ch))
         return NULL;
     ch = DeeFile_Ungetc(self,ch);
     if unlikely(ch == GETC_ERR) return NULL;
     return_bool_(ch != GETC_EOF);
    }

    {
     int ch;
    case FILE_OPERATOR_PUTC:
     if (DeeArg_Unpack(argc,argv,"d:__putc__",&ch))
         return NULL;
     ch = DeeFile_Putc(self,ch);
     if unlikely(ch == GETC_ERR) return NULL;
     return_bool_(ch != GETC_EOF);
    }

    default: break;
    }
   }
   if (info->oi_type == OPTYPE_SPECIAL) goto special_operator;
   if (info->oi_private) goto private_operator;
   if (info->oi_type&OPTYPE_INPLACE && !pself) goto requires_inplace;
  }
  DeeError_Throwf(&DeeError_TypeError,
                  "Cannot invoke unknown operator #%X",name);
  return NULL;
special_operator:
  DeeError_Throwf(&DeeError_TypeError,
                  "Cannot invoke special `operator %s' (`__%s__') directly",
                  info->oi_uname,info->oi_sname);
  return NULL;
private_operator:
  DeeError_Throwf(&DeeError_TypeError,
                  "Cannot invoke private `operator %s' (`__%s__') directly",
                  info->oi_uname,info->oi_sname);
  return NULL;
requires_inplace:
  typetype = Dee_TYPE(self);
  if (typetype == &DeeSuper_Type)
      typetype = DeeSuper_TYPE(self);
  typetype = Dee_TYPE(typetype);
  info = Dee_OperatorInfo(typetype,name);
  if likely(info) {
   DeeError_Throwf(&DeeError_TypeError,
                   "Cannot invoke inplace `operator %s' (`__%s__') without l-value",
                   info->oi_uname,info->oi_sname);
  } else {
   DeeError_Throwf(&DeeError_TypeError,
                   "Cannot invoke inplace operator #%X without l-value",
                   name);
  }
  return NULL;
return_none_:
  return_none;
return_self:
  return_reference_(self);
 }
}

PUBLIC DREF DeeObject *DCALL
DeeObject_InvokeOperator(DeeObject *__restrict self, uint16_t name,
                         size_t argc, DeeObject **__restrict argv) {
 return invoke_operator(self,NULL,name,argc,argv);
}
PUBLIC DREF DeeObject *DCALL
DeeObject_PInvokeOperator(DeeObject **__restrict pself, uint16_t name,
                          size_t argc, DeeObject **__restrict argv) {
 ASSERT(pself);
 return invoke_operator(*pself,pself,name,argc,argv);
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_INFO_C */
