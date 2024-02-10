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
#ifndef GUARD_DEX_HOSTASM_MEMSTATE_C
#define GUARD_DEX_HOSTASM_MEMSTATE_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/alloc.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/module.h>
#include <deemon/none.h>

#include <hybrid/limitcore.h>
#include <hybrid/overflow.h>
#include <hybrid/sequence/bitset.h>
#include <hybrid/typecore.h>
#include <hybrid/unaligned.h>

#ifndef UINT16_MAX
#define UINT16_MAX __UINT16_MAX__
#endif /* !UINT16_MAX */

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#ifdef Dee_memval_initmove_IS_MEMCPY
#define Dee_memval_vec_movedown(dst, src, num_items) \
	((struct Dee_memval *)memmovedownc(dst, src, num_items, sizeof(struct Dee_memval)))
#define Dee_memval_vec_moveup(dst, src, num_items) \
	((struct Dee_memval *)memmoveupc(dst, src, num_items, sizeof(struct Dee_memval)))
#else /* Dee_memval_initmove_IS_MEMCPY */
PRIVATE ATTR_RETNONNULL ATTR_INS(2, 3) ATTR_OUTS(1, 3) struct Dee_memval *DCALL
Dee_memval_vec_movedown(struct Dee_memval *dst,
                        struct Dee_memval const *src,
                        size_t num_items) {
	ASSERT(dst <= src || !num_items);
	if (dst != src) {
		size_t i = num_items;
		while (i) {
			--i;
			Dee_memval_initmove(&dst[i], &src[i]);
		}
	}
	return dst;
}

PRIVATE ATTR_RETNONNULL ATTR_INS(2, 3) ATTR_OUTS(1, 3) struct Dee_memval *DCALL
Dee_memval_vec_moveup(struct Dee_memval *dst,
                      struct Dee_memval const *src,
                      size_t num_items) {
	ASSERT(dst >= src || !num_items);
	if (dst != src) {
		size_t i;
		for (i = 0; i < num_items; ++i)
			Dee_memval_initmove(&dst[i], &src[i]);
	}
	return dst;
}
#endif /* !Dee_memval_initmove_IS_MEMCPY */

/* Check if there is a register that contains `usage'.
 * Returns some value `>= HOST_REGISTER_COUNT' if non-existent. */
INTERN ATTR_PURE WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_memstate_hregs_find_usage(struct Dee_memstate const *__restrict self,
                              Dee_host_regusage_t usage) {
	Dee_host_register_t result;
	for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
		if (self->ms_rusage[result] == usage)
			break;
	}
	return result;
}

/* Check if there is a register that is completely unused.
 * Returns some value `>= HOST_REGISTER_COUNT' if non-existent.
 * @param: accept_if_with_regusage: When true, allowed to return registers with
 *                                  `ms_rusage[return] != DEE_HOST_REGUSAGE_GENERIC' */
INTERN ATTR_PURE WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_memstate_hregs_find_unused(struct Dee_memstate const *__restrict self,
                               bool accept_if_with_regusage) {
	Dee_host_register_t result;
	/* Check if we can find a register that not used anywhere. */
	for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
		if (!Dee_memstate_hregs_isused(self, result) &&
		    self->ms_rusage[result] == DEE_HOST_REGUSAGE_GENERIC &&
		    self->ms_memequiv.meqs_regs[result] == 0)
			return result;
	}
	for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
		if (!Dee_memstate_hregs_isused(self, result) &&
		    self->ms_rusage[result] == DEE_HOST_REGUSAGE_GENERIC)
			return result;
	}
	if (accept_if_with_regusage) {
		/* Check for registers that are only used by regusage. */
		for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
			if (!Dee_memstate_hregs_isused(self, result) &&
			    self->ms_memequiv.meqs_regs[result] == 0)
				return result;
		}
		for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
			if (!Dee_memstate_hregs_isused(self, result))
				break; /* Found an unused register! */
		}
	}
	return result;
}

/* Same as `Dee_memstate_hregs_find_unused(self, true)', but don't return `not_these',
 * which is an array of register numbers terminated by one `>= HOST_REGISTER_COUNT'.
 * Returns some value `>= HOST_REGISTER_COUNT' if non-existent. */
INTERN WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_memstate_hregs_find_unused_ex(struct Dee_memstate *__restrict self,
                                  Dee_host_register_t const *not_these) {
	Dee_host_register_t result;
	BITSET(HOST_REGISTER_COUNT) used;
	bzero(&used, sizeof(used));
	for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
		if (Dee_memstate_hregs_isused(self, result))
			BITSET_TURNON(&used, result);
	}

	/* If specified, exclude certain registers. */
	if (not_these != NULL) {
		size_t i;
		for (i = 0; (result = not_these[i]) < HOST_REGISTER_COUNT; ++i)
			BITSET_TURNON(&used, result);
	}

	/* Even when other usage registers can be re-used, try not to do so unless necessary. */
	for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
		if (!BITSET_GET(&used, result) &&
		    self->ms_rusage[result] == DEE_HOST_REGUSAGE_GENERIC &&
		    self->ms_memequiv.meqs_regs[result] == 0)
			goto done;
	}
	for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
		if (!BITSET_GET(&used, result) &&
		    self->ms_rusage[result] == DEE_HOST_REGUSAGE_GENERIC)
			goto done;
	}
	for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
		if (!BITSET_GET(&used, result) &&
		    self->ms_memequiv.meqs_regs[result] == 0)
			goto done;
	}
	for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
		if (!BITSET_GET(&used, result))
			break; /* Found an unused register! */
	}
done:
	return result;
}

/* Adjust register-related memory locations to account for `%regno = %regno + delta' */
INTERN NONNULL((1)) void DCALL
Dee_memstate_hregs_adjust_delta(struct Dee_memstate *__restrict self,
                                Dee_host_register_t regno, ptrdiff_t delta) {
	struct Dee_memval *val;
	Dee_memstate_foreach(val, self) {
		struct Dee_memobj *obj;
		Dee_memval_foreach_obj(obj, val) {
			if (Dee_memobj_hasreg(obj) && Dee_memobj_getreg(obj) == regno)
				Dee_memobj_setoff(obj, Dee_memobj_getoff(obj) + delta);
		}
		Dee_memval_foreach_obj_end;
	}
	Dee_memstate_foreach_end;
}


