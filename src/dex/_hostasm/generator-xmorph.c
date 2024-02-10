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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_XMORPH_C
#define GUARD_DEX_HOSTASM_GENERATOR_XMORPH_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/alloc.h>
#include <deemon/none.h>

DECL_BEGIN

STATIC_ASSERT(sizeof(struct Dee_memval) == sizeof(struct Dee_memref));
STATIC_ASSERT(offsetof(struct Dee_memval, mv_obj.mvo_0.mo_xinfo) == offsetof(struct Dee_memref, _mr_always0_1));
STATIC_ASSERT(offsetof(struct Dee_memval, mv_obj.mvo_0.mo_loc) == offsetof(struct Dee_memref, mr_loc));
STATIC_ASSERT(offsetof(struct Dee_memval, mv_obj.mvo_0.mo_typeof) == offsetof(struct Dee_memref, mr_refc));
STATIC_ASSERT(offsetof(struct Dee_memval, mv_obj.mvo_0.mo_flags) == offsetof(struct Dee_memref, _mr_always0_2));
STATIC_ASSERT(offsetof(struct Dee_memval, mv_vmorph) == offsetof(struct Dee_memref, _mr_always0_3));
STATIC_ASSERT(offsetof(struct Dee_memval, mv_flags) == offsetof(struct Dee_memref, _mr_always0_4));

/* Assign a score to the complexity of moving "from" to "to"
 * 
 * If the move is impossible (can happen if "to" is CONST and
 * differs from FROM), return `(size_t)-1' */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) size_t DCALL
memref_mov_score(struct Dee_memref const *__restrict from,
                 struct Dee_memref const *__restrict to) {
	STATIC_ASSERT(MEMADR_TYPE_CONST == 0);
	STATIC_ASSERT(MEMADR_TYPE_UNDEFINED == 2);
	STATIC_ASSERT(MEMADR_TYPE_HSTACK == 4);
	STATIC_ASSERT(MEMADR_TYPE_HSTACKIND == 5);
	STATIC_ASSERT(MEMADR_TYPE_HREG == 6);
	STATIC_ASSERT(MEMADR_TYPE_HREGIND == 7);

	/* Prefer locations with higher scores here! */
	PRIVATE uint8_t const move_score_for_type[] = {
		/* [MEMADR_TYPE_CONST]     = */ 4,
		/* [1]                     = */ 0,
		/* [MEMADR_TYPE_UNDEFINED] = */ 0,
		/* [3]                     = */ 0,
		/* [MEMADR_TYPE_HSTACK]    = */ 4,
		/* [MEMADR_TYPE_HSTACKIND] = */ 6,
		/* [MEMADR_TYPE_HREG]      = */ 2,
		/* [MEMADR_TYPE_HREGIND]   = */ 6,
	};

	size_t result = 0;
	if (!Dee_memloc_sameloc(&from->mr_loc, &to->mr_loc)) {
		if (Dee_memloc_gettyp(&to->mr_loc) == MEMADR_TYPE_CONST)
			return (size_t)-1; /* Impossible move (can't move into fixed constant) */
		if (Dee_memloc_gettyp(&to->mr_loc) == MEMADR_TYPE_HREGIND)
			return (size_t)-1; /* Impossible move (can't move into possibly externally visible address) */
		result += 2; /* Base score for move */
		result += move_score_for_type[Dee_memloc_gettyp(&from->mr_loc)];
		result += move_score_for_type[Dee_memloc_gettyp(&to->mr_loc)];
	}

	/* Penalty for extra code: "if (!from) from = Dee_None;" */
	if ((from->mr_flags & MEMREF_F_NULLABLE) && !(to->mr_flags & MEMREF_F_NULLABLE))
		result += 20;

	/* Penalty for extra code: "from->ob_refcnt += ..." */
	if (from->mr_refc != to->mr_refc) {
		if (from->mr_flags & MEMREF_F_NULLABLE)
			result += 10; /* Extra NULL-check */
		result += 2;
	}

	if (to->mr_flags & MEMREF_F_NOKILL) {
		if (!(from->mr_flags & MEMREF_F_NOKILL))
			return (size_t)-1; /* Impossible move -> target expects a _nokill source */
	} else if (from->mr_flags & MEMREF_F_NOKILL) {
		result += 1; /* Penalty for normal decref where _nokill would be possible */
	}

	return result;
}

