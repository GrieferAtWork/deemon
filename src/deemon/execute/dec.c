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
#ifndef GUARD_DEEMON_EXECUTE_DEC_C
#define GUARD_DEEMON_EXECUTE_DEC_C 1

#include <deemon/api.h>
#include <deemon/dec.h>
#include <deemon/object.h>

#ifndef CONFIG_NO_DEC
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
#include <deemon/alloc.h>
#include <deemon/dex.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/gc.h>
#include <deemon/heap.h>
#include <deemon/module.h>
#include <deemon/string.h>
#include <deemon/system.h>

#include <hybrid/align.h>
#include <hybrid/overflow.h>
#include <hybrid/sequence/bsearch.h>

#undef byte_t
#define byte_t __BYTE_TYPE__
#undef container_of
#define container_of COMPILER_CONTAINER_OF

DECL_BEGIN

/* Destructor linked into `struct Dee_heapregion' for dec file mappings. */
PRIVATE NONNULL((1)) void DCALL
DeeDec_heapregion_destroy(struct Dee_heapregion *__restrict self) {
	Dec_Ehdr *ehdr = container_of(self, Dec_Ehdr, e_heap);
	/* Finalize the dec file's file mapping (which will cause the mapping to be unloaded) */
	DeeMapFile_Fini(&ehdr->e_mapping);
}


/* Must relocate all symbols from the deemon core against the
 * address of the deemon module itself (since that one's also
 * allocated statically) */
#define DeeDeemonModule_GetRelBase() \
	(uintptr_t)DeeModule_GetDeemon()


PRIVATE WUNUSED NONNULL((1)) uintptr_t DCALL
DeeNativeModule_GetRelBase(DeeModuleObject *__restrict self) {
	struct Dee_heapregion *region;
#ifndef CONFIG_NO_DEX
	if (DeeDex_Check(self)) {
		DeeDexObject *me = (DeeDexObject *)self;
		return (uintptr_t)me->d_dex;
	}
#endif /* !CONFIG_NO_DEX */
	ASSERT(self == DeeModule_GetDeemon());
	return DeeDeemonModule_GetRelBase();
}

PRIVATE WUNUSED NONNULL((1)) uintptr_t DCALL
DeeModule_GetRelBase(DeeModuleObject *__restrict self) {
	struct Dee_heapregion *region;
	struct gc_head *self_gc_head;
	Dec_Ehdr *ehdr;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);

#ifndef CONFIG_NO_DEX
	if (DeeDex_Check(self)) {
		DeeDexObject *me = (DeeDexObject *)self;
		return (uintptr_t)me->d_dex;
	}
#endif /* !CONFIG_NO_DEX */
	if (self == DeeModule_GetDeemon())
		return DeeDeemonModule_GetRelBase();

	/* Dynamically allocated (user) module */
	ASSERT(DeeType_IsGC(Dee_TYPE(self)));
	self_gc_head = DeeGC_Head(self);
	region = DeeHeap_GetRegionOf(self_gc_head);
	ASSERT(region);
	ASSERT(region->hr_destroy == &DeeDec_heapregion_destroy);
	ehdr = container_of(self, Dec_Ehdr, e_heap);
	return (uintptr_t)ehdr;
}


/* Initialize/finalize a dec writer. Note that unlike usual, `DeeDecWriter_Init()'
 * already needs to allocate a small amount of heap memory, meaning it can actually
 * fail due to OOM and do so by returning `-1'
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeDecWriter_Init(DeeDecWriter *__restrict self) {
	self->dw_ehdr = (Dec_Ehdr *)Dee_TryMalloc(sizeof(Dec_Ehdr) + (64 * 1024));
	if unlikely(!self->dw_ehdr) {
		self->dw_ehdr = (Dec_Ehdr *)Dee_Malloc(sizeof(Dec_Ehdr) + sizeof(struct Dee_heaptail));
		if unlikely(!self->dw_ehdr)
			goto err;
	}
	self->dw_alloc = Dee_MallocUsableSize(self->dw_ehdr);
	/* The header struct ends at the start of the first heap chunk's user-area */
	self->dw_used  = sizeof(Dec_Ehdr);
	self->dw_hlast = 0;
	self->dw_srel.drlt_relv  = NULL;
	self->dw_srel.drlt_relc  = 0;
	self->dw_srel.drlt_rela  = 0;
	self->dw_drel.drlt_relv  = NULL;
	self->dw_drel.drlt_relc  = 0;
	self->dw_drel.drlt_rela  = 0;
	self->dw_drrel.drlt_relv = NULL;
	self->dw_drrel.drlt_relc = 0;
	self->dw_drrel.drlt_rela = 0;
	self->dw_deps.ddpt_depv  = NULL;
	self->dw_deps.ddpt_depc  = 0;
	self->dw_deps.ddpt_depa  = 0;
	self->dw_fdeps.dfdt_depv = NULL;
	self->dw_fdeps.dfdt_depc = 0;
	self->dw_fdeps.dfdt_depa = 0;
	self->dw_gchead = 0;
	self->dw_gctail = 0;

	/* Allocate an empty table for the used-object map */
	self->dw_known.dot_list = (struct Dee_dec_objtab_entry *)Dee_Calloc(sizeof(struct Dee_dec_objtab_entry));
	if unlikely(!self->dw_known.dot_list)
		goto err_ehdr;
	self->dw_known.dot_mask = 0;
	self->dw_known.dot_size = 0;
	return 0;
err_ehdr:
	Dee_Free(self->dw_ehdr);
err:
	return -1;
}

PUBLIC NONNULL((1)) void DCALL
DeeDecWriter_Fini(DeeDecWriter *__restrict self) {
	size_t i;
	for (i = 0; i < self->dw_deps.ddpt_depc; ++i) {
		struct Dee_dec_depmod *dep = &self->dw_deps.ddpt_depv[i];
		Dee_Decref_unlikely(dep->ddm_mod);
		Dee_Free(dep->ddm_rel.drlt_relv);
		Dee_Free(dep->ddm_rrel.drlt_relv);
		Dee_Free(dep->ddm_impstr);
	}
	Dee_Free(self->dw_base);
	Dee_Free(self->dw_srel.drlt_relv);
	Dee_Free(self->dw_drrel.drlt_relv);
	Dee_Free(self->dw_drel.drlt_relv);
	Dee_Free(self->dw_deps.ddpt_depv);
	Dee_Free(self->dw_fdeps.dfdt_depv);
	Dee_Free(self->dw_known.dot_list);
}



/* Generate import strings for module dependencies (s.a. `struct Dee_dec_depmod::ddm_impstr') */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeDecWriter_GenImpStr(DeeDecWriter *__restrict self,
                       /*utf-8*/ char const *dec_dirname,
                       size_t dec_dirname_len) {
	size_t i;
	ASSERT(dec_dirname_len);
	ASSERT(DeeSystem_IsSep(dec_dirname[dec_dirname_len - 1]));
	for (i = 0; i < self->dw_deps.ddpt_depc; ++i) {
		struct Dee_dec_depmod *dep = &self->dw_deps.ddpt_depv[i];
		DeeModuleObject *mod = dep->ddm_mod;
		ASSERT(!dep->ddm_impstr);
//		dep->ddm_impstr = ; /* TODO */
	}
	return 0;
err:
	return -1;
}


/* Pack the dec file into a format where it can easily be written to a file:
 * >> DeeDec_Ehdr *ehdr = DeeDecWriter_PackMapping(&writer);
 * >> DeeFile_WriteAll(fp, ehdr, ehdr->e_offsetof_eof);
 * >> Dee_Free(e_offsetof_eof);
 *
 * The returned pointer should either:
 * - be free'd using `Dee_Free(return)'
 * - be passed to `DeeDecWriter_PackModule()'
 *   to turn it into a `DeeModuleObject'
 *
 * @param: dec_dirname: Absolute directory where the `.dec' file will go.
 *                      **MUST** end with a path separator (`DeeSystem_IsSep')
 * @return: * :   The not-yet-relocated dec file (header + contents)
 * @return: NULL: An error was thrown */
PUBLIC WUNUSED NONNULL((1)) DeeDec_Ehdr *DCALL
DeeDecWriter_PackMapping(DeeDecWriter *__restrict self,
                         /*utf-8*/ char const *dec_dirname, size_t dec_dirname_len) {
	Dec_Ehdr *ehdr = self->dw_ehdr;
	size_t i;
	Dee_dec_addr32_t total_used, total_need;
	Dee_dec_addr32_t addrof_zero;
	Dee_dec_addr32_t addrof_modrel; /* Start address for relocation tables pointed to by "Dec_Dhdr" entries. */
	Dee_dec_addr32_t addrof_modstr; /* Start address of `d_offsetof_modname' string table (possibly unaligned) */

	/* Generate `ddm_impstr' strings for dependencies. */
	if (DeeDecWriter_GenImpStr(self, dec_dirname, dec_dirname_len))
		goto err;

	total_used = (Dee_dec_addr32_t)self->dw_used;
	ehdr->e_ident[DI_MAG0] = DECMAG0;
	ehdr->e_ident[DI_MAG1] = DECMAG1;
	ehdr->e_ident[DI_MAG2] = DECMAG2;
	ehdr->e_ident[DI_MAG3] = DECMAG3;
	ehdr->e_mach = Dee_DEC_MACH;
	ehdr->e_heapoff = offsetof(Dec_Ehdr, e_heap);
	ehdr->e_version = DVERSION_CUR;
	ehdr->e_build_timestamp = DeeSystem_GetWalltime();
	ehdr->e_deemon_timestamp = DeeExec_GetTimestamp();
//	ehdr->e_deemon_build_id; /* TODO */
//	ehdr->e_deemon_host_id; /* TODO */
	ehdr->e_offsetof_gchead = self->dw_gchead;
	ehdr->e_offsetof_gctail = self->dw_gctail;

	/* Space for the heap tail is always pre-allocated! */
	{
		size_t offsetof_tail = self->dw_used;
		struct Dee_heaptail *tail = (struct Dee_heaptail *)((byte_t *)ehdr + offsetof_tail);
		addrof_zero = (Dee_dec_addr32_t)(offsetof_tail + offsetof(struct Dee_heaptail, ht_zero));
		offsetof_tail += sizeof(struct Dee_heaptail);
		tail->ht_lastsize = self->dw_hlast;
		tail->ht_zero     = 0;
		ehdr->e_heap.hr_size = offsetof_tail - offsetof(Dec_Ehdr, e_heap);
	}

	/* Calculate the total needed buffer size. */
	total_need = (Dee_dec_addr32_t)self->dw_used;
	total_need += sizeof(struct Dee_heaptail); /* Heap tail */

	/* Module dependency tables. */
	if (self->dw_deps.ddpt_depc) {
		ehdr->e_offsetof_deps = total_need;
		total_need += self->dw_deps.ddpt_depc * sizeof(Dec_Dhdr);
		/* Add space for "terminated by a d_offsetof_modname==0-entry" */
		total_need += COMPILER_OFFSETAFTER(Dec_Dhdr, d_offsetof_modname);
	} else {
		ehdr->e_offsetof_deps = addrof_zero - offsetof(Dec_Dhdr, d_offsetof_modname);
	}

	/* Relocations against self. */
	if (self->dw_srel.drlt_relc) {
		ehdr->e_offsetof_srel = total_need;
		total_need += (self->dw_srel.drlt_relc + 1) * sizeof(Dee_dec_addr32_t);
	} else {
		ehdr->e_offsetof_srel = addrof_zero;
	}

	/* Relocations against deemon. */
	if (self->dw_drel.drlt_relc) {
		ehdr->e_offsetof_drel = total_need;
		total_need += (self->dw_drel.drlt_relc + 1) * sizeof(Dee_dec_addr32_t);
	} else {
		ehdr->e_offsetof_drel = addrof_zero;
	}
	if (self->dw_drrel.drlt_relc) {
		ehdr->e_offsetof_drrel = total_need;
		total_need += (self->dw_drrel.drlt_relc + 1) * sizeof(Dee_dec_addr32_t);
	} else {
		ehdr->e_offsetof_drrel = addrof_zero;
	}

	/* Relocation tables for dependencies. */
	addrof_modrel = total_need;
	for (i = 0; i < self->dw_deps.ddpt_depc; ++i) {
		struct Dee_dec_depmod *dep = &self->dw_deps.ddpt_depv[i];
		if (dep->ddm_rel.drlt_relc)
			total_need += (dep->ddm_rel.drlt_relc + 1) * sizeof(Dee_dec_addr32_t);
		if (dep->ddm_rrel.drlt_relc)
			total_need += (dep->ddm_rrel.drlt_relc + 1) * sizeof(Dee_dec_addr32_t);
	}

	/* String tables */
	addrof_modstr = total_need;
	for (i = 0; i < self->dw_deps.ddpt_depc; ++i) {
		struct Dee_dec_depmod *dep = &self->dw_deps.ddpt_depv[i];
		ASSERT(dep->ddm_impstr);
		total_need = CEIL_ALIGN(total_need, __ALIGNOF_SIZE_T__);
		total_need += offsetof(Dec_Dstr, ds_string);
		total_need += dep->ddm_impstr->ds_length;
	}

	/* Additional file dependencies */
	if (self->dw_fdeps.dfdt_depc) {
		total_need = CEIL_ALIGN(total_need, __ALIGNOF_SIZE_T__);
		ehdr->e_offsetof_files = total_need;
		total_need += self->dw_fdeps.dfdt_depc;
		total_need = CEIL_ALIGN(total_need, __ALIGNOF_SIZE_T__);
		total_need += COMPILER_OFFSETAFTER(Dec_Dstr, ds_length); /* For trailing 0 */
	} else {
		ehdr->e_offsetof_files = 0;
	}

	/* EOF marker */
	ehdr->e_offsetof_eof = total_need;

	/* Resize the main buffer to fit. */
	ehdr = (Dec_Ehdr *)Dee_Realloc(ehdr, total_need);
	if unlikely(!ehdr)
		goto err;
	self->dw_ehdr = ehdr;

	/* With the buffer resized to its final size, and offsets all determined, copy data. */

	/* Module dependency tables. */
	if (self->dw_deps.ddpt_depc) {
		size_t i;
		Dec_Dstr *out_name;
		Dec_Dhdr *out_deps = (Dec_Dhdr *)((byte_t *)ehdr + ehdr->e_offsetof_deps);
		Dee_dec_addr32_t addrof_outname = addrof_modstr;
		Dee_dec_addr32_t addrof_outrel  = addrof_modrel;
		for (i = 0; i < self->dw_deps.ddpt_depc; ++i, ++out_deps) {
			struct Dee_dec_depmod *dep = &self->dw_deps.ddpt_depv[i];

			/* Emit relocations (noref) */
			if (dep->ddm_rel.drlt_relc) {
				Dee_dec_addr32_t *out_rel = (Dee_dec_addr32_t *)((byte_t *)ehdr + addrof_outrel);
				out_deps->d_offsetof_mrel = addrof_outrel;
				out_rel = (Dee_dec_addr32_t *)mempcpyc(out_rel, dep->ddm_rel.drlt_relv,
				                                       dep->ddm_rel.drlt_relc,
				                                       sizeof(Dee_dec_addr32_t));
				*out_rel = 0;
				addrof_outrel += (dep->ddm_rel.drlt_relc + 1) * sizeof(Dee_dec_addr32_t);
			} else {
				out_deps->d_offsetof_mrel = addrof_zero;
			}

			/* Emit relocations (ref) */
			if (dep->ddm_rrel.drlt_relc) {
				Dee_dec_addr32_t *out_rel = (Dee_dec_addr32_t *)((byte_t *)ehdr + addrof_outrel);
				out_deps->d_offsetof_mrrel = addrof_outrel;
				out_rel = (Dee_dec_addr32_t *)mempcpyc(out_rel, dep->ddm_rrel.drlt_relv,
				                                       dep->ddm_rrel.drlt_relc,
				                                       sizeof(Dee_dec_addr32_t));
				*out_rel = 0;
				addrof_outrel += (dep->ddm_rrel.drlt_relc + 1) * sizeof(Dee_dec_addr32_t);
			} else {
				out_deps->d_offsetof_mrrel = addrof_zero;
			}

			/* Emit module name */
			addrof_outname = CEIL_ALIGN(addrof_outname, __ALIGNOF_SIZE_T__);
			out_name = (Dec_Dstr *)((byte_t *)ehdr + addrof_outname);
			out_name->ds_length = dep->ddm_impstr->ds_length;
			memcpy(out_name->ds_string, dep->ddm_impstr->ds_string, out_name->ds_length);
			out_deps->d_offsetof_modname = addrof_outname;
			addrof_outname += offsetof(Dec_Dstr, ds_string);
			addrof_outname += out_name->ds_length;
		}
		addrof_outname = CEIL_ALIGN(addrof_outname, __ALIGNOF_SIZE_T__);
		out_name = (Dec_Dstr *)((byte_t *)ehdr + addrof_outname);
		out_name->ds_length = 0; /* "terminated by a d_offsetof_modname==0-entry" */
	}

	/* Relocations against self. */
	if (self->dw_srel.drlt_relc) {
		Dee_dec_addr32_t *out_rel = (Dee_dec_addr32_t *)((byte_t *)ehdr + ehdr->e_offsetof_srel);
		out_rel = (Dee_dec_addr32_t *)mempcpyc(out_rel, self->dw_srel.drlt_relv,
		                                       self->dw_srel.drlt_relc,
		                                       sizeof(Dee_dec_addr32_t));
		*out_rel = 0;
	}

	/* Relocations against deemon. */
	if (self->dw_drel.drlt_relc) {
		Dee_dec_addr32_t *out_rel = (Dee_dec_addr32_t *)((byte_t *)ehdr + ehdr->e_offsetof_drel);
		out_rel = (Dee_dec_addr32_t *)mempcpyc(out_rel, self->dw_drel.drlt_relv,
		                                       self->dw_drel.drlt_relc,
		                                       sizeof(Dee_dec_addr32_t));
		*out_rel = 0;
	}
	if (self->dw_drrel.drlt_relc) {
		Dee_dec_addr32_t *out_rel = (Dee_dec_addr32_t *)((byte_t *)ehdr + ehdr->e_offsetof_drrel);
		out_rel = (Dee_dec_addr32_t *)mempcpyc(out_rel, self->dw_drrel.drlt_relv,
		                                       self->dw_drrel.drlt_relc,
		                                       sizeof(Dee_dec_addr32_t));
		*out_rel = 0;
	}

	/* Additional file dependencies */
	if (self->dw_fdeps.dfdt_depc) {
		byte_t *out_deps = (byte_t *)ehdr + ehdr->e_offsetof_files;
		memcpy(out_deps, self->dw_fdeps.dfdt_depv, self->dw_fdeps.dfdt_depc);
		out_deps += CEIL_ALIGN(self->dw_fdeps.dfdt_depc, __ALIGNOF_SIZE_T__);
		((Dec_Dstr *)out_deps)->ds_length = 0; /* For trailing 0 */
	}

	/* Steal the ehdr from `self' */
	self->dw_ehdr  = NULL;
	self->dw_used  = 0;
	self->dw_alloc = 0;
	return ehdr;
err:
	return NULL;
}

/* Slightly more efficient convenience wrapper for:
 * >> DeeDec_Relocate(DeeDecWriter_PackMapping(self)) */
PUBLIC WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeDecWriter_PackModule(DeeDecWriter *__restrict self) {
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}


/* Add an additional file dependency to `self'. The given `filename' must
 * be relative to the directory that the `.dec' file will eventually reside
 * within. By default, only the relevant `.dee' file will be a dependency
 * of the produced `.dec' file.
 * @return: 0 : Success
 * @return: -1: Error */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeDecWriter_AddFileDep(DeeDecWriter *__restrict self,
                        char const *filename,
                        size_t filename_len) {
	Dec_Dstr *dst;
	size_t old_size = CEIL_ALIGN(self->dw_fdeps.dfdt_depc, __ALIGNOF_SIZE_T__);
	size_t min_size = old_size + sizeof(size_t) + filename_len * sizeof(char);
	ASSERTF(filename_len, "Empty string cannot be appended -- that one is used internally to indicate EOF");
	if (self->dw_fdeps.dfdt_depa < min_size) {
		byte_t *new_vector;
		size_t new_alloc = self->dw_fdeps.dfdt_depa * 2;
		if (new_alloc < min_size) {
			new_alloc = min_size * 2;
			if (new_alloc < min_size)
				new_alloc = min_size;
		}
		new_vector = (byte_t *)Dee_TryRealloc(self->dw_fdeps.dfdt_depv, new_alloc);
		if unlikely(!new_vector) {
			new_alloc = min_size;
			new_vector = (byte_t *)Dee_Realloc(self->dw_fdeps.dfdt_depv, new_alloc);
			if unlikely(!new_vector)
				goto err;
		}
		self->dw_fdeps.dfdt_depv = new_vector;
		self->dw_fdeps.dfdt_depa = new_alloc;
	}
	dst = (Dec_Dstr *)(self->dw_fdeps.dfdt_depv + old_size);
	dst->ds_length = filename_len;
	memcpy(dst->ds_string, filename, filename_len * sizeof(char));
	self->dw_fdeps.dfdt_depc = min_size;
	return 0;
err:
	return -1;
}


/* Append a relocation to "self" */
PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_dec_reltab_append(struct Dee_dec_reltab *__restrict self, Dee_dec_addr32_t addr) {
	ASSERT(self->drlt_relc <= self->drlt_rela);
	if (self->drlt_relc >= self->drlt_rela) {
		Dee_dec_addr32_t *new_relv;
		size_t new_alloc = self->drlt_rela * 2;
		if (new_alloc < 16)
			new_alloc = 16;
		new_relv = (Dee_dec_addr32_t *)Dee_TryReallocc(self->drlt_relv, new_alloc, sizeof(Dee_dec_addr32_t));
		if unlikely(!new_relv) {
			new_alloc = self->drlt_relc + 1;
			new_relv  = (Dee_dec_addr32_t *)Dee_Reallocc(self->drlt_relv, new_alloc, sizeof(Dee_dec_addr32_t));
			if unlikely(!new_relv)
				goto err;
		}
		self->drlt_relv = new_relv;
		self->drlt_rela = new_alloc;
	}
	self->drlt_relv[self->drlt_relc++] = addr;
	return 0;
err:
	return -1;
}

/* Add an entry for `ref' to `self->dw_known' */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeDecWriter_AddKnown(DeeDecWriter *__restrict self,
                      DeeObject *__restrict ref,
                      Dee_dec_addr_t addr) {
	Dee_hash_t hash = DeeObject_HashGeneric(ref); /* Always hash by-addr */
	Dee_hash_t i, perturb, hash;
	perturb = i = Dee_dec_objtab_hashst(&self->dw_known, hash);
	for (;; Dee_dec_objtab_hashnx(i, perturb)) {

	}
	self->dw_known;
	/* TODO */
	return DeeError_NOTIMPLEMENTED();
}

/* Check if `ref' was already written to "self". If so,
 * return the address where it resides. If not, return "0" */
