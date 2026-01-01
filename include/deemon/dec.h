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
#ifndef GUARD_DEEMON_DEC_H
#define GUARD_DEEMON_DEC_H 1

#include "api.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintX_t */

#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
#include "serial.h"
#include <hybrid/typecore.h>

#include "types.h"

#ifdef DEE_SOURCE
#include <hybrid/host.h>

#include "heap.h"
#include "mapfile.h"
#endif /* DEE_SOURCE */


/*
 * MMAP-Dec files allow for fast object encode/decode:
 *
 * - General idea is still that only specific object types can be dec-encoded
 * - However:
 *   - Give a file mapping of a ".dec" file, that mapping already contains
 *     fully functional objects (with reference counters, type headers, and
 *     everything already allocated)
 *   - While the associated module is still loaded, a mapped ".dec" file always
 *     holds 1 reference to every object it contains.
 *   - When the module is unloaded, decref all objects within the file mapping
 *   - Objects that can be dec-encoded need a special free-function that uses
 *     a custom heap that is able to function correctly, even if the pointer
 *     is part of a file mapping
 *     - If this heap needs extra headers for "allocated" pointers, then those
 *       headers are also present within the file mapping
 *
 *
 * Example: "Tuple { "foo", 42 }"
 * >> ...
 * >>              HEAP_HEADER,           // DeeTupleObject
 * >>my_tuple:     1,                     // ob_refcnt
 * >>              DREL(&DeeType_Type),   // ob_type    (relocated)
 * >>              2,                     // t_size
 * >>              IREL(&tuple_elem_1),   // t_elem[0]  (relocated)
 * >>              IREL(&tuple_elem_2),   // t_elem[1]  (relocated)
 * >>
 * >>              HEAP_HEADER,           // DeeStringObject
 * >>tuple_elem_1: 2,                     // ob_refcnt  (2: +1: my_tuple, +1: file mapping)
 * >>              DREL(&DeeString_Type), // ob_type    (relocated)
 * >>              NULL,                  // s_data
 * >>              1234,                  // s_hash
 * >>              3,                     // s_len
 * >>              "foo\0",               // s_str
 * >>
 * >>              HEAP_HEADER,           // DeeIntObject
 * >>tuple_elem_2: 2,                     // ob_refcnt  (2: +1: my_tuple, +1: file mapping)
 * >>              DREL(&DeeInt_Type),    // ob_type    (relocated)
 * >>              1,                     // ob_size
 * >>              42,                    // ob_digit
 *
 * After the file was mapped into memory:
 * - Parse the header, which contains (at least):
 *   - An exact identification for the deemon build used to create the file
 *   - The address of the root "DeeModule_Type" object of the associated module
 *   - Offsets to D/I-relocation tables
 *   - Offset to a table of all objects within the mapping (for decref on module unload)
 *   - Offset to a table of all HEAP_HEADER within the mapping (so the custom heap knows
 *     it can only free the file mapping once **all** of these have been freed).
 *     - This table should also come in a format where the custom heap can directly
 *       inherit it, without the table needing to be re-parsed/re-interpreted in some
 *       way, shape, or form.
 *     - For this purpose, the custom heap needs a O(1) function "gift_allocations":
 *       >> // @param allocs:    "table of all HEAP_HEADER within the mapping"
 *       >> //                   After all of these are free()'d, `free_fmap(fmap_base, fmap_size)' is called
 *       >> // @param fmap_base: Base address of file mapping
 *       >> // @param fmap_size: Size of file mapping
 *       >> // @param free_fmap: Nested free function to invoke once everything from "allocs" was freed
 *       >> void gift_allocations(struct alloc_list *allocs, void *fmap_base, size_t fmap_size,
 *       >>                       void (*free_fmap)(void *fmap_base, size_t fmap_size));
 *     - Note that "struct alloc_list" can either be:
 *       - An in-place vector with offsets to every "HEAP_HEADER"
 *       - The head of an intrusive linked list with elements embedded within "HEAP_HEADER"
 *       (It'll probably be the later, though...)
 *     - Using this, it would even be possible to implement "DeeModule_OfPointer"
 *       for user-defined modules, just like it's already implemented for dex modules!
 * - Verify that the file was created by the hosting build of deemon
 * - Execute relocations
 *   - DREL: *(uintptr_t *)LOC += (uintptr_t)&DeeModule_Type; // Or some other global symbol in the deemon code
 *   - IREL: *(uintptr_t *)LOC += (uintptr_t)FMAP_ADDR;       // Start address of file mapping
 * - return the now-fully-valid root "DeeModule_Type" object
 *
 *
 * Advantages:
 * - **any** type from the deemon core can be stored in a .dec file
 * - Loading of .dec file becomes much faster, because no additional heap allocations are necessary
 *
 * Disadvantages:
 * - Objects need a custom heap that can work with file mappings as though they were heap memory
 * - No validation during module loading means HARD CRASHES (i.e. SIGSEGV) if file was corrupt
 *   - Because of this, .dec files should at least have a simple checksum that is validated during load
 * - .dec files become **much** larger
 * - Requires its own, custom heap
 *
 */

DECL_BEGIN


#ifdef DEE_SOURCE
#define Dee_module_object    module_object
#define Dee_string_object    string_object
#define Dee_compiler_options compiler_options
#endif /* DEE_SOURCE */
struct Dee_module_object;
struct Dee_string_object;
struct Dee_compiler_options;
struct Dee_dec_writer;

typedef uint32_t Dee_dec_addr32_t; /* Offset from start of `Dec_Ehdr' to some other structure. */
typedef int32_t Dee_dec_off32_t;

#ifdef DEE_SOURCE
/* The max size of a DEC file.
 * If a file is larger than this, the loader is allowed to stop its attempts
 * at parsing it and simply act as though it wasn't a DEC file to begin with. */
#define DFILE_LIMIT  0xfffffff

#define DI_MAG0   0    /* File identification byte 0 index */
#define DECMAG0   0x7f /* Magic number byte 0 */
#define DI_MAG1   1    /* File identification byte 1 index */
#define DECMAG1   'D'  /* Magic number byte 1 */
#define DI_MAG2   2    /* File identification byte 2 index */
#define DECMAG2   'E'  /* Magic number byte 2 */
#define DI_MAG3   3    /* File identification byte 3 index */
#define DECMAG3   'C'  /* Magic number byte 3 */
#define DI_NIDENT 4    /* Number of identification bytes */


#define DVERSION_CUR  1 /* The currently active version of the DEC file format. */


#define Dee_DEC_MACH_UNKNOWN 0
#define Dee_DEC_MACH_I386    1
#define Dee_DEC_MACH_X86_64  2
#define Dee_DEC_MACH_ARM     3
#define Dee_DEC_MACH_ARM64   4

#ifndef Dee_DEC_MACH
#if defined(__x86_64__)
#define Dee_DEC_MACH Dee_DEC_MACH_X86_64
#elif defined(__i386__)
#define Dee_DEC_MACH Dee_DEC_MACH_I386
#elif defined(__arm__) && __SIZEOF_POINTER__ == 8
#define Dee_DEC_MACH Dee_DEC_MACH_ARM64
#elif defined(__arm__)
#define Dee_DEC_MACH Dee_DEC_MACH_ARM
#else /* ... */
#define Dee_DEC_MACH Dee_DEC_MACH_UNKNOWN
#endif /* !... */
#endif /* !Dee_DEC_MACH */

#define Dee_DEC_TYPE_RELOC 0 /* File contains serializable relocations (that are free'd once `DeeDec_Track()' is called) */
#define Dee_DEC_TYPE_IMAGE 1 /* File contains non-serializable relocations (that are free'd once `DeeDec_Track()' is called) */

typedef struct dec_rel Dec_Rel;
typedef struct dec_rrel Dec_RRel;
typedef struct dec_rrela Dec_RRela;


