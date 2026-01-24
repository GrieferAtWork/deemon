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
#ifndef GUARD_DEX_HOSTASM_UNWIND_C
#define GUARD_DEX_HOSTASM_UNWIND_C 1
#define DEE_SOURCE

#include "unwind.h"
/**/

#include <deemon/api.h>

#ifdef CONFIG_HAVE_LIBHOSTASM
#ifndef CONFIG_host_unwind_USES_NOOP
#include "libhostasm.h"
/**/

#include <deemon/alloc.h>
#include <deemon/error.h>
#include <deemon/format.h>

#include <hybrid/align.h>           /* CEIL_ALIGN */
#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */
#include <hybrid/sequence/list.h>   /* TAILQ_FOREACH */

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, ptrdiff_t, size_t */
#include <stdint.h>  /* uintN_t, uintptr_t */

DECL_BEGIN

#ifdef CONFIG_host_unwind_USES_NT_UNWIND_INFO

struct host_unwind_ms64_writer {
	byte_t   *humw_RUNTIME_FUNCTION; /* Writer for generated "RUNTIME_FUNCTION" */
	byte_t   *humw_UNWIND_INFO;      /* Writer for "UNWIND_INFO" pointed-to by "RUNTIME_FUNCTION" */
	byte_t   *humw_image_base;       /* [const] "image base pointer" (used for encoding "image-relative" offsets) */
	uint32_t *humw_last_EndAddress;  /* [0..1] Pointer to the last-written EndAddress */
	uint32_t  humw_last_sp_offset;   /* [valid_if(humw_last_EndAddress)] SP offset at `humw_last_EndAddress' */
	uint32_t  humw_function_count;   /* # of generated functions. */
};

PRIVATE NONNULL((1, 2)) void DCALL
host_unwind_ms64_writer_writefun(struct host_unwind_ms64_writer *__restrict self,
                                 uint32_t imagerel_BeginAddress,
                                 uint32_t imagerel_EndAddress,
                                 uint32_t sp_offset) {
	NT_RUNTIME_FUNCTION *pFunction;
	NT_UNWIND_INFO *pUnwindInfo;
	NT_UNWIND_CODE *pCode;
#if 1
	HA_printf("unwind %p-%p with cfa_offset=%I32u\n",
	          self->humw_image_base + imagerel_BeginAddress,
	          self->humw_image_base + imagerel_EndAddress,
	          sp_offset);
#endif

	/* Optimization: merge with predecessor if same SP offset */
	if (self->humw_last_EndAddress &&
	    self->humw_last_sp_offset == sp_offset &&
	    *self->humw_last_EndAddress == imagerel_BeginAddress) {
		*self->humw_last_EndAddress = imagerel_EndAddress;
		return;
	}

	/* Special case: at an SP-offset of 0, we don't need any extra info! */
	if (sp_offset == 0)
		return;

	pFunction   = (NT_RUNTIME_FUNCTION *)self->humw_RUNTIME_FUNCTION;
	pUnwindInfo = (NT_UNWIND_INFO *)self->humw_UNWIND_INFO;
	pFunction->BeginAddress = imagerel_BeginAddress;
	pFunction->EndAddress   = imagerel_EndAddress;
	pFunction->UnwindData   = (uint32_t)((byte_t *)pUnwindInfo - self->humw_image_base);

	pUnwindInfo->Version       = 1;
	pUnwindInfo->Flags         = 0;
	pUnwindInfo->SizeOfProlog  = 0;
	pUnwindInfo->FrameRegister = 0;
	pUnwindInfo->FrameOffset   = 0;
	pCode = pUnwindInfo->UnwindCode;
	if (sp_offset <= 128 && (sp_offset & 7) == 0) {
		pCode[0].CodeOffset = 0;
		pCode[0].UnwindOp   = NT_UWOP_ALLOC_SMALL;
		pCode[0].OpInfo     = (uint8_t)((sp_offset - 8) / 8);
		bzero(&pCode[1], sizeof(pCode[1])); /* Padding */
		pCode += 2;
	} else if ((sp_offset / 8) <= 0xffff && (sp_offset & 7) == 0) {
		pCode[0].CodeOffset  = 0;
		pCode[0].UnwindOp    = NT_UWOP_ALLOC_LARGE;
		pCode[0].OpInfo      = 0;
		pCode[1].FrameOffset = (uint16_t)(sp_offset / 8);
		pCode += 2;
	} else {
		pCode[0].CodeOffset  = 0;
		pCode[0].UnwindOp    = NT_UWOP_ALLOC_LARGE;
		pCode[0].OpInfo      = 1;
		pCode[1].FrameOffset = (uint16_t)(sp_offset);
		pCode[2].FrameOffset = (uint16_t)(sp_offset >> 16);
		bzero(&pCode[3], sizeof(pCode[3])); /* Padding */
		pCode += 4;
	}
	pUnwindInfo->CountOfCodes   = (uint8_t)(pCode - pUnwindInfo->UnwindCode);
	self->humw_RUNTIME_FUNCTION = (byte_t *)(pFunction + 1);
	self->humw_UNWIND_INFO      = (byte_t *)pCode;
	self->humw_last_EndAddress  = &pFunction->EndAddress;
	self->humw_last_sp_offset   = sp_offset;
	++self->humw_function_count;
}