PRIVATE WUNUSED NONNULL((1, 2)) Dee_dec_addr_t DCALL
DeeDecWriter_GetKnown(DeeDecWriter *__restrict self,
                      DeeObject *__restrict ref) {
	Dee_hash_t hash = DeeObject_HashGeneric(ref); /* Always hash by-addr */
	Dee_hash_t i, perturb;
	perturb = i = Dee_dec_objtab_hashst(&self->dw_known, hash);
	for (;; Dee_dec_objtab_hashnx(i, perturb)) {
		struct Dee_dec_objtab_entry *item;
		item = Dee_dec_objtab_hashit(&self->dw_known, i);
		if (item->dote_obj == ref)
			return item->dote_off;
		if (!item->dote_obj)
			break;
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeDecWriter_ResizeDeps(DeeDecWriter *__restrict self) {
	ASSERT(self->dw_deps.ddpt_depc <= self->dw_deps.ddpt_depa);
	if (self->dw_deps.ddpt_depc >= self->dw_deps.ddpt_depa) {
		struct Dee_dec_depmod *new_relv;
		size_t new_alloc = self->dw_deps.ddpt_depa * 2;
		if (new_alloc < 16)
			new_alloc = 16;
		new_relv = (struct Dee_dec_depmod *)Dee_TryReallocc(self->dw_deps.ddpt_depv, new_alloc, sizeof(struct Dee_dec_depmod));
		if unlikely(!new_relv) {
			new_alloc = self->dw_deps.ddpt_depc + 1;
			new_relv  = (struct Dee_dec_depmod *)Dee_Reallocc(self->dw_deps.ddpt_depv, new_alloc, sizeof(struct Dee_dec_depmod));
			if unlikely(!new_relv)
				goto err;
		}
		self->dw_deps.ddpt_depv = new_relv;
		self->dw_deps.ddpt_depa = new_alloc;
	}
	return 0;
err:
	return -1;
}

/* Find an existing dependency on "mod", and if none exists, add one.
 * @return: * :   The dependent module descriptor for `mod'
 * @return: NULL: An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2)) struct Dee_dec_depmod *DCALL
DeeDecWriter_GetDep(DeeDecWriter *__restrict self,
                    DeeModuleObject *__restrict mod) {
	size_t index;
	struct Dee_dec_depmod *dst;
	BSEARCH (index, self->dw_deps.ddpt_depv, self->dw_deps.ddpt_depc, .ddm_mod, mod) {
		return &self->dw_deps.ddpt_depv[index];
	}
	if unlikely(DeeDecWriter_ResizeDeps(self))
		goto err;
	dst = &self->dw_deps.ddpt_depv[index];
	memmoveupc(dst + 1, dst, self->dw_deps.ddpt_depc - index, sizeof(*dst));
	++self->dw_deps.ddpt_depc;
	Dee_Incref(mod);
	dst->ddm_mod = mod;
	dst->ddm_rel.drlt_relv  = NULL;
	dst->ddm_rel.drlt_relc  = 0;
	dst->ddm_rel.drlt_rela  = 0;
	dst->ddm_rrel.drlt_relv = NULL;
	dst->ddm_rrel.drlt_relc = 0;
	dst->ddm_rrel.drlt_rela = 0;
	return dst;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) Dee_dec_addr_t
(DCALL DeeDecWriter_Malloc_impl)(DeeDecWriter *__restrict self,
                                 size_t num_bytes, bool try_malloc) {
	Dee_dec_addr_t result;
	struct Dee_heapchunk *chunk;
	size_t avail, used, nb, req;
	if unlikely(num_bytes > DFILE_LIMIT)
		goto err_too_much;

	/* Force alignment of new chunk size */
	num_bytes = CEIL_ALIGN(num_bytes, Dee_HEAPCHUNK_ALIGN);
	nb = num_bytes + sizeof(struct Dee_heapchunk);

	/* Force alignment of new chunk addr */
	used = CEIL_ALIGN(self->dw_used, Dee_HEAPCHUNK_ALIGN);

	/* Ensure that enough space has been allocated.
	 * Always include space for the eventual `struct Dee_heaptail'! */
	if (OVERFLOW_USUB(self->dw_alloc, used, &avail))
		avail = 0;
	req = nb + sizeof(struct Dee_heaptail);
	if likely(avail < req) {
		byte_t *new_base;
		size_t min_alloc = (self->dw_used + req);
		size_t new_alloc = CEIL_ALIGN(min_alloc * 2, __SIZEOF_POINTER__ * 4 * 1024);
		if (new_alloc < min_alloc)
			new_alloc = min_alloc;
		new_base = (byte_t *)Dee_TryRealloc(self->dw_base, new_alloc);
		if unlikely(!new_base) {
			new_alloc = min_alloc;
			new_base = try_malloc ? (byte_t *)Dee_TryRealloc(self->dw_base, new_alloc)
			                      : (byte_t *)Dee_Realloc(self->dw_base, new_alloc);
			if unlikely(!new_base)
				goto err;
		}
		self->dw_base  = new_base;
		self->dw_alloc = Dee_MallocUsableSize(new_base);
		ASSERT(self->dw_alloc >= new_alloc);
	}

	/* Initialize the heap chunk for the newly made allocation */
	result = (Dee_dec_addr_t)used;
	chunk  = DeeDecWriter_Addr2Mem(self, result, struct Dee_heapchunk);
	chunk->hc_prevsize = self->dw_hlast; /* Must be 0 the first time around */
	chunk->hc_head     = Dee_HEAPCHUNK_HEAD(num_bytes);
	result += sizeof(struct Dee_heapchunk);
	self->dw_used  = used + num_bytes;
	self->dw_hlast = Dee_HEAPCHUNK_PREV(num_bytes); /* Override with whatever the next chunk will need */
	return result;
err_too_much:
	if (!try_malloc)
		Dee_BadAlloc(num_bytes);
err:
	return 0;
}

/* Free a heap pointer
 * CAUTION: Only the most-recent pointer can *actually* be free'd!
 *          If you pass anything else, this function is a no-op! */
PUBLIC NONNULL((1)) void
(DCALL DeeDecWriter_Free)(DeeDecWriter *__restrict self, Dee_dec_addr_t addr) {
	size_t last_bytes = self->dw_hlast - Dee_HEAPCHUNK_PREV(0);
	Dee_dec_addr_t last = self->dw_used - last_bytes;
	struct Dee_heapchunk *p;
	if (last != addr)
		return; /* Cannot free... */
	/* Undo state changes made by allocation */
	addr -= sizeof(struct Dee_heapchunk);
	p = DeeDecWriter_Addr2Mem(self, addr, struct Dee_heapchunk);
	self->dw_used  = addr;
	self->dw_hlast = p->hc_prevsize;
}


/* Allocate heap memory within the dec file for:
 * - DeeDecWriter_Malloc:          Regular heap memory (as per `Dee_Malloc()')
 * - DeeDecWriter_Object_Malloc:   Object heap memory (as per `DeeObject_Malloc()')
 * - DeeDecWriter_GCObject_Malloc: GC Object heap memory (as per `DeeGCObject_Malloc()')
 * After calls to these functions, the caller is responsible for writing
 * to the newly allocated memory (s.a. `DeeDecWriter_Put*' functions below)
 *
 * NOTE: The Object-allocator functions will auto-initialize "ob_refcnt" and "ob_type"
 *       of the newly allocated object (alongside any other standard headers that may
 *       be present based on other CONFIG_* options)
 *
 * @param: ref: The object that's about to be written to `DeeDecWriter_Addr2Mem(self, return, DeeObject)'
 *              Needed for the sake of self-recursion such that any additional
 *              calls to `DeeDecWriter_PutObject()' for the same object "ref"
 *              will simply encode a self-referencing relocation against the
 *              already-written object, and to encode relocation for "ob_type".
 * @return: 0 : Allocation failed (an error was thrown)
 * @return: * : base address of user-area of new heap chunk */
PUBLIC WUNUSED NONNULL((1)) Dee_dec_addr_t
(DCALL DeeDecWriter_Malloc)(DeeDecWriter *__restrict self, size_t num_bytes) {
	return DeeDecWriter_Malloc_impl(self, num_bytes, false);
}

PUBLIC WUNUSED NONNULL((1)) Dee_dec_addr_t
(DCALL DeeDecWriter_TryMalloc)(DeeDecWriter *__restrict self, size_t num_bytes) {
	return DeeDecWriter_Malloc_impl(self, num_bytes, true);
}

PUBLIC WUNUSED NONNULL((1, 3)) Dee_dec_addr_t
(DCALL DeeDecWriter_Object_Malloc)(DeeDecWriter *__restrict self,
                                   size_t num_bytes,
                                   DeeObject *__restrict ref) {
	DeeObject *copy;
	Dee_dec_addr_t result;
	ASSERTF(!DeeType_IsGC(Dee_TYPE(ref)), "Use DeeDecWriter_GCObject_Malloc()");
	result = DeeDecWriter_Malloc(self, num_bytes);
	if unlikely(!result)
		goto err;
	if unlikely(DeeDecWriter_AddKnown(self, ref, result))
		goto err;

	/* Initialize "ob_refcnt" and "ob_type" of the newly allocated object */
	copy = DeeDecWriter_Addr2Mem(self, result, DeeObject);
	copy->ob_refcnt = 1;
#ifdef CONFIG_TRACE_REFCHANGES
	copy->ob_trace = NULL;
#endif /* CONFIG_TRACE_REFCHANGES */
	if unlikely(DeeDecWriter_PutObject(self, result + offsetof(DeeObject, ob_type),
	                                   (DeeObject *)Dee_TYPE(ref)))
		goto err;
	return result;
err:
	return 0;
}

PUBLIC WUNUSED NONNULL((1, 3)) Dee_dec_addr_t
(DCALL DeeDecWriter_GCObject_Malloc)(DeeDecWriter *__restrict self,
                                     size_t num_bytes,
                                     DeeObject *__restrict ref) {
	size_t total;
	Dee_dec_addr_t result;
	DeeObject *copy;
	ASSERTF(DeeType_IsGC(Dee_TYPE(ref)), "Use DeeDecWriter_Object_Malloc()");
	if (OVERFLOW_UADD(num_bytes, sizeof(struct gc_head_link), &total))
		total = (size_t)-1;
	result = DeeDecWriter_Malloc(self, num_bytes);
	if unlikely(!result)
		goto err;
	/* Initialize GC head/tail link pointers */
	ASSERT((self->dw_gchead == 0) ==
	       (self->dw_gctail == 0));
	if (self->dw_gctail == 0) {
		/* First GC object... */
		struct gc_head_link *link;
		self->dw_gchead = result;
		link = DeeDecWriter_Addr2Mem(self, result, struct gc_head_link);
		link->gc_next  = NULL;
		link->gc_pself = NULL;
	} else {
		/* Append to end of GC list. */
		if (DeeDecWriter_PutRel(self, self->dw_gctail + offsetof(struct gc_head_link, gc_next), result))
			goto err;
		if (DeeDecWriter_PutRel(self, result + offsetof(struct gc_head_link, gc_pself),
		                        self->dw_gctail + offsetof(struct gc_head_link, gc_next)))
			goto err;
	}
	self->dw_gctail = result;
	result += sizeof(struct gc_head_link);

	/* Initialize "ob_refcnt" and "ob_type" of the newly allocated object */
	copy = DeeDecWriter_Addr2Mem(self, result, DeeObject);
	copy->ob_refcnt = 1;
#ifdef CONFIG_TRACE_REFCHANGES
	copy->ob_trace = NULL;
#endif /* CONFIG_TRACE_REFCHANGES */
	if unlikely(DeeDecWriter_PutObject(self, result + offsetof(DeeObject, ob_type), Dee_TYPE(ref)))
		goto err;
	return result;
err:
	return 0;
}

/* Emit a relocation:
 * >> *DeeDecWriter_Addr2Mem(self, addrof_pointer, void *) =
 * >>     DeeDecWriter_Addr2Mem(self, addrof_target, void);
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeDecWriter_PutRel(DeeDecWriter *__restrict self,
                    Dee_dec_addr_t addrof_pointer,
                    Dee_dec_addr_t addrof_target) {
	void **pointer;
	pointer = DeeDecWriter_Addr2Mem(self, addrof_pointer, void *);
	*pointer = (void *)(uintptr_t)addrof_target;
	return Dee_dec_reltab_append(&self->dw_srel, (Dee_dec_addr32_t)addrof_pointer);
}

typedef WUNUSED_T NONNULL_T((1, 2)) Dee_dec_addr_t
(DCALL *Dee_tp_writedec_var_t)(struct Dee_dec_writer *__restrict writer,
                               DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int
(DCALL *Dee_tp_writedec_obj_t)(struct Dee_dec_writer *__restrict writer,
                               DeeObject *__restrict self,
                               Dee_dec_addr_t addr);

/* Append a copy of `obj' to self and return the address of the written `DeeObject'
 * @return: * : Address of the written `DeeObject'
 * @return: 0 : An error was thrown */
PRIVATE WUNUSED NONNULL((1, 2)) Dee_dec_addr_t DCALL
DeeDecWriter_AppendObject(DeeDecWriter *__restrict self,
                          DeeObject *__restrict obj) {
	int status;
	Dee_dec_addr_t addr;
	size_t instance_size;
	DeeTypeObject *tp = Dee_TYPE(obj);
	Dee_funptr_t tp_writedec;
	void (DCALL *tp_free)(void *__restrict ob);
	tp_writedec = DeeType_GetTpWriteDec(tp);
	if unlikely(!tp_writedec)
		goto err_cannot_serialize;
	if (tp->tp_flags & TP_FVARIABLE)
		return (*(Dee_tp_writedec_var_t)tp_writedec)(self, obj);

	/* Figure out instance size (with support for slab allocators). */
	tp_free = tp->tp_init.tp_alloc.tp_free;
	if (tp_free == NULL) {
		instance_size = tp->tp_init.tp_alloc.tp_instance_size;
	} else if (tp->tp_flags & TP_FGC) {
#define CHECK_ALLOCATOR(index, size)                  \
		if (tp_free == &DeeGCObject_SlabFree##size) { \
			instance_size = size * sizeof(void *);    \
		} else
		DeeSlab_ENUMERATE(CHECK_ALLOCATOR)
#undef CHECK_ALLOCATOR
		{
			goto err_cannot_serialize;
		}
	} else {
#define CHECK_ALLOCATOR(index, size)                \
		if (tp_free == &DeeObject_SlabFree##size) { \
			instance_size = size * sizeof(void *);  \
		} else
		DeeSlab_ENUMERATE(CHECK_ALLOCATOR)
#undef CHECK_ALLOCATOR
		{
			goto err_cannot_serialize;
		}
	}

	/* Allocate buffer for object. */
	addr = tp->tp_flags & TP_FGC
	       ? DeeDecWriter_GCObject_Malloc(self, instance_size, obj)
	       : DeeDecWriter_Object_Malloc(self, instance_size, obj);
	if unlikely(!addr)
		goto err;
	/* NOTE: Standard fields have already been initialized by "DeeDecWriter_[GC]Object_Malloc" */
	status = (*(Dee_tp_writedec_obj_t)tp_writedec)(self, obj, addr);
	if unlikely(status)
		goto err;
	return addr;
err_cannot_serialize:
	DeeRT_ErrCannotDecSerialize(obj);
err:
	return 0;
}

/* Encode a reference to `obj' at `DeeDecWriter_Addr2Mem(self, addr, DeeObject)'
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeDecWriter_PutObject)(DeeDecWriter *__restrict self,
                               Dee_dec_addr_t addr,
                               DeeObject *__restrict obj) {
	struct Dee_heapregion *region;
	DREF DeeModuleObject *mod;
	Dee_dec_addr_t known;
	Dee_dec_addr_t copy;
	void **pointer;
	void *obj_base;

	/* Check if "obj" has already been written */
	known = DeeDecWriter_GetKnown(self, obj);
	if (known != 0) {
		DeeObject *known_obj = DeeDecWriter_Addr2Mem(self, known, DeeObject);
		++known_obj->ob_refcnt; /* Because now there is another reference to this known object! */
		return DeeDecWriter_PutRel(self, addr, known);
	}

	/* Check if "obj" points into a dex module, or the deemon core */
	/* TODO: For this to work properly, dex modules must statically allocate their own "DeeModuleObject" objects! */
	mod = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(obj);
	if (mod) {
		struct Dee_dec_depmod *dep;
		uintptr_t relbase;
		if (mod == DeeModule_GetDeemon()) {
			Dee_DecrefNokill(mod);
			relbase = DeeDeemonModule_GetRelBase();
			pointer = DeeDecWriter_Addr2Mem(self, addr, void *);
			*pointer = (void *)((uintptr_t)obj - relbase);
			return Dee_dec_reltab_append(&self->dw_drrel, (Dee_dec_addr32_t)addr);
		}
		dep = DeeDecWriter_GetDep(self, mod);
		Dee_Decref_unlikely(mod);
		if unlikely(!dep)
			goto err;
		relbase = DeeNativeModule_GetRelBase(mod);
		pointer = DeeDecWriter_Addr2Mem(self, addr, void *);
		*pointer = (void *)((uintptr_t)obj - relbase);
		return Dee_dec_reltab_append(&dep->ddm_rrel, (Dee_dec_addr32_t)addr);
	}

	/* If "obj" is neither known, nor a native static symbol, then
	 * it **must** be a dynamically allocated object. Now there are
	 * only 2 sub-cases left:
	 * - It points into a custom heap region (meaning it's part of
	 *   another dec file mapping)
	 * - It is heap-allocated
	 *
	 * In either case: it *must* be a heap object! */
	obj_base = obj;
	if (DeeType_IsGC(Dee_TYPE(obj)))
		obj_base = DeeGC_Head(obj);
	region = DeeHeap_GetRegionOf(obj_base);
	if (region && region->hr_destroy == &DeeDec_heapregion_destroy) {
		/* Yes: the object belongs to a custom heap region, and that heap
		 *      region uses the custom dec-file-heap-region destructor,
		 *      meaning this object **DOES** belong to another dec file!
		 *
		 * But: if the module has already been destroyed, then we cannot
		 *      reference it! */
		Dec_Ehdr *ehdr = container_of(self, Dec_Ehdr, e_heap);
		struct gc_head *mod_gc_head = (struct gc_head *)(&ehdr->e_heap.hr_first + 1);
		mod = (DeeModuleObject *)DeeGC_Object(mod_gc_head);
		ASSERT(Dee_TYPE(mod) == &DeeModule_Type);
		if (Dee_IncrefIfNotZero(mod)) {
			struct Dee_dec_depmod *dep;
			ASSERT(DeeModule_GetRelBase(mod) == (uintptr_t)ehdr);
			dep = DeeDecWriter_GetDep(self, mod);
			Dee_Decref_unlikely(mod);
			if unlikely(!dep)
				goto err;
			pointer = DeeDecWriter_Addr2Mem(self, addr, void *);
			*pointer = (void *)((uintptr_t)obj - (uintptr_t)ehdr);
			return Dee_dec_reltab_append(&dep->ddm_rrel, (Dee_dec_addr32_t)addr);
		}
	}

	/* Fallback: must embed a copy of the object within the dec file. */
	copy = DeeDecWriter_AppendObject(self, obj);
	if unlikely(copy == 0)
		goto err;
	return DeeDecWriter_PutRel(self, addr, copy);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeDecWriter_PutObjectInherited)(DeeDecWriter *__restrict self,
                                        Dee_dec_addr_t addr,
                                        /*inherit(always)*/ DREF DeeObject *obj) {
	int result = DeeDecWriter_PutObject(self, addr, obj);
	Dee_Decref(obj);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeDecWriter_XPutObject)(DeeDecWriter *__restrict self,
                                Dee_dec_addr_t addr,
                                /*0..1*/ DeeObject *obj) {
	if (obj == NULL) {
		DREF DeeObject **p_obj = DeeDecWriter_Addr2Mem(self, addr, DREF DeeObject *);
		*p_obj = NULL;
		return 0;
	}
	return DeeDecWriter_PutObject(self, addr, obj);
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeDecWriter_XPutObjectInherited)(DeeDecWriter *__restrict self,
                                         Dee_dec_addr_t addr,
                                         /*0..1*/ DeeObject *obj) {
	if (obj == NULL) {
		DREF DeeObject **p_obj = DeeDecWriter_Addr2Mem(self, addr, DREF DeeObject *);
		*p_obj = NULL;
		return 0;
	}
	return DeeDecWriter_PutObjectInherited(self, addr, obj);
}

/* Create a duplicate of memory `data...+=num_bytes' and put an address to
 * this newly allocated copy at `DeeDecWriter_Addr2Mem(self, addr, void *)'
 * @return: * : Address of the duplicated memory (as also stored at `addr')
 * @return: 0 : An error was thrown */
PUBLIC WUNUSED ATTR_INS(3, 4) NONNULL((1)) Dee_dec_addr_t
(DCALL DeeDecWriter_PutMemDup)(DeeDecWriter *__restrict self, Dee_dec_addr_t addr,
                               void const *data, size_t num_bytes) {
	Dee_dec_addr_t md_addr = DeeDecWriter_Malloc(self, num_bytes);
	if likely(md_addr) {
		void *dst = DeeDecWriter_Addr2Mem(self, md_addr, void);
		memcpy(dst, data, num_bytes);
		if unlikely(DeeDecWriter_PutRel(self, addr, md_addr)) {
			DeeDecWriter_Free(self, md_addr);
			md_addr = 0;
		}
	}
	return md_addr;
}



/* Inplace-replace a object references with dec-encoded object references:
 * >> DREF DeeObject *obj = *DeeDecWriter_Addr2Mem(self, addr, DREF DeeObject *);
 * >> int result = DeeDecWriter_PutObject(self, addr, obj);
 * >> Dee_Decref(obj);
 * >> return result; */
PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeDecWriter_InplacePutObject)(DeeDecWriter *__restrict self,
                                      Dee_dec_addr_t addr) {
	int result;
	DREF DeeObject *obj;
	obj = *DeeDecWriter_Addr2Mem(self, addr, DREF DeeObject *);
	ASSERT_OBJECT(obj);
	result = DeeDecWriter_PutObject(self, addr, obj);
	Dee_Decref_unlikely(obj);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeDecWriter_XInplacePutObject)(DeeDecWriter *__restrict self,
                                       Dee_dec_addr_t addr) {
	int result = 0;
	DREF DeeObject *obj;
	obj = *DeeDecWriter_Addr2Mem(self, addr, DREF DeeObject *);
	ASSERT_OBJECT_OPT(obj);
	if (obj) {
		result = DeeDecWriter_PutObject(self, addr, obj);
		Dee_Decref_unlikely(obj);
	}
	return result;
}

/* Inplace-replace an array object references with dec-encoded object
 * references. Said array of object references is **ALWAYS** inherited:
 * >> size_t i;
 * >> for (i = 0; i < objc; ++i) {
 * >>     if (DeeDecWriter_InplacePutObject(self, addr)) {
 * >>         for (; i < objc; ++i) {
 * >>             Dee_Decref(*DeeDecWriter_Addr2Mem(self, addr, DREF DeeObject *));
 * >>             addr += sizeof(DREF DeeObject *);
 * >>         }
 * >>         return -1;
 * >>     }
 * >>     addr += sizeof(DREF DeeObject *);
 * >> }
 * >> return 0; */
PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeDecWriter_InplacePutObjectv)(DeeDecWriter *__restrict self,
                                       Dee_dec_addr_t addr, size_t objc) {
	DREF DeeObject **objv;
	while (objc) {
		if (DeeDecWriter_InplacePutObject(self, addr))
			goto err;
		addr += sizeof(DREF DeeObject *);
		--objc;
	}
	return 0;
err:
	objv = DeeDecWriter_Addr2Mem(self, addr, DREF DeeObject *);
	Dee_Decrefv_unlikely(objv, objc);
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeDecWriter_XInplacePutObjectv)(DeeDecWriter *__restrict self,
                                        Dee_dec_addr_t addr, size_t objc) {
	DREF DeeObject **objv;
	while (objc) {
		if (DeeDecWriter_XInplacePutObject(self, addr))
			goto err;
		addr += sizeof(DREF DeeObject *);
		--objc;
	}
	return 0;
err:
	objv = DeeDecWriter_Addr2Mem(self, addr, DREF DeeObject *);
	Dee_XDecrefv_unlikely(objv, objc);
	return -1;
}



/* Encode static pointers.
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeDecWriter_PutStaticPointer)(DeeDecWriter *__restrict self,
                                      Dee_dec_addr_t addr, void const *value) {
	DREF DeeModuleObject *mod;
	struct Dee_dec_depmod *dep;
	uintptr_t relbase;
	void **pointer;
	mod = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(value);
	if (mod == DeeModule_GetDeemon()) {
		Dee_DecrefNokill(mod);
		return DeeDecWriter_PutDeemonPointer(self, addr, value);
	}
	dep = DeeDecWriter_GetDep(self, mod);
	Dee_Decref_unlikely(mod);
	if unlikely(!dep)
		goto err;
	relbase = DeeNativeModule_GetRelBase(mod);
	pointer = DeeDecWriter_Addr2Mem(self, addr, void *);
	*pointer = (void *)((uintptr_t)value - relbase);
	return Dee_dec_reltab_append(&dep->ddm_rel, (Dee_dec_addr32_t)addr);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeDecWriter_XPutStaticPointer)(DeeDecWriter *__restrict self,
                                       Dee_dec_addr_t addr, void const *value) {
	if (value == NULL) {
		void **pointer = DeeDecWriter_Addr2Mem(self, addr, void *);
		*pointer = NULL;
		return 0;
	}
	return DeeDecWriter_PutStaticPointer(self, addr, value);
}

INTERN WUNUSED NONNULL((1, 3)) int
(DCALL DeeDecWriter_PutDeemonPointer)(DeeDecWriter *__restrict self,
                                      Dee_dec_addr_t addr, void const *value) {
	uintptr_t relbase = DeeDeemonModule_GetRelBase();
	void **pointer = DeeDecWriter_Addr2Mem(self, addr, void *);
	*pointer = (void *)((uintptr_t)value - relbase);
	return Dee_dec_reltab_append(&self->dw_drel, (Dee_dec_addr32_t)addr);
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeDecWriter_XPutDeemonPointer)(DeeDecWriter *__restrict self,
                                       Dee_dec_addr_t addr, void const *value) {
	if (value == NULL) {
		void **pointer = DeeDecWriter_Addr2Mem(self, addr, void *);
		*pointer = NULL;
		return 0;
	}
	return DeeDecWriter_PutDeemonPointer(self, addr, value);
}


/* Write the given module `mod' to the dec file. This function should
 * only ever be called once, and only on a freshly initialized dec
 * writer. This function will also recursively write all objects
 * referenced/reachable by `mod' into `self', and emit relocations.
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeDecWriter_AppendModule(DeeDecWriter *__restrict self,
                          DeeModuleObject *__restrict mod) {
	/* TODO: In order to module life-time to work properly, modules
	 *       need to have a custom tp_free() method that asserts that
	 *       the module itself lives as a heap-chunk within a custom
	 *       heap region, and must then do the same as Dee_Free()
	 *       does, but **NEVER** clobber the `ob_refcnt' field of the
	 *       module (which must stay set to `0' so that any other
	 *       object allocated within the module is able to query its
	 *       containing module and notice the 0 reference counter as
	 *       indicative of the module having been unloaded, but some
	 *       objects originating from that module still being alive) */
	Dee_dec_addr_t addr;
	ASSERT_OBJECT_TYPE_EXACT(mod, &DeeModule_Type); /* *_EXACT because it mustn't be a Dex module */
	addr = DeeDecWriter_AppendObject(self, (DeeObject *)mod);
	if unlikely(addr == 0)
		goto err;
	ASSERTF(addr == (offsetof(Dec_Ehdr, e_heap.hr_first) +
	                 sizeof(struct Dee_heapchunk) +
	                 DEE_GC_OBJECT_OFFSET),
	        "Module starts at wrong offset");
	return 0;
