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

#include <deemon/dec.h>              /* DBUILTINS_MAX, DECMAG*, DEC_BUILTINID_MAKE, DEC_BUILTINID_UNKNOWN, DFILE_LIMIT, DI_MAG*, DTYPE16_BUILTIN_MIN, DTYPE16_CELL, DTYPE16_CLASSDESC, DTYPE16_DICT, DTYPE16_HASHSET, DTYPE16_NONE, DTYPE16_RODICT, DTYPE16_ROSET, DTYPE_BUILTIN_MAX, DTYPE_BUILTIN_MIN, DTYPE_BUILTIN_NUM, DTYPE_CLASSDESC, DTYPE_CODE, DTYPE_EXTENDED, DTYPE_FUNCTION, DTYPE_IEEE754, DTYPE_KWDS, DTYPE_LIST, DTYPE_NONE, DTYPE_NULL, DTYPE_SLEB, DTYPE_STRING, DTYPE_TUPLE, DTYPE_ULEB, DVERSION_CUR, Dec_*, DeeDecWriter, DeeDecWriter_*, DeeDec_Ehdr, DeeDec_Ehdr_*, Dee_ALIGNOF_DEC_*, Dee_DEC_ENDIAN, Dee_DEC_MACH, Dee_DEC_TYPE_IMAGE, Dee_DEC_TYPE_RELOC, Dee_dec_* */
#include <deemon/object.h>           /* ASSERT_OBJECT, ASSERT_OBJECT_TYPE, ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, DeeObject_Type, DeeTypeObject, Dee_AsObject, Dee_Decref*, Dee_Incref, Dee_IncrefIfNotZero, Dee_OBJECT_OFFSETOF_DATA, Dee_TYPE, Dee_XClear, Dee_XDecref, Dee_XDecrefv, Dee_funptr_t, Dee_hash_t, Dee_uint128_t, Dee_weakref_support_cinit, ITER_DONE, ITER_ISOK */
#include <deemon/serial.h>           /* DeeSerial, Dee_SERADDR_INVALID, Dee_SERADDR_ISOK, Dee_seraddr_t, Dee_serial_type */
#include <deemon/type.h>             /* DeeObject_Init, DeeType_*, Dee_operator_t */
#include <deemon/util/hash.h>        /* Dee_HashPtr, Dee_HashStr */
#include <deemon/util/slab-config.h> /* Dee_SLAB_* */
#include <deemon/util/slab.h>        /* Dee_SLAB_PAGESIZE, Dee_SLAB_PAGE_META_CUSTOM_MARKER, Dee_slab_page, Dee_slab_page_buildinit, Dee_slab_page_buildmalloc */
#include <deemon/util/weakref.h>     /* Dee_weakref, Dee_weakref_callback_t, Dee_weakref_initempty */

#ifndef CONFIG_NO_DEC
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
#include <deemon/alloc.h>           /* DeeObject_Callocc, DeeObject_Free, DeeSlab_Free, Dee_*alloc*, Dee_BadAlloc, Dee_Free, Dee_Memalign, _Dee_MallococBufsize */
#include <deemon/error-rt.h>        /* DeeRT_ErrCannotSerialize */
#include <deemon/error.h>           /* DeeError_* */
#include <deemon/exec.h>            /* DeeExec_GetTimestamp */
#include <deemon/format.h>          /* PRF* */
#include <deemon/gc.h>              /* DeeGCObject_*alloc*, DeeGCObject_Free, DeeGCSlab_Free, DeeGC_*, Dee_GC_OBJECT_OFFSET, Dee_gc_head, Dee_gc_head_link */
#include <deemon/heap.h>            /* DeeHeap_GetRegionOf, Dee_HEAPCHUNK_*, Dee_heapchunk, Dee_heapregion, Dee_heaptail */
#include <deemon/module.h>          /* DeeModule*, Dee_DEC_FLOADOUTDATED, Dee_DEC_FUNTRUSTED, Dee_MODSYM_F*, Dee_MODULE_F*, Dee_MODULE_HASHNX, Dee_module_buildid, Dee_module_symbol */
#include <deemon/string.h>          /* DeeString*, Dee_EmptyString, STRING_ERROR_FSTRICT, WSTR_LENGTH */
#include <deemon/system-features.h> /* DeeSystem_DEFINE_memcasecmp, bcmp, bzero, link, memcmp, memcpy*, memmoveupc, mempcpy*, memset, strlen */
#include <deemon/system.h>          /* DeeSystem_* */
#include <deemon/util/atomic.h>     /* atomic_* */
#include <deemon/util/md5.h>        /* DeeMD5_* */

#include <hybrid/align.h>            /* CEIL_ALIGN, IS_ALIGNED, IS_POWER_OF_TWO */
#include <hybrid/limitcore.h>        /* __UINT32_MAX__ */
#include <hybrid/overflow.h>         /* OVERFLOW_UADD */
#include <hybrid/sequence/bsearch.h> /* BSEARCH */
#include <hybrid/typecore.h>         /* __BYTE_TYPE__, __SIZEOF_POINTER__ */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* offsetof, ptrdiff_t, size_t */
#include <stdint.h>  /* int32_t, uintN_t, uintptr_t */

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
		 * FLAG4_BIT_INDICATES_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE
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
#ifdef CONFIG_EXPERIMENTAL_REWORKED_GC
		DeeObject *gc_head = (DeeObject *)((byte_t *)self + self->e_typedata.td_reloc.er_offsetof_gchead);
		DeeObject *gc_tail = (DeeObject *)((byte_t *)self + self->e_typedata.td_reloc.er_offsetof_gctail);
		DeeGC_TrackAll(gc_head, gc_tail, DeeGC_TRACK_F_NORMAL);
#else /* CONFIG_EXPERIMENTAL_REWORKED_GC */
		struct Dee_gc_head *gc_head = (struct Dee_gc_head *)((byte_t *)self + self->e_typedata.td_reloc.er_offsetof_gchead);
		struct Dee_gc_head *gc_tail = (struct Dee_gc_head *)((byte_t *)self + self->e_typedata.td_reloc.er_offsetof_gctail);
		DeeGC_TrackMany_Lock();
		DeeGC_TrackMany_Exec(DeeGC_Object(gc_head), DeeGC_Object(gc_tail));
		DeeGC_TrackMany_Unlock();
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */
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
#define goto_fail(reason)     \
	do {                      \
		fail_reason = reason; \
		goto fail;            \
	}	__WHILE0