/* Output "sect" into "self" */
PRIVATE NONNULL((1, 2)) void DCALL
host_unwind_ms64_writer_writesect(struct host_unwind_ms64_writer *__restrict self,
                                  struct host_section *__restrict sect) {
	size_t i;
	uint32_t sect_delta = (uint32_t)(sect->hs_base - self->humw_image_base);
	uint32_t last_sp = 0;
	uint32_t last_pc = 0;
	uint32_t stop_pc = (uint32_t)host_section_size(sect);
	if (sect->hs_unwind.hu_unwindc >= 1 &&
	    sect->hs_unwind.hu_unwindv[sect->hs_unwind.hu_unwindc - 1].UnwindData == sect->hs_unwind.hu_currsp)
		sect->hs_unwind.hu_unwindv[sect->hs_unwind.hu_unwindc - 1].EndAddress = stop_pc;
	for (i = 0; i < sect->hs_unwind.hu_unwindc; ++i) {
		NT_RUNTIME_FUNCTION *src = &sect->hs_unwind.hu_unwindv[i];
		ASSERT(src->BeginAddress < src->EndAddress);
		ASSERT(src->EndAddress <= (uint32_t)host_section_size(sect));
		host_unwind_ms64_writer_writefun(self,
		                                     (uint32_t)(src->BeginAddress + sect_delta),
		                                     (uint32_t)(src->EndAddress + sect_delta),
		                                     (uint32_t)src->UnwindData);
		last_pc = (uint32_t)src->EndAddress;
		last_sp = (uint32_t)src->UnwindData;
	}
	ASSERT(last_pc <= stop_pc);
	if (last_pc < stop_pc) {
		host_unwind_ms64_writer_writefun(self,
		                                     (uint32_t)(last_pc + sect_delta),
		                                     (uint32_t)(stop_pc + sect_delta),
		                                     (uint32_t)sect->hs_unwind.hu_currsp);
	}
}

