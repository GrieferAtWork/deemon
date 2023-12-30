/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_HOSTASM_LOADER_LOCUSE_C
#define GUARD_DEX_HOSTASM_LOADER_LOCUSE_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/alloc.h>
#include <deemon/asm.h>

#include <hybrid/byteswap.h>
#include <hybrid/unaligned.h>

#include "utils.h"

DECL_BEGIN

/* NOTE: These limits apply to:
 * >> mov local N, varkwds
 * Which does:
 * - rd(xlid(MEMSTATE_XLOCAL_VARKWDS))
 * - rd(xlid(MEMSTATE_XLOCAL_A_KW))
 * - rd(xlid(MEMSTATE_XLOCAL_A_ARGC))
 * - rd(xlid(MEMSTATE_XLOCAL_A_ARGV))
 * - wr(xlid(MEMSTATE_XLOCAL_VARKWDS))
 * - wr(N) */
#define ASM_RDMAX 4 /* Max # of times a single instruction can read some local */
#define ASM_WRMAX 2 /* Max # of times a single instruction can write some local */

struct asm_locuse {
	/* NOTE: When it comes to ordering, *all* reads *always* happen before writes. */
	size_t alu_rd[ASM_RDMAX]; /* LIDs read by the instruction (unused slots are set to `(size_t)-1') */
	size_t alu_wr[ASM_WRMAX]; /* LIDs written by the instruction (unused slots are set to `(size_t)-1') */
};


/* Load information on which locals are used by an instruction.
 * NOTE: This function intentionally *doesn't* track:
 * - MEMSTATE_XLOCAL_STDOUT
 * - MEMSTATE_XLOCAL_POPITER
 */