err:
	return -1;
}

/* Execute relocations on `self' and return a pointer to the
 * first object of the dec file's heap (which is always the
 * `DeeModuleObject' describing the dec file itself).
 *
 * On success, `self' is inherited by `return', such that rather
 * than calling `Dee_Free(self)', you must `Dee_Decref(return)'
 *
 * @return: * :   The module object described by `self'
 * @return: NULL: An error was thrown
 * @return: ITER_DONE: The DEC file was out of date or had been corrupted */
PUBLIC WUNUSED NONNULL((1, 2)) DREF struct Dee_module_object *DCALL
DeeDec_Relocate(/*inherit(on_success)*/ DeeDec_Ehdr *__restrict self,
                /*utf-8*/ char const *dec_dirname, size_t dec_dirname_len,
                struct Dee_compiler_options *options) {
	DREF DeeModuleObject *result;
	DREF DeeModuleObject **dependencies;
	Dec_Dhdr *dhdr = (Dec_Dhdr *)((byte_t *)self + self->e_offsetof_deps);
	size_t dep_count = 0;
	while (dhdr[dep_count].d_offsetof_modname)
		++dep_count;
	if (dep_count == 0) {
		dependencies = NULL;
	} else {
		dependencies = (DREF DeeModuleObject **)Dee_Mallocc(dep_count, sizeof(DREF DeeModuleObject *));
		if unlikely(!dependencies)
			goto err;
		for (dep_count = 0; dhdr->d_offsetof_modname; ++dep_count, ++dhdr) {
			DREF DeeObject *dep;
			Dec_Dstr *name = (Dec_Dstr *)((byte_t *)self + dhdr->d_offsetof_modname);
			dep = DeeModule_OpenRelativeString(name->ds_string, name->ds_length,
			                                   dec_dirname, dec_dirname_len,
			                                   options, true);
			if unlikely(!dep)
				goto err_dependencies_count;
			dependencies[dep_count] = (DREF DeeModuleObject *)dep;
		}
	}
	result = DeeDec_RelocateEx(self, dependencies);
	Dee_Decrefv(dependencies, dep_count);
	Dee_Free(dependencies);
	return result;
err_dependencies_count:
	Dee_Decrefv(dependencies, dep_count);
	Dee_Free(dependencies);
err:
	return NULL;
}


PRIVATE NONNULL((1)) void DCALL
apply_relocations(DeeDec_Ehdr *__restrict self,
                  Dee_dec_addr32_t offsetof_reltab,
                  uintptr_t relbase) {
	Dee_dec_addr32_t reladdr, *reltab;
	reltab = (Dee_dec_addr32_t *)((byte_t *)self + offsetof_reltab);
	while ((reladdr = *reltab++) != 0) {
		byte_t **pointer = (byte_t **)((byte_t *)self + reladdr);
		*pointer += relbase;
	}
}

PRIVATE NONNULL((1)) void DCALL
apply_relocations_with_incref(DeeDec_Ehdr *__restrict self,
                              Dee_dec_addr32_t offsetof_reltab,
                              uintptr_t relbase) {
	Dee_dec_addr32_t reladdr, *reltab;
	reltab = (Dee_dec_addr32_t *)((byte_t *)self + offsetof_reltab);
	while ((reladdr = *reltab++) != 0) {
		DeeObject *obj;
		byte_t **pointer = (byte_t **)((byte_t *)self + reladdr);
		obj = (DeeObject *)(*pointer += relbase);
		Dee_Incref(obj);
	}
}

/* Same as `DeeDec_Relocate()', but takes the list of dependent
 * modules as an already-populated array given by the caller.
 * @param: dependencies: Already-loaded array of dependent modules,
 *                       matching `self->e_offsetof_deps'
 *
 * @return: * :   The module object described by `self'
 * @return: NULL: An error was thrown
 * @return: ITER_DONE: The DEC file was out of date or had been corrupted */
PUBLIC ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeDec_RelocateEx(/*inherit(always)*/ DeeDec_Ehdr *__restrict self,
                  DeeModuleObject *const *dependencies) {
	size_t i;
	Dec_Dhdr *dhdr;
	DeeModuleObject *result;
	struct gc_head *first_gc;

	/* Self-relocations... */
	apply_relocations(self, self->e_offsetof_srel, (uintptr_t)self);

	/* Deemon-relocations... */
	apply_relocations(self, self->e_offsetof_drel, DeeDeemonModule_GetRelBase());
	apply_relocations_with_incref(self, self->e_offsetof_drrel, DeeDeemonModule_GetRelBase());

	/* Dependent-module-relocations... */
	dhdr = (Dec_Dhdr *)((byte_t *)self + self->e_offsetof_deps);
	for (i = 0; dhdr[i].d_offsetof_modname; ++i) {
		DeeModuleObject *mod = dependencies[i];
		uintptr_t mod_base = DeeModule_GetRelBase(mod);
		apply_relocations(self, dhdr[i].d_offsetof_mrel, mod_base);
		apply_relocations_with_incref(self, dhdr[i].d_offsetof_mrrel, mod_base);
	}

	/* At this point, the dec file should be fully initialized, and the
	 * first contained object should be the relevant DeeModuleObject! */
	first_gc = (struct gc_head *)(&self->e_heap.hr_first + 1);
	result = (DeeModuleObject *)DeeGC_Object(first_gc);
	if (Dee_TYPE(result) != &DeeModule_Type)
		goto fail;
	if (result->ob_refcnt == 0)
		goto fail;

	/* Link in GC objects (if there are any) */
	if (self->e_offsetof_gchead) {
		struct gc_head *gc_head = (struct gc_head *)((byte_t *)self + self->e_offsetof_gchead);
		struct gc_head *gc_tail = (struct gc_head *)((byte_t *)self + self->e_offsetof_gctail);
		DeeGC_TrackAll(DeeGC_Object(gc_head), DeeGC_Object(gc_tail));
	}

	return result;
fail:
	return (DeeModuleObject *)ITER_DONE;
}

/* Map the contents of `input_stream' into memory, validate them, and
 * relocate them. This function is actually a convenience wrapper around
 * `DeeDec_OpenFileEx()'
 * @return: * :        Successfully loaded the given DEC file.
 * @return: ITER_DONE: The DEC file was out of date or had been corrupted.
 * @return: NULL:      An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
DeeDec_OpenFile(DeeObject *__restrict input_stream,
                struct Dee_compiler_options *options) {
	char const *dec_dirname;
	size_t dec_dirname_len;
	struct DeeMapFile fmap;
	DREF DeeObject *filename;
	DREF DeeModuleObject *result;

	/* Load stream filename. */
	filename = DeeFile_Filename(input_stream);
	if unlikely(!filename)
		goto err;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);

	/* Load UTF-8 version of filename. */
	dec_dirname = DeeString_AsUtf8(filename);
	if unlikely(!dec_dirname)
		goto err_filename;
	dec_dirname_len = WSTR_LENGTH(dec_dirname);

	/* Extract directory part of stream filename. */
	while (dec_dirname_len && !DeeSystem_IsSep(dec_dirname[dec_dirname_len - 1]))
		--dec_dirname_len;
	while (dec_dirname_len && DeeSystem_IsSep(dec_dirname[dec_dirname_len - 1]))
		--dec_dirname_len;

	/* Map file into memory */
	if unlikely(DeeMapFile_InitFile(&fmap, input_stream, 0, 0, DFILE_LIMIT, 0, DEE_MAPFILE_F_READALL))
		goto err_filename;

	/* Open file mapping as a dec file */
	result = DeeDec_OpenFileEx(&fmap, dec_dirname, dec_dirname_len, options);

	/* On success, the returned module inherits the file mapping */
	if unlikely(!result)
		DeeMapFile_Fini(&fmap);

	/* Cleanup... */
	Dee_Decref(filename);
	return result;
err_filename:
	Dee_Decref(filename);
err:
	return NULL;
}


/* Extended version of `DeeDec_OpenFile()' that can be used to load a
 * dec file that has already been mapped into memory.
 * @return: * :        Successfully loaded the given DEC file.
 * @return: ITER_DONE: The DEC file was out of date or had been corrupted.
 * @return: NULL:      An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
DeeDec_OpenFileEx(/*inherit(on_success)*/ struct DeeMapFile *__restrict fmap,
                  /*utf-8*/ char const *dec_dirname, size_t dec_dirname_len,
                  struct Dee_compiler_options *options) {
	DREF DeeModuleObject *result;
	Dec_Ehdr *ehdr = (Dec_Ehdr *)DeeMapFile_GetBase(fmap);
	if unlikely(DeeMapFile_GetSize(fmap) < sizeof(Dec_Ehdr))
		goto fail;

	/* Validate dec file header... */
	if (ehdr->e_deemon_timestamp != DeeExec_GetTimestamp())
		goto fail;
	/* TODO: verify "ehdr->e_deemon_build_id" */
	/* TODO: verify "ehdr->e_deemon_host_id" */
	if unlikely(ehdr->e_ident[DI_MAG0] != DECMAG0)
		goto fail;
	if unlikely(ehdr->e_ident[DI_MAG1] != DECMAG1)
		goto fail;
	if unlikely(ehdr->e_ident[DI_MAG2] != DECMAG2)
		goto fail;
	if unlikely(ehdr->e_ident[DI_MAG3] != DECMAG3)
		goto fail;
	if unlikely(ehdr->e_mach != Dee_DEC_MACH)
		goto fail;
	if unlikely(ehdr->e_heapoff != offsetof(Dec_Ehdr, e_heap))
		goto fail;
	if unlikely(ehdr->e_version != DVERSION_CUR)
		goto fail;
	if unlikely(ehdr->e_offsetof_eof != DeeMapFile_GetSize(fmap))
		goto fail;
	if unlikely(ehdr->e_offsetof_eof > DFILE_LIMIT)
		goto fail;
	if unlikely(ehdr->e_offsetof_srel >= ehdr->e_offsetof_eof)
		goto fail;
	if unlikely(ehdr->e_offsetof_drel >= ehdr->e_offsetof_eof)
		goto fail;
	if unlikely(ehdr->e_offsetof_drrel >= ehdr->e_offsetof_eof)
		goto fail;
	if unlikely(ehdr->e_offsetof_deps >= ehdr->e_offsetof_eof)
		goto fail;
	if unlikely(ehdr->e_offsetof_files && ehdr->e_offsetof_files >= ehdr->e_offsetof_eof)
		goto fail;
	if unlikely(ehdr->e_offsetof_gchead && ehdr->e_offsetof_gchead >= ehdr->e_offsetof_eof)
		goto fail;
	if unlikely(ehdr->e_offsetof_gctail && ehdr->e_offsetof_gctail >= ehdr->e_offsetof_eof)
		goto fail;
	if unlikely((ehdr->e_offsetof_gchead != 0) != (ehdr->e_offsetof_gctail != 0))
		goto fail;
	if unlikely(ehdr->e_heap.hr_size >= (ehdr->e_offsetof_eof - offsetof(Dec_Ehdr, e_heap)))
		goto fail;

	/* Check if additionally dependent files have been modified since the dec file was created... */
	if (ehdr->e_offsetof_files) {
		Dec_Dstr *dep_files = (Dec_Dstr *)((byte_t *)ehdr + ehdr->e_offsetof_files);
		while (dep_files->ds_length) {
			Dee_DPRINTF("[dec] TODO: Checking timestamp of dependent file %$q in %$q\n",
			            dep_files->ds_length, dep_files->ds_string,
			            dec_dirname_len, dec_dirname);
			/* TODO: Check timestamp of dependent file */
			dep_files = (Dec_Dstr *)(dep_files->ds_string + dep_files->ds_length);
			dep_files = (Dec_Dstr *)CEIL_ALIGN((uintptr_t)dep_files, __ALIGNOF_SIZE_T__);
		}
	}

	/* Configure the runtime portion of the ehdr */
	ehdr->e_mapping         = *fmap;
	ehdr->e_heap.hr_destroy = &DeeDec_heapregion_destroy;

	/* Relocate the dec file to turn it into the embedded module object. */
	return DeeDec_Relocate(ehdr, dec_dirname, dec_dirname_len, options);
fail:
	return (DREF DeeModuleObject *)ITER_DONE;
}

DECL_END

#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
#include <deemon/alloc.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/callable.h>
#include <deemon/cell.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/mapfile.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/numeric.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h> /* memcpy(), bzero(), ... */
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>
#include <deemon/weakref.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/typecore.h>
#include <hybrid/unaligned.h>
/**/

#include <stdarg.h> /* va_list */
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

#ifdef DEE_SYSTEM_FS_ICASE
#ifndef CONFIG_HAVE_memcasecmp
#define CONFIG_HAVE_memcasecmp
#define memcasecmp dee_memcasecmp
DeeSystem_DEFINE_memcasecmp(dee_memcasecmp)
#endif /* !CONFIG_HAVE_memcasecmp */
#define fs_memcmp      memcasecmp
#define fs_bcmp        memcasecmp
#define fs_hashobj(ob) DeeString_HashCase(Dee_REQUIRES_OBJECT(ob))
#else /* DEE_SYSTEM_FS_ICASE */
#define fs_memcmp      memcmp
#define fs_bcmp        bcmp
#define fs_hashobj(ob) DeeString_Hash(Dee_REQUIRES_OBJECT(ob))
#endif /* !DEE_SYSTEM_FS_ICASE */



/* NOTE: Not all error types are present here.
 *       This selection only mirrors what can reasonably be expected to
 *       be arguably commonly used in user-defined exception handlers.
 *       Builtin exceptions that do not appear in this list cannot use
 *       a hard-coded exception mask, but must generate wrapper code:
 *    >> .Lexcept_1_start:
 *    >> ...
 *    >> .Lexcept_1_end:
 *    >> 
 *    >> // Assembly for a catch-guard for `Error.FSError.UnsupportedAPI'
 *    >> .except .Lexcept_1_start, .Lexcept_1_end, .Lexcept_1_entry
 *    >> .Lexcept_1_entry:
 *    >>     push       except
 *    >>     push       const @FSError  // `FSError' can be encoded as a DEC constant (see `DEC_BUILTIN_SET0_FSError')
 *    >>     getattr    top, @"UnsupportedAPI"
 *    >>     instanceof top, pop
 *    >>     jt         1f
 *    >>     throw      except  // Rethrow the eror if the runtime mask didn't match
 *    >> 1:
 *       However if the mask can be encoded as a DEC constant,
 *       no masking code needs to be generated at all:
 *    >> .Lexcept_1_start:
 *    >> ...
 *    >> .Lexcept_1_end:
 *    >> 
 *    >> // Handler for `Error'
 *    >> .except .Lexcept_2_start, .Lexcept_2_end, .Lexcept_2_entry, @mask(Error)
 *    >> .Lexcept_2_entry:
 */


/* Builtin object set #0 (The original one)
 * NOTE: Just because most of these are types, doesn't mean they all have to be! */
#define DEC_BUILTIN_SET0_Signal                0x10 /* DeeError_Signal         */
#define DEC_BUILTIN_SET0_Interrupt             0x11 /* DeeError_Interrupt      */
#define DEC_BUILTIN_SET0_StopIteration         0x12 /* DeeError_StopIteration  */
/*      DEC_BUILTIN_SET0_                      0x13 /* ... */
/*      DEC_BUILTIN_SET0_                      0x14 /* ... */
/*      DEC_BUILTIN_SET0_                      0x15 /* ... */
/*      DEC_BUILTIN_SET0_                      0x16 /* ... */
/*      DEC_BUILTIN_SET0_                      0x17 /* ... */
#define DEC_BUILTIN_SET0_Error                 0x18 /* DeeError_Error          */
#define DEC_BUILTIN_SET0_AttributeError        0x19 /* DeeError_AttributeError */
#define DEC_BUILTIN_SET0_UnboundAttribute      0x1a /* DeeError_UnboundAttribute */
#define DEC_BUILTIN_SET0_CompilerError         0x1b /* DeeError_CompilerError  */
#define DEC_BUILTIN_SET0_ThreadCrash           0x1c /* DeeError_ThreadCrash    */
#define DEC_BUILTIN_SET0_RuntimeError          0x1d /* DeeError_RuntimeError   */
#define DEC_BUILTIN_SET0_NotImplemented        0x1e /* DeeError_NotImplemented */
#define DEC_BUILTIN_SET0_AssertionError        0x1f /* DeeError_AssertionError */
#define DEC_BUILTIN_SET0_UnboundLocal          0x20 /* DeeError_UnboundLocal   */
#define DEC_BUILTIN_SET0_StackOverflow         0x21 /* DeeError_StackOverflow  */
#define DEC_BUILTIN_SET0_TypeError             0x22 /* DeeError_TypeError      */
#define DEC_BUILTIN_SET0_ValueError            0x23 /* DeeError_ValueError     */
#define DEC_BUILTIN_SET0_ArithmeticError       0x24 /* DeeError_ArithmeticError */
#define DEC_BUILTIN_SET0_DivideByZero          0x25 /* DeeError_DivideByZero   */
#define DEC_BUILTIN_SET0_KeyError              0x26 /* DeeError_KeyError       */
#define DEC_BUILTIN_SET0_IndexError            0x27 /* DeeError_IndexError     */
#define DEC_BUILTIN_SET0_UnboundItem           0x28 /* DeeError_UnboundItem    */
#define DEC_BUILTIN_SET0_SequenceError         0x29 /* DeeError_SequenceError  */
#define DEC_BUILTIN_SET0_UnicodeError          0x2a /* DeeError_UnicodeError   */
#define DEC_BUILTIN_SET0_ReferenceError        0x2b /* DeeError_ReferenceError */
#define DEC_BUILTIN_SET0_UnpackError           0x2c /* DeeError_UnpackError    */
#define DEC_BUILTIN_SET0_SystemError           0x2d /* DeeError_SystemError    */
#define DEC_BUILTIN_SET0_FSError               0x2e /* DeeError_FSError        */
#define DEC_BUILTIN_SET0_FileAccessError       0x2f /* DeeError_FileAccessError */
#define DEC_BUILTIN_SET0_FileNotFound          0x30 /* DeeError_FileNotFound   */
#define DEC_BUILTIN_SET0_FileExists            0x31 /* DeeError_FileExists     */
#define DEC_BUILTIN_SET0_FileClosed            0x32 /* DeeError_FileClosed     */
#define DEC_BUILTIN_SET0_NoMemory              0x33 /* DeeError_NoMemory       */
#define DEC_BUILTIN_SET0_IntegerOverflow       0x34 /* DeeError_IntegerOverflow */
#define DEC_BUILTIN_SET0_UnknownKey            0x35 /* DeeError_UnknownKey     */
#define DEC_BUILTIN_SET0_ItemNotFound          0x36 /* DeeError_ItemNotFound   */
#define DEC_BUILTIN_SET0_BufferError           0x37 /* DeeError_BufferError    */
/*      DEC_BUILTIN_SET0_                      0x33 /* ... */
/*      DEC_BUILTIN_SET0_                      0x34 /* ... */
/*      DEC_BUILTIN_SET0_                      0x35 /* ... */
/*      DEC_BUILTIN_SET0_                      0x36 /* ... */
/*      DEC_BUILTIN_SET0_                      0x37 /* ... */
/*      DEC_BUILTIN_SET0_                      0x38 /* ... */
/*      DEC_BUILTIN_SET0_                      0x39 /* ... */
/*      DEC_BUILTIN_SET0_                      0x3a /* ... */
/*      DEC_BUILTIN_SET0_                      0x3b /* ... */
/*      DEC_BUILTIN_SET0_                      0x3c /* ... */
/*      DEC_BUILTIN_SET0_                      0x3d /* ... */
/*      DEC_BUILTIN_SET0_                      0x3e /* ... */
/*      DEC_BUILTIN_SET0_                      0x3f /* ... */

/* Other builtin object types that are arguably useful
 * for base-classes in user-defined classes */

/* Highly useful (unless `@nobase' is used, this one's always the default base) */
#define DEC_BUILTIN_SET0_Object                0x40 /* DeeObject_Type */
/* Abstract base classes. */
#define DEC_BUILTIN_SET0_Sequence              0x41 /* DeeSeq_Type */
#define DEC_BUILTIN_SET0_Mapping               0x42 /* DeeMapping_Type */
#define DEC_BUILTIN_SET0_Iterator              0x43 /* DeeIterator_Type */
#define DEC_BUILTIN_SET0_Callable              0x44 /* DeeCallable_Type */
#define DEC_BUILTIN_SET0_Numeric               0x45 /* DeeNumeric_Type */
#define DEC_BUILTIN_SET0_WeakRefAble           0x46 /* DeeWeakRefAble_Type */
/*      DEC_BUILTIN_SET0_                      0x47 /* ... */
/*      DEC_BUILTIN_SET0_                      0x48 /* ... */
/*      DEC_BUILTIN_SET0_                      0x49 /* ... */
/*      DEC_BUILTIN_SET0_                      0x4a /* ... */
/*      DEC_BUILTIN_SET0_                      0x4b /* ... */
/*      DEC_BUILTIN_SET0_                      0x4c /* ... */
/*      DEC_BUILTIN_SET0_                      0x4d /* ... */
/*      DEC_BUILTIN_SET0_                      0x4e /* ... */
/*      DEC_BUILTIN_SET0_                      0x4f /* ... */

/*      DEC_BUILTIN_SET0_                      0x50 /* ... */
/*      DEC_BUILTIN_SET0_                      ...  /* ... */
/*      DEC_BUILTIN_SET0_                      0x5f /* ... */

/* Builtin sequence types (that aren't final). */
#define DEC_BUILTIN_SET0_List                  0x60 /* DeeList_Type */
#define DEC_BUILTIN_SET0_Dict                  0x61 /* DeeDict_Type */
#define DEC_BUILTIN_SET0_HashSet               0x62 /* DeeHashSet_Type */
#define DEC_BUILTIN_SET0_Cell                  0x63 /* DeeCell_Type */
/*      DEC_BUILTIN_SET0_                      0x64 /* ... */
/*      DEC_BUILTIN_SET0_                      0x65 /* ... */
/*      DEC_BUILTIN_SET0_                      0x66 /* ... */
/*      DEC_BUILTIN_SET0_                      0x67 /* ... */

#define DEC_BUILTIN_SET0_False                 0x68 /* Dee_FalseTrue[0] */
#define DEC_BUILTIN_SET0_True                  0x69 /* Dee_FalseTrue[1] */
#define DEC_BUILTIN_SET0_EmptySeq              0x6a /* DeeSeq_EmptyInstance */
#define DEC_BUILTIN_SET0_EmptySet              0x6b /* DeeSet_EmptyInstance */
#define DEC_BUILTIN_SET0_EmptyMapping          0x6c /* DeeMapping_EmptyInstance */
/*      DEC_BUILTIN_SET0_                      0x6d /* ... */
/*      DEC_BUILTIN_SET0_                      0x6e /* ... */
/*      DEC_BUILTIN_SET0_                      0x6f /* ... */

/*      DEC_BUILTIN_SET0_                      0x70 /* ... */
/*      DEC_BUILTIN_SET0_                      ...  /* ... */
/*      DEC_BUILTIN_SET0_                      0xbf /* ... */

/* Misc. builtin objects to which access as constants could proof useful. */
#define DEC_BUILTIN_SET0_Type                  0xc0 /* DeeType_Type */
#define DEC_BUILTIN_SET0_Traceback             0xc1 /* DeeTraceback_Type */
#define DEC_BUILTIN_SET0_Thread                0xc2 /* DeeThread_Type */
#define DEC_BUILTIN_SET0_Super                 0xc3 /* DeeSuper_Type */
#define DEC_BUILTIN_SET0_String                0xc4 /* DeeString_Type */
#define DEC_BUILTIN_SET0_None                  0xc5 /* DeeNone_Type */
#define DEC_BUILTIN_SET0_Int                   0xc6 /* DeeInt_Type */
#define DEC_BUILTIN_SET0_Float                 0xc7 /* DeeFloat_Type */
#define DEC_BUILTIN_SET0_Module                0xc8 /* DeeModule_Type */
#define DEC_BUILTIN_SET0_Code                  0xc9 /* DeeCode_Type */
#define DEC_BUILTIN_SET0_Tuple                 0xd0 /* DeeTuple_Type */
#define DEC_BUILTIN_SET0_Bool                  0xd1 /* DeeBool_Type */
#define DEC_BUILTIN_SET0_WeakRef               0xd2 /* DeeWeakRef_Type */

