/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
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
#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/none.h>
#include <deemon/tuple.h>

#include <hybrid/bitset.h>        /* bitset_test */
#include <hybrid/byteswap.h>      /* UNALIGNED_GETLE32 */
#include <hybrid/limitcore.h>     /* __INT8_MAX__, __INT8_MIN__ */
#include <hybrid/overflow.h>      /* OVERFLOW_SADD */
#include <hybrid/sequence/list.h> /* TAILQ_* */
#include <hybrid/unaligned.h>     /* UNALIGNED_GETLE32 */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, ptrdiff_t, size_t */
#include <stdint.h>  /* INT8_MAX, INT8_MIN, int8_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uintptr_t */

DECL_BEGIN

#ifdef __INTELLISENSE__
#define DO /* nothing */
#else /* __INTELLISENSE__ */
#define DO(x) if unlikely(x) goto err
#endif /* !__INTELLISENSE__ */
#define EDO(err, x) if unlikely(x) goto err

#ifndef INT8_MIN
#define INT8_MIN __INT8_MIN__
#endif /* !INT8_MIN */
#ifndef INT8_MAX
#define INT8_MAX __INT8_MAX__
#endif /* !INT8_MAX */

STATIC_ASSERT(HOST_CC_CALL == (0));
STATIC_ASSERT(HOST_CC_CALL_KW == (HOST_CC_F_KW));
STATIC_ASSERT(HOST_CC_CALL_TUPLE == (HOST_CC_F_TUPLE));
STATIC_ASSERT(HOST_CC_CALL_TUPLE_KW == (HOST_CC_F_KW | HOST_CC_F_TUPLE));
STATIC_ASSERT(HOST_CC_FUNC_CALL == (HOST_CC_F_FUNC));
STATIC_ASSERT(HOST_CC_FUNC_CALL_KW == (HOST_CC_F_KW | HOST_CC_F_FUNC));
STATIC_ASSERT(HOST_CC_FUNC_CALL_TUPLE == (HOST_CC_F_TUPLE | HOST_CC_F_FUNC));
STATIC_ASSERT(HOST_CC_FUNC_CALL_TUPLE_KW == (HOST_CC_F_KW | HOST_CC_F_TUPLE | HOST_CC_F_FUNC));
STATIC_ASSERT(HOST_CC_THISCALL == (HOST_CC_F_THIS));
STATIC_ASSERT(HOST_CC_THISCALL_KW == (HOST_CC_F_KW | HOST_CC_F_THIS));
STATIC_ASSERT(HOST_CC_THISCALL_TUPLE == (HOST_CC_F_TUPLE | HOST_CC_F_THIS));
STATIC_ASSERT(HOST_CC_THISCALL_TUPLE_KW == (HOST_CC_F_KW | HOST_CC_F_TUPLE | HOST_CC_F_THIS));
STATIC_ASSERT(HOST_CC_FUNC_THISCALL == (HOST_CC_F_FUNC | HOST_CC_F_THIS));
STATIC_ASSERT(HOST_CC_FUNC_THISCALL_KW == (HOST_CC_F_KW | HOST_CC_F_FUNC | HOST_CC_F_THIS));
STATIC_ASSERT(HOST_CC_FUNC_THISCALL_TUPLE == (HOST_CC_F_TUPLE | HOST_CC_F_FUNC | HOST_CC_F_THIS));
STATIC_ASSERT(HOST_CC_FUNC_THISCALL_TUPLE_KW == (HOST_CC_F_KW | HOST_CC_F_TUPLE | HOST_CC_F_FUNC | HOST_CC_F_THIS));
STATIC_ASSERT((HOST_CC_F_KW | HOST_CC_F_TUPLE | HOST_CC_F_FUNC | HOST_CC_F_THIS) == 15);


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
basic_block_compile(struct basic_block *__restrict self,
                    struct function_assembler *__restrict assembler) {
	int result;
	struct fungen gen;
	gen.fg_block     = self;
	gen.fg_sect      = &self->bb_htext;
	gen.fg_assembler = assembler;
	gen.fg_state     = self->bb_mem_start;
	_fg_initcommon(&gen);
	ASSERT(gen.fg_state);
	memstate_incref(gen.fg_state);
#ifdef DEFINED_host_section_unwind_setsp_initial
	host_section_unwind_setsp_initial(gen.fg_sect, gen.fg_state->ms_host_cfa_offset);
#else /* DEFINED_host_section_unwind_setsp_initial */
	result = host_section_unwind_setsp(gen.fg_sect, gen.fg_state->ms_host_cfa_offset);
	if likely(result == 0)
#endif /* !DEFINED_host_section_unwind_setsp_initial */
	{
		result = fg_genall(&gen);
	}
	memstate_decref(gen.fg_state);
	ASSERT(gen.fg_block == self);
	ASSERT(gen.fg_sect == &self->bb_htext);
	ASSERT(gen.fg_assembler == assembler);
	return result;
}