/* Check if a pointer-sized blob at `cfa_offset' is being used by something. */
INTERN ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_memstate_hstack_isused(struct Dee_memstate const *__restrict self,
                           Dee_cfa_t cfa_offset) {
	struct Dee_memval const *val;
	Dee_memstate_foreach(val, self) {
		struct Dee_memobj const *obj;
		Dee_memval_foreach_obj(obj, val) {
			if (Dee_memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND &&
			    Dee_memobj_hstackind_getcfa(obj) == cfa_offset)
				return true;
		}
		Dee_memval_foreach_obj_end;
	}
	Dee_memstate_foreach_end;
	return false;
}

#define Dee_memobj_hstack_used(self, start_offset, end_offset) \
	Dee_memadr_hstack_used(Dee_memobj_getadr(self), start_offset, end_offset)
#define Dee_memloc_hstack_used(self, start_offset, end_offset) \
	Dee_memadr_hstack_used(Dee_memloc_getadr(self), start_offset, end_offset)
PRIVATE WUNUSED NONNULL((1)) bool DCALL
Dee_memadr_hstack_used(struct Dee_memadr const *__restrict self,
                       Dee_cfa_t start_offset, Dee_cfa_t end_offset) {
	if (self->ma_typ == MEMADR_TYPE_HSTACKIND) {
		if (RANGES_OVERLAP(Dee_memadr_getcfastart(self),
		                   Dee_memadr_getcfaend(self),
		                   start_offset, end_offset))
			return true;
	}
	return false; /* No used by this one! */
}

#define Dee_memstate_hstack_unused(self, hstack_reserved, min_offset, end_offset) \
	(!Dee_memstate_hstack_used(self, hstack_reserved, min_offset, end_offset))
PRIVATE WUNUSED NONNULL((1)) bool DCALL
Dee_memstate_hstack_used(struct Dee_memstate const *__restrict self,
                         struct Dee_memstate const *hstack_reserved,
                         Dee_cfa_t min_offset, Dee_cfa_t end_offset) {
	struct Dee_memval const *val;
	Dee_memstate_foreach(val, self) {
		struct Dee_memobj const *obj;
		Dee_memval_foreach_obj(obj, val) {
			if (Dee_memobj_hstack_used(obj, min_offset, end_offset))
				return true;
		}
		Dee_memval_foreach_obj_end;
	}
	Dee_memstate_foreach_end;
	if (hstack_reserved != NULL) {
		Dee_memstate_foreach(val, hstack_reserved) {
			struct Dee_memobj const *obj;
			Dee_memval_foreach_obj(obj, val) {
				if (Dee_memobj_hstack_used(obj, min_offset, end_offset))
					return true;
			}
			Dee_memval_foreach_obj_end;
		}
		Dee_memstate_foreach_end;
	}
	return false;
}

/* Try to find a `n_bytes'-large free section of host stack memory.
 * @param: hstack_reserved: When non-NULL, only consider locations that are *also* free in here
 * @return: * :            The base-CFA offset of the free section of memory
 * @return: (Dee_cfa_t)-1: There is no free section of at least `n_bytes' bytes.
 *                         In this case, allocate using `Dee_memstate_hstack_alloca()' */
INTERN ATTR_PURE WUNUSED NONNULL((1)) Dee_cfa_t DCALL
Dee_memstate_hstack_find(struct Dee_memstate const *__restrict self,
                         struct Dee_memstate const *hstack_reserved,
                         size_t n_bytes) {
	ASSERT(IS_ALIGNED(n_bytes, HOST_SIZEOF_POINTER));
#ifdef HOSTASM_X86_64_MSABI
	/* MSABI provides an additional 32 bytes of GP memory at CFA offsets [-40, -8) */
	if (n_bytes <= 4 * HOST_SIZEOF_POINTER) {
		size_t a_pointers = 4;
		size_t n_pointers = n_bytes / HOST_SIZEOF_POINTER;
		size_t i, check = (a_pointers - n_pointers) + 1;
		for (i = 0; i < check; ++i) {
			Dee_cfa_t min_offset = (Dee_cfa_t)(-(ptrdiff_t)((5 * HOST_SIZEOF_POINTER) -
			                                                (i * HOST_SIZEOF_POINTER)));
			Dee_cfa_t end_offset = min_offset + n_bytes;
			if (Dee_memstate_hstack_unused(self, hstack_reserved, min_offset, end_offset)) {
#ifdef HOSTASM_STACK_GROWS_DOWN
				return end_offset;
#else /* HOSTASM_STACK_GROWS_DOWN */
				return min_offset;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			}
		}
	}
#endif /* HOSTASM_X86_64_MSABI */
	if ((Dee_cfa_t)n_bytes <= self->ms_host_cfa_offset) {
		size_t a_pointers = self->ms_host_cfa_offset / HOST_SIZEOF_POINTER;
		size_t n_pointers = n_bytes / HOST_SIZEOF_POINTER;
		size_t i, check = (a_pointers - n_pointers) + 1;
		for (i = 0; i < check; ++i) {
			Dee_cfa_t min_offset = i * HOST_SIZEOF_POINTER;
			Dee_cfa_t end_offset = min_offset + n_bytes;
			if (Dee_memstate_hstack_unused(self, hstack_reserved, min_offset, end_offset)) {
#ifdef HOSTASM_STACK_GROWS_DOWN
				return end_offset;
#else /* HOSTASM_STACK_GROWS_DOWN */
				return min_offset;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			}
		}
	}
	return (Dee_cfa_t)-1;
}

/* Try to free unused stack memory near the top of the stack.
 * @return: true:  The CFA offset was reduced.
 * @return: false: The CFA offset remains the same. */
INTERN NONNULL((1)) bool DCALL
Dee_memstate_hstack_free(struct Dee_memstate *__restrict self) {
	bool result = false;
	while (self->ms_host_cfa_offset > 0) {
		size_t a_pointers = self->ms_host_cfa_offset / HOST_SIZEOF_POINTER;
		Dee_cfa_t min_offset = (Dee_cfa_t)((a_pointers - 1) * HOST_SIZEOF_POINTER);
		Dee_cfa_t end_offset = (Dee_cfa_t)(min_offset + HOST_SIZEOF_POINTER);
		if (!Dee_memstate_hstack_unused(self, NULL, min_offset, end_offset))
			break;
		self->ms_host_cfa_offset -= HOST_SIZEOF_POINTER;
		result = true;
	}
	return false;
}

