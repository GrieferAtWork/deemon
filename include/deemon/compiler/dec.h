/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_DEC_H
#define GUARD_DEEMON_COMPILER_DEC_H 1

#include "../api.h"

#ifdef CONFIG_BUILDING_DEEMON
#ifndef CONFIG_NO_DEC
#include "../code.h"
#include "../dec.h"
#include "../module.h"
#include "../object.h"

DECL_BEGIN


/* DEC writer relocation types. */
#define DECREL_NONE       0
#define DECREL_ABS16      1 /* `u16 = u16 + dr_sym->ds_addr' */
#define DECREL_ABS32      2 /* `u32 = u32 + dr_sym->ds_addr' */
#define DECREL_ABS16_NULL 3 /* `u16 = u16 + (DEC_SECTION_ISEMPTY(dr_sym->ds_sect) ? 0 : dr_sym->ds_addr)' */
#define DECREL_ABS32_NULL 4 /* `u32 = u32 + (DEC_SECTION_ISEMPTY(dr_sym->ds_sect) ? 0 : dr_sym->ds_addr)' */
#define DECREL_SIZE32     5 /* `u32 = u32 + (dr_sym->ds_sect->ds_iter - dr_sym->ds_sect->ds_begin)' */


#define DEC_SECTION_HEADER     0 /* Section containing the header. */
#define DEC_SECTION_IMPORTS    1 /* Section containing the import table (pointed to by `e_impoff'). */
#define DEC_SECTION_DEPS       2 /* Section containing additional dependency data (pointed to by `e_depoff'). */
#define DEC_SECTION_GLOBALS    3 /* Section containing global symbols (pointed to by `e_globoff'). */
#define DEC_SECTION_ROOT       4 /* Section containing the root code object (pointed to by `e_rootoff'). */
/* ... Split: Dynamically allocated sections are inserted here. */
#define DEC_SECTION_TEXT       5 /* Section containing user assembly text (used by all contained code objects). */
#define DEC_SECTION_DEBUG      6 /* Section containing DDI object descriptors. */
#define DEC_SECTION_DEBUG_DATA 7 /* Section containing DDI object data (string tables). */
#define DEC_SECTION_DEBUG_TEXT 8 /* Section containing DDI assembly text (used by all contained DDI objects). */
#define DEC_SECTION_STRING     9 /* Section containing the entirety of string data. */
#define DEC_SECTION_COUNT     10 /* The number of builtin, standard sections. */


struct dec_sym {
	struct dec_sym     *ds_next; /* [0..1][owned] Next symbol. */
	struct dec_section *ds_sect; /* [0..1] Section within which this symbol is defined. */
	uint32_t            ds_addr; /* Offset into `ds_sect' where the symbol is defined. */
};
#define DEC_SYM_DEFINED(x) ((x)->ds_sect != NULL)

struct dec_rel {
	struct dec_sym *dr_sym;    /* [0..1] Symbol against which the relocation is performed. */
	uint32_t        dr_addr;   /* Offset into the associated section where this relocation must be applied. */
	uint8_t         dr_type;   /* Relocation type (One of `DECREL_*') */
#ifndef __INTELLISENSE__
	uint8_t         dr_pad[3]; /* ... */
#endif
};

struct dec_section {
	struct dec_section *ds_next;  /* [0..1][owned] Additional sections inserted after this one, but before the next. */
	uint8_t            *ds_begin; /* [0..1][owned] Allocated base pointer. */
	uint8_t            *ds_iter;  /* [(!= NULL) == (ds_begin != NULL)][>= ds_begin && <= ds_end] Target text pointer. */
	uint8_t            *ds_end;   /* [0..1][owned] Allocated base pointer. */
	size_t              ds_relc;  /* Used amount of relocations. */
	size_t              ds_rela;  /* Allocated amount of relocations. */
	struct dec_rel     *ds_relv;  /* [0..ds_relc|ALLOC(ds_rela)][SORT(ASCENDING(->dr_addr))][owned] Vector of relocations. */
	uint32_t            ds_base;  /* Base file address of the section (used during final linkage) */
	struct dec_sym      ds_start; /* A statically allocated symbol pointing to the start of this section. */
#if __SIZEOF_POINTER__ > 4
	uint32_t            ds_pad;   /* ... */
#endif
};
#define DEC_SECTION_ISEMPTY(x) ((x)->ds_iter == (x)->ds_begin)

