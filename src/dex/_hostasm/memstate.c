/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
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

/* Check if `a' and `b' describe the same host memory location (i.e. are aliasing each other). */
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
Dee_memloc_sameloc(struct Dee_memloc const *a,
                   struct Dee_memloc const *b) {
	if (a->ml_type != b->ml_type)
		return false;
	switch (a->ml_type) {
	case MEMLOC_TYPE_HREG:
		return a->ml_value.v_hreg.r_regno == b->ml_value.v_hreg.r_regno &&
		       a->ml_value.v_hreg.r_off == b->ml_value.v_hreg.r_off;
	case MEMLOC_TYPE_HREGIND:
		return a->ml_value.v_hreg.r_regno == b->ml_value.v_hreg.r_regno &&
		       a->ml_value.v_hreg.r_off == b->ml_value.v_hreg.r_off &&
		       a->ml_value.v_hreg.r_voff == b->ml_value.v_hreg.r_voff;
	case MEMLOC_TYPE_HSTACK:
		return a->ml_value.v_hstack.s_cfa == b->ml_value.v_hstack.s_cfa;
	case MEMLOC_TYPE_HSTACKIND:
		return a->ml_value.v_hstack.s_cfa == b->ml_value.v_hstack.s_cfa &&
		       a->ml_value.v_hstack.s_off == b->ml_value.v_hstack.s_off;
	case MEMLOC_TYPE_CONST:
		return a->ml_value.v_const == b->ml_value.v_const;
	case MEMLOC_TYPE_UNALLOC:
		return true;
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}


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
		if (!Dee_memstate_hregs_isused(self, result)) {
			if (self->ms_rusage[result] == DEE_HOST_REGUSAGE_GENERIC)
				return result;
		}
	}
	if (accept_if_with_regusage) {
		/* Check for registers that are only used by regusage. */
		for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
			if (Dee_memstate_hregs_isused(self, result))
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
		if (!BITSET_GET(&used, result) && self->ms_rusage[result] == DEE_HOST_REGUSAGE_GENERIC)
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
	size_t i;
	for (i = 0; i < self->ms_stackc; ++i) {
		struct Dee_memloc *loc = &self->ms_stackv[i];
		if (MEMLOC_TYPE_HASREG(loc->ml_type) &&
		    loc->ml_value.v_hreg.r_regno == regno)
			loc->ml_value.v_hreg.r_off -= delta;
	}
	for (i = 0; i < self->ms_localc; ++i) {
		struct Dee_memloc *loc = &self->ms_localv[i];
		if (MEMLOC_TYPE_HASREG(loc->ml_type) &&
		    loc->ml_value.v_hreg.r_regno == regno)
			loc->ml_value.v_hreg.r_off -= delta;
	}
}