/* @param: return_for_any: When true, return for any matching memloc.
 *                         When false, only return if there is a single matching memloc. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) Dee_vstackaddr_t DCALL
find_notdone_oldrefi_for_memadr(struct Dee_memstate const *self,
                                struct Dee_memadr const *adr,
                                bool return_for_any) {
	Dee_vstackaddr_t lo = 0;
	Dee_vstackaddr_t hi = self->ms_stackc;
	while (lo < hi) {
		Dee_vstackaddr_t mid = (lo + hi) / 2;
		struct Dee_memref const *mref = &((struct Dee_memref const *)self->ms_stackv)[mid];
		int cmp = memcmp(adr, &mref->mr_loc.ml_adr, sizeof(struct Dee_memadr));
		if (cmp < 0) {
			hi = mid;
		} else if (cmp > 0) {
			lo = mid + 1;
		} else {
			/* Got a match */
			size_t count;
			Dee_vstackaddr_t result = (Dee_vstackaddr_t)-1;
			while (mid > 0) { /* Find the start of the alias list */
				mref = &((struct Dee_memref const *)self->ms_stackv)[mid - 1];
				if (memcmp(adr, &mref->mr_loc.ml_adr, sizeof(struct Dee_memadr)) != 0)
					break;
				--mid;
			}
			/* See if there is an alias that isn't _MEMREF_F_DONE */
			count = 0;
			for (;;) {
				mref = &((struct Dee_memref const *)self->ms_stackv)[mid];
				if (!(mref->mr_flags & _MEMREF_F_DONE)) {
					result = mid;
					++count;
					if (!return_for_any && count > 1)
						return (Dee_vstackaddr_t)-1;
				}
				++mid;
				mref = &((struct Dee_memref const *)self->ms_stackv)[mid + 1];
				if (memcmp(adr, &mref->mr_loc.ml_adr, sizeof(struct Dee_memadr)) != 0)
					break;
			}
			return result;
		}
	}
	return (Dee_vstackaddr_t)-1;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) Dee_vstackaddr_t DCALL
find_notdone_oldrefi_for_cfa_boundary(struct Dee_memstate const *__restrict self) {
	struct Dee_memadr cfa_boundary;
#ifdef HOSTASM_STACK_GROWS_DOWN
	Dee_memadr_init_hstackind(&cfa_boundary, self->ms_host_cfa_offset);
#else /* HOSTASM_STACK_GROWS_DOWN */
	Dee_memadr_init_hstackind(&cfa_boundary, self->ms_host_cfa_offset - HOST_SIZEOF_POINTER);
#endif /* !HOSTASM_STACK_GROWS_DOWN */
	return find_notdone_oldrefi_for_memadr(self, &cfa_boundary, false);
}

/* Re-sort memory locations by `Dee_memref_compare()'
 * Also update indices in `oldref_targets' */
