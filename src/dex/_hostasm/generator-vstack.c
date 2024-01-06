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
	if unlikely(DeeObject_AssertTypeExact(constant, type))
		goto err;
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
		if unlikely(_Dee_function_generator_vpush_xlocal(self, MEMSTATE_XLOCAL_A_FUNC))
			goto err;
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
		struct Dee_memloc *loc;
		Dee_aid_t opt_aid = aid - code->co_argc_min;
		Dee_lid_t xlid = MEMSTATE_XLOCAL_DEFARG(opt_aid);
		Dee_lid_t lid = self->fg_assembler->fa_localc + xlid;
		DeeObject *default_value = code->co_defaultv[opt_aid];
		if (default_value != NULL)
			return Dee_function_generator_vpush_const(self, Dee_True);
		loc = &self->fg_state->ms_localv[lid];
		if (loc->ml_flags & MEMLOC_F_LOCAL_BOUND)
			return Dee_function_generator_vpush_const(self, Dee_True);
		if (loc->ml_flags & MEMLOC_F_LOCAL_UNBOUND)
			return Dee_function_generator_vpush_const(self, Dee_False);
		if (loc->ml_type != MEMLOC_TYPE_UNALLOC) {
			/* Check if the already-allocated argument location contains a value. */
			if unlikely(Dee_function_generator_vpush(self, loc))
				goto err;
			if unlikely(Dee_function_generator_vdirect(self, 1))
				goto err;
			loc = Dee_function_generator_vtop(self);
			ASSERT(MEMLOC_VMORPH_ISDIRECT(loc->ml_vmorph));
			loc->ml_vmorph = MEMLOC_VMORPH_TESTNZ(loc->ml_vmorph);
			return 0;
		}

		/* Argument hasn't been accessed or populated by the keyword loader.
		 * -> Simply check if the argument is present positionally:
		 *         argc >= aid
		 *    <=>  argc > (aid - 1)
		 *    <=>  argc - (aid - 1) > 0
		 */
		if unlikely(Dee_function_generator_vpush_argc(self))
			goto err;
		if unlikely(Dee_function_generator_vdelta(self, -((ptrdiff_t)aid - 1)))
			goto err;
		if unlikely(Dee_function_generator_vdirect(self, 1))
			goto err;
		loc = Dee_function_generator_vtop(self);
		ASSERT(MEMLOC_VMORPH_ISDIRECT(loc->ml_vmorph));
		loc->ml_vmorph = MEMLOC_VMORPH_BOOL_GZ;
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
		struct Dee_memloc *args_or_argv_loc;
		ptrdiff_t ind_offset = (ptrdiff_t)aid * sizeof(DeeObject *);
		if (self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE)
			ind_offset += offsetof(DeeTupleObject, t_elem);
		args_or_argv_loc = &self->fg_state->ms_localv[self->fg_assembler->fa_localc +
		                                              MEMSTATE_XLOCAL_A_ARGV];
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
			if unlikely(Dee_function_generator_vdel_local(self, delete_lid))
				goto err;
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
					temp = *self->fg_nextlastloc;
					memmovedownc(self->fg_nextlastloc,
					             self->fg_nextlastloc + 1,
					             (size_t)(item - self->fg_nextlastloc),
					             sizeof(struct Dee_basic_block_loclastread));
					*item = temp;
					item = self->fg_nextlastloc;
				}
				ASSERT(item == self->fg_nextlastloc);
				ASSERT(item->bbl_instr == instr);
				ASSERT(item->bbl_lid == lid);
				if unlikely(Dee_function_generator_vdel_local(self, lid))
					goto err;
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
	if unlikely(delete_unused_local_after_read(self, instr, lid))
		goto err;
	return 0;
err:
	return -1;
}

/* `instr' is needed for automatic deletion of unused locals */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vbound_local(struct Dee_function_generator *__restrict self,
                                    Dee_instruction_t const *instr, Dee_lid_t lid) {
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
	if ((src_loc->ml_type == MEMLOC_TYPE_UNALLOC) ||
	    (src_loc->ml_flags & MEMLOC_F_LOCAL_UNBOUND)) {
		/* Variable is always unbound -> return "false" */
		return Dee_function_generator_vpush_const(self, Dee_False);
	} else if (src_loc->ml_flags & MEMLOC_F_LOCAL_BOUND) {
		/* Variable is always bound -> return "true" */
		return Dee_function_generator_vpush_const(self, Dee_True);
	}
	dst_loc = &state->ms_stackv[state->ms_stackc];
	*dst_loc = *src_loc;
	dst_loc->ml_flags &= ~MEMLOC_M_LOCAL_BSTATE;
	dst_loc->ml_flags |= MEMLOC_F_NOREF; /* alias! (so no reference) */
	if (MEMLOC_TYPE_HASREG(dst_loc->ml_type))
		Dee_memstate_incrinuse(state, dst_loc->ml_value.v_hreg.r_regno);
	++state->ms_stackc;
	if unlikely(delete_unused_local_after_read(self, instr, lid))
		goto err;
	if unlikely(Dee_function_generator_gdirect(self, dst_loc))
		goto err;
	ASSERT(MEMLOC_VMORPH_ISDIRECT(dst_loc->ml_vmorph));
	dst_loc->ml_vmorph = MEMLOC_VMORPH_TESTNZ(dst_loc->ml_vmorph);
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
		if unlikely(has_ref_alias
		            ? Dee_function_generator_gdecref_nokill(self, loc, 1)
		            : (loc->ml_flags & MEMLOC_F_ONEREF)
		              ? Dee_function_generator_gdecref_dokill(self, loc) /* TODO: If types are known, inline DeeObject_Destroy() as tp_fini() + DeeType_FreeInstance() */
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
		if unlikely(has_ref_alias
		            ? Dee_function_generator_gdecref_nokill(self, loc, 1)
		            : (loc->ml_flags & MEMLOC_F_ONEREF)
		              ? Dee_function_generator_gdecref_dokill(self, loc) /* TODO: If types are known, inline DeeObject_Destroy() as tp_fini() + DeeType_FreeInstance() */
		              : Dee_function_generator_gdecref(self, loc, 1))
			goto err;
	} else {
		/* Location is conditionally bound.
		 * Check if there is another conditionally bound
		 * location which we can off-load the decref onto. */
		Dee_lid_t i;
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
                                  Dee_lid_t lid) {
	struct Dee_memstate *state;
	struct Dee_memloc *dst;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	state = self->fg_state;
	ASSERT(lid < state->ms_localc);
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
	*dst = *Dee_memstate_vtop(state);
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
                                  Dee_lid_t lid) {
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


/* Check if top `n' elements are all `MEMLOC_TYPE_CONST' */
INTDEF WUNUSED NONNULL((1)) bool DCALL
Dee_function_generator_vallconst(struct Dee_function_generator *__restrict self,
                                 Dee_vstackaddr_t n) {
	struct Dee_memstate *state = self->fg_state;
	struct Dee_memloc *itemv;
	Dee_vstackaddr_t i;
	ASSERT(n <= state->ms_stackc);
	itemv = state->ms_stackv + state->ms_stackc - n;
	for (i = 0; i < n; ++i) {
		if (itemv[i].ml_type != MEMLOC_TYPE_CONST)
			return false;
	}
	return true;
}

/* Remember that VTOP, as well as any other memory location
 * that might be aliasing it is an instance of "type" at runtime. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vsettyp(struct Dee_function_generator *__restrict self,
                               DeeTypeObject *type) {
	struct Dee_memstate *state;
	struct Dee_memloc *vtop;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	state = self->fg_state;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	vtop = Dee_memstate_vtop(state);
	ASSERTF(vtop->ml_vmorph == MEMLOC_VMORPH_DIRECT,
	        "Can only assign types to direct values");
	if (vtop->ml_valtyp != type) {
		struct Dee_memloc *alias;
		Dee_memstate_foreach(alias, state) {
			if (Dee_memloc_sameloc(alias, vtop))
				alias->ml_valtyp = type;
		}
		Dee_memstate_foreach_end;
		ASSERT(vtop->ml_valtyp == type);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vsettyp_noalias(struct Dee_function_generator *__restrict self,
                                       DeeTypeObject *type) {
	struct Dee_memstate *state;
	struct Dee_memloc *vtop;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	state = self->fg_state;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	vtop = Dee_memstate_vtop(state);
	ASSERTF(vtop->ml_vmorph == MEMLOC_VMORPH_DIRECT,
	        "Can only assign types to direct values");
	if (vtop->ml_valtyp != type) {
#ifndef NDEBUG
		struct Dee_memloc *alias;
		Dee_memstate_foreach(alias, state) {
			ASSERT(alias == vtop || !Dee_memloc_sameloc(alias, vtop));
		}
		Dee_memstate_foreach_end;
#endif /* !NDEBUG */
		vtop->ml_valtyp = type;
	}
	return 0;
err:
	return -1;
}


/* VTOP = VTOP == <value> */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_veqconstaddr(struct Dee_function_generator *__restrict self,
                                    DeeObject *value) {
	/* Kind-of weird, but works perfectly and can use vmorph mechanism:
	 * >> PUSH((POP() - <value>) == 0); */
	struct Dee_memloc *loc;
	if unlikely(Dee_function_generator_vdelta(self, -(ptrdiff_t)(uintptr_t)value))
		goto err;
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;
	loc = Dee_function_generator_vtop(self);
	ASSERT(MEMLOC_VMORPH_ISDIRECT(loc->ml_vmorph));
	loc->ml_vmorph = MEMLOC_VMORPH_BOOL_Z;
	return 0;
err:
	return -1;
}

/* PUSH(POP() == POP()); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_veqaddr(struct Dee_function_generator *__restrict self) {
	DeeObject *retval;
	struct Dee_memloc *a, *b;
	if unlikely(self->fg_state->ms_stackc < 2)
		return err_illegal_stack_effect();
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	b = Dee_function_generator_vtop(self);
	a = b - 1;

	/* If either of the 2 locations is a constant, then
	 * we can use `Dee_function_generator_veqconstaddr()' */
	if (a->ml_type == MEMLOC_TYPE_CONST && a->ml_vmorph == b->ml_vmorph) {
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		goto do_constant;
	} else if (b->ml_type == MEMLOC_TYPE_CONST && a->ml_vmorph == b->ml_vmorph) {
		DeeObject *const_value;
do_constant:
		ASSERT(b->ml_type == MEMLOC_TYPE_CONST);
		ASSERT(a->ml_vmorph == b->ml_vmorph);
		a->ml_vmorph = MEMLOC_VMORPH_DIRECT;
		b->ml_vmorph = MEMLOC_VMORPH_DIRECT;
		const_value = b->ml_value.v_const;
		if (a->ml_type == MEMLOC_TYPE_CONST) {
			/* Special case: Compare addresses of 2 constants */
			retval = DeeBool_For(a->ml_value.v_const == const_value);
do_return_retval:
			if unlikely(Dee_function_generator_vpopmany(self, 2))
				goto err;
			return Dee_function_generator_vpush_const(self, retval);
		}
		if unlikely(Dee_function_generator_vpop(self))
			goto err;
		return Dee_function_generator_veqconstaddr(self, const_value);
	}
	if unlikely(Dee_function_generator_vdirect(self, 2))
		goto err;
	if (a->ml_type == MEMLOC_TYPE_CONST) {
		if unlikely(Dee_function_generator_vswap(self))
			goto err;
		goto do_constant;
	} else if (b->ml_type == MEMLOC_TYPE_CONST) {
		goto do_constant;
	}

	/* Another special case: do the 2 locations alias each other? */
	if (Dee_memloc_sameloc(a, b)) {
		retval = Dee_True;
		goto do_return_retval;
	}

	/* TODO: Arch-specific code here:
	 * >> xorP  %retreg, %retreg
	 * >> cmpP  <a>, <b>
	 * >> sete  %retreg
	 *
	 * Note that when ASM_CMP_SO/ASM_CMP_DO is followed by ASM_JT/ASM_JF, then
	 * `Dee_function_generator_veqaddr()' isn't used and `_Dee_function_generator_gjcmp()'
	 * is used to do the compare+jump between the 2 operands. */
	return DeeError_NOTIMPLEMENTED();
err:
	return -1;
}


