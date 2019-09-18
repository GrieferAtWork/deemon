/* Copyright (c) 2019 Griefer@Work                                            *
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

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/tuple.h>

#include "../objects/gc_inspect.h"
#include "runtime_error.h"

DECL_BEGIN

typedef DeeTypeObject Type;


INTERN struct opinfo basic_opinfo[OPERATOR_USERCOUNT] = {
	/* [OPERATOR_CONSTRUCTOR]  = */ { OPTYPE_SPECIAL,                               OPCLASS_TYPE, 0, offsetof(Type,tp_init.tp_alloc.tp_any_ctor),  "this",         "constructor", "tp_any_ctor" },
	/* [OPERATOR_COPY]         = */ { OPTYPE_SPECIAL,                               OPCLASS_TYPE, 0, offsetof(Type,tp_init.tp_alloc.tp_copy_ctor), "copy",         "copy",        "tp_copy_ctor" },
	/* [OPERATOR_DEEPCOPY]     = */ { OPTYPE_SPECIAL,                               OPCLASS_TYPE, 0, offsetof(Type,tp_init.tp_alloc.tp_deep_ctor), "deepcopy",     "deepcopy",    "tp_deep_ctor" },
	/* [OPERATOR_DESTRUCTOR]   = */ { OPTYPE_RVOID|OPTYPE_UNARY,                    OPCLASS_TYPE, 1, offsetof(Type,tp_init.tp_dtor),               "~this",        "destructor",  "tp_dtor" },
	/* [OPERATOR_ASSIGN]       = */ { OPTYPE_RINT|OPTYPE_BINARY,                    OPCLASS_TYPE, 0, offsetof(Type,tp_init.tp_assign),             ":=",           "assign",      "tp_assign" },
	/* [OPERATOR_MOVEASSIGN]   = */ { OPTYPE_RINT|OPTYPE_BINARY,                    OPCLASS_TYPE, 0, offsetof(Type,tp_init.tp_move_assign),        "move:=",       "moveassign",  "tp_move_assign" },
	/* [OPERATOR_STR]          = */ { OPTYPE_ROBJECT|OPTYPE_UNARY,                  OPCLASS_TYPE, 0, offsetof(Type,tp_cast.tp_str),                "str",          "str",         "tp_str" },
	/* [OPERATOR_REPR]         = */ { OPTYPE_ROBJECT|OPTYPE_UNARY,                  OPCLASS_TYPE, 0, offsetof(Type,tp_cast.tp_repr),               "repr",         "repr",        "tp_repr" },
	/* [OPERATOR_BOOL]         = */ { OPTYPE_RINT|OPTYPE_UNARY,                     OPCLASS_TYPE, 0, offsetof(Type,tp_cast.tp_bool),               "bool",         "bool",        "tp_bool" },
	/* [OPERATOR_ITERNEXT]     = */ { OPTYPE_ROBJECT|OPTYPE_UNARY,                  OPCLASS_TYPE, 0, offsetof(Type,tp_iter_next),                  "next",         "next",        "tp_iter_next" },
	/* [OPERATOR_CALL]         = */ { OPTYPE_ROBJECT|OPTYPE_BINARY|OPTYPE_VARIABLE, OPCLASS_TYPE, 0, offsetof(Type,tp_call),                       "()",           "call",        "tp_call" },
	/* [OPERATOR_INT]          = */ { OPTYPE_SPECIAL,                               OPCLASS_MATH, 0, offsetof(struct type_math,tp_int),            "int",          "int",         "tp_int" },
	/* [OPERATOR_FLOAT]        = */ { OPTYPE_DOUBLE,                                OPCLASS_MATH, 0, offsetof(struct type_math,tp_double),         "float",        "float",       "tp_double" },
	/* [OPERATOR_INV]          = */ { OPTYPE_ROBJECT|OPTYPE_UNARY,                  OPCLASS_MATH, 0, offsetof(struct type_math,tp_inv),            "~",            "inv",         "tp_inv" },
	/* [OPERATOR_POS]          = */ { OPTYPE_ROBJECT|OPTYPE_UNARY,                  OPCLASS_MATH, 0, offsetof(struct type_math,tp_pos),            "+",            "pos",         "tp_pos" },
	/* [OPERATOR_NEG]          = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_neg),            "-",            "neg",         "tp_neg" },
	/* [OPERATOR_ADD]          = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_add),            "+",            "add",         "tp_add" },
	/* [OPERATOR_SUB]          = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_sub),            "-",            "sub",         "tp_sub" },
	/* [OPERATOR_MUL]          = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_mul),            "*",            "mul",         "tp_mul" },
	/* [OPERATOR_DIV]          = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_div),            "/",            "div",         "tp_div" },
	/* [OPERATOR_MOD]          = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_mod),            "%",            "mod",         "tp_mod" },
	/* [OPERATOR_SHL]          = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_shl),            "<<",           "shl",         "tp_shl" },
	/* [OPERATOR_SHR]          = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_shr),            ">>",           "shr",         "tp_shr" },
	/* [OPERATOR_AND]          = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_and),            "&",            "and",         "tp_and" },
	/* [OPERATOR_OR]           = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_or),             "|",            "or",          "tp_or" },
	/* [OPERATOR_XOR]          = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_xor),            "^",            "xor",         "tp_xor" },
	/* [OPERATOR_POW]          = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_MATH, 0, offsetof(struct type_math,tp_pow),            "**",           "pow",         "tp_pow" },
	/* [OPERATOR_INC]          = */ { OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_UNARY,      OPCLASS_MATH, 0, offsetof(struct type_math,tp_inc),            "++",           "inc",         "tp_inc" },
	/* [OPERATOR_DEC]          = */ { OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_UNARY,      OPCLASS_MATH, 0, offsetof(struct type_math,tp_dec),            ",,",           "dec",         "tp_dec" },
	/* [OPERATOR_INPLACE_ADD]  = */ { OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_add),    "+=",           "iadd",        "tp_inplace_add" },
	/* [OPERATOR_INPLACE_SUB]  = */ { OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_sub),    ",=",           "isub",        "tp_inplace_sub" },
	/* [OPERATOR_INPLACE_MUL]  = */ { OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_mul),    "*=",           "imul",        "tp_inplace_mul" },
	/* [OPERATOR_INPLACE_DIV]  = */ { OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_div),    "/=",           "idiv",        "tp_inplace_div" },
	/* [OPERATOR_INPLACE_MOD]  = */ { OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_mod),    "%=",           "imod",        "tp_inplace_mod" },
	/* [OPERATOR_INPLACE_SHL]  = */ { OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_shl),    "<<=",          "ishl",        "tp_inplace_shl" },
	/* [OPERATOR_INPLACE_SHR]  = */ { OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_shr),    ">>=",          "ishr",        "tp_inplace_shr" },
	/* [OPERATOR_INPLACE_AND]  = */ { OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_and),    "&=",           "iand",        "tp_inplace_and" },
	/* [OPERATOR_INPLACE_OR]   = */ { OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_or),     "|=",           "ior",         "tp_inplace_or" },
	/* [OPERATOR_INPLACE_XOR]  = */ { OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_xor),    "^=",           "ixor",        "tp_inplace_xor" },
	/* [OPERATOR_INPLACE_POW]  = */ { OPTYPE_RINT|OPTYPE_INPLACE|OPTYPE_BINARY,     OPCLASS_MATH, 0, offsetof(struct type_math,tp_inplace_pow),    "**=",          "ipow",        "tp_inplace_pow" },
	/* [OPERATOR_HASH]         = */ { OPTYPE_RUINTPTR|OPTYPE_UNARY,                 OPCLASS_CMP,  0, offsetof(struct type_cmp,tp_hash),            "hash",         "hash",        "tp_hash" },
	/* [OPERATOR_EQ]           = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_CMP,  0, offsetof(struct type_cmp,tp_eq),              "==",           "eq",          "tp_eq" },
	/* [OPERATOR_NE]           = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_CMP,  0, offsetof(struct type_cmp,tp_ne),              "!=",           "ne",          "tp_ne" },
	/* [OPERATOR_LO]           = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_CMP,  0, offsetof(struct type_cmp,tp_lo),              "<",            "lo",          "tp_lo" },
	/* [OPERATOR_LE]           = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_CMP,  0, offsetof(struct type_cmp,tp_le),              "<=",           "le",          "tp_le" },
	/* [OPERATOR_GR]           = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_CMP,  0, offsetof(struct type_cmp,tp_gr),              ">",            "gr",          "tp_gr" },
	/* [OPERATOR_GE]           = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_CMP,  0, offsetof(struct type_cmp,tp_ge),              ">=",           "ge",          "tp_ge" },
	/* [OPERATOR_ITERSELF]     = */ { OPTYPE_ROBJECT|OPTYPE_UNARY,                  OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_iter_self),       "iter",         "iter",        "tp_iter_self" },
	/* [OPERATOR_SIZE]         = */ { OPTYPE_ROBJECT|OPTYPE_UNARY,                  OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_size),            "#",            "size",        "tp_size" },
	/* [OPERATOR_CONTAINS]     = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_contains),        "contains",     "contains",    "tp_contains" },
	/* [OPERATOR_GETITEM]      = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_get),             "[]",           "getitem",     "tp_get" },
	/* [OPERATOR_DELITEM]      = */ { OPTYPE_RINT|OPTYPE_BINARY,                    OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_del),             "del[]",        "delitem",     "tp_del" },
	/* [OPERATOR_SETITEM]      = */ { OPTYPE_RINT|OPTYPE_TRINARY,                   OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_set),             "[]=",          "setitem",     "tp_set" },
	/* [OPERATOR_GETRANGE]     = */ { OPTYPE_ROBJECT|OPTYPE_TRINARY,                OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_range_get),       "[:]",          "getrange",    "tp_range_get" },
	/* [OPERATOR_DELRANGE]     = */ { OPTYPE_RINT|OPTYPE_TRINARY,                   OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_range_del),       "del[:]",       "delrange",    "tp_range_del" },
	/* [OPERATOR_SETRANGE]     = */ { OPTYPE_RINT|OPTYPE_QUAD,                      OPCLASS_SEQ,  0, offsetof(struct type_seq,tp_range_set),       "[:]=",         "setrange",    "tp_range_set" },
	/* [OPERATOR_GETATTR]      = */ { OPTYPE_ROBJECT|OPTYPE_BINARY,                 OPCLASS_ATTR, 0, offsetof(struct type_attr,tp_getattr),        ".",            "getattr",     "tp_getattr" },
	/* [OPERATOR_DELATTR]      = */ { OPTYPE_RINT|OPTYPE_BINARY,                    OPCLASS_ATTR, 0, offsetof(struct type_attr,tp_delattr),        "del.",         "delattr",     "tp_delattr" },
	/* [OPERATOR_SETATTR]      = */ { OPTYPE_RINT|OPTYPE_TRINARY,                   OPCLASS_ATTR, 0, offsetof(struct type_attr,tp_setattr),        ".=",           "setattr",     "tp_setattr" },
	/* [OPERATOR_ENUMATTR]     = */ { OPTYPE_ENUMATTR,                              OPCLASS_ATTR, 0, offsetof(struct type_attr,tp_enumattr),       "...",          "enumattr",    "tp_enumattr" },
	/* [OPERATOR_ENTER]        = */ { OPTYPE_RINT|OPTYPE_UNARY,                     OPCLASS_WITH, 0, offsetof(struct type_with,tp_enter),          "enter",        "enter",       "tp_enter" },
	/* [OPERATOR_LEAVE]        = */ { OPTYPE_RINT|OPTYPE_UNARY,                     OPCLASS_WITH, 0, offsetof(struct type_with,tp_leave),          "leave",        "leave",       "tp_leave" }
};

