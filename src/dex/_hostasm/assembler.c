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
#include <deemon/alloc.h>
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
	generator.fg_sect      = &self->bb_htext;
	generator.fg_assembler = assembler;
	generator.fg_state     = self->bb_mem_start;
	ASSERT(generator.fg_state);
	Dee_memstate_incref(generator.fg_state);
	result = Dee_function_generator_genall(&generator);
	Dee_memstate_decref(generator.fg_state);
	ASSERT(generator.fg_block == self);
	ASSERT(generator.fg_sect == &self->bb_htext);
	ASSERT(generator.fg_assembler == assembler);
	return result;
}

/* Given the set of all basic block that have yet to be compiled,
 * find the one that has the most entry descriptors with a defined
 * `jd_stat' and return that one.
 * @return: (size_t)-1: Everything has been compiled. */
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
find_next_block_to_compile(struct Dee_function_assembler *__restrict self) {
	size_t result_index = (size_t)-1;
	size_t i, result_missing_entries = (size_t)-1;
	for (i = 0; i < self->fa_blockc; ++i) {
		size_t j, num_missing_entries;
		struct Dee_basic_block *block = self->fa_blockv[i];
		if (block->bb_mem_start == NULL)
			continue; /* Block hasn't been reached at all, yet. */
		if (block->bb_mem_end != NULL)
			continue; /* Already compiled. */
		if (i == 0)
			return 0; /* Always compile the entry-block first! */
		num_missing_entries = 0;
		for (j = 0; j < block->bb_entries.jds_size; ++j) {
			struct Dee_jump_descriptor *entry = block->bb_entries.jds_list[j];
			if (entry->jd_stat == NULL)
				++num_missing_entries;
		}
		for (j = 0; j < self->fa_blockc; ++j) {
			struct Dee_basic_block *block2 = self->fa_blockv[j];
			if (block2->bb_next == block && block2->bb_mem_end == NULL)
				++num_missing_entries;
		}
		for (j = 0; j < self->fa_except_exitc; ++j) {
			struct Dee_basic_block *block2 = self->fa_except_exitv[j]->exi_block;
			if (block2->bb_next == block && block2->bb_mem_end == NULL)
				++num_missing_entries;
		}
		if (num_missing_entries == 0)
			return i; /* No missing entries -> compile this one next! */
		if (result_missing_entries > num_missing_entries) {
			result_missing_entries = num_missing_entries;
			result_index           = i;
		}
	}
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
	size_t local_count;
	ASSERT(self->fa_blockc >= 1);
	block = self->fa_blockv[0];
	ASSERT(block);
	ASSERT(block->bb_mem_start == NULL);

	/* Figure out how many locals we need. */
	local_count = self->fa_code->co_localc;
	local_count += DEE_MEMSTATE_EXTRA_LOCALS_MINCOUNT;
	local_count += self->fa_code->co_argc_max;
	local_count -= self->fa_code->co_argc_max;

	/* Setup the state of the function's first (entry) block. */
	state = Dee_memstate_alloc(local_count);
	if unlikely(!state)
		goto err;
	state->ms_refcnt = 1;
	state->ms_host_cfa_offset = 0;
	state->ms_localc = local_count;
	state->ms_stackc = 0;
	state->ms_stacka = 0;
	bzero(state->ms_rinuse, sizeof(state->ms_rinuse));
	Dee_memstate_hregs_clear_usage(state);
	state->ms_stackv = NULL;

	/* Initially, all variables are unbound */
	{
		size_t lid;
		for (lid = 0; lid < state->ms_localc; ++lid) {
			state->ms_localv[lid].ml_flags = MEMLOC_F_NOREF | MEMLOC_F_LOCAL_UNBOUND;
			state->ms_localv[lid].ml_type = MEMLOC_TYPE_UNALLOC;
		}
	}

	/* Set the mem-state for the initial block. */
	block->bb_mem_start = state; /* Inherit reference */

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
			state->ms_rusage[HOST_REGISTER_R_ARG0] = DEE_HOST_REGUSAGE_THIS;
			trueargc += 1;
		}
		if (cc & HOSTFUNC_CC_F_TUPLE) {
			state->ms_rusage[truearg_regno[trueargc]] = DEE_HOST_REGUSAGE_ARGS;
			trueargc += 1;
		} else {
			state->ms_rusage[truearg_regno[trueargc]] = DEE_HOST_REGUSAGE_ARGC;
			trueargc += 1;
			state->ms_rusage[truearg_regno[trueargc]] = DEE_HOST_REGUSAGE_ARGV;
			trueargc += 1;
		}
		if (cc & HOSTFUNC_CC_F_KW) {
			state->ms_rusage[truearg_regno[trueargc]] = DEE_HOST_REGUSAGE_KW;
			trueargc += 1;
		}

#if 0 /* TODO: This can't go in the first basic block; this needs to go into
       *       a special prolog section that is generated during linking! */
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
#endif
	}