/*      DEC_BUILTIN_SET0_                      0xd3 /* ... */
/*      DEC_BUILTIN_SET0_                      ...  /* ... */
/*      DEC_BUILTIN_SET0_                      0xef /* ... */



struct builtin_desc {
	DeeObject *bd_obj; /* [1..1] The object being mapped. */
	uint16_t   bd_id;  /* Set and object ID (Decode using `DEC_BUILTINID_SETOF'). */
	uint8_t    bd_pad[sizeof(void *)-2];
};

/*[[[deemon
import * from deemon;

local sets = Dict();
for (local l: File.open("dec.c")) {
	local setid, name, id, typeval;
	try {
		setid, name, id, typeval = l.scanf(" # define DEC_BUILTIN_SET%[^_]_%[^ ] %[^ ] /" "* %[^ ] *" "/")...;
	} catch (...) {
		continue;
	}
	id = (int)id;
	local setlist = sets.setdefault(setid, []);
	if (#setlist <= id)
		setlist.resize(id+1);
	if (setlist[id] !is none) {
		throw "Set #%s id 0x%x is already used by `%s' when `%s' attempted to set it for `%s'" %
			(setid, id, setlist[id][0], name, typeval);
	}
	setlist[id] = pack(name, typeval);
}


// Count the total number of builtin objects.
local num_builtin_objects = (
	for (local x: sets.values) #(
		for (local y: x)
			if (y !is none)
				y
	)) + ...;
print "#define NUM_BUILTIN_OBJECT_SETS", #sets;
print "#define NUM_BUILTIN_OBJECTS    ", num_builtin_objects;
print "PRIVATE struct builtin_desc builtin_descs[NUM_BUILTIN_OBJECTS] = {";
for (local setname, setlist: sets) {
	for (local i, data: setlist.enumerate()) {
		if (data is none)
			continue;
		local name, typeval = data...;
		print("\t{ (DeeObject *)&", typeval, ", DEC_BUILTINID_MAKE(", setname,
		      ", DEC_BUILTIN_SET", setname, "_", name, ") },");
	}
}
print "};";


for (local setname, setlist: sets) {
	print "PRIVATE DeeObject *buitlin_set"+setname+"[DTYPE_BUILTIN_NUM] = {";
	if (#setlist < 0xf0)
		setlist.resize(0xf0);
	for (local i, data: setlist[0x10:].enumerate()) {
		if (data is none) {
			print "\t/" "* 0x%.2x *" "/ NULL," % (i + 0x10);
		} else {
			local name, typeval = data...;
			print "\t/" "* 0x%.2x *" "/ (DeeObject *)&%s, /" "* %s *" "/" % (i+0x10, typeval, name);
		}
	}
	print "};";
}
]]]*/
#define NUM_BUILTIN_OBJECT_SETS 1
#define NUM_BUILTIN_OBJECTS     64
PRIVATE struct builtin_desc builtin_descs[NUM_BUILTIN_OBJECTS] = {
	{ (DeeObject *)&DeeError_Signal, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Signal) },
	{ (DeeObject *)&DeeError_Interrupt, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Interrupt) },
	{ (DeeObject *)&DeeError_StopIteration, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_StopIteration) },
	{ (DeeObject *)&DeeError_Error, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Error) },
	{ (DeeObject *)&DeeError_AttributeError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_AttributeError) },
	{ (DeeObject *)&DeeError_UnboundAttribute, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_UnboundAttribute) },
	{ (DeeObject *)&DeeError_CompilerError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_CompilerError) },
	{ (DeeObject *)&DeeError_ThreadCrash, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_ThreadCrash) },
	{ (DeeObject *)&DeeError_RuntimeError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_RuntimeError) },
	{ (DeeObject *)&DeeError_NotImplemented, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_NotImplemented) },
	{ (DeeObject *)&DeeError_AssertionError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_AssertionError) },
	{ (DeeObject *)&DeeError_UnboundLocal, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_UnboundLocal) },
	{ (DeeObject *)&DeeError_StackOverflow, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_StackOverflow) },
	{ (DeeObject *)&DeeError_TypeError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_TypeError) },
	{ (DeeObject *)&DeeError_ValueError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_ValueError) },
	{ (DeeObject *)&DeeError_ArithmeticError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_ArithmeticError) },
	{ (DeeObject *)&DeeError_DivideByZero, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_DivideByZero) },
	{ (DeeObject *)&DeeError_KeyError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_KeyError) },
	{ (DeeObject *)&DeeError_IndexError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_IndexError) },
	{ (DeeObject *)&DeeError_UnboundItem, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_UnboundItem) },
	{ (DeeObject *)&DeeError_SequenceError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_SequenceError) },
	{ (DeeObject *)&DeeError_UnicodeError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_UnicodeError) },
	{ (DeeObject *)&DeeError_ReferenceError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_ReferenceError) },
	{ (DeeObject *)&DeeError_UnpackError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_UnpackError) },
	{ (DeeObject *)&DeeError_SystemError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_SystemError) },
	{ (DeeObject *)&DeeError_FSError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_FSError) },
	{ (DeeObject *)&DeeError_FileAccessError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_FileAccessError) },
	{ (DeeObject *)&DeeError_FileNotFound, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_FileNotFound) },
	{ (DeeObject *)&DeeError_FileExists, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_FileExists) },
	{ (DeeObject *)&DeeError_FileClosed, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_FileClosed) },
	{ (DeeObject *)&DeeError_NoMemory, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_NoMemory) },
	{ (DeeObject *)&DeeError_IntegerOverflow, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_IntegerOverflow) },
	{ (DeeObject *)&DeeError_UnknownKey, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_UnknownKey) },
	{ (DeeObject *)&DeeError_ItemNotFound, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_ItemNotFound) },
	{ (DeeObject *)&DeeError_BufferError, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_BufferError) },
	{ (DeeObject *)&DeeObject_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Object) },
	{ (DeeObject *)&DeeSeq_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Sequence) },
	{ (DeeObject *)&DeeMapping_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Mapping) },
	{ (DeeObject *)&DeeIterator_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Iterator) },
	{ (DeeObject *)&DeeCallable_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Callable) },
	{ (DeeObject *)&DeeNumeric_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Numeric) },
	{ (DeeObject *)&DeeWeakRefAble_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_WeakRefAble) },
	{ (DeeObject *)&DeeList_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_List) },
	{ (DeeObject *)&DeeDict_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Dict) },
	{ (DeeObject *)&DeeHashSet_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_HashSet) },
	{ (DeeObject *)&DeeCell_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Cell) },
	{ (DeeObject *)&Dee_FalseTrue[0], DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_False) },
	{ (DeeObject *)&Dee_FalseTrue[1], DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_True) },
	{ (DeeObject *)&DeeSeq_EmptyInstance, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_EmptySeq) },
	{ (DeeObject *)&DeeSet_EmptyInstance, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_EmptySet) },
	{ (DeeObject *)&DeeMapping_EmptyInstance, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_EmptyMapping) },
	{ (DeeObject *)&DeeType_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Type) },
	{ (DeeObject *)&DeeTraceback_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Traceback) },
	{ (DeeObject *)&DeeThread_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Thread) },
	{ (DeeObject *)&DeeSuper_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Super) },
	{ (DeeObject *)&DeeString_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_String) },
	{ (DeeObject *)&DeeNone_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_None) },
	{ (DeeObject *)&DeeInt_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Int) },
	{ (DeeObject *)&DeeFloat_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Float) },
	{ (DeeObject *)&DeeModule_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Module) },
	{ (DeeObject *)&DeeCode_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Code) },
	{ (DeeObject *)&DeeTuple_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Tuple) },
	{ (DeeObject *)&DeeBool_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Bool) },
	{ (DeeObject *)&DeeWeakRef_Type, DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_WeakRef) },
};
PRIVATE DeeObject *buitlin_set0[DTYPE_BUILTIN_NUM] = {
	/* 0x10 */ (DeeObject *)&DeeError_Signal, /* Signal */
	/* 0x11 */ (DeeObject *)&DeeError_Interrupt, /* Interrupt */
	/* 0x12 */ (DeeObject *)&DeeError_StopIteration, /* StopIteration */
	/* 0x13 */ NULL,
	/* 0x14 */ NULL,
	/* 0x15 */ NULL,
	/* 0x16 */ NULL,
	/* 0x17 */ NULL,
	/* 0x18 */ (DeeObject *)&DeeError_Error, /* Error */
	/* 0x19 */ (DeeObject *)&DeeError_AttributeError, /* AttributeError */
	/* 0x1a */ (DeeObject *)&DeeError_UnboundAttribute, /* UnboundAttribute */
	/* 0x1b */ (DeeObject *)&DeeError_CompilerError, /* CompilerError */
	/* 0x1c */ (DeeObject *)&DeeError_ThreadCrash, /* ThreadCrash */
	/* 0x1d */ (DeeObject *)&DeeError_RuntimeError, /* RuntimeError */
	/* 0x1e */ (DeeObject *)&DeeError_NotImplemented, /* NotImplemented */
	/* 0x1f */ (DeeObject *)&DeeError_AssertionError, /* AssertionError */
	/* 0x20 */ (DeeObject *)&DeeError_UnboundLocal, /* UnboundLocal */
	/* 0x21 */ (DeeObject *)&DeeError_StackOverflow, /* StackOverflow */
	/* 0x22 */ (DeeObject *)&DeeError_TypeError, /* TypeError */
	/* 0x23 */ (DeeObject *)&DeeError_ValueError, /* ValueError */
	/* 0x24 */ (DeeObject *)&DeeError_ArithmeticError, /* ArithmeticError */
	/* 0x25 */ (DeeObject *)&DeeError_DivideByZero, /* DivideByZero */
	/* 0x26 */ (DeeObject *)&DeeError_KeyError, /* KeyError */
	/* 0x27 */ (DeeObject *)&DeeError_IndexError, /* IndexError */
	/* 0x28 */ (DeeObject *)&DeeError_UnboundItem, /* UnboundItem */
	/* 0x29 */ (DeeObject *)&DeeError_SequenceError, /* SequenceError */
	/* 0x2a */ (DeeObject *)&DeeError_UnicodeError, /* UnicodeError */
	/* 0x2b */ (DeeObject *)&DeeError_ReferenceError, /* ReferenceError */
	/* 0x2c */ (DeeObject *)&DeeError_UnpackError, /* UnpackError */
	/* 0x2d */ (DeeObject *)&DeeError_SystemError, /* SystemError */
	/* 0x2e */ (DeeObject *)&DeeError_FSError, /* FSError */
	/* 0x2f */ (DeeObject *)&DeeError_FileAccessError, /* FileAccessError */
	/* 0x30 */ (DeeObject *)&DeeError_FileNotFound, /* FileNotFound */
	/* 0x31 */ (DeeObject *)&DeeError_FileExists, /* FileExists */
	/* 0x32 */ (DeeObject *)&DeeError_FileClosed, /* FileClosed */
	/* 0x33 */ (DeeObject *)&DeeError_NoMemory, /* NoMemory */
	/* 0x34 */ (DeeObject *)&DeeError_IntegerOverflow, /* IntegerOverflow */
	/* 0x35 */ (DeeObject *)&DeeError_UnknownKey, /* UnknownKey */
	/* 0x36 */ (DeeObject *)&DeeError_ItemNotFound, /* ItemNotFound */
	/* 0x37 */ (DeeObject *)&DeeError_BufferError, /* BufferError */
	/* 0x38 */ NULL,
	/* 0x39 */ NULL,
	/* 0x3a */ NULL,
	/* 0x3b */ NULL,
	/* 0x3c */ NULL,
	/* 0x3d */ NULL,
	/* 0x3e */ NULL,
	/* 0x3f */ NULL,
	/* 0x40 */ (DeeObject *)&DeeObject_Type, /* Object */
	/* 0x41 */ (DeeObject *)&DeeSeq_Type, /* Sequence */
	/* 0x42 */ (DeeObject *)&DeeMapping_Type, /* Mapping */
	/* 0x43 */ (DeeObject *)&DeeIterator_Type, /* Iterator */
	/* 0x44 */ (DeeObject *)&DeeCallable_Type, /* Callable */
	/* 0x45 */ (DeeObject *)&DeeNumeric_Type, /* Numeric */
	/* 0x46 */ (DeeObject *)&DeeWeakRefAble_Type, /* WeakRefAble */
	/* 0x47 */ NULL,
	/* 0x48 */ NULL,
	/* 0x49 */ NULL,
	/* 0x4a */ NULL,
	/* 0x4b */ NULL,
	/* 0x4c */ NULL,
	/* 0x4d */ NULL,
	/* 0x4e */ NULL,
	/* 0x4f */ NULL,
	/* 0x50 */ NULL,
	/* 0x51 */ NULL,
	/* 0x52 */ NULL,
	/* 0x53 */ NULL,
	/* 0x54 */ NULL,
	/* 0x55 */ NULL,
	/* 0x56 */ NULL,
	/* 0x57 */ NULL,
	/* 0x58 */ NULL,
	/* 0x59 */ NULL,
	/* 0x5a */ NULL,
	/* 0x5b */ NULL,
	/* 0x5c */ NULL,
	/* 0x5d */ NULL,
	/* 0x5e */ NULL,
	/* 0x5f */ NULL,
	/* 0x60 */ (DeeObject *)&DeeList_Type, /* List */
	/* 0x61 */ (DeeObject *)&DeeDict_Type, /* Dict */
	/* 0x62 */ (DeeObject *)&DeeHashSet_Type, /* HashSet */
	/* 0x63 */ (DeeObject *)&DeeCell_Type, /* Cell */
	/* 0x64 */ NULL,
	/* 0x65 */ NULL,
	/* 0x66 */ NULL,
	/* 0x67 */ NULL,
	/* 0x68 */ (DeeObject *)&Dee_FalseTrue[0], /* False */
	/* 0x69 */ (DeeObject *)&Dee_FalseTrue[1], /* True */
	/* 0x6a */ (DeeObject *)&DeeSeq_EmptyInstance, /* EmptySeq */
	/* 0x6b */ (DeeObject *)&DeeSet_EmptyInstance, /* EmptySet */
	/* 0x6c */ (DeeObject *)&DeeMapping_EmptyInstance, /* EmptyMapping */
	/* 0x6d */ NULL,
	/* 0x6e */ NULL,
	/* 0x6f */ NULL,
	/* 0x70 */ NULL,
	/* 0x71 */ NULL,
	/* 0x72 */ NULL,
	/* 0x73 */ NULL,
	/* 0x74 */ NULL,
	/* 0x75 */ NULL,
	/* 0x76 */ NULL,
	/* 0x77 */ NULL,
	/* 0x78 */ NULL,
	/* 0x79 */ NULL,
	/* 0x7a */ NULL,
	/* 0x7b */ NULL,
	/* 0x7c */ NULL,
	/* 0x7d */ NULL,
	/* 0x7e */ NULL,
	/* 0x7f */ NULL,
	/* 0x80 */ NULL,
	/* 0x81 */ NULL,
	/* 0x82 */ NULL,
	/* 0x83 */ NULL,
	/* 0x84 */ NULL,
	/* 0x85 */ NULL,
	/* 0x86 */ NULL,
	/* 0x87 */ NULL,
	/* 0x88 */ NULL,
	/* 0x89 */ NULL,
	/* 0x8a */ NULL,
	/* 0x8b */ NULL,
	/* 0x8c */ NULL,
	/* 0x8d */ NULL,
	/* 0x8e */ NULL,
	/* 0x8f */ NULL,
	/* 0x90 */ NULL,
	/* 0x91 */ NULL,
	/* 0x92 */ NULL,
	/* 0x93 */ NULL,
	/* 0x94 */ NULL,
	/* 0x95 */ NULL,
	/* 0x96 */ NULL,
	/* 0x97 */ NULL,
	/* 0x98 */ NULL,
	/* 0x99 */ NULL,
	/* 0x9a */ NULL,
	/* 0x9b */ NULL,
	/* 0x9c */ NULL,
	/* 0x9d */ NULL,
	/* 0x9e */ NULL,
	/* 0x9f */ NULL,
	/* 0xa0 */ NULL,
	/* 0xa1 */ NULL,
	/* 0xa2 */ NULL,
	/* 0xa3 */ NULL,
	/* 0xa4 */ NULL,
	/* 0xa5 */ NULL,
	/* 0xa6 */ NULL,
	/* 0xa7 */ NULL,
	/* 0xa8 */ NULL,
	/* 0xa9 */ NULL,
	/* 0xaa */ NULL,
	/* 0xab */ NULL,
	/* 0xac */ NULL,
	/* 0xad */ NULL,
	/* 0xae */ NULL,
	/* 0xaf */ NULL,
	/* 0xb0 */ NULL,
	/* 0xb1 */ NULL,
	/* 0xb2 */ NULL,
	/* 0xb3 */ NULL,
	/* 0xb4 */ NULL,
	/* 0xb5 */ NULL,
	/* 0xb6 */ NULL,
	/* 0xb7 */ NULL,
	/* 0xb8 */ NULL,
	/* 0xb9 */ NULL,
	/* 0xba */ NULL,
	/* 0xbb */ NULL,
	/* 0xbc */ NULL,
	/* 0xbd */ NULL,
	/* 0xbe */ NULL,
	/* 0xbf */ NULL,
	/* 0xc0 */ (DeeObject *)&DeeType_Type, /* Type */
	/* 0xc1 */ (DeeObject *)&DeeTraceback_Type, /* Traceback */
	/* 0xc2 */ (DeeObject *)&DeeThread_Type, /* Thread */
	/* 0xc3 */ (DeeObject *)&DeeSuper_Type, /* Super */
	/* 0xc4 */ (DeeObject *)&DeeString_Type, /* String */
	/* 0xc5 */ (DeeObject *)&DeeNone_Type, /* None */
	/* 0xc6 */ (DeeObject *)&DeeInt_Type, /* Int */
	/* 0xc7 */ (DeeObject *)&DeeFloat_Type, /* Float */
	/* 0xc8 */ (DeeObject *)&DeeModule_Type, /* Module */
	/* 0xc9 */ (DeeObject *)&DeeCode_Type, /* Code */
	/* 0xca */ NULL,
	/* 0xcb */ NULL,
	/* 0xcc */ NULL,
	/* 0xcd */ NULL,
	/* 0xce */ NULL,
	/* 0xcf */ NULL,
	/* 0xd0 */ (DeeObject *)&DeeTuple_Type, /* Tuple */
	/* 0xd1 */ (DeeObject *)&DeeBool_Type, /* Bool */
	/* 0xd2 */ (DeeObject *)&DeeWeakRef_Type, /* WeakRef */
	/* 0xd3 */ NULL,
	/* 0xd4 */ NULL,
	/* 0xd5 */ NULL,
	/* 0xd6 */ NULL,
	/* 0xd7 */ NULL,
	/* 0xd8 */ NULL,
	/* 0xd9 */ NULL,
	/* 0xda */ NULL,
	/* 0xdb */ NULL,
	/* 0xdc */ NULL,
	/* 0xdd */ NULL,
	/* 0xde */ NULL,
	/* 0xdf */ NULL,
	/* 0xe0 */ NULL,
	/* 0xe1 */ NULL,
	/* 0xe2 */ NULL,
	/* 0xe3 */ NULL,
	/* 0xe4 */ NULL,
	/* 0xe5 */ NULL,
	/* 0xe6 */ NULL,
	/* 0xe7 */ NULL,
	/* 0xe8 */ NULL,
	/* 0xe9 */ NULL,
	/* 0xea */ NULL,
	/* 0xeb */ NULL,
	/* 0xec */ NULL,
	/* 0xed */ NULL,
	/* 0xee */ NULL,
	/* 0xef */ NULL,
};
/*[[[end]]]*/


/* Return the ID of a given object.
 * If the given object isn't a builtin object, `DEC_BUILTINID_UNKNOWN' is returned.
 * The set and ID of the returned identifier can be extracted through
 * use of the `DEC_BUILTINID_SETOF' and `DEC_BUILTINID_IDOF' macros below.
 * To qualify as a builtin object, the object must be defined as an object
 * exported publicly by the deemon core, such that this includes the builtin
 * error types, as well as pretty much all other types found in deemon headers.
 * However there may be exceptions to this rule as there would be no point to
 * have an object like `DeeObjMethod_Type' or `DeeCMethod_Type' be made available
 * as a builtin object, considering access to those should happen through use of
 * the builtin `deemon' module.
 * The main idea behind builtin ids is to be able to encode the builtin error
 * types, because without a way of encoding them, we couldn't be using them
 * as compile-time exception masks in user-defined exception handlers (because
 * if we still did this, the resulting code object could not be saved to a DEC
 * compiled source files)
 * Note however that the compiler is very much aware of what can be used as a
 * builtin object, and will automatically prevent constant propagation if there
 * is no way of encoding the resulting object as a DEC constant expression. */
INTERN WUNUSED NONNULL((1)) uint16_t DCALL
Dec_BuiltinID(DeeObject *__restrict obj) {
	struct builtin_desc *iter;
	for (iter = builtin_descs; iter < COMPILER_ENDOF(builtin_descs); ++iter) {
		if (iter->bd_obj == obj)
			return iter->bd_id;
	}
	return DEC_BUILTINID_UNKNOWN;
}

/* Return the builtin object associated with `id'
 * within `set', return `NULL' when the given `set'
 * is unknown, or `id' is unassigned.
 * NOTE: The caller is responsible for passing an `id' that is located
 *       within the inclusive bounds `DTYPE_BUILTIN_MIN...DTYPE_BUILTIN_MAX'.
 *       This function internally asserts this and crashes if that is not the case. */
INTERN WUNUSED DeeObject *DCALL
Dec_GetBuiltin(uint8_t set, uint8_t id) {
	ASSERT(id >= DTYPE_BUILTIN_MIN);
#if DTYPE_BUILTIN_MAX != 0xff
	ASSERT(id <= DTYPE_BUILTIN_MAX);
#endif /* DTYPE_BUILTIN_MAX != 0xff */
	/* Default case: object set #0 */
	if (set == 0)
		return (buitlin_set0 - DTYPE_BUILTIN_MIN)[id];
	return NULL;
}


/************************************************************************/
/* INTERNAL DEC API                                                     */
/************************************************************************/

#define DECFILE_PADDING 32

struct module_object;
struct compiler_options;
struct string_object;
struct code_object;
struct ddi_object;