PRIVATE struct opinfo private_opinfo[(OPERATOR_PRIVMAX-OPERATOR_PRIVMIN)+1] = {
	/* [OPERATOR_VISIT  - OPERATOR_PRIVMIN] = */ { OPTYPE_VISIT,              OPCLASS_TYPE,   1, offsetof(Type,tp_visit),               "", "", "tp_visit" },
	/* [OPERATOR_CLEAR  - OPERATOR_PRIVMIN] = */ { OPTYPE_RVOID|OPTYPE_UNARY, OPCLASS_GC,     1, offsetof(struct type_gc,tp_clear),     "", "", "tp_clear" },
	/* [OPERATOR_PCLEAR - OPERATOR_PRIVMIN] = */ { OPTYPE_PCLEAR,             OPCLASS_GC,     1, offsetof(struct type_gc,tp_pclear),    "", "", "tp_pclear" },
	/* [OPERATOR_GETBUF - OPERATOR_PRIVMIN] = */ { OPTYPE_GETBUF,             OPCLASS_BUFFER, 1, offsetof(struct type_buffer,tp_getbuf),"", "", "tp_getbuf" }
};

INTERN struct opinfo file_opinfo[FILE_OPERATOR_COUNT] = {
	/* [FILE_OPERATOR_READ   - OPERATOR_EXTENDED(0)] = */ { OPTYPE_READWRITE,         OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_read),   "read",   "read",   "ft_read",   },
	/* [FILE_OPERATOR_WRITE  - OPERATOR_EXTENDED(0)] = */ { OPTYPE_READWRITE,         OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_write),  "write",  "write",  "ft_write",  },
	/* [FILE_OPERATOR_SEEK   - OPERATOR_EXTENDED(0)] = */ { OPTYPE_SEEK,              OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_seek),   "seek",   "seek",   "ft_seek",   },
	/* [FILE_OPERATOR_SYNC   - OPERATOR_EXTENDED(0)] = */ { OPTYPE_RINT|OPTYPE_UNARY, OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_sync),   "sync",   "sync",   "ft_sync",   },
	/* [FILE_OPERATOR_TRUNC  - OPERATOR_EXTENDED(0)] = */ { OPTYPE_TRUNC,             OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_trunc),  "trunc",  "trunc",  "ft_trunc",  },
	/* [FILE_OPERATOR_CLOSE  - OPERATOR_EXTENDED(0)] = */ { OPTYPE_RINT|OPTYPE_UNARY, OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_close),  "close",  "close",  "ft_close",  },
	/* [FILE_OPERATOR_PREAD  - OPERATOR_EXTENDED(0)] = */ { OPTYPE_PREADWRITE,        OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_pread),  "pread",  "pread",  "ft_pread",  },
	/* [FILE_OPERATOR_PWRITE - OPERATOR_EXTENDED(0)] = */ { OPTYPE_PREADWRITE,        OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_pwrite), "pwrite", "pwrite", "ft_pwrite", },
	/* [FILE_OPERATOR_GETC   - OPERATOR_EXTENDED(0)] = */ { OPTYPE_RINT|OPTYPE_UNARY, OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_getc),   "getc",   "getc",   "ft_getc",   },
	/* [FILE_OPERATOR_UNGETC - OPERATOR_EXTENDED(0)] = */ { OPTYPE_UNGETPUT,          OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_ungetc), "ungetc", "ungetc", "ft_ungetc", },
	/* [FILE_OPERATOR_PUTC   - OPERATOR_EXTENDED(0)] = */ { OPTYPE_UNGETPUT,          OPCLASS_TYPE, 0, offsetof(DeeFileTypeObject,ft_putc),   "putc",   "putc",   "ft_putc",   }
};

PUBLIC uint16_t DCALL
Dee_OperatorFromNameLen(DeeTypeObject *typetype,
                        char const *__restrict name,
                        size_t namelen) {
	char buf[32];
	if (namelen >= COMPILER_LENOF(buf))
		return (uint16_t)-1; /* No valid operator has that long of a name... */
	memcpy(buf, name, namelen);
	buf[namelen] = 0;
	return Dee_OperatorFromName(typetype, buf);
}

