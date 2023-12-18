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
#ifndef GUARD_DEX_HOSTASM_ASSEMBLER_C
#define GUARD_DEX_HOSTASM_ASSEMBLER_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/error.h>
#include <deemon/format.h>

#include <hybrid/unaligned.h>

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_basic_block_compile(struct Dee_basic_block *__restrict self,
                        struct Dee_function_assembler *__restrict assembler) {
	int result;
	struct Dee_function_generator generator;
	generator.fg_block     = self;
	generator.fg_assembler = assembler;
	generator.fg_state     = self->bb_mem_start;
	ASSERT(generator.fg_state);
	Dee_memstate_incref(generator.fg_state);
	result = Dee_function_generator_genall(&generator);
	Dee_memstate_decref(generator.fg_state);
	ASSERT(result != 0 || self->bb_mem_end != NULL);
	return result;
}

/* Given the set of all basic block that have yet to be compiled,
 * find the one that has the most entry descriptors with a defined
 * `jd_stat' and return that one. */
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
find_not_compiled_block_with_most_defined_entry_points(struct Dee_function_assembler *__restrict self) {
	size_t result_index = 0;
	size_t i, result_missing_entries = (size_t)-1;
	for (i = 1; i < self->fa_blockc; ++i) {
		size_t entry_i, num_missing_entries;
		struct Dee_basic_block *block = self->fa_blockv[i];
		if (block->bb_mem_end != NULL)
			continue; /* Already compiled. */
		num_missing_entries = 0;

		/* Special case for when the block can only be reached via fallthru (shouldn't happen) */
		if (block->bb_entries.jds_size == 0) {
			ASSERTF(self->fa_blockv[i - 1]->bb_next,
			        "That would mean the block is entirely unreachable. "
			        /**/ "If that were the case, then `Dee_function_assembler_trimunused()' "
			        /**/ "should have removed it!");
			if (self->fa_blockv[i - 1]->bb_mem_end == NULL)
				++num_missing_entries;
		}
		for (entry_i = 0; entry_i < block->bb_entries.jds_size; ++entry_i) {
			struct Dee_jump_descriptor *entry = block->bb_entries.jds_list[entry_i];
			if (entry->jd_stat == NULL)
				++num_missing_entries;
		}
		if (num_missing_entries == 0)
			return i; /* No missing entries -> compile this one next! */
		if (result_missing_entries > num_missing_entries) {
			result_missing_entries = num_missing_entries;
			result_index           = i;
		}
	}
	ASSERTF(result_index == 0 ||
	        result_missing_entries < self->fa_blockv[result_index]->bb_entries.jds_size,
	        "There must be at least 1 present entry, else the block itself would "
	        "be completely unreachable, which is impossible because the loader created "
	        "it because it found a label that points to it.");
	return result_index;
}



#ifdef HOSTASM_X86_64_SYSVABI
INTDEF size_t const _Dee_function_assembler_cfa_addend[HOSTFUNC_CC_COUNT];
INTERN_CONST size_t const _Dee_function_assembler_cfa_addend[HOSTFUNC_CC_COUNT] = {
#define N_ARGS(n) ((n) * 8)
	/* [HOSTFUNC_CC_CALL]              = */ N_ARGS(2), /* DREF DeeObject *(DCALL *)(size_t argc, DeeObject *const *argv); */
	/* [HOSTFUNC_CC_CALL_KW]           = */ N_ARGS(3), /* DREF DeeObject *(DCALL *)(size_t argc, DeeObject *const *argv, DeeObject *kw); */
	/* [HOSTFUNC_CC_THISCALL]          = */ N_ARGS(3), /* DREF DeeObject *(DCALL *)(DeeObject *self, size_t argc, DeeObject *const *argv); */
	/* [HOSTFUNC_CC_THISCALL_KW]       = */ N_ARGS(4), /* DREF DeeObject *(DCALL *)(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw); */
	/* [HOSTFUNC_CC_CALL_TUPLE]        = */ N_ARGS(1), /* DREF DeeObject *(DCALL *)(DeeObject *args); */
	/* [HOSTFUNC_CC_CALL_TUPLE_KW]     = */ N_ARGS(2), /* DREF DeeObject *(DCALL *)(DeeObject *args, DeeObject *kw); */
	/* [HOSTFUNC_CC_THISCALL_TUPLE]    = */ N_ARGS(2), /* DREF DeeObject *(DCALL *)(DeeObject *self, DeeObject *args); */
	/* [HOSTFUNC_CC_THISCALL_TUPLE_KW] = */ N_ARGS(3), /* DREF DeeObject *(DCALL *)(DeeObject *self, DeeObject *args, DeeObject *kw); */
#undef N_ARGS
};
#endif /* !HOSTASM_X86_64_SYSVABI */

