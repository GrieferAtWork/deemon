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
/*!export **/
/*!export DECMAG**/
/*!export DI_MAG**/
/*!export Dec_**/
/*!export Dec_XREL_R_**/
/*!export DeeDecWriter_**/
/*!export DeeDec_**/
/*!export DeeDec_Ehdr_**/
/*!export Dee_ALIGNOF_DEC_**/
/*!export Dee_dec_**/
/*!export -ATTR_PACKED*/
#ifndef GUARD_DEEMON_DEC_H
#define GUARD_DEEMON_DEC_H 1 /*!export-*/

#include "api.h"

#include <hybrid/typecore.h> /* __BYTE_TYPE__, __SIZEOF_POINTER__, __SIZEOF_SIZE_T__ */

#include "alloc.h"  /* Dee_Free */
#include "object.h" /* Dee_Decref_unlikely, Dee_XDecref_likely */
#include "serial.h" /* Dee_SERIAL_HEAD, Dee_seraddr_t */
#include "types.h"  /* DREF, DeeObject */

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* int32_t, uintN_t, uintptr_t */

#ifdef DEE_SOURCE
#include <hybrid/byteorder.h> /* __BYTE_ORDER__, __ORDER_BIG_ENDIAN__, __ORDER_LITTLE_ENDIAN__, __ORDER_PDP_ENDIAN__ */
#include <hybrid/host.h>      /* __arm__, __i386__, __x86_64__ */

#include "gc.h"      /* DeeGC_Object, Dee_GC_OBJECT_OFFSET, Dee_gc_head */
#include "heap.h"    /* Dee_HEAPCHUNK_ALIGN, Dee_heapregion */
#include "mapfile.h" /* DeeMapFile_Fini, Dee_SIZEOF_DeeMapFile */
#endif /* DEE_SOURCE */

/*
 * MMAP-Dec files allow for fast object encode/decode:
 *
 * - General idea is still that only types with specific support
 *   (ones that implement "tp_serialize") can be dec-encoded
 * - However:
 *   - Given a file mapping of a ".dec" file, that mapping already contains
 *     fully functional objects (with reference counters, type headers, and
 *     everything already allocated)
 *   - The ".dec" file mapping remains allocated as long as at least 1
 *     contained object still has a non-zero reference counter (or more
 *     specifically: until the embedded `struct Dee_heapregion' is destroyed)
 *
 *
 * Example: "Tuple { "foo", 42 }"
 * >> ...
 * >>              HEAP_HEADER,           // DeeTupleObject
 * >>my_tuple:     1,                     // ob_refcnt  (1: referenced somewhere else; e.g.: DeeCodeObject::co_constv)
 * >>              DREL(&DeeType_Type),   // ob_type    (relocated)
 * >>              2,                     // t_size
 * >>              IREL(&tuple_elem_1),   // t_elem[0]  (relocated)
 * >>              IREL(&tuple_elem_2),   // t_elem[1]  (relocated)
 * >>
 * >>              HEAP_HEADER,           // DeeStringObject
 * >>tuple_elem_1: 1,                     // ob_refcnt  (1: +1: my_tuple)
 * >>              DREL(&DeeString_Type), // ob_type    (relocated)
 * >>              NULL,                  // s_data
 * >>              1234,                  // s_hash
 * >>              3,                     // s_len
 * >>              "foo\0",               // s_str
 * >>
 * >>              HEAP_HEADER,           // DeeIntObject
 * >>tuple_elem_2: 1,                     // ob_refcnt  (1: +1: my_tuple)
 * >>              DREL(&DeeInt_Type),    // ob_type    (relocated)
 * >>              1,                     // ob_size
 * >>              42,                    // ob_digit
 *
 * Advantages:
 * - Pretty much all types can be serialized and stored in a .dec file
 * - Loading of .dec file is **much** faster, because no additional heap allocations are necessary
 * - Objects that are embedded within a .dec file can be linked to their module via their address in
 *   memory (similarly to how statically allocated objects from DEX modules can be linked to their
 *   DEX module). The function to combine DEE and DEX modules here is `DeeModule_OfPointer()'.
 *
 * Disadvantages:
 * - Objects need a custom heap that defines an ABI for serialization. This has been implemented
 *   in the form of `struct Dee_heapregion', but also means that the system `malloc(3)' can no
 *   longer be (directly) used by deemon.
 * - No validation during module loading means HARD CRASHES (i.e. SIGSEGV) if file was corrupt
 *   - A .dec file can easily contain some invalid relocation that could cause it to wrongfully
 *     Dee_Incref() a memory location that isn't some object's "ob_refcnt".
 *   - Because of this, .dec files contain a checksum that is validated during load
 * - .dec files are **much** larger than they used to be in the old model (which essentially
 *   boiled down to being a recursive array of <Magic type code> <variable-length data>, where
 *   "Magic type code" specified the type of object encoded by <variable-length data>).
 */

