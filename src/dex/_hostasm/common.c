/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_HOSTASM_COMMON_C
#define GUARD_DEX_HOSTASM_COMMON_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/alloc.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>

#include <hybrid/align.h>

DECL_BEGIN

#ifndef CHAR_BIT
#include <hybrid/typecore.h>
#define CHAR_BIT __CHAR_BIT__
#endif /* !CHAR_BIT */

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */


/************************************************************************/
/* memequivs                                                        */
/************************************************************************/

STATIC_ASSERT(!MEMEQUIV_TYPE_SUPPORTED(MEMADR_TYPE_UNDEFINED));
STATIC_ASSERT(!MEMEQUIV_TYPE_SUPPORTED(MEMADR_TYPE_HSTACK));
STATIC_ASSERT(MEMEQUIV_TYPE_SUPPORTED(MEMADR_TYPE_HREG));
STATIC_ASSERT(MEMEQUIV_TYPE_SUPPORTED(MEMADR_TYPE_HREGIND));
STATIC_ASSERT(MEMEQUIV_TYPE_SUPPORTED(MEMADR_TYPE_HSTACKIND));
STATIC_ASSERT(MEMEQUIV_TYPE_SUPPORTED(MEMADR_TYPE_CONST));
STATIC_ASSERT(!MEMEQUIV_TYPE_SUPPORTED(MEMEQUIV_TYPE_UNUSED));
STATIC_ASSERT(!MEMEQUIV_TYPE_SUPPORTED(MEMEQUIV_TYPE_DUMMY));


INTERN struct memequiv const memequivs_dummy_list[1] = {
	/* [0] = */ {
		/* .meq_loc = */ {
			/* .ml_adr = */ {
				/* .ma_typ  = */ MEMEQUIV_TYPE_UNUSED,
				/* .ma_reg  = */ 0,
				/* ._ma_zro = */ { 0, },
				/* .ma_val  = */ { 0 },
			},
			/* .ml_off = */ 0,
		},
		/* .meq_class = */ RINGQ_ENTRY_UNBOUND_INITIALIZER,
	}
};

#ifdef HAVE__memequivs_verifyrinuse_d
INTERN NONNULL((1)) void DCALL
_memequivs_verifyrinuse_d(struct memequivs const *__restrict self) {
	size_t i, correct_rinuse[HOST_REGNO_COUNT];
	bzero(correct_rinuse, sizeof(correct_rinuse));
	for (i = 0; i <= self->meqs_mask; ++i) {
		struct memequiv const *eq = &self->meqs_list[i];
		switch (eq->meq_loc.ml_adr.ma_typ) {
		case MEMEQUIV_TYPE_HREG:
		case MEMEQUIV_TYPE_HREGIND:
			ASSERT(eq->meq_loc.ml_adr.ma_reg < HOST_REGNO_COUNT);
			++correct_rinuse[eq->meq_loc.ml_adr.ma_reg];
			break;
		case MEMEQUIV_TYPE_HSTACKIND:
		case MEMEQUIV_TYPE_CONST:
			ASSERT(eq->meq_loc.ml_adr.ma_reg == 0);
			break;
		default: break;
		}
	}
	ASSERTF(memcmp(self->meqs_regs, correct_rinuse, sizeof(correct_rinuse)) == 0,
	        "Incorrect register-in-use numbers");
}
#endif /* HAVE__memequivs_verifyrinuse_d */


/* Inplace-replace `self->meqs_list' with a copy of itself. */
INTERN WUNUSED NONNULL((1)) int DCALL
_memequivs_inplace_copy(struct memequivs *__restrict self) {
	size_t i, mask = self->meqs_mask;
	intptr_t delta;
	struct memequiv *newmap;
	newmap = (struct memequiv *)Dee_Mallocc(mask + 1, sizeof(struct memequiv));
	if unlikely(!newmap)
		goto err;

	/* Copy over the map 1-to-1. */
	newmap = (struct memequiv *)memcpyc(newmap, self->meqs_list, mask + 1,
	                                        sizeof(struct memequiv));

	/* Adjust ring pointer deltas to point into the map copy. */
	delta = (intptr_t)((byte_t *)newmap - (byte_t *)self->meqs_list);
	for (i = 0; i <= mask; ++i) {
		struct memequiv *it = &newmap[i];
		it->meq_class.rqe_prev = (struct memequiv *)((byte_t *)it->meq_class.rqe_prev + delta);
		it->meq_class.rqe_next = (struct memequiv *)((byte_t *)it->meq_class.rqe_next + delta);
	}
	self->meqs_list = newmap;
	_memequivs_verifyrinuse(self);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) void DCALL
memequiv_make_undefined(struct memequivs *__restrict self,
                        struct memequiv *__restrict eq) {
	struct memequiv *prev = RINGQ_PREV(eq, meq_class);
	struct memequiv *next = RINGQ_NEXT(eq, meq_class);
	ASSERT(self->meqs_used >= 2);
	if (MEMEQUIV_TYPE_HASREG(eq->meq_loc.ml_adr.ma_typ))
		_memequivs_decrinuse(self, eq->meq_loc.ml_adr.ma_reg);
	if (prev == next) {
		/* Removal would leave the class containing only 1 more element
		 * -> get rid of the class entirely! */
		if (MEMEQUIV_TYPE_HASREG(prev->meq_loc.ml_adr.ma_typ))
			_memequivs_decrinuse(self, prev->meq_loc.ml_adr.ma_reg);
		DBG_memset(prev, 0xcc, sizeof(*prev));
		DBG_memset(eq, 0xcc, sizeof(*eq));
		prev->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
		eq->meq_loc.ml_adr.ma_typ   = MEMEQUIV_TYPE_DUMMY;
		self->meqs_used -= 2;
	} else {
		/* Remove entry from the class. */
		RINGQ_REMOVE(eq, meq_class);
		DBG_memset(eq, 0xcc, sizeof(*eq));
		eq->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
		--self->meqs_used;
	}
}

/* Check if "ring" contains an element that is *identical*
 * to "item" (including having the same value-offset) */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
memequiv_ring_contains_identical(struct memequiv const *__restrict ring,
                                 struct memloc const *__restrict item) {
	struct memequiv const *iter = ring;
	do {
		if (memloc_sameadr(&iter->meq_loc, item)) {
			return memloc_getoff(&iter->meq_loc) ==
			       memloc_getoff(item);
		}
	} while ((iter = memequiv_next(iter)) != ring);
	return false;
}

/* Constrain equivalences in `self' by deleting all that aren't also present in `other'
 * @return: true:  At least 1 equivalence had to be deleted.
 * @return: false: Everything is good! */
INTERN NONNULL((1, 2)) bool DCALL
memequivs_constrainwith(struct memequivs *__restrict self,
                        struct memequivs const *__restrict other) {
	size_t i;
	bool result = false;
	_memequivs_verifyrinuse(self);
#if !defined(NO_HOSTASM_DEBUG_PRINT) && 0
	HA_printf("memequivs_constrainwith:self:");
	_memequivs_debug_print(self);
	HA_printf("memequivs_constrainwith:other:");
	_memequivs_debug_print(other);
#endif /* !NO_HOSTASM_DEBUG_PRINT */

	for (i = 0; i <= self->meqs_mask; ++i) {
		struct memequiv *eq = &self->meqs_list[i];
		struct memequiv *eq_other, *eq_iter, *eq_next;
		if (eq->meq_loc.ml_adr.ma_typ == MEMEQUIV_TYPE_UNUSED)
			continue;
		if (eq->meq_loc.ml_adr.ma_typ == MEMEQUIV_TYPE_DUMMY)
			continue;
		eq_other = memequivs_getclassof(other, &eq->meq_loc.ml_adr);
		if (!eq_other) {
			/* Equivalence doesn't exist in "other" -> delete it */
			memequiv_make_undefined(self, eq);
			result = true;
			continue;
		}

		/* If value deltas between the 2 equivalences don't match, then
		 * adjust the delta of all equivalence items in "self" such that
		 * the delta of "eq" matches "other".
		 *
		 * Example:
		 *    self:  A + 1 <=> B + 2 <=> C + 2
		 *    other: A + 5 <=> B + 6 <=> C + 7
		 * The value offsets differ, but the equivalences being described
		 * are compatible! */
		if (memloc_getoff(&eq->meq_loc) != memloc_getoff(&eq_other->meq_loc)) {
			struct memequiv *iter = eq;
			ptrdiff_t delta = memloc_getoff(&eq_other->meq_loc) -
			                  memloc_getoff(&eq->meq_loc);
			do {
				memloc_adjoff(&iter->meq_loc, delta);
			} while ((iter = memequiv_next(iter)) != eq);
		}

		/* Compare the equivalence rings of "eq" and "eq_other", and
		 * remove all locations from "eq" that don't appear *exactly*
		 * the same way in "eq_other" (including having to have equal
		 * value offsets) */
		ASSERT(memequiv_ring_contains_identical(eq_other, &eq->meq_loc));
		eq_iter = memequiv_next(eq);
		ASSERT(eq_iter != eq);
		do {
			eq_next = memequiv_next(eq_iter);
			if (!memequiv_ring_contains_identical(eq_other, &eq_iter->meq_loc)) {
				/* Must remove "eq_iter" from "self". */
				if (MEMEQUIV_TYPE_HASREG(eq_iter->meq_loc.ml_adr.ma_typ))
					_memequivs_decrinuse(self, eq_iter->meq_loc.ml_adr.ma_reg);
				ASSERT(RINGQ_PREV(eq_iter, meq_class) != eq_iter);
				ASSERT(RINGQ_NEXT(eq_iter, meq_class) != eq_iter);

				/* XXX: This is doing more than needs to happen:
				 * >> a: { %eax <=> %ecx <=> %edx <=> %ebx }
				 * >> b: { %eax <=> %ecx, %edx <=> %ebx }
				 *
				 * Currently, this results either:
				 * >> result = { %eax <=> %ecx }
				 * >> result = { %edx <=> %ebx }
				 * based on which location is encountered first,
				 * when the optimal result would be:
				 * >> result = { %eax <=> %ecx, %edx <=> %ebx }
				 */
				RINGQ_REMOVE(eq_iter, meq_class);
				DBG_memset(eq_iter, 0xcc, sizeof(*eq_iter));
				eq_iter->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
				--self->meqs_used;
				result = true;
			}
		} while ((eq_iter = eq_next) != eq);

		/* Check for special case: "eq" was the only equivalence present in "other"
		 * -> In this case, we must delete "eq" as well because an equivalence class
		 *    must always contain at least 2 items! */
		if (memequiv_next(eq) == eq) {
			ASSERT(result);
			ASSERT(RINGQ_PREV(eq, meq_class) == eq);
			ASSERT(RINGQ_NEXT(eq, meq_class) == eq);
			if (MEMEQUIV_TYPE_HASREG(eq->meq_loc.ml_adr.ma_typ))
				_memequivs_decrinuse(self, eq->meq_loc.ml_adr.ma_reg);
			DBG_memset(eq, 0xcc, sizeof(*eq));
			eq->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
			--self->meqs_used;
		}
	}
	_memequivs_verifyrinuse(self);
	return result;
}


