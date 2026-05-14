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
#ifndef GUARD_DEEMON_EXECUTE_DEC_C
#define GUARD_DEEMON_EXECUTE_DEC_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>            /* DeeSlab_Free, Dee_*alloc*, Dee_BadAlloc, Dee_Free, Dee_Memalign */
#include <deemon/bool.h>             /* DeeBool_Check, DeeBool_IsTrue, Dee_CONFIG_BOOL_TLS, Dee_FalseTrue */
#include <deemon/dec.h>              /* DECMAG*, DFILE_LIMIT, DI_MAG*, DVERSION_CUR, Dec_*, DeeDecWriter, DeeDecWriter_*, DeeDec_Ehdr, DeeDec_Ehdr_*, Dee_ALIGNOF_DEC_*, Dee_DEC_ENDIAN, Dee_DEC_MACH, Dee_DEC_TYPE_IMAGE, Dee_DEC_TYPE_RELOC, Dee_dec_* */
#include <deemon/error-rt.h>         /* DeeRT_ErrCannotSerialize */
#include <deemon/error.h>            /* DeeError_* */
#include <deemon/format.h>           /* PRF* */
#include <deemon/gc.h>               /* DeeGCSlab_Free, DeeGC_Head, Dee_GC_OBJECT_OFFSET, Dee_gc_head */
#include <deemon/heap.h>             /* DeeHeap_GetRegionOf, Dee_HEAPCHUNK_*, Dee_heapchunk, Dee_heapregion, Dee_heaptail */
#include <deemon/mapfile.h>          /* DeeMapFile*, Dee_SIZEOF_DeeMapFile */
#include <deemon/module.h>           /* DeeModule*, Dee_MODULE_FHASBUILDID, Dee_MODULE_FNOSERIAL, Dee_compiler_options, Dee_module_buildid, Dee_module_object */
#include <deemon/object.h>           /* ASSERT_OBJECT, ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, DeeTypeObject, Dee_AsObject, Dee_Decref*, Dee_Incref, Dee_IncrefIfNotZero, Dee_OBJECT_OFFSETOF_DATA, Dee_TYPE, Dee_funptr_t, Dee_uint128_t, ITER_DONE, ITER_ISOK */
#include <deemon/serial.h>           /* DeeSerial, Dee_SERADDR_INVALID, Dee_SERADDR_ISOK, Dee_seraddr_t, Dee_serial_type */
#include <deemon/string.h>           /* DeeStringObject, DeeString_AsUtf8, WSTR_LENGTH */
#include <deemon/system-features.h>  /* bzero, link, memcpy, memmoveupc, mempcpyc, memset */
#include <deemon/system.h>           /* DeeSystem_GetWalltime */
#include <deemon/type.h>             /* DeeType_* */
#include <deemon/util/atomic.h>      /* atomic_read */
#include <deemon/util/lock.h>        /* Dee_atomic_lock_* */
#include <deemon/util/md5.h>         /* DeeMD5_* */
#include <deemon/util/slab-config.h> /* Dee_SLAB_* */
#include <deemon/util/slab.h>        /* Dee_SLAB_PAGESIZE, Dee_SLAB_PAGE_META_CUSTOM_MARKER, Dee_slab_page, Dee_slab_page_buildinit, Dee_slab_page_buildmalloc */
#include <deemon/util/weakref.h>     /* Dee_weakref, Dee_weakref_callback_t, Dee_weakref_initempty */

#include <hybrid/align.h>            /* CEIL_ALIGN, IS_ALIGNED, IS_POWER_OF_TWO */
#include <hybrid/limitcore.h>        /* __UINT32_MAX__ */
#include <hybrid/overflow.h>         /* OVERFLOW_UADD */
#include <hybrid/sequence/bsearch.h> /* BSEARCH */
#include <hybrid/typecore.h>         /* __BYTE_TYPE__, __SIZEOF_POINTER__ */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, ptrdiff_t, size_t */
#include <stdint.h>  /* uint64_t, uintptr_t */

#undef byte_t
#define byte_t __BYTE_TYPE__
#undef container_of
#define container_of COMPILER_CONTAINER_OF
#undef offsetafter
#define offsetafter COMPILER_OFFSETAFTER

DECL_BEGIN

#define ASSERT_FIELD(T, field, offset, size)       \
	STATIC_ASSERT(offsetof(T, field) == (offset)); \
	STATIC_ASSERT(sizeof(((T *)0)->field) == (size));
ASSERT_FIELD(Dec_Ehdr, e_ident, DeeDec_Ehdr_OFFSETOF__e_ident, 4);
ASSERT_FIELD(Dec_Ehdr, e_mach, DeeDec_Ehdr_OFFSETOF__e_mach, 1);
ASSERT_FIELD(Dec_Ehdr, e_type, DeeDec_Ehdr_OFFSETOF__e_type, 1);
ASSERT_FIELD(Dec_Ehdr, e_version, DeeDec_Ehdr_OFFSETOF__e_version, 2);
STATIC_ASSERT(offsetof(Dec_Ehdr, e_typedata) == DeeDec_Ehdr_OFFSETOF__e_typedata);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_offsetof_heap, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_heap, 2);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_sizeof_pointer, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_sizeof_pointer, 1);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_endian, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_endian, 1);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_offsetof_eof, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_eof, 4);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_deemon_build_id, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_deemon_build_id, 16);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_build_timestamp, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_build_timestamp, 8);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_offsetof_gchead, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_gchead, 4);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_offsetof_gctail, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_gctail, 4);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_offsetof_srel, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_srel, 4);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_offsetof_drel, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_drel, 4);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_offsetof_drrel, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_drrel, 4);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_offsetof_drrela, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_drrela, 4);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_offsetof_deps, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_deps, 4);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_offsetof_files, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_files, 4);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_offsetof_xrel, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_xrel, 4);
ASSERT_FIELD(Dec_Ehdr, e_typedata.td_reloc.er_alignment, DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_alignment, 4);
ASSERT_FIELD(Dec_Ehdr, e_mapping, DeeDec_Ehdr_OFFSETOF__e_mapping, Dee_SIZEOF_DeeMapFile);
ASSERT_FIELD(Dec_Ehdr, e_heap, DeeDec_Ehdr_OFFSETOF__e_heap, sizeof(struct Dee_heapregion));
STATIC_ASSERT(IS_ALIGNED(DeeDec_Ehdr_OFFSETOF__e_heap, Dee_HEAPCHUNK_ALIGN));

ASSERT_FIELD(Dec_Rel, r_addr, 0, 4);
STATIC_ASSERT(sizeof(Dec_Rel) == 4);

ASSERT_FIELD(Dec_RRel, r_addr, 0, 4);
STATIC_ASSERT(sizeof(Dec_RRel) == 4);

ASSERT_FIELD(Dec_RRela, r_addr, 0, 4);
ASSERT_FIELD(Dec_RRela, r_offs, 4, 4);
STATIC_ASSERT(sizeof(Dec_RRela) == 8);

ASSERT_FIELD(Dec_Dhdr, d_modspec.d_file.d_offsetof_modname, 0, 4);
ASSERT_FIELD(Dec_Dhdr, d_modspec.d_file.d_offsetof_rel, 4, 4);
ASSERT_FIELD(Dec_Dhdr, d_offsetof_rrel, 8, 4);
ASSERT_FIELD(Dec_Dhdr, d_offsetof_rrela, 12, 4);
ASSERT_FIELD(Dec_Dhdr, d_buildid, 16, 16);
STATIC_ASSERT(sizeof(Dec_Dhdr) == 32);
#undef ASSERT_FIELD


#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

/* Destructor linked into `struct Dee_heapregion' for dec file mappings. */
INTERN NONNULL((1)) void DCALL
DeeDec_heapregion_destroy(struct Dee_heapregion *__restrict self) {
	Dec_Ehdr *ehdr = container_of(self, Dec_Ehdr, e_heap);
	DeeDec_Ehdr_Destroy(ehdr);
}

/************************************************************************/
/************************************************************************/
/*                                                                      */
/* DEC FILE LOADING / RELOCATION                                        */
/*                                                                      */
/************************************************************************/
/************************************************************************/

#ifndef UINT32_MAX
#define UINT32_MAX __UINT32_MAX__
#endif /* !UINT32_MAX */

#if __SIZEOF_POINTER__ <= 4
#define seraddr32(seraddr) ((Dee_dec_addr32_t)(seraddr))
#else /* __SIZEOF_POINTER__ <= 4 */
#define seraddr32(seraddr) (ASSERT((seraddr) <= UINT32_MAX), (Dee_dec_addr32_t)(seraddr))
#endif /* __SIZEOF_POINTER__ > 4 */



PRIVATE NONNULL((1)) void DCALL
apply_rel(DeeDec_Ehdr *__restrict self,
          Dee_dec_addr32_t offsetof_reltab,
          uintptr_t relbase) {
	Dee_dec_addr32_t reladdr;
	Dec_Rel const *rel = (Dec_Rel const *)((byte_t *)self + offsetof_reltab);
	for (; (reladdr = rel->r_addr) != 0; ++rel) {
		byte_t **p_pointer = (byte_t **)((byte_t *)self + reladdr);
		*p_pointer += relbase;
	}
}

/* @return: * :   Pointer to already-destroyed object
 * @return: NULL: Success */
PRIVATE WUNUSED NONNULL((1)) DeeObject *DCALL
apply_rrel_incref(DeeDec_Ehdr *__restrict self,
                  Dee_dec_addr32_t offsetof_reltab,
                  uintptr_t relbase) {
	Dee_dec_addr32_t reladdr;
	Dec_RRel const *rel = (Dec_RRel const *)((byte_t *)self + offsetof_reltab);
	for (; (reladdr = rel->r_addr) != 0; ++rel) {
		byte_t **p_pointer = (byte_t **)((byte_t *)self + reladdr);
		DeeObject *obj = (DeeObject *)(*p_pointer += relbase);
		/* In this case, "obj" is always a statically allocated object within some
		 * other module. However, if that "other module" is a DEE module, there is
		 * a possibility that "obj" has since been destroyed. Granted: this should
		 * not happen, since the compiler should only serialize objects that cannot
		 * be destroyed before the associated module itself gets destroyed. I.e.:
		 * Objects from "global final" slots of the module, and any nested objects
		 * reachable from there-on that are referenced via [const] relations.
		 *
		 * However, if "obj" is already dead, then we have to roll-back everything
		 * and refuse to load the DEX module.
		 *
		 * This edge-case is also the reason for the special extension to dlmalloc:
		 * FLAG4_BIT_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE
		 *
		 * Thanks to that extension, dlfree() will never overwrite fields at offsets
		 * - offsetof(DeeObject, ob_refcnt)
		 * - offsetof(struct Dee_gc_head, gc_object.ob_refcnt)
		 * of whatever heap-pointer it is given, meaning that all regular- and GC-
		 * objects will still be detectable as already-destroyed, even after that
		 * has happened (but the associated module hasn't been destroyed, yet). */
		if unlikely(!Dee_IncrefIfNotZero(obj)) {
			Dec_RRel const *base = (Dec_RRel const *)((byte_t *)self + offsetof_reltab);
			while (--rel >= base) {
				DREF DeeObject *sub_obj;
				p_pointer = (byte_t **)((byte_t *)self + rel->r_addr);
				sub_obj = (DeeObject *)*p_pointer;
				ASSERT_OBJECT(sub_obj);
				Dee_Decref_unlikely(sub_obj);
			}
			return obj;
		}
		ASSERT_OBJECT(obj);
	}
	return NULL;
}

/* @return: * :   Pointer to already-destroyed object
 * @return: NULL: Success */
PRIVATE WUNUSED NONNULL((1)) DeeObject *DCALL
apply_rrela_incref(DeeDec_Ehdr *__restrict self,
                   Dee_dec_addr32_t offsetof_reltab,
                   uintptr_t relbase) {
	Dee_dec_addr32_t reladdr;
	Dec_RRela const *rel = (Dec_RRela const *)((byte_t *)self + offsetof_reltab);
	for (; (reladdr = rel->r_addr) != 0; ++rel) {
		byte_t **p_pointer = (byte_t **)((byte_t *)self + reladdr);
		byte_t *pointer = (*p_pointer += relbase);
		DeeObject *obj = (DeeObject *)(pointer + (ptrdiff_t)rel->r_offs);
		if unlikely(!Dee_IncrefIfNotZero(obj)) {
			Dec_RRela const *base = (Dec_RRela const *)((byte_t *)self + offsetof_reltab);
			while (--rel >= base) {
				DREF DeeObject *sub_obj;
				p_pointer = (byte_t **)((byte_t *)self + rel->r_addr);
				sub_obj = (DeeObject *)(*p_pointer + (ptrdiff_t)rel->r_offs);
				ASSERT_OBJECT(sub_obj);
				Dee_Decref_unlikely(sub_obj);
			}
			return obj;
		}
		ASSERT_OBJECT(obj);
	}
	return NULL;
}

/* Only use this one for rrel against the deemon core, and DEX modules */
PRIVATE NONNULL((1)) void DCALL
apply_rrel_incref_nofail(DeeDec_Ehdr *__restrict self,
                         Dee_dec_addr32_t offsetof_reltab,
                         uintptr_t relbase) {
	Dee_dec_addr32_t reladdr;
	Dec_RRel const *rel = (Dec_RRel const *)((byte_t *)self + offsetof_reltab);
	for (; (reladdr = rel->r_addr) != 0; ++rel) {
		byte_t **p_pointer = (byte_t **)((byte_t *)self + reladdr);
		DeeObject *obj = (DeeObject *)(*p_pointer += relbase);
		ASSERT_OBJECT(obj);
		Dee_Incref(obj);
	}
}

/* Only use this one for rrel against the deemon core, and DEX modules */
PRIVATE NONNULL((1)) void DCALL
apply_rrela_incref_nofail(DeeDec_Ehdr *__restrict self,
                          Dee_dec_addr32_t offsetof_reltab,
                          uintptr_t relbase) {
	Dee_dec_addr32_t reladdr;
	Dec_RRela const *rel = (Dec_RRela const *)((byte_t *)self + offsetof_reltab);
	for (; (reladdr = rel->r_addr) != 0; ++rel) {
		byte_t **p_pointer = (byte_t **)((byte_t *)self + reladdr);
		byte_t *pointer = (*p_pointer += relbase);
		DeeObject *obj = (DeeObject *)(pointer + (ptrdiff_t)rel->r_offs);
		ASSERT_OBJECT(obj);
		Dee_Incref(obj);
	}
}

PRIVATE NONNULL((1)) void DCALL
applied_rrel_decref(DeeDec_Ehdr *__restrict self,
                    Dee_dec_addr32_t offsetof_reltab) {
	Dee_dec_addr32_t reladdr;
	Dec_RRel const *rel = (Dec_RRel const *)((byte_t *)self + offsetof_reltab);
	for (; (reladdr = rel->r_addr) != 0; ++rel) {
		byte_t **pointer = (byte_t **)((byte_t *)self + reladdr);
		DeeObject *obj = (DeeObject *)*pointer;
		ASSERT_OBJECT(obj);
		Dee_Decref_unlikely(obj);
	}
}

PRIVATE NONNULL((1)) void DCALL
applied_rrela_decref(DeeDec_Ehdr *__restrict self,
                     Dee_dec_addr32_t offsetof_reltab) {
	Dee_dec_addr32_t reladdr;
	Dec_RRela const *rel = (Dec_RRela const *)((byte_t *)self + offsetof_reltab);
	for (; (reladdr = rel->r_addr) != 0; ++rel) {
		byte_t **p_pointer = (byte_t **)((byte_t *)self + reladdr);
		byte_t *pointer = *p_pointer;
		DeeObject *obj = (DeeObject *)(pointer + (ptrdiff_t)rel->r_offs);
		ASSERT_OBJECT(obj);
		Dee_Decref_unlikely(obj);
	}
}

PRIVATE NONNULL((1)) void DCALL
applied_rrel_decref_nokill(DeeDec_Ehdr *__restrict self,
                           Dee_dec_addr32_t offsetof_reltab) {
	Dee_dec_addr32_t reladdr;
	Dec_RRel const *rel = (Dec_RRel const *)((byte_t *)self + offsetof_reltab);
	for (; (reladdr = rel->r_addr) != 0; ++rel) {
		byte_t **pointer = (byte_t **)((byte_t *)self + reladdr);
		DeeObject *obj = (DeeObject *)*pointer;
		ASSERT_OBJECT(obj);
		Dee_DecrefNokill(obj);
	}
}

PRIVATE NONNULL((1)) void DCALL
applied_rrela_decref_nokill(DeeDec_Ehdr *__restrict self,
                            Dee_dec_addr32_t offsetof_reltab) {
	Dee_dec_addr32_t reladdr;
	Dec_RRela const *rel = (Dec_RRela const *)((byte_t *)self + offsetof_reltab);
	for (; (reladdr = rel->r_addr) != 0; ++rel) {
		byte_t **p_pointer = (byte_t **)((byte_t *)self + reladdr);
		byte_t *pointer = *p_pointer;
		DeeObject *obj = (DeeObject *)(pointer + (ptrdiff_t)rel->r_offs);
		ASSERT_OBJECT(obj);
		Dee_DecrefNokill(obj);
	}
}

PRIVATE NONNULL((1)) void DCALL
DeeDec_RELOC_undo_rrel_and_decref_deps(DeeDec_Ehdr *__restrict self,
                                       size_t max_dep_incref_applied_count) {
	/* Undo incref() operations */
	size_t i;
	Dec_Dhdr *dhdr = (Dec_Dhdr *)((byte_t *)self + self->e_typedata.td_reloc.er_offsetof_deps);
	for (i = 0; dhdr[i].d_modspec.d_mod; ++i) {
		Dec_Dhdr *dep = &dhdr[i];
		DREF DeeModuleObject *mod = dep->d_modspec.d_mod;
		DBG_memset(&dep->d_modspec.d_mod, 0xcc, sizeof(dep->d_modspec.d_mod));
		if (i < max_dep_incref_applied_count) {
#if !defined(CONFIG_NO_DEX) && !defined(__OPTIMIZE_SIZE__)
			if (Dee_TYPE(mod) == &DeeModuleDex_Type) {
				/* Statics from DEX modules cannot be destroyed prematurely */
				applied_rrela_decref_nokill(self, dep->d_offsetof_rrela);
				applied_rrel_decref_nokill(self, dep->d_offsetof_rrel);
			} else
#endif /* !CONFIG_NO_DEX && !__OPTIMIZE_SIZE__ */
			{
				applied_rrela_decref(self, dep->d_offsetof_rrela);
				applied_rrel_decref(self, dep->d_offsetof_rrel);
			}
		}
		Dee_Decref(mod);
	}
	applied_rrela_decref_nokill(self, self->e_typedata.td_reloc.er_offsetof_drrela);
	applied_rrel_decref_nokill(self, self->e_typedata.td_reloc.er_offsetof_drrel);
}



/* Execute relocations on `*p_self' and return a pointer to the
 * first object of the dec file's heap (which is always the
 * `DeeModuleObject' describing the dec file itself).
 * NOTE: Can only be used when `(*p_self)->e_type == Dee_DEC_TYPE_RELOC'
 *
 * On success, `*p_self' is inherited by `return', such that rather than calling
 * `DeeDec_Ehdr_Destroy(*p_self)', you must `DeeDec_DestroyUntracked(return)'
 *
 * @param: flags: Set of `0 | DeeModule_IMPORT_F_CTXDIR':
 *                - DeeModule_IMPORT_F_CTXDIR: When set, "context_absname...+=context_absname_size" is
 *                                             the directory containing the .dec file mapped by "*p_self",
 *                                             rather than the .dec file itself.
 * @return: * :   The module object described by `*p_self'
 * @return: NULL: An error was thrown
 * @return: ITER_DONE: The DEC file was out of date or had been corrupted */