DECL_BEGIN

struct Dee_module_object;
struct Dee_string_object;
struct Dee_compiler_options;
struct Dee_dec_writer;

typedef uint32_t Dee_dec_addr32_t; /* Offset from start of `Dec_Ehdr' to some other structure. */
typedef int32_t Dee_dec_off32_t;

#ifdef DEE_SOURCE
/* The max size of a DEC file.
 * If a file is larger than this, the loader is allowed to stop its attempts
 * at parsing it and simply act as though it wasn't a DEC file to begin with.
 *
 * Similarly, if the compiler tries to generate a file larger than this, it
 * will not bother to actually emit it, and will just execute it in-place. */
#define DFILE_LIMIT 0xfffffff /* 256MiB */

#define DI_MAG0   0    /* File identification byte 0 index */
#define DECMAG0   0x7f /* Magic number byte 0 */
#define DI_MAG1   1    /* File identification byte 1 index */
#define DECMAG1   'D'  /* Magic number byte 1 */
#define DI_MAG2   2    /* File identification byte 2 index */
#define DECMAG2   'E'  /* Magic number byte 2 */
#define DI_MAG3   3    /* File identification byte 3 index */
#define DECMAG3   'C'  /* Magic number byte 3 */
#define DI_NIDENT 4    /* Number of identification bytes */


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define DVERSION_CUR  0x0001 /* The currently active version of the DEC file format. */
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
#define DVERSION_CUR  0x0100 /* The currently active version of the DEC file format. */
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */


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

#define Dee_DEC_ENDIAN_UNKNOWN 0
#define Dee_DEC_ENDIAN_LITTLE  1
#define Dee_DEC_ENDIAN_BIG     2
#define Dee_DEC_ENDIAN_PDP     3
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define Dee_DEC_ENDIAN Dee_DEC_ENDIAN_LITTLE
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define Dee_DEC_ENDIAN Dee_DEC_ENDIAN_BIG
#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#define Dee_DEC_ENDIAN Dee_DEC_ENDIAN_PDP
#else /* __BYTE_ORDER__ == ... */
#define Dee_DEC_ENDIAN Dee_DEC_ENDIAN_UNKNOWN
#endif /* __BYTE_ORDER__ != ... */


#ifndef CONFIG_NO_DEC
#define Dee_DEC_TYPE_RELOC 0 /* File contains serializable relocations (that are free'd once `DeeDec_Track()' is called) */
#endif /* !CONFIG_NO_DEC */
#define Dee_DEC_TYPE_IMAGE 1 /* File contains non-serializable relocations (that are free'd once `DeeDec_Track()' is called) */

typedef struct Dee_dec_rel Dec_Rel;
typedef struct Dee_dec_rrel Dec_RRel;
typedef struct Dee_dec_rrela Dec_RRela;
#if 0 /* TODO */
typedef struct Dee_dec_xrel Dec_XRel;
#endif


#define DeeDec_Ehdr_OFFSETOF__e_ident                                   0
#define DeeDec_Ehdr_OFFSETOF__e_mach                                    4
#define DeeDec_Ehdr_OFFSETOF__e_type                                    5
#define DeeDec_Ehdr_OFFSETOF__e_version                                 6
#define DeeDec_Ehdr_OFFSETOF__e_typedata                                8
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_heap    8
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_sizeof_pointer   10
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_endian           11
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_eof     12
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_deemon_build_id  16
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_build_timestamp  32
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_gchead  40
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_gctail  44
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_srel    48
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_drel    52
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_drrel   56
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_drrela  60
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_deps    64
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_files   68
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_offsetof_xrel    72
#define DeeDec_Ehdr_OFFSETOF__e_typedata__td_reloc__er_alignment        76
#define DeeDec_Ehdr_OFFSETOF__e_mapping                                 80
#define DeeDec_Ehdr_OFFSETOF__e_heap                             \
	(DeeDec_Ehdr_OFFSETOF__e_mapping + Dee_SIZEOF_DeeMapFile +   \
	 ((Dee_HEAPCHUNK_ALIGN - ((DeeDec_Ehdr_OFFSETOF__e_mapping + \
	                           Dee_SIZEOF_DeeMapFile) %          \
	                          Dee_HEAPCHUNK_ALIGN)) %            \
	  Dee_HEAPCHUNK_ALIGN))
