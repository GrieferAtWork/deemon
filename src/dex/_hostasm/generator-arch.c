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

#define p_pc(block) (&(block)->bb_host_end)


/* Code generators. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gincref(struct Dee_function_generator *__restrict self,
                               struct Dee_memloc *__restrict loc) {
	/* TODO:
	 * #IF(loc->ml_where == MEMLOC_TYPE_CONST && FIT32)
	 * >>     lock  decP <loc->ml_value.ml_const->ob_refcnt>
	 * #ELIF(loc->ml_where == MEMLOC_TYPE_CONST && !FIT32)
	 * >>     movabs $<loc->ml_value.ml_const->ob_refcnt>, %Pax
	 * >>     lock  incP (%Pax)
	 * #ELSE
	 * >> movP  <loc>, %Pax
	 * >> lock  incP ob_refcnt(%Pax)
	 * #ENDIF
	 */
	(void)self;
	(void)loc;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gdecref(struct Dee_function_generator *__restrict self,
                               struct Dee_memloc *__restrict loc) {
	/* TODO:
	 * #IF(loc->ml_where == MEMLOC_TYPE_CONST && FIT32)
	 * >>     lock  decP <loc->ml_value.ml_const->ob_refcnt>
	 * #ELIF(loc->ml_where == MEMLOC_TYPE_CONST && !FIT32)
	 * >>     movabs $<loc->ml_value.ml_const->ob_refcnt>, %Pax
	 * >>     lock  decP (%Pax)
	 * #ELIF_OPTION_1
	 * >> <Dee_function_generator_gflushregs>
	 * >>     movP  <loc>, %Pax
	 * >>     lock  subP $1, ob_refcnt(%Pax)
	 * >>     jnz   1f
	 * >>     pushl %Pax
	 * >>     call  DeeObject_Destroy@4
	 * >> 1:
	 * #ELSE
	 * >>     movP  <loc>, %Pax
	 * >>     lock  subP $1, ob_refcnt(%Pax)
	 * >>     jnz   1f
	 * >>     <push_all_used_registers>
	 * >>     pushl %Pax
	 * >>     call  DeeObject_Destroy@4
	 * >>     <pop_all_used_registers>
	 * >> 1:
	 * #ENDIF
	 */
	(void)self;
	(void)loc;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxincref(struct Dee_function_generator *__restrict self,
                                struct Dee_memloc *__restrict loc) {
	/* TODO: Like `Dee_function_generator_gincref()', but generate a NULL-check */
	(void)self;
	(void)loc;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gxdecref(struct Dee_function_generator *__restrict self,
                                struct Dee_memloc *__restrict loc) {
	/* TODO: Like `Dee_function_generator_gdecref()', but generate a NULL-check */
	(void)self;
	(void)loc;
	return DeeError_NOTIMPLEMENTED();
}

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
 * pointer-sized arguments whose values are taken from `argv'. */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gcall_c_function(struct Dee_function_generator *__restrict self,
                                        void *c_function, size_t argc,
                                        struct Dee_memloc const *argv) {
	/* TODO */
	(void)self;
	(void)c_function;
	(void)argc;
	(void)argv;
	return DeeError_NOTIMPLEMENTED();
}

/* Generate checks to enter exception handling mode. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gexcept_if_zero(struct Dee_function_generator *__restrict self,
                                       struct Dee_memloc *__restrict loc) {
	/* TODO */
	(void)self;
	(void)loc;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
Dee_function_generator_gexcept_if_nonzero(struct Dee_function_generator *__restrict self,
                                          struct Dee_memloc *__restrict loc) {
	/* TODO */
	(void)self;
	(void)loc;
	return DeeError_NOTIMPLEMENTED();
}

INTERN WUNUSED NONNULL((1)) int DCALL
Dee_function_generator_gexcept(struct Dee_function_generator *__restrict self) {
	/* TODO */
	(void)self;
	return DeeError_NOTIMPLEMENTED();
}



DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_GENERATOR_ARCH_C */