typedef struct {
	union {
		uint8_t const         *df_data;    /* [0..df_size+DECFILE_PADDING]
		                                    * A full mapping of all data from the input DEC file, followed
		                                    * by a couple of bytes of padding data that is ZERO-initialized. */
		uint8_t const         *df_base;    /* [0..df_size] Base address of the DEC image mapped into host memory. */
		Dec_Ehdr const        *df_ehdr;    /* [0..1] A pointer to the DEC file header mapped into host memory. */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define df_data _dee_aunion.df_data
#define df_base _dee_aunion.df_base
#define df_ehdr _dee_aunion.df_ehdr
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	size_t                     df_size;    /* Total number of usable bytes of memory
	                                        * that can be found within the source file. */
	DREF struct string_object *df_name;    /* [1..1] The filename of the `*.dee' file opened by this descriptor. */
	DREF struct module_object *df_module;  /* [1..1] The module that is being loaded. */
	struct compiler_options   *df_options; /* [0..1] Compilation options. */
	DREF struct string_object *df_strtab;  /* [0..1] Lazily allocated copy of the string table.
	                                        *        This string is used by DDI descriptors in
	                                        *        order to allow for sharing of string tables. */
} DecFile;


/* Initialize a DEC file, given an input stream, as well as its pathname.
 * @return:  1: The given `input_stream' doesn't describe a valid DEC file.
 * @return:  0: Successfully initialized the DEC file.
 * @return: -1: An error occurred while attempting to read the DEC's data,
 *              or failed to allocate a sufficient buffer for the DEC. */
PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DecFile_Init(DecFile *__restrict self,
             struct DeeMapFile *__restrict input,
             struct module_object *__restrict module,
             struct string_object *__restrict dec_pathname,
             struct compiler_options *options);
PRIVATE NONNULL((1)) void DCALL
DecFile_Fini(DecFile *__restrict self);

/* Return a string for the entire strtab of a given DEC-file.
 * Upon error, NULL is returned.
 * NOTE: The return value is _NOT_ a reference! */
PRIVATE WUNUSED NONNULL((1)) DeeObject *DCALL
DecFile_Strtab(DecFile *__restrict self);

/* Check if a given DEC file is up to date, or if it must not be loaded.
 * because it a dependency has changed since it was created.
 * @return:  0: The file is up-to-date.
 * @return:  1: The file is not up-to-date.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DecFile_IsUpToDate(DecFile *__restrict self);

/* Load a given DEC file and fill in the given `module'.
 * @return:  0: Successfully loaded the given DEC file.
 * @return:  1: The DEC file has been corrupted or is out of date.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DecFile_Load(DecFile *__restrict self);

/* Return the last-modified time (in microseconds since 01-01-1970).
 * For this purpose, an internal cache is kept that is consulted
 * before and populated after making an attempt at contacting the
 * host operating system for the required information.
 * @return: * :           Last-modified time (in microseconds since 01-01-1970).
 * @return: 0 :           The given file could not be found.
 * @return: (uint64_t)-1: The lookup failed and an error was thrown. */
PRIVATE WUNUSED NONNULL((1)) uint64_t DCALL
DecTime_Lookup(DeeObject *__restrict filename);

/* DEC loader implementation. */

/* @return: * :        A reference to the object that got loaded.
 * @return: NULL:      An error occurred. (NOTE: `DTYPE_NULL' is not allowed and indicates a corrupt file)
 * @return: ITER_DONE: The DEC file has been corrupted. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DecFile_LoadObject(DecFile *__restrict self,
                   uint8_t const **__restrict p_reader);

/* @param: allow_dtype_null: When true, individual vector elements are allowed
 *                           to be `NULL' as the result of `DTYPE_NULL'
 * @return: * :              Newly heap-allocated vector of objects (length is stored in `*p_count').
 * @return: NULL:            An error occurred.
 * @return: ITER_DONE:       The DEC file has been corrupted. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject **DCALL
DecFile_LoadObjectVector(DecFile *__restrict self,
                         uint16_t *__restrict p_count,
                         uint8_t const **__restrict p_reader,
                         bool allow_dtype_null);

/* @return: * :        New reference to a code object.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The DEC file has been corrupted. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF struct code_object *DCALL
DecFile_LoadCode(DecFile *__restrict self,
                 uint8_t const **__restrict p_reader);

/* @return: * :        New reference to a ddi object.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The DEC file has been corrupted. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF struct ddi_object *DCALL
DecFile_LoadDDI(DecFile *__restrict self,
                uint8_t const *__restrict reader,
                bool is_8bit_ddi);

/************************************************************************/




/************************************************************************/
/* DEC FILE CORRUPTION TRACING                                          */
/************************************************************************/
#if !defined(NDEBUG) && 1
PRIVATE void DCALL
corrupt_here(DecFile *__restrict self,
             void const *reader,
             char const *file, int line) {
	Dee_DPRINTF("%s(%d) : %k", file, line, self->df_name);
	if (reader)
		Dee_DPRINTF("@%#Ix", (size_t)((uint8_t *)reader - self->df_base));
	Dee_DPRINT(" : ERROR : Module corruption detected\n");
}

PRIVATE void
corrupt_heref(DecFile *__restrict self,
              void const *reader,
              char const *file, int line,
              char const *format, ...) {
	va_list args;
	Dee_DPRINTF("%s(%d) : %k", file, line, self->df_name);
	if (reader)
		Dee_DPRINTF("@%#Ix", (size_t)((uint8_t *)reader - self->df_base));
	Dee_DPRINT(" : ERROR : Module corruption detected : ");
	va_start(args, format);
	Dee_VDPRINTF(format, args);
	va_end(args);
	Dee_DPRINT("\n");
}
#define HAVE_GOTO_CORRUPTED 1
#define GOTO_CORRUPTED(reader, sym)                     \
	do {                                                \
		corrupt_here(self, reader, __FILE__, __LINE__); \
		goto sym;                                       \
	}	__WHILE0
#define GOTO_CORRUPTEDF(reader, sym, ...)                             \
	do {                                                              \
		corrupt_heref(self, reader, __FILE__, __LINE__, __VA_ARGS__); \
		goto sym;                                                     \
	}	__WHILE0
#define SET_CORRUPTED(reader, expr) \
	(corrupt_here(self, reader, __FILE__, __LINE__), expr)
#define SET_CORRUPTEDF(reader, expr, ...) \
	(corrupt_heref(self, reader, __FILE__, __LINE__, __VA_ARGS__), expr)
#else /* !NDEBUG */
#define GOTO_CORRUPTED(reader, sym)       goto sym
#define GOTO_CORRUPTEDF(reader, sym, ...) goto sym
#define SET_CORRUPTED(reader, expr)       expr
#define SET_CORRUPTEDF(reader, expr, ...) expr
#endif /* NDEBUG */







/* Initialize a DEC file, given an input stream, as well as its pathname.
 * NOTE: The given `input' must have been loaded with `DECFILE_PADDING'
 *       trailing NUL-bytes (or possibly even more)
 * @return:  1: The given `input' doesn't describe a valid DEC file.
 * @return:  0: Successfully initialized the DEC file.
 * @return: -1: An error occurred while attempting to read the DEC's data,
 *              or failed to allocate a sufficient buffer for the DEC. */
PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DecFile_Init(DecFile *__restrict self,
             struct DeeMapFile *__restrict input,
             DeeModuleObject *__restrict module,
             DeeStringObject *__restrict dec_pathname,
             struct compiler_options *options) {
	Dec_Ehdr *hdr;
	ASSERT_OBJECT_TYPE(module, &DeeModule_Type);
	ASSERT_OBJECT_TYPE_EXACT(dec_pathname, &DeeString_Type);

	/* Quick check: If the file is larger than the allowed limit,
	 *              don't even consider attempting to load it. */
	if unlikely(DeeMapFile_GetSize(input) > DFILE_LIMIT)
		goto end_not_a_dec;

	/* Another quick check: If the file isn't even large enough for the
	 *                      basic header, it's not a DEC file either. */
	if unlikely(DeeMapFile_GetSize(input) < sizeof(Dec_Ehdr))
		goto end_not_a_dec;

	/* Load file map size. */
	hdr = (Dec_Ehdr *)DeeMapFile_GetBase(input);
	self->df_ehdr = hdr;
	self->df_size = DeeMapFile_GetSize(input);

	/* All right! we've read the file.
	 * Now to do a quick validation of the header. */
	if unlikely(hdr->e_ident[DI_MAG0] != DECMAG0)
		goto end_not_a_dec;
	if unlikely(hdr->e_ident[DI_MAG1] != DECMAG1)
		goto end_not_a_dec;
	if unlikely(hdr->e_ident[DI_MAG2] != DECMAG2)
		goto end_not_a_dec;
	if unlikely(hdr->e_ident[DI_MAG3] != DECMAG3)
		goto end_not_a_dec;
	if unlikely(hdr->e_version != HTOLE16_C(DVERSION_CUR))
		goto end_not_a_dec;
	if unlikely(hdr->e_size < sizeof(Dec_Ehdr))
		goto end_not_a_dec;
	if unlikely(hdr->e_builtinset > DBUILTINS_MAX)
		goto end_not_a_dec;
	/* Validate pointers from the header. */
	if unlikely(LETOH32(hdr->e_impoff) >= (size_t)self->df_size)
		goto end_not_a_dec;
	if unlikely(LETOH32(hdr->e_depoff) >= (size_t)self->df_size)
		goto end_not_a_dec;
	if unlikely(LETOH32(hdr->e_globoff) >= (size_t)self->df_size)
		goto end_not_a_dec;
	if unlikely(LETOH32(hdr->e_rootoff) >= (size_t)self->df_size)
		goto end_not_a_dec;
	if unlikely(LETOH32(hdr->e_stroff) < hdr->e_size)
		goto end_not_a_dec; /* Missing string table. */
	if unlikely(LETOH32(hdr->e_rootoff) < hdr->e_size)
		goto end_not_a_dec; /* Validate the root-code pointer. */
	if unlikely(LETOH32(hdr->e_stroff) +
	            LETOH32(hdr->e_strsiz) <
	            LETOH32(hdr->e_stroff))
		goto end_not_a_dec; /* Check for overflow */
	if unlikely(LETOH32(hdr->e_stroff) +
	            LETOH32(hdr->e_strsiz) >
	            (size_t)self->df_size)
		goto end_not_a_dec;

	/* Save the given options in the DEC file descriptor. */
	self->df_options = options;

	/* ZERO-initialize everything we've not initializing explicitly. */
	bzero(&self->df_strtab,
	      sizeof(DecFile) -
	      offsetof(DecFile, df_strtab));

	/* Save the module and filename of the DEC input file. */
	self->df_module = module;
	self->df_name   = dec_pathname;
	Dee_Incref(module);
	Dee_Incref(dec_pathname);
	return 0;
end_not_a_dec:
	return 1;
}

PRIVATE NONNULL((1)) void DCALL
DecFile_Fini(DecFile *__restrict self) {
	Dee_XDecref(self->df_strtab);
	Dee_Decref(self->df_module);
	Dee_Decref(self->df_name);
}

/* Return a string for the entire strtab of a given DEC-file.
 * Upon error, NULL is returned.
 * NOTE: The return value is _NOT_ a reference! */
PRIVATE WUNUSED NONNULL((1)) DeeObject *DCALL
DecFile_Strtab(DecFile *__restrict self) {
	DeeStringObject *result;
	if ((result = self->df_strtab) == NULL) {
		result = (DeeStringObject *)DeeString_NewSized((char const *)(self->df_base +
		                                                              LETOH32(self->df_ehdr->e_stroff)),
		                                               LETOH32(self->df_ehdr->e_strsiz));
		self->df_strtab = result; /* Inherit reference. */
	}
	return (DeeObject *)result;
}




/* Check if a given DEC file is up to date, or if it must not be loaded.
 * because it a dependency has changed since it was created.
 * @return:  0: The file is up-to-date.
 * @return:  1: The file is not up-to-date.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DecFile_IsUpToDate(DecFile *__restrict self) {
	Dec_Ehdr const *hdr = self->df_ehdr;
	uint64_t timestamp, other;
	other = DecTime_Lookup((DeeObject *)self->df_name);
	if unlikely(other == (uint64_t)-1)
		goto err;
	timestamp = (((uint64_t)LETOH32(hdr->e_timestamp_hi) << 32) |
	             ((uint64_t)LETOH32(hdr->e_timestamp_lo)));
	if (other > timestamp)
		goto changed; /* Base source file has changed. */
#if 0 /* TODO */
	/* Check additional dependencies. */
	if (hdr->e_depoff != 0) {
		Dec_Strmap *depmap;
		char *strtab, *filend;
		uint16_t count;
		uint8_t *reader;
		depmap = (Dec_Strmap *)(self->df_base + LETOH32(hdr->e_depoff));
		if unlikely((count = UNALIGNED_GETLE16(&depmap->i_len)) == 0)
			goto done; /* Unlikely, but allowed. */
		reader = depmap->i_map;
		while (module_pathlen &&
		       !DeeSystem_IsSep(module_pathstr[module_pathlen - 1])) {
			--module_pathlen;
		}
		strtab = (char *)(self->df_base + LETOH32(hdr->e_stroff));
		filend = (char *)(self->df_base + self->df_size);
		while (count--) {
			size_t name_len;
			char *p, *name = strtab + Dec_DecodePointer(&reader);
			if unlikely(name >= filend)
				goto changed; /* Corrupted */
			name_len = strlen(name);
			/* Create a string buffer for the filename to-be checked.
			 * NOTE: The `module_pathstr+=module_pathlen' already has a trailing slash! */
			filename = DeeString_NewBuffer(module_pathlen + name_len);
			if unlikely(!filename)
				goto err;
			p = (char *)mempcpyc(DeeString_STR(filename), module_pathstr,
			                     module_pathlen, sizeof(char));
			memcpyc(p, filename, name_len, sizeof(char));
			other = DecTime_Lookup(filename);
			Dee_Decref(filename);
			if unlikely(other == (uint64_t)-1)
				goto err;
			/* Check if this dependency has changed since. */
			if (other > timestamp)
				goto changed;
		}
	}
done:
#endif
	return 0;
changed:
	return 1;
err:
	return -1;
}