PUBLIC WUNUSED NONNULL((1, 2)) DREF /*untracked*/ struct Dee_module_object *DCALL
DeeDec_Relocate(/*inherit(on_success)*/ DeeDec_Ehdr **__restrict p_self,
                /*utf-8*/ char const *context_absname, size_t context_absname_size,
                unsigned int flags, struct Dee_compiler_options *options,
                uint64_t dee_file_last_modified) {
	DREF DeeModuleObject *result;
	Dec_Dhdr *dhdr;
	size_t dep_index;
	DeeDec_Ehdr *self = *p_self;
	ASSERTF(self->e_type == Dee_DEC_TYPE_RELOC,
	        "Bad API usage -- Only use this function with 'DeeDec_OpenFile()', "
	        "which should have already asserted that 'e_type == Dee_DEC_TYPE_RELOC'");
	dhdr = (Dec_Dhdr *)((byte_t *)self + self->e_typedata.td_reloc.er_offsetof_deps);

	/* Check if source file was modified **after** .dec file */
	if (dee_file_last_modified > self->e_typedata.td_reloc.er_build_timestamp) {
		Dee_DPRINTF("[LD][dec %q] CORRUPT: Source file modified after .dec file "
		            /**/ "was created: %" PRFu64 " > %" PRFu64 "\n",
		            context_absname, dee_file_last_modified,
		            self->e_typedata.td_reloc.er_build_timestamp);
		goto corrupt;
	}

	/* Check if additionally dependent files have been modified since the dec file was created... */
	if (self->e_typedata.td_reloc.er_offsetof_files) {
		Dec_Dstr const *dep_files = (Dec_Dstr const *)((byte_t const *)self + self->e_typedata.td_reloc.er_offsetof_files);
		while (dep_files->ds_length) {
			Dee_DPRINTF("[LD][dec %q] XXX: Check Dependency file %$q is modified after .dec file was created\n",
			            context_absname, dep_files->ds_length, dep_files->ds_string);

			/* XXX: Check timestamp of dependent file (if the encoded path
			 *      is relative, use "context_absname" to resolve it) */

			dep_files = (Dec_Dstr const *)(dep_files->ds_string + dep_files->ds_length + 1);
			dep_files = (Dec_Dstr const *)CEIL_ALIGN((uintptr_t)dep_files, Dee_ALIGNOF_DEC_DSTR);
		}
	}

	/* Enforce alignment requirements specified by header
	 *
	 * This is actually rather unlikely, since normally the required alignment
	 * should also be matched by "self". When embedded slab allocators are used,
	 * the requirement becomes PAGESIZE, and if "self" is a file-mapping, then
	 * it already supports that alignment natively. */
	if unlikely(!IS_ALIGNED((uintptr_t)self, self->e_typedata.td_reloc.er_alignment)) {
		/* Allocate properly aligned buffer for image */
		void *aligned_image;
		size_t image_size;
		image_size    = self->e_typedata.td_reloc.er_offsetof_eof;
		aligned_image = Dee_Memalign(self->e_typedata.td_reloc.er_alignment, image_size);
		if unlikely(!aligned_image)
			goto err;
		aligned_image = memcpy(aligned_image, self, image_size);
		DeeMapFile_Fini(&self->e_mapping);
		*p_self = self = (DeeDec_Ehdr *)aligned_image;
		DeeMapFile_SETHEAP(&self->e_mapping);
		DeeMapFile_SETADDR(&self->e_mapping, aligned_image);
		DeeMapFile_SETSIZE(&self->e_mapping, image_size);
	}

	/* Load dependencies */
	flags &= DeeModule_IMPORT_F_CTXDIR;
	flags |= DeeModule_IMPORT_F_ENOENT | DeeModule_IMPORT_F_ERECUR;
	for (dep_index = 0; dhdr[dep_index].d_modspec.d_file.d_offsetof_modname;) {
		DREF DeeModuleObject *dep;
		union Dee_module_buildid const *dep_buildid;
		Dec_Dhdr *dependency = &dhdr[dep_index];
		Dec_Dstr *dependency_name = (Dec_Dstr *)((byte_t *)self + dependency->d_modspec.d_file.d_offsetof_modname);
		dep = DeeModule_OpenEx(dependency_name->ds_string,
		                       dependency_name->ds_length,
		                       context_absname, context_absname_size,
		                       flags, options);
		if unlikely(!DeeModule_IMPORT_ISOK(dep)) {
			if unlikely(dep == DeeModule_IMPORT_ERROR)
				goto err_dep_index;
			Dee_DPRINTF("[LD][dec %q] CORRUPT: Failed to open dependency %$q\n",
			            context_absname, (size_t)dependency_name->ds_length, dependency_name->ds_string);
			goto corrupt_dep_index;
		}

		/* Apply non-incref relocations now, since we're about to overwrite the pointer to that table */
		apply_rel(self, dependency->d_modspec.d_file.d_offsetof_rel, (uintptr_t)dep);
		dependency->d_modspec.d_mod = dep; /* Inherit reference */
		++dep_index;

		/* Check timestamp of dependency (must be older than our dec file's timestamp) */
		dep_buildid = DeeModule_GetBuildId(dep);
		if unlikely(!dep_buildid)
			goto err_dep_index;
		if (dep_buildid->mbi_word64[0] != dependency->d_buildid[0] ||
		    dep_buildid->mbi_word64[1] != dependency->d_buildid[1]) {
#ifndef Dee_DPRINT_IS_NOOP
			Dee_uint128_t expected_buildid, actual_buildid;
			memcpy(&expected_buildid, dependency->d_buildid, sizeof(expected_buildid));
			memcpy(&actual_buildid, dep_buildid, sizeof(actual_buildid));
			Dee_DPRINTF("[LD][dec %q] CORRUPT: Dependency %q has unexpected build ID. "
			            /**/ "Expected: %#.32" PRFx128 ", Actual: %#.32" PRFx128 "\n",
			            context_absname, DeeModule_GetAbsName(dep),
			            expected_buildid, actual_buildid);
#endif /* !Dee_DPRINT_IS_NOOP */
			goto corrupt_dep_index;
		}
	}

	/* Self-relocations... */
	apply_rel(self, self->e_typedata.td_reloc.er_offsetof_srel, (uintptr_t)self);

	/* Deemon-relocations... */
	apply_rel(self, self->e_typedata.td_reloc.er_offsetof_drel, (uintptr_t)&DeeModule_Deemon);
	apply_rrel_incref_nofail(self, self->e_typedata.td_reloc.er_offsetof_drrel, (uintptr_t)&DeeModule_Deemon);
	apply_rrela_incref_nofail(self, self->e_typedata.td_reloc.er_offsetof_drrela, (uintptr_t)&DeeModule_Deemon);

	/* Dependent-module-relocations... */
	ASSERT(dhdr == (Dec_Dhdr *)((byte_t *)self + self->e_typedata.td_reloc.er_offsetof_deps));
	for (dep_index = 0; dhdr[dep_index].d_modspec.d_mod; ++dep_index) {
		Dec_Dhdr *dep = &dhdr[dep_index];
		DeeModuleObject *mod = dep->d_modspec.d_mod;
		DeeObject *dead_object;
		dead_object = apply_rrel_incref(self, dep->d_offsetof_rrel, (uintptr_t)mod);
		if unlikely(dead_object) {
corrupt_dep_index_reloc_rrel:
			Dee_DPRINTF("[LD][dec %q] CORRUPT: Failed to incref dead object in "
			            /**/ "dependency #%" PRFuSIZ "(%q) at %#" PRFxSIZ " (%p)\n",
			            context_absname, dep_index, DeeModule_GetAbsName(mod),
			            (uintptr_t)dead_object - (uintptr_t)mod, dead_object);
			goto corrupt_dep_index_reloc;
		}
		dead_object = apply_rrela_incref(self, dep->d_offsetof_rrela, (uintptr_t)mod);
		if unlikely(dead_object) {
			applied_rrel_decref(self, dep->d_offsetof_rrel);
			goto corrupt_dep_index_reloc_rrel;
		}
	}

	/* Extended relocations */
	if (self->e_typedata.td_reloc.er_offsetof_xrel) {
		/* TODO */
	}

	/* At this point, the dec file should be fully initialized, and the
	 * first contained object should be the relevant DeeModuleObject! */
	result = DeeDec_Ehdr_GetModule(self);
	if (Dee_TYPE(result) != &DeeModuleDee_Type) {
		Dee_DPRINTF("[LD][dec %q] CORRUPT: Module object has unexpected type %p when %p (DeeModuleDee_Type) was expected\n",
		            context_absname, Dee_TYPE(result), &DeeModuleDee_Type);
		goto corrupt_dep_index_reloc;
	}
	if (result->ob_refcnt == 0) {
		Dee_DPRINTF("[LD][dec %q] CORRUPT: Module object has 'ob_refcnt == 0'\n",
		            context_absname);
		goto corrupt_dep_index_reloc;
	}

#if 0 /* This needs to be done by the caller using `DeeDec_Track()' */
	if (self->e_typedata.td_reloc.er_offsetof_gchead) {
		/* Link in GC objects (if there are any) */
		DeeObject *gc_head = DeeDec_Ehdr_RELOC_GetGCHead(self);
		DeeObject *gc_tail = DeeDec_Ehdr_RELOC_GetGCTail(self);
		DeeGC_TrackAll(gc_head, gc_tail, DeeGC_TRACK_F_NORMAL);
	}
#endif

	return result;
corrupt_dep_index_reloc:
	/* Undo incref() operations */
	DeeDec_RELOC_undo_rrel_and_decref_deps(self, dep_index);
	return (DeeModuleObject *)ITER_DONE;
err_dep_index:
	while (dep_index--)
		Dee_Decref(dhdr[dep_index].d_modspec.d_mod);
err:
	return NULL;
corrupt_dep_index:
	while (dep_index--)
		Dee_Decref(dhdr[dep_index].d_modspec.d_mod);
corrupt:
	return (DREF DeeModuleObject *)ITER_DONE;
}





PRIVATE NONNULL((1)) void DCALL
w_apply_rel(DeeDec_Ehdr *__restrict self,
            Dec_Rel const *vec, size_t count,
            uintptr_t relbase) {
	size_t i;
	for (i = 0; i < count; ++i) {
		Dee_dec_addr32_t reladdr = vec[i].r_addr;
		byte_t **p_pointer = (byte_t **)((byte_t *)self + reladdr);
		*p_pointer += relbase;
	}
}

#if 1
STATIC_ASSERT(sizeof(Dec_RRel) == sizeof(Dec_Rel));
STATIC_ASSERT(offsetof(Dec_RRel, r_addr) == offsetof(Dec_Rel, r_addr));
STATIC_ASSERT(offsetafter(Dec_RRel, r_addr) == offsetafter(Dec_Rel, r_addr));
STATIC_ASSERT(sizeof(struct Dee_dec_rreltab) == sizeof(struct Dee_dec_reltab));
#ifdef __INTELLISENSE__
PRIVATE NONNULL((1)) void DCALL
w_apply_rrel(DeeDec_Ehdr *__restrict self,
             Dec_RRel const *vec, size_t count,
             uintptr_t relbase);
#else /* __INTELLISENSE__ */
#define w_apply_rrel(self, vec, count, relbase) w_apply_rel(self, (Dec_Rel const *)(vec), count, relbase)
#endif /* !__INTELLISENSE__ */
#else
PRIVATE NONNULL((1)) void DCALL
w_apply_rrel(DeeDec_Ehdr *__restrict self,
             Dec_RRel const *vec, size_t count,
             uintptr_t relbase) {
	size_t i;
	for (i = 0; i < count; ++i) {
		Dee_dec_addr32_t reladdr = vec[i].r_addr;
		byte_t **p_pointer = (byte_t **)((byte_t *)self + reladdr);
		*p_pointer += relbase;
	}
}
#endif

PRIVATE NONNULL((1)) void DCALL
w_apply_rrela(DeeDec_Ehdr *__restrict self,
              Dec_RRela const *vec, size_t count,
              uintptr_t relbase) {
	size_t i;
	for (i = 0; i < count; ++i) {
		Dee_dec_addr32_t reladdr = vec[i].r_addr;
		byte_t **p_pointer = (byte_t **)((byte_t *)self + reladdr);
		*p_pointer += relbase;
	}
}

PRIVATE NONNULL((1)) void DCALL
w_applied_rrel_decref(DeeDec_Ehdr *__restrict self,
                      Dec_RRel const *vec, size_t count) {
	size_t i;
	for (i = 0; i < count; ++i) {
		Dee_dec_addr32_t reladdr = vec[i].r_addr;
		byte_t **pointer = (byte_t **)((byte_t *)self + reladdr);
		DeeObject *obj = (DeeObject *)*pointer;
		ASSERT_OBJECT(obj);
		Dee_Decref_unlikely(obj);
	}
}

PRIVATE NONNULL((1)) void DCALL
w_applied_rrela_decref(DeeDec_Ehdr *__restrict self,
                       Dec_RRela const *vec, size_t count) {
	size_t i;
	for (i = 0; i < count; ++i) {
		Dee_dec_addr32_t reladdr = vec[i].r_addr;
		byte_t **p_pointer = (byte_t **)((byte_t *)self + reladdr);
		byte_t *pointer = *p_pointer;
		DeeObject *obj = (DeeObject *)(pointer + (ptrdiff_t)vec[i].r_offs);
		ASSERT_OBJECT(obj);
		Dee_Decref_unlikely(obj);
	}
}


PRIVATE NONNULL((1)) void DCALL
w_applied_rrel_decref_nokill(DeeDec_Ehdr *__restrict self,
                             Dec_RRel const *vec, size_t count) {
	size_t i;
	for (i = 0; i < count; ++i) {
		Dee_dec_addr32_t reladdr = vec[i].r_addr;
		byte_t **pointer = (byte_t **)((byte_t *)self + reladdr);
		DeeObject *obj = (DeeObject *)*pointer;
		ASSERT_OBJECT(obj);
		Dee_DecrefNokill(obj);
	}
}

#ifndef __OPTIMIZE_SIZE__
PRIVATE NONNULL((1)) void DCALL
w_applied_rrela_decref_nokill(DeeDec_Ehdr *__restrict self,
                              Dec_RRela const *vec, size_t count) {
	size_t i;
	for (i = 0; i < count; ++i) {
		Dee_dec_addr32_t reladdr = vec[i].r_addr;
		byte_t **p_pointer = (byte_t **)((byte_t *)self + reladdr);
		byte_t *pointer = *p_pointer;
		DeeObject *obj = (DeeObject *)(pointer + (ptrdiff_t)vec[i].r_offs);
		ASSERT_OBJECT(obj);
		Dee_DecrefNokill(obj);
	}
}
#endif /* !__OPTIMIZE_SIZE__ */


PRIVATE NONNULL((1)) void DCALL
w_rrel_decref(DeeDec_Ehdr *__restrict self,
              Dec_RRel const *vec, size_t count,
              uintptr_t relbase) {
	size_t i;
	for (i = 0; i < count; ++i) {
		Dee_dec_addr32_t reladdr = vec[i].r_addr;
		byte_t **pointer = (byte_t **)((byte_t *)self + reladdr);
		DeeObject *obj = (DeeObject *)(*pointer + relbase);
		ASSERT_OBJECT(obj);
		Dee_Decref_unlikely(obj);
	}
}

PRIVATE NONNULL((1)) void DCALL
w_rrela_decref(DeeDec_Ehdr *__restrict self,
               Dec_RRela const *vec, size_t count,
               uintptr_t relbase) {
	size_t i;
	for (i = 0; i < count; ++i) {
		Dee_dec_addr32_t reladdr = vec[i].r_addr;
		byte_t **p_pointer = (byte_t **)((byte_t *)self + reladdr);
		byte_t *pointer = *p_pointer;
		DeeObject *obj = (DeeObject *)(pointer + relbase + (ptrdiff_t)vec[i].r_offs);
		ASSERT_OBJECT(obj);
		Dee_Decref_unlikely(obj);
	}
}


PRIVATE NONNULL((1)) void DCALL
w_rrel_decref_nokill(DeeDec_Ehdr *__restrict self,
                     Dec_RRel const *vec, size_t count,
                     uintptr_t relbase) {
	size_t i;
	for (i = 0; i < count; ++i) {
		Dee_dec_addr32_t reladdr = vec[i].r_addr;
		byte_t **pointer = (byte_t **)((byte_t *)self + reladdr);
		DeeObject *obj = (DeeObject *)(*pointer + relbase);
		ASSERT_OBJECT(obj);
		Dee_DecrefNokill(obj);
	}
}

#if 0 /* Unused */
PRIVATE NONNULL((1)) void DCALL
w_rrela_decref_nokill(DeeDec_Ehdr *__restrict self,
                      Dec_RRela const *vec, size_t count,
                      uintptr_t relbase) {
	size_t i;
	for (i = 0; i < count; ++i) {
		Dee_dec_addr32_t reladdr = vec[i].r_addr;
		byte_t **p_pointer = (byte_t **)((byte_t *)self + reladdr);
		byte_t *pointer = *p_pointer;
		DeeObject *obj = (DeeObject *)(pointer + relbase + (ptrdiff_t)vec[i].r_offs);
		ASSERT_OBJECT(obj);
		Dee_DecrefNokill(obj);
	}
}
#endif


/* Similar to `DeeDec_Relocate()', but also works when `ehdr' is a "simplified" DEC
 * EHDR (as created by `DeeDecWriter_PackEhdr()' when `DeeModule_IMPORT_F_NOGDEC'
 * is set), since this function will take info about relocations and dependencies
 * from `self', rather than `ehdr->e_typedata.td_reloc'
 * NOTE: Can be used with both `Dee_DEC_TYPE_RELOC' and `Dee_DEC_TYPE_IMAGE'
 *
 * @return: * :    Success (given "ehdr" has been inherited). Caller must still start
 *                 tracking returned module via `DeeDec_Track()', or destroy it using
 *                 `DeeDec_Ehdr_Destroy(DeeDec_Ehdr_FromModule(return))'
 * @return: NULL : An error was thrown (given "ehdr" was *NOT* inherited) */
PUBLIC WUNUSED NONNULL((1, 2)) DREF /*untracked*/ struct Dee_module_object *DCALL
DeeDecWriter_PackModule(DeeDecWriter *__restrict self,
                        /*inherit(on_success)*/ DeeDec_Ehdr *__restrict ehdr) {
	size_t dep_index;
	DREF /*untracked*/ struct Dee_module_object *result;
	result = DeeDec_Ehdr_GetModule(ehdr);

	/* If the module didn't end up having relocation info,
	 * remember that fact within the module itself, so that
	 * other modules that depend on this one will know that
	 * trying to generate .dec files is impossible. */
	if (ehdr->e_type != Dee_DEC_TYPE_RELOC)
		result->mo_flags |= Dee_MODULE_FNOSERIAL;

	/* Self-relocations... */
	w_apply_rel(ehdr, self->dw_srel.drlt_relv, self->dw_srel.drlt_relc, (uintptr_t)ehdr);

	/* Deemon-relocations... */
	w_apply_rel(ehdr, self->dw_drel.drlt_relv, self->dw_drel.drlt_relc, (uintptr_t)&DeeModule_Deemon);
	w_apply_rrel(ehdr, self->dw_drrel.drrt_relv, self->dw_drrel.drrt_relc, (uintptr_t)&DeeModule_Deemon);
	w_apply_rrela(ehdr, self->dw_drrela.drat_relv, self->dw_drrela.drat_relc, (uintptr_t)&DeeModule_Deemon);

	/* Dependent-module-relocations... */
	for (dep_index = 0; dep_index < self->dw_deps.ddpt_depc; ++dep_index) {
		struct Dee_dec_depmod *dep = &self->dw_deps.ddpt_depv[dep_index];
		DeeModuleObject *mod = dep->ddm_mod;
		w_apply_rel(ehdr, dep->ddm_rel.drlt_relv, dep->ddm_rel.drlt_relc, (uintptr_t)mod);
		w_apply_rrel(ehdr, dep->ddm_rrel.drrt_relv, dep->ddm_rrel.drrt_relc, (uintptr_t)mod);
		w_apply_rrela(ehdr, dep->ddm_rrela.drat_relv, dep->ddm_rrela.drat_relc, (uintptr_t)mod);
	}

	/* At this point, the dec file should be fully initialized, and the
	 * first contained object should be the relevant DeeModuleObject!
	 *
	 * And since relocation info originates from deemon itself, rather
	 * than an external file (i.e.: is a trusted source), we can simply
	 * assert that the module looks correct, and not have to implement
	 * some kind of incref-undo functionality. */
	ASSERT_OBJECT_TYPE_EXACT(result, &DeeModuleDee_Type);

	/* Setup the EHDR to own the relocation tables related to externally incref'd objects */
	ehdr->e_type = Dee_DEC_TYPE_IMAGE;

	/* Steal relocation tables that must be inherited by the EHDR.
	 * NOTE: This also causes us to steal all the references to objects
	 *       that were already stored in those relocation tables. */
	ehdr->e_typedata.td_image.ei_drrel_v = self->dw_drrel.drrt_relv;
	ehdr->e_typedata.td_image.ei_drrel_c = self->dw_drrel.drrt_relc;
	Dee_dec_rreltab_init(&self->dw_drrel);
	ehdr->e_typedata.td_image.ei_drrela_v = self->dw_drrela.drat_relv;
	ehdr->e_typedata.td_image.ei_drrela_c = self->dw_drrela.drat_relc;
	Dee_dec_rrelatab_init(&self->dw_drrela);
	ehdr->e_typedata.td_image.ei_deps_v = self->dw_deps.ddpt_depv;
	ehdr->e_typedata.td_image.ei_deps_c = self->dw_deps.ddpt_depc;
	Dee_dec_deptab_init(&self->dw_deps);
	ehdr->e_typedata.td_image.ei_offsetof_gchead = self->dw_gchead;
	ehdr->e_typedata.td_image.ei_offsetof_gctail = self->dw_gctail;

#if !defined(NDEBUG) && 0
	/* Debug-print heap blocks */
	{
		struct Dee_heapchunk *chunk = &ehdr->e_heap.hr_first;
		size_t count = 0;
		while (chunk->hc_head) {
			size_t size = sizeof(*chunk) + (chunk->hc_head - Dee_HEAPCHUNK_HEAD(0));
			Dee_DPRINTF("dec: heap[%03Iu]: %p-%p (%#Ix)\n", count, chunk, (char *)chunk + size - 1, size);
			chunk = (struct Dee_heapchunk *)((char *)chunk + size);
			++count;
		}
	}
#endif /* !NDEBUG */

	/* Return module linked to EHDR */
	return result;
}