/* Step #4: Compile basic blocks and determine memory states. Fills in:
 * - self->fa_blockv[*]->bb_entries.jds_list[*]->jd_stat
 * - self->fa_blockv[*]->bb_mem_start->* (everything not already done by `Dee_function_assembler_loadboundlocals()')
 * - self->fa_blockv[*]->bb_mem_end
 * - self->fa_blockv[*]->bb_host_start
 * - self->fa_blockv[*]->bb_host_end
 * Also makes sure that memory states at start/end of basic blocks are
 * always identical (or compatible; i.e.: MEMLOC_F_LOCAL_UNKNOWN can
 * be set at the start of a block, but doesn't need to be set at the
 * end of a preceding block). When not compatible, extra block(s) are
 * inserted with `bb_deemon_start==bb_deemon_end', but non-empty host
 * assembly, which serves the purpose of transforming memory states.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_compileblocks(struct Dee_function_assembler *__restrict self) {
	size_t block_i;
	struct Dee_memstate *state;
	struct Dee_basic_block *block;
	ASSERT(self->fa_blockc >= 1);
	block = self->fa_blockv[0];
	ASSERT(block);
	state = block->bb_mem_start;
	ASSERT(state);

	/* Save caller-provided arguments onto stack. */
#ifdef HOSTASM_X86_64
	{
		PRIVATE Dee_host_register_t const truearg_regno[4] = {
			HOST_REGISTER_R_ARG0,
			HOST_REGISTER_R_ARG1,
			HOST_REGISTER_R_ARG2,
			HOST_REGISTER_R_ARG0,
		};

		Dee_hostfunc_cc_t cc = self->fa_cc;
		size_t trueargc = 0;
		if (cc & HOSTFUNC_CC_F_THIS) {
			state->ms_regs[HOST_REGISTER_R_ARG0] = REGISTER_USAGE_THIS;
			trueargc += 1;
		}
		if (cc & HOSTFUNC_CC_F_TUPLE) {
			state->ms_regs[truearg_regno[trueargc]] = REGISTER_USAGE_ARGS;
			trueargc += 1;
		} else {
			state->ms_regs[truearg_regno[trueargc]] = REGISTER_USAGE_ARGC;
			trueargc += 1;
			state->ms_regs[truearg_regno[trueargc]] = REGISTER_USAGE_ARGV;
			trueargc += 1;
		}
		if (cc & HOSTFUNC_CC_F_KW) {
			state->ms_regs[truearg_regno[trueargc]] = REGISTER_USAGE_KW;
			trueargc += 1;
		}
#ifdef HOSTASM_X86_64_SYSVABI
		/* Push arguments onto stack */
		if (trueargc >= 4 && _Dee_basic_block_ghstack_pushreg(block, HOST_REGISTER_R_ARG3))
			goto err;
		if (trueargc >= 3 && _Dee_basic_block_ghstack_pushreg(block, HOST_REGISTER_R_ARG2))
			goto err;
		if (trueargc >= 2 && _Dee_basic_block_ghstack_pushreg(block, HOST_REGISTER_R_ARG1))
			goto err;
		if (_Dee_basic_block_ghstack_pushreg(block, HOST_REGISTER_R_ARG0))
			goto err;
		ASSERT(_Dee_function_assembler_cfa_addend[cc] == trueargc * HOST_SIZEOF_POINTER);
#elif defined(HOSTASM_X86_64_MSABI)
		/* Save register arguments in caller-allocated locations */
		if (_Dee_basic_block_gmov_reg2hstack(block, HOST_REGISTER_R_ARG0, HOST_SIZEOF_POINTER * 1))
			goto err;
		if (trueargc >= 2 && _Dee_basic_block_gmov_reg2hstack(block, HOST_REGISTER_R_ARG1, HOST_SIZEOF_POINTER * 2))
			goto err;
		if (trueargc >= 3 && _Dee_basic_block_gmov_reg2hstack(block, HOST_REGISTER_R_ARG2, HOST_SIZEOF_POINTER * 3))
			goto err;
		if (trueargc >= 4 && _Dee_basic_block_gmov_reg2hstack(block, HOST_REGISTER_R_ARG3, HOST_SIZEOF_POINTER * 4))
			goto err;
#endif /* ... */
	}
