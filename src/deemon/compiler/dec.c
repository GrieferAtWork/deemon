/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_DEC_C
#define GUARD_DEEMON_COMPILER_DEC_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <deemon/api.h>

#ifndef CONFIG_NO_DEC
#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/compiler/dec.h>
#include <deemon/dec.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/unaligned.h>

#include <string.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#else /* CONFIG_HOST_WINDOWS */
#include <unistd.h>
#endif /* !CONFIG_HOST_WINDOWS */

DECL_BEGIN

#ifndef __USE_GNU
#define memrchr dee_memrchr
LOCAL void *dee_memrchr(void const *__restrict p, int c, size_t n) {
	uint8_t *iter = (uint8_t *)p + n;
	while (iter != (uint8_t *)p) {
		if (*--iter == c)
			return iter;
	}
	return NULL;
}
#endif /* !__USE_GNU */

/* linux's memmem() doesn't do what we need it to do. - We
 * need it to return `NULL' when `needle_length' is 0
 * Additionally, there has never been a point where
 * it was working entirely flawless. */
#if !defined(__USE_KOS) || !defined(__USE_GNU)
#define memmem dee_memmem
LOCAL void *dee_memmem(void const *__restrict haystack, size_t haystack_length,
                       void const *__restrict needle, size_t needle_length) {
	uint8_t *candidate;
	uint8_t marker;
	if unlikely(!needle_length || needle_length > haystack_length)
		return NULL;
	haystack_length -= (needle_length - 1), marker = *(uint8_t *)needle;
	while ((candidate = (uint8_t *)memchr(haystack, marker, haystack_length)) != NULL) {
		if (memcmp(candidate, needle, needle_length) == 0)
			return (void *)candidate;
		++candidate;
		haystack_length = ((uint8_t *)haystack + haystack_length) - candidate;
		haystack        = (void const *)candidate;
	}
	return NULL;
}
#endif /* !__KOS__ || !__USE_GNU */


INTERN struct dec_writer current_dec;
#define sym_alloc() ((struct dec_sym *)Dee_Malloc(sizeof(struct dec_sym)))
#define sym_free(x) Dee_Free(x)


/* Initialize/finalize the global DEC writer context. */
INTERN void DCALL dec_writer_init(void) {
	unsigned int i;
	/* ZERO-initialize everything. */
	memset(&current_dec, 0, sizeof(struct dec_writer));
	for (i = 0; i < DEC_SECTION_COUNT; ++i)
		current_dec.dw_sec_defl[i].ds_start.ds_sect = &current_dec.dw_sec_defl[i];
}

INTERN void DCALL dec_writer_fini(void) {
	struct dec_sym *sym_iter, *sym_next;
	struct dec_section *sec_iter;
	/* Free all DEC symbol that had been used. */
	sym_iter = current_dec.dw_symbols;
	while (sym_iter) {
		sym_next = sym_iter->ds_next;
		sym_free(sym_iter);
		sym_iter = sym_next;
	}
	/* Clear all sections and their associated data. */
	for (sec_iter = current_dec.dw_sec_defl;
	     sec_iter != COMPILER_ENDOF(current_dec.dw_sec_defl); ++sec_iter) {
		struct dec_section *iter, *next;
		Dee_Free(sec_iter->ds_begin);
		Dee_Free(sec_iter->ds_relv);
		iter = sec_iter->ds_next;
		while (iter) {
			next = iter->ds_next;
			Dee_Free(iter->ds_begin);
			Dee_Free(iter->ds_relv);
			Dee_Free(iter);
			iter = next;
		}
	}
}

/* Allocate `n_bytes' of data at the current
 * text position within the current section. */
