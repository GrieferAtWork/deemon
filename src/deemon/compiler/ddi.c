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
#ifndef GUARD_DEEMON_COMPILER_DDI_C
#define GUARD_DEEMON_COMPILER_DDI_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/symbol.h>
#include <deemon/error.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* qsort(), bzero(), ... */
#include <deemon/system.h>          /* DeeSystem_BaseName() */
#include <deemon/util/bytewriter.h>

#include <hybrid/byteswap.h>
#include <hybrid/typecore.h>
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* intN_t, uintN_t, uintptr_t */

DECL_BEGIN

#ifndef CONFIG_HAVE_qsort
#define CONFIG_HAVE_qsort
#define qsort   dee_qsort
DeeSystem_DEFINE_qsort(dee_qsort)
#endif /* !CONFIG_HAVE_qsort */



#define ddi current_assembler.a_ddi
/* Replace all symbols in checkpoints with load addresses. */
PRIVATE void DCALL ddi_load_symbols(void) {
	struct ddi_checkpoint *iter, *end;
	end = (iter = ddi.da_checkv) + ddi.da_checkc;
	for (; iter < end; ++iter) {
		ASSERT(iter->dc_sym);
		ASSERT(ASM_SYM_DEFINED(iter->dc_sym));
		/* NOTE: We disregard the fact that we're still holding
		 *       a reference to `iter->dc_sym->as_used', simply
		 *       because at this point those use counters no
		 *       longer matter. */
		iter->dc_addr = iter->dc_sym->as_addr;
	}
}


#ifndef __LIBCCALL
#define __LIBCCALL /* nothing */
#endif /* !__LIBCCALL */

PRIVATE WUNUSED int __LIBCCALL
checkpoint_compare(void const *a, void const *b) {
	struct ddi_checkpoint *lhs;
	struct ddi_checkpoint *rhs;
	lhs = (struct ddi_checkpoint *)a;
	rhs = (struct ddi_checkpoint *)b;
#if __SIZEOF_INT__ > 4
	return ((int)lhs->dc_addr - (int)rhs->dc_addr);
#else /* __SIZEOF_INT__ > 4 */
	if (lhs->dc_addr < rhs->dc_addr)
		return -1;
	if (lhs->dc_addr > rhs->dc_addr)
		return 1;
	return 0;
#endif /* __SIZEOF_INT__ < 4 */
}

struct ddi_gen_state {
	code_addr_t     reg_uip;  /* The current user instruction. */
	uint16_t        reg_usp;  /* The current stack alignment/depth. */
	uint16_t        reg_path; /* The current path number. */
	uint16_t        reg_file; /* The current file number. */
	uint16_t        reg_name; /* The current function name. */
	int             reg_col;  /* The current column number within the active line. */
	int             reg_lno;  /* Line number (0-based). */
	struct TPPFile *tpp_file; /* [0..1] The current TPP file object. */
};

PRIVATE WUNUSED NONNULL((1, 2)) int32_t DCALL
find_or_alloc_offset(uint32_t **__restrict p_vector,
                     uint32_t *__restrict p_size,
                     uint32_t offset_value,
                     uint32_t max_size) {
	/* Search for `offset_value' in the given vector and return its index.
	 * If not found, extend the vector by adding to the end, but don't
	 * exceed a size of `max_size'.
	 * If the size limit cannot be sustained, raise a Compiler Error.
	 * Upon failure, return -1. Upon success return the index of `offset_value' */
	uint32_t *vector = *p_vector, *iter, *end;
	uint32_t size    = *p_size;
	end              = (iter = vector) + size;
	for (; iter < end; ++iter) {
		if (*iter == offset_value)
			return (int32_t)(iter - vector);
	}
	/* Ensure that we're not already at the limit. */
	if unlikely(size == max_size) {
		DeeError_Throwf(&DeeError_CompilerError,
		                "Too many paths/files for DDI");
		goto err;
	}
	/* Append a new value. */
	vector = (uint32_t *)Dee_Reallocc(vector, size + 1, sizeof(uint32_t));
	if unlikely(!vector)
		goto err;
	vector[size] = offset_value;
	*p_vector    = vector;
	*p_size      = size + 1;
	return size;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) uint8_t *DCALL