PRIVATE NONNULL((1, 2, 3)) void DCALL
asm_get_locuse(struct Dee_function_assembler *__restrict self,
               Dee_instruction_t const *instr,
               struct asm_locuse *__restrict result) {
#define xlid(id) (self->fa_localc + (id))
	uint16_t opcode;
	memset(result, 0xff, sizeof(*result));
scan_instr:
	opcode = instr[0];
	if (ASM_ISEXTENDED(opcode))
		opcode = (opcode << 8) | instr[1];

	switch (opcode) {

	case ASM_PUSH_BND_LOCAL:
	case ASM_PUSH_LOCAL:
	case ASM_CALL_LOCAL:
		result->alu_rd[0] = instr[1];
		break;

	case ASM16_PUSH_BND_LOCAL:
	case ASM16_PUSH_LOCAL:
	case ASM16_CALL_LOCAL:
		result->alu_rd[0] = UNALIGNED_GETLE16(instr + 1);
		break;

	case ASM_POP_LOCAL:
	case ASM_DEL_LOCAL:
		result->alu_wr[0] = instr[1];
		break;

	case ASM16_POP_LOCAL:
	case ASM16_DEL_LOCAL:
		result->alu_wr[0] = UNALIGNED_GETLE16(instr + 1);
		break;

		/* Must also track uses of extended locals (e.g. `MEMSTATE_XLOCAL_A_ARGV' by `ASM_PUSH_ARG') */
	case ASM_PUSH_BND_ARG: {
		uint16_t aid;
		aid = instr[1];
		__IF0 { case ASM16_PUSH_BND_ARG: aid = UNALIGNED_GETLE16(instr + 1); }
		if (self->fa_cc & HOSTFUNC_CC_F_TUPLE) {
			result->alu_rd[0] = xlid(MEMSTATE_XLOCAL_A_ARGS);
		} else {
			result->alu_rd[0] = xlid(MEMSTATE_XLOCAL_A_ARGC);
		}
	}	break;

	case ASM_PUSH_ARG: {
		uint16_t aid;
		aid = instr[1];
		__IF0 { case ASM16_PUSH_ARG: aid = UNALIGNED_GETLE16(instr + 1); }
		if (self->fa_cc & HOSTFUNC_CC_F_TUPLE) {
			result->alu_rd[0] = xlid(MEMSTATE_XLOCAL_A_ARGS);
		} else {
			result->alu_rd[0] = xlid(MEMSTATE_XLOCAL_A_ARGC);
			result->alu_rd[1] = xlid(MEMSTATE_XLOCAL_A_ARGV);
		}
		if (aid < self->fa_code->co_argc_min) {
			/* Normal positional argument */
		} else if (aid < self->fa_code->co_argc_max) {
			/* Optional argument */
			result->alu_rd[2] = xlid(MEMSTATE_XLOCAL_DEFARG(aid - self->fa_code->co_argc_min));
			/* XXX: ARGS/ARGC/ARGV isn't only needed if DEFARG isn't unconditionally bound at this point! */
			if (result->alu_rd[1] == (size_t)-1) {
				result->alu_rd[1] = result->alu_rd[2];
				result->alu_rd[2] = (size_t)-1;
			}
		}
	}	break;

	case ASM_SUPER_THIS_R:
	case ASM16_SUPER_THIS_R:
	case ASM_GETATTR_THIS_C:
	case ASM16_GETATTR_THIS_C:
	case ASM_DELATTR_THIS_C:
	case ASM16_DELATTR_THIS_C:
	case ASM_SETATTR_THIS_C:
	case ASM16_SETATTR_THIS_C:
	case ASM_CALLCMEMBER_THIS_R:
	case ASM16_CALLCMEMBER_THIS_R:
	case ASM_CALLATTR_THIS_C:
	case ASM16_CALLATTR_THIS_C:
	case ASM_CALLATTR_THIS_C_TUPLE:
	case ASM16_CALLATTR_THIS_C_TUPLE:
	case ASM_GETMEMBER_THIS_R:
	case ASM16_GETMEMBER_THIS_R:
	case ASM_DELMEMBER_THIS_R:
	case ASM16_DELMEMBER_THIS_R:
	case ASM_SETMEMBER_THIS_R:
	case ASM16_SETMEMBER_THIS_R:
	case ASM_BOUNDMEMBER_THIS_R:
	case ASM16_BOUNDMEMBER_THIS_R:
	case ASM_PUSH_THIS:
	case ASM_SUPERGETATTR_THIS_RC:
	case ASM16_SUPERGETATTR_THIS_RC:
	case ASM_SUPERCALLATTR_THIS_RC:
	case ASM16_SUPERCALLATTR_THIS_RC:
	case ASM_GETMEMBER_THIS:
	case ASM16_GETMEMBER_THIS:
	case ASM_DELMEMBER_THIS:
	case ASM16_DELMEMBER_THIS:
	case ASM_SETMEMBER_THIS:
	case ASM16_SETMEMBER_THIS:
	case ASM_BOUNDMEMBER_THIS:
	case ASM16_BOUNDMEMBER_THIS:
		result->alu_rd[0] = xlid(MEMSTATE_XLOCAL_A_THIS);
		break;

	case ASM_PUSH_THIS_FUNCTION:
		if (self->fa_cc & HOSTFUNC_CC_F_FUNC)
			result->alu_rd[0] = xlid(MEMSTATE_XLOCAL_A_FUNC);
		break;

	case ASM_VARARGS_CMP_EQ_SZ:
	case ASM_VARARGS_CMP_GR_SZ:
	case ASM_VARARGS_GETSIZE:
		if (self->fa_cc & HOSTFUNC_CC_F_TUPLE) {
			result->alu_rd[0] = xlid(MEMSTATE_XLOCAL_A_ARGS);
		} else {
			result->alu_rd[0] = xlid(MEMSTATE_XLOCAL_A_ARGC);
		}
		break;

	case ASM_VARARGS_GETITEM:
		if (self->fa_cc & HOSTFUNC_CC_F_TUPLE) {
			result->alu_rd[0] = xlid(MEMSTATE_XLOCAL_A_ARGS);
		} else {
			result->alu_rd[0] = xlid(MEMSTATE_XLOCAL_A_ARGC);
			result->alu_rd[1] = xlid(MEMSTATE_XLOCAL_A_ARGV);
		}
		break;

	case ASM_VARARGS_GETITEM_I:
		if (self->fa_cc & HOSTFUNC_CC_F_TUPLE) {
			result->alu_rd[0] = xlid(MEMSTATE_XLOCAL_A_ARGS);
		} else {
			result->alu_rd[0] = xlid(MEMSTATE_XLOCAL_A_ARGV);
		}
		break;

	case ASM_PUSH_VARARGS:
	case ASM_VARARGS_UNPACK:
		result->alu_wr[0] = xlid(MEMSTATE_XLOCAL_VARARGS);
		result->alu_rd[0] = xlid(MEMSTATE_XLOCAL_VARARGS);
		/* XXX: Below is only needed if `MEMSTATE_XLOCAL_VARARGS'
		 *      isn't unconditionally bound at this point! */
		if (self->fa_cc & HOSTFUNC_CC_F_TUPLE) {
			result->alu_rd[1] = xlid(MEMSTATE_XLOCAL_A_ARGS);
		} else {
			result->alu_rd[1] = xlid(MEMSTATE_XLOCAL_A_ARGC);
			result->alu_rd[2] = xlid(MEMSTATE_XLOCAL_A_ARGV);
		}
		break;

	case ASM_PUSH_VARKWDS:
	case ASM_PUSH_VARKWDS_NE:
		result->alu_wr[0] = xlid(MEMSTATE_XLOCAL_VARKWDS);
		result->alu_rd[0] = xlid(MEMSTATE_XLOCAL_VARKWDS);
		/* XXX: Below is only needed if `MEMSTATE_XLOCAL_VARKWDS'
		 *      isn't unconditionally bound at this point! */
		result->alu_rd[1] = xlid(MEMSTATE_XLOCAL_A_KW);
		if (self->fa_cc & HOSTFUNC_CC_F_TUPLE) {
			result->alu_rd[2] = xlid(MEMSTATE_XLOCAL_A_ARGS);
		} else {
			result->alu_rd[2] = xlid(MEMSTATE_XLOCAL_A_ARGC);
			result->alu_rd[3] = xlid(MEMSTATE_XLOCAL_A_ARGV);
		}
		break;



		/* All other prefixes (simply skip and re-parse since these don't affect how/which locals are used) */
	case ASM_GLOBAL:
	case ASM_STATIC:
	case ASM_STACK:
		instr += 2;
		__IF0 {
	case ASM_EXTERN:
			instr += 3;
		}
		__IF0 {
	case ASM16_GLOBAL:
	case ASM16_STATIC:
	case ASM16_STACK:
			instr += 4;
		}
		__IF0 {
	case ASM16_EXTERN:
			instr += 6;
		}
		goto scan_instr;

	case ASM_LOCAL: {
		uint16_t prefix_lid;
		prefix_lid = instr[1];
		instr += 2;
		__IF0 {
	case ASM16_LOCAL:
			prefix_lid = UNALIGNED_GETLE16(instr + 1);
			instr += 4;
		}
		opcode = instr[0];
		if (ASM_ISEXTENDED(opcode))
			opcode = (opcode << 8) | instr[1];

		/* Check what instruction being prefixed does. */
		asm_get_locuse(self, instr, result);
		switch (opcode) {

		case ASM_JF:           /* jf PREFIX, <Sdisp8> */
		case ASM_JF16:         /* jf PREFIX, <Sdisp16> */
		case ASM_JT:           /* jt PREFIX, <Sdisp8> */
		case ASM_JT16:         /* jt PREFIX, <Sdisp16> */
		case ASM_FOREACH:      /* foreach PREFIX, <Sdisp8> */
		case ASM_FOREACH16:    /* foreach PREFIX, <Sdisp16> */
		case ASM_RET:          /* ret PREFIX */
		case ASM_THROW:        /* throw PREFIX */
		case ASM_SETRET:       /* setret PREFIX */
		case ASM_POP_STATIC:   /* mov static <imm8>, PREFIX */
		case ASM16_POP_STATIC: /* mov static <imm16>, PREFIX */
		case ASM_POP_EXTERN:   /* mov extern <imm8>:<imm8>, PREFIX */
		case ASM16_POP_EXTERN: /* mov extern <imm16>:<imm16>, PREFIX */
		case ASM_POP_GLOBAL:   /* mov global <imm8>, PREFIX */
		case ASM16_POP_GLOBAL: /* mov global <imm16>, PREFIX */
		case ASM_POP_LOCAL:    /* mov local <imm8>, PREFIX */
		case ASM16_POP_LOCAL:  /* mov local <imm16>, PREFIX */
		case ASM_UNPACK:       /* unpack PREFIX, #<imm8> */
		case ASM_POP_N:        /* mov #SP - <imm8> - 2, PREFIX */
			/* Insert an extra read at the start */
			memmoveupc(&result->alu_rd[1], &result->alu_rd[0], ASM_RDMAX - 1, sizeof(size_t));
			result->alu_rd[0] = prefix_lid;
			break;

		case ASM_SWAP:           /* swap top, PREFIX */
		case ASM_LROT:           /* lrot #<imm8>+2, PREFIX */
		case ASM_RROT:           /* rrot #<imm8>+2, PREFIX */
		case ASM16_LROT:         /* lrot #<imm16>+2, PREFIX */
		case ASM16_RROT:         /* rrot #<imm16>+2, PREFIX */
		case ASM_ADD:            /* add PREFIX, pop */
		case ASM_SUB:            /* sub PREFIX, pop */
		case ASM_MUL:            /* mul PREFIX, pop */
		case ASM_DIV:            /* div PREFIX, pop */
		case ASM_MOD:            /* mod PREFIX, pop */
		case ASM_SHL:            /* shl PREFIX, pop */
		case ASM_SHR:            /* shr PREFIX, pop */
		case ASM_AND:            /* and PREFIX, pop */
		case ASM_OR:             /* or  PREFIX, pop */
		case ASM_XOR:            /* xor PREFIX, pop */
		case ASM_POW:            /* pow PREFIX, pop */
		case ASM_INC:            /* inc PREFIX */
		case ASM_DEC:            /* dec PREFIX */
		case ASM_ADD_SIMM8:      /* add PREFIX, $<Simm8> */
		case ASM_ADD_IMM32:      /* add PREFIX, $<imm32> */
		case ASM_SUB_SIMM8:      /* sub PREFIX, $<Simm8> */
		case ASM_SUB_IMM32:      /* sub PREFIX, $<imm32> */
		case ASM_MUL_SIMM8:      /* mul PREFIX, $<Simm8> */
		case ASM_DIV_SIMM8:      /* div PREFIX, $<Simm8> */
		case ASM_MOD_SIMM8:      /* mod PREFIX, $<Simm8> */
		case ASM_SHL_IMM8:       /* shl PREFIX, $<Simm8> */
		case ASM_SHR_IMM8:       /* shr PREFIX, $<Simm8> */
		case ASM_AND_IMM32:      /* and PREFIX, $<imm32> */
		case ASM_OR_IMM32:       /* or  PREFIX, $<imm32> */
		case ASM_XOR_IMM32:      /* xor PREFIX, $<imm32> */
		case ASM_INCPOST:        /* push inc PREFIX' - `PREFIX: push inc */
		case ASM_DECPOST:        /* push dec PREFIX' - `PREFIX: push dec */
		case ASM_OPERATOR:       /* PREFIX: push op $<imm8>, #<imm8> */
		case ASM16_OPERATOR:     /* PREFIX: push op $<imm16>, #<imm8> */
		case ASM_OPERATOR_TUPLE: /* PREFIX: push op $<imm8>, pop... */
			/* Insert an extra read at the start, and then a write at the end */
			memmoveupc(&result->alu_rd[1], &result->alu_rd[0], ASM_RDMAX - 1, sizeof(size_t));
			result->alu_rd[0] = prefix_lid;
			ATTR_FALLTHROUGH
		case ASM_DUP:                /* mov  PREFIX, top', `mov PREFIX, #SP - 1 */
		case ASM_DUP_N:              /* mov  PREFIX, #SP - <imm8> - 2 */
		case ASM16_DUP_N:            /* mov  PREFIX, #SP - <imm16> - 2 */
		case ASM_POP:                /* mov  top, PREFIX */
		case ASM_PUSH_NONE:          /* mov  PREFIX, none */
		case ASM_PUSH_VARARGS:       /* mov  PREFIX, varargs */
		case ASM_PUSH_VARKWDS:       /* mov  PREFIX, varkwds */
		case ASM_PUSH_MODULE:        /* mov  PREFIX, module <imm8> */
		case ASM_PUSH_ARG:           /* mov  PREFIX, arg <imm8> */
		case ASM_PUSH_CONST:         /* mov  PREFIX, const <imm8> */
		case ASM_PUSH_REF:           /* mov  PREFIX, ref <imm8> */
		case ASM_PUSH_STATIC:        /* mov  PREFIX, static <imm8> */
		case ASM_PUSH_EXTERN:        /* mov  PREFIX, extern <imm8>:<imm8> */
		case ASM_PUSH_GLOBAL:        /* mov  PREFIX, global <imm8> */
		case ASM_PUSH_LOCAL:         /* mov  PREFIX, local <imm8> */
		case ASM_FUNCTION_C:         /* PREFIX: function const <imm8>, #<imm8>+1 */
		case ASM16_FUNCTION_C:       /* PREFIX: function const <imm16>, #<imm8>+1 */
		case ASM_FUNCTION_C_16:      /* PREFIX: function const <imm8>, #<imm16>+1 */
		case ASM16_FUNCTION_C_16:    /* PREFIX: function const <imm16>, #<imm16>+1 */
		case ASM_PUSH_EXCEPT:        /* mov  PREFIX, except */
		case ASM_PUSH_THIS:          /* mov  PREFIX, this */
		case ASM_PUSH_THIS_MODULE:   /* mov  PREFIX, this_module */
		case ASM_PUSH_THIS_FUNCTION: /* mov  PREFIX, this_function */
		case ASM16_PUSH_MODULE:      /* mov  PREFIX, module <imm16> */
		case ASM16_PUSH_ARG:         /* mov  PREFIX, arg <imm16> */
		case ASM16_PUSH_CONST:       /* mov  PREFIX, const <imm16> */
		case ASM16_PUSH_REF:         /* mov  PREFIX, ref <imm16> */
		case ASM16_PUSH_STATIC:      /* mov  PREFIX, static <imm16> */
		case ASM16_PUSH_EXTERN:      /* mov  PREFIX, extern <imm16>:<imm16> */
		case ASM16_PUSH_GLOBAL:      /* mov  PREFIX, global <imm16> */
		case ASM16_PUSH_LOCAL:       /* mov  PREFIX, local <imm16> */
		case ASM_PUSH_TRUE:          /* mov  PREFIX, true */
		case ASM_PUSH_FALSE: {       /* mov  PREFIX, false */
			/* Append an extra write at the end. */
			size_t i;
			for (i = 0; i < ASM_WRMAX - 1; ++i) {
				if (result->alu_wr[i] == (size_t)-1)
					break;
			}
			result->alu_wr[i] = prefix_lid;
		}	break;

		default: break;
		}
	}	break;

	default: break;
	}
#undef xlid
}