INTERN uint8_t *DCALL dec_alloc(size_t n_bytes) {
	struct dec_section *sec = dec_curr;
	uint8_t *result         = sec->ds_iter;
	size_t req_size, new_size;
	if likely(result + n_bytes <= sec->ds_end)
		{
		/* Fast path: the section already has enough buffer memory allocated. */
		sec->ds_iter = result + n_bytes;
		goto end;
	}
	new_size = (size_t)(sec->ds_iter - sec->ds_begin);
	req_size = new_size + n_bytes;
	if unlikely(!new_size)
		new_size = 32;
	while (new_size < req_size)
		new_size *= 2;
do_realloc:
	/* Allocate more memory. */
	result = (uint8_t *)Dee_TryRealloc(sec->ds_begin, new_size);
	if unlikely(!result)
		{
		if (new_size != req_size) {
			new_size = req_size;
			goto do_realloc;
		}
		if (Dee_CollectMemory(new_size))
			goto do_realloc;
		return NULL;
	}
	sec->ds_iter  = result + (sec->ds_iter - sec->ds_begin);
	sec->ds_begin = result;
	sec->ds_end   = result + new_size;
	result        = sec->ds_iter;
	sec->ds_iter += n_bytes;
end:
	return result;
}

/* Search for an existing instance of `data...+=n_bytes' within the
 * current section and either return a pointer to it, or append the
 * given data block at its end and return a pointer to where it starts.
 * If the later case fails to re-allocate the section buffer, `NULL' is returned. */
INTERN uint8_t *DCALL
dec_allocstr(void const *__restrict data, size_t n_bytes) {
	struct dec_section *sec = dec_curr;
	uint8_t *result;
	/* Search for an existing instance that could be re-used. */
	result = (uint8_t *)memmem(sec->ds_begin,
	                           (size_t)(sec->ds_iter - sec->ds_begin),
	                           data, n_bytes);
	if (!result) {
		/* Append a new block of data if no existing one could be found. */
		result = dec_alloc(n_bytes);
		if likely(result)
			memcpy(result, data, n_bytes);
	}
	return result;
}

/* Take a look at the last-written `n_bytes' of data and search
 * all sections (that could reasonably contain a mirror copy) for
 * a copy of the last written `n_bytes' of data, as well as all
 * relocations that are accompanying that data.
 * If such a copy is found, the last written `n_bytes' of data
 * are deleted and the given `sym' is defined to point at the
 * existing shadow copy.
 * If such a copy could not be found or if the `DEC_WRITE_FREUSE_GLOBAL'
 * flag isn't set, `sym' is defined to point at the start of searched
 * data, located at `dec_curr:dec_addr-n_bytes'. */
INTERN void DCALL
dec_reuseglobal_define(struct dec_sym *__restrict sym, size_t n_bytes) {
	ASSERT(dec_addr >= n_bytes);
	if (current_dec.dw_flags & DEC_WRITE_FREUSE_GLOBAL) {
		DEC_FOREACH_SECTION_VARS;
		struct dec_section *sec;
		uint8_t *data = dec_ptr - n_bytes;
		/* Search for an existing instance of the `data+=n_bytes'. */
		DEC_FOREACH_SECTION(sec) {
			uint8_t *existing_data;
			existing_data = (uint8_t *)memmem(sec->ds_begin, (size_t)(sec->ds_iter - sec->ds_begin),
			                                  data, n_bytes);
			if (!existing_data)
				continue;
			if (existing_data == data)
				continue; /* Skip the one we're trying to delete. */
			/* We've actually managed to find something! */
			dec_curr->ds_iter -= n_bytes; /* Delete the last `n_bytes' bytes. */
			/* Initialize the symbol as pointing to the existing block of data. */
			sym->ds_sect = sec;
			sym->ds_addr = (uint32_t)(existing_data - sec->ds_begin);
			return;
		}
	}
	sym->ds_sect = dec_curr;
	sym->ds_addr = (uint32_t)(dec_addr - n_bytes);
}

INTERN uint8_t *DCALL dec_reuselocal(size_t n_bytes) {
	uint8_t *result;
	ASSERT(dec_addr >= n_bytes);
	result = (uint8_t *)memmem(dec_curr->ds_begin,
	                           (dec_curr->ds_iter - dec_curr->ds_begin) - n_bytes,
	                           dec_curr->ds_iter - n_bytes, n_bytes);
	if (result) {
		ASSERT(!n_bytes || result >= dec_curr->ds_begin);
		ASSERT(!n_bytes || result < (dec_curr->ds_iter - n_bytes));
		ASSERT(!n_bytes || result + n_bytes <= (dec_curr->ds_iter - n_bytes));
		/* Found an existing alternative. */
		dec_curr->ds_iter -= n_bytes;
	} else {
		/* No existing version was found.
		 * Just keep using the one created just now. */
		result = dec_curr->ds_iter - n_bytes;
	}
	return result;
}

