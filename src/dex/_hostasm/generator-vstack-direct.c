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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_VSTACK_DIRECT_C
#define GUARD_DEX_HOSTASM_GENERATOR_VSTACK_DIRECT_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/alloc.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include "utils.h"

DECL_BEGIN

/************************************************************************/
/* VSTACK DIRECT VALUE CONVERSION                                       */
/************************************************************************/

#define DeeInt_NEWSFUNC(n) DEE_PRIVATE_NEWINT(n)
#define DeeInt_NEWUFUNC(n) DEE_PRIVATE_NEWUINT(n)

/* Try to figure out the guarantied runtime object type of `vdirect()' */
INTERN ATTR_PURE WUNUSED NONNULL((1)) DeeTypeObject *DCALL
Dee_memval_typeof(struct Dee_memval const *self) {
	switch (self->mv_vmorph) {
	case MEMVAL_VMORPH_DIRECT:
	case MEMVAL_VMORPH_DIRECT_01:
		return Dee_memobj_typeof(Dee_memval_direct_getobj(self));
	case MEMVAL_VMORPH_BOOL_Z:
	case MEMVAL_VMORPH_BOOL_Z_01:
	case MEMVAL_VMORPH_BOOL_NZ:
	case MEMVAL_VMORPH_BOOL_NZ_01:
		return &DeeBool_Type;
	case MEMVAL_VMORPH_INT:
	case MEMVAL_VMORPH_UINT:
		return &DeeInt_Type;
	default: break;
	}
	return NULL;
}


/* Possible propagation strategies returned by `Dee_function_generator_vdirect_impl()'
 * These affect how/which aliases of the (previously) non-direct value are updated to
 * reflect the (then) direct equivalent of their original value.
 *
 * Different strategies need to be used for immutable vs. mutable objects:
 * >> local t1 = (foo, bar); // MEMVAL_VMORPH_TUPLE
 * >> local t2 = (foo, bar); // MEMVAL_VMORPH_TUPLE
 * >> local t3 = t1;         // MEMVAL_VMORPH_TUPLE  (alias)
 * >> local l1 = [foo, bar]; // MEMVAL_VMORPH_LIST
 * >> local l2 = [foo, bar]; // MEMVAL_VMORPH_LIST
 * >> local l3 = l1;         // MEMVAL_VMORPH_LIST  (alias)
 * >> cb(t1); // vdirect() is allowed to write to "t1", "t2" and "t3"
 * >> cb(l1); // vdirect() is allowed to write to "l1" and "l3" (but *NOT* l2, even though that has the same value as "l1")
 */
#define MAKEDIRECT_PROPAGATE_STRATEGY_SAMEVAL  0 /* For immutable objects: copy result to all non-direct, identical values */
#define MAKEDIRECT_PROPAGATE_STRATEGY_SAMECOPY 1 /* For mutable objects: copy result to all values that refer to the same copy (only when Dee_memobjs was used) */

