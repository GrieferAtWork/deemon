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

DECL_BEGIN

/************************************************************************/
/* VSTACK DIRECT VALUE CONVERSION                                       */
/************************************************************************/

#ifdef __INTELLISENSE__
#define DO /* nothing */
#else /* __INTELLISENSE__ */
#define DO(x) if unlikely(x) goto err
#endif /* !__INTELLISENSE__ */
#define EDO(err, x) if unlikely(x) goto err


#define DeeInt_NEWSFUNC(n) DEE_PRIVATE_NEWINT(n)
#define DeeInt_NEWUFUNC(n) DEE_PRIVATE_NEWUINT(n)

/* Try to figure out the guarantied runtime object type of `vdirect()' */
INTERN ATTR_PURE WUNUSED NONNULL((1)) DeeTypeObject *DCALL
memval_typeof(struct memval const *self) {
	switch (self->mv_vmorph) {
	case MEMVAL_VMORPH_DIRECT:
	case MEMVAL_VMORPH_DIRECT_01:
		return memobj_typeof(memval_direct_getobj(self));
	case MEMVAL_VMORPH_BOOL_Z:
	case MEMVAL_VMORPH_BOOL_Z_01:
	case MEMVAL_VMORPH_BOOL_NZ:
	case MEMVAL_VMORPH_BOOL_NZ_01:
		return &DeeBool_Type;
	case MEMVAL_VMORPH_INT:
	case MEMVAL_VMORPH_UINT:
		return &DeeInt_Type;
	case MEMVAL_VMORPH_SUPER:
		return &DeeSuper_Type;
	default: break;
	}
	return NULL;
}


/* Possible propagation strategies returned by `fg_vdirect_impl()'
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
#define MAKEDIRECT_PROPAGATE_STRATEGY_SAMECOPY 1 /* For mutable objects: copy result to all values that refer to the same copy (only when memobjs was used) */

