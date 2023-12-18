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
#ifndef GUARD_DEX_HOSTASM_LOADER_C
#define GUARD_DEX_HOSTASM_LOADER_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/asm.h>
#include <deemon/error.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/unaligned.h>

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
register_jump(struct Dee_function_assembler *__restrict self,
              struct Dee_basic_block *from_block,
              Dee_instruction_t const *from,
              Dee_instruction_t const *to) {
	struct Dee_basic_block *to_block;
	struct Dee_jump_descriptor *jump;
	ASSERT(from >= from_block->bb_deemon_start);
	ASSERT(from < from_block->bb_deemon_end);

	/* Make sure that a basic block begins at `to' */
	to_block = Dee_function_assembler_splitblock(self, to);
	if unlikely(!to_block)
		goto err;
	ASSERT(to_block->bb_deemon_start == to);

	/* Allocate a descriptor for the jump. */
	jump = Dee_jump_descriptor_alloc();
	if unlikely(!jump)
		goto err;
	jump->jd_from = from;
	jump->jd_to   = to_block;
	jump->jd_stat = NULL; /* Filled in later... */

	/* Fix from_block if it got split. */
	if (!(from >= from_block->bb_deemon_start &&
	      from < from_block->bb_deemon_end)) {
		from_block = Dee_function_assembler_locateblock(self, from);
		ASSERT(from_block);
	}
	ASSERT(from >= from_block->bb_deemon_start);
	ASSERT(from < from_block->bb_deemon_end);

	/* Insert the jump into the from->exit and to->entry tables. */
	if unlikely(Dee_jump_descriptors_insert(&to_block->bb_entries, jump))
		goto err_jump;
	if unlikely(Dee_jump_descriptors_insert(&from_block->bb_exits, jump))
		goto err; /* `to_block->bb_entries' owns the jump at this point. */

	return 0;
err_jump:
	Dee_jump_descriptor_free(jump);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
scan_and_split_block(struct Dee_function_assembler *__restrict self,
                     struct Dee_basic_block *block) {
	Dee_instruction_t const *iter;
	for (iter = block->bb_deemon_start;
	     iter < block->bb_deemon_end;
	     iter = DeeAsm_NextInstr(iter)) {
		iter = DeeAsm_SkipPrefix(iter);
		switch (iter[0]) {

		case ASM_JF:
		case ASM_JT:
		case ASM_JMP:
		case ASM_FOREACH: {
			int8_t delta = *(int8_t const *)(iter + 1);
			Dee_instruction_t const *to = iter + 2 + delta;
			if unlikely(register_jump(self, block, iter, to))
				goto err;
		}	break;

		case ASM_JF16:
		case ASM_JT16:
		case ASM_JMP16:
		case ASM_FOREACH16: {
			int16_t delta = (int16_t)UNALIGNED_GETLE16(iter + 1);
			Dee_instruction_t const *to = iter + 3 + delta;
			if unlikely(register_jump(self, block, iter, to))
				goto err;
		}	break;

		case ASM_JMP_POP:
			/* TODO: Try to detect code pattern:
			 * >>    push @{ "foo": 10, "bar": 20 }
			 * >>    push ...
			 * >>    push @30
			 * >>    callattr top, @"get", #2
			 * >>    jmp  pop   // Can jump to 10, 20 and 30
			 */
			return DeeError_Throwf(&DeeError_NotImplemented, "Complex jump instructions");

		case ASM_EXTENDED1:
			switch (iter[1]) {

			case ASM32_JMP & 0xff: {
				int32_t delta = (int32_t)UNALIGNED_GET32(iter + 2);
				Dee_instruction_t const *to = iter + 6 + delta;
				if unlikely(register_jump(self, block, iter, to))
					goto err;
			}	break;

			case ASM_JMP_POP_POP & 0xff:
				/* TODO: Try to detect code pattern:
				 * >>    push @{ "foo": (10, 1), "bar": (20, 1) }
				 * >>    push ...
				 * >>    push @(30, 2)
				 * >>    callattr top, @"get", #2
				 * >>    jmp  pop   // Can jump to 10:1, 20:1 and 30:2
				 */
				return DeeError_Throwf(&DeeError_NotImplemented, "Complex jump instructions");

			default: break;
			}
			break;

		default:
			break;
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) void DCALL
scan_block_for_noreturn(struct Dee_basic_block *__restrict block, uint16_t code_flags) {
	Dee_instruction_t const *iter;
	for (iter = block->bb_deemon_start;
	     iter < block->bb_deemon_end;
	     iter = DeeAsm_NextInstr(iter)) {
		uint16_t opcode;
		iter   = DeeAsm_SkipPrefix(iter);
		opcode = iter[0];
		if (ASM_ISEXTENDED(opcode))
			opcode = (opcode << 8) | iter[1];
		if (DeeAsm_IsNoreturn(opcode, code_flags)) {
			block->bb_deemon_end = DeeAsm_NextInstr(iter);
			block->bb_next = NULL;
			break;
		}
	}
}


/* Step #1: Load basic blocks. Fills in:
 * - self->fa_blockc
 * - self->fa_blockv[*]->bb_deemon_start
 * - self->fa_blockv[*]->bb_deemon_end
 * - self->fa_blockv[*]->bb_entries+bb_exits
 * - self->fa_blockv[*]->bb_entries.jds_list[*]->jd_from
 * - self->fa_blockv[*]->bb_entries.jds_list[*]->jd_to
 * - self->fa_blockv[*]->bb_next
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_loadblocks(struct Dee_function_assembler *__restrict self) {
	size_t i;
	struct Dee_basic_block *block;
	ASSERT(self->fa_blockc == 0);

	if (self->fa_code->co_flags & CODE_FYIELDING)
		return DeeError_Throwf(&DeeError_IllegalInstruction, "Cannot re-compile yield function");
	if (self->fa_code->co_flags & CODE_FASSEMBLY)
		return DeeError_Throwf(&DeeError_IllegalInstruction, "Cannot re-compile function with user-assembly");
	if (self->fa_code->co_exceptc > 0)
		return DeeError_Throwf(&DeeError_IllegalInstruction, "Cannot re-compile function with exception handlers");

	/* Start out with a single block that spans the entire code object.
	 * This block will be split into smaller ones as we scan the code. */
	if (!self->fa_blocka) {
		self->fa_blocka = 8;
		self->fa_blockv = (struct Dee_basic_block **)Dee_TryMallocc(8, sizeof(struct Dee_basic_block *));
		if (!self->fa_blockv) {
			self->fa_blocka = 1;
			self->fa_blockv = (struct Dee_basic_block **)Dee_Mallocc(1, sizeof(struct Dee_basic_block *));
			if unlikely(!self->fa_blockv)
				goto err;
		}
	}

	/* Initialize the initial block. */
	block = Dee_basic_block_alloc();
	if unlikely(!block)
		goto err;
	Dee_basic_block_init_common(block);
	block->bb_deemon_start = self->fa_code->co_code;
	block->bb_deemon_end   = self->fa_code->co_code + self->fa_code->co_codebytes;
	Dee_jump_descriptors_init(&block->bb_exits);

	/* Set the initial block. */
	self->fa_blockv[0] = block;
	self->fa_blockc    = 1;

	/* Scan and split the primary block. */
	if unlikely(scan_and_split_block(self, block))
		goto err;

	/* Scan all remaining blocks. */
	ASSERT(self->fa_blockc >= 1);
	ASSERT(self->fa_blockv[0] == block);
	for (i = 1; i < self->fa_blockc; ++i) {
		if unlikely(scan_and_split_block(self, self->fa_blockv[i]))
			goto err;
	}

	/* Assign block exits (for fallthru). */
	for (i = 0; i < self->fa_blockc - 1; ++i) {
		self->fa_blockv[i]->bb_next = self->fa_blockv[i + 1];
	}

	/* Search basic blocks for noreturn instructions.
	 * When one such instruction is found, the block ends on that instruction,
	 * and has its `bb_next' field set to `NULL' and its end trimmed. */
	for (i = 0; i < self->fa_blockc; ++i) {
		scan_block_for_noreturn(self->fa_blockv[i], self->fa_code->co_flags);
	}

	return 0;
err:
	return -1;
}


/* Assign `state' to `self->bb_mem_start', or merge it with that state.
 * @return: 1 : Entry state was assigned or became more restrictive
 * @return: 0 : Nothing changed
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_memstate_assign_or_merge(DREF struct Dee_memstate **__restrict p_dst,
                             struct Dee_memstate *__restrict state) {
	int result;
	uint16_t lid;
	struct Dee_memstate *block = *p_dst;
	if (block == NULL) {
		/* Simple case: no state has yet to be assigned. */
		*p_dst = state;
		Dee_memstate_incref(state);
		return 1;
	}


	/* Complicated case: actually need to merge states. */
	result = 0;
	ASSERT(block->ms_localc == state->ms_localc);
	for (lid = 0; lid < block->ms_localc; ++lid) {
		uint16_t block_flags = block->ms_localv[lid].ml_flags & MEMLOC_M_LOCAL_BSTATE;
		uint16_t state_flags = state->ms_localv[lid].ml_flags & MEMLOC_M_LOCAL_BSTATE;
		uint16_t merge_flags = ((block_flags & state_flags) & MEMLOC_F_LOCAL_BOUND) |
		                       ((block_flags | state_flags) & MEMLOC_F_LOCAL_UNBOUND);
		ASSERT(merge_flags != (MEMLOC_F_LOCAL_BOUND | MEMLOC_F_LOCAL_UNBOUND));
		if (merge_flags != block_flags) {
			/* Must update the block's state. */
			if (Dee_memstate_isshared(block)) {
				block = Dee_memstate_copy(block);
				if unlikely(!block)
					goto err;
				ASSERT((*p_dst)->ms_refcnt >= 2);
				--(*p_dst)->ms_refcnt;
				(*p_dst) = block;
			}
			block->ms_localv[lid].ml_flags &= ~MEMLOC_M_LOCAL_BSTATE;
			block->ms_localv[lid].ml_flags |= merge_flags;
			result = 1;
		}
	}

	return result;
err:
	return -1;
}