PRIVATE ATTR_RETNONNULL NONNULL((1, 3)) struct memequiv *DCALL
memequiv_find_insert_dst(struct memequiv *__restrict map, size_t mask,
                         struct memadr const *item) {
	uintptr_t hash, perturb, i;
	hash    = memadr_hashof(item);
	perturb = i = hash & mask;
	for (;; memequivs_hashnx(i, perturb)) {
		struct memequiv *dst = &map[i & mask];
		ASSERT(dst->meq_loc.ml_adr.ma_typ != MEMEQUIV_TYPE_DUMMY);
		if (dst->meq_loc.ml_adr.ma_typ == MEMEQUIV_TYPE_UNUSED)
			return dst;
	}
}

PRIVATE NONNULL((1, 3)) void DCALL
memequiv_rehash(struct memequiv *__restrict oldmap, size_t oldmask,
                struct memequiv *__restrict newmap, size_t newmask) {
	size_t i;
	for (i = 0; i <= newmask; ++i)
		newmap[i].meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_UNUSED;
	for (i = 0; i <= oldmask; ++i) {
		struct memequiv *dst_next, *dst_iter, *dst;
		struct memequiv *src_next, *src_iter, *src = &oldmap[i];
		if (src->meq_loc.ml_adr.ma_typ == MEMEQUIV_TYPE_UNUSED ||
		    src->meq_loc.ml_adr.ma_typ == MEMEQUIV_TYPE_DUMMY)
			continue;
		ASSERT(RINGQ_PREV(src, meq_class) != src);
		ASSERT(RINGQ_NEXT(src, meq_class) != src);
		/* Transfer whole classes at-a-time, so we can easily migrate ring pointers. */
		src_iter = RINGQ_NEXT(src, meq_class);
		dst = memequiv_find_insert_dst(newmap, newmask, &src->meq_loc.ml_adr);
		*dst = *src;
		src->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
		dst_iter = dst;
		do {
			src_next = RINGQ_NEXT(src_iter, meq_class);
			dst_next = memequiv_find_insert_dst(newmap, newmask, &src_iter->meq_loc.ml_adr);
			ASSERT(src_iter->meq_loc.ml_adr.ma_typ != MEMEQUIV_TYPE_DUMMY);
			*dst_next = *src_iter;
			src_iter->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
			RINGQ_NEXT(dst_iter, meq_class) = dst_next;
			RINGQ_PREV(dst_next, meq_class) = dst_iter;
			dst_iter = dst_next;
		} while ((src_iter = src_next) != src);
		RINGQ_NEXT(dst_next, meq_class) = dst;
		RINGQ_PREV(dst, meq_class) = dst_next;
	}
}

/* Search for "eqadr", or create an entry for it if none exists, yet. */
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) struct memequiv *DCALL
memequivs_find_or_insert(struct memequivs *__restrict self,
                         struct memadr const *__restrict eqadr,
                         bool remove_from_old_class_if_already_present) {
	struct memequiv *newslot = NULL;
	uintptr_t hash, perturb, i;
	hash    = memadr_hashof(eqadr);
	perturb = i = memequivs_hashst(self, hash);
	for (;; memequivs_hashnx(i, perturb)) {
		struct memequiv *result = memequivs_hashit(self, i);
		if (memadr_sameadr(&result->meq_loc.ml_adr, eqadr)) {
			if (remove_from_old_class_if_already_present) {
				struct memequiv *prev = RINGQ_PREV(result, meq_class);
				struct memequiv *next = RINGQ_NEXT(result, meq_class);
				ASSERT(self->meqs_used >= 2);
				if (prev == next) {
					/* Removal would leave the class containing only 1 more element
					 * -> get rid of the class entirely! */
					if (MEMEQUIV_TYPE_HASREG(prev->meq_loc.ml_adr.ma_typ))
						_memequivs_decrinuse(self, prev->meq_loc.ml_adr.ma_reg);
					DBG_memset(prev, 0xcc, sizeof(*prev));
					prev->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
					self->meqs_used -= 1;
				} else {
					/* Remove entry from the class, turning it into its own class. */
					RINGQ_REMOVE(result, meq_class);
				}
				RINGQ_INIT(result, meq_class);
			}
			return result;
		}
		if (result->meq_loc.ml_adr.ma_typ == MEMEQUIV_TYPE_UNUSED) {
			if (newslot == NULL) {
				++self->meqs_size;
				newslot = result;
			}
			break;
		}
		if (newslot == NULL && result->meq_loc.ml_adr.ma_typ == MEMEQUIV_TYPE_DUMMY)
			newslot = result;
	}

	/* Location isn't being tracked, yet -> create it now. */
	memloc_init_memadr(&newslot->meq_loc, eqadr, 0);
	RINGQ_INIT(newslot, meq_class); /* New item is part of a 1-element ring (by default) */
	++self->meqs_used;
	if (MEMEQUIV_TYPE_HASREG(eqadr->ma_typ))
		_memequivs_incrinuse(self, eqadr->ma_reg);
	return newslot;
}

PRIVATE NONNULL((1)) void DCALL
memequivs_undefined_hregind_for_hreg(struct memequivs *__restrict self,
                                     host_regno_t regno) {
	size_t i;
	ASSERT(regno < HOST_REGNO_COUNT);
	if (self->meqs_regs[regno] == 0)
		return; /* Nothing uses this register -> nothing to do! */
	for (i = 0; i <= self->meqs_mask; ++i) {
		struct memequiv *prev, *next;
		struct memequiv *eq = &self->meqs_list[i];
		if (eq->meq_loc.ml_adr.ma_typ != MEMEQUIV_TYPE_HREGIND)
			continue;
		if (eq->meq_loc.ml_adr.ma_reg != regno)
			continue;

		/* Remove this equivalence entry. */
		prev = RINGQ_PREV(eq, meq_class);
		next = RINGQ_NEXT(eq, meq_class);
		ASSERT(self->meqs_used >= 2);
		_memequivs_decrinuse(self, regno);
		if (prev == next) {
			/* Removal would leave the class containing only 1 more element
			 * -> get rid of the class entirely! */
			if (MEMEQUIV_TYPE_HASREG(prev->meq_loc.ml_adr.ma_typ))
				_memequivs_decrinuse(self, prev->meq_loc.ml_adr.ma_reg);
			DBG_memset(prev, 0xcc, sizeof(*prev));
			DBG_memset(eq, 0xcc, sizeof(*eq));
			prev->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
			eq->meq_loc.ml_adr.ma_typ   = MEMEQUIV_TYPE_DUMMY;
			self->meqs_used -= 2;
		} else {
			/* Remove entry from the class. */
			RINGQ_REMOVE(eq, meq_class);
			DBG_memset(eq, 0xcc, sizeof(*eq));
			eq->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
			--self->meqs_used;
		}
	}
}


