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
#ifndef GUARD_DEX_HOSTASM_ASSEMBLER_C
#define GUARD_DEX_HOSTASM_ASSEMBLER_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/alloc.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/none.h>
#include <deemon/tuple.h>

#include <hybrid/byteswap.h>
#include <hybrid/limitcore.h>
#include <hybrid/minmax.h>
#include <hybrid/overflow.h>
#include <hybrid/unaligned.h>

#include "utils.h"

DECL_BEGIN

#ifndef INT8_MIN
#define INT8_MIN __INT8_MIN__
#endif /* !INT8_MIN */
#ifndef INT8_MAX
#define INT8_MAX __INT8_MAX__
#endif /* !INT8_MAX */


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

	/* Setup the state of the function's first (entry) block. */
	result = Dee_memstate_alloc(self->fa_xlocalc);
	if unlikely(!result)
		goto done;
	result->ms_refcnt = 1;
	result->ms_host_cfa_offset = 0;
	result->ms_localc    = self->fa_xlocalc;
	result->ms_stackc    = 0;
	result->ms_stacka    = 0;
	result->ms_flags     = MEMSTATE_F_NORMAL;
	result->ms_uargc_min = self->fa_code->co_argc_min;
	bzero(result->ms_rinuse, sizeof(result->ms_rinuse));
	Dee_memstate_hregs_clear_usage(result);
	result->ms_stackv = NULL;

	/* Initially, all variables are unbound */
	{
		Dee_lid_t lid;
		for (lid = 0; lid < result->ms_localc; ++lid) {
			result->ms_localv[lid].ml_flags  = MEMLOC_F_NOREF | MEMLOC_F_LOCAL_UNBOUND;
			result->ms_localv[lid].ml_vmorph = MEMLOC_VMORPH_DIRECT;
			result->ms_localv[lid].ml_type   = MEMLOC_TYPE_UNALLOC;
			result->ms_localv[lid].ml_valtyp = NULL;
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
#ifdef HOST_REGISTER_R_ARG4
			HOST_REGISTER_R_ARG4,
#endif /* HOST_REGISTER_R_ARG4 */
		};
#else /* HOSTASM_X86_64 */
#define Dee_memloc_set_x86_arg(self, argi)                                          \
	((self)->ml_flags = MEMLOC_F_NOREF | MEMLOC_F_LOCAL_BOUND,                      \
	 (self)->ml_type  = MEMLOC_TYPE_HSTACKIND,                                      \
	 (self)->ml_value.v_hstack.s_cfa = (uintptr_t)(-(ptrdiff_t)(((argi) + 1) * 4)), \
	 (self)->ml_value.v_hstack.s_off = 0)
#endif /* !HOSTASM_X86_64 */
		size_t argi = 0;
		Dee_lid_t extra_base = self->fa_localc;
		if (cc & HOSTFUNC_CC_F_FUNC) {
			Dee_memloc_set_x86_arg(&state->ms_localv[extra_base + MEMSTATE_XLOCAL_A_FUNC], argi);
			++argi;
		}
		if (cc & HOSTFUNC_CC_F_THIS) {
			Dee_memloc_set_x86_arg(&state->ms_localv[extra_base + MEMSTATE_XLOCAL_A_THIS], argi);
			++argi;
		}
		if (cc & HOSTFUNC_CC_F_TUPLE) {
			Dee_memloc_set_x86_arg(&state->ms_localv[extra_base + MEMSTATE_XLOCAL_A_ARGS], argi);
			++argi;
		} else {
			Dee_memloc_set_x86_arg(&state->ms_localv[extra_base + MEMSTATE_XLOCAL_A_ARGC], argi);
			++argi;
			Dee_memloc_set_x86_arg(&state->ms_localv[extra_base + MEMSTATE_XLOCAL_A_ARGV], argi);
			++argi;
		}
		if (cc & HOSTFUNC_CC_F_KW) {
#if defined(HOSTASM_X86_64) && !defined(HOST_REGISTER_R_ARG4)
			if (argi == 5) {
				struct Dee_memloc *a_kw = &state->ms_localv[extra_base + MEMSTATE_XLOCAL_A_KW];
				/*  */
				a_kw->ml_flags = MEMLOC_F_NOREF | MEMLOC_F_LOCAL_BOUND;
				a_kw->ml_type  = MEMLOC_TYPE_HSTACKIND;
#ifdef HOSTASM_X86_64_MSABI
				a_kw->ml_value.v_hstack.s_cfa = (uintptr_t)(-(5 * HOST_SIZEOF_POINTER));
#else /* HOSTASM_X86_64_MSABI */
				a_kw->ml_value.v_hstack.s_cfa = (uintptr_t)(-(1 * HOST_SIZEOF_POINTER));
#endif /* !HOSTASM_X86_64_MSABI */
				a_kw->ml_value.v_hstack.s_off = 0;
			} else
#endif /* HOSTASM_X86_64 && !HOST_REGISTER_R_ARG4 */
			{
				Dee_memloc_set_x86_arg(&state->ms_localv[extra_base + MEMSTATE_XLOCAL_A_KW], argi);
			}
		}
#undef Dee_memloc_set_x86_arg
	}