/* Scan `self' for changes to the binding of locals, as well as assign/merge
 * the then-active `Dee_memstate' to jumps that take place inside `self'. The
 * returned `Dee_memstate' represents the binding-state of locals at the end
 * of `self'.
 * @param: primary_state: The entry-state of the function. When a variable is
 *                        read when it *may* be unbound, its `ml_where' is set
 *                        to `MEMLOC_TYPE_HSTACK' in this state. This in
 *                        turn causes `Dee_function_assembler_compileblocks'
 *                        to allocate the variable early.
 * @param: trim_if_end_unreachable: When true, trim unreachable code before returning `ITER_DONE'
 * @return: * :        The binding-state of locals at the end of `self'
 * @return: ITER_DONE: The end of the basic block cannot be reached like this.
 * @return: NULL:      Error */
PRIVATE WUNUSED NONNULL((1)) DREF struct Dee_memstate *DCALL
Dee_basic_block_scan_boundlocals(struct Dee_basic_block *__restrict self,
                                 struct Dee_memstate *__restrict primary_state,
                                 bool trim_if_end_unreachable) {
	uint16_t _lid;
	byte_t const *iter;
	DREF struct Dee_memstate *state;
#define do_state_unshare()                        \
	do {                                          \
		if unlikely(Dee_memstate_unshare(&state)) \
			goto err_state;                       \
	}	__WHILE0
#define state_local_verify(lid)                  \
	do {                                         \
		if unlikely((lid) >= state->ms_localc) { \
			_lid = (lid);                        \
			goto err_state_bad_lid;              \
		}                                        \
	}	__WHILE0
#define primary_state_remember_read_before_write(lid) \
	(void)(primary_state->ms_localv[lid].ml_where = MEMLOC_TYPE_HSTACK)
#define state_local_read(lid)                                           \
	do {                                                                \
		/* Variable is always unbound -> unconditional throw. */        \
		if (state->ms_localv[lid].ml_flags & MEMLOC_F_LOCAL_UNBOUND)    \
			goto do_return_unreachable;                                 \
		if (!(state->ms_localv[lid].ml_flags & MEMLOC_F_LOCAL_BOUND)) { \
			/* After a read, a variable is *always* bound (because if   \
			 * it wasn't, the read would throw an exception. */         \
			do_state_unshare();                                         \
			state->ms_localv[lid].ml_flags |= MEMLOC_F_LOCAL_BOUND;     \
			primary_state_remember_read_before_write(lid);              \
		}                                                               \
	}	__WHILE0
#define state_local_write(lid)                                          \
	do {                                                                \
		if ((state->ms_localv[lid].ml_flags & MEMLOC_M_LOCAL_BSTATE) != \
		    /*----------------------------*/ (MEMLOC_F_LOCAL_BOUND)) {  \
			do_state_unshare();                                         \
			state->ms_localv[lid].ml_flags &= ~MEMLOC_M_LOCAL_BSTATE;   \
			state->ms_localv[lid].ml_flags |= MEMLOC_F_LOCAL_BOUND;     \
		}                                                               \
	}	__WHILE0
#define state_local_readwrite(lid) state_local_read(lid)
	state = self->bb_mem_start;
	ASSERT(state);
	Dee_memstate_incref(state);
	for (iter = self->bb_deemon_start; iter < self->bb_deemon_end;
	     iter = DeeAsm_NextInstr(iter)) {
		uint16_t opcode = iter[0];
		if (ASM_ISEXTENDED(opcode))
			opcode = (opcode << 8) | iter[1];
		switch (opcode) {

		case ASM_JF:
		case ASM_JF16:
		case ASM_JT:
		case ASM_JT16:
		case ASM_JMP:
		case ASM_JMP16:
		case ASM32_JMP:
		case ASM_FOREACH:
		case ASM_FOREACH16: {
			struct Dee_jump_descriptor *jmp;
			jmp = Dee_jump_descriptors_lookup(&self->bb_exits, iter);
			ASSERTF(jmp, "Should have been found by `scan_and_split_block()'");
			if unlikely(Dee_memstate_assign_or_merge(&jmp->jd_stat, state) < 0)
				goto err_state;
		}	break;

		case ASM_PUSH_BND_LOCAL: {
			uint16_t lid;
			lid = iter[1];
			__IF0 {
		case ASM16_PUSH_BND_LOCAL: 
				lid = UNALIGNED_GETLE16(iter + 2);
			}
			state_local_verify(lid);
			/* If the variable's binding-state is unknown, then it must be pre-allocated */
			if ((state->ms_localv[lid].ml_flags & MEMLOC_M_LOCAL_BSTATE) == MEMLOC_F_LOCAL_UNKNOWN)
				primary_state_remember_read_before_write(lid);
		}	break;

		case ASM_DEL_LOCAL: {
			uint16_t lid;
			lid = iter[1];
			__IF0 {
		case ASM16_DEL_LOCAL: 
				lid = UNALIGNED_GETLE16(iter + 2);
			}
			state_local_verify(lid);
			/* After being deleted, a local variable is always unbound */
			if ((state->ms_localv[lid].ml_flags & MEMLOC_M_LOCAL_BSTATE) != MEMLOC_F_LOCAL_UNBOUND) {
				do_state_unshare();
				state->ms_localv[lid].ml_flags &= ~MEMLOC_M_LOCAL_BSTATE;
				state->ms_localv[lid].ml_flags |= MEMLOC_F_LOCAL_UNBOUND;
			}
		}	break;

		case ASM_POP_LOCAL: {
			uint16_t lid;
			lid = iter[1];
			__IF0 {
		case ASM16_POP_LOCAL: 
				lid = UNALIGNED_GETLE16(iter + 2);
			}
			state_local_verify(lid);
			state_local_write(lid);
		}	break;

		case ASM_PUSH_LOCAL:
		case ASM_CALL_LOCAL: {
			uint16_t lid;
			lid = iter[1];
			__IF0 {
		case ASM16_PUSH_LOCAL:
		case ASM16_CALL_LOCAL:
				lid = UNALIGNED_GETLE16(iter + 2);
			}
			state_local_verify(lid);
			state_local_read(lid);
		}	break;

		case ASM_LOCAL: {
			uint16_t lid;
			byte_t const *prefix_iter;
			uint16_t prefix_opcode;
			lid         = iter[1];
			prefix_iter = iter + 2;
			__IF0 {
		case ASM16_LOCAL:
				lid         = UNALIGNED_GETLE16(iter + 2);
				prefix_iter = iter + 4;
			}
			prefix_opcode = prefix_iter[0];
			if (ASM_ISEXTENDED(prefix_opcode))
				prefix_opcode = (prefix_opcode << 8) | prefix_iter[1];
			state_local_verify(lid);
			switch (prefix_opcode) {

			case ASM_RET:          /* ret PREFIX */
			case ASM_YIELDALL:     /* yield foreach, PREFIX */
			case ASM_THROW:        /* throw PREFIX */
			case ASM_SETRET:       /* setret PREFIX */
			case ASM_POP:          /* mov top, PREFIX */
			case ASM_POP_N:        /* mov #SP - <imm8> - 2, PREFIX */
			case ASM16_POP_N:      /* mov #SP - <imm16> - 2, PREFIX */
			case ASM_POP_STATIC:   /* mov static <imm8>, PREFIX */
			case ASM16_POP_STATIC: /* mov static <imm16>, PREFIX */
			case ASM_POP_EXTERN:   /* mov extern <imm8>:<imm8>, PREFIX */
			case ASM16_POP_EXTERN: /* mov extern <imm16>:<imm16>, PREFIX */
			case ASM_POP_GLOBAL:   /* mov global <imm8>, PREFIX */
			case ASM16_POP_GLOBAL: /* mov global <imm16>, PREFIX */
			case ASM_UNPACK:       /* unpack PREFIX, #<imm8> */
			case ASM16_UNPACK:     /* unpack PREFIX, #<imm16> */
				state_local_read(lid);
				break;

			case ASM_DUP:                /* mov PREFIX, top */
			case ASM_DUP_N:              /* mov PREFIX, #SP - <imm8> - 2 */
			case ASM16_DUP_N:            /* mov PREFIX, #SP - <imm16> - 2 */
			case ASM_PUSH_NONE:          /* mov PREFIX, none */
			case ASM_PUSH_VARARGS:       /* mov PREFIX, varargs */
			case ASM_PUSH_VARKWDS:       /* mov PREFIX, varkwds */
			case ASM_PUSH_MODULE:        /* mov PREFIX, module <imm8> */
			case ASM16_PUSH_MODULE:      /* mov PREFIX, module <imm16> */
			case ASM_PUSH_ARG:           /* mov PREFIX, arg <imm8> */
			case ASM16_PUSH_ARG:         /* mov PREFIX, arg <imm16> */
			case ASM_PUSH_CONST:         /* mov PREFIX, const <imm8> */
			case ASM16_PUSH_CONST:       /* mov PREFIX, const <imm16> */
			case ASM_PUSH_REF:           /* mov PREFIX, ref <imm8> */
			case ASM16_PUSH_REF:         /* mov PREFIX, ref <imm16> */
			case ASM_PUSH_STATIC:        /* mov PREFIX, static <imm8> */
			case ASM16_PUSH_STATIC:      /* mov PREFIX, static <imm16> */
			case ASM_PUSH_EXTERN:        /* mov PREFIX, extern <imm8>:<imm8> */
			case ASM16_PUSH_EXTERN:      /* mov PREFIX, extern <imm16>:<imm16> */
			case ASM_PUSH_GLOBAL:        /* mov PREFIX, global <imm8> */
			case ASM16_PUSH_GLOBAL:      /* mov PREFIX, global <imm16> */
			case ASM_FUNCTION_C:         /* PREFIX: function const <imm8>, #<imm8>+1 */
			case ASM_FUNCTION_C_16:      /* PREFIX: function const <imm8>, #<imm16>+1 */
			case ASM16_FUNCTION_C:       /* PREFIX: function const <imm16>, #<imm8>+1 */
			case ASM16_FUNCTION_C_16:    /* PREFIX: function const <imm16>, #<imm16>+1 */
			case ASM_PUSH_EXCEPT:        /* mov PREFIX, except */
			case ASM_PUSH_THIS:          /* mov PREFIX, this */
			case ASM_PUSH_THIS_MODULE:   /* mov PREFIX, this_module */
			case ASM_PUSH_THIS_FUNCTION: /* mov PREFIX, this_function */
			case ASM_PUSH_TRUE:          /* mov PREFIX, true */
			case ASM_PUSH_FALSE:         /* mov PREFIX, false */
				state_local_write(lid);
				break;

			case ASM_OPERATOR:         /* PREFIX: push op $<imm8>, #<imm8> */
			case ASM16_OPERATOR:       /* PREFIX: push op $<imm16>, #<imm8> */
			case ASM_OPERATOR_TUPLE:   /* PREFIX: PREFIX: push op $<imm8>, pop... */
			case ASM16_OPERATOR_TUPLE: /* PREFIX: PREFIX: push op $<imm16>, pop... */
			case ASM_SWAP:             /* swap top, PREFIX */
			case ASM_LROT:             /* lrot #<imm8>+2, PREFIX */
			case ASM16_LROT:           /* lrot #<imm16>+2, PREFIX */
			case ASM_RROT:             /* rrot #<imm8>+2, PREFIX */
			case ASM16_RROT:           /* rrot #<imm16>+2, PREFIX */
			case ASM_ADD:              /* add PREFIX, pop */
			case ASM_SUB:              /* sub PREFIX, pop */
			case ASM_MUL:              /* mul PREFIX, pop */
			case ASM_DIV:              /* div PREFIX, pop */
			case ASM_MOD:              /* mod PREFIX, pop */
			case ASM_SHL:              /* shl PREFIX, pop */
			case ASM_SHR:              /* shr PREFIX, pop */
			case ASM_AND:              /* and PREFIX, pop */
			case ASM_OR:               /* or PREFIX, pop */
			case ASM_XOR:              /* xor PREFIX, pop */
			case ASM_POW:              /* pow PREFIX, pop */
			case ASM_INC:              /* inc PREFIX */
			case ASM_DEC:              /* dec PREFIX */
			case ASM_ADD_SIMM8:        /* add PREFIX, $<Simm8> */
			case ASM_ADD_IMM32:        /* add $<imm32>, PREFIX */
			case ASM_SUB_SIMM8:        /* sub PREFIX, $<Simm8> */
			case ASM_SUB_IMM32:        /* sub $<imm32>, PREFIX */
			case ASM_MUL_SIMM8:        /* mul PREFIX, $<Simm8> */
			case ASM_DIV_SIMM8:        /* div PREFIX, $<Simm8> */
			case ASM_MOD_SIMM8:        /* mod PREFIX, $<Simm8> */
			case ASM_SHL_IMM8:         /* shl PREFIX, $<Simm8> */
			case ASM_SHR_IMM8:         /* shr PREFIX, $<Simm8> */
			case ASM_AND_IMM32:        /* and PREFIX, $<imm32> */
			case ASM_OR_IMM32:         /* or PREFIX, $<imm32>  */
			case ASM_XOR_IMM32:        /* xor PREFIX, $<imm32> */
			case ASM_INCPOST:          /* push inc PREFIX */
			case ASM_DECPOST:          /* push dec PREFIX */
				state_local_readwrite(lid);
				break;

			case ASM_POP_LOCAL: {
				uint16_t dst_lid;
				dst_lid = prefix_iter[1];
				__IF0 {
			case ASM16_POP_LOCAL: 
					dst_lid = UNALIGNED_GETLE16(prefix_iter + 2);
				}
				state_local_verify(dst_lid);
				state_local_read(lid);
				state_local_write(dst_lid);
			}	break;

			case ASM_PUSH_LOCAL: {
				uint16_t src_lid;
				src_lid = prefix_iter[1];
				__IF0 {
			case ASM16_PUSH_LOCAL: 
					src_lid = UNALIGNED_GETLE16(prefix_iter + 2);
				}
				state_local_verify(src_lid);
				state_local_read(src_lid);
				state_local_write(lid);
			}	break;

			case ASM_JF:
			case ASM_JF16:
			case ASM_JT:
			case ASM_JT16: {
				struct Dee_jump_descriptor *jmp;
				state_local_read(lid);
				jmp = Dee_jump_descriptors_lookup(&self->bb_exits, iter);
				ASSERTF(jmp, "Should have been found by `scan_and_split_block()'");
				if unlikely(Dee_memstate_assign_or_merge(&jmp->jd_stat, state) < 0)
					goto err_state;
			}	break;
				
			case ASM_FOREACH:
			case ASM_FOREACH16: {
				/* Special case: here, the jump happens in a context where the
				 * local variable's state remains unchanged, whereas the jump
				 * doesn't happen in a context where the variable gets assigned */
				struct Dee_jump_descriptor *jmp;
				jmp = Dee_jump_descriptors_lookup(&self->bb_exits, iter);
				ASSERTF(jmp, "Should have been found by `scan_and_split_block()'");
				if unlikely(Dee_memstate_assign_or_merge(&jmp->jd_stat, state) < 0)
					goto err_state;
				state_local_write(lid);
			}	break;

			default:
				break;
			}
		}	break;

		case ASM_STACK:
		case ASM_STATIC:
		case ASM_GLOBAL: {
			byte_t const *prefix_iter;
			uint16_t prefix_opcode;
			prefix_iter = iter + 2;
			__IF0 {
		case ASM_EXTERN:
				prefix_iter = iter + 3;
			}
			__IF0 {
		case ASM16_STACK:
		case ASM16_STATIC:
		case ASM16_GLOBAL:
				prefix_iter = iter + 4;
			}
			__IF0 {
		case ASM16_EXTERN:
				prefix_iter = iter + 6;
			}
			prefix_opcode = prefix_iter[0];
			if (ASM_ISEXTENDED(prefix_opcode))
				prefix_opcode = (prefix_opcode << 8) | prefix_iter[1];
			switch (prefix_opcode) {

			case ASM_POP_LOCAL: {
				uint16_t dst_lid;
				dst_lid = prefix_iter[1];
				__IF0 {
			case ASM16_POP_LOCAL: 
					dst_lid = UNALIGNED_GETLE16(prefix_iter + 2);
				}
				state_local_verify(dst_lid);
				state_local_write(dst_lid);
			}	break;

			case ASM_PUSH_LOCAL: {
				uint16_t src_lid;
				src_lid = prefix_iter[1];
				__IF0 {
			case ASM16_PUSH_LOCAL: 
					src_lid = UNALIGNED_GETLE16(prefix_iter + 2);
				}
				state_local_verify(src_lid);
				state_local_read(src_lid);
			}	break;

			case ASM_JF:
			case ASM_JF16:
			case ASM_JT:
			case ASM_JT16:
			case ASM_FOREACH:
			case ASM_FOREACH16: {
				struct Dee_jump_descriptor *jmp;
				jmp = Dee_jump_descriptors_lookup(&self->bb_exits, iter);
				ASSERTF(jmp, "Should have been found by `scan_and_split_block()'");
				if unlikely(Dee_memstate_assign_or_merge(&jmp->jd_stat, state) < 0)
					goto err_state;
			}	break;

			default:
				break;
			}
		}	break;

		default:
			break;
		}
	}
	return state;
err_state_bad_lid:
	err_illegal_lid(_lid);
err_state:
	Dee_memstate_decref(state);
	return NULL;
do_return_unreachable:
	if (trim_if_end_unreachable) {
		self->bb_deemon_end = DeeAsm_NextInstr(iter);
		self->bb_next       = NULL;
	}
	Dee_memstate_decref(state);
	return (DREF struct Dee_memstate *)ITER_DONE;
#undef state_local_write
#undef state_local_read
#undef primary_state_remember_read_before_write
#undef state_local_verify
#undef do_state_unshare
}