PRIVATE NONNULL((1, 2)) void DCALL
sort_memlocs(struct Dee_memstate *__restrict self,
             Dee_vstackaddr_t *__restrict oldref_targets) {
	size_t i, j;
	struct Dee_memref *refv = (struct Dee_memref *)self->ms_stackv;
	Dee_vstackaddr_t refc   = self->ms_stackc;
	/* TODO: Better sort algorithm? */
	for (i = 0; i < refc; ++i) {
		for (j = i + 1; j < refc; ++j) {
			struct Dee_memref *a = &refv[i];
			struct Dee_memref *b = &refv[j];
			if (memcmp(a, b, sizeof(struct Dee_memref)) > 0) {
				struct Dee_memref temp;
				Dee_vstackaddr_t temp2;
				temp = *a;
				*a = *b;
				*b = temp;
				temp2 = oldref_targets[i];
				oldref_targets[i] = oldref_targets[j];
				oldref_targets[j] = temp2;
			}
		}
	}

}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_xmorph_impl(struct Dee_function_generator *__restrict self,
                                   struct Dee_except_exitinfo_id *__restrict newinfo) {
	struct Dee_memref *new_oldrefv;
	struct Dee_memstate *state = self->fg_state;
	struct Dee_memref *oldrefv = (struct Dee_memref *)state->ms_stackv;
	struct Dee_memref *newrefv = newinfo->exi_memrefv;
	Dee_vstackaddr_t oldrefi, oldrefc = state->ms_stackc;
	Dee_vstackaddr_t newrefi, newrefc = newinfo->exi_memrefc;
	Dee_vstackaddr_t *oldref_targets;

	/* Calculate register usage counters in `state'. */
	bzero(state->ms_rinuse, sizeof(state->ms_rinuse));
	bzero(state->ms_rusage, sizeof(state->ms_rusage));
	for (oldrefi = 0; oldrefi < oldrefc; ++oldrefi) {
		struct Dee_memref *oldref = &oldrefv[oldrefi];
		Dee_memstate_incrinuse_for_memloc(state, &oldref->mr_loc);
	}
	_Dee_memstate_verifyrinuse(state);

	/* Indices into `newrefv' detailing which location to use as target for "oldrefv".
	 * Elements that don't have sources get the `_MEMREF_F_NOSRC' flag set. */
	oldref_targets = (Dee_vstackaddr_t *)Dee_Mallocac(oldrefc, sizeof(Dee_vstackaddr_t));
	if unlikely(!oldref_targets)
		goto err;
	for (oldrefi = 0; oldrefi < oldrefc; ++oldrefi)
		oldref_targets[oldrefi] = (Dee_vstackaddr_t)-1;
	for (oldrefi = 0, newrefi = 0; newrefi < newrefc; ++newrefi) {
		Dee_vstackaddr_t test_oldrefi;
		Dee_vstackaddr_t best_oldrefi;
		size_t best_oldrefi_score;
		struct Dee_memref *newref = &newrefv[newrefi];
		ASSERTF(!(newref->mr_flags & _MEMREF_F_NOSRC),
		        "This flag mustn't already be set for anything!");

		/* Fast check: does the same location appear in the old state? */
		while (oldrefi < oldrefc) {
			struct Dee_memref *oldref = &oldrefv[oldrefi];
			int cmp = Dee_memref_compare(oldref, newref);
			if (cmp < 0) {
				/* Appears-before -> skip */
				++oldrefi;
			} else if (cmp > 0) {
				/* Appears-after -> newref doesn't appear in oldrefv */
				break;
			} else {
				/* Found it! -> See if it is a perfect match. */
				if ((oldref->mr_refc == newref->mr_refc) &&
				    (oldref_targets[oldrefi] == (Dee_vstackaddr_t)-1) &&
				    ((newref->mr_flags & MEMREF_F_NULLABLE) || !(oldref->mr_flags & MEMREF_F_NULLABLE)) &&
				    (!(newref->mr_flags & MEMREF_F_NOKILL) || (oldref->mr_flags & MEMREF_F_NOKILL))) {
					oldref_targets[oldrefi] = newrefi;
					++oldrefi;
					goto bind_next_newref;
				}
				++oldrefi;
				break;
			}
		}

		/* Search through unbound old references and check
		 * which match results in the lowest binding score. */
		best_oldrefi = (Dee_vstackaddr_t)-1;
		best_oldrefi_score = (size_t)-1;
		for (test_oldrefi = 0; test_oldrefi < oldrefc; ++test_oldrefi) {
			size_t score;
			if (oldref_targets[test_oldrefi] != (Dee_vstackaddr_t)-1)
				continue; /* Reference is already used as a source -> can't use again. */
			score = memref_mov_score(&oldrefv[test_oldrefi], newref);
			if (best_oldrefi_score > score) {
				best_oldrefi = test_oldrefi;
				best_oldrefi_score = score;
			}
		}

		ASSERT(best_oldrefi < oldrefc || best_oldrefi == (Dee_vstackaddr_t)-1);
		if (best_oldrefi < oldrefc) {
			/* Set-up "best_oldrefi" to be the source for "newrefi" */
			ASSERT(oldref_targets[best_oldrefi] == (Dee_vstackaddr_t)-1);
			oldref_targets[best_oldrefi] = newrefi;
		} else {
			/* Impossible to match -> can happen for references to constants.
			 * -> Remember that this reference doesn't have a source, so it
			 *    can be created from scratch. */
			newref->mr_flags |= _MEMREF_F_NOSRC;
		}
bind_next_newref:;
	}

	/* Sources for all new references have been assigned. With this in mind, we must do:
	 * #1: Go through "oldref_targets" and drop references from all objects that don't
	 *     have targets. Those that do have targets, move to those targets. Special care
	 *     also needs to be taken where the move target is the source of another ref to
	 *     be moved.
	 *     Also: have special handling when "pop" can be used to load the next source.
	 * #2: Re-order "oldrefv" to have the same element order as "newrefv"
	 * #3: Go through "newrefv" and create Dee_None refs for all references that don't
	 *     have a source in the old state (as per `_MEMREF_F_NOSRC'). At the same time,
	 *     insert these new locations into "oldrefv".
	 * #4: Construct a fake `Dee_memstate' for "newinfo"
	 * #5: Use `Dee_function_generator_vmorph()' to force any flushed registers/etc back
	 *     into their proper place (as well as do some final adjustment of the CFA). */
	for (;;) {
		Dee_vstackaddr_t initial_oldrefi;
		Dee_vstackaddr_t target_newrefi;
		struct Dee_memref *oldref;

		/* Prefer moving objects at the CFA boundary. */
		oldrefi = find_notdone_oldrefi_for_cfa_boundary(state);
		ASSERT(oldrefi < oldrefc || oldrefi == (Dee_vstackaddr_t)-1);
		if (oldrefi == (Dee_vstackaddr_t)-1) {
			for (oldrefi = 0; oldrefi < oldrefc; ++oldrefi) {
				if (!(oldrefv[oldrefi].mr_flags & _MEMREF_F_DONE))
					break;
			}
			if (oldrefi >= oldrefc)
				break;
		}
		initial_oldrefi = oldrefi;

		/* Figure out what's supposed to happen with "source_oldrefi" */
do_move_source_oldrefi:
		oldref = &oldrefv[oldrefi];
		ASSERT(!(oldref->mr_flags & _MEMREF_F_DONE));
		target_newrefi = oldref_targets[oldrefi];
		ASSERT(target_newrefi < newrefc || target_newrefi == (Dee_vstackaddr_t)-1);
		if (target_newrefi == (Dee_vstackaddr_t)-1) {
			/* Simple case: get rid of this reference. */
			int temp;
			if (oldref->mr_flags & MEMREF_F_NULLABLE) {
				if (oldref->mr_flags & MEMREF_F_NOKILL) {
					temp = Dee_function_generator_gxdecref_nokill_loc(self, &oldref->mr_loc, oldref->mr_refc);
				} else {
					temp = Dee_function_generator_gxdecref_loc(self, &oldref->mr_loc, oldref->mr_refc);
				}
			} else {
				if (oldref->mr_flags & MEMREF_F_NOKILL) {
					temp = Dee_function_generator_gdecref_nokill_loc(self, &oldref->mr_loc, oldref->mr_refc);
				} else {
					temp = Dee_function_generator_gdecref_loc(self, &oldref->mr_loc, oldref->mr_refc);
				}
			}
			if unlikely(temp)
				goto err_oldref_targets;
			Dee_memstate_decrinuse_for_memloc(state, &oldref->mr_loc);
			Dee_memloc_init_undefined(&oldref->mr_loc);
			oldref->mr_refc  = 0;
			oldref->mr_flags = MEMREF_F_NORMAL;
		} else {
			/* Complicated case: must move a reference (and possibly adjust its count).
			 * Also need to check if the address of "newref" also appears  */
			struct Dee_memref const *newref = &newrefv[target_newrefi];
			Dee_vstackaddr_t oldref_inuse_i;
			if (!Dee_memadr_sameadr(&oldref->mr_loc.ml_adr, &newref->mr_loc.ml_adr)) {
				oldref_inuse_i = find_notdone_oldrefi_for_memadr(state, &newref->mr_loc.ml_adr, true);
				ASSERT(oldref_inuse_i < oldrefc || oldref_inuse_i == (Dee_vstackaddr_t)-1);
				if (oldref_inuse_i != (Dee_vstackaddr_t)-1) {
					ASSERT(!(oldrefv[oldref_inuse_i].mr_flags & _MEMREF_F_DONE));
					/* "oldref_inuse_i" needs to happen *before* "oldrefi" can happen */
					if unlikely(initial_oldrefi == oldref_inuse_i) {
						/* Infinite loop. Must "oldref_inuse_i" to a different,
						 * unused location in order to break the loop. */
						Dee_host_register_t tempreg;
						ptrdiff_t tempreg_delta;
						tempreg = Dee_function_generator_gallocreg(self, NULL);
						if unlikely(tempreg >= HOST_REGISTER_COUNT)
							goto err_oldref_targets;
						oldref = &oldrefv[oldref_inuse_i];
						if unlikely(Dee_function_generator_gmov_loc2regy(self, &oldref->mr_loc, tempreg, &tempreg_delta))
							goto err_oldref_targets;
						Dee_memstate_decrinuse_for_memloc(state, &oldref->mr_loc);
						Dee_memloc_init_hreg(&oldref->mr_loc, tempreg, tempreg_delta);
						Dee_memstate_incrinuse(state, tempreg);
					}
					oldrefi = oldref_inuse_i;
					goto do_move_source_oldrefi;
				}
				/* Do the move */
				if unlikely(Dee_function_generator_gmov_loc2loc(self, &oldref->mr_loc, &newref->mr_loc))
					goto err_oldref_targets;
				Dee_memstate_decrinuse_for_memloc(state, &oldref->mr_loc);
				oldref->mr_loc = newref->mr_loc;
				Dee_memstate_incrinuse_for_memloc(state, &oldref->mr_loc);
			}
			if ((oldref->mr_flags & MEMREF_F_NULLABLE) &&
			    !(newref->mr_flags & MEMREF_F_NULLABLE)) {
				/* Supplement missing references with "Dee_None" */
				int temp;
				struct Dee_host_symbol *Lnotnull;
				DREF struct Dee_memstate *common_state;
				Lnotnull = Dee_function_generator_newsym_named(self, ".Lnotnull");
				if unlikely(!Lnotnull)
					goto err_oldref_targets;
				if unlikely(Dee_function_generator_gjnz(self, &oldref->mr_loc, Lnotnull))
					goto err_oldref_targets;
				common_state = self->fg_state;
				Dee_memstate_incref(common_state);
				temp = Dee_function_generator_gmov_const2loc(self, Dee_None, &oldref->mr_loc);
				if likely(temp == 0)
					temp = Dee_function_generator_gincref_loc(self, &oldref->mr_loc, oldref->mr_refc);
				Dee_memstate_decref(self->fg_state);
				self->fg_state = common_state;
				if unlikely(temp)
					goto err_oldref_targets;
				Dee_host_symbol_setsect(Lnotnull, self->fg_sect);
				oldref->mr_flags &= ~MEMREF_F_NULLABLE;
			}
			if (oldref->mr_refc != newref->mr_refc) {
				/* Adjust refcnts */
				int temp;
				if (oldref->mr_refc < newref->mr_refc) {
					Dee_refcnt_t delta = newref->mr_refc - oldref->mr_refc;
					temp = (oldref->mr_flags & MEMREF_F_NULLABLE)
					       ? Dee_function_generator_gxincref_loc(self, &oldref->mr_loc, delta)
					       : Dee_function_generator_gincref_loc(self, &oldref->mr_loc, delta);
				} else {
					Dee_refcnt_t delta = oldref->mr_refc - newref->mr_refc;
					temp = (oldref->mr_flags & MEMREF_F_NULLABLE)
					       ? Dee_function_generator_gxdecref_nokill_loc(self, &oldref->mr_loc, delta)
					       : Dee_function_generator_gdecref_nokill_loc(self, &oldref->mr_loc, delta);
				}
				if unlikely(temp)
					goto err;
			}
		}
		oldref->mr_flags |= _MEMREF_F_DONE;

		/* Re-sort locations since stuff may have gotten flushed/re-loaded. */
		sort_memlocs(state, oldref_targets);
	}

	/* #2: Re-order "oldrefv" to have the same element order as "newrefv":
	 * >> new_oldrefv[i] = oldref_targets[i] == -1 ? <undefined> : oldrefv[oldref_targets[i]]; */
	new_oldrefv = (struct Dee_memref *)Dee_Callocac(newrefc, sizeof(struct Dee_memref));
	if unlikely(!new_oldrefv)
		goto err;
#if MEMADR_TYPE_HASREG(0)
	for (newrefi = 0; newrefi < newrefc; ++newrefi)
		new_oldrefv[newrefi].mr_loc.ml_adr.ma_typ = MEMADR_TYPE_CONST;
#endif /* MEMADR_TYPE_HASREG(0) */
	for (oldrefi = 0; oldrefi < oldrefc; ++oldrefi) {
		newrefi = oldref_targets[oldrefi];
		if (newrefi != (Dee_vstackaddr_t)-1)
			new_oldrefv[newrefi] = oldrefv[oldrefi];
	}
	state->ms_stackc = newrefc;
	state->ms_stacka = newrefc;
	state->ms_stackv = (struct Dee_memval *)new_oldrefv;

	/* #3: Go through "newrefv" and create Dee_None refs for all references that don't
	 *     have a source in the old state (as per `_MEMREF_F_NOSRC'). At the same time,
	 *     insert these new locations into "oldrefv". */
	for (newrefi = 0; newrefi < newrefc; ++newrefi) {
		struct Dee_memloc constval;
		struct Dee_memref *newref = &newrefv[newrefi];
		if (!(newref->mr_flags & _MEMREF_F_NOSRC))
			continue;
		constval = newref->mr_loc;
		if (!Dee_memloc_isconst(&constval))
			Dee_memloc_init_const(&constval, Dee_None);
		if unlikely(Dee_function_generator_gmov_loc2loc(self, &constval, &newref->mr_loc))
			goto err_oldref_targets_new_oldrefv;
		if unlikely(Dee_function_generator_gincref_loc(self, &constval, newref->mr_refc))
			goto err_oldref_targets_new_oldrefv;
		ASSERTF(new_oldrefv[newrefi].mr_loc.ml_adr.ma_typ == 0,
		        "Should not have been initialized above");
		new_oldrefv[newrefi] = *newref;
		Dee_memstate_incrinuse_for_memloc(state, &newref->mr_loc);
	}

	{
		int temp;
		void *_newstatebuf[Dee_memstate_sizeof(1) / sizeof(void *)];
		struct Dee_memstate *newstate = (struct Dee_memstate *)_newstatebuf;

		/* #4: Construct a fake `Dee_memstate' for "newinfo" */
		newstate->ms_host_cfa_offset = newinfo->exi_cfa_offset;
		newstate->ms_refcnt    = 1;
		newstate->ms_localc    = 1;
		newstate->ms_uargc_min = 0;
		newstate->ms_flags     = MEMSTATE_F_NORMAL;
		newstate->ms_stacka    = newrefc;
		newstate->ms_stackc    = newrefc;
		newstate->ms_stackv    = (struct Dee_memval *)newrefv;
		newstate->ms_localv[0] = state->ms_localv[0];

		/* #5: Use `Dee_function_generator_vmorph()' to force any flushed registers/etc back
		 *     into their proper place (as well as do some final adjustment of the CFA). */
		Dee_memequivs_init(&newstate->ms_memequiv);
		temp = Dee_function_generator_vmorph_no_constrain_equivalences(self, newstate);
		Dee_memequivs_fini(&newstate->ms_memequiv);
		if unlikely(temp)
			goto err_oldref_targets_new_oldrefv;
	}

	/* Cleanup */
	Dee_Freea(new_oldrefv);
	Dee_Freea(oldref_targets);
	return 0;
err_oldref_targets_new_oldrefv:
	Dee_Freea(new_oldrefv);
err_oldref_targets:
	Dee_Freea(oldref_targets);
err:
	return -1;
}