/* >> if (THIRD == SECOND) // Address-based
 * >>     THIRD = FIRST;
 * >> POP();
 * >> POP(); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcoalesce(struct Dee_function_generator *__restrict self) {
	DREF struct Dee_memstate *common_state;
	struct Dee_host_symbol *text_Lalready_equal;
	struct Dee_memloc *dst, *p_coalesce_from, *p_coalesce_to;
	struct Dee_memloc coalesce_from;
	if unlikely(self->fg_state->ms_stackc < 3)
		return err_illegal_stack_effect();
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	p_coalesce_to   = Dee_function_generator_vtop(self);
	p_coalesce_from = p_coalesce_to - 1;
	dst             = p_coalesce_from - 1;
	if (dst->ml_type == MEMLOC_TYPE_CONST) {
		struct Dee_memloc temp;
		temp = *dst;
		*dst = *p_coalesce_from;
		*p_coalesce_from = temp;
	}
	if (Dee_memloc_sameval(p_coalesce_from, p_coalesce_to) ||
	    Dee_memloc_sameval(dst, p_coalesce_from)) {
		/* Special case: result is always "coalesce_to" */
		if unlikely(Dee_function_generator_vrrot(self, 3))
			goto err;
		if unlikely(Dee_function_generator_vpop(self))
			goto err;
		return Dee_function_generator_vpop(self);
	}

	/* Fallback: generate code to branch at runtime. */
	if unlikely(Dee_function_generator_vlrot(self, 3))
		goto err; /* from, to, dst */
	if unlikely(Dee_function_generator_vreg(self, NULL))
		goto err; /* from, to, reg:dst */
	if unlikely(Dee_function_generator_vrrot(self, 3))
		goto err; /* reg:dst, from, to */
	coalesce_from = *p_coalesce_from;
	if unlikely(Dee_function_generator_vswap(self))
		goto err; /* reg:dst, to, from */
	if unlikely(Dee_function_generator_vpop(self))
		goto err; /* reg:dst, to */
	ASSERT(p_coalesce_from == Dee_function_generator_vtop(self));
	ASSERT(p_coalesce_to == Dee_function_generator_vtop(self) + 1);
	text_Lalready_equal = Dee_function_generator_newsym_named(self, ".Lalready_equal");
	if unlikely(!text_Lalready_equal)
		goto err;
	common_state = Dee_memstate_copy(self->fg_state);
	if unlikely(!common_state)
		goto err;
	if unlikely(_Dee_function_generator_gjcmp(self, dst, &coalesce_from, false,
	                                          NULL, text_Lalready_equal, NULL))
		goto err_common_state;
	if unlikely(Dee_function_generator_vswap(self))
		goto err_common_state; /* to, reg:dst */
	if unlikely(Dee_function_generator_vpop(self))
		goto err_common_state; /* to */
	if unlikely(Dee_function_generator_vdup(self))
		goto err_common_state; /* to, to */
	if unlikely(Dee_function_generator_vmorph(self, common_state))
		goto err_common_state; /* ... */
	Dee_memstate_decref(common_state);
	Dee_host_symbol_setsect(text_Lalready_equal, self->fg_sect);
	return Dee_function_generator_vpop(self);
err_common_state:
	Dee_memstate_decref(common_state);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcoalesce_c(struct Dee_function_generator *__restrict self,
                                   void const *from, void const *to) {
	if unlikely(Dee_function_generator_vpush_addr(self, from))
		goto err;
	if unlikely(Dee_function_generator_vpush_addr(self, to))
		goto err;
	return Dee_function_generator_vcoalesce(self);
err:
	return -1;
}