put_sleb(uint8_t *__restrict text, int value) {
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
		*text++ = byte;
		if (!temp)
			break;
		byte = temp & 0x7f;
		temp >>= 7;
	}
	return text;
}

PRIVATE WUNUSED NONNULL((1)) uint8_t *DCALL
put_uleb(uint8_t *__restrict text, unsigned int value) {
	uint8_t byte;
	unsigned int temp;
	byte = value & 0x7f;
	temp = value >> 7;
	for (;;) {
		if (temp)
			byte |= 0x80;
		*text++ = byte;
		if (!temp)
			break;
		byte = temp & 0x7f;
		temp >>= 7;
	}
	return text;
}


PRIVATE WUNUSED NONNULL((1)) uint8_t *DCALL
usp_transition(uint8_t *__restrict text,
               uint16_t old_usp,
               uint16_t new_usp) {
	int32_t usp_diff;
	usp_diff = ((int32_t)new_usp - (int32_t)old_usp);
	if (usp_diff == +1) {
		*text++ = DDI_INCUSP;
	} else if (usp_diff == -1) {
		*text++ = DDI_DECUSP;
	} else if (usp_diff) {
		*text++ = DDI_ADDUSP;
		text    = put_sleb(text, usp_diff);
	}
	return text;
}

/* Generate DDI assembly to transition from `old_state' to `new_state' and write to `*test'.
 * This function assumes that sufficient memory is available inside `*text' to write any
 * potential transition that may exist.
 * NOTE: The caller is required to ensure that the new state's UIP is > the old state's. */