PUBLIC uint16_t DCALL
Dee_OperatorFromName(DeeTypeObject *typetype,
                     char const *__restrict name) {
#define EQAT(ptr, str) (memcmp(ptr, str, sizeof(str)) == 0)
#define RETURN(id)     \
	do {               \
		result = (id); \
		goto done;     \
	} __WHILE0
	uint16_t result = (uint16_t)-1;
	switch (*name) {

	case '.':
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_SETATTR);
		goto done;

	case '[':
		if (name[1] == ']' && name[2] == '=' && !name[3])
			RETURN(OPERATOR_SETITEM);
		if (name[1] == ':' && name[2] == ']' && name[3] == '=' && !name[4])
			RETURN(OPERATOR_SETITEM);
		goto done;

	case '+':
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_ADD);
		if (name[1] == '+' && !name[2])
			RETURN(OPERATOR_INC);
		goto done;

	case '-':
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_SUB);
		if (name[1] == '-' && !name[2])
			RETURN(OPERATOR_DEC);
		goto done;

	case '~':
		if (!name[1])
			RETURN(OPERATOR_INV);
		break;

	case '*':
		if (!name[1])
			RETURN(OPERATOR_MUL);
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_MUL);
		if (name[1] == '*') {
			if (!name[2])
				RETURN(OPERATOR_POW);
			if (name[2] == '=' && !name[3])
				RETURN(OPERATOR_INPLACE_POW);
		}
		break;

	case '/':
		if (!name[1])
			RETURN(OPERATOR_DIV);
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_DIV);
		break;

	case '%':
		if (!name[1])
			RETURN(OPERATOR_MOD);
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_MOD);
		break;

	case '&':
		if (!name[1])
			RETURN(OPERATOR_AND);
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_AND);
		break;

	case '|':
		if (!name[1])
			RETURN(OPERATOR_OR);
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_OR);
		break;

	case '^':
		if (!name[1])
			RETURN(OPERATOR_XOR);
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_INPLACE_XOR);
		break;

	case ':':
		if (name[1] != '=')
			break;
		if (!name[2])
			RETURN(OPERATOR_ASSIGN);
		if (EQAT(name + 2, "move"))
			RETURN(OPERATOR_MOVEASSIGN);
		break;

	case '<':
		if (!name[1])
			RETURN(OPERATOR_LO);
		if (name[1] == '<') {
			if (!name[2])
				RETURN(OPERATOR_SHL);
			if (name[2] == '=' && !name[3])
				RETURN(OPERATOR_INPLACE_SHL);
		} else {
			if (name[1] == '=' && !name[2])
				RETURN(OPERATOR_LE);
		}
		break;

	case '>':
		if (!name[1])
			RETURN(OPERATOR_GR);
		if (name[1] == '>') {
			if (!name[2])
				RETURN(OPERATOR_SHR);
			if (name[2] == '=' && !name[3])
				RETURN(OPERATOR_INPLACE_SHR);
		} else {
			if (name[1] == '=' && !name[2])
				RETURN(OPERATOR_GE);
		}
		break;

	case '=':
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_EQ);
		break;

	case '!':
		if (name[1] == '=' && !name[2])
			RETURN(OPERATOR_NE);
		break;

	case '#':
		if (!name[1])
			RETURN(OPERATOR_SIZE);
		break;

	case 'a':
		if (EQAT(name + 1, "ssign"))
			RETURN(OPERATOR_ASSIGN);
		if (EQAT(name + 1, "dd"))
			RETURN(OPERATOR_ADD);
		if (EQAT(name + 1, "nd"))
			RETURN(OPERATOR_AND);
		break;

	case 'b':
		if (EQAT(name + 1, "ool"))
			RETURN(OPERATOR_BOOL);
		break;

	case 'c':
		if (EQAT(name + 1, "opy"))
			RETURN(OPERATOR_COPY);
		if (EQAT(name + 1, "all"))
			RETURN(OPERATOR_CALL);
		if (EQAT(name + 1, "ontains"))
			RETURN(OPERATOR_CONTAINS);
		break;

	case 'd':
		if (EQAT(name + 1, "iv"))
			RETURN(OPERATOR_DIV);
		if (name[1] == 'e') {
			if (EQAT(name + 2, "epcopy"))
				RETURN(OPERATOR_DEEPCOPY);
			if (EQAT(name + 2, "structor"))
				RETURN(OPERATOR_DESTRUCTOR);
			if (name[2] == 'c' && !name[3])
				RETURN(OPERATOR_DEC);
			if (name[2] == 'l') {
				if (name[3] == '[') {
					if (name[4] == ']' && !name[5])
						RETURN(OPERATOR_DELITEM);
					if (name[4] == ':' && name[5] == ']' && !name[6])
						RETURN(OPERATOR_DELRANGE);
				}
				if (name[3] == '.' && !name[4])
					RETURN(OPERATOR_DELATTR);
				if (EQAT(name + 3, "item"))
					RETURN(OPERATOR_DELITEM);
				if (EQAT(name + 3, "range"))
					RETURN(OPERATOR_DELRANGE);
				if (EQAT(name + 3, "attr"))
					RETURN(OPERATOR_DELATTR);
			}
		}
		break;

	case 'e':
		if (name[1] == 'q' && !name[2])
			RETURN(OPERATOR_EQ);
		if (EQAT(name + 1, "numattr"))
			RETURN(OPERATOR_ENUMATTR);
		if (EQAT(name + 1, "nter"))
			RETURN(OPERATOR_ENTER);
		break;

	case 'f':
		if (EQAT(name + 1, "loat"))
			RETURN(OPERATOR_FLOAT);
		break;

	case 'g':
		if (name[1] == 'r' && !name[2])
			RETURN(OPERATOR_GR);
		if (name[1] == 'e') {
			if (!name[2])
				RETURN(OPERATOR_GE);
			if (name[2] == 't') {
				if (EQAT(name + 3, "item"))
					RETURN(OPERATOR_GETITEM);
				if (EQAT(name + 3, "range"))
					RETURN(OPERATOR_GETRANGE);
				if (EQAT(name + 3, "attr"))
					RETURN(OPERATOR_GETATTR);
			}
		}
		break;

	case 'h':
		if (EQAT(name + 1, "ash"))
			RETURN(OPERATOR_HASH);
		break;

	case 'i':
		if (name[1] == 't' && name[2] == 'e' && name[3] == 'r') {
			if (!name[4])
				RETURN(OPERATOR_ITERSELF);
			if (EQAT(name + 4, "self"))
				RETURN(OPERATOR_ITERSELF);
			if (EQAT(name + 4, "next"))
				RETURN(OPERATOR_ITERNEXT);
		} else {
			char const *iter;
			if (name[1] == 'n') {
				if (name[2] == 't' && !name[3])
					RETURN(OPERATOR_INT);
				if (name[2] == 'v' && !name[3])
					RETURN(OPERATOR_INV);
				if (name[2] == 'c' && !name[3])
					RETURN(OPERATOR_INC);
			}
			iter = name;
			++iter;
			if (iter[0] == 'n' && iter[1] == 'p' &&
			    iter[2] == 'l' && iter[3] == 'a' &&
			    iter[4] == 'c' && iter[5] == 'e') {
				iter += 6;
				if (iter[0] == '_')
					++iter;
			}
			switch (iter[0]) {

			case 'a':
				if (iter[2] != 'd')
					break;
				if (iter[1] == 'd' && !iter[3])
					RETURN(OPERATOR_INPLACE_ADD);
				if (iter[1] == 'n' && !iter[3])
					RETURN(OPERATOR_INPLACE_AND);
				break;

			case 's':
				if (iter[1] == 'u' && iter[2] == 'b' && !iter[3])
					RETURN(OPERATOR_INPLACE_SUB);
				if (iter[1] == 'h') {
					if (iter[2] == 'l' && !iter[3])
						RETURN(OPERATOR_INPLACE_SHL);
					if (iter[2] == 'r' && !iter[3])
						RETURN(OPERATOR_INPLACE_SHR);
				}
				break;

			case 'm':
				if (iter[1] == 'u' && iter[2] == 'l' && !iter[3])
					RETURN(OPERATOR_INPLACE_MUL);
				if (iter[1] == 'o' && iter[2] == 'd' && !iter[3])
					RETURN(OPERATOR_INPLACE_MOD);
				break;

			case 'd':
				if (iter[1] == 'i' && iter[2] == 'v' && !iter[3])
					RETURN(OPERATOR_INPLACE_DIV);
				break;

			case 'o':
				if (iter[1] == 'r' && !iter[2])
					RETURN(OPERATOR_INPLACE_OR);
				break;

			case 'x':
				if (iter[1] == 'o' && iter[2] == 'r' && !iter[3])
					RETURN(OPERATOR_INPLACE_XOR);
				break;

			case 'p':
				if (iter[1] == 'o' && iter[2] == 'w' && !iter[3])
					RETURN(OPERATOR_INPLACE_POW);
				break;

			default: break;
			}
		}
		break;

	case 'l':
		if (name[1] == 'o' && !name[2])
			RETURN(OPERATOR_LO);
		if (name[1] == 'e') {
			if (!name[2])
				RETURN(OPERATOR_LE);
			if (EQAT(name + 2, "ave"))
				RETURN(OPERATOR_LEAVE);
		}
		break;

	case 'm':
		if (EQAT(name + 1, "oveassign"))
			RETURN(OPERATOR_MOVEASSIGN);
		if (EQAT(name + 1, "ul"))
			RETURN(OPERATOR_MUL);
		if (EQAT(name + 1, "od"))
			RETURN(OPERATOR_MOD);
		break;

	case 'n':
		if (name[1] == 'e') {
			if (!name[2])
				RETURN(OPERATOR_NE);
			if (name[2] == 'g' && !name[3])
				RETURN(OPERATOR_NEG);
			if (name[2] == 'x' && name[3] == 't' && !name[4])
				RETURN(OPERATOR_ITERNEXT);
		}
		break;

	case 'o':
		if (name[1] == 'r' && !name[2])
			RETURN(OPERATOR_OR);
		break;

	case 'p':
		if (name[1] == 'o' && !name[3]) {
			if (name[2] == 's')
				RETURN(OPERATOR_POS);
			if (name[2] == 'w')
				RETURN(OPERATOR_POW);
		}
		break;

	case 'r':
		if (EQAT(name + 1, "epr"))
			RETURN(OPERATOR_REPR);
		break;

	case 's':
		if (EQAT(name + 1, "tr"))
			RETURN(OPERATOR_STR);
		if (EQAT(name + 1, "ub"))
			RETURN(OPERATOR_SUB);
		if (EQAT(name + 1, "hl"))
			RETURN(OPERATOR_SHL);
		if (EQAT(name + 1, "hr"))
			RETURN(OPERATOR_SHR);
		if (EQAT(name + 1, "ize"))
			RETURN(OPERATOR_SIZE);
		if (name[1] == 'e' && name[2] == 't') {
			if (EQAT(name + 3, "item"))
				RETURN(OPERATOR_SETITEM);
			if (EQAT(name + 3, "range"))
				RETURN(OPERATOR_SETRANGE);
			if (EQAT(name + 3, "attr"))
				RETURN(OPERATOR_SETATTR);
		}
		break;

	case 'x':
		if (EQAT(name + 1, "or"))
			RETURN(OPERATOR_XOR);
		break;

	default: break;
	}
	/* Fallback: manually resolve names. */
	for (result = 0; result < OPERATOR_USERCOUNT; ++result) {
		if (!strcmp(basic_opinfo[result].oi_sname, name))
			goto done;
		if (!strcmp(basic_opinfo[result].oi_uname, name))
			goto done;
	}
	if (typetype == &DeeFileType_Type) {
		for (result = 0; result < OPERATOR_USERCOUNT; ++result) {
			if (!strcmp(file_opinfo[result].oi_sname, name))
				goto done_extended;
			if (!strcmp(file_opinfo[result].oi_uname, name))
				goto done_extended;
		}
	}
	result = (uint16_t)-1; /* Not found! */
