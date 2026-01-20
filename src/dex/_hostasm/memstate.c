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

#include <hybrid/bitset.h>
#include <hybrid/overflow.h>
#include <hybrid/typecore.h>
#include <hybrid/unaligned.h>

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, ptrdiff_t, size_t */
#include <stdint.h>  /* UINT16_MAX, uint8_t */

#ifndef UINT16_MAX
#include <hybrid/limitcore.h>
#define UINT16_MAX __UINT16_MAX__
#endif /* !UINT16_MAX */

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#ifdef memval_initmove_IS_MEMCPY
#define memval_vec_movedown(dst, src, num_items) \
	((struct memval *)memmovedownc(dst, src, num_items, sizeof(struct memval)))
#define memval_vec_moveup(dst, src, num_items) \
	((struct memval *)memmoveupc(dst, src, num_items, sizeof(struct memval)))
#else /* memval_initmove_IS_MEMCPY */
PRIVATE ATTR_RETNONNULL ATTR_INS(2, 3) ATTR_OUTS(1, 3) struct memval *DCALL
memval_vec_movedown(struct memval *dst,
                    struct memval const *src,
                    size_t num_items) {
	ASSERT(dst <= src || !num_items);
	if (dst != src) {
		size_t i = num_items;
		while (i) {
			--i;
			memval_initmove(&dst[i], &src[i]);
		}
	}
	return dst;
}

PRIVATE ATTR_RETNONNULL ATTR_INS(2, 3) ATTR_OUTS(1, 3) struct memval *DCALL
memval_vec_moveup(struct memval *dst,
                  struct memval const *src,
                  size_t num_items) {
	ASSERT(dst >= src || !num_items);
	if (dst != src) {
		size_t i;
		for (i = 0; i < num_items; ++i)
			memval_initmove(&dst[i], &src[i]);
	}
	return dst;
}
#endif /* !memval_initmove_IS_MEMCPY */

/* Check if there is a register that contains `usage'.
 * Returns some value `>= HOST_REGNO_COUNT' if non-existent. */
INTERN ATTR_PURE WUNUSED NONNULL((1)) host_regno_t DCALL
memstate_hregs_find_usage(struct memstate const *__restrict self,
                          host_regusage_t usage) {
	host_regno_t result;
	for (result = 0; result < HOST_REGNO_COUNT; ++result) {
		if (self->ms_rusage[result] == usage)
			break;
	}
	return result;
}

/* Check if there is a register that is completely unused.
 * Returns some value `>= HOST_REGNO_COUNT' if non-existent.
 * @param: accept_if_with_regusage: When true, allowed to return registers with
 *                                  `ms_rusage[return] != HOST_REGUSAGE_GENERIC' */
INTERN ATTR_PURE WUNUSED NONNULL((1)) host_regno_t DCALL
memstate_hregs_find_unused(struct memstate const *__restrict self,
                           bool accept_if_with_regusage) {
	host_regno_t result;
	/* Check if we can find a register that not used anywhere. */
	for (result = 0; result < HOST_REGNO_COUNT; ++result) {
		if (!memstate_hregs_isused(self, result) &&
		    self->ms_rusage[result] == HOST_REGUSAGE_GENERIC &&
		    self->ms_memequiv.meqs_regs[result] == 0)
			return result;
	}
	for (result = 0; result < HOST_REGNO_COUNT; ++result) {
		if (!memstate_hregs_isused(self, result) &&
		    self->ms_rusage[result] == HOST_REGUSAGE_GENERIC)
			return result;
	}
	if (accept_if_with_regusage) {
		/* Check for registers that are only used by regusage. */
		for (result = 0; result < HOST_REGNO_COUNT; ++result) {
			if (!memstate_hregs_isused(self, result) &&
			    self->ms_memequiv.meqs_regs[result] == 0)
				return result;
		}
		for (result = 0; result < HOST_REGNO_COUNT; ++result) {
			if (!memstate_hregs_isused(self, result))
				break; /* Found an unused register! */
		}
	}
	return result;
}

/* Same as `memstate_hregs_find_unused(self, true)', but don't return `not_these',
 * which is an array of register numbers terminated by one `>= HOST_REGNO_COUNT'.
 * Returns some value `>= HOST_REGNO_COUNT' if non-existent. */
INTERN WUNUSED NONNULL((1)) host_regno_t DCALL
memstate_hregs_find_unused_ex(struct memstate *__restrict self,
                              host_regno_t const *not_these) {
	host_regno_t result;
	bitset_t used[BITSET_LENGTHOF(HOST_REGNO_COUNT)];
	bitset_clearall(used, HOST_REGNO_COUNT);
	for (result = 0; result < HOST_REGNO_COUNT; ++result) {
		if (memstate_hregs_isused(self, result))
			bitset_set(used, result);
	}

	/* If specified, exclude certain registers. */
	if (not_these != NULL) {
		size_t i;
		for (i = 0; (result = not_these[i]) < HOST_REGNO_COUNT; ++i)
			bitset_set(used, result);
	}

	/* Even when other usage registers can be re-used, try not to do so unless necessary. */
	for (result = 0; result < HOST_REGNO_COUNT; ++result) {
		if (!bitset_test(used, result) &&
		    self->ms_rusage[result] == HOST_REGUSAGE_GENERIC &&
		    self->ms_memequiv.meqs_regs[result] == 0)
			goto done;
	}
	for (result = 0; result < HOST_REGNO_COUNT; ++result) {
		if (!bitset_test(used, result) &&
		    self->ms_rusage[result] == HOST_REGUSAGE_GENERIC)
			goto done;
	}
	for (result = 0; result < HOST_REGNO_COUNT; ++result) {
		if (!bitset_test(used, result) &&
		    self->ms_memequiv.meqs_regs[result] == 0)
			goto done;
	}
	for (result = 0; result < HOST_REGNO_COUNT; ++result) {
		if (!bitset_test(used, result))
			break; /* Found an unused register! */
	}
done:
	return result;
}

