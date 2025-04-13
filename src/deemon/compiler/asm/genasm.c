/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    in a product, an acknowledgement (see the following) in the product     *
 *    documentation is required:                                              *
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_ASM_GENASM_C
#define GUARD_DEEMON_COMPILER_ASM_GENASM_C 1

#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/tpp.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/tuple.h>

#include "../../runtime/builtin.h"
/**/

#include <stdint.h> /* uint16_t */

#ifndef __INTELLISENSE__
#include "genasm-yield.c.inl"
#else /* !__INTELLISENSE__ */
DECL_BEGIN

PRIVATE WUNUSED NONNULL((1, 2)) int
(DCALL ast_genasm_yield)(struct ast *__restrict yieldexpr,
                         struct ast *__restrict ddi);

DECL_END
#endif /* __INTELLISENSE__ */

DECL_BEGIN

#define DO(expr) if unlikely(expr) goto err

STATIC_ASSERT(ASM_NOT & 1);
STATIC_ASSERT(!(ASM_BOOL & 1));
STATIC_ASSERT(ASM_BOOL == (ASM_NOT ^ 1));
STATIC_ASSERT((ASM_INCPOST & 0xff) == ASM_INC);
STATIC_ASSERT((ASM_DECPOST & 0xff) == ASM_DEC);
STATIC_ASSERT((ASM_INCPOST & 0xff00) == (ASM_DECPOST & 0xff00));

/* Quick translation table for most basic operator instruction codes. */
INTERN_CONST instruction_t const operator_instr_table[] = {
	/* [OPERATOR_CONSTRUCTOR] = */ 0,
	/* [OPERATOR_COPY]        = */ ASM_COPY,
	/* [OPERATOR_DEEPCOPY]    = */ ASM_DEEPCOPY,
	/* [OPERATOR_DESTRUCTOR]  = */ 0,
	/* [OPERATOR_ASSIGN]      = */ ASM_ASSIGN,
	/* [OPERATOR_MOVEASSIGN]  = */ ASM_MOVE_ASSIGN,
	/* [OPERATOR_STR]         = */ ASM_STR,
	/* [OPERATOR_REPR]        = */ ASM_REPR,
	/* [OPERATOR_BOOL]        = */ ASM_BOOL,
	/* [OPERATOR_ITERNEXT]    = */ 0, /* ASM_ITERNEXT - Extended opcodes can't be used here. */
	/* [OPERATOR_CALL]        = */ ASM_CALL_TUPLE,
	/* [OPERATOR_INT]         = */ ASM_CAST_INT,
	/* [OPERATOR_FLOAT]       = */ 0,
	/* [OPERATOR_INV]         = */ ASM_INV,
	/* [OPERATOR_POS]         = */ ASM_POS,
	/* [OPERATOR_NEG]         = */ ASM_NEG,
	/* [OPERATOR_ADD]         = */ ASM_ADD,
	/* [OPERATOR_SUB]         = */ ASM_SUB,
	/* [OPERATOR_MUL]         = */ ASM_MUL,
	/* [OPERATOR_DIV]         = */ ASM_DIV,
	/* [OPERATOR_MOD]         = */ ASM_MOD,
	/* [OPERATOR_SHL]         = */ ASM_SHL,
	/* [OPERATOR_SHR]         = */ ASM_SHR,
	/* [OPERATOR_AND]         = */ ASM_AND,
	/* [OPERATOR_OR]          = */ ASM_OR,
	/* [OPERATOR_XOR]         = */ ASM_XOR,
	/* [OPERATOR_POW]         = */ ASM_POW,
	/* [OPERATOR_INC]         = */ ASM_INC, /* Inplace! */
	/* [OPERATOR_DEC]         = */ ASM_DEC, /* Inplace! */
	/* [OPERATOR_INPLACE_ADD] = */ ASM_ADD, /* Inplace! */
	/* [OPERATOR_INPLACE_SUB] = */ ASM_SUB, /* Inplace! */
	/* [OPERATOR_INPLACE_MUL] = */ ASM_MUL, /* Inplace! */
	/* [OPERATOR_INPLACE_DIV] = */ ASM_DIV, /* Inplace! */
	/* [OPERATOR_INPLACE_MOD] = */ ASM_MOD, /* Inplace! */
	/* [OPERATOR_INPLACE_SHL] = */ ASM_SHL, /* Inplace! */
	/* [OPERATOR_INPLACE_SHR] = */ ASM_SHR, /* Inplace! */
	/* [OPERATOR_INPLACE_AND] = */ ASM_AND, /* Inplace! */
	/* [OPERATOR_INPLACE_OR]  = */ ASM_OR,  /* Inplace! */
	/* [OPERATOR_INPLACE_XOR] = */ ASM_XOR, /* Inplace! */
	/* [OPERATOR_INPLACE_POW] = */ ASM_POW, /* Inplace! */
	/* [OPERATOR_HASH]        = */ 0,
	/* [OPERATOR_EQ]          = */ ASM_CMP_EQ,
	/* [OPERATOR_NE]          = */ ASM_CMP_NE,
	/* [OPERATOR_LO]          = */ ASM_CMP_LO,
	/* [OPERATOR_LE]          = */ ASM_CMP_LE,
	/* [OPERATOR_GR]          = */ ASM_CMP_GR,
	/* [OPERATOR_GE]          = */ ASM_CMP_GE,
	/* [OPERATOR_ITER]        = */ ASM_ITERSELF,
	/* [OPERATOR_SIZE]        = */ ASM_GETSIZE,
	/* [OPERATOR_CONTAINS]    = */ ASM_CONTAINS,
	/* [OPERATOR_GETITEM]     = */ ASM_GETITEM,
	/* [OPERATOR_DELITEM]     = */ ASM_DELITEM,
	/* [OPERATOR_SETITEM]     = */ ASM_SETITEM,
	/* [OPERATOR_GETRANGE]    = */ ASM_GETRANGE,
	/* [OPERATOR_DELRANGE]    = */ ASM_DELRANGE,
	/* [OPERATOR_SETRANGE]    = */ ASM_SETRANGE,
	/* [OPERATOR_GETATTR]     = */ ASM_GETATTR,
	/* [OPERATOR_DELATTR]     = */ ASM_DELATTR,
	/* [OPERATOR_SETATTR]     = */ ASM_SETATTR,
	/* [OPERATOR_ENUMATTR]    = */ 0,
	/* [OPERATOR_ENTER]       = */ ASM_ENTER,
	/* [OPERATOR_LEAVE]       = */ ASM_LEAVE
};

INTERN_CONST uint8_t const operator_opcount_table[OPERATOR_USERCOUNT] = {
#define OPCOUNT_OPCOUNTMASK 0x0f
#define OPCOUNT_RESULTMASK  0xf0
#define OPCOUNT_INSTRIN     0x00 /* The instruction intrinsically pushes a result. */
#define OPCOUNT_PUSHFIRST   0x10 /* You must re-return the first operand. */
#define OPCOUNT_PUSHSECOND  0x20 /* You must re-return the second operand. */
#define OPCOUNT_PUSHTHIRD   0x30 /* You must re-return the third operand. */
#define OPCOUNT_PUSHFOURTH  0x40 /* You must re-return the fourth operand. */
#define OPCOUNT_POPPUSHNONE 0x70 /* You must pop one object, the push `none'. */
#define OPCOUNT_PUSHNONE    0x80 /* You must re-return none. */
#define ENTRY(push_mode, opcount) (push_mode | opcount)
	/* [OPERATOR_CONSTRUCTOR] = */ 0,
	/* [OPERATOR_COPY]        = */ ENTRY(OPCOUNT_INSTRIN, 1),
	/* [OPERATOR_DEEPCOPY]    = */ ENTRY(OPCOUNT_INSTRIN, 1),
	/* [OPERATOR_DESTRUCTOR]  = */ 0,
	/* [OPERATOR_ASSIGN]      = */ ENTRY(OPCOUNT_PUSHFIRST, 2),
	/* [OPERATOR_MOVEASSIGN]  = */ ENTRY(OPCOUNT_PUSHFIRST, 2),
	/* [OPERATOR_STR]         = */ ENTRY(OPCOUNT_INSTRIN, 1),
	/* [OPERATOR_REPR]        = */ ENTRY(OPCOUNT_INSTRIN, 1),
	/* [OPERATOR_BOOL]        = */ ENTRY(OPCOUNT_INSTRIN, 1),
	/* [OPERATOR_ITERNEXT]    = */ 0, /* ASM_ITERNEXT - Extended opcodes can't be used here. */
	/* [OPERATOR_CALL]        = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_INT]         = */ ENTRY(OPCOUNT_INSTRIN, 1),
	/* [OPERATOR_FLOAT]       = */ 0,
	/* [OPERATOR_INV]         = */ ENTRY(OPCOUNT_INSTRIN, 1),
	/* [OPERATOR_POS]         = */ ENTRY(OPCOUNT_INSTRIN, 1),
	/* [OPERATOR_NEG]         = */ ENTRY(OPCOUNT_INSTRIN, 1),
	/* [OPERATOR_ADD]         = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_SUB]         = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_MUL]         = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_DIV]         = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_MOD]         = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_SHL]         = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_SHR]         = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_AND]         = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_OR]          = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_XOR]         = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_POW]         = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_INC]         = */ ENTRY(OPCOUNT_PUSHFIRST, 1), /* Inplace! */
	/* [OPERATOR_DEC]         = */ ENTRY(OPCOUNT_PUSHFIRST, 1), /* Inplace! */
	/* [OPERATOR_INPLACE_ADD] = */ ENTRY(OPCOUNT_PUSHFIRST, 2), /* Inplace! */
	/* [OPERATOR_INPLACE_SUB] = */ ENTRY(OPCOUNT_PUSHFIRST, 2), /* Inplace! */
	/* [OPERATOR_INPLACE_MUL] = */ ENTRY(OPCOUNT_PUSHFIRST, 2), /* Inplace! */
	/* [OPERATOR_INPLACE_DIV] = */ ENTRY(OPCOUNT_PUSHFIRST, 2), /* Inplace! */
	/* [OPERATOR_INPLACE_MOD] = */ ENTRY(OPCOUNT_PUSHFIRST, 2), /* Inplace! */
	/* [OPERATOR_INPLACE_SHL] = */ ENTRY(OPCOUNT_PUSHFIRST, 2), /* Inplace! */
	/* [OPERATOR_INPLACE_SHR] = */ ENTRY(OPCOUNT_PUSHFIRST, 2), /* Inplace! */
	/* [OPERATOR_INPLACE_AND] = */ ENTRY(OPCOUNT_PUSHFIRST, 2), /* Inplace! */
	/* [OPERATOR_INPLACE_OR]  = */ ENTRY(OPCOUNT_PUSHFIRST, 2), /* Inplace! */
	/* [OPERATOR_INPLACE_XOR] = */ ENTRY(OPCOUNT_PUSHFIRST, 2), /* Inplace! */
	/* [OPERATOR_INPLACE_POW] = */ ENTRY(OPCOUNT_PUSHFIRST, 2), /* Inplace! */
	/* [OPERATOR_HASH]        = */ 0,
	/* [OPERATOR_EQ]          = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_NE]          = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_LO]          = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_LE]          = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_GR]          = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_GE]          = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_ITER]        = */ ENTRY(OPCOUNT_INSTRIN, 1),
	/* [OPERATOR_SIZE]        = */ ENTRY(OPCOUNT_INSTRIN, 1),
	/* [OPERATOR_CONTAINS]    = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_GETITEM]     = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_DELITEM]     = */ ENTRY(OPCOUNT_PUSHNONE, 2),
	/* [OPERATOR_SETITEM]     = */ ENTRY(OPCOUNT_PUSHTHIRD, 3),
	/* [OPERATOR_GETRANGE]    = */ ENTRY(OPCOUNT_INSTRIN, 3),
	/* [OPERATOR_DELRANGE]    = */ ENTRY(OPCOUNT_PUSHNONE, 3),
	/* [OPERATOR_SETRANGE]    = */ ENTRY(OPCOUNT_PUSHFOURTH, 4),
	/* [OPERATOR_GETATTR]     = */ ENTRY(OPCOUNT_INSTRIN, 2),
	/* [OPERATOR_DELATTR]     = */ ENTRY(OPCOUNT_PUSHNONE, 2),
	/* [OPERATOR_SETATTR]     = */ ENTRY(OPCOUNT_PUSHTHIRD, 3),
	/* [OPERATOR_ENUMATTR]    = */ 0,
	/* [OPERATOR_ENTER]       = */ ENTRY(OPCOUNT_POPPUSHNONE, 1),
	/* [OPERATOR_LEAVE]       = */ ENTRY(OPCOUNT_PUSHNONE, 1)
};

