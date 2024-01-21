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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_MORPH_C
#define GUARD_DEX_HOSTASM_GENERATOR_MORPH_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/alloc.h>
#include <deemon/error.h>
#include <deemon/format.h>

#include <hybrid/sequence/list.h>

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

struct Dee_memaction;
SLIST_HEAD(Dee_memaction_slist, Dee_memaction);
struct Dee_memaction {
	struct Dee_memloc         *ma_oldloc;   /* [0..1] Old location, or NULL if already done (though in the case a
	                                         * register, it may be gotten flushed again) (always a hasloc0 value) */
	struct Dee_memloc const   *ma_newloc;   /* [1..1] New location (always a hasloc0 value) */
	uint8_t                    ma_oldflags; /* "mv_flags" of the old location */
	uint8_t                    ma_newflags; /* "mv_flags" of the new location */
	uint8_t _ma_pad[sizeof(void *) - 2];    /* ... */
	struct Dee_memaction_slist ma_before;   /* [0..n] List of actions that need to happen *before* this one */
	SLIST_ENTRY(Dee_memaction) ma_bflink;   /* Link in some other action's `ma_before' */
};

/* Check if `self' has already happened. */
#define Dee_memaction_isdone(self) ((self)->ma_oldloc == NULL)

/* Check if all actions that need to happen before `self' already have. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_memaction_isready(struct Dee_memaction const *__restrict self) {
	struct Dee_memaction *iter;
	SLIST_FOREACH (iter, &self->ma_before, ma_bflink) {
		if (!Dee_memaction_isdone(iter))
			return false;
	}
	return true;
}

/* Search for a ready- and not-yet-done memory action where:
 * - the old location is placed such that it can pop from `host_cfa_offset'
 * - the new location is placed such that it can push to `host_cfa_offset'
 */
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) struct Dee_memaction *DCALL
Dee_memaction_find_push_or_pop_at_cfa_boundary(struct Dee_memaction *mactv, Dee_lid_t mactc,
                                               uintptr_t host_cfa_offset) {
	Dee_lid_t i;
	for (i = 0; i < mactc; ++i) {
		struct Dee_memaction *act = &mactv[i];
		if (Dee_memaction_isdone(act))
			continue;
		if (!Dee_memaction_isready(act))
			continue;
		if ((Dee_memloc_gettyp(act->ma_oldloc) == MEMADR_TYPE_HSTACKIND && /* Is pop possible? */
		     Dee_memloc_getcfaend(act->ma_oldloc) == host_cfa_offset) ||
		    (Dee_memloc_gettyp(act->ma_newloc) == MEMADR_TYPE_HSTACKIND && /* Is push possible? */
		     Dee_memloc_getcfastart(act->ma_newloc) == host_cfa_offset))
			return act;
	}
	return NULL;
}

/* Return the action that references the greatest CFA in its `ma_oldloc'
 * If no such action exists, return `NULL' instead. */
PRIVATE WUNUSED ATTR_INS(1, 2) struct Dee_memaction *DCALL
Dee_memaction_at_greatest_oldloc_cfa(struct Dee_memaction *mactv, Dee_lid_t mactc) {
	Dee_lid_t i;
	struct Dee_memaction *result = NULL;
	for (i = 0; i < mactc; ++i) {
		struct Dee_memaction *act = &mactv[i];
		if (Dee_memaction_isdone(act))
			continue;
		if (!Dee_memaction_isready(act))
			continue;
		if (act->ma_oldloc->ml_adr.ma_typ != MEMADR_TYPE_HSTACKIND)
			continue;
		if (result == NULL ||
		    result->ma_oldloc->ml_adr.ma_val.v_cfa < act->ma_oldloc->ml_adr.ma_val.v_cfa)
			result = act;
	}
	return result;
}

/* Remove aliases (and throw an error an error if something is
 * an alias in the new state, but wasn't one in the old state)
 * @return: * :            The new # of actions
 * @return: (Dee_lid_t)-1: Error */