/* Force the top `n' elements of the v-stack to use `MEMLOC_VMORPH_ISDIRECT'.
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
		struct Dee_memloc *loc = &state->ms_stackv[i];
		if (!MEMLOC_VMORPH_ISDIRECT(loc->ml_vmorph)) {
			if (Dee_memstate_isshared(state)) {
				state = Dee_memstate_copy(state);
				if unlikely(!state)
					goto err;
				Dee_memstate_decref_nokill(self->fg_state);
				self->fg_state = state;
				loc = &state->ms_stackv[i];
			}
			if unlikely(Dee_function_generator_gdirect(self, loc))
				goto err;
		}
	}
	return 0;
err:
	return -1;
}

/* Clear the `MEMLOC_F_ONEREF' flag for the top `n' v-stack elements,
 * as well as any other memory location that might be aliasing them. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vnotoneref(struct Dee_function_generator *__restrict self,
                                  Dee_vstackaddr_t n) {
	Dee_vstackaddr_t i;
	struct Dee_memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < n)
		return err_illegal_stack_effect();
	for (i = state->ms_stackc - n; i < state->ms_stackc; ++i) {
		struct Dee_memloc *loc = &state->ms_stackv[i];
		if (loc->ml_flags & MEMLOC_F_ONEREF) {
			if (Dee_memstate_isshared(state)) {
				state = Dee_memstate_copy(state);
				if unlikely(!state)
					goto err;
				Dee_memstate_decref_nokill(self->fg_state);
				self->fg_state = state;
				loc = &state->ms_stackv[i];
			}
			if unlikely(Dee_function_generator_gnotoneref_impl(self, loc))
				goto err;
		}
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vnotoneref_at(struct Dee_function_generator *__restrict self,
                                     Dee_vstackaddr_t off) {
	struct Dee_memloc *loc;
	struct Dee_memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < off)
		return err_illegal_stack_effect();
	loc = &state->ms_stackv[state->ms_stackc - off];
	if (loc->ml_flags & MEMLOC_F_ONEREF) {
		if (Dee_memstate_isshared(state)) {
			state = Dee_memstate_copy(state);
			if unlikely(!state)
				goto err;
			Dee_memstate_decref_nokill(self->fg_state);
			self->fg_state = state;
			loc = &state->ms_stackv[state->ms_stackc - off];
		}
		if unlikely(Dee_function_generator_gnotoneref_impl(self, loc))
			goto err;
	}
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
		struct Dee_memloc *loc = &state->ms_stackv[i];
		if (loc->ml_flags & MEMLOC_F_ONEREF) {
			DeeTypeObject *loctype = Dee_memloc_typeof(loc);
			if (loctype != NULL && DeeType_IsOperatorNoRefEscape(loctype, operator_name))
				continue; /* Type is known to not let references to its instances escape. */
			if (Dee_memstate_isshared(state)) {
				state = Dee_memstate_copy(state);
				if unlikely(!state)
					goto err;
				Dee_memstate_decref_nokill(self->fg_state);
				self->fg_state = state;
				loc = &state->ms_stackv[i];
			}
			if unlikely(Dee_function_generator_gnotoneref_impl(self, loc))
				goto err;
		}
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vnotoneref_if_operator_at(struct Dee_function_generator *__restrict self,
                                                 uint16_t operator_name, Dee_vstackaddr_t off) {
	struct Dee_memloc *loc;
	struct Dee_memstate *state = self->fg_state;
	if unlikely(state->ms_stackc < off)
		return err_illegal_stack_effect();
	loc = &state->ms_stackv[state->ms_stackc - off];
	if (loc->ml_flags & MEMLOC_F_ONEREF) {
		DeeTypeObject *loctype = Dee_memloc_typeof(loc);
		if (loctype != NULL && DeeType_IsOperatorNoRefEscape(loctype, operator_name))
			return 0; /* Type is known to not let references to its instances escape. */
		if (Dee_memstate_isshared(state)) {
			state = Dee_memstate_copy(state);
			if unlikely(!state)
				goto err;
			Dee_memstate_decref_nokill(self->fg_state);
			self->fg_state = state;
			loc = &state->ms_stackv[state->ms_stackc - off];
		}
		if unlikely(Dee_function_generator_gnotoneref_impl(self, loc))
			goto err;
	}
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
		if unlikely(Dee_function_generator_vpush_const(self, default_value))
			goto err; /* default_value */
		if unlikely(Dee_function_generator_vreg(self, NULL))
			goto err; /* reg:default_value */

		/* Check if the caller has provided enough arguments. */
		if unlikely(Dee_function_generator_vpush_argc(self))
			goto err; /* reg:default_value, argc */
		Luse_default = Dee_function_generator_newsym_named(self, ".Luse_default");
		if unlikely(!Luse_default)
			goto err;
		{
			struct Dee_memloc l_argc, l_aid;
			l_argc = *Dee_function_generator_vtop(self);
			ASSERT(l_argc.ml_flags & MEMLOC_F_NOREF);
			--self->fg_state->ms_stackc;
			if (MEMLOC_TYPE_HASREG(l_argc.ml_type))
				Dee_memstate_decrinuse(self->fg_state, l_argc.ml_value.v_hreg.r_regno);
			if unlikely(Dee_function_generator_gdirect(self, &l_argc))
				goto err;
			l_aid.ml_type          = MEMLOC_TYPE_CONST;
			l_aid.ml_value.v_const = (DeeObject *)(uintptr_t)aid;
			if unlikely(_Dee_function_generator_gjcmp(self, &l_aid, &l_argc, false, NULL,
			                                          Luse_default, Luse_default))
				goto err;
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
	
			if unlikely(Dee_function_generator_vpush_argc(self))
				goto err;
			l_argc = *Dee_function_generator_vtop(self);
			ASSERT(l_argc.ml_flags & MEMLOC_F_NOREF);
			--self->fg_state->ms_stackc;
			if (MEMLOC_TYPE_HASREG(l_argc.ml_type))
				Dee_memstate_decrinuse(self->fg_state, l_argc.ml_value.v_hreg.r_regno);
			if unlikely(Dee_function_generator_gdirect(self, &l_argc))
				goto err;
			l_aid.ml_type          = MEMLOC_TYPE_CONST;
			l_aid.ml_value.v_const = (DeeObject *)(uintptr_t)aid;
			if unlikely(_Dee_function_generator_gjcmp(self, &l_aid, &l_argc, false,
			                                          text != cold ? NULL : Ltarget,
			                                          text != cold ? Ltarget : NULL,
			                                          text != cold ? Ltarget : NULL))
				goto err;
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
		if unlikely(Dee_function_generator_vpush_argv(self))
			goto err; /* argv */
		if unlikely(Dee_function_generator_vind(self, (ptrdiff_t)aid * sizeof(DeeObject *)))
			goto err; /* argv[aid] */
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
	if unlikely(Dee_function_generator_vpush_argc(self))
		goto err; /* argc */
	if unlikely(Dee_function_generator_vdelta(self, -(ptrdiff_t)co_argc_max))
		goto err; /* argc-co_argc_max */
	if (co_argc_min < co_argc_max && self->fg_state->ms_uargc_min < co_argc_max) {
		/* TODO: FIXME: If `argc-co_argc_max' rolls over or is 0, then we have to push an empty tuple
		 *              This is because less than `co_argc_max' may be provided by the caller if the
		 *              function also takes default/optional arguments. */
	}
	if unlikely(Dee_function_generator_vpush_argv(self))
		goto err; /* argc-co_argc_max, argv */
	if unlikely(Dee_function_generator_vdelta(self, (ptrdiff_t)co_argc_max * sizeof(DeeObject *)))
		goto err; /* argc-co_argc_max, argv+co_argc_max */
	if unlikely(Dee_function_generator_vcallapi(self, &DeeTuple_NewVector, VCALL_CC_OBJECT, 2))
		goto err; /* varargs */
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_ONEREF;
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
	struct Dee_memloc *dst_loc, *src_loc;
	struct Dee_memstate *state;

	/* Push the initializer for the x-local onto the v-stack. */
	if unlikely(Dee_function_generator_vpushinit_xlocal(self, instr, xlid))
		goto err;

	/* Hard-transfer the pushed value of into the local-slot */
	state = self->fg_state;
	ASSERT(state->ms_stackc >= 1);
	dst_loc = &state->ms_localv[self->fg_assembler->fa_localc + xlid];
	src_loc = &state->ms_stackv[state->ms_stackc - 1];
	if (MEMLOC_TYPE_HASREG(dst_loc->ml_type))
		Dee_memstate_decrinuse(state, dst_loc->ml_value.v_hreg.r_regno);
	*dst_loc = *src_loc;
	dst_loc->ml_flags &= ~MEMLOC_M_LOCAL_BSTATE;
	dst_loc->ml_flags |= MEMLOC_F_LOCAL_BOUND;
	--state->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_xlocal(struct Dee_function_generator *__restrict self,
                                    Dee_instruction_t const *instr, Dee_lid_t xlid) {
	DREF struct Dee_memstate *common_state;
	struct Dee_memloc *loc;

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
			return Dee_function_generator_vpush_args(self);
		break;

	case MEMSTATE_XLOCAL_VARKWDS:
		if unlikely(!(self->fg_assembler->fa_code->co_flags & CODE_FVARKWDS))
			return err_unsupported_opcode(self->fg_assembler->fa_code, instr);
		break;

	default: break;
	}

	/* Check if the slot needs to be initialized (and if so: initialize it) */
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	loc = &self->fg_state->ms_localv[self->fg_assembler->fa_localc + xlid];
	if (loc->ml_flags & MEMLOC_F_LOCAL_UNBOUND) {
		if unlikely(Dee_function_generator_vinit_xlocal(self, instr, xlid))
			goto err;
	} else if (!(loc->ml_flags & MEMLOC_F_LOCAL_BOUND)) {
		struct Dee_host_symbol *Lskipinit;
		if (loc->ml_type != MEMLOC_TYPE_HSTACKIND &&
		    loc->ml_type != MEMLOC_TYPE_HREG) {
			if unlikely(Dee_function_generator_greg(self, loc, NULL))
				goto err;
			ASSERT(loc->ml_type == MEMLOC_TYPE_HREG);
		}
		Lskipinit = Dee_function_generator_newsym_named(self, ".Lskipinit");
		if unlikely(!Lskipinit)
			goto err;
		if unlikely(_Dee_function_generator_gjnz(self, loc, Lskipinit))
			goto err;
		if unlikely(Dee_function_generator_state_unshare(self))
			goto err;
		common_state = Dee_memstate_copy(self->fg_state);
		if unlikely(!common_state)
			goto err;
		loc = &self->fg_state->ms_localv[self->fg_assembler->fa_localc + xlid];
		ASSERT(!(loc->ml_flags & MEMLOC_M_LOCAL_BSTATE));
		loc->ml_flags |= MEMLOC_F_LOCAL_UNBOUND; /* Known to always be NULL if the branch isn't taken. */
		loc = &common_state->ms_localv[self->fg_assembler->fa_localc + xlid];
		ASSERT(!(loc->ml_flags & MEMLOC_M_LOCAL_BSTATE));
		loc->ml_flags |= MEMLOC_F_LOCAL_BOUND; /* Known to always be bound in the end. */
		if unlikely(Dee_function_generator_vinit_xlocal(self, instr, xlid))
			goto err_common_state;
		ASSERT(!(self->fg_state->ms_localv[self->fg_assembler->fa_localc + xlid].ml_flags & MEMLOC_F_LOCAL_UNBOUND));
		ASSERT(self->fg_state->ms_localv[self->fg_assembler->fa_localc + xlid].ml_flags & MEMLOC_F_LOCAL_BOUND);
		if unlikely(Dee_function_generator_vmorph(self, common_state))
			goto err_common_state;
		Dee_memstate_decref(common_state);
		Dee_host_symbol_setsect(Lskipinit, self->fg_sect);
	}
	ASSERT(!(self->fg_state->ms_localv[self->fg_assembler->fa_localc + xlid].ml_flags & MEMLOC_F_LOCAL_UNBOUND));
	ASSERT(self->fg_state->ms_localv[self->fg_assembler->fa_localc + xlid].ml_flags & MEMLOC_F_LOCAL_BOUND);
	return _Dee_function_generator_vpush_xlocal(self, xlid);
err_common_state:
	Dee_memstate_decref(common_state);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_this_function(struct Dee_function_generator *__restrict self) {
	if (self->fg_assembler->fa_cc & HOSTFUNC_CC_F_FUNC)
		return _Dee_function_generator_vpush_xlocal(self, MEMSTATE_XLOCAL_A_FUNC);
	return Dee_function_generator_vpush_const(self, self->fg_assembler->fa_function);
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_argc(struct Dee_function_generator *__restrict self) {
	if (!(self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE))
		return _Dee_function_generator_vpush_xlocal(self, MEMSTATE_XLOCAL_A_ARGC);
	if unlikely(_Dee_function_generator_vpush_xlocal(self, MEMSTATE_XLOCAL_A_ARGS))
		goto err;
	return Dee_function_generator_vind(self, offsetof(DeeTupleObject, t_size));
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_argv(struct Dee_function_generator *__restrict self) {
	if (!(self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE))
		return _Dee_function_generator_vpush_xlocal(self, MEMSTATE_XLOCAL_A_ARGV);
	if unlikely(_Dee_function_generator_vpush_xlocal(self, MEMSTATE_XLOCAL_A_ARGS))
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

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_except(struct Dee_function_generator *__restrict self) {
	if unlikely(Dee_function_generator_vpush_usage(self, DEE_HOST_REGUSAGE_THREAD))
		goto err; /* DeeThread_Self() */
	if unlikely(Dee_function_generator_vind(self, offsetof(DeeThreadObject, t_except)))
		goto err; /* DeeThread_Self()->t_except */
	/* Check if there is an active exception if not already checked. */
	if (!(self->fg_state->ms_flags & MEMSTATE_F_GOTEXCEPT)) {
		int temp;
		DREF struct Dee_memstate *saved_state;
		struct Dee_host_section *text = self->fg_sect;
		struct Dee_host_section *cold = &self->fg_block->bb_hcold;
		if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
			cold = text;
		if unlikely(Dee_function_generator_vdirect(self, 1))
			goto err;
		if (text == cold) {
			struct Dee_host_symbol *text_Ldone;
			text_Ldone = Dee_function_generator_newsym_named(self, ".Ldone");
			if unlikely(!text_Ldone)
				goto err;
			if unlikely(_Dee_function_generator_gjnz(self, Dee_function_generator_vtop(self), text_Ldone))
				goto err;
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
			if unlikely(_Dee_function_generator_gjz(self, Dee_function_generator_vtop(self), Lerr_no_active_exception))
				goto err;
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
	if unlikely(Dee_function_generator_vpush_const(self, class_type))
		goto err;
	if unlikely(Dee_function_generator_vpush_imm16(self, addr))
		goto err;
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
		if unlikely(Dee_function_generator_vpush_const(self, class_type))
			goto err;
		if unlikely(Dee_function_generator_vpush_imm16(self, addr))
			goto err;
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
	if ((flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF) &&
	    unlikely(Dee_function_generator_grwlock_read_const(self, &desc->cd_lock)))
		goto err;
	if unlikely(Dee_function_generator_vpush_addr(self, &desc->cd_members[addr]))
		goto err; /* p_value */
	if unlikely(Dee_function_generator_vind(self, 0))
		goto err; /* *p_value */
	if unlikely(Dee_function_generator_vreg(self, NULL))
		goto err; /* reg:value */
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;
	text = self->fg_sect;
	cold = &self->fg_block->bb_hcold;
	if (self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_OSIZE)
		cold = text;
	if (cold == text) {
		struct Dee_host_symbol *text_Lbound;
		text_Lbound = Dee_function_generator_newsym_named(self, ".Lbound");
		if unlikely(!text_Lbound)
			goto err;
		if unlikely(_Dee_function_generator_gjnz(self, Dee_function_generator_vtop(self), text_Lbound))
			goto err;
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
		if unlikely(_Dee_function_generator_gjz(self, Dee_function_generator_vtop(self), cold_Lunbound_member))
			goto err;
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
	ASSERT(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF);
	if (!(flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF))
		return 0;
	if unlikely(Dee_function_generator_gincref(self, Dee_function_generator_vtop(self), 1))
		goto err;
	Dee_function_generator_vtop(self)->ml_flags &= ~MEMLOC_F_NOREF;
	return Dee_function_generator_grwlock_endread_const(self, &desc->cd_lock);
err:
	return -1;
}

/* type -> value */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_cmember(struct Dee_function_generator *__restrict self,
                                     uint16_t addr, unsigned int flags) {
	struct Dee_memloc *type_loc;
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;
	type_loc = Dee_function_generator_vtop(self);
	if (type_loc->ml_type == MEMLOC_TYPE_CONST) {
		struct class_desc *desc;
		DeeTypeObject *class_type = (DeeTypeObject *)type_loc->ml_value.v_const;
		if unlikely(DeeObject_AssertType(class_type, &DeeType_Type))
			goto err;
		if unlikely(!DeeType_IsClass(class_type))
			return libhostasm_rt_err_requires_class(class_type);
		desc = DeeClass_DESC(class_type);
		if unlikely(addr >= desc->cd_desc->cd_cmemb_size)
			return libhostasm_rt_err_invalid_class_addr(class_type, addr);
		if unlikely(Dee_function_generator_vpop(self))
			goto err; /* Get rid of the `class_type' v-stack item. */
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
	if unlikely(Dee_function_generator_vpush_imm16(self, addr))
		goto err;
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
	struct Dee_memloc *type_loc, *vtop;
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;
	type_loc = Dee_function_generator_vtop(self);
	if (type_loc->ml_type == MEMLOC_TYPE_CONST) {
		DeeObject **p_valloc;
		struct class_desc *desc;
		DeeTypeObject *class_type = (DeeTypeObject *)type_loc->ml_value.v_const;
		if unlikely(DeeObject_AssertType(class_type, &DeeType_Type))
			goto err;
		if unlikely(!DeeType_IsClass(class_type))
			return libhostasm_rt_err_requires_class(class_type);
		desc = DeeClass_DESC(class_type);
		if unlikely(addr >= desc->cd_desc->cd_cmemb_size)
			return libhostasm_rt_err_invalid_class_addr(class_type, addr);
		if unlikely(Dee_function_generator_vpop(self))
			goto err; /* N/A */
		p_valloc = &desc->cd_members[addr];
		if (!(self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_NOROINLINE) &&
		    atomic_read(p_valloc) != NULL &&
		    DeeClassDescriptor_IsClassAttributeReadOnly(desc->cd_desc, addr)) {
			return Dee_function_generator_vpush_const(self, Dee_True);
		}
		if unlikely(Dee_function_generator_vpush_addr(self, p_valloc))
			goto err; /* p_valloc */
		if unlikely(Dee_function_generator_vind(self, 0))
			goto err; /* *p_valloc */
		if unlikely(Dee_function_generator_vreg(self, NULL))
			goto err; /* reg:*p_valloc */
		if unlikely(Dee_function_generator_vdirect(self, 1))
			goto err; /* reg:*p_valloc */
		vtop = Dee_function_generator_vtop(self);
		ASSERT(MEMLOC_VMORPH_ISDIRECT(vtop->ml_vmorph));
		vtop->ml_vmorph = MEMLOC_VMORPH_BOOL_NZ;
		return 0;
	}

	/* Fallback: access the class member at runtime */
	if ((self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_SAFE) ||
	    (flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE)) {
		if unlikely(Dee_function_generator_vpush_imm16(self, addr))
			goto err;
		if unlikely(Dee_function_generator_vcallapi(self, &DeeClass_BoundMemberSafe, VCALL_CC_NEGINT, 2))
			goto err;
		vtop = Dee_function_generator_vtop(self);
		ASSERT(MEMLOC_VMORPH_ISDIRECT(vtop->ml_vmorph));
		vtop->ml_vmorph = MEMLOC_VMORPH_TESTNZ(vtop->ml_vmorph);
	} else {
		if unlikely(Dee_function_generator_vind(self, offsetof(DeeTypeObject, tp_class)))
			goto err; /* type->tp_class */
		if unlikely(Dee_function_generator_vind(self,
		                                        offsetof(struct Dee_class_desc, cd_members[0]) +
		                                        (addr * sizeof(DREF DeeObject *))))
			goto err; /* type->tp_class->cd_members[addr] */
		if unlikely(Dee_function_generator_vreg(self, NULL))
			goto err; /* reg:type->tp_class->cd_members[addr] */
		if unlikely(Dee_function_generator_vdirect(self, 1))
			goto err; /* reg:type->tp_class->cd_members[addr] */
		vtop = Dee_function_generator_vtop(self);
		ASSERT(MEMLOC_VMORPH_ISDIRECT(vtop->ml_vmorph));
		vtop->ml_vmorph = MEMLOC_VMORPH_BOOL_NZ;
	}
	return 0;
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
		if unlikely(Dee_function_generator_vpush_const(self, type))
			goto err; /* this, type */
		if unlikely(Dee_function_generator_vswap(self))
			goto err; /* type, this */
		if unlikely(Dee_function_generator_vpush_imm16(self, addr))
			goto err; /* type, this, addr */
		return Dee_function_generator_vcallapi(self, &DeeInstance_GetMember, VCALL_CC_OBJECT, 3);
	}

	lock_offset = desc->cd_offset + offsetof(struct instance_desc, id_lock);
	slot_offset = desc->cd_offset + offsetof(struct instance_desc, id_vtab) +
	              addr * sizeof(DREF DeeObject *);
	/* XXX: (and this is a problem with the normal executor): assert that "this" is an instance of `type' */

	/* TODO: In case of reading members, if one of the next instructions also does a read,
	 *       keep the lock acquired. The same should also go when it comes to accessing
	 *       global/extern variables. */
	if unlikely(Dee_function_generator_vdelta(self, lock_offset))
		goto err; /* &this->[...].id_lock */
	if ((flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF) &&
	    unlikely(Dee_function_generator_grwlock_read(self, Dee_function_generator_vtop(self))))
		goto err;
	if ((flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_REF) &&
	    unlikely(Dee_function_generator_vdup(self)))
		goto err; /* [&this->[...].id_lock], &this->[...].id_lock */
	if unlikely(Dee_function_generator_vdelta(self, slot_offset - lock_offset))
		goto err; /* [&this->[...].id_lock], &this->[...].VALUE */
	if unlikely(Dee_function_generator_vind(self, 0))
		goto err; /* [&this->[...].id_lock], value */
	if unlikely(Dee_function_generator_vreg(self, NULL))
		goto err; /* [&this->[...].id_lock], reg:value */

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
		if unlikely(_Dee_function_generator_gjnz(self, Dee_function_generator_vtop(self), text_Lbound))
			goto err;
		saved_state = self->fg_state;
		Dee_memstate_incref(saved_state);
		temp = Dee_function_generator_state_dounshare(self);
		if likely(temp == 0)
			temp = Dee_function_generator_grwlock_endread(self, Dee_function_generator_vtop(self) - 1);
		if likely(temp == 0)
			temp = Dee_function_generator_gunbound_instance_member(self, type, addr);
		Dee_host_symbol_setsect(text_Lbound, text);
	} else {
		struct Dee_host_symbol *cold_Lunbound_member;
		cold_Lunbound_member = Dee_function_generator_newsym_named(self, ".Lunbound_member");
		if unlikely(!cold_Lunbound_member)
			goto err;
		if unlikely(_Dee_function_generator_gjz(self, Dee_function_generator_vtop(self), cold_Lunbound_member))
			goto err;
		saved_state = self->fg_state;
		Dee_memstate_incref(saved_state);
		HA_printf(".section .cold\n");
		self->fg_sect = cold;
		Dee_host_symbol_setsect(cold_Lunbound_member, cold);
		temp = Dee_function_generator_state_dounshare(self);
		if likely(temp == 0)
			temp = Dee_function_generator_grwlock_endread(self, Dee_function_generator_vtop(self) - 1);
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
		if unlikely(Dee_function_generator_vref(self))
			goto err; /* &this->[...].id_lock, ref:value */
		if unlikely(Dee_function_generator_vswap(self))
			goto err; /* ref:value, &this->[...].id_lock */
		if unlikely(Dee_function_generator_grwlock_endread(self, Dee_function_generator_vtop(self)))
			goto err; /* ref:value, &this->[...].id_lock */
		if unlikely(Dee_function_generator_vpop(self))
			goto err; /* ref:value */
	}
	return 0;
err:
	return -1;
}

/* this, type -> value */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpush_imember(struct Dee_function_generator *__restrict self,
                                     uint16_t addr, unsigned int flags) {
	bool safe = ((self->fg_assembler->fa_flags & DEE_FUNCTION_ASSEMBLER_F_SAFE) ||
	             (flags & DEE_FUNCTION_GENERATOR_CIMEMBER_F_SAFE));
	struct Dee_memloc *type_loc;
	if unlikely(Dee_function_generator_vdirect(self, 2))
		goto err;

	/* We only allow inline-resolving of this-member accesses when safe is disabled.
	 * This is because in code with user-assembly *any* type can be used together
	 * with the "this" argument, meaning that we *always* have to check that "this"
	 * actually implements the type *at runtime*.
	 *
	 * This differs when accessing class members, since for those no "this" argument
	 * is needed and the entire access can be performed immediately, no matter if
	 * safe semantics are required or not. */
	type_loc = Dee_function_generator_vtop(self);
	if (type_loc->ml_type == MEMLOC_TYPE_CONST && !safe) {
		struct class_desc *desc;
		DeeTypeObject *class_type = (DeeTypeObject *)type_loc->ml_value.v_const;
		if (DeeObject_AssertType(class_type, &DeeType_Type))
			goto err;
		if (!DeeType_IsClass(class_type))
			return libhostasm_rt_err_requires_class(class_type);
		desc = DeeClass_DESC(class_type);
		if (addr >= desc->cd_desc->cd_imemb_size)
			return libhostasm_rt_err_invalid_instance_addr(class_type, addr);
		if unlikely(Dee_function_generator_vpop(self))
			goto err; /* this */

		/* Push the member at runtime, but allowed to use (normally) unsafe
		 * semantics since we were already able to verify all constraints. */
		return Dee_function_generator_vpush_imember_unsafe_at_runtime(self, class_type, addr, flags);
	}

	/* Fallback: access the class member at runtime */
	if unlikely(Dee_function_generator_vswap(self))
		goto err; /* type, self */
	if unlikely(Dee_function_generator_vpush_imm16(self, addr))
		goto err; /* type, self, addr */
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
	if unlikely(Dee_function_generator_vdirect(self, 2))
		goto err;
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
		struct Dee_memloc *value_loc;
		if unlikely(Dee_function_generator_vpush_const(self, type))
			goto err; /* this, value, type */
		if unlikely(Dee_function_generator_vrrot(self, 3))
			goto err; /* type, this, value */
		if unlikely(Dee_function_generator_vpush_imm16(self, addr))
			goto err; /* type, this, value, addr */
		if unlikely(Dee_function_generator_vswap(self))
			goto err; /* type, this, addr, value */
		value_loc = Dee_function_generator_vtop(self);
		if (Dee_memloc_isnull(value_loc)) {
			if unlikely(Dee_function_generator_vpop(self))
				goto err; /* type, this, addr */
			return Dee_function_generator_vcallapi(self, &DeeInstance_DelMember, VCALL_CC_INT, 3);
		}
		if unlikely(Dee_function_generator_vnotoneref(self, 1))
			goto err; /* type, this, addr, value */
		return Dee_function_generator_vcallapi(self, &DeeInstance_SetMember, VCALL_CC_INT, 4);
	} else {
		struct Dee_memloc *value_loc;
		value_loc = Dee_function_generator_vtop(self);
		if (!Dee_memloc_isnull(value_loc)) {
			if unlikely(Dee_function_generator_vref(self))
				goto err; /* this, ref:value */
		}
	}

	lock_offset = desc->cd_offset + offsetof(struct instance_desc, id_lock);
	slot_offset = desc->cd_offset + offsetof(struct instance_desc, id_vtab) +
	              addr * sizeof(DREF DeeObject *);
	/* XXX: (and this is a problem with the normal executor): assert that "this" is an instance of `type' */

	if unlikely(Dee_function_generator_vswap(self))
		goto err; /* ref:value, this */
	if unlikely(Dee_function_generator_vdelta(self, lock_offset))
		goto err; /* ref:value, &this->[...].id_lock */
	if unlikely(Dee_function_generator_grwlock_write(self, Dee_function_generator_vtop(self)))
		goto err;
	if unlikely(Dee_function_generator_vdup(self))
		goto err; /* ref:value, &this->[...].id_lock, &this->[...].id_lock */
	if unlikely(Dee_function_generator_vdelta(self, slot_offset - lock_offset))
		goto err; /* ref:value, &this->[...].id_lock, &this->[...].VALUE */
	if unlikely(Dee_function_generator_vlrot(self, 3))
		goto err; /* &this->[...].id_lock, &this->[...].VALUE, ref:value */
	if unlikely(Dee_function_generator_vswapind(self, 0))
		goto err; /* &this->[...].id_lock, old_value */
	if unlikely(Dee_function_generator_vswap(self))
		goto err; /* old_value, &this->[...].id_lock */
	if unlikely(Dee_function_generator_grwlock_endwrite(self, Dee_function_generator_vtop(self)))
		goto err; /* old_value, &this->[...].id_lock */
	if unlikely(Dee_function_generator_vpop(self))
		goto err; /* old_value */
	ASSERT(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF);
	if unlikely(Dee_function_generator_gxdecref(self, Dee_function_generator_vtop(self), 1))
		goto err;
	ASSERT(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF);
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
	struct Dee_memloc *loc;
	if unlikely(Dee_function_generator_vdirect(self, 3))
		goto err;

	/* We only allow inline-resolving of this-member accesses when safe is disabled.
	 * This is because in code with user-assembly *any* type can be used together
	 * with the "this" argument, meaning that we *always* have to check that "this"
	 * actually implements the type *at runtime*.
	 *
	 * This differs when accessing class members, since for those no "this" argument
	 * is needed and the entire access can be performed immediately, no matter if
	 * safe semantics are required or not. */
	loc = Dee_function_generator_vtop(self) - 1;
	if (loc->ml_type == MEMLOC_TYPE_CONST && !safe) {
		struct class_desc *desc;
		DeeTypeObject *class_type = (DeeTypeObject *)loc->ml_value.v_const;
		if (DeeObject_AssertType(class_type, &DeeType_Type))
			goto err;
		if (!DeeType_IsClass(class_type))
			return libhostasm_rt_err_requires_class(class_type);
		desc = DeeClass_DESC(class_type);
		if (addr >= desc->cd_desc->cd_imemb_size)
			return libhostasm_rt_err_invalid_instance_addr(class_type, addr);
		if unlikely(Dee_function_generator_vswap(self))
			goto err; /* this, value, type */
		if unlikely(Dee_function_generator_vpop(self))
			goto err; /* this, value */

		/* Push the member at runtime, but allowed to use (normally) unsafe
		 * semantics since we were already able to verify all constraints. */
		return Dee_function_generator_vdel_or_pop_imember_unsafe_at_runtime(self, class_type, addr);
	}

	/* Fallback: access the class member at runtime */
	if unlikely(Dee_function_generator_vswap(self))
		goto err; /* this, value, type */
	if unlikely(Dee_function_generator_vrrot(self, 3))
		goto err; /* type, this, value */
	if unlikely(Dee_function_generator_vpush_imm16(self, addr))
		goto err; /* type, this, value, addr */
	if unlikely(Dee_function_generator_vswap(self))
		goto err; /* type, this, addr, value */
	loc = Dee_function_generator_vtop(self);
	if (Dee_memloc_isnull(loc)) {
		if unlikely(Dee_function_generator_vpop(self))
			goto err; /* type, this, addr */
		return Dee_function_generator_vcallapi(self,
		                                       safe ? (void const *)&DeeInstance_DelMemberSafe
		                                            : (void const *)&DeeInstance_DelMember,
		                                       VCALL_CC_INT, 3);
	}
	if unlikely(Dee_function_generator_vnotoneref(self, 1))
		goto err; /* type, this, addr, value */
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
	uintptr_t old_cfa_offset = self->fg_state->ms_host_cfa_offset;
	if (Dee_memstate_hstack_free(self->fg_state)) {
		uintptr_t new_cfa_offset = self->fg_state->ms_host_cfa_offset;
		ptrdiff_t freed = old_cfa_offset - new_cfa_offset;
		ASSERT(freed > 0);
#ifdef HOSTASM_STACK_GROWS_DOWN
		return _Dee_function_generator_ghstack_adjust(self, freed);
#else /* HOSTASM_STACK_GROWS_DOWN */
		return _Dee_function_generator_ghstack_adjust(self, -freed);
#endif /* !HOSTASM_STACK_GROWS_DOWN */
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
	vtop_type = Dee_memloc_typeof(Dee_function_generator_vtop(self));
	if (vtop_type != NULL) {
		struct Dee_memloc *vtop;
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
		if (vtop->ml_type == MEMLOC_TYPE_CONST) {
			if unlikely(Dee_function_generator_greg(self, vtop, NULL))
				goto err;
		}
		ASSERT(vtop->ml_type != MEMLOC_TYPE_CONST);
		vtop->ml_valtyp = NULL;
		return 0;
	}
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;
	if unlikely(Dee_function_generator_vdup(self))
		goto err; /* value, value */
	if unlikely(Dee_function_generator_vpush_const(self, type))
		goto err; /* value, value, type */
	if unlikely(impl_vassert_type_exact(self))
		goto err; /* value */
	return Dee_function_generator_vsettyp(self, type);
err:
	return -1;
}

/* obj, type -> N/A */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vassert_type_exact(struct Dee_function_generator *__restrict self) {
	struct Dee_memloc *typeloc;
	if unlikely(Dee_function_generator_vdirect(self, 2))
		goto err;
	typeloc = Dee_function_generator_vtop(self);
	if (typeloc->ml_type == MEMLOC_TYPE_CONST) {
		DeeTypeObject *type = (DeeTypeObject *)typeloc->ml_value.v_const;
		if unlikely(Dee_function_generator_vpop(self))
			goto err; /* obj */
		if unlikely(Dee_function_generator_vassert_type_exact_c(self, type))
			goto err; /* obj */
		return Dee_function_generator_vpop(self); /* N/A */
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
	vtop_type = Dee_memloc_typeof(Dee_function_generator_vtop(self));
	if (vtop_type != NULL) {
		struct Dee_memloc *vtop;
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
		if (vtop->ml_type == MEMLOC_TYPE_CONST) {
			if unlikely(Dee_function_generator_greg(self, vtop, NULL))
				goto err;
		}
		ASSERT(vtop->ml_type != MEMLOC_TYPE_CONST);
		vtop->ml_valtyp = NULL;
		return 0;
	}
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;
	if unlikely(Dee_function_generator_vdup(self))
		goto err; /* value, value */
	if unlikely(Dee_function_generator_vpush_const(self, type))
		goto err; /* value, value, type */
	return impl_vassert_type(self);
err:
	return -1;
}

/* obj, type -> N/A */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vassert_type(struct Dee_function_generator *__restrict self) {
	struct Dee_memloc *typeloc;
	if unlikely(Dee_function_generator_vdirect(self, 2))
		goto err;
	typeloc = Dee_function_generator_vtop(self);
	if (typeloc->ml_type == MEMLOC_TYPE_CONST) {
		DeeTypeObject *type = (DeeTypeObject *)typeloc->ml_value.v_const;
		if unlikely(Dee_function_generator_vpop(self))
			goto err; /* obj */
		if unlikely(Dee_function_generator_vassert_type_c(self, type))
			goto err; /* obj */
		return Dee_function_generator_vpop(self); /* N/A */
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
	struct Dee_memloc loc;
#ifdef DEE_HOST_RELOCVALUE_SECT
	struct Dee_host_symbol _Ljmp;
#endif /* DEE_HOST_RELOCVALUE_SECT */
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;

	/* TODO: If this jump might be the result of infinite loops,
	 *       must emit a call to `DeeThread_CheckInterrupt()' */

	bool_status = Dee_function_generator_vopbool(self, VOPBOOL_F_NOFALLBACK | VOPBOOL_F_FORCE_MORPH);
	if unlikely(bool_status < 0)
		goto err; /* Force vtop into a bool constant, or a MEMLOC_VMORPH_ISBOOL-style morph */
	loc = *Dee_function_generator_vtop(self);

	/* Special case for when the top-element is a constant. */
	if (loc.ml_type == MEMLOC_TYPE_CONST && likely(MEMLOC_VMORPH_ISDIRECT(loc.ml_vmorph))) {
		ASSERT(DeeBool_Check(loc.ml_value.v_const));
		if (loc.ml_value.v_const != Dee_False) {
			/* Unconditional jump -> the block ends here and falls into the next one */
			self->fg_block->bb_next       = target;
			self->fg_block->bb_deemon_end = instr; /* The jump doesn't exist anymore now! */
		}
		if unlikely(Dee_function_generator_vpop(self))
			goto err;
		goto assign_desc_stat;
	}

	/* If the jump target location already has its starting memory state generated,
	 * and that state requires a small CFA offset than we currently have, then try
	 * to reclaim unused host stack memory.
	 *
	 * This is necessary so we don't end up in a situation where a block keeps on
	 * marking itself for re-compilation with ever-growing CFA offsets. */
	if (target->bb_mem_start != NULL &&
	    target->bb_mem_start->ms_host_cfa_offset < self->fg_state->ms_host_cfa_offset) {
		if unlikely(reclaim_unused_stack_space(self))
			goto err;
	}

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
		struct Dee_except_exitinfo *except_exit;
		struct Dee_host_symbol *Lexcept;
#ifdef DEE_HOST_RELOCVALUE_SECT
		struct Dee_host_symbol _Lexcept;
		Dee_host_symbol_initcommon_named(&_Lexcept, ".Lexcept");
		Lexcept = &_Lexcept;
#else /* DEE_HOST_RELOCVALUE_SECT */
		Lexcept = Dee_function_generator_newsym_named(self, ".Lexcept");
		if unlikely(!Lexcept)
			goto err;
#endif /* !DEE_HOST_RELOCVALUE_SECT */
		except_exit = Dee_function_generator_except_exit(self);
		if unlikely(!except_exit)
			goto err;
		if (self->fg_exceptinject != NULL) {
			/* TODO: Must also inject custom exception handling code! */
			DeeError_NOTIMPLEMENTED();
			goto err;
		} else {
			Dee_host_symbol_setsect_ex(Lexcept, &except_exit->exi_block->bb_htext, 0);
		}
		if (loc.ml_flags & MEMLOC_F_ONEREF) {
			ASSERT(Dee_memloc_sameloc(&loc, Dee_function_generator_vtop(self)));
			if unlikely(Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_BOOL, 1))
				goto err;
			loc = *Dee_function_generator_vtop(self);
		}

		loctype = Dee_memloc_typeof(&loc);
		hasbool = loctype && DeeType_InheritOperator(loctype, OPERATOR_BOOL);
		ASSERT(!hasbool || (loctype->tp_cast.tp_bool != NULL));
		if unlikely(Dee_function_generator_vcallapi(self,
		                                            hasbool ? (void const *)loctype->tp_cast.tp_bool
		                                                    : (void const *)&DeeObject_Bool,
		                                            VCALL_CC_RAWINT, 1))
			goto err;
		loc = *Dee_function_generator_vtop(self);

		/* Silently remove the bool-morph location from the v-stack. */
		ASSERT(self->fg_state->ms_stackc >= 1);
		--self->fg_state->ms_stackc;
		if (MEMLOC_TYPE_HASREG(loc.ml_type))
			Dee_memstate_decrinuse(self->fg_state, loc.ml_value.v_hreg.r_regno);

		/* Generate code to branch depending on the value of `loc' */
		zero.ml_type = MEMLOC_TYPE_CONST;
		zero.ml_value.v_const = NULL;
		if unlikely(_Dee_function_generator_gjcmp(self, &loc, &zero, true,
		                                          Lexcept,                     /* loc < 0 */
		                                          jump_if_true ? NULL : Ljmp,  /* loc == 0 */
		                                          jump_if_true ? Ljmp : NULL)) /* loc > 0 */
			goto err;
	} else {
		struct Dee_memloc cmp_lhs, cmp_rhs;
		struct Dee_host_symbol *Llo, *Leq, *Lgr;

		/* In this case, `Dee_function_generator_vopbool()' already created a morph. */
		ASSERT(MEMLOC_VMORPH_ISBOOL(loc.ml_vmorph));
		ASSERT(loc.ml_flags & MEMLOC_F_NOREF);

		/* Silently remove the bool-morph location from the v-stack. */
		ASSERT(self->fg_state->ms_stackc >= 1);
		--self->fg_state->ms_stackc;
		if (MEMLOC_TYPE_HASREG(loc.ml_type))
			Dee_memstate_decrinuse(self->fg_state, loc.ml_value.v_hreg.r_regno);

		/* Compute compare operands and target labels. */
		Llo = NULL;
		Leq = NULL;
		Lgr = NULL;
		cmp_lhs = loc;
		cmp_rhs.ml_type = MEMLOC_TYPE_CONST;
		cmp_rhs.ml_value.v_const = NULL;
		switch (loc.ml_vmorph) {
		case MEMLOC_VMORPH_BOOL_Z:
		case MEMLOC_VMORPH_BOOL_Z_01:
			/* Jump-if-zero */
			Leq = Ljmp;
			break;
		case MEMLOC_VMORPH_BOOL_NZ:
		case MEMLOC_VMORPH_BOOL_NZ_01:
			Llo = Ljmp;
			Lgr = Ljmp;
			break;
		case MEMLOC_VMORPH_BOOL_LZ:
			Llo = Ljmp;
			/* (X-1) < 0   <=>   X <= 0 */
			if (Dee_memloc_getvaldelta(&cmp_lhs) == -1) {
				Dee_memloc_setvaldelta(&cmp_lhs, 0);
				Leq = Ljmp;
			}
			break;
		case MEMLOC_VMORPH_BOOL_GZ:
			Lgr = Ljmp;
			/* (X+1) > 0   <=>   X >= 0 */
			if (Dee_memloc_getvaldelta(&cmp_lhs) == 1) {
				Dee_memloc_setvaldelta(&cmp_lhs, 0);
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
		if unlikely(_Dee_function_generator_gjcmp(self, &cmp_lhs, &cmp_rhs, true, Llo, Leq, Lgr))
			goto err;
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
	struct Dee_memloc decref_on_iter_done;
	DREF struct Dee_memstate *desc_state;
	struct Dee_host_symbol *sym;
	struct Dee_basic_block *target = desc->jd_to;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(Dee_function_generator_vnotoneref_if_operator(self, OPERATOR_ITERNEXT, 1))
		goto err;
	if (!always_pop_iterator) {
		if unlikely(Dee_function_generator_vdup(self))
			goto err; /* iter, iter */
	}
	if unlikely(Dee_function_generator_vcallapi(self, &DeeObject_IterNext, VCALL_CC_RAWINT, 1))
		goto err; /* [if(!always_pop_iterator) iter], UNCHECKED(elem) */

	/* TODO: If this jump might be the result of infinite loops,
	 *       must emit a call to `DeeThread_CheckInterrupt()' */

	/* If the jump target location already has its starting memory state generated,
	 * and that state requires a small CFA offset than we currently have, then try
	 * to reclaim unused host stack memory.
	 *
	 * This is necessary so we don't end up in a situation where a block keeps on
	 * marking itself for re-compilation with ever-growing CFA offsets. */
	if (target->bb_mem_start != NULL &&
	    target->bb_mem_start->ms_host_cfa_offset < self->fg_state->ms_host_cfa_offset) {
		if unlikely(reclaim_unused_stack_space(self))
			goto err;
	}

	/* >> if (elem == NULL) HANDLE_EXCEPT(); */
	if unlikely(Dee_function_generator_gjz_except(self, Dee_function_generator_vtop(self)))
		goto err;

	/* >> if (elem == ITER_DONE) goto <desc>; */
	sym = Dee_function_generator_newsym(self);
	if unlikely(!sym)
		goto err;
	Dee_host_symbol_setjump(sym, desc);
	{
		struct Dee_memloc iter_done;
		iter_done.ml_type = MEMLOC_TYPE_CONST;
		iter_done.ml_value.v_const = ITER_DONE;
		if unlikely(_Dee_function_generator_gjcmp(self, Dee_function_generator_vtop(self),
		                                          &iter_done, false, NULL, sym, NULL))
			goto err;
	}
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;

	/* Remember the memory-state as it is when the jump is made. */
	ASSERTF(!desc->jd_stat, "Who assigned this? Doing that is *my* job!");
	desc_state = Dee_memstate_copy(self->fg_state);
	if unlikely(!desc_state)
		goto err;
	ASSERT(desc_state->ms_stackc >= 1);
	--desc_state->ms_stackc; /* Get rid of `UNCHECKED(result)' */
	if (MEMLOC_TYPE_HASREG(desc_state->ms_stackv[desc_state->ms_stackc].ml_type))
		Dee_memstate_decrinuse(desc_state, desc_state->ms_stackv[desc_state->ms_stackc].ml_value.v_hreg.r_regno);
	decref_on_iter_done.ml_type = MEMLOC_TYPE_UNALLOC;
	if (!always_pop_iterator) {
		/* Pop another vstack item (the iterator) and store it in `MEMSTATE_XLOCAL_POPITER'.
		 * When the time comes to generate morph-code, the iterator will then be decref'd. */
		ASSERT(desc_state->ms_stackc >= 1);
		--desc_state->ms_stackc;
		decref_on_iter_done = desc_state->ms_stackv[desc_state->ms_stackc];
		if (MEMLOC_TYPE_HASREG(decref_on_iter_done.ml_type))
			Dee_memstate_decrinuse(desc_state, decref_on_iter_done.ml_value.v_hreg.r_regno);
		if (decref_on_iter_done.ml_flags & MEMLOC_F_NOREF)
			decref_on_iter_done.ml_type = MEMLOC_TYPE_UNALLOC;
	}
	desc->jd_stat = desc_state; /* Inherit reference */

	/* Adjust out own current state to make the top-item (i.e. the "elem") to become a reference */
	ASSERT(self->fg_state != desc_state);
	ASSERT(!Dee_memstate_isshared(self->fg_state));
	ASSERT(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF);
	Dee_function_generator_vtop(self)->ml_flags &= ~MEMLOC_F_NOREF;

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
	if (decref_on_iter_done.ml_type != MEMLOC_TYPE_UNALLOC) {
		struct Dee_memloc *popiter_loc;
		popiter_loc = &desc_state->ms_localv[self->fg_assembler->fa_localc + MEMSTATE_XLOCAL_POPITER];
		ASSERT(popiter_loc->ml_flags == (MEMLOC_F_NOREF | MEMLOC_F_LOCAL_UNBOUND));
		ASSERT(popiter_loc->ml_type == MEMLOC_TYPE_UNALLOC);
		popiter_loc->ml_flags  = MEMLOC_F_LOCAL_BOUND;
		popiter_loc->ml_type   = decref_on_iter_done.ml_type;
		popiter_loc->ml_value  = decref_on_iter_done.ml_value;
		popiter_loc->ml_valtyp = decref_on_iter_done.ml_valtyp;
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
	struct Dee_memloc *loc;
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	loc = Dee_function_generator_vtop(self);
	ASSERTF(loc->ml_flags & MEMLOC_F_NOREF, "Cannot do indirection on location holding a reference");
	if unlikely(Dee_function_generator_gind(self, loc, ind_delta))
		goto err;
	loc->ml_valtyp = NULL; /* Unknown */
	return 0;
err:
	return -1;
}

/* >> *(SECOND + ind_delta) = POP(); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpopind(struct Dee_function_generator *__restrict self,
                               ptrdiff_t ind_delta) {
	struct Dee_memloc src, *loc;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(Dee_function_generator_vdirect(self, 2))
		goto err;
	if unlikely(Dee_function_generator_vnotoneref(self, 1))
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

/* >> TOP = TOP + val_delta; // NOTE: Ignores `ml_vmorph' */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vdelta(struct Dee_function_generator *__restrict self,
                              ptrdiff_t val_delta) {
	struct Dee_memloc *loc;
	if unlikely(val_delta == 0)
		return 0;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	loc = Dee_function_generator_vtop(self);
	ASSERTF(loc->ml_flags & MEMLOC_F_NOREF, "Cannot add delta to location holding a reference");
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
	loc->ml_valtyp = NULL; /* Unknown */
	return 0;
err:
	return -1;
}

/* >> temp = *(SECOND + ind_delta);
 * >> *(SECOND + ind_delta) = FIRST;
 * >> POP();
 * >> POP();
 * >> PUSH(temp, MEMLOC_F_NOREF); */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vswapind(struct Dee_function_generator *__restrict self,
                                ptrdiff_t ind_delta) {
	if unlikely(Dee_function_generator_vswap(self))
		goto err; /* src, dst */
	if unlikely(Dee_function_generator_vdup(self))
		goto err; /* src, dst, dst */
	if unlikely(Dee_function_generator_vind(self, ind_delta))
		goto err; /* src, dst, *(dst + ind_delta) */
	if unlikely(Dee_function_generator_vreg(self, NULL))
		goto err; /* src, dst, reg:*(dst + ind_delta) */
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_NOREF;
	if unlikely(Dee_function_generator_vrrot(self, 3))
		goto err; /* reg:*(dst + ind_delta), src, dst */
	if unlikely(Dee_function_generator_vswap(self))
		goto err; /* reg:*(dst + ind_delta), dst, src */
	if unlikely(Dee_function_generator_vpopind(self, ind_delta))
		goto err; /* reg:*(dst + ind_delta), dst */
	return Dee_function_generator_vpop(self);
err:
	return -1;
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
		if unlikely(Dee_function_generator_vdirect(self, 1))
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
		return Dee_function_generator_gnotoneref(self, loc);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vref2(struct Dee_function_generator *__restrict self,
                             Dee_vstackaddr_t dont_steal_from_vtop_n) {
	struct Dee_memstate *state = self->fg_state;
	struct Dee_memloc *loc;
	if unlikely(state->ms_stackc < 1)
		return err_illegal_stack_effect();
	loc = Dee_memstate_vtop(state);
	if (loc->ml_flags & MEMLOC_F_NOREF) {
		if unlikely(Dee_function_generator_state_unshare(self))
			goto err;
		if unlikely(Dee_function_generator_vdirect(self, 1))
			goto err;
		return Dee_function_generator_gref2(self, loc, dont_steal_from_vtop_n);
	}
	return 0;
err:
	return -1;
}


/* Ensure that `loc' is holding a reference. If said location has aliases,
 * and isn't a constant, then also ensure that at least one of those aliases
 * also contains a second reference.
 * @param: dont_steal_from_vtop_n: Ignore the top n v-stack items when searching for aliases.*/
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gref2(struct Dee_function_generator *__restrict self,
                             struct Dee_memloc *loc,
                             Dee_vstackaddr_t dont_steal_from_vtop_n) {
	struct Dee_memstate *state = self->fg_state;
	struct Dee_memloc *alias;
	struct Dee_memloc *alias_with_reference = NULL;    /* Alias that has a reference */
	struct Dee_memloc *alias_without_reference = NULL; /* Alias that needs a reference */
	bool got_alias = false; /* There *are* aliases. */
	ASSERT(state->ms_stackc <= dont_steal_from_vtop_n);
	state->ms_stackc -= dont_steal_from_vtop_n;
	Dee_memstate_foreach(alias, state) {
		if (alias == loc)
			continue;
		if (!Dee_memloc_sameloc(alias, loc))
			continue;
		/* Got an alias! */
		got_alias = true;
		if (alias->ml_flags & MEMLOC_F_NOREF) {
			alias_without_reference = alias;
		} else if (loc->ml_flags & MEMLOC_F_NOREF) {
			/* Steal reference from alias */
			loc->ml_flags &= ~MEMLOC_F_NOREF;
			alias->ml_flags |= MEMLOC_F_NOREF;
			alias_without_reference = alias;
		} else {
			alias_with_reference = alias;
		}
	}
	Dee_memstate_foreach_end;
	state->ms_stackc += dont_steal_from_vtop_n;
	if (got_alias) {
		ASSERT(!alias_with_reference || !(alias_with_reference->ml_flags & MEMLOC_F_NOREF));
		ASSERT(!alias_without_reference || (alias_without_reference->ml_flags & MEMLOC_F_NOREF));
		if (loc->ml_flags & MEMLOC_F_NOREF) {
			/* There are aliases, but no-one is holding a reference.
			 * This can happen if the location points to a constant
			 * that got flushed, or is a function argument, in which
			 * case we only need a single reference. */
			ASSERT(alias_without_reference);
			ASSERT(!alias_with_reference);
			ASSERT(!Dee_memstate_isshared(state));
			if unlikely(Dee_function_generator_gincref(self, loc, 1))
				goto err;
			loc->ml_flags &= ~MEMLOC_F_NOREF;
			if unlikely(Dee_function_generator_gnotoneref(self, loc))
				goto err;
		} else if (alias_without_reference && !alias_with_reference &&
		           /* When it's a constant, there is already an extra reference through code dependencies */
		           loc->ml_type != MEMLOC_TYPE_CONST) {
			/* There are aliases, but less that 2 references -> make sure there are at least 2 references */
			ASSERT(!Dee_memstate_isshared(state));
			ASSERT(alias_without_reference->ml_flags & MEMLOC_F_NOREF);
			if unlikely(Dee_function_generator_gincref(self, alias_without_reference, 1))
				goto err;
			alias_without_reference->ml_flags &= ~MEMLOC_F_NOREF;
			if unlikely(Dee_function_generator_gnotoneref(self, alias_without_reference))
				goto err;
		}
	} else {
		/* No aliases exist, so there's no need to force a distinct location. */
		if (loc->ml_flags & MEMLOC_F_NOREF) {
			ASSERT(!Dee_memstate_isshared(state));
			if unlikely(Dee_function_generator_gincref(self, loc, 1))
				goto err;
			loc->ml_flags &= ~MEMLOC_F_NOREF;
			if unlikely(Dee_function_generator_gnotoneref(self, loc))
				goto err;
		}
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
	if unlikely(Dee_function_generator_vpush_addr(self, &mod->mo_globalv[gid]))
		goto err;
	if (ref) {
		if unlikely(Dee_function_generator_grwlock_read_const(self, &mod->mo_lock))
			goto err;
	}
	if unlikely(Dee_function_generator_vind(self, 0))
		goto err;
	if unlikely(Dee_function_generator_vreg(self, NULL))
		goto err;
	loc = Dee_function_generator_vtop(self);
	ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
	if unlikely(Dee_function_generator_gassert_bound(self, loc, NULL, mod, gid,
	                                                 ref ? &mod->mo_lock : NULL,
	                                                 NULL))
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
		if unlikely(Dee_function_generator_grwlock_endread_const(self, &mod->mo_lock))
			goto err;
		loc = Dee_function_generator_vtop(self);
		ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
		loc->ml_flags &= ~MEMLOC_F_NOREF;
	}
	return 0;
err:
	return -1;
}

/* Generate code to check if a global variable is currently bound. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vbound_mod_global(struct Dee_function_generator *__restrict self,
                                         struct Dee_module_object *mod, uint16_t gid) {
	struct Dee_memloc *loc;
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
	if unlikely(Dee_function_generator_vpush_addr(self, &mod->mo_globalv[gid]))
		goto err;
	if unlikely(Dee_function_generator_vind(self, 0))
		goto err;
	if unlikely(Dee_function_generator_vdirect(self, 0))
		goto err;
	loc = Dee_function_generator_vtop(self);
	ASSERT(MEMLOC_VMORPH_ISDIRECT(loc->ml_vmorph));
	loc->ml_vmorph = MEMLOC_VMORPH_TESTNZ(loc->ml_vmorph);
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
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;
	if unlikely(Dee_function_generator_vpush_addr(self, &mod->mo_globalv[gid]))
		goto err;
	if unlikely(Dee_function_generator_vswap(self))
		goto err;
	if unlikely(Dee_function_generator_grwlock_write_const(self, &mod->mo_lock))
		goto err;
	if unlikely(Dee_function_generator_vswapind(self, 0))
		goto err;
	if unlikely(Dee_function_generator_grwlock_endwrite_const(self, &mod->mo_lock))
		goto err;
	ASSERT(self->fg_state->ms_stackc >= 1);
	loc = *Dee_function_generator_vtop(self);
	ASSERT(loc.ml_flags & MEMLOC_F_NOREF);
	--self->fg_state->ms_stackc;
	if (MEMLOC_TYPE_HASREG(loc.ml_type))
		Dee_memstate_decrinuse(self->fg_state, loc.ml_value.v_hreg.r_regno);
	return Dee_function_generator_gxdecref(self, &loc, 1); /* xdecref in case global wasn't bound before. */
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
	int result = Dee_function_generator_vref(self);
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
	struct Dee_memloc *loc;
	DeeCodeObject *code = self->fg_assembler->fa_code;
	if unlikely(sid >= code->co_staticc)
		return err_illegal_sid(sid);
	if unlikely(Dee_function_generator_vpush_addr(self, &code->co_staticv[sid]))
		goto err;
	if unlikely(Dee_function_generator_grwlock_read_const(self, &code->co_static_lock))
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
	return Dee_function_generator_grwlock_endread_const(self, &code->co_static_lock);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vpop_static(struct Dee_function_generator *__restrict self, uint16_t sid) {
	DeeCodeObject *code = self->fg_assembler->fa_code;
	if unlikely(sid >= code->co_staticc)
		return err_illegal_sid(sid);
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;
	if unlikely(Dee_function_generator_vref(self))
		goto err;
	ASSERT(!(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF));
	if unlikely(Dee_function_generator_vpush_addr(self, &code->co_staticv[sid]))
		goto err; /* value, addr */
	if unlikely(Dee_function_generator_vswap(self))
		goto err; /* addr, value */
	if unlikely(Dee_function_generator_grwlock_write_const(self, &code->co_static_lock))
		goto err;
	if unlikely(Dee_function_generator_vswapind(self, 0))
		goto err; /* old_value */
	if unlikely(Dee_function_generator_grwlock_endwrite_const(self, &code->co_static_lock))
		goto err;
	ASSERT(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF);
	Dee_function_generator_vtop(self)->ml_flags &= ~MEMLOC_F_NOREF;
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vrwlock_read(struct Dee_function_generator *__restrict self) {
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;
	if unlikely(Dee_function_generator_grwlock_read(self, Dee_function_generator_vtop(self)))
		goto err;
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vrwlock_write(struct Dee_function_generator *__restrict self) {
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;
	if unlikely(Dee_function_generator_grwlock_write(self, Dee_function_generator_vtop(self)))
		goto err;
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vrwlock_endread(struct Dee_function_generator *__restrict self) {
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;
	if unlikely(Dee_function_generator_grwlock_endread(self, Dee_function_generator_vtop(self)))
		goto err;
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vrwlock_endwrite(struct Dee_function_generator *__restrict self) {
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;
	if unlikely(Dee_function_generator_grwlock_endwrite(self, Dee_function_generator_vtop(self)))
		goto err;
	return Dee_function_generator_vpop(self);
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
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;
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
	Dee_vstackaddr_t stackc;
	Dee_lid_t lid;
	if unlikely(Dee_function_generator_vdirect(self, 1))
		goto err;

	/* Move the final return value to the bottom of the stack. */
	stackc = self->fg_state->ms_stackc;
	if unlikely(Dee_function_generator_vrrot(self, stackc))
		goto err;

	/* Remove all but the final element from the stack. */
	if unlikely(Dee_function_generator_vpopmany(self, stackc - 1))
		goto err;

	/* Unbind all local variables. */
	for (lid = 0; lid < self->fg_state->ms_localc; ++lid) {
		if unlikely(Dee_function_generator_vdel_local(self, lid))
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



/* Do calling-convention-specific handling of the return value. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcallapi_checkresult(struct Dee_function_generator *__restrict self,
                                            unsigned int cc, Dee_vstackaddr_t argc) {
	switch (cc) {

	case VCALL_CC_OBJECT:
		if unlikely(Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN, 0))
			goto err; /* [args...], UNCHECKED(result) */
		if unlikely(Dee_function_generator_vrrot(self, argc + 1))
			goto err; /* UNCHECKED(result), [args...] */
		if unlikely(Dee_function_generator_vpopmany(self, argc))
			goto err; /* UNCHECKED(result) */
		return Dee_function_generator_vcheckobj(self); /* result */

	case VCALL_CC_RAWINT:
		if unlikely(Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN, 0))
			goto err; /* [args...], UNCHECKED(result) */
		if unlikely(Dee_function_generator_vrrot(self, argc + 1))
			goto err; /* UNCHECKED(result), [args...] */
		break;

	case VCALL_CC_BOOL:
		if unlikely(Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN, 0))
			goto err; /* [args...], UNCHECKED(result) */
		ASSERT(Dee_function_generator_vtop(self)->ml_vmorph == MEMLOC_VMORPH_DIRECT);
		Dee_function_generator_vtop(self)->ml_vmorph = MEMLOC_VMORPH_BOOL_NZ;
		if unlikely(Dee_function_generator_vrrot(self, argc + 1))
			goto err; /* UNCHECKED(result), [args...] */
		break;

	case VCALL_CC_RAWINT_KEEPARGS:
		return Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN, 0);

	case VCALL_CC_VOID:
		break;

	case VCALL_CC_EXCEPT:
		if unlikely(Dee_function_generator_vpopmany(self, argc))
			goto err;
		return Dee_function_generator_gjmp_except(self);

	case VCALL_CC_INT:
		if unlikely(Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN, 0))
			goto err; /* [args...], UNCHECKED(result) */
		if unlikely(Dee_function_generator_vrrot(self, argc + 1))
			goto err; /* UNCHECKED(result), [args...] */
		if unlikely(Dee_function_generator_vpopmany(self, argc))
			goto err; /* UNCHECKED(result) */
		return Dee_function_generator_vcheckint(self); /* - */

	case VCALL_CC_NEGINT: {
		if unlikely(Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN, 0))
			goto err; /* [args...], UNCHECKED(result) */
		if unlikely(Dee_function_generator_vrrot(self, argc + 1))
			goto err; /* UNCHECKED(result), [args...] */
		if unlikely(Dee_function_generator_vpopmany(self, argc))
			goto err; /* UNCHECKED(result) */
		return Dee_function_generator_gjcmp_except(self, Dee_function_generator_vtop(self), 0,
		                                           Dee_FUNCTION_GENERATOR_GJCMP_EXCEPT_LO);
	}	break;

	case VCALL_CC_M1INT: {
		if unlikely(Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN, 0))
			goto err; /* [args...], UNCHECKED(result) */
		if unlikely(Dee_function_generator_vrrot(self, argc + 1))
			goto err; /* UNCHECKED(result), [args...] */
		if unlikely(Dee_function_generator_vpopmany(self, argc))
			goto err; /* UNCHECKED(result) */
		return Dee_function_generator_gjeq_except(self, Dee_function_generator_vtop(self), -1);
	}	break;

	case VCALL_CC_MORPH_INTPTR:
	case VCALL_CC_MORPH_UINTPTR:
		if unlikely(Dee_function_generator_vpush_reg(self, HOST_REGISTER_RETURN, 0))
			goto err; /* [args...], UNCHECKED(result) */
		ASSERT(Dee_function_generator_vtop(self)->ml_vmorph == MEMLOC_VMORPH_DIRECT);
		Dee_function_generator_vtop(self)->ml_vmorph = cc == VCALL_CC_MORPH_UINTPTR
		                                               ? MEMLOC_VMORPH_UINT
		                                               : MEMLOC_VMORPH_INT;
		if unlikely(Dee_function_generator_vrrot(self, argc + 1))
			goto err; /* UNCHECKED(result), [args...] */
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
	struct Dee_memloc *loc, *argv;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(Dee_function_generator_vdirect(self, argc))
		goto err;

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
	if unlikely(Dee_function_generator_gflushregs(self, argc, cc == VCALL_CC_EXCEPT))
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
	return Dee_function_generator_vcallapi_checkresult(self, cc, argc);
err_argv:
	Dee_Freea(argv);
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
	int temp;
	struct Dee_memloc *loc, *argv, func;
	Dee_host_register_t func_regno;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(Dee_function_generator_vdirect(self, argc + 1))
		goto err;
	loc = Dee_function_generator_vtop(self);
	if (loc->ml_type == MEMLOC_TYPE_CONST) {
		/* Special case: function being called is actually at a constant address. */
		void const *api_function = (void const *)loc->ml_value.v_const;
		if unlikely(Dee_function_generator_vpop(self))
			goto err;
		return Dee_function_generator_vcallapi(self, api_function, cc, argc);
	}

	/* Save argument memory locations from before the flush. This is because after the
	 * flush, registers are written to the stack, and if we were to pass the then-current
	 * `Dee_memloc' to `_Dee_function_generator_gcallapi', it would have to load those
	 * values from the stack (when they can also be found in registers) */
	loc = self->fg_state->ms_stackv;
	loc += self->fg_state->ms_stackc;
	loc -= argc + 1;
	argv = (struct Dee_memloc *)Dee_Mallocac(argc, sizeof(struct Dee_memloc));
	if unlikely(!argv)
		goto err;
	argv = (struct Dee_memloc *)memcpyc(argv, loc, argc, sizeof(struct Dee_memloc));

	/* Flush registers that don't appear in the top `argc' stack locations.
	 * When the function always throw an exception, we *only* need to preserve
	 * stuff that contains references! */
	if unlikely(Dee_function_generator_gflushregs(self, argc + 1, cc == VCALL_CC_EXCEPT))
		goto err_argv;
	func = *Dee_function_generator_vtop(self);
	if unlikely(Dee_function_generator_vpop(self))
		goto err_argv;
	if (func.ml_type == MEMLOC_TYPE_HREG ||
	    (func.ml_type == MEMLOC_TYPE_HREGIND && func.ml_value.v_hreg.r_voff == 0) ||
	    (func.ml_type == MEMLOC_TYPE_HSTACKIND || func.ml_value.v_hstack.s_off != 0)) {
		/* Function can be generated like this */
	} else {
		/* Need to load function into a register. */
		if unlikely(Dee_function_generator_greg(self, &func, NULL))
			goto err_argv;
		ASSERT(func.ml_type == MEMLOC_TYPE_HREG);
	}
	func_regno = HOST_REGISTER_COUNT;
	if (MEMLOC_TYPE_HASREG(func.ml_type))
		func_regno = func.ml_value.v_hreg.r_regno;
	if (func.ml_type == MEMLOC_TYPE_HREG && func.ml_value.v_hreg.r_off != 0) {
		if unlikely(_Dee_function_generator_gmov_regx2reg(self,
		                                                  func.ml_value.v_hreg.r_regno,
		                                                  func.ml_value.v_hreg.r_off,
		                                                  func.ml_value.v_hreg.r_regno))
			goto err_argv;
		func.ml_value.v_hreg.r_off = 0;
	}

	/* Check if any of the argument registers got clobbered by register usage during flushing. */
	{
		uint16_t argi;
		for (argi = 0; argi < argc; ++argi) {
			struct Dee_memloc *stck_loc;
			struct Dee_memloc *argv_loc = &argv[argi];
			if (!MEMLOC_TYPE_HASREG(argv_loc->ml_type))
				continue;
			ASSERT(argv_loc->ml_value.v_hreg.r_regno < HOST_REGISTER_COUNT);
			if (self->fg_state->ms_rusage[argv_loc->ml_value.v_hreg.r_regno] != DEE_HOST_REGUSAGE_GENERIC ||
			    argv_loc->ml_value.v_hreg.r_regno == func_regno) {
				/* Register got clobbered */
				stck_loc = &self->fg_state->ms_stackv[self->fg_state->ms_stackc - (argc + 1) + argi];
				*argv_loc = *stck_loc;
			}
		}
	}

	/* Call the actual C function */
	ASSERT(func.ml_type == MEMLOC_TYPE_HREG ||
	       func.ml_type == MEMLOC_TYPE_HSTACKIND ||
	       func.ml_type == MEMLOC_TYPE_HREGIND);
	switch (func.ml_type) {
	case MEMLOC_TYPE_HREG:
		temp = _Dee_function_generator_gcalldynapi_reg(self, func.ml_value.v_hreg.r_regno, argc, argv);
		break;
	case MEMLOC_TYPE_HSTACKIND:
		temp = _Dee_function_generator_gcalldynapi_hstackind(self, func.ml_value.v_hstack.s_cfa, argc, argv);
		break;
	case MEMLOC_TYPE_HREGIND:
		temp = _Dee_function_generator_gcalldynapi_hregind(self,
		                                                   func.ml_value.v_hreg.r_regno,
		                                                   func.ml_value.v_hreg.r_off,
		                                                   argc, argv);
		break;
	default: __builtin_unreachable();
	}
	if unlikely(temp)
		goto err_argv;
	Dee_Freea(argv);

	/* Do calling-convention-specific handling of the return value. */
	return Dee_function_generator_vcallapi_checkresult(self, cc, argc);
err_argv:
	Dee_Freea(argv);
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
	struct Dee_memloc *loc;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	loc = Dee_function_generator_vtop(self);
	ASSERTF(MEMLOC_VMORPH_ISDIRECT(loc->ml_vmorph), "non-sensical call: vcheckobj() on non-direct value");
	if unlikely(Dee_function_generator_gjz_except(self, loc))
		goto err;
	/* Clear the NOREF flag now that we know the return value to be non-NULL */
	loc = Dee_function_generator_vtop(self);
	if (loc->ml_type != MEMLOC_TYPE_CONST) {
		ASSERT(loc->ml_flags & MEMLOC_F_NOREF);
		loc->ml_flags &= ~MEMLOC_F_NOREF;
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
	struct Dee_memloc *loc;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	loc = Dee_function_generator_vtop(self);
	ASSERTF(MEMLOC_VMORPH_ISDIRECT(loc->ml_vmorph), "non-sensical call: vcheckint() on non-direct value");
	if unlikely(Dee_function_generator_gjnz_except(self, loc))
		goto err;
	return Dee_function_generator_vpop(self);
err:
	return -1;
}

/* Branch to exception handling if `vtop' is equal to `except_val' */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcheckerr(struct Dee_function_generator *__restrict self,
                                 intptr_t except_val) {
	struct Dee_memloc *loc;
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(self->fg_state->ms_stackc < 1)
		return err_illegal_stack_effect();
	loc = Dee_function_generator_vtop(self);
	ASSERTF(MEMLOC_VMORPH_ISDIRECT(loc->ml_vmorph), "non-sensical call: vcheckint() on non-direct value");
	if unlikely(Dee_function_generator_gjeq_except(self, loc, except_val))
		goto err;
	return Dee_function_generator_vpop(self);
err:
	return -1;
}


/* Generate a call to `DeeObject_MALLOC()' to allocate an uninitialized object that
 * provides for "alloc_size" bytes of memory. If possible, try to dispatch against
 * a slap allocator instead (just like the real DeeObject_MALLOC also does).
 * NOTE: The value pushed onto the V-stack already has its MEMLOC_F_NOREF flag CLEAR!
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_vcall_DeeObject_MALLOC(struct Dee_function_generator *__restrict self,
                                              size_t alloc_size) {
#ifndef CONFIG_NO_OBJECT_SLABS
	void const *api_function = NULL;
	size_t alloc_pointers = CEILDIV(alloc_size, sizeof(void *));
#define LOCAL_checkfit(_, num_pointers)                                   \
	if (alloc_pointers <= (num_pointers))                                 \
		api_function = (void const *)&DeeObject_SlabMalloc##num_pointers; \
	else
	DeeSlab_ENUMERATE(LOCAL_checkfit);
#undef LOCAL_checkfit
	if (api_function != NULL) {
		if unlikely(Dee_function_generator_vcallapi(self, api_function, VCALL_CC_OBJECT, 0))
			goto err;
	} else
#endif /* !CONFIG_NO_OBJECT_SLABS */
	{
		if unlikely(Dee_function_generator_vpush_immSIZ(self, alloc_size))
			goto err;
		if unlikely(Dee_function_generator_vcallapi(self, &DeeObject_Malloc, VCALL_CC_OBJECT, 1))
			goto err;
	}
	/* The NOREF flag must *NOT* be set (because the intend is for the caller to create an object) */
	ASSERT(!(Dee_function_generator_vtop(self)->ml_flags & MEMLOC_F_NOREF));
	Dee_function_generator_vtop(self)->ml_flags |= MEMLOC_F_ONEREF; /* Initial reference -> oneref */
	return 0;
err:
	return -1;
}



typedef struct {
	OBJECT_HEAD
	COMPILER_FLEXIBLE_ARRAY(void *, dvo_items);
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
 * @param: hstack_reserved: Cache of currently reserved hstack location (s.a. `MEMLOC_F_LINEAR') */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) ATTR_INS(4, 5) size_t DCALL
hstack_linear_score(bitset_t const *__restrict hstack_inuse,
                    bitset_t const *__restrict hstack_reserved,
                    uintptr_t hstack_cfa_offset,
                    struct Dee_memloc const *linbase,
                    Dee_vstackaddr_t linsize,
                    uintptr_t cfa_offset) {
	Dee_vstackaddr_t i;
	size_t result = 0;
	for (i = 0; i < linsize; ++i) {
		struct Dee_memloc const *src = &linbase[i];
#ifdef HOSTASM_STACK_GROWS_DOWN
		uintptr_t dst_cfa_offset = cfa_offset - i * HOST_SIZEOF_POINTER;
#else /* HOSTASM_STACK_GROWS_DOWN */
		uintptr_t dst_cfa_offset = cfa_offset + i * HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		if (src->ml_type == MEMLOC_TYPE_HSTACKIND &&
		    src->ml_value.v_hstack.s_cfa == dst_cfa_offset)
			continue; /* Already at the perfect location! */
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
Dee_memlocs_anyhstackind(struct Dee_memloc const *__restrict base,
                         Dee_vstackaddr_t count) {
	Dee_vstackaddr_t i;
	for (i = 0; i < count; ++i) {
		if (base[i].ml_type == MEMLOC_TYPE_HSTACKIND)
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
	if unlikely(Dee_function_generator_state_unshare(self))
		goto err;
	if unlikely(Dee_function_generator_vdirect(self, argc))
		goto err;
	if unlikely(!argc) {
		/* The base address of an empty vector doesn't matter, meaning it's undefined */
		return Dee_function_generator_vpush_undefined(self);
	} else if (readonly && Dee_function_generator_vallconst(self, argc)) {
		/* Dynamically allocate a dummy object which includes space
		 * for "argc" pointers. Then, fill those pointers with values
		 * from the v-stack, inline the reference to dummy object, and
		 * finally: push a pointer to the base address of the dummy's
		 * value array. */
		Dee_vstackaddr_t i;
		struct Dee_memloc *cbase = self->fg_state->ms_stackv + self->fg_state->ms_stackc - argc;
		DREF DummyVectorObject *vec = DummyVector_New(argc);
		if unlikely(!vec)
			goto err;
		vec = (DREF DummyVectorObject *)Dee_function_generator_inlineref(self, (DREF DeeObject *)vec);
		if unlikely(!vec)
			goto err;
		for (i = 0; i < argc; ++i)
			vec->dvo_items[i] = (void *)cbase[i].ml_value.v_const;
		if unlikely(Dee_function_generator_vpopmany(self, argc))
			goto err;
		return Dee_function_generator_vpush_addr(self, vec->dvo_items);
	} else if (argc == 1) {
		/* Deal with simple case: caller only wants the address of a single location.
		 * In this case, we only need to make sure that said location resides in
		 * memory (which is even allowed to be a REGIND location), and then push
		 * the address of that location. */
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
	} else {
		/* General case: figure out the optimal CFA base address of the linear vector. */
		Dee_vstackaddr_t i;
		uintptr_t result_cfa_offset;
		DREF struct Dee_memstate *linear_state;
		struct Dee_memstate *state = self->fg_state;
		struct Dee_memloc *linbase = state->ms_stackv + state->ms_stackc - argc;

		/* Check for special case: if none of the linear locations are HSTACKIND,
		 * then the optimal target location is always either a sufficiently large
		 * free region, or new newly alloca'd region. */
		if (!Dee_memlocs_anyhstackind(linbase, argc)) {
			size_t num_bytes = argc * sizeof(void *);
			result_cfa_offset = Dee_memstate_hstack_find(state, self->fg_state_hstack_res, num_bytes);
			if (result_cfa_offset == (uintptr_t)-1) {
				uintptr_t saved_cfa = state->ms_host_cfa_offset;
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
			uintptr_t cfa_offset_max;
			uintptr_t cfa_offset_min;
			uintptr_t cfa_offset;
			struct Dee_memloc *loc;
			bitset_t *hstack_inuse;    /* Bitset for currently in-use hstack locations (excluding locations used by linear slots) */
			bitset_t *hstack_reserved; /* Bitset of hstack locations that can never be used (because they belong to `MEMLOC_F_LINEAR' items) */
			size_t hstack_inuse_sizeof;
			hstack_inuse_sizeof = _bitset_sizeof((state->ms_host_cfa_offset / HOST_SIZEOF_POINTER) * 2);
			hstack_inuse = (bitset_t *)Dee_Calloca(hstack_inuse_sizeof * 2);
			if unlikely(!hstack_inuse)
				goto err;
			hstack_reserved = hstack_inuse + hstack_inuse_sizeof;
			Dee_memstate_foreach(loc, state) {
				if (loc->ml_type == MEMLOC_TYPE_HSTACKIND) {
					uintptr_t cfa = Dee_memloc_getcfastart(loc);
					ASSERT(cfa < state->ms_host_cfa_offset);
					bitset_set(hstack_inuse, cfa / HOST_SIZEOF_POINTER);
					if (loc->ml_flags & MEMLOC_F_LINEAR)
						bitset_set(hstack_reserved, cfa / HOST_SIZEOF_POINTER);
				}
			}
			Dee_memstate_foreach_end;
			/* hstack locations currently in use by the linear portion don't count as in-use.
			 * NOTE: We do this in a second pass, so we also hit all of the aliases. */
			for (i = 0; i < argc; ++i) {
				if (linbase[i].ml_type == MEMLOC_TYPE_HSTACKIND) {
					uintptr_t cfa = Dee_memloc_getcfastart(&linbase[i]);
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
#define TYPE_LINLOC MEMLOC_TYPE_UNALLOC
#define TYPE_ALIAS  MEMLOC_TYPE_UNDEFINED
		for (i = 0; i < argc; ++i) {
			struct Dee_memloc *alias, *loc = &linbase[i];
			struct Dee_memloc locval = *loc;
			loc->ml_type = TYPE_LINLOC;
			loc->ml_value._v_next = NULL;
			Dee_memstate_foreach(alias, linear_state) {
				if (alias->ml_type == TYPE_LINLOC || alias->ml_type == TYPE_ALIAS)
					continue;
				if (Dee_memloc_sameloc(alias, &locval)) {
					alias->ml_type = TYPE_ALIAS;
					alias->ml_value._v_next = loc->ml_value._v_next;
					loc->ml_value._v_next = alias;
				}
			}
			Dee_memstate_foreach_end;
		}
		for (i = 0; i < argc; ++i) {
			struct Dee_memloc *loc = &linbase[i];
			if (loc->ml_type == TYPE_LINLOC) {
				struct Dee_memloc *next;
#ifdef HOSTASM_STACK_GROWS_DOWN
				uintptr_t dst_cfa_offset = result_cfa_offset - i * HOST_SIZEOF_POINTER;
#else /* HOSTASM_STACK_GROWS_DOWN */
				uintptr_t dst_cfa_offset = result_cfa_offset + i * HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
				do {
					next = loc->ml_value._v_next;
					loc->ml_type = MEMLOC_TYPE_HSTACKIND;
					loc->ml_value.v_hstack.s_cfa = dst_cfa_offset;
					loc->ml_value.v_hstack.s_off = 0;
				} while ((loc = next) != NULL);
			}
		}
#undef TYPE_LINLOC
#undef TYPE_ALIAS

		/* Fix locations where linear elements were aliasing each other. */
		for (i = 0; i < argc; ++i) {
#ifdef HOSTASM_STACK_GROWS_DOWN
			uintptr_t dst_cfa_offset = result_cfa_offset - i * HOST_SIZEOF_POINTER;
#else /* HOSTASM_STACK_GROWS_DOWN */
			uintptr_t dst_cfa_offset = result_cfa_offset + i * HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			struct Dee_memloc *loc = &linbase[i];
			ASSERT(loc->ml_type == MEMLOC_TYPE_HSTACKIND);
			loc->ml_value.v_hstack.s_cfa = dst_cfa_offset;
			loc->ml_flags |= MEMLOC_F_LINEAR; /* Not allowed to move until popped */
		}

		/* Make sure that `linear_state's CFA offset is large enough to hold the linear vector. */
		{
#ifdef HOSTASM_STACK_GROWS_DOWN
			uintptr_t req_min_host_cfa = result_cfa_offset;
#else /* HOSTASM_STACK_GROWS_DOWN */
			uintptr_t req_min_host_cfa = result_cfa_offset + argc * HOST_SIZEOF_POINTER;
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
	return Dee_function_generator_vcallapi(self, me->fei_cva_func, VCALL_CC_VOID, me->fei_cva_argc);
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_VSTACK_C */
