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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_VMORPH_C
#define GUARD_DEX_HOSTASM_GENERATOR_VMORPH_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_Freea, Dee_Mallocac */
#include <deemon/error.h>           /* DeeError_* */
#include <deemon/format.h>          /* PRFu16 */
#include <deemon/system-features.h> /* memmovedownc, memset */

#include <hybrid/sequence/list.h> /* SLIST_* */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, ptrdiff_t, size_t */

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

struct memaction;
SLIST_HEAD(memaction_slist, memaction);
struct memaction {
	struct memobj         *ma_oldobj;   /* [0..1] Old location, or NULL if already done (though in the case a register, it may be gotten flushed again) */
	struct memobj const   *ma_newobj;   /* [1..1] New location */
	struct memaction_slist ma_before;   /* [0..n] List of actions that need to happen *before* this one */
	SLIST_ENTRY(memaction) ma_bflink;   /* Link in some other action's `ma_before' */
};

/* Check if `self' has already happened. */
#define memaction_isdone(self) ((self)->ma_oldobj == NULL)

/* Check if all actions that need to happen before `self' already have. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
memaction_isready(struct memaction const *__restrict self) {
	struct memaction *iter;
	SLIST_FOREACH (iter, &self->ma_before, ma_bflink) {
		if (!memaction_isdone(iter))
			return false;
	}
	return true;
}

/* Search for a ready- and not-yet-done memory action where:
 * - the old location is placed such that it can pop from `host_cfa_offset'
 * - the new location is placed such that it can push to `host_cfa_offset'
 */
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) struct memaction *DCALL
memaction_find_push_or_pop_at_cfa_boundary(struct memaction *mactv, lid_t mactc,
                                           host_cfa_t host_cfa_offset) {
	lid_t i;
	for (i = 0; i < mactc; ++i) {
		struct memaction *act = &mactv[i];
		if (memaction_isdone(act))
			continue;
		if (!memaction_isready(act))
			continue;
		if ((memobj_gettyp(act->ma_oldobj) == MEMADR_TYPE_HSTACKIND && /* Is pop possible? */
		     memobj_getcfaend(act->ma_oldobj) == host_cfa_offset) ||
		    (memobj_gettyp(act->ma_newobj) == MEMADR_TYPE_HSTACKIND && /* Is push possible? */
		     memobj_getcfastart(act->ma_newobj) == host_cfa_offset))
			return act;
	}
	return NULL;
}

/* Return the action that references the greatest CFA in its `ma_oldobj'
 * If no such action exists, return `NULL' instead. */
PRIVATE WUNUSED ATTR_INS(1, 2) struct memaction *DCALL
memaction_at_greatest_oldobj_cfa(struct memaction *mactv, lid_t mactc) {
	lid_t i;
	struct memaction *result = NULL;
	for (i = 0; i < mactc; ++i) {
		struct memaction *act = &mactv[i];
		if (memaction_isdone(act))
			continue;
		if (!memaction_isready(act))
			continue;
		if (memobj_gettyp(act->ma_oldobj) != MEMADR_TYPE_HSTACKIND)
			continue;
		if ((result == NULL) ||
		    (memobj_hstackind_getcfa(result->ma_oldobj) <
		     memobj_hstackind_getcfa(act->ma_oldobj)))
			result = act;
	}
	return result;
}

/* Remove aliases (and throw an error an error if something is
 * an alias in the new state, but wasn't one in the old state)
 * @return: * :            The new # of actions
 * @return: (lid_t)-1: Error */