/* Step #2: Scan all basic blocks to determine the bound-flags of
 *          local variables at the start of every block. Fills in:
 * - self->fa_blockv[*]->bb_mem_start  (but only `ms_localc+ms_localv'; ms_stackv is left NULL/empty)
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_loadboundlocals(struct Dee_function_assembler *__restrict self) {
	size_t i;
	uint16_t lid;
	struct Dee_memstate *state, *primary_state;
	struct Dee_basic_block *block;
	bool has_unreachable_code = false;

	/* Setup the state of the function's first (entry) block. */
	state = Dee_memstate_alloc(self->fa_code->co_localc);
	if unlikely(!state)
		goto err;
	state->ms_refcnt = 1;
	state->ms_host_cfa_offset = 0;
	state->ms_localc = self->fa_code->co_localc;
	state->ms_stackc = 0;
	state->ms_stacka = 0;
	Dee_memstate_hregs_clear_usage(state);
	state->ms_stackv = NULL;

	/* Initially, all variables are unbound */
	for (lid = 0; lid < state->ms_localc; ++lid) {
		state->ms_localv[lid].ml_flags = MEMLOC_F_NOREF | MEMLOC_F_LOCAL_UNBOUND;
		state->ms_localv[lid].ml_where = MEMLOC_TYPE_UNALLOC;
	}
	block = self->fa_blockv[0];
	block->bb_mem_start = state; /* Inherit reference */
	primary_state = state;

	/* Scan the primary fall-thru execution chain. */
	for (;;) {
		int error;
		state = Dee_basic_block_scan_boundlocals(block, primary_state, false);
		if unlikely(!state)
			goto err;
		if (state == (struct Dee_memstate *)ITER_DONE) {
			block->bb_host_relc = 1;
			has_unreachable_code = true;
			break;
		}
		block = block->bb_next;
		if (!block) {
			Dee_memstate_decref(state);
			break;
		}
		error = Dee_memstate_assign_or_merge(&block->bb_mem_start, state);
		Dee_memstate_decref(state);
		if unlikely(error)
			goto err;
	}

	/* Go through all blocks and merge  */
	for (;;) {
		bool changed = false;
		for (i = 0; i < self->fa_blockc; ++i) {
			bool block_changed = false;
			size_t entry_i;
			block = self->fa_blockv[i];
			ASSERT(block);
merge_and_scan_changed_block:
			for (entry_i = 0; entry_i < block->bb_entries.jds_size; ++entry_i) {
				int temp;
				struct Dee_jump_descriptor *jump;
				jump = block->bb_entries.jds_list[entry_i];
				ASSERT(jump);
				if (!jump->jd_stat)
					continue; /* Not yet loaded */
				temp = Dee_memstate_assign_or_merge(&block->bb_mem_start, jump->jd_stat);
				if (temp != 0) {
					if unlikely(temp < 0)
						goto err;
					block_changed = true;
				}
			}
			if (block_changed) {
				changed = true;
				/* (Re-)scan the block for changes */
				block->bb_host_relc = 0;
				state = Dee_basic_block_scan_boundlocals(block, primary_state, false);
				if unlikely(!state)
					goto err;
				if (state == (struct Dee_memstate *)ITER_DONE) {
					block->bb_host_relc = 1;
					has_unreachable_code = true;
				} else {
					/* Merge the block's exit status with the block it fall into. */
					block = block->bb_next;
					if (!block) {
						Dee_memstate_decref(state);
					} else {
						int temp;
						temp = Dee_memstate_assign_or_merge(&block->bb_mem_start, state);
						Dee_memstate_decref(state);
						if (temp != 0) {
							if unlikely(temp < 0)
								goto err;
							goto merge_and_scan_changed_block;
						}
					}
				}
			}
		}
		if (!changed)
			break;
	}

	/* Trim unreachable code if we discovered any. */
	if (has_unreachable_code) {
		for (i = 0; i < self->fa_blockc; ++i) {
			block = self->fa_blockv[i];
			ASSERT(block);
			if (block->bb_host_relc) {
				block->bb_host_relc = 0;
				state = Dee_basic_block_scan_boundlocals(block, primary_state, true);
				if unlikely(!state)
					goto err;
			}
		}
	}

	return 0;