/* Remember that "to" now contains the same value as "from".
 * In the even that "to" was already part of another equivalence
 * class, it will first be removed from that class the same way
 * a call to `memequivs_undefined(self, to)' would.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
memequivs_movevalue(struct memequivs *__restrict self,
                    struct memloc const *__restrict from,
                    struct memloc const *__restrict to) {
	struct memequiv *from_eq, *to_eq;
	_memequivs_verifyrinuse(self);
	if (!MEMEQUIV_TYPE_SUPPORTED(from->ml_adr.ma_typ))
		return 0;
	if (!MEMEQUIV_TYPE_SUPPORTED(to->ml_adr.ma_typ))
		return 0;
	ASSERTF(to->ml_adr.ma_typ != MEMEQUIV_TYPE_CONST,
	        "Bad usage: can't move *into* a constant");
#if !defined(NO_HOSTASM_DEBUG_PRINT) && 0
	HA_printf("memequivs_movevalue(");
	_memloc_debug_print(from);
	Dee_DPRINT(" => ");
	_memloc_debug_print(to);
	Dee_DPRINT(")\n");
	_memequivs_debug_print(self);
#endif /* !NO_HOSTASM_DEBUG_PRINT */

	/* Check for special case: "from" and "to" are the same memory locations.
	 * In this case, see if there's a difference in value delta between the
	 * two, and if so: remember that value shift. */
	if (memloc_sameadr(from, to)) {
		ptrdiff_t from_delta   = memloc_getoff(from);
		ptrdiff_t to_delta     = memloc_getoff(to);
		ptrdiff_t change_delta = to_delta - from_delta;
		/* >> ... <==> LOC + X;
		 * >> LOC + to_delta := LOC + from_delta;
		 * >> ... <==> LOC + X - from_delta + to_delta; */
		if (change_delta != 0)
			memequivs_deltavalue(self, &to->ml_adr, change_delta);
		return 0;
	}

	/* Make sure there is enough space in the hash-vector. */
	ASSERT(self->meqs_used <= self->meqs_size);
	if (self->meqs_mask <= ((self->meqs_size + 3) * 2)) { /* +3 for +2 (max new entries), +1 (mandatory sentinel) */
		size_t new_mask = 1;
		struct memequiv *new_list;
		while (new_mask <= ((self->meqs_used + 3) * 2))
			new_mask = (new_mask << 1) | 1;
		if (new_mask < 7)
			new_mask = 7;
		new_list = (struct memequiv *)Dee_TryMallocc(new_mask + 1, sizeof(struct memequiv));
		if unlikely(!new_list) {
			new_mask = 1;
			while (new_mask <= self->meqs_used + 3)
				new_mask = (new_mask << 1) | 1;
			if (new_mask == self->meqs_mask)
				goto vector_is_ready;
			new_list = (struct memequiv *)Dee_Mallocc(new_mask + 1, sizeof(struct memequiv));
			if unlikely(!new_list)
				goto err;
		}
		memequiv_rehash(self->meqs_list, self->meqs_mask,
		                    new_list, new_mask);
		if (self->meqs_list != (struct memequiv *)memequivs_dummy_list)
			Dee_Free(self->meqs_list);
		self->meqs_list = new_list;
		self->meqs_mask = new_mask;
		self->meqs_used = self->meqs_size; /* Because dummy items were removed. */
	}
vector_is_ready:

	/* NOTE: Order here is important in case "to" was already linked to "from",
	 *       in which case the lookup of "to" might end up making the (old) link
	 *       of "from" undefined. */
	to_eq   = memequivs_find_or_insert(self, &to->ml_adr, true);
	from_eq = memequivs_find_or_insert(self, &from->ml_adr, false);

	/* Calculate the correct value-offset-delta for "to_eq" */
	ASSERTF(to_eq->meq_loc.ml_adr.ma_typ == MEMADR_TYPE_HSTACKIND ||
	        to_eq->meq_loc.ml_adr.ma_typ == MEMADR_TYPE_HREG ||
	        to_eq->meq_loc.ml_adr.ma_typ == MEMADR_TYPE_HREGIND,
	        "This must be the case for us to directly modify `to_eq->meq_loc.ml_off'");
	to_eq->meq_loc.ml_off = memloc_getoff(to);
	to_eq->meq_loc.ml_off -= memloc_getoff(from);
	to_eq->meq_loc.ml_off += memloc_getoff(&from_eq->meq_loc);

	/* Append `to_eq' onto the equivalence class of `from_eq' */
	RINGQ_INSERT_AFTER(from_eq, to_eq, meq_class);

	if (to->ml_adr.ma_typ == MEMADR_TYPE_HREG) {
		/* Special case: when a register value becomes undefined, any knowledge
		 * about stuff that might be located at indirect locations addressable
		 * from that register also become undefined. */
		memequivs_undefined_hregind_for_hreg(self, to->ml_adr.ma_reg);
	}
	_memequivs_verifyrinuse(self);
	return 0;
err:
	_memequivs_verifyrinuse(self);
	return -1;
}

/* Remember that a value change happened: "loc = loc + delta" */
INTERN NONNULL((1, 2)) void DCALL
memequivs_deltavalue(struct memequivs *__restrict self,
                     struct memadr const *__restrict loc,
                     ptrdiff_t delta) {
	struct memequiv *eq;
	ASSERTF(loc->ma_typ != MEMADR_TYPE_CONST,
	        "How can you \"move\" the value of a constant? "
	        "That doesn't make any sense!");

	/* Update the equivalence relation (if there is one) */
	eq = memequivs_getclassof(self, loc);
	if (eq != NULL)
		eq->meq_loc.ml_off += delta;
	_memequivs_verifyrinuse(self);
}

/* Check if "self" might use register locations. */
INTERN ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
memequivs_hasregs(struct memequivs const *__restrict self) {
	host_regno_t regno;
	for (regno = 0; regno < HOST_REGNO_COUNT; ++regno) {
		if (self->meqs_regs[regno] != 0)
			return true;
	}
	return false;
}

/* Remember that "loc" contains an undefined value (remove
 * from its equivalence class, should that class still exist). */
INTERN NONNULL((1, 2)) void DCALL
memequivs_undefined(struct memequivs *__restrict self,
                    struct memadr const *__restrict loc) {
	struct memequiv *eq;
#if !defined(NO_HOSTASM_DEBUG_PRINT) && 0
	struct memequiv_loc _dbgloc;
	if likely(memloc_fromloc(&_dbgloc, loc)) {
		HA_printf("memequivs_undefined(");
		_memloc_debug_print(&_dbgloc, 0);
		Dee_DPRINT(")\n");
		_memequivs_debug_print(self);
	}
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	_memequivs_verifyrinuse(self);
	eq = memequivs_getclassof(self, loc);
	if (eq != NULL) {
		struct memequiv *prev = RINGQ_PREV(eq, meq_class);
		struct memequiv *next = RINGQ_NEXT(eq, meq_class);
		ASSERT(self->meqs_used >= 2);
		if (MEMEQUIV_TYPE_HASREG(eq->meq_loc.ml_adr.ma_typ))
			_memequivs_decrinuse(self, eq->meq_loc.ml_adr.ma_reg);
		if (prev == next) {
			/* Removal would leave the class containing only 1 more element
			 * -> get rid of the class entirely! */
			if (MEMEQUIV_TYPE_HASREG(prev->meq_loc.ml_adr.ma_typ))
				_memequivs_decrinuse(self, prev->meq_loc.ml_adr.ma_reg);
			DBG_memset(prev, 0xcc, sizeof(*prev));
			DBG_memset(eq, 0xcc, sizeof(*eq));
			prev->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
			eq->meq_loc.ml_adr.ma_typ   = MEMEQUIV_TYPE_DUMMY;
			self->meqs_used -= 2;
		} else {
			/* Remove entry from the class. */
			RINGQ_REMOVE(eq, meq_class);
			DBG_memset(eq, 0xcc, sizeof(*eq));
			eq->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
			--self->meqs_used;
		}
	}
	if (loc->ma_typ == MEMADR_TYPE_HREG) {
		/* Special case: when a register value becomes undefined, any knowledge
		 * about stuff that might be located at indirect locations addressable
		 * from that register also become undefined. */
		memequivs_undefined_hregind_for_hreg(self, loc->ma_reg);
	}
	_memequivs_verifyrinuse(self);
}

/* Mark all HREG and HREGIND locations as undefined. */
INTERN NONNULL((1)) void DCALL
memequivs_undefined_allregs(struct memequivs *__restrict self) {
	size_t i;
#if !defined(NO_HOSTASM_DEBUG_PRINT) && 0
	HA_printf("memequivs_undefined_allregs()\n");
	_memequivs_debug_print(self);
#endif /* !NO_HOSTASM_DEBUG_PRINT */
	_memequivs_verifyrinuse(self);
	if (!memequivs_hasregs(self))
		return; /* Fast-pass: no registers are in use. */
	for (i = 0; i <= self->meqs_mask; ++i) {
		struct memequiv *prev, *next;
		struct memequiv *eq = &self->meqs_list[i];
		if (!MEMEQUIV_TYPE_HASREG(eq->meq_loc.ml_adr.ma_typ))
			continue;

		/* Remove this equivalence entry. */
		prev = RINGQ_PREV(eq, meq_class);
		next = RINGQ_NEXT(eq, meq_class);
		ASSERT(self->meqs_used >= 2);
		_memequivs_decrinuse(self, eq->meq_loc.ml_adr.ma_reg);
		if (prev == next) {
			/* Removal would leave the class containing only 1 more element
			 * -> get rid of the class entirely! */
			if (MEMEQUIV_TYPE_HASREG(prev->meq_loc.ml_adr.ma_typ))
				_memequivs_decrinuse(self, prev->meq_loc.ml_adr.ma_reg);
			DBG_memset(prev, 0xcc, sizeof(*prev));
			DBG_memset(eq, 0xcc, sizeof(*eq));
			prev->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
			eq->meq_loc.ml_adr.ma_typ   = MEMEQUIV_TYPE_DUMMY;
			self->meqs_used -= 2;
		} else {
			/* Remove entry from the class. */
			RINGQ_REMOVE(eq, meq_class);
			DBG_memset(eq, 0xcc, sizeof(*eq));
			eq->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
			--self->meqs_used;
		}
	}
	_memequivs_verifyrinuse(self);
}