PRIVATE ATTR_INOUTS(1, 2) Dee_lid_t DCALL
Dee_memaction_remove_aliases(struct Dee_memaction *mactv, Dee_lid_t mactc) {
	Dee_lid_t i;
	for (i = 0; i < mactc; ++i) {
		Dee_lid_t j;
		struct Dee_memaction *act = &mactv[i];
		for (j = i + 1; j < mactc;) {
			struct Dee_memaction *alias = &mactv[j];
			if (Dee_memloc_sameadr(act->ma_newloc, alias->ma_newloc)) {
				ptrdiff_t old_value_delta;
				ptrdiff_t new_value_delta;
				if (!Dee_memloc_sameadr(act->ma_oldloc, alias->ma_oldloc)) {
cannot_morph:
					return (Dee_lid_t)DeeError_Throwf(&DeeError_IllegalInstruction,
					                                  "Cannot morph memory state. Have at least 2 "
					                                  "distinct values for singular memory location.");
				}
				/* Make sure that value-addend-deltas between the original action and its alias are identical:
				 * >> old_state = [%eax+4, %eax+8]
				 * >> new_state = [%eax+4, %eax+12]
				 *
				 * This would be impossible to morph, and so would this:
				 * >> old_state = [0(%eax)+4, 4(%eax)+8]
				 * >> new_state = [0(%eax)+4, 4(%eax)+12]
				 * This would be impossible to morph!
				 */
				old_value_delta = Dee_memloc_getoff(alias->ma_oldloc) - Dee_memloc_getoff(act->ma_oldloc);
				new_value_delta = Dee_memloc_getoff(alias->ma_newloc) - Dee_memloc_getoff(act->ma_newloc);
				if (old_value_delta != new_value_delta)
					goto cannot_morph;

				/* Remove the alias. */
				--mactc;
				memmovedownc(&mactv[j],
				             &mactv[j + 1],
				             mactc - j,
				             sizeof(struct Dee_memaction));
			} else {
				++j;
			}
		}
		SLIST_INIT(&act->ma_before);
		DBG_memset(&act->ma_bflink, 0xcc, sizeof(act->ma_bflink));
	}
	return mactc;
}


/* Fill in the `ma_before' of actions:
 * - mactv[i].ma_before = [act in mactv if Dee_memloc_sameadr(act->ma_oldloc, mactv[i].ma_newloc)]
 * Every action should only ever appear at most *once* in any other action's before-list.
 * This is because aliases were already removed, meaning that every `ma_newloc' is
 * distinct (when it comes to the underlying base memory location), which also means
 * that (if you look at the pseudo-code above), every `ma_oldloc' can equal at most
 * one `ma_newloc'. */
PRIVATE ATTR_INOUTS(1, 2) void DCALL
Dee_memaction_assign_before(struct Dee_memaction *mactv, Dee_lid_t mactc) {
	Dee_lid_t i, j;
	for (i = 0; i < mactc; ++i) {
		struct Dee_memaction *act = &mactv[i];
		for (j = 0; j < mactc; ++j) {
			struct Dee_memaction *after_act;
			if (i == j)
				continue;
			after_act = &mactv[j];
			if (Dee_memloc_sameadr(act->ma_oldloc, after_act->ma_newloc)) {
				SLIST_INSERT(&after_act->ma_before, act, ma_bflink);
				break;
			}
		}
	}
}


PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
morph_location(struct Dee_function_generator *__restrict self,
               struct Dee_memloc *old_loc, uint8_t old_val_flags,
               struct Dee_memloc const *new_loc, uint8_t new_val_flags,
               bool check_different) {
	struct Dee_memstate *state = self->fg_state;
	if (old_val_flags & MEMVAL_F_LOCAL_UNBOUND) {
		if (new_val_flags & MEMVAL_F_LOCAL_UNBOUND)
			return 0; /* Value is never bound -> nothing to move! */
		if (new_val_flags & MEMVAL_F_LOCAL_BOUND)
			return DeeError_Throwf(&DeeError_IllegalInstruction, "Cannot morph unbound -> not unbound");
		/* Value is conditionally bound in the target -> must generate code to assign "0" to it. */
		old_val_flags &= ~(MEMVAL_F_NOREF | MEMVAL_M_LOCAL_BSTATE);
		old_val_flags |= new_val_flags & MEMVAL_F_NOREF;
		Dee_memstate_decrinuse_for_memloc(state, old_loc);
		Dee_memloc_init_const(old_loc, NULL);
		goto do_move_to_new_loc;
	}
	if (old_val_flags & MEMVAL_F_LOCAL_BOUND) {
		if (!(new_val_flags & MEMVAL_F_LOCAL_BOUND))
			old_val_flags &= ~MEMVAL_F_LOCAL_BOUND;
	} else if (new_val_flags & MEMVAL_F_LOCAL_BOUND) {
		return DeeError_Throwf(&DeeError_IllegalInstruction, "Cannot morph maybe/not bound -> bound");
	}
	if ((old_val_flags & MEMVAL_F_NOREF) != (new_val_flags & MEMVAL_F_NOREF)) {
		bool must_incref = old_val_flags & MEMVAL_F_NOREF;
		/* NOTE: Can always use decref_nokill() here, since the only situation where
		 *       the caller is ever allowed to morph code such that something is no
		 *       longer a reference, is when it is known that the reference is question
		 *       is also being held someplace else!
		 * TODO: This is incorrect. `MEMSTATE_XLOCAL_POPITER' should be decref'd normally
		 *       if the location isn't being aliased elsewhere. As such, the decref *can*
		 *       kill if there isn't an alias. */
		if (!(old_val_flags & MEMVAL_F_LOCAL_BOUND)) {
			if unlikely(must_incref ? Dee_function_generator_gxincref_loc(self, old_loc, 1)
			                        : Dee_function_generator_gxdecref_nokill_loc(self, old_loc, 1))
				goto err;
		} else {
			if unlikely(must_incref ? Dee_function_generator_gincref_loc(self, old_loc, 1)
			                        : Dee_function_generator_gdecref_nokill_loc(self, old_loc, 1))
				goto err;
		}
		old_val_flags &= ~MEMVAL_F_NOREF;
		old_val_flags |= new_val_flags & MEMVAL_F_NOREF;
	}
	if (!check_different || !Dee_memloc_sameloc(old_loc, new_loc)) {
do_move_to_new_loc:
		if unlikely(Dee_function_generator_gmov_loc2loc(self, old_loc, new_loc))
			goto err;
		Dee_memstate_decrinuse_for_memloc(state, old_loc);
		*old_loc = *new_loc;
		Dee_memstate_incrinuse_for_memloc(state, old_loc);
	}
	return 0;
err:
	return -1;
}

