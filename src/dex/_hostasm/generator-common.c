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
#include <deemon/asm.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/tuple.h>
#include <deemon/util/lock.h>

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

	case MEMLOC_VMORPH_BOOL_Z:
	case MEMLOC_VMORPH_BOOL_Z_01:
	case MEMLOC_VMORPH_BOOL_NZ:
	case MEMLOC_VMORPH_BOOL_NZ_01:
	case MEMLOC_VMORPH_BOOL_LZ:
	case MEMLOC_VMORPH_BOOL_GZ: {
		Dee_host_register_t retreg;
		retreg = Dee_function_generator_gallocreg(self, NULL);
		if unlikely(retreg >= HOST_REGISTER_COUNT)
			goto err;
		//TODO: #define MEMLOC_VMORPH_BOOL_Z     2 /* >> value = DeeBool_For(value == 0 ? 1 : 0); */
		//TODO: #define MEMLOC_VMORPH_BOOL_Z_01  3 /* >> value = DeeBool_For({1,0}[value]); */
		//TODO: #define MEMLOC_VMORPH_BOOL_NZ    4 /* >> value = DeeBool_For(value != 0 ? 1 : 0); */
		//TODO: #define MEMLOC_VMORPH_BOOL_NZ_01 5 /* >> value = DeeBool_For(value); */
		//TODO: #define MEMLOC_VMORPH_BOOL_LZ    6 /* >> value = DeeBool_For((intptr_t)value < 0); */
		//TODO: #define MEMLOC_VMORPH_BOOL_GZ    7 /* >> value = DeeBool_For((intptr_t)value > 0); */
		/* TODO */
		return DeeError_NOTIMPLEMENTED();
	}	break;

	case MEMLOC_VMORPH_INT:
	case MEMLOC_VMORPH_UINT: {
		/* Construct a new deemon integer object. */
		if unlikely(_Dee_function_generator_gcallapi(self,
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
	struct Dee_memloc *aliases, *iter, value;
	struct Dee_memstate *state;
	if (MEMLOC_VMORPH_ISDIRECT(loc->ml_vmorph))
		return 0; /* Already a direct value. */

	/* Find value-aliases of `loc' and delete them all (for now)
	 * Reason: When normalizing direct value, be sure to kill all aliases
	 *         *before* calling API function or allocating temp registers.
	 *         Otherwise, aliases will have their non-direct values flushed,
	 *         which is unnecessary as those values will get overwritten
	 *         in the end! */
	value = *loc;
	state = self->fg_state;
	aliases = NULL;
	Dee_memstate_foreach(iter, state) {
		if (Dee_memloc_sameval(iter, &value)) {
			if (MEMLOC_TYPE_HASREG(iter->ml_type))
				Dee_memstate_decrinuse(state, iter->ml_value.v_hreg.r_regno);;
			iter->ml_type   = MEMLOC_TYPE_UNDEFINED;
			iter->ml_vmorph = MEMLOC_VMORPH_DIRECT;
			iter->ml_value._v_next = aliases;
			aliases = iter;
		}
	}
	Dee_memstate_foreach_end;

	/* Force the value to become direct. */
	if unlikely(Dee_function_generator_gdirect_impl(self, &value))
		goto err;

	/* Write the updated value into all aliases. */
	ASSERT(MEMLOC_VMORPH_ISDIRECT(value.ml_vmorph));
	value.ml_flags &= ~MEMLOC_M_LOCAL_BSTATE;
	while (aliases) {
		iter = aliases->ml_value._v_next;
		ASSERT(aliases->ml_type == MEMLOC_TYPE_UNDEFINED);
		*aliases = value;
		aliases->ml_flags |= MEMLOC_F_NOREF; /* Only the original `loc' will be a reference! */
		if (aliases >= state->ms_localv &&
		    aliases < state->ms_localv + state->ms_localc) {
			aliases->ml_flags |= MEMLOC_F_LOCAL_BOUND;
			goto inc_reguse_for_alias;
		} else if (aliases >= state->ms_stackv &&
		           aliases < state->ms_stackv + state->ms_stackc) {
inc_reguse_for_alias:
			if (MEMLOC_TYPE_HASREG(value.ml_type))
				Dee_memstate_incrinuse(state, value.ml_value.v_hreg.r_regno);;
		}
		aliases = iter;
	}

	/* NOTE: If `loc' is part of the tracked state, then register use was already adjusted! */
	*loc = value;
	if (loc >= state->ms_localv &&
	    loc < state->ms_localv + state->ms_localc)
		loc->ml_flags |= MEMLOC_F_LOCAL_BOUND;
	return 0;
err:
	return -1;
}




INTERN WUNUSED NONNULL((1, 3)) int DCALL
Dee_function_generator_gmov_const2loc(struct Dee_function_generator *__restrict self,
                                      DeeObject *value, struct Dee_memloc const *__restrict dst_loc) {
	switch (dst_loc->ml_type) {

	case MEMLOC_TYPE_HSTACKIND: {
		uintptr_t cfa_offset = dst_loc->ml_value.v_hstack.s_cfa;
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		DeeObject *final_value = (DeeObject *)((uintptr_t)value - dst_loc->ml_value.v_hstack.s_off);
		return _Dee_function_generator_gmov_const2hstackind(self, final_value, sp_offset);
	}	break;

	case MEMLOC_TYPE_HREGIND: {
		DeeObject *final_value = (DeeObject *)((uintptr_t)value - dst_loc->ml_value.v_hreg.r_voff);
		return _Dee_function_generator_gmov_const2regind(self, final_value,
		                                                 dst_loc->ml_value.v_hreg.r_regno,
		                                                 dst_loc->ml_value.v_hreg.r_off);
	}	break;

	case MEMLOC_TYPE_HREG: {
		DeeObject *final_value = (DeeObject *)((uintptr_t)value - dst_loc->ml_value.v_hreg.r_off);
		return _Dee_function_generator_gmov_const2reg(self, final_value, dst_loc->ml_value.v_hreg.r_regno);
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
		if unlikely(_Dee_function_generator_gmov_const2reg(self, value, temp_regno))
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
	ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
	switch (dst_loc->ml_type) {

	case MEMLOC_TYPE_HREG: {
		ptrdiff_t src_sp_offset = sp_offset - dst_loc->ml_value.v_hreg.r_off;
		return _Dee_function_generator_gmov_hstack2reg(self, src_sp_offset, dst_loc->ml_value.v_hreg.r_regno);
	}	break;

	default: {
		Dee_host_register_t temp_regno, not_these[2];
		ptrdiff_t src_sp_offset = sp_offset - dst_loc->ml_value.v_hreg.r_voff;
		not_these[0] = HOST_REGISTER_COUNT;
		not_these[1] = HOST_REGISTER_COUNT;
		if (MEMLOC_TYPE_HASREG(dst_loc->ml_type))
			not_these[0] = dst_loc->ml_value.v_hreg.r_regno;
		temp_regno = Dee_function_generator_gallocreg(self, not_these);
		if unlikely(temp_regno >= HOST_REGISTER_COUNT)
			goto err;
		if unlikely(_Dee_function_generator_gmov_hstack2reg(self, src_sp_offset, temp_regno))
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

	case MEMLOC_TYPE_UNDEFINED:
		return 0;

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
		                                                 dst_loc->ml_value.v_hreg.r_off + dst_delta);
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

	case MEMLOC_TYPE_UNDEFINED:
		return 0;

	default: {
		/* Need to use a temporary register. */
		Dee_host_register_t temp_regno;
		Dee_host_register_t not_these[2];
		ptrdiff_t ind_delta;
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
	switch (src_loc->ml_type) {

	case MEMLOC_TYPE_HREG: {
		ptrdiff_t final_delta = src_loc->ml_value.v_hreg.r_off - dst_delta;
		if (final_delta != 0) {
			if unlikely(Dee_function_generator_gmov_regx2reg(self, src_loc->ml_value.v_hreg.r_regno,
			                                                 final_delta, src_loc->ml_value.v_hreg.r_regno))
				goto err;
			Dee_memstate_hregs_adjust_delta(self->fg_state, src_loc->ml_value.v_hreg.r_regno, final_delta);
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
                                           struct Dee_memloc const *__restrict src_loc,
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
			if unlikely(Dee_function_generator_gmov_regx2reg(self, src_loc->ml_value.v_hreg.r_regno,
			                                                 final_delta, src_loc->ml_value.v_hreg.r_regno))
				goto err;
			Dee_memstate_hregs_adjust_delta(self->fg_state, src_loc->ml_value.v_hreg.r_regno, final_delta);
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
                                         struct Dee_memloc const *__restrict src_loc,
                                         DeeObject **p_value, ptrdiff_t dst_delta) {
	switch (src_loc->ml_type) {

	case MEMLOC_TYPE_HREG: {
		ptrdiff_t final_delta = src_loc->ml_value.v_hreg.r_off - dst_delta;
		if (final_delta != 0) {
			if unlikely(Dee_function_generator_gmov_regx2reg(self, src_loc->ml_value.v_hreg.r_regno,
			                                                 final_delta, src_loc->ml_value.v_hreg.r_regno))
				goto err;
			Dee_memstate_hregs_adjust_delta(self->fg_state, src_loc->ml_value.v_hreg.r_regno, final_delta);
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
		result = _Dee_function_generator_gmov_const2regind(self, value,
		                                                   dst_loc->ml_value.v_hreg.r_regno,
		                                                   dst_loc->ml_value.v_hreg.r_off + dst_delta);
		break;

	case MEMLOC_TYPE_HSTACK: {
		uintptr_t cfa_offset = dst_loc->ml_value.v_hstack.s_cfa;
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, cfa_offset);
		result = _Dee_function_generator_gmov_const2hstackind(self, value, sp_offset + dst_delta);
	}	break;

	case MEMLOC_TYPE_CONST: {
		uintptr_t dst_value = (uintptr_t)dst_loc->ml_value.v_const + dst_delta;
		result = _Dee_function_generator_gmov_const2constind(self, value, (DeeObject **)dst_value);
	}	break;

	case MEMLOC_TYPE_UNDEFINED:
		return 0;

	default: {
		/* Need to use a temporary register. */
		Dee_host_register_t temp_regno;
		ptrdiff_t ind_delta;
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
		result = _Dee_function_generator_ghstack_adjust(self, -alloc_delta);
		if unlikely(result != 0)
			goto done;
	}

	/* Generate the arch-specific return instruction sequence. */
	result = _Dee_function_generator_gret(self);
done:
	return result;
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
	size_t i, xloc_base = self->fg_assembler->fa_localc;
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
		cfa_offset = Dee_memstate_hstack_alloca(self->fg_state, HOST_SIZEOF_POINTER);
		if unlikely(_Dee_function_generator_ghstack_pushregind(self, regno, off))
			goto err;
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
 * NOTE: Usage-registers are cleared by arch-specific code (e.g. `_Dee_function_generator_gcallapi()')
 * @param: ignore_top_n_stack_if_not_ref: From the top-most N stack locations, ignore any
 *                                        that don't contain object references.
 * @param: only_if_reference: Only flush locations that contain references. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gflushregs(struct Dee_function_generator *__restrict self,
                                  Dee_vstackaddr_t ignore_top_n_stack_if_not_ref,
                                  bool only_if_reference) {
	size_t i;
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


/* Helper that returns a register that's been populated for `usage' */
INTERN WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_function_generator_gusagereg(struct Dee_function_generator *__restrict self,
                                 Dee_host_regusage_t usage,
                                 Dee_host_register_t const *dont_alloc_these) {
	struct Dee_memstate *state = self->fg_state;
	Dee_host_register_t regno;
	regno = Dee_memstate_hregs_find_usage(state, usage);
	if (regno >= HOST_REGISTER_COUNT) {
		(void)dont_alloc_these;
		/* TODO */
		DeeError_NOTIMPLEMENTED();
		goto err;
	}
	return regno;
err:
	return HOST_REGISTER_COUNT;
}





/* Generate code to assert that location `loc' is non-NULL:
 * >> Dee_function_generator_gassert_bound(self, loc, ASM_LOCAL, lid, <ignored>, NULL, NULL);
 * >> Dee_function_generator_gassert_bound(self, loc, ASM_GLOBAL, gid, <ignored>, NULL, NULL);
 * >> Dee_function_generator_gassert_bound(self, loc, ASM_EXTERN, mid, gid, NULL, NULL);
 * The `kind', `id1' and `id2' arguments simply select `Dee_function_generator_gthrow_*_unbound()'
 * @param: opt_endread_before_throw: When non-NULL, emit `Dee_function_generator_grwlock_endread()'
 *                                   before the `Dee_function_generator_gthrow_*_unbound' code. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gassert_bound(struct Dee_function_generator *__restrict self,
                                     struct Dee_memloc *loc, Dee_instruction_t const *instr,
                                     uint8_t kind, uint16_t id1, uint16_t id2,
                                     Dee_atomic_rwlock_t *opt_endread_before_throw,
                                     Dee_atomic_rwlock_t *opt_endwrite_before_throw) {
	int temp;
	DREF struct Dee_memstate *saved_state;
	struct Dee_host_symbol *target;
	struct Dee_host_section *text_sect;
	struct Dee_host_section *cold_sect;
	ASSERT(kind == ASM_LOCAL || kind == ASM_GLOBAL || kind == ASM_EXTERN);
	target = Dee_function_generator_newsym(self);
	if unlikely(!target)
		goto err;
	text_sect = self->fg_sect;
	cold_sect = text_sect;
	if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE))
		cold_sect = &self->fg_block->bb_hcold;
	if unlikely(text_sect == cold_sect
	            ? _Dee_function_generator_gjnz(self, loc, target)
	            : _Dee_function_generator_gjz(self, loc, target))
		goto err;
	saved_state = self->fg_state;
	Dee_memstate_incref(saved_state);
	if unlikely(Dee_function_generator_state_dounshare(self))
		goto err_saved_state;

	self->fg_sect = cold_sect;
	if (text_sect != cold_sect)
		Dee_host_symbol_setsect(target, cold_sect);

	/* Location isn't bound -> generate code to throw an exception. */
	if (opt_endwrite_before_throw != NULL &&
	    unlikely(Dee_function_generator_grwlock_endwrite_const(self, opt_endwrite_before_throw)))
		goto err_saved_state;
	if (opt_endread_before_throw != NULL &&
	    unlikely(Dee_function_generator_grwlock_endread_const(self, opt_endread_before_throw)))
		goto err_saved_state;
	switch (kind) {
	case ASM_LOCAL:
		temp = Dee_function_generator_gthrow_local_unbound(self, instr, id1);
		break;
	case ASM_GLOBAL:
		temp = Dee_function_generator_gthrow_global_unbound(self, id1);
		break;
	case ASM_EXTERN:
		temp = Dee_function_generator_gthrow_extern_unbound(self, id1, id2);
		break;
	default: __builtin_unreachable();
	}
	if unlikely(temp)
		goto err_saved_state;

	/* Switch back to the original section, and restore the saved mem-state. */
	Dee_memstate_decref(self->fg_state);
	self->fg_state = saved_state;
	self->fg_sect = text_sect;
	ASSERT((text_sect == cold_sect) == !!Dee_host_symbol_isdefined(target));
	if (text_sect == cold_sect)
		Dee_host_symbol_setsect(target, text_sect);
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
                                          Dee_instruction_t const *instr, uint16_t aid) {
	if unlikely(Dee_function_generator_vpush_const(self, (DeeObject *)self->fg_assembler->fa_code))
		goto err;
	if unlikely(Dee_function_generator_vpush_addr(self, instr))
		goto err;
	if unlikely(Dee_function_generator_vpush_imm16(self, aid))
		goto err;
	return Dee_function_generator_vcallapi(self, &libhostasm_rt_err_unbound_arg, VCALLOP_CC_EXCEPT, 3);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gthrow_local_unbound(struct Dee_function_generator *__restrict self,
                                            Dee_instruction_t const *instr, uint16_t lid) {
	if unlikely(Dee_function_generator_vpush_const(self, (DeeObject *)self->fg_assembler->fa_code))
		goto err;
	if unlikely(Dee_function_generator_vpush_addr(self, instr))
		goto err;
	if unlikely(Dee_function_generator_vpush_imm16(self, lid))
		goto err;
	return Dee_function_generator_vcallapi(self, &libhostasm_rt_err_unbound_local, VCALLOP_CC_EXCEPT, 3);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gthrow_global_or_extern_unbound(struct Dee_function_generator *__restrict self,
                                                       DeeModuleObject *mod, uint16_t gid) {
	if unlikely(Dee_function_generator_vpush_const(self, (DeeObject *)mod))
		goto err;
	if unlikely(Dee_function_generator_vpush_imm16(self, gid))
		goto err;
	return Dee_function_generator_vcallapi(self, &libhostasm_rt_err_unbound_global, VCALLOP_CC_EXCEPT, 2);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gthrow_global_unbound(struct Dee_function_generator *__restrict self,
                                             uint16_t gid) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	return Dee_function_generator_gthrow_global_or_extern_unbound(self, mod, gid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gthrow_extern_unbound(struct Dee_function_generator *__restrict self,
                                             uint16_t mid, uint16_t gid) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	ASSERT(mid < mod->mo_importc);
	mod = mod->mo_importv[mid];
	return Dee_function_generator_gthrow_global_or_extern_unbound(self, mod, gid);
}


/* Generate checks to enter exception handling mode. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gjz_except(struct Dee_function_generator *__restrict self,
                                  struct Dee_memloc *loc) {
	struct Dee_except_exitinfo *info;
	info = Dee_function_generator_except_exit(self);
	if unlikely(!info)
		goto err;
	{
		Dee_function_generator_DEFINE_Dee_host_symbol_section(self, err, sym, &info->exi_block->bb_htext, 0);
		return _Dee_function_generator_gjz(self, loc, sym);
	}
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gjnz_except(struct Dee_function_generator *__restrict self,
                                   struct Dee_memloc *loc) {
	struct Dee_except_exitinfo *info;
	info = Dee_function_generator_except_exit(self);
	if unlikely(!info)
		goto err;
	{
		Dee_function_generator_DEFINE_Dee_host_symbol_section(self, err, sym, &info->exi_block->bb_htext, 0);
		return _Dee_function_generator_gjnz(self, loc, sym);
	}
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gjmp_except(struct Dee_function_generator *__restrict self) {
	struct Dee_except_exitinfo *info;
	info = Dee_function_generator_except_exit(self);
	if unlikely(!info)
		goto err;
	{
		Dee_function_generator_DEFINE_Dee_host_symbol_section(self, err, sym, &info->exi_block->bb_htext, 0);
		return _Dee_function_generator_gjmp(self, sym);
	}
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gincref(struct Dee_function_generator *__restrict self,
                               struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gincref_regx(self,
		                                            loc->ml_value.v_hreg.r_regno,
		                                            loc->ml_value.v_hreg.r_off,
		                                            n);
	case MEMLOC_TYPE_CONST:
		return _Dee_function_generator_gincref_const(self, loc->ml_value.v_const, n);
	case MEMLOC_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdecref(struct Dee_function_generator *__restrict self,
                               struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gdecref_regx(self,
		                                            loc->ml_value.v_hreg.r_regno,
		                                            loc->ml_value.v_hreg.r_off,
		                                            n);
	case MEMLOC_TYPE_CONST:
		return _Dee_function_generator_gdecref_const(self, loc->ml_value.v_const, n);
	case MEMLOC_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdecref_nokill(struct Dee_function_generator *__restrict self,
                                      struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gdecref_regx_nokill(self,
		                                                   loc->ml_value.v_hreg.r_regno,
		                                                   loc->ml_value.v_hreg.r_off,
		                                                   n);
	case MEMLOC_TYPE_CONST:
		return _Dee_function_generator_gdecref_const(self, loc->ml_value.v_const, n);
	case MEMLOC_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxincref(struct Dee_function_generator *__restrict self,
                                struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gxincref_regx(self,
		                                             loc->ml_value.v_hreg.r_regno,
		                                             loc->ml_value.v_hreg.r_off,
		                                             n);
	case MEMLOC_TYPE_CONST:
		if (!loc->ml_value.v_const)
			return 0;
		return _Dee_function_generator_gincref_const(self, loc->ml_value.v_const, n);
	case MEMLOC_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxdecref(struct Dee_function_generator *__restrict self,
                                struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gxdecref_regx(self,
		                                             loc->ml_value.v_hreg.r_regno,
		                                             loc->ml_value.v_hreg.r_off,
		                                             n);
	case MEMLOC_TYPE_CONST:
		if (!loc->ml_value.v_const)
			return 0;
		return _Dee_function_generator_gdecref_const(self, loc->ml_value.v_const, n);
	case MEMLOC_TYPE_UNDEFINED:
		return 0;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxdecref_nokill(struct Dee_function_generator *__restrict self,
                                       struct Dee_memloc *__restrict loc, Dee_refcnt_t n) {
	switch (loc->ml_type) {
	default:
		if unlikely(Dee_function_generator_greg(self, loc, NULL))
			goto err;
		ATTR_FALLTHROUGH
	case MEMLOC_TYPE_HREG:
		return _Dee_function_generator_gxdecref_regx_nokill(self,
		                                                    loc->ml_value.v_hreg.r_regno,
		                                                    loc->ml_value.v_hreg.r_off,
		                                                    n);
	case MEMLOC_TYPE_CONST:
		if (!loc->ml_value.v_const)
			return 0;
		return _Dee_function_generator_gdecref_const(self, loc->ml_value.v_const, n);
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
	ASSERTF(loc->ml_flags & MEMLOC_F_NOREF, "Dee_function_generator_gind() called on reference");
	switch (loc->ml_type) {

	case MEMLOC_TYPE_HSTACK:
		loc->ml_type = MEMLOC_TYPE_HSTACKIND;
#ifdef HOSTASM_STACK_GROWS_DOWN
		loc->ml_value.v_hstack.s_cfa -= ind_delta;
#else /* HOSTASM_STACK_GROWS_DOWN */
		loc->ml_value.v_hstack.s_cfa += ind_delta;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		loc->ml_value.v_hstack.s_off = 0;
		return 0;

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
		/* Turn the location from an HREG into HREGIND */
		loc->ml_type = MEMLOC_TYPE_HREGIND;
		loc->ml_value.v_hreg.r_off += ind_delta;
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
	struct Dee_memstate *state;
	Dee_host_register_t regno;
	ptrdiff_t val_delta;
	bool is_in_state;
	if (loc->ml_type == MEMLOC_TYPE_HREG)
		return 0; /* Already in a register! */

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
		if (loc->ml_value.v_hstack.s_off == 0)
			return 0; /* Already on-stack at offset=0 */

		/* TODO: emit `addP $..., sp_offset(%Psp)' to adjust the offset of the stored value
		 *       Afterwards, go through all stack/local variables and adjust value offsets
		 *       wherever the same CFA offset is referenced. */
	}

	/* Figure out where we want to allocate the value. */
#ifdef HAVE_try_restore_xloc_arg_cfa_offset
	if (loc->ml_type == MEMLOC_TYPE_HREG &&
		(cfa_offset = try_restore_xloc_arg_cfa_offset(self, loc->ml_value.v_hreg.r_regno)) != (uintptr_t)-1) {
		/* CFA offset restored */
	} else
#endif /* HAVE_try_restore_xloc_arg_cfa_offset */
	{
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
			/*src_loc->ml_value.v_hreg.r_off = 0;*/
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
