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

/************************************************************************/
/* COMMON CODE GENERATION FUNCTIONS                                     */
/************************************************************************/

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_adjust(struct Dee_function_generator *__restrict self,
                                      ptrdiff_t cfa_delta) {
	int result;
	if unlikely(cfa_delta == 0)
		return 0;
#ifdef HOSTASM_STACK_GROWS_DOWN
	result = _Dee_function_generator_ghstack_adjust(self, -cfa_delta);
#else /* HOSTASM_STACK_GROWS_DOWN */
	result = _Dee_function_generator_ghstack_adjust(self, cfa_delta);
#endif /* !HOSTASM_STACK_GROWS_DOWN */
	if unlikely(result != 0)
		goto done;
	result = Dee_function_generator_state_unshare(self);
	if unlikely(result != 0)
		goto done;
	Dee_function_generator_gadjust_cfa_offset(self, cfa_delta);
done:
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_pushreg(struct Dee_function_generator *__restrict self,
                                       Dee_host_register_t src_regno) {
	int result = _Dee_function_generator_ghstack_pushreg(self, src_regno);
	if unlikely(result != 0)
		goto done;
	result = Dee_function_generator_state_unshare(self);
	if unlikely(result != 0)
		goto done;
	Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
done:
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_pushconst(struct Dee_function_generator *__restrict self,
                                         DeeObject *value) {
	int result = _Dee_function_generator_ghstack_pushconst(self, value);
	if unlikely(result != 0)
		goto done;
	result = Dee_function_generator_state_unshare(self);
	if unlikely(result != 0)
		goto done;
	Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
done:
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_pushhstack(struct Dee_function_generator *__restrict self,
                                          uintptr_t cfa_offset) {
	int result;
	ptrdiff_t sp_offset;
	sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	result = _Dee_function_generator_ghstack_pushhstack(self, sp_offset);
	if unlikely(result != 0)
		goto done;
	result = Dee_function_generator_state_unshare(self);
	if unlikely(result != 0)
		goto done;
	Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
done:
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_arg2reg(struct Dee_function_generator *__restrict self,
                                    uint16_t aid, Dee_host_register_t dst_regno) {
	ptrdiff_t delta = (ptrdiff_t)aid * HOST_SIZEOF_POINTER;
	struct Dee_memstate *state = self->fg_state;
	Dee_host_register_t argv_regno;
	argv_regno = Dee_memstate_hregs_find_usage(state, REGISTER_USAGE_ARGV);
	if (argv_regno >= HOST_REGISTER_COUNT) {
		argv_regno = Dee_memstate_hregs_find_unused(state, true);
		if (argv_regno >= HOST_REGISTER_COUNT)
			argv_regno = dst_regno;
		if unlikely(Dee_function_generator_gmov_usage2reg(self, REGISTER_USAGE_ARGV, argv_regno))
			goto err;
	}
	if unlikely(Dee_function_generator_gmov_regind2reg(self, argv_regno, delta, dst_regno))
		goto err;
	state->ms_regs[dst_regno] = REGISTER_USAGE_GENERIC;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
Dee_basic_block_gmov_reg2loc(struct Dee_basic_block *__restrict self,
                             Dee_host_register_t src_regno,
                             struct Dee_memloc const *__restrict dst_loc) {
	switch (dst_loc->ml_where) {
	case MEMLOC_TYPE_HSTACK:
		return _Dee_basic_block_gmov_reg2hstack(self, src_regno, dst_loc->ml_value.ml_hstack);
	case MEMLOC_TYPE_HREG:
		return _Dee_basic_block_gmov_reg2reg(self, src_regno, dst_loc->ml_value.ml_hreg);
	default: break;
	}
	return DeeError_Throwf(&DeeError_IllegalInstruction,
	                       "Cannot move register to location type %" PRFu16,
	                       dst_loc->ml_where);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gmov_loc2reg(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc const *__restrict src_loc,
                                    Dee_host_register_t dst_regno) {
	int result;
	switch (src_loc->ml_where) {
	case MEMLOC_TYPE_HSTACK: {
		uintptr_t cfa_offset = src_loc->ml_value.ml_hstack;
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
			result = _Dee_function_generator_gmov_hstack2reg(self, sp_offset, dst_regno);
		}
	}	break;
	case MEMLOC_TYPE_HREG:
		result = _Dee_function_generator_gmov_reg2reg(self, src_loc->ml_value.ml_hreg, dst_regno);
		break;
	case MEMLOC_TYPE_ARG:
		result = Dee_function_generator_gmov_arg2reg(self, src_loc->ml_value.ml_harg, dst_regno);
		break;
	case MEMLOC_TYPE_CONST:
		result = _Dee_function_generator_gmov_const2reg(self, src_loc->ml_value.ml_const, dst_regno);
		break;
	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot move location type %" PRFu16 " to register",
		                       src_loc->ml_where);
	}
	if likely(result == 0)
		self->fg_state->ms_regs[dst_regno] = REGISTER_USAGE_GENERIC;
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gmov_locind2reg(struct Dee_function_generator *__restrict self,
                                       struct Dee_memloc const *__restrict src_loc,
                                       ptrdiff_t src_delta, Dee_host_register_t dst_regno) {
	int result;
	switch (src_loc->ml_where) {
	case MEMLOC_TYPE_HSTACK:
		result = _Dee_function_generator_gmov_hstack2reg(self, src_loc->ml_value.ml_hstack, dst_regno);
		if likely(result == 0)
			result = _Dee_function_generator_gmov_regind2reg(self, dst_regno, src_delta, dst_regno);
		break;
	case MEMLOC_TYPE_HREG:
		result = _Dee_function_generator_gmov_regind2reg(self, src_loc->ml_value.ml_hreg, src_delta, dst_regno);
		break;
	case MEMLOC_TYPE_ARG:
		result = Dee_function_generator_gmov_arg2reg(self, src_loc->ml_value.ml_harg, dst_regno);
		if likely(result == 0)
			result = _Dee_function_generator_gmov_regind2reg(self, dst_regno, src_delta, dst_regno);
		break;
	case MEMLOC_TYPE_CONST: {
		uintptr_t value = (uintptr_t)src_loc->ml_value.ml_const + src_delta;
		result = _Dee_function_generator_gmov_const2reg(self, (DeeObject *)value, dst_regno);
	}	break;
	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot move location type %" PRFu16 " to register",
		                       src_loc->ml_where);
	}
	if likely(result == 0)
		self->fg_state->ms_regs[dst_regno] = REGISTER_USAGE_GENERIC;
	return result;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
Dee_function_generator_gmov_reg2locind(struct Dee_function_generator *__restrict self,
                                       Dee_host_register_t src_regno,
                                       struct Dee_memloc const *__restrict dst_loc,
                                       ptrdiff_t dst_delta) {
	int result;
	switch (dst_loc->ml_where) {

	case MEMLOC_TYPE_HREG:
		result = _Dee_function_generator_gmov_reg2regind(self, src_regno, dst_loc->ml_value.ml_hreg, dst_delta);
		break;

	case MEMLOC_TYPE_CONST: {
		uintptr_t value = (uintptr_t)dst_loc->ml_value.ml_const + dst_delta;
		result = _Dee_function_generator_gmov_reg2constind(self, src_regno, (DeeObject **)value);
	}	break;

	default: {
		/* Need to use a temporary register. */
		Dee_host_register_t temp_reg;
		Dee_host_register_t not_these[2];
		not_these[0] = src_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		temp_reg = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(temp_reg >= HOST_REGISTER_COUNT)
			goto err;
		if unlikely(Dee_function_generator_gmov_loc2reg(self, dst_loc, temp_reg))
			goto err;
		result = Dee_function_generator_gmov_reg2regind(self, src_regno, temp_reg, dst_delta);
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
	if unlikely(state->ms_regs[dst_regno] == usage)
		return 0;
	if ((usage == REGISTER_USAGE_ARGC || usage == REGISTER_USAGE_ARGV) &&
	    (self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE)) {
		/* Special case: need to load via argc-tuple indirection */
		ptrdiff_t args_offset;
		Dee_host_register_t args_regno;
		args_regno = Dee_memstate_hregs_find_usage(state, REGISTER_USAGE_ARGS);
		if (args_regno >= HOST_REGISTER_COUNT) {
			args_regno = Dee_memstate_hregs_find_unused(state, true);
			if (args_regno >= HOST_REGISTER_COUNT)
				args_regno = dst_regno;
			if unlikely(_Dee_function_generator_gmov_usage2reg(self, REGISTER_USAGE_ARGS, args_regno))
				goto err;
			state->ms_regs[args_regno] = REGISTER_USAGE_ARGV;
		}

		/* Now load the relevant tuple object field. */
		args_offset = offsetof(DeeTupleObject, t_elem);
		if (usage == REGISTER_USAGE_ARGC)
			args_offset = offsetof(DeeTupleObject, t_size);
		if unlikely(_Dee_function_generator_gmov_regind2reg(self, args_regno, args_offset, dst_regno))
			goto err;
	} else {
		if unlikely(_Dee_function_generator_gmov_usage2reg(self, usage, dst_regno))
			goto err;
	}
	state->ms_regs[dst_regno] = usage;
	return 0;
err:
	return -1;
}

/* Generate code to return `loc'. No extra code to decref stack/locals is generated. If you
 * want that extra code to be generated, you need to use `Dee_function_generator_vret()'. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gret(struct Dee_function_generator *__restrict self,
                            /*inherit_ref*/ struct Dee_memloc *__restrict loc) {
	ptrdiff_t sp_delta;
	int result;

	/* Move the return value into its proper register. */
	result = Dee_function_generator_gmov_loc2reg(self, loc, HOST_REGISTER_RETURN);
	if unlikely(result != 0)
		goto done;

	/* Release any remaining stack memory. */
	sp_delta = self->fg_state->ms_host_cfa_offset;
	sp_delta += Dee_function_assembler_get_cfa_addend(self->fg_assembler);
#ifdef HOSTASM_STACK_GROWS_DOWN
	sp_delta = -sp_delta;
#endif /* HOSTASM_STACK_GROWS_DOWN */
	if (sp_delta != 0) {
		result = _Dee_function_generator_ghstack_adjust(self, sp_delta);
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
		if unlikely(Dee_function_generator_gmov_reg2hstack(self, regno, cfa_offset))
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

/* Generate code to flush all registers used by the deemon stack/locals into the host stack.
 * NOTE: Usage-registers are cleared by arch-specific code (e.g. `_Dee_function_generator_gcall_c_function()')
 * @param: ignore_top_n_stack_if_not_ref: From the top-most N stack locations, ignore any
 *                                        that don't contain object references. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gflushregs(struct Dee_function_generator *__restrict self,
                                  uint16_t ignore_top_n_stack_if_not_ref) {
	uint16_t i;
	uintptr_t register_cfa[HOST_REGISTER_COUNT];
	struct Dee_memstate *state = self->fg_state;
	ASSERT(!Dee_memstate_isshared(state));

	/* Figure out which registers are in use, and assign them CFA offsets. */
	for (i = 0; i < HOST_REGISTER_COUNT; ++i)
		register_cfa[i] = (uintptr_t)-1;
	for (i = 0; i < state->ms_localc; ++i) {
		if (state->ms_localv[i].ml_where == MEMLOC_TYPE_HREG) {
			Dee_host_register_t regno;
			regno = state->ms_localv[i].ml_value.ml_hreg;
			ASSERT(regno < HOST_REGISTER_COUNT);
			if (register_cfa[regno] == (uintptr_t)-1) {
				register_cfa[regno] = Dee_function_generator_gflushreg(self, regno);
				if unlikely(register_cfa[regno] == (uintptr_t)-1)
					goto err;
			}
			state->ms_localv[i].ml_where = MEMLOC_TYPE_HSTACK;
			state->ms_localv[i].ml_value.ml_hstack = register_cfa[regno];
		}
	}
	for (i = 0; i < state->ms_stackc; ++i) {
		if ((i >= (state->ms_stackc - ignore_top_n_stack_if_not_ref)) &&
		    (state->ms_stackv[i].ml_flags & MEMLOC_F_NOREF))
			continue; /* Slot contains no reference and is in top-most n of stack. */
		if (state->ms_stackv[i].ml_where == MEMLOC_TYPE_HREG) {
			Dee_host_register_t regno;
			regno = state->ms_stackv[i].ml_value.ml_hreg;
			ASSERT(regno < HOST_REGISTER_COUNT);
			if (register_cfa[regno] == (uintptr_t)-1) {
				register_cfa[regno] = Dee_function_generator_gflushreg(self, regno);
				if unlikely(register_cfa[regno] == (uintptr_t)-1)
					goto err;
			}
			state->ms_stackv[i].ml_where = MEMLOC_TYPE_HSTACK;
			state->ms_stackv[i].ml_value.ml_hstack = register_cfa[regno];
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

/* Allocate at host register, possibly flushing other an already used register to stack.
 * @param: not_these: Array of registers not to allocated, terminated by one `>= HOST_REGISTER_COUNT'.
 * @return: * : The allocated register
 * @return: >= HOST_REGISTER_COUNT: Error */
INTERN WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_function_generator_gallocreg(struct Dee_function_generator *__restrict self,
                                 Dee_host_register_t const *not_these) {
	uint16_t i;
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
		if (loc->ml_where != MEMLOC_TYPE_HREG)
			continue;
		result = loc->ml_value.ml_hreg;
		if (nullable_host_register_list_contains(not_these, result))
			continue;

		/* Flush this one! */
		cfa_offset = Dee_function_generator_gflushreg(self, result);
		if unlikely(cfa_offset == (uintptr_t)-1)
			goto err;
		loc->ml_where = MEMLOC_TYPE_HSTACK;
		loc->ml_value.ml_hstack = cfa_offset;
		goto done;
	}

	/* ... then check for stack slots (starting with older ones first) */
	state = self->fg_state;
	for (i = 0; i < state->ms_stackc; ++i) {
		uintptr_t cfa_offset;
		struct Dee_memloc *loc = &state->ms_stackv[i];
		if (loc->ml_where != MEMLOC_TYPE_HREG)
			continue;
		result = loc->ml_value.ml_hreg;
		if (nullable_host_register_list_contains(not_these, result))
			continue;

		/* Flush this one! */
		cfa_offset = Dee_function_generator_gflushreg(self, result);
		if unlikely(cfa_offset == (uintptr_t)-1)
			goto err;
		loc->ml_where = MEMLOC_TYPE_HSTACK;
		loc->ml_value.ml_hstack = cfa_offset;
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
Dee_function_generator_gthrow_local_unbound(struct Dee_function_generator *__restrict self, uint16_t lid) {
	/* TODO: Implement using `_Dee_function_generator_gcall_c_function()'+`Dee_function_generator_gjmp_except()' */
	(void)self;
	(void)lid;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gthrow_global_unbound(struct Dee_function_generator *__restrict self, uint16_t gid) {
	/* TODO: Implement using `_Dee_function_generator_gcall_c_function()'+`Dee_function_generator_gjmp_except()' */
	(void)self;
	(void)gid;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gthrow_extern_unbound(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid) {
	/* TODO: Implement using `_Dee_function_generator_gcall_c_function()'+`Dee_function_generator_gjmp_except()' */
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
	bb = Dee_function_generator_except_exit(self);
	if unlikely(!bb)
		goto err;
	return _Dee_function_generator_gjz(self, &bb->bb_htext, loc);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gjnz_except(struct Dee_function_generator *__restrict self,
                                   struct Dee_memloc *loc) {
	struct Dee_basic_block *bb;
	bb = Dee_function_generator_except_exit(self);
	if unlikely(!bb)
		goto err;
	return _Dee_function_generator_gjnz(self, &bb->bb_htext, loc);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjmp_except(struct Dee_function_generator *__restrict self) {
	struct Dee_basic_block *bb;
	bb = Dee_function_generator_except_exit(self);
	if unlikely(!bb)
		goto err;
	return _Dee_function_generator_gjmp(self, &bb->bb_htext);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gincref(struct Dee_function_generator *__restrict self,
                               struct Dee_memloc *__restrict loc) {
	switch (loc->ml_where) {
	case MEMLOC_TYPE_HSTACK:
	case MEMLOC_TYPE_ARG:
		if unlikely(Dee_function_generator_greg(self, loc))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gincref_reg(self, loc->ml_value.ml_hreg);
	case MEMLOC_TYPE_CONST:
		return _Dee_function_generator_gincref_const(self, loc->ml_value.ml_const);
	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot incref memory location type %#" PRFx16,
		                       loc->ml_where);
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdecref(struct Dee_function_generator *__restrict self,
                               struct Dee_memloc *__restrict loc) {
	switch (loc->ml_where) {
	case MEMLOC_TYPE_HSTACK:
	case MEMLOC_TYPE_ARG:
		if unlikely(Dee_function_generator_greg(self, loc))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gdecref_reg(self, loc->ml_value.ml_hreg);
	case MEMLOC_TYPE_CONST:
		return _Dee_function_generator_gdecref_const(self, loc->ml_value.ml_const);
	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot decref memory location type %#" PRFx16,
		                       loc->ml_where);
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxincref(struct Dee_function_generator *__restrict self,
                                struct Dee_memloc *__restrict loc) {
	switch (loc->ml_where) {
	case MEMLOC_TYPE_HSTACK:
	case MEMLOC_TYPE_ARG:
		if unlikely(Dee_function_generator_greg(self, loc))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gxincref_reg(self, loc->ml_value.ml_hreg);
	case MEMLOC_TYPE_CONST:
		if (!loc->ml_value.ml_const)
			return 0;
		return _Dee_function_generator_gincref_const(self, loc->ml_value.ml_const);
	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot incref memory location type %#" PRFx16,
		                       loc->ml_where);
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxdecref(struct Dee_function_generator *__restrict self,
                                struct Dee_memloc *__restrict loc) {
	switch (loc->ml_where) {
	case MEMLOC_TYPE_HSTACK:
	case MEMLOC_TYPE_ARG:
		if unlikely(Dee_function_generator_greg(self, loc))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gxdecref_reg(self, loc->ml_value.ml_hreg);
	case MEMLOC_TYPE_CONST:
		if (!loc->ml_value.ml_const)
			return 0;
		return _Dee_function_generator_gdecref_const(self, loc->ml_value.ml_const);
	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot incref memory location type %#" PRFx16,
		                       loc->ml_where);
	}
	__builtin_unreachable();
err:
	return -1;
}

/* Force `loc' to become a register (`MEMLOC_TYPE_HREG'). */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_greg(struct Dee_function_generator *__restrict self,
                            struct Dee_memloc *__restrict loc) {
	Dee_host_register_t regno;
	if (loc->ml_where == MEMLOC_TYPE_HREG)
		return 0; /* Already in a register! */

	/* Allocate a register. */
	regno = Dee_function_generator_gallocreg(self, NULL);
	if unlikely(regno >= HOST_REGISTER_COUNT)
		goto err;

	/* Move value into register. */
	if unlikely(Dee_function_generator_gmov_loc2reg(self, loc, regno))
		goto err;

	/* Remember that `loc' now lies in a register. */
	loc->ml_where = MEMLOC_TYPE_HREG;
	loc->ml_value.ml_hreg = regno;
	return 0;
err:
	return -1;
}

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_COMMON_C */