done:
	return result;
done_extended:
	result += OPERATOR_EXTENDED(0);
	goto done;
#undef EQAT
#undef RETURN
}


PUBLIC struct opinfo *DCALL
Dee_OperatorInfo(DeeTypeObject *typetype,
                 uint16_t id) {
	ASSERT_OBJECT_TYPE_OPT(typetype, &DeeType_Type);
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
	DeeObject *other;
	ASSERT_OBJECT(self);
	/* Fast-pass for known operators. */
	switch (name) {

	case OPERATOR_CONSTRUCTOR:
		/* Special wrapper: invoke the construction operator. */
		if (DeeObject_AssertType(self, &DeeType_Type))
			goto err;
		return DeeObject_New((DeeTypeObject *)self, argc, argv);

	case OPERATOR_COPY:
		/* Special wrapper: invoke the copy constructor. */
		if (DeeArg_Unpack(argc, argv, ":__copy__"))
			goto err;
		return DeeObject_Copy(self);

	case OPERATOR_DEEPCOPY:
		/* Special wrapper: invoke the deepcopy constructor. */
		if (DeeArg_Unpack(argc, argv, ":__deepcopy__"))
			goto err;
		return DeeObject_DeepCopy(self);

	case OPERATOR_ASSIGN:
		if (DeeArg_Unpack(argc, argv, "o:__assign__", &other))
			goto err;
		if (DeeObject_Assign(self, other))
			goto err;
		goto return_self;

	case OPERATOR_MOVEASSIGN:
		if (DeeArg_Unpack(argc, argv, "o:__moveassign__", &other))
			goto err;
		if (DeeObject_MoveAssign(self, other))
			goto err;
		goto return_self;

	case OPERATOR_STR:
		if (DeeArg_Unpack(argc, argv, ":__str__"))
			goto err;
		return DeeObject_Str(self);

	case OPERATOR_REPR:
		if (DeeArg_Unpack(argc, argv, ":__repr__"))
			goto err;
		return DeeObject_Repr(self);

	case OPERATOR_BOOL: {
		int result;
		if (DeeArg_Unpack(argc, argv, ":__bool__"))
			goto err;
		result = DeeObject_Bool(self);
		if unlikely(result < 0)
			goto err;
		return_bool_(result);
	}

	case OPERATOR_CALL: {
		DeeObject *kw;
		kw = NULL;
		if (DeeArg_Unpack(argc, argv, "o|o:__call__", &other, &kw))
			goto err;
		if (DeeObject_AssertTypeExact(other, &DeeTuple_Type))
			goto err;
		return DeeObject_CallTupleKw(self, other, kw);
	}

	case OPERATOR_HASH: {
		dhash_t result;
		if (DeeArg_Unpack(argc, argv, ":__hash__"))
			goto err;
		result = DeeObject_Hash(self);
		return DeeInt_NewHash(result);
	}

	case OPERATOR_ITERNEXT: {
		DREF DeeObject *result;
		if (DeeArg_Unpack(argc, argv, ":__next__"))
			goto err;
		result = DeeObject_IterNext(self);
		if (result == ITER_DONE) {
			DeeError_Throw(&DeeError_StopIteration_instance);
			result = NULL;
		}
		return result;
	}

	case OPERATOR_INT:
		if (DeeArg_Unpack(argc, argv, ":__int__"))
			goto err;
		return DeeObject_Int(self);

	case OPERATOR_FLOAT: {
		double value;
		if (DeeArg_Unpack(argc, argv, ":__float__"))
			goto err;
		if (DeeObject_AsDouble(self, &value))
			goto err;
		return DeeFloat_New(value);
	}

	case OPERATOR_INV:
		if (DeeArg_Unpack(argc, argv, ":__inv__"))
			goto err;
		return DeeObject_Inv(self);
	case OPERATOR_POS:
		if (DeeArg_Unpack(argc, argv, ":__pos__"))
			goto err;
		return DeeObject_Pos(self);
	case OPERATOR_NEG:
		if (DeeArg_Unpack(argc, argv, ":__neg__"))
			goto err;
		return DeeObject_Neg(self);

	case OPERATOR_ADD:
		if (DeeArg_Unpack(argc, argv, "o:__add__", &other))
			goto err;
		return DeeObject_Add(self, other);

	case OPERATOR_SUB:
		if (DeeArg_Unpack(argc, argv, "o:__sub__", &other))
			goto err;
		return DeeObject_Sub(self, other);

	case OPERATOR_MUL:
		if (DeeArg_Unpack(argc, argv, "o:__mul__", &other))
			goto err;
		return DeeObject_Mul(self, other);

	case OPERATOR_DIV:
		if (DeeArg_Unpack(argc, argv, "o:__div__", &other))
			goto err;
		return DeeObject_Div(self, other);

	case OPERATOR_MOD:
		if (DeeArg_Unpack(argc, argv, "o:__mod__", &other))
			goto err;
		return DeeObject_Mod(self, other);

	case OPERATOR_SHL:
		if (DeeArg_Unpack(argc, argv, "o:__shl__", &other))
			goto err;
		return DeeObject_Shl(self, other);

	case OPERATOR_SHR:
		if (DeeArg_Unpack(argc, argv, "o:__shr__", &other))
			goto err;
		return DeeObject_Shr(self, other);

	case OPERATOR_AND:
		if (DeeArg_Unpack(argc, argv, "o:__and__", &other))
			goto err;
		return DeeObject_And(self, other);

	case OPERATOR_OR:
		if (DeeArg_Unpack(argc, argv, "o:__or__", &other))
			goto err;
		return DeeObject_Or(self, other);

	case OPERATOR_XOR:
		if (DeeArg_Unpack(argc, argv, "o:__xor__", &other))
			goto err;
		return DeeObject_Xor(self, other);

	case OPERATOR_POW:
		if (DeeArg_Unpack(argc, argv, "o:__pow__", &other))
			goto err;
		return DeeObject_Pow(self, other);

	case OPERATOR_INC:
		if unlikely(!pself)
			goto requires_inplace;
		if (DeeArg_Unpack(argc, argv, ":__inc__"))
			goto err;
		if (DeeObject_Inc(pself))
			goto err;
		goto return_self;

	case OPERATOR_DEC:
		if unlikely(!pself)
			goto requires_inplace;
		if (DeeArg_Unpack(argc, argv, ":__dec__"))
			goto err;
		if (DeeObject_Dec(pself))
			goto err;
		goto return_self;

	case OPERATOR_INPLACE_ADD:
		if unlikely(!pself)
			goto requires_inplace;
		if (DeeArg_Unpack(argc, argv, "o:__iadd__", &other))
			goto err;
		if (DeeObject_InplaceAdd(pself, other))
			goto err;
		goto return_self;

	case OPERATOR_INPLACE_SUB:
		if unlikely(!pself)
			goto requires_inplace;
		if (DeeArg_Unpack(argc, argv, "o:__isub__", &other))
			goto err;
		if (DeeObject_InplaceSub(pself, other))
			goto err;
		goto return_self;

	case OPERATOR_INPLACE_MUL:
		if unlikely(!pself)
			goto requires_inplace;
		if (DeeArg_Unpack(argc, argv, "o:__imul__", &other))
			goto err;
		if (DeeObject_InplaceMul(pself, other))
			goto err;
		goto return_self;

	case OPERATOR_INPLACE_DIV:
		if unlikely(!pself)
			goto requires_inplace;
		if (DeeArg_Unpack(argc, argv, "o:__idiv__", &other))
			goto err;
		if (DeeObject_InplaceDiv(pself, other))
			goto err;
		goto return_self;

	case OPERATOR_INPLACE_MOD:
		if unlikely(!pself)
			goto requires_inplace;
		if (DeeArg_Unpack(argc, argv, "o:__imod__", &other))
			goto err;
		if (DeeObject_InplaceMod(pself, other))
			goto err;
		goto return_self;

	case OPERATOR_INPLACE_SHL:
		if unlikely(!pself)
			goto requires_inplace;
		if (DeeArg_Unpack(argc, argv, "o:__ishl__", &other))
			goto err;
		if (DeeObject_InplaceShl(pself, other))
			goto err;
		goto return_self;

	case OPERATOR_INPLACE_SHR:
		if unlikely(!pself)
			goto requires_inplace;
		if (DeeArg_Unpack(argc, argv, "o:__ishr__", &other))
			goto err;
		if (DeeObject_InplaceShr(pself, other))
			goto err;
		goto return_self;

	case OPERATOR_INPLACE_AND:
		if unlikely(!pself)
			goto requires_inplace;
		if (DeeArg_Unpack(argc, argv, "o:__iand__", &other))
			goto err;
		if (DeeObject_InplaceAnd(pself, other))
			goto err;
		goto return_self;

	case OPERATOR_INPLACE_OR:
		if unlikely(!pself)
			goto requires_inplace;
		if (DeeArg_Unpack(argc, argv, "o:__ior__", &other))
			goto err;
		if (DeeObject_InplaceOr(pself, other))
			goto err;
		goto return_self;

	case OPERATOR_INPLACE_XOR:
		if unlikely(!pself)
			goto requires_inplace;
		if (DeeArg_Unpack(argc, argv, "o:__ixor__", &other))
			goto err;
		if (DeeObject_InplaceXor(pself, other))
			goto err;
		goto return_self;

	case OPERATOR_INPLACE_POW:
		if unlikely(!pself)
			goto requires_inplace;
		if (DeeArg_Unpack(argc, argv, "o:__ipow__", &other))
			goto err;
		if (DeeObject_InplacePow(pself, other))
			goto err;
		goto return_self;

	case OPERATOR_EQ:
		if (DeeArg_Unpack(argc, argv, "o:__eq__", &other))
			goto err;
		return DeeObject_CompareEqObject(self, other);

	case OPERATOR_NE:
		if (DeeArg_Unpack(argc, argv, "o:__ne__", &other))
			goto err;
		return DeeObject_CompareNeObject(self, other);

	case OPERATOR_LO:
		if (DeeArg_Unpack(argc, argv, "o:__lo__", &other))
			goto err;
		return DeeObject_CompareLoObject(self, other);

	case OPERATOR_LE:
		if (DeeArg_Unpack(argc, argv, "o:__le__", &other))
			goto err;
		return DeeObject_CompareLeObject(self, other);

	case OPERATOR_GR:
		if (DeeArg_Unpack(argc, argv, "o:__gr__", &other))
			goto err;
		return DeeObject_CompareGrObject(self, other);

	case OPERATOR_GE:
		if (DeeArg_Unpack(argc, argv, "o:__ge__", &other))
			goto err;
		return DeeObject_CompareGeObject(self, other);

	case OPERATOR_ITERSELF:
		if (DeeArg_Unpack(argc, argv, ":__iter__"))
			goto err;
		return DeeObject_IterSelf(self);

	case OPERATOR_SIZE:
		if (DeeArg_Unpack(argc, argv, ":__size__"))
			goto err;
		return DeeObject_SizeObject(self);

	case OPERATOR_CONTAINS:
		if (DeeArg_Unpack(argc, argv, "o:__contains__", &other))
			goto err;
		return DeeObject_ContainsObject(self, other);

	case OPERATOR_GETITEM:
		if (DeeArg_Unpack(argc, argv, "o:__getitem__", &other))
			goto err;
		return DeeObject_GetItem(self, other);

	case OPERATOR_DELITEM:
		if (DeeArg_Unpack(argc, argv, "o:__delitem__", &other))
			goto err;
		if (DeeObject_DelItem(self, other))
			goto err;
		goto return_none_;

	{
		DeeObject *begin, *end;
	case OPERATOR_SETITEM:
		if (DeeArg_Unpack(argc, argv, "oo:__setitem__", &begin, &other))
			goto err;
		if (DeeObject_SetItem(self, begin, other))
			goto err;
		return_reference_(other);
	case OPERATOR_GETRANGE:
		if (DeeArg_Unpack(argc, argv, "oo:__getrange__", &begin, &end))
			goto err;
		return DeeObject_GetRange(self, begin, end);
	case OPERATOR_DELRANGE:
		if (DeeArg_Unpack(argc, argv, "oo:__delrange__", &begin, &end))
			goto err;
		if (DeeObject_DelRange(self, begin, end))
			goto err;
		goto return_none_;
	case OPERATOR_SETRANGE:
		if (DeeArg_Unpack(argc, argv, "ooo:__setrange__", &begin, &end, &other))
			goto err;
		if (DeeObject_SetRange(self, begin, end, other))
			goto err;
		return_reference_(other);
	}

	case OPERATOR_GETATTR:
		if (DeeArg_Unpack(argc, argv, "o:__getattr__", &other))
			goto err;
		if (DeeObject_AssertTypeExact(other, &DeeString_Type))
			goto err;
		return DeeObject_GetAttr(self, other);

	case OPERATOR_DELATTR:
		if (DeeArg_Unpack(argc, argv, "o:__delattr__", &other))
			goto err;
		if (DeeObject_AssertTypeExact(other, &DeeString_Type))
			goto err;
		if (DeeObject_DelAttr(self, other))
			goto err;
		goto return_none_;

	case OPERATOR_SETATTR: {
		DeeObject *value;
		if (DeeArg_Unpack(argc, argv, "oo:__setattr__", &other, &value))
			goto err;
		if (DeeObject_AssertTypeExact(other, &DeeString_Type))
			goto err;
		if (DeeObject_SetAttr(self, other, value))
			goto err;
		return_reference_(value);
	}

	case OPERATOR_ENUMATTR:
		if (DeeArg_Unpack(argc, argv, ":__enumattr__"))
			goto err;
		return DeeObject_New(&DeeEnumAttr_Type, 1, (DeeObject **)&self);

	case OPERATOR_ENTER:
		if (DeeArg_Unpack(argc, argv, ":__enter__"))
			goto err;
		if (DeeObject_Enter(self))
			goto err;
		goto return_none_;

	case OPERATOR_LEAVE:
		if (DeeArg_Unpack(argc, argv, ":__leave__"))
			goto err;
		if (DeeObject_Leave(self))
			goto err;
		goto return_none_;


		/* Hidden private operators */
	case OPERATOR_VISIT:
		if (DeeArg_Unpack(argc, argv, ":__visit__"))
			goto err;
		return (DREF DeeObject *)DeeGC_NewReferred(self);

	case OPERATOR_CLEAR:
		if (DeeArg_Unpack(argc, argv, ":__clear__"))
			goto err;
		DeeObject_Clear(self);
		goto return_none_;

	case OPERATOR_PCLEAR: {
		unsigned int gc_prio;
		if (DeeArg_Unpack(argc, argv, "u:__pclear__", &gc_prio))
			goto err;
		DeeObject_PClear(self, gc_prio);
		goto return_none_;
	}

	case OPERATOR_GETBUF: {
		bool writable;
		size_t start, end;
		writable = false;
		start = 0, end = (size_t)-1;
		if (DeeArg_Unpack(argc, argv, "|bIuIu:__getbuf__", &writable, &start, &end))
			goto err;
		return DeeObject_Bytes(self,
		                       writable ? Dee_BUFFER_FWRITABLE
		                                : Dee_BUFFER_FREADONLY,
		                       start,
		                       end);
	}	break;

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
		info     = Dee_OperatorInfo(typetype, name);
		if (info) {
			if (typetype == &DeeFileType_Type) {
				/* Invoke file operators. */
				switch (name) {

				case FILE_OPERATOR_READ: {
					size_t max_bytes;
					max_bytes = (size_t)-1;
					/* TODO: Support for reading into buffers */
					if (DeeArg_Unpack(argc, argv, "|Iu:__read__", &max_bytes))
						goto err;
					return DeeFile_ReadText(self, max_bytes, false);
				}

				case FILE_OPERATOR_WRITE: {
					DeeObject *data, *begin, *end;
					dssize_t result;
					begin = end = NULL;
					/* TODO: Support for writing buffers */
					if (DeeArg_Unpack(argc, argv, "o|oo:__write__", &data, &begin, &end) ||
					    DeeObject_AssertTypeExact(data, &DeeString_Type))
						goto err;
					if (end) {
						dssize_t ibegin, iend;
						if (DeeObject_AsSSize(begin, &ibegin) ||
						    DeeObject_AsSSize(end, &iend))
							goto err;
						if ((size_t)iend > DeeString_SIZE(data) * sizeof(char))
							iend = (dssize_t)DeeString_SIZE(data) * sizeof(char);
						if ((size_t)ibegin >= (size_t)iend)
							return_reference_(&DeeInt_Zero);
						result = DeeFile_Write(self, DeeString_STR(data) + ibegin, (size_t)(iend - ibegin));
					} else if (begin) {
						size_t max_size;
						if (DeeObject_AsSize(begin, &max_size))
							goto err;
						if (max_size > DeeString_SIZE(data) * sizeof(char))
							max_size = DeeString_SIZE(data) * sizeof(char);
						result = DeeFile_Write(self, DeeString_STR(data), max_size);
					} else {
						result = DeeFile_Write(self, DeeString_STR(data),
						                       DeeString_SIZE(data) * sizeof(char));
					}
					if unlikely(result < 0)
						goto err;
					return DeeInt_NewSize((size_t)result);
				}

				case FILE_OPERATOR_SEEK: {
					doff_t off;
					int whence;
					whence = SEEK_SET;
					if (DeeArg_Unpack(argc, argv, "I64d|d:seek", &off, &whence))
						goto err;
					off = DeeFile_Seek(self, off, whence);
					if unlikely(off < 0)
						goto err;
					return DeeInt_NewU64((uint64_t)off);
				}	break;

				case FILE_OPERATOR_SYNC:
					if (DeeArg_Unpack(argc, argv, ":sync"))
						goto err;
					if (DeeFile_Sync(self))
						goto err;
					goto return_none_;

				case FILE_OPERATOR_TRUNC: {
					dpos_t length;
					if (DeeArg_Unpack(argc, argv, "I64u:trunc", &length))
						goto err;
					if (DeeFile_Trunc(self, length))
						goto err;
					goto return_none_;
				}

				case FILE_OPERATOR_CLOSE:
					if (DeeArg_Unpack(argc, argv, ":close"))
						goto err;
					if (DeeFile_Close(self))
						goto err;
					goto return_none_;

				case FILE_OPERATOR_PREAD: {
					DeeObject *a, *b;
					dpos_t pos;
					size_t max_bytes;
					b = NULL;
					/* TODO: Support for reading into buffers */
					if (DeeArg_Unpack(argc, argv, "o|o:__pread__", &a, &b))
						goto err;
					if (b) {
						if (DeeObject_AsSize(a, &max_bytes) ||
						    DeeObject_AsUInt64(b, &pos))
							goto err;
					} else {
						max_bytes = (size_t)-1;
						if (DeeObject_AsUInt64(a, &pos))
							goto err;
					}
					return DeeFile_PReadText(self, max_bytes, pos, false);
				}

				case FILE_OPERATOR_PWRITE: {
					DeeObject *data, *a, *b, *c;
					dssize_t result;
					dpos_t pos;
					b = c = NULL;
					if (DeeArg_Unpack(argc, argv, "oo|oo:__write__", &data, &a, &b, &c) ||
					    DeeObject_AssertTypeExact(data, &DeeString_Type))
						goto err;
					/* TODO: Support for writing buffers */
					if (c) {
						dssize_t ibegin, iend;
						if (DeeObject_AsSSize(a, &ibegin) ||
						    DeeObject_AsSSize(b, &iend) ||
						    DeeObject_AsUInt64(c, &pos))
							goto err;
						if ((size_t)iend > DeeString_SIZE(data) * sizeof(char))
							iend = (dssize_t)DeeString_SIZE(data) * sizeof(char);
						if ((size_t)ibegin >= (size_t)iend)
							return_reference_(&DeeInt_Zero);
						result = DeeFile_PWrite(self, DeeString_STR(data) + ibegin, (size_t)(iend - ibegin), pos);
					} else if (b) {
						size_t max_size;
						if (DeeObject_AsUInt64(b, &pos))
							goto err;
						if (DeeObject_AsSize(a, &max_size))
							goto err;
						if (max_size > DeeString_SIZE(data) * sizeof(char))
							max_size = DeeString_SIZE(data) * sizeof(char);
						result = DeeFile_PWrite(self, DeeString_STR(data), max_size, pos);
					} else {
						if (DeeObject_AsUInt64(a, &pos))
							goto err;
						result = DeeFile_PWrite(self, DeeString_STR(data),
						                        DeeString_SIZE(data) * sizeof(char),
						                        pos);
					}
					if unlikely(result < 0)
						goto err;
					return DeeInt_NewSize((size_t)result);
				}

				case FILE_OPERATOR_GETC: {
					int result;
					if (DeeArg_Unpack(argc, argv, ":__getc__"))
						goto err;
					result = DeeFile_Getc(self);
					if unlikely(result == GETC_ERR)
						goto err;
					return DeeInt_NewInt(result);
				}

				case FILE_OPERATOR_UNGETC: {
					int ch;
					if (DeeArg_Unpack(argc, argv, "d:__ungetc__", &ch))
						goto err;
					ch = DeeFile_Ungetc(self, ch);
					if unlikely(ch == GETC_ERR)
						goto err;
					return_bool_(ch != GETC_EOF);
				}

				case FILE_OPERATOR_PUTC: {
					int ch;
					if (DeeArg_Unpack(argc, argv, "d:__putc__", &ch))
						goto err;
					ch = DeeFile_Putc(self, ch);
					if unlikely(ch == GETC_ERR)
						goto err;
					return_bool_(ch != GETC_EOF);
				}

				default: break;
				}
			}
			if (info->oi_type == OPTYPE_SPECIAL)
				goto special_operator;
			if (info->oi_private)
				goto private_operator;
			if (info->oi_type & OPTYPE_INPLACE && !pself)
				goto requires_inplace;
		}
		DeeError_Throwf(&DeeError_TypeError,
		                "Cannot invoke unknown operator #%X", name);
		goto err;
special_operator:
		DeeError_Throwf(&DeeError_TypeError,
		                "Cannot invoke special `operator %s' (`__%s__') directly",
		                info->oi_uname, info->oi_sname);
		goto err;
private_operator:
		DeeError_Throwf(&DeeError_TypeError,
		                "Cannot invoke private `operator %s' (`__%s__') directly",
		                info->oi_uname, info->oi_sname);
		goto err;
requires_inplace:
		typetype = Dee_TYPE(self);
		if (typetype == &DeeSuper_Type)
			typetype = DeeSuper_TYPE(self);
		typetype = Dee_TYPE(typetype);
		info     = Dee_OperatorInfo(typetype, name);
		if likely(info) {
			DeeError_Throwf(&DeeError_TypeError,
			                "Cannot invoke inplace `operator %s' (`__%s__') without l-value",
			                info->oi_uname, info->oi_sname);
		} else {
			DeeError_Throwf(&DeeError_TypeError,
			                "Cannot invoke inplace operator #%X without l-value",
			                name);
		}
err:
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
	return invoke_operator(self, NULL, name, argc, argv);
}

PUBLIC DREF DeeObject *DCALL
DeeObject_PInvokeOperator(DeeObject **__restrict pself, uint16_t name,
                          size_t argc, DeeObject **__restrict argv) {
	ASSERT(pself);
	return invoke_operator(*pself, pself, name, argc, argv);
}



PRIVATE void *DCALL
DeeType_GetOpPointer(DeeTypeObject *__restrict self,
                     struct opinfo *__restrict info) {
	switch (info->oi_class) {

	case OPCLASS_TYPE:
		return *(void **)((uintptr_t)self + info->oi_offset);

	case OPCLASS_GC:
		if (!self->tp_gc)
			break;
		return *(void **)((uintptr_t)self->tp_gc + info->oi_offset);

	case OPCLASS_MATH:
		if (!self->tp_math)
			break;
		return *(void **)((uintptr_t)self->tp_math + info->oi_offset);

	case OPCLASS_CMP:
		if (!self->tp_cmp)
			break;
		return *(void **)((uintptr_t)self->tp_cmp + info->oi_offset);

	case OPCLASS_SEQ:
		if (!self->tp_seq)
			break;
		return *(void **)((uintptr_t)self->tp_seq + info->oi_offset);

	case OPCLASS_ATTR:
		if (!self->tp_attr)
			break;
		return *(void **)((uintptr_t)self->tp_attr + info->oi_offset);

	case OPCLASS_WITH:
		if (!self->tp_with)
			break;
		return *(void **)((uintptr_t)self->tp_with + info->oi_offset);

	case OPCLASS_BUFFER:
		if (!self->tp_buffer)
			break;
		return *(void **)((uintptr_t)self->tp_buffer + info->oi_offset);

	default:
		break; /* XXX: Extended operators? */
	}
	return NULL;
}

/* Check if `name' is being implemented by the given path, or has been inherited by a base-type. */
PUBLIC bool DCALL
DeeType_HasOperator(DeeTypeObject *__restrict self, uint16_t name) {
	struct opinfo *info;
	if (name == OPERATOR_CONSTRUCTOR) {
		/* Special case: the constructor operator (which cannot be inherited). */
		return (self->tp_init.tp_alloc.tp_ctor != NULL ||
		        self->tp_init.tp_alloc.tp_any_ctor != NULL ||
		        self->tp_init.tp_alloc.tp_any_ctor_kw != NULL);
	}
	info = Dee_OperatorInfo(self, name);
	if (info) {
		do {
			if (DeeType_GetOpPointer(self, info))
				return true;
		} while ((self = DeeType_Base(self)) != NULL);
	}
	return false;
}

/* Same as `DeeType_HasOperator()', however don't return `true' if the
 * operator has been inherited implicitly through caching mechanisms. */
PUBLIC bool DCALL
DeeType_HasPrivateOperator(DeeTypeObject *__restrict self, uint16_t name) {
	void *my_ptr;
	struct opinfo *info;
	if (name == OPERATOR_CONSTRUCTOR) {
		/* Special case: the constructor operator (which cannot be inherited). */
		return (self->tp_init.tp_alloc.tp_ctor != NULL ||
		        self->tp_init.tp_alloc.tp_any_ctor != NULL ||
		        self->tp_init.tp_alloc.tp_any_ctor_kw != NULL);
	}
	info = Dee_OperatorInfo(self, name);
	return (info != NULL &&
	        (my_ptr = DeeType_GetOpPointer(self, info)) != NULL &&
	        (!self->tp_base || my_ptr != DeeType_GetOpPointer(self->tp_base, info)));
}






typedef struct {
	OBJECT_HEAD
	DREF DeeTypeObject  *to_type; /* [1..1][const] The type who's operators should be enumerated. */
	ATOMIC_DATA uint16_t to_opid; /* Next operator ID to check. */
	bool                 to_name; /* [const] When true, try to assign human-readable names to operators. */
} TypeOperatorsIterator;

typedef struct {
	OBJECT_HEAD
	DREF DeeTypeObject *to_type; /* [1..1][const] The type who's operators should be enumerated. */
	bool                to_name; /* [const] When true, try to assign human-readable names to operators. */
} TypeOperators;

INTDEF DeeTypeObject TypeOperators_Type;
INTDEF DeeTypeObject TypeOperatorsIterator_Type;


PRIVATE void DCALL
to_fini(TypeOperators *__restrict self) {
	Dee_Decref_unlikely(self->to_type);
}

PRIVATE void DCALL
to_visit(TypeOperators *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->to_type);
}