/* Generate code to morph the reference state from "oldinfo" to "newinfo" */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_function_generator_xmorph(struct Dee_function_generator *__restrict self,
                              struct Dee_except_exitinfo_id const *__restrict oldinfo,
                              struct Dee_except_exitinfo_id *__restrict newinfo) {
	int result;
	struct Dee_memval *stackv;
	struct Dee_memstate *state = self->fg_state;
	ASSERT(state);
	ASSERT(state->ms_stacka == 0);
	ASSERT(state->ms_stackc == 0);
	ASSERT(state->ms_stackv == NULL);
	stackv = (struct Dee_memval *)Dee_Mallocac(oldinfo->exi_memrefc, sizeof(struct Dee_memval));
	if unlikely(!stackv)
		goto err;
	stackv = (struct Dee_memval *)memcpyc(stackv, oldinfo->exi_memrefv,
	                                      oldinfo->exi_memrefc,
	                                      sizeof(struct Dee_memval));
	state->ms_host_cfa_offset = oldinfo->exi_cfa_offset;
	state->ms_stackc = oldinfo->exi_memrefc;
	state->ms_stacka = oldinfo->exi_memrefc;
	state->ms_stackv = stackv;

	/* Do the morph */
	result = Dee_function_generator_xmorph_impl(self, newinfo);

	/* Cleanup */
	ASSERT(result || state->ms_host_cfa_offset == newinfo->exi_cfa_offset);
	ASSERT(result || state->ms_stackc == newinfo->exi_memrefc);
	Dee_Freea(stackv);
	state->ms_stackc = 0;
	state->ms_stacka = 0;
	state->ms_stackv = NULL;
	return result;
err:
	return -1;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_XMORPH_C */