/* non_direct -> direct */
PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdirect_impl(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *mval;
	ASSERT(self->fg_state->ms_stackc >= 1);
	mval = &self->fg_state->ms_stackv[self->fg_state->ms_stackc - 1];
	ASSERT(!Dee_memval_isdirect(mval));

	/* Convert `mval' (which is located in VTOP) into a DIRECT value. */
	switch (mval->mv_vmorph) {

	case MEMVAL_VMORPH_NULLABLE: {
		uint8_t saved_flags = mval->mv_obj.mvo_0.mo_flags;
		Dee_memobj_clearref(&mval->mv_obj.mvo_0);
		if unlikely(Dee_function_generator_gjz_except(self, &mval->mv_obj.mvo_0.mo_loc))
			goto err;
		mval->mv_obj.mvo_0.mo_flags = saved_flags;
		mval->mv_vmorph = MEMVAL_VMORPH_DIRECT;
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
		if (Dee_memobj_gettyp(&mval->mv_obj.mvo_0) == MEMADR_TYPE_HREG) {
			retreg = Dee_memobj_hreg_getreg(&mval->mv_obj.mvo_0);
		} else {
			Dee_host_register_t not_these[2];
			not_these[0] = HOST_REGISTER_COUNT;
			not_these[1] = HOST_REGISTER_COUNT;
			if (Dee_memobj_hasreg(&mval->mv_obj.mvo_0))
				not_these[0] = Dee_memobj_getreg(&mval->mv_obj.mvo_0);
			retreg = Dee_function_generator_gallocreg(self, not_these);
			if unlikely(retreg >= HOST_REGISTER_COUNT)
				goto err;
		}
		if (mval->mv_vmorph == MEMVAL_VMORPH_BOOL_NZ_01) {
			temp = Dee_function_generator_gmorph_loc012regbooly(self,
			                                                    Dee_memobj_getloc(&mval->mv_obj.mvo_0),
			                                                    0, retreg, &retreg_delta);
		} else {
			unsigned int cmp;
			switch (mval->mv_vmorph) {
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
			                                                  Dee_memobj_getloc(&mval->mv_obj.mvo_0),
			                                                  0, cmp, retreg, &retreg_delta);
		}
		if unlikely(temp)
			goto err;
		if (Dee_memstate_ismemvalinstate(self->fg_state, mval)) {
			Dee_memstate_decrinuse_for_memobj(self->fg_state, &mval->mv_obj.mvo_0);
			Dee_memstate_incrinuse(self->fg_state, retreg);
		}
		Dee_memval_init_hreg(mval, retreg, retreg_delta, &DeeBool_Type, MEMOBJ_F_NORMAL);
	}	break;

	case MEMVAL_VMORPH_INT:
	case MEMVAL_VMORPH_UINT: {
		/* Construct a new deemon integer object. */
		void const *api_function;
		api_function = mval->mv_vmorph == MEMVAL_VMORPH_INT
		                  ? (void const *)&DeeInt_NEWSFUNC(HOST_SIZEOF_POINTER)
		                  : (void const *)&DeeInt_NEWUFUNC(HOST_SIZEOF_POINTER);
		mval->mv_vmorph = MEMVAL_VMORPH_DIRECT;
		return Dee_function_generator_vcallapi(self, api_function, VCALL_CC_OBJECT, 1);
	}	break;

	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Unsupported location value type %#" PRFx8,
		                       mval->mv_vmorph);
	}
	return MAKEDIRECT_PROPAGATE_STRATEGY_SAMEVAL;
err:
	return -1;
}

/* Force VTOP to become a direct object. Any memory locations that aliases it is also changed.
 * NOTE: This function is usually called automatically by other `Dee_function_generator_v*' functions. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdirect1(struct Dee_function_generator *__restrict self) {
	int propagation_strategy;
	struct Dee_memval *alias, oldval;
	struct Dee_memval *mval;
	struct Dee_memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = &state->ms_stackv[state->ms_stackc - 1];
	if (Dee_memval_isdirect(mval))
		return 0; /* Simple case! */
	if (Dee_memstate_isshared(state)) {
		state = Dee_memstate_copy(state);
		if unlikely(!state)
			goto err;
		Dee_memstate_decref_nokill(self->fg_state);
		self->fg_state = state;
		mval = &state->ms_stackv[state->ms_stackc - 1];
	}
	Dee_memval_initcopy(&oldval, mval);
	propagation_strategy = Dee_function_generator_vdirect_impl(self);
	if unlikely(propagation_strategy < 0)
		goto err_oldval;
	state = self->fg_state;
	mval  = &state->ms_stackv[state->ms_stackc - 1];
	ASSERT(Dee_memval_isdirect(mval));
	Dee_memstate_foreach(alias, state) {
		/* TODO: This only looks at *primary* storage locations,
		 *       but it should also look at location equivalences! */
		if (!(propagation_strategy == MAKEDIRECT_PROPAGATE_STRATEGY_SAMEVAL
		      ? Dee_memval_sameval_mayalias(alias, &oldval)
		      : Dee_memval_sameval(alias, &oldval)))
			continue;
		if (alias == mval)
			continue;

		/* Object references held by "alias" must be dropped!
		 * NOTE: We can always use *vstack semantics here because
		 *       even in the case of a local variable, that variable
		 *       is known to be a non-direct value, meaning that it
		 *       has to be bound unconditionally! */
		if unlikely(Dee_function_generator_vgdecref_vstack(self, alias))
			goto err_oldval;
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
err:
	return -1;
}


