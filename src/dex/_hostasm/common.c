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
#ifndef GUARD_DEX_HOSTASM_COMMON_C
#define GUARD_DEX_HOSTASM_COMMON_C 1
#define DEE_SOURCE

#include "libhostasm.h"
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
#include <deemon/alloc.h>
#include <deemon/error.h>
#include <deemon/format.h>

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

INTERN NONNULL((1)) void DCALL
Dee_memstate_destroy(struct Dee_memstate *__restrict self) {
	Dee_Free(self->ms_stackv);
	Dee_memstate_free(self);
}

/* Replace `*p_self' with a copy of itself
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
Dee_memstate_inplace_copy_because_shared(struct Dee_memstate **__restrict p_self) {
	struct Dee_memstate *copy, *self;
	self = *p_self;
	ASSERT(Dee_memstate_isshared(self));
	copy = Dee_memstate_copy(self);
	if unlikely(!copy)
		goto err;
	Dee_memstate_decref_nokill(self);
	*p_self = copy;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF struct Dee_memstate *DCALL
Dee_memstate_copy(struct Dee_memstate *__restrict self) {
	DREF struct Dee_memstate *result;
	result = Dee_memstate_alloc(self->ms_localc);
	if unlikely(!result)
		goto err;
	memcpy(result, self,
	       offsetof(struct Dee_memstate, ms_localv) +
	       self->ms_localc * sizeof(struct Dee_memloc));
	result->ms_stackv = (struct Dee_memloc *)Dee_Mallocc(self->ms_stackc,
	                                                     sizeof(struct Dee_memloc));
	if unlikely(!result->ms_stackv)
		goto err_r;
	result->ms_refcnt = 1;
	result->ms_stacka = self->ms_stackc;
	memcpyc(result->ms_stackv, self->ms_stackv,
	        self->ms_stackc, sizeof(struct Dee_memloc));
	return result;
err_r:
	Dee_memstate_free(result);
err:
	return NULL;
}




/* Fill in `self->hr_vtype' and `self->hr_value' based on `sym'
 * If `sym' has already been defined as absolute or pointing to
 * the start of a section, directly inline it. */
INTERN NONNULL((1, 2)) void DCALL
Dee_host_reloc_setsym(struct Dee_host_reloc *__restrict self,
                      struct Dee_host_symbol *__restrict sym) {
	switch (sym->hs_type) {
	case DEE_HOST_SYMBOL_ABS:
		self->hr_vtype = DEE_HOST_RELOCVALUE_ABS;
		self->hr_value.rv_abs = sym->hs_value.sv_abs;
		return;
	case DEE_HOST_SYMBOL_SECT:
		if (sym->hs_value.sv_sect.ss_off == 0) {
			self->hr_vtype = DEE_HOST_RELOCVALUE_SECT;
			self->hr_value.rv_sect = sym->hs_value.sv_sect.ss_sect;
			return;
		}
		break;
	default: break;
	}
	self->hr_vtype = DEE_HOST_RELOCVALUE_SYM;
	self->hr_value.rv_sym = sym;
}