/* Pass #1:
 * - Clear `block->bb_locuse'
 * - Have a second bitset `b_written' (cleared by default)
 * - For each instruction:
 *   - If a read happens while `b_written[lid] == 0', do `block->bb_locuse[lid] = 1'
 *   - If a read happens while `b_written[lid] == 1', do nothing
 *   - If a write happens, do `b_written[lid] = 1' */
PRIVATE NONNULL((1, 2, 3)) void DCALL
Dee_basic_block_locuse_pass1(struct Dee_function_assembler *__restrict assembler,
                             struct Dee_basic_block *__restrict block,
                             byte_t *b_written) {
	Dee_instruction_t const *instr;
	size_t n_locals = assembler->fa_xlocalc;
	bitset_clearall(b_written, n_locals);
	bitset_clearall(block->bb_locuse, n_locals);
	for (instr = block->bb_deemon_start;
	     instr < block->bb_deemon_end;
	     instr = DeeAsm_NextInstr(instr)) {
		size_t i;
		struct asm_locuse use;
		asm_get_locuse(assembler, instr, &use);
		for (i = 0; i < ASM_RDMAX; ++i) {
			size_t lid = use.alu_rd[i];
			if (lid < n_locals && !bitset_test(b_written, lid))
				bitset_set(block->bb_locuse, lid);
		}
		for (i = 0; i < ASM_WRMAX; ++i) {
			size_t lid = use.alu_wr[i];
			if (lid < n_locals)
				bitset_set(b_written, lid);
		}
	}
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) bool DCALL
Dee_basic_block_locuse_or(struct Dee_function_assembler *__restrict assembler,
                          struct Dee_basic_block *__restrict block,
                          struct Dee_basic_block const *__restrict other,
                          byte_t const *b_written) {
	bool result = false;
	size_t n_locals = assembler->fa_xlocalc;
	size_t n_bytes  = _bitset_sizeof(n_locals);
	size_t i;
	for (i = 0; i < n_bytes; ++i) {
		byte_t old_byte = block->bb_locuse[i];
		byte_t oth_byte = other->bb_locuse[i];
		byte_t wrt_byte = b_written[i];
		byte_t new_byte = old_byte | (oth_byte & ~wrt_byte);
		if (old_byte != new_byte) {
			block->bb_locuse[i] = new_byte;
			result = true;
		}
	}
	return result;
}