#endif /* HOSTASM_X86_64 */

	/* Compile all basic blocks until everything has been compiled and everyone is happy. */
	while ((block_i = find_next_block_to_compile(self)) != (size_t)-1) {
		block = self->fa_blockv[block_i];
		Dee_DPRINTF("Dee_function_assembler_compileblocks: %" PRFuSIZ " [%.4" PRFx32 "-%.4" PRFx32 "]\n",
		            block_i,
		            Dee_function_assembler_addrof(self, block->bb_deemon_start),
		            Dee_function_assembler_addrof(self, block->bb_deemon_end - 1));
#ifndef NO_HOSTASM_DEBUG_PRINT
		_Dee_memstate_debug_print(block->bb_mem_start);
#endif /* !NO_HOSTASM_DEBUG_PRINT */
		ASSERT(block->bb_mem_start != NULL);
		ASSERT(block->bb_mem_end == NULL);
		ASSERT(block->bb_htext.hs_end == block->bb_htext.hs_start);
		ASSERT(block->bb_htext.hs_relc == 0);
		ASSERT(block->bb_hcold.hs_end == block->bb_hcold.hs_start);
		ASSERT(block->bb_hcold.hs_relc == 0);

		/* Compile this block. */
		if unlikely(Dee_basic_block_compile(block, self))
			goto err;
	}

	return 0;
err:
	return -1;
}


struct host_section_set {
	struct Dee_host_section **hss_list; /* [1..1][0..hss_size] List of basic blocks (sorted by pointer) */
	size_t                   hss_size; /* # of items in `hss_list' */
#ifndef NDEBUG
	size_t                   hss_smax; /* Max # of items in `hss_list' */
#endif /* !NDEBUG */
};

#ifdef NDEBUG
#define _host_section_set_init_set_smax(self, smax) (void)0
#else /* NDEBUG */
#define _host_section_set_init_set_smax(self, smax) ((self)->hss_smax = (smax))
#endif /* !NDEBUG */
#define host_section_set_fini(self) Dee_Freea((self)->hss_list)
#define host_section_set_init(self, smax)         \
	(_host_section_set_init_set_smax(self, smax), \
	 (self)->hss_size = 0,                        \
	 ((self)->hss_list = (struct Dee_host_section **)Dee_Mallocac(smax, sizeof(struct Dee_host_section *))) != NULL ? 0 : -1)


