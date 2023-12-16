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

/* Check if there is a register that contains `usage'.
 * Returns some value `>= HOST_REGISTER_COUNT' if non-existent. */
INTERN ATTR_PURE WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_memstate_hregs_find_usage(struct Dee_memstate const *__restrict self,
                              Dee_host_regusage_t usage) {
	Dee_host_register_t result;
	for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
		if (self->ms_regs[result] == usage)
			break;
	}
	return result;
}

/* Check if there is a register that is completely unused.
 * Returns some value `>= HOST_REGISTER_COUNT' if non-existent.
 * @param: accept_if_with_regusage: When true, allowed to return registers with
 *                                  `ms_regs[return] != REGISTER_USAGE_GENERIC' */
INTERN ATTR_PURE WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_memstate_hregs_find_unused(struct Dee_memstate const *__restrict self,
                               bool accept_if_with_regusage) {
	size_t i;
	Dee_host_register_t result;
	BITSET(HOST_REGISTER_COUNT) used;
	bzero(&used, sizeof(used));
	for (i = 0; i < self->ms_stackc; ++i) {
		struct Dee_memloc const *loc = &self->ms_stackv[i];
		if (loc->ml_where == MEMLOC_TYPE_HREG) {
			ASSERT(loc->ml_value.ml_hreg < HOST_REGISTER_COUNT);
			BITSET_TURNON(&used, loc->ml_value.ml_hreg);
		}
	}
	for (i = 0; i < self->ms_localc; ++i) {
		struct Dee_memloc const *loc = &self->ms_localv[i];
		if (loc->ml_where == MEMLOC_TYPE_HREG) {
			ASSERT(loc->ml_value.ml_hreg < HOST_REGISTER_COUNT);
			BITSET_TURNON(&used, loc->ml_value.ml_hreg);
		}
	}
	if (!accept_if_with_regusage) {
		for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
			if (self->ms_regs[result] != REGISTER_USAGE_GENERIC)
				BITSET_TURNON(&used, result);
		}
	} else {
		/* Even when other usage registers can be re-used, try not to do so unless necessary. */
		for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
			if (!BITSET_GET(&used, result) && self->ms_regs[result] == REGISTER_USAGE_GENERIC)
				goto done;
		}
	}
	for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
		if (!BITSET_GET(&used, result))
			break; /* Found an unused register! */
	}
done:
	return result;
}

/* Same as `Dee_memstate_hregs_find_unused(self, true)', but don't return `not_these',
 * which is an array of register numbers terminated by one `>= HOST_REGISTER_COUNT'.
 * Returns some value `>= HOST_REGISTER_COUNT' if non-existent. */
INTERN WUNUSED NONNULL((1)) Dee_host_register_t DCALL
Dee_memstate_hregs_find_unused_ex(struct Dee_memstate *__restrict self,
                                  Dee_host_register_t const *not_these) {
	size_t i;
	Dee_host_register_t result;
	BITSET(HOST_REGISTER_COUNT) used;
	bzero(&used, sizeof(used));
	for (i = 0; i < self->ms_stackc; ++i) {
		struct Dee_memloc const *loc = &self->ms_stackv[i];
		if (loc->ml_where == MEMLOC_TYPE_HREG) {
			ASSERT(loc->ml_value.ml_hreg < HOST_REGISTER_COUNT);
			BITSET_TURNON(&used, loc->ml_value.ml_hreg);
		}
	}
	for (i = 0; i < self->ms_localc; ++i) {
		struct Dee_memloc const *loc = &self->ms_localv[i];
		if (loc->ml_where == MEMLOC_TYPE_HREG) {
			ASSERT(loc->ml_value.ml_hreg < HOST_REGISTER_COUNT);
			BITSET_TURNON(&used, loc->ml_value.ml_hreg);
		}
	}

	/* If specified, exclude certain registers. */
	if (not_these != NULL) {
		for (i = 0; (result = not_these[i]) < HOST_REGISTER_COUNT; ++i)
			BITSET_TURNON(&used, result);
	}

	/* Even when other usage registers can be re-used, try not to do so unless necessary. */
	for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
		if (!BITSET_GET(&used, result) && self->ms_regs[result] == REGISTER_USAGE_GENERIC)
			goto done;
	}
	for (result = 0; result < HOST_REGISTER_COUNT; ++result) {
		if (!BITSET_GET(&used, result))
			break; /* Found an unused register! */
	}
done:
	return result;
}


