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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_COMMON_C
#define GUARD_DEX_HOSTASM_GENERATOR_COMMON_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/tuple.h>
#include <deemon/util/lock.h> /* Dee_atomic_rwlock_t */

#include <hybrid/compiler.h>
#include <hybrid/overflow.h> /* OVERFLOW_* */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, ptrdiff_t, size_t */
#include <stdint.h>  /* intptr_t, uint16_t, uintptr_t */

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
#ifdef _fungen_gmov_const2regind_MAYFAIL
#define NEED_constasreg
#endif /* _fungen_gmov_const2regind_MAYFAIL */
#ifdef _fungen_gmov_const2hstackind_MAYFAIL
#define NEED_constasreg
#endif /* _fungen_gmov_const2hstackind_MAYFAIL */
#ifdef _host_section_gmov_const2constind_MAYFAIL
#define NEED_constasreg
#endif /* _host_section_gmov_const2constind_MAYFAIL */
#ifdef _fungen_gmov_constind2reg_MAYFAIL
#define NEED_constasreg
#endif /* _fungen_gmov_constind2reg_MAYFAIL */
#ifdef _fungen_gmov_reg2constind_MAYFAIL
#define NEED_constasreg
#endif /* _fungen_gmov_reg2constind_MAYFAIL */
#ifdef _fungen_gbitop_regconst2reg_MAYFAIL
#define NEED_constasreg
#endif /* !_fungen_gbitop_regconst2reg_MAYFAIL */
#ifdef _fungen_gjcc_regindCconst_MAYFAIL
#define NEED_constasreg
#endif /* !_fungen_gjcc_regindCconst_MAYFAIL */
#ifdef _fungen_gjcc_regCconst_MAYFAIL
#define NEED_constasreg
#endif /* !_fungen_gjcc_regCconst_MAYFAIL */
#ifdef _fungen_gjcc_hstackindCconst_MAYFAIL
#define NEED_constasreg
#endif /* !_fungen_gjcc_hstackindCconst_MAYFAIL */
#if defined(HAVE__fungen_gjcc_regindAconst) && defined(_fungen_gjcc_regindAconst_MAYFAIL)
#define NEED_constasreg
#endif /* HAVE__fungen_gjcc_regindAconst && _fungen_gjcc_regindAconst_MAYFAIL */
#if defined(HAVE__fungen_gjcc_regAconst) && defined(_fungen_gjcc_regAconst_MAYFAIL)
#define NEED_constasreg
#endif /* _fungen_gjcc_regAconst && _fungen_gjcc_regAconst_MAYFAIL */
#if defined(HAVE__fungen_gjcc_hstackindAconst) && defined(_fungen_gjcc_hstackindAconst_MAYFAIL)
#define NEED_constasreg
#endif /* HAVE__fungen_gjcc_hstackindAconst && _fungen_gjcc_hstackindAconst_MAYFAIL */


#ifdef NEED_constasreg
#ifdef fg_gmov_const2reg_MAYFAIL
#error "This function is NOT allowed to fail!"
#endif /* fg_gmov_const2reg_MAYFAIL */
INTERN WUNUSED NONNULL((1)) host_regno_t DCALL
fg_gconst_as_reg(struct fungen *__restrict self,
                                     void const *value,
                                     host_regno_t const *not_these) {
	host_regno_t result;
	result = fg_gallocreg(self, not_these);
	if likely(result < HOST_REGNO_COUNT) {
		if unlikely(fg_gmov_const2reg(self, value, result))
			goto err;
	}
	return result;
err:
	return HOST_REGNO_COUNT;
}
#endif /* NEED_constasreg */