PRIVATE NONNULL((1, 2, 3)) void DCALL
Dee_memloc_makedistinct(struct Dee_memstate *__restrict self,
                        struct Dee_memloc *loc,
                        ptrdiff_t val_offset) {
	Dee_host_register_t regno;
	Dee_memstate_decrinuse_for_memloc(self, loc);
	regno = Dee_memstate_hregs_find_unused(self, true);
	if (regno < HOST_REGISTER_COUNT) {
		Dee_memloc_init_hreg(loc, regno, val_offset);
		Dee_memstate_incrinuse(self, regno);
		self->ms_rusage[regno] = DEE_HOST_REGUSAGE_GENERIC;
	} else {
		/* Use a stack location. */
		Dee_cfa_t cfa_offset = Dee_memstate_hstack_find(self, NULL, HOST_SIZEOF_POINTER);
		if (cfa_offset == (Dee_cfa_t)-1)
			cfa_offset = Dee_memstate_hstack_alloca(self, HOST_SIZEOF_POINTER);
		Dee_memloc_init_hstackind(loc, cfa_offset, val_offset);
	}
}

PRIVATE NONNULL((1, 2, 3)) bool DCALL
Dee_memloc_makewritable(struct Dee_memstate *__restrict self,
                        struct Dee_memloc *loc) {
	switch (loc->ml_adr.ma_typ) {
	case MEMADR_TYPE_HREG: /* Already writable */
		break;
	case MEMADR_TYPE_HSTACKIND:
		if (loc->ml_adr.ma_val.v_cfa >= 0)
			break; /* Already writable */
		ATTR_FALLTHROUGH
	default:
		Dee_memloc_makedistinct(self, loc, 0);
		return true;
	}
	return false;
}

PRIVATE NONNULL((1, 2, 3, 4)) bool DCALL
Dee_memloc_constrainwith(struct Dee_memstate *__restrict self,
                         struct Dee_memstate const *__restrict other_state,
                         struct Dee_memloc *loc,
                         struct Dee_memloc const *other_loc) {
	bool result = false;
	switch (loc->ml_adr.ma_typ) {

	case MEMADR_TYPE_HREG:
		break;

	case MEMADR_TYPE_HSTACKIND:
#if defined(HOSTASM_X86) && !defined(HOSTASM_X86_64)
		if (loc->ml_adr.ma_val.v_cfa >= 0)
			break; /* Normal stack location */
		ATTR_FALLTHROUGH
#else /* HOSTASM_X86 && !HOSTASM_X86_64 */
		break;
#endif /* !HOSTASM_X86 || HOSTASM_X86_64 */

	default: {
		/* If the location describe by `other' isn't already in use in `state',
		 * then use *it* as-it. That way, we can reduce the necessary number of
		 * memory state transformation! */
		switch (other_loc->ml_adr.ma_typ) {
	
		case MEMADR_TYPE_HSTACKIND:
#if defined(HOSTASM_X86) && !defined(HOSTASM_X86_64)
			if (other_loc->ml_adr.ma_val.v_cfa < 0)
				break; /* Out-of-band location (e.g. true arguments on i386) */
#endif /* HOSTASM_X86 && !HOSTASM_X86_64 */
			if (!Dee_memstate_hstack_isused(self, Dee_memloc_hstackind_getcfa(other_loc))) {
				Dee_cfa_t min_cfa_offset;
				Dee_memstate_decrinuse_for_memloc(self, loc);
				Dee_memloc_init_hstackind(loc,
				                          Dee_memloc_hstackind_getcfa(other_loc),
				                          Dee_memloc_hstackind_getvaloff(other_loc));
				min_cfa_offset = other_loc->ml_adr.ma_val.v_cfa;
#ifndef HOSTASM_STACK_GROWS_DOWN
				min_cfa_offset += HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
				if (self->ms_host_cfa_offset < min_cfa_offset)
					self->ms_host_cfa_offset = min_cfa_offset;
				goto did_runtime_value_merge;
			}
			break;

		case MEMADR_TYPE_HREG:
			if (!Dee_memstate_hregs_isused(self, Dee_memloc_hreg_getreg(other_loc))) {
				Dee_memstate_decrinuse_for_memloc(self, loc);
				Dee_memstate_incrinuse(self, Dee_memloc_hreg_getreg(other_loc));
				*loc = *other_loc;
				goto did_runtime_value_merge;
			}
			break;
	
		default:
			break;
		}
		Dee_memloc_makedistinct(self, loc, Dee_memloc_getoff(other_loc));
did_runtime_value_merge:
		result = true;
	}	break;

	}

	/* Even if both sides have it in-register/on-stack, must still
	 * ensure that all aliases in `self' also appear in `other'.
	 * Any alias that doesn't must become a distinct memory location. */
	{
		unsigned int kind;
		for (kind = 0; kind < 2; ++kind) {
			Dee_lid_t i, valc = self->ms_localc;
			struct Dee_memval *my_valv = self->ms_localv;
			struct Dee_memval const *ot_valv = other_state->ms_localv;
			if (kind != 0) {
				valc = self->ms_stackc;
				my_valv = self->ms_stackv;
				ot_valv = other_state->ms_stackv;
			}
			for (i = 0; i < valc; ++i) {
				size_t obji, my_alias_objc, ot_alias_objc;
				struct Dee_memobj *my_alias_objv;
				struct Dee_memobj const *ot_alias_objv;
				struct Dee_memval *my_alias = &my_valv[i];
				struct Dee_memval const *ot_alias = &ot_valv[i];
				if (my_alias->mv_vmorph != ot_alias->mv_vmorph)
					continue; /* Handled by caller. */
				my_alias_objc = Dee_memval_getobjc(my_alias);
				ot_alias_objc = Dee_memval_getobjc(ot_alias);
				if (my_alias_objc != ot_alias_objc)
					continue; /* Handled by caller. */
				my_alias_objv = Dee_memval_getobjv(my_alias);
				ot_alias_objv = Dee_memval_getobjv(ot_alias);
				for (obji = 0; obji < my_alias_objc; ++obji) {
					struct Dee_memobj *my_alias_obj       = &my_alias_objv[obji];
					struct Dee_memobj const *ot_alias_obj = &ot_alias_objv[obji];
					if (loc == Dee_memobj_getloc(my_alias_obj))
						continue;
					/* If it's an alias in our state, but not in the other, then it must become distinct */
					if (Dee_memloc_sameloc(loc, Dee_memobj_getloc(my_alias_obj)) &&
					    !Dee_memloc_sameloc(other_loc, Dee_memobj_getloc(ot_alias_obj))) {
						Dee_memloc_makedistinct(self,
						                        Dee_memobj_getloc(my_alias_obj),
						                        Dee_memobj_getoff(ot_alias_obj));
						result = true;
					}
				}
			}
		}
	}

	return result;
}