/* Given the set of all basic block that have yet to be compiled,
 * find the one that has the most entry descriptors with a defined
 * `jd_stat' and return that one.
 * @return: (size_t)-1: Everything has been compiled. */
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
find_next_block_to_compile(struct function_assembler *__restrict self) {
	size_t result_index = (size_t)-1;
	size_t i, result_missing_entries = (size_t)-1;
	for (i = 0; i < self->fa_blockc; ++i) {
		size_t j, num_missing_entries;
		struct basic_block *block = self->fa_blockv[i];
		if (block->bb_mem_start == NULL)
			continue; /* Block hasn't been reached at all, yet. */
		if (block->bb_mem_end != NULL)
			continue; /* Already compiled. */
		if (i == 0)
			return 0; /* Always compile the entry-block first! */
		num_missing_entries = 0;
		for (j = 0; j < block->bb_entries.jds_size; ++j) {
			struct jump_descriptor *entry = block->bb_entries.jds_list[j];
			if (entry->jd_stat == NULL)
				++num_missing_entries;
		}
		for (j = 0; j < self->fa_blockc; ++j) {
			struct basic_block *block2 = self->fa_blockv[j];
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


PRIVATE WUNUSED NONNULL((1)) DREF struct memstate *DCALL
function_assembler_alloc_zero_memstate(struct function_assembler const *__restrict self) {
	lid_t lid, n_optargs;
	DREF struct memstate *result;

	/* Setup the state of the function's first (entry) block. */
	result = memstate_alloc(self->fa_xlocalc);
	if unlikely(!result)
		goto done;
	result->ms_refcnt = 1;
	result->ms_host_cfa_offset = 0;
	result->ms_localc    = self->fa_xlocalc;
	result->ms_stackc    = 0;
	result->ms_stacka    = 0;
	result->ms_flags     = MEMSTATE_F_NORMAL;
	result->ms_uargc_min = self->fa_code->co_argc_min;
	n_optargs            = self->fa_code->co_argc_max - self->fa_code->co_argc_min;
	bzero(result->ms_rinuse, sizeof(result->ms_rinuse));
	memequivs_init(&result->ms_memequiv);
	memstate_hregs_clear_usage(result);
	result->ms_stackv = NULL;

	/* Initially, all variables are unbound... */
	for (lid = 0; lid < result->ms_localc - n_optargs; ++lid)
		memval_init_local_unbound(&result->ms_localv[lid]);

	/* ... except for optional argument cache slots, which we initialize as undefined.
	 * This special handling then causes optional arguments to be cached on first use
	 * into their respective slots. */
	for (; lid < result->ms_localc; ++lid)
		memval_init_undefined(&result->ms_localv[lid]);
done:
	return result;
}

/* Allocate and return the initial memory-state when the
 * generated function is entered at the start of the prolog.
 * @return: * :   The initial memory-state
 * @return: NULL: Error */
PRIVATE WUNUSED NONNULL((1)) DREF struct memstate *DCALL
function_assembler_alloc_init_memstate(struct function_assembler const *__restrict self) {
	host_cc_t cc = self->fa_cc;
	struct memstate *state;
	ASSERT(self->fa_blockc >= 1);

	/* Figure out how many locals we need. */
	state = function_assembler_alloc_zero_memstate(self);
	if unlikely(!state)
		goto err;

	/* Set-up the mem-state to indicate where arguments are stored at */
#ifdef HOSTASM_X86
	{
#ifdef HOSTASM_X86_64
#define memval_set_x86_arg(self, argi)                                      \
	(memval_init_hreg(self, truearg_regno[argi], 0, NULL, MEMOBJ_F_NORMAL), \
	 memstate_incrinuse(state, memval_direct_hreg_getreg(self)))
		PRIVATE host_regno_t const truearg_regno[4] = {
			HOST_REGNO_R_ARG0,
			HOST_REGNO_R_ARG1,
			HOST_REGNO_R_ARG2,
			HOST_REGNO_R_ARG0,
#ifdef HOST_REGNO_R_ARG4
			HOST_REGNO_R_ARG4,
#endif /* HOST_REGNO_R_ARG4 */
		};
#else /* HOSTASM_X86_64 */
#define memval_set_x86_arg(self, argi)                                       \
	memval_init_hstackind(self, (uintptr_t)(-(ptrdiff_t)(((argi) + 1) * 4)), \
	                          0, NULL, MEMOBJ_F_NORMAL)
#endif /* !HOSTASM_X86_64 */
		size_t argi = 0;
		lid_t extra_base = self->fa_localc;
		if (cc & HOST_CC_F_FUNC) {
			memval_set_x86_arg(&state->ms_localv[extra_base + MEMSTATE_XLOCAL_A_FUNC], argi);
			++argi;
		}
		if (cc & HOST_CC_F_THIS) {
			memval_set_x86_arg(&state->ms_localv[extra_base + MEMSTATE_XLOCAL_A_THIS], argi);
			++argi;
		}
		if (cc & HOST_CC_F_TUPLE) {
			memval_set_x86_arg(&state->ms_localv[extra_base + MEMSTATE_XLOCAL_A_ARGS], argi);
			++argi;
		} else {
			memval_set_x86_arg(&state->ms_localv[extra_base + MEMSTATE_XLOCAL_A_ARGC], argi);
			++argi;
			memval_set_x86_arg(&state->ms_localv[extra_base + MEMSTATE_XLOCAL_A_ARGV], argi);
			++argi;
		}
		if (cc & HOST_CC_F_KW) {
#if defined(HOSTASM_X86_64) && !defined(HOST_REGNO_R_ARG4)
			if (argi == 5) {
				struct memval *a_kw = &state->ms_localv[extra_base + MEMSTATE_XLOCAL_A_KW];
#ifdef HOSTASM_X86_64_MSABI
				memval_init_hstackind(a_kw, (uintptr_t)(-(5 * HOST_SIZEOF_POINTER)), 0, NULL, MEMOBJ_F_NORMAL);
#else /* HOSTASM_X86_64_MSABI */
				memval_init_hstackind(a_kw, (uintptr_t)(-(1 * HOST_SIZEOF_POINTER)), 0, NULL, MEMOBJ_F_NORMAL);
#endif /* !HOSTASM_X86_64_MSABI */
			} else
#endif /* HOSTASM_X86_64 && !HOST_REGNO_R_ARG4 */
			{
				memval_set_x86_arg(&state->ms_localv[extra_base + MEMSTATE_XLOCAL_A_KW], argi);
			}
		}
#undef memval_set_x86_arg
	}
#else /* ... */
#error "Initial register state not implemented for this architecture"
#endif /* !... */

	/* Some arguments have known object types. */
	if (cc & HOST_CC_F_FUNC)
		memval_direct_settypeof(&state->ms_localv[self->fa_xlocalc + MEMSTATE_XLOCAL_A_FUNC], &DeeFunction_Type);
	if (cc & HOST_CC_F_TUPLE)
		memval_direct_settypeof(&state->ms_localv[self->fa_xlocalc + MEMSTATE_XLOCAL_A_ARGS], &DeeTuple_Type);

	return state;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_makeprolog(struct fungen *__restrict self) {
	struct fg_branch branch;
	DeeCodeObject *code = self->fg_assembler->fa_code;
	uint16_t co_argc_min = code->co_argc_min;
	uint16_t co_argc_max = code->co_argc_max;
	if (self->fg_assembler->fa_cc & HOST_CC_F_KW) {
		/* Generate code:
		 * >> size_t argc;
		 * >> DeeObject **argv;
		 * >> DeeObject *kw;
		 * >> DeeObject **kw_argv = alloca({co_argc_max * sizeof(DeeObject *)});
		 * >> if (kw != NULL) {
		 * >>     if likely(kw->ob_type == &DeeKwds_Type) {
		 * >>         DeeObject **kwds_argv;
		 * >> #if !(co_flags & CODE_FVARKWDS)
		 * >>         size_t kw_used = 0;
		 * >> #endif
		 * >>         size_t effective_argc;
		 * >>         if unlikely(OVERFLOW_USUB(argc, kw->kw_size, &effective_argc)) {
		 * >>             err_keywords_bad_for_argc(kw, argc, argv);
		 * >>             HANDLE_EXCEPT();
		 * >>         }
		 * >>         argc = effective_argc;
		 * >> #if !(co_flags & CODE_FVARARGS)
		 * >>         if (argc > <co_argc_max>))
		 * >>             err_invalid_argc(...); // ERROR: Too many positional arguments
		 * >> #endif
		 * >>         kwds_argv = argv + argc;
		 * >> #if {co_argc_min} > 0
		 * >>         if (argc < {co_argc_min}) {
		 * >>             size_t i = argc;
		 * >>             do {
		 * >>                 size_t index = DeeKwds_IndexOf(kw, {co_keywords}[i]);
		 * >>                 if (index == (size_t)-1) {
		 * >>                     err_invalid_argc_missing_kw(...);
		 * >>                     HANDLE_EXCEPT();
		 * >>                 }
		 * >>                 kw_argv[i] = kwds_argv[index];
		 * >> #if !(co_flags & CODE_FVARKWDS)
		 * >>                 ++kw_used;
		 * >> #endif
		 * >>                 ++i;
		 * >>             } while (i < {co_argc_min});
		 * >>         }
		 * >> #endif
		 * >> #if {co_argc_max} > {co_argc_min}
		 * >> #for (size_t i = {co_argc_min}; i < {co_argc_max}; ++i)
		 * >>         size_t index = DeeKwds_IndexOf(kw, {co_keywords[i]});
		 * >>         DeeObject *temp = {co_defaultv[i]};
		 * >>         if (index != (size_t)-1) {
		 * >>             temp = kwds_argv[index];
		 * >> #if !(co_flags & CODE_FVARKWDS)
		 * >>             ++kw_used;
		 * >> #endif
		 * >>         }
		 * >>         kw_argv[{i}] = temp;
		 * >> #endfor
		 * >> #endif
		 * >> #if !(co_flags & CODE_FVARKWDS)
		 * >>         if (kw_used < kw->kw_size) {
		 * >>             err_invalid_argc(...); // ERROR: Unused keyword arguments
		 * >>             HANDLE_EXCEPT();
		 * >>         }
		 * >> #endif
		 * >>     } else {
		 * >> #if !(co_flags & CODE_FVARARGS)
		 * >>         if (argc > <co_argc_max>))
		 * >>             err_invalid_argc(...); // ERROR: Too many positional arguments
		 * >> #endif
		 * >> #if !(co_flags & CODE_FVARKWDS)
		 * >>         size_t kw_used = {co_argc_max - co_argc_min};
		 * >> #endif
		 * >> #if {co_argc_min} > 0
		 * >>         if (argc < {co_argc_min}) {
		 * >>             size_t i = argc;
		 * >>             do {
		 * >>                 DeeObject *arg = DeeKw_TryGetItemNR(kw, {co_keywords}[i]);
		 * >>                 if (!arg)
		 * >>                     HANDLE_EXCEPT();
		 * >>                 if (arg == ITER_DONE)
		 * >>                     err_invalid_argc(...); // ERROR: Too few positional arguments / missing keyword argument
		 * >>                 kw_argv[i] = arg;
		 * >> #if !(co_flags & CODE_FVARKWDS)
		 * >>                 ++kw_used;
		 * >> #endif
		 * >>                 ++i;
		 * >>             } while (i < {co_argc_min});
		 * >>         }
		 * >> #endif
		 * >> #if {co_argc_max} > {co_argc_min}
		 * >> #for (size_t i = {co_argc_min}; i < {co_argc_max}; ++i)
		 * >>         DeeObject *temp = DeeKw_GetItemNRDef(kw, {co_keywords[i]}, ITER_DONE);
		 * >>         if (!temp)
		 * >>             HANDLE_EXCEPT();
		 * >>         if (temp == ITER_DONE) {
		 * >> #if !(co_flags & CODE_FVARKWDS)
		 * >>             --kw_used;
		 * >> #endif
		 * >> #if co_defaultv[i]
		 * >>             temp = {co_defaultv[i]};
		 * >> #endif
		 * >>         }
		 * >>         kw_argv[{i}] = temp;
		 * >> #endfor
		 * >> #endif
		 * >> #if !(co_flags & CODE_FVARKWDS)
		 * >>         size_t kw_argc = DeeObject_Size(kw);
		 * >>         if (kw_argc > kw_used) {
		 * >>             if (kw_argc == (size_t)-1)
		 * >>                 HANDLE_EXCEPT();
		 * >>             err_invalid_argc(...); // ERROR: Unused keyword arguments
		 * >>         }
		 * >> #endif
		 * >>     }
		 * >> }
		 */
		/* TODO */
		return 0;
	}

	if (co_argc_min == 0 && (code->co_flags & CODE_FVARARGS)) {
		/* Special case: *any* number of arguments is accepted */
	} else {
		struct host_symbol *Lerr_invalid_argc;
		struct memloc l_argc_min;
		Lerr_invalid_argc = fg_newsym_named(self, ".Lerr_invalid_argc");
		if unlikely(!Lerr_invalid_argc)
			goto err;
		memloc_init_const(&l_argc_min, (void const *)(uintptr_t)co_argc_min);
		DO(fg_vpush_argc(self)); /* argc */
		DO(fg_vdirect1(self));   /* argc */
		if (code->co_flags & CODE_FVARARGS) {
			/* if (argc < co_argc_min) err_invalid_argc(...); */
			DO(fg_gjcc(self, fg_vtopdloc(self), &l_argc_min, false, Lerr_invalid_argc, NULL, NULL));
		} else if (co_argc_min == co_argc_max) {
			/* if (argc != co_argc_min) err_invalid_argc(...); */
			DO(fg_gjcc(self, fg_vtopdloc(self), &l_argc_min, false, Lerr_invalid_argc, NULL, Lerr_invalid_argc));
		} else {
			struct memloc l_argc_max;
			memloc_init_const(&l_argc_max, (void const *)(uintptr_t)co_argc_max);
			/* if (argc < co_argc_min || argc > co_argc_max) err_invalid_argc(...); */
			DO(fg_gjcc(self, fg_vtopdloc(self), &l_argc_min, false, Lerr_invalid_argc, NULL, NULL));
			DO(fg_gjcc(self, fg_vtopdloc(self), &l_argc_max, false, NULL, NULL, Lerr_invalid_argc));
		}
		DO(fg_vcold_enter(self, &branch));
		host_symbol_setsect(Lerr_invalid_argc, fg_gettext(self));                                /* argc */
		EDO(err_branch, fg_vpush_const(self, self->fg_assembler->fa_code));                      /* argc, code */
		EDO(err_branch, fg_vswap(self));                                                         /* code, argc */
		EDO(err_branch, fg_vcallapi(self, &libhostasm_rt_err_invalid_argc, VCALL_CC_EXCEPT, 2)); /* N/A */
		DO(fg_vjx_leave_noreturn(self, &branch));                                                /* argc */
		DO(fg_vpop(self));                                                                       /* N/A */
	}

	/*
	 * At this point, arguments, varargs and varkwds are accessed as follows:
	 *
	 * if HOST_CC_F_KW:
	 * >> varkwds = ({ // CAUTION: Must be decref'd using "DeeKwBlackList_Decref()"
	 * >>     DeeKwBlackList_New({code}, argc, argv, kw);
	 * >> });
	 *
	 * if HOST_CC_F_KW:
	 * >> arg[{i}] = ({
	 * >>     if ({i} < argc) {
	 * >>         argv[{i}];
	 * >>     } else {
	 * >> #if {co_argc_min} < {co_argc_max} && {co_defaultv[i - co_argc_min] == NULL}
	 * >>         if (!kw_argv[{i}]) {
	 * >>             err_unbound_arg(...);
	 * >>             HANDLE_EXCEPT();
	 * >>         }
	 * >> #endif
	 * >>         kw_argv[{i}];
	 * >>     }
	 * >> });
	 *
	 * if !HOST_CC_F_KW:
	 * >> arg[{i}] = ({
	 * >>     __assume({i} < {co_argc_max});
	 * >> #if {co_argc_min} >= {co_argc_max}
	 * >>     argv[{i}];
	 * >> #else // {co_argc_min} >= {co_argc_max}
	 * >>     if ({i} < argc) {
	 * >>         argv[{i}];
	 * >>     } else {
	 * >> #if co_defaultv[i - co_argc_min] != NULL
	 * >>         {co_defaultv[i - co_argc_min]};
	 * >> #else
	 * >>         err_unbound_arg(...);
	 * >>         HANDLE_EXCEPT();
	 * >> #endif
	 * >>     }
	 * >> #endif // {co_argc_min} < {co_argc_max}
	 * >> });
	 *
	 * >> varargs = ({
	 * >> #if {co_argc_min} < {co_argc_max}
	 * >>     if (argc < {co_argc_max}) {
	 * >>         Dee_EmptyTuple;
	 * >>     } else
	 * >> #endif
	 * >>     {
	 * >>         DeeTuple_New(argc - {co_argc_max}, argv + {co_argc_max});
	 * >>     }
	 * >> });
	 * 
	 */

	(void)self;
	return 0;
err_branch:
	fg_branch_fini(&branch);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_makeprolog_cleanup(struct fungen *__restrict self) {
	/* Delete all locals initialized by the prolog
	 * that aren't used within the function. */
	lid_t lid;
	struct memstate *state = self->fg_state;
	struct basic_block *block0 = self->fg_block;
	for (lid = 0; lid < state->ms_localc; ++lid) {
		if (bitset_test(block0->bb_locuse, lid))
			continue;
		if (memval_isdirect(&state->ms_localv[lid]) &&
		    memval_direct_local_neverbound(&state->ms_localv[lid]))
			continue;
		if unlikely(fg_vdel_local(self, lid))
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
function_assembler_makeprolog(struct function_assembler *__restrict self,
                              /*inherit(always)*/ DREF struct memstate *__restrict state) {
	int result;
	struct fungen gen;
	gen.fg_assembler   = self;
	gen.fg_block       = self->fa_blockv[0];
	gen.fg_sect        = &self->fa_prolog;
	gen.fg_state       = state; /* Inherit reference */
	gen.fg_nextlastloc = NULL;
	_fg_initcommon(&gen);
	ASSERT(gen.fg_block);
	ASSERT(self->fa_prolog_end == NULL);
	ASSERT(gen.fg_state != NULL);
	result = fg_makeprolog(&gen);
	if likely(result == 0)
		result = fg_makeprolog_cleanup(&gen);
	ASSERT(gen.fg_state != NULL);
	ASSERT(self->fa_prolog_end == NULL);
	if likely(result == 0) {
		self->fa_prolog_end = gen.fg_state; /* Inherit reference */
	} else {
		memstate_decref(gen.fg_state);
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
 * always identical (or compatible; i.e.: MEMVAL_F_LOCAL_UNKNOWN can
 * be set at the start of a block, but doesn't need to be set at the
 * end of a preceding block). When not compatible, extra block(s) are
 * inserted with `bb_deemon_start==bb_deemon_end', but non-empty host
 * assembly, which serves the purpose of transforming memory states.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
function_assembler_compileblocks(struct function_assembler *__restrict self) {
	size_t block_i;
	struct memstate *state;
	struct basic_block *block;
	ASSERT(self->fa_blockc >= 1);

	/* Setup the state of the function's first (entry) block. */
	state = function_assembler_alloc_init_memstate(self);
	if unlikely(!state)
		goto err;

	/* Generate the function prolog. */
	if unlikely(function_assembler_makeprolog(self, state))
		goto err;

	/* Set the mem-state for the initial block. */
	ASSERT(self->fa_prolog_end);
	block = self->fa_blockv[0];
	ASSERT(block);
	ASSERT(block->bb_mem_start == NULL);
	memstate_incref(self->fa_prolog_end);
	block->bb_mem_start = self->fa_prolog_end; /* Inherit reference */

	/* Compile all basic blocks until everything has been compiled and everyone is happy. */
	while ((block_i = find_next_block_to_compile(self)) != (size_t)-1) {
		block = self->fa_blockv[block_i];
		Dee_DPRINTF("basic_block_compile: %" PRFuSIZ " [%.4" PRFx32 "-%.4" PRFx32 "]\n",
		            block_i,
		            function_assembler_addrof(self, block->bb_deemon_start),
		            function_assembler_addrof(self, block->bb_deemon_end - 1));
#ifndef NO_HOSTASM_DEBUG_PRINT
		_memstate_debug_print(block->bb_mem_start, self, block->bb_deemon_start);
#endif /* !NO_HOSTASM_DEBUG_PRINT */
		ASSERT(block->bb_mem_start != NULL);
		ASSERT(block->bb_mem_end == NULL);
		ASSERT(block->bb_htext.hs_end == block->bb_htext.hs_start);
		ASSERT(block->bb_htext.hs_relc == 0);

		/* Compile this block. */
		if unlikely(basic_block_compile(block, self))
			goto err;
	}

	return 0;
err:
	return -1;
}


struct host_section_set {
	struct host_section **hss_list; /* [1..1][0..hss_size] List of basic blocks (sorted by pointer) */
	size_t                hss_size; /* # of items in `hss_list' */
#ifndef NDEBUG
	size_t                hss_smax; /* Max # of items in `hss_list' */
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
	 ((self)->hss_list = (struct host_section **)Dee_Mallocac(smax, sizeof(struct host_section *))) != NULL ? 0 : -1)


/* Try to insert `sect' into `self' (if it wasn't inserted already) */
PRIVATE NONNULL((1, 2)) void DCALL
host_section_set_insert(struct host_section_set *__restrict self,
                        struct host_section *sect) {
	size_t lo = 0, hi = self->hss_size;
	ASSERT(sect);
	while (lo < hi) {
		size_t mid = (lo + hi) / 2;
		struct host_section *ex = self->hss_list[mid];
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
	           sizeof(struct host_section *));
	self->hss_list[lo] = sect;
	++self->hss_size;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
host_section_set_contains(struct host_section_set const *__restrict self,
                          struct host_section *sect) {
	size_t lo = 0, hi = self->hss_size;
	while (lo < hi) {
		size_t mid = (lo + hi) / 2;
		struct host_section *ex = self->hss_list[mid];
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
                                            struct host_section *sect) {
	size_t i;
	for (i = 0; i < sect->hs_relc; ++i) {
		struct host_reloc *rel = &sect->hs_relv[i];
		switch (rel->hr_vtype) {
		case DEE_HOST_RELOCVALUE_SYM: {
			struct host_symbol const *sym = rel->hr_value.rv_sym;
			switch (sym->hs_type) {
#if 0 /* We only care about sections that might be used for exception handling! */
			case HOST_SYMBOL_JUMP:
				host_section_set_insert(self, &sym->hs_value.sv_jump->jd_morph);
				host_section_set_insert(self, &sym->hs_value.sv_jump->jd_to->bb_htext);
				break;
#endif
			case HOST_SYMBOL_SECT:
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
function_assembler_trimdead(struct function_assembler *__restrict self) {
	struct host_section_set referenced_sections;
	size_t i, num_sections;

	/* Note how we skip the primary block here.
	 * That's because it could never not have been compiled. */
	ASSERT(self->fa_blockc >= 1);
	ASSERT(self->fa_blockv[0]);
	ASSERT(self->fa_blockv[0]->bb_mem_start != NULL);
	ASSERT(self->fa_blockv[0]->bb_mem_end != NULL);
	for (i = 1; i < self->fa_blockc;) {
		struct basic_block *block = self->fa_blockv[i];
		ASSERT((block->bb_mem_start != NULL) == (block->bb_mem_end != NULL));
		if (block->bb_mem_start == NULL) {
			/* Fully get rid of this block. */
			--self->fa_blockc;
			memmovedownc(&self->fa_blockv[i],
			             &self->fa_blockv[i + 1],
			             self->fa_blockc - i,
			             sizeof(struct basic_block *));
			basic_block_destroy(block);
			continue;
		}
		if (block->bb_deemon_end < block->bb_deemon_end_r) {
			/* Block ends with a late-detected unreachable instruction.
			 * In this case, it is possible that there are further jump
			 * descriptors for parts of the code that hasn't been compiled,
			 * which we must get rid of now. */
			basic_block_trim_unused_exits(block);
		}
		++i;
	}

	num_sections = 1 + self->fa_blockc + self->fa_except_exitc;
	{
		struct host_section *sect = &self->fa_prolog;
		while ((sect = sect->hs_cold) != NULL)
			++num_sections;
	}
	for (i = 0; i < self->fa_blockc; ++i) {
		struct basic_block *block = self->fa_blockv[i];
		struct host_section *sect;
		ASSERT(block);
		ASSERT(block->bb_mem_start);
		ASSERT(block->bb_mem_end);
		sect = &block->bb_htext;
		while ((sect = sect->hs_cold) != NULL)
			++num_sections;
	}
	for (i = 0; i < self->fa_except_exitc; ++i) {
		struct except_exitinfo *xinfo = self->fa_except_exitv[i];
		struct host_section *sect;
		ASSERT(xinfo);
		sect = &xinfo->exi_text;
		while ((sect = sect->hs_cold) != NULL)
			++num_sections;
	}

	/* Go through all blocks and check which exception exit info blocks
	 * are actually still being used, and remove those that aren't. */
	if unlikely(host_section_set_init(&referenced_sections, num_sections))
		goto err;
	{
		struct host_section *sect = &self->fa_prolog;
		do {
			host_section_set_insertall_from_relocations(&referenced_sections, sect);
		} while ((sect = sect->hs_cold) != NULL);
	}
	for (i = 0; i < self->fa_blockc; ++i) {
		struct basic_block *block = self->fa_blockv[i];
		struct host_section *sect;
		ASSERT(block);
		ASSERT(block->bb_mem_start);
		ASSERT(block->bb_mem_end);
		sect = &block->bb_htext;
		do {
			host_section_set_insertall_from_relocations(&referenced_sections, sect);
		} while ((sect = sect->hs_cold) != NULL);
	}

	/* Go through exception handlers and remove those that aren't referenced. */
	for (i = 0; i < self->fa_except_exitc;) {
		struct except_exitinfo *info = self->fa_except_exitv[i];
		struct host_section *sect = &info->exi_text;
		bool found_any = false;
		do {
			found_any = host_section_set_contains(&referenced_sections, sect);
		} while (!found_any && (sect = sect->hs_cold) != NULL);
		if (found_any) {
			++i;
			continue;
		}

		/* Unreferenced exit info blob -> get rid of it! */
		info->exi_next = self->fa_except_del; /* Must keep the block around (in case symbols reference its sections) */
		self->fa_except_del = info;
		--self->fa_except_exitc;
		memmovedownc(&self->fa_except_exitv[i],
		             &self->fa_except_exitv[i + 1],
		             self->fa_except_exitc - i,
		             sizeof(struct except_exitinfo *));
	}
	host_section_set_fini(&referenced_sections);

	return 0;
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1, 2)) struct except_exitinfo *DCALL
function_assembler_except_select_predecessor(struct function_assembler *__restrict self,
                                             struct except_exitinfo_id const *__restrict newinfo) {
	size_t i, result_score = (size_t)-1;
	struct except_exitinfo *result = NULL;
	for (i = 0; i < self->fa_except_exitc; ++i) {
		size_t c_score;
		struct except_exitinfo *c = self->fa_except_exitv[i];
		if (except_exitinfo_wascompiled(c))
			continue;
		c_score = except_exitinfo_id_distance(except_exitinfo_asid(c), newinfo);
		if (c_score <= result_score) {
			result       = c;
			result_score = c_score;
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_gretNULL(struct fungen *__restrict self) {
	struct memloc zero;
	memloc_init_const(&zero, 0);
	return fg_gret(self, &zero);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
function_assembler_compileexcept(struct function_assembler *__restrict self) {
	void *_memstatebuf[memstate_sizeof_constexpr(1) / sizeof(void *)];
	void *_exitidbuf[offsetof(struct except_exitinfo_id, exi_memrefv) / sizeof(void *)];
	struct memstate *state = (struct memstate *)_memstatebuf;
	struct except_exitinfo_id *exit_id = (struct except_exitinfo_id *)_exitidbuf;
	struct fungen gen;
	struct except_exitinfo *info, *next;
	ASSERT(self->fa_blockc >= 1);

	/* Setup the fake mem-state */
	gen.fg_assembler = self;
	gen.fg_block     = self->fa_blockv[self->fa_blockc - 1];
	gen.fg_sect      = &gen.fg_block->bb_htext;
	gen.fg_state     = state;
	_fg_initcommon(&gen);
	ASSERT(gen.fg_state != NULL);
	state->ms_refcnt    = 1;
	state->ms_localc    = 1;
	state->ms_uargc_min = 0;
	state->ms_flags     = MEMSTATE_F_NORMAL;
	state->ms_stacka    = 0;
	state->ms_stackc    = 0;
	state->ms_stackv    = NULL;
	memval_init_undefined(&state->ms_localv[0]);
	exit_id->exi_cfa_offset = 0;
	exit_id->exi_memrefc = 0;

	/* Generate the special last exception exit block (the one which includes "return NULL") */
	memequivs_init(&state->ms_memequiv);
	info = function_assembler_except_select_predecessor(self, exit_id);
	ASSERT(info);
	ASSERT(!except_exitinfo_wascompiled(info));
	gen.fg_sect = &info->exi_text;
	if unlikely(fg_xmorph(&gen, except_exitinfo_asid(info), exit_id))
		goto err_equiv;
	if unlikely(fg_gretNULL(&gen))
		goto err_equiv;
	ASSERT(state->ms_stacka == 0);
	ASSERT(state->ms_stackc == 0);
	ASSERT(state->ms_stackv == NULL);
	ASSERT(except_exitinfo_wascompiled(info));
	ASSERT(info->exi_next == NULL);
	memequivs_fini(&state->ms_memequiv);

	/* Compile all of the other exception exit blocks for fallthru */
	while ((next = function_assembler_except_select_predecessor(self, except_exitinfo_asid(info))) != NULL) {
		ASSERT(!except_exitinfo_wascompiled(next));
		gen.fg_sect = &next->exi_text;
		ASSERT(state->ms_stacka == 0);
		ASSERT(state->ms_stackc == 0);
		ASSERT(state->ms_stackv == NULL);
		memequivs_init(&state->ms_memequiv);
		if unlikely(fg_xmorph(&gen, except_exitinfo_asid(next), except_exitinfo_asid(info)))
			goto err_equiv;
		memequivs_fini(&state->ms_memequiv);
		ASSERT(state->ms_stacka == 0);
		ASSERT(state->ms_stackc == 0);
		ASSERT(state->ms_stackv == NULL);
		ASSERT(next->exi_next == NULL);
		next->exi_next = info;
		info = next;
		ASSERT(except_exitinfo_wascompiled(next));
	}
	ASSERT(state->ms_stacka == 0);
	ASSERT(state->ms_stackc == 0);
	ASSERT(state->ms_stackv == NULL);

	/* Remember the "first" exception handler. */
	self->fa_except_first = info;
	return 0;
err_equiv:
	memequivs_fini(&state->ms_memequiv);
	ASSERT(state->ms_stacka == 0);
	ASSERT(state->ms_stackc == 0);
	ASSERT(state->ms_stackv == NULL);
/*err:*/
	return -1;
}





/* Append morphing code for `from_state' to `to_state' to `sect' */
#undef assemble_morph
PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
assemble_morph(struct function_assembler *__restrict assembler,
               struct host_section *sect,
               struct memstate *from_state,
               struct basic_block *to_block
#ifndef NO_HOSTASM_DEBUG_PRINT
               , Dee_instruction_t const *from_instr
#endif /* !NO_HOSTASM_DEBUG_PRINT */
               ) {
#ifdef NO_HOSTASM_DEBUG_PRINT
#define assemble_morph(a, b, c, d, e) assemble_morph(a, b, c, d)
#endif /* NO_HOSTASM_DEBUG_PRINT */
	int result;
	struct fungen gen;
	ASSERT(to_block->bb_mem_start);
	Dee_DPRINTF("fg_vmorph: %.4" PRFx32 " -> %.4" PRFx32 "\n",
	            function_assembler_addrof(assembler, from_instr),
	            to_block->bb_deemon_start < to_block->bb_deemon_end
	            ? function_assembler_addrof(assembler, to_block->bb_deemon_start)
	            : 0xffff);
#ifndef NO_HOSTASM_DEBUG_PRINT
	_memstate_debug_print(from_state, assembler, from_instr);
	_memstate_debug_print(to_block->bb_mem_start, assembler,
	                      to_block->bb_deemon_start < to_block->bb_deemon_end
	                      ? to_block->bb_deemon_start
	                      : NULL);
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	gen.fg_assembler = assembler;
	gen.fg_block     = to_block;
	gen.fg_sect      = sect;
	gen.fg_state     = from_state;
	_fg_initcommon(&gen);
	memstate_incref(gen.fg_state);
	/* Since we just throw the mem-state away anyways, we don't need to do equivalence constraining here. */
	result = fg_vmorph_no_constrain_equivalences(&gen, to_block->bb_mem_start);
	memstate_decref(gen.fg_state);
	return result;
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
function_assembler_compilemorph(struct function_assembler *__restrict self) {
	size_t block_i;
	/* Initial morph after the prolog to match the start-state of the first block. */
	if unlikely(assemble_morph(self, &self->fa_prolog, self->fa_prolog_end,
	                           self->fa_blockv[0],
	                           self->fa_blockv[0]->bb_deemon_start))
		goto err;
	for (block_i = 0; block_i < self->fa_blockc; ++block_i) {
		size_t exit_i;
		struct basic_block *block = self->fa_blockv[block_i];
		ASSERT(block);
		ASSERT(block->bb_mem_start);
		ASSERT(block->bb_mem_end);
		for (exit_i = 0; exit_i < block->bb_exits.jds_size; ++exit_i) {
			struct jump_descriptor *jd = block->bb_exits.jds_list[exit_i];
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
		return function_assembler_compileexcept(self);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) void DCALL
collect_morph_sections(struct host_section_tailq *__restrict text,
                       struct basic_block *__restrict morph_block) {
	size_t exit_i;
	for (exit_i = 0; exit_i < morph_block->bb_exits.jds_size; ++exit_i) {
		struct jump_descriptor *jmp = morph_block->bb_exits.jds_list[exit_i];
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
 *   - self->fa_blockv[*]->bb_exits.jds_list[*]->jd_morph.hs_link+hs_symbols
 *   - self->fa_except_exitv[*]->exi_block->bb_htext.hs_link+hs_symbols
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
function_assembler_ordersections(struct function_assembler *__restrict self) {
#ifdef host_section_unwind_maxsize
#ifdef HAVE_hostfunc_unwind_enabled
	bool unwind_enabled;
#endif /* HAVE_hostfunc_unwind_enabled */
#endif /* host_section_unwind_maxsize */
	struct host_section *sect;
	size_t block_i, morph_flush_i;
#ifdef __INTELLISENSE__
	struct host_section_tailq text;
#else /* __INTELLISENSE__ */
#define text self->fa_sections
#endif /* !__INTELLISENSE__ */
	struct host_section_tailq cold;
	TAILQ_INIT(&text);
	TAILQ_INIT(&cold);
	ASSERT(!host_section_islinked(&self->fa_prolog));
	self->fa_prolog.hs_fallthru = &self->fa_blockv[0]->bb_htext;
	TAILQ_INSERT_HEAD(&text, &self->fa_prolog, hs_link);
	ASSERT(host_section_islinked(&self->fa_prolog));
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
		struct basic_block *jmp_next;
		struct basic_block *block = self->fa_blockv[block_i];
		ASSERT(block);
		jmp_next = block->bb_next;
		block->bb_htext.hs_fallthru = jmp_next ? &jmp_next->bb_htext : NULL;
		TAILQ_INSERT_TAIL(&text, &block->bb_htext, hs_link);
		if (jmp_next == NULL) {
			/* Block doesn't return normally -> this is a good spot to insert morph sections. */
			LOCAL_morph_flush(block_i);
		}
	}
	ASSERT(block_i == self->fa_blockc);
	LOCAL_morph_flush(self->fa_blockc - 1);
#undef LOCAL_morph_flush

	/* Go through exception exits and put them into the section order.
	 * Note that `function_assembler_compilemorph()' already ordered
	 * them for us, so all we need to do is start walking at `fa_except_first' */
	ASSERT((self->fa_except_first != NULL) ==
	       (self->fa_except_exitc > 0));
	if (self->fa_except_first != NULL) {
		struct except_exitinfo *block;
		block = self->fa_except_first;
		do {
			struct except_exitinfo *jmp_next = block->exi_next;
			block->exi_text.hs_fallthru = jmp_next ? &jmp_next->exi_text : NULL;
			TAILQ_INSERT_TAIL(&text, &block->exi_text, hs_link);
		} while ((block = block->exi_next) != NULL);
	}

	/* Collect cold sections. */
	{
		size_t depth;
		for (depth = 0;; ++depth) {
			bool found_any = false;
			/* Search the prolog */
			{
				struct host_section *csect;
				size_t depth_i;
				csect = self->fa_prolog.hs_cold;
				if (!csect)
					goto check_blocks;
				for (depth_i = 0; depth_i < depth; ++depth_i) {
					csect = csect->hs_cold;
					if (!csect)
						goto check_blocks;
				}
				csect->hs_fallthru = NULL;
				TAILQ_INSERT_TAIL(&cold, csect, hs_link);
				found_any = true;
			}
check_blocks:
			for (block_i = 0; block_i < self->fa_blockc; ++block_i) {
				size_t depth_i;
				struct basic_block *block = self->fa_blockv[block_i];
				struct host_section *csect = block->bb_htext.hs_cold;
				if (!csect)
					goto next_cold_block;
				for (depth_i = 0; depth_i < depth; ++depth_i) {
					csect = csect->hs_cold;
					if (!csect)
						goto next_cold_block;
				}
				csect->hs_fallthru = NULL;
				TAILQ_INSERT_TAIL(&cold, csect, hs_link);
				found_any = true;
next_cold_block:;
			}
			for (block_i = 0; block_i < self->fa_except_exitc; ++block_i) {
				size_t depth_i;
				struct except_exitinfo *xinfo = self->fa_except_exitv[block_i];
				struct host_section *csect = xinfo->exi_text.hs_cold;
				if (!csect)
					goto next_cold_xinfo;
				for (depth_i = 0; depth_i < depth; ++depth_i) {
					csect = csect->hs_cold;
					if (!csect)
						goto next_cold_xinfo;
				}
				csect->hs_fallthru = NULL;
				TAILQ_INSERT_TAIL(&cold, csect, hs_link);
				found_any = true;
next_cold_xinfo:;
			}
			if (!found_any)
				break;
		}
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
		struct host_section *sort_next = TAILQ_NEXT(sect, hs_link);
		struct host_section *want_next = sect->hs_fallthru;
		ASSERT(sort_next != sect);
		if (want_next && want_next != sort_next && want_next != sect) {
			struct host_section *want_prev;
			/* Special case: can *always* move an empty section */
			if (host_section_size(sect) == 0)
				goto do_move_section;

			/* See if we can move this section to where it wants to go. */
			want_prev = TAILQ_PREV(want_next, host_section_tailq, hs_link);
			while (want_prev && want_prev->hs_fallthru == want_next &&
			       host_section_size(want_prev) == 0) {
				/* Special case: the other section is empty, so it
				 * doesn't matter if we jump to it, or after it.
				 *
				 * As such, try to jump *to* it so we get another
				 * chance of shifting `sect'. */
				want_next = want_prev;
				want_prev = TAILQ_PREV(want_next, host_section_tailq, hs_link);
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
		struct host_section *want_next = sect->hs_fallthru;
		struct host_section *sort_next = TAILQ_NEXT(sect, hs_link);
		unsigned int n;
		if (want_next == NULL)
			goto no_jmp_needed; /* No specific successor needed */
		if (want_next == sort_next)
			goto no_jmp_needed; /* Expected successor already matched */

		/* If the intended target is a zero-sized section, then the
		 * actual jump goes to wherever that section falls to. */
		want_next = sect->hs_fallthru;
		for (n = 128; n > 0; --n) { /* Limit is needed for infinite loops */
			struct host_section *next;
			if (host_section_size(want_next) != 0)
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
			function_assembler_DEFINE_host_symbol_section(self, err, Lnext_dst, want_next, 0);
#ifdef Dee_MallocUsableSize
			sect->hs_alend = sect->hs_start + Dee_MallocUsableSize(sect->hs_start);
			ASSERT(sect->hs_alend >= sect->hs_end);
#else /* Dee_MallocUsableSize */
			sect->hs_alend = sect->hs_end;
#endif /* !Dee_MallocUsableSize */
			if unlikely(_host_section_gjmp(sect, Lnext_dst))
				goto err;
		}
		/*sect->hs_fallthru = NULL;*/
no_jmp_needed:;
	}

	/* Go through the big ol' list of sections to figure out the total text size.
	 * Note that we can't remove empty sections because they might still be
	 * referenced by host_symbol-s. */
	self->fa_sectsize = 0;
#ifdef host_section_unwind_maxsize
#ifdef HAVE_hostfunc_unwind_enabled
	unwind_enabled = hostfunc_unwind_enabled();
#endif /* HAVE_hostfunc_unwind_enabled */
#endif /* host_section_unwind_maxsize */
	TAILQ_FOREACH (sect, &self->fa_sections, hs_link) {
		self->fa_sectsize += host_section_size(sect);
#ifdef host_section_unwind_maxsize
#ifdef HAVE_hostfunc_unwind_enabled
		if (unwind_enabled)
#endif /* HAVE_hostfunc_unwind_enabled */
		{
			self->fa_sectsize += host_section_unwind_maxsize(sect);
		}
#endif /* host_section_unwind_maxsize */
#ifdef HOSTASM_HAVE_SHRINKJUMPS
		sect->hs_symbols = NULL; /* Needed for symbol ordering */
#endif /* HOSTASM_HAVE_SHRINKJUMPS */
	}

	/* Align the total section size to whole pointers */
	self->fa_sectsize += (HOST_SIZEOF_POINTER - 1);
	self->fa_sectsize &= ~(HOST_SIZEOF_POINTER - 1);

#ifdef HOSTASM_HAVE_SHRINKJUMPS
	/* Sort host text symbols into their proper sections (needed
	 * so that shrinkjumps() can adjust symbols when deleting text) */
	{
		struct host_symbol *misc_symbols = NULL;
		struct host_symbol *sym = self->fa_symbols;
		while (sym) {
			struct host_symbol *next = sym->_hs_next;
			struct host_symbol **p_list;
			/* Must also resolve `HOST_SYMBOL_JUMP' -> `HOST_SYMBOL_SECT' */
			if (sym->hs_type == HOST_SYMBOL_JUMP) {
				struct jump_descriptor *jmp = sym->hs_value.sv_jump;
				struct host_section *target_sect;
				if (host_section_islinked(&jmp->jd_morph)) {
					target_sect = &jmp->jd_morph;
				} else {
					struct basic_block *block;
					block = jmp->jd_to;
					while (host_section_islinked(&block->bb_htext)) {
						ASSERTF(block->bb_next, "symbol points to not-linked block with no successor");
						block = block->bb_next;
					}
					target_sect = &block->bb_htext;
				}
				sym->hs_type = HOST_SYMBOL_SECT;
				sym->hs_value.sv_sect.ss_sect = target_sect;
				sym->hs_value.sv_sect.ss_off  = 0;
			}
			ASSERT(sym->hs_type == HOST_SYMBOL_SECT ||
			       sym->hs_type == HOST_SYMBOL_ABS);
			p_list = &misc_symbols;
			if (sym->hs_type == HOST_SYMBOL_SECT) {
				struct host_section *symsect;
				symsect = sym->hs_value.sv_sect.ss_sect;
				ASSERT(symsect);
				/* The section may not be linked if it was deleted by `function_assembler_trimdead()'.
				 * When that is the case, simply keep the symbol as part of the misc-symbols list. */
				if (host_section_islinked(symsect))
					p_list = &symsect->hs_symbols;
			}
			ASSERT(!*p_list || (*p_list)->hs_type != HOST_SYMBOL_UNDEF);
			ASSERT(!*p_list || (*p_list)->hs_type < HOST_SYMBOL_TCNT);
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
host_section_deltext(struct host_section *__restrict self,
                     uint32_t sectrel_addr, uint32_t num_bytes) {
	struct host_section *sect;
	struct host_symbol *sym;
	size_t i;
	ASSERT((sectrel_addr) < host_section_size(self));
	ASSERT((sectrel_addr + num_bytes) <= host_section_size(self));
	self->hs_end -= num_bytes;
	memmovedown(self->hs_start + sectrel_addr,
	            self->hs_start + sectrel_addr + num_bytes,
	            host_section_size(self) - sectrel_addr);

	/* Adjust offsets of relocations that happen after the deleted area. */
	for (i = 0; i < self->hs_relc; ++i) {
		struct host_reloc *rel = &self->hs_relv[i];
		if (rel->hr_offset >= sectrel_addr)
			rel->hr_offset -= num_bytes;
	}

	/* Adjust addresses of symbols that appear after the deleted area */
	for (sym = self->hs_symbols; sym; sym = sym->_hs_next) {
		ASSERT(sym->hs_type == HOST_SYMBOL_SECT);
		ASSERT(sym->hs_value.sv_sect.ss_sect == self);
		if (sym->hs_value.sv_sect.ss_off >= sectrel_addr)
			sym->hs_value.sv_sect.ss_off -= num_bytes;
	}

	/* All sections that code after `self' need to have their base offset adjusted. */
	for (sect = TAILQ_NEXT(self, hs_link); sect;
	     sect = TAILQ_NEXT(sect, hs_link))
		sect->hs_badr -= num_bytes;

#ifndef CONFIG_host_unwind_USES_NOOP
	host_section_unwind_trimrange(self, sectrel_addr, num_bytes);
#endif /* !CONFIG_host_unwind_USES_NOOP */
}

PRIVATE NONNULL((1, 2)) bool DCALL
host_reloc_shrinkjump(struct host_section *__restrict sect,
                      struct host_reloc *self) {
	byte_t *rel_templ = sect->hs_start + self->hr_offset;
	uintptr_t rel_adr = sect->hs_badr + self->hr_offset;
	uintptr_t rel_val;
	ASSERT(self->hr_offset < host_section_size(sect));
	switch (self->hr_vtype) {
	case DEE_HOST_RELOCVALUE_ABS:
		goto nope; /* Cannot shrink absolute relocation */
	case DEE_HOST_RELOCVALUE_SYM: {
		struct host_symbol *sym = self->hr_value.rv_sym;
		if (sym->hs_type == HOST_SYMBOL_ABS)
			goto nope; /* Cannot shrink absolute relocation */
		rel_val = host_symbol_value(sym);
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
				host_section_deltext(sect, self->hr_offset + 1, 3);
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
				host_section_deltext(sect, self->hr_offset + 1, 4);
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
host_section_shrinkjumps(struct host_section *__restrict self) {
	bool result = false;
	size_t i;
	for (i = 0; i < self->hs_relc; ++i)
		result |= host_reloc_shrinkjump(self, &self->hs_relv[i]);
	return result;
}

/* Step #6: Try to shrink large in generated host text with smaller ones.
 * This is an arch-specific step. On x86 it replaces `jmpl' with `jmp8' (if possible) */
INTERN NONNULL((1)) void DCALL
function_assembler_shrinkjumps(struct function_assembler *__restrict self) {
	struct host_section *sect;
	uintptr_t badr;
	bool did_something;

	/* Figure out the relative base addresses of each section. */
	badr = 0;
	TAILQ_FOREACH (sect, &self->fa_sections, hs_link) {
		sect->hs_badr = badr;
		badr += host_section_size(sect);
	}

	/* Try to shrink host jump instructions. */
	do {
		did_something = false;
		TAILQ_FOREACH (sect, &self->fa_sections, hs_link) {
			did_something |= host_section_shrinkjumps(sect);
		}
	} while (did_something);
}
#endif /* HOSTASM_HAVE_SHRINKJUMPS */


INTERN NONNULL((1)) bool DCALL
host_section_reloc(struct host_section *__restrict self) {
	size_t i;
	for (i = 0; i < self->hs_relc; ++i) {
		struct host_reloc *rel = &self->hs_relv[i];
		byte_t *rel_addr = self->hs_base + rel->hr_offset;
		uintptr_t value = host_reloc_value(rel);
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
function_assembler_output(struct function_assembler *__restrict self,
                          struct hostfunc *__restrict result) {
	byte_t *writer;
	struct host_section *sect;
again:
	if unlikely(host_rawfunc_init(&result->hf_raw, self->fa_sectsize)) {
		if (Dee_CollectMemory(self->fa_sectsize))
			goto again;
		goto err;
	}
	result->hf_raw.rhf_entry.hfe_addr = result->hf_raw._rhf_base;

	/* Figure out the final base addresses of sections. */
	writer = (byte_t *)result->hf_raw._rhf_base;
	TAILQ_FOREACH (sect, &self->fa_sections, hs_link) {
		size_t size = host_section_size(sect);
		sect->hs_base = writer;
		writer += size;
	}

	/* Output sections and resolve relocations. */
	writer = (byte_t *)result->hf_raw._rhf_base;
	TAILQ_FOREACH (sect, &self->fa_sections, hs_link) {
		size_t size = host_section_size(sect);
		ASSERT(writer == sect->hs_base);
		writer = (byte_t *)mempcpy(writer, sect->hs_start, size);
		if (!host_section_reloc(sect)) {
			/* TODO: Error if overflow on x86_64 (to re-compile w/ large memory model) */
			DeeError_NOTIMPLEMENTED();
			goto err_result;
		}
	}

	/* Output unwind information (if enabled). */
#ifndef CONFIG_host_unwind_USES_NOOP
	hostfunc_unwind_init(&result->hf_unwind, self,
	                         (byte_t *)result->hf_raw._rhf_base,
	                         writer);
#endif /* !CONFIG_host_unwind_USES_NOOP */

	/* Make the function executable. */
	if unlikely(host_rawfunc_mkexec(&result->hf_raw)) {
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
	host_rawfunc_fini(&result->hf_raw);
err:
	return -1;
}

/* High-level wrapper function to fully assemble `function' into its host-asm equivalent.
 * @param: cc:    Calling convention of the generated function
 * @param: flags: Set of `FUNCTION_ASSEMBLER_F_*'
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((2, 3)) int DCALL
hostfunc_assemble(DeeFunctionObject *function,
                  DeeCodeObject *code,
                  struct hostfunc *__restrict result,
                  host_cc_t cc, uint16_t flags) {
	struct function_assembler assembler;
	function_assembler_init(&assembler, function, code, cc, flags);

	/* Special case: deemon code that contains user-written deemon assembly
	 *               requires special care to include some extra checks in
	 *               generated host assembly. */
	if unlikely(assembler.fa_code->co_flags & CODE_FASSEMBLY)
		assembler.fa_flags |= FUNCTION_ASSEMBLER_F_SAFE;

#ifdef FUNCTION_ASSEMBLER_F_MCLARGE
	/* TODO: Only enable by default on __PE__. On __ELF__, try to compile and
	 *       link w/o, and only fall-back to enabling this if that fails. */
	assembler.fa_flags |= FUNCTION_ASSEMBLER_F_MCLARGE;
#endif /* FUNCTION_ASSEMBLER_F_MCLARGE */

	/* Go through all the steps of assembling the function. */
	if unlikely(function_assembler_loadblocks(&assembler))
		goto err_assembler;
	if (!(assembler.fa_flags & FUNCTION_ASSEMBLER_F_NOEARLYDEL)) {
		if unlikely(function_assembler_loadlocuse(&assembler))
			goto err_assembler;
	}
	if unlikely(function_assembler_compileblocks(&assembler))
		goto err_assembler;
	if unlikely(function_assembler_trimdead(&assembler))
		goto err_assembler;
	if unlikely(function_assembler_compilemorph(&assembler))
		goto err_assembler;
	if unlikely(function_assembler_ordersections(&assembler))
		goto err_assembler;
#ifdef HOSTASM_HAVE_SHRINKJUMPS
	function_assembler_shrinkjumps(&assembler);
#endif /* HOSTASM_HAVE_SHRINKJUMPS */
	if unlikely(function_assembler_output(&assembler, result))
		goto err_assembler;
	function_assembler_fini(&assembler);
	return 0;
err_assembler:
	function_assembler_fini(&assembler);
/*err:*/
	return -1;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_ASSEMBLER_C */