#define DeeDec_Ehdr_OFFSETOF__e_ident                                   0
#define DeeDec_Ehdr_OFFSETOF__e_mach                                    4
#define DeeDec_Ehdr_OFFSETOF__e_type                                    5
#define DeeDec_Ehdr_OFFSETOF__e_version                                 6
#define DeeDec_Ehdr_OFFSETOF__e_offsetof_heap                           8
#define DeeDec_Ehdr_OFFSETOF__e_sizeof_pointer                          10
#define DeeDec_Ehdr_OFFSETOF__e_offsetof_eof                            12
#define DeeDec_Ehdr_OFFSETOF__e_offsetof_gchead                         16
#define DeeDec_Ehdr_OFFSETOF__e_offsetof_gctail                         20
#define DeeDec_Ehdr_OFFSETOF__e_typedata                                24
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_deemon_timestamp 24
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_deemon_build_id  32
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_deemon_host_id   48
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_srel    64
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_drel    68
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_drrel   72
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_drrela  76
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_deps    80
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_files   84
#define DeeDec_Ehdr_OFFSETOF__e_mapping                                 88
#define DeeDec_Ehdr_OFFSETOF__e_heap \
	(88 + Dee_SIZEOF_DeeMapFile + ((Dee_HEAPCHUNK_ALIGN - ((88 + Dee_SIZEOF_DeeMapFile) % Dee_HEAPCHUNK_ALIGN)) % Dee_HEAPCHUNK_ALIGN))
#if DeeDec_Ehdr_OFFSETOF__e_heap == 128
#undef DeeDec_Ehdr_OFFSETOF__e_heap
#define DeeDec_Ehdr_OFFSETOF__e_heap 128
#elif DeeDec_Ehdr_OFFSETOF__e_heap == 120
#undef DeeDec_Ehdr_OFFSETOF__e_heap
#define DeeDec_Ehdr_OFFSETOF__e_heap 120
#elif DeeDec_Ehdr_OFFSETOF__e_heap == 112
#undef DeeDec_Ehdr_OFFSETOF__e_heap
#define DeeDec_Ehdr_OFFSETOF__e_heap 112
#elif DeeDec_Ehdr_OFFSETOF__e_heap == 104
#undef DeeDec_Ehdr_OFFSETOF__e_heap
#define DeeDec_Ehdr_OFFSETOF__e_heap 104
#elif DeeDec_Ehdr_OFFSETOF__e_heap == 96
#undef DeeDec_Ehdr_OFFSETOF__e_heap
#define DeeDec_Ehdr_OFFSETOF__e_heap 96
#endif /* DeeDec_Ehdr_OFFSETOF__e_heap == ... */

/* Main header for `.dec` files. For more details, see file ./dec.md */
typedef struct {
	uint8_t               e_ident[DI_NIDENT]; /* [AT(0-3)] Identification bytes. (See `DI_*') */
	uint8_t               e_mach;             /* [AT(4-4)] Machine identification (`Dee_DEC_MACH') */
	uint8_t               e_type;             /* [AT(5-5)] EHDR type (one of `Dee_DEC_TYPE_*') */
	uint16_t              e_version;          /* [AT(6-7)] DEC version number (One of `DVERSION_*') -- NOTE: __MUST__ remain at this specific offset for backwards compatibility! */
	uint16_t              e_offsetof_heap;    /* Offset from start of this struct to `e_heap' (== `DeeDec_Ehdr_OFFSETOF__e_heap') */
	uint8_t               e_sizeof_pointer;   /* Size of pointer (and thus of: relocation targets, and `Dee_refcnt_t') */
	uint8_t              _e_unused_zero;      /* Unused; must be set to `0' */
	Dee_dec_addr32_t      e_offsetof_eof;     /* [1..1] Offset to EOF of file mapping (should also equal the dec file's size) */
	Dee_dec_addr32_t      e_offsetof_gchead;  /* [0..1] Offset to first `struct gc_head_link' (tracking for these objects must begin after relocations were done) */
	Dee_dec_addr32_t      e_offsetof_gctail;  /* [0..1] Offset to last `struct gc_head_link' (links between these objects were already established via `e_offsetof_srel') */
	union {

		struct {
			/* The following 3 fields are here as validation that the
			 * dec file wasn't generated by a different deemon version. */
			uint64_t         er_deemon_timestamp;   /* Microseconds since `01-01-1970', when deemon was compiled. */
			uint64_t         er_deemon_build_id[2]; /* Deemon build ID (a random UUID generated when compiling deemon) */
			uint64_t         er_deemon_host_id[2];  /* Deemon host ID (a random UUID generated when installing deemon / starting deemon for the first time) */
			Dee_dec_addr32_t er_offsetof_srel;      /* [1..1] Offset to array of `Dec_Rel[]' (terminated by a r_addr==0-entry) of relocations with "REL_BASE=(uintptr_t)ehdr" */
			Dee_dec_addr32_t er_offsetof_drel;      /* [1..1] Offset to array of `Dec_Rel[]' (terminated by a r_addr==0-entry) of relocations with "REL_BASE=(uintptr_t)&DeeModule_Deemon" */
			Dee_dec_addr32_t er_offsetof_drrel;     /* [1..1] Offset to array of `Dec_RRel[]' (terminated by a r_addr==0-entry) of relocations with "REL_BASE=(uintptr_t)&DeeModule_Deemon" */
			Dee_dec_addr32_t er_offsetof_drrela;    /* [1..1] Offset to array of `Dec_RRela[]' (terminated by a r_addr==0-entry) of relocations with "REL_BASE=(uintptr_t)&DeeModule_Deemon" */
			Dee_dec_addr32_t er_offsetof_deps;      /* [1..1] Offset to array of `Dec_Dhdr[]' (terminated by a d_modspec.d_mod==NULL-entry) of other dependent deemon modules */
			Dee_dec_addr32_t er_offsetof_files;     /* [0..1] Offset to array of `Dec_Dstr[]' (terminated by a ds_length==0-entry, each aligned to Dee_ALIGNOF_DEC_DSTR) of extra filenames relative to the directory containing the .dec-file. If any of these files is newer than `e_build_timestamp', don't load dec file */
		}                 td_reloc;                 /* [valid_if(e_type == Dee_DEC_TYPE_RELOC)] */

		struct {
			Dec_RRel              *ei_drrel_v;  /* [0..ei_drrel_c][owned] List of incref-relocations against deemon-core objects */
			size_t                 ei_drrel_c;  /* # of entries in `ei_drrel_v' */
			Dec_RRela             *ei_drrela_v; /* [0..ei_drrela_c][owned] List of incref-offset-relocations against deemon-core objects */
			size_t                 ei_drrela_c; /* # of entries in `ei_drrela_v' */
			struct Dee_dec_depmod *ei_deps_v;   /* [0..ei_deps_c][owned] Vector of dependent modules */
			size_t                 ei_deps_c;   /* # of entries in `ei_deps_v' */
		}                 td_image;             /* [valid_if(e_type == Dee_DEC_TYPE_IMAGE)] */

	}                     e_typedata;         /* Data dependent on `e_type' */
	struct DeeMapFile     e_mapping;          /* Uninitialized/unused in file mappings; when mapped into memory, populated with the dec file's own file map descriptor. */
#if ((DeeDec_Ehdr_OFFSETOF__e_mapping + Dee_SIZEOF_DeeMapFile) % Dee_HEAPCHUNK_ALIGN) != 0
	__BYTE_TYPE__ _e_heap_pad[Dee_HEAPCHUNK_ALIGN - ((DeeDec_Ehdr_OFFSETOF__e_mapping + Dee_SIZEOF_DeeMapFile) % Dee_HEAPCHUNK_ALIGN)];
#endif /* (Dee_SIZEOF_DeeMapFile % Dee_HEAPCHUNK_ALIGN) != 0 */
	struct Dee_heapregion e_heap;             /* Heap region descriptor for objects embedded within this dec file. The first chunk of
	                                           * this heap is assumed to point at the `DeeModuleObject' describing the dec file itself. */
} Dec_Ehdr;

struct dec_rel {
	/* Non-reference-counted relocation:
	 * >> byte_t **p_ptr = (byte_t **)((byte_t *)EHDR + r_addr);
	 * >> *p_ptr += REL_BASE; */
	Dee_dec_addr32_t r_addr; /* Offset from start of "Dec_Ehdr" to relocated pointer */
};

struct dec_rrel {
	/* Reference-counted relocation:
	 * >> byte_t **p_ptr = (byte_t **)((byte_t *)EHDR + r_addr);
	 * >> DeeObject *obj = (DeeObject *)(*p_ptr += REL_BASE);
	 * >> ASSERT_OBJECT(obj);
	 * >> Dee_Incref(obj); */
	Dee_dec_addr32_t r_addr; /* Offset from start of "Dec_Ehdr" to relocated pointer */
};