STATIC_ASSERT(COMPILER_OFFSETOF(TypeOperatorsIterator,to_type) ==
              COMPILER_OFFSETOF(TypeOperators,to_type));
#define toi_fini  to_fini
#define toi_visit to_visit

PRIVATE DREF DeeObject *DCALL
to_str(TypeOperators *__restrict self) {
	return DeeString_Newf("%k.__operator%ss__",
	                      self->to_type,
	                      self->to_name ? "" : "id");
}

PRIVATE DREF TypeOperatorsIterator *DCALL
to_iter(TypeOperators *__restrict self) {
	DREF TypeOperatorsIterator *result;
	result = DeeObject_MALLOC(TypeOperatorsIterator);
	if unlikely(!result)
		goto done;
	result->to_type = self->to_type;
	result->to_opid = 0;
	result->to_name = self->to_name;
	Dee_Incref(self->to_type);
	DeeObject_Init(result, &TypeOperatorsIterator_Type);
done:
	return result;
}

PRIVATE DREF DeeObject *DCALL
to_contains(TypeOperators *__restrict self,
            DeeObject *__restrict name_or_id) {
	uint16_t id;
	if (DeeString_Check(name_or_id)) {
		id = Dee_OperatorFromName(Dee_TYPE(self->to_type),
		                          DeeString_STR(name_or_id));
		if (id == (uint16_t)-1)
			return_false;
	} else {
		if (DeeObject_AsUInt16(name_or_id, &id))
			goto err;
	}
	return_bool(DeeType_HasPrivateOperator(self->to_type, id));
err:
	return NULL;
}