struct dec_writer {
	struct dec_section  dw_sec_defl[DEC_SECTION_COUNT]; /* Default (builtin) sections. */
	struct dec_section *dw_sec_curr;      /* [1..1] The currently selected section. */
	struct dec_sym     *dw_symbols;       /* [0..1][owned] Chain of all symbols that have been allocated. */
#define DEC_WRITE_FNORMAL       0x0000    /* Normal DEC writer flags. */
#define DEC_WRITE_FREUSE_GLOBAL 0x0001    /* Try to reduce the size of the DEC file by
	                                       * merging mirror copies of data blocks found anywhere
	                                       * within the file, when both blocks of data are pointed
	                                       * to by file offsets found anywhere else in the file.
	                                       * This optimization is expensive but greatly helps to
	                                       * counteract data redundancy that can easily be introduced
	                                       * through duplicate user-functions, etc. etc.
	                                       * Due to the fact that the actual yield from this option is
	                                       * fairly low when compared against its practicality and most
	                                       * importantly: it's impact on performance, to use this option
	                                       * you must explicitly opt-in when enabling it, as it is
	                                       * disabled by default. */
#define DEC_WRITE_FNODEBUG      0x0002    /* Don't generate DDI object, or DDI text, leaving the
	                                       * `DEC_SECTION_DEBUG' and `DEC_SECTION_DEBUG_TEXT' sections empty. */
#define DEC_WRITE_FNODOC        0x0004    /* Don't include documentation strings in the generated DEC file. */
#define DEC_WRITE_FBIGFILE      0x8000    /* Try to never make use of `DECREL_ABS16' or `DECREL_ABS16_NULL' relocations.
	                                       * When the final link fails because of a `DECREL_ABS16' or `DECREL_ABS16_NULL'
	                                       * relocations being truncated, this flag is set and linking is restarted. */
	uint16_t            dw_flags;
	uint8_t             dw_objset;        /* The default set of builtin objects written within `DEC_SECTION_HEADER'. */
};
#define DEC_FOREACH_SECTION_VARS \
	struct dec_section *_main_section
#define DEC_FOREACH_SECTION(sec)                                   \
	for (_main_section = current_dec.dw_sec_defl;                  \
	     _main_section != COMPILER_ENDOF(current_dec.dw_sec_defl); \
	     ++_main_section)                                          \
		for ((sec) = _main_section; (sec); (sec) = (sec)->ds_next)


/* The currently active DEC writer. */
INTDEF struct dec_writer current_dec;

/* Initialize/finalize the global DEC writer context. */
INTDEF void DCALL dec_writer_init(void);
INTDEF void DCALL dec_writer_fini(void);

#define dec_ptr                 current_dec.dw_sec_curr->ds_iter
#define dec_addr     (uint32_t)(current_dec.dw_sec_curr->ds_iter-current_dec.dw_sec_curr->ds_begin)
#define dec_curr                current_dec.dw_sec_curr
#define dec_setcurr(x)         (current_dec.dw_sec_curr = (x))

/* >> uint32_t dec_ptr2addr(uint8_t *);
 * Return the section-relative address of a given buffer pointer. */
#define dec_ptr2addr(x) (uint32_t)((x)-current_dec.dw_sec_curr->ds_begin)

/* >> uint8_t *dec_ptr2addr(uint32_t);
 * Return the buffer pointer of a given section-relative address. */
#define dec_addr2ptr(x)           ((x)+current_dec.dw_sec_curr->ds_begin)

/* Allocate `n_bytes' of data at the current
 * text position within the current section.
 * WARNING: Multiple successive calls to this function
 *          may invalid previously returned pointers. */
INTDEF uint8_t *DCALL dec_alloc(size_t n_bytes);