PRIVATE NONNULL((1, 2, 3, 4)) bool DCALL
Dee_memobj_constrainwith(struct Dee_memstate *__restrict self,
                         struct Dee_memstate const *__restrict other_state,
                         struct Dee_memobj *obj,
                         struct Dee_memobj const *other_obj,
                         bool is_local) {
	bool result = Dee_memloc_constrainwith(self, other_state,
	                                       Dee_memobj_getloc(obj),
	                                       Dee_memobj_getloc(other_obj));

	/* If `MEMOBJ_F_ISREF' isn't set in "other_obj", it must also be set in `self' */
	if (!(obj->mo_flags & MEMOBJ_F_ISREF) && (other_obj->mo_flags & MEMOBJ_F_ISREF)) {
		obj->mo_flags |= MEMOBJ_F_ISREF;
		result = true;
	}

	/* Merge `MEMOBJ_F_MAYBEUNBOUND' flags. */
	if ((is_local) &&
	    (obj->mo_flags & MEMOBJ_F_MAYBEUNBOUND) == 0 &&
	    (other_obj->mo_flags & MEMOBJ_F_MAYBEUNBOUND) != 0) {
		obj->mo_flags |= MEMOBJ_F_MAYBEUNBOUND;
		result = true;
	}

	/* Merge extended object information. */
	if (obj->mo_xinfo && obj->mo_xinfo != other_obj->mo_xinfo &&
	    !Dee_memobj_xinfo_equals(Dee_memobj_getxinfo(obj),
	                             Dee_memobj_getxinfo(other_obj))) {
		/* Incompatible extended object information */
		DREF struct Dee_memobj_xinfo *xinfo;
		xinfo = Dee_memobj_getxinfo(obj);
		obj->mo_xinfo = NULL;
		Dee_memobj_xinfo_decref(xinfo);
		result = true;
	}
	return result;
}

PRIVATE NONNULL((1, 2, 3, 4)) bool DCALL
Dee_memval_constrainwith(struct Dee_memstate *__restrict self,
                         struct Dee_memstate const *__restrict other_state,
                         struct Dee_memval *val,
                         struct Dee_memval const *other_val,
                         bool is_local) {
	bool result = false;

	/* If the value differs between the 2 sides, then default to a direct value. */
	if (!Dee_memval_sameval(val, other_val)) {
		if (val->mv_vmorph == other_val->mv_vmorph) {
			size_t i;
			size_t my_objc = Dee_memval_getobjc(val);
			size_t ot_objc = Dee_memval_getobjc(other_val);
			struct Dee_memobj *my_objv;
			struct Dee_memobj const *ot_objv;
			if (my_objc != ot_objc)
				goto incompatible_morph;
			my_objv = Dee_memval_getobjv(val);
			ot_objv = Dee_memval_getobjv(other_val);
			for (i = 0; i < my_objc; ++i) {
				struct Dee_memobj *my_obj = &my_objv[i];
				struct Dee_memobj const *ot_obj = &ot_objv[i];
				result |= Dee_memobj_constrainwith(self, other_state, my_obj, ot_obj, is_local);
			}
		} else {
incompatible_morph:
			if (val->mv_vmorph != MEMVAL_VMORPH_DIRECT) {
				if (!Dee_memval_isdirect(val)) {
					/* Make "val" into a distinct, writable location */
					DeeTypeObject *saved_mo_typeof;
					uint8_t saved_mo_flags;
					saved_mo_typeof = Dee_memval_typeof(val);
					saved_mo_flags  = MEMOBJ_F_NORMAL;
					if (Dee_memval_hasobj0(val))
						saved_mo_flags = Dee_memval_getobj0(val)->mo_flags;
					Dee_memstate_decrinuse_for_memval(self, val);
					Dee_memval_fini(val);
					Dee_memval_init_undefined(val);
					Dee_memloc_makedistinct(self, Dee_memval_direct_getloc(val), 0);
					ASSERT(val->mv_vmorph == MEMVAL_VMORPH_DIRECT);
					ASSERT(Dee_memval_hasobj0(val));
					Dee_memval_getobj0(val)->mo_typeof = saved_mo_typeof;
					Dee_memval_getobj0(val)->mo_flags  = saved_mo_flags;
				}
				val->mv_vmorph = MEMVAL_VMORPH_DIRECT;
				result = true;
			}
			if (Dee_memval_isdirect(other_val)) {
				result |= Dee_memloc_constrainwith(self, other_state,
				                                   Dee_memval_direct_getloc(val),
				                                   Dee_memval_direct_getloc(other_val));
			} else {
				result |= Dee_memloc_makewritable(self, Dee_memval_direct_getloc(val));
			}
		}
		if (Dee_memval_isdirect(val)) {
			/* Merge compile-time known location typing. */
			if (Dee_memval_direct_typeof(val) != NULL) {
				DeeTypeObject *other_type = Dee_memval_typeof(other_val);
				if (Dee_memval_direct_typeof(val) != other_type) {
					/* Location has multiple/unknown object types. */
					Dee_memval_direct_settypeof(val, NULL);
					result = true;
				}
			}
		}
	}

	return result;
}

/* Constrain `self' with `other', such that it is possible to generate code to
 * transition from `other' to `self', as well as any other mem-state that might
 * be the result of further constraints applied to `self'.
 * @return: true:  State become more constrained
 * @return: false: State didn't change */
INTERN NONNULL((1, 2)) bool DCALL
Dee_memstate_constrainwith(struct Dee_memstate *__restrict self,
                           struct Dee_memstate const *__restrict other) {
	size_t i;
	bool result = false;
	Dee_host_register_t regno;
	ASSERT(self->ms_stackc == other->ms_stackc);
	ASSERT(self->ms_localc == other->ms_localc);

	/* Mark usage registers as undefined if different between blocks. */
	for (regno = 0; regno < HOST_REGISTER_COUNT; ++regno) {
		if (self->ms_rusage[regno] != other->ms_rusage[regno]) {
			self->ms_rusage[regno] = DEE_HOST_REGUSAGE_GENERIC;
			result = true;
		}
	}

	/* Merge stack/locals memory locations. */
	for (i = 0; i < self->ms_stackc; ++i)
		result |= Dee_memval_constrainwith(self, other, &self->ms_stackv[i], &other->ms_stackv[i], false);
	for (i = 0; i < self->ms_localc; ++i)
		result |= Dee_memval_constrainwith(self, other, &self->ms_localv[i], &other->ms_localv[i], true);

	/* Combine state flags. */
	if ((self->ms_flags & other->ms_flags) != self->ms_flags) {
		self->ms_flags &= other->ms_flags;
		result = true;
	}

	/* Combine minimum argument count. */
	if (self->ms_uargc_min > other->ms_uargc_min) {
		self->ms_uargc_min = other->ms_uargc_min;
		result = true;
	}

	/* Constrain memory equivalences by deleting all
	 * equivalences that aren't present in both states. */
	result |= Dee_memequivs_constrainwith(&self->ms_memequiv, &other->ms_memequiv);

	return result;
}