/* Pass #2:
 * - Have a second bitset `b_written' (cleared by default)
 * - For each instruction:
 *   - NOTE: The unconditional branch at the end of the block also counts as a branch
 *   - If a write happens, do `b_written[lid] = 1'
 *   - If it's a branch-instruction, take the `target' block and do:
 *     >> block->bb_locuse |= target->bb_locuse & ~b_written; */
PRIVATE WUNUSED NONNULL((1, 2, 3)) bool DCALL
Dee_basic_block_locuse_pass2(struct Dee_function_assembler *__restrict assembler,
                             struct Dee_basic_block *__restrict block,
                             byte_t *b_written) {
	bool result = false;
	Dee_instruction_t const *instr;
	struct Dee_jump_descriptor **exit_iter, **exit_end;
	size_t n_locals = assembler->fa_xlocalc;
	bitset_clearall(b_written, n_locals);
	exit_iter = block->bb_exits.jds_list;
	exit_end  = exit_iter + block->bb_exits.jds_size;
	for (instr = block->bb_deemon_start;
	     instr < block->bb_deemon_end;
	     instr = DeeAsm_NextInstr(instr)) {
		size_t i;
		struct asm_locuse use;
		for (; exit_iter < exit_end; ++exit_iter) {
			struct Dee_jump_descriptor *exit_jmp;
			exit_jmp = *exit_iter;
			if (exit_jmp->jd_from > instr)
				break;
			result |= Dee_basic_block_locuse_or(assembler, block, exit_jmp->jd_to, b_written);
		}
		asm_get_locuse(assembler, instr, &use);
		for (i = 0; i < ASM_WRMAX; ++i) {
			size_t lid = use.alu_wr[i];
			if (lid < n_locals)
				bitset_set(b_written, lid);
		}
	}
	for (; unlikely(exit_iter < exit_end); ++exit_iter) {
		ASSERT((*exit_iter)->jd_from < block->bb_deemon_end);
		result |= Dee_basic_block_locuse_or(assembler, block, (*exit_iter)->jd_to, b_written);
	}
	/* Deal with the unconditional branch at the end of the block. */
	if (block->bb_next != NULL)
		result |= Dee_basic_block_locuse_or(assembler, block, block->bb_next, b_written);
	return result;
}

