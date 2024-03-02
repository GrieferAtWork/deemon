/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
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
#include <deemon/alloc.h>
#include <deemon/asm.h>
#include <deemon/error.h>
#include <deemon/format.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/unaligned.h>

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
register_jump(struct function_assembler *__restrict self,
              Dee_instruction_t const *from,
              Dee_instruction_t const *to) {
	struct basic_block *from_block;
	struct basic_block *to_block;
	struct jump_descriptor *jump;

	/* Make sure that a basic block begins at `to' */
	to_block = function_assembler_splitblock(self, to);
	if unlikely(!to_block)
		goto err;
	ASSERT(to_block->bb_deemon_start == to);
	from_block = function_assembler_locateblock(self, from);
	ASSERT(from_block);
	ASSERT(from >= from_block->bb_deemon_start &&
	       from < from_block->bb_deemon_end);

	/* Allocate a descriptor for the jump. */
	jump = jump_descriptor_alloc();
	if unlikely(!jump)
		goto err;
	jump->jd_from = from;
	jump->jd_to   = to_block;
	jump->jd_stat = NULL; /* Filled in later... */
	host_section_init(&jump->jd_morph);

	/* Insert the jump into the from->exit and to->entry tables. */
	if unlikely(jump_descriptors_insert(&to_block->bb_entries, jump))
		goto err_jump;
	if unlikely(jump_descriptors_insert(&from_block->bb_exits, jump))
		goto err; /* `to_block->bb_entries' owns the jump at this point. */

	return 0;
err_jump:
	jump_descriptor_free(jump);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
scan_and_split_blocks(struct function_assembler *__restrict self,
                      Dee_instruction_t const *iter,
                      Dee_instruction_t const *end) {
	for (; iter < end; iter = DeeAsm_NextInstr(iter)) {
		iter = DeeAsm_SkipPrefix(iter);
		switch (iter[0]) {

		case ASM_JF:
		case ASM_JT:
		case ASM_JMP:
		case ASM_FOREACH: {
			int8_t delta = *(int8_t const *)(iter + 1);
			Dee_instruction_t const *to = iter + 2 + delta;
			if unlikely(register_jump(self, iter, to))
				goto err;
		}	break;

		case ASM_JF16:
		case ASM_JT16:
		case ASM_JMP16:
		case ASM_FOREACH16: {
			int16_t delta = (int16_t)UNALIGNED_GETLE16(iter + 1);
			Dee_instruction_t const *to = iter + 3 + delta;
			if unlikely(register_jump(self, iter, to))
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
				int32_t delta = (int32_t)UNALIGNED_GETLE32(iter + 2);
				Dee_instruction_t const *to = iter + 6 + delta;
				if unlikely(register_jump(self, iter, to))
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

PRIVATE WUNUSED NONNULL((1)) bool DCALL
scan_block_for_noreturn(struct basic_block *__restrict block, uint16_t code_flags) {
	Dee_instruction_t const *iter;
	for (iter = block->bb_deemon_start;
	     iter < block->bb_deemon_end;
	     iter = DeeAsm_NextInstr(iter)) {
		uint16_t opcode;
		iter   = DeeAsm_SkipPrefix(iter);
		opcode = iter[0];
		if (ASM_ISEXTENDED(opcode))
			opcode = (opcode << 8) | iter[1];
		switch (opcode) {

		case ASM_JMP:
		case ASM_JMP16:
		case ASM32_JMP: {
			/* Convert unconditional jump -> fallthru */
			struct jump_descriptor *desc;
			desc = jump_descriptors_lookup(&block->bb_exits, iter);
			ASSERTF(desc, "Jump at %p should have been found by the loader", iter);
			block->bb_next = desc->jd_to;
			block->bb_deemon_end = iter;
			return true;
		}	break;

		default:
			if (DeeAsm_IsNoreturn(opcode, code_flags)) {
				block->bb_deemon_end = DeeAsm_NextInstr(iter);
				block->bb_next = NULL;
				return true;
			}
			break;
		}
	}
	return false;
}


/* Remove exits from `self' that have origins beyond `self->bb_deemon_end' */
INTERN NONNULL((1)) void DCALL
basic_block_trim_unused_exits(struct basic_block *__restrict self) {
	size_t exit_count = self->bb_exits.jds_size;
	while (exit_count > 0) {
		struct jump_descriptor *jmp;
		jmp = self->bb_exits.jds_list[exit_count - 1];
		ASSERT(jmp);
		if (jmp->jd_from < self->bb_deemon_end)
			break;
		--exit_count;
	}
	ASSERT(exit_count <= self->bb_exits.jds_size);
	while (exit_count < self->bb_exits.jds_size) {
		/* Get rid of trimmed exits! */
		struct jump_descriptor *unused_jmp;
		--self->bb_exits.jds_size;
		unused_jmp = self->bb_exits.jds_list[self->bb_exits.jds_size];
		ASSERT(unused_jmp->jd_to != NULL);
		ASSERT(unused_jmp->jd_stat == NULL);
		jump_descriptors_remove(&unused_jmp->jd_to->bb_entries, unused_jmp);
		jump_descriptor_destroy(unused_jmp);
	}
}


/* Step #1: Load basic blocks. Fills in:
 * - self->fa_blockc
 * - self->fa_blockv
 * - self->fa_blockv[*]->bb_deemon_start
 * - self->fa_blockv[*]->bb_deemon_end
 * - self->fa_blockv[*]->bb_entries+bb_exits
 * - self->fa_blockv[*]->bb_entries.jds_list[*]->jd_from
 * - self->fa_blockv[*]->bb_entries.jds_list[*]->jd_to
 * - self->fa_blockv[*]->bb_next
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
function_assembler_loadblocks(struct function_assembler *__restrict self) {
	size_t i;
	struct basic_block *block;
	ASSERT(self->fa_blockc == 0);

	if (self->fa_code->co_flags & CODE_FYIELDING)
		return DeeError_Throwf(&DeeError_IllegalInstruction, "Cannot re-compile yield function");
	if (self->fa_code->co_exceptc > 0)
		return DeeError_Throwf(&DeeError_IllegalInstruction, "Cannot re-compile function with exception handlers");

	/* Start out with a single block that spans the entire code object.
	 * This block will be split into smaller ones as we scan the code. */
	if (!self->fa_blocka) {
		self->fa_blocka = 8;
		self->fa_blockv = (struct basic_block **)Dee_TryMallocc(8, sizeof(struct basic_block *));
		if (!self->fa_blockv) {
			self->fa_blocka = 1;
			self->fa_blockv = (struct basic_block **)Dee_Mallocc(1, sizeof(struct basic_block *));
			if unlikely(!self->fa_blockv)
				goto err;
		}
	}

	/* Initialize the initial block. */
	block = basic_block_alloc(self->fa_xlocalc);
	if unlikely(!block)
		goto err;
	basic_block_init_common(block);
	block->bb_deemon_start = self->fa_code->co_code;
	block->bb_deemon_end   = self->fa_code->co_code + self->fa_code->co_codebytes;
	jump_descriptors_init(&block->bb_exits);

	/* Set the initial block. */
	self->fa_blockv[0] = block;
	self->fa_blockc    = 1;

	/* Scan and split deemon code. */
	if unlikely(scan_and_split_blocks(self,
	                                  block->bb_deemon_start,
	                                  block->bb_deemon_end))
		goto err;

	/* Assign block exits (for fallthru). */
	for (i = 0; i < self->fa_blockc - 1; ++i) {
		self->fa_blockv[i]->bb_next = self->fa_blockv[i + 1];
	}

	/* Search basic blocks for noreturn instructions.
	 * When one such instruction is found, the block ends on that instruction,
	 * and has its `bb_next' field set to `NULL' and its end trimmed. */
	for (i = 0; i < self->fa_blockc; ++i) {
		bool has_noreturn;
		block = self->fa_blockv[i];
		ASSERT(block);
		has_noreturn = scan_block_for_noreturn(block, self->fa_code->co_flags);
		block->bb_deemon_end_r = block->bb_deemon_end;
		block->bb_next_r       = block->bb_next;
		if (has_noreturn)
			basic_block_trim_unused_exits(block);
	}

	/* Check if there are blocks that are entirely unreachable, and merge
	 * blocks that can only be reached via fallthru with their predecessor. */
	for (i = 1; i < self->fa_blockc;) {
		struct basic_block *prev_block;
		block = self->fa_blockv[i];
		if (block->bb_entries.jds_size > 0) {
continue_with_next_block:
			++i;
			continue;
		}

		/* Check if some other block other than "prev_block" wants to fall into "block" */
		{
			size_t j;
			for (j = 0; j < self->fa_blockc; ++j) {
				struct basic_block *other_block = self->fa_blockv[j];
				if (j == i - 1)
					continue;
				if (other_block->bb_next == block)
					goto continue_with_next_block;
			}
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
						struct jump_descriptor **new_list;
						size_t min_alloc = prev_block->bb_exits.jds_size + need;
						size_t new_alloc = prev_block->bb_exits.jds_alloc * 2;
						if (new_alloc < min_alloc)
							new_alloc = min_alloc;
						new_list = (struct jump_descriptor **)Dee_TryReallocc(prev_block->bb_exits.jds_list,
						                                                      new_alloc,
						                                                      sizeof(struct jump_descriptor *));
						if unlikely(!new_list) {
							new_alloc = min_alloc;
							new_list = (struct jump_descriptor **)Dee_Reallocc(prev_block->bb_exits.jds_list,
							                                                   new_alloc,
							                                                   sizeof(struct jump_descriptor *));
							if unlikely(!new_list)
								goto err;
						}
						prev_block->bb_exits.jds_list  = new_list;
						prev_block->bb_exits.jds_alloc = new_alloc;
					}
					memcpyc(&prev_block->bb_exits.jds_list[prev_block->bb_exits.jds_size],
					        &block->bb_exits.jds_list[0],
					        need, sizeof(struct jump_descriptor *));
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
		             sizeof(struct basic_block *));
		basic_block_destroy(block);
	}

	return 0;
err:
	return -1;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_LOADER_C */