PRIVATE WUNUSED NONNULL((1)) int DCALL
DecFile_LoadImports(DecFile *__restrict self) {
	int result = 1;
	Dec_Strmap const *impmap;
	Dec_Ehdr const *hdr = self->df_ehdr;
	uint64_t timestamp;
	uint8_t const *end = self->df_base + self->df_size;
	DREF DeeModuleObject *module;
	DREF DeeModuleObject **importv;
	uint8_t const *reader;
	DREF DeeModuleObject **moditer, **modend;
	uint16_t importc;
	char const *strtab;
	char const *module_pathstr;
	size_t module_pathlen;
	/* Quick check: Without an import table, nothing needs to be loaded. */
	if (!hdr->e_impoff)
		return 0;
	timestamp = (((uint64_t)LETOH32(hdr->e_timestamp_hi) << 32) |
	             ((uint64_t)LETOH32(hdr->e_timestamp_lo)));

	/* Load the import table. */
	strtab         = (char const *)(self->df_base + LETOH32(hdr->e_stroff));
	module_pathstr = DeeString_AsUtf8((DeeObject *)self->df_name);
	if unlikely(!module_pathstr)
		goto err;
	module_pathlen = WSTR_LENGTH(module_pathstr);
	while (module_pathlen &&
	       !DeeSystem_IsSep(module_pathstr[module_pathlen - 1]))
		--module_pathlen;
	impmap  = (Dec_Strmap const *)(self->df_base + LETOH32(hdr->e_impoff));
	importc = UNALIGNED_GETLE16(&impmap->i_len);
	importv = (DREF DeeModuleObject **)Dee_Mallocc(importc, sizeof(DREF DeeModuleObject *));
	if unlikely(!importv)
		goto err;
	moditer = importv;
	modend  = importv + importc;
	reader  = impmap->i_map;
	for (; moditer < modend; ++moditer) {
		uint32_t off;
		char const *module_name;
		if unlikely(reader >= end)
			GOTO_CORRUPTED(reader, stop_imports);
		off = Dec_DecodePointer(&reader);
		if unlikely(off >= LETOH32(hdr->e_strsiz))
			GOTO_CORRUPTED(reader, stop_imports);
		module_name = strtab + off;
		/* Load the imported module. */
		module = (DREF DeeModuleObject *)DeeModule_OpenRelativeString(module_name, strlen(module_name),
		                                                              module_pathstr, module_pathlen,
		                                                              self->df_options ? self->df_options->co_inner : NULL,
		                                                              false);
		if unlikely(!ITER_ISOK(module)) {
			if (!module)
				goto err_imports;
			/* Don't throw an error for this when `module_name' describes
			 * the name of a global module. - This could happen if the
			 * module library path was set up differently when this DEC file
			 * was generated last, now resulting in that module no longer being
			 * reachable as one that is global.
			 * Technically, this is a problem with how the DEC encoder generates
			 * dependency names, as it no longer knows if the original source
			 * addressed its dependencies as relative, or global modules, forcing
			 * it to guess which one it should choose.
			 * However, since DEC files are intended as simple caches, rather than
			 * stand-alone object files, or even executables all-together, we can
			 * simply indicate that the DEC cache has been corrupted, or has fallen
			 * out-of-date, and have the caller re-compile the module, thus updating
			 * the dependency, and loading the required module via its actual location. */
			if (*module_name != '.')
				GOTO_CORRUPTEDF(reader, stop_imports, "module_name = %q", module_name);

			/* A missing relative import is something that even re-compiling won't
			 * fix, so we may as well throw the error now, and include some helpful
			 * context information about where the dependency came from.
			 * -> This can happen if global modules that were previously used
			 *    suddenly disappear between execution cycles. */
			DeeError_Throwf(&DeeError_FileNotFound,
			                "Dependency %q of module %r in %$q could not be found",
			                strtab + off, self->df_module->mo_name,
			                module_pathlen, module_pathstr);
			goto err_imports;
		}

		/* Check if the module has changed. */
		if (!self->df_options || !(self->df_options->co_decloader & Dee_DEC_FLOADOUTDATED)) {
			uint64_t modtime = DeeModule_GetCTime((DeeObject *)module);
			if unlikely(modtime == (uint64_t)-1)
				GOTO_CORRUPTED(reader, err_imports_module);
			/* If the module has changed since the time
			 * described on the DEC header, stop loading. */
			if unlikely(modtime > timestamp) {
				GOTO_CORRUPTEDF(reader, stop_imports_module,
				                "Dependency %q of module %r in %$q changed "
				                "after .dec file was created (%" PRFu64 " > %" PRFu64 ")",
				                strtab + off, self->df_module->mo_name,
				                module_pathlen, module_pathstr,
				                modtime, timestamp);
			}
		}
		*moditer = module; /* Inherit */
	}
	/* Write the module import table. */
	self->df_module->mo_importc = importc;
	self->df_module->mo_importv = importv; /* Inherit. */
	result                      = 0;
stop:
	return result;
stop_imports:
	if (importv) {
		Dee_Decrefv(importv, (size_t)(moditer - importv));
		Dee_Free(importv);
	}
	goto stop;
stop_imports_module:
	Dee_Decref(module);
	goto stop_imports;
err_imports_module:
	Dee_Decref(module);
err_imports:
	result = -1;
	goto stop_imports;
err:
	result = -1;
	goto stop;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DecFile_LoadGlobals(DecFile *__restrict self) {
	int result = 1;
	Dec_Glbmap const *glbmap;
	DeeModuleObject *module = self->df_module;
	Dec_Ehdr const *hdr = self->df_ehdr;
	uint16_t i, globalc, symbolc;
	uint8_t const *end = self->df_base + self->df_size;
	uint8_t const *reader;
	uint16_t bucket_mask;
	struct module_symbol *bucketv;
	char const *strtab;
	/* Quick check: Without a global variable table, nothing needs to be loaded. */
	if (!hdr->e_globoff)
		return 0;

	/* Load the global object table. */
	glbmap  = (Dec_Glbmap const *)(self->df_base + LETOH32(hdr->e_globoff));
	globalc = UNALIGNED_GETLE16(&glbmap->g_cnt);
	symbolc = UNALIGNED_GETLE16(&glbmap->g_len);
	if unlikely(globalc > symbolc)
		GOTO_CORRUPTED(&glbmap->g_cnt, stop);
	if unlikely(!symbolc)
		return 0; /* Unlikely, but allowed. */
	strtab = (char const *)(self->df_base + LETOH32(hdr->e_stroff));
	reader = (uint8_t const *)glbmap + 4;

	/* Figure out how large the hash-mask should be. */
	bucket_mask = 1;
	while (bucket_mask < symbolc)
		bucket_mask <<= 1;
	if ((bucket_mask - symbolc) < 16)
		bucket_mask <<= 1;
	--bucket_mask;

	/* Allocate the module bucket vector. */
	bucketv = (struct module_symbol *)Dee_Callocc(bucket_mask + 1,
	                                              sizeof(struct module_symbol));
	if unlikely(!bucketv)
		goto err;

	/* Read symbol information. */
	for (i = 0; i < symbolc; ++i) {
		uint16_t flags, addr, addr2;
		char const *name, *doc;
		uint32_t doclen;
		Dee_hash_t name_hash, hash_i, perturb;
		if unlikely(reader >= end)
			GOTO_CORRUPTED(reader, stop_symbolv); /* Validate bounds. */
		flags = UNALIGNED_GETLE16(reader), reader += 2;
		if (flags & ~MODSYM_FMASK)
			GOTO_CORRUPTED(reader, stop_symbolv); /* Unknown flags are being used. */
		/* The first `globalc' descriptors lack the `s_addr' field. */
		addr2 = (uint16_t)-1;
		if (i < globalc) {
			addr = i;
		} else {
			addr = UNALIGNED_GETLE16(reader), reader += 2;
			if (flags & MODSYM_FEXTERN) {
				addr2 = UNALIGNED_GETLE16(reader), reader += 2;
				if (!(flags & MODSYM_FPROPERTY) && addr2 >= self->df_module->mo_importc)
					GOTO_CORRUPTED(reader, stop_symbolv); /* Validate module index. */
			} else {
				if unlikely(addr >= globalc)
					GOTO_CORRUPTED(reader, stop_symbolv); /* Validate symbol address. */
			}
		}
		name = strtab + Dec_DecodePointer(&reader);
		if unlikely(name >= (char *)end)
			GOTO_CORRUPTED(reader, stop_symbolv);
		/* If the name points to an empty string, skip this entry. */
		if (!*name)
			continue;
		doclen = Dec_DecodePointer(&reader);
		doc    = strtab;
		if (doclen)
			doc += Dec_DecodePointer(&reader);
		if unlikely(doc + doclen >= (char const *)end)
			GOTO_CORRUPTED(reader, stop_symbolv);

		/* Figure out the proper hash for the name. */
		name_hash = Dee_HashStr(name);
		perturb = hash_i = name_hash & bucket_mask;
		for (;; Dee_MODULE_HASHNX(hash_i, perturb)) {
			DREF DeeStringObject *temp;
			struct module_symbol *target = &bucketv[hash_i & bucket_mask];
			if (target->ss_name)
				continue;
			/* Found an unused slot compatible with the hash of this symbol's name. */
			temp = (DREF DeeStringObject *)DeeString_New(name);
			if unlikely(!temp)
				goto err_symbolv;
			temp->s_hash    = name_hash; /* Save the name hash. */
			target->ss_name = DeeString_STR(temp);
			flags |= MODSYM_FNAMEOBJ;
			if (doclen) {
				/* Allocate the documentation string. */
				temp = (DREF DeeStringObject *)DeeString_NewUtf8(doc,
				                                                 doclen,
				                                                 STRING_ERROR_FSTRICT);
				if unlikely(!temp)
					goto err_symbolv;
				target->ss_doc = DeeString_STR(temp);
				flags |= MODSYM_FDOCOBJ;
			}
			target->ss_extern.ss_symid = addr;
			target->ss_extern.ss_impid = addr2;
			target->ss_hash            = name_hash;
			target->ss_flags           = flags;
			break;
		}
	}

	/* Allocate and setup the global variable vector. */
	module->mo_globalv = (DREF DeeObject **)Dee_Callocc(globalc, sizeof(DREF DeeObject *));
	if unlikely(!module->mo_globalv)
		goto err_symbolv;
	module->mo_globalc = globalc;

	/* Install the symbol mask and hash-table. */
	module->mo_bucketm = bucket_mask;
	module->mo_bucketv = bucketv;

	result = 0;
stop:
	return result;
err_symbolv:
	result = -1;
stop_symbolv:
	do {
		if (bucketv[bucket_mask].ss_name)
			Dee_Decref(COMPILER_CONTAINER_OF(bucketv[bucket_mask].ss_name, DeeStringObject, s_str));
		if (bucketv[bucket_mask].ss_doc)
			Dee_Decref(COMPILER_CONTAINER_OF(bucketv[bucket_mask].ss_doc, DeeStringObject, s_str));
	} while (bucket_mask--);
	Dee_Free(bucketv);
	goto stop;
err:
	result = -1;
	goto stop;
}


INTDEF struct class_operator empty_class_operators[];
INTDEF struct class_attribute empty_class_attributes[];


/* @return: * :        A reference to the object that got loaded.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The DEC file has been corrupted. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DecFile_LoadObject(DecFile *__restrict self,
                   uint8_t const **__restrict p_reader) {
	DREF DeeObject *result = ITER_DONE;
	uint8_t const *reader;
	uint8_t opcode;
	reader = *p_reader;
	opcode = *reader++;
	switch (opcode) {

	case DTYPE_NONE:
set_none_result:
		result = DeeNone_NewRef();
		break;

	case DTYPE_IEEE754: {
		union {
			double value;
			uint64_t data;
		} buffer;
		buffer.data = UNALIGNED_GETLE64(reader), reader += 8;
		/* XXX: Special decoding when `double' doesn't conform to ieee754 */
		result = DeeFloat_New(buffer.value);
	}	break;

	/* Variable-length integers. */
	case DTYPE_SLEB:
		result = DeeInt_NewSleb(&reader);
		break;

	case DTYPE_ULEB:
		result = DeeInt_NewUleb(&reader);
		break;

	/* A string that is apart of the string table. */
	case DTYPE_STRING: {
		uint32_t len;
		len = Dec_DecodePointer(&reader);
		if (!len) {
			/* Special case: Empty string. */
			result = Dee_EmptyString;
			Dee_Incref(result);
		} else {
			char const *str;
			uint32_t ptr;
			ptr = Dec_DecodePointer(&reader);
			str = (char const *)(self->df_base + LETOH32(self->df_ehdr->e_stroff) + ptr);
			if unlikely(ptr + len < ptr)
				GOTO_CORRUPTED(reader, done); /* Check for overflow. */
			if unlikely(str + len > (char const *)(self->df_base + self->df_size))
				GOTO_CORRUPTED(reader, done); /* Validate bounds. */
			/* Create the new string. */
			result = DeeString_NewUtf8(str, len, STRING_ERROR_FSTRICT);
		}
	}	break;

	/* Another code object. */
	case DTYPE_CODE:
		result = (DREF DeeObject *)DecFile_LoadCode(self, &reader);
		break;

	/* A function object. */
	case DTYPE_FUNCTION: {
		DREF DeeCodeObject *code;
		uint16_t i, refc;
		code = DecFile_LoadCode(self, &reader);
		if unlikely(!ITER_ISOK(code)) {
			if unlikely(!code)
				goto err;
			goto corrupt;
		}
		refc = code->co_refc;
		ASSERT(code->co_refstaticc >= refc);
		if likely(code->co_refstaticc == refc) {
			result = (DREF DeeObject *)DeeGCObject_Mallocc(offsetof(DeeFunctionObject, fo_refv),
			                                               refc, sizeof(DREF DeeObject *));
		} else {
			result = (DREF DeeObject *)DeeGCObject_Callocc(offsetof(DeeFunctionObject, fo_refv),
			                                               code->co_refstaticc, sizeof(DREF DeeObject *));
		}
		if unlikely(!result) {
err_function_code:
			Dee_Decref(code);
			goto err;
		}
		for (i = 0; i < refc; ++i) {
			DREF DeeObject *temp;
			temp = DecFile_LoadObject(self, &reader);
			if unlikely(!ITER_ISOK(temp)) {
				Dee_Decrefv(((DREF DeeFunctionObject *)result)->fo_refv, i);
				DeeObject_Free(result);
				if (!temp)
					goto err_function_code;
				Dee_Decref(code);
				goto corrupt;
			}
			((DREF DeeFunctionObject *)result)->fo_refv[i] = temp; /* Inherit reference. */
		}
		((DREF DeeFunctionObject *)result)->fo_code = code; /* Inherit reference. */
#ifdef CONFIG_HAVE_CODE_METRICS
		atomic_inc(&code->co_metrics.com_functions);
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
		Dee_hostasm_function_init(&((DREF DeeFunctionObject *)result)->fo_hostasm);
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */
		DeeObject_Init(result, &DeeFunction_Type);
		Dee_atomic_rwlock_init(&((DREF DeeFunctionObject *)result)->fo_reflock);
		result = DeeGC_Track(result);
	}	break;

	case DTYPE_TUPLE: {
		uint32_t i, length;
		uint8_t const *end;
		length = Dec_DecodePointer(&reader);
		result = (DREF DeeObject *)DeeTuple_NewUninitialized(length);
		if unlikely(!result)
			goto done;
		end = self->df_data + self->df_size;
		for (i = 0; i < length; ++i) {
			DREF DeeObject *item;
			/* Read the individual tuple items. */
			if unlikely(reader >= end) {
				item = ITER_DONE;
			} else {
				item = DecFile_LoadObject(self, &reader);
			}
			if unlikely(!ITER_ISOK(item)) {
				Dee_Decrefv(DeeTuple_ELEM(result), i);
				DeeTuple_FreeUninitialized((DeeTupleObject *)result);
				result = item;
				goto done;
			}
			/* Save the item within the tuple. */
			DeeTuple_SET(result, i, item);
		}
	}	break;

	case DTYPE_LIST: {
		uint32_t i, length;
		uint8_t const *end;
		DeeListObject *ret;
		length = Dec_DecodePointer(&reader);
		ret    = DeeList_NewUninitialized(length);
		if unlikely(!ret)
			goto err;
		end = self->df_data + self->df_size;
		for (i = 0; i < length; ++i) {
			DREF DeeObject *item;

			/* Read the individual list items. */
			if unlikely(reader >= end) {
				item = ITER_DONE;
			} else {
				item = DecFile_LoadObject(self, &reader);
			}
			if unlikely(!ITER_ISOK(item)) {
				Dee_Decrefv(DeeList_ELEM(ret), i);
				DeeList_FreeUninitialized(ret);
				result = item;
				goto done;
			}

			/* Save the item within the list. */
			DeeList_SET(ret, i, item);
		}

		/* Start tracking the new list now that it's been initialized. */
		result = DeeList_FinalizeUninitialized(ret);
	}	break;

	case DTYPE_CLASSDESC: {
		uint8_t flags, cmemb_size, imemb_size, i;
		uint8_t op_count, cattr_count, iattr_count;
		Dee_operator_t opbind_mask;
		size_t cattr_mask, iattr_mask;
		char const *strtab, *fileend;
		char const *name, *doc;
		size_t doclen;
#ifdef __INTELLISENSE__
		DeeClassDescriptorObject *descriptor;
#else /* __INTELLISENSE__ */
#define descriptor ((DeeClassDescriptorObject *)result)
#endif /* !__INTELLISENSE__ */
		/* 8-bit class descriptor. */
		flags   = *(uint8_t *)reader, reader += 1;
		fileend = (char const *)(self->df_base + self->df_size);
		strtab  = (char const *)(self->df_base + LETOH32(self->df_ehdr->e_stroff));
		name    = strtab + Dec_DecodePointer(&reader);
		if unlikely(name < strtab || name >= fileend)
			GOTO_CORRUPTED(reader, corrupt);
		doclen = Dec_DecodePointer(&reader);
		doc    = NULL;
		if (doclen) {
			doc = strtab + Dec_DecodePointer(&reader);
			if unlikely(doc < strtab || doc >= fileend)
				GOTO_CORRUPTED(reader, corrupt);
		}
		cmemb_size  = UNALIGNED_GETLE8(reader), reader += 1;
		imemb_size  = UNALIGNED_GETLE8(reader), reader += 1;
		op_count    = UNALIGNED_GETLE8(reader), reader += 1;
		cattr_count = UNALIGNED_GETLE8(reader), reader += 1;
		iattr_count = UNALIGNED_GETLE8(reader), reader += 1;
		iattr_mask  = 0;
		if (iattr_count) {
			while (iattr_count > (iattr_mask / 3) * 2)
				iattr_mask = (iattr_mask << 1) | 1;
		}
		result = (DREF DeeObject *)DeeObject_Callocc(offsetof(DeeClassDescriptorObject, cd_iattr_list),
		                                             iattr_mask + 1, sizeof(struct class_attribute));
		if unlikely(!result)
			goto err;
		DeeObject_Init(result, &DeeClassDescriptor_Type);
		descriptor->cd_flags      = flags;
		descriptor->cd_cattr_list = empty_class_attributes;
		descriptor->cd_clsop_list = empty_class_operators;
		descriptor->cd_iattr_mask = iattr_mask;
		descriptor->cd_cmemb_size = cmemb_size;
		descriptor->cd_imemb_size = imemb_size;
		/* Load the class's name and documentation string from the string table. */
		if (*name && (descriptor->cd_name = (DREF DeeStringObject *)DeeString_New(name)) == NULL)
			goto err_r;
		if (doclen && (descriptor->cd_doc = (DREF DeeStringObject *)
		               DeeString_NewUtf8(doc, doclen, STRING_ERROR_FSTRICT)) == NULL)
			goto err_r;
		if (op_count) {
			struct class_operator *opbind_list;
			/* Load the operator descriptor table. */
			if (reader + op_count * 2 > (uint8_t *)fileend)
				GOTO_CORRUPTED(reader, corrupt_r);
			opbind_mask = 0;
			while (op_count > (opbind_mask / 3) * 2)
				opbind_mask = (opbind_mask << 1) | 1;
			opbind_list = (struct class_operator *)Dee_Mallocc(opbind_mask + 1,
			                                                   sizeof(struct class_operator));
			if unlikely(!opbind_list)
				goto err_r;
			memset(opbind_list, 0xff,
			       (opbind_mask + 1) *
			       sizeof(struct class_operator));
			descriptor->cd_clsop_mask = opbind_mask;
			descriptor->cd_clsop_list = opbind_list;
			for (i = 0; i < op_count; ++i) {
				struct class_operator *entry;
				uint8_t opname, opaddr;
				Dee_operator_t j, perturb;
				opname = UNALIGNED_GETLE8(reader), reader += 1;
				opaddr = UNALIGNED_GETLE8(reader), reader += 1;
				if (opaddr >= cmemb_size)
					GOTO_CORRUPTED(reader, corrupt_r);
				j = perturb = opname & opbind_mask;
				for (;; DeeClassDescriptor_CLSOPNEXT(j, perturb)) {
					entry = &opbind_list[j & opbind_mask];
					if (entry->co_name == (Dee_operator_t)-1)
						break;
				}
				entry->co_name = opname;
				entry->co_addr = opaddr;
			}
		}
		if (cattr_count) {
			struct class_attribute *cattr_list;
			/* Load the class attribute descriptor table. */
			cattr_mask = 0;
			while (cattr_count > (cattr_mask / 3) * 2)
				cattr_mask = (cattr_mask << 1) | 1;
			cattr_list = (struct class_attribute *)Dee_Callocc(cattr_mask + 1,
			                                                   sizeof(struct class_attribute));
			if unlikely(!cattr_list)
				goto err_r;
			descriptor->cd_cattr_list = cattr_list;
			descriptor->cd_cattr_mask = cattr_mask;
			for (i = 0; i < cattr_count; ++i) {
				struct class_attribute *entry;
				Dee_hash_t j, perturb, hash;
				uint8_t ataddr, atflags;
				DREF DeeStringObject *name_ob;
				if unlikely(reader >= (uint8_t const *)fileend)
					GOTO_CORRUPTED(reader, corrupt_r);
				ataddr  = UNALIGNED_GETLE8(reader), reader += 1;
				atflags = UNALIGNED_GETLE8(reader), reader += 1;
				if unlikely(atflags & ~CLASS_ATTRIBUTE_FMASK)
					GOTO_CORRUPTED(reader, corrupt_r);
				if unlikely(ataddr >= cmemb_size)
					GOTO_CORRUPTED(reader, corrupt_r);
				name = strtab + Dec_DecodePointer(&reader);
				if unlikely(name < strtab || name >= fileend)
					GOTO_CORRUPTED(reader, corrupt_r);
				doclen = Dec_DecodePointer(&reader);
				doc    = NULL;
				if (doclen) {
					doc = strtab + Dec_DecodePointer(&reader);
					if unlikely(doc < strtab || doc >= fileend)
						GOTO_CORRUPTED(reader, corrupt_r);
				}
				name_ob = (DREF DeeStringObject *)DeeString_New(name);
				if unlikely(!name_ob)
					goto err_r;
				hash = DeeString_Hash(name_ob);
				j = perturb = hash & cattr_mask;
				for (;; DeeClassDescriptor_CATTRNEXT(j, perturb)) {
					entry = &cattr_list[j & cattr_mask];
					if (!entry->ca_name)
						break;
				}
				entry->ca_name = name_ob; /* Inherit reference. */
				entry->ca_hash = hash;
				if (doclen) {
					entry->ca_doc = (DREF DeeStringObject *)DeeString_NewUtf8(doc, doclen, STRING_ERROR_FSTRICT);
					if unlikely(!entry->ca_doc)
						goto err_r;
				}
				entry->ca_addr = ataddr;
				entry->ca_flag = atflags;
			}
		}
		/* Load the instance attribute descriptor table. */
		for (i = 0; i < iattr_count; ++i) {
			struct class_attribute *entry;
			Dee_hash_t j, perturb, hash;
			uint8_t ataddr, atflags;
			DREF DeeStringObject *name_ob;
			if unlikely(reader >= (uint8_t const *)fileend)
				GOTO_CORRUPTED(reader, corrupt_r);
			ataddr  = UNALIGNED_GETLE8(reader), reader += 1;
			atflags = UNALIGNED_GETLE8(reader), reader += 1;
			if unlikely(atflags & ~CLASS_ATTRIBUTE_FMASK)
				GOTO_CORRUPTED(reader, corrupt_r);
			if unlikely(ataddr >= ((atflags & CLASS_ATTRIBUTE_FCLASSMEM) ? cmemb_size : imemb_size))
				GOTO_CORRUPTED(reader, corrupt_r);
			name = strtab + Dec_DecodePointer(&reader);
			if unlikely(name < strtab || name >= fileend)
				GOTO_CORRUPTED(reader, corrupt_r);
			doclen = Dec_DecodePointer(&reader);
			doc    = NULL;
			if (doclen) {
				doc = strtab + Dec_DecodePointer(&reader);
				if unlikely(doc < strtab || doc >= fileend)
					GOTO_CORRUPTED(reader, corrupt_r);
			}
			name_ob = (DREF DeeStringObject *)DeeString_New(name);
			if unlikely(!name_ob)
				goto err_r;
			hash = DeeString_Hash(name_ob);
			j = perturb = hash & iattr_mask;
			for (;; DeeClassDescriptor_IATTRNEXT(j, perturb)) {
				entry = &descriptor->cd_iattr_list[j & iattr_mask];
				if (!entry->ca_name)
					break;
			}
			entry->ca_name = name_ob; /* Inherit reference. */
			entry->ca_hash = hash;
			if (doclen) {
				entry->ca_doc = (DREF DeeStringObject *)DeeString_NewUtf8(doc, doclen, STRING_ERROR_FSTRICT);
				if unlikely(!entry->ca_doc)
					goto err_r;
			}
			entry->ca_addr = ataddr;
			entry->ca_flag = atflags;
		}
#undef descriptor
	}	break;

	case DTYPE_KWDS: {
		uint32_t i, count;
		char const *strtab;
		uint8_t const *end;
		/* Invocation keywords descriptor. */
		count  = Dec_DecodePointer(&reader);
		result = DeeKwds_NewWithHint(count);
		if unlikely(!result)
			goto done;
		strtab = (char const *)(self->df_base + LETOH32(self->df_ehdr->e_stroff));
		end    = self->df_base + self->df_size;
		for (i = 0; i < count; ++i) {
			uint32_t addr;
			char const *name;
			size_t name_len;
			if unlikely(reader >= end)
				GOTO_CORRUPTED(reader, corrupt_r); /* Validate bounds. */
			addr = Dec_DecodePointer(&reader);
			name = strtab + addr;
			if unlikely(name >= (char const *)end)
				GOTO_CORRUPTED(reader, corrupt_r); /* Validate bounds. */
			name_len = strlen(name);
			if unlikely(DeeKwds_AppendStringLenHash(&result, name, name_len,
			                              Dee_HashPtr(name, name_len)))
				goto err_r;
		}
	}	break;


	case DTYPE_EXTENDED:
		opcode = *reader++;
		switch (opcode) {

		case DTYPE16_NONE & 0xff:
			goto set_none_result;

		case DTYPE16_HASHSET & 0xff: {
			uint32_t num_items;
			uint8_t const *end;
			num_items = Dec_DecodePointer(&reader);
			result    = DeeHashSet_New();
			if unlikely(!result)
				goto done;
			end = self->df_data + self->df_size;
			while (num_items--) {
				DREF DeeObject *item;
				int error;
				/* Read the individual set items. */
				if unlikely(reader >= end) {
					item = SET_CORRUPTED(reader, ITER_DONE);
				} else {
					item = DecFile_LoadObject(self, &reader);
				}
				if unlikely(!ITER_ISOK(item)) {
					Dee_Decref(result);
					result = item;
					goto done;
				}
				/* Insert the item into the result set. */
				error = DeeHashSet_Insert(result, item);
				Dee_Decref(item);
				if unlikely(error)
					goto err_r;
			}
		}	break;

		case DTYPE16_ROSET & 0xff: {
			uint32_t num_items;
			uint8_t const *end;
			num_items = Dec_DecodePointer(&reader);
			result    = (DREF DeeObject *)DeeRoSet_NewWithHint(num_items);
			if unlikely(!result)
				goto done;
			end = self->df_data + self->df_size;
			while (num_items--) {
				DREF DeeObject *item;
				int error;

				/* Read the individual set items. */
				if unlikely(reader >= end) {
					item = SET_CORRUPTED(reader, ITER_DONE);
				} else {
					item = DecFile_LoadObject(self, &reader);
				}
				if unlikely(!ITER_ISOK(item)) {
					Dee_Decref(result);
					result = item;
					goto done;
				}

				/* Insert the item into the result set. */
				error = DeeRoSet_Insert((DREF DeeRoSetObject **)&result, item);
				Dee_Decref(item);
				if unlikely(error)
					goto err_r;
			}
		}	break;

		case DTYPE16_DICT & 0xff: {
			uint32_t num_items;
			uint8_t const *end;
			num_items = Dec_DecodePointer(&reader);
			result    = DeeDict_New();
			if unlikely(!result)
				goto done;
			end = self->df_data + self->df_size;
			while (num_items--) {
				DREF DeeObject *key, *value;
				int error;
				/* Read the individual Dict key-item pairs. */
				if unlikely(reader >= end) {
					key = SET_CORRUPTED(reader, ITER_DONE);
				} else {
					key = DecFile_LoadObject(self, &reader);
				}
				if unlikely(!ITER_ISOK(key)) {
					Dee_Decref(result);
					result = key;
					goto done;
				}
				if unlikely(reader >= end) {
					value = SET_CORRUPTED(reader, ITER_DONE);
				} else {
					value = DecFile_LoadObject(self, &reader);
				}
				if unlikely(!ITER_ISOK(value)) {
					Dee_Decref(value);
					Dee_Decref(result);
					result = value;
					goto done;
				}
				/* Insert the key and item into the Dict. */
				error = (*DeeDict_Type.tp_seq->tp_setitem)(result, key, value);
				Dee_Decref(value);
				Dee_Decref(key);
				if unlikely(error)
					goto err_r;
			}
		}	break;

		case DTYPE16_RODICT & 0xff: {
			uint32_t num_items;
			uint8_t const *end;
			struct Dee_rodict_builder result_builder;
			num_items = Dec_DecodePointer(&reader);
			Dee_rodict_builder_init_with_hint(&result_builder, num_items);
			end = self->df_data + self->df_size;
			while (num_items--) {
				DREF DeeObject *key, *value;
				/* Read the individual Dict key-item pairs. */
				if unlikely(reader >= end) {
					key = SET_CORRUPTED(reader, ITER_DONE);
				} else {
					key = DecFile_LoadObject(self, &reader);
				}
				if unlikely(!ITER_ISOK(key)) {
					Dee_rodict_builder_fini(&result_builder);
					result = key;
					goto done;
				}
				if unlikely(reader >= end) {
					value = SET_CORRUPTED(reader, ITER_DONE);
				} else {
					value = DecFile_LoadObject(self, &reader);
				}
				if unlikely(!ITER_ISOK(value)) {
					Dee_Decref(value);
					Dee_rodict_builder_fini(&result_builder);
					result = value;
					goto done;
				}
				/* Insert the key and item into the Dict. */
				if unlikely(Dee_rodict_builder_setitem_inherited(&result_builder, key, value)) {
					Dee_rodict_builder_fini(&result_builder);
					goto err;
				}
			}
			result = (DREF DeeObject *)Dee_rodict_builder_pack(&result_builder);
		}	break;

		case DTYPE16_CLASSDESC & 0xff: {
			Dee_operator_t op_count, opbind_mask;
			uint16_t flags, cmemb_size, imemb_size;
			uint32_t cattr_count, iattr_count, i;
			size_t cattr_mask, iattr_mask;
			char const *strtab, *fileend;
			char const *name, *doc;
			size_t doclen;
#ifdef __INTELLISENSE__
			DeeClassDescriptorObject *descriptor;
#else /* __INTELLISENSE__ */
#define descriptor ((DeeClassDescriptorObject *)result)
#endif /* !__INTELLISENSE__ */
			/* 16-bit class descriptor. */
			flags   = UNALIGNED_GETLE16(reader), reader += 2;
			fileend = (char const *)(self->df_base + self->df_size);
			strtab  = (char const *)(self->df_base + LETOH32(self->df_ehdr->e_stroff));
			name    = strtab + Dec_DecodePointer(&reader);
			if unlikely(name < strtab || name >= fileend)
				GOTO_CORRUPTED(reader, corrupt);
			doclen = Dec_DecodePointer(&reader);
			doc    = NULL;
			if (doclen) {
				doc = strtab + Dec_DecodePointer(&reader);
				if unlikely(doc < strtab || doc >= fileend)
					GOTO_CORRUPTED(reader, corrupt);
			}
			cmemb_size  = UNALIGNED_GETLE16(reader), reader += 2;
			imemb_size  = UNALIGNED_GETLE16(reader), reader += 2;
			op_count    = UNALIGNED_GETLE16(reader), reader += 2;
			cattr_count = Dec_DecodePointer(&reader);
			iattr_count = Dec_DecodePointer(&reader);
			iattr_mask  = 0;
			if (iattr_count) {
#if __SIZEOF_POINTER__ < 8
				if unlikely(iattr_count > (((size_t)-1) / 3) * 2)
					GOTO_CORRUPTED(reader, corrupt);
#endif /* __SIZEOF_POINTER__ < 8 */
				while (iattr_count > (iattr_mask / 3) * 2)
					iattr_mask = (iattr_mask << 1) | 1;
			}
			result = (DREF DeeObject *)DeeObject_Callocc(offsetof(DeeClassDescriptorObject, cd_iattr_list),
			                                             iattr_mask + 1, sizeof(struct class_attribute));
			if unlikely(!result)
				goto err;
			DeeObject_Init(result, &DeeClassDescriptor_Type);
			descriptor->cd_flags      = flags;
			descriptor->cd_cattr_list = empty_class_attributes;
			descriptor->cd_clsop_list = empty_class_operators;
			descriptor->cd_iattr_mask = iattr_mask;
			descriptor->cd_cmemb_size = cmemb_size;
			descriptor->cd_imemb_size = imemb_size;
			/* Load the class's name and documentation string from the string table. */
			if (*name && (descriptor->cd_name = (DREF DeeStringObject *)DeeString_New(name)) == NULL)
				goto err_r;
			if (doclen && (descriptor->cd_doc = (DREF DeeStringObject *)
			               DeeString_NewUtf8(doc, doclen, STRING_ERROR_FSTRICT)) == NULL)
				goto err_r;
			if (op_count) {
				struct class_operator *opbind_list;
				/* Load the operator descriptor table. */
				if (reader + op_count * 4 > (uint8_t *)fileend)
					GOTO_CORRUPTED(reader, corrupt_r);
				opbind_mask = 0;
				while (op_count > (opbind_mask / 3) * 2)
					opbind_mask = (opbind_mask << 1) | 1;
				opbind_list = (struct class_operator *)Dee_Mallocc(opbind_mask + 1,
				                                                   sizeof(struct class_operator));
				if unlikely(!opbind_list)
					goto err_r;
				memset(opbind_list, 0xff,
				       (opbind_mask + 1) *
				       sizeof(struct class_operator));
				descriptor->cd_clsop_mask = opbind_mask;
				descriptor->cd_clsop_list = opbind_list;
				for (i = 0; i < op_count; ++i) {
					struct class_operator *entry;
					Dee_operator_t opname, j, perturb;
					uint16_t opaddr;
					opname = UNALIGNED_GETLE16(reader), reader += 2;
					opaddr = UNALIGNED_GETLE16(reader), reader += 2;
					if unlikely(opaddr == (uint16_t)-1)
						GOTO_CORRUPTED(reader, corrupt_r);
					if (opaddr >= cmemb_size)
						GOTO_CORRUPTED(reader, corrupt_r);
					j = perturb = opname & opbind_mask;
					for (;; DeeClassDescriptor_CLSOPNEXT(j, perturb)) {
						entry = &opbind_list[j & opbind_mask];
						if (entry->co_name == (Dee_operator_t)-1)
							break;
					}
					entry->co_name = opname;
					entry->co_addr = opaddr;
				}
			}
			if (cattr_count) {
				struct class_attribute *cattr_list;
				/* Load the class attribute descriptor table. */
				cattr_mask = 0;
#if __SIZEOF_POINTER__ < 8
				if unlikely(cattr_count > (((size_t)-1) / 3) * 2)
					GOTO_CORRUPTED(reader, corrupt_r);
#endif /* __SIZEOF_POINTER__ < 8 */
				while (cattr_count > (cattr_mask / 3) * 2)
					cattr_mask = (cattr_mask << 1) | 1;
				cattr_list = (struct class_attribute *)Dee_Callocc(cattr_mask + 1,
				                                                   sizeof(struct class_attribute));
				if unlikely(!cattr_list)
					goto err_r;
				descriptor->cd_cattr_list = cattr_list;
				descriptor->cd_cattr_mask = cattr_mask;
				for (i = 0; i < cattr_count; ++i) {
					struct class_attribute *entry;
					Dee_hash_t j, perturb, hash;
					uint16_t ataddr, atflags;
					DREF DeeStringObject *name_ob;
					if unlikely(reader >= (uint8_t const *)fileend)
						GOTO_CORRUPTED(reader, corrupt_r);
					ataddr  = UNALIGNED_GETLE16(reader), reader += 2;
					atflags = UNALIGNED_GETLE16(reader), reader += 2;
					if unlikely(atflags & ~CLASS_ATTRIBUTE_FMASK)
						GOTO_CORRUPTED(reader, corrupt_r);
					if unlikely(ataddr >= cmemb_size)
						GOTO_CORRUPTED(reader, corrupt_r);
					name = strtab + Dec_DecodePointer(&reader);
					if unlikely(name < strtab || name >= fileend)
						GOTO_CORRUPTED(reader, corrupt_r);
					doclen = Dec_DecodePointer(&reader);
					doc    = NULL;
					if (doclen) {
						doc = strtab + Dec_DecodePointer(&reader);
						if unlikely(doc < strtab || doc >= fileend)
							GOTO_CORRUPTED(reader, corrupt_r);
					}
					name_ob = (DREF DeeStringObject *)DeeString_New(name);
					if unlikely(!name_ob)
						goto err_r;
					hash = DeeString_Hash(name_ob);
					j = perturb = hash & cattr_mask;
					for (;; DeeClassDescriptor_CATTRNEXT(j, perturb)) {
						entry = &cattr_list[j & cattr_mask];
						if (!entry->ca_name)
							break;
					}
					entry->ca_name = name_ob; /* Inherit reference. */
					entry->ca_hash = hash;
					if (doclen) {
						entry->ca_doc = (DREF DeeStringObject *)DeeString_NewUtf8(doc, doclen, STRING_ERROR_FSTRICT);
						if unlikely(!entry->ca_doc)
							goto err_r;
					}
					entry->ca_addr = ataddr;
					entry->ca_flag = atflags;
				}
			}
			/* Load the instance attribute descriptor table. */
			for (i = 0; i < iattr_count; ++i) {
				struct class_attribute *entry;
				Dee_hash_t j, perturb, hash;
				uint16_t ataddr, atflags;
				DREF DeeStringObject *name_ob;
				if unlikely(reader >= (uint8_t const *)fileend)
					GOTO_CORRUPTED(reader, corrupt_r);
				ataddr  = UNALIGNED_GETLE16(reader), reader += 2;
				atflags = UNALIGNED_GETLE16(reader), reader += 2;
				if unlikely(atflags & ~CLASS_ATTRIBUTE_FMASK)
					GOTO_CORRUPTED(reader, corrupt_r);
				if unlikely(ataddr >= ((atflags & CLASS_ATTRIBUTE_FCLASSMEM) ? cmemb_size : imemb_size))
					GOTO_CORRUPTED(reader, corrupt_r);
				name = strtab + Dec_DecodePointer(&reader);
				if unlikely(name < strtab || name >= fileend)
					GOTO_CORRUPTED(reader, corrupt_r);
				doclen = Dec_DecodePointer(&reader);
				doc    = NULL;
				if (doclen) {
					doc = strtab + Dec_DecodePointer(&reader);
					if unlikely(doc < strtab || doc >= fileend)
						GOTO_CORRUPTED(reader, corrupt_r);
				}
				name_ob = (DREF DeeStringObject *)DeeString_New(name);
				if unlikely(!name_ob)
					goto err_r;
				hash = DeeString_Hash(name_ob);
				j = perturb = hash & iattr_mask;
				for (;; DeeClassDescriptor_IATTRNEXT(j, perturb)) {
					entry = &descriptor->cd_iattr_list[j & iattr_mask];
					if (!entry->ca_name)
						break;
				}
				entry->ca_name = name_ob; /* Inherit reference. */
				entry->ca_hash = hash;
				if (doclen) {
					entry->ca_doc = (DREF DeeStringObject *)DeeString_NewUtf8(doc, doclen, STRING_ERROR_FSTRICT);
					if unlikely(!entry->ca_doc)
						goto err_r;
				}
				entry->ca_addr = ataddr;
				entry->ca_flag = atflags;
			}
#undef descriptor
		}	break;

		case DTYPE16_CELL & 0xff:
			if (*reader == DTYPE_NULL) {
				/* When followed by `DTYPE_NULL', create an empty Cell. */
				++reader;
				result = DeeCell_NewEmpty();
			} else {
				result = DecFile_LoadObject(self, &reader);
				if likely(ITER_ISOK(result)) {
					/* Pack the read object into a Cell. */
					DREF DeeObject *new_result;
					new_result = DeeCell_New(result);
					Dee_Decref(result);
					result = new_result;
				}
			}
			break;

		default:
			if unlikely(opcode < (DTYPE16_BUILTIN_MIN & 0xff))
				GOTO_CORRUPTED(reader, done);
			/* Load a builtin object from a custom data set. */
			result = Dec_GetBuiltin(opcode - (DTYPE16_BUILTIN_MIN & 0xff), *(uint8_t *)reader);
			if unlikely(!result) {
				result = SET_CORRUPTED(reader, ITER_DONE);
			} else {
				Dee_Incref(result);
			}
			++reader;
			break;
		}
		break;

	default:
		if unlikely(opcode < DTYPE_BUILTIN_MIN)
			GOTO_CORRUPTED(reader, done);