#ifndef CONFIG_HAVE_qsort
#define CONFIG_HAVE_qsort
#define qsort   dee_qsort
DeeSystem_DEFINE_qsort(dee_qsort)
#endif /* !CONFIG_HAVE_qsort */

#ifndef __LIBCCALL
#define __LIBCCALL /* nothing */
#endif /* !__LIBCCALL */

PRIVATE WUNUSED int __LIBCCALL
Dee_basic_block_loclastread_compare(void const *a, void const *b) {
	struct Dee_basic_block_loclastread const *lhs = (struct Dee_basic_block_loclastread const *)a;
	struct Dee_basic_block_loclastread const *rhs = (struct Dee_basic_block_loclastread const *)b;
	if (lhs->bbl_instr < rhs->bbl_instr)
		return -1;
	if (lhs->bbl_instr > rhs->bbl_instr)
		return 1;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_basic_block_bb_locreadv_append(struct Dee_basic_block *__restrict self,
                                   size_t *__restrict p_bb_locreada,
                                   Dee_instruction_t const *instr, size_t lid) {
	size_t bb_locreada = *p_bb_locreada;
	ASSERT((self->bb_locreadc + 1) <= bb_locreada || !self->bb_locreadc);
	if ((self->bb_locreadc + 1) >= bb_locreada) {
		struct Dee_basic_block_loclastread *new_vec;
		size_t min_alloc = self->bb_locreadc + 2; /* +2 because we need an extra slot for the trailing NULL-entry */
		size_t new_alloc = self->bb_locreadc * 2;
		if (new_alloc < 4)
			new_alloc = 4;
		if (new_alloc < min_alloc)
			new_alloc = min_alloc;
		new_vec = (struct Dee_basic_block_loclastread *)Dee_TryReallocc(self->bb_locreadv,
		                                                                new_alloc,
		                                                                sizeof(struct Dee_basic_block_loclastread));
		if unlikely(!new_vec) {
			new_alloc = min_alloc;
			new_vec = (struct Dee_basic_block_loclastread *)Dee_Reallocc(self->bb_locreadv,
			                                                             new_alloc,
			                                                             sizeof(struct Dee_basic_block_loclastread));
			if likely(!new_vec)
				goto err;
		}
		self->bb_locreadv = new_vec;
		*p_bb_locreada = new_alloc;
	}
	self->bb_locreadv[self->bb_locreadc].bbl_instr = instr;
	self->bb_locreadv[self->bb_locreadc].bbl_lid   = lid;
	++self->bb_locreadc;
	return 0;
err:
	return -1;
}

/* Pass #3:
 * - Have a second bitset `b_written' (cleared by default)
 * - Have a map `Dee_instruction_t const *i_lastread[ms_localc]' (all NULL by default)
 * - For each instruction:
 *   - NOTE: In case of a branch-instructions, "read happens" for `target->bb_locuse'
 *   - NOTE: Unconditional branches shouldn't appear (already removed by `Dee_function_assembler_loadblocks()')
 *   - If a read happens, do `i_lastread[lid] = instr'
 *   - If a write happens, do:
 *     >> if (i_lastread[lid] != NULL)
 *     >>     block->bb_locreadv.append(Dee_basic_block_loclastread { i_lastread[lid], lid });
 *     >> i_lastread[lid] = instr;
 * - At the end of the block, do:
 *   >> size_t i;
 *   >> for (i = 0; i < ms_localc; ++i) {
 *   >>     Dee_instruction_t const *lr = i_lastread[i];
 *   >>     if (lr == NULL)
 *   >>         continue;
 *   >>     if (block->bb_next != NULL && block->bb_next->bb_locuse[i])
 *   >>         continue;
 *   >>     block->bb_locreadv.append(Dee_basic_block_loclastread { lr, i });
 *   >> }
 * - Sort `block->bb_locreadv' by `bbl_instr' */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_basic_block_locuse_pass3(struct Dee_function_assembler *__restrict assembler,
                             struct Dee_basic_block *__restrict block,
                             Dee_instruction_t const **i_lastread) {
#define LOCAL_append_lastreadat(instr, lid)                                              \
	do {                                                                                 \
		if unlikely(Dee_basic_block_bb_locreadv_append(block, &bb_locreada, instr, lid)) \
			goto err;                                                                    \
	}	__WHILE0
	size_t bb_locreada = 0;
	Dee_instruction_t const *instr;
	struct Dee_jump_descriptor **exit_iter, **exit_end;
	size_t i, n_locals = assembler->fa_xlocalc;
	ASSERT(block->bb_locreadc == 0);
	ASSERT(block->bb_locreadv == NULL);
	bzeroc(i_lastread, n_locals, sizeof(Dee_instruction_t const *));
	exit_iter = block->bb_exits.jds_list;
	exit_end  = exit_iter + block->bb_exits.jds_size;

	/* Figure out the last time a read happens prior to a write */
	for (instr = block->bb_deemon_start;
	     instr < block->bb_deemon_end;
	     instr = DeeAsm_NextInstr(instr)) {
		struct asm_locuse use;
		asm_get_locuse(assembler, instr, &use);
		for (i = 0; i < ASM_RDMAX; ++i) {
			size_t lid = use.alu_rd[i];
			if (lid < n_locals)
				i_lastread[lid] = instr;
		}
		for (; exit_iter < exit_end; ++exit_iter) {
			struct Dee_jump_descriptor *exit_jmp;
			struct Dee_basic_block *to;
			exit_jmp = *exit_iter;
			if (exit_jmp->jd_from > instr)
				break;
			to = exit_jmp->jd_to;
			for (i = 0; i < n_locals; ++i) {
				if (bitset_test(to->bb_locuse, i))
					i_lastread[i] = instr;
			}
		}
		for (i = 0; i < ASM_WRMAX; ++i) {
			size_t lid = use.alu_wr[i];
			if (lid < n_locals) {
				if (i_lastread[lid] != NULL)
					LOCAL_append_lastreadat(i_lastread[lid], lid);
				i_lastread[i] = instr;
			}
		}
	}

	/* Deal with dangling jump descriptors (shouldn't happen). */
	for (; unlikely(exit_iter < exit_end); ++exit_iter) {
		struct Dee_basic_block *to = (*exit_iter)->jd_to;
		ASSERT((*exit_iter)->jd_from < block->bb_deemon_end);
		for (i = 0; i < n_locals; ++i) {
			if (bitset_test(to->bb_locuse, i))
				i_lastread[i] = instr;
		}
	}

	/* Deal with locals that aren't used by fallthru blocks. */
	for (i = 0; i < n_locals; ++i) {
		Dee_instruction_t const *lr = i_lastread[i];
		if (lr == NULL)
			continue;
		if (block->bb_next != NULL && bitset_test(block->bb_next->bb_locuse, i))
			continue;
		LOCAL_append_lastreadat(lr, i);
	}

	if (block->bb_locreadc > 0) {
#ifndef __OPTIMIZE_SIZE__
		/* Release unused memory. */
		if ((block->bb_locreadc + 1) < bb_locreada) {
			struct Dee_basic_block_loclastread *final_vec;
			final_vec = (struct Dee_basic_block_loclastread *)Dee_TryReallocc(block->bb_locreadv,
			                                                                  block->bb_locreadc + 1,
			                                                                  sizeof(struct Dee_basic_block_loclastread));
			if likely(final_vec)
				block->bb_locreadv = final_vec;
		}
#endif /* !__OPTIMIZE_SIZE__ */
	
		/* Sort last-local-reads of the block. */
		qsort(block->bb_locreadv, block->bb_locreadc,
		      sizeof(struct Dee_basic_block_loclastread),
		      &Dee_basic_block_loclastread_compare);
		block->bb_locreadv[block->bb_locreadc].bbl_instr = (Dee_instruction_t const *)-1;
	}
	return 0;
#undef LOCAL_append_lastreadat
err:
	return -1;
}

/* Step #1.1 (optional: `DEE_FUNCTION_ASSEMBLER_F_NOEARLYDEL'):
 * Figure out all the instructions that read from a local the last time before
 * the function ends, or the variable gets b_written to again. By using this info,
 * `Dee_function_generator_geninstr()' emits extra instrumentation in order to
 * delete local variables earlier than usual, which in turn significantly lowers
 * the overhead associated with keeping objects alive longer than strictly
 * necessary. When this step is skipped, local simply aren't deleted early.
 * - self->fa_blockv[*]->bb_locuse
 * - self->fa_blockv[*]->bb_locreadv
 * - self->fa_blockv[*]->bb_locreadc
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_loadlocuse(struct Dee_function_assembler *__restrict self) {
	size_t i;
	union u_temps {
		Dee_instruction_t const **i_lastread;
		byte_t                   *b_written;
	};
	union u_temps tempbuf;
	tempbuf.i_lastread = (Dee_instruction_t const **)Dee_Mallocac(self->fa_xlocalc,
	                                                              sizeof(Dee_instruction_t const *));
	if unlikely(!tempbuf.i_lastread)
		goto err;

	/* - For each `block':
	 *   - Clear `block->bb_locuse'
	 *   - Have a second bitset `b_written' (cleared by default)
	 *   - For each instruction:
	 *     - If a read happens while `b_written[lid] == 0', do `block->bb_locuse[lid] = 1'
	 *     - If a read happens while `b_written[lid] == 1', do nothing
	 *     - If a write happens, do `b_written[lid] = 1'
	 * - For each `block' (until nothing changes anymore):
	 *   - Have a second bitset `b_written' (cleared by default)
	 *   - For each instruction:
	 *     - NOTE: The unconditional branch at the end of the block also counts as a branch
	 *     - If a write happens, do `b_written[lid] = 1'
	 *     - If it's a branch-instruction, take the `target' block and do:
	 *       >> block->bb_locuse |= target->bb_locuse & ~b_written;
	 * - For each `block':
	 *   - Have a second bitset `b_written' (cleared by default)
	 *   - Have a map `Dee_instruction_t const *i_lastread[ms_localc]' (all NULL by default)
	 *   - For each instruction:
	 *     - NOTE: In case of a branch-instructions, "read happens" for `target->bb_locuse'
	 *     - NOTE: Unconditional branches shouldn't appear (already removed by `Dee_function_assembler_loadblocks()')
	 *     - If a read happens, do `i_lastread[lid] = instr'
	 *     - If a write happens, do:
	 *       >> if (i_lastread[lid] != NULL)
	 *       >>     block->bb_locreadv.append(Dee_basic_block_loclastread { i_lastread[lid], lid });
	 *       >> i_lastread[lid] = instr;
	 *   - At the end of the block, do:
	 *     >> size_t i;
	 *     >> for (i = 0; i < ms_localc; ++i) {
	 *     >>     Dee_instruction_t const *lr = i_lastread[i];
	 *     >>     if (lr == NULL)
	 *     >>         continue;
	 *     >>     if (block->bb_next != NULL && block->bb_next->bb_locuse[i])
	 *     >>         continue;
	 *     >>     block->bb_locreadv.append(Dee_basic_block_loclastread { lr, i });
	 *     >> }
	 *   - Sort `block->bb_locreadv' by `bbl_instr' */
	for (i = 0; i < self->fa_blockc; ++i)
		Dee_basic_block_locuse_pass1(self, self->fa_blockv[i], tempbuf.b_written);
	{
		bool changed;
		do {
			changed = false;
			for (i = 0; i < self->fa_blockc; ++i)
				changed |= Dee_basic_block_locuse_pass2(self, self->fa_blockv[i], tempbuf.b_written);
		} while (changed);
	}
	for (i = 0; i < self->fa_blockc; ++i) {
		if unlikely(Dee_basic_block_locuse_pass3(self, self->fa_blockv[i], tempbuf.i_lastread))
			goto err_tempbuf;
	}

	Dee_Freea(tempbuf.b_written);
	return 0;
err_tempbuf:
	Dee_Freea(tempbuf.b_written);
err:
	return -1;
}

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_LOADER_LOCUSE_C */