err:
	for (i = 0; i < self->fa_blockc; ++i) {
		block = self->fa_blockv[i];
		ASSERT(block);
		block->bb_host_relc = 0;
	}
	return -1;
}

/* Step #3: Trim jumps whose origin has been determined to be unreachable.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_trimunused(struct Dee_function_assembler *__restrict self) {
	size_t i;
	for (i = 0; i < self->fa_blockc; ++i) {
		size_t exit_count;
		struct Dee_basic_block *block = self->fa_blockv[i];
		ASSERT(block);
		exit_count = block->bb_exits.jds_size;
		while (exit_count > 0) {
			struct Dee_jump_descriptor *jmp;
			jmp = block->bb_exits.jds_list[exit_count - 1];
			ASSERT(jmp);
			if (jmp->jd_from < block->bb_deemon_end)
				break;
			--exit_count;
		}
		ASSERT(exit_count <= block->bb_exits.jds_size);
		while (exit_count < block->bb_exits.jds_size) {
			/* Get rid of trimmed exits! */
			struct Dee_jump_descriptor *unused_jmp;
			--block->bb_exits.jds_size;
			unused_jmp = block->bb_exits.jds_list[block->bb_exits.jds_size];
			ASSERT(unused_jmp->jd_to);
			Dee_jump_descriptors_remove(&unused_jmp->jd_to->bb_entries, unused_jmp);
			Dee_jump_descriptor_destroy(unused_jmp);
		}
	}

	/* Check if there are blocks that are entirely unreachable, and merge
	 * blocks that can only be reached via fallthru with their predecessor. */
	for (i = 1; i < self->fa_blockc;) {
		struct Dee_basic_block *prev_block;
		struct Dee_basic_block *block = self->fa_blockv[i];
		if (block->bb_entries.jds_size > 0) {
			++i;
			continue;
		}

		/* Block can't be reached via jumps (can happen if jumps were unreachable).
		 * Either merge with predecessor block (if reachable by fallthru), or remove
		 * entirely (if predecessor block is noreturn) */
		prev_block = self->fa_blockv[i - 1];
		ASSERT(prev_block->bb_next == NULL ||
		       prev_block->bb_next == block);
		if (prev_block->bb_next == NULL) {
			/* Block is entirely unreachable. */
		} else {
			ASSERT(prev_block->bb_deemon_end <= block->bb_deemon_start);
			/* Merge with predecessor block. */
			if (block->bb_exits.jds_size > 0) {
				if (prev_block->bb_exits.jds_size == 0) {
					prev_block->bb_exits = block->bb_exits;
					block->bb_exits.jds_list = NULL;
				} else {
					size_t avail, need;
					ASSERT(prev_block->bb_exits.jds_alloc >= prev_block->bb_exits.jds_size);
					avail = prev_block->bb_exits.jds_alloc - prev_block->bb_exits.jds_size;
					need  = block->bb_exits.jds_size;
					if (need > avail) {
						struct Dee_jump_descriptor **new_list;
						size_t min_alloc = prev_block->bb_exits.jds_size + need;
						size_t new_alloc = prev_block->bb_exits.jds_alloc * 2;
						if (new_alloc < min_alloc)
							new_alloc = min_alloc;
						new_list = (struct Dee_jump_descriptor **)Dee_TryReallocc(prev_block->bb_exits.jds_list,
						                                                          new_alloc,
						                                                          sizeof(struct Dee_jump_descriptor *));
						if unlikely(!new_list) {
							new_alloc = min_alloc;
							new_list = (struct Dee_jump_descriptor **)Dee_Reallocc(prev_block->bb_exits.jds_list,
							                                                       new_alloc,
							                                                       sizeof(struct Dee_jump_descriptor *));
							if unlikely(!new_list)
								goto err;
						}
						prev_block->bb_exits.jds_list  = new_list;
						prev_block->bb_exits.jds_alloc = new_alloc;
					}
					memcpyc(&prev_block->bb_exits.jds_list[prev_block->bb_exits.jds_size],
					        &block->bb_exits.jds_list[0],
					        need, sizeof(struct Dee_jump_descriptor *));
					prev_block->bb_exits.jds_size += need;
					block->bb_exits.jds_size = 0;
				}
			}
			prev_block->bb_next = block->bb_next;
		}

		/* Remove `block' */
		--self->fa_blockc;
		memmovedownc(&self->fa_blockv[i],
		             &self->fa_blockv[i + 1],
		             self->fa_blockc - i,
		             sizeof(struct Dee_basic_block *));
		Dee_basic_block_destroy(block);
	}
	return 0;
err:
	return -1;
}



DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_LOADER_C */