/* Ensure that at least `num_bytes' of host text memory are available.
 * @return: 0 : Success
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1)) int DCALL
_Dee_host_section_reqhost(struct Dee_host_section *__restrict self,
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
INTERN WUNUSED NONNULL((1)) struct Dee_host_reloc *DCALL
Dee_host_section_newhostrel(struct Dee_host_section *__restrict self) {
	struct Dee_host_reloc *result;
	ASSERT(self->hs_relc <= self->hs_rela);
	if unlikely(self->hs_relc >= self->hs_rela) {
		size_t min_alloc = self->hs_relc + 1;
		size_t new_alloc = self->hs_rela * 2;
		struct Dee_host_reloc *new_list;
		if (new_alloc < 4)
			new_alloc = 4;
		if (new_alloc < min_alloc)
			new_alloc = min_alloc;
		new_list = (struct Dee_host_reloc *)Dee_TryReallocc(self->hs_relv,
		                                                    new_alloc,
		                                                    sizeof(struct Dee_host_reloc));
		if unlikely(!new_list) {
			new_alloc = min_alloc;
			new_list = (struct Dee_host_reloc *)Dee_Reallocc(self->hs_relv,
			                                                 new_alloc,
			                                                 sizeof(struct Dee_host_reloc));
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
INTERN WUNUSED NONNULL((1)) struct Dee_jump_descriptor *DCALL
Dee_jump_descriptors_lookup(struct Dee_jump_descriptors const *__restrict self,
                            Dee_instruction_t const *deemon_from) {
	size_t lo = 0;
	size_t hi = self->jds_size;
	while (lo < hi) {
		struct Dee_jump_descriptor *result;
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
Dee_jump_descriptors_insert(struct Dee_jump_descriptors *__restrict self,
                            struct Dee_jump_descriptor *__restrict descriptor) {
	byte_t const *deemon_from = descriptor->jd_from;
	size_t lo = 0;
	size_t hi = self->jds_size;
	while (lo < hi) {
		struct Dee_jump_descriptor *result;
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
		struct Dee_jump_descriptor **new_list;
		size_t new_alloc = (self->jds_alloc << 1);
		if (new_alloc < 8)
			new_alloc = 8;
		new_list = (struct Dee_jump_descriptor **)Dee_TryReallocc(self->jds_list, new_alloc,
		                                                          sizeof(struct Dee_jump_descriptor *));
		if (!new_list) {
			new_alloc = self->jds_size + 1;
			new_list = (struct Dee_jump_descriptor **)Dee_Reallocc(self->jds_list, new_alloc,
			                                                       sizeof(struct Dee_jump_descriptor *));
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
	           sizeof(struct Dee_jump_descriptor *));
	self->jds_list[lo] = descriptor;
	++self->jds_size;
	return 0;
err:
	return -1;
}

/* Remove `descriptor' from `self' (said descriptor *must* be part of `self') */
INTERN WUNUSED NONNULL((1, 2)) void DCALL
Dee_jump_descriptors_remove(struct Dee_jump_descriptors *__restrict self,
                            struct Dee_jump_descriptor *__restrict descriptor) {
	size_t lo = 0;
	size_t hi = self->jds_size;
	for (;;) {
		struct Dee_jump_descriptor *result;
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
			             sizeof(struct Dee_jump_descriptor *));
			break;
		}
	}
}