#if DeeDec_Ehdr_OFFSETOF__e_heap == 120
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
#elif DeeDec_Ehdr_OFFSETOF__e_heap == 88
#undef DeeDec_Ehdr_OFFSETOF__e_heap
#define DeeDec_Ehdr_OFFSETOF__e_heap 88
#endif /* DeeDec_Ehdr_OFFSETOF__e_heap == ... */

/* Main header for `.dec` files. For more details, see file ./dec.md */
#define Dee_ALIGNOF_DEC_EHDR 8
struct Dee_dec_depmod;
typedef struct {
	uint8_t               e_ident[DI_NIDENT]; /* [AT(0-3)] Identification bytes. (See `DI_*') */
	uint8_t               e_mach;             /* [AT(4-4)] Machine identification (`Dee_DEC_MACH') */
	uint8_t               e_type;             /* [AT(5-5)] EHDR type (one of `Dee_DEC_TYPE_*') */
	uint16_t              e_version;          /* [AT(6-7)] DEC version number (One of `DVERSION_*') -- NOTE: __MUST__ remain at this specific offset=6 for backwards compatibility! */
	union {

		struct {
			uint16_t          er_offsetof_heap;      /* Offset from start of this struct to `e_heap' (== `DeeDec_Ehdr_OFFSETOF__e_heap') */
			uint8_t           er_sizeof_pointer;     /* Size of pointer (and thus of: relocation targets, and `Dee_refcnt_t') */
			uint8_t           er_endian;             /* Endian type code, specifying the format of all multi-byte fields (== Dee_DEC_ENDIAN) */
			Dee_dec_addr32_t  er_offsetof_eof;       /* [1..1] Offset to EOF of file mapping (should also equal the dec file's size) */
			uint64_t          er_deemon_build_id[2]; /* Deemon build ID (128-bit unsigned integer) */
			uint64_t          er_build_timestamp;    /* Microseconds since `01-01-1970', when this dec file was created. */
			Dee_dec_addr32_t  er_offsetof_gchead;    /* [0..1] Offset to first GC `DeeObject' (tracking for these objects must begin after relocations were done) */
			Dee_dec_addr32_t  er_offsetof_gctail;    /* [0..1] Offset to last GC `DeeObject' (links between these objects were already established via `e_offsetof_srel') */
			Dee_dec_addr32_t  er_offsetof_srel;      /* [1..1] Offset to array of `Dec_Rel[]' (terminated by a r_addr==0-entry) of relocations with "REL_BASE=(uintptr_t)ehdr" */
			Dee_dec_addr32_t  er_offsetof_drel;      /* [1..1] Offset to array of `Dec_Rel[]' (terminated by a r_addr==0-entry) of relocations with "REL_BASE=(uintptr_t)&DeeModule_Deemon" */
			Dee_dec_addr32_t  er_offsetof_drrel;     /* [1..1] Offset to array of `Dec_RRel[]' (terminated by a r_addr==0-entry) of relocations with "REL_BASE=(uintptr_t)&DeeModule_Deemon" */
			Dee_dec_addr32_t  er_offsetof_drrela;    /* [1..1] Offset to array of `Dec_RRela[]' (terminated by a r_addr==0-entry) of relocations with "REL_BASE=(uintptr_t)&DeeModule_Deemon" */
			Dee_dec_addr32_t  er_offsetof_deps;      /* [1..1] Offset to array of `Dec_Dhdr[]' (terminated by a d_modspec.d_mod==NULL-entry) of other dependent deemon modules */
			Dee_dec_addr32_t  er_offsetof_files;     /* [0..1] Offset to array of `Dec_Dstr[]' (terminated by a ds_length==0-entry, each aligned to Dee_ALIGNOF_DEC_DSTR) of extra filenames relative to the directory containing the .dec-file. If any of these files is newer than `e_build_timestamp', don't load dec file */
			Dee_dec_addr32_t  er_offsetof_xrel;      /* [0..1] Offset to array of `Dec_XRel[]' (terminated by a xr_addr==0-entry) of extended relocations */
			Dee_dec_addr32_t  er_alignment;          /* Minimum alignment with which the dec file must be loaded in memory. */
		}                 td_reloc;                  /* [valid_if(e_type == Dee_DEC_TYPE_RELOC)] */

		struct {
			Dec_RRel              *ei_drrel_v;  /* [0..ei_drrel_c][owned] List of incref-relocations against deemon-core objects */
			size_t                 ei_drrel_c;  /* # of entries in `ei_drrel_v' */
			Dec_RRela             *ei_drrela_v; /* [0..ei_drrela_c][owned] List of incref-offset-relocations against deemon-core objects */
			size_t                 ei_drrela_c; /* # of entries in `ei_drrela_v' */
			struct Dee_dec_depmod *ei_deps_v;   /* [0..ei_deps_c][owned] Vector of dependent modules */
			size_t                 ei_deps_c;   /* # of entries in `ei_deps_v' */
#if __SIZEOF_SIZE_T__ == 4 && __SIZEOF_POINTER__ == 4
			size_t                _ei_pad[2];   /* So "ei_offsetof_gchead" has the same offset as "er_offsetof_gchead" */
#endif /* __SIZEOF_SIZE_T__ == 4 && __SIZEOF_POINTER__ == 4 */
			Dee_seraddr_t          ei_offsetof_gchead; /* [1..1] Offset to first `struct Dee_gc_head::gc_self' */
			Dee_seraddr_t          ei_offsetof_gctail; /* [1..1] Offset to last `struct Dee_gc_head::gc_self' */
		}                 td_image;             /* [valid_if(e_type == Dee_DEC_TYPE_IMAGE)] */

	}                     e_typedata;         /* Data dependent on `e_type' */
	struct DeeMapFile     e_mapping;          /* Uninitialized/unused in file mappings; when mapped into memory, populated with the dec file's own file map descriptor. */
#if ((DeeDec_Ehdr_OFFSETOF__e_mapping + Dee_SIZEOF_DeeMapFile) % Dee_HEAPCHUNK_ALIGN) != 0
	__BYTE_TYPE__ _e_heap_pad[Dee_HEAPCHUNK_ALIGN - ((DeeDec_Ehdr_OFFSETOF__e_mapping + Dee_SIZEOF_DeeMapFile) % Dee_HEAPCHUNK_ALIGN)];
#endif /* (Dee_SIZEOF_DeeMapFile % Dee_HEAPCHUNK_ALIGN) != 0 */
	struct Dee_heapregion e_heap;             /* Heap region descriptor for objects embedded within this dec file. The first chunk of
	                                           * this heap is assumed to point at the `DeeModuleObject' describing the dec file itself. */
} Dec_Ehdr;

