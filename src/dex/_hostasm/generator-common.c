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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_COMMON_C
#define GUARD_DEX_HOSTASM_GENERATOR_COMMON_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/tuple.h>
#include <deemon/util/lock.h>

DECL_BEGIN

STATIC_ASSERT(offsetof(struct Dee_memloc, ml_value.v_hreg.r_voff) ==
              offsetof(struct Dee_memloc, ml_value.v_hstack.s_off));

/************************************************************************/
/* COMMON CODE GENERATION FUNCTIONS                                     */
/************************************************************************/

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_arg2reg(struct Dee_function_generator *__restrict self,
                                    uint16_t aid, Dee_host_register_t dst_regno) {
	ptrdiff_t delta = (ptrdiff_t)aid * HOST_SIZEOF_POINTER;
	struct Dee_memstate *state = self->fg_state;
	Dee_host_register_t argv_regno;
	argv_regno = Dee_memstate_hregs_find_usage(state, DEE_HOST_REGUSAGE_ARGV);
	if (argv_regno >= HOST_REGISTER_COUNT) {
		argv_regno = Dee_memstate_hregs_find_unused(state, true);
		if (argv_regno >= HOST_REGISTER_COUNT)
			argv_regno = dst_regno;
		if unlikely(Dee_function_generator_gmov_usage2reg(self, DEE_HOST_REGUSAGE_ARGV, argv_regno))
			goto err;
	}
	if unlikely(Dee_function_generator_gmov_regind2reg(self, argv_regno, delta, dst_regno))
		goto err;
	state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
Dee_function_generator_gmov_regx2loc(struct Dee_function_generator *__restrict self,
                                     Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                     struct Dee_memloc const *__restrict dst_loc) {
	switch (dst_loc->ml_type) {

	case MEMLOC_TYPE_HSTACKIND: {
		uintptr_t cfa_offset = dst_loc->ml_value.v_hstack.s_cfa;
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		ptrdiff_t delta_delta = dst_loc->ml_value.v_hstack.s_off - src_delta;
		if (delta_delta != 0) {
			/* Adjust `src_regno' to have the correct value-delta */
			if unlikely(_Dee_function_generator_gmov_regx2reg(self, src_regno, delta_delta, src_regno))
				goto err;
		}
		return _Dee_function_generator_gmov_reg2hstackind(self, src_regno, sp_offset);
	}	break;

	case MEMLOC_TYPE_HREGIND: {
		ptrdiff_t delta_delta = dst_loc->ml_value.v_hreg.r_voff - src_delta;
		if (delta_delta != 0) {
			if unlikely(_Dee_function_generator_gmov_regx2reg(self, src_regno, delta_delta, src_regno))
				goto err;
		}
		return _Dee_function_generator_gmov_reg2regind(self, src_regno,
		                                               dst_loc->ml_value.v_hreg.r_regno,
		                                               dst_loc->ml_value.v_hreg.r_off);
	}	break;

	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gmov_regx2reg(self, src_regno,
		                                             src_delta - dst_loc->ml_value.v_hreg.r_off,
		                                             dst_loc->ml_value.v_hreg.r_regno);

	default: break;
	}
	return DeeError_Throwf(&DeeError_IllegalInstruction,
	                       "Cannot move register to location type %" PRFu16,
	                       dst_loc->ml_type);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gmov_loc2regx(struct Dee_function_generator *__restrict self,
                                     struct Dee_memloc const *__restrict src_loc,
                                     Dee_host_register_t dst_regno, ptrdiff_t dst_delta) {
	int result;
	switch (src_loc->ml_type) {

	case MEMLOC_TYPE_HSTACKIND: {
		uintptr_t cfa_offset = src_loc->ml_value.v_hstack.s_cfa;
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		ptrdiff_t delta_delta = src_loc->ml_value.v_hstack.s_off - dst_delta;
#ifdef HOSTASM_STACK_GROWS_DOWN
		if (sp_offset == 0)
#else /* HOSTASM_STACK_GROWS_DOWN */
		if (sp_offset == -HOST_SIZEOF_POINTER)
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		{
			/* Special case: if the value lies on-top of the host stack, then pop it instead of move it. */
			result = _Dee_function_generator_ghstack_popreg(self, dst_regno);
			if likely(result == 0)
				Dee_function_generator_gadjust_cfa_offset(self, -HOST_SIZEOF_POINTER);
		} else {
			result = _Dee_function_generator_gmov_hstackind2reg(self, sp_offset, dst_regno);
		}
		if likely(result == 0 && delta_delta != 0)
			result = _Dee_function_generator_gmov_regx2reg(self, dst_regno, delta_delta, dst_regno);
	}	break;

	case MEMLOC_TYPE_HSTACK: {
		uintptr_t cfa_offset = src_loc->ml_value.v_hstack.s_cfa;
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		result = _Dee_function_generator_gmov_hstack2reg(self, sp_offset - dst_delta, dst_regno);
	}	break;

	case MEMLOC_TYPE_HREGIND: {
		ptrdiff_t delta_delta = src_loc->ml_value.v_hreg.r_voff - dst_delta;
		result = _Dee_function_generator_gmov_regind2reg(self,
		                                                 src_loc->ml_value.v_hreg.r_regno,
		                                                 src_loc->ml_value.v_hreg.r_off,
		                                                 dst_regno);
		if likely(result == 0 && delta_delta != 0)
			result = _Dee_function_generator_gmov_regx2reg(self, dst_regno, delta_delta, dst_regno);
	}	break;

	case MEMLOC_TYPE_HREG:
		result = _Dee_function_generator_gmov_regx2reg(self,
		                                               src_loc->ml_value.v_hreg.r_regno,
		                                               src_loc->ml_value.v_hreg.r_off - dst_delta,
		                                               dst_regno);
		break;

	case MEMLOC_TYPE_CONST:
		result = _Dee_function_generator_gmov_const2reg(self,
		                                                (DeeObject *)((uintptr_t)src_loc->ml_value.v_const -
		                                                              dst_delta),
		                                                dst_regno);
		break;

	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot move location type %" PRFu16 " to register",
		                       src_loc->ml_type);
	}
	if likely(result == 0)
		self->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC;
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
Dee_function_generator_gmov_loc2regy(struct Dee_function_generator *__restrict self,
                                     struct Dee_memloc const *__restrict src_loc,
                                     Dee_host_register_t dst_regno,
                                     ptrdiff_t *__restrict p_dst_delta) {
	int result;
	switch (src_loc->ml_type) {

	case MEMLOC_TYPE_HSTACKIND: {
		uintptr_t cfa_offset = src_loc->ml_value.v_hstack.s_cfa;
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
#ifdef HOSTASM_STACK_GROWS_DOWN
		if (sp_offset == 0)
#else /* HOSTASM_STACK_GROWS_DOWN */
		if (sp_offset == -HOST_SIZEOF_POINTER)
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		{
			/* Special case: if the value lies on-top of the host stack, then pop it instead of move it. */
			result = _Dee_function_generator_ghstack_popreg(self, dst_regno);
			if likely(result == 0)
				Dee_function_generator_gadjust_cfa_offset(self, -HOST_SIZEOF_POINTER);
		} else {
			result = _Dee_function_generator_gmov_hstackind2reg(self, sp_offset, dst_regno);
		}
		*p_dst_delta = src_loc->ml_value.v_hstack.s_off;
	}	break;

	case MEMLOC_TYPE_HSTACK: {
		uintptr_t cfa_offset = src_loc->ml_value.v_hstack.s_cfa;
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		result = _Dee_function_generator_gmov_hstack2reg(self, 0, dst_regno);
		*p_dst_delta = sp_offset;
	}	break;

	case MEMLOC_TYPE_HREGIND:
		result = _Dee_function_generator_gmov_regind2reg(self,
		                                                 src_loc->ml_value.v_hreg.r_regno,
		                                                 src_loc->ml_value.v_hreg.r_off,
		                                                 dst_regno);
		*p_dst_delta = src_loc->ml_value.v_hreg.r_voff;
		break;

	case MEMLOC_TYPE_HREG:
		result = _Dee_function_generator_gmov_reg2reg(self, src_loc->ml_value.v_hreg.r_regno, dst_regno);
		*p_dst_delta = src_loc->ml_value.v_hreg.r_off;
		break;

	case MEMLOC_TYPE_CONST:
		result = _Dee_function_generator_gmov_const2reg(self, src_loc->ml_value.v_const, dst_regno);
		*p_dst_delta = 0;
		break;

	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot move location type %" PRFu16 " to register",
		                       src_loc->ml_type);
	}
	if likely(result == 0)
		self->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC;
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gmov_locind2reg(struct Dee_function_generator *__restrict self,
                                       struct Dee_memloc const *__restrict src_loc, ptrdiff_t src_delta,
                                       Dee_host_register_t dst_regno) {
	int result;
	switch (src_loc->ml_type) {

	case MEMLOC_TYPE_HREG:
		result = _Dee_function_generator_gmov_regind2reg(self,
		                                                 src_loc->ml_value.v_hreg.r_regno,
		                                                 src_loc->ml_value.v_hreg.r_off + src_delta,
		                                                 dst_regno);
		break;

	case MEMLOC_TYPE_HSTACK: {
		uintptr_t cfa_offset = src_loc->ml_value.v_hstack.s_cfa;
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		result = _Dee_function_generator_gmov_hstackind2reg(self, sp_offset + src_delta, dst_regno);
	}	break;

	case MEMLOC_TYPE_CONST: {
		DeeObject **value = (DeeObject **)((uintptr_t)src_loc->ml_value.v_const + src_delta);
		result = _Dee_function_generator_gmov_constind2reg(self, value, dst_regno);
	}	break;

	default: {
		ptrdiff_t ind_delta;
		result = Dee_function_generator_gmov_loc2regy(self, src_loc, dst_regno, &ind_delta);
		if likely(result == 0)
			result = _Dee_function_generator_gmov_regind2reg(self, dst_regno, ind_delta + src_delta, dst_regno);
	}	break;

	}
	if likely(result == 0)
		self->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC;
	return result;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
Dee_function_generator_gmov_reg2locind(struct Dee_function_generator *__restrict self,
                                       Dee_host_register_t src_regno,
                                       struct Dee_memloc const *__restrict dst_loc,
                                       ptrdiff_t dst_delta) {
	int result;
	switch (dst_loc->ml_type) {

	case MEMLOC_TYPE_HREG:
		result = _Dee_function_generator_gmov_reg2regind(self, src_regno,
		                                                 dst_loc->ml_value.v_hreg.r_regno,
		                                                 dst_delta);
		break;

	case MEMLOC_TYPE_HSTACK: {
		uintptr_t cfa_offset = dst_loc->ml_value.v_hstack.s_cfa;
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		result = _Dee_function_generator_gmov_reg2hstackind(self, src_regno, sp_offset + dst_delta);
	}	break;

	case MEMLOC_TYPE_CONST: {
		uintptr_t value = (uintptr_t)dst_loc->ml_value.v_const + dst_delta;
		result = _Dee_function_generator_gmov_reg2constind(self, src_regno, (DeeObject **)value);
	}	break;

	default: {
		/* Need to use a temporary register. */
		Dee_host_register_t temp_reg;
		Dee_host_register_t not_these[2];
		ptrdiff_t ind_delta;
		not_these[0] = src_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		temp_reg = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(temp_reg >= HOST_REGISTER_COUNT)
			goto err;
		if unlikely(Dee_function_generator_gmov_loc2regy(self, dst_loc, temp_reg, &ind_delta))
			goto err;
		result = Dee_function_generator_gmov_reg2regind(self, src_regno, temp_reg, ind_delta + dst_delta);
	}	break;

	}
	return result;
err:
	return -1;
}


/* Load special runtime values into `dst_regno' */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_usage2reg(struct Dee_function_generator *__restrict self,
                                      Dee_host_regusage_t usage,
                                      Dee_host_register_t dst_regno) {
	struct Dee_memstate *state = self->fg_state;
	if unlikely(state->ms_rusage[dst_regno] == usage)
		return 0;
	if ((usage == DEE_HOST_REGUSAGE_ARGC || usage == DEE_HOST_REGUSAGE_ARGV) &&
	    (self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE)) {
		/* Special case: need to load via argc-tuple indirection */
		ptrdiff_t args_offset;
		Dee_host_register_t args_regno;
		args_regno = Dee_memstate_hregs_find_usage(state, DEE_HOST_REGUSAGE_ARGS);
		if (args_regno >= HOST_REGISTER_COUNT) {
			args_regno = Dee_memstate_hregs_find_unused(state, true);
			if (args_regno >= HOST_REGISTER_COUNT)
				args_regno = dst_regno;
			if unlikely(_Dee_function_generator_gmov_usage2reg(self, DEE_HOST_REGUSAGE_ARGS, args_regno))
				goto err;
			state->ms_rusage[args_regno] = DEE_HOST_REGUSAGE_ARGV;
		}

		/* Now load the relevant tuple object field. */
		args_offset = offsetof(DeeTupleObject, t_elem);
		if (usage == DEE_HOST_REGUSAGE_ARGC)
			args_offset = offsetof(DeeTupleObject, t_size);
		if unlikely(_Dee_function_generator_gmov_regind2reg(self, args_regno, args_offset, dst_regno))
			goto err;
	} else {
		if unlikely(_Dee_function_generator_gmov_usage2reg(self, usage, dst_regno))
			goto err;
	}
	state->ms_rusage[dst_regno] = usage;
	return 0;
err:
	return -1;
}

/* Generate code to return `loc'. No extra code to decref stack/locals is generated. If you
 * want that extra code to be generated, you need to use `Dee_function_generator_vret()'. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gret(struct Dee_function_generator *__restrict self,
                            /*inherit_ref*/ struct Dee_memloc *__restrict loc) {
	ptrdiff_t alloc_delta;
	int result;

	/* Move the return value into its proper register. */
	result = Dee_function_generator_gmov_loc2reg(self, loc, HOST_REGISTER_RETURN);
	if unlikely(result != 0)
		goto done;

	/* Release any remaining stack memory. */
	alloc_delta = self->fg_state->ms_host_cfa_offset;
	alloc_delta += Dee_function_assembler_get_cfa_addend(self->fg_assembler);
	if (alloc_delta != 0) {
		result = _Dee_function_generator_ghstack_adjust(self, -alloc_delta);
		if unlikely(result != 0)
			goto done;
	}

	/* Generate the arch-specific return instruction sequence. */
	result = _Dee_function_generator_gret(self);
done:
	return result;
}

/* Push/move `regno' onto the host stack, returning the CFA offset of the target location. */
PRIVATE WUNUSED NONNULL((1)) uintptr_t DCALL
Dee_function_generator_gflushreg(struct Dee_function_generator *__restrict self,
                                 Dee_host_register_t regno) {
	uintptr_t cfa_offset;
	ASSERT(!Dee_memstate_isshared(self->fg_state));
	cfa_offset = Dee_memstate_hstack_find(self->fg_state, HOST_SIZEOF_POINTER);
	if (cfa_offset != (uintptr_t)-1) {
		if unlikely(Dee_function_generator_gmov_reg2hstackind(self, regno, cfa_offset))
			goto err;
	} else {
		/* Allocate more stack space. */
		cfa_offset = Dee_memstate_hstack_alloca(self->fg_state, HOST_SIZEOF_POINTER);
		if unlikely(_Dee_function_generator_ghstack_pushreg(self, regno))
			goto err;
	}
	return cfa_offset;
err:
	return (uintptr_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gflushregind(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc *flush_loc) {
	size_t i;
	struct Dee_memstate *state = self->fg_state;
	Dee_host_register_t regno = flush_loc->ml_value.v_hreg.r_regno;
	ptrdiff_t off = flush_loc->ml_value.v_hreg.r_off;
	uintptr_t cfa_offset;
	ASSERT(flush_loc->ml_type == MEMLOC_TYPE_HREGIND);
	cfa_offset = Dee_memstate_hstack_find(self->fg_state, HOST_SIZEOF_POINTER);
	if (cfa_offset != (uintptr_t)-1) {
		bool did_save_temp_regno = false;
		Dee_host_register_t temp_regno;
		temp_regno = Dee_memstate_hregs_find_unused(state, true);
		if (temp_regno >= HOST_REGISTER_COUNT) {
			temp_regno = flush_loc->ml_value.v_hreg.r_regno;
			did_save_temp_regno = true;
			if unlikely(Dee_function_generator_ghstack_pushreg(self, temp_regno))
				goto err;
		}
		if unlikely(Dee_function_generator_gmov_regind2reg(self, regno, off, temp_regno))
			goto err;
		if unlikely(Dee_function_generator_gmov_reg2hstackind(self, temp_regno, cfa_offset))
			goto err;
		if (did_save_temp_regno) {
			if unlikely(Dee_function_generator_ghstack_popreg(self, temp_regno))
				goto err;
		}
	} else {
		cfa_offset = Dee_memstate_hstack_alloca(self->fg_state, HOST_SIZEOF_POINTER);
		if unlikely(_Dee_function_generator_ghstack_pushregind(self, regno, off))
			goto err;
	}

	/* Convert all locations that use `MEMLOC_TYPE_HREGIND:regno:off' to `MEMLOC_TYPE_HSTACKIND' */
	for (i = 0; i < state->ms_localc; ++i) {
		struct Dee_memloc *loc = &state->ms_localv[i];
		if (loc->ml_type == MEMLOC_TYPE_HREGIND &&
		    loc->ml_value.v_hreg.r_regno == regno &&
		    loc->ml_value.v_hreg.r_off == off) {
			loc->ml_type = MEMLOC_TYPE_HSTACKIND;
			loc->ml_value.v_hstack.s_off = loc->ml_value.v_hreg.r_voff;
			loc->ml_value.v_hstack.s_cfa = cfa_offset;
			Dee_memstate_decrinuse(state, regno);
		}
	}
	for (i = 0; i < state->ms_stackc; ++i) {
		struct Dee_memloc *loc = &state->ms_stackv[i];
		if (loc->ml_type == MEMLOC_TYPE_HREGIND &&
		    loc->ml_value.v_hreg.r_regno == regno &&
		    loc->ml_value.v_hreg.r_off == off) {
			loc->ml_type = MEMLOC_TYPE_HSTACKIND;
			loc->ml_value.v_hstack.s_off = loc->ml_value.v_hreg.r_voff;
			loc->ml_value.v_hstack.s_cfa = cfa_offset;
			Dee_memstate_decrinuse(state, regno);
		}
	}

	ASSERT(flush_loc->ml_type == MEMLOC_TYPE_HSTACKIND);
	return 0;
err:
	return -1;
}

/* Generate code to flush all registers used by the deemon stack/locals into the host stack.
 * NOTE: Usage-registers are cleared by arch-specific code (e.g. `_Dee_function_generator_gcallapi()')
 * @param: ignore_top_n_stack_if_not_ref: From the top-most N stack locations, ignore any
 *                                        that don't contain object references. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gflushregs(struct Dee_function_generator *__restrict self,
                                  uint16_t ignore_top_n_stack_if_not_ref) {
	size_t i;
	uintptr_t register_cfa[HOST_REGISTER_COUNT];
	struct Dee_memstate *state = self->fg_state;
	ASSERT(!Dee_memstate_isshared(state));

	/* Figure out which registers are in use, and assign them CFA offsets. */
	for (i = 0; i < HOST_REGISTER_COUNT; ++i)
		register_cfa[i] = (uintptr_t)-1;
	for (i = 0; i < state->ms_localc; ++i) {
		struct Dee_memloc *loc = &state->ms_localv[i];
		if (loc->ml_type == MEMLOC_TYPE_HREG) {
			Dee_host_register_t regno = loc->ml_value.v_hreg.r_regno;
			ASSERT(regno < HOST_REGISTER_COUNT);
			if (register_cfa[regno] == (uintptr_t)-1) {
				register_cfa[regno] = Dee_function_generator_gflushreg(self, regno);
				if unlikely(register_cfa[regno] == (uintptr_t)-1)
					goto err;
			}
			loc->ml_type = MEMLOC_TYPE_HSTACKIND;
			loc->ml_value.v_hstack.s_off = loc->ml_value.v_hreg.r_off;
			loc->ml_value.v_hstack.s_cfa = register_cfa[regno];
			Dee_memstate_decrinuse(state, regno);
		} else if (loc->ml_type == MEMLOC_TYPE_HREGIND) {
			if unlikely(Dee_function_generator_gflushregind(self, loc))
				goto err;
			ASSERT(loc->ml_type == MEMLOC_TYPE_HSTACKIND);
		}
	}
	for (i = 0; i < state->ms_stackc; ++i) {
		struct Dee_memloc *loc = &state->ms_stackv[i];
		if ((i >= (uint16_t)(state->ms_stackc - ignore_top_n_stack_if_not_ref)) &&
		    (loc->ml_flags & MEMLOC_F_NOREF))
			continue; /* Slot contains no reference and is in top-most n of stack. */
		if (loc->ml_type == MEMLOC_TYPE_HREG) {
			Dee_host_register_t regno;
			regno = loc->ml_value.v_hreg.r_regno;
			ASSERT(regno < HOST_REGISTER_COUNT);
			if (register_cfa[regno] == (uintptr_t)-1) {
				register_cfa[regno] = Dee_function_generator_gflushreg(self, regno);
				if unlikely(register_cfa[regno] == (uintptr_t)-1)
					goto err;
			}
			loc->ml_type = MEMLOC_TYPE_HSTACKIND;
			loc->ml_value.v_hstack.s_off = loc->ml_value.v_hreg.r_off;
			loc->ml_value.v_hstack.s_cfa = register_cfa[regno];
			Dee_memstate_decrinuse(state, regno);
		} else if (loc->ml_type == MEMLOC_TYPE_HREGIND) {
			if unlikely(Dee_function_generator_gflushregind(self, loc))
				goto err;
			ASSERT(loc->ml_type == MEMLOC_TYPE_HSTACKIND);
		}
	}

	/* NOTE: Usage-registers must be cleared by the caller! */
	return 0;
err:
	return -1;
}

PRIVATE ATTR_PURE WUNUSED bool DCALL
nullable_host_register_list_contains(Dee_host_register_t const *list,
                                     Dee_host_register_t regno) {
	Dee_host_register_t item;
	if (list != NULL) {
		while ((item = *list++) < HOST_REGISTER_COUNT) {
			if (item == regno)
				return true;
		}
	}
	return false;
}

/* Allocate at host register, possibly flushing an already used register to stack.
 * @param: not_these: Array of registers not to allocated, terminated by one `>= HOST_REGISTER_COUNT'.
 * @return: * : The allocated register
 * @return: >= HOST_REGISTER_COUNT: Error */
INTERN WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_function_generator_gallocreg(struct Dee_function_generator *__restrict self,
                                 Dee_host_register_t const *not_these) {
	size_t i;
	struct Dee_memstate *state;
	Dee_host_register_t result;
	result = Dee_function_generator_gtryallocreg(self, not_these);
	if (result < HOST_REGISTER_COUNT)
		goto done;

	/* Find something to write to the stack.
	 * For this purpose, try to flush local variables first. */
	state = self->fg_state;
	for (i = 0; i < state->ms_localc; ++i) {
		uintptr_t cfa_offset;
		struct Dee_memloc *loc = &state->ms_localv[i];
		if (!MEMLOC_TYPE_HASREG(loc->ml_type))
			continue;
		result = loc->ml_value.v_hreg.r_regno;
		if (nullable_host_register_list_contains(not_these, result))
			continue;

		/* Flush this one! */
		cfa_offset = Dee_function_generator_gflushreg(self, result);
		if unlikely(cfa_offset == (uintptr_t)-1)
			goto err;
		loc->ml_type = MEMLOC_TYPE_HSTACKIND;
		loc->ml_value.v_hstack.s_cfa = cfa_offset;
		loc->ml_value.v_hstack.s_off = 0;
		goto done;
	}

	/* ... then check for stack slots (starting with older ones first) */
	state = self->fg_state;
	for (i = 0; i < state->ms_stackc; ++i) {
		uintptr_t cfa_offset;
		struct Dee_memloc *loc = &state->ms_stackv[i];
		if (loc->ml_type != MEMLOC_TYPE_HREG)
			continue;
		result = loc->ml_value.v_hreg.r_regno;
		if (nullable_host_register_list_contains(not_these, result))
			continue;

		/* Flush this one! */
		cfa_offset = Dee_function_generator_gflushreg(self, result);
		if unlikely(cfa_offset == (uintptr_t)-1)
			goto err;
		loc->ml_type = MEMLOC_TYPE_HSTACKIND;
		loc->ml_value.v_hstack.s_cfa = cfa_offset;
		loc->ml_value.v_hstack.s_off = 0;
		goto done;
	}

	/* Impossible to allocate register. */
	DeeError_Throwf(&DeeError_IllegalInstruction,
	                "No way to allocate register");
err:
	return HOST_REGISTER_COUNT;
done:
	return result;
}


/* Helper that returns a register that's been populated for `usage' */
INTERN WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_function_generator_gusagereg(struct Dee_function_generator *__restrict self,
                                 Dee_host_regusage_t usage,
                                 Dee_host_register_t const *dont_alloc_these) {
	struct Dee_memstate *state = self->fg_state;
	Dee_host_register_t regno;
	regno = Dee_memstate_hregs_find_usage(state, usage);
	if (regno >= HOST_REGISTER_COUNT) {
		/* Need to allocate a register for `usage'. */
		regno = Dee_function_generator_gallocreg(self, dont_alloc_these);
		if unlikely(regno >= HOST_REGISTER_COUNT)
			goto err;
		if unlikely(Dee_function_generator_gmov_usage2reg(self, usage, regno))
			goto err;
	}
	return regno;
err:
	return HOST_REGISTER_COUNT;
}





/* Generate code to assert that location `loc' is non-NULL:
 * >> Dee_function_generator_gassert_bound(self, loc, ASM_LOCAL, lid, <ignored>, NULL);
 * >> Dee_function_generator_gassert_bound(self, loc, ASM_GLOBAL, gid, <ignored>, NULL);
 * >> Dee_function_generator_gassert_bound(self, loc, ASM_EXTERN, mid, gid, NULL);
 * The `kind', `id1' and `id2' arguments simply select `Dee_function_generator_gthrow_*_unbound()'
 * @param: opt_endread_before_throw: When non-NULL, emit `Dee_function_generator_grwlock_endread()'
 *                                   before the `Dee_function_generator_gthrow_*_unbound' code. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gassert_bound(struct Dee_function_generator *__restrict self,
                                     struct Dee_memloc *loc, uint8_t kind,
                                     uint16_t id1, uint16_t id2,
                                     Dee_atomic_rwlock_t *opt_endread_before_throw) {
	/* TODO: Implement portably once arch-specific jumps and labels can be created */
	(void)self;
	(void)loc;
	(void)kind;
	(void)id1;
	(void)id2;
	(void)opt_endread_before_throw;
	return DeeError_NOTIMPLEMENTED();
}

/* Generate code to throw an error indicating that local variable `lid'
 * is unbound. This includes any necessary jump for the purpose of entering
 * an exception handler. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gthrow_arg_unbound(struct Dee_function_generator *__restrict self, uint16_t aid) {
	/* TODO */
	(void)self;
	(void)aid;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gthrow_local_unbound(struct Dee_function_generator *__restrict self, uint16_t lid) {
	/* TODO */
	(void)self;
	(void)lid;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gthrow_global_unbound(struct Dee_function_generator *__restrict self, uint16_t gid) {
	/* TODO */
	(void)self;
	(void)gid;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gthrow_extern_unbound(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid) {
	/* TODO */
	(void)self;
	(void)mid;
	(void)gid;
	return DeeError_NOTIMPLEMENTED();
}


/* Generate checks to enter exception handling mode. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gjz_except(struct Dee_function_generator *__restrict self,
                                  struct Dee_memloc *loc) {
	struct Dee_basic_block *bb;
	struct Dee_host_symbol sym;
	bb = Dee_function_generator_except_exit(self);
	if unlikely(!bb)
		goto err;
	sym.hs_type = DEE_HOST_SYMBOL_SECT;
	sym.hs_value.sv_sect.ss_sect = &bb->bb_htext;
	sym.hs_value.sv_sect.ss_off  = 0;
	return _Dee_function_generator_gjz(self, loc, &sym);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gjnz_except(struct Dee_function_generator *__restrict self,
                                   struct Dee_memloc *loc) {
	struct Dee_basic_block *bb;
	struct Dee_host_symbol sym;
	bb = Dee_function_generator_except_exit(self);
	if unlikely(!bb)
		goto err;
	sym.hs_type = DEE_HOST_SYMBOL_SECT;
	sym.hs_value.sv_sect.ss_sect = &bb->bb_htext;
	sym.hs_value.sv_sect.ss_off  = 0;
	return _Dee_function_generator_gjnz(self, loc, &sym);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjmp_except(struct Dee_function_generator *__restrict self) {
	struct Dee_basic_block *bb;
	struct Dee_host_symbol sym;
	bb = Dee_function_generator_except_exit(self);
	if unlikely(!bb)
		goto err;
	sym.hs_type = DEE_HOST_SYMBOL_SECT;
	sym.hs_value.sv_sect.ss_sect = &bb->bb_htext;
	sym.hs_value.sv_sect.ss_off  = 0;
	return _Dee_function_generator_gjmp(self, &sym);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gincref(struct Dee_function_generator *__restrict self,
                               struct Dee_memloc *__restrict loc) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gincref_regx(self,
		                                            loc->ml_value.v_hreg.r_regno,
		                                            loc->ml_value.v_hreg.r_off);
	case MEMLOC_TYPE_CONST:
		return _Dee_function_generator_gincref_const(self, loc->ml_value.v_const);
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdecref(struct Dee_function_generator *__restrict self,
                               struct Dee_memloc *__restrict loc) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gdecref_regx(self,
		                                            loc->ml_value.v_hreg.r_regno,
		                                            loc->ml_value.v_hreg.r_off);
	case MEMLOC_TYPE_CONST:
		return _Dee_function_generator_gdecref_const(self, loc->ml_value.v_const);
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxincref(struct Dee_function_generator *__restrict self,
                                struct Dee_memloc *__restrict loc) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gxincref_regx(self,
		                                             loc->ml_value.v_hreg.r_regno,
		                                             loc->ml_value.v_hreg.r_off);
	case MEMLOC_TYPE_CONST:
		if (!loc->ml_value.v_const)
			return 0;
		return _Dee_function_generator_gincref_const(self, loc->ml_value.v_const);
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxdecref(struct Dee_function_generator *__restrict self,
                                struct Dee_memloc *__restrict loc) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gxdecref_regx(self,
		                                             loc->ml_value.v_hreg.r_regno,
		                                             loc->ml_value.v_hreg.r_off);
	case MEMLOC_TYPE_CONST:
		if (!loc->ml_value.v_const)
			return 0;
		return _Dee_function_generator_gdecref_const(self, loc->ml_value.v_const);
	}
	__builtin_unreachable();
err:
	return -1;
}

/* Force `loc' to become a register (`MEMLOC_TYPE_HREG'). */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_greg(struct Dee_function_generator *__restrict self,
                            struct Dee_memloc *__restrict loc,
                            Dee_host_register_t const *not_these) {
	struct Dee_memstate *state;
	Dee_host_register_t regno;
	ptrdiff_t reg_delta;
	bool is_in_state;
	if (loc->ml_type == MEMLOC_TYPE_HREG)
		return 0; /* Already in a register! */

	/* Allocate a register. */
	regno = Dee_function_generator_gallocreg(self, not_these);
	if unlikely(regno >= HOST_REGISTER_COUNT)
		goto err;

	/* Move value into register. */
	if unlikely(Dee_function_generator_gmov_loc2regy(self, loc, regno, &reg_delta))
		goto err;

	state = self->fg_state;
	is_in_state = (loc >= state->ms_localv && loc < state->ms_localv + state->ms_localc) ||
	              (loc >= state->ms_localv && loc < state->ms_localv + state->ms_localc);
	if (is_in_state && MEMLOC_TYPE_HASREG(loc->ml_type))
		Dee_memstate_decrinuse(self->fg_state, loc->ml_value.v_hreg.r_regno);

	/* Remember that `loc' now lies in a register. */
	loc->ml_type = MEMLOC_TYPE_HREG;
	loc->ml_value.v_hreg.r_regno = regno;
	loc->ml_value.v_hreg.r_off   = reg_delta;
	if (is_in_state)
		Dee_memstate_incrinuse(self->fg_state, regno);
	return 0;
err:
	return -1;
}

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_COMMON_C */