struct seqops {
	/* Opcodes are encoded in big-endian.
	 * When the mask 0xff00 is ZERO, the opcode is a single byte long. */
	DeeTypeObject *so_typ; /* The deemon type for this sequence. */
	uint16_t so_pck[2];    /* Pack - [0]: 8-bit; [1]: 16-bit; */
	uint16_t so_cas;       /* Cast */
};


/* Make sure that generic sequences are encoded as either
 * tuples or lists with used in non-specific contexts. */
STATIC_ASSERT((AST_FMULTIPLE_GENERIC & 3) <= 1);
STATIC_ASSERT((AST_FMULTIPLE_GENERIC_MAP & 3) == (AST_FMULTIPLE_DICT & 3));

INTDEF struct seqops seqops_info[4];
INTERN struct seqops seqops_info[4] = {
	/* [AST_FMULTIPLE_TUPLE   & 3] = */ { &DeeTuple_Type,   { ASM_PACK_TUPLE,   ASM16_PACK_TUPLE   }, ASM_CAST_TUPLE   },
	/* [AST_FMULTIPLE_LIST    & 3] = */ { &DeeList_Type,    { ASM_PACK_LIST,    ASM16_PACK_LIST    }, ASM_CAST_LIST    },
	/* [AST_FMULTIPLE_HASHSET & 3] = */ { &DeeHashSet_Type, { ASM_PACK_HASHSET, ASM16_PACK_HASHSET }, ASM_CAST_HASHSET },
	/* [AST_FMULTIPLE_DICT    & 3] = */ { &DeeDict_Type,    { ASM_PACK_DICT,    ASM16_PACK_DICT    }, ASM_CAST_DICT    }
};