#define DeeDec_Ehdr_IMAGE_GetGCHead(self) (Dee_ASSERT((self)->e_typedata.td_image.ei_offsetof_gchead), (DeeObject *)((__BYTE_TYPE__ *)(self) + (self)->e_typedata.td_image.ei_offsetof_gchead))
#define DeeDec_Ehdr_IMAGE_GetGCTail(self) (Dee_ASSERT((self)->e_typedata.td_image.ei_offsetof_gctail), (DeeObject *)((__BYTE_TYPE__ *)(self) + (self)->e_typedata.td_image.ei_offsetof_gctail))
#define DeeDec_Ehdr_RELOC_GetGCHead(self) (Dee_ASSERT((self)->e_typedata.td_reloc.er_offsetof_gchead), (DeeObject *)((__BYTE_TYPE__ *)(self) + (self)->e_typedata.td_reloc.er_offsetof_gchead))
#define DeeDec_Ehdr_RELOC_GetGCTail(self) (Dee_ASSERT((self)->e_typedata.td_reloc.er_offsetof_gctail), (DeeObject *)((__BYTE_TYPE__ *)(self) + (self)->e_typedata.td_reloc.er_offsetof_gctail))

#define Dee_ALIGNOF_DEC_REL 4
struct Dee_dec_rel {
	/* Non-reference-counted relocation:
	 * >> byte_t **p_ptr = (byte_t **)((byte_t *)EHDR + r_addr);
	 * >> *p_ptr += REL_BASE; */
	Dee_dec_addr32_t r_addr; /* Offset from start of "Dec_Ehdr" to relocated pointer */
};