/* Search for an existing instance of `data...+=n_bytes' within the
 * current section and either return a pointer to it, or append the
 * given data block at its end and return a pointer to where it starts.
 * If the later case fails to re-allocate the section buffer, `NULL' is returned. */
INTDEF uint8_t *DCALL dec_allocstr(void const *__restrict data, size_t n_bytes);

/* Take a look at the last-written `n_bytes' of data and search
 * all sections (that could reasonably contain a mirror copy) for
 * a copy of the last written `n_bytes' of data.
 * WARNING: This function does not take relocations into consideration!
 * If such a copy is found, the last written `n_bytes' of data
 * are deleted and the given `sym' is defined to point at the
 * existing shadow copy.
 * If such a copy could not be found or if the `DEC_WRITE_FREUSE_GLOBAL'
 * flag isn't set, `sym' is defined to point at the start of searched
 * data, located at `dec_curr:dec_addr-n_bytes'. */
INTDEF void DCALL dec_reuseglobal_define(struct dec_sym *__restrict sym, size_t n_bytes);
/* Similar to `dec_reuseglobal_define', but only operates within the current
 * section, and is not bound by the rules of the `DEC_WRITE_FREUSE_GLOBAL'.
 * The return value is a pointer to the first block of data found that
 * equals the data described within the last `n_bytes' of written memory,
 * or a pointer equal to `dec_ptr - n_bytes'. */
INTDEF ATTR_RETNONNULL uint8_t *DCALL dec_reuselocal(size_t n_bytes);

/* Given some data encoded in host-endian, convert it to
 * little-endian and write it to the current text position
 * before advancing the text pointer. */
INTDEF int (DCALL dec_putb)(uint8_t byte);
INTDEF int (DCALL dec_putw)(uint16_t host_endian_word);
INTDEF int (DCALL dec_putl)(uint32_t host_endian_dword);
#ifdef __INTELLISENSE__
INTDEF int (DCALL dec_putc)(char ch);
#else /* __INTELLISENSE__ */
#define dec_putc(ch) dec_putb((uint8_t)(ch))
#endif /* !__INTELLISENSE__ */

/* Encode a given floating point value as an ieee754-double */
INTDEF int (DCALL dec_putieee754)(double value);

/* Encode the given `value' such that it
 * can later be decoded using `Dec_DecodePointer()' */
INTDEF int (DCALL dec_putptr)(uint32_t value);

/* Encode LEB integers. */
INTDEF int (DCALL dec_putsleb)(int value);
#ifdef __INTELLISENSE__
INTDEF int (DCALL dec_putuleb)(unsigned int value);
#else /* __INTELLISENSE__ */
#define dec_putuleb(value)  dec_putptr((uint32_t)(value))
#endif /* !__INTELLISENSE__ */


/* Create, insert and return a new section that will
 * appear after `sect' in the resulting DEC file. */
INTDEF struct dec_section *DCALL
dec_newsection_after(struct dec_section *__restrict sect);

/* Allocate a new, undefined DEC symbol.
 * NOTE: Should the symbol appear in at least one relocation,
 *       then it must also be defined at some point.
 *       There is no unresolved-relocation error here.
 *       If you forget to define it, `dec_link' will crash
 *       with an assertion error. */
INTDEF struct dec_sym *DCALL dec_newsym(void);


/* Define the given symbol at the current text position. */
LOCAL void DCALL
dec_defsym(struct dec_sym *__restrict sym) {
	Dee_ASSERT(!DEC_SYM_DEFINED(sym));
	/* Simply set the section and address to what is currently active. */
	sym->ds_sect = dec_curr;
	sym->ds_addr = dec_addr;
}

/* Define the given symbol at the given address within the current section. */
LOCAL void DCALL
dec_defsymat(struct dec_sym *__restrict sym, uint32_t addr) {
	Dee_ASSERT(!DEC_SYM_DEFINED(sym));
	/* Simply set the section and address. */
	sym->ds_sect = dec_curr;
	sym->ds_addr = addr;
}