/* Given some data encoded in host-endian, convert it to
 * little-endian and write it to the current text position
 * before advancing the text pointer. */
INTERN int (DCALL dec_putb)(uint8_t byte) {
	uint8_t *buf = dec_alloc(1);
	if unlikely(!buf)
		return -1;
	*buf = byte;
	return 0;
}

INTERN int (DCALL dec_putw)(uint16_t host_endian_word) {
	uint8_t *buf = dec_alloc(2);
	if unlikely(!buf)
		return -1;
	UNALIGNED_SETLE16((uint16_t *)buf, host_endian_word);
	return 0;
}

INTERN int (DCALL dec_putl)(uint32_t host_endian_dword) {
	uint8_t *buf = dec_alloc(4);
	if unlikely(!buf)
		return -1;
	UNALIGNED_SETLE32((uint32_t *)buf, host_endian_dword);
	return 0;
}

INTERN int (DCALL dec_putsleb)(int value) {
	uint8_t byte;
	unsigned int temp;
	byte = (unsigned int)value & 0x3f;
	if (value < 0) {
		value = -value;
		byte  = 0x40 | ((unsigned int)value & 0x3f);
	}
	temp = (unsigned int)value >> 6;
	for (;;) {
		if (temp)
			byte |= 0x80;
		if (dec_putb(byte))
			goto err;
		if (!temp)
			break;
		byte = temp & 0x7f;
		temp >>= 7;
	}
	return 0;
err:
	return -1;
}

INTERN int (DCALL dec_putptr)(uint32_t value) {
	uint8_t byte;
	unsigned int temp;
	byte = value & 0x7f;
	temp = value >> 7;
	for (;;) {
		if (temp)
			byte |= 0x80;
		if (dec_putb(byte))
			goto err;
		if (!temp)
			break;
		byte = temp & 0x7f;
		temp >>= 7;
	}
	return 0;
err:
	return -1;
}

INTERN int (DCALL dec_putieee754)(double value) {
	union {
		double value;
		uint64_t data;
	} buffer;
	uint8_t *dst;
	/* XXX: Special decoding when `double' doesn't conform to ieee754 */
	buffer.value = value;
	dst          = dec_alloc(8);
	if unlikely(!dst)
		return -1;
	UNALIGNED_SETLE64((uint64_t *)dst, buffer.data);
	return 0;
}


/* Create, insert and return a new section that will
 * appear after `sect' in the resulting DEC file. */
INTERN struct dec_section *DCALL
dec_newsection_after(struct dec_section *__restrict sect) {
	struct dec_section *result;
	ASSERT(sect);
	result = (struct dec_section *)Dee_Calloc(sizeof(struct dec_section));
	if unlikely(!result)
		goto done;
	result->ds_start.ds_sect = result;
	/* Insert the new section after the one given. */
	result->ds_next = sect->ds_next;
	sect->ds_next   = result;
done:
	return result;
}

/* Allocate a new, undefined DEC symbol.
 * NOTE: Should the symbol appear in at least one relocation,
 *       then it must also be defined at some point.
 *       There is no unresolved-relocation error here.
 *       If you forget to define it, `dec_link' will crash
 *       with an assertion error. */
INTERN struct dec_sym *DCALL dec_newsym(void) {
	struct dec_sym *result = sym_alloc();
	if likely(result)
		{
		/* Keep track of the newly allocated symbol. */
		result->ds_next        = current_dec.dw_symbols;
		current_dec.dw_symbols = result;
		result->ds_sect        = NULL;
	}
	return result;
}


/* Allocate a new, uninitialized relocation at the end of the current section.
 * The caller is responsible to ensure that the relocation will be defined
 * after the location of the previously allocated relocation.
 * WARNING: Multiple successive calls to this function
 *          may invalid previously returned pointers. */
