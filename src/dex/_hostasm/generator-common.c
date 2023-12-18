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
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/hashset.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/super.h>
#include <deemon/tuple.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/overflow.h>
#include <hybrid/unaligned.h>

DECL_BEGIN

/************************************************************************/
/* CODE GENERATION                                                      */
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
	return _Dee_function_generator_gjz(self, bb, loc);
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
	return _Dee_function_generator_gjnz(self, bb, loc);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjmp_except(struct Dee_function_generator *__restrict self) {
	struct Dee_basic_block *bb;
	bb = Dee_function_generator_except_exit(self);
	if unlikely(!bb)
		goto err;
	return _Dee_function_generator_gjmp(self, bb);
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




















/************************************************************************/
/* VSTACK CONTROLS                                                      */
/************************************************************************/

/* Code generator helpers to manipulate the V-stack. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vswap(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vswap(self->fg_state);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vlrot(struct Dee_function_generator *__restrict self, size_t n) {
	int result;
	if unlikely(n <= 1)
		return 0;
	result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vlrot(self->fg_state, n);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vrrot(struct Dee_function_generator *__restrict self, size_t n) {
	int result;
	if unlikely(n <= 1)
		return 0;
	result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vrrot(self->fg_state, n);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpush(struct Dee_function_generator *__restrict self, struct Dee_memloc *loc) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush(self->fg_state, loc);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_const(struct Dee_function_generator *__restrict self, DeeObject *value) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_const(self->fg_state, value);
	return result;
}


/* Sets the `MEMLOC_F_NOREF' flag */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_reg(struct Dee_function_generator *__restrict self,
                                 Dee_host_register_t regno) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_reg(self->fg_state, regno);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_arg(struct Dee_function_generator *__restrict self, uint16_t aid) {
	int result;
	if unlikely(aid >= self->fg_assembler->fa_code->co_argc_max)
		return err_illegal_aid(aid);
	result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_arg(self->fg_state, aid);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_local(struct Dee_function_generator *__restrict self, uint16_t lid) {
	struct Dee_memstate *state;
	struct Dee_memloc *loc;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	state = self->fg_state;
	if unlikely(lid >= state->ms_localc)
		return err_illegal_lid(lid);
	if unlikely(state->ms_stackc >= state->ms_stacka &&
	            Dee_memstate_reqvstack(state, state->ms_stackc + 1))
		goto err;
	loc = &state->ms_localv[lid];
	if ((loc->ml_where == MEMLOC_TYPE_UNALLOC) ||
	    (loc->ml_flags & MEMLOC_F_LOCAL_UNBOUND)) {
		/* Variable is always unbound -> generate code to throw an exception */
		if unlikely(Dee_function_generator_gthrow_local_unbound(self, lid))
			goto err;
		return Dee_function_generator_vpush_const(self, Dee_None);
	} else if (!(loc->ml_flags & MEMLOC_F_LOCAL_BOUND)) {
		/* Variable is not guarantied bound -> generate code to check if it's bound */
		if unlikely(Dee_function_generator_gassert_local_bound(self, lid))
			goto err;
		/* After a bound assertion, the local variable is guarantied to be bound. */
		loc->ml_flags |= MEMLOC_F_LOCAL_BOUND;
	}
	state->ms_stackv[state->ms_stackc] = *loc;
	state->ms_stackv[state->ms_stackc].ml_flags &= ~MEMLOC_M_LOCAL_BSTATE;
	++state->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdup_n(struct Dee_function_generator *__restrict self, size_t n) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vdup_n(self->fg_state, n);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop(struct Dee_function_generator *__restrict self) {
	struct Dee_memstate *state;
	struct Dee_memloc *loc;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	state = self->fg_state;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	loc = Dee_memstate_vtop(state);
	--state->ms_stackc;
	if (!(loc->ml_flags & MEMLOC_F_NOREF)) {
		uint16_t i;
		struct Dee_memloc *other_loc;

		/* Try and shift the burden of the reference to the other location. */
		for (i = 0; i < state->ms_stackc - 1; ++i) {
			other_loc = &state->ms_stackv[i];
			if (Dee_memloc_sameloc(loc, other_loc) &&
			    (other_loc->ml_flags & MEMLOC_F_NOREF)) {
				other_loc->ml_flags &= ~MEMLOC_F_NOREF;
				goto done;
			}
		}
		for (i = 0; i < state->ms_localc; ++i) {
			other_loc = &state->ms_localv[i];
			if (Dee_memloc_sameloc(loc, other_loc) &&
			    (other_loc->ml_flags & MEMLOC_F_NOREF)) {
				other_loc->ml_flags &= ~MEMLOC_F_NOREF;
				goto done;
			}
		}

		/* No-where to shift the reference to -> must decref the object ourselves. */
		if unlikely(Dee_function_generator_gdecref(self, loc)) {
			++state->ms_stackc;
			goto err;
		}
	}

done:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_n(struct Dee_function_generator *__restrict self, size_t n) {
	ASSERT(n >= 1);
	if unlikely(Dee_function_generator_vlrot(self, n))
		goto err;
	if unlikely(Dee_function_generator_vpop(self))
		goto err;
	return Dee_function_generator_vrrot(self, n - 1);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpush_usage(struct Dee_function_generator *__restrict self,
                                   Dee_host_regusage_t usage) {
	struct Dee_memstate *state;
	Dee_host_register_t regno;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	state = self->fg_state;
	for (regno = 0; regno < HOST_REGISTER_COUNT; ++regno) {
		if (state->ms_regs[regno] == usage)
			break;
	}
	if (regno >= HOST_REGISTER_COUNT) {
		/* Need to allocate a register for `usage'. */
		regno = Dee_function_generator_gallocreg(self, NULL);
		if unlikely(regno >= HOST_REGISTER_COUNT)
			goto err;
		if unlikely(Dee_function_generator_gmov_usage2reg(self, usage, regno))
			goto err;
	}
	if unlikely(state->ms_stackc >= state->ms_stacka &&
	            Dee_memstate_reqvstack(state, state->ms_stackc + 1))
		goto err;
	state->ms_stackv[state->ms_stackc].ml_flags = MEMLOC_F_NOREF;
	state->ms_stackv[state->ms_stackc].ml_where = MEMLOC_TYPE_HREG;
	state->ms_stackv[state->ms_stackc].ml_value.ml_hreg = regno;
	++state->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_local(struct Dee_function_generator *__restrict self,
                                  uint16_t lid) {
	struct Dee_memstate *state;
	struct Dee_memloc *src, *dst;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	state = self->fg_state;
	if unlikely(lid >= state->ms_localc)
		return err_illegal_lid(lid);
	src = Dee_memstate_vtop(state);
	dst = &state->ms_localv[lid];
	if ((dst->ml_where == MEMLOC_TYPE_UNALLOC) ||
	    (dst->ml_flags & MEMLOC_F_NOREF)) {
		/* Simple case: local variable hadn't been
		 * allocated yet, or didn't contain a reference. */
	} else {
		/* Local variable already has a assigned -> must decref that value. */
		if (dst->ml_flags & MEMLOC_F_LOCAL_BOUND) {
			/* Guarantied bound -> normal decref() */
			if unlikely(Dee_function_generator_gdecref(self, dst))
				goto err;
		} else if (!(dst->ml_flags & MEMLOC_F_LOCAL_UNBOUND)) {
			/* Not guarantied bound (but also not guarantied unbound) -> xdecref() */
			if unlikely(Dee_function_generator_gxdecref(self, dst))
				goto err;
		}
	}

	/* Because stack elements are always bound, the local is guarantied bound at this point. */
	*dst = *src;
	dst->ml_flags &= ~MEMLOC_M_LOCAL_BSTATE;
	dst->ml_flags |= MEMLOC_F_LOCAL_BOUND;
	--state->ms_stackc;
/*done:*/
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdel_local(struct Dee_function_generator *__restrict self, uint16_t lid) {
	struct Dee_memstate *state;
	struct Dee_memloc *loc;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	state = self->fg_state;
	if unlikely(lid >= state->ms_localc)
		return err_illegal_lid(lid);
	loc = &state->ms_localv[lid];
	if ((loc->ml_where != MEMLOC_TYPE_UNALLOC) &&
	    !(loc->ml_flags & MEMLOC_F_NOREF)) {
		if (loc->ml_flags & MEMLOC_F_LOCAL_UNBOUND) {
			/* Nothing to do here! */
		} else if (loc->ml_flags & MEMLOC_F_LOCAL_BOUND) {
			if unlikely(Dee_function_generator_gdecref(self, loc))
				goto err;
		} else {
			if unlikely(Dee_function_generator_gxdecref(self, loc))
				goto err;
		}
		loc = &state->ms_localv[lid];
	}
	loc->ml_flags = MEMLOC_F_NOREF | MEMLOC_F_LOCAL_UNBOUND;
	loc->ml_where = MEMLOC_TYPE_UNALLOC;
	return 0;
err:
	return -1;
}


#ifdef CONFIG_HAVE_FPU
/* API function called by `operator float()' */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
api_object_as_float(DeeObject *__restrict self) {
	double result;
	if unlikely(DeeObject_AsDouble(self, &result))
		goto err;
	return DeeFloat_New(result);
err:
	return NULL;
}
#endif /* CONFIG_HAVE_FPU */

struct host_operator_specs {
	void   *hos_apifunc; /* [0..1] API function (or NULL if fallback handling must be used) */
	uint8_t hos_argc;    /* Argument count (1-4) */
	uint8_t hos_cc;      /* Operator calling convention (one of `VCALLOP_CC_*') */
};

PRIVATE struct host_operator_specs const operator_apis[] = {
	/* [OPERATOR_CONSTRUCTOR]  = */ { (void *)NULL },
	/* [OPERATOR_COPY]         = */ { (void *)&DeeObject_Copy, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_DEEPCOPY]     = */ { (void *)&DeeObject_DeepCopy, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_DESTRUCTOR]   = */ { (void *)NULL },
	/* [OPERATOR_ASSIGN]       = */ { (void *)&DeeObject_Assign, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_MOVEASSIGN]   = */ { (void *)&DeeObject_MoveAssign, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_STR]          = */ { (void *)&DeeObject_Str, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_REPR]         = */ { (void *)&DeeObject_Repr, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_BOOL]         = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_ITERNEXT]     = */ { (void *)&DeeObject_IterNext, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_CALL]         = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INT]          = */ { (void *)&DeeObject_Int, 1, VCALLOP_CC_OBJECT },
#ifdef CONFIG_HAVE_FPU
	/* [OPERATOR_FLOAT]        = */ { (void *)&api_object_as_float, 1, VCALLOP_CC_OBJECT },
#else /* CONFIG_HAVE_FPU */
	/* [OPERATOR_FLOAT]        = */ { (void *)NULL },
#endif /* !CONFIG_HAVE_FPU */
	/* [OPERATOR_INV]          = */ { (void *)&DeeObject_Inv, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_POS]          = */ { (void *)&DeeObject_Pos, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_NEG]          = */ { (void *)&DeeObject_Neg, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_ADD]          = */ { (void *)&DeeObject_Add, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_SUB]          = */ { (void *)&DeeObject_Sub, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_MUL]          = */ { (void *)&DeeObject_Mul, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_DIV]          = */ { (void *)&DeeObject_Div, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_MOD]          = */ { (void *)&DeeObject_Mod, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_SHL]          = */ { (void *)&DeeObject_Shl, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_SHR]          = */ { (void *)&DeeObject_Shr, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_AND]          = */ { (void *)&DeeObject_And, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_OR]           = */ { (void *)&DeeObject_Or, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_XOR]          = */ { (void *)&DeeObject_Xor, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_POW]          = */ { (void *)&DeeObject_Pow, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_INC]          = */ { (void *)&DeeObject_Inc, 1, VCALLOP_CC_INPLACE },
	/* [OPERATOR_DEC]          = */ { (void *)&DeeObject_Dec, 1, VCALLOP_CC_INPLACE },
	/* [OPERATOR_INPLACE_ADD]  = */ { (void *)&DeeObject_InplaceAdd, 2, VCALLOP_CC_INPLACE },
	/* [OPERATOR_INPLACE_SUB]  = */ { (void *)&DeeObject_InplaceSub, 2, VCALLOP_CC_INPLACE },
	/* [OPERATOR_INPLACE_MUL]  = */ { (void *)&DeeObject_InplaceMul, 2, VCALLOP_CC_INPLACE },
	/* [OPERATOR_INPLACE_DIV]  = */ { (void *)&DeeObject_InplaceDiv, 2, VCALLOP_CC_INPLACE },
	/* [OPERATOR_INPLACE_MOD]  = */ { (void *)&DeeObject_InplaceMod, 2, VCALLOP_CC_INPLACE },
	/* [OPERATOR_INPLACE_SHL]  = */ { (void *)&DeeObject_InplaceShl, 2, VCALLOP_CC_INPLACE },
	/* [OPERATOR_INPLACE_SHR]  = */ { (void *)&DeeObject_InplaceShr, 2, VCALLOP_CC_INPLACE },
	/* [OPERATOR_INPLACE_AND]  = */ { (void *)&DeeObject_InplaceAnd, 2, VCALLOP_CC_INPLACE },
	/* [OPERATOR_INPLACE_OR]   = */ { (void *)&DeeObject_InplaceOr, 2, VCALLOP_CC_INPLACE },
	/* [OPERATOR_INPLACE_XOR]  = */ { (void *)&DeeObject_InplaceXor, 2, VCALLOP_CC_INPLACE },
	/* [OPERATOR_INPLACE_POW]  = */ { (void *)&DeeObject_InplacePow, 2, VCALLOP_CC_INPLACE },
	/* [OPERATOR_HASH]         = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_EQ]           = */ { (void *)&DeeObject_CompareEqObject, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_NE]           = */ { (void *)&DeeObject_CompareNeObject, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_LO]           = */ { (void *)&DeeObject_CompareLoObject, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_LE]           = */ { (void *)&DeeObject_CompareLeObject, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_GR]           = */ { (void *)&DeeObject_CompareGrObject, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_GE]           = */ { (void *)&DeeObject_CompareGeObject, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_ITERSELF]     = */ { (void *)&DeeObject_IterSelf, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_SIZE]         = */ { (void *)&DeeObject_SizeObject, 1, VCALLOP_CC_OBJECT },
	/* [OPERATOR_CONTAINS]     = */ { (void *)&DeeObject_Contains, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_GETITEM]      = */ { (void *)&DeeObject_GetItem, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_DELITEM]      = */ { (void *)&DeeObject_DelItem, 2, VCALLOP_CC_INT },
	/* [OPERATOR_SETITEM]      = */ { (void *)&DeeObject_SetItem, 3, VCALLOP_CC_INT },
	/* [OPERATOR_GETRANGE]     = */ { (void *)&DeeObject_GetRange, 3, VCALLOP_CC_OBJECT },
	/* [OPERATOR_DELRANGE]     = */ { (void *)&DeeObject_DelRange, 3, VCALLOP_CC_INT },
	/* [OPERATOR_SETRANGE]     = */ { (void *)&DeeObject_SetRange, 4, VCALLOP_CC_INT },
	/* [OPERATOR_GETATTR]      = */ { (void *)&DeeObject_GetAttr, 2, VCALLOP_CC_OBJECT },
	/* [OPERATOR_DELATTR]      = */ { (void *)&DeeObject_GetAttr, 2, VCALLOP_CC_INT },
	/* [OPERATOR_SETATTR]      = */ { (void *)&DeeObject_GetAttr, 3, VCALLOP_CC_INT },
	/* [OPERATOR_ENUMATTR]     = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_ENTER]        = */ { (void *)&DeeObject_Enter, 1, VCALLOP_CC_INT },
	/* [OPERATOR_LEAVE]        = */ { (void *)&DeeObject_Leave, 1, VCALLOP_CC_INT },
};

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vop(struct Dee_function_generator *__restrict self,
                           uint16_t operator_name, uint16_t argc) {
	struct Dee_memstate *state;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	state = self->fg_state;
	if unlikely(argc > state->ms_stackc)
		return err_illegal_stack_effect();

	/* Check if there is a dedicated API function. */
	if (operator_name < COMPILER_LENOF(operator_apis)) {
		struct host_operator_specs const *specs = &operator_apis[operator_name];
		if (specs->hos_apifunc != NULL && specs->hos_argc == argc) {
			if unlikely(Dee_function_generator_vcallop(self, specs->hos_apifunc,
			                                           specs->hos_cc, argc))
				goto err;
			/* Always make sure to return some value on-stack. */
			if (specs->hos_cc != VCALLOP_CC_OBJECT)
				return Dee_function_generator_vpush_const(self, Dee_None);
			return 0;
		}
	}

	switch (operator_name) {

	case OPERATOR_CALL:
		if (argc == 2)
			return Dee_function_generator_vcallop(self, (void *)&DeeObject_CallTuple, VCALLOP_CC_OBJECT, 2);
		if (argc == 3)
			return Dee_function_generator_vcallop(self, (void *)&DeeObject_CallTupleKw, VCALLOP_CC_OBJECT, 3);
		break;

	default: break;
	}

	/* TODO: make a call to `DeeObject_InvokeOperator()' */
	return DeeError_NOTIMPLEMENTED();
err:
	return -1;
}

/* doesn't leave result on-stack */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopv(struct Dee_function_generator *__restrict self,
                            uint16_t operator_name, uint16_t argc) {
#ifndef __OPTIMIZE_SIZE__
	/* Check if there is a dedicated API function. */
	if (operator_name < COMPILER_LENOF(operator_apis)) {
		struct host_operator_specs const *specs = &operator_apis[operator_name];
		if (specs->hos_apifunc != NULL && specs->hos_argc == argc) {
			if unlikely(argc > self->fg_state->ms_stackc)
				return err_illegal_stack_effect();
			if unlikely(Dee_function_generator_vcallop(self, specs->hos_apifunc,
			                                           specs->hos_cc, argc))
				goto err;
			/* Pop the result if there was one. */
			if (specs->hos_cc == VCALLOP_CC_OBJECT)
				goto done_pop;
			return 0;
		}
	}
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(Dee_function_generator_vop(self, operator_name, argc))
		goto err;
#ifndef __OPTIMIZE_SIZE__
done_pop:
#endif /* !__OPTIMIZE_SIZE__ */
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vjcc(struct Dee_function_generator *__restrict self,
                            struct Dee_basic_block *target, bool jump_if_true) {
	struct Dee_basic_block *except_exit;
	struct Dee_memstate *state;
	struct Dee_memloc *loc, arg_loc;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	state = self->fg_state;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	arg_loc = *Dee_memstate_vtop(state);

	/* Special case for when the top-element is a constant. */
	if (arg_loc.ml_where == MEMLOC_TYPE_CONST) {
		int temp = DeeObject_Bool(arg_loc.ml_value.ml_const);
		if unlikely(temp < 0) {
			DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
		} else {
			bool should_jump = (temp > 0) == jump_if_true;
			if (should_jump) {
				if unlikely(Dee_function_generator_gjmp(self, target))
					goto err;
			}
			return Dee_function_generator_vpop(self);
		}
	}

	/* Evaluate the top stack-object to a boolean (via `DeeObject_Bool'). */
	if unlikely(Dee_function_generator_gflushregs(self, 1))
		goto err;
	if unlikely(_Dee_function_generator_gcall_c_function(self, (void *)&DeeObject_Bool, 1, &arg_loc))
		goto err;
	if unlikely(Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN))
		goto err;
	if unlikely(Dee_function_generator_vswap(self))
		goto err;
	if unlikely(Dee_function_generator_vpop(self))
		goto err;

	/* At this point, the stack-top location  */
	loc = Dee_function_generator_vtop(self);
	ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
	except_exit = Dee_function_generator_except_exit(self);
	if unlikely(!except_exit)
		goto err;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(_Dee_function_generator_gjcmp0(self,
	                                           except_exit,
	                                           jump_if_true ? NULL : target,
	                                           jump_if_true ? target : NULL,
	                                           loc))
		goto err;
	return Dee_function_generator_vpop(self);
err:
	return -1;
}



/* Take the base address of the top-most `DeeObject' from the object-stack, add `offset' to that
 * base address (possibly at runtime), then re-interpret that address as `(DeeObject **)<addr>'
 * and dereference it, before storing the resulting value back into the to-most stack item. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vind(struct Dee_function_generator *__restrict self, ptrdiff_t offset) {
	Dee_host_register_t dst_regno;
	struct Dee_memstate *state = self->fg_state;
	struct Dee_memloc *loc;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	loc = Dee_memstate_vtop(state);
	ASSERTF(loc->ml_flags & MEMLOC_F_NOREF,
	        "Dee_function_generator_vind() called on reference");
	if (loc->ml_where == MEMLOC_TYPE_HREG) {
		/* Special case: source operand is already a register. */
		dst_regno = loc->ml_value.ml_hreg;
		return Dee_function_generator_gmov_locind2reg(self, loc, offset, dst_regno);
	}

	/* Need a temporary register for the indirection result. */
	dst_regno = Dee_function_generator_gallocreg(self, NULL);
	if unlikely(dst_regno >= HOST_REGISTER_COUNT)
		goto err;
	if unlikely(Dee_function_generator_gmov_locind2reg(self, loc, offset, dst_regno))
		goto err;

	/* Remember the register as stack value. */
	loc->ml_where = MEMLOC_TYPE_HREG;
	loc->ml_value.ml_hreg = dst_regno;
	return 0;
err:
	return -1;
}

/* >> *(SECOND + offset) = FIRST; POP(); POP(); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_ind(struct Dee_function_generator *__restrict self, ptrdiff_t offset) {
	/* TODO */
	(void)self;
	(void)offset;
	return DeeError_NOTIMPLEMENTED();
}

/* >> temp = *(SECOND + offset); *(SECOND + offset) = FIRST; FIRST = temp; */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vxch_ind(struct Dee_function_generator *__restrict self, ptrdiff_t offset) {
	/* TODO */
	(void)self;
	(void)offset;
	return DeeError_NOTIMPLEMENTED();
}

/* Ensure that the top-most `DeeObject' from the object-stack is a reference. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vref(struct Dee_function_generator *__restrict self) {
	struct Dee_memstate *state = self->fg_state;
	struct Dee_memloc *loc;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	loc = Dee_memstate_vtop(state);
	if (loc->ml_flags & MEMLOC_F_NOREF) {
		if unlikely(Dee_function_generator_state_unshare(self))
			goto err;
		state = self->fg_state;
		loc   = Dee_memstate_vtop(state);
		ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
		if unlikely(Dee_function_generator_gincref(self, loc))
			goto err;
		loc = Dee_memstate_vtop(state);
		ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
		loc->ml_flags &= ~MEMLOC_F_NOREF;
	}
	return 0;
err:
	return -1;
}

/* Generate code to push a global variable onto the virtual stack. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vpush_global_or_extern(struct Dee_function_generator *__restrict self,
                       DeeModuleObject *__restrict mod, uint16_t gid,
                       uint8_t kind, uint16_t id1, uint16_t id2) {
	struct module_symbol *symbol;
	struct Dee_memloc *loc;
	symbol = DeeModule_GetSymbolID(mod, gid);
	if unlikely(!symbol)
		return err_illegal_gid(mod, gid);
	ASSERT(symbol->ss_index == gid);
	/* Global object references can be inlined if they are `final' and bound */
	if (symbol->ss_flags & Dee_MODSYM_FREADONLY) {
		DeeObject *current_value;
		DeeModule_LockRead(mod);
		current_value = mod->mo_globalv[gid];
		DeeModule_LockEndRead(mod);
		if (current_value != NULL)
			return Dee_function_generator_vpush_const(self, current_value);
	}
	if unlikely(Dee_function_generator_vpush_addr(self, &mod->mo_globalv[gid]))
		goto err;
	if unlikely(Dee_function_generator_grwlock_read(self, &mod->mo_lock))
		goto err;
	if unlikely(Dee_function_generator_vind(self, 0))
		goto err;
	loc = Dee_function_generator_vtop(self);
	ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
	if unlikely(Dee_function_generator_gassert_bound(self, loc, kind, id1, id2, &mod->mo_lock))
		goto err;
	loc = Dee_function_generator_vtop(self);
	ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
	if unlikely(Dee_function_generator_gincref(self, loc))
		goto err;
	if unlikely(Dee_function_generator_grwlock_endread(self, &mod->mo_lock))
		goto err;
	loc = Dee_function_generator_vtop(self);
	ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
	loc->ml_flags &= ~MEMLOC_F_NOREF;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_global(struct Dee_function_generator *__restrict self, uint16_t gid) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	return vpush_global_or_extern(self, mod, gid, ASM_GLOBAL, gid, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_extern(struct Dee_function_generator *__restrict self,
                                    uint16_t mid, uint16_t gid) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	if unlikely(mid >= mod->mo_importc)
		return err_illegal_mid(mid);
	mod = mod->mo_importv[mid];
	return vpush_global_or_extern(self, mod, gid, ASM_EXTERN, mid, gid);
}

/* Generate code to pop a global variable from the virtual stack. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vpop_global_or_extern(struct Dee_function_generator *__restrict self,
                      DeeModuleObject *__restrict mod, uint16_t gid) {
	struct Dee_memloc *loc;
	if unlikely(gid >= mod->mo_globalc)
		return err_illegal_gid(mod, gid);
	if unlikely(Dee_function_generator_vpush_addr(self, &mod->mo_globalv[gid]))
		goto err;
	if unlikely(Dee_function_generator_vswap(self))
		goto err;
	if unlikely(Dee_function_generator_grwlock_write(self, &mod->mo_lock))
		goto err;
	if unlikely(Dee_function_generator_vxch_ind(self, 0))
		goto err;
	if unlikely(Dee_function_generator_grwlock_endwrite(self, &mod->mo_lock))
		goto err;
	ASSERT(self->fg_state->ms_stackc >= 2);
	loc = Dee_function_generator_vtop(self);
	loc->ml_flags |= MEMLOC_F_NOREF;
	if unlikely(Dee_function_generator_gxdecref(self, loc)) /* xdecref in case global wasn't bound before. */
		goto err;
	if unlikely(Dee_function_generator_vpop(self))
		goto err;
	if unlikely(Dee_function_generator_vpop(self))
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_global(struct Dee_function_generator *__restrict self, uint16_t gid) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	int result = Dee_function_generator_vref(self);
	if likely(result == 0)
		result = vpop_global_or_extern(self, mod, gid);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid) {
	int result;
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	if unlikely(mid >= mod->mo_importc)
		return err_illegal_mid(mid);
	mod = mod->mo_importv[mid];
	result = Dee_function_generator_vref(self);
	if likely(result == 0)
		result = vpop_global_or_extern(self, mod, gid);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdel_global(struct Dee_function_generator *__restrict self, uint16_t gid) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	int result = Dee_function_generator_vpush_addr(self, NULL);
	if likely(result == 0)
		result = vpop_global_or_extern(self, mod, gid);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdel_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid) {
	int result;
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	if unlikely(mid >= mod->mo_importc)
		return err_illegal_mid(mid);
	mod = mod->mo_importv[mid];
	result = Dee_function_generator_vpush_addr(self, NULL);
	if likely(result == 0)
		result = vpop_global_or_extern(self, mod, gid);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_static(struct Dee_function_generator *__restrict self, uint16_t sid) {
	struct Dee_memloc *loc;
	DeeCodeObject *code = self->fg_assembler->fa_code;
	if unlikely(sid >= code->co_staticc)
		return err_illegal_sid(sid);
	if unlikely(Dee_function_generator_vpush_addr(self, &code->co_staticv[sid]))
		goto err;
	if unlikely(Dee_function_generator_grwlock_read(self, &code->co_static_lock))
		goto err;
	if unlikely(Dee_function_generator_vind(self, 0))
		goto err;
	loc = Dee_function_generator_vtop(self);
	ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
	loc->ml_flags &= ~MEMLOC_F_NOREF;
	if unlikely(Dee_function_generator_gincref(self, loc))
		goto err;
	return Dee_function_generator_grwlock_endread(self, &code->co_static_lock);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_static(struct Dee_function_generator *__restrict self, uint16_t sid) {
	DeeCodeObject *code = self->fg_assembler->fa_code;
	if unlikely(sid >= code->co_staticc)
		return err_illegal_sid(sid);
	if unlikely(Dee_function_generator_vref(self))
		goto err;
	ASSERT(!(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF));
	if unlikely(Dee_function_generator_vpush_addr(self, &code->co_staticv[sid]))
		goto err;
	if unlikely(Dee_function_generator_vswap(self))
		goto err;
	if unlikely(Dee_function_generator_grwlock_write(self, &code->co_static_lock))
		goto err;
	if unlikely(Dee_function_generator_vxch_ind(self, 0))
		goto err;
	if unlikely(Dee_function_generator_grwlock_endwrite(self, &code->co_static_lock))
		goto err;
	ASSERT(!(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF));
	if unlikely(Dee_function_generator_vpop(self))
		goto err;
	ASSERT(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF);
	if unlikely(Dee_function_generator_vpop(self))
		goto err;
	return 0;
err:
	return -1;
}


/* Return and pass the top-most stack element as function result.
 * This function will clear the stack and unbind all local variables. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vret(struct Dee_function_generator *__restrict self) {
	struct Dee_memloc loc;
	uint16_t i, stackc = self->fg_state->ms_stackc;
	if unlikely(stackc < 1)
		return err_illegal_stack_effect();

	/* Move the final return value to the bottom of the stack. */
	if unlikely(Dee_function_generator_vrrot(self, stackc))
		goto err;

	/* Remove all but the final element from the stack. */
	while (stackc > 1) {
		if unlikely(Dee_function_generator_vpop(self))
			goto err;
		--stackc;
	}

	/* Unbind all local variables. */
	for (i = 0; i < self->fg_state->ms_localc; ++i) {
		if unlikely(Dee_function_generator_vdel_local(self, i))
			goto err;
	}

	/* Ensure that the final stack element contains a reference. */
	if unlikely(Dee_function_generator_vref(self))
		goto err;

	/* Steal the final (returned) object from stack */
	ASSERT(self->fg_state->ms_stackc == 1);
	loc = self->fg_state->ms_stackv[0];
	self->fg_state->ms_stackc = 0;
	ASSERT(!(loc.ml_flags & MEMLOC_F_NOREF));

	/* Generate code to do the return. */
	return Dee_function_generator_gret(self, &loc);
err:
	return -1;
}

PRIVATE NONNULL((1)) int
(DCALL DeeError_Throw_inherited)(DREF DeeObject *__restrict error) {
	int result = DeeError_Throw(error);
	Dee_Decref_unlikely(error);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vthrow(struct Dee_function_generator *__restrict self) {
	struct Dee_memloc loc;
	uint16_t stackc;
	if unlikely(Dee_function_generator_vref(self))
		goto err;
	/* Steal object from stack */
	stackc = self->fg_state->ms_stackc;
	ASSERT(stackc >= 1);
	loc = self->fg_state->ms_stackv[stackc - 1];
	ASSERT(!(loc.ml_flags & MEMLOC_F_NOREF));
	--self->fg_state->ms_stackc;
	if unlikely(_Dee_function_generator_gcall_c_function(self, &DeeError_Throw_inherited, 1, &loc))
		goto err;
	return Dee_function_generator_gjmp_except(self);
err:
	return -1;
}



/* Generate host text to invoke `api_function' with the top-most `argc' items from the stack.
 * @param: cc: One of `VCALLOP_CC_*', describing the calling-convention of `api_function' */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcallop(struct Dee_function_generator *__restrict self,
                               void *api_function, unsigned int cc, uint16_t argc) {
	struct Dee_memloc *loc, *argv;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(argc > self->fg_state->ms_stackc)
		return err_illegal_stack_effect();

	/* Save argument memory locations from before the flush. This is because after the
	 * flush, registers are written to the stack, and if we were to pass the then-current
	 * `Dee_memloc' to `_Dee_function_generator_gcall_c_function', it would have to load
	 * those values from the stack (when they can also be found in registers) */
	loc = self->fg_state->ms_stackv;
	loc += self->fg_state->ms_stackc;
	loc -= argc;
	argv = (struct Dee_memloc *)Dee_Mallocac(argc, sizeof(struct Dee_memloc));
	if unlikely(!argv)
		goto err;
	argv = (struct Dee_memloc *)memcpyc(argv, loc, argc, sizeof(struct Dee_memloc));

	/* Flush registers that don't appear in the top `argc' stack locations. */
	if unlikely(Dee_function_generator_gflushregs(self, argc))
		goto err_argv;

	/* Call the actual C function */
	if unlikely(_Dee_function_generator_gcall_c_function(self, api_function, argc, argv))
		goto err_argv;
	Dee_Freea(argv);
	if unlikely(Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN))
		goto err;
	loc = Dee_function_generator_vtop(self);
	if (cc == VCALLOP_CC_OBJECT) {
		if unlikely(Dee_function_generator_gjz_except(self, loc))
			goto err;
		/* Clear the NOREF flag now that we know the return value to be non-NULL */
		loc = Dee_function_generator_vtop(self);
		ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
		loc->ml_flags &= ~MEMLOC_F_NOREF;

		/* Rotate the return value to be located *below* arguments (which get popped below) */
		if unlikely(Dee_function_generator_vrrot(self, argc + 1))
			goto err;
	} else {
		if unlikely(Dee_function_generator_gjnz_except(self, loc))
			goto err;
		ASSERT(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF);
	}

	/* Pop function arguments. */
	while (argc) {
		--argc;
		if unlikely(Dee_function_generator_vpop(self))
			goto err;
	}
	return 0;
err_argv:
	Dee_Freea(argv);
err:
	return -1;
}




PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_prefix(struct Dee_function_generator *__restrict self,
                                    uint8_t prefix_type, uint16_t id1, uint16_t id2) {
	switch (prefix_type) {
	case ASM_STACK:
		return Dee_function_generator_vdup_n(self, self->fg_state->ms_stackc - id1);
	case ASM_STATIC:
		return Dee_function_generator_vpush_static(self, id1);
	case ASM_EXTERN:
		return Dee_function_generator_vpush_extern(self, id1, id2);
	case ASM_GLOBAL:
		return Dee_function_generator_vpush_global(self, id1);
	case ASM_LOCAL:
		return Dee_function_generator_vpush_local(self, id1);
	default: __builtin_unreachable();
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_prefix(struct Dee_function_generator *__restrict self,
                                   uint8_t prefix_type, uint16_t id1, uint16_t id2) {
	switch (prefix_type) {
	case ASM_STACK:
		return Dee_function_generator_vpop_n(self, self->fg_state->ms_stackc - id1);
	case ASM_STATIC:
		return Dee_function_generator_vpop_static(self, id1);
	case ASM_EXTERN:
		return Dee_function_generator_vpop_extern(self, id1, id2);
	case ASM_GLOBAL:
		return Dee_function_generator_vpop_global(self, id1);
	case ASM_LOCAL:
		return Dee_function_generator_vpop_local(self, id1);
	default: __builtin_unreachable();
	}
}



/* Convert a single deemon instruction `instr' to host assembly and adjust the host memory
 * state according to the instruction in question. This is the core function to parse deemon
 * code and convert it to host assembly. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_geninstr(struct Dee_function_generator *__restrict self,
                                Dee_instruction_t const *instr) {
	uint16_t opcode = instr[0];
	if (ASM_ISEXTENDED(opcode))
		opcode = (opcode << 8) | instr[1];
	switch (opcode) {

	case ASM_DELOP:
	case ASM16_DELOP:
	case ASM_NOP:
	case ASM16_NOP:
		/* No-op instructions */
		break;

	case ASM_SWAP:
		return Dee_function_generator_vswap(self);
	case ASM_LROT:
		return Dee_function_generator_vlrot(self, instr[1] + 3);
	case ASM16_LROT:
		return Dee_function_generator_vlrot(self, UNALIGNED_GETLE16(instr + 2) + 3);
	case ASM_RROT:
		return Dee_function_generator_vrrot(self, instr[1] + 3);
	case ASM16_RROT:
		return Dee_function_generator_vrrot(self, UNALIGNED_GETLE16(instr + 2) + 3);
	case ASM_DUP:
		return Dee_function_generator_vdup(self);
	case ASM_DUP_N:
		return Dee_function_generator_vdup_n(self, instr[1] + 2);
	case ASM16_DUP_N:
		return Dee_function_generator_vdup_n(self, UNALIGNED_GETLE16(instr + 2) + 2);
	case ASM_POP:
		return Dee_function_generator_vpop(self);
	case ASM_POP_N: {
		uint32_t n;
		n = instr[1] + 2;
		__IF0 { case ASM16_POP_N: n = UNALIGNED_GETLE16(instr + 2) + 2; }
		/* pop #SP - <imm8> - 2    (N = <imm8> + 2)
		 * <==>
		 * >> DECREF(STACKV[STACKC - N]);
		 * >> STACKV[STACKC - N] = STACKV[STACKC - 1];
		 * >> --STACKC;
		 * <==>
		 * >> LROT(N);
		 * >> POP();
		 * >> --N;
		 * >> RROT(N);
		 */
		if unlikely(Dee_function_generator_vlrot(self, n))
			goto err;
		if unlikely(Dee_function_generator_vpop(self))
			goto err;
		return Dee_function_generator_vrrot(self, n - 1);
	}	break;

	case ASM_ADJSTACK: {
		int32_t delta;
		delta = *(int8_t const *)(instr + 1);
		__IF0 { case ASM16_ADJSTACK: delta = (int16_t)UNALIGNED_GETLE16(instr + 2); }
		if (delta > 0) {
			do {
				if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
					goto err;
			} while (--delta > 0);
		} else if (delta < 0) {
			do {
				if unlikely(Dee_function_generator_vpop(self))
					goto err;
			} while (++delta < 0);
		}
	}	break;

	case ASM_POP_LOCAL: {
		uint16_t lid;
		lid = instr[1];
		__IF0 { case ASM16_POP_LOCAL: lid = UNALIGNED_GETLE16(instr + 2); }
		return Dee_function_generator_vpop_local(self, lid);
	}	break;

	case ASM_PUSH_NONE:
		return Dee_function_generator_vpush_const(self, Dee_None);
	case ASM_PUSH_THIS_MODULE:
		return Dee_function_generator_vpush_const(self, (DeeObject *)self->fg_assembler->fa_code->co_module);
	case ASM_PUSH_THIS_FUNCTION:
		return Dee_function_generator_vpush_const(self, (DeeObject *)self->fg_assembler->fa_function);

	case ASM_PUSH_MODULE: {
		DeeModuleObject *code_module;
		DeeModuleObject *push_module;
		uint16_t mid;
		mid = instr[1];
		__IF0 { case ASM16_PUSH_MODULE: mid = UNALIGNED_GETLE16(instr + 2); }
		code_module = self->fg_assembler->fa_code->co_module;
		if unlikely(mid >= code_module->mo_importc)
			return err_illegal_mid(mid);
		push_module = code_module->mo_importv[mid];
		return Dee_function_generator_vpush_const(self, (DeeObject *)push_module);
	}	break;

	case ASM_PUSH_ARG:
		return Dee_function_generator_vpush_arg(self, instr[1]);
	case ASM16_PUSH_ARG:
		return Dee_function_generator_vpush_arg(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_PUSH_CONST: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_PUSH_CONST: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		return Dee_function_generator_vpush_const(self, constant);
	}	break;

	case ASM_PUSH_REF: {
		uint16_t rid;
		DeeObject *reference;
		rid = instr[1];
		__IF0 { case ASM16_PUSH_REF: rid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(rid >= self->fg_assembler->fa_code->co_refc)
			return err_illegal_rid(rid);
		reference = self->fg_assembler->fa_function->fo_refv[rid];
		return Dee_function_generator_vpush_const(self, reference);
	}	break;

	case ASM_PUSH_EXTERN:
		return Dee_function_generator_vpush_extern(self, instr[1], instr[2]);
	case ASM16_PUSH_EXTERN:
		return Dee_function_generator_vpush_extern(self, UNALIGNED_GETLE16(instr + 2), UNALIGNED_GETLE16(instr + 4));
	case ASM_PUSH_GLOBAL:
		return Dee_function_generator_vpush_global(self, instr[1]);
	case ASM16_PUSH_GLOBAL:
		return Dee_function_generator_vpush_global(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_PUSH_LOCAL:
		return Dee_function_generator_vpush_local(self, instr[1]);
	case ASM16_PUSH_LOCAL:
		return Dee_function_generator_vpush_local(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_RET_NONE:
		if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
			goto err;
		return Dee_function_generator_vret(self);

	case ASM_RET:
		return Dee_function_generator_vret(self);

	case ASM_THROW:
		return Dee_function_generator_vthrow(self);

	//TODO: case ASM_SETRET:

	case ASM_DEL_LOCAL:
		return Dee_function_generator_vdel_local(self, instr[1]);
	case ASM16_DEL_LOCAL:
		return Dee_function_generator_vdel_local(self, UNALIGNED_GETLE16(instr + 2));

	//TODO: case ASM_CALL_KW:
	//TODO: case ASM16_CALL_KW:

	case ASM_CALL_TUPLE_KW: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_CALL_TUPLE_KW: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		return Dee_function_generator_vcallop(self, (void *)&DeeObject_CallTupleKw, VCALLOP_CC_OBJECT, 3);
	}	break;

	//TODO: case ASM_PUSH_BND_ARG:
	//TODO: case ASM16_PUSH_BND_ARG:
	//TODO: case ASM_PUSH_BND_EXTERN:
	//TODO: case ASM16_PUSH_BND_EXTERN:
	//TODO: case ASM_PUSH_BND_GLOBAL:
	//TODO: case ASM16_PUSH_BND_GLOBAL:
	//TODO: case ASM_PUSH_BND_LOCAL:
	//TODO: case ASM16_PUSH_BND_LOCAL:

	case ASM_JF:
	case ASM_JF16:
	case ASM_JT:
	case ASM_JT16:
	case ASM_JMP:
	case ASM_JMP16:
	case ASM32_JMP: {
		int temp;
		struct Dee_jump_descriptor *desc;
		struct Dee_basic_block *target;
do_jcc:
		desc = Dee_jump_descriptors_lookup(&self->fg_block->bb_exits, instr);
		ASSERTF(desc, "Jump at +%.4" PRFx32 " should have been found by the loader",
		        Dee_function_assembler_addrof(self->fg_assembler, instr));
		target = desc->jd_to;
		switch (opcode & 0xff) {
		case ASM_JF:
			temp = Dee_function_generator_vjf(self, target);
			break;
		case ASM_JT:
			temp = Dee_function_generator_vjt(self, target);
			break;
		case ASM_JMP:
		case ASM32_JMP: {
			/* Check for special case: if the jump happens at the end of the
			 * block (which it always should be due to the loader trimming
			 * anything that happens after a noreturn instruction), then don't
			 * generate any additional code but instead set the block's fallthru
			 * next-pointer to point at the target-block.
			 *
			 * That way, the jump will be generated during the block-stitching
			 * phase, at which point any additional memory transformation that
			 * might be necessary can be appended to the original block (while
			 * also allowing blocks to seamlessly flow into each other) */
			Dee_instruction_t const *after_jmp;
			after_jmp = DeeAsm_NextInstr(instr);
			if likely(after_jmp >= self->fg_block->bb_deemon_end) {
				self->fg_block->bb_next       = target;
				self->fg_block->bb_deemon_end = instr;  /* The jump doesn't exist anymore! */
				/* Get rid of the jump */
				Dee_jump_descriptors_remove(&self->fg_block->bb_exits, desc);
				Dee_jump_descriptors_remove(&target->bb_entries, desc);
				Dee_jump_descriptor_destroy(desc);
				return 0;
			}
			temp = Dee_function_generator_gjmp(self, target);
		}	break;

		default: __builtin_unreachable();
		}
		if unlikely(temp)
			goto err;

		/* Remember the memory-state as it is when the jump is made. */
		ASSERTF(!desc->jd_stat, "Who assigned this? Doing that is *my* job!");
		desc->jd_stat = self->fg_state;
		Dee_memstate_incref(desc->jd_stat);
		return 0;
	}	break;

	//TODO: case ASM_FOREACH:
	//TODO: case ASM_FOREACH16:
	//TODO: case ASM_JMP_POP:
	//TODO: case ASM_JMP_POP_POP:
	//TODO: case ASM_OPERATOR:
	//TODO: case ASM16_OPERATOR:
	//TODO: case ASM_OPERATOR_TUPLE:
	//TODO: case ASM16_OPERATOR_TUPLE:
	//TODO: case ASM_CALL:

	case ASM_CALL_TUPLE:
		return Dee_function_generator_vcallop(self, (void *)&DeeObject_CallTuple, VCALLOP_CC_OBJECT, 2);

	case ASM_DEL_GLOBAL:
		return Dee_function_generator_vdel_global(self, instr[1]);
	case ASM16_DEL_GLOBAL:
		return Dee_function_generator_vdel_global(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_SUPER:
		return Dee_function_generator_vcallop(self, (void *)&DeeSuper_New, VCALLOP_CC_OBJECT, 2);

	case ASM_SUPER_THIS_R: {
		uint16_t rid;
		DeeObject *reference;
		rid = instr[1];
		__IF0 { case ASM16_SUPER_THIS_R: rid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(rid >= self->fg_assembler->fa_code->co_refc)
			return err_illegal_rid(rid);
		reference = self->fg_assembler->fa_function->fo_refv[rid];
		if unlikely(Dee_function_generator_vpush_const(self, reference))
			goto err;
		if unlikely(Dee_function_generator_vpush_usage(self, REGISTER_USAGE_THIS))
			goto err;
		return Dee_function_generator_vcallop(self, (void *)&DeeSuper_New, VCALLOP_CC_OBJECT, 2);
	}	break;

	case ASM_POP_STATIC:
		return Dee_function_generator_vpop_static(self, instr[1]);
	case ASM16_POP_STATIC:
		return Dee_function_generator_vpop_static(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_POP_EXTERN:
		return Dee_function_generator_vpop_extern(self, instr[1], instr[2]);
	case ASM16_POP_EXTERN:
		return Dee_function_generator_vpop_extern(self, UNALIGNED_GETLE16(instr + 2), UNALIGNED_GETLE16(instr + 4));
	case ASM_POP_GLOBAL:
		return Dee_function_generator_vpop_global(self, instr[1]);
	case ASM16_POP_GLOBAL:
		return Dee_function_generator_vpop_global(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_PUSH_VARARGS:
		if (!(self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE)) {
			if unlikely(Dee_function_generator_vpush_usage(self, REGISTER_USAGE_ARGC))
				goto err;
			if unlikely(Dee_function_generator_vpush_usage(self, REGISTER_USAGE_ARGV))
				goto err;
			return Dee_function_generator_vcallop(self, (void *)&DeeTuple_NewVector, VCALLOP_CC_OBJECT, 2);
		}
		return Dee_function_generator_vpush_usage(self, REGISTER_USAGE_ARGS);

	case ASM_PUSH_VARKWDS:
		return Dee_function_generator_vpush_usage(self, REGISTER_USAGE_KW);

	case ASM_PUSH_STATIC:
		return Dee_function_generator_vpush_static(self, instr[1]);
	case ASM16_PUSH_STATIC:
		return Dee_function_generator_vpush_static(self, UNALIGNED_GETLE16(instr + 2));

	case ASM_CAST_TUPLE:
		return Dee_function_generator_vcallop(self, (void *)&DeeTuple_FromSequence, VCALLOP_CC_OBJECT, 1);
	case ASM_CAST_LIST:
		return Dee_function_generator_vcallop(self, (void *)&DeeList_FromSequence, VCALLOP_CC_OBJECT, 1);

	//TODO: case ASM_PACK_TUPLE:
	//TODO: case ASM16_PACK_TUPLE:
	//TODO: case ASM_PACK_LIST:
	//TODO: case ASM16_PACK_LIST:
	//TODO: case ASM_UNPACK:
	//TODO: case ASM16_UNPACK:
	//TODO: case ASM_CONCAT:
	//TODO: case ASM_EXTEND:
	//TODO: case ASM_TYPEOF:
	//TODO: case ASM_CLASSOF:
	//TODO: case ASM_SUPEROF:
	//TODO: case ASM_INSTANCEOF:
	//TODO: case ASM_IMPLEMENTS:

	case ASM_STR:
		return Dee_function_generator_vop(self, OPERATOR_STR, 1);
	case ASM_REPR:
		return Dee_function_generator_vop(self, OPERATOR_REPR, 1);
	case ASM_BOOL:
		return Dee_function_generator_vop(self, OPERATOR_BOOL, 1);

	//TODO: case ASM_NOT:

	case ASM_ASSIGN:
		return Dee_function_generator_vopv(self, OPERATOR_ASSIGN, 2);
	case ASM_MOVE_ASSIGN:
		return Dee_function_generator_vopv(self, OPERATOR_MOVEASSIGN, 2);

	case ASM_COPY:
		return Dee_function_generator_vop(self, OPERATOR_COPY, 1);
	case ASM_DEEPCOPY:
		return Dee_function_generator_vop(self, OPERATOR_DEEPCOPY, 1);
	case ASM_GETATTR:
		return Dee_function_generator_vop(self, OPERATOR_GETATTR, 2);
	case ASM_DELATTR:
		return Dee_function_generator_vopv(self, OPERATOR_DELATTR, 2);
	case ASM_SETATTR:
		return Dee_function_generator_vopv(self, OPERATOR_SETATTR, 3);

	//TODO: case ASM_BOUNDATTR:
	
	case ASM_GETATTR_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_GETATTR_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		return Dee_function_generator_vop(self, OPERATOR_GETATTR, 2);
	}	break;

	case ASM_DELATTR_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_DELATTR_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vop(self, OPERATOR_DELATTR, 2))
			goto err;
		return Dee_function_generator_vpop(self);
	}	break;

	case ASM_SETATTR_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_SETATTR_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		if unlikely(Dee_function_generator_vop(self, OPERATOR_SETATTR, 3))
			goto err;
		return Dee_function_generator_vpop(self);
	}	break;

	case ASM_GETATTR_THIS_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_GETATTR_THIS_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		if unlikely(Dee_function_generator_vpush_usage(self, REGISTER_USAGE_THIS))
			goto err;
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		return Dee_function_generator_vop(self, OPERATOR_GETATTR, 2);
	}	break;

	case ASM_DELATTR_THIS_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_DELATTR_THIS_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		if unlikely(Dee_function_generator_vpush_usage(self, REGISTER_USAGE_THIS))
			goto err;
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vop(self, OPERATOR_DELATTR, 2))
			goto err;
		return Dee_function_generator_vpop(self);
	}	break;

	case ASM_SETATTR_THIS_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_SETATTR_THIS_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		if unlikely(Dee_function_generator_vpush_usage(self, REGISTER_USAGE_THIS))
			goto err;
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vlrot(self, 3))
			goto err;
		if unlikely(Dee_function_generator_vop(self, OPERATOR_SETATTR, 3))
			goto err;
		return Dee_function_generator_vpop(self);
	}	break;

	case ASM_CMP_EQ:
		return Dee_function_generator_vop(self, OPERATOR_EQ, 1);
	case ASM_CMP_NE:
		return Dee_function_generator_vop(self, OPERATOR_NE, 1);
	case ASM_CMP_GE:
		return Dee_function_generator_vop(self, OPERATOR_GE, 1);
	case ASM_CMP_LO:
		return Dee_function_generator_vop(self, OPERATOR_LO, 1);
	case ASM_CMP_LE:
		return Dee_function_generator_vop(self, OPERATOR_LE, 1);
	case ASM_CMP_GR:
		return Dee_function_generator_vop(self, OPERATOR_GR, 1);

	case ASM_CLASS_C:      /* push class pop, const <imm8> */
	case ASM16_CLASS_C:    /* push class pop, const <imm8> */
	case ASM_CLASS_GC:     /* push class global <imm8>, const <imm8> */
	case ASM16_CLASS_GC:   /* push class global <imm8>, const <imm8> */
	case ASM_CLASS_EC:     /* push class extern <imm8>:<imm8>, const <imm8> */
	case ASM16_CLASS_EC: { /* push class extern <imm8>:<imm8>, const <imm8> */
		DeeObject *desc;
		uint16_t desc_cid;
		switch (opcode) {
		case ASM_CLASS_C:
			desc_cid = instr[1];
			break;
		case ASM16_CLASS_C:
			desc_cid = UNALIGNED_GETLE16(instr + 2);
			break;
		case ASM_CLASS_GC: {
			uint16_t base_gid;
			base_gid = instr[1];
			desc_cid = instr[2];
			__IF0 {
		case ASM16_CLASS_GC:
				base_gid = UNALIGNED_GETLE16(instr + 2);
				desc_cid = UNALIGNED_GETLE16(instr + 4);
			}
			if unlikely(Dee_function_generator_vpush_global(self, base_gid))
				goto err;
		}	break;
		case ASM_CLASS_EC: {
			uint16_t base_mid;
			uint16_t base_gid;
			base_mid = instr[1];
			base_gid = instr[2];
			desc_cid = instr[3];
			__IF0 {
		case ASM16_CLASS_EC:
				base_mid = UNALIGNED_GETLE16(instr + 2);
				base_gid = UNALIGNED_GETLE16(instr + 4);
				desc_cid = UNALIGNED_GETLE16(instr + 6);
			}
			if unlikely(Dee_function_generator_vpush_extern(self, base_mid, base_gid))
				goto err;
		}	break;
		default: __builtin_unreachable();
		}
		if unlikely(desc_cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(desc_cid);
		desc = self->fg_assembler->fa_code->co_staticv[desc_cid];
		if unlikely(Dee_function_generator_vpush_const(self, desc))
			goto err;
		ATTR_FALLTHROUGH
	case ASM_CLASS:
		if unlikely(Dee_function_generator_vpush_const(self, (DeeObject *)self->fg_assembler->fa_code->co_module))
			goto err;
		if unlikely(Dee_function_generator_vcallop(self, (void *)&DeeClass_New, VCALLOP_CC_OBJECT, 3))
			goto err;
	}	break;

	//TODO: case ASM_DEFCMEMBER:
	//TODO: case ASM16_DEFCMEMBER:
	//TODO: case ASM_GETCMEMBER_R:
	//TODO: case ASM16_GETCMEMBER:
	//TODO: case ASM16_GETCMEMBER_R:
	//TODO: case ASM_CALLCMEMBER_THIS_R:
	//TODO: case ASM16_CALLCMEMBER_THIS_R:

	//TODO: case ASM_FUNCTION_C:
	//TODO: case ASM16_FUNCTION_C:
	//TODO: case ASM_FUNCTION_C_16:
	//TODO: case ASM16_FUNCTION_C_16:


	case ASM_CAST_INT:
		return Dee_function_generator_vop(self, OPERATOR_INT, 1);
	case ASM_INV:
		return Dee_function_generator_vop(self, OPERATOR_INV, 1);
	case ASM_POS:
		return Dee_function_generator_vop(self, OPERATOR_POS, 1);
	case ASM_NEG:
		return Dee_function_generator_vop(self, OPERATOR_NEG, 1);
	case ASM_ADD:
		return Dee_function_generator_vop(self, OPERATOR_ADD, 2);
	case ASM_SUB:
		return Dee_function_generator_vop(self, OPERATOR_SUB, 2);
	case ASM_MUL:
		return Dee_function_generator_vop(self, OPERATOR_MUL, 2);
	case ASM_DIV:
		return Dee_function_generator_vop(self, OPERATOR_DIV, 2);
	case ASM_MOD:
		return Dee_function_generator_vop(self, OPERATOR_MOD, 2);
	case ASM_SHL:
		return Dee_function_generator_vop(self, OPERATOR_SHL, 2);
	case ASM_SHR:
		return Dee_function_generator_vop(self, OPERATOR_SHR, 2);
	case ASM_AND:
		return Dee_function_generator_vop(self, OPERATOR_AND, 2);
	case ASM_OR:
		return Dee_function_generator_vop(self, OPERATOR_OR, 2);
	case ASM_XOR:
		return Dee_function_generator_vop(self, OPERATOR_XOR, 2);
	case ASM_POW:
		return Dee_function_generator_vop(self, OPERATOR_POW, 2);

	//TODO: case ASM_ADD_SIMM8:
	//TODO: case ASM_ADD_IMM32:
	//TODO: case ASM_SUB_SIMM8:
	//TODO: case ASM_SUB_IMM32:
	//TODO: case ASM_MUL_SIMM8:
	//TODO: case ASM_DIV_SIMM8:
	//TODO: case ASM_MOD_SIMM8:
	//TODO: case ASM_SHL_IMM8:
	//TODO: case ASM_SHR_IMM8:
	//TODO: case ASM_AND_IMM32:
	//TODO: case ASM_OR_IMM32:
	//TODO: case ASM_XOR_IMM32:
	//TODO: case ASM_ISNONE:
	//TODO: case ASM_PRINT:
	//TODO: case ASM_PRINT_SP:
	//TODO: case ASM_PRINT_NL:
	//TODO: case ASM_PRINTNL:
	//TODO: case ASM_PRINTALL:
	//TODO: case ASM_PRINTALL_SP:
	//TODO: case ASM_PRINTALL_NL:
	//TODO: case ASM_FPRINT:
	//TODO: case ASM_FPRINT_SP:
	//TODO: case ASM_FPRINT_NL:
	//TODO: case ASM_FPRINTNL:
	//TODO: case ASM_FPRINTALL:
	//TODO: case ASM_FPRINTALL_SP:
	//TODO: case ASM_FPRINTALL_NL:
	//TODO: case ASM_PRINT_C:
	//TODO: case ASM_PRINT_C_SP:
	//TODO: case ASM_PRINT_C_NL:
	//TODO: case ASM_RANGE_0_I16:

	case ASM_ENTER:
		if unlikely(Dee_function_generator_vdup(self))
			goto err;
		return Dee_function_generator_vopv(self, OPERATOR_ENTER, 1);
	case ASM_LEAVE:
		return Dee_function_generator_vopv(self, OPERATOR_LEAVE, 1);

	//TODO: case ASM_FPRINT_C:
	//TODO: case ASM_FPRINT_C_SP:
	//TODO: case ASM_FPRINT_C_NL:
	//TODO: case ASM_RANGE:
	//TODO: case ASM_RANGE_DEF:
	//TODO: case ASM_RANGE_STEP:
	//TODO: case ASM_RANGE_STEP_DEF:

	case ASM_CONTAINS:
		return Dee_function_generator_vop(self, OPERATOR_CONTAINS, 2);

	case ASM_CONTAINS_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_CONTAINS_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		return Dee_function_generator_vop(self, OPERATOR_CONTAINS, 2);
	}	break;

	case ASM_GETITEM:
		return Dee_function_generator_vop(self, OPERATOR_GETITEM, 2);

	//TODO: case ASM_GETITEM_I:

	case ASM_GETITEM_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_GETITEM_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		return Dee_function_generator_vop(self, OPERATOR_GETITEM, 2);
	}	break;

	case ASM_GETSIZE:
		return Dee_function_generator_vop(self, OPERATOR_SIZE, 2);

	//TODO: case ASM_SETITEM_I:

	case ASM_SETITEM_C: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_SETITEM_C: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		if unlikely(Dee_function_generator_vop(self, OPERATOR_SETITEM, 3))
			goto err;
		return Dee_function_generator_vpop(self);
	}	break;

	case ASM_ITERSELF:
		return Dee_function_generator_vop(self, OPERATOR_ITERSELF, 1);
	case ASM_DELITEM:
		return Dee_function_generator_vopv(self, OPERATOR_DELITEM, 2);
	case ASM_SETITEM:
		return Dee_function_generator_vopv(self, OPERATOR_SETITEM, 3);
	case ASM_GETRANGE:
		return Dee_function_generator_vop(self, OPERATOR_GETRANGE, 3);
	case ASM_GETRANGE_PN:
		if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
			goto err;
		return Dee_function_generator_vop(self, OPERATOR_GETRANGE, 3);
	case ASM_GETRANGE_NP:
		if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		return Dee_function_generator_vop(self, OPERATOR_GETRANGE, 3);

	//TODO: case ASM_GETRANGE_PI:
	//TODO: case ASM_GETRANGE_IP:
	//TODO: case ASM_GETRANGE_NI:
	//TODO: case ASM_GETRANGE_IN:
	//TODO: case ASM_GETRANGE_II:

	case ASM_DELRANGE:
		return Dee_function_generator_vopv(self, OPERATOR_DELRANGE, 3);
	case ASM_SETRANGE:
		return Dee_function_generator_vopv(self, OPERATOR_SETRANGE, 4);
	case ASM_SETRANGE_PN:
		if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
			goto err;
		if unlikely(Dee_function_generator_vop(self, OPERATOR_SETRANGE, 3))
			goto err;
		return Dee_function_generator_vpop(self);
	case ASM_SETRANGE_NP:
		if unlikely(Dee_function_generator_vpush_const(self, Dee_None))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		if unlikely(Dee_function_generator_vop(self, OPERATOR_SETRANGE, 3))
			goto err;
		return Dee_function_generator_vpop(self);

	//TODO: case ASM_SETRANGE_PI:
	//TODO: case ASM_SETRANGE_IP:
	//TODO: case ASM_SETRANGE_NI:
	//TODO: case ASM_SETRANGE_IN:
	//TODO: case ASM_SETRANGE_II:

	//TODO: case ASM_BREAKPOINT:
	//TODO: case ASM_UD:

	//TODO: case ASM_CALLATTR_C_KW:
	//TODO: case ASM16_CALLATTR_C_KW:

	case ASM_CALLATTR_C_TUPLE_KW: {
		uint16_t args_cid;
		uint16_t kwds_cid;
		DeeObject *constant;
		args_cid = instr[1];
		kwds_cid = instr[2];
		__IF0 {
	case ASM16_CALLATTR_C_TUPLE_KW:
			args_cid = UNALIGNED_GETLE16(instr + 2);
			kwds_cid = UNALIGNED_GETLE16(instr + 4);
		}
		if unlikely(args_cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(args_cid);
		if unlikely(kwds_cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(kwds_cid);
		constant = self->fg_assembler->fa_code->co_staticv[args_cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		constant = self->fg_assembler->fa_code->co_staticv[kwds_cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		ATTR_FALLTHROUGH
	case ASM_CALLATTR_TUPLE_KWDS:
		return Dee_function_generator_vcallop(self, (void *)&DeeObject_CallAttrTupleKw, VCALLOP_CC_OBJECT, 4);
	}	break;

	//TODO: case ASM_CALLATTR:
	//TODO: case ASM_CALLATTR_C:
	//TODO: case ASM16_CALLATTR_C:

	case ASM_CALLATTR_C_TUPLE: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_CALLATTR_C_TUPLE: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		ATTR_FALLTHROUGH
	case ASM_CALLATTR_TUPLE:
		return Dee_function_generator_vcallop(self, (void *)&DeeObject_CallAttrTuple, VCALLOP_CC_OBJECT, 3);
	}	break;

	//TODO: case ASM_CALLATTR_THIS_C:
	//TODO: case ASM16_CALLATTR_THIS_C:

	case ASM_CALLATTR_THIS_C_TUPLE: {
		uint16_t cid;
		DeeObject *constant;
		cid = instr[1];
		__IF0 { case ASM16_CALLATTR_THIS_C_TUPLE: cid = UNALIGNED_GETLE16(instr + 2); }
		if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
			return err_illegal_cid(cid);
		if unlikely(Dee_function_generator_vpush_usage(self, REGISTER_USAGE_THIS))
			goto err;
		constant = self->fg_assembler->fa_code->co_staticv[cid];
		if unlikely(Dee_function_generator_vpush_const(self, constant))
			goto err;
		if unlikely(Dee_function_generator_vlrot(self, 3))
			goto err;
		ATTR_FALLTHROUGH
		return Dee_function_generator_vcallop(self, (void *)&DeeObject_CallAttrTuple, VCALLOP_CC_OBJECT, 3);
	}	break;

	//TODO: case ASM_CALLATTR_C_SEQ:
	//TODO: case ASM16_CALLATTR_C_SEQ:
	//TODO: case ASM_CALLATTR_C_MAP:
	//TODO: case ASM16_CALLATTR_C_MAP:

	//TODO: case ASM_CALLATTR_KWDS:

	//TODO: case ASM_GETMEMBER_THIS_R:
	//TODO: case ASM16_GETMEMBER_THIS_R:
	//TODO: case ASM_DELMEMBER_THIS_R:
	//TODO: case ASM16_DELMEMBER_THIS_R:
	//TODO: case ASM_SETMEMBER_THIS_R:
	//TODO: case ASM16_SETMEMBER_THIS_R:
	//TODO: case ASM_BOUNDMEMBER_THIS_R:
	//TODO: case ASM16_BOUNDMEMBER_THIS_R:
	//TODO: case ASM_CALL_EXTERN:
	//TODO: case ASM16_CALL_EXTERN:
	//TODO: case ASM_CALL_GLOBAL:
	//TODO: case ASM16_CALL_GLOBAL:
	//TODO: case ASM_CALL_LOCAL:
	//TODO: case ASM16_CALL_LOCAL:

	//TODO: case ASM_CALL_SEQ:
	//TODO: case ASM_CALL_MAP:

	case ASM_THISCALL_TUPLE:
		return Dee_function_generator_vcallop(self, (void *)&DeeObject_ThisCallTuple, VCALLOP_CC_OBJECT, 3);
	case ASM_CALL_TUPLE_KWDS:
		return Dee_function_generator_vcallop(self, (void *)&DeeObject_CallTupleKw, VCALLOP_CC_OBJECT, 3);

	//TODO: case ASM_PUSH_EXCEPT:

	case ASM_PUSH_THIS:
		return Dee_function_generator_vpush_usage(self, REGISTER_USAGE_THIS);
	case ASM_CAST_HASHSET:
		return Dee_function_generator_vcallop(self, (void *)&DeeHashSet_FromSequence, VCALLOP_CC_OBJECT, 1);
	case ASM_CAST_DICT:
		return Dee_function_generator_vcallop(self, (void *)&DeeDict_FromSequence, VCALLOP_CC_OBJECT, 1);

	case ASM_PUSH_TRUE:
		return Dee_function_generator_vpush_const(self, Dee_True);
	case ASM_PUSH_FALSE:
		return Dee_function_generator_vpush_const(self, Dee_False);

	//TODO: case ASM_PACK_HASHSET:
	//TODO: case ASM16_PACK_HASHSET:
	//TODO: case ASM_PACK_DICT:
	//TODO: case ASM16_PACK_DICT:
	//TODO: case ASM_BOUNDITEM:
	//TODO: case ASM_CMP_SO:
	//TODO: case ASM_CMP_DO:
	
	//TODO: case ASM_SUPERGETATTR_THIS_RC:
	//TODO: case ASM16_SUPERGETATTR_THIS_RC:
	//TODO: case ASM_SUPERCALLATTR_THIS_RC:
	//TODO: case ASM16_SUPERCALLATTR_THIS_RC:
	//TODO: case ASM_INCPOST:
	//TODO: case ASM_DECPOST:

	case ASM_REDUCE_MIN:
		if unlikely(Dee_function_generator_vpush_addr(self, NULL))
			goto err;
		return Dee_function_generator_vcallop(self, (void *)&DeeSeq_Min, VCALLOP_CC_OBJECT, 2);
	case ASM_REDUCE_MAX:
		if unlikely(Dee_function_generator_vpush_addr(self, NULL))
			goto err;
		return Dee_function_generator_vcallop(self, (void *)&DeeSeq_Max, VCALLOP_CC_OBJECT, 2);
	case ASM_REDUCE_SUM:
		return Dee_function_generator_vcallop(self, (void *)&DeeSeq_Sum, VCALLOP_CC_OBJECT, 1);

	//TODO: case ASM_REDUCE_ANY:
	//TODO: case ASM_REDUCE_ALL:
	//TODO: case ASM16_PRINT_C:
	//TODO: case ASM16_PRINT_C_SP:
	//TODO: case ASM16_PRINT_C_NL:
	//TODO: case ASM_RANGE_0_I32:
	//TODO: case ASM_VARARGS_UNPACK:
	//TODO: case ASM_PUSH_VARKWDS_NE:
	//TODO: case ASM16_FPRINT_C:
	//TODO: case ASM16_FPRINT_C_SP:
	//TODO: case ASM16_FPRINT_C_NL:
	//TODO: case ASM_VARARGS_CMP_EQ_SZ:
	//TODO: case ASM_VARARGS_CMP_GR_SZ:
	//TODO: case ASM_VARARGS_GETITEM:
	//TODO: case ASM_VARARGS_GETITEM_I:
	//TODO: case ASM16_GETITEM_C:
	//TODO: case ASM_VARARGS_GETSIZE:
	//TODO: case ASM16_SETITEM_C:
	//TODO: case ASM_ITERNEXT:
	//TODO: case ASM_GETMEMBER:
	//TODO: case ASM16_GETMEMBER:
	//TODO: case ASM_DELMEMBER:
	//TODO: case ASM16_DELMEMBER:
	//TODO: case ASM_SETMEMBER:
	//TODO: case ASM16_SETMEMBER:
	//TODO: case ASM_BOUNDMEMBER:
	//TODO: case ASM16_BOUNDMEMBER:
	//TODO: case ASM_GETMEMBER_THIS:
	//TODO: case ASM16_GETMEMBER_THIS:
	//TODO: case ASM_DELMEMBER_THIS:
	//TODO: case ASM16_DELMEMBER_THIS:
	//TODO: case ASM_SETMEMBER_THIS:
	//TODO: case ASM16_SETMEMBER_THIS:
	//TODO: case ASM_BOUNDMEMBER_THIS:
	//TODO: case ASM16_BOUNDMEMBER_THIS:



		/* TODO: Instruction prefixes. */
	case ASM_STACK:
	case ASM16_STACK:
	case ASM_STATIC:
	case ASM16_STATIC:
	case ASM_EXTERN:
	case ASM16_EXTERN:
	case ASM_GLOBAL:
	case ASM16_GLOBAL:
	case ASM_LOCAL:
	case ASM16_LOCAL: {
		Dee_instruction_t const *prefix_instr;
		uint8_t prefix_type = opcode & 0xff;
		uint16_t prefix_opcode;
		uint16_t id1 = instr[1];
		uint16_t id2 = instr[2];
		if (opcode & 0xff00) {
			id1 = UNALIGNED_GETLE16(instr + 2);
			id2 = UNALIGNED_GETLE16(instr + 4);
		}
		switch (prefix_type) {
		case ASM_STACK:
			if unlikely(id1 >= self->fg_state->ms_stackc)
				return err_illegal_stack_effect();
			break;
		case ASM_STATIC:
			if unlikely(id1 >= self->fg_assembler->fa_code->co_staticc)
				return err_illegal_sid(id1);
			break;
		case ASM_EXTERN: {
			DeeModuleObject *mod;
			mod = self->fg_assembler->fa_code->co_module;
			if unlikely(id1 >= mod->mo_importc)
				return err_illegal_sid(id1);
			mod = mod->mo_importv[id1];
			if unlikely(id2 >= mod->mo_globalc)
				return err_illegal_gid(mod, id2);
		}	break;
		case ASM_GLOBAL: {
			DeeModuleObject *mod;
			mod = self->fg_assembler->fa_code->co_module;
			if unlikely(id1 >= mod->mo_globalc)
				return err_illegal_gid(mod, id1);
		}	break;
		case ASM_LOCAL:
			if unlikely(id1 >= self->fg_state->ms_localc)
				return err_illegal_lid(id1);
			break;
		default: __builtin_unreachable();
		}
		prefix_instr  = DeeAsm_SkipPrefix(instr);
		prefix_opcode = prefix_instr[0];
		if (ASM_ISEXTENDED(prefix_opcode))
			prefix_opcode = (prefix_opcode << 8) | prefix_instr[1];
		switch (prefix_opcode) {

		case ASM_JF:   /* jf PREFIX, <Sdisp8> */
		case ASM_JF16: /* jf PREFIX, <Sdisp16> */
		case ASM_JT:   /* jt PREFIX, <Sdisp8> */
		case ASM_JT16: /* jt PREFIX, <Sdisp16> */
			if unlikely(Dee_function_generator_vpush_prefix(self, prefix_type, id1, id2))
				goto err;
			opcode = prefix_opcode;
			/* Need to do this in a special way because `instr' must not become `prefix_instr' here. */
			goto do_jcc;

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
			if unlikely(Dee_function_generator_vpush_prefix(self, prefix_type, id1, id2))
				goto err;
			return Dee_function_generator_geninstr(self, prefix_instr);

		case ASM_FOREACH:            /* foreach PREFIX, <Sdisp8> */
		case ASM_FOREACH16:          /* foreach PREFIX, <Sdisp16> */
		case ASM_DUP:                /* mov PREFIX, top', `mov PREFIX, #SP - 1 */
		case ASM_DUP_N:              /* mov PREFIX, #SP - <imm8> - 2 */
		case ASM16_DUP_N:            /* mov PREFIX, #SP - <imm16> - 2 */
		case ASM_POP:                /* mov top, PREFIX */
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
		case ASM_PUSH_FALSE:         /* mov  PREFIX, false */
			if unlikely(Dee_function_generator_geninstr(self, prefix_instr))
				goto err;
			return Dee_function_generator_vpop_prefix(self, prefix_type, id1, id2);


		//TODO: case ASM_POP_N: /* mov #SP - <imm8> - 2, PREFIX */
		//TODO: case ASM16_POP_N: /* mov #SP - <imm16> - 2, PREFIX */
		//TODO: case ASM_OPERATOR: /* PREFIX: push op $<imm8>, #<imm8> */
		//TODO: case ASM_OPERATOR_TUPLE: /* PREFIX: push op $<imm8>, pop... */
		//TODO: case ASM_SWAP: /* swap top, PREFIX */
		//TODO: case ASM_LROT: /* lrot #<imm8>+2, PREFIX */
		//TODO: case ASM_RROT: /* rrot #<imm8>+2, PREFIX */
		//TODO: case ASM16_LROT: /* lrot #<imm16>+2, PREFIX */
		//TODO: case ASM16_RROT: /* rrot #<imm16>+2, PREFIX */

		//TODO: case ASM_ADD: /* add PREFIX, pop */
		//TODO: case ASM_SUB: /* sub PREFIX, pop */
		//TODO: case ASM_MUL: /* mul PREFIX, pop */
		//TODO: case ASM_DIV: /* div PREFIX, pop */
		//TODO: case ASM_MOD: /* mod PREFIX, pop */
		//TODO: case ASM_SHL: /* shl PREFIX, pop */
		//TODO: case ASM_SHR: /* shr PREFIX, pop */
		//TODO: case ASM_AND: /* and PREFIX, pop */
		//TODO: case ASM_OR: /* or PREFIX, pop */
		//TODO: case ASM_XOR: /* xor PREFIX, pop */
		//TODO: case ASM_POW: /* pow PREFIX, pop */
		//TODO: case ASM_INC: /* inc PREFIX */
		//TODO: case ASM_DEC: /* dec PREFIX */
		//TODO: case ASM_ADD_SIMM8: /* add PREFIX, $<Simm8> */
		//TODO: case ASM_ADD_IMM32: /* add PREFIX, $<imm32> */
		//TODO: case ASM_SUB_SIMM8: /* sub PREFIX, $<Simm8> */
		//TODO: case ASM_SUB_IMM32: /* sub PREFIX, $<imm32> */
		//TODO: case ASM_MUL_SIMM8: /* mul PREFIX, $<Simm8> */
		//TODO: case ASM_DIV_SIMM8: /* div PREFIX, $<Simm8> */
		//TODO: case ASM_MOD_SIMM8: /* mod PREFIX, $<Simm8> */
		//TODO: case ASM_SHL_IMM8: /* shl PREFIX, $<Simm8> */
		//TODO: case ASM_SHR_IMM8: /* shr PREFIX, $<Simm8> */
		//TODO: case ASM_AND_IMM32: /* and PREFIX, $<imm32> */
		//TODO: case ASM_OR_IMM32: /* or PREFIX, $<imm32> */
		//TODO: case ASM_XOR_IMM32: /* xor PREFIX, $<imm32> */

		case ASM_DELOP:
		case ASM16_DELOP:
		case ASM_NOP:   /* nop PREFIX */
		case ASM16_NOP: /* nop16 PREFIX' - `PREFIX: nop16 */
			break;

		//TODO: case ASM16_OPERATOR: /* PREFIX: push op $<imm16>, #<imm8> */
		//TODO: case ASM16_OPERATOR_TUPLE: /* PREFIX: push op $<imm16>, pop */

		//TODO: case ASM_INCPOST: /* push inc PREFIX' - `PREFIX: push inc */
		//TODO: case ASM_DECPOST: /* push dec PREFIX' - `PREFIX: push dec */

		default:
			DeeError_Throwf(&DeeError_IllegalInstruction,
			                "Opcode not supported: %#.2" PRFx16 ":%#.2" PRFx16,
			                opcode, prefix_opcode);
			goto err;
		}
	}	break;

	default:
		DeeError_Throwf(&DeeError_IllegalInstruction,
		                "Opcode not supported: %#.2" PRFx16,
		                opcode);
		goto err;
	}
	return 0;
err:
	return -1;
}


/* Wrapper around `Dee_function_generator_geninstr()' to generate the entire basic block. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_genall(struct Dee_function_generator *__restrict self) {
	struct Dee_basic_block *block = self->fg_block;
	Dee_instruction_t const *instr;
	ASSERT(block->bb_mem_start != NULL);
	ASSERT(block->bb_mem_end == NULL);

	/* Generate text. */
	for (instr = block->bb_deemon_start;
	     instr < block->bb_deemon_end;
	     instr = DeeAsm_NextInstr(instr)) {
		if unlikely(Dee_function_generator_geninstr(self, instr))
			goto err;
	}

	/* Assign the final memory state. */
	block->bb_mem_end = self->fg_state;
	Dee_memstate_incref(self->fg_state);
	return 0;
err:
	return -1;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_COMMON_C */