struct dec_rrela {
	/* Reference-counted+addend relocation:
	 * >> byte_t **p_ptr = (byte_t **)((byte_t *)EHDR + r_addr);
	 * >> byte_t *pointer = (*p_ptr += REL_BASE);
	 * >> DeeObject *obj = (DeeObject *)(pointer + r_offs);
	 * >> ASSERT_OBJECT(obj);
	 * >> Dee_Incref(obj); */
	Dee_dec_addr32_t r_addr; /* Offset from start of "Dec_Ehdr" to relocated pointer */
	Dee_dec_off32_t  r_offs; /* Offset added to resulting pointer to get to base of object that must be incref'd */
};

typedef struct {
	union {
		struct {
			Dee_dec_addr32_t d_offsetof_modname; /* [1..1] Offset to `(Dec_Dstr *)' to pass to `DeeModule_OpenRelative()' in order to load this dependency. The dependency must not be newer than this file! */
			Dee_dec_addr32_t d_offsetof_rel;     /* [1..1] Offset to array of `Dec_Rel[]' (terminated by a r_addr==0-entry) of relocations with "REL_BASE=(uintptr_t)d_mod" */
		}                              d_file;   /* Layout of in-file data */
		DREF struct Dee_module_object *d_mod;    /* Used internally during relocation */
	}                d_modspec;
	Dee_dec_addr32_t d_offsetof_rrel;  /* [1..1] Offset to array of `Dec_RRel[]' (terminated by a r_addr==0-entry) of relocations with "REL_BASE=(uintptr_t)d_mod" */
	Dee_dec_addr32_t d_offsetof_rrela; /* [1..1] Offset to array of `Dec_RRela[]' (terminated by a r_addr==0-entry) of relocations with "REL_BASE=(uintptr_t)d_mod" */
} Dec_Dhdr;

#define Dee_ALIGNOF_DEC_DSTR 4
typedef struct {
	Dee_dec_addr32_t              ds_length;  /* Length of `ds_length' (in bytes; excluding trailing NUL) */
	COMPILER_FLEXIBLE_ARRAY(char, ds_string); /* [ds_length] UTF-8 string (NUL-terminated) */
} Dec_Dstr;

typedef Dec_Ehdr DeeDec_Ehdr;

/* Finalize the dec file's file mapping (which will cause the mapping to be unloaded) */
#define DeeDec_Ehdr_Destroy(self) \
	DeeMapFile_Fini(&(self)->e_mapping)

/* Return a pointer to the `DeeModuleObject' that follow `Dec_Ehdr' */
#define DeeDec_Ehdr_GetModule(self) \
	((DeeModuleObject *)&((struct Dee_gc_head *)(&(self)->e_heap.hr_first + 1))->gc_object)
#define DeeDec_Ehdr_FromModule(self)                                      \
	((Dec_Ehdr *)((uintptr_t)Dee_REQUIRES_TYPE(DeeModuleObject *, self) - \
	              (COMPILER_OFFSETAFTER(Dec_Ehdr, e_heap.hr_first) +      \
	               __builtin_offsetof(struct Dee_gc_head, gc_object))))

#else /* DEE_SOURCE */
typedef char DeeDec_Ehdr;
#endif /* !DEE_SOURCE */



struct Dee_dec_reltab {
	Dec_Rel *drlt_relv; /* [0..drlt_relc][owned] Vector of relocations */
	size_t   drlt_relc; /* # of relocations already written */
	size_t   drlt_rela; /* Allocated # of entries in `drlt_relv' */
};
#define Dee_dec_reltab_init(self) \
	(void)((self)->drlt_relv = NULL, (self)->drlt_relc = (self)->drlt_rela = 0)
#define Dee_dec_reltab_fini(self) \
	Dee_Free((self)->drlt_relv)

struct Dee_dec_rreltab {
	Dec_RRel *drrt_relv; /* [0..drrt_relc][owned] Vector of relocations */
	size_t    drrt_relc; /* # of relocations already written */
	size_t    drrt_rela; /* Allocated # of entries in `drrt_relv' */
};
#define Dee_dec_rreltab_init(self) \
	(void)((self)->drrt_relv = NULL, (self)->drrt_relc = (self)->drrt_rela = 0)
#define Dee_dec_rreltab_fini(self) \
	Dee_Free((self)->drrt_relv)

struct Dee_dec_rrelatab {
	Dec_RRela *drat_relv; /* [0..drlt_relc][owned] Vector of relocations */
	size_t     drat_relc; /* # of relocations already written */
	size_t     drat_rela; /* Allocated # of entries in `drat_relv' */
};
#define Dee_dec_rrelatab_init(self) \
	(void)((self)->drat_relv = NULL, (self)->drat_relc = (self)->drat_rela = 0)
#define Dee_dec_rrelatab_fini(self) \
	Dee_Free((self)->drat_relv)

struct Dee_dec_depmod {
	DREF struct Dee_module_object *ddm_mod;    /* [1..1] The dependent module */
	DREF struct Dee_string_object *ddm_impstr; /* [0..1] Import string (lazily generated during `DeeDecWriter_PackEhdr()') */
	struct Dee_dec_reltab          ddm_rel;    /* Relocations against `ddm_mod' */
	struct Dee_dec_rreltab         ddm_rrel;   /* Incref-relocations against `ddm_mod' */
	struct Dee_dec_rrelatab        ddm_rrela;  /* Incref-addend-relocations against `ddm_mod' */
};
#define Dee_dec_depmod_init(self, /*inherit(always)*/ mod) \
	((self)->ddm_mod = (mod), (self)->ddm_impstr = NULL,   \
	 Dee_dec_reltab_init(&(self)->ddm_rel),                \
	 Dee_dec_rreltab_init(&(self)->ddm_rrel),              \
	 Dee_dec_rrelatab_init(&(self)->ddm_rrela))
#define Dee_dec_depmod_fini(self)             \
	(Dee_Decref_unlikely((self)->ddm_mod),    \
	 Dee_XDecref_likely((self)->ddm_impstr),  \
	 Dee_dec_reltab_fini(&(self)->ddm_rel),   \
	 Dee_dec_rreltab_fini(&(self)->ddm_rrel), \
	 Dee_dec_rrelatab_fini(&(self)->ddm_rrela))

struct Dee_dec_deptab {
	struct Dee_dec_depmod *ddpt_depv; /* [0..ddpt_depc][owned] Vector of dependent modules (sorted by `ddm_mod ASC') */
	size_t                 ddpt_depc; /* # of dependent modules */
	size_t                 ddpt_depa; /* Allocated # of entries in `ddpt_depv' */
};
#define Dee_dec_deptab_init(self) \
	(void)((self)->ddpt_depv = NULL, (self)->ddpt_depc = (self)->ddpt_depa = 0)

struct Dee_dec_ptrtab_entry {
	void            *dpte_ptr; /* [1..1] Source address that was already encoded */
	Dee_dec_addr32_t dote_off; /* Offset from `dw_base' to where "dpte_ptr" was written */
	Dee_dec_addr32_t dote_siz; /* # of bytes associated with "dpte_ptr" */
};
#define Dee_dec_ptrtab_entry_getminaddr(self) ((__BYTE_TYPE__ *)(self)->dpte_ptr)
#define Dee_dec_ptrtab_entry_getmaxaddr(self) ((__BYTE_TYPE__ *)(self)->dpte_ptr + (self)->dote_siz - 1)

struct Dee_dec_ptrtab {
	struct Dee_dec_ptrtab_entry *dpt_ptrv; /* [0..dpt_ptrc][SORT(dpte_ptr ASC)][owned] List of already-encoded objects */
	size_t                       dpt_ptrc; /* Amount of non-NULL key-item pairs. */
	size_t                       dpt_ptra; /* Amount of non-NULL key-item pairs. */
};
#define Dee_dec_ptrtab_init(self) \
	(void)((self)->dpt_ptrv = NULL, (self)->dpt_ptrc = (self)->dpt_ptra = 0)
#define Dee_dec_ptrtab_fini(self) \
	Dee_Free((self)->dpt_ptrv)