/* Bit operations */
INTERN ATTR_CONST WUNUSED uintptr_t DCALL
host_bitop_forconst(host_bitop_t op, uintptr_t lhs, uintptr_t rhs) {
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
host_arithop_forconst(host_arithop_t op,
                      uintptr_t lhs, uintptr_t rhs,
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
fg_gnotoneref_impl(struct fungen *__restrict self,
                   struct memobj *mobj) {
	struct memval *alias_mval;
	ASSERT(mobj->mo_flags & MEMOBJ_F_ONEREF);
	memstate_foreach(alias_mval, self->fg_state) {
		struct memobj *alias_mobj;
		memval_foreach_obj(alias_mobj, alias_mval) {
			if (memobj_sameloc(alias_mobj, mobj))
				memobj_clearoneref(alias_mobj);
		}
		memval_foreach_obj_end;
	}
	memstate_foreach_end;
	/* TODO: Must also forget anything related to equivalences when it comes to
	 *       the indirection of "mobj":
	 * >> local x = [10, 20];
	 * >> foo(x); // When this clears the ONEREF flag, DeeList_ELEM(x) is no longer
	 * >>         // known (and transitively also DeeList_GET(x, 0), ...)
	 *
	 * XXX: Is this actually a problem? indirection requires registers, but since
	 *      we never use callee-preserve registers, *all* registers are always,
	 *      already marked as clobbered after a function call... */
	memobj_clearoneref(mobj);
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
memstate_remember_undefined_hstack_after_redzone(struct memstate *__restrict self) {
	host_cfa_t min_undef_cfa;
	min_undef_cfa = self->ms_host_cfa_offset + HOSTASM_REDZONE_SIZE;
#ifdef HOSTASM_STACK_GROWS_DOWN
	min_undef_cfa += HOST_SIZEOF_POINTER;
#endif /* HOSTASM_STACK_GROWS_DOWN */
	memequivs_undefined_hstackind_after(&self->ms_memequiv, min_undef_cfa);
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_ghstack_adjust(struct fungen *__restrict self,
                  ptrdiff_t cfa_delta) {
	int result;
	if (cfa_delta == 0)
		return 0;
#ifdef HOSTASM_STACK_GROWS_DOWN
	result = _fungen_ghstack_adjust(self, -cfa_delta);
#else /* HOSTASM_STACK_GROWS_DOWN */
	result = _fungen_ghstack_adjust(self, cfa_delta);
#endif /* !HOSTASM_STACK_GROWS_DOWN */
	if likely(result == 0) {
		if (cfa_delta < 0) {
			/* Remember that values beyond the stack's red zone become undefined,
			 * as they might get clobbered by sporadic interrupt handlers. */
			if (cfa_delta == -HOST_SIZEOF_POINTER) {
#ifdef HOSTASM_STACK_GROWS_DOWN
				host_cfa_t pop_dst_cfa_offset = self->fg_state->ms_host_cfa_offset + HOST_SIZEOF_POINTER;
#else /* HOSTASM_STACK_GROWS_DOWN */
				host_cfa_t pop_dst_cfa_offset = self->fg_state->ms_host_cfa_offset;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
				fg_remember_undefined_hstackind(self, pop_dst_cfa_offset + HOSTASM_REDZONE_SIZE);
			} else {
				memstate_remember_undefined_hstack_after_redzone(self->fg_state);
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
fg_ghstack_pushreg(struct fungen *__restrict self,
                   host_regno_t src_regno) {
	host_cfa_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	int result;
#if HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER
	/* TODO: If "[#dst_cfa_offset]" is a known equivalence of "src_regno", do nothing */
#endif /* HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER */
	result = _fungen_ghstack_pushreg(self, src_regno);
	if likely(result == 0)
		result = fg_remember_movevalue_reg2hstackind(self, src_regno, dst_cfa_offset);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_ghstack_pushregind(struct fungen *__restrict self,
                      host_regno_t src_regno, ptrdiff_t src_delta) {
	host_cfa_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	int result;
#if HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER
	/* TODO: If "[#dst_cfa_offset]" is a known equivalence of "[src_regno + src_delta]", do nothing */
#endif /* HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER */
	result = _fungen_ghstack_pushregind(self, src_regno, src_delta);
	if likely(result == 0)
		result = fg_remember_movevalue_regind2hstackind(self, src_regno, src_delta, dst_cfa_offset);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_ghstack_pushconst(struct fungen *__restrict self,
                     void const *value) {
	host_cfa_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	int result;
#if HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER
	/* TODO: If "[#dst_cfa_offset]" is a known equivalence of "value", do nothing */
#endif /* HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER */
	result = _fungen_ghstack_pushconst(self, value);
	if likely(result == 0)
		result = fg_remember_movevalue_const2hstackind(self, value, dst_cfa_offset);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_ghstack_pushhstackind(struct fungen *__restrict self,
                         host_cfa_t cfa_offset) {
	host_cfa_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	ptrdiff_t sp_offset = memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	int result;
#if HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER
	/* TODO: If "[#dst_cfa_offset]" is a known equivalence of "[#cfa_offset]", do nothing */
#endif /* HOSTASM_REDZONE_SIZE >= HOST_SIZEOF_POINTER */
	result = _fungen_ghstack_pushhstackind(self, sp_offset);
	if likely(result == 0)
		result = fg_remember_movevalue_hstackind2hstackind(self, cfa_offset, dst_cfa_offset);
	return result;
}

#ifdef HAVE_fg_ghstack_pushhstack_at_cfa_boundary_np
INTERN WUNUSED NONNULL((1)) int DCALL
fg_ghstack_pushhstack_at_cfa_boundary_np(struct fungen *__restrict self) {
	host_cfa_t src_cfa_offset = GET_ADDRESS_OF_NEXT_POP(self);
	host_cfa_t dst_cfa_offset = GET_ADDRESS_OF_NEXT_PUSH(self);
	int result = _fungen_ghstack_pushhstack_at_cfa_boundary_np(self);
	if likely(result == 0) {
		(void)src_cfa_offset;
		(void)dst_cfa_offset;
		result = fg_remember_movevalue_hstack2hstackind(self, src_cfa_offset, dst_cfa_offset);
	}
	return result;
}
#endif /* HAVE_fg_ghstack_pushhstack_at_cfa_boundary_np */

INTERN WUNUSED NONNULL((1)) int DCALL
fg_ghstack_popreg(struct fungen *__restrict self,
                  host_regno_t dst_regno) {
	host_cfa_t src_cfa_offset = GET_ADDRESS_OF_NEXT_POP(self);
	int result;
	/* TODO: If "dst_regno" is a known equivalence of "[#src_cfa_offset]", do nothing */
	result = _fungen_ghstack_popreg(self, dst_regno);
	if likely(result == 0) {
		result = fg_remember_movevalue_hstackind2reg(self, src_cfa_offset, dst_regno);
		fg_remember_undefined_hstackind(self, src_cfa_offset + HOSTASM_REDZONE_SIZE);
	}
	return result;
}

#undef GET_ADDRESS_OF_NEXT_PUSH
#undef GET_ADDRESS_OF_NEXT_POP

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmov_reg2hstackind(struct fungen *__restrict self,
                      host_regno_t src_regno, host_cfa_t cfa_offset) {
	ptrdiff_t sp_offset;
	int result;
	/* TODO: If "[#cfa_offset]" is a known equivalence of "src_regno", do nothing */

	/* Check if the value *must* be pushed onto the h-stack. */
	if (memstate_hstack_mustpush(self->fg_state, cfa_offset)) {
		host_cfa_t skip = memstate_hstack_mustpush_skip(self->fg_state, cfa_offset);
		ASSERT((skip == 0) == !!memstate_hstack_canpush(self->fg_state, cfa_offset));
		result = fg_ghstack_adjust(self, (ptrdiff_t)skip);
		if likely(result == 0)
			result = fg_ghstack_pushreg(self, src_regno);
		return result;
	}
	sp_offset = memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	result = _fungen_gmov_reg2hstackind(self, src_regno, sp_offset);
	if likely(result == 0)
		result = fg_remember_movevalue_reg2hstackind(self, src_regno, cfa_offset);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmov_hstack2reg(struct fungen *__restrict self,
                   host_cfa_t cfa_offset, host_regno_t dst_regno) {
	ptrdiff_t sp_offset = memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	int result = _fungen_gmov_hstack2reg(self, sp_offset, dst_regno);
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = HOST_REGUSAGE_GENERIC;
		result = fg_remember_movevalue_hstack2reg(self, cfa_offset, dst_regno);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmov_hstackind2reg(struct fungen *__restrict self,
                      host_cfa_t cfa_offset, host_regno_t dst_regno) {
	/* TODO: If "dst_regno" is a known equivalence of "[#cfa_offset]", do nothing */

	/* Special case: if the value lies on-top of the host stack, then pop it instead of move it. */
	if (memstate_hstack_canpop(self->fg_state, cfa_offset)) {
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
		struct memval *val;
		struct memobj *obj, *cfa_objs = NULL;
		memstate_foreach(val, self->fg_state) {
			memval_foreach_obj(obj, val) {
				if (memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND &&
				    memobj_hstackind_getcfa(obj) == cfa_offset) {
					STATIC_ASSERT(offsetof(struct memobj, mo_loc.ml_adr.ma_val.v_cfa) ==
					              offsetof(struct memobj, mo_loc.ml_adr.ma_val._v_nextobj));
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
			memval_foreach_obj_end;
		}
		memstate_foreach_end;
		if (cfa_objs != NULL) {
			struct memobj *next;
			do {
				next = cfa_objs->mo_loc.ml_adr.ma_val._v_nextobj;
				memloc_init_hreg(&cfa_objs->mo_loc, dst_regno, memloc_getoff(&cfa_objs->mo_loc));
				memstate_incrinuse(self->fg_state, dst_regno);
			} while ((cfa_objs = next) != NULL);
		}
		return fg_ghstack_popreg(self, dst_regno);
	}
do_use_mov:
	return fg_gmov_hstackind2reg_nopop(self, cfa_offset, dst_regno);
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmov_hstackind2reg_nopop(struct fungen *__restrict self,
                            host_cfa_t cfa_offset, host_regno_t dst_regno) {
	int result;
	ptrdiff_t sp_offset;
	/* TODO: If "dst_regno" is a known equivalence of "[#cfa_offset]", do nothing */
	sp_offset = memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	result    = _fungen_gmov_hstackind2reg(self, sp_offset, dst_regno);
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = HOST_REGUSAGE_GENERIC;
		result = fg_remember_movevalue_hstackind2reg(self, cfa_offset, dst_regno);
	}
	return result;
}


INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmov_const2reg(struct fungen *__restrict self,
                  void const *value, host_regno_t dst_regno) {
	/* TODO: If "dst_regno" is a known equivalence of "value", do nothing */
	int result = _fungen_gmov_const2reg(self, value, dst_regno);
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = HOST_REGUSAGE_GENERIC;
		result = fg_remember_movevalue_const2reg(self, value, dst_regno);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmov_const2regind(struct fungen *__restrict self,
                     void const *value, host_regno_t dst_regno,
                     ptrdiff_t dst_delta) {
	int result = _fungen_gmov_const2regind(self, value, dst_regno, dst_delta);
#ifdef _fungen_gmov_const2regind_MAYFAIL
	if unlikely(result > 0) {
		host_regno_t valreg;
		host_regno_t not_these[2];
		not_these[0] = dst_regno;
		not_these[1] = HOST_REGNO_COUNT;
		valreg = fg_gconst_as_reg(self, value, not_these);
		if unlikely(valreg >= HOST_REGNO_COUNT)
			return -1;
		return fg_gmov_reg2regind(self, valreg, dst_regno, dst_delta);
	}
#endif /* _fungen_gmov_const2regind_MAYFAIL */
	if likely(result == 0)
		result = fg_remember_movevalue_const2regind(self, value, dst_regno, dst_delta);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmov_const2hstackind(struct fungen *__restrict self,
                        void const *value, host_cfa_t cfa_offset) {
	ptrdiff_t sp_offset;
	int result;
	/* TODO: If "[#cfa_offset]" is a known equivalence of "value", do nothing */

	/* Check if the value *must* be pushed onto the h-stack. */
	if (memstate_hstack_mustpush(self->fg_state, cfa_offset)) {
		ptrdiff_t skip = memstate_hstack_mustpush_skip(self->fg_state, cfa_offset);
		ASSERT((skip == 0) == !!memstate_hstack_canpush(self->fg_state, cfa_offset));
		result = fg_ghstack_adjust(self, skip);
		if likely(result == 0)
			result = fg_ghstack_pushconst(self, value);
		return result;
	}
	sp_offset = memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	result = _fungen_gmov_const2hstackind(self, value, sp_offset);
#ifdef _fungen_gmov_const2hstackind_MAYFAIL
	if unlikely(result > 0) {
		host_regno_t valreg;
		valreg = fg_gconst_as_reg(self, value, NULL);
		if unlikely(valreg >= HOST_REGNO_COUNT)
			return -1;
		return fg_gmov_reg2hstackind(self, valreg, cfa_offset);
	}
#endif /* _fungen_gmov_const2hstackind_MAYFAIL */
	if likely(result == 0)
		result = fg_remember_movevalue_const2hstackind(self, value, cfa_offset);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmov_const2constind(struct fungen *__restrict self,
                       void const *value, void const **p_value) {
	int result = _fungen_gmov_const2constind(self, value, p_value);
#ifdef _host_section_gmov_const2constind_MAYFAIL
	if unlikely(result > 0) {
		host_regno_t valreg;
		ASSERT(result == 1 || result == 2);
		valreg = fg_gconst_as_reg(self, result == 1 ? value : (void const *)p_value, NULL);
		if unlikely(valreg >= HOST_REGNO_COUNT)
			return -1;
		return result == 1 ? fg_gmov_reg2constind(self, valreg, p_value)
		                   : fg_gmov_const2regind(self, value, valreg, 0);
	}
#endif /* _host_section_gmov_const2constind_MAYFAIL */
	if likely(result == 0)
		result = fg_remember_movevalue_const2constind(self, value, p_value);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmov_reg2reg(struct fungen *__restrict self,
                host_regno_t src_regno, host_regno_t dst_regno) {
	/* TODO: If "dst_regno" is a known equivalence of "src_regno", do nothing */
	int result = _fungen_gmov_reg2reg(self, src_regno, dst_regno);
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = self->fg_state->ms_rusage[src_regno];
		result = fg_remember_movevalue_reg2reg(self, src_regno, dst_regno);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmov_regx2reg(struct fungen *__restrict self,
                 host_regno_t src_regno, ptrdiff_t src_delta,
                 host_regno_t dst_regno) {
	/* TODO: If "dst_regno" is a known equivalence of "src_regno + src_delta", do nothing */
	int result = _fungen_gmov_regx2reg(self, src_regno, src_delta, dst_regno);
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = self->fg_state->ms_rusage[src_regno];
		if (src_delta != 0)
			self->fg_state->ms_rusage[dst_regno] = HOST_REGUSAGE_GENERIC;
		result = fg_remember_movevalue_regx2reg(self, src_regno, src_delta, dst_regno);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmov_regind2reg(struct fungen *__restrict self,
                   host_regno_t src_regno, ptrdiff_t src_delta,
                   host_regno_t dst_regno) {
	/* TODO: If "dst_regno" is a known equivalence of "[src_regno + src_delta]", do nothing */
	int result = _fungen_gmov_regind2reg(self, src_regno, src_delta, dst_regno);
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = HOST_REGUSAGE_GENERIC;
		result = fg_remember_movevalue_regind2reg(self, src_regno, src_delta, dst_regno);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmov_reg2regind(struct fungen *__restrict self,
                   host_regno_t src_regno,
                   host_regno_t dst_regno, ptrdiff_t dst_delta) {
	int result = _fungen_gmov_reg2regind(self, src_regno, dst_regno, dst_delta);
	if likely(result == 0)
		result = fg_remember_movevalue_reg2regind(self, src_regno, dst_regno, dst_delta);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmov_constind2reg(struct fungen *__restrict self,
                     void const **p_value, host_regno_t dst_regno) {
	int result = _fungen_gmov_constind2reg(self, p_value, dst_regno);
#ifdef _fungen_gmov_constind2reg_MAYFAIL
	if unlikely(result > 0) {
		host_regno_t valreg;
		valreg = fg_gconst_as_reg(self, (void const *)p_value, NULL);
		if unlikely(valreg >= HOST_REGNO_COUNT)
			return -1;
		return fg_gmov_regind2reg(self, valreg, 0, dst_regno);
	}
#endif /* _fungen_gmov_constind2reg_MAYFAIL */
	if likely(result == 0) {
		self->fg_state->ms_rusage[dst_regno] = HOST_REGUSAGE_GENERIC;
		result = fg_remember_movevalue_constind2reg(self, p_value, dst_regno);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmov_reg2constind(struct fungen *__restrict self,
                     host_regno_t src_regno, void const **p_value) {
	int result = _fungen_gmov_reg2constind(self, src_regno, p_value);
#ifdef _fungen_gmov_reg2constind_MAYFAIL
	if unlikely(result > 0) {
		host_regno_t valreg;
		host_regno_t not_these[2];
		not_these[0] = src_regno;
		not_these[1] = HOST_REGNO_COUNT;
		valreg = fg_gconst_as_reg(self, (void const *)p_value, not_these);
		if unlikely(valreg >= HOST_REGNO_COUNT)
			return -1;
		return fg_gmov_reg2regind(self, src_regno, valreg, 0);
	}
#endif /* _fungen_gmov_reg2constind_MAYFAIL */
	if likely(result == 0)
		result = fg_remember_movevalue_reg2constind(self, src_regno, p_value);
	return result;
}


/* dst_regno = src1_regno <op> src2_regno; */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gbitop_regreg2reg(struct fungen *__restrict self, host_bitop_t op,
                     host_regno_t src1_regno, host_regno_t src2_regno,
                     host_regno_t dst_regno) {
	int result = _fungen_gbitop_regreg2reg(self, op, src1_regno, src2_regno, dst_regno);
	if likely(result == 0)
		fg_remember_undefined_reg(self, dst_regno);
	return result;
}

/* dst_regno = src1_regno <op> *(src2_regno + src2_ind_delta); */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gbitop_regregind2reg(struct fungen *__restrict self, host_bitop_t op,
                        host_regno_t src1_regno, host_regno_t src2_regno,
                        ptrdiff_t src2_ind_delta, host_regno_t dst_regno) {
	int result = _fungen_gbitop_regregind2reg(self, op, src1_regno, src2_regno, src2_ind_delta, dst_regno);
	if likely(result == 0)
		fg_remember_undefined_reg(self, dst_regno);
	return result;
}

/* dst_regno = src1_regno <op> *(SP ... src2_cfa_offset); */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gbitop_reghstackind2reg(struct fungen *__restrict self, host_bitop_t op,
                           host_regno_t src1_regno, host_cfa_t src2_cfa_offset,
                           host_regno_t dst_regno) {
	ptrdiff_t src2_sp_offset = memstate_hstack_cfa2sp(self->fg_state, src2_cfa_offset);
	int result = _fungen_gbitop_reghstackind2reg(self, op, src1_regno, src2_sp_offset, dst_regno);
	if likely(result == 0)
		fg_remember_undefined_reg(self, dst_regno);
	return result;
}


#ifdef _fungen_gbitop_regconst2reg_MAYFAIL
/* dst_regno = src_regno <op> value; */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gbitop_regconst2reg(struct fungen *__restrict self, host_bitop_t op,
                       host_regno_t src_regno, void const *value,
                       host_regno_t dst_regno) {
	int result = _fungen_gbitop_regconst2reg(self, op, src_regno, value, dst_regno);
	if unlikely(result > 0) {
		host_regno_t not_these[3], tempreg;
		not_these[0] = src_regno;
		not_these[1] = dst_regno;
		not_these[2] = HOST_REGNO_COUNT;
		tempreg = fg_gconst_as_reg(self, value, not_these);
		if unlikely(tempreg >= HOST_REGNO_COUNT)
			return -1;
		return fg_gbitop_regreg2reg(self, op, src_regno, tempreg, dst_regno);
	}
	if likely(result == 0)
		fg_remember_undefined_reg(self, dst_regno);
	return result;
}
#endif /* _fungen_gbitop_regconst2reg_MAYFAIL */

PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_gbitop_regxregx2reg(struct fungen *__restrict self, host_bitop_t op,
                       host_regno_t src1_regno, ptrdiff_t src_regno1_off,
                       host_regno_t src2_regno, ptrdiff_t src_regno2_off,
                       host_regno_t dst_regno) {
	if (src_regno1_off == 0 && src_regno2_off == 0) /* Both sides have value-offset = 0 */
		return fg_gbitop_regreg2reg(self, op, src1_regno, src2_regno, dst_regno);
	if (src_regno1_off == 0 && dst_regno != src1_regno) {
		/* Left side has value-offset == 0, and destination isn't src1. Encode as:
		 * >> dst_regno = src2_regno + src_regno2_off;
		 * >> dst_regno = src1_regno & dst_regno; */
		DO(fg_gmov_regx2reg(self, src2_regno, src_regno2_off, dst_regno));
		return fg_gbitop_regreg2reg(self, op, src1_regno, dst_regno, dst_regno);
	}
	if (src_regno2_off == 0 && dst_regno != src2_regno) {
		/* Right side has value-offset == 0, and destination isn't src2. Encode as:
		 * >> dst_regno = src1_regno + src_regno1_off;
		 * >> dst_regno = dst_regno & src2_regno; */
		DO(fg_gmov_regx2reg(self, src1_regno, src_regno1_off, dst_regno));
		return fg_gbitop_regreg2reg(self, op, dst_regno, src2_regno, dst_regno);
	}

	/* Fallback: inline-adjust */
	if (src_regno1_off != 0) {
		host_regno_t new_src_regno1 = src1_regno;
		if (new_src_regno1 == src2_regno)
			new_src_regno1 = dst_regno;
		DO(fg_gmov_regx2reg(self, src1_regno, src_regno1_off, new_src_regno1));
		memstate_hregs_adjust_delta(self->fg_state, new_src_regno1, src_regno1_off);
		if (src2_regno == new_src_regno1)
			src_regno2_off += src_regno1_off;
		src1_regno = new_src_regno1;
		/*src_regno1_off = 0;*/
	}
	if (src_regno2_off != 0) {
		if (src1_regno != src2_regno) {
			DO(fg_gmov_regx2reg(self, src2_regno, src_regno2_off, src2_regno));
			/*src_regno2_off = 0;*/
		} else if (src1_regno != dst_regno) {
			DO(fg_gmov_regx2reg(self, src2_regno, src_regno2_off, dst_regno));
			src2_regno = dst_regno;
			/*src_regno2_off = 0;*/
		} else {
			host_regno_t tempreg;
			host_regno_t not_these[2];
			not_these[0] = src2_regno;
			not_these[1] = HOST_REGNO_COUNT;
			tempreg = fg_gallocreg(self, not_these);
			if unlikely(tempreg >= HOST_REGNO_COUNT)
				goto err;
			DO(fg_gmov_regx2reg(self, src2_regno, src_regno2_off, tempreg));
			src2_regno = tempreg;
			/*src_regno2_off = 0;*/
		}
	}
	return fg_gbitop_regreg2reg(self, op, src1_regno, src2_regno, dst_regno);
err:
	return -1;
}

/* dst_regno = src_loc1 <op> src_loc2; */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gbitop_locloc2reg(struct fungen *__restrict self, host_bitop_t op,
                     struct memloc const *src_loc1, struct memloc const *src_loc2,
                     host_regno_t dst_regno) {
	struct memloc loc1_asreg, loc2_asreg;
	host_regno_t not_these[2];
	if (memloc_gettyp(src_loc2) == MEMADR_TYPE_HREG ||
	    memloc_gettyp(src_loc1) == MEMADR_TYPE_CONST) {
		/* Always want the constant to appear on the *right*
		 * side, and a register to appear on the left side. */
		struct memloc const *temp;
		temp = src_loc2;
		src_loc2 = src_loc1;
		src_loc1 = temp;
	}

	if (memloc_sameloc(src_loc1, src_loc2)) {
		if (op == BITOP_XOR)
			return fg_gmov_const2reg(self, (void const *)0, dst_regno);
		return fg_gmov_loc2reg(self, src_loc1, dst_regno);
	}
	if unlikely(memloc_gettyp(src_loc1) == MEMADR_TYPE_UNDEFINED)
		return 0;
	switch (memloc_gettyp(src_loc2)) {

	case MEMADR_TYPE_HSTACKIND: {
		host_cfa_t src_loc2_cfa = memloc_hstackind_getcfa(src_loc2);
		if (memloc_hstackind_getvaloff(src_loc2) != 0)
			goto src_loc2_fallback;
		switch (memloc_gettyp(src_loc1)) {
		default:
			DO(fg_gmov_loc2reg(self, src_loc1, dst_regno));
			memloc_init_hreg(&loc1_asreg, dst_regno, 0);
			src_loc1 = &loc1_asreg;
			ATTR_FALLTHROUGH
		case MEMADR_TYPE_HREG:
			if (memloc_hreg_getvaloff(src_loc1) != 0) {
				if unlikely(fg_gmov_regx2reg(self,
				                             memloc_hreg_getreg(src_loc1),
				                             memloc_hreg_getvaloff(src_loc1),
				                             dst_regno))
					goto err;
				return fg_gbitop_reghstackind2reg(self, op, dst_regno, src_loc2_cfa, dst_regno);
			}
			return fg_gbitop_reghstackind2reg(self, op, memloc_hreg_getreg(src_loc1),
			                                  src_loc2_cfa, dst_regno);
		}
	}	break;

	case MEMADR_TYPE_HREGIND: {
		host_regno_t src_loc2_regno = memloc_hregind_getreg(src_loc2);
		ptrdiff_t src_loc2_indoff = memloc_hregind_getindoff(src_loc2);
		if (memloc_hregind_getvaloff(src_loc2) != 0)
			goto src_loc2_fallback;
		switch (memloc_gettyp(src_loc1)) {
		default:
			DO(fg_gmov_loc2reg(self, src_loc1, dst_regno));
			memloc_init_hreg(&loc1_asreg, dst_regno, 0);
			src_loc1 = &loc1_asreg;
			ATTR_FALLTHROUGH
		case MEMADR_TYPE_HREG:
			if (memloc_hreg_getvaloff(src_loc1) != 0) {
				if unlikely(fg_gmov_regx2reg(self,
				                             memloc_hreg_getreg(src_loc1),
				                             memloc_hreg_getvaloff(src_loc1),
				                             dst_regno))
					goto err;
				return fg_gbitop_regregind2reg(self, op, dst_regno, src_loc2_regno,
				                               src_loc2_indoff, dst_regno);
			}
			return fg_gbitop_regregind2reg(self, op, memloc_hreg_getreg(src_loc1),
			                               src_loc2_regno, src_loc2_indoff, dst_regno);
		}
	}	break;

	default:
src_loc2_fallback:
		not_these[0] = HOST_REGNO_COUNT;
		not_these[1] = HOST_REGNO_COUNT;
		if (memloc_hasreg(src_loc1))
			not_these[0] = memloc_getreg(src_loc1);
		DO(fg_gasreg(self, src_loc2, &loc2_asreg, not_these));
		src_loc2 = &loc2_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		if (memloc_gettyp(src_loc1) != MEMADR_TYPE_HREG) {
			not_these[0] = memloc_getreg(src_loc2);
			not_these[1] = HOST_REGNO_COUNT;
			DO(fg_gasreg(self, src_loc1, &loc1_asreg, not_these));
			src_loc1 = &loc1_asreg;
		}
		ASSERT(memloc_gettyp(src_loc1) == MEMADR_TYPE_HREG);
		ASSERT(memloc_gettyp(src_loc2) == MEMADR_TYPE_HREG);
		return fg_gbitop_regxregx2reg(self, op,
		                              memloc_hreg_getreg(src_loc1),
		                              memloc_hreg_getvaloff(src_loc1),
		                              memloc_hreg_getreg(src_loc2),
		                              memloc_hreg_getvaloff(src_loc2),
		                              dst_regno);

	case MEMADR_TYPE_CONST: {
		void const *src_loc2_value = memloc_const_getaddr(src_loc2);
		switch (memloc_gettyp(src_loc1)) {
		default:
			DO(fg_gmov_loc2reg(self, src_loc1, dst_regno));
			memloc_init_hreg(&loc1_asreg, dst_regno, 0);
			src_loc1 = &loc1_asreg;
			ATTR_FALLTHROUGH
		case MEMADR_TYPE_HREG:
			if (memloc_hreg_getvaloff(src_loc1) != 0) {
				if unlikely(fg_gmov_regx2reg(self,
				                             memloc_hreg_getreg(src_loc1),
				                             memloc_hreg_getvaloff(src_loc1),
				                             dst_regno))
					goto err;
				return fg_gbitop_regconst2reg(self, op, dst_regno, src_loc2_value, dst_regno);
			}
			return fg_gbitop_regconst2reg(self, op, memloc_hreg_getreg(src_loc1),
			                              src_loc2_value, dst_regno);
		case MEMADR_TYPE_CONST: {
			void const *src_loc1_value = memloc_const_getaddr(src_loc1);
			void const *dst_value = (void const *)host_bitop_forconst(op, (uintptr_t)src_loc1_value, (uintptr_t)src_loc2_value);
			return fg_gmov_const2reg(self, dst_value, dst_regno);
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
fg_gjarith_regreg2reg(struct fungen *__restrict self, host_bitop_t op,
                      host_regno_t src1_regno, host_regno_t src2_regno,
                      host_regno_t dst_regno,
                      struct host_symbol *dst_o, struct host_symbol *dst_no) {
	int result = _fungen_gjarith_regreg2reg(self, op, src1_regno, src2_regno, dst_regno, dst_o, dst_no);
	if likely(result == 0)
		fg_remember_undefined_reg(self, dst_regno);
	return result;
}

/* dst_regno = src1_regno <op> *(src2_regno + src2_ind_delta); */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjarith_regregind2reg(struct fungen *__restrict self, host_bitop_t op,
                         host_regno_t src1_regno, host_regno_t src2_regno,
                         ptrdiff_t src2_ind_delta, host_regno_t dst_regno,
                         struct host_symbol *dst_o, struct host_symbol *dst_no) {
	int result = _fungen_gjarith_regregind2reg(self, op, src1_regno, src2_regno, src2_ind_delta, dst_regno, dst_o, dst_no);
	if likely(result == 0)
		fg_remember_undefined_reg(self, dst_regno);
	return result;
}

/* dst_regno = src1_regno <op> *(SP ... src2_cfa_offset); */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjarith_reghstackind2reg(struct fungen *__restrict self, host_bitop_t op,
                            host_regno_t src1_regno, host_cfa_t src2_cfa_offset,
                            host_regno_t dst_regno,
                            struct host_symbol *dst_o, struct host_symbol *dst_no) {
	ptrdiff_t src2_sp_offset = memstate_hstack_cfa2sp(self->fg_state, src2_cfa_offset);
	int result = _fungen_gjarith_reghstackind2reg(self, op, src1_regno, src2_sp_offset, dst_regno, dst_o, dst_no);
	if likely(result == 0)
		fg_remember_undefined_reg(self, dst_regno);
	return result;
}


#ifdef _fungen_gjarith_regconst2reg_MAYFAIL
/* dst_regno = src_regno <op> value; */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjarith_regconst2reg(struct fungen *__restrict self, host_bitop_t op,
                        host_regno_t src_regno, void const *value,
                        host_regno_t dst_regno,
                        struct host_symbol *dst_o, struct host_symbol *dst_no) {
	int result = _fungen_gjarith_regconst2reg(self, op, src_regno, value, dst_regno, dst_o, dst_no);
	if unlikely(result > 0) {
		host_regno_t not_these[3], tempreg;
		not_these[0] = src_regno;
		not_these[1] = dst_regno;
		not_these[2] = HOST_REGNO_COUNT;
		tempreg = fg_gconst_as_reg(self, value, not_these);
		if unlikely(tempreg >= HOST_REGNO_COUNT)
			return -1;
		return fg_gjarith_regreg2reg(self, op, src_regno, tempreg, dst_regno, dst_o, dst_no);
	}
	if likely(result == 0)
		fg_remember_undefined_reg(self, dst_regno);
	return result;
}
#endif /* _fungen_gjarith_regconst2reg_MAYFAIL */

PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_gjarith_regxregx2reg(struct fungen *__restrict self, host_bitop_t op,
                        host_regno_t src1_regno, ptrdiff_t src_regno1_off,
                        host_regno_t src2_regno, ptrdiff_t src_regno2_off,
                        host_regno_t dst_regno,
                        struct host_symbol *dst_o, struct host_symbol *dst_no) {
	if (src_regno1_off == 0 && src_regno2_off == 0) /* Both sides have value-offset = 0 */
		return fg_gjarith_regreg2reg(self, op, src1_regno, src2_regno, dst_regno, dst_o, dst_no);
	if (src1_regno == src2_regno && ARITHOP_MAYMOVEOFF(op)) {
		src_regno2_off -= src_regno1_off;
		src_regno1_off = 0;
	}
	if (src_regno1_off == 0 && dst_regno != src1_regno) {
		/* Left side has value-offset == 0, and destination isn't src1. Encode as:
		 * >> dst_regno = src2_regno + src_regno2_off;
		 * >> dst_regno = src1_regno & dst_regno; */
		DO(fg_gmov_regx2reg(self, src2_regno, src_regno2_off, dst_regno));
		return fg_gjarith_regreg2reg(self, op, src1_regno, dst_regno, dst_regno, dst_o, dst_no);
	}
	if (src_regno2_off == 0 && dst_regno != src2_regno) {
		/* Right side has value-offset == 0, and destination isn't src2. Encode as:
		 * >> dst_regno = src1_regno + src_regno1_off;
		 * >> dst_regno = dst_regno & src2_regno; */
		DO(fg_gmov_regx2reg(self, src1_regno, src_regno1_off, dst_regno));
		return fg_gjarith_regreg2reg(self, op, dst_regno, src2_regno, dst_regno, dst_o, dst_no);
	}

	/* Fallback: inline-adjust */
	if (src_regno1_off != 0) {
		host_regno_t new_src_regno1 = src1_regno;
		if (new_src_regno1 == src2_regno)
			new_src_regno1 = dst_regno;
		DO(fg_gmov_regx2reg(self, src1_regno, src_regno1_off, new_src_regno1));
		memstate_hregs_adjust_delta(self->fg_state, new_src_regno1, src_regno1_off);
		if (src2_regno == new_src_regno1)
			src_regno2_off += src_regno1_off;
		src1_regno = new_src_regno1;
		/*src_regno1_off = 0;*/
	}
	if (src_regno2_off != 0) {
		if (src1_regno != src2_regno) {
			DO(fg_gmov_regx2reg(self, src2_regno, src_regno2_off, src2_regno));
			/*src_regno2_off = 0;*/
		} else if (src1_regno != dst_regno) {
			DO(fg_gmov_regx2reg(self, src2_regno, src_regno2_off, dst_regno));
			src2_regno = dst_regno;
			/*src_regno2_off = 0;*/
		} else {
			host_regno_t tempreg;
			host_regno_t not_these[2];
			not_these[0] = src2_regno;
			not_these[1] = HOST_REGNO_COUNT;
			tempreg = fg_gallocreg(self, not_these);
			if unlikely(tempreg >= HOST_REGNO_COUNT)
				goto err;
			DO(fg_gmov_regx2reg(self, src2_regno, src_regno2_off, tempreg));
			src2_regno = tempreg;
			/*src_regno2_off = 0;*/
		}
	}
	return fg_gjarith_regreg2reg(self, op, src1_regno, src2_regno, dst_regno, dst_o, dst_no);
err:
	return -1;
}

/* dst_regno = src_loc1 <op> src_loc2; */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjarith_locloc2reg(struct fungen *__restrict self, host_bitop_t op,
                      struct memloc const *src_loc1, struct memloc const *src_loc2,
                      host_regno_t dst_regno,
                      struct host_symbol *dst_o, struct host_symbol *dst_no) {
	struct memloc loc1_asnormal, loc2_asnormal;
	struct memloc loc1_asreg, loc2_asreg;
	host_regno_t not_these[2];
	if (memloc_gettyp(src_loc2) == MEMADR_TYPE_HREG ||
	    memloc_gettyp(src_loc1) == MEMADR_TYPE_CONST) {
		if (ARITHOP_MAYREORDER(op)) {
			/* Always want the constant to appear on the *right*
			 * side, and a register to appear on the left side. */
			struct memloc const *temp;
			temp = src_loc2;
			src_loc2 = src_loc1;
			src_loc1 = temp;
		} else if (memloc_gettyp(src_loc1) == MEMADR_TYPE_CONST) {
			switch (op) {

			case ARITHOP_USUB:
			case ARITHOP_SSUB: {
				intptr_t negval = -(intptr_t)(uintptr_t)memloc_const_getaddr(src_loc1);
				src_loc1 = src_loc2;
				src_loc2 = &loc2_asnormal;
				memloc_init_const(&loc2_asnormal, (void *)(uintptr_t)negval);
				op = op == ARITHOP_USUB ? ARITHOP_UADD : ARITHOP_SADD;
			}	break;

			default: break;
			}
		}
	}

	/* If allowed by the operation, move all value offsets into "src_loc2" */
	if (ARITHOP_MAYMOVEOFF(op)) {
		ptrdiff_t off1 = memloc_getoff(src_loc1);
		if (off1 == 0) {
			/* Nothing do to: lhs already has a value-offset of 0 */
		} else {
			if (src_loc2 != &loc2_asnormal)
				loc2_asnormal = *src_loc2;
			src_loc2 = &loc2_asnormal;
			memloc_adjoff(&loc2_asnormal, -off1);
			loc1_asnormal = *src_loc1;
			memloc_setoff(&loc1_asnormal, 0);
			src_loc1 = &loc1_asnormal;
		}
	}

	if (memloc_sameloc(src_loc1, src_loc2)) {
		switch (op) {
		case ARITHOP_USUB:
		case ARITHOP_SSUB:
			return fg_gmov_const2reg(self, (void const *)0, dst_regno);
		default: break;
		}
		return fg_gmov_loc2reg(self, src_loc1, dst_regno);
	}
	if unlikely(memloc_gettyp(src_loc1) == MEMADR_TYPE_UNDEFINED)
		return (dst_o && dst_no) ? fg_gjmp(self, dst_no) : 0;
	switch (memloc_gettyp(src_loc2)) {

	case MEMADR_TYPE_HSTACKIND: {
		host_cfa_t src_loc2_cfa = memloc_hstackind_getcfa(src_loc2);
		if (memloc_hstackind_getvaloff(src_loc2) != 0)
			goto src_loc2_fallback;
		switch (memloc_gettyp(src_loc1)) {
		default:
			DO(fg_gmov_loc2reg(self, src_loc1, dst_regno));
			memloc_init_hreg(&loc1_asreg, dst_regno, 0);
			src_loc1 = &loc1_asreg;
			ATTR_FALLTHROUGH
		case MEMADR_TYPE_HREG:
			if (memloc_hreg_getvaloff(src_loc1) != 0) {
				if unlikely(fg_gmov_regx2reg(self,
				                             memloc_hreg_getreg(src_loc1),
				                             memloc_hreg_getvaloff(src_loc1),
				                             dst_regno))
					goto err;
				return fg_gjarith_reghstackind2reg(self, op, dst_regno, src_loc2_cfa,
				                                   dst_regno, dst_o, dst_no);
			}
			return fg_gjarith_reghstackind2reg(self, op, memloc_hreg_getreg(src_loc1),
			                                   src_loc2_cfa, dst_regno, dst_o, dst_no);
		}
	}	break;

	case MEMADR_TYPE_HREGIND: {
		host_regno_t src_loc2_regno = memloc_hregind_getreg(src_loc2);
		ptrdiff_t src_loc2_indoff = memloc_hregind_getindoff(src_loc2);
		if (memloc_hregind_getvaloff(src_loc2) != 0)
			goto src_loc2_fallback;
		switch (memloc_gettyp(src_loc1)) {
		default:
			DO(fg_gmov_loc2reg(self, src_loc1, dst_regno));
			memloc_init_hreg(&loc1_asreg, dst_regno, 0);
			src_loc1 = &loc1_asreg;
			ATTR_FALLTHROUGH
		case MEMADR_TYPE_HREG:
			if (memloc_hreg_getvaloff(src_loc1) != 0) {
				if unlikely(fg_gmov_regx2reg(self,
				                             memloc_hreg_getreg(src_loc1),
				                             memloc_hreg_getvaloff(src_loc1),
				                             dst_regno))
					goto err;
				return fg_gjarith_regregind2reg(self, op, dst_regno, src_loc2_regno,
				                                src_loc2_indoff, dst_regno, dst_o, dst_no);
			}
			return fg_gjarith_regregind2reg(self, op, memloc_hreg_getreg(src_loc1),
			                                src_loc2_regno, src_loc2_indoff, dst_regno, dst_o, dst_no);
		}
	}	break;

	default:
src_loc2_fallback:
		not_these[0] = HOST_REGNO_COUNT;
		not_these[1] = HOST_REGNO_COUNT;
		if (memloc_hasreg(src_loc1))
			not_these[0] = memloc_getreg(src_loc1);
		DO(fg_gasreg(self, src_loc2, &loc2_asreg, not_these));
		src_loc2 = &loc2_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		if (memloc_gettyp(src_loc1) != MEMADR_TYPE_HREG) {
			not_these[0] = memloc_getreg(src_loc2);
			not_these[1] = HOST_REGNO_COUNT;
			DO(fg_gasreg(self, src_loc1, &loc1_asreg, not_these));
			src_loc1 = &loc1_asreg;
		}
		ASSERT(memloc_gettyp(src_loc1) == MEMADR_TYPE_HREG);
		ASSERT(memloc_gettyp(src_loc2) == MEMADR_TYPE_HREG);
		return fg_gjarith_regxregx2reg(self, op,
		                               memloc_hreg_getreg(src_loc1),
		                               memloc_hreg_getvaloff(src_loc1),
		                               memloc_hreg_getreg(src_loc2),
		                               memloc_hreg_getvaloff(src_loc2),
		                               dst_regno, dst_o, dst_no);

	case MEMADR_TYPE_CONST: {
		uintptr_t src_loc2_value = (uintptr_t)memloc_const_getaddr(src_loc2);
		switch (memloc_gettyp(src_loc1)) {
		default:
			DO(fg_gmov_loc2reg(self, src_loc1, dst_regno));
			memloc_init_hreg(&loc1_asreg, dst_regno, 0);
			src_loc1 = &loc1_asreg;
			ATTR_FALLTHROUGH
		case MEMADR_TYPE_HREG:
			if (memloc_hreg_getvaloff(src_loc1) != 0) {
				if unlikely(fg_gmov_regx2reg(self,
				                             memloc_hreg_getreg(src_loc1),
				                             memloc_hreg_getvaloff(src_loc1),
				                             dst_regno))
					goto err;
				return fg_gjarith_regconst2reg(self, op, dst_regno, (void const *)src_loc2_value,
				                               dst_regno, dst_o, dst_no);
			}
			return fg_gjarith_regconst2reg(self, op, memloc_hreg_getreg(src_loc1),
			                               (void const *)src_loc2_value, dst_regno, dst_o, dst_no);
		case MEMADR_TYPE_CONST: {
			uintptr_t dst_value;
			uintptr_t src_loc1_value = (uintptr_t)memloc_const_getaddr(src_loc1);
			bool is_overflow = host_arithop_forconst(op, src_loc1_value, src_loc2_value, &dst_value);
			struct host_symbol *dst = is_overflow ? dst_o : dst_no;
			DO(fg_gmov_const2reg(self, (void const *)dst_value, dst_regno));
			return dst ? fg_gjmp(self, dst) : 0;
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
fg_gmov_const2loc(struct fungen *__restrict self,
                  void const *value, struct memloc const *__restrict dst_loc) {
	/* TODO: If "dst_loc" is a known equivalence of "value", do nothing */
	switch (memloc_gettyp(dst_loc)) {

	case MEMADR_TYPE_HSTACKIND: {
		host_cfa_t cfa_offset = memloc_hstackind_getcfa(dst_loc);
		void const *final_value = (byte_t const *)value - memloc_getoff(dst_loc);
		return fg_gmov_const2hstackind(self, final_value, cfa_offset);
	}	break;

	case MEMADR_TYPE_HREGIND: {
		void const *final_value = (byte_t const *)value - memloc_getoff(dst_loc);
		return fg_gmov_const2regind(self, final_value,
		                            memloc_hregind_getreg(dst_loc),
		                            memloc_hregind_getindoff(dst_loc));
	}	break;

	case MEMADR_TYPE_HREG: {
		void const *final_value = (byte_t const *)value - memloc_getoff(dst_loc);
		return fg_gmov_const2reg(self, final_value, memloc_hreg_getreg(dst_loc));
	}	break;

	default: {
		host_regno_t temp_regno, not_these[2];
		not_these[0] = HOST_REGNO_COUNT;
		not_these[1] = HOST_REGNO_COUNT;
		if (memloc_hasreg(dst_loc))
			not_these[0] = memloc_getreg(dst_loc);
		temp_regno = fg_gallocreg(self, not_these);
		if unlikely(temp_regno >= HOST_REGNO_COUNT)
			goto err;
		if unlikely(fg_gmov_const2reg(self, value, temp_regno))
			goto err;
		return fg_gmov_reg2loc(self, temp_regno, dst_loc);
	}	break;

	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
fg_gmov_hstack2loc(struct fungen *__restrict self,
                   host_cfa_t cfa_offset,
                   struct memloc const *__restrict dst_loc) {
	switch (memloc_gettyp(dst_loc)) {

	case MEMADR_TYPE_HREG: {
		cfa_offset = HA_cfa_offset_PLUS_sp_offset(cfa_offset, -memloc_hreg_getvaloff(dst_loc));
		return fg_gmov_hstack2reg(self, cfa_offset, memloc_hreg_getreg(dst_loc));
	}	break;

	default: {
		host_regno_t temp_regno, not_these[2];
		not_these[0] = HOST_REGNO_COUNT;
		not_these[1] = HOST_REGNO_COUNT;
		if (memloc_hasreg(dst_loc))
			not_these[0] = memloc_getreg(dst_loc);
		temp_regno = fg_gallocreg(self, not_these);
		if unlikely(temp_regno >= HOST_REGNO_COUNT)
			goto err;
		cfa_offset = HA_cfa_offset_PLUS_sp_offset(cfa_offset, -memloc_getoff(dst_loc));
		if unlikely(fg_gmov_hstack2reg(self, cfa_offset, temp_regno))
			goto err;
		return fg_gmov_reg2loc(self, temp_regno, dst_loc);
	}	break;

	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
fg_gmov_regx2loc(struct fungen *__restrict self,
                 host_regno_t src_regno, ptrdiff_t src_delta,
                 struct memloc const *__restrict dst_loc) {
	switch (memloc_gettyp(dst_loc)) {

	case MEMADR_TYPE_HSTACKIND: {
		host_cfa_t cfa_offset = memloc_hstackind_getcfa(dst_loc);
		ptrdiff_t delta_delta = memloc_hstackind_getvaloff(dst_loc) - src_delta;
		if (delta_delta != 0) {
			/* Adjust `src_regno' to have the correct value-delta */
			if unlikely(fg_gmov_regx2reg(self, src_regno, delta_delta, src_regno))
				goto err;
		}
		return fg_gmov_reg2hstackind(self, src_regno, cfa_offset);
	}	break;

	case MEMADR_TYPE_HREGIND: {
		ptrdiff_t delta_delta = memloc_hregind_getvaloff(dst_loc) - src_delta;
		if (delta_delta != 0) {
			if unlikely(fg_gmov_regx2reg(self, src_regno, delta_delta, src_regno))
				goto err;
		}
		return fg_gmov_reg2regind(self, src_regno,
		                          memloc_hregind_getreg(dst_loc),
		                          memloc_hregind_getindoff(dst_loc));
	}	break;

	case MEMADR_TYPE_HREG:
		return fg_gmov_regx2reg(self, src_regno,
		                        src_delta - memloc_hreg_getvaloff(dst_loc),
		                        memloc_hreg_getreg(dst_loc));

	default: break;
	}
	return DeeError_Throwf(&DeeError_IllegalInstruction,
	                       "Cannot move register to location type %" PRFu16,
	                       memloc_gettyp(dst_loc));
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gmov_loc2regx(struct fungen *__restrict self,
                 struct memloc const *__restrict src_loc,
                 host_regno_t dst_regno, ptrdiff_t dst_delta) {
	int result;
	/* TODO: Go through equivalences of "src_loc" and pick the best one for the move */
	switch (memloc_gettyp(src_loc)) {

	case MEMADR_TYPE_HSTACKIND: {
		host_cfa_t cfa_offset = memloc_hstackind_getcfa(src_loc);
		ptrdiff_t delta_delta = memloc_hstackind_getvaloff(src_loc) - dst_delta;
		result = fg_gmov_hstackind2reg(self, cfa_offset, dst_regno);
		if likely(result == 0 && delta_delta != 0)
			result = fg_gmov_regx2reg(self, dst_regno, delta_delta, dst_regno);
	}	break;

	case MEMADR_TYPE_HSTACK: {
		host_cfa_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(memloc_hstack_getcfa(src_loc), -dst_delta);
		result = fg_gmov_hstack2reg(self, cfa_offset, dst_regno);
	}	break;

	case MEMADR_TYPE_HREGIND: {
		ptrdiff_t delta_delta = memloc_hregind_getvaloff(src_loc) - dst_delta;
		result = fg_gmov_regind2reg(self,
		                            memloc_hregind_getreg(src_loc),
		                            memloc_hregind_getindoff(src_loc),
		                            dst_regno);
		if likely(result == 0 && delta_delta != 0)
			result = fg_gmov_regx2reg(self, dst_regno, delta_delta, dst_regno);
	}	break;

	case MEMADR_TYPE_HREG:
		result = fg_gmov_regx2reg(self,
		                          memloc_hreg_getreg(src_loc),
		                          memloc_hreg_getvaloff(src_loc) - dst_delta,
		                          dst_regno);
		break;

	case MEMADR_TYPE_CONST:
		result = fg_gmov_const2reg(self,
		                           memloc_const_getaddr(src_loc) - dst_delta,
		                           dst_regno);
		break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot move location type %" PRFu16 " to register",
		                       memloc_gettyp(src_loc));
	}
	if likely(result == 0)
		self->fg_state->ms_rusage[dst_regno] = HOST_REGUSAGE_GENERIC;
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
fg_gmov_loc2regy(struct fungen *__restrict self,
                 struct memloc const *__restrict src_loc,
                 host_regno_t dst_regno,
                 ptrdiff_t *__restrict p_dst_delta) {
	int result;
	/* TODO: Check if (%dst_regno + dst_delta) is an equivalence of "src_loc" */
	/* TODO: Go through equivalences of "src_loc" and pick the best one for the move */
	switch (memloc_gettyp(src_loc)) {

	case MEMADR_TYPE_HSTACKIND: {
		host_cfa_t cfa_offset = memloc_hstackind_getcfa(src_loc);
		result = fg_gmov_hstackind2reg(self, cfa_offset, dst_regno);
		*p_dst_delta = memloc_hstackind_getvaloff(src_loc);
	}	break;

	case MEMADR_TYPE_HSTACK: {
		host_cfa_t cfa_offset  = memloc_hstack_getcfa(src_loc);
		ptrdiff_t sp_offset   = memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		host_cfa_t cfa0_offset = memstate_hstack_sp2cfa(self->fg_state, 0);
		result = fg_gmov_hstack2reg(self, cfa0_offset, dst_regno);
		*p_dst_delta = sp_offset;
	}	break;

	case MEMADR_TYPE_HREGIND:
		result = fg_gmov_regind2reg(self,
		                            memloc_hregind_getreg(src_loc),
		                            memloc_hregind_getindoff(src_loc),
		                            dst_regno);
		*p_dst_delta = memloc_hregind_getvaloff(src_loc);
		break;

	case MEMADR_TYPE_HREG:
		result = fg_gmov_reg2reg(self, memloc_hreg_getreg(src_loc), dst_regno);
		*p_dst_delta = memloc_hreg_getvaloff(src_loc);
		break;

	case MEMADR_TYPE_CONST:
		result = fg_gmov_const2reg(self, memloc_const_getaddr(src_loc), dst_regno);
		*p_dst_delta = 0;
		break;

	default:
		result = fg_gmov_loc2regx(self, src_loc, dst_regno, 0);
		*p_dst_delta = 0;
		return result;
	}
	if likely(result == 0)
		self->fg_state->ms_rusage[dst_regno] = HOST_REGUSAGE_GENERIC;
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gmov_locind2reg(struct fungen *__restrict self,
                   struct memloc const *__restrict src_loc, ptrdiff_t src_delta,
                   host_regno_t dst_regno) {
	int result;
	switch (memloc_gettyp(src_loc)) {

	case MEMADR_TYPE_HREG:
		result = fg_gmov_regind2reg(self,
		                            memloc_hreg_getreg(src_loc),
		                            memloc_hreg_getvaloff(src_loc) + src_delta,
		                            dst_regno);
		break;

	case MEMADR_TYPE_HSTACK: {
		host_cfa_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(memloc_hstack_getcfa(src_loc), src_delta);
		result = fg_gmov_hstackind2reg(self, cfa_offset, dst_regno);
	}	break;

	case MEMADR_TYPE_CONST: {
		void const *value = memloc_const_getaddr(src_loc) + src_delta;
		result = fg_gmov_constind2reg(self, (void const **)value, dst_regno);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	default: {
		ptrdiff_t ind_delta;
		result = fg_gmov_loc2regy(self, src_loc, dst_regno, &ind_delta);
		if likely(result == 0)
			result = fg_gmov_regind2reg(self, dst_regno, ind_delta + src_delta, dst_regno);
	}	break;

	}
	if likely(result == 0)
		self->fg_state->ms_rusage[dst_regno] = HOST_REGUSAGE_GENERIC;
	return result;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
fg_gmov_reg2locind(struct fungen *__restrict self,
                   host_regno_t src_regno,
                   struct memloc const *__restrict dst_loc,
                   ptrdiff_t dst_delta) {
	int result;
	switch (memloc_gettyp(dst_loc)) {

	case MEMADR_TYPE_HREG:
		result = fg_gmov_reg2regind(self, src_regno,
		                            memloc_hreg_getreg(dst_loc),
		                            memloc_hreg_getvaloff(dst_loc) + dst_delta);
		break;

	case MEMADR_TYPE_HSTACK: {
		host_cfa_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(memloc_hstack_getcfa(dst_loc), dst_delta);
		result = fg_gmov_reg2hstackind(self, src_regno, cfa_offset);
	}	break;

	case MEMADR_TYPE_CONST: {
		void const *value = memloc_const_getaddr(dst_loc) + dst_delta;
		result = fg_gmov_reg2constind(self, src_regno, (void const **)value);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	default: {
		host_regno_t temp_regno;
		host_regno_t not_these[2];
		ptrdiff_t ind_delta;
		struct memequiv *eq;

		/* See if "dst_loc" has a register/const equivalence. */
		eq = fg_remember_getclassof(self, memloc_getadr(dst_loc));
		if (eq != NULL) {
			struct memequiv *iter = memequiv_next(eq);
			do {
				if (memloc_gettyp(&iter->meq_loc) == MEMEQUIV_TYPE_HREG ||
				    memloc_gettyp(&iter->meq_loc) == MEMEQUIV_TYPE_CONST) {
					struct memloc reg_loc;
					ptrdiff_t val_delta = memloc_getoff(dst_loc);
					val_delta -= memloc_getoff(&eq->meq_loc);
					reg_loc = iter->meq_loc;
					ASSERT(memloc_gettyp(&reg_loc) == MEMADR_TYPE_HREG ||
					       memloc_gettyp(&reg_loc) == MEMADR_TYPE_CONST);
					memloc_adjoff(&reg_loc, val_delta);
					return fg_gmov_reg2locind(self, src_regno, &reg_loc, dst_delta);
				}
			} while ((iter = memequiv_next(iter)) != eq);
		}

		/* Need to use a temporary register. */
		not_these[0] = src_regno;
		not_these[1] = HOST_REGNO_COUNT;
		temp_regno = fg_gallocreg(self, not_these);
		if unlikely(temp_regno >= HOST_REGNO_COUNT)
			goto err;
		if unlikely(fg_gmov_loc2regy(self, dst_loc, temp_regno, &ind_delta))
			goto err;
		result = fg_gmov_reg2regind(self, src_regno, temp_regno, ind_delta + dst_delta);
	}	break;

	}
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_ghstack_pushlocx(struct fungen *__restrict self,
                    struct memloc const *__restrict src_loc,
                    ptrdiff_t dst_delta) {
	struct memloc src_asreg0;
	switch (memloc_gettyp(src_loc)) {

	case MEMADR_TYPE_HREG: {
		ptrdiff_t final_delta = memloc_hreg_getvaloff(src_loc) - dst_delta;
		if (final_delta != 0) {
			ptrdiff_t writeback_delta = memloc_hreg_getvaloff(src_loc) + final_delta;
			if unlikely(fg_gmov_regx2reg(self,
			                             memloc_hreg_getreg(src_loc),
			                             final_delta,
			                             memloc_hreg_getreg(src_loc)))
				goto err;
			memstate_hregs_adjust_delta(self->fg_state, memloc_hreg_getreg(src_loc), final_delta);
			src_asreg0 = *src_loc;
			memloc_hreg_setvaloff(&src_asreg0, writeback_delta);
			src_loc = &src_asreg0;
		}
		return fg_ghstack_pushreg(self, memloc_hreg_getreg(src_loc));
	}	break;

	case MEMADR_TYPE_HREGIND: {
		ptrdiff_t final_delta = memloc_hregind_getvaloff(src_loc) - dst_delta;
		if (final_delta != 0)
			goto fallback;
		return fg_ghstack_pushregind(self,
		                             memloc_hregind_getreg(src_loc),
		                             memloc_hregind_getindoff(src_loc));
	}	break;

	case MEMADR_TYPE_HSTACKIND: {
		host_cfa_t cfa_offset = memloc_hstackind_getcfa(src_loc);
		ptrdiff_t final_delta = memloc_hstackind_getvaloff(src_loc) - dst_delta;
		if (final_delta != 0)
			goto fallback;
		return fg_ghstack_pushhstackind(self, cfa_offset);
	}	break;

#ifdef HAVE_fg_ghstack_pushhstack_at_cfa_boundary_np
	case MEMADR_TYPE_HSTACK: {
		host_cfa_t cfa_offset = memloc_hstack_getcfa(src_loc);
		if (cfa_offset == self->fg_state->ms_host_cfa_offset) /* Special case: push current CFA offset */
			return fg_ghstack_pushhstack_at_cfa_boundary_np(self);
		goto fallback;
	}	break;
#endif /* HAVE_fg_ghstack_pushhstack_at_cfa_boundary_np */

	case MEMADR_TYPE_CONST: {
		void const *value = memloc_const_getaddr(src_loc) - dst_delta;
#ifndef fg_ghstack_pushconst_MAYFAIL
		return fg_ghstack_pushconst(self, value);
#else /* !fg_ghstack_pushconst_MAYFAIL */
		int result = fg_ghstack_pushconst(self, value);
		if unlikely(result > 0)
			goto fallback;
		return result;
#endif /* fg_ghstack_pushconst_MAYFAIL */
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return fg_ghstack_adjust(self, HOST_SIZEOF_POINTER);

	default: {
		host_regno_t temp_regno;
		host_regno_t not_these[2];
fallback:
		not_these[0] = HOST_REGNO_COUNT;
		not_these[1] = HOST_REGNO_COUNT;
		if (memloc_hasreg(src_loc))
			not_these[0] = memloc_getreg(src_loc);
		temp_regno = fg_gallocreg(self, not_these);
		if unlikely(temp_regno >= HOST_REGNO_COUNT)
			goto err;
		if unlikely(fg_gmov_loc2regx(self, src_loc, temp_regno, dst_delta))
			goto err;
		return fg_ghstack_pushreg(self, temp_regno);
	}	break;

	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gmov_loc2hstackindx(struct fungen *__restrict self,
                       struct memloc const *__restrict src_loc,
                       host_cfa_t dst_cfa_offset, ptrdiff_t dst_delta) {
	struct memloc src_asreg0;
#ifdef HOSTASM_STACK_GROWS_DOWN
	if (dst_cfa_offset == self->fg_state->ms_host_cfa_offset + HOST_SIZEOF_POINTER)
#else /* HOSTASM_STACK_GROWS_DOWN */
	if (dst_cfa_offset == self->fg_state->ms_host_cfa_offset)
#endif /* !HOSTASM_STACK_GROWS_DOWN */
	{
		/* Push the value instead! */
		return fg_ghstack_pushlocx(self, src_loc, dst_delta);
	}
	switch (memloc_gettyp(src_loc)) {

	case MEMADR_TYPE_HREG: {
		ptrdiff_t final_delta = memloc_hreg_getvaloff(src_loc) - dst_delta;
		if (final_delta != 0) {
			ptrdiff_t writeback_delta = memloc_hreg_getvaloff(src_loc) + final_delta;
			if unlikely(fg_gmov_regx2reg(self,
			                             memloc_hreg_getreg(src_loc),
			                             final_delta,
			                             memloc_hreg_getreg(src_loc)))
				goto err;
			memstate_hregs_adjust_delta(self->fg_state, memloc_hreg_getreg(src_loc), final_delta);
			src_asreg0 = *src_loc;
			memloc_hreg_setvaloff(&src_asreg0, writeback_delta);
			src_loc = &src_asreg0;
		}
		return fg_gmov_reg2hstackind(self, memloc_hreg_getreg(src_loc), dst_cfa_offset);
	}	break;

	case MEMADR_TYPE_CONST: {
		void const *value = memloc_const_getaddr(src_loc) - dst_delta;
		return fg_gmov_const2hstackind(self, value, dst_cfa_offset);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	default: {
		host_regno_t temp_regno;
		host_regno_t not_these[2];
/*fallback:*/
		not_these[0] = HOST_REGNO_COUNT;
		not_these[1] = HOST_REGNO_COUNT;
		if (memloc_hasreg(src_loc))
			not_these[0] = memloc_getreg(src_loc);
		temp_regno = fg_gallocreg(self, not_these);
		if unlikely(temp_regno >= HOST_REGNO_COUNT)
			goto err;
		if unlikely(fg_gmov_loc2regx(self, src_loc, temp_regno, dst_delta))
			goto err;
		return fg_gmov_reg2hstackind(self, temp_regno, dst_cfa_offset);
	}	break;

	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gmov_loc2constind(struct fungen *__restrict self,
                     struct memloc const *__restrict src_loc,
                     void const **p_value, ptrdiff_t dst_delta) {
	struct memloc src_asreg0;
	switch (memloc_gettyp(src_loc)) {

	case MEMADR_TYPE_HREG: {
		/*     *p_value + dst_delta = memloc_hreg_getreg(src_loc) + memloc_hreg_getvaloff(src_loc)
		 * <=> *p_value = memloc_hreg_getreg(src_loc) + memloc_hreg_getvaloff(src_loc) - dst_delta */
		ptrdiff_t final_delta = memloc_hreg_getvaloff(src_loc) - dst_delta;
		if (final_delta != 0) {
			ptrdiff_t writeback_delta = memloc_hreg_getvaloff(src_loc) + final_delta;
			if unlikely(fg_gmov_regx2reg(self,
			                             memloc_hreg_getreg(src_loc),
			                             final_delta,
			                             memloc_hreg_getreg(src_loc)))
				goto err;
			memstate_hregs_adjust_delta(self->fg_state, memloc_hreg_getreg(src_loc), final_delta);
			src_asreg0 = *src_loc;
			memloc_hreg_setvaloff(&src_asreg0, writeback_delta);
			src_loc = &src_asreg0;
		}
		return fg_gmov_reg2constind(self, memloc_hreg_getreg(src_loc), p_value);
	}	break;

	case MEMADR_TYPE_CONST: {
		void const *value = memloc_const_getaddr(src_loc) - dst_delta;
		return fg_gmov_const2constind(self, value, p_value);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	default: {
		host_regno_t temp_regno;
		host_regno_t not_these[2];
/*fallback:*/
		not_these[0] = HOST_REGNO_COUNT;
		not_these[1] = HOST_REGNO_COUNT;
		if (memloc_hasreg(src_loc))
			not_these[0] = memloc_getreg(src_loc);
		temp_regno = fg_gallocreg(self, not_these);
		if unlikely(temp_regno >= HOST_REGNO_COUNT)
			goto err;
		if unlikely(fg_gmov_loc2regx(self, src_loc, temp_regno, dst_delta))
			goto err;
		return fg_gmov_reg2constind(self, temp_regno, p_value);
	}	break;

	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
fg_gmov_const2locind(struct fungen *__restrict self, void const *value,
                     struct memloc const *__restrict dst_loc, ptrdiff_t dst_delta) {
	int result;
	switch (memloc_gettyp(dst_loc)) {

	case MEMADR_TYPE_HREG:
		result = fg_gmov_const2regind(self, value,
		                              memloc_hreg_getreg(dst_loc),
		                              memloc_hreg_getvaloff(dst_loc) + dst_delta);
		break;

	case MEMADR_TYPE_HSTACK: {
		host_cfa_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(memloc_hstack_getcfa(dst_loc), dst_delta);
		result = fg_gmov_const2hstackind(self, value, cfa_offset);
	}	break;

	case MEMADR_TYPE_CONST: {
		void const *dst_value = memloc_const_getaddr(dst_loc) + dst_delta;
		result = fg_gmov_const2constind(self, value, (void const **)dst_value);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	default: {
		host_regno_t temp_regno;
		ptrdiff_t ind_delta;
		struct memequiv *eq;

		/* See if "dst_loc" has a register/const equivalence. */
		eq = fg_remember_getclassof(self, memloc_getadr(dst_loc));
		if (eq != NULL) {
			struct memequiv *iter = memequiv_next(eq);
			do {
				if (memloc_gettyp(&iter->meq_loc) == MEMEQUIV_TYPE_HREG ||
				    memloc_gettyp(&iter->meq_loc) == MEMEQUIV_TYPE_CONST) {
					struct memloc reg_loc;
					ptrdiff_t val_delta = memloc_getoff(dst_loc);
					val_delta -= memloc_getoff(&eq->meq_loc);
					reg_loc = iter->meq_loc;
					ASSERT(memloc_gettyp(&reg_loc) == MEMADR_TYPE_HREG ||
					       memloc_gettyp(&reg_loc) == MEMADR_TYPE_CONST);
					memloc_adjoff(&reg_loc, val_delta);
					return fg_gmov_const2locind(self, value, &reg_loc, dst_delta);
				}
			} while ((iter = memequiv_next(iter)) != eq);
		}

		/* Need to use a temporary register. */
		temp_regno = fg_gallocreg(self, NULL);
		if unlikely(temp_regno >= HOST_REGNO_COUNT)
			goto err;
		if unlikely(fg_gmov_loc2regy(self, dst_loc, temp_regno, &ind_delta))
			goto err;
		result = fg_gmov_const2regind(self, value, temp_regno, ind_delta + dst_delta);
	}	break;

	}
	return result;
err:
	return -1;
}


/* Generate code to return `loc'. No extra code to decref stack/locals is generated. If you
 * want that extra code to be generated, you need to use `fg_vret()'. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gret(struct fungen *__restrict self,
        /*inherit_ref*/ struct memloc const *__restrict loc) {
	ptrdiff_t alloc_delta;
	int result;

	/* Move the return value into its proper register. */
	result = fg_gmov_loc2reg(self, loc, HOST_REGNO_RETURN);
	if unlikely(result != 0)
		goto done;

	/* Release any remaining stack memory. */
	alloc_delta = (ptrdiff_t)self->fg_state->ms_host_cfa_offset;
	if (alloc_delta != 0) {
		result = fg_ghstack_adjust(self, -alloc_delta);
		if unlikely(result != 0)
			goto done;
	}

	/* Generate the arch-specific return instruction sequence. */
	result = _fungen_gret(self);
done:
	return result;
}



/* Helpers for transforming locations into deemon boolean objects. */


INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmorph_regx2reg01(struct fungen *__restrict self,
                     host_regno_t src_regno, ptrdiff_t src_delta,
                     unsigned int cmp, host_regno_t dst_regno) {
	int result = _fungen_gmorph_regx2reg01(self, src_regno, src_delta, cmp, dst_regno);
	if likely(result == 0)
		fg_remember_undefined_reg(self, dst_regno);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmorph_regind2reg01(struct fungen *__restrict self,
                       host_regno_t src_regno, ptrdiff_t ind_delta, ptrdiff_t val_delta,
                       unsigned int cmp, host_regno_t dst_regno) {
	int result = _fungen_gmorph_regind2reg01(self, src_regno, ind_delta, val_delta, cmp, dst_regno);
#ifdef _fungen_gmorph_regind2reg01_MAYFAIL
	if unlikely(result > 0) {
		host_regno_t val_delta_reg;
		host_regno_t not_these[3];
		not_these[0] = src_regno;
		not_these[1] = dst_regno;
		not_these[2] = HOST_REGNO_RETURN;
		val_delta_reg = fg_gallocreg(self, not_these);
		if unlikely(val_delta_reg >= HOST_REGNO_RETURN)
			return -1;
		result = fg_gmov_const2reg(self, (void const *)(uintptr_t)(-val_delta), val_delta_reg);
		if likely(result == 0) {
			result = fg_gmorph_regindCreg2reg01(self, src_regno, ind_delta, 0,
			                                    cmp, val_delta_reg, dst_regno);
		}
		return result;
	}
#endif /* _fungen_gmorph_regind2reg01_MAYFAIL */
	if likely(result == 0)
		fg_remember_undefined_reg(self, dst_regno);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmorph_hstackind2reg01(struct fungen *__restrict self,
                          host_cfa_t cfa_offset, ptrdiff_t val_delta,
                          unsigned int cmp, host_regno_t dst_regno) {
	ptrdiff_t sp_offset = memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	int result = _fungen_gmorph_hstackind2reg01(self, sp_offset, val_delta, cmp, dst_regno);
#ifdef _fungen_gmorph_hstackind2reg01_MAYFAIL
	if unlikely(result > 0) {
		host_regno_t val_delta_reg;
		host_regno_t not_these[2];
		not_these[0] = dst_regno;
		not_these[1] = HOST_REGNO_RETURN;
		val_delta_reg = fg_gallocreg(self, not_these);
		if unlikely(val_delta_reg >= HOST_REGNO_RETURN)
			return -1;
		result = fg_gmov_const2reg(self, (void const *)(uintptr_t)(-val_delta), val_delta_reg);
		if likely(result == 0) {
			result = fg_gmorph_hstackindCreg2reg01(self, cfa_offset, 0,
			                                       cmp, val_delta_reg, dst_regno);
		}
		return result;
	}
#endif /* _fungen_gmorph_hstackind2reg01_MAYFAIL */
	if likely(result == 0)
		fg_remember_undefined_reg(self, dst_regno);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmorph_regxCreg2reg01(struct fungen *__restrict self,
                         host_regno_t src_regno, ptrdiff_t src_delta,
                         unsigned int cmp, host_regno_t rhs_regno,
                         host_regno_t dst_regno) {
	int result = _fungen_gmorph_regxCreg2reg01(self, src_regno, src_delta, cmp, rhs_regno, dst_regno);
	if likely(result == 0)
		fg_remember_undefined_reg(self, dst_regno);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmorph_regindCreg2reg01(struct fungen *__restrict self,
                           host_regno_t src_regno, ptrdiff_t ind_delta, ptrdiff_t val_delta,
                           unsigned int cmp, host_regno_t rhs_regno,
                           host_regno_t dst_regno) {
	int result = _fungen_gmorph_regindCreg2reg01(self, src_regno, ind_delta, val_delta, cmp, rhs_regno, dst_regno);
	if likely(result == 0)
		fg_remember_undefined_reg(self, dst_regno);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gmorph_hstackindCreg2reg01(struct fungen *__restrict self,
                              host_cfa_t cfa_offset, ptrdiff_t val_delta,
                              unsigned int cmp, host_regno_t rhs_regno,
                              host_regno_t dst_regno) {
	ptrdiff_t sp_offset = memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	int result = _fungen_gmorph_hstackindCreg2reg01(self, sp_offset, val_delta, cmp, rhs_regno, dst_regno);
	if likely(result == 0)
		fg_remember_undefined_reg(self, dst_regno);
	return result;
}

/* dst_regno = (src_loc + src_delta) <CMP> 0 ? 1 : 0; */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gmorph_loc2reg01(struct fungen *__restrict self,
                    struct memloc const *src_loc, ptrdiff_t src_delta,
                    unsigned int cmp, host_regno_t dst_regno) {
	struct memloc src_asreg;
	switch (memloc_gettyp(src_loc)) {
	default: {
		host_regno_t not_these[2];
		not_these[0] = dst_regno;
		not_these[1] = HOST_REGNO_COUNT;
		if unlikely(fg_gasreg(self, src_loc, &src_asreg, not_these))
			goto err;
		src_loc = &src_asreg;
	}	ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return fg_gmorph_regx2reg01(self,
		                            memloc_hreg_getreg(src_loc),
		                            memloc_hreg_getvaloff(src_loc) + src_delta,
		                            cmp, dst_regno);
	case MEMADR_TYPE_HREGIND:
		return fg_gmorph_regind2reg01(self,
		                              memloc_hregind_getreg(src_loc),
		                              memloc_hregind_getindoff(src_loc),
		                              memloc_hregind_getvaloff(src_loc) + src_delta,
		                              cmp, dst_regno);
	case MEMADR_TYPE_HSTACKIND:
		return fg_gmorph_hstackind2reg01(self,
		                                 memloc_hstackind_getcfa(src_loc),
		                                 memloc_hstackind_getvaloff(src_loc) + src_delta,
		                                 cmp, dst_regno);
	}
	__builtin_unreachable();
err:
	return -1;
}

/* dst_regno = (src_loc + src_delta) <CMP> rhs_regno ? 1 : 0; */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gmorph_locCreg2reg01(struct fungen *__restrict self,
                        struct memloc const *src_loc, ptrdiff_t src_delta,
                        unsigned int cmp, host_regno_t rhs_regno,
                        host_regno_t dst_regno) {
	struct memloc src_asreg;
	switch (memloc_gettyp(src_loc)) {
	default: {
		host_regno_t not_these[3];
		not_these[0] = rhs_regno;
		not_these[1] = dst_regno;
		not_these[2] = HOST_REGNO_COUNT;
		if unlikely(fg_gasreg(self, src_loc, &src_asreg, not_these))
			goto err;
		src_loc = &src_asreg;
	}	ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return fg_gmorph_regxCreg2reg01(self,
		                                memloc_hreg_getreg(src_loc),
		                                memloc_hreg_getvaloff(src_loc) + src_delta,
		                                cmp, rhs_regno, dst_regno);
	case MEMADR_TYPE_HREGIND:
		return fg_gmorph_regindCreg2reg01(self,
		                                  memloc_hregind_getreg(src_loc),
		                                  memloc_hregind_getindoff(src_loc),
		                                  memloc_hregind_getvaloff(src_loc) + src_delta,
		                                  cmp, rhs_regno, dst_regno);
	case MEMADR_TYPE_HSTACKIND:
		return fg_gmorph_hstackindCreg2reg01(self,
		                                     memloc_hstackind_getcfa(src_loc),
		                                     memloc_hstackind_getvaloff(src_loc) + src_delta,
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
fg_gmorph_locCloc2reg01(struct fungen *__restrict self,
                        struct memloc const *src_loc, ptrdiff_t src_delta,
                        unsigned int cmp, struct memloc const *rhs_loc,
                        host_regno_t dst_regno) {
	struct memloc rhs_asreg;
	if (memloc_gettyp(rhs_loc) != MEMADR_TYPE_HREG) {
		if (memloc_gettyp(src_loc) == MEMADR_TYPE_HREG) {
			/* Flip operands so the register appears in "rhs_loc" */
			struct memloc const *temp;
			cmp = flip_gmorphbool_cc(cmp, &src_delta);
			temp    = src_loc;
			src_loc = rhs_loc;
			rhs_loc = temp;
		} else {
			/* Force "rhs_loc" into a register. */
			host_regno_t not_these[3];
			not_these[0] = dst_regno;
			not_these[1] = HOST_REGNO_COUNT;
			not_these[2] = HOST_REGNO_COUNT;
			if (memloc_hasreg(src_loc))
				not_these[1] = memloc_getreg(src_loc);
			if unlikely(fg_gasreg(self, rhs_loc, &rhs_asreg, not_these))
				goto err;
			rhs_loc = &rhs_asreg;
		}
	}
	ASSERT(memloc_gettyp(rhs_loc) == MEMADR_TYPE_HREG);
	return fg_gmorph_locCreg2reg01(self, src_loc,
	                               src_delta - memloc_hreg_getvaloff(rhs_loc),
	                               cmp,
	                               memloc_hreg_getreg(rhs_loc), dst_regno);
err:
	return -1;
}


/* dst_regno = &Dee_FalseTrue[(src_loc + src_delta) <CMP> 0 ? 1 : 0] - *p_dst_delta; */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
fg_gmorph_loc2regbooly(struct fungen *__restrict self,
                       struct memloc const *src_loc, ptrdiff_t src_delta,
                       unsigned int cmp, host_regno_t dst_regno,
                       ptrdiff_t *__restrict p_dst_delta) {
	int result = fg_gmorph_loc2reg01(self, src_loc, src_delta, cmp, dst_regno);
	if likely(result == 0) {
		struct memloc uloc;
		memloc_init_hreg(&uloc, dst_regno, 0);
		result = fg_gmorph_loc012regbooly(self, &uloc, 0, dst_regno, p_dst_delta);
	}
	return result;
}

#ifndef HAVE__host_section_gmorph_reg012regbool
/* dst_regno = &Dee_FalseTrue[src_regno + src_delta] - *p_dst_delta; */
INTERN WUNUSED NONNULL((1, 5)) int DCALL
fg_gmorph_reg012regbooly(struct fungen *__restrict self,
                         host_regno_t src_regno, ptrdiff_t src_delta,
                         host_regno_t dst_regno, ptrdiff_t *__restrict p_dst_delta) {
	struct host_section *sect = fg_gettext(self);
	if unlikely(_host_section_gumul_regconst2reg(sect, src_regno,
	                                             sizeof(DeeBoolObject), dst_regno))
		goto err;
	src_delta *= sizeof(DeeBoolObject);
	src_delta += (ptrdiff_t)(uintptr_t)Dee_FalseTrue;
	*p_dst_delta = 0;
	return _host_section_gmov_regx2reg(sect, dst_regno, src_delta, dst_regno);
err:
	return -1;
}
#endif /* !HAVE__host_section_gmorph_reg012regbool */

/* dst_regno = &Dee_FalseTrue[src_loc + src_delta] - *p_dst_delta; */
INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
fg_gmorph_loc012regbooly(struct fungen *__restrict self,
                         struct memloc const *src_loc, ptrdiff_t src_delta,
                         host_regno_t dst_regno, ptrdiff_t *__restrict p_dst_delta) {
	switch (memloc_gettyp(src_loc)) {
	case MEMADR_TYPE_HREG:
		return fg_gmorph_reg012regbooly(self,
		                                memloc_hreg_getreg(src_loc),
		                                memloc_hreg_getvaloff(src_loc) + src_delta,
		                                dst_regno, p_dst_delta);

	case MEMADR_TYPE_CONST: {
		DeeObject *value = Dee_False;
		if ((uintptr_t)(memloc_const_getaddr(src_loc) + src_delta) != 0)
			value = Dee_True;
		*p_dst_delta = 0;
		return fg_gmov_const2reg(self, value, dst_regno);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		*p_dst_delta = 0;
		return fg_gmov_const2reg(self, Dee_False, dst_regno);

	default: {
		ptrdiff_t dst_delta;
		if unlikely(fg_gmov_loc2regy(self, src_loc, dst_regno, &dst_delta))
			goto err;
		src_delta -= dst_delta;
		return fg_gmorph_reg012regbooly(self, dst_regno, src_delta, dst_regno, p_dst_delta);
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
PRIVATE WUNUSED NONNULL((1)) host_cfa_t DCALL
try_restore_xloc_arg_cfa_offset(struct fungen *__restrict self,
                                host_regno_t regno) {
#define MEMSTATE_XLOCAL_A_MIN MEMSTATE_XLOCAL_A_THIS
#define MEMSTATE_XLOCAL_A_MAX MEMSTATE_XLOCAL_A_KW
	lid_t i, xloc_base = self->fg_assembler->fa_localc;
	struct memstate *state = self->fg_state;
	for (i = MEMSTATE_XLOCAL_A_MIN; i <= MEMSTATE_XLOCAL_A_MAX; ++i) {
		struct memval *xval = &state->ms_localv[xloc_base + i];
		if (memval_isdirect(xval) &&
		    memval_direct_gettyp(xval) == MEMADR_TYPE_HREG &&
		    memval_direct_hreg_getreg(xval) == regno) {
			host_cfa_t cfa_offset;
			host_cc_t cc = self->fg_assembler->fa_cc;
			size_t true_argi = 0;
			switch (i) {
			case MEMSTATE_XLOCAL_A_THIS:
				ASSERT(cc & HOST_CC_F_THIS);
				break;
			case MEMSTATE_XLOCAL_A_ARGC:
				ASSERT(!(cc & HOST_CC_F_TUPLE));
				if (cc & HOST_CC_F_THIS)
					++true_argi;
				break;
			case MEMSTATE_XLOCAL_A_ARGV: /* or `MEMSTATE_XLOCAL_A_ARGS' */
				if (cc & HOST_CC_F_THIS)
					++true_argi;
				if (!(cc & HOST_CC_F_TUPLE))
					++true_argi;
				break;
			case MEMSTATE_XLOCAL_A_KW:
				ASSERT(cc & HOST_CC_F_KW);
				if (cc & HOST_CC_F_THIS)
					++true_argi;
				if (cc & HOST_CC_F_TUPLE)
					++true_argi;
				++true_argi;
				break;
			default: __builtin_unreachable();
			}
			cfa_offset = (host_cfa_t)(-(ptrdiff_t)((true_argi + 1) * HOST_SIZEOF_POINTER));
			return cfa_offset;
		}
	}
	return (host_cfa_t)-1;
}
#endif /* HOSTASM_X86 && !HOSTASM_X86_64 */

/* Push/move `regno' onto the host stack, returning the CFA offset of the target location. */
PRIVATE WUNUSED NONNULL((1)) host_cfa_t DCALL
fg_gsavereg(struct fungen *__restrict self,
            host_regno_t regno) {
	host_cfa_t cfa_offset;
	ASSERT(!memstate_isshared(self->fg_state));
#ifdef HAVE_try_restore_xloc_arg_cfa_offset
	cfa_offset = try_restore_xloc_arg_cfa_offset(self, regno);
	if (cfa_offset != (host_cfa_t)-1)
		return cfa_offset;
#endif /* HAVE_try_restore_xloc_arg_cfa_offset */
	cfa_offset = memstate_hstack_find(self->fg_state,
	                                  self->fg_state_hstack_res,
	                                  HOST_SIZEOF_POINTER);
	if (cfa_offset != (host_cfa_t)-1) {
		if unlikely(fg_gmov_reg2hstackind(self, regno, cfa_offset))
			goto err;
	} else {
		/* Allocate more stack space. */
		if unlikely(fg_ghstack_pushreg(self, regno))
			goto err;
		cfa_offset = self->fg_state->ms_host_cfa_offset;
#ifndef HOSTASM_STACK_GROWS_DOWN
		cfa_offset -= HOST_SIZEOF_POINTER;
#endif /* HOSTASM_STACK_GROWS_DOWN */
	}
	return cfa_offset;
err:
	return (host_cfa_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
fg_gflushregind(struct fungen *__restrict self,
                struct memloc const *flush_loc) {
	struct memval *val;
	struct memstate *state = self->fg_state;
	host_regno_t regno = memloc_hregind_getreg(flush_loc);
	ptrdiff_t ind_offset = memloc_hregind_getindoff(flush_loc);
	host_cfa_t cfa_offset;
	ASSERT(memloc_gettyp(flush_loc) == MEMADR_TYPE_HREGIND);
	cfa_offset = memstate_hstack_find(self->fg_state,
	                                  self->fg_state_hstack_res,
	                                  HOST_SIZEOF_POINTER);
	if (cfa_offset != (host_cfa_t)-1) {
		bool did_save_temp_regno = false;
		host_regno_t temp_regno;
		temp_regno = memstate_hregs_find_unused(state, true);
		if (temp_regno >= HOST_REGNO_COUNT) {
			temp_regno = regno;
			did_save_temp_regno = true;
			if unlikely(fg_ghstack_pushreg(self, temp_regno))
				goto err;
		}
		if unlikely(fg_gmov_regind2reg(self, regno, ind_offset, temp_regno))
			goto err;
		if unlikely(fg_gmov_reg2hstackind(self, temp_regno, cfa_offset))
			goto err;
		if (did_save_temp_regno) {
			if unlikely(fg_ghstack_popreg(self, temp_regno))
				goto err;
		}
	} else {
		if unlikely(fg_ghstack_pushregind(self, regno, ind_offset))
			goto err;
		cfa_offset = self->fg_state->ms_host_cfa_offset;
#ifndef HOSTASM_STACK_GROWS_DOWN
		cfa_offset -= HOST_SIZEOF_POINTER;
#endif /* HOSTASM_STACK_GROWS_DOWN */
	}

	/* Convert all locations that use `MEMADR_TYPE_HREGIND:regno:off' to `MEMADR_TYPE_HSTACKIND' */
	memstate_foreach(val, state) {
		struct memobj *obj;
		memval_foreach_obj(obj, val) {
			if (memobj_gettyp(obj) == MEMADR_TYPE_HREGIND &&
			    memobj_hregind_getreg(obj) == regno &&
			    memobj_hregind_getindoff(obj) == ind_offset) {
				memstate_decrinuse(state, regno);
				memloc_init_hstackind(memobj_getloc(obj), cfa_offset,
				                      memobj_hregind_getvaloff(obj));
			}
			if (memobj_hasxinfo(obj)) {
				struct memobj_xinfo *xinfo;
				xinfo = memobj_getxinfo(obj);
				if (memloc_gettyp(&xinfo->mox_dep) == MEMADR_TYPE_HREGIND &&
				    memloc_hregind_getreg(&xinfo->mox_dep) == regno &&
				    memloc_hregind_getindoff(&xinfo->mox_dep) == ind_offset) {
					memloc_init_hstackind(&xinfo->mox_dep, cfa_offset,
					                      memloc_hregind_getvaloff(&xinfo->mox_dep));
				}
			}
		}
		memval_foreach_obj_end;
	}
	memstate_foreach_end;

	ASSERT(memloc_gettyp(flush_loc) == MEMADR_TYPE_HSTACKIND);
	return 0;
err:
	return -1;
}

/* Generate code to flush all registers used by the deemon stack/locals into the host stack.
 * NOTE: Usage-registers are cleared by arch-specific code (e.g. `fg_gcallapi()')
 * @param: ignore_top_n_stack_if_not_ref: From the top-most N stack locations, ignore any
 *                                        that don't contain object references.
 * @param: only_if_reference: Only flush locations that contain references. */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vflushregs(struct fungen *__restrict self,
              vstackaddr_t ignore_top_n_stack_if_not_ref,
              bool only_if_reference) {
	lid_t i;
	host_cfa_t register_cfa[HOST_REGNO_COUNT];
	struct memstate *state = self->fg_state;
	bool changed_dependencies_to_stach = false;
	ASSERT(!memstate_isshared(state));

	/* Figure out which registers are in use, and assign them CFA offsets. */
	for (i = 0; i < HOST_REGNO_COUNT; ++i)
		register_cfa[i] = (host_cfa_t)-1;
	for (i = 0; i < state->ms_localc; ++i) {
		struct memobj *obj;
		struct memval *val = &state->ms_localv[i];
		if (only_if_reference && memval_isdirect(val) && !memval_direct_isref(val))
			continue;
		memval_foreach_obj(obj, val) {
			if (memobj_gettyp(obj) == MEMADR_TYPE_HREG) {
				host_regno_t regno = memobj_hreg_getreg(obj);
				ASSERT(regno < HOST_REGNO_COUNT);
				if (register_cfa[regno] == (host_cfa_t)-1) {
					register_cfa[regno] = fg_gsavereg(self, regno);
					if unlikely(register_cfa[regno] == (host_cfa_t)-1)
						goto err;
				}
				memstate_decrinuse(state, regno);
				memloc_init_hstackind(memobj_getloc(obj), register_cfa[regno],
				                      memobj_hreg_getvaloff(obj));
				if (obj->mo_flags & MEMOBJ_F_HASDEP)
					changed_dependencies_to_stach = true;
			} else if (memobj_gettyp(obj) == MEMADR_TYPE_HREGIND) {
				if unlikely(fg_gflushregind(self, memobj_getloc(obj)))
					goto err;
				ASSERT(memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND);
			}
		}
		memval_foreach_obj_end;
	}
	for (i = 0; i < state->ms_stackc; ++i) {
		struct memobj *obj;
		struct memval *val = &state->ms_stackv[i];
		if (memval_isdirect(val) && !memval_direct_isref(val)) {
			if (i >= (vstackaddr_t)(state->ms_stackc - ignore_top_n_stack_if_not_ref))
				continue; /* Slot contains no reference and is in top-most n of stack. */
			if (only_if_reference)
				continue;
		}
		memval_foreach_obj(obj, val) {
			if (memobj_gettyp(obj) == MEMADR_TYPE_HREG) {
				host_regno_t regno = memobj_hreg_getreg(obj);
				ASSERT(regno < HOST_REGNO_COUNT);
				if (register_cfa[regno] == (host_cfa_t)-1) {
					register_cfa[regno] = fg_gsavereg(self, regno);
					if unlikely(register_cfa[regno] == (host_cfa_t)-1)
						goto err;
				}
				memstate_decrinuse(state, regno);
				memloc_init_hstackind(memobj_getloc(obj), register_cfa[regno],
				                      memobj_hreg_getvaloff(obj));
				if (obj->mo_flags & MEMOBJ_F_HASDEP)
					changed_dependencies_to_stach = true;
			} else if (memobj_gettyp(obj) == MEMADR_TYPE_HREGIND) {
				if unlikely(fg_gflushregind(self, memobj_getloc(obj)))
					goto err;
				ASSERT(memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND);
			}
		}
		memval_foreach_obj_end;
	}
	if (changed_dependencies_to_stach) {
		/* Must update consumers of dependencies of use new stack locations instead of registers. */
		struct memval *depends_mval;
		memstate_foreach(depends_mval, state) {
			struct memobj *depends_mobj;
			memval_foreach_obj(depends_mobj, depends_mval) {
				if (memobj_hasxinfo(depends_mobj)) {
					struct memobj_xinfo *xinfo;
					xinfo = memobj_getxinfo(depends_mobj);
					if (memloc_gettyp(&xinfo->mox_dep) == MEMADR_TYPE_HREG) {
						host_regno_t regno;
						regno = memloc_hreg_getreg(&xinfo->mox_dep);
						ASSERT(regno < HOST_REGNO_COUNT);
						if (register_cfa[regno] != (host_cfa_t)-1) {
							memloc_init_hstackind(&xinfo->mox_dep, register_cfa[regno],
							                      memloc_hreg_getvaloff(&xinfo->mox_dep));
						}
					}
				}
			}
			memval_foreach_obj_end;
		}
		memstate_foreach_end;
	}

	/* NOTE: Usage-registers must be cleared by the caller! */
	return 0;
err:
	return -1;
}

/* Flush memory locations that make use of `regno' onto the hstack. */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vflushreg(struct fungen *__restrict self,
             vstackaddr_t ignore_top_n_stack_if_not_ref,
             bool only_if_reference, host_regno_t regno) {
	lid_t i;
	host_cfa_t regno_cfa = (host_cfa_t)-1;
	bool changed_dependencies_to_stach = false;
	struct memstate *state = self->fg_state;
	ASSERT(!memstate_isshared(state));
	ASSERT(regno < HOST_REGNO_COUNT);

	for (i = 0; i < state->ms_localc; ++i) {
		struct memobj *obj;
		struct memval *val = &state->ms_localv[i];
		if (only_if_reference && memval_isdirect(val) && !memval_direct_isref(val))
			continue;
		memval_foreach_obj(obj, val) {
			if (memobj_gettyp(obj) == MEMADR_TYPE_HREG &&
			    memobj_hreg_getreg(obj) == regno) {
				if (regno_cfa == (host_cfa_t)-1) {
					regno_cfa = fg_gsavereg(self, regno);
					if unlikely(regno_cfa == (host_cfa_t)-1)
						goto err;
				}
				memstate_decrinuse(state, regno);
				memloc_init_hstackind(memobj_getloc(obj), regno_cfa,
				                      memobj_hreg_getvaloff(obj));
				if (obj->mo_flags & MEMOBJ_F_HASDEP)
					changed_dependencies_to_stach = true;
			} else if (memobj_gettyp(obj) == MEMADR_TYPE_HREGIND &&
			           memobj_hregind_getreg(obj) == regno) {
				if unlikely(fg_gflushregind(self, memobj_getloc(obj)))
					goto err;
				ASSERT(memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND);
			}
		}
		memval_foreach_obj_end;
	}
	for (i = 0; i < state->ms_stackc; ++i) {
		struct memobj *obj;
		struct memval *val = &state->ms_stackv[i];
		if (memval_isdirect(val) && !memval_direct_isref(val)) {
			if (i >= (vstackaddr_t)(state->ms_stackc - ignore_top_n_stack_if_not_ref))
				continue; /* Slot contains no reference and is in top-most n of stack. */
			if (only_if_reference)
				continue;
		}
		memval_foreach_obj(obj, val) {
			if (memobj_gettyp(obj) == MEMADR_TYPE_HREG &&
			    memobj_hreg_getreg(obj) == regno) {
				if (regno_cfa == (host_cfa_t)-1) {
					regno_cfa = fg_gsavereg(self, regno);
					if unlikely(regno_cfa == (host_cfa_t)-1)
						goto err;
				}
				memstate_decrinuse(state, regno);
				memloc_init_hstackind(memobj_getloc(obj), regno_cfa,
				                      memobj_hreg_getvaloff(obj));
				if (obj->mo_flags & MEMOBJ_F_HASDEP)
					changed_dependencies_to_stach = true;
			} else if (memobj_gettyp(obj) == MEMADR_TYPE_HREGIND &&
			           memobj_hregind_getreg(obj) == regno) {
				if unlikely(fg_gflushregind(self, memobj_getloc(obj)))
					goto err;
				ASSERT(memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND);
			}
		}
		memval_foreach_obj_end;
	}
	if (changed_dependencies_to_stach) {
		/* Must update consumers of dependencies of use new stack locations instead of registers. */
		struct memval *depends_mval;
		ASSERT(regno_cfa != (host_cfa_t)-1);
		memstate_foreach(depends_mval, state) {
			struct memobj *depends_mobj;
			memval_foreach_obj(depends_mobj, depends_mval) {
				if (memobj_hasxinfo(depends_mobj)) {
					struct memobj_xinfo *xinfo;
					xinfo = memobj_getxinfo(depends_mobj);
					if (memloc_gettyp(&xinfo->mox_dep) == MEMADR_TYPE_HREG &&
					    memloc_hreg_getreg(&xinfo->mox_dep) == regno) {
						memloc_init_hstackind(&xinfo->mox_dep, regno_cfa,
						                      memloc_hreg_getvaloff(&xinfo->mox_dep));
					}
				}
			}
			memval_foreach_obj_end;
		}
		memstate_foreach_end;
	}

	/* NOTE: Usage-registers must be cleared by the caller! */
	return 0;
err:
	return -1;
}


PRIVATE ATTR_PURE WUNUSED bool DCALL
nullable_host_register_list_contains(host_regno_t const *list,
                                     host_regno_t regno) {
	host_regno_t item;
	if (list != NULL) {
		while ((item = *list++) < HOST_REGNO_COUNT) {
			if (item == regno)
				return true;
		}
	}
	return false;
}

/* Assuming that `fg_gtryallocreg()' wasn't able to allocate
 * an unused register, use this function to pick which register should be picked.
 * When it's impossible to allocate *any* register, return >= HOST_REGNO_COUNT,
 * in which case the caller should throw an exception saying that allocation was
 * impossible. */
PRIVATE WUNUSED NONNULL((1)) host_regno_t DCALL
fg_gallocreg_pickreg(struct fungen *__restrict self,
                     host_regno_t const *not_these) {
	lid_t i;
	struct memstate *state;
	host_regno_t result;

	/* TODO: Look at equivalence classes of registers. If a class exists that
	 *       contains more than 1 register, pick one of those registers and
	 *       update all memloc-s to use one of the other registers, then
	 *       return that register to the caller. */

	/* TODO: Look at equivalence classes of registers. If a class exists that
	 *       contains both a register and a CONST item, update all memloc-s
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
		struct memobj *obj;
		struct memval *val = &state->ms_localv[i];
		memval_foreach_obj(obj, val) {
			if (!memobj_hasreg(obj))
				continue;
			result = memobj_getreg(obj);
			if (!nullable_host_register_list_contains(not_these, result))
				return result;
		}
		memval_foreach_obj_end;
	}
	state = self->fg_state;
	for (i = 0; i < state->ms_stackc; ++i) {
		struct memobj *obj;
		struct memval *val = &state->ms_stackv[i];
		memval_foreach_obj(obj, val) {
			if (!memobj_hasreg(obj))
				continue;
			result = memobj_getreg(obj);
			if (!nullable_host_register_list_contains(not_these, result))
				return result;
		}
		memval_foreach_obj_end;
	}

	return HOST_REGNO_COUNT;
}

/* Allocate at host register, possibly flushing an already used register to stack.
 * @param: not_these: Array of registers not to allocated, terminated by one `>= HOST_REGNO_COUNT'.
 * @return: * : The allocated register
 * @return: >= HOST_REGNO_COUNT: Error */
INTERN WUNUSED NONNULL((1)) host_regno_t DCALL
fg_gallocreg(struct fungen *__restrict self,
             host_regno_t const *not_these) {
	host_regno_t result;
	result = fg_gtryallocreg(self, not_these);
	if unlikely(result >= HOST_REGNO_COUNT) {
		/* Pick the register that should be allocated. */
		result = fg_gallocreg_pickreg(self, not_these);
		if unlikely(result >= HOST_REGNO_COUNT)
			goto err_no_way_to_allocate;
		if unlikely(fg_vflushreg(self, 0, false, result))
			goto err;
	}
	return result;

err_no_way_to_allocate:
	/* Impossible to allocate register. */
	DeeError_Throwf(&DeeError_IllegalInstruction,
	                "No way to allocate register");
err:
	return HOST_REGNO_COUNT;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
gmov_usage2reg(struct fungen *__restrict self,
               host_regusage_t usage,
               host_regno_t result_regno) {
	/* TODO */
	(void)self;
	(void)usage;
	(void)result_regno;
	return DeeError_Throwf(&DeeError_IllegalInstruction,
	                       "Unknown usage code: %u",
	                       (unsigned int)usage);
}

/* Helper that returns a register that's been populated for `usage' */
INTERN WUNUSED NONNULL((1)) host_regno_t DCALL
fg_gusagereg(struct fungen *__restrict self,
             host_regusage_t usage,
             host_regno_t const *dont_alloc_these) {
	host_regno_t regno;
	regno = memstate_hregs_find_usage(self->fg_state, usage);
	if (regno >= HOST_REGNO_COUNT) {
		regno = fg_gallocreg(self, dont_alloc_these);
		if likely(regno < HOST_REGNO_COUNT) {
			if unlikely(gmov_usage2reg(self, usage, regno))
				goto err;
			self->fg_state->ms_rusage[regno] = usage;
		}
	}
	return regno;
err:
	return HOST_REGNO_COUNT;
}





/* Generate code to assert that location `loc' is non-NULL:
 * >> fg_gassert_bound(self, loc, instr, NULL, lid, NULL, NULL);
 * >> fg_gassert_bound(self, loc, instr, mod, gid, NULL, NULL);
 * @param: opt_endread_before_throw: When non-NULL, emit `fg_grwlock_endread()'
 *                                   before the `fg_gthrow_*_unbound' code. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gassert_bound(struct fungen *__restrict self,
                 struct memloc const *loc, Dee_instruction_t const *instr,
                 struct Dee_module_object *mod, uint16_t id
#ifndef CONFIG_NO_THREADS
                 , Dee_atomic_rwlock_t *opt_endread_before_throw
                 , Dee_atomic_rwlock_t *opt_endwrite_before_throw
#endif /* !CONFIG_NO_THREADS */
                 ) {
	DREF struct memstate *saved_state;
	struct host_symbol *target;
	struct host_section *text;
	struct host_section *cold;
	text = fg_gettext(self);
	cold = fg_getcold(self);
	if unlikely(!cold)
		goto err;
	target = fg_newsym_named(self, text == cold ? ".Lbound" : ".Lunbound");
	if unlikely(!target)
		goto err;
	DO(text == cold ? fg_gjnz(self, loc, target)
	                : fg_gjz(self, loc, target));
	saved_state = self->fg_state;
	memstate_incref(saved_state);
	EDO(err_saved_state, fg_state_dounshare(self));

	if (text != cold) {
		HA_printf(".section .cold\n");
		EDO(err_saved_state, fg_settext(self, cold));
		host_symbol_setsect(target, cold);
	}

	/* Location isn't bound -> generate code to throw an exception. */
#ifndef CONFIG_NO_THREADS
	if (opt_endwrite_before_throw != NULL)
		EDO(err_saved_state, fg_grwlock_endwrite_const(self, opt_endwrite_before_throw));
	if (opt_endread_before_throw != NULL)
		EDO(err_saved_state, fg_grwlock_endread_const(self, opt_endread_before_throw));
#endif /* !CONFIG_NO_THREADS */
	EDO(err_saved_state,
	    mod ? fg_gthrow_global_unbound(self, mod, id)
	        : fg_gthrow_local_unbound(self, instr, id));

	/* Switch back to the original section, and restore the saved mem-state. */
	memstate_decref(self->fg_state);
	self->fg_state = saved_state;

	ASSERT((text == cold) == !!host_symbol_isdefined(target));
	if (text != cold) {
		DO(fg_settext(self, text));
		HA_printf(".section .text\n");
	} else {
		host_symbol_setsect(target, text);
	}
	return 0;
err_saved_state:
	memstate_decref(self->fg_state);
err:
	return -1;
}

/* Generate code to throw an error indicating that local variable `lid'
 * is unbound. This includes any necessary jump for the purpose of entering
 * an exception handler. */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gthrow_arg_unbound(struct fungen *__restrict self,
                      Dee_instruction_t const *instr, aid_t aid) {
	if unlikely(fg_vpush_const(self, self->fg_assembler->fa_code))
		goto err;
	if unlikely(fg_vpush_addr(self, instr))
		goto err;
	if unlikely(fg_vpush_imm16(self, aid))
		goto err;
	return fg_vcallapi(self, &libhostasm_rt_err_unbound_arg, VCALL_CC_EXCEPT, 3);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gthrow_local_unbound(struct fungen *__restrict self,
                        Dee_instruction_t const *instr, ulid_t lid) {
	if unlikely(fg_vpush_const(self, self->fg_assembler->fa_code))
		goto err;
	if unlikely(fg_vpush_addr(self, instr))
		goto err;
	if unlikely(fg_vpush_imm16(self, lid))
		goto err;
	return fg_vcallapi(self, &libhostasm_rt_err_unbound_local, VCALL_CC_EXCEPT, 3);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gthrow_global_unbound(struct fungen *__restrict self,
                         struct Dee_module_object *mod, uint16_t gid) {
	if unlikely(fg_vpush_const(self, mod))
		goto err;
	if unlikely(fg_vpush_imm16(self, gid))
		goto err;
	return fg_vcallapi(self, &libhostasm_rt_err_unbound_global, VCALL_CC_EXCEPT, 2);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
do_gjz_except(struct fungen *__restrict self,
              struct memloc const *loc) {
	struct except_exitinfo *info;
	if (memloc_gettyp(loc) == MEMADR_TYPE_CONST &&
	    memloc_const_getaddr(loc) != 0)
		return 0;
	info = fg_except_exit(self);
	if unlikely(!info)
		goto err;
	{
		fg_DEFINE_host_symbol_section(self, err, Lexcept, &info->exi_text, 0);
		if (memloc_gettyp(loc) == MEMADR_TYPE_CONST)
			return fg_gjmp(self, Lexcept);
		return fg_gjz(self, loc, Lexcept);
	}
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
do_gjnz_except(struct fungen *__restrict self,
               struct memloc const *loc) {
	struct except_exitinfo *info;
	if (memloc_gettyp(loc) == MEMADR_TYPE_CONST &&
	    memloc_const_getaddr(loc) == 0)
		return 0;
	info = fg_except_exit(self);
	if unlikely(!info)
		goto err;
	{
		fg_DEFINE_host_symbol_section(self, err, Lexcept, &info->exi_text, 0);
		if (memloc_gettyp(loc) == MEMADR_TYPE_CONST)
			return fg_gjmp(self, Lexcept);
		return fg_gjnz(self, loc, Lexcept);
	}
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
do_gjcmp_except(struct fungen *__restrict self,
                struct memloc const *loc, intptr_t threshold,
                unsigned int flags) {
	struct except_exitinfo *info;
	struct memloc threshold_loc;
	info = fg_except_exit(self);
	if unlikely(!info)
		goto err;
	memloc_init_const(&threshold_loc, (byte_t const *)(uintptr_t)threshold);
	{
		fg_DEFINE_host_symbol_section(self, err, Lexcept, &info->exi_text, 0);
		return fg_gjcc(self, loc, &threshold_loc,
		               !(flags & FG_GJCMP_EXCEPT_UNSIGNED),
		               (flags & FG_GJCMP_EXCEPT_LO) ? Lexcept : NULL,
		               (flags & FG_GJCMP_EXCEPT_EQ) ? Lexcept : NULL,
		               (flags & FG_GJCMP_EXCEPT_GR) ? Lexcept : NULL);
	}
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
do_gjmp_except(struct fungen *__restrict self) {
	struct except_exitinfo *info;
	info = fg_except_exit(self);
	if unlikely(!info)
		goto err;
	{
		fg_DEFINE_host_symbol_section(self, err, Lexcept, &info->exi_text, 0);
		return fg_gjmp(self, Lexcept);
	}
err:
	return -1;
}

/* save the current mem-state and execute injected exception cleanup. */
PRIVATE WUNUSED NONNULL((1)) DREF struct memstate *DCALL
gsave_state_and_do_exceptinject(struct fungen *__restrict self) {
	struct fungen_exceptinject *chain;
	DREF struct memstate *saved_state;
	ASSERT(self->fg_exceptinject != NULL);

	/* Generate code that needed for custom exception handling. */
	saved_state = self->fg_state;
	memstate_incref(saved_state);
	chain = self->fg_exceptinject;
	do {
		vstackaddr_t n_pop;
		ASSERT(self->fg_state->ms_stackc >= chain->fei_stack);
		n_pop = self->fg_state->ms_stackc - chain->fei_stack;
		if (n_pop != 0) {
			vstackaddr_t i;
			struct memval *pop_base;
			if unlikely(fg_state_unshare(self))
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
				if (memval_isnullable(&pop_base[i])) {
					memval_nullable_makedirect(&pop_base[i]);
					memval_direct_clearref(&pop_base[i]);
				}
			}

			/* Pop vstack items which the injection handler doesn't care about. */
			if unlikely(fg_vpopmany(self, n_pop))
				goto err_saved_state;
		}
		if unlikely((*chain->fei_inject)(self, chain))
			goto err_saved_state;
	} while ((chain = chain->fei_next) != NULL);
	return saved_state;
err_saved_state:
	memstate_decref(self->fg_state);
/*err:*/
	return NULL;
}





/* Generate checks to enter exception handling mode. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gjz_except(struct fungen *__restrict self,
              struct memloc const *loc) {
	if likely(self->fg_exceptinject == NULL)
		return do_gjz_except(self, loc);
	return fg_gjeq_except(self, loc, 0);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gjnz_except(struct fungen *__restrict self,
               struct memloc const *loc) {
	if likely(self->fg_exceptinject == NULL)
		return do_gjnz_except(self, loc);
	return fg_gjne_except(self, loc, 0);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
do_slow_gjcmp_except(struct fungen *__restrict self,
                     struct memloc const *loc, intptr_t threshold,
                     unsigned int flags) {
	bool signed_cmp = !(flags & FG_GJCMP_EXCEPT_UNSIGNED);
	struct host_section *text = fg_gettext(self);
	struct host_section *cold = fg_getcold(self);
	struct memloc compare_value_loc;
	if unlikely(!cold)
		goto err;
	memloc_init_const(&compare_value_loc, (byte_t const *)(uintptr_t)threshold);
	if (cold == text) {
		struct host_symbol *Lno_except;
		Lno_except = fg_newsym_named(self, ".Lno_except");
		if unlikely(!Lno_except)
			goto err;
		if unlikely(fg_gjcc(self, loc, &compare_value_loc, signed_cmp,
		                     (flags & FG_GJCMP_EXCEPT_LO) ? NULL : Lno_except,
		                     (flags & FG_GJCMP_EXCEPT_EQ) ? NULL : Lno_except,
		                     (flags & FG_GJCMP_EXCEPT_GR) ? NULL : Lno_except))
			goto err;
		if unlikely(fg_gjmp_except(self))
			goto err;
		host_symbol_setsect(Lno_except, text);
	} else {
		struct host_symbol *Ldo_except;
		Ldo_except = fg_newsym_named(self, ".Ldo_except");
		if unlikely(!Ldo_except)
			goto err;
		if unlikely(fg_gjcc(self, loc, &compare_value_loc, false,
		                     (flags & FG_GJCMP_EXCEPT_LO) ? Ldo_except : NULL,
		                     (flags & FG_GJCMP_EXCEPT_EQ) ? Ldo_except : NULL,
		                     (flags & FG_GJCMP_EXCEPT_GR) ? Ldo_except : NULL))
			goto err;
		HA_printf(".section .cold\n");
		DO(fg_settext(self, cold));
		host_symbol_setsect(Ldo_except, cold);
		DO(fg_gjmp_except(self));
		HA_printf(".section .text\n");
		DO(fg_settext(self, text));
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gjcmp_except(struct fungen *__restrict self,
                struct memloc const *loc, intptr_t threshold,
                unsigned int flags) {
	if likely(self->fg_exceptinject == NULL)
		return do_gjcmp_except(self, loc, threshold, flags);
	if (memloc_gettyp(loc) == MEMADR_TYPE_CONST) {
		bool should_jump_except = false;
		intptr_t lhs = (intptr_t)(uintptr_t)memloc_const_getaddr(loc);
		if (flags & FG_GJCMP_EXCEPT_LO) {
			should_jump_except |= (flags & FG_GJCMP_EXCEPT_UNSIGNED)
			                      ? ((uintptr_t)lhs < (uintptr_t)threshold)
			                      : (lhs < threshold);
		}
		if (flags & FG_GJCMP_EXCEPT_EQ)
			should_jump_except |= lhs == threshold;
		if (flags & FG_GJCMP_EXCEPT_GR) {
			should_jump_except |= (flags & FG_GJCMP_EXCEPT_UNSIGNED)
			                      ? ((uintptr_t)lhs > (uintptr_t)threshold)
			                      : (lhs > threshold);
		}
		return should_jump_except
		       ? fg_gjmp_except(self)
		       : 0;
	}
	return do_slow_gjcmp_except(self, loc, threshold, flags);
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjmp_except(struct fungen *__restrict self) {
	DREF struct memstate *saved_state;
	if unlikely(self->fg_exceptinject == NULL)
		return do_gjmp_except(self);
	saved_state = gsave_state_and_do_exceptinject(self);
	if unlikely(!saved_state)
		goto err;
	if unlikely(do_gjmp_except(self))
		goto err_saved_state;
	memstate_decref(self->fg_state);
	self->fg_state = saved_state;
	return 0;
err_saved_state:
	memstate_decref(self->fg_state);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
fg_gcallapi(struct fungen *__restrict self,
            struct memloc *locv, size_t argc) {
	int result = _fungen_gcallapi(self, locv, argc);
	if likely(result == 0) {
		fg_remember_undefined_allregs(self);
		memstate_hregs_clear_usage(self->fg_state);
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gincref_loc(struct fungen *__restrict self,
               struct memloc const *loc, Dee_refcnt_t n) {
	struct memloc loc_asreg;
	switch (memloc_gettyp(loc)) {
	case MEMADR_TYPE_CONST:
#ifndef fg_gincref_const_MAYFAIL
		return fg_gincref_const(self, (DeeObject *)memloc_const_getaddr(loc), n);
#else /* !fg_gincref_const_MAYFAIL */
		{
			int result = fg_gincref_const(self, (DeeObject *)memloc_const_getaddr(loc), n);
			if likely(result <= 0)
				return result;
		}
		ATTR_FALLTHROUGH
#endif /* fg_gincref_const_MAYFAIL */
	default:
		if unlikely(fg_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return fg_gincref_regx(self,
		                       memloc_hreg_getreg(loc),
		                       memloc_hreg_getvaloff(loc),
		                       n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gdecref_loc(struct fungen *__restrict self,
               struct memloc const *loc, Dee_refcnt_t n) {
	struct memloc loc_asreg;
	switch (memloc_gettyp(loc)) {
	case MEMADR_TYPE_CONST:
#ifndef fg_gdecref_const_MAYFAIL
		return fg_gdecref_const(self, (DeeObject *)memloc_const_getaddr(loc), n);
#else  /* !fg_gdecref_const_MAYFAIL */
	{
		int result = fg_gdecref_const(self, (DeeObject *)memloc_const_getaddr(loc), n);
		if likely(result <= 0)
			return result;
	}	ATTR_FALLTHROUGH
#endif /* fg_gdecref_const_MAYFAIL */
	default:
		if unlikely(fg_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return fg_gdecref_regx(self,
		                       memloc_hreg_getreg(loc),
		                       memloc_hreg_getvaloff(loc),
		                       n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gdecref_dokill_loc(struct fungen *__restrict self,
                      struct memloc const *loc) {
	struct memloc loc_asreg;
	switch (memloc_gettyp(loc)) {
	default:
		if unlikely(fg_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return fg_gdecref_regx_dokill(self,
		                              memloc_hreg_getreg(loc),
		                              memloc_hreg_getvaloff(loc));
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
fg_gdecref_nokill_loc(struct fungen *__restrict self,
                      struct memloc const *loc, Dee_refcnt_t n) {
	struct memloc loc_asreg;
	switch (memloc_gettyp(loc)) {
	default:
		if unlikely(fg_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return fg_gdecref_nokill_regx(self,
		                              memloc_hreg_getreg(loc),
		                              memloc_hreg_getvaloff(loc),
		                              n);
	case MEMADR_TYPE_CONST:
		return fg_gdecref_loc(self, loc, n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gxincref_loc(struct fungen *__restrict self,
                struct memloc const *loc, Dee_refcnt_t n) {
	struct memloc loc_asreg;
	switch (memloc_gettyp(loc)) {
	default:
		if unlikely(fg_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return fg_gxincref_regx(self,
		                        memloc_hreg_getreg(loc),
		                        memloc_hreg_getvaloff(loc),
		                        n);
	case MEMADR_TYPE_CONST:
		if (memloc_const_getaddr(loc) == NULL)
			return 0;
		return fg_gincref_loc(self, loc, n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gxdecref_loc(struct fungen *__restrict self,
                struct memloc const *loc, Dee_refcnt_t n) {
	struct memloc loc_asreg;
	switch (memloc_gettyp(loc)) {
	default:
		if unlikely(fg_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return fg_gxdecref_regx(self,
		                        memloc_hreg_getreg(loc),
		                        memloc_hreg_getvaloff(loc),
		                        n);
	case MEMADR_TYPE_CONST:
		if (memloc_const_getaddr(loc) == NULL)
			return 0;
		return fg_gdecref_loc(self, loc, n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gxdecref_nokill_loc(struct fungen *__restrict self,
                       struct memloc const *loc, Dee_refcnt_t n) {
	struct memloc loc_asreg;
	switch (memloc_gettyp(loc)) {
	default:
		if unlikely(fg_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return fg_gxdecref_nokill_regx(self,
		                               memloc_hreg_getreg(loc),
		                               memloc_hreg_getvaloff(loc),
		                               n);
	case MEMADR_TYPE_CONST:
		if (memloc_const_getaddr(loc) == NULL)
			return 0;
		return fg_gdecref_loc(self, loc, n);
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

/* Change `loc' into the value of `<result> = *(<loc> + ind_delta)'
 * Note that unlike the `fg_gmov*' functions, this
 * one may use `MEMADR_TYPE_*IND' to defer the indirection until later. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
fg_gasind(struct fungen *__restrict self,
          /*in*/ struct memloc const *loc,
          /*out*/ struct memloc *result,
          ptrdiff_t ind_delta) {
	struct memloc loc_asreg;
	struct memequiv *eq;
	switch (memloc_gettyp(loc)) {

	case MEMADR_TYPE_HSTACK: {
		host_cfa_t cfa_offset = HA_cfa_offset_PLUS_sp_offset(memloc_hstack_getcfa(loc), ind_delta);
		eq = fg_remember_getclassof_hstackind(self, cfa_offset);
		if (eq != NULL) {
			memequiv_next_asloc(eq, result);
			return 0;
		}
		memloc_init_hstackind(result, cfa_offset, 0);
		return 0;
	}	break;

	case MEMADR_TYPE_CONST: {
		void const *p_value = memloc_const_getaddr(loc) + ind_delta;
		host_regno_t temp_regno;
		temp_regno = fg_gallocreg(self, NULL);
		if unlikely(temp_regno >= HOST_REGNO_COUNT)
			goto err;
		if unlikely(fg_gmov_constind2reg(self, (void const **)p_value, temp_regno))
			goto err;
		memloc_init_hreg(result, temp_regno, 0);
	}	break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	default:
		if unlikely(fg_gasreg(self, loc, &loc_asreg, NULL))
			goto err;
		loc = &loc_asreg;
		ASSERT(memloc_gettyp(loc) == MEMADR_TYPE_HREG);
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG: {
		host_regno_t regno = memloc_hreg_getreg(loc);
		ptrdiff_t final_ind_delta = memloc_hreg_getvaloff(loc) + ind_delta;
		eq = fg_remember_getclassof_regind(self, regno, final_ind_delta);
		if (eq != NULL) {
			memequiv_next_asloc(eq, result);
			return 0;
		}
		/* Turn the location from an HREG into HREGIND */
		memloc_init_hregind(result, regno, final_ind_delta, 0);
	}	break;
	}
	return 0;
err:
	return -1;
}

/* Force `loc' to become a register (`MEMADR_TYPE_HREG'). */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
fg_gasreg(struct fungen *__restrict self,
          /*in*/ struct memloc const *loc,
          /*out*/ struct memloc *result,
          host_regno_t const *not_these) {
	struct memequiv *eq;
	host_regno_t regno;
	ptrdiff_t val_delta;
	if (memloc_gettyp(loc) == MEMADR_TYPE_HREG) {
		*result = *loc;
		return 0; /* Already in a register! */
	}

	/* Check if "loc" has a known register equivalence. */
	eq = fg_remember_getclassof(self, memloc_getadr(loc));
	if (eq != NULL) {
		struct memequiv *reg_eq = memequiv_next(eq);
		while (memloc_gettyp(&reg_eq->meq_loc) != MEMEQUIV_TYPE_HREG) {
			if (reg_eq == eq)
				goto no_equivalence;
			reg_eq = memequiv_next(reg_eq);
		}

		/* Example:
		 * >> eq(FOO + 3)  == reg_eq(BAR + 5)
		 * >> loc(FOO + 7) == reg_eq(BAR + 9)
		 * >> 9 = 7 - 3 + 5
		 * >> 9 == RESULT_VAL_DELTA
		 * >> 7 == memloc_getoff(loc)
		 * >> 3 == memloc_getoff(&eq->meq_loc)
		 * >> 5 == memloc_getoff(&reg_eq->meq_loc)   (gets added in `memequiv_asloc()') */
		val_delta = memloc_getoff(loc);
		val_delta -= memloc_getoff(&eq->meq_loc);
		*result = reg_eq->meq_loc;
		ASSERT(memloc_gettyp(result) == MEMADR_TYPE_HREG);
		memloc_adjoff(result, val_delta);
		return 0;
	}
no_equivalence:

	/* Allocate a register. */
	regno = fg_gallocreg(self, not_these);
	if unlikely(regno >= HOST_REGNO_COUNT)
		goto err;

	/* Move value into register. */
	if unlikely(fg_gmov_loc2regy(self, loc, regno, &val_delta))
		goto err;

	/* Remember that `loc' now lies in a register. */
	memloc_init_hreg(result, regno, val_delta);
	return 0;
err:
	return -1;
}

/* Force `loc' to reside on the stack, giving it an address
 * (`MEMADR_TYPE_HSTACKIND, memloc_hstackind_getvaloff = 0').
 * @param: require_valoff_0: When false, forgo the exit requirement
 *                           of `memloc_hstackind_getvaloff = 0' */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
fg_gasflush(struct fungen *__restrict self,
            /*in*/ struct memloc const *loc,
            /*out*/ struct memloc *result,
            bool require_valoff_0) {
	ptrdiff_t val_offset;
	host_cfa_t cfa_offset;
	struct memstate *state = self->fg_state;
	ASSERT(!memstate_isshared(state));
	*result = *loc;
	if (memloc_gettyp(result) == MEMADR_TYPE_HSTACKIND) {
handle_hstackind_loc:
		if (memloc_hstackind_getvaloff(result) == 0)
			return 0; /* Already on-stack at offset=0 */
		if (require_valoff_0)
			return 0; /* Caller doesn't care about value offset */

#ifdef HAVE__host_section_gadd_const2hstackind
		/* emit `addP $..., sp_offset(%Psp)' to adjust the offset of the stored value
		 * Afterwards, go through all stack/local variables and adjust value offsets
		 * wherever the same CFA offset is referenced. */
		return _host_section_gadd_const2hstackind(fg_gettext(self),
		                                          (void const *)(uintptr_t)(intptr_t)memloc_hstackind_getvaloff(result),
		                                          memstate_hstack_cfa2sp(self->fg_state, memloc_hstackind_getcfa(result)));
#endif /* HAVE__host_section_gadd_const2hstackind */
	}

	/* Figure out where we want to allocate the value. */
#ifdef HAVE_try_restore_xloc_arg_cfa_offset
	if (memloc_gettyp(result) == MEMADR_TYPE_HREG &&
	    (cfa_offset = try_restore_xloc_arg_cfa_offset(self, memloc_hreg_getreg(result))) != (host_cfa_t)-1) {
		val_offset = 0; /* CFA offset restored */
	} else
#endif /* HAVE_try_restore_xloc_arg_cfa_offset */
	{
		/* Check if "result" has a known HSTACKIND equivalence. */
		struct memequiv *eq;

		eq = fg_remember_getclassof(self, memloc_getadr(result));
		if (eq != NULL) {
			ptrdiff_t val_delta = memloc_getoff(result);
			struct memequiv *hstackind_eq_any = NULL;
			struct memequiv *hstackind_eq;
			val_delta -= memloc_getoff(&eq->meq_loc);
			for (hstackind_eq = memequiv_next(eq); hstackind_eq != eq;
			     hstackind_eq = memequiv_next(hstackind_eq)) {
				if (memloc_gettyp(&hstackind_eq->meq_loc) == MEMEQUIV_TYPE_HSTACKIND) {
					if ((memloc_getoff(&hstackind_eq->meq_loc) + val_delta) == 0) {
						/* Perfect match: this equivalence allows for `v_hstack.s_off = 0' */
						*result = hstackind_eq->meq_loc;
						ASSERT((memloc_getoff(result) + val_delta) == 0);
						memloc_setoff(result, 0);
						ASSERT(memloc_gettyp(result) == MEMADR_TYPE_HSTACKIND);
						ASSERT(memloc_getoff(result) == 0);
						return 0;
					}
					hstackind_eq_any = hstackind_eq;
				}
			}
			if (hstackind_eq_any) {
				*result = hstackind_eq->meq_loc;
				memloc_setoff(result, memloc_getoff(result) + val_delta);
				ASSERT(memloc_gettyp(result) == MEMADR_TYPE_HSTACKIND);
				ASSERT(memloc_getoff(result) != 0);
				goto handle_hstackind_loc;
			}
		}

		/* Search for a currently free stack location. */
		cfa_offset = memstate_hstack_find(state, self->fg_state_hstack_res, HOST_SIZEOF_POINTER);
		val_offset = result->ml_off;
		if (!require_valoff_0)
			result->ml_off = 0; /* Don't include value offset when saving location */
		if (cfa_offset != (host_cfa_t)-1) {
			if unlikely(fg_gmov_loc2hstackind(self, result, cfa_offset))
				goto err;
		} else {
			if unlikely(fg_ghstack_pushloc(self, result))
				goto err;
#ifdef HOSTASM_STACK_GROWS_DOWN
			cfa_offset = state->ms_host_cfa_offset;
#else  /* HOSTASM_STACK_GROWS_DOWN */
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
	if (memloc_gettyp(result) == MEMADR_TYPE_HREG ||
	    memloc_gettyp(result) == MEMADR_TYPE_HREGIND) {
		ptrdiff_t val_delta_change = -memloc_getoff(result);
		if (val_delta_change != val_offset) {
			struct memval *alias_val;
			struct memobj *alias_obj;
			struct memloc orig_loc = *result;
			ASSERT(val_offset == 0);
			val_delta_change += val_offset;
			memstate_foreach(alias_val, state) {
				memval_foreach_obj(alias_obj, alias_val) {
					if (!memloc_sameadr(memobj_getloc(alias_obj), &orig_loc))
						continue;
					ASSERT(memobj_hasreg(alias_obj));
					memstate_decrinuse(self->fg_state, memobj_getreg(alias_obj));
					memloc_init_hstackind(memobj_getloc(alias_obj), cfa_offset,
					                      memobj_getoff(alias_obj) + val_delta_change);
				}
				memval_foreach_obj_end;
			}
			memstate_foreach_end;
		}
	}

	/* Remember that `result' now lies on-stack (with an offset of `val_offset') */
	memloc_init_hstackind(result, cfa_offset, val_offset);
	return 0;
err:
	return -1;
}


/* Check if `src_loc' differs from `dst_loc', and if so: move `src_loc' *into* `dst_loc'. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
fg_gmov_loc2loc(struct fungen *__restrict self,
                struct memloc const *src_loc,
                struct memloc const *dst_loc) {
	int result;
	struct memloc src_asreg;
	if (memloc_sameloc(src_loc, dst_loc))
		return 0;
	switch (memloc_gettyp(dst_loc)) {
	case MEMADR_TYPE_HREG:
		result = fg_gmov_loc2regx(self, src_loc,
		                          memloc_hreg_getreg(dst_loc),
		                          memloc_hreg_getvaloff(dst_loc));
		break;
	case MEMADR_TYPE_HSTACKIND:
		result = fg_gmov_loc2hstackindx(self, src_loc,
		                                memloc_hstackind_getcfa(dst_loc),
		                                memloc_hstackind_getvaloff(dst_loc));
		break;
	case MEMADR_TYPE_UNDEFINED:
		return 0;
	default:
		switch (memloc_gettyp(src_loc)) {
		default: {
			host_regno_t not_these[2];
			not_these[0] = HOST_REGNO_COUNT;
			not_these[1] = HOST_REGNO_COUNT;
			if (memloc_hasreg(dst_loc))
				not_these[0] = memloc_getreg(dst_loc);
			if unlikely(fg_gasreg(self, src_loc, &src_asreg, not_these))
				goto err;
			src_loc = &src_asreg;
			ASSERT(memloc_gettyp(src_loc) == MEMADR_TYPE_HREG);
		}	ATTR_FALLTHROUGH
		case MEMADR_TYPE_HREG:
			result = fg_gmov_regx2loc(self,
			                          memloc_hreg_getreg(src_loc),
			                          memloc_hreg_getvaloff(src_loc),
			                          dst_loc);
			break;
		case MEMADR_TYPE_CONST:
			result = fg_gmov_const2loc(self, memloc_const_getaddr(src_loc), dst_loc);
			break;
		case MEMADR_TYPE_HSTACK:
			result = fg_gmov_hstack2loc(self, memloc_hstack_getcfa(src_loc), dst_loc);
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
fg_gmov_loc2locind(struct fungen *__restrict self,
                   struct memloc const *src_loc,
                   struct memloc const *dst_loc, ptrdiff_t ind_delta) {
	int result;
	struct memloc src_asreg;
	switch (memloc_gettyp(src_loc)) {
	default: {
		host_regno_t not_these[2];
		not_these[0] = HOST_REGNO_COUNT;
		not_these[1] = HOST_REGNO_COUNT;
		if (memloc_hasreg(dst_loc))
			not_these[0] = memloc_getreg(dst_loc);
		if unlikely(fg_gasreg(self, src_loc, &src_asreg, not_these))
			goto err;
		src_loc = &src_asreg;
		ASSERT(memloc_gettyp(src_loc) == MEMADR_TYPE_HREG);
	}	ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		if (memloc_hreg_getvaloff(src_loc) != 0) {
			if (fg_gmov_regx2reg(self,
			                     memloc_hreg_getreg(src_loc),
			                     memloc_hreg_getvaloff(src_loc),
			                     memloc_hreg_getreg(src_loc)))
				goto err;
			memstate_hregs_adjust_delta(self->fg_state,
			                            memloc_hreg_getreg(src_loc),
			                            memloc_hreg_getvaloff(src_loc));
			if (src_loc != &src_asreg) {
				src_asreg = *src_loc;
				src_loc   = &src_asreg;
			}
			memloc_hreg_setvaloff(&src_asreg, 0);
		}
		result = fg_gmov_reg2locind(self,
		                            memloc_hreg_getreg(src_loc),
		                            dst_loc, ind_delta);
		break;

	case MEMADR_TYPE_UNDEFINED:
		return 0;

	case MEMADR_TYPE_CONST:
		result = fg_gmov_const2locind(self,
		                              memloc_const_getaddr(src_loc),
		                              dst_loc, ind_delta);
		break;
	}
	return result;
err:
	return -1;
}


#ifndef CONFIG_NO_THREADS
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_grwlock_read_const(struct fungen *__restrict self,
                      Dee_atomic_rwlock_t *__restrict lock) {
	struct memloc loc;
	memloc_init_const(&loc, lock);
	return fg_grwlock_read(self, &loc);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_grwlock_write_const(struct fungen *__restrict self,
                       Dee_atomic_rwlock_t *__restrict lock) {
	struct memloc loc;
	memloc_init_const(&loc, lock);
	return fg_grwlock_write(self, &loc);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_grwlock_endread_const(struct fungen *__restrict self,
                         Dee_atomic_rwlock_t *__restrict lock) {
	struct memloc loc;
	memloc_init_const(&loc, lock);
	return fg_grwlock_endread(self, &loc);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_grwlock_endwrite_const(struct fungen *__restrict self,
                          Dee_atomic_rwlock_t *__restrict lock) {
	struct memloc loc;
	memloc_init_const(&loc, lock);
	return fg_grwlock_endwrite(self, &loc);
}
#endif /* !CONFIG_NO_THREADS */




#ifdef _fungen_gjcc_regindCconst_MAYFAIL
INTDEF WUNUSED NONNULL((1)) int DCALL
fg_gjcc_regindCconst(struct fungen *__restrict self,
                     host_regno_t lhs_regno, ptrdiff_t lhs_ind_delta,
                     void const *rhs_value, bool signed_cmp,
                     struct host_symbol *dst_lo, struct host_symbol *dst_eq,
                     struct host_symbol *dst_gr) {
	int result = _fungen_gjcc_regindCconst(self, lhs_regno, lhs_ind_delta, rhs_value,
	                                       signed_cmp, dst_lo, dst_eq, dst_gr);
	if unlikely(result > 0) {
		host_regno_t tempreg;
		host_regno_t not_these[2];
		not_these[0] = lhs_regno;
		not_these[1] = HOST_REGNO_COUNT;
		tempreg = fg_gconst_as_reg(self, rhs_value, not_these);
		if unlikely(tempreg >= HOST_REGNO_COUNT)
			return -1;
		result = fg_gjcc_regindCreg(self, lhs_regno, lhs_ind_delta, tempreg,
		                            signed_cmp, dst_lo, dst_eq, dst_gr);
	}
	return result;
}
#endif /* !_fungen_gjcc_regindCconst_MAYFAIL */

#ifdef _fungen_gjcc_regCconst_MAYFAIL
INTDEF WUNUSED NONNULL((1)) int DCALL
fg_gjcc_regCconst(struct fungen *__restrict self,
                  host_regno_t lhs_regno, void const *rhs_value, bool signed_cmp,
                  struct host_symbol *dst_lo, struct host_symbol *dst_eq,
                  struct host_symbol *dst_gr) {
	int result = _fungen_gjcc_regCconst(self, lhs_regno, rhs_value,
	                                    signed_cmp, dst_lo, dst_eq, dst_gr);
	if unlikely(result > 0) {
		host_regno_t tempreg;
		host_regno_t not_these[2];
		not_these[0] = lhs_regno;
		not_these[1] = HOST_REGNO_COUNT;
		tempreg = fg_gconst_as_reg(self, rhs_value, not_these);
		if unlikely(tempreg >= HOST_REGNO_COUNT)
			return -1;
		result = fg_gjcc_regCreg(self, lhs_regno, tempreg,
		                         signed_cmp, dst_lo, dst_eq, dst_gr);
	}
	return result;
}
#endif /* !_fungen_gjcc_regCconst_MAYFAIL */

#ifdef _fungen_gjcc_hstackindCconst_MAYFAIL
INTDEF WUNUSED NONNULL((1)) int DCALL
fg_gjcc_hstackindCconst(struct fungen *__restrict self,
                        host_cfa_t lhs_cfa_offset, void const *rhs_value, bool signed_cmp,
                        struct host_symbol *dst_lo, struct host_symbol *dst_eq,
                        struct host_symbol *dst_gr) {
	ptrdiff_t sp_offset = memstate_hstack_cfa2sp(self->fg_state, lhs_cfa_offset);
	int result = _fungen_gjcc_hstackindCconst(self, sp_offset, rhs_value,
	                                          signed_cmp, dst_lo, dst_eq, dst_gr);
	if unlikely(result > 0) {
		host_regno_t tempreg;
		tempreg = fg_gconst_as_reg(self, rhs_value, NULL);
		if unlikely(tempreg >= HOST_REGNO_COUNT)
			return -1;
		result = fg_gjcc_hstackindCreg(self, lhs_cfa_offset, tempreg,
		                               signed_cmp, dst_lo, dst_eq, dst_gr);
	}
	return result;
}
#endif /* !_fungen_gjcc_hstackindCconst_MAYFAIL */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gjcc_locCregx(struct fungen *__restrict self, struct memloc const *lhs,
                 host_regno_t rhs_regno, ptrdiff_t rhs_val_offset, bool signed_cmp,
                 struct host_symbol *dst_lo, struct host_symbol *dst_eq,
                 struct host_symbol *dst_gr) {
	struct memloc lhs_asreg;
	switch (memloc_gettyp(lhs)) {
	default: {
		host_regno_t not_these[2];
fallback:
		not_these[0] = rhs_regno;
		not_these[1] = HOST_REGNO_COUNT;
		if unlikely(fg_gasreg(self, lhs, &lhs_asreg, not_these))
			goto err;
		lhs = &lhs_asreg;
	}	ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		rhs_val_offset -= memloc_hreg_getvaloff(lhs);
		if (memloc_hreg_getreg(lhs) == rhs_regno) { /* 0 <=> rhs_val_offset */
			struct host_symbol *dst;
			if (0 < rhs_val_offset) {
				dst = dst_lo;
			} else if (0 > rhs_val_offset) {
				dst = dst_gr;
			} else {
				dst = dst_eq;
			}
			return dst ? fg_gjmp(self, dst) : 0;
		}
		if (rhs_val_offset != 0) {
			if unlikely(fg_gmov_regx2reg(self, rhs_regno, rhs_val_offset, rhs_regno))
				goto err;
			memstate_hregs_adjust_delta(self->fg_state, rhs_regno, rhs_val_offset);
		}
		return fg_gjcc_regCreg(self, memloc_hreg_getreg(lhs), rhs_regno,
		                       signed_cmp, dst_lo, dst_eq, dst_gr);
	case MEMADR_TYPE_HREGIND:
		if ((rhs_val_offset - memloc_hregind_getvaloff(lhs)) != 0)
			goto fallback;
		return fg_gjcc_regindCreg(self,
		                          memloc_hregind_getreg(lhs),
		                          memloc_hregind_getindoff(lhs), rhs_regno,
		                          signed_cmp, dst_lo, dst_eq, dst_gr);
	case MEMADR_TYPE_HSTACKIND:
		if ((rhs_val_offset - memloc_hstackind_getvaloff(lhs)) != 0)
			goto fallback;
		return fg_gjcc_hstackindCreg(self,
		                             memloc_hstackind_getcfa(lhs), rhs_regno,
		                             signed_cmp, dst_lo, dst_eq, dst_gr);
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gjcc_locCconst(struct fungen *__restrict self, struct memloc const *lhs,
                  void const *rhs_value, bool signed_cmp, struct host_symbol *dst_lo,
                  struct host_symbol *dst_eq, struct host_symbol *dst_gr) {
	struct memloc lhs_asreg;
	switch (memloc_gettyp(lhs)) {
	default:
		if unlikely(fg_gasreg(self, lhs, &lhs_asreg, NULL))
			goto err;
		lhs = &lhs_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		rhs_value = (void const *)((uintptr_t)rhs_value - memloc_hreg_getvaloff(lhs));
		return fg_gjcc_regCconst(self, memloc_hreg_getreg(lhs), rhs_value,
		                         signed_cmp, dst_lo, dst_eq, dst_gr);
	case MEMADR_TYPE_HREGIND:
		rhs_value = (void const *)((uintptr_t)rhs_value - memloc_hregind_getvaloff(lhs));
		return fg_gjcc_regindCconst(self,
		                            memloc_hregind_getreg(lhs),
		                            memloc_hregind_getindoff(lhs), rhs_value,
		                            signed_cmp, dst_lo, dst_eq, dst_gr);
	case MEMADR_TYPE_HSTACKIND:
		rhs_value = (void const *)((uintptr_t)rhs_value - memloc_hstackind_getvaloff(lhs));
		return fg_gjcc_hstackindCconst(self,
		                               memloc_hstackind_getcfa(lhs), rhs_value,
		                               signed_cmp, dst_lo, dst_eq, dst_gr);
	}
	__builtin_unreachable();
err:
	return -1;
}


#undef NEED_fungen_gjccA_reg
#ifndef HAVE__fungen_gjcc_regAreg
#define NEED_fungen_gjccA_reg
#endif /* !HAVE__fungen_gjcc_regAreg */
#ifndef HAVE__fungen_gjcc_regindAreg
#define NEED_fungen_gjccA_reg
#endif /* !HAVE__fungen_gjcc_regindAreg */
#ifndef HAVE__fungen_gjcc_hstackindAreg
#define NEED_fungen_gjccA_reg
#endif /* !HAVE__fungen_gjcc_hstackindAreg */
#ifndef HAVE__fungen_gjcc_regAconst
#define NEED_fungen_gjccA_reg
#endif /* !HAVE__fungen_gjcc_regAconst */

#ifdef NEED_fungen_gjccA_reg
PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_gjccA_reg(struct fungen *__restrict self, host_regno_t regno,
             struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	if (dst_nz) {
		if (dst_z) {
			if (dst_nz != dst_z)
				DO(fg_gjnz_reg(self, regno, dst_nz));
			return fg_gjmp(self, dst_z);
		}
		return fg_gjnz_reg(self, regno, dst_nz);
	} else if (dst_z) {
		return fg_gjz_reg(self, regno, dst_z);
	}
	return 0;
err:
	return -1;
}
#endif /* NEED_fungen_gjccA_reg */


/* Conditional jump based on "(<lhs> & <rhs>) !=/= 0" */
#ifndef HAVE__fungen_gjcc_regAreg
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjcc_regAreg(struct fungen *__restrict self,
                host_regno_t lhs_regno, host_regno_t rhs_regno,
                struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	host_regno_t dst_regno = fg_gallocreg(self, NULL);
	if unlikely(dst_regno >= HOST_SIZEOF_POINTER)
		goto err;
	DO(fg_gbitop_regreg2reg(self, BITOP_AND, lhs_regno, rhs_regno, dst_regno));
	return fg_gjccA_reg(self, dst_regno, dst_nz, dst_z);
err:
	return -1;
}
#endif /* !HAVE__fungen_gjcc_regAreg */

#ifndef HAVE__fungen_gjcc_regindAreg
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjcc_regindAreg(struct fungen *__restrict self,
                   host_regno_t lhs_regno, ptrdiff_t lhs_ind_delta,
                   host_regno_t rhs_regno,
                   struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	host_regno_t dst_regno = fg_gallocreg(self, NULL);
	if unlikely(dst_regno >= HOST_SIZEOF_POINTER)
		goto err;
	DO(fg_gbitop_regregind2reg(self, BITOP_AND, rhs_regno, lhs_regno, lhs_ind_delta, dst_regno));
	return fg_gjccA_reg(self, dst_regno, dst_nz, dst_z);
err:
	return -1;
}
#endif /* !HAVE__fungen_gjcc_regindAreg */

#ifndef HAVE__fungen_gjcc_hstackindAreg
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjcc_hstackindAreg(struct fungen *__restrict self,
                      host_cfa_t lhs_cfa_offset, host_regno_t rhs_regno,
                      struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	host_regno_t dst_regno = fg_gallocreg(self, NULL);
	if unlikely(dst_regno >= HOST_SIZEOF_POINTER)
		goto err;
	DO(fg_gbitop_reghstackind2reg(self, BITOP_AND, rhs_regno, lhs_cfa_offset, dst_regno));
	return fg_gjccA_reg(self, dst_regno, dst_nz, dst_z);
err:
	return -1;
}
#endif /* !HAVE__fungen_gjcc_hstackindAreg */

#ifndef HAVE__fungen_gjcc_regindAconst
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjcc_regindAconst(struct fungen *__restrict self,
                     host_regno_t lhs_regno, ptrdiff_t lhs_ind_delta, void const *rhs_value,
                     struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	host_regno_t tempreg = fg_gallocreg(self, NULL);
	if unlikely(tempreg >= HOST_REGNO_COUNT)
		goto err;
	DO(fg_gmov_regind2reg(self, lhs_regno, lhs_ind_delta, tempreg));
	return fg_gjcc_regAconst(self, tempreg, rhs_value, dst_nz, dst_z);
err:
	return -1;
}
#elif defined(_fungen_gjcc_regindAconst_MAYFAIL)
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjcc_regindAconst(struct fungen *__restrict self,
                     host_regno_t lhs_regno, ptrdiff_t lhs_ind_delta, void const *rhs_value,
                     struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	int result = _fungen_gjcc_regindAconst(self, lhs_regno, lhs_ind_delta, rhs_value, dst_nz, dst_z);
	if unlikely(result > 0) {
		host_regno_t tempreg, not_these[2];
		not_these[0] = lhs_regno;
		not_these[1] = HOST_REGNO_COUNT;
		tempreg = fg_gconst_as_reg(self, rhs_value, not_these);
		if unlikely(tempreg >= HOST_REGNO_COUNT)
			return -1;
		return fg_gjcc_regindAreg(self, lhs_regno, lhs_ind_delta, tempreg, dst_nz, dst_z);
	}
	return result;
}
#endif /* !HAVE__fungen_gjcc_regindAconst || _fungen_gjcc_regindAconst_MAYFAIL */

#ifndef HAVE__fungen_gjcc_regAconst
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjcc_regAconst(struct fungen *__restrict self,
                  host_regno_t lhs_regno, void const *rhs_value,
                  struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	host_regno_t dst_regno = fg_gallocreg(self, NULL);
	if unlikely(dst_regno >= HOST_REGNO_COUNT)
		goto err;
	DO(fg_gbitop_regconst2reg(self, BITOP_AND, lhs_regno, rhs_value, dst_regno));
	return fg_gjccA_reg(self, dst_regno, dst_nz, dst_z);
err:
	return -1;
}
#elif defined(_fungen_gjcc_regAconst_MAYFAIL)
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjcc_regAconst(struct fungen *__restrict self,
                  host_regno_t lhs_regno, void const *rhs_value,
                  struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	int result = _fungen_gjcc_regAconst(self, lhs_regno, rhs_value, dst_nz, dst_z);
	if unlikely(result > 0) {
		host_regno_t tempreg, not_these[2];
		not_these[0] = lhs_regno;
		not_these[1] = HOST_REGNO_COUNT;
		tempreg = fg_gconst_as_reg(self, rhs_value, not_these);
		if unlikely(tempreg >= HOST_REGNO_COUNT)
			return -1;
		return fg_gjcc_regAreg(self, lhs_regno, tempreg, dst_nz, dst_z);
	}
	return result;
}
#endif /* !HAVE__fungen_gjcc_regAconst || _fungen_gjcc_regAconst_MAYFAIL */

#ifndef HAVE__fungen_gjcc_hstackindAconst
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjcc_hstackindAconst(struct fungen *__restrict self,
                        host_cfa_t lhs_cfa_offset, void const *rhs_value,
                        struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	host_regno_t tempreg = fg_gallocreg(self, NULL);
	if unlikely(tempreg >= HOST_REGNO_COUNT)
		goto err;
	DO(fg_gmov_hstackind2reg(self, lhs_cfa_offset, tempreg));
	return fg_gjcc_regAconst(self, tempreg, rhs_value, dst_nz, dst_z);
err:
	return -1;
}
#elif defined(_fungen_gjcc_hstackindAconst_MAYFAIL)
INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjcc_hstackindAconst(struct fungen *__restrict self,
                        host_cfa_t lhs_cfa_offset, void const *rhs_value,
                        struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	int result = _fungen_gjcc_hstackindAconst(self, lhs_cfa_offset, rhs_value, dst_nz, dst_z);
	if unlikely(result > 0) {
		host_regno_t tempreg;
		tempreg = fg_gconst_as_reg(self, rhs_value, NULL);
		if unlikely(tempreg >= HOST_REGNO_COUNT)
			return -1;
		return fg_gjcc_hstackindAreg(self, lhs_cfa_offset, tempreg, dst_nz, dst_z);
	}
	return result;
}
#endif /* !HAVE__fungen_gjcc_hstackindAconst || _fungen_gjcc_hstackindAconst_MAYFAIL */


INTERN WUNUSED NONNULL((1)) int DCALL
fg_gjcc_regxAregx(struct fungen *__restrict self,
                  host_regno_t lhs_regno, ptrdiff_t lhs_val_offset,
                  host_regno_t rhs_regno, ptrdiff_t rhs_val_offset,
                  struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	if (lhs_val_offset != 0 || rhs_val_offset != 0) {
		if (lhs_regno == rhs_regno) {
			host_regno_t temp, not_these[2];
			/* Need to use a temporary register */
			not_these[0] = lhs_regno;
			not_these[1] = HOST_REGNO_COUNT;
			temp = fg_gallocreg(self, not_these);
			if unlikely(temp >= HOST_REGNO_COUNT)
				goto err;
			if (lhs_val_offset != 0) {
				DO(fg_gmov_regx2reg(self, lhs_regno, lhs_val_offset, temp));
				lhs_regno = temp;
				lhs_val_offset = 0;
			} else {
				DO(fg_gmov_regx2reg(self, rhs_regno, rhs_val_offset, temp));
				rhs_regno = temp;
				rhs_val_offset = 0;
			}
		}
		if (lhs_val_offset != 0) {
			DO(fg_gmov_regx2reg(self, lhs_regno, lhs_val_offset, lhs_regno));
			memstate_hregs_adjust_delta(self->fg_state, lhs_regno, lhs_val_offset);
			/*lhs_val_offset = 0;*/
		}
		if (rhs_val_offset != 0) {
			DO(fg_gmov_regx2reg(self, rhs_regno, rhs_val_offset, rhs_regno));
			memstate_hregs_adjust_delta(self->fg_state, rhs_regno, rhs_val_offset);
			/*rhs_val_offset = 0;*/
		}
	}
	return fg_gjcc_regAreg(self, lhs_regno, rhs_regno, dst_nz, dst_z);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gjcc_locAregx(struct fungen *__restrict self, struct memloc const *lhs,
                 host_regno_t rhs_regno, ptrdiff_t rhs_val_offset,
                 struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	struct memloc lhs_asreg;
	switch (memloc_gettyp(lhs)) {
	default: {
		host_regno_t not_these[2];
fallback:
		not_these[0] = rhs_regno;
		not_these[1] = HOST_REGNO_COUNT;
		DO(fg_gasreg(self, lhs, &lhs_asreg, not_these));
		lhs = &lhs_asreg;
	}
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return fg_gjcc_regxAregx(self,
		                         memloc_hreg_getreg(lhs),
		                         memloc_hreg_getvaloff(lhs),
		                         rhs_regno, rhs_val_offset,
		                         dst_nz, dst_z);

	case MEMADR_TYPE_HREGIND: {
		host_regno_t lhs_regno = memloc_hregind_getreg(lhs);
		ptrdiff_t lhs_indoff   = memloc_hregind_getindoff(lhs);
		if (memloc_hregind_getvaloff(lhs) != 0)
			goto fallback;
		if (rhs_val_offset != 0) {
			DO(fg_gmov_regx2reg(self, rhs_regno, rhs_val_offset, rhs_regno));
			memstate_hregs_adjust_delta(self->fg_state, rhs_regno, rhs_val_offset);
			if (lhs_regno == rhs_regno)
				lhs_indoff += rhs_val_offset;
		}
		return fg_gjcc_regindAreg(self, lhs_regno, lhs_indoff,
		                          rhs_regno, dst_nz, dst_z);
	}	break;

	case MEMADR_TYPE_HSTACKIND: {
		if (memloc_hstackind_getvaloff(lhs) != 0)
			goto fallback;
		if (rhs_val_offset != 0) {
			DO(fg_gmov_regx2reg(self, rhs_regno, rhs_val_offset, rhs_regno));
			memstate_hregs_adjust_delta(self->fg_state, rhs_regno, rhs_val_offset);
		}
		return fg_gjcc_hstackindAreg(self,
		                             memloc_hstackind_getcfa(lhs),
		                             rhs_regno, dst_nz, dst_z);
	}	break;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gjcc_locAconst(struct fungen *__restrict self,
                  struct memloc const *lhs, void const *rhs_value,
                  struct host_symbol *dst_nz, struct host_symbol *dst_z) {
	struct memloc lhs_asreg;
	switch (memloc_gettyp(lhs)) {
	default:
fallback:
		DO(fg_gasreg(self, lhs, &lhs_asreg, NULL));
		lhs = &lhs_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG: {
		host_regno_t lhs_regno = memloc_hreg_getreg(lhs);
		ptrdiff_t lhs_valoff   = memloc_hreg_getvaloff(lhs);
		if (lhs_valoff != 0) {
			DO(fg_gmov_regx2reg(self, lhs_regno, lhs_valoff, lhs_regno));
			memstate_hregs_adjust_delta(self->fg_state, lhs_regno, lhs_valoff);
			/*lhs_valoff = 0;*/
		}
		return fg_gjcc_regAconst(self, lhs_regno, rhs_value, dst_nz, dst_z);
	}	break;

	case MEMADR_TYPE_HREGIND:
		if (memloc_hregind_getvaloff(lhs) != 0)
			goto fallback;
		return fg_gjcc_regindAconst(self,
		                            memloc_hregind_getreg(lhs),
		                            memloc_hregind_getindoff(lhs),
		                            rhs_value, dst_nz, dst_z);

	case MEMADR_TYPE_HSTACKIND:
		if (memloc_hstackind_getvaloff(lhs) != 0)
			goto fallback;
		return fg_gjcc_hstackindAconst(self,
		                               memloc_hstackind_getcfa(lhs),
		                               rhs_value, dst_nz, dst_z);
	}
	__builtin_unreachable();
err:
	return -1;
}




/* Generate jumps. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
fg_gjz(struct fungen *__restrict self,
       struct memloc const *test_loc,
       struct host_symbol *__restrict dst) {
	struct memloc test_loc_asreg;
	switch (memloc_gettyp(test_loc)) {
	default:
		if unlikely(fg_gasreg(self, test_loc, &test_loc_asreg, NULL))
			goto err;
		test_loc = &test_loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		if (memloc_hreg_getvaloff(test_loc) == 0)
			return fg_gjz_reg(self, memloc_hreg_getreg(test_loc), dst);
		return fg_gjcc_regCconst(self, memloc_hreg_getreg(test_loc),
		                         (void const *)(uintptr_t)(intptr_t)-memloc_hreg_getvaloff(test_loc),
		                         false, NULL, dst, NULL);
	case MEMADR_TYPE_HREGIND:
		if (memloc_hregind_getvaloff(test_loc) == 0) {
			return fg_gjz_regind(self,
			                     memloc_hregind_getreg(test_loc),
			                     memloc_hregind_getindoff(test_loc),
			                     dst);
		}
		return fg_gjcc_regindCconst(self,
		                            memloc_hregind_getreg(test_loc),
		                            memloc_hregind_getindoff(test_loc),
		                            (void const *)(uintptr_t)(-memloc_hregind_getvaloff(test_loc)),
		                            false, NULL, dst, NULL);
	case MEMADR_TYPE_HSTACKIND:
		if (memloc_hstackind_getvaloff(test_loc) == 0)
			return fg_gjz_hstackind(self, memloc_hstackind_getcfa(test_loc), dst);
		return fg_gjcc_hstackindCconst(self, memloc_hstackind_getcfa(test_loc),
		                               (void const *)(uintptr_t)(-memloc_hstackind_getvaloff(test_loc)),
		                               false, NULL, dst, NULL);
	case MEMADR_TYPE_UNDEFINED:
		break;
	case MEMADR_TYPE_CONST:
		if ((uintptr_t)memloc_const_getaddr(test_loc) == 0)
			return fg_gjmp(self, dst);
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HSTACK: /* Never zero */
		return 0;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
fg_gjnz(struct fungen *__restrict self,
        struct memloc const *test_loc,
        struct host_symbol *__restrict dst) {
	struct memloc test_loc_asreg;
	switch (memloc_gettyp(test_loc)) {
	default:
		if unlikely(fg_gasreg(self, test_loc, &test_loc_asreg, NULL))
			goto err;
		test_loc = &test_loc_asreg;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		if (memloc_hreg_getvaloff(test_loc) == 0)
			return fg_gjnz_reg(self, memloc_hreg_getreg(test_loc), dst);
		return fg_gjcc_regCconst(self, memloc_hreg_getreg(test_loc),
		                         (void const *)(uintptr_t)(intptr_t)-memloc_hreg_getvaloff(test_loc),
		                         false, dst, NULL, dst);
	case MEMADR_TYPE_HREGIND:
		if (memloc_hregind_getvaloff(test_loc) == 0) {
			return fg_gjnz_regind(self,
			                      memloc_hregind_getreg(test_loc),
			                      memloc_hregind_getindoff(test_loc),
			                      dst);
		}
		return fg_gjcc_regindCconst(self,
		                            memloc_hregind_getreg(test_loc),
		                            memloc_hregind_getindoff(test_loc),
		                            (void const *)(uintptr_t)(-memloc_hregind_getvaloff(test_loc)),
		                            false, dst, NULL, dst);
	case MEMADR_TYPE_HSTACKIND:
		if (memloc_hstackind_getvaloff(test_loc) == 0)
			return fg_gjnz_hstackind(self, memloc_hstackind_getcfa(test_loc), dst);
		return fg_gjcc_hstackindCconst(self, memloc_hstackind_getcfa(test_loc),
		                               (void const *)(uintptr_t)(-memloc_hstackind_getvaloff(test_loc)),
		                               false, dst, NULL, dst);
	case MEMADR_TYPE_UNDEFINED:
		break;
	case MEMADR_TYPE_CONST:
		if ((uintptr_t)memloc_const_getaddr(test_loc) == 0)
			return 0;
		ATTR_FALLTHROUGH
	case MEMADR_TYPE_HSTACK: /* Never zero */
		return fg_gjmp(self, dst);
	}
	return 0;
err:
	return -1;
}


/* Emit conditional jump(s) based on `<lhs> <=> <rhs>'
 * NOTE: This function may clobber `lhs' and `rhs', and may flush/shift local/stack locations. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
fg_gjcc(struct fungen *__restrict self,
        struct memloc const *lhs, struct memloc const *rhs, bool signed_cmp,
        struct host_symbol *dst_lo,   /* Jump here if `<lhs> < <rhs>' */
        struct host_symbol *dst_eq,   /* Jump here if `<lhs> == <rhs>' */
        struct host_symbol *dst_gr) { /* Jump here if `<lhs> > <rhs>' */
	struct memloc rhs_asreg;

	/* Swap operands if "rhs" isn't CONST or REG, or lhs is CONST */
	if ((memloc_gettyp(lhs) == MEMADR_TYPE_CONST) ||
	    (memloc_gettyp(rhs) != MEMADR_TYPE_CONST && memloc_gettyp(rhs) != MEMADR_TYPE_HREG)) {
#define Tswap(T, a, b) do { T _temp = a; a = b; b = _temp; } __WHILE0
		Tswap(struct memloc const *, lhs, rhs);
		Tswap(struct host_symbol *, dst_lo, dst_gr);
#undef Tswap
	}

	/* Special case: if both operands share the same underlying address,
	 *               then the compare is compile-time constant and the
	 *               jump happens based on offset-deltas. */
	if (memloc_sameadr(lhs, rhs)) {
		ptrdiff_t offset_delta = memloc_getoff(lhs) -
		                         memloc_getoff(rhs);
		struct host_symbol *dst;
		if (offset_delta < 0) {
			dst = dst_lo;
		} else if (offset_delta > 0) {
			dst = dst_gr;
		} else {
			dst = dst_eq;
		}
		return dst ? fg_gjmp(self, dst) : 0;
	}

	/* Branch based on the rhs operand's typing. */
	switch (memloc_gettyp(rhs)) {
	default: {
		host_regno_t not_these[2];
		not_these[0] = HOST_REGNO_COUNT;
		not_these[1] = HOST_REGNO_COUNT;
		if (memloc_hasreg(lhs))
			not_these[0] = memloc_getreg(lhs);
		if unlikely(fg_gasreg(self, rhs, &rhs_asreg, not_these))
			goto err;
		rhs = &rhs_asreg;
	}	ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return fg_gjcc_locCregx(self, lhs,
		                        memloc_hreg_getreg(rhs),
		                        memloc_hreg_getvaloff(rhs),
		                        signed_cmp, dst_lo, dst_eq, dst_gr);
	case MEMADR_TYPE_CONST:
		return fg_gjcc_locCconst(self, lhs, memloc_const_getaddr(rhs),
		                         signed_cmp, dst_lo, dst_eq, dst_gr);
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
fg_gjca(struct fungen *__restrict self,
        struct memloc const *lhs, struct memloc const *rhs,
        struct host_symbol *dst_nz,  /* Jump here if `(<lhs> & <rhs>) != 0' */
        struct host_symbol *dst_z) { /* Jump here if `(<lhs> & <rhs>) == 0' */
	struct memloc rhs_asreg;

	/* Swap operands if "rhs" isn't CONST or REG, or lhs is CONST */
	if ((memloc_gettyp(lhs) == MEMADR_TYPE_CONST) ||
	    (memloc_gettyp(rhs) != MEMADR_TYPE_CONST && memloc_gettyp(rhs) != MEMADR_TYPE_HREG)) {
#define Tswap(T, a, b) do { T _temp = a; a = b; b = _temp; } __WHILE0
		Tswap(struct memloc const *, lhs, rhs);
#undef Tswap
	}

	/* Special case: if both operands describe the same location,
	 *               then the result is the same as doing a z/nz
	 *               jump based on either one of the operands. */
	if (memloc_sameloc(lhs, rhs)) {
		if (dst_nz) {
			if (dst_z) {
				if (dst_nz != dst_z)
					DO(fg_gjnz(self, lhs, dst_nz));
				return fg_gjmp(self, dst_z);
			}
			return fg_gjnz(self, lhs, dst_nz);
		} else if (dst_z) {
			return fg_gjz(self, lhs, dst_z);
		}
		return 0;
	}

	/* Branch based on the rhs operand's typing. */
	switch (memloc_gettyp(rhs)) {
	default: {
		host_regno_t not_these[2];
		not_these[0] = HOST_REGNO_COUNT;
		not_these[1] = HOST_REGNO_COUNT;
		if (memloc_hasreg(lhs))
			not_these[0] = memloc_getreg(lhs);
		if unlikely(fg_gasreg(self, rhs, &rhs_asreg, not_these))
			goto err;
		rhs = &rhs_asreg;
	}	ATTR_FALLTHROUGH
	case MEMADR_TYPE_HREG:
		return fg_gjcc_locAregx(self, lhs,
		                        memloc_hreg_getreg(rhs),
		                        memloc_hreg_getvaloff(rhs),
		                        dst_nz, dst_z);
	case MEMADR_TYPE_CONST:
		return fg_gjcc_locAconst(self, lhs, memloc_const_getaddr(rhs),
		                         dst_nz, dst_z);
	}
	__builtin_unreachable();
err:
	return -1;
}

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_COMMON_C */
