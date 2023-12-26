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
	generator.fg_state_hstack_res = NULL;
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


PRIVATE WUNUSED NONNULL((1)) DREF struct Dee_memstate *DCALL
Dee_function_assembler_alloc_zero_memstate(struct Dee_function_assembler const *__restrict self) {
	struct Dee_memstate *result;
	size_t local_count;
	size_t extra_base;

	/* Figure out how many locals we need. */
	extra_base  = self->fa_localc;
	local_count = extra_base;
	local_count += DEE_MEMSTATE_EXTRA_LOCAL_MINCOUNT;
	local_count += self->fa_code->co_argc_max;
	local_count -= self->fa_code->co_argc_min;

	/* Setup the state of the function's first (entry) block. */
	result = Dee_memstate_alloc(local_count);
	if unlikely(!result)
		goto done;
	result->ms_refcnt = 1;
	result->ms_host_cfa_offset = 0;
	result->ms_localc = local_count;
	result->ms_stackc = 0;
	result->ms_stacka = 0;
	bzero(result->ms_rinuse, sizeof(result->ms_rinuse));
	Dee_memstate_hregs_clear_usage(result);
	result->ms_stackv = NULL;

	/* Initially, all variables are unbound */
	{
		size_t lid;
		for (lid = 0; lid < result->ms_localc; ++lid) {
			result->ms_localv[lid].ml_flags = MEMLOC_F_NOREF | MEMLOC_F_LOCAL_UNBOUND;
			result->ms_localv[lid].ml_type  = MEMLOC_TYPE_UNALLOC;
		}
	}
done:
	return result;
}

/* Allocate and return the initial memory-state when the
 * generated function is entered at the start of the prolog.
 * @return: * :   The initial memory-state
 * @return: NULL: Error */
PRIVATE WUNUSED NONNULL((1)) DREF struct Dee_memstate *DCALL
Dee_function_assembler_alloc_init_memstate(struct Dee_function_assembler const *__restrict self) {
	Dee_hostfunc_cc_t cc = self->fa_cc;
	struct Dee_memstate *state;
	ASSERT(self->fa_blockc >= 1);

	/* Figure out how many locals we need. */
	state = Dee_function_assembler_alloc_zero_memstate(self);
	if unlikely(!state)
		goto err;

	/* Set-up the mem-state to indicate where arguments are stored at */
#ifdef HOSTASM_X86
	{
#ifdef HOSTASM_X86_64
#define Dee_memloc_set_x86_arg(self, argi)                     \
	((self)->ml_flags = MEMLOC_F_NOREF | MEMLOC_F_LOCAL_BOUND, \
	 (self)->ml_type  = MEMLOC_TYPE_HREG,                      \
	 (self)->ml_value.v_hreg.r_regno = truearg_regno[argi],    \
	 (self)->ml_value.v_hreg.r_off   = 0,                      \
	 Dee_memstate_incrinuse(state, truearg_regno[argi]))
		PRIVATE Dee_host_register_t const truearg_regno[4] = {
			HOST_REGISTER_R_ARG0,
			HOST_REGISTER_R_ARG1,
			HOST_REGISTER_R_ARG2,
			HOST_REGISTER_R_ARG0,
		};
#else /* HOSTASM_X86_64 */
#define Dee_memloc_set_x86_arg(self, argi)                                          \
	((self)->ml_flags = MEMLOC_F_NOREF | MEMLOC_F_LOCAL_BOUND,                      \
	 (self)->ml_type  = MEMLOC_TYPE_HSTACKIND,                                      \
	 (self)->ml_value.v_hstack.s_cfa = (uintptr_t)(-(ptrdiff_t)(((argi) + 1) * 4)), \
	 (self)->ml_value.v_hstack.s_off = 0)
#endif /* !HOSTASM_X86_64 */
		size_t argi = 0, extra_base = self->fa_localc;
		if (cc & HOSTFUNC_CC_F_THIS) {
			Dee_memloc_set_x86_arg(&state->ms_localv[extra_base + DEE_MEMSTATE_EXTRA_LOCAL_A_THIS], argi);
			++argi;
		}
		if (cc & HOSTFUNC_CC_F_TUPLE) {
			Dee_memloc_set_x86_arg(&state->ms_localv[extra_base + DEE_MEMSTATE_EXTRA_LOCAL_A_ARGS], argi);
			++argi;
		} else {
			Dee_memloc_set_x86_arg(&state->ms_localv[extra_base + DEE_MEMSTATE_EXTRA_LOCAL_A_ARGC], argi);
			++argi;
			Dee_memloc_set_x86_arg(&state->ms_localv[extra_base + DEE_MEMSTATE_EXTRA_LOCAL_A_ARGV], argi);
			++argi;
		}
		if (cc & HOSTFUNC_CC_F_KW)
			Dee_memloc_set_x86_arg(&state->ms_localv[extra_base + DEE_MEMSTATE_EXTRA_LOCAL_A_KW], argi);
#undef Dee_memloc_set_x86_arg
	}
#else /* ... */
#error "Initial register state not implemented for this architecture"
#endif /* !... */

	return state;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_makeprolog(struct Dee_function_generator *__restrict self) {
	/* TODO: Check argc */
	/* TODO: Unpack keywords */
	(void)self;
	return 0;
}


/* Generate the prolog of the function (check argc, unpack keywords, etc...).
 * This fills in:
 * - self->fa_prolog
 * - self->fa_prolog_end
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_makeprolog(struct Dee_function_assembler *__restrict self,
                                  /*inherit(always)*/ DREF struct Dee_memstate *__restrict state) {
	int result;
	struct Dee_function_generator gen;
	gen.fg_assembler = self;
	gen.fg_block     = self->fa_blockv[0];
	gen.fg_sect      = &self->fa_prolog;
	gen.fg_state     = state; /* Inherit reference */
	gen.fg_state_hstack_res = NULL;
	ASSERT(gen.fg_block);
	ASSERT(self->fa_prolog_end == NULL);
	ASSERT(gen.fg_state != NULL);
	result = Dee_function_generator_makeprolog(&gen);
	ASSERT(gen.fg_state != NULL);
	ASSERT(self->fa_prolog_end == NULL);
	if likely(result == 0) {
		self->fa_prolog_end = gen.fg_state; /* Inherit reference */
	} else {
		Dee_memstate_decref(gen.fg_state);
	}
	return result;
}