/* Remember that the return address is at "*(void **)(%Psp + sp_offset)" right now. */
INTERN WUNUSED NONNULL((1)) int DCALL
host_section_unwind_setsp(struct host_section *__restrict self,
                          ptrdiff_t sp_offset) {
	uint32_t old_pc, new_pc;
	uint32_t new_sp = (uint32_t)sp_offset;
	NT_RUNTIME_FUNCTION *slot;
	if unlikely(new_sp < 0) {
		return DeeError_Throwf(&DeeError_IllegalInstruction,
		                       "host SP offset %" PRFdSIZ " is negative",
		                       new_sp);
	}
	if (self->hs_unwind.hu_currsp == new_sp)
		return 0; /* Nothing changed */
	old_pc = 0;
	new_pc = (uint32_t)host_section_size(self);
	if (self->hs_unwind.hu_unwindc > 0) {
		NT_RUNTIME_FUNCTION *old;
		old    = &self->hs_unwind.hu_unwindv[self->hs_unwind.hu_unwindc - 1];
		old_pc = old->EndAddress;
		ASSERT(old_pc < new_pc);
	}
	ASSERT(old_pc <= new_pc);
	if (old_pc == new_pc) {
		/* Special case: change SP without any code (can happen due to jumps) */
		self->hs_unwind.hu_currsp = new_sp;
		return 0;
	}
	if (self->hs_unwind.hu_unwindc >= self->hs_unwind.hu_unwinda) {
		NT_RUNTIME_FUNCTION *newvec;
		size_t new_alloc = (self->hs_unwind.hu_unwinda * 2);
		if (new_alloc < 4)
			new_alloc = 4;
		newvec = (NT_RUNTIME_FUNCTION *)Dee_TryReallocc(self->hs_unwind.hu_unwindv,
		                                                new_alloc,
		                                                sizeof(NT_RUNTIME_FUNCTION));
		if unlikely(!newvec) {
			new_alloc = self->hs_unwind.hu_unwindc + 1;
			newvec = (NT_RUNTIME_FUNCTION *)Dee_Reallocc(self->hs_unwind.hu_unwindv,
			                                             new_alloc,
			                                             sizeof(NT_RUNTIME_FUNCTION));
			if unlikely(!newvec)
				goto err;
		}
		self->hs_unwind.hu_unwinda = new_alloc;
		self->hs_unwind.hu_unwindv = newvec;
	}
	slot = &self->hs_unwind.hu_unwindv[self->hs_unwind.hu_unwindc];
	slot->BeginAddress = old_pc;
	slot->EndAddress   = new_pc;
	ASSERT(slot->BeginAddress < slot->EndAddress);
	ASSERT(slot->EndAddress <= (uint32_t)host_section_size(self));
	slot->UnwindData   = self->hs_unwind.hu_currsp; /* End of old SP region */
	++self->hs_unwind.hu_unwindc;
	self->hs_unwind.hu_currsp = new_sp;
	return 0;
err:
	return -1;
}

#ifdef HOSTASM_HAVE_SHRINKJUMPS
/* Called as part of jump shrinking to remove the given address range. */
INTERN NONNULL((1)) void DCALL
host_section_unwind_trimrange(struct host_section *__restrict self,
                              uint32_t sectrel_addr, uint32_t num_bytes) {
	size_t i = self->hs_unwind.hu_unwindc;
	NT_RUNTIME_FUNCTION *vec = self->hs_unwind.hu_unwindv;
	while (i > 0) {
		NT_RUNTIME_FUNCTION *slot = &vec[--i];
		if (slot->EndAddress <= sectrel_addr)
			break; /* No longer affected. */
		slot->EndAddress -= num_bytes;
		if (slot->EndAddress < sectrel_addr)
			slot->EndAddress = sectrel_addr;
		if (slot->BeginAddress >= sectrel_addr) {
			slot->BeginAddress -= num_bytes;
			if (slot->BeginAddress < sectrel_addr)
				slot->BeginAddress = sectrel_addr;
		}
		if (slot->BeginAddress >= slot->EndAddress) {
			/* Can fully remove this slot. */
			--self->hs_unwind.hu_unwindc;
			memmovedownc(slot, slot + 1,
			             self->hs_unwind.hu_unwindc - i,
			             sizeof(NT_RUNTIME_FUNCTION));
		}
	}
}
#endif /* HOSTASM_HAVE_SHRINKJUMPS */



#undef RtlAddFunctionTable
#undef RtlDeleteFunctionTable
typedef BOOLEAN (ATTR_CDECL *LPRTLADDFUNCTIONTABLE)(NT_RUNTIME_FUNCTION *FunctionTable, uint32_t EntryCount, uint64_t BaseAddress);
typedef BOOLEAN (ATTR_CDECL *LPRTLDELETEFUNCTIONTABLE)(NT_RUNTIME_FUNCTION *FunctionTable);
PRIVATE LPRTLADDFUNCTIONTABLE pdyn_RtlAddFunctionTable;
PRIVATE LPRTLDELETEFUNCTIONTABLE pdyn_RtlDeleteFunctionTable;
#define RtlAddFunctionTable    (*pdyn_RtlAddFunctionTable)
#define RtlDeleteFunctionTable (*pdyn_RtlDeleteFunctionTable)

