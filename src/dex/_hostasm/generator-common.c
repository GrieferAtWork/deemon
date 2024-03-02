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
#include <deemon/alloc.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/tuple.h>
#include <deemon/util/lock.h>

#include <hybrid/overflow.h>

#include "utils.h"

DECL_BEGIN

/************************************************************************/
/* COMMON CODE GENERATION FUNCTIONS                                     */
/************************************************************************/

#ifdef __INTELLISENSE__
#define DO /* nothing */
#else /* __INTELLISENSE__ */
#define DO(x) if unlikely(x) goto err
#endif /* !__INTELLISENSE__ */
#define EDO(err, x) if unlikely(x) goto err


#undef NEED_constasreg
#ifdef _Dee_function_generator_gmov_const2regind_MAYFAIL
#define NEED_constasreg
#endif /* _Dee_function_generator_gmov_const2regind_MAYFAIL */
#ifdef _Dee_function_generator_gmov_const2hstackind_MAYFAIL
#define NEED_constasreg
#endif /* _Dee_function_generator_gmov_const2hstackind_MAYFAIL */
#ifdef _Dee_host_section_gmov_const2constind_MAYFAIL
#define NEED_constasreg
#endif /* _Dee_host_section_gmov_const2constind_MAYFAIL */
#ifdef _Dee_function_generator_gmov_constind2reg_MAYFAIL
#define NEED_constasreg
#endif /* _Dee_function_generator_gmov_constind2reg_MAYFAIL */
#ifdef _Dee_function_generator_gmov_reg2constind_MAYFAIL
#define NEED_constasreg
#endif /* _Dee_function_generator_gmov_reg2constind_MAYFAIL */
#ifdef _Dee_function_generator_gbitop_regconst2reg_MAYFAIL
#define NEED_constasreg
#endif /* !_Dee_function_generator_gbitop_regconst2reg_MAYFAIL */
#ifdef _Dee_function_generator_gjcc_regindCconst_MAYFAIL
#define NEED_constasreg
#endif /* !_Dee_function_generator_gjcc_regindCconst_MAYFAIL */
#ifdef _Dee_function_generator_gjcc_regCconst_MAYFAIL
#define NEED_constasreg
#endif /* !_Dee_function_generator_gjcc_regCconst_MAYFAIL */
#ifdef _Dee_function_generator_gjcc_hstackindCconst_MAYFAIL
#define NEED_constasreg
#endif /* !_Dee_function_generator_gjcc_hstackindCconst_MAYFAIL */
#if defined(HAVE__Dee_function_generator_gjcc_regindAconst) && defined(_Dee_function_generator_gjcc_regindAconst_MAYFAIL)
#define NEED_constasreg
#endif /* HAVE__Dee_function_generator_gjcc_regindAconst && _Dee_function_generator_gjcc_regindAconst_MAYFAIL */
#if defined(HAVE__Dee_function_generator_gjcc_regAconst) && defined(_Dee_function_generator_gjcc_regAconst_MAYFAIL)
#define NEED_constasreg
#endif /* _Dee_function_generator_gjcc_regAconst && _Dee_function_generator_gjcc_regAconst_MAYFAIL */
#if defined(HAVE__Dee_function_generator_gjcc_hstackindAconst) && defined(_Dee_function_generator_gjcc_hstackindAconst_MAYFAIL)
#define NEED_constasreg
#endif /* HAVE__Dee_function_generator_gjcc_hstackindAconst && _Dee_function_generator_gjcc_hstackindAconst_MAYFAIL */


#ifdef NEED_constasreg
#ifdef Dee_function_generator_gmov_const2reg_MAYFAIL
#error "This function is NOT allowed to fail!"
#endif /* Dee_function_generator_gmov_const2reg_MAYFAIL */
INTERN WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_function_generator_gconst_as_reg(struct Dee_function_generator *__restrict self,
                                     void const *value,
                                     Dee_host_register_t const *not_these) {
	Dee_host_register_t result;
	result = Dee_function_generator_gallocreg(self, not_these);
	if likely(result < HOST_REGISTER_COUNT) {
		if unlikely(Dee_function_generator_gmov_const2reg(self, value, result))
			goto err;
	}
	return result;
err:
	return HOST_REGISTER_COUNT;
}
#endif /* NEED_constasreg */


/* Bit operations */
INTERN ATTR_CONST WUNUSED uintptr_t DCALL
Dee_bitop_forconst(Dee_bitop_t op, uintptr_t lhs, uintptr_t rhs) {
	uintptr_t result;
	switch (op) {
	case BITOP_AND:
		result = lhs & rhs;
		break;
	case BITOP_OR:
		result = lhs | rhs;
		break;
	case BITOP_XOR:
		result = lhs ^ rhs;
		break;
	default: __builtin_unreachable();
	}
	return result;
}

/* @return: true:  overflow
 * @return: false: no overflow */