#else /* ... */
#error "Initial register state not implemented for this architecture"
#endif /* !... */

	/* Some arguments have known object types. */
	if (cc & HOSTFUNC_CC_F_FUNC)
		state->ms_localv[self->fa_xlocalc + MEMSTATE_XLOCAL_A_FUNC].ml_valtyp = &DeeFunction_Type;
	if (cc & HOSTFUNC_CC_F_TUPLE)
		state->ms_localv[self->fa_xlocalc + MEMSTATE_XLOCAL_A_ARGS].ml_valtyp = &DeeTuple_Type;

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

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_makeprolog_cleanup(struct Dee_function_generator *__restrict self) {
	/* Delete all locals initialized by the prolog
	 * that aren't used within the function. */
	Dee_lid_t lid;
	struct Dee_memstate *state = self->fg_state;
	struct Dee_basic_block *block0 = self->fg_block;
	for (lid = 0; lid < state->ms_localc; ++lid) {
		if (bitset_test(block0->bb_locuse, lid))
			continue;
		if (state->ms_localv[lid].ml_type == MEMLOC_TYPE_UNALLOC)
			continue;
		if unlikely(Dee_function_generator_vdel_local(self, lid))
			goto err;
		state = self->fg_state; /* Re-load in case unshare happened. */
	}
	return 0;
err:
	return -1;
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
	if likely(result == 0)
		result = Dee_function_generator_makeprolog_cleanup(&gen);
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

#ifdef DEE_HOST_RELOCVALUE_SECT
		case DEE_HOST_RELOCVALUE_SECT:
			host_section_set_insert(self, rel->hr_value.rv_sect);
			break;
#endif /* DEE_HOST_RELOCVALUE_SECT */

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
		block->bb_next   = self->fa_deleted; /* Must keep the block around (in case symbols reference its sections) */
		self->fa_deleted = block;
		_Dee_except_exitinfo_destroy_noblock(info);
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
Dee_function_generator_gexcept_morph_mov(struct Dee_function_generator *__restrict self,
                                         struct Dee_memloc *oldloc,
                                         struct Dee_memloc const *newloc,
                                         uint16_t old_refcnt, uint16_t new_refcnt) {
	int temp;
	ASSERT(old_refcnt != 0);
	ASSERT(new_refcnt != 0);
	if ((old_refcnt & DEE_EXCEPT_EXITINFO_NULLFLAG) != 0 &&
	    (new_refcnt & DEE_EXCEPT_EXITINFO_NULLFLAG) == 0) {
		/* New state expects the value to never be NULL
		 * -> Handle this by loading the proper refcnt and `Dee_None' into `oldloc' if it's NULL
		 */
		struct Dee_host_symbol *not_null_sym;
		not_null_sym = Dee_function_generator_newsym(self);
		if unlikely(!not_null_sym)
			goto err;
		if unlikely(_Dee_function_generator_gjnz(self, oldloc, not_null_sym))
			goto err;
		old_refcnt &= ~DEE_EXCEPT_EXITINFO_NULLFLAG; /* No longer nullable! */
		_Dee_function_generator_gincref_const(self, Dee_None, old_refcnt);
		if unlikely(Dee_function_generator_gmov_const2loc(self, Dee_None, oldloc))
			goto err;
		Dee_host_symbol_setsect(not_null_sym, self->fg_sect);
	}
	new_refcnt &= ~DEE_EXCEPT_EXITINFO_NULLFLAG; /* Don't care about nullable here anymore! */

	if (old_refcnt & DEE_EXCEPT_EXITINFO_NULLFLAG) {
		ptrdiff_t refcnt_delta = (ptrdiff_t)(new_refcnt) -
		                         (ptrdiff_t)(old_refcnt & ~DEE_EXCEPT_EXITINFO_NULLFLAG);
		if (refcnt_delta > 0) {
			temp = Dee_function_generator_gxincref(self, oldloc, (Dee_refcnt_t)refcnt_delta);
		} else if (refcnt_delta < 0) {
			temp = new_refcnt ? Dee_function_generator_gxdecref_nokill(self, oldloc, (Dee_refcnt_t)(-refcnt_delta))
			                  : Dee_function_generator_gxdecref(self, oldloc, (Dee_refcnt_t)(-refcnt_delta));
		} else {
			temp = 0;
		}
	} else {
		ptrdiff_t refcnt_delta = (ptrdiff_t)new_refcnt - (ptrdiff_t)old_refcnt;
		if (refcnt_delta > 0) {
			temp = Dee_function_generator_gincref(self, oldloc, (Dee_refcnt_t)refcnt_delta);
		} else if (refcnt_delta < 0) {
			temp = new_refcnt ? Dee_function_generator_gdecref_nokill(self, oldloc, (Dee_refcnt_t)(-refcnt_delta))
			                  : Dee_function_generator_gdecref(self, oldloc, (Dee_refcnt_t)(-refcnt_delta));
		} else {
			temp = 0;
		}
	}
	if unlikely(temp)
		goto err;
	return Dee_function_generator_gmov_loc2loc(self, oldloc, newloc);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gexcept_morph_adjref(struct Dee_function_generator *__restrict self,
                                            struct Dee_memloc *__restrict oldloc,
                                            uint16_t old_refcnt, uint16_t new_refcnt) {
	return Dee_function_generator_gexcept_morph_mov(self, oldloc, oldloc, old_refcnt, new_refcnt);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gexcept_morph_decref(struct Dee_function_generator *__restrict self,
                                            struct Dee_memloc *__restrict loc,
                                            uint16_t old_refcnt) {
	ASSERT(old_refcnt != 0);
	if (old_refcnt & DEE_EXCEPT_EXITINFO_NULLFLAG) {
		return Dee_function_generator_gxdecref(self, loc, old_refcnt & ~DEE_EXCEPT_EXITINFO_NULLFLAG);
	} else {
		return Dee_function_generator_gdecref(self, loc, old_refcnt);
	}
}

/* Pop the V-stack entry describing `loc' */
PRIVATE NONNULL((1, 2)) void DCALL
Dee_memstate_vundef_loc(struct Dee_memstate *__restrict self,
                        Dee_vstackaddr_t i) {
	struct Dee_memloc *vloc;
	ASSERT(i < self->ms_stackc);
	vloc = &self->ms_stackv[i];
	if (MEMLOC_TYPE_HASREG(vloc->ml_type))
		Dee_memstate_decrinuse(self, vloc->ml_value.v_hreg.r_regno);
	vloc->ml_type = MEMLOC_TYPE_UNDEFINED;
	vloc->ml_valtyp = NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gexcept_morph(struct Dee_function_generator *__restrict self,
                                     struct Dee_except_exitinfo const *__restrict oldinfo,
                                     struct Dee_except_exitinfo const *__restrict newinfo) {
	Dee_vstackaddr_t *curinfo_vaddr; /* Map from `i < Dee_except_exitinfo_locc()' -> `state->ms_stackv' */
	struct Dee_memstate *state;
	size_t cur_loci, cur_locc;
	size_t new_loci, new_locc, max_locc;
	size_t cursize;
	struct Dee_except_exitinfo *curinfo;
	cursize = Dee_except_exitinfo_sizeof(oldinfo->exi_cfa_offset);
	curinfo = (struct Dee_except_exitinfo *)Dee_Malloca(cursize);
	if unlikely(!curinfo)
		goto err;
	curinfo = (struct Dee_except_exitinfo *)memcpy(curinfo, oldinfo, cursize);

	/* Allocate a mem-state and then push all registers containing references
	 * onto the v-stack, so they get saved properly in case they need to be
	 * flushed. */
	state = (struct Dee_memstate *)Dee_Malloca(Dee_memstate_sizeof(1));
	if unlikely(!state)
		goto err_infostate;
	state->ms_refcnt          = 1;
	state->ms_host_cfa_offset = curinfo->exi_cfa_offset;
	state->ms_localc          = 1;
	state->ms_stackc          = 0;
	state->ms_stacka          = 0;
	state->ms_stackv          = NULL;
	bzero(state->ms_rinuse, sizeof(state->ms_rinuse));
	bzero(state->ms_rusage, sizeof(state->ms_rusage));
	self->fg_state = state;

	cur_locc = Dee_except_exitinfo_locc(curinfo);
	new_locc = Dee_except_exitinfo_locc(newinfo);
	while (cur_locc) {
		size_t cur_locX = Dee_except_exitinfo_loci_revstack(curinfo, cur_locc - 1);
		if (Dee_except_exitinfo_locv(curinfo, cur_locX))
			break;
		--cur_locc;
	}
	while (new_locc) {
		size_t new_locX = Dee_except_exitinfo_loci_revstack(newinfo, new_locc - 1);
		if (Dee_except_exitinfo_locv(newinfo, new_locX))
			break;
		--new_locc;
	}

	/* Push locations used by the old state onto the V-stack (so they aren't clobbered). */
	max_locc = MAX(cur_locc, new_locc);
	curinfo_vaddr = (Dee_vstackaddr_t *)Dee_Mallocac(max_locc, sizeof(Dee_vstackaddr_t));
	if unlikely(!curinfo_vaddr)
		goto err_infostate_state;
	for (cur_loci = 0; cur_loci < cur_locc; ++cur_loci) {
		struct Dee_memloc cur_loc;
		uint16_t old_refcnt = Dee_except_exitinfo_locv(curinfo, cur_loci);
		ASSERT(old_refcnt != DEE_EXCEPT_EXITINFO_NULLFLAG);
		if (!old_refcnt) {
			curinfo_vaddr[cur_loci] = (Dee_vstackaddr_t)-1;
			continue;
		}
		curinfo_vaddr[cur_loci] = state->ms_stackc;
		Dee_except_exitinfo_asloc(cur_loci, &cur_loc);
		cur_loc.ml_vmorph = MEMLOC_VMORPH_DIRECT;
		cur_loc.ml_valtyp = NULL;
		if unlikely(Dee_memstate_vpush(state, &cur_loc))
			goto err_infostate_state_curinfo_vaddr;
	}
	for (; cur_loci < max_locc; ++cur_loci)
		curinfo_vaddr[cur_loci] = (Dee_vstackaddr_t)-1;

	/* Try to shift values between locations. */
	for (cur_loci = 0; cur_loci < cur_locc; ++cur_loci) {
		size_t cur_locX;
		struct Dee_memloc cur_loc;
		uint16_t cur_refcnt_i, new_refcnt_i;
		uint16_t old_refcnt;
		uint16_t new_refcnt;
		cur_locX   = Dee_except_exitinfo_loci_revstack(curinfo, cur_loci);
		old_refcnt = Dee_except_exitinfo_locv(curinfo, cur_locX);
		new_refcnt = 0;
		if (cur_locX < new_locc)
			new_refcnt = Dee_except_exitinfo_locv(newinfo, cur_locX);
		ASSERT(old_refcnt != DEE_EXCEPT_EXITINFO_NULLFLAG);
		ASSERT(new_refcnt != DEE_EXCEPT_EXITINFO_NULLFLAG);
		if (old_refcnt == 0)
			continue; /* Nothing here */
		if (old_refcnt == (new_refcnt) ||
		    old_refcnt == (new_refcnt & ~DEE_EXCEPT_EXITINFO_NULLFLAG))
			continue; /* Location already matches target perfectly. */

		/* Check if we can off-load to a different location. */
		Dee_except_exitinfo_asloc(cur_locX, &cur_loc);
		for (new_loci = 0; new_loci < new_locc; ++new_loci) {
			cur_refcnt_i = 0;
			if (new_loci < cur_locc)
				cur_refcnt_i = Dee_except_exitinfo_locv(curinfo, new_loci);
			new_refcnt_i = Dee_except_exitinfo_locv(newinfo, new_loci);
			if (new_refcnt_i == old_refcnt && !cur_refcnt_i) {
				struct Dee_memloc new_loc;
offload_cur_locX_to_new_loci:
				Dee_except_exitinfo_asloc(cur_locX, &cur_loc);
				Dee_except_exitinfo_asloc(new_loci, &new_loc);
				ASSERT(Dee_except_exitinfo_locv(curinfo, new_loci) == 0);
				if (curinfo_vaddr[cur_loci] != (Dee_vstackaddr_t)-1) {
					Dee_memstate_vundef_loc(state, curinfo_vaddr[cur_loci]);
					curinfo_vaddr[cur_loci] = (Dee_vstackaddr_t)-1;
				}
				if unlikely(Dee_function_generator_gexcept_morph_mov(self, &cur_loc, &new_loc,
				                                                     old_refcnt, new_refcnt_i))
					goto err_infostate_state_curinfo_vaddr;
				ASSERT(Dee_except_exitinfo_locv(curinfo, cur_locX) != 0);
				ASSERT(Dee_except_exitinfo_locv(curinfo, new_loci) == 0);
				Dee_except_exitinfo_locv(curinfo, cur_locX) = 0;
				Dee_except_exitinfo_locv(curinfo, new_loci) = new_refcnt_i;
				ASSERT(curinfo_vaddr[new_loci] == (Dee_vstackaddr_t)-1);
				curinfo_vaddr[new_loci] = state->ms_stackc;
				new_loc.ml_vmorph = MEMLOC_VMORPH_DIRECT;
				new_loc.ml_valtyp = NULL;
				if unlikely(Dee_memstate_vpush(state, &new_loc))
					goto err_infostate_state_curinfo_vaddr;
				goto next_cur_locX;
			}
		}
		for (new_loci = 0; new_loci < new_locc; ++new_loci) {
			cur_refcnt_i = 0;
			if (new_loci < cur_locc)
				cur_refcnt_i = Dee_except_exitinfo_locv(curinfo, new_loci);
			new_refcnt_i = Dee_except_exitinfo_locv(newinfo, new_loci);
			if (new_refcnt_i == (old_refcnt & ~DEE_EXCEPT_EXITINFO_NULLFLAG) && !cur_refcnt_i)
				goto offload_cur_locX_to_new_loci;
		}
		for (new_loci = 0; new_loci < new_locc; ++new_loci) {
			cur_refcnt_i = 0;
			if (new_loci < cur_locc)
				cur_refcnt_i = Dee_except_exitinfo_locv(curinfo, new_loci);
			new_refcnt_i = Dee_except_exitinfo_locv(newinfo, new_loci);
			if (new_refcnt_i >= old_refcnt && !cur_refcnt_i)
				goto offload_cur_locX_to_new_loci;
		}
		for (new_loci = 0; new_loci < new_locc; ++new_loci) {
			cur_refcnt_i = 0;
			if (new_loci < cur_locc)
				cur_refcnt_i = Dee_except_exitinfo_locv(curinfo, new_loci);
			new_refcnt_i = Dee_except_exitinfo_locv(newinfo, new_loci);
			if (new_refcnt_i >= (old_refcnt & ~DEE_EXCEPT_EXITINFO_NULLFLAG) && !cur_refcnt_i)
				goto offload_cur_locX_to_new_loci;
		}

		/* Nowhere to off-load to -> must do the (possibly partial) decref ourselves. */
		if (new_refcnt != 0) {
			if unlikely(Dee_function_generator_gexcept_morph_adjref(self, &cur_loc, old_refcnt, new_refcnt))
				goto err_infostate_state_curinfo_vaddr;
		} else {
			/* Value doesn't matter anymore. */
			if (curinfo_vaddr[cur_loci] != (Dee_vstackaddr_t)-1) {
				Dee_memstate_vundef_loc(state, curinfo_vaddr[cur_loci]);
				curinfo_vaddr[cur_loci] = (Dee_vstackaddr_t)-1;
			}
			if unlikely(Dee_function_generator_gexcept_morph_decref(self, &cur_loc, old_refcnt))
				goto err_infostate_state_curinfo_vaddr;
		}
		Dee_except_exitinfo_locv(curinfo, cur_locX) = new_refcnt;
next_cur_locX:;
	}

	/* With value shifting down, force same-slot locations to have the proper reference counts. */
	for (new_loci = 0; new_loci < new_locc; ++new_loci) {
		struct Dee_memloc new_loc;
		size_t new_locX;
		uint16_t old_refcnt = 0;
		uint16_t new_refcnt;
		new_locX   = Dee_except_exitinfo_loci_revstack(newinfo, new_loci);
		new_refcnt = Dee_except_exitinfo_locv(newinfo, new_locX);
		old_refcnt = 0;
		if (new_locX < cur_locc)
			old_refcnt = Dee_except_exitinfo_locv(curinfo, new_locX);
		ASSERT(old_refcnt != DEE_EXCEPT_EXITINFO_NULLFLAG);
		ASSERT(new_refcnt != DEE_EXCEPT_EXITINFO_NULLFLAG);
		if (new_refcnt == 0) {
			ASSERT(old_refcnt == 0);
			continue; /* Nothing here */
		}
		if (old_refcnt == (new_refcnt) ||
		    old_refcnt == (new_refcnt & ~DEE_EXCEPT_EXITINFO_NULLFLAG))
			continue; /* Location already matches target perfectly. */
		Dee_except_exitinfo_asloc(new_locX, &new_loc);

		/* Handle special case: "push" only gets used if at the CFA boundary.
		 * So if the target is a stack location that is out-of-bounds (and not
		 * at the boundary), then allocate the remaining stack. */
		if (new_loc.ml_type == MEMLOC_TYPE_HSTACKIND) {
			uintptr_t loc_cfa = new_loc.ml_value.v_hstack.s_cfa;
			uintptr_t cur_cfa = state->ms_host_cfa_offset;
#ifdef HOSTASM_STACK_GROWS_DOWN
			if (loc_cfa > cur_cfa)
#else /* HOSTASM_STACK_GROWS_DOWN */
			if (loc_cfa >= cur_cfa)
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			{
				uintptr_t new_cfa = newinfo->exi_cfa_offset;
				ptrdiff_t cfa_delta = (ptrdiff_t)new_cfa - (ptrdiff_t)cur_cfa;
				if unlikely(Dee_function_generator_ghstack_adjust(self, cfa_delta))
					goto err_infostate_state_curinfo_vaddr;
			}
		}
		if (old_refcnt == 0) {
			/* Need to get an object out of the void. */
			struct Dee_memloc none;
			none.ml_type = MEMLOC_TYPE_CONST;
			none.ml_value.v_const = Dee_None;
			if unlikely(Dee_function_generator_gexcept_morph_mov(self, &none, &new_loc,
			                                                     1, new_refcnt + 1))
				goto err_infostate_state_curinfo_vaddr;
			ASSERT(curinfo_vaddr[new_loci] == (Dee_vstackaddr_t)-1);
			curinfo_vaddr[new_loci] = state->ms_stackc;
			new_loc.ml_vmorph = MEMLOC_VMORPH_DIRECT;
			new_loc.ml_valtyp = NULL;
			if unlikely(Dee_memstate_vpush(state, &new_loc))
				goto err_infostate_state_curinfo_vaddr;
		} else {
			if unlikely(Dee_function_generator_gexcept_morph_adjref(self, &new_loc, old_refcnt, new_refcnt))
				goto err_infostate_state_curinfo_vaddr;
		}
		/*if (new_locX < cur_locc)
			Dee_except_exitinfo_locv(curinfo, new_locX) = new_refcnt;*/
	}

	/* Load register-references needed by the new state into registers (if they were flushed). */
	{
		Dee_host_register_t new_regi, new_regc;
		new_regc = HOST_REGISTER_COUNT;
		if (new_locc < HOST_REGISTER_COUNT)
			new_regc = (Dee_host_register_t)new_locc;
		for (new_regi = 0; new_regi < new_regc; ++new_regi) {
			struct Dee_memloc *loc;
			uint16_t new_refcnt = Dee_except_exitinfo_locv(newinfo, new_regi);
			ASSERT(new_refcnt != DEE_EXCEPT_EXITINFO_NULLFLAG);
			if (new_refcnt == 0)
				continue;
			ASSERT(curinfo_vaddr[new_regi] != (Dee_vstackaddr_t)-1);
			ASSERT(curinfo_vaddr[new_regi] < state->ms_stackc);
			loc = &state->ms_stackv[curinfo_vaddr[new_regi]];
			ASSERT(loc->ml_type == MEMLOC_TYPE_HREG ||
			       loc->ml_type == MEMLOC_TYPE_HSTACKIND);
			ASSERT(loc->ml_type != MEMLOC_TYPE_HREG ||
			       (loc->ml_value.v_hreg.r_regno == new_regi ||
			        loc->ml_value.v_hreg.r_off == 0));
			if (loc->ml_type != MEMLOC_TYPE_HREG) {
				if unlikely(Dee_function_generator_gmov_loc2regx(self, loc, new_regi, 0))
					goto err_infostate_state_curinfo_vaddr;
				loc->ml_type = MEMLOC_TYPE_HREG;
				loc->ml_value.v_hreg.r_regno = new_regi;
				loc->ml_value.v_hreg.r_off   = 0;
			}
		}
	}

	/* Fix any remaining delta in the CFA offset. */
	if (state->ms_host_cfa_offset != newinfo->exi_cfa_offset) {
		ptrdiff_t cfa_delta = (ptrdiff_t)newinfo->exi_cfa_offset -
		                      (ptrdiff_t)state->ms_host_cfa_offset;
		if unlikely(Dee_function_generator_ghstack_adjust(self, cfa_delta))
			goto err_infostate_state_curinfo_vaddr;
	}
	ASSERT(state->ms_host_cfa_offset == newinfo->exi_cfa_offset);

	/* Cleanup... */
	Dee_Freea(curinfo_vaddr);
	ASSERT(self->fg_state == state);
	ASSERT(state->ms_refcnt == 1);
	Dee_Free(state->ms_stackv);
	Dee_Freea(state);
	Dee_Freea(curinfo);
	return 0;
err_infostate_state_curinfo_vaddr:
	Dee_Freea(curinfo_vaddr);
err_infostate_state:
	ASSERT(self->fg_state == state);
	ASSERT(state->ms_refcnt == 1);
	Dee_Free(state->ms_stackv);
	Dee_Freea(state);
err_infostate:
	Dee_Freea(curinfo);
err:
	return -1;
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
			ASSERT(last_info->exi_block->bb_mem_start != NULL);
			if unlikely(Dee_function_generator_gexcept_morph(&gen, next_info, last_info))
				goto err;
			ASSERT(next_info->exi_block->bb_mem_end == NULL);
			ASSERT(next_info->exi_block->bb_next == NULL);
			next_info->exi_block->bb_next = last_info->exi_block;
			last_info                     = next_info;
			goto search_for_next_block;
		}
	}
	/* Remember the most "convoluted" exception exit block, so ordering can start with *it* */
	self->fa_except_first = last_info;
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
 * - self->fa_except_first
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_compilemorph(struct Dee_function_assembler *__restrict self) {
	size_t block_i;
	/* Initial morph after the prolog to match the start-state of the first block. */
	if unlikely(assemble_morph(self, &self->fa_prolog, self->fa_prolog_end,
	                           self->fa_blockv[0],
	                           self->fa_blockv[0]->bb_deemon_start))
		goto err;
	for (block_i = 0; block_i < self->fa_blockc; ++block_i) {
		size_t exit_i;
		struct Dee_basic_block *block = self->fa_blockv[block_i];
		ASSERT(block);
		ASSERT(block->bb_mem_start);
		ASSERT(block->bb_mem_end);
		for (exit_i = 0; exit_i < block->bb_exits.jds_size; ++exit_i) {
			struct Dee_jump_descriptor *jd = block->bb_exits.jds_list[exit_i];
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

PRIVATE NONNULL((1, 2)) void DCALL
collect_morph_sections(struct Dee_host_section_tailq *__restrict text,
                       struct Dee_basic_block *__restrict morph_block) {
	size_t exit_i;
	for (exit_i = 0; exit_i < morph_block->bb_exits.jds_size; ++exit_i) {
		struct Dee_jump_descriptor *jmp = morph_block->bb_exits.jds_list[exit_i];
		ASSERT(jmp);
		TAILQ_INSERT_TAIL(text, &jmp->jd_morph, hs_link);
		jmp->jd_morph.hs_fallthru = &jmp->jd_to->bb_htext;
	}
}

/* Step #5: Generate missing unconditional jumps to jump from one block to the next
 * - Find loops of blocks that "fall through" back on each other in a loop, and
 *   append a jump-to-the-start on all blocks that "fall through" to themselves.
 *   For one of these blocks, also generate a call to `DeeThread_CheckInterrupt()'
 * - For all blocks that have more than 1 fallthru predecessors, take all but
 *   1 of those predecessors and append unconditional jumps to them, then set
 *   the `bb_next' field of those blocks to `NULL'.
 * - Also fills in:
 *   - self->fa_sections
 *   - self->fa_prolog.hs_link+hs_symbols
 *   - self->fa_blockv[*]->bb_htext.hs_link+hs_symbols
 *   - self->fa_blockv[*]->bb_hcold.hs_link+hs_symbols
 *   - self->fa_blockv[*]->bb_exits.jds_list[*]->jd_morph.hs_link+hs_symbols
 *   - self->fa_except_exitv[*]->exi_block->bb_htext.hs_link+hs_symbols
 *   - self->fa_except_exitv[*]->exi_block->bb_hcold.hs_link+hs_symbols
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_assembler_ordersections(struct Dee_function_assembler *__restrict self) {
	struct Dee_host_section *sect;
	size_t block_i, morph_flush_i;
#ifdef __INTELLISENSE__
	struct Dee_host_section_tailq text;
#else /* __INTELLISENSE__ */
#define text self->fa_sections
#endif /* !__INTELLISENSE__ */
	struct Dee_host_section_tailq cold;
	TAILQ_INIT(&text);
	TAILQ_INIT(&cold);
	ASSERT(!Dee_host_section_islinked(&self->fa_prolog));
	TAILQ_INSERT_HEAD(&text, &self->fa_prolog, hs_link);
	ASSERT(Dee_host_section_islinked(&self->fa_prolog));
	ASSERT(self->fa_blockc >= 1);
#define LOCAL_morph_flush(until_block_i)                                     \
	do {                                                                     \
		size_t morph_i;                                                      \
		for (morph_i = morph_flush_i; morph_i <= (until_block_i); ++morph_i) \
			collect_morph_sections(&text, self->fa_blockv[morph_i]);         \
		morph_flush_i = (until_block_i) + 1;                                 \
	}	__WHILE0

	/* For now, instead of trying to re-order blocks, trust that the original byte-code
	 * already got it good enough, and simply output based on the position of blocks in
	 * relation to deemon bytecode address ranges. */
	morph_flush_i = 0;
	for (block_i = 0; block_i < self->fa_blockc; ++block_i) {
		struct Dee_basic_block *jmp_next;
		struct Dee_basic_block *block = self->fa_blockv[block_i];
		ASSERT(block);
		jmp_next = block->bb_next;
		block->bb_htext.hs_fallthru = jmp_next ? &jmp_next->bb_htext : NULL;
		block->bb_hcold.hs_fallthru = NULL;
		TAILQ_INSERT_TAIL(&text, &block->bb_htext, hs_link);
		TAILQ_INSERT_TAIL(&cold, &block->bb_hcold, hs_link);
		if (jmp_next == NULL) {
			/* Block doesn't return normally -> this is a good spot to insert morph sections. */
			LOCAL_morph_flush(block_i);
		}
	}
	ASSERT(block_i == self->fa_blockc);
	LOCAL_morph_flush(self->fa_blockc - 1);
#undef LOCAL_morph_flush

	/* Go through exception exits and put them into the section order.
	 * Note that `Dee_function_assembler_compilemorph()' already ordered
	 * them for us, so all we need to do is start walking at `fa_except_first' */
	ASSERT((self->fa_except_first != NULL) ==
	       (self->fa_except_exitc > 0));
	if (self->fa_except_first != NULL) {
		struct Dee_basic_block *block;
		block = self->fa_except_first->exi_block;
		do {
			struct Dee_basic_block *jmp_next = block->bb_next;
			block->bb_htext.hs_fallthru = jmp_next ? &jmp_next->bb_htext : NULL;
			block->bb_hcold.hs_fallthru = NULL;
			TAILQ_INSERT_TAIL(&text, &block->bb_htext, hs_link);
			TAILQ_INSERT_TAIL(&cold, &block->bb_hcold, hs_link);
		} while ((block = block->bb_next) != NULL);
		block = self->fa_except_first->exi_block;
		do {
			size_t exit_i;
			for (exit_i = 0; exit_i < block->bb_exits.jds_size; ++exit_i) {
				struct Dee_jump_descriptor *jmp = block->bb_exits.jds_list[exit_i];
				ASSERT(jmp);
				jmp->jd_morph.hs_fallthru = &jmp->jd_to->bb_htext;
				TAILQ_INSERT_TAIL(&text, &jmp->jd_morph, hs_link);
			}
		} while ((block = block->bb_next) != NULL);
	}

	/* Append all of the cold text to the end of normal text. */
	TAILQ_CONCAT(&text, &cold, hs_link);

	/* Go through the big ol' section list and search for sections that have
	 * a non-NULL `hs_fallthru' that differs from `TAILQ_NEXT(sect, hs_link)'
	 *
	 * When encountered, try to move the section before their intended fallthru
	 * (so-long as doing so doesn't mean that some other section's fallthru
	 * becomes blocked) */
	sect = TAILQ_FIRST(&text);
	while (sect) {
		struct Dee_host_section *sort_next = TAILQ_NEXT(sect, hs_link);
		struct Dee_host_section *want_next = sect->hs_fallthru;
		ASSERT(sort_next != sect);
		if (want_next && want_next != sort_next && want_next != sect) {
			struct Dee_host_section *want_prev;
			/* Special case: can *always* move an empty section */
			if (Dee_host_section_size(sect) == 0)
				goto do_move_section;

			/* See if we can move this section to where it wants to go. */
			want_prev = TAILQ_PREV(want_next, Dee_host_section_tailq, hs_link);
			while (want_prev && want_prev->hs_fallthru == want_next &&
			       Dee_host_section_size(want_prev) == 0) {
				/* Special case: the other section is empty, so it
				 * doesn't matter if we jump to it, or after it.
				 *
				 * As such, try to jump *to* it so we get another
				 * chance of shifting `sect'. */
				want_next = want_prev;
				want_prev = TAILQ_PREV(want_next, Dee_host_section_tailq, hs_link);
			}
			if (want_prev == NULL || want_prev->hs_fallthru != want_next) {
				/* Moving the section doesn't have any downsides (other
				 * than potential cache locality) -> so move it! */
do_move_section:
				sect->hs_fallthru = want_next;
				TAILQ_REMOVE(&text, sect, hs_link);
				TAILQ_INSERT_BEFORE(want_next, sect, hs_link);
				ASSERT(TAILQ_NEXT(sect, hs_link) == sect->hs_fallthru);
				ASSERT(TAILQ_NEXT(sect, hs_link) == want_next);
				}
		}
		sect = sort_next;
	}

	/* Go through the list of sections and append jump instructions wherever
	 * the intended fallthru target differs from the actual successor section. */
	TAILQ_FOREACH (sect, &text, hs_link) {
		struct Dee_host_section *want_next = sect->hs_fallthru;
		struct Dee_host_section *sort_next = TAILQ_NEXT(sect, hs_link);
		unsigned int n;
		if (want_next == NULL)
			goto no_jmp_needed; /* No specific successor needed */
		if (want_next == sort_next)
			goto no_jmp_needed; /* Expected successor already matched */

		/* If the intended target is a zero-sized section, then the
		 * actual jump goes to wherever that section falls to. */
		want_next = sect->hs_fallthru;
		for (n = 128; n > 0; --n) { /* Limit is needed for infinite loops */
			struct Dee_host_section *next;
			if (Dee_host_section_size(want_next) != 0)
				break;
			next = want_next->hs_fallthru;
			if (next == NULL) {
				DeeError_Throwf(&DeeError_IllegalInstruction,
				                "Cannot fallthru into unmapped code");
				goto err;
			}
			if (next == sort_next)
				goto no_jmp_needed;
			want_next = next;
			sect->hs_fallthru = want_next;
		}

		/* Must generate a jump */
		{
			Dee_function_assembler_DEFINE_Dee_host_symbol_section(self, err, dst, want_next, 0);
#ifdef Dee_MallocUsableSize
			sect->hs_alend = sect->hs_start + Dee_MallocUsableSize(sect->hs_start);
			ASSERT(sect->hs_alend >= sect->hs_end);
#else /* Dee_MallocUsableSize */
			sect->hs_alend = sect->hs_end;
#endif /* !Dee_MallocUsableSize */
			if unlikely(_Dee_host_section_gjmp(sect, dst))
				goto err;
		}
		/*sect->hs_fallthru = NULL;*/
no_jmp_needed:;
	}

	/* Go through the big ol' list of sections to figure out the total text size.
	 * Note that we can't remove empty sections because they might still be
	 * referenced by Dee_host_symbol-s. */
	self->fa_sectsize = 0;
	TAILQ_FOREACH (sect, &self->fa_sections, hs_link) {
		self->fa_sectsize += Dee_host_section_size(sect);
#ifdef HOSTASM_HAVE_SHRINKJUMPS
		sect->hs_symbols = NULL; /* Needed for symbol ordering */
#endif /* HOSTASM_HAVE_SHRINKJUMPS */
	}

#ifdef HOSTASM_HAVE_SHRINKJUMPS
	/* Sort host text symbols into their proper sections (needed
	 * so that shrinkjumps() can adjust symbols when deleting text) */
	{
		struct Dee_host_symbol *misc_symbols = NULL;
		struct Dee_host_symbol *sym = self->fa_symbols;
		while (sym) {
			struct Dee_host_symbol *next = sym->_hs_next;
			struct Dee_host_symbol **p_list;
			/* Must also resolve `DEE_HOST_SYMBOL_JUMP' -> `DEE_HOST_SYMBOL_SECT' */
			if (sym->hs_type == DEE_HOST_SYMBOL_JUMP) {
				struct Dee_jump_descriptor *jmp = sym->hs_value.sv_jump;
				struct Dee_host_section *target_sect;
				if (Dee_host_section_islinked(&jmp->jd_morph)) {
					target_sect = &jmp->jd_morph;
				} else {
					struct Dee_basic_block *block;
					block = jmp->jd_to;
					while (Dee_host_section_islinked(&block->bb_htext)) {
						ASSERTF(block->bb_next, "symbol points to not-linked block with no successor");
						block = block->bb_next;
					}
					target_sect = &block->bb_htext;
				}
				sym->hs_type = DEE_HOST_SYMBOL_SECT;
				sym->hs_value.sv_sect.ss_sect = target_sect;
				sym->hs_value.sv_sect.ss_off  = 0;
			}
			ASSERT(sym->hs_type == DEE_HOST_SYMBOL_SECT ||
			       sym->hs_type == DEE_HOST_SYMBOL_ABS);
			p_list = &misc_symbols;
			if (sym->hs_type == DEE_HOST_SYMBOL_SECT) {
				struct Dee_host_section *symsect;
				symsect = sym->hs_value.sv_sect.ss_sect;
				ASSERT(symsect);
				/* The section may not be linked if it was deleted by `Dee_function_assembler_trimdead()'.
				 * When that is the case, simply keep the symbol as part of the misc-symbols list. */
				if (Dee_host_section_islinked(symsect))
					p_list = &symsect->hs_symbols;
			}
			ASSERT(!*p_list || (*p_list)->hs_type != DEE_HOST_SYMBOL_UNDEF);
			sym->_hs_next = *p_list;
			*p_list = sym;
			sym = next;
		}
		/* Remember non-section symbols here. */
		self->fa_symbols = misc_symbols;
	}
#endif /* HOSTASM_HAVE_SHRINKJUMPS */

	return 0;
err:
	TAILQ_FOREACH (sect, &text, hs_link)
		sect->hs_symbols = NULL;
	return -1;
#undef text
}


#ifdef HOSTASM_HAVE_SHRINKJUMPS
/* Delete the specified address range, and adjust  */
PRIVATE NONNULL((1)) void DCALL
Dee_host_section_deltext(struct Dee_host_section *__restrict self,
                         uintptr_t sectrel_addr, size_t num_bytes) {
	struct Dee_host_section *sect;
	struct Dee_host_symbol *sym;
	size_t i;
	ASSERT((sectrel_addr) < Dee_host_section_size(self));
	ASSERT((sectrel_addr + num_bytes) <= Dee_host_section_size(self));
	self->hs_end -= num_bytes;
	memmovedown(self->hs_start + sectrel_addr,
	            self->hs_start + sectrel_addr + num_bytes,
	            Dee_host_section_size(self) - sectrel_addr);

	/* Adjust offsets of relocations that happen after the deleted area. */
	for (i = 0; i < self->hs_relc; ++i) {
		struct Dee_host_reloc *rel = &self->hs_relv[i];
		if (rel->hr_offset >= sectrel_addr)
			rel->hr_offset -= num_bytes;
	}

	/* Adjust addresses of symbols that appear after the deleted area */
	for (sym = self->hs_symbols; sym; sym = sym->_hs_next) {
		ASSERT(sym->hs_type == DEE_HOST_SYMBOL_SECT);
		ASSERT(sym->hs_value.sv_sect.ss_sect == self);
		if (sym->hs_value.sv_sect.ss_off >= sectrel_addr)
			sym->hs_value.sv_sect.ss_off -= num_bytes;
	}

	/* All sections that code after `self' need to have their base offset adjusted. */
	for (sect = TAILQ_NEXT(self, hs_link); sect;
	     sect = TAILQ_NEXT(sect, hs_link))
		sect->hs_badr -= num_bytes;
}

PRIVATE NONNULL((1, 2)) bool DCALL
Dee_host_reloc_shrinkjump(struct Dee_host_section *__restrict sect,
                          struct Dee_host_reloc *self) {
	byte_t *rel_templ = sect->hs_start + self->hr_offset;
	uintptr_t rel_adr = sect->hs_badr + self->hr_offset;
	uintptr_t rel_val;
	ASSERT(self->hr_offset < Dee_host_section_size(sect));
	switch (self->hr_vtype) {
	case DEE_HOST_RELOCVALUE_ABS:
		goto nope; /* Cannot shrink absolute relocation */
	case DEE_HOST_RELOCVALUE_SYM: {
		struct Dee_host_symbol *sym = self->hr_value.rv_sym;
		if (sym->hs_type == DEE_HOST_SYMBOL_ABS)
			goto nope; /* Cannot shrink absolute relocation */
		rel_val = Dee_host_symbol_value(sym);
	}	break;
	default: __builtin_unreachable();
	}
#ifdef HOSTASM_X86
	switch (self->hr_rtype) {

	case DEE_HOST_RELOC_PCREL32: {
		int64_t pcval;
		pcval = (int64_t)rel_val;
		pcval += (int32_t)UNALIGNED_GETLE32(rel_templ);
		pcval -= rel_adr;
		if (pcval >= INT8_MIN && pcval <= INT8_MAX) {
			int8_t Simm8 = (int8_t)(pcval + rel_adr - rel_val) + 3;
			if (self->hr_offset >= 1 && rel_templ[-1] == 0xe9) {
				/* jmpl <Simm32>   ->  jmp8 <Simm8>
				 * e9 XX XX XX XX  ->  eb XX */
				rel_templ[-1] = 0xeb;
				rel_templ[0]  = (byte_t)(uint8_t)Simm8;
				self->hr_rtype = DEE_HOST_RELOC_PCREL8;
				Dee_host_section_deltext(sect, self->hr_offset + 1, 3);
				return true;
			} else if (self->hr_offset >= 2 && (rel_templ[-2] == 0x0f &&
			                                    rel_templ[-1] >= 0x80 &&
			                                    rel_templ[-1] <= 0x8f)) {
				/* jccl <Simm32>      ->  jcc8 <Simm8>
				 * 0f 8x XX XX XX XX  ->  7x XX */
				rel_templ[-2] = 0x70 | (rel_templ[-1] & 0x0f);
				rel_templ[-1] = (byte_t)(uint8_t)Simm8;
				self->hr_rtype = DEE_HOST_RELOC_PCREL8;
				--self->hr_offset;
				Dee_host_section_deltext(sect, self->hr_offset + 1, 4);
				return true;
			}
		}
	}	break;

	default: break;
	}
#else /* HOSTASM_X86 */
#error "'HOSTASM_HAVE_SHRINKJUMPS' not implemented for this architecture"
#endif /* !HOSTASM_X86 */
nope:
	return false;
}

PRIVATE NONNULL((1)) bool DCALL
Dee_host_section_shrinkjumps(struct Dee_host_section *__restrict self) {
	bool result = false;
	size_t i;
	for (i = 0; i < self->hs_relc; ++i)
		result |= Dee_host_reloc_shrinkjump(self, &self->hs_relv[i]);
	return result;
}

/* Step #6: Try to shrink large in generated host text with smaller ones.
 * This is an arch-specific step. On x86 it replaces `jmpl' with `jmp8' (if possible) */
INTERN NONNULL((1)) void DCALL
Dee_function_assembler_shrinkjumps(struct Dee_function_assembler *__restrict self) {
	struct Dee_host_section *sect;
	uintptr_t badr;
	bool did_something;

	/* Figure out the relative base addresses of each section. */
	badr = 0;
	TAILQ_FOREACH (sect, &self->fa_sections, hs_link) {
		sect->hs_badr = badr;
		badr += Dee_host_section_size(sect);
	}

	/* Try to shrink host jump instructions. */
	do {
		did_something = false;
		TAILQ_FOREACH (sect, &self->fa_sections, hs_link) {
			did_something |= Dee_host_section_shrinkjumps(sect);
		}
	} while (did_something);
}
#endif /* HOSTASM_HAVE_SHRINKJUMPS */


INTERN NONNULL((1)) bool DCALL
Dee_host_section_reloc(struct Dee_host_section *__restrict self) {
	size_t i;
	for (i = 0; i < self->hs_relc; ++i) {
		struct Dee_host_reloc *rel = &self->hs_relv[i];
		byte_t *rel_addr = self->hs_base + rel->hr_offset;
		uintptr_t value = Dee_host_reloc_value(rel);
		switch (rel->hr_rtype) {
		case DEE_HOST_RELOC_NONE:
			break;

#ifdef DEE_HOST_RELOC_PCREL32
		case DEE_HOST_RELOC_PCREL32: {
			ptrdiff_t diff = (ptrdiff_t)(value - (uintptr_t)rel_addr);
			int32_t *addr  = (int32_t *)rel_addr;
			if (OVERFLOW_SADD(*addr, diff, addr))
				return false;
		}	break;
#endif /* DEE_HOST_RELOC_PCREL32 */

#ifdef DEE_HOST_RELOC_PCREL8
		case DEE_HOST_RELOC_PCREL8: {
			ptrdiff_t diff = (ptrdiff_t)(value - (uintptr_t)rel_addr);
			int8_t *addr   = (int8_t *)rel_addr;
			if (OVERFLOW_SADD(*addr, diff, addr))
				return false;
		}	break;
#endif /* DEE_HOST_RELOC_PCREL8 */

		default: __builtin_unreachable();
		}
	}
	return true;
}

/* Step #6: Link blocks into an executable function blob.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_assembler_output(struct Dee_function_assembler *__restrict self,
                              struct Dee_hostfunc *__restrict result) {
	byte_t *writer;
	struct Dee_host_section *sect;
again:
	if unlikely(Dee_rawhostfunc_init(&result->hf_raw, self->fa_sectsize)) {
		if (Dee_CollectMemory(self->fa_sectsize))
			goto again;
		goto err;
	}
	result->hf_raw.rhf_entry.hfe_addr = result->hf_raw._rhf_base;

	/* Figure out the final base addresses of sections. */
	writer = (byte_t *)result->hf_raw._rhf_base;
	TAILQ_FOREACH (sect, &self->fa_sections, hs_link) {
		size_t size = Dee_host_section_size(sect);
		sect->hs_base = writer;
		writer += size;
	}

	/* Output sections and resolve relocations. */
	writer = (byte_t *)result->hf_raw._rhf_base;
	TAILQ_FOREACH (sect, &self->fa_sections, hs_link) {
		size_t size = Dee_host_section_size(sect);
		ASSERT(writer == sect->hs_base);
		writer = (byte_t *)mempcpy(writer, sect->hs_start, size);
		if (!Dee_host_section_reloc(sect)) {
			/* TODO: Error if overflow on x86_64 (to re-compile w/ large memory model) */
			DeeError_NOTIMPLEMENTED();
			goto err_result;
		}
	}
	if unlikely(Dee_rawhostfunc_mkexec(&result->hf_raw)) {
		Dee_BadAlloc(self->fa_sectsize);
		goto err_result;
	}

	/* Let the generated function inherit inlined references (if there are any).
	 * NOTE: this is the only reason why you can call output only once, so if it
	 *       ever becomes necessary to output the same code multiple times, you
	 *       have to change this part to create copies of the refs vector! */
	result->hf_refs = NULL;
	if (self->fa_irefs.ir_size > 0) {
		size_t i;
		DREF DeeObject **refs = self->fa_irefs.ir_elem;
		for (i = 0; i <= self->fa_irefs.ir_mask; ++i) {
			/* Fill gaps with references to `Dee_None' */
			if (refs[i] == NULL)
				refs[i] = DeeNone_NewRef();
		}
		refs[i] = NULL; /* Sentinel */
		result->hf_refs = refs;
		self->fa_irefs.ir_elem = NULL; /* Stolen... */
	}

	return 0;
err_result:
	Dee_rawhostfunc_fini(&result->hf_raw);
err:
	return -1;
}

/* High-level wrapper function to fully assemble `function' into its host-asm equivalent.
 * @param: cc:    Calling convention of the generated function
 * @param: flags: Set of `DEE_FUNCTION_ASSEMBLER_F_*'
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_assemble(DeeFunctionObject *__restrict function,
             struct Dee_hostfunc *__restrict result,
             Dee_hostfunc_cc_t cc, uint16_t flags) {
	struct Dee_function_assembler assembler;
	Dee_function_assembler_init(&assembler, function, function->fo_code, cc, flags);

	/* Special case: deemon code that contains user-written deemon assembly
	 *               requires special care to include some extra checks in
	 *               generated host assembly. */
	if unlikely(assembler.fa_code->co_flags & CODE_FASSEMBLY)
		assembler.fa_flags |= DEE_FUNCTION_ASSEMBLER_F_SAFE;

	/* Go through all the steps of assembling the function. */
	if unlikely(Dee_function_assembler_loadblocks(&assembler))
		goto err_assembler;
	if (!(assembler.fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYDEL)) {
		if unlikely(Dee_function_assembler_loadlocuse(&assembler))
			goto err_assembler;
	}
	if unlikely(Dee_function_assembler_compileblocks(&assembler))
		goto err_assembler;
	if unlikely(Dee_function_assembler_trimdead(&assembler))
		goto err_assembler;
	if unlikely(Dee_function_assembler_compilemorph(&assembler))
		goto err_assembler;
	if unlikely(Dee_function_assembler_ordersections(&assembler))
		goto err_assembler;
#ifdef HOSTASM_HAVE_SHRINKJUMPS
	Dee_function_assembler_shrinkjumps(&assembler);
#endif /* HOSTASM_HAVE_SHRINKJUMPS */
	if unlikely(Dee_function_assembler_output(&assembler, result))
		goto err_assembler;
	Dee_function_assembler_fini(&assembler);
	return 0;
err_assembler:
	Dee_function_assembler_fini(&assembler);
/*err:*/
	return -1;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_ASSEMBLER_C */