/* Free relocation-only data from the image mapping of `DeeDec_Ehdr'.
 *
 * - Dee_DEC_TYPE_RELOC:
 *   For EHDRs created by `DeeDec_Relocate()', this will try to munmap()
 *   or realloc_in_place() all data of `self' that comes after the end
 *   of the file's object heap (iow: will truncate `self->e_mapping' to
 *   have a size of `offsetof(DeeDec_Ehdr, e_heap) + self->e_heap.hr_size')
 *
 * - Dee_DEC_TYPE_IMAGE:
 *   For EHDRs created by `DeeDecWriter_PackModule()', this frees the
 *   relocation tables that were stolen from the associated `DeeDecWriter'
 *   and only kept within the dec's EHDR for `DeeDec_DestroyUntracked()'
 *   to be able to undo incref()s that had been done.
 *   Because 'Dee_DEC_TYPE_RELOC' may be converted to this type of EHDR,
 *   this type will also try to truncate `self->e_mapping'. */
INTERN NONNULL((1)) void DCALL
DeeDec_Ehdr_FreeRelocationData(DeeDec_Ehdr *__restrict self) {
	size_t heap_end_offset;
	switch (self->e_type) {

	case Dee_DEC_TYPE_RELOC: {
		/* Drop references to dependencies that were stored in `Dec_Dhdr::d_modspec.d_mod'. */
		size_t i;
		Dec_Dhdr *dhdr = (Dec_Dhdr *)((byte_t *)self + self->e_typedata.td_reloc.er_offsetof_deps);
		for (i = 0; dhdr[i].d_modspec.d_mod; ++i) {
			DREF DeeModuleObject *mod = dhdr[i].d_modspec.d_mod;
			DBG_memset(&dhdr[i].d_modspec.d_mod, 0xcc, sizeof(dhdr[i].d_modspec.d_mod));
			Dee_Decref_unlikely(mod);
		}
	}	break;

	case Dee_DEC_TYPE_IMAGE: {
		struct Dee_dec_depmod *deps_v;
		size_t i, deps_c;
		Dee_Free(self->e_typedata.td_image.ei_drrel_v);
		Dee_Free(self->e_typedata.td_image.ei_drrela_v);
		deps_v = self->e_typedata.td_image.ei_deps_v;
		deps_c = self->e_typedata.td_image.ei_deps_c;
		for (i = 0; i < deps_c; ++i) {
			struct Dee_dec_depmod *dep = &deps_v[i];
			Dee_dec_depmod_fini(dep);
		}
		Dee_Free(deps_v);
		/* Don't "DBG_memset()" all of "e_typedata" -- we still need "ei_offsetof_gchead" and "ei_offsetof_gctail" */
		DBG_memset(&self->e_typedata.td_image.ei_drrel_v, 0xcc, sizeof(self->e_typedata.td_image.ei_drrel_v));
		DBG_memset(&self->e_typedata.td_image.ei_drrel_c, 0xcc, sizeof(self->e_typedata.td_image.ei_drrel_c));
		DBG_memset(&self->e_typedata.td_image.ei_drrela_v, 0xcc, sizeof(self->e_typedata.td_image.ei_drrela_v));
		DBG_memset(&self->e_typedata.td_image.ei_drrela_c, 0xcc, sizeof(self->e_typedata.td_image.ei_drrela_c));
		DBG_memset(&self->e_typedata.td_image.ei_deps_v, 0xcc, sizeof(self->e_typedata.td_image.ei_deps_v));
		DBG_memset(&self->e_typedata.td_image.ei_deps_c, 0xcc, sizeof(self->e_typedata.td_image.ei_deps_c));
	}	break;

	default: __builtin_unreachable();
	}

	/* Try to munmap() or realloc_in_place() to truncate unused trailing
	 * memory within `self'. Namely: everything after `e_heap', which ends
	 * at offset `offsetof(DeeDec_Ehdr, e_heap) + self->e_heap.hr_size'
	 *
	 * As such, the value of `DeeMapFile_GetSize(&self->e_mapping)' will
	 * be lowered up until (but not becoming less than) the end of the
	 * heap: `offsetof(DeeDec_Ehdr, e_heap) + self->e_heap.hr_size'.
	 */
	ASSERTF(DeeMapFile_GetAddr(&self->e_mapping) == (void *)self,
	        "The EHDR should be located at the start of the file mapping");
	heap_end_offset = offsetof(Dec_Ehdr, e_heap) + self->e_heap.hr_size;
	ASSERTF(heap_end_offset <= DeeMapFile_GetSize(&self->e_mapping),
	        "The heap should have ended before the EOF as stated by the EHDR");
	if (DeeMapFile_TryTruncate(&self->e_mapping, heap_end_offset)) {
		ASSERTF(DeeMapFile_GetSize(&self->e_mapping) >= heap_end_offset,
		        "DeeMapFile_TryTruncate() removed more than it was allowed to?");
	}
}

/* Destroy a module and all contained objects prior to `DeeDec_Track()' having been called. */
PUBLIC NONNULL((1)) void DCALL
DeeDec_DestroyUntracked(DREF /*untracked*/ struct Dee_module_object *__restrict self) {
	Dec_Ehdr *ehdr = DeeDec_Ehdr_FromModule(self);
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeModuleDee_Type);
	ASSERT(ehdr->e_heap.hr_destroy == &DeeDec_heapregion_destroy);
	ASSERT(DeeHeap_GetRegionOf(DeeGC_Head(self)) == &ehdr->e_heap);

	/* decref all objects from external modules that were incref'd
	 * when "self" was relocated. For this purpose, must support 2
	 * ways of the module having been relocated:
	 *
	 * - DeeDecWriter_PackModule() (following `DeeDecWriter_PackEhdr()'
	 *   not having encoded relocation info within the dec file image)
	 * - DeeDec_RelocateEx() (which can only be used when relocation
	 *   info exists within the dec file image, as per `DeeDec_Ehdr')
	 */

	switch (ehdr->e_type) {

	case Dee_DEC_TYPE_RELOC:
		/* Drop references to dependencies that were stored
		 * in `Dec_Dhdr::d_modspec.d_mod', as well as undo
		 * all incref() relocations against that module. */
		DeeDec_RELOC_undo_rrel_and_decref_deps(ehdr, (size_t)-1);
		break;

	case Dee_DEC_TYPE_IMAGE: {
		struct Dee_dec_depmod *deps_v;
		size_t i, deps_c;
		w_applied_rrel_decref_nokill(ehdr, ehdr->e_typedata.td_image.ei_drrel_v, ehdr->e_typedata.td_image.ei_drrel_c);
		Dee_Free(ehdr->e_typedata.td_image.ei_drrel_v);
		/* Special case: this might actually kill if it was a "DeeDecWriter_F_NRELOC" relocation.
		 *               As such, can't use `w_applied_rrela_decref_nokill' here! */
		w_applied_rrela_decref(ehdr, ehdr->e_typedata.td_image.ei_drrela_v, ehdr->e_typedata.td_image.ei_drrela_c);
		Dee_Free(ehdr->e_typedata.td_image.ei_drrela_v);
		deps_v = ehdr->e_typedata.td_image.ei_deps_v;
		deps_c = ehdr->e_typedata.td_image.ei_deps_c;
		for (i = 0; i < deps_c; ++i) {
			struct Dee_dec_depmod *dep = &deps_v[i];
#ifndef __OPTIMIZE_SIZE__
			if (Dee_TYPE(dep->ddm_mod) == &DeeModuleDex_Type) {
				/* Statics from DEX modules cannot be destroyed prematurely */
				w_applied_rrel_decref_nokill(ehdr, dep->ddm_rrel.drrt_relv, dep->ddm_rrel.drrt_relc);
				w_applied_rrela_decref_nokill(ehdr, dep->ddm_rrela.drat_relv, dep->ddm_rrela.drat_relc);
			} else
#endif /* !__OPTIMIZE_SIZE__ */
			{
				w_applied_rrel_decref(ehdr, dep->ddm_rrel.drrt_relv, dep->ddm_rrel.drrt_relc);
				w_applied_rrela_decref(ehdr, dep->ddm_rrela.drat_relv, dep->ddm_rrela.drat_relc);
			}
			Dee_dec_depmod_fini(dep);
		}
		Dee_Free(deps_v);
		DBG_memset(&ehdr->e_typedata, 0xcc, sizeof(ehdr->e_typedata));
	}	break;

	default: __builtin_unreachable();
	}

	DeeDec_Ehdr_Destroy(ehdr);
}


/* Validate the contents of `fmap' and relocate them. Once all locks have been
 * acquired to register the module globally, the caller must call `DeeDec_Track()'
 * to hook the start tracking GC objects related to the returned module (including
 * the returned module itself).
 *
 * @param: flags: See `DeeDec_Relocate()'
 * @param: dee_file_last_modified: Timestamp when the ".dee" file was last modified
 * @return: * :        Successfully loaded the given DEC file.
 * @return: ITER_DONE: The DEC file was out of date or had been corrupted.
 * @return: NULL:      An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF /*untracked*/ struct Dee_module_object *DCALL
DeeDec_OpenFile(/*inherit(on_success)*/ struct DeeMapFile *__restrict fmap,
                /*utf-8*/ char const *context_absname, size_t context_absname_size,
                unsigned int flags, struct Dee_compiler_options *options,
                uint64_t dee_file_last_modified) {
	DREF /*untracked*/ struct Dee_module_object *result;
#ifndef Dee_DPRINT_IS_NOOP
	char const *fail_reason;
#ifdef __OPTIMIZE_SIZE__
#define goto_fail_if(cond, reason) \
	fail_reason = reason;          \
	if unlikely(cond)              \
		goto fail;
#else /* __OPTIMIZE_SIZE__ */
#define goto_fail_if(cond, reason) \
	do {                           \
		if unlikely(cond) {        \
			fail_reason = reason;  \
			goto fail;             \
		}                          \
	}	__WHILE0
#endif /* !__OPTIMIZE_SIZE__ */
#else /* !Dee_DPRINT_IS_NOOP */
#define goto_fail_if(cond, reason) \
	if unlikely(cond)              \
		goto fail
#endif /* Dee_DPRINT_IS_NOOP */
	union Dee_module_buildid const *deemon_buildid;
	Dec_Ehdr *ehdr = (Dec_Ehdr *)DeeMapFile_GetAddr(fmap);
	goto_fail_if(DeeMapFile_GetSize(fmap) < sizeof(Dec_Ehdr),
	             "File is smaller than 'sizeof(Dec_Ehdr)'");

	/* Validate dec file header... */
	deemon_buildid = DeeModule_GetBuildId(&DeeModule_Deemon);
	if unlikely(!deemon_buildid)
		goto err;
	if (ehdr->e_typedata.td_reloc.er_deemon_build_id[0] != deemon_buildid->mbi_word64[0] ||
	    ehdr->e_typedata.td_reloc.er_deemon_build_id[1] != deemon_buildid->mbi_word64[1]) {
#ifndef Dee_DPRINT_IS_NOOP
		Dee_uint128_t expected_buildid, actual_buildid;
		memcpy(&expected_buildid, deemon_buildid, sizeof(expected_buildid));
		memcpy(&actual_buildid, ehdr->e_typedata.td_reloc.er_deemon_build_id, sizeof(actual_buildid));
		Dee_DPRINTF("[LD][dec %q] CORRUPT: Deemon core Build ID doesn't match. "
		            /**/ "Expected: %#.32" PRFx128 ", Actual: %#.32" PRFx128 "\n",
		            context_absname, expected_buildid, actual_buildid);
#endif /* !Dee_DPRINT_IS_NOOP */
		goto fail_nomsg;
	}
	goto_fail_if(ehdr->e_ident[DI_MAG0] != DECMAG0, "Bad DI_MAG0");
	goto_fail_if(ehdr->e_ident[DI_MAG1] != DECMAG1, "Bad DI_MAG1");
	goto_fail_if(ehdr->e_ident[DI_MAG2] != DECMAG2, "Bad DI_MAG2");
	goto_fail_if(ehdr->e_ident[DI_MAG3] != DECMAG3, "Bad DI_MAG3");
	goto_fail_if(ehdr->e_mach != Dee_DEC_MACH, "Bad 'e_mach'");
	/* Only relocatable images can be written to disk, so that's the only valid type */
	goto_fail_if(ehdr->e_type != Dee_DEC_TYPE_RELOC, "Bad 'e_type'");
	goto_fail_if(ehdr->e_version != DVERSION_CUR, "Bad 'e_version'");

	/* We actually hard-code the expected heap-offset. This is because it could only
	 * be different for some different installation of deemon, in which case the dec
	 * file really should have already failed loading at "er_deemon_build_id" above. */
	goto_fail_if(ehdr->e_typedata.td_reloc.er_offsetof_heap != offsetof(Dec_Ehdr, e_heap),
	             "Bad 'er_offsetof_heap'");
	goto_fail_if(ehdr->e_typedata.td_reloc.er_sizeof_pointer != sizeof(void *),
	             "Bad 'er_sizeof_pointer'");
	goto_fail_if(ehdr->e_typedata.td_reloc.er_endian != Dee_DEC_ENDIAN,
	             "Bad 'er_endian'");
	goto_fail_if(ehdr->e_typedata.td_reloc.er_offsetof_eof != DeeMapFile_GetSize(fmap),
	             "Bad 'er_offsetof_eof' does not match size of file");
	goto_fail_if(ehdr->e_typedata.td_reloc.er_offsetof_eof > DFILE_LIMIT,
	             "Bad 'er_offsetof_eof' is greater than 'DFILE_LIMIT'");
	goto_fail_if(ehdr->e_typedata.td_reloc.er_offsetof_srel >= ehdr->e_typedata.td_reloc.er_offsetof_eof,
	             "Bad 'er_offsetof_srel'");
	goto_fail_if(ehdr->e_typedata.td_reloc.er_offsetof_drel >= ehdr->e_typedata.td_reloc.er_offsetof_eof,
	             "Bad 'er_offsetof_drel'");
	goto_fail_if(ehdr->e_typedata.td_reloc.er_offsetof_drrel >= ehdr->e_typedata.td_reloc.er_offsetof_eof,
	             "Bad 'er_offsetof_drrel'");
	goto_fail_if(ehdr->e_typedata.td_reloc.er_offsetof_deps >= ehdr->e_typedata.td_reloc.er_offsetof_eof,
	             "Bad 'er_offsetof_deps'");
	goto_fail_if(/*ehdr->e_typedata.td_reloc.er_offsetof_files != 0 &&*/
	             ehdr->e_typedata.td_reloc.er_offsetof_files >= ehdr->e_typedata.td_reloc.er_offsetof_eof,
	             "Bad 'er_offsetof_files'");
	goto_fail_if(/*ehdr->e_typedata.td_reloc.er_offsetof_xrel != 0 &&*/
	             ehdr->e_typedata.td_reloc.er_offsetof_xrel >= ehdr->e_typedata.td_reloc.er_offsetof_eof,
	             "Bad 'er_offsetof_xrel'");
	goto_fail_if(!IS_POWER_OF_TWO(ehdr->e_typedata.td_reloc.er_alignment),
	             "Bad 'er_alignment'");
	goto_fail_if(ehdr->e_typedata.td_reloc.er_offsetof_gchead == 0 ||
	             ehdr->e_typedata.td_reloc.er_offsetof_gchead >= ehdr->e_typedata.td_reloc.er_offsetof_eof,
	             "Bad 'er_offsetof_gchead'");
	goto_fail_if(ehdr->e_typedata.td_reloc.er_offsetof_gctail == 0 ||
	             ehdr->e_typedata.td_reloc.er_offsetof_gctail >= ehdr->e_typedata.td_reloc.er_offsetof_eof,
	             "Bad 'er_offsetof_gctail'");
	goto_fail_if((ehdr->e_typedata.td_reloc.er_offsetof_gchead != 0) !=
	             (ehdr->e_typedata.td_reloc.er_offsetof_gctail != 0),
	             "Presence of 'er_offsetof_gchead' and 'er_offsetof_gctail' do not match");
	goto_fail_if(ehdr->e_heap.hr_size >= (ehdr->e_typedata.td_reloc.er_offsetof_eof - offsetof(Dec_Ehdr, e_heap)),
	             "Bad 'e_heap.hr_size' points past EOF");

#if 1 /* Validate the module's MD5 checksum (which is also its "build ID") */
	{
		DeeMD5_Context md5;
		DeeModuleObject *mod = DeeDec_Ehdr_GetModule(ehdr);
		union Dee_module_buildid true_buildid;
		union Dee_module_buildid saved_mo_buildid = mod->mo_buildid;
		uint64_t saved_build_timestamp = ehdr->e_typedata.td_reloc.er_build_timestamp;

		/* The build ID obviously can't participate in its own calculation */
		mod->mo_buildid.mbi_word64[0] = 0;
		mod->mo_buildid.mbi_word64[1] = 0;

		/* The build timestamp also doesn't participate (this way, a module source
		 * being touched without any changes made to the source code (or the only
		 * changes made not affected the generated code), will not result in a
		 * cascade of there-on depending modules also needing to be re-built).
		 *
		 * s.a.: the MD5 generation code in `DeeDecWriter_PackEhdr()' */
		ehdr->e_typedata.td_reloc.er_build_timestamp = 0;

		/* Calculate MD5 checksum */
		DeeMD5_Init(&md5);
		DeeMD5_Update(&md5, ehdr, ehdr->e_typedata.td_reloc.er_offsetof_eof);
		DeeMD5_Finalize(&md5, true_buildid.mbi_word32);

		/* Restore  */
		ASSERT(mod == DeeDec_Ehdr_GetModule(ehdr));
		mod->mo_buildid = saved_mo_buildid;
		ehdr->e_typedata.td_reloc.er_build_timestamp = saved_build_timestamp;

		if unlikely(true_buildid.mbi_word64[0] != saved_mo_buildid.mbi_word64[0] ||
		            true_buildid.mbi_word64[1] != saved_mo_buildid.mbi_word64[1]) {
#ifndef Dee_DPRINT_IS_NOOP
			Dee_uint128_t expected_buildid, actual_buildid;
			memcpy(&expected_buildid, &saved_mo_buildid, sizeof(expected_buildid));
			memcpy(&actual_buildid, &true_buildid, sizeof(actual_buildid));
			Dee_DPRINTF("[LD][dec %q] CORRUPT: Bad checksum. "
			            /**/ "Expected: %#.32" PRFx128 ", Actual: %#.32" PRFx128 "\n",
			            context_absname, expected_buildid, actual_buildid);
#endif /* !Dee_DPRINT_IS_NOOP */
			goto fail_nomsg;
		}
	}
#endif

	/* Configure the runtime portion of the ehdr */
	ehdr->e_mapping         = *fmap; /* Inherit */
	ehdr->e_heap.hr_destroy = &DeeDec_heapregion_destroy;

	/* Relocate the dec file to turn it into the embedded module object. */
	result = DeeDec_Relocate(&ehdr, context_absname, context_absname_size,
	                         flags, options, dee_file_last_modified);
	if unlikely(!ITER_ISOK(result) && (ehdr != (Dec_Ehdr *)DeeMapFile_GetAddr(fmap))) {
		/* Must update "fmap" to reflect the relocated "ehdr" */
		*fmap = ehdr->e_mapping; /* Inherit */
	}
	return result;
#undef goto_fail_if
fail:
#ifndef Dee_DPRINT_IS_NOOP
	Dee_DPRINTF("[LD][dec %q] CORRUPT: Header verification failed: %s\n",
	            context_absname, fail_reason);
#endif /* !Dee_DPRINT_IS_NOOP */
fail_nomsg:
	return (DREF DeeModuleObject *)ITER_DONE;
err:
	return NULL;
}





/************************************************************************/
/************************************************************************/
/*                                                                      */
/* DEC FILE CREATION                                                    */
/*                                                                      */
/************************************************************************/
/************************************************************************/