/* Check if a reference is being held by `mval' or some other location that may be aliasing it. */
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
Dee_memstate_hasref(struct Dee_memstate const *__restrict self,
                    struct Dee_memval const *mval) {
	struct Dee_memval const *iter;
	if (!Dee_memval_isdirect(mval))
		return false; /* Virtual object */
	if (Dee_memobj_isref(Dee_memval_direct_getobj(mval)))
		return true;
	Dee_memstate_foreach(iter, self) {
		if (!Dee_memval_isdirect(iter))
			continue;
		if (Dee_memval_direct_isref(iter) &&
		    Dee_memval_direct_sameloc(iter, mval))
			return true;
	}
	Dee_memstate_foreach_end;
	return false;
}

/* Check if `mval' has an alias. */
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
Dee_memstate_hasalias(struct Dee_memstate const *__restrict self,
                      struct Dee_memval const *mval) {
	struct Dee_memval const *iter;
	Dee_memstate_foreach(iter, self) {
		if (iter != mval && Dee_memval_sameval(iter, mval))
			return true;
	}
	Dee_memstate_foreach_end;
	return false;
}


PRIVATE NONNULL((1)) void DCALL
Dee_basic_block_clear_hcode_and_exits(struct Dee_basic_block *__restrict self) {
	size_t i;
	for (i = 0; i < self->bb_exits.jds_size; ++i) {
		struct Dee_jump_descriptor *jump;
		jump = self->bb_exits.jds_list[i];
		ASSERT(jump);
		if (jump->jd_stat) {
			Dee_memstate_decref(jump->jd_stat);
			jump->jd_stat = NULL;
		}
	}
	Dee_host_section_clear(&self->bb_htext);
	Dee_host_section_clear(&self->bb_hcold);
}

/* Constrain or assign `self->bb_mem_start' with the memory state `state'
 * @param: self_start_addr: The starting-address of `self' (for error messages)
 * @return: 1 : State become more constrained
 * @return: 0 : State didn't change 
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_basic_block_constrainwith(struct Dee_basic_block *__restrict self,
                              struct Dee_memstate *__restrict state,
                              code_addr_t self_start_addr) {
	bool result;
	struct Dee_memstate *block_start = self->bb_mem_start;
	/* Check for simple case: the block doesn't have a state assigned, yet. */
	if (block_start == NULL) {
		self->bb_mem_start = state;
		Dee_memstate_incref(state);
		ASSERT(self->bb_htext.hs_end == self->bb_htext.hs_start);
		ASSERT(self->bb_htext.hs_relc == 0);
		ASSERT(self->bb_hcold.hs_end == self->bb_hcold.hs_start);
		ASSERT(self->bb_hcold.hs_relc == 0);
		return 0;
	}
	if unlikely(block_start->ms_stackc != state->ms_stackc) {
#ifndef NO_HOSTASM_DEBUG_PRINT
		_Dee_memstate_debug_print(block_start, NULL, NULL);
		_Dee_memstate_debug_print(state, NULL, NULL);
#endif /* !NO_HOSTASM_DEBUG_PRINT */
		DeeError_Throwf(&DeeError_IllegalInstruction,
		                "Unbalanced stack depth at +%.4" PRFx32 ". "
		                "Both %" PRFu16 " and %" PRFu16 " encountered",
		                self_start_addr,
		                block_start->ms_stackc,
		                state->ms_stackc);
		goto err;
	}
	if (Dee_memstate_isshared(block_start)) {
		ASSERT(self->bb_mem_start == block_start);
		block_start = Dee_memstate_copy(block_start);
		if unlikely(!block_start)
			goto err;
		Dee_memstate_decref_nokill(self->bb_mem_start);
		self->bb_mem_start = block_start;
	}
	result = Dee_memstate_constrainwith(block_start, state);
	if (result) {
		if (self->bb_mem_end != NULL) {
			Dee_memstate_decref(self->bb_mem_end);
			self->bb_mem_end = NULL;
		}
		Dee_basic_block_clear_hcode_and_exits(self);
	}
	return result;
err:
	return -1;
}



/* Ensure that at least `min_alloc' stack slots are allocated. */
INTERN NONNULL((1)) int DCALL
Dee_memstate_reqvstack(struct Dee_memstate *__restrict self,
                       Dee_vstackaddr_t min_alloc) {
	ASSERT(self->ms_stackc <= self->ms_stacka);
	if (min_alloc > self->ms_stacka) {
		struct Dee_memval *new_stack;
		new_stack = (struct Dee_memval *)Dee_Reallocc(self->ms_stackv, min_alloc,
		                                              sizeof(struct Dee_memval));
		if unlikely(!new_stack)
			goto err;
		self->ms_stackv = new_stack;
		self->ms_stacka = min_alloc;
	}
	return 0;
err:
	return -1;
}

#ifdef HAVE__Dee_memstate_verifyrinuse_d
INTERN NONNULL((1)) void DCALL
_Dee_memstate_verifyrinuse_d(struct Dee_memstate const *__restrict self) {
	struct Dee_memval const *val;
	size_t correct_rinuse[HOST_REGISTER_COUNT];
	bzero(correct_rinuse, sizeof(correct_rinuse));
	Dee_memstate_foreach(val, self) {
		struct Dee_memobj const *obj;
		Dee_memval_foreach_obj(obj, val) {
			if (Dee_memobj_hasreg(obj)) {
				ASSERT(Dee_memobj_getreg(obj) < HOST_REGISTER_COUNT);
				++correct_rinuse[Dee_memobj_getreg(obj)];
			}
		}
		Dee_memval_foreach_obj_end;
	}
	Dee_memstate_foreach_end;
	ASSERTF(memcmp(self->ms_rinuse, correct_rinuse, sizeof(correct_rinuse)) == 0,
	        "Incorrect register-in-use numbers");
}
#endif /* HAVE__Dee_memstate_verifyrinuse_d */

/* Mark all register equivalences undefined for registers that are not in active use:
 * >> FOREACH REGNO DO
 * >>     IF self->ms_rinuse[REGNO] == 0 THEN
 * >>         Dee_memequivs_undefined_reg(&self->ms_memequiv, REGNO);
 * >>     FI
 * >> DONE */