INTERN struct dec_rel *DCALL dec_newrel(void) {
	struct dec_section *sec = dec_curr;
	struct dec_rel *result  = sec->ds_relv;
	ASSERT(sec->ds_relc <= sec->ds_rela);
	if (sec->ds_relc == sec->ds_rela) {
		size_t new_alloc = sec->ds_rela * 2;
		if (!new_alloc)
			new_alloc = 2;
do_realloc:
		result = (struct dec_rel *)Dee_TryRealloc(sec->ds_relv, new_alloc *
		                                                        sizeof(struct dec_rel));
		if unlikely(!result)
			{
			if (new_alloc != sec->ds_relc + 1) {
				new_alloc = sec->ds_relc + 1;
				goto do_realloc;
			}
			if (Dee_CollectMemory(new_alloc * sizeof(struct dec_rel)))
				goto do_realloc;
			goto done;
		}
		sec->ds_relv = result;
		sec->ds_rela = new_alloc;
	}
	/* Reserve one relocation. */
	result += sec->ds_relc++;
#ifndef NDEBUG
	memset(result, 0xcc, sizeof(struct dec_rel));
#endif /* !NDEBUG */
done:
	return result;
}

/* Create a new relocation `type' against `sym' at the
 * current text address within the current section.
 * The caller must ensure that `sym' be only `NULL'
 * when the relocation associated with `type' doesn't
 * make use of a symbol. */
INTERN int (DCALL dec_putrel)(uint8_t type, struct dec_sym *sym) {
	struct dec_rel *rel = dec_newrel();
	if unlikely(!rel)
		return -1;
	ASSERT(rel == dec_curr->ds_relv ||
	       rel[-1].dr_addr <= dec_addr);
	rel->dr_sym  = sym;
	rel->dr_addr = dec_addr;
	rel->dr_type = type;
	return 0;
}

INTERN int (DCALL dec_putrelat)(uint32_t addr, uint8_t type,
                               struct dec_sym *sym) {
	struct dec_rel *rel = dec_newrel();
	if unlikely(!rel)
		return -1;
	ASSERT(rel == dec_curr->ds_relv ||
	       rel[-1].dr_addr <= addr);
	rel->dr_sym  = sym;
	rel->dr_addr = addr;
	rel->dr_type = type;
	return 0;
}

PRIVATE bool DCALL
dec_section_empty(struct dec_section *__restrict self) {
	do {
		if (self->ds_iter != self->ds_begin)
			return false;
	} while ((self = self->ds_next) != NULL);
	return true;
}

/* @return: 0: Successfully linked all DEC sections.
 * @return: *: One of `DECREL_*' that was truncated, causing linking to fail. */
INTERN uint8_t(DCALL dec_link)(void) {
	DEC_FOREACH_SECTION_VARS;
	struct dec_section *sec;
	struct dec_rel *iter, *end;
	DEC_FOREACH_SECTION(sec) {
		end = (iter = sec->ds_relv) + sec->ds_relc;
		for (; iter != end; ++iter) {
			struct dec_sym *relsym = iter->dr_sym;
			uint8_t *target        = sec->ds_begin + iter->dr_addr;
			uint32_t relval;
			ASSERT(iter->dr_type == DECREL_NONE ||
			       (iter->dr_sym && DEC_SYM_DEFINED(iter->dr_sym)));
			ASSERT(iter->dr_type == DECREL_NONE ||
			       iter->dr_addr < (size_t)(sec->ds_iter - sec->ds_begin));
			switch (iter->dr_type) {

			case DECREL_ABS16_NULL:
				if (dec_section_empty(relsym->ds_sect)) {
					UNALIGNED_SET16((uint16_t *)target, 0);
					break;
				}
				ATTR_FALLTHROUGH
			case DECREL_ABS16:
				relval = UNALIGNED_GETLE16((uint16_t *)target) + (relsym->ds_addr +
				                                                  relsym->ds_sect->ds_base);
				if unlikely(relval > UINT16_MAX)
					goto rel_trunc;
				UNALIGNED_SETLE16((uint16_t *)target, (uint16_t)relval);
				break;

			case DECREL_ABS32_NULL:
				if (dec_section_empty(relsym->ds_sect)) {
					UNALIGNED_SET32((uint32_t *)target, 0);
					break;
				}
				ATTR_FALLTHROUGH
			case DECREL_ABS32:
				UNALIGNED_SETLE32((uint32_t *)target,
				                  UNALIGNED_GETLE32((uint32_t *)target) +
				                  (relsym->ds_addr + relsym->ds_sect->ds_base));
				break;

			case DECREL_SIZE32:
				UNALIGNED_SETLE32((uint32_t *)target,
				                  UNALIGNED_GETLE32((uint32_t *)target) +
				                  (uint32_t)(relsym->ds_sect->ds_iter - relsym->ds_sect->ds_begin));
				break;

				/* case DECREL_NONE: // Always skip empty relocations. */

			default: break;
			}
		}
	}
	return DECREL_NONE;
rel_trunc:
	return iter->dr_type;
}


