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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_COMMON_C
#define GUARD_DEX_HOSTASM_GENERATOR_COMMON_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/tuple.h>
#include <deemon/util/lock.h>

#include "utils.h"

DECL_BEGIN

STATIC_ASSERT(offsetof(struct Dee_memloc, ml_value.v_hreg.r_voff) ==
              offsetof(struct Dee_memloc, ml_value.v_hstack.s_off));

/************************************************************************/
/* COMMON CODE GENERATION FUNCTIONS                                     */
/************************************************************************/

#define DeeInt_NEWSFUNC(n) DEE_PRIVATE_NEWINT(n)
#define DeeInt_NEWUFUNC(n) DEE_PRIVATE_NEWUINT(n)

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdirect_impl(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc *__restrict loc) {
	uint8_t vmorph = loc->ml_vmorph;
	ASSERT(!MEMLOC_VMORPH_ISDIRECT(vmorph));
	switch (vmorph) {

	case MEMLOC_VMORPH_NULLABLE: {
		int result;
		uint8_t saved_flags = loc->ml_flags;
		loc->ml_flags |= MEMLOC_F_NOREF;
		result = Dee_function_generator_gjz_except(self, loc);
		loc->ml_flags  = saved_flags;
		loc->ml_vmorph = MEMLOC_VMORPH_DIRECT;
		return result;
	}	break;

	case MEMLOC_VMORPH_BOOL_Z:
	case MEMLOC_VMORPH_BOOL_Z_01:
	case MEMLOC_VMORPH_BOOL_NZ:
	case MEMLOC_VMORPH_BOOL_NZ_01:
	case MEMLOC_VMORPH_BOOL_LZ:
	case MEMLOC_VMORPH_BOOL_GZ: {
		int temp;
		Dee_host_register_t retreg;
		ptrdiff_t retreg_delta;
		if (loc->ml_type == MEMLOC_TYPE_HREG) {
			retreg = loc->ml_value.v_hreg.r_regno;
		} else {
			Dee_host_register_t not_these[2];
			not_these[0] = HOST_REGISTER_COUNT;
			not_these[1] = HOST_REGISTER_COUNT;
			if (MEMLOC_TYPE_HASREG(loc->ml_type))
				not_these[0] = loc->ml_value.v_hreg.r_regno;
			retreg = Dee_function_generator_gallocreg(self, not_these);
			if unlikely(retreg >= HOST_REGISTER_COUNT)
				goto err;
		}
		if (vmorph == MEMLOC_VMORPH_BOOL_NZ_01) {
			temp = Dee_function_generator_gmorph_loc012regbooly(self, loc, 0, retreg, &retreg_delta);
		} else {
			unsigned int cmp;
			switch (vmorph) {
			case MEMLOC_VMORPH_BOOL_Z:
			case MEMLOC_VMORPH_BOOL_Z_01:
				cmp = GMORPHBOOL_CC_EQ;
				break;
			case MEMLOC_VMORPH_BOOL_NZ:
				cmp = GMORPHBOOL_CC_NE;
				break;
			case MEMLOC_VMORPH_BOOL_LZ:
				cmp = GMORPHBOOL_CC_LO;
				break;
			case MEMLOC_VMORPH_BOOL_GZ:
				cmp = GMORPHBOOL_CC_GR;
				break;
			default: __builtin_unreachable();
			}
			temp = Dee_function_generator_gmorph_loc2regbooly(self, loc, 0, cmp, retreg, &retreg_delta);
		}
		if unlikely(temp)
			goto err;
		if (Dee_memstate_isinstate(self->fg_state, loc)) {
			if (MEMLOC_TYPE_HASREG(loc->ml_type))
				Dee_memstate_decrinuse(self->fg_state, loc->ml_value.v_hreg.r_regno);
			Dee_memstate_incrinuse(self->fg_state, retreg);
		}
		loc->ml_type = MEMLOC_TYPE_HREG;
		loc->ml_value.v_hreg.r_regno = retreg;
		loc->ml_value.v_hreg.r_off   = retreg_delta;
		loc->ml_vmorph = MEMLOC_VMORPH_DIRECT;
		loc->ml_valtyp = &DeeBool_Type;
		return 0;
	}	break;

	case MEMLOC_VMORPH_INT:
	case MEMLOC_VMORPH_UINT: {
		/* Construct a new deemon integer object. */
		if unlikely(Dee_function_generator_gcallapi(self,
		                                            vmorph == MEMLOC_VMORPH_INT
		                                            ? (void const *)&DeeInt_NEWSFUNC(HOST_SIZEOF_POINTER)
		                                            : (void const *)&DeeInt_NEWUFUNC(HOST_SIZEOF_POINTER),
		                                            1, loc))
			goto err;
		loc->ml_flags  = MEMLOC_F_NOREF;
		loc->ml_vmorph = MEMLOC_VMORPH_DIRECT;
		loc->ml_type   = MEMLOC_TYPE_HREG;
		loc->ml_value.v_hreg.r_regno = HOST_REGISTER_RETURN;
		loc->ml_value.v_hreg.r_off   = 0;
		if unlikely(Dee_function_generator_gjz_except(self, loc))
			goto err;
		ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
		loc->ml_flags &= ~MEMLOC_F_NOREF;
		loc->ml_valtyp = &DeeInt_Type;
	}	break;

	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Unsupported location value type %#" PRFx8,
		                       loc->ml_vmorph);
	}
	return 0;
err:
	return -1;
}

/* Force `loc' to use `MEMLOC_VMORPH_ISDIRECT'.
 * NOTE: This is the only `Dee_function_generator_g*' function that
 *       doesn't simply assume `MEMLOC_VMORPH_ISDIRECT(ml_vmorph)'. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdirect(struct Dee_function_generator *__restrict self,
                               struct Dee_memloc *loc) {
	struct Dee_memloc *alias, oldloc;
	struct Dee_memstate *state;
	if (MEMLOC_VMORPH_ISDIRECT(loc->ml_vmorph))
		return 0; /* Already a direct value. */

	/* Force the value to become direct. */
	oldloc = *loc;
	if unlikely(Dee_function_generator_gdirect_impl(self, loc))
		goto err;

	/* Write the updated value into all aliases. */
	ASSERT(MEMLOC_VMORPH_ISDIRECT(loc->ml_vmorph));
	oldloc.ml_flags &= ~MEMLOC_M_LOCAL_BSTATE;
	state = self->fg_state;
	Dee_memstate_foreach(alias, state) {
		if (alias->ml_vmorph != oldloc.ml_vmorph)
			continue;
		if (!Dee_memloc_sameloc(alias, &oldloc))
			continue;
		if (alias == loc)
			continue;
		if (MEMLOC_TYPE_HASREG(alias->ml_type))
			Dee_memstate_decrinuse(state, alias->ml_value.v_hreg.r_regno);;
		if (Dee_memstate_foreach_islocal(alias, state)) {
			alias->ml_flags &= ~MEMLOC_M_LOCAL_BSTATE;
			alias->ml_flags |= MEMLOC_F_LOCAL_BOUND;
		}
		alias->ml_vmorph = loc->ml_vmorph;
		alias->ml_type   = loc->ml_type;
		alias->ml_value  = loc->ml_value;
		alias->ml_valtyp = loc->ml_valtyp;
		if (MEMLOC_TYPE_HASREG(alias->ml_type))
			Dee_memstate_incrinuse(state, alias->ml_value.v_hreg.r_regno);;
	}
	Dee_memstate_foreach_end;
	if (loc >= state->ms_localv &&
	    loc < state->ms_localv + state->ms_localc)
		loc->ml_flags |= MEMLOC_F_LOCAL_BOUND;
	return 0;
err:
	return -1;
}


/* Clear the `MEMLOC_F_ONEREF' flag from `loc', as well
 * as any other memory location that might be aliasing it. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gnotoneref_impl(struct Dee_function_generator *__restrict self,
                                       struct Dee_memloc *loc) {
	struct Dee_memloc *alias;
	ASSERT(loc->ml_flags & MEMLOC_F_ONEREF);
	Dee_memstate_foreach(alias, self->fg_state) {
		if (Dee_memloc_sameloc(alias, loc))
			alias->ml_flags &= ~MEMLOC_F_ONEREF;
	}
	Dee_memstate_foreach_end;
	loc->ml_flags &= ~MEMLOC_F_ONEREF;
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
Dee_memstate_remember_undefined_hstack_after_redzone(struct Dee_memstate *__restrict self) {
	uintptr_t min_undef_cfa;
	min_undef_cfa = self->ms_host_cfa_offset + HOSTASM_REDZONE_SIZE;
#ifdef HOSTASM_STACK_GROWS_DOWN
	min_undef_cfa += HOST_SIZEOF_POINTER;
#endif /* HOSTASM_STACK_GROWS_DOWN */
	Dee_memequivs_undefined_hstackind_after(&self->ms_memequiv, min_undef_cfa);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_adjust(struct Dee_function_generator *__restrict self,
                                      ptrdiff_t cfa_delta) {
	int result;
	if (cfa_delta == 0)
		return 0;
#ifdef HOSTASM_STACK_GROWS_DOWN
	result = _Dee_function_generator_ghstack_adjust(self, -cfa_delta);
#else /* HOSTASM_STACK_GROWS_DOWN */
	result = _Dee_function_generator_ghstack_adjust(self, cfa_delta);
#endif /* !HOSTASM_STACK_GROWS_DOWN */
	if likely(result == 0) {
		Dee_function_generator_gadjust_cfa_offset(self, cfa_delta);
		if (cfa_delta < 0) {
			/* Remember that values beyond the stack's red zone become undefined,
			 * as they might get clobbered by sporadic interrupt handlers. */
			if (cfa_delta == -HOST_SIZEOF_POINTER) {
#ifdef HOSTASM_STACK_GROWS_DOWN
				uintptr_t pop_dst_cfa_offset = self->fg_state->ms_host_cfa_offset + HOST_SIZEOF_POINTER;
#else /* HOSTASM_STACK_GROWS_DOWN */
				uintptr_t pop_dst_cfa_offset = self->fg_state->ms_host_cfa_offset;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
				Dee_function_generator_remember_undefined_hstackind(self, pop_dst_cfa_offset + HOSTASM_REDZONE_SIZE);
			} else {
				Dee_memstate_remember_undefined_hstack_after_redzone(self->fg_state);
			}
		}
	}
	return result;
}