/* Mark all HSTACKIND locations with CFA offsets `>= min_cfa_offset' as undefined. */
INTERN NONNULL((1)) void DCALL
memequivs_undefined_hstackind_after(struct memequivs *__restrict self,
                                    host_cfa_t min_cfa_offset) {
	size_t i;
	_memequivs_verifyrinuse(self);
	for (i = 0; i <= self->meqs_mask; ++i) {
		struct memequiv *prev, *next;
		struct memequiv *eq = &self->meqs_list[i];
		if (eq->meq_loc.ml_adr.ma_typ != MEMEQUIV_TYPE_HSTACKIND)
			continue;
		if (eq->meq_loc.ml_adr.ma_val.v_cfa < min_cfa_offset)
			continue;

		/* Remove this equivalence entry. */
		prev = RINGQ_PREV(eq, meq_class);
		next = RINGQ_NEXT(eq, meq_class);
		ASSERT(self->meqs_used >= 2);
		if (prev == next) {
			/* Removal would leave the class containing only 1 more element
			 * -> get rid of the class entirely! */
			if (MEMEQUIV_TYPE_HASREG(prev->meq_loc.ml_adr.ma_typ))
				_memequivs_decrinuse(self, prev->meq_loc.ml_adr.ma_reg);
			DBG_memset(prev, 0xcc, sizeof(*prev));
			DBG_memset(eq, 0xcc, sizeof(*eq));
			prev->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
			eq->meq_loc.ml_adr.ma_typ   = MEMEQUIV_TYPE_DUMMY;
			self->meqs_used -= 2;
		} else {
			/* Remove entry from the class. */
			RINGQ_REMOVE(eq, meq_class);
			DBG_memset(eq, 0xcc, sizeof(*eq));
			eq->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
			--self->meqs_used;
		}
	}
	_memequivs_verifyrinuse(self);
}

/* Mark all HSTACKIND locations where [memequiv_getcfastart()...memequiv_getcfaend())
 * overlaps with [start_cfa_offset, end_cfa_offset) as undefined. */
INTERN NONNULL((1)) void DCALL
memequivs_undefined_hstackind_inrange(struct memequivs *__restrict self,
                                      host_cfa_t start_cfa_offset,
                                      host_cfa_t end_cfa_offset) {
	size_t i;
	_memequivs_verifyrinuse(self);
	for (i = 0; i <= self->meqs_mask; ++i) {
		struct memequiv *prev, *next;
		struct memequiv *eq = &self->meqs_list[i];
		if (eq->meq_loc.ml_adr.ma_typ != MEMEQUIV_TYPE_HSTACKIND)
			continue;
		if (!(memequiv_getcfaend(eq) > start_cfa_offset &&
		      memequiv_getcfastart(eq) < end_cfa_offset))
			continue; /* Not affected */

		/* Remove this equivalence entry. */
		prev = RINGQ_PREV(eq, meq_class);
		next = RINGQ_NEXT(eq, meq_class);
		ASSERT(self->meqs_used >= 2);
		if (prev == next) {
			/* Removal would leave the class containing only 1 more element
			 * -> get rid of the class entirely! */
			if (MEMEQUIV_TYPE_HASREG(prev->meq_loc.ml_adr.ma_typ))
				_memequivs_decrinuse(self, prev->meq_loc.ml_adr.ma_reg);
			DBG_memset(prev, 0xcc, sizeof(*prev));
			DBG_memset(eq, 0xcc, sizeof(*eq));
			prev->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
			eq->meq_loc.ml_adr.ma_typ   = MEMEQUIV_TYPE_DUMMY;
			self->meqs_used -= 2;
		} else {
			/* Remove entry from the class. */
			RINGQ_REMOVE(eq, meq_class);
			DBG_memset(eq, 0xcc, sizeof(*eq));
			eq->meq_loc.ml_adr.ma_typ = MEMEQUIV_TYPE_DUMMY;
			--self->meqs_used;
		}
	}
	_memequivs_verifyrinuse(self);
}




/* Return a pointer to the equivalence location of "loc" (ignoring
 * value offsets), or `NULL' if there aren't any additional locations
 * that are known to be equivalent to "loc".
 *
 * Equivalent locations can be enumerated via the `meq_class' ring.
 * NOTE: This function ignores the value-delta of "loc" */
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) struct memequiv *DCALL
memequivs_getclassof(struct memequivs const *__restrict self,
                     struct memadr const *__restrict loc) {
#if 0 /* Not needed (also checked by `memadr_sameadr()') */
	if likely(MEMEQUIV_TYPE_SUPPORTED(loc->ma_typ))
#endif
	{
		uintptr_t hash, perturb, i;
		hash    = memadr_hashof(loc);
		perturb = i = memequivs_hashst(self, hash);
		for (;; memequivs_hashnx(i, perturb)) {
			struct memequiv *result = memequivs_hashit(self, i);
			if (result->meq_loc.ml_adr.ma_typ == MEMEQUIV_TYPE_UNUSED)
				break;
			if (memadr_sameadr(&result->meq_loc.ml_adr, loc))
				return result;
		}
	}
	return NULL;
}



/************************************************************************/
/* memval                                                           */
/************************************************************************/

INTERN ATTR_PURE NONNULL((1, 2)) bool DCALL
memobj_xinfo_cdesc_equals(struct memobj_xinfo_cdesc const *a,
                          struct memobj_xinfo_cdesc const *b) {
	if (a->moxc_desc != b->moxc_desc)
		goto nope;
	if (memcmp(a->moxc_init, b->moxc_init,
	           CEILDIV(a->moxc_desc->cd_cmemb_size, CHAR_BIT)) != 0)
		goto nope;
	return true;
nope:
	return false;
}

INTERN NONNULL((1)) void DCALL
memobj_xinfo_destroy(struct memobj_xinfo *__restrict self) {
	Dee_Free(self->mox_cdesc);
	Dee_Free(self);
}

INTERN ATTR_PURE NONNULL((1, 2)) bool DCALL
memobj_xinfo_equals(struct memobj_xinfo const *a,
                    struct memobj_xinfo const *b) {
	if (a == b)
		return true;
	if (a->mox_cdesc != b->mox_cdesc) {
		if (!a->mox_cdesc || !b->mox_cdesc)
			goto nope;
		if (!memobj_xinfo_cdesc_equals(a->mox_cdesc, b->mox_cdesc))
			goto nope;
	}
	return memloc_sameloc(&a->mox_dep, &b->mox_dep);
nope:
	return false;
}

/* Ensure that `self->mo_xinfo' has been allocated, then return it.
 * @return: NULL: Extended object info had yet to be allocated, and allocation failed. */
INTERN WUNUSED NONNULL((1)) struct memobj_xinfo *DCALL
memobj_reqxinfo(struct memobj *__restrict self) {
	if (!memobj_hasxinfo(self)) {
		struct memobj_xinfo *result;
		result = (struct memobj_xinfo *)Dee_Calloc(sizeof(struct memobj_xinfo));
		if unlikely(!result)
			goto err;
		result->mox_refcnt = 1;
		self->mo_xinfo = (byte_t *)result + MEMOBJ_MO_XINFO_OFFSET;
	}
	return memobj_getxinfo(self);
err:
	return NULL;
}


/* Construct a new `struct memobjs' with an uninitialized `mos_objv'. */
INTERN WUNUSED NONNULL((1)) struct memobjs *DCALL
memobjs_new(size_t objc) {
	struct memobjs *result;
	result = (struct memobjs *)Dee_Mallococ(offsetof(struct memobjs, mos_objv),
	                                        objc, sizeof(struct memobj));
	if likely(result) {
		result->mos_refcnt = 1;
		RINGQ_INIT(result, mos_copies);
		result->mos_objc = objc;
		DBG_memset(result->mos_objv, 0xcc, objc * sizeof(struct memobj));
	}
	return result;
}


INTERN NONNULL((1)) void DCALL
memobjs_destroy(struct memobjs *__restrict self) {
	size_t i;
	RINGQ_REMOVE(self, mos_copies);
	for (i = 0; i < self->mos_objc; ++i)
		memobj_fini(&self->mos_objv[i]);
	Dee_Free(self);
}

INTERN NONNULL((1)) void DCALL
memval_do_destroy_objn_or_xinfo(struct memval *__restrict self) {
	if (memval_hasobjn(self)) {
		struct memobjs *objn = memval_getobjn(self);
		memobjs_destroy(objn);
	} else {
		struct memobj_xinfo *xinfo;
		ASSERT(memobj_hasxinfo(memval_getobj0(self)));
		xinfo = memobj_getxinfo(memval_getobj0(self));
		memobj_xinfo_destroy(xinfo);
	}
}