/* Assign base addresses to all allocated sections. */
INTERN void DCALL dec_setbases(void) {
	DEC_FOREACH_SECTION_VARS;
	struct dec_section *sec;
	uint32_t base = 0;
	DEC_FOREACH_SECTION(sec) {
		/* Set the section's base address. */
		sec->ds_base = base;
		/* Simply advance the base address by the size of the section. */
		base += (uint32_t)(sec->ds_iter - sec->ds_begin);
	}
}


/* Write all sections (in order) to the given `file_stream'.
 * For this purpose, file write operations from <deemon/file.h> are
 * using, meaning that `file_stream' should be derived from `DeeFile_Type'.
 * @return:  0: Successfully written the DEC file.
 * @return: -1: An error occurred while writing. */
INTERN int(DCALL dec_write)(DeeObject *__restrict file_stream) {
	DEC_FOREACH_SECTION_VARS;
	struct dec_section *sec;
	DEC_FOREACH_SECTION(sec) {
		dssize_t temp;
		temp = DeeFile_WriteAll(file_stream, sec->ds_begin,
		                        (size_t)(sec->ds_iter - sec->ds_begin));
		if unlikely(temp < 0)
			goto err;
	}
	return 0;
err:
	return -1;
}


#if !defined(NDEBUG) && 0
PRIVATE char const section_names[DEC_SECTION_COUNT][12] = {
	/* [DEC_SECTION_HEADER    ] = */ "header    ",
	/* [DEC_SECTION_IMPORTS   ] = */ "imports   ",
	/* [DEC_SECTION_DEPS      ] = */ "deps      ",
	/* [DEC_SECTION_GLOBALS   ] = */ "globals   ",
	/* [DEC_SECTION_ROOT      ] = */ "root      ",
	/* [DEC_SECTION_TEXT      ] = */ "text      ",
	/* [DEC_SECTION_DEBUG     ] = */ "debug     ",
	/* [DEC_SECTION_DEBUG_DATA] = */ "debug_data",
	/* [DEC_SECTION_DEBUG_TEXT] = */ "debug_text",
	/* [DEC_SECTION_STRING    ] = */ "string    "
};

PRIVATE size_t DCALL
sections_size(struct dec_section *start) {
	size_t result = 0;
	for (; start; start = start->ds_next) {
		result += (size_t)(start->ds_iter - start->ds_begin);
	}
	return result;
}

PRIVATE unsigned int DCALL
sections_subsec(struct dec_section *start) {
	unsigned int result = 0;
	for (; start; start = start->ds_next)
		++result;
	return result;
}

/* Print usage information in DEC sections. */
PRIVATE void (DCALL dec_print_usage)(DeeModuleObject *__restrict module) {
#define PRINTF(...)                                                 \
	do {                                                            \
		if (DeeFile_Printf(DeeFile_DefaultStddbg, __VA_ARGS__) < 0) \
			goto err;                                               \
	} __WHILE0
	unsigned int root_id;
	size_t total_size = 0;
	for (root_id = 0; root_id < DEC_SECTION_COUNT; ++root_id)
		total_size += sections_size(&current_dec.dw_sec_defl[root_id]);
	PRINTF("module %k (%k), %Iu bytes:\n",
	       module->mo_name, module->mo_path, total_size);
	for (root_id = 0; root_id < DEC_SECTION_COUNT; ++root_id) {
		struct dec_section *sec = &current_dec.dw_sec_defl[root_id];
		size_t sec_size         = sections_size(sec);
		PRINTF("\t.%s: %u%%, %Iu bytes, %u subsections\n",
		       section_names[root_id], (unsigned int)((sec_size * 100 + (total_size - 1)) / total_size),
		       sec_size, sections_subsec(sec));
	}
	for (root_id = 0; root_id < DEC_SECTION_COUNT; ++root_id) {
		struct dec_section *sec = &current_dec.dw_sec_defl[root_id];
		unsigned int secnum     = 0;
		for (;;) {
			size_t sec_size = (size_t)(sec->ds_iter - sec->ds_begin);
			PRINTF("\t.%s.%u: %u%%, %Iu bytes\n",
			       section_names[root_id], secnum,
			       (unsigned int)((sec_size * 100 + (total_size - 1)) / total_size), sec_size);
			if ((sec = sec->ds_next) == NULL)
				break;
			++secnum;
		}
	}
#undef PRINTF
	return;
err:
	DeeError_Handled(ERROR_HANDLED_RESTORE);
}
#else
#define dec_print_usage(module) (void)0
#endif