#else /* !Dee_DPRINT_IS_NOOP */
#define goto_fail(reason) goto fail
#endif /* Dee_DPRINT_IS_NOOP */
	union Dee_module_buildid const *deemon_buildid;
	Dec_Ehdr *ehdr = (Dec_Ehdr *)DeeMapFile_GetAddr(fmap);
	if unlikely(DeeMapFile_GetSize(fmap) < sizeof(Dec_Ehdr))
		goto_fail("File is smaller than 'sizeof(Dec_Ehdr)'");

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
	if unlikely(ehdr->e_ident[DI_MAG0] != DECMAG0)
		goto_fail("Bad DI_MAG0");
	if unlikely(ehdr->e_ident[DI_MAG1] != DECMAG1)
		goto_fail("Bad DI_MAG1");
	if unlikely(ehdr->e_ident[DI_MAG2] != DECMAG2)
		goto_fail("Bad DI_MAG2");
	if unlikely(ehdr->e_ident[DI_MAG3] != DECMAG3)
		goto_fail("Bad DI_MAG3");
	if unlikely(ehdr->e_mach != Dee_DEC_MACH)
		goto_fail("Bad 'e_mach'");
	/* Only relocatable images can be written to disk, so that's the only valid type */
	if unlikely(ehdr->e_type != Dee_DEC_TYPE_RELOC)
		goto_fail("Bad 'e_type'");
	if unlikely(ehdr->e_version != DVERSION_CUR)
		goto_fail("Bad 'e_version'");
	if unlikely(ehdr->e_typedata.td_reloc.er_offsetof_heap != offsetof(Dec_Ehdr, e_heap))
		goto_fail("Bad 'er_offsetof_heap'");
	if unlikely(ehdr->e_typedata.td_reloc.er_sizeof_pointer != sizeof(void *))
		goto_fail("Bad 'er_sizeof_pointer'");
	if unlikely(ehdr->e_typedata.td_reloc.er_endian != Dee_DEC_ENDIAN)
		goto_fail("Bad 'er_endian'");
	if unlikely(ehdr->e_typedata.td_reloc.er_offsetof_eof != DeeMapFile_GetSize(fmap))
		goto_fail("Bad 'er_offsetof_eof' does not match size of file");
	if unlikely(ehdr->e_typedata.td_reloc.er_offsetof_eof > DFILE_LIMIT)
		goto_fail("Bad 'er_offsetof_eof' is greater than 'DFILE_LIMIT'");
	if unlikely(ehdr->e_typedata.td_reloc.er_offsetof_srel >= ehdr->e_typedata.td_reloc.er_offsetof_eof)
		goto_fail("Bad 'er_offsetof_srel'");
	if unlikely(ehdr->e_typedata.td_reloc.er_offsetof_drel >= ehdr->e_typedata.td_reloc.er_offsetof_eof)
		goto_fail("Bad 'er_offsetof_drel'");
	if unlikely(ehdr->e_typedata.td_reloc.er_offsetof_drrel >= ehdr->e_typedata.td_reloc.er_offsetof_eof)
		goto_fail("Bad 'er_offsetof_drrel'");
	if unlikely(ehdr->e_typedata.td_reloc.er_offsetof_deps >= ehdr->e_typedata.td_reloc.er_offsetof_eof)
		goto_fail("Bad 'er_offsetof_deps'");
	if unlikely(/*ehdr->e_typedata.td_reloc.er_offsetof_files != 0 &&*/
	            ehdr->e_typedata.td_reloc.er_offsetof_files >= ehdr->e_typedata.td_reloc.er_offsetof_eof)
		goto_fail("Bad 'er_offsetof_files'");
	if unlikely(/*ehdr->e_typedata.td_reloc.er_offsetof_xrel != 0 &&*/
	            ehdr->e_typedata.td_reloc.er_offsetof_xrel >= ehdr->e_typedata.td_reloc.er_offsetof_eof)
		goto_fail("Bad 'er_offsetof_xrel'");
	if unlikely(!IS_POWER_OF_TWO(ehdr->e_typedata.td_reloc.er_alignment))
		goto_fail("Bad 'er_alignment'");
	if unlikely(ehdr->e_typedata.td_reloc.er_offsetof_gchead == 0 || ehdr->e_typedata.td_reloc.er_offsetof_gchead >= ehdr->e_typedata.td_reloc.er_offsetof_eof)
		goto_fail("Bad 'er_offsetof_gchead'");
	if unlikely(ehdr->e_typedata.td_reloc.er_offsetof_gctail == 0 || ehdr->e_typedata.td_reloc.er_offsetof_gctail >= ehdr->e_typedata.td_reloc.er_offsetof_eof)
		goto_fail("Bad 'er_offsetof_gctail'");
	if unlikely((ehdr->e_typedata.td_reloc.er_offsetof_gchead != 0) != (ehdr->e_typedata.td_reloc.er_offsetof_gctail != 0))
		goto_fail("Presence of 'er_offsetof_gchead' and 'er_offsetof_gctail' do not match");
	if unlikely(ehdr->e_heap.hr_size >= (ehdr->e_typedata.td_reloc.er_offsetof_eof - offsetof(Dec_Ehdr, e_heap)))
		goto_fail("Bad 'e_heap.hr_size' points past EOF");

#if 1 /* Validate the module's MD5 checksum */
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
		 * cascade of dependent modules also needing to be re-built).
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
#undef goto_fail
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

#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR

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

/* NOTE: We mark these functions with 'ATTR_NOINLINE' to signify that they must
 *       not alias each other (even though they are implemented identically). I
 *       know that C already guaranties that functions with identical impls will
 *       still have distinct addresses, but that is actually something I don't
 *       want to rely on in deemon, since a (potential) compiler switch to make
 *       this requirement go away could result in some very nice optimizations.
 *
 * A better attribute (if it existed) would be something like "ATTR_NOALIAS" */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
decslab_freeN_first(struct Dee_slab_page *__restrict self) {
	decslab_freeN(self);
}

PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
decslab_freeN_middle(struct Dee_slab_page *__restrict self) {
	decslab_freeN(self);
}

PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
decslab_freeN_last(struct Dee_slab_page *__restrict self) {
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
#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

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

#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
	/* Update `dw_used' to include memory reserved for the currently-allocated page */
	if (self->dw_slabs) {
		ASSERT(self->dw_slabs > sizeof(Dee_heapchunk));
		ASSERT(IS_ALIGNED(self->dw_slabs - sizeof(Dee_heapchunk), Dee_SLAB_PAGESIZE));
		ASSERT(IS_ALIGNED(self->dw_slabb + sizeof(Dee_heapchunk), Dee_SLAB_PAGESIZE));
		if (self->dw_used < self->dw_slabb) {
			struct Dee_heapchunk *slab_chunk;
			/* Extend the last heap-chunk t */
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
#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

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
			impstr_utf8 = DeeString_AsUtf8((DeeObject *)dep->ddm_impstr);
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
				impstr_utf8 = DeeString_AsUtf8((DeeObject *)dep->ddm_impstr);
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
#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
	DBG_memset(&self->dw_slabb, 0xcc, sizeof(self->dw_slabb));
	self->dw_slabs = 0;
#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

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
#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
	if (self->dw_slabs) {
		ASSERT(self->dw_alloc >= (self->dw_slabb + self->dw_slabs + sizeof(struct Dee_heaptail)));
		if (self->dw_used < self->dw_slabb) {
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
#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

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
		self->dw_alloc = Dee_MallocUsableSize(new_base);
		ASSERT(self->dw_alloc >= new_alloc);
	}

	/* Initialize the heap chunk for the newly made allocation */
#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
do_allocate:
#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
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
	if unlikely(decwriter_putobject(self,
	                                result + offsetof(DeeObject, ob_type),
	                                Dee_AsObject(Dee_TYPE(ref))))
		goto err_r;
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
#ifdef CONFIG_EXPERIMENTAL_REWORKED_GC
	DeeObject *copy;
#else /* CONFIG_EXPERIMENTAL_REWORKED_GC */
	struct Dee_gc_head *copy;
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */
	ASSERTF(DeeType_IsGC(Dee_TYPE(ref)), "Use decwriter_object_malloc_impl()");
	if (OVERFLOW_UADD(num_bytes, Dee_GC_OBJECT_OFFSET, &total))
		total = (size_t)-1;
	result = decwriter_malloc_impl(self, total, DeeGC_Head(ref), flags);
	if unlikely(!Dee_SERADDR_ISOK(result))
		goto err;

#ifdef CONFIG_EXPERIMENTAL_REWORKED_GC
	/* Initialize GC head/tail link pointers */
	ASSERT((self->dw_gchead == 0) ==
	       (self->dw_gctail == 0));
	result += Dee_GC_OBJECT_OFFSET;
	if (self->dw_gctail == 0) {
		/* First GC object... */
		struct Dee_gc_head_link *link;
		self->dw_gchead = result;
		link = DeeDecWriter_Addr2Mem(self, result - Dee_GC_OBJECT_OFFSET,
		                             struct Dee_gc_head_link);
		link->gc_info.gi_pself = NULL;
		link->gc_next          = NULL;
	} else {
		/* Append to end of GC list. */
		if (decwriter_putaddr(self,
		                      (result - Dee_GC_OBJECT_OFFSET) +
		                      offsetof(struct Dee_gc_head_link, gc_info.gi_pself),
		                      (self->dw_gctail - Dee_GC_OBJECT_OFFSET) +
		                      offsetof(struct Dee_gc_head_link, gc_next)))
			goto err_r;
		if (decwriter_putaddr(self,
		                      (self->dw_gctail - Dee_GC_OBJECT_OFFSET) +
		                      offsetof(struct Dee_gc_head_link, gc_next),
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
	if unlikely(decwriter_putobject(self,
	                                result +
	                                offsetof(DeeObject, ob_type),
	                                Dee_AsObject(Dee_TYPE(ref))))
		goto err_r;
#else /* CONFIG_EXPERIMENTAL_REWORKED_GC */
	/* Initialize GC head/tail link pointers */
	ASSERT((self->dw_gchead == 0) ==
	       (self->dw_gctail == 0));
	if (self->dw_gctail == 0) {
		/* First GC object... */
		struct Dee_gc_head_link *link;
		self->dw_gchead = result;
		link = DeeDecWriter_Addr2Mem(self, result, struct Dee_gc_head_link);
		link->gc_next  = NULL;
		link->gc_pself = NULL;
	} else {
		/* Append to end of GC list. */
		if (decwriter_putaddr(self, self->dw_gctail + (Dee_dec_addr32_t)offsetof(struct Dee_gc_head_link, gc_next), result))
			goto err_r;
		if (decwriter_putaddr(self, result + (Dee_dec_addr32_t)offsetof(struct Dee_gc_head_link, gc_pself),
		                      self->dw_gctail + (Dee_dec_addr32_t)offsetof(struct Dee_gc_head_link, gc_next)))
			goto err_r;
	}
	self->dw_gctail = result;

	/* Initialize "ob_refcnt" and "ob_type" of the newly allocated object */
	copy = DeeDecWriter_Addr2Mem(self, result, struct Dee_gc_head);
	DeeGC_Object(copy)->ob_refcnt = 1;
#ifdef CONFIG_TRACE_REFCHANGES
	copy->gc_object.ob_trace = NULL;
#endif /* CONFIG_TRACE_REFCHANGES */
	if unlikely(decwriter_putobject(self,
	                                result + Dee_GC_OBJECT_OFFSET +
	                                offsetof(DeeObject, ob_type),
	                                Dee_AsObject(Dee_TYPE(ref))))
		goto err_r;
	result += Dee_GC_OBJECT_OFFSET;
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */
	return result;
err_r:
#ifdef CONFIG_EXPERIMENTAL_REWORKED_GC
	decwriter_free(self, result - Dee_GC_OBJECT_OFFSET, DeeGC_Head(ref));
#else /* CONFIG_EXPERIMENTAL_REWORKED_GC */
	decwriter_free(self, result, ref);
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */
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


#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
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
				self->dw_alloc = Dee_MallocUsableSize(new_base);
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
		self->dw_alloc = Dee_MallocUsableSize(new_base);
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

PRIVATE WUNUSED NONNULL((1)) void DCALL
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
	if unlikely(decwriter_putobject(self,
	                                result + offsetof(DeeObject, ob_type),
	                                Dee_AsObject(Dee_TYPE(ref))))
		goto err_r;
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
		struct Dee_gc_head_link *link;
		self->dw_gchead = result;
		link = DeeDecWriter_Addr2Mem(self, result - Dee_GC_OBJECT_OFFSET,
		                             struct Dee_gc_head_link);
		link->gc_info.gi_pself = NULL;
		link->gc_next          = NULL;
	} else {
		/* Append to end of GC list. */
		if (decwriter_putaddr(self,
		                      (result - Dee_GC_OBJECT_OFFSET) +
		                      offsetof(struct Dee_gc_head_link, gc_info.gi_pself),
		                      (self->dw_gctail - Dee_GC_OBJECT_OFFSET) +
		                      offsetof(struct Dee_gc_head_link, gc_next)))
			goto err_r;
		if (decwriter_putaddr(self,
		                      (self->dw_gctail - Dee_GC_OBJECT_OFFSET) +
		                      offsetof(struct Dee_gc_head_link, gc_next),
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
	if unlikely(decwriter_putobject(self,
	                                result +
	                                offsetof(DeeObject, ob_type),
	                                Dee_AsObject(Dee_TYPE(ref))))
		goto err_r;
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
#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */



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
#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
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
#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
	} else {
		/* Figure out instance size (with support for slab allocators). */
		int status;
#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
		size_t instance_size = tp->tp_init.tp_alloc.tp_instance_size;
#else /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
		size_t instance_size = DeeType_GetInstanceSize(tp);
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
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
#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
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
#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
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
	self->dw_alloc = Dee_MallocUsableSize(self->dw_ehdr);
	/* The header struct ends at the start of the first heap chunk's user-area */
	self->dw_used  = offsetof(Dec_Ehdr, e_heap.hr_first);
	self->dw_hlast = 0;
	self->dw_align = Dee_HEAPCHUNK_ALIGN;
#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
	DBG_memset(&self->dw_slabb, 0xcc, sizeof(self->dw_slabb));
	self->dw_slabs = 0;
#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
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

#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
#include <deemon/alloc.h>
#include <deemon/asm.h>      /* ASM_RET_NONE, DDI_INSTRLEN_MAX, DDI_STOP, INSTRLEN_MAX, instruction_t */
#include <deemon/bool.h>     /* DeeBool_Type, Dee_FalseTrue */
#include <deemon/callable.h> /* DeeCallable_Type */
#include <deemon/cell.h>     /* DeeCell* */
#include <deemon/class.h>    /* DeeClassDescriptorObject, DeeClassDescriptor_*, Dee_CLASS_ATTRIBUTE_FCLASSMEM, Dee_CLASS_ATTRIBUTE_FMASK, Dee_class_attribute, Dee_class_operator */
#include <deemon/code.h>     /* DeeCodeObject, DeeCode_Malloc, DeeCode_Type, DeeDDIObject, DeeDDI_Empty, DeeDDI_Type, DeeFunctionObject, DeeFunction_Type, Dee_CODE_F*, Dee_CODE_LARGEFRAME_THRESHOLD, Dee_DDI_EXDAT_MAXSIZE, Dee_DDI_REGS_FMASK, Dee_EXCEPTION_HANDLER_FMASK, Dee_code_metrics_init, Dee_ddi_exdat, Dee_except_handler, Dee_hostasm_code_init, Dee_hostasm_function_init, code_addr_t, instruction_t */
#include <deemon/dict.h>     /* DeeDict_New, DeeDict_Type */
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/float.h> /* DeeFloat_New, DeeFloat_Type */
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/hashset.h> /* DeeHashSet_* */
#include <deemon/int.h>     /* DeeInt_* */
#include <deemon/kwds.h>    /* DeeKwds_AppendStringLenHash, DeeKwds_NewWithHint */
#include <deemon/list.h>    /* DeeListObject, DeeList_* */
#include <deemon/map.h>     /* DeeMapping_EmptyInstance, DeeMapping_Type */
#include <deemon/mapfile.h> /* DeeMapFile*, Dee_SIZEOF_DeeMapFile */
#include <deemon/module.h>
#include <deemon/none.h>    /* DeeNone_NewRef, DeeNone_Type */
#include <deemon/numeric.h> /* DeeNumeric_Type */
#include <deemon/rodict.h>  /* Dee_rodict_builder* */
#include <deemon/roset.h>   /* DeeRoSet* */
#include <deemon/seq.h>     /* DeeIterator_Type, DeeSeq_EmptyInstance, DeeSeq_Type */
#include <deemon/set.h>     /* DeeSet_EmptyInstance */
#include <deemon/string.h>
#include <deemon/super.h> /* DeeSuper_Type */
#include <deemon/system-features.h> /* memcpy(), bzero(), ... */
#include <deemon/system.h>
#include <deemon/thread.h>    /* DeeThread_Type */
#include <deemon/traceback.h> /* DeeTraceback_Type */
#include <deemon/tuple.h>     /* DeeTuple* */
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h> /* Dee_ATOMIC_RWLOCK_INIT, Dee_atomic_lock_*, Dee_atomic_rwlock_* */
#include <deemon/weakref.h>   /* DeeWeakRefAble_Type, DeeWeakRef_Type */

#include <hybrid/byteorder.h> /* __BYTE_ORDER__, __ORDER_LITTLE_ENDIAN__ */
#include <hybrid/byteswap.h>  /* HTOLE16_C, LETOH16, LETOH32, UNALIGNED_GETLE* */
#include <hybrid/typecore.h>
#include <hybrid/unaligned.h> /* UNALIGNED_GETLE* */

#include <stdarg.h> /* va_end, va_list, va_start */
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

#ifdef DeeSystem_HAVE_FS_ICASE
#ifndef CONFIG_HAVE_memcasecmp
#define CONFIG_HAVE_memcasecmp
#define memcasecmp dee_memcasecmp
DeeSystem_DEFINE_memcasecmp(dee_memcasecmp)
#endif /* !CONFIG_HAVE_memcasecmp */
#define fs_memcmp      memcasecmp
#define fs_bcmp        memcasecmp
#define fs_hashobj(ob) DeeString_HashCase(Dee_AsObject(ob))
#else /* DeeSystem_HAVE_FS_ICASE */
#define fs_memcmp      memcmp
#define fs_bcmp        bcmp
#define fs_hashobj(ob) DeeString_Hash(Dee_AsObject(ob))
#endif /* !DeeSystem_HAVE_FS_ICASE */



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
		print("\t{ Dee_AsObject(&", typeval, "), DEC_BUILTINID_MAKE(", setname,
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
			print "\t/" "* 0x%.2x *" "/ Dee_AsObject(&%s), /" "* %s *" "/" % (i+0x10, typeval, name);
		}
	}
	print "};";
}
]]]*/
#define NUM_BUILTIN_OBJECT_SETS 1
#define NUM_BUILTIN_OBJECTS     64
PRIVATE struct builtin_desc builtin_descs[NUM_BUILTIN_OBJECTS] = {
	{ Dee_AsObject(&DeeError_Signal), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Signal) },
	{ Dee_AsObject(&DeeError_Interrupt), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Interrupt) },
	{ Dee_AsObject(&DeeError_StopIteration), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_StopIteration) },
	{ Dee_AsObject(&DeeError_Error), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Error) },
	{ Dee_AsObject(&DeeError_AttributeError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_AttributeError) },
	{ Dee_AsObject(&DeeError_UnboundAttribute), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_UnboundAttribute) },
	{ Dee_AsObject(&DeeError_CompilerError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_CompilerError) },
	{ Dee_AsObject(&DeeError_ThreadCrash), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_ThreadCrash) },
	{ Dee_AsObject(&DeeError_RuntimeError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_RuntimeError) },
	{ Dee_AsObject(&DeeError_NotImplemented), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_NotImplemented) },
	{ Dee_AsObject(&DeeError_AssertionError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_AssertionError) },
	{ Dee_AsObject(&DeeError_UnboundLocal), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_UnboundLocal) },
	{ Dee_AsObject(&DeeError_StackOverflow), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_StackOverflow) },
	{ Dee_AsObject(&DeeError_TypeError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_TypeError) },
	{ Dee_AsObject(&DeeError_ValueError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_ValueError) },
	{ Dee_AsObject(&DeeError_ArithmeticError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_ArithmeticError) },
	{ Dee_AsObject(&DeeError_DivideByZero), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_DivideByZero) },
	{ Dee_AsObject(&DeeError_KeyError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_KeyError) },
	{ Dee_AsObject(&DeeError_IndexError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_IndexError) },
	{ Dee_AsObject(&DeeError_UnboundItem), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_UnboundItem) },
	{ Dee_AsObject(&DeeError_SequenceError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_SequenceError) },
	{ Dee_AsObject(&DeeError_UnicodeError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_UnicodeError) },
	{ Dee_AsObject(&DeeError_ReferenceError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_ReferenceError) },
	{ Dee_AsObject(&DeeError_UnpackError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_UnpackError) },
	{ Dee_AsObject(&DeeError_SystemError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_SystemError) },
	{ Dee_AsObject(&DeeError_FSError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_FSError) },
	{ Dee_AsObject(&DeeError_FileAccessError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_FileAccessError) },
	{ Dee_AsObject(&DeeError_FileNotFound), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_FileNotFound) },
	{ Dee_AsObject(&DeeError_FileExists), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_FileExists) },
	{ Dee_AsObject(&DeeError_FileClosed), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_FileClosed) },
	{ Dee_AsObject(&DeeError_NoMemory), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_NoMemory) },
	{ Dee_AsObject(&DeeError_IntegerOverflow), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_IntegerOverflow) },
	{ Dee_AsObject(&DeeError_UnknownKey), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_UnknownKey) },
	{ Dee_AsObject(&DeeError_ItemNotFound), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_ItemNotFound) },
	{ Dee_AsObject(&DeeError_BufferError), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_BufferError) },
	{ Dee_AsObject(&DeeObject_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Object) },
	{ Dee_AsObject(&DeeSeq_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Sequence) },
	{ Dee_AsObject(&DeeMapping_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Mapping) },
	{ Dee_AsObject(&DeeIterator_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Iterator) },
	{ Dee_AsObject(&DeeCallable_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Callable) },
	{ Dee_AsObject(&DeeNumeric_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Numeric) },
	{ Dee_AsObject(&DeeWeakRefAble_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_WeakRefAble) },
	{ Dee_AsObject(&DeeList_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_List) },
	{ Dee_AsObject(&DeeDict_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Dict) },
	{ Dee_AsObject(&DeeHashSet_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_HashSet) },
	{ Dee_AsObject(&DeeCell_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Cell) },
	{ Dee_AsObject(&Dee_FalseTrue[0]), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_False) },
	{ Dee_AsObject(&Dee_FalseTrue[1]), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_True) },
	{ Dee_AsObject(&DeeSeq_EmptyInstance), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_EmptySeq) },
	{ Dee_AsObject(&DeeSet_EmptyInstance), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_EmptySet) },
	{ Dee_AsObject(&DeeMapping_EmptyInstance), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_EmptyMapping) },
	{ Dee_AsObject(&DeeType_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Type) },
	{ Dee_AsObject(&DeeTraceback_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Traceback) },
	{ Dee_AsObject(&DeeThread_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Thread) },
	{ Dee_AsObject(&DeeSuper_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Super) },
	{ Dee_AsObject(&DeeString_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_String) },
	{ Dee_AsObject(&DeeNone_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_None) },
	{ Dee_AsObject(&DeeInt_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Int) },
	{ Dee_AsObject(&DeeFloat_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Float) },
	{ Dee_AsObject(&DeeModule_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Module) },
	{ Dee_AsObject(&DeeCode_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Code) },
	{ Dee_AsObject(&DeeTuple_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Tuple) },
	{ Dee_AsObject(&DeeBool_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_Bool) },
	{ Dee_AsObject(&DeeWeakRef_Type), DEC_BUILTINID_MAKE(0, DEC_BUILTIN_SET0_WeakRef) },
};
PRIVATE DeeObject *buitlin_set0[DTYPE_BUILTIN_NUM] = {
	/* 0x10 */ Dee_AsObject(&DeeError_Signal), /* Signal */
	/* 0x11 */ Dee_AsObject(&DeeError_Interrupt), /* Interrupt */
	/* 0x12 */ Dee_AsObject(&DeeError_StopIteration), /* StopIteration */
	/* 0x13 */ NULL,
	/* 0x14 */ NULL,
	/* 0x15 */ NULL,
	/* 0x16 */ NULL,
	/* 0x17 */ NULL,
	/* 0x18 */ Dee_AsObject(&DeeError_Error), /* Error */
	/* 0x19 */ Dee_AsObject(&DeeError_AttributeError), /* AttributeError */
	/* 0x1a */ Dee_AsObject(&DeeError_UnboundAttribute), /* UnboundAttribute */
	/* 0x1b */ Dee_AsObject(&DeeError_CompilerError), /* CompilerError */
	/* 0x1c */ Dee_AsObject(&DeeError_ThreadCrash), /* ThreadCrash */
	/* 0x1d */ Dee_AsObject(&DeeError_RuntimeError), /* RuntimeError */
	/* 0x1e */ Dee_AsObject(&DeeError_NotImplemented), /* NotImplemented */
	/* 0x1f */ Dee_AsObject(&DeeError_AssertionError), /* AssertionError */
	/* 0x20 */ Dee_AsObject(&DeeError_UnboundLocal), /* UnboundLocal */
	/* 0x21 */ Dee_AsObject(&DeeError_StackOverflow), /* StackOverflow */
	/* 0x22 */ Dee_AsObject(&DeeError_TypeError), /* TypeError */
	/* 0x23 */ Dee_AsObject(&DeeError_ValueError), /* ValueError */
	/* 0x24 */ Dee_AsObject(&DeeError_ArithmeticError), /* ArithmeticError */
	/* 0x25 */ Dee_AsObject(&DeeError_DivideByZero), /* DivideByZero */
	/* 0x26 */ Dee_AsObject(&DeeError_KeyError), /* KeyError */
	/* 0x27 */ Dee_AsObject(&DeeError_IndexError), /* IndexError */
	/* 0x28 */ Dee_AsObject(&DeeError_UnboundItem), /* UnboundItem */
	/* 0x29 */ Dee_AsObject(&DeeError_SequenceError), /* SequenceError */
	/* 0x2a */ Dee_AsObject(&DeeError_UnicodeError), /* UnicodeError */
	/* 0x2b */ Dee_AsObject(&DeeError_ReferenceError), /* ReferenceError */
	/* 0x2c */ Dee_AsObject(&DeeError_UnpackError), /* UnpackError */
	/* 0x2d */ Dee_AsObject(&DeeError_SystemError), /* SystemError */
	/* 0x2e */ Dee_AsObject(&DeeError_FSError), /* FSError */
	/* 0x2f */ Dee_AsObject(&DeeError_FileAccessError), /* FileAccessError */
	/* 0x30 */ Dee_AsObject(&DeeError_FileNotFound), /* FileNotFound */
	/* 0x31 */ Dee_AsObject(&DeeError_FileExists), /* FileExists */
	/* 0x32 */ Dee_AsObject(&DeeError_FileClosed), /* FileClosed */
	/* 0x33 */ Dee_AsObject(&DeeError_NoMemory), /* NoMemory */
	/* 0x34 */ Dee_AsObject(&DeeError_IntegerOverflow), /* IntegerOverflow */
	/* 0x35 */ Dee_AsObject(&DeeError_UnknownKey), /* UnknownKey */
	/* 0x36 */ Dee_AsObject(&DeeError_ItemNotFound), /* ItemNotFound */
	/* 0x37 */ Dee_AsObject(&DeeError_BufferError), /* BufferError */
	/* 0x38 */ NULL,
	/* 0x39 */ NULL,
	/* 0x3a */ NULL,
	/* 0x3b */ NULL,
	/* 0x3c */ NULL,
	/* 0x3d */ NULL,
	/* 0x3e */ NULL,
	/* 0x3f */ NULL,
	/* 0x40 */ Dee_AsObject(&DeeObject_Type), /* Object */
	/* 0x41 */ Dee_AsObject(&DeeSeq_Type), /* Sequence */
	/* 0x42 */ Dee_AsObject(&DeeMapping_Type), /* Mapping */
	/* 0x43 */ Dee_AsObject(&DeeIterator_Type), /* Iterator */
	/* 0x44 */ Dee_AsObject(&DeeCallable_Type), /* Callable */
	/* 0x45 */ Dee_AsObject(&DeeNumeric_Type), /* Numeric */
	/* 0x46 */ Dee_AsObject(&DeeWeakRefAble_Type), /* WeakRefAble */
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
	/* 0x60 */ Dee_AsObject(&DeeList_Type), /* List */
	/* 0x61 */ Dee_AsObject(&DeeDict_Type), /* Dict */
	/* 0x62 */ Dee_AsObject(&DeeHashSet_Type), /* HashSet */
	/* 0x63 */ Dee_AsObject(&DeeCell_Type), /* Cell */
	/* 0x64 */ NULL,
	/* 0x65 */ NULL,
	/* 0x66 */ NULL,
	/* 0x67 */ NULL,
	/* 0x68 */ Dee_AsObject(&Dee_FalseTrue[0]), /* False */
	/* 0x69 */ Dee_AsObject(&Dee_FalseTrue[1]), /* True */
	/* 0x6a */ Dee_AsObject(&DeeSeq_EmptyInstance), /* EmptySeq */
	/* 0x6b */ Dee_AsObject(&DeeSet_EmptyInstance), /* EmptySet */
	/* 0x6c */ Dee_AsObject(&DeeMapping_EmptyInstance), /* EmptyMapping */
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
	/* 0xc0 */ Dee_AsObject(&DeeType_Type), /* Type */
	/* 0xc1 */ Dee_AsObject(&DeeTraceback_Type), /* Traceback */
	/* 0xc2 */ Dee_AsObject(&DeeThread_Type), /* Thread */
	/* 0xc3 */ Dee_AsObject(&DeeSuper_Type), /* Super */
	/* 0xc4 */ Dee_AsObject(&DeeString_Type), /* String */
	/* 0xc5 */ Dee_AsObject(&DeeNone_Type), /* None */
	/* 0xc6 */ Dee_AsObject(&DeeInt_Type), /* Int */
	/* 0xc7 */ Dee_AsObject(&DeeFloat_Type), /* Float */
	/* 0xc8 */ Dee_AsObject(&DeeModule_Type), /* Module */
	/* 0xc9 */ Dee_AsObject(&DeeCode_Type), /* Code */
	/* 0xca */ NULL,
	/* 0xcb */ NULL,
	/* 0xcc */ NULL,
	/* 0xcd */ NULL,
	/* 0xce */ NULL,
	/* 0xcf */ NULL,
	/* 0xd0 */ Dee_AsObject(&DeeTuple_Type), /* Tuple */
	/* 0xd1 */ Dee_AsObject(&DeeBool_Type), /* Bool */
	/* 0xd2 */ Dee_AsObject(&DeeWeakRef_Type), /* WeakRef */
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

struct Dee_module_object;
struct Dee_compiler_options;
struct Dee_string_object;
struct Dee_code_object;
struct Dee_ddi_object;

typedef struct {
	union {
		uint8_t const             *df_data;    /* [0..df_size+DECFILE_PADDING]
		                                        * A full mapping of all data from the input DEC file, followed
		                                        * by a couple of bytes of padding data that is ZERO-initialized. */
		uint8_t const             *df_base;    /* [0..df_size] Base address of the DEC image mapped into host memory. */
		Dec_Ehdr const            *df_ehdr;    /* [0..1] A pointer to the DEC file header mapped into host memory. */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define df_data _dee_aunion.df_data
#define df_base _dee_aunion.df_base
#define df_ehdr _dee_aunion.df_ehdr
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	size_t                         df_size;    /* Total number of usable bytes of memory
	                                            * that can be found within the source file. */
	DREF struct Dee_string_object *df_name;    /* [1..1] The filename of the `*.dee' file opened by this descriptor. */
	DREF struct Dee_module_object *df_module;  /* [1..1] The module that is being loaded. */
	struct Dee_compiler_options   *df_options; /* [0..1] Compilation options. */
	DREF struct Dee_string_object *df_strtab;  /* [0..1] Lazily allocated copy of the string table.
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
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
             struct Dee_module_object *__restrict module,
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
             struct Dee_string_object *__restrict dec_pathname,
             struct Dee_compiler_options *options);
PRIVATE NONNULL((1)) void DCALL
DecFile_Fini(DecFile *__restrict self);

/* Return a string for the entire strtab of a given DEC-file.
 * Upon error, NULL is returned.
 * NOTE: The return value is _NOT_ a reference! */
PRIVATE WUNUSED NONNULL((1)) DeeObject *DCALL
DecFile_Strtab(DecFile *__restrict self);

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
/* Check if a given DEC file is up to date, or if it must not be loaded.
 * because it a dependency has changed since it was created.
 * @return:  0: The file is up-to-date.
 * @return:  1: The file is not up-to-date.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DecFile_IsUpToDate(DecFile *__restrict self);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

/* Load a given DEC file and fill in the given `module'.
 * @return:  0: Successfully loaded the given DEC file.
 * @return:  1: The DEC file has been corrupted or is out of date.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DecFile_Load(DecFile *__restrict self);

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
/* Return the last-modified time (in microseconds since 01-01-1970).
 * For this purpose, an internal cache is kept that is consulted
 * before and populated after making an attempt at contacting the
 * host operating system for the required information.
 * @return: * :           Last-modified time (in microseconds since 01-01-1970).
 * @return: 0 :           The given file could not be found.
 * @return: (uint64_t)-1: The lookup failed and an error was thrown. */
PRIVATE WUNUSED NONNULL((1)) uint64_t DCALL
DecTime_Lookup(DeeObject *__restrict filename);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

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
PRIVATE WUNUSED NONNULL((1, 2)) DREF struct Dee_code_object *DCALL
DecFile_LoadCode(DecFile *__restrict self,
                 uint8_t const **__restrict p_reader);

/* @return: * :        New reference to a ddi object.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The DEC file has been corrupted. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF struct Dee_ddi_object *DCALL
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
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
             DeeModuleObject *__restrict module,
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
             DeeStringObject *__restrict dec_pathname,
             struct Dee_compiler_options *options) {
	Dec_Ehdr *hdr;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	ASSERT_OBJECT_TYPE(module, &DeeModule_Type);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
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
	hdr = (Dec_Ehdr *)DeeMapFile_GetAddr(input);
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
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	self->df_module = NULL;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	self->df_module = module;
	Dee_Incref(module);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	self->df_name = dec_pathname;
	Dee_Incref(dec_pathname);
	return 0;
end_not_a_dec:
	return 1;
}

PRIVATE NONNULL((1)) void DCALL
DecFile_Fini(DecFile *__restrict self) {
	Dee_XDecref(self->df_strtab);
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	Dee_Decref(self->df_module);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
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
	return Dee_AsObject(result);
}




#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
/* Check if a given DEC file is up to date, or if it must not be loaded.
 * because it a dependency has changed since it was created.
 * @return:  0: The file is up-to-date.
 * @return:  1: The file is not up-to-date.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DecFile_IsUpToDate(DecFile *__restrict self) {
	Dec_Ehdr const *hdr = self->df_ehdr;
	uint64_t timestamp, other;
	other = DecTime_Lookup(Dee_AsObject(self->df_name));
	if unlikely(other == (uint64_t)-1)
		goto err;
	timestamp = (((uint64_t)LETOH32(hdr->e_timestamp_hi) << 32) |
	             ((uint64_t)LETOH32(hdr->e_timestamp_lo)));
	if (other > timestamp)
		goto changed; /* Base source file has changed. */
#if 0 /* TO-DO Will never be implemented */
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
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */




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
	module_pathstr = DeeString_AsUtf8(Dee_AsObject(self->df_name));
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
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		/* Important: don't set "DeeModule_IMPORT_F_CTXDIR" flag since "module_pathstr" still ends with a trailing slash! */
		module = DeeModule_OpenEx(module_name, strlen(module_name),
		                          module_pathstr, module_pathlen,
		                          DeeModule_IMPORT_F_ENOENT /*| DeeModule_IMPORT_F_CTXDIR*/,
		                          self->df_options ? self->df_options->co_inner : NULL);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		module = DeeModule_OpenRelativeString(module_name, strlen(module_name),
		                                      module_pathstr, module_pathlen,
		                                      self->df_options ? self->df_options->co_inner : NULL,
		                                      false);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
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
			                "Dependency %q of module %r could not be found",
			                strtab + off, self->df_name);
			goto err_imports;
		}

		/* Check if the module has changed. */
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		if (!self->df_options || !(self->df_options->co_decloader & Dee_DEC_FLOADOUTDATED))
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		{
			uint64_t modtime = DeeModule_GetCTime(module);
			if unlikely(modtime == (uint64_t)-1)
				GOTO_CORRUPTED(reader, err_imports_module);
			/* If the module has changed since the time
			 * described on the DEC header, stop loading. */
			if unlikely(modtime > timestamp) {
				GOTO_CORRUPTEDF(reader, stop_imports_module,
				                "Dependency %q of module %r changed after .dec "
				                "file was created (%" PRFu64 " > %" PRFu64 ")",
				                strtab + off, self->df_name, modtime, timestamp);
			}
		}
		*moditer = module; /* Inherit */
	}
	/* Write the module import table. */
	self->df_module->mo_importc = importc;
	self->df_module->mo_importv = importv; /* Inherit. */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	self->df_module->mo_ctime = timestamp;
	self->df_module->mo_flags |= Dee_MODULE_FHASCTIME;
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	result = 0;
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