INTERN WUNUSED NONNULL((1)) int DCALL
memval_do_objn_unshare(struct memval *__restrict self) {
	struct memobjs *objs, *copy;
	size_t sizeof_struct;
	ASSERT(memval_hasobjn(self));
	objs = memval_getobjn(self);
	ASSERT(memobjs_isshared(objs));
	sizeof_struct = (offsetof(struct memobjs, mos_objv)) +
	                (objs->mos_objc * sizeof(struct memobj));
	copy = (struct memobjs *)Dee_Malloc(sizeof_struct);
	if unlikely(!copy)
		goto err;
	copy = (struct memobjs *)memcpy(copy, objs, sizeof_struct);
	copy->mos_refcnt = 1;
	memobjs_decref_nokill(objs);
	self->mv_obj.mvo_n = copy->mos_objv;
	RINGQ_INSERT_AFTER(objs, copy, mos_copies); /* Part of same copy-ring */
	if (self->mv_flags & MEMVAL_F_NOREF) {
		size_t i; /* Inline the NOREF flag. */
		for (i = 0; i < copy->mos_objc; ++i)
			memobj_clearref(&copy->mos_objv[i]);
		self->mv_flags &= ~MEMVAL_F_NOREF;
	}
	return 0;
err:
	return -1;
}

/* Clear the buffered "MEMVAL_F_NOREF" flag, by unsharing memobjs,
 * and clearing the MEMOBJ_F_ISREF flags of all references objects. */
INTERN WUNUSED NONNULL((1)) int DCALL
memval_do_clear_MEMVAL_F_NOREF(struct memval *__restrict self) {
	size_t i;
	struct memobjs *objs;
	ASSERT(self->mv_flags & MEMVAL_F_NOREF);
	ASSERT(memval_hasobjn(self));
	objs = memval_getobjn(self);
	if (memobjs_isshared(objs)) {
		int temp;
		bool hasrefs = false;
		for (i = 0; i < objs->mos_objc; ++i)
			hasrefs |= memobj_isref(&objs->mos_objv[i]);
		if unlikely(!hasrefs)
			return 0;
		temp = memval_do_objn_unshare(self);
		if unlikely(temp)
			return temp;
		objs = memval_getobjn(self);
	}
	for (i = 0; i < objs->mos_objc; ++i)
		memobj_clearref(&objs->mos_objv[i]);
	self->mv_flags &= ~MEMVAL_F_NOREF;
	return 0;
}




/************************************************************************/
/* memstate                                                         */
/************************************************************************/

INTERN NONNULL((1)) void DCALL
memstate_destroy(struct memstate *__restrict self) {
	memequivs_fini(&self->ms_memequiv);
	Dee_Free(self->ms_stackv);
	memstate_free(self);
}

/* Replace `*p_self' with a copy of itself
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
memstate_inplace_copy_because_shared(struct memstate **__restrict p_self) {
	struct memstate *copy, *self;
	self = *p_self;
	ASSERT(memstate_isshared(self));
	copy = memstate_copy(self);
	if unlikely(!copy)
		goto err;
	memstate_decref_nokill(self);
	*p_self = copy;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF struct memstate *DCALL
memstate_copy(struct memstate *__restrict self) {
	DREF struct memstate *result;
	result = memstate_alloc(self->ms_localc);
	if unlikely(!result)
		goto err;
	memcpy(result, self,
	       offsetof(struct memstate, ms_localv) +
	       self->ms_localc * sizeof(struct memval));
	result->ms_stackv = (struct memval *)Dee_Mallocc(self->ms_stackc,
	                                                 sizeof(struct memval));
	if unlikely(!result->ms_stackv)
		goto err_r;
	result->ms_refcnt = 1;
	result->ms_stacka = self->ms_stackc;
	memcpyc(result->ms_stackv, self->ms_stackv,
	        self->ms_stackc, sizeof(struct memval));
	if unlikely(_memequivs_inplace_copy(&result->ms_memequiv))
		goto err_r_stack;
	/* TODO: Inplace-copy memval-s that use a variable number of memory locations. */
	return result;
err_r_stack:
	Dee_Free(result->ms_stackv);
err_r:
	memstate_free(result);
err:
	return NULL;
}




/* Fill in `self->hr_vtype' and `self->hr_value' based on `sym'
 * If `sym' has already been defined as absolute or pointing to
 * the start of a section, directly inline it. */
INTERN NONNULL((1, 2)) void DCALL
host_reloc_setsym(struct host_reloc *__restrict self,
                  struct host_symbol *__restrict sym) {
	switch (sym->hs_type) {
	case HOST_SYMBOL_ABS:
		self->hr_vtype = DEE_HOST_RELOCVALUE_ABS;
		self->hr_value.rv_abs = sym->hs_value.sv_abs;
		return;
#ifdef DEE_HOST_RELOCVALUE_SECT
	case HOST_SYMBOL_SECT:
		if (sym->hs_value.sv_sect.ss_off == 0) {
			self->hr_vtype = DEE_HOST_RELOCVALUE_SECT;
			self->hr_value.rv_sect = sym->hs_value.sv_sect.ss_sect;
			return;
		}
		break;
#endif /* DEE_HOST_RELOCVALUE_SECT */
	default: break;
	}
	self->hr_vtype = DEE_HOST_RELOCVALUE_SYM;
	self->hr_value.rv_sym = sym;
}


/* Calculate and return the value of `self'
 * Only returns valid values after `hs_base' have been assigned. */
INTERN ATTR_PURE WUNUSED NONNULL((1)) uintptr_t DCALL
host_symbol_value(struct host_symbol const *__restrict self) {
	ASSERTF(self->hs_type != HOST_SYMBOL_UNDEF,
	        "Symbol was never defined");
	switch (self->hs_type) {
	case HOST_SYMBOL_ABS:
		return (uintptr_t)self->hs_value.sv_abs;
	case HOST_SYMBOL_JUMP: {
		struct jump_descriptor *jmp = self->hs_value.sv_jump;
		if (host_section_islinked(&jmp->jd_morph)) {
			return (uintptr_t)jmp->jd_morph.hs_base;
		} else {
			struct basic_block *block;
			block = jmp->jd_to;
			while (host_section_islinked(&block->bb_htext)) {
				ASSERTF(block->bb_next, "symbol points to not-linked block with not successor");
				block = block->bb_next;
			}
			return (uintptr_t)block->bb_htext.hs_base;
		}
	}	break;
	case HOST_SYMBOL_SECT:
		return (uintptr_t)self->hs_value.sv_sect.ss_sect->hs_base +
		       (uintptr_t)self->hs_value.sv_sect.ss_off;
	default: __builtin_unreachable();
	}
}

/* Calculate and return the value of `self'
 * Only returns valid values after `hs_base' have been assigned. */
INTERN ATTR_PURE WUNUSED NONNULL((1)) uintptr_t DCALL
host_reloc_value(struct host_reloc const *__restrict self) {
	switch (self->hr_vtype) {
	case DEE_HOST_RELOCVALUE_SYM:
		return host_symbol_value(self->hr_value.rv_sym);
	case DEE_HOST_RELOCVALUE_ABS:
		return (uintptr_t)self->hr_value.rv_abs;
#ifdef DEE_HOST_RELOCVALUE_SECT
	case DEE_HOST_RELOCVALUE_SECT:
		return (uintptr_t)self->hr_value.rv_sect->hs_base;
#endif /* DEE_HOST_RELOCVALUE_SECT */
	default: __builtin_unreachable();
	}
}



INTERN NONNULL((1)) void DCALL
host_section_fini(struct host_section *__restrict self) {
#ifdef HOSTASM_HAVE_SHRINKJUMPS
	if (TAILQ_ISBOUND(self, hs_link)) {
		struct host_symbol *sym = self->hs_symbols;
		while (sym) {
			struct host_symbol *next;
			next = sym->_hs_next;
			_host_symbol_free(sym);
			sym = next;
		}
	}
#endif /* HOSTASM_HAVE_SHRINKJUMPS */
	Dee_Free(self->hs_start);
	Dee_Free(self->hs_relv);
	if (self->hs_cold) {
		host_section_fini(self->hs_cold);
		Dee_Free(self->hs_cold);
	}
	host_unwind_fini(&self->hs_unwind);
}

INTERN NONNULL((1)) void DCALL
host_section_clear(struct host_section *__restrict self) {
	do {
		self->hs_end  = self->hs_start;
		self->hs_relc = 0;
		host_unwind_clear(&self->hs_unwind);
		/* Must recursively clear cold (sub-)sections */
	} while ((self = self->hs_cold) != NULL);
}

/* Lazily allocate the cold sub-section of "self"
 * @return: NULL: Error */
INTERN NONNULL((1)) struct host_section *DCALL
host_section_getcold(struct host_section *__restrict self) {
	struct host_section *result = self->hs_cold;
	if (result == NULL) {
		result = (struct host_section *)Dee_Calloc(sizeof(struct host_section));
		self->hs_cold = result;
	}
	return result;
}