PRIVATE WCHAR const wKernel32_dll[] = { 'K', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', 0 };


/* Check if unwind instrumentation should be used. */
INTERN WUNUSED bool DCALL hostfunc_unwind_enabled(void) {
	if (!pdyn_RtlAddFunctionTable) {
		LPRTLADDFUNCTIONTABLE new_RtlAddFunctionTable;
		LPRTLDELETEFUNCTIONTABLE new_RtlDeleteFunctionTable;
		HMODULE mKernel32 = LoadLibraryW(wKernel32_dll);
		if (!mKernel32)
			goto fail;
		new_RtlAddFunctionTable = (LPRTLADDFUNCTIONTABLE)GetProcAddress(mKernel32, "RtlAddFunctionTable");
		new_RtlDeleteFunctionTable = (LPRTLDELETEFUNCTIONTABLE)GetProcAddress(mKernel32, "RtlDeleteFunctionTable");
		if (!new_RtlAddFunctionTable || !new_RtlDeleteFunctionTable) {
fail:
			new_RtlAddFunctionTable = (LPRTLADDFUNCTIONTABLE)(void *)-1;
			new_RtlDeleteFunctionTable = (LPRTLDELETEFUNCTIONTABLE)(void *)-1;
		}
		COMPILER_WRITE_BARRIER();
		pdyn_RtlDeleteFunctionTable = new_RtlDeleteFunctionTable;
		COMPILER_WRITE_BARRIER();
		pdyn_RtlAddFunctionTable = new_RtlAddFunctionTable;
		COMPILER_WRITE_BARRIER();
	}
	return pdyn_RtlAddFunctionTable != (LPRTLADDFUNCTIONTABLE)(void *)-1;
}


/* Initialize host function unwind data. */
INTERN NONNULL((1, 2, 3, 4)) void DCALL
hostfunc_unwind_init(struct hostfunc_unwind *__restrict self,
                     struct function_assembler *__restrict assembler,
                     byte_t *start_of_text, byte_t *end_of_text) {
	BOOLEAN bAddOk;
	struct host_unwind_ms64_writer writer;
	struct host_section *sect;
	NT_RUNTIME_FUNCTION *pFunctionTable;
	size_t num_functions;

	/* Try to load unwind functions, but if this
	 * fails, don't produce unwind instrumentation. */
	if (!hostfunc_unwind_enabled()) {
		self->hfu_FunctionTable = NULL;
		return;
	}

	writer.humw_image_base       = start_of_text;
	writer.humw_RUNTIME_FUNCTION = (byte_t *)CEIL_ALIGN((uintptr_t)end_of_text, HOST_SIZEOF_POINTER);
	pFunctionTable = (NT_RUNTIME_FUNCTION *)writer.humw_RUNTIME_FUNCTION;
	num_functions = 0;
	TAILQ_FOREACH (sect, &assembler->fa_sections, hs_link) {
		num_functions += sect->hs_unwind.hu_unwindc;
		if (host_section_size(sect) && sect->hs_unwind.hu_currsp != 0)
			++num_functions;
	}
	if (num_functions == 0) {
		/* Special case: nothing needs to be output. */
		self->hfu_FunctionTable = NULL;
		return;
	}
	writer.humw_UNWIND_INFO = writer.humw_RUNTIME_FUNCTION + (num_functions * sizeof(NT_RUNTIME_FUNCTION));
	writer.humw_last_EndAddress = NULL;
	writer.humw_function_count = 0;
	TAILQ_FOREACH (sect, &assembler->fa_sections, hs_link)
		host_unwind_ms64_writer_writesect(&writer, sect);

	/* Register the function table with NT. */
	self->hfu_FunctionTable = pFunctionTable;

	DBG_ALIGNMENT_DISABLE();
	bAddOk = RtlAddFunctionTable(pFunctionTable, writer.humw_function_count,
	                             (uintptr_t)writer.humw_image_base);
	DBG_ALIGNMENT_ENABLE();
	if (!bAddOk) {
		HA_printf("Error calling RtlAddFunctionTable: %u\n", (unsigned int)GetLastError());
		self->hfu_FunctionTable = NULL;
	}
}

/* Finalize host function unwind data. */
INTERN NONNULL((1)) void DCALL
hostfunc_unwind_fini(struct hostfunc_unwind *__restrict self) {
	if (self->hfu_FunctionTable) {
		DBG_ALIGNMENT_DISABLE();
		RtlDeleteFunctionTable(self->hfu_FunctionTable);
		DBG_ALIGNMENT_ENABLE();
	}
}

#endif /* CONFIG_host_unwind_USES_NT_UNWIND_INFO */

DECL_END
#endif /* !CONFIG_host_unwind_USES_NOOP */
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_UNWIND_C */
