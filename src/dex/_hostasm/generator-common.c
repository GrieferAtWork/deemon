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

/************************************************************************/
/* COMMON CODE GENERATION FUNCTIONS                                     */
/************************************************************************/

#define DeeInt_NEWSFUNC(n) DEE_PRIVATE_NEWINT(n)
#define DeeInt_NEWUFUNC(n) DEE_PRIVATE_NEWUINT(n)

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdirect_impl(struct Dee_function_generator *__restrict self,
                                    struct Dee_memval *__restrict mval) {
	uint8_t vmorph = mval->mv_vmorph;
	ASSERT(!MEMVAL_VMORPH_ISDIRECT(vmorph));
	switch (vmorph) {

	case MEMVAL_VMORPH_NULLABLE: {
		int result;
		uint8_t saved_flags = mval->mv_obj0.mo_flags;
		Dee_memobj_clearref(&mval->mv_obj0);
		result = Dee_function_generator_gjz_except(self, &mval->mv_obj0.mo_loc);
		mval->mv_obj0.mo_flags  = saved_flags;
		mval->mv_vmorph = MEMVAL_VMORPH_DIRECT;
		return result;
	}	break;

	case MEMVAL_VMORPH_BOOL_Z:
	case MEMVAL_VMORPH_BOOL_Z_01:
	case MEMVAL_VMORPH_BOOL_NZ:
	case MEMVAL_VMORPH_BOOL_NZ_01:
	case MEMVAL_VMORPH_BOOL_LZ:
	case MEMVAL_VMORPH_BOOL_GZ: {
		int temp;
		Dee_host_register_t retreg;
		ptrdiff_t retreg_delta;
		if (Dee_memobj_gettyp(&mval->mv_obj0) == MEMADR_TYPE_HREG) {
			retreg = Dee_memobj_hreg_getreg(&mval->mv_obj0);
		} else {
			Dee_host_register_t not_these[2];
			not_these[0] = HOST_REGISTER_COUNT;
			not_these[1] = HOST_REGISTER_COUNT;
			if (Dee_memobj_hasreg(&mval->mv_obj0))
				not_these[0] = Dee_memobj_getreg(&mval->mv_obj0);
			retreg = Dee_function_generator_gallocreg(self, not_these);
			if unlikely(retreg >= HOST_REGISTER_COUNT)
				goto err;
		}
		if (vmorph == MEMVAL_VMORPH_BOOL_NZ_01) {
			temp = Dee_function_generator_gmorph_loc012regbooly(self,
			                                                    Dee_memobj_getloc(&mval->mv_obj0),
			                                                    0, retreg, &retreg_delta);
		} else {
			unsigned int cmp;
			switch (vmorph) {
			case MEMVAL_VMORPH_BOOL_Z:
			case MEMVAL_VMORPH_BOOL_Z_01:
				cmp = GMORPHBOOL_CC_EQ;
				break;
			case MEMVAL_VMORPH_BOOL_NZ:
				cmp = GMORPHBOOL_CC_NE;
				break;
			case MEMVAL_VMORPH_BOOL_LZ:
				cmp = GMORPHBOOL_CC_LO;
				break;
			case MEMVAL_VMORPH_BOOL_GZ:
				cmp = GMORPHBOOL_CC_GR;
				break;
			default: __builtin_unreachable();
			}
			temp = Dee_function_generator_gmorph_loc2regbooly(self,
			                                                  Dee_memobj_getloc(&mval->mv_obj0),
			                                                  0, cmp, retreg, &retreg_delta);
		}
		if unlikely(temp)
			goto err;
		if (Dee_memstate_ismemvalinstate(self->fg_state, mval)) {
			Dee_memstate_decrinuse_for_memobj(self->fg_state, &mval->mv_obj0);
			Dee_memstate_incrinuse(self->fg_state, retreg);
		}
		Dee_memval_init_hreg(mval, retreg, retreg_delta, &DeeBool_Type, MEMOBJ_F_NORMAL);
		return 0;
	}	break;

	case MEMVAL_VMORPH_INT:
	case MEMVAL_VMORPH_UINT: {
		/* Construct a new deemon integer object. */
		if unlikely(Dee_function_generator_gcallapi(self,
		                                            vmorph == MEMVAL_VMORPH_INT
		                                            ? (void const *)&DeeInt_NEWSFUNC(HOST_SIZEOF_POINTER)
		                                            : (void const *)&DeeInt_NEWUFUNC(HOST_SIZEOF_POINTER),
		                                            1, Dee_memobj_getloc(&mval->mv_obj0)))
			goto err;
		mval->mv_obj0.mo_flags = MEMOBJ_F_NORMAL;
		mval->mv_vmorph = MEMVAL_VMORPH_DIRECT;
		if (Dee_memstate_ismemvalinstate(self->fg_state, mval)) {
			Dee_memstate_decrinuse_for_memobj(self->fg_state, &mval->mv_obj0);
			Dee_memstate_incrinuse(self->fg_state, HOST_REGISTER_RETURN);
		}
		Dee_memval_init_hreg(mval, HOST_REGISTER_RETURN, 0, NULL, MEMOBJ_F_NORMAL);
		if unlikely(Dee_function_generator_gjz_except(self, Dee_memobj_getloc(&mval->mv_obj0)))
			goto err;
		ASSERT(!Dee_memobj_isref(&mval->mv_obj0));
		ASSERT(Dee_memobj_typeof(&mval->mv_obj0) == NULL);
		Dee_memobj_setref(&mval->mv_obj0);
		Dee_memobj_settypeof(&mval->mv_obj0, &DeeInt_Type);
	}	break;

	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Unsupported location value type %#" PRFx8,
		                       mval->mv_vmorph);
	}
	return 0;
err:
	return -1;
}

/* Force `mval' to use `MEMVAL_VMORPH_ISDIRECT'.
 * NOTE: This is the only `Dee_function_generator_g*' function that
 *       doesn't simply assume `MEMVAL_VMORPH_ISDIRECT(mv_vmorph)'. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdirect(struct Dee_function_generator *__restrict self,
                               struct Dee_memval *mval) {
	struct Dee_memval *alias, oldval;
	struct Dee_memstate *state;
	if (Dee_memval_isdirect(mval))
		return 0; /* Already a direct value. */

	/* Force the value to become direct. */
	Dee_memval_initcopy(&oldval, mval);
	if unlikely(Dee_function_generator_gdirect_impl(self, mval))
		goto err_oldval;

	/* Write the updated value into all aliases. */
	ASSERT(Dee_memval_isdirect(mval));
	/*oldval.mv_obj0.mo_flags &= ~MEMVAL_M_LOCAL_BSTATE;*/ /* Not needed */
	state = self->fg_state;
	Dee_memstate_foreach(alias, state) {
		if (!Dee_memval_sameval(alias, &oldval))
			continue;
		if (alias == mval)
			continue;
		Dee_memstate_decrinuse_for_memval(state, alias);
		Dee_memval_fini(alias);
		Dee_memval_direct_initcopy(alias, mval);
		Dee_memstate_incrinuse_for_direct_memval(state, mval);
	}
	Dee_memstate_foreach_end;
	Dee_memval_fini(&oldval);
	return 0;
err_oldval:
	Dee_memval_fini(&oldval);
/*err:*/
	return -1;
}


