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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C
#define GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/alloc.h>
#include <deemon/asm.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/tuple.h>

DECL_BEGIN

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
Dee_function_generator_vlrot(struct Dee_function_generator *__restrict self,
                             Dee_vstackaddr_t n) {
	int result;
	if unlikely(n <= 1)
		return 0;
	result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vlrot(self->fg_state, n);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vrrot(struct Dee_function_generator *__restrict self,
                             Dee_vstackaddr_t n) {
	int result;
	if unlikely(n <= 1)
		return 0;
	result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vrrot(self->fg_state, n);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpush(struct Dee_function_generator *__restrict self,
                             struct Dee_memloc *loc) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush(self->fg_state, loc);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_undefined(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_undefined(self->fg_state);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_const(struct Dee_function_generator *__restrict self,
                                   DeeObject *value) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_const(self->fg_state, value);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_cid(struct Dee_function_generator *__restrict self,
                                 uint16_t cid) {
	DeeObject *constant;
	if unlikely(cid >= self->fg_assembler->fa_code->co_staticc)
		return err_illegal_cid(cid);
	constant = self->fg_assembler->fa_code->co_staticv[cid];
	return Dee_function_generator_vpush_const(self, constant);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_rid(struct Dee_function_generator *__restrict self,
                                 uint16_t rid) {
	DeeObject *constant;
	if unlikely(rid >= self->fg_assembler->fa_code->co_refc)
		return err_illegal_rid(rid);
	constant = self->fg_assembler->fa_function->fo_refv[rid];
	return Dee_function_generator_vpush_const(self, constant);
}


/* Sets the `MEMLOC_F_NOREF' flag */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_reg(struct Dee_function_generator *__restrict self,
                                 Dee_host_register_t regno, ptrdiff_t delta) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_reg(self->fg_state, regno, delta);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_regind(struct Dee_function_generator *__restrict self,
                                    Dee_host_register_t regno, ptrdiff_t ind_delta,
                                    ptrdiff_t val_delta) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_regind(self->fg_state, regno, ind_delta, val_delta);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_hstack(struct Dee_function_generator *__restrict self,
                                    uintptr_t cfa_offset) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_hstack(self->fg_state, cfa_offset);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_hstackind(struct Dee_function_generator *__restrict self,
                                       uintptr_t cfa_offset, ptrdiff_t val_delta) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_hstackind(self->fg_state, cfa_offset, val_delta);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vinit_unbound_arg(struct Dee_function_generator *__restrict self,
                                         Dee_instruction_t const *instr, uint16_t aid) {
	DREF struct Dee_memstate *common_state;
	DeeCodeObject *code = self->fg_assembler->fa_code;
	uint16_t opt_aid = aid - code->co_argc_min;
	size_t xlid = DEE_MEMSTATE_EXTRA_LOCAL_DEFARG(opt_aid);
	size_t lid  = self->fg_assembler->fa_localc + xlid;
	struct Dee_memloc *loc;
	DeeObject *default_value;

	/* Lazily initialize the extra-local value if not unconditionally bound */
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	loc = &self->fg_state->ms_localv[lid];
	ASSERT(!(loc->ml_flags & MEMLOC_F_LOCAL_BOUND));
	default_value = self->fg_assembler->fa_code->co_defaultv[opt_aid];
	if (default_value) {
		struct Dee_host_symbol *use_default_sym;

		/* Load the default value into a register and into the local */
		if unlikely(Dee_function_generator_vpush_const(self, default_value))
			goto err; /* default_value */
		if unlikely(Dee_function_generator_vreg(self, NULL))
			goto err; /* reg:default_value */
		if unlikely(Dee_function_generator_vpop_local(self, lid))
			goto err; /* - */

		/* Check if the caller has provided enough arguments. */
		if unlikely(Dee_function_generator_vpush_argc(self))
			goto err; /* argc */
		use_default_sym = Dee_function_generator_newsym(self);
		if unlikely(!use_default_sym)
			goto err;
		{
			struct Dee_memloc l_argc, l_aid;
			l_argc = *Dee_function_generator_vtop(self);
			ASSERT(l_argc.ml_flags & MEMLOC_F_NOREF);
			--self->fg_state->ms_stackc;
			if (MEMLOC_TYPE_HASREG(l_argc.ml_type))
				Dee_memstate_decrinuse(self->fg_state, l_argc.ml_value.v_hreg.r_regno);
			l_aid.ml_type          = MEMLOC_TYPE_CONST;
			l_aid.ml_value.v_const = (DeeObject *)(uintptr_t)aid;
			if unlikely(_Dee_function_generator_gjcmp(self, &l_aid, &l_argc, false, NULL,
			                                          use_default_sym, use_default_sym))
				goto err;
		}
		common_state = self->fg_state;
		Dee_memstate_incref(common_state);
		if unlikely(Dee_function_generator_vpush_argv(self))
			goto err_common_state; /* argv */
		if unlikely(Dee_function_generator_vind(self, (ptrdiff_t)aid * sizeof(DeeObject *)))
			goto err_common_state; /* argv[aid] */
		if unlikely(Dee_function_generator_vpop_local(self, lid))
			goto err_common_state; /* <arg> = argv[aid] */
		if unlikely(Dee_function_generator_vmorph(self, common_state))
			goto err_common_state; /* <arg> = argv[aid] */
		Dee_memstate_decref(common_state);
		Dee_host_symbol_setsect(use_default_sym, self->fg_sect);
	} else {
		struct Dee_host_section *text_sect;
		struct Dee_host_section *cold_sect;
		struct Dee_host_symbol *target_sym;
		struct Dee_memloc l_argc, l_aid;
		target_sym = Dee_function_generator_newsym(self);
		if unlikely(!target_sym)
			goto err;
		text_sect = self->fg_sect;
		cold_sect = text_sect;
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE))
			cold_sect = &self->fg_block->bb_hcold;

		if unlikely(Dee_function_generator_vpush_argc(self))
			goto err;
		l_argc = *Dee_function_generator_vtop(self);
		ASSERT(l_argc.ml_flags & MEMLOC_F_NOREF);
		--self->fg_state->ms_stackc;
		if (MEMLOC_TYPE_HASREG(l_argc.ml_type))
			Dee_memstate_decrinuse(self->fg_state, l_argc.ml_value.v_hreg.r_regno);
		l_aid.ml_type          = MEMLOC_TYPE_CONST;
		l_aid.ml_value.v_const = (DeeObject *)(uintptr_t)aid;
		if unlikely(_Dee_function_generator_gjcmp(self, &l_aid, &l_argc, false,
		                                          text_sect != cold_sect ? NULL : target_sym,
		                                          text_sect != cold_sect ? target_sym : NULL,
		                                          text_sect != cold_sect ? target_sym : NULL))
			goto err;
		self->fg_sect = cold_sect;
		if (text_sect != cold_sect)
			Dee_host_symbol_setsect(target_sym, cold_sect);
		if unlikely(Dee_function_generator_gthrow_arg_unbound(self, instr, aid))
			goto err;
		if (text_sect == cold_sect)
			Dee_host_symbol_setsect(target_sym, self->fg_sect);
		self->fg_sect = text_sect;
		if unlikely(Dee_function_generator_vpush_argv(self))
			goto err; /* argv */
		if unlikely(Dee_function_generator_vind(self, (ptrdiff_t)aid * sizeof(DeeObject *)))
			goto err; /* argv[aid] */
		loc = &self->fg_state->ms_localv[lid];
		if (MEMLOC_TYPE_HASREG(loc->ml_type))
			Dee_memstate_decrinuse(self->fg_state, loc->ml_value.v_hreg.r_regno);
		*loc = *Dee_function_generator_vtop(self);
		ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
		--self->fg_state->ms_stackc;
		loc->ml_flags &= ~MEMLOC_M_LOCAL_BSTATE;
		loc->ml_flags |= MEMLOC_F_LOCAL_BOUND;
	}
	return 0;
err_common_state:
	Dee_memstate_decref(common_state);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_arg(struct Dee_function_generator *__restrict self,
                                 Dee_instruction_t const *instr, uint16_t aid) {
	DREF struct Dee_memstate *common_state;
	DeeCodeObject *code = self->fg_assembler->fa_code;
	if (aid < code->co_argc_min) /* Simple case: mandatory argument */
		return Dee_function_generator_vpush_arg_present(self, aid);
	if (aid < code->co_argc_max) { /* Special case: optional argument */
		uint16_t opt_aid = aid - code->co_argc_min;
		size_t xlid = DEE_MEMSTATE_EXTRA_LOCAL_DEFARG(opt_aid);
		size_t lid  = self->fg_assembler->fa_localc + xlid;
		struct Dee_memloc *loc = &self->fg_state->ms_localv[lid];
		if (!(loc->ml_flags & MEMLOC_F_LOCAL_BOUND)) {
			if (loc->ml_flags & MEMLOC_F_LOCAL_UNBOUND) {
				if unlikely(Dee_function_generator_vinit_unbound_arg(self, instr, aid))
					goto err;
			} else {
				/* Argument is conditionally bound -> this can happen
				 * if it conditionally bound via keyword arguments in
				 * the function prolog.
				 *
				 * In this case, must generate a conditional initialization. */
				struct Dee_host_symbol *skipinit;
				if (loc->ml_type != MEMLOC_TYPE_HSTACKIND &&
				    loc->ml_type != MEMLOC_TYPE_HREG) {
					if unlikely(Dee_function_generator_greg(self, loc, NULL))
						goto err;
					ASSERT(loc->ml_type == MEMLOC_TYPE_HREG);
				}
				skipinit = Dee_function_generator_newsym(self);
				if unlikely(!skipinit)
					goto err;
				if unlikely(_Dee_function_generator_gjnz(self, loc, skipinit))
					goto err;
				common_state = self->fg_state;
				Dee_memstate_incref(common_state);
				if unlikely(Dee_function_generator_vinit_unbound_arg(self, instr, aid))
					goto err_common_state;
				if unlikely(Dee_function_generator_vmorph(self, common_state))
					goto err_common_state;
				Dee_memstate_decref(common_state);
				Dee_host_symbol_setsect(skipinit, self->fg_sect);
			}
			loc = &self->fg_state->ms_localv[lid];
		}
		ASSERT((loc->ml_flags & MEMLOC_M_LOCAL_BSTATE) == MEMLOC_F_LOCAL_BOUND);
		return Dee_function_generator_vpush(self, loc);
	}
	return err_illegal_aid(aid);
err_common_state:
	Dee_memstate_decref(common_state);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_arg_present(struct Dee_function_generator *__restrict self, uint16_t aid) {
	int result;
	ASSERT(aid < self->fg_assembler->fa_code->co_argc_min);
	result = Dee_function_generator_state_unshare(self);
	if likely(result == 0) {
		STATIC_ASSERT(DEE_MEMSTATE_EXTRA_LOCAL_A_ARGS == DEE_MEMSTATE_EXTRA_LOCAL_A_ARGV);
		struct Dee_memloc *args_or_argv_loc;
		ptrdiff_t ind_offset = (ptrdiff_t)aid * sizeof(DeeObject *);
		if (self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE)
			ind_offset += offsetof(DeeTupleObject, t_elem);
		args_or_argv_loc = &self->fg_state->ms_localv[self->fg_assembler->fa_localc +
		                                              DEE_MEMSTATE_EXTRA_LOCAL_A_ARGV];
		if unlikely(Dee_function_generator_greg(self, args_or_argv_loc, NULL))
			goto err;
		ASSERT(args_or_argv_loc->ml_type == MEMLOC_TYPE_HREG);
		result = Dee_memstate_vpush_regind(self->fg_state,
		                                   args_or_argv_loc->ml_value.v_hreg.r_regno,
		                                   args_or_argv_loc->ml_value.v_hreg.r_off + ind_offset,
		                                   0);
	}
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_local(struct Dee_function_generator *__restrict self,
                                   Dee_instruction_t const *instr, size_t lid) {
	struct Dee_memstate *state;
	struct Dee_memloc *dst_loc, *src_loc;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	state = self->fg_state;
	ASSERT(lid < state->ms_localc);
	if unlikely(state->ms_stackc >= state->ms_stacka &&
	            Dee_memstate_reqvstack(state, state->ms_stackc + 1))
		goto err;
	src_loc = &state->ms_localv[lid];
	if (instr) {
		if ((src_loc->ml_type == MEMLOC_TYPE_UNALLOC) ||
		    (src_loc->ml_flags & MEMLOC_F_LOCAL_UNBOUND)) {
			/* Variable is always unbound -> generate code to throw an exception */
			if unlikely(Dee_function_generator_gthrow_local_unbound(self, instr, (uint16_t)lid))
				goto err;
			return Dee_function_generator_vpush_const(self, Dee_None);
		} else if (!(src_loc->ml_flags & MEMLOC_F_LOCAL_BOUND)) {
			/* Variable is not guarantied bound -> generate code to check if it's bound */
			if unlikely(Dee_function_generator_gassert_local_bound(self, instr, (uint16_t)lid))
				goto err;
			/* After a bound assertion, the local variable is guarantied to be bound. */
			src_loc->ml_flags |= MEMLOC_F_LOCAL_BOUND;
		}
	}
	dst_loc = &state->ms_stackv[state->ms_stackc];
	*dst_loc = *src_loc;
	dst_loc->ml_flags &= ~MEMLOC_M_LOCAL_BSTATE;
	dst_loc->ml_flags |= MEMLOC_F_NOREF; /* alias! (so no reference) */
	if (MEMLOC_TYPE_HASREG(dst_loc->ml_type))
		Dee_memstate_incrinuse(state, dst_loc->ml_value.v_hreg.r_regno);
	++state->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdup_n(struct Dee_function_generator *__restrict self,
                              Dee_vstackaddr_t n) {
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
	if (MEMLOC_TYPE_HASREG(loc->ml_type))
		Dee_memstate_decrinuse(state, loc->ml_value.v_hreg.r_regno);

	if (!(loc->ml_flags & MEMLOC_F_NOREF)) {
		struct Dee_memloc *other_loc;
		bool has_ref_alias = false;

		/* Try and shift the burden of the reference to the other location. */
		Dee_memstate_foreach(other_loc, state) {
			if (!Dee_memloc_sameloc(loc, other_loc))
				continue;
			if (!(other_loc->ml_flags & MEMLOC_F_NOREF)) {
				has_ref_alias = true;
				continue;
			}
			other_loc->ml_flags &= ~MEMLOC_F_NOREF;
			goto done;
		}
		Dee_memstate_foreach_end;

		/* No-where to shift the reference to -> must decref the object ourselves. */
		if unlikely(has_ref_alias ? Dee_function_generator_gdecref_nokill(self, loc, 1)
		                          : Dee_function_generator_gdecref(self, loc, 1)) {
			if (MEMLOC_TYPE_HASREG(loc->ml_type))
				Dee_memstate_incrinuse(state, loc->ml_value.v_hreg.r_regno);
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
Dee_function_generator_vpopmany(struct Dee_function_generator *__restrict self,
                                Dee_vstackaddr_t n) {
	int result = 0;
	while (n) {
		--n;
		result = Dee_function_generator_vpop(self);
		if unlikely(result)
			break;
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_n(struct Dee_function_generator *__restrict self,
                              Dee_vstackaddr_t n) {
	ASSERT(n >= 1);
	if unlikely(Dee_function_generator_vlrot(self, n))
		goto err;
	if unlikely(Dee_function_generator_vpop(self))
		goto err;
	return Dee_function_generator_vrrot(self, n - 1);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_decref_local(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc *__restrict loc) {
	struct Dee_memstate *state = self->fg_state;
	ASSERT(loc->ml_type != MEMLOC_TYPE_UNALLOC);
	ASSERT(!(loc->ml_flags & MEMLOC_F_NOREF));
	if (loc->ml_flags & MEMLOC_F_LOCAL_UNBOUND) {
		/* Nothing to do here! */
	} else if (loc->ml_flags & MEMLOC_F_LOCAL_BOUND) {
		/* Check if we can off-load the reference to someone else. */
		bool has_ref_alias = false;
		struct Dee_memloc *other_loc;
		Dee_memstate_foreach(other_loc, state) {
			if (!Dee_memloc_sameloc(loc, other_loc))
				continue;
			if (Dee_memstate_foreach_islocal(other_loc, state) &&
			    !(other_loc->ml_flags & MEMLOC_F_LOCAL_BOUND))
				continue;
			if (loc == other_loc)
				continue;
			if (!(other_loc->ml_flags & MEMLOC_F_NOREF)) {
				has_ref_alias = true;
				continue;
			}
			other_loc->ml_flags &= ~MEMLOC_F_NOREF;
			return 0;
		}
		Dee_memstate_foreach_end;
		if unlikely(has_ref_alias ? Dee_function_generator_gdecref_nokill(self, loc, 1)
		                          : Dee_function_generator_gdecref(self, loc, 1))
			goto err;
	} else {
		/* Location is conditionally bound.
		 * Check if there is another conditionally bound
		 * location which we can off-load the decref onto. */
		size_t i;
		bool has_ref_alias = false;
		struct Dee_memloc *other_loc;
		for (i = 0; i < state->ms_localc; ++i) {
			other_loc = &state->ms_localv[i];
			if (other_loc == loc)
				continue;
			if (!Dee_memloc_sameloc(loc, other_loc))
				continue;
			if ((other_loc->ml_flags & MEMLOC_M_LOCAL_BSTATE) != 0)
				continue;
			if (!(other_loc->ml_flags & MEMLOC_F_NOREF)) {
				has_ref_alias = true;
				continue;
			}
			other_loc->ml_flags &= ~MEMLOC_F_NOREF;
			return 0;
		}
		if unlikely(has_ref_alias ? Dee_function_generator_gxdecref_nokill(self, loc, 1)
		                          : Dee_function_generator_gxdecref(self, loc, 1))
			goto err;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_local(struct Dee_function_generator *__restrict self,
                                  size_t lid) {
	struct Dee_memstate *state;
	struct Dee_memloc *src, *dst;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	state = self->fg_state;
	ASSERT(lid < state->ms_localc);
	src = Dee_memstate_vtop(state);
	dst = &state->ms_localv[lid];
	if (dst->ml_type != MEMLOC_TYPE_UNALLOC && !(dst->ml_flags & MEMLOC_F_NOREF)) {
		if unlikely(Dee_function_generator_decref_local(self, dst))
			goto err;
		ASSERT(state == self->fg_state);
		ASSERT(dst == &state->ms_localv[lid]);
	}

	/* Because stack elements are always bound, the local is guarantied bound at this point. */
	if (MEMLOC_TYPE_HASREG(dst->ml_type))
		Dee_memstate_decrinuse(state, dst->ml_value.v_hreg.r_regno);
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
Dee_function_generator_vdel_local(struct Dee_function_generator *__restrict self,
                                  size_t lid) {
	struct Dee_memstate *state;
	struct Dee_memloc *loc;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	state = self->fg_state;
	ASSERT(lid < state->ms_localc);
	loc = &state->ms_localv[lid];
	if (loc->ml_type != MEMLOC_TYPE_UNALLOC && !(loc->ml_flags & MEMLOC_F_NOREF)) {
		if unlikely(Dee_function_generator_decref_local(self, loc))
			goto err;
		ASSERT(state == self->fg_state);
		ASSERT(loc == &state->ms_localv[lid]);
	}
	if (MEMLOC_TYPE_HASREG(loc->ml_type))
		Dee_memstate_decrinuse(state, loc->ml_value.v_hreg.r_regno);
	loc->ml_flags = MEMLOC_F_NOREF | MEMLOC_F_LOCAL_UNBOUND;
	loc->ml_type = MEMLOC_TYPE_UNALLOC;
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
	/* [OPERATOR_ITERNEXT]     = */ { (void *)NULL }, /* Special handling (because `DeeObject_IterNext' can return ITER_DONE) */
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
	/* [OPERATOR_INC]          = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_DEC]          = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_ADD]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_SUB]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_MUL]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_DIV]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_MOD]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_SHL]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_SHR]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_AND]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_OR]   = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_XOR]  = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_INPLACE_POW]  = */ { (void *)NULL }, /* Special handling */
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
	/* [OPERATOR_DELATTR]      = */ { (void *)&DeeObject_DelAttr, 2, VCALLOP_CC_INT },
	/* [OPERATOR_SETATTR]      = */ { (void *)&DeeObject_SetAttr, 3, VCALLOP_CC_INT },
	/* [OPERATOR_ENUMATTR]     = */ { (void *)NULL }, /* Special handling */
	/* [OPERATOR_ENTER]        = */ { (void *)&DeeObject_Enter, 1, VCALLOP_CC_INT },
	/* [OPERATOR_LEAVE]        = */ { (void *)&DeeObject_Leave, 1, VCALLOP_CC_INT },
};

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vop(struct Dee_function_generator *__restrict self,
                           uint16_t operator_name, Dee_vstackaddr_t argc) {
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
			if unlikely(Dee_function_generator_vcallapi(self, specs->hos_apifunc,
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
			return Dee_function_generator_vcallapi(self, (void *)&DeeObject_CallTuple, VCALLOP_CC_OBJECT, 2);
		if (argc == 3)
			return Dee_function_generator_vcallapi(self, (void *)&DeeObject_CallTupleKw, VCALLOP_CC_OBJECT, 3);
		break;

	default: break;
	}

	/* Encode a call to `DeeObject_InvokeOperator()' */
	if unlikely(argc < 1)
		return err_illegal_stack_effect();
	--argc; /* The "this"-argument is passed individually */
	if unlikely(Dee_function_generator_vlinear(self, argc))
		goto err; /* this, [args...], argv */
	if unlikely(Dee_function_generator_vlrot(self, argc + 2))
		goto err; /* [args...], argv, this */
	if unlikely(Dee_function_generator_vpush_imm16(self, operator_name))
		goto err; /* [args...], argv, this, opname */
	if unlikely(Dee_function_generator_vpush_immSIZ(self, argc))
		goto err; /* [args...], argv, this, opname, argc */
	if unlikely(Dee_function_generator_vlrot(self, 4))
		goto err; /* [args...], this, opname, argc, argv */
	if unlikely(Dee_function_generator_vcallapi(self, &DeeObject_InvokeOperator, VCALLOP_CC_RAWINT, 4))
		goto err; /* [args...], UNCHECKED(result) */
	if unlikely(Dee_function_generator_vrrot(self, argc + 1))
		goto err; /* UNCHECKED(result), [args...] */
	if unlikely(Dee_function_generator_vpopmany(self, argc))
		goto err; /* UNCHECKED(result) */
	return Dee_function_generator_vcheckobj(self);
err:
	return -1;
}

/* doesn't leave result on-stack */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vopv(struct Dee_function_generator *__restrict self,
                            uint16_t operator_name, Dee_vstackaddr_t argc) {
#ifndef __OPTIMIZE_SIZE__
	/* Check if there is a dedicated API function. */
	if (operator_name < COMPILER_LENOF(operator_apis)) {
		struct host_operator_specs const *specs = &operator_apis[operator_name];
		if (specs->hos_apifunc != NULL && specs->hos_argc == argc) {
			if unlikely(argc > self->fg_state->ms_stackc)
				return err_illegal_stack_effect();
			if unlikely(Dee_function_generator_vcallapi(self, specs->hos_apifunc,
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


INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_ulocal(struct Dee_function_generator *__restrict self,
                                    Dee_instruction_t const *instr, uint16_t lid) {
	if unlikely(lid >= self->fg_assembler->fa_localc)
		return err_illegal_lid(lid);
	return Dee_function_generator_vpush_local(self, instr, (size_t)lid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_ulocal(struct Dee_function_generator *__restrict self,
                                   uint16_t lid) {
	if unlikely(lid >= self->fg_assembler->fa_localc)
		return err_illegal_lid(lid);
	return Dee_function_generator_vpop_local(self, (size_t)lid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdel_ulocal(struct Dee_function_generator *__restrict self,
                                   uint16_t lid) {
	if unlikely(lid >= self->fg_assembler->fa_localc)
		return err_illegal_lid(lid);
	return Dee_function_generator_vdel_local(self, (size_t)lid);
}




INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_argc(struct Dee_function_generator *__restrict self) {
	if (!(self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE))
		return Dee_function_generator_vpush_xlocal(self, DEE_MEMSTATE_EXTRA_LOCAL_A_ARGC);
	if unlikely(Dee_function_generator_vpush_xlocal(self, DEE_MEMSTATE_EXTRA_LOCAL_A_ARGS))
		goto err;
	return Dee_function_generator_vind(self, offsetof(DeeTupleObject, t_size));
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_argv(struct Dee_function_generator *__restrict self) {
	if (!(self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE))
		return Dee_function_generator_vpush_xlocal(self, DEE_MEMSTATE_EXTRA_LOCAL_A_ARGV);
	if unlikely(Dee_function_generator_vpush_xlocal(self, DEE_MEMSTATE_EXTRA_LOCAL_A_ARGS))
		goto err;
	return Dee_function_generator_vdelta(self, offsetof(DeeTupleObject, t_elem));
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_usage(struct Dee_function_generator *__restrict self,
                                   Dee_host_regusage_t usage) {
	Dee_host_register_t regno;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	regno = Dee_function_generator_gusagereg(self, usage, NULL);
	if unlikely(regno >= HOST_REGISTER_COUNT)
		goto err;
	return Dee_function_generator_vpush_reg(self, regno, 0);
err:
	return -1;
}




/* Perform a conditional jump to `desc' based on `jump_if_true'
 * @param: instr: Pointer to start of deemon jmp-instruction (for bb-truncation, and error message)
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vjcc(struct Dee_function_generator *__restrict self,
                            struct Dee_jump_descriptor *desc,
                            Dee_instruction_t const *instr, bool jump_if_true) {
	int temp;
	struct Dee_basic_block *target = desc->jd_to;
	struct Dee_except_exitinfo *except_exit;
	struct Dee_memloc loc;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	loc = *Dee_function_generator_vtop(self);

	/* Special case for when the top-element is a constant. */
	if (loc.ml_type == MEMLOC_TYPE_CONST) {
		temp = DeeObject_Bool(loc.ml_value.v_const);
		if unlikely(temp < 0) {
			DeeError_Handled(Dee_ERROR_HANDLED_RESTORE);
		} else {
			bool should_jump = (temp > 0) == jump_if_true;
			if (should_jump) {
				/* Unconditional jump -> the block ends here and falls into the next one */
				self->fg_block->bb_next       = target;
				self->fg_block->bb_deemon_end = instr; /* The jump doesn't exist anymore now! */
			}
			return Dee_function_generator_vpop(self);
		}
	}

	/* Evaluate the top stack-object to a boolean (via `DeeObject_Bool'). */
	if unlikely(Dee_function_generator_gflushregs(self, 1, false))
		goto err;

	/* Check if the location was clobbered by the register flush. */
	if (loc.ml_type == MEMLOC_TYPE_HREG &&
		self->fg_state->ms_rusage[loc.ml_value.v_hreg.r_regno] != DEE_HOST_REGUSAGE_GENERIC)
		loc = *Dee_function_generator_vtop(self);

	/* Emit the actual call. */
	if unlikely(_Dee_function_generator_gcallapi(self, (void *)&DeeObject_Bool, 1, &loc))
		goto err;
	if unlikely(Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN, 0))
		goto err;
	if unlikely(Dee_function_generator_vswap(self))
		goto err;
	if unlikely(Dee_function_generator_vpop(self))
		goto err;

	/* At this point, the stack-top location contains the -1/0/1 returned by `DeeObject_Bool()' */
	except_exit = Dee_function_generator_except_exit(self);
	if unlikely(!except_exit)
		goto err;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;

	/* If the jump target location already has its starting memory state generated,
	 * and that state requires a small CFA offset than we currently have, then try
	 * to reclaim unused host stack memory.
	 *
	 * This is necessary so we don't end up in a situation where a block keeps on
	 * marking itself for re-compilation with ever-growing CFA offsets. */
	if (target->bb_mem_start != NULL &&
	    target->bb_mem_start->ms_host_cfa_offset < self->fg_state->ms_host_cfa_offset) {
		uintptr_t old_cfa_offset = self->fg_state->ms_host_cfa_offset;
		if (Dee_memstate_hstack_free(self->fg_state)) {
			uintptr_t new_cfa_offset = self->fg_state->ms_host_cfa_offset;
			ptrdiff_t freed = old_cfa_offset - new_cfa_offset;
			ASSERT(freed > 0);
#ifdef HOSTASM_STACK_GROWS_DOWN
			if unlikely(_Dee_function_generator_ghstack_adjust(self, freed))
#else /* HOSTASM_STACK_GROWS_DOWN */
			if unlikely(_Dee_function_generator_ghstack_adjust(self, -freed))
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			{
				goto err;
			}
		}
	}

	/* Silently remove the -1/0/1 from DeeObject_Bool from the vstack. */
	ASSERT(self->fg_state->ms_stackc >= 1);
	loc = *Dee_function_generator_vtop(self);
	ASSERT(loc.ml_flags & MEMLOC_F_NOREF);
	--self->fg_state->ms_stackc;
	if (MEMLOC_TYPE_HASREG(loc.ml_type))
		Dee_memstate_decrinuse(self->fg_state, loc.ml_value.v_hreg.r_regno);

	/* TODO: If this jump might be the result of infinite loops,
	 *       must emit a call to `DeeThread_CheckInterrupt()' */

	/* Generate code to branch depending on the value of `loc' */
	{
		struct Dee_memloc zero;
		struct Dee_host_symbol tsym;
		struct Dee_host_symbol xsym;
		zero.ml_type = MEMLOC_TYPE_CONST;
		zero.ml_value.v_const = NULL;
		tsym.hs_type = DEE_HOST_SYMBOL_SECT;
		tsym.hs_value.sv_sect.ss_sect = &target->bb_htext;
		tsym.hs_value.sv_sect.ss_off  = 0;
		xsym.hs_type = DEE_HOST_SYMBOL_SECT;
		xsym.hs_value.sv_sect.ss_sect = &except_exit->exi_block->bb_htext;
		xsym.hs_value.sv_sect.ss_off  = 0;
		if unlikely(_Dee_function_generator_gjcmp(self, &loc, &zero, true,
		                                          &xsym,                        /* loc < 0 */
		                                          jump_if_true ? NULL : &tsym,  /* loc == 0 */
		                                          jump_if_true ? &tsym : NULL)) /* loc > 0 */
			goto err;
	}

	/* Remember the memory-state as it is when the jump is made. */
	ASSERTF(!desc->jd_stat, "Who assigned this? Doing that is *my* job!");
	desc->jd_stat = self->fg_state;
	Dee_memstate_incref(self->fg_state);

	temp = Dee_basic_block_constrainwith(target, desc->jd_stat,
	                                     Dee_function_assembler_addrof(self->fg_assembler,
	                                                                   target->bb_deemon_start));
	if (temp > 0) {
		temp = 0;
		if (target == self->fg_block) {
			/* Special case for when the block that's currently being
			 * compiled had to constrain itself a little further. */
			ASSERT(target->bb_htext.hs_end == target->bb_htext.hs_start);
			ASSERT(target->bb_htext.hs_relc == 0);
			ASSERT(target->bb_hcold.hs_end == target->bb_hcold.hs_start);
			ASSERT(target->bb_hcold.hs_relc == 0);
			ASSERT(target->bb_mem_end == NULL);
			target->bb_mem_end = (DREF struct Dee_memstate *)-1;
		}
	}

	return temp;
err:
	return -1;
}



/* >> TOP = *(TOP + ind_delta); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vind(struct Dee_function_generator *__restrict self,
                            ptrdiff_t ind_delta) {
	struct Dee_memloc *loc;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	loc = Dee_function_generator_vtop(self);
	return Dee_function_generator_gind(self, loc, ind_delta);
err:
	return -1;
}

/* >> *(SECOND + ind_delta) = POP(); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpopind(struct Dee_function_generator *__restrict self,
                               ptrdiff_t ind_delta) {
	struct Dee_memloc src, *loc;
	if unlikely(self->fg_state->ms_stackc < 2)
		return err_illegal_stack_effect();
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	src = *Dee_function_generator_vtop(self);
	--self->fg_state->ms_stackc;
	if (MEMLOC_TYPE_HASREG(src.ml_type))
		Dee_memstate_decrinuse(self->fg_state, src.ml_value.v_hreg.r_regno);
	loc = Dee_function_generator_vtop(self);
	return Dee_function_generator_gmov_loc2locind(self, &src, loc, ind_delta);
err:
	return -1;
}

/* >> TOP = TOP + val_delta; */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdelta(struct Dee_function_generator *__restrict self,
                              ptrdiff_t val_delta) {
	struct Dee_memloc *loc;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	if unlikely(val_delta == 0)
		return 0;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	loc = Dee_function_generator_vtop(self);
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ASSERT(loc->ml_type == MEMLOC_TYPE_HREG);
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
	case MEMLOC_TYPE_HSTACK:
		loc->ml_value.v_hreg.r_off += val_delta;
		break;
	case MEMLOC_TYPE_HREGIND:
	case MEMLOC_TYPE_HSTACKIND:
		loc->ml_value.v_hreg.r_voff += val_delta;
		break;
	case MEMLOC_TYPE_CONST:
		loc->ml_value.v_const = (DeeObject *)((uintptr_t)loc->ml_value.v_const + val_delta);
		break;
	}
	return 0;
err:
	return -1;
}

/* >> temp = *(SECOND + ind_delta);
 * >> *(SECOND + ind_delta) = FIRST;
 * >> FIRST = temp; */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vxch_ind(struct Dee_function_generator *__restrict self, ptrdiff_t ind_delta) {
	/* TODO */
	(void)self;
	(void)ind_delta;
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

		/* If at least 2 other memory locations (or 1 if it's a constant) are already
		 * holding a reference to the same value, then we can steal a reference from
		 * one of them!
		 *
		 * The reason for that "2" is because as long as there are 2 references, an
		 * object is guarantied to have `DeeObject_IsShared()', meaning that whatever
		 * the caller might need the reference for, the object won't end up getting
		 * destroyed if the reference ends up being dropped! */
		{
			struct Dee_memloc *alias1, *alias2;
			alias1 = Dee_memstate_findrefalias(state, loc, NULL);
			if (alias1) {
				ASSERT(!(alias1->ml_flags & MEMLOC_F_NOREF));
				alias2 = Dee_memstate_findrefalias(state, loc, alias1);
				if (alias2 != NULL) {
					/* Steal the reference from `alias2' */
					ASSERT(!(alias2->ml_flags & MEMLOC_F_NOREF));
					ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
					loc->ml_flags &= ~MEMLOC_F_NOREF;
					alias2->ml_flags |= MEMLOC_F_NOREF;
					return 0;
				}
			}
		}

		if unlikely(Dee_function_generator_gincref(self, loc, 1))
			goto err;
		ASSERT(loc == Dee_memstate_vtop(state));
		ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
		loc->ml_flags &= ~MEMLOC_F_NOREF;
	}
	return 0;
err:
	return -1;
}

/* Force vtop into a register (ensuring it has type `MEMLOC_TYPE_HREG') */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vreg(struct Dee_function_generator *__restrict self,
                            Dee_host_register_t const *not_these) {
	struct Dee_memstate *state = self->fg_state;
	struct Dee_memloc *loc;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	loc = Dee_memstate_vtop(state);
	if (loc->ml_type != MEMLOC_TYPE_HREG) {
		if unlikely(Dee_function_generator_state_unshare(self))
			goto err;
		state = self->fg_state;
		loc   = Dee_memstate_vtop(state);
		if unlikely(Dee_function_generator_greg(self, loc, not_these))
			goto err;
		ASSERT(loc->ml_type == MEMLOC_TYPE_HREG);
	}
	return 0;
err:
	return -1;
}

/* Force vtop onto the stack (ensuring it has type `MEMLOC_TYPE_HSTACKIND, v_hstack.s_off = 0') */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vflush(struct Dee_function_generator *__restrict self) {
	struct Dee_memstate *state = self->fg_state;
	struct Dee_memloc *loc;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	loc = Dee_memstate_vtop(state);
	if (loc->ml_type != MEMLOC_TYPE_HSTACKIND ||
	    loc->ml_value.v_hstack.s_off != 0) {
		if unlikely(Dee_function_generator_state_unshare(self))
			goto err;
		state = self->fg_state;
		loc   = Dee_memstate_vtop(state);
		if unlikely(Dee_function_generator_gflush(self, loc))
			goto err;
		ASSERT(loc->ml_type == MEMLOC_TYPE_HSTACKIND);
	}
	return 0;
err:
	return -1;
}


/* Generate code to push a global variable onto the virtual stack. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vpush_global_or_extern(struct Dee_function_generator *__restrict self,
                       DeeModuleObject *__restrict mod, uint16_t gid,
                       uint8_t kind, uint16_t id1, uint16_t id2, bool ref) {
	struct Dee_memloc *loc;
	struct module_symbol *symbol;
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
	if (ref) {
		if unlikely(Dee_function_generator_grwlock_read(self, &mod->mo_lock))
			goto err;
	}
	if unlikely(Dee_function_generator_vind(self, 0))
		goto err;
	if unlikely(Dee_function_generator_vreg(self, NULL))
		goto err;
	loc = Dee_function_generator_vtop(self);
	ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
	if unlikely(Dee_function_generator_gassert_bound(self, loc, NULL, kind, id1, id2,
	                                                 ref ? &mod->mo_lock : NULL, NULL))
		goto err;

	/* Depending on how the value will be used, we may not need a reference.
	 * If only its value is used (ASM_ISNONE, ASM_CMP_SO, ASM_CMP_DO), we
	 * won't actually need to take a reference here!
	 * Also: when not needing a reference, we don't need to acquire the lock,
	 *       either! */
	ASSERT(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF);
	if (ref) {
		if unlikely(Dee_function_generator_gincref(self, loc, 1))
			goto err;
		if unlikely(Dee_function_generator_grwlock_endread(self, &mod->mo_lock))
			goto err;
		loc = Dee_function_generator_vtop(self);
		ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
		loc->ml_flags &= ~MEMLOC_F_NOREF;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_global(struct Dee_function_generator *__restrict self,
                                    uint16_t gid, bool ref) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	return vpush_global_or_extern(self, mod, gid, ASM_GLOBAL, gid, 0, ref);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_extern(struct Dee_function_generator *__restrict self,
                                    uint16_t mid, uint16_t gid, bool ref) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	if unlikely(mid >= mod->mo_importc)
		return err_illegal_mid(mid);
	mod = mod->mo_importv[mid];
	return vpush_global_or_extern(self, mod, gid, ASM_EXTERN, mid, gid, ref);
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
	if unlikely(Dee_function_generator_gxdecref(self, loc, 1)) /* xdecref in case global wasn't bound before. */
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
	if unlikely(Dee_function_generator_vreg(self, NULL))
		goto err;
	loc = Dee_function_generator_vtop(self);
	ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
	loc->ml_flags &= ~MEMLOC_F_NOREF;
	if unlikely(Dee_function_generator_gincref(self, loc, 1))
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


/* Check if `loc' differs from vtop, and if so: move vtop
 * *into* `loc', the assign the *exact* given `loc' to vtop. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vsetloc(struct Dee_function_generator *__restrict self,
                               struct Dee_memloc const *loc) {
	int result;
	struct Dee_memloc *vtop;
	uint16_t stackc = self->fg_state->ms_stackc;
	if unlikely(stackc < 1)
		return err_illegal_stack_effect();
	vtop = Dee_function_generator_vtop(self);
	if (Dee_memloc_sameloc(vtop, loc))
		return 0; /* Already the same location -> no need to do anything */
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	vtop   = Dee_function_generator_vtop(self);
	result = Dee_function_generator_gmov_loc2loc(self, vtop, loc);
	if likely(result == 0) {
		if (MEMLOC_TYPE_HASREG(vtop->ml_type))
			Dee_memstate_decrinuse(self->fg_state, vtop->ml_value.v_hreg.r_regno);
		vtop->ml_type  = loc->ml_type;
		vtop->ml_value = loc->ml_value;
		if (MEMLOC_TYPE_HASREG(vtop->ml_type))
			Dee_memstate_incrinuse(self->fg_state, vtop->ml_value.v_hreg.r_regno);
	}
	return result;
err:
	return -1;
}


/* Return and pass the top-most stack element as function result.
 * This function will clear the stack and unbind all local variables. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vret(struct Dee_function_generator *__restrict self) {
	struct Dee_memloc loc;
	uint16_t stackc = self->fg_state->ms_stackc;
	size_t i;
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
	if (MEMLOC_TYPE_HASREG(loc.ml_type))
		Dee_memstate_decrinuse(self->fg_state, loc.ml_value.v_hreg.r_regno);

	/* Generate code to do the return. */
	return Dee_function_generator_gret(self, &loc);
err:
	return -1;
}



/* Generate host text to invoke `api_function' with the top-most `argc' items from the stack.
 * @param: cc: One of `VCALLOP_CC_*', describing the calling-convention of `api_function' */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcallapi_(struct Dee_function_generator *__restrict self,
                                 void const *api_function, unsigned int cc,
                                 Dee_vstackaddr_t argc) {
	struct Dee_memloc *loc, *argv;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(argc > self->fg_state->ms_stackc)
		return err_illegal_stack_effect();

	/* Save argument memory locations from before the flush. This is because after the
	 * flush, registers are written to the stack, and if we were to pass the then-current
	 * `Dee_memloc' to `_Dee_function_generator_gcallapi', it would have to load those
	 * values from the stack (when they can also be found in registers) */
	loc = self->fg_state->ms_stackv;
	loc += self->fg_state->ms_stackc;
	loc -= argc;
	argv = (struct Dee_memloc *)Dee_Mallocac(argc, sizeof(struct Dee_memloc));
	if unlikely(!argv)
		goto err;
	argv = (struct Dee_memloc *)memcpyc(argv, loc, argc, sizeof(struct Dee_memloc));

	/* Flush registers that don't appear in the top `argc' stack locations.
	 * When the function always throw an exception, we *only* need to preserve
	 * stuff that contains references! */
	if unlikely(Dee_function_generator_gflushregs(self, argc, cc == VCALLOP_CC_EXCEPT))
		goto err_argv;

	/* Check if any of the argument registers got clobbered by register usage during flushing. */
	{
		uint16_t argi;
		for (argi = 0; argi < argc; ++argi) {
			struct Dee_memloc *stck_loc;
			struct Dee_memloc *argv_loc = &argv[argi];
			if (!MEMLOC_TYPE_HASREG(argv_loc->ml_type))
				continue;
			ASSERT(argv_loc->ml_value.v_hreg.r_regno < HOST_REGISTER_COUNT);
			if (self->fg_state->ms_rusage[argv_loc->ml_value.v_hreg.r_regno] == DEE_HOST_REGUSAGE_GENERIC)
				continue;
			stck_loc = &self->fg_state->ms_stackv[self->fg_state->ms_stackc - argc + argi];
			*argv_loc = *stck_loc;
		}
	}

	/* Call the actual C function */
	if unlikely(_Dee_function_generator_gcallapi(self, api_function, argc, argv))
		goto err_argv;
	Dee_Freea(argv);

	/* Do calling-convention-specific handling of the return value. */
	switch (cc) {

	case VCALLOP_CC_OBJECT:
		if unlikely(Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN, 0))
			goto err;
		loc = Dee_function_generator_vtop(self);
		if unlikely(Dee_function_generator_gjz_except(self, loc))
			goto err;
		/* Clear the NOREF flag now that we know the return value to be non-NULL */
		loc = Dee_function_generator_vtop(self);
		ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
		loc->ml_flags &= ~MEMLOC_F_NOREF;

		/* Rotate the return value to be located *below* arguments (which get popped below) */
rotate_return_register:
		if unlikely(Dee_function_generator_vrrot(self, argc + 1))
			goto err;
		break;

	case VCALLOP_CC_RAWINT:
		if unlikely(Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN, 0))
			goto err;
		goto rotate_return_register;

	case VCALLOP_CC_RAWINT_KEEPARGS:
		return Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN, 0);

	case VCALLOP_CC_VOID:
		break;

	case VCALLOP_CC_EXCEPT:
		if unlikely(Dee_function_generator_vpopmany(self, argc))
			goto err;
		return Dee_function_generator_gjmp_except(self);

	case VCALLOP_CC_INT:
		if unlikely(Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN, 0))
			goto err;
		loc = Dee_function_generator_vtop(self);
		if unlikely(Dee_function_generator_gjnz_except(self, loc))
			goto err;
		ASSERT(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF);
		if unlikely(Dee_function_generator_vpop(self))
			goto err;
		break;

	default: __builtin_unreachable();
	}

	/* Pop function arguments. */
	return Dee_function_generator_vpopmany(self, argc);
err_argv:
	Dee_Freea(argv);
err:
	return -1;
}

/* After a call to `Dee_function_generator_vcallapi()' with `VCALLOP_CC_RAWINT',
 * do the extra trailing checks needed to turn that call into `VCALLOP_CC_OBJECT'
 * The difference to directly passing `VCALLOP_CC_OBJECT' is that using this 2-step
 * method, you're able to pop more elements from the stack first.
 *
 * However: be careful not to do anything that might throw additional exceptions!
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcheckobj(struct Dee_function_generator *__restrict self) {
	struct Dee_memloc *loc;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	loc = Dee_function_generator_vtop(self);
	if unlikely(Dee_function_generator_gjz_except(self, loc))
		goto err;
	/* Clear the NOREF flag now that we know the return value to be non-NULL */
	loc = Dee_function_generator_vtop(self);
	ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
	loc->ml_flags &= ~MEMLOC_F_NOREF;
	return 0;
err:
	return -1;
}

/* After a call to `Dee_function_generator_vcallapi()' with `VCALLOP_CC_RAWINT',
 * do the extra trailing checks needed to turn that call into `VCALLOP_CC_INT'
 * The difference to directly passing `VCALLOP_CC_INT' is that using this 2-step
 * method, you're able to pop more elements from the stack first.
 * NOTE: This function pops one element from the V-stack.
 *
 * However: be careful not to do anything that might throw additional exceptions!
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcheckint(struct Dee_function_generator *__restrict self) {
	struct Dee_memloc *loc;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	loc = Dee_function_generator_vtop(self);
	if unlikely(Dee_function_generator_gjnz_except(self, loc))
		goto err;
	return Dee_function_generator_vpop(self);
err:
	return -1;
}



/* Arrange the top `argc' stack-items linearly, such that they all appear somewhere in memory
 * (probably on the host-stack), in consecutive order (with `vtop' at the greatest address,
 * and STACK[SIZE-argc] appearing at the lowest address). Once that has been accomplished,
 * push a value onto the vstack that describes the base-address (that is a `DeeObject **'
 * pointing to `STACK[SIZE-argc]') of the linear vector.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vlinear(struct Dee_function_generator *__restrict self,
                               Dee_vstackaddr_t argc) {
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(argc > self->fg_state->ms_stackc)
		return err_illegal_stack_effect();

	/* Deal with special argument count cases. */
	switch (argc) {

	case 0:
		/* The base address of an empty vector doesn't matter, meaning it's undefined */
		return Dee_function_generator_vpush_undefined(self);

	case 1: {
		/* Simple case: form a 1-element vector */
		uintptr_t cfa_offset;
		struct Dee_memloc *loc = Dee_function_generator_vtop(self);
		if (loc->ml_type == MEMLOC_TYPE_HREGIND && loc->ml_value.v_hreg.r_voff == 0) {
			/* Special case: address of `*(%reg + off) + 0'  is `%reg + off' */
			return Dee_function_generator_vpush_reg(self,
			                                        loc->ml_value.v_hreg.r_regno,
			                                        loc->ml_value.v_hreg.r_off);
		}
		/* Flush value to the stack to give is an addressable memory location. */
		if unlikely(Dee_function_generator_vflush(self))
			goto err;
		ASSERT(Dee_function_generator_vtop(self)->ml_type == MEMLOC_TYPE_HSTACKIND);
		ASSERT(Dee_function_generator_vtop(self)->ml_value.v_hstack.s_off == 0);
		cfa_offset = Dee_function_generator_vtop(self)->ml_value.v_hstack.s_cfa;
		return Dee_function_generator_vpush_hstack(self, cfa_offset);
	}	break;

	default:
		break;
	}

	/* TODO */
	(void)self;
	(void)argc;
	return DeeError_NOTIMPLEMENTED();
err:
	return -1;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C */