#if DTYPE_BUILTIN_MAX != 0xff
		if unlikely(opcode > DTYPE_BUILTIN_MAX)
			GOTO_CORRUPTED(reader, done);
#endif /* DTYPE_BUILTIN_MAX != 0xff */
		/* Load a builtin object. */
		result = Dec_GetBuiltin(self->df_ehdr->e_builtinset, opcode);
		if unlikely(!result) {
			result = SET_CORRUPTED(reader, ITER_DONE);
		} else {
			Dee_Incref(result);
		}
		break;
	}
done:
	*p_reader = reader;
	return result;
err_r:
	Dee_Decref(result);
err:
	result = NULL;
	goto done;
corrupt_r:
	Dee_Decref(result);
corrupt:
	result = ITER_DONE;
	goto done;
}


/* @param: allow_dtype_null: When true, individual vector elements are allowed
 *                           to be `NULL' as the result of `DTYPE_NULL'
 * @return: * :              Newly heap-allocated vector of objects (length is stored in `*p_count').
 * @return: NULL:            An error occurred.
 * @return: ITER_DONE:       The DEC file has been corrupted. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject **DCALL
DecFile_LoadObjectVector(DecFile *__restrict self,
                         uint16_t *__restrict p_count,
                         uint8_t const **__restrict p_reader,
                         bool allow_dtype_null) {
	DREF DeeObject **result;
	uint8_t const *reader = *p_reader;
	uint8_t const *end    = self->df_base + self->df_size;
	uint16_t i, count;
	void *new_result;
	count    = UNALIGNED_GETLE16(reader);
	*p_count = count;
	reader += 2;
	result = (DREF DeeObject **)Dee_Mallocc(count, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto err;
	for (i = 0; i < count; ++i) {
		/* Validate the the vector is still in-bounds. */
		if unlikely(reader >= end) {
			new_result = ITER_DONE;
			GOTO_CORRUPTED(reader, read_failed);
		}
		if (allow_dtype_null && *reader == DTYPE_NULL) {
			result[i] = NULL;
			++reader;
		} else {
			/* Read one object. */
			result[i] = DecFile_LoadObject(self, &reader);
			if unlikely(!ITER_ISOK(result[i])) {
				new_result = result[i];
read_failed:
				Dee_Decrefv(result, i);
				Dee_Free(result);
				*p_reader = reader;
				return (DREF DeeObject **)new_result;
			}
		}
	}
	*p_reader = reader;
	return result;
err:
	return NULL;
}

#define decode_uleb(p_ptr) Dec_DecodePointer(p_ptr)

LOCAL int32_t DCALL
decode_sleb(uint8_t const **__restrict p_ptr) {
	int32_t result;
	uint8_t byte;
	uint8_t const *ptr = *p_ptr;
	uint8_t num_bits;
	bool is_neg;
	byte     = *ptr++;
	result   = byte & 0x3f;
	is_neg   = (byte & 0x40) != 0;
	num_bits = 6;
	while (byte & 0x80) {
		byte = *ptr++;
		result |= (byte & 0x7f) << num_bits;
		num_bits += 7;
	}
	*p_ptr = ptr;
	if (is_neg)
		result = -result;
	return result;
}

PRIVATE int DCALL
load_strmap(DecFile *__restrict self,
            uint32_t map_addr,
            uint32_t *__restrict pmaplen,
            uint32_t const **__restrict pmapvec) {
	uint32_t i, map_length;
	uint8_t const *reader;
	uint32_t *vector;
	uint32_t string_size;
	if (!map_addr)
		return 0; /* Undefined map. */
	if unlikely(map_addr + 2 >= self->df_size)
		GOTO_CORRUPTED(self->df_base + map_addr, err_currupt); /* Map is out-of-bounds. */
	reader     = self->df_base + map_addr;
	map_length = UNALIGNED_GETLE16(reader), reader += 2;
	if unlikely(!map_length)
		return 0; /* Empty map (same as undefined). */
	if unlikely(map_length == (uint16_t)-1) {
		map_length = UNALIGNED_GETLE32(reader), reader += 4;
		if unlikely(!map_length)
			return 0; /* Empty map (same as undefined). */
	}
	if unlikely(map_length > (self->df_size - (map_addr + 2)))
		GOTO_CORRUPTED(reader - 2, err_currupt); /* Map items are out-of-bounds. */
	/* Allocate the map vector. */
	vector = (uint32_t *)Dee_Mallocc(map_length, sizeof(uint32_t));
	if unlikely(!vector)
		goto err;

	string_size = LETOH32(self->df_ehdr->e_strsiz);
	/* Read vector contents. */
	for (i = 0; i < map_length; ++i) {
		uint32_t pointer;
		pointer = Dec_DecodePointer(&reader);
		/* Validate that the pointer fits into the string-table. */
		if unlikely(pointer >= string_size) {
			GOTO_CORRUPTEDF(reader, err_currupt_vec,
			                "[%" PRFu16 "/%" PRFu16 "] pointer = %#" PRFx32 ", string_size = %#" PRFx32 "",
			                i, map_length, pointer, string_size);
		}
		vector[i] = pointer;
	}

	/* Fill in the caller-provided data fields. */
	*pmapvec = vector;
	*pmaplen = map_length;
	return 0;
err_currupt_vec:
	Dee_Free(vector);
err_currupt:
	return 1;
err:
	return -1;
}


/* @return: * :        New reference to a ddi object.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The DEC file has been corrupted. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeDDIObject *DCALL
DecFile_LoadDDI(DecFile *__restrict self,
                uint8_t const *__restrict reader,
                bool is_8bit_ddi) {
	int map_error;
	DREF DeeDDIObject *result;
	uint8_t const *ddi_text;
	uint32_t ddi_strings; /* Absolute pointer to a `Dec_Strmap' structure describing DDI string. */
	uint32_t ddi_ddixdat; /* Absolute pointer to a `Dec_DDIExdat' structure, or 0. */
	uint32_t ddi_ddiaddr; /* Absolute offset into the file to a block of `cd_ddisize' bytes of text describing DDI code (s.a.: `DDI_*'). */
	uint32_t ddi_ddisize; /* The total size (in bytes) of DDI text for translating instruction pointers to file+line, etc. */
	uint16_t ddi_ddiinit; /* Amount of leading DDI instruction bytes that are used for state initialization */

	/* Read generic DDI fields. */
	if (is_8bit_ddi) {
		ddi_strings = UNALIGNED_GETLE16(reader), reader += 2; /* Dec_8BitCodeDDI.cd_strings */
		ddi_ddixdat = UNALIGNED_GETLE16(reader), reader += 2; /* Dec_8BitCodeDDI.cd_ddixdat */
		ddi_ddiaddr = UNALIGNED_GETLE16(reader), reader += 2; /* Dec_8BitCodeDDI.cd_ddiaddr */
		ddi_ddisize = UNALIGNED_GETLE16(reader), reader += 2; /* Dec_8BitCodeDDI.cd_ddisize */
		ddi_ddiinit = UNALIGNED_GETLE8(reader), reader += 1;                    /* Dec_8BitCodeDDI.cd_ddiinit */
	} else {
		ddi_strings = UNALIGNED_GETLE32(reader), reader += 4; /* Dec_CodeDDI.cd_strings */
		ddi_ddixdat = UNALIGNED_GETLE32(reader), reader += 4; /* Dec_CodeDDI.cd_ddixdat */
		ddi_ddiaddr = UNALIGNED_GETLE32(reader), reader += 4; /* Dec_CodeDDI.cd_ddiaddr */
		ddi_ddisize = UNALIGNED_GETLE32(reader), reader += 4; /* Dec_CodeDDI.cd_ddisize */
		ddi_ddiinit = UNALIGNED_GETLE16(reader), reader += 2; /* Dec_CodeDDI.cd_ddiinit */
	}
	ddi_text = self->df_base + ddi_ddiaddr;

	/* Make sure that DDI text is contained entirely within the DEC object file. */
	if ((ddi_text < self->df_base ||
	     ddi_text + ddi_ddisize < ddi_text ||
	     ddi_text + ddi_ddisize > self->df_base + self->df_size) &&
	    ddi_ddisize != 0) {
		GOTO_CORRUPTED(reader, err_currupted);
	}
	result = (DREF DeeDDIObject *)DeeObject_Callocc(offsetof(DeeDDIObject, d_ddi),
	                                                ddi_ddisize + DDI_INSTRLEN_MAX,
	                                                sizeof(uint8_t));
	if unlikely(!result)
		goto err;

	/* Copy DDI text. */
	memcpy(result->d_ddi, ddi_text, ddi_ddisize);
#if DDI_STOP != 0
	memset(result->d_ddi + ddi_ddisize,
	       DDI_STOP, DDI_INSTRLEN_MAX);
#endif /* DDI_STOP != 0 */
	result->d_ddiinit = ddi_ddiinit;
	result->d_ddisize = ddi_ddisize;

	/* Parse the initial DDI register state. */
	result->d_start.dr_flags = UNALIGNED_GETLE16(reader), reader += 2;
	if (result->d_start.dr_flags & ~DDI_REGS_FMASK)
		GOTO_CORRUPTED(reader, err_currupted_r);
	result->d_start.dr_uip  = (code_addr_t)decode_uleb((uint8_t const **)&reader);
	result->d_start.dr_usp  = (uint16_t)decode_uleb((uint8_t const **)&reader);
	result->d_start.dr_path = (uint16_t)decode_uleb((uint8_t const **)&reader);
	result->d_start.dr_file = (uint16_t)decode_uleb((uint8_t const **)&reader);
	result->d_start.dr_name = (uint16_t)decode_uleb((uint8_t const **)&reader);
	result->d_start.dr_col  = (int)decode_sleb((uint8_t const **)&reader);
	result->d_start.dr_lno  = (int)decode_sleb((uint8_t const **)&reader);
	if (ddi_ddixdat) {
		/* Load DDI extension data. */
		uint8_t const *xdat = self->df_base + ddi_ddixdat;
		uint32_t xsiz;
		if (xdat < self->df_base)
			GOTO_CORRUPTED(xdat, err_currupted_r_maps);
		if (xdat >= self->df_base + self->df_size)
			GOTO_CORRUPTED(xdat, err_currupted_r_maps);
		xsiz = UNALIGNED_GETLE16(xdat), xdat += 2;
		if unlikely(xsiz == (uint16_t)-1)
			xsiz = UNALIGNED_GETLE32(xdat), xdat += 4;
		if likely(xsiz != 0) {
			void *bssptr;
			struct Dee_ddi_exdat *xres;
			if (xdat < self->df_base)
				GOTO_CORRUPTED(xdat, err_currupted_r_maps);
			if (xdat + xsiz < xdat)
				GOTO_CORRUPTED(xdat, err_currupted_r_maps);
			if (xdat + xsiz >= self->df_base + self->df_size)
				GOTO_CORRUPTED(xdat, err_currupted_r_maps);
			xres = (struct Dee_ddi_exdat *)Dee_MallococSafe(offsetof(struct Dee_ddi_exdat, dx_data),
			                                                xsiz, DDI_EXDAT_MAXSIZE);
			if unlikely(!xres)
				goto err_r_maps;
			xres->dx_size = xsiz;
			/* Initialize X-data information. */
			bssptr = mempcpy(xres->dx_data, xdat, xsiz);
			bzero(bssptr, DDI_EXDAT_MAXSIZE);
			result->d_exdat = xres;
		}
	}

	/* Load all the DDI string maps. */
	map_error = load_strmap(self, ddi_strings,
	                        &result->d_nstring,
	                        &result->d_strings);
	if (map_error != 0)
		goto handle_map_error;

	/* Use the string table of the DEC file as DDI string table,
	 * thus allowing data-reuse and solving the problem that DEC
	 * files allow DDI strings to be mixed & shared with all the
	 * other strings which we'd have to separate otherwise. */
	result->d_strtab = (DREF DeeStringObject *)DecFile_Strtab(self);
	if unlikely(!result->d_strtab)
		goto err_r_maps;

	Dee_Incref(result->d_strtab);
	DeeObject_Init(result, &DeeDDI_Type);
	return result;
err_r_maps:
	Dee_Free((void *)result->d_strings);
	Dee_Free((void *)result->d_exdat);
/*err_r:*/
	Dee_Free(result);
err:
	return NULL;
err_currupted_r_maps:
	Dee_Free((void *)result->d_strings);
	Dee_Free((void *)result->d_exdat);
err_currupted_r:
	Dee_Free(result);
err_currupted:
	return (DREF DeeDDIObject *)ITER_DONE;
handle_map_error:
	if (map_error > 0)
		goto err_currupted_r_maps;
	goto err_r_maps;
}


/* @return: * :        New reference to a code object.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The DEC file has been corrupted. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeCodeObject *DCALL
DecFile_LoadCode(DecFile *__restrict self,
                 uint8_t const **__restrict p_reader) {
	DREF DeeCodeObject *result;
	Dec_Code header;
	uint8_t const *reader = *p_reader, *end;
	result = (DREF DeeCodeObject *)ITER_DONE;
	end    = self->df_base + self->df_size;
	header.co_flags = UNALIGNED_GETLE16(reader), reader += 2;
	/* Validate known flags. */
	if (header.co_flags & ~(CODE_FMASK | DEC_CODE_F8BIT))
		GOTO_CORRUPTED(reader, corrupt);
	if (header.co_flags & DEC_CODE_F8BIT) {
		if unlikely(reader + sizeof(Dec_8BitCode) - 2 >= end)
			GOTO_CORRUPTED(reader, done); /* Validate bounds. */
		/* Read all the fields and widen them. */
		header.co_localc     = UNALIGNED_GETLE8(reader), reader += 1;
		header.co_refc       = UNALIGNED_GETLE8(reader), reader += 1;
		header.co_argc_min   = UNALIGNED_GETLE8(reader), reader += 1;
		header.co_stackmax   = UNALIGNED_GETLE8(reader), reader += 1;
		header.co_constoff   = UNALIGNED_GETLE16(reader), reader += 2;
		header.co_exceptoff  = UNALIGNED_GETLE16(reader), reader += 2;
		header.co_defaultoff = UNALIGNED_GETLE16(reader), reader += 2;
		header.co_ddioff     = UNALIGNED_GETLE16(reader), reader += 2;
		header.co_kwdoff     = UNALIGNED_GETLE16(reader), reader += 2;
		header.co_textsiz    = UNALIGNED_GETLE16(reader), reader += 2;
		header.co_textoff    = UNALIGNED_GETLE16(reader), reader += 2;
		header.co_staticc = 0;
	} else {
		if unlikely(reader + sizeof(Dec_Code) - 2 >= end)
			GOTO_CORRUPTED(reader, done); /* Validate bounds. */
		memcpy(&header.co_flags + 1, reader, sizeof(Dec_Code) - 2);
		reader += sizeof(Dec_Code) - 2;
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
		header.co_localc     = LETOH16(header.co_localc);
		header.co_refc       = LETOH16(header.co_refc);
		header.co_staticc    = LETOH16(header.co_staticc);
		header.co_argc_min   = LETOH16(header.co_argc_min);
		header.co_stackmax   = LETOH16(header.co_stackmax);
		header.co_constoff   = LETOH32(header.co_constoff);
		header.co_exceptoff  = LETOH32(header.co_exceptoff);
		header.co_defaultoff = LETOH32(header.co_defaultoff);
		header.co_ddioff     = LETOH32(header.co_ddioff);
		header.co_kwdoff     = LETOH32(header.co_kwdoff);
		header.co_textsiz    = LETOH32(header.co_textsiz);
		header.co_textoff    = LETOH32(header.co_textoff);
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
	}

	/* Validate the that text address is in-bounds (it will be read later). */
	if unlikely(header.co_textsiz &&
	            header.co_textoff + header.co_textsiz >= self->df_size)
		GOTO_CORRUPTED(reader, done);

	if (self->df_options &&
	    (self->df_options->co_decloader & Dee_DEC_FUNTRUSTED)) {
		/* The origin of the code cannot be trusted and we must append
		 * a couple of trailing instruction bytes to the code object. */

		/* Allocate the resulting code object, as well as set the CODE_FASSEMBLY flag. */
		header.co_flags |= CODE_FASSEMBLY;
		result = DeeCode_Malloc(header.co_textsiz + INSTRLEN_MAX);
		if likely(result) {
			/* Initialize trailing bytes as `ret none' instructions. */
#if ASM_RET_NONE == 0
			bzero(result->co_code + header.co_textsiz, INSTRLEN_MAX);
#else /* ASM_RET_NONE == 0 */
			memset(result->co_code + header.co_textsiz, ASM_RET_NONE, INSTRLEN_MAX);
#endif /* ASM_RET_NONE != 0 */
		}
	} else {
		/* Allocate the resulting code object. */
		result = (DREF DeeCodeObject *)DeeCode_Malloc(header.co_textsiz);
	}

	/* Check for errors during code allocation. */
	if unlikely(!result)
		goto done;

	/* Fill in argument information and default arguments. */
	result->co_argc_min = header.co_argc_min;
	result->co_argc_max = header.co_argc_min;
	result->co_defaultv = NULL;

	if (header.co_defaultoff) {
		/* Load the vector of default argument objects. */
		uint16_t defaultc;
		DREF DeeObject **defv;
		uint8_t const *def_reader = self->df_base + header.co_defaultoff;
		if unlikely(def_reader >= end)
			GOTO_CORRUPTED(reader, corrupt_r);

		/* Default default object vector. */
		defv = DecFile_LoadObjectVector(self, &defaultc, &def_reader, true);
		if unlikely(!ITER_ISOK(defv)) {
			if (!defv)
				goto err_r;
			goto corrupt_r;
		}
		result->co_defaultv = defv;
		if unlikely(defaultc + result->co_argc_max < defaultc) {
			/* Too many default objects (the counter overflows). */
			Dee_Decrefv(defv, defaultc);
			Dee_Free(defv);
			GOTO_CORRUPTED(reader, corrupt_r);
			__builtin_unreachable();
		}

		/* Add the number of default objects to the argc_max field. */
		result->co_argc_max += defaultc;
	}

	result->co_constc = 0;
	result->co_constv = NULL;
	if (header.co_constoff) {
		uint16_t staticc;
		DREF DeeObject **staticv;
		uint8_t const *sta_reader;
		sta_reader = self->df_base + header.co_constoff;
		if unlikely(sta_reader >= end)
			GOTO_CORRUPTED(reader, corrupt_r_default);

		/* Default object vector. */
		staticv = DecFile_LoadObjectVector(self, &staticc, &sta_reader, false);
		if unlikely(!ITER_ISOK(staticv)) {
			if (!staticv)
				goto err_r_default;
			goto corrupt_r_default;
		}

		/* Save the static object vectors. */
		result->co_constv = staticv;
		result->co_constc = staticc;
	}

	result->co_exceptc = 0;
	result->co_exceptv = NULL;
	if (header.co_exceptoff) {
		/* Read exception descriptors. */
		uint16_t count;
		uint8_t const *except_reader;
		bool is8bit;
		struct except_handler *exceptv;
		except_reader = self->df_base + header.co_exceptoff;
		if unlikely(except_reader >= end) /* Validate bounds */
			GOTO_CORRUPTED(except_reader, corrupt_r_static);
		is8bit = !!(header.co_flags & DEC_CODE_F8BIT);
		if (is8bit) {
			count = UNALIGNED_GETLE8(except_reader);
			except_reader += 1;
		} else {
			count = UNALIGNED_GETLE16(except_reader);
			except_reader += 2;
		}

		/* Allocate the exception vector. */
		exceptv = (struct except_handler *)Dee_Mallocc(count, sizeof(struct except_handler));
		if unlikely(!exceptv)
			goto err_r_static;

		/* Write the exception descriptors to the resulting code object. */
		result->co_exceptv = exceptv;

		/* Load all the exception handlers. */
		for (result->co_exceptc = 0;
		     result->co_exceptc < count; ++result->co_exceptc) {
			struct except_handler *hand;
			hand = exceptv + result->co_exceptc;
			if unlikely(except_reader >= end) /* Validate bounds */
				GOTO_CORRUPTED(except_reader, corrupt_r_except);
			hand->eh_flags = UNALIGNED_GETLE16(except_reader), except_reader += 2; /* Dec_8BitCodeExcept.ce_flags / Dec_CodeExcept.ce_flags */
			if (is8bit) {
				hand->eh_start = UNALIGNED_GETLE16(except_reader), except_reader += 2; /* Dec_8BitCodeExcept.ce_begin */
				hand->eh_end   = UNALIGNED_GETLE16(except_reader), except_reader += 2; /* Dec_8BitCodeExcept.ce_end */
				hand->eh_addr  = UNALIGNED_GETLE16(except_reader), except_reader += 2; /* Dec_8BitCodeExcept.ce_addr */
				hand->eh_stack = UNALIGNED_GETLE8(except_reader), except_reader += 1;  /* Dec_8BitCodeExcept.ce_stack */
			} else {
				hand->eh_start = UNALIGNED_GETLE32(except_reader), except_reader += 4; /* Dec_CodeExcept.ce_begin */
				hand->eh_end   = UNALIGNED_GETLE32(except_reader), except_reader += 4; /* Dec_CodeExcept.ce_end */
				hand->eh_addr  = UNALIGNED_GETLE32(except_reader), except_reader += 4; /* Dec_CodeExcept.ce_addr */
				hand->eh_stack = UNALIGNED_GETLE16(except_reader), except_reader += 2; /* Dec_CodeExcept.ce_stack */
			}

			/* Do some quick validation on the exception descriptor. */
			if (hand->eh_flags & ~EXCEPTION_HANDLER_FMASK)
				GOTO_CORRUPTED(except_reader, corrupt_r_except);
			if (hand->eh_start >= hand->eh_end)
				GOTO_CORRUPTED(except_reader, corrupt_r_except);
			if (hand->eh_end > header.co_textsiz)
				GOTO_CORRUPTED(except_reader, corrupt_r_except);
			if (hand->eh_addr > header.co_textsiz)
				GOTO_CORRUPTED(except_reader, corrupt_r_except);
			if (hand->eh_stack > header.co_stackmax)
				GOTO_CORRUPTED(except_reader, corrupt_r_except);

			/* Read the mask (Dec_8BitCodeExcept.ce_mask / Dec_CodeExcept.ce_mask). */
			if (*except_reader == DTYPE_NULL) {
				hand->eh_mask = NULL;
				except_reader += 1;
			} else {
				hand->eh_mask = (DREF DeeTypeObject *)DecFile_LoadObject(self, &except_reader);
				if unlikely(!ITER_ISOK(hand->eh_mask)) {
					if (!hand->eh_mask)
						goto err_r_except;
					goto corrupt_r_except;
				}

				/* Ensure that the exception mask is a type object */
				if (!DeeType_Check(hand->eh_mask)) {
					Dee_Decref(hand->eh_mask);
					GOTO_CORRUPTED(except_reader, corrupt_r_except);
				}
			}
		}
	}

	/* Load DDI information. */
	if (header.co_ddioff) {
		DREF DeeDDIObject *ddi;
		uint8_t const *ddi_reader;
		ddi_reader = self->df_base + header.co_ddioff;
		if unlikely(ddi_reader >= end || ddi_reader < self->df_base)
			GOTO_CORRUPTED(ddi_reader, corrupt_r_except);
		ddi = DecFile_LoadDDI(self, ddi_reader, !!(header.co_flags & DEC_CODE_F8BIT));
		if unlikely(!ITER_ISOK(ddi)) {
			if (!ddi)
				goto err_r_except;
			goto corrupt_r_except;
		}
		result->co_ddi = ddi; /* Inherit */
	} else {
		result->co_ddi = &DeeDDI_Empty;
		Dee_Incref(&DeeDDI_Empty);
	}

	/* Load keyword information. */
	result->co_keywords = NULL;
	if (header.co_kwdoff && result->co_argc_max != 0) {
		DREF DeeStringObject **kwds;
		uint16_t i;
		uint32_t string_size       = LETOH32(self->df_ehdr->e_strsiz);
		char const *strtab         = (char const *)(self->df_base + LETOH32(self->df_ehdr->e_stroff));
		uint8_t const *kwd_reader  = self->df_base + header.co_kwdoff;
		if unlikely(kwd_reader >= end || kwd_reader < self->df_base)
			GOTO_CORRUPTED(kwd_reader, corrupt_r_ddi);
		kwds = (DREF DeeStringObject **)Dee_Mallocc(result->co_argc_max,
		                                            sizeof(DREF DeeStringObject *));
		if unlikely(!kwds)
			goto err_r_ddi;
		if (header.co_flags & DEC_CODE_F8BIT) {
			for (i = 0; i < result->co_argc_max; ++i) {
				uint32_t addr;
				char const *name;
				if unlikely(kwd_reader >= end) {
corrupt_kwds_i:
					Dee_Decrefv(kwds, i);
					Dee_Free(kwds);
					GOTO_CORRUPTED(kwd_reader, corrupt_r_ddi);
				}
				addr = Dec_DecodePointer((uint8_t const **)&kwd_reader);
				if unlikely(addr >= string_size)
					goto corrupt_kwds_i;
				name = strtab + addr;
				if ((kwds[i] = (DREF DeeStringObject *)DeeString_NewUtf8(name, strlen(name),
				                                                         STRING_ERROR_FSTRICT)) == NULL) {
err_kwds_i:
					Dee_Decrefv(kwds, i);
					Dee_Free(kwds);
					goto err_r_ddi;
				}
			}
		} else {
			for (i = 0; i < result->co_argc_max; ++i) {
				uint32_t addr, size;
				DREF DeeStringObject *kwd;
				if unlikely(kwd_reader >= end)
					goto corrupt_kwds_i;
				size = Dec_DecodePointer((uint8_t const **)&kwd_reader);
				if (!size) {
					kwds[i] = (DREF DeeStringObject *)DeeString_NewEmpty();
					continue;
				}
				addr = Dec_DecodePointer((uint8_t const **)&kwd_reader);
				if unlikely(addr >= string_size)
					goto corrupt_kwds_i;
				if unlikely((uint32_t)(addr + size) < addr)
					goto corrupt_kwds_i;
				if unlikely((uint32_t)(addr + size) > string_size)
					goto corrupt_kwds_i;
				kwd = (DREF DeeStringObject *)DeeString_NewUtf8(strtab + addr, size,
				                                                STRING_ERROR_FSTRICT);
				if unlikely(!kwd)
					goto err_kwds_i;
				kwds[i] = kwd;
			}
		}
		result->co_keywords = kwds;
	}

	/* Load the code's text assembly from the file.
	 * NOTE: The existence of the text segment has already been checked above. */