struct Dee_dec_fdeptab {
	__BYTE_TYPE__ *dfdt_depv; /* [0..dfdt_depc][owned] Flat array of `Dec_Dstr' (each aligned to `__ALIGNOF_SIZE_T__') */
	size_t         dfdt_depc; /* # of bytes used in "dfdt_depv" */
	size_t         dfdt_depa; /* Allocated # of bytes in `dfdt_depv' */
};

#define Dee_dec_fdeptab_init(self) \
	(void)((self)->dfdt_depv = NULL, (self)->dfdt_depc = (self)->dfdt_depa = 0)
#define Dee_dec_fdeptab_fini(self) Dee_Free((self)->dfdt_depv)


typedef struct Dee_dec_writer {
	Dee_SERIAL_HEAD
#ifdef DEE_SOURCE
	union {
		__BYTE_TYPE__      *dw_base;   /* [1..1][owned] Base address of memory block of dec file being built */
		Dec_Ehdr           *dw_ehdr;   /* [1..1][owned] Dec executable header, and subsequent object heap.
		                                * Until the dec file is finalized, everything in here, except for
		                                * `e_heap' is undefined */
	};
#else /* DEE_SOURCE */
	__BYTE_TYPE__          *dw_base;   /* [1..1][owned] Base address of memory block of dec file being built */
#endif /* !DEE_SOURCE */
	size_t                  dw_alloc;  /* Allocated buffer size for `dw_base' */
	size_t                  dw_used;   /* [<= dw_alloc] Used buffer size for `dw_base' */
	size_t                  dw_hlast;  /* Chunk size during the previous call to `DeeDecWriter_Malloc()' */
	struct Dee_dec_reltab   dw_srel;   /* Table of self-relocations */
	struct Dee_dec_reltab   dw_drel;   /* Table of relocations against deemon-core objects */
	struct Dee_dec_rreltab  dw_drrel;  /* Table of incref-relocations against deemon-core objects */
	struct Dee_dec_rrelatab dw_drrela; /* Table of incref-relocations against deemon-core objects */
	size_t                  dw_drrela_miss; /* # of leading relocations in `dw_drrela' that already happened (when non-zero, 'DeeDecWriter_F_NRELOC' will also be set) */
	struct Dee_dec_deptab   dw_deps;   /* Table of dependent modules */
	struct Dee_dec_fdeptab  dw_fdeps;  /* Table of dependent files */
	Dee_dec_addr32_t        dw_gchead; /* [0..1] Offset to first `struct gc_head_link' (tracking for these objects must begin after relocations were done) */
	Dee_dec_addr32_t        dw_gctail; /* [0..1] Offset to last `struct gc_head_link' (links between these objects were already established via `dw_srel') */
	struct Dee_dec_ptrtab   dw_known;  /* Table of known, already-encoded pointers */
	unsigned int            dw_flags;  /* Dec writer flags (set of `DeeDecWriter_F_*') */
} DeeDecWriter;

#define DeeDecWriter_F_NORMAL 0x0000 /* Normal flags */
#define DeeDecWriter_F_FRELOC 0x0002 /* Force the produced dec file to be relocatable: `DeeDecWriter_F_NRELOC' will not be set and `DeeRT_ErrCannotSerialize' will be thrown */
#define DeeDecWriter_F_NRELOC 0x0080 /* Encountered a object/pointer that cannot be serialized.
                                      * Dec generation continues, but resulting image cannot be
                                      * written to a file (iow: is "Dee_DEC_TYPE_IMAGE"). This
                                      * flag may also speed up object serialization, since it
                                      * means that relocations against other modules don't have
                                      * to be kept track of. */

/* Same as `DeeSerial_Addr2Mem()' */
#define DeeDecWriter_Addr2Mem(self, addr, T) ((T *)((self)->dw_base + (addr)))

/* Initialize/finalize a dec writer. Note that unlike usual, `DeeDecWriter_Init()'
 * already needs to allocate a small amount of heap memory, meaning it can actually
 * fail due to OOM and do so by returning `-1'
 * @param: flags: Set of `DeeDecWriter_F_*'
 * @return: 0 : Success
 * @return: -1: An error was thrown */
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeDecWriter_Init(DeeDecWriter *__restrict self, unsigned int flags);
#else /* __INTELLISENSE__ */
#define DeeDecWriter_Init(self, flags) \
	((self)->dw_flags = (flags), _DeeDecWriter_Init(self))
#endif /* !__INTELLISENSE__ */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
_DeeDecWriter_Init(DeeDecWriter *__restrict self);
DFUNDEF NONNULL((1)) void DCALL DeeDecWriter_Fini(DeeDecWriter *__restrict self);


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
DFUNDEF WUNUSED NONNULL((1)) /*inherit*/ DeeDec_Ehdr *DCALL
DeeDecWriter_PackEhdr(DeeDecWriter *__restrict self,
                      /*utf-8*/ char const *context_absname,
                      size_t context_absname_size,
                      unsigned int flags);

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
DFUNDEF WUNUSED NONNULL((1, 2)) DREF /*untracked*/ struct Dee_module_object *DCALL
DeeDecWriter_PackModule(DeeDecWriter *__restrict self,
                        /*inherit(on_success)*/ DeeDec_Ehdr *__restrict ehdr);


/* Add an additional file dependency to `self'. The given `filename' must
 * be relative to the directory that the `.dec' file will eventually reside
 * within. By default, only the relevant `.dee' file will be a dependency
 * of the produced `.dec' file.
 * @return: 0 : Success
 * @return: -1: Error */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeDecWriter_AddFileDep(DeeDecWriter *__restrict self,
                        char const *filename,
                        size_t filename_len);

/* Execute relocations on `self' and return a pointer to the
 * first object of the dec file's heap (which is always the
 * `DeeModuleObject' describing the dec file itself).
 * NOTE: Can only be used when `self->e_type == Dee_DEC_TYPE_RELOC'
 *
 * On success, `self' is inherited by `return', such that rather than
 * calling `DeeDec_Ehdr_Destroy(self)', you must `(return)'
 *
 * @param: flags: Set of `0 | DeeModule_IMPORT_F_CTXDIR':
 *                - DeeModule_IMPORT_F_CTXDIR: When set, "context_absname...+=context_absname_size" is
 *                                             the directory containing the .dec file mapped by "self",
 *                                             rather than the .dec file itself.
 * @return: * :   The module object described by `self'
 * @return: NULL: An error was thrown
 * @return: ITER_DONE: The DEC file was out of date or had been corrupted */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF /*untracked*/ struct Dee_module_object *DCALL
DeeDec_Relocate(/*inherit(on_success)*/ DeeDec_Ehdr *__restrict self,
                /*utf-8*/ char const *context_absname, size_t context_absname_size,
                unsigned int flags, struct Dee_compiler_options *options,
                uint64_t dee_file_last_modified);

/* Start GC-tracking (and allowing reverse address lookup of) "self", as returned by:
 * - DeeDec_OpenFile()
 * - DeeDec_Relocate()
 * - DeeDecWriter_PackModule() */
DFUNDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF /*tracked*/ struct Dee_module_object *DCALL
DeeDec_Track(DREF /*untracked*/ struct Dee_module_object *__restrict self);

/* Destroy a module and all contained objects prior to `DeeDec_Track()' having been called. */
DFUNDEF NONNULL((1)) void DCALL
DeeDec_DestroyUntracked(DREF /*untracked*/ struct Dee_module_object *__restrict self);

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
DFUNDEF WUNUSED NONNULL((1, 2)) DREF /*untracked*/ struct Dee_module_object *DCALL
DeeDec_OpenFile(/*inherit(on_success)*/ struct DeeMapFile *__restrict fmap,
                /*utf-8*/ char const *context_absname, size_t context_absname_size,
                unsigned int flags, struct Dee_compiler_options *options,
                uint64_t dee_file_last_modified);



DECL_END
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */

#include <hybrid/byteorder.h>
/**/

#ifdef CONFIG_BUILDING_DEEMON
#ifndef CONFIG_NO_DEC
#include "types.h"
#endif /* !CONFIG_NO_DEC */
#endif /* CONFIG_BUILDING_DEEMON */
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintX_t */

DECL_BEGIN

#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(push, 1)
#endif /* __COMPILER_HAVE_PRAGMA_PACK */