/* Ensure that at least `num_bytes' of host text memory are available.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
_host_section_reqhost(struct host_section *__restrict self,
                      size_t num_bytes) {
	byte_t *new_blob;
	size_t old_used  = (size_t)(self->hs_end - self->hs_start);
	size_t old_alloc = (size_t)(self->hs_alend - self->hs_start);
	size_t min_alloc = old_used + num_bytes;
	size_t new_alloc = old_alloc << 1;
	if (new_alloc < min_alloc)
		new_alloc = min_alloc;
	new_blob = (byte_t *)Dee_TryRealloc(self->hs_start, new_alloc);
	if (new_blob == NULL) {
		new_alloc = min_alloc;
		new_blob = (byte_t *)Dee_Realloc(self->hs_start, new_alloc);
		if unlikely(!new_blob)
			goto err;
	}
	self->hs_start = new_blob;
	self->hs_end   = new_blob + old_used;
	self->hs_alend = new_blob + new_alloc;
	ASSERT(self->hs_alend >= self->hs_end);
	return 0;
err:
	return -1;
}


/* Allocate and return a new host relocation. The caller is responsible
 * for filling in said relocation, and the returned pointer only remains
 * valid until the next call to this function with the same `self'.
 * @return: * :   The (uninitialized) host relocation
 * @return: NULL: Error  */
INTERN WUNUSED NONNULL((1)) struct host_reloc *DCALL
host_section_newhostrel(struct host_section *__restrict self) {
	struct host_reloc *result;
	ASSERT(self->hs_relc <= self->hs_rela);
	if unlikely(self->hs_relc >= self->hs_rela) {
		size_t min_alloc = self->hs_relc + 1;
		size_t new_alloc = self->hs_rela * 2;
		struct host_reloc *new_list;
		if (new_alloc < 4)
			new_alloc = 4;
		if (new_alloc < min_alloc)
			new_alloc = min_alloc;
		new_list = (struct host_reloc *)Dee_TryReallocc(self->hs_relv,
		                                                new_alloc,
		                                                sizeof(struct host_reloc));
		if unlikely(!new_list) {
			new_alloc = min_alloc;
			new_list = (struct host_reloc *)Dee_Reallocc(self->hs_relv,
			                                             new_alloc,
			                                             sizeof(struct host_reloc));
			if unlikely(!new_list)
				goto err;
		}
		self->hs_relv = new_list;
		self->hs_rela = new_alloc;
	}
	result = &self->hs_relv[self->hs_relc];
	++self->hs_relc;
	DBG_memset(result, 0xcc, sizeof(*result));
	return result;
err:
	return NULL;
}




/* Lookup the jump descriptor for `deemon_from'
 * @return: * :   The jump descriptor in question.
 * @return: NULL: No such jump descriptor. */
INTERN WUNUSED NONNULL((1)) struct jump_descriptor *DCALL
jump_descriptors_lookup(struct jump_descriptors const *__restrict self,
                        Dee_instruction_t const *deemon_from) {
	size_t lo = 0;
	size_t hi = self->jds_size;
	while (lo < hi) {
		struct jump_descriptor *result;
		size_t mid = (lo + hi) / 2;
		result = self->jds_list[mid];
		ASSERT(result);
		if (deemon_from < result->jd_from) {
			hi = mid;
		} else if (deemon_from > result->jd_from) {
			lo = mid + 1;
		} else {
			return result;
		}
	}
	return NULL;
}

/* Insert a new jump descriptor into `self'
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
jump_descriptors_insert(struct jump_descriptors *__restrict self,
                        struct jump_descriptor *__restrict descriptor) {
	byte_t const *deemon_from = descriptor->jd_from;
	size_t lo = 0;
	size_t hi = self->jds_size;
	while (lo < hi) {
		struct jump_descriptor *result;
		size_t mid = (lo + hi) / 2;
		result = self->jds_list[mid];
		ASSERT(result);
		ASSERTF(deemon_from < result->jd_from ||
		        deemon_from > result->jd_from,
		        "Duplicate jump descriptor for %p",
		        deemon_from);
		if (deemon_from < result->jd_from) {
			hi = mid;
		} else {
			lo = mid + 1;
		}
	}
	ASSERT(lo == hi);
	ASSERT(self->jds_size <= self->jds_alloc);
	if (self->jds_size >= self->jds_alloc) {
		struct jump_descriptor **new_list;
		size_t new_alloc = (self->jds_alloc << 1);
		if (new_alloc < 8)
			new_alloc = 8;
		new_list = (struct jump_descriptor **)Dee_TryReallocc(self->jds_list, new_alloc,
		                                                      sizeof(struct jump_descriptor *));
		if (!new_list) {
			new_alloc = self->jds_size + 1;
			new_list = (struct jump_descriptor **)Dee_Reallocc(self->jds_list, new_alloc,
			                                                   sizeof(struct jump_descriptor *));
			if unlikely(!new_list)
				goto err;
		}
		self->jds_list  = new_list;
		self->jds_alloc = new_alloc;
	}

	/* Insert descriptor into the list. */
	memmoveupc(&self->jds_list[lo + 1],
	           &self->jds_list[lo],
	           self->jds_size - lo,
	           sizeof(struct jump_descriptor *));
	self->jds_list[lo] = descriptor;
	++self->jds_size;
	return 0;
err:
	return -1;
}

/* Remove `descriptor' from `self' (said descriptor *must* be part of `self') */
INTERN WUNUSED NONNULL((1, 2)) void DCALL
jump_descriptors_remove(struct jump_descriptors *__restrict self,
                        struct jump_descriptor *__restrict descriptor) {
	size_t lo = 0;
	size_t hi = self->jds_size;
	for (;;) {
		struct jump_descriptor *result;
		size_t mid;
		ASSERT(lo < hi);
		mid = (lo + hi) / 2;
		result = self->jds_list[mid];
		ASSERT(result);
		if (descriptor->jd_from < result->jd_from) {
			hi = mid;
		} else if (descriptor->jd_from > result->jd_from) {
			lo = mid + 1;
		} else {
			ASSERT(result == descriptor);
			--self->jds_size;
			memmovedownc(&self->jds_list[mid],
			             &self->jds_list[mid + 1],
			             self->jds_size - mid,
			             sizeof(struct jump_descriptor *));
			break;
		}
	}
}