/* Generate import strings for module dependencies (s.a. `struct Dee_dec_depmod::ddm_impstr') */
PRIVATE WUNUSED NONNULL((1)) int DCALL
decwriter_genimpstr(DeeDecWriter *__restrict self,
                    /*utf-8*/ char const *context_absname,
                    size_t context_absname_size,
                    unsigned int flags) {
	size_t dep_index;
	unsigned int relname_flags;
#if DeeModule_IMPORT_F_CTXDIR == DeeModule_RELNAME_F_CTXDIR
	relname_flags = flags & DeeModule_IMPORT_F_CTXDIR;
#else /* DeeModule_IMPORT_F_CTXDIR == DeeModule_RELNAME_F_CTXDIR */
	relname_flags = flags & DeeModule_IMPORT_F_CTXDIR ? DeeModule_RELNAME_F_CTXDIR : 0;
#endif /* DeeModule_IMPORT_F_CTXDIR != DeeModule_RELNAME_F_CTXDIR */
	relname_flags |= DeeModule_RELNAME_F_LIBNAM; /* Enable libname-based dependencies */
	for (dep_index = 0; dep_index < self->dw_deps.ddpt_depc; ++dep_index) {
		DREF DeeStringObject *dep_name;
		struct Dee_dec_depmod *dep = &self->dw_deps.ddpt_depv[dep_index];
		DeeModuleObject *mod = dep->ddm_mod;
		ASSERT(!dep->ddm_impstr);
		dep_name = (DREF DeeStringObject *)DeeModule_GetRelNameEx(mod, context_absname,
		                                                          context_absname_size,
		                                                          relname_flags);
		if unlikely(!ITER_ISOK(dep_name)) {
			if (dep_name) {
				DeeError_Throwf(&DeeError_ValueError,
				                "Unable to determine relative import name for module "
				                /**/ "%k while trying to generate dec in %$q",
				                mod, context_absname_size, context_absname);
			}
			goto err;
		}
		dep->ddm_impstr = dep_name;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
decwriter_putpointer(DeeDecWriter *__restrict self,
                     Dee_seraddr_t addrof_pointer,
                     void const *pointer);

/* Free a singular, stand-alone dec slab page */
#if 1
#define decslab_free1 (Dee_Free)
#else
PRIVATE NONNULL((1)) void DCALL
decslab_free1(struct Dee_slab_page *__restrict self) {
	Dee_Free(self);
}
#endif

#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_lock_t decslab_freeN_lock = Dee_ATOMIC_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define decslab_freeN_lock_available()  Dee_atomic_lock_available(&decslab_freeN_lock)
#define decslab_freeN_lock_acquired()   Dee_atomic_lock_acquired(&decslab_freeN_lock)
#define decslab_freeN_lock_tryacquire() Dee_atomic_lock_tryacquire(&decslab_freeN_lock)
#define decslab_freeN_lock_acquire()    Dee_atomic_lock_acquire(&decslab_freeN_lock)
#define decslab_freeN_lock_waitfor()    Dee_atomic_lock_waitfor(&decslab_freeN_lock)
#define decslab_freeN_lock_release()    Dee_atomic_lock_release(&decslab_freeN_lock)

PRIVATE NONNULL((1)) void DCALL decslab_freeN_first(struct Dee_slab_page *__restrict self);
PRIVATE NONNULL((1)) void DCALL decslab_freeN_middle(struct Dee_slab_page *__restrict self);
PRIVATE NONNULL((1)) void DCALL decslab_freeN_last(struct Dee_slab_page *__restrict self);

/* Need some value that is distinct from "Dee_SLAB_PAGE_META_CUSTOM_MARKER"
 * to mark decslab-N pages that have already been free'd. To accomplish that,
 * simply derive a distinct value from "Dee_SLAB_PAGE_META_CUSTOM_MARKER". */
#define decslab_freeN_FREE_MAKER ((void *)((uintptr_t)Dee_SLAB_PAGE_META_CUSTOM_MARKER - 1))

/* Helper macros for interacting with decslab-N pages */
#define decslab_freeN_gettype(self) ((self)->sp_meta.spm_type.t_custom.c_free)
#define decslab_freeN_hasnext(self) (decslab_freeN_gettype(self) != &decslab_freeN_last)
#define decslab_freeN_hasprev(self) (decslab_freeN_gettype(self) != &decslab_freeN_first)
#define decslab_freeN_isfree(self)  ((self)->sp_meta.spm_type.t_custom.c_marker == decslab_freeN_FREE_MAKER)
#define decslab_freeN_mkfree(self)  (void)((self)->sp_meta.spm_type.t_custom.c_marker = decslab_freeN_FREE_MAKER)

PRIVATE WUNUSED NONNULL((1)) bool DCALL
decslab_freeN_allfree_locked(struct Dee_slab_page *__restrict self) {
	struct Dee_slab_page *iter;
	ASSERTF(decslab_freeN_isfree(self), "Caller should have marked this one as free");
	for (iter = self; decslab_freeN_hasprev(iter);) {
		--iter;
		if (!decslab_freeN_isfree(iter))
			goto nope;
	}
	for (iter = self; decslab_freeN_hasnext(iter);) {
		++iter;
		if (!decslab_freeN_isfree(iter))
			goto nope;
	}
	return true;
nope:
	return false;
}

PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
decslab_freeN(struct Dee_slab_page *__restrict self) {
	bool all_free;
	decslab_freeN_lock_acquire();
	decslab_freeN_mkfree(self); /* Mark as free */
	all_free = decslab_freeN_allfree_locked(self);
	decslab_freeN_lock_release();
	if (all_free) {
		while (decslab_freeN_hasprev(self))
			--self;
		Dee_Free(self);
	}
}

#ifdef __NO_ATTR_NOICF
/* Well... MSVC does ICF, but doesn't have an attribute to prevent it!
 * >> decslab_freeN_first  0x0056ddb0 {deemon.exe!decslab_freeN_middle(Dee_slab_page *)} void (Dee_slab_page *)
 * >> decslab_freeN_middle 0x0056ddb0 {deemon.exe!decslab_freeN_middle(Dee_slab_page *)} void (Dee_slab_page *)
 * >> decslab_freeN_last   0x0056ddb0 {deemon.exe!decslab_freeN_middle(Dee_slab_page *)} void (Dee_slab_page *)
 *
 * To prevent that from happening, we have to inject some unoptimizable dummy code... */
PRIVATE ATTR_NOINLINE void DCALL noicf_dummy(void) {
	Dee_funptr_t volatile x = (Dee_funptr_t)&noicf_dummy;
	(void)x;
}
#else /* __NO_ATTR_NOICF */
#define noicf_dummy() (void)0
#endif /* !__NO_ATTR_NOICF */

PRIVATE ATTR_NOICF NONNULL((1)) void DCALL
decslab_freeN_first(struct Dee_slab_page *__restrict self) {
	decslab_freeN(self);
}

PRIVATE ATTR_NOICF NONNULL((1)) void DCALL
decslab_freeN_middle(struct Dee_slab_page *__restrict self) {
	decslab_freeN(self);
	noicf_dummy(); /* Dummy to ensure function differs from "decslab_freeN_first()" */
}

PRIVATE ATTR_NOICF NONNULL((1)) void DCALL
decslab_freeN_last(struct Dee_slab_page *__restrict self) {
	noicf_dummy(); /* Dummy to ensure function differs from "decslab_freeN_first()" */
	decslab_freeN(self);
}


/* Finish build the currently active set of slab
 * pages (~ala `Dee_slab_page_buildpack()') */
PRIVATE WUNUSED NONNULL((1)) int DCALL
decwriter_build_slab_pages(DeeDecWriter *__restrict self) {
	struct Dee_heapchunk *slab_chunk;
	Dee_seraddr_t addrof_first_page;
	size_t i, num_pages;
	ASSERT(self->dw_slabs != 0);
	ASSERT(IS_ALIGNED(self->dw_slabs - sizeof(struct Dee_heapchunk), Dee_SLAB_PAGESIZE));
	ASSERT(IS_ALIGNED(self->dw_slabb + sizeof(struct Dee_heapchunk), Dee_SLAB_PAGESIZE));
#ifdef __NO_ATTR_NOICF
	ASSERT(&decslab_freeN_first != &decslab_freeN_middle);
	ASSERT(&decslab_freeN_middle != &decslab_freeN_last);
#endif /* __NO_ATTR_NOICF */
	slab_chunk = DeeDecWriter_Addr2Mem(self, self->dw_slabb, struct Dee_heapchunk);
	slab_chunk->hc_head = Dee_HEAPCHUNK_HEAD(self->dw_slabs - sizeof(struct Dee_heapchunk));

	/* Use static relocations to replicate `Dee_slab_page_buildpack()' such that
	 * each slab page can be free'd individually, and once all slab pages have
	 * been freed, the last free operation will call `Dee_Free()' on the base
	 * address of the first slab page (since that page has been preceded by a
	 * heap chunk header spanning the entirety of the slab segment).
	 *
	 * This way, once all slab pages are free, the slab system will automatically
	 * call forward into Dee_Free()'s "flag4" system, which will eventually allow
	 * the dec file's mapping to be unloaded once all parts of it have been free'd. */
	addrof_first_page = self->dw_slabb + sizeof(struct Dee_heapchunk);
	num_pages = (self->dw_slabs - sizeof(struct Dee_heapchunk)) / Dee_SLAB_PAGESIZE;

	/* This part here replicates what is done by `Dee_slab_page_buildpack()'
	 * (only that instead of directly initializing the slab page, it writes
	 * the relevant pointers to the dec file) */
	for (i = 0; i < num_pages; ++i) {
		typedef void (DCALL *slab_page_free_t)(struct Dee_slab_page *__restrict self);
		slab_page_free_t c_free;
		Dee_seraddr_t addrof_page;
		struct Dee_slab_page *page;
		if (i == 0) {
			c_free = num_pages == 1 ? (slab_page_free_t)&decslab_free1
			                        : &decslab_freeN_first;
		} else if (i == (num_pages - 1)) {
			c_free = &decslab_freeN_middle;
		} else {
			c_free = &decslab_freeN_last;
		}
		addrof_page = addrof_first_page + i * Dee_SLAB_PAGESIZE;
		page = DeeDecWriter_Addr2Mem(self, addrof_page, struct Dee_slab_page);
		if (page->sp_meta.spm_type.t_builder.spb_unused_lo <
		    page->sp_meta.spm_type.t_builder.spb_unused_hi) {
			byte_t *unused_base = (byte_t *)page + page->sp_meta.spm_type.t_builder.spb_unused_lo;
			size_t unused_size = (size_t)(page->sp_meta.spm_type.t_builder.spb_unused_hi -
			                              page->sp_meta.spm_type.t_builder.spb_unused_lo);
			memset(unused_base, 0xfe, unused_size);
		}

		page->sp_meta.spm_type.t_custom.c_marker = Dee_SLAB_PAGE_META_CUSTOM_MARKER;

		/* Encode free-function pointer */
		if unlikely(decwriter_putpointer(self,
		                                 addrof_page + offsetof(struct Dee_slab_page,
		                                                        sp_meta.spm_type.t_custom.c_free),
		                                 (void const *)(Dee_funptr_t)c_free))
			goto err;
	}
	return 0;
err:
	return -1;
}

/* Pack the dec file into a format where it can easily be written to a file:
 * >> DeeDec_Ehdr *ehdr = DeeDecWriter_PackEhdr(&writer);
 * >> DeeFile_WriteAll(fp, ehdr, ehdr->e_typedata.td_reloc.er_offsetof_eof);
 * >> DeeDec_Ehdr_Destroy(ehdr);
 *
 * The returned pointer should either:
 * - be free'd using `DeeDec_Ehdr_Destroy(return)'
 * - be passed to `DeeDecWriter_PackModule()'
 *   to turn it into a `DeeModuleObject'
 *
 * The returned EHDR has typing:
 * - Dee_DEC_TYPE_RELOC: when 'flags & DeeModule_IMPORT_F_NOGDEC' isn't given
 * - Dee_DEC_TYPE_IMAGE: when 'flags & DeeModule_IMPORT_F_NOGDEC' is given
 *
 * @param: context_absname: see `DeeDec_Relocate()' (ignored when `DeeModule_IMPORT_F_NOGDEC' is given)
 * @param: flags:           Set of `DeeModule_IMPORT_F_NOGDEC' + flags taken by `DeeDec_Relocate()'
 * @return: * :   The not-yet-relocated dec file (header + contents).
 *                This blob is serialized to the point where it can simply be written to some file
 *                (but only if `DeeModule_IMPORT_F_NOGDEC' wasn't given), and is also no longer
 *                owned by "self".
 * @return: NULL: An error was thrown */
PUBLIC WUNUSED NONNULL((1)) DeeDec_Ehdr *DCALL
DeeDecWriter_PackEhdr(DeeDecWriter *__restrict self,
                      /*utf-8*/ char const *context_absname,
                      size_t context_absname_size,
                      unsigned int flags) {
	Dec_Ehdr *ehdr = self->dw_ehdr;
	size_t total_need;

	ehdr->e_ident[DI_MAG0] = DECMAG0;
	ehdr->e_ident[DI_MAG1] = DECMAG1;
	ehdr->e_ident[DI_MAG2] = DECMAG2;
	ehdr->e_ident[DI_MAG3] = DECMAG3;
	ehdr->e_mach = Dee_DEC_MACH;
	ehdr->e_version = DVERSION_CUR;

	/* The main module should have been initialize with a 0 build ID. */
#ifndef NDEBUG
	{
		DeeModuleObject *mod = DeeDec_Ehdr_GetModule(ehdr);
		ASSERT(mod->mo_flags & Dee_MODULE_FHASBUILDID);
		ASSERT(mod->mo_buildid.mbi_word64[0] == 0);
		ASSERT(mod->mo_buildid.mbi_word64[1] == 0);
	}
#endif /* !NDEBUG */

	/* Update `dw_used' to include memory reserved for the currently-allocated page */
	if (self->dw_slabs) {
		ASSERT(self->dw_slabs > sizeof(struct Dee_heapchunk));
		ASSERT(IS_ALIGNED(self->dw_slabs - sizeof(struct Dee_heapchunk), Dee_SLAB_PAGESIZE));
		ASSERT(IS_ALIGNED(self->dw_slabb + sizeof(struct Dee_heapchunk), Dee_SLAB_PAGESIZE));
		if (self->dw_used <= self->dw_slabb) {
			struct Dee_heapchunk *slab_chunk;
			/* Extend the last heap-chunk */
			size_t avail_before_slab = (self->dw_slabb - self->dw_used);
			/* Extend the previous heap-chunk to go up to the slab page. */
			Dee_seraddr_t addrof_prev_heapchunk;
			struct Dee_heapchunk *prev_chunk;
			ASSERTF(self->dw_hlast != 0, "The first thing that's allocated should "
			                             "already be regular heap memory, so there "
			                             "should always be a preceding allocation");
			ASSERT(self->dw_used >= self->dw_hlast);
			addrof_prev_heapchunk = self->dw_used;
			addrof_prev_heapchunk -= sizeof(struct Dee_heapchunk);
			addrof_prev_heapchunk -= (self->dw_hlast - Dee_HEAPCHUNK_PREV(0));
			prev_chunk = DeeDecWriter_Addr2Mem(self, addrof_prev_heapchunk, struct Dee_heapchunk);
			prev_chunk->hc_head += avail_before_slab;
			memset(DeeDecWriter_Addr2Mem(self, self->dw_used, void), 0xfe, avail_before_slab);
			self->dw_hlast += avail_before_slab;
			self->dw_used += avail_before_slab;
			ASSERT(self->dw_used == self->dw_slabb);
			/* Fill in the heap-chunk descriptor for the slab area */
			slab_chunk = DeeDecWriter_Addr2Mem(self, self->dw_used, struct Dee_heapchunk);
			slab_chunk->hc_prevsize = self->dw_hlast;
			self->dw_used += self->dw_slabs;
			self->dw_hlast = Dee_HEAPCHUNK_PREV(self->dw_slabs - sizeof(struct Dee_heapchunk));
			ASSERT(self->dw_used == (self->dw_slabb + self->dw_slabs));
		}
		ASSERT(self->dw_used >= (self->dw_slabb + self->dw_slabs));
		if unlikely(decwriter_build_slab_pages(self))
			goto err;
		DBG_memset(&self->dw_slabb, 0xcc, sizeof(self->dw_slabb));
		self->dw_slabs = 0;
	}

	/* Space for the heap tail is always pre-allocated! */
	{
		Dee_seraddr_t offsetof_tail = self->dw_used;
		struct Dee_heaptail *tail = (struct Dee_heaptail *)((byte_t *)ehdr + offsetof_tail);
		offsetof_tail += sizeof(struct Dee_heaptail);
		tail->ht_lastsize = self->dw_hlast;
		tail->ht_zero     = 0;
		ehdr->e_heap.hr_size = offsetof_tail - offsetof(Dec_Ehdr, e_heap);
	}

#ifndef CONFIG_NO_DEC
	if (!(flags & DeeModule_IMPORT_F_NOGDEC) &&
	    !(self->dw_flags & DeeDecWriter_F_NRELOC) && likely(self->dw_used < DFILE_LIMIT)) {
		Dee_dec_addr32_t addrof_modrel; /* Start address for relocation tables pointed to by "Dec_Dhdr" entries. */
		Dee_dec_addr32_t addrof_modstr; /* Start address of `d_offsetof_modname' string table (possibly unaligned) */
		size_t dep_index;
		Dee_dec_addr32_t addrof_zero;
		union Dee_module_buildid const *deemon_buildid;

		/* Calculate the total needed buffer size. */
		total_need = self->dw_used;
		addrof_zero = (Dee_dec_addr32_t)(total_need + offsetof(struct Dee_heaptail, ht_zero));
		total_need += sizeof(struct Dee_heaptail); /* Heap tail */

		/* Produce a dec file that includes relocation info */
		ehdr->e_type = Dee_DEC_TYPE_RELOC;
		ehdr->e_typedata.td_reloc.er_offsetof_gchead = (Dee_dec_addr32_t)self->dw_gchead;
		ehdr->e_typedata.td_reloc.er_offsetof_gctail = (Dee_dec_addr32_t)self->dw_gctail;
		ehdr->e_typedata.td_reloc.er_offsetof_heap   = offsetof(Dec_Ehdr, e_heap);
		ehdr->e_typedata.td_reloc.er_sizeof_pointer  = sizeof(void *);
		ehdr->e_typedata.td_reloc.er_endian          = Dee_DEC_ENDIAN;
		deemon_buildid = DeeModule_GetBuildId(&DeeModule_Deemon);
		if unlikely(!deemon_buildid)
			goto err;
		ehdr->e_typedata.td_reloc.er_deemon_build_id[0] = deemon_buildid->mbi_word64[0];
		ehdr->e_typedata.td_reloc.er_deemon_build_id[1] = deemon_buildid->mbi_word64[1];

		/* Generate `ddm_impstr' strings for dependencies. */
		if unlikely(decwriter_genimpstr(self, context_absname, context_absname_size, flags))
			goto err;

		/* Module dependency tables. */
		if (self->dw_deps.ddpt_depc) {
			ASSERT(IS_ALIGNED(total_need, Dee_ALIGNOF_DEC_DHDR));
			ehdr->e_typedata.td_reloc.er_offsetof_deps = (Dee_dec_addr32_t)total_need;
			total_need += self->dw_deps.ddpt_depc * sizeof(Dec_Dhdr);
			/* Add space for "terminated by a d_modspec.d_mod==NULL-entry" */
			total_need += offsetafter(Dec_Dhdr, d_modspec.d_mod);
		} else {
			ehdr->e_typedata.td_reloc.er_offsetof_deps = addrof_zero - offsetof(Dec_Dhdr, d_modspec.d_mod);
		}

		/* Relocations against self. */
		if (self->dw_srel.drlt_relc) {
			ASSERT(IS_ALIGNED(total_need, Dee_ALIGNOF_DEC_REL));
			ehdr->e_typedata.td_reloc.er_offsetof_srel = (Dee_dec_addr32_t)total_need;
			total_need += self->dw_srel.drlt_relc * sizeof(Dec_Rel);
			total_need += offsetafter(Dec_Rel, r_addr);
		} else {
			ehdr->e_typedata.td_reloc.er_offsetof_srel = addrof_zero;
		}

		/* Relocations against deemon. */
		if (self->dw_drel.drlt_relc) {
			ehdr->e_typedata.td_reloc.er_offsetof_drel = (Dee_dec_addr32_t)total_need;
			total_need += self->dw_drel.drlt_relc * sizeof(Dec_Rel);
			total_need += offsetafter(Dec_Rel, r_addr);
		} else {
			ehdr->e_typedata.td_reloc.er_offsetof_drel = addrof_zero;
		}
		if (self->dw_drrel.drrt_relc) {
			ehdr->e_typedata.td_reloc.er_offsetof_drrel = (Dee_dec_addr32_t)total_need;
			total_need += self->dw_drrel.drrt_relc * sizeof(Dec_RRel);
			total_need += offsetafter(Dec_RRel, r_addr);
		} else {
			ehdr->e_typedata.td_reloc.er_offsetof_drrel = addrof_zero;
		}
		if (self->dw_drrela.drat_relc) {
			ehdr->e_typedata.td_reloc.er_offsetof_drrela = (Dee_dec_addr32_t)total_need;
			total_need += self->dw_drrela.drat_relc * sizeof(Dec_RRela);
			total_need += offsetafter(Dec_RRela, r_addr);
		} else {
			ehdr->e_typedata.td_reloc.er_offsetof_drrela = addrof_zero;
		}

		/* Relocation tables for dependencies. */
		addrof_modrel = (Dee_dec_addr32_t)total_need;
		for (dep_index = 0; dep_index < self->dw_deps.ddpt_depc; ++dep_index) {
			struct Dee_dec_depmod *dep = &self->dw_deps.ddpt_depv[dep_index];
			if (dep->ddm_rel.drlt_relc) {
				ASSERT(IS_ALIGNED(total_need, Dee_ALIGNOF_DEC_REL));
				total_need += dep->ddm_rel.drlt_relc * sizeof(Dec_Rel);
				total_need += offsetafter(Dec_Rel, r_addr);
			}
			if (dep->ddm_rrel.drrt_relc) {
				ASSERT(IS_ALIGNED(total_need, Dee_ALIGNOF_DEC_RREL));
				total_need += dep->ddm_rrel.drrt_relc * sizeof(Dec_RRel);
				total_need += offsetafter(Dec_RRel, r_addr);
			}
			if (dep->ddm_rrela.drat_relc) {
				ASSERT(IS_ALIGNED(total_need, Dee_ALIGNOF_DEC_RRELA));
				total_need += dep->ddm_rrela.drat_relc * sizeof(Dec_RRela);
				total_need += offsetafter(Dec_RRela, r_addr);
			}
		}

		/* String tables */
		addrof_modstr = (Dee_dec_addr32_t)total_need;
		for (dep_index = 0; dep_index < self->dw_deps.ddpt_depc; ++dep_index) {
			struct Dee_dec_depmod *dep = &self->dw_deps.ddpt_depv[dep_index];
			char const *impstr_utf8;
			ASSERT(dep->ddm_impstr);
			total_need = CEIL_ALIGN(total_need, Dee_ALIGNOF_DEC_DSTR);
			total_need += offsetof(Dec_Dstr, ds_string);
			impstr_utf8 = DeeString_AsUtf8(dep->ddm_impstr);
			if unlikely(!impstr_utf8)
				goto err;
			total_need += (WSTR_LENGTH(impstr_utf8) + 1) * sizeof(char);
		}

		/* Additional file dependencies */
		if (self->dw_fdeps.dfdt_depc) {
			total_need = CEIL_ALIGN(total_need, Dee_ALIGNOF_DEC_DSTR);
			ehdr->e_typedata.td_reloc.er_offsetof_files = (Dee_dec_addr32_t)total_need;
			total_need += self->dw_fdeps.dfdt_depc;
			total_need = CEIL_ALIGN(total_need, Dee_ALIGNOF_DEC_DSTR);
			total_need += (Dee_dec_addr32_t)offsetafter(Dec_Dstr, ds_length); /* For trailing 0 */
		} else {
			ehdr->e_typedata.td_reloc.er_offsetof_files = 0;
		}

		/* Extended relocations aren't used (yet) */
		ehdr->e_typedata.td_reloc.er_offsetof_xrel = 0;

		/* Alignment is important for embedded slab allocators */
		ehdr->e_typedata.td_reloc.er_alignment = seraddr32(self->dw_align);

		/* Assert that the image won't be too large */
		if unlikely(total_need > DFILE_LIMIT)
			goto output_image;

		/* EOF marker */
		ehdr->e_typedata.td_reloc.er_offsetof_eof = (Dee_dec_addr32_t)total_need;

		/* Resize the main buffer to fit (and satisfy extended alignment requirements if there are any) */
		if (self->dw_align > Dee_HEAPCHUNK_ALIGN) {
			Dec_Ehdr *final_ehdr = (Dec_Ehdr *)Dee_Memalign(self->dw_align, total_need);
			if unlikely(!final_ehdr)
				goto err;
			ASSERT(total_need >= (self->dw_used + sizeof(struct Dee_heaptail)));
			final_ehdr = (Dec_Ehdr *)memcpy(final_ehdr, ehdr, self->dw_used + sizeof(struct Dee_heaptail));
			Dee_Free(ehdr);
			ehdr = final_ehdr;
		} else {
			ehdr = (Dec_Ehdr *)Dee_Realloc(ehdr, total_need);
			if unlikely(!ehdr)
				goto err;
		}
		self->dw_ehdr = ehdr;

		/* With the buffer resized to its final size, and offsets all determined, copy data. */

		/* Module dependency tables. */
		if (self->dw_deps.ddpt_depc) {
			Dec_Dstr *out_name;
			Dec_Dhdr *out_dep = (Dec_Dhdr *)((byte_t *)ehdr + ehdr->e_typedata.td_reloc.er_offsetof_deps);
			Dee_dec_addr32_t addrof_outname = addrof_modstr;
			Dee_dec_addr32_t addrof_outrel  = addrof_modrel;
			for (dep_index = 0; dep_index < self->dw_deps.ddpt_depc; ++dep_index, ++out_dep) {
				struct Dee_dec_depmod *dep = &self->dw_deps.ddpt_depv[dep_index];
				char const *impstr_utf8;
				union Dee_module_buildid const *dep_buildid;

				/* Remember correct build ID of dependency */
				dep_buildid = DeeModule_GetBuildId(dep->ddm_mod);
				if unlikely(!dep_buildid)
					goto err;
				out_dep->d_buildid[0] = dep_buildid->mbi_word64[0];
				out_dep->d_buildid[1] = dep_buildid->mbi_word64[1];

				/* Emit relocations (noref) */
				if (dep->ddm_rel.drlt_relc) {
					Dec_Rel *out_rel = (Dec_Rel *)((byte_t *)ehdr + addrof_outrel);
					out_dep->d_modspec.d_file.d_offsetof_rel = addrof_outrel;
					out_rel = (Dec_Rel *)mempcpyc(out_rel, dep->ddm_rel.drlt_relv,
					                              dep->ddm_rel.drlt_relc,
					                              sizeof(Dec_Rel));
					out_rel->r_addr = 0;
					addrof_outrel += seraddr32(dep->ddm_rel.drlt_relc * sizeof(Dec_Rel));
					addrof_outrel += (Dee_dec_addr32_t)offsetafter(Dec_Rel, r_addr);
				} else {
					out_dep->d_modspec.d_file.d_offsetof_rel = addrof_zero;
				}

				/* Emit relocations (ref) */
				if (dep->ddm_rrel.drrt_relc) {
					Dec_RRel *out_rel = (Dec_RRel *)((byte_t *)ehdr + addrof_outrel);
					out_dep->d_offsetof_rrel = addrof_outrel;
					out_rel = (Dec_RRel *)mempcpyc(out_rel, dep->ddm_rrel.drrt_relv,
					                               dep->ddm_rrel.drrt_relc,
					                               sizeof(Dec_RRel));
					out_rel->r_addr = 0;
					addrof_outrel += seraddr32(dep->ddm_rrel.drrt_relc * sizeof(Dec_RRel));
					addrof_outrel += (Dee_dec_addr32_t)offsetafter(Dec_RRel, r_addr);
				} else {
					out_dep->d_offsetof_rrel = addrof_zero;
				}

				/* Emit relocations (ref+addend) */
				if (dep->ddm_rrela.drat_relc) {
					Dec_RRela *out_rel = (Dec_RRela *)((byte_t *)ehdr + addrof_outrel);
					out_dep->d_offsetof_rrela = addrof_outrel;
					out_rel = (Dec_RRela *)mempcpyc(out_rel, dep->ddm_rrela.drat_relv,
					                                dep->ddm_rrela.drat_relc,
					                                sizeof(Dec_RRela));
					out_rel->r_addr = 0;
					addrof_outrel += seraddr32(dep->ddm_rrela.drat_relc * sizeof(Dec_RRela));
					addrof_outrel += (Dee_dec_addr32_t)offsetafter(Dec_RRela, r_addr);
				} else {
					out_dep->d_offsetof_rrela = addrof_zero;
				}

				/* Emit module name */
				impstr_utf8 = DeeString_AsUtf8(dep->ddm_impstr);
				ASSERTF(impstr_utf8, "Should have been pre-loaded since was needed to calc buffer size");
				addrof_outname = CEIL_ALIGN(addrof_outname, Dee_ALIGNOF_DEC_DSTR);
				out_name = (Dec_Dstr *)((byte_t *)ehdr + addrof_outname);
				out_name->ds_length = seraddr32(WSTR_LENGTH(impstr_utf8));
				*(char *)mempcpyc(out_name->ds_string, impstr_utf8, out_name->ds_length, sizeof(char)) = '\0';
				out_dep->d_modspec.d_file.d_offsetof_modname = addrof_outname;
				addrof_outname += offsetof(Dec_Dstr, ds_string);
				addrof_outname += (out_name->ds_length + 1) * sizeof(char);
			}

			/* Emit terminating "d_modspec.d_mod==NULL"-entry */
			out_dep->d_modspec.d_mod = NULL;
		}

		/* Relocations against self. */
		if (self->dw_srel.drlt_relc) {
			Dec_Rel *out_rel = (Dec_Rel *)((byte_t *)ehdr + ehdr->e_typedata.td_reloc.er_offsetof_srel);
			out_rel = (Dec_Rel *)mempcpyc(out_rel, self->dw_srel.drlt_relv,
			                              self->dw_srel.drlt_relc,
			                              sizeof(Dec_Rel));
			out_rel->r_addr = 0;
		}

		/* Relocations against deemon. */
		if (self->dw_drel.drlt_relc) {
			Dec_Rel *out_rel = (Dec_Rel *)((byte_t *)ehdr + ehdr->e_typedata.td_reloc.er_offsetof_drel);
			out_rel = (Dec_Rel *)mempcpyc(out_rel, self->dw_drel.drlt_relv,
			                              self->dw_drel.drlt_relc,
			                              sizeof(Dec_Rel));
			out_rel->r_addr = 0;
		}
		if (self->dw_drrel.drrt_relc) {
			Dec_RRel *out_rel = (Dec_RRel *)((byte_t *)ehdr + ehdr->e_typedata.td_reloc.er_offsetof_drrel);
			out_rel = (Dec_RRel *)mempcpyc(out_rel, self->dw_drrel.drrt_relv,
			                               self->dw_drrel.drrt_relc,
			                               sizeof(Dec_RRel));
			out_rel->r_addr = 0;
		}
		if (self->dw_drrela.drat_relc) {
			Dec_RRela *out_rel = (Dec_RRela *)((byte_t *)ehdr + ehdr->e_typedata.td_reloc.er_offsetof_drrela);
			out_rel = (Dec_RRela *)mempcpyc(out_rel, self->dw_drrela.drat_relv,
			                                self->dw_drrela.drat_relc,
			                                sizeof(Dec_RRela));
			out_rel->r_addr = 0;
		}

		/* Additional file dependencies */
		if (self->dw_fdeps.dfdt_depc) {
			byte_t *out_deps = (byte_t *)ehdr + ehdr->e_typedata.td_reloc.er_offsetof_files;
			memcpy(out_deps, self->dw_fdeps.dfdt_depv, self->dw_fdeps.dfdt_depc);
			out_deps += CEIL_ALIGN(self->dw_fdeps.dfdt_depc, Dee_ALIGNOF_DEC_DSTR);
			((Dec_Dstr *)out_deps)->ds_length = 0; /* For trailing 0 */
		}

		/* Generate a build ID for the dec file */
		{
			DeeMD5_Context md5;
			DeeModuleObject *mod = DeeDec_Ehdr_GetModule(ehdr);
			ehdr->e_typedata.td_reloc.er_build_timestamp = 0;
			ehdr->e_heap.hr_destroy = NULL;
			bzero(&ehdr->e_mapping, sizeof(ehdr->e_mapping));
#if ((DeeDec_Ehdr_OFFSETOF__e_mapping + Dee_SIZEOF_DeeMapFile) % Dee_HEAPCHUNK_ALIGN) != 0
			memset(ehdr->_e_heap_pad, 0xfe, sizeof(ehdr->_e_heap_pad)); /* Padding byte */
#endif /* (Dee_SIZEOF_DeeMapFile % Dee_HEAPCHUNK_ALIGN) != 0 */
			DeeMD5_Init(&md5);
			DeeMD5_Update(&md5, ehdr, ehdr->e_typedata.td_reloc.er_offsetof_eof);
			DeeMD5_Finalize(&md5, mod->mo_buildid.mbi_word32);
		}

		/* Fill in the build timestamp */
		ehdr->e_typedata.td_reloc.er_build_timestamp = DeeSystem_GetWalltime();
	} else
#endif /* !CONFIG_NO_DEC */
	{
#ifndef CONFIG_NO_DEC
output_image:
#endif /* !CONFIG_NO_DEC */
		/* Calculate the total needed buffer size. */
		total_need = self->dw_used;
		total_need += sizeof(struct Dee_heaptail); /* Heap tail */

		/* Do less when we don't need to generate a .dec file */
		DBG_memset(&ehdr->e_typedata, 0xcc, sizeof(ehdr->e_typedata));

		/* Produce a dec file that is only the heap image,
		 * with space for temporary relocation vectors. */
		ehdr->e_type = Dee_DEC_TYPE_IMAGE;

		/* Resize the main buffer to fit (and satisfy extended alignment requirements if there are any) */
		if (self->dw_align > Dee_HEAPCHUNK_ALIGN) {
			Dec_Ehdr *final_ehdr = (Dec_Ehdr *)Dee_Memalign(self->dw_align, total_need);
			if unlikely(!final_ehdr)
				goto err;
			ASSERT(total_need >= (self->dw_used + sizeof(struct Dee_heaptail)));
			final_ehdr = (Dec_Ehdr *)memcpy(final_ehdr, ehdr, self->dw_used + sizeof(struct Dee_heaptail));
			Dee_Free(ehdr);
			ehdr = final_ehdr;
		} else {
			ehdr = (Dec_Ehdr *)Dee_Realloc(ehdr, total_need);
			if unlikely(!ehdr)
				goto err;
		}
		self->dw_ehdr = ehdr;

		/* NOTE: Leave the zero-initialized build-id, as well as
		 *       the 'Dee_MODULE_FHASBUILDID' flag in "mod" as-is.
		 * The module will just return `0' as its `__buildid__'. */
	}

	/* Steal the ehdr from `self' */
	self->dw_ehdr  = NULL;
	self->dw_used  = 0;
	self->dw_alloc = 0;
	DBG_memset(&self->dw_slabb, 0xcc, sizeof(self->dw_slabb));
	self->dw_slabs = 0;

	/* Finish initialization of "ehdr" by setting it up as a HEAP mapping */
	DeeMapFile_SETADDR(&ehdr->e_mapping, ehdr);
	DeeMapFile_SETSIZE(&ehdr->e_mapping, total_need);
	DeeMapFile_SETHEAP(&ehdr->e_mapping);

	/* Finish initialization of the heap region */
	ehdr->e_heap.hr_destroy = &DeeDec_heapregion_destroy;

	/* Return the fully initialized DEC exec header. */
	return ehdr;
err:
	return NULL;
}







/************************************************************************/
/************************************************************************/
/*                                                                      */
/* DEC FILE GENERATION                                                  */
/*                                                                      */
/************************************************************************/
/************************************************************************/

/* Append a relocation to "self" */
PRIVATE WUNUSED NONNULL((1)) int DCALL
reltab_append(struct Dee_dec_reltab *__restrict self, Dee_dec_addr32_t addr) {
	Dec_Rel *rel;
	ASSERT(self->drlt_relc <= self->drlt_rela);
	if (self->drlt_relc >= self->drlt_rela) {
		Dec_Rel *new_relv;
		size_t new_alloc = self->drlt_rela * 2;
		if (new_alloc < 16)
			new_alloc = 16;
		new_relv = (Dec_Rel *)Dee_TryReallocc(self->drlt_relv, new_alloc, sizeof(Dec_Rel));
		if unlikely(!new_relv) {
			new_alloc = self->drlt_relc + 1;
			new_relv  = (Dec_Rel *)Dee_Reallocc(self->drlt_relv, new_alloc, sizeof(Dec_Rel));
			if unlikely(!new_relv)
				goto err;
		}
		self->drlt_relv = new_relv;
		self->drlt_rela = new_alloc;
	}
	rel = &self->drlt_relv[self->drlt_relc++];
	rel->r_addr = addr;
	return 0;
err:
	return -1;
}

#if 1
STATIC_ASSERT(sizeof(Dec_RRel) == sizeof(Dec_Rel));
STATIC_ASSERT(offsetof(Dec_RRel, r_addr) == offsetof(Dec_Rel, r_addr));
STATIC_ASSERT(offsetafter(Dec_RRel, r_addr) == offsetafter(Dec_Rel, r_addr));
STATIC_ASSERT(sizeof(struct Dee_dec_rreltab) == sizeof(struct Dee_dec_reltab));
STATIC_ASSERT(offsetof(struct Dee_dec_rreltab, drrt_relv) == offsetof(struct Dee_dec_reltab, drlt_relv));
STATIC_ASSERT(offsetof(struct Dee_dec_rreltab, drrt_relc) == offsetof(struct Dee_dec_reltab, drlt_relc));
STATIC_ASSERT(offsetof(struct Dee_dec_rreltab, drrt_rela) == offsetof(struct Dee_dec_reltab, drlt_rela));
STATIC_ASSERT(offsetafter(struct Dee_dec_rreltab, drrt_relv) == offsetafter(struct Dee_dec_reltab, drlt_relv));
STATIC_ASSERT(offsetafter(struct Dee_dec_rreltab, drrt_relc) == offsetafter(struct Dee_dec_reltab, drlt_relc));
STATIC_ASSERT(offsetafter(struct Dee_dec_rreltab, drrt_rela) == offsetafter(struct Dee_dec_reltab, drlt_rela));
#ifdef __INTELLISENSE__
PRIVATE WUNUSED NONNULL((1)) int DCALL
rreltab_append(struct Dee_dec_rreltab *__restrict self, Dee_dec_addr32_t addr);
#else /* __INTELLISENSE__ */
#define rreltab_append(self, addr) reltab_append((struct Dee_dec_reltab *)(self), addr)
#endif /* !__INTELLISENSE__ */
#else
PRIVATE WUNUSED NONNULL((1)) int DCALL
rreltab_append(struct Dee_dec_rreltab *__restrict self, Dee_dec_addr32_t addr) {
	Dec_RRel *rel;
	ASSERT(self->drrt_relc <= self->drrt_rela);
	if (self->drrt_relc >= self->drrt_rela) {
		Dec_RRel *new_relv;
		size_t new_alloc = self->drrt_rela * 2;
		if (new_alloc < 16)
			new_alloc = 16;
		new_relv = (Dec_RRel *)Dee_TryReallocc(self->drrt_relv, new_alloc, sizeof(Dec_RRel));
		if unlikely(!new_relv) {
			new_alloc = self->drrt_relc + 1;
			new_relv  = (Dec_RRel *)Dee_Reallocc(self->drrt_relv, new_alloc, sizeof(Dec_RRel));
			if unlikely(!new_relv)
				goto err;
		}
		self->drrt_relv = new_relv;
		self->drrt_rela = new_alloc;
	}
	rel = &self->drrt_relv[self->drrt_relc++];
	rel->r_addr = addr;
	return 0;
err:
	return -1;
}
#endif

PRIVATE WUNUSED NONNULL((1)) int DCALL
rrelatab_append(struct Dee_dec_rrelatab *__restrict self,
                Dee_dec_addr32_t addr, Dee_dec_off32_t offs) {
	Dec_RRela *rel;
	ASSERT(self->drat_relc <= self->drat_rela);
	if (self->drat_relc >= self->drat_rela) {
		Dec_RRela *new_relv;
		size_t new_alloc = self->drat_rela * 2;
		if (new_alloc < 16)
			new_alloc = 16;
		new_relv = (Dec_RRela *)Dee_TryReallocc(self->drat_relv, new_alloc, sizeof(Dec_RRela));
		if unlikely(!new_relv) {
			new_alloc = self->drat_relc + 1;
			new_relv  = (Dec_RRela *)Dee_Reallocc(self->drat_relv, new_alloc, sizeof(Dec_RRela));
			if unlikely(!new_relv)
				goto err;
		}
		self->drat_relv = new_relv;
		self->drat_rela = new_alloc;
	}
	rel = &self->drat_relv[self->drat_relc++];
	rel->r_addr = addr;
	rel->r_offs = offs;
	return 0;
err:
	return -1;
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
	size_t old_size = CEIL_ALIGN(self->dw_fdeps.dfdt_depc, Dee_ALIGNOF_DEC_DSTR);
	size_t min_size = old_size + offsetof(Dec_Dstr, ds_string) +
	                  (filename_len + 1) * sizeof(char);
	if unlikely(filename_len > DFILE_LIMIT)
		goto err_string_too_large;
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
	dst->ds_length = seraddr32(filename_len); /* Length was checked against 'DFILE_LIMIT' above */
	*(char *)mempcpyc(dst->ds_string, filename, filename_len, sizeof(char)) = '\0';
	self->dw_fdeps.dfdt_depc = min_size;
	return 0;
err_string_too_large:
	Dee_BadAlloc(filename_len);
err:
	return -1;
}


/* Emit a relocation:
 * >> *DeeDecWriter_Addr2Mem(self, addrof_pointer, void *) =
 * >>     DeeDecWriter_Addr2Mem(self, addrof_target, void);
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1)) int DCALL
decwriter_putaddr(DeeDecWriter *__restrict self,
                  Dee_seraddr_t addrof_pointer,
                  Dee_seraddr_t addrof_target) {
	void **pointer = DeeDecWriter_Addr2Mem(self, addrof_pointer, void *);
	/* Special case: self-relocations are relative to the start of the EHDR,
	 *               rather than the dec file's module object. Even though
	 *               this differs from what is done by other relocations,
	 *               it does make this case here, and applying relocations
	 *               easier. */
	*pointer = (void *)(uintptr_t)addrof_target;
	/* XXX: Support for large (>4GiB) relocation targets on 64-bit hosts */
	return reltab_append(&self->dw_srel, seraddr32(addrof_pointer));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
decwriter_resize_deps(DeeDecWriter *__restrict self) {
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
decwriter_getdep(DeeDecWriter *__restrict self,
                 DeeModuleObject *__restrict mod) {
	size_t index;
	struct Dee_dec_depmod *dst;
	BSEARCH (index, self->dw_deps.ddpt_depv, self->dw_deps.ddpt_depc, .ddm_mod, mod) {
		return &self->dw_deps.ddpt_depv[index];
	}
	if unlikely(decwriter_resize_deps(self))
		goto err;
	dst = &self->dw_deps.ddpt_depv[index];
	memmoveupc(dst + 1, dst, self->dw_deps.ddpt_depc - index, sizeof(*dst));
	++self->dw_deps.ddpt_depc;
	Dee_Incref(mod);
	Dee_dec_depmod_init(dst, mod);
	if unlikely(atomic_read(&mod->mo_flags) & Dee_MODULE_FNOSERIAL) {
		Dee_DPRINTF("[LD][dec] Warning: dependent module %q could not be "
		            /**/ "serialized, meaning this one can't be either\n",
		            DeeModule_GetAbsName(mod));
		if (self->dw_flags & DeeDecWriter_F_FRELOC) {
			DeeRT_ErrCannotSerialize(mod);
			goto err;
		}
		self->dw_flags |= DeeDecWriter_F_NRELOC;
	}
	return dst;
err:
	return NULL;
}


/* Remember that `obj' has been serialized at offset `addr' (s.a. `self->dw_known')
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
decwriter_addknown(DeeDecWriter *__restrict self,
                   void const *__restrict ptr,
                   Dee_seraddr_t addr, size_t size,
                   bool do_try) {
	size_t lo = 0, hi = self->dw_known.dpt_ptrc;
	struct Dee_dec_ptrtab_entry *vec = self->dw_known.dpt_ptrv;
	if (hi >= self->dw_known.dpt_ptra) {
		size_t new_alloc = self->dw_known.dpt_ptra * 2;
		if (new_alloc < 16)
			new_alloc = 16;
		if (new_alloc < hi + 1)
			new_alloc = hi + 1;
		vec = (struct Dee_dec_ptrtab_entry *)Dee_TryReallocc(vec, new_alloc, sizeof(struct Dee_dec_ptrtab_entry));
		if unlikely(!vec) {
			new_alloc = hi + 1;
			vec = (struct Dee_dec_ptrtab_entry *)Dee_TryReallocc(self->dw_known.dpt_ptrv, new_alloc, sizeof(struct Dee_dec_ptrtab_entry));
			if unlikely(!vec) {
				if (do_try)
					goto err;
				vec = (struct Dee_dec_ptrtab_entry *)Dee_Reallocc(self->dw_known.dpt_ptrv, new_alloc, sizeof(struct Dee_dec_ptrtab_entry));
				if unlikely(!vec)
					goto err;
			}
		}
		self->dw_known.dpt_ptrv = vec;
		self->dw_known.dpt_ptra = new_alloc;
		hi = self->dw_known.dpt_ptrc;
	}
	while (lo < hi) {
		size_t mid = (lo + hi) / 2;
		struct Dee_dec_ptrtab_entry *ent = &vec[mid];
		if ((byte_t *)ptr < (byte_t *)ent->dpte_ptr) {
			hi = mid;
		} else {
			ASSERTF((byte_t *)ptr > (byte_t *)ent->dpte_ptr,
			        "Reference to %p at %#" PRFxSIZ "-%#" PRFxSIZ " was already "
			        "serialized into dec file, and API design should have prevented "
			        "a second copy from being serialized again (the first instance "
			        "should have been re-used)",
			        ptr, (size_t)ent->dote_off, (size_t)ent->dote_off + ent->dote_siz - 1);
			lo = mid + 1;
		}
	}

	ASSERT(lo == hi);
	ASSERT(lo <= self->dw_known.dpt_ptrc);
	vec += lo;
	hi = self->dw_known.dpt_ptrc - lo;
	/* XXX: This "memmoveupc" has worst-case O(N) and average case O(N/2)=O(N) run
	 *      time. Figure out some way where the average case runs faster than that! */
	memmoveupc(vec + 1, vec, hi, sizeof(struct Dee_dec_ptrtab_entry));
	++self->dw_known.dpt_ptrc;
	vec->dpte_ptr = ptr;
	vec->dote_off = addr;
	vec->dote_siz = size;
	return 0;
err:
	return -1;
}


/* Possible values for `decwriter_malloc_impl::flags' */
#define decwriter_malloc_impl_F_NORMAL 0x0000 /* Normal flags */
#define decwriter_malloc_impl_F_TRY    0x0001 /* Do try-malloc semantics */
#define decwriter_malloc_impl_F_BZERO  0x0002 /* Zero-initialize result */

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
decwriter_malloc_impl(DeeDecWriter *__restrict self, size_t num_bytes,
                      void const *ref, unsigned int flags) {
	void *payload;
	Dee_seraddr_t result;
	struct Dee_heapchunk *chunk;
	size_t cur_avail, nb, req_avail, unused;
	if unlikely(num_bytes > DFILE_LIMIT)
		goto err_too_much;

	/* Force alignment of new chunk size */
	unused = CEIL_ALIGN(num_bytes, Dee_HEAPCHUNK_ALIGN) - num_bytes;
	num_bytes += unused;
	nb = sizeof(struct Dee_heapchunk) + num_bytes;
	if (self->dw_slabs) {
		ASSERT(self->dw_alloc >= (self->dw_slabb + self->dw_slabs + sizeof(struct Dee_heaptail)));
		if (self->dw_used <= self->dw_slabb) {
			struct Dee_heapchunk *slab_chunk;
			size_t avail_before_slab = (self->dw_slabb - self->dw_used);
			if (avail_before_slab >= nb)
				goto do_allocate; /* Can just allocate ahead of slab page! */
			if (avail_before_slab != 0) {
				/* Extend the previous heap-chunk to go up to the slab page. */
				Dee_seraddr_t addrof_prev_heapchunk;
				struct Dee_heapchunk *prev_chunk;
				ASSERTF(self->dw_hlast != 0, "The first thing that's allocated should "
				                             "already be regular heap memory, so there "
				                             "should always be a preceding allocation");
				ASSERT(self->dw_used >= self->dw_hlast);
				addrof_prev_heapchunk = self->dw_used;
				addrof_prev_heapchunk -= sizeof(struct Dee_heapchunk);
				addrof_prev_heapchunk -= (self->dw_hlast - Dee_HEAPCHUNK_PREV(0));
				prev_chunk = DeeDecWriter_Addr2Mem(self, addrof_prev_heapchunk, struct Dee_heapchunk);
				prev_chunk->hc_head += avail_before_slab;
				memset(DeeDecWriter_Addr2Mem(self, self->dw_used, void), 0xfe, avail_before_slab);
				self->dw_hlast += avail_before_slab;
				self->dw_used += avail_before_slab;
			}
			ASSERT(self->dw_used == self->dw_slabb);

			/* Fill in the heap-chunk descriptor for the slab area */
			slab_chunk = DeeDecWriter_Addr2Mem(self, self->dw_used, struct Dee_heapchunk);
			slab_chunk->hc_prevsize = self->dw_hlast;

			/* Must continue after slab page(s) */
			self->dw_used += self->dw_slabs;
			self->dw_hlast = Dee_HEAPCHUNK_PREV(self->dw_slabs - sizeof(struct Dee_heapchunk));
		}
		ASSERT(self->dw_used >= (self->dw_slabb + self->dw_slabs));
	}

	/* Ensure that enough space has been allocated.
	 * Always include space for the eventual `struct Dee_heaptail'! */
	ASSERT(self->dw_alloc >= self->dw_used);
	cur_avail = self->dw_alloc - self->dw_used;
	req_avail = nb + sizeof(struct Dee_heaptail);
	if likely(cur_avail < req_avail) {
		byte_t *new_base;
		size_t min_alloc = (self->dw_used + req_avail);
		size_t new_alloc = CEIL_ALIGN(min_alloc * 2, __SIZEOF_POINTER__ * 4 * 1024);
		if (new_alloc < min_alloc)
			new_alloc = min_alloc;
		new_base = (byte_t *)Dee_TryRealloc(self->dw_base, new_alloc);
		if unlikely(!new_base) {
			new_alloc = min_alloc;
			new_base = (flags & decwriter_malloc_impl_F_TRY)
			           ? (byte_t *)Dee_TryRealloc(self->dw_base, new_alloc)
			           : (byte_t *)Dee_Realloc(self->dw_base, new_alloc);
			if unlikely(!new_base)
				goto err;
		}
		self->dw_base  = new_base;
		self->dw_alloc = Dee_MallocUsableSizeNonNull(new_base);
		ASSERT(self->dw_alloc >= new_alloc);
	}

	/* Initialize the heap chunk for the newly made allocation */
do_allocate:
	result = self->dw_used;
	ASSERT(IS_ALIGNED(result, Dee_HEAPCHUNK_ALIGN));
	ASSERT(IS_ALIGNED(nb, Dee_HEAPCHUNK_ALIGN));
	chunk = DeeDecWriter_Addr2Mem(self, result, struct Dee_heapchunk);
	chunk->hc_prevsize = self->dw_hlast; /* Must be 0 the first time around */
	chunk->hc_head     = Dee_HEAPCHUNK_HEAD(num_bytes);
	payload = chunk + 1;
	if (flags & decwriter_malloc_impl_F_BZERO) {
		bzero(payload, num_bytes);
	} else {
		/* Pre-initialize heap user-data to FD bytes (to prevent
		 * uninitialized bytes from appearing in ".dec" files)
		 *
		 * If it is known that no ".dec" file can/will be created,
		 * then this step is simply skipped. */
		if (!(self->dw_flags & DeeDecWriter_F_NRELOC))
			memset(payload, 0xfd, num_bytes);
	}

	/* Fill in padding tail area with FE bytes */
	memset((byte_t *)payload + num_bytes - unused, 0xfe, unused);
	if (ref) {
		if unlikely(decwriter_addknown(self, ref,
		                               result + sizeof(struct Dee_heapchunk), num_bytes,
		                               (flags & decwriter_malloc_impl_F_TRY) != 0))
			goto err;
	}
	self->dw_used  = result + nb;
	self->dw_hlast = Dee_HEAPCHUNK_PREV(num_bytes); /* Override with whatever the next chunk will need */
	result += sizeof(struct Dee_heapchunk);
	return result;
err_too_much:
	if (!(flags & decwriter_malloc_impl_F_TRY))
		Dee_BadAlloc(num_bytes);
err:
	return Dee_SERADDR_INVALID;
}

/* Free a heap pointer
 * CAUTION: Only the most-recent pointer can *actually* be free'd!
 *          If you pass anything else, this function is a no-op! */
PRIVATE NONNULL((1)) void DCALL
decwriter_free(DeeDecWriter *__restrict self, Dee_seraddr_t addr, void const *ref) {
	size_t last_bytes = self->dw_hlast - Dee_HEAPCHUNK_PREV(0);
	Dee_seraddr_t last = self->dw_used - last_bytes;
	struct Dee_heapchunk *p;
	if (last != addr)
		return; /* Cannot free... */

	if (ref) {
		/* TODO: decwriter_delknown() */
	}

	/* Undo state changes made by allocation */
	addr -= sizeof(struct Dee_heapchunk);
	p = DeeDecWriter_Addr2Mem(self, addr, struct Dee_heapchunk);
	self->dw_used  = addr;
	self->dw_hlast = p->hc_prevsize;
}

/* Encode a reference to `obj' at `DeeDecWriter_Addr2Mem(self, addr, DeeObject)'
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
decwriter_putobject(DeeDecWriter *__restrict self,
                    Dee_seraddr_t addr,
                    DeeObject *__restrict obj);

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_object_malloc_impl(DeeDecWriter *__restrict self, size_t num_bytes,
                             DeeObject *__restrict ref, unsigned int flags) {
	DeeObject *copy;
	Dee_seraddr_t result;
	ASSERTF(!DeeType_IsGC(Dee_TYPE(ref)), "Use decwriter_gcobject_malloc_impl()");
	result = decwriter_malloc_impl(self, num_bytes, ref, flags);
	if unlikely(!Dee_SERADDR_ISOK(result))
		goto err;

	/* Initialize "ob_refcnt" and "ob_type" of the newly allocated object */
	copy = DeeDecWriter_Addr2Mem(self, result, DeeObject);
	copy->ob_refcnt = 1;
#ifdef CONFIG_TRACE_REFCHANGES
	copy->ob_trace = NULL;
#endif /* CONFIG_TRACE_REFCHANGES */
	if (!DeeType_IsHeapType(Dee_TYPE(ref))) {
		if unlikely(decwriter_putpointer(self,
		                                 result + offsetof(DeeObject, ob_type),
		                                 Dee_AsObject(Dee_TYPE(ref))))
			goto err_r;
	} else {
		if unlikely(decwriter_putobject(self,
		                                result + offsetof(DeeObject, ob_type),
		                                Dee_AsObject(Dee_TYPE(ref))))
			goto err_r;
	}
	return result;
err_r:
	decwriter_free(self, result, ref);
err:
	return Dee_SERADDR_INVALID;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_gcobject_malloc_impl(DeeDecWriter *__restrict self, size_t num_bytes,
                               DeeObject *__restrict ref, unsigned int flags) {
	size_t total;
	Dee_seraddr_t result;
	DeeObject *copy;
	ASSERTF(DeeType_IsGC(Dee_TYPE(ref)), "Use decwriter_object_malloc_impl()");
	if (OVERFLOW_UADD(num_bytes, Dee_GC_OBJECT_OFFSET, &total))
		total = (size_t)-1;
	result = decwriter_malloc_impl(self, total, DeeGC_Head(ref), flags);
	if unlikely(!Dee_SERADDR_ISOK(result))
		goto err;

	/* Initialize GC head/tail link pointers */
	ASSERT((self->dw_gchead == 0) ==
	       (self->dw_gctail == 0));
	result += Dee_GC_OBJECT_OFFSET;
	if (self->dw_gctail == 0) {
		/* First GC object... */
		struct Dee_gc_head *link;
		self->dw_gchead = result;
		link = DeeDecWriter_Addr2Mem(self, result - Dee_GC_OBJECT_OFFSET,
		                             struct Dee_gc_head);
		link->gc_info.gi_pself = NULL;
		link->gc_next          = NULL;
	} else {
		/* Append to end of GC list. */
		if (decwriter_putaddr(self,
		                      (result - Dee_GC_OBJECT_OFFSET) +
		                      offsetof(struct Dee_gc_head, gc_info.gi_pself),
		                      (self->dw_gctail - Dee_GC_OBJECT_OFFSET) +
		                      offsetof(struct Dee_gc_head, gc_next)))
			goto err_r;
		if (decwriter_putaddr(self,
		                      (self->dw_gctail - Dee_GC_OBJECT_OFFSET) +
		                      offsetof(struct Dee_gc_head, gc_next),
		                      result))
			goto err_r;
	}
	self->dw_gctail = result;

	/* Initialize "ob_refcnt" and "ob_type" of the newly allocated object */
	copy = DeeDecWriter_Addr2Mem(self, result, DeeObject);
	copy->ob_refcnt = 1;
#ifdef CONFIG_TRACE_REFCHANGES
	copy->ob_trace = NULL;
#endif /* CONFIG_TRACE_REFCHANGES */
	if (!DeeType_IsHeapType(Dee_TYPE(ref))) {
		if unlikely(decwriter_putpointer(self,
		                                 result +
		                                 offsetof(DeeObject, ob_type),
		                                 Dee_AsObject(Dee_TYPE(ref))))
			goto err_r;
	} else {
		if unlikely(decwriter_putobject(self,
		                                result +
		                                offsetof(DeeObject, ob_type),
		                                Dee_AsObject(Dee_TYPE(ref))))
			goto err_r;
	}
	return result;
err_r:
	decwriter_free(self, result - Dee_GC_OBJECT_OFFSET, DeeGC_Head(ref));
err:
	return Dee_SERADDR_INVALID;
}

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
decwriter_malloc(DeeDecWriter *__restrict self, size_t num_bytes, void const *ref) {
	return decwriter_malloc_impl(self, num_bytes, ref,
	                             decwriter_malloc_impl_F_NORMAL);
}

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
decwriter_trymalloc(DeeDecWriter *__restrict self, size_t num_bytes, void const *ref) {
	return decwriter_malloc_impl(self, num_bytes, ref,
	                             decwriter_malloc_impl_F_TRY);
}

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
decwriter_calloc(DeeDecWriter *__restrict self, size_t num_bytes, void const *ref) {
	return decwriter_malloc_impl(self, num_bytes, ref,
	                             decwriter_malloc_impl_F_NORMAL |
	                             decwriter_malloc_impl_F_BZERO);
}

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
decwriter_trycalloc(DeeDecWriter *__restrict self, size_t num_bytes, void const *ref) {
	return decwriter_malloc_impl(self, num_bytes, ref,
	                             decwriter_malloc_impl_F_TRY |
	                             decwriter_malloc_impl_F_BZERO);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_object_malloc(DeeDecWriter *__restrict self, size_t num_bytes, DeeObject *__restrict ref) {
	return decwriter_object_malloc_impl(self, num_bytes, ref,
	                                    decwriter_malloc_impl_F_NORMAL);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_object_trymalloc(DeeDecWriter *__restrict self, size_t num_bytes, DeeObject *__restrict ref) {
	return decwriter_object_malloc_impl(self, num_bytes, ref,
	                                    decwriter_malloc_impl_F_TRY);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_object_calloc(DeeDecWriter *__restrict self, size_t num_bytes, DeeObject *__restrict ref) {
	return decwriter_object_malloc_impl(self, num_bytes, ref,
	                                    decwriter_malloc_impl_F_NORMAL |
	                                    decwriter_malloc_impl_F_BZERO);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_object_trycalloc(DeeDecWriter *__restrict self, size_t num_bytes, DeeObject *__restrict ref) {
	return decwriter_object_malloc_impl(self, num_bytes, ref,
	                                    decwriter_malloc_impl_F_TRY |
	                                    decwriter_malloc_impl_F_BZERO);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_gcobject_malloc(DeeDecWriter *__restrict self, size_t num_bytes, DeeObject *__restrict ref) {
	return decwriter_gcobject_malloc_impl(self, num_bytes, ref,
	                                      decwriter_malloc_impl_F_NORMAL);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_gcobject_trymalloc(DeeDecWriter *__restrict self, size_t num_bytes, DeeObject *__restrict ref) {
	return decwriter_gcobject_malloc_impl(self, num_bytes, ref,
	                                      decwriter_malloc_impl_F_TRY);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_gcobject_calloc(DeeDecWriter *__restrict self, size_t num_bytes, DeeObject *__restrict ref) {
	return decwriter_gcobject_malloc_impl(self, num_bytes, ref,
	                                      decwriter_malloc_impl_F_NORMAL |
	                                      decwriter_malloc_impl_F_BZERO);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_gcobject_trycalloc(DeeDecWriter *__restrict self, size_t num_bytes, DeeObject *__restrict ref) {
	return decwriter_gcobject_malloc_impl(self, num_bytes, ref,
	                                      decwriter_malloc_impl_F_TRY |
	                                      decwriter_malloc_impl_F_BZERO);
}

PRIVATE NONNULL((1)) void DCALL
decwriter_object_free(DeeDecWriter *__restrict self, Dee_seraddr_t addr,
                      DeeObject *__restrict ref) {
	/* TODO */
	(void)self;
	(void)addr;
	(void)ref;
	COMPILER_IMPURE();
}

PRIVATE NONNULL((1)) void DCALL
decwriter_gcobject_free(DeeDecWriter *__restrict self, Dee_seraddr_t addr,
                        DeeObject *__restrict ref) {
	/* TODO */
	(void)self;
	(void)addr;
	(void)ref;
	COMPILER_IMPURE();
}


#ifdef Dee_SLAB_CHUNKSIZE_MAX

/* Allocate a new slab-page and return its base-address, or `Dee_SERADDR_INVALID' on error.
 * The returned base-address is always aligned by `Dee_SLAB_PAGESIZE', and spans a total of
 * `Dee_SLAB_PAGESIZE' bytes of memory.
 *
 * WARNING: While the returned address is properly aligned, note that the dec writer's buffer
 *          probably isn't. However, that is fine since the writer's buffer is only indirectly
 *          accessible, and its only purpose is to construct the contents of a dec file. It is
 *          only after `DeeDecWriter_PackEhdr()' was called that the buffer becomes a proper
 *          dec file memory mapping, and any bad alignment of buffers will finally be fixed
 *          by `DeeDec_Relocate()'. */
PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
decwriter_slab_malloc_newpage_impl(DeeDecWriter *__restrict self, bool do_try) {
	Dee_seraddr_t result;
	size_t min_alloc, cur_alloc;
	if (self->dw_slabs) {
		if (self->dw_used <= self->dw_slabb) {
			/* Can extend the current slab-heap-segment by another page. */
			ASSERT(IS_ALIGNED(self->dw_slabb + sizeof(struct Dee_heapchunk), Dee_SLAB_PAGESIZE));
			ASSERT(IS_ALIGNED(self->dw_slabs - sizeof(struct Dee_heapchunk), Dee_SLAB_PAGESIZE));
			min_alloc = self->dw_slabb + self->dw_slabs + Dee_SLAB_PAGESIZE + sizeof(struct Dee_heaptail);
			cur_alloc = self->dw_alloc;
			if (min_alloc > cur_alloc) {
				byte_t *new_base;
				size_t new_alloc = CEIL_ALIGN(min_alloc * 2, __SIZEOF_POINTER__ * 4 * 1024);
				if (new_alloc < min_alloc)
					new_alloc = min_alloc;
				new_base = (byte_t *)Dee_TryRealloc(self->dw_base, new_alloc);
				if unlikely(!new_base) {
					new_alloc = min_alloc;
					new_base = do_try
					           ? (byte_t *)Dee_TryRealloc(self->dw_base, new_alloc)
					           : (byte_t *)Dee_Realloc(self->dw_base, new_alloc);
					if unlikely(!new_base)
						goto err;
				}
				self->dw_base  = new_base;
				self->dw_alloc = Dee_MallocUsableSizeNonNull(new_base);
				ASSERT(self->dw_alloc >= new_alloc);
			}
			result = self->dw_slabb + self->dw_slabs;
			self->dw_slabs += Dee_SLAB_PAGESIZE;
			ASSERT(IS_ALIGNED(result, Dee_SLAB_PAGESIZE));
			return result;
		}

		/* Must terminate (pack) the current slab segment */
		if unlikely(decwriter_build_slab_pages(self))
			goto err;
		self->dw_slabs = 0;
		DBG_memset(&self->dw_slabb, 0xcc, sizeof(self->dw_slabb));
	}

	/* Create a new slab segment at the next properly-aligned address */
	result = CEIL_ALIGN(self->dw_used, Dee_SLAB_PAGESIZE);

	/* Ensure that the output buffer is large enough */
	min_alloc = result + Dee_SLAB_PAGESIZE + sizeof(struct Dee_heaptail);
	cur_alloc = self->dw_alloc;
	if (min_alloc > cur_alloc) {
		byte_t *new_base;
		size_t new_alloc = CEIL_ALIGN(min_alloc * 2, __SIZEOF_POINTER__ * 4 * 1024);
		if (new_alloc < min_alloc)
			new_alloc = min_alloc;
		new_base = (byte_t *)Dee_TryRealloc(self->dw_base, new_alloc);
		if unlikely(!new_base) {
			new_alloc = min_alloc;
			new_base = do_try
			           ? (byte_t *)Dee_TryRealloc(self->dw_base, new_alloc)
			           : (byte_t *)Dee_Realloc(self->dw_base, new_alloc);
			if unlikely(!new_base)
				goto err;
		}
		self->dw_base  = new_base;
		self->dw_alloc = Dee_MallocUsableSizeNonNull(new_base);
		ASSERT(self->dw_alloc >= new_alloc);
	}

	/* Remember the new slab segment */
	self->dw_slabb = result - sizeof(struct Dee_heapchunk);
	self->dw_slabs = sizeof(struct Dee_heapchunk) + Dee_SLAB_PAGESIZE;

	/* Ensure that the resulting dec file will be properly aligned */
	if (self->dw_align < Dee_SLAB_PAGESIZE)
		self->dw_align = Dee_SLAB_PAGESIZE;
	ASSERT(IS_ALIGNED(result, Dee_SLAB_PAGESIZE));
	return result;
err:
	return Dee_SERADDR_INVALID;
}

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
decwriter_slab_malloc_impl(DeeDecWriter *__restrict self, size_t n, bool do_try) {
#if 1
#define ptr2addr(p) ((Dee_seraddr_t)((byte_t *)(p) - (self)->dw_base))
	void *result_ptr;
	struct Dee_slab_page *page;
	Dee_seraddr_t addrof_page;
	if (self->dw_slabs) {
		/* Try every slab page currently allocated
		 * to see if it can be used to allocate "n" */
		Dee_seraddr_t addrof_pages;
		struct Dee_slab_page *pages;
		size_t i, num_pages;
		ASSERT(IS_ALIGNED(self->dw_slabs - sizeof(struct Dee_heapchunk), Dee_SLAB_PAGESIZE));
		ASSERT(IS_ALIGNED(self->dw_slabb + sizeof(struct Dee_heapchunk), Dee_SLAB_PAGESIZE));
		addrof_pages = self->dw_slabb + sizeof(struct Dee_heapchunk);
		pages = DeeDecWriter_Addr2Mem(self, addrof_pages, struct Dee_slab_page);
		num_pages = (self->dw_slabs - sizeof(struct Dee_heapchunk)) / Dee_SLAB_PAGESIZE;
		ASSERT(num_pages >= 1);
		for (i = 0; i < num_pages; ++i) {
			STATIC_ASSERT(sizeof(struct Dee_slab_page) == Dee_SLAB_PAGESIZE);
			page = &pages[i];
			result_ptr = Dee_slab_page_buildmalloc(page, n);
			if (result_ptr != NULL)
				return ptr2addr(result_ptr); /* Got it! */
		}
	}

	/* None of the existing slab pages allow us to allocate an "n"-byte slab chunk
	 * -> use `decwriter_slab_malloc_newpage_impl()' to allocate a new slab page. */
	addrof_page = decwriter_slab_malloc_newpage_impl(self, do_try);
	if (!Dee_SERADDR_ISOK(addrof_page))
		goto err;
	page = DeeDecWriter_Addr2Mem(self, addrof_page, struct Dee_slab_page);
	Dee_slab_page_buildinit(page);
	result_ptr = Dee_slab_page_buildmalloc(page, n);
	ASSERTF(result_ptr, "How can allocation fail on an entirely empty page?");
	return ptr2addr(result_ptr);
err:
	return Dee_SERADDR_INVALID;
#undef ptr2addr
#else
	return do_try ? decwriter_trymalloc(self, n, NULL)
	              : decwriter_malloc(self, n, NULL);
#endif
}

PRIVATE NONNULL((1)) void DCALL
decwriter_slab_free_impl(DeeDecWriter *__restrict self, Dee_seraddr_t addr, size_t n) {
#if 1
	/* TODO: Dee_slab_page_buildfree() */
	(void)self;
	(void)addr;
	(void)n;
#else
	decwriter_free(self, addr, NULL);
#endif
}



PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
decwriter_slab_malloc(DeeDecWriter *__restrict self,
                      size_t n, /*0..1*/ void const *ref) {
	Dee_seraddr_t result = decwriter_slab_malloc_impl(self, n, false);
	if (Dee_SERADDR_ISOK(result) && ref) {
		if unlikely(decwriter_addknown(self, ref, result, n, false))
			goto err_r;
	}
	return result;
err_r:
	decwriter_slab_free_impl(self, result, n);
	return Dee_SERADDR_INVALID;
}

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
decwriter_slab_trymalloc(DeeDecWriter *__restrict self,
                         size_t n, /*0..1*/ void const *ref) {
	Dee_seraddr_t result = decwriter_slab_malloc_impl(self, n, true);
	if (Dee_SERADDR_ISOK(result) && ref) {
		if unlikely(decwriter_addknown(self, ref, result, n, true))
			goto err_r;
	}
	return result;
err_r:
	decwriter_slab_free_impl(self, result, n);
	return Dee_SERADDR_INVALID;
}

PRIVATE NONNULL((1)) void DCALL
decwriter_slab_free(DeeDecWriter *__restrict self, Dee_seraddr_t addr,
                    size_t n, /*0..1*/ void const *ref) {
	if (ref) {
		/* TODO: decwriter_delknown() */
	}
	decwriter_slab_free_impl(self, addr, n);
}


PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_slab_object_malloc_impl(DeeDecWriter *__restrict self, size_t n,
                                  DeeObject *__restrict ref, bool do_try) {
	DeeObject *copy;
	Dee_seraddr_t result;
	ASSERTF(!DeeType_IsGC(Dee_TYPE(ref)), "Use decwriter_slab_gcobject_malloc_impl()");
	result = do_try ? decwriter_slab_trymalloc(self, n, ref)
	                : decwriter_slab_malloc(self, n, ref);
	if unlikely(!Dee_SERADDR_ISOK(result))
		goto err;

	/* Initialize "ob_refcnt" and "ob_type" of the newly allocated object */
	copy = DeeDecWriter_Addr2Mem(self, result, DeeObject);
	copy->ob_refcnt = 1;
#ifdef CONFIG_TRACE_REFCHANGES
	copy->ob_trace = NULL;
#endif /* CONFIG_TRACE_REFCHANGES */
	if (!DeeType_IsHeapType(Dee_TYPE(ref))) {
		if unlikely(decwriter_putpointer(self,
		                                 result + offsetof(DeeObject, ob_type),
		                                 Dee_AsObject(Dee_TYPE(ref))))
			goto err_r;
	} else {
		if unlikely(decwriter_putobject(self,
		                                result + offsetof(DeeObject, ob_type),
		                                Dee_AsObject(Dee_TYPE(ref))))
			goto err_r;
	}
	return result;
err_r:
	decwriter_slab_free(self, result, n, ref);
err:
	return Dee_SERADDR_INVALID;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_slab_object_malloc(DeeDecWriter *__restrict self,
                             size_t n, DeeObject *__restrict ref) {
	return decwriter_slab_object_malloc_impl(self, n, ref, false);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_slab_object_trymalloc(DeeDecWriter *__restrict self,
                               size_t n, DeeObject *__restrict ref) {
	return decwriter_slab_object_malloc_impl(self, n, ref, true);
}


PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_slab_gcobject_malloc_impl(DeeDecWriter *__restrict self, size_t n,
                                    DeeObject *__restrict ref, bool do_try) {
	size_t total = n + Dee_GC_OBJECT_OFFSET;
	Dee_seraddr_t result;
	DeeObject *copy;
	ASSERTF(DeeType_IsGC(Dee_TYPE(ref)), "Use decwriter_object_malloc_impl()");
	result = do_try ? decwriter_slab_trymalloc(self, total, DeeGC_Head(ref))
	                : decwriter_slab_malloc(self, total, DeeGC_Head(ref));
	if unlikely(!Dee_SERADDR_ISOK(result))
		goto err;

	/* Initialize GC head/tail link pointers */
	ASSERT((self->dw_gchead == 0) ==
	       (self->dw_gctail == 0));
	result += Dee_GC_OBJECT_OFFSET;
	if (self->dw_gctail == 0) {
		/* First GC object... */
		struct Dee_gc_head *link;
		self->dw_gchead = result;
		link = DeeDecWriter_Addr2Mem(self, result - Dee_GC_OBJECT_OFFSET,
		                             struct Dee_gc_head);
		link->gc_info.gi_pself = NULL;
		link->gc_next          = NULL;
	} else {
		/* Append to end of GC list. */
		if (decwriter_putaddr(self,
		                      (result - Dee_GC_OBJECT_OFFSET) +
		                      offsetof(struct Dee_gc_head, gc_info.gi_pself),
		                      (self->dw_gctail - Dee_GC_OBJECT_OFFSET) +
		                      offsetof(struct Dee_gc_head, gc_next)))
			goto err_r;
		if (decwriter_putaddr(self,
		                      (self->dw_gctail - Dee_GC_OBJECT_OFFSET) +
		                      offsetof(struct Dee_gc_head, gc_next),
		                      result))
			goto err_r;
	}
	self->dw_gctail = result;

	/* Initialize "ob_refcnt" and "ob_type" of the newly allocated object */
	copy = DeeDecWriter_Addr2Mem(self, result, DeeObject);
	copy->ob_refcnt = 1;
#ifdef CONFIG_TRACE_REFCHANGES
	copy->ob_trace = NULL;
#endif /* CONFIG_TRACE_REFCHANGES */
	if (!DeeType_IsHeapType(Dee_TYPE(ref))) {
		if unlikely(decwriter_putpointer(self,
		                                 result +
		                                 offsetof(DeeObject, ob_type),
		                                 Dee_AsObject(Dee_TYPE(ref))))
			goto err_r;
	} else {
		if unlikely(decwriter_putobject(self,
		                                result +
		                                offsetof(DeeObject, ob_type),
		                                Dee_AsObject(Dee_TYPE(ref))))
			goto err_r;
	}
	return result;
err_r:
	decwriter_slab_free(self, result - Dee_GC_OBJECT_OFFSET, total, DeeGC_Head(ref));
err:
	return Dee_SERADDR_INVALID;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_slab_gcobject_malloc(DeeDecWriter *__restrict self,
                               size_t n, DeeObject *__restrict ref) {
	return decwriter_slab_gcobject_malloc_impl(self, n, ref, false);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_slab_gcobject_trymalloc(DeeDecWriter *__restrict self,
                                  size_t n, DeeObject *__restrict ref) {
	return decwriter_slab_gcobject_malloc_impl(self, n, ref, true);
}

PRIVATE NONNULL((1, 4)) void DCALL
decwriter_slab_object_free(DeeDecWriter *__restrict self, Dee_seraddr_t addr,
                           size_t n, DeeObject *__restrict ref) {
	/* TODO */
	(void)self;
	(void)addr;
	(void)n;
	(void)ref;
	COMPILER_IMPURE();
}

PRIVATE NONNULL((1, 4)) void DCALL
decwriter_slab_gcobject_free(DeeDecWriter *__restrict self, Dee_seraddr_t addr,
                             size_t n, DeeObject *__restrict ref) {
	/* TODO */
	(void)self;
	(void)addr;
	(void)n;
	(void)ref;
	COMPILER_IMPURE();
}

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
decwriter_slab_calloc(DeeDecWriter *__restrict self,
                      size_t n, /*0..1*/ void const *ref) {
	Dee_seraddr_t result = decwriter_slab_malloc(self, n, ref);
	if (Dee_SERADDR_ISOK(result))
		bzero(DeeDecWriter_Addr2Mem(self, result, void), n);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
decwriter_slab_trycalloc(DeeDecWriter *__restrict self,
                         size_t n, /*0..1*/ void const *ref) {
	Dee_seraddr_t result = decwriter_slab_trymalloc(self, n, ref);
	if (Dee_SERADDR_ISOK(result))
		bzero(DeeDecWriter_Addr2Mem(self, result, void), n);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_slab_object_calloc(DeeDecWriter *__restrict self,
                             size_t n, DeeObject *__restrict ref) {
	Dee_seraddr_t result = decwriter_slab_object_malloc(self, n, ref);
	if (Dee_SERADDR_ISOK(result))
		bzero(DeeDecWriter_Addr2Mem(self, result, byte_t) + Dee_OBJECT_OFFSETOF_DATA, n - Dee_OBJECT_OFFSETOF_DATA);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_slab_object_trycalloc(DeeDecWriter *__restrict self,
                                size_t n, DeeObject *__restrict ref) {
	Dee_seraddr_t result = decwriter_slab_object_trymalloc(self, n, ref);
	if (Dee_SERADDR_ISOK(result))
		bzero(DeeDecWriter_Addr2Mem(self, result, byte_t) + Dee_OBJECT_OFFSETOF_DATA, n - Dee_OBJECT_OFFSETOF_DATA);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_slab_gcobject_calloc(DeeDecWriter *__restrict self,
                               size_t n, DeeObject *__restrict ref) {
	Dee_seraddr_t result = decwriter_slab_gcobject_malloc(self, n, ref);
	if (Dee_SERADDR_ISOK(result))
		bzero(DeeDecWriter_Addr2Mem(self, result, byte_t) + Dee_OBJECT_OFFSETOF_DATA, n - Dee_OBJECT_OFFSETOF_DATA);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
decwriter_slab_gcobject_trycalloc(DeeDecWriter *__restrict self,
                                  size_t n, DeeObject *__restrict ref) {
	Dee_seraddr_t result = decwriter_slab_gcobject_trymalloc(self, n, ref);
	if (Dee_SERADDR_ISOK(result))
		bzero(DeeDecWriter_Addr2Mem(self, result, byte_t) + Dee_OBJECT_OFFSETOF_DATA, n - Dee_OBJECT_OFFSETOF_DATA);
	return result;
}
#endif /* Dee_SLAB_CHUNKSIZE_MAX */



/* Append a copy of `obj' to self and write its address to `addrof_object'
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
decwriter_appendobject(DeeDecWriter *__restrict self,
                       Dee_seraddr_t addrof_object,
                       DeeObject *__restrict obj,
                       ptrdiff_t offset_into_ob) {
	typedef WUNUSED_T NONNULL_T((1, 2)) Dee_seraddr_t
	(DCALL *Dee_tp_serialize_var_t)(DeeObject *__restrict self,
	                                DeeSerial *__restrict writer);
	typedef WUNUSED_T NONNULL_T((1, 2)) int
	(DCALL *Dee_tp_serialize_obj_t)(DeeObject *__restrict self,
	                                DeeSerial *__restrict writer,
	                                Dee_seraddr_t out_addr);
	Dee_seraddr_t out_addr;
	DeeTypeObject *tp = Dee_TYPE(obj);
	Dee_funptr_t tp_serialize = DeeType_GetTpSerialize(tp);
	if unlikely(!tp_serialize)
		goto cannot_serialize;
	if (DeeType_IsVariable(tp)) {
		out_addr = (*(Dee_tp_serialize_var_t)tp_serialize)(obj, (DeeSerial *)self);
		if (!Dee_SERADDR_ISOK(out_addr))
			goto err;
	} else if (tp->tp_init.tp_alloc.tp_free != NULL) {
#ifdef Dee_SLAB_CHUNKSIZE_MAX
		int status;
		size_t slab_instance_size;
		void (DCALL *tp_free)(void *__restrict ob);

		/* Detect known slab allocators so we can serialize them */
		tp_free = tp->tp_init.tp_alloc.tp_free;
		/* TODO: This sort-of detection of slab allocators doesn't work in DEX
		 *       modules when PLT wrapper functions are used by the linker! */
		if (DeeType_IsGC(tp)) {
#define CHECK_N(n, _)                          \
			if (tp_free == &DeeGCSlab_Free##n) \
				slab_instance_size = n;        \
			else
			Dee_SLAB_CHUNKSIZE_GC_FOREACH(CHECK_N, ~)
#undef CHECK_N
			{
				goto cannot_serialize;
			}
		} else {
#define CHECK_N(n, _)                        \
			if (tp_free == &DeeSlab_Free##n) \
				slab_instance_size = n;      \
			else
			Dee_SLAB_CHUNKSIZE_FOREACH(CHECK_N, ~)
#undef CHECK_N
			{
				goto cannot_serialize;
			}
		}

		/* Allocate buffer for object (in slab memory) */
		out_addr = DeeType_IsGC(tp)
		           ? decwriter_slab_gcobject_malloc(self, slab_instance_size, obj)
		           : decwriter_slab_object_malloc(self, slab_instance_size, obj);
		if unlikely(!Dee_SERADDR_ISOK(out_addr))
			goto err;
	
		/* NOTE: Standard fields have already been initialized by "decwriter_[gc]object_malloc" */
		status = (*(Dee_tp_serialize_obj_t)tp_serialize)(obj, (DeeSerial *)self, out_addr);
		if unlikely(status)
			goto err;
#else /* Dee_SLAB_CHUNKSIZE_MAX */
		goto cannot_serialize;
#endif /* !Dee_SLAB_CHUNKSIZE_MAX */
	} else {
		/* Figure out instance size (with support for slab allocators). */
		int status;
		size_t instance_size = tp->tp_init.tp_alloc.tp_instance_size;
		if unlikely(!instance_size)
			goto cannot_serialize;

		/* Allocate buffer for object. */
		out_addr = DeeType_IsGC(tp)
		           ? decwriter_gcobject_malloc(self, instance_size, obj)
		           : decwriter_object_malloc(self, instance_size, obj);
		if unlikely(!Dee_SERADDR_ISOK(out_addr))
			goto err;
	
		/* NOTE: Standard fields have already been initialized by "decwriter_[gc]object_malloc" */
		status = (*(Dee_tp_serialize_obj_t)tp_serialize)(obj, (DeeSerial *)self, out_addr);
		if unlikely(status)
			goto err;
	}
	return decwriter_putaddr(self, addrof_object, out_addr + offset_into_ob);
cannot_serialize:
	Dee_DPRINTF("[LD][dec@%#" PRFxSIZ "] Warning: Unable to serialize instance of %q at %p%+" PRFdSIZ ": %r\n",
	            addrof_object, DeeType_GetName(Dee_TYPE(obj)), obj, offset_into_ob, obj);
	if (self->dw_flags & DeeDecWriter_F_FRELOC)
		return DeeRT_ErrCannotSerialize(obj);

	/* Create a fake RRELA relocation against the deemon core */
	/* XXX: Support for large (>4GiB) relocation targets on 64-bit hosts */
	if (rrelatab_append(&self->dw_drrela, seraddr32(addrof_object), -(Dee_dec_off32_t)offset_into_ob))
		goto err;
	self->dw_flags |= DeeDecWriter_F_NRELOC; /* Remember that serialization will be impossible */
	Dee_Incref(obj);
	{
		byte_t **p_pointer = DeeDecWriter_Addr2Mem(self, addrof_object, byte_t *);
		*p_pointer = (byte_t *)obj + offset_into_ob - (uintptr_t)&DeeModule_Deemon;
	}
	return 0;
err:
	return -1;
}


/* Encode a reference to `obj' at `DeeDecWriter_Addr2Mem(self, addr, DeeObject)'
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
decwriter_putobject_ex(DeeDecWriter *__restrict self,
                       Dee_seraddr_t addrof_object,
                       DeeObject *__restrict obj,
                       ptrdiff_t offset_into_ob) {
	void **p_pointer;
	DREF DeeModuleObject *mod;
	ASSERT(Dee_SERADDR_ISOK(addrof_object));
	ASSERT_OBJECT(obj);

	/* Check if "obj" has already been written */
	{
		size_t lo = 0, hi = self->dw_known.dpt_ptrc;
		struct Dee_dec_ptrtab_entry *known_v = self->dw_known.dpt_ptrv;
		while (lo < hi) {
			size_t mid = (lo + hi) / 2;
			struct Dee_dec_ptrtab_entry *ent = &known_v[mid];
			if ((byte_t *)obj < Dee_dec_ptrtab_entry_getminaddr(ent)) {
				hi = mid;
			} else if ((byte_t *)obj > Dee_dec_ptrtab_entry_getmaxaddr(ent)) {
				lo = mid + 1;
			} else {
				/* Found the entry! */
				DeeObject *known_obj;
				size_t addr = ent->dote_off;
				addr += (byte_t *)obj - Dee_dec_ptrtab_entry_getminaddr(ent);
				known_obj = DeeDecWriter_Addr2Mem(self, addr, DeeObject);
				++known_obj->ob_refcnt; /* Because now there is another reference to this known object! */
				return decwriter_putaddr(self, addrof_object, addr + offset_into_ob);
			}
		}
	}

#ifdef Dee_CONFIG_BOOL_TLS
	if (DeeBool_Check(obj)) {
		/* Always encode the global, static boolean objects,
		 * rather than any potentially thread-local ones. */
		obj = Dee_AsObject(&Dee_FalseTrue.bp_bools[DeeBool_IsTrue(obj)]);
	}
#endif /* Dee_CONFIG_BOOL_TLS */

	/* Check if "obj" points into a dex module, or the deemon core */
	mod = DeeModule_OfPointer(obj);
	if (mod) {
		int result;
		if (mod == DeeModule_GetDeemon()) {
			Dee_DecrefNokill(mod);
			p_pointer = DeeDecWriter_Addr2Mem(self, addrof_object, void *);
			*p_pointer = (void *)((uintptr_t)obj + offset_into_ob - (uintptr_t)mod);
			if (offset_into_ob != 0) {
				/* XXX: Support for large (>4GiB) relocation targets on 64-bit hosts */
				result = rrelatab_append(&self->dw_drrela, seraddr32(addrof_object),
				                         (Dee_dec_off32_t)-offset_into_ob);
			} else {
				/* XXX: Support for large (>4GiB) relocation targets on 64-bit hosts */
				result = rreltab_append(&self->dw_drrel, seraddr32(addrof_object));
			}
		} else {
			struct Dee_dec_depmod *dep;
			dep = decwriter_getdep(self, mod);
			Dee_Decref_unlikely(mod);
			if unlikely(!dep)
				goto err;
			p_pointer = DeeDecWriter_Addr2Mem(self, addrof_object, void *);
			*p_pointer = (void *)((uintptr_t)obj + offset_into_ob - (uintptr_t)mod);
			if (offset_into_ob != 0) {
				/* XXX: Support for large (>4GiB) relocation targets on 64-bit hosts */
				result = rrelatab_append(&dep->ddm_rrela, seraddr32(addrof_object),
				                         (Dee_dec_off32_t)-offset_into_ob);
			} else {
				/* XXX: Support for large (>4GiB) relocation targets on 64-bit hosts */
				result = rreltab_append(&dep->ddm_rrel, seraddr32(addrof_object));
			}
		}
		if likely(result == 0)
			Dee_Incref(obj);
		return result;
	}

	/* Fallback: must embed a copy of the object within the dec file. */
	return decwriter_appendobject(self, addrof_object, obj, offset_into_ob);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
decwriter_putobject(DeeDecWriter *__restrict self,
                    Dee_seraddr_t addrof_object,
                    DeeObject *__restrict obj) {
	return decwriter_putobject_ex(self, addrof_object, obj, 0);
}

/* Encode pointers to either static data, or some
 * other object that has already been written.
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
decwriter_putpointer(DeeDecWriter *__restrict self,
                     Dee_seraddr_t addrof_pointer,
                     void const *pointer) {
	DREF DeeModuleObject *mod;

	/* Check if "pointer" is statically allocated. */
	if ((mod = DeeModule_OfPointer(pointer)) != NULL) {
		struct Dee_dec_depmod *dep;
		void **out_pointer;
		if (mod == DeeModule_GetDeemon()) {
			Dee_DecrefNokill(mod);
			out_pointer = DeeDecWriter_Addr2Mem(self, addrof_pointer, void *);
			*out_pointer = (void *)((uintptr_t)pointer - (uintptr_t)&DeeModule_Deemon);
			/* XXX: Support for large (>4GiB) relocation targets on 64-bit hosts */
			return reltab_append(&self->dw_drel, seraddr32(addrof_pointer));
		}
		dep = decwriter_getdep(self, mod);
		Dee_Decref_unlikely(mod);
		if unlikely(!dep)
			goto err;
		out_pointer = DeeDecWriter_Addr2Mem(self, addrof_pointer, void *);
		*out_pointer = (void *)((uintptr_t)pointer - (uintptr_t)mod);
		/* XXX: Support for large (>4GiB) relocation targets on 64-bit hosts */
		return reltab_append(&dep->ddm_rel, seraddr32(addrof_pointer));
	}

	/* Check if "pointer" is part of some already-serialized section of memory
	 * NOTE: This is why "DeeSerial_Malloc()" must also take a "void const *ref" argument!
	 *
	 * Reason: >> ClassOperatorTableIterator
	 *         When being serialized, its "co_iter" and "co_end" pointers point
	 *         into the associated "ClassDescriptor *co_desc"'s cd_clsop_list,
	 *         but when "ClassDescriptor" is serialized, 'cd_clsop_list' is
	 *         copied using 'DeeSerial_Malloc()', meaning that we won't be able
	 *         to link "co_iter" and "co_end" to 'cd_clsop_list'! */
	{
		size_t lo = 0, hi = self->dw_known.dpt_ptrc;
		struct Dee_dec_ptrtab_entry *known = self->dw_known.dpt_ptrv;
		while (lo < hi) {
			size_t mid = (lo + hi) / 2;
			struct Dee_dec_ptrtab_entry *ent = &known[mid];
			if ((byte_t *)pointer < Dee_dec_ptrtab_entry_getminaddr(ent)) {
				hi = mid;
			} else if ((byte_t *)pointer >
			           /* +1, because we want to include the object's end! */
			           Dee_dec_ptrtab_entry_getmaxaddr(ent) + 1) {
				lo = mid + 1;
			} else {
				/* Found the entry! */
				Dee_seraddr_t addrof_target;
				addrof_target = (Dee_seraddr_t)((byte_t *)pointer - Dee_dec_ptrtab_entry_getminaddr(ent));
				addrof_target += ent->dote_off;
				return decwriter_putaddr(self, addrof_pointer, addrof_target);
			}
		}
	}

	/* If neither was the case, throw an error indicating that serialization isn't possible */
	Dee_DPRINTF("[LD][dec@%#" PRFxSIZ "] Warning: Unable to serialize pointer %p\n", addrof_pointer, pointer);
	if (self->dw_flags & DeeDecWriter_F_FRELOC) {
		return DeeError_Throwf(&DeeError_NotImplemented,
		                       "Unable to serialize pointer '%p': not statically allocated, "
		                       /**/ "and not part of the 'ref' of an already-serialized object",
		                       pointer);
	}

	/* Encode the pointer as-is (which will make serialization impossible, but we can deal with that) */
	self->dw_flags |= DeeDecWriter_F_NRELOC; /* Remember that serialization will be impossible */
	*DeeDecWriter_Addr2Mem(self, addrof_pointer, void const *) = pointer;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
decwriter_putweakref_ex(DeeDecWriter *__restrict self,
                        Dee_seraddr_t addrof_weakref,
                        DeeObject *__restrict ob,
                        Dee_weakref_callback_t del) {
	struct Dee_weakref *out;
	/* TODO: Proper support serializing for weak references */
	(void)ob;
	(void)del;
	out = DeeDecWriter_Addr2Mem(self, addrof_weakref, struct Dee_weakref);
	Dee_weakref_initempty(out);
	return 0;
}



PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) void *DCALL
decwriter_addr2mem(DeeDecWriter *__restrict self, Dee_seraddr_t addr) {
	return DeeDecWriter_Addr2Mem(self, addr, void);
}


PRIVATE struct Dee_serial_type tpconst decwriter_serial_type = {
	/* .set_addr2mem                = */ (void *(DCALL *)(DeeSerial *__restrict, Dee_seraddr_t))&decwriter_addr2mem,
	/* .set_malloc                  = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))&decwriter_malloc,
	/* .set_calloc                  = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))&decwriter_calloc,
	/* .set_trymalloc               = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))&decwriter_trymalloc,
	/* .set_trycalloc               = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))&decwriter_trycalloc,
	/* .set_free                    = */ (void (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, void const *))&decwriter_free,
	/* .set_object_malloc           = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_object_malloc,
	/* .set_object_calloc           = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_object_calloc,
	/* .set_object_trymalloc        = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_object_trymalloc,
	/* .set_object_trycalloc        = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_object_trycalloc,
	/* .set_object_free             = */ (void (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, DeeObject *__restrict))&decwriter_object_free,
	/* .set_gcobject_malloc         = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_gcobject_malloc,
	/* .set_gcobject_calloc         = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_gcobject_calloc,
	/* .set_gcobject_trymalloc      = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_gcobject_trymalloc,
	/* .set_gcobject_trycalloc      = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_gcobject_trycalloc,
	/* .set_gcobject_free           = */ (void (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, DeeObject *__restrict))&decwriter_gcobject_free,
#ifdef Dee_SLAB_CHUNKSIZE_MAX
	/* .set_slab_malloc             = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))&decwriter_slab_malloc,
	/* .set_slab_calloc             = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))&decwriter_slab_calloc,
	/* .set_slab_trymalloc          = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))&decwriter_slab_trymalloc,
	/* .set_slab_trycalloc          = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))&decwriter_slab_trycalloc,
	/* .set_slab_free               = */ (void (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, size_t, void const *))&decwriter_slab_free,
	/* .set_slab_object_malloc      = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_slab_object_malloc,
	/* .set_slab_object_calloc      = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_slab_object_calloc,
	/* .set_slab_object_trymalloc   = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_slab_object_trymalloc,
	/* .set_slab_object_trycalloc   = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_slab_object_trycalloc,
	/* .set_slab_object_free        = */ (void (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, size_t, DeeObject *__restrict))&decwriter_slab_object_free,
	/* .set_slab_gcobject_malloc    = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_slab_gcobject_malloc,
	/* .set_slab_gcobject_calloc    = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_slab_gcobject_calloc,
	/* .set_slab_gcobject_trymalloc = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_slab_gcobject_trymalloc,
	/* .set_slab_gcobject_trycalloc = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&decwriter_slab_gcobject_trycalloc,
	/* .set_slab_gcobject_free      = */ (void (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, size_t, DeeObject *__restrict))&decwriter_slab_gcobject_free,
#else /* Dee_SLAB_CHUNKSIZE_MAX */
	/* .set_slab_malloc             = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))(Dee_funptr_t)(void const *)(uintptr_t)-1,
	/* .set_slab_calloc             = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))(Dee_funptr_t)(void const *)(uintptr_t)-1,
	/* .set_slab_trymalloc          = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))(Dee_funptr_t)(void const *)(uintptr_t)-1,
	/* .set_slab_trycalloc          = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))(Dee_funptr_t)(void const *)(uintptr_t)-1,
	/* .set_slab_free               = */ (void (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, size_t, void const *))(Dee_funptr_t)(void const *)(uintptr_t)-1,
	/* .set_slab_object_malloc      = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))(Dee_funptr_t)(void const *)(uintptr_t)-1,
	/* .set_slab_object_calloc      = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))(Dee_funptr_t)(void const *)(uintptr_t)-1,
	/* .set_slab_object_trymalloc   = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))(Dee_funptr_t)(void const *)(uintptr_t)-1,
	/* .set_slab_object_trycalloc   = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))(Dee_funptr_t)(void const *)(uintptr_t)-1,
	/* .set_slab_object_free        = */ (void (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, size_t, DeeObject *__restrict))(Dee_funptr_t)(void const *)(uintptr_t)-1,
	/* .set_slab_gcobject_malloc    = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))(Dee_funptr_t)(void const *)(uintptr_t)-1,
	/* .set_slab_gcobject_calloc    = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))(Dee_funptr_t)(void const *)(uintptr_t)-1,
	/* .set_slab_gcobject_trymalloc = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))(Dee_funptr_t)(void const *)(uintptr_t)-1,
	/* .set_slab_gcobject_trycalloc = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))(Dee_funptr_t)(void const *)(uintptr_t)-1,
	/* .set_slab_gcobject_free      = */ (void (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, size_t, DeeObject *__restrict))(Dee_funptr_t)(void const *)(uintptr_t)-1,
#endif /* !Dee_SLAB_CHUNKSIZE_MAX */
	/* .set_putaddr                 = */ (int (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, Dee_seraddr_t))&decwriter_putaddr,
	/* .set_putobject               = */ (int (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, DeeObject *__restrict))&decwriter_putobject,
	/* .set_putobject_ex            = */ (int (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, DeeObject *__restrict, ptrdiff_t))&decwriter_putobject_ex,
	/* .set_putpointer              = */ (int (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, void const *__restrict))&decwriter_putpointer,
	/* .set_putweakref_ex           = */ (int (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, DeeObject *__restrict, Dee_weakref_callback_t))&decwriter_putweakref_ex,
};


/* Initialize/finalize a dec writer. Note that unlike usual, `DeeDecWriter_Init()'
 * already needs to allocate a small amount of heap memory, meaning it can actually
 * fail due to OOM and do so by returning `-1'
 * @return: 0 : Success
 * @return: -1: An error was thrown */
PUBLIC WUNUSED NONNULL((1)) int DCALL
_DeeDecWriter_Init(DeeDecWriter *__restrict self) {
	STATIC_ASSERT_MSG(IS_ALIGNED(offsetof(Dec_Ehdr, e_heap.hr_first), Dee_HEAPCHUNK_ALIGN),
	                  "This is required for the embedded heap to work properly, and is "
	                  /**/ "also required for 'decwriter_malloc_impl()' to function");
	self->ser_type = &decwriter_serial_type;
	self->dw_ehdr = (Dec_Ehdr *)Dee_TryMalloc(sizeof(Dec_Ehdr) + (64 * 1024));
	if unlikely(!self->dw_ehdr) {
		self->dw_ehdr = (Dec_Ehdr *)Dee_Malloc(sizeof(Dec_Ehdr) + sizeof(struct Dee_heaptail));
		if unlikely(!self->dw_ehdr)
			goto err;
	}
	self->dw_alloc = Dee_MallocUsableSizeNonNull(self->dw_ehdr);
	/* The header struct ends at the start of the first heap chunk's user-area */
	self->dw_used  = offsetof(Dec_Ehdr, e_heap.hr_first);
	self->dw_hlast = 0;
	self->dw_align = Dee_HEAPCHUNK_ALIGN;
	DBG_memset(&self->dw_slabb, 0xcc, sizeof(self->dw_slabb));
	self->dw_slabs = 0;
	Dee_dec_reltab_init(&self->dw_srel);
	Dee_dec_reltab_init(&self->dw_drel);
	Dee_dec_rreltab_init(&self->dw_drrel);
	Dee_dec_rrelatab_init(&self->dw_drrela);
	Dee_dec_deptab_init(&self->dw_deps);
	Dee_dec_fdeptab_init(&self->dw_fdeps);
	self->dw_gchead = 0;
	self->dw_gctail = 0;
	Dee_dec_ptrtab_init(&self->dw_known);
	return 0;
err:
	return -1;
}

PUBLIC NONNULL((1)) void DCALL
DeeDecWriter_Fini(DeeDecWriter *__restrict self) {
	size_t i;
	ASSERT(self->ser_type == &decwriter_serial_type);
	Dee_dec_ptrtab_fini(&self->dw_known);
	Dee_dec_fdeptab_fini(&self->dw_fdeps);
	w_rrel_decref_nokill(self->dw_ehdr, self->dw_drrel.drrt_relv, self->dw_drrel.drrt_relc, (uintptr_t)&DeeModule_Deemon);
	/* NOTE: "DeeDecWriter_F_NRELOC"-relocations in `decwriter_putobject' are
	 *       written to "dw_drrela", so can't use w_rrela_decref_nokill here! */
	w_rrela_decref(self->dw_ehdr, self->dw_drrela.drat_relv, self->dw_drrela.drat_relc, (uintptr_t)&DeeModule_Deemon);
	Dee_dec_rrelatab_fini(&self->dw_drrela);
	Dee_dec_rreltab_fini(&self->dw_drrel);
	Dee_dec_reltab_fini(&self->dw_drel);
	Dee_dec_reltab_fini(&self->dw_srel);
	for (i = 0; i < self->dw_deps.ddpt_depc; ++i) {
		struct Dee_dec_depmod *dep = &self->dw_deps.ddpt_depv[i];
		uintptr_t relbase = (uintptr_t)dep->ddm_mod;
		w_rrel_decref(self->dw_ehdr, dep->ddm_rrel.drrt_relv, dep->ddm_rrel.drrt_relc, relbase);
		w_rrela_decref(self->dw_ehdr, dep->ddm_rrela.drat_relv, dep->ddm_rrela.drat_relc, relbase);
		Dee_dec_depmod_fini(dep);
	}
	Dee_Free(self->dw_deps.ddpt_depv);
	Dee_Free(self->dw_base);
	DBG_memset(self, 0xcc, sizeof(*self));
}

DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_DEC_C */