PRIVATE WUNUSED NONNULL((1, 2, 3)) uint8_t *DCALL
ddi_transition(uint8_t *__restrict text,
               struct ddi_gen_state *__restrict old_state,
               struct ddi_gen_state *__restrict new_state) {
	code_addr_t uip_diff;
	int lno_diff;
	ASSERT(new_state->reg_uip > old_state->reg_uip);
	if (old_state->reg_path != new_state->reg_path) {
		/* Update the path. */
		*text++ = DDI_SETPATH;
		text    = put_uleb(text, new_state->reg_path);
	}
	if (old_state->reg_file != new_state->reg_file) {
		/* Update the file. */
		*text++ = DDI_SETFILE;
		text    = put_uleb(text, new_state->reg_file);
	}
	if (old_state->reg_name != new_state->reg_name) {
		/* Update the function name. */
		*text++ = DDI_SETNAME;
		text    = put_uleb(text, new_state->reg_name);
	}
	if (old_state->reg_col != new_state->reg_col) {
		/* Update the column number. */
		*text++ = DDI_SETCOL;
		text    = put_sleb(text, new_state->reg_col);
	}
	uip_diff = (new_state->reg_uip - old_state->reg_uip);
	lno_diff = (new_state->reg_lno - old_state->reg_lno);
	/* Adjust for a different stack alignment. */
	text = usp_transition(text, old_state->reg_usp, new_state->reg_usp);
	if (lno_diff >= 0 &&
	    lno_diff < DDI_GENERIC_LN(DDI_MAXSHIFT) &&
	    uip_diff <= DDI_GENERIC_IP(DDI_MAXSHIFT)) {
		/* Simple case: can encoded the shift as a single instruction. */
		*text++ = (uint8_t)DDI_GENERIC(uip_diff, lno_diff);
	} else if (uip_diff <= DDI_GENERIC_IP(DDI_MAXSHIFT)) {
		/* We can still encode the address shift using a single instruction. */
		*text++ = DDI_ADDLNO;
		text    = put_sleb(text, lno_diff);
		*text++ = (uint8_t)DDI_GENERIC(uip_diff, 0);
	} else {
		/* Fallback: Encode using 2 separate instructions. */
		*text++ = DDI_ADDLNO;
		text    = put_sleb(text, lno_diff);
		*text++ = DDI_ADDUIP;
		text    = put_uleb(text, uip_diff - 1);
	}
	return text;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
xddi_putsymbol(struct bytewriter *__restrict writer,
               uint8_t symbol_class,
               uint16_t symbol_id,
               uint32_t name_offset) {
	uint8_t length;
	if (name_offset > UINT16_MAX) {
		length = DDI_EXDAT_OP32;
	} else if (name_offset > UINT8_MAX || symbol_id > UINT8_MAX) {
		length = DDI_EXDAT_OP16;
	} else {
		length = DDI_EXDAT_OP8;
	}
	if (bytewriter_putb(writer, length | symbol_class))
		goto err;
	switch (length) {

	case DDI_EXDAT_OP8:
		if (bytewriter_putb(writer, (uint8_t)symbol_id))
			goto err;
		if (bytewriter_putb(writer, (uint8_t)name_offset))
			goto err;
		break;

	case DDI_EXDAT_OP16:
		if (bytewriter_putw(writer, HTOLE16(symbol_id)))
			goto err;
		if (bytewriter_putw(writer, HTOLE16((uint16_t)name_offset)))
			goto err;
		break;

	case DDI_EXDAT_OP32:
		if (bytewriter_putw(writer, HTOLE16(symbol_id)))
			goto err;
		if (bytewriter_putl(writer, HTOLE32(name_offset)))
			goto err;
		break;

	default: __builtin_unreachable();
	}
	return 0;
err:
	return -1;
}


INTERN WUNUSED DREF DeeDDIObject *DCALL ddi_compile(void) {
	DeeDDIObject *result;
	size_t result_size;
	uint8_t *code_iter;
	struct ascii_printer strtab;
	/* Check for simple case: no DDI information needs to be generated. */
	if (current_assembler.a_flag & ASM_FNODDI)
		return_reference_(&DeeDDI_Empty);
	/* Start out with a buffer that should already be sufficient for most cases. */
	result_size = (current_assembler.a_sect[0].sec_iter -
	               current_assembler.a_sect[0].sec_begin) *
	              3;
	result = (DeeDDIObject *)DeeObject_TryCallocc(offsetof(DeeDDIObject, d_ddi),
	                                              result_size + 1, sizeof(uint8_t));
	if unlikely(!result) {
		result_size = 0;
		result = (DeeDDIObject *)DeeObject_Callocc(offsetof(DeeDDIObject, d_ddi),
		                                           result_size + 1, sizeof(uint8_t));
		if unlikely(!result)
			goto err;
	}
	/* Initialize the string printer we're using for the ddi's string table. */
	ascii_printer_init(&strtab);
	code_iter = result->d_ddi;

	/* As the first step, let's replace all checkpoint
	 * symbols with their proper addresses, just so we
	 * can work a little bit easier. */
	ddi_load_symbols();

	/* With all symbols loaded, let's sort them by address.
	 * This should be pretty fast because it's most likely
	 * that the DDI checkpoint list is already sorted. */
	qsort(ddi.da_checkv, ddi.da_checkc,
	      sizeof(struct ddi_checkpoint),
	      &checkpoint_compare);

	ASSERT(current_basescope != NULL);
	if (current_basescope->bs_name) {
		/* Allocate the name of the current function. */
		ASSERT(strtab.ap_length == 0);
		if (ascii_printer_print(&strtab, current_basescope->bs_name->k_name,
		                        current_basescope->bs_name->k_size + 1) < 0)
			goto err_result_printer;
		/* Link the initial symbol name for the main function. */
		result->d_strings = (uint32_t *)Dee_Malloc(sizeof(uint32_t));
		if unlikely(!result->d_strings)
			goto err_result_printer;
		result->d_nstring                  = 1;
		((uint32_t *)result->d_strings)[0] = 0;
		/* No need to set this field. - The name has
		 * already been set to point at offset ZERO(0). */
		ASSERT(result->d_start.dr_name == 0);
	}

	{
		struct ddi_checkpoint *iter, *end;
		/* NOTE: A DDI text buffer of `64' just has to
		 *       be enough for any possible transition. */
		uint8_t buffer[64], *text;
		size_t text_size;
		struct ddi_gen_state old_state, new_state;
		bool did_last, is_first;
		bzero(&old_state, sizeof(struct ddi_gen_state));
		end      = (iter = ddi.da_checkv) + ddi.da_checkc;
		did_last = false, is_first = true;
		for (; iter < end; ++iter) {
			/* If the UIP didn't change, don't do anything. */
			if (iter->dc_addr == old_state.reg_uip && !is_first)
				continue;
			text = buffer;
			ASSERT(iter->dc_loc.l_file);
			/* Setup the new state. */
			new_state.tpp_file = iter->dc_loc.l_file;
			new_state.reg_uip  = iter->dc_addr;
			new_state.reg_usp  = iter->dc_sp;
			new_state.reg_col  = iter->dc_loc.l_col;
			new_state.reg_lno  = iter->dc_loc.l_line;
			new_state.reg_name = old_state.reg_name; /* XXX: This needs to change for inline functions... */

			new_state.reg_path = old_state.reg_path;
			new_state.reg_file = old_state.reg_file;
			if (new_state.tpp_file != old_state.tpp_file) {
				/* The source file has changed and we must
				 * allocate the new one's path & file. */
				char *filename;
				size_t length;
				char *file_begin;
				uint32_t path_offset, file_offset;
				int32_t temp;
				filename   = (char *)TPPFile_Filename(new_state.tpp_file, &length);
				file_begin = (char *)DeeSystem_BaseName(filename, length);
				if (file_begin > filename) {
					char *tab_str, backup;
					tab_str = ascii_printer_allocstr(&strtab, file_begin,
					                                 (size_t)(length - (file_begin - filename)) + 1);
					if unlikely(!tab_str)
						goto err_result_printer;
					file_offset = (uint32_t)(tab_str - strtab.ap_string->s_str);
					/* Now to allocate the path. */
					backup         = file_begin[-1];
					file_begin[-1] = '\0'; /* TPP allocates these dynamically to we can cheat a bit... */
					tab_str = ascii_printer_allocstr(&strtab, filename,
					                                 (size_t)(file_begin - filename));
					file_begin[-1] = backup;
					if unlikely(!tab_str)
						goto err_result_printer;
					path_offset = (uint32_t)(tab_str - strtab.ap_string->s_str);
					temp = find_or_alloc_offset((uint32_t **)&result->d_strings,
					                            &result->d_nstring,
					                            path_offset, UINT32_MAX - 1);
					if unlikely(temp < 0)
						goto err_result_printer;
					new_state.reg_path = (uint16_t)(temp + 1);
				} else {
					/* Special case: The filename has no path associated. */
					if (length)
						++length;
					filename = ascii_printer_allocstr(&strtab, filename, length);
					if unlikely(!filename)
						goto err_result_printer;
					file_offset        = (uint32_t)(filename - strtab.ap_string->s_str);
					new_state.reg_path = 0; /* No path. */
				}
				/* Allocate an entry for the file name. */
				temp = find_or_alloc_offset((uint32_t **)&result->d_strings,
				                            &result->d_nstring,
				                            file_offset, UINT32_MAX);
				if unlikely(temp < 0)
					goto err_result_printer;
				new_state.reg_file = (uint16_t)temp;
			}
			/* Generate DDI assembly to update symbol name bindings. */
			{
				uint16_t i;
				struct ddi_binding *vec = iter->dc_bndv;
				for (i = 0; i < iter->dc_bndc; ++i) {
					uint8_t bind_buffer[64], *bind_text = bind_buffer;
					struct ddi_binding *binding = &vec[i];
					size_t bind_size;
					char *symbol_name_str;
					int32_t symbol_name_id;
					if (binding->db_class == DDI_BINDING_CLASS_LOCAL) {
						/* Local-binding */
						ASSERT(binding->db_index < current_assembler.a_localc);
						if (binding->db_name) {
							symbol_name_str = ascii_printer_allocstr(&strtab,
							                                         binding->db_name->k_name,
							                                         binding->db_name->k_size + 1);
							if unlikely(!symbol_name_str)
								goto err_result_printer;
							/* Allocate an entry for the symbol name. */
							symbol_name_id = find_or_alloc_offset((uint32_t **)&result->d_strings,
							                                      &result->d_nstring,
							                                      (uint32_t)(symbol_name_str -
							                                                 strtab.ap_string->s_str),
							                                      UINT32_MAX);
							if unlikely(symbol_name_id < 0)
								goto err_result_printer;
							*bind_text++ = DDI_DEFLCNAME;
							bind_text    = put_uleb(bind_text, binding->db_index);
							bind_text    = put_uleb(bind_text, (uint16_t)(uint32_t)symbol_name_id);
						} else {
							*bind_text++ = DDI_DELLCNAME;
							bind_text    = put_uleb(bind_text, binding->db_index);
						}
					} else {
						/* Stack-binding */
						if (binding->db_name) {
							symbol_name_str = ascii_printer_allocstr(&strtab,
							                                     binding->db_name->k_name,
							                                     binding->db_name->k_size + 1);
							if unlikely(!symbol_name_str)
								goto err_result_printer;
							/* Allocate an entry for the symbol name. */
							symbol_name_id = find_or_alloc_offset((uint32_t **)&result->d_strings,
							                                      &result->d_nstring,
							                                      (uint32_t)(symbol_name_str -
							                                                 strtab.ap_string->s_str),
							                                      UINT32_MAX);
							if unlikely(symbol_name_id < 0)
								goto err_result_printer;
							/* Optimize to make use of the `DDI_DEFSPNAME' instruction. */
							if (old_state.reg_usp == binding->db_index + 1) {
								*bind_text++ = DDI_DEFSPNAME;
							} else if (new_state.reg_usp == binding->db_index + 1) {
								bind_text         = usp_transition(bind_text, old_state.reg_usp, new_state.reg_usp);
								old_state.reg_usp = new_state.reg_usp;
								*bind_text++      = DDI_DEFSPNAME;
							} else {
								/* Fallback: Encode using the `DDI_DEFLCNAME' instruction. */
								*bind_text++ = DDI_DEFLCNAME;
								bind_text = put_uleb(bind_text,
								                     current_assembler.a_localc +
								                     binding->db_index);
							}
							bind_text = put_uleb(bind_text, (uint16_t)(uint32_t)symbol_name_id);
						} else if (new_state.reg_usp > binding->db_index) {
							*bind_text++ = DDI_DELLCNAME;
							bind_text = put_uleb(bind_text,
							                     current_assembler.a_localc +
							                     binding->db_index);
						}
					}
					/* Append the new text to the resulting DDI object. */
					bind_size = (size_t)(bind_text - bind_buffer);
					if (!bind_size)
						continue;
					/* NOTE: With our very generous preallocation above,
					 *       we're unlikely to have to allocate even more... */
					while unlikely(bind_size > result_size) {
						/* Must allocate more buffer memory. */
						size_t old_alloc = result_size + (size_t)(code_iter - result->d_ddi);
						size_t new_alloc = old_alloc * 2;
						DeeDDIObject *new_result;
do_realloc_bind:
						ASSERT(old_alloc != 0);
						ASSERT(new_alloc != 0);
						ASSERT(old_alloc != new_alloc);
						new_result = (DeeDDIObject *)DeeObject_TryReallocc(result, offsetof(DeeDDIObject, d_ddi),
							                                               new_alloc + 1, sizeof(uint8_t));
						if unlikely(!new_result) {
							size_t min_alloc = bind_size + (size_t)(code_iter - result->d_ddi);
							if (new_alloc != min_alloc) {
								new_alloc = min_alloc;
								goto do_realloc_bind;
							}
							if (Dee_CollectMemoryoc(offsetof(DeeDDIObject, d_ddi),
								                    new_alloc + 1, sizeof(uint8_t)))
								goto do_realloc_bind;
							goto err_result_printer;
						}
						/* Relocate the code pointer into the new result buffer. */
						code_iter -= (uintptr_t)result;
						code_iter += (uintptr_t)new_result;
						/* Update the result buffer and its remaining size. */
						result      = new_result;
						result_size = new_alloc - (size_t)(code_iter - result->d_ddi);
					}
					code_iter = (uint8_t *)mempcpy(code_iter, bind_buffer, bind_size);
					result_size -= bind_size;
				}
			}
			/* Use the path/file/line/col of the
			 * first checkpoint as initial state. */
			if (is_first) {
				old_state.reg_col       = new_state.reg_col;
				old_state.reg_lno       = new_state.reg_lno;
				old_state.reg_path      = new_state.reg_path;
				old_state.reg_file      = new_state.reg_file;
				old_state.reg_name      = new_state.reg_name;
				old_state.tpp_file      = new_state.tpp_file;
				result->d_start.dr_path = old_state.reg_path;
				result->d_start.dr_file = old_state.reg_file;
				result->d_start.dr_name = old_state.reg_name;
				result->d_start.dr_col  = old_state.reg_col;
				result->d_start.dr_lno  = old_state.reg_lno;
				/* Remember how many text-bytes were used to describe initial transformations. */
				result->d_ddiinit = (uint16_t)(code_iter - result->d_ddi);
				is_first          = false;
				/* Don't emit a transition if the first checkpoint
				 * is located at the start of the function */
				if (iter->dc_addr == old_state.reg_uip)
					continue;
			}
do_last_transition:

			/* Generate DDI assembly to transition to the new state. */
			text = ddi_transition(text, &old_state, &new_state);

			/* Copy the new state to override the old one. */
			memcpy(&old_state, &new_state, sizeof(struct ddi_gen_state));
			/* Append the new text to the resulting DDI object. */
			text_size = (size_t)(text - buffer);
			/* NOTE: With our very generous preallocation above,
			 *       we're unlikely to have to allocate even more... */
			while unlikely(text_size > result_size) {
				/* Must allocate more buffer memory. */
				size_t old_alloc = result_size + (size_t)(code_iter - result->d_ddi);
				size_t new_alloc = old_alloc * 2;
				DeeDDIObject *new_result;
do_realloc:
				ASSERT(old_alloc != 0);
				ASSERT(new_alloc != 0);
				ASSERT(old_alloc != new_alloc);
				new_result = (DeeDDIObject *)DeeObject_TryReallocc(result, offsetof(DeeDDIObject, d_ddi),
					                                               new_alloc + 1, sizeof(uint8_t));
				if unlikely(!new_result) {
					size_t min_alloc = text_size + (size_t)(code_iter - result->d_ddi);
					if (new_alloc != min_alloc) {
						new_alloc = min_alloc;
						goto do_realloc;
					}
					if (Dee_CollectMemoryoc(offsetof(DeeDDIObject, d_ddi),
						                    new_alloc + 1, sizeof(uint8_t)))
						goto do_realloc;
					goto err_result_printer;
				}
				/* Relocate the code pointer into the new result buffer. */
				code_iter -= (uintptr_t)result;
				code_iter += (uintptr_t)new_result;
				/* Update the result buffer and its remaining size. */
				result      = new_result;
				result_size = new_alloc - (size_t)(code_iter - result->d_ddi);
			}
			code_iter = (uint8_t *)mempcpy(code_iter, buffer, text_size);
			result_size -= text_size;
		}
		if (!did_last) {
			/* Create one last transition to the end of the text
			 * section, thus covering the remainder of user-code. */
			new_state.reg_uip = (code_addr_t)(current_assembler.a_sect[0].sec_iter -
			                                  current_assembler.a_sect[0].sec_begin);
			if (new_state.reg_uip != old_state.reg_uip) {
				new_state.reg_usp = 0;
				did_last          = true;
				--iter;
				text = buffer;
				goto do_last_transition;
			}
		}
	}

	if (!(current_assembler.a_flag & ASM_FNODDIXDAT)) {
		uint16_t i, offset;
		struct symbol *sym;
		struct bytewriter writer = BYTEWRITER_INIT;
		if unlikely(!bytewriter_alloc(&writer, 4))
			goto err_xwriter; /* `dx_size' */
		/* Generate debug information for references and static variables. */
		for (i = 0; i < current_assembler.a_refc; ++i) {
			char *namebuf;
			sym = current_assembler.a_refv[i].sr_sym;
			if (sym->s_name->k_size == 0)
				continue; /* Anonymous reference. */
			namebuf = ascii_printer_allocstr(&strtab,
			                                 sym->s_name->k_name,
			                                 sym->s_name->k_size + 1);
			if unlikely(!namebuf)
				goto err_xwriter;
			if unlikely(xddi_putsymbol(&writer, DDI_EXDAT_O_RNAM, i,
			                           (uint32_t)(namebuf - strtab.ap_string->s_str)))
				goto err_xwriter;
		}
		if (current_assembler.a_staticc != 0) {
			offset = current_assembler.a_refc;
			/* Generate information about the names of static variables. */
			for (i = 0; i < current_assembler.a_staticc; ++i) {
				char *namebuf;
				sym = current_assembler.a_staticv[i].ss_sym;
				if (!sym)
					continue; /* Anonymous symbol (asm-level). */
				if (sym->s_name->k_size == 0)
					continue; /* Anonymous symbol (ast-level). */
				namebuf = ascii_printer_allocstr(&strtab,
				                              sym->s_name->k_name,
				                              sym->s_name->k_size + 1);
				if unlikely(!namebuf)
					goto err_xwriter;
				if unlikely(xddi_putsymbol(&writer, DDI_EXDAT_O_RNAM, offset + i,
				                           (uint32_t)(namebuf - strtab.ap_string->s_str)))
					goto err_xwriter;
			}
		}
		if (writer.bw_size <= 4) {
			/* No extended data. */
			bytewriter_fini(&writer);
			/*result->d_exdat = NULL;*/
		} else {
			if unlikely(bytewriter_putb(&writer, DDI_EXDAT_O_END))
				goto err_xwriter;
			((struct Dee_ddi_exdat *)writer.bw_base)->dx_size = (uint32_t)(writer.bw_size - 4);
			result->d_exdat = (struct Dee_ddi_exdat *)bytewriter_flush(&writer);
		}
		__IF0 {
err_xwriter:
			bytewriter_fini(&writer);
			goto err_result_printer;
		}
	}

	*code_iter++ = DDI_STOP; /* This is why we always overallocate by one. */
	/* Free up unused memory. */
	{
		uint32_t used_size = (uint32_t)(code_iter - result->d_ddi);
		DeeDDIObject *new_result;
		result->d_ddisize = used_size;
		new_result = (DeeDDIObject *)DeeObject_TryRealloc(result,
		                                                  used_size +
		                                                  offsetof(DeeDDIObject, d_ddi));
		if (new_result)
			result = new_result;
	}
	/* Pack the string printer and call it a day. */
	result->d_strtab = (DREF struct string_object *)ascii_printer_pack(&strtab);
	if unlikely(!result->d_strtab)
		goto err_result;
	if (!current_basescope->bs_name) {
		/* If no function name was set, mark the initial DDI-name register as invalid. */
		result->d_start.dr_name = (uint16_t)-1;
	}

	/* Finally, initialize the DDI object. */
	DeeObject_Init(result, &DeeDDI_Type);
	return result;
err_result_printer:
	/* Free all buffers and fail. */
	ascii_printer_fini(&strtab);
err_result:
	Dee_Free((void *)result->d_exdat);
	Dee_Free((void *)result->d_strings);
	DeeObject_Free(result);
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_DDI_C */