INTERN WUNUSED NONNULL((4)) bool DCALL
Dee_arithop_forconst(Dee_bitop_t op, uintptr_t lhs, uintptr_t rhs,
                     uintptr_t *__restrict p_result) {
	bool did_overfow;
	switch (op) {
	case ARITHOP_UADD:
		did_overfow = OVERFLOW_UADD(lhs, rhs, p_result);
		break;
	case ARITHOP_SADD:
		did_overfow = OVERFLOW_SADD((intptr_t)lhs, (intptr_t)rhs, (intptr_t *)p_result);
		break;
	case ARITHOP_USUB:
		did_overfow = OVERFLOW_USUB(lhs, rhs, p_result);
		break;
	case ARITHOP_SSUB:
		did_overfow = OVERFLOW_SSUB((intptr_t)lhs, (intptr_t)rhs, (intptr_t *)p_result);
		break;
	case ARITHOP_UMUL:
		did_overfow = OVERFLOW_UMUL(lhs, rhs, p_result);
		break;
	case ARITHOP_SMUL:
		did_overfow = OVERFLOW_SMUL((intptr_t)lhs, (intptr_t)rhs, (intptr_t *)p_result);
		break;
	default: __builtin_unreachable();
	}
	return did_overfow;
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
		Dee_memval_foreach_obj_end;
	}
	Dee_memstate_foreach_end;
	/* TODO: Must also forget anything related to equivalences when it comes to
	 *       the indirection of "mobj":
	 * >> local x = [10, 20];
	 * >> foo(x); // When this clears the ONEREF flag, DeeList_ELEM(x) is no longer
	 * >>         // known (and transitively also DeeList_GET(x, 0), ...)
	 *
	 * XXX: Is this actually a problem? indirection requires registers, but since
	 *      we never use callee-preserve registers, *all* registers are always,
	 *      already marked as clobbered after a function call... */
	Dee_memobj_clearoneref(mobj);
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
Dee_memstate_remember_undefined_hstack_after_redzone(struct Dee_memstate *__restrict self) {
	Dee_cfa_t min_undef_cfa;
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
		if (cfa_delta < 0) {
			/* Remember that values beyond the stack's red zone become undefined,
			 * as they might get clobbered by sporadic interrupt handlers. */
			if (cfa_delta == -HOST_SIZEOF_POINTER) {
#ifdef HOSTASM_STACK_GROWS_DOWN
				Dee_cfa_t pop_dst_cfa_offset = self->fg_state->ms_host_cfa_offset + HOST_SIZEOF_POINTER;
#else /* HOSTASM_STACK_GROWS_DOWN */
				Dee_cfa_t pop_dst_cfa_offset = self->fg_state->ms_host_cfa_offset;
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
	Dee_cfa_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	int result;
#if HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER
	/* TODO: If "[#dst_cfa_offset]" is a known equivalence of "src_regno", do nothing */
#endif /* HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER */
	result = _Dee_function_generator_ghstack_pushreg(self, src_regno);
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_reg2hstackind(self, src_regno, dst_cfa_offset);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_pushregind(struct Dee_function_generator *__restrict self,
                                          Dee_host_register_t src_regno, ptrdiff_t src_delta) {
	Dee_cfa_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	int result;
#if HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER
	/* TODO: If "[#dst_cfa_offset]" is a known equivalence of "[src_regno + src_delta]", do nothing */
#endif /* HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER */
	result = _Dee_function_generator_ghstack_pushregind(self, src_regno, src_delta);
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_regind2hstackind(self, src_regno, src_delta, dst_cfa_offset);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_pushconst(struct Dee_function_generator *__restrict self,
                                         void const *value) {
	Dee_cfa_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	int result;
#if HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER
	/* TODO: If "[#dst_cfa_offset]" is a known equivalence of "value", do nothing */
#endif /* HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER */
	result = _Dee_function_generator_ghstack_pushconst(self, value);
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_const2hstackind(self, value, dst_cfa_offset);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_pushhstackind(struct Dee_function_generator *__restrict self,
                                             Dee_cfa_t cfa_offset) {
	Dee_cfa_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	int result;
#if HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER
	/* TODO: If "[#dst_cfa_offset]" is a known equivalence of "[#cfa_offset]", do nothing */
#endif /* HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER */
	result = _Dee_function_generator_ghstack_pushhstackind(self, sp_offset);
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_hstackind2hstackind(self, cfa_offset, dst_cfa_offset);
	return result;
}

#ifdef HAVE_Dee_function_generator_ghstack_pushhstack_at_cfa_boundary_np
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_pushhstack_at_cfa_boundary_np(struct Dee_function_generator *__restrict self) {
	Dee_cfa_t src_cfa_offset = GET_ADDRESS_OF_NEXT_POP(self);
	Dee_cfa_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	int result = _Dee_function_generator_ghstack_pushhstack_at_cfa_boundary_np(self);
	if likely(result == 0) {
		(void)src_cfa_offset;
		(void)dst_cfa_offset;
		result = Dee_function_generator_remember_movevalue_hstack2hstackind(self, src_cfa_offset, dst_cfa_offset);
	}
	return result;
}
#endif /* HAVE_Dee_function_generator_ghstack_pushhstack_at_cfa_boundary_np */

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_ghstack_popreg(struct Dee_function_generator *__restrict self,
                                      Dee_host_register_t dst_regno) {
	Dee_cfa_t src_cfa_offset = GET_ADDRESS_OF_NEXT_POP(self);
	int result;
	/* TODO: If "dst_regno" is a known equivalence of "[#src_cfa_offset]", do nothing */
	result = _Dee_function_generator_ghstack_popreg(self, dst_regno);
	if likely(result == 0) {
		result = Dee_function_generator_remember_movevalue_hstackind2reg(self, src_cfa_offset, dst_regno);
		Dee_function_generator_remember_undefined_hstackind(self, src_cfa_offset + HOSTASM_REDZONE_SIZE);
	}
	return result;
}

#undef GET_ADDRESS_OF_NEXT_PUSH
#undef GET_ADDRESS_OF_NEXT_POP

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_reg2hstackind(struct Dee_function_generator *__restrict self,
                                          Dee_host_register_t src_regno, Dee_cfa_t cfa_offset) {
	ptrdiff_t sp_offset;
	int result;
	/* TODO: If "[#cfa_offset]" is a known equivalence of "src_regno", do nothing */

	/* Check if the value *must* be pushed onto the h-stack. */
	if (Dee_memstate_hstack_mustpush(self->fg_state, cfa_offset)) {
		Dee_cfa_t skip = Dee_memstate_hstack_mustpush_skip(self->fg_state, cfa_offset);
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
                                       Dee_cfa_t cfa_offset, Dee_host_register_t dst_regno) {
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
                                          Dee_cfa_t cfa_offset, Dee_host_register_t dst_regno) {
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
		struct Dee_memobj *obj, *cfa_objs = NULL;
		Dee_memstate_foreach(val, self->fg_state) {
			Dee_memval_foreach_obj(obj, val) {
				if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND &&
				    Dee_memobj_hstackind_getcfa(obj) == cfa_offset) {
					STATIC_ASSERT(offsetof(struct Dee_memobj, mo_loc.ml_adr.ma_val.v_cfa) ==
					              offsetof(struct Dee_memobj, mo_loc.ml_adr.ma_val._v_nextobj));
					if (obj->mo_flags & MEMOBJ_F_LINEAR) {
						/* Ooops: not allowed. (Location must not be moved to a register) */
						while (cfa_objs) {
							obj = cfa_objs->mo_loc.ml_adr.ma_val._v_nextobj;
							cfa_objs->mo_loc.ml_adr.ma_val.v_cfa = cfa_offset;
							cfa_objs = obj;
						}
						goto do_use_mov;
					}
					obj->mo_loc.ml_adr.ma_val._v_nextobj = cfa_objs;
					cfa_objs = obj;
				}
			}
			Dee_memval_foreach_obj_end;
		}
		Dee_memstate_foreach_end;
		if (cfa_objs != NULL) {
			struct Dee_memobj *next;
			do {
				next = cfa_objs->mo_loc.ml_adr.ma_val._v_nextobj;
				Dee_memloc_init_hreg(&cfa_objs->mo_loc, dst_regno, Dee_memloc_getoff(&cfa_objs->mo_loc));
				Dee_memstate_incrinuse(self->fg_state, dst_regno);
			} while ((cfa_objs = next) != NULL);
		}
		return Dee_function_generator_ghstack_popreg(self, dst_regno);
	}
do_use_mov:
	return Dee_function_generator_gmov_hstackind2reg_nopop(self, cfa_offset, dst_regno);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_hstackind2reg_nopop(struct Dee_function_generator *__restrict self,
                                                Dee_cfa_t cfa_offset, Dee_host_register_t dst_regno) {
	int result;
	ptrdiff_t sp_offset;
	/* TODO: If "dst_regno" is a known equivalence of "[#cfa_offset]", do nothing */
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
#ifdef _Dee_function_generator_gmov_const2regind_MAYFAIL
	if unlikely(result > 0) {
		Dee_host_register_t valreg;
		Dee_host_register_t not_these[2];
		not_these[0] = dst_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		valreg = Dee_function_generator_gconst_as_reg(self, value, not_these);
		if unlikely(valreg >= HOST_REGISTER_COUNT)
			return -1;
		return Dee_function_generator_gmov_reg2regind(self, valreg, dst_regno, dst_delta);
	}
#endif /* _Dee_function_generator_gmov_const2regind_MAYFAIL */
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_const2regind(self, value, dst_regno, dst_delta);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_const2hstackind(struct Dee_function_generator *__restrict self,
                                            void const *value, Dee_cfa_t cfa_offset) {
	ptrdiff_t sp_offset;
	int result;
	/* TODO: If "[#cfa_offset]" is a known equivalence of "value", do nothing */

	/* Check if the value *must* be pushed onto the h-stack. */
	if (Dee_memstate_hstack_mustpush(self->fg_state, cfa_offset)) {
		ptrdiff_t skip = Dee_memstate_hstack_mustpush_skip(self->fg_state, cfa_offset);
		ASSERT((skip == 0) == !!Dee_memstate_hstack_canpush(self->fg_state, cfa_offset));
		result = Dee_function_generator_ghstack_adjust(self, skip);
		if likely(result == 0)
			result = Dee_function_generator_ghstack_pushconst(self, value);
		return result;
	}
	sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	result = _Dee_function_generator_gmov_const2hstackind(self, value, sp_offset);
#ifdef _Dee_function_generator_gmov_const2hstackind_MAYFAIL
	if unlikely(result > 0) {
		Dee_host_register_t valreg;
		valreg = Dee_function_generator_gconst_as_reg(self, value, NULL);
		if unlikely(valreg >= HOST_REGISTER_COUNT)
			return -1;
		return Dee_function_generator_gmov_reg2hstackind(self, valreg, cfa_offset);
	}
#endif /* _Dee_function_generator_gmov_const2hstackind_MAYFAIL */
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_const2hstackind(self, value, cfa_offset);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmov_const2constind(struct Dee_function_generator *__restrict self,
                                           void const *value, void const **p_value) {
	int result = _Dee_function_generator_gmov_const2constind(self, value, p_value);
#ifdef _Dee_host_section_gmov_const2constind_MAYFAIL
	if unlikely(result > 0) {
		Dee_host_register_t valreg;
		ASSERT(result == 1 || result == 2);
		valreg = Dee_function_generator_gconst_as_reg(self, result == 1 ? value : (void const *)p_value, NULL);
		if unlikely(valreg >= HOST_REGISTER_COUNT)
			return -1;
		return result == 1 ? Dee_function_generator_gmov_reg2constind(self, valreg, p_value)
		                   : Dee_function_generator_gmov_const2regind(self, value, valreg, 0);
	}
#endif /* _Dee_host_section_gmov_const2constind_MAYFAIL */
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
#ifdef _Dee_function_generator_gmov_constind2reg_MAYFAIL
	if unlikely(result > 0) {
		Dee_host_register_t valreg;
		valreg = Dee_function_generator_gconst_as_reg(self, (void const *)p_value, NULL);
		if unlikely(valreg >= HOST_REGISTER_COUNT)
			return -1;
		return Dee_function_generator_gmov_regind2reg(self, valreg, 0, dst_regno);
	}
#endif /* _Dee_function_generator_gmov_constind2reg_MAYFAIL */
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
#ifdef _Dee_function_generator_gmov_reg2constind_MAYFAIL
	if unlikely(result > 0) {
		Dee_host_register_t valreg;
		Dee_host_register_t not_these[2];
		not_these[0] = src_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		valreg = Dee_function_generator_gconst_as_reg(self, (void const *)p_value, not_these);
		if unlikely(valreg >= HOST_REGISTER_COUNT)
			return -1;
		return Dee_function_generator_gmov_reg2regind(self, src_regno, valreg, 0);
	}
#endif /* _Dee_function_generator_gmov_reg2constind_MAYFAIL */
	if likely(result == 0)
		result = Dee_function_generator_remember_movevalue_reg2constind(self, src_regno, p_value);
	return result;
}


/* dst_regno = src1_regno <op> src2_regno; */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gbitop_regreg2reg(struct Dee_function_generator *__restrict self, Dee_bitop_t op,
                                         Dee_host_register_t src1_regno, Dee_host_register_t src2_regno,
                                         Dee_host_register_t dst_regno) {
	int result = _Dee_function_generator_gbitop_regreg2reg(self, op, src1_regno, src2_regno, dst_regno);
	if likely(result == 0)
		Dee_function_generator_remember_undefined_reg(self, dst_regno);
	return result;
}

/* dst_regno = src1_regno <op> *(src2_regno + src2_ind_delta); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gbitop_regregind2reg(struct Dee_function_generator *__restrict self, Dee_bitop_t op,
                                            Dee_host_register_t src1_regno, Dee_host_register_t src2_regno,
                                            ptrdiff_t src2_ind_delta, Dee_host_register_t dst_regno) {
	int result = _Dee_function_generator_gbitop_regregind2reg(self, op, src1_regno, src2_regno, src2_ind_delta, dst_regno);
	if likely(result == 0)
		Dee_function_generator_remember_undefined_reg(self, dst_regno);
	return result;
}

/* dst_regno = src1_regno <op> *(SP ... src2_cfa_offset); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gbitop_reghstackind2reg(struct Dee_function_generator *__restrict self, Dee_bitop_t op,
                                               Dee_host_register_t src1_regno, Dee_cfa_t src2_cfa_offset,
                                               Dee_host_register_t dst_regno) {
	ptrdiff_t src2_sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, src2_cfa_offset);
	int result = _Dee_function_generator_gbitop_reghstackind2reg(self, op, src1_regno, src2_sp_offset, dst_regno);
	if likely(result == 0)
		Dee_function_generator_remember_undefined_reg(self, dst_regno);
	return result;
}


#ifdef _Dee_function_generator_gbitop_regconst2reg_MAYFAIL
/* dst_regno = src_regno <op> value; */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gbitop_regconst2reg(struct Dee_function_generator *__restrict self, Dee_bitop_t op,
                                           Dee_host_register_t src_regno, void const *value,
                                           Dee_host_register_t dst_regno) {
	int result = _Dee_function_generator_gbitop_regconst2reg(self, op, src_regno, value, dst_regno);
	if unlikely(result > 0) {
		Dee_host_register_t not_these[3], tempreg;
		not_these[0] = src_regno;
		not_these[1] = dst_regno;
		not_these[2] = HOST_REGISTER_COUNT;
		tempreg = Dee_function_generator_gconst_as_reg(self, value, not_these);
		if unlikely(tempreg >= HOST_REGISTER_COUNT)
			return -1;
		return Dee_function_generator_gbitop_regreg2reg(self, op, src_regno, tempreg, dst_regno);
	}
	if likely(result == 0)
		Dee_function_generator_remember_undefined_reg(self, dst_regno);
	return result;
}
#endif /* _Dee_function_generator_gbitop_regconst2reg_MAYFAIL */

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gbitop_regxregx2reg(struct Dee_function_generator *__restrict self, Dee_bitop_t op,
                                           Dee_host_register_t src1_regno, ptrdiff_t src_regno1_off,
                                           Dee_host_register_t src2_regno, ptrdiff_t src_regno2_off,
                                           Dee_host_register_t dst_regno) {
	if (src_regno1_off == 0 && src_regno2_off == 0) /* Both sides have value-offset = 0 */
		return Dee_function_generator_gbitop_regreg2reg(self, op, src1_regno, src2_regno, dst_regno);
	if (src_regno1_off == 0 && dst_regno != src1_regno) {
		/* Left side has value-offset == 0, and destination isn't src1. Encode as:
		 * >> dst_regno = src2_regno + src_regno2_off;
		 * >> dst_regno = src1_regno & dst_regno; */
		DO(Dee_function_generator_gmov_regx2reg(self, src2_regno, src_regno2_off, dst_regno));
		return Dee_function_generator_gbitop_regreg2reg(self, op, src1_regno, dst_regno, dst_regno);
	}
	if (src_regno2_off == 0 && dst_regno != src2_regno) {
		/* Right side has value-offset == 0, and destination isn't src2. Encode as:
		 * >> dst_regno = src1_regno + src_regno1_off;
		 * >> dst_regno = dst_regno & src2_regno; */
		DO(Dee_function_generator_gmov_regx2reg(self, src1_regno, src_regno1_off, dst_regno));
		return Dee_function_generator_gbitop_regreg2reg(self, op, dst_regno, src2_regno, dst_regno);
	}

	/* Fallback: inline-adjust */
	if (src_regno1_off != 0) {
		Dee_host_register_t new_src_regno1 = src1_regno;
		if (new_src_regno1 == src2_regno)
			new_src_regno1 = dst_regno;
		DO(Dee_function_generator_gmov_regx2reg(self, src1_regno, src_regno1_off, new_src_regno1));
		Dee_memstate_hregs_adjust_delta(self->fg_state, new_src_regno1, src_regno1_off);
		if (src2_regno == new_src_regno1)
			src_regno2_off += src_regno1_off;
		src1_regno = new_src_regno1;
		/*src_regno1_off = 0;*/
	}
	if (src_regno2_off != 0) {
		if (src1_regno != src2_regno) {
			DO(Dee_function_generator_gmov_regx2reg(self, src2_regno, src_regno2_off, src2_regno));
			/*src_regno2_off = 0;*/
		} else if (src1_regno != dst_regno) {
			DO(Dee_function_generator_gmov_regx2reg(self, src2_regno, src_regno2_off, dst_regno));
			src2_regno = dst_regno;
			/*src_regno2_off = 0;*/
		} else {
			Dee_host_register_t tempreg;
			Dee_host_register_t not_these[2];
			not_these[0] = src2_regno;
			not_these[1] = HOST_REGISTER_COUNT;
			tempreg = Dee_function_generator_gallocreg(self, not_these);
			if unlikely(tempreg >= HOST_REGISTER_COUNT)
				goto err;
			DO(Dee_function_generator_gmov_regx2reg(self, src2_regno, src_regno2_off, tempreg));
			src2_regno = tempreg;
			/*src_regno2_off = 0;*/
		}
	}
	return Dee_function_generator_gbitop_regreg2reg(self, op, src1_regno, src2_regno, dst_regno);
err:
	return -1;
}

/* dst_regno = src_loc1 <op> src_loc2; */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gbitop_locloc2reg(struct Dee_function_generator *__restrict self, Dee_bitop_t op,
                                         struct Dee_memloc const *src_loc1, struct Dee_memloc const *src_loc2,
                                         Dee_host_register_t dst_regno) {
	struct Dee_memloc loc1_asreg, loc2_asreg;
	Dee_host_register_t not_these[2];
	if (Dee_memloc_gettyp(src_loc2) == MEMADR_TYPE_HREG ||
	    Dee_memloc_gettyp(src_loc1) == MEMADR_TYPE_CONST) {
		/* Always want the constant to appear on the *right*
		 * side, and a register to appear on the left side. */
		struct Dee_memloc const *temp;
		temp = src_loc2;
		src_loc2 = src_loc1;
		src_loc1 = temp;
	}

	if (Dee_memloc_sameloc(src_loc1, src_loc2)) {
		if (op == BITOP_XOR)
			return Dee_function_generator_gmov_const2reg(self, (void const *)0, dst_regno);
		return Dee_function_generator_gmov_loc2reg(self, src_loc1, dst_regno);
	}
	if unlikely(Dee_memloc_gettyp(src_loc1) == MEMADR_TYPE_UNDEFINED)
		return 0;
	switch (Dee_memloc_gettyp(src_loc2)) {

	case MEMADR_TYPE_HSTACKIND: {
		Dee_cfa_t src_loc2_cfa = Dee_memloc_hstackind_getcfa(src_loc2);
		if (Dee_memloc_hstackind_getvaloff(src_loc2) != 0)
			goto src_loc2_fallback;
		switch (Dee_memloc_gettyp(src_loc1)) {
		default:
			DO(Dee_function_generator_gmov_loc2reg(self, src_loc1, dst_regno));
			Dee_memloc_init_hreg(&loc1_asreg, dst_regno, 0);
			src_loc1 = &loc1_asreg;
			ATTR_FALLTHROUGH
		case MEMADR_TYPE_HREG:
			if (Dee_memloc_hreg_getvaloff(src_loc1) != 0) {
				if unlikely(Dee_function_generator_gmov_regx2reg(self,
				                                                 Dee_memloc_hreg_getreg(src_loc1),
				                                                 Dee_memloc_hreg_getvaloff(src_loc1),
				                                                 dst_regno))
					goto err;
				return Dee_function_generator_gbitop_reghstackind2reg(self, op, dst_regno, src_loc2_cfa, dst_regno);
			}
			return Dee_function_generator_gbitop_reghstackind2reg(self, op, Dee_memloc_hreg_getreg(src_loc1),
			                                                      src_loc2_cfa, dst_regno);
		}
	}	break;

	case MEMADR_TYPE_HREGIND: {
		Dee_host_register_t src_loc2_regno = Dee_memloc_hregind_getreg(src_loc2);
		ptrdiff_t src_loc2_indoff = Dee_memloc_hregind_getindoff(src_loc2);
		if (Dee_memloc_hregind_getvaloff(src_loc2) != 0)
			goto src_loc2_fallback;
		switch (Dee_memloc_gettyp(src_loc1)) {
		default:
			DO(Dee_function_generator_gmov_loc2reg(self, src_loc1, dst_regno));
			Dee_memloc_init_hreg(&loc1_asreg, dst_regno, 0);
			src_loc1 = &loc1_asreg;
			ATTR_FALLTHROUGH
		case MEMADR_TYPE_HREG:
			if (Dee_memloc_hreg_getvaloff(src_loc1) != 0) {
				if unlikely(Dee_function_generator_gmov_regx2reg(self,
				                                                 Dee_memloc_hreg_getreg(src_loc1),
				                                                 Dee_memloc_hreg_getvaloff(src_loc1),
				                                                 dst_regno))
					goto err;
				return Dee_function_generator_gbitop_regregind2reg(self, op, dst_regno, src_loc2_regno,
				                                                   src_loc2_indoff, dst_regno);
			}
			return Dee_function_generator_gbitop_regregind2reg(self, op, Dee_memloc_hreg_getreg(src_loc1),
			                                                   src_loc2_regno, src_loc2_indoff, dst_regno);
		}
	}	break;

	default:
src_loc2_fallback:
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (Dee_memloc_hasreg(src_loc1))
			not_these[0] = Dee_memloc_getreg(src_loc1);
		DO(Dee_function_generator_gasreg(self, src_loc2, &loc2_asreg, not_these));
		src_loc2 = &loc2_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		if (Dee_memloc_gettyp(src_loc1) != MEMADR_TYPE_HREG) {
			not_these[0] = Dee_memloc_getreg(src_loc2);
			not_these[1] = HOST_REGISTER_COUNT;
			DO(Dee_function_generator_gasreg(self, src_loc1, &loc1_asreg, not_these));
			src_loc1 = &loc1_asreg;
		}
		ASSERT(Dee_memloc_gettyp(src_loc1) == MEMADR_TYPE_HREG);
		ASSERT(Dee_memloc_gettyp(src_loc2) == MEMADR_TYPE_HREG);
		return Dee_function_generator_gbitop_regxregx2reg(self, op,
		                                                  Dee_memloc_hreg_getreg(src_loc1),
		                                                  Dee_memloc_hreg_getvaloff(src_loc1),
		                                                  Dee_memloc_hreg_getreg(src_loc2),
		                                                  Dee_memloc_hreg_getvaloff(src_loc2),
		                                                  dst_regno);

	case MEMADR_TYPE_CONST: {
		void const *src_loc2_value = Dee_memloc_const_getaddr(src_loc2);
		switch (Dee_memloc_gettyp(src_loc1)) {
		default:
			DO(Dee_function_generator_gmov_loc2reg(self, src_loc1, dst_regno));
			Dee_memloc_init_hreg(&loc1_asreg, dst_regno, 0);
			src_loc1 = &loc1_asreg;
			ATTR_FALLTHROUGH
		case MEMADR_TYPE_HREG:
			if (Dee_memloc_hreg_getvaloff(src_loc1) != 0) {
				if unlikely(Dee_function_generator_gmov_regx2reg(self,
				                                                 Dee_memloc_hreg_getreg(src_loc1),
				                                                 Dee_memloc_hreg_getvaloff(src_loc1),
				                                                 dst_regno))
					goto err;
				return Dee_function_generator_gbitop_regconst2reg(self, op, dst_regno, src_loc2_value, dst_regno);
			}
			return Dee_function_generator_gbitop_regconst2reg(self, op, Dee_memloc_hreg_getreg(src_loc1),
			                                                  src_loc2_value, dst_regno);
		case MEMADR_TYPE_CONST: {
			void const *src_loc1_value = Dee_memloc_const_getaddr(src_loc1);
			void const *dst_value = (void const *)Dee_bitop_forconst(op, (uintptr_t)src_loc1_value, (uintptr_t)src_loc2_value);
			return Dee_function_generator_gmov_const2reg(self, dst_value, dst_regno);
		}	break;

		}
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}



/* dst_regno = src1_regno <op> src2_regno; */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjarith_regreg2reg(struct Dee_function_generator *__restrict self, Dee_bitop_t op,
                                          Dee_host_register_t src1_regno, Dee_host_register_t src2_regno,
                                          Dee_host_register_t dst_regno,
                                          struct Dee_host_symbol *dst_o, struct Dee_host_symbol *dst_no) {
	int result = _Dee_function_generator_gjarith_regreg2reg(self, op, src1_regno, src2_regno, dst_regno, dst_o, dst_no);
	if likely(result == 0)
		Dee_function_generator_remember_undefined_reg(self, dst_regno);
	return result;
}

/* dst_regno = src1_regno <op> *(src2_regno + src2_ind_delta); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjarith_regregind2reg(struct Dee_function_generator *__restrict self, Dee_bitop_t op,
                                             Dee_host_register_t src1_regno, Dee_host_register_t src2_regno,
                                             ptrdiff_t src2_ind_delta, Dee_host_register_t dst_regno,
                                             struct Dee_host_symbol *dst_o, struct Dee_host_symbol *dst_no) {
	int result = _Dee_function_generator_gjarith_regregind2reg(self, op, src1_regno, src2_regno, src2_ind_delta, dst_regno, dst_o, dst_no);
	if likely(result == 0)
		Dee_function_generator_remember_undefined_reg(self, dst_regno);
	return result;
}

/* dst_regno = src1_regno <op> *(SP ... src2_cfa_offset); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjarith_reghstackind2reg(struct Dee_function_generator *__restrict self, Dee_bitop_t op,
                                                Dee_host_register_t src1_regno, Dee_cfa_t src2_cfa_offset,
                                                Dee_host_register_t dst_regno,
                                                struct Dee_host_symbol *dst_o, struct Dee_host_symbol *dst_no) {
	ptrdiff_t src2_sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, src2_cfa_offset);
	int result = _Dee_function_generator_gjarith_reghstackind2reg(self, op, src1_regno, src2_sp_offset, dst_regno, dst_o, dst_no);
	if likely(result == 0)
		Dee_function_generator_remember_undefined_reg(self, dst_regno);
	return result;
}


#ifdef _Dee_function_generator_gjarith_regconst2reg_MAYFAIL
/* dst_regno = src_regno <op> value; */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjarith_regconst2reg(struct Dee_function_generator *__restrict self, Dee_bitop_t op,
                                            Dee_host_register_t src_regno, void const *value,
                                            Dee_host_register_t dst_regno,
                                            struct Dee_host_symbol *dst_o, struct Dee_host_symbol *dst_no) {
	int result = _Dee_function_generator_gjarith_regconst2reg(self, op, src_regno, value, dst_regno, dst_o, dst_no);
	if unlikely(result > 0) {
		Dee_host_register_t not_these[3], tempreg;
		not_these[0] = src_regno;
		not_these[1] = dst_regno;
		not_these[2] = HOST_REGISTER_COUNT;
		tempreg = Dee_function_generator_gconst_as_reg(self, value, not_these);
		if unlikely(tempreg >= HOST_REGISTER_COUNT)
			return -1;
		return Dee_function_generator_gjarith_regreg2reg(self, op, src_regno, tempreg, dst_regno, dst_o, dst_no);
	}
	if likely(result == 0)
		Dee_function_generator_remember_undefined_reg(self, dst_regno);
	return result;
}
#endif /* _Dee_function_generator_gjarith_regconst2reg_MAYFAIL */

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjarith_regxregx2reg(struct Dee_function_generator *__restrict self, Dee_bitop_t op,
                                            Dee_host_register_t src1_regno, ptrdiff_t src_regno1_off,
                                            Dee_host_register_t src2_regno, ptrdiff_t src_regno2_off,
                                            Dee_host_register_t dst_regno,
                                            struct Dee_host_symbol *dst_o, struct Dee_host_symbol *dst_no) {
	if (src_regno1_off == 0 && src_regno2_off == 0) /* Both sides have value-offset = 0 */
		return Dee_function_generator_gjarith_regreg2reg(self, op, src1_regno, src2_regno, dst_regno, dst_o, dst_no);
	if (src1_regno == src2_regno && ARITHOP_MAYMOVEOFF(op)) {
		src_regno2_off -= src_regno1_off;
		src_regno1_off = 0;
	}
	if (src_regno1_off == 0 && dst_regno != src1_regno) {
		/* Left side has value-offset == 0, and destination isn't src1. Encode as:
		 * >> dst_regno = src2_regno + src_regno2_off;
		 * >> dst_regno = src1_regno & dst_regno; */
		DO(Dee_function_generator_gmov_regx2reg(self, src2_regno, src_regno2_off, dst_regno));
		return Dee_function_generator_gjarith_regreg2reg(self, op, src1_regno, dst_regno, dst_regno, dst_o, dst_no);
	}
	if (src_regno2_off == 0 && dst_regno != src2_regno) {
		/* Right side has value-offset == 0, and destination isn't src2. Encode as:
		 * >> dst_regno = src1_regno + src_regno1_off;
		 * >> dst_regno = dst_regno & src2_regno; */
		DO(Dee_function_generator_gmov_regx2reg(self, src1_regno, src_regno1_off, dst_regno));
		return Dee_function_generator_gjarith_regreg2reg(self, op, dst_regno, src2_regno, dst_regno, dst_o, dst_no);
	}

	/* Fallback: inline-adjust */
	if (src_regno1_off != 0) {
		Dee_host_register_t new_src_regno1 = src1_regno;
		if (new_src_regno1 == src2_regno)
			new_src_regno1 = dst_regno;
		DO(Dee_function_generator_gmov_regx2reg(self, src1_regno, src_regno1_off, new_src_regno1));
		Dee_memstate_hregs_adjust_delta(self->fg_state, new_src_regno1, src_regno1_off);
		if (src2_regno == new_src_regno1)
			src_regno2_off += src_regno1_off;
		src1_regno = new_src_regno1;
		/*src_regno1_off = 0;*/
	}
	if (src_regno2_off != 0) {
		if (src1_regno != src2_regno) {
			DO(Dee_function_generator_gmov_regx2reg(self, src2_regno, src_regno2_off, src2_regno));
			/*src_regno2_off = 0;*/
		} else if (src1_regno != dst_regno) {
			DO(Dee_function_generator_gmov_regx2reg(self, src2_regno, src_regno2_off, dst_regno));
			src2_regno = dst_regno;
			/*src_regno2_off = 0;*/
		} else {
			Dee_host_register_t tempreg;
			Dee_host_register_t not_these[2];
			not_these[0] = src2_regno;
			not_these[1] = HOST_REGISTER_COUNT;
			tempreg = Dee_function_generator_gallocreg(self, not_these);
			if unlikely(tempreg >= HOST_REGISTER_COUNT)
				goto err;
			DO(Dee_function_generator_gmov_regx2reg(self, src2_regno, src_regno2_off, tempreg));
			src2_regno = tempreg;
			/*src_regno2_off = 0;*/
		}
	}
	return Dee_function_generator_gjarith_regreg2reg(self, op, src1_regno, src2_regno, dst_regno, dst_o, dst_no);
err:
	return -1;
}

/* dst_regno = src_loc1 <op> src_loc2; */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjarith_locloc2reg(struct Dee_function_generator *__restrict self, Dee_bitop_t op,
                                          struct Dee_memloc const *src_loc1, struct Dee_memloc const *src_loc2,
                                          Dee_host_register_t dst_regno,
                                          struct Dee_host_symbol *dst_o, struct Dee_host_symbol *dst_no) {
	struct Dee_memloc loc1_asnormal, loc2_asnormal;
	struct Dee_memloc loc1_asreg, loc2_asreg;
	Dee_host_register_t not_these[2];
	if (Dee_memloc_gettyp(src_loc2) == MEMADR_TYPE_HREG ||
	    Dee_memloc_gettyp(src_loc1) == MEMADR_TYPE_CONST) {
		if (ARITHOP_MAYREORDER(op)) {
			/* Always want the constant to appear on the *right*
			 * side, and a register to appear on the left side. */
			struct Dee_memloc const *temp;
			temp = src_loc2;
			src_loc2 = src_loc1;
			src_loc1 = temp;
		} else if (Dee_memloc_gettyp(src_loc1) == MEMADR_TYPE_CONST) {
			switch (op) {

			case ARITHOP_USUB:
			case ARITHOP_SSUB: {
				intptr_t negval = -(intptr_t)(uintptr_t)Dee_memloc_const_getaddr(src_loc1);
				src_loc1 = src_loc2;
				src_loc2 = &loc2_asnormal;
				Dee_memloc_init_const(&loc2_asnormal, (void *)(uintptr_t)negval);
				op = op == ARITHOP_USUB ? ARITHOP_UADD : ARITHOP_SADD;
			}	break;

			default: break;
			}
		}
	}

	/* If allowed by the operation, move all value offsets into "src_loc2" */
	if (ARITHOP_MAYMOVEOFF(op)) {
		ptrdiff_t off1 = Dee_memloc_getoff(src_loc1);
		if (off1 == 0) {
			/* Nothing do to: lhs already has a value-offset of 0 */
		} else {
			if (src_loc2 != &loc2_asnormal)
				loc2_asnormal = *src_loc2;
			src_loc2 = &loc2_asnormal;
			Dee_memloc_adjoff(&loc2_asnormal, -off1);
			loc1_asnormal = *src_loc1;
			Dee_memloc_setoff(&loc1_asnormal, 0);
			src_loc1 = &loc1_asnormal;
		}
	}

	if (Dee_memloc_sameloc(src_loc1, src_loc2)) {
		switch (op) {
		case ARITHOP_USUB:
		case ARITHOP_SSUB:
			return Dee_function_generator_gmov_const2reg(self, (void const *)0, dst_regno);
		default: break;
		}
		return Dee_function_generator_gmov_loc2reg(self, src_loc1, dst_regno);
	}
	if unlikely(Dee_memloc_gettyp(src_loc1) == MEMADR_TYPE_UNDEFINED)
		return (dst_o && dst_no) ? Dee_function_generator_gjmp(self, dst_no) : 0;
	switch (Dee_memloc_gettyp(src_loc2)) {

	case MEMADR_TYPE_HSTACKIND: {
		Dee_cfa_t src_loc2_cfa = Dee_memloc_hstackind_getcfa(src_loc2);
		if (Dee_memloc_hstackind_getvaloff(src_loc2) != 0)
			goto src_loc2_fallback;
		switch (Dee_memloc_gettyp(src_loc1)) {
		default:
			DO(Dee_function_generator_gmov_loc2reg(self, src_loc1, dst_regno));
			Dee_memloc_init_hreg(&loc1_asreg, dst_regno, 0);
			src_loc1 = &loc1_asreg;
			ATTR_FALLTHROUGH
		case MEMADR_TYPE_HREG:
			if (Dee_memloc_hreg_getvaloff(src_loc1) != 0) {
				if unlikely(Dee_function_generator_gmov_regx2reg(self,
				                                                 Dee_memloc_hreg_getreg(src_loc1),
				                                                 Dee_memloc_hreg_getvaloff(src_loc1),
				                                                 dst_regno))
					goto err;
				return Dee_function_generator_gjarith_reghstackind2reg(self, op, dst_regno, src_loc2_cfa,
				                                                       dst_regno, dst_o, dst_no);
			}
			return Dee_function_generator_gjarith_reghstackind2reg(self, op, Dee_memloc_hreg_getreg(src_loc1),
			                                                       src_loc2_cfa, dst_regno, dst_o, dst_no);
		}
	}	break;

	case MEMADR_TYPE_HREGIND: {
		Dee_host_register_t src_loc2_regno = Dee_memloc_hregind_getreg(src_loc2);
		ptrdiff_t src_loc2_indoff = Dee_memloc_hregind_getindoff(src_loc2);
		if (Dee_memloc_hregind_getvaloff(src_loc2) != 0)
			goto src_loc2_fallback;
		switch (Dee_memloc_gettyp(src_loc1)) {
		default:
			DO(Dee_function_generator_gmov_loc2reg(self, src_loc1, dst_regno));
			Dee_memloc_init_hreg(&loc1_asreg, dst_regno, 0);
			src_loc1 = &loc1_asreg;
			ATTR_FALLTHROUGH
		case MEMADR_TYPE_HREG:
			if (Dee_memloc_hreg_getvaloff(src_loc1) != 0) {
				if unlikely(Dee_function_generator_gmov_regx2reg(self,
				                                                 Dee_memloc_hreg_getreg(src_loc1),
				                                                 Dee_memloc_hreg_getvaloff(src_loc1),
				                                                 dst_regno))
					goto err;
				return Dee_function_generator_gjarith_regregind2reg(self, op, dst_regno, src_loc2_regno,
				                                                    src_loc2_indoff, dst_regno, dst_o, dst_no);
			}
			return Dee_function_generator_gjarith_regregind2reg(self, op, Dee_memloc_hreg_getreg(src_loc1),
			                                                    src_loc2_regno, src_loc2_indoff, dst_regno, dst_o, dst_no);
		}
	}	break;

	default:
src_loc2_fallback:
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (Dee_memloc_hasreg(src_loc1))
			not_these[0] = Dee_memloc_getreg(src_loc1);
		DO(Dee_function_generator_gasreg(self, src_loc2, &loc2_asreg, not_these));
		src_loc2 = &loc2_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		if (Dee_memloc_gettyp(src_loc1) != MEMADR_TYPE_HREG) {
			not_these[0] = Dee_memloc_getreg(src_loc2);
			not_these[1] = HOST_REGISTER_COUNT;
			DO(Dee_function_generator_gasreg(self, src_loc1, &loc1_asreg, not_these));
			src_loc1 = &loc1_asreg;
		}
		ASSERT(Dee_memloc_gettyp(src_loc1) == MEMADR_TYPE_HREG);
		ASSERT(Dee_memloc_gettyp(src_loc2) == MEMADR_TYPE_HREG);
		return Dee_function_generator_gjarith_regxregx2reg(self, op,
		                                                  Dee_memloc_hreg_getreg(src_loc1),
		                                                  Dee_memloc_hreg_getvaloff(src_loc1),
		                                                  Dee_memloc_hreg_getreg(src_loc2),
		                                                  Dee_memloc_hreg_getvaloff(src_loc2),
		                                                  dst_regno, dst_o, dst_no);

	case MEMADR_TYPE_CONST: {
		uintptr_t src_loc2_value = (uintptr_t)Dee_memloc_const_getaddr(src_loc2);
		switch (Dee_memloc_gettyp(src_loc1)) {
		default:
			DO(Dee_function_generator_gmov_loc2reg(self, src_loc1, dst_regno));
			Dee_memloc_init_hreg(&loc1_asreg, dst_regno, 0);
			src_loc1 = &loc1_asreg;
			ATTR_FALLTHROUGH
		case MEMADR_TYPE_HREG:
			if (Dee_memloc_hreg_getvaloff(src_loc1) != 0) {
				if unlikely(Dee_function_generator_gmov_regx2reg(self,
				                                                 Dee_memloc_hreg_getreg(src_loc1),
				                                                 Dee_memloc_hreg_getvaloff(src_loc1),
				                                                 dst_regno))
					goto err;
				return Dee_function_generator_gjarith_regconst2reg(self, op, dst_regno, (void const *)src_loc2_value,
				                                                   dst_regno, dst_o, dst_no);
			}
			return Dee_function_generator_gjarith_regconst2reg(self, op, Dee_memloc_hreg_getreg(src_loc1),
			                                                   (void const *)src_loc2_value, dst_regno, dst_o, dst_no);
		case MEMADR_TYPE_CONST: {
			uintptr_t dst_value;
			uintptr_t src_loc1_value = (uintptr_t)Dee_memloc_const_getaddr(src_loc1);
			bool is_overflow = Dee_arithop_forconst(op, src_loc1_value, src_loc2_value, &dst_value);
			struct Dee_host_symbol *dst = is_overflow ? dst_o : dst_no;
			DO(Dee_function_generator_gmov_const2reg(self, (void const *)dst_value, dst_regno));
			return dst ? Dee_function_generator_gjmp(self, dst) : 0;
		}	break;

		}
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}





INTERN WUNUSED NONNULL((1, 3)) int DCALL
Dee_function_generator_gmov_const2loc(struct Dee_function_generator *__restrict self,
                                      void const *value, struct Dee_memloc const *__restrict dst_loc) {
	/* TODO: If "dst_loc" is a known equivalence of "value", do nothing */
	switch (Dee_memloc_gettyp(dst_loc)) {

	case MEMADR_TYPE_HSTACKIND: {
		Dee_cfa_t cfa_offset = Dee_memloc_hstackind_getcfa(dst_loc);
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
                                       Dee_cfa_t cfa_offset,
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
		Dee_cfa_t cfa_offset = Dee_memloc_hstackind_getcfa(dst_loc);
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
		Dee_cfa_t cfa_offset = Dee_memloc_hstackind_getcfa(src_loc);
		ptrdiff_t delta_delta = Dee_memloc_hstackind_getvaloff(src_loc) - dst_delta;
		result = Dee_function_generator_gmov_hstackind2reg(self, cfa_offset, dst_regno);
		if likely(result == 0 && delta_delta != 0)
			result = Dee_function_generator_gmov_regx2reg(self, dst_regno, delta_delta, dst_regno);
	}	break;

	case MEMADR_TYPE_HSTACK: {
		Dee_cfa_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(Dee_memloc_hstack_getcfa(src_loc), -dst_delta);
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
		Dee_cfa_t cfa_offset = Dee_memloc_hstackind_getcfa(src_loc);
		result = Dee_function_generator_gmov_hstackind2reg(self, cfa_offset, dst_regno);
		*p_dst_delta = Dee_memloc_hstackind_getvaloff(src_loc);
	}	break;

	case MEMADR_TYPE_HSTACK: {
		Dee_cfa_t cfa_offset  = Dee_memloc_hstack_getcfa(src_loc);
		ptrdiff_t sp_offset   = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		Dee_cfa_t cfa0_offset = Dee_memstate_hstack_sp2cfa(self->fg_state, 0);
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
		Dee_cfa_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(Dee_memloc_hstack_getcfa(src_loc), src_delta);
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
		Dee_cfa_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(Dee_memloc_hstack_getcfa(dst_loc), dst_delta);
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
                                        struct Dee_memloc const *__restrict src_loc,
                                        ptrdiff_t dst_delta) {
	struct Dee_memloc src_asreg0;
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
			src_asreg0 = *src_loc;
			Dee_memloc_hreg_setvaloff(&src_asreg0, writeback_delta);
			src_loc = &src_asreg0;
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
		Dee_cfa_t cfa_offset = Dee_memloc_hstackind_getcfa(src_loc);
		ptrdiff_t final_delta = Dee_memloc_hstackind_getvaloff(src_loc) - dst_delta;
		if (final_delta != 0)
			goto fallback;
		return Dee_function_generator_ghstack_pushhstackind(self, cfa_offset);
	}	break;

#ifdef HAVE_Dee_function_generator_ghstack_pushhstack_at_cfa_boundary_np
	case MEMADR_TYPE_HSTACK: {
		Dee_cfa_t cfa_offset = Dee_memloc_hstack_getcfa(src_loc);
		if (cfa_offset == self->fg_state->ms_host_cfa_offset) /* Special case: push current CFA offset */
			return Dee_function_generator_ghstack_pushhstack_at_cfa_boundary_np(self);
		goto fallback;
	}	break;
#endif /* HAVE_Dee_function_generator_ghstack_pushhstack_at_cfa_boundary_np */

	case MEMADR_TYPE_CONST: {
		void const *value = Dee_memloc_const_getaddr(src_loc) - dst_delta;
#ifndef Dee_function_generator_ghstack_pushconst_MAYFAIL
		return Dee_function_generator_ghstack_pushconst(self, value);
#else /* !Dee_function_generator_ghstack_pushconst_MAYFAIL */
		int result = Dee_function_generator_ghstack_pushconst(self, value);
		if unlikely(result > 0)
			goto fallback;
		return result;
#endif /* Dee_function_generator_ghstack_pushconst_MAYFAIL */
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
                                           struct Dee_memloc const *__restrict src_loc,
                                           Dee_cfa_t dst_cfa_offset, ptrdiff_t dst_delta) {
	struct Dee_memloc src_asreg0;
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
			src_asreg0 = *src_loc;
			Dee_memloc_hreg_setvaloff(&src_asreg0, writeback_delta);
			src_loc = &src_asreg0;
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
                                         struct Dee_memloc const *__restrict src_loc,
                                         void const **p_value, ptrdiff_t dst_delta) {
	struct Dee_memloc src_asreg0;
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
			src_asreg0 = *src_loc;
			Dee_memloc_hreg_setvaloff(&src_asreg0, writeback_delta);
			src_loc = &src_asreg0;
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
		Dee_cfa_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(Dee_memloc_hstack_getcfa(dst_loc), dst_delta);
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
                            /*inherit_ref*/ struct Dee_memloc const *__restrict loc) {
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


INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmorph_regx2reg01(struct Dee_function_generator *__restrict self,
                                         Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                         unsigned int cmp, Dee_host_register_t dst_regno) {
	int result = _Dee_function_generator_gmorph_regx2reg01(self, src_regno, src_delta, cmp, dst_regno);
	if likely(result == 0)
		Dee_function_generator_remember_undefined_reg(self, dst_regno);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmorph_regind2reg01(struct Dee_function_generator *__restrict self,
                                           Dee_host_register_t src_regno, ptrdiff_t ind_delta, ptrdiff_t val_delta,
                                           unsigned int cmp, Dee_host_register_t dst_regno) {
	int result = _Dee_function_generator_gmorph_regind2reg01(self, src_regno, ind_delta, val_delta, cmp, dst_regno);
#ifdef _Dee_function_generator_gmorph_regind2reg01_MAYFAIL
	if unlikely(result > 0) {
		Dee_host_register_t val_delta_reg;
		Dee_host_register_t not_these[3];
		not_these[0] = src_regno;
		not_these[1] = dst_regno;
		not_these[2] = HOST_REGISTER_RETURN;
		val_delta_reg = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(val_delta_reg >= HOST_REGISTER_RETURN)
			return -1;
		result = Dee_function_generator_gmov_const2reg(self, (void const *)(uintptr_t)(-val_delta), val_delta_reg);
		if likely(result == 0) {
			result = Dee_function_generator_gmorph_regindCreg2reg01(self, src_regno, ind_delta, 0,
			                                                        cmp, val_delta_reg, dst_regno);
		}
		return result;
	}
#endif /* _Dee_function_generator_gmorph_regind2reg01_MAYFAIL */
	if likely(result == 0)
		Dee_function_generator_remember_undefined_reg(self, dst_regno);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmorph_hstackind2reg01(struct Dee_function_generator *__restrict self,
                                              Dee_cfa_t cfa_offset, ptrdiff_t val_delta,
                                              unsigned int cmp, Dee_host_register_t dst_regno) {
	ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	int result = _Dee_function_generator_gmorph_hstackind2reg01(self, sp_offset, val_delta, cmp, dst_regno);
#ifdef _Dee_function_generator_gmorph_hstackind2reg01_MAYFAIL
	if unlikely(result > 0) {
		Dee_host_register_t val_delta_reg;
		Dee_host_register_t not_these[2];
		not_these[0] = dst_regno;
		not_these[1] = HOST_REGISTER_RETURN;
		val_delta_reg = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(val_delta_reg >= HOST_REGISTER_RETURN)
			return -1;
		result = Dee_function_generator_gmov_const2reg(self, (void const *)(uintptr_t)(-val_delta), val_delta_reg);
		if likely(result == 0) {
			result = Dee_function_generator_gmorph_hstackindCreg2reg01(self, cfa_offset, 0,
			                                                           cmp, val_delta_reg, dst_regno);
		}
		return result;
	}
#endif /* _Dee_function_generator_gmorph_hstackind2reg01_MAYFAIL */
	if likely(result == 0)
		Dee_function_generator_remember_undefined_reg(self, dst_regno);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmorph_regxCreg2reg01(struct Dee_function_generator *__restrict self,
                                             Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                             unsigned int cmp, Dee_host_register_t rhs_regno,
                                             Dee_host_register_t dst_regno) {
	int result = _Dee_function_generator_gmorph_regxCreg2reg01(self, src_regno, src_delta, cmp, rhs_regno, dst_regno);
	if likely(result == 0)
		Dee_function_generator_remember_undefined_reg(self, dst_regno);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmorph_regindCreg2reg01(struct Dee_function_generator *__restrict self,
                                               Dee_host_register_t src_regno, ptrdiff_t ind_delta, ptrdiff_t val_delta,
                                               unsigned int cmp, Dee_host_register_t rhs_regno,
                                               Dee_host_register_t dst_regno) {
	int result = _Dee_function_generator_gmorph_regindCreg2reg01(self, src_regno, ind_delta, val_delta, cmp, rhs_regno, dst_regno);
	if likely(result == 0)
		Dee_function_generator_remember_undefined_reg(self, dst_regno);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gmorph_hstackindCreg2reg01(struct Dee_function_generator *__restrict self,
                                                  Dee_cfa_t cfa_offset, ptrdiff_t val_delta,
                                                  unsigned int cmp, Dee_host_register_t rhs_regno,
                                                  Dee_host_register_t dst_regno) {
	ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	int result = _Dee_function_generator_gmorph_hstackindCreg2reg01(self, sp_offset, val_delta, cmp, rhs_regno, dst_regno);
	if likely(result == 0)
		Dee_function_generator_remember_undefined_reg(self, dst_regno);
	return result;
}

/* dst_regno = (src_loc + src_delta) <CMP> 0 ? 1 : 0; */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gmorph_loc2reg01(struct Dee_function_generator *__restrict self,
                                        struct Dee_memloc const *src_loc, ptrdiff_t src_delta,
                                        unsigned int cmp, Dee_host_register_t dst_regno) {
	struct Dee_memloc src_asreg;
	switch (Dee_memloc_gettyp(src_loc)) {
	default: {
		Dee_host_register_t not_these[2];
		not_these[0] = dst_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		if unlikely(Dee_function_generator_gasreg(self, src_loc, &src_asreg, not_these))
			goto err;
		src_loc = &src_asreg;
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
                                            struct Dee_memloc const *src_loc, ptrdiff_t src_delta,
                                            unsigned int cmp, Dee_host_register_t rhs_regno,
                                            Dee_host_register_t dst_regno) {
	struct Dee_memloc src_asreg;
	switch (Dee_memloc_gettyp(src_loc)) {
	default: {
		Dee_host_register_t not_these[3];
		not_these[0] = rhs_regno;
		not_these[1] = dst_regno;
		not_these[2] = HOST_REGISTER_COUNT;
		if unlikely(Dee_function_generator_gasreg(self, src_loc, &src_asreg, not_these))
			goto err;
		src_loc = &src_asreg;
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
                                            struct Dee_memloc const *src_loc, ptrdiff_t src_delta,
                                            unsigned int cmp, struct Dee_memloc const *rhs_loc,
                                            Dee_host_register_t dst_regno) {
	struct Dee_memloc rhs_asreg;
	if (Dee_memloc_gettyp(rhs_loc) != MEMADR_TYPE_HREG) {
		if (Dee_memloc_gettyp(src_loc) == MEMADR_TYPE_HREG) {
			/* Flip operands so the register appears in "rhs_loc" */
			struct Dee_memloc const *temp;
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
			if unlikely(Dee_function_generator_gasreg(self, rhs_loc, &rhs_asreg, not_these))
				goto err;
			rhs_loc = &rhs_asreg;
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
                                           struct Dee_memloc const *src_loc, ptrdiff_t src_delta,
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
	struct Dee_host_section *sect = Dee_function_generator_gettext(self);
	if unlikely(_Dee_host_section_gumul_regconst2reg(sect, src_regno,
	                                                 sizeof(DeeBoolObject), dst_regno))
		goto err;
	src_delta *= sizeof(DeeBoolObject);
	src_delta += (ptrdiff_t)(uintptr_t)Dee_FalseTrue;
	*p_dst_delta = 0;
	return _Dee_host_section_gmov_regx2reg(sect, dst_regno, src_delta, dst_regno);
err:
	return -1;
}
#endif /* !HAVE__Dee_host_section_gmorph_reg012regbool */

/* dst_regno = &Dee_FalseTrue[src_loc + src_delta] - *p_dst_delta; */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
Dee_function_generator_gmorph_loc012regbooly(struct Dee_function_generator *__restrict self,
                                             struct Dee_memloc const *src_loc, ptrdiff_t src_delta,
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
PRIVATE WUNUSED NONNULL((1)) Dee_cfa_t DCALL
try_restore_xloc_arg_cfa_offset(struct Dee_function_generator *__restrict self,
                                Dee_host_register_t regno) {
#define MEMSTATE_XLOCAL_A_MIN MEMSTATE_XLOCAL_A_THIS
#define MEMSTATE_XLOCAL_A_MAX MEMSTATE_XLOCAL_A_KW
	Dee_lid_t i, xloc_base = self->fg_assembler->fa_localc;
	struct Dee_memstate *state = self->fg_state;
	for (i = MEMSTATE_XLOCAL_A_MIN; i <= MEMSTATE_XLOCAL_A_MAX; ++i) {
		struct Dee_memval *xval = &state->ms_localv[xloc_base + i];
		if (Dee_memval_isdirect(xval) &&
		    Dee_memval_direct_gettyp(xval) == MEMADR_TYPE_HREG &&
		    Dee_memval_direct_hreg_getreg(xval) == regno) {
			Dee_cfa_t cfa_offset;
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
			cfa_offset = (Dee_cfa_t)(-(ptrdiff_t)((true_argi + 1) * HOST_SIZEOF_POINTER));
			return cfa_offset;
		}
	}
	return (Dee_cfa_t)-1;
}
#endif /* HOSTASM_X86 && !HOSTASM_X86_64 */

/* Push/move `regno' onto the host stack, returning the CFA offset of the target location. */
PRIVATE WUNUSED NONNULL((1)) Dee_cfa_t DCALL
Dee_function_generator_gsavereg(struct Dee_function_generator *__restrict self,
                                Dee_host_register_t regno) {
	Dee_cfa_t cfa_offset;
	ASSERT(!Dee_memstate_isshared(self->fg_state));
#ifdef HAVE_try_restore_xloc_arg_cfa_offset
	cfa_offset = try_restore_xloc_arg_cfa_offset(self, regno);
	if (cfa_offset != (Dee_cfa_t)-1)
		return cfa_offset;
#endif /* HAVE_try_restore_xloc_arg_cfa_offset */
	cfa_offset = Dee_memstate_hstack_find(self->fg_state,
	                                      self->fg_state_hstack_res,
	                                      HOST_SIZEOF_POINTER);
	if (cfa_offset != (Dee_cfa_t)-1) {
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
	return (Dee_cfa_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gflushregind(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc const *flush_loc) {
	struct Dee_memval *val;
	struct Dee_memstate *state = self->fg_state;
	Dee_host_register_t regno = Dee_memloc_hregind_getreg(flush_loc);
	ptrdiff_t ind_offset = Dee_memloc_hregind_getindoff(flush_loc);
	Dee_cfa_t cfa_offset;
	ASSERT(Dee_memloc_gettyp(flush_loc) == MEMADR_TYPE_HREGIND);
	cfa_offset = Dee_memstate_hstack_find(self->fg_state,
	                                      self->fg_state_hstack_res,
	                                      HOST_SIZEOF_POINTER);
	if (cfa_offset != (Dee_cfa_t)-1) {
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
			if (Dee_memobj_hasxinfo(obj)) {
				struct Dee_memobj_xinfo *xinfo;
				xinfo = Dee_memobj_getxinfo(obj);
				if (Dee_memloc_gettyp(&xinfo->mox_dep) == MEMADR_TYPE_HREGIND &&
				    Dee_memloc_hregind_getreg(&xinfo->mox_dep) == regno &&
				    Dee_memloc_hregind_getindoff(&xinfo->mox_dep) == ind_offset) {
					Dee_memloc_init_hstackind(&xinfo->mox_dep, cfa_offset,
					                          Dee_memloc_hregind_getvaloff(&xinfo->mox_dep));
				}
			}
		}
		Dee_memval_foreach_obj_end;
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
Dee_function_generator_vflushregs(struct Dee_function_generator *__restrict self,
                                  Dee_vstackaddr_t ignore_top_n_stack_if_not_ref,
                                  bool only_if_reference) {
	Dee_lid_t i;
	Dee_cfa_t register_cfa[HOST_REGISTER_COUNT];
	struct Dee_memstate *state = self->fg_state;
	bool changed_dependencies_to_stach = false;
	ASSERT(!Dee_memstate_isshared(state));

	/* Figure out which registers are in use, and assign them CFA offsets. */
	for (i = 0; i < HOST_REGISTER_COUNT; ++i)
		register_cfa[i] = (Dee_cfa_t)-1;
	for (i = 0; i < state->ms_localc; ++i) {
		struct Dee_memobj *obj;
		struct Dee_memval *val = &state->ms_localv[i];
		if (only_if_reference && Dee_memval_isdirect(val) && !Dee_memval_direct_isref(val))
			continue;
		Dee_memval_foreach_obj(obj, val) {
			if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HREG) {
				Dee_host_register_t regno = Dee_memobj_hreg_getreg(obj);
				ASSERT(regno < HOST_REGISTER_COUNT);
				if (register_cfa[regno] == (Dee_cfa_t)-1) {
					register_cfa[regno] = Dee_function_generator_gsavereg(self, regno);
					if unlikely(register_cfa[regno] == (Dee_cfa_t)-1)
						goto err;
				}
				Dee_memstate_decrinuse(state, regno);
				Dee_memloc_init_hstackind(Dee_memobj_getloc(obj), register_cfa[regno],
				                          Dee_memobj_hreg_getvaloff(obj));
				if (obj->mo_flags & MEMOBJ_F_HASDEP)
					changed_dependencies_to_stach = true;
			} else if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HREGIND) {
				if unlikely(Dee_function_generator_gflushregind(self, Dee_memobj_getloc(obj)))
					goto err;
				ASSERT(Dee_memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND);
			}
		}
		Dee_memval_foreach_obj_end;
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
				if (register_cfa[regno] == (Dee_cfa_t)-1) {
					register_cfa[regno] = Dee_function_generator_gsavereg(self, regno);
					if unlikely(register_cfa[regno] == (Dee_cfa_t)-1)
						goto err;
				}
				Dee_memstate_decrinuse(state, regno);
				Dee_memloc_init_hstackind(Dee_memobj_getloc(obj), register_cfa[regno],
				                          Dee_memobj_hreg_getvaloff(obj));
				if (obj->mo_flags & MEMOBJ_F_HASDEP)
					changed_dependencies_to_stach = true;
			} else if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HREGIND) {
				if unlikely(Dee_function_generator_gflushregind(self, Dee_memobj_getloc(obj)))
					goto err;
				ASSERT(Dee_memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND);
			}
		}
		Dee_memval_foreach_obj_end;
	}
	if (changed_dependencies_to_stach) {
		/* Must update consumers of dependencies of use new stack locations instead of registers. */
		struct Dee_memval *depends_mval;
		Dee_memstate_foreach(depends_mval, state) {
			struct Dee_memobj *depends_mobj;
			Dee_memval_foreach_obj(depends_mobj, depends_mval) {
				if (Dee_memobj_hasxinfo(depends_mobj)) {
					struct Dee_memobj_xinfo *xinfo;
					xinfo = Dee_memobj_getxinfo(depends_mobj);
					if (Dee_memloc_gettyp(&xinfo->mox_dep) == MEMADR_TYPE_HREG) {
						Dee_host_register_t regno;
						regno = Dee_memloc_hreg_getreg(&xinfo->mox_dep);
						ASSERT(regno < HOST_REGISTER_COUNT);
						if (register_cfa[regno] != (Dee_cfa_t)-1) {
							Dee_memloc_init_hstackind(&xinfo->mox_dep, register_cfa[regno],
							                          Dee_memloc_hreg_getvaloff(&xinfo->mox_dep));
						}
					}
				}
			}
			Dee_memval_foreach_obj_end;
		}
		Dee_memstate_foreach_end;
	}

	/* NOTE: Usage-registers must be cleared by the caller! */
	return 0;
err:
	return -1;
}

/* Flush memory locations that make use of `regno' onto the hstack. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vflushreg(struct Dee_function_generator *__restrict self,
                                 Dee_vstackaddr_t ignore_top_n_stack_if_not_ref,
                                 bool only_if_reference, Dee_host_register_t regno) {
	Dee_lid_t i;
	Dee_cfa_t regno_cfa = (Dee_cfa_t)-1;
	bool changed_dependencies_to_stach = false;
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
				if (regno_cfa == (Dee_cfa_t)-1) {
					regno_cfa = Dee_function_generator_gsavereg(self, regno);
					if unlikely(regno_cfa == (Dee_cfa_t)-1)
						goto err;
				}
				Dee_memstate_decrinuse(state, regno);
				Dee_memloc_init_hstackind(Dee_memobj_getloc(obj), regno_cfa,
				                          Dee_memobj_hreg_getvaloff(obj));
				if (obj->mo_flags & MEMOBJ_F_HASDEP)
					changed_dependencies_to_stach = true;
			} else if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HREGIND &&
			           Dee_memobj_hregind_getreg(obj) == regno) {
				if unlikely(Dee_function_generator_gflushregind(self, Dee_memobj_getloc(obj)))
					goto err;
				ASSERT(Dee_memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND);
			}
		}
		Dee_memval_foreach_obj_end;
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
				if (regno_cfa == (Dee_cfa_t)-1) {
					regno_cfa = Dee_function_generator_gsavereg(self, regno);
					if unlikely(regno_cfa == (Dee_cfa_t)-1)
						goto err;
				}
				Dee_memstate_decrinuse(state, regno);
				Dee_memloc_init_hstackind(Dee_memobj_getloc(obj), regno_cfa,
				                          Dee_memobj_hreg_getvaloff(obj));
				if (obj->mo_flags & MEMOBJ_F_HASDEP)
					changed_dependencies_to_stach = true;
			} else if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HREGIND &&
			           Dee_memobj_hregind_getreg(obj) == regno) {
				if unlikely(Dee_function_generator_gflushregind(self, Dee_memobj_getloc(obj)))
					goto err;
				ASSERT(Dee_memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND);
			}
		}
		Dee_memval_foreach_obj_end;
	}
	if (changed_dependencies_to_stach) {
		/* Must update consumers of dependencies of use new stack locations instead of registers. */
		struct Dee_memval *depends_mval;
		ASSERT(regno_cfa != (Dee_cfa_t)-1);
		Dee_memstate_foreach(depends_mval, state) {
			struct Dee_memobj *depends_mobj;
			Dee_memval_foreach_obj(depends_mobj, depends_mval) {
				if (Dee_memobj_hasxinfo(depends_mobj)) {
					struct Dee_memobj_xinfo *xinfo;
					xinfo = Dee_memobj_getxinfo(depends_mobj);
					if (Dee_memloc_gettyp(&xinfo->mox_dep) == MEMADR_TYPE_HREG &&
					    Dee_memloc_hreg_getreg(&xinfo->mox_dep) == regno) {
						Dee_memloc_init_hstackind(&xinfo->mox_dep, regno_cfa,
						                          Dee_memloc_hreg_getvaloff(&xinfo->mox_dep));
					}
				}
			}
			Dee_memval_foreach_obj_end;
		}
		Dee_memstate_foreach_end;
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
		Dee_memval_foreach_obj_end;
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
		Dee_memval_foreach_obj_end;
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
		if unlikely(Dee_function_generator_vflushreg(self, 0, false, result))
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
                                     struct Dee_memloc const *loc, Dee_instruction_t const *instr,
                                     struct Dee_module_object *mod, uint16_t id
#ifndef CONFIG_NO_THREADS
                                     , Dee_atomic_rwlock_t *opt_endread_before_throw
                                     , Dee_atomic_rwlock_t *opt_endwrite_before_throw
#endif /* !CONFIG_NO_THREADS */
                                     ) {
	DREF struct Dee_memstate *saved_state;
	struct Dee_host_symbol *target;
	struct Dee_host_section *text;
	struct Dee_host_section *cold;
	text = Dee_function_generator_gettext(self);
	cold = Dee_function_generator_getcold(self);
	if unlikely(!cold)
		goto err;
	target = Dee_function_generator_newsym_named(self, text == cold ? ".Lbound" : ".Lunbound");
	if unlikely(!target)
		goto err;
	DO(text == cold ? Dee_function_generator_gjnz(self, loc, target)
	                : Dee_function_generator_gjz(self, loc, target));
	saved_state = self->fg_state;
	Dee_memstate_incref(saved_state);
	EDO(err_saved_state, Dee_function_generator_state_dounshare(self));

	if (text != cold) {
		HA_printf(".section .cold\n");
		EDO(err_saved_state, Dee_function_generator_settext(self, cold));
		Dee_host_symbol_setsect(target, cold);
	}

	/* Location isn't bound -> generate code to throw an exception. */
#ifndef CONFIG_NO_THREADS
	if (opt_endwrite_before_throw != NULL)
		EDO(err_saved_state, Dee_function_generator_grwlock_endwrite_const(self, opt_endwrite_before_throw));
	if (opt_endread_before_throw != NULL)
		EDO(err_saved_state, Dee_function_generator_grwlock_endread_const(self, opt_endread_before_throw));
#endif /* !CONFIG_NO_THREADS */
	EDO(err_saved_state,
	    mod ? Dee_function_generator_gthrow_global_unbound(self, mod, id)
	        : Dee_function_generator_gthrow_local_unbound(self, instr, id));

	/* Switch back to the original section, and restore the saved mem-state. */
	Dee_memstate_decref(self->fg_state);
	self->fg_state = saved_state;

	ASSERT((text == cold) == !!Dee_host_symbol_isdefined(target));
	if (text != cold) {
		DO(Dee_function_generator_settext(self, text));
		HA_printf(".section .text\n");
	} else {
		Dee_host_symbol_setsect(target, text);
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
              struct Dee_memloc const *loc) {
	struct Dee_except_exitinfo *info;
	if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_CONST &&
	    Dee_memloc_const_getaddr(loc) != 0)
		return 0;
	info = Dee_function_generator_except_exit(self);
	if unlikely(!info)
		goto err;
	{
		Dee_function_generator_DEFINE_Dee_host_symbol_section(self, err, Lexcept, &info->exi_text, 0);
		if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_CONST)
			return Dee_function_generator_gjmp(self, Lexcept);
		return Dee_function_generator_gjz(self, loc, Lexcept);
	}
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
do_gjnz_except(struct Dee_function_generator *__restrict self,
               struct Dee_memloc const *loc) {
	struct Dee_except_exitinfo *info;
	if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_CONST &&
	    Dee_memloc_const_getaddr(loc) == 0)
		return 0;
	info = Dee_function_generator_except_exit(self);
	if unlikely(!info)
		goto err;
	{
		Dee_function_generator_DEFINE_Dee_host_symbol_section(self, err, Lexcept, &info->exi_text, 0);
		if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_CONST)
			return Dee_function_generator_gjmp(self, Lexcept);
		return Dee_function_generator_gjnz(self, loc, Lexcept);
	}
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
do_gjcmp_except(struct Dee_function_generator *__restrict self,
                struct Dee_memloc const *loc, intptr_t threshold,
                unsigned int flags) {
	struct Dee_except_exitinfo *info;
	struct Dee_memloc threshold_loc;
	info = Dee_function_generator_except_exit(self);
	if unlikely(!info)
		goto err;
	Dee_memloc_init_const(&threshold_loc, (byte_t const *)(uintptr_t)threshold);
	{
		Dee_function_generator_DEFINE_Dee_host_symbol_section(self, err, Lexcept, &info->exi_text, 0);
		return Dee_function_generator_gjcc(self, loc, &threshold_loc,
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
		Dee_function_generator_DEFINE_Dee_host_symbol_section(self, err, Lexcept, &info->exi_text, 0);
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
                                  struct Dee_memloc const *loc) {
	if likely(self->fg_exceptinject == NULL)
		return do_gjz_except(self, loc);
	return Dee_function_generator_gjeq_except(self, loc, 0);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gjnz_except(struct Dee_function_generator *__restrict self,
                                   struct Dee_memloc const *loc) {
	if likely(self->fg_exceptinject == NULL)
		return do_gjnz_except(self, loc);
	return Dee_function_generator_gjne_except(self, loc, 0);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
do_slow_gjcmp_except(struct Dee_function_generator *__restrict self,
                     struct Dee_memloc const *loc, intptr_t threshold,
                     unsigned int flags) {
	bool signed_cmp = !(flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_UNSIGNED);
	struct Dee_host_section *text = Dee_function_generator_gettext(self);
	struct Dee_host_section *cold = Dee_function_generator_getcold(self);
	struct Dee_memloc compare_value_loc;
	if unlikely(!cold)
		goto err;
	Dee_memloc_init_const(&compare_value_loc, (byte_t const *)(uintptr_t)threshold);
	if (cold == text) {
		struct Dee_host_symbol *Lno_except;
		Lno_except = Dee_function_generator_newsym_named(self, ".Lno_except");
		if unlikely(!Lno_except)
			goto err;
		if unlikely(Dee_function_generator_gjcc(self, loc, &compare_value_loc, signed_cmp,
		                                        (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_LO) ? NULL : Lno_except,
		                                        (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_EQ) ? NULL : Lno_except,
		                                        (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_GR) ? NULL : Lno_except))
			goto err;
		if unlikely(Dee_function_generator_gjmp_except(self))
			goto err;
		Dee_host_symbol_setsect(Lno_except, text);
	} else {
		struct Dee_host_symbol *Ldo_except;
		Ldo_except = Dee_function_generator_newsym_named(self, ".Ldo_except");
		if unlikely(!Ldo_except)
			goto err;
		if unlikely(Dee_function_generator_gjcc(self, loc, &compare_value_loc, false,
		                                        (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_LO) ? Ldo_except : NULL,
		                                        (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_EQ) ? Ldo_except : NULL,
		                                        (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_GR) ? Ldo_except : NULL))
			goto err;
		HA_printf(".section .cold\n");
		DO(Dee_function_generator_settext(self, cold));
		Dee_host_symbol_setsect(Ldo_except, cold);
		DO(Dee_function_generator_gjmp_except(self));
		HA_printf(".section .text\n");
		DO(Dee_function_generator_settext(self, text));
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gjcmp_except(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc const *loc, intptr_t threshold,
                                    unsigned int flags) {
	if likely(self->fg_exceptinject == NULL)
		return do_gjcmp_except(self, loc, threshold, flags);
	if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_CONST) {
		bool should_jump_except = false;
		intptr_t lhs = (intptr_t)(uintptr_t)Dee_memloc_const_getaddr(loc);
		if (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_LO) {
			should_jump_except |= (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_UNSIGNED)
			                      ? ((uintptr_t)lhs < (uintptr_t)threshold)
			                      : (lhs < threshold);
		}
		if (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_EQ)
			should_jump_except |= lhs == threshold;
		if (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_GR) {
			should_jump_except |= (flags & Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_UNSIGNED)
			                      ? ((uintptr_t)lhs > (uintptr_t)threshold)
			                      : (lhs > threshold);
		}
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
                                struct Dee_memloc *locv, size_t argc) {
	int result = _Dee_function_generator_gcallapi(self, locv, argc);
	if likely(result == 0) {
		Dee_function_generator_remember_undefined_allregs(self);
		Dee_memstate_hregs_clear_usage(self->fg_state);
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gincref_loc(struct Dee_function_generator *__restrict self,
                                   struct Dee_memloc const *loc, Dee_refcnt_t n) {
	struct Dee_memloc loc_asreg;
	switch (Dee_memloc_gettyp(loc)) {
	case MEMADR_TYPE_CONST:
#ifndef Dee_function_generator_gincref_const_MAYFAIL
		return Dee_function_generator_gincref_const(self, (DeeObject *)Dee_memloc_const_getaddr(loc), n);
#else /* !Dee_function_generator_gincref_const_MAYFAIL */
		{
			int result = Dee_function_generator_gincref_const(self, (DeeObject *)Dee_memloc_const_getaddr(loc), n);
			if likely(result <= 0)
				return result;
		}
		ATTR_FALLTHROUGH
#endif /* Dee_function_generator_gincref_const_MAYFAIL */
	default:
		if unlikely(Dee_function_generator_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gincref_regx(self,
		                                           Dee_memloc_hreg_getreg(loc),
		                                           Dee_memloc_hreg_getvaloff(loc),
		                                           n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdecref_loc(struct Dee_function_generator *__restrict self,
                                   struct Dee_memloc const *loc, Dee_refcnt_t n) {
	struct Dee_memloc loc_asreg;
	switch (Dee_memloc_gettyp(loc)) {
	case MEMADR_TYPE_CONST:
#ifndef Dee_function_generator_gdecref_const_MAYFAIL
		return Dee_function_generator_gdecref_const(self, (DeeObject *)Dee_memloc_const_getaddr(loc), n);
#else /* !Dee_function_generator_gdecref_const_MAYFAIL */
		{
			int result = Dee_function_generator_gdecref_const(self, (DeeObject *)Dee_memloc_const_getaddr(loc), n);
			if likely(result <= 0)
				return result;
		}
		ATTR_FALLTHROUGH
#endif /* Dee_function_generator_gdecref_const_MAYFAIL */
	default:
		if unlikely(Dee_function_generator_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gdecref_regx(self,
		                                           Dee_memloc_hreg_getreg(loc),
		                                           Dee_memloc_hreg_getvaloff(loc),
		                                           n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdecref_dokill_loc(struct Dee_function_generator *__restrict self,
                                          struct Dee_memloc const *loc) {
	struct Dee_memloc loc_asreg;
	switch (Dee_memloc_gettyp(loc)) {
	default:
		if unlikely(Dee_function_generator_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
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
                                          struct Dee_memloc const *loc, Dee_refcnt_t n) {
	struct Dee_memloc loc_asreg;
	switch (Dee_memloc_gettyp(loc)) {
	default:
		if unlikely(Dee_function_generator_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gdecref_nokill_regx(self,
		                                                  Dee_memloc_hreg_getreg(loc),
		                                                  Dee_memloc_hreg_getvaloff(loc),
		                                                  n);
	case MEMADR_TYPE_CONST:
		return Dee_function_generator_gdecref_loc(self, loc, n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxincref_loc(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc const *loc, Dee_refcnt_t n) {
	struct Dee_memloc loc_asreg;
	switch (Dee_memloc_gettyp(loc)) {
	default:
		if unlikely(Dee_function_generator_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gxincref_regx(self,
		                                            Dee_memloc_hreg_getreg(loc),
		                                            Dee_memloc_hreg_getvaloff(loc),
		                                            n);
	case MEMADR_TYPE_CONST:
		if (Dee_memloc_const_getaddr(loc) == NULL)
			return 0;
		return Dee_function_generator_gincref_loc(self, loc, n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxdecref_loc(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc const *loc, Dee_refcnt_t n) {
	struct Dee_memloc loc_asreg;
	switch (Dee_memloc_gettyp(loc)) {
	default:
		if unlikely(Dee_function_generator_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gxdecref_regx(self,
		                                            Dee_memloc_hreg_getreg(loc),
		                                            Dee_memloc_hreg_getvaloff(loc),
		                                            n);
	case MEMADR_TYPE_CONST:
		if (Dee_memloc_const_getaddr(loc) == NULL)
			return 0;
		return Dee_function_generator_gdecref_loc(self, loc, n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxdecref_nokill_loc(struct Dee_function_generator *__restrict self,
                                           struct Dee_memloc const *loc, Dee_refcnt_t n) {
	struct Dee_memloc loc_asreg;
	switch (Dee_memloc_gettyp(loc)) {
	default:
		if unlikely(Dee_function_generator_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gxdecref_nokill_regx(self,
		                                                   Dee_memloc_hreg_getreg(loc),
		                                                   Dee_memloc_hreg_getvaloff(loc),
		                                                   n);
	case MEMADR_TYPE_CONST:
		if (Dee_memloc_const_getaddr(loc) == NULL)
			return 0;
		return Dee_function_generator_gdecref_loc(self, loc, n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

/* Change `loc' into the value of `<result> = *(<loc> + ind_delta)'
 * Note that unlike the `Dee_function_generator_gmov*' functions, this
 * one may use `MEMADR_TYPE_*IND' to defer the indirection until later. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gasind(struct Dee_function_generator *__restrict self,
                              /*in*/ struct Dee_memloc const *loc,
                              /*out*/ struct Dee_memloc *result,
                              ptrdiff_t ind_delta) {
	struct Dee_memloc loc_asreg;
	struct Dee_memequiv *eq;
	switch (Dee_memloc_gettyp(loc)) {

	case MEMADR_TYPE_HSTACK: {
		Dee_cfa_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(Dee_memloc_hstack_getcfa(loc), ind_delta);
		eq = Dee_function_generator_remember_getclassof_hstackind(self, cfa_offset);
		if (eq != NULL) {
			Dee_memequiv_next_asloc(eq, result);
			return 0;
		}
		Dee_memloc_init_hstackind(result, cfa_offset, 0);
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
		Dee_memloc_init_hreg(result, temp_regno, 0);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	default:
		if unlikely(Dee_function_generator_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ASSERT(Dee_memloc_gettyp(loc) == MEMADR_TYPE_HREG);
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG: {
		Dee_host_register_t regno = Dee_memloc_hreg_getreg(loc);
		ptrdiff_t final_ind_delta = Dee_memloc_hreg_getvaloff(loc) + ind_delta;
		eq = Dee_function_generator_remember_getclassof_regind(self, regno, final_ind_delta);
		if (eq != NULL) {
			Dee_memequiv_next_asloc(eq, result);
			return 0;
		}
		/* Turn the location from an HREG into HREGIND */
		Dee_memloc_init_hregind(result, regno, final_ind_delta, 0);
	}	break;

	}
	return 0;
err:
	return -1;
}

/* Force `loc' to become a register (`MEMADR_TYPE_HREG'). */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gasreg(struct Dee_function_generator *__restrict self,
                              /*in*/ struct Dee_memloc const *loc,
                              /*out*/ struct Dee_memloc *result,
                              Dee_host_register_t const *not_these) {
	struct Dee_memequiv *eq;
	Dee_host_register_t regno;
	ptrdiff_t val_delta;
	if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_HREG) {
		*result = *loc;
		return 0; /* Already in a register! */
	}

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
		*result = reg_eq->meq_loc;
		ASSERT(Dee_memloc_gettyp(result) == MEMADR_TYPE_HREG);
		Dee_memloc_adjoff(result, val_delta);
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

	/* Remember that `loc' now lies in a register. */
	Dee_memloc_init_hreg(result, regno, val_delta);
	return 0;
err:
	return -1;
}

/* Force `loc' to reside on the stack, giving it an address
 * (`MEMADR_TYPE_HSTACKIND, Dee_memloc_hstackind_getvaloff = 0').
 * @param: require_valoff_0: When false, forgo the exit requirement
 *                           of `Dee_memloc_hstackind_getvaloff = 0' */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gasflush(struct Dee_function_generator *__restrict self,
                                /*in*/ struct Dee_memloc const *loc,
                                /*out*/ struct Dee_memloc *result,
                                bool require_valoff_0) {
	ptrdiff_t val_offset;
	Dee_cfa_t cfa_offset;
	struct Dee_memstate *state = self->fg_state;
	ASSERT(!Dee_memstate_isshared(state));
	*result = *loc;
	if (Dee_memloc_gettyp(result) == MEMADR_TYPE_HSTACKIND) {
handle_hstackind_loc:
		if (Dee_memloc_hstackind_getvaloff(result) == 0)
			return 0; /* Already on-stack at offset=0 */
		if (require_valoff_0)
			return 0; /* Caller doesn't care about value offset */

#ifdef HAVE__Dee_host_section_gadd_const2hstackind
		/* emit `addP $..., sp_offset(%Psp)' to adjust the offset of the stored value
		 * Afterwards, go through all stack/local variables and adjust value offsets
		 * wherever the same CFA offset is referenced. */
		return _Dee_host_section_gadd_const2hstackind(Dee_function_generator_gettext(self),
		                                              (void const *)(uintptr_t)(intptr_t)Dee_memloc_hstackind_getvaloff(result),
		                                              Dee_memstate_hstack_cfa2sp(self->fg_state, Dee_memloc_hstackind_getcfa(result)));
#endif /* HAVE__Dee_host_section_gadd_const2hstackind */
	}

	/* Figure out where we want to allocate the value. */
#ifdef HAVE_try_restore_xloc_arg_cfa_offset
	if (Dee_memloc_gettyp(result) == MEMADR_TYPE_HREG &&
		(cfa_offset = try_restore_xloc_arg_cfa_offset(self, Dee_memloc_hreg_getreg(result))) != (Dee_cfa_t)-1) {
		val_offset = 0; /* CFA offset restored */
	} else
#endif /* HAVE_try_restore_xloc_arg_cfa_offset */
	{
		/* Check if "result" has a known HSTACKIND equivalence. */
		struct Dee_memequiv *eq;

		eq = Dee_function_generator_remember_getclassof(self, Dee_memloc_getadr(result));
		if (eq != NULL) {
			ptrdiff_t val_delta = Dee_memloc_getoff(result);
			struct Dee_memequiv *hstackind_eq_any = NULL;
			struct Dee_memequiv *hstackind_eq;
			val_delta -= Dee_memloc_getoff(&eq->meq_loc);
			for (hstackind_eq = Dee_memequiv_next(eq); hstackind_eq != eq;
			     hstackind_eq = Dee_memequiv_next(hstackind_eq)) {
				if (Dee_memloc_gettyp(&hstackind_eq->meq_loc) == MEMEQUIV_TYPE_HSTACKIND) {
					if ((Dee_memloc_getoff(&hstackind_eq->meq_loc) + val_delta) == 0) {
						/* Perfect match: this equivalence allows for `v_hstack.s_off = 0' */
						*result = hstackind_eq->meq_loc;
						ASSERT((Dee_memloc_getoff(result) + val_delta) == 0);
						Dee_memloc_setoff(result, 0);
						ASSERT(Dee_memloc_gettyp(result) == MEMADR_TYPE_HSTACKIND);
						ASSERT(Dee_memloc_getoff(result) == 0);
						return 0;
					}
					hstackind_eq_any = hstackind_eq;
				}
			}
			if (hstackind_eq_any) {
				*result = hstackind_eq->meq_loc;
				Dee_memloc_setoff(result, Dee_memloc_getoff(result) + val_delta);
				ASSERT(Dee_memloc_gettyp(result) == MEMADR_TYPE_HSTACKIND);
				ASSERT(Dee_memloc_getoff(result) != 0);
				goto handle_hstackind_loc;
			}
		}

		/* Search for a currently free stack location. */
		cfa_offset = Dee_memstate_hstack_find(state, self->fg_state_hstack_res, HOST_SIZEOF_POINTER);
		val_offset = result->ml_off;
		if (!require_valoff_0)
			result->ml_off = 0; /* Don't include value offset when saving location */
		if (cfa_offset != (Dee_cfa_t)-1) {
			if unlikely(Dee_function_generator_gmov_loc2hstackind(self, result, cfa_offset))
				goto err;
		} else {
			if unlikely(Dee_function_generator_ghstack_pushloc(self, result))
				goto err;
#ifdef HOSTASM_STACK_GROWS_DOWN
			cfa_offset = state->ms_host_cfa_offset;
#else /* HOSTASM_STACK_GROWS_DOWN */
			cfa_offset = state->ms_host_cfa_offset - HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		}
		result->ml_off = val_offset;
		if (require_valoff_0)
			val_offset = 0;
	}

	/* If the location used to be a writable location, then we must
	 * also update any other location that used to alias `result'. */
	state = self->fg_state;
	if (Dee_memloc_gettyp(result) == MEMADR_TYPE_HREG ||
	    Dee_memloc_gettyp(result) == MEMADR_TYPE_HREGIND) {
		ptrdiff_t val_delta_change = -Dee_memloc_getoff(result);
		if (val_delta_change != val_offset) {
			struct Dee_memval *alias_val;
			struct Dee_memobj *alias_obj;
			struct Dee_memloc orig_loc = *result;
			ASSERT(val_offset == 0);
			val_delta_change += val_offset;
			Dee_memstate_foreach(alias_val, state) {
				Dee_memval_foreach_obj(alias_obj, alias_val) {
					if (!Dee_memloc_sameadr(Dee_memobj_getloc(alias_obj), &orig_loc))
						continue;
					ASSERT(Dee_memobj_hasreg(alias_obj));
					Dee_memstate_decrinuse(self->fg_state, Dee_memobj_getreg(alias_obj));
					Dee_memloc_init_hstackind(Dee_memobj_getloc(alias_obj), cfa_offset,
					                          Dee_memobj_getoff(alias_obj) + val_delta_change);
				}
				Dee_memval_foreach_obj_end;
			}
			Dee_memstate_foreach_end;
		}
	}

	/* Remember that `result' now lies on-stack (with an offset of `val_offset') */
	Dee_memloc_init_hstackind(result, cfa_offset, val_offset);
	return 0;
err:
	return -1;
}


/* Check if `src_loc' differs from `dst_loc', and if so: move `src_loc' *into* `dst_loc'. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gmov_loc2loc(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc const *src_loc,
                                    struct Dee_memloc const *dst_loc) {
	int result;
	struct Dee_memloc src_asreg;
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
			if unlikely(Dee_function_generator_gasreg(self, src_loc, &src_asreg, not_these))
				goto err;
			src_loc = &src_asreg;
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
                                       struct Dee_memloc const *src_loc,
                                       struct Dee_memloc const *dst_loc, ptrdiff_t ind_delta) {
	int result;
	struct Dee_memloc src_asreg;
	switch (Dee_memloc_gettyp(src_loc)) {
	default: {
		Dee_host_register_t not_these[2];
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (Dee_memloc_hasreg(dst_loc))
			not_these[0] = Dee_memloc_getreg(dst_loc);
		if unlikely(Dee_function_generator_gasreg(self, src_loc, &src_asreg, not_these))
			goto err;
		src_loc = &src_asreg;
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
			if (src_loc != &src_asreg) {
				src_asreg = *src_loc;
				src_loc = &src_asreg;
			}
			Dee_memloc_hreg_setvaloff(&src_asreg, 0);
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


#ifndef CONFIG_NO_THREADS
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
#endif /* !CONFIG_NO_THREADS */




#ifdef _Dee_function_generator_gjcc_regindCconst_MAYFAIL
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjcc_regindCconst(struct Dee_function_generator *__restrict self,
                                         Dee_host_register_t lhs_regno, ptrdiff_t lhs_ind_delta,
                                         void const *rhs_value, bool signed_cmp,
                                         struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq,
                                         struct Dee_host_symbol *dst_gr) {
	int result = _Dee_function_generator_gjcc_regindCconst(self, lhs_regno, lhs_ind_delta, rhs_value,
	                                                       signed_cmp, dst_lo, dst_eq, dst_gr);
	if unlikely(result > 0) {
		Dee_host_register_t tempreg;
		Dee_host_register_t not_these[2];
		not_these[0] = lhs_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		tempreg = Dee_function_generator_gconst_as_reg(self, rhs_value, not_these);
		if unlikely(tempreg >= HOST_REGISTER_COUNT)
			return -1;
		result = Dee_function_generator_gjcc_regindCreg(self, lhs_regno, lhs_ind_delta, tempreg,
		                                                signed_cmp, dst_lo, dst_eq, dst_gr);
	}
	return result;
}
#endif /* !_Dee_function_generator_gjcc_regindCconst_MAYFAIL */

#ifdef _Dee_function_generator_gjcc_regCconst_MAYFAIL
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjcc_regCconst(struct Dee_function_generator *__restrict self,
                                      Dee_host_register_t lhs_regno, void const *rhs_value, bool signed_cmp,
                                      struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq,
                                      struct Dee_host_symbol *dst_gr) {
	int result = _Dee_function_generator_gjcc_regCconst(self, lhs_regno, rhs_value,
	                                                    signed_cmp, dst_lo, dst_eq, dst_gr);
	if unlikely(result > 0) {
		Dee_host_register_t tempreg;
		Dee_host_register_t not_these[2];
		not_these[0] = lhs_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		tempreg = Dee_function_generator_gconst_as_reg(self, rhs_value, not_these);
		if unlikely(tempreg >= HOST_REGISTER_COUNT)
			return -1;
		result = Dee_function_generator_gjcc_regCreg(self, lhs_regno, tempreg,
		                                             signed_cmp, dst_lo, dst_eq, dst_gr);
	}
	return result;
}
#endif /* !_Dee_function_generator_gjcc_regCconst_MAYFAIL */

#ifdef _Dee_function_generator_gjcc_hstackindCconst_MAYFAIL
INTDEF WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjcc_hstackindCconst(struct Dee_function_generator *__restrict self,
                                            Dee_cfa_t lhs_cfa_offset, void const *rhs_value, bool signed_cmp,
                                            struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq,
                                            struct Dee_host_symbol *dst_gr) {
	ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, lhs_cfa_offset);
	int result = _Dee_function_generator_gjcc_hstackindCconst(self, sp_offset, rhs_value,
	                                                          signed_cmp, dst_lo, dst_eq, dst_gr);
	if unlikely(result > 0) {
		Dee_host_register_t tempreg;
		tempreg = Dee_function_generator_gconst_as_reg(self, rhs_value, NULL);
		if unlikely(tempreg >= HOST_REGISTER_COUNT)
			return -1;
		result = Dee_function_generator_gjcc_hstackindCreg(self, lhs_cfa_offset, tempreg,
		                                                   signed_cmp, dst_lo, dst_eq, dst_gr);
	}
	return result;
}
#endif /* !_Dee_function_generator_gjcc_hstackindCconst_MAYFAIL */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gjcc_locCregx(struct Dee_function_generator *__restrict self, struct Dee_memloc const *lhs,
                                     Dee_host_register_t rhs_regno, ptrdiff_t rhs_val_offset, bool signed_cmp,
                                     struct Dee_host_symbol *dst_lo, struct Dee_host_symbol *dst_eq,
                                     struct Dee_host_symbol *dst_gr) {
	struct Dee_memloc lhs_asreg;
	switch (Dee_memloc_gettyp(lhs)) {
	default: {
		Dee_host_register_t not_these[2];
fallback:
		not_these[0] = rhs_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		if unlikely(Dee_function_generator_gasreg(self, lhs, &lhs_asreg, not_these))
			goto err;
		lhs = &lhs_asreg;
	}	ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		rhs_val_offset -= Dee_memloc_hreg_getvaloff(lhs);
		if (Dee_memloc_hreg_getreg(lhs) == rhs_regno) { /* 0 <=> rhs_val_offset */
			struct Dee_host_symbol *dst;
			if (0 < rhs_val_offset) {
				dst = dst_lo;
			} else if (0 > rhs_val_offset) {
				dst = dst_gr;
			} else {
				dst = dst_eq;
			}
			return dst ? Dee_function_generator_gjmp(self, dst) : 0;
		}
		if (rhs_val_offset != 0) {
			if unlikely(Dee_function_generator_gmov_regx2reg(self, rhs_regno, rhs_val_offset, rhs_regno))
				goto err;
			Dee_memstate_hregs_adjust_delta(self->fg_state, rhs_regno, rhs_val_offset);
		}
		return Dee_function_generator_gjcc_regCreg(self, Dee_memloc_hreg_getreg(lhs), rhs_regno,
		                                           signed_cmp, dst_lo, dst_eq, dst_gr);
	case MEMADR_TYPE_HREGIND:
		if ((rhs_val_offset - Dee_memloc_hregind_getvaloff(lhs)) != 0)
			goto fallback;
		return Dee_function_generator_gjcc_regindCreg(self,
		                                              Dee_memloc_hregind_getreg(lhs),
		                                              Dee_memloc_hregind_getindoff(lhs), rhs_regno,
		                                              signed_cmp, dst_lo, dst_eq, dst_gr);
	case MEMADR_TYPE_HSTACKIND:
		if ((rhs_val_offset - Dee_memloc_hstackind_getvaloff(lhs)) != 0)
			goto fallback;
		return Dee_function_generator_gjcc_hstackindCreg(self,
		                                                 Dee_memloc_hstackind_getcfa(lhs), rhs_regno,
		                                                 signed_cmp, dst_lo, dst_eq, dst_gr);
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gjcc_locCconst(struct Dee_function_generator *__restrict self, struct Dee_memloc const *lhs,
                                      void const *rhs_value, bool signed_cmp, struct Dee_host_symbol *dst_lo,
                                      struct Dee_host_symbol *dst_eq, struct Dee_host_symbol *dst_gr) {
	struct Dee_memloc lhs_asreg;
	switch (Dee_memloc_gettyp(lhs)) {
	default:
		if unlikely(Dee_function_generator_gasreg(self, lhs, &lhs_asreg, NULL))
			goto err;
		lhs = &lhs_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		rhs_value = (void const *)((uintptr_t)rhs_value - Dee_memloc_hreg_getvaloff(lhs));
		return Dee_function_generator_gjcc_regCconst(self, Dee_memloc_hreg_getreg(lhs), rhs_value,
		                                             signed_cmp, dst_lo, dst_eq, dst_gr);
	case MEMADR_TYPE_HREGIND:
		rhs_value = (void const *)((uintptr_t)rhs_value - Dee_memloc_hregind_getvaloff(lhs));
		return Dee_function_generator_gjcc_regindCconst(self,
		                                                Dee_memloc_hregind_getreg(lhs),
		                                                Dee_memloc_hregind_getindoff(lhs), rhs_value,
		                                                signed_cmp, dst_lo, dst_eq, dst_gr);
	case MEMADR_TYPE_HSTACKIND:
		rhs_value = (void const *)((uintptr_t)rhs_value - Dee_memloc_hstackind_getvaloff(lhs));
		return Dee_function_generator_gjcc_hstackindCconst(self,
		                                                   Dee_memloc_hstackind_getcfa(lhs), rhs_value,
		                                                   signed_cmp, dst_lo, dst_eq, dst_gr);
	}
	__builtin_unreachable();
err:
	return -1;
}


#undef NEED_Dee_function_generator_gjccA_reg
#ifndef HAVE__Dee_function_generator_gjcc_regAreg
#define NEED_Dee_function_generator_gjccA_reg
#endif /* !HAVE__Dee_function_generator_gjcc_regAreg */
#ifndef HAVE__Dee_function_generator_gjcc_regindAreg
#define NEED_Dee_function_generator_gjccA_reg
#endif /* !HAVE__Dee_function_generator_gjcc_regindAreg */
#ifndef HAVE__Dee_function_generator_gjcc_hstackindAreg
#define NEED_Dee_function_generator_gjccA_reg
#endif /* !HAVE__Dee_function_generator_gjcc_hstackindAreg */
#ifndef HAVE__Dee_function_generator_gjcc_regAconst
#define NEED_Dee_function_generator_gjccA_reg
#endif /* !HAVE__Dee_function_generator_gjcc_regAconst */

#ifdef NEED_Dee_function_generator_gjccA_reg
PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjccA_reg(struct Dee_function_generator *__restrict self, Dee_host_register_t regno,
                                 struct Dee_host_symbol *dst_nz, struct Dee_host_symbol *dst_z) {
	if (dst_nz) {
		if (dst_z) {
			if (dst_nz != dst_z)
				DO(Dee_function_generator_gjnz_reg(self, regno, dst_nz));
			return Dee_function_generator_gjmp(self, dst_z);
		}
		return Dee_function_generator_gjnz_reg(self, regno, dst_nz);
	} else if (dst_z) {
		return Dee_function_generator_gjz_reg(self, regno, dst_z);
	}
	return 0;
err:
	return -1;
}
#endif /* NEED_Dee_function_generator_gjccA_reg */


/* Conditional jump based on "(<lhs> & <rhs>) !=/= 0" */
#ifndef HAVE__Dee_function_generator_gjcc_regAreg
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjcc_regAreg(struct Dee_function_generator *__restrict self,
                                    Dee_host_register_t lhs_regno, Dee_host_register_t rhs_regno,
                                    struct Dee_host_symbol *dst_nz, struct Dee_host_symbol *dst_z) {
	Dee_host_register_t dst_regno = Dee_function_generator_gallocreg(self, NULL);
	if unlikely(dst_regno >= HOST_SIZEOF_POINTER)
		goto err;
	DO(Dee_function_generator_gbitop_regreg2reg(self, BITOP_AND, lhs_regno, rhs_regno, dst_regno));
	return Dee_function_generator_gjccA_reg(self, dst_regno, dst_nz, dst_z);
err:
	return -1;
}
#endif /* !HAVE__Dee_function_generator_gjcc_regAreg */

#ifndef HAVE__Dee_function_generator_gjcc_regindAreg
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjcc_regindAreg(struct Dee_function_generator *__restrict self,
                                       Dee_host_register_t lhs_regno, ptrdiff_t lhs_ind_delta,
                                       Dee_host_register_t rhs_regno,
                                       struct Dee_host_symbol *dst_nz, struct Dee_host_symbol *dst_z) {
	Dee_host_register_t dst_regno = Dee_function_generator_gallocreg(self, NULL);
	if unlikely(dst_regno >= HOST_SIZEOF_POINTER)
		goto err;
	DO(Dee_function_generator_gbitop_regregind2reg(self, BITOP_AND, rhs_regno, lhs_regno, lhs_ind_delta, dst_regno));
	return Dee_function_generator_gjccA_reg(self, dst_regno, dst_nz, dst_z);
err:
	return -1;
}
#endif /* !HAVE__Dee_function_generator_gjcc_regindAreg */

#ifndef HAVE__Dee_function_generator_gjcc_hstackindAreg
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjcc_hstackindAreg(struct Dee_function_generator *__restrict self,
                                          Dee_cfa_t lhs_cfa_offset, Dee_host_register_t rhs_regno,
                                          struct Dee_host_symbol *dst_nz, struct Dee_host_symbol *dst_z) {
	Dee_host_register_t dst_regno = Dee_function_generator_gallocreg(self, NULL);
	if unlikely(dst_regno >= HOST_SIZEOF_POINTER)
		goto err;
	DO(Dee_function_generator_gbitop_reghstackind2reg(self, BITOP_AND, rhs_regno, lhs_cfa_offset, dst_regno));
	return Dee_function_generator_gjccA_reg(self, dst_regno, dst_nz, dst_z);
err:
	return -1;
}
#endif /* !HAVE__Dee_function_generator_gjcc_hstackindAreg */

#ifndef HAVE__Dee_function_generator_gjcc_regindAconst
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjcc_regindAconst(struct Dee_function_generator *__restrict self,
                                         Dee_host_register_t lhs_regno, ptrdiff_t lhs_ind_delta, void const *rhs_value,
                                         struct Dee_host_symbol *dst_nz, struct Dee_host_symbol *dst_z) {
	Dee_host_register_t tempreg = Dee_function_generator_gallocreg(self, NULL);
	if unlikely(tempreg >= HOST_REGISTER_COUNT)
		goto err;
	DO(Dee_function_generator_gmov_regind2reg(self, lhs_regno, lhs_ind_delta, tempreg));
	return Dee_function_generator_gjcc_regAconst(self, tempreg, rhs_value, dst_nz, dst_z);
err:
	return -1;
}
#elif defined(_Dee_function_generator_gjcc_regindAconst_MAYFAIL)
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjcc_regindAconst(struct Dee_function_generator *__restrict self,
                                         Dee_host_register_t lhs_regno, ptrdiff_t lhs_ind_delta, void const *rhs_value,
                                         struct Dee_host_symbol *dst_nz, struct Dee_host_symbol *dst_z) {
	int result = _Dee_function_generator_gjcc_regindAconst(self, lhs_regno, lhs_ind_delta, rhs_value, dst_nz, dst_z);
	if unlikely(result > 0) {
		Dee_host_register_t tempreg, not_these[2];
		not_these[0] = lhs_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		tempreg = Dee_function_generator_gconst_as_reg(self, rhs_value, not_these);
		if unlikely(tempreg >= HOST_REGISTER_COUNT)
			return -1;
		return Dee_function_generator_gjcc_regindAreg(self, lhs_regno, lhs_ind_delta, tempreg, dst_nz, dst_z);
	}
	return result;
}
#endif /* !HAVE__Dee_function_generator_gjcc_regindAconst || _Dee_function_generator_gjcc_regindAconst_MAYFAIL */

#ifndef HAVE__Dee_function_generator_gjcc_regAconst
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjcc_regAconst(struct Dee_function_generator *__restrict self,
                                      Dee_host_register_t lhs_regno, void const *rhs_value,
                                      struct Dee_host_symbol *dst_nz, struct Dee_host_symbol *dst_z) {
	Dee_host_register_t dst_regno = Dee_function_generator_gallocreg(self, NULL);
	if unlikely(dst_regno >= HOST_REGISTER_COUNT)
		goto err;
	DO(Dee_function_generator_gbitop_regconst2reg(self, BITOP_AND, lhs_regno, rhs_value, dst_regno));
	return Dee_function_generator_gjccA_reg(self, dst_regno, dst_nz, dst_z);
err:
	return -1;
}
#elif defined(_Dee_function_generator_gjcc_regAconst_MAYFAIL)
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjcc_regAconst(struct Dee_function_generator *__restrict self,
                                      Dee_host_register_t lhs_regno, void const *rhs_value,
                                      struct Dee_host_symbol *dst_nz, struct Dee_host_symbol *dst_z) {
	int result = _Dee_function_generator_gjcc_regAconst(self, lhs_regno, rhs_value, dst_nz, dst_z);
	if unlikely(result > 0) {
		Dee_host_register_t tempreg, not_these[2];
		not_these[0] = lhs_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		tempreg = Dee_function_generator_gconst_as_reg(self, rhs_value, not_these);
		if unlikely(tempreg >= HOST_REGISTER_COUNT)
			return -1;
		return Dee_function_generator_gjcc_regAreg(self, lhs_regno, tempreg, dst_nz, dst_z);
	}
	return result;
}
#endif /* !HAVE__Dee_function_generator_gjcc_regAconst || _Dee_function_generator_gjcc_regAconst_MAYFAIL */

#ifndef HAVE__Dee_function_generator_gjcc_hstackindAconst
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjcc_hstackindAconst(struct Dee_function_generator *__restrict self,
                                            Dee_cfa_t lhs_cfa_offset, void const *rhs_value,
                                            struct Dee_host_symbol *dst_nz, struct Dee_host_symbol *dst_z) {
	Dee_host_register_t tempreg = Dee_function_generator_gallocreg(self, NULL);
	if unlikely(tempreg >= HOST_REGISTER_COUNT)
		goto err;
	DO(Dee_function_generator_gmov_hstackind2reg(self, lhs_cfa_offset, tempreg));
	return Dee_function_generator_gjcc_regAconst(self, tempreg, rhs_value, dst_nz, dst_z);
err:
	return -1;
}
#elif defined(_Dee_function_generator_gjcc_hstackindAconst_MAYFAIL)
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjcc_hstackindAconst(struct Dee_function_generator *__restrict self,
                                            Dee_cfa_t lhs_cfa_offset, void const *rhs_value,
                                            struct Dee_host_symbol *dst_nz, struct Dee_host_symbol *dst_z) {
	int result = _Dee_function_generator_gjcc_hstackindAconst(self, lhs_cfa_offset, rhs_value, dst_nz, dst_z);
	if unlikely(result > 0) {
		Dee_host_register_t tempreg;
		tempreg = Dee_function_generator_gconst_as_reg(self, rhs_value, NULL);
		if unlikely(tempreg >= HOST_REGISTER_COUNT)
			return -1;
		return Dee_function_generator_gjcc_hstackindAreg(self, lhs_cfa_offset, tempreg, dst_nz, dst_z);
	}
	return result;
}
#endif /* !HAVE__Dee_function_generator_gjcc_hstackindAconst || _Dee_function_generator_gjcc_hstackindAconst_MAYFAIL */


INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjcc_regxAregx(struct Dee_function_generator *__restrict self,
                                      Dee_host_register_t lhs_regno, ptrdiff_t lhs_val_offset,
                                      Dee_host_register_t rhs_regno, ptrdiff_t rhs_val_offset,
                                      struct Dee_host_symbol *dst_nz, struct Dee_host_symbol *dst_z) {
	if (lhs_val_offset != 0 || rhs_val_offset != 0) {
		if (lhs_regno == rhs_regno) {
			Dee_host_register_t temp, not_these[2];
			/* Need to use a temporary register */
			not_these[0] = lhs_regno;
			not_these[1] = HOST_REGISTER_COUNT;
			temp = Dee_function_generator_gallocreg(self, not_these);
			if unlikely(temp >= HOST_REGISTER_COUNT)
				goto err;
			if (lhs_val_offset != 0) {
				DO(Dee_function_generator_gmov_regx2reg(self, lhs_regno, lhs_val_offset, temp));
				lhs_regno = temp;
				lhs_val_offset = 0;
			} else {
				DO(Dee_function_generator_gmov_regx2reg(self, rhs_regno, rhs_val_offset, temp));
				rhs_regno = temp;
				rhs_val_offset = 0;
			}
		}
		if (lhs_val_offset != 0) {
			DO(Dee_function_generator_gmov_regx2reg(self, lhs_regno, lhs_val_offset, lhs_regno));
			Dee_memstate_hregs_adjust_delta(self->fg_state, lhs_regno, lhs_val_offset);
			/*lhs_val_offset = 0;*/
		}
		if (rhs_val_offset != 0) {
			DO(Dee_function_generator_gmov_regx2reg(self, rhs_regno, rhs_val_offset, rhs_regno));
			Dee_memstate_hregs_adjust_delta(self->fg_state, rhs_regno, rhs_val_offset);
			/*rhs_val_offset = 0;*/
		}
	}
	return Dee_function_generator_gjcc_regAreg(self, lhs_regno, rhs_regno, dst_nz, dst_z);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gjcc_locAregx(struct Dee_function_generator *__restrict self, struct Dee_memloc const *lhs,
                                     Dee_host_register_t rhs_regno, ptrdiff_t rhs_val_offset,
                                     struct Dee_host_symbol *dst_nz, struct Dee_host_symbol *dst_z) {
	struct Dee_memloc lhs_asreg;
	switch (Dee_memloc_gettyp(lhs)) {
	default: {
		Dee_host_register_t not_these[2];
fallback:
		not_these[0] = rhs_regno;
		not_these[1] = HOST_REGISTER_COUNT;
		DO(Dee_function_generator_gasreg(self, lhs, &lhs_asreg, not_these));
		lhs = &lhs_asreg;
	}	ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gjcc_regxAregx(self,
		                                             Dee_memloc_hreg_getreg(lhs),
		                                             Dee_memloc_hreg_getvaloff(lhs),
		                                             rhs_regno, rhs_val_offset,
		                                             dst_nz, dst_z);

	case MEMADR_TYPE_HREGIND: {
		Dee_host_register_t lhs_regno = Dee_memloc_hregind_getreg(lhs);
		ptrdiff_t lhs_indoff = Dee_memloc_hregind_getindoff(lhs);
		if (Dee_memloc_hregind_getvaloff(lhs) != 0)
			goto fallback;
		if (rhs_val_offset != 0) {
			DO(Dee_function_generator_gmov_regx2reg(self, rhs_regno, rhs_val_offset, rhs_regno));
			Dee_memstate_hregs_adjust_delta(self->fg_state, rhs_regno, rhs_val_offset);
			if (lhs_regno == rhs_regno)
				lhs_indoff += rhs_val_offset;
		}
		return Dee_function_generator_gjcc_regindAreg(self, lhs_regno, lhs_indoff,
		                                              rhs_regno, dst_nz, dst_z);
	}	break;

	case MEMADR_TYPE_HSTACKIND: {
		if (Dee_memloc_hstackind_getvaloff(lhs) != 0)
			goto fallback;
		if (rhs_val_offset != 0) {
			DO(Dee_function_generator_gmov_regx2reg(self, rhs_regno, rhs_val_offset, rhs_regno));
			Dee_memstate_hregs_adjust_delta(self->fg_state, rhs_regno, rhs_val_offset);
		}
		return Dee_function_generator_gjcc_hstackindAreg(self,
		                                                 Dee_memloc_hstackind_getcfa(lhs),
		                                                 rhs_regno, dst_nz, dst_z);
	}	break;

	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gjcc_locAconst(struct Dee_function_generator *__restrict self,
                                      struct Dee_memloc const *lhs, void const *rhs_value,
                                      struct Dee_host_symbol *dst_nz, struct Dee_host_symbol *dst_z) {
	struct Dee_memloc lhs_asreg;
	switch (Dee_memloc_gettyp(lhs)) {
	default:
fallback:
		DO(Dee_function_generator_gasreg(self, lhs, &lhs_asreg, NULL));
		lhs = &lhs_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG: {
		Dee_host_register_t lhs_regno = Dee_memloc_hreg_getreg(lhs);
		ptrdiff_t lhs_valoff = Dee_memloc_hreg_getvaloff(lhs);
		if (lhs_valoff != 0) {
			DO(Dee_function_generator_gmov_regx2reg(self, lhs_regno, lhs_valoff, lhs_regno));
			Dee_memstate_hregs_adjust_delta(self->fg_state, lhs_regno, lhs_valoff);
			/*lhs_valoff = 0;*/
		}
		return Dee_function_generator_gjcc_regAconst(self, lhs_regno, rhs_value, dst_nz, dst_z);
	}	break;

	case MEMADR_TYPE_HREGIND:
		if (Dee_memloc_hregind_getvaloff(lhs) != 0)
			goto fallback;
		return Dee_function_generator_gjcc_regindAconst(self,
		                                                Dee_memloc_hregind_getreg(lhs),
		                                                Dee_memloc_hregind_getindoff(lhs),
		                                                rhs_value, dst_nz, dst_z);

	case MEMADR_TYPE_HSTACKIND:
		if (Dee_memloc_hstackind_getvaloff(lhs) != 0)
			goto fallback;
		return Dee_function_generator_gjcc_hstackindAconst(self,
		                                                   Dee_memloc_hstackind_getcfa(lhs),
		                                                   rhs_value, dst_nz, dst_z);

	}
	__builtin_unreachable();
err:
	return -1;
}




/* Generate jumps. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gjz(struct Dee_function_generator *__restrict self,
                           struct Dee_memloc const *test_loc,
                           struct Dee_host_symbol *__restrict dst) {
	struct Dee_memloc test_loc_asreg;
	switch (Dee_memloc_gettyp(test_loc)) {
	default:
		if unlikely(Dee_function_generator_gasreg(self, test_loc, &test_loc_asreg, NULL))
			goto err;
		test_loc = &test_loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		if (Dee_memloc_hreg_getvaloff(test_loc) == 0)
			return Dee_function_generator_gjz_reg(self, Dee_memloc_hreg_getreg(test_loc), dst);
		return Dee_function_generator_gjcc_regCconst(self, Dee_memloc_hreg_getreg(test_loc),
		                                             (void const *)(uintptr_t)(intptr_t)-Dee_memloc_hreg_getvaloff(test_loc),
		                                             false, NULL, dst, NULL);
	case MEMADR_TYPE_HREGIND:
		if (Dee_memloc_hregind_getvaloff(test_loc) == 0) {
			return Dee_function_generator_gjz_regind(self,
			                                         Dee_memloc_hregind_getreg(test_loc),
			                                         Dee_memloc_hregind_getindoff(test_loc),
			                                         dst);
		}
		return Dee_function_generator_gjcc_regindCconst(self,
		                                                Dee_memloc_hregind_getreg(test_loc),
		                                                Dee_memloc_hregind_getindoff(test_loc),
		                                                (void const *)(uintptr_t)(-Dee_memloc_hregind_getvaloff(test_loc)),
		                                                false, NULL, dst, NULL);
	case MEMADR_TYPE_HSTACKIND:
		if (Dee_memloc_hstackind_getvaloff(test_loc) == 0)
			return Dee_function_generator_gjz_hstackind(self, Dee_memloc_hstackind_getcfa(test_loc), dst);
		return Dee_function_generator_gjcc_hstackindCconst(self, Dee_memloc_hstackind_getcfa(test_loc),
		                                                   (void const *)(uintptr_t)(-Dee_memloc_hstackind_getvaloff(test_loc)),
		                                                   false, NULL, dst, NULL);
	case MEMADR_TYPE_UNDEFINED:
		break;
	case MEMADR_TYPE_CONST:
		if ((uintptr_t)Dee_memloc_const_getaddr(test_loc) == 0)
			return Dee_function_generator_gjmp(self, dst);
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HSTACK: /* Never zero */
		return 0;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gjnz(struct Dee_function_generator *__restrict self,
                            struct Dee_memloc const *test_loc,
                            struct Dee_host_symbol *__restrict dst) {
	struct Dee_memloc test_loc_asreg;
	switch (Dee_memloc_gettyp(test_loc)) {
	default:
		if unlikely(Dee_function_generator_gasreg(self, test_loc, &test_loc_asreg, NULL))
			goto err;
		test_loc = &test_loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		if (Dee_memloc_hreg_getvaloff(test_loc) == 0)
			return Dee_function_generator_gjnz_reg(self, Dee_memloc_hreg_getreg(test_loc), dst);
		return Dee_function_generator_gjcc_regCconst(self, Dee_memloc_hreg_getreg(test_loc),
		                                             (void const *)(uintptr_t)(intptr_t)-Dee_memloc_hreg_getvaloff(test_loc),
		                                             false, dst, NULL, dst);
	case MEMADR_TYPE_HREGIND:
		if (Dee_memloc_hregind_getvaloff(test_loc) == 0) {
			return Dee_function_generator_gjnz_regind(self,
			                                          Dee_memloc_hregind_getreg(test_loc),
			                                          Dee_memloc_hregind_getindoff(test_loc),
			                                          dst);
		}
		return Dee_function_generator_gjcc_regindCconst(self,
		                                                Dee_memloc_hregind_getreg(test_loc),
		                                                Dee_memloc_hregind_getindoff(test_loc),
		                                                (void const *)(uintptr_t)(-Dee_memloc_hregind_getvaloff(test_loc)),
		                                                false, dst, NULL, dst);
	case MEMADR_TYPE_HSTACKIND:
		if (Dee_memloc_hstackind_getvaloff(test_loc) == 0)
			return Dee_function_generator_gjnz_hstackind(self, Dee_memloc_hstackind_getcfa(test_loc), dst);
		return Dee_function_generator_gjcc_hstackindCconst(self, Dee_memloc_hstackind_getcfa(test_loc),
		                                                   (void const *)(uintptr_t)(-Dee_memloc_hstackind_getvaloff(test_loc)),
		                                                   false, dst, NULL, dst);
	case MEMADR_TYPE_UNDEFINED:
		break;
	case MEMADR_TYPE_CONST:
		if ((uintptr_t)Dee_memloc_const_getaddr(test_loc) == 0)
			return 0;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HSTACK: /* Never zero */
		return Dee_function_generator_gjmp(self, dst);
	}
	return 0;
err:
	return -1;
}


/* Emit conditional jump(s) based on `<lhs> <=> <rhs>'
 * NOTE: This function may clobber `lhs' and `rhs', and may flush/shift local/stack locations. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gjcc(struct Dee_function_generator *__restrict self,
                            struct Dee_memloc const *lhs, struct Dee_memloc const *rhs, bool signed_cmp,
                            struct Dee_host_symbol *dst_lo,   /* Jump here if `<lhs> < <rhs>' */
                            struct Dee_host_symbol *dst_eq,   /* Jump here if `<lhs> == <rhs>' */
                            struct Dee_host_symbol *dst_gr) { /* Jump here if `<lhs> > <rhs>' */
	struct Dee_memloc rhs_asreg;

	/* Swap operands if "rhs" isn't CONST or REG, or lhs is CONST */
	if ((Dee_memloc_gettyp(lhs) == MEMADR_TYPE_CONST) ||
	    (Dee_memloc_gettyp(rhs) != MEMADR_TYPE_CONST && Dee_memloc_gettyp(rhs) != MEMADR_TYPE_HREG)) {
#define Tswap(T, a, b) do { T _temp = a; a = b; b = _temp; } __WHILE0
		Tswap(struct Dee_memloc const *, lhs, rhs);
		Tswap(struct Dee_host_symbol *, dst_lo, dst_gr);
#undef Tswap
	}

	/* Special case: if both operands share the same underlying address,
	 *               then the compare is compile-time constant and the
	 *               jump happens based on offset-deltas. */
	if (Dee_memloc_sameadr(lhs, rhs)) {
		ptrdiff_t offset_delta = Dee_memloc_getoff(lhs) -
		                         Dee_memloc_getoff(rhs);
		struct Dee_host_symbol *dst;
		if (offset_delta < 0) {
			dst = dst_lo;
		} else if (offset_delta > 0) {
			dst = dst_gr;
		} else {
			dst = dst_eq;
		}
		return dst ? Dee_function_generator_gjmp(self, dst) : 0;
	}

	/* Branch based on the rhs operand's typing. */
	switch (Dee_memloc_gettyp(rhs)) {
	default: {
		Dee_host_register_t not_these[2];
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (Dee_memloc_hasreg(lhs))
			not_these[0] = Dee_memloc_getreg(lhs);
		if unlikely(Dee_function_generator_gasreg(self, rhs, &rhs_asreg, not_these))
			goto err;
		rhs = &rhs_asreg;
	}	ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gjcc_locCregx(self, lhs,
		                                            Dee_memloc_hreg_getreg(rhs),
		                                            Dee_memloc_hreg_getvaloff(rhs),
		                                            signed_cmp, dst_lo, dst_eq, dst_gr);
	case MEMADR_TYPE_CONST:
		return Dee_function_generator_gjcc_locCconst(self, lhs, Dee_memloc_const_getaddr(rhs),
		                                             signed_cmp, dst_lo, dst_eq, dst_gr);
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_gjca(struct Dee_function_generator *__restrict self,
                            struct Dee_memloc const *lhs, struct Dee_memloc const *rhs,
                            struct Dee_host_symbol *dst_nz, /* Jump here if `(<lhs> & <rhs>) != 0' */
                            struct Dee_host_symbol *dst_z) { /* Jump here if `(<lhs> & <rhs>) == 0' */
	struct Dee_memloc rhs_asreg;

	/* Swap operands if "rhs" isn't CONST or REG, or lhs is CONST */
	if ((Dee_memloc_gettyp(lhs) == MEMADR_TYPE_CONST) ||
	    (Dee_memloc_gettyp(rhs) != MEMADR_TYPE_CONST && Dee_memloc_gettyp(rhs) != MEMADR_TYPE_HREG)) {
#define Tswap(T, a, b) do { T _temp = a; a = b; b = _temp; } __WHILE0
		Tswap(struct Dee_memloc const *, lhs, rhs);
#undef Tswap
	}

	/* Special case: if both operands describe the same location,
	 *               then the result is the same as doing a z/nz
	 *               jump based on either one of the operands. */
	if (Dee_memloc_sameloc(lhs, rhs)) {
		if (dst_nz) {
			if (dst_z) {
				if (dst_nz != dst_z)
					DO(Dee_function_generator_gjnz(self, lhs, dst_nz));
				return Dee_function_generator_gjmp(self, dst_z);
			}
			return Dee_function_generator_gjnz(self, lhs, dst_nz);
		} else if (dst_z) {
			return Dee_function_generator_gjz(self, lhs, dst_z);
		}
		return 0;
	}

	/* Branch based on the rhs operand's typing. */
	switch (Dee_memloc_gettyp(rhs)) {
	default: {
		Dee_host_register_t not_these[2];
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (Dee_memloc_hasreg(lhs))
			not_these[0] = Dee_memloc_getreg(lhs);
		if unlikely(Dee_function_generator_gasreg(self, rhs, &rhs_asreg, not_these))
			goto err;
		rhs = &rhs_asreg;
	}	ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return Dee_function_generator_gjcc_locAregx(self, lhs,
		                                            Dee_memloc_hreg_getreg(rhs),
		                                            Dee_memloc_hreg_getvaloff(rhs),
		                                            dst_nz, dst_z);
	case MEMADR_TYPE_CONST:
		return Dee_function_generator_gjcc_locAconst(self, lhs, Dee_memloc_const_getaddr(rhs),
		                                             dst_nz, dst_z);
	}
	__builtin_unreachable();
err:
	return -1;
}

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_COMMON_C */