PRIVATE WUNUSED NONNULL((1)) bool DCALL
Dee_memloc_hstack_used(struct Dee_memloc const *__restrict self,
                       uintptr_t min_offset, uintptr_t end_offset) {
	if (self->ml_where == MEMLOC_TYPE_HSTACK) {
		if (self->ml_value.ml_hstack >= min_offset &&
		    self->ml_value.ml_hstack < end_offset)
			return true;
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
			uintptr_t min_offset, end_offset;
			min_offset = i * HOST_SIZEOF_POINTER;
#ifdef HOSTASM_STACK_GROWS_DOWN
			end_offset = min_offset;
			min_offset -= n_bytes;
#else /* HOSTASM_STACK_GROWS_DOWN */
			end_offset = min_offset + n_bytes;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
			if (Dee_memstate_hstack_unused(self, min_offset, end_offset)) {
#ifdef HOSTASM_STACK_GROWS_DOWN
				min_offset += n_bytes;
#endif /* HOSTASM_STACK_GROWS_DOWN */
				return min_offset;
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
#ifdef HOSTASM_STACK_GROWS_DOWN
		uintptr_t end_offset = (a_pointers - 1) * HOST_SIZEOF_POINTER;
		uintptr_t min_offset = end_offset - HOST_SIZEOF_POINTER;
#else /* HOSTASM_STACK_GROWS_DOWN */
		uintptr_t min_offset = (a_pointers - 1) * HOST_SIZEOF_POINTER;
		uintptr_t end_offset = min_offset + HOST_SIZEOF_POINTER;
#endif /* !HOSTASM_STACK_GROWS_DOWN */
		if (!Dee_memstate_hstack_unused(self, min_offset, end_offset))
			break;
		self->ms_host_cfa_offset -= HOST_SIZEOF_POINTER;
		result = true;
	}
	return false;
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) void DCALL
Dee_memloc_constrainwith(struct Dee_memstate *__restrict state,
                         struct Dee_memloc *self,
                         struct Dee_memloc const *__restrict other) {
	/* Quick check: are locations identical? */
	if (Dee_memloc_sameloc(self, other))
		return;
	switch (self->ml_where) {

	case MEMLOC_TYPE_CONST:
	case MEMLOC_TYPE_ARG: {
		/* On one side it's an argument or a constant, and on the other
		 * side it's something else.
		 * In this case, need to convert to a register/stack location. */
		Dee_host_register_t regno;
		regno = Dee_memstate_hregs_find_unused(state, true);
		if (regno < HOST_REGISTER_COUNT) {
			self->ml_where = MEMLOC_TYPE_HREG;
			self->ml_value.ml_hreg = regno;
		} else {
			/* Use a stack location. */
			uintptr_t cfa_offset;
			cfa_offset = Dee_memstate_hstack_find(state, HOST_SIZEOF_POINTER);
			if (cfa_offset == (uintptr_t)-1)
				cfa_offset = Dee_memstate_hstack_alloca(state, HOST_SIZEOF_POINTER);
			self->ml_where = MEMLOC_TYPE_HSTACK;
			self->ml_value.ml_hstack = cfa_offset;
		}
	}	break;

	default:
		break;
	}
}

/* Constrain `self' with `other', such that it is possible to generate code to
 * transition from `other' to `self', as well as any other mem-state that might
 * be the result of further constraints applied to `self'. */
INTERN WUNUSED NONNULL((1, 2)) void DCALL
Dee_memstate_constrainwith(struct Dee_memstate *__restrict self,
                           struct Dee_memstate const *__restrict other) {
	uint16_t i;
	Dee_host_register_t regno;
	ASSERT(self->ms_stackc == other->ms_stackc);

	/* Always use the largest host-stack size. */
	if (self->ms_host_cfa_offset < other->ms_host_cfa_offset)
		self->ms_host_cfa_offset = other->ms_host_cfa_offset;

	/* Mark usage registers as undefined if different between blocks. */
	for (regno = 0; regno < HOST_REGISTER_COUNT; ++regno) {
		if (self->ms_regs[regno] != other->ms_regs[regno])
			self->ms_regs[regno] = REGISTER_USAGE_GENERIC;
	}

	/* Merge stack/locals memory locations. */
	for (i = 0; i < self->ms_stackc; ++i)
		Dee_memloc_constrainwith(self, &self->ms_stackv[i], &other->ms_stackv[i]);
	for (i = 0; i < self->ms_localc; ++i)
		Dee_memloc_constrainwith(self, &self->ms_localv[i], &other->ms_localv[i]);
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
		             self->ms_stackc - (n - 1), sizeof(struct Dee_memloc));
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
		           self->ms_stackc - (n - 1), sizeof(struct Dee_memloc));
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
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_memstate_vpush_const(struct Dee_memstate *__restrict self, DeeObject *value) {
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	self->ms_stackv[self->ms_stackc].ml_flags = MEMLOC_F_NOREF;
	self->ms_stackv[self->ms_stackc].ml_where = MEMLOC_TYPE_CONST;
	self->ms_stackv[self->ms_stackc].ml_value.ml_const = value;
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_memstate_vpush_arg(struct Dee_memstate *__restrict self, uint16_t aid) {
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	self->ms_stackv[self->ms_stackc].ml_flags = MEMLOC_F_NOREF;
	self->ms_stackv[self->ms_stackc].ml_where = MEMLOC_TYPE_ARG;
	self->ms_stackv[self->ms_stackc].ml_value.ml_harg = aid;
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_memstate_vdup_n(struct Dee_memstate *__restrict self, size_t n) {
	size_t index;
	ASSERT(n >= 1);
	if (OVERFLOW_USUB(self->ms_stackc, n, &index))
		return err_illegal_stack_effect();
	if unlikely(self->ms_stackc >= self->ms_stacka &&
	            Dee_memstate_reqvstack(self, self->ms_stackc + 1))
		goto err;
	self->ms_stackv[self->ms_stackc] = self->ms_stackv[index];
	self->ms_stackv[self->ms_stackc].ml_flags |= MEMLOC_F_NOREF; /* alias! (so no reference) */
	++self->ms_stackc;
	return 0;
err:
	return -1;
}

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_MEMSTATE_C */