/* Destroy the given basic block `self'. */
INTERN NONNULL((1)) void DCALL
Dee_basic_block_destroy(struct Dee_basic_block *__restrict self) {
	size_t i;
	for (i = 0; i < self->bb_entries.jds_size; ++i) {
		struct Dee_jump_descriptor *descriptor;
		descriptor = self->bb_entries.jds_list[i];
		Dee_jump_descriptor_destroy(descriptor);
	}
	Dee_Free(self->bb_entries.jds_list);
	Dee_Free(self->bb_exits.jds_list);
	if (self->bb_mem_start)
		Dee_memstate_decref(self->bb_mem_start);
	if (self->bb_mem_end)
		Dee_memstate_decref(self->bb_mem_end);
	Dee_host_section_fini(&self->bb_htext);
	Dee_host_section_fini(&self->bb_hcold);
	Dee_basic_block_free(self);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
Dee_jump_descriptors_find_lowest_addr(struct Dee_jump_descriptors const *__restrict self,
                                      Dee_instruction_t const *deemon_from) {
	size_t lo = 0;
	size_t hi = self->jds_size;
	size_t result = hi;
	while (lo < hi) {
		struct Dee_jump_descriptor *descriptor;
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
INTERN WUNUSED NONNULL((1)) struct Dee_basic_block *DCALL
Dee_basic_block_splitat(struct Dee_basic_block *__restrict self,
                        Dee_instruction_t const *addr) {
	size_t exit_split;
	struct Dee_basic_block *result;
	ASSERT(addr > self->bb_deemon_start);
	ASSERT(addr < self->bb_deemon_end);
	result = Dee_basic_block_alloc();
	if unlikely(!result)
		goto err;

	/* Figure out how many exits to transfer. */
	exit_split = Dee_jump_descriptors_find_lowest_addr(&self->bb_exits, addr);
	if (exit_split >= self->bb_exits.jds_size) {
		/* Special case: nothing to transfer */
		Dee_jump_descriptors_init(&result->bb_exits); /* TODO */
	} else if (exit_split == 0) {
		/* Special case: transfer everything */
		memcpy(&result->bb_exits, &self->bb_exits, sizeof(struct Dee_jump_descriptors));
		Dee_jump_descriptors_init(&self->bb_exits);
	} else {
		struct Dee_jump_descriptor **result_exits;
		size_t num_transfer;
		Dee_jump_descriptors_init(&result->bb_exits);
		num_transfer = self->bb_exits.jds_size - exit_split;
		ASSERT(num_transfer > 0);
		ASSERT(num_transfer < self->bb_exits.jds_size);
		result_exits = (struct Dee_jump_descriptor **)Dee_Mallocc(num_transfer,
		                                                          sizeof(struct Dee_jump_descriptor *));
		if unlikely(!result_exits)
			goto err_r;
		memcpyc(result_exits, self->bb_exits.jds_list + exit_split,
		        num_transfer, sizeof(struct Dee_jump_descriptor *));
		result->bb_exits.jds_list  = result_exits;
		result->bb_exits.jds_size  = num_transfer;
		result->bb_exits.jds_alloc = num_transfer;
		self->bb_exits.jds_size    = exit_split;
	}

	/* Fill in remaining fields and adjust caller-given block bounds. */
	Dee_basic_block_init_common(result);
	result->bb_deemon_start = addr;
	result->bb_deemon_end   = self->bb_deemon_end;
	self->bb_deemon_end     = addr;
	return result;
err_r:
	Dee_basic_block_free(result);
err:
	return NULL;
}




INTERN NONNULL((1)) void DCALL
Dee_function_assembler_fini(struct Dee_function_assembler *__restrict self) {
	size_t i;
	for (i = 0; i < self->fa_blockc; ++i)
		Dee_basic_block_destroy(self->fa_blockv[i]);
	for (i = 0; i < self->fa_except_exitc; ++i)
		Dee_except_exitinfo_destroy(self->fa_except_exitv[i]);
	if (self->fa_prolog_end)
		Dee_memstate_decref(self->fa_prolog_end);
	Dee_host_section_fini(&self->fa_prolog);
	Dee_Free(self->fa_blockv);
	Dee_Free(self->fa_except_exitv);
	{
		struct Dee_host_symbol *sym = self->fa_symbols;
		while (sym) {
			struct Dee_host_symbol *next = sym->_hs_next;
			_Dee_host_symbol_free(sym);
			sym = next;
		}
	}
}


/* Ensure that the basic block containing `deemon_addr' also *starts* at that address.
 * This function is used during the initial scan-pass where basic blocks are identified
 * and created.
 * @return: * :   The basic block in question.
 * @return: NULL: An error occurred (OOM or address is out-of-bounds). */
INTERN WUNUSED NONNULL((1)) struct Dee_basic_block *DCALL
Dee_function_assembler_splitblock(struct Dee_function_assembler *__restrict self,
                                  Dee_instruction_t const *deemon_addr) {
	size_t lo = 0;
	size_t hi = self->fa_blockc;
	ASSERT(self->fa_blockc <= self->fa_blocka);
	while (lo < hi) {
		struct Dee_basic_block *result;
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
					struct Dee_basic_block **new_list;
					size_t new_alloc = (self->fa_blocka << 1);
					if (new_alloc < 8)
						new_alloc = 8;
					new_list = (struct Dee_basic_block **)Dee_TryReallocc(self->fa_blockv, new_alloc,
					                                                      sizeof(struct Dee_basic_block *));
					if (!new_list) {
						new_alloc = self->fa_blockc + 1;
						new_list = (struct Dee_basic_block **)Dee_Reallocc(self->fa_blockv, new_alloc,
						                                                   sizeof(struct Dee_basic_block *));
						if unlikely(!new_list)
							goto err;
					}
					self->fa_blockv = new_list;
					self->fa_blocka = new_alloc;
				}

				/* Must split this basic block. */
				result = Dee_basic_block_splitat(result, deemon_addr);
				if unlikely(!result)
					goto err;

				/* Insert the new block into the vector at `mid+1' */
				++mid;
				memmoveupc(&self->fa_blockv[mid + 1],
				           &self->fa_blockv[mid],
				           self->fa_blockc - mid,
				           sizeof(struct Dee_jump_descriptor *));
				self->fa_blockv[mid] = result;
				++self->fa_blockc;
			}
			return result;
		}
	}
	DeeError_Throwf(&DeeError_SegFault, "Out-of-bounds text location %#.4" PRFx32 " accessed",
	                Dee_function_assembler_addrof(self, deemon_addr));
err:
	return NULL;
}