/* Destroy the given basic block `self'. */
INTERN NONNULL((1)) void DCALL
basic_block_destroy(struct basic_block *__restrict self) {
	size_t i;
	for (i = 0; i < self->bb_entries.jds_size; ++i) {
		struct jump_descriptor *descriptor;
		descriptor = self->bb_entries.jds_list[i];
		jump_descriptor_destroy(descriptor);
	}
	Dee_Free(self->bb_entries.jds_list);
	Dee_Free(self->bb_exits.jds_list);
	if (self->bb_mem_start)
		memstate_decref(self->bb_mem_start);
	if (self->bb_mem_end)
		memstate_decref(self->bb_mem_end);
	host_section_fini(&self->bb_htext);
	Dee_Free(self->bb_locreadv);
	basic_block_free(self);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
jump_descriptors_find_lowest_addr(struct jump_descriptors const *__restrict self,
                                  Dee_instruction_t const *deemon_from) {
	size_t lo = 0;
	size_t hi = self->jds_size;
	size_t result = hi;
	while (lo < hi) {
		struct jump_descriptor *descriptor;
		size_t mid = (lo + hi) / 2;
		descriptor = self->jds_list[mid];
		ASSERT(descriptor);
		if (descriptor->jd_from < deemon_from) {
			lo = mid + 1;
		} else if (descriptor->jd_from > deemon_from) {
			hi = result = mid;
		} else {
			return mid;
		}
	}
	while (result && self->jds_list[result - 1]->jd_from > deemon_from)
		--result;
	return result;
}

/* Split this basic block at `addr' (which must be `> bb_deemon_start'),
 * and move all jumps from `bb_exits' into the new basic block, as needed.
 * @return: * :   A new basic block that starts at `addr'
 * @return: NULL: Error */
INTERN WUNUSED NONNULL((1)) struct basic_block *DCALL
basic_block_splitat(struct basic_block *__restrict self,
                    Dee_instruction_t const *addr,
                    lid_t n_locals) {
	size_t exit_split;
	struct basic_block *result;
	ASSERT(addr > self->bb_deemon_start);
	ASSERT(addr < self->bb_deemon_end);
	result = basic_block_alloc(n_locals);
	if unlikely(!result)
		goto err;

	/* Figure out how many exits to transfer. */
	exit_split = jump_descriptors_find_lowest_addr(&self->bb_exits, addr);
	if (exit_split >= self->bb_exits.jds_size) {
		/* Special case: nothing to transfer */
		jump_descriptors_init(&result->bb_exits);
	} else if (exit_split == 0) {
		/* Special case: transfer everything */
		memcpy(&result->bb_exits, &self->bb_exits, sizeof(struct jump_descriptors));
		jump_descriptors_init(&self->bb_exits);
	} else {
		struct jump_descriptor **result_exits;
		size_t num_transfer;
		jump_descriptors_init(&result->bb_exits);
		num_transfer = self->bb_exits.jds_size - exit_split;
		ASSERT(num_transfer > 0);
		ASSERT(num_transfer < self->bb_exits.jds_size);
		result_exits = (struct jump_descriptor **)Dee_Mallocc(num_transfer,
		                                                          sizeof(struct jump_descriptor *));
		if unlikely(!result_exits)
			goto err_r;
		memcpyc(result_exits, self->bb_exits.jds_list + exit_split,
		        num_transfer, sizeof(struct jump_descriptor *));
		result->bb_exits.jds_list  = result_exits;
		result->bb_exits.jds_size  = num_transfer;
		result->bb_exits.jds_alloc = num_transfer;
		self->bb_exits.jds_size    = exit_split;
	}

	/* Fill in remaining fields and adjust caller-given block bounds. */
	basic_block_init_common(result);
	result->bb_deemon_start = addr;
	result->bb_deemon_end   = self->bb_deemon_end;
	self->bb_deemon_end     = addr;
	return result;
err_r:
	basic_block_free(result);
err:
	return NULL;
}



INTERN NONNULL((1)) void DCALL
inlined_references_fini(struct inlined_references *__restrict self) {
	if (self->ir_elem != NULL) {
		size_t i;
		for (i = 0; i <= self->ir_mask; ++i) {
			DREF DeeObject *ob = self->ir_elem[i];
			Dee_XDecref(ob);
		}
		Dee_Free(self->ir_elem);
	}
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
inlined_references_rehash(struct inlined_references *__restrict self) {
	DREF DeeObject **new_vector, **iter, **end;
	size_t new_mask = self->ir_mask;
	new_mask = (new_mask << 1) | 1;
	if unlikely(new_mask == 1)
		new_mask = 16 - 1; /* Start out bigger than 2. */
	new_vector = (DREF DeeObject **)Dee_Callocc(new_mask + 2, sizeof(DREF DeeObject *));
	if unlikely(!new_vector)
		return false;
	ASSERT((self->ir_elem == NULL) == (self->ir_mask == 0));
	ASSERT((self->ir_elem == NULL) == (self->ir_size == 0));
	if (self->ir_elem != NULL) {
		/* Re-insert all existing items into the new set vector. */
		end = (iter = self->ir_elem) + (self->ir_mask + 1);
		for (; iter < end; ++iter) {
			DREF DeeObject **item;
			dhash_t i, perturb;
			/* Skip NULL keys. */
			if (*iter == NULL)
				continue;
			perturb = i = inlined_references_hashof(*iter) & new_mask;
			for (;; inlined_references_hashnx(i, perturb)) {
				item = &new_vector[i & new_mask];
				if (!*item)
					break; /* Empty slot found. */
			}
			/* Transfer this object. */
			*item = *iter;
		}
		Dee_Free(self->ir_elem);
	}
	self->ir_mask = new_mask;
	self->ir_elem = new_vector;
	return 0;
}

/* Make sure that `inherit_me' appears in `self', thus inheriting a reference to it.
 * @return: inherit_me: Success: `self' now owns the reference to `inherit_me', and you can use it lazily
 * @return: NULL:       Error */
INTERN WUNUSED NONNULL((1, 2)) DeeObject *DCALL
inlined_references_ref(struct inlined_references *__restrict self,
                       /*inherit(always)*/ DREF DeeObject *inherit_me) {
	dhash_t i, perturb, hash;
	/* Check if a rehash is needed */
	if ((self->ir_size + 1) * 2 > self->ir_mask) {
		if unlikely(inlined_references_rehash(self)) {
			Dee_Decref_unlikely(inherit_me);
			return NULL;
		}
	}
	hash    = inlined_references_hashof(inherit_me);
	perturb = i = inlined_references_hashst(self, hash);
	for (;; inlined_references_hashnx(i, perturb)) {
		DREF DeeObject **item;
		item = inlined_references_hashit(self, i);
		if (*item) { /* Already in use */
			if likely(*item == inherit_me) {
				Dee_DecrefNokill(inherit_me);
				break;
			}
		} else {
			/* Fill in unused slot. */
			*item = inherit_me; /* Inherited */
			++self->ir_size;
			break;
		}
	}
	return inherit_me;
}



INTERN NONNULL((1)) void DCALL
function_assembler_fini(struct function_assembler *__restrict self) {
	size_t i;
	for (i = 0; i < self->fa_blockc; ++i)
		basic_block_destroy(self->fa_blockv[i]);
	for (i = 0; i < self->fa_except_exitc; ++i)
		except_exitinfo_destroy(self->fa_except_exitv[i]);
	if (self->fa_prolog_end)
		memstate_decref(self->fa_prolog_end);
	inlined_references_fini(&self->fa_irefs);
	host_section_fini(&self->fa_prolog);
	Dee_Free(self->fa_blockv);
	Dee_Free(self->fa_except_exitv);
	{
		struct basic_block *block = self->fa_deleted;
		while (block) {
			struct basic_block *next;
			next = block->bb_next;
			basic_block_destroy(block);
			block = next;
		}
	}
	{
		struct except_exitinfo *info = self->fa_except_del;
		while (info) {
			struct except_exitinfo *next;
			next = info->exi_next;
			except_exitinfo_destroy(info);
			info = next;
		}
	}
	{
		struct host_symbol *sym = self->fa_symbols;
		while (sym) {
			struct host_symbol *next = sym->_hs_next;
			_host_symbol_free(sym);
			sym = next;
		}
	}
}


/* Ensure that the basic block containing `deemon_addr' also *starts* at that address.
 * This function is used during the initial scan-pass where basic blocks are identified
 * and created.
 * @return: * :   The basic block in question.
 * @return: NULL: An error occurred (OOM or address is out-of-bounds). */
INTERN WUNUSED NONNULL((1)) struct basic_block *DCALL
function_assembler_splitblock(struct function_assembler *__restrict self,
                              Dee_instruction_t const *deemon_addr) {
	size_t lo = 0;
	size_t hi = self->fa_blockc;
	ASSERT(self->fa_blockc <= self->fa_blocka);
	while (lo < hi) {
		struct basic_block *result;
		size_t mid = (lo + hi) / 2;
		result = self->fa_blockv[mid];
		ASSERT(result);
		if (deemon_addr < result->bb_deemon_start) {
			hi = mid;
		} else if (deemon_addr >= result->bb_deemon_end) {
			lo = mid + 1;
		} else {
			if (deemon_addr > result->bb_deemon_start) {
				/* Ensure that there is sufficient space in the bb-vector. */
				if (self->fa_blockc >= self->fa_blocka) {
					struct basic_block **new_list;
					size_t new_alloc = (self->fa_blocka << 1);
					if (new_alloc < 8)
						new_alloc = 8;
					new_list = (struct basic_block **)Dee_TryReallocc(self->fa_blockv, new_alloc,
					                                                      sizeof(struct basic_block *));
					if (!new_list) {
						new_alloc = self->fa_blockc + 1;
						new_list = (struct basic_block **)Dee_Reallocc(self->fa_blockv, new_alloc,
						                                                   sizeof(struct basic_block *));
						if unlikely(!new_list)
							goto err;
					}
					self->fa_blockv = new_list;
					self->fa_blocka = new_alloc;
				}

				/* Must split this basic block. */
				result = basic_block_splitat(result, deemon_addr, self->fa_xlocalc);
				if unlikely(!result)
					goto err;

				/* Insert the new block into the vector at `mid+1' */
				++mid;
				memmoveupc(&self->fa_blockv[mid + 1],
				           &self->fa_blockv[mid],
				           self->fa_blockc - mid,
				           sizeof(struct jump_descriptor *));
				self->fa_blockv[mid] = result;
				++self->fa_blockc;
			}
			return result;
		}
	}
	DeeError_Throwf(&DeeError_SegFault,
	                "Out-of-bounds text location %#.4" PRFx32 " accessed",
	                function_assembler_addrof(self, deemon_addr));
err:
	return NULL;
}

/* Locate the basic block that contains `deemon_addr'
 * @return: * :   The basic block in question.
 * @return: NULL: Address is out-of-bounds. */
INTERN WUNUSED NONNULL((1)) struct basic_block *DCALL
function_assembler_locateblock(struct function_assembler const *__restrict self,
                               Dee_instruction_t const *deemon_addr) {
	size_t lo = 0;
	size_t hi = self->fa_blockc;
	ASSERT(self->fa_blockc <= self->fa_blocka);
	while (lo < hi) {
		struct basic_block *result;
		size_t mid = (lo + hi) / 2;
		result = self->fa_blockv[mid];
		ASSERT(result);
		if (deemon_addr < result->bb_deemon_start) {
			hi = mid;
		} else if (deemon_addr >= result->bb_deemon_end) {
			lo = mid + 1;
		} else {
			return result;
		}
	}
	return NULL;
}


PRIVATE ATTR_CONST WUNUSED uint8_t DCALL
memref_constrain_flags(uint8_t a, uint8_t b) {
	return ((a | b) & MEMREF_F_NULLABLE) |
	       ((a & b) & ~MEMREF_F_NULLABLE);
}

/* Lookup/allocate an exception-exit basic block that can be used to clean
 * up `state' and then return `NULL' to the caller of the generated function.
 * @return: * :   The basic block to which to jump in order to clean up `state'.
 * @return: NULL: Error. */
INTERN WUNUSED NONNULL((1, 2)) struct except_exitinfo *DCALL
function_assembler_except_exit(struct function_assembler *__restrict self,
                               struct memstate const *__restrict state) {
	size_t lo, hi;
	size_t infsize;
	struct except_exitinfo *result;
	struct except_exitinfo_id *info_id;
	infsize = except_exitinfo_id_sizefor(state);
	info_id = (struct except_exitinfo_id *)Dee_Malloca(infsize);
	if unlikely(!info_id)
		goto err;

	/* Fill in info. */
	info_id = except_exitinfo_id_init(info_id, state);

	/* Check if we already have a block for this state. */
	lo = 0;
	hi = self->fa_except_exitc;
	while (lo < hi) {
		int diff;
		struct except_exitinfo *oldinfo;
		size_t mid = (lo + hi) / 2;
		oldinfo = self->fa_except_exitv[mid];
		ASSERT(oldinfo);
		diff = except_exitinfo_id_compare(info_id, except_exitinfo_asid(oldinfo));
		if (diff < 0) {
			hi = mid;
		} else if (diff > 0) {
			lo = mid + 1;
		} else {
			size_t i;
			/* Found it (but may need to constrain) */
			ASSERT(oldinfo->exi_memrefc == info_id->exi_memrefc);
			for (i = 0; i < oldinfo->exi_memrefc; ++i) {
				oldinfo->exi_memrefv[i].mr_flags = memref_constrain_flags(oldinfo->exi_memrefv[i].mr_flags,
				                                                              info_id->exi_memrefv[i].mr_flags);
			}
			Dee_Freea(info_id);
			return oldinfo;
		}
	}
	ASSERT(lo == hi);

	/* Need to create+insert a new exit information descriptor. */
	result = except_exitinfo_alloc(infsize + (offsetof(struct except_exitinfo, exi_memrefv) -
	                                          offsetof(struct except_exitinfo_id, exi_memrefv)));
	if unlikely(!result)
		goto err_info_id;
	memcpy(except_exitinfo_asid(result), info_id, infsize);
	host_section_init(&result->exi_text);
	result->exi_next = NULL;

	/* Make sure there is enough space in the sorted list of exit information descriptors. */
	ASSERT(self->fa_except_exitc <= self->fa_except_exita);
	if unlikely(self->fa_except_exitc >= self->fa_except_exita) {
		struct except_exitinfo **new_list;
		size_t new_alloc = self->fa_except_exita * 2;
		size_t min_alloc = self->fa_except_exitc + 1;
		if (new_alloc < 8)
			new_alloc = 8;
		if (new_alloc < min_alloc)
			new_alloc = min_alloc;
		new_list = (struct except_exitinfo **)Dee_TryReallocc(self->fa_except_exitv,
		                                                      new_alloc,
		                                                      sizeof(struct except_exitinfo *));
		if unlikely(!new_list) {
			new_alloc = min_alloc;
			new_list = (struct except_exitinfo **)Dee_Reallocc(self->fa_except_exitv,
			                                                   new_alloc,
			                                                   sizeof(struct except_exitinfo *));
			if unlikely(!new_list)
				goto err_info_id_result;
		}
		self->fa_except_exitv = new_list;
		self->fa_except_exita = new_alloc;
	}

	/* Insert the new info-descriptor at the appropriate location (`lo'). */
	memmoveupc(&self->fa_except_exitv[lo + 1],
	           &self->fa_except_exitv[lo],
	           self->fa_except_exitc - lo,
	           sizeof(struct except_exitinfo *));
	self->fa_except_exitv[lo] = result;
	++self->fa_except_exitc;

	return result;
err_info_id_result:
	basic_block_free(result);
err_info_id:
	Dee_Freea(info_id);
err:
	return NULL;
}

/* Allocate a new host text symbol and return it.
 * @return: * :   The newly allocated host text symbol
 * @return: NULL: Error */
#ifdef HAVE_HOST_SYMBOL_ALLOC_INFO
#ifdef NO_HOSTASM_DEBUG_PRINT
INTERN WUNUSED NONNULL((1)) struct host_symbol *DCALL
function_assembler_newsym_dbg(struct function_assembler *__restrict self,
                              char const *file, int line)
#else /* NO_HOSTASM_DEBUG_PRINT */
INTERN WUNUSED NONNULL((1)) struct host_symbol *DCALL
function_assembler_newsym_named_dbg(struct function_assembler *__restrict self,
                                    char const *name, char const *file, int line)
#endif /* !NO_HOSTASM_DEBUG_PRINT */
#elif defined(NO_HOSTASM_DEBUG_PRINT)
INTERN WUNUSED NONNULL((1)) struct host_symbol *DCALL
function_assembler_newsym(struct function_assembler *__restrict self)
#else /* ... */
INTERN WUNUSED NONNULL((1)) struct host_symbol *DCALL
function_assembler_newsym_named(struct function_assembler *__restrict self,
                                char const *name)
#endif /* !... */
{
	struct host_symbol *result = _host_symbol_alloc();
	if likely(result) {
#ifndef NO_HOSTASM_DEBUG_PRINT
		result->hs_name = name;
#endif /* !NO_HOSTASM_DEBUG_PRINT */
#ifdef HAVE_HOST_SYMBOL_ALLOC_INFO
		result->hs_file = file;
		result->hs_line = line;
#endif /* HAVE_HOST_SYMBOL_ALLOC_INFO */
		result->hs_type  = HOST_SYMBOL_UNDEF;
		result->_hs_next = self->fa_symbols;
		self->fa_symbols = result;
	}
	return result;
}





/* Error throwing helper functions. */
INTERN ATTR_COLD int DCALL err_illegal_stack_effect(void) {
	return DeeError_Throwf(&DeeError_SegFault, "Illegal stack effect");
}
INTERN ATTR_COLD int DCALL err_illegal_ulid(ulid_t lid) {
	return DeeError_Throwf(&DeeError_SegFault, "Illegal local variable ID: %#" PRFx16, lid);
}
INTERN ATTR_COLD int DCALL err_illegal_mid(uint16_t mid) {
	return DeeError_Throwf(&DeeError_SegFault, "Illegal module ID: %#" PRFx16, mid);
}
INTERN ATTR_COLD int DCALL err_illegal_aid(aid_t aid) {
	return DeeError_Throwf(&DeeError_SegFault, "Illegal argument ID: %#" PRFx16, aid);
}
INTERN ATTR_COLD int DCALL err_illegal_cid(uint16_t cid) {
	return DeeError_Throwf(&DeeError_SegFault, "Illegal constant ID: %#" PRFx16, cid);
}
INTERN ATTR_COLD int DCALL err_illegal_rid(uint16_t rid) {
	return DeeError_Throwf(&DeeError_SegFault, "Illegal reference ID: %#" PRFx16, rid);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_illegal_gid(struct Dee_module_object *__restrict mod, uint16_t gid) {
	return DeeError_Throwf(&DeeError_SegFault, "Illegal global ID in %r: %#" PRFx16, mod, gid);
}

INTERN ATTR_COLD NONNULL((1)) int DCALL
err_unsupported_opcode(DeeCodeObject *__restrict code, Dee_instruction_t const *instr) {
	uint16_t opcode;
	char const *code_name = DeeCode_NAME(code);
	if (!instr) {
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Unsupported opcode in '%s'",
		                       code_name);
	}
	opcode = instr[0];
	if (ASM_ISEXTENDED(opcode))
		opcode = (opcode << 8) | instr[1];
	if (!ASM_ISPREFIX(opcode & 0xff)) {
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Unsupported opcode %#.2" PRFx16 " at +%.4" PRFx32 " in '%s'",
		                       opcode, (Dee_code_addr_t)(instr - code->co_code), code_name);
	} else {
		uint16_t opcode2;
		Dee_instruction_t const *instr2;
		instr2 = instr + 1;
		if (opcode > 0xff)
			++instr2;
		opcode2 = instr2[0];
		if (ASM_ISEXTENDED(opcode2))
			opcode2 = (opcode2 << 8) | instr2[1];
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Unsupported opcode %#.2" PRFx16 " %.2" PRFx16 " at +%.4" PRFx32 " in '%s'",
		                       opcode, opcode2,
		                       (Dee_code_addr_t)(instr - code->co_code),
		                       code_name);
	}
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_invalid_refs_size)(DeeCodeObject *__restrict code, size_t num_refs) {
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Code object expects %" PRFu16 " references when %" PRFuSIZ " were given",
	                       code->co_refc, num_refs);
}




/************************************************************************/
/* C-Callable host function output data structure                       */
/************************************************************************/

INTERN NONNULL((1)) void DCALL
hostfunc_fini(struct hostfunc *__restrict self) {
#ifndef CONFIG_host_unwind_USES_NOOP
	hostfunc_unwind_fini(&self->hf_unwind);
#endif /* !CONFIG_host_unwind_USES_NOOP */
	host_rawfunc_fini(&self->hf_raw);
	if (self->hf_refs) {
		size_t i;
		DeeObject *obj;
		for (i = 0; (obj = self->hf_refs[i]) != NULL; ++i)
			Dee_Decref(obj);
		Dee_Free(self->hf_refs);
	}
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_COMMON_C */