#define Dee_ALIGNOF_DEC_RREL 4
struct Dee_dec_rrel {
	/* Reference-counted relocation:
	 * >> byte_t **p_ptr = (byte_t **)((byte_t *)EHDR + r_addr);
	 * >> DeeObject *obj = (DeeObject *)(*p_ptr += REL_BASE);
	 * >> ASSERT_OBJECT(obj);
	 * >> Dee_Incref(obj); */
	Dee_dec_addr32_t r_addr; /* Offset from start of "Dec_Ehdr" to relocated pointer */
};

#define Dee_ALIGNOF_DEC_RRELA 4
struct Dee_dec_rrela {
	/* Reference-counted+addend relocation:
	 * >> byte_t **p_ptr = (byte_t **)((byte_t *)EHDR + r_addr);
	 * >> byte_t *pointer = (*p_ptr += REL_BASE);
	 * >> DeeObject *obj = (DeeObject *)(pointer + r_offs);
	 * >> ASSERT_OBJECT(obj);
	 * >> Dee_Incref(obj); */
	Dee_dec_addr32_t r_addr; /* Offset from start of "Dec_Ehdr" to relocated pointer */
	Dee_dec_off32_t  r_offs; /* Offset added to resulting pointer to get to base of object that must be incref'd */
};

#if 0 /* TODO */
#define Dec_XREL_R_NONE    0 /* End-of-extended-relocation-table (`xr_addr' is unused and not necessarily allocated) */
#define Dec_XREL_R_FILE    1 /* "xr_addr" points at a `Dec_Dstr' which is a relative (to the dir containing
                              * the .dec file), or absolute filename of a file that was `#include'ed by the
                              * associated ".dee" file, and similarly to said ".dee" file, must not be newer
                              * than `er_build_timestamp' */
#define Dec_XREL_R_WEAKREF 2 /* "xr_addr" points at a `struct Dee_weakref' currently initialized as:
                              * - wr_pself: NULL
                              * - wr_next:  NULL
                              * - wr_obj:   &SOME_OBJECT
                              * - wr_del:   <IGNORED>
                              *
                              * This relocation will now try to initialize "wr_pself" and "wr_next" such that
                              * they become part of "SOME_OBJECT" (which should have been initialized by some
                              * other relocation against a weakref-able object from some dependent module).
                              * - If it turns out that "SOME_OBJECT" has already been destroyed (which can be
                              *   detected by its "ob_refcnt" already being "0"), nothing is done, and `wr_obj'
                              *   is set to NULL.
                              *
                              * Implementation note: the actual inserting into the "ob_weakrefs" of "wr_obj"
                              * must **ONLY** happen after/during `DeeDec_Track()'. If it were done before
                              * then, parts of the incomplete dec file would already be visible to objects
                              * outside the dec file (which mustn't happen since dec files must become visible
                              * all-at-once, and not one-piece-at-a-time)
                              */
#define Dec_XREL_R_NUM     3 /* # of valid extended relocations (values >= this must be interpreted as a corrupted dec file) */


#define Dee_ALIGNOF_DEC_XREL 4
struct Dee_dec_xrel {
	Dee_dec_addr32_t xr_type; /* Extended relocation type (one of `Dec_XREL_R_*') */
	Dee_dec_addr32_t xr_addr; /* Address of relocation or payload (what's at that address depends on `xr_type') */
};
#endif

#define Dee_ALIGNOF_DEC_DHDR 8
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
	/* TODO: Dec_XRel Extended relocations against this module (for stuff like "Dee_weakref")
	 * Though, maybe just have a single "Dec_XRel" array per dec file... */
	uint64_t         d_buildid[2];     /* Expected build ID for this dependency */
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
	((struct Dee_module_object *)DeeGC_Object((struct Dee_gc_head *)(&(self)->e_heap.hr_first + 1)))
#define DeeDec_Ehdr_FromModule(self)                                               \
	((Dec_Ehdr *)((uintptr_t)Dee_REQUIRES_TYPE(struct Dee_module_object *, self) - \
	              (COMPILER_OFFSETAFTER(Dec_Ehdr, e_heap.hr_first) +               \
	               Dee_GC_OBJECT_OFFSET)))

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
	struct Dee_dec_rreltab         ddm_rrel;   /* Incref-relocations against `ddm_mod' (increfs already happened here) */
	struct Dee_dec_rrelatab        ddm_rrela;  /* Incref-addend-relocations against `ddm_mod' (increfs already happened here) */
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
	void const   *dpte_ptr; /* [1..1] Source address that was already encoded */
	Dee_seraddr_t dote_off; /* Offset from `dw_base' to where "dpte_ptr" was written */
	size_t        dote_siz; /* # of bytes associated with "dpte_ptr" */
};
#define Dee_dec_ptrtab_entry_getminaddr(self) ((__BYTE_TYPE__ *)(self)->dpte_ptr)
#define Dee_dec_ptrtab_entry_getmaxaddr(self) ((__BYTE_TYPE__ *)(self)->dpte_ptr + (self)->dote_siz - 1)