PRIVATE ATTR_INOUTS(1, 2) lid_t DCALL
memaction_remove_aliases(struct memaction *mactv, lid_t mactc) {
	lid_t i;
	for (i = 0; i < mactc; ++i) {
		lid_t j;
		struct memaction *act = &mactv[i];
		for (j = i + 1; j < mactc;) {
			struct memaction *alias = &mactv[j];
			if (memobj_sameadr(act->ma_newobj, alias->ma_newobj)) {
				ptrdiff_t old_value_delta;
				ptrdiff_t new_value_delta;
				if (!memobj_sameadr(act->ma_oldobj, alias->ma_oldobj)) {
cannot_morph:
					return (lid_t)DeeError_Throwf(&DeeError_IllegalInstruction,
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
				old_value_delta = memobj_getoff(alias->ma_oldobj) - memobj_getoff(act->ma_oldobj);
				new_value_delta = memobj_getoff(alias->ma_newobj) - memobj_getoff(act->ma_newobj);
				if (old_value_delta != new_value_delta)
					goto cannot_morph;

				/* Remove the alias. */
				--mactc;
				memmovedownc(&mactv[j],
				             &mactv[j + 1],
				             mactc - j,
				             sizeof(struct memaction));
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
 * - mactv[i].ma_before = [act in mactv if memobj_sameadr(act->ma_oldobj, mactv[i].ma_newobj)]
 * Every action should only ever appear at most *once* in any other action's before-list.
 * This is because aliases were already removed, meaning that every `ma_newobj' is
 * distinct (when it comes to the underlying base memory location), which also means
 * that (if you look at the pseudo-code above), every `ma_oldobj' can equal at most
 * one `ma_newobj'. */
PRIVATE ATTR_INOUTS(1, 2) void DCALL
memaction_assign_before(struct memaction *mactv, lid_t mactc) {
	lid_t i, j;
	for (i = 0; i < mactc; ++i) {
		struct memaction *act = &mactv[i];
		for (j = 0; j < mactc; ++j) {
			struct memaction *after_act;
			if (i == j)
				continue;
			after_act = &mactv[j];
			if (memobj_sameadr(act->ma_oldobj, after_act->ma_newobj)) {
				SLIST_INSERT(&after_act->ma_before, act, ma_bflink);
				break;
			}
		}
	}
}


PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
morph_location(struct fungen *__restrict self,
               struct memobj *oldobj,
               struct memobj const *newobj,
               bool check_different) {
	struct memstate *state = self->fg_state;
	if (!memobj_local_alwaysbound(oldobj)) {
		if (memobj_local_alwaysbound(newobj))
			return DeeError_Throwf(&DeeError_IllegalInstruction, "Cannot morph maybe/never bound -> bound");
		if (memobj_local_neverbound(oldobj)) {
			if (memobj_local_neverbound(newobj))
				return 0; /* Value is never bound -> nothing to move! */
			/* Value is conditionally bound in the target -> must generate code to assign "0" to it. */
			oldobj->mo_flags |= MEMOBJ_F_MAYBEUNBOUND;
			oldobj->mo_flags |= newobj->mo_flags & MEMOBJ_F_ISREF;
			ASSERT(memobj_isnull(oldobj));
			goto do_move_to_newobj;
		}
	} else {
		oldobj->mo_flags |= newobj->mo_flags & MEMOBJ_F_MAYBEUNBOUND;
	}
	if ((oldobj->mo_flags & MEMOBJ_F_ISREF) != (newobj->mo_flags & MEMOBJ_F_ISREF)) {
		bool must_incref = newobj->mo_flags & MEMOBJ_F_ISREF;
		/* NOTE: Can always use decref_nokill() here, since the only situation where
		 *       the caller is ever allowed to morph code such that something is no
		 *       longer a reference, is when it is known that the reference is question
		 *       is also being held someplace else!
		 * TODO: This is incorrect. `MEMSTATE_XLOCAL_POPITER' should be decref'd normally
		 *       if the location isn't being aliased elsewhere. As such, the decref *can*
		 *       kill if there isn't an alias. */
		if (memobj_local_alwaysbound(oldobj)) {
			if unlikely(must_incref ? fg_gincref_loc(self, memobj_getloc(oldobj), 1)
			                        : fg_gdecref_nokill_loc(self, memobj_getloc(oldobj), 1))
				goto err;
		} else {
			if unlikely(must_incref ? fg_gxincref_loc(self, memobj_getloc(oldobj), 1)
			                        : fg_gxdecref_nokill_loc(self, memobj_getloc(oldobj), 1))
				goto err;
		}
		oldobj->mo_flags &= ~MEMOBJ_F_ISREF;
		oldobj->mo_flags |= newobj->mo_flags & MEMOBJ_F_ISREF;
	}
	if (!check_different || !memobj_sameloc(oldobj, newobj)) {
do_move_to_newobj:
		if unlikely(fg_gmov_loc2loc(self,
		                                                memobj_getloc(oldobj),
		                                                memobj_getloc(newobj)))
			goto err;
		memstate_decrinuse_for_memobj(state, oldobj);
		*oldobj = *newobj;
		memstate_incrinuse_for_memobj(state, oldobj);
	}
	return 0;
err:
	return -1;
}

/* Generate code in `fg_gettext(self)' to morph `self->fg_state' into `new_state' */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vmorph_no_constrain_equivalences(struct fungen *__restrict self,
                                    struct memstate const *new_state) {
	lid_t i, j, mactc, finished;
	struct memaction *mactv;
	struct memstate *old_state;
	struct memstate const *saved_fg_state_hstack_res;
	unsigned int kind;

	/* Fast-pass: if they're literally the same states, then we've got nothing to do! */
	if (self->fg_state == new_state)
		return 0;
	if unlikely(self->fg_state->ms_stackc != new_state->ms_stackc) {
#ifndef NO_HOSTASM_DEBUG_PRINT
		_memstate_debug_print(self->fg_state, self->fg_assembler, NULL);
		_memstate_debug_print(new_state, self->fg_assembler, NULL);
#endif /* !NO_HOSTASM_DEBUG_PRINT */
		DeeError_Throwf(&DeeError_IllegalInstruction,
		                "Unbalanced stack depth when trying to morph code. "
		                "Both %" PRFu16 " and %" PRFu16 " encountered",
		                self->fg_state->ms_stackc,
		                new_state->ms_stackc);
		goto err;
	}
	if (fg_state_unshare(self))
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
		lid_t valc = old_state->ms_localc;
		struct memval *old_valv = old_state->ms_localv;
		struct memval const *new_valv = new_state->ms_localv;
		if (kind != 0) {
			valc = old_state->ms_stackc;
			old_valv = old_state->ms_stackv;
			new_valv = new_state->ms_stackv;
		}
		for (i = 0; i < valc; ++i) {
			struct memobj *oldobjv;
			struct memobj const *newobjv;
			struct memval *oldval = &old_valv[i];
			struct memval const *newval = &new_valv[i];
			size_t loci, locc = memval_getobjc(oldval);
			if (memval_sameval(oldval, newval))
				continue;
			if unlikely(oldval->mv_vmorph != newval->mv_vmorph) {
				ASSERTF(newval->mv_vmorph == MEMVAL_VMORPH_DIRECT,
				        "In the case of incompatible morphs, `memstate_constrainwith()' "
				        "should have normalized to `MEMVAL_VMORPH_DIRECT'!");
				if unlikely(fg_vdirect_memval(self, oldval))
					goto err;
				old_valv = old_state->ms_localv;
				if (kind != 0)
					old_valv = old_state->ms_stackv;
				oldval = &old_valv[i];
				ASSERT(memval_isdirect(oldval));
				oldval->mv_vmorph = MEMVAL_VMORPH_DIRECT;
				goto again_search_changes; /* Must restart because vdirect() may have flushed registers */
			}
			ASSERTF(locc == memval_getobjc(newval),
			        "Incompatible location counts (this should have "
			        "been handled by `memstate_constrainwith()')");
			oldobjv = memval_getobjv(oldval);
			newobjv = memval_getobjv(newval);
			for (loci = 0; loci < locc; ++loci) {
				struct memobj *oldobj = &oldobjv[loci];
				struct memobj const *newobj = &newobjv[loci];
				if (!memobj_sameloc(oldobj, newobj))
					++mactc;
			}
		}
	}

	/* Handle (simple) special case: only a single action needs to be performed. */
	if (mactc == 0) {
		ptrdiff_t cfa_delta = (ptrdiff_t)new_state->ms_host_cfa_offset -
		                      (ptrdiff_t)old_state->ms_host_cfa_offset;
		if (cfa_delta != 0)
			return fg_ghstack_adjust(self, cfa_delta);
		return 0;
	}

	/* Allocate the vector used to keep track of necessary changes. */
	mactv = (struct memaction *)Dee_Mallocac(mactc, sizeof(struct memaction));
	if unlikely(!mactv)
		goto err;
	j = 0;
	for (kind = 0; kind < 2; ++kind) {
		lid_t valc = old_state->ms_localc;
		struct memval *old_valv = old_state->ms_localv;
		struct memval const *new_valv = new_state->ms_localv;
		if (kind != 0) {
			valc = old_state->ms_stackc;
			old_valv = old_state->ms_stackv;
			new_valv = new_state->ms_stackv;
		}
		for (i = 0; i < valc; ++i) {
			struct memobj *oldobjv;
			struct memobj const *newobjv;
			struct memval *oldval = &old_valv[i];
			struct memval const *newval = &new_valv[i];
			size_t loci, locc = memval_getobjc(oldval);
			if (memval_sameval(oldval, newval))
				continue;
			ASSERT(oldval->mv_vmorph == newval->mv_vmorph);
			ASSERTF(locc == memval_getobjc(newval),
			        "Incompatible location counts (this should have "
			        "been handled by `memstate_constrainwith()')");
			oldobjv = memval_getobjv(oldval);
			newobjv = memval_getobjv(newval);
			for (loci = 0; loci < locc; ++loci) {
				struct memobj *oldobj = &oldobjv[loci];
				struct memobj const *newobj = &newobjv[loci];
				if (!memobj_sameloc(oldobj, newobj)) {
					ASSERT(j < mactc);
					ASSERT(kind == 0 || !(oldobj->mo_flags & MEMOBJ_F_MAYBEUNBOUND));
					ASSERT(kind == 0 || !(newobj->mo_flags & MEMOBJ_F_MAYBEUNBOUND));
					mactv[j].ma_oldobj = oldobj;
					mactv[j].ma_newobj = newobj;
					++j;
				}
			}
		}
	}
	ASSERT(j == mactc);
	mactc = memaction_remove_aliases(mactv, mactc);
	if unlikely(mactc == (lid_t)-1)
		goto err_actv;
	memaction_assign_before(mactv, mactc);

	/* Set-up the assembler for state morphing */
	saved_fg_state_hstack_res = self->fg_state_hstack_res;
	self->fg_state_hstack_res = new_state;
	finished = 0;

	/* Adjust the CFA offset to match that of the target state. */
	if (new_state->ms_host_cfa_offset != old_state->ms_host_cfa_offset) {
		ptrdiff_t cfa_delta;
		/* NOTE: Before adjusting the CFA, execute actions that can be
		 *       served by pushing/popping registers at the CFA boundary. */
		struct memaction *early_act;
again_search_for_push_or_pop:
		while ((early_act = memaction_find_push_or_pop_at_cfa_boundary(mactv, mactc,
		                                                               old_state->ms_host_cfa_offset)) != NULL) {
			ASSERT(!memaction_isdone(early_act));
			ASSERT(memaction_isready(early_act));
			if unlikely(morph_location(self, early_act->ma_oldobj, early_act->ma_newobj, false))
				goto err_actv_restore;
			early_act->ma_oldobj = NULL; /* Mark as done */
			++finished;
		}
		cfa_delta = (ptrdiff_t)new_state->ms_host_cfa_offset -
		            (ptrdiff_t)old_state->ms_host_cfa_offset;
		if (cfa_delta != 0) {
			/* Special case to prevent broken code:
			 * >> fg_vmorph: 000a -> 0003
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
			 *           `memaction_find_push_or_pop_at_cfa_boundary()' for
			 *           another pop-style morph.
			 */
			if (cfa_delta < 0) {
				struct memaction *next_cfa_act;
				next_cfa_act = memaction_at_greatest_oldobj_cfa(mactv, mactc);
				if (next_cfa_act &&
				    memobj_getcfaend(next_cfa_act->ma_oldobj) > new_state->ms_host_cfa_offset) {
					/* This action needs to be performed *before* the stack is adjusted! */
					ptrdiff_t next_cfa_act_delta;
					next_cfa_act_delta = (ptrdiff_t)memobj_getcfaend(next_cfa_act->ma_oldobj) -
					                     (ptrdiff_t)old_state->ms_host_cfa_offset;
					if unlikely(fg_ghstack_adjust(self, next_cfa_act_delta))
						goto err_actv_restore;
					goto again_search_for_push_or_pop;
				}
			}
			if unlikely(fg_ghstack_adjust(self, cfa_delta))
				goto err_actv_restore;
		}
	}

	/* Initial pass where we perform actions that don't contain dependency loops. */
	do {
		bool did_something = false;
		for (i = 0; i < mactc; ++i) {
			struct memaction *act = &mactv[i];
			if (memaction_isdone(act))
				continue;
			if (!memaction_isready(act))
				continue;

			/* Perform this action */
			if unlikely(morph_location(self, act->ma_oldobj, act->ma_newobj, false))
				goto err_actv_restore;

			act->ma_oldobj = NULL; /* Mark as done */
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
		lid_t valc = old_state->ms_localc;
		struct memval *old_valv = old_state->ms_localv;
		struct memval const *new_valv = new_state->ms_localv;
		if (kind != 0) {
			valc = old_state->ms_stackc;
			old_valv = old_state->ms_stackv;
			new_valv = new_state->ms_stackv;
		}
		for (i = 0; i < valc; ++i) {
			struct memobj *oldobjv;
			struct memobj const *newobjv;
			struct memval *oldval = &old_valv[i];
			struct memval const *newval = &new_valv[i];
			size_t loci, locc = memval_getobjc(oldval);
			ASSERT(oldval->mv_vmorph == newval->mv_vmorph);
			ASSERTF(locc == memval_getobjc(newval),
			        "Incompatible location counts (this should have "
			        "been handled by `memstate_constrainwith()')");
			oldobjv = memval_getobjv(oldval);
			newobjv = memval_getobjv(newval);
			for (loci = 0; loci < locc; ++loci) {
				struct memobj *oldobj = &oldobjv[loci];
				struct memobj const *newobj = &newobjv[loci];
				if unlikely(morph_location(self, oldobj, newobj, true))
					goto err;
			}
		}
	}

	/* Make sure that the CFA differences between the 2 states are resolved. */
	{
		ptrdiff_t cfa_delta = (ptrdiff_t)new_state->ms_host_cfa_offset -
		                      (ptrdiff_t)old_state->ms_host_cfa_offset;
		if (cfa_delta != 0) {
			if unlikely(fg_ghstack_adjust(self, cfa_delta))
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

/* Same as `fg_vmorph_no_constrain_equivalences()', but also
 * generate code to constrain the equivalences from `new_state' into "self". */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
fg_vmorph(struct fungen *__restrict self,
          struct memstate const *new_state) {
	int result = fg_vmorph_no_constrain_equivalences(self, new_state);
	if likely(result == 0)
		memequivs_constrainwith(&self->fg_state->ms_memequiv, &new_state->ms_memequiv);
	return result;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_VMORPH_C */