/* Generate code in `self->fg_sect' to morph `self->fg_state' into `new_state' */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vmorph_no_constrain_equivalences(struct Dee_function_generator *__restrict self,
                                                        struct Dee_memstate const *new_state) {
	Dee_lid_t i, j, mactc, finished;
	struct Dee_memaction *mactv;
	struct Dee_memstate *old_state;
	struct Dee_memstate const *saved_fg_state_hstack_res;
	unsigned int kind;

	/* Fast-pass: if they're literally the same states, then we've got nothing to do! */
	if (self->fg_state == new_state)
		return 0;
	if unlikely(self->fg_state->ms_stackc != new_state->ms_stackc) {
#ifndef NO_HOSTASM_DEBUG_PRINT
		_Dee_memstate_debug_print(self->fg_state, self->fg_assembler, NULL);
		_Dee_memstate_debug_print(new_state, self->fg_assembler, NULL);
#endif /* !NO_HOSTASM_DEBUG_PRINT */
		DeeError_Throwf(&DeeError_IllegalInstruction,
		                "Unbalanced stack depth when trying to morph code. "
		                "Both %" PRFu16 " and %" PRFu16 " encountered",
		                self->fg_state->ms_stackc,
		                new_state->ms_stackc);
		goto err;
	}
	if (Dee_function_generator_state_unshare(self))
		goto err;
	
	/* Morph one memory state to match what is required by another:
	 * - Find all memory locations that need to be altered
	 * - For each altered memory location, see if another altered one uses the
	 *   new location's memory/register location (w/o value added) as its old
	 *   value. Wherever this is the case, remember that other location as a
	 *   must-happen-before dependency of the first location.
	 * - Recursively transform memory location wherever happens-before is empty,
	 *   or all happens-before have already been performed.
	 *   - This step also does incref/decref_nokill of locations
	 *   - When flushing registers here, only consider stack locations as usable
	 *     that aren't in-use by `new_state'
	 *   - When writing to a stack location, automatically make sure that it is
	 *     always in-bounds of the stack (by increasing the CFA offset)
	 * - As long as there are still remaining locations, take one of them, move
	 *   its old value into a register (that's not designated as the new-location
	 *   of any of the actions in the same dependency-loop), which should thereby
	 *   break the loop and allow more actions to be flushed.
	 *   - If no such register exists, that must mean that *all* registers are
	 *     part of the dependency loop. In this case, save one of them to stack
	 *   - When flushing registers here, only consider stack locations as usable
	 *     that aren't in-use by `new_state' (s.a. `fg_state_hstack_res')
	 * - With all memory actions served, load any registers that needed to be
	 *   flushed during the transformation phase above.
	 * - Adjust CFA offset to match `new_state' (possibly needs to be done a
	 *   second time due to extra stack space needed for saving registers)
	 */
	old_state = self->fg_state;
again_search_changes:
	mactc = 0;
	for (kind = 0; kind < 2; ++kind) {
		Dee_lid_t valc = old_state->ms_localc;
		struct Dee_memval *old_valv = old_state->ms_localv;
		struct Dee_memval const *new_valv = new_state->ms_localv;
		if (kind != 0) {
			valc = old_state->ms_stackc;
			old_valv = old_state->ms_stackv;
			new_valv = new_state->ms_stackv;
		}
		for (i = 0; i < valc; ++i) {
			struct Dee_memloc *oldlocv;
			struct Dee_memloc const *newlocv;
			struct Dee_memval *oldval = &old_valv[i];
			struct Dee_memval const *newval = &new_valv[i];
			size_t loci, locc = Dee_memval_getlocc(oldval);
			if (Dee_memval_sameval(oldval, newval))
				continue;
			if unlikely(oldval->mv_vmorph != newval->mv_vmorph) {
				ASSERTF(newval->mv_vmorph == MEMVAL_VMORPH_DIRECT,
				        "In the case of incompatible morphs, `Dee_memstate_constrainwith()' "
				        "should have normalized to `MEMVAL_VMORPH_DIRECT'!");
				if unlikely(Dee_function_generator_gdirect(self, oldval))
					goto err;
				ASSERT(Dee_memval_isdirect(oldval));
				oldval->mv_vmorph = MEMVAL_VMORPH_DIRECT;
				goto again_search_changes; /* Must restart because gdirect() may have flushed registers */
			}
			ASSERTF(locc == Dee_memval_getlocc(newval),
			        "Incompatible location counts (this should have "
			        "been handled by `Dee_memstate_constrainwith()')");
			oldlocv = Dee_memval_getlocv(oldval);
			newlocv = Dee_memval_getlocv(newval);
			for (loci = 0; loci < locc; ++loci) {
				struct Dee_memloc *oldloc = &oldlocv[loci];
				struct Dee_memloc const *newloc = &newlocv[loci];
				if (!Dee_memloc_sameloc(oldloc, newloc))
					++mactc;
			}
		}
	}

	/* Handle (simple) special case: only a single action needs to be performed. */
	if (mactc == 0) {
		ptrdiff_t cfa_delta = (ptrdiff_t)new_state->ms_host_cfa_offset -
		                      (ptrdiff_t)old_state->ms_host_cfa_offset;
		if (cfa_delta != 0)
			return Dee_function_generator_ghstack_adjust(self, cfa_delta);
		return 0;
	}

	/* Allocate the vector used to keep track of necessary changes. */
	mactv = (struct Dee_memaction *)Dee_Mallocac(mactc, sizeof(struct Dee_memaction));
	if unlikely(!mactv)
		goto err;
	j = 0;
	for (kind = 0; kind < 2; ++kind) {
		Dee_lid_t valc = old_state->ms_localc;
		struct Dee_memval *old_valv = old_state->ms_localv;
		struct Dee_memval const *new_valv = new_state->ms_localv;
		if (kind != 0) {
			valc = old_state->ms_stackc;
			old_valv = old_state->ms_stackv;
			new_valv = new_state->ms_stackv;
		}
		for (i = 0; i < valc; ++i) {
			struct Dee_memloc *oldlocv;
			struct Dee_memloc const *newlocv;
			struct Dee_memval *oldval = &old_valv[i];
			struct Dee_memval const *newval = &new_valv[i];
			size_t loci, locc = Dee_memval_getlocc(oldval);
			if (Dee_memval_sameval(oldval, newval))
				continue;
			ASSERT(oldval->mv_vmorph == newval->mv_vmorph);
			ASSERTF(locc == Dee_memval_getlocc(newval),
			        "Incompatible location counts (this should have "
			        "been handled by `Dee_memstate_constrainwith()')");
			oldlocv = Dee_memval_getlocv(oldval);
			newlocv = Dee_memval_getlocv(newval);
			for (loci = 0; loci < locc; ++loci) {
				struct Dee_memloc *oldloc = &oldlocv[loci];
				struct Dee_memloc const *newloc = &newlocv[loci];
				if (!Dee_memloc_sameloc(oldloc, newloc)) {
					ASSERT(j < mactc);
					mactv[j].ma_oldloc   = oldloc;
					mactv[j].ma_oldflags = oldval->mv_flags;
					mactv[j].ma_newloc   = newloc;
					mactv[j].ma_newflags = newval->mv_flags;
					if (kind == 0) {
						/* Stack locations always behave as though unconditionally bound. */
						mactv[j].ma_oldflags &= ~MEMVAL_M_LOCAL_BSTATE;
						mactv[j].ma_newflags &= ~MEMVAL_M_LOCAL_BSTATE;
						mactv[j].ma_oldflags |= MEMVAL_F_LOCAL_BOUND;
						mactv[j].ma_newflags |= MEMVAL_F_LOCAL_BOUND;
					}
					++j;
				}
			}
		}
	}
	ASSERT(j == mactc);
	mactc = Dee_memaction_remove_aliases(mactv, mactc);
	if unlikely(mactc == (Dee_lid_t)-1)
		goto err_actv;
	Dee_memaction_assign_before(mactv, mactc);

	/* Set-up the assembler for state morphing */
	saved_fg_state_hstack_res = self->fg_state_hstack_res;
	self->fg_state_hstack_res = new_state;
	finished = 0;

	/* Adjust the CFA offset to match that of the target state. */
	if (new_state->ms_host_cfa_offset != old_state->ms_host_cfa_offset) {
		ptrdiff_t cfa_delta;
		/* NOTE: Before adjusting the CFA, execute actions that can be
		 *       served by pushing/popping registers at the CFA boundary. */
		struct Dee_memaction *early_act;
again_search_for_push_or_pop:
		while ((early_act = Dee_memaction_find_push_or_pop_at_cfa_boundary(mactv, mactc,
		                                                                   old_state->ms_host_cfa_offset)) != NULL) {
			ASSERT(!Dee_memaction_isdone(early_act));
			ASSERT(Dee_memaction_isready(early_act));
			if unlikely(morph_location(self,
			                           early_act->ma_oldloc, early_act->ma_oldflags,
			                           early_act->ma_newloc, early_act->ma_newflags,
			                           false))
				goto err_actv_restore;
			early_act->ma_oldloc = NULL; /* Mark as done */
			++finished;
		}
		cfa_delta = (ptrdiff_t)new_state->ms_host_cfa_offset -
		            (ptrdiff_t)old_state->ms_host_cfa_offset;
		if (cfa_delta != 0) {
			/* Special case to prevent broken code:
			 * >> Dee_function_generator_vmorph: 000a -> 0003
			 * >>     CFA:   #8
			 * >>     stack: r[#4]
			 * >>     CFA:   #0
			 * >>     stack: r%eax
			 * >> hostasm:addl	$8, %esp
			 * >> hostasm:movl	-4(%esp), %eax
			 *
			 * Correct code would be:
			 * >> hostasm:addl	$4, %esp
			 * >> hostasm:popl	%eax
			 *
			 * Solution: When `cfa_delta < 0', only release up until the next
			 *           value that still lies on the stack but should also be
			 *           deallocated, then jump back to the while-loop to check
			 *           `Dee_memaction_find_push_or_pop_at_cfa_boundary()' for
			 *           another pop-style morph.
			 */
			if (cfa_delta < 0) {
				struct Dee_memaction *next_cfa_act;
				next_cfa_act = Dee_memaction_at_greatest_oldloc_cfa(mactv, mactc);
				if (next_cfa_act &&
				    Dee_memloc_getcfaend(next_cfa_act->ma_oldloc) > new_state->ms_host_cfa_offset) {
					/* This action needs to be performed *before* the stack is adjusted! */
					ptrdiff_t next_cfa_act_delta;
					next_cfa_act_delta = (ptrdiff_t)Dee_memloc_getcfaend(next_cfa_act->ma_oldloc) -
					                     (ptrdiff_t)old_state->ms_host_cfa_offset;
					if unlikely(Dee_function_generator_ghstack_adjust(self, next_cfa_act_delta))
						goto err_actv_restore;
					goto again_search_for_push_or_pop;
				}
			}
			if unlikely(Dee_function_generator_ghstack_adjust(self, cfa_delta))
				goto err_actv_restore;
		}
	}

	/* Initial pass where we perform actions that don't contain dependency loops. */
	do {
		bool did_something = false;
		for (i = 0; i < mactc; ++i) {
			struct Dee_memaction *act = &mactv[i];
			if (Dee_memaction_isdone(act))
				continue;
			if (!Dee_memaction_isready(act))
				continue;

			/* Perform this action */
			if unlikely(morph_location(self,
			                           act->ma_oldloc, act->ma_oldflags,
			                           act->ma_newloc, act->ma_newflags,
			                           false))
				goto err_actv_restore;

			act->ma_oldloc = NULL; /* Mark as done */
			did_something  = true;
			++finished;
		}
		if (!did_something)
			break;
	} while (finished < mactc);

	if (finished < mactc) {
		/* TODO: Perform actions that have dependency loops */
		DeeError_NOTIMPLEMENTED();
		goto err_actv;
	}

	/* Check if there is anything that should be a register in new
	 * state, but was flushed to the stack in the old state. */
	for (kind = 0; kind < 2; ++kind) {
		Dee_lid_t valc = old_state->ms_localc;
		struct Dee_memval *old_valv = old_state->ms_localv;
		struct Dee_memval const *new_valv = new_state->ms_localv;
		if (kind != 0) {
			valc = old_state->ms_stackc;
			old_valv = old_state->ms_stackv;
			new_valv = new_state->ms_stackv;
		}
		for (i = 0; i < valc; ++i) {
			struct Dee_memloc *oldlocv;
			struct Dee_memloc const *newlocv;
			struct Dee_memval *oldval = &old_valv[i];
			struct Dee_memval const *newval = &new_valv[i];
			size_t loci, locc = Dee_memval_getlocc(oldval);
			ASSERT(oldval->mv_vmorph == newval->mv_vmorph);
			ASSERTF(locc == Dee_memval_getlocc(newval),
			        "Incompatible location counts (this should have "
			        "been handled by `Dee_memstate_constrainwith()')");
			oldlocv = Dee_memval_getlocv(oldval);
			newlocv = Dee_memval_getlocv(newval);
			for (loci = 0; loci < locc; ++loci) {
				struct Dee_memloc *oldloc = &oldlocv[loci];
				struct Dee_memloc const *newloc = &newlocv[loci];
				if unlikely(morph_location(self,
				                           oldloc, oldval->mv_flags,
				                           newloc, newval->mv_flags,
				                           true))
					goto err;
			}
		}
	}

	/* Make sure that the CFA differences between the 2 states are resolved. */
	{
		ptrdiff_t cfa_delta = (ptrdiff_t)new_state->ms_host_cfa_offset -
		                      (ptrdiff_t)old_state->ms_host_cfa_offset;
		if (cfa_delta != 0) {
			if unlikely(Dee_function_generator_ghstack_adjust(self, cfa_delta))
				goto err_actv_restore;
		}
	}

	self->fg_state_hstack_res = saved_fg_state_hstack_res;
	Dee_Freea(mactv);
	return 0;
err_actv_restore:
	self->fg_state_hstack_res = saved_fg_state_hstack_res;
err_actv:
	Dee_Freea(mactv);
err:
	return -1;
}

/* Same as `Dee_function_generator_vmorph_no_constrain_equivalences()', but also
 * generate code to constrain the equivalences from `new_state' into "self". */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_vmorph(struct Dee_function_generator *__restrict self,
                              struct Dee_memstate const *new_state) {
	int result = Dee_function_generator_vmorph_no_constrain_equivalences(self, new_state);
	if likely(result == 0)
		Dee_memequivs_constrainwith(&self->fg_state->ms_memequiv, &new_state->ms_memequiv);
	return result;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_MORPH_C */
