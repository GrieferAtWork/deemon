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
#ifndef GUARD_DEX_HOSTASM_GENERATOR_ARCH_C
#define GUARD_DEX_HOSTASM_GENERATOR_ARCH_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/tuple.h>

#include <hybrid/sequence/bitset.h>
#include <hybrid/unaligned.h>

#ifdef HOSTASM_X86
#include "libgen86/gen.h"
#include "libgen86/register.h"
#endif /* HOSTASM_X86 */

DECL_BEGIN

#define Dee_basic_block_reqx86(self, n_instructions) \
	Dee_basic_block_reqhost(self, GEN86_INSTRLEN_MAX * (n_instructions))

PRIVATE uint8_t const gen86_registers[HOST_REGISTER_COUNT] = {
#ifdef HOSTASM_X86_64_MSABI
	GEN86_R_RAX,
	GEN86_R_RCX,
	GEN86_R_RDX,
	GEN86_R_R8,
	GEN86_R_R9,
	GEN86_R_R10,
	GEN86_R_R11,
#elif defined(HOSTASM_X86_64_SYSVABI)
	GEN86_R_RAX,
	GEN86_R_RCX,
	GEN86_R_RDX,
	GEN86_R_RDI,
	GEN86_R_RSI,
	GEN86_R_R8,
	GEN86_R_R9,
	GEN86_R_R10,
	GEN86_R_R11,
#elif defined(HOSTASM_X86)
	GEN86_R_EAX,
	GEN86_R_ECX,
	GEN86_R_EDX,
#endif /* ... */
};

#define p_pc(block)  (&(block)->bb_host_end)
#define p_off(block) (uintptr_t)((block)->bb_host_end - (block)->bb_host_start)