/* non_direct -> direct */
PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_vdirect_impl(struct fungen *__restrict self) {
	struct memval *mval;
	ASSERT(self->fg_state->ms_stackc >= 1);
	mval = &self->fg_state->ms_stackv[self->fg_state->ms_stackc - 1];
	ASSERT(!memval_isdirect(mval));

	/* Convert `mval' (which is located in VTOP) into a DIRECT value. */
	switch (mval->mv_vmorph) {

	case MEMVAL_VMORPH_NULLABLE: {
		uint8_t saved_flags = mval->mv_obj.mvo_0.mo_flags;
		memobj_clearref(&mval->mv_obj.mvo_0);
		DO(fg_gjz_except(self, &mval->mv_obj.mvo_0.mo_loc));
		mval->mv_obj.mvo_0.mo_flags = saved_flags;
		mval->mv_vmorph = MEMVAL_VMORPH_DIRECT;

		/* There can only ever be a single NULLABLE location, so once
		 * that location becomes non-NULL, we know there aren't any
		 * NULLABLE location left (so we can clear the relevant flag) */
		self->fg_state->ms_flags &= ~MEMSTATE_F_GOTNULLABLE;
	}	break;

	case MEMVAL_VMORPH_BOOL_Z:
	case MEMVAL_VMORPH_BOOL_Z_01:
	case MEMVAL_VMORPH_BOOL_NZ:
	case MEMVAL_VMORPH_BOOL_NZ_01:
	case MEMVAL_VMORPH_BOOL_LZ:
	case MEMVAL_VMORPH_BOOL_GZ: {
		int temp;
		host_regno_t retreg;
		ptrdiff_t retreg_delta;
		if (memobj_gettyp(&mval->mv_obj.mvo_0) == MEMADR_TYPE_HREG) {
			retreg = memobj_hreg_getreg(&mval->mv_obj.mvo_0);
		} else {
			host_regno_t not_these[2];
			not_these[0] = HOST_REGNO_COUNT;
			not_these[1] = HOST_REGNO_COUNT;
			if (memobj_hasreg(&mval->mv_obj.mvo_0))
				not_these[0] = memobj_getreg(&mval->mv_obj.mvo_0);
			retreg = fg_gallocreg(self, not_these);
			if unlikely(retreg >= HOST_REGNO_COUNT)
				goto err;
		}
		if (mval->mv_vmorph == MEMVAL_VMORPH_BOOL_NZ_01) {
			temp = fg_gmorph_loc012regbooly(self,
			                                memobj_getloc(&mval->mv_obj.mvo_0),
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
			temp = fg_gmorph_loc2regbooly(self,
			                              memobj_getloc(&mval->mv_obj.mvo_0),
			                              0, cmp, retreg, &retreg_delta);
		}
		if unlikely(temp)
			goto err;
		memstate_decrinuse_for_memobj(self->fg_state, &mval->mv_obj.mvo_0);
		memstate_incrinuse(self->fg_state, retreg);
		memval_init_hreg(mval, retreg, retreg_delta, &DeeBool_Type, MEMOBJ_F_NORMAL);
	}	break;

	case MEMVAL_VMORPH_INT:
	case MEMVAL_VMORPH_UINT: {
		/* Construct a new deemon integer object. */
		void const *api_function;
		api_function = mval->mv_vmorph == MEMVAL_VMORPH_INT
		               ? (void const *)&DeeInt_NEWSFUNC(HOST_SIZEOF_POINTER)
		               : (void const *)&DeeInt_NEWUFUNC(HOST_SIZEOF_POINTER);
		mval->mv_vmorph = MEMVAL_VMORPH_DIRECT;
		return fg_vcallapi(self, api_function, VCALL_CC_OBJECT, 1);
	}	break;

	//TODO:case MEMVAL_VMORPH_LIST:
	//TODO:case MEMVAL_VMORPH_TUPLE:
	//TODO:case MEMVAL_VMORPH_HASHSET:
	//TODO:case MEMVAL_VMORPH_ROSET:
	//TODO:case MEMVAL_VMORPH_DICT:
	//TODO:case MEMVAL_VMORPH_RODICT:
	//TODO:	ASSERT(memval_hasobjn(mval));
	//TODO:	break;

	case MEMVAL_VMORPH_SUPER: {
		struct memobjs *objs;
		ASSERT(memval_hasobjn(mval));
		objs = memval_getobjn(mval);
		ASSERT(objs->mos_objc == 2);
		DO(fg_vcall_DeeObject_MALLOC(self, sizeof(DeeSuperObject), false)); /* super, result */
		DO(fg_vcall_DeeObject_Init_c(self, &DeeSuper_Type));                /* super, result */
		DO(fg_vpush_memobj(self, &objs->mos_objv[1]));                      /* super, result, s_type */
		DO(fg_vref2(self, 3));                                              /* super, result, ref:s_type */
		DO(fg_vpopind(self, offsetof(DeeSuperObject, s_type)));             /* super, result */
		DO(fg_vpush_memobj(self, &objs->mos_objv[0]));                      /* super, result, s_self */
		DO(fg_vref2(self, 3));                                              /* super, result, ref:s_self */
		DO(fg_vpopind(self, offsetof(DeeSuperObject, s_self)));             /* super, result */
		return fg_vpop_at(self, 2);                                         /* result */
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
 * NOTE: This function is usually called automatically by other `fg_v*' functions. */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vdirect1(struct fungen *__restrict self) {
	int propagation_strategy;
	struct memval *alias, oldval;
	struct memval *mval;
	struct memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = &state->ms_stackv[state->ms_stackc - 1];
	if (memval_isdirect(mval))
		return 0; /* Simple case! */
	if (memstate_isshared(state)) {
		state = memstate_copy(state);
		if unlikely(!state)
			goto err;
		memstate_decref_nokill(self->fg_state);
		self->fg_state = state;
		mval = &state->ms_stackv[state->ms_stackc - 1];
	}
	memval_initcopy(&oldval, mval);
	propagation_strategy = fg_vdirect_impl(self);
	if unlikely(propagation_strategy < 0)
		goto err_oldval;
	state = self->fg_state;
	mval  = &state->ms_stackv[state->ms_stackc - 1];
	if (memval_isnullable(mval)) { /* Force nullable to direct */
		int temp = fg_vdirect_impl(self);
		if unlikely(temp < 0)
			goto err_oldval;
		state = self->fg_state;
		mval  = &state->ms_stackv[state->ms_stackc - 1];
	}
	ASSERT(memval_isdirect(mval));
	memstate_foreach(alias, state) {
		/* NOTE: It's OK that this only looks at *primary* storage locations,
		 *       since all memloc-s from the mem-state that are aliases must
		 *       always use the same location!
		 *       i.e.: [#4, %eax] with an equivalence #4 <=> %eax would NOT
		 *             be a valid memory state */
		if (!(propagation_strategy == MAKEDIRECT_PROPAGATE_STRATEGY_SAMEVAL
		      ? memval_sameval_mayalias(alias, &oldval)
		      : memval_sameval(alias, &oldval)))
			continue;
		if (alias == mval)
			continue;

		/* Object references held by "alias" must be dropped!
		 * NOTE: We can always use *vstack semantics here because
		 *       even in the case of a local variable, that variable
		 *       is known to be a non-direct value, meaning that it
		 *       has to be bound unconditionally! */
		EDO(err_oldval, fg_vgdecref_vstack(self, alias));
		memstate_decrinuse_for_memval(state, alias);
		memval_fini(alias);
		memval_direct_initcopy(alias, mval);
		memstate_incrinuse_for_direct_memval(state, mval);
		memval_direct_clearref(alias); /* Aliases don't get references! */
	}
	memstate_foreach_end;
	memval_fini(&oldval);
	return 0;
err_oldval:
	memval_fini(&oldval);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vndirect1(struct fungen *__restrict self) {
	int propagation_strategy;
	struct memval *alias, oldval;
	struct memval *mval;
	struct memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = &state->ms_stackv[state->ms_stackc - 1];
	if (memval_isdirect(mval) || memval_isnullable(mval))
		return 0; /* Simple case! */
	if (memstate_isshared(state)) {
		state = memstate_copy(state);
		if unlikely(!state)
			goto err;
		memstate_decref_nokill(self->fg_state);
		self->fg_state = state;
		mval = &state->ms_stackv[state->ms_stackc - 1];
	}
	memval_initcopy(&oldval, mval);
	propagation_strategy = fg_vdirect_impl(self);
	if unlikely(propagation_strategy < 0)
		goto err_oldval;
	state = self->fg_state;
	mval  = &state->ms_stackv[state->ms_stackc - 1];
	ASSERT(memval_isdirect(mval) || memval_isnullable(mval));
	memstate_foreach(alias, state) {
		/* NOTE: It's OK that this only looks at *primary* storage locations,
		 *       since all memloc-s from the mem-state that are aliases must
		 *       always use the same location!
		 *       i.e.: [#4, %eax] with an equivalence #4 <=> %eax would NOT
		 *             be a valid memory state */
		if (!(propagation_strategy == MAKEDIRECT_PROPAGATE_STRATEGY_SAMEVAL
		      ? memval_sameval_mayalias(alias, &oldval)
		      : memval_sameval(alias, &oldval)))
			continue;
		if (alias == mval)
			continue;

		/* Object references held by "alias" must be dropped!
		 * NOTE: We can always use *vstack semantics here because
		 *       even in the case of a local variable, that variable
		 *       is known to be a non-direct value, meaning that it
		 *       has to be bound unconditionally! */
		EDO(err_oldval, fg_vgdecref_vstack(self, alias));
		memstate_decrinuse_for_memval(state, alias);
		memval_fini(alias);
		memval_direct_initcopy(alias, mval);
		memstate_incrinuse_for_direct_memval(state, mval);
		memval_direct_clearref(alias); /* Aliases don't get references! */
	}
	memstate_foreach_end;
	memval_fini(&oldval);
	return 0;
err_oldval:
	memval_fini(&oldval);
err:
	return -1;
}


/* Force the top `n' elements of the v-stack to use `MEMVAL_VMORPH_ISDIRECT'.
 * Any memory locations that might alias one of those locations is also changed.
 * NOTE: This function is usually called automatically by other `fg_v*' functions. */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vdirect(struct fungen *__restrict self, vstackaddr_t n) {
	vstackaddr_t i;
	struct memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < n)
		return err_illegal_stack_effect();
	for (i = state->ms_stackc - n; i < state->ms_stackc; ++i) {
		struct memval *mval = &state->ms_stackv[i];
		if (!memval_isdirect(mval)) {
			vstackaddr_t rot_n = state->ms_stackc - i;
			DO(fg_vlrot(self, rot_n));
			DO(fg_vdirect1(self));
			DO(fg_vrrot(self, rot_n));
		}
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vndirect(struct fungen *__restrict self, vstackaddr_t n) {
	vstackaddr_t i;
	struct memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < n)
		return err_illegal_stack_effect();
	for (i = state->ms_stackc - n; i < state->ms_stackc; ++i) {
		struct memval *mval = &state->ms_stackv[i];
		if (!memval_isdirect(mval) && !memval_isnullable(mval)) {
			vstackaddr_t rot_n = state->ms_stackc - i;
			DO(fg_vlrot(self, rot_n));
			DO(fg_vndirect1(self));
			DO(fg_vrrot(self, rot_n));
		}
	}
	return 0;
err:
	return -1;
}

/* Same as (but requires that "n >= 1"):
 * >> fg_vlrot(self, n);
 * >> fg_vdirect1(self);
 * >> fg_vrrot(self, n); */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vdirect_at(struct fungen *__restrict self, vstackaddr_t n) {
	int result;
	struct memval *mval;
	struct memstate *state = self->fg_state;
	ASSERT(n >= 1);
	if unlikely(state->ms_stackc < n)
		return err_illegal_stack_effect();
	mval = &state->ms_stackv[state->ms_stackc - n];
	if (memval_isdirect(mval))
		return 0;
	result = fg_vlrot(self, n);
	if likely(result == 0)
		result = fg_vdirect1(self);
	if likely(result == 0)
		result = fg_vrrot(self, n);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vndirect_at(struct fungen *__restrict self, vstackaddr_t n) {
	int result;
	struct memval *mval;
	struct memstate *state = self->fg_state;
	ASSERT(n >= 1);
	if unlikely(state->ms_stackc < n)
		return err_illegal_stack_effect();
	mval = &state->ms_stackv[state->ms_stackc - n];
	if (memval_isdirect(mval) || memval_isnullable(mval))
		return 0;
	result = fg_vlrot(self, n);
	if likely(result == 0)
		result = fg_vndirect1(self);
	if likely(result == 0)
		result = fg_vrrot(self, n);
	return result;
}

/* Make sure that "val" is direct. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vdirect_memval(struct fungen *__restrict self,
                  struct memval *val) {
	struct memval *dst, *src;
	struct memstate *state;
	if (memval_isdirect(val))
		return 0;
	state = self->fg_state;
	if (val >= state->ms_stackv && val < state->ms_stackv + state->ms_stackc) {
		vstackaddr_t addr = (vstackaddr_t)((state->ms_stackv + state->ms_stackc) - val);
		return fg_vdirect_at(self, addr);
	}
	if (memstate_isshared(state)) {
		bool islocal = val >= state->ms_localv &&
		               val < state->ms_localv + state->ms_localc;
		DO(memstate_inplace_copy_because_shared(&self->fg_state));
		if (islocal)
			val = self->fg_state->ms_localv + (val - state->ms_localv);
		state = self->fg_state;
	}
	if unlikely(state->ms_stackc >= state->ms_stacka)
		DO(memstate_reqvstack(state, state->ms_stackc + 1));
	dst = &state->ms_stackv[state->ms_stackc];
	memval_initmove(dst, val);
	++state->ms_stackc;
	if (val >= state->ms_localv && val < state->ms_localv + state->ms_localc) {
		lid_t val_lid = (lid_t)(val - state->ms_localv);
		memval_init_undefined(val);
		DO(fg_vdirect1(self));
		val = state->ms_localv + val_lid;
		memstate_decrinuse_for_memval(state, val);
		memval_fini(val);
#if 0 /* Already checked and implemented as its own case */
	} else if (val >= state->ms_stackv && val < state->ms_stackv + state->ms_stackc) {
		vstackaddr_t val_adr = (vstackaddr_t)(val - state->ms_stackv);
		memval_init_undefined(val);
		DO(fg_vdirect1(self));
		val = state->ms_stackv + val_adr;
		memstate_decrinuse_for_memval(state, val);
		memval_fini(val);
#endif
	} else {
		memstate_incrinuse_for_memval(state, dst);
		DO(fg_vdirect1(self));
		src = &state->ms_stackv[state->ms_stackc - 1];
		memstate_decrinuse_for_memval(state, src);
	}

	/* Move the now-direct value back into the caller-given buffer. */
	--state->ms_stackc;
	src = &state->ms_stackv[state->ms_stackc];
	memval_initmove(val, src);
	ASSERT(memval_isdirect(val));
	return 0;
err:
	return -1;
}



DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_VSTACK_DIRECT_C */