PRIVATE struct type_seq to_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&to_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&to_contains
};

PRIVATE struct type_member to_class_members[] = {
	TYPE_MEMBER_CONST("Iterator",&TypeOperatorsIterator_Type),
	TYPE_MEMBER_END
};

#ifdef CONFIG_NO_THREADS
#define TOI_GETOPID(x)            ((x)->to_opid)
#else /* CONFIG_NO_THREADS */
#define TOI_GETOPID(x) ATOMIC_READ((x)->to_opid)
#endif /* !CONFIG_NO_THREADS */

PRIVATE int DCALL
toi_copy(TypeOperatorsIterator *__restrict self,
         TypeOperatorsIterator *__restrict other) {
	self->to_type = other->to_type;
	self->to_opid = TOI_GETOPID(other);
	self->to_name = other->to_name;
	Dee_Incref(self->to_type);
	return 0;
}

#define DEFINE_TOI_COMPARE(name, op)                                                    \
	PRIVATE DREF DeeObject *DCALL                                                       \
	name(TypeOperatorsIterator *__restrict self,                                        \
	     TypeOperatorsIterator *__restrict other) {                                     \
		if (DeeObject_AssertTypeExact((DeeObject *)other, &TypeOperatorsIterator_Type)) \
			goto err;                                                                   \
		return_bool(TOI_GETOPID(self) op TOI_GETOPID(other));                           \
	err:                                                                                \
		return NULL;                                                                    \
	}