INTERN int (DCALL dec_generate_and_link)(DeeModuleObject *__restrict module) {
	int result;
again:
	/* Generate DEC data for the given module. */
	result = dec_generate(module);
	if unlikely(result)
		goto done;

	/* Set base addresses for all DEC sections. */
	dec_setbases();

	/* Link all DEC data. */
	if unlikely(dec_link() != DECREL_NONE)
		{
		/* Relocation was truncated. - Set the `DEC_WRITE_FBIGFILE' flag and try again. */
		if unlikely(current_dec.dw_flags & DEC_WRITE_FBIGFILE)
			{
			/* This is bad... (and shouldn't actually happen!) */
			DeeError_Throwf(&DeeError_NotImplemented,
			                "DEC relocation was truncated");
			result = -1;
			goto done;
		}
		/* Re-initialize the current DEC context and set the FBIGFILE flag. */
		dec_writer_fini();
		dec_writer_init();
		current_dec.dw_flags |= DEC_WRITE_FBIGFILE;
		goto again;
	}
	dec_print_usage(module);
done:
	return result;
}

#if 0
INTERN int (DCALL dec_createfp)(DeeModuleObject *__restrict module,
                                DeeObject *__restrict file_stream) {
	struct dec_writer old_dec;
	int result;
	/* Save the previously active DEC context. */
	memcpy(&old_dec, &current_dec, sizeof(struct dec_writer));
	/* Initialize a new DEC context. */
	dec_writer_init();

	result = dec_generate_and_link(module);
	if unlikely(result)
		goto done;

	/* Write all DEC data to the provided file stream. */
	result = dec_write(file_stream);

done:
	/* Cleanup... */
	dec_writer_fini();
	memcpy(&current_dec, &old_dec, sizeof(struct dec_writer));
	return result;
}
#endif

#ifdef CONFIG_HOST_WINDOWS
#define SEP '\\'
#else /* CONFIG_HOST_WINDOWS */
#define SEP '/'
#endif /* !CONFIG_HOST_WINDOWS */

#ifdef CONFIG_LITTLE_ENDIAN
#define ENCODE4(a, b, c, d) ((d) << 24 | (c) << 16 | (b) << 8 | (a))
#else /* CONFIG_LITTLE_ENDIAN */
#define ENCODE4(a, b, c, d) ((d) | (c) << 8 | (b) << 16 | (a) << 24)
#endif /* !CONFIG_LITTLE_ENDIAN */