/* Step #2: Compile basic blocks and determine memory states. Fills in:
 * - self->fa_prolog
 * - self->fa_prolog_end
 * - self->fa_blockv[*]->bb_entries.jds_list[*]->jd_stat
 * - self->fa_blockv[*]->bb_mem_start
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

	/* Setup the state of the function's first (entry) block. */
	state = Dee_function_assembler_alloc_init_memstate(self);
	if unlikely(!state)
		goto err;

	/* Generate the function prolog. */
	if unlikely(Dee_function_assembler_makeprolog(self, state))
		goto err;

	/* Set the mem-state for the initial block. */
	ASSERT(self->fa_prolog_end);
	block = self->fa_blockv[0];
	ASSERT(block);
	ASSERT(block->bb_mem_start == NULL);
	Dee_memstate_incref(self->fa_prolog_end);
	block->bb_mem_start = self->fa_prolog_end; /* Inherit reference */

	/* Compile all basic blocks until everything has been compiled and everyone is happy. */
	while ((block_i = find_next_block_to_compile(self)) != (size_t)-1) {
		block = self->fa_blockv[block_i];
		Dee_DPRINTF("Dee_basic_block_compile: %" PRFuSIZ " [%.4" PRFx32 "-%.4" PRFx32 "]\n",
		            block_i,
		            Dee_function_assembler_addrof(self, block->bb_deemon_start),
		            Dee_function_assembler_addrof(self, block->bb_deemon_end - 1));
#ifndef NO_HOSTASM_DEBUG_PRINT
		_Dee_memstate_debug_print(block->bb_mem_start, self, block->bb_deemon_start);
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
	size_t                    hss_size; /* # of items in `hss_list' */
#ifndef NDEBUG
	size_t                    hss_smax; /* Max # of items in `hss_list' */
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


/* Append morphing code for `from_state' to `to_state' to `sect' */
#undef assemble_morph
PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
assemble_morph(struct Dee_function_assembler *__restrict assembler,
               struct Dee_host_section *sect,
               struct Dee_memstate *from_state,
               struct Dee_basic_block *to_block
#ifndef Dee_DPRINT_IS_NOOP
               , Dee_instruction_t const *from_instr
#endif /* !Dee_DPRINT_IS_NOOP */
               ) {
#ifdef Dee_DPRINT_IS_NOOP
#define assemble_morph(a, b, c, d, e) assemble_morph(a, b, c, d)
#endif /* Dee_DPRINT_IS_NOOP */
	int result;
	struct Dee_function_generator gen;
	ASSERT(to_block->bb_mem_start);
	Dee_DPRINTF("Dee_function_generator_vmorph: %.4" PRFx32 " -> %.4" PRFx32 "\n",
	            Dee_function_assembler_addrof(assembler, from_instr),
	            to_block->bb_deemon_start < to_block->bb_deemon_end
	            ? Dee_function_assembler_addrof(assembler, to_block->bb_deemon_start)
	            : 0xffff);
#ifndef NO_HOSTASM_DEBUG_PRINT
	_Dee_memstate_debug_print(from_state, assembler, from_instr);
	_Dee_memstate_debug_print(to_block->bb_mem_start, assembler,
	                          to_block->bb_deemon_start < to_block->bb_deemon_end
	                          ? to_block->bb_deemon_start
	                          : NULL);
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	gen.fg_assembler = assembler;
	gen.fg_block     = to_block;
	gen.fg_sect      = sect;
	gen.fg_state     = from_state;
	gen.fg_state_hstack_res = NULL;
	Dee_memstate_incref(gen.fg_state);
	result = Dee_function_generator_vmorph(&gen, to_block->bb_mem_start);
	Dee_memstate_decref(gen.fg_state);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gretNULL(struct Dee_function_generator *__restrict self) {
	struct Dee_memloc zero;
	zero.ml_flags = MEMLOC_F_NORMAL;
	zero.ml_type  = MEMLOC_TYPE_CONST;
	zero.ml_value.v_const = NULL;
	return Dee_function_generator_gret(self, &zero);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gexcept_morph(struct Dee_function_generator *__restrict self,
                                     struct Dee_except_exitinfo *__restrict from,
                                     struct Dee_except_exitinfo *__restrict to) {
	(void)self;
	(void)from;
	(void)to;
	/* TODO */
	/* TODO: It's also possible to shift references from one location to another! */
	return DeeError_NOTIMPLEMENTED();
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_compileexcept(struct Dee_function_assembler *__restrict self) {
	DREF struct Dee_memstate *retstate;
	struct Dee_except_exitinfo *last_info;
	struct Dee_function_generator gen;
	retstate = Dee_function_assembler_alloc_zero_memstate(self);
	if unlikely(!retstate)
		goto err;
	last_info = Dee_function_assembler_except_exit(self, retstate);
	Dee_memstate_decref(retstate);
	if unlikely(!last_info)
		goto err;

	/* Generate code for `return NULL' */
	gen.fg_assembler        = self;
	gen.fg_block            = last_info->exi_block;
	gen.fg_sect             = &gen.fg_block->bb_htext;
	gen.fg_state            = gen.fg_block->bb_mem_start;
	gen.fg_state_hstack_res = NULL;
	ASSERT(gen.fg_state != NULL);
	Dee_memstate_incref(gen.fg_state);
	if unlikely(Dee_function_generator_gretNULL(&gen))
		goto err_gen_fg_state;
	ASSERT(gen.fg_block == last_info->exi_block);
	ASSERT(gen.fg_block->bb_mem_end == NULL);
	gen.fg_block->bb_mem_end = gen.fg_state; /* Inherit reference */

	/* Compile and order all of the other blocks. */
	if (self->fa_except_exitc > 1) {
		size_t i;
		struct Dee_except_exitinfo *next_info;
		size_t next_info_score;
search_for_next_block:
		next_info       = NULL;
		next_info_score = (size_t)-1;
		for (i = 0; i < self->fa_except_exitc; ++i) {
			size_t score;
			struct Dee_except_exitinfo *info;
			info = self->fa_except_exitv[i];
			ASSERT(info);
			if (Dee_except_exitinfo_compiled(info))
				continue;
			score = Dee_except_exitinfo_distance(last_info, info);
			if (next_info_score > score || next_info == NULL) {
				next_info_score = score;
				next_info       = info;
			}
		}
		if (next_info) {
			ASSERT(gen.fg_assembler == self);
			ASSERT(gen.fg_state_hstack_res == NULL);
			gen.fg_block = next_info->exi_block;
			gen.fg_sect  = &gen.fg_block->bb_htext;
			gen.fg_state = gen.fg_block->bb_mem_start;
			Dee_memstate_incref(gen.fg_state);
			ASSERT(last_info->exi_block->bb_mem_start != NULL);
			if unlikely(Dee_function_generator_gexcept_morph(&gen, next_info, last_info))
				goto err_gen_fg_state;
			ASSERT(next_info->exi_block->bb_mem_end == NULL);
			ASSERT(next_info->exi_block->bb_next == NULL);
			next_info->exi_block->bb_mem_end = gen.fg_state; /* Inherit reference */
			next_info->exi_block->bb_next = last_info->exi_block;
			last_info                     = next_info;
			goto search_for_next_block;
		}
	}
	return 0;
err_gen_fg_state:
	Dee_memstate_decref(gen.fg_state);
err:
	return -1;
}

/* Step #4: Generate morph instruction sequences to perform memory state transitions.
 * This also extends the host text of basic blocks that fall through to some
 * other basic block with an extra instructions needed for morphing:
 * - self->fa_prolog                                (to transition )
 * - self->fa_blockv[*]->bb_exits.jds_list[*]->jd_morph
 * - self->fa_blockv[*]->bb_htext                   (extend with transition code so that `bb_mem_end == bb_next->bb_mem_start')
 * - self->fa_except_exitv[*]->exi_block->bb_htext  (generate morph-code to transition to an empty stack, or fall into another exit block)
 * - self->fa_except_exitv[*]->exi_block->bb_next   (set if intend is to fall into another exit block)
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_compilemorph(struct Dee_function_assembler *__restrict self) {
	size_t i;
	/* Initial morph after the prolog to match the start-state of the first block. */
	if unlikely(assemble_morph(self, &self->fa_prolog, self->fa_prolog_end,
	                           self->fa_blockv[0],
	                           self->fa_blockv[0]->bb_deemon_start))
		goto err;
	for (i = 0; i < self->fa_blockc; ++i) {
		size_t j;
		struct Dee_basic_block *block = self->fa_blockv[i];
		ASSERT(block);
		ASSERT(block->bb_mem_start);
		ASSERT(block->bb_mem_end);
		for (j = 0; j < block->bb_exits.jds_size; ++j) {
			struct Dee_jump_descriptor *jd = block->bb_exits.jds_list[j];
			ASSERT(jd);
			ASSERT(jd->jd_from >= block->bb_deemon_start);
			ASSERT(jd->jd_from < block->bb_deemon_end);
			ASSERT(jd->jd_stat != NULL);
			ASSERT(jd->jd_to != NULL);
			if unlikely(assemble_morph(self, &jd->jd_morph, jd->jd_stat,
			                           jd->jd_to, jd->jd_from))
				goto err;
		}
		if (block->bb_next != NULL) {
			if unlikely(assemble_morph(self, &block->bb_htext, block->bb_mem_end,
			                           block->bb_next, block->bb_deemon_end))
				goto err;
		}
	}

	/* Generate cleanup/fallthru code for exit descriptors. */
	if (self->fa_except_exitc > 0)
		return Dee_function_assembler_compileexcept(self);
	return 0;
err:
	return -1;
}

/* Step #5: Generate missing unconditional jumps to jump from one block to the next
 * - Find loops of blocks that "fall through" back on each other in a loop, and
 *   append a jump-to-the-start on all blocks that "fall through" to themselves.
 *   For one of these blocks, also generate a call to `DeeThread_CheckInterrupt()'
 * - For all blocks that have more than 1 fallthru predecessors, take all but
 *   1 of those predecessors and append unconditional jumps to them, then set
 *   the `bb_next' field of those blocks to `NULL'.
 * - Also fills in:
 *   - self->fa_prolog.hs_link
 *   - self->fa_blockv[*]->bb_htext.hs_link
 *   - self->fa_blockv[*]->bb_hcold.hs_link
 *   - self->fa_blockv[*]->bb_exits.jds_list[*]->jd_morph.hs_link
 *   - self->fa_except_exitv[*]->exi_block->bb_htext.hs_link
 *   - self->fa_except_exitv[*]->exi_block->bb_hcold.hs_link
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