#ifdef HOSTASM_STACK_GROWS_DOWN
#define GET_ADDRESS_OF_NEXT_PUSH(self) ((self)->fg_state->ms_host_cfa_offset + HOST_SIZEOF_POINTER)
#define GET_ADDRESS_OF_NEXT_POP(self)  ((self)->fg_state->ms_host_cfa_offset)
#else /* HOSTASM_STACK_GROWS_DOWN */
#define GET_ADDRESS_OF_NEXT_PUSH(self) ((self)->fg_state->ms_host_cfa_offset)
#define GET_ADDRESS_OF_NEXT_POP(self)  ((self)->fg_state->ms_host_cfa_offset - HOST_SIZEOF_POINTER)
#endif /* !HOSTASM_STACK_GROWS_DOWN */

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_pushreg(struct Dee_function_generator *__restrict self,
                                       Dee_host_register_t src_regno) {
	uintptr_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	int result;
#if HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER
	/* TODO: If "[#dst_cfa_offset]" is a known equivalence of "src_regno", do nothing */
#endif /* HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER */
	result = _Dee_function_generator_ghstack_pushreg(self, src_regno);
	if likely(result == 0) {
		Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
		result = Dee_function_generator_remember_movevalue_reg2hstackind(self, src_regno, dst_cfa_offset);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_pushregind(struct Dee_function_generator *__restrict self,
                                          Dee_host_register_t src_regno, ptrdiff_t src_delta) {
	uintptr_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	int result;
#if HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER
	/* TODO: If "[#dst_cfa_offset]" is a known equivalence of "[src_regno + src_delta]", do nothing */
#endif /* HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER */
	result = _Dee_function_generator_ghstack_pushregind(self, src_regno, src_delta);
	if likely(result == 0) {
		Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
		result = Dee_function_generator_remember_movevalue_regind2hstackind(self, src_regno, src_delta, dst_cfa_offset);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_pushconst(struct Dee_function_generator *__restrict self,
                                         DeeObject *value) {
	uintptr_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	int result;
#if HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER
	/* TODO: If "[#dst_cfa_offset]" is a known equivalence of "value", do nothing */
#endif /* HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER */
	result = _Dee_function_generator_ghstack_pushconst(self, value);
	if likely(result == 0) {
		Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
		result = Dee_function_generator_remember_movevalue_const2hstackind(self, value, dst_cfa_offset);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_pushhstackind(struct Dee_function_generator *__restrict self,
                                             uintptr_t cfa_offset) {
	uintptr_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	int result;
#if HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER
	/* TODO: If "[#dst_cfa_offset]" is a known equivalence of "[#cfa_offset]", do nothing */
#endif /* HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER */
	result = _Dee_function_generator_ghstack_pushhstackind(self, sp_offset);
	if likely(result == 0) {
		Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
		result = Dee_function_generator_remember_movevalue_hstackind2hstackind(self, cfa_offset, dst_cfa_offset);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_pushhstack_at_cfa_boundary(struct Dee_function_generator *__restrict self) {
	uintptr_t src_cfa_offset = GET_ADDRESS_OF_NEXT_POP(self);
	uintptr_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	int result = _Dee_function_generator_ghstack_pushhstack_at_cfa_boundary(self);
	if likely(result == 0) {
		Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
		(void)src_cfa_offset;
		(void)dst_cfa_offset;
		result = Dee_function_generator_remember_movevalue_hstack2hstackind(self, src_cfa_offset, dst_cfa_offset);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_popreg(struct Dee_function_generator *__restrict self,
                                      Dee_host_register_t dst_regno) {
	uintptr_t src_cfa_offset = GET_ADDRESS_OF_NEXT_POP(self);
	int result;
	/* TODO: If "dst_regno" is a known equivalence of "[#src_cfa_offset]", do nothing */
	result = _Dee_function_generator_ghstack_popreg(self, dst_regno);
	if likely(result == 0) {
		Dee_function_generator_gadjust_cfa_offset(self, -HOST_SIZEOF_POINTER);
		result = Dee_function_generator_remember_movevalue_hstackind2reg(self, src_cfa_offset, dst_regno);
		Dee_function_generator_remember_undefined_hstackind(self, src_cfa_offset + HOSTASM_REDZONE_SIZE);
	}
	return result;
}

#undef GET_ADDRESS_OF_NEXT_PUSH
#undef GET_ADDRESS_OF_NEXT_POP

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_reg2hstackind(struct Dee_function_generator *__restrict self,
                                          Dee_host_register_t src_regno, uintptr_t cfa_offset) {
	ptrdiff_t sp_offset;
	int result;
	/* TODO: If "[#cfa_offset]" is a known equivalence of "src_regno", do nothing */

	/* Check if the value *must* be pushed onto the h-stack. */
	if (Dee_memstate_hstack_mustpush(self->fg_state, cfa_offset)) {
		uintptr_t skip = Dee_memstate_hstack_mustpush_skip(self->fg_state, cfa_offset);
		ASSERT((skip == 0) == !!Dee_memstate_hstack_canpush(self->fg_state, cfa_offset));
		result = Dee_function_generator_ghstack_adjust(self, (ptrdiff_t)skip);
		if likely(result == 0)
			result = Dee_function_generator_ghstack_pushreg(self, src_regno);
		return result;
	}
	sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	result = _Dee_function_generator_gmov_reg2hstackind(self, src_regno, sp_offset);
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_reg2hstackind(self, src_regno, cfa_offset);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_hstack2reg(struct Dee_function_generator *__restrict self,
                                       uintptr_t cfa_offset, Dee_host_register_t dst_regno) {
	ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	int result = _Dee_function_generator_gmov_hstack2reg(self, sp_offset, dst_regno);
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC;
		result = Dee_function_generator_remember_movevalue_hstack2reg(self, cfa_offset, dst_regno);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_hstackind2reg(struct Dee_function_generator *__restrict self,
                                          uintptr_t cfa_offset, Dee_host_register_t dst_regno) {
	int result;
	ptrdiff_t sp_offset;
	/* TODO: If "dst_regno" is a known equivalence of "[#cfa_offset]", do nothing */

	/* Special case: if the value lies on-top of the host stack, then pop it instead of move it. */
	if (Dee_memstate_hstack_canpop(self->fg_state, cfa_offset)) {
		/* Update all vstack/local mem locations to instruct them to
		 * no longer use the HSTACK location. When there is no red zone,
		 * this is necessary (since the stack slot no longer exists),
		 * and even when there is a red zone, we'd have to do the same
		 * for anything that lies outside the red zone if we didn't do
		 * it for anything removed the instant it is removed.
		 *
		 * Plus: if someone *really* wants to use an hstack location
		 *       rather than a register location, they can look into
		 *       memory equivalences and find the HSTACKIND.
		 *
		 * !!! HOWEVER !!!
		 * If there is a memory location with the MEMLOC_F_LINEAR flag
		 * set, then we MUST NOT use pop (since the location can't be
		 * altered) */
		struct Dee_memloc *loc, *cfa_locs = NULL;
		Dee_memstate_foreach(loc, self->fg_state) {
			if (loc->ml_type == MEMLOC_TYPE_HSTACKIND &&
			    loc->ml_value.v_hstack.s_cfa == cfa_offset) {
				STATIC_ASSERT(offsetof(union Dee_memloc_value, _v_next) != offsetof(union Dee_memloc_value, v_hstack.s_cfa));
				STATIC_ASSERT(offsetof(union Dee_memloc_value, _v_next) != offsetof(union Dee_memloc_value, v_hstack.s_off));
				if (loc->ml_flags & MEMLOC_F_LINEAR)
					goto do_use_mov; /* Ooops: not allowed. (Location must not be moved to a register) */
				loc->ml_value._v_next = cfa_locs;
				cfa_locs = loc;
			}
		}
		Dee_memstate_foreach_end;
		if (cfa_locs != NULL) {
			struct Dee_memloc *next;
			do {
				next = cfa_locs->ml_value._v_next;
				cfa_locs->ml_type = MEMLOC_TYPE_HREG;
				cfa_locs->ml_value.v_hreg.r_off   = cfa_locs->ml_value.v_hstack.s_off;
				cfa_locs->ml_value.v_hreg.r_regno = dst_regno;
				Dee_memstate_incrinuse(self->fg_state, dst_regno);
			} while ((cfa_locs = next) != NULL);
		}
		return Dee_function_generator_ghstack_popreg(self, dst_regno);
	}
do_use_mov:
	sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	result    = _Dee_function_generator_gmov_hstackind2reg(self, sp_offset, dst_regno);
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC;
		result = Dee_function_generator_remember_movevalue_hstackind2reg(self, cfa_offset, dst_regno);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_const2reg(struct Dee_function_generator *__restrict self,
                                      DeeObject *value, Dee_host_register_t dst_regno) {
	/* TODO: If "dst_regno" is a known equivalence of "value", do nothing */
	int result = _Dee_function_generator_gmov_const2reg(self, value, dst_regno);
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC;
		result = Dee_function_generator_remember_movevalue_const2reg(self, value, dst_regno);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_const2regind(struct Dee_function_generator *__restrict self,
                                         DeeObject *value, Dee_host_register_t dst_regno,
                                         ptrdiff_t dst_delta) {
	int result = _Dee_function_generator_gmov_const2regind(self, value, dst_regno, dst_delta);
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_const2regind(self, value, dst_regno, dst_delta);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_const2hstackind(struct Dee_function_generator *__restrict self,
                                            DeeObject *value, uintptr_t cfa_offset) {
	ptrdiff_t sp_offset;
	int result;
	/* TODO: If "[#cfa_offset]" is a known equivalence of "value", do nothing */

	/* Check if the value *must* be pushed onto the h-stack. */
	if (Dee_memstate_hstack_mustpush(self->fg_state, cfa_offset)) {
		uintptr_t skip = Dee_memstate_hstack_mustpush_skip(self->fg_state, cfa_offset);
		ASSERT((skip == 0) == !!Dee_memstate_hstack_canpush(self->fg_state, cfa_offset));
		result = Dee_function_generator_ghstack_adjust(self, (ptrdiff_t)skip);
		if likely(result == 0)
			result = Dee_function_generator_ghstack_pushconst(self, value);
		return result;
	}
	sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	result = _Dee_function_generator_gmov_const2hstackind(self, value, sp_offset);
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_const2hstackind(self, value, cfa_offset);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_const2constind(struct Dee_function_generator *__restrict self,
                                           DeeObject *value, DeeObject **p_value) {
	int result = _Dee_function_generator_gmov_const2constind(self, value, p_value);
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_const2constind(self, value, p_value);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_reg2reg(struct Dee_function_generator *__restrict self,
                                    Dee_host_register_t src_regno, Dee_host_register_t dst_regno) {
	/* TODO: If "dst_regno" is a known equivalence of "src_regno", do nothing */
	int result = _Dee_function_generator_gmov_reg2reg(self, src_regno, dst_regno);
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = self->fg_state->ms_rusage[src_regno];
		result = Dee_function_generator_remember_movevalue_reg2reg(self, src_regno, dst_regno);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_regx2reg(struct Dee_function_generator *__restrict self,
                                     Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                     Dee_host_register_t dst_regno) {
	/* TODO: If "dst_regno" is a known equivalence of "src_regno + src_delta", do nothing */
	int result = _Dee_function_generator_gmov_regx2reg(self, src_regno, src_delta, dst_regno);
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = self->fg_state->ms_rusage[src_regno];
		if (src_delta != 0)
			self->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC;
		result = Dee_function_generator_remember_movevalue_regx2reg(self, src_regno, src_delta, dst_regno);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_regind2reg(struct Dee_function_generator *__restrict self,
                                       Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                       Dee_host_register_t dst_regno) {
	/* TODO: If "dst_regno" is a known equivalence of "[src_regno + src_delta]", do nothing */
	int result = _Dee_function_generator_gmov_regind2reg(self, src_regno, src_delta, dst_regno);
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC;
		result = Dee_function_generator_remember_movevalue_regind2reg(self, src_regno, src_delta, dst_regno);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_reg2regind(struct Dee_function_generator *__restrict self,
                                       Dee_host_register_t src_regno,
                                       Dee_host_register_t dst_regno, ptrdiff_t dst_delta) {
	int result = _Dee_function_generator_gmov_reg2regind(self, src_regno, dst_regno, dst_delta);
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_reg2regind(self, src_regno, dst_regno, dst_delta);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_constind2reg(struct Dee_function_generator *__restrict self,
                                         DeeObject **p_value, Dee_host_register_t dst_regno) {
	int result = _Dee_function_generator_gmov_constind2reg(self, p_value, dst_regno);
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC;
		result = Dee_function_generator_remember_movevalue_constind2reg(self, p_value, dst_regno);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_reg2constind(struct Dee_function_generator *__restrict self,
                                         Dee_host_register_t src_regno, DeeObject **p_value) {
	int result = _Dee_function_generator_gmov_reg2constind(self, src_regno, p_value);
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_reg2constind(self, src_regno, p_value);
	return result;
}







INTERN WUNUSED NONNULL((1, 3)) int DCALL
Dee_function_generator_gmov_const2loc(struct Dee_function_generator *__restrict self,
                                      DeeObject *value, struct Dee_memloc const *__restrict dst_loc) {
	/* TODO: If "dst_loc" is a known equivalence of "value", do nothing */
	switch (dst_loc->ml_type) {

	case MEMLOC_TYPE_HSTACKIND: {
		uintptr_t cfa_offset = dst_loc->ml_value.v_hstack.s_cfa;
		DeeObject *final_value = (DeeObject *)((uintptr_t)value - dst_loc->ml_value.v_hstack.s_off);
		return Dee_function_generator_gmov_const2hstackind(self, final_value, cfa_offset);
	}	break;

	case MEMLOC_TYPE_HREGIND: {
		DeeObject *final_value = (DeeObject *)((uintptr_t)value - dst_loc->ml_value.v_hreg.r_voff);
		return Dee_function_generator_gmov_const2regind(self, final_value,
		                                                dst_loc->ml_value.v_hreg.r_regno,
		                                                dst_loc->ml_value.v_hreg.r_off);
	}	break;

	case MEMLOC_TYPE_HREG: {
		DeeObject *final_value = (DeeObject *)((uintptr_t)value - dst_loc->ml_value.v_hreg.r_off);
		return Dee_function_generator_gmov_const2reg(self, final_value, dst_loc->ml_value.v_hreg.r_regno);
	}	break;

	default: {
		Dee_host_register_t temp_regno, not_these[2];
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (MEMLOC_TYPE_HASREG(dst_loc->ml_type))
			not_these[0] = dst_loc->ml_value.v_hreg.r_regno;
		temp_regno = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(temp_regno >= HOST_REGISTER_COUNT)
			goto err;
		if unlikely(Dee_function_generator_gmov_const2reg(self, value, temp_regno))
			goto err;
		return Dee_function_generator_gmov_reg2loc(self, temp_regno, dst_loc);
	}	break;

	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
Dee_function_generator_gmov_hstack2loc(struct Dee_function_generator *__restrict self,
                                       uintptr_t cfa_offset,
                                       struct Dee_memloc const *__restrict dst_loc) {
	switch (dst_loc->ml_type) {

	case MEMLOC_TYPE_HREG: {
		cfa_offset = HA_cfa_offset_PLUS_sp_offset(cfa_offset, -dst_loc->ml_value.v_hreg.r_off);
		return Dee_function_generator_gmov_hstack2reg(self, cfa_offset, dst_loc->ml_value.v_hreg.r_regno);
	}	break;

	default: {
		Dee_host_register_t temp_regno, not_these[2];
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (MEMLOC_TYPE_HASREG(dst_loc->ml_type))
			not_these[0] = dst_loc->ml_value.v_hreg.r_regno;
		temp_regno = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(temp_regno >= HOST_REGISTER_COUNT)
			goto err;
		cfa_offset = HA_cfa_offset_PLUS_sp_offset(cfa_offset, -dst_loc->ml_value.v_hreg.r_voff);
		if unlikely(Dee_function_generator_gmov_hstack2reg(self, cfa_offset, temp_regno))
			goto err;
		return Dee_function_generator_gmov_reg2loc(self, temp_regno, dst_loc);
	}	break;

	}
	__builtin_unreachable();
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
		ptrdiff_t delta_delta = dst_loc->ml_value.v_hstack.s_off - src_delta;
		if (delta_delta != 0) {
			/* Adjust `src_regno' to have the correct value-delta */
			if unlikely(Dee_function_generator_gmov_regx2reg(self, src_regno, delta_delta, src_regno))
				goto err;
		}
		return Dee_function_generator_gmov_reg2hstackind(self, src_regno, cfa_offset);
	}	break;

	case MEMLOC_TYPE_HREGIND: {
		ptrdiff_t delta_delta = dst_loc->ml_value.v_hreg.r_voff - src_delta;
		if (delta_delta != 0) {
			if unlikely(Dee_function_generator_gmov_regx2reg(self, src_regno, delta_delta, src_regno))
				goto err;
		}
		return Dee_function_generator_gmov_reg2regind(self, src_regno,
		                                              dst_loc->ml_value.v_hreg.r_regno,
		                                              dst_loc->ml_value.v_hreg.r_off);
	}	break;

	case MEMLOC_TYPE_HREG:
		return Dee_function_generator_gmov_regx2reg(self, src_regno,
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
	/* TODO: Go through equivalences of "src_loc" and pick the best one for the move */
	switch (src_loc->ml_type) {

	case MEMLOC_TYPE_HSTACKIND: {
		uintptr_t cfa_offset = src_loc->ml_value.v_hstack.s_cfa;
		ptrdiff_t delta_delta = src_loc->ml_value.v_hstack.s_off - dst_delta;
		result = Dee_function_generator_gmov_hstackind2reg(self, cfa_offset, dst_regno);
		if likely(result == 0 && delta_delta != 0)
			result = Dee_function_generator_gmov_regx2reg(self, dst_regno, delta_delta, dst_regno);
	}	break;

	case MEMLOC_TYPE_HSTACK: {
		uintptr_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(src_loc->ml_value.v_hstack.s_cfa, -dst_delta);
		result = Dee_function_generator_gmov_hstack2reg(self, cfa_offset, dst_regno);
	}	break;

	case MEMLOC_TYPE_HREGIND: {
		ptrdiff_t delta_delta = src_loc->ml_value.v_hreg.r_voff - dst_delta;
		result = Dee_function_generator_gmov_regind2reg(self,
		                                                src_loc->ml_value.v_hreg.r_regno,
		                                                src_loc->ml_value.v_hreg.r_off,
		                                                dst_regno);
		if likely(result == 0 && delta_delta != 0)
			result = Dee_function_generator_gmov_regx2reg(self, dst_regno, delta_delta, dst_regno);
	}	break;

	case MEMLOC_TYPE_HREG:
		result = Dee_function_generator_gmov_regx2reg(self,
		                                              src_loc->ml_value.v_hreg.r_regno,
		                                              src_loc->ml_value.v_hreg.r_off - dst_delta,
		                                              dst_regno);
		break;

	case MEMLOC_TYPE_CONST:
		result = Dee_function_generator_gmov_const2reg(self,
		                                               (DeeObject *)((uintptr_t)src_loc->ml_value.v_const -
		                                                             dst_delta),
		                                               dst_regno);
		break;

	case MEMLOC_TYPE_UNDEFINED:
		return 0;

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
	/* TODO: Check if (%dst_regno + dst_delta) is an equivalence of "src_loc" */
	/* TODO: Go through equivalences of "src_loc" and pick the best one for the move */
	switch (src_loc->ml_type) {

	case MEMLOC_TYPE_HSTACKIND: {
		uintptr_t cfa_offset = src_loc->ml_value.v_hstack.s_cfa;
		result = Dee_function_generator_gmov_hstackind2reg(self, cfa_offset, dst_regno);
		*p_dst_delta = src_loc->ml_value.v_hstack.s_off;
	}	break;

	case MEMLOC_TYPE_HSTACK: {
		uintptr_t cfa_offset = src_loc->ml_value.v_hstack.s_cfa;
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		uintptr_t cfa0_offset = Dee_memstate_hstack_sp2cfa(self->fg_state, 0);
		result = Dee_function_generator_gmov_hstack2reg(self, cfa0_offset, dst_regno);
		*p_dst_delta = sp_offset;
	}	break;

	case MEMLOC_TYPE_HREGIND:
		result = Dee_function_generator_gmov_regind2reg(self,
		                                                src_loc->ml_value.v_hreg.r_regno,
		                                                src_loc->ml_value.v_hreg.r_off,
		                                                dst_regno);
		*p_dst_delta = src_loc->ml_value.v_hreg.r_voff;
		break;

	case MEMLOC_TYPE_HREG:
		result = Dee_function_generator_gmov_reg2reg(self, src_loc->ml_value.v_hreg.r_regno, dst_regno);
		*p_dst_delta = src_loc->ml_value.v_hreg.r_off;
		break;

	case MEMLOC_TYPE_CONST:
		result = Dee_function_generator_gmov_const2reg(self, src_loc->ml_value.v_const, dst_regno);
		*p_dst_delta = 0;
		break;

	default:
		result = Dee_function_generator_gmov_loc2regx(self, src_loc, dst_regno, 0);
		*p_dst_delta = 0;
		return result;
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
		result = Dee_function_generator_gmov_regind2reg(self,
		                                                src_loc->ml_value.v_hreg.r_regno,
		                                                src_loc->ml_value.v_hreg.r_off + src_delta,
		                                                dst_regno);
		break;

	case MEMLOC_TYPE_HSTACK: {
		uintptr_t cfa_offset = src_loc->ml_value.v_hstack.s_cfa;
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		result = Dee_function_generator_gmov_hstackind2reg(self, sp_offset + src_delta, dst_regno);
	}	break;

	case MEMLOC_TYPE_CONST: {
		DeeObject **value = (DeeObject **)((uintptr_t)src_loc->ml_value.v_const + src_delta);
		result = Dee_function_generator_gmov_constind2reg(self, value, dst_regno);
	}	break;

	case MEMLOC_TYPE_UNDEFINED:
		return 0;

	default: {
		ptrdiff_t ind_delta;
		result = Dee_function_generator_gmov_loc2regy(self, src_loc, dst_regno, &ind_delta);
		if likely(result == 0)
			result = Dee_function_generator_gmov_regind2reg(self, dst_regno, ind_delta + src_delta, dst_regno);
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
		result = Dee_function_generator_gmov_reg2regind(self, src_regno,
		                                                dst_loc->ml_value.v_hreg.r_regno,
		                                                dst_loc->ml_value.v_hreg.r_off + dst_delta);
		break;

	case MEMLOC_TYPE_HSTACK: {
		uintptr_t cfa_offset = dst_loc->ml_value.v_hstack.s_cfa;
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		result = Dee_function_generator_gmov_reg2hstackind(self, src_regno, sp_offset + dst_delta);
	}	break;

	case MEMLOC_TYPE_CONST: {
		uintptr_t value = (uintptr_t)dst_loc->ml_value.v_const + dst_delta;
		result = Dee_function_generator_gmov_reg2constind(self, src_regno, (DeeObject **)value);
	}	break;

	case MEMLOC_TYPE_UNDEFINED:
		return 0;

	default: {
		Dee_host_register_t temp_regno;
		Dee_host_register_t not_these[2];
		ptrdiff_t ind_delta;
		struct Dee_memequiv *eq;

		/* See if "dst_loc" has a register/const equivalence. */
		eq = Dee_function_generator_remember_getclassof(self, dst_loc);
		if (eq != NULL) {
			struct Dee_memequiv *iter = Dee_memequiv_next(eq);
			do {
				if (iter->meq_loc.meql_type == MEMEQUIV_TYPE_HREG ||
				    iter->meq_loc.meql_type == MEMEQUIV_TYPE_CONST) {
					struct Dee_memloc reg_loc;
					ptrdiff_t val_delta = Dee_memloc_getvaldelta_c0(dst_loc);
					val_delta -= eq->meq_valoff;
					Dee_memequiv_asloc(iter, &reg_loc, val_delta);
					ASSERT(reg_loc.ml_type == MEMLOC_TYPE_HREG ||
					       reg_loc.ml_type == MEMLOC_TYPE_CONST);
					return Dee_function_generator_gmov_reg2locind(self, src_regno, &reg_loc, dst_delta);
				}
			} while ((iter = Dee_memequiv_next(iter)) != eq);
		}

		/* Need to use a temporary register. */
		not_these[0] = src_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		temp_regno = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(temp_regno >= HOST_REGISTER_COUNT)
			goto err;
		if unlikely(Dee_function_generator_gmov_loc2regy(self, dst_loc, temp_regno, &ind_delta))
			goto err;
		result = Dee_function_generator_gmov_reg2regind(self, src_regno, temp_regno, ind_delta + dst_delta);
	}	break;

	}
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_ghstack_pushlocx(struct Dee_function_generator *__restrict self,
                                        struct Dee_memloc *__restrict src_loc,
                                        ptrdiff_t dst_delta) {
	switch (src_loc->ml_type) {

	case MEMLOC_TYPE_HREG: {
		ptrdiff_t final_delta = src_loc->ml_value.v_hreg.r_off - dst_delta;
		if (final_delta != 0) {
			ptrdiff_t writeback_delta = src_loc->ml_value.v_hreg.r_off + final_delta;
			if unlikely(Dee_function_generator_gmov_regx2reg(self,
			                                                 src_loc->ml_value.v_hreg.r_regno,
			                                                 final_delta,
			                                                 src_loc->ml_value.v_hreg.r_regno))
				goto err;
			Dee_memstate_hregs_adjust_delta(self->fg_state, src_loc->ml_value.v_hreg.r_regno, final_delta);
			src_loc->ml_value.v_hreg.r_off = writeback_delta;
		}
		return Dee_function_generator_ghstack_pushreg(self, src_loc->ml_value.v_hreg.r_regno);
	}	break;

	case MEMLOC_TYPE_HREGIND: {
		ptrdiff_t final_delta = src_loc->ml_value.v_hreg.r_voff - dst_delta;
		if (final_delta != 0)
			goto fallback;
		return Dee_function_generator_ghstack_pushregind(self,
		                                                 src_loc->ml_value.v_hreg.r_regno,
		                                                 src_loc->ml_value.v_hreg.r_off);
	}	break;

	case MEMLOC_TYPE_HSTACKIND: {
		uintptr_t cfa_offset = src_loc->ml_value.v_hstack.s_cfa;
		ptrdiff_t final_delta = src_loc->ml_value.v_hstack.s_off - dst_delta;
		if (final_delta != 0)
			goto fallback;
		return Dee_function_generator_ghstack_pushhstackind(self, cfa_offset);
	}	break;

#ifdef Dee_function_generator_ghstack_pushhstack_at_cfa_boundary
	case MEMLOC_TYPE_HSTACK: {
		uintptr_t cfa_offset = src_loc->ml_value.v_hstack.s_cfa;
		if (cfa_offset == self->fg_state->ms_host_cfa_offset) /* Special case: push current CFA offset */
			return Dee_function_generator_ghstack_pushhstack_at_cfa_boundary(self);
		goto fallback;
	}	break;
#endif /* Dee_function_generator_ghstack_pushhstack_at_cfa_boundary */

	case MEMLOC_TYPE_CONST: {
		uintptr_t value = (uintptr_t)src_loc->ml_value.v_const - dst_delta;
		return Dee_function_generator_ghstack_pushconst(self, (DeeObject *)value);
	}	break;

	case MEMLOC_TYPE_UNDEFINED:
		return Dee_function_generator_ghstack_adjust(self, HOST_SIZEOF_POINTER);

	default: {
		Dee_host_register_t temp_regno;
		Dee_host_register_t not_these[2];
fallback:
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (MEMLOC_TYPE_HASREG(src_loc->ml_type))
			not_these[0] = src_loc->ml_value.v_hreg.r_regno;
		temp_regno = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(temp_regno >= HOST_REGISTER_COUNT)
			goto err;
		if unlikely(Dee_function_generator_gmov_loc2regx(self, src_loc, temp_regno, dst_delta))
			goto err;
		return Dee_function_generator_ghstack_pushreg(self, temp_regno);
	}	break;

	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gmov_loc2hstackindx(struct Dee_function_generator *__restrict self,
                                           struct Dee_memloc *__restrict src_loc,
                                           uintptr_t dst_cfa_offset, ptrdiff_t dst_delta) {
#ifdef HOSTASM_STACK_GROWS_DOWN
	if (dst_cfa_offset == self->fg_state->ms_host_cfa_offset + HOST_SIZEOF_POINTER)
#else /* HOSTASM_STACK_GROWS_DOWN */
	if (dst_cfa_offset == self->fg_state->ms_host_cfa_offset)
#endif /* !HOSTASM_STACK_GROWS_DOWN */
	{
		/* Push the value instead! */
		return Dee_function_generator_ghstack_pushlocx(self, src_loc, dst_delta);
	}
	switch (src_loc->ml_type) {

	case MEMLOC_TYPE_HREG: {
		ptrdiff_t final_delta = src_loc->ml_value.v_hreg.r_off - dst_delta;
		if (final_delta != 0) {
			ptrdiff_t writeback_delta = src_loc->ml_value.v_hreg.r_off + final_delta;
			if unlikely(Dee_function_generator_gmov_regx2reg(self,
			                                                 src_loc->ml_value.v_hreg.r_regno,
			                                                 final_delta,
			                                                 src_loc->ml_value.v_hreg.r_regno))
				goto err;
			Dee_memstate_hregs_adjust_delta(self->fg_state, src_loc->ml_value.v_hreg.r_regno, final_delta);
			src_loc->ml_value.v_hreg.r_off = writeback_delta;
		}
		return Dee_function_generator_gmov_reg2hstackind(self, src_loc->ml_value.v_hreg.r_regno, dst_cfa_offset);
	}	break;

	case MEMLOC_TYPE_CONST: {
		uintptr_t value = (uintptr_t)src_loc->ml_value.v_const - dst_delta;
		return Dee_function_generator_gmov_const2hstackind(self, (DeeObject *)value, dst_cfa_offset);
	}	break;

	case MEMLOC_TYPE_UNDEFINED:
		return 0;

	default: {
		Dee_host_register_t temp_regno;
		Dee_host_register_t not_these[2];
/*fallback:*/
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (MEMLOC_TYPE_HASREG(src_loc->ml_type))
			not_these[0] = src_loc->ml_value.v_hreg.r_regno;
		temp_regno = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(temp_regno >= HOST_REGISTER_COUNT)
			goto err;
		if unlikely(Dee_function_generator_gmov_loc2regx(self, src_loc, temp_regno, dst_delta))
			goto err;
		return Dee_function_generator_gmov_reg2hstackind(self, temp_regno, dst_cfa_offset);
	}	break;

	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gmov_loc2constind(struct Dee_function_generator *__restrict self,
                                         struct Dee_memloc *__restrict src_loc,
                                         DeeObject **p_value, ptrdiff_t dst_delta) {
	switch (src_loc->ml_type) {

	case MEMLOC_TYPE_HREG: {
		/*     *p_value + dst_delta = src_loc->ml_value.v_hreg.r_regno + src_loc->ml_value.v_hreg.r_off
		 * <=> *p_value = src_loc->ml_value.v_hreg.r_regno + src_loc->ml_value.v_hreg.r_off - dst_delta */
		ptrdiff_t final_delta = src_loc->ml_value.v_hreg.r_off - dst_delta;
		if (final_delta != 0) {
			ptrdiff_t writeback_delta = src_loc->ml_value.v_hreg.r_off + final_delta;
			if unlikely(Dee_function_generator_gmov_regx2reg(self,
			                                                 src_loc->ml_value.v_hreg.r_regno,
			                                                 final_delta,
			                                                 src_loc->ml_value.v_hreg.r_regno))
				goto err;
			Dee_memstate_hregs_adjust_delta(self->fg_state, src_loc->ml_value.v_hreg.r_regno, final_delta);
			src_loc->ml_value.v_hreg.r_off = writeback_delta;
		}
		return Dee_function_generator_gmov_reg2constind(self, src_loc->ml_value.v_hreg.r_regno, p_value);
	}	break;

	case MEMLOC_TYPE_CONST: {
		uintptr_t value = (uintptr_t)src_loc->ml_value.v_const - dst_delta;
		return Dee_function_generator_gmov_const2constind(self, (DeeObject *)value, p_value);
	}	break;

	case MEMLOC_TYPE_UNDEFINED:
		return 0;

	default: {
		Dee_host_register_t temp_regno;
		Dee_host_register_t not_these[2];
/*fallback:*/
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (MEMLOC_TYPE_HASREG(src_loc->ml_type))
			not_these[0] = src_loc->ml_value.v_hreg.r_regno;
		temp_regno = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(temp_regno >= HOST_REGISTER_COUNT)
			goto err;
		if unlikely(Dee_function_generator_gmov_loc2regx(self, src_loc, temp_regno, dst_delta))
			goto err;
		return Dee_function_generator_gmov_reg2constind(self, temp_regno, p_value);
	}	break;

	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
Dee_function_generator_gmov_const2locind(struct Dee_function_generator *__restrict self, DeeObject *value,
                                         struct Dee_memloc const *__restrict dst_loc, ptrdiff_t dst_delta) {
	int result;
	switch (dst_loc->ml_type) {

	case MEMLOC_TYPE_HREG:
		result = Dee_function_generator_gmov_const2regind(self, value,
		                                                  dst_loc->ml_value.v_hreg.r_regno,
		                                                  dst_loc->ml_value.v_hreg.r_off + dst_delta);
		break;

	case MEMLOC_TYPE_HSTACK: {
		uintptr_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(dst_loc->ml_value.v_hstack.s_cfa, dst_delta);
		result = Dee_function_generator_gmov_const2hstackind(self, value, cfa_offset);
	}	break;

	case MEMLOC_TYPE_CONST: {
		uintptr_t dst_value = (uintptr_t)dst_loc->ml_value.v_const + dst_delta;
		result = Dee_function_generator_gmov_const2constind(self, value, (DeeObject **)dst_value);
	}	break;

	case MEMLOC_TYPE_UNDEFINED:
		return 0;

	default: {
		Dee_host_register_t temp_regno;
		ptrdiff_t ind_delta;
		struct Dee_memequiv *eq;

		/* See if "dst_loc" has a register/const equivalence. */
		eq = Dee_function_generator_remember_getclassof(self, dst_loc);
		if (eq != NULL) {
			struct Dee_memequiv *iter = Dee_memequiv_next(eq);
			do {
				if (iter->meq_loc.meql_type == MEMEQUIV_TYPE_HREG ||
				    iter->meq_loc.meql_type == MEMEQUIV_TYPE_CONST) {
					struct Dee_memloc reg_loc;
					ptrdiff_t val_delta = Dee_memloc_getvaldelta_c0(dst_loc);
					val_delta -= eq->meq_valoff;
					Dee_memequiv_asloc(iter, &reg_loc, val_delta);
					ASSERT(reg_loc.ml_type == MEMLOC_TYPE_HREG ||
					       reg_loc.ml_type == MEMLOC_TYPE_CONST);
					return Dee_function_generator_gmov_const2locind(self, value, &reg_loc, dst_delta);
				}
			} while ((iter = Dee_memequiv_next(iter)) != eq);
		}

		/* Need to use a temporary register. */
		temp_regno = Dee_function_generator_gallocreg(self, NULL);
		if unlikely(temp_regno >= HOST_REGISTER_COUNT)
			goto err;
		if unlikely(Dee_function_generator_gmov_loc2regy(self, dst_loc, temp_regno, &ind_delta))
			goto err;
		result = Dee_function_generator_gmov_const2regind(self, value, temp_regno, ind_delta + dst_delta);
	}	break;

	}
	return result;
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
	alloc_delta = (ptrdiff_t)self->fg_state->ms_host_cfa_offset;
	if (alloc_delta != 0) {
		result = Dee_function_generator_ghstack_adjust(self, -alloc_delta);
		if unlikely(result != 0)
			goto done;
	}

	/* Generate the arch-specific return instruction sequence. */
	result = _Dee_function_generator_gret(self);
done:
	return result;
}



/* Helpers for transforming locations into deemon boolean objects. */

/* dst_regno = (src_loc + src_delta) <CMP> 0 ? 1 : 0; */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gmorph_loc2reg01(struct Dee_function_generator *__restrict self,
                                        struct Dee_memloc *src_loc, ptrdiff_t src_delta,
                                        unsigned int cmp, Dee_host_register_t dst_regno) {
	switch (src_loc->ml_type) {
	default: {
		Dee_host_register_t not_these[2];
		not_these[0] = dst_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		if unlikely(Dee_function_generator_greg(self, src_loc, not_these))
			goto err;
	}	ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return Dee_function_generator_gmorph_regx2reg01(self,
		                                                src_loc->ml_value.v_hreg.r_regno,
		                                                src_loc->ml_value.v_hreg.r_off + src_delta,
		                                                cmp, dst_regno);
	case MEMLOC_TYPE_HREGIND:
		return Dee_function_generator_gmorph_regind2reg01(self,
		                                                  src_loc->ml_value.v_hreg.r_regno,
		                                                  src_loc->ml_value.v_hreg.r_off,
		                                                  src_loc->ml_value.v_hreg.r_voff + src_delta,
		                                                  cmp, dst_regno);
	case MEMLOC_TYPE_HSTACKIND:
		return Dee_function_generator_gmorph_hstackind2reg01(self,
		                                                     Dee_memstate_hstack_cfa2sp(self->fg_state, src_loc->ml_value.v_hstack.s_cfa),
		                                                     src_loc->ml_value.v_hstack.s_off + src_delta,
		                                                     cmp, dst_regno);
	}
	__builtin_unreachable();
err:
	return -1;
}

/* dst_regno = (src_loc + src_delta) <CMP> rhs_regno ? 1 : 0; */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gmorph_locCreg2reg01(struct Dee_function_generator *__restrict self,
                                            struct Dee_memloc *src_loc, ptrdiff_t src_delta,
                                            unsigned int cmp, Dee_host_register_t rhs_regno,
                                            Dee_host_register_t dst_regno) {
	switch (src_loc->ml_type) {
	default: {
		Dee_host_register_t not_these[3];
		not_these[0] = rhs_regno;
		not_these[1] = dst_regno;
		not_these[2] = HOST_REGISTER_COUNT;
		if unlikely(Dee_function_generator_greg(self, src_loc, not_these))
			goto err;
	}	ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return Dee_function_generator_gmorph_regxCreg2reg01(self,
		                                                    src_loc->ml_value.v_hreg.r_regno,
		                                                    src_loc->ml_value.v_hreg.r_off + src_delta,
		                                                    cmp, rhs_regno, dst_regno);
	case MEMLOC_TYPE_HREGIND:
		return Dee_function_generator_gmorph_regindCreg2reg01(self,
		                                                      src_loc->ml_value.v_hreg.r_regno,
		                                                      src_loc->ml_value.v_hreg.r_off,
		                                                      src_loc->ml_value.v_hreg.r_voff + src_delta,
		                                                      cmp, rhs_regno, dst_regno);
	case MEMLOC_TYPE_HSTACKIND:
		return Dee_function_generator_gmorph_hstackindCreg2reg01(self,
		                                                         Dee_memstate_hstack_cfa2sp(self->fg_state, src_loc->ml_value.v_hstack.s_cfa),
		                                                         src_loc->ml_value.v_hstack.s_off + src_delta,
		                                                         cmp, rhs_regno, dst_regno);
	}
	__builtin_unreachable();
err:
	return -1;
}



/* Flip the calling convention such that LHS and RHS can be swapped. */
PRIVATE NONNULL((1)) unsigned int DCALL
flip_gmorphbool_cc(unsigned int cc, ptrdiff_t *__restrict p_src_delta) {
	*p_src_delta = -*p_src_delta;
	switch (cc) {
	case GMORPHBOOL_CC_EQ:
	case GMORPHBOOL_CC_NE:
		return cc;
	case GMORPHBOOL_CC_LO:
		/*     LHS + *p_src_delta < RHS
		 * <=> RHS > LHS + *p_src_delta
		 * <=> RHS - *p_src_delta > LHS
		 * <=> LHS < RHS - *p_src_delta */
		return GMORPHBOOL_CC_GR;
	case GMORPHBOOL_CC_GR:
		return GMORPHBOOL_CC_LO;
	default: __builtin_unreachable();
	}
}

/* dst_regno = (src_loc + src_delta) <CMP> rhs_loc ? 1 : 0; */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gmorph_locCloc2reg01(struct Dee_function_generator *__restrict self,
                                            struct Dee_memloc *src_loc, ptrdiff_t src_delta,
                                            unsigned int cmp, struct Dee_memloc *rhs_loc,
                                            Dee_host_register_t dst_regno) {
	if (rhs_loc->ml_type != MEMLOC_TYPE_HREG) {
		if (src_loc->ml_type == MEMLOC_TYPE_HREG) {
			/* Flip operands so the register appears in "rhs_loc" */
			struct Dee_memloc *temp;
			cmp = flip_gmorphbool_cc(cmp, &src_delta);
			temp    = src_loc;
			src_loc = rhs_loc;
			rhs_loc = temp;
		} else {
			/* Force "rhs_loc" into a register. */
			Dee_host_register_t not_these[3];
			not_these[0] = dst_regno;
			not_these[1] = HOST_REGISTER_COUNT;
			not_these[2] = HOST_REGISTER_COUNT;
			if (MEMLOC_TYPE_HASREG(src_loc->ml_type))
				not_these[1] = src_loc->ml_value.v_hreg.r_regno;
			if unlikely(Dee_function_generator_greg(self, rhs_loc, not_these))
				goto err;
		}
	}
	ASSERT(rhs_loc->ml_type == MEMLOC_TYPE_HREG);
	return Dee_function_generator_gmorph_locCreg2reg01(self, src_loc,
	                                                   src_delta - rhs_loc->ml_value.v_hreg.r_off, cmp,
	                                                   rhs_loc->ml_value.v_hreg.r_regno, dst_regno);
err:
	return -1;
}


/* dst_regno = &Dee_FalseTrue[(src_loc + src_delta) <CMP> 0 ? 1 : 0] - *p_dst_delta; */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
Dee_function_generator_gmorph_loc2regbooly(struct Dee_function_generator *__restrict self,
                                           struct Dee_memloc *src_loc, ptrdiff_t src_delta,
                                           unsigned int cmp, Dee_host_register_t dst_regno,
                                           ptrdiff_t *__restrict p_dst_delta) {
	int result = Dee_function_generator_gmorph_loc2reg01(self, src_loc, src_delta, cmp, dst_regno);
	if likely(result == 0) {
		struct Dee_memloc uloc;
		uloc.ml_type = MEMLOC_TYPE_HREG;
		uloc.ml_value.v_hreg.r_regno = dst_regno;
		uloc.ml_value.v_hreg.r_off = 0;
		result = Dee_function_generator_gmorph_loc012regbooly(self, &uloc, 0, dst_regno, p_dst_delta);
	}
	return result;
}

#ifndef HAVE__Dee_host_section_gmorph_reg012regbool
/* dst_regno = &Dee_FalseTrue[src_regno + src_delta] - *p_dst_delta; */
INTERN WUNUSED NONNULL((1, 5)) int DCALL
Dee_function_generator_gmorph_reg012regbooly(struct Dee_function_generator *__restrict self,
                                             Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                             Dee_host_register_t dst_regno, ptrdiff_t *__restrict p_dst_delta) {
	if unlikely(_Dee_host_section_gumul_regconst2reg(self->fg_sect, src_regno,
	                                                 sizeof(DeeBoolObject), dst_regno))
		goto err;
	src_delta *= sizeof(DeeBoolObject);
	src_delta += (ptrdiff_t)(uintptr_t)Dee_FalseTrue;
	*p_dst_delta = 0;
	return _Dee_host_section_gmov_regx2reg(self->fg_sect, dst_regno, src_delta, dst_regno);
err:
	return -1;
}
#endif /* !HAVE__Dee_host_section_gmorph_reg012regbool */

/* dst_regno = &Dee_FalseTrue[src_loc + src_delta] - *p_dst_delta; */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
Dee_function_generator_gmorph_loc012regbooly(struct Dee_function_generator *__restrict self,
                                             struct Dee_memloc *src_loc, ptrdiff_t src_delta,
                                             Dee_host_register_t dst_regno, ptrdiff_t *__restrict p_dst_delta) {
	switch (src_loc->ml_type) {
	case MEMLOC_TYPE_HREG:
		return Dee_function_generator_gmorph_reg012regbooly(self,
		                                                    src_loc->ml_value.v_hreg.r_regno,
		                                                    src_loc->ml_value.v_hreg.r_off + src_delta,
		                                                    dst_regno, p_dst_delta);

	case MEMLOC_TYPE_CONST: {
		DeeObject *value = Dee_False;
		if (((uintptr_t)src_loc->ml_value.v_const + src_delta) != 0)
			value = Dee_True;
		*p_dst_delta = 0;
		return Dee_function_generator_gmov_const2reg(self, value, dst_regno);
	}	break;

	case MEMLOC_TYPE_UNDEFINED:
		*p_dst_delta = 0;
		return Dee_function_generator_gmov_const2reg(self, Dee_False, dst_regno);

	default: {
		ptrdiff_t dst_delta;
		if unlikely(Dee_function_generator_gmov_loc2regy(self, src_loc, dst_regno, &dst_delta))
			goto err;
		src_delta -= dst_delta;
		return Dee_function_generator_gmorph_reg012regbooly(self, dst_regno, src_delta, dst_regno, p_dst_delta);
	}	break;

	}
	__builtin_unreachable();
err:
	return -1;
}



#if defined(HOSTASM_X86) && !defined(HOSTASM_X86_64)
#define HAVE_try_restore_xloc_arg_cfa_offset

/* On i386, caller-argument locals don't have to be flushed to the stack.
 * Instead, if you try to flush a register that's been populated with one
 * of the function's caller-arguments, no code needs to be generated and
 * the CFA offset can just be reset to point at the argument again. */
PRIVATE WUNUSED NONNULL((1)) uintptr_t DCALL
try_restore_xloc_arg_cfa_offset(struct Dee_function_generator *__restrict self,
                                Dee_host_register_t regno) {
#define MEMSTATE_XLOCAL_A_MIN MEMSTATE_XLOCAL_A_THIS
#define MEMSTATE_XLOCAL_A_MAX MEMSTATE_XLOCAL_A_KW
	Dee_lid_t i, xloc_base = self->fg_assembler->fa_localc;
	struct Dee_memstate *state = self->fg_state;
	for (i = MEMSTATE_XLOCAL_A_MIN; i <= MEMSTATE_XLOCAL_A_MAX; ++i) {
		struct Dee_memloc *xloc = &state->ms_localv[xloc_base + i];
		if (xloc->ml_type == MEMLOC_TYPE_HREG &&
		    xloc->ml_value.v_hreg.r_regno == regno) {
			uintptr_t cfa_offset;
			Dee_hostfunc_cc_t cc = self->fg_assembler->fa_cc;
			size_t true_argi = 0;
			switch (i) {
			case MEMSTATE_XLOCAL_A_THIS:
				ASSERT(cc & HOSTFUNC_CC_F_THIS);
				break;
			case MEMSTATE_XLOCAL_A_ARGC:
				ASSERT(!(cc & HOSTFUNC_CC_F_TUPLE));
				if (cc & HOSTFUNC_CC_F_THIS)
					++true_argi;
				break;
			case MEMSTATE_XLOCAL_A_ARGV: /* or `MEMSTATE_XLOCAL_A_ARGS' */
				if (cc & HOSTFUNC_CC_F_THIS)
					++true_argi;
				if (!(cc & HOSTFUNC_CC_F_TUPLE))
					++true_argi;
				break;
			case MEMSTATE_XLOCAL_A_KW:
				ASSERT(cc & HOSTFUNC_CC_F_KW);
				if (cc & HOSTFUNC_CC_F_THIS)
					++true_argi;
				if (cc & HOSTFUNC_CC_F_TUPLE)
					++true_argi;
				++true_argi;
				break;
			default: __builtin_unreachable();
			}
			cfa_offset = (uintptr_t)(-(ptrdiff_t)((true_argi + 1) * HOST_SIZEOF_POINTER));
			return cfa_offset;
		}
	}
	return (uintptr_t)-1;
}
#endif /* HOSTASM_X86 && !HOSTASM_X86_64 */

/* Push/move `regno' onto the host stack, returning the CFA offset of the target location. */
PRIVATE WUNUSED NONNULL((1)) uintptr_t DCALL
Dee_function_generator_gflushreg(struct Dee_function_generator *__restrict self,
                                 Dee_host_register_t regno) {
	uintptr_t cfa_offset;
	ASSERT(!Dee_memstate_isshared(self->fg_state));
#ifdef HAVE_try_restore_xloc_arg_cfa_offset
	cfa_offset = try_restore_xloc_arg_cfa_offset(self, regno);
	if (cfa_offset != (uintptr_t)-1)
		return cfa_offset;
#endif /* HAVE_try_restore_xloc_arg_cfa_offset */
	cfa_offset = Dee_memstate_hstack_find(self->fg_state,
	                                      self->fg_state_hstack_res,
	                                      HOST_SIZEOF_POINTER);
	if (cfa_offset != (uintptr_t)-1) {
		if unlikely(Dee_function_generator_gmov_reg2hstackind(self, regno, cfa_offset))
			goto err;
	} else {
		/* Allocate more stack space. */
		if unlikely(Dee_function_generator_ghstack_pushreg(self, regno))
			goto err;
		cfa_offset = self->fg_state->ms_host_cfa_offset;
#ifndef HOSTASM_STACK_GROWS_DOWN
		cfa_offset -= HOST_SIZEOF_POINTER;
#endif /* HOSTASM_STACK_GROWS_DOWN */
	}
	return cfa_offset;
err:
	return (uintptr_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gflushregind(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc *flush_loc) {
	struct Dee_memloc *loc;
	struct Dee_memstate *state = self->fg_state;
	Dee_host_register_t regno = flush_loc->ml_value.v_hreg.r_regno;
	ptrdiff_t off = flush_loc->ml_value.v_hreg.r_off;
	uintptr_t cfa_offset;
	ASSERT(flush_loc->ml_type == MEMLOC_TYPE_HREGIND);
	cfa_offset = Dee_memstate_hstack_find(self->fg_state,
	                                      self->fg_state_hstack_res,
	                                      HOST_SIZEOF_POINTER);
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
		if unlikely(Dee_function_generator_ghstack_pushregind(self, regno, off))
			goto err;
		cfa_offset = self->fg_state->ms_host_cfa_offset;
#ifndef HOSTASM_STACK_GROWS_DOWN
		cfa_offset -= HOST_SIZEOF_POINTER;
#endif /* HOSTASM_STACK_GROWS_DOWN */
	}

	/* Convert all locations that use `MEMLOC_TYPE_HREGIND:regno:off' to `MEMLOC_TYPE_HSTACKIND' */
	Dee_memstate_foreach(loc, state) {
		if (loc->ml_type == MEMLOC_TYPE_HREGIND &&
		    loc->ml_value.v_hreg.r_regno == regno &&
		    loc->ml_value.v_hreg.r_off == off) {
			loc->ml_type = MEMLOC_TYPE_HSTACKIND;
			loc->ml_value.v_hstack.s_off = loc->ml_value.v_hreg.r_voff;
			loc->ml_value.v_hstack.s_cfa = cfa_offset;
			Dee_memstate_decrinuse(state, regno);
		}
	}
	Dee_memstate_foreach_end;

	ASSERT(flush_loc->ml_type == MEMLOC_TYPE_HSTACKIND);
	return 0;
err:
	return -1;
}

/* Generate code to flush all registers used by the deemon stack/locals into the host stack.
 * NOTE: Usage-registers are cleared by arch-specific code (e.g. `Dee_function_generator_gcallapi()')
 * @param: ignore_top_n_stack_if_not_ref: From the top-most N stack locations, ignore any
 *                                        that don't contain object references.
 * @param: only_if_reference: Only flush locations that contain references. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gflushregs(struct Dee_function_generator *__restrict self,
                                  Dee_vstackaddr_t ignore_top_n_stack_if_not_ref,
                                  bool only_if_reference) {
	Dee_lid_t i;
	uintptr_t register_cfa[HOST_REGISTER_COUNT];
	struct Dee_memstate *state = self->fg_state;
	ASSERT(!Dee_memstate_isshared(state));

	/* Figure out which registers are in use, and assign them CFA offsets. */
	for (i = 0; i < HOST_REGISTER_COUNT; ++i)
		register_cfa[i] = (uintptr_t)-1;
	for (i = 0; i < state->ms_localc; ++i) {
		struct Dee_memloc *loc = &state->ms_localv[i];
		if (only_if_reference && (loc->ml_flags & MEMLOC_F_NOREF))
			continue;
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
		if (loc->ml_flags & MEMLOC_F_NOREF) {
			if (i >= (Dee_vstackaddr_t)(state->ms_stackc - ignore_top_n_stack_if_not_ref))
				continue; /* Slot contains no reference and is in top-most n of stack. */
			if (only_if_reference)
				continue;
		}
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
	Dee_lid_t i;
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
		Dee_memstate_decrinuse(state, loc->ml_value.v_hreg.r_regno);
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
		if (!MEMLOC_TYPE_HASREG(loc->ml_type))
			continue;
		result = loc->ml_value.v_hreg.r_regno;
		if (nullable_host_register_list_contains(not_these, result))
			continue;

		/* Flush this one! */
		cfa_offset = Dee_function_generator_gflushreg(self, result);
		if unlikely(cfa_offset == (uintptr_t)-1)
			goto err;
		Dee_memstate_decrinuse(state, loc->ml_value.v_hreg.r_regno);
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


PRIVATE WUNUSED NONNULL((1)) int DCALL
gmov_usage2reg(struct Dee_function_generator *__restrict self,
               Dee_host_regusage_t usage,
               Dee_host_register_t result_regno) {
	/* TODO */
	(void)self;
	(void)usage;
	(void)result_regno;
	return DeeError_Throwf(&DeeError_IllegalInstruction,
	                       "Unknown usage code: %u",
	                       (unsigned int)usage);
}

/* Helper that returns a register that's been populated for `usage' */
INTERN WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_function_generator_gusagereg(struct Dee_function_generator *__restrict self,
                                 Dee_host_regusage_t usage,
                                 Dee_host_register_t const *dont_alloc_these) {
	Dee_host_register_t regno;
	regno = Dee_memstate_hregs_find_usage(self->fg_state, usage);
	if (regno >= HOST_REGISTER_COUNT) {
		regno = Dee_function_generator_gallocreg(self, dont_alloc_these);
		if likely(regno < HOST_REGISTER_COUNT) {
			if unlikely(gmov_usage2reg(self, usage, regno))
				goto err;
			self->fg_state->ms_rusage[regno] = usage;
		}
	}
	return regno;
err:
	return HOST_REGISTER_COUNT;
}





/* Generate code to assert that location `loc' is non-NULL:
 * >> Dee_function_generator_gassert_bound(self, loc, instr, NULL, lid, NULL, NULL);
 * >> Dee_function_generator_gassert_bound(self, loc, instr, mod, gid, NULL, NULL);
 * @param: opt_endread_before_throw: When non-NULL, emit `Dee_function_generator_grwlock_endread()'
 *                                   before the `Dee_function_generator_gthrow_*_unbound' code. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gassert_bound(struct Dee_function_generator *__restrict self,
                                     struct Dee_memloc *loc, Dee_instruction_t const *instr,
                                     struct Dee_module_object *mod, uint16_t id,
                                     Dee_atomic_rwlock_t *opt_endread_before_throw,
                                     Dee_atomic_rwlock_t *opt_endwrite_before_throw) {
	int temp;
	DREF struct Dee_memstate *saved_state;
	struct Dee_host_symbol *target;
	struct Dee_host_section *text_sect;
	struct Dee_host_section *cold_sect;
	text_sect = self->fg_sect;
	cold_sect = text_sect;
	if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE))
		cold_sect = &self->fg_block->bb_hcold;
	target = Dee_function_generator_newsym_named(self, text_sect == cold_sect ? ".Lbound" : ".Lunbound");
	if unlikely(!target)
		goto err;
	if unlikely(text_sect == cold_sect
	            ? Dee_function_generator_gjnz(self, loc, target)
	            : Dee_function_generator_gjz(self, loc, target))
		goto err;
	saved_state = self->fg_state;
	Dee_memstate_incref(saved_state);
	if unlikely(Dee_function_generator_state_dounshare(self))
		goto err_saved_state;

	self->fg_sect = cold_sect;
	if (text_sect != cold_sect) {
		HA_printf(".section .cold\n");
		Dee_host_symbol_setsect(target, cold_sect);
	}

	/* Location isn't bound -> generate code to throw an exception. */
	if (opt_endwrite_before_throw != NULL &&
	    unlikely(Dee_function_generator_grwlock_endwrite_const(self, opt_endwrite_before_throw)))
		goto err_saved_state;
	if (opt_endread_before_throw != NULL &&
	    unlikely(Dee_function_generator_grwlock_endread_const(self, opt_endread_before_throw)))
		goto err_saved_state;
	if (mod) {
		temp = Dee_function_generator_gthrow_global_unbound(self, mod, id);
	} else {
		temp = Dee_function_generator_gthrow_local_unbound(self, instr, id);
	}
	if unlikely(temp)
		goto err_saved_state;

	/* Switch back to the original section, and restore the saved mem-state. */
	Dee_memstate_decref(self->fg_state);
	self->fg_state = saved_state;
	self->fg_sect = text_sect;

	ASSERT((text_sect == cold_sect) == !!Dee_host_symbol_isdefined(target));
	if (text_sect != cold_sect) {
		HA_printf(".section .text\n");
	} else {
		Dee_host_symbol_setsect(target, text_sect);
	}
	return 0;
err_saved_state:
	Dee_memstate_decref(self->fg_state);
err:
	return -1;
}

/* Generate code to throw an error indicating that local variable `lid'
 * is unbound. This includes any necessary jump for the purpose of entering
 * an exception handler. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gthrow_arg_unbound(struct Dee_function_generator *__restrict self,
                                          Dee_instruction_t const *instr, Dee_aid_t aid) {
	if unlikely(Dee_function_generator_vpush_const(self, self->fg_assembler->fa_code))
		goto err;
	if unlikely(Dee_function_generator_vpush_addr(self, instr))
		goto err;
	if unlikely(Dee_function_generator_vpush_imm16(self, aid))
		goto err;
	return Dee_function_generator_vcallapi(self, &libhostasm_rt_err_unbound_arg, VCALL_CC_EXCEPT, 3);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gthrow_local_unbound(struct Dee_function_generator *__restrict self,
                                            Dee_instruction_t const *instr, Dee_ulid_t lid) {
	if unlikely(Dee_function_generator_vpush_const(self, self->fg_assembler->fa_code))
		goto err;
	if unlikely(Dee_function_generator_vpush_addr(self, instr))
		goto err;
	if unlikely(Dee_function_generator_vpush_imm16(self, lid))
		goto err;
	return Dee_function_generator_vcallapi(self, &libhostasm_rt_err_unbound_local, VCALL_CC_EXCEPT, 3);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gthrow_global_unbound(struct Dee_function_generator *__restrict self,
                                             struct Dee_module_object *mod, uint16_t gid) {
	if unlikely(Dee_function_generator_vpush_const(self, mod))
		goto err;
	if unlikely(Dee_function_generator_vpush_imm16(self, gid))
		goto err;
	return Dee_function_generator_vcallapi(self, &libhostasm_rt_err_unbound_global, VCALL_CC_EXCEPT, 2);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
do_gjz_except(struct Dee_function_generator *__restrict self,
              struct Dee_memloc *loc) {
	struct Dee_except_exitinfo *info;
	if (loc->ml_type == MEMLOC_TYPE_CONST && loc->ml_value.v_const != (DeeObject *)0)
		return 0;
	info = Dee_function_generator_except_exit(self);
	if unlikely(!info)
		goto err;
	{
		Dee_function_generator_DEFINE_Dee_host_symbol_section(self, err, Lexcept, &info->exi_block->bb_htext, 0);
		if (loc->ml_type == MEMLOC_TYPE_CONST)
			return Dee_function_generator_gjmp(self, Lexcept);
		return Dee_function_generator_gjz(self, loc, Lexcept);
	}
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
do_gjnz_except(struct Dee_function_generator *__restrict self,
               struct Dee_memloc *loc) {
	struct Dee_except_exitinfo *info;
	if (loc->ml_type == MEMLOC_TYPE_CONST && loc->ml_value.v_const == (DeeObject *)0)
		return 0;
	info = Dee_function_generator_except_exit(self);
	if unlikely(!info)
		goto err;
	{
		Dee_function_generator_DEFINE_Dee_host_symbol_section(self, err, Lexcept, &info->exi_block->bb_htext, 0);
		if (loc->ml_type == MEMLOC_TYPE_CONST)
			return Dee_function_generator_gjmp(self, Lexcept);
		return Dee_function_generator_gjnz(self, loc, Lexcept);
	}
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
do_gjcmp_except(struct Dee_function_generator *__restrict self,
                struct Dee_memloc *loc, intptr_t threshold,
                unsigned int flags) {
	struct Dee_except_exitinfo *info;
	struct Dee_memloc threshold_loc;
	info = Dee_function_generator_except_exit(self);
	if unlikely(!info)
		goto err;
	threshold_loc.ml_type = MEMLOC_TYPE_CONST;
	threshold_loc.ml_value.v_const = (DeeObject *)(uintptr_t)threshold;
	{
		Dee_function_generator_DEFINE_Dee_host_symbol_section(self, err, Lexcept, &info->exi_block->bb_htext, 0);
		return Dee_function_generator_gjcmp(self, loc, &threshold_loc,
		                                    !(flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_UNSIGNED),
		                                    (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_LO) ? Lexcept : NULL,
		                                    (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_EQ) ? Lexcept : NULL,
		                                    (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_GR) ? Lexcept : NULL);
	}
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
do_gjmp_except(struct Dee_function_generator *__restrict self) {
	struct Dee_except_exitinfo *info;
	info = Dee_function_generator_except_exit(self);
	if unlikely(!info)
		goto err;
	{
		Dee_function_generator_DEFINE_Dee_host_symbol_section(self, err, Lexcept, &info->exi_block->bb_htext, 0);
		return Dee_function_generator_gjmp(self, Lexcept);
	}
err:
	return -1;
}

/* save the current mem-state and execute injected exception cleanup. */
PRIVATE WUNUSED NONNULL((1)) DREF struct Dee_memstate *DCALL
gsave_state_and_do_exceptinject(struct Dee_function_generator *__restrict self) {
	struct Dee_function_exceptinject *chain;
	DREF struct Dee_memstate *saved_state;
	ASSERT(self->fg_exceptinject != NULL);

	/* Generate code that needed for custom exception handling. */
	saved_state = self->fg_state;
	Dee_memstate_incref(saved_state);
	chain = self->fg_exceptinject;
	do {
		Dee_vstackaddr_t n_pop;
		ASSERT(self->fg_state->ms_stackc >= chain->fei_stack);
		n_pop = self->fg_state->ms_stackc - chain->fei_stack;
		if (n_pop != 0 && unlikely(Dee_function_generator_vpopmany(self, n_pop)))
			goto err_saved_state;
		if unlikely((*chain->fei_inject)(self, chain))
			goto err_saved_state;
	} while ((chain = chain->fei_next) != NULL);
	return saved_state;
err_saved_state:
	Dee_memstate_decref(self->fg_state);
/*err:*/
	return NULL;
}





/* Generate checks to enter exception handling mode. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gjz_except(struct Dee_function_generator *__restrict self,
                                  struct Dee_memloc *loc) {
	if likely(self->fg_exceptinject == NULL)
		return do_gjz_except(self, loc);
	return Dee_function_generator_gjeq_except(self, loc, 0);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gjnz_except(struct Dee_function_generator *__restrict self,
                                   struct Dee_memloc *loc) {
	if likely(self->fg_exceptinject == NULL)
		return do_gjnz_except(self, loc);
	return Dee_function_generator_gjne_except(self, loc, 0);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
do_slow_gjcmp_except(struct Dee_function_generator *__restrict self,
                     struct Dee_memloc *loc, intptr_t threshold,
                     unsigned int flags) {
	bool signed_cmp = !(flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_UNSIGNED);
	struct Dee_host_section *text = self->fg_sect;
	struct Dee_host_section *cold = &self->fg_block->bb_hcold;
	struct Dee_memloc compare_value_loc;
	compare_value_loc.ml_type = MEMLOC_TYPE_CONST;
	compare_value_loc.ml_value.v_const = (DeeObject *)(uintptr_t)threshold;
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
		cold = text;
	if (cold == text) {
		struct Dee_host_symbol *Lno_except;
		Lno_except = Dee_function_generator_newsym_named(self, ".Lno_except");
		if unlikely(!Lno_except)
			goto err;
		if unlikely(Dee_function_generator_gjcmp(self, loc, &compare_value_loc, signed_cmp,
		                                         (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_LO) ? NULL : Lno_except,
		                                         (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_EQ) ? NULL : Lno_except,
		                                         (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_GR) ? NULL : Lno_except))
			goto err;
		if unlikely(Dee_function_generator_gjmp_except(self))
			goto err;
		Dee_host_symbol_setsect(Lno_except, self->fg_sect);
	} else {
		struct Dee_host_symbol *Ldo_except;
		Ldo_except = Dee_function_generator_newsym_named(self, ".Ldo_except");
		if unlikely(!Ldo_except)
			goto err;
		if unlikely(Dee_function_generator_gjcmp(self, loc, &compare_value_loc, false,
		                                         (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_LO) ? Ldo_except : NULL,
		                                         (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_EQ) ? Ldo_except : NULL,
		                                         (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_GR) ? Ldo_except : NULL))
			goto err;
		HA_printf(".section .cold\n");
		self->fg_sect = cold;
		Dee_host_symbol_setsect(Ldo_except, self->fg_sect);
		if unlikely(Dee_function_generator_gjmp_except(self))
			goto err;
		HA_printf(".section .text\n");
		self->fg_sect = text;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gjcmp_except(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc *loc, intptr_t threshold,
                                    unsigned int flags) {
	if likely(self->fg_exceptinject == NULL)
		return do_gjcmp_except(self, loc, threshold, flags);
	if (loc->ml_type == MEMLOC_TYPE_CONST) {
		bool should_jump_except = false;
		intptr_t lhs = (intptr_t)(uintptr_t)loc->ml_value.v_const;
		if (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_LO)
			should_jump_except |= (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_UNSIGNED) ? ((uintptr_t)lhs < (uintptr_t)threshold) : (lhs < threshold);
		if (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_EQ)
			should_jump_except |= lhs == threshold;
		if (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_GR)
			should_jump_except |= (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_UNSIGNED) ? ((uintptr_t)lhs > (uintptr_t)threshold) : (lhs > threshold);
		return should_jump_except
		       ? Dee_function_generator_gjmp_except(self)
		       : 0;
	}
	return do_slow_gjcmp_except(self, loc, threshold, flags);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjmp_except(struct Dee_function_generator *__restrict self) {
	DREF struct Dee_memstate *saved_state;
	if unlikely(self->fg_exceptinject == NULL)
		return do_gjmp_except(self);
	saved_state = gsave_state_and_do_exceptinject(self);
	if unlikely(!saved_state)
		goto err;
	if unlikely(do_gjmp_except(self))
		goto err_saved_state;
	Dee_memstate_decref(self->fg_state);
	self->fg_state = saved_state;
	return 0;
err_saved_state:
	Dee_memstate_decref(self->fg_state);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gcallapi(struct Dee_function_generator *__restrict self,
                                void const *api_function, size_t argc,
                                struct Dee_memloc *argv) {
	int result = _Dee_function_generator_gcallapi(self, api_function, argc, argv);
	if likely(result == 0)
		Dee_function_generator_remember_undefined_allregs(self);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gcalldynapi(struct Dee_function_generator *__restrict self,
                                   struct Dee_memloc *api_function_loc,
                                   size_t argc, struct Dee_memloc *argv) {
	int result;
	switch (api_function_loc->ml_type) {
	default: {
		size_t i;
		Dee_host_register_t regno, not_these[HOST_REGISTER_COUNT + 1];
		bitset_t not_these_bitset[_bitset_sizeof(HOST_REGISTER_COUNT)];
fallback:
		bitset_clearall(not_these_bitset, HOST_REGISTER_COUNT);
		for (i = 0; i < argc; ++i) {
			struct Dee_memloc *arg = &argv[i];
			if (MEMLOC_TYPE_HASREG(arg->ml_type))
				bitset_set(not_these_bitset, arg->ml_value.v_hreg.r_regno);
		}
		for (i = 0, regno = 0; regno < HOST_REGISTER_COUNT; ++regno) {
			if (bitset_test(not_these_bitset, regno)) {
				not_these[i] = regno;
				++i;
			}
		}
		not_these[i] = HOST_REGISTER_COUNT;
		result = Dee_function_generator_greg(self, api_function_loc, not_these);
		if unlikely(result != 0)
			break;
	}	ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		if unlikely(api_function_loc->ml_value.v_hreg.r_off != 0) {
			result = Dee_function_generator_gmov_regx2reg(self,
			                                              api_function_loc->ml_value.v_hreg.r_regno,
			                                              api_function_loc->ml_value.v_hreg.r_off,
			                                              api_function_loc->ml_value.v_hreg.r_regno);
			if unlikely(result != 0)
				break;
			Dee_memstate_hregs_adjust_delta(self->fg_state,
			                                api_function_loc->ml_value.v_hreg.r_regno,
			                                api_function_loc->ml_value.v_hreg.r_off);
			api_function_loc->ml_value.v_hreg.r_off = 0;
		}
		result = _Dee_function_generator_gcalldynapi_reg(self, api_function_loc->ml_value.v_hreg.r_regno, argc, argv);
		break;
	case MEMLOC_TYPE_HSTACKIND:
		if unlikely(api_function_loc->ml_value.v_hstack.s_off != 0)
			goto fallback;
		result = _Dee_function_generator_gcalldynapi_hstackind(self, api_function_loc->ml_value.v_hstack.s_cfa, argc, argv);
		break;
	case MEMLOC_TYPE_HREGIND:
		if unlikely(api_function_loc->ml_value.v_hreg.r_voff != 0)
			goto fallback;
		result = _Dee_function_generator_gcalldynapi_hregind(self,
		                                                     api_function_loc->ml_value.v_hreg.r_regno,
		                                                     api_function_loc->ml_value.v_hreg.r_off,
		                                                     argc, argv);
		break;
	case MEMLOC_TYPE_CONST:
		result = _Dee_function_generator_gcallapi(self, (void const *)api_function_loc->ml_value.v_const, argc, argv);
		break;
	}
	if likely(result == 0)
		Dee_function_generator_remember_undefined_allregs(self);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gincref_loc(struct Dee_function_generator *__restrict self,
                                   struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return Dee_function_generator_gincref_regx(self,
		                                           loc->ml_value.v_hreg.r_regno,
		                                           loc->ml_value.v_hreg.r_off,
		                                           n);
	case MEMLOC_TYPE_CONST:
		return Dee_function_generator_gincref_const(self, loc->ml_value.v_const, n);
	case MEMLOC_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdecref_loc(struct Dee_function_generator *__restrict self,
                                   struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return Dee_function_generator_gdecref_regx(self,
		                                           loc->ml_value.v_hreg.r_regno,
		                                           loc->ml_value.v_hreg.r_off,
		                                           n);
	case MEMLOC_TYPE_CONST:
		return Dee_function_generator_gdecref_const(self, loc->ml_value.v_const, n);
	case MEMLOC_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdecref_dokill_loc(struct Dee_function_generator *__restrict self,
                                          struct Dee_memloc *__restrict loc) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return Dee_function_generator_gdecref_regx_dokill(self,
		                                                  loc->ml_value.v_hreg.r_regno,
		                                                  loc->ml_value.v_hreg.r_off);
	case MEMLOC_TYPE_CONST:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "decref_dokill called on CONST location. "
		                       "Constants can never be destroyed");
	case MEMLOC_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdecref_nokill_loc(struct Dee_function_generator *__restrict self,
                                          struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return Dee_function_generator_gdecref_nokill_regx(self,
		                                                  loc->ml_value.v_hreg.r_regno,
		                                                  loc->ml_value.v_hreg.r_off,
		                                                  n);
	case MEMLOC_TYPE_CONST:
		return Dee_function_generator_gdecref_const(self, loc->ml_value.v_const, n);
	case MEMLOC_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxincref_loc(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return Dee_function_generator_gxincref_regx(self,
		                                            loc->ml_value.v_hreg.r_regno,
		                                            loc->ml_value.v_hreg.r_off,
		                                            n);
	case MEMLOC_TYPE_CONST:
		if (!loc->ml_value.v_const)
			return 0;
		return Dee_function_generator_gincref_const(self, loc->ml_value.v_const, n);
	case MEMLOC_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxdecref_loc(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return Dee_function_generator_gxdecref_regx(self,
		                                            loc->ml_value.v_hreg.r_regno,
		                                            loc->ml_value.v_hreg.r_off,
		                                            n);
	case MEMLOC_TYPE_CONST:
		if (!loc->ml_value.v_const)
			return 0;
		return Dee_function_generator_gdecref_const(self, loc->ml_value.v_const, n);
	case MEMLOC_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxdecref_nokill_loc(struct Dee_function_generator *__restrict self,
                                           struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return Dee_function_generator_gxdecref_nokill_regx(self,
		                                                   loc->ml_value.v_hreg.r_regno,
		                                                   loc->ml_value.v_hreg.r_off,
		                                                   n);
	case MEMLOC_TYPE_CONST:
		if (!loc->ml_value.v_const)
			return 0;
		return Dee_function_generator_gdecref_const(self, loc->ml_value.v_const, n);
	case MEMLOC_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

/* Change `loc' into the value of `<loc> = *(<loc> + ind_delta)'
 * Note that unlike the `Dee_function_generator_gmov*' functions, this
 * one may use `MEMLOC_TYPE_*IND' to defer the indirection until later. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gind(struct Dee_function_generator *__restrict self,
                            struct Dee_memloc *loc, ptrdiff_t ind_delta) {
	struct Dee_memequiv *eq;
	ASSERTF(loc->ml_flags & MEMLOC_F_NOREF, "Dee_function_generator_gind() called on reference");
	switch (loc->ml_type) {

	case MEMLOC_TYPE_HSTACK: {
		loc->ml_value.v_hstack.s_cfa = HA_cfa_offset_PLUS_sp_offset(loc->ml_value.v_hstack.s_cfa, ind_delta);
		eq = Dee_function_generator_remember_getclassof_hstackind(self, loc->ml_value.v_hstack.s_cfa);
		if (eq != NULL) {
			Dee_memequiv_next_asloc(eq, loc);
			return 0;
		}
		loc->ml_type = MEMLOC_TYPE_HSTACKIND;
		loc->ml_value.v_hstack.s_off = 0;
		return 0;
	}	break;

	case MEMLOC_TYPE_CONST: {
		struct Dee_memstate *state = self->fg_state;
		DeeObject **p_value;
		Dee_host_register_t temp_regno;
		temp_regno = Dee_function_generator_gallocreg(self, NULL);
		if unlikely(temp_regno >= HOST_REGISTER_COUNT)
			goto err;
		p_value = (DeeObject **)((uintptr_t)loc->ml_value.v_const + ind_delta);
		if unlikely(Dee_function_generator_gmov_constind2reg(self, p_value, temp_regno))
			goto err;
		loc->ml_type = MEMLOC_TYPE_HREG;
		loc->ml_value.v_hreg.r_regno = temp_regno;
		loc->ml_value.v_hreg.r_off   = 0;
		loc->ml_value.v_hreg.r_voff  = 0;
		if (Dee_memstate_isinstate(state, loc))
			Dee_memstate_incrinuse(self->fg_state, loc->ml_value.v_hreg.r_regno);
	}	break;

	case MEMLOC_TYPE_UNDEFINED:
		return 0;

	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ASSERT(loc->ml_type == MEMLOC_TYPE_HREG);
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		loc->ml_value.v_hreg.r_off += ind_delta;
		eq = Dee_function_generator_remember_getclassof_regind(self,
		                                                       loc->ml_value.v_hreg.r_regno,
		                                                       loc->ml_value.v_hreg.r_off);
		if (eq != NULL) {
			Dee_memequiv_next_asloc(eq, loc);
			return 0;
		}
		/* Turn the location from an HREG into HREGIND */
		loc->ml_type = MEMLOC_TYPE_HREGIND;
		loc->ml_value.v_hreg.r_voff = 0;
		break;
	}
	return 0;
err:
	return -1;
}

/* Force `loc' to become a register (`MEMLOC_TYPE_HREG'). */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_greg(struct Dee_function_generator *__restrict self,
                            struct Dee_memloc *loc,
                            Dee_host_register_t const *not_these) {
	struct Dee_memequiv *eq;
	struct Dee_memstate *state;
	Dee_host_register_t regno;
	ptrdiff_t val_delta;
	bool is_in_state;
	if (loc->ml_type == MEMLOC_TYPE_HREG)
		return 0; /* Already in a register! */

	/* Check if "loc" has a known register equivalence. */
	eq = Dee_function_generator_remember_getclassof(self, loc);
	if (eq != NULL) {
		struct Dee_memequiv *reg_eq = Dee_memequiv_next(eq);
		while (reg_eq->meq_loc.meql_type != MEMEQUIV_TYPE_HREG) {
			if (reg_eq == eq)
				goto no_equivalence;
			reg_eq = Dee_memequiv_next(reg_eq);
		}
		/* Example:
		 * >> eq(FOO + 3)  == reg_eq(BAR + 5)
		 * >> loc(FOO + 7) == reg_eq(BAR + 9)
		 * >> 9 = 7 - 3 + 5
		 * >> 9 == RESULT_VAL_DELTA
		 * >> 7 == Dee_memloc_getvaldelta_c0(loc)
		 * >> 3 == eq->meq_valoff
		 * >> 5 == reg_eq->meq_valoff   (gets added in `Dee_memequiv_asloc()') */
		val_delta = Dee_memloc_getvaldelta_c0(loc);
		val_delta -= eq->meq_valoff;
		Dee_memequiv_asloc(reg_eq, loc, val_delta);
		ASSERT(loc->ml_type == MEMLOC_TYPE_HREG);
		return 0;
	}
no_equivalence:

	/* Allocate a register. */
	regno = Dee_function_generator_gallocreg(self, not_these);
	if unlikely(regno >= HOST_REGISTER_COUNT)
		goto err;

	/* Move value into register. */
	if unlikely(Dee_function_generator_gmov_loc2regy(self, loc, regno, &val_delta))
		goto err;

	/* If the location used to be a writable location, then we must
	 * also update any other location that used to alias `loc'. */
	state = self->fg_state;
	if (loc->ml_type == MEMLOC_TYPE_HREGIND ||
	    loc->ml_type == MEMLOC_TYPE_HSTACKIND) {
		ptrdiff_t delta_change = val_delta - loc->ml_value.v_hreg.r_voff;
		struct Dee_memloc *alias;
		Dee_memstate_foreach(alias, state) {
			if (alias->ml_type != loc->ml_type)
				continue;
			if (alias->ml_value.v_hreg.r_off != loc->ml_value.v_hreg.r_off)
				continue;
			if (loc->ml_type == MEMLOC_TYPE_HREGIND &&
			    (alias->ml_value.v_hreg.r_regno != loc->ml_value.v_hreg.r_regno))
				continue;
			if (alias == loc)
				continue;
			if (MEMLOC_TYPE_HASREG(alias->ml_type))
				Dee_memstate_decrinuse(self->fg_state, alias->ml_value.v_hreg.r_regno);
			alias->ml_type = MEMLOC_TYPE_HREG;
			alias->ml_value.v_hreg.r_regno = regno;
			alias->ml_value.v_hreg.r_off   = alias->ml_value.v_hreg.r_voff + delta_change;
			Dee_memstate_incrinuse(self->fg_state, regno);
		}
		Dee_memstate_foreach_end;
	}

	/* Adjust register usage if `loc' is tracked by the memory state. */
	is_in_state = Dee_memstate_isinstate(state, loc);
	if (is_in_state && MEMLOC_TYPE_HASREG(loc->ml_type))
		Dee_memstate_decrinuse(self->fg_state, loc->ml_value.v_hreg.r_regno);

	/* Remember that `loc' now lies in a register. */
	loc->ml_type = MEMLOC_TYPE_HREG;
	loc->ml_value.v_hreg.r_regno = regno;
	loc->ml_value.v_hreg.r_off   = val_delta;
	if (is_in_state)
		Dee_memstate_incrinuse(self->fg_state, regno);
	return 0;
err:
	return -1;
}

/* Force `loc' to reside on the stack, giving it an address (`MEMLOC_TYPE_HSTACKIND, v_hstack.s_off = 0'). */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gflush(struct Dee_function_generator *__restrict self,
                              struct Dee_memloc *loc) {
	uintptr_t cfa_offset;
	struct Dee_memstate *state = self->fg_state;
	ASSERT(!Dee_memstate_isshared(state));
	if (loc->ml_type == MEMLOC_TYPE_HSTACKIND) {
handle_hstackind_loc:
		if (loc->ml_value.v_hstack.s_off == 0)
			return 0; /* Already on-stack at offset=0 */

#ifdef HAVE__Dee_host_section_gadd_const2hstackind
		/* emit `addP $..., sp_offset(%Psp)' to adjust the offset of the stored value
		 * Afterwards, go through all stack/local variables and adjust value offsets
		 * wherever the same CFA offset is referenced. */
		return _Dee_host_section_gadd_const2hstackind(self->fg_sect,
		                                              (DeeObject *)(uintptr_t)(intptr_t)loc->ml_value.v_hstack.s_off,
		                                              Dee_memstate_hstack_cfa2sp(self->fg_state, loc->ml_value.v_hstack.s_cfa));
#endif /* HAVE__Dee_host_section_gadd_const2hstackind */
	}

	/* Figure out where we want to allocate the value. */
#ifdef HAVE_try_restore_xloc_arg_cfa_offset
	if (loc->ml_type == MEMLOC_TYPE_HREG &&
		(cfa_offset = try_restore_xloc_arg_cfa_offset(self, loc->ml_value.v_hreg.r_regno)) != (uintptr_t)-1) {
		/* CFA offset restored */
	} else
#endif /* HAVE_try_restore_xloc_arg_cfa_offset */
	{
		/* Check if "loc" has a known HSTACKIND equivalence. */
		struct Dee_memequiv *eq;
		eq = Dee_function_generator_remember_getclassof(self, loc);
		if (eq != NULL) {
			ptrdiff_t val_delta = Dee_memloc_getvaldelta_c0(loc);
			struct Dee_memequiv *hstackind_eq_any = NULL;
			struct Dee_memequiv *hstackind_eq;
			val_delta -= eq->meq_valoff;
			for (hstackind_eq = Dee_memequiv_next(eq); hstackind_eq != eq;
			     hstackind_eq = Dee_memequiv_next(hstackind_eq)) {
				if (hstackind_eq->meq_loc.meql_type == MEMEQUIV_TYPE_HSTACKIND) {
					if ((hstackind_eq->meq_valoff + val_delta) == 0) {
						/* Perfect match: this equivalence allows for `v_hstack.s_off = 0' */
						Dee_memequiv_asloc(hstackind_eq, loc, val_delta);
						ASSERT(loc->ml_type == MEMLOC_TYPE_HSTACKIND);
						ASSERT(loc->ml_value.v_hstack.s_off == 0);
						return 0;
					}
					hstackind_eq_any = hstackind_eq;
				}
			}
			if (hstackind_eq_any) {
				Dee_memequiv_asloc(hstackind_eq_any, loc, val_delta);
				ASSERT(loc->ml_type == MEMLOC_TYPE_HSTACKIND);
				ASSERT(loc->ml_value.v_hstack.s_off != 0);
				goto handle_hstackind_loc;
			}
		}

		/* Search for a currently free stack location. */
		cfa_offset = Dee_memstate_hstack_find(state, self->fg_state_hstack_res, HOST_SIZEOF_POINTER);
		if (cfa_offset != (uintptr_t)-1) {
			if unlikely(Dee_function_generator_gmov_loc2hstackind(self, loc, cfa_offset))
				goto err;
		} else {
			if unlikely(Dee_function_generator_ghstack_pushloc(self, loc))
				goto err;
#ifdef HOSTASM_STACK_GROWS_DOWN
			cfa_offset = state->ms_host_cfa_offset;
#else /* HOSTASM_STACK_GROWS_DOWN */
			cfa_offset = state->ms_host_cfa_offset - HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		}
	}

	/* If the location used to be a writable location, then we must
	 * also update any other location that used to alias `loc'. */
	state = self->fg_state;
	if (loc->ml_type == MEMLOC_TYPE_HREG ||
	    loc->ml_type == MEMLOC_TYPE_HREGIND) {
		ptrdiff_t delta_change = -loc->ml_value.v_hreg.r_voff;
		struct Dee_memloc *alias;
		Dee_memstate_foreach(alias, state) {
			if (alias->ml_type != loc->ml_type)
				continue;
			if (alias->ml_value.v_hreg.r_regno != loc->ml_value.v_hreg.r_regno)
				continue;
			if (alias->ml_value.v_hreg.r_off != loc->ml_value.v_hreg.r_off)
				continue;
			if (alias == loc)
				continue;
			ASSERT(MEMLOC_TYPE_HASREG(alias->ml_type));
			Dee_memstate_decrinuse(self->fg_state, alias->ml_value.v_hreg.r_regno);
			alias->ml_type = MEMLOC_TYPE_HSTACKIND;
			alias->ml_value.v_hstack.s_off = alias->ml_value.v_hreg.r_voff + delta_change;
			alias->ml_value.v_hstack.s_cfa = cfa_offset;
		}
		Dee_memstate_foreach_end;
	}

	/* Adjust register usage if `loc' is tracked by the memory state. */
	if (MEMLOC_TYPE_HASREG(loc->ml_type) && Dee_memstate_isinstate(state, loc))
		Dee_memstate_decrinuse(self->fg_state, loc->ml_value.v_hreg.r_regno);

	/* Remember that `loc' now lies on-stack (with an offset of `0') */
	loc->ml_type = MEMLOC_TYPE_HSTACKIND;
	loc->ml_value.v_hstack.s_cfa = cfa_offset;
	loc->ml_value.v_hstack.s_off = 0;
	return 0;
err:
	return -1;
}


/* Check if `src_loc' differs from `dst_loc', and if so: move `src_loc' *into* `dst_loc'. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gmov_loc2loc(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc *src_loc,
                                    struct Dee_memloc const *dst_loc) {
	int result;
	if (Dee_memloc_sameloc(src_loc, dst_loc))
		return 0;
	switch (dst_loc->ml_type) {
	case MEMLOC_TYPE_HREG:
		result = Dee_function_generator_gmov_loc2regx(self, src_loc,
		                                              dst_loc->ml_value.v_hreg.r_regno,
		                                              dst_loc->ml_value.v_hreg.r_off);
		break;
	case MEMLOC_TYPE_HSTACKIND:
		result = Dee_function_generator_gmov_loc2hstackindx(self, src_loc,
		                                                    dst_loc->ml_value.v_hstack.s_cfa,
		                                                    dst_loc->ml_value.v_hstack.s_off);
		break;
	case MEMLOC_TYPE_UNDEFINED:
		return 0;
	default:
		switch (src_loc->ml_type) {
		default: {
			Dee_host_register_t not_these[2];
			not_these[0] = HOST_REGISTER_COUNT;
			not_these[1] = HOST_REGISTER_COUNT;
			if (MEMLOC_TYPE_HASREG(dst_loc->ml_type))
				not_these[0] = dst_loc->ml_value.v_hreg.r_regno;
			if unlikely(Dee_function_generator_greg(self, src_loc, not_these))
				goto err;
			ASSERT(src_loc->ml_type == MEMLOC_TYPE_HREG);
		}	ATTR_FALLTHROUGH
		case MEMLOC_TYPE_HREG:
			result = Dee_function_generator_gmov_regx2loc(self,
			                                              src_loc->ml_value.v_hreg.r_regno,
			                                              src_loc->ml_value.v_hreg.r_off,
			                                              dst_loc);
			break;
		case MEMLOC_TYPE_CONST:
			result = Dee_function_generator_gmov_const2loc(self, src_loc->ml_value.v_const, dst_loc);
			break;
		case MEMLOC_TYPE_HSTACK:
			result = Dee_function_generator_gmov_hstack2loc(self, src_loc->ml_value.v_hstack.s_cfa, dst_loc);
			break;
		case MEMLOC_TYPE_UNDEFINED:
			return 0;
		}
		break;
	}
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gmov_loc2locind(struct Dee_function_generator *__restrict self,
                                       struct Dee_memloc *src_loc,
                                       struct Dee_memloc const *dst_loc, ptrdiff_t ind_delta) {
	int result;
	switch (src_loc->ml_type) {
	default: {
		Dee_host_register_t not_these[2];
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (MEMLOC_TYPE_HASREG(dst_loc->ml_type))
			not_these[0] = dst_loc->ml_value.v_hreg.r_regno;
		if unlikely(Dee_function_generator_greg(self, src_loc, not_these))
			goto err;
		ASSERT(src_loc->ml_type == MEMLOC_TYPE_HREG);
	}	ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG: {
		if (src_loc->ml_value.v_hreg.r_off != 0) {
			if (Dee_function_generator_gmov_regx2reg(self,
			                                         src_loc->ml_value.v_hreg.r_regno,
			                                         src_loc->ml_value.v_hreg.r_off,
			                                         src_loc->ml_value.v_hreg.r_regno))
				goto err;
			Dee_memstate_hregs_adjust_delta(self->fg_state,
			                                src_loc->ml_value.v_hreg.r_regno,
			                                src_loc->ml_value.v_hreg.r_off);
			src_loc->ml_value.v_hreg.r_off = 0;
		}
		result = Dee_function_generator_gmov_reg2locind(self,
		                                                src_loc->ml_value.v_hreg.r_regno,
		                                                dst_loc, ind_delta);
	}	break;

	case MEMLOC_TYPE_UNDEFINED:
		return 0;

	case MEMLOC_TYPE_CONST:
		result = Dee_function_generator_gmov_const2locind(self,
		                                                  src_loc->ml_value.v_const,
		                                                  dst_loc, ind_delta);
		break;
	}
	return result;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_grwlock_read_const(struct Dee_function_generator *__restrict self,
                                          Dee_atomic_rwlock_t *__restrict lock) {
	struct Dee_memloc loc;
	loc.ml_type = MEMLOC_TYPE_CONST;
	loc.ml_value.v_const = (DeeObject *)lock;
	return Dee_function_generator_grwlock_read(self, &loc);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_grwlock_write_const(struct Dee_function_generator *__restrict self,
                                           Dee_atomic_rwlock_t *__restrict lock) {
	struct Dee_memloc loc;
	loc.ml_type = MEMLOC_TYPE_CONST;
	loc.ml_value.v_const = (DeeObject *)lock;
	return Dee_function_generator_grwlock_write(self, &loc);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_grwlock_endread_const(struct Dee_function_generator *__restrict self,
                                             Dee_atomic_rwlock_t *__restrict lock) {
	struct Dee_memloc loc;
	loc.ml_type = MEMLOC_TYPE_CONST;
	loc.ml_value.v_const = (DeeObject *)lock;
	return Dee_function_generator_grwlock_endread(self, &loc);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_grwlock_endwrite_const(struct Dee_function_generator *__restrict self,
                                              Dee_atomic_rwlock_t *__restrict lock) {
	struct Dee_memloc loc;
	loc.ml_type = MEMLOC_TYPE_CONST;
	loc.ml_value.v_const = (DeeObject *)lock;
	return Dee_function_generator_grwlock_endwrite(self, &loc);
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_COMMON_C */