#if 0 /* Hide the fact that we've extended the text segment by a couple of `ret none' instructions. \
       * Without this, code would be able to determine that it was                                  \
       * loaded in untrusted mode by examining its own code object. */
	result->co_codebytes = header.co_textsiz + INSTRLEN_MAX;
#else
	result->co_codebytes = header.co_textsiz;
#endif
	memcpyc(result->co_code,
	        self->df_base + header.co_textoff,
	        header.co_textsiz,
	        sizeof(instruction_t));

	/* Fill in remaining, basic fields of the resulting code object. */
	result->co_flags      = header.co_flags;
	result->co_localc     = header.co_localc;
	result->co_refc       = header.co_refc;
	result->co_refstaticc = header.co_refc + header.co_staticc;

	/* Calculate the size of the required execution frame. */
	result->co_framesize = ((uint32_t)header.co_localc +
	                        (uint32_t)header.co_stackmax) *
	                       sizeof(DREF DeeObject *);

	/* Forceably set the heapframe flag when the frame is very large. */
	if unlikely(result->co_framesize >= CODE_LARGEFRAME_THRESHOLD)
		result->co_flags |= CODE_FHEAPFRAME;

	/* Fill in module information for the code object. */
	result->co_module = self->df_module;
	Dee_Incref(self->df_module);
#ifdef CONFIG_HAVE_CODE_METRICS
	Dee_code_metrics_init(&result->co_metrics);
#endif /* CONFIG_HAVE_CODE_METRICS */
#ifdef CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE
	Dee_hostasm_code_init(&result->co_hostasm);
#endif /* CONFIG_HAVE_HOSTASM_AUTO_RECOMPILE */

	/* Finally, initialize the resulting code object and start tracking it. */
	DeeObject_Init(result, &DeeCode_Type);
done:
	*p_reader = reader;
	return result;
err_r_ddi:
	Dee_Decref(result->co_ddi);
err_r_except:
	while (result->co_exceptc) {
		--result->co_exceptc;
		Dee_XClear(result->co_exceptv[result->co_exceptc].eh_mask);
	}
	Dee_Free(result->co_exceptv);
err_r_static:
	Dee_Decrefv(result->co_constv, result->co_constc);
	Dee_Free((void *)result->co_constv);
err_r_default:
	/* Destroy default objects. */
	ASSERT(result->co_argc_max >= result->co_argc_min);
	Dee_XDecrefv(result->co_defaultv, result->co_argc_max -
	                                  result->co_argc_min);
	Dee_Free((void *)result->co_defaultv);
err_r:
	DeeGCObject_Free(result);
	result = NULL;
	goto done;
corrupt_r_ddi:
	Dee_Decref(result->co_ddi);
corrupt_r_except:
	while (result->co_exceptc) {
		--result->co_exceptc;
		Dee_XDecref(result->co_exceptv[result->co_exceptc].eh_mask);
	}
	Dee_Free(result->co_exceptv);
corrupt_r_static:
	Dee_Decrefv(result->co_constv, result->co_constc);
	Dee_Free((void *)result->co_constv);
corrupt_r_default:
	/* Destroy default objects. */
	ASSERT(result->co_argc_max >= result->co_argc_min);
	Dee_XDecrefv(result->co_defaultv, result->co_argc_max - result->co_argc_min);
	Dee_Free((void *)result->co_defaultv);
corrupt_r:
	DeeGCObject_Free(result);
corrupt:
	result = (DREF DeeCodeObject *)ITER_DONE;
	goto done;
}

INTDEF struct module_symbol empty_module_buckets[];

/* Load a given DEC file and fill in the given `module'.
 * @return:  0: Successfully loaded the given DEC file.
 * @return:  1: The DEC file has been corrupted or is out of date.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DecFile_Load(DecFile *__restrict self) {
	DeeModuleObject *module;
	int result;
	module = self->df_module;

	/* Load the module import table and all collect all dependency modules. */
	result = DecFile_LoadImports(self);
	if (result != 0)
		goto err;

	/* Load global variables related to this module. */
	result = DecFile_LoadGlobals(self);
	if (result != 0)
		goto err;

	{
		uint8_t const *root_reader;
		DREF DeeCodeObject *root_code;
		root_reader = self->df_base + self->df_ehdr->e_rootoff;
		/* Read the root code object. */
		root_code = DecFile_LoadCode(self, &root_reader);
		if unlikely(!ITER_ISOK(root_code)) {
			result = root_code ? 1 : -1;
			goto err;
		}
		module->mo_root = root_code; /* Inherit. */
	}

	return 0;
err:
	{
		DREF DeeCodeObject *root;
		root            = module->mo_root;
		module->mo_root = NULL;
		Dee_XDecref(root);
	}

	/* Free the module's global variable vector.
	 * NOTE: At this point, we can still assume that it was filled with all NULLs. */
	ASSERT((module->mo_globalv != NULL) ==
	       (module->mo_globalc != 0));
	Dee_Free(module->mo_globalv);
	module->mo_globalc = 0;
	module->mo_globalv = NULL;

	/* Free the module's symbol table. */
	ASSERT((module->mo_bucketv != empty_module_buckets) ==
	       (module->mo_bucketm != 0));
	if (module->mo_bucketm) {
		do {
			if (module->mo_bucketv[module->mo_bucketm].ss_name) {
				if (module->mo_bucketv[module->mo_bucketm].ss_flags & MODSYM_FNAMEOBJ)
					Dee_Decref(COMPILER_CONTAINER_OF(module->mo_bucketv[module->mo_bucketm].ss_name, DeeStringObject, s_str));
				if (module->mo_bucketv[module->mo_bucketm].ss_flags & MODSYM_FDOCOBJ)
					Dee_Decref(COMPILER_CONTAINER_OF(module->mo_bucketv[module->mo_bucketm].ss_doc, DeeStringObject, s_str));
			}
		} while (module->mo_bucketm--);
		Dee_Free(module->mo_bucketv);
		module->mo_bucketm = 0;
		module->mo_bucketv = empty_module_buckets;
	}

	/* Free the module's import table. */
	ASSERT((module->mo_importv != NULL) ==
	       (module->mo_importc != 0));
	Dee_Decrefv(module->mo_importv, module->mo_importc);
	Dee_Free((void *)module->mo_importv);
	module->mo_importv = NULL;
	return result;
}

/* @return:  0: Successfully loaded the given DEC file.
 * @return:  1: The DEC file was out of date or had been corrupted.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_OpenDec(DeeModuleObject *__restrict mod,
                  DeeObject *__restrict input_stream,
                  struct compiler_options *options) {
	DecFile file;
	struct DeeMapFile filemap;
	int result;
	ASSERT(mod->mo_path);

	/* Initialize the file */
	if unlikely(DeeMapFile_InitFile(&filemap, input_stream,
	                                0, 0, DFILE_LIMIT + 1, DECFILE_PADDING,
	                                DEE_MAPFILE_F_READALL | DEE_MAPFILE_F_ATSTART))
		return -1;
	result = DecFile_Init(&file, &filemap, mod, mod->mo_path, options);
	if unlikely(result != 0)
		goto done_map;
	Dee_DPRINTF("[LD] Opened dec file for %r\n", file.df_name);

	/* Check if the file is up-to-date (unless this check is being suppressed). */
	if (!options || !(options->co_decloader & Dee_DEC_FLOADOUTDATED)) {
		result = DecFile_IsUpToDate(&file);
		if (result != 0) {
			Dee_DPRINTF("[LD] Dec file for %r is out-of-date\n", file.df_name);
			goto done_map_file;
		}
	}

	/* With all that out of the way, actually load the file. */
	result = DecFile_Load(&file);
#ifndef Dee_DPRINT_IS_NOOP
	if unlikely(result > 0)
		Dee_DPRINTF("[LD] Dec file for %r is corrupted\n", file.df_name);
#endif /* !Dee_DPRINT_IS_NOOP */
done_map_file:
	DecFile_Fini(&file);
done_map:
	DeeMapFile_Fini(&filemap);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) uint64_t DCALL
DeeModule_GetCTime(/*Module*/ DeeObject *__restrict self) {
	uint64_t result;
	DeeModuleObject *me = (DeeModuleObject *)self;
	ASSERT_OBJECT_TYPE(me, &DeeModule_Type);
	if (me->mo_flags & Dee_MODULE_FHASCTIME) {
		result = me->mo_ctime;
		ASSERT(result != (uint64_t)-1);
	} else if (me == &DeeModule_Deemon) {
		/* `DeeExec_GetTimestamp()' already uses the `mo_ctime' field
		 *  of `DeeModule_Deemon' as cache if that field is available. */
		result = DeeExec_GetTimestamp();
	} else {
		/* Lookup the last-modified time of the module's path file. */
		DeeStringObject *path = me->mo_path;
		if unlikely(!path) {
			result = 0;
		} else {
			result = DecTime_Lookup((DeeObject *)path);
			if unlikely(result == (uint64_t)-1)
				goto done;
		}
		/* Cache the result value in the module itself. */
		me->mo_ctime = result;
		atomic_or(&me->mo_flags, Dee_MODULE_FHASCTIME);
	}
done:
	return result;
}


struct mtime_entry {
	DREF DeeStringObject *me_file; /* [0..1] Absolute, normalized filename.
	                                *  NOTE: When NULL, then this entry is unused. */
#ifdef DEE_SYSTEM_FS_ICASE
	Dee_hash_t               me_casehash; /* Case-insensitive hash for `me_file' */
#endif /* DEE_SYSTEM_FS_ICASE */
	uint64_t              me_mtim; /* Last-modified time of `me_file' */
};

#ifdef DEE_SYSTEM_FS_ICASE
#define MTIME_ENTRY_HASH(x) ((x)->me_casehash)
#else /* DEE_SYSTEM_FS_ICASE */
#define MTIME_ENTRY_HASH(x) DeeString_HASH((x)->me_file)
#endif /* !DEE_SYSTEM_FS_ICASE */

struct mtime_cache {
	size_t              mc_size; /* [lock(mc_lock)] Amount of cache entries currently in use. */
	size_t              mc_mask; /* [lock(mc_lock)] Allocated hash-vector size -1 / hash-mask. */
	struct mtime_entry *mc_list; /* [1..mc_mask+1][lock(mc_lock)]
	                              * [owned_if(!= empty_mtime_items)]
	                              * Filename -> last-modified mappings. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t mc_lock; /* Lock for synchronizing access to the cache. */
#endif /* !CONFIG_NO_THREADS */
};

#define MCACHE_HASHST(hash)        ((hash) & mtime_cache.mc_mask)
#define MCACHE_HASHNX(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define MCACHE_HASHIT(i)           (mtime_cache.mc_list + ((i) & mtime_cache.mc_mask))

PRIVATE struct mtime_entry empty_mtime_items[1] = { { NULL, 0 } };

PRIVATE struct mtime_cache mtime_cache = {
	/* .mc_size = */ 0,
	/* .mc_mask = */ 0,
	/* .mc_list = */ empty_mtime_items
#ifndef CONFIG_NO_THREADS
	,
	/* .mc_lock = */ Dee_ATOMIC_RWLOCK_INIT
#endif /* !CONFIG_NO_THREADS */
};

#define mtime_cache_lock_reading()    Dee_atomic_rwlock_reading(&mtime_cache.mc_lock)
#define mtime_cache_lock_writing()    Dee_atomic_rwlock_writing(&mtime_cache.mc_lock)
#define mtime_cache_lock_tryread()    Dee_atomic_rwlock_tryread(&mtime_cache.mc_lock)
#define mtime_cache_lock_trywrite()   Dee_atomic_rwlock_trywrite(&mtime_cache.mc_lock)
#define mtime_cache_lock_canread()    Dee_atomic_rwlock_canread(&mtime_cache.mc_lock)
#define mtime_cache_lock_canwrite()   Dee_atomic_rwlock_canwrite(&mtime_cache.mc_lock)
#define mtime_cache_lock_waitread()   Dee_atomic_rwlock_waitread(&mtime_cache.mc_lock)
#define mtime_cache_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&mtime_cache.mc_lock)
#define mtime_cache_lock_read()       Dee_atomic_rwlock_read(&mtime_cache.mc_lock)
#define mtime_cache_lock_write()      Dee_atomic_rwlock_write(&mtime_cache.mc_lock)
#define mtime_cache_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&mtime_cache.mc_lock)
#define mtime_cache_lock_upgrade()    Dee_atomic_rwlock_upgrade(&mtime_cache.mc_lock)
#define mtime_cache_lock_downgrade()  Dee_atomic_rwlock_downgrade(&mtime_cache.mc_lock)
#define mtime_cache_lock_endwrite()   Dee_atomic_rwlock_endwrite(&mtime_cache.mc_lock)
#define mtime_cache_lock_endread()    Dee_atomic_rwlock_endread(&mtime_cache.mc_lock)
#define mtime_cache_lock_end()        Dee_atomic_rwlock_end(&mtime_cache.mc_lock)


/* Try to free up memory from the dec time-cache. */
INTERN size_t DCALL
DecTime_ClearCache(size_t UNUSED(max_clear)) {
	size_t result, old_mask;
	struct mtime_entry *old_list;
	mtime_cache_lock_write();
	ASSERT((mtime_cache.mc_mask == 0) ==
	       (mtime_cache.mc_list == empty_mtime_items));
	old_mask            = mtime_cache.mc_mask;
	old_list            = mtime_cache.mc_list;
	mtime_cache.mc_size = 0;
	mtime_cache.mc_mask = 0;
	mtime_cache.mc_list = empty_mtime_items;
	mtime_cache_lock_endwrite();

	/* Check for special case: no mask was allocated. */
	if (!old_mask)
		return 0;

	/* Figure out how much memory will be released. */
	result = (old_mask + 1) * sizeof(struct mtime_entry);

	/* Clear the old hash-map. */
	for (;;) {
		Dee_XDecref(old_list[old_mask].me_file);
		if (!old_mask)
			break;
		--old_mask;
	}

	/* Free the map descriptor. */
	Dee_Free(old_list);
	return result;
}


PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
mtime_cache_lookup(DeeObject *__restrict path,
                   uint64_t *__restrict p_result) {
	bool result = false;
	Dee_hash_t i, perturb;
	Dee_hash_t hash = fs_hashobj(path);
	mtime_cache_lock_read();
	perturb = i = MCACHE_HASHST(hash);
	for (;; MCACHE_HASHNX(i, perturb)) {
		struct mtime_entry *item = MCACHE_HASHIT(i);
		if (!item->me_file)
			break; /* Entry not found. */
		if (MTIME_ENTRY_HASH(item) != hash)
			continue; /* Differing hashes. */
		if (!DeeString_EqualsSTR(item->me_file, path))
			continue; /* Differing strings. */
		/* Found it! */
		*p_result = item->me_mtim;
		result   = true;
		break;
	}
	mtime_cache_lock_endread();
	return result;
}


/* Rehash the mtime cache. */
PRIVATE bool DCALL
mtime_cache_rehash(void) {
	struct mtime_entry *new_vector, *iter, *end;
	size_t new_mask = mtime_cache.mc_mask;
	new_mask        = (new_mask << 1) | 1;
	if unlikely(new_mask == 1)
		new_mask = 64 - 1; /* Start out bigger than 2. */
	ASSERT(mtime_cache.mc_size < new_mask);
	new_vector = (struct mtime_entry *)Dee_TryCallocc(new_mask + 1, sizeof(struct mtime_entry));
	if unlikely(!new_vector)
		return false;
	ASSERT((mtime_cache.mc_list == empty_mtime_items) == (mtime_cache.mc_size == 0));
	ASSERT((mtime_cache.mc_list == empty_mtime_items) == (mtime_cache.mc_mask == 0));
	if (mtime_cache.mc_list != empty_mtime_items) {
		/* Re-insert all existing items into the new table vector. */
		end = (iter = mtime_cache.mc_list) + (mtime_cache.mc_mask + 1);
		for (; iter < end; ++iter) {
			struct mtime_entry *item;
			Dee_hash_t i, perturb;

			/* Skip NULL entries. */
			if (!iter->me_file)
				continue;
			perturb = i = MTIME_ENTRY_HASH(iter) & new_mask;
			for (;; MCACHE_HASHNX(i, perturb)) {
				item = &new_vector[i & new_mask];
				if (!item->me_file)
					break; /* Empty slot found. */
			}

			/* Transfer this object. */
			memcpy(item, iter, sizeof(struct mtime_entry));
		}
		Dee_Free(mtime_cache.mc_list);
	}
	mtime_cache.mc_mask = new_mask;
	mtime_cache.mc_list = new_vector;
	return true;
}


PRIVATE NONNULL((1)) void DCALL
mtime_cache_insert(DeeObject *__restrict path,
                   uint64_t value) {
	size_t mask;
	Dee_hash_t i, perturb, hash;
	hash = fs_hashobj(path);
	mtime_cache_lock_write();
again:
	mask    = mtime_cache.mc_mask;
	perturb = i = hash & mask;
	for (;; MCACHE_HASHNX(i, perturb)) {
		struct mtime_entry *item = &mtime_cache.mc_list[i & mask];
		if (!item->me_file) {
			if (mtime_cache.mc_size + 1 >= mtime_cache.mc_mask)
				break; /* Rehash the table and try again. */

			/* Not found. - Use this empty slot. */
			item->me_file = (DREF DeeStringObject *)path;
#ifdef DEE_SYSTEM_FS_ICASE
			item->me_casehash = hash;
#endif /* DEE_SYSTEM_FS_ICASE */
			item->me_mtim = value;
			Dee_Incref(path);
			++mtime_cache.mc_size;

			/* Try to keep the table vector big at least twice as big as the element count. */
			if (mtime_cache.mc_size * 2 > mtime_cache.mc_mask)
				mtime_cache_rehash();
			goto done;
		}
		if (MTIME_ENTRY_HASH(item) != hash)
			continue; /* Non-matching hash */
		if (!DeeString_EqualsSTR(item->me_file, path))
			continue; /* Differing strings. */

		/* The item already exists. (Can happen due to race conditions) */
		goto done;
	}

	/* Rehash the table and try again. */
	if (mtime_cache_rehash())
		goto again;
done:
	mtime_cache_lock_endwrite();
}



/* Return the last-modified time (in microseconds since 01-01-1970).
 * For this purpose, an internal cache is kept that is consulted
 * before and populated after making an attempt at contacting the
 * host operating system for the required information.
 * @return: * :           Last-modified time (in microseconds since 01-01-1970).
 * @return: 0 :           The given file could not be found.
 * @return: (uint64_t)-1: The lookup failed and an error was thrown. */
PRIVATE WUNUSED NONNULL((1)) uint64_t DCALL
DecTime_Lookup(DeeObject *__restrict filename) {
	uint64_t result;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);

	/* Ensure that we are using an absolute, fixed path. */
	filename = DeeSystem_MakeAbsolute(filename);
	if unlikely(!filename)
		return (uint64_t)-1;

	/* Consult the cache before asking the OS. */
	if (!mtime_cache_lookup(filename, &result)) {
		result = DeeSystem_GetLastModified(filename);
		/* Add the new information to the cache. */
		if likely(result != (uint64_t)-1)
			mtime_cache_insert(filename, result);
	}
	Dee_Decref(filename);
#if 0
	if (result != (uint64_t)-1)
		result = (uint64_t)-2;
#endif
	return result;
}


DECL_END
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
#endif /* !CONFIG_NO_DEC */

#endif /* !GUARD_DEEMON_EXECUTE_DEC_C */