/* Try to insert `sect' into `self' (if it wasn't inserted already) */
PRIVATE NONNULL((1, 2)) void DCALL
host_section_set_insert(struct host_section_set *__restrict self,
                        struct Dee_host_section *sect) {
	size_t lo = 0, hi = self->hss_size;
	ASSERT(sect);
	while (lo < hi) {
		size_t mid = (lo + hi) / 2;
		struct Dee_host_section *ex = self->hss_list[mid];
		if (sect < ex) {
			hi = mid;
		} else if (sect > ex) {
			lo = mid + 1;
		} else {
			return; /* Already inserted. */
		}
	}
#ifndef NDEBUG
	ASSERTF(self->hss_size < self->hss_smax,
	        "This would mean that there are more "
	        "sections than are known to the assembler");
#endif /* !NDEBUG */
	ASSERT(lo == hi);
	memmoveupc(&self->hss_list[lo + 1],
	           &self->hss_list[lo],
	           self->hss_size - lo,
	           sizeof(struct Dee_host_section *));
	self->hss_list[lo] = sect;
	++self->hss_size;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
host_section_set_contains(struct host_section_set const *__restrict self,
                          struct Dee_host_section *sect) {
	size_t lo = 0, hi = self->hss_size;
	while (lo < hi) {
		size_t mid = (lo + hi) / 2;
		struct Dee_host_section *ex = self->hss_list[mid];
		if (sect < ex) {
			hi = mid;
		} else if (sect > ex) {
			lo = mid + 1;
		} else {
			return true;
		}
	}
	return false;
}

PRIVATE NONNULL((1, 2)) void DCALL
host_section_set_insertall_from_relocations(struct host_section_set *__restrict self,
                                            struct Dee_host_section *sect) {
	size_t i;
	for (i = 0; i < sect->hs_relc; ++i) {
		struct Dee_host_reloc *rel = &sect->hs_relv[i];
		switch (rel->hr_vtype) {
		case DEE_HOST_RELOCVALUE_SYM: {
			struct Dee_host_symbol const *sym = rel->hr_value.rv_sym;
			switch (sym->hs_type) {
#if 0 /* We only care about sections that might be used for exception handling! */
			case DEE_HOST_SYMBOL_JUMP:
				host_section_set_insert(self, &sym->hs_value.sv_jump->jd_morph);
				host_section_set_insert(self, &sym->hs_value.sv_jump->jd_to->bb_htext);
				break;
#endif
			case DEE_HOST_SYMBOL_SECT:
				host_section_set_insert(self, sym->hs_value.sv_sect.ss_sect);
				break;
			default: break;
			}
		}	break;
		case DEE_HOST_RELOCVALUE_SECT:
			host_section_set_insert(self, rel->hr_value.rv_sect);
			break;

		default:
			break;
		}
	}
}


/* Step #3: Trim basic blocks (and parts thereof) that turned out to be unreachable.
 * This extra step is needed for when part of a basic block only turns out to be
 * unreachable during the compileblocks-phase, as would be the case in code like this:
 *       >>     push true
 *       >>     jf   pop, 1f
 *       >>     ret
 *       >> 1:  print @"NEVER REACHED", nl
 *       >>     ret
 * This function also gets rid of exception handling basic blocks that are never used
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_trimdead(struct Dee_function_assembler *__restrict self) {
	struct host_section_set referenced_sections;
	size_t i;

	/* Note how we skip the primary block here.
	 * That's because it could never not have been compiled. */
	ASSERT(self->fa_blockc >= 1);
	ASSERT(self->fa_blockv[0]);
	ASSERT(self->fa_blockv[0]->bb_mem_start != NULL);
	ASSERT(self->fa_blockv[0]->bb_mem_end != NULL);
	for (i = 1; i < self->fa_blockc;) {
		struct Dee_basic_block *block = self->fa_blockv[i];
		ASSERT((block->bb_mem_start != NULL) == (block->bb_mem_end != NULL));
		if (block->bb_mem_start == NULL) {
			/* Fully get rid of this block. */
			--self->fa_blockc;
			memmovedownc(&self->fa_blockv[i],
			             &self->fa_blockv[i + 1],
			             self->fa_blockc - i,
			             sizeof(struct Dee_basic_block *));
			Dee_basic_block_destroy(block);
			continue;
		}
		if (block->bb_deemon_end < block->bb_deemon_end_r) {
			/* Block ends with a late-detected unreachable instruction.
			 * In this case, it is possible that there are further jump
			 * descriptors for parts of the code that hasn't been compiled,
			 * which we must get rid of now. */
			Dee_basic_block_trim_unused_exits(block);
		}
		++i;
	}

	/* Go through all blocks and check which exception exit info blocks
	 * are actually still being used, and remove those that aren't. */
	if unlikely(host_section_set_init(&referenced_sections,
	                                  (self->fa_blockc * 2) +
	                                  self->fa_except_exitc))
		goto err;
	for (i = 0; i < self->fa_blockc; ++i) {
		struct Dee_basic_block *block = self->fa_blockv[i];
		ASSERT(block);
		ASSERT(block->bb_mem_start);
		ASSERT(block->bb_mem_end);
		host_section_set_insertall_from_relocations(&referenced_sections, &block->bb_htext);
		host_section_set_insertall_from_relocations(&referenced_sections, &block->bb_hcold);
	}

	/* Go through exception handlers and remove those that aren't referenced. */
	for (i = 0; i < self->fa_except_exitc;) {
		struct Dee_except_exitinfo *info = self->fa_except_exitv[i];
		struct Dee_basic_block *block = info->exi_block;

		/* Note that the cold section of exception handlers is never referenced by code
		 * blocks, which is why we only allocate 1 slot for each except exit in the set
		 * above, and only check the bb_htext section here. */
		if (host_section_set_contains(&referenced_sections, &block->bb_htext)) {
			++i;
			continue;
		}

		/* Unreferenced exit info blob -> get rid of it! */
		Dee_except_exitinfo_destroy(info);
		--self->fa_except_exitc;
		memmovedownc(&self->fa_except_exitv[i],
		             &self->fa_except_exitv[i + 1],
		             self->fa_except_exitc - i,
		             sizeof(struct Dee_except_exitinfo *));
	}
	host_section_set_fini(&referenced_sections);

	return 0;
err:
	return -1;
}

/* Step #4: Generate morph instruction sequences to perform memory state transitions.
 * This also extends the host text of basic blocks that fall through to some
 * other basic block with an extra instructions needed for morphing:
 * - self->fa_prolog
 * - self->fa_blockv[*]->bb_exits.jds_list[*]->jd_morph
 * - self->fa_blockv[*]->bb_htext        (extend with transition code so that `bb_mem_end == bb_next->bb_mem_start')
 * - self->fa_except_exitv[*]->bb_htext  (generate morph-code to transition to an empty stack, or fall into another exit block)
 * - self->fa_except_exitv[*]->bb_next   (set if intend is to fall into another exit block)
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_compilemorph(struct Dee_function_assembler *__restrict self) {
	/* TODO */
	(void)self;
	return DeeError_NOTIMPLEMENTED();
}

/* Step #5: Generate missing unconditional jumps to jump from one block to the next
 * - Find loops of blocks that "fall through" back on each other in a loop, and
 *   append a jump-to-the-start on all blocks that "fall through" to themselves.
 *   For one of these blocks, also generate a call to `DeeThread_CheckInterrupt()'
 * - For all blocks that have more than 1 fallthru predecessors, take all but
 *   1 of those predecessors and append unconditional jumps to them, then set
 *   the `bb_next' field of those blocks to `NULL'.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_stitchblocks(struct Dee_function_assembler *__restrict self) {
	/* TODO */
	(void)self;
	return DeeError_NOTIMPLEMENTED();
}

/* Step #6: Link blocks into an executable function blob.
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