/* Allocate a new, uninitialized relocation at the end of the current section.
 * The caller is responsible to ensure that the relocation will be defined
 * after the location of the previously allocated relocation.
 * WARNING: Multiple successive calls to this function
 *          may invalid previously returned pointers. */
INTDEF struct dec_rel *DCALL dec_newrel(void);

/* Create a new relocation `type' against `sym' at the
 * current text address within the current section.
 * The caller must ensure that `sym' be only `NULL'
 * when the relocation associated with `type' doesn't
 * make use of a symbol. */
INTDEF int (DCALL dec_putrel)(uint8_t type, struct dec_sym *sym);
INTDEF int (DCALL dec_putrelat)(uint32_t addr, uint8_t type, struct dec_sym *sym);

/* @return: DECREL_NONE: Successfully linked all DEC sections.
 * @return: * :          One of `DECREL_*' that was truncated, causing linking to fail. */
INTDEF uint8_t (DCALL dec_link)(void);
/* Assign base addresses to all allocated sections. */
INTDEF void DCALL dec_setbases(void);

/* Write all sections (in order) to the given `file_stream'.
 * For this purpose, file write operations from <deemon/file.h> are
 * using, meaning that `file_stream' should be derived from `DeeFile_Type'.
 * @return:  0: Successfully written the DEC file.
 * @return: -1: An error occurred while writing. */
INTDEF int (DCALL dec_write)(DeeObject *__restrict file_stream);

/* Generate DEC code for compiling the given module.
 * @throw NotImplemented Somewhere during generation process, a constant
 *                       could not be encoded a DTYPE expression.
 * @return:  0: Successfully populated DEC sections.
 * @return: -1: An error occurred. */
INTDEF int (DCALL dec_generate)(DeeModuleObject *__restrict module);
/* Generate a link DEC text for the given module. */
INTDEF int (DCALL dec_generate_and_link)(DeeModuleObject *__restrict module);

/* Encode an object using DTYPE codes. */
INTDEF int (DCALL dec_putobj)(DeeObject *self);
INTDEF int (DCALL dec_putcode)(DeeCodeObject *__restrict self);
INTDEF int (DCALL dec_putobjv)(uint16_t count, DeeObject **__restrict vec); /* `Dec_Objects' */

/* Create and emit a DEC file for the given module. */
INTDEF int (DCALL dec_create)(DeeModuleObject *__restrict module);

#if 0
/* Similar to `dec_create', but write data to the given `file_stream' */
INTDEF int (DCALL dec_createfp)(DeeModuleObject *__restrict module,
                                DeeObject *__restrict file_stream);
#endif

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define dec_putb(byte)                __builtin_expect(dec_putb(byte), 0)
#define dec_putw(host_endian_word)    __builtin_expect(dec_putw(host_endian_word), 0)
#define dec_putl(host_endian_dword)   __builtin_expect(dec_putl(host_endian_dword), 0)
#define dec_putieee754(value)         __builtin_expect(dec_putieee754(value), 0)
#define dec_putsleb(value)            __builtin_expect(dec_putsleb(value), 0)
#define dec_putptr(value)             __builtin_expect(dec_putptr(value), 0)
#define dec_putrel(type, sym)         __builtin_expect(dec_putrel(type, sym), 0)
#define dec_putrelat(addr, type, sym) __builtin_expect(dec_putrelat(addr, type, sym), 0)
#define dec_link()                    __builtin_expect(dec_link(), DECREL_NONE)
#define dec_write(file_stream)        __builtin_expect(dec_write(file_stream), 0)
#define dec_generate(module)          __builtin_expect(dec_generate(module), 0)
#define dec_putobj(self)              __builtin_expect(dec_putobj(self), 0)
#define dec_putcode(self)             __builtin_expect(dec_putcode(self), 0)
#define dec_putobjv(count, vec)       __builtin_expect(dec_putobjv(count, vec), 0)
#define dec_create(module)            __builtin_expect(dec_create(module), 0)
#endif
#endif

DECL_END

#endif /* !CONFIG_NO_DEC */
#endif /* CONFIG_BUILDING_DEEMON */

#endif /* !GUARD_DEEMON_COMPILER_DEC_H */
