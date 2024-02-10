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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C
#define GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C 1
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

#define DeeInt_NEWSFUNC(n) DEE_PRIVATE_NEWINT(n)
#define DeeInt_NEWUFUNC(n) DEE_PRIVATE_NEWUINT(n)


#ifdef __INTELLISENSE__
#define DO /* nothing */
#else /* __INTELLISENSE__ */
#define DO(x) if unlikely(x) goto err
#endif /* !__INTELLISENSE__ */
#define EDO(err, x) if unlikely(x) goto err


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

/* a,b,c,d -> d,c,b,a */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vmirror(struct Dee_function_generator *__restrict self,
                               Dee_vstackaddr_t n) {
	int result;
	if unlikely(n <= 1)
		return 0;
	result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vmirror(self->fg_state, n);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpush_memadr(struct Dee_function_generator *__restrict self,
                                    struct Dee_memadr const *adr) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_memadr(self->fg_state, adr);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpush_memloc(struct Dee_function_generator *__restrict self,
                                    struct Dee_memloc const *loc) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_memloc(self->fg_state, loc);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpush_memobj(struct Dee_function_generator *__restrict self,
                                    struct Dee_memobj const *obj) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_memobj(self->fg_state, obj);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpush_memval(struct Dee_function_generator *__restrict self,
                                    struct Dee_memval const *val) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_memval(self->fg_state, val);
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
Dee_function_generator_vpush_addr(struct Dee_function_generator *__restrict self,
                                  void const *addr) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_addr(self->fg_state, addr);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpush_const_(struct Dee_function_generator *__restrict self,
                                    DeeObject *value) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_const(self->fg_state, value);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DeeObject *DCALL
Dee_function_generator_getconst(struct Dee_function_generator *__restrict self,
                                uint16_t cid) {
	DeeObject *result;
	DeeCodeObject *code;
	if unlikely(cid >= self->fg_assembler->fa_code->co_staticc) {
		err_illegal_cid(cid);
		goto err;
	}
	code = self->fg_assembler->fa_code;
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_SAFE) {
		/* When needing to be safe, constants could illegally be re-assigned
		 * via statics. This isn't allowed, but if it happens we mustn't crash,
		 * so we have to inline all constants as references. */
		DeeCode_StaticLockRead(code);
		result = code->co_staticv[cid];
		Dee_Incref(result);
		DeeCode_StaticLockEndRead(code);
		result = Dee_function_generator_inlineref(self, result);
		/*if unlikely(!result)
			goto err;*/
	} else {
		result = code->co_staticv[cid];
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_cid(struct Dee_function_generator *__restrict self,
                                 uint16_t cid) {
	DeeObject *constant;
	constant = Dee_function_generator_getconst(self, cid);
	if unlikely(!constant)
		goto err;
	return Dee_function_generator_vpush_const(self, constant);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
Dee_function_generator_vpush_cid_t(struct Dee_function_generator *__restrict self,
                                   uint16_t cid, DeeTypeObject *__restrict type) {
	DeeObject *constant;
	constant = Dee_function_generator_getconst(self, cid);
	if unlikely(!constant)
		goto err;
	DO(DeeObject_AssertTypeExact(constant, type));
	return Dee_function_generator_vpush_const(self, constant);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_rid(struct Dee_function_generator *__restrict self,
                                 uint16_t rid) {
	DeeObject *constant;
	if unlikely(rid >= self->fg_assembler->fa_code->co_refc)
		return err_illegal_rid(rid);
	if (self->fg_assembler->fa_cc & HOSTFUNC_CC_F_FUNC) {
		/* Special case: must access the "fo_refv" field of the runtime "func" parameter. */
		DO(_Dee_function_generator_vpush_xlocal(self, NULL, MEMSTATE_XLOCAL_A_FUNC));
		return Dee_function_generator_vind(self,
		                                   offsetof(DeeFunctionObject, fo_refv) +
		                                   rid * sizeof(DREF DeeObject *));
	}
	/* Simple case: able to directly inline function references */
	constant = self->fg_assembler->fa_function->fo_refv[rid];
	return Dee_function_generator_vpush_const(self, constant);
err:
	return -1;
}


/* Sets the `MEMOBJ_F_NOREF' flag */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_hreg(struct Dee_function_generator *__restrict self,
                                  Dee_host_register_t regno, ptrdiff_t val_delta) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_hreg(self->fg_state, regno, val_delta);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_hregind(struct Dee_function_generator *__restrict self,
                                     Dee_host_register_t regno, ptrdiff_t ind_delta,
                                     ptrdiff_t val_delta) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_hregind(self->fg_state, regno, ind_delta, val_delta);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_hstack(struct Dee_function_generator *__restrict self,
                                    Dee_cfa_t cfa_offset) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_hstack(self->fg_state, cfa_offset);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_hstackind(struct Dee_function_generator *__restrict self,
                                       Dee_cfa_t cfa_offset, ptrdiff_t val_delta) {
	int result = Dee_function_generator_state_unshare(self);
	if likely(result == 0)
		result = Dee_memstate_vpush_hstackind(self->fg_state, cfa_offset, val_delta);
	return result;
}


/* Force "loc" to become HREG, assuming that "loc" is tracked by the mem-state. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
tracked_memloc_forcereg(struct Dee_function_generator *__restrict self,
                        struct Dee_memloc *loc, Dee_host_register_t const *not_these) {
	int result;
	struct Dee_memloc retloc;
	result = Dee_function_generator_gasreg(self, loc, &retloc, not_these);
	if likely(result == 0) {
		Dee_memstate_decrinuse_for_memloc(self->fg_state, loc);
		ASSERT(Dee_memloc_gettyp(&retloc) == MEMADR_TYPE_HREG);
		Dee_memstate_incrinuse(self->fg_state, Dee_memloc_hreg_getreg(&retloc));
		*loc = retloc;
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_arg(struct Dee_function_generator *__restrict self,
                                 Dee_instruction_t const *instr, Dee_aid_t aid) {
	DeeCodeObject *code = self->fg_assembler->fa_code;
	if (aid < code->co_argc_min) /* Simple case: mandatory argument */
		return Dee_function_generator_vpush_arg_present(self, aid);
	if (aid < code->co_argc_max) /* Special case: optional argument (push the x-local) */
		return Dee_function_generator_vpush_xlocal(self, instr, MEMSTATE_XLOCAL_DEFARG(aid - code->co_argc_min));
	return err_illegal_aid(aid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vbound_arg(struct Dee_function_generator *__restrict self, Dee_aid_t aid) {
	DeeCodeObject *code = self->fg_assembler->fa_code;
	if (aid < code->co_argc_min) /* Simple case: mandatory argument */
		return Dee_function_generator_vpush_const(self, Dee_True);
	if (aid < code->co_argc_max) {
		/* Special case: optional argument
		 * NOTE: The normal code executor doesn't look at default values,
		 *       but we are forced to, since cached optional arguments are
		 *       shared with keyword arguments, making it impossible to
		 *       differentiate between a caller-given argument and a cached
		 *       argument with its default value already assigned. */
		struct Dee_memval *mval;
		Dee_aid_t opt_aid = aid - code->co_argc_min;
		Dee_lid_t xlid = MEMSTATE_XLOCAL_DEFARG(opt_aid);
		Dee_lid_t lid = self->fg_assembler->fa_localc + xlid;
		DeeObject *default_value = code->co_defaultv[opt_aid];
		if (default_value != NULL)
			return Dee_function_generator_vpush_const(self, Dee_True);
		mval = &self->fg_state->ms_localv[lid];
		if (!Dee_memval_isdirect(mval)) /* Non-direct values are *always* bound (logically speaking) */
			return Dee_function_generator_vpush_const(self, Dee_True);
		if (Dee_memval_direct_local_neverbound(mval))
			return Dee_function_generator_vpush_const(self, Dee_False);

		/* Check if the argument location has already been allocated. */
		if (!Dee_memval_direct_isundefined(mval)) {
			if (Dee_memval_direct_local_alwaysbound(mval))
				return Dee_function_generator_vpush_const(self, Dee_True);
			DO(Dee_function_generator_vpush_memval(self, mval));
			DO(Dee_function_generator_state_unshare(self));
			mval = Dee_function_generator_vtop(self);
			ASSERT(Dee_memval_isdirect(mval));
			mval->mv_vmorph = MEMVAL_VMORPH_TESTNZ(mval->mv_vmorph);
			return 0;
		}

		/* Argument hasn't been accessed or populated by the keyword loader.
		 * -> Simply check if the argument is present positionally:
		 *         argc >= aid
		 *    <=>  argc > (aid - 1)
		 *    <=>  argc - (aid - 1) > 0
		 */
		DO(Dee_function_generator_vpush_argc(self));
		DO(Dee_function_generator_vdelta(self, -((ptrdiff_t)aid - 1)));
		mval = Dee_function_generator_vtop(self);
		ASSERT(Dee_memval_isdirect(mval));
		mval->mv_vmorph = MEMVAL_VMORPH_BOOL_GZ;
		return 0;
	}
	return err_illegal_aid(aid);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_arg_present(struct Dee_function_generator *__restrict self, Dee_aid_t aid) {
	int result;
	ASSERT(aid < self->fg_assembler->fa_code->co_argc_min);
	result = Dee_function_generator_state_unshare(self);
	if likely(result == 0) {
		STATIC_ASSERT(MEMSTATE_XLOCAL_A_ARGS == MEMSTATE_XLOCAL_A_ARGV);
		struct Dee_memval *args_or_argv_val;
		struct Dee_memloc *args_or_argv_loc;
		ptrdiff_t ind_offset = (ptrdiff_t)aid * sizeof(DeeObject *);
		if (self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE)
			ind_offset += offsetof(DeeTupleObject, t_elem);
		args_or_argv_val = &self->fg_state->ms_localv[self->fg_assembler->fa_localc + MEMSTATE_XLOCAL_A_ARGV];
		if (!Dee_memval_isdirect(args_or_argv_val)) {
			DO(Dee_function_generator_vdirect_memval(self, args_or_argv_val));
			args_or_argv_val = &self->fg_state->ms_localv[self->fg_assembler->fa_localc + MEMSTATE_XLOCAL_A_ARGV];
		}
		args_or_argv_loc = Dee_memval_direct_getloc(args_or_argv_val);
		DO(tracked_memloc_forcereg(self, args_or_argv_loc, NULL));
		ASSERT(Dee_memloc_gettyp(args_or_argv_loc) == MEMADR_TYPE_HREG);
		result = Dee_memstate_vpush_hregind(self->fg_state,
		                                    Dee_memloc_hreg_getreg(args_or_argv_loc),
		                                    Dee_memloc_hreg_getvaloff(args_or_argv_loc) + ind_offset,
		                                    0);
	}
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
delete_unused_local_after_read(struct Dee_function_generator *__restrict self,
                               Dee_instruction_t const *instr, Dee_lid_t lid) {
	if (self->fg_nextlastloc != NULL /*&& instr*/) {
		/* If the caller-given `instr' has an entry that says that this is the last
		 * time `lid' is read from, then automatically delete the local *now*. This
		 * then allows the variable to be deleted earlier than usual:
		 * >>     call global @getValue
		 * >>     pop  local @foo
		 * >>     jt   @local foo, 1f    // If this is the last time "foo" is used, and "1f" doesn't
		 * >>                            // use it, it can be decref'd *before* the jump happens!
		 * >>     ...
		 * >> 1:
		 * Here, we're able to delete "foo" *before* the jump! */
		while (self->fg_nextlastloc->bbl_instr < instr) {
			/* Delete local after the last time it was read. */
			Dee_lid_t delete_lid = self->fg_nextlastloc->bbl_lid;
			DO(Dee_function_generator_vdel_local(self, delete_lid));
			++self->fg_nextlastloc;
		}
		ASSERT(self->fg_nextlastloc->bbl_instr >= instr);
		if (self->fg_nextlastloc->bbl_instr == instr) {
			struct Dee_basic_block_loclastread *item = self->fg_nextlastloc;
			while (item->bbl_instr == instr && item->bbl_lid != lid)
				++item;
			if (item->bbl_instr == instr) {
				ASSERT(item->bbl_lid == lid);
				if (item != self->fg_nextlastloc) {
					struct Dee_basic_block_loclastread temp;
					temp = *item;
					memmoveupc(self->fg_nextlastloc + 1,
					           self->fg_nextlastloc,
					           (size_t)(item - self->fg_nextlastloc),
					           sizeof(struct Dee_basic_block_loclastread));
					item = self->fg_nextlastloc;
					*item = temp;
				}
				ASSERT(item == self->fg_nextlastloc);
				ASSERT(item->bbl_instr == instr);
				ASSERT(item->bbl_lid == lid);
				DO(Dee_function_generator_vdel_local(self, lid));
				++self->fg_nextlastloc;
			}
		}
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_local(struct Dee_function_generator *__restrict self,
                                   Dee_instruction_t const *instr, Dee_lid_t lid) {
	struct Dee_memstate *state;
	struct Dee_memval *dst, *src;
	DO(Dee_function_generator_state_unshare(self));
	state = self->fg_state;
	ASSERT(lid < state->ms_localc);
	if unlikely(state->ms_stackc >= state->ms_stacka)
		DO(Dee_memstate_reqvstack(state, state->ms_stackc + 1));
	src = &state->ms_localv[lid];
	if (instr && Dee_memval_isdirect(src) && !Dee_memval_direct_local_alwaysbound(src)) {
		if (Dee_memval_direct_local_neverbound(src)) {
			/* Variable is always unbound -> generate code to throw an exception */
			DO(Dee_function_generator_gthrow_local_unbound(self, instr, (Dee_ulid_t)lid));
			return Dee_function_generator_vpush_none(self);
		}

		/* Variable is not guarantied bound -> generate code to check if it's bound */
		DO(Dee_function_generator_gassert_local_bound(self, instr, (Dee_ulid_t)lid));

		/* After a bound assertion, the local variable is guarantied to be bound. */
		Dee_memval_direct_local_setbound(src);
	}
	dst = &state->ms_stackv[state->ms_stackc];
	Dee_memval_initcopy(dst, src);
	Dee_memval_clearref(dst);
	Dee_memstate_incrinuse_for_memval(state, dst);
	++state->ms_stackc;
	return delete_unused_local_after_read(self, instr, lid);
err:
	return -1;
}

/* `instr' is needed for automatic deletion of unused locals */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vbound_local(struct Dee_function_generator *__restrict self,
                                    Dee_instruction_t const *instr, Dee_lid_t lid) {
	struct Dee_memstate *state;
	struct Dee_memval *dst, *src;
	DO(Dee_function_generator_state_unshare(self));
	state = self->fg_state;
	ASSERT(lid < state->ms_localc);
	if unlikely(state->ms_stackc >= state->ms_stacka)
		DO(Dee_memstate_reqvstack(state, state->ms_stackc + 1));
	src = &state->ms_localv[lid];
	if (!Dee_memval_isdirect(src)) /* Non-direct values are *always* bound (logically speaking) */
		return Dee_function_generator_vpush_const(self, Dee_True);
	if (Dee_memval_direct_local_alwaysbound(src))
		return Dee_function_generator_vpush_const(self, Dee_True);
	if (Dee_memval_direct_local_neverbound(src))
		return Dee_function_generator_vpush_const(self, Dee_False);
	dst = &state->ms_stackv[state->ms_stackc];
	Dee_memval_direct_initcopy(dst, src);
	Dee_memstate_incrinuse_for_direct_memval(state, src);
	++state->ms_stackc;
	DO(delete_unused_local_after_read(self, instr, lid));
	ASSERT(Dee_memval_isdirect(dst));
	dst->mv_vmorph = MEMVAL_VMORPH_TESTNZ(dst->mv_vmorph);
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

/* Generate code needed to drop references held by `mval' (where `mval' must be a vstack item,
 * or a local variable that is unconditionally bound or non-direct).
 * NOTE: This function is somewhere between the v* and g* APIs, though it does *NOT* unshare or
 *       realloc memstate components. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vgdecref_vstack(struct Dee_function_generator *__restrict self,
                                       struct Dee_memval *mval) {
	struct Dee_memstate *state = self->fg_state;
	struct Dee_memobj *mobj;
	if unlikely(mval->mv_flags & MEMVAL_F_NOREF) {
		ASSERT(!Dee_memval_hasobj0(mval));
		return 0;
	}
	Dee_memval_foreach_obj(mobj, mval) {
		if (Dee_memobj_isref(mobj)) {
			struct Dee_memval *other_mval;
			bool has_ref_alias = false;
			int temp;
	
			/* Try and shift the burden of the reference to the other location. */
			Dee_memstate_foreach(other_mval, state) {
				struct Dee_memobj *other_mobj;
				if unlikely(other_mval->mv_flags & MEMVAL_F_NOREF) {
					ASSERT(!Dee_memval_hasobj0(other_mval));
					continue;
				}
				_Dee_memval_foreach_obj(other, other_mobj, other_mval) {
					if (Dee_memobj_sameloc(other_mobj, mobj) && other_mobj != mobj) {
						if (Dee_memobj_isref(other_mobj)) {
							has_ref_alias = true;
						} else {
							Dee_memobj_setref(other_mobj);
							goto done;
						}
					}
				}
				Dee_memval_foreach_obj_end;
			}
			Dee_memstate_foreach_end;
			if (Dee_memval_isconst(mval))
				has_ref_alias = true; /* Constants are always aliased by static storage */
	
			/* No-where to shift the reference to -> must decref the object ourselves. */
			if (has_ref_alias) {
				temp = Dee_function_generator_gdecref_nokill_loc(self, Dee_memval_direct_getloc(mval), 1);
			} else if (Dee_memobj_isoneref(mobj)) {
				/* TODO: If types are known, inline DeeObject_Destroy() as tp_fini() + DeeType_FreeInstance() */
				temp = Dee_function_generator_gdecref_dokill_loc(self, Dee_memval_direct_getloc(mval));
			} else {
				temp = Dee_function_generator_gdecref_loc(self, Dee_memval_direct_getloc(mval), 1);
			}
			if unlikely(temp)
				goto err;
		}
	}
	Dee_memval_foreach_obj_end;
done:
	return 0;
err:
	return -1;
}

/* Generate code needed to drop references held by `mval' (where `mval' must point into locals)
 * NOTE: This function is somewhere between the v* and g* APIs, though it does *NOT* unshare or
 *       realloc memstate components. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vgdecref_local(struct Dee_function_generator *__restrict self,
                                      struct Dee_memval *__restrict mval) {
	struct Dee_memstate *state = self->fg_state;
	if (!Dee_memval_isdirect(mval)) {
decref_mval:
		return Dee_function_generator_vgdecref_vstack(self, mval);
	} else if (!Dee_memval_direct_isref(mval)) {
		/* Nothing to do here! */
	} else if (Dee_memval_direct_local_neverbound(mval)) {
		/* Nothing to do here! */
	} else if (Dee_memval_direct_local_alwaysbound(mval)) {
		goto decref_mval;
	} else {
		/* Location is conditionally bound.
		 * Check if there is another conditionally bound
		 * location which we can off-load the decref onto. */
		int temp;
		Dee_lid_t i;
		bool has_ref_alias = false;
		struct Dee_memval *other_mval;
		for (i = 0; i < state->ms_localc; ++i) {
			other_mval = &state->ms_localv[i];
			if (other_mval == mval)
				continue;
			if (!Dee_memval_isdirect(other_mval))
				continue;
			if (Dee_memval_direct_local_alwaysbound(other_mval))
				continue; /* Need a conditionally bound alias */
			if (!Dee_memval_direct_sameloc(mval, other_mval))
				continue;
			if (!Dee_memval_direct_isref(other_mval)) {
				/* Off-load reference to here! */
				Dee_memval_direct_clearref(mval);
				Dee_memval_direct_setref(other_mval);
				return 0;
			}
			has_ref_alias = true;
		}
		if (has_ref_alias) {
			temp = Dee_function_generator_gxdecref_nokill_loc(self, Dee_memval_direct_getloc(mval), 1);
		} else {
			temp = Dee_function_generator_gxdecref_loc(self, Dee_memval_direct_getloc(mval), 1);
		}
		return temp;
	}
	return 0;
}


#if 0
/* Wrapper around:
 * - Dee_function_generator_vgdecref_vstack
 * - Dee_function_generator_vgdecref_local
 * ... that automatically checks if `mval' points into the current mem-state's
 * local variable list to see which function needs to be used. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vgdecref(struct Dee_function_generator *__restrict self,
                                struct Dee_memval *__restrict mval) {
	struct Dee_memstate *state = self->fg_state;
	bool is_local = mval >= state->ms_localv &&
	                mval < state->ms_localv + state->ms_localc;
	return is_local ? Dee_function_generator_vgdecref_local(self, mval)
	                : Dee_function_generator_vgdecref_vstack(self, mval);
}
#endif


INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop(struct Dee_function_generator *__restrict self) {
	struct Dee_memstate *state;
	struct Dee_memval *mval;
	DO(Dee_function_generator_state_unshare(self));
	state = self->fg_state;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = Dee_memstate_vtop(state);
	if (mval->mv_vmorph == MEMVAL_VMORPH_NULLABLE) {
		/* Still need to do the null-check (because an exception may have been thrown),
		 * unless the underlying location is being aliased somewhere else. */
		uint8_t saved_mo_flags;
		struct Dee_memval *alias_mval;
		Dee_memstate_foreach(alias_mval, state) {
			if (alias_mval->mv_vmorph == MEMVAL_VMORPH_NULLABLE &&
			    Dee_memobj_sameloc(Dee_memval_nullable_getobj(alias_mval),
			                       Dee_memval_nullable_getobj(mval)) &&
			    alias_mval != mval) {
				/* Found an alias! */
				Dee_memval_nullable_makedirect(mval);
#ifndef __OPTIMIZE_SIZE__ /* Shift a held reference as well (if possible) */
				if (Dee_memval_direct_isref(mval) &&
				    !Dee_memobj_isref(Dee_memval_nullable_getobj(alias_mval))) {
					Dee_memobj_clearref(Dee_memval_direct_getobj(mval));
					Dee_memobj_setref(Dee_memval_nullable_getobj(alias_mval));
				}
#endif /* !__OPTIMIZE_SIZE__ */
				goto do_maybe_decref;
			}
		}
		Dee_memstate_foreach_end;

		saved_mo_flags = Dee_memval_nullable_getobj(mval)->mo_flags;
		Dee_memobj_clearref(Dee_memval_nullable_getobj(mval));
		DO(Dee_function_generator_gjz_except(self, Dee_memval_nullable_getloc(mval)));
		DO(Dee_function_generator_state_unshare(self));
		state = self->fg_state;
		mval = Dee_memstate_vtop(state);
		Dee_memval_nullable_getobj(mval)->mo_flags = saved_mo_flags;
		Dee_memval_nullable_makedirect(mval);
		if (Dee_memval_direct_typeof(mval) == &DeeNone_Type) {
			Dee_memstate_decrinuse_for_memloc(state, Dee_memval_direct_getloc(mval));
			Dee_memval_init_const(mval, Dee_None, &DeeNone_Type);
		}
	}
do_maybe_decref:
	DO(Dee_function_generator_vgdecref_vstack(self, mval));
	--state->ms_stackc;
	Dee_memstate_decrinuse_for_memval(state, mval);
	Dee_memval_fini(mval);
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
	DO(Dee_function_generator_vlrot(self, n));
	DO(Dee_function_generator_vpop(self));
	return Dee_function_generator_vrrot(self, n - 1);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_local(struct Dee_function_generator *__restrict self,
                                  Dee_lid_t lid) {
	struct Dee_memstate *state;
	struct Dee_memval *dst, *src;
	DO(Dee_function_generator_state_unshare(self));
	state = self->fg_state;
	ASSERT(lid < state->ms_localc);
	src = Dee_memstate_vtop(state);

	/* Load the destination and see if there's already something stored in there.
	 * This shouldn't usually be the case, especially if you didn't turn off
	 * DEE_FUNCTION_ASSEMBLER_F_NOEARLYDEL, but there always be situations where
	 * we need to delete a previously assigned value! */
	dst = &state->ms_localv[lid];
	DO(Dee_function_generator_vgdecref_local(self, dst));
	ASSERT(state == self->fg_state);
	ASSERT(dst == &state->ms_localv[lid]);
	ASSERT(src == Dee_memstate_vtop(state));

	/* Because stack elements are always bound, the local is guarantied bound at this point. */
	Dee_memstate_decrinuse_for_memval(state, dst);
	Dee_memval_fini(dst);
	Dee_memval_initmove(dst, src);
#ifndef NDEBUG
	{
		struct Dee_memobj *obj;
		Dee_memval_foreach_obj(obj, dst) {
			ASSERTF(!(obj->mo_flags & MEMOBJ_F_MAYBEUNBOUND),
			        "Shouldn't have been set for stack item");
		}
		Dee_memval_foreach_obj_end;
	}
#endif /* !NDEBUG */
	--state->ms_stackc;
/*done:*/
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdel_local(struct Dee_function_generator *__restrict self,
                                  Dee_lid_t lid) {
	struct Dee_memstate *state;
	struct Dee_memval *mval;
	DO(Dee_function_generator_state_unshare(self));
	state = self->fg_state;
	ASSERT(lid < state->ms_localc);
	mval = &state->ms_localv[lid];
	DO(Dee_function_generator_vgdecref_local(self, mval));
	ASSERT(state == self->fg_state);
	ASSERT(mval == &state->ms_localv[lid]);
	Dee_memstate_decrinuse_for_memval(state, mval);
	Dee_memval_fini(mval);
	Dee_memval_init_local_unbound(mval);
	return 0;
err:
	return -1;
}


/* Check if top `n' elements are all `MEMADR_TYPE_CONST' */
INTDEF WUNUSED NONNULL((1)) bool DCALL
Dee_function_generator_vallconst(struct Dee_function_generator *__restrict self,
                                 Dee_vstackaddr_t n) {
	struct Dee_memstate *state = self->fg_state;
	struct Dee_memval *itemv;
	Dee_vstackaddr_t i;
	ASSERT(n <= state->ms_stackc);
	itemv = state->ms_stackv + state->ms_stackc - n;
	for (i = 0; i < n; ++i) {
		if (!Dee_memval_isconst(&itemv[i]))
			return false;
	}
	return true;
}

/* Check if top `n' elements are all `MEMADR_TYPE_CONST' and have the `MEMOBJ_F_NOREF' flag set. */
INTERN WUNUSED NONNULL((1)) bool DCALL
Dee_function_generator_vallconst_noref(struct Dee_function_generator *__restrict self,
                                       Dee_vstackaddr_t n) {
	struct Dee_memstate *state = self->fg_state;
	struct Dee_memval *itemv;
	Dee_vstackaddr_t i;
	bool has_refs = false;
	ASSERT(n <= state->ms_stackc);
	itemv = state->ms_stackv + state->ms_stackc - n;
	for (i = 0; i < n; ++i) {
		if (!Dee_memval_isconst(&itemv[i]))
			return false;
		if (Dee_memval_direct_isref(&itemv[i]))
			has_refs = true;
	}
	if (has_refs) {
		for (i = 0; i < n; ++i) {
			struct Dee_memval *alias;
			struct Dee_memval *mval = &itemv[i];
			ASSERT(Dee_memval_isconst(mval));
			if (!Dee_memval_direct_isref(mval))
				continue;

			/* See if there is some alias which we can off-load the reference on-to. */
			state->ms_stackc -= n;
			Dee_memstate_foreach(alias, state) {
				if (!Dee_memval_isconst(alias))
					continue;
				if (Dee_memval_const_getobj(alias) != Dee_memval_const_getobj(mval))
					continue;
				if (Dee_memval_direct_isref(alias))
					continue;
				Dee_memval_direct_setref(alias);
				Dee_memval_direct_clearref(mval);
				state->ms_stackc += n;
				goto check_next_vstack_item;
			}
			Dee_memstate_foreach_end;
			state->ms_stackc += n;
			return false; /* Cannot off-load this reference :( */
check_next_vstack_item:
			;
		}
	}
	return true;
}

PRIVATE NONNULL((1, 2)) void DCALL
memval_direct_setvaltype(struct Dee_memstate *__restrict state,
                  struct Dee_memval *mval, DeeTypeObject *type) {
	Dee_memval_direct_settypeof(mval, type);
	if (type == &DeeNone_Type && Dee_memval_isdirect(mval)) {
		/* Special case: none is a singleton, so if that's the type,
		 * we can assume the runtime value of its location. */
		Dee_memstate_decrinuse_for_memloc(state, Dee_memval_direct_getloc(mval));
		Dee_memval_const_setobj_keeptyp(mval, Dee_None);
	}
}

/* Remember that VTOP, as well as any other memory location
 * that might be aliasing it is an instance of "type" at runtime. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vsettyp(struct Dee_function_generator *__restrict self,
                               DeeTypeObject *type) {
	struct Dee_memstate *state;
	struct Dee_memval *vtop;
	DO(Dee_function_generator_state_unshare(self));
	state = self->fg_state;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	vtop = Dee_memstate_vtop(state);
	ASSERTF(Dee_memval_isdirect(vtop) || Dee_memval_isnullable(vtop),
	        "Can only assign types to direct or nullable values");
	if (Dee_memval_direct_typeof(vtop) != type) {
		struct Dee_memval *alias;
		Dee_memstate_foreach(alias, state) {
			if (!Dee_memval_isdirect(alias) && !Dee_memval_isnullable(alias))
				continue;
			if (!Dee_memval_direct_sameloc(alias, vtop))
				continue;
			memval_direct_setvaltype(state, alias, type);
		}
		Dee_memstate_foreach_end;
		/* TODO: Check for equivalences of "vtop" and assign the type to those as well! */
		ASSERT(Dee_memval_direct_typeof(vtop) == type);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vsettyp_noalias(struct Dee_function_generator *__restrict self,
                                       DeeTypeObject *type) {
	struct Dee_memstate *state;
	struct Dee_memval *vtop;
	DO(Dee_function_generator_state_unshare(self));
	state = self->fg_state;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	vtop = Dee_memstate_vtop(state);
	ASSERTF(Dee_memval_isdirect(vtop) || Dee_memval_isnullable(vtop),
	        "Can only assign types to direct or nullable values");
	if (Dee_memval_direct_typeof(vtop) != type) {
#ifndef NDEBUG
		struct Dee_memval *alias;
		Dee_memstate_foreach(alias, state) {
			ASSERT(alias == vtop || !Dee_memval_sameval(alias, vtop));
		}
		Dee_memstate_foreach_end;
#endif /* !NDEBUG */
		memval_direct_setvaltype(state, vtop, type);
	}
	return 0;
err:
	return -1;
}


/* VTOP = VTOP == <value> */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_veqconstaddr(struct Dee_function_generator *__restrict self,
                                    void const *value) {
	/* Kind-of weird, but works perfectly and can use vmorph mechanism:
	 * >> PUSH((POP() - <value>) == 0); */
	struct Dee_memval *mval;
	DO(Dee_function_generator_vdelta(self, -(ptrdiff_t)(uintptr_t)value));
	DO(Dee_function_generator_state_unshare(self));
	mval = Dee_function_generator_vtop(self);
	ASSERT(Dee_memval_isdirect(mval));
	mval->mv_vmorph = MEMVAL_VMORPH_BOOL_Z;
	return 0;
err:
	return -1;
}

/* PUSH(POP() == POP()); // Based on address */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_veqaddr(struct Dee_function_generator *__restrict self) {
	DeeObject *retval;
	Dee_host_register_t result_regno;
	struct Dee_memval *a, *b;
	if unlikely(self->fg_state->ms_stackc < 2)
		return err_illegal_stack_effect();
	DO(Dee_function_generator_state_unshare(self));
	b = Dee_function_generator_vtop(self);
	a = b - 1;

	/* If either of the 2 locations is a constant, then
	 * we can use `Dee_function_generator_veqconstaddr()' */
	if (Dee_memval_hasobj0(a) && Dee_memval_obj0_isconst(a) && a->mv_vmorph == b->mv_vmorph) {
		DO(Dee_function_generator_vswap(self));
		goto do_constant;
	} else if (Dee_memval_hasobj0(b) && Dee_memval_obj0_isconst(b) && a->mv_vmorph == b->mv_vmorph) {
		void const *const_value;
do_constant:
		ASSERT(Dee_memval_isconst(b));
		ASSERT(Dee_memval_hasobj0(a));
		ASSERT(a->mv_vmorph == b->mv_vmorph);
		a->mv_vmorph = MEMVAL_VMORPH_DIRECT;
		b->mv_vmorph = MEMVAL_VMORPH_DIRECT;
		const_value = Dee_memval_obj0_const_getaddr(b);
		if (Dee_memval_obj0_isconst(a)) {
			/* Special case: Compare addresses of 2 constants */
			retval = DeeBool_For(Dee_memval_obj0_const_getaddr(a) == const_value);
do_return_retval:
			DO(Dee_function_generator_vpopmany(self, 2));
			return Dee_function_generator_vpush_const(self, retval);
		}
		DO(Dee_function_generator_vpop(self));
		return Dee_function_generator_veqconstaddr(self, const_value);
	}
	DO(Dee_function_generator_vdirect(self, 2));
	if (Dee_memval_hasobj0(a) && Dee_memval_obj0_isconst(a)) {
		DO(Dee_function_generator_vswap(self));
		goto do_constant;
	} else if (Dee_memval_hasobj0(b) && Dee_memval_obj0_isconst(b)) {
		goto do_constant;
	}

	/* Another special case: do the 2 locations alias each other? */
	if (Dee_memval_sameval(a, b)) {
		retval = Dee_True;
		goto do_return_retval;
	}

	/* Fallback: allocate a result register, then generate code to fill that
	 * register with a 0/1 value indicative of the 2 memory location being equal. */
	DO(Dee_function_generator_vdirect(self, 2));
	b = Dee_function_generator_vtop(self);
	a = b - 1;
	ASSERT(Dee_memval_isdirect(a));
	ASSERT(Dee_memval_isdirect(b));
	result_regno = Dee_function_generator_gallocreg(self, NULL);
	if unlikely(result_regno >= HOST_REGISTER_COUNT)
		goto err;
	ASSERT(Dee_memval_isdirect(a));
	ASSERT(Dee_memval_isdirect(b));
	DO(Dee_function_generator_gmorph_locCloc2reg01(self,
	                                               Dee_memval_direct_getloc(a), 0, GMORPHBOOL_CC_EQ,
	                                               Dee_memval_direct_getloc(b), result_regno));
	DO(Dee_function_generator_vpush_hreg(self, result_regno, 0));
	a = Dee_function_generator_vtop(self);
	ASSERT(Dee_memval_isdirect(a));
	ASSERT(Dee_memval_direct_gettyp(a) == MEMADR_TYPE_HREG);
	ASSERT(Dee_memloc_hreg_getreg(Dee_memval_direct_getloc(a)) == result_regno);
	ASSERT(Dee_memloc_hreg_getvaloff(Dee_memval_direct_getloc(a)) == 0);
	a->mv_vmorph = MEMVAL_VMORPH_BOOL_NZ_01;
	DO(Dee_function_generator_vrrot(self, 3));
	DO(Dee_function_generator_vpop(self));
	return Dee_function_generator_vpop(self);
err:
	return -1;
}


/* >> if (THIRD == SECOND) // Based on address
 * >>     THIRD = FIRST;
 * >> POP();
 * >> POP(); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcoalesce(struct Dee_function_generator *__restrict self) {
	DREF struct Dee_memstate *common_state;
	struct Dee_host_symbol *text_Lnot_equal;
	struct Dee_memval *p_dst, *p_coalesce_from, *p_coalesce_to;
	struct Dee_memloc coalesce_from;
	if unlikely(self->fg_state->ms_stackc < 3)
		return err_illegal_stack_effect();
	DO(Dee_function_generator_state_unshare(self));
	p_coalesce_to   = Dee_function_generator_vtop(self);
	p_coalesce_from = p_coalesce_to - 1;
	p_dst           = p_coalesce_from - 1;
	if (Dee_memval_hasobj0(p_dst) && Dee_memval_obj0_isconst(p_dst)) {
		struct Dee_memval temp;
		Dee_memval_initmove(&temp, p_dst);
		Dee_memval_initmove(p_dst, p_coalesce_from);
		Dee_memval_initmove(p_coalesce_from, &temp);
	}
	if (Dee_memval_sameval(p_coalesce_from, p_coalesce_to) ||
	    Dee_memval_sameval(p_dst, p_coalesce_from)) {
		/* Special case: result is always "coalesce_to" */
		DO(Dee_function_generator_vrrot(self, 3));
		DO(Dee_function_generator_vpop(self));
		return Dee_function_generator_vpop(self);
	}

	/* Fallback: generate code to branch at runtime. */
	DO(Dee_function_generator_vdirect(self, 3)); /* from, to, dst */
	DO(Dee_function_generator_vlrot(self, 3));   /* from, to, dst */
	DO(Dee_function_generator_vreg(self, NULL)); /* from, to, reg:dst */
	DO(Dee_function_generator_vrrot(self, 3));   /* reg:dst, from, to */
	p_coalesce_to   = Dee_function_generator_vtop(self);
	p_coalesce_from = p_coalesce_to - 1;
	p_dst           = p_coalesce_from - 1;
	coalesce_from   = *Dee_memval_direct_getloc(p_coalesce_from);
	DO(Dee_function_generator_vswap(self)); /* reg:dst, to, from */
	DO(Dee_function_generator_vpop(self));  /* reg:dst, to */
	ASSERT(p_coalesce_from == Dee_function_generator_vtop(self));
	ASSERT(p_coalesce_to == Dee_function_generator_vtop(self) + 1);
	text_Lnot_equal = Dee_function_generator_newsym_named(self, ".text_Lnot_equal");
	if unlikely(!text_Lnot_equal)
		goto err;
	common_state = Dee_memstate_copy(self->fg_state);
	if unlikely(!common_state)
		goto err;
	EDO(err_common_state,
	    Dee_function_generator_gjcc(self, Dee_memval_direct_getloc(p_dst),
	                                &coalesce_from, false,
	                                text_Lnot_equal, NULL, text_Lnot_equal));
	EDO(err_common_state, Dee_function_generator_vswap(self));                /* to, reg:dst */
	EDO(err_common_state, Dee_function_generator_vpop(self));                 /* to */
	EDO(err_common_state, Dee_function_generator_vdup(self));                 /* to, to */
	EDO(err_common_state, Dee_function_generator_vmorph(self, common_state)); /* ... */
	Dee_memstate_decref(common_state);
	Dee_host_symbol_setsect(text_Lnot_equal, self->fg_sect);
	return Dee_function_generator_vpop(self);
err_common_state:
	Dee_memstate_decref(common_state);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcoalesce_c(struct Dee_function_generator *__restrict self,
                                   void const *from, void const *to) {
	DO(Dee_function_generator_vpush_addr(self, from));
	DO(Dee_function_generator_vpush_addr(self, to));
	return Dee_function_generator_vcoalesce(self);
err:
	return -1;
}

/* Clear the `MEMOBJ_F_ONEREF' flag for the top `n' v-stack elements,
 * as well as any other memory location that might be aliasing them. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vnotoneref(struct Dee_function_generator *__restrict self,
                                  Dee_vstackaddr_t n) {
	Dee_vstackaddr_t i;
	struct Dee_memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < n)
		return err_illegal_stack_effect();
	for (i = state->ms_stackc - n; i < state->ms_stackc; ++i) {
		struct Dee_memval *mval = &state->ms_stackv[i];
		struct Dee_memobj *mobj;
again_foreach_mobj:
		Dee_memval_foreach_obj(mobj, mval) {
			if (Dee_memobj_isoneref(mobj)) {
				if (Dee_memstate_isshared(state)) {
					state = Dee_memstate_copy(state);
					if unlikely(!state)
						goto err;
					Dee_memstate_decref_nokill(self->fg_state);
					self->fg_state = state;
					mval = &state->ms_stackv[i];
					goto again_foreach_mobj;
				}
				DO(Dee_function_generator_gnotoneref_impl(self, mobj));
			}
		}
		Dee_memval_foreach_obj_end;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vnotoneref_at(struct Dee_function_generator *__restrict self,
                                     Dee_vstackaddr_t off) {
	struct Dee_memval *mval;
	struct Dee_memobj *mobj;
	struct Dee_memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < off)
		return err_illegal_stack_effect();
again_foreach_mobj:
	mval = &state->ms_stackv[state->ms_stackc - off];
	Dee_memval_foreach_obj(mobj, mval) {
		if (Dee_memobj_isoneref(mobj)) {
			if (Dee_memstate_isshared(state)) {
				state = Dee_memstate_copy(state);
				if unlikely(!state)
					goto err;
				Dee_memstate_decref_nokill(self->fg_state);
				self->fg_state = state;
				goto again_foreach_mobj;
			}
			DO(Dee_function_generator_gnotoneref_impl(self, mobj));
		}
	}
	Dee_memval_foreach_obj_end;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vnotoneref_if_operator(struct Dee_function_generator *__restrict self,
                                              uint16_t operator_name, Dee_vstackaddr_t n) {
	Dee_vstackaddr_t i;
	struct Dee_memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < n)
		return err_illegal_stack_effect();
	for (i = state->ms_stackc - n; i < state->ms_stackc; ++i) {
		struct Dee_memval *mval = &state->ms_stackv[i];
		struct Dee_memobj *mobj;
again_foreach_mobj:
		Dee_memval_foreach_obj(mobj, mval) {
			DeeTypeObject *loctype = Dee_memval_typeof(mval);
			if (loctype != NULL && DeeType_IsOperatorNoRefEscape(loctype, operator_name))
				continue; /* Type is known to not let references to its instances escape. */
			if (Dee_memobj_isoneref(mobj)) {
				if (Dee_memstate_isshared(state)) {
					state = Dee_memstate_copy(state);
					if unlikely(!state)
						goto err;
					Dee_memstate_decref_nokill(self->fg_state);
					self->fg_state = state;
					mval = &state->ms_stackv[i];
					goto again_foreach_mobj;
				}
				DO(Dee_function_generator_gnotoneref_impl(self, mobj));
			}
		}
		Dee_memval_foreach_obj_end;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vnotoneref_if_operator_at(struct Dee_function_generator *__restrict self,
                                                 uint16_t operator_name, Dee_vstackaddr_t off) {
	struct Dee_memval *mval;
	struct Dee_memobj *mobj;
	struct Dee_memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < off)
		return err_illegal_stack_effect();
again_foreach_mobj:
	mval = &state->ms_stackv[state->ms_stackc - off];
	DeeTypeObject *loctype = Dee_memval_typeof(mval);
	if (loctype != NULL && DeeType_IsOperatorNoRefEscape(loctype, operator_name))
		return 0; /* Type is known to not let references to its instances escape. */
	Dee_memval_foreach_obj(mobj, mval) {
		if (Dee_memobj_isoneref(mobj)) {
			if (Dee_memstate_isshared(state)) {
				state = Dee_memstate_copy(state);
				if unlikely(!state)
					goto err;
				Dee_memstate_decref_nokill(self->fg_state);
				self->fg_state = state;
				goto again_foreach_mobj;
			}
			DO(Dee_function_generator_gnotoneref_impl(self, mobj));
		}
	}
	Dee_memval_foreach_obj_end;
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_ulocal(struct Dee_function_generator *__restrict self,
                                    Dee_instruction_t const *instr, Dee_ulid_t ulid) {
	if unlikely(ulid >= self->fg_assembler->fa_localc)
		return err_illegal_ulid(ulid);
	return Dee_function_generator_vpush_local(self, instr, (Dee_lid_t)ulid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vbound_ulocal(struct Dee_function_generator *__restrict self,
                                     Dee_instruction_t const *instr, Dee_ulid_t ulid) {
	if unlikely(ulid >= self->fg_assembler->fa_localc)
		return err_illegal_ulid(ulid);
	return Dee_function_generator_vbound_local(self, instr, (Dee_lid_t)ulid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_ulocal(struct Dee_function_generator *__restrict self,
                                   Dee_ulid_t ulid) {
	if unlikely(ulid >= self->fg_assembler->fa_localc)
		return err_illegal_ulid(ulid);
	return Dee_function_generator_vpop_local(self, (Dee_lid_t)ulid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdel_ulocal(struct Dee_function_generator *__restrict self,
                                   Dee_ulid_t ulid) {
	if unlikely(ulid >= self->fg_assembler->fa_localc)
		return err_illegal_ulid(ulid);
	return Dee_function_generator_vdel_local(self, (Dee_lid_t)ulid);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpushinit_optarg(struct Dee_function_generator *__restrict self,
                                        Dee_instruction_t const *instr, Dee_lid_t xlid) {
	DREF struct Dee_memstate *common_state;
	DeeCodeObject *code = self->fg_assembler->fa_code;
	uint16_t opt_aid = (uint16_t)(xlid - MEMSTATE_XLOCAL_DEFARG_MIN);
	Dee_aid_t aid = opt_aid + code->co_argc_min;
	Dee_lid_t lid = self->fg_assembler->fa_localc + xlid;
	DeeObject *default_value;
	ASSERT(xlid >= MEMSTATE_XLOCAL_DEFARG_MIN);
	ASSERT(lid < self->fg_state->ms_localc);
	default_value = self->fg_assembler->fa_code->co_defaultv[opt_aid];
	if (default_value) {
		struct Dee_host_symbol *Luse_default;

		/* Load the default value into a register and into the local */
		DO(Dee_function_generator_vpush_const(self, default_value)); /* default_value */
		DO(Dee_function_generator_vreg(self, NULL));                 /* reg:default_value */

		/* Check if the caller has provided enough arguments. */
		DO(Dee_function_generator_vpush_argc(self)); /* reg:default_value, argc */
		DO(Dee_function_generator_vdirect1(self));   /* reg:default_value, argc */
		Luse_default = Dee_function_generator_newsym_named(self, ".Luse_default");
		if unlikely(!Luse_default)
			goto err;
		{
			struct Dee_memloc l_argc, l_aid;
			ASSERT(Dee_function_generator_vtop_isdirect(self));
			ASSERT(!Dee_function_generator_vtop_direct_isref(self));
			l_argc = *Dee_function_generator_vtopdloc(self);
			--self->fg_state->ms_stackc;
			Dee_memstate_decrinuse_for_memloc(self->fg_state, &l_argc);
			Dee_memloc_init_const(&l_aid, (void *)(uintptr_t)aid);
			DO(Dee_function_generator_gjcc(self, &l_aid, &l_argc, false, NULL,
			                               Luse_default, Luse_default));
		}
		common_state = self->fg_state;
		Dee_memstate_incref(common_state);
		if unlikely(Dee_function_generator_vpop(self))
			goto err_common_state; /* - */
		if unlikely(Dee_function_generator_vpush_argv(self))
			goto err_common_state; /* argv */
		if unlikely(Dee_function_generator_vind(self, (ptrdiff_t)aid * sizeof(DeeObject *)))
			goto err_common_state; /* argv[aid] */
		if unlikely(Dee_function_generator_vmorph(self, common_state))
			goto err_common_state; /* reg:value */
		Dee_memstate_decref(common_state);
		Dee_host_symbol_setsect(Luse_default, self->fg_sect);
	} else {
		if (self->fg_state->ms_uargc_min <= aid) {
			struct Dee_host_section *text;
			struct Dee_host_section *cold;
			struct Dee_host_symbol *Ltarget;
			struct Dee_memloc l_argc, l_aid;
			Ltarget = Dee_function_generator_newsym_named(self, ".Ltarget");
			if unlikely(!Ltarget)
				goto err;
			text = self->fg_sect;
			cold = text;
			if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE))
				cold = &self->fg_block->bb_hcold;
			DO(Dee_function_generator_vpush_argc(self));
			DO(Dee_function_generator_vdirect1(self));
			ASSERT(Dee_function_generator_vtop_isdirect(self));
			ASSERT(!Dee_function_generator_vtop_direct_isref(self));
			l_argc = *Dee_function_generator_vtopdloc(self);
			--self->fg_state->ms_stackc;
			Dee_memstate_decrinuse_for_memloc(self->fg_state, &l_argc);
			Dee_memloc_init_const(&l_aid, (void *)(uintptr_t)aid);
			DO(Dee_function_generator_gjcc(self, &l_aid, &l_argc, false,
			                               text != cold ? NULL : Ltarget,
			                               text != cold ? Ltarget : NULL,
			                               text != cold ? Ltarget : NULL));
			common_state = self->fg_state;
			Dee_memstate_incref(common_state);
			HA_printf(".section .cold\n");
			self->fg_sect = cold;
			if (text != cold)
				Dee_host_symbol_setsect(Ltarget, cold);
			if unlikely(Dee_function_generator_gthrow_arg_unbound(self, instr, aid))
				goto err_common_state;
			if (text == cold)
				Dee_host_symbol_setsect(Ltarget, self->fg_sect);
			HA_printf(".section .text\n");
			self->fg_sect = text;
			/* After the check above, we're allowed to remember that the argument
			 * count is great enough to always include the accessed argument. */
			self->fg_state->ms_uargc_min = aid + 1;

			/* Restore state from before exception handling was entered. */
			Dee_memstate_decref(self->fg_state);
			self->fg_state = common_state;
		}
		DO(Dee_function_generator_vpush_argv(self));                                 /* argv */
		DO(Dee_function_generator_vind(self, (ptrdiff_t)aid * sizeof(DeeObject *))); /* argv[aid] */
	}
	return 0;
err_common_state:
	Dee_memstate_decref(common_state);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpushinit_varargs(struct Dee_function_generator *__restrict self) {
	uint16_t co_argc_min = self->fg_assembler->fa_code->co_argc_min;
	uint16_t co_argc_max = self->fg_assembler->fa_code->co_argc_max;
	DO(Dee_function_generator_vpush_argc(self));                      /* argc */
	DO(Dee_function_generator_vdelta(self, -(ptrdiff_t)co_argc_max)); /* argc-co_argc_max */
	if (co_argc_min < co_argc_max && self->fg_state->ms_uargc_min < co_argc_max) {
		/* TODO: FIXME: If `argc-co_argc_max' rolls over or is 0, then we have to push an empty tuple
		 *              This is because less than `co_argc_max' may be provided by the caller if the
		 *              function also takes default/optional arguments:
		 * >> function foo(a = 0, b = 1, args...) {
		 * >>     return args;
		 * >> }
		 * >> print repr foo();               // ()
		 * >> print repr foo(10);             // ()
		 * >> print repr foo(10, 20);         // ()
		 * >> print repr foo(10, 20, 30);     // (30,)
		 * >> print repr foo(10, 20, 30, 40); // (30,40)
		 */
	}
	DO(Dee_function_generator_vpush_argv(self));                                           /* argc-co_argc_max, argv */
	DO(Dee_function_generator_vdelta(self, (ptrdiff_t)co_argc_max * sizeof(DeeObject *))); /* argc-co_argc_max, argv+co_argc_max */
	DO(Dee_function_generator_vcallapi(self, &DeeTuple_NewVector, VCALL_CC_OBJECT, 2));    /* varargs */
	Dee_function_generator_voneref_noalias(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpushinit_varkwds(struct Dee_function_generator *__restrict self) {
	/* TODO */
	(void)self;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpushinit_stdout(struct Dee_function_generator *__restrict self) {
	int result = Dee_function_generator_vpush_imm32(self, DEE_STDOUT);
	if likely(result == 0)
		result = Dee_function_generator_vcallapi(self, &DeeFile_GetStd, VCALL_CC_OBJECT, 1);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpushinit_xlocal(struct Dee_function_generator *__restrict self,
                                        Dee_instruction_t const *instr, Dee_lid_t xlid) {
	switch (xlid) {

	case MEMSTATE_XLOCAL_VARARGS:
		return Dee_function_generator_vpushinit_varargs(self);
	case MEMSTATE_XLOCAL_VARKWDS:
		return Dee_function_generator_vpushinit_varkwds(self);
	case MEMSTATE_XLOCAL_STDOUT:
		return Dee_function_generator_vpushinit_stdout(self);

	default:
		if (xlid >= MEMSTATE_XLOCAL_DEFARG_MIN)
			return Dee_function_generator_vpushinit_optarg(self, instr, xlid);
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "No way to init xlid=%" PRFuSIZ,
		                       xlid);
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vinit_xlocal(struct Dee_function_generator *__restrict self,
                                    Dee_instruction_t const *instr, Dee_lid_t xlid) {
	/* Push the initializer for the x-local onto the v-stack. */
	DO(Dee_function_generator_vpushinit_xlocal(self, instr, xlid)); /* init */
	DO(Dee_function_generator_vdirect1(self));                      /* init */
	DO(Dee_function_generator_state_unshare(self));                 /* init */
	return Dee_function_generator_vpop_local(self, self->fg_assembler->fa_localc + xlid);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_xlocal(struct Dee_function_generator *__restrict self,
                                    Dee_instruction_t const *instr, Dee_lid_t xlid) {
	DREF struct Dee_memstate *common_state;
	struct Dee_memval *xlocal_mval;

	/* Optimizations (and special handling) for certain xlocal slots. */
	switch (xlid) {

	case MEMSTATE_XLOCAL_VARARGS:
		/* Check for special case: when the function *only* takes varargs, and
		 * the calling convention provides us with a caller-given argument tuple,
		 * then simply push that tuple instead of allocating a new one! */
		if unlikely(!(self->fg_assembler->fa_code->co_flags & CODE_FVARARGS))
			return err_unsupported_opcode(self->fg_assembler->fa_code, instr);
		if (self->fg_assembler->fa_code->co_argc_max == 0 &&
		    (self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE))
			return Dee_function_generator_vpush_args(self, instr);
		break;

	case MEMSTATE_XLOCAL_VARKWDS:
		if unlikely(!(self->fg_assembler->fa_code->co_flags & CODE_FVARKWDS))
			return err_unsupported_opcode(self->fg_assembler->fa_code, instr);
		break;

	default: break;
	}

	/* Check if the slot needs to be initialized (and if so: initialize it) */
	DO(Dee_function_generator_state_unshare(self));
	xlocal_mval = &self->fg_state->ms_localv[self->fg_assembler->fa_localc + xlid];
	if (!Dee_memval_isdirect(xlocal_mval)) {
		/* Non-direct values don't need initialization! */
	} else if (xlid >= MEMSTATE_XLOCAL_DEFARG_MIN
	           /* Special case: for default arguments, UNDEFINED means uninitialized,
	            * and NEVERBOUND is a valid, initialized state for argument-not-given. */
	           ? Dee_memval_direct_isundefined(xlocal_mval)
	           : Dee_memval_direct_local_neverbound(xlocal_mval)) {
		DO(Dee_function_generator_vinit_xlocal(self, instr, xlid));
	} else if (!Dee_memval_direct_local_alwaysbound(xlocal_mval)) {
		struct Dee_host_symbol *Lskipinit;
		ASSERT(Dee_memval_isdirect(xlocal_mval));
		if (Dee_memval_direct_gettyp(xlocal_mval) != MEMADR_TYPE_HSTACKIND &&
		    Dee_memval_direct_gettyp(xlocal_mval) != MEMADR_TYPE_HREG) {
			DO(tracked_memloc_forcereg(self, Dee_memval_direct_getloc(xlocal_mval), NULL));
			ASSERT(Dee_memval_direct_gettyp(xlocal_mval) == MEMADR_TYPE_HREG);
		}
		Lskipinit = Dee_function_generator_newsym_named(self, ".Lskipinit");
		if unlikely(!Lskipinit)
			goto err;
		DO(Dee_function_generator_gjnz(self, Dee_memval_direct_getloc(xlocal_mval), Lskipinit));
		DO(Dee_function_generator_state_unshare(self));
		common_state = Dee_memstate_copy(self->fg_state);
		if unlikely(!common_state)
			goto err;
		xlocal_mval = &self->fg_state->ms_localv[self->fg_assembler->fa_localc + xlid];
		Dee_memval_fini_direct(xlocal_mval);
		Dee_memval_init_local_unbound(xlocal_mval);
		if unlikely(Dee_function_generator_vinit_xlocal(self, instr, xlid))
			goto err_common_state;
		if unlikely(Dee_function_generator_vmorph(self, common_state))
			goto err_common_state;
		Dee_memstate_decref(common_state);
		Dee_host_symbol_setsect(Lskipinit, self->fg_sect);
	}
	ASSERTF(!Dee_memval_isdirect(&self->fg_state->ms_localv[self->fg_assembler->fa_localc + xlid]) ||
	        Dee_memval_direct_local_alwaysbound(&self->fg_state->ms_localv[self->fg_assembler->fa_localc + xlid]),
	        "The local should be unconditionally bound at this point!");
	return _Dee_function_generator_vpush_xlocal(self, instr, xlid);
err_common_state:
	Dee_memstate_decref(common_state);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_this_function(struct Dee_function_generator *__restrict self) {
	if (self->fg_assembler->fa_cc & HOSTFUNC_CC_F_FUNC)
		return _Dee_function_generator_vpush_xlocal(self, NULL, MEMSTATE_XLOCAL_A_FUNC);
	return Dee_function_generator_vpush_const(self, self->fg_assembler->fa_function);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_argc(struct Dee_function_generator *__restrict self) {
	if (!(self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE))
		return _Dee_function_generator_vpush_xlocal(self, NULL, MEMSTATE_XLOCAL_A_ARGC);
	DO(_Dee_function_generator_vpush_xlocal(self, NULL, MEMSTATE_XLOCAL_A_ARGS));
	return Dee_function_generator_vind(self, offsetof(DeeTupleObject, t_size));
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_argv(struct Dee_function_generator *__restrict self) {
	if (!(self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE))
		return _Dee_function_generator_vpush_xlocal(self, NULL, MEMSTATE_XLOCAL_A_ARGV);
	DO(_Dee_function_generator_vpush_xlocal(self, NULL, MEMSTATE_XLOCAL_A_ARGS));
	return Dee_function_generator_vdelta(self, offsetof(DeeTupleObject, t_elem));
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_usage(struct Dee_function_generator *__restrict self,
                                   Dee_host_regusage_t usage) {
	Dee_host_register_t regno;
	DO(Dee_function_generator_state_unshare(self));
	regno = Dee_function_generator_gusagereg(self, usage, NULL);
	if unlikely(regno >= HOST_REGISTER_COUNT)
		goto err;
	return Dee_function_generator_vpush_hreg(self, regno, 0);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_except(struct Dee_function_generator *__restrict self) {
	DO(Dee_function_generator_vpush_usage(self, DEE_HOST_REGUSAGE_THREAD));     /* DeeThread_Self() */
	DO(Dee_function_generator_vind(self, offsetof(DeeThreadObject, t_except))); /* DeeThread_Self()->t_except */
	/* Check if there is an active exception if not already checked. */
	if (!(self->fg_state->ms_flags & MEMSTATE_F_GOTEXCEPT)) {
		int temp;
		DREF struct Dee_memstate *saved_state;
		struct Dee_host_section *text = self->fg_sect;
		struct Dee_host_section *cold = &self->fg_block->bb_hcold;
		if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
			cold = text;
		ASSERT(Dee_function_generator_vtop_isdirect(self));
		if (text == cold) {
			struct Dee_host_symbol *text_Ldone;
			text_Ldone = Dee_function_generator_newsym_named(self, ".Ldone");
			if unlikely(!text_Ldone)
				goto err;
			DO(Dee_function_generator_gjnz(self, Dee_function_generator_vtopdloc(self), text_Ldone));
			saved_state = self->fg_state;
			Dee_memstate_incref(saved_state);
			temp = Dee_function_generator_state_dounshare(self);
			if likely(temp == 0)
				temp = Dee_function_generator_vcallapi(self, &libhostasm_rt_err_no_active_exception, VCALL_CC_EXCEPT, 0);
			Dee_memstate_decref(self->fg_state);
			self->fg_state = saved_state;
			Dee_host_symbol_setsect(text_Ldone, text);
		} else {
			struct Dee_host_symbol *Lerr_no_active_exception;
			Lerr_no_active_exception = Dee_function_generator_newsym_named(self, ".Lerr_no_active_exception");
			if unlikely(!Lerr_no_active_exception)
				goto err;
			DO(Dee_function_generator_gjz(self, Dee_function_generator_vtopdloc(self), Lerr_no_active_exception));
			saved_state = self->fg_state;
			Dee_memstate_incref(saved_state);
			HA_printf(".section .cold\n");
			self->fg_sect = cold;
			Dee_host_symbol_setsect(Lerr_no_active_exception, cold);
			temp = Dee_function_generator_state_dounshare(self);
			if likely(temp == 0)
				temp = Dee_function_generator_vcallapi(self, &libhostasm_rt_err_no_active_exception, VCALL_CC_EXCEPT, 0);
			HA_printf(".section .text\n");
			self->fg_sect = text;
			Dee_memstate_decref(self->fg_state);
			self->fg_state = saved_state;
		}
		if unlikely(temp)
			goto err;
		/* Remember that there is an exception */
		self->fg_state->ms_flags |= MEMSTATE_F_GOTEXCEPT;
	}
	/* DeeThread_Self()->t_except->ef_error */
	return Dee_function_generator_vind(self, offsetof(struct Dee_except_frame, ef_error));
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gunbound_member(struct Dee_function_generator *__restrict self,
                                       DeeTypeObject *__restrict class_type, uint16_t addr,
                                       void const *api_function) {
	DO(Dee_function_generator_vpush_const(self, class_type));
	DO(Dee_function_generator_vpush_imm16(self, addr));
	return Dee_function_generator_vcallapi(self, api_function, VCALL_CC_EXCEPT, 2);
err:
	return -1;
}

#define Dee_function_generator_gunbound_class_member(self, class_type, addr) \
	Dee_function_generator_gunbound_member(self, class_type, addr, (void const *)&libhostasm_rt_err_unbound_class_member)
#define Dee_function_generator_gunbound_instance_member(self, class_type, addr) \
	Dee_function_generator_gunbound_member(self, class_type, addr, (void const *)&libhostasm_rt_err_unbound_instance_member)

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeClassDescriptor_IsClassAttributeReadOnly(DeeClassDescriptorObject const *__restrict self,
                                            uint16_t addr) {
	size_t i;
	for (i = 0; i <= self->cd_iattr_mask; ++i) {
		struct class_attribute const *attr;
		attr = &self->cd_iattr_list[i];
		if ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) &&
		    (addr >= attr->ca_addr || addr < attr->ca_addr + Dee_CLASS_GETSET_COUNT))
			return true; /* get/del/set callbacks should only be assigned once */
		if (addr == attr->ca_addr)
			return (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FREADONLY) != 0;
	}
	/* Address isn't described. Assume some write-once out-of-band shenanigans */
	return true;
}




/* N/A -> value */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpush_cmember_unsafe_at_runtime(struct Dee_function_generator *__restrict self,
                                                       DeeTypeObject *class_type,
                                                       uint16_t addr, unsigned int flags) {
	int temp;
	struct class_desc *desc = DeeClass_DESC(class_type);
	struct Dee_host_section *text;
	struct Dee_host_section *cold;
	DREF struct Dee_memstate *saved_state;

	/* When optimizing for size, generate a (smaller) call to
	 * `DeeClass_GetMember()', instead of inlining the function. */
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE) {
		DO(Dee_function_generator_vpush_const(self, class_type));
		DO(Dee_function_generator_vpush_imm16(self, addr));
		return Dee_function_generator_vcallapi(self, &DeeClass_GetMember, VCALL_CC_OBJECT, 2);
	}

	/* Perform the inline equivalent of `DeeClass_GetMember()':
	 * >> Dee_class_desc_lock_read(desc);
	 * >> result = desc->cd_members[addr];
	 * >> if unlikely(!result) {
	 * >>     Dee_class_desc_lock_endread(desc);
	 * >>     libhostasm_rt_err_unbound_class_member(<class_type>, addr);
	 * >>     HANDLE_ERROR();
	 * >> }
	 * >> Dee_Incref(result);
	 * >> Dee_class_desc_lock_endread(desc); */
	if (flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)
		DO(Dee_function_generator_grwlock_read_const(self, &desc->cd_lock));
	DO(Dee_function_generator_vpush_addr(self, &desc->cd_members[addr])); /* p_value */
	DO(Dee_function_generator_vind(self, 0));                             /* *p_value */
	DO(Dee_function_generator_vreg(self, NULL));                          /* reg:value */
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	text = self->fg_sect;
	cold = &self->fg_block->bb_hcold;
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
		cold = text;
	if (cold == text) {
		struct Dee_host_symbol *text_Lbound;
		text_Lbound = Dee_function_generator_newsym_named(self, ".Lbound");
		if unlikely(!text_Lbound)
			goto err;
		DO(Dee_function_generator_gjnz(self, Dee_function_generator_vtopdloc(self), text_Lbound));
		saved_state = self->fg_state;
		Dee_memstate_incref(saved_state);
		temp = Dee_function_generator_state_dounshare(self);
		if ((flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF) && likely(temp == 0))
			temp = Dee_function_generator_grwlock_endread_const(self, &desc->cd_lock);
		if likely(temp == 0)
			temp = Dee_function_generator_gunbound_class_member(self, class_type, addr);
		Dee_host_symbol_setsect(text_Lbound, text);
	} else {
		struct Dee_host_symbol *cold_Lunbound_member;
		cold_Lunbound_member = Dee_function_generator_newsym_named(self, ".Lunbound_member");
		if unlikely(!cold_Lunbound_member)
			goto err;
		DO(Dee_function_generator_gjz(self, Dee_function_generator_vtopdloc(self), cold_Lunbound_member));
		saved_state = self->fg_state;
		Dee_memstate_incref(saved_state);
		HA_printf(".section .cold\n");
		self->fg_sect = cold;
		Dee_host_symbol_setsect(cold_Lunbound_member, cold);
		temp = Dee_function_generator_state_dounshare(self);
		if ((flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF) && likely(temp == 0))
			temp = Dee_function_generator_grwlock_endread_const(self, &desc->cd_lock);
		if likely(temp == 0)
			temp = Dee_function_generator_gunbound_class_member(self, class_type, addr);
		HA_printf(".section .text\n");
		self->fg_sect = text;
	}
	Dee_memstate_decref(self->fg_state);
	self->fg_state = saved_state;
	if unlikely(temp)
		goto err;
	ASSERT(!Dee_function_generator_vtop_direct_isref(self));
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	if (!(flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF))
		return 0;
	DO(Dee_function_generator_gincref_loc(self, Dee_function_generator_vtopdloc(self), 1));
	Dee_function_generator_vtop_direct_setref(self);
	return Dee_function_generator_grwlock_endread_const(self, &desc->cd_lock);
err:
	return -1;
}

/* type -> value */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_cmember(struct Dee_function_generator *__restrict self,
                                     uint16_t addr, unsigned int flags) {
	struct Dee_memval *type_mval;
	DO(Dee_function_generator_vdirect1(self));
	type_mval = Dee_function_generator_vtop(self);
	if (Dee_memval_direct_isconst(type_mval)) {
		struct class_desc *desc;
		DeeTypeObject *class_type = (DeeTypeObject *)Dee_memval_const_getobj(type_mval);
		if unlikely(DeeObject_AssertType(class_type, &DeeType_Type))
			goto err;
		if unlikely(!DeeType_IsClass(class_type))
			return libhostasm_rt_err_requires_class(class_type);
		desc = DeeClass_DESC(class_type);
		if unlikely(addr >= desc->cd_desc->cd_cmemb_size)
			return libhostasm_rt_err_invalid_class_addr(class_type, addr);
		DO(Dee_function_generator_vpop(self)); /* Get rid of the `class_type' v-stack item. */
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOROINLINE)) {
			DREF DeeObject *member_value;
			Dee_class_desc_lock_write(desc);
			member_value = desc->cd_members[addr];
			Dee_XIncref(member_value);
			Dee_class_desc_lock_endwrite(desc);
			if (member_value) {
				/* Check if the class attribute linked to the member is READONLY,
				 * or doesn't exist. In both cases, assume that the member can only
				 * be written one, which has already happened, meaning we're allowed
				 * to inline its value. */
				if (DeeClassDescriptor_IsClassAttributeReadOnly(desc->cd_desc, addr)) {
					member_value = Dee_function_generator_inlineref(self, member_value);
					if unlikely(!member_value)
						goto err;
					return Dee_function_generator_vpush_const(self, member_value);
				}
				Dee_Decref(member_value);
			}
		}

		/* Push the member at runtime, but allowed to use (normally) unsafe
		 * semantics since we were already able to verify all constraints. */
		return Dee_function_generator_vpush_cmember_unsafe_at_runtime(self, class_type, addr, flags);
	}

	/* Fallback: access the class member at runtime */
	DO(Dee_function_generator_vpush_imm16(self, addr));
	return Dee_function_generator_vcallapi(self,
	                                       ((self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_SAFE) ||
	                                        (flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE))
	                                       ? (void const *)&DeeClass_GetMemberSafe
	                                       : (void const *)&DeeClass_GetMember,
	                                       VCALL_CC_OBJECT, 2);
err:
	return -1;
}


/* type -> bound */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vbound_cmember(struct Dee_function_generator *__restrict self,
                                      uint16_t addr, unsigned int flags) {
	struct Dee_memval *type_mval, *vtop;
	DO(Dee_function_generator_vdirect1(self));
	type_mval = Dee_function_generator_vtop(self);
	if (Dee_memval_direct_isconst(type_mval)) {
		DeeObject **p_valloc;
		struct class_desc *desc;
		DeeTypeObject *class_type = (DeeTypeObject *)Dee_memval_const_getobj(type_mval);
		if unlikely(DeeObject_AssertType(class_type, &DeeType_Type))
			goto err;
		if unlikely(!DeeType_IsClass(class_type))
			return libhostasm_rt_err_requires_class(class_type);
		desc = DeeClass_DESC(class_type);
		if unlikely(addr >= desc->cd_desc->cd_cmemb_size)
			return libhostasm_rt_err_invalid_class_addr(class_type, addr);
		DO(Dee_function_generator_vpop(self)); /* N/A */
		p_valloc = &desc->cd_members[addr];
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOROINLINE) &&
		    atomic_read(p_valloc) != NULL &&
		    DeeClassDescriptor_IsClassAttributeReadOnly(desc->cd_desc, addr)) {
			return Dee_function_generator_vpush_const(self, Dee_True);
		}
		DO(Dee_function_generator_vpush_addr(self, p_valloc)); /* p_valloc */
		DO(Dee_function_generator_vind(self, 0));              /* *p_valloc */
		DO(Dee_function_generator_vreg(self, NULL));           /* reg:*p_valloc */
		vtop = Dee_function_generator_vtop(self);
		ASSERT(Dee_memval_isdirect(vtop));
		vtop->mv_vmorph = MEMVAL_VMORPH_BOOL_NZ;
		return 0;
	}

	/* Fallback: access the class member at runtime */
	if ((self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_SAFE) ||
	    (flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE)) {
		DO(Dee_function_generator_vpush_imm16(self, addr));
		DO(Dee_function_generator_vcallapi(self, &DeeClass_BoundMemberSafe, VCALL_CC_NEGINT, 2));
		vtop = Dee_function_generator_vtop(self);
		ASSERT(Dee_memval_isdirect(vtop));
		vtop->mv_vmorph = MEMVAL_VMORPH_TESTNZ(vtop->mv_vmorph);
	} else {
		DO(Dee_function_generator_vind(self, offsetof(DeeTypeObject, tp_class))); /* type->tp_class */
		DO(Dee_function_generator_vind(self,                                      /* type->tp_class->cd_members[addr] */
		                               offsetof(struct Dee_class_desc, cd_members[0]) +
		                               (addr * sizeof(DREF DeeObject *))));
		DO(Dee_function_generator_vreg(self, NULL)); /* reg:type->tp_class->cd_members[addr] */
		vtop = Dee_function_generator_vtop(self);
		ASSERT(Dee_memval_isdirect(vtop));
		vtop->mv_vmorph = MEMVAL_VMORPH_BOOL_NZ;
	}
	return 0;
err:
	return -1;
}

/* type, value -> N/A */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_cmember(struct Dee_function_generator *__restrict self,
                                    uint16_t addr, unsigned int flags) {
	struct Dee_memobj *type_mobj;
	DO(Dee_function_generator_vdirect(self, 2));       /* type, value */
	DO(Dee_function_generator_vnotoneref_at(self, 1)); /* type, value */
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_SAFE)
		flags |= DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE; /* Force safe semantics. */

	/* Check if we have known meta-data about the "type" operand. */
	type_mobj = Dee_memval_getobj0(Dee_function_generator_vtop(self) - 1);
	if (Dee_memobj_hasxinfo(type_mobj)) {
		struct Dee_memobj_xinfo *type_xinfo = Dee_memobj_getxinfo(type_mobj);
		struct Dee_memobj_xinfo_cdesc *type_cdesc = type_xinfo->mox_cdesc;
		if (type_cdesc && addr < type_cdesc->moxc_desc->cd_cmemb_size) {
			/* If we can predict certain things about the state of the class
			 * descriptor, we can generate some highly optimized inline code:
			 * - Omit locking if we know there's only a single reference
			 * - Omit xdecref'ing previously assigned values for never-before assigned slots
			 * - Let the class inherit a reference to "value"
			 *
			 * TODO: When a reference to the class type is used to construct a member
			 *       function (i.e. used as a reference in a member function), then that
			 *       should *not* count as being a reason not to do the MEMOBJ_F_ONEREF
			 *       optimization here. (however: the MEMOBJ_F_ONEREF flag itself must
			 *       still be cleared in that case).
			 *       However: if one of those functions gets called or is passed somewhere
			 *       where that function might get called (or have its references inspected),
			 *       then we must once again *NOT* do this optimization here!
			 */
			if (type_mobj->mo_flags & MEMOBJ_F_ONEREF) {
				if (!Dee_memobj_xinfo_cdesc_wasinit(type_cdesc, addr)) {
					Dee_memobj_xinfo_cdesc_setinit(type_cdesc, addr);
					/* The cmember slot is known to be NULL, so we can just directly write to it:
					 * >> struct class_desc *cd = <type>->tp_class;
					 * >> cd->cd_members[<addr>] = <value>; // Inherit */
					DO(Dee_function_generator_vref2(self, 2));                                /* type, value */
					DO(Dee_function_generator_vdup_n(self, 2));                               /* type, value, type */
					DO(Dee_function_generator_vind(self, offsetof(DeeTypeObject, tp_class))); /* type, value, type->tp_class */
					DO(Dee_function_generator_vswap(self));                                   /* type, type->tp_class, value */
					DO(Dee_function_generator_vpopind(self,                                   /* type, type->tp_class */
					                                  offsetof(struct Dee_class_desc, cd_members[0]) +
					                                  (addr * sizeof(DREF DeeObject *))));
					DO(Dee_function_generator_vpop(self)); /* type */
					return Dee_function_generator_vpop(self); /* N/A */
				}
			}
			if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
				/* Object is being shared -> cannot trust what we (think we) know
				 * about cmember slot initialization. However, can still generate
				 * inline code to do:
				 * >> struct class_desc *cd = <type>->tp_class;
				 * >> #if !(type_mobj->mo_flags & MEMOBJ_F_ONEREF)
				 * >> Dee_atomic_rwlock_write(cd);
				 * >> #endif
				 * >> DREF DeeObject *old_value = cd->cd_members[<addr>];
				 * >> cd->cd_members[<addr>] = <value>; // Inherit
				 * >> #if !(type_mobj->mo_flags & MEMOBJ_F_ONEREF)
				 * >> Dee_atomic_rwlock_endwrite(cd);
				 * >> #endif
				 * >> Dee_XDecref(old_value); */
				/* TODO */
			}

			/* Can force unsafe semantics since we know that none of the assertions can fail. */
			flags &= ~DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE;
		}
	}

	/* Fallback: do the assignment at runtime. */
	DO(Dee_function_generator_vpush_imm16(self, addr)); /* type, value, addr */
	DO(Dee_function_generator_vswap(self));             /* type, addr, value */
	return (flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE)
	       ? Dee_function_generator_vcallapi(self, &DeeClass_SetMemberSafe, VCALL_CC_INT, 3)
	       : Dee_function_generator_vcallapi(self, &DeeClass_SetMember, VCALL_CC_VOID, 3);
err:
	return -1;
}


/* this -> value */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpush_imember_unsafe_at_runtime(struct Dee_function_generator *__restrict self,
                                                       DeeTypeObject *type, uint16_t addr, unsigned int flags) {
	int temp;
	struct Dee_host_section *text;
	struct Dee_host_section *cold;
	DREF struct Dee_memstate *saved_state;
	struct class_desc *desc = DeeClass_DESC(type);
	ptrdiff_t lock_offset;
	ptrdiff_t slot_offset;
	ASSERT(self->fg_state->ms_stackc >= 1);

	/* When optimizing for size, generate a (smaller) call to
	 * `DeeInstance_GetMember()', instead of inlining the function. */
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE) {
		DO(Dee_function_generator_vpush_const(self, type)); /* this, type */
		DO(Dee_function_generator_vswap(self));             /* type, this */
		DO(Dee_function_generator_vpush_imm16(self, addr)); /* type, this, addr */
		return Dee_function_generator_vcallapi(self, &DeeInstance_GetMember, VCALL_CC_OBJECT, 3);
	}

	lock_offset = desc->cd_offset + offsetof(struct instance_desc, id_lock);
	slot_offset = desc->cd_offset + offsetof(struct instance_desc, id_vtab) +
	              addr * sizeof(DREF DeeObject *);
	/* XXX: (and this is a problem with the normal executor): assert that "this" is an instance of `type' */

	/* TODO: In case of reading members, if one of the next instructions also does a read,
	 *       keep the lock acquired. The same should also go when it comes to accessing
	 *       global/extern variables. */
	DO(Dee_function_generator_vdelta(self, lock_offset)); /* &this->[...].id_lock */
	if (flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)
		DO(Dee_function_generator_grwlock_read(self, Dee_function_generator_vtopdloc(self)));
	if (flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF)
		DO(Dee_function_generator_vdup(self));                          /* [&this->[...].id_lock], &this->[...].id_lock */
	DO(Dee_function_generator_vdelta(self, slot_offset - lock_offset)); /* [&this->[...].id_lock], &this->[...].VALUE */
	DO(Dee_function_generator_vind(self, 0));                           /* [&this->[...].id_lock], value */
	DO(Dee_function_generator_vreg(self, NULL));                        /* [&this->[...].id_lock], reg:value */

	/* Assert that the member is bound */
	text = self->fg_sect;
	cold = &self->fg_block->bb_hcold;
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
		cold = text;
	if (cold == text) {
		struct Dee_host_symbol *text_Lbound;
		text_Lbound = Dee_function_generator_newsym_named(self, ".Lbound");
		if unlikely(!text_Lbound)
			goto err;
		DO(Dee_function_generator_gjnz(self, Dee_function_generator_vtopdloc(self), text_Lbound));
		saved_state = self->fg_state;
		Dee_memstate_incref(saved_state);
		temp = Dee_function_generator_state_dounshare(self);
		if likely(temp == 0)
			temp = Dee_function_generator_grwlock_endread(self, Dee_function_generator_vtopdloc(self) - 1);
		if likely(temp == 0)
			temp = Dee_function_generator_gunbound_instance_member(self, type, addr);
		Dee_host_symbol_setsect(text_Lbound, text);
	} else {
		struct Dee_host_symbol *cold_Lunbound_member;
		cold_Lunbound_member = Dee_function_generator_newsym_named(self, ".Lunbound_member");
		if unlikely(!cold_Lunbound_member)
			goto err;
		DO(Dee_function_generator_gjz(self, Dee_function_generator_vtopdloc(self), cold_Lunbound_member));
		saved_state = self->fg_state;
		Dee_memstate_incref(saved_state);
		HA_printf(".section .cold\n");
		self->fg_sect = cold;
		Dee_host_symbol_setsect(cold_Lunbound_member, cold);
		temp = Dee_function_generator_state_dounshare(self);
		if likely(temp == 0)
			temp = Dee_function_generator_grwlock_endread(self, Dee_function_generator_vtopdloc(self) - 1);
		if likely(temp == 0)
			temp = Dee_function_generator_gunbound_instance_member(self, type, addr);
		HA_printf(".section .text\n");
		self->fg_sect = text;
	}
	Dee_memstate_decref(self->fg_state);
	self->fg_state = saved_state;
	if unlikely(temp)
		goto err;
	if (flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF) {
		DO(Dee_function_generator_vref2(self, 2));                                               /* &this->[...].id_lock, ref:value */
		DO(Dee_function_generator_vswap(self));                                                  /* ref:value, &this->[...].id_lock */
		DO(Dee_function_generator_grwlock_endread(self, Dee_function_generator_vtopdloc(self))); /* ref:value, &this->[...].id_lock */
		DO(Dee_function_generator_vpop(self));                                                   /* ref:value */
	}
	return 0;
err:
	return -1;
}

/* this, type -> value */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_imember(struct Dee_function_generator *__restrict self,
                                     uint16_t addr, unsigned int flags) {
	struct Dee_memval *type_mval;
	bool safe = ((self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_SAFE) ||
	             (flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE));
	DO(Dee_function_generator_vdirect(self, 2));

	/* We only allow inline-resolving of this-member accesses when safe is disabled.
	 * This is because in code with user-assembly *any* type can be used together
	 * with the "this" argument, meaning that we *always* have to check that "this"
	 * actually implements the type *at runtime*.
	 *
	 * This differs when accessing class members, since for those no "this" argument
	 * is needed and the entire access can be performed immediately, no matter if
	 * safe semantics are required or not. */
	type_mval = Dee_function_generator_vtop(self);
	if (Dee_memval_direct_isconst(type_mval) && !safe) {
		struct class_desc *desc;
		DeeTypeObject *class_type = (DeeTypeObject *)Dee_memval_const_getobj(type_mval);
		if (DeeObject_AssertType(class_type, &DeeType_Type))
			goto err;
		if (!DeeType_IsClass(class_type))
			return libhostasm_rt_err_requires_class(class_type);
		desc = DeeClass_DESC(class_type);
		if (addr >= desc->cd_desc->cd_imemb_size)
			return libhostasm_rt_err_invalid_instance_addr(class_type, addr);
		DO(Dee_function_generator_vpop(self)); /* this */

		/* Push the member at runtime, but allowed to use (normally) unsafe
		 * semantics since we were already able to verify all constraints. */
		return Dee_function_generator_vpush_imember_unsafe_at_runtime(self, class_type, addr, flags);
	}

	/* Fallback: access the class member at runtime */
	DO(Dee_function_generator_vswap(self));             /* type, self */
	DO(Dee_function_generator_vpush_imm16(self, addr)); /* type, self, addr */
	return Dee_function_generator_vcallapi(self,
	                                       safe ? (void const *)&DeeInstance_GetMemberSafe
	                                            : (void const *)&DeeInstance_GetMember,
	                                       VCALL_CC_OBJECT, 3);
err:
	return -1;
}

/* this, type -> bound */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vbound_imember(struct Dee_function_generator *__restrict self,
                                      uint16_t addr, unsigned int flags) {
	bool safe = ((self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_SAFE) ||
	             (flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE));
	DO(Dee_function_generator_vdirect(self, 2));
	/* TODO */
	(void)self;
	(void)addr;
	(void)safe;
	return DeeError_NOTIMPLEMENTED();
err:
	return -1;
}

/* this, value -> N/A */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vdel_or_pop_imember_unsafe_at_runtime(struct Dee_function_generator *__restrict self,
                                                             DeeTypeObject *type, uint16_t addr) {
	struct class_desc *desc = DeeClass_DESC(type);
	ptrdiff_t lock_offset;
	ptrdiff_t slot_offset;
	ASSERT(self->fg_state->ms_stackc >= 2);

	/* When optimizing for size, generate a (smaller) call to
	 * `DeeInstance_GetMember()', instead of inlining the function. */
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE) {
		struct Dee_memval *value_mval;
		DO(Dee_function_generator_vpush_const(self, type)); /* this, value, type */
		DO(Dee_function_generator_vrrot(self, 3));          /* type, this, value */
		DO(Dee_function_generator_vpush_imm16(self, addr)); /* type, this, value, addr */
		DO(Dee_function_generator_vswap(self));             /* type, this, addr, value */
		value_mval = Dee_function_generator_vtop(self);
		if (Dee_memval_isnull(value_mval)) {
			DO(Dee_function_generator_vpop(self)); /* type, this, addr */
			return Dee_function_generator_vcallapi(self, &DeeInstance_DelMember, VCALL_CC_INT, 3);
		}
		DO(Dee_function_generator_vnotoneref_at(self, 1)); /* type, this, addr, value */
		return Dee_function_generator_vcallapi(self, &DeeInstance_SetMember, VCALL_CC_INT, 4);
	} else {
		struct Dee_memval *value_mval;
		value_mval = Dee_function_generator_vtop(self);
		if (!Dee_memval_isnull(value_mval))
			DO(Dee_function_generator_vref2(self, 2)); /* this, ref:value */
	}

	lock_offset = desc->cd_offset + offsetof(struct instance_desc, id_lock);
	slot_offset = desc->cd_offset + offsetof(struct instance_desc, id_vtab) +
	              addr * sizeof(DREF DeeObject *);
	/* XXX: (and this is a problem with the normal executor): assert that "this" is an instance of `type' */

	DO(Dee_function_generator_vswap(self));                                                   /* ref:value, this */
	DO(Dee_function_generator_vdelta(self, lock_offset));                                     /* ref:value, &this->[...].id_lock */
	DO(Dee_function_generator_grwlock_write(self, Dee_function_generator_vtopdloc(self)));    /* ... */
	DO(Dee_function_generator_vdup(self));                                                    /* ref:value, &this->[...].id_lock, &this->[...].id_lock */
	DO(Dee_function_generator_vdelta(self, slot_offset - lock_offset));                       /* ref:value, &this->[...].id_lock, &this->[...].VALUE */
	DO(Dee_function_generator_vlrot(self, 3));                                                /* &this->[...].id_lock, &this->[...].VALUE, ref:value */
	DO(Dee_function_generator_vswapind(self, 0));                                             /* &this->[...].id_lock, old_value */
	DO(Dee_function_generator_vswap(self));                                                   /* old_value, &this->[...].id_lock */
	DO(Dee_function_generator_grwlock_endwrite(self, Dee_function_generator_vtopdloc(self))); /* old_value, &this->[...].id_lock */
	DO(Dee_function_generator_vpop(self));                                                    /* old_value */
	ASSERT(!Dee_function_generator_vtop_direct_isref(self));
	DO(Dee_function_generator_gxdecref_loc(self, Dee_function_generator_vtopdloc(self), 1));
	ASSERT(!Dee_function_generator_vtop_direct_isref(self));
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

/* this, type, value -> N/A */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_imember(struct Dee_function_generator *__restrict self,
                                    uint16_t addr, unsigned int flags) {
	bool safe = ((self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_SAFE) ||
	             (flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE));
	struct Dee_memval *mval;
	DO(Dee_function_generator_vdirect(self, 3));

	/* We only allow inline-resolving of this-member accesses when safe is disabled.
	 * This is because in code with user-assembly *any* type can be used together
	 * with the "this" argument, meaning that we *always* have to check that "this"
	 * actually implements the type *at runtime*.
	 *
	 * This differs when accessing class members, since for those no "this" argument
	 * is needed and the entire access can be performed immediately, no matter if
	 * safe semantics are required or not. */
	mval = Dee_function_generator_vtop(self) - 1;
	if (Dee_memval_direct_isconst(mval) && !safe) {
		struct class_desc *desc;
		DeeTypeObject *class_type = (DeeTypeObject *)Dee_memval_const_getobj(mval);
		if (DeeObject_AssertType(class_type, &DeeType_Type))
			goto err;
		if (!DeeType_IsClass(class_type))
			return libhostasm_rt_err_requires_class(class_type);
		desc = DeeClass_DESC(class_type);
		if (addr >= desc->cd_desc->cd_imemb_size)
			return libhostasm_rt_err_invalid_instance_addr(class_type, addr);
		DO(Dee_function_generator_vswap(self)); /* this, value, type */
		DO(Dee_function_generator_vpop(self));  /* this, value */

		/* Push the member at runtime, but allowed to use (normally) unsafe
		 * semantics since we were already able to verify all constraints. */
		return Dee_function_generator_vdel_or_pop_imember_unsafe_at_runtime(self, class_type, addr);
	}

	/* Fallback: access the class member at runtime */
	DO(Dee_function_generator_vswap(self));             /* this, value, type */
	DO(Dee_function_generator_vrrot(self, 3));          /* type, this, value */
	DO(Dee_function_generator_vpush_imm16(self, addr)); /* type, this, value, addr */
	DO(Dee_function_generator_vswap(self));             /* type, this, addr, value */
	mval = Dee_function_generator_vtop(self);
	if (Dee_memval_isnull(mval)) {
		DO(Dee_function_generator_vpop(self)); /* type, this, addr */
		return Dee_function_generator_vcallapi(self,
		                                       safe ? (void const *)&DeeInstance_DelMemberSafe
		                                            : (void const *)&DeeInstance_DelMember,
		                                       VCALL_CC_INT, 3);
	}
	DO(Dee_function_generator_vnotoneref_at(self, 1)); /* type, this, addr, value */
	return Dee_function_generator_vcallapi(self,
	                                       safe ? (void const *)&DeeInstance_SetMemberSafe
	                                            : (void const *)&DeeInstance_SetMember,
	                                       VCALL_CC_INT, 4);
err:
	return -1;
}

/* this, type -> N/A */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdel_imember(struct Dee_function_generator *__restrict self,
                                    uint16_t addr, unsigned int flags) {
	int result = Dee_function_generator_vpush_addr(self, NULL);
	if likely(result == 0)
		result = Dee_function_generator_vpop_imember(self, addr, flags);
	return result;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
reclaim_unused_stack_space(struct Dee_function_generator *__restrict self) {
	Dee_cfa_t old_cfa_offset = self->fg_state->ms_host_cfa_offset;
	if (Dee_memstate_hstack_free(self->fg_state)) {
		Dee_cfa_t new_cfa_offset = self->fg_state->ms_host_cfa_offset;
		ptrdiff_t freed = old_cfa_offset - new_cfa_offset;
		ASSERT(freed > 0);
		return Dee_function_generator_ghstack_adjust(self, -freed);
	}
	return 0;
}

/* obj, type -> N/A */
PRIVATE WUNUSED NONNULL((1)) int DCALL
impl_vassert_type_exact(struct Dee_function_generator *__restrict self) {
	/* TODO: Must check for `DeeType_IsAbstract(type)' */
	if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
		/* TODO: emit code equivalent to:
		 * >> if (Dee_TYPE(self) != type) {
		 * >>     DeeObject_TypeAssertFailed(obj, type);
		 * >>     HANDLE_EXCEPT();
		 * >> }
		 */
	}
	return Dee_function_generator_vcallapi(self, &DeeObject_AssertTypeExact, VCALL_CC_INT, 2);
}

/* obj, type -> N/A */
PRIVATE WUNUSED NONNULL((1)) int DCALL
impl_vassert_type(struct Dee_function_generator *__restrict self) {
	if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)) {
		/* TODO: emit code equivalent to:
		 * >> if (!DeeType_InheritsFrom(Dee_TYPE(obj), type)) {
		 * >>     DeeObject_TypeAssertFailed(obj, type);
		 * >>     HANDLE_EXCEPT();
		 * >> }
		 */
	}
	return Dee_function_generator_vcallapi(self, &DeeObject_AssertType, VCALL_CC_INT, 2);
}


/* Generate code equivalent to `DeeObject_AssertTypeExact(VTOP, type)', but don't pop `VTOP' from the v-stack. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vassert_type_exact_c(struct Dee_function_generator *__restrict self,
                                            DeeTypeObject *__restrict type) {
	DeeTypeObject *vtop_type;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	vtop_type = Dee_memval_typeof(Dee_function_generator_vtop(self));
	if (vtop_type != NULL) {
		struct Dee_memval *vtop;
		if (vtop_type == type)
			return 0;
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR)) {
			return DeeError_Throwf(&DeeError_TypeError,
			                       "Expected exact instance of `%r', but got a `%r' object",
			                       type, vtop_type);
		}
		/* Other pieces of code are allowed to assume that in case of a compile-time
		 * constant, the produced type assertions will ensure that constants always
		 * have the proper types. As such, we mustn't leave the value be a constant,
		 * so-as not to cause those assertions to fail. */
		vtop = Dee_function_generator_vtop(self);
		if (Dee_memval_isconst(vtop)) {
			ASSERT(Dee_memval_isdirect(vtop));
			DO(tracked_memloc_forcereg(self, Dee_memval_direct_getloc(vtop), NULL));
			ASSERT(!Dee_memloc_isconst(Dee_memval_direct_getloc(vtop)));
			Dee_memobj_settypeof(Dee_memval_direct_getobj(vtop), NULL);
		}
	}
	DO(Dee_function_generator_vdirect1(self));          /* value */
	DO(Dee_function_generator_vdup(self));              /* value, value */
	DO(Dee_function_generator_vpush_const(self, type)); /* value, value, type */
	DO(impl_vassert_type_exact(self));                  /* value */
	return Dee_function_generator_vsettyp(self, type);
err:
	return -1;
}

/* obj, type -> N/A */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vassert_type_exact(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *typeval;
	DO(Dee_function_generator_vdirect(self, 2)); /* obj, type */
	typeval = Dee_function_generator_vtop(self);
	if (Dee_memval_direct_isconst(typeval)) {
		DeeTypeObject *type = (DeeTypeObject *)Dee_memval_const_getobj(typeval);
		DO(Dee_function_generator_vpop(self));                       /* obj */
		DO(Dee_function_generator_vassert_type_exact_c(self, type)); /* obj */
		return Dee_function_generator_vpop(self);                    /* N/A */
	}
	return impl_vassert_type_exact(self);
err:
	return -1;
}


/* Generate code equivalent to `DeeObject_AssertType(VTOP, type)', but don't pop `VTOP' from the v-stack. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vassert_type_c(struct Dee_function_generator *__restrict self,
                                      DeeTypeObject *__restrict type) {
	DeeTypeObject *vtop_type;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	if (DeeType_IsAbstract(type))
		return 0; /* Special case: abstract types don't need to be checked! */
	vtop_type = Dee_memval_typeof(Dee_function_generator_vtop(self));
	if (vtop_type != NULL) {
		struct Dee_memval *vtop;
		if (DeeType_InheritsFrom(vtop_type, type))
			return 0;
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOEARLYERR)) {
			return DeeError_Throwf(&DeeError_TypeError,
			                       "Expected instance of `%r', but got a `%r' object",
			                       type, vtop_type);
		}

		/* Other pieces of code are allowed to assume that in case of a compile-time
		 * constant, the produced type assertions will ensure that constants always
		 * have the proper types. As such, we mustn't leave the value be a constant,
		 * so-as not to cause those assertions to fail. */
		vtop = Dee_function_generator_vtop(self);
		if (Dee_memval_isconst(vtop)) {
			ASSERT(Dee_memval_isdirect(vtop));
			DO(tracked_memloc_forcereg(self, Dee_memval_direct_getloc(vtop), NULL));
			ASSERT(!Dee_memloc_isconst(Dee_memval_direct_getloc(vtop)));
			Dee_memobj_settypeof(Dee_memval_direct_getobj(vtop), NULL);
		}
	}
	DO(Dee_function_generator_vdirect1(self)); /* value */
	DO(Dee_function_generator_vdup(self));              /* value, value */
	DO(Dee_function_generator_vpush_const(self, type)); /* value, value, type */
	return impl_vassert_type(self);
err:
	return -1;
}

/* obj, type -> N/A */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vassert_type(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *typeval;
	DO(Dee_function_generator_vdirect(self, 2)); /* obj, type */
	typeval = Dee_function_generator_vtop(self);
	if (Dee_memval_isconst(typeval)) {
		DeeTypeObject *type = (DeeTypeObject *)Dee_memval_const_getobj(typeval);
		DO(Dee_function_generator_vpop(self));                 /* obj */
		DO(Dee_function_generator_vassert_type_c(self, type)); /* obj */
		return Dee_function_generator_vpop(self);              /* N/A */
	}
	return impl_vassert_type(self);
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
	struct Dee_host_symbol *Ljmp;
	int bool_status;
	struct Dee_basic_block *target = desc->jd_to;
	struct Dee_memval *cond_mval;
#ifdef DEE_HOST_RELOCVALUE_SECT
	struct Dee_host_symbol _Ljmp;
#endif /* DEE_HOST_RELOCVALUE_SECT */
	DO(Dee_function_generator_state_unshare(self));

	/* TODO: If this jump might be the result of infinite loops,
	 *       must emit a call to `DeeThread_CheckInterrupt()' */

	bool_status = Dee_function_generator_vopbool(self, VOPBOOL_F_NOFALLBACK | VOPBOOL_F_FORCE_MORPH);
	if unlikely(bool_status < 0)
		goto err; /* Force vtop into a bool constant, or a MEMVAL_VMORPH_ISBOOL-style morph */
	cond_mval = Dee_function_generator_vtop(self);

	/* Special case for when the top-element is a constant. */
	if (Dee_memval_isconst(cond_mval)) {
		ASSERT(DeeBool_Check(Dee_memval_const_getobj(cond_mval)));
		if (Dee_memval_const_getobj(cond_mval) != Dee_False) {
			/* Unconditional jump -> the block ends here and falls into the next one */
			self->fg_block->bb_next       = target;
			self->fg_block->bb_deemon_end = instr; /* The jump doesn't exist anymore now! */
		}
		DO(Dee_function_generator_vpop(self));
		goto assign_desc_stat;
	}

	/* If the jump target location already has its starting memory state generated,
	 * and that state requires a small CFA offset than we currently have, then try
	 * to reclaim unused host stack memory.
	 *
	 * This is necessary so we don't end up in a situation where a block keeps on
	 * marking itself for re-compilation with ever-growing CFA offsets. */
	if (target->bb_mem_start != NULL &&
	    target->bb_mem_start->ms_host_cfa_offset < self->fg_state->ms_host_cfa_offset)
		DO(reclaim_unused_stack_space(self));

	/* Initialize the symbol for jumping to `desc'. */
#ifdef DEE_HOST_RELOCVALUE_SECT
	Dee_host_symbol_initcommon_named(&_Ljmp, ".Ljmp");
	Ljmp = &_Ljmp;
#else /* DEE_HOST_RELOCVALUE_SECT */
	Ljmp = Dee_function_generator_newsym_named(self, ".Ljmp");
	if unlikely(!Ljmp)
		goto err;
#endif /* !DEE_HOST_RELOCVALUE_SECT */
	Dee_host_symbol_setjump(Ljmp, desc);

	/* Check for special case: `Dee_function_generator_vopbool()' needed to do its fallback operation.
	 * Handle this case by doing the call to `DeeObject_Bool()' ourselves, so we can combine the bool
	 * branch with the except branch, thus saving on a couple of otherwise redundant instructions. */
	if (bool_status > 0) {
		bool hasbool;
		DeeTypeObject *loctype;
		struct Dee_memloc zero;
		struct Dee_host_symbol *Lexcept, *Lnot_except;
#ifdef DEE_HOST_RELOCVALUE_SECT
		struct Dee_host_symbol _Lexcept;
		Dee_host_symbol_initcommon_named(&_Lexcept, ".Lexcept");
		Lexcept = &_Lexcept;
#else /* DEE_HOST_RELOCVALUE_SECT */
		Lexcept = Dee_function_generator_newsym_named(self, ".Lexcept");
		if unlikely(!Lexcept)
			goto err;
#endif /* !DEE_HOST_RELOCVALUE_SECT */
		ASSERT(cond_mval == Dee_function_generator_vtop(self));
		DO(Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_BOOL, 1));
		ASSERT(cond_mval == Dee_function_generator_vtop(self));

		loctype = Dee_memval_typeof(cond_mval);
		hasbool = loctype && DeeType_InheritOperator(loctype, OPERATOR_BOOL);
		ASSERT(!hasbool || (loctype->tp_cast.tp_bool != NULL));
		DO(Dee_function_generator_vcallapi(self,
		                                   hasbool ? (void const *)loctype->tp_cast.tp_bool
		                                           : (void const *)&DeeObject_Bool,
		                                   VCALL_CC_RAWINT, 1));

		/* Silently remove the bool-morph location from the v-stack. */
		cond_mval = Dee_function_generator_vtop(self);
		ASSERT(Dee_memval_isdirect(cond_mval));
		ASSERT(self->fg_state->ms_stackc >= 1);
		Dee_memstate_decrinuse_for_memloc(self->fg_state, Dee_memval_direct_getloc(cond_mval));
		--self->fg_state->ms_stackc;

		/* Figure out how to do exception handling. */
		Lnot_except = NULL;
		if (self->fg_exceptinject != NULL) {
			/* Prepare stuff so we can inject custom exception handling code. */
			if (self->fg_sect == &self->fg_block->bb_hcold) {
				Lnot_except = Lexcept;
				Lexcept     = NULL;
				Dee_host_symbol_setname(Lnot_except, ".Lnot_except");
			}
		}

		/* Generate code to branch depending on the value of `loc' */
		Dee_memloc_init_const(&zero, (void *)0);
		DO(Dee_function_generator_gjcc(self, Dee_memval_direct_getloc(cond_mval), &zero, true,
		                               Lexcept,                             /* loc < 0 */
		                               jump_if_true ? Lnot_except : Ljmp,   /* loc == 0 */
		                               jump_if_true ? Ljmp : Lnot_except)); /* loc > 0 */

		if (self->fg_exceptinject != NULL) {
			/* Must inject custom exception handling code! */
			ASSERT((Lnot_except == NULL) || (Lnot_except == NULL));
			ASSERT((Lnot_except != NULL) || (Lnot_except != NULL));
			if (Lnot_except) {
				ASSERT(!Lexcept);
				ASSERT(!Dee_host_symbol_isdefined(Lnot_except));
				DO(Dee_function_generator_gjmp_except(self));
				Dee_host_symbol_setsect(Lnot_except, self->fg_sect);
			} else {
				struct Dee_host_section *text;
				ASSERT(Lexcept);
				ASSERT(!Dee_host_symbol_isdefined(Lexcept));
				ASSERT(self->fg_sect != &self->fg_block->bb_hcold);
				text = self->fg_sect;
				HA_printf(".section .cold\n");
				self->fg_sect = &self->fg_block->bb_hcold;
				Dee_host_symbol_setsect(Lexcept, self->fg_sect);
				DO(Dee_function_generator_gjmp_except(self));
				HA_printf(".section .text\n");
				self->fg_sect = text;
			}
		} else {
			struct Dee_except_exitinfo *except_exit;
			except_exit = Dee_function_generator_except_exit(self);
			if unlikely(!except_exit)
				goto err;
			Dee_host_symbol_setsect_ex(Lexcept, &except_exit->exi_text, 0);
		}
	} else {
		struct Dee_memloc cmp_lhs, cmp_rhs;
		struct Dee_host_symbol *Llo, *Leq, *Lgr;

		/* In this case, `Dee_function_generator_vopbool()' already created a morph. */
		ASSERT(MEMVAL_VMORPH_ISBOOL(cond_mval->mv_vmorph));
		ASSERT(!Dee_memobj_isref(&cond_mval->mv_obj.mvo_0));

		/* Silently remove the bool-morph location from the v-stack. */
		ASSERT(self->fg_state->ms_stackc >= 1);
		/*Dee_memval_fini(cond_mval);*/ /* Not needed for `MEMVAL_VMORPH_ISBOOL()' */
		Dee_memstate_decrinuse_for_memobj(self->fg_state, &cond_mval->mv_obj.mvo_0);
		--self->fg_state->ms_stackc;

		/* Compute compare operands and target labels. */
		Llo = NULL;
		Leq = NULL;
		Lgr = NULL;
		cmp_lhs = *Dee_memobj_getloc(&cond_mval->mv_obj.mvo_0);
		Dee_memloc_init_const(&cmp_rhs, NULL);
		switch (cond_mval->mv_vmorph) {
		case MEMVAL_VMORPH_BOOL_Z:
		case MEMVAL_VMORPH_BOOL_Z_01:
			/* Jump-if-zero */
			Leq = Ljmp;
			break;
		case MEMVAL_VMORPH_BOOL_NZ:
		case MEMVAL_VMORPH_BOOL_NZ_01:
			Llo = Ljmp;
			Lgr = Ljmp;
			break;
		case MEMVAL_VMORPH_BOOL_LZ:
			Llo = Ljmp;
			/* (X-1) < 0   <=>   X <= 0 */
			if (Dee_memloc_getoff(&cmp_lhs) == -1) {
				Dee_memloc_setoff(&cmp_lhs, 0);
				Leq = Ljmp;
			}
			break;
		case MEMVAL_VMORPH_BOOL_GZ:
			Lgr = Ljmp;
			/* (X+1) > 0   <=>   X >= 0 */
			if (Dee_memloc_getoff(&cmp_lhs) == 1) {
				Dee_memloc_setoff(&cmp_lhs, 0);
				Leq = Ljmp;
			}
			break;
		default: __builtin_unreachable();
		}

		if (!jump_if_true) {
			/* Invert the logical meaning of the jump. */
			struct Dee_host_symbol *temp;
			if (Llo == Lgr) {
				temp = Leq;
				Leq = Llo;
				Llo = temp;
				Lgr = temp;
			} else if (Leq == Llo) {
				temp = Lgr;
				Lgr = Leq;
				Llo = temp;
				Leq = temp;
			} else {
				ASSERT(Leq == Lgr);
				temp = Llo;
				Llo = Leq;
				Leq = temp;
				Lgr = temp;
			}
		}

		/* Emit the jump */
		DO(Dee_function_generator_gjcc(self, &cmp_lhs, &cmp_rhs, true, Llo, Leq, Lgr));
	}

	/* Remember the memory-state as it is when the jump is made. */
assign_desc_stat:
	ASSERTF(!desc->jd_stat, "Who assigned this? Doing that is *my* job!");
	desc->jd_stat = self->fg_state;
	Dee_memstate_incref(self->fg_state);

	bool_status = Dee_basic_block_constrainwith(target, desc->jd_stat,
	                                            Dee_function_assembler_addrof(self->fg_assembler,
	                                                                          target->bb_deemon_start));
	if (bool_status > 0) {
		bool_status = 0;
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

	return bool_status;
err:
	return -1;
}


__pragma_GCC_diagnostic_push_ignored(Wmaybe_uninitialized)

/* Implement a ASM_FOREACH-style jump to `desc'
 * @param: instr:               Pointer to start of deemon jmp-instruction (for bb-truncation, and error message)
 * @param: always_pop_iterator: When true, the iterator is also popped during the jump to `desc'
 *                              This is needed to implement ASM_FOREACH when used with a prefix.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vforeach(struct Dee_function_generator *__restrict self,
                                struct Dee_jump_descriptor *desc,
                                bool always_pop_iterator) {
	int temp;
	struct Dee_memobj decref_on_iter_done;
	DREF struct Dee_memstate *desc_state;
	struct Dee_host_symbol *sym;
	struct Dee_basic_block *target = desc->jd_to;
	DO(Dee_function_generator_state_unshare(self));
	DO(Dee_function_generator_vdirect1(self));                                     /* iter */
	DO(Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_ITERNEXT, 1)); /* iter */
	if (!always_pop_iterator)
		DO(Dee_function_generator_vdup(self)); /* iter, iter */
	DO(Dee_function_generator_vcallapi(self, &DeeObject_IterNext, VCALL_CC_RAWINTPTR, 1)); /* [if(!always_pop_iterator) iter], UNCHECKED(elem) */

	/* TODO: If this jump might be the result of infinite loops,
	 *       must emit a call to `DeeThread_CheckInterrupt()' */

	/* If the jump target location already has its starting memory state generated,
	 * and that state requires a small CFA offset than we currently have, then try
	 * to reclaim unused host stack memory.
	 *
	 * This is necessary so we don't end up in a situation where a block keeps on
	 * marking itself for re-compilation with ever-growing CFA offsets. */
	if (target->bb_mem_start != NULL &&
	    target->bb_mem_start->ms_host_cfa_offset < self->fg_state->ms_host_cfa_offset)
		DO(reclaim_unused_stack_space(self));

	/* >> if (elem == NULL) HANDLE_EXCEPT(); */
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	DO(Dee_function_generator_gjz_except(self, Dee_function_generator_vtopdloc(self)));

	/* >> if (elem == ITER_DONE) goto <desc>; */
	sym = Dee_function_generator_newsym(self);
	if unlikely(!sym)
		goto err;
	Dee_host_symbol_setjump(sym, desc);
	{
		struct Dee_memloc iter_done;
		Dee_memloc_init_const(&iter_done, ITER_DONE);
		DO(Dee_function_generator_gjcc(self, Dee_function_generator_vtopdloc(self),
		                               &iter_done, false, NULL, sym, NULL));
	}
	DO(Dee_function_generator_state_unshare(self));

	/* Remember the memory-state as it is when the jump is made. */
	ASSERTF(!desc->jd_stat, "Who assigned this? Doing that is *my* job!");
	desc_state = Dee_memstate_copy(self->fg_state);
	if unlikely(!desc_state)
		goto err;
	ASSERT(desc_state->ms_stackc >= 1);
	--desc_state->ms_stackc; /* Get rid of `UNCHECKED(result)' */
	ASSERT(Dee_memval_isdirect(&desc_state->ms_stackv[desc_state->ms_stackc]));
	Dee_memstate_decrinuse_for_memloc(desc_state, Dee_memval_direct_getloc(&desc_state->ms_stackv[desc_state->ms_stackc]));
	Dee_memobj_init_local_unbound(&decref_on_iter_done);
	if (!always_pop_iterator) {
		/* Pop another vstack item (the iterator) and store it in `MEMSTATE_XLOCAL_POPITER'.
		 * When the time comes to generate morph-code, the iterator will then be decref'd. */
		ASSERT(desc_state->ms_stackc >= 1);
		--desc_state->ms_stackc;
		ASSERT(Dee_memval_isdirect(&desc_state->ms_stackv[desc_state->ms_stackc]));
		decref_on_iter_done = *Dee_memval_direct_getobj(&desc_state->ms_stackv[desc_state->ms_stackc]);
		Dee_memstate_decrinuse_for_memobj(desc_state, &decref_on_iter_done);
	}
	desc->jd_stat = desc_state; /* Inherit reference */

	/* Adjust out own current state to make the top-item (i.e. the "elem") to become a reference */
	ASSERT(self->fg_state != desc_state);
	ASSERT(!Dee_memstate_isshared(self->fg_state));
	ASSERT(!Dee_function_generator_vtop_direct_isref(self));
	Dee_function_generator_vtop_direct_setref(self);

	/* Constrain the jump-target block with the mem-state from the descriptor. */
	temp = Dee_basic_block_constrainwith(target, desc_state,
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

	/* Do a little bit of black magic to drop the reference from the iterator
	 * as part of the morph done during the jump. */
	if (Dee_memobj_isref(&decref_on_iter_done)) {
		struct Dee_memval *popiter_mval;
		popiter_mval = &desc_state->ms_localv[self->fg_assembler->fa_localc + MEMSTATE_XLOCAL_POPITER];
		ASSERT(Dee_memval_isdirect(popiter_mval));
		ASSERT(Dee_memval_direct_local_neverbound(popiter_mval));
		Dee_memval_init_memobj_inherit(popiter_mval, &decref_on_iter_done);
	} else {
		Dee_memobj_fini(&decref_on_iter_done);
	}

	return temp;
err:
	return -1;
}

__pragma_GCC_diagnostic_pop_ignored(Wmaybe_uninitialized)



/* >> TOP = *(TOP + ind_delta); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vind(struct Dee_function_generator *__restrict self,
                            ptrdiff_t ind_delta) {
	struct Dee_memstate *state;
	struct Dee_memval *mval;
	struct Dee_memloc ind_loc;
	DO(Dee_function_generator_vdirect1(self));
	DO(Dee_function_generator_state_unshare(self));
	state = self->fg_state;
	mval  = Dee_memstate_vtop(state);
	ASSERTF(Dee_memval_isdirect(mval), "Cannot do indirection on non-direct location"); /* TODO: This shouldn't be the case! */
	ASSERTF(!Dee_memval_direct_isref(mval), "Cannot do indirection on location holding a reference");
	DO(Dee_function_generator_gasind(self, Dee_memval_direct_getloc(mval), &ind_loc, ind_delta));
	ASSERT(state == self->fg_state);
	ASSERT(mval == Dee_memstate_vtop(state));
	Dee_memstate_decrinuse_for_memloc(state, Dee_memval_direct_getloc(mval));
	*Dee_memval_direct_getloc(mval) = ind_loc;
	Dee_memstate_incrinuse_for_memloc(state, &ind_loc);
	Dee_memval_direct_settypeof(mval, NULL); /* Unknown */
	return 0;
err:
	return -1;
}

/* >> *(SECOND + ind_delta) = POP(); // NOTE: Ignores `mv_vmorph' in SECOND */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpopind(struct Dee_function_generator *__restrict self,
                               ptrdiff_t ind_delta) {
	struct Dee_memloc src, *dst;
	DO(Dee_function_generator_vdirect1(self)) /* !!! Only the value getting assigned is made direct! */;
	DO(Dee_function_generator_vnotoneref_at(self, 1));
	DO(Dee_function_generator_state_unshare(self));
	src = *Dee_function_generator_vtopdloc(self);
	Dee_memstate_decrinuse_for_memloc(self->fg_state, &src);
	--self->fg_state->ms_stackc;
	ASSERT(Dee_memval_hasobj0(Dee_function_generator_vtop(self)));
	dst = Dee_memobj_getloc(Dee_memval_getobj0(Dee_function_generator_vtop(self)));
	return Dee_function_generator_gmov_loc2locind(self, &src, dst, ind_delta);
err:
	return -1;
}

/* >> TOP = TOP + val_delta; // NOTE: Ignores `mv_vmorph' */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdelta(struct Dee_function_generator *__restrict self,
                              ptrdiff_t val_delta) {
	struct Dee_memval *mval;
	if unlikely(val_delta == 0)
		return 0;
	DO(Dee_function_generator_state_unshare(self));
	mval = Dee_function_generator_vtop(self);
	ASSERT(Dee_memval_hasobj0(mval));
	ASSERTF(!Dee_memobj_isref(Dee_memval_getobj0(mval)),
	        "Cannot add delta to location holding a reference");
	Dee_memloc_adjoff(Dee_memobj_getloc(Dee_memval_getobj0(mval)), val_delta);
	mval->mv_vmorph = MEMVAL_VMORPH_DIRECT;
	Dee_memval_direct_settypeof(mval, NULL); /* Unknown */
	return 0;
err:
	return -1;
}

/* >> temp = *(SECOND + ind_delta);
 * >> *(SECOND + ind_delta) = FIRST;
 * >> POP();
 * >> POP();
 * >> PUSH(temp, MEMOBJ_F_NOREF); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vswapind(struct Dee_function_generator *__restrict self,
                                ptrdiff_t ind_delta) {
	DO(Dee_function_generator_vswap(self));              /* src, dst */
	DO(Dee_function_generator_vdup(self));               /* src, dst, dst */
	DO(Dee_function_generator_vind(self, ind_delta));    /* src, dst, *(dst + ind_delta) */
	DO(Dee_function_generator_vreg(self, NULL));         /* src, dst, reg:*(dst + ind_delta) */
	Dee_function_generator_vtop_direct_clearref(self);   /* src, dst, reg:*(dst + ind_delta) */
	DO(Dee_function_generator_vrrot(self, 3));           /* reg:*(dst + ind_delta), src, dst */
	DO(Dee_function_generator_vswap(self));              /* reg:*(dst + ind_delta), dst, src */
	DO(Dee_function_generator_vpopind(self, ind_delta)); /* reg:*(dst + ind_delta), dst */
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

/* Ensure that the top-most `DeeObject' from the object-stack is a reference. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vref(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *mval;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = Dee_function_generator_vtop(self);
	if (Dee_memval_isnullable(mval) && Dee_memobj_isref(Dee_memval_nullable_getobj(mval)))
		return 0; /* Special case: NULLABLE+REF is allowed (and we don't make it direct) */
	DO(Dee_function_generator_vdirect1(self));
	mval = Dee_function_generator_vtop(self);
	ASSERT(Dee_memval_isdirect(mval));
	if (!Dee_memval_direct_isref(mval)) {
		struct Dee_memstate *state;
		struct Dee_memval *alias_mval;
		struct Dee_memobj *alias_mobj;
		bool did_find_first_alias;
		DO(Dee_function_generator_state_unshare(self));
		state = self->fg_state;
		mval  = Dee_memstate_vtop(state);
		ASSERT(Dee_memval_isdirect(mval));
		ASSERT(!Dee_memval_direct_isref(mval));

		/* If at least 2 other memory locations (or 1 if it's a constant) are already
		 * holding a reference to the same value, then we can steal a reference from
		 * one of them!
		 *
		 * The reason for that "2" is because as long as there are 2 references, an
		 * object is guarantied to have `DeeObject_IsShared()', meaning that whatever
		 * the caller might need the reference for, the object won't end up getting
		 * destroyed if the reference ends up being dropped! */
		did_find_first_alias = false;
		Dee_memstate_foreach(alias_mval, state) {
			if unlikely(alias_mval->mv_flags & MEMVAL_F_NOREF) {
				ASSERT(!Dee_memval_hasobj0(alias_mval));
				continue;
			}
			Dee_memval_foreach_obj(alias_mobj, alias_mval) {
				if (Dee_memobj_isref(alias_mobj) &&
				    Dee_memobj_sameloc(alias_mobj, Dee_memval_direct_getobj(mval))) {
					if (did_find_first_alias) {
						/* Steal the reference from `alias_mobj' */
						Dee_memobj_clearref(alias_mobj);
						Dee_memval_direct_setref(mval);
						return 0;
					}
					did_find_first_alias = true;
				}
			}
			Dee_memval_foreach_obj_end;
		}
		Dee_memstate_foreach_end;
		DO(Dee_function_generator_gincref_loc(self, Dee_memval_direct_getloc(mval), 1));
		ASSERT(mval == Dee_memstate_vtop(state));
		ASSERT(!Dee_memval_direct_isref(mval));
		ASSERT(!Dee_memval_direct_isoneref(mval));
		Dee_memval_direct_setref(mval);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vref_noconst(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *mval;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = Dee_function_generator_vtop(self);
	if (Dee_memval_isnullable(mval) && Dee_memobj_isref(Dee_memval_nullable_getobj(mval)))
		return 0; /* Special case: NULLABLE+REF is allowed (and we don't make it direct) */
	DO(Dee_function_generator_vdirect1(self));
	mval = Dee_function_generator_vtop(self);
	ASSERT(Dee_memval_isdirect(mval));
	if (!Dee_memval_direct_isref(mval) &&
	    !Dee_memval_direct_isconst(mval))
		return Dee_function_generator_vref(self);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vref_noalias(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *mval;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = Dee_function_generator_vtop(self);
	if (Dee_memval_isnullable(mval) && Dee_memobj_isref(Dee_memval_nullable_getobj(mval)))
		return 0; /* Special case: NULLABLE+REF is allowed (and we don't make it direct) */
	DO(Dee_function_generator_vdirect1(self));
	mval = Dee_function_generator_vtop(self);
	ASSERT(Dee_memval_isdirect(mval));
	if (!Dee_memval_direct_isref(mval)) {
		struct Dee_memstate *state;
		DO(Dee_function_generator_state_unshare(self));
		state = self->fg_state;
		mval  = Dee_memstate_vtop(state);
		ASSERT(Dee_memval_isdirect(mval));
		ASSERT(!Dee_memval_direct_isref(mval));
		DO(Dee_function_generator_gincref_loc(self, Dee_memval_direct_getloc(mval), 1));
		ASSERT(mval == Dee_memstate_vtop(state));
		ASSERT(!Dee_memval_direct_isref(mval));
		ASSERT(!Dee_memval_direct_isoneref(mval));
		Dee_memval_direct_setref(mval);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vref_noconst_noalias(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *mval;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = Dee_function_generator_vtop(self);
	if (Dee_memval_isnullable(mval) && Dee_memobj_isref(Dee_memval_nullable_getobj(mval)))
		return 0; /* Special case: NULLABLE+REF is allowed (and we don't make it direct) */
	DO(Dee_function_generator_vdirect1(self));
	mval = Dee_function_generator_vtop(self);
	ASSERT(Dee_memval_isdirect(mval));
	if (!Dee_memval_direct_isref(mval) &&
	    !Dee_memval_direct_isconst(mval)) {
		struct Dee_memstate *state;
		DO(Dee_function_generator_state_unshare(self));
		state = self->fg_state;
		mval  = Dee_memstate_vtop(state);
		ASSERT(Dee_memval_isdirect(mval));
		ASSERT(!Dee_memval_direct_isref(mval));
		DO(Dee_function_generator_gincref_loc(self, Dee_memval_direct_getloc(mval), 1));
		ASSERT(mval == Dee_memstate_vtop(state));
		ASSERT(!Dee_memval_direct_isref(mval));
		ASSERT(!Dee_memval_direct_isoneref(mval));
		Dee_memval_direct_setref(mval);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vref2(struct Dee_function_generator *__restrict self,
                             Dee_vstackaddr_t dont_steal_from_vtop_n) {
	struct Dee_memval *mval;
	DO(Dee_function_generator_vdirect1(self));
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = Dee_function_generator_vtop(self);
	ASSERT(Dee_memval_isdirect(mval));
	if (Dee_memval_direct_isconst(mval) && Dee_memval_direct_isref(mval))
		return 0; /* Special case: a reference to a constant doesn't need its aliases to have references also. */
	DO(Dee_function_generator_state_unshare(self));
	mval = Dee_function_generator_vtop(self);
	ASSERT(Dee_memval_isdirect(mval));
	return Dee_function_generator_gref2(self, Dee_memval_direct_getobj(mval),
	                                    dont_steal_from_vtop_n);
err:
	return -1;
}


/* Ensure that `mobj' is holding a reference. If said location has aliases,
 * and isn't a constant, then also ensure that at least one of those aliases
 * also contains a second reference.
 * @param: dont_steal_from_vtop_n: Ignore the top n v-stack items when searching for aliases. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gref2(struct Dee_function_generator *__restrict self,
                             struct Dee_memobj *mobj,
                             Dee_vstackaddr_t dont_steal_from_vtop_n) {
	struct Dee_memstate *state = self->fg_state;
	struct Dee_memval *alias_mval;
	struct Dee_memobj *alias_mobj;
	struct Dee_memobj *alias_with_reference = NULL;    /* Alias that has a reference */
	struct Dee_memobj *alias_without_reference = NULL; /* Alias that needs a reference */
	bool got_alias = false; /* There *are* aliases. */
	ASSERT(state->ms_stackc >= dont_steal_from_vtop_n);
	state->ms_stackc -= dont_steal_from_vtop_n;
	Dee_memstate_foreach(alias_mval, state) {
again_foreach_alias_mobj:
		Dee_memval_foreach_obj(alias_mobj, alias_mval) {
			if (Dee_memobj_sameloc(alias_mobj, mobj) && alias_mobj != mobj) {
				if unlikely(alias_mval->mv_flags & MEMVAL_F_NOREF) {
					ASSERT(!Dee_memval_hasobj0(alias_mval));
					if unlikely(Dee_memval_do_clear_MEMVAL_F_NOREF(alias_mval))
						goto again_foreach_alias_mobj;
				}
				/* Got an alias! */
				got_alias = true;
				if (!Dee_memobj_isref(alias_mobj)) {
					alias_without_reference = alias_mobj;
				} else if (!Dee_memobj_isref(mobj)) {
					/* Steal reference from alias_mobj */
					Dee_memobj_setref(mobj);
					Dee_memobj_clearref(alias_mobj);
					alias_without_reference = alias_mobj;
				} else {
					alias_with_reference = alias_mobj;
				}
			}
		}
		Dee_memval_foreach_obj_end;
	}
	Dee_memstate_foreach_end;
	state->ms_stackc += dont_steal_from_vtop_n;
	if (got_alias) {
		ASSERT(!alias_with_reference || Dee_memobj_isref(alias_with_reference));
		ASSERT(!alias_without_reference || !Dee_memobj_isref(alias_without_reference));
		if (!Dee_memobj_isref(mobj)) {
			/* There are aliases, but no-one is holding a reference.
			 * This can happen if the location points to a constant
			 * that got flushed, or is a function argument, in which
			 * case we only need a single reference. */
			ASSERT(alias_without_reference);
			ASSERT(!alias_with_reference);
			ASSERT(!Dee_memstate_isshared(state));
			DO(Dee_function_generator_gincref_loc(self, Dee_memobj_getloc(mobj), 1));
			Dee_memobj_setref(mobj);
			DO(Dee_function_generator_gnotoneref(self, mobj));
		} else if (alias_without_reference && !alias_with_reference &&
		           /* When it's a constant, there is already an extra reference through code dependencies */
		           !Dee_memobj_isconst(mobj)) {
			/* There are aliases, but less that 2 references -> make sure there are at least 2 references */
			ASSERT(!Dee_memstate_isshared(state));
			ASSERT(!Dee_memobj_isref(alias_without_reference));
			DO(Dee_function_generator_gincref_loc(self, Dee_memobj_getloc(alias_without_reference), 1));
			Dee_memobj_setref(alias_without_reference);
			DO(Dee_function_generator_gnotoneref(self, alias_without_reference));
		}
	} else {
		/* No aliases exist, so there's no need to force a distinct location. */
		if (!Dee_memobj_isref(mobj)) {
			ASSERT(!Dee_memstate_isshared(state));
			DO(Dee_function_generator_gincref_loc(self, Dee_memobj_getloc(mobj), 1));
			Dee_memobj_setref(mobj);
			DO(Dee_function_generator_gnotoneref(self, mobj));
		}
	}
	ASSERT(Dee_memobj_isref(mobj));
	return 0;
err:
	return -1;
}

/* Force vtop into a register (ensuring it has type `MEMADR_TYPE_HREG' for all locations used by VTOP) */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vreg(struct Dee_function_generator *__restrict self,
                            Dee_host_register_t const *not_these) {
	struct Dee_memstate *state = self->fg_state;
	struct Dee_memval *mval;
	struct Dee_memobj *mobj;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
again:
	mval = Dee_memstate_vtop(state);
	Dee_memval_foreach_obj(mobj, mval) {
		if (Dee_memobj_gettyp(mobj) != MEMADR_TYPE_HREG) {
			if (Dee_memstate_isshared(state)) {
				state = Dee_memstate_copy(state);
				if unlikely(!state)
					goto err;
				Dee_memstate_decref_nokill(self->fg_state);
				self->fg_state = state;
				goto again;
			}
			DO(tracked_memloc_forcereg(self, Dee_memobj_getloc(mobj), not_these));
			ASSERT(Dee_memobj_gettyp(mobj) == MEMADR_TYPE_HREG);
		}
	}
	Dee_memval_foreach_obj_end;
	return 0;
err:
	return -1;
}

/* Force vtop onto the stack (ensuring it has type `MEMADR_TYPE_HSTACKIND,
 * Dee_memloc_hstackind_getvaloff = 0' for all locations used by VTOP) */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vflush(struct Dee_function_generator *__restrict self,
                              bool require_valoff_0) {
	struct Dee_memstate *state = self->fg_state;
	struct Dee_memval *mval;
	struct Dee_memobj *mobj;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
again:
	mval = Dee_memstate_vtop(state);
	Dee_memval_foreach_obj(mobj, mval) {
		if (Dee_memobj_gettyp(mobj) != MEMADR_TYPE_HSTACKIND ||
		    (Dee_memobj_hstackind_getvaloff(mobj) != 0 && require_valoff_0)) {
			struct Dee_memloc flushed_loc;
			if (Dee_memstate_isshared(state)) {
				state = Dee_memstate_copy(state);
				if unlikely(!state)
					goto err;
				Dee_memstate_decref_nokill(self->fg_state);
				self->fg_state = state;
				goto again;
			}
			DO(Dee_function_generator_gasflush(self, Dee_memobj_getloc(mobj),
			                                   &flushed_loc, require_valoff_0));
			ASSERT(Dee_memloc_gettyp(&flushed_loc) == MEMADR_TYPE_HSTACKIND);
			ASSERT(Dee_memloc_hstackind_getvaloff(&flushed_loc) == 0 || !require_valoff_0);
			Dee_memstate_decrinuse_for_memobj(state, mobj);
			*Dee_memobj_getloc(mobj) = flushed_loc;
		}
	}
	Dee_memval_foreach_obj_end;
	return 0;
err:
	return -1;
}


/* Generate code to push a global variable onto the virtual stack. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpush_mod_global(struct Dee_function_generator *__restrict self,
                                        struct Dee_module_object *mod, uint16_t gid, bool ref) {
	struct Dee_memloc *loc;
	struct module_symbol *symbol;
	if unlikely(gid >= mod->mo_globalc)
		return err_illegal_gid(mod, gid);
	symbol = DeeModule_GetSymbolID(mod, gid);
	ASSERT(!symbol || symbol->ss_index == gid);
	/* Global object references can be inlined if they are `final' and bound */
	if (((symbol == NULL) || /* Can be NULL in case it's the DELETE/SETTER of a property */
	     (symbol->ss_flags & (Dee_MODSYM_FPROPERTY | Dee_MODSYM_FREADONLY))) &&
	    !(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOROINLINE)) {
		DeeObject *current_value;
		DeeModule_LockRead(mod);
		current_value = mod->mo_globalv[gid];
		if (current_value != NULL) {
			Dee_Incref(current_value);
			DeeModule_LockEndRead(mod);
			current_value = Dee_function_generator_inlineref(self, current_value);
			if unlikely(!current_value)
				goto err;
			return Dee_function_generator_vpush_const(self, current_value);
		}
		DeeModule_LockEndRead(mod);
	}
	DO(Dee_function_generator_vpush_addr(self, &mod->mo_globalv[gid]));
	if (ref)
		DO(Dee_function_generator_grwlock_read_const(self, &mod->mo_lock));
	DO(Dee_function_generator_vind(self, 0));
	DO(Dee_function_generator_vreg(self, NULL));
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	ASSERT(!Dee_function_generator_vtop_direct_isref(self));
	loc = Dee_function_generator_vtopdloc(self);
	DO(Dee_function_generator_gassert_bound(self, loc, NULL, mod, gid,
	                                        ref ? &mod->mo_lock : NULL,
	                                        NULL));

	/* Depending on how the value will be used, we may not need a reference.
	 * If only its value is used (ASM_ISNONE, ASM_CMP_SO, ASM_CMP_DO), we
	 * won't actually need to take a reference here!
	 * Also: when not needing a reference, we don't need to acquire the lock,
	 *       either! */
	ASSERT(!Dee_function_generator_vtop_direct_isref(self));
	if (ref) {
		DO(Dee_function_generator_gincref_loc(self, loc, 1));
		DO(Dee_function_generator_grwlock_endread_const(self, &mod->mo_lock));
		ASSERT(!Dee_function_generator_vtop_direct_isref(self));
		Dee_function_generator_vtop_direct_setref(self);
	}
	return 0;
err:
	return -1;
}

/* Generate code to check if a global variable is currently bound. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vbound_mod_global(struct Dee_function_generator *__restrict self,
                                         struct Dee_module_object *mod, uint16_t gid) {
	struct Dee_memval *mval;
	struct module_symbol *symbol;
	if unlikely(gid >= mod->mo_globalc)
		return err_illegal_gid(mod, gid);
	symbol = DeeModule_GetSymbolID(mod, gid);
	ASSERT(!symbol || symbol->ss_index == gid);
	/* If the symbol is read-only and bound, then we know it can't be unbound */
	if (((symbol == NULL) || /* Can be NULL in case it's the DELETE/SETTER of a property */
	     (symbol->ss_flags & (Dee_MODSYM_FPROPERTY | Dee_MODSYM_FREADONLY))) &&
	    !(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOROINLINE)) {
		DeeObject *current_value = atomic_read(&mod->mo_globalv[gid]);
		if (current_value != NULL)
			return Dee_function_generator_vpush_const(self, Dee_True);
	}
	DO(Dee_function_generator_vpush_addr(self, &mod->mo_globalv[gid]));
	DO(Dee_function_generator_vind(self, 0));
	mval = Dee_function_generator_vtop(self);
	ASSERT(Dee_memval_isdirect(mval));
	mval->mv_vmorph = MEMVAL_VMORPH_TESTNZ(mval->mv_vmorph);
	return 0;
err:
	return -1;
}

/* Generate code to pop a global variable from the virtual stack. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vpopref_mod_global(struct Dee_function_generator *__restrict self,
                   struct Dee_module_object *mod, uint16_t gid) {
	struct Dee_memloc loc;
	if unlikely(gid >= mod->mo_globalc)
		return err_illegal_gid(mod, gid);
	DO(Dee_function_generator_vdirect1(self));                              /* value */
	DO(Dee_function_generator_vpush_addr(self, &mod->mo_globalv[gid]));     /* value, &GLOBAL */
	DO(Dee_function_generator_vswap(self));                                 /* &GLOBAL, value */
	DO(Dee_function_generator_grwlock_write_const(self, &mod->mo_lock));    /* &GLOBAL, value */
	DO(Dee_function_generator_vswapind(self, 0));                           /* ref:old_value */
	DO(Dee_function_generator_grwlock_endwrite_const(self, &mod->mo_lock)); /* ref:old_value */
	ASSERT(self->fg_state->ms_stackc >= 1);
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	ASSERT(!Dee_function_generator_vtop_direct_isref(self));
	loc = *Dee_function_generator_vtopdloc(self);
	Dee_memstate_decrinuse_for_memloc(self->fg_state, &loc);
	--self->fg_state->ms_stackc;
	return Dee_function_generator_gxdecref_loc(self, &loc, 1); /* xdecref in case global wasn't bound before. */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vdel_mod_global(struct Dee_function_generator *__restrict self,
                                       struct Dee_module_object *mod, uint16_t gid) {
	int result = Dee_function_generator_vpush_addr(self, NULL);
	if likely(result == 0)
		result = vpopref_mod_global(self, mod, gid);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vpop_mod_global(struct Dee_function_generator *__restrict self,
                                       struct Dee_module_object *mod, uint16_t gid) {
	int result = Dee_function_generator_vref2(self, 1);
	if likely(result == 0)
		result = vpopref_mod_global(self, mod, gid);
	return result;
}


INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_extern(struct Dee_function_generator *__restrict self,
                                    uint16_t mid, uint16_t gid, bool ref) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	if unlikely(mid >= mod->mo_importc)
		return err_illegal_mid(mid);
	mod = mod->mo_importv[mid];
	return Dee_function_generator_vpush_mod_global(self, mod, gid, ref);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vbound_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	if unlikely(mid >= mod->mo_importc)
		return err_illegal_mid(mid);
	mod = mod->mo_importv[mid];
	return Dee_function_generator_vbound_mod_global(self, mod, gid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdel_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	if unlikely(mid >= mod->mo_importc)
		return err_illegal_mid(mid);
	mod = mod->mo_importv[mid];
	return Dee_function_generator_vdel_mod_global(self, mod, gid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_extern(struct Dee_function_generator *__restrict self, uint16_t mid, uint16_t gid) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	if unlikely(mid >= mod->mo_importc)
		return err_illegal_mid(mid);
	mod = mod->mo_importv[mid];
	return Dee_function_generator_vpop_mod_global(self, mod, gid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_static(struct Dee_function_generator *__restrict self, uint16_t sid) {
	DeeCodeObject *code = self->fg_assembler->fa_code;
	if unlikely(sid >= code->co_staticc)
		return err_illegal_sid(sid);
	DO(Dee_function_generator_vpush_addr(self, &code->co_staticv[sid]));
	DO(Dee_function_generator_grwlock_read_const(self, &code->co_static_lock));
	DO(Dee_function_generator_vind(self, 0));
	DO(Dee_function_generator_vreg(self, NULL));
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	ASSERT(!Dee_function_generator_vtop_direct_isref(self));
	DO(Dee_function_generator_gincref_loc(self, Dee_function_generator_vtopdloc(self), 1));
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	ASSERT(!Dee_function_generator_vtop_direct_isref(self));
	Dee_function_generator_vtop_direct_setref(self);
	return Dee_function_generator_grwlock_endread_const(self, &code->co_static_lock);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_static(struct Dee_function_generator *__restrict self, uint16_t sid) {
	DeeCodeObject *code = self->fg_assembler->fa_code;
	if unlikely(sid >= code->co_staticc)
		return err_illegal_sid(sid);
	DO(Dee_function_generator_vdirect1(self));
	DO(Dee_function_generator_vref2(self, 1));
	ASSERT(Dee_function_generator_vtop_direct_isref(self));
	DO(Dee_function_generator_vpush_addr(self, &code->co_staticv[sid])); /* value, addr */
	DO(Dee_function_generator_vswap(self));                              /* addr, value */
	DO(Dee_function_generator_grwlock_write_const(self, &code->co_static_lock));
	DO(Dee_function_generator_vswapind(self, 0)); /* old_value */
	DO(Dee_function_generator_grwlock_endwrite_const(self, &code->co_static_lock));
	ASSERT(!Dee_function_generator_vtop_direct_isref(self));
	Dee_function_generator_vtop_direct_setref(self);
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vrwlock_read(struct Dee_function_generator *__restrict self) {
	DO(Dee_function_generator_vdirect1(self));
	DO(Dee_function_generator_state_unshare(self));
	DO(Dee_function_generator_grwlock_read(self, Dee_function_generator_vtopdloc(self)));
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vrwlock_write(struct Dee_function_generator *__restrict self) {
	DO(Dee_function_generator_vdirect1(self));
	DO(Dee_function_generator_state_unshare(self));
	DO(Dee_function_generator_grwlock_write(self, Dee_function_generator_vtopdloc(self)));
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vrwlock_endread(struct Dee_function_generator *__restrict self) {
	DO(Dee_function_generator_vdirect1(self));
	DO(Dee_function_generator_state_unshare(self));
	DO(Dee_function_generator_grwlock_endread(self, Dee_function_generator_vtopdloc(self)));
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vrwlock_endwrite(struct Dee_function_generator *__restrict self) {
	DO(Dee_function_generator_vdirect1(self));
	DO(Dee_function_generator_state_unshare(self));
	DO(Dee_function_generator_grwlock_endwrite(self, Dee_function_generator_vtopdloc(self)));
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

/* Make sure there are no NULLABLE memobj-s anywhere on the stack or in locals. */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_vnonullable(struct Dee_function_generator *__restrict self) {
	struct Dee_memstate *state;
	struct Dee_memval *mval;
	DO(Dee_function_generator_state_unshare(self));
	state = self->fg_state;
	ASSERT(state->ms_flags & MEMSTATE_F_GOTNULLABLE);
	Dee_memstate_foreach(mval, state) {
		if (mval->mv_vmorph == MEMVAL_VMORPH_NULLABLE)
			return Dee_function_generator_vdirect_memval(self, mval);
	}
	Dee_memstate_foreach_end;
	/* Just clear the flag (something might have forgotten to clear it) */
	state->ms_flags &= ~MEMSTATE_F_GOTNULLABLE;
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
	struct Dee_memloc *vtop_loc;
	DO(Dee_function_generator_vdirect1(self));
	ASSERT(Dee_function_generator_vtop_isdirect(self));
	vtop_loc = Dee_function_generator_vtopdloc(self);
	if (Dee_memloc_sameloc(vtop_loc, loc))
		return 0; /* Already the same location -> no need to do anything */
	DO(Dee_function_generator_state_unshare(self));
	vtop_loc = Dee_function_generator_vtopdloc(self);
	result   = Dee_function_generator_gmov_loc2loc(self, vtop_loc, loc);
	if likely(result == 0) {
		Dee_memstate_decrinuse_for_memloc(self->fg_state, vtop_loc);
		*vtop_loc = *loc;
		Dee_memstate_incrinuse_for_memloc(self->fg_state, vtop_loc);
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
	struct Dee_memval *p_mval;
	Dee_vstackaddr_t stackc = self->fg_state->ms_stackc;
	Dee_lid_t lid;
	if unlikely(stackc < 1)
		return err_illegal_stack_effect();

	/* Special case: NULLABLE locations can be returned as-is */
	p_mval = &self->fg_state->ms_stackv[stackc - 1];
	if (!Dee_memval_isdirect(p_mval) && !Dee_memval_isnullable(p_mval)) {
		DO(Dee_function_generator_vdirect1(self));
	}

	/* Move the final return value to the bottom of the stack. */
	DO(Dee_function_generator_vrrot(self, stackc));

	/* Remove all but the final element from the stack. */
	DO(Dee_function_generator_vpopmany(self, stackc - 1));

	/* Unbind all local variables. */
	for (lid = 0; lid < self->fg_state->ms_localc; ++lid)
		DO(Dee_function_generator_vdel_local(self, lid));

	/* Ensure that the final stack element contains a reference. */
	DO(Dee_function_generator_vref_noalias(self));

	/* Steal the final (returned) object from stack */
	ASSERT(self->fg_state->ms_stackc == 1);
	ASSERT(Dee_memval_isdirect(&self->fg_state->ms_stackv[0]) ||
	       Dee_memval_isnullable(&self->fg_state->ms_stackv[0]));
	ASSERT(Dee_memval_hasobj0(&self->fg_state->ms_stackv[0]));
	ASSERT(Dee_memobj_isref(Dee_memval_getobj0(&self->fg_state->ms_stackv[0])));
	loc = *Dee_memobj_getloc(Dee_memval_getobj0(&self->fg_state->ms_stackv[0]));
	Dee_memstate_decrinuse_for_memloc(self->fg_state, &loc);
	self->fg_state->ms_stackc = 0;

	/* Generate code to do the return. */
	return Dee_function_generator_gret(self, &loc);
err:
	return -1;
}



/* Do calling-convention-specific handling of the return value. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcallapi_checkresult(struct Dee_function_generator *__restrict self,
                                            unsigned int cc, Dee_vstackaddr_t argc) {
	switch (cc) {

	case VCALL_CC_OBJECT:
		DO(Dee_function_generator_vpush_hreg(self, HOST_REGISTER_RETURN, 0)); /* [args...], UNCHECKED(result) */
		DO(Dee_function_generator_vrrot(self, argc + 1));                     /* UNCHECKED(result), [args...] */
		DO(Dee_function_generator_vpopmany(self, argc));                      /* UNCHECKED(result) */
		return Dee_function_generator_vcheckobj(self);                        /* result */

	case VCALL_CC_RAWINTPTR:
	case VCALL_CC_RAWINTPTR_NX:
		DO(Dee_function_generator_vpush_hreg(self, HOST_REGISTER_RETURN, 0)); /* [args...], UNCHECKED(result) */
		DO(Dee_function_generator_vrrot(self, argc + 1));                     /* UNCHECKED(result), [args...] */
		break;

	case VCALL_CC_BOOL:
	case VCALL_CC_BOOL_NX:
		DO(Dee_function_generator_vpush_hreg(self, HOST_REGISTER_RETURN, 0)); /* [args...], UNCHECKED(result) */
		ASSERT(Dee_function_generator_vtop(self)->mv_vmorph == MEMVAL_VMORPH_DIRECT);
		Dee_function_generator_vtop(self)->mv_vmorph = MEMVAL_VMORPH_BOOL_NZ;
		DO(Dee_function_generator_vrrot(self, argc + 1)); /* UNCHECKED(result), [args...] */
		break;

	case VCALL_CC_RAWINTPTR_KEEPARGS:
	case VCALL_CC_RAWINTPTR_KEEPARGS_NX:
		return Dee_function_generator_vpush_hreg(self, HOST_REGISTER_RETURN, 0);

	case VCALL_CC_VOID:
	case VCALL_CC_VOID_NX:
		break;

	case VCALL_CC_EXCEPT:
		DO(Dee_function_generator_vpopmany(self, argc));
		return Dee_function_generator_gjmp_except(self);

	case VCALL_CC_INT:
		DO(Dee_function_generator_vpush_hreg(self, HOST_REGISTER_RETURN, 0)); /* [args...], UNCHECKED(result) */
		DO(Dee_function_generator_vrrot(self, argc + 1));                     /* UNCHECKED(result), [args...] */
		DO(Dee_function_generator_vpopmany(self, argc));                      /* UNCHECKED(result) */
		return Dee_function_generator_vcheckint(self); /* - */

	case VCALL_CC_NEGINT: {
		DO(Dee_function_generator_vpush_hreg(self, HOST_REGISTER_RETURN, 0)); /* [args...], UNCHECKED(result) */
		DO(Dee_function_generator_vrrot(self, argc + 1));                     /* UNCHECKED(result), [args...] */
		DO(Dee_function_generator_vpopmany(self, argc));                      /* UNCHECKED(result) */
		return Dee_function_generator_gjcmp_except(self, Dee_function_generator_vtopdloc(self), 0,
		                                           Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_LO);
	}	break;

	case VCALL_CC_M1INT: {
		DO(Dee_function_generator_vpush_hreg(self, HOST_REGISTER_RETURN, 0)); /* [args...], UNCHECKED(result) */
		DO(Dee_function_generator_vrrot(self, argc + 1));                     /* UNCHECKED(result), [args...] */
		DO(Dee_function_generator_vpopmany(self, argc));                      /* UNCHECKED(result) */
		return Dee_function_generator_gjeq_except(self, Dee_function_generator_vtopdloc(self), -1);
	}	break;

	case VCALL_CC_MORPH_INTPTR:
	case VCALL_CC_MORPH_UINTPTR:
	case VCALL_CC_MORPH_INTPTR_NX:
	case VCALL_CC_MORPH_UINTPTR_NX:
		DO(Dee_function_generator_vpush_hreg(self, HOST_REGISTER_RETURN, 0)); /* [args...], UNCHECKED(result) */
		ASSERT(Dee_function_generator_vtop(self)->mv_vmorph == MEMVAL_VMORPH_DIRECT);
		Dee_function_generator_vtop(self)->mv_vmorph = (cc == VCALL_CC_MORPH_UINTPTR ||
		                                                cc == VCALL_CC_MORPH_UINTPTR_NX)
		                                               ? MEMVAL_VMORPH_UINT
		                                               : MEMVAL_VMORPH_INT;
		DO(Dee_function_generator_vrrot(self, argc + 1)); /* UNCHECKED(result), [args...] */
		break;

	default: __builtin_unreachable();
	}

	/* Pop function arguments. */
	return Dee_function_generator_vpopmany(self, argc);
err:
	return -1;
}

/* Generate host text to invoke `api_function' with the top-most `argc' items from the stack.
 * @param: cc: One of `VCALL_CC_*', describing the calling-convention of `api_function'.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcallapi_(struct Dee_function_generator *__restrict self,
                                 void const *api_function, unsigned int cc,
                                 Dee_vstackaddr_t argc) {
	Dee_vstackaddr_t argi;
	struct Dee_memloc *l_argv;
	struct Dee_memval *v_argv;
	DO(Dee_function_generator_vdirect(self, argc));
	DO(Dee_function_generator_state_unshare(self));

	/* Unless the class is *_NX, make sure there aren't any NULLABLE locations */
	switch (cc) {
	case VCALL_CC_RAWINT_NX:
	case VCALL_CC_RAWINT_KEEPARGS_NX:
	case VCALL_CC_VOID_NX:
	case VCALL_CC_BOOL_NX:
	case VCALL_CC_MORPH_INTPTR_NX:
	case VCALL_CC_MORPH_UINTPTR_NX:
		break;
	default:
		DO(Dee_function_generator_vnonullable(self));
		break;
	}

	/* Flush registers that don't appear in the top `argc' stack locations.
	 * When the function always throw an exception, we *only* need to preserve
	 * stuff that contains references! */
	DO(Dee_function_generator_gflushregs(self, argc, cc == VCALL_CC_EXCEPT));

	/* Build up the argument list. */
	v_argv = self->fg_state->ms_stackv;
	v_argv += self->fg_state->ms_stackc;
	v_argv -= argc;
	l_argv = (struct Dee_memloc *)Dee_Mallocac(argc + 1, sizeof(struct Dee_memloc));
	if unlikely(!l_argv)
		goto err;
	Dee_memloc_init_const(&l_argv[0], api_function);
	for (argi = 0; argi < argc; ++argi)
		l_argv[argi + 1] = *Dee_memval_direct_getloc(&v_argv[argi]);

	/* Call the actual C function */
	if unlikely(Dee_function_generator_gcallapi(self, l_argv, argc))
		goto err_l_argv;
	Dee_Freea(l_argv);

	/* Do calling-convention-specific handling of the return value. */
	return Dee_function_generator_vcallapi_checkresult(self, cc, argc);
err_l_argv:
	Dee_Freea(l_argv);
err:
	return -1;
}

/* [args...], funcaddr -> ...
 * Same as `Dee_function_generator_vcallapi()', but after the normal argument list,
 * there is an additional item "funcaddr" that contains the (possibly) runtime-
 * evaluated address of the function that should be called. Also note that said
 * "funcaddr" location is *always* popped.
 * @param: cc: One of `VCALL_CC_*', describing the calling-convention of `api_function' 
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcalldynapi(struct Dee_function_generator *__restrict self,
                                   unsigned int cc, Dee_vstackaddr_t argc) {
	Dee_vstackaddr_t argi;
	struct Dee_memloc *l_argv;
	struct Dee_memval *v_argv;
	DO(Dee_function_generator_vdirect(self, argc + 1));
	DO(Dee_function_generator_state_unshare(self));
	v_argv = Dee_function_generator_vtop(self);
	if (Dee_memval_direct_isconst(v_argv)) {
		/* Special case: function being called is actually at a constant address. */
		void const *api_function = Dee_memval_const_getaddr(v_argv);
		DO(Dee_function_generator_vpop(self));
		return Dee_function_generator_vcallapi(self, api_function, cc, argc);
	}

	/* Flush registers that don't appear in the top `argc' stack locations.
	 * When the function always throw an exception, we *only* need to preserve
	 * stuff that contains references! */
	DO(Dee_function_generator_gflushregs(self, argc + 1, cc == VCALL_CC_EXCEPT));

	/* Build up the argument list. */
	v_argv -= argc;
	l_argv = (struct Dee_memloc *)Dee_Mallocac(1 + argc, sizeof(struct Dee_memloc));
	if unlikely(!l_argv)
		goto err;
	for (argi = 0; argi < argc; ++argi)
		l_argv[1 + argi] = *Dee_memval_direct_getloc(&v_argv[argi]);
	l_argv[0] = *Dee_function_generator_vtopdloc(self);
	if unlikely(Dee_function_generator_vpop(self))
		goto err_l_argv;

	/* Call the actual C function */
	if unlikely(Dee_function_generator_gcallapi(self, l_argv, argc))
		goto err_l_argv;
	Dee_Freea(l_argv);

	/* Do calling-convention-specific handling of the return value. */
	return Dee_function_generator_vcallapi_checkresult(self, cc, argc);
err_l_argv:
	Dee_Freea(l_argv);
err:
	return -1;
}


/* After a call to `Dee_function_generator_vcallapi()' with `VCALL_CC_RAWINT',
 * do the extra trailing checks needed to turn that call into `VCALL_CC_OBJECT'
 * The difference to directly passing `VCALL_CC_OBJECT' is that using this 2-step
 * method, you're able to pop more elements from the stack first.
 *
 * However: be careful not to do anything that might throw additional exceptions!
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcheckobj(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *mval;
	DO(Dee_function_generator_state_unshare(self));
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = Dee_function_generator_vtop(self);
	ASSERTF(Dee_memval_isdirect(mval), "non-sensical call: vcheckobj() on non-direct value");

	/* Delay NULL checks until a later point in time:
	 * >> return foo();     // No NULL-check needed here (if foo() returns NULL, just propagate as-is)
	 * >> foo({ a, b, c }); // Delayed NULL-check means that the SharedVector can be decref'd before the
	 * >>                   // NULL-check (meaning it doesn't need to be decref'd by exception cleanup code)
	 */
	mval->mv_vmorph = MEMVAL_VMORPH_NULLABLE;

	/* Clear the NOREF flag when the return value is non-NULL */
	if (Dee_memobj_gettyp(Dee_memval_nullable_getobj(mval)) != MEMADR_TYPE_CONST) {
		ASSERT(!Dee_memobj_isref(Dee_memval_nullable_getobj(mval)));
		Dee_memobj_setref(Dee_memval_nullable_getobj(mval));
	}
	return 0;
err:
	return -1;
}

/* After a call to `Dee_function_generator_vcallapi()' with `VCALL_CC_RAWINT',
 * do the extra trailing checks needed to turn that call into `VCALL_CC_INT'
 * The difference to directly passing `VCALL_CC_INT' is that using this 2-step
 * method, you're able to pop more elements from the stack first.
 * NOTE: This function pops one element from the V-stack.
 *
 * However: be careful not to do anything that might throw additional exceptions!
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcheckint(struct Dee_function_generator *__restrict self) {
	struct Dee_memval *mval;
	DO(Dee_function_generator_state_unshare(self));
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = Dee_function_generator_vtop(self);
	ASSERTF(Dee_memval_isdirect(mval), "non-sensical call: vcheckint() on non-direct value");
	DO(Dee_function_generator_gjnz_except(self, Dee_memval_direct_getloc(mval)));
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

/* Branch to exception handling if `vtop' is equal to `except_val' */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcheckerr(struct Dee_function_generator *__restrict self,
                                 intptr_t except_val) {
	struct Dee_memval *mval;
	DO(Dee_function_generator_state_unshare(self));
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = Dee_function_generator_vtop(self);
	ASSERTF(Dee_memval_isdirect(mval), "non-sensical call: vcheckint() on non-direct value");
	DO(Dee_function_generator_gjeq_except(self, Dee_memval_direct_getloc(mval), except_val));
	return Dee_function_generator_vpop(self);
err:
	return -1;
}


/* Generate a call to `DeeObject_MALLOC()' to allocate an uninitialized object that
 * provides for "alloc_size" bytes of memory. If possible, try to dispatch against
 * a slap allocator instead (just like the real DeeObject_MALLOC also does).
 * NOTE: The value pushed onto the V-stack...
 *       - ... already has its MEMOBJ_F_NOREF flag CLEAR!
 *       - ... has already been NULL-checked (i.e. already is a direct value)
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcall_DeeObject_MALLOC(struct Dee_function_generator *__restrict self,
                                              size_t alloc_size, bool do_calloc) {
#ifndef CONFIG_NO_OBJECT_SLABS
	void const *api_function = NULL;
	size_t alloc_pointers = CEILDIV(alloc_size, sizeof(void *));
#define LOCAL_checkfit(_, num_pointers)                                               \
	if (alloc_pointers <= (num_pointers))                                             \
		api_function = do_calloc ? (void const *)&DeeObject_SlabCalloc##num_pointers  \
		                         : (void const *)&DeeObject_SlabMalloc##num_pointers; \
	else
	DeeSlab_ENUMERATE(LOCAL_checkfit);
#undef LOCAL_checkfit
	if (api_function != NULL) {
		DO(Dee_function_generator_vcallapi(self, api_function, VCALL_CC_OBJECT, 0));
	} else
#endif /* !CONFIG_NO_OBJECT_SLABS */
	{
		DO(Dee_function_generator_vpush_immSIZ(self, alloc_size));
		DO(Dee_function_generator_vcallapi(self,
		                                   do_calloc ? (void const *)&DeeObject_Calloc
		                                             : (void const *)&DeeObject_Malloc,
		                                   VCALL_CC_OBJECT, 1));
	}
	DO(Dee_function_generator_vdirect1(self));
	DO(Dee_function_generator_state_unshare(self));
	/* The NOREF flag must *NOT* be set (because the intend is for the caller to create an object) */
	ASSERT(Dee_function_generator_vtop_direct_isref(self));
	return Dee_function_generator_voneref_noalias(self); /* Initial reference -> oneref */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcall_DeeObject_Malloc(struct Dee_function_generator *__restrict self,
                                              size_t alloc_size, bool do_calloc) {
	DO(Dee_function_generator_vpush_immSIZ(self, alloc_size));
	DO(Dee_function_generator_vcallapi(self,
	                                   do_calloc ? (void const *)&DeeObject_Calloc
	                                             : (void const *)&DeeObject_Malloc,
	                                   VCALL_CC_OBJECT, 1));
	DO(Dee_function_generator_vdirect1(self));
	DO(Dee_function_generator_state_unshare(self));
	/* The NOREF flag must *NOT* be set (because the intend is for the caller to create an object) */
	ASSERT(Dee_function_generator_vtop_direct_isref(self));
	return Dee_function_generator_voneref_noalias(self); /* Initial reference -> oneref */
err:
	return -1;
}



typedef struct {
	OBJECT_HEAD
	COMPILER_FLEXIBLE_ARRAY(void const *, dvo_items);
} DummyVectorObject;

PRIVATE DeeTypeObject DeeVectorDummy_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_VectorDummy",
	/* .tp_doc      = */ DOC("Used for storing dummy vector in _hostasm"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				/* .tp_free      = */ (dfunptr_t)NULL
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE WUNUSED DREF DummyVectorObject *DCALL
DummyVector_New(size_t num_items) {
	DREF DummyVectorObject *result;
	result = (DREF DummyVectorObject *)DeeObject_Malloc(offsetof(DummyVectorObject, dvo_items) +
	                                                    (num_items * sizeof(void *)));
	if likely(result)
		DeeObject_Init(result, &DeeVectorDummy_Type);
	return result;
}


#define LOCAL_hstack_lowcfa_inuse(bitset, low_cfa_offset) bitset_test(bitset, (low_cfa_offset) / HOST_SIZEOF_POINTER)
#ifdef HOSTASM_STACK_GROWS_DOWN
#define LOCAL_hstack_cfa_inuse(bitset, cfa_offset) LOCAL_hstack_lowcfa_inuse(bitset, (cfa_offset) - HOST_SIZEOF_POINTER)
#else /* HOSTASM_STACK_GROWS_DOWN */
#define LOCAL_hstack_cfa_inuse(bitset, cfa_offset) LOCAL_hstack_lowcfa_inuse(bitset, cfa_offset)
#endif /* !HOSTASM_STACK_GROWS_DOWN */


#define HSTACK_LINEAR_SCORE_MOV_LINEAR    1 /* Move a location to make it linear */
#define HSTACK_LINEAR_SCORE_MOV_UNRELATED 2 /* Move an unrelated location out of the way */

/* Calculate a score describing the complexity of shifting memory
 * in order to construct a linear vector of `linbase...+=linsize' at `cfa_offset'
 * @param: hstack_inuse:    Cache of currently in-use hstack location (see `LOCAL_hstack_cfa_*' macros)
 * @param: hstack_reserved: Cache of currently reserved hstack location (s.a. `MEMOBJ_F_LINEAR') */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) ATTR_INS(4, 5) size_t DCALL
hstack_linear_score(bitset_t const *__restrict hstack_inuse,
                    bitset_t const *__restrict hstack_reserved,
                    Dee_cfa_t hstack_cfa_offset,
                    struct Dee_memval const *linbase,
                    Dee_vstackaddr_t linsize,
                    Dee_cfa_t cfa_offset) {
	Dee_vstackaddr_t i;
	size_t result = 0;
	for (i = 0; i < linsize; ++i) {
		struct Dee_memval const *src = &linbase[i];
#ifdef HOSTASM_STACK_GROWS_DOWN
		Dee_cfa_t dst_cfa_offset = cfa_offset - i * HOST_SIZEOF_POINTER;
#else /* HOSTASM_STACK_GROWS_DOWN */
		Dee_cfa_t dst_cfa_offset = cfa_offset + i * HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		if (Dee_memloc_gettyp(Dee_memval_direct_getloc(src)) == MEMADR_TYPE_HSTACKIND &&
		    Dee_memloc_hstackind_getcfa(Dee_memval_direct_getloc(src)) == dst_cfa_offset)
			continue; /* Already at the perfect location! */
		/* TODO: Check if "src" has an equivalence alias at the correct HSTACKIND location. */
#ifdef HOSTASM_STACK_GROWS_DOWN
		if (dst_cfa_offset <= hstack_cfa_offset)
#else /* HOSTASM_STACK_GROWS_DOWN */
		if (dst_cfa_offset < hstack_cfa_offset)
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		{
			/* If there's something there already, it must be moved. */
			if (LOCAL_hstack_cfa_inuse(hstack_inuse, dst_cfa_offset))
				result += HSTACK_LINEAR_SCORE_MOV_UNRELATED;
			if (LOCAL_hstack_cfa_inuse(hstack_reserved, dst_cfa_offset))
				return (size_t)-1; /* This would conflict with a reserved location */
		}
		result += HSTACK_LINEAR_SCORE_MOV_LINEAR;
	}
	return result;
}

/* Check if any of the given locations are HSTACKIND */
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) bool DCALL
Dee_memvals_anyhstackind(struct Dee_memval const *__restrict base,
                         Dee_vstackaddr_t count) {
	Dee_vstackaddr_t i;
	for (i = 0; i < count; ++i) {
		if (Dee_memval_direct_gettyp(&base[i]) == MEMADR_TYPE_HSTACKIND)
			return true;
	}
	return false;
}

/* Arrange the top `argc' stack-items linearly, such that they all appear somewhere in memory
 * (probably on the host-stack), in consecutive order (with `vtop' at the greatest address,
 * and STACK[SIZE-argc] appearing at the lowest address). Once that has been accomplished,
 * push a value onto the vstack that describes the base-address (that is a `DeeObject **'
 * pointing to `STACK[SIZE-argc]') of the linear vector.
 * @param: readonly: Special case to allow the `DeeObject **' vector being generated
 *                   as `DeeObject *const *'. This in turn makes it possible to not
 *                   have to construct argument vectors on-stack when all arguments
 *                   are (re-)compile-time constants.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vlinear(struct Dee_function_generator *__restrict self,
                               Dee_vstackaddr_t argc, bool readonly) {
	DO(Dee_function_generator_vdirect(self, argc));
	DO(Dee_function_generator_state_unshare(self));
	if unlikely(!argc) {
		/* The base address of an empty vector doesn't matter, meaning it's undefined */
		return Dee_function_generator_vpush_undefined(self);
	} else if (readonly && Dee_function_generator_vallconst_noref(self, argc)) {
		/* Dynamically allocate a dummy object which includes space
		 * for "argc" pointers. Then, fill those pointers with values
		 * from the v-stack, inline the reference to dummy object, and
		 * finally: push a pointer to the base address of the dummy's
		 * value array. */
		Dee_vstackaddr_t i;
		struct Dee_memval *cbase;
		DREF DummyVectorObject *vec = DummyVector_New(argc);
		if unlikely(!vec)
			goto err;
		vec = (DREF DummyVectorObject *)Dee_function_generator_inlineref(self, (DREF DeeObject *)vec);
		if unlikely(!vec)
			goto err;
		cbase = self->fg_state->ms_stackv + self->fg_state->ms_stackc - argc;
		for (i = 0; i < argc; ++i)
			vec->dvo_items[i] = Dee_memval_const_getaddr(&cbase[i]);
		return Dee_function_generator_vpush_addr(self, vec->dvo_items);
	} else if (argc == 1) {
		/* Deal with simple case: caller only wants the address of a single location.
		 * In this case, we only need to make sure that said location resides in
		 * memory (which is even allowed to be a REGIND location), and then push
		 * the address of that location. */
		Dee_cfa_t cfa_offset;
		struct Dee_memloc *loc = Dee_function_generator_vtopdloc(self);
		if (Dee_memloc_gettyp(loc) == MEMADR_TYPE_HREGIND &&
		    Dee_memloc_hregind_getvaloff(loc) == 0) {
			/* Special case: address of `*(%reg + off) + 0' is `%reg + off' */
			return Dee_function_generator_vpush_hreg(self,
			                                        Dee_memloc_hregind_getreg(loc),
			                                        Dee_memloc_hregind_getindoff(loc));
		}

		/* Flush value to the stack to give is an addressable memory location. */
		DO(Dee_function_generator_vflush(self, true));
		loc = Dee_function_generator_vtopdloc(self);
		ASSERT(Dee_memloc_gettyp(loc) == MEMADR_TYPE_HSTACKIND);
		ASSERT(Dee_memloc_hstackind_getvaloff(loc) == 0);
		cfa_offset = Dee_memloc_hstackind_getcfa(loc);
		return Dee_function_generator_vpush_hstack(self, cfa_offset);
	} else {
		/* General case: figure out the optimal CFA base address of the linear vector. */
		Dee_vstackaddr_t i;
		Dee_cfa_t result_cfa_offset;
		DREF struct Dee_memstate *linear_state;
		struct Dee_memstate *state = self->fg_state;
		struct Dee_memval *linbase = state->ms_stackv + state->ms_stackc - argc;

		/* Check for special case: if none of the linear locations are HSTACKIND,
		 * then the optimal target location is always either a sufficiently large
		 * free region, or new newly alloca'd region. */
		if (!Dee_memvals_anyhstackind(linbase, argc)) {
			size_t num_bytes = argc * sizeof(void *);
			result_cfa_offset = Dee_memstate_hstack_find(state, self->fg_state_hstack_res, num_bytes);
			if (result_cfa_offset == (Dee_cfa_t)-1) {
				Dee_cfa_t saved_cfa = state->ms_host_cfa_offset;
				Dee_memstate_hstack_free(state);
				result_cfa_offset = state->ms_host_cfa_offset;
#ifdef HOSTASM_STACK_GROWS_DOWN
				result_cfa_offset += num_bytes;
#endif /* HOSTASM_STACK_GROWS_DOWN */
				state->ms_host_cfa_offset = saved_cfa;
			}
		} else {
			/* Fallback: Assign scores to all possible CFA offsets for the linear vector.
			 *           Then, choose whatever CFA offset has the lowest score. */
			size_t result_cfa_offset_score;
			Dee_cfa_t cfa_offset_max;
			Dee_cfa_t cfa_offset_min;
			Dee_cfa_t cfa_offset;
			struct Dee_memval *mval;
			bitset_t *hstack_inuse;    /* Bitset for currently in-use hstack locations (excluding locations used by linear slots) */
			bitset_t *hstack_reserved; /* Bitset of hstack locations that can never be used (because they belong to `MEMOBJ_F_LINEAR' items) */
			size_t hstack_inuse_sizeof;
			hstack_inuse_sizeof = _bitset_sizeof((state->ms_host_cfa_offset / HOST_SIZEOF_POINTER) * 2);
			hstack_inuse = (bitset_t *)Dee_Calloca(hstack_inuse_sizeof * 2);
			if unlikely(!hstack_inuse)
				goto err;
			hstack_reserved = hstack_inuse + hstack_inuse_sizeof;
			Dee_memstate_foreach(mval, state) {
				struct Dee_memobj *mobj;
				Dee_memval_foreach_obj(mobj, mval) {
					if (Dee_memobj_gettyp(mobj) == MEMADR_TYPE_HSTACKIND) {
						Dee_cfa_t cfa = Dee_memobj_getcfastart(mobj);
						ASSERT(cfa < state->ms_host_cfa_offset);
						bitset_set(hstack_inuse, cfa / HOST_SIZEOF_POINTER);
						if (mobj->mo_flags & MEMOBJ_F_LINEAR)
							bitset_set(hstack_reserved, cfa / HOST_SIZEOF_POINTER);
					}
				}
				Dee_memval_foreach_obj_end;
			}
			Dee_memstate_foreach_end;
			/* hstack locations currently in use by the linear portion don't count as in-use.
			 * NOTE: We do this in a second pass, so we also hit all of the aliases. */
			for (i = 0; i < argc; ++i) {
				ASSERT(Dee_memval_isdirect(&linbase[i]));
				if (Dee_memval_direct_gettyp(&linbase[i]) == MEMADR_TYPE_HSTACKIND) {
					Dee_cfa_t cfa = Dee_memloc_getcfastart(Dee_memval_direct_getloc(&linbase[i]));
					ASSERT(cfa < state->ms_host_cfa_offset);
					bitset_clear(hstack_inuse, cfa / HOST_SIZEOF_POINTER);
				}
			}
#ifdef HOSTASM_STACK_GROWS_DOWN
			cfa_offset_min = argc * HOST_SIZEOF_POINTER;
			cfa_offset_max = state->ms_host_cfa_offset + cfa_offset_min;
#else /* HOSTASM_STACK_GROWS_DOWN */
			cfa_offset_min = 0;
			cfa_offset_max = state->ms_host_cfa_offset;
#endif /* !HOSTASM_STACK_GROWS_DOWN */

			/* Enumerate candidates starting with low CFA offsets.
			 * That way, we prefer equally complex candidates that are closer to the frame base. */
			result_cfa_offset = cfa_offset_min;
			result_cfa_offset_score = hstack_linear_score(hstack_inuse, hstack_reserved,
			                                              state->ms_host_cfa_offset,
			                                              linbase, argc, result_cfa_offset);
			if likely(result_cfa_offset_score != 0) {
				for (cfa_offset = cfa_offset_min + HOST_SIZEOF_POINTER;
				     cfa_offset <= cfa_offset_max; cfa_offset += HOST_SIZEOF_POINTER) {
					size_t score = hstack_linear_score(hstack_inuse, hstack_reserved,
					                                   state->ms_host_cfa_offset,
					                                   linbase, argc, cfa_offset);
					if (result_cfa_offset_score > score) {
						result_cfa_offset_score = score;
						result_cfa_offset = cfa_offset;
						if (score == 0)
							break;
					}
				}
			}
			ASSERTF(result_cfa_offset_score != (size_t)-1,
			        "Not possible! The combination where we'd be pushing everything into a "
			        "freshly allocated portion of the hstack shouldn't have hit any reserved "
			        "locations!");
			Dee_Freea(hstack_inuse);
			if unlikely(result_cfa_offset_score == 0) {
				/* Special case: the score only becomes 0 when no morph is needed.
				 * This means that everything is already in place such that a linear
				 * vector is formed at `result_cfa_offset'! */
				return Dee_function_generator_vpush_hstack(self, result_cfa_offset);
			}

		}

		/* Construct a memstate that puts the linear items along `result_cfa_offset' */
		linear_state = Dee_memstate_copy(state);
		if unlikely(!linear_state)
			goto err;

		/* Collect all locations that are aliases to those that should become linear.
		 * Then, assign intended target locations to aliases as far as possible. */
		linbase = linear_state->ms_stackv + linear_state->ms_stackc - argc;

		/* Gather aliases */
#define TYPE_LINLOC MEMEQUIV_TYPE_UNUSED
#define TYPE_ALIAS  MEMEQUIV_TYPE_DUMMY
		for (i = 0; i < argc; ++i) {
			struct Dee_memval *aliasval, *mval = &linbase[i];
			struct Dee_memloc locval = mval->mv_obj.mvo_0.mo_loc;
			mval->mv_obj.mvo_0.mo_loc.ml_adr.ma_typ = TYPE_LINLOC;
			mval->mv_obj.mvo_0.mo_loc.ml_adr.ma_val._v_nextobj = NULL;
			Dee_memstate_foreach(aliasval, linear_state) {
				struct Dee_memobj *alias;
				Dee_memval_foreach_obj(alias, aliasval) {
					if (alias->mo_loc.ml_adr.ma_typ == TYPE_LINLOC ||
					    alias->mo_loc.ml_adr.ma_typ == TYPE_ALIAS)
						continue;
					if (Dee_memloc_sameloc(&alias->mo_loc, &locval)) {
						alias->mo_loc.ml_adr.ma_typ = TYPE_ALIAS;
						alias->mo_loc.ml_adr.ma_val._v_nextobj = mval->mv_obj.mvo_0.mo_loc.ml_adr.ma_val._v_nextobj;
						mval->mv_obj.mvo_0.mo_loc.ml_adr.ma_val._v_nextobj = alias;
					}
				}
				Dee_memval_foreach_obj_end;
			}
			Dee_memstate_foreach_end;
		}
		for (i = 0; i < argc; ++i) {
			struct Dee_memobj *loc = &linbase[i].mv_obj.mvo_0;
			if (loc->mo_loc.ml_adr.ma_typ == TYPE_LINLOC) {
				struct Dee_memobj *next;
#ifdef HOSTASM_STACK_GROWS_DOWN
				Dee_cfa_t dst_cfa_offset = result_cfa_offset - i * HOST_SIZEOF_POINTER;
#else /* HOSTASM_STACK_GROWS_DOWN */
				Dee_cfa_t dst_cfa_offset = result_cfa_offset + i * HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
				do {
					next = loc->mo_loc.ml_adr.ma_val._v_nextobj;
					Dee_memloc_init_hstackind(&loc->mo_loc, dst_cfa_offset, 0);
				} while ((loc = next) != NULL);
			}
		}
#undef TYPE_LINLOC
#undef TYPE_ALIAS

		/* TODO: Move already-present, but unrelated locations out-of-the way. */

		/* Fix locations where linear elements were aliasing each other. */
		for (i = 0; i < argc; ++i) {
#ifdef HOSTASM_STACK_GROWS_DOWN
			Dee_cfa_t dst_cfa_offset = result_cfa_offset - i * HOST_SIZEOF_POINTER;
#else /* HOSTASM_STACK_GROWS_DOWN */
			Dee_cfa_t dst_cfa_offset = result_cfa_offset + i * HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			struct Dee_memval *mval = &linbase[i];
			ASSERT(mval->mv_obj.mvo_0.mo_loc.ml_adr.ma_typ == MEMADR_TYPE_HSTACKIND);
			mval->mv_obj.mvo_0.mo_loc.ml_adr.ma_val.v_cfa = dst_cfa_offset;
			mval->mv_obj.mvo_0.mo_flags |= MEMOBJ_F_LINEAR; /* Not allowed to move until popped */
		}

		/* Make sure that `linear_state's CFA offset is large enough to hold the linear vector. */
		{
#ifdef HOSTASM_STACK_GROWS_DOWN
			Dee_cfa_t req_min_host_cfa = result_cfa_offset;
#else /* HOSTASM_STACK_GROWS_DOWN */
			Dee_cfa_t req_min_host_cfa = result_cfa_offset + argc * HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			if (linear_state->ms_host_cfa_offset < req_min_host_cfa)
				linear_state->ms_host_cfa_offset = req_min_host_cfa;
		}

		/* Generate code to morph the current memory state to that of `linear_state'. */
		{
			int temp = Dee_function_generator_vmorph(self, linear_state);
			Dee_memstate_decref(linear_state);
			if likely(temp == 0)
				temp = Dee_function_generator_vpush_hstack(self, result_cfa_offset);
			return temp;
		}
		__builtin_unreachable();
	}
	__builtin_unreachable();
err:
	return -1;
}

#undef LOCAL_hstack_lowcfa_inuse
#undef LOCAL_hstack_cfa_inuse


/* Pre-defined exception injectors. */

/* `fei_inject' value for `struct Dee_function_exceptinject_callvoidapi' */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_exceptinject_callvoidapi_f(struct Dee_function_generator *__restrict self,
                                        struct Dee_function_exceptinject *__restrict inject) {
	struct Dee_function_exceptinject_callvoidapi *me;
	me = (struct Dee_function_exceptinject_callvoidapi *)inject;
	return Dee_function_generator_vcallapi(self, me->fei_cva_func, VCALL_CC_VOID_NX, me->fei_cva_argc);
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C */