DEFINE_TOI_COMPARE(toi_eq, ==)
DEFINE_TOI_COMPARE(toi_ne, !=)
DEFINE_TOI_COMPARE(toi_lo, <)
DEFINE_TOI_COMPARE(toi_le, <=)
DEFINE_TOI_COMPARE(toi_gr, >)
DEFINE_TOI_COMPARE(toi_ge, >=)
#undef DEFINE_TOI_COMPARE

PRIVATE struct type_cmp toi_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&toi_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&toi_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&toi_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&toi_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&toi_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&toi_ge,
};


PRIVATE DREF DeeObject *DCALL
toi_next(TypeOperatorsIterator *__restrict self) {
	DeeTypeObject *tp = self->to_type;
	struct opinfo *info;
	uint16_t result;
#ifndef CONFIG_NO_THREADS
	uint16_t start;
#endif /* !CONFIG_NO_THREADS */
#ifndef CONFIG_NO_THREADS
	for (;;)
#endif /* !CONFIG_NO_THREADS */
	{
#ifndef CONFIG_NO_THREADS
		result = start = ATOMIC_READ(self->to_opid);
#else /* !CONFIG_NO_THREADS */
		result        = self->to_opid;
#endif /* CONFIG_NO_THREADS */
		for (;; ++result) {
			void *my_ptr;
			/* Query information about the given operator. */
			info = Dee_OperatorInfo(Dee_TYPE(tp), result);
			if (result == OPERATOR_CONSTRUCTOR) {
				/* Special case: the constructor operator (which cannot be inherited). */
				if (tp->tp_init.tp_alloc.tp_ctor != NULL ||
				    tp->tp_init.tp_alloc.tp_any_ctor != NULL ||
				    tp->tp_init.tp_alloc.tp_any_ctor_kw != NULL)
					break;
				continue;
			}
			if (!info) {
				/* If there isn't an operator record, switch to extended operators. */
				if (result < OPERATOR_EXTENDED(0)) {
					result = OPERATOR_EXTENDED(0);
					continue;
				}
				/* If we already were within extended operators, then
				 * we know we've check all of them at this point, meaning
				 * we know that all operators have now been enumerated. */
				return ITER_DONE;
			}
			/* Check if this operator is implemented (though isn't inherited). */
			if ((my_ptr = DeeType_GetOpPointer(tp, info)) != NULL &&
			    (!tp->tp_base || my_ptr != DeeType_GetOpPointer(tp->tp_base, info)))
				break;
		}
#ifndef CONFIG_NO_THREADS
		if (ATOMIC_CMPXCH_WEAK(self->to_opid, start, result + 1))
			break;
#else /* !CONFIG_NO_THREADS */
		self->to_opid = result + 1;
#endif /* CONFIG_NO_THREADS */
	}
	if (self->to_name && *info->oi_sname)
		return DeeString_New(info->oi_sname);
	return DeeInt_NewU16(result);
}