struct Dee_dec_ptrtab {
	struct Dee_dec_ptrtab_entry *dpt_ptrv; /* [0..dpt_ptrc][SORT(dpte_ptr ASC)][owned] List of already-encoded objects */
	size_t                       dpt_ptrc; /* # of serialized pointers */
	size_t                       dpt_ptra; /* Allocated # of entries in `dpt_ptrv' */
};
#define Dee_dec_ptrtab_init(self) \
	(void)((self)->dpt_ptrv = NULL, (self)->dpt_ptrc = (self)->dpt_ptra = 0)
#define Dee_dec_ptrtab_fini(self) \
	Dee_Free((self)->dpt_ptrv)

struct Dee_dec_fdeptab {
	__BYTE_TYPE__   *dfdt_depv; /* [0..dfdt_depc][owned] Flat array of `Dec_Dstr' (each aligned to `__ALIGNOF_SIZE_T__') */
	size_t           dfdt_depc; /* # of bytes used in "dfdt_depv" */
	size_t           dfdt_depa; /* Allocated # of bytes in `dfdt_depv' */
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
	Dee_seraddr_t           dw_used;   /* [<= dw_alloc] Used buffer size for `dw_base' */
	size_t                  dw_align;  /* Minimum alignment requirements for the resulting dec file */
	size_t                  dw_hlast;  /* Chunk size during the previous call to `DeeDecWriter_Malloc()' */
	Dee_seraddr_t           dw_slabb;   /* [<= dw_alloc][valid_if(dw_slabs != 0)] Address of the `struct Dee_heapchunk' preceding
	                                     * the properly aligned base of the most-recently allocated section of slab pages. When
	                                     * valid, this value is `IS_ALIGNED(. + sizeof(struct Dee_heapchunk), Dee_SLAB_PAGESIZE)'
	                                     * During allocation, `dw_used' is *NOT* adjusted to point after the slab chunk.
	                                     * Instead, heap allocation checks if it might collide with slab pages, and if it does,
	                                     * the last preceding heap allocation is only extended to join up with the allocation
	                                     * of the slab pages themselves, before then continuing on after the slabs:
	                                     * >> 0          4096    8182
	                                     * >> HDR HEAP...SLAB....HEAP */
	size_t                  dw_slabs;   /* # of bytes reserved at `dw_slabb' for slab memory (always 'sizeof(struct Dee_heapchunk)'
	                                     * plus a multiple of `Dee_SLAB_PAGESIZE'). When more slab pages are needed, check if `dw_used'
	                                     * has grown beyond the last slab page, and if so: start a new chunk of slab pages. Otherwise,
	                                     * extend the previous slab segment by one more page. */
	struct Dee_dec_reltab   dw_srel;   /* Table of self-relocations */
	struct Dee_dec_reltab   dw_drel;   /* Table of relocations against deemon-core objects */
	struct Dee_dec_rreltab  dw_drrel;  /* Table of incref-relocations against deemon-core objects (increfs already happened here) */
	struct Dee_dec_rrelatab dw_drrela; /* Table of incref-relocations against deemon-core objects (increfs already happened here) */
	struct Dee_dec_deptab   dw_deps;   /* Table of dependent modules */
	struct Dee_dec_fdeptab  dw_fdeps;  /* Table of dependent files */
	Dee_seraddr_t           dw_gchead; /* [0..1] Offset to first `struct Dee_gc_head::gc_self' (tracking for these objects must begin after relocations were done) */
	Dee_seraddr_t           dw_gctail; /* [0..1] Offset to last `struct Dee_gc_head::gc_self' (links between these objects were already established via `dw_srel') */
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
DFUNDEF NONNULL((1)) void DCALL
DeeDecWriter_Fini(DeeDecWriter *__restrict self);


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
DFUNDEF WUNUSED NONNULL((1, 2)) DREF /*untracked*/ struct Dee_module_object *DCALL
DeeDec_Relocate(/*inherit(on_success)*/ DeeDec_Ehdr **__restrict p_self,
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

struct DeeMapFile;

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

#endif /* !GUARD_DEEMON_DEC_H */