/* Force the top `n' elements of the v-stack to use `MEMVAL_VMORPH_ISDIRECT'.
 * Any memory locations that might alias one of those locations is also changed.
 * NOTE: This function is usually called automatically by other `Dee_function_generator_v*' functions. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdirect(struct Dee_function_generator *__restrict self,
                               Dee_vstackaddr_t n) {
	Dee_vstackaddr_t i;
	struct Dee_memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < n)
		return err_illegal_stack_effect();
	for (i = state->ms_stackc - n; i < state->ms_stackc; ++i) {
		struct Dee_memval *mval = &state->ms_stackv[i];
		if (!Dee_memval_isdirect(mval)) {
			Dee_vstackaddr_t rot_n = state->ms_stackc - i;
			if unlikely(Dee_function_generator_vlrot(self, rot_n))
				goto err;
			if unlikely(Dee_function_generator_vdirect1(self))
				goto err;
			if unlikely(Dee_function_generator_vrrot(self, rot_n))
				goto err;
		}
	}
	return 0;
err:
	return -1;
}

/* Same as (but requires that "n >= 1"):
 * >> Dee_function_generator_vlrot(self, n);
 * >> Dee_function_generator_vdirect1(self);
 * >> Dee_function_generator_vrrot(self, n); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdirect_at(struct Dee_function_generator *__restrict self,
                                  Dee_vstackaddr_t n) {
	int result;
	struct Dee_memval *mval;
	struct Dee_memstate *state = self->fg_state;
	ASSERT(n >= 1);
	if unlikely(state->ms_stackc < n)
		return err_illegal_stack_effect();
	mval = &state->ms_stackv[state->ms_stackc - n];
	if (Dee_memval_isdirect(mval))
		return 0;
	result = Dee_function_generator_vlrot(self, n);
	if likely(result == 0)
		result = Dee_function_generator_vdirect1(self);
	if likely(result == 0)
		result = Dee_function_generator_vrrot(self, n);
	return result;
}

/* Make sure that "val" is direct. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vdirect_memval(struct Dee_function_generator *__restrict self,
                                      struct Dee_memval *val) {
	struct Dee_memval *dst, *src;
	struct Dee_memstate *state;
	if (Dee_memval_isdirect(val))
		return 0;
	state = self->fg_state;
	if (val >= state->ms_stackv && val < state->ms_stackv + state->ms_stackc) {
		Dee_vstackaddr_t addr = (Dee_vstackaddr_t)((state->ms_stackv + state->ms_stackc) - val);
		return Dee_function_generator_vdirect_at(self, addr);
	}
	if (Dee_memstate_isshared(state)) {
		bool islocal = val >= state->ms_localv &&
		               val < state->ms_localv + state->ms_localc;
		if unlikely(Dee_memstate_inplace_copy_because_shared(&self->fg_state))
			goto err;
		if (islocal)
			val = self->fg_state->ms_localv + (val - state->ms_localv);
		state = self->fg_state;
	}
	if unlikely(state->ms_stackc >= state->ms_stacka &&
	            Dee_memstate_reqvstack(state, state->ms_stackc + 1))
		goto err;
	dst = &state->ms_stackv[state->ms_stackc];
	Dee_memval_initmove(dst, val);
	++state->ms_stackc;
	if (val >= state->ms_localv && val < state->ms_localv + state->ms_localc) {
		Dee_lid_t val_lid = (Dee_lid_t)(val - state->ms_localv);
		Dee_memval_init_undefined(val);
		if unlikely(Dee_function_generator_vdirect1(self))
			goto err;
		val = state->ms_localv + val_lid;
		Dee_memstate_decrinuse_for_memval(state, val);
#if 0 /* Already checked and implemented as its own case */
	} else if (val >= state->ms_stackv && val < state->ms_stackv + state->ms_stackc) {
		Dee_vstackaddr_t val_adr = (Dee_vstackaddr_t)(val - state->ms_stackv);
		Dee_memval_init_undefined(val);
		if unlikely(Dee_function_generator_vdirect1(self))
			goto err;
		val = state->ms_stackv + val_adr;
		Dee_memstate_decrinuse_for_memval(state, val);
#endif
	} else {
		Dee_memstate_incrinuse_for_memval(state, val);
		if unlikely(Dee_function_generator_vdirect1(self))
			goto err;
		src = &state->ms_stackv[state->ms_stackc - 1];
		Dee_memstate_decrinuse_for_memval(state, src);
	}

	/* Move the now-direct value back into the caller-given buffer. */
	--state->ms_stackc;
	src = &state->ms_stackv[state->ms_stackc];
	Dee_memval_fini(val);
	Dee_memval_initmove(val, src);
	ASSERT(Dee_memval_isdirect(val));
	return 0;
err:
	return -1;
}



DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_VSTACK_DIRECT_C */