/* The max size of a DEC file.
 * If a file is larger than this, the loader is allowed to stop its attempts
 * at parsing it and simply act as though it wasn't a DEC file to begin with. */
#define DFILE_LIMIT  0xffffff


#define DI_MAG0   0    /* File identification byte 0 index */
#define DECMAG0   0x7f /* Magic number byte 0 */
#define DI_MAG1   1    /* File identification byte 1 index */
#define DECMAG1   'D'  /* Magic number byte 1 */
#define DI_MAG2   2    /* File identification byte 2 index */
#define DECMAG2   'E'  /* Magic number byte 2 */
#define DI_MAG3   3    /* File identification byte 3 index */
#define DECMAG3   'C'  /* Magic number byte 3 */
#define DI_NIDENT 4    /* Number of identification bytes */



#define DVERSION_CUR  0 /* The currently active version of the DEC file format. */

typedef struct ATTR_PACKED {
	uint8_t  e_ident[DI_NIDENT]; /* Identification bytes. (See `DI_*') */
	uint8_t  e_builtinset;       /* Set of builtin object (One of `DBUILTINS_*') */
	uint8_t  e_size;             /* Absolute size of the header (`Dec_Ehdr').
	                              * NOTE: When larger than expected, don't assume errors, but ignore trailing data! */
	uint16_t e_version;          /* DEC version number (One of `DVERSION_*'). */
	uint32_t e_impoff;           /* Absolute file-offset of the import string map (`Dec_Strmap').
	                              * NOTE: When ZERO(0), then there is no import table.
	                              * NOTE: Strings are encoded in relative-import format
	                              *      (as accepted by `DeeModule_OpenRelative') */
	uint32_t e_depoff;           /* When checking if a DEC file has become outdated, the obvious
	                              * candidate that must be checked is the `.*.dec' replaced with `*.dee'.
	                              * However because deemon makes use of a C-compatible preprocessor,
	                              * user-code can create additional dependencies that must also be
	                              * able to trigger dismissal and re-generation of `.*.dec' files.
	                              * This field describes an absolute file-offset to a `Dec_Strmap'
	                              * structure containing the relative pathnames (using `/' for slashes)
	                              * of all additional dependencies who's timestamps must be checked
	                              * before it can be confirmed that no dependency of this DEC file
	                              * has been modified. */
	uint32_t e_globoff;          /* Absolute file-offset of the global variable table (`Dec_Glbmap').
	                              * NOTE: When ZERO(0), then there are no globals. */
	uint32_t e_rootoff;          /* Absolute file-offset of the root code object (`Dec_Code'). */
	uint32_t e_stroff;           /* Absolute file-offset of the UTF-8 encoded string table. */
	uint32_t e_strsiz;           /* The absolute length of the string table (in bytes). */
	uint32_t e_timestamp_lo;     /* In microseconds since `01-01-1970', the point in
	                              * time when this compiled source file was generated.
	                              * Since DEC files are meant to be used as a shallow
	                              * cache file that is automatically discarded if the
	                              * actual source file has since changed, this is the
	                              * time against which everything must be compared. */
	uint32_t e_timestamp_hi;     /* High 32-bits of the timestamp. */
} Dec_Ehdr;



typedef struct ATTR_PACKED {
	uint16_t i_len;    /* Number of string pointers.
	                    * NOTE: When (uint16_t)-1, a `uint32_t' follows with the actual size. */
	uint8_t  i_map[1]; /* Offsets into the string table (`e_stroff') to nul-terminated string.
	                    * NOTE: Individual pointers are decoded using `Dec_DecodePointer()'. */
} Dec_Strmap;

typedef struct ATTR_PACKED {
	uint16_t   s_flg;       /* Symbol flags (Set of `MODSYM_F*') */
	uint8_t    s_nam[1];    /* Name of the symbol. - offsets into the string table (`e_stroff').
	                         * NOTE: Individual pointers are decoded using `Dec_DecodePointer()'.
	                         * NOTE: If this value points to an empty string, then this
	                         *       symbol hasn't been given a name and neither `s_doclen',
	                         *       nor `s_doc' exist. */
	uint8_t    s_doclen[1]; /* Length of the documentation string (decodable using `Dec_DecodePointer()'). */
	uint8_t    s_doc[1];    /* Documentation string of the symbol. - offsets into the string table (`e_stroff').
	                         * NOTE: Individual pointers are decoded using `Dec_DecodePointer()'.
	                         * NOTE: Only exists when `s_doclen' evaluates to non-zero. */
} Dec_GlbSym;

typedef struct ATTR_PACKED {
	uint16_t   s_flg;       /* Symbol flags (Set of `MODSYM_F*') */
#undef s_addr
	uint16_t   s_addr;      /* Symbol address, or module import index, or getter symbol index. */
	uint16_t   s_addr2;     /* [exists_if(s_flg & MODSYM_FEXTERN)] External module symbol index. */
	uint8_t    s_nam[1];    /* Name of the symbol. - offsets into the string table (`e_stroff').
	                         * NOTE: Individual pointers are decoded using `Dec_DecodePointer()'.
	                         * NOTE: If this value points to an empty string, then this
	                         *       symbol hasn't been given a name and neither `s_doclen',
	                         *       nor `s_doc' exist. */
	uint8_t    s_doclen[1]; /* Length of the documentation string (decodable using `Dec_DecodePointer()'). */
	uint8_t    s_doc[1];    /* Documentation string of the symbol. - offsets into the string table (`e_stroff').
	                         * NOTE: Individual pointers are decoded using `Dec_DecodePointer()'.
	                         * NOTE: Only exists when `s_doclen' evaluates to non-zero. */
} Dec_GlbExt;

typedef struct ATTR_PACKED {
	uint16_t   g_cnt;    /* Number of global variables. */
	uint16_t   g_len;    /* Number of global symbols. */
	Dec_GlbSym g_map[1]; /* [g_cnt] Vector of global variable descriptors. */
	Dec_GlbExt g_ext[1]; /* [g_len-g_cnt] Vector of extended global variable descriptors. */
} Dec_Glbmap;


typedef struct ATTR_PACKED {
	uint8_t    cs_type;    /* Type code (One of `DTYPE_*') */
	uint8_t    cs_data[1]; /* Type data (depends on `DTYPE_*'; may not actually exist)
	                        * NOTE: If or not `DTYPE_NULL' is allowed depends on the usage context. */
} Dec_Object;


/* Code controller data structures. */
typedef struct ATTR_PACKED {
	uint16_t   os_len;    /* Number of objects. */
	Dec_Object os_vec[1]; /* Vector of objects.
	                       * NOTE: Whether or not `DTYPE_NULL' is allowed
	                       *       depends on the usage context. */
} Dec_Objects;

typedef struct ATTR_PACKED {
	uint16_t   ce_flags;  /* Set of `EXCEPTION_HANDLER_F*' */
	uint32_t   ce_begin;  /* [<= ce_end] Exception handler protection start address. */
	uint32_t   ce_end;    /* [>= ce_begin] Exception handler protection end address. */
	uint32_t   ce_addr;   /* [< ce_begin && >= ce_end] Exception handler entry point. */
	uint16_t   ce_stack;  /* Stack depth that must be ensured when this handler is executed. */
	Dec_Object ce_mask;   /* Exception handler mask. (NOTE: `DTYPE_NULL' is allowed) */
} Dec_CodeExcept;

typedef struct ATTR_PACKED {
	uint16_t       ces_len;    /* Amount of exception descriptors. */
	Dec_CodeExcept ces_vec[1]; /* [ces_len] Vector of exception descriptors. */
} Dec_CodeExceptions;

typedef struct ATTR_PACKED {
	uint16_t   ce_flags;  /* Set of `EXCEPTION_HANDLER_F*' */
	uint16_t   ce_begin;  /* [<= ce_end] Exception handler protection start address. */
	uint16_t   ce_end;    /* [>= ce_begin] Exception handler protection end address. */
	uint16_t   ce_addr;   /* [< ce_begin && >= ce_end] Exception handler entry point. */
	uint8_t    ce_stack;  /* Stack depth that must be ensured when this handler is executed. */
	Dec_Object ce_mask;   /* Exception handler mask. (NOTE: `DTYPE_NULL' is allowed) */
} Dec_8BitCodeExcept;