/* Check if a pointer-sized blob at `cfa_offset' is being used by something. */
INTERN ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_memstate_hstack_isused(struct Dee_memstate const *__restrict self,
                           uintptr_t cfa_offset) {
	size_t i;
	for (i = 0; i < self->ms_stackc; ++i) {
		struct Dee_memloc const *loc = &self->ms_stackv[i];
		if (loc->ml_type == MEMLOC_TYPE_HSTACKIND &&
		    loc->ml_value.v_hstack.s_cfa == cfa_offset)
			return true;
	}
	for (i = 0; i < self->ms_localc; ++i) {
		struct Dee_memloc const *loc = &self->ms_localv[i];
		if (loc->ml_type == MEMLOC_TYPE_HSTACKIND &&
		    loc->ml_value.v_hstack.s_cfa == cfa_offset)
			return true;
	}
	return false;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
Dee_memloc_hstack_used(struct Dee_memloc const *__restrict self,
                       uintptr_t min_offset, uintptr_t end_offset) {
	if (self->ml_type == MEMLOC_TYPE_HSTACKIND) {
#ifdef HOSTASM_STACK_GROWS_DOWN
		if (self->ml_value.v_hstack.s_cfa > min_offset &&
		    self->ml_value.v_hstack.s_cfa <= end_offset)
			return true;
#else /* HOSTASM_STACK_GROWS_DOWN */
		if (self->ml_value.v_hstack.s_cfa >= min_offset &&
		    self->ml_value.v_hstack.s_cfa < end_offset)
			return true;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
	}
	return false; /* No used by this one! */
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
Dee_memstate_hstack_unused(struct Dee_memstate const *__restrict self,
                           uintptr_t min_offset, uintptr_t end_offset) {
	size_t i;
	for (i = 0; i < self->ms_stackc; ++i) {
		if (Dee_memloc_hstack_used(&self->ms_stackv[i], min_offset, end_offset))
			return false;
	}
	for (i = 0; i < self->ms_localc; ++i) {
		if (Dee_memloc_hstack_used(&self->ms_localv[i], min_offset, end_offset))
			return false;
	}
	return true;
}

/* Try to find a `n_bytes'-large free section of host stack memory.
 * @return: * :            The base-CFA offset of the free section of memory
 * @return: (uintptr_t)-1: There is no free section of at least `n_bytes' bytes.
 *                         In this case, allocate using `Dee_memstate_hstack_alloca()' */
INTERN ATTR_PURE WUNUSED NONNULL((1)) uintptr_t DCALL
Dee_memstate_hstack_find(struct Dee_memstate const *__restrict self, size_t n_bytes) {
	ASSERT(IS_ALIGNED(n_bytes, HOST_SIZEOF_POINTER));
	if (n_bytes <= self->ms_host_cfa_offset) {
		size_t a_pointers = self->ms_host_cfa_offset / HOST_SIZEOF_POINTER;
		size_t n_pointers = n_bytes / HOST_SIZEOF_POINTER;
		size_t i, check = (a_pointers - n_pointers) + 1;
		for (i = 0; i < check; ++i) {
			uintptr_t min_offset = i * HOST_SIZEOF_POINTER;
			uintptr_t end_offset = min_offset + n_bytes;
			if (Dee_memstate_hstack_unused(self, min_offset, end_offset)) {
#ifdef HOSTASM_STACK_GROWS_DOWN
				return end_offset;
#else /* HOSTASM_STACK_GROWS_DOWN */
				return min_offset;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			}
		}
	}
	return (uintptr_t)-1;
}

/* Try to free unused stack memory near the top of the stack.
 * @return: true:  The CFA offset was reduced.
 * @return: false: The CFA offset remains the same. */
INTERN NONNULL((1)) bool DCALL
Dee_memstate_hstack_free(struct Dee_memstate *__restrict self) {
	bool result = false;
	while (self->ms_host_cfa_offset > 0) {
		size_t a_pointers = self->ms_host_cfa_offset / HOST_SIZEOF_POINTER;
		uintptr_t min_offset = (a_pointers - 1) * HOST_SIZEOF_POINTER;
		uintptr_t end_offset = min_offset + HOST_SIZEOF_POINTER;
		if (!Dee_memstate_hstack_unused(self, min_offset, end_offset))
			break;
		self->ms_host_cfa_offset -= HOST_SIZEOF_POINTER;
		result = true;
	}
	return false;
}


PRIVATE NONNULL((1, 2, 3)) bool DCALL
Dee_memloc_constrainwith(struct Dee_memstate *__restrict state,
                         struct Dee_memloc *self,
                         struct Dee_memloc const *__restrict other,
                         bool is_local) {
	bool result = false;

	/* If `MEMLOC_F_NOREF' isn't set in both locations, must clear it in `self' */
	if ((self->ml_flags & MEMLOC_F_NOREF) && !(other->ml_flags & MEMLOC_F_NOREF)) {
		self->ml_flags &= ~MEMLOC_F_NOREF;
		result = true;
	}

	/* For local variables, merge the binding state of the variable. */
	if (is_local) {
		uint16_t nw_bound;
		uint16_t my_bound = self->ml_flags & MEMLOC_M_LOCAL_BSTATE;
		uint16_t ot_bound = other->ml_flags & MEMLOC_M_LOCAL_BSTATE;
		ASSERTF(my_bound != (MEMLOC_F_LOCAL_BOUND | MEMLOC_F_LOCAL_UNBOUND), "Can't be both bound and unbound at once");
		ASSERTF(ot_bound != (MEMLOC_F_LOCAL_BOUND | MEMLOC_F_LOCAL_UNBOUND), "Can't be both bound and unbound at once");
		nw_bound = ((my_bound & ot_bound) & MEMLOC_F_LOCAL_BOUND) |
		           ((my_bound | ot_bound) & MEMLOC_F_LOCAL_UNBOUND);
		if (my_bound != nw_bound) {
			self->ml_flags &= ~MEMLOC_M_LOCAL_BSTATE;
			self->ml_flags |= nw_bound;
			result = true;
		}
	}

	/* Quick check: are locations identical? */
	if (!Dee_memloc_sameloc(self, other)) {
		switch (self->ml_type) {
	
		case MEMLOC_TYPE_HSTACKIND:
			if ((intptr_t)other->ml_value.v_hstack.s_cfa >= 0)
				break; /* Normal stack location */
			ATTR_FALLTHROUGH
		case MEMLOC_TYPE_HREGIND:
		case MEMLOC_TYPE_HSTACK:
		case MEMLOC_TYPE_CONST: {
			/* On one side it's an argument or a constant, and on the other
			 * side it's something else.
			 * In this case, need to convert to a register/stack location. */
			Dee_host_register_t regno;

			/* If the location describe by `other' isn't already in use in `state',
			 * then use *it* as-it. That way, we can reduce the necessary number of
			 * memory state transformation! */
			switch (other->ml_type) {
	
			case MEMLOC_TYPE_HSTACKIND:
				if ((intptr_t)other->ml_value.v_hstack.s_cfa < 0)
					break; /* Out-of-band location (e.g. true arguments on i386) */
				if (!Dee_memstate_hstack_isused(state, other->ml_value.v_hstack.s_cfa)) {
					uintptr_t min_cfa_offset;
					self->ml_type = MEMLOC_TYPE_HSTACKIND;
					self->ml_value.v_hstack.s_cfa = other->ml_value.v_hstack.s_cfa;
					self->ml_value.v_hstack.s_off = other->ml_value.v_hstack.s_off;
					min_cfa_offset = other->ml_value.v_hstack.s_cfa;
#ifndef HOSTASM_STACK_GROWS_DOWN
					min_cfa_offset += HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
					if (state->ms_host_cfa_offset < min_cfa_offset)
						state->ms_host_cfa_offset = min_cfa_offset;
					goto did_runtime_value_merge;
				}
				break;

			case MEMLOC_TYPE_HREG:
				if (!Dee_memstate_hregs_isused(state, other->ml_value.v_hreg.r_regno)) {
					self->ml_type = MEMLOC_TYPE_HREG;
					self->ml_value.v_hreg.r_regno = other->ml_value.v_hreg.r_regno;
					goto did_runtime_value_merge;
				}
				break;
	
			default:
				break;
			}
	
			regno = Dee_memstate_hregs_find_unused(state, true);
			if (regno < HOST_REGISTER_COUNT) {
				self->ml_type = MEMLOC_TYPE_HREG;
				self->ml_value.v_hreg.r_regno = regno;
				self->ml_value.v_hreg.r_off   = 0;
				state->ms_rusage[regno] = DEE_HOST_REGUSAGE_GENERIC;
			} else {
				/* Use a stack location. */
				uintptr_t cfa_offset;
				cfa_offset = Dee_memstate_hstack_find(state, HOST_SIZEOF_POINTER);
				if (cfa_offset == (uintptr_t)-1)
					cfa_offset = Dee_memstate_hstack_alloca(state, HOST_SIZEOF_POINTER);
				self->ml_type = MEMLOC_TYPE_HSTACKIND;
				self->ml_value.v_hstack.s_cfa = cfa_offset;
				self->ml_value.v_hstack.s_off = 0;
			}
			if (other->ml_type == MEMLOC_TYPE_HREGIND ||
			    other->ml_type == MEMLOC_TYPE_HSTACKIND) {
				if (self->ml_type == MEMLOC_TYPE_HREG) {
					self->ml_value.v_hreg.r_off = other->ml_value.v_hstack.s_off;
				} else {
					self->ml_value.v_hstack.s_off = other->ml_value.v_hstack.s_off;
				}
			}
did_runtime_value_merge:
			result = true;
		}	break;
	
		default:
			break;
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

	/* Mark usage registers as undefined if different between blocks. */
	for (regno = 0; regno < HOST_REGISTER_COUNT; ++regno) {
		if (self->ms_rusage[regno] != other->ms_rusage[regno]) {
			self->ms_rusage[regno] = DEE_HOST_REGUSAGE_GENERIC;
			result = true;
		}
	}

	/* Merge stack/locals memory locations. */
	for (i = 0; i < self->ms_stackc; ++i)
		result |= Dee_memloc_constrainwith(self, &self->ms_stackv[i], &other->ms_stackv[i], false);
	for (i = 0; i < self->ms_localc; ++i)
		result |= Dee_memloc_constrainwith(self, &self->ms_localv[i], &other->ms_localv[i], true);
	return result;
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
		_Dee_memstate_debug_print(block_start);
		_Dee_memstate_debug_print(state);
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
Dee_memstate_reqvstack(struct Dee_memstate *__restrict self, uint16_t min_alloc) {
	ASSERT(self->ms_stackc <= self->ms_stacka);
	if (min_alloc > self->ms_stacka) {
		struct Dee_memloc *new_stack;
		new_stack = (struct Dee_memloc *)Dee_Reallocc(self->ms_stackv, min_alloc,
		                                              sizeof(struct Dee_memloc));
		if unlikely(!new_stack)
			goto err;
		self->ms_stackv = new_stack;
		self->ms_stacka = min_alloc;
	}
	return 0;
err:
	return -1;
}

#ifndef NDEBUG
INTERN NONNULL((1)) void DCALL
Dee_memstate_verifyrinuse_d(struct Dee_memstate *__restrict self) {
	size_t i, correct_rinuse[HOST_REGISTER_COUNT];
	bzero(correct_rinuse, sizeof(correct_rinuse));
	for (i = 0; i < self->ms_stackc; ++i) {
		struct Dee_memloc *loc = &self->ms_stackv[i];
		if (MEMLOC_TYPE_HASREG(loc->ml_type)) {
			ASSERT(loc->ml_value.v_hreg.r_regno < HOST_REGISTER_COUNT);
			++correct_rinuse[loc->ml_value.v_hreg.r_regno];
		}
	}
	for (i = 0; i < self->ms_localc; ++i) {
		struct Dee_memloc *loc = &self->ms_localv[i];
		if (MEMLOC_TYPE_HASREG(loc->ml_type)) {
			ASSERT(loc->ml_value.v_hreg.r_regno < HOST_REGISTER_COUNT);
			++correct_rinuse[loc->ml_value.v_hreg.r_regno];
		}
	}
	ASSERTF(memcmp(self->ms_rinuse, correct_rinuse, sizeof(correct_rinuse)) == 0,
	        "Incorrect register-in-use numbers");
}
#endif /* !NDEBUG */


INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vswap(struct Dee_memstate *__restrict self) {
	struct Dee_memloc temp;
	if unlikely(self->ms_stackc < 2)
		return err_illegal_stack_effect();
	temp = self->ms_stackv[self->ms_stackc - 2];
	self->ms_stackv[self->ms_stackc - 2] = self->ms_stackv[self->ms_stackc - 1];
	self->ms_stackv[self->ms_stackc - 1] = temp;
	return 0;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vlrot(struct Dee_memstate *__restrict self, size_t n) {
	if likely(n > 1) {
		struct Dee_memloc temp;
		if unlikely(self->ms_stackc < n)
			return err_illegal_stack_effect();
		temp = self->ms_stackv[self->ms_stackc - n];
		memmovedownc(&self->ms_stackv[self->ms_stackc - n],
		             &self->ms_stackv[self->ms_stackc - (n - 1)],
		             n - 1, sizeof(struct Dee_memloc));
		self->ms_stackv[self->ms_stackc - 1] = temp;
	}
	return 0;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vrrot(struct Dee_memstate *__restrict self, size_t n) {
	if likely(n > 1) {
		struct Dee_memloc temp;
		if unlikely(self->ms_stackc < n)
			return err_illegal_stack_effect();
		temp = self->ms_stackv[self->ms_stackc - 1];
		memmoveupc(&self->ms_stackv[self->ms_stackc - (n - 1)],
		           &self->ms_stackv[self->ms_stackc - n],
		           n - 1, sizeof(struct Dee_memloc));
		self->ms_stackv[self->ms_stackc - n] = temp;
	}
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_memstate_vpush(struct Dee_memstate *__restrict self, struct Dee_memloc *loc) {
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	self->ms_stackv[self->ms_stackc] = *loc;
	self->ms_stackv[self->ms_stackc].ml_flags &= ~MEMLOC_M_LOCAL_BSTATE;
	if (MEMLOC_TYPE_HASREG(loc->ml_type))
		Dee_memstate_incrinuse(self, loc->ml_value.v_hreg.r_regno);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vpush_const(struct Dee_memstate *__restrict self, DeeObject *value) {
	struct Dee_memloc *loc;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	loc = &self->ms_stackv[self->ms_stackc];
	loc->ml_flags = MEMLOC_F_NOREF;
	loc->ml_type  = MEMLOC_TYPE_CONST;
	loc->ml_value.v_const = value;
	++self->ms_stackc;
	return 0;
err:
	return -1;
}


/* Sets the `MEMLOC_F_NOREF' flag */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vpush_reg(struct Dee_memstate *__restrict self,
                       Dee_host_register_t regno, ptrdiff_t delta) {
	struct Dee_memloc *loc;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	loc = &self->ms_stackv[self->ms_stackc];
	loc->ml_flags = MEMLOC_F_NOREF;
	loc->ml_type  = MEMLOC_TYPE_HREG;
	loc->ml_value.v_hreg.r_regno = regno;
	loc->ml_value.v_hreg.r_off   = delta;
	Dee_memstate_incrinuse(self, regno);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vpush_regind(struct Dee_memstate *__restrict self,
                          Dee_host_register_t regno,
                          ptrdiff_t ind_delta, ptrdiff_t val_delta) {
	struct Dee_memloc *loc;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	loc = &self->ms_stackv[self->ms_stackc];
	loc->ml_flags = MEMLOC_F_NOREF;
	loc->ml_type  = MEMLOC_TYPE_HREGIND;
	loc->ml_value.v_hreg.r_regno = regno;
	loc->ml_value.v_hreg.r_off   = ind_delta;
	loc->ml_value.v_hreg.r_voff  = val_delta;
	Dee_memstate_incrinuse(self, regno);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vpush_hstack(struct Dee_memstate *__restrict self,
                          uintptr_t cfa_offset) {
	struct Dee_memloc *loc;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	loc = &self->ms_stackv[self->ms_stackc];
	loc->ml_flags = MEMLOC_F_NOREF;
	loc->ml_type  = MEMLOC_TYPE_HSTACK;
	loc->ml_value.v_hstack.s_cfa = cfa_offset;
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_vpush_hstackind(struct Dee_memstate *__restrict self,
                             uintptr_t cfa_offset, ptrdiff_t val_delta) {
	struct Dee_memloc *loc;
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	loc = &self->ms_stackv[self->ms_stackc];
	loc->ml_flags = MEMLOC_F_NOREF;
	loc->ml_type  = MEMLOC_TYPE_HSTACKIND;
	loc->ml_value.v_hstack.s_cfa = cfa_offset;
	loc->ml_value.v_hstack.s_off = val_delta;
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_memstate_vdup_n(struct Dee_memstate *__restrict self, size_t n) {
	struct Dee_memloc *dst_loc;
	size_t index;
	ASSERT(n >= 1);
	if (OVERFLOW_USUB(self->ms_stackc, n, &index))
		return err_illegal_stack_effect();
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	dst_loc = &self->ms_stackv[self->ms_stackc];
	*dst_loc = self->ms_stackv[index];
	dst_loc->ml_flags |= MEMLOC_F_NOREF; /* alias! (so no reference) */
	if (MEMLOC_TYPE_HASREG(dst_loc->ml_type))
		Dee_memstate_incrinuse(self, dst_loc->ml_value.v_hreg.r_regno);
	++self->ms_stackc;
	return 0;
err:
	return -1;
}




/* Initialize `self' from `state' (with the exception of `self->exi_block')
 * @return: 0 : Success
 * @return: -1: Error (you're holding a reference to an argument/constant; why?) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_except_exitinfo_init(struct Dee_except_exitinfo *__restrict self,
                         struct Dee_memstate *__restrict state) {
	size_t i;
	self->exi_cfa_offset = state->ms_host_cfa_offset;
	bzero(&self->exi_regs,
	      ((offsetof(struct Dee_except_exitinfo, exi_stack) -
	        offsetof(struct Dee_except_exitinfo, exi_regs)) +
	       (self->exi_cfa_offset / HOST_SIZEOF_POINTER) * sizeof(uint16_t)));
	for (i = 0; i < state->ms_localc; ++i) {
		struct Dee_memloc *loc = &state->ms_localv[i];
		if (loc->ml_flags & (MEMLOC_F_NOREF | MEMLOC_F_LOCAL_UNBOUND))
			continue;
		switch (loc->ml_type) {

		case MEMLOC_TYPE_HSTACKIND: {
			size_t index;
			if unlikely(loc->ml_value.v_hstack.s_off != 0)
				goto err_bad_loc;
			index = Dee_except_exitinfo_cfa2index(loc->ml_value.v_hstack.s_cfa);
			ASSERT(index < (self->exi_cfa_offset / HOST_SIZEOF_POINTER));
			++self->exi_stack[index];
		}	break;

		case MEMLOC_TYPE_HREG:
			ASSERT(loc->ml_value.v_hreg.r_regno < HOST_REGISTER_COUNT);
			if unlikely(loc->ml_value.v_hreg.r_off != 0)
				goto err_bad_loc;
			++self->exi_regs[loc->ml_value.v_hreg.r_regno];
			break;

		case MEMLOC_TYPE_UNALLOC:
			break;

		default: goto err_bad_loc;
		}
	}
	for (i = 0; i < state->ms_stackc; ++i) {
		struct Dee_memloc *loc = &state->ms_stackv[i];
		if (loc->ml_flags & MEMLOC_F_NOREF)
			continue;
		switch (loc->ml_type) {

		case MEMLOC_TYPE_HSTACKIND: {
			size_t index;
			if unlikely(loc->ml_value.v_hstack.s_off != 0)
				goto err_bad_loc;
			index = Dee_except_exitinfo_cfa2index(loc->ml_value.v_hstack.s_cfa);
			ASSERT(index < (self->exi_cfa_offset / HOST_SIZEOF_POINTER));
			++self->exi_stack[index];
		}	break;

		case MEMLOC_TYPE_HREG:
			ASSERT(loc->ml_value.v_hreg.r_regno < HOST_REGISTER_COUNT);
			if unlikely(loc->ml_value.v_hreg.r_off != 0)
				goto err_bad_loc;
			++self->exi_regs[loc->ml_value.v_hreg.r_regno];
			break;

		default: goto err_bad_loc;
		}
	}
	return 0;
err_bad_loc:
	return DeeError_Throwf(&DeeError_IllegalInstruction,
	                       "Cannot jump to exception handler while holding "
	                       "a reference to such a complex object location");
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_MEMSTATE_C */
