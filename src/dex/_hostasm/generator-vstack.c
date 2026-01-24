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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C
#define GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/align.h>    /* CEILDIV */
#include <hybrid/bitset.h>   /* BITSET_SIZEOF, bitset_* */
#include <hybrid/compiler.h>

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, ptrdiff_t, size_t */
#include <stdint.h>  /* intptr_t, uint8_t, uint16_t, uintptr_t */

DECL_BEGIN

#define DeeInt_NEWSFUNC(n) DEE_PRIVATE_NEWINT(n)
#define DeeInt_NEWUFUNC(n) DEE_PRIVATE_NEWUINT(n)

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

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
fg_vswap(struct fungen *__restrict self) {
	int result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vswap(self->fg_state);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vlrot(struct fungen *__restrict self, vstackaddr_t n) {
	int result;
	if unlikely(n <= 1)
		return 0;
	result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vlrot(self->fg_state, n);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vrrot(struct fungen *__restrict self, vstackaddr_t n) {
	int result;
	if unlikely(n <= 1)
		return 0;
	result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vrrot(self->fg_state, n);
	return result;
}

/* a,b,c,d -> d,c,b,a */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vmirror(struct fungen *__restrict self, vstackaddr_t n) {
	int result;
	if unlikely(n <= 1)
		return 0;
	result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vmirror(self->fg_state, n);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vpush_memadr(struct fungen *__restrict self,
                struct memadr const *adr) {
	int result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vpush_memadr(self->fg_state, adr);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vpush_memloc(struct fungen *__restrict self,
                struct memloc const *loc) {
	int result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vpush_memloc(self->fg_state, loc);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vpush_memobj(struct fungen *__restrict self,
                struct memobj const *obj) {
	int result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vpush_memobj(self->fg_state, obj);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_undefined(struct fungen *__restrict self) {
	int result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vpush_undefined(self->fg_state);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_addr(struct fungen *__restrict self,
              void const *addr) {
	int result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vpush_addr(self->fg_state, addr);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vpush_const_(struct fungen *__restrict self,
                DeeObject *value) {
	int result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vpush_const(self->fg_state, value);
	return result;
}

INTERN WUNUSED NONNULL((1)) DeeObject *DCALL
fg_getconst(struct fungen *__restrict self, uint16_t cid) {
	DeeObject *result;
	DeeCodeObject *code;
	if unlikely(cid >= self->fg_assembler->fa_code->co_constc) {
		err_illegal_cid(cid);
		goto err;
	}
	code   = self->fg_assembler->fa_code;
	result = code->co_constv[cid];
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_cid(struct fungen *__restrict self, uint16_t cid) {
	DeeObject *constant;
	constant = fg_getconst(self, cid);
	if unlikely(!constant)
		goto err;
	return fg_vpush_const(self, constant);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
fg_vpush_cid_t(struct fungen *__restrict self,
               uint16_t cid, DeeTypeObject *__restrict type) {
	DeeObject *constant;
	constant = fg_getconst(self, cid);
	if unlikely(!constant)
		goto err;
	DO(DeeObject_AssertTypeExact(constant, type));
	return fg_vpush_const(self, constant);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_rid(struct fungen *__restrict self, uint16_t rid) {
	DeeObject *constant;
	if unlikely(rid >= self->fg_assembler->fa_code->co_refc)
		return err_illegal_rid(rid);
	if (self->fg_assembler->fa_cc & HOST_CC_F_FUNC) {
		/* Special case: must access the "fo_refv" field of the runtime "func" parameter. */
		DO(_fg_vpush_xlocal(self, NULL, MEMSTATE_XLOCAL_A_FUNC));
		return fg_vind(self,
		               _Dee_MallococBufsize(offsetof(DeeFunctionObject, fo_refv),
		                                    rid, sizeof(DREF DeeObject *)));
	}

	/* Simple case: able to directly inline function references */
	constant = self->fg_assembler->fa_function->fo_refv[rid];
	return fg_vpush_const(self, constant);
err:
	return -1;
}


/* Sets the `MEMOBJ_F_NOREF' flag */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_hreg(struct fungen *__restrict self,
              host_regno_t regno, ptrdiff_t val_delta) {
	int result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vpush_hreg(self->fg_state, regno, val_delta);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_hregind(struct fungen *__restrict self,
                 host_regno_t regno, ptrdiff_t ind_delta,
                 ptrdiff_t val_delta) {
	int result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vpush_hregind(self->fg_state, regno, ind_delta, val_delta);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_hstack(struct fungen *__restrict self,
                host_cfa_t cfa_offset) {
	int result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vpush_hstack(self->fg_state, cfa_offset);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_hstackind(struct fungen *__restrict self,
                   host_cfa_t cfa_offset, ptrdiff_t val_delta) {
	int result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vpush_hstackind(self->fg_state, cfa_offset, val_delta);
	return result;
}


/* Force "loc" to become HREG, assuming that "loc" is tracked by the mem-state. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
tracked_memloc_forcereg(struct fungen *__restrict self,
                        struct memloc *loc, host_regno_t const *not_these) {
	int result;
	struct memloc retloc;
	result = fg_gasreg(self, loc, &retloc, not_these);
	if likely(result == 0) {
		memstate_changeloc(self->fg_state, loc, &retloc);
		ASSERTF(memloc_sameloc(loc, &retloc), "This should have gotten updated!");
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_arg(struct fungen *__restrict self,
             Dee_instruction_t const *instr, aid_t aid) {
	DeeCodeObject *code = self->fg_assembler->fa_code;
	if (aid < code->co_argc_min) /* Simple case: mandatory argument */
		return fg_vpush_arg_present(self, aid);
	if (aid < code->co_argc_max) /* Special case: optional argument (push the x-local) */
		return fg_vpush_xlocal(self, instr, MEMSTATE_XLOCAL_DEFARG(aid - code->co_argc_min));
	return err_illegal_aid(aid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vbound_arg(struct fungen *__restrict self, aid_t aid) {
	DeeCodeObject *code = self->fg_assembler->fa_code;
	if (aid < code->co_argc_min) /* Simple case: mandatory argument */
		return fg_vpush_const(self, Dee_True);
	if (aid < code->co_argc_max) {
		/* Special case: optional argument
		 * NOTE: The normal code executor doesn't look at default values,
		 *       but we are forced to, since cached optional arguments are
		 *       shared with keyword arguments, making it impossible to
		 *       differentiate between a caller-given argument and a cached
		 *       argument with its default value already assigned. */
		struct memval *mval;
		aid_t opt_aid = aid - code->co_argc_min;
		lid_t xlid = MEMSTATE_XLOCAL_DEFARG(opt_aid);
		lid_t lid = self->fg_assembler->fa_localc + xlid;
		DeeObject *default_value = code->co_defaultv[opt_aid];
		if (default_value != NULL)
			return fg_vpush_const(self, Dee_True);
		mval = &self->fg_state->ms_localv[lid];
		if (!memval_isdirect(mval)) /* Non-direct values are *always* bound (logically speaking) */
			return fg_vpush_const(self, Dee_True);
		if (memval_direct_local_neverbound(mval))
			return fg_vpush_const(self, Dee_False);

		/* Check if the argument location has already been allocated. */
		if (!memval_direct_isundefined(mval)) {
			if (memval_direct_local_alwaysbound(mval))
				return fg_vpush_const(self, Dee_True);
			DO(fg_vpush_memobj(self, memval_direct_getobj(mval)));
			DO(fg_state_unshare(self));
			mval = fg_vtop(self);
			ASSERT(memval_isdirect(mval));
			mval->mv_vmorph = MEMVAL_VMORPH_TESTNZ(mval->mv_vmorph);
			return 0;
		}

		/* Argument hasn't been accessed or populated by the keyword loader.
		 * -> Simply check if the argument is present positionally:
		 *         argc >= aid
		 *    <=>  argc > (aid - 1)
		 *    <=>  argc - (aid - 1) > 0
		 */
		DO(fg_vpush_argc(self));
		DO(fg_vdelta(self, -((ptrdiff_t)aid - 1)));
		mval = fg_vtop(self);
		ASSERT(memval_isdirect(mval));
		mval->mv_vmorph = MEMVAL_VMORPH_BOOL_GZ;
		return 0;
	}
	return err_illegal_aid(aid);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_arg_present(struct fungen *__restrict self, aid_t aid) {
	int result;
	ASSERT(aid < self->fg_assembler->fa_code->co_argc_min);
	result = fg_state_unshare(self);
	if likely(result == 0) {
		STATIC_ASSERT(MEMSTATE_XLOCAL_A_ARGS == MEMSTATE_XLOCAL_A_ARGV);
		struct memval *args_or_argv_val;
		struct memloc *args_or_argv_loc;
		ptrdiff_t ind_offset = (ptrdiff_t)aid * sizeof(DeeObject *);
		if (self->fg_assembler->fa_cc & HOST_CC_F_TUPLE)
			ind_offset += offsetof(DeeTupleObject, t_elem);
		args_or_argv_val = &self->fg_state->ms_localv[self->fg_assembler->fa_localc + MEMSTATE_XLOCAL_A_ARGV];
		if (!memval_isdirect(args_or_argv_val)) {
			DO(fg_vdirect_memval(self, args_or_argv_val));
			args_or_argv_val = &self->fg_state->ms_localv[self->fg_assembler->fa_localc + MEMSTATE_XLOCAL_A_ARGV];
		}
		args_or_argv_loc = memval_direct_getloc(args_or_argv_val);
		DO(tracked_memloc_forcereg(self, args_or_argv_loc, NULL));
		ASSERT(memloc_gettyp(args_or_argv_loc) == MEMADR_TYPE_HREG);
		result = memstate_vpush_hregind(self->fg_state,
		                                    memloc_hreg_getreg(args_or_argv_loc),
		                                    memloc_hreg_getvaloff(args_or_argv_loc) + ind_offset,
		                                    0);
	}
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
delete_unused_local_after_read(struct fungen *__restrict self,
                               Dee_instruction_t const *instr, lid_t lid) {
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
			lid_t delete_lid = self->fg_nextlastloc->bbl_lid;
			DO(fg_vdel_local(self, delete_lid));
			++self->fg_nextlastloc;
		}
		ASSERT(self->fg_nextlastloc->bbl_instr >= instr);
		if (self->fg_nextlastloc->bbl_instr == instr) {
			struct bb_loclastread *item = self->fg_nextlastloc;
			while (item->bbl_instr == instr && item->bbl_lid != lid)
				++item;
			if (item->bbl_instr == instr) {
				ASSERT(item->bbl_lid == lid);
				if (item != self->fg_nextlastloc) {
					struct bb_loclastread temp;
					temp = *item;
					memmoveupc(self->fg_nextlastloc + 1,
					           self->fg_nextlastloc,
					           (size_t)(item - self->fg_nextlastloc),
					           sizeof(struct bb_loclastread));
					item = self->fg_nextlastloc;
					*item = temp;
				}
				ASSERT(item == self->fg_nextlastloc);
				ASSERT(item->bbl_instr == instr);
				ASSERT(item->bbl_lid == lid);
				DO(fg_vdel_local(self, lid));
				++self->fg_nextlastloc;
			}
		}
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_local(struct fungen *__restrict self,
               Dee_instruction_t const *instr, lid_t lid) {
	struct memstate *state;
	struct memval *dst, *src;
	DO(fg_state_unshare(self));
	state = self->fg_state;
	ASSERT(lid < state->ms_localc);
	if unlikely(state->ms_stackc >= state->ms_stacka)
		DO(memstate_reqvstack(state, state->ms_stackc + 1));
	src = &state->ms_localv[lid];
	if (instr && memval_isdirect(src) && !memval_direct_local_alwaysbound(src)) {
		if (memval_direct_local_neverbound(src)) {
			/* Variable is always unbound -> generate code to throw an exception */
			DO(fg_gthrow_local_unbound(self, instr, (ulid_t)lid));
			return fg_vpush_none(self);
		}

		/* Variable is not guarantied bound -> generate code to check if it's bound */
		DO(fg_gassert_local_bound(self, instr, (ulid_t)lid));

		/* After a bound assertion, the local variable is guarantied to be bound. */
		memval_direct_local_setbound(src);
	}
	dst = &state->ms_stackv[state->ms_stackc];
	memval_initcopy(dst, src);
	memval_clearref(dst);
	memstate_incrinuse_for_memval(state, dst);
	++state->ms_stackc;
	return delete_unused_local_after_read(self, instr, lid);
err:
	return -1;
}

/* `instr' is needed for automatic deletion of unused locals */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vbound_local(struct fungen *__restrict self,
                Dee_instruction_t const *instr, lid_t lid) {
	struct memstate *state;
	struct memval *dst, *src;
	DO(fg_state_unshare(self));
	state = self->fg_state;
	ASSERT(lid < state->ms_localc);
	if unlikely(state->ms_stackc >= state->ms_stacka)
		DO(memstate_reqvstack(state, state->ms_stackc + 1));
	src = &state->ms_localv[lid];
	if (!memval_isdirect(src)) /* Non-direct values are *always* bound (logically speaking) */
		return fg_vpush_const(self, Dee_True);
	if (memval_direct_local_alwaysbound(src))
		return fg_vpush_const(self, Dee_True);
	if (memval_direct_local_neverbound(src))
		return fg_vpush_const(self, Dee_False);
	dst = &state->ms_stackv[state->ms_stackc];
	memval_direct_initcopy(dst, src);
	memstate_incrinuse_for_direct_memval(state, src);
	++state->ms_stackc;
	DO(delete_unused_local_after_read(self, instr, lid));
	ASSERT(memval_isdirect(dst));
	dst->mv_vmorph = MEMVAL_VMORPH_TESTNZ(dst->mv_vmorph);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vdup_at(struct fungen *__restrict self, vstackaddr_t n) {
	int result = fg_state_unshare(self);
	if likely(result == 0)
		result = memstate_vdup_at(self->fg_state, n);
	return result;
}

/* Ensure that "mobj" (or one of its aliases) is holding a reference. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ensure_tracked_memobj_holds_reference(struct fungen *__restrict self,
                                      struct memobj *mobj) {
	struct memstate *state = self->fg_state;
	if (memstate_hasref(state, mobj))
		return 0; /* Already a reference! -> all is fine! */

	/* Check for special case: if the dependent object is a register indirection,
	 * then that means there it still points into the object that may be about
	 * to be destroyed. If that is the case, just change it to a register first. */
	if (memobj_gettyp(mobj) == MEMADR_TYPE_HREGIND) {
		struct memloc mobj_asreg;
		DO(fg_gasreg(self, memobj_getloc(mobj), &mobj_asreg, NULL));
		memstate_changeloc(state, memobj_getloc(mobj), &mobj_asreg);
		ASSERT(memloc_sameloc(memobj_getloc(mobj), &mobj_asreg));
	}

	/* Generate the reference *now* */
	DO(fg_gincref_loc(self, memobj_getloc(mobj), 1));
	ASSERT(!memobj_isref(mobj));
	memobj_setref(mobj);
	return 0;
err:
	return -1;
}

/* Ensure that objects that depend on "dependencies_of_this" are holding references. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ensure_dependent_objects_hold_references(struct fungen *__restrict self,
                                         struct memloc *dependencies_of_this) {
	struct memval *dep_mval;
	struct memstate *state = self->fg_state;
	memstate_foreach(dep_mval, state) {
		struct memobj *dep_mobj;
		memval_foreach_obj(dep_mobj, dep_mval) {
			if (memobj_hasxinfo(dep_mobj)) {
				struct memobj_xinfo *xinfo;
				xinfo = memobj_getxinfo(dep_mobj);
				if (memloc_sameloc(&xinfo->mox_dep, dependencies_of_this)) {
					/* This object depends on us -> make sure it's a reference. */
					DO(ensure_tracked_memobj_holds_reference(self, dep_mobj));

					/* With the reference created, the object no longer needs us! */
					bzero(&xinfo->mox_dep, sizeof(xinfo->mox_dep));
				}
			}
		}
		memval_foreach_obj_end;
	}
	memstate_foreach_end;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
memstate_hasconstval(struct memstate const *__restrict self,
                     struct memobj const *mobj) {
	struct memequiv *eq;
	if (memobj_isconst(mobj))
		return true;
	eq = memequivs_getclassof(&self->ms_memequiv, memobj_getadr(mobj));
	if (eq) {
		struct memequiv *iter = eq;
		do {
			if (memloc_isconst(&iter->meq_loc))
				return true;
		} while ((iter = memequiv_next(iter)) != eq);
	}
	return false;
}

/* Generate code needed to drop references held by `mval' (where `mval' must be a vstack item,
 * or a local variable that is unconditionally bound or non-direct).
 * NOTE: This function is somewhere between the v* and g* APIs, though it does *NOT* unshare or
 *       realloc memstate components. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vgdecref_vstack(struct fungen *__restrict self,
                   struct memval *mval) {
	struct memstate *state = self->fg_state;
	struct memobj *mobj;
	if unlikely(mval->mv_flags & MEMVAL_F_NOREF) {
		ASSERT(!memval_hasobj0(mval));
		return 0;
	}
	memval_foreach_obj(mobj, mval) {
		if (memobj_isref(mobj)) {
			struct memval *other_mval;
			bool has_ref_alias = false;
			int temp;
	
			/* Try and shift the burden of the reference to the other location. */
			memstate_foreach(other_mval, state) {
				struct memobj *other_mobj;
				if unlikely(other_mval->mv_flags & MEMVAL_F_NOREF) {
					ASSERT(!memval_hasobj0(other_mval));
					continue;
				}
				_memval_foreach_obj(other, other_mobj, other_mval) {
					if (memobj_sameloc(other_mobj, mobj) && other_mobj != mobj) {
						if (memobj_isref(other_mobj)) {
							has_ref_alias = true;
						} else {
							memobj_setref(other_mobj);
							goto done;
						}
					}
				}
				memval_foreach_obj_end;
			}
			memstate_foreach_end;
			if (!has_ref_alias && memstate_hasconstval(state, mobj))
				has_ref_alias = true; /* Constants are always aliased by static storage */
	
			/* No-where to shift the reference to -> must decref the object ourselves. */
			if (has_ref_alias) {
				temp = fg_gdecref_nokill_loc(self, memobj_getloc(mobj), 1);
			} else {
				/* If the object acts as a dependency, then we must make sure that
				 * every distinct dependent object is holding at least 1 reference! */
				if (mobj->mo_flags & MEMOBJ_F_HASDEP)
					DO(ensure_dependent_objects_hold_references(self, memobj_getloc(mobj)));

				/* XXX: If types are known, inline DeeObject_Destroy() as tp_fini() + DeeType_FreeInstance() */
				if (memobj_isoneref(mobj)) {
					temp = fg_gdecref_dokill_loc(self, memobj_getloc(mobj));
				} else {
					temp = fg_gdecref_loc(self, memobj_getloc(mobj), 1);
				}
			}
			if unlikely(temp)
				goto err;
		}
	}
	memval_foreach_obj_end;
done:
	return 0;
err:
	return -1;
}

/* Generate code needed to drop references held by `mval' (where `mval' must point into locals)
 * NOTE: This function is somewhere between the v* and g* APIs, though it does *NOT* unshare or
 *       realloc memstate components. */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vgdecref_local(struct fungen *__restrict self,
                  struct memval *__restrict mval) {
	struct memstate *state = self->fg_state;
	if (!memval_isdirect(mval)) {
decref_mval:
		return fg_vgdecref_vstack(self, mval);
	} else if (!memval_direct_isref(mval)) {
		/* Nothing to do here! */
	} else if (memval_direct_local_neverbound(mval)) {
		/* Nothing to do here! */
	} else if (memval_direct_local_alwaysbound(mval)) {
		goto decref_mval;
	} else {
		/* Location is conditionally bound.
		 * Check if there is another conditionally bound
		 * location which we can off-load the decref onto. */
		int temp;
		lid_t i;
		bool has_ref_alias = false;
		struct memval *other_mval;
		for (i = 0; i < state->ms_localc; ++i) {
			other_mval = &state->ms_localv[i];
			if (other_mval == mval)
				continue;
			if (!memval_isdirect(other_mval))
				continue;
			if (memval_direct_local_alwaysbound(other_mval))
				continue; /* Need a conditionally bound alias */
			if (!memval_direct_sameloc(mval, other_mval))
				continue;
			if (!memval_direct_isref(other_mval)) {
				/* Off-load reference to here! */
				memval_direct_clearref(mval);
				memval_direct_setref(other_mval);
				return 0;
			}
			has_ref_alias = true;
		}
		if (has_ref_alias) {
			temp = fg_gxdecref_nokill_loc(self, memval_direct_getloc(mval), 1);
		} else {
			temp = fg_gxdecref_loc(self, memval_direct_getloc(mval), 1);
		}
		return temp;
	}
	return 0;
}


#if 0
/* Wrapper around:
 * - fg_vgdecref_vstack
 * - fg_vgdecref_local
 * ... that automatically checks if `mval' points into the current mem-state's
 * local variable list to see which function needs to be used. */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vgdecref(struct fungen *__restrict self,
            struct memval *__restrict mval) {
	struct memstate *state = self->fg_state;
	bool is_local = mval >= state->ms_localv &&
	                mval < state->ms_localv + state->ms_localc;
	return is_local ? fg_vgdecref_local(self, mval)
	                : fg_vgdecref_vstack(self, mval);
}
#endif


INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpop(struct fungen *__restrict self) {
	struct memstate *state;
	struct memval *mval;
	DO(fg_state_unshare(self));
	state = self->fg_state;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = memstate_vtop(state);
	if (mval->mv_vmorph == MEMVAL_VMORPH_NULLABLE) {
		/* Still need to do the null-check (because an exception may have been thrown),
		 * unless the underlying location is being aliased somewhere else. */
		uint8_t saved_mo_flags;
		struct memval *alias_mval;
		memstate_foreach(alias_mval, state) {
			if (alias_mval->mv_vmorph == MEMVAL_VMORPH_NULLABLE &&
			    memobj_sameloc(memval_nullable_getobj(alias_mval),
			                       memval_nullable_getobj(mval)) &&
			    alias_mval != mval) {
				/* Found an alias! */
				memval_nullable_makedirect(mval);
#ifndef __OPTIMIZE_SIZE__ /* Shift a held reference as well (if possible) */
				if (memval_direct_isref(mval) &&
				    !memobj_isref(memval_nullable_getobj(alias_mval))) {
					memobj_clearref(memval_direct_getobj(mval));
					memobj_setref(memval_nullable_getobj(alias_mval));
				}
#endif /* !__OPTIMIZE_SIZE__ */
				goto do_maybe_decref;
			}
		}
		memstate_foreach_end;

		saved_mo_flags = memval_nullable_getobj(mval)->mo_flags;
		memobj_clearref(memval_nullable_getobj(mval));
		DO(fg_gjz_except(self, memval_nullable_getloc(mval)));
		DO(fg_state_unshare(self));
		state = self->fg_state;
		mval = memstate_vtop(state);
		memval_nullable_getobj(mval)->mo_flags = saved_mo_flags;
		memval_nullable_makedirect(mval);
		if (memval_direct_typeof(mval) == &DeeNone_Type) {
			memstate_decrinuse_for_memloc(state, memval_direct_getloc(mval));
			memval_init_const(mval, Dee_None, &DeeNone_Type);
		}
	}
do_maybe_decref:
	DO(fg_vgdecref_vstack(self, mval));
	--state->ms_stackc;
	memstate_decrinuse_for_memval(state, mval);
	memval_fini(mval);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpopmany(struct fungen *__restrict self, vstackaddr_t n) {
	int result = 0;
	while (n) {
		--n;
		result = fg_vpop(self);
		if unlikely(result)
			break;
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpop_at(struct fungen *__restrict self, vstackaddr_t n) {
	ASSERT(n >= 1);        /* x, [n...] */
	DO(fg_vlrot(self, n)); /* [n...], x */
	DO(fg_vpop(self));     /* [n...] */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpop_local(struct fungen *__restrict self, lid_t lid) {
	struct memstate *state;
	struct memval *dst, *src;
	DO(fg_state_unshare(self));
	state = self->fg_state;
	ASSERT(lid < state->ms_localc);
	src = memstate_vtop(state);

	/* Load the destination and see if there's already something stored in there.
	 * This shouldn't usually be the case, especially if you didn't turn off
	 * FUNCTION_ASSEMBLER_F_NOEARLYDEL, but there always be situations where
	 * we need to delete a previously assigned value! */
	dst = &state->ms_localv[lid];
	DO(fg_vgdecref_local(self, dst));
	ASSERT(state == self->fg_state);
	ASSERT(dst == &state->ms_localv[lid]);
	ASSERT(src == memstate_vtop(state));

	/* Because stack elements are always bound, the local is guarantied bound at this point. */
	memstate_decrinuse_for_memval(state, dst);
	memval_fini(dst);
	memval_initmove(dst, src);
#ifndef NDEBUG
	{
		struct memobj *obj;
		memval_foreach_obj(obj, dst) {
			ASSERTF(!(obj->mo_flags & MEMOBJ_F_MAYBEUNBOUND),
			        "Shouldn't have been set for stack item");
		}
		memval_foreach_obj_end;
	}
#endif /* !NDEBUG */
	--state->ms_stackc;
/*done:*/
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vdel_local(struct fungen *__restrict self, lid_t lid) {
	struct memstate *state;
	struct memval *mval;
	DO(fg_state_unshare(self));
	state = self->fg_state;
	ASSERT(lid < state->ms_localc);
	mval = &state->ms_localv[lid];
	DO(fg_vgdecref_local(self, mval));
	ASSERT(state == self->fg_state);
	ASSERT(mval == &state->ms_localv[lid]);
	memstate_decrinuse_for_memval(state, mval);
	memval_fini(mval);
	memval_init_local_unbound(mval);
	return 0;
err:
	return -1;
}


/* Check if top `n' elements are all `MEMADR_TYPE_CONST' */
INTDEF WUNUSED NONNULL((1)) bool DCALL
fg_vallconst(struct fungen *__restrict self, vstackaddr_t n) {
	struct memstate *state = self->fg_state;
	struct memval *itemv;
	vstackaddr_t i;
	ASSERT(n <= state->ms_stackc);
	itemv = state->ms_stackv + state->ms_stackc - n;
	for (i = 0; i < n; ++i) {
		if (!memval_isconst(&itemv[i]))
			return false;
	}
	return true;
}

/* Check if top `n' elements are all `MEMADR_TYPE_CONST' and have the `MEMOBJ_F_NOREF' flag set. */
INTERN WUNUSED NONNULL((1)) bool DCALL
fg_vallconst_noref(struct fungen *__restrict self, vstackaddr_t n) {
	struct memstate *state = self->fg_state;
	struct memval *itemv;
	vstackaddr_t i;
	bool has_refs = false;
	ASSERT(n <= state->ms_stackc);
	itemv = state->ms_stackv + state->ms_stackc - n;
	for (i = 0; i < n; ++i) {
		if (!memval_isconst(&itemv[i]))
			return false;
		if (memval_direct_isref(&itemv[i]))
			has_refs = true;
	}
	if (has_refs) {
		for (i = 0; i < n; ++i) {
			struct memval *alias;
			struct memval *mval = &itemv[i];
			ASSERT(memval_isconst(mval));
			if (!memval_direct_isref(mval))
				continue;

			/* See if there is some alias which we can off-load the reference on-to. */
			state->ms_stackc -= n;
			memstate_foreach(alias, state) {
				if (!memval_isconst(alias))
					continue;
				if (memval_const_getobj(alias) != memval_const_getobj(mval))
					continue;
				if (memval_direct_isref(alias))
					continue;
				memval_direct_setref(alias);
				memval_direct_clearref(mval);
				state->ms_stackc += n;
				goto check_next_vstack_item;
			}
			memstate_foreach_end;
			state->ms_stackc += n;
			return false; /* Cannot off-load this reference :( */
check_next_vstack_item:
			;
		}
	}
	return true;
}

PRIVATE NONNULL((1, 2)) void DCALL
memval_direct_setvaltype(struct memstate *__restrict state,
                         struct memval *mval, DeeTypeObject *type) {
	memval_direct_settypeof(mval, type);
	if (type == &DeeNone_Type && memval_isdirect(mval)) {
		/* Special case: none is a singleton, so if that's the type,
		 * we can assume the runtime value of its location. */
		memstate_decrinuse_for_memloc(state, memval_direct_getloc(mval));
		memval_const_setobj_keeptyp(mval, Dee_None);
	}
}

/* Remember that VTOP, as well as any other memory location
 * that might be aliasing it is an instance of "type" at runtime. */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vsettyp(struct fungen *__restrict self,
           DeeTypeObject *type) {
	struct memstate *state;
	struct memval *vtop;
	DO(fg_state_unshare(self));
	state = self->fg_state;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	vtop = memstate_vtop(state);
	ASSERTF(memval_isdirect(vtop) || memval_isnullable(vtop),
	        "Can only assign types to direct or nullable values");
	if (memval_direct_typeof(vtop) != type) {
		struct memval *alias;
		memstate_foreach(alias, state) {
			if (!memval_isdirect(alias) && !memval_isnullable(alias))
				continue;
			if (!memval_direct_sameloc(alias, vtop))
				continue;
			memval_direct_setvaltype(state, alias, type);
		}
		memstate_foreach_end;
		ASSERT(memval_direct_typeof(vtop) == type);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vsettyp_noalias(struct fungen *__restrict self,
                   DeeTypeObject *type) {
	struct memstate *state;
	struct memval *vtop;
	DO(fg_state_unshare(self));
	state = self->fg_state;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	vtop = memstate_vtop(state);
	ASSERTF(memval_isdirect(vtop) || memval_isnullable(vtop),
	        "Can only assign types to direct or nullable values");
	if (memval_direct_typeof(vtop) != type) {
#ifndef NDEBUG
		struct memval *alias;
		memstate_foreach(alias, state) {
			ASSERT(alias == vtop || !memval_sameval(alias, vtop));
		}
		memstate_foreach_end;
#endif /* !NDEBUG */
		memval_direct_setvaltype(state, vtop, type);
	}
	return 0;
err:
	return -1;
}


/* VTOP = VTOP == <value> */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_veqconstaddr(struct fungen *__restrict self,
                void const *value) {
	/* Kind-of weird, but works perfectly and can use vmorph mechanism:
	 * >> PUSH((POP() - <value>) == 0); */
	struct memval *mval;
	DO(fg_vdelta(self, -(ptrdiff_t)(uintptr_t)value));
	DO(fg_state_unshare(self));
	mval = fg_vtop(self);
	ASSERT(memval_isdirect(mval));
	mval->mv_vmorph = MEMVAL_VMORPH_BOOL_Z;
	return 0;
err:
	return -1;
}

/* PUSH(POP() == POP()); // Based on address */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_veqaddr(struct fungen *__restrict self) {
	DeeObject *retval;
	host_regno_t result_regno;
	struct memval *a, *b;
	if unlikely(self->fg_state->ms_stackc < 2)
		return err_illegal_stack_effect();
	DO(fg_state_unshare(self));
	b = fg_vtop(self);
	a = b - 1;

	/* If either of the 2 locations is a constant, then
	 * we can use `fg_veqconstaddr()' */
	if (memval_hasobj0(a) && memval_obj0_isconst(a) && a->mv_vmorph == b->mv_vmorph) {
		DO(fg_vswap(self));
		goto do_constant;
	} else if (memval_hasobj0(b) && memval_obj0_isconst(b) && a->mv_vmorph == b->mv_vmorph) {
		void const *const_value;
do_constant:
		ASSERT(memval_isconst(b));
		ASSERT(memval_hasobj0(a));
		ASSERT(a->mv_vmorph == b->mv_vmorph);
		a->mv_vmorph = MEMVAL_VMORPH_DIRECT;
		b->mv_vmorph = MEMVAL_VMORPH_DIRECT;
		const_value = memval_obj0_const_getaddr(b);
		if (memval_obj0_isconst(a)) {
			/* Special case: Compare addresses of 2 constants */
			retval = DeeBool_For(memval_obj0_const_getaddr(a) == const_value);
do_return_retval:
			DO(fg_vpopmany(self, 2));
			return fg_vpush_const(self, retval);
		}
		DO(fg_vpop(self));
		return fg_veqconstaddr(self, const_value);
	}
	DO(fg_vdirect(self, 2));
	if (memval_hasobj0(a) && memval_obj0_isconst(a)) {
		DO(fg_vswap(self));
		goto do_constant;
	} else if (memval_hasobj0(b) && memval_obj0_isconst(b)) {
		goto do_constant;
	}

	/* Another special case: do the 2 locations alias each other? */
	if (memval_sameval(a, b)) {
		retval = Dee_True;
		goto do_return_retval;
	}

	/* Fallback: allocate a result register, then generate code to fill that
	 * register with a 0/1 value indicative of the 2 memory location being equal. */
	DO(fg_vdirect(self, 2));
	b = fg_vtop(self);
	a = b - 1;
	ASSERT(memval_isdirect(a));
	ASSERT(memval_isdirect(b));
	result_regno = fg_gallocreg(self, NULL);
	if unlikely(result_regno >= HOST_REGNO_COUNT)
		goto err;
	ASSERT(memval_isdirect(a));
	ASSERT(memval_isdirect(b));
	DO(fg_gmorph_locCloc2reg01(self,
	                                               memval_direct_getloc(a), 0, GMORPHBOOL_CC_EQ,
	                                               memval_direct_getloc(b), result_regno));
	DO(fg_vpush_hreg(self, result_regno, 0));
	a = fg_vtop(self);
	ASSERT(memval_isdirect(a));
	ASSERT(memval_direct_gettyp(a) == MEMADR_TYPE_HREG);
	ASSERT(memloc_hreg_getreg(memval_direct_getloc(a)) == result_regno);
	ASSERT(memloc_hreg_getvaloff(memval_direct_getloc(a)) == 0);
	a->mv_vmorph = MEMVAL_VMORPH_BOOL_NZ_01;
	DO(fg_vrrot(self, 3));
	DO(fg_vpop(self));
	return fg_vpop(self);
err:
	return -1;
}


/* >> if (THIRD == SECOND) // Based on address
 * >>     THIRD = FIRST;
 * >> POP();
 * >> POP(); */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcoalesce(struct fungen *__restrict self) {
	DREF struct memstate *common_state;
	struct host_symbol *Lnot_equal;
	struct memval *p_dst, *p_coalesce_from, *p_coalesce_to;
	struct memloc coalesce_from;
	if unlikely(self->fg_state->ms_stackc < 3)
		return err_illegal_stack_effect();
	DO(fg_state_unshare(self));
	p_coalesce_to   = fg_vtop(self);
	p_coalesce_from = p_coalesce_to - 1;
	p_dst           = p_coalesce_from - 1;
	if (memval_hasobj0(p_dst) && memval_obj0_isconst(p_dst)) {
		struct memval temp;
		memval_initmove(&temp, p_dst);
		memval_initmove(p_dst, p_coalesce_from);
		memval_initmove(p_coalesce_from, &temp);
	}
	if (memval_sameval(p_coalesce_from, p_coalesce_to) ||
	    memval_sameval(p_dst, p_coalesce_from)) {
		/* Special case: result is always "coalesce_to" */
		DO(fg_vrrot(self, 3));
		DO(fg_vpop(self));
		return fg_vpop(self);
	}

	/* Fallback: generate code to branch at runtime. */
	DO(fg_vdirect(self, 3)); /* from, to, dst */
	DO(fg_vlrot(self, 3));   /* from, to, dst */
	DO(fg_vreg(self, NULL)); /* from, to, reg:dst */
	DO(fg_vrrot(self, 3));   /* reg:dst, from, to */
	p_coalesce_to   = fg_vtop(self);
	p_coalesce_from = p_coalesce_to - 1;
	p_dst           = p_coalesce_from - 1;
	coalesce_from   = *memval_direct_getloc(p_coalesce_from);
	DO(fg_vpop_at(self, 2)); /* reg:dst, to */
	ASSERT(p_coalesce_from == fg_vtop(self));
	ASSERT(p_coalesce_to == fg_vtop(self) + 1);
	Lnot_equal = fg_newsym_named(self, ".Lnot_equal");
	if unlikely(!Lnot_equal)
		goto err;
	common_state = memstate_copy(self->fg_state);
	if unlikely(!common_state)
		goto err;
	EDO(err_common_state, fg_gjcc(self, memval_direct_getloc(p_dst),
	                              &coalesce_from, false,
	                              Lnot_equal, NULL, Lnot_equal));
	EDO(err_common_state, fg_vpop_at(self, 2));           /* to */
	EDO(err_common_state, fg_vdup(self));                 /* to, to */
	EDO(err_common_state, fg_vmorph(self, common_state)); /* ... */
	memstate_decref(common_state);
	host_symbol_setsect(Lnot_equal, fg_gettext(self));
	return fg_vpop(self);
err_common_state:
	memstate_decref(common_state);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcoalesce_c(struct fungen *__restrict self,
               void const *from, void const *to) {
	DO(fg_vpush_addr(self, from));
	DO(fg_vpush_addr(self, to));
	return fg_vcoalesce(self);
err:
	return -1;
}

/* Clear the `MEMOBJ_F_ONEREF' flag for the top `n' v-stack elements,
 * as well as any other memory location that might be aliasing them. */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vnotoneref(struct fungen *__restrict self, vstackaddr_t n) {
	vstackaddr_t i;
	struct memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < n)
		return err_illegal_stack_effect();
	for (i = state->ms_stackc - n; i < state->ms_stackc; ++i) {
		struct memval *mval = &state->ms_stackv[i];
		struct memobj *mobj;
again_foreach_mobj:
		memval_foreach_obj(mobj, mval) {
			if (memobj_isoneref(mobj)) {
				if (memstate_isshared(state)) {
					state = memstate_copy(state);
					if unlikely(!state)
						goto err;
					memstate_decref_nokill(self->fg_state);
					self->fg_state = state;
					mval = &state->ms_stackv[i];
					goto again_foreach_mobj;
				}
				DO(fg_gnotoneref_impl(self, mobj));
			}
		}
		memval_foreach_obj_end;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vnotoneref_at(struct fungen *__restrict self, vstackaddr_t off) {
	struct memval *mval;
	struct memobj *mobj;
	struct memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < off)
		return err_illegal_stack_effect();
again_foreach_mobj:
	mval = &state->ms_stackv[state->ms_stackc - off];
	memval_foreach_obj(mobj, mval) {
		if (memobj_isoneref(mobj)) {
			if (memstate_isshared(state)) {
				state = memstate_copy(state);
				if unlikely(!state)
					goto err;
				memstate_decref_nokill(self->fg_state);
				self->fg_state = state;
				goto again_foreach_mobj;
			}
			DO(fg_gnotoneref_impl(self, mobj));
		}
	}
	memval_foreach_obj_end;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vnotoneref_if_operator(struct fungen *__restrict self,
                          Dee_operator_t operator_name, vstackaddr_t n) {
	vstackaddr_t i;
	struct memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < n)
		return err_illegal_stack_effect();
	for (i = state->ms_stackc - n; i < state->ms_stackc; ++i) {
		struct memval *mval = &state->ms_stackv[i];
		struct memobj *mobj;
again_foreach_mobj:
		memval_foreach_obj(mobj, mval) {
			DeeTypeObject *loctype = memval_typeof(mval);
			if (loctype != NULL && DeeType_IsOperatorNoRefEscape(loctype, operator_name))
				continue; /* Type is known to not let references to its instances escape. */
			if (memobj_isoneref(mobj)) {
				if (memstate_isshared(state)) {
					state = memstate_copy(state);
					if unlikely(!state)
						goto err;
					memstate_decref_nokill(self->fg_state);
					self->fg_state = state;
					mval = &state->ms_stackv[i];
					goto again_foreach_mobj;
				}
				DO(fg_gnotoneref_impl(self, mobj));
			}
		}
		memval_foreach_obj_end;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vnotoneref_if_operator_at(struct fungen *__restrict self,
                             Dee_operator_t operator_name, vstackaddr_t off) {
	struct memval *mval;
	struct memobj *mobj;
	struct memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < off)
		return err_illegal_stack_effect();
again_foreach_mobj:
	mval = &state->ms_stackv[state->ms_stackc - off];
	if (operator_name != OPERATOR_ITER) {
	}
	DeeTypeObject *loctype = memval_typeof(mval);
	if (loctype != NULL && DeeType_IsOperatorNoRefEscape(loctype, operator_name))
		return 0; /* Type is known to not let references to its instances escape. */
	memval_foreach_obj(mobj, mval) {
		if (memobj_isoneref(mobj)) {
			if (memstate_isshared(state)) {
				state = memstate_copy(state);
				if unlikely(!state)
					goto err;
				memstate_decref_nokill(self->fg_state);
				self->fg_state = state;
				goto again_foreach_mobj;
			}
			DO(fg_gnotoneref_impl(self, mobj));
		}
	}
	memval_foreach_obj_end;
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_ulocal(struct fungen *__restrict self,
                Dee_instruction_t const *instr, ulid_t ulid) {
	if unlikely(ulid >= self->fg_assembler->fa_localc)
		return err_illegal_ulid(ulid);
	return fg_vpush_local(self, instr, (lid_t)ulid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vbound_ulocal(struct fungen *__restrict self,
                 Dee_instruction_t const *instr, ulid_t ulid) {
	if unlikely(ulid >= self->fg_assembler->fa_localc)
		return err_illegal_ulid(ulid);
	return fg_vbound_local(self, instr, (lid_t)ulid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpop_ulocal(struct fungen *__restrict self, ulid_t ulid) {
	if unlikely(ulid >= self->fg_assembler->fa_localc)
		return err_illegal_ulid(ulid);
	return fg_vpop_local(self, (lid_t)ulid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vdel_ulocal(struct fungen *__restrict self, ulid_t ulid) {
	if unlikely(ulid >= self->fg_assembler->fa_localc)
		return err_illegal_ulid(ulid);
	return fg_vdel_local(self, (lid_t)ulid);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_vpushinit_optarg(struct fungen *__restrict self,
                    Dee_instruction_t const *instr, lid_t xlid) {
	DREF struct memstate *common_state;
	DeeCodeObject *code = self->fg_assembler->fa_code;
	uint16_t opt_aid = (uint16_t)(xlid - MEMSTATE_XLOCAL_DEFARG_MIN);
	aid_t aid = opt_aid + code->co_argc_min;
	lid_t lid = self->fg_assembler->fa_localc + xlid;
	DeeObject *default_value;
	ASSERT(xlid >= MEMSTATE_XLOCAL_DEFARG_MIN);
	ASSERT(lid < self->fg_state->ms_localc);
	default_value = self->fg_assembler->fa_code->co_defaultv[opt_aid];
	if (default_value) {
		struct host_symbol *Luse_default;

		/* Load the default value into a register and into the local */
		DO(fg_vpush_const(self, default_value)); /* default_value */
		DO(fg_vreg(self, NULL));                 /* reg:default_value */

		/* Check if the caller has provided enough arguments. */
		DO(fg_vpush_argc(self)); /* reg:default_value, argc */
		DO(fg_vdirect1(self));   /* reg:default_value, argc */
		Luse_default = fg_newsym_named(self, ".Luse_default");
		if unlikely(!Luse_default)
			goto err;
		{
			struct memloc l_argc, l_aid;
			ASSERT(fg_vtop_isdirect(self));
			ASSERT(!fg_vtop_direct_isref(self));
			l_argc = *fg_vtopdloc(self);
			memval_direct_fini(fg_vtop(self));
			--self->fg_state->ms_stackc;
			memstate_decrinuse_for_memloc(self->fg_state, &l_argc);
			memloc_init_const(&l_aid, (void *)(uintptr_t)aid);
			DO(fg_gjcc(self, &l_aid, &l_argc, false, NULL,
			                               Luse_default, Luse_default));
		}
		common_state = self->fg_state;
		memstate_incref(common_state);
		EDO(err_common_state, fg_vpop(self));                                       /* - */
		EDO(err_common_state, fg_vpush_argv(self));                                 /* argv */
		EDO(err_common_state, fg_vind(self, (ptrdiff_t)aid * sizeof(DeeObject *))); /* argv[aid] */
		EDO(err_common_state, fg_vmorph(self, common_state));                       /* reg:value */
		memstate_decref(common_state);
		host_symbol_setsect(Luse_default, fg_gettext(self));
	} else {
		if (self->fg_state->ms_uargc_min <= aid) {
			struct host_section *text;
			struct host_section *cold;
			struct host_symbol *Ltarget;
			struct memloc l_argc, l_aid;
			Ltarget = fg_newsym_named(self, ".Ltarget");
			if unlikely(!Ltarget)
				goto err;
			text = fg_gettext(self);
			cold = fg_getcold(self);
			if unlikely(!cold)
				goto err;
			DO(fg_vpush_argc(self));
			DO(fg_vdirect1(self));
			ASSERT(fg_vtop_isdirect(self));
			ASSERT(!fg_vtop_direct_isref(self));
			l_argc = *fg_vtopdloc(self);
			memval_direct_fini(fg_vtop(self));
			--self->fg_state->ms_stackc;
			memstate_decrinuse_for_memloc(self->fg_state, &l_argc);
			memloc_init_const(&l_aid, (void *)(uintptr_t)aid);
			DO(fg_gjcc(self, &l_aid, &l_argc, false,
			                               text != cold ? NULL : Ltarget,
			                               text != cold ? Ltarget : NULL,
			                               text != cold ? Ltarget : NULL));
			common_state = self->fg_state;
			memstate_incref(common_state);
			HA_printf(".section .cold\n");
			if (text != cold) {
				EDO(err_common_state, fg_settext(self, cold));
				host_symbol_setsect(Ltarget, cold);
			}
			EDO(err_common_state, fg_gthrow_arg_unbound(self, instr, aid));
			if (text == cold) {
				host_symbol_setsect(Ltarget, text);
			} else {
				HA_printf(".section .text\n");
				EDO(err_common_state, fg_settext(self, text));
			}

			/* After the check above, we're allowed to remember that the argument
			 * count is great enough to always include the accessed argument. */
			self->fg_state->ms_uargc_min = aid + 1;

			/* Restore state from before exception handling was entered. */
			memstate_decref(self->fg_state);
			self->fg_state = common_state;
		}
		DO(fg_vpush_argv(self));                                 /* argv */
		DO(fg_vind(self, (ptrdiff_t)aid * sizeof(DeeObject *))); /* argv[aid] */
	}
	return 0;
err_common_state:
	memstate_decref(common_state);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_vpushinit_varargs(struct fungen *__restrict self) {
	struct fg_branch branch;
	uint16_t co_argc_min = self->fg_assembler->fa_code->co_argc_min;
	uint16_t co_argc_max = self->fg_assembler->fa_code->co_argc_max;
	/* NOTE: The special "co_argc_max == 0 && HOST_CC_F_TUPLE" case
	 *       is handled in `fg_vpush_xlocal()'! */
	DO(fg_vpush_argc(self)); /* argc */
	if (co_argc_min < co_argc_max && self->fg_state->ms_uargc_min < co_argc_max) {
		/* Special case: If `argc-co_argc_max' rolls over or is 0, then we have to push an empty tuple
		 *               This is because less than `co_argc_max' may be provided by the caller if the
		 *               function also takes default/optional arguments:
		 * >> function foo(a = 0, b = 1, args...) {
		 * >>     return args;
		 * >> }
		 * >> print repr foo();               // ()
		 * >> print repr foo(10);             // ()
		 * >> print repr foo(10, 20);         // ()
		 * >> print repr foo(10, 20, 30);     // (30,)
		 * >> print repr foo(10, 20, 30, 40); // (30,40)
		 */
		DO(fg_vpush_const(self, Dee_EmptyTuple));     /* argc, empty_tuple */
		DO(fg_vreg(self, NULL));                      /* argc, reg:empty_tuple */
		DO(fg_vref_noalias(self));                    /* argc, ref:reg:empty_tuple */
		DO(fg_vswap(self));                           /* ref:reg:empty_tuple, argc */
		DO(fg_vpush_immSIZ(self, co_argc_max));       /* ref:reg:empty_tuple, argc, co_argc_max */
		DO(fg_vjno_usub_enter_likely(self, &branch)); /* ref:reg:empty_tuple, argc-co_argc_max */
		EDO(err_branch, fg_vpop_at(self, 2));         /* argc-co_argc_max */
		EDO(err_branch, fg_vpush_argv(self));         /* argc-co_argc_max, argv */
		EDO(err_branch, fg_vdelta(self, (ptrdiff_t)co_argc_max * sizeof(DeeObject *))); /* argc-co_argc_max, argv+co_argc_max */
		EDO(err_branch, fg_vcallapi(self, &DeeTuple_NewVector, VCALL_CC_OBJECT, 2));    /* varargs */
		EDO(err_branch, fg_vpush_undefined(self));    /* varargs, undefined */
		DO(fg_vjx_leave(self, &branch));              /* varargs, <some_value> */
		return fg_vpop(self);                         /* varargs */
	} else {
		DO(fg_vdelta(self, -(ptrdiff_t)co_argc_max)); /* argc-co_argc_max */
	}
	DO(fg_vpush_argv(self));                                           /* argc-co_argc_max, argv */
	DO(fg_vdelta(self, (ptrdiff_t)co_argc_max * sizeof(DeeObject *))); /* argc-co_argc_max, argv+co_argc_max */
	DO(fg_vcallapi(self, &DeeTuple_NewVector, VCALL_CC_OBJECT, 2));    /* varargs */
	fg_voneref_noalias(self);
	return 0;
err_branch:
	fg_branch_fini(&branch);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_vpushinit_varkwds(struct fungen *__restrict self) {
	/* TODO */
	(void)self;
	return DeeError_NOTIMPLEMENTED();
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_vpushinit_stdout(struct fungen *__restrict self) {
	int result = fg_vpush_imm32(self, DEE_STDOUT);
	if likely(result == 0)
		result = fg_vcallapi(self, &DeeFile_GetStd, VCALL_CC_OBJECT, 1);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_vpushinit_xlocal(struct fungen *__restrict self,
                    Dee_instruction_t const *instr, lid_t xlid) {
	switch (xlid) {

	case MEMSTATE_XLOCAL_VARARGS:
		return fg_vpushinit_varargs(self);
	case MEMSTATE_XLOCAL_VARKWDS:
		return fg_vpushinit_varkwds(self);
	case MEMSTATE_XLOCAL_STDOUT:
		return fg_vpushinit_stdout(self);

	default:
		if (xlid >= MEMSTATE_XLOCAL_DEFARG_MIN)
			return fg_vpushinit_optarg(self, instr, xlid);
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "No way to init xlid=%" PRFuSIZ,
		                       xlid);
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
fg_vinit_xlocal(struct fungen *__restrict self,
                Dee_instruction_t const *instr, lid_t xlid) {
	/* Push the initializer for the x-local onto the v-stack. */
	DO(fg_vpushinit_xlocal(self, instr, xlid)); /* init */
	DO(fg_vdirect1(self));                      /* init */
	DO(fg_state_unshare(self));                 /* init */
	return fg_vpop_local(self, self->fg_assembler->fa_localc + xlid);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_xlocal(struct fungen *__restrict self,
                Dee_instruction_t const *instr, lid_t xlid) {
	DREF struct memstate *common_state;
	struct memval *xlocal_mval;

	/* Optimizations (and special handling) for certain xlocal slots. */
	switch (xlid) {

	case MEMSTATE_XLOCAL_VARARGS:
		/* Check for special case: when the function *only* takes varargs, and
		 * the calling convention provides us with a caller-given argument tuple,
		 * then simply push that tuple instead of allocating a new one! */
		if unlikely(!(self->fg_assembler->fa_code->co_flags & CODE_FVARARGS))
			return err_unsupported_opcode(self->fg_assembler->fa_code, instr);
		if (self->fg_assembler->fa_code->co_argc_max == 0 &&
		    (self->fg_assembler->fa_cc & HOST_CC_F_TUPLE))
			return fg_vpush_args(self, instr);
		break;

	case MEMSTATE_XLOCAL_VARKWDS:
		if unlikely(!(self->fg_assembler->fa_code->co_flags & CODE_FVARKWDS))
			return err_unsupported_opcode(self->fg_assembler->fa_code, instr);
		break;

	default: break;
	}

	/* Check if the slot needs to be initialized (and if so: initialize it) */
	DO(fg_state_unshare(self));
	xlocal_mval = &self->fg_state->ms_localv[self->fg_assembler->fa_localc + xlid];
	if (!memval_isdirect(xlocal_mval)) {
		/* Non-direct values don't need initialization! */
	} else if (xlid >= MEMSTATE_XLOCAL_DEFARG_MIN
	           /* Special case: for default arguments, UNDEFINED means uninitialized,
	            * and NEVERBOUND is a valid, initialized state for argument-not-given. */
	           ? memval_direct_isundefined(xlocal_mval)
	           : memval_direct_local_neverbound(xlocal_mval)) {
		DO(fg_vinit_xlocal(self, instr, xlid));
	} else if (!memval_direct_local_alwaysbound(xlocal_mval)) {
		struct host_symbol *Lskipinit;
		ASSERT(memval_isdirect(xlocal_mval));
		if (memval_direct_gettyp(xlocal_mval) != MEMADR_TYPE_HSTACKIND &&
		    memval_direct_gettyp(xlocal_mval) != MEMADR_TYPE_HREG) {
			DO(tracked_memloc_forcereg(self, memval_direct_getloc(xlocal_mval), NULL));
			ASSERT(memval_direct_gettyp(xlocal_mval) == MEMADR_TYPE_HREG);
		}
		Lskipinit = fg_newsym_named(self, ".Lskipinit");
		if unlikely(!Lskipinit)
			goto err;
		DO(fg_gjnz(self, memval_direct_getloc(xlocal_mval), Lskipinit));
		DO(fg_state_unshare(self));
		common_state = memstate_copy(self->fg_state);
		if unlikely(!common_state)
			goto err;
		xlocal_mval = &self->fg_state->ms_localv[self->fg_assembler->fa_localc + xlid];
		memval_fini_direct(xlocal_mval);
		memval_init_local_unbound(xlocal_mval);
		if unlikely(fg_vinit_xlocal(self, instr, xlid))
			goto err_common_state;
		if unlikely(fg_vmorph(self, common_state))
			goto err_common_state;
		memstate_decref(common_state);
		host_symbol_setsect(Lskipinit, fg_gettext(self));
	}
	ASSERTF(!memval_isdirect(&self->fg_state->ms_localv[self->fg_assembler->fa_localc + xlid]) ||
	        memval_direct_local_alwaysbound(&self->fg_state->ms_localv[self->fg_assembler->fa_localc + xlid]),
	        "The local should be unconditionally bound at this point!");
	return _fg_vpush_xlocal(self, instr, xlid);
err_common_state:
	memstate_decref(common_state);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_this_function(struct fungen *__restrict self) {
	if (self->fg_assembler->fa_cc & HOST_CC_F_FUNC)
		return _fg_vpush_xlocal(self, NULL, MEMSTATE_XLOCAL_A_FUNC);
	return fg_vpush_const(self, self->fg_assembler->fa_function);
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_argc(struct fungen *__restrict self) {
	if (!(self->fg_assembler->fa_cc & HOST_CC_F_TUPLE))
		return _fg_vpush_xlocal(self, NULL, MEMSTATE_XLOCAL_A_ARGC);
	DO(_fg_vpush_xlocal(self, NULL, MEMSTATE_XLOCAL_A_ARGS));
	return fg_vind(self, offsetof(DeeTupleObject, t_size));
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_argv(struct fungen *__restrict self) {
	if (!(self->fg_assembler->fa_cc & HOST_CC_F_TUPLE))
		return _fg_vpush_xlocal(self, NULL, MEMSTATE_XLOCAL_A_ARGV);
	DO(_fg_vpush_xlocal(self, NULL, MEMSTATE_XLOCAL_A_ARGS));
	return fg_vdelta(self, offsetof(DeeTupleObject, t_elem));
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_usage(struct fungen *__restrict self,
               host_regusage_t usage) {
	host_regno_t regno;
	DO(fg_state_unshare(self));
	regno = fg_gusagereg(self, usage, NULL);
	if unlikely(regno >= HOST_REGNO_COUNT)
		goto err;
	return fg_vpush_hreg(self, regno, 0);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_except(struct fungen *__restrict self) {
	DREF struct memstate *saved_state;
	DO(fg_vpush_usage(self, HOST_REGUSAGE_THREAD));     /* DeeThread_Self() */
	DO(fg_vind(self, offsetof(DeeThreadObject, t_except))); /* DeeThread_Self()->t_except */
	/* Check if there is an active exception if not already checked. */
	if (!(self->fg_state->ms_flags & MEMSTATE_F_GOTEXCEPT)) {
		struct host_section *text = fg_gettext(self);
		struct host_section *cold = fg_getcold(self);
		if unlikely(!cold)
			goto err;
		ASSERT(fg_vtop_isdirect(self));
		if (text == cold) {
			struct host_symbol *text_Ldone;
			text_Ldone = fg_newsym_named(self, ".Ldone");
			if unlikely(!text_Ldone)
				goto err;
			DO(fg_gjnz(self, fg_vtopdloc(self), text_Ldone));
			saved_state = self->fg_state;
			memstate_incref(saved_state);
			EDO(err_saved_state, fg_state_dounshare(self));
			EDO(err_saved_state, fg_vcallapi(self, &libhostasm_rt_err_no_active_exception, VCALL_CC_EXCEPT, 0));
			memstate_decref(self->fg_state);
			self->fg_state = saved_state;
			host_symbol_setsect(text_Ldone, text);
		} else {
			struct host_symbol *Lerr_no_active_exception;
			Lerr_no_active_exception = fg_newsym_named(self, ".Lerr_no_active_exception");
			if unlikely(!Lerr_no_active_exception)
				goto err;
			DO(fg_gjz(self, fg_vtopdloc(self), Lerr_no_active_exception));
			saved_state = self->fg_state;
			memstate_incref(saved_state);
			HA_printf(".section .cold\n");
			EDO(err_saved_state, fg_settext(self, cold));
			host_symbol_setsect(Lerr_no_active_exception, cold);
			EDO(err_saved_state, fg_state_dounshare(self));
			EDO(err_saved_state, fg_vcallapi(self, &libhostasm_rt_err_no_active_exception, VCALL_CC_EXCEPT, 0));
			HA_printf(".section .text\n");
			EDO(err_saved_state, fg_settext(self, text));
			memstate_decref(self->fg_state);
			self->fg_state = saved_state;
		}
		/* Remember that there is an exception */
		self->fg_state->ms_flags |= MEMSTATE_F_GOTEXCEPT;
	}
	/* DeeThread_Self()->t_except->ef_error */
	return fg_vind(self, offsetof(struct Dee_except_frame, ef_error));
err_saved_state:
	memstate_decref(saved_state);
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
fg_gunbound_member(struct fungen *__restrict self,
                   DeeTypeObject *__restrict class_type, uint16_t addr,
                   void const *api_function) {
	DO(fg_vpush_const(self, class_type));
	DO(fg_vpush_imm16(self, addr));
	return fg_vcallapi(self, api_function, VCALL_CC_EXCEPT, 2);
err:
	return -1;
}

#define fg_gunbound_class_member(self, class_type, addr) \
	fg_gunbound_member(self, class_type, addr, (void const *)&libhostasm_rt_err_unbound_class_member)
#define fg_gunbound_instance_member(self, class_type, addr) \
	fg_gunbound_member(self, class_type, addr, (void const *)&libhostasm_rt_err_unbound_instance_member)

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
fg_vpush_cmember_unsafe_at_runtime(struct fungen *__restrict self,
                                   DeeTypeObject *class_type,
                                   uint16_t addr, unsigned int flags) {
	struct class_desc *desc = DeeClass_DESC(class_type);
	struct host_section *text;
	struct host_section *cold;
	DREF struct memstate *saved_state;

	/* When optimizing for size, generate a (smaller) call to
	 * `DeeClass_GetMember()', instead of inlining the function. */
	if (self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE) {
		DO(fg_vpush_const(self, class_type));
		DO(fg_vpush_imm16(self, addr));
		return fg_vcallapi(self, &DeeClass_GetMember, VCALL_CC_OBJECT, 2);
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
#ifndef CONFIG_NO_THREADS
	if (flags & FG_CIMEMBER_F_REF)
		DO(fg_grwlock_read_const(self, &desc->cd_lock));
#endif /* !CONFIG_NO_THREADS */
	DO(fg_vpush_addr(self, &desc->cd_members[addr])); /* p_value */
	DO(fg_vind(self, 0));                             /* *p_value */
	DO(fg_vreg(self, NULL));                          /* reg:value */
	ASSERT(fg_vtop_isdirect(self));
	text = fg_gettext(self);
	cold = fg_getcold(self);
	if unlikely(!cold)
		goto err;
	if (cold == text) {
		struct host_symbol *text_Lbound;
		text_Lbound = fg_newsym_named(self, ".Lbound");
		if unlikely(!text_Lbound)
			goto err;
		DO(fg_gjnz(self, fg_vtopdloc(self), text_Lbound));
		saved_state = self->fg_state;
		memstate_incref(saved_state);
		EDO(err_saved_state, fg_state_dounshare(self));
#ifndef CONFIG_NO_THREADS
		if (flags & FG_CIMEMBER_F_REF)
			EDO(err_saved_state, fg_grwlock_endread_const(self, &desc->cd_lock));
#endif /* !CONFIG_NO_THREADS */
		EDO(err_saved_state, fg_gunbound_class_member(self, class_type, addr));
		host_symbol_setsect(text_Lbound, text);
	} else {
		struct host_symbol *cold_Lunbound_member;
		cold_Lunbound_member = fg_newsym_named(self, ".Lunbound_member");
		if unlikely(!cold_Lunbound_member)
			goto err;
		DO(fg_gjz(self, fg_vtopdloc(self), cold_Lunbound_member));
		saved_state = self->fg_state;
		memstate_incref(saved_state);
		HA_printf(".section .cold\n");
		EDO(err_saved_state, fg_settext(self, cold));
		host_symbol_setsect(cold_Lunbound_member, cold);
		EDO(err_saved_state, fg_state_dounshare(self));
#ifndef CONFIG_NO_THREADS
		if (flags & FG_CIMEMBER_F_REF)
			EDO(err_saved_state, fg_grwlock_endread_const(self, &desc->cd_lock));
#endif /* !CONFIG_NO_THREADS */
		EDO(err_saved_state, fg_gunbound_class_member(self, class_type, addr));
		HA_printf(".section .text\n");
		EDO(err_saved_state, fg_settext(self, text));
	}
	memstate_decref(self->fg_state);
	self->fg_state = saved_state;
	ASSERT(!fg_vtop_direct_isref(self));
	ASSERT(fg_vtop_isdirect(self));
	if (!(flags & FG_CIMEMBER_F_REF))
		return 0;
	DO(fg_gincref_loc(self, fg_vtopdloc(self), 1));
	fg_vtop_direct_setref(self);
#ifndef CONFIG_NO_THREADS
	DO(fg_grwlock_endread_const(self, &desc->cd_lock));
#endif /* !CONFIG_NO_THREADS */
	return 0;
err_saved_state:
	memstate_decref(saved_state);
err:
	return -1;
}

/* type -> value */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_cmember(struct fungen *__restrict self,
                 uint16_t addr, unsigned int flags) {
	struct memval *type_mval;
	DO(fg_vdirect1(self));
	type_mval = fg_vtop(self);
	if (memval_direct_isconst(type_mval)) {
		struct class_desc *desc;
		DeeTypeObject *class_type = (DeeTypeObject *)memval_const_getobj(type_mval);
		if unlikely(DeeObject_AssertType(class_type, &DeeType_Type))
			goto err;
		if unlikely(!DeeType_IsClass(class_type))
			return libhostasm_rt_err_requires_class(class_type);
		desc = DeeClass_DESC(class_type);
		if unlikely(addr >= desc->cd_desc->cd_cmemb_size)
			return libhostasm_rt_err_invalid_class_addr(class_type, addr);
		DO(fg_vpop(self)); /* Get rid of the `class_type' v-stack item. */
		if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_NOROINLINE)) {
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
					member_value = fg_inlineref(self, member_value);
					if unlikely(!member_value)
						goto err;
					return fg_vpush_const(self, member_value);
				}
				Dee_Decref(member_value);
			}
		}

		/* Push the member at runtime, but allowed to use (normally) unsafe
		 * semantics since we were already able to verify all constraints. */
		return fg_vpush_cmember_unsafe_at_runtime(self, class_type, addr, flags);
	}

	/* Fallback: access the class member at runtime */
	DO(fg_vpush_imm16(self, addr));
	return fg_vcallapi(self,
	                   ((self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_SAFE) ||
	                    (flags & FG_CIMEMBER_F_SAFE))
	                   ? (void const *)&DeeClass_GetMemberSafe
	                   : (void const *)&DeeClass_GetMember,
	                   VCALL_CC_OBJECT, 2);
err:
	return -1;
}


/* type -> bound */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vbound_cmember(struct fungen *__restrict self,
                  uint16_t addr, unsigned int flags) {
	struct memval *type_mval, *vtop;
	DO(fg_vdirect1(self));
	type_mval = fg_vtop(self);
	if (memval_direct_isconst(type_mval)) {
		DeeObject **p_valloc;
		struct class_desc *desc;
		DeeTypeObject *class_type = (DeeTypeObject *)memval_const_getobj(type_mval);
		if unlikely(DeeObject_AssertType(class_type, &DeeType_Type))
			goto err;
		if unlikely(!DeeType_IsClass(class_type))
			return libhostasm_rt_err_requires_class(class_type);
		desc = DeeClass_DESC(class_type);
		if unlikely(addr >= desc->cd_desc->cd_cmemb_size)
			return libhostasm_rt_err_invalid_class_addr(class_type, addr);
		DO(fg_vpop(self)); /* N/A */
		p_valloc = &desc->cd_members[addr];
		if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_NOROINLINE) &&
		    atomic_read(p_valloc) != NULL &&
		    DeeClassDescriptor_IsClassAttributeReadOnly(desc->cd_desc, addr)) {
			return fg_vpush_const(self, Dee_True);
		}
		DO(fg_vpush_addr(self, p_valloc)); /* p_valloc */
		DO(fg_vind(self, 0));              /* *p_valloc */
		DO(fg_vreg(self, NULL));           /* reg:*p_valloc */
		vtop = fg_vtop(self);
		ASSERT(memval_isdirect(vtop));
		vtop->mv_vmorph = MEMVAL_VMORPH_BOOL_NZ;
		return 0;
	}

	/* Fallback: access the class member at runtime */
	if ((self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_SAFE) ||
	    (flags & FG_CIMEMBER_F_SAFE)) {
		DO(fg_vpush_imm16(self, addr));
		DO(fg_vcallapi(self, &DeeClass_BoundMemberSafe, VCALL_CC_NEGINT, 2));
		vtop = fg_vtop(self);
		ASSERT(memval_isdirect(vtop));
		vtop->mv_vmorph = MEMVAL_VMORPH_TESTNZ(vtop->mv_vmorph);
	} else {
		DO(fg_vind(self, offsetof(DeeTypeObject, tp_class))); /* type->tp_class */
		DO(fg_vind(self,                                      /* type->tp_class->cd_members[addr] */
		           _Dee_MallococBufsize(offsetof(struct Dee_class_desc, cd_members[0]),
		                                addr, sizeof(DREF DeeObject *))));
		DO(fg_vreg(self, NULL)); /* reg:type->tp_class->cd_members[addr] */
		vtop = fg_vtop(self);
		ASSERT(memval_isdirect(vtop));
		vtop->mv_vmorph = MEMVAL_VMORPH_BOOL_NZ;
	}
	return 0;
err:
	return -1;
}

/* type, value -> N/A */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpop_cmember(struct fungen *__restrict self,
                uint16_t addr, unsigned int flags) {
	struct memobj *type_mobj;
	DO(fg_vdirect(self, 2));       /* type, value */
	DO(fg_vnotoneref_at(self, 1)); /* type, value */
	if (self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_SAFE)
		flags |= FG_CIMEMBER_F_SAFE; /* Force safe semantics. */

	/* Check if we have known meta-data about the "type" operand. */
	type_mobj = memval_getobj0(fg_vtop(self) - 1);
	if (memobj_hasxinfo(type_mobj)) {
		struct memobj_xinfo *type_xinfo = memobj_getxinfo(type_mobj);
		struct memobj_xinfo_cdesc *type_cdesc = type_xinfo->mox_cdesc;
		if (type_cdesc && addr < type_cdesc->moxc_desc->cd_cmemb_size) {
			/* If we can predict certain things about the state of the class
			 * descriptor, we can generate some highly optimized inline code:
			 * - Omit locking if we know there's only a single reference
			 * - Omit xdecref'ing previously assigned values for never-before assigned slots
			 * - Let the class inherit a reference to "value"
			 *
			 * XXX: When a reference to the class type is used to construct a member
			 *      function (i.e. used as a reference in a member function), then that
			 *      should *not* count as being a reason not to do the MEMOBJ_F_ONEREF
			 *      optimization here. (however: the MEMOBJ_F_ONEREF flag itself must
			 *      still be cleared in that case).
			 *      However: if one of those functions gets called or is passed somewhere
			 *      where that function might get called (or have its references inspected),
			 *      then we must once again *NOT* do this optimization here!
			 */
			if (type_mobj->mo_flags & MEMOBJ_F_ONEREF) {
				if (!memobj_xinfo_cdesc_wasinit(type_cdesc, addr)) {
					memobj_xinfo_cdesc_setinit(type_cdesc, addr);
					/* The cmember slot is known to be NULL, so we can just directly write to it:
					 * >> struct class_desc *cd = <type>->tp_class;
					 * >> cd->cd_members[<addr>] = <value>; // Inherit */
					DO(fg_vref2(self, 2));                                /* type, value */
					DO(fg_vdup_at(self, 2));                              /* type, value, type */
					DO(fg_vind(self, offsetof(DeeTypeObject, tp_class))); /* type, value, type->tp_class */
					DO(fg_vswap(self));                                   /* type, type->tp_class, value */
					DO(fg_vpopind(self,                                   /* type, type->tp_class */
					              _Dee_MallococBufsize(offsetof(struct Dee_class_desc, cd_members[0]),
					                                   addr, sizeof(DREF DeeObject *))));
					return fg_vpopmany(self, 2); /* N/A */
				}
			}
			if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE)) {
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
			flags &= ~FG_CIMEMBER_F_SAFE;
		}
	}

	/* Fallback: do the assignment at runtime. */
	DO(fg_vpush_imm16(self, addr)); /* type, value, addr */
	DO(fg_vswap(self));             /* type, addr, value */
	return (flags & FG_CIMEMBER_F_SAFE)
	       ? fg_vcallapi(self, &DeeClass_SetMemberSafe, VCALL_CC_INT, 3)
	       : fg_vcallapi(self, &DeeClass_SetMember, VCALL_CC_VOID, 3);
err:
	return -1;
}


/* this -> value */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
fg_vpush_imember_unsafe_at_runtime(struct fungen *__restrict self,
                                   DeeTypeObject *type, uint16_t addr,
                                   unsigned int flags) {
	struct host_section *text;
	struct host_section *cold;
	DREF struct memstate *saved_state;
	struct class_desc *desc = DeeClass_DESC(type);
#ifndef CONFIG_NO_THREADS
	ptrdiff_t lock_offset;
#endif /* !CONFIG_NO_THREADS */
	ptrdiff_t slot_offset;
	ASSERT(self->fg_state->ms_stackc >= 1);

	/* When optimizing for size, generate a (smaller) call to
	 * `DeeInstance_GetMember()', instead of inlining the function. */
	if (self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE) {
		DO(fg_vpush_const(self, type)); /* this, type */
		DO(fg_vswap(self));             /* type, this */
		DO(fg_vpush_imm16(self, addr)); /* type, this, addr */
		return fg_vcallapi(self, &DeeInstance_GetMember, VCALL_CC_OBJECT, 3);
	}

#ifndef CONFIG_NO_THREADS
	lock_offset = desc->cd_offset + offsetof(struct instance_desc, id_lock);
#endif /* !CONFIG_NO_THREADS */
	slot_offset = _Dee_MallococBufsize(desc->cd_offset + offsetof(struct instance_desc, id_vtab),
	                                   addr, sizeof(DREF DeeObject *));
	/* XXX: (and this is a problem with the normal executor): assert that "this" is an instance of `type' */

	/* TODO: In case of reading members, if one of the next instructions also does a read,
	 *       keep the lock acquired. The same should also go when it comes to accessing
	 *       global/extern variables. */
#ifndef CONFIG_NO_THREADS
	DO(fg_vdelta(self, lock_offset)); /* &this->[...].id_lock */
	if (flags & FG_CIMEMBER_F_REF)
		DO(fg_grwlock_read(self, fg_vtopdloc(self)));
	if (flags & FG_CIMEMBER_F_REF)
		DO(fg_vdup(self));                        /* [&this->[...].id_lock], &this->[...].id_lock */
	DO(fg_vind(self, slot_offset - lock_offset)); /* [&this->[...].id_lock], value */
#else /* !CONFIG_NO_THREADS */
	DO(fg_vind(self, slot_offset)); /* &this->[...].VALUE */
#endif /* CONFIG_NO_THREADS */
	DO(fg_vreg(self, NULL)); /* [&this->[...].id_lock], reg:value */

	/* Assert that the member is bound */
	text = fg_gettext(self);
	cold = fg_getcold(self);
	if unlikely(!cold)
		goto err;
	if (cold == text) {
		struct host_symbol *text_Lbound;
		text_Lbound = fg_newsym_named(self, ".Lbound");
		if unlikely(!text_Lbound)
			goto err;
		DO(fg_gjnz(self, fg_vtopdloc(self), text_Lbound));
		saved_state = self->fg_state;
		memstate_incref(saved_state);
		EDO(err_saved_state, fg_state_dounshare(self));
#ifndef CONFIG_NO_THREADS
		if (flags & FG_CIMEMBER_F_REF)
			EDO(err_saved_state, fg_grwlock_endread(self, fg_vtopdloc(self) - 1));
#endif /* !CONFIG_NO_THREADS */
		EDO(err_saved_state, fg_gunbound_instance_member(self, type, addr));
		host_symbol_setsect(text_Lbound, text);
	} else {
		struct host_symbol *cold_Lunbound_member;
		cold_Lunbound_member = fg_newsym_named(self, ".Lunbound_member");
		if unlikely(!cold_Lunbound_member)
			goto err;
		DO(fg_gjz(self, fg_vtopdloc(self), cold_Lunbound_member));
		saved_state = self->fg_state;
		memstate_incref(saved_state);
		HA_printf(".section .cold\n");
		EDO(err_saved_state, fg_settext(self, cold));
		host_symbol_setsect(cold_Lunbound_member, cold);
		EDO(err_saved_state, fg_state_dounshare(self));
#ifndef CONFIG_NO_THREADS
		if (flags & FG_CIMEMBER_F_REF)
			EDO(err_saved_state, fg_grwlock_endread(self, fg_vtopdloc(self) - 1));
#endif /* !CONFIG_NO_THREADS */
		EDO(err_saved_state, fg_gunbound_instance_member(self, type, addr));
		HA_printf(".section .text\n");
		EDO(err_saved_state, fg_settext(self, text));
	}
	memstate_decref(self->fg_state);
	self->fg_state = saved_state;
	if (flags & FG_CIMEMBER_F_REF) {
		DO(fg_vref_noalias(self)); /* &this->[...].id_lock, ref:value */
#ifndef CONFIG_NO_THREADS
		DO(fg_vswap(self));                              /* ref:value, &this->[...].id_lock */
		DO(fg_grwlock_endread(self, fg_vtopdloc(self))); /* ref:value, &this->[...].id_lock */
		DO(fg_vpop(self));                               /* ref:value */
#endif /* !CONFIG_NO_THREADS */
	}
	return 0;
err_saved_state:
	memstate_decref(saved_state);
err:
	return -1;
}

/* this, type -> value */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_imember(struct fungen *__restrict self,
                 uint16_t addr, unsigned int flags) {
	struct memval *type_mval;
	bool safe = ((self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_SAFE) ||
	             (flags & FG_CIMEMBER_F_SAFE));
	DO(fg_vdirect(self, 2));

	/* We only allow inline-resolving of this-member accesses when safe is disabled.
	 * This is because in code with user-assembly *any* type can be used together
	 * with the "this" argument, meaning that we *always* have to check that "this"
	 * actually implements the type *at runtime*.
	 *
	 * This differs when accessing class members, since for those no "this" argument
	 * is needed and the entire access can be performed immediately, no matter if
	 * safe semantics are required or not. */
	type_mval = fg_vtop(self);
	if (memval_direct_isconst(type_mval) && !safe) {
		struct class_desc *desc;
		DeeTypeObject *class_type = (DeeTypeObject *)memval_const_getobj(type_mval);
		if (DeeObject_AssertType(class_type, &DeeType_Type))
			goto err;
		if (!DeeType_IsClass(class_type))
			return libhostasm_rt_err_requires_class(class_type);
		desc = DeeClass_DESC(class_type);
		if (addr >= desc->cd_desc->cd_imemb_size)
			return libhostasm_rt_err_invalid_instance_addr(class_type, addr);
		DO(fg_vpop(self)); /* this */

		/* Push the member at runtime, but allowed to use (normally) unsafe
		 * semantics since we were already able to verify all constraints. */
		return fg_vpush_imember_unsafe_at_runtime(self, class_type, addr, flags);
	}

	/* Fallback: access the class member at runtime */
	DO(fg_vswap(self));             /* type, self */
	DO(fg_vpush_imm16(self, addr)); /* type, self, addr */
	return fg_vcallapi(self,
	                   safe ? (void const *)&DeeInstance_GetMemberSafe
	                        : (void const *)&DeeInstance_GetMember,
	                   VCALL_CC_OBJECT, 3);
err:
	return -1;
}

/* this, type -> bound */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vbound_imember(struct fungen *__restrict self,
                  uint16_t addr, unsigned int flags) {
	bool safe = ((self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_SAFE) ||
	             (flags & FG_CIMEMBER_F_SAFE));
	DO(fg_vdirect(self, 2));
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
fg_vdel_or_pop_imember_unsafe_at_runtime(struct fungen *__restrict self,
                                         DeeTypeObject *type, uint16_t addr) {
	struct class_desc *desc = DeeClass_DESC(type);
#ifndef CONFIG_NO_THREADS
	ptrdiff_t lock_offset;
#endif /* !CONFIG_NO_THREADS */
	ptrdiff_t slot_offset;
	ASSERT(self->fg_state->ms_stackc >= 2);

	/* When optimizing for size, generate a (smaller) call to
	 * `DeeInstance_GetMember()', instead of inlining the function. */
	if (self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE) {
		struct memval *value_mval;
		DO(fg_vpush_const(self, type)); /* this, value, type */
		DO(fg_vrrot(self, 3));          /* type, this, value */
		DO(fg_vpush_imm16(self, addr)); /* type, this, value, addr */
		DO(fg_vswap(self));             /* type, this, addr, value */
		value_mval = fg_vtop(self);
		if (memval_isnull(value_mval)) {
			DO(fg_vpop(self)); /* type, this, addr */
			return fg_vcallapi(self, &DeeInstance_DelMember, VCALL_CC_INT, 3);
		}
		DO(fg_vnotoneref_at(self, 1)); /* type, this, addr, value */
		return fg_vcallapi(self, &DeeInstance_SetMember, VCALL_CC_INT, 4);
	} else {
		struct memval *value_mval;
		value_mval = fg_vtop(self);
		if (!memval_isnull(value_mval))
			DO(fg_vref2(self, 2)); /* this, ref:value */
	}

#ifndef CONFIG_NO_THREADS
	lock_offset = desc->cd_offset + offsetof(struct instance_desc, id_lock);
#endif /* !CONFIG_NO_THREADS */
	slot_offset = _Dee_MallococBufsize(desc->cd_offset + offsetof(struct instance_desc, id_vtab),
	                                   addr, sizeof(DREF DeeObject *));
	/* XXX: (and this is a problem with the normal executor): assert that "this" is an instance of `type' */

#ifndef CONFIG_NO_THREADS
	DO(fg_vswap(self));                               /* ref:value, this */
	DO(fg_vdelta(self, lock_offset));                 /* ref:value, &this->[...].id_lock */
	DO(fg_grwlock_write(self, fg_vtopdloc(self)));    /* ... */
	DO(fg_vdup(self));                                /* ref:value, &this->[...].id_lock, &this->[...].id_lock */
	DO(fg_vdelta(self, slot_offset - lock_offset));   /* ref:value, &this->[...].id_lock, &this->[...].VALUE */
	DO(fg_vlrot(self, 3));                            /* &this->[...].id_lock, &this->[...].VALUE, ref:value */
	DO(fg_vswapind(self, 0));                         /* &this->[...].id_lock, old_value */
	DO(fg_vswap(self));                               /* old_value, &this->[...].id_lock */
	DO(fg_grwlock_endwrite(self, fg_vtopdloc(self))); /* old_value, &this->[...].id_lock */
	DO(fg_vpop(self));                                /* old_value */
#else /* !CONFIG_NO_THREADS */
	DO(fg_vswapind(self, slot_offset)); /* old_value */
#endif /* CONFIG_NO_THREADS */
	ASSERT(!fg_vtop_direct_isref(self));
	DO(fg_gxdecref_loc(self, fg_vtopdloc(self), 1));
	ASSERT(!fg_vtop_direct_isref(self));
	return fg_vpop(self);
err:
	return -1;
}

/* this, type, value -> N/A */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpop_imember(struct fungen *__restrict self,
                uint16_t addr, unsigned int flags) {
	bool safe = ((self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_SAFE) ||
	             (flags & FG_CIMEMBER_F_SAFE));
	struct memval *mval;
	DO(fg_vdirect(self, 3));

	/* We only allow inline-resolving of this-member accesses when safe is disabled.
	 * This is because in code with user-assembly *any* type can be used together
	 * with the "this" argument, meaning that we *always* have to check that "this"
	 * actually implements the type *at runtime*.
	 *
	 * This differs when accessing class members, since for those no "this" argument
	 * is needed and the entire access can be performed immediately, no matter if
	 * safe semantics are required or not. */
	mval = fg_vtop(self) - 1;
	if (memval_direct_isconst(mval) && !safe) {
		struct class_desc *desc;
		DeeTypeObject *class_type = (DeeTypeObject *)memval_const_getobj(mval);
		if (DeeObject_AssertType(class_type, &DeeType_Type))
			goto err;
		if (!DeeType_IsClass(class_type))
			return libhostasm_rt_err_requires_class(class_type);
		desc = DeeClass_DESC(class_type);
		if (addr >= desc->cd_desc->cd_imemb_size)
			return libhostasm_rt_err_invalid_instance_addr(class_type, addr);
		DO(fg_vpop_at(self, 2)); /* this, value */

		/* Push the member at runtime, but allowed to use (normally) unsafe
		 * semantics since we were already able to verify all constraints. */
		return fg_vdel_or_pop_imember_unsafe_at_runtime(self, class_type, addr);
	}

	/* Fallback: access the class member at runtime */
	DO(fg_vswap(self));             /* this, value, type */
	DO(fg_vrrot(self, 3));          /* type, this, value */
	DO(fg_vpush_imm16(self, addr)); /* type, this, value, addr */
	DO(fg_vswap(self));             /* type, this, addr, value */
	mval = fg_vtop(self);
	if (memval_isnull(mval)) {
		DO(fg_vpop(self)); /* type, this, addr */
		return fg_vcallapi(self,
		                   safe ? (void const *)&DeeInstance_DelMemberSafe
		                        : (void const *)&DeeInstance_DelMember,
		                   VCALL_CC_INT, 3);
	}
	DO(fg_vnotoneref_at(self, 1)); /* type, this, addr, value */
	return fg_vcallapi(self,
	                   safe ? (void const *)&DeeInstance_SetMemberSafe
	                        : (void const *)&DeeInstance_SetMember,
	                   VCALL_CC_INT, 4);
err:
	return -1;
}

/* this, type -> N/A */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vdel_imember(struct fungen *__restrict self,
                uint16_t addr, unsigned int flags) {
	int result = fg_vpush_addr(self, NULL);
	if likely(result == 0)
		result = fg_vpop_imember(self, addr, flags);
	return result;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
reclaim_unused_stack_space(struct fungen *__restrict self) {
	struct memstate *state = self->fg_state;
	host_cfa_t min_cfa = memstate_hstack_greatest_inuse(state);
	ptrdiff_t free_space;
#ifndef HOSTASM_STACK_GROWS_DOWN
	min_cfa += HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
	free_space = state->ms_host_cfa_offset - min_cfa;

	/* Only free space if there is more free space than
	 * needed for doing an unaligned call to another function. */
	if (free_space > HOSTASM_SCRACHAREA_SIZE + HOSTASM_STACK_ALIGNMENT - HOST_SIZEOF_POINTER)
		return fg_ghstack_adjust(self, -free_space);
	return 0;
}

/* test_type, extended_type -> DeeType_Extends(test_type, extended_type) */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcall_DeeType_Extends(struct fungen *__restrict self) {
	struct memval *extended_type_val;
	DeeTypeObject *extended_type;

	/* When "extended_type" is constant and a FINAL type,
	 * can encode as "test_type === extended_type" */
	DO(fg_vdirect(self, 2)); /* test_type, extended_type */
	extended_type_val = fg_vtop(self);
	if (memval_isconst(extended_type_val)) {
		struct memval *test_type_val;
		extended_type = (DeeTypeObject *)memval_const_getobj(extended_type_val);
		if (!DeeType_Check(extended_type))
			goto always_returns_false;
		if (DeeType_IsFinal(extended_type)) {
			/* TODO: Need a special vmorph here: >> value = (objv[0] == objv[1]) ? <non-zero> : 0 */
		}
		test_type_val = fg_vtop(self) - 1;
		if (memval_isconst(test_type_val)) {
			DeeTypeObject *test_type = (DeeTypeObject *)memval_const_getobj(test_type_val);
			unsigned int does_implement = DeeType_Extends(test_type, extended_type);
			DO(fg_vpopmany(self, 2));
			return fg_vpush_immINT(self, does_implement);
		}
	}
	extended_type = memval_typeof(extended_type_val);
	if (extended_type != NULL && !DeeType_IsTypeType(extended_type)) {
		/* Checked type can't possibly be implemented. */
always_returns_false:
		DO(fg_vpopmany(self, 2));
		return fg_vpush_immINT(self, 0);
	}
	return fg_vcallapi(self, &DeeType_Extends, VCALL_CC_RAWINT, 2);
err:
	return -1;
}

/* test_type, implemented_type -> DeeType_Implements(test_type, implemented_type) */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcall_DeeType_Implements(struct fungen *__restrict self) {
	struct memval *implemented_type_val;
	DeeTypeObject *implemented_type;

	/* When "implemented_type" is constant and non-ABSTRACT type,
	 * can encode as `fg_vcall_DeeType_Extends()' */
	DO(fg_vdirect(self, 2)); /* test_type, implemented_type */
	implemented_type_val = fg_vtop(self);
	if (memval_isconst(implemented_type_val)) {
		struct memval *test_type_val;
		implemented_type = (DeeTypeObject *)memval_const_getobj(implemented_type_val);
		if (!DeeType_Check(implemented_type))
			goto always_returns_false;
		if (!DeeType_IsAbstract(implemented_type))
			return fg_vcall_DeeType_Extends(self);
		test_type_val = fg_vtop(self) - 1;
		if (memval_isconst(test_type_val)) {
			DeeTypeObject *test_type = (DeeTypeObject *)memval_const_getobj(test_type_val);
			unsigned int does_implement = DeeType_Implements(test_type, implemented_type);
			DO(fg_vpopmany(self, 2));
			return fg_vpush_immINT(self, does_implement);
		}
	}
	implemented_type = memval_typeof(implemented_type_val);
	if (implemented_type != NULL && !DeeType_IsTypeType(implemented_type)) {
		/* Checked type can't possibly be implemented. */
always_returns_false:
		DO(fg_vpopmany(self, 2));
		return fg_vpush_immINT(self, 0);
	}
	return fg_vcallapi(self, &DeeType_Implements, VCALL_CC_RAWINT, 2);
err:
	return -1;
}


/* obj, type -> N/A */
PRIVATE WUNUSED NONNULL((1)) int DCALL
impl_vcall_DeeObject_AssertType(struct fungen *__restrict self) {
	struct fg_branch branch;
	if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE)) {
		/* Emit code equivalent to:
		 * >> if (!DeeType_Extends(Dee_TYPE(obj), type)) {
		 * >>     DeeObject_TypeAssertFailed(obj, type);
		 * >>     HANDLE_EXCEPT();
		 * >> } */
		DO(fg_vdup_at(self, 2));                         /* obj, type, obj */
		DO(fg_vind(self, offsetof(DeeObject, ob_type))); /* obj, type, obj->ob_type */
		DO(fg_vdup_at(self, 2));                         /* obj, type, obj->ob_type, type */
		DO(fg_vcall_DeeType_Extends(self));              /* obj, type, extends */
		DO(fg_vjz_enter_unlikely(self, &branch));        /* obj, type */
		EDO(err_branch, fg_vcall_DeeObject_TypeAssertFailed(self)); /* N/A */
		DO(fg_vjx_leave_noreturn(self, &branch));        /* obj, type */
		return fg_vpopmany(self, 2);                     /* N/A */
	}
	return fg_vcallapi(self, &DeeObject_AssertType, VCALL_CC_INT, 2);
err_branch:
	fg_branch_fini(&branch);
err:
	return -1;
}

/* obj, type -> N/A */
PRIVATE WUNUSED NONNULL((1)) int DCALL
impl_vcall_DeeObject_AssertTypeOrAbstract(struct fungen *__restrict self) {
	struct fg_branch b1, b2;
	if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE)) {
		/* Emit code equivalent to:
		 * >> if (!DeeType_IsAbstract(type)) {
		 * >>     if (!DeeType_Extends(Dee_TYPE(obj), type)) {
		 * >>         DeeObject_TypeAssertFailed(obj, type);
		 * >>         HANDLE_EXCEPT();
		 * >>     }
		 * >> } */
		DO(fg_vdup(self));                                                    /* obj, type, type */
		DO(fg_vind(self, offsetof(DeeTypeObject, tp_flags)));                 /* obj, type, type->tp_flags */
		DO(fg_vpush_immSIZ(self, DeeTypeObject_tp_flags_FLAG(TP_FABSTRACT))); /* obj, type, type->tp_flags, TP_FABSTRACT */
		DO(fg_vjaz_enter(self, &b1));                                         /* obj, type */
		EDO(err_b1, fg_vdup_at(self, 2));                                     /* obj, type, obj */
		EDO(err_b1, fg_vind(self, offsetof(DeeObject, ob_type)));             /* obj, type, obj->ob_type */
		EDO(err_b1, fg_vdup_at(self, 2));                                     /* obj, type, obj->ob_type, type */
		EDO(err_b1, fg_vcall_DeeType_Extends(self));                          /* obj, type, extends */
		EDO(err_b1, fg_vjz_enter_unlikely(self, &b2));                        /* obj, type */
		EDO(err_b1_b2, fg_vcall_DeeObject_TypeAssertFailed(self));            /* N/A */
		EDO(err_b1, fg_vjx_leave_noreturn(self, &b2));                        /* obj, type */
		DO(fg_vjx_leave(self, &b1));                                          /* obj, type */
		return fg_vpopmany(self, 2);                                          /* N/A */
	}
	return fg_vcallapi(self, &DeeObject_AssertType, VCALL_CC_INT, 2);
err_b1_b2:
	fg_branch_fini(&b2);
err_b1:
	fg_branch_fini(&b1);
err:
	return -1;
}

/* obj, type -> N/A */
PRIVATE WUNUSED NONNULL((1)) int DCALL
impl_vcall_DeeObject_AssertTypeExact(struct fungen *__restrict self) {
	struct fg_branch branch;
	if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE)) {
		/* Emit code equivalent to:
		 * >> if (Dee_TYPE(obj) != type) {
		 * >>     DeeObject_TypeAssertFailed(obj, type);
		 * >>     HANDLE_EXCEPT();
		 * >> } */
		DO(fg_vdup_at(self, 2));                         /* obj, type, obj */
		DO(fg_vind(self, offsetof(DeeObject, ob_type))); /* obj, type, obj->ob_type */
		DO(fg_vdup_at(self, 2));                         /* obj, type, obj->ob_type, type */
		DO(fg_vjne_enter_unlikely(self, &branch));       /* obj, type */
		EDO(err_branch, fg_vcall_DeeObject_TypeAssertFailed(self)); /* N/A */
		DO(fg_vjx_leave_noreturn(self, &branch));        /* obj, type */
		return fg_vpopmany(self, 2);                     /* N/A */
	}
	return fg_vcallapi(self, &DeeObject_AssertTypeExact, VCALL_CC_INT, 2);
err_branch:
	fg_branch_fini(&branch);
err:
	return -1;
}



/* obj -> obj */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vcall_DeeObject_AssertType_c(struct fungen *__restrict self,
                                DeeTypeObject *__restrict type) {
	DeeTypeObject *vtop_type;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	vtop_type = memval_typeof(fg_vtop(self));
	if (vtop_type != NULL) {
		if (DeeType_Extends(vtop_type, type))
			return 0;
		if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_NOEARLYERR)) {
			return DeeError_Throwf(&DeeError_TypeError,
			                       "Expected instance of `%r', but got a `%r' object",
			                       type, vtop_type);
		}
	}
	DO(fg_vdup(self));                            /* value, value */
	DO(fg_vpush_const(self, type));               /* value, value, type */
	return impl_vcall_DeeObject_AssertType(self); /* value */
err:
	return -1;
}

/* obj -> obj */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vcall_DeeObject_AssertTypeOrAbstract_c(struct fungen *__restrict self,
                                          DeeTypeObject *__restrict type) {
	if (DeeType_IsAbstract(type))
		return 0; /* Special case: abstract types don't need to be checked! */
	return fg_vcall_DeeObject_AssertType_c(self, type);
}

/* obj -> obj */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vcall_DeeObject_AssertTypeExact_c(struct fungen *__restrict self,
                                     DeeTypeObject *__restrict type) {
	DeeTypeObject *vtop_type;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	vtop_type = memval_typeof(fg_vtop(self));
	if (vtop_type != NULL) {
		if (vtop_type == type)
			return 0;
		if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_NOEARLYERR)) {
			return DeeError_Throwf(&DeeError_TypeError,
			                       "Expected exact instance of `%r', but got a `%r' object",
			                       type, vtop_type);
		}
	}
	DO(fg_vdup(self));                              /* value, value */
	DO(fg_vpush_const(self, type));                 /* value, value, type */
	DO(impl_vcall_DeeObject_AssertTypeExact(self)); /* value */
	return fg_vsettyp(self, type);                  /* value */
err:
	return -1;
}

/* obj, type -> N/A */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcall_DeeObject_AssertType(struct fungen *__restrict self) {
	struct memval *typeval;
	DeeTypeObject *type;
	DO(fg_vdirect(self, 2)); /* obj, type */
	typeval = fg_vtop(self);
	if (memval_isconst(typeval)) {
		type = (DeeTypeObject *)memval_const_getobj(typeval);
		DO(fg_vpop(self));                               /* obj */
		DO(fg_vcall_DeeObject_AssertType_c(self, type)); /* obj */
		return fg_vpop(self);                            /* N/A */
	}
	type = memval_typeof(typeval);
	if (type != NULL && !DeeType_IsTypeType(type)) {
		/* Assertion always fails because "typeval" isn't a type. */
		if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_NOEARLYERR)) {
			return DeeError_Throwf(&DeeError_TypeError,
			                       "Type assertion `? is instance of %r' always fails because %r isn't a type-type",
			                       type, type);
		}
		return fg_vcall_DeeObject_TypeAssertFailed(self);
	}
	return impl_vcall_DeeObject_AssertType(self);
err:
	return -1;
}

/* obj, type -> N/A */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcall_DeeObject_AssertTypeOrAbstract(struct fungen *__restrict self) {
	struct memval *typeval;
	DeeTypeObject *type;
	DO(fg_vdirect(self, 2)); /* obj, type */
	typeval = fg_vtop(self);
	if (memval_isconst(typeval)) {
		type = (DeeTypeObject *)memval_const_getobj(typeval);
		DO(fg_vpop(self));                                         /* obj */
		DO(fg_vcall_DeeObject_AssertTypeOrAbstract_c(self, type)); /* obj */
		return fg_vpop(self);                                      /* N/A */
	}
	type = memval_typeof(typeval);
	if (type != NULL && !DeeType_IsTypeType(type)) {
		/* Assertion always fails because "typeval" isn't a type. */
		if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_NOEARLYERR)) {
			return DeeError_Throwf(&DeeError_TypeError,
			                       "Type assertion `? is instance of %r or abstract' always fails because %r isn't a type-type",
			                       type, type);
		}
		return fg_vcall_DeeObject_TypeAssertFailed(self);
	}
	return impl_vcall_DeeObject_AssertTypeOrAbstract(self);
err:
	return -1;
}

/* obj, type -> N/A */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcall_DeeObject_AssertTypeExact(struct fungen *__restrict self) {
	struct memval *typeval;
	DeeTypeObject *type;
	DO(fg_vdirect(self, 2)); /* obj, type */
	typeval = fg_vtop(self);
	if (memval_direct_isconst(typeval)) {
		type = (DeeTypeObject *)memval_const_getobj(typeval);
		DO(fg_vpop(self));                                    /* obj */
		DO(fg_vcall_DeeObject_AssertTypeExact_c(self, type)); /* obj */
		return fg_vpop(self);                                 /* N/A */
	}
	type = memval_typeof(typeval);
	if (type != NULL && !DeeType_IsTypeType(type)) {
		/* Assertion always fails because "typeval" isn't a type. */
		if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_NOEARLYERR)) {
			return DeeError_Throwf(&DeeError_TypeError,
			                       "Type assertion `? is exact instance of %r' always fails because %r isn't a type-type",
			                       type, type);
		}
		return fg_vcall_DeeObject_TypeAssertFailed(self);
	}
	return impl_vcall_DeeObject_AssertTypeExact(self);
err:
	return -1;
}

/* obj, type -> N/A */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcall_DeeObject_TypeAssertFailed(struct fungen *__restrict self) {
	return fg_vcallapi(self, &DeeObject_TypeAssertFailed, VCALL_CC_EXCEPT, 2);
}




/* Perform a conditional jump to `desc' based on `jump_if_true'
 * @param: instr: Pointer to start of deemon jmp-instruction (for bb-truncation, and error message)
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vjcc(struct fungen *__restrict self,
        struct jump_descriptor *desc,
        Dee_instruction_t const *instr, bool jump_if_true) {
	struct host_symbol *Ljmp;
	int bool_status;
	struct basic_block *target = desc->jd_to;
	struct memval *cond_mval;
#ifdef DEE_HOST_RELOCVALUE_SECT
	struct host_symbol _Ljmp;
#endif /* DEE_HOST_RELOCVALUE_SECT */
	DO(fg_state_unshare(self));

	/* TODO: If this jump might be the result of infinite loops,
	 *       must emit a call to `DeeThread_CheckInterrupt()' */

	bool_status = fg_vopbool(self, VOPBOOL_F_NOFALLBACK | VOPBOOL_F_FORCE_MORPH);
	if unlikely(bool_status < 0)
		goto err; /* Force vtop into a bool constant, or a MEMVAL_VMORPH_ISBOOL-style morph */
	cond_mval = fg_vtop(self);

	/* Special case for when the top-element is a constant. */
	if (memval_isconst(cond_mval)) {
		ASSERT(DeeBool_Check(memval_const_getobj(cond_mval)));
		if (memval_const_getobj(cond_mval) != Dee_False) {
			/* Unconditional jump -> the block ends here and falls into the next one */
			self->fg_block->bb_next       = target;
			self->fg_block->bb_deemon_end = instr; /* The jump doesn't exist anymore now! */
		}
		DO(fg_vpop(self));
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
	host_symbol_initcommon_named(&_Ljmp, ".Ljmp");
	Ljmp = &_Ljmp;
#else /* DEE_HOST_RELOCVALUE_SECT */
	Ljmp = fg_newsym_named(self, ".Ljmp");
	if unlikely(!Ljmp)
		goto err;
#endif /* !DEE_HOST_RELOCVALUE_SECT */
	host_symbol_setjump(Ljmp, desc);

	/* Check for special case: `fg_vopbool()' needed to do its fallback operation.
	 * Handle this case by doing the call to `DeeObject_Bool()' ourselves, so we can combine the bool
	 * branch with the except branch, thus saving on a couple of otherwise redundant instructions. */
	if (bool_status > 0) {
		struct memloc cond_mloc;
		bool hasbool;
		DeeTypeObject *loctype;
		struct memloc zero;
		struct host_symbol *Lexcept, *Lnot_except;
		struct host_section *text, *cold;
#ifdef DEE_HOST_RELOCVALUE_SECT
		struct host_symbol _Lexcept;
		host_symbol_initcommon_named(&_Lexcept, ".Lexcept");
		Lexcept = &_Lexcept;
#else /* DEE_HOST_RELOCVALUE_SECT */
		Lexcept = fg_newsym_named(self, ".Lexcept");
		if unlikely(!Lexcept)
			goto err;
#endif /* !DEE_HOST_RELOCVALUE_SECT */
		ASSERT(cond_mval == fg_vtop(self));
		DO(fg_vnotoneref_if_operator(self, OPERATOR_BOOL, 1));
		ASSERT(cond_mval == fg_vtop(self));

		loctype = memval_typeof(cond_mval);
		hasbool = loctype && DeeType_InheritOperator(loctype, OPERATOR_BOOL);
		ASSERT(!hasbool || (loctype->tp_cast.tp_bool != NULL));
		DO(fg_vcallapi(self,
		               hasbool ? (void const *)loctype->tp_cast.tp_bool
		                       : (void const *)&DeeObject_Bool,
		               VCALL_CC_RAWINT, 1));

		/* Silently remove the bool-morph location from the v-stack. */
		cond_mval = fg_vtop(self);
		ASSERT(memval_isdirect(cond_mval));
		ASSERT(self->fg_state->ms_stackc >= 1);
		cond_mloc = *memval_direct_getloc(cond_mval);
		memstate_decrinuse_for_memloc(self->fg_state, &cond_mloc);
		memval_direct_fini(cond_mval);
		--self->fg_state->ms_stackc;

		/* Figure out how to do exception handling. */
		Lnot_except = NULL;
		text = fg_gettext(self);
		cold = fg_getcold(self);
		if unlikely(!cold)
			goto err;
		if (self->fg_exceptinject != NULL) {
			/* Prepare stuff so we can inject custom exception handling code. */
			if (text == cold) {
				Lnot_except = Lexcept;
				Lexcept     = NULL;
				host_symbol_setname(Lnot_except, ".Lnot_except");
			}
		}

		/* Generate code to branch depending on the value of `loc' */
		memloc_init_const(&zero, (void *)0);
		DO(fg_gjcc(self, &cond_mloc, &zero, true,
		           Lexcept,                             /* loc < 0 */
		           jump_if_true ? Lnot_except : Ljmp,   /* loc == 0 */
		           jump_if_true ? Ljmp : Lnot_except)); /* loc > 0 */

		if (self->fg_exceptinject != NULL) {
			/* Must inject custom exception handling code! */
			ASSERT((Lnot_except == NULL) || (Lnot_except == NULL));
			ASSERT((Lnot_except != NULL) || (Lnot_except != NULL));
			if (Lnot_except) {
				ASSERT(!Lexcept);
				ASSERT(!host_symbol_isdefined(Lnot_except));
				DO(fg_gjmp_except(self));
				host_symbol_setsect(Lnot_except, fg_gettext(self));
			} else {
				ASSERT(Lexcept);
				ASSERT(!host_symbol_isdefined(Lexcept));
				ASSERT(text != cold);
				HA_printf(".section .cold\n");
				DO(fg_settext(self, cold));
				host_symbol_setsect(Lexcept, cold);
				DO(fg_gjmp_except(self));
				HA_printf(".section .text\n");
				DO(fg_settext(self, text));
			}
		} else {
			struct except_exitinfo *except_exit;
			except_exit = fg_except_exit(self);
			if unlikely(!except_exit)
				goto err;
			host_symbol_setsect_ex(Lexcept, &except_exit->exi_text, 0);
		}
	} else {
		struct memloc cmp_lhs, cmp_rhs;
		struct host_symbol *Llo, *Leq, *Lgr;

		/* In this case, `fg_vopbool()' already created a morph. */
		ASSERT(MEMVAL_VMORPH_ISBOOL(cond_mval->mv_vmorph));
		ASSERT(!memobj_isref(&cond_mval->mv_obj.mvo_0));

		/* Silently remove the bool-morph location from the v-stack. */
		ASSERT(self->fg_state->ms_stackc >= 1);
		/*memval_fini(cond_mval);*/ /* Not needed for `MEMVAL_VMORPH_ISBOOL()' */
		memstate_decrinuse_for_memobj(self->fg_state, &cond_mval->mv_obj.mvo_0);
		--self->fg_state->ms_stackc;

		/* Compute compare operands and target labels. */
		Llo = NULL;
		Leq = NULL;
		Lgr = NULL;
		cmp_lhs = *memobj_getloc(&cond_mval->mv_obj.mvo_0);
		memloc_init_const(&cmp_rhs, NULL);
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
			if (memloc_getoff(&cmp_lhs) == -1) {
				memloc_setoff(&cmp_lhs, 0);
				Leq = Ljmp;
			}
			break;
		case MEMVAL_VMORPH_BOOL_GZ:
			Lgr = Ljmp;
			/* (X+1) > 0   <=>   X >= 0 */
			if (memloc_getoff(&cmp_lhs) == 1) {
				memloc_setoff(&cmp_lhs, 0);
				Leq = Ljmp;
			}
			break;
		default: __builtin_unreachable();
		}
		memval_direct_fini(cond_mval);

		if (!jump_if_true) {
			/* Invert the logical meaning of the jump. */
			struct host_symbol *temp;
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
		DO(fg_gjcc(self, &cmp_lhs, &cmp_rhs, true, Llo, Leq, Lgr));
	}

	/* Remember the memory-state as it is when the jump is made. */
assign_desc_stat:
	ASSERTF(!desc->jd_stat, "Who assigned this? Doing that is *my* job!");
	desc->jd_stat = self->fg_state;
	memstate_incref(self->fg_state);

	bool_status = basic_block_constrainwith(target, desc->jd_stat,
	                                        function_assembler_addrof(self->fg_assembler,
	                                                                  target->bb_deemon_start));
	if (bool_status > 0) {
		bool_status = 0;
		if (target == self->fg_block) {
			/* Special case for when the block that's currently being
			 * compiled had to constrain itself a little further. */
			ASSERT(target->bb_htext.hs_end == target->bb_htext.hs_start);
			ASSERT(target->bb_htext.hs_relc == 0);
			ASSERT(target->bb_mem_end == NULL);
			target->bb_mem_end = (DREF struct memstate *)-1;
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
fg_vforeach(struct fungen *__restrict self,
            struct jump_descriptor *desc,
            bool always_pop_iterator) {
	int temp;
	struct memobj decref_on_iter_done;
	struct memval *mval;
	DREF struct memstate *desc_state;
	struct host_symbol *sym;
	struct basic_block *target = desc->jd_to;
	DO(fg_state_unshare(self));
	DO(fg_vdirect1(self));                                     /* iter */
	DO(fg_vnotoneref_if_operator(self, OPERATOR_ITERNEXT, 1)); /* iter */
	if (!always_pop_iterator)
		DO(fg_vdup(self)); /* iter, iter */
	DO(fg_vcallapi(self, &DeeObject_IterNext, VCALL_CC_RAWINTPTR, 1)); /* [if(!always_pop_iterator) iter], UNCHECKED(elem) */

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
	ASSERT(fg_vtop_isdirect(self));
	DO(fg_gjz_except(self, fg_vtopdloc(self)));

	/* >> if (elem == ITER_DONE) goto <desc>; */
	sym = fg_newsym(self);
	if unlikely(!sym)
		goto err;
	host_symbol_setjump(sym, desc);
	{
		struct memloc iter_done;
		memloc_init_const(&iter_done, ITER_DONE);
		DO(fg_gjcc(self, fg_vtopdloc(self),
		           &iter_done, false, NULL, sym, NULL));
	}
	DO(fg_state_unshare(self));

	/* Remember the memory-state as it is when the jump is made. */
	ASSERTF(!desc->jd_stat, "Who assigned this? Doing that is *my* job!");
	desc_state = memstate_copy(self->fg_state);
	if unlikely(!desc_state)
		goto err;
	ASSERT(desc_state->ms_stackc >= 1);
	mval = memstate_vtop(desc_state);
	--desc_state->ms_stackc; /* Get rid of `UNCHECKED(result)' */
	ASSERT(memval_isdirect(mval));
	memstate_decrinuse_for_memloc(desc_state, memval_direct_getloc(mval));
	memval_direct_fini(mval);
	memobj_init_local_unbound(&decref_on_iter_done);
	if (!always_pop_iterator) {
		/* Pop another vstack item (the iterator) and store it in `MEMSTATE_XLOCAL_POPITER'.
		 * When the time comes to generate morph-code, the iterator will then be decref'd. */
		ASSERT(desc_state->ms_stackc >= 1);
		mval = memstate_vtop(desc_state);
		--desc_state->ms_stackc;
		ASSERT(memval_isdirect(mval));
		decref_on_iter_done = *memval_direct_getobj(mval);
		memval_direct_fini(mval);
		memstate_decrinuse_for_memobj(desc_state, &decref_on_iter_done);
	}
	desc->jd_stat = desc_state; /* Inherit reference */

	/* Adjust out own current state to make the top-item (i.e. the "elem") to become a reference */
	ASSERT(self->fg_state != desc_state);
	ASSERT(!memstate_isshared(self->fg_state));
	ASSERT(!fg_vtop_direct_isref(self));
	fg_vtop_direct_setref(self);

	/* Constrain the jump-target block with the mem-state from the descriptor. */
	temp = basic_block_constrainwith(target, desc_state,
	                                 function_assembler_addrof(self->fg_assembler,
	                                                           target->bb_deemon_start));
	if (temp > 0) {
		temp = 0;
		if (target == self->fg_block) {
			/* Special case for when the block that's currently being
			 * compiled had to constrain itself a little further. */
			ASSERT(target->bb_htext.hs_end == target->bb_htext.hs_start);
			ASSERT(target->bb_htext.hs_relc == 0);
			ASSERT(target->bb_mem_end == NULL);
			target->bb_mem_end = (DREF struct memstate *)-1;
		}
	}

	/* Do a little bit of black magic to drop the reference from the iterator
	 * as part of the morph done during the jump. */
	if (memobj_isref(&decref_on_iter_done)) {
		struct memval *popiter_mval;
		popiter_mval = &desc_state->ms_localv[self->fg_assembler->fa_localc + MEMSTATE_XLOCAL_POPITER];
		ASSERT(memval_isdirect(popiter_mval));
		ASSERT(memval_direct_local_neverbound(popiter_mval));
		memval_init_memobj_inherit(popiter_mval, &decref_on_iter_done);
	} else {
		memobj_fini(&decref_on_iter_done);
	}

	return temp;
err:
	return -1;
}

__pragma_GCC_diagnostic_pop_ignored(Wmaybe_uninitialized)



/* >> TOP = *(TOP + ind_delta); */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vind(struct fungen *__restrict self,
        ptrdiff_t ind_delta) {
	struct memstate *state;
	struct memval *mval;
	struct memloc ind_loc;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	DO(fg_state_unshare(self));
	state = self->fg_state;
	mval  = memstate_vtop(state);

	/* Special case when accessing "ob_type" */
	if (ind_delta == offsetof(DeeObject, ob_type)) {
		DeeTypeObject *typeof_mval = memval_typeof(mval);
		if (typeof_mval != NULL) {
			DO(fg_vpop(self));
			return fg_vpush_const(self, typeof_mval);
		}
	} else if (ind_delta == offsetof(DeeObject, ob_refcnt)) {
		switch (mval->mv_vmorph) {
		case MEMVAL_VMORPH_DIRECT:
		case MEMVAL_VMORPH_DIRECT_01:
			if (memobj_isoneref(memval_getobj0(mval))) {
				DO(fg_vpop(self));
				return fg_vpush_immSIZ(self, 1);
			}
			break;
		case MEMVAL_VMORPH_TUPLE:
		case MEMVAL_VMORPH_ROSET:
		case MEMVAL_VMORPH_RODICT:
			if (memval_getobjn(mval)->mos_objc == 0)
				break; /* In this case, a shared constant may be used (so ob_refcnt may be != 1) */
			ATTR_FALLTHROUGH
		case MEMVAL_VMORPH_LIST:
		case MEMVAL_VMORPH_HASHSET:
		case MEMVAL_VMORPH_DICT:
		case MEMVAL_VMORPH_SUPER:
			/* Memory value is the virtual representation of a freshly created object. */
			DO(fg_vpop(self));
			return fg_vpush_immSIZ(self, 1);
		default: break;
		}
	}

	/* Special handling for certain morphs */
	switch (mval->mv_vmorph) {

	case MEMVAL_VMORPH_LIST:
		if (ind_delta == offsetof(DeeListObject, l_list.ol_elemc)) {
			size_t i;
			struct memobjs *objs;
push_list_mval_objc:
			objs = memval_getobjn(mval);
			/* Size is only a known compile-time if no element is marked as _MEMOBJ_F_EXPAND */
			for (i = 0; i < objs->mos_objc; ++i) {
				if (objs->mos_objv[i].mo_flags & _MEMOBJ_F_EXPAND)
					goto no_vmorph_optimization;
			}
			DO(fg_vpush_immSIZ(self, objs->mos_objc)); /* self, rd_size */
			return fg_vpop_at(self, 2);                /* rd_size */
		}
		break;

	case MEMVAL_VMORPH_TUPLE:
		if (ind_delta == offsetof(DeeTupleObject, t_size))
			goto push_list_mval_objc;
		if (ind_delta >= offsetof(DeeTupleObject, t_elem)) {
			struct memobjs *objs = memval_getobjn(mval);
			if ((size_t)ind_delta < _Dee_MallococBufsize(offsetof(DeeTupleObject, t_elem),
			                                             objs->mos_objc, sizeof(DREF DeeObject *))) {
				size_t index = (size_t)ind_delta - (size_t)offsetof(DeeTupleObject, t_elem);
				if ((index % sizeof(DREF DeeObject *)) == 0) {
					index /= sizeof(DREF DeeObject *);
					DO(fg_vpush_memobj(self, &objs->mos_objv[index])); /* tuple, t_elem[index] */
					return fg_vpop_at(self, 2);                        /* t_elem[index] */
				}
			}
		}
		break;

	case MEMVAL_VMORPH_SUPER: {
		struct memobjs *objs = memval_getobjn(mval);
		ASSERT(objs->mos_objc == 2);
		if (ind_delta == offsetof(DeeSuperObject, s_self)) {
			DO(fg_vpush_memobj(self, &objs->mos_objv[0])); /* super, s_self */
			return fg_vpop_at(self, 2);                    /* s_self */
		} else if (ind_delta == offsetof(DeeSuperObject, s_type)) {
			DO(fg_vpush_memobj(self, &objs->mos_objv[1])); /* super, s_type */
			return fg_vpop_at(self, 2);                    /* s_type */
		}
	}	break;

	default: break;
	}

	/* Fallback: force direct value, and load object attribute. */
no_vmorph_optimization:
	DO(fg_vdirect1(self));
	state = self->fg_state;
	mval  = memstate_vtop(state);
	ASSERT(memval_isdirect(mval));
	ASSERTF(!memval_direct_isref(mval), "Cannot do indirection on location holding a reference");
	DO(fg_gasind(self, memval_direct_getloc(mval), &ind_loc, ind_delta));
	ASSERT(state == self->fg_state);
	ASSERT(mval == memstate_vtop(state));
	memstate_decrinuse_for_memloc(state, memval_direct_getloc(mval));
	*memval_direct_getloc(mval) = ind_loc;
	memstate_incrinuse_for_memloc(state, &ind_loc);
	memval_direct_settypeof(mval, NULL); /* Unknown */
	return 0;
err:
	return -1;
}

/* Remember that "TOP" is a dependency of "SECOND" */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vdep(struct fungen *__restrict self) {
	struct memval *this_object;
	struct memval *depends_on_this;
	struct memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < 2)
		return err_illegal_stack_effect();
	this_object     = memstate_vtop(state);
	depends_on_this = this_object - 1;

	/* You can only form dependencies to:
	 * - direct objects
	 * - that aren't constants
	 * - and that are holding references (or have an alias with a reference)
	 *
	 * In the case of vmorph objects, the caller must do their own handling,
	 * or rely on this default handling where such a dependency is a no-op. */
	if (memval_isdirect(depends_on_this) && !memval_direct_isconst(depends_on_this) &&
	    memstate_hasref(self->fg_state, memval_direct_getobj(depends_on_this))) {
		struct memobj *this_object_item;
		if unlikely(memstate_isshared(state)) {
			state = memstate_copy(state);
			if unlikely(!state)
				goto err;
			memstate_decref_nokill(self->fg_state);
			self->fg_state  = state;
			this_object     = memstate_vtop(state);
			depends_on_this = this_object - 1;
		}
		memval_foreach_obj(this_object_item, this_object) {
			if unlikely(memstate_dependency(self->fg_state, this_object_item,
			                                memval_direct_getobj(depends_on_this)))
				goto err;
		}
		memval_foreach_obj_end;
	}
	return 0;
err:
	return -1;
}

/* >> *(SECOND + ind_delta) = POP(); // NOTE: Ignores `mv_vmorph' in SECOND */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpopind(struct fungen *__restrict self, ptrdiff_t ind_delta) {
	struct memval *mval;
	struct memloc src, *dst;
	DO(fg_vdirect1(self)) /* !!! Only the value getting assigned is made direct! */;
	DO(fg_vnotoneref_at(self, 1));
	DO(fg_state_unshare(self));
	src = *fg_vtopdloc(self);
	memstate_decrinuse_for_memloc(self->fg_state, &src);
	mval = fg_vtop(self);
	--self->fg_state->ms_stackc;
	memval_direct_fini(mval);
	mval = fg_vtop(self);
	ASSERT(memval_hasobj0(mval));
	dst = memobj_getloc(memval_getobj0(mval));
	return fg_gmov_loc2locind(self, &src, dst, ind_delta);
err:
	return -1;
}

/* >> TOP = TOP + val_delta; // NOTE: Ignores `mv_vmorph' */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vdelta(struct fungen *__restrict self, ptrdiff_t val_delta) {
	struct memval *mval;
	if unlikely(val_delta == 0)
		return 0;
	DO(fg_state_unshare(self));
	mval = fg_vtop(self);
	ASSERT(memval_hasobj0(mval));
	ASSERTF(!memobj_isref(memval_getobj0(mval)),
	        "Cannot add delta to location holding a reference");
	memloc_adjoff(memobj_getloc(memval_getobj0(mval)), val_delta);
	mval->mv_vmorph = MEMVAL_VMORPH_DIRECT;
	memval_direct_settypeof(mval, NULL); /* Unknown */
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
fg_vswapind(struct fungen *__restrict self, ptrdiff_t ind_delta) {
	DO(fg_vswap(self));              /* src, dst */
	DO(fg_vdup(self));               /* src, dst, dst */
	DO(fg_vind(self, ind_delta));    /* src, dst, *(dst + ind_delta) */
	DO(fg_vreg(self, NULL));         /* src, dst, reg:*(dst + ind_delta) */
	fg_vtop_direct_clearref(self);   /* src, dst, reg:*(dst + ind_delta) */
	DO(fg_vrrot(self, 3));           /* reg:*(dst + ind_delta), src, dst */
	DO(fg_vswap(self));              /* reg:*(dst + ind_delta), dst, src */
	DO(fg_vpopind(self, ind_delta)); /* reg:*(dst + ind_delta), dst */
	return fg_vpop(self);            /* reg:*(dst + ind_delta) */
err:
	return -1;
}

/* Ensure that the top-most `DeeObject' from the object-stack is a reference. */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vref(struct fungen *__restrict self) {
	struct memval *mval;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = fg_vtop(self);
	if (memval_isnullable(mval) && memobj_isref(memval_nullable_getobj(mval)))
		return 0; /* Special case: NULLABLE+REF is allowed (and we don't make it direct) */
	DO(fg_vdirect1(self));
	mval = fg_vtop(self);
	ASSERT(memval_isdirect(mval));
	if (!memval_direct_isref(mval)) {
		struct memstate *state;
		struct memval *alias_mval;
		struct memobj *alias_mobj;
		bool did_find_first_alias;
		DO(fg_state_unshare(self));
		state = self->fg_state;
		mval  = memstate_vtop(state);
		ASSERT(memval_isdirect(mval));
		ASSERT(!memval_direct_isref(mval));

		/* If at least 2 other memory locations (or 1 if it's a constant) are already
		 * holding a reference to the same value, then we can steal a reference from
		 * one of them!
		 *
		 * The reason for that "2" is because as long as there are 2 references, an
		 * object is guarantied to have `DeeObject_IsShared()', meaning that whatever
		 * the caller might need the reference for, the object won't end up getting
		 * destroyed if the reference ends up being dropped! */
		did_find_first_alias = false;
		memstate_foreach(alias_mval, state) {
			if unlikely(alias_mval->mv_flags & MEMVAL_F_NOREF) {
				ASSERT(!memval_hasobj0(alias_mval));
				continue;
			}
			memval_foreach_obj(alias_mobj, alias_mval) {
				if (memobj_isref(alias_mobj) &&
				    memobj_sameloc(alias_mobj, memval_direct_getobj(mval))) {
					if (did_find_first_alias) {
						/* Steal the reference from `alias_mobj' */
						memobj_clearref(alias_mobj);
						memval_direct_setref(mval);
						return 0;
					}
					did_find_first_alias = true;
				}
			}
			memval_foreach_obj_end;
		}
		memstate_foreach_end;
		DO(fg_gincref_loc(self, memval_direct_getloc(mval), 1));
		ASSERT(mval == memstate_vtop(state));
		ASSERT(!memval_direct_isref(mval));
		ASSERT(!memval_direct_isoneref(mval));
		memval_direct_setref(mval);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vref_noconst(struct fungen *__restrict self) {
	struct memval *mval;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = fg_vtop(self);
	if (memval_isnullable(mval) && memobj_isref(memval_nullable_getobj(mval)))
		return 0; /* Special case: NULLABLE+REF is allowed (and we don't make it direct) */
	DO(fg_vdirect1(self));
	mval = fg_vtop(self);
	ASSERT(memval_isdirect(mval));
	if (!memval_direct_isref(mval) &&
	    !memval_direct_isconst(mval))
		return fg_vref(self);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vref_noalias(struct fungen *__restrict self) {
	struct memval *mval;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = fg_vtop(self);
	if (memval_isnullable(mval) && memobj_isref(memval_nullable_getobj(mval)))
		return 0; /* Special case: NULLABLE+REF is allowed (and we don't make it direct) */
	DO(fg_vdirect1(self));
	mval = fg_vtop(self);
	ASSERT(memval_isdirect(mval));
	if (!memval_direct_isref(mval)) {
		struct memstate *state;
		DO(fg_state_unshare(self));
		state = self->fg_state;
		mval  = memstate_vtop(state);
		ASSERT(memval_isdirect(mval));
		ASSERT(!memval_direct_isref(mval));
		DO(fg_gincref_loc(self, memval_direct_getloc(mval), 1));
		ASSERT(mval == memstate_vtop(state));
		ASSERT(!memval_direct_isref(mval));
		ASSERT(!memval_direct_isoneref(mval));
		memval_direct_setref(mval);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vref_noconst_noalias(struct fungen *__restrict self) {
	struct memval *mval;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = fg_vtop(self);
	if (memval_isnullable(mval) && memobj_isref(memval_nullable_getobj(mval)))
		return 0; /* Special case: NULLABLE+REF is allowed (and we don't make it direct) */
	DO(fg_vdirect1(self));
	mval = fg_vtop(self);
	ASSERT(memval_isdirect(mval));
	if (!memval_direct_isref(mval) &&
	    !memval_direct_isconst(mval)) {
		struct memstate *state;
		DO(fg_state_unshare(self));
		state = self->fg_state;
		mval  = memstate_vtop(state);
		ASSERT(memval_isdirect(mval));
		ASSERT(!memval_direct_isref(mval));
		DO(fg_gincref_loc(self, memval_direct_getloc(mval), 1));
		ASSERT(mval == memstate_vtop(state));
		ASSERT(!memval_direct_isref(mval));
		ASSERT(!memval_direct_isoneref(mval));
		memval_direct_setref(mval);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vref2(struct fungen *__restrict self,
         vstackaddr_t dont_steal_from_vtop_n) {
	struct memval *mval;
	DO(fg_vdirect1(self));
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = fg_vtop(self);
	ASSERT(memval_isdirect(mval));
	if (memval_direct_isconst(mval) && memval_direct_isref(mval))
		return 0; /* Special case: a reference to a constant doesn't need its aliases to have references also. */
	DO(fg_state_unshare(self));
	mval = fg_vtop(self);
	ASSERT(memval_isdirect(mval));
	return fg_gref2(self, memval_direct_getobj(mval),
	                dont_steal_from_vtop_n);
err:
	return -1;
}


/* Ensure that `mobj' is holding a reference. If said location has aliases,
 * and isn't a constant, then also ensure that at least one of those aliases
 * also contains a second reference.
 * @param: dont_steal_from_vtop_n: Ignore the top n v-stack items when searching for aliases. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_gref2(struct fungen *__restrict self,
         struct memobj *mobj,
         vstackaddr_t dont_steal_from_vtop_n) {
	struct memstate *state = self->fg_state;
	struct memval *alias_mval;
	struct memobj *alias_mobj;
	struct memobj *alias_with_reference = NULL;    /* Alias that has a reference */
	struct memobj *alias_without_reference = NULL; /* Alias that needs a reference */
	bool got_alias = false; /* There *are* aliases. */
	ASSERT(state->ms_stackc >= dont_steal_from_vtop_n);
	state->ms_stackc -= dont_steal_from_vtop_n;
	memstate_foreach(alias_mval, state) {
again_foreach_alias_mobj:
		memval_foreach_obj(alias_mobj, alias_mval) {
			if (memobj_sameloc(alias_mobj, mobj) && alias_mobj != mobj) {
				if unlikely(alias_mval->mv_flags & MEMVAL_F_NOREF) {
					ASSERT(!memval_hasobj0(alias_mval));
					if unlikely(memval_do_clear_MEMVAL_F_NOREF(alias_mval))
						goto again_foreach_alias_mobj;
				}
				/* Got an alias! */
				got_alias = true;
				if (!memobj_isref(alias_mobj)) {
					alias_without_reference = alias_mobj;
				} else if (!memobj_isref(mobj)) {
					/* Steal reference from alias_mobj */
					memobj_setref(mobj);
					memobj_clearref(alias_mobj);
					alias_without_reference = alias_mobj;
				} else {
					alias_with_reference = alias_mobj;
				}
			}
		}
		memval_foreach_obj_end;
	}
	memstate_foreach_end;
	state->ms_stackc += dont_steal_from_vtop_n;
	if (got_alias) {
		ASSERT(!alias_with_reference || memobj_isref(alias_with_reference));
		ASSERT(!alias_without_reference || !memobj_isref(alias_without_reference));
		if (!memobj_isref(mobj)) {
			/* There are aliases, but no-one is holding a reference.
			 * This can happen if the location points to a constant
			 * that got flushed, or is a function argument, in which
			 * case we only need a single reference. */
			ASSERT(alias_without_reference);
			ASSERT(!alias_with_reference);
			ASSERT(!memstate_isshared(state));
			DO(fg_gincref_loc(self, memobj_getloc(mobj), 1));
			memobj_setref(mobj);
			DO(fg_gnotoneref(self, mobj));
		} else if (alias_without_reference && !alias_with_reference &&
		           /* When it's a constant, there is already an extra reference through code dependencies */
		           !memobj_isconst(mobj)) {
			/* There are aliases, but less that 2 references -> make sure there are at least 2 references */
			ASSERT(!memstate_isshared(state));
			ASSERT(!memobj_isref(alias_without_reference));
			DO(fg_gincref_loc(self, memobj_getloc(alias_without_reference), 1));
			memobj_setref(alias_without_reference);
			DO(fg_gnotoneref(self, alias_without_reference));
		}
	} else {
		/* No aliases exist, so there's no need to force a distinct location. */
		if (!memobj_isref(mobj)) {
			ASSERT(!memstate_isshared(state));
			DO(fg_gincref_loc(self, memobj_getloc(mobj), 1));
			memobj_setref(mobj);
			DO(fg_gnotoneref(self, mobj));
		}
	}
	ASSERT(memobj_isref(mobj));
	return 0;
err:
	return -1;
}

/* Force vtop into a register (ensuring it has type `MEMADR_TYPE_HREG' for all locations used by VTOP) */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vreg(struct fungen *__restrict self,
        host_regno_t const *not_these) {
	struct memstate *state = self->fg_state;
	struct memval *mval;
	struct memobj *mobj;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
again:
	mval = memstate_vtop(state);
	memval_foreach_obj(mobj, mval) {
		if (memobj_gettyp(mobj) != MEMADR_TYPE_HREG) {
			if (memstate_isshared(state)) {
				state = memstate_copy(state);
				if unlikely(!state)
					goto err;
				memstate_decref_nokill(self->fg_state);
				self->fg_state = state;
				goto again;
			}
			DO(tracked_memloc_forcereg(self, memobj_getloc(mobj), not_these));
			ASSERT(memobj_gettyp(mobj) == MEMADR_TYPE_HREG);
		}
	}
	memval_foreach_obj_end;
	return 0;
err:
	return -1;
}

/* Force vtop onto the stack (ensuring it has type `MEMADR_TYPE_HSTACKIND,
 * memloc_hstackind_getvaloff = 0' for all locations used by VTOP) */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vflush(struct fungen *__restrict self,
          bool require_valoff_0) {
	struct memstate *state = self->fg_state;
	struct memval *mval;
	struct memobj *mobj;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
again:
	mval = memstate_vtop(state);
	memval_foreach_obj(mobj, mval) {
		if (memobj_gettyp(mobj) != MEMADR_TYPE_HSTACKIND ||
		    (memobj_hstackind_getvaloff(mobj) != 0 && require_valoff_0)) {
			struct memloc flushed_loc;
			if (memstate_isshared(state)) {
				state = memstate_copy(state);
				if unlikely(!state)
					goto err;
				memstate_decref_nokill(self->fg_state);
				self->fg_state = state;
				goto again;
			}
			DO(fg_gasflush(self, memobj_getloc(mobj),
			                                   &flushed_loc, require_valoff_0));
			memstate_changeloc(self->fg_state, memobj_getloc(mobj), &flushed_loc);
			ASSERTF(memloc_sameloc(memobj_getloc(mobj), &flushed_loc),
			        "This should have gotten updated!");
		}
	}
	memval_foreach_obj_end;
	return 0;
err:
	return -1;
}


/* Generate code to push a global variable onto the virtual stack. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vpush_mod_global(struct fungen *__restrict self,
                    struct Dee_module_object *mod, uint16_t gid, bool ref) {
	struct memloc *loc;
	struct module_symbol *symbol;
	if unlikely(gid >= mod->mo_globalc)
		return err_illegal_gid(mod, gid);
	symbol = DeeModule_GetSymbolID(mod, gid);
	ASSERT(!symbol || Dee_module_symbol_getindex(symbol) == gid);
	/* Global object references can be inlined if they are `final' and bound */
	if (((symbol == NULL) || /* Can be NULL in case it's the DELETE/SETTER of a property */
	     (symbol->ss_flags & (Dee_MODSYM_FPROPERTY | Dee_MODSYM_FREADONLY))) &&
	    !(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_NOROINLINE)) {
		DeeObject *current_value;
		DeeModule_LockRead(mod);
		current_value = mod->mo_globalv[gid];
		if (current_value != NULL) {
			Dee_Incref(current_value);
			DeeModule_LockEndRead(mod);
			current_value = fg_inlineref(self, current_value);
			if unlikely(!current_value)
				goto err;
			return fg_vpush_const(self, current_value);
		}
		DeeModule_LockEndRead(mod);
	}
	DO(fg_vpush_addr(self, &mod->mo_globalv[gid]));
#ifndef CONFIG_NO_THREADS
	if (ref)
		DO(fg_grwlock_read_const(self, &mod->mo_lock));
#endif /* !CONFIG_NO_THREADS */
	DO(fg_vind(self, 0));
	DO(fg_vreg(self, NULL));
	ASSERT(fg_vtop_isdirect(self));
	ASSERT(!fg_vtop_direct_isref(self));
	loc = fg_vtopdloc(self);
#ifndef CONFIG_NO_THREADS
	DO(fg_gassert_bound(self, loc, NULL, mod, gid,
	                    ref ? &mod->mo_lock : NULL,
	                    NULL));
#else /* !CONFIG_NO_THREADS */
	DO(fg_gassert_bound(self, loc, NULL, mod, gid));
#endif /* CONFIG_NO_THREADS */

	/* Depending on how the value will be used, we may not need a reference.
	 * If only its value is used (ASM_ISNONE, ASM_CMP_SO, ASM_CMP_DO), we
	 * won't actually need to take a reference here!
	 * Also: when not needing a reference, we don't need to acquire the lock,
	 *       either! */
	ASSERT(!fg_vtop_direct_isref(self));
	if (ref) {
		DO(fg_gincref_loc(self, loc, 1));
#ifndef CONFIG_NO_THREADS
		DO(fg_grwlock_endread_const(self, &mod->mo_lock));
#endif /* !CONFIG_NO_THREADS */
		ASSERT(!fg_vtop_direct_isref(self));
		fg_vtop_direct_setref(self);
	}
	return 0;
err:
	return -1;
}

/* Generate code to check if a global variable is currently bound. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vbound_mod_global(struct fungen *__restrict self,
                     struct Dee_module_object *mod, uint16_t gid) {
	struct memval *mval;
	struct module_symbol *symbol;
	if unlikely(gid >= mod->mo_globalc)
		return err_illegal_gid(mod, gid);
	symbol = DeeModule_GetSymbolID(mod, gid);
	ASSERT(!symbol || Dee_module_symbol_getindex(symbol) == gid);
	/* If the symbol is read-only and bound, then we know it can't be unbound */
	if (((symbol == NULL) || /* Can be NULL in case it's the DELETE/SETTER of a property */
	     (symbol->ss_flags & (Dee_MODSYM_FPROPERTY | Dee_MODSYM_FREADONLY))) &&
	    !(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_NOROINLINE)) {
		DeeObject *current_value = atomic_read(&mod->mo_globalv[gid]);
		if (current_value != NULL)
			return fg_vpush_const(self, Dee_True);
	}
	DO(fg_vpush_addr(self, &mod->mo_globalv[gid]));
	DO(fg_vind(self, 0));
	mval = fg_vtop(self);
	ASSERT(memval_isdirect(mval));
	mval->mv_vmorph = MEMVAL_VMORPH_TESTNZ(mval->mv_vmorph);
	return 0;
err:
	return -1;
}

/* Generate code to pop a global variable from the virtual stack. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
vpopref_mod_global(struct fungen *__restrict self,
                   struct Dee_module_object *mod, uint16_t gid) {
	struct memval *mval;
	struct memloc loc;
	if unlikely(gid >= mod->mo_globalc)
		return err_illegal_gid(mod, gid);
	DO(fg_vdirect1(self));                              /* value */
	DO(fg_vpush_addr(self, &mod->mo_globalv[gid]));     /* value, &GLOBAL */
	DO(fg_vswap(self));                                 /* &GLOBAL, value */
#ifndef CONFIG_NO_THREADS
	DO(fg_grwlock_write_const(self, &mod->mo_lock));    /* &GLOBAL, value */
#endif /* !CONFIG_NO_THREADS */
	DO(fg_vswapind(self, 0));                           /* ref:old_value */
#ifndef CONFIG_NO_THREADS
	DO(fg_grwlock_endwrite_const(self, &mod->mo_lock)); /* ref:old_value */
#endif /* !CONFIG_NO_THREADS */
	ASSERT(self->fg_state->ms_stackc >= 1);
	mval = fg_vtop(self);
	--self->fg_state->ms_stackc;
	ASSERT(memval_isdirect(mval));
	ASSERT(!memval_direct_isref(mval));
	loc = *memval_direct_getloc(mval);
	memval_direct_fini(mval);
	memstate_decrinuse_for_memloc(self->fg_state, &loc);
	return fg_gxdecref_loc(self, &loc, 1); /* xdecref in case global wasn't bound before. */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vdel_mod_global(struct fungen *__restrict self,
                   struct Dee_module_object *mod, uint16_t gid) {
	int result = fg_vpush_addr(self, NULL);
	if likely(result == 0)
		result = vpopref_mod_global(self, mod, gid);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vpop_mod_global(struct fungen *__restrict self,
                   struct Dee_module_object *mod, uint16_t gid) {
	int result = fg_vref2(self, 1);
	if likely(result == 0)
		result = vpopref_mod_global(self, mod, gid);
	return result;
}


INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_extern(struct fungen *__restrict self,
                uint16_t mid, uint16_t gid, bool ref) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	if unlikely(mid >= mod->mo_importc)
		return err_illegal_mid(mid);
	mod = mod->mo_importv[mid];
	return fg_vpush_mod_global(self, mod, gid, ref);
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vbound_extern(struct fungen *__restrict self, uint16_t mid, uint16_t gid) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	if unlikely(mid >= mod->mo_importc)
		return err_illegal_mid(mid);
	mod = mod->mo_importv[mid];
	return fg_vbound_mod_global(self, mod, gid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vdel_extern(struct fungen *__restrict self, uint16_t mid, uint16_t gid) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	if unlikely(mid >= mod->mo_importc)
		return err_illegal_mid(mid);
	mod = mod->mo_importv[mid];
	return fg_vdel_mod_global(self, mod, gid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpop_extern(struct fungen *__restrict self, uint16_t mid, uint16_t gid) {
	DeeModuleObject *mod = self->fg_assembler->fa_code->co_module;
	if unlikely(mid >= mod->mo_importc)
		return err_illegal_mid(mid);
	mod = mod->mo_importv[mid];
	return fg_vpop_mod_global(self, mod, gid);
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpush_static(struct fungen *__restrict self, uint16_t sid) {
	DeeCodeObject *code = self->fg_assembler->fa_code;
	if unlikely(sid < code->co_refc || sid >= code->co_refstaticc)
		return err_illegal_sid(sid);
	/* TODO */
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vpop_static(struct fungen *__restrict self, uint16_t sid) {
	DeeCodeObject *code = self->fg_assembler->fa_code;
	if unlikely(sid < code->co_refc || sid >= code->co_refstaticc)
		return err_illegal_sid(sid);
	/* TODO */
	return DeeError_NOTIMPLEMENTED();
}

#ifndef CONFIG_NO_THREADS
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vrwlock_read(struct fungen *__restrict self) {
	DO(fg_vdirect1(self));
	DO(fg_state_unshare(self));
	DO(fg_grwlock_read(self, fg_vtopdloc(self)));
	return fg_vpop(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vrwlock_write(struct fungen *__restrict self) {
	DO(fg_vdirect1(self));
	DO(fg_state_unshare(self));
	DO(fg_grwlock_write(self, fg_vtopdloc(self)));
	return fg_vpop(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vrwlock_endread(struct fungen *__restrict self) {
	DO(fg_vdirect1(self));
	DO(fg_state_unshare(self));
	DO(fg_grwlock_endread(self, fg_vtopdloc(self)));
	return fg_vpop(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vrwlock_endwrite(struct fungen *__restrict self) {
	DO(fg_vdirect1(self));
	DO(fg_state_unshare(self));
	DO(fg_grwlock_endwrite(self, fg_vtopdloc(self)));
	return fg_vpop(self);
err:
	return -1;
}
#endif /* !CONFIG_NO_THREADS */

/* Make sure there are no NULLABLE memobj-s anywhere on the stack or in locals. */
INTERN WUNUSED NONNULL((1)) int DCALL
_fg_vnonullable(struct fungen *__restrict self) {
	struct memstate *state;
	struct memval *mval;
	DO(fg_state_unshare(self));
	state = self->fg_state;
	ASSERT(state->ms_flags & MEMSTATE_F_GOTNULLABLE);
	memstate_foreach(mval, state) {
		if (mval->mv_vmorph == MEMVAL_VMORPH_NULLABLE)
			return fg_vdirect_memval(self, mval);
	}
	memstate_foreach_end;
	/* Just clear the flag (something might have forgotten to clear it) */
	state->ms_flags &= ~MEMSTATE_F_GOTNULLABLE;
	return 0;
err:
	return -1;
}


/* Check if `loc' differs from vtop, and if so: move vtop
 * *into* `loc', the assign the *exact* given `loc' to vtop. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vsetloc(struct fungen *__restrict self,
           struct memloc const *loc) {
	int result;
	struct memloc *vtop_loc;
	DO(fg_vdirect1(self));
	ASSERT(fg_vtop_isdirect(self));
	vtop_loc = fg_vtopdloc(self);
	if (memloc_sameloc(vtop_loc, loc))
		return 0; /* Already the same location -> no need to do anything */
	DO(fg_state_unshare(self));
	vtop_loc = fg_vtopdloc(self);
	result   = fg_gmov_loc2loc(self, vtop_loc, loc);
	if likely(result == 0) {
		memstate_decrinuse_for_memloc(self->fg_state, vtop_loc);
		*vtop_loc = *loc;
		memstate_incrinuse_for_memloc(self->fg_state, vtop_loc);
	}
	return result;
err:
	return -1;
}


/* Return and pass the top-most stack element as function result.
 * This function will clear the stack and unbind all local variables. */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vret(struct fungen *__restrict self) {
	struct memloc loc;
	struct memobj *retobj;
	vstackaddr_t stackc;
	lid_t lid;

	/* Special case: NULLABLE locations can be returned as-is */
	DO(fg_vndirect1(self));

	/* Move the final return value to the bottom of the stack. */
	stackc = self->fg_state->ms_stackc;
	DO(fg_vrrot(self, stackc));

	/* Remove all but the final element from the stack. */
	DO(fg_vpopmany(self, stackc - 1));

	/* Unbind all local variables. */
	for (lid = 0; lid < self->fg_state->ms_localc; ++lid)
		DO(fg_vdel_local(self, lid));

	/* Ensure that the final stack element contains a reference. */
	DO(fg_vref_noalias(self));

	/* Steal the final (returned) object from stack */
	ASSERT(self->fg_state->ms_stackc == 1);
	ASSERT(memval_isdirect(&self->fg_state->ms_stackv[0]) ||
	       memval_isnullable(&self->fg_state->ms_stackv[0]));
	ASSERT(memval_hasobj0(&self->fg_state->ms_stackv[0]));
	retobj = memval_getobj0(&self->fg_state->ms_stackv[0]);
	ASSERT(memobj_isref(retobj));
	loc = *memobj_getloc(retobj);
	memobj_fini(retobj);
	memstate_decrinuse_for_memloc(self->fg_state, &loc);
	self->fg_state->ms_stackc = 0;

	/* Generate code to do the return. */
	return fg_gret(self, &loc);
err:
	return -1;
}



/* Do calling-convention-specific handling of the return value. */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcallapi_checkresult(struct fungen *__restrict self,
                        unsigned int cc, vstackaddr_t n_pop) {
	switch (cc) {

	case VCALL_CC_OBJECT:
		DO(fg_vpush_hreg(self, HOST_REGNO_RETURN, 0)); /* [args...], UNCHECKED(result) */
		DO(fg_vrrot(self, n_pop + 1));                 /* UNCHECKED(result), [args...] */
		DO(fg_vpopmany(self, n_pop));                  /* UNCHECKED(result) */
		return fg_vcheckobj(self);                     /* result */

	case VCALL_CC_RAWINTPTR:
	case VCALL_CC_RAWINTPTR_NX:
		DO(fg_vpush_hreg(self, HOST_REGNO_RETURN, 0)); /* [args...], UNCHECKED(result) */
		DO(fg_vrrot(self, n_pop + 1));                 /* UNCHECKED(result), [args...] */
		break;

	case VCALL_CC_BOOL:
	case VCALL_CC_BOOL_NX:
		DO(fg_vpush_hreg(self, HOST_REGNO_RETURN, 0)); /* [args...], UNCHECKED(result) */
		ASSERT(fg_vtop(self)->mv_vmorph == MEMVAL_VMORPH_DIRECT);
		fg_vtop(self)->mv_vmorph = MEMVAL_VMORPH_BOOL_NZ;
		DO(fg_vrrot(self, n_pop + 1)); /* UNCHECKED(result), [args...] */
		break;

	case VCALL_CC_VOID:
	case VCALL_CC_VOID_NX:
		break;

	case VCALL_CC_EXCEPT:
		DO(fg_vpopmany(self, n_pop));
		return fg_gjmp_except(self);

	case VCALL_CC_INT:
		DO(fg_vpush_hreg(self, HOST_REGNO_RETURN, 0)); /* [args...], UNCHECKED(result) */
		DO(fg_vrrot(self, n_pop + 1));                 /* UNCHECKED(result), [args...] */
		DO(fg_vpopmany(self, n_pop));                  /* UNCHECKED(result) */
		return fg_vcheckint(self);                     /* - */

	case VCALL_CC_NEGINT: {
		DO(fg_vpush_hreg(self, HOST_REGNO_RETURN, 0)); /* [args...], UNCHECKED(result) */
		DO(fg_vrrot(self, n_pop + 1));                 /* UNCHECKED(result), [args...] */
		DO(fg_vpopmany(self, n_pop));                  /* UNCHECKED(result) */
		return fg_gjcmp_except(self, fg_vtopdloc(self), 0,
		                       FG_GJCMP_EXCEPT_LO);
	}	break;

	case VCALL_CC_M1INT: {
		DO(fg_vpush_hreg(self, HOST_REGNO_RETURN, 0)); /* [args...], UNCHECKED(result) */
		DO(fg_vrrot(self, n_pop + 1));                 /* UNCHECKED(result), [args...] */
		DO(fg_vpopmany(self, n_pop));                  /* UNCHECKED(result) */
		return fg_gjeq_except(self, fg_vtopdloc(self), -1);
	}	break;

	case VCALL_CC_MORPH_INTPTR:
	case VCALL_CC_MORPH_UINTPTR:
	case VCALL_CC_MORPH_INTPTR_NX:
	case VCALL_CC_MORPH_UINTPTR_NX:
		DO(fg_vpush_hreg(self, HOST_REGNO_RETURN, 0)); /* [args...], UNCHECKED(result) */
		ASSERT(fg_vtop(self)->mv_vmorph == MEMVAL_VMORPH_DIRECT);
		fg_vtop(self)->mv_vmorph = (cc == VCALL_CC_MORPH_UINTPTR ||
		                            cc == VCALL_CC_MORPH_UINTPTR_NX)
		                           ? MEMVAL_VMORPH_UINT
		                           : MEMVAL_VMORPH_INT;
		DO(fg_vrrot(self, n_pop + 1)); /* UNCHECKED(result), [args...] */
		break;

	default: __builtin_unreachable();
	}

	/* Pop function arguments. */
	return fg_vpopmany(self, n_pop);
err:
	return -1;
}

/* Generate host text to invoke `api_function' with the top-most `argc' items from the stack.
 * @param: cc:    One of `VCALL_CC_*', describing the calling-convention of `api_function'.
 * @param: n_pop: The # of stack items to pop during the call (in case of registers, these won't need to be saved)
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcallapi_ex_(struct fungen *__restrict self,
                void const *api_function, unsigned int cc,
                vstackaddr_t argc, vstackaddr_t n_pop) {
	vstackaddr_t argi;
	struct memloc *l_argv;
	struct memval *v_argv;
	DO(fg_vdirect(self, argc));
	DO(fg_state_unshare(self));

	/* Unless the class is *_NX, make sure there aren't any NULLABLE locations */
	switch (cc) {
	case VCALL_CC_RAWINT_NX:
	case VCALL_CC_VOID_NX:
	case VCALL_CC_BOOL_NX:
	case VCALL_CC_MORPH_INTPTR_NX:
	case VCALL_CC_MORPH_UINTPTR_NX:
		break;
	default:
		DO(fg_vnonullable(self));
		break;
	}

	/* Flush registers that don't appear in the top `n_pop' stack locations.
	 * When the function always throw an exception, we *only* need to preserve
	 * stuff that contains references! */
	DO(fg_vflushregs(self, n_pop, cc == VCALL_CC_EXCEPT));

	/* Build up the argument list. */
	v_argv = self->fg_state->ms_stackv;
	v_argv += self->fg_state->ms_stackc;
	v_argv -= argc;
	l_argv = (struct memloc *)Dee_Mallocac(argc + 1, sizeof(struct memloc));
	if unlikely(!l_argv)
		goto err;
	memloc_init_const(&l_argv[0], api_function);
	for (argi = 0; argi < argc; ++argi)
		l_argv[argi + 1] = *memval_direct_getloc(&v_argv[argi]);

	/* Call the actual C function */
	if unlikely(fg_gcallapi(self, l_argv, argc))
		goto err_l_argv;
	Dee_Freea(l_argv);

	/* Do calling-convention-specific handling of the return value. */
	return fg_vcallapi_checkresult(self, cc, n_pop);
err_l_argv:
	Dee_Freea(l_argv);
err:
	return -1;
}

/* [args...], funcaddr -> ...
 * Same as `fg_vcallapi()', but after the normal argument list,
 * there is an additional item "funcaddr" that contains the (possibly) runtime-
 * evaluated address of the function that should be called. Also note that said
 * "funcaddr" location is *always* popped.
 * @param: cc: One of `VCALL_CC_*', describing the calling-convention of `api_function' 
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcalldynapi_ex(struct fungen *__restrict self,
                  unsigned int cc, vstackaddr_t argc,
                  vstackaddr_t n_pop) {
	vstackaddr_t argi;
	struct memloc *l_argv;
	struct memval *v_argv;
	DO(fg_vdirect(self, argc + 1));
	DO(fg_state_unshare(self));
	v_argv = fg_vtop(self);
	if (memval_direct_isconst(v_argv)) {
		/* Special case: function being called is actually at a constant address. */
		void const *api_function = memval_const_getaddr(v_argv);
		DO(fg_vpop(self));
		return fg_vcallapi(self, api_function, cc, argc);
	}

	/* Flush registers that don't appear in the top `n_pop' stack locations.
	 * When the function always throw an exception, we *only* need to preserve
	 * stuff that contains references! */
	DO(fg_vflushregs(self, n_pop, cc == VCALL_CC_EXCEPT));

	/* Build up the argument list. */
	v_argv -= argc;
	l_argv = (struct memloc *)Dee_Mallocac(1 + argc, sizeof(struct memloc));
	if unlikely(!l_argv)
		goto err;
	for (argi = 0; argi < argc; ++argi)
		l_argv[1 + argi] = *memval_direct_getloc(&v_argv[argi]);
	l_argv[0] = *fg_vtopdloc(self);
	if unlikely(fg_vpop(self))
		goto err_l_argv;

	/* Call the actual C function */
	if unlikely(fg_gcallapi(self, l_argv, argc))
		goto err_l_argv;
	Dee_Freea(l_argv);

	/* Do calling-convention-specific handling of the return value. */
	return fg_vcallapi_checkresult(self, cc, n_pop);
err_l_argv:
	Dee_Freea(l_argv);
err:
	return -1;
}


/* After a call to `fg_vcallapi()' with `VCALL_CC_RAWINTPTR',
 * do the extra trailing checks needed to turn that call into `VCALL_CC_OBJECT'
 * The difference to directly passing `VCALL_CC_OBJECT' is that using this 2-step
 * method, you're able to pop more elements from the stack first.
 *
 * However: be careful not to do anything that might throw additional exceptions!
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcheckobj(struct fungen *__restrict self) {
	struct memval *mval;
	DO(fg_state_unshare(self));
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = fg_vtop(self);
	ASSERTF(memval_isdirect(mval), "non-sensical call: vcheckobj() on non-direct value");

	/* Delay NULL checks until a later point in time:
	 * >> return foo();     // No NULL-check needed here (if foo() returns NULL, just propagate as-is)
	 * >> foo({ a, b, c }); // Delayed NULL-check means that the SharedVector can be decref'd before the
	 * >>                   // NULL-check (meaning it doesn't need to be decref'd by exception cleanup code)
	 */
	mval->mv_vmorph = MEMVAL_VMORPH_NULLABLE;

	/* Clear the NOREF flag when the return value is non-NULL */
	if (memobj_gettyp(memval_nullable_getobj(mval)) != MEMADR_TYPE_CONST) {
		ASSERT(!memobj_isref(memval_nullable_getobj(mval)));
		memobj_setref(memval_nullable_getobj(mval));
	}
	return 0;
err:
	return -1;
}

/* After a call to `fg_vcallapi()' with `VCALL_CC_RAWINTPTR',
 * do the extra trailing checks needed to turn that call into `VCALL_CC_INT'
 * The difference to directly passing `VCALL_CC_INT' is that using this 2-step
 * method, you're able to pop more elements from the stack first.
 * NOTE: This function pops one element from the V-stack.
 *
 * However: be careful not to do anything that might throw additional exceptions!
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcheckint(struct fungen *__restrict self) {
	struct memval *mval;
	DO(fg_state_unshare(self));
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = fg_vtop(self);
	ASSERTF(memval_isdirect(mval), "non-sensical call: vcheckint() on non-direct value");
	DO(fg_gjnz_except(self, memval_direct_getloc(mval)));
	return fg_vpop(self);
err:
	return -1;
}

/* Branch to exception handling if `vtop' is equal to `except_val' */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcheckerr(struct fungen *__restrict self,
             intptr_t except_val) {
	struct memval *mval;
	DO(fg_state_unshare(self));
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	mval = fg_vtop(self);
	ASSERTF(memval_isdirect(mval), "non-sensical call: vcheckint() on non-direct value");
	DO(fg_gjeq_except(self, memval_direct_getloc(mval), except_val));
	return fg_vpop(self);
err:
	return -1;
}


/* Generate a call to `DeeObject_MALLOC()' to allocate an uninitialized object that
 * provides for "alloc_size" bytes of memory. If possible, try to dispatch against
 * a slap allocator instead (just like the real DeeObject_MALLOC also does).
 * NOTE: The value pushed onto the V-stack...
 *       - ... already has its MEMOBJ_F_ISREF flag SET!
 *       - ... has already been NULL-checked (i.e. already is a direct value)
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcall_DeeObject_MALLOC(struct fungen *__restrict self,
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
		DO(fg_vcallapi(self, api_function, VCALL_CC_OBJECT, 0));
	} else
#endif /* !CONFIG_NO_OBJECT_SLABS */
	{
		DO(fg_vpush_immSIZ(self, alloc_size));
		DO(fg_vcallapi(self,
		               do_calloc ? (void const *)&DeeObject_Calloc
		                         : (void const *)&DeeObject_Malloc,
		               VCALL_CC_OBJECT, 1));
	}
	DO(fg_vdirect1(self));
	DO(fg_state_unshare(self));
	/* The NOREF flag must *NOT* be set (because the intend is for the caller to create an object) */
	ASSERT(fg_vtop_direct_isref(self));
	return fg_voneref_noalias(self); /* Initial reference -> oneref */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcall_DeeObject_Malloc(struct fungen *__restrict self,
                          size_t alloc_size, bool do_calloc) {
	DO(fg_vpush_immSIZ(self, alloc_size));
	DO(fg_vcallapi(self,
	               do_calloc ? (void const *)&DeeObject_Calloc
	                         : (void const *)&DeeObject_Malloc,
	               VCALL_CC_OBJECT, 1));
	DO(fg_vdirect1(self));
	DO(fg_state_unshare(self));
	/* The NOREF flag must *NOT* be set (because the intend is for the caller to create an object) */
	ASSERT(fg_vtop_direct_isref(self));
	return fg_voneref_noalias(self); /* Initial reference -> oneref */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fg_vcall_DeeGCObject_Malloc(struct fungen *__restrict self,
                            size_t alloc_size, bool do_calloc) {
	DO(fg_vpush_immSIZ(self, alloc_size));
	DO(fg_vcallapi(self,
	               do_calloc ? (void const *)&DeeGCObject_Calloc
	                         : (void const *)&DeeGCObject_Malloc,
	               VCALL_CC_OBJECT, 1));
	DO(fg_vdirect1(self));
	DO(fg_state_unshare(self));
	/* The NOREF flag must *NOT* be set (because the intend is for the caller to create an object) */
	ASSERT(fg_vtop_direct_isref(self));
	return fg_voneref_noalias(self); /* Initial reference -> oneref */
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
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL, /* Not needed; only used internally */
			/* tp_free:        */ NULL
		),
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
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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
	result = (DREF DummyVectorObject *)DeeObject_Mallocc(offsetof(DummyVectorObject, dvo_items),
	                                                     num_items, sizeof(void *));
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
                    host_cfa_t hstack_cfa_offset,
                    struct memval const *linbase,
                    vstackaddr_t linsize,
                    host_cfa_t cfa_offset) {
	vstackaddr_t i;
	size_t result = 0;
	for (i = 0; i < linsize; ++i) {
		struct memval const *src = &linbase[i];
#ifdef HOSTASM_STACK_GROWS_DOWN
		host_cfa_t dst_cfa_offset = cfa_offset - i * HOST_SIZEOF_POINTER;
#else /* HOSTASM_STACK_GROWS_DOWN */
		host_cfa_t dst_cfa_offset = cfa_offset + i * HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		if (memloc_gettyp(memval_direct_getloc(src)) == MEMADR_TYPE_HSTACKIND &&
		    memloc_hstackind_getcfa(memval_direct_getloc(src)) == dst_cfa_offset)
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
memvals_anyhstackind(struct memval const *__restrict base,
                         vstackaddr_t count) {
	vstackaddr_t i;
	for (i = 0; i < count; ++i) {
		if (memval_direct_gettyp(&base[i]) == MEMADR_TYPE_HSTACKIND)
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
fg_vlinear(struct fungen *__restrict self,
                               vstackaddr_t argc, bool readonly) {
	DO(fg_vdirect(self, argc));
	DO(fg_state_unshare(self));
	if unlikely(!argc) {
		/* The base address of an empty vector doesn't matter, meaning it's undefined */
		return fg_vpush_undefined(self);
	} else if (readonly && fg_vallconst_noref(self, argc)) {
		/* Dynamically allocate a dummy object which includes space
		 * for "argc" pointers. Then, fill those pointers with values
		 * from the v-stack, inline the reference to dummy object, and
		 * finally: push a pointer to the base address of the dummy's
		 * value array. */
		vstackaddr_t i;
		struct memval *cbase;
		DREF DummyVectorObject *vec = DummyVector_New(argc);
		if unlikely(!vec)
			goto err;
		vec = (DREF DummyVectorObject *)fg_inlineref(self, Dee_AsObject(vec));
		if unlikely(!vec)
			goto err;
		cbase = self->fg_state->ms_stackv + self->fg_state->ms_stackc - argc;
		for (i = 0; i < argc; ++i)
			vec->dvo_items[i] = memval_const_getaddr(&cbase[i]);
		return fg_vpush_addr(self, vec->dvo_items);
	} else if (argc == 1) {
		/* Deal with simple case: caller only wants the address of a single location.
		 * In this case, we only need to make sure that said location resides in
		 * memory (which is even allowed to be a REGIND location), and then push
		 * the address of that location. */
		host_cfa_t cfa_offset;
		struct memloc *loc = fg_vtopdloc(self);
		if (memloc_gettyp(loc) == MEMADR_TYPE_HREGIND &&
		    memloc_hregind_getvaloff(loc) == 0) {
			/* Special case: address of `*(%reg + off) + 0' is `%reg + off' */
			return fg_vpush_hreg(self,
			                     memloc_hregind_getreg(loc),
			                     memloc_hregind_getindoff(loc));
		}

		/* Flush value to the stack to give is an addressable memory location. */
		DO(fg_vflush(self, true));
		loc = fg_vtopdloc(self);
		ASSERT(memloc_gettyp(loc) == MEMADR_TYPE_HSTACKIND);
		ASSERT(memloc_hstackind_getvaloff(loc) == 0);
		cfa_offset = memloc_hstackind_getcfa(loc);
		return fg_vpush_hstack(self, cfa_offset);
	} else {
		/* General case: figure out the optimal CFA base address of the linear vector. */
		vstackaddr_t i;
		host_cfa_t result_cfa_offset;
		DREF struct memstate *linear_state;
		struct memstate *state = self->fg_state;
		struct memval *linbase = state->ms_stackv + state->ms_stackc - argc;

		/* Check for special case: if none of the linear locations are HSTACKIND,
		 * then the optimal target location is always either a sufficiently large
		 * free region, or new newly alloca'd region. */
		if (!memvals_anyhstackind(linbase, argc)) {
			size_t num_bytes = argc * sizeof(void *);
			result_cfa_offset = memstate_hstack_find(state, self->fg_state_hstack_res, num_bytes);
			if (result_cfa_offset == (host_cfa_t)-1) {
				result_cfa_offset = memstate_hstack_greatest_inuse(state);
#ifdef HOSTASM_STACK_GROWS_DOWN
				result_cfa_offset += num_bytes;
#else /* HOSTASM_STACK_GROWS_DOWN */
				result_cfa_offset += HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			}
		} else {
			/* Fallback: Assign scores to all possible CFA offsets for the linear vector.
			 *           Then, choose whatever CFA offset has the lowest score. */
			size_t result_cfa_offset_score;
			host_cfa_t cfa_offset_max;
			host_cfa_t cfa_offset_min;
			host_cfa_t cfa_offset;
			struct memval *mval;
			bitset_t *hstack_inuse;    /* Bitset for currently in-use hstack locations (excluding locations used by linear slots) */
			bitset_t *hstack_reserved; /* Bitset of hstack locations that can never be used (because they belong to `MEMOBJ_F_LINEAR' items) */
			size_t hstack_inuse_sizeof;
			hstack_inuse_sizeof = BITSET_SIZEOF(state->ms_host_cfa_offset / HOST_SIZEOF_POINTER);
			hstack_inuse = (bitset_t *)Dee_Calloca(hstack_inuse_sizeof * 2);
			if unlikely(!hstack_inuse)
				goto err;
			hstack_reserved = (bitset_t *)((byte_t *)hstack_inuse + hstack_inuse_sizeof);
			memstate_foreach(mval, state) {
				struct memobj *mobj;
				memval_foreach_obj(mobj, mval) {
					if (memobj_gettyp(mobj) == MEMADR_TYPE_HSTACKIND) {
						host_cfa_t cfa = memobj_getcfastart(mobj);
						ASSERT(cfa < state->ms_host_cfa_offset);
						bitset_set(hstack_inuse, cfa / HOST_SIZEOF_POINTER);
						if (mobj->mo_flags & MEMOBJ_F_LINEAR)
							bitset_set(hstack_reserved, cfa / HOST_SIZEOF_POINTER);
					}
				}
				memval_foreach_obj_end;
			}
			memstate_foreach_end;
			/* hstack locations currently in use by the linear portion don't count as in-use.
			 * NOTE: We do this in a second pass, so we also hit all of the aliases. */
			for (i = 0; i < argc; ++i) {
				ASSERT(memval_isdirect(&linbase[i]));
				if (memval_direct_gettyp(&linbase[i]) == MEMADR_TYPE_HSTACKIND) {
					host_cfa_t cfa = memloc_getcfastart(memval_direct_getloc(&linbase[i]));
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
				return fg_vpush_hstack(self, result_cfa_offset);
			}

		}

		/* Construct a memstate that puts the linear items along `result_cfa_offset' */
		linear_state = memstate_copy(state);
		if unlikely(!linear_state)
			goto err;

		/* Collect all locations that are aliases to those that should become linear.
		 * Then, assign intended target locations to aliases as far as possible. */
		linbase = linear_state->ms_stackv + linear_state->ms_stackc - argc;

		/* Gather aliases */
#define TYPE_LINLOC MEMEQUIV_TYPE_UNUSED
#define TYPE_ALIAS  MEMEQUIV_TYPE_DUMMY
		for (i = 0; i < argc; ++i) {
			struct memval *aliasval, *mval = &linbase[i];
			struct memloc locval = mval->mv_obj.mvo_0.mo_loc;
			mval->mv_obj.mvo_0.mo_loc.ml_adr.ma_typ = TYPE_LINLOC;
			mval->mv_obj.mvo_0.mo_loc.ml_adr.ma_val._v_nextobj = NULL;
			memstate_foreach(aliasval, linear_state) {
				struct memobj *alias;
				memval_foreach_obj(alias, aliasval) {
					if (alias->mo_loc.ml_adr.ma_typ == TYPE_LINLOC ||
					    alias->mo_loc.ml_adr.ma_typ == TYPE_ALIAS)
						continue;
					if (memloc_sameloc(&alias->mo_loc, &locval)) {
						alias->mo_loc.ml_adr.ma_typ = TYPE_ALIAS;
						alias->mo_loc.ml_adr.ma_val._v_nextobj = mval->mv_obj.mvo_0.mo_loc.ml_adr.ma_val._v_nextobj;
						mval->mv_obj.mvo_0.mo_loc.ml_adr.ma_val._v_nextobj = alias;
					}
				}
				memval_foreach_obj_end;
			}
			memstate_foreach_end;
		}
		for (i = 0; i < argc; ++i) {
			struct memobj *loc = &linbase[i].mv_obj.mvo_0;
			if (loc->mo_loc.ml_adr.ma_typ == TYPE_LINLOC) {
				struct memobj *next;
#ifdef HOSTASM_STACK_GROWS_DOWN
				host_cfa_t dst_cfa_offset = result_cfa_offset - i * HOST_SIZEOF_POINTER;
#else /* HOSTASM_STACK_GROWS_DOWN */
				host_cfa_t dst_cfa_offset = result_cfa_offset + i * HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
				do {
					next = loc->mo_loc.ml_adr.ma_val._v_nextobj;
					memloc_init_hstackind(&loc->mo_loc, dst_cfa_offset, 0);
				} while ((loc = next) != NULL);
			}
		}
#undef TYPE_LINLOC
#undef TYPE_ALIAS

		/* TODO: Move already-present, but unrelated locations out-of-the way. */

		/* Fix locations where linear elements were aliasing each other. */
		for (i = 0; i < argc; ++i) {
#ifdef HOSTASM_STACK_GROWS_DOWN
			host_cfa_t dst_cfa_offset = result_cfa_offset - i * HOST_SIZEOF_POINTER;
#else /* HOSTASM_STACK_GROWS_DOWN */
			host_cfa_t dst_cfa_offset = result_cfa_offset + i * HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			struct memval *mval = &linbase[i];
			ASSERT(mval->mv_obj.mvo_0.mo_loc.ml_adr.ma_typ == MEMADR_TYPE_HSTACKIND);
			mval->mv_obj.mvo_0.mo_loc.ml_adr.ma_val.v_cfa = dst_cfa_offset;
			mval->mv_obj.mvo_0.mo_flags |= MEMOBJ_F_LINEAR; /* Not allowed to move until popped */
		}

		/* Make sure that `linear_state's CFA offset is large enough to hold the linear vector. */
		{
#ifdef HOSTASM_STACK_GROWS_DOWN
			host_cfa_t req_min_host_cfa = result_cfa_offset;
#else /* HOSTASM_STACK_GROWS_DOWN */
			host_cfa_t req_min_host_cfa = result_cfa_offset + argc * HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			if (linear_state->ms_host_cfa_offset < req_min_host_cfa)
				linear_state->ms_host_cfa_offset = req_min_host_cfa;
		}

		/* Generate code to morph the current memory state to that of `linear_state'. */
		{
			int temp = fg_vmorph(self, linear_state);
			memstate_decref(linear_state);
			if likely(temp == 0)
				temp = fg_vpush_hstack(self, result_cfa_offset);
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



/* Perform bitwise or arithmetic operations (in the later case, also support doing jumps based on the operation overflowing) */

/* PUSH(POP(2) <op> POP(1)); */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_vbitop(struct fungen *__restrict self, host_bitop_t op) {
	host_regno_t retreg;
	struct memloc lhs, rhs;
	DO(fg_vdirect(self, 2));
	/* Juggle values so we can get rid of references.
	 * FIXME: This breaks when "lhs" and "rhs" depend on each other... */
	lhs = *memval_direct_getloc(fg_vtop(self) - 1);
	rhs = *memval_direct_getloc(fg_vtop(self) - 0);
	DO(fg_vpush_memloc(self, &lhs)); /* a, b, lhs */
	DO(fg_vpush_memloc(self, &rhs)); /* a, b, lhs, rhs */
	DO(fg_vpop_at(self, 2));         /* a, lhs, rhs */
	DO(fg_vpop_at(self, 2));         /* lhs, rhs */
	lhs = *memval_direct_getloc(fg_vtop(self) - 1);
	rhs = *memval_direct_getloc(fg_vtop(self) - 0);
	DO(fg_vpopmany(self, 2));        /* N/A */
	if (memloc_isconst(&lhs) && memloc_isconst(&rhs)) {
		uintptr_t lhsval = (uintptr_t)memloc_const_getaddr(&lhs);
		uintptr_t rhsval = (uintptr_t)memloc_const_getaddr(&rhs);
		uintptr_t result = host_bitop_forconst(op, lhsval, rhsval);
		return fg_vpush_addr(self, (void const *)result);
	}
	retreg = fg_gallocreg(self, NULL);
	if unlikely(retreg >= HOST_REGNO_COUNT)
		goto err;
	DO(fg_gbitop_locloc2reg(self, op, &lhs, &rhs, retreg));
	return fg_vpush_hreg(self, retreg, 0);
err:
	return -1;
}

/* PUSH(POP(2) <op> POP(1)); */
INTERN WUNUSED NONNULL((1)) int DCALL
fg_varithop(struct fungen *__restrict self, host_arithop_t op,
            struct host_symbol *dst_o, struct host_symbol *dst_no) {
	host_regno_t retreg;
	struct memloc lhs, rhs;
	DO(fg_vdirect(self, 2));
	/* Juggle values so we can get rid of references.
	 * FIXME: This breaks when "lhs" and "rhs" depend on each other, and
	 *        one is HREG and the other is HREGIND for the same register...
	 * Solution: there needs to be a function "vnoref(vstackaddr_t n)"
	 *           that can be used to ensure that the top N slots don't
	 *           hold any references, by either moving those references to
	 *           aliases not in the top N slots, or doing the decref (in
	 *           which case, also force dependencies to be loaded) */
	lhs = *memval_direct_getloc(fg_vtop(self) - 1);
	rhs = *memval_direct_getloc(fg_vtop(self) - 0);
	DO(fg_vpush_memloc(self, &lhs)); /* a, b, lhs */
	DO(fg_vpush_memloc(self, &rhs)); /* a, b, lhs, rhs */
	DO(fg_vpop_at(self, 2));         /* a, lhs, rhs */
	DO(fg_vpop_at(self, 2));         /* lhs, rhs */
	lhs = *memval_direct_getloc(fg_vtop(self) - 1);
	rhs = *memval_direct_getloc(fg_vtop(self) - 0);
	DO(fg_vpopmany(self, 2));        /* N/A */
	if (memloc_isconst(&lhs) && memloc_isconst(&rhs)) {
		uintptr_t result;
		uintptr_t lhsval = (uintptr_t)memloc_const_getaddr(&lhs);
		uintptr_t rhsval = (uintptr_t)memloc_const_getaddr(&rhs);
		bool did_overflow = host_arithop_forconst(op, lhsval, rhsval, &result);
		struct host_symbol *dst = did_overflow ? dst_o : dst_no;
		DO(fg_vpush_addr(self, (void const *)result));
		return dst ? fg_gjmp(self, dst) : 0;
	}
	retreg = fg_gallocreg(self, NULL);
	if unlikely(retreg >= HOST_REGNO_COUNT)
		goto err;
	DO(fg_gjarith_locloc2reg(self, op, &lhs, &rhs, retreg, dst_o, dst_no));
	return fg_vpush_hreg(self, retreg, 0);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vjox_arith_enter(struct fungen *__restrict self,
                    /*out*/ struct fg_branch *__restrict branch,
                    host_arithop_t op, unsigned int flags) {
	struct host_section *text = fg_gettext(self);
	struct host_section *cold = branch->fgb_oldtext = text;
	if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE) && (flags & VJX_F_UNLIKELY)) {
		cold = fg_getcold_always(self);
		if unlikely(!cold)
			goto err;
	}
	if (text != cold) {
		struct host_symbol *Ljump;
		Ljump = fg_newsym_named(self, ".Ljump");
		if unlikely(!Ljump)
			goto err;
		DO(fg_varithop(self, op,
		               (flags & VJX_F_JNZ) ? Ljump : NULL,
		               (flags & VJX_F_JNZ) ? NULL : Ljump));
		HA_printf(".section .cold\n");
		DO(fg_settext(self, cold));
		host_symbol_setsect(Ljump, cold);
		branch->fgb_skip = NULL;
	} else {
		struct host_symbol *Lskip;
		Lskip = fg_newsym_named(self, ".Lskip");
		if unlikely(!Lskip)
			goto err;
		DO(fg_varithop(self, op,
		               (flags & VJX_F_JNZ) ? NULL : Lskip,
		               (flags & VJX_F_JNZ) ? Lskip : NULL));
		branch->fgb_skip = Lskip;
	}
	branch->fgb_saved = self->fg_state;
	memstate_incref(branch->fgb_saved);
	return 0;
err:
	return -1;
}


/* Helpers for generating conditional code. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vjx_enter(struct fungen *__restrict self,
             /*inherit(out)*/ struct fg_branch *__restrict branch,
             unsigned int flags) {
	struct memloc testloc;
	struct host_section *text;
	struct host_section *cold;
	DO(fg_vdirect1(self));
	testloc = *fg_vtopdloc(self);
	DO(fg_vpop(self));
	text = fg_gettext(self);
	cold = branch->fgb_oldtext = text;
	if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE) && (flags & VJX_F_UNLIKELY)) {
		cold = fg_getcold_always(self);
		if unlikely(!cold)
			goto err;
	}
	if (text != cold) {
		struct host_symbol *Ljump;
		Ljump = fg_newsym_named(self, ".Ljump");
		if unlikely(!Ljump)
			goto err;
		DO((flags & VJX_F_JNZ) ? fg_gjnz(self, &testloc, Ljump)
		                       : fg_gjz(self, &testloc, Ljump));
		HA_printf(".section .cold\n");
		DO(fg_settext(self, cold));
		host_symbol_setsect(Ljump, cold);
		branch->fgb_skip = NULL;
	} else {
		struct host_symbol *Lskip;
		Lskip = fg_newsym_named(self, ".Lskip");
		if unlikely(!Lskip)
			goto err;
		DO((flags & VJX_F_JNZ) ? fg_gjz(self, &testloc, Lskip)
		                       : fg_gjnz(self, &testloc, Lskip));
		branch->fgb_skip = Lskip;
	}
	branch->fgb_saved = self->fg_state;
	memstate_incref(branch->fgb_saved);
	return 0;
err:
	return -1;
}

/* Force-enter .cold without generating any code. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vcold_enter(struct fungen *__restrict self,
               /*out*/ struct fg_branch *__restrict branch) {
	struct host_section *cold;
	branch->fgb_oldtext = fg_gettext(self);
	cold = fg_getcold_always(self);
	if unlikely(!cold)
		goto err;
	HA_printf(".section .cold\n");
	DO(fg_settext(self, cold));
	branch->fgb_skip  = NULL;
	branch->fgb_saved = self->fg_state;
	memstate_incref(branch->fgb_saved);
	return 0;
err:
	return -1;
}


/* Check if "self" contains references or acts as a dependency. */
PRIVATE WUNUSED NONNULL((1)) bool DCALL
memval_hasref_or_isdep(struct memval *__restrict self) {
	struct memobj *mobj;
	memval_foreach_obj(mobj, self) {
		if (mobj->mo_flags & (MEMOBJ_F_ISREF | MEMOBJ_F_HASDEP))
			return true;
	}
	memval_foreach_obj_end;
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vjex_enter(struct fungen *__restrict self,
              /*inherit(out)*/ struct fg_branch *__restrict branch,
              unsigned int flags) {
	struct host_section *text;
	struct host_section *cold;
	struct memval *temp;
	struct memloc lhsloc, rhsloc;
	DO(fg_vdirect(self, 2));
	temp   = fg_vtop(self);
	rhsloc = *memval_direct_getloc(temp);
	DO(fg_vpop(self)); /* lhs */
	temp   = fg_vtop(self);
	lhsloc = *memval_direct_getloc(temp);
	if (memval_hasref_or_isdep(temp)) {
		/* Special case: poping VTOP may alter registers, so we need to preserve "rhsloc" */
		DO(fg_vpush_memloc(self, &lhsloc)); /* lhs, lhsloc, rhsloc */
		DO(fg_vpush_memloc(self, &rhsloc)); /* lhs, lhsloc, rhsloc */
		DO(fg_vpop_at(self, 3));            /* lhsloc, rhsloc */
		temp   = fg_vtop(self);
		rhsloc = *memval_direct_getloc(temp);
		DO(fg_vpop(self));
		temp   = fg_vtop(self);
		lhsloc = *memval_direct_getloc(temp);
		DO(fg_vpop(self));
	} else {
		DO(fg_vpop(self));
	}
	text = fg_gettext(self);
	cold = branch->fgb_oldtext = text;
	if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE) && (flags & VJX_F_UNLIKELY)) {
		cold = fg_getcold_always(self);
		if unlikely(!cold)
			goto err;
	}
	if (text != cold) {
		struct host_symbol *Ljump;
		Ljump = fg_newsym_named(self, ".Ljump");
		if unlikely(!Ljump)
			goto err;
		DO(fg_gjcc(self, &lhsloc, &rhsloc, false,
		           (flags & VJX_F_JNZ) ? Ljump : NULL,
		           (flags & VJX_F_JNZ) ? NULL : Ljump,
		           (flags & VJX_F_JNZ) ? Ljump : NULL));
		HA_printf(".section .cold\n");
		DO(fg_settext(self, cold));
		host_symbol_setsect(Ljump, cold);
		branch->fgb_skip = NULL;
	} else {
		struct host_symbol *Lskip;
		Lskip = fg_newsym_named(self, ".Lskip");
		if unlikely(!Lskip)
			goto err;
		DO(fg_gjcc(self, &lhsloc, &rhsloc, false,
		           (flags & VJX_F_JNZ) ? NULL : Lskip,
		           (flags & VJX_F_JNZ) ? Lskip : NULL,
		           (flags & VJX_F_JNZ) ? NULL : Lskip));
		branch->fgb_skip = Lskip;
	}
	branch->fgb_saved = self->fg_state;
	memstate_incref(branch->fgb_saved);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vjax_enter(struct fungen *__restrict self,
              /*inherit(out)*/ struct fg_branch *__restrict branch,
              unsigned int flags) {
	struct host_section *text;
	struct host_section *cold;
	struct memval *temp;
	struct memloc lhsloc, rhsloc;
	DO(fg_vdirect(self, 2));
	temp   = fg_vtop(self);
	rhsloc = *memval_direct_getloc(temp);
	DO(fg_vpop(self)); /* lhs */
	temp   = fg_vtop(self);
	lhsloc = *memval_direct_getloc(temp);
	if (memval_hasref_or_isdep(temp)) {
		/* Special case: poping VTOP may alter registers, so we need to preserve "rhsloc" */
		DO(fg_vpush_memloc(self, &lhsloc)); /* lhs, lhsloc, rhsloc */
		DO(fg_vpush_memloc(self, &rhsloc)); /* lhs, lhsloc, rhsloc */
		DO(fg_vpop_at(self, 3));            /* lhsloc, rhsloc */
		temp   = fg_vtop(self);
		rhsloc = *memval_direct_getloc(temp);
		DO(fg_vpop(self));
		temp   = fg_vtop(self);
		lhsloc = *memval_direct_getloc(temp);
		DO(fg_vpop(self));
	} else {
		DO(fg_vpop(self));
	}
	text = fg_gettext(self);
	cold = branch->fgb_oldtext = text;
	if (!(self->fg_assembler->fa_flags & FUNCTION_ASSEMBLER_F_OSIZE) && (flags & VJX_F_UNLIKELY)) {
		cold = fg_getcold_always(self);
		if unlikely(!cold)
			goto err;
	}
	if (text != cold) {
		struct host_symbol *Ljump;
		Ljump = fg_newsym_named(self, ".Ljump");
		if unlikely(!Ljump)
			goto err;
		DO(fg_gjca(self, &lhsloc, &rhsloc,
		           (flags & VJX_F_JNZ) ? Ljump : NULL,
		           (flags & VJX_F_JNZ) ? NULL : Ljump));
		HA_printf(".section .cold\n");
		DO(fg_settext(self, cold));
		host_symbol_setsect(Ljump, cold);
		branch->fgb_skip = NULL;
	} else {
		struct host_symbol *Lskip;
		Lskip = fg_newsym_named(self, ".Lskip");
		if unlikely(!Lskip)
			goto err;
		DO(fg_gjca(self, &lhsloc, &rhsloc,
		           (flags & VJX_F_JNZ) ? NULL : Lskip,
		           (flags & VJX_F_JNZ) ? Lskip : NULL));
		branch->fgb_skip = Lskip;
	}
	branch->fgb_saved = self->fg_state;
	memstate_incref(branch->fgb_saved);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vjx_leave(struct fungen *__restrict self,
             /*inherit(always)*/ struct fg_branch *__restrict branch) {
	ASSERT(branch->fgb_saved);
	EDO(err_saved, fg_vmorph(self, branch->fgb_saved));
	memstate_decref(branch->fgb_saved);
	if (fg_gettext(self) != branch->fgb_oldtext) {
		fg_DEFINE_host_symbol_section(self, err, Lreturn, branch->fgb_oldtext, 0);
		DO(fg_gjmp(self, Lreturn));
		HA_printf(".section .text\n");
		DO(fg_settext(self, branch->fgb_oldtext));
		HA_printf(".Lreturn:\n");
	}
	if (branch->fgb_skip)
		host_symbol_setsect(branch->fgb_skip, fg_gettext(self));
	DBG_memset(branch, 0xcc, sizeof(*branch));
	return 0;
err_saved:
	memstate_decref(branch->fgb_saved);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vjx_leave_noreturn(struct fungen *__restrict self,
                      /*inherit(always)*/ struct fg_branch *__restrict branch) {
	ASSERT(branch->fgb_saved);
	memstate_decref(self->fg_state);
	self->fg_state = branch->fgb_saved; /* Inherit */
	if (self->fg_sect != branch->fgb_oldtext) {
		HA_printf(".section .text\n");
		DO(fg_settext(self, branch->fgb_oldtext));
	}
	if (branch->fgb_skip)
		host_symbol_setsect(branch->fgb_skip, self->fg_sect);
	DBG_memset(branch, 0xcc, sizeof(*branch));
	return 0;
err:
	return -1;
}



/* Pre-defined exception injectors. */

/* `fei_inject' value for `struct fungen_exceptinject_callvoidapi' */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fungen_exceptinject_callvoidapi_f(struct fungen *__restrict self,
                                  struct fungen_exceptinject *__restrict inject) {
	struct fungen_exceptinject_callvoidapi *me;
	me = (struct fungen_exceptinject_callvoidapi *)inject;
	return fg_vcallapi(self, me->fei_cva_func, VCALL_CC_VOID_NX, me->fei_cva_argc);
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C */