typedef struct ATTR_PACKED {
	uint8_t            ces_len;    /* Amount of exception descriptors. */
	Dec_8BitCodeExcept ces_vec[1]; /* [ces_len] Vector of exception descriptors. */
} Dec_8BitCodeExceptions;

typedef struct ATTR_PACKED {
	/* All uint8_t[1]-fields in this structure are tightly packed
	 * integers decodable using `READ_SLEB()' or `READ_ULEB()'
	 * The actual length and member-offsets within this
	 * structure are therefor only known at compile-time. */
	uint16_t     rs_flags;   /* Set of `DDI_REGS_F*' */
	uint8_t      rs_uip[1];  /* [DECODE(READ_ULEB)] Initial user instruction. */
	uint8_t      rs_usp[1];  /* [DECODE(READ_ULEB)] Initial stack alignment/depth. */
	uint8_t      rs_path[1]; /* [DECODE(READ_ULEB)] Initial path number. (NOTE: ZERO indicates no path and all other values are used as index-1 in the `cd_paths' vector of the associated `Dec_CodeDDI') */
	uint8_t      rs_file[1]; /* [DECODE(READ_ULEB)] Initial file number. */
	uint8_t      rs_name[1]; /* [DECODE(READ_ULEB)] Initial name offset (points into the string table). */
	uint8_t      rs_col[1];  /* [DECODE(READ_SLEB)] Initial column number within the active line. */
	uint8_t      rs_lno[1];  /* [DECODE(READ_SLEB)] Initial Line number (0-based). */
} Dec_DDIRegStart;

typedef struct ATTR_PACKED {
	uint16_t  dx_size;    /* Data size (when (uint16_t)-1, the a uint32_t containing the actual size follows immediately) */
	uint8_t   dx_data[1]; /* [dx_size] DDI extension data. */
} Dec_DDIExdat;

typedef struct ATTR_PACKED {
	uint32_t        cd_strings; /* Absolute pointer to a `Dec_Strmap' structure describing DDI string. */
	uint32_t        cd_ddixdat; /* Absolute pointer to a `Dec_DDIExdat' structure, or 0. */
	uint32_t        cd_ddiaddr; /* Absolute offset into the file to a block of `cd_ddisize' bytes of text describing DDI code (s.a.: `DDI_*'). */
	uint32_t        cd_ddisize; /* The total size (in bytes) of DDI text for translating instruction pointers to file+line, etc. */
	uint16_t        cd_ddiinit; /* Amount of leading DDI instruction bytes that are used for state initialization */
	Dec_DDIRegStart cd_regs;    /* The initial register state. */
} Dec_CodeDDI;

typedef struct ATTR_PACKED {
	uint16_t        cd_strings; /* Absolute pointer to a `Dec_Strmap' structure describing DDI string. */
	uint16_t        cd_ddixdat; /* Absolute pointer to a `Dec_DDIExdat' structure, or 0. */
	uint16_t        cd_ddiaddr; /* Absolute offset into the file to a block of `cd_ddisize' bytes of text describing DDI code (s.a.: `DDI_*'). */
	uint16_t        cd_ddisize; /* The total size (in bytes) of DDI text for translating instruction pointers to file+line, etc. */
	uint8_t         cd_ddiinit; /* Amount of leading DDI instruction bytes that are used for state initialization */
	Dec_DDIRegStart cd_regs;    /* The initial register state. */
} Dec_8BitCodeDDI;

typedef struct ATTR_PACKED {
	uint8_t     ck_len[1]; /* Length of the keyword (excluding any trailing \0 character) */
	uint8_t     ck_off[1]; /* [exists_if(ck_len != 0)] Offset into string table to where the keyword's name is written. */
} Dec_CodeKwd;

typedef struct ATTR_PACKED {
	Dec_CodeKwd ck_map[1]; /* Vector of code keywords.
	                        * NOTE: The length of this vector is `co_argc_max' */
} Dec_CodeKwds;

typedef struct ATTR_PACKED {
	uint8_t  ck_map[1]; /* Offsets into the string table (`e_stroff') to nul-terminated string.
	                     * NOTE: Individual pointers are decoded using `Dec_DecodePointer()'.
	                     * NOTE: The length of this vector is `co_argc_max' */
} Dec_8BitCodeKwds;

typedef struct ATTR_PACKED {
	uint16_t   co_flags;      /* Set of `CODE_F*' optionally or'd with `DEC_CODE_F8BIT'.
	                           * NOTE: When set, this data structure must be
	                           *       interpreted as an `Dec_8BitCode' object */
	uint16_t   co_localc;     /* Amount of local variables used by code. */
	uint16_t   co_refc;       /* Amount of reference variables used by this code. */
	uint16_t   co_staticc;    /* Number of static variables that appear after `co_refc' */
	uint16_t   co_argc_min;   /* Min amount of arguments required to execute this code. */
	uint16_t   co_stackmax;   /* The greatest allowed stack depth of the assembly associated with this code. */
	uint32_t   co_constoff;   /* Absolute file offset to a constant variable descriptor table (`Dec_Objects').
	                           * NOTE: `DTYPE_NULL' is not allowed. */
	uint32_t   co_exceptoff;  /* Absolute file offset to an exception handler descriptor table (`Dec_CodeExceptions'). */
	uint32_t   co_defaultoff; /* Absolute file offset to an argument-default descriptor table (`Dec_Objects').
	                           * NOTE: When ZERO(0), there are no default arguments, but if not,
	                           *       `co_argc_max' can be calculated from `co_argc_min+os_len'
	                           * NOTE: `DTYPE_NULL' is allowed. */
	uint32_t   co_ddioff;     /* Absolute file offset to optional code debug information (`Dec_CodeDDI').
	                           * NOTE: When ZERO(0), there is no DDI information. */
	uint32_t   co_kwdoff;     /* Absolute file offset to optional keyword argument information (`Dec_CodeKwds')
	                           * NOTE: When ZERO(0), there is no keyword information. */
	uint32_t   co_textsiz;    /* Absolute size of this code's text section (in bytes) */
	uint32_t   co_textoff;    /* Absolute file offset to the assembly text that will be executed by this code. */
} Dec_Code;

typedef struct ATTR_PACKED {
	uint16_t   co_flags;      /* Set of `CODE_F*' optionally or'd with `DEC_CODE_F8BIT'.
	                           * NOTE: This data structure must only (and always) be used
	                           *       when the `DEC_CODE_F8BIT' flag is set. */
	uint8_t    co_localc;     /* Amount of local variables used by code. */
	uint8_t    co_refc;       /* Amount of reference variables used by this code. */
	uint8_t    co_argc_min;   /* Min amount of arguments required to execute this code. */
	uint8_t    co_stackmax;   /* The greatest allowed stack depth of the assembly associated with this code. */
	uint16_t   co_constoff;  /* Absolute file offset to a static/constant variable descriptor table (`Dec_Objects'). */
	uint16_t   co_exceptoff;  /* Absolute file offset to an exception handler descriptor table (`Dec_8BitCodeExceptions'). */
	uint16_t   co_defaultoff; /* Absolute file offset to an argument-default descriptor table (`Dec_Objects').
	                           * NOTE: When ZERO(0), there are no default arguments, but if not,
	                           *      `co_argc_max' can be calculated from `co_argc_min+os_len' */
	uint16_t   co_ddioff;     /* Absolute file offset to optional code debug information (`Dec_8BitCodeDDI').
	                           * NOTE: When ZERO(0), there is no DDI information. */
	uint16_t   co_kwdoff;     /* Absolute file offset to optional keyword argument information (`Dec_8BitCodeKwds')
	                           * NOTE: When ZERO(0), there is no keyword information. */
	uint16_t   co_textsiz;    /* Absolute size of this code's text section (in bytes) */
	uint16_t   co_textoff;    /* Absolute file offset to the assembly text that will be executed by this code. */
} Dec_8BitCode;

/* The following 2 structure are taken from /usr/include/i386-linux-gnu/ieee754.h */
/* Copyright (C) 1992-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
typedef union ATTR_PACKED {
	double d;
	struct {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		unsigned int negative : 1;
		unsigned int exponent : 11;
		unsigned int mantissa0 : 20;
		unsigned int mantissa1 : 32;
#elif __FLOAT_WORD_ORDER__ == 4321
		unsigned int mantissa0 : 20;
		unsigned int exponent : 11;
		unsigned int negative : 1;
		unsigned int mantissa1 : 32;
#else /* Endian... */
		unsigned int mantissa1 : 32;
		unsigned int mantissa0 : 20;
		unsigned int exponent : 11;
		unsigned int negative : 1;
