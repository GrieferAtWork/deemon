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
#ifndef GUARD_DEX_HOSTASM_UNWIND_H
#define GUARD_DEX_HOSTASM_UNWIND_H 1

#include "host.h"
/**/

#include <deemon/api.h>

#ifdef CONFIG_HAVE_LIBHOSTASM
#undef CONFIG_host_unwind_USES_NT_UNWIND_INFO
#undef CONFIG_host_unwind_USES_NOOP
#if (defined(HOSTASM_X86_64_MSABI) && \
     (defined(CONFIG_HOST_WINDOWS) || defined(__CYGWIN__) || defined(__CYGWIN32__)))
#define CONFIG_host_unwind_USES_NT_UNWIND_INFO
#else /* ... */
#define CONFIG_host_unwind_USES_NOOP
#endif /* !... */

#ifdef CONFIG_host_unwind_USES_NT_UNWIND_INFO
#include <Windows.h>
#endif /* CONFIG_host_unwind_USES_NT_UNWIND_INFO */

DECL_BEGIN

struct host_section;



/************************************************************************/
/* NT x64 IMPLEMENTATION                                                */
/************************************************************************/
#ifdef CONFIG_host_unwind_USES_NT_UNWIND_INFO
/* ===== START OF MS BS ===== */
/* From: https://learn.microsoft.com/en-us/cpp/build/exception-handling-x64?view=msvc-170 */
enum {
	NT_UWOP_PUSH_NONVOL = 0,
	NT_UWOP_ALLOC_LARGE,
	NT_UWOP_ALLOC_SMALL,
	NT_UWOP_SET_FPREG,
	NT_UWOP_SAVE_NONVOL,
	NT_UWOP_SAVE_NONVOL_FAR,
	NT_UWOP_SAVE_XMM128 = 8,
	NT_UWOP_SAVE_XMM128_FAR,
	NT_UWOP_PUSH_MACHFRAME
};
typedef union {
	struct {
		uint8_t CodeOffset;
		unsigned int UnwindOp: 4;
		unsigned int OpInfo  : 4;
	};
	uint16_t FrameOffset;
} NT_UNWIND_CODE;
#define NT_UNW_FLAG_EHANDLER  0x01
#define NT_UNW_FLAG_UHANDLER  0x02
#define NT_UNW_FLAG_CHAININFO 0x04
typedef struct {
	unsigned int Version: 3;
	unsigned int Flags  : 5;
	unsigned int SizeOfProlog;
	unsigned int CountOfCodes;
	unsigned int FrameRegister: 4;
	unsigned int FrameOffset  : 4;
	NT_UNWIND_CODE UnwindCode[1024];
} NT_UNWIND_INFO;
typedef struct {
	uint32_t BeginAddress;
	uint32_t EndAddress;
	uint32_t UnwindData;
} NT_RUNTIME_FUNCTION;
/* ===== END OF MS BS ===== */


struct host_unwind {
	/* NOTE: This stuff will eventually be registered with "RtlAddFunctionTable" */
	/* Because MS is dumb and doesn't want you to change %Psp while in the middle
	 * of a function, we actually have to encode every change to %Psp as its own
	 * function (wow...) */
	uint32_t             hu_currsp;  /* Currently active SP */
	size_t               hu_unwinda; /* Allocated # of unwind info segments */
	size_t               hu_unwindc; /* # of unwind info segments in use */
	NT_RUNTIME_FUNCTION *hu_unwindv; /* [0..hu_unwindc|ALLOC(hu_unwinda)][owned] Vector of unwind ranges ("UnwindData" is just the sp-offset) */
};

#define host_unwind_init_IS_BZERO
#define host_unwind_init(self)  bzero(self, sizeof(struct host_unwind))
#define host_unwind_fini(self)  Dee_Free((self)->hu_unwindv)
#define host_unwind_clear(self) (void)((self)->hu_unwindc = 0)

#define RUNTIME_FUNCTION_MAXSIZE 24 /* [sizeof(NT_RUNTIME_FUNCTION)=12] + [sizeof(NT_UNWIND_INFO)=12 (NT_UWOP_ALLOC_LARGE,1 form, +2 padding)] */

/* Returns the max # of text bytes that may be needed for unwind info.
 * This info is needed for knowing how much extra space to mmap() at the start. */
#define host_section_unwind_maxsize(self) \
	(((self)->hs_unwind.hu_unwindc + 1) * RUNTIME_FUNCTION_MAXSIZE)

/* Remember that the return address is at "*(void **)(%Psp + sp_offset)" right now.
 * This function is called by arch-specific code in "generator-arch.c */
INTDEF WUNUSED NONNULL((1)) int DCALL
host_section_unwind_setsp(struct host_section *__restrict self,
                          ptrdiff_t sp_offset);
#define DEFINED_host_section_unwind_setsp

#define host_section_unwind_setsp_initial(self, sp_offset) \
	(void)((self)->hs_unwind.hu_currsp = (uint32_t)(sp_offset))
#define DEFINED_host_section_unwind_setsp_initial

#ifdef HOSTASM_HAVE_SHRINKJUMPS
/* Called as part of jump shrinking to remove the given address range. */
INTDEF NONNULL((1)) void DCALL
host_section_unwind_trimrange(struct host_section *__restrict self,
                              uint32_t sectrel_addr, uint32_t num_bytes);
#endif /* HOSTASM_HAVE_SHRINKJUMPS */

/* Check if unwind instrumentation should be used. */
INTDEF WUNUSED bool DCALL hostfunc_unwind_enabled(void);
#define HAVE_hostfunc_unwind_enabled

struct hostfunc_unwind {
	NT_RUNTIME_FUNCTION *hfu_FunctionTable; /* [0..1] Function table pointer (or NULL if not present). */
};

/* Initialize host function unwind data. */
INTDEF NONNULL((1, 2, 3, 4)) void DCALL
hostfunc_unwind_init(struct hostfunc_unwind *__restrict self,
                     struct function_assembler *__restrict assembler,
                     byte_t *start_of_text, byte_t *end_of_text);

/* Finalize host function unwind data. */
INTDEF NONNULL((1)) void DCALL
hostfunc_unwind_fini(struct hostfunc_unwind *__restrict self);
#endif /* CONFIG_host_unwind_USES_NT_UNWIND_INFO */





/************************************************************************/
/* NOOP IMPLEMENTATION                                                  */
/************************************************************************/
#ifdef CONFIG_host_unwind_USES_NOOP
#define host_unwind_init_IS_BZERO
#define host_unwind_init(self)  (void)0
#define host_unwind_fini(self)  (void)0
#define host_unwind_clear(self) (void)0
#define host_section_unwind_setsp(self, sp_offset) 0
#endif /* CONFIG_host_unwind_USES_NOOP */

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_UNWIND_H */