/* Adjust register-related memory locations to account for `%regno = %regno + delta' */
INTERN NONNULL((1)) void DCALL
memstate_hregs_adjust_delta(struct memstate *__restrict self,
                            host_regno_t regno, ptrdiff_t delta) {
	struct memval *val;
	memstate_foreach(val, self) {
		struct memobj *obj;
		memval_foreach_obj(obj, val) {
			if (memobj_hasreg(obj) && memobj_getreg(obj) == regno)
				memobj_setoff(obj, memobj_getoff(obj) + delta);
		}
		memval_foreach_obj_end;
	}
	memstate_foreach_end;
}


/* Check if a pointer-sized blob at `cfa_offset' is being used by something. */
INTERN ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
memstate_hstack_isused(struct memstate const *__restrict self,
                       host_cfa_t cfa_offset) {
	struct memval const *val;
	memstate_foreach(val, self) {
		struct memobj const *obj;
		memval_foreach_obj(obj, val) {
			if (memobj_gettyp(obj) == MEMADR_TYPE_HSTACKIND &&
			    memobj_hstackind_getcfa(obj) == cfa_offset)
				return true;
		}
		memval_foreach_obj_end;
	}
	memstate_foreach_end;
	return false;
}


/* Returns the greatest in-use CFA address used by any location.
 * When nothing uses the HSTACK, return "0" instead (only HSTACKIND count). */
INTERN ATTR_PURE WUNUSED NONNULL((1)) host_cfa_t DCALL
memstate_hstack_greatest_inuse(struct memstate const *__restrict self) {
	host_cfa_t result = 0;
	struct memval *mval;
	memstate_foreach(mval, self) {
		struct memobj *mobj;
		memval_foreach_obj(mobj, mval) {
			if (memobj_gettyp(mobj) == MEMADR_TYPE_HSTACKIND) {
				host_cfa_t cfa = memobj_hstackind_getcfa(mobj);
				if (result < cfa)
					result = cfa;
			}
		}
		memval_foreach_obj_end;
	}
	memstate_foreach_end;
	return result;
}


#define memobj_hstack_used(self, start_offset, end_offset) \
	memadr_hstack_used(memobj_getadr(self), start_offset, end_offset)
#define memloc_hstack_used(self, start_offset, end_offset) \
	memadr_hstack_used(memloc_getadr(self), start_offset, end_offset)
PRIVATE WUNUSED NONNULL((1)) bool DCALL
memadr_hstack_used(struct memadr const *__restrict self,
                   host_cfa_t start_offset, host_cfa_t end_offset) {
	if (self->ma_typ == MEMADR_TYPE_HSTACKIND) {
		if (RANGES_OVERLAP(memadr_getcfastart(self),
		                   memadr_getcfaend(self),
		                   start_offset, end_offset))
			return true;
	}
	return false; /* No used by this one! */
}

#define memstate_hstack_unused(self, hstack_reserved, min_offset, end_offset) \
	(!memstate_hstack_used(self, hstack_reserved, min_offset, end_offset))
PRIVATE WUNUSED NONNULL((1)) bool DCALL
memstate_hstack_used(struct memstate const *__restrict self,
                     struct memstate const *hstack_reserved,
                     host_cfa_t min_offset, host_cfa_t end_offset) {
	struct memval const *val;
	memstate_foreach(val, self) {
		struct memobj const *obj;
		memval_foreach_obj(obj, val) {
			if (memobj_hstack_used(obj, min_offset, end_offset))
				return true;
		}
		memval_foreach_obj_end;
	}
	memstate_foreach_end;
	if (hstack_reserved != NULL) {
		memstate_foreach(val, hstack_reserved) {
			struct memobj const *obj;
			memval_foreach_obj(obj, val) {
				if (memobj_hstack_used(obj, min_offset, end_offset))
					return true;
			}
			memval_foreach_obj_end;
		}
		memstate_foreach_end;
	}
	return false;
}

/* Try to find a `n_bytes'-large free section of host stack memory.
 * @param: hstack_reserved: When non-NULL, only consider locations that are *also* free in here
 * @return: * :            The base-CFA offset of the free section of memory
 * @return: (host_cfa_t)-1: There is no free section of at least `n_bytes' bytes.
 *                         In this case, allocate using `memstate_hstack_alloca()' */
INTERN ATTR_PURE WUNUSED NONNULL((1)) host_cfa_t DCALL
memstate_hstack_find(struct memstate const *__restrict self,
                     struct memstate const *hstack_reserved,
                     size_t n_bytes) {
	ASSERT(IS_ALIGNED(n_bytes, HOST_SIZEOF_POINTER));
#ifdef HOSTASM_X86_64_MSABI
	/* MSABI provides an additional 32 bytes of GP memory at CFA offsets [-40, -8) */
	if (n_bytes <= 4 * HOST_SIZEOF_POINTER) {
		size_t a_pointers = 4;
		size_t n_pointers = n_bytes / HOST_SIZEOF_POINTER;
		size_t i, check = (a_pointers - n_pointers) + 1;
		for (i = 0; i < check; ++i) {
			host_cfa_t min_offset = (host_cfa_t)(-(ptrdiff_t)((5 * HOST_SIZEOF_POINTER) -
			                                                (i * HOST_SIZEOF_POINTER)));
			host_cfa_t end_offset = min_offset + n_bytes;
			if (memstate_hstack_unused(self, hstack_reserved, min_offset, end_offset)) {
#ifdef HOSTASM_STACK_GROWS_DOWN
				return end_offset;
#else /* HOSTASM_STACK_GROWS_DOWN */
				return min_offset;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			}
		}
	}
#endif /* HOSTASM_X86_64_MSABI */
	if ((host_cfa_t)n_bytes <= self->ms_host_cfa_offset) {
		size_t a_pointers = self->ms_host_cfa_offset / HOST_SIZEOF_POINTER;
		size_t n_pointers = n_bytes / HOST_SIZEOF_POINTER;
		size_t i, check = (a_pointers - n_pointers) + 1;
		for (i = 0; i < check; ++i) {
			host_cfa_t min_offset = i * HOST_SIZEOF_POINTER;
			host_cfa_t end_offset = min_offset + n_bytes;
			if (memstate_hstack_unused(self, hstack_reserved, min_offset, end_offset)) {
#ifdef HOSTASM_STACK_GROWS_DOWN
				return end_offset;
#else /* HOSTASM_STACK_GROWS_DOWN */
				return min_offset;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			}
		}
	}
	return (host_cfa_t)-1;
}