#endif /* !Endian... */
	} ieee;
	struct {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		unsigned int negative : 1;
		unsigned int exponent : 11;
		unsigned int quiet_nan : 1;
		unsigned int mantissa0 : 19;
		unsigned int mantissa1 : 32;
#elif __FLOAT_WORD_ORDER__ == 4321
		unsigned int mantissa0 : 19;
		unsigned int quiet_nan : 1;
		unsigned int exponent : 11;
		unsigned int negative : 1;
		unsigned int mantissa1 : 32;
#else /* Endian... */
		unsigned int mantissa1 : 32;
		unsigned int mantissa0 : 19;
		unsigned int quiet_nan : 1;
		unsigned int exponent : 11;
		unsigned int negative : 1;
#endif /* !Endian... */
	} ieee_nan;
} Dec_host_ieee754_double;

typedef union ATTR_PACKED {
	struct {
		unsigned int mantissa1 : 32;
		unsigned int mantissa0 : 20;
		unsigned int exponent : 11;
		unsigned int negative : 1;
	} ieee;
	struct {
		unsigned int mantissa1 : 32;
		unsigned int mantissa0 : 19;
		unsigned int quiet_nan : 1;
		unsigned int exponent : 11;
		unsigned int negative : 1;
	} ieee_nan;
} Dec_ieee754_double;


typedef struct ATTR_PACKED {
	uint16_t           co_name; /* Name of the operator (one of `OPERATOR_*') */
	uint16_t           co_addr; /* [<= :cd_cmemb_size] Index into the class member
	                             * table, to where the operator callback can be bound. */
} Dec_ClassOperator;

typedef struct ATTR_PACKED {
	uint16_t           ca_addr;          /* Address of the attribute (behavior depends on flags) */
	uint16_t           ca_flags;         /* Attribute flags (Set of `CLASS_ATTRIBUTE_F*') */
	uint8_t            ca_nam[1];        /* Name of the attribute (Pointer to a NUL-terminated string within the `e_stroff' string table)
	                                      * NOTE: Decode using `Dec_DecodePointer()' */
	uint8_t            ca_doclen[1];     /* The length of the documentation string (when ZERO, there is doc-string)
	                                      * NOTE: Decode using `Dec_DecodePointer()' */
	uint8_t            ca_doc[1];        /* Only exists when `ca_doclen' evaluates to non-zero:
	                                      * The offset into `e_stroff' of the documentation string's start.
	                                      * NOTE: Decode using `Dec_DecodePointer()' */
} Dec_ClassAttribute;

typedef struct ATTR_PACKED {
	uint16_t           cd_flags;         /* Additional flags to set for the resulting type (set of `TP_F*'). */
	uint8_t            cd_nam[1];        /* Name of the class (Pointer to a NUL-terminated string within the `e_stroff' string table)
	                                      * NOTE: Decode using `Dec_DecodePointer()' */
	uint8_t            cd_doclen[1];     /* The length of the documentation string (when ZERO, there is doc-string)
	                                      * NOTE: Decode using `Dec_DecodePointer()' */
	uint8_t            cd_doc[1];        /* Only exists when `cd_doclen' evaluates to non-zero:
	                                      * The offset into `e_stroff' of the documentation string's start.
	                                      * NOTE: Decode using `Dec_DecodePointer()' */
	uint16_t           cd_cmemb_size;    /* The allocation size of the class member table. */
	uint16_t           cd_imemb_size;    /* The allocation size of the instance member table. */
	uint16_t           cd_op_count;      /* Amount of user-defined operator bindings. */
	uint8_t            cd_cattr_count[1];/* Amount of class attributes (Decode using `Dec_DecodePointer()'). */
	uint8_t            cd_iattr_count[1];/* Amount of instance attributes (Decode using `Dec_DecodePointer()'). */
	Dec_ClassOperator  cd_op_list[1];    /* [cd_op_count] List of operator bindings. */
	Dec_ClassAttribute cd_cattr_list[1]; /* [cd_cattr_count] List of class attributes. */
	Dec_ClassAttribute cd_iattr_list[1]; /* [cd_iattr_count] List of instance attributes. */
} Dec_ClassDescriptor;

typedef struct ATTR_PACKED {
	uint8_t                co_name; /* Name of the operator (one of `OPERATOR_*') */
	uint8_t                co_addr; /* [<= :cd_cmemb_size] Index into the class member
	                                 * table, to where the operator callback can be bound. */
} Dec_8BitClassOperator;

typedef struct ATTR_PACKED {
	uint8_t                ca_addr;          /* Address of the attribute (behavior depends on flags) */
	uint8_t                ca_flags;         /* Attribute flags (Set of `CLASS_ATTRIBUTE_F*') */
	uint8_t                ca_nam[1];        /* Name of the attribute (Pointer to a NUL-terminated string within the `e_stroff' string table)
	                                          * NOTE: Decode using `Dec_DecodePointer()' */
	uint8_t                ca_doclen[1];     /* The length of the documentation string (when ZERO, there is doc-string)
	                                          * NOTE: Decode using `Dec_DecodePointer()' */
	uint8_t                ca_doc[1];        /* Only exists when `ca_doclen' evaluates to non-zero:
	                                          * The offset into `e_stroff' of the documentation string's start.
	                                          * NOTE: Decode using `Dec_DecodePointer()' */
} Dec_8BitClassAttribute;

typedef struct ATTR_PACKED {
	uint8_t                cd_flags;         /* Additional flags to set for the resulting type (set of `TP_F*'). */
	uint8_t                cd_nam[1];        /* Name of the class (Pointer to a NUL-terminated string within the `e_stroff' string table)
	                                          * NOTE: Decode using `Dec_DecodePointer()' */
	uint8_t                cd_doclen[1];     /* The length of the documentation string (when ZERO, there is no doc-string)
	                                          * NOTE: Decode using `Dec_DecodePointer()' */
	uint8_t                cd_doc[1];        /* Only exists when `cd_doclen' evaluates to non-zero:
	                                          * The offset into `e_stroff' of the documentation string's start.
	                                          * NOTE: Decode using `Dec_DecodePointer()' */
	uint8_t                cd_cmemb_size;    /* The allocation size of the class member table. */
	uint8_t                cd_imemb_size;    /* The allocation size of the instance member table. */
	uint8_t                cd_op_count;      /* Amount of user-defined operator bindings. */
	uint8_t                cd_cattr_count;   /* Amount of class attributes. */
	uint8_t                cd_iattr_count;   /* Amount of instance attributes. */
	Dec_8BitClassOperator  cd_op_list[1];    /* [cd_op_count] List of operator bindings. */
	Dec_8BitClassAttribute cd_cattr_list[1]; /* [cd_cattr_count] List of class attributes. */
	Dec_8BitClassAttribute cd_iattr_list[1]; /* [cd_iattr_count] List of instance attributes. */
} Dec_8BitClassDescriptor;


typedef struct ATTR_PACKED {
	uint8_t       kwe_nam[1];    /* Name of the keyword (Pointer to a NUL-terminated string within the `e_stroff' string table)
	                              * NOTE: Decode using `Dec_DecodePointer()' */
} Dec_KwdsEntry;

typedef struct ATTR_PACKED {
	uint8_t       kw_siz;        /* The amount of keyword entries. */
	Dec_KwdsEntry kw_members[1]; /* [kw_siz] One entry for each member.
	                              * NOTE: The keyword index (`struct kwds_entry::ke_index')
	                              *       is the index into this vector. */
} Dec_Kwds;





/* Decode a DEC pointer into an offset (usually into the string table)
 * HINT: The DEC pointer encoding format is
 *       identical to ULEB encoding used by DDI. */
LOCAL NONNULL((1)) uint32_t DCALL
Dec_DecodePointer(uint8_t const **__restrict p_ptr) {
	uint8_t const *ptr = *p_ptr;
	uint32_t result = 0;
	uint8_t byte, num_bits = 0;
	do {
		byte = *ptr++;
		result |= (byte & 0x7f) << num_bits;
		num_bits += 7;
	} while (byte & 0x80);
	*p_ptr = ptr;
	return result;
}