/* Create and emit a DEC file for the given module. */
INTERN int (DCALL dec_create)(DeeModuleObject *__restrict module) {
	int result;
	struct dec_writer old_dec;
	DREF DeeObject *dec_filename;
	DeeObject *config_filename;
	/* Save the previously active DEC context. */

	memcpy(&old_dec, &current_dec, sizeof(struct dec_writer));
	/* Initialize a new DEC context. */
	dec_writer_init();

	/* Load dec writer options from active compiler options. */
	ASSERT(DeeCompiler_Current);
	config_filename = NULL;
	if (DeeCompiler_Current->cp_options) {
		current_dec.dw_flags = DeeCompiler_Current->cp_options->co_decwriter;
		config_filename      = DeeCompiler_Current->cp_options->co_decoutput;
	}

	/* Generate and link DEC data for the given module. */
	result = dec_generate_and_link(module);
	if unlikely(result)
		goto done;

	/* Create the DEC output file only after we've generated
	 * all dec data, so we don't have to concern ourselves
	 * with deletion of the output file is generation fails. */
	if ((dec_filename = config_filename) == NULL) {
		char *dec_filestr  = DeeString_STR(module->mo_path);
		size_t dec_filelen = DeeString_SIZE(module->mo_path);
		/* Generate the filename of the DEC file to-be created. */
		for (;;) {
			if (!dec_filelen)
				goto cannot_create;
			if (dec_filestr[dec_filelen - 1] == '.')
				break;
			if (dec_filestr[dec_filelen - 1] == SEP)
				goto cannot_create;
			--dec_filelen;
		}
		dec_filename = DeeString_NewBuffer(dec_filelen + 4);
		if unlikely(!dec_filename)
			goto err;
		{
			size_t pathlen;
			char *dst, *dec_filestart;
			dec_filestart = (char *)memrchr(dec_filestr, SEP, dec_filelen);
			if (dec_filestart)
				++dec_filestart;
			else
				dec_filestart = dec_filestr;
			pathlen = (size_t)(dec_filestart - dec_filestr);
			dst     = DeeString_STR(dec_filename);
			memcpy(dst, dec_filestr, pathlen * sizeof(char));
			dst += pathlen, *dst++ = '.';
			pathlen = (size_t)((dec_filestr + dec_filelen) - dec_filestart);
			memcpy(dst, dec_filestart, pathlen * sizeof(char));
			dst += pathlen;
			UNALIGNED_SET32((uint32_t *)dst, ENCODE4('d', 'e', 'c', 0));
		}
	} else {
		Dee_Incref(dec_filename);
	}
	DEE_DPRINTF("DECGEN: %r -> %r\n", module->mo_path, dec_filename);
	ASSERT_OBJECT_TYPE_EXACT(dec_filename, &DeeString_Type);

	if (DeeString_Check(dec_filename)) {
		DREF DeeObject *output_fp;
		ASSERT_OBJECT_TYPE_EXACT(dec_filename, &DeeString_Type);
		output_fp = DeeFile_Open(dec_filename,
		                         OPEN_FWRONLY | OPEN_FCREAT |
		                         OPEN_FTRUNC | OPEN_FHIDDEN,
		                         0644);
		ASSERT_OBJECT_TYPE_EXACT(dec_filename, &DeeString_Type);
		if unlikely(!output_fp)
			goto err_filename;
		/* Write all DEC data to the provided file stream. */
		ASSERT_OBJECT_TYPE_EXACT(dec_filename, &DeeString_Type);
		result = dec_write(output_fp);
		ASSERT_OBJECT_TYPE_EXACT(dec_filename, &DeeString_Type);
		Dee_Decref(output_fp);
		ASSERT_OBJECT_TYPE_EXACT(dec_filename, &DeeString_Type);

		if (result) {
			/* Delete the DEC file, considering that we've failed to do write to it. */
#ifdef CONFIG_HOST_WINDOWS
			LPWSTR wname = (LPWSTR)DeeString_AsWide(dec_filename);
			if likely(wname)
				DeleteFileW(wname);
			else {
				DeleteFileA(DeeString_STR(dec_filename));
				if (!DeeError_Handled(ERROR_HANDLED_NORMAL))
					goto err_filename;
			}
#else /* CONFIG_HOST_WINDOWS */
			unlink(DeeString_STR(dec_filename));
#endif /* !CONFIG_HOST_WINDOWS */
		}
		ASSERT_OBJECT_TYPE_EXACT(dec_filename, &DeeString_Type);
	} else {
		/* Assume that `dec_filename' is a stream-output */
		ASSERT_OBJECT_TYPE_EXACT(dec_filename, &DeeString_Type);
		result = dec_write(dec_filename);
		ASSERT_OBJECT_TYPE_EXACT(dec_filename, &DeeString_Type);
	}
	ASSERT_OBJECT_TYPE_EXACT(dec_filename, &DeeString_Type);
	Dee_Decref(dec_filename);

done:
	/* Cleanup... */
	dec_writer_fini();
	memcpy(&current_dec, &old_dec, sizeof(struct dec_writer));
	return result;
err_filename:
	Dee_Decref(dec_filename);
err:
	result = -1;
	goto done;
cannot_create:
	result = 0;
	goto done;
}


DECL_END

#endif /* !CONFIG_NO_DEC */

#endif /* !GUARD_DEEMON_COMPILER_DEC_C */