/* Clear the `MEMOBJ_F_ONEREF' flag from `mobj', as well
 * as any other memory location that might be aliasing it. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gnotoneref_impl(struct Dee_function_generator *__restrict self,
                                       struct Dee_memobj *mobj) {
	struct Dee_memval *alias_mval;
	ASSERT(mobj->mo_flags & MEMOBJ_F_ONEREF);
	Dee_memstate_foreach(alias_mval, self->fg_state) {
		struct Dee_memobj *alias_mobj;
		Dee_memval_foreach_obj(alias_mobj, alias_mval) {
			if (Dee_memobj_sameloc(alias_mobj, mobj))
				Dee_memobj_clearoneref(alias_mobj);
		}
	}
	Dee_memstate_foreach_end;
	Dee_memobj_clearoneref(mobj);
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
                                         void const *value) {
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
		 * If there is a memory location with the MEMOBJ_F_LINEAR flag
		 * set, then we MUST NOT use pop (since the location can't be
		 * altered) */
		struct Dee_memval *val;
		struct Dee_memobj *loc, *cfa_locs = NULL;
		Dee_memstate_foreach(val, self->fg_state) {
			Dee_memval_foreach_obj(loc, val) {
				if (Dee_memobj_gettyp(loc) == MEMADR_TYPE_HSTACKIND &&
				    Dee_memobj_hstackind_getcfa(loc) == cfa_offset) {
					STATIC_ASSERT(offsetof(struct Dee_memobj, mo_loc.ml_adr.ma_val.v_cfa) ==
					              offsetof(struct Dee_memobj, mo_loc.ml_adr.ma_val._v_nextobj));
					if (val->mv_obj0.mo_flags & MEMOBJ_F_LINEAR) {
						/* Ooops: not allowed. (Location must not be moved to a register) */
						while (cfa_locs) {
							loc = cfa_locs->mo_loc.ml_adr.ma_val._v_nextobj;
							cfa_locs->mo_loc.ml_adr.ma_val.v_cfa = cfa_offset;
							cfa_locs = loc;
						}
						goto do_use_mov;
					}
					loc->mo_loc.ml_adr.ma_val._v_nextobj = cfa_locs;
					cfa_locs = loc;
				}
			}
		}
		Dee_memstate_foreach_end;
		if (cfa_locs != NULL) {
			struct Dee_memobj *next;
			do {
				next = cfa_locs->mo_loc.ml_adr.ma_val._v_nextobj;
				Dee_memloc_init_hreg(&cfa_locs->mo_loc, dst_regno, Dee_memloc_getoff(&cfa_locs->mo_loc));
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
                                      void const *value, Dee_host_register_t dst_regno) {
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
                                         void const *value, Dee_host_register_t dst_regno,
                                         ptrdiff_t dst_delta) {
	int result = _Dee_function_generator_gmov_const2regind(self, value, dst_regno, dst_delta);
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_const2regind(self, value, dst_regno, dst_delta);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_const2hstackind(struct Dee_function_generator *__restrict self,
                                            void const *value, uintptr_t cfa_offset) {
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
                                           void const *value, void const **p_value) {
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
                                         void const **p_value, Dee_host_register_t dst_regno) {
	int result = _Dee_function_generator_gmov_constind2reg(self, p_value, dst_regno);
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = DEE_HOST_REGUSAGE_GENERIC;
		result = Dee_function_generator_remember_movevalue_constind2reg(self, p_value, dst_regno);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_reg2constind(struct Dee_function_generator *__restrict self,
                                         Dee_host_register_t src_regno, void const **p_value) {
	int result = _Dee_function_generator_gmov_reg2constind(self, src_regno, p_value);
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_reg2constind(self, src_regno, p_value);
	return result;
}







INTERN WUNUSED NONNULL((1, 3)) int DCALL
Dee_function_generator_gmov_const2loc(struct Dee_function_generator *__restrict self,
                                      void const *value, struct Dee_memloc const *__restrict dst_loc) {
	/* TODO: If "dst_loc" is a known equivalence of "value", do nothing */
	switch (Dee_memloc_gettyp(dst_loc)) {

	case MEMADR_TYPE_HSTACKIND: {
		uintptr_t cfa_offset = Dee_memloc_hstackind_getcfa(dst_loc);
		void const *final_value = (byte_t const *)value - Dee_memloc_getoff(dst_loc);
		return Dee_function_generator_gmov_const2hstackind(self, final_value, cfa_offset);
	}	break;

	case MEMADR_TYPE_HREGIND: {
		void const *final_value = (byte_t const *)value - Dee_memloc_getoff(dst_loc);
		return Dee_function_generator_gmov_const2regind(self, final_value,
		                                                Dee_memloc_hregind_getreg(dst_loc),
		                                                Dee_memloc_hregind_getindoff(dst_loc));
	}	break;

	case MEMADR_TYPE_HREG: {
		void const *final_value = (byte_t const *)value - Dee_memloc_getoff(dst_loc);
		return Dee_function_generator_gmov_const2reg(self, final_value, Dee_memloc_hreg_getreg(dst_loc));
	}	break;

	default: {
		Dee_host_register_t temp_regno, not_these[2];
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (Dee_memloc_hasreg(dst_loc))
			not_these[0] = Dee_memloc_getreg(dst_loc);
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
	switch (Dee_memloc_gettyp(dst_loc)) {

	case MEMADR_TYPE_HREG: {
		cfa_offset = HA_cfa_offset_PLUS_sp_offset(cfa_offset, -Dee_memloc_hreg_getvaloff(dst_loc));
		return Dee_function_generator_gmov_hstack2reg(self, cfa_offset, Dee_memloc_hreg_getreg(dst_loc));
	}	break;

	default: {
		Dee_host_register_t temp_regno, not_these[2];
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (Dee_memloc_hasreg(dst_loc))
			not_these[0] = Dee_memloc_getreg(dst_loc);
		temp_regno = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(temp_regno >= HOST_REGISTER_COUNT)
			goto err;
		cfa_offset = HA_cfa_offset_PLUS_sp_offset(cfa_offset, -Dee_memloc_getoff(dst_loc));
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
	switch (Dee_memloc_gettyp(dst_loc)) {

	case MEMADR_TYPE_HSTACKIND: {
		uintptr_t cfa_offset = Dee_memloc_hstackind_getcfa(dst_loc);
		ptrdiff_t delta_delta = Dee_memloc_hstackind_getvaloff(dst_loc) - src_delta;
		if (delta_delta != 0) {
			/* Adjust `src_regno' to have the correct value-delta */
			if unlikely(Dee_function_generator_gmov_regx2reg(self, src_regno, delta_delta, src_regno))
				goto err;
		}
		return Dee_function_generator_gmov_reg2hstackind(self, src_regno, cfa_offset);
	}	break;

	case MEMADR_TYPE_HREGIND: {
		ptrdiff_t delta_delta = Dee_memloc_hregind_getvaloff(dst_loc) - src_delta;
		if (delta_delta != 0) {
			if unlikely(Dee_function_generator_gmov_regx2reg(self, src_regno, delta_delta, src_regno))
				goto err;
		}
		return Dee_function_generator_gmov_reg2regind(self, src_regno,
		                                              Dee_memloc_hregind_getreg(dst_loc),
		                                              Dee_memloc_hregind_getindoff(dst_loc));
	}	break;

	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gmov_regx2reg(self, src_regno,
		                                            src_delta - Dee_memloc_hreg_getvaloff(dst_loc),
		                                            Dee_memloc_hreg_getreg(dst_loc));

	default: break;
	}
	return DeeError_Throwf(&DeeError_IllegalInstruction,
	                       "Cannot move register to location type %" PRFu16,
	                       Dee_memloc_gettyp(dst_loc));
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gmov_loc2regx(struct Dee_function_generator *__restrict self,
                                     struct Dee_memloc const *__restrict src_loc,
                                     Dee_host_register_t dst_regno, ptrdiff_t dst_delta) {
	int result;
	/* TODO: Go through equivalences of "src_loc" and pick the best one for the move */
	switch (Dee_memloc_gettyp(src_loc)) {

	case MEMADR_TYPE_HSTACKIND: {
		uintptr_t cfa_offset = Dee_memloc_hstackind_getcfa(src_loc);
		ptrdiff_t delta_delta = Dee_memloc_hstackind_getvaloff(src_loc) - dst_delta;
		result = Dee_function_generator_gmov_hstackind2reg(self, cfa_offset, dst_regno);
		if likely(result == 0 && delta_delta != 0)
			result = Dee_function_generator_gmov_regx2reg(self, dst_regno, delta_delta, dst_regno);
	}	break;

	case MEMADR_TYPE_HSTACK: {
		uintptr_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(Dee_memloc_hstack_getcfa(src_loc), -dst_delta);
		result = Dee_function_generator_gmov_hstack2reg(self, cfa_offset, dst_regno);
	}	break;

	case MEMADR_TYPE_HREGIND: {
		ptrdiff_t delta_delta = Dee_memloc_hregind_getvaloff(src_loc) - dst_delta;
		result = Dee_function_generator_gmov_regind2reg(self,
		                                                Dee_memloc_hregind_getreg(src_loc),
		                                                Dee_memloc_hregind_getindoff(src_loc),
		                                                dst_regno);
		if likely(result == 0 && delta_delta != 0)
			result = Dee_function_generator_gmov_regx2reg(self, dst_regno, delta_delta, dst_regno);
	}	break;

	case MEMADR_TYPE_HREG:
		result = Dee_function_generator_gmov_regx2reg(self,
		                                              Dee_memloc_hreg_getreg(src_loc),
		                                              Dee_memloc_hreg_getvaloff(src_loc) - dst_delta,
		                                              dst_regno);
		break;

	case MEMADR_TYPE_CONST:
		result = Dee_function_generator_gmov_const2reg(self,
		                                               Dee_memloc_const_getaddr(src_loc) - dst_delta,
		                                               dst_regno);
		break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot move location type %" PRFu16 " to register",
		                       Dee_memloc_gettyp(src_loc));
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
	switch (Dee_memloc_gettyp(src_loc)) {

	case MEMADR_TYPE_HSTACKIND: {
		uintptr_t cfa_offset = Dee_memloc_hstackind_getcfa(src_loc);
		result = Dee_function_generator_gmov_hstackind2reg(self, cfa_offset, dst_regno);
		*p_dst_delta = Dee_memloc_hstackind_getvaloff(src_loc);
	}	break;

	case MEMADR_TYPE_HSTACK: {
		uintptr_t cfa_offset  = Dee_memloc_hstack_getcfa(src_loc);
		ptrdiff_t sp_offset   = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		uintptr_t cfa0_offset = Dee_memstate_hstack_sp2cfa(self->fg_state, 0);
		result = Dee_function_generator_gmov_hstack2reg(self, cfa0_offset, dst_regno);
		*p_dst_delta = sp_offset;
	}	break;

	case MEMADR_TYPE_HREGIND:
		result = Dee_function_generator_gmov_regind2reg(self,
		                                                Dee_memloc_hregind_getreg(src_loc),
		                                                Dee_memloc_hregind_getindoff(src_loc),
		                                                dst_regno);
		*p_dst_delta = Dee_memloc_hregind_getvaloff(src_loc);
		break;

	case MEMADR_TYPE_HREG:
		result = Dee_function_generator_gmov_reg2reg(self, Dee_memloc_hreg_getreg(src_loc), dst_regno);
		*p_dst_delta = Dee_memloc_hreg_getvaloff(src_loc);
		break;

	case MEMADR_TYPE_CONST:
		result = Dee_function_generator_gmov_const2reg(self, Dee_memloc_const_getaddr(src_loc), dst_regno);
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
	switch (Dee_memloc_gettyp(src_loc)) {

	case MEMADR_TYPE_HREG:
		result = Dee_function_generator_gmov_regind2reg(self,
		                                                Dee_memloc_hreg_getreg(src_loc),
		                                                Dee_memloc_hreg_getvaloff(src_loc) + src_delta,
		                                                dst_regno);
		break;

	case MEMADR_TYPE_HSTACK: {
		uintptr_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(Dee_memloc_hstack_getcfa(src_loc), src_delta);
		result = Dee_function_generator_gmov_hstackind2reg(self, cfa_offset, dst_regno);
	}	break;

	case MEMADR_TYPE_CONST: {
		void const *value = Dee_memloc_const_getaddr(src_loc) + src_delta;
		result = Dee_function_generator_gmov_constind2reg(self, (void const **)value, dst_regno);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
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
	switch (Dee_memloc_gettyp(dst_loc)) {

	case MEMADR_TYPE_HREG:
		result = Dee_function_generator_gmov_reg2regind(self, src_regno,
		                                                Dee_memloc_hreg_getreg(dst_loc),
		                                                Dee_memloc_hreg_getvaloff(dst_loc) + dst_delta);
		break;

	case MEMADR_TYPE_HSTACK: {
		uintptr_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(Dee_memloc_hstack_getcfa(dst_loc), dst_delta);
		result = Dee_function_generator_gmov_reg2hstackind(self, src_regno, cfa_offset);
	}	break;

	case MEMADR_TYPE_CONST: {
		void const *value = Dee_memloc_const_getaddr(dst_loc) + dst_delta;
		result = Dee_function_generator_gmov_reg2constind(self, src_regno, (void const **)value);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	default: {
		Dee_host_register_t temp_regno;
		Dee_host_register_t not_these[2];
		ptrdiff_t ind_delta;
		struct Dee_memequiv *eq;

		/* See if "dst_loc" has a register/const equivalence. */
		eq = Dee_function_generator_remember_getclassof(self, Dee_memloc_getadr(dst_loc));
		if (eq != NULL) {
			struct Dee_memequiv *iter = Dee_memequiv_next(eq);
			do {
				if (Dee_memloc_gettyp(&iter->meq_loc) == MEMEQUIV_TYPE_HREG ||
				    Dee_memloc_gettyp(&iter->meq_loc) == MEMEQUIV_TYPE_CONST) {
					struct Dee_memloc reg_loc;
					ptrdiff_t val_delta = Dee_memloc_getoff(dst_loc);
					val_delta -= Dee_memloc_getoff(&eq->meq_loc);
					reg_loc = iter->meq_loc;
					ASSERT(Dee_memloc_gettyp(&reg_loc) == MEMADR_TYPE_HREG ||
					       Dee_memloc_gettyp(&reg_loc) == MEMADR_TYPE_CONST);
					Dee_memloc_adjoff(&reg_loc, val_delta);
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
	switch (Dee_memloc_gettyp(src_loc)) {

	case MEMADR_TYPE_HREG: {
		ptrdiff_t final_delta = Dee_memloc_hreg_getvaloff(src_loc) - dst_delta;
		if (final_delta != 0) {
			ptrdiff_t writeback_delta = Dee_memloc_hreg_getvaloff(src_loc) + final_delta;
			if unlikely(Dee_function_generator_gmov_regx2reg(self,
			                                                 Dee_memloc_hreg_getreg(src_loc),
			                                                 final_delta,
			                                                 Dee_memloc_hreg_getreg(src_loc)))
				goto err;
			Dee_memstate_hregs_adjust_delta(self->fg_state, Dee_memloc_hreg_getreg(src_loc), final_delta);
			Dee_memloc_hreg_setvaloff(src_loc, writeback_delta);
		}
		return Dee_function_generator_ghstack_pushreg(self, Dee_memloc_hreg_getreg(src_loc));
	}	break;

	case MEMADR_TYPE_HREGIND: {
		ptrdiff_t final_delta = Dee_memloc_hregind_getvaloff(src_loc) - dst_delta;
		if (final_delta != 0)
			goto fallback;
		return Dee_function_generator_ghstack_pushregind(self,
		                                                 Dee_memloc_hregind_getreg(src_loc),
		                                                 Dee_memloc_hregind_getindoff(src_loc));
	}	break;

	case MEMADR_TYPE_HSTACKIND: {
		uintptr_t cfa_offset = Dee_memloc_hstackind_getcfa(src_loc);
		ptrdiff_t final_delta = Dee_memloc_hstackind_getvaloff(src_loc) - dst_delta;
		if (final_delta != 0)
			goto fallback;
		return Dee_function_generator_ghstack_pushhstackind(self, cfa_offset);
	}	break;

#ifdef Dee_function_generator_ghstack_pushhstack_at_cfa_boundary
	case MEMADR_TYPE_HSTACK: {
		uintptr_t cfa_offset = Dee_memloc_hstack_getcfa(src_loc);
		if (cfa_offset == self->fg_state->ms_host_cfa_offset) /* Special case: push current CFA offset */
			return Dee_function_generator_ghstack_pushhstack_at_cfa_boundary(self);
		goto fallback;
	}	break;
#endif /* Dee_function_generator_ghstack_pushhstack_at_cfa_boundary */

	case MEMADR_TYPE_CONST: {
		void const *value = Dee_memloc_const_getaddr(src_loc) - dst_delta;
		return Dee_function_generator_ghstack_pushconst(self, value);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return Dee_function_generator_ghstack_adjust(self, HOST_SIZEOF_POINTER);

	default: {
		Dee_host_register_t temp_regno;
		Dee_host_register_t not_these[2];
fallback:
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (Dee_memloc_hasreg(src_loc))
			not_these[0] = Dee_memloc_getreg(src_loc);
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
	switch (Dee_memloc_gettyp(src_loc)) {

	case MEMADR_TYPE_HREG: {
		ptrdiff_t final_delta = Dee_memloc_hreg_getvaloff(src_loc) - dst_delta;
		if (final_delta != 0) {
			ptrdiff_t writeback_delta = Dee_memloc_hreg_getvaloff(src_loc) + final_delta;
			if unlikely(Dee_function_generator_gmov_regx2reg(self,
			                                                 Dee_memloc_hreg_getreg(src_loc),
			                                                 final_delta,
			                                                 Dee_memloc_hreg_getreg(src_loc)))
				goto err;
			Dee_memstate_hregs_adjust_delta(self->fg_state, Dee_memloc_hreg_getreg(src_loc), final_delta);
			Dee_memloc_hreg_setvaloff(src_loc, writeback_delta);
		}
		return Dee_function_generator_gmov_reg2hstackind(self, Dee_memloc_hreg_getreg(src_loc), dst_cfa_offset);
	}	break;

	case MEMADR_TYPE_CONST: {
		void const *value = Dee_memloc_const_getaddr(src_loc) - dst_delta;
		return Dee_function_generator_gmov_const2hstackind(self, value, dst_cfa_offset);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	default: {
		Dee_host_register_t temp_regno;
		Dee_host_register_t not_these[2];
/*fallback:*/
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (Dee_memloc_hasreg(src_loc))
			not_these[0] = Dee_memloc_getreg(src_loc);
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
                                         void const **p_value, ptrdiff_t dst_delta) {
	switch (Dee_memloc_gettyp(src_loc)) {

	case MEMADR_TYPE_HREG: {
		/*     *p_value + dst_delta = Dee_memloc_hreg_getreg(src_loc) + Dee_memloc_hreg_getvaloff(src_loc)
		 * <=> *p_value = Dee_memloc_hreg_getreg(src_loc) + Dee_memloc_hreg_getvaloff(src_loc) - dst_delta */
		ptrdiff_t final_delta = Dee_memloc_hreg_getvaloff(src_loc) - dst_delta;
		if (final_delta != 0) {
			ptrdiff_t writeback_delta = Dee_memloc_hreg_getvaloff(src_loc) + final_delta;
			if unlikely(Dee_function_generator_gmov_regx2reg(self,
			                                                 Dee_memloc_hreg_getreg(src_loc),
			                                                 final_delta,
			                                                 Dee_memloc_hreg_getreg(src_loc)))
				goto err;
			Dee_memstate_hregs_adjust_delta(self->fg_state, Dee_memloc_hreg_getreg(src_loc), final_delta);
			Dee_memloc_hreg_setvaloff(src_loc, writeback_delta);
		}
		return Dee_function_generator_gmov_reg2constind(self, Dee_memloc_hreg_getreg(src_loc), p_value);
	}	break;

	case MEMADR_TYPE_CONST: {
		void const *value = Dee_memloc_const_getaddr(src_loc) - dst_delta;
		return Dee_function_generator_gmov_const2constind(self, value, p_value);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	default: {
		Dee_host_register_t temp_regno;
		Dee_host_register_t not_these[2];
/*fallback:*/
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (Dee_memloc_hasreg(src_loc))
			not_these[0] = Dee_memloc_getreg(src_loc);
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
Dee_function_generator_gmov_const2locind(struct Dee_function_generator *__restrict self, void const *value,
                                         struct Dee_memloc const *__restrict dst_loc, ptrdiff_t dst_delta) {
	int result;
	switch (Dee_memloc_gettyp(dst_loc)) {

	case MEMADR_TYPE_HREG:
		result = Dee_function_generator_gmov_const2regind(self, value,
		                                                  Dee_memloc_hreg_getreg(dst_loc),
		                                                  Dee_memloc_hreg_getvaloff(dst_loc) + dst_delta);
		break;

	case MEMADR_TYPE_HSTACK: {
		uintptr_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(Dee_memloc_hstack_getcfa(dst_loc), dst_delta);
		result = Dee_function_generator_gmov_const2hstackind(self, value, cfa_offset);
	}	break;

	case MEMADR_TYPE_CONST: {
		void const *dst_value = Dee_memloc_const_getaddr(dst_loc) + dst_delta;
		result = Dee_function_generator_gmov_const2constind(self, value, (void const **)dst_value);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	default: {
		Dee_host_register_t temp_regno;
		ptrdiff_t ind_delta;
		struct Dee_memequiv *eq;

		/* See if "dst_loc" has a register/const equivalence. */
		eq = Dee_function_generator_remember_getclassof(self, Dee_memloc_getadr(dst_loc));
		if (eq != NULL) {
			struct Dee_memequiv *iter = Dee_memequiv_next(eq);
			do {
				if (Dee_memloc_gettyp(&iter->meq_loc) == MEMEQUIV_TYPE_HREG ||
				    Dee_memloc_gettyp(&iter->meq_loc) == MEMEQUIV_TYPE_CONST) {
					struct Dee_memloc reg_loc;
					ptrdiff_t val_delta = Dee_memloc_getoff(dst_loc);
					val_delta -= Dee_memloc_getoff(&eq->meq_loc);
					reg_loc = iter->meq_loc;
					ASSERT(Dee_memloc_gettyp(&reg_loc) == MEMADR_TYPE_HREG ||
					       Dee_memloc_gettyp(&reg_loc) == MEMADR_TYPE_CONST);
					Dee_memloc_adjoff(&reg_loc, val_delta);
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
	switch (Dee_memloc_gettyp(src_loc)) {
	default: {
		Dee_host_register_t not_these[2];
		not_these[0] = dst_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		if unlikely(Dee_function_generator_greg(self, src_loc, not_these))
			goto err;
	}	ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gmorph_regx2reg01(self,
		                                                Dee_memloc_hreg_getreg(src_loc),
		                                                Dee_memloc_hreg_getvaloff(src_loc) + src_delta,
		                                                cmp, dst_regno);
	case MEMADR_TYPE_HREGIND:
		return Dee_function_generator_gmorph_regind2reg01(self,
		                                                  Dee_memloc_hregind_getreg(src_loc),
		                                                  Dee_memloc_hregind_getindoff(src_loc),
		                                                  Dee_memloc_hregind_getvaloff(src_loc) + src_delta,
		                                                  cmp, dst_regno);
	case MEMADR_TYPE_HSTACKIND:
		return Dee_function_generator_gmorph_hstackind2reg01(self,
		                                                     Dee_memloc_hstackind_getcfa(src_loc),
		                                                     Dee_memloc_hstackind_getvaloff(src_loc) + src_delta,
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
	switch (Dee_memloc_gettyp(src_loc)) {
	default: {
		Dee_host_register_t not_these[3];
		not_these[0] = rhs_regno;
		not_these[1] = dst_regno;
		not_these[2] = HOST_REGISTER_COUNT;
		if unlikely(Dee_function_generator_greg(self, src_loc, not_these))
			goto err;
	}	ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gmorph_regxCreg2reg01(self,
		                                                    Dee_memloc_hreg_getreg(src_loc),
		                                                    Dee_memloc_hreg_getvaloff(src_loc) + src_delta,
		                                                    cmp, rhs_regno, dst_regno);
	case MEMADR_TYPE_HREGIND:
		return Dee_function_generator_gmorph_regindCreg2reg01(self,
		                                                      Dee_memloc_hregind_getreg(src_loc),
		                                                      Dee_memloc_hregind_getindoff(src_loc),
		                                                      Dee_memloc_hregind_getvaloff(src_loc) + src_delta,
		                                                      cmp, rhs_regno, dst_regno);
	case MEMADR_TYPE_HSTACKIND:
		return Dee_function_generator_gmorph_hstackindCreg2reg01(self,
		                                                         Dee_memloc_hstackind_getcfa(src_loc),
		                                                         Dee_memloc_hstackind_getvaloff(src_loc) + src_delta,
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
	if (Dee_memloc_gettyp(rhs_loc) != MEMADR_TYPE_HREG) {
		if (Dee_memloc_gettyp(src_loc) == MEMADR_TYPE_HREG) {
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
			if (Dee_memloc_hasreg(src_loc))
				not_these[1] = Dee_memloc_getreg(src_loc);
			if unlikely(Dee_function_generator_greg(self, rhs_loc, not_these))
				goto err;
		}
	}
	ASSERT(Dee_memloc_gettyp(rhs_loc) == MEMADR_TYPE_HREG);
	return Dee_function_generator_gmorph_locCreg2reg01(self, src_loc,
	                                                   src_delta - Dee_memloc_hreg_getvaloff(rhs_loc),
	                                                   cmp,
	                                                   Dee_memloc_hreg_getreg(rhs_loc), dst_regno);
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
		Dee_memloc_init_hreg(&uloc, dst_regno, 0);
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
	switch (Dee_memloc_gettyp(src_loc)) {
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gmorph_reg012regbooly(self,
		                                                    Dee_memloc_hreg_getreg(src_loc),
		                                                    Dee_memloc_hreg_getvaloff(src_loc) + src_delta,
		                                                    dst_regno, p_dst_delta);

	case MEMADR_TYPE_CONST: {
		DeeObject *value = Dee_False;
		if ((uintptr_t)(Dee_memloc_const_getaddr(src_loc) + src_delta) != 0)
			value = Dee_True;
		*p_dst_delta = 0;
		return Dee_function_generator_gmov_const2reg(self, value, dst_regno);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
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
		struct Dee_memval *xval = &state->ms_localv[xloc_base + i];
		if (Dee_memval_isdirect(xval) &&
		    Dee_memobj_gettyp(&xval->mv_obj0) == MEMADR_TYPE_HREG &&
		    Dee_memobj_hreg_getreg(&xval->mv_obj0) == regno) {
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
Dee_function_generator_gsavereg(struct Dee_function_generator *__restrict self,
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
	struct Dee_memval *val;
	struct Dee_memstate *state = self->fg_state;
	Dee_host_register_t regno = Dee_memloc_hregind_getreg(flush_loc);
	ptrdiff_t ind_offset = Dee_memloc_hregind_getindoff(flush_loc);
	uintptr_t cfa_offset;
	ASSERT(Dee_memloc_gettyp(flush_loc) == MEMADR_TYPE_HREGIND);
	cfa_offset = Dee_memstate_hstack_find(self->fg_state,
	                                      self->fg_state_hstack_res,
	                                      HOST_SIZEOF_POINTER);
	if (cfa_offset != (uintptr_t)-1) {
		bool did_save_temp_regno = false;
		Dee_host_register_t temp_regno;
		temp_regno = Dee_memstate_hregs_find_unused(state, true);
		if (temp_regno >= HOST_REGISTER_COUNT) {
			temp_regno = regno;
			did_save_temp_regno = true;
			if unlikely(Dee_function_generator_ghstack_pushreg(self, temp_regno))
				goto err;
		}
		if unlikely(Dee_function_generator_gmov_regind2reg(self, regno, ind_offset, temp_regno))
			goto err;
		if unlikely(Dee_function_generator_gmov_reg2hstackind(self, temp_regno, cfa_offset))
			goto err;
		if (did_save_temp_regno) {
			if unlikely(Dee_function_generator_ghstack_popreg(self, temp_regno))
				goto err;
		}
	} else {
		if unlikely(Dee_function_generator_ghstack_pushregind(self, regno, ind_offset))
			goto err;
		cfa_offset = self->fg_state->ms_host_cfa_offset;
#ifndef HOSTASM_STACK_GROWS_DOWN
		cfa_offset -= HOST_SIZEOF_POINTER;
#endif /* HOSTASM_STACK_GROWS_DOWN */
	}

	/* Convert all locations that use `MEMADR_TYPE_HREGIND:regno:off' to `MEMADR_TYPE_HSTACKIND' */
	Dee_memstate_foreach(val, state) {
		struct Dee_memobj *obj;
		Dee_memval_foreach_obj(obj, val) {
			if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HREGIND &&
			    Dee_memobj_hregind_getreg(obj) == regno &&
			    Dee_memobj_hregind_getindoff(obj) == ind_offset) {
				Dee_memstate_decrinuse(state, regno);
				Dee_memloc_init_hstackind(Dee_memobj_getloc(obj), cfa_offset,
				                          Dee_memobj_hregind_getvaloff(obj));
			}
		}
	}
	Dee_memstate_foreach_end;

	ASSERT(Dee_memloc_gettyp(flush_loc) == MEMADR_TYPE_HSTACKIND);
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
		struct Dee_memobj *obj;
		struct Dee_memval *val = &state->ms_localv[i];
		if (only_if_reference && Dee_memval_isdirect(val) && !Dee_memval_direct_isref(val))
			continue;
		Dee_memval_foreach_obj(obj, val) {
			if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HREG) {
				Dee_host_register_t regno = Dee_memobj_hreg_getreg(obj);
				ASSERT(regno < HOST_REGISTER_COUNT);
				if (register_cfa[regno] == (uintptr_t)-1) {
					register_cfa[regno] = Dee_function_generator_gsavereg(self, regno);
					if unlikely(register_cfa[regno] == (uintptr_t)-1)
						goto err;
				}
				Dee_memstate_decrinuse(state, regno);
				Dee_memloc_init_hstackind(Dee_memobj_getloc(obj), register_cfa[regno],
				                          Dee_memobj_hreg_getvaloff(obj));
			} else if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HREGIND) {
				if unlikely(Dee_function_generator_gflushregind(self, Dee_memobj_getloc(obj)))
					goto err;
				ASSERT(Dee_memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND);
			}
		}
	}
	for (i = 0; i < state->ms_stackc; ++i) {
		struct Dee_memobj *obj;
		struct Dee_memval *val = &state->ms_stackv[i];
		if (Dee_memval_isdirect(val) && !Dee_memval_direct_isref(val)) {
			if (i >= (Dee_vstackaddr_t)(state->ms_stackc - ignore_top_n_stack_if_not_ref))
				continue; /* Slot contains no reference and is in top-most n of stack. */
			if (only_if_reference)
				continue;
		}
		Dee_memval_foreach_obj(obj, val) {
			if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HREG) {
				Dee_host_register_t regno = Dee_memobj_hreg_getreg(obj);
				ASSERT(regno < HOST_REGISTER_COUNT);
				if (register_cfa[regno] == (uintptr_t)-1) {
					register_cfa[regno] = Dee_function_generator_gsavereg(self, regno);
					if unlikely(register_cfa[regno] == (uintptr_t)-1)
						goto err;
				}
				Dee_memstate_decrinuse(state, regno);
				Dee_memloc_init_hstackind(Dee_memobj_getloc(obj), register_cfa[regno],
				                          Dee_memobj_hreg_getvaloff(obj));
			} else if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HREGIND) {
				if unlikely(Dee_function_generator_gflushregind(self, Dee_memobj_getloc(obj)))
					goto err;
				ASSERT(Dee_memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND);
			}
		}
	}

	/* NOTE: Usage-registers must be cleared by the caller! */
	return 0;
err:
	return -1;
}

/* Flush memory locations that make use of `regno' onto the hstack. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gflushreg(struct Dee_function_generator *__restrict self,
                                 Dee_vstackaddr_t ignore_top_n_stack_if_not_ref,
                                 bool only_if_reference, Dee_host_register_t regno) {
	Dee_lid_t i;
	uintptr_t regno_cfa = (uintptr_t)-1;
	struct Dee_memstate *state = self->fg_state;
	ASSERT(!Dee_memstate_isshared(state));
	ASSERT(regno < HOST_REGISTER_COUNT);

	for (i = 0; i < state->ms_localc; ++i) {
		struct Dee_memobj *obj;
		struct Dee_memval *val = &state->ms_localv[i];
		if (only_if_reference && Dee_memval_isdirect(val) && !Dee_memval_direct_isref(val))
			continue;
		Dee_memval_foreach_obj(obj, val) {
			if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HREG &&
			    Dee_memobj_hreg_getreg(obj) == regno) {
				if (regno_cfa == (uintptr_t)-1) {
					regno_cfa = Dee_function_generator_gsavereg(self, regno);
					if unlikely(regno_cfa == (uintptr_t)-1)
						goto err;
				}
				Dee_memstate_decrinuse(state, regno);
				Dee_memloc_init_hstackind(Dee_memobj_getloc(obj), regno_cfa,
				                          Dee_memobj_hreg_getvaloff(obj));
			} else if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HREGIND &&
			           Dee_memobj_hregind_getreg(obj) == regno) {
				if unlikely(Dee_function_generator_gflushregind(self, Dee_memobj_getloc(obj)))
					goto err;
				ASSERT(Dee_memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND);
			}
		}
	}
	for (i = 0; i < state->ms_stackc; ++i) {
		struct Dee_memobj *obj;
		struct Dee_memval *val = &state->ms_stackv[i];
		if (Dee_memval_isdirect(val) && !Dee_memval_direct_isref(val)) {
			if (i >= (Dee_vstackaddr_t)(state->ms_stackc - ignore_top_n_stack_if_not_ref))
				continue; /* Slot contains no reference and is in top-most n of stack. */
			if (only_if_reference)
				continue;
		}
		Dee_memval_foreach_obj(obj, val) {
			if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HREG &&
			    Dee_memobj_hreg_getreg(obj) == regno) {
				if (regno_cfa == (uintptr_t)-1) {
					regno_cfa = Dee_function_generator_gsavereg(self, regno);
					if unlikely(regno_cfa == (uintptr_t)-1)
						goto err;
				}
				Dee_memstate_decrinuse(state, regno);
				Dee_memloc_init_hstackind(Dee_memobj_getloc(obj), regno_cfa,
				                          Dee_memobj_hreg_getvaloff(obj));
			} else if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HREGIND &&
			           Dee_memobj_hregind_getreg(obj) == regno) {
				if unlikely(Dee_function_generator_gflushregind(self, Dee_memobj_getloc(obj)))
					goto err;
				ASSERT(Dee_memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND);
			}
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

/* Assuming that `Dee_function_generator_gtryallocreg()' wasn't able to allocate
 * an unused register, use this function to pick which register should be picked.
 * When it's impossible to allocate *any* register, return >= HOST_REGISTER_COUNT,
 * in which case the caller should throw an exception saying that allocation was
 * impossible. */
PRIVATE WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_function_generator_gallocreg_pickreg(struct Dee_function_generator *__restrict self,
                                         Dee_host_register_t const *not_these) {
	Dee_lid_t i;
	struct Dee_memstate *state;
	Dee_host_register_t result;

	/* TODO: Look at equivalence classes of registers. If a class exists that
	 *       contains more than 1 register, pick one of those registers and
	 *       update all Dee_memloc-s to use one of the other registers, then
	 *       return that register to the caller. */

	/* TODO: Look at equivalence classes of registers. If a class exists that
	 *       contains both a register and a CONST item, update all Dee_memloc-s
	 *       that use the register to instead refer to the CONST, then return
	 *       that register to the caller. */

	/* TODO: Figure out which register requires the least amount of flushes.
	 *       For this purpose, every HREGIND with a distinct v_indoff requires
	 *       an additional flush for the corresponding register. */

	/* TODO: Within the set of registers that requires the least # of flushes,
	 *       as per the previous TODO, look at the equivalence classes of those
	 *       registers, and narrow down the set of registers to those where the
	 *       equivalence class is either empty, or contains the least number of
	 *       elements.
	 * Then, from this sub-set, pick a random register that will be the one to
	 * be returned to our caller. */

	/* TODO: Remove the below implementation and implement the above TODOs */
	state = self->fg_state;
	for (i = 0; i < state->ms_localc; ++i) {
		struct Dee_memobj *obj;
		struct Dee_memval *val = &state->ms_localv[i];
		Dee_memval_foreach_obj(obj, val) {
			if (!Dee_memobj_hasreg(obj))
				continue;
			result = Dee_memobj_getreg(obj);
			if (!nullable_host_register_list_contains(not_these, result))
				return result;
		}
	}
	state = self->fg_state;
	for (i = 0; i < state->ms_stackc; ++i) {
		struct Dee_memobj *obj;
		struct Dee_memval *val = &state->ms_stackv[i];
		Dee_memval_foreach_obj(obj, val) {
			if (!Dee_memobj_hasreg(obj))
				continue;
			result = Dee_memobj_getreg(obj);
			if (!nullable_host_register_list_contains(not_these, result))
				return result;
		}
	}

	return HOST_REGISTER_COUNT;
}

/* Allocate at host register, possibly flushing an already used register to stack.
 * @param: not_these: Array of registers not to allocated, terminated by one `>= HOST_REGISTER_COUNT'.
 * @return: * : The allocated register
 * @return: >= HOST_REGISTER_COUNT: Error */
INTERN WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_function_generator_gallocreg(struct Dee_function_generator *__restrict self,
                                 Dee_host_register_t const *not_these) {
	Dee_host_register_t result;
	result = Dee_function_generator_gtryallocreg(self, not_these);
	if unlikely(result >= HOST_REGISTER_COUNT) {
		/* Pick the register that should be allocated. */
		result = Dee_function_generator_gallocreg_pickreg(self, not_these);
		if unlikely(result >= HOST_REGISTER_COUNT)
			goto err_no_way_to_allocate;
		if unlikely(Dee_function_generator_gflushreg(self, 0, false, result))
			goto err;
	}
	return result;

err_no_way_to_allocate:
	/* Impossible to allocate register. */
	DeeError_Throwf(&DeeError_IllegalInstruction,
	                "No way to allocate register");
err:
	return HOST_REGISTER_COUNT;
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
	if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_CONST &&
	    Dee_memloc_const_getaddr(loc) != 0)
		return 0;
	info = Dee_function_generator_except_exit(self);
	if unlikely(!info)
		goto err;
	{
		Dee_function_generator_DEFINE_Dee_host_symbol_section(self, err, Lexcept, &info->exi_block->bb_htext, 0);
		if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_CONST)
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
	if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_CONST &&
	    Dee_memloc_const_getaddr(loc) == 0)
		return 0;
	info = Dee_function_generator_except_exit(self);
	if unlikely(!info)
		goto err;
	{
		Dee_function_generator_DEFINE_Dee_host_symbol_section(self, err, Lexcept, &info->exi_block->bb_htext, 0);
		if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_CONST)
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
	Dee_memloc_init_const(&threshold_loc, (byte_t const *)(uintptr_t)threshold);
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
		if (n_pop != 0) {
			Dee_vstackaddr_t i;
			struct Dee_memval *pop_base;
			if unlikely(Dee_function_generator_state_unshare(self))
				goto err_saved_state;

			/* If any of the memory locations that need to be popped is
			 * MEMVAL_VMORPH_NULLABLE, then we must alter it to become
			 * MEMVAL_VMORPH_DIRECT with the MEMOBJ_F_NOREF flag set.
			 *
			 * Because we're allowed to assume that only 1 exception
			 * may be pending handling at a time, and given the fact
			 * that at this point in the code we know that an exception
			 * is active, we can infer that no other NULLABLE vstack
			 * item can actually be NULL at runtime. */
			pop_base = self->fg_state->ms_stackv + self->fg_state->ms_stackc - n_pop;
			for (i = 0; i < n_pop; ++i) {
				if (Dee_memval_isnullable(&pop_base[i])) {
					Dee_memval_nullable_makedirect(&pop_base[i]);
					Dee_memval_direct_clearref(&pop_base[i]);
				}
			}

			/* Pop vstack items which the injection handler doesn't care about. */
			if unlikely(Dee_function_generator_vpopmany(self, n_pop))
				goto err_saved_state;
		}
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
	Dee_memloc_init_const(&compare_value_loc, (byte_t const *)(uintptr_t)threshold);
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
	if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_CONST) {
		bool should_jump_except = false;
		intptr_t lhs = (intptr_t)(uintptr_t)Dee_memloc_const_getaddr(loc);
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
	switch (Dee_memloc_gettyp(api_function_loc)) {
	default: {
		size_t i;
		Dee_host_register_t regno, not_these[HOST_REGISTER_COUNT + 1];
		bitset_t not_these_bitset[_bitset_sizeof(HOST_REGISTER_COUNT)];
fallback:
		bitset_clearall(not_these_bitset, HOST_REGISTER_COUNT);
		for (i = 0; i < argc; ++i) {
			struct Dee_memloc *arg = &argv[i];
			if (Dee_memloc_hasreg(arg))
				bitset_set(not_these_bitset, Dee_memloc_getreg(arg));
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
	case MEMADR_TYPE_HREG:
		if unlikely(Dee_memloc_hreg_getvaloff(api_function_loc) != 0) {
			result = Dee_function_generator_gmov_regx2reg(self,
			                                              Dee_memloc_hreg_getreg(api_function_loc),
			                                              Dee_memloc_hreg_getvaloff(api_function_loc),
			                                              Dee_memloc_hreg_getreg(api_function_loc));
			if unlikely(result != 0)
				break;
			Dee_memstate_hregs_adjust_delta(self->fg_state,
			                                Dee_memloc_hreg_getreg(api_function_loc),
			                                Dee_memloc_hreg_getvaloff(api_function_loc));
			Dee_memloc_hreg_setvaloff(api_function_loc, 0);
		}
		result = _Dee_function_generator_gcalldynapi_reg(self, Dee_memloc_hreg_getreg(api_function_loc), argc, argv);
		break;
	case MEMADR_TYPE_HSTACKIND:
		if unlikely(Dee_memloc_hstackind_getvaloff(api_function_loc) != 0)
			goto fallback;
		result = _Dee_function_generator_gcalldynapi_hstackind(self, Dee_memloc_hstackind_getcfa(api_function_loc), argc, argv);
		break;
	case MEMADR_TYPE_HREGIND:
		if unlikely(Dee_memloc_hregind_getvaloff(api_function_loc) != 0)
			goto fallback;
		result = _Dee_function_generator_gcalldynapi_hregind(self,
		                                                     Dee_memloc_hregind_getreg(api_function_loc),
		                                                     Dee_memloc_hregind_getindoff(api_function_loc),
		                                                     argc, argv);
		break;
	case MEMADR_TYPE_CONST:
		result = _Dee_function_generator_gcallapi(self, (void const *)Dee_memloc_const_getaddr(api_function_loc), argc, argv);
		break;
	}
	if likely(result == 0)
		Dee_function_generator_remember_undefined_allregs(self);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gincref_loc(struct Dee_function_generator *__restrict self,
                                   struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (Dee_memloc_gettyp(loc)) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gincref_regx(self,
		                                           Dee_memloc_hreg_getreg(loc),
		                                           Dee_memloc_hreg_getvaloff(loc),
		                                           n);
	case MEMADR_TYPE_CONST:
		return Dee_function_generator_gincref_const(self, (DeeObject *)Dee_memloc_const_getaddr(loc), n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdecref_loc(struct Dee_function_generator *__restrict self,
                                   struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (Dee_memloc_gettyp(loc)) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gdecref_regx(self,
		                                           Dee_memloc_hreg_getreg(loc),
		                                           Dee_memloc_hreg_getvaloff(loc),
		                                           n);
	case MEMADR_TYPE_CONST:
		return Dee_function_generator_gdecref_const(self, (DeeObject *)Dee_memloc_const_getaddr(loc), n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdecref_dokill_loc(struct Dee_function_generator *__restrict self,
                                          struct Dee_memloc *__restrict loc) {
	switch (Dee_memloc_gettyp(loc)) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gdecref_regx_dokill(self,
		                                                  Dee_memloc_hreg_getreg(loc),
		                                                  Dee_memloc_hreg_getvaloff(loc));
	case MEMADR_TYPE_CONST:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "decref_dokill called on CONST location. "
		                       "Constants can never be destroyed");
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdecref_nokill_loc(struct Dee_function_generator *__restrict self,
                                          struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (Dee_memloc_gettyp(loc)) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gdecref_nokill_regx(self,
		                                                  Dee_memloc_hreg_getreg(loc),
		                                                  Dee_memloc_hreg_getvaloff(loc),
		                                                  n);
	case MEMADR_TYPE_CONST:
		return Dee_function_generator_gdecref_const(self, (DeeObject *)Dee_memloc_const_getaddr(loc), n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxincref_loc(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (Dee_memloc_gettyp(loc)) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gxincref_regx(self,
		                                            Dee_memloc_hreg_getreg(loc),
		                                            Dee_memloc_hreg_getvaloff(loc),
		                                            n);
	case MEMADR_TYPE_CONST:
		if (Dee_memloc_const_getaddr(loc) == NULL)
			return 0;
		return Dee_function_generator_gincref_const(self, (DeeObject *)Dee_memloc_const_getaddr(loc), n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxdecref_loc(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (Dee_memloc_gettyp(loc)) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gxdecref_regx(self,
		                                            Dee_memloc_hreg_getreg(loc),
		                                            Dee_memloc_hreg_getvaloff(loc),
		                                            n);
	case MEMADR_TYPE_CONST:
		if (Dee_memloc_const_getaddr(loc) == NULL)
			return 0;
		return Dee_function_generator_gdecref_const(self, (DeeObject *)Dee_memloc_const_getaddr(loc), n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxdecref_nokill_loc(struct Dee_function_generator *__restrict self,
                                           struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (Dee_memloc_gettyp(loc)) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gxdecref_nokill_regx(self,
		                                                   Dee_memloc_hreg_getreg(loc),
		                                                   Dee_memloc_hreg_getvaloff(loc),
		                                                   n);
	case MEMADR_TYPE_CONST:
		if (Dee_memloc_const_getaddr(loc) == NULL)
			return 0;
		return Dee_function_generator_gdecref_const(self, (DeeObject *)Dee_memloc_const_getaddr(loc), n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

/* Change `loc' into the value of `<loc> = *(<loc> + ind_delta)'
 * Note that unlike the `Dee_function_generator_gmov*' functions, this
 * one may use `MEMADR_TYPE_*IND' to defer the indirection until later. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gind(struct Dee_function_generator *__restrict self,
                            struct Dee_memloc *loc, ptrdiff_t ind_delta) {
	struct Dee_memequiv *eq;
	switch (Dee_memloc_gettyp(loc)) {

	case MEMADR_TYPE_HSTACK: {
		uintptr_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(Dee_memloc_hstack_getcfa(loc), ind_delta);
		eq = Dee_function_generator_remember_getclassof_hstackind(self, cfa_offset);
		if (eq != NULL) {
			Dee_memequiv_next_asloc(eq, loc);
			if (Dee_memstate_ismemlocinstate(self->fg_state, loc))
				Dee_memstate_incrinuse_for_memloc(self->fg_state, loc);
			return 0;
		}
		Dee_memloc_init_hstackind(loc, cfa_offset, 0);
		return 0;
	}	break;

	case MEMADR_TYPE_CONST: {
		void const *p_value = Dee_memloc_const_getaddr(loc) + ind_delta;
		Dee_host_register_t temp_regno;
		temp_regno = Dee_function_generator_gallocreg(self, NULL);
		if unlikely(temp_regno >= HOST_REGISTER_COUNT)
			goto err;
		if unlikely(Dee_function_generator_gmov_constind2reg(self, (void const **)p_value, temp_regno))
			goto err;
		Dee_memloc_init_hreg(loc, temp_regno, 0);
		if (Dee_memstate_ismemlocinstate(self->fg_state, loc))
			Dee_memstate_incrinuse(self->fg_state, temp_regno);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ASSERT(Dee_memloc_gettyp(loc) == MEMADR_TYPE_HREG);
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG: {
		Dee_host_register_t regno = Dee_memloc_hreg_getreg(loc);
		ptrdiff_t final_ind_delta = Dee_memloc_hreg_getvaloff(loc) + ind_delta;
		eq = Dee_function_generator_remember_getclassof_regind(self, regno, final_ind_delta);
		if (eq != NULL) {
			bool is_in_state = Dee_memstate_ismemlocinstate(self->fg_state, loc);
			if (is_in_state)
				Dee_memstate_decrinuse(self->fg_state, regno);
			Dee_memequiv_next_asloc(eq, loc);
			if (is_in_state)
				Dee_memstate_incrinuse_for_memloc(self->fg_state, loc);
			return 0;
		}
		/* Turn the location from an HREG into HREGIND */
		Dee_memloc_init_hregind(loc, regno, final_ind_delta, 0);
	}	break;

	}
	return 0;
err:
	return -1;
}

/* Force `loc' to become a register (`MEMADR_TYPE_HREG'). */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_greg(struct Dee_function_generator *__restrict self,
                            struct Dee_memloc *loc,
                            Dee_host_register_t const *not_these) {
	struct Dee_memequiv *eq;
	struct Dee_memstate *state;
	Dee_host_register_t regno;
	ptrdiff_t val_delta;
	bool is_in_state;
	if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_HREG)
		return 0; /* Already in a register! */

	/* Check if "loc" has a known register equivalence. */
	eq = Dee_function_generator_remember_getclassof(self, Dee_memloc_getadr(loc));
	if (eq != NULL) {
		struct Dee_memequiv *reg_eq = Dee_memequiv_next(eq);
		while (Dee_memloc_gettyp(&reg_eq->meq_loc) != MEMEQUIV_TYPE_HREG) {
			if (reg_eq == eq)
				goto no_equivalence;
			reg_eq = Dee_memequiv_next(reg_eq);
		}

		/* Example:
		 * >> eq(FOO + 3)  == reg_eq(BAR + 5)
		 * >> loc(FOO + 7) == reg_eq(BAR + 9)
		 * >> 9 = 7 - 3 + 5
		 * >> 9 == RESULT_VAL_DELTA
		 * >> 7 == Dee_memloc_getoff(loc)
		 * >> 3 == Dee_memloc_getoff(&eq->meq_loc)
		 * >> 5 == Dee_memloc_getoff(&reg_eq->meq_loc)   (gets added in `Dee_memequiv_asloc()') */
		val_delta = Dee_memloc_getoff(loc);
		val_delta -= Dee_memloc_getoff(&eq->meq_loc);
		is_in_state = Dee_memstate_ismemlocinstate(self->fg_state, loc);
		if (is_in_state)
			Dee_memstate_decrinuse_for_memloc(self->fg_state, loc);
		*loc = reg_eq->meq_loc;
		ASSERT(Dee_memloc_gettyp(loc) == MEMADR_TYPE_HREG);
		Dee_memloc_adjoff(loc, val_delta);
		if (is_in_state)
			Dee_memstate_incrinuse(self->fg_state, Dee_memloc_hreg_getreg(loc));
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
	if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_HREGIND ||
	    Dee_memloc_gettyp(loc) == MEMADR_TYPE_HSTACKIND) {
		ptrdiff_t delta_change = val_delta - Dee_memloc_getoff(loc);
		struct Dee_memval *alias_val;
		struct Dee_memobj *alias_obj;
		Dee_memstate_foreach(alias_val, state) {
			Dee_memval_foreach_obj(alias_obj, alias_val) {
				if (!Dee_memloc_sameadr(Dee_memobj_getloc(alias_obj), loc))
					continue;
				if (Dee_memobj_getloc(alias_obj) == loc)
					continue;
				if (Dee_memobj_gettyp(alias_obj) == MEMADR_TYPE_HSTACKIND &&
				    (alias_val->mv_obj0.mo_flags & MEMOBJ_F_LINEAR))
					continue; /* The alias must remain on-stack (don't update it) */
				Dee_memstate_decrinuse_for_memobj(self->fg_state, alias_obj);
				Dee_memloc_init_hreg(Dee_memobj_getloc(alias_obj), regno,
				                     Dee_memobj_getoff(alias_obj) + delta_change);
				Dee_memstate_incrinuse(self->fg_state, regno);
			}
		}
		Dee_memstate_foreach_end;
	}

	/* Adjust register usage if `loc' is tracked by the memory state. */
	is_in_state = Dee_memstate_ismemlocinstate(state, loc);
	if (is_in_state)
		Dee_memstate_decrinuse_for_memloc(self->fg_state, loc);

	/* Remember that `loc' now lies in a register. */
	Dee_memloc_init_hreg(loc, regno, val_delta);
	if (is_in_state)
		Dee_memstate_incrinuse(self->fg_state, regno);
	return 0;
err:
	return -1;
}

/* Force `loc' to reside on the stack, giving it an address
 * (`MEMADR_TYPE_HSTACKIND, Dee_memloc_hstackind_getvaloff = 0').
 * @param: require_valoff_0: When false, forgo the exit requirement of `Dee_memloc_hstackind_getvaloff = 0' */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gflush(struct Dee_function_generator *__restrict self,
                              struct Dee_memloc *loc, bool require_valoff_0) {
	ptrdiff_t val_offset;
	uintptr_t cfa_offset;
	struct Dee_memstate *state = self->fg_state;
	ASSERT(!Dee_memstate_isshared(state));
	if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_HSTACKIND) {
handle_hstackind_loc:
		if (Dee_memloc_hstackind_getvaloff(loc) == 0)
			return 0; /* Already on-stack at offset=0 */
		if (require_valoff_0)
			return 0; /* Caller doesn't care about value offset */

#ifdef HAVE__Dee_host_section_gadd_const2hstackind
		/* emit `addP $..., sp_offset(%Psp)' to adjust the offset of the stored value
		 * Afterwards, go through all stack/local variables and adjust value offsets
		 * wherever the same CFA offset is referenced. */
		return _Dee_host_section_gadd_const2hstackind(self->fg_sect,
		                                              (void const *)(uintptr_t)(intptr_t)Dee_memloc_hstackind_getvaloff(loc),
		                                              Dee_memstate_hstack_cfa2sp(self->fg_state, Dee_memloc_hstackind_getcfa(loc)));
#endif /* HAVE__Dee_host_section_gadd_const2hstackind */
	}

	/* Figure out where we want to allocate the value. */
#ifdef HAVE_try_restore_xloc_arg_cfa_offset
	if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_HREG &&
		(cfa_offset = try_restore_xloc_arg_cfa_offset(self, Dee_memloc_hreg_getreg(loc))) != (uintptr_t)-1) {
		val_offset = 0; /* CFA offset restored */
	} else
#endif /* HAVE_try_restore_xloc_arg_cfa_offset */
	{
		/* Check if "loc" has a known HSTACKIND equivalence. */
		struct Dee_memequiv *eq;

		eq = Dee_function_generator_remember_getclassof(self, Dee_memloc_getadr(loc));
		if (eq != NULL) {
			ptrdiff_t val_delta = Dee_memloc_getoff(loc);
			struct Dee_memequiv *hstackind_eq_any = NULL;
			struct Dee_memequiv *hstackind_eq;
			val_delta -= Dee_memloc_getoff(&eq->meq_loc);
			for (hstackind_eq = Dee_memequiv_next(eq); hstackind_eq != eq;
			     hstackind_eq = Dee_memequiv_next(hstackind_eq)) {
				if (Dee_memloc_gettyp(&hstackind_eq->meq_loc) == MEMEQUIV_TYPE_HSTACKIND) {
					if ((Dee_memloc_getoff(&hstackind_eq->meq_loc) + val_delta) == 0) {
						/* Perfect match: this equivalence allows for `v_hstack.s_off = 0' */
						if (Dee_memstate_ismemlocinstate(state, loc))
							Dee_memstate_decrinuse_for_memloc(state, loc);
						*loc = hstackind_eq->meq_loc;
						ASSERT((Dee_memloc_getoff(loc) + val_delta) == 0);
						Dee_memloc_setoff(loc, 0);
						ASSERT(Dee_memloc_gettyp(loc) == MEMADR_TYPE_HSTACKIND);
						ASSERT(Dee_memloc_getoff(loc) == 0);
						return 0;
					}
					hstackind_eq_any = hstackind_eq;
				}
			}
			if (hstackind_eq_any) {
				if (Dee_memstate_ismemlocinstate(state, loc))
					Dee_memstate_decrinuse_for_memloc(state, loc);
				*loc = hstackind_eq->meq_loc;
				Dee_memloc_setoff(loc, Dee_memloc_getoff(loc) + val_delta);
				ASSERT(Dee_memloc_gettyp(loc) == MEMADR_TYPE_HSTACKIND);
				ASSERT(Dee_memloc_getoff(loc) != 0);
				goto handle_hstackind_loc;
			}
		}

		/* Search for a currently free stack location. */
		cfa_offset = Dee_memstate_hstack_find(state, self->fg_state_hstack_res, HOST_SIZEOF_POINTER);
		val_offset = loc->ml_off;
		if (!require_valoff_0)
			loc->ml_off = 0; /* Don't include value offset when saving location */
		if (cfa_offset != (uintptr_t)-1) {
			if unlikely(Dee_function_generator_gmov_loc2hstackind(self, loc, cfa_offset)) {
err_restore_val_offset:
				loc->ml_off = val_offset;
				goto err;
			}
		} else {
			if unlikely(Dee_function_generator_ghstack_pushloc(self, loc))
				goto err_restore_val_offset;
#ifdef HOSTASM_STACK_GROWS_DOWN
			cfa_offset = state->ms_host_cfa_offset;
#else /* HOSTASM_STACK_GROWS_DOWN */
			cfa_offset = state->ms_host_cfa_offset - HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		}
		loc->ml_off = val_offset;
		if (require_valoff_0)
			val_offset = 0;
	}

	/* If the location used to be a writable location, then we must
	 * also update any other location that used to alias `loc'. */
	state = self->fg_state;
	if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_HREG ||
	    Dee_memloc_gettyp(loc) == MEMADR_TYPE_HREGIND) {
		ptrdiff_t val_delta_change = -Dee_memloc_getoff(loc);
		if (val_delta_change != val_offset) {
			struct Dee_memval *alias_val;
			struct Dee_memobj *alias_obj;
			ASSERT(val_offset == 0);
			val_delta_change += val_offset;
			Dee_memstate_foreach(alias_val, state) {
				Dee_memval_foreach_obj(alias_obj, alias_val) {
					if (!Dee_memloc_sameadr(Dee_memobj_getloc(alias_obj), loc))
						continue;
					if (Dee_memobj_getloc(alias_obj) == loc)
						continue;
					ASSERT(Dee_memobj_hasreg(alias_obj));
					Dee_memstate_decrinuse(self->fg_state, Dee_memobj_getreg(alias_obj));
					Dee_memloc_init_hstackind(Dee_memobj_getloc(alias_obj), cfa_offset,
					                          Dee_memobj_getoff(alias_obj) + val_delta_change);
				}
			}
			Dee_memstate_foreach_end;
		}
	}

	/* Adjust register usage if `loc' is tracked by the memory state. */
	if (Dee_memloc_hasreg(loc) && Dee_memstate_ismemlocinstate(state, loc))
		Dee_memstate_decrinuse(self->fg_state, Dee_memloc_getreg(loc));

	/* Remember that `loc' now lies on-stack (with an offset of `val_offset') */
	Dee_memloc_init_hstackind(loc, cfa_offset, val_offset);
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
	switch (Dee_memloc_gettyp(dst_loc)) {
	case MEMADR_TYPE_HREG:
		result = Dee_function_generator_gmov_loc2regx(self, src_loc,
		                                              Dee_memloc_hreg_getreg(dst_loc),
		                                              Dee_memloc_hreg_getvaloff(dst_loc));
		break;
	case MEMADR_TYPE_HSTACKIND:
		result = Dee_function_generator_gmov_loc2hstackindx(self, src_loc,
		                                                    Dee_memloc_hstackind_getcfa(dst_loc),
		                                                    Dee_memloc_hstackind_getvaloff(dst_loc));
		break;
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	default:
		switch (Dee_memloc_gettyp(src_loc)) {
		default: {
			Dee_host_register_t not_these[2];
			not_these[0] = HOST_REGISTER_COUNT;
			not_these[1] = HOST_REGISTER_COUNT;
			if (Dee_memloc_hasreg(dst_loc))
				not_these[0] = Dee_memloc_getreg(dst_loc);
			if unlikely(Dee_function_generator_greg(self, src_loc, not_these))
				goto err;
			ASSERT(Dee_memloc_gettyp(src_loc) == MEMADR_TYPE_HREG);
		}	ATTR_FALLTHROUGH
		case MEMADR_TYPE_HREG:
			result = Dee_function_generator_gmov_regx2loc(self,
			                                              Dee_memloc_hreg_getreg(src_loc),
			                                              Dee_memloc_hreg_getvaloff(src_loc),
			                                              dst_loc);
			break;
		case MEMADR_TYPE_CONST:
			result = Dee_function_generator_gmov_const2loc(self, Dee_memloc_const_getaddr(src_loc), dst_loc);
			break;
		case MEMADR_TYPE_HSTACK:
			result = Dee_function_generator_gmov_hstack2loc(self, Dee_memloc_hstack_getcfa(src_loc), dst_loc);
			break;
		case MEMADR_TYPE_UNDEFINED:
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
	switch (Dee_memloc_gettyp(src_loc)) {
	default: {
		Dee_host_register_t not_these[2];
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (Dee_memloc_hasreg(dst_loc))
			not_these[0] = Dee_memloc_getreg(dst_loc);
		if unlikely(Dee_function_generator_greg(self, src_loc, not_these))
			goto err;
		ASSERT(Dee_memloc_gettyp(src_loc) == MEMADR_TYPE_HREG);
	}	ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		if (Dee_memloc_hreg_getvaloff(src_loc) != 0) {
			if (Dee_function_generator_gmov_regx2reg(self,
			                                         Dee_memloc_hreg_getreg(src_loc),
			                                         Dee_memloc_hreg_getvaloff(src_loc),
			                                         Dee_memloc_hreg_getreg(src_loc)))
				goto err;
			Dee_memstate_hregs_adjust_delta(self->fg_state,
			                                Dee_memloc_hreg_getreg(src_loc),
			                                Dee_memloc_hreg_getvaloff(src_loc));
			Dee_memloc_hreg_setvaloff(src_loc, 0);
		}
		result = Dee_function_generator_gmov_reg2locind(self,
		                                                Dee_memloc_hreg_getreg(src_loc),
		                                                dst_loc, ind_delta);
		break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	case MEMADR_TYPE_CONST:
		result = Dee_function_generator_gmov_const2locind(self,
		                                                  Dee_memloc_const_getaddr(src_loc),
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
	Dee_memloc_init_const(&loc, lock);
	return Dee_function_generator_grwlock_read(self, &loc);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_grwlock_write_const(struct Dee_function_generator *__restrict self,
                                           Dee_atomic_rwlock_t *__restrict lock) {
	struct Dee_memloc loc;
	Dee_memloc_init_const(&loc, lock);
	return Dee_function_generator_grwlock_write(self, &loc);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_grwlock_endread_const(struct Dee_function_generator *__restrict self,
                                             Dee_atomic_rwlock_t *__restrict lock) {
	struct Dee_memloc loc;
	Dee_memloc_init_const(&loc, lock);
	return Dee_function_generator_grwlock_endread(self, &loc);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_grwlock_endwrite_const(struct Dee_function_generator *__restrict self,
                                              Dee_atomic_rwlock_t *__restrict lock) {
	struct Dee_memloc loc;
	Dee_memloc_init_const(&loc, lock);
	return Dee_function_generator_grwlock_endwrite(self, &loc);
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_COMMON_C */