/* Object type IDs within a compiled source file (One of DTYPE_*).
 * NOTE: Unless otherwise stated, all operands are encoded in little-endian. */
#define DTYPE_NONE       0x00   /* +0   `Dee_None'       -- Simply the none builtin object (no operand) */
#define DTYPE_IEEE754    0x01   /* +8   `DeeFloat_Type'  -- 64-bit IEEE754 double-precision floating point number (`Dec_ieee754_double')
                                 *                          (byteorder: little-endian, float-word-order: little-endian). */
#define DTYPE_SLEB       0x02   /* +n   `DeeInt_Type'    -- Variable-length, signed immediate integer (s.a. `READ_SLEB()') */
#define DTYPE_ULEB       0x03   /* +n   `DeeInt_Type'    -- Variable-length, unsigned immediate integer (s.a. `READ_ULEB()') */
#define DTYPE_STRING     0x04   /* +n   `DeeString_Type' -- Followed by 1/2 `Dec_DecodePointer()' immediate values, the first being
                                 *                          the length of the string and the second being an offset into `e_stroff'.
                                 *                          Note however that if the first value equals ZERO(0), the second doesn't exist.
                                 *                          NOTE: Unlike debug-string (which are encoded as LATIN-1), text associated
                                 *                                with string objects is encoded as UTF-8 (allowing for full unicode) */
#define DTYPE_TUPLE      0x05   /* +n   `DeeTuple_Type'  -- Followed by `Dec_DecodePointer()' describing the length, then length more `DTYPE_*' codes for each element. */
#define DTYPE_LIST       0x06   /* +n   `DeeList_Type'   -- Followed by `Dec_DecodePointer()' describing the length, then length more `DTYPE_*' codes for each element. */
#define DTYPE_CLASSDESC  0x07   /* +n   `DeeClassDescriptor_Type' -- Followed by an immediate `Dec_8BitClassDescriptor' structure. */
#define DTYPE_FUNCTION   0x08   /* +n   `DeeFunction_Type' -- An immediate `Dec_Code' data structure defining a new code object, packed within a function object. - Following this, referenced objects are stored in-line. */
#define DTYPE_KWDS       0x09   /* +n   `DeeKwds_Type'   -- An immediate `Dec_Kwds' data structure defining a new keywords object. */
/*      DTYPE_           0x0a    */
/*      DTYPE_           0x0b    */
/*      DTYPE_           0x0c    */
#define DTYPE_NULL       0x0d   /* +0   `NULL'           -- A placeholder/NULL object. (Not allowed in all contexts) */
#define DTYPE_CODE       0x0e   /* +n   `DeeCode_Type'   -- An immediate `Dec_Code' data structure defining a new code object. */
#define DTYPE_EXTENDED   0x0f   /* +1+n `?'              -- Followed by an extended DTYPE-code (One of `DTYPE16_*'). */

/* All remaining single-byte type codes refer to individual builtin objects.
 * NOTE: Which set of builtins is made available depends on the
 *      `DBUILTINS_*' that was specified in `e_builtinset'. */
#define DTYPE_ISBUILTIN(x) ((x) > 0x0f)
#define DTYPE_BUILTIN_MIN  0x10 /* Lowest builtin object id. */
#define DTYPE_BUILTIN_MAX  0xff /* Greatest builtin object id. */
#define DTYPE_BUILTIN_NUM  0xf0 /* Max number of single-byte builtin object codes. */

/* 16-bit DEC-type codes. */
#define DTYPE16_NONE        0x0f00 /* Same as `DTYPE_NONE' */
/*      DTYPE16_            0x0f01  */
/*      DTYPE16_            0x0f02  */
/*      DTYPE16_            0x0f03  */
/*      DTYPE16_            0x0f04  */
#define DTYPE16_HASHSET     0x0f05 /* +n     `DeeHashSet_Type'     -- Followed by `Dec_DecodePointer()' describing the length, then length more `DTYPE_*' codes for each element. */
#define DTYPE16_DICT        0x0f06 /* +n     `DeeDict_Type'        -- Followed by `Dec_DecodePointer()' describing the length, then length*2 more `DTYPE_*' codes for each key/item pair (the key appearing first). */
#define DTYPE16_CLASSDESC   0x0f07 /* +n     `DeeClassDescriptor_Type' -- Followed by an immediate `Dec_ClassDescriptor' structure. */
#define DTYPE16_ROSET       0x0f08 /* +n     `DeeRoSet_Type'       -- Encoded the same way as `DTYPE16_HASHSET' */
#define DTYPE16_RODICT      0x0f09 /* +n     `DeeRoDict_Type'      -- Encoded the same way as `DTYPE16_DICT' */
/*      DTYPE16_            0x0f0a  */
/*      DTYPE16_            0x0f0b  */
/*      DTYPE16_            0x0f0c  */
/*      DTYPE16_            0x0f0d  */
/*      DTYPE16_            0x0f0e  */
#define DTYPE16_CELL        0x0f0d /* +n     `DeeCell_Type'  -- Followed either by `DTYPE_NULL' for an empty Cell, or another object that is contained within. */
#define DTYPE16_EXTENDED    0x0f0f /* Reserved for future expansion. */
#define DTYPE16_BUILTIN_MIN 0x0f10 /* All extended type codes greater-or-equal than this refer to a custom builtin SET
                                    * id-0x10, that is then followed by 1 more byte naming the object within that set:
                                    * >> 0F 11 42 // Use builtin object 0x42 from set #0x01  */



/* Builtin object set ids. */
#define DBUILTINS_NORMAL 0 /* The original, normal set of builtin objects is used. */
#define DBUILTINS_MAX    0 /* The greatest (currently) recognized builtin-set ID. */


#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif /* __COMPILER_HAVE_PRAGMA_PACK */




/* Deemon's implementation header of the DEC specifications. */
#ifdef CONFIG_BUILDING_DEEMON
#ifndef CONFIG_NO_DEC

/* Return the builtin object associated with `id'
 * within `set', return `NULL' when the given `set'
 * is unknown, or `id' is unassigned.
 * NOTE: The caller is responsible for passing an `id' that is located
 *       within the inclusive bounds `DTYPE_BUILTIN_MIN...DTYPE_BUILTIN_MAX'.
 *       This function internally asserts this and crashes if that is not the case. */
INTDEF WUNUSED DeeObject *DCALL Dec_GetBuiltin(uint8_t set, uint8_t id);

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
INTDEF WUNUSED NONNULL((1)) uint16_t DCALL Dec_BuiltinID(DeeObject *__restrict obj);
#define DEC_BUILTINID_UNKNOWN            0 /* The object is not recognized as a builtin. */
#define DEC_BUILTINID_MAKE(setid, objid) ((setid) << 8 | (objid))
#define DEC_BUILTINID_SETOF(x)           (((x) & 0xff00) >> 8)
#define DEC_BUILTINID_IDOF(x)            ((x) & 0xff)

#ifdef DEE_SOURCE
#define Dee_module_object    module_object
#define Dee_compiler_options compiler_options
#endif /* DEE_SOURCE */
struct Dee_module_object;
struct Dee_compiler_options;

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
INTDEF WUNUSED NONNULL((1)) DREF /*untracked*/ struct Dee_module_object *DCALL
DeeModule_OpenDec(DeeObject *__restrict input_stream, struct Dee_compiler_options *options,
                  /*utf-8*/ char const *__restrict dec_dirname, size_t dec_dirname_len,
                  uint64_t dee_file_last_modified);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
/* @return:  0: Successfully loaded the given DEC file.
 * @return:  1: The DEC file was out of date or had been corrupted.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_OpenDec(struct Dee_module_object *__restrict mod,
                  DeeObject *__restrict input_stream,
                  struct Dee_compiler_options *options);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
/* Try to free up memory from the dec time-cache. */
INTDEF size_t DCALL DecTime_ClearCache(size_t max_clear);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

#endif /* !CONFIG_NO_DEC */
#endif /* CONFIG_BUILDING_DEEMON */


DECL_END
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */

#endif /* !GUARD_DEEMON_DEC_H */