/* Object reference count incref/decref */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_gincref_reg(struct Dee_basic_block *__restrict self,
                             Dee_host_register_t regno) {
	if unlikely(Dee_basic_block_reqx86(self, 2))
		goto err;
	gen86_lock(p_pc(self));
	gen86_incP_mod(p_pc(self), gen86_modrm_db,
	               offsetof(DeeObject, ob_refcnt),
	               gen86_registers[regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_gdecref_reg_nokill(struct Dee_basic_block *__restrict self,
                                    Dee_host_register_t regno) {
	if unlikely(Dee_basic_block_reqx86(self, 2))
		goto err;
	gen86_lock(p_pc(self));
	gen86_decP_mod(p_pc(self), gen86_modrm_db,
	               offsetof(DeeObject, ob_refcnt),
	               gen86_registers[regno]);
	return 0;
err:
	return -1;
}

#if !defined(CONFIG_NO_BADREFCNT_CHECKS) || defined(CONFIG_TRACE_REFCHANGES)
DFUNDEF NONNULL((1)) void (DCALL DeeObject_Destroy)(DeeObject *__restrict self);
#endif /* !CONFIG_NO_BADREFCNT_CHECKS || CONFIG_TRACE_REFCHANGES */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_gdestroy_reg(struct Dee_function_generator *__restrict self,
                                     struct Dee_basic_block *block,
                                     Dee_host_register_t regno) {
	struct Dee_memstate *state = self->fg_state;
	BITSET(HOST_REGISTER_COUNT) used_regs;
	Dee_host_register_t save_regno;
	uint16_t i;

	/* Figure out all registers that are currently in use (because
	 * the call to `DeeObject_Destory()' might clobber them). Note
	 * that we push and then later pop those registers! */
	bzero(&used_regs, sizeof(used_regs));
	for (i = 0; i < state->ms_localc; ++i) {
		struct Dee_memloc const *loc = &state->ms_localv[i];
		if (loc->ml_where == MEMLOC_TYPE_HREG) {
			ASSERT(loc->ml_value.ml_hreg < HOST_REGISTER_COUNT);
			BITSET_TURNON(&used_regs, loc->ml_value.ml_hreg);
		}
	}
	for (i = 0; i < state->ms_stackc; ++i) {
		struct Dee_memloc const *loc = &state->ms_stackv[i];
		if (loc->ml_where == MEMLOC_TYPE_HREG) {
			ASSERT(loc->ml_value.ml_hreg < HOST_REGISTER_COUNT);
			BITSET_TURNON(&used_regs, loc->ml_value.ml_hreg);
		}
	}

	/* The caller-given register never needs to be preserved.
	 * Since this is the path where the object gets destroyed,
	 * there isn't any reason to save its address somewhere! */
	BITSET_TURNOFF(&used_regs, regno);

	/* Save registers. */
	for (save_regno = 0; save_regno < HOST_REGISTER_COUNT; ++save_regno) {
		if (!BITSET_GET(&used_regs, save_regno))
			continue;
		if unlikely(Dee_basic_block_reqx86(block, 1))
			goto err;
		gen86_pushP_r(p_pc(block), gen86_registers[save_regno]);
		Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
	}

	/* Load `regno' as argument for the call to `DeeObject_Destroy()' */
#ifdef HOST_REGISTER_R_ARG0
	if (regno != HOST_REGISTER_R_ARG0) {
		if unlikely(Dee_basic_block_reqx86(block, 1))
			goto err;
		gen86_movP_r_r(p_pc(block),
		               gen86_registers[regno],
		               gen86_registers[HOST_REGISTER_R_ARG0]);
	}
#else /* HOST_REGISTER_R_ARG0 */
	if unlikely(Dee_basic_block_reqx86(block, 1))
		goto err;
	gen86_pushP_r(p_pc(block), gen86_registers[regno]);
	Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
#endif /* !HOST_REGISTER_R_ARG0 */

	/* Make the call to `DeeObject_Destroy()' */
	{
		struct Dee_host_reloc *rel;
#ifdef HOSTASM_X86_64_MSABI
		if unlikely(Dee_basic_block_reqx86(block, 3))
			goto err;
		gen86_subP_imm_r(p_pc(block), 32, GEN86_R_PSP);
#else /* HOSTASM_X86_64_MSABI */
		if unlikely(Dee_basic_block_reqx86(block, 1))
			goto err;
#endif /* !HOSTASM_X86_64_MSABI */
		rel = Dee_basic_block_newhostrel(block);
		if unlikely(!rel)
			goto err;
		gen86_calll_offset(p_pc(block), -4);
		rel->hr_offset = p_off(block) - 4;
		rel->hr_type   = DEE_HOST_RELOC_PTREL32;
		rel->hr_value.hr_pt = (void *)&DeeObject_Destroy;
#ifdef HOSTASM_X86_64_MSABI
		gen86_addP_imm_r(p_pc(block), 32, GEN86_R_PSP);
#endif /* !HOSTASM_X86_64_MSABI */
	}

	/* Account for the fact that `DeeObject_Destroy()' pops its own arguments on i386 */
#ifndef HOST_REGISTER_R_ARG0
	Dee_function_generator_gadjust_cfa_offset(self, -HOST_SIZEOF_POINTER);
#endif /* !HOST_REGISTER_R_ARG0 */

	/* After calling an external function, usage registers may have gotten clobbered. */
	Dee_memstate_hregs_clear_usage(self->fg_state);

	/* Restore registers (in reverse order). */
	save_regno = HOST_REGISTER_COUNT;
	while (save_regno) {
		--save_regno;
		if (!BITSET_GET(&used_regs, save_regno))
			continue;
		if unlikely(Dee_basic_block_reqx86(block, 1))
			goto err;
		gen86_popP_r(p_pc(block), gen86_registers[save_regno]);
		Dee_function_generator_gadjust_cfa_offset(self, -HOST_SIZEOF_POINTER);
	}

	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gdecref_reg(struct Dee_function_generator *__restrict self,
                                    Dee_host_register_t regno) {
	struct Dee_basic_block *cold_block = self->fg_assembler->fa_cold_block;
	struct Dee_basic_block *block = self->fg_block;
	if unlikely(Dee_basic_block_reqx86(block, 2))
		goto err;
	gen86_lock(p_pc(block));
	gen86_decP_mod(p_pc(block), gen86_modrm_db,
	               offsetof(DeeObject, ob_refcnt),
	               gen86_registers[regno]);

	/* NOTE: decP sets FLAGS.Z=1 when the reference counter becomes `0'. */
	if (cold_block != NULL) {
		struct Dee_host_reloc *enter_rel;
		struct Dee_host_reloc *leave_rel;
		int32_t delta;

		/* Generate code that jumps into the cold block when the reference counter became `0'. */
		delta = -4 + p_off(cold_block);
		gen86_jccl_offset(p_pc(block), GEN86_CC_Z, delta);
		enter_rel = Dee_basic_block_newhostrel(block);
		if unlikely(!enter_rel)
			goto err;
		enter_rel->hr_offset      = p_off(block);
		enter_rel->hr_type        = DEE_HOST_RELOC_BBREL32;
		enter_rel->hr_value.hr_bb = cold_block;

		/* Generate the destroy-code *within* the cold block. */
		if unlikely(_Dee_function_generator_gdestroy_reg(self, cold_block, regno))
			goto err;

		/* Generate code to jump from the cold block back to the normal block. */
		if unlikely(Dee_basic_block_reqx86(cold_block, 1))
			goto err;
		delta = -4 + p_off(block);
		gen86_jmpl_offset(p_pc(cold_block), delta);
		leave_rel = Dee_basic_block_newhostrel(cold_block);
		if unlikely(!leave_rel)
			goto err;
		leave_rel->hr_offset      = p_off(cold_block);
		leave_rel->hr_type        = DEE_HOST_RELOC_BBREL32;
		leave_rel->hr_value.hr_bb = block;
	} else {
		uintptr_t jcc8_offset;
		gen86_jcc8_offset(p_pc(block), GEN86_CC_NZ, -1);
		jcc8_offset = p_off(block) - 1;
		if unlikely(_Dee_function_generator_gdestroy_reg(self, block, regno))
			goto err;
		/* Fill in the delta for the non-zero-jump above. */
		*(int8_t *)(block->bb_host_start + jcc8_offset) += (int8_t)(p_off(block) - jcc8_offset);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_gincref_const(struct Dee_basic_block *__restrict self,
                               DeeObject *value) {
	if unlikely(Dee_basic_block_reqx86(self, 2))
		goto err;
	gen86_lock(p_pc(self));
	gen86_incP_mod(p_pc(self), gen86_modrm_d, (intptr_t)(uintptr_t)&value->ob_refcnt); /* TODO: movabs */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_gdecref_const(struct Dee_basic_block *__restrict self,
                               DeeObject *value) {
	if unlikely(Dee_basic_block_reqx86(self, 2))
		goto err;
	/* Constants can never be destroyed, so decref'ing one is
	 * like `Dee_DecrefNoKill()' (iow: doesn't need a zero-check) */
	gen86_lock(p_pc(self));
	gen86_decP_mod(p_pc(self), gen86_modrm_d, (intptr_t)(uintptr_t)&value->ob_refcnt); /* TODO: movabs */
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_gxincref_reg(struct Dee_basic_block *__restrict self,
                              Dee_host_register_t regno) {
	uint8_t reg86;
	uintptr_t jcc8_offset;
	if unlikely(Dee_basic_block_reqx86(self, 2))
		goto err;
	reg86 = gen86_registers[regno];
	gen86_testP_r_r(p_pc(self), reg86, reg86);
	gen86_jcc8_offset(p_pc(self), GEN86_CC_Z, -1);
	jcc8_offset = p_off(self) - 1;
	if unlikely(_Dee_basic_block_gincref_reg(self, regno))
		goto err;
	/* Fix-up the jump */
	*(int8_t *)(self->bb_host_start + jcc8_offset) += (int8_t)(p_off(self) - jcc8_offset);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gxdecref_reg(struct Dee_function_generator *__restrict self,
                                     Dee_host_register_t regno) {
	struct Dee_basic_block *block = self->fg_block;
	uint8_t reg86;
	uintptr_t jcc8_offset;
	if unlikely(Dee_basic_block_reqx86(block, 2))
		goto err;
	reg86 = gen86_registers[regno];
	gen86_testP_r_r(p_pc(block), reg86, reg86);
	gen86_jcc8_offset(p_pc(block), GEN86_CC_Z, -1);
	jcc8_offset = p_off(block) - 1;
	if unlikely(_Dee_function_generator_gdecref_reg(self, regno))
		goto err;
	/* Fix-up the jump */
	*(int8_t *)(block->bb_host_start + jcc8_offset) += (int8_t)(p_off(block) - jcc8_offset);
	return 0;
err:
	return -1;
}



/* Controls for operating with R/W-locks (as needed for accessing global/extern variables) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_read(struct Dee_function_generator *__restrict self,
                                     Dee_atomic_rwlock_t *__restrict lock) {
	/* TODO */
	(void)self;
	(void)lock;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_write(struct Dee_function_generator *__restrict self,
                                      Dee_atomic_rwlock_t *__restrict lock) {
	/* TODO */
	(void)self;
	(void)lock;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_endwrite(struct Dee_function_generator *__restrict self,
                                         Dee_atomic_rwlock_t *__restrict lock) {
	/* TODO */
	(void)self;
	(void)lock;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_function_generator_grwlock_endread(struct Dee_function_generator *__restrict self,
                                        Dee_atomic_rwlock_t *__restrict lock) {
	/* TODO */
	(void)self;
	(void)lock;
	return DeeError_NOTIMPLEMENTED();
}


/* Allocate/deallocate memory from the host stack.
 * If stack memory gets allocated, zero-initialize it. */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_ghstack_adjust(struct Dee_basic_block *__restrict self,
                                ptrdiff_t sp_delta) {
	if (sp_delta < 0) {
		/* Release stack memory. */
		if unlikely(Dee_basic_block_reqx86(self, 1))
			goto err;
		gen86_addP_imm_r(p_pc(self), (-sp_delta), GEN86_R_PSP);
	} else if (sp_delta > 0) {
		/* Acquire stack memory. */
		size_t n_pointers = (uintptr_t)sp_delta / HOST_SIZEOF_POINTER;
		ASSERT(IS_ALIGNED((uintptr_t)sp_delta, HOST_SIZEOF_POINTER));
		ASSERT(n_pointers >= 1);
		/* Generate a bunch of `pushP $0' instructions. */
		if unlikely(Dee_basic_block_reqx86(self, n_pointers))
			goto err;
		do {
			gen86_pushP_imm(p_pc(self), 0);
		} while (--n_pointers);
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_ghstack_pushreg(struct Dee_basic_block *__restrict self,
                                 Dee_host_register_t src_regno) {
	if unlikely(Dee_basic_block_reqx86(self, 1))
		goto err;
	gen86_pushP_r(p_pc(self), gen86_registers[src_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_ghstack_pushconst(struct Dee_basic_block *__restrict self,
                                   DeeObject *value) {
	if unlikely(Dee_basic_block_reqx86(self, 1))
		goto err;
	gen86_pushP_imm(p_pc(self), (intptr_t)(uintptr_t)value); /* TODO: movabs */
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_ghstack_pushhstack(struct Dee_basic_block *__restrict self,
                                    ptrdiff_t sp_offset) {
	if unlikely(Dee_basic_block_reqx86(self, 1))
		goto err;
	gen86_pushP_mod(p_pc(self), gen86_modrm_db, sp_offset, GEN86_R_PSP);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_ghstack_popreg(struct Dee_basic_block *__restrict self,
                                Dee_host_register_t dst_regno) {
	if unlikely(Dee_basic_block_reqx86(self, 1))
		goto err;
	gen86_popP_r(p_pc(self), gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_gmov_reg2hstack(struct Dee_basic_block *__restrict self,
                                 Dee_host_register_t src_regno, ptrdiff_t sp_offset) {
	if unlikely(Dee_basic_block_reqx86(self, 1))
		goto err;
	gen86_movP_r_db(p_pc(self), gen86_registers[src_regno], sp_offset, GEN86_R_PSP);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_gmov_hstack2reg(struct Dee_basic_block *__restrict self,
                                 ptrdiff_t sp_offset, Dee_host_register_t dst_regno) {
	if unlikely(Dee_basic_block_reqx86(self, 1))
		goto err;
	gen86_movP_db_r(p_pc(self), sp_offset, GEN86_R_PSP, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_gmov_const2reg(struct Dee_basic_block *__restrict self,
                                DeeObject *value, Dee_host_register_t dst_regno) {
	if unlikely(Dee_basic_block_reqx86(self, 1))
		goto err;
	if (value == NULL) {
		gen86_xorP_r_r(p_pc(self), gen86_registers[dst_regno], gen86_registers[dst_regno]);
	} else {
		gen86_movP_imm_r(p_pc(self), (intptr_t)(uintptr_t)value, gen86_registers[dst_regno]); /* TODO: movabs */
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_gmov_reg2reg(struct Dee_basic_block *__restrict self,
                              Dee_host_register_t src_regno,
                              Dee_host_register_t dst_regno) {
	if unlikely(src_regno == dst_regno)
		return 0;
	if unlikely(Dee_basic_block_reqx86(self, 1))
		goto err;
	gen86_movP_r_r(p_pc(self), gen86_registers[src_regno], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_gmov_constind2reg(struct Dee_basic_block *__restrict self,
                                   DeeObject **p_value, Dee_host_register_t dst_regno) {
	if unlikely(Dee_basic_block_reqx86(self, 1))
		goto err;
	gen86_movP_d_r(p_pc(self), (intptr_t)(uintptr_t)p_value, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_gmov_reg2constind(struct Dee_basic_block *__restrict self,
                                   Dee_host_register_t src_regno, DeeObject **p_value) {
	if unlikely(Dee_basic_block_reqx86(self, 1))
		goto err;
	gen86_movP_r_d(p_pc(self), gen86_registers[src_regno], (intptr_t)(uintptr_t)p_value);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_gmov_regind2reg(struct Dee_basic_block *__restrict self,
                                 Dee_host_register_t src_regno, ptrdiff_t src_delta,
                                 Dee_host_register_t dst_regno) {
	if unlikely(src_regno == dst_regno)
		return 0;
	if unlikely(Dee_basic_block_reqx86(self, 1))
		goto err;
	gen86_movP_db_r(p_pc(self), src_delta, gen86_registers[src_regno], gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_basic_block_gmov_reg2regind(struct Dee_basic_block *__restrict self,
                                 Dee_host_register_t src_regno,
                                 Dee_host_register_t dst_regno, ptrdiff_t dst_delta) {
	if unlikely(src_regno == dst_regno)
		return 0;
	if unlikely(Dee_basic_block_reqx86(self, 1))
		goto err;
	gen86_movP_r_db(p_pc(self), gen86_registers[src_regno], dst_delta, gen86_registers[dst_regno]);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gmov_truearg2reg(struct Dee_function_generator *__restrict self,
                                         size_t argno, Dee_host_register_t dst_regno) {
	size_t sp_offset;
	sp_offset = self->fg_state->ms_host_cfa_offset; /* Offset to return-pc */
#if defined(HOSTASM_X86_64) && defined(HOSTASM_X86_64_SYSVABI)
	/* This matches the way `Dee_function_assembler_compileblocks()' saves registers. */
	{
		Dee_hostfunc_cc_t cc = self->fg_assembler->fa_cc;
		size_t trueargc = cc & HOSTFUNC_CC_F_TUPLE ? 1 : 2;
		if (cc & HOSTFUNC_CC_F_THIS)
			++trueargc;
		if (cc & HOSTFUNC_CC_F_KW)
			++trueargc;
		sp_offset -= trueargc * HOST_SIZEOF_POINTER; /* Base address of saved register arguments */
		sp_offset += argno * HOST_SIZEOF_POINTER;    /* Load the relevant argument */
	}
#else /* HOSTASM_X86_64 */
	sp_offset += HOST_SIZEOF_POINTER;               /* Skip over return-pc */
	sp_offset += argno * HOST_SIZEOF_POINTER;       /* Load the relevant argument */
#endif /* !HOSTASM_X86_64 */
	return _Dee_function_generator_gmov_hstack2reg(self, sp_offset, dst_regno);
}

/* Load special runtime values into `dst_regno' */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gmov_usage2reg(struct Dee_function_generator *__restrict self,
                                       Dee_host_regusage_t usage,
                                       Dee_host_register_t dst_regno) {
	Dee_hostfunc_cc_t cc = self->fg_assembler->fa_cc;
	switch (usage) {
	case REGISTER_USAGE_THIS:
		if (!(cc & HOSTFUNC_CC_F_THIS))
			break;
		return _Dee_function_generator_gmov_truearg2reg(self, 0, dst_regno);

	case REGISTER_USAGE_ARGC: {
		size_t offset = 0;
		if (cc & HOSTFUNC_CC_F_TUPLE)
			break;
		if (cc & HOSTFUNC_CC_F_THIS)
			++offset;
		return _Dee_function_generator_gmov_truearg2reg(self, offset, dst_regno);
	}	break;

	case REGISTER_USAGE_ARGV: {
		size_t offset = 1;
		if (cc & HOSTFUNC_CC_F_TUPLE)
			break;
		if (cc & HOSTFUNC_CC_F_THIS)
			++offset;
		return _Dee_function_generator_gmov_truearg2reg(self, offset, dst_regno);
	}	break;

	case REGISTER_USAGE_ARGS: {
		size_t offset = 0;
		if (!(cc & HOSTFUNC_CC_F_TUPLE))
			break;
		if (cc & HOSTFUNC_CC_F_THIS)
			++offset;
		return _Dee_function_generator_gmov_truearg2reg(self, offset, dst_regno);
	}	break;

	case REGISTER_USAGE_KW: {
		size_t offset = 1;
		if (!(cc & HOSTFUNC_CC_F_KW))
			break;
		if (!(cc & HOSTFUNC_CC_F_TUPLE))
			++offset;
		if (cc & HOSTFUNC_CC_F_THIS)
			++offset;
		return _Dee_function_generator_gmov_truearg2reg(self, offset, dst_regno);
	}	break;

	default: break;
	}
	return DeeError_Throwf(&DeeError_IllegalInstruction,
	                       "Unsupported register usage %" PRFu8,
	                       (uint8_t)usage);
}

INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gret(struct Dee_function_generator *__restrict self) {
	struct Dee_basic_block *block = self->fg_block;
	if unlikely(Dee_basic_block_reqx86(block, 1))
		goto err;
#ifdef HOSTASM_X86_64
	gen86_retP(p_pc(block));
#else /* HOSTASM_X86_64 */
	/* Special handling needed because DCALL is STDCALL on i386 */
	{
		Dee_hostfunc_cc_t cc = self->fg_assembler->fa_cc;
		size_t ret_imm = cc & HOSTFUNC_CC_F_TUPLE ? 4 : 8;
		if (cc & HOSTFUNC_CC_F_THIS)
			ret_imm += 4;
		if (cc & HOSTFUNC_CC_F_KW)
			ret_imm += 4;
		gen86_retP_imm(p_pc(block), ret_imm);
	}
#endif /* !HOSTASM_X86_64 */
	return 0;
err:
	return -1;
}



/* Generate a call to a C-function `c_function' with `argc'
 * pointer-sized arguments whose values are taken from `argv'.
 * NOTE: The given `c_function' is assumed to use the `DCALL' calling convention. */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_function_generator_gcall_c_function(struct Dee_function_generator *__restrict self,
                                         void *c_function, size_t argc,
                                         struct Dee_memloc const *argv) {
#ifdef HOSTASM_X86_64
	/* TODO */
	(void)self;
	(void)c_function;
	(void)argc;
	(void)argv;
	return DeeError_NOTIMPLEMENTED();
#else /* HOSTASM_X86_64 */
	struct Dee_basic_block *block = self->fg_block;
	size_t argi;
	/* MEMLOC_TYPE_ARG-type arguments with index >= this must be filled after-the-fact */
	size_t fill_arg_args_later;

	/* Push arguments onto the host stack in reverse order. */
	argi = argc;
	fill_arg_args_later = argc;
	while (argi) {
		struct Dee_memloc const *arg;
		--argi;
		arg = &argv[argi];
		switch (arg->ml_where) {
		case MEMLOC_TYPE_HSTACK:
			if unlikely(Dee_function_generator_ghstack_pushhstack(self, arg->ml_value.ml_hstack))
				goto err;
			break;
		case MEMLOC_TYPE_HREG:
			if unlikely(Dee_function_generator_ghstack_pushreg(self, arg->ml_value.ml_hreg))
				goto err;
			break;
		case MEMLOC_TYPE_ARG: {
			/* Must load the `argv' vector, for which we need a temp register.
			 * However, we mustn't use registers needed by later argument pushs for this! */
			uint16_t aid = arg->ml_value.ml_harg;
			struct Dee_memstate *state = self->fg_state;
			Dee_host_register_t temp_regno;
			BITSET(HOST_REGISTER_COUNT) used_regs;
			size_t future_argi;
			if unlikely(Dee_basic_block_reqx86(block, 1))
				goto err;
			temp_regno = Dee_memstate_hregs_find_usage(state, REGISTER_USAGE_ARGV);
			if (temp_regno < HOST_REGISTER_COUNT) {
				ptrdiff_t offset;
do_load_arg_with_argv_regno:
				offset = (ptrdiff_t)aid * HOST_SIZEOF_POINTER;
				gen86_pushP_mod(p_pc(block), gen86_modrm_db, offset, gen86_registers[temp_regno]);
				Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
				break;
			}
			temp_regno = Dee_memstate_hregs_find_usage(state, REGISTER_USAGE_ARGS);
			if (temp_regno < HOST_REGISTER_COUNT) {
				ptrdiff_t offset;
do_load_arg_with_args_regno:
				offset = offsetof(DeeTupleObject, t_elem) + ((ptrdiff_t)aid * HOST_SIZEOF_POINTER);
				gen86_pushP_mod(p_pc(block), gen86_modrm_db, offset, gen86_registers[temp_regno]);
				Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
				break;
			}

			/* Figure out which registers are used by later pushs. */
			bzero(&used_regs, sizeof(used_regs));
			for (future_argi = 0; future_argi < argi; ++future_argi) {
				struct Dee_memloc const *future_arg = &argv[future_argi];
				if (future_arg->ml_where == MEMLOC_TYPE_HREG) {
					ASSERT(future_arg->ml_value.ml_hreg < HOST_REGISTER_COUNT);
					BITSET_TURNON(&used_regs, future_arg->ml_value.ml_hreg);
				}
			}
			for (temp_regno = 0; temp_regno < HOST_REGISTER_COUNT; ++temp_regno) {
				if (!BITSET_GET(&used_regs, temp_regno)) {
					/* Found an empty register! */
					if (self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE) {
						if unlikely(_Dee_function_generator_gmov_usage2reg(self, REGISTER_USAGE_ARGS, temp_regno))
							goto err;
						state->ms_regs[temp_regno] = REGISTER_USAGE_ARGS;
						goto do_load_arg_with_args_regno;
					}
					if unlikely(_Dee_function_generator_gmov_usage2reg(self, REGISTER_USAGE_ARGV, temp_regno))
						goto err;
					state->ms_regs[temp_regno] = REGISTER_USAGE_ARGV;
					goto do_load_arg_with_argv_regno;
				}
			}

			/* Must fill in this argument later */
			fill_arg_args_later = argi;
			gen86_pushP_imm(p_pc(block), 0);
			Dee_function_generator_gadjust_cfa_offset(self, HOST_SIZEOF_POINTER);
		}	break;

		case MEMLOC_TYPE_CONST:
			if unlikely(Dee_function_generator_ghstack_pushconst(self, arg->ml_value.ml_const))
				goto err;
			break;
		default:
			return DeeError_Throwf(&DeeError_IllegalInstruction,
			                       "Cannot push memory location with type %#" PRFx16,
			                       arg->ml_where);
			break;
		}
	}
	if (fill_arg_args_later < argc) {
		/* Fill in missing arg-arguments */
		ptrdiff_t extra_offset = 0;
		Dee_host_register_t temp_regno, temp2_regno;
		temp_regno = Dee_memstate_hregs_find_usage(self->fg_state, REGISTER_USAGE_ARGV);
		if (temp_regno >= HOST_REGISTER_COUNT) {
			temp_regno = Dee_memstate_hregs_find_usage(self->fg_state, REGISTER_USAGE_ARGS);
			if (temp_regno < HOST_REGISTER_COUNT) {
				extra_offset = offsetof(DeeTupleObject, t_elem);
			} else {
				temp_regno = HOST_REGISTER_RETURN;
				if (self->fg_assembler->fa_cc & HOSTFUNC_CC_F_TUPLE) {
					extra_offset = offsetof(DeeTupleObject, t_elem);
					if unlikely(_Dee_function_generator_gmov_usage2reg(self, REGISTER_USAGE_ARGS, temp_regno))
						goto err;
				} else {
					if unlikely(_Dee_function_generator_gmov_usage2reg(self, REGISTER_USAGE_ARGV, temp_regno))
						goto err;
				}
				self->fg_state->ms_regs[temp_regno] = REGISTER_USAGE_ARGV;
			}
		}
		temp2_regno = temp_regno + 1;
		if (temp2_regno >= HOST_REGISTER_COUNT)
			temp2_regno = 0;
		for (argi = fill_arg_args_later; argi < argc; ++argi) {
			ptrdiff_t offset;
			ptrdiff_t sp_offset;
			struct Dee_memloc const *arg = &argv[argi];
			if (arg->ml_where != MEMLOC_TYPE_ARG)
				continue;
			if unlikely(Dee_basic_block_reqx86(block, 2))
				goto err;
			offset = (ptrdiff_t)arg->ml_value.ml_harg * HOST_SIZEOF_POINTER;
			offset += extra_offset;
			sp_offset = (ptrdiff_t)argi * HOST_SIZEOF_POINTER;
			gen86_movP_db_r(p_pc(block), offset, gen86_registers[temp_regno], gen86_registers[temp2_regno]);
			gen86_movP_r_db(p_pc(block), gen86_registers[temp2_regno], sp_offset, GEN86_R_PSP);
		}
	}

	/* With everything pushed onto the stack and in the correct position, generate the call.
	 * Note that because deemon API functions use DCALL (STDCALL), the called function does
	 * the stack cleanup. */
	{
		struct Dee_host_reloc *rel;
		if unlikely(Dee_basic_block_reqx86(block, 1))
			goto err;
		rel = Dee_basic_block_newhostrel(block);
		if unlikely(!rel)
			goto err;
		gen86_calll_offset(p_pc(block), -4);
		rel->hr_offset = p_off(block) - 4;
		rel->hr_type   = DEE_HOST_RELOC_PTREL32;
		rel->hr_value.hr_pt = c_function;
	}

	/* Adjust the CFA offset to account for the called function having cleaned up stack arguments. */
	Dee_function_generator_gadjust_cfa_offset(self, -(ptrdiff_t)(argc * HOST_SIZEOF_POINTER));

	/* After calling an external function, usage registers may have gotten clobbered. */
	Dee_memstate_hregs_clear_usage(self->fg_state);
	return 0;
err:
	return -1;
#endif /* !HOSTASM_X86_64 */
}




PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
_Dee_function_generator_gtest_and_jcc(struct Dee_function_generator *__restrict self,
                                      struct Dee_basic_block *dst,
                                      struct Dee_memloc *test_loc, uint8_t cc) {
	struct Dee_basic_block *block = self->fg_block;
	struct Dee_host_reloc *rel;
	if unlikely(Dee_basic_block_reqx86(block, 2))
		goto err;
	switch (test_loc->ml_where) {

	case MEMLOC_TYPE_HSTACK: {
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, test_loc->ml_value.ml_hstack);
		gen86_cmpP_imm_mod(p_pc(block), gen86_modrm_db, 0, sp_offset, GEN86_R_PSP);
	}	break;

	case MEMLOC_TYPE_HREG: {
		uint8_t reg86 = gen86_registers[test_loc->ml_value.ml_hreg];
		gen86_testP_r_r(p_pc(block), reg86, reg86);
	}	break;

	case MEMLOC_TYPE_ARG:
		/* Arguments are always non-NULL */
		if (cc == GEN86_CC_Z)
			return 0; /* Don't jump */
		goto always_jump;
	case MEMLOC_TYPE_CONST:
		if ((test_loc->ml_value.ml_const == NULL) != (cc == GEN86_CC_Z))
			return 0; /* Don't jump */
always_jump:
		return _Dee_basic_block_gjmp(block, dst);
	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot test memory location with type %#" PRFx16,
		                       test_loc->ml_where);
	}
	gen86_jccl_offset(p_pc(block), cc, -4);
	rel = Dee_basic_block_newhostrel(block);
	if unlikely(!rel)
		goto err;
	rel->hr_offset = p_off(block) - 4;
	rel->hr_type   = DEE_HOST_RELOC_BBREL32;
	rel->hr_value.hr_bb = dst;
	return 0;
err:
	return -1;
}

/* Generate jumps. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
_Dee_function_generator_gjz(struct Dee_function_generator *__restrict self,
                            struct Dee_basic_block *dst, struct Dee_memloc *test_loc) {
	return _Dee_function_generator_gtest_and_jcc(self, dst, test_loc, GEN86_CC_Z);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
_Dee_function_generator_gjnz(struct Dee_function_generator *__restrict self,
                             struct Dee_basic_block *dst, struct Dee_memloc *test_loc) {
	return _Dee_function_generator_gtest_and_jcc(self, dst, test_loc, GEN86_CC_NZ);
}

/* Emit conditional jump(s) based on `<test_loc> <=> 0' */
INTERN WUNUSED NONNULL((1, 5)) int DCALL
_Dee_function_generator_gjcmp0(struct Dee_function_generator *__restrict self,
                               struct Dee_basic_block *dst_lo_0, /* Jump here if `<test_loc> < 0' */
                               struct Dee_basic_block *dst_eq_0, /* Jump here if `<test_loc> == 0' */
                               struct Dee_basic_block *dst_gr_0, /* Jump here if `<test_loc> > 0' */
                               struct Dee_memloc *test_loc) {
	struct Dee_basic_block *block = self->fg_block;
	/* Check for some special cases. */
	if (!dst_lo_0 && !dst_eq_0 && !dst_gr_0)
		return 0;
	if (!dst_lo_0 && dst_eq_0 && !dst_gr_0)
		return _Dee_function_generator_gjz(self, dst_eq_0, test_loc);
	if (!dst_eq_0 && dst_lo_0 == dst_gr_0)
		return _Dee_function_generator_gjnz(self, dst_lo_0, test_loc);
	if unlikely(Dee_basic_block_reqx86(block, 2))
		goto err;
	switch (test_loc->ml_where) {

	case MEMLOC_TYPE_HSTACK: {
		ptrdiff_t sp_offset = Dee_memstate_hstack_cfa2sp(self->fg_state, test_loc->ml_value.ml_hstack);
		gen86_cmpP_imm_mod(p_pc(block), gen86_modrm_db, 0, sp_offset, GEN86_R_PSP);
	}	break;

	case MEMLOC_TYPE_HREG: {
		uint8_t reg86 = gen86_registers[test_loc->ml_value.ml_hreg];
		gen86_cmpP_imm_r(p_pc(block), 0, reg86);
	}	break;

	case MEMLOC_TYPE_CONST: {
		struct Dee_basic_block *dst;
		if ((intptr_t)test_loc->ml_value.ml_const < 0) {
			dst = dst_lo_0;
		} else if ((intptr_t)test_loc->ml_value.ml_const > 0) {
			dst = dst_gr_0;
		} else {
			dst = NULL;
		}
		return dst ? _Dee_basic_block_gjmp(block, dst) : 0;
	}	break;

	case MEMLOC_TYPE_ARG:
		/* Always non-NULL, and sign-bit shouldn't be
		 * set because that's where the kernel is at! */
		return dst_gr_0 ? _Dee_basic_block_gjmp(block, dst_gr_0) : 0;

	default:
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "Cannot test memory location with type %#" PRFx16,
		                       test_loc->ml_where);
	}

	/* Emit jumps to different basic blocks (while trying to combine shared destinations). */
	if (dst_lo_0) {
		struct Dee_host_reloc *rel;
		uint8_t cc = GEN86_CC_L;
		if (dst_lo_0 == dst_eq_0) {
			cc = GEN86_CC_LE;
			dst_eq_0 = NULL;
		} else if (dst_lo_0 == dst_gr_0) {
			cc = GEN86_CC_NE;
			dst_gr_0 = NULL;
		}
		gen86_jccl_offset(p_pc(block), cc, -4);
		rel = Dee_basic_block_newhostrel(block);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = p_off(block) - 4;
		rel->hr_type   = DEE_HOST_RELOC_BBREL32;
		rel->hr_value.hr_bb = dst_lo_0;
		if (cc == GEN86_CC_LE) {
			if (dst_gr_0) {
				dst_eq_0 = dst_gr_0;
emit_unconditional_jump:
				if unlikely(Dee_basic_block_reqx86(block, 1))
					goto err;
				goto do_emit_unconditional_jump;
			}
			return 0;
		} else if (cc == GEN86_CC_NE) {
			if (dst_eq_0)
				goto emit_unconditional_jump;
			return 0;
		}
	}
	if (dst_gr_0) {
		struct Dee_host_reloc *rel;
		uint8_t cc = GEN86_CC_G;
		if (dst_eq_0 == dst_gr_0) {
			if (dst_lo_0)
				goto emit_unconditional_jump;
			cc = GEN86_CC_GE;
			dst_eq_0 = NULL;
		}
		if unlikely(Dee_basic_block_reqx86(block, 1))
			goto err;
		gen86_jccl_offset(p_pc(block), cc, -4);
		rel = Dee_basic_block_newhostrel(block);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = p_off(block) - 4;
		rel->hr_type   = DEE_HOST_RELOC_BBREL32;
		rel->hr_value.hr_bb = dst_gr_0;
	}
	if (dst_eq_0) {
		struct Dee_host_reloc *rel;
		if unlikely(Dee_basic_block_reqx86(block, 1))
			goto err;
		if (dst_lo_0 && dst_gr_0) {
do_emit_unconditional_jump:
			gen86_jmpl_offset(p_pc(block), -4);
		} else {
			gen86_jccl_offset(p_pc(block), GEN86_CC_E, -4);
		}
		rel = Dee_basic_block_newhostrel(block);
		if unlikely(!rel)
			goto err;
		rel->hr_offset = p_off(block) - 4;
		rel->hr_type   = DEE_HOST_RELOC_BBREL32;
		rel->hr_value.hr_bb = dst_eq_0;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
_Dee_basic_block_gjmp(struct Dee_basic_block *__restrict self,
                      struct Dee_basic_block *dst) {
	struct Dee_host_reloc *rel;
	if unlikely(Dee_basic_block_reqx86(self, 1))
		goto err;
	gen86_jmpl_offset(p_pc(self), -4);
	rel = Dee_basic_block_newhostrel(self);
	if unlikely(!rel)
		goto err;
	rel->hr_offset = p_off(self) - 4;
	rel->hr_type   = DEE_HOST_RELOC_BBREL32;
	rel->hr_value.hr_bb = dst;
	return 0;
err:
	return -1;
}


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_ARCH_C */