/* Locate the basic block that contains `deemon_addr'
 * @return: * :   The basic block in question.
 * @return: NULL: Address is out-of-bounds. */
INTERN WUNUSED NONNULL((1)) struct Dee_basic_block *DCALL
Dee_function_assembler_locateblock(struct Dee_function_assembler const *__restrict self,
                                   Dee_instruction_t const *deemon_addr) {
	size_t lo = 0;
	size_t hi = self->fa_blockc;
	ASSERT(self->fa_blockc <= self->fa_blocka);
	while (lo < hi) {
		struct Dee_basic_block *result;
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

/* Lookup/allocate an exception-exit basic block that can be used to clean
 * up `state' and then return `NULL' to the caller of the generated function.
 * @return: * :   The basic block to which to jump in order to clean up `state'.
 * @return: NULL: Error. */
INTERN WUNUSED NONNULL((1, 2)) struct Dee_basic_block *DCALL
Dee_function_assembler_except_exit(struct Dee_function_assembler *__restrict self,
                                   struct Dee_memstate *__restrict state) {
	size_t lo, hi;
	size_t infosize = Dee_except_exitinfo_sizeof(state->ms_host_cfa_offset);
	struct Dee_basic_block *result;
	struct Dee_except_exitinfo *info;
#ifdef Dee_Alloca
	info = (struct Dee_except_exitinfo *)Dee_Alloca(infosize);
#else /* Dee_Alloca */
	info = Dee_except_exitinfo_alloc(infosize);
	if unlikely(!info)
		goto err;
#endif /* !Dee_Alloca */

	/* Fill in info. */
	if unlikely(Dee_except_exitinfo_init(info, state))
		goto err_info_alloca;

	/* Check if we already have a block for this state. */
	lo = 0;
	hi = self->fa_except_exitc;
	while (lo < hi) {
		int diff;
		struct Dee_except_exitinfo *oldinfo;
		size_t mid = (lo + hi) / 2;
		oldinfo = self->fa_except_exitv[mid];
		ASSERT(oldinfo);
		ASSERT(oldinfo->exi_block);
		diff = Dee_except_exitinfo_cmp(info, oldinfo);
		if (diff < 0) {
			hi = mid;
		} else if (diff > 0) {
			lo = mid + 1;
		} else {
			/* Found it! */
#ifndef Dee_Alloca
			Dee_except_exitinfo_free(info);
#endif /* !Dee_Alloca */
			return oldinfo->exi_block;
		}
	}
	ASSERT(lo == hi);

	/* Need to insert a new exit information descriptor. */
#ifdef Dee_Alloca
	{
		struct Dee_except_exitinfo *heapinfo;
		heapinfo = Dee_except_exitinfo_alloc(infosize);
		if unlikely(!heapinfo)
			goto err_info_alloca;
		info = (struct Dee_except_exitinfo *)memcpy(heapinfo, info, infosize);
	}
#endif /* Dee_Alloca */

	/* Allocate a basic block for `info'. */
	result = Dee_basic_block_alloc();
	if unlikely(!result)
		goto err_info;
	info->exi_block = result;
	Dee_basic_block_init_common(result);
	Dee_jump_descriptors_init(&result->bb_exits);
	result->bb_deemon_start = NULL; /* No deemon code here */
	result->bb_deemon_end   = NULL;
	result->bb_mem_start    = state; /* Remember (some) state when jumping to this block. */
	Dee_memstate_incref(state);

	/* Make sure there is enough space in the sorted list of exit information descriptors. */
	ASSERT(self->fa_except_exitc <= self->fa_except_exita);
	if unlikely(self->fa_except_exitc >= self->fa_except_exita) {
		struct Dee_except_exitinfo **new_list;
		size_t new_alloc = self->fa_except_exita * 2;
		size_t min_alloc = self->fa_except_exitc + 1;
		if (new_alloc < 8)
			new_alloc = 8;
		if (new_alloc < min_alloc)
			new_alloc = min_alloc;
		new_list = (struct Dee_except_exitinfo **)Dee_TryReallocc(self->fa_except_exitv,
		                                                          new_alloc,
		                                                          sizeof(struct Dee_except_exitinfo *));
		if unlikely(!new_list) {
			new_alloc = min_alloc;
			new_list = (struct Dee_except_exitinfo **)Dee_Reallocc(self->fa_except_exitv,
			                                                       new_alloc,
			                                                       sizeof(struct Dee_except_exitinfo *));
			if unlikely(!new_list)
				goto err_info_result;
		}
		self->fa_except_exitv = new_list;
		self->fa_except_exita = new_alloc;
	}

	/* Insert the new info-descriptor at the appropriate location (`lo'). */
	memmoveupc(&self->fa_except_exitv[lo + 1],
	           &self->fa_except_exitv[lo],
	           self->fa_except_exitc - lo,
	           sizeof(struct Dee_except_exitinfo *));
	self->fa_except_exitv[lo] = info;
	++self->fa_except_exitc;

	return result;
err_info_result:
	Dee_basic_block_free(result);
err_info:
#ifdef Dee_Alloca
	Dee_except_exitinfo_free(info);
#endif /* Dee_Alloca */
err_info_alloca:
#ifndef Dee_Alloca
	Dee_except_exitinfo_free(info);
err:
#endif /* !Dee_Alloca */
	return NULL;
}

/* Allocate a new host text symbol and return it.
 * @return: * :   The newly allocated host text symbol
 * @return: NULL: Error */
INTERN WUNUSED NONNULL((1)) struct Dee_host_symbol *DCALL
Dee_function_assembler_newsym(struct Dee_function_assembler *__restrict self) {
	struct Dee_host_symbol *result = _Dee_host_symbol_alloc();
	if likely(result) {
		result->hs_type  = DEE_HOST_SYMBOL_UNDEF;
		result->_hs_next = self->fa_symbols;
		self->fa_symbols = result;
	}
	return result;
}





/* Error throwing helper functions. */
INTERN ATTR_COLD int DCALL err_illegal_stack_effect(void) {
	return DeeError_Throwf(&DeeError_SegFault, "Illegal stack effect");
}
INTERN ATTR_COLD int DCALL err_illegal_lid(uint16_t lid) {
	return DeeError_Throwf(&DeeError_SegFault, "Illegal local variable ID: %#" PRFx16, lid);
}
INTERN ATTR_COLD int DCALL err_illegal_mid(uint16_t mid) {
	return DeeError_Throwf(&DeeError_SegFault, "Illegal module ID: %#" PRFx16, mid);
}
INTERN ATTR_COLD int DCALL err_illegal_aid(uint16_t aid) {
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


DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_COMMON_C */