INTDEF struct Dee_module_symbol empty_module_buckets[];

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
PRIVATE WUNUSED DREF DeeModuleObject *DCALL
DecFile_CreateModule(uint16_t num_globals) {
	DREF DeeModuleObject *result;
	size_t size = _Dee_MallococBufsize(offsetof(DeeModuleObject, mo_globalv),
	                                   num_globals, sizeof(DREF DeeObject *));
	result = (DREF DeeModuleObject *)DeeGCObject_Calloc(size);
	if likely(result) {
		result->mo_globalc = num_globals;
		Dee_atomic_rwlock_cinit(&result->mo_lock);
		Dee_weakref_support_cinit(result);
		DeeObject_Init(result, &DeeModuleDee_Type);
	}
	return result;
}

PRIVATE WUNUSED DREF DeeModuleObject *DCALL
DecFile_CreateEmptyModule(void) {
	DREF DeeModuleObject *result = DecFile_CreateModule(0);
	if likely(result) {
		/* Install the symbol mask and hash-table. */
		Dee_ASSERT(result->mo_bucketm == 0);
		result->mo_bucketv = empty_module_buckets;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF /*untracked*/ DeeModuleObject *DCALL
DecFile_LoadGlobals(DecFile *__restrict self)
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DecFile_LoadGlobals(DecFile *__restrict self)
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
{
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	int result = 1;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	Dec_Glbmap const *glbmap;
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	DREF DeeModuleObject *module = (DREF DeeModuleObject *)ITER_DONE;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	DeeModuleObject *module = self->df_module;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	Dec_Ehdr const *hdr = self->df_ehdr;
	uint16_t i, globalc, symbolc;
	uint8_t const *end = self->df_base + self->df_size;
	uint8_t const *reader;
	uint16_t bucket_mask;
	struct Dee_module_symbol *bucketv;
	char const *strtab;
	/* Quick check: Without a global variable table, nothing needs to be loaded. */
	if (!hdr->e_globoff) {
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		return DecFile_CreateEmptyModule();
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		return 0;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	}

	/* Load the global object table. */
	glbmap  = (Dec_Glbmap const *)(self->df_base + LETOH32(hdr->e_globoff));
	globalc = UNALIGNED_GETLE16(&glbmap->g_cnt);
	symbolc = UNALIGNED_GETLE16(&glbmap->g_len);
	if unlikely(globalc > symbolc)
		GOTO_CORRUPTED(&glbmap->g_cnt, stop);
	if unlikely(!symbolc) {
		/* Unlikely, but allowed. */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		return DecFile_CreateEmptyModule();
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		return 0;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	}
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
	bucketv = (struct Dee_module_symbol *)Dee_Callocc(bucket_mask + 1,
	                                              sizeof(struct Dee_module_symbol));
	if unlikely(!bucketv)
		goto err;

	/* Read symbol information. */
	for (i = 0; i < symbolc; ++i) {
		uint8_t flags, addr2;
		uint16_t addr;
		char const *name, *doc;
		uint32_t doclen;
		Dee_hash_t name_hash, hash_i, perturb;
		if unlikely(reader >= end)
			GOTO_CORRUPTED(reader, stop_symbolv); /* Validate bounds. */
		flags = (uint8_t)UNALIGNED_GETLE16(reader), reader += 2;
		if (flags & ~Dee_MODSYM_FMASK)
			GOTO_CORRUPTED(reader, stop_symbolv); /* Unknown flags are being used. */
		/* The first `globalc' descriptors lack the `s_addr' field. */
		addr2 = (uint8_t)-1;
		if (i < globalc) {
			addr = i;
		} else {
			addr = UNALIGNED_GETLE16(reader), reader += 2;
			if (flags & Dee_MODSYM_FEXTERN) {
				addr2 = (uint8_t)UNALIGNED_GETLE16(reader), reader += 2;
				if (!(flags & Dee_MODSYM_FPROPERTY)) {
					/* Validate module index. */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
					Dec_Strmap const *impmap  = (Dec_Strmap const *)(self->df_base + LETOH32(hdr->e_impoff));
					uint16_t importc = UNALIGNED_GETLE16(&impmap->i_len);
					if (addr2 >= importc)
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
					if (addr2 >= self->df_module->mo_importc)
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
					{
						GOTO_CORRUPTED(reader, stop_symbolv);
					}
				}
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
			struct Dee_module_symbol *target = &bucketv[hash_i & bucket_mask];
			if (target->ss_name)
				continue;
			/* Found an unused slot compatible with the hash of this symbol's name. */
			temp = (DREF DeeStringObject *)DeeString_New(name);
			if unlikely(!temp)
				goto err_symbolv;
			temp->s_hash    = name_hash; /* Save the name hash. */
			target->ss_name = DeeString_STR(temp);
			flags |= Dee_MODSYM_FNAMEOBJ;
			if (doclen) {
				/* Allocate the documentation string. */
				temp = (DREF DeeStringObject *)DeeString_NewUtf8(doc,
				                                                 doclen,
				                                                 STRING_ERROR_FSTRICT);
				if unlikely(!temp)
					goto err_symbolv;
				target->ss_doc = DeeString_STR(temp);
				flags |= Dee_MODSYM_FDOCOBJ;
			}
			target->ss_index = addr;
			target->ss_impid = (uint8_t)addr2;
			target->ss_hash  = name_hash;
			target->ss_flags = flags;
			break;
		}
	}

	/* Allocate and setup the global variable vector. */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	module = DecFile_CreateModule(globalc);
	if unlikely(!module)
		goto err_symbolv;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	module->mo_globalv = (DREF DeeObject **)Dee_Callocc(globalc, sizeof(DREF DeeObject *));
	if unlikely(!module->mo_globalv)
		goto err_symbolv;
	module->mo_globalc = globalc;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

	/* Install the symbol mask and hash-table. */
	module->mo_bucketm = bucket_mask;
	module->mo_bucketv = bucketv;

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	result = 0;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
stop:
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	return module;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return result;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
err_symbolv:
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	module = NULL;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	result = -1;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
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
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	module = NULL;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	result = -1;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	goto stop;
}


INTDEF struct Dee_class_operator empty_class_operators[];
INTDEF struct Dee_class_attribute empty_class_attributes[];


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
		result = Dee_AsObject(DecFile_LoadCode(self, &reader));
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
		DeeObject_Init((DeeFunctionObject *)result, &DeeFunction_Type);
		Dee_atomic_rwlock_init(&((DREF DeeFunctionObject *)result)->fo_reflock);
		result = DeeGC_Track(result);
	}	break;

	case DTYPE_TUPLE: {
		uint32_t i, length;
		uint8_t const *end;
		length = Dec_DecodePointer(&reader);
		result = Dee_AsObject(DeeTuple_NewUninitialized(length));
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
		                                             iattr_mask + 1, sizeof(struct Dee_class_attribute));
		if unlikely(!result)
			goto err;
		DeeObject_Init(descriptor, &DeeClassDescriptor_Type);
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
			struct Dee_class_operator *opbind_list;
			/* Load the operator descriptor table. */
			if (reader + op_count * 2 > (uint8_t *)fileend)
				GOTO_CORRUPTED(reader, corrupt_r);
			opbind_mask = 0;
			while (op_count > (opbind_mask / 3) * 2)
				opbind_mask = (opbind_mask << 1) | 1;
			opbind_list = (struct Dee_class_operator *)Dee_Mallocc(opbind_mask + 1,
			                                                   sizeof(struct Dee_class_operator));
			if unlikely(!opbind_list)
				goto err_r;
			memset(opbind_list, 0xff,
			       (opbind_mask + 1) *
			       sizeof(struct Dee_class_operator));
			descriptor->cd_clsop_mask = opbind_mask;
			descriptor->cd_clsop_list = opbind_list;
			for (i = 0; i < op_count; ++i) {
				struct Dee_class_operator *entry;
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
			struct Dee_class_attribute *cattr_list;
			/* Load the class attribute descriptor table. */
			cattr_mask = 0;
			while (cattr_count > (cattr_mask / 3) * 2)
				cattr_mask = (cattr_mask << 1) | 1;
			cattr_list = (struct Dee_class_attribute *)Dee_Callocc(cattr_mask + 1,
			                                                   sizeof(struct Dee_class_attribute));
			if unlikely(!cattr_list)
				goto err_r;
			descriptor->cd_cattr_list = cattr_list;
			descriptor->cd_cattr_mask = cattr_mask;
			for (i = 0; i < cattr_count; ++i) {
				struct Dee_class_attribute *entry;
				Dee_hash_t j, perturb, hash;
				uint8_t ataddr, atflags;
				DREF DeeStringObject *name_ob;
				if unlikely(reader >= (uint8_t const *)fileend)
					GOTO_CORRUPTED(reader, corrupt_r);
				ataddr  = UNALIGNED_GETLE8(reader), reader += 1;
				atflags = UNALIGNED_GETLE8(reader), reader += 1;
				if unlikely(atflags & ~Dee_CLASS_ATTRIBUTE_FMASK)
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
			struct Dee_class_attribute *entry;
			Dee_hash_t j, perturb, hash;
			uint8_t ataddr, atflags;
			DREF DeeStringObject *name_ob;
			if unlikely(reader >= (uint8_t const *)fileend)
				GOTO_CORRUPTED(reader, corrupt_r);
			ataddr  = UNALIGNED_GETLE8(reader), reader += 1;
			atflags = UNALIGNED_GETLE8(reader), reader += 1;
			if unlikely(atflags & ~Dee_CLASS_ATTRIBUTE_FMASK)
				GOTO_CORRUPTED(reader, corrupt_r);
			if unlikely(ataddr >= ((atflags & Dee_CLASS_ATTRIBUTE_FCLASSMEM) ? cmemb_size : imemb_size))
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
			result    = Dee_AsObject(DeeRoSet_NewWithHint(num_items));
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
			result = Dee_AsObject(Dee_rodict_builder_pack(&result_builder));
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
			                                             iattr_mask + 1, sizeof(struct Dee_class_attribute));
			if unlikely(!result)
				goto err;
			DeeObject_Init(descriptor, &DeeClassDescriptor_Type);
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
				struct Dee_class_operator *opbind_list;
				/* Load the operator descriptor table. */
				if (reader + op_count * 4 > (uint8_t *)fileend)
					GOTO_CORRUPTED(reader, corrupt_r);
				opbind_mask = 0;
				while (op_count > (opbind_mask / 3) * 2)
					opbind_mask = (opbind_mask << 1) | 1;
				opbind_list = (struct Dee_class_operator *)Dee_Mallocc(opbind_mask + 1,
				                                                   sizeof(struct Dee_class_operator));
				if unlikely(!opbind_list)
					goto err_r;
				memset(opbind_list, 0xff,
				       (opbind_mask + 1) *
				       sizeof(struct Dee_class_operator));
				descriptor->cd_clsop_mask = opbind_mask;
				descriptor->cd_clsop_list = opbind_list;
				for (i = 0; i < op_count; ++i) {
					struct Dee_class_operator *entry;
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
				struct Dee_class_attribute *cattr_list;
				/* Load the class attribute descriptor table. */
				cattr_mask = 0;
#if __SIZEOF_POINTER__ < 8
				if unlikely(cattr_count > (((size_t)-1) / 3) * 2)
					GOTO_CORRUPTED(reader, corrupt_r);
#endif /* __SIZEOF_POINTER__ < 8 */
				while (cattr_count > (cattr_mask / 3) * 2)
					cattr_mask = (cattr_mask << 1) | 1;
				cattr_list = (struct Dee_class_attribute *)Dee_Callocc(cattr_mask + 1,
				                                                   sizeof(struct Dee_class_attribute));
				if unlikely(!cattr_list)
					goto err_r;
				descriptor->cd_cattr_list = cattr_list;
				descriptor->cd_cattr_mask = cattr_mask;
				for (i = 0; i < cattr_count; ++i) {
					struct Dee_class_attribute *entry;
					Dee_hash_t j, perturb, hash;
					uint16_t ataddr, atflags;
					DREF DeeStringObject *name_ob;
					if unlikely(reader >= (uint8_t const *)fileend)
						GOTO_CORRUPTED(reader, corrupt_r);
					ataddr  = UNALIGNED_GETLE16(reader), reader += 2;
					atflags = UNALIGNED_GETLE16(reader), reader += 2;
					if unlikely(atflags & ~Dee_CLASS_ATTRIBUTE_FMASK)
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
				struct Dee_class_attribute *entry;
				Dee_hash_t j, perturb, hash;
				uint16_t ataddr, atflags;
				DREF DeeStringObject *name_ob;
				if unlikely(reader >= (uint8_t const *)fileend)
					GOTO_CORRUPTED(reader, corrupt_r);
				ataddr  = UNALIGNED_GETLE16(reader), reader += 2;
				atflags = UNALIGNED_GETLE16(reader), reader += 2;
				if unlikely(atflags & ~Dee_CLASS_ATTRIBUTE_FMASK)
					GOTO_CORRUPTED(reader, corrupt_r);
				if unlikely(ataddr >= ((atflags & Dee_CLASS_ATTRIBUTE_FCLASSMEM) ? cmemb_size : imemb_size))
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
	if (result->d_start.dr_flags & ~Dee_DDI_REGS_FMASK)
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
			                                                xsiz, Dee_DDI_EXDAT_MAXSIZE);
			if unlikely(!xres)
				goto err_r_maps;
			xres->dx_size = xsiz;
			/* Initialize X-data information. */
			bssptr = mempcpy(xres->dx_data, xdat, xsiz);
			bzero(bssptr, Dee_DDI_EXDAT_MAXSIZE);
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
	if (header.co_flags & ~(Dee_CODE_FMASK | Dee_CODE_FDEC_8BIT))
		GOTO_CORRUPTED(reader, corrupt);
	if (header.co_flags & Dee_CODE_FDEC_8BIT) {
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

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	if (self->df_options &&
	    (self->df_options->co_decloader & Dee_DEC_FUNTRUSTED)) {
		/* The origin of the code cannot be trusted and we must append
		 * a couple of trailing instruction bytes to the code object. */

		/* Allocate the resulting code object, as well as set the Dee_CODE_FASSEMBLY flag. */
		header.co_flags |= Dee_CODE_FASSEMBLY;
		result = DeeCode_Malloc(header.co_textsiz + INSTRLEN_MAX);
		if likely(result) {
			/* Initialize trailing bytes as `ret none' instructions. */
#if ASM_RET_NONE == 0
			bzero(result->co_code + header.co_textsiz, INSTRLEN_MAX);
#else /* ASM_RET_NONE == 0 */
			memset(result->co_code + header.co_textsiz, ASM_RET_NONE, INSTRLEN_MAX);
#endif /* ASM_RET_NONE != 0 */
		}
	} else
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	{
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
		struct Dee_except_handler *exceptv;
		except_reader = self->df_base + header.co_exceptoff;
		if unlikely(except_reader >= end) /* Validate bounds */
			GOTO_CORRUPTED(except_reader, corrupt_r_static);
		is8bit = !!(header.co_flags & Dee_CODE_FDEC_8BIT);
		if (is8bit) {
			count = UNALIGNED_GETLE8(except_reader);
			except_reader += 1;
		} else {
			count = UNALIGNED_GETLE16(except_reader);
			except_reader += 2;
		}

		/* Allocate the exception vector. */
		exceptv = (struct Dee_except_handler *)Dee_Mallocc(count, sizeof(struct Dee_except_handler));
		if unlikely(!exceptv)
			goto err_r_static;

		/* Write the exception descriptors to the resulting code object. */
		result->co_exceptv = exceptv;

		/* Load all the exception handlers. */
		for (result->co_exceptc = 0;
		     result->co_exceptc < count; ++result->co_exceptc) {
			struct Dee_except_handler *hand;
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
			if (hand->eh_flags & ~Dee_EXCEPTION_HANDLER_FMASK)
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
		ddi = DecFile_LoadDDI(self, ddi_reader, !!(header.co_flags & Dee_CODE_FDEC_8BIT));
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
		if (header.co_flags & Dee_CODE_FDEC_8BIT) {
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
	if unlikely(result->co_framesize >= Dee_CODE_LARGEFRAME_THRESHOLD)
		result->co_flags |= Dee_CODE_FHEAPFRAME;

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

/* Load a given DEC file and fill in the given `module'.
 * @return:  0: Successfully loaded the given DEC file.
 * @return:  1: The DEC file has been corrupted or is out of date.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DecFile_Load(DecFile *__restrict self) {
	/*untracked*/ DeeModuleObject *module;
	int result;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	module = self->df_module;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

	/* Load global variables related to this module. */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	module = DecFile_LoadGlobals(self);
	if (!ITER_ISOK(module))
		return module ? 1 : -1;
	self->df_module = module;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	result = DecFile_LoadGlobals(self);
	if (result != 0)
		goto err;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

	/* Load the module import table and all collect all dependency modules. */
	result = DecFile_LoadImports(self);
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
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		module->mo_moddata.mo_rootcode = root_code; /* Inherit. */
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		module->mo_root = root_code; /* Inherit. */
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	}

	return 0;
err:
	{
		DREF DeeCodeObject *root;
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		root = module->mo_moddata.mo_rootcode;
		module->mo_moddata.mo_rootcode = NULL;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		root = module->mo_root;
		module->mo_root = NULL;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		Dee_XDecref(root);
	}

	/* Free the module's global variable vector.
	 * NOTE: At this point, we can still assume that it was filled with all NULLs. */
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	ASSERT((module->mo_globalv != NULL) ==
	       (module->mo_globalc != 0));
	Dee_Free(module->mo_globalv);
	module->mo_globalv = NULL;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	module->mo_globalc = 0;

	/* Free the module's symbol table. */
	ASSERT((module->mo_bucketv != empty_module_buckets) ==
	       (module->mo_bucketm != 0));
	if (module->mo_bucketm) {
		do {
			if (module->mo_bucketv[module->mo_bucketm].ss_name) {
				if (module->mo_bucketv[module->mo_bucketm].ss_flags & Dee_MODSYM_FNAMEOBJ)
					Dee_Decref(COMPILER_CONTAINER_OF(module->mo_bucketv[module->mo_bucketm].ss_name, DeeStringObject, s_str));
				if (module->mo_bucketv[module->mo_bucketm].ss_flags & Dee_MODSYM_FDOCOBJ)
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
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	DeeGCObject_Free(module);
	self->df_module = NULL;
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return result;
}

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
/* @param: dec_dirname:     Absolute filename of the corresponding ".dee" file (ZERO-terminated)
 * @param: dec_dirname_len: Offset (in bytes) from "dec_dirname" where the filename portion begins
 * @return:  * :       Successfully loaded the given DEC file. The caller must still initialize:
 *                      - return->mo_absname  (Current set to "NULL")
 *                      - return->mo_absnode
 *                      - return->mo_libname  (Current set to "NULL")
 *                      - return->mo_libnode
 * @return: ITER_DONE: The DEC file was out of date or had been corrupted.
 * @return: NULL:      An error occurred. */
INTERN WUNUSED NONNULL((1)) DREF /*untracked*/ struct Dee_module_object *DCALL
DeeModule_OpenDec(DeeObject *__restrict input_stream, struct Dee_compiler_options *options,
                  /*utf-8*/ char const *__restrict dec_dirname, size_t dec_dirname_len,
                  uint64_t dee_file_last_modified)
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
/* @return:  0: Successfully loaded the given DEC file.
 * @return:  1: The DEC file was out of date or had been corrupted.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_OpenDec(struct Dee_module_object *__restrict mod,
                  DeeObject *__restrict input_stream,
                  struct Dee_compiler_options *options)
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
{
	DecFile file;
	struct DeeMapFile filemap;
	int result;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	ASSERT(mod->mo_path);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

	/* Initialize the file */
	if unlikely(DeeMapFile_InitFile(&filemap, input_stream,
	                                0, 0, DFILE_LIMIT + 1, DECFILE_PADDING,
	                                DeeMapFile_F_READALL | DeeMapFile_F_ATSTART)) {
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		return NULL;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		return -1;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	}

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	{
		DREF DeeObject *path;
		(void)dec_dirname_len;
		/* Use the full "strlen(dec_dirname)", which is the filename of the ".dee" file */
		path = DeeString_NewUtf8(dec_dirname, strlen(dec_dirname), STRING_ERROR_FSTRICT);
		if unlikely(!path) {
			DeeMapFile_Fini(&filemap);
			return NULL;
		}
		result = DecFile_Init(&file, &filemap, (DeeStringObject *)path, options);
		Dee_Decref(path);
		if (result != 0)
			file.df_module = NULL; /* Hack -- needed for cleanup path below */
	}
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	result = DecFile_Init(&file, &filemap, mod, mod->mo_path, options);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

	if unlikely(result != 0)
		goto done_map;
	Dee_DPRINTF("[LD] Opened dec file for %r\n", file.df_name);

	/* Check if the file is up-to-date (unless this check is being suppressed). */
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	{
		uint64_t timestamp;
		timestamp = (((uint64_t)LETOH32(file.df_ehdr->e_timestamp_hi) << 32) |
		             ((uint64_t)LETOH32(file.df_ehdr->e_timestamp_lo)));
		if (dee_file_last_modified > timestamp) {
			Dee_DPRINTF("[LD] Dec file for %r is out-of-date\n", file.df_name);
			result = 1;
			goto done_map_file;
		}
	}
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	if (!options || !(options->co_decloader & Dee_DEC_FLOADOUTDATED)) {
		result = DecFile_IsUpToDate(&file);
		if (result != 0) {
			Dee_DPRINTF("[LD] Dec file for %r is out-of-date\n", file.df_name);
			goto done_map_file;
		}
	}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

	/* With all that out of the way, actually load the file. */
	result = DecFile_Load(&file);

	/* Translate certain errors into the "module-corrupted" status */
	if (result < 0) {
		if (DeeError_Catch(&DeeError_ValueError))
			result = 1;
	}
#ifndef Dee_DPRINT_IS_NOOP
	if unlikely(result > 0)
		Dee_DPRINTF("[LD] Dec file for %r is corrupted\n", file.df_name);
#endif /* !Dee_DPRINT_IS_NOOP */
done_map_file:
	DecFile_Fini(&file);
done_map:
	DeeMapFile_Fini(&filemap);
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	if (result == 0) {
		ASSERT(ITER_ISOK(file.df_module));
		return file.df_module;
	}
	Dee_XDecref(file.df_module);
	if (result > 0)
		return (DREF struct Dee_module_object *)ITER_DONE;
	return NULL;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	return result;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
}

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
PUBLIC WUNUSED NONNULL((1)) uint64_t DCALL
DeeModule_GetCTime(DeeModuleObject *__restrict self) {
	uint64_t result;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	if (self->mo_flags & Dee_MODULE_FHASCTIME) {
		result = self->mo_ctime;
		ASSERT(result != (uint64_t)-1);
	} else if (self == &DeeModule_Deemon) {
		/* `DeeExec_GetTimestamp()' already uses the `mo_ctime' field
		 *  of `DeeModule_Deemon' as cache if that field is available. */
		result = DeeExec_GetTimestamp();
	} else {
		/* Lookup the last-modified time of the module's path file. */
		DeeStringObject *path = self->mo_path;
		if unlikely(!path) {
			result = 0;
		} else {
			result = DecTime_Lookup(Dee_AsObject(path));
			if unlikely(result == (uint64_t)-1)
				goto done;
		}
		/* Cache the result value in the module itself. */
		self->mo_ctime = result;
		atomic_or(&self->mo_flags, Dee_MODULE_FHASCTIME);
	}
done:
	return result;
}
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */


#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
struct mtime_entry {
	DREF DeeStringObject *me_file; /* [0..1] Absolute, normalized filename.
	                                *  NOTE: When NULL, then this entry is unused. */
#ifdef DeeSystem_HAVE_FS_ICASE
	Dee_hash_t               me_casehash; /* Case-insensitive hash for `me_file' */
#endif /* DeeSystem_HAVE_FS_ICASE */
	uint64_t              me_mtim; /* Last-modified time of `me_file' */
};

#ifdef DeeSystem_HAVE_FS_ICASE
#define MTIME_ENTRY_HASH(x) ((x)->me_casehash)
#else /* DeeSystem_HAVE_FS_ICASE */
#define MTIME_ENTRY_HASH(x) DeeString_HASH((x)->me_file)
#endif /* !DeeSystem_HAVE_FS_ICASE */

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
#ifdef DeeSystem_HAVE_FS_ICASE
			item->me_casehash = hash;
#endif /* DeeSystem_HAVE_FS_ICASE */
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
	filename = DeeSystem_MakeNormalAndAbsolute(filename);
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
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */


DECL_END
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
#endif /* !CONFIG_NO_DEC */

#endif /* !GUARD_DEEMON_EXECUTE_DEC_C */