PRIVATE NONNULL((1, 2, 3)) void DCALL
memloc_makedistinct(struct memstate *__restrict self,
                    struct memloc *loc,
                    ptrdiff_t val_offset) {
	host_regno_t regno;
	memstate_decrinuse_for_memloc(self, loc);
	regno = memstate_hregs_find_unused(self, true);
	if (regno < HOST_REGNO_COUNT) {
		memloc_init_hreg(loc, regno, val_offset);
		memstate_incrinuse(self, regno);
		self->ms_rusage[regno] = HOST_REGUSAGE_GENERIC;
	} else {
		/* Use a stack location. */
		host_cfa_t cfa_offset = memstate_hstack_find(self, NULL, HOST_SIZEOF_POINTER);
		if (cfa_offset == (host_cfa_t)-1)
			cfa_offset = memstate_hstack_alloca(self, HOST_SIZEOF_POINTER);
		memloc_init_hstackind(loc, cfa_offset, val_offset);
	}
}

PRIVATE NONNULL((1, 2, 3)) bool DCALL
memloc_makewritable(struct memstate *__restrict self,
                    struct memloc *loc) {
	switch (loc->ml_adr.ma_typ) {
	case MEMADR_TYPE_HREG: /* Already writable */
		break;
	case MEMADR_TYPE_HSTACKIND:
		if (loc->ml_adr.ma_val.v_cfa >= 0)
			break; /* Already writable */
		ATTR_FALLTHROUGH
	default:
		memloc_makedistinct(self, loc, 0);
		return true;
	}
	return false;
}

PRIVATE NONNULL((1, 2, 3, 4)) bool DCALL
memloc_constrainwith(struct memstate *__restrict self,
                     struct memstate const *__restrict other_state,
                     struct memloc *loc,
                     struct memloc const *other_loc) {
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
			if (!memstate_hstack_isused(self, memloc_hstackind_getcfa(other_loc))) {
				host_cfa_t min_cfa_offset;
				memstate_decrinuse_for_memloc(self, loc);
				memloc_init_hstackind(loc,
				                          memloc_hstackind_getcfa(other_loc),
				                          memloc_hstackind_getvaloff(other_loc));
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
			if (!memstate_hregs_isused(self, memloc_hreg_getreg(other_loc))) {
				memstate_decrinuse_for_memloc(self, loc);
				memstate_incrinuse(self, memloc_hreg_getreg(other_loc));
				*loc = *other_loc;
				goto did_runtime_value_merge;
			}
			break;
	
		default:
			break;
		}
		memloc_makedistinct(self, loc, memloc_getoff(other_loc));
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
			lid_t i, valc = self->ms_localc;
			struct memval *my_valv = self->ms_localv;
			struct memval const *ot_valv = other_state->ms_localv;
			if (kind != 0) {
				valc = self->ms_stackc;
				my_valv = self->ms_stackv;
				ot_valv = other_state->ms_stackv;
			}
			for (i = 0; i < valc; ++i) {
				size_t obji, my_alias_objc, ot_alias_objc;
				struct memobj *my_alias_objv;
				struct memobj const *ot_alias_objv;
				struct memval *my_alias = &my_valv[i];
				struct memval const *ot_alias = &ot_valv[i];
				if (my_alias->mv_vmorph != ot_alias->mv_vmorph)
					continue; /* Handled by caller. */
				my_alias_objc = memval_getobjc(my_alias);
				ot_alias_objc = memval_getobjc(ot_alias);
				if (my_alias_objc != ot_alias_objc)
					continue; /* Handled by caller. */
				my_alias_objv = memval_getobjv(my_alias);
				ot_alias_objv = memval_getobjv(ot_alias);
				for (obji = 0; obji < my_alias_objc; ++obji) {
					struct memobj *my_alias_obj       = &my_alias_objv[obji];
					struct memobj const *ot_alias_obj = &ot_alias_objv[obji];
					if (loc == memobj_getloc(my_alias_obj))
						continue;
					/* If it's an alias in our state, but not in the other, then it must become distinct */
					if (memloc_sameloc(loc, memobj_getloc(my_alias_obj)) &&
					    !memloc_sameloc(other_loc, memobj_getloc(ot_alias_obj))) {
						memloc_makedistinct(self,
						                        memobj_getloc(my_alias_obj),
						                        memobj_getoff(ot_alias_obj));
						result = true;
					}
				}
			}
		}
	}

	return result;
}

PRIVATE NONNULL((1, 2, 3, 4)) bool DCALL
memobj_constrainwith(struct memstate *__restrict self,
                     struct memstate const *__restrict other_state,
                     struct memobj *obj,
                     struct memobj const *other_obj,
                     bool is_local) {
	bool result = memloc_constrainwith(self, other_state,
	                                   memobj_getloc(obj),
	                                   memobj_getloc(other_obj));

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
	if (obj->mo_xinfo && obj->mo_xinfo != other_obj->mo_xinfo) {
		struct memobj_xinfo *cur_xinfo = memobj_getxinfo(obj);
		struct memobj_xinfo *new_xinfo = NULL;
		if (other_obj->mo_xinfo)
			new_xinfo = memobj_getxinfo(other_obj);
		if (new_xinfo &&
		    memobj_xinfo_equals(memobj_getxinfo(obj),
		                        memobj_getxinfo(other_obj))) {
			/* Information is actually compatible! */
		} else {
			/* Incompatible extended object information */
			if (cur_xinfo->mox_cdesc && (!new_xinfo || !new_xinfo->mox_cdesc ||
			                             !memobj_xinfo_cdesc_equals(cur_xinfo->mox_cdesc,
			                                                        new_xinfo->mox_cdesc))) {
				/* Incompatible class descriptor info. */
				result = true;
				Dee_Free(cur_xinfo->mox_cdesc);
				cur_xinfo->mox_cdesc = NULL;
			}
			if (memloc_gettyp(&cur_xinfo->mox_dep) != 0 &&
			    (!new_xinfo || !memloc_sameloc(&cur_xinfo->mox_dep,
			                                   &new_xinfo->mox_dep))) {
				/* Incompatible dependent objects.
				 * In this case, check if "obj" (or one of its aliases)
				 * holds a reference. If not, then we must force it to
				 * become a reference. */
				if (!memstate_hasref(self, obj)) {
					obj->mo_flags |= MEMOBJ_F_ISREF;
					result = true;
				}
				bzero(&cur_xinfo->mox_dep, sizeof(cur_xinfo->mox_dep));
			}

			/* Cleanup: if the xinfo object becomes empty, get rid of it. */
			if (cur_xinfo->mox_cdesc == 0 &&
			    memloc_gettyp(&cur_xinfo->mox_dep) == 0) {
				memobj_xinfo_decref(cur_xinfo);
				obj->mo_xinfo = NULL;
			}
		}
	}
	return result;
}