INTERN DeeTypeObject TypeOperatorsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_TypeOperatorsIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ NULL,
				/* .tp_copy_ctor   = */ (void *)&toi_copy,
				/* .tp_deep_ctor   = */ NULL,
				/* .tp_any_ctor    = */ NULL,
				TYPE_FIXED_ALLOCATOR(TypeOperatorsIterator)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&toi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&toi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &toi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&toi_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject TypeOperators_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_TypeOperators",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ NULL,
				/* .tp_copy_ctor   = */ NULL,
				/* .tp_deep_ctor   = */ NULL,
				/* .tp_any_ctor    = */ NULL,
				TYPE_FIXED_ALLOCATOR(TypeOperators)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&to_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&to_str,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&to_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &to_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */to_class_members
};


INTERN DREF DeeObject *DCALL
type_get_operators(DeeTypeObject *__restrict self) {
	DREF TypeOperators *result;
	result = DeeObject_MALLOC(TypeOperators);
	if unlikely(!result)
		goto done;
	result->to_type = self;
	result->to_name = true;
	Dee_Incref(self);
	DeeObject_Init(result, &TypeOperators_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN DREF DeeObject *DCALL
type_get_operatorids(DeeTypeObject *__restrict self) {
	DREF TypeOperators *result;
	result = DeeObject_MALLOC(TypeOperators);
	if unlikely(!result)
		goto done;
	result->to_type = self;
	result->to_name = false;
	Dee_Incref(self);
	DeeObject_Init(result, &TypeOperators_Type);
done:
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_INFO_C */