#endif /* HOSTASM_X86_64 */

	/* Allocate stack space for local variables that are used before they're initialized. */
	{
		uint16_t lid;
		for (lid = 0; lid < state->ms_localc; ++lid) {
			if (state->ms_localv[lid].ml_where == MEMLOC_TYPE_HSTACK) {
				uintptr_t loc = Dee_memstate_hstack_alloca(state, HOST_SIZEOF_POINTER);
				state->ms_localv[lid].ml_value.ml_hstack = loc;
			}
		}
	}

	/* Adjust host SP for pre-allocated locals. */
	if (state->ms_host_cfa_offset != 0) {
		int temp;
#ifdef HOSTASM_STACK_GROWS_DOWN
		temp = _Dee_basic_block_ghstack_adjust(block, -(ptrdiff_t)state->ms_host_cfa_offset);
#else /* HOSTASM_STACK_GROWS_DOWN */
		temp = _Dee_basic_block_ghstack_adjust(block, (ptrdiff_t)state->ms_host_cfa_offset);
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		if unlikely(temp)
			goto err;
	}

	/* Compile the first block. */
	if unlikely(Dee_basic_block_compile(block, self))
		goto err;

	/* Compile all of the other blocks. */
	while ((block_i = find_not_compiled_block_with_most_defined_entry_points(self)) != 0) {
		block = self->fa_blockv[block_i];
		ASSERT(block->bb_mem_start != NULL);
		ASSERT(block->bb_mem_end == NULL);

		/* Check for special case where the preceding block falls through into this one. */
		{
			uint16_t lid;
			struct Dee_basic_block *prev = self->fa_blockv[block_i - 1];
			struct Dee_memstate *initial = prev->bb_mem_end;
			struct Dee_memstate *mem_start;
			ASSERT(prev->bb_next == NULL ||
			       prev->bb_next == block);
			if (prev->bb_next != block)
				initial = NULL;
			if (initial == NULL) {
				size_t entry_i;
				for (entry_i = 0; entry_i < block->bb_entries.jds_size; ++entry_i) {
					struct Dee_jump_descriptor *entry;
					entry = block->bb_entries.jds_list[entry_i];
					if (entry->jd_stat)
						initial = entry->jd_stat;
				}
			}
			ASSERT(initial != NULL);
			if unlikely(Dee_memstate_unshare(&block->bb_mem_start))
				goto err;

			/* Fill in the initial mem-state of the block. */
			mem_start = block->bb_mem_start;
			ASSERTF(mem_start->ms_stackc == 0,
			        "Should have been set like this by `Dee_function_assembler_loadboundlocals()'");
			ASSERT(mem_start->ms_localc == initial->ms_localc);
			if unlikely(Dee_memstate_reqvstack(mem_start, initial->ms_stackc))
				goto err;
			memcpyc(mem_start->ms_stackv, initial->ms_stackv,
			        initial->ms_stackc, sizeof(struct Dee_memloc));
			memcpy(mem_start->ms_regs, initial->ms_regs, sizeof(initial->ms_regs));
			mem_start->ms_host_cfa_offset = initial->ms_host_cfa_offset;
			mem_start->ms_stackc = initial->ms_stackc;
			for (lid = 0; lid < initial->ms_localc; ++lid) {
				struct Dee_memloc *dst = &mem_start->ms_localv[lid];
				struct Dee_memloc *src = &initial->ms_localv[lid];
				/* Important: *don't* inherit the is-bound of the local here.
				 * That state was already set by `Dee_function_assembler_loadboundlocals()' */
				dst->ml_flags &= MEMLOC_M_LOCAL_BSTATE;
				dst->ml_flags |= src->ml_flags & ~MEMLOC_M_LOCAL_BSTATE;
				dst->ml_where = src->ml_where;
				dst->ml_value._ml_data = src->ml_value._ml_data;
			}
		}

		/* Merge all other already-defined entry points into the block. */
		{
			size_t entry_i;
			for (entry_i = 0; entry_i < block->bb_entries.jds_size; ++entry_i) {
				struct Dee_jump_descriptor *entry;
				struct Dee_memstate *entry_state;
				entry       = block->bb_entries.jds_list[entry_i];
				entry_state = entry->jd_stat;
				if (entry_state != NULL) {
					if unlikely(block->bb_mem_start->ms_stackc != entry_state->ms_stackc) {
						DeeError_Throwf(&DeeError_IllegalInstruction,
						                "Conflicting stack depth at +%.4" PRFx32 ": "
						                "both %" PRFu16 " and %" PRFu16 " matched",
						                Dee_function_assembler_addrof(self, block->bb_deemon_start),
						                block->bb_mem_start->ms_stackc, entry_state->ms_stackc);
						goto err;
					}
					Dee_memstate_constrainwith(block->bb_mem_start, entry_state);
				}
			}
		}

		/* Try to deallocate unused stack memory. */
		Dee_memstate_hstack_free(block->bb_mem_start);

		/* Compile this block. */
		if unlikely(Dee_basic_block_compile(block, self))
			goto err;
	}

	return 0;
err:
	return -1;
}

/* Step #5: Link blocks into an executable function blob.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_assembler_linkblocks(struct Dee_function_assembler *__restrict self,
                                  struct Dee_hostfunc *__restrict result) {
	/* TODO */
	(void)self;
	(void)result;
	return DeeError_NOTIMPLEMENTED();
}

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_ASSEMBLER_C */