/* @param: type: One of `AST_FMULTIPLE_*' */
PRIVATE int DCALL pack_sequence(uint16_t type, uint16_t num_args) {
	uint16_t (*p_opcode)[2], op;
	ASSERT(type != AST_FMULTIPLE_KEEPLAST);
	if (type == AST_FMULTIPLE_GENERIC && num_args == 1)
		return asm_gpack_one();
	if (num_args == 0) {
		switch (type) {
		case AST_FMULTIPLE_GENERIC:
			return asm_gpush_constexpr(Dee_EmptySeq);
		case AST_FMULTIPLE_GENERIC_SET:
			return asm_gpush_constexpr(Dee_EmptySet);
		case AST_FMULTIPLE_GENERIC_MAP:
			return asm_gpush_constexpr(Dee_EmptyMapping);
		default: break;
		}
	}
	if (AST_FMULTIPLE_ISMAP(type)) {
		/* Special case: Dict. */
		if (num_args & 1)
			DO(asm_gpop()); /* Discard superfluous element. */
		num_args /= 2;
		asm_subsp(num_args); /* Adjust for the second half. */
	}
	p_opcode = &seqops_info[type & 3].so_pck;
	if (num_args > UINT8_MAX) {
		op = (*p_opcode)[1];
		if (op & 0xff00)
			DO(asm_put((op & 0xff00) >> 8));
		DO(asm_putimm16(op & 0xff, num_args));
	} else {
		op = (*p_opcode)[0];
		if (op & 0xff00)
			DO(asm_put((op & 0xff00) >> 8));
		DO(asm_putimm8(op & 0xff, (uint8_t)num_args));
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
	if (op & 0xff00)
		DO(asm_put((op & 0xff00) >> 8));
	return asm_put(op & 0xff);
err:
	return -1;
}




/* The heart of the compiler: The AST --> Assembly generator. */
INTERN WUNUSED NONNULL((1)) int
(DCALL ast_genasm)(struct ast *__restrict self,
                   unsigned int gflags) {
#define PUSH_RESULT (gflags & ASM_G_FPUSHRES)
	ASSERT_AST(self);
	/* Set the given AST as location information for error messages. */
	ASM_PUSH_LOC(&self->a_ddi);
	ASM_PUSH_SCOPE(self->a_scope, err);
	switch (self->a_type) {

	case AST_CONSTEXPR:
		if (!PUSH_RESULT)
			break;
		DO(asm_putddi(self));
		DO(asm_gpush_constexpr(self->a_constexpr));
		break;

	case AST_SYM: {
		struct symbol *sym;
		sym = self->a_sym;
		SYMBOL_INPLACE_UNWIND_ALIAS(sym);
		/* Make sure to still generate code if the symbol access could have side-effects. */
		if (!PUSH_RESULT &&
		    !symbol_get_haseffect(sym, self->a_scope))
			break;
		DO(asm_putddi(self));
		if (gflags & ASM_G_FLAZYBOOL) {
			if (DeeBaseScope_IsVarargs(current_basescope, sym)) {
				/* Special case: If the caller accesses the varargs-symbol in a boolean-context,
				 *               then we can simply check if the number of varargs is non-zero,
				 *               emulating the behavior of tuple's `operator bool()'. */
				DO(asm_gcmp_gr_varargs_sz(0));
				goto done;
			}
			if (DeeBaseScope_IsVarkwds(current_basescope, sym)) {
				/* Special case: If the caller accesses the varkwds-symbol in a boolean-context.
				 * NOTE: Don't do this when optimizing for size, as `push bool varkwds' takes
				 *       one additional byte of text when compared to `push varkwds' */
				if (!(current_assembler.a_flag & ASM_FOPTIMIZE_SIZE)) {
					DO(asm_gbool_varkwds());
					goto done;
				}
			}
		}
		DO(asm_gpush_symbol(sym, self));
		goto pop_unused;
	}	break;

	case AST_UNBIND: {
		struct symbol *sym;
		DO(asm_putddi(self));
		sym = SYMBOL_UNWIND_ALIAS(self->a_unbind);
		DO(asm_gdel_symbol(sym, self));
		/* NOTE: Only make the symbol ID be available again, when
		 *       the symbol itself was declared in the same scope,
		 *       as the one it is being deleted from. */
		if (sym->s_type == SYMBOL_TYPE_LOCAL &&
		    sym->s_scope == self->a_scope) {
			asm_dellocal(sym->s_symid);
			sym->s_flag &= ~SYMBOL_FALLOC;
		}
done_push_none:
		if (PUSH_RESULT)
			DO(asm_gpush_none());
	}	break;

	case AST_BOUND: {
		struct symbol *sym = self->a_sym;
		SYMBOL_INPLACE_UNWIND_ALIAS(sym);
		if (!PUSH_RESULT && !symbol_bnd_haseffect(sym, self->a_scope))
			break;
		DO(asm_putddi(self));
		DO(asm_gpush_bnd_symbol(sym, self));
		goto pop_unused;
	}

	case AST_MULTIPLE: {
		unsigned int need_all;
		bool expand_encountered;
		struct ast **iter, **end;
		uint16_t active_size;
		int error;
		end = (iter = self->a_multiple.m_astv) +
		      self->a_multiple.m_astc;
		if unlikely(iter == end) {
			/* Special case: empty multiple list. */
			if (!PUSH_RESULT)
				goto done;
			DO(asm_putddi(self));
			if (self->a_flag == AST_FMULTIPLE_KEEPLAST) {
				/* Simply push `none' */
				DO(asm_gpush_none());
			} else {
				/* Must push an empty sequence. */
				if (self->a_flag == AST_FMULTIPLE_HASHSET) {
					error = asm_gpack_hashset(0);
				} else if (self->a_flag == AST_FMULTIPLE_LIST) {
					error = asm_gpack_list(0);
				} else if (self->a_flag == AST_FMULTIPLE_TUPLE) {
					error = asm_gpack_tuple(0);
				} else if (self->a_flag == AST_FMULTIPLE_DICT) {
					error = asm_gpack_dict(0);
				} else if (AST_FMULTIPLE_ISMAP(self->a_flag)) {
					error = asm_gpush_constexpr(Dee_EmptyMapping);
				} else if (AST_FMULTIPLE_ISSET(self->a_flag)) {
					error = asm_gpush_constexpr(Dee_EmptySet);
				} else {
					error = asm_gpush_constexpr(Dee_EmptySeq);
				}
				if unlikely(error)
					goto err;
			}
			goto done;
		}

#if 0 /* This breaks. Not because the result is a proxy (if you want to ensure that the elements
       * of a sequence aren't the result of a proxy, you must call ".frozen"), but because
       * "(x as Sequence).operator iter()" isn't implemented (because Sequence doesn't implement
       * that operator and expects its sub-classes to implement it *for* it) */
		/* `{ foo... }' normally compiles as:
		 * >> push   @foo
		 * >> cast   top, Tuple
		 *
		 * However (when not optimizing for size), this can be done more efficiently
		 * such that we compile the expression as `foo as Sequence from deemon' instead:
		 * >> push   @foo
		 * >> push   extern @deemon:@Sequence
		 * >> super  top, pop
		 *
		 * The resulting assembly can then execute in O(1), but would still comply
		 * with the (only) requirement of generic sequence expressions being that the
		 * returned object be derived from `Sequence from deemon'!
		 *
		 * Something similar could be done for practically all generic sequence expression
		 * containing expand expressions by wrapping them in yield-functions:
		 * >> { 10, foo..., 20, bar..., 30 }
		 * Could be compiled as:
		 * >> () -> { yield 10; yield foo...; yield 20; yield bar...; yield 30; }()
		 * With the construction of the resulting sequence then being O(1) as well.
		 * However, this method would introduce a lot of unavoidable overhead that
		 * would likely be far greater than the overhead of iterating foo and bar
		 * during construction, so this ~optimization~ would probably not work in
		 * favor of program speed...
		 */
		if (self->a_multiple.m_astc == 1 &&
		    !(current_assembler.a_flag & ASM_FOPTIMIZE_SIZE) &&
		    self->a_flag == AST_FMULTIPLE_GENERIC &&
		    (*iter)->a_type == AST_EXPAND && PUSH_RESULT) {
			int32_t deemon_modid;
			struct ast *expandexpr = (*iter)->a_expand;
			DO(ast_genasm(expandexpr, ASM_G_FPUSHRES));
			deemon_modid = asm_newmodule(DeeModule_GetDeemon());
			if unlikely(deemon_modid < 0)
				goto err;
			DO(asm_putddi(self));
			DO(asm_gpush_extern((uint16_t)deemon_modid, id_Sequence));
			DO(asm_gsuper());
			break;
		}
#endif

		/* When `need_all' is true, we must push the results of all elements onto the stack. */
		need_all    = (self->a_flag != AST_FMULTIPLE_KEEPLAST) ? PUSH_RESULT : ASM_G_FNORMAL;
		active_size = 0;
		expand_encountered = false;
		for (; iter < end; ++iter) {
			struct ast *elem = *iter;
			/* Only need to push the last element when _we_ are supposed to push our result. */
			unsigned int need_this;
			need_this = need_all ? need_all : (iter == end - 1 ? gflags : ASM_G_FNORMAL);
			if (elem->a_type == AST_EXPAND && need_all) {
				if (active_size) {
					DO(asm_putddi(self));
					if (expand_encountered) {
						DO(asm_gextend(active_size));
					} else {
						DO(pack_sequence(self->a_flag, active_size));
					}
				}
				error = ast_genasm_one(elem->a_expand, ASM_G_FPUSHRES);
				if unlikely(error)
					goto err;
				if (active_size || expand_encountered) {
					/* Concat the old an new parts. */
					DO(asm_putddi(self));
					DO(asm_gconcat());
				} else {
					/* The AST starts with an expand expression.
					 * Because of that, we have to make sure that the entire
					 * branch gets the correct type by casting now. */
					DeeTypeObject *expected_type = seqops_info[self->a_flag & 3].so_typ;
					if ((expected_type == &DeeTuple_Type /* Immutable sequence type */ ||
					     !ast_predict_object_shared(elem->a_expand)) &&
					    (ast_predict_type(elem->a_expand) == expected_type)) {
						/* Sequence type is immutable, or not shared */
					} else {
						DO(asm_putddi(self));
						DO(cast_sequence(self->a_flag));
					}
				}
				expand_encountered = true;
				active_size        = 0;
			} else {
				if (need_all) {
					if unlikely(active_size >= UINT16_MAX) {
						PERRAST(self, W_ASM_SEQUENCE_TOO_LONG);
						goto err;
					}
					++active_size;
				}
				error = self->a_flag != AST_FMULTIPLE_KEEPLAST
				        ? ast_genasm_one(elem, need_this)
				        : ast_genasm(elem, need_this);
			}
			if unlikely(error)
				goto err;
		}
		if (need_all) {
			if (active_size) {
				/* Pack together a sequence, as requested by the caller. */
				DO(asm_putddi(self));
				/* In case we were packing an sequence containing expand expressions,
				 * we must still concat this part with the one before that. */
				if (expand_encountered) {
					DO(asm_gextend(active_size));
				} else {
					DO(pack_sequence(self->a_flag, active_size));
				}
			}
		}
	}	break;

	case AST_RETURN:
		if (!self->a_return ||
		    /* NOTE: Don't optimize `return none' --> `return' in yield functions.
		     *       When yielding, the `ASM_RET_NONE' instruction behaves differently
		     *       from what a regular `ASM_RET' for `Dee_None' does! */
		    (!(current_basescope->bs_flags & CODE_FYIELDING) &&
		     self->a_return->a_type == AST_CONSTEXPR &&
		     DeeNone_Check(self->a_return->a_constexpr))) {
			DO(asm_putddi(self));
			DO(asm_gunwind());
			DO(asm_gret_none());
		} else if (self->a_return->a_type == AST_SYM &&
		           asm_can_prefix_symbol_for_read(self->a_return->a_sym)) {
			DO(asm_putddi(self));
			DO(asm_gunwind());
			DO(asm_gprefix_symbol_for_read(self->a_return->a_sym, self->a_return));
			DO(asm_gret_p());
		} else {
			DO(ast_genasm(self->a_return, ASM_G_FPUSHRES));
			DO(asm_putddi(self));
			DO(asm_gunwind());
			DO(asm_gret());
		}
		/* NOTE: We're not responsible to perform dead-code elimination.
		 *       The only thing we're responsible for, is to prevent
		 *       generation of trailing code still associated with
		 *       instructions that are known to never return.
		 *       As is the case now, we only fake pushing something when
		 *       a `return' expression is supposed to yield something. */
done_fake_none:
		if (PUSH_RESULT)
			asm_incsp();
		break;

	case AST_YIELD:
		DO(ast_genasm_yield(self->a_yield, self));
		goto done_push_none;

	case AST_THROW:
		if (!self->a_throw) {
			DO(asm_putddi(self));
			DO(asm_grethrow());
		} else if (self->a_throw->a_type == AST_SYM &&
		           asm_can_prefix_symbol_for_read(self->a_throw->a_sym)) {
			DO(asm_putddi(self));
			DO(asm_gprefix_symbol_for_read(self->a_throw->a_sym, self->a_throw));
			DO(asm_gthrow_p());
		} else {
			DO(ast_genasm(self->a_throw, ASM_G_FPUSHRES));
			DO(asm_putddi(self));
			DO(asm_gthrow());
		}
		goto done_fake_none;

	case AST_TRY:
		DO(asm_gentry(self, gflags));
		break;

	case AST_LOOP: {
		struct asm_sym *loop_break;
		loop_break = asm_genloop(self->a_flag,
		                         self->a_loop.l_elem,
		                         self->a_loop.l_iter,
		                         self->a_loop.l_loop,
		                         self);
		if unlikely(!loop_break)
			goto err;

		/* Reset the stack depth to its original value.
			 * NOTE: The current value may be distorted and invalid due to
			 *       stack variables having been initialized inside the loop. */
		ASM_BREAK_SCOPE(0, err);

		/* This is where `break' jumps to. (After the re-aligned stack) */
		asm_defsym(loop_break);

		/* Loop expressions simply return `none'. */
		/* Because we've already cleaned up after stack variables initialized
			 * within the loop's scope, we must not allow the code below to do so again! */
		if (PUSH_RESULT)
			DO(asm_gpush_none());
		goto done_noalign;
	}	break;

	case AST_LOOPCTL: {
		struct asm_sym *loopsym;
		uint16_t old_stack;
		ASSERT(self->a_flag == AST_FLOOPCTL_BRK ||
		       self->a_flag == AST_FLOOPCTL_CON);
#if AST_FLOOPCTL_BRK == ASM_LOOPCTL_BRK && AST_FLOOPCTL_CON == ASM_LOOPCTL_CON
		loopsym = current_assembler.a_loopctl[self->a_flag];
#else /* AST_FLOOPCTL_BRK == ASM_LOOPCTL_BRK && AST_FLOOPCTL_CON == ASM_LOOPCTL_CON */
		loopsym = current_assembler.a_loopctl[self->a_flag == AST_FLOOPCTL_BRK ? ASM_LOOPCTL_BRK : ASM_LOOPCTL_CON];
#endif /* AST_FLOOPCTL_BRK != ASM_LOOPCTL_BRK || AST_FLOOPCTL_CON != ASM_LOOPCTL_CON */
		if unlikely(!loopsym) {
			DO(WARNAST(self, W_ASM_BREAK_OR_CONTINUE_NOT_ALLOWED));
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
		 *    push the address of `print "done";', and before entering
		 *    the finally block during normal code-flow, the address of
		 *    `print "next";' is pushed instead.
		 *    HINT: The implementation of this is further documented in `AST_TRY' above.
		 */
		old_stack = current_assembler.a_stackcur;
		/* Adjust the stack and jump to the proper symbol. */
		DO(asm_putddi(self));
		if (current_assembler.a_finsym &&
		    !(current_assembler.a_finflag & ASM_FINFLAG_NOLOOP)) {
			/* Special case: Must push `ls_sym', but jump to `ls_fsym'. */
			current_assembler.a_finflag |= ASM_FINFLAG_USED;
			DO(asm_gpush_abs(loopsym));
			DO(asm_gpush_stk(loopsym));
			DO(asm_gjmps(current_assembler.a_finsym));
		} else {
			DO(asm_gjmps(loopsym));
		}
		current_assembler.a_stackcur = old_stack;
		goto done_fake_none;
	}	break;

	case AST_CONDITIONAL: {
		bool invert_condition;
		struct ast *condition;
		ASSERT_AST(self->a_conditional.c_cond);
		ASSERTF(self->a_conditional.c_tt ||
		        self->a_conditional.c_ff,
		        "At least one branch must exist");
		ASSERTF((self->a_conditional.c_tt != self->a_conditional.c_cond) ||
		        (self->a_conditional.c_ff != self->a_conditional.c_cond),
		        "At most one branch can equal the conditional branch");
		invert_condition = false;

		/* Special handling for boolean conditions.
		 * NOTE: Be careful about condition re-use for this one! */
		if (self->a_conditional.c_cond->a_type == AST_BOOL &&
		    self->a_conditional.c_tt != self->a_conditional.c_cond &&
		    self->a_conditional.c_ff != self->a_conditional.c_cond) {
			invert_condition = (self->a_conditional.c_cond->a_flag & AST_FBOOL_NEGATE);
			condition        = self->a_conditional.c_cond->a_bool;
		} else {
			condition = self->a_conditional.c_cond;
		}

		if (self->a_conditional.c_tt &&
		    self->a_conditional.c_ff) {
			unsigned int cond_flags = ASM_G_FPUSHRES | ASM_G_FLAZYBOOL;

			if (!(self->a_flag & AST_FCOND_BOOL)) {
				struct ast *replace_this_1 = self->a_conditional.c_ff;
				struct ast *replace_this_2;
				DeeObject *if_equal_to = Dee_None;
				struct ast *if_equal_to_ast = NULL;
				struct ast *with_this = self->a_conditional.c_tt;
				if (invert_condition) {
					struct ast *temp = replace_this_1;
					replace_this_1 = with_this;
					with_this = temp;
				}

				/* Check for special cases that can be encoded using the "cmpxch" instruction:
				 * >> a1 is none ? b : a2
				 * >> a1 is type(none) ? b : a2
				 * >> a1 === <if_equal_to> ? b : a2
				 * >> a1 !== <if_equal_to> ? a2 : b
				 * >> <if_equal_to> === a1 ? b : a2
				 * >> <if_equal_to> !== a1 ? a2 : b
				 * Where "b" is a constant expression, and a1 and a2 are either the exact
				 * same AST, or both refer to the same stack/local symbol. */
				if (condition->a_type == AST_ACTION &&
				    condition->a_flag == AST_FACTION_IS &&
				    ((condition->a_action.a_act1->a_type == AST_CONSTEXPR &&
				      (condition->a_action.a_act1->a_constexpr == Dee_None ||
				       condition->a_action.a_act1->a_constexpr == (DeeObject *)&DeeNone_Type)) ||
				     (condition->a_action.a_act1->a_type == AST_ACTION &&
				      condition->a_action.a_act1->a_flag == AST_FACTION_TYPEOF &&
				      condition->a_action.a_act1->a_action.a_act0->a_type == AST_CONSTEXPR &&
				      condition->a_action.a_act1->a_action.a_act0->a_constexpr == Dee_None))) {
					/* replace_this_alias is none */
					/* replace_this_alias is type(none) */
					replace_this_2 = condition->a_action.a_act0;
do_check_encode_cmpxch:
#define BRANCHES_IDENTICAL(a, b) \
	((a) == (b) || (ast_equal(a, b) && !ast_has_sideeffects(b)))
					if (BRANCHES_IDENTICAL(replace_this_1, replace_this_2)) {
do_encode_cmpxch_or_reuse_replace_this_branch:
						if ((if_equal_to_ast && BRANCHES_IDENTICAL(if_equal_to_ast, with_this)) ||
						    (if_equal_to && ast_isconstexpr(with_this, if_equal_to))) {
							/* Special case: `x === y ? y : x'.
							 * Always evaluates to "x", but evaluates "y" as well. */
							DO(ast_genasm(replace_this_1, gflags));
							DO(if_equal_to_ast && ast_genasm_one(if_equal_to_ast, gflags & ~ASM_G_FPUSHRES));
							break;
						} else if ((if_equal_to_ast && BRANCHES_IDENTICAL(if_equal_to_ast, replace_this_1)) ||
						           (if_equal_to && ast_isconstexpr(replace_this_1, if_equal_to))) {
							/* Special case: `y === y ? with_this : ...'.
							 * -> replacement always happens */
							DO(if_equal_to_ast && ast_genasm(if_equal_to_ast, gflags & ~ASM_G_FPUSHRES));
							DO(ast_genasm(with_this, gflags));
							break;
						} else if (!ast_has_sideeffects(with_this)) {
							/* Generate code:
							 * >> push @replace_this_1
							 * >> push @if_equal_to[_ast]
							 * >> push @with_this
							 * >> cmpxch top, pop, pop */
							DO(ast_genasm(replace_this_1, gflags));
							if (PUSH_RESULT) {
								if (if_equal_to && DeeNone_Check(if_equal_to)) {
									if (with_this->a_type == AST_CONSTEXPR) {
										int32_t cid = asm_newconst(with_this->a_constexpr);
										if unlikely(cid < 0)
											goto err;
										DO(asm_putddi(self));
										DO(asm_gcmpxch_top_none_c((uint16_t)cid));
									} else {
										DO(ast_genasm_one(with_this, gflags));
										DO(asm_putddi(self));
										DO(asm_gcmpxch_top_none_pop());
									}
								} else {
									DO(if_equal_to_ast ? ast_genasm_one(if_equal_to_ast, gflags)
									                   : asm_gpush_constexpr(if_equal_to));
									if (with_this->a_type == AST_CONSTEXPR &&
									    DeeNone_Check(with_this->a_constexpr)) {
										DO(asm_putddi(self));
										DO(asm_gcmpxch_top_pop_none());
									} else {
										DO(ast_genasm_one(with_this, gflags));
										DO(asm_putddi(self));
										DO(asm_gcmpxch_top_pop_pop());
									}
								}
							}
							break;
						} else {
							/* Generate code:
							 * >>     push @replace_this_1        # replace_this_1
							 * >>     dup                         # replace_this_1, replace_this_1
							 * >>     push @if_equal_to[_ast]     # replace_this_1, replace_this_1, if_equal_to
							 * >>     push cmp  so, pop, pop      # replace_this_1, is_same
							 * >>     jf   pop, 1f                # replace_this_1
							 * >>     pop                         # N/A
							 * >>     push @with_this             # with_this     (this push is known to have side-effects, meaning it can't be optimized away)
							 * >> 1:                              # replace_this_1|with_this
							 */
							DO(ast_genasm(replace_this_1, gflags | ASM_G_FPUSHRES));
							if (PUSH_RESULT)
								DO(asm_gdup());
							if (if_equal_to && DeeNone_Check(if_equal_to)) {
								DO(asm_putddi(condition));
								DO(asm_gisnone());
							} else {
								DO(if_equal_to_ast ? ast_genasm_one(if_equal_to_ast, gflags)
								                   : asm_gpush_constexpr(if_equal_to));
								DO(asm_putddi(condition));
								DO(asm_gsameobj());
							}
							DO(asm_putddi(self));
							if (((with_this == self->a_conditional.c_tt && (self->a_flag & AST_FCOND_UNLIKELY)) ||
							     (with_this == self->a_conditional.c_ff && (self->a_flag & AST_FCOND_LIKELY))) &&
							    (current_assembler.a_curr != &current_assembler.a_sect[SECTION_COLD]) &&
							    !(current_assembler.a_flag & ASM_FOPTIMIZE_SIZE)) {
								/* User explicitly stated that it is unlikely that the value needs to be replaced.
								 * In this case, generate slightly different code:
								 * >> // BEGIN: ALREADY GENERATED
								 * >>     push @replace_this_1        # replace_this_1
								 * >>     dup                         # replace_this_1, replace_this_1
								 * >>     push @if_equal_to[_ast]     # replace_this_1, replace_this_1, if_equal_to
								 * >>     push cmp  so, pop, pop      # replace_this_1, is_same
								 * >> // END: ALREADY GENERATED
								 * >>     jt   pop, 1f                # replace_this_1
								 * >> .section .cold
								 * >> 1:  pop                         # N/A
								 * >>     push @with_this             # with_this     (this push is known to have side-effects, meaning it can't be optimized away)
								 * >>     jmp  1f
								 * >> .section .text
								 * >> 1:                              # replace_this_1|with_this
								 */
								struct asm_sym *enter_cold_sym;
								struct asm_sym *leave_cold_sym;
								struct asm_sec *prev_section;
								enter_cold_sym = asm_newsym();
								if unlikely(!enter_cold_sym)
									goto err;
								leave_cold_sym = asm_newsym();
								if unlikely(!leave_cold_sym)
									goto err;
								DO(asm_gjmp(ASM_JT, enter_cold_sym));
								prev_section = current_assembler.a_curr;
								current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
								asm_decsp();
								asm_defsym(enter_cold_sym);
								if (PUSH_RESULT) {
									DO(asm_putddi(with_this));
									DO(asm_gpop());
								}
								DO(ast_genasm_one(with_this, gflags));
								DO(asm_gjmp(ASM_JMP, leave_cold_sym));
								current_assembler.a_curr = prev_section;
								asm_defsym(leave_cold_sym);
							} else {
								struct asm_sym *expr_end;
								expr_end = asm_newsym();
								if unlikely(!expr_end)
									goto err;
								DO(asm_gjmp(ASM_JF, expr_end));
								asm_decsp();
								if (PUSH_RESULT) {
									DO(asm_putddi(with_this));
									DO(asm_gpop());
								}
								DO(ast_genasm_one(with_this, gflags));
								asm_defsym(expr_end);
							}
							break;
						}
					}
				} else if (condition->a_type == AST_ACTION) {
					if (condition->a_flag == AST_FACTION_DIFFOBJ) {
						struct ast *temp = replace_this_1;
						replace_this_1 = with_this;
						with_this = temp;
						goto do_check_encode_cmpxch_for_sameobj_condition;
					} else if (condition->a_flag == AST_FACTION_SAMEOBJ) {
do_check_encode_cmpxch_for_sameobj_condition:
						if (condition->a_action.a_act0->a_type == AST_CONSTEXPR) {
							if_equal_to    = condition->a_action.a_act0->a_constexpr;
							replace_this_2 = condition->a_action.a_act1;
							goto do_check_encode_cmpxch;
						} else if (condition->a_action.a_act1->a_type == AST_CONSTEXPR) {
							if_equal_to    = condition->a_action.a_act1->a_constexpr;
							replace_this_2 = condition->a_action.a_act0;
							goto do_check_encode_cmpxch;
						} else if (BRANCHES_IDENTICAL(replace_this_1, condition->a_action.a_act0)) {
							if_equal_to_ast = condition->a_action.a_act1;
							if_equal_to     = NULL;
							goto do_encode_cmpxch_or_reuse_replace_this_branch;
						} else if (BRANCHES_IDENTICAL(replace_this_1, condition->a_action.a_act1)) {
							if_equal_to_ast = condition->a_action.a_act0;
							if_equal_to     = NULL;
							goto do_encode_cmpxch_or_reuse_replace_this_branch;
						}
					}
				}
			}


			/* If the condition expression is re-used, we can't
			 * have it auto-optimize itself into a boolean value
			 * if the caller expects the real expression value. */
			if (!(gflags & ASM_G_FLAZYBOOL) &&
			    (self->a_conditional.c_tt == self->a_conditional.c_cond ||
			     self->a_conditional.c_ff == self->a_conditional.c_cond))
				cond_flags &= ~ASM_G_FLAZYBOOL;
			DO(ast_genasm(condition, cond_flags));

			/* Branch with specific code for both paths. */
			if (self->a_conditional.c_tt == self->a_conditional.c_cond ||
			    self->a_conditional.c_ff == self->a_conditional.c_cond) {
				struct asm_sym *cond_end;
				/* Special case: re-use the condition as true or false branch. */

				/* If the condition will be re-used as result, and `AST_FCOND_BOOL' is set, we
				 * must first convert the conditional into a boolean if it's not already one. */
				DO(asm_putddi(self));
				if (PUSH_RESULT && (self->a_flag & AST_FCOND_BOOL) &&
				    ast_predict_type(condition) != &DeeBool_Type) {
					/* Force the condition to become a boolean. */
					DO(asm_gbool(invert_condition));
					invert_condition = false;
				}

				/*     push <cond>
				 *     [dup]
				 *     jt   pop, 1f  # Inverted by `invert_condition ^ (ast->a_conditional.c_ff == ast->a_conditional.c_cond)'
				 *     [pop]
				 *     [push] <false-branch> / <true-branch>
				 *1:   */
				cond_end = asm_newsym();
				if unlikely(!cond_end)
					goto err;
				if (PUSH_RESULT)
					DO(asm_gdup());
				if (self->a_conditional.c_ff == self->a_conditional.c_cond)
					invert_condition = !invert_condition;
				if (self->a_flag & (AST_FCOND_LIKELY | AST_FCOND_UNLIKELY) &&
				    current_assembler.a_curr != &current_assembler.a_sect[SECTION_COLD]) {
					/*     push <cond>
					 *     [dup]
					 *     jf   pop, .cold.1f  # Inverted by `invert_condition ^ (ast->a_conditional.c_ff == ast->a_conditional.c_cond)'
					 *2:
					 *
					 *.cold.1:
					 *     pop
					 *     [push] <false-branch> / <true-branch>
					 *     jmp   2b */
					struct asm_sym *cold_entry = asm_newsym();
					struct asm_sec *prev_section;
					struct ast *genast;
					if unlikely(!cold_entry)
						goto err;
					DO(asm_putddi(self));
					DO(asm_gjmp(invert_condition ? ASM_JT : ASM_JF, cold_entry));
					asm_decsp();
					prev_section             = current_assembler.a_curr;
					current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
					DO(asm_gpop());
					genast = self->a_conditional.c_tt != self->a_conditional.c_cond
					         ? self->a_conditional.c_tt
					         : self->a_conditional.c_ff;
					DO(ast_genasm(genast, (self->a_flag & AST_FCOND_BOOL) ? (gflags | ASM_G_FLAZYBOOL) : gflags));
					if (PUSH_RESULT && (self->a_flag & AST_FCOND_BOOL) &&
					    ast_predict_type(genast) != &DeeBool_Type)
						DO(asm_gbool(false));
					current_assembler.a_curr = prev_section;
				} else {
					struct ast *genast;
					DO(asm_gjmp(invert_condition ? ASM_JF : ASM_JT, cond_end));
					asm_decsp();
					if (PUSH_RESULT)
						DO(asm_gpop());
					genast = self->a_conditional.c_tt != self->a_conditional.c_cond
					         ? self->a_conditional.c_tt
					         : self->a_conditional.c_ff;
					DO(ast_genasm(genast, (self->a_flag & AST_FCOND_BOOL) ? (gflags | ASM_G_FLAZYBOOL) : gflags));
					if (PUSH_RESULT && (self->a_flag & AST_FCOND_BOOL) &&
					    ast_predict_type(genast) != &DeeBool_Type)
						DO(asm_gbool(false));
				}
				asm_defsym(cond_end);
			} else if (self->a_flag & (AST_FCOND_LIKELY | AST_FCOND_UNLIKELY) &&
			           current_assembler.a_curr != &current_assembler.a_sect[SECTION_COLD]) {
				/* Special case where one of the branches is placed in cold text. */
				/*     push <cond>
				 *     jf   pop, .cold.1f  # Inverted by `invert_condition ^ <likely-branch == false-branch>'
				 *    [push] <likely-branch>
				 *2:
				 *
				 *.cold.1:
				 *    [push] <unlikely-branch>
				 *     jmp   2b */
				struct ast *likely_branch, *unlikely_branch;
				struct asm_sym *cold_entry;
				struct asm_sym *text_return;
				struct asm_sec *prev_section;
				bool likely_is_bool, unlikely_is_bool;
				cold_entry = asm_newsym();
				if unlikely(!cold_entry)
					goto err;
				text_return = asm_newsym();
				if unlikely(!text_return)
					goto err;
				if (self->a_flag & AST_FCOND_LIKELY) {
					likely_branch   = self->a_conditional.c_tt;
					unlikely_branch = self->a_conditional.c_ff;
				} else {
					unlikely_branch  = self->a_conditional.c_tt;
					likely_branch    = self->a_conditional.c_ff;
					invert_condition = !invert_condition;
				}
				unlikely_is_bool = !PUSH_RESULT ||
				                   !(self->a_flag & AST_FCOND_BOOL);
				likely_is_bool = unlikely_is_bool;
				if (!likely_is_bool)
					likely_is_bool = ast_predict_type(likely_branch) == &DeeBool_Type;
				if (!unlikely_is_bool)
					unlikely_is_bool = ast_predict_type(unlikely_branch) == &DeeBool_Type;

				DO(asm_putddi(self));
				DO(asm_gjmp(invert_condition ? ASM_JT : ASM_JF, cold_entry));
				asm_decsp();
				DO(ast_genasm(likely_branch, gflags));
				if (!likely_is_bool && unlikely_is_bool)
					DO(asm_gbool(false));

				/* Pre-adjust the stack for anything the other branch may do. */
				if (current_assembler.a_flag & ASM_FSTACKDISP)
					DO(asm_gsetstack_s(text_return));

				/* Compile the unlikely code. */
				prev_section             = current_assembler.a_curr;
				current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
				if (PUSH_RESULT)
					asm_decsp(); /* Adjust stack depth to before the likely-branch. */
				asm_defsym(cold_entry);
				DO(ast_genasm(unlikely_branch, gflags));
				DO(asm_putddi(self));
				if (likely_is_bool && !unlikely_is_bool)
					DO(asm_gbool(false));

				/* Return to regular text. */
				DO(asm_gjmp(ASM_JMP, text_return));
				current_assembler.a_curr = prev_section;

				asm_defsym(text_return);
				if (!likely_is_bool && !unlikely_is_bool)
					DO(asm_gbool(false));
			} else {
				/*     push <cond>
				 *     jf   pop, 1f  # Inverted by `invert_condition'
				 *     [push] <true-branch>
				 *     jmp  pop, 2f
				 *1:   [push] <false-branch>
				 *2:   */
				struct asm_sym *ff_enter;
				struct asm_sym *ff_leave;
				bool tt_is_bool, ff_is_bool;
				ff_enter = asm_newsym();
				if unlikely(!ff_enter)
					goto err;
				ff_leave = asm_newsym();
				if unlikely(!ff_leave)
					goto err;
				tt_is_bool = ff_is_bool = !PUSH_RESULT || !(self->a_flag & AST_FCOND_BOOL);
				if (!tt_is_bool)
					tt_is_bool = ast_predict_type(self->a_conditional.c_tt) == &DeeBool_Type;
				if (!ff_is_bool)
					ff_is_bool = ast_predict_type(self->a_conditional.c_ff) == &DeeBool_Type;
				DO(asm_putddi(self));
				DO(asm_gjmp(invert_condition ? ASM_JT : ASM_JF, ff_enter));
				asm_decsp(); /* Popped by `ASM_JT' / `ASM_JF' */
				DO(ast_genasm(self->a_conditional.c_tt, gflags));
				DO(asm_putddi(self));
				if (!tt_is_bool && ff_is_bool)
					DO(asm_gbool(false));
				DO(asm_gjmp(ASM_JMP, ff_leave));
				if (PUSH_RESULT)
					asm_decsp(); /* Adjust to before `tt' was executed. */
				asm_defsym(ff_enter);
				DO(ast_genasm(self->a_conditional.c_ff, gflags));
				DO(asm_putddi(self));
				if (tt_is_bool && !ff_is_bool)
					DO(asm_gbool(false));
				asm_defsym(ff_leave);
				if (!tt_is_bool && !ff_is_bool)
					DO(asm_gbool(false));
			}
		} else {
			/* Only the one of the branches exists. - the other should return `none'. */
			struct ast *existing_branch;
			bool invert_boolean = invert_condition;
			ASSERT(self->a_conditional.c_tt || self->a_conditional.c_ff);
			ASSERT(!self->a_conditional.c_tt || !self->a_conditional.c_ff);
			existing_branch = self->a_conditional.c_tt;
			if (!existing_branch) {
				existing_branch  = self->a_conditional.c_ff;
				invert_condition = !invert_condition;
			}
			if (existing_branch == self->a_conditional.c_cond) {
				if (!PUSH_RESULT) {
					DO(ast_genasm(condition, ASM_G_FNORMAL));
					goto done;
				}

				/* Special case: The only existing branch is a duplicate of the condition. */
				/*    push <cond>
				 *    [bool]
				 *    dup
				 *    jf   pop, 1f  # Inverted by `existing_branch == ast->a_conditional.c_ff'
				 *    pop
				 *    push none
				 *1: */
				DO(ast_genasm(condition, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				if (PUSH_RESULT && (self->a_flag & AST_FCOND_BOOL) &&
				    ast_predict_type(condition) != &DeeBool_Type) {
					/* Force the condition to become a boolean. */
					DO(asm_gbool(invert_boolean));
					invert_condition ^= invert_boolean;
				}

				DO(asm_gdup());

				/* Do a bit of hacking to prevent creation of a pointless relocation. */
				DO(asm_putimm8(invert_condition ? ASM_JT : ASM_JF,
				               sizeof(instruction_t) * 2));
				DO(asm_put(ASM_PUSH_NONE));
				DO(asm_put(ASM_POP));
				asm_decsp();
			} else {
				/*   [push none|false]
				 *    push <cond>
				 *   [bool]
				 *    jf   pop, 1f    # Inverted by `existing_branch == ast->a_conditional.c_ff'
				 *   [pop]
				 *   [push] <existing-branch>
				 *1: */
				struct asm_sym *after_existing = asm_newsym();
				if unlikely(!after_existing)
					goto err;
				if (PUSH_RESULT) {
					/* Due to stack displacement, the conditional may leave more
					 * than just its return value on the stack, meaning that
					 * we cannot rely on our `none' remaining immediately below
					 * when stack displacement is enabled.
					 * Instead, we must rely on peephole optimization to then optimize
					 * text like this:
					 * >> push @condition
					 * >> push none
					 * >> swap
					 * ... into this:
					 * >> push none
					 * >> push @condition
					 * HINT: This uses the same facility that optimizes
					 *      `a in b' --> `b.operator contains(a)'.
					 */
					if (current_assembler.a_flag & ASM_FSTACKDISP) {
						DO(ast_genasm(condition, ASM_G_FPUSHRES));
					}
					DO(asm_putddi(self));
					if (self->a_flag & AST_FCOND_BOOL) {
						DO(asm_gpush_constexpr(Dee_False));
					} else {
						DO(asm_gpush_none());
					}
					if (current_assembler.a_flag & ASM_FSTACKDISP) {
						DO(asm_gswap());
					} else {
						DO(ast_genasm(condition, ASM_G_FPUSHRES));
					}
					DO(asm_putddi(self));
					if (self->a_flag & AST_FCOND_BOOL &&
					    ast_predict_type(condition) != &DeeBool_Type)
						DO(asm_gbool(false));
					DO(asm_gjmp(invert_condition ? ASM_JT : ASM_JF, after_existing));
					asm_decsp(); /* Adjust for the value popped by `ASM_JT' / `ASM_JF' */
				} else {
					DO(asm_gjcc(condition,
					            invert_condition ? ASM_JT : ASM_JF,
					            after_existing, self));
				}
				if (PUSH_RESULT)
					DO(asm_gpop());
				DO(ast_genasm(existing_branch, gflags));
				asm_defsym(after_existing);
			}
		}
	}	break;

	case AST_BOOL: {
		instruction_t instr;
#if AST_FBOOL_NEGATE == 1
		instr = ASM_BOOL ^ (self->a_flag & AST_FBOOL_NEGATE);
#else /* AST_FBOOL_NEGATE == 1 */
		instr = ASM_BOOL ^ (instruction_t) !!(self->a_flag & AST_FBOOL_NEGATE);
#endif /* AST_FBOOL_NEGATE != 1 */
		DO(ast_genasm(self->a_bool, ASM_G_FLAZYBOOL | ASM_G_FPUSHRES));

		/* If the result won't be used, no need to do any inversion.
		 * However, we must still invoke the bool operator of the
		 * top-object, just in case doing so has any side-effects. */
		if (!PUSH_RESULT)
			instr = ASM_BOOL;
		if (instr == ASM_BOOL) {
			if (gflags & ASM_G_FLAZYBOOL)
				break;
			if (ast_predict_type(self->a_bool) == &DeeBool_Type)
				break;
		}
		DO(asm_putddi(self));
		DO(asm_put(instr));
		goto pop_unused;
	}	break;

	case AST_EXPAND:
		if (PUSH_RESULT) {
			/* Expand to a single object by default. */
			DO(asm_gunpack_expr(self->a_expand, 1, self));
		} else {
			/* TODO: Don't generate any code for constant expressions
			 *       when their enumeration doesn't have any side-effects! */

			/* If the result isn't being used, follow the regular comma-rule
			 * to generate expected assembly for code like this:
			 * >> foo()...;
			 * Same as:
			 * >> for (none: foo()); // Force enumeration
			 */
			struct asm_sym *loop, *stop;
			DO(ast_genasm(self->a_expand, ASM_G_FPUSHRES));

			/* Generate code to enumerate the sequence */
			DO(asm_putddi(self));
			DO(asm_giterself());
			loop = asm_newsym();
			if unlikely(!loop)
				goto err;
			stop = asm_newsym();
			if unlikely(!stop)
				goto err;
			asm_defsym(loop);
			DO(asm_put(ASM_FOREACH));
			DO(asm_putrel(R_DMN_DISP8, stop, 0));
			DO(asm_put((instruction_t)(uint8_t)(int8_t) - 1));
			asm_incsp();
			DO(asm_gpop()); /* The sequence element pushed by `foreach' */
			DO(asm_put(ASM_JMP));
			DO(asm_putrel(R_DMN_DISP8, loop, 0));
			DO(asm_put((instruction_t)(uint8_t)(int8_t) - 1));
			asm_decsp(); /* FOREACH pops 1 element when jumping here */
			asm_defsym(stop);
		}
		break;

	case AST_FUNCTION:
		if (!PUSH_RESULT)
			break;
		DO(asm_gpush_function(self));
		break;

	case AST_OPERATOR_FUNC:
		if (PUSH_RESULT) {
			/* Only need to generate the operator function binding if
			 * the result of the expression is actually being used! */
			DO(ast_gen_operator_func(self->a_operator_func.of_binding,
			                         self, self->a_flag));
		} else if (self->a_operator_func.of_binding) {
			/* Still generate code the binding-expression (in case it has side-effects) */
			DO(ast_genasm(self->a_operator_func.of_binding, ASM_G_FNORMAL));
		}
		break;

	case AST_OPERATOR: {
		Dee_operator_t operator_name;

		/* Probably one of the most important AST types: The operator AST. */
		operator_name = self->a_flag;

		/* Special case: The arguments of the operator are variadic. */
		if unlikely(self->a_operator.o_exflag & AST_OPERATOR_FVARARGS) {
			struct symbol *prefix_symbol;
			struct opinfo const *info;
			info = DeeTypeType_GetOperatorById(&DeeType_Type, operator_name);
			if (self->a_operator.o_op0->a_type == AST_SYM &&
			    (!info || (info->oi_cc & OPCC_FINPLACE))) {
				/* Generate a prefixed instruction. */
				prefix_symbol = self->a_operator.o_op0->a_sym;
				if ((self->a_operator.o_exflag & AST_OPERATOR_FMAYBEPFX) &&
				    !asm_can_prefix_symbol(prefix_symbol))
					goto varop_without_prefix;
			} else {
varop_without_prefix:
				DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
				prefix_symbol = NULL;
			}

			/* Compile operand tuple or push an empty one. */
			if (self->a_operator.o_op1) {
				DO(ast_genasm_one(self->a_operator.o_op1, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
			} else {
				DO(asm_putddi(self));
				DO(asm_gpack_tuple(0));
			}
			if (prefix_symbol) {
				DO(asm_gprefix_symbol(prefix_symbol, self->a_operator.o_op0));
				DO(asm_ginplace_operator_tuple(operator_name));
			} else {
				DO(asm_goperator_tuple(operator_name));
			}
pop_unused:
			if (!PUSH_RESULT)
				DO(asm_gpop());
			break;
		}
		switch (operator_name) {

			/* Special instruction encoding for call operations. */
		case OPERATOR_CALL:
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			DO(asm_gcall_expr(self->a_operator.o_op0,
			                  self->a_operator.o_op1,
			                  self, gflags));
			goto done;

		case OPERATOR_GETITEM: {
			DeeObject *index;
			int32_t temp;
			struct ast *sequence;
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			sequence = self->a_operator.o_op0;
			for (;;) {
				struct ast *inner;
				if (sequence->a_type != AST_MULTIPLE)
					break;
				if (sequence->a_flag == AST_FMULTIPLE_KEEPLAST)
					break;
				if (sequence->a_multiple.m_astc != 1)
					break;
				inner = sequence->a_multiple.m_astv[0];
				if (inner->a_type != AST_EXPAND)
					break;
				sequence = inner->a_expand;
			}
			if (sequence->a_type == AST_SYM) {
				struct symbol *sym = sequence->a_sym;
				SYMBOL_INPLACE_UNWIND_ALIAS(sym);
				if (DeeBaseScope_IsVarargs(current_basescope, sym)) {
					uint8_t va_index;
					if (!PUSH_RESULT) {
						DO(ast_genasm(self->a_operator.o_op1, ASM_G_FNORMAL));
						goto done;
					}

					/* Lookup a varargs-argument by index. */
					if (self->a_operator.o_op1->a_type == AST_CONSTEXPR &&
					    DeeInt_Check(self->a_operator.o_op1->a_constexpr) &&
					    DeeInt_TryAsUInt8(self->a_operator.o_op1->a_constexpr, &va_index)) {
						DO(asm_putddi(self));
						DO(asm_gvarargs_getitem_i((uint8_t)va_index));
						goto done;
					}
					DO(ast_genasm(self->a_operator.o_op1, ASM_G_FPUSHRES));
					DO(asm_putddi(self));
					DO(asm_gvarargs_getitem());
					goto done;
				}
			}

			if (self->a_operator.o_op1->a_type != AST_CONSTEXPR)
				break;
			index = self->a_operator.o_op1->a_constexpr;

			/* Special optimizations for integer indices. */
			if (DeeInt_Check(index) &&
			    DeeInt_TryAsInt32(index, &temp) &&
			    temp >= INT16_MIN && temp <= INT16_MAX) {
				DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(asm_ggetitem_index((int16_t)temp));
				goto pop_unused;
			}

			/* Special optimizations for constant indices. */
			if (asm_allowconst(index)) {
				temp = asm_newconst(index);
				if unlikely(temp < 0)
					goto err;
				DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(asm_ggetitem_const((uint16_t)temp));
				goto pop_unused;
			}
		}	break;

		case OPERATOR_GETRANGE: {
			struct ast *begin, *end;
			int32_t intval;
			if unlikely(!self->a_operator.o_op2)
				goto generic_operator;
			DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
			begin = self->a_operator.o_op1;
			end   = self->a_operator.o_op2;
			if (begin->a_type == AST_CONSTEXPR) {
				DeeObject *const_begin = begin->a_constexpr;
				if (end->a_type == AST_CONSTEXPR) {
					DeeObject *const_end = end->a_constexpr;
					if (DeeInt_Check(const_begin) &&
					    DeeInt_TryAsInt32(const_begin, &intval) &&
					    intval >= INT16_MIN && intval <= INT16_MAX) {
						int32_t endval;
						if (DeeNone_Check(const_end)) {
							DO(asm_putddi(self));
							DO(asm_ggetrange_in((int16_t)intval));
							goto pop_unused;
						}
						if (DeeInt_Check(const_end) &&
						    DeeInt_TryAsInt32(const_end, &endval) &&
						    endval >= INT16_MIN && endval <= INT16_MAX) {
							DO(asm_putddi(self));
							DO(asm_ggetrange_ii((int16_t)intval, (int16_t)endval));
							goto pop_unused;
						}
					} else if (DeeNone_Check(const_begin) &&
					           DeeInt_Check(const_end) &&
					           DeeInt_TryAsInt32(const_end, &intval) &&
					           intval >= INT16_MIN && intval <= INT16_MAX) {
						DO(asm_putddi(self));
						DO(asm_ggetrange_ni((int16_t)intval));
						goto pop_unused;
					}
				}
				if (DeeNone_Check(const_begin)) {
					DO(ast_genasm_one(end, ASM_G_FPUSHRES));
					DO(asm_putddi(self));
					DO(asm_ggetrange_np());
					goto pop_unused;
				}
				if (DeeInt_Check(const_begin) &&
				    DeeInt_TryAsInt32(const_begin, &intval) &&
				    intval >= INT16_MIN && intval <= INT16_MAX) {
					DO(ast_genasm_one(end, ASM_G_FPUSHRES));
					DO(asm_putddi(self));
					DO(asm_ggetrange_ip((int16_t)intval));
					goto pop_unused;
				}
			} else if (end->a_type == AST_CONSTEXPR) {
				DeeObject *const_end = end->a_constexpr;
				if (DeeNone_Check(const_end)) {
					DO(ast_genasm_one(begin, ASM_G_FPUSHRES));
					DO(asm_putddi(self));
					DO(asm_ggetrange_pn());
					goto pop_unused;
				}
				if (DeeInt_Check(const_end) &&
				    DeeInt_TryAsInt32(const_end, &intval) &&
				    intval >= INT16_MIN && intval <= INT16_MAX) {
					DO(ast_genasm_one(begin, ASM_G_FPUSHRES));
					DO(asm_putddi(self));
					DO(asm_ggetrange_pi((int16_t)intval));
					goto pop_unused;
				}
			}
			DO(ast_genasm_one(begin, ASM_G_FPUSHRES));
			DO(ast_genasm_one(end, ASM_G_FPUSHRES));
			DO(asm_putddi(self));
			DO(asm_ggetrange());
			goto pop_unused;
		}

		case OPERATOR_SETITEM:
			if unlikely(!self->a_operator.o_op2)
				goto generic_operator;
			DO(ast_gen_setitem(self->a_operator.o_op0,
			                   self->a_operator.o_op1,
			                   self->a_operator.o_op2,
			                   self, gflags));
			goto done;

		case OPERATOR_GETATTR:
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			DO(ast_gen_getattr(self->a_operator.o_op0,
			                   self->a_operator.o_op1,
			                   self, gflags));
			goto done;

		case OPERATOR_DELATTR:
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			DO(ast_gen_delattr(self->a_operator.o_op0,
			                   self->a_operator.o_op1,
			                   self));
			goto done_push_none;

		case OPERATOR_SETATTR:
			if unlikely(!self->a_operator.o_op2)
				goto generic_operator;
			DO(ast_gen_setattr(self->a_operator.o_op0,
			                   self->a_operator.o_op1,
			                   self->a_operator.o_op2,
			                   self, gflags));
			goto done;

		/* Arithmetic-with-constant-operand optimizations. */
		case OPERATOR_ADD:
		case OPERATOR_SUB: {
			int32_t intval;
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			if (self->a_operator.o_op1->a_type != AST_CONSTEXPR)
				break;
			if (!DeeInt_Check(self->a_operator.o_op1->a_constexpr))
				break;
			if (DeeInt_TryAsInt32(self->a_operator.o_op1->a_constexpr, &intval) &&
			    (intval >= INT8_MIN && intval <= INT8_MAX)) {
				DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(operator_name == OPERATOR_ADD ? asm_gadd_simm8((int8_t)intval)
				                                 : asm_gsub_simm8((int8_t)intval));
				goto pop_unused;
			}
			if (DeeInt_TryAsUInt32(self->a_operator.o_op1->a_constexpr,
			                    (uint32_t *)&intval)) {
				DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(operator_name == OPERATOR_ADD ? asm_gadd_imm32(*(uint32_t *)&intval)
				                                 : asm_gsub_imm32(*(uint32_t *)&intval));
				goto pop_unused;
			}
		}	break;

		case OPERATOR_INPLACE_ADD:
		case OPERATOR_INPLACE_SUB: {
			int32_t intval;
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			if (self->a_operator.o_op1->a_type != AST_CONSTEXPR)
				break;
			if (self->a_operator.o_op0->a_type != AST_SYM)
				break;
			if (!asm_can_prefix_symbol(self->a_operator.o_op0->a_sym))
				break;
			if (!DeeInt_Check(self->a_operator.o_op1->a_constexpr))
				break;
			if (DeeInt_TryAsInt32(self->a_operator.o_op1->a_constexpr, &intval) &&
			    (intval >= INT8_MIN && intval <= INT8_MAX)) {
				DO(asm_gprefix_symbol(self->a_operator.o_op0->a_sym,
				                      self->a_operator.o_op0));
				DO(asm_putddi(self));
				DO(operator_name == OPERATOR_INPLACE_ADD ? asm_gadd_inplace_simm8((int8_t)intval)
				                                         : asm_gsub_inplace_simm8((int8_t)intval));
push_a_if_used:
				DO(PUSH_RESULT && ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
				goto done;
			}
			if (DeeInt_TryAsUInt32(self->a_operator.o_op1->a_constexpr,
			                    (uint32_t *)&intval)) {
				DO(asm_gprefix_symbol(self->a_operator.o_op0->a_sym,
				                      self->a_operator.o_op0));
				DO(asm_putddi(self));
				DO(operator_name == OPERATOR_INPLACE_ADD ? asm_gadd_inplace_imm32(*(uint32_t *)&intval)
				                                         : asm_gsub_inplace_imm32(*(uint32_t *)&intval));
				goto push_a_if_used;
			}
		}	break;

		case OPERATOR_MUL:
		case OPERATOR_DIV:
		case OPERATOR_MOD: {
			int32_t intval;
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			if (self->a_operator.o_op1->a_type != AST_CONSTEXPR)
				break;
			if (!DeeInt_Check(self->a_operator.o_op1->a_constexpr))
				break;
			if (DeeInt_TryAsInt32(self->a_operator.o_op1->a_constexpr, &intval) &&
			    (intval >= INT8_MIN && intval <= INT8_MAX)) {
				DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				if (operator_name == OPERATOR_MUL) {
					DO(asm_gmul_simm8((int8_t)intval));
				} else if (operator_name == OPERATOR_DIV) {
					DO(asm_gdiv_simm8((int8_t)intval));
				} else {
					DO(asm_gmod_simm8((int8_t)intval));
				}
				goto pop_unused;
			}
		}	break;

		case OPERATOR_INPLACE_MUL:
		case OPERATOR_INPLACE_DIV:
		case OPERATOR_INPLACE_MOD: {
			int32_t intval;
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			if (self->a_operator.o_op1->a_type != AST_CONSTEXPR)
				break;
			if (self->a_operator.o_op0->a_type != AST_SYM)
				break;
			if (!asm_can_prefix_symbol(self->a_operator.o_op0->a_sym))
				break;
			if (!DeeInt_Check(self->a_operator.o_op1->a_constexpr))
				break;
			if (DeeInt_TryAsInt32(self->a_operator.o_op1->a_constexpr, &intval) &&
			    (intval >= INT8_MIN && intval <= INT8_MAX)) {
				DO(asm_putddi(self));
				DO(asm_gprefix_symbol(self->a_operator.o_op0->a_sym,
				                      self->a_operator.o_op0));
				if (operator_name == OPERATOR_INPLACE_MUL) {
					DOC(asm_gmul_inplace_simm8((int8_t)intval));
				} else if (operator_name == OPERATOR_INPLACE_DIV) {
					DOC(asm_gdiv_inplace_simm8((int8_t)intval));
				} else {
					DOC(asm_gmod_inplace_simm8((int8_t)intval));
				}
				goto push_a_if_used;
			}
		}	break;

		case OPERATOR_AND:
		case OPERATOR_OR:
		case OPERATOR_XOR: {
			uint32_t intval;
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			if (self->a_operator.o_op1->a_type != AST_CONSTEXPR)
				break;
			if (!DeeInt_Check(self->a_operator.o_op1->a_constexpr))
				break;
			if (DeeInt_TryAsUInt32(self->a_operator.o_op1->a_constexpr, &intval)) {
				DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				if (operator_name == OPERATOR_AND) {
					DO(asm_gand_imm32(intval));
				} else if (operator_name == OPERATOR_OR) {
					DO(asm_gor_imm32(intval));
				} else {
					DO(asm_gxor_imm32(intval));
				}
				goto pop_unused;
			}
		}	break;

		case OPERATOR_INPLACE_AND:
		case OPERATOR_INPLACE_OR:
		case OPERATOR_INPLACE_XOR: {
			uint32_t intval;
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			if (self->a_operator.o_op1->a_type != AST_CONSTEXPR)
				break;
			if (self->a_operator.o_op0->a_type != AST_SYM)
				break;
			if (!asm_can_prefix_symbol(self->a_operator.o_op0->a_sym))
				break;
			if (!DeeInt_Check(self->a_operator.o_op1->a_constexpr))
				break;
			if (DeeInt_TryAsUInt32(self->a_operator.o_op1->a_constexpr, &intval)) {
				DO(asm_putddi(self));
				DO(asm_gprefix_symbol(self->a_operator.o_op0->a_sym,
				                      self->a_operator.o_op0));
				if (operator_name == OPERATOR_INPLACE_AND) {
					DO(asm_gand_inplace_imm32(intval));
				} else if (operator_name == OPERATOR_INPLACE_OR) {
					DO(asm_gor_inplace_imm32(intval));
				} else {
					DO(asm_gxor_inplace_imm32(intval));
				}
				goto push_a_if_used;
			}
		}	break;

		case OPERATOR_ENTER: {
			struct ast *enter_expr;
			enter_expr = self->a_operator.o_op0;
			if (enter_expr->a_type == AST_SYM) {
				struct symbol *sym = SYMBOL_UNWIND_ALIAS(enter_expr->a_sym);
				if (sym->s_type == SYMBOL_TYPE_STACK &&
				    !SYMBOL_MUST_REFERENCE_TYPEMAY(sym) &&
				    (sym->s_flag & SYMBOL_FALLOC) &&
				    SYMBOL_STACK_OFFSET(sym) == current_assembler.a_stackcur - 1) {
					/* Special optimization: Since `ASM_ENTER' doesn't modify the top stack item,
					 * if the operand _is_ the top stack item, then we can simply generate the enter
					 * instruction without the need of any kludge. */
					DO(asm_putddi(self));
					DO(asm_genter());
					goto done_push_none;
				}
			}
			DO(ast_genasm(enter_expr, ASM_G_FPUSHRES));
			DO(asm_putddi(self));
			DO(asm_genter());
			DO(asm_gpop());
			goto done_push_none;
		}

		case OPERATOR_LEAVE:
			/* NOTE: The case of the operand being stack-top, in which
			 *       case `dup; leave pop; pop;' is generated, will later
			 *       be optimized away by the peephole optimizer. */
			DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
			DO(asm_putddi(self));
			DO(asm_gleave());
			goto done_push_none;

		case OPERATOR_ITERNEXT:
			DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
			DO(asm_putddi(self));
			DO(asm_giternext()); /* This one's an extended instruction... */
			goto pop_unused;

		case OPERATOR_SIZE:
			if unlikely(!self->a_operator.o_op0)
				goto generic_operator;
			if ((current_basescope->bs_flags & CODE_FVARARGS) &&
			    self->a_operator.o_op0->a_type == AST_SYM) {
				struct symbol *sym = self->a_operator.o_op0->a_sym;
				SYMBOL_INPLACE_UNWIND_ALIAS(sym);
				if (DeeBaseScope_IsVarargs(current_basescope, sym)) {
					/* Special case: Get the size of varargs. */
					if (!PUSH_RESULT)
						goto done;
					DO(asm_putddi(self));
					if ((gflags & ASM_G_FLAZYBOOL) &&
					    !(current_assembler.a_flag & ASM_FOPTIMIZE_SIZE)) {
						/* When not optimizing for size, we can compare the number
						 * of varargs against 1 and push the inverted result. */
						DO(asm_gcmp_gr_varargs_sz(0));
					} else {
						/* NOTE: If optional arguments are being used, don't go this
						 *       route because we'd need a lot of overhead to get the
						 *       number of variable arguments minus the number of optional
						 *       arguments. */
						DO(asm_ggetsize_varargs());
					}
					goto done;
				}
				if (DeeBaseScope_IsVarkwds(current_basescope, sym) &&
				    (gflags & ASM_G_FLAZYBOOL)) {
					if (!PUSH_RESULT)
						goto done;
					DO(asm_putddi(self));
					DO(asm_gbool_varkwds());
					goto done;
				}
			}
			break;

		case OPERATOR_EQ:
		case OPERATOR_NE: {
			struct symbol *sym;
			struct ast *sizeast;
			DeeObject *sizeval;
			uint8_t va_size_val;
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			/* Check for special case:
			 * >> if (#varargs == 42) ...
			 * >> if (42 == #varargs) ...
			 * There is a dedicated instruction for comparing the size of varargs.
			 */
			if unlikely(!(current_basescope->bs_flags & CODE_FVARARGS))
				break;
			if (self->a_operator.o_op0->a_type == AST_OPERATOR) {
				if (self->a_operator.o_op0->a_flag != OPERATOR_SIZE)
					break;
				if (self->a_operator.o_op0->a_operator.o_exflag &
				    (AST_OPERATOR_FPOSTOP | AST_OPERATOR_FVARARGS))
					break;
				if (!self->a_operator.o_op0->a_operator.o_op0)
					break;
				if (self->a_operator.o_op0->a_operator.o_op0->a_type != AST_SYM)
					break;
				sym     = self->a_operator.o_op0->a_operator.o_op0->a_sym;
				sizeast = self->a_operator.o_op1;
			} else {
				if (self->a_operator.o_op1->a_type != AST_OPERATOR)
					break;
				if (self->a_operator.o_op1->a_flag != OPERATOR_SIZE)
					break;
				if (self->a_operator.o_op1->a_operator.o_exflag &
				    (AST_OPERATOR_FPOSTOP | AST_OPERATOR_FVARARGS))
					break;
				if (!self->a_operator.o_op1->a_operator.o_op0)
					break;
				if (self->a_operator.o_op1->a_operator.o_op0->a_type != AST_SYM)
					break;
				sym     = self->a_operator.o_op1->a_operator.o_op0->a_sym;
				sizeast = self->a_operator.o_op0;
			}
			SYMBOL_INPLACE_UNWIND_ALIAS(sym);
			if (!DeeBaseScope_IsVarargs(current_basescope, sym))
				break;
			if (sizeast->a_type != AST_CONSTEXPR)
				break;
			sizeval = sizeast->a_constexpr;
			if (!DeeInt_Check(sizeval))
				break;

			/* If the expression result isn't being used,
			 * then there is no need to do anything! */
			if (!PUSH_RESULT)
				goto done;
			if (!DeeInt_TryAsUInt8(sizeval, &va_size_val))
				break;

			/* All right! we can encode this one as `cmp eq, #varargs, $<va_size_val>' */
			DO(asm_putddi(self));
			DO(asm_gcmp_eq_varargs_sz(va_size_val));

			/* If the expression is checking for inequality, invert the result. */
			if (self->a_flag == OPERATOR_NE)
				DO(asm_gbool(true));
			goto done;
		}	break;

		case OPERATOR_CONTAINS:
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			if (self->a_operator.o_op0->a_type == AST_CONSTEXPR &&
			    (current_assembler.a_flag & (ASM_FOPTIMIZE | ASM_FOPTIMIZE_SIZE))) {
				DREF DeeObject *push_seq;
				int32_t cid;
				push_seq = DeeRoSet_FromSequenceOrMappingForContains(self->a_operator.o_op0->a_constexpr);
				if unlikely(!push_seq) {
					DO(!DeeError_Handled(ERROR_HANDLED_RESTORE));
					push_seq = self->a_operator.o_op0->a_constexpr;
					Dee_Incref(push_seq);
				}
				if unlikely(!asm_allowconst(push_seq)) {
					Dee_Decref_likely(push_seq);
					goto action_in_without_const;
				}
				cid = asm_newconst(push_seq);
				Dee_Decref_unlikely(push_seq);
				if unlikely(cid < 0)
					goto err;
				DO(ast_genasm(self->a_operator.o_op1, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(asm_gcontains_const((uint16_t)cid));
			} else {
				DO(ast_genasm_set(self->a_operator.o_op0, ASM_G_FPUSHRES));
				DO(ast_genasm_one(self->a_operator.o_op1, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(asm_gcontains());
			}
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
		case OPERATOR_ITER:
		case OPERATOR_SIZE:
			DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
			DO(asm_putddi(self));
			ASSERT(operator_instr_table[operator_name] != 0);
			DO((asm_dicsp(), asm_put(operator_instr_table[operator_name])));
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
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
			DO(ast_genasm_one(self->a_operator.o_op1, ASM_G_FPUSHRES));
			DO(asm_putddi(self));
			ASSERT(operator_instr_table[operator_name] != 0);
			DO((asm_ddicsp(), asm_put(operator_instr_table[operator_name])));
			goto pop_unused;

		case OPERATOR_DELITEM:
		case OPERATOR_DELATTR:
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
			DO(ast_genasm_one(self->a_operator.o_op1, ASM_G_FPUSHRES));
			DO(asm_putddi(self));
			ASSERT(operator_instr_table[operator_name] != 0);
			DO((asm_ddcsp(), asm_put(operator_instr_table[operator_name])));
			goto done_push_none;

		case OPERATOR_ASSIGN:
		case OPERATOR_MOVEASSIGN:
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			ASSERT(operator_instr_table[operator_name] != 0);
			if (PUSH_RESULT &&
			    ast_can_exchange(self->a_operator.o_op0,
			                     self->a_operator.o_op1)) {
				/* Optimization when A and B can be exchanged. */
				DO(ast_genasm(self->a_operator.o_op1, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(asm_gdup());
				DO(ast_genasm_one(self->a_operator.o_op0, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(asm_gswap());
			} else {
				DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
				DO(ast_genasm_one(self->a_operator.o_op1, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				if (PUSH_RESULT) {
					DO(asm_gdup());
					DO(asm_grrot(3));
				}
			}
			DO((asm_ddcsp(), asm_put(operator_instr_table[operator_name])));
			goto done;

		case OPERATOR_DELRANGE:
			if unlikely(!self->a_operator.o_op1)
				goto generic_operator;
			if unlikely(!self->a_operator.o_op2)
				goto generic_operator;
			DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
			DO(ast_genasm_one(self->a_operator.o_op1, ASM_G_FPUSHRES));
			DO(ast_genasm_one(self->a_operator.o_op2, ASM_G_FPUSHRES));
			DO(asm_putddi(self));
			ASSERT(operator_instr_table[operator_name] != 0);
			DO((asm_dddcsp(), asm_put(operator_instr_table[operator_name])));
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
				DO(ast_gen_symbol_inplace(opa->a_sym,
				                          is_unary ? NULL : self->a_operator.o_op1,
				                          self, operator_name,
				                          (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) != 0,
				                          gflags));
				goto done;

			case AST_OPERATOR:
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
				if (!opa->a_operator.o_op1)
					goto generic_operator;
				switch (opa->a_flag) {

				case OPERATOR_GETATTR:
					DO(ast_gen_setattr_inplace(opa->a_operator.o_op0,
					                           opa->a_operator.o_op1,
					                           is_unary ? NULL : self->a_operator.o_op1,
					                           self, operator_name,
					                           (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) != 0,
					                           gflags));
					goto done;

				case OPERATOR_GETITEM:
					DO(ast_gen_setitem_inplace(opa->a_operator.o_op0,
					                           opa->a_operator.o_op1,
					                           is_unary ? NULL : self->a_operator.o_op1,
					                           self, operator_name,
					                           (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) != 0,
					                           gflags));
					goto done;

				case OPERATOR_GETRANGE:
					if (!opa->a_operator.o_op2)
						goto generic_operator;
					DO(ast_gen_setrange_inplace(opa->a_operator.o_op0,
					                            opa->a_operator.o_op1,
					                            opa->a_operator.o_op2,
					                            is_unary ? NULL : self->a_operator.o_op1,
					                            self, operator_name,
					                            (self->a_operator.o_exflag & AST_OPERATOR_FPOSTOP) != 0,
					                            gflags));
					goto done;

				default: break;
				}
				goto generic_operator;


			default:
				/* TODO: Warning: Inplace operation used without prefix. */
				goto generic_operator;
			}
		}

		{
			/* Generic operator assembler. */
			uint8_t argc;
			struct symbol *prefix_symbol;
			struct opinfo const *info;
generic_operator:
			argc = 0;
			info = DeeTypeType_GetOperatorById(&DeeType_Type, operator_name);
			if (self->a_operator.o_op0->a_type == AST_SYM &&
			    (!info || (info->oi_cc & OPCC_FINPLACE))) {
				/* Generate a prefixed instruction. */
				prefix_symbol = self->a_operator.o_op0->a_sym;
				/* Make sure that the symbol can actually be used as a prefix. */
				if ((self->a_operator.o_exflag & AST_OPERATOR_FMAYBEPFX) &&
				    !asm_can_prefix_symbol(prefix_symbol))
					goto operator_without_prefix;
			} else {
operator_without_prefix:
				DO(ast_genasm(self->a_operator.o_op0, ASM_G_FPUSHRES));
				prefix_symbol = NULL;
			}
			/* Compile operands. */
			if (self->a_operator.o_op1) {
				++argc;
				DO(ast_genasm_one(self->a_operator.o_op1, ASM_G_FPUSHRES));
			}
			if (self->a_operator.o_op2) {
				++argc;
				DO(ast_genasm_one(self->a_operator.o_op2, ASM_G_FPUSHRES));
			}
			if (self->a_operator.o_op3) {
				++argc;
				DO(ast_genasm_one(self->a_operator.o_op3, ASM_G_FPUSHRES));
			}
			DO(asm_putddi(self));
			if (prefix_symbol) {
				DO(asm_gprefix_symbol(prefix_symbol, self->a_operator.o_op0));
				DO(asm_ginplace_operator(operator_name, argc));
			} else {
				DO(asm_goperator(operator_name, argc));
			}
			goto pop_unused;
		}
	}	break;

	case AST_ACTION: {
		uint16_t action_type;
		/* Special action operation. */
		action_type = self->a_flag & AST_FACTION_KINDMASK;
		ASSERT(AST_FACTION_ARGC_GT(self->a_flag) <= 3);
		ASSERT((AST_FACTION_ARGC_GT(self->a_flag) >= 3) == (self->a_action.a_act2 != NULL));
		ASSERT((AST_FACTION_ARGC_GT(self->a_flag) >= 2) == (self->a_action.a_act1 != NULL));
		ASSERT((AST_FACTION_ARGC_GT(self->a_flag) >= 1) == (self->a_action.a_act0 != NULL));
		switch (action_type) {

#define ACTION(x) case x & AST_FACTION_KINDMASK:
			ACTION(AST_FACTION_CELL0) {
				int32_t deemon_modid;
				if (!PUSH_RESULT)
					goto done;
				deemon_modid = asm_newmodule(DeeModule_GetDeemon());
				if unlikely(deemon_modid < 0)
					goto err;
				DO(asm_gcall_extern((uint16_t)deemon_modid, id_Cell, 0));
			}	break;

			ACTION(AST_FACTION_CELL1) {
				int32_t deemon_modid;
				DO(ast_genasm(self->a_action.a_act0, PUSH_RESULT));
				if (!PUSH_RESULT)
					goto done;
				deemon_modid = asm_newmodule(DeeModule_GetDeemon());
				if unlikely(deemon_modid < 0)
					goto err;
				DO(asm_gcall_extern((uint16_t)deemon_modid, id_Cell, 1));
			}	break;

			ACTION(AST_FACTION_TYPEOF) {
				DO(ast_genasm(self->a_action.a_act0, PUSH_RESULT));
				if (PUSH_RESULT) {
					DO(asm_putddi(self));
					DO(asm_gtypeof());
				}
			}	break;

			ACTION(AST_FACTION_CLASSOF) {
				DO(ast_genasm(self->a_action.a_act0, PUSH_RESULT));
				if (PUSH_RESULT) {
					DO(asm_putddi(self));
					DO(asm_gclassof());
				}
			}	break;

			ACTION(AST_FACTION_SUPEROF) {
				DO(ast_genasm(self->a_action.a_act0, PUSH_RESULT));
				if (PUSH_RESULT) {
					DO(asm_putddi(self));
					DO(asm_gsuperof());
				}
			}	break;

			ACTION(AST_FACTION_AS) {
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
								if unlikely(symid < 0)
									goto err;
								DO(asm_gsuper_this_r((uint16_t)symid));
								goto done;
							}
						}
						if (self->a_action.a_act1->a_type == AST_CONSTEXPR &&
						    current_basescope != (DeeBaseScopeObject *)current_rootscope &&
						    !(current_assembler.a_flag & ASM_FREDUCEREFS)) {
							typesym = asm_bind_deemon_export(self->a_action.a_act1->a_constexpr);
							if unlikely(!typesym)
								goto err;
							if (typesym != ASM_BIND_DEEMON_EXPORT_NOTFOUND)
								goto do_this_as_typesym_ref;
						}
					}
				}
				DO(ast_genasm(self->a_action.a_act0, PUSH_RESULT));
				DO(ast_genasm_one(self->a_action.a_act1, PUSH_RESULT));
				if (PUSH_RESULT) {
					DO(asm_putddi(self));
					DO(asm_gsuper());
				}
			}	break;

			ACTION(AST_FACTION_PRINT) {
				DO(ast_genprint(PRINT_MODE_NORMAL + PRINT_MODE_ALL,
				                self->a_action.a_act0, self));
				goto done_push_none;
			}

			ACTION(AST_FACTION_PRINTLN) {
				DO(ast_genprint(PRINT_MODE_NL + PRINT_MODE_ALL,
				                self->a_action.a_act0, self));
				goto done_push_none;
			}

			ACTION(AST_FACTION_FPRINT) {
				DO(ast_genasm(self->a_action.a_act0, ASM_G_FPUSHRES));
				DO(ast_genprint(PRINT_MODE_NORMAL + PRINT_MODE_FILE + PRINT_MODE_ALL,
				                self->a_action.a_act1, self));
				goto pop_unused;
			}

			ACTION(AST_FACTION_FPRINTLN) {
				DO(ast_genasm(self->a_action.a_act0, ASM_G_FPUSHRES));
				DO(ast_genprint(PRINT_MODE_NL + PRINT_MODE_FILE + PRINT_MODE_ALL,
				                self->a_action.a_act1, self));
				goto pop_unused;
			}

			ACTION(AST_FACTION_RANGE) {
				if (self->a_action.a_act0->a_type == AST_CONSTEXPR) {
					DeeObject *index_object = self->a_action.a_act0->a_constexpr;
					uint32_t intval;
					if (DeeNone_Check(index_object) ||
					    (DeeInt_Check(index_object) &&
					     (DeeInt_TryAsUInt32(index_object, &intval) && intval == 0))) {
						/* Special optimization: The begin index is not set, or equal to int(0). */
						if (AST_ISNONE(self->a_action.a_act2) &&
						    (self->a_action.a_act1->a_type == AST_CONSTEXPR &&
						     (index_object = self->a_action.a_act1->a_constexpr,
						      DeeInt_Check(index_object)) &&
						     DeeInt_TryAsUInt32(index_object, &intval))) {
							/* Special optimization: No step is given and the
							 * end index is known to be a constant integer. */
							if (PUSH_RESULT) {
								DO(asm_putddi(self));
								DO(asm_grange_0_i(intval));
							}
							break;
						}
						DO(ast_genasm(self->a_action.a_act1, PUSH_RESULT));
						if (AST_ISNONE(self->a_action.a_act2)) {
							if (PUSH_RESULT) {
								DO(asm_putddi(self));
								DO(asm_grange_0());
							}
						} else {
							DO(ast_genasm_one(self->a_action.a_act2, PUSH_RESULT));
							if (PUSH_RESULT) {
								DO(asm_putddi(self));
								DO(asm_grange_step_0());
							}
						}
						break;
					}
				}
				DO(ast_genasm(self->a_action.a_act0, PUSH_RESULT));
				DO(ast_genasm_one(self->a_action.a_act1, PUSH_RESULT));
				if (AST_ISNONE(self->a_action.a_act2)) {
					if (PUSH_RESULT) {
						DO(asm_putddi(self));
						DO(asm_grange());
					}
				} else {
					DO(ast_genasm_one(self->a_action.a_act2, PUSH_RESULT));
					if (PUSH_RESULT) {
						DO(asm_putddi(self));
						DO(asm_grange_step());
					}
				}
			}	break;

			ACTION(AST_FACTION_IS) {
				DO(ast_genasm(self->a_action.a_act0, PUSH_RESULT));
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
					DO(asm_putddi(self));
					DO(asm_gisnone());
					break;
				}
				DO(ast_genasm_one(self->a_action.a_act1, PUSH_RESULT));
				if (PUSH_RESULT) {
					DO(asm_putddi(self));
					if (self->a_action.a_act1->a_type == AST_CONSTEXPR &&
					    DeeType_Check(self->a_action.a_act1->a_constexpr) &&
					    !DeeType_IsAbstract((DeeTypeObject *)self->a_action.a_act1->a_constexpr)) {
						/* The type being checked isn't abstract, meaning that it
						 * cannot appear as a secondary base, meaning that INSTANCEOF
						 * is sufficient. */
						DO(asm_ginstanceof());
					} else {
						DO(asm_gimplements());
					}
				}
			}	break;

			ACTION(AST_FACTION_IN) {
				if (self->a_action.a_act1->a_type == AST_CONSTEXPR &&
				    (current_assembler.a_flag & (ASM_FOPTIMIZE | ASM_FOPTIMIZE_SIZE))) {
					DREF DeeObject *push_seq;
					int32_t cid;
					push_seq = DeeRoSet_FromSequenceOrMappingForContains(self->a_action.a_act1->a_constexpr);
					if unlikely(!push_seq) {
						DO(!DeeError_Handled(ERROR_HANDLED_RESTORE));
						push_seq = self->a_action.a_act1->a_constexpr;
						Dee_Incref(push_seq);
					}
					if unlikely(!asm_allowconst(push_seq)) {
						Dee_Decref_likely(push_seq);
						goto action_in_without_const;
					}
					cid = asm_newconst(push_seq);
					Dee_Decref_unlikely(push_seq);
					if unlikely(cid < 0)
						goto err;
					DO(ast_genasm(self->a_action.a_act0, ASM_G_FPUSHRES));
					DO(asm_putddi(self));
					DO(asm_gcontains_const((uint16_t)cid));
				} else {
action_in_without_const:
					if (ast_can_exchange(self->a_action.a_act0,
					                     self->a_action.a_act1)) {
						DO(ast_genasm_set(self->a_action.a_act1, ASM_G_FPUSHRES));
						DO(ast_genasm_one(self->a_action.a_act0, ASM_G_FPUSHRES));
						DO(asm_putddi(self));
						DO(asm_gcontains());
					} else {
						DO(ast_genasm(self->a_action.a_act0, ASM_G_FPUSHRES));
						DO(ast_genasm_set_one(self->a_action.a_act1, ASM_G_FPUSHRES));
						DO(asm_putddi(self));
						DO(asm_gswap());
						DO(asm_gcontains());
					}
				}
				goto pop_unused;
			}

			ACTION(AST_FACTION_MIN) {
				DO(ast_genasm(self->a_action.a_act0, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(asm_greduce_min());
				goto pop_unused;
			}

			ACTION(AST_FACTION_MAX) {
				DO(ast_genasm(self->a_action.a_act0, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(asm_greduce_max());
				goto pop_unused;
			}

			ACTION(AST_FACTION_SUM) {
				DO(ast_genasm(self->a_action.a_act0, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(asm_greduce_sum());
				goto pop_unused;
			}

			ACTION(AST_FACTION_ANY) {
				DO(ast_genasm(self->a_action.a_act0, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(asm_greduce_any());
				goto pop_unused;
			}

			ACTION(AST_FACTION_ALL) {
				DO(ast_genasm(self->a_action.a_act0, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(asm_greduce_all());
				goto pop_unused;
			}

			ACTION(AST_FACTION_STORE) {
				DO(asm_gstore(self->a_action.a_act0,
				              self->a_action.a_act1,
				              self, PUSH_RESULT));
			}	break;

			ACTION(AST_FACTION_BOUNDATTR) {
				DO(ast_gen_boundattr(self->a_action.a_act0,
				                     self->a_action.a_act1,
				                     self, PUSH_RESULT));
			}	break;

			ACTION(AST_FACTION_BOUNDITEM) {
				DO(ast_genasm(self->a_action.a_act0, ASM_G_FPUSHRES));
				DO(ast_genasm_one(self->a_action.a_act1, ASM_G_FPUSHRES));
				DO(asm_putddi(self));
				DO(asm_gbounditem());
				goto pop_unused;
			}

			ACTION(AST_FACTION_CALL_KW) {
				/* Call with keyword list. */
				DO(asm_gcall_kw_expr(self->a_action.a_act0,
				                     self->a_action.a_act1,
				                     self->a_action.a_act2,
				                     self, gflags));
			}	break;

			ACTION(AST_FACTION_ASSERT) {
				DO(asm_genassert(self->a_action.a_act0,
				                 NULL, self, gflags));
			}	break;

			ACTION(AST_FACTION_ASSERT_M) {
				DO(asm_genassert(self->a_action.a_act0,
				                 self->a_action.a_act1,
				                 self, gflags));
			}	break;

			ACTION(AST_FACTION_SAMEOBJ) {
				DO(ast_genasm(self->a_action.a_act0, PUSH_RESULT));
				DO(ast_genasm_one(self->a_action.a_act1, PUSH_RESULT));
				if (PUSH_RESULT) {
					DO(asm_putddi(self));
					DO(asm_gsameobj());
				}
				goto pop_unused;
			}

			ACTION(AST_FACTION_DIFFOBJ) {
				DO(ast_genasm(self->a_action.a_act0, PUSH_RESULT));
				DO(ast_genasm_one(self->a_action.a_act1, PUSH_RESULT));
				if (PUSH_RESULT) {
					DO(asm_putddi(self));
					DO(asm_gdiffobj());
				}
				goto pop_unused;
			}

#undef ACTION
		default:
			ASSERTF(0, "Invalid action type: %x", (unsigned int)action_type);
		}
	}	break;

	case AST_CLASS:
		DO(asm_genclass(self, gflags));
		break;

	case AST_LABEL: {
		struct text_label *label;
		struct asm_sym *sym;
		label = self->a_label.l_label;
		if (!label->tl_goto) {
			DO(WARNAST(self, W_ASM_LABEL_NEVER_USED,
			           text_label_name(label, self->a_flag & AST_FLABEL_CASE)));
		}
		sym = label->tl_asym;
		if (!sym) {
			sym = asm_newsym();
			if unlikely(!sym)
				goto err;
			label->tl_asym = sym;
		}
		if unlikely(ASM_SYM_DEFINED(sym)) {
			/* Warn if the label had already been defined. */
			DO(WARNAST(self, W_ASM_LABEL_ALREADY_DEFINED,
			           text_label_name(label, self->a_flag & AST_FLABEL_CASE)));
			goto done_push_none;
		}
		asm_defsym(sym);
		goto done_push_none;
	}

	case AST_GOTO: {
		struct text_label *label;
		struct asm_sym *sym;
		uint16_t old_stack;
		/* Just to a specified symbol. */
		label = self->a_goto.g_label;
		sym   = label->tl_asym;
		if (!sym) {
			sym = asm_newsym();
			if unlikely(!sym)
				goto err;
			label->tl_asym = sym;
		}
		old_stack = current_assembler.a_stackcur;
		/* Adjust the stack and jump to the proper symbol. */
		DO(asm_putddi(self));
		if (current_assembler.a_finsym) {
			/* NOTE: Must only go through the finally-block
			 *       if the jump target is located outside. */
			current_assembler.a_finflag |= ASM_FINFLAG_USED;
			DO(asm_gpush_abs(sym));
			DO(asm_gpush_stk(sym));
			DO(asm_gjmps(current_assembler.a_finsym));
		} else {
			DO(asm_gjmps(sym));
		}
		current_assembler.a_stackcur = old_stack;
		goto done_fake_none;
	}

	case AST_SWITCH:
		DO(ast_genasm_switch(self));
		/* Switch statements simply return `none',
		 * and cannot be used in expressions. */
		goto done_push_none;

	case AST_ASSEMBLY:
		DO(ast_genasm_userasm(self));
		goto done_push_none;

	default:
		ASSERTF(0, "Invalid AST type: %x", (unsigned int)self->a_type);
		break;
	}
done:
	ASM_POP_SCOPE(PUSH_RESULT ? 1 : 0, err);
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