PRIVATE NONNULL((1, 2, 3, 4)) bool DCALL
memval_constrainwith(struct memstate *__restrict self,
                     struct memstate const *__restrict other_state,
                     struct memval *val,
                     struct memval const *other_val,
                     bool is_local) {
	bool result = false;

	/* If the value differs between the 2 sides, then default to a direct value. */
	if (!memval_sameval(val, other_val)) {
		if (val->mv_vmorph == other_val->mv_vmorph) {
			size_t i;
			size_t my_objc = memval_getobjc(val);
			size_t ot_objc = memval_getobjc(other_val);
			struct memobj *my_objv;
			struct memobj const *ot_objv;
			if (my_objc != ot_objc)
				goto incompatible_morph;
			my_objv = memval_getobjv(val);
			ot_objv = memval_getobjv(other_val);
			for (i = 0; i < my_objc; ++i) {
				struct memobj *my_obj = &my_objv[i];
				struct memobj const *ot_obj = &ot_objv[i];
				result |= memobj_constrainwith(self, other_state, my_obj, ot_obj, is_local);
			}
		} else {
incompatible_morph:
			if (val->mv_vmorph != MEMVAL_VMORPH_DIRECT) {
				if (!memval_isdirect(val)) {
					/* Make "val" into a distinct, writable location */
					DeeTypeObject *saved_mo_typeof;
					uint8_t saved_mo_flags;
					saved_mo_typeof = memval_typeof(val);
					saved_mo_flags  = MEMOBJ_F_NORMAL;
					if (memval_hasobj0(val))
						saved_mo_flags = memval_getobj0(val)->mo_flags;
					memstate_decrinuse_for_memval(self, val);
					memval_fini(val);
					memval_init_undefined(val);
					memloc_makedistinct(self, memval_direct_getloc(val), 0);
					ASSERT(val->mv_vmorph == MEMVAL_VMORPH_DIRECT);
					ASSERT(memval_hasobj0(val));
					memval_getobj0(val)->mo_typeof = saved_mo_typeof;
					memval_getobj0(val)->mo_flags  = saved_mo_flags;
				}
				val->mv_vmorph = MEMVAL_VMORPH_DIRECT;
				result = true;
			}
			if (memval_isdirect(other_val)) {
				result |= memloc_constrainwith(self, other_state,
				                               memval_direct_getloc(val),
				                               memval_direct_getloc(other_val));
			} else {
				result |= memloc_makewritable(self, memval_direct_getloc(val));
			}
		}
		if (memval_isdirect(val)) {
			/* Merge compile-time known location typing. */
			if (memval_direct_typeof(val) != NULL) {
				DeeTypeObject *other_type = memval_typeof(other_val);
				if (memval_direct_typeof(val) != other_type) {
					/* Location has multiple/unknown object types. */
					memval_direct_settypeof(val, NULL);
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
memstate_constrainwith(struct memstate *__restrict self,
                       struct memstate const *__restrict other) {
	size_t i;
	bool result = false;
	host_regno_t regno;
	ASSERT(self->ms_stackc == other->ms_stackc);
	ASSERT(self->ms_localc == other->ms_localc);

	/* Mark usage registers as undefined if different between blocks. */
	for (regno = 0; regno < HOST_REGNO_COUNT; ++regno) {
		if (self->ms_rusage[regno] != other->ms_rusage[regno]) {
			self->ms_rusage[regno] = HOST_REGUSAGE_GENERIC;
			result = true;
		}
	}

	/* Merge stack/locals memory locations. */
	for (i = 0; i < self->ms_stackc; ++i)
		result |= memval_constrainwith(self, other, &self->ms_stackv[i], &other->ms_stackv[i], false);
	for (i = 0; i < self->ms_localc; ++i)
		result |= memval_constrainwith(self, other, &self->ms_localv[i], &other->ms_localv[i], true);

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
	result |= memequivs_constrainwith(&self->ms_memequiv, &other->ms_memequiv);

	return result;
}

/* Check if a reference is being held by `mobj' or some other location that may be aliasing it. */
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
memstate_hasref(struct memstate const *__restrict self,
                struct memobj const *mobj) {
	struct memval const *alias_mval;
	if (memobj_isref(mobj))
		return true;
	memstate_foreach(alias_mval, self) {
		struct memobj const *alias_mobj;
		memval_foreach_obj(alias_mobj, alias_mval) {
			if (memobj_isref(alias_mobj) && memobj_sameloc(alias_mobj, mobj))
				return true;
		}
		memval_foreach_obj_end;
	}
	memstate_foreach_end;
	return false;
}

/* Check if `mval' has an alias. */
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
memstate_hasalias(struct memstate const *__restrict self,
                  struct memval const *mval) {
	struct memval const *iter;
	memstate_foreach(iter, self) {
		if (iter != mval && memval_sameval(iter, mval))
			return true;
	}
	memstate_foreach_end;
	return false;
}


PRIVATE NONNULL((1)) void DCALL
basic_block_clear_hcode_and_exits(struct basic_block *__restrict self) {
	size_t i;
	for (i = 0; i < self->bb_exits.jds_size; ++i) {
		struct jump_descriptor *jump;
		jump = self->bb_exits.jds_list[i];
		ASSERT(jump);
		if (jump->jd_stat) {
			memstate_decref(jump->jd_stat);
			jump->jd_stat = NULL;
		}
	}
	host_section_clear(&self->bb_htext);
}

/* Constrain or assign `self->bb_mem_start' with the memory state `state'
 * @param: self_start_addr: The starting-address of `self' (for error messages)
 * @return: 1 : State become more constrained
 * @return: 0 : State didn't change 
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
basic_block_constrainwith(struct basic_block *__restrict self,
                          struct memstate *__restrict state,
                          code_addr_t self_start_addr) {
	bool result;
	struct memstate *block_start = self->bb_mem_start;
	/* Check for simple case: the block doesn't have a state assigned, yet. */
	if (block_start == NULL) {
		self->bb_mem_start = state;
		memstate_incref(state);
		ASSERT(self->bb_htext.hs_end == self->bb_htext.hs_start);
		ASSERT(self->bb_htext.hs_relc == 0);
		return 0;
	}
	if unlikely(block_start->ms_stackc != state->ms_stackc) {
#ifndef NO_HOSTASM_DEBUG_PRINT
		_memstate_debug_print(block_start, NULL, NULL);
		_memstate_debug_print(state, NULL, NULL);
#endif /* !NO_HOSTASM_DEBUG_PRINT */
		DeeError_Throwf(&DeeError_IllegalInstruction,
		                "Unbalanced stack depth at +%.4" PRFx32 ". "
		                "Both %" PRFu16 " and %" PRFu16 " encountered",
		                self_start_addr,
		                block_start->ms_stackc,
		                state->ms_stackc);
		goto err;
	}
	if (memstate_isshared(block_start)) {
		ASSERT(self->bb_mem_start == block_start);
		block_start = memstate_copy(block_start);
		if unlikely(!block_start)
			goto err;
		memstate_decref_nokill(self->bb_mem_start);
		self->bb_mem_start = block_start;
	}
	result = memstate_constrainwith(block_start, state);
	if (result) {
		if (self->bb_mem_end != NULL) {
			memstate_decref(self->bb_mem_end);
			self->bb_mem_end = NULL;
		}
		basic_block_clear_hcode_and_exits(self);
	}
	return result;
err:
	return -1;
}



/* Ensure that at least `min_alloc' stack slots are allocated. */
INTERN WUNUSED NONNULL((1)) int DCALL
memstate_reqvstack(struct memstate *__restrict self,
                   vstackaddr_t min_alloc) {
	ASSERT(self->ms_stackc <= self->ms_stacka);
	if (min_alloc > self->ms_stacka) {
		struct memval *new_stack;
		new_stack = (struct memval *)Dee_Reallocc(self->ms_stackv, min_alloc,
		                                              sizeof(struct memval));
		if unlikely(!new_stack)
			goto err;
		self->ms_stackv = new_stack;
		self->ms_stacka = min_alloc;
	}
	return 0;
err:
	return -1;
}

#ifdef HAVE__memstate_verifyrinuse_d
INTERN NONNULL((1)) void DCALL
_memstate_verifyrinuse_d(struct memstate const *__restrict self) {
	struct memval const *val;
	size_t correct_rinuse[HOST_REGNO_COUNT];
	bzero(correct_rinuse, sizeof(correct_rinuse));
	memstate_foreach(val, self) {
		struct memobj const *obj;
		memval_foreach_obj(obj, val) {
			if (memobj_hasreg(obj)) {
				ASSERT(memobj_getreg(obj) < HOST_REGNO_COUNT);
				++correct_rinuse[memobj_getreg(obj)];
			}
		}
		memval_foreach_obj_end;
	}
	memstate_foreach_end;
	ASSERTF(memcmp(self->ms_rinuse, correct_rinuse, sizeof(correct_rinuse)) == 0,
	        "Incorrect register-in-use numbers");
}
#endif /* HAVE__memstate_verifyrinuse_d */

/* Mark all register equivalences undefined for registers that are not in active use:
 * >> FOREACH REGNO DO
 * >>     IF self->ms_rinuse[REGNO] == 0 THEN
 * >>         memequivs_undefined_reg(&self->ms_memequiv, REGNO);
 * >>     FI
 * >> DONE */
INTERN NONNULL((1)) void DCALL
memstate_remember_undefined_unusedregs(struct memstate *__restrict self) {
	size_t i;
	if (!memequivs_hasregs(&self->ms_memequiv))
		return; /* Fast-pass: no registers are in use. */
	for (i = 0; i <= self->ms_memequiv.meqs_mask; ++i) {
		struct memequiv *prev, *next;
		struct memequiv *eq = &self->ms_memequiv.meqs_list[i];
		if (!MEMEQUIV_TYPE_HASREG(eq->meq_loc.ml_adr.ma_typ))
			continue; /* Not a register location */
		if (self->ms_rinuse[eq->meq_loc.ml_adr.ma_reg] != 0)
			continue; /* Register is in use -> don't mark as undefined */

		/* Remove this equivalence entry. */
		prev = RINGQ_PREV(eq, meq_class);
		next = RINGQ_NEXT(eq, meq_class);
		ASSERT(self->ms_memequiv.meqs_used >= 2);
		_memequivs_decrinuse(&self->ms_memequiv, eq->meq_loc.ml_adr.ma_reg);
		if (prev == next) {
			/* Removal would leave the class containing only 1 more element
			 * -> get rid of the class entirely! */
			if (MEMEQUIV_TYPE_HASREG(prev->meq_loc.ml_adr.ma_typ))
				_memequivs_decrinuse(&self->ms_memequiv, prev->meq_loc.ml_adr.ma_reg);
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


/* Remember that "this_object" depends on "depends_on_this". That means that when
 * "depends_on_this" (or one of its aliases) gets decref'd such that it *might*
 * get destroyed, it must *first* ensure that "this_object" (or one of its aliases)
 * is holding a reference.
 * Note that any object can only ever have at most 1 dependency (so if "this_object"
 * already has a dependency, that dependency gets overwritten by this function). */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
memstate_dependency(struct memstate *__restrict self,
                    struct memobj *__restrict this_object,
                    struct memobj *__restrict depends_on_this) {
	struct memval *alias_mval;
	struct memobj_xinfo *xinfo;
	ASSERTF(this_object != depends_on_this, "You can't depend on yourself");
	ASSERTF(!memobj_sameloc(this_object, depends_on_this), "You can't depend on yourself");
	xinfo = memstate_reqxinfo(self, this_object);
	if unlikely(!xinfo)
		goto err;
	xinfo->mox_dep = *memobj_getloc(depends_on_this);

	/* Must also set the "MEMOBJ_F_HASDEP" in all aliases. */
	if (!(depends_on_this->mo_flags & MEMOBJ_F_HASDEP)) {
		depends_on_this->mo_flags |= MEMOBJ_F_HASDEP;
		memstate_foreach(alias_mval, self) {
			struct memobj *alias_mobj;
			memval_foreach_obj(alias_mobj, alias_mval) {
				if (memobj_sameloc(alias_mobj, depends_on_this) &&
					alias_mobj != depends_on_this)
					alias_mobj->mo_flags |= MEMOBJ_F_HASDEP;
			}
			memval_foreach_obj_end;
		}
		memstate_foreach_end;
	}
	return 0;
err:
	return -1;
}

/* Same as `memobj_reqxinfo()', but must be used when "obj" may be aliased by
 * other memory locations, in which case the returned struct will be allocated in
 * all aliases as well. */
INTERN WUNUSED NONNULL((1, 2)) struct memobj_xinfo *DCALL
memstate_reqxinfo(struct memstate *__restrict self,
                  struct memobj *__restrict obj) {
	struct memobj_xinfo *result;
	if (memobj_hasxinfo(obj))
		return memobj_getxinfo(obj);
	result = memobj_reqxinfo(obj);
	if likely(result) {
		struct memval *alias_mval;
again_assign_aliases:
		memstate_foreach(alias_mval, self) {
			struct memobj *alias_mobj;
			memval_foreach_obj(alias_mobj, alias_mval) {
				if (memobj_sameloc(alias_mobj, obj) /*&& alias_mobj != obj*/) {
					if unlikely(memobj_hasxinfo(alias_mobj)) {
						if (memobj_getxinfo(alias_mobj) != result) {
							/* This might happen due to lazy aliasing
							 * -> fix it by using the existing value instead. */
							memobj_xinfo_destroy(result);
							result = memobj_getxinfo(alias_mobj);
							obj->mo_xinfo = (byte_t *)result + MEMOBJ_MO_XINFO_OFFSET;
							memobj_xinfo_incref(result);
							goto again_assign_aliases;
						}
					} else {
						alias_mobj->mo_xinfo = (byte_t *)result + MEMOBJ_MO_XINFO_OFFSET;
						memobj_xinfo_incref(result);
					}
				}
			}
			memval_foreach_obj_end;
		}
		memstate_foreach_end;
	}
	return result;
}

/* Change all reference to "from" to instead refer to "to" */
INTERN NONNULL((1, 2, 3)) void DCALL
memstate_changeloc(struct memstate *__restrict self,
                   struct memloc const *from,
                   struct memloc const *to) {
	struct memval *mval;
	struct memloc _from = *from;
	size_t n_changed = 0;

	/* Update all aliases. */
	memstate_foreach(mval, self) {
		struct memobj *mobj;
		memval_foreach_obj(mobj, mval) {
			if (memloc_sameadr(memobj_getloc(mobj), &_from)) {
				mobj->mo_loc.ml_adr = to->ml_adr;
				mobj->mo_loc.ml_off -= _from.ml_off;
				mobj->mo_loc.ml_off += to->ml_off;
				++n_changed;
			}
			if (memobj_hasxinfo(mobj)) {
				struct memobj_xinfo *xinfo;
				xinfo = memobj_getxinfo(mobj);
				if (memloc_sameadr(&xinfo->mox_dep, &_from)) {
					mobj->mo_loc.ml_adr = to->ml_adr;
					mobj->mo_loc.ml_off -= _from.ml_off;
					mobj->mo_loc.ml_off += to->ml_off;
#if 0 /* Doesn't count towards register usage! */
					++n_changed;
#endif
				}
			}
		}
		memval_foreach_obj_end;
	}
	memstate_foreach_end;

	/* Update register usage. */
	if (memloc_hasreg(&_from))
		self->ms_rinuse[memloc_getreg(&_from)] -= n_changed;
	if (memloc_hasreg(to))
		self->ms_rinuse[memloc_getreg(to)] += n_changed;
}


INTERN WUNUSED NONNULL((1)) int DCALL
memstate_vswap(struct memstate *__restrict self) {
	struct memval temp;
	if unlikely(self->ms_stackc < 2)
		return err_illegal_stack_effect();
	memval_initmove(&temp, &self->ms_stackv[self->ms_stackc - 2]);
	memval_initmove(&self->ms_stackv[self->ms_stackc - 2], &self->ms_stackv[self->ms_stackc - 1]);
	memval_initmove(&self->ms_stackv[self->ms_stackc - 1], &temp);
	return 0;
}

INTERN WUNUSED NONNULL((1)) int DCALL
memstate_vlrot(struct memstate *__restrict self, vstackaddr_t n) {
	if likely(n > 1) {
		struct memval temp;
		if unlikely(self->ms_stackc < n)
			return err_illegal_stack_effect();
		memval_initmove(&temp, &self->ms_stackv[self->ms_stackc - n]);
		memval_vec_movedown(&self->ms_stackv[self->ms_stackc - n],
		                    &self->ms_stackv[self->ms_stackc - (n - 1)],
		                    n - 1);
		memval_initmove(&self->ms_stackv[self->ms_stackc - 1], &temp);
	}
	return 0;
}

INTERN WUNUSED NONNULL((1)) int DCALL
memstate_vrrot(struct memstate *__restrict self, vstackaddr_t n) {
	if likely(n > 1) {
		struct memval temp;
		if unlikely(self->ms_stackc < n)
			return err_illegal_stack_effect();
		memval_initmove(&temp, &self->ms_stackv[self->ms_stackc - 1]);
		memval_vec_moveup(&self->ms_stackv[self->ms_stackc - (n - 1)],
		                  &self->ms_stackv[self->ms_stackc - n],
		                  n - 1);
		memval_initmove(&self->ms_stackv[self->ms_stackc - n], &temp);
	}
	return 0;
}

INTERN WUNUSED NONNULL((1)) int DCALL
memstate_vmirror(struct memstate *__restrict self, vstackaddr_t n) {
	vstackaddr_t i;
	struct memval *swap_base;
	if unlikely(self->ms_stackc < n)
		return err_illegal_stack_effect();
	swap_base = self->ms_stackv + self->ms_stackc - n;
	for (i = 0; i < (n / 2); ++i) {
		struct memval temp;
		vstackaddr_t swap_a_index = i;
		vstackaddr_t swap_b_index = (n - 1) - i;
		memval_initmove(&temp, &swap_base[swap_a_index]);
		memval_initmove(&swap_base[swap_a_index], &swap_base[swap_b_index]);
		memval_initmove(&swap_base[swap_b_index], &temp);
	}
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
memstate_vpush_memadr(struct memstate *__restrict self,
                      struct memadr const *adr) {
	struct memval *dst;
	struct memadr temp;
	if unlikely(self->ms_stackc >= self->ms_stacka) {
		temp = *adr; /* In case "adr" was already part of the v-stack. */
		if unlikely(memstate_reqvstack(self, self->ms_stackc + 1))
			goto err;
		adr = &temp;
	}
	dst = &self->ms_stackv[self->ms_stackc];
	memval_init_memadr(dst, adr, 0, NULL, MEMOBJ_F_NORMAL);
	memstate_incrinuse_for_memadr(self, adr);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
memstate_vpush_memloc(struct memstate *__restrict self,
                      struct memloc const *loc) {
	struct memval *dst;
	struct memloc temp;
	if unlikely(self->ms_stackc >= self->ms_stacka) {
		temp = *loc; /* In case "loc" was already part of the v-stack. */
		if unlikely(memstate_reqvstack(self, self->ms_stackc + 1))
			goto err;
		loc = &temp;
	}
	dst = &self->ms_stackv[self->ms_stackc];
	memval_init_memloc(dst, loc, NULL, MEMOBJ_F_NORMAL);
	memstate_incrinuse_for_memloc(self, loc);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
memstate_vpush_memobj(struct memstate *__restrict self,
                      struct memobj const *obj) {
	struct memval *dst;
	struct memobj temp;
	if unlikely(self->ms_stackc >= self->ms_stacka) {
		temp = *obj; /* In case "obj" was already part of the v-stack. */
		if unlikely(memstate_reqvstack(self, self->ms_stackc + 1))
			goto err;
		obj = &temp;
	}
	dst = &self->ms_stackv[self->ms_stackc];
	memval_init_memobj(dst, obj);
	memval_direct_clearref(dst); /* Pushed value is an alias, so clear the REF flag. */
	memstate_incrinuse_for_memobj(self, obj);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
memstate_vpush_undefined(struct memstate *__restrict self) {
	struct memval *dst;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	memval_init_undefined(dst);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
memstate_vpush_addr(struct memstate *__restrict self, void const *addr) {
	struct memval *dst;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	memval_init_constaddr(dst, addr);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
memstate_vpush_const(struct memstate *__restrict self, DeeObject *value) {
	struct memval *dst;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	memval_init_constobj(dst, value);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
memstate_vpush_hreg(struct memstate *__restrict self,
                    host_regno_t regno, ptrdiff_t val_delta) {
	struct memval *dst;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	memval_init_hreg(dst, regno, val_delta, NULL, MEMOBJ_F_NORMAL);
	memstate_incrinuse(self, regno);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
memstate_vpush_hregind(struct memstate *__restrict self,
                       host_regno_t regno,
                       ptrdiff_t ind_delta, ptrdiff_t val_delta) {
	struct memval *dst;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	memval_init_hregind(dst, regno, ind_delta, val_delta, NULL, MEMOBJ_F_NORMAL);
	memstate_incrinuse(self, regno);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
memstate_vpush_hstack(struct memstate *__restrict self,
                      host_cfa_t cfa_offset) {
	struct memval *dst;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	memval_init_hstack(dst, cfa_offset, NULL, MEMOBJ_F_NORMAL);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
memstate_vpush_hstackind(struct memstate *__restrict self,
                         host_cfa_t cfa_offset, ptrdiff_t val_delta) {
	struct memval *dst;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	memval_init_hstackind(dst, cfa_offset, val_delta, NULL, MEMOBJ_F_NORMAL);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
memstate_vdup_at(struct memstate *__restrict self, vstackaddr_t n) {
	struct memval *dst;
	vstackaddr_t index;
	ASSERT(n >= 1);
	if (OVERFLOW_USUB(self->ms_stackc, n, &index))
		return err_illegal_stack_effect();
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst = &self->ms_stackv[self->ms_stackc];
	memval_initcopy(dst, &self->ms_stackv[index]);
	memstate_incrinuse_for_memval(self, dst);
	++self->ms_stackc;
	memval_clearref(dst); /* alias! (so no reference) */
	return 0;
err:
	return -1;
}



/* Compare all of the memory locations and reference counts between "a" and "b" */
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) int DCALL
except_exitinfo_id_compare(struct except_exitinfo_id const *__restrict a,
                           struct except_exitinfo_id const *__restrict b) {
	size_t i;
	if (a->exi_cfa_offset != b->exi_cfa_offset)
		return a->exi_cfa_offset < b->exi_cfa_offset ? -1 : 1;
	if (a->exi_memrefc != b->exi_memrefc)
		return a->exi_memrefc < b->exi_memrefc ? -1 : 1;
	for (i = 0; i < a->exi_memrefc; ++i) {
		int cmp = memref_compare2(&a->exi_memrefv[i],
		                              &b->exi_memrefv[i]);
		if (cmp != 0)
			return cmp;
	}
	return 0;
}


/* Return the upper bound for the required buffer size in order to represent "state" */
INTERN ATTR_PURE WUNUSED NONNULL((1)) size_t DCALL
except_exitinfo_id_sizefor(struct memstate const *__restrict state) {
	size_t result = offsetof(struct except_exitinfo_id, exi_memrefv);
	struct memval const *mval;
	memstate_foreach(mval, state) {
		struct memobj const *mobj;
		memval_foreach_obj(mobj, mval) {
			if (mobj->mo_flags & MEMOBJ_F_ISREF)
				result += sizeof(struct memref);
		}
		memval_foreach_obj_end;
	}
	memstate_foreach_end;
	return result;
}


/* Add "ref" to "self". If it is already present, update it; else, insert it. */
PRIVATE NONNULL((1, 2)) void DCALL
except_exitinfo_id_addref(struct except_exitinfo_id *__restrict self,
                          struct memref const *__restrict ref) {
	vstackaddr_t lo, hi;
	lo = 0;
	hi = self->exi_memrefc;
	while (lo < hi) {
		vstackaddr_t mid = (lo + hi) / 2;
		struct memref *it = &self->exi_memrefv[mid];
		int cmp = memref_compare(ref, it);
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
	           sizeof(struct memref));
	self->exi_memrefv[lo] = *ref;
	++self->exi_memrefc;
}


/* Check if `self' is more "canonical" than `other' */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
memloc_is_more_canonical_than(struct memloc const *__restrict self,
                              struct memloc const *__restrict other) {
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

	ASSERT(memloc_gettyp(self) < COMPILER_LENOF(score_for_type));
	ASSERT(memloc_gettyp(other) < COMPILER_LENOF(score_for_type));
	ASSERT(score_for_type[memloc_gettyp(self)] != 0);
	ASSERT(score_for_type[memloc_gettyp(other)] != 0);
	if (memloc_gettyp(self) != memloc_gettyp(other)) {
		return score_for_type[memloc_gettyp(self)] >
		       score_for_type[memloc_gettyp(other)];
	}

	/* Prefer locations with small value offsets */
	return memloc_getoff(self) < memloc_getoff(other);
}

/* Looking at equivalence classes, fill `*result' with the "canonical" description of `loc'
 * @return: true: if there is a constant equivalence. */
PRIVATE NONNULL((1, 2, 3)) bool DCALL
memstate_select_canonical_equiv(struct memstate const *__restrict self,
                                struct memloc const *__restrict loc,
                                struct memloc *__restrict result) {
	bool isconst;
	struct memequiv const *eq;
	*result = *loc;
	isconst = memloc_gettyp(result) == MEMADR_TYPE_CONST;
	eq = memequivs_getclassof(&self->ms_memequiv, memloc_getadr(loc));
	if (eq) {
		/* Select the most "canonical" equivalence to "loc" */
		struct memequiv const *iter;
		ptrdiff_t val_offset;
		val_offset = memloc_getoff(loc);
		val_offset -= memloc_getoff(&eq->meq_loc);
		iter = eq;
		do {
			struct memloc candidate;
			candidate = iter->meq_loc;
			memloc_adjoff(&candidate, val_offset);
			isconst |= memloc_gettyp(&candidate) == MEMADR_TYPE_CONST;
			if (memloc_is_more_canonical_than(&candidate, result))
				*result = candidate;
		} while ((iter = memequiv_next(iter)) != eq);
	}
	return isconst;
}

/* Initialize `self' from `state'
 * @return: * : Always re-returns `self' */
INTERN NONNULL((1, 2)) struct except_exitinfo_id *DCALL
except_exitinfo_id_init(struct except_exitinfo_id *__restrict self,
                        struct memstate const *__restrict state) {
	struct memval const *mval;
	self->exi_cfa_offset = state->ms_host_cfa_offset;
	self->exi_memrefc    = 0;
	memstate_foreach(mval, state) {
		struct memobj const *mobj;
		memval_foreach_obj(mobj, mval) {
			if (mobj->mo_flags & MEMOBJ_F_ISREF) {
				/* Construct the canonical memref from the object. */
				struct memref ref;
				ref.mr_refc       = 1;
				ref._mr_always0_1 = 0;
				ref._mr_always0_2 = 0;
				ref._mr_always0_3 = 0;
				ref._mr_always0_4 = 0;
				ref.mr_flags      = MEMREF_F_NORMAL;
				if (memstate_select_canonical_equiv(state, memobj_getloc(mobj), &ref.mr_loc))
					ref.mr_flags |= MEMREF_F_NOKILL;
#ifdef MEMREF_F_DOKILL
				if (mobj->mo_flags & MEMOBJ_F_ONEREF)
					ref.mr_flags |= MEMREF_F_DOKILL;
#endif /* MEMREF_F_DOKILL */
				if (mobj->mo_flags & MEMOBJ_F_MAYBEUNBOUND)
					ref.mr_flags |= MEMREF_F_NULLABLE;
				if (memloc_gettyp(&ref.mr_loc) != MEMADR_TYPE_UNDEFINED)
					except_exitinfo_id_addref(self, &ref);
			}
		}
		memval_foreach_obj_end;
	}
	memstate_foreach_end;
	return self;
}


/* Calculate the "distance" score that determines the complexity of the
 * transitioning code needed to morph from `oldinfo' to `newinfo'. When
 * ordering exception cleanup code, exit descriptors should be ordered
 * such that the fallthru of one to the next always yields the lowest
 * distance score.
 * @return: * : The distance scrore for morphing from `oldinfo' to `newinfo' */
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) size_t DCALL
except_exitinfo_id_distance(struct except_exitinfo_id const *__restrict oldinfo,
                            struct except_exitinfo_id const *__restrict newinfo) {
	vstackaddr_t oldinfo_i, newinfo_i;
	size_t result = 0;
	if (oldinfo->exi_cfa_offset != newinfo->exi_cfa_offset)
		result += 1;
	oldinfo_i = 0;
	newinfo_i = 0;
	while (oldinfo_i < oldinfo->exi_memrefc &&
	       newinfo_i < newinfo->exi_memrefc) {
		struct memref const *oldref = &oldinfo->exi_memrefv[oldinfo_i];
		struct memref const *newref = &newinfo->exi_memrefv[newinfo_i];
		int cmp = memref_compare(oldref, newref);
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