INTERN NONNULL((1)) void DCALL
Dee_memstate_remember_undefined_unusedregs(struct Dee_memstate *__restrict self) {
	size_t i;
	if (!Dee_memequivs_hasregs(&self->ms_memequiv))
		return; /* Fast-pass: no registers are in use. */
	for (i = 0; i <= self->ms_memequiv.meqs_mask; ++i) {
		struct Dee_memequiv *prev, *next;
		struct Dee_memequiv *eq = &self->ms_memequiv.meqs_list[i];
		if (!MEMEQUIV_TYPE_HASREG(eq->meq_loc.ml_adr.ma_typ))
			continue; /* Not a register location */
		if (self->ms_rinuse[eq->meq_loc.ml_adr.ma_reg] != 0)
			continue; /* Register is in use -> don't mark as undefined */

		/* Remove this equivalence entry. */
		prev = RINGQ_PREV(eq, meq_class);
		next = RINGQ_NEXT(eq, meq_class);
		ASSERT(self->ms_memequiv.meqs_used >= 2);
		_Dee_memequivs_decrinuse(&self->ms_memequiv, eq->meq_loc.ml_adr.ma_reg);
		if (prev == next) {
			/* Removal would leave the class containing only 1 more element
			 * -> get rid of the class entirely! */
			if (MEMEQUIV_TYPE_HASREG(prev->meq_loc.ml_adr.ma_typ))
				_Dee_memequivs_decrinuse(&self->ms_memequiv, prev->meq_loc.ml_adr.ma_reg);
			DBG_memset(prev, 0xcc, sizeof(*prev));
			DBG_memset(eq, 0xcc, sizeof(*eq));
			prev->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
			eq->meq_loc.ml_adr.ma_typ   = MEMEQUIV_TYPE_DUMMY;
			self->ms_memequiv.meqs_used -= 2;
		} else {
			/* Remove entry from the class. */
			RINGQ_REMOVE(eq, meq_class);
			DBG_memset(eq, 0xcc, sizeof(*eq));
			eq->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
			--self->ms_memequiv.meqs_used;
		}
	}
}



INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vswap(struct Dee_memstate *__restrict self) {
	struct Dee_memval temp;
	if unlikely(self->ms_stackc < 2)
		return err_illegal_stack_effect();
	Dee_memval_initmove(&temp, &self->ms_stackv[self->ms_stackc - 2]);
	Dee_memval_initmove(&self->ms_stackv[self->ms_stackc - 2], &self->ms_stackv[self->ms_stackc - 1]);
	Dee_memval_initmove(&self->ms_stackv[self->ms_stackc - 1], &temp);
	return 0;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vlrot(struct Dee_memstate *__restrict self, Dee_vstackaddr_t n) {
	if likely(n > 1) {
		struct Dee_memval temp;
		if unlikely(self->ms_stackc < n)
			return err_illegal_stack_effect();
		Dee_memval_initmove(&temp, &self->ms_stackv[self->ms_stackc - n]);
		Dee_memval_vec_movedown(&self->ms_stackv[self->ms_stackc - n],
		                        &self->ms_stackv[self->ms_stackc - (n - 1)],
		                        n - 1);
		Dee_memval_initmove(&self->ms_stackv[self->ms_stackc - 1], &temp);
	}
	return 0;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vrrot(struct Dee_memstate *__restrict self, Dee_vstackaddr_t n) {
	if likely(n > 1) {
		struct Dee_memval temp;
		if unlikely(self->ms_stackc < n)
			return err_illegal_stack_effect();
		Dee_memval_initmove(&temp, &self->ms_stackv[self->ms_stackc - 1]);
		Dee_memval_vec_moveup(&self->ms_stackv[self->ms_stackc - (n - 1)],
		                      &self->ms_stackv[self->ms_stackc - n],
		                      n - 1);
		Dee_memval_initmove(&self->ms_stackv[self->ms_stackc - n], &temp);
	}
	return 0;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vmirror(struct Dee_memstate *__restrict self, Dee_vstackaddr_t n) {
	Dee_vstackaddr_t i;
	struct Dee_memval *swap_base;
	if unlikely(self->ms_stackc < n)
		return err_illegal_stack_effect();
	swap_base = self->ms_stackv + self->ms_stackc - n;
	for (i = 0; i < (n / 2); ++i) {
		struct Dee_memval temp;
		Dee_vstackaddr_t swap_a_index = i;
		Dee_vstackaddr_t swap_b_index = (n - 1) - i;
		Dee_memval_initmove(&temp, &swap_base[swap_a_index]);
		Dee_memval_initmove(&swap_base[swap_a_index], &swap_base[swap_b_index]);
		Dee_memval_initmove(&swap_base[swap_b_index], &temp);
	}
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_memstate_vpush_memadr(struct Dee_memstate *__restrict self,
                          struct Dee_memadr const *adr) {
	struct Dee_memval *dst;
	struct Dee_memadr temp;
	if unlikely(self->ms_stackc >= self->ms_stacka) {
		temp = *adr; /* In case "adr" was already part of the v-stack. */
		if unlikely(Dee_memstate_reqvstack(self, self->ms_stackc + 1))
			goto err;
		adr = &temp;
	}
	dst = &self->ms_stackv[self->ms_stackc];
	Dee_memval_init_memadr(dst, adr, 0, NULL, MEMOBJ_F_NORMAL);
	Dee_memstate_incrinuse_for_memadr(self, adr);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_memstate_vpush_memloc(struct Dee_memstate *__restrict self,
                          struct Dee_memloc const *loc) {
	struct Dee_memval *dst;
	struct Dee_memloc temp;
	if unlikely(self->ms_stackc >= self->ms_stacka) {
		temp = *loc; /* In case "loc" was already part of the v-stack. */
		if unlikely(Dee_memstate_reqvstack(self, self->ms_stackc + 1))
			goto err;
		loc = &temp;
	}
	dst = &self->ms_stackv[self->ms_stackc];
	Dee_memval_init_memloc(dst, loc, NULL, MEMOBJ_F_NORMAL);
	Dee_memstate_incrinuse_for_memloc(self, loc);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_memstate_vpush_memobj(struct Dee_memstate *__restrict self,
                          struct Dee_memobj const *obj) {
	struct Dee_memval *dst;
	struct Dee_memobj temp;
	if unlikely(self->ms_stackc >= self->ms_stacka) {
		temp = *obj; /* In case "obj" was already part of the v-stack. */
		if unlikely(Dee_memstate_reqvstack(self, self->ms_stackc + 1))
			goto err;
		obj = &temp;
	}
	dst = &self->ms_stackv[self->ms_stackc];
	Dee_memval_init_memobj(dst, obj);
	Dee_memval_direct_clearref(dst); /* Pushed value is an alias, so clear the REF flag. */
	Dee_memstate_incrinuse_for_memobj(self, obj);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vpush_undefined(struct Dee_memstate *__restrict self) {
	struct Dee_memval *dst;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	Dee_memval_init_undefined(dst);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vpush_addr(struct Dee_memstate *__restrict self, void const *addr) {
	struct Dee_memval *dst;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	Dee_memval_init_constaddr(dst, addr);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_memstate_vpush_const(struct Dee_memstate *__restrict self, DeeObject *value) {
	struct Dee_memval *dst;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	Dee_memval_init_constobj(dst, value);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vpush_hreg(struct Dee_memstate *__restrict self,
                        Dee_host_register_t regno, ptrdiff_t val_delta) {
	struct Dee_memval *dst;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	Dee_memval_init_hreg(dst, regno, val_delta, NULL, MEMOBJ_F_NORMAL);
	Dee_memstate_incrinuse(self, regno);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vpush_hregind(struct Dee_memstate *__restrict self,
                           Dee_host_register_t regno,
                           ptrdiff_t ind_delta, ptrdiff_t val_delta) {
	struct Dee_memval *dst;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	Dee_memval_init_hregind(dst, regno, ind_delta, val_delta, NULL, MEMOBJ_F_NORMAL);
	Dee_memstate_incrinuse(self, regno);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vpush_hstack(struct Dee_memstate *__restrict self,
                          Dee_cfa_t cfa_offset) {
	struct Dee_memval *dst;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	Dee_memval_init_hstack(dst, cfa_offset, NULL, MEMOBJ_F_NORMAL);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vpush_hstackind(struct Dee_memstate *__restrict self,
                             Dee_cfa_t cfa_offset, ptrdiff_t val_delta) {
	struct Dee_memval *dst;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	Dee_memval_init_hstackind(dst, cfa_offset, val_delta, NULL, MEMOBJ_F_NORMAL);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_memstate_vdup_n(struct Dee_memstate *__restrict self, Dee_vstackaddr_t n) {
	struct Dee_memval *dst;
	Dee_vstackaddr_t index;
	ASSERT(n >= 1);
	if (OVERFLOW_USUB(self->ms_stackc, n, &index))
		return err_illegal_stack_effect();
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	Dee_memval_initcopy(dst, &self->ms_stackv[index]);
	Dee_memstate_incrinuse_for_memval(self, dst);
	++self->ms_stackc;
	Dee_memval_clearref(dst); /* alias! (so no reference) */
	return 0;
err:
	return -1;
}



/* Compare all of the memory locations and reference counts between "a" and "b" */
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) int DCALL
Dee_except_exitinfo_id_compare(struct Dee_except_exitinfo_id const *__restrict a,
                               struct Dee_except_exitinfo_id const *__restrict b) {
	size_t i;
	if (a->exi_cfa_offset != b->exi_cfa_offset)
		return a->exi_cfa_offset < b->exi_cfa_offset ? -1 : 1;
	if (a->exi_memrefc != b->exi_memrefc)
		return a->exi_memrefc < b->exi_memrefc ? -1 : 1;
	for (i = 0; i < a->exi_memrefc; ++i) {
		int cmp = Dee_memref_compare2(&a->exi_memrefv[i],
		                              &b->exi_memrefv[i]);
		if (cmp != 0)
			return cmp;
	}
	return 0;
}


/* Return the upper bound for the required buffer size in order to represent "state" */
INTERN ATTR_PURE WUNUSED NONNULL((1)) size_t DCALL
Dee_except_exitinfo_id_sizefor(struct Dee_memstate const *__restrict state) {
	size_t result = offsetof(struct Dee_except_exitinfo_id, exi_memrefv);
	struct Dee_memval const *mval;
	Dee_memstate_foreach(mval, state) {
		struct Dee_memobj const *mobj;
		Dee_memval_foreach_obj(mobj, mval) {
			if (mobj->mo_flags & MEMOBJ_F_ISREF)
				result += sizeof(struct Dee_memref);
		}
		Dee_memval_foreach_obj_end;
	}
	Dee_memstate_foreach_end;
	return result;
}


/* Add "ref" to "self". If it is already present, update it; else, insert it. */
PRIVATE NONNULL((1, 2)) void DCALL
Dee_except_exitinfo_id_addref(struct Dee_except_exitinfo_id *__restrict self,
                              struct Dee_memref const *__restrict ref) {
	Dee_vstackaddr_t lo, hi;
	lo = 0;
	hi = self->exi_memrefc;
	while (lo < hi) {
		Dee_vstackaddr_t mid = (lo + hi) / 2;
		struct Dee_memref *it = &self->exi_memrefv[mid];
		int cmp = Dee_memref_compare(ref, it);
		if (cmp < 0) {
			hi = mid;
		} else if (cmp > 0) {
			lo = mid + 1;
		} else {
			/* Combine common location */
			it->mr_refc += ref->mr_refc;
			it->mr_flags |= ref->mr_flags & ~MEMREF_F_NOKILL;
			it->mr_flags &= ref->mr_flags & MEMREF_F_NOKILL;
			return;
		}
	}
	ASSERT(lo == hi);
	memmoveupc(&self->exi_memrefv[lo + 1],
	           &self->exi_memrefv[lo],
	           self->exi_memrefc - lo,
	           sizeof(struct Dee_memref));
	self->exi_memrefv[lo] = *ref;
	++self->exi_memrefc;
}


/* Check if `self' is more "canonical" than `other' */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
Dee_memloc_is_more_canonical_than(struct Dee_memloc const *__restrict self,
                                  struct Dee_memloc const *__restrict other) {
	STATIC_ASSERT(MEMADR_TYPE_CONST == 0);
	STATIC_ASSERT(MEMADR_TYPE_UNDEFINED == 2);
	STATIC_ASSERT(MEMADR_TYPE_HSTACK == 4);
	STATIC_ASSERT(MEMADR_TYPE_HSTACKIND == 5);
	STATIC_ASSERT(MEMADR_TYPE_HREG == 6);
	STATIC_ASSERT(MEMADR_TYPE_HREGIND == 7);

	/* Prefer locations with higher scores here! */
	PRIVATE uint8_t const score_for_type[] = {
		/* [MEMADR_TYPE_CONST]     = */ 4,
		/* [1]                     = */ 0,
		/* [MEMADR_TYPE_UNDEFINED] = */ 6,
		/* [3]                     = */ 0,
		/* [MEMADR_TYPE_HSTACK]    = */ 2,
		/* [MEMADR_TYPE_HSTACKIND] = */ 3,
		/* [MEMADR_TYPE_HREG]      = */ 5,
		/* [MEMADR_TYPE_HREGIND]   = */ 1,
	};

	ASSERT(Dee_memloc_gettyp(self) < COMPILER_LENOF(score_for_type));
	ASSERT(Dee_memloc_gettyp(other) < COMPILER_LENOF(score_for_type));
	ASSERT(score_for_type[Dee_memloc_gettyp(self)] != 0);
	ASSERT(score_for_type[Dee_memloc_gettyp(other)] != 0);
	if (Dee_memloc_gettyp(self) != Dee_memloc_gettyp(other)) {
		return score_for_type[Dee_memloc_gettyp(self)] >
		       score_for_type[Dee_memloc_gettyp(other)];
	}

	/* Prefer locations with small value offsets */
	return Dee_memloc_getoff(self) < Dee_memloc_getoff(other);
}

/* Looking at equivalence classes, fill `*result' with the "canonical" description of `loc'
 * @return: true: if there is a constant equivalence. */
PRIVATE NONNULL((1, 2, 3)) bool DCALL
Dee_memstate_select_canonical_equiv(struct Dee_memstate const *__restrict self,
                                    struct Dee_memloc const *__restrict loc,
                                    struct Dee_memloc *__restrict result) {
	bool isconst;
	struct Dee_memequiv const *eq;
	*result = *loc;
	isconst = Dee_memloc_gettyp(result) == MEMADR_TYPE_CONST;
	eq = Dee_memequivs_getclassof(&self->ms_memequiv, Dee_memloc_getadr(loc));
	if (eq) {
		/* Select the most "canonical" equivalence to "loc" */
		struct Dee_memequiv const *iter;
		ptrdiff_t val_offset;
		val_offset = Dee_memloc_getoff(loc);
		val_offset -= Dee_memloc_getoff(&eq->meq_loc);
		iter = eq;
		do {
			struct Dee_memloc candidate;
			candidate = iter->meq_loc;
			Dee_memloc_adjoff(&candidate, val_offset);
			isconst |= Dee_memloc_gettyp(&candidate) == MEMADR_TYPE_CONST;
			if (Dee_memloc_is_more_canonical_than(&candidate, result))
				*result = candidate;
		} while ((iter = Dee_memequiv_next(iter)) != eq);
	}
	return isconst;
}

/* Initialize `self' from `state'
 * @return: * : Always re-returns `self' */
INTERN NONNULL((1, 2)) struct Dee_except_exitinfo_id *DCALL
Dee_except_exitinfo_id_init(struct Dee_except_exitinfo_id *__restrict self,
                            struct Dee_memstate const *__restrict state) {
	struct Dee_memval const *mval;
	self->exi_cfa_offset = state->ms_host_cfa_offset;
	self->exi_memrefc    = 0;
	Dee_memstate_foreach(mval, state) {
		struct Dee_memobj const *mobj;
		Dee_memval_foreach_obj(mobj, mval) {
			if (mobj->mo_flags & MEMOBJ_F_ISREF) {
				/* Construct the canonical memref from the object. */
				struct Dee_memref ref;
				ref.mr_refc       = 1;
				ref._mr_always0_1 = 0;
				ref._mr_always0_2 = 0;
				ref._mr_always0_3 = 0;
				ref._mr_always0_4 = 0;
				ref.mr_flags      = MEMREF_F_NORMAL;
				if (Dee_memstate_select_canonical_equiv(state, Dee_memobj_getloc(mobj), &ref.mr_loc))
					ref.mr_flags |= MEMREF_F_NOKILL;
#ifdef MEMREF_F_DOKILL
				if (mobj->mo_flags & MEMOBJ_F_ONEREF)
					ref.mr_flags |= MEMREF_F_DOKILL;
#endif /* MEMREF_F_DOKILL */
				if (mobj->mo_flags & MEMOBJ_F_MAYBEUNBOUND)
					ref.mr_flags |= MEMREF_F_NULLABLE;
				if (Dee_memloc_gettyp(&ref.mr_loc) != MEMADR_TYPE_UNDEFINED)
					Dee_except_exitinfo_id_addref(self, &ref);
			}
		}
		Dee_memval_foreach_obj_end;
	}
	Dee_memstate_foreach_end;
	return self;
}


/* Calculate the "distance" score that determines the complexity of the
 * transitioning code needed to morph from `oldinfo' to `newinfo'. When
 * ordering exception cleanup code, exit descriptors should be ordered
 * such that the fallthru of one to the next always yields the lowest
 * distance score.
 * @return: * : The distance scrore for morphing from `oldinfo' to `newinfo' */
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) size_t DCALL
Dee_except_exitinfo_id_distance(struct Dee_except_exitinfo_id const *__restrict oldinfo,
                                struct Dee_except_exitinfo_id const *__restrict newinfo) {
	Dee_vstackaddr_t oldinfo_i, newinfo_i;
	size_t result = 0;
	if (oldinfo->exi_cfa_offset != newinfo->exi_cfa_offset)
		result += 1;
	oldinfo_i = 0;
	newinfo_i = 0;
	while (oldinfo_i < oldinfo->exi_memrefc &&
	       newinfo_i < newinfo->exi_memrefc) {
		struct Dee_memref const *oldref = &oldinfo->exi_memrefv[oldinfo_i];
		struct Dee_memref const *newref = &newinfo->exi_memrefv[newinfo_i];
		int cmp = Dee_memref_compare(oldref, newref);
		if (cmp < 0) {
			result += 4; /* XXX: More points if NULLABLE */
			++oldinfo_i;
		} else if (cmp > 0) {
			result += 4; /* XXX: More points if NULLABLE */
			++newinfo_i;
		} else {
			if (oldref->mr_refc != newref->mr_refc)
				result += 1;
			if ((oldref->mr_flags & MEMREF_F_NULLABLE) && !(newref->mr_flags & MEMREF_F_NULLABLE))
				result += 1;
			++oldinfo_i;
			++newinfo_i;
		}
	}
	if (oldinfo_i < oldinfo->exi_memrefc)
		result += (oldinfo->exi_memrefc - oldinfo_i) * 4; /* XXX: More points if NULLABLE */
	if (newinfo_i < newinfo->exi_memrefc)
		result += (newinfo->exi_memrefc - newinfo_i) * 4; /* XXX: More points if NULLABLE */
	return result;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_MEMSTATE_C */
