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
#ifndef GUARD_DEEMON_COMPILER_GENDEC_C
#define GUARD_DEEMON_COMPILER_GENDEC_C 1

#include <deemon/api.h>

#ifndef CONFIG_NO_DEC
#include <deemon/hashset.h>
#include <deemon/arg.h>
#include <deemon/cell.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/compiler/dec.h>
#include <deemon/dec.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/string.h>
#include <deemon/tuple.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/unaligned.h>

DECL_BEGIN

#define SC_HEADER     (&current_dec.dw_sec_defl[DEC_SECTION_HEADER])
#define SC_IMPORTS    (&current_dec.dw_sec_defl[DEC_SECTION_IMPORTS])
#define SC_DEPS       (&current_dec.dw_sec_defl[DEC_SECTION_DEPS])
#define SC_GLOBALS    (&current_dec.dw_sec_defl[DEC_SECTION_GLOBALS])
#define SC_ROOT       (&current_dec.dw_sec_defl[DEC_SECTION_ROOT])
#define SC_TEXT       (&current_dec.dw_sec_defl[DEC_SECTION_TEXT])
#define SC_DEBUG      (&current_dec.dw_sec_defl[DEC_SECTION_DEBUG])
#define SC_DEBUG_DATA (&current_dec.dw_sec_defl[DEC_SECTION_DEBUG_DATA])
#define SC_DEBUG_TEXT (&current_dec.dw_sec_defl[DEC_SECTION_DEBUG_TEXT])
#define SC_STRING     (&current_dec.dw_sec_defl[DEC_SECTION_STRING])


#ifdef CONFIG_HOST_WINDOWS
#define SEP '\\'
#else /* CONFIG_HOST_WINDOWS */
#define SEP '/'
#endif /* !CONFIG_HOST_WINDOWS */

PRIVATE WUNUSED NONNULL((1)) int DCALL
decgen_imports(DeeModuleObject *__restrict self) {
	DeeModuleObject **iter, **end;
	char *module_pathstr, *module_pathend;
	if (!self->mo_importc)
		goto done;
	dec_curr = SC_IMPORTS;
	if (dec_putw(self->mo_importc))
		goto err; /* Dec_Strmap.i_len */
	end = (iter = (DeeModuleObject **)self->mo_importv) + self->mo_importc;
	module_pathstr = module_pathend = NULL;
	for (; iter != end; ++iter) {
		DeeModuleObject *mod = *iter;
		uint8_t *data;
		uint32_t addr;
		ASSERT_OBJECT_TYPE(mod, &DeeModule_Type);
		dec_curr = SC_STRING;
		if (mod->mo_globpself) {
			/* Globally available module (loadable as part of the library path). */
			char const *global_name;
import_module_by_name:
			global_name = DeeString_AsUtf8((DeeObject *)mod->mo_name);
			if unlikely(!global_name)
				goto err;
			data = dec_allocstr(global_name,
			                    (WSTR_LENGTH(global_name) + 1) * sizeof(char));
			if unlikely(!data)
				goto err;
		} else {
			char *self_pathstr, *other_pathstr;
			char *self_pathend, *other_pathend;
			char *self_pathpart, *other_pathpart;
			size_t num_dots;
			uint8_t *buffer;
			bool is_dec_file;
			/* NOTE: The builtin `deemon' module, as well as
			 *       the empty module don't have a path assigned. */
			if (!mod->mo_path)
				goto import_module_by_name;
			other_pathstr = DeeString_AsUtf8((DeeObject *)mod->mo_path);
			if unlikely(!other_pathstr)
				goto err;
			other_pathend = other_pathstr + WSTR_LENGTH(other_pathstr);
			if (!module_pathstr) {
				/* Lazily calculate the module's path when it's start being used. */
				module_pathstr = DeeString_AsUtf8((DeeObject *)self->mo_path);
				if unlikely(!module_pathstr)
					goto err;
				module_pathend = module_pathstr + WSTR_LENGTH(module_pathstr);
				/* NOTE: No need to check for alternative separators
				 *       because the path has been sanitized. */
				while (module_pathstr != module_pathend &&
				       module_pathend[-1] != SEP)
					--module_pathend;
			}
			self_pathstr = module_pathstr;
			self_pathend = module_pathend;
			/* NOTE: No need to check for spaces surrounding
			 *       slashes, because the path has been sanitized. */
			self_pathpart  = self_pathstr;
			other_pathpart = other_pathstr;
			while (self_pathpart != self_pathend &&
			       other_pathpart != other_pathend) {
#ifdef CONFIG_HOST_WINDOWS
				/* Do a case-insensitive match on windows. */
				if (DeeUni_ToLower(*self_pathpart) !=
				    DeeUni_ToLower(*other_pathpart))
					break;
#else /* CONFIG_HOST_WINDOWS */
				if (*self_pathpart != *other_pathpart)
					break;
#endif /* !CONFIG_HOST_WINDOWS */
				++self_pathpart, ++other_pathpart;
				if (self_pathpart[-1] == SEP) {
					/* Save the first character after the last matching separator. */
					self_pathstr  = self_pathpart;
					other_pathstr = other_pathpart;
				}
			}

			/* All right! we've gotten rid of the common path portion.
			 * Now to handle the portion that isn't common.
			 * For this, we need to write 1+self_pathstr.count(SEP) `.'
			 * characters to deal with up-path reference, which is then
			 * followed by `other_pathstr.replace(SEP,".")' */
			num_dots = 1; /* +1 leading dot to identify the use of a local dependency name. */
			while (self_pathstr != self_pathend) {
				if (*self_pathstr++ == SEP)
					++num_dots;
			}

			/* Rewind to exclude the extension. */
			if (other_pathend - 4 <= other_pathstr)
				goto import_module_by_name;
			if (other_pathend[-4] != '.')
				goto import_module_by_name;
#ifdef CONFIG_HOST_WINDOWS
			if (other_pathend[-3] != 'd' && other_pathend[-3] != 'D')
				goto import_module_by_name;
			if (other_pathend[-2] != 'e' && other_pathend[-2] != 'E')
				goto import_module_by_name;
			if (other_pathend[-1] != 'c' && other_pathend[-1] != 'C' &&
			    other_pathend[-1] != 'e' && other_pathend[-1] != 'E')
				goto import_module_by_name;
			is_dec_file = (other_pathend[-1] == 'c' || other_pathend[-1] == 'C');
#else /* CONFIG_HOST_WINDOWS */
			if (other_pathend[-3] != 'd')
				goto import_module_by_name;
			if (other_pathend[-2] != 'e')
				goto import_module_by_name;
			if (other_pathend[-1] != 'c' &&
			    other_pathend[-1])
				goto import_module_by_name;
			is_dec_file = (other_pathend[-1] == 'c');
#endif /* !CONFIG_HOST_WINDOWS */
			other_pathend -= 4;

			/* Write all of the dots. */
			buffer = dec_alloc(num_dots);
			if unlikely(!buffer)
				goto err;
			memset(buffer, '.', num_dots);

			/* Now write the other pathname with all SEPs replaced with `.' */
			buffer = dec_alloc(1 + (size_t)(other_pathend - other_pathstr));
			if unlikely(!buffer)
				goto err;
			other_pathpart = other_pathstr;
			if (is_dec_file) {
				/* In dec files, we must erase the first `.' character of the filename. */
				char *last_dot = (char *)buffer;
				for (; other_pathpart != other_pathend;
				     ++other_pathpart, ++buffer) {
					if ((*buffer = *other_pathpart) == SEP)
						*(char *)buffer = '.', last_dot = (char *)buffer + 1;
				}
				if (*last_dot != '.') {
					/* Shouldn't happen: The leading dot is missing from the pathname of the module. */
					dec_curr->ds_iter -= 1 + (size_t)(other_pathend - other_pathstr);
					goto import_module_by_name;
				}
				memmove(last_dot, last_dot + 1,
				        (size_t)((char *)buffer - last_dot) * sizeof(char));
				/* Release one character from the buffer. */
				--buffer, --other_pathend;
				--dec_curr->ds_iter;
				;
			} else {
				for (; other_pathpart != other_pathend;
				     ++other_pathpart, ++buffer) {
					if ((*buffer = *other_pathpart) == SEP)
						*buffer = '.';
				}
			}
			*buffer = '\0'; /* Add the ZERO-terminator. */

			/* Package the newly written relative module
			 * name and retrieve its base pointer. */
			data = dec_reuselocal(num_dots + 1 + (size_t)(other_pathend - other_pathstr));
			ASSERT(data); /* `dec_reuselocal()' never returns NULL */
		}
		addr     = dec_ptr2addr(data);
		dec_curr = SC_IMPORTS;
		/* Encode the string address as a pointer. */
		if (dec_putptr(addr))
			goto err;
	}
done:
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
decgen_globals(DeeModuleObject *__restrict self) {
#if 1
	struct module_symbol *symbegin, *symend;
	struct module_symbol *symiter;
	uint16_t globalc, symcount, normali;
	struct dec_section *symtab; /* Vector of regular symbols (`Dec_GlbSym' in `Dec_Glbmap.g_map') */
	struct dec_section *exttab; /* Vector of extended symbols (`Dec_GlbExt' in `Dec_Glbmap.g_ext') */
	bool has_special_symbols = false;
	symtab = dec_newsection_after(SC_GLOBALS);
	if unlikely(!symtab)
		goto err;
	exttab = dec_newsection_after(symtab);
	if unlikely(!exttab)
		goto err;
	globalc = self->mo_globalc, symcount = 0;
	symend = (symbegin = self->mo_bucketv) +
	         (self->mo_bucketm + 1);
	for (normali = 0; normali < globalc; ++normali) {
		/* Find the original symbol for this variable. */
		uint8_t *ptr;
		uint32_t addr;
		struct module_symbol *first_alias = NULL;
		for (symiter = symbegin; symiter != symend; ++symiter) {
			if (!symiter->ss_name)
				continue; /* Skip empty entries. */
			if (symiter->ss_flags & MODSYM_FEXTERN) {
				has_special_symbols = true;
				continue;
			}
			if (symiter->ss_index == normali) {
				if (!first_alias)
					first_alias = symiter;
				if (!(symiter->ss_flags & MODSYM_FALIAS))
					break;
			}
		}
		++symcount; /* Track the total number of symbols. */
		dec_curr = symtab;
		if unlikely(!symiter) {
			/* No symbol exist do describe this global variable. */
			if (dec_putw(0))
				goto err; /* Dec_GlbSym.s_flg */
			dec_curr = SC_STRING;
			ptr      = dec_allocstr("", sizeof(char));
			if unlikely(!ptr)
				goto err;
			addr     = dec_ptr2addr(ptr);
			dec_curr = symtab;
			if (dec_putptr(addr))
				goto err; /* Dec_GlbSym.s_nam */
		} else {
			size_t name_len;
			/* Write information for this symbol. */
			if (dec_putw(symiter->ss_flags & ~(MODSYM_FNAMEOBJ | MODSYM_FDOCOBJ)))
				goto err; /* Dec_GlbSym.s_flg */
			dec_curr = SC_STRING;
			name_len = MODULE_SYMBOL_GETNAMELEN(symiter);
			ptr = dec_allocstr(MODULE_SYMBOL_GETNAMESTR(symiter),
			                   (name_len + 1) * sizeof(char));
			if unlikely(!ptr)
				goto err;
			addr     = dec_ptr2addr(ptr);
			dec_curr = symtab;
			if (dec_putptr(addr))
				goto err; /* Dec_GlbSym.s_nam */
			if likely(name_len) {
				char const *doc_str;
				size_t doc_len;
				if (!symiter->ss_doc || (current_dec.dw_flags & DEC_WRITE_FNODOC)) {
					doc_str = NULL;
					doc_len = 0;
				} else {
					doc_str = MODULE_SYMBOL_GETDOCSTR(symiter);
					doc_len = MODULE_SYMBOL_GETDOCLEN(symiter);
				}
				if (dec_putptr((uint32_t)doc_len))
					goto err; /* Dec_GlbSym.s_doclen */
				if (doc_len) {
					dec_curr = SC_STRING;
					ptr = dec_allocstr(doc_str, (doc_len + 1) * sizeof(char));
					if unlikely(!ptr)
						goto err;
					addr     = dec_ptr2addr(ptr);
					dec_curr = symtab;
					if (dec_putptr(addr))
						goto err; /* Dec_GlbSym.s_doc */
				}
			}
			/* Dump all other symbol with for the same
			 * address in the extended symbol table.
			 * NOTE: To speed up this check, `first_alias' is
			 *       a pointer to the first encountered symbol
			 *       that was aliasing the current global variable
			 *       associated with the current `normali' address. */
			ASSERT(first_alias);
			for (++first_alias; first_alias != symend; ++first_alias) {
				if (!first_alias->ss_name)
					continue;
				if (first_alias->ss_flags & MODSYM_FEXTERN)
					continue;
				if (first_alias->ss_index != normali)
					continue;
				/* Found another symbol for this address.
				 * Add it to the extended symbol table. */
				dec_curr = exttab;
				++symcount; /* Track the total number of symbols. */
				if (dec_putw(first_alias->ss_flags & ~(MODSYM_FNAMEOBJ | MODSYM_FDOCOBJ)))
					goto err; /* Dec_GlbExt.s_flg */
				if (dec_putw(normali))
					goto err; /* Dec_GlbExt.s_addr */
				dec_curr = SC_STRING;
				name_len = MODULE_SYMBOL_GETNAMELEN(first_alias);
				ptr = dec_allocstr(MODULE_SYMBOL_GETNAMESTR(first_alias),
				                   (name_len + 1) * sizeof(char));
				if unlikely(!ptr)
					goto err;
				addr     = dec_ptr2addr(ptr);
				dec_curr = exttab;
				if (dec_putptr(addr))
					goto err; /* Dec_GlbExt.s_nam */
				if likely(name_len) {
					/* Write the length of the doc, and potentially the doc, too. */
					char const *doc_str;
					size_t doc_len;
					if (!first_alias->ss_doc || (current_dec.dw_flags & DEC_WRITE_FNODOC)) {
						doc_str = NULL;
						doc_len = 0;
					} else {
						doc_str = MODULE_SYMBOL_GETDOCSTR(first_alias);
						doc_len = MODULE_SYMBOL_GETDOCLEN(first_alias);
					}
					if (dec_putptr((uint32_t)doc_len))
						goto err; /* Dec_GlbSym.s_doclen */
					if (doc_len) {
						dec_curr = SC_STRING;
						ptr = dec_allocstr(doc_str, (doc_len + 1) * sizeof(char));
						if unlikely(!ptr)
							goto err;
						addr     = dec_ptr2addr(ptr);
						dec_curr = symtab;
						if (dec_putptr(addr))
							goto err; /* Dec_GlbSym.s_doc */
					}
				}
			}
		}
	}
	if (has_special_symbols) {
		/* Emit special (extern) symbols */
		dec_curr = exttab;
		for (symiter = symbegin; symiter != symend; ++symiter) {
			uint8_t *ptr;
			uint32_t addr;
			size_t name_len;
			if (!symiter->ss_name)
				continue; /* Skip empty entries. */
			if (!(symiter->ss_flags & MODSYM_FEXTERN))
				continue;
			++symcount; /* Track the total number of symbols. */
			if (dec_putw(symiter->ss_flags & ~(MODSYM_FNAMEOBJ | MODSYM_FDOCOBJ)))
				goto err; /* Dec_GlbExt.s_flg */
			if (dec_putw(symiter->ss_extern.ss_symid))
				goto err; /* Dec_GlbExt.s_addr */
			if (dec_putw(symiter->ss_extern.ss_impid))
				goto err; /* Dec_GlbExt.s_addr2 */
			dec_curr = SC_STRING;
			name_len = MODULE_SYMBOL_GETNAMELEN(symiter);
			ptr = dec_allocstr(MODULE_SYMBOL_GETNAMESTR(symiter),
			                   (name_len + 1) * sizeof(char));
			if unlikely(!ptr)
				goto err;
			addr     = dec_ptr2addr(ptr);
			dec_curr = exttab;
			if (dec_putptr(addr))
				goto err; /* Dec_GlbExt.s_nam */
			if likely(name_len) {
				/* Write the length of the doc, and potentially the doc, too. */
				char const *doc_str;
				size_t doc_len;
				if (!symiter->ss_doc || (current_dec.dw_flags & DEC_WRITE_FNODOC)) {
					doc_str = NULL;
					doc_len = 0;
				} else {
					doc_str = MODULE_SYMBOL_GETDOCSTR(symiter);
					doc_len = MODULE_SYMBOL_GETDOCLEN(symiter);
				}
				if (dec_putptr((uint32_t)doc_len))
					goto err; /* Dec_GlbSym.s_doclen */
				if (doc_len) {
					dec_curr = SC_STRING;
					ptr = dec_allocstr(doc_str,
					                   (doc_len + 1) * sizeof(char));
					if unlikely(!ptr)
						goto err;
					addr     = dec_ptr2addr(ptr);
					dec_curr = symtab;
					if (dec_putptr(addr))
						goto err; /* Dec_GlbSym.s_doc */
				}
			}
		}
	}

	/* Print the header for the global variable table. */
	dec_curr = SC_GLOBALS;
	if (dec_putw(globalc))
		goto err; /* Dec_Glbmap.g_cnt */
	if (dec_putw(symcount))
		goto err; /* Dec_Glbmap.g_len */
		          /* ... This is where the 2 sections we've created above will appear. */
#else
	struct module_symbol *symbegin, *symend;
	struct module_symbol *symiter;
	uint16_t symcount = 0;
	dec_curr = SC_GLOBALS;
	symend = (symbegin = self->mo_bucketv) +
	         (self->mo_bucketm + 1);
	/* Count the actual number of symbols defined by the module. */
	for (symiter = symbegin;
	     symiter != symend; ++symiter) {
		if (symiter->ss_name)
			++symcount;
	}
	if (dec_putw(self->mo_globalc))
		goto err; /* Dec_Glbmap.g_cnt */
	if (dec_putw(symcount))
		goto err; /* Dec_Glbmap.g_len */
	/* Go through and define descriptors for all exported symbols. */
	for (symiter = symbegin;
	     symiter != symend; ++symiter) {
		uint8_t *nameptr;
		uint32_t nameaddr;
		DeeStringObject *docstr;
		if (!symiter->ss_name)
			continue;
		if (dec_putw(symiter->ss_flags))
			goto err; /* Dec_GlbSym.s_flg */
		if (dec_putw(symiter->ss_index))
			goto err; /* Dec_GlbSym.s_addr */
		dec_curr = SC_STRING;
		nameptr = dec_allocstr(DeeString_STR(symiter->ss_name),
		                       (DeeString_SIZE(symiter->ss_name) + 1) * sizeof(char));
		if unlikely(!nameptr)
			goto err;
		nameaddr = dec_ptr2addr(nameptr);
		dec_curr = SC_GLOBALS;
		if (dec_putptr(nameaddr))
			goto err; /* Dec_GlbSym.s_nam */
		docstr = symiter->ss_doc;
		if (!docstr || (current_dec.dw_flags & DEC_WRITE_FNODOC))
			docstr = (DeeStringObject *)Dee_EmptyString;
		if (dec_putptr(DeeString_SIZE(docstr)))
			goto err; /* Dec_GlbSym.s_doclen */
		if (DeeString_SIZE(docstr)) {
			/* Also include the symbol's documentation string. */
			dec_curr = SC_STRING;
			nameptr = dec_allocstr(DeeString_STR(docstr),
			                       DeeString_SIZE(docstr) * sizeof(char));
			if unlikely(!nameptr)
				goto err;
			nameaddr = dec_ptr2addr(nameptr);
			dec_curr = SC_GLOBALS;
			if (dec_putptr(nameaddr))
				goto err; /* Dec_GlbSym.s_doc */
		}
	}
#endif
	return 0;
err:
	return -1;
}


STATIC_ASSERT((DTYPE16_CLASSDESC & 0xff) == DTYPE_CLASSDESC);

PRIVATE WUNUSED NONNULL((1)) int DCALL
dec_putclassdesc(DeeClassDescriptorObject *__restrict self) {
	uint32_t straddr;
	uint8_t *strptr;
	struct dec_section *oldsec;
	bool use_8bit = true;
	size_t i;
	uint16_t op_count  = 0;
	size_t cattr_count = 0;
	size_t iattr_count = 0;
	/* Count the number of operators, class- and instance-attributes. */
	for (i = 0; i <= self->cd_clsop_mask; ++i) {
		if (self->cd_clsop_list[i].co_name == (uint16_t)-1)
			continue;
		if (self->cd_clsop_list[i].co_name > UINT8_MAX)
			use_8bit = false;
		ASSERT(self->cd_clsop_list[i].co_addr < self->cd_cmemb_size);
		++op_count;
	}
	for (i = 0; i <= self->cd_cattr_mask; ++i) {
		if (self->cd_cattr_list[i].ca_name == NULL)
			continue;
		ASSERT(self->cd_cattr_list[i].ca_addr < self->cd_cmemb_size);
		ASSERT(!(self->cd_cattr_list[i].ca_flag & ~CLASS_ATTRIBUTE_FMASK));
#if CLASS_ATTRIBUTE_FMASK > UINT8_MAX
		if (self->cd_cattr_list[i].ca_flag > UINT8_MAX)
			use_8bit = false;
#endif /* CLASS_ATTRIBUTE_FMASK > UINT8_MAX */
		++cattr_count;
	}
	for (i = 0; i <= self->cd_iattr_mask; ++i) {
		if (self->cd_iattr_list[i].ca_name == NULL)
			continue;
		ASSERT((self->cd_iattr_list[i].ca_addr) <
		       ((self->cd_iattr_list[i].ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		        ? self->cd_cmemb_size
		        : self->cd_imemb_size));
		ASSERT(!(self->cd_iattr_list[i].ca_flag & ~CLASS_ATTRIBUTE_FMASK));
#if CLASS_ATTRIBUTE_FMASK > UINT8_MAX
		if (self->cd_iattr_list[i].ca_flag > UINT8_MAX)
			use_8bit = false;
#endif /* CLASS_ATTRIBUTE_FMASK > UINT8_MAX */
		++iattr_count;
	}
	/* Check control sizes to see if we can still use the 8-bit encoding. */
	if (self->cd_flags > UINT8_MAX || self->cd_cmemb_size > UINT8_MAX ||
	    self->cd_imemb_size > UINT8_MAX || op_count > UINT8_MAX ||
	    cattr_count > UINT8_MAX || iattr_count > UINT8_MAX)
		use_8bit = false;
	if (!use_8bit && dec_putb((DTYPE16_CLASSDESC & 0xff00) >> 8))
		goto err;
	if (dec_putb(DTYPE_CLASSDESC))
		goto err;
	if (use_8bit ? dec_putb((uint8_t)self->cd_flags) : dec_putw(self->cd_flags)) /* Dec_ClassDescriptor::cd_flags */
		goto err;
	oldsec   = dec_curr;
	dec_curr = SC_STRING;
	if (self->cd_name) {
		strptr = dec_allocstr(DeeString_STR(self->cd_name),
		                      (DeeString_SIZE(self->cd_name) + 1) * sizeof(char));
	} else {
		strptr = dec_allocstr("", sizeof(char));
	}
	if unlikely(!strptr)
		goto err;
	straddr  = dec_ptr2addr(strptr);
	dec_curr = oldsec;
	if (dec_putptr(straddr))
		goto err; /* Dec_ClassDescriptor::cd_nam */
	if (self->cd_doc) {
		char *doc_str = DeeString_AsUtf8((DeeObject *)self->cd_doc);
		if unlikely(!doc_str)
			goto err;
		if (WSTR_LENGTH(doc_str) == 0)
			goto empty_doc;
		if (dec_putptr((uint32_t)WSTR_LENGTH(doc_str)))
			goto err; /* Dec_ClassDescriptor::cd_doclen */
		oldsec   = dec_curr;
		dec_curr = SC_STRING;
		strptr   = dec_allocstr(doc_str, WSTR_LENGTH(doc_str) * sizeof(char));
		if unlikely(!strptr)
			goto err;
		straddr  = dec_ptr2addr(strptr);
		dec_curr = oldsec;
		if (dec_putptr(straddr))
			goto err; /* Dec_ClassDescriptor::cd_doc */
	} else {
empty_doc:
		if (dec_putptr(0))
			goto err; /* Dec_ClassDescriptor::cd_doclen */
	}
	if (use_8bit) {
		if (dec_putb((uint8_t)self->cd_cmemb_size))
			goto err; /* Dec_8BitClassDescriptor::cd_cmemb_size */
		if (dec_putb((uint8_t)self->cd_imemb_size))
			goto err; /* Dec_8BitClassDescriptor::cd_imemb_size */
		if (dec_putb((uint8_t)op_count))
			goto err; /* Dec_8BitClassDescriptor::cd_op_count */
		if (dec_putb((uint8_t)cattr_count))
			goto err; /* Dec_8BitClassDescriptor::cd_cattr_count */
		if (dec_putb((uint8_t)iattr_count))
			goto err; /* Dec_8BitClassDescriptor::cd_iattr_count */
	} else {
		if (dec_putw(self->cd_cmemb_size))
			goto err; /* Dec_ClassDescriptor::cd_cmemb_size */
		if (dec_putw(self->cd_imemb_size))
			goto err; /* Dec_ClassDescriptor::cd_imemb_size */
		if (dec_putw(op_count))
			goto err; /* Dec_ClassDescriptor::cd_op_count */
		if (dec_putptr((uint32_t)cattr_count))
			goto err; /* Dec_ClassDescriptor::cd_cattr_count */
		if (dec_putptr((uint32_t)iattr_count))
			goto err; /* Dec_ClassDescriptor::cd_iattr_count */
	}
	/* Emit data for all the operators. */
	for (i = 0; i <= self->cd_clsop_mask; ++i) {
		uint16_t name, addr;
		name = self->cd_clsop_list[i].co_name;
		if (name == (uint16_t)-1)
			continue;
		addr = self->cd_clsop_list[i].co_addr;
		ASSERT(addr < self->cd_cmemb_size);
		if (use_8bit) {
			if (dec_putb((uint8_t)name))
				goto err; /* Dec_8BitClassOperator::co_name */
			if (dec_putb((uint8_t)addr))
				goto err; /* Dec_8BitClassOperator::co_addr */
		} else {
			if (dec_putw(name))
				goto err; /* Dec_ClassOperator::co_name */
			if (dec_putw(addr))
				goto err; /* Dec_ClassOperator::co_addr */
		}
	}
	/* Emit data for class attributes. */
	for (i = 0; i <= self->cd_cattr_mask; ++i) {
		struct class_attribute *attr;
		attr = &self->cd_cattr_list[i];
		if (attr->ca_name == NULL)
			continue;
		ASSERT(attr->ca_addr < self->cd_cmemb_size);
		ASSERT(!(attr->ca_flag & ~CLASS_ATTRIBUTE_FMASK));
		if (use_8bit) {
			if (dec_putb((uint8_t)attr->ca_addr))
				goto err; /* Dec_8BitClassAttribute::ca_addr */
			if (dec_putb((uint8_t)attr->ca_flag))
				goto err; /* Dec_8BitClassAttribute::ca_flags */
		} else {
			if (dec_putw(attr->ca_addr))
				goto err; /* Dec_ClassAttribute::ca_addr */
			if (dec_putw(attr->ca_flag))
				goto err; /* Dec_ClassAttribute::ca_flags */
		}
		oldsec   = dec_curr;
		dec_curr = SC_STRING;
		strptr = dec_allocstr(DeeString_STR(attr->ca_name),
		                      (DeeString_SIZE(attr->ca_name) + 1) * sizeof(char));
		if unlikely(!strptr)
			goto err;
		straddr  = dec_ptr2addr(strptr);
		dec_curr = oldsec;
		if (dec_putptr(straddr))
			goto err; /* Dec_ClassAttribute::ca_nam */
		if (attr->ca_doc) {
			char *doc_str = DeeString_AsUtf8((DeeObject *)attr->ca_doc);
			if unlikely(!doc_str)
				goto err;
			if (WSTR_LENGTH(doc_str) == 0)
				goto empty_cattr_doc;
			if (dec_putptr((uint32_t)WSTR_LENGTH(doc_str)))
				goto err; /* Dec_ClassAttribute::ca_doclen */
			oldsec   = dec_curr;
			dec_curr = SC_STRING;
			strptr   = dec_allocstr(doc_str, WSTR_LENGTH(doc_str) * sizeof(char));
			if unlikely(!strptr)
				goto err;
			straddr  = dec_ptr2addr(strptr);
			dec_curr = oldsec;
			if (dec_putptr(straddr))
				goto err; /* Dec_ClassAttribute::ca_doc */
		} else {
empty_cattr_doc:
			if (dec_putptr(0))
				goto err; /* Dec_ClassAttribute::ca_doclen */
		}
	}
	/* Emit data for instance attributes. */
	for (i = 0; i <= self->cd_iattr_mask; ++i) {
		struct class_attribute *attr;
		attr = &self->cd_iattr_list[i];
		if (attr->ca_name == NULL)
			continue;
		ASSERT((attr->ca_addr) <
		       ((attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		        ? self->cd_cmemb_size
		        : self->cd_imemb_size));
		ASSERT(!(attr->ca_flag & ~CLASS_ATTRIBUTE_FMASK));
		if (use_8bit) {
			if (dec_putb((uint8_t)attr->ca_addr))
				goto err; /* Dec_8BitClassAttribute::ca_addr */
			if (dec_putb((uint8_t)attr->ca_flag))
				goto err; /* Dec_8BitClassAttribute::ca_flags */
		} else {
			if (dec_putw(attr->ca_addr))
				goto err; /* Dec_ClassAttribute::ca_addr */
			if (dec_putw(attr->ca_flag))
				goto err; /* Dec_ClassAttribute::ca_flags */
		}
		oldsec   = dec_curr;
		dec_curr = SC_STRING;
		strptr = dec_allocstr(DeeString_STR(attr->ca_name),
		                      (DeeString_SIZE(attr->ca_name) + 1) * sizeof(char));
		if unlikely(!strptr)
			goto err;
		straddr  = dec_ptr2addr(strptr);
		dec_curr = oldsec;
		if (dec_putptr(straddr))
			goto err; /* Dec_ClassAttribute::ca_nam */
		if (attr->ca_doc) {
			char *doc_str = DeeString_AsUtf8((DeeObject *)attr->ca_doc);
			if unlikely(!doc_str)
				goto err;
			if (WSTR_LENGTH(doc_str) == 0)
				goto empty_iattr_doc;
			if (dec_putptr((uint32_t)WSTR_LENGTH(doc_str)))
				goto err; /* Dec_ClassAttribute::ca_doclen */
			oldsec   = dec_curr;
			dec_curr = SC_STRING;
			strptr = dec_allocstr(doc_str, WSTR_LENGTH(doc_str) * sizeof(char));
			if unlikely(!strptr)
				goto err;
			straddr  = dec_ptr2addr(strptr);
			dec_curr = oldsec;
			if (dec_putptr(straddr))
				goto err; /* Dec_ClassAttribute::ca_doc */
		} else {
empty_iattr_doc:
			if (dec_putptr(0))
				goto err; /* Dec_ClassAttribute::ca_doclen */
		}
	}
	return 0;
err:
	return -1;
}


/* Emit a `DTYPE_KWDS', followed by a keywords descriptor. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
dec_putkwds(DeeKwdsObject *__restrict self) {
	if (dec_putb(DTYPE_KWDS))
		goto err;
	if (dec_putptr((uint32_t)self->kw_size))
		goto err; /* Dec_Kwds.kw_siz */
	if (self->kw_size) {
		size_t i, j;
		for (i = 0; i < self->kw_size; ++i) {
			uint32_t straddr;
			uint8_t *strptr;
			struct dec_section *oldsec;
			DeeStringObject *name;
			for (j = 0;; ++j) {
				ASSERT(j <= self->kw_mask);
				if (!self->kw_map[j].ke_name)
					continue;
				if (self->kw_map[j].ke_index == i)
					break;
			}
			name     = self->kw_map[j].ke_name;
			oldsec   = dec_curr;
			dec_curr = SC_STRING;
			strptr = dec_allocstr(DeeString_STR(name),
			                      (DeeString_SIZE(name) + 1) * sizeof(char));
			if unlikely(!strptr)
				goto err;
			straddr  = dec_ptr2addr(strptr);
			dec_curr = oldsec;
			if (dec_putptr(straddr))
				goto err; /* Dec_KwdsEntry.kwe_nam */
		}
	}
	return 0;
err:
	return -1;
}


INTERN int (DCALL dec_putobjv)(uint16_t count, DeeObject **__restrict vec) {
	uint16_t i;
	if (dec_putw(count))
		goto err; /* Dec_Objects.os_len */
	for (i = 0; i < count; ++i) {
		/* Encode the object. */
		if (dec_putobj(vec[i]))
			goto err;
	}
	return 0;
err:
	return -1;
}

struct dec_recursion_frame {
	struct dec_recursion_frame *drf_prev; /* [0..1] Previous frame. */
	DeeObject                  *drf_obj;  /* The Potentially recursive object. */
};

PRIVATE struct dec_recursion_frame *dec_obj_recursion = NULL;
#define DEC_RECURSION_BEGIN(obj)             \
	do {                                     \
		struct dec_recursion_frame _frame;   \
		_frame.drf_prev = dec_obj_recursion; \
		_frame.drf_obj  = (obj);             \
		dec_obj_recursion = &_frame
#define DEC_RECURSION_BREAK() \
		(dec_obj_recursion = _frame.drf_prev)
#define DEC_RECURSION_END()    \
		DEC_RECURSION_BREAK(); \
	} __WHILE0

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define dec_recursion_check(self) \
	__builtin_expect(dec_recursion_check(self), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

/* Protection against GC-enabled object recursion:
 * DEC cannot encode recursive sequence objects.
 * Because of this, a non-implemented error is thrown if this
 * function determines that DEC type codes for the the given
 * object `self' are already being generated.
 * NOTE: This function must only be called
 *       before encoding any GC-able sequence object,
 *       such as a Cell, a List, a Set, or a Dict. */
PRIVATE int (DCALL dec_recursion_check)(DeeObject *__restrict self) {
	struct dec_recursion_frame *iter;
	for (iter = dec_obj_recursion; iter;
	     iter = iter->drf_prev) {
		if unlikely(iter->drf_obj == self)
			goto err_recursion;
	}
	return 0;
err_recursion:
	DeeError_Throwf(&DeeError_NotImplemented,
	                "Self-referencing %k %r cannot be encoded in a DEC file",
	                Dee_TYPE(self), self);
	return -1;
}



INTERN int (DCALL dec_putobj)(DeeObject *self) {
	DeeTypeObject *tp_self;
	/* Special handling for encoding various different types of objects. */
	uint16_t builtin_id;
	/* Special case: Encode a NULL object using `DTYPE_NULL' */
	if (!self)
		return dec_putb(DTYPE_NULL);

	tp_self = Dee_TYPE(self);
	if (tp_self == &DeeNone_Type)
		return dec_putb(DTYPE_NONE);
	if (tp_self == &DeeFloat_Type) {
		if (dec_putb(DTYPE_IEEE754))
			goto err;
		return dec_putieee754(DeeFloat_VALUE(self));
	}
	if (tp_self == &DeeInt_Type) {
		uint8_t *buffer;
		if (DeeInt_IsNeg(self)) {
			/* Encode as signed. */
			buffer = dec_alloc(1 + DeeInt_GetSlebMaxSize(self));
			if unlikely(!buffer)
				goto err;
			*buffer++ = DTYPE_SLEB; /* Type byte */
			buffer    = DeeInt_GetSleb(self, buffer);
		} else {
			/* Encode as unsigned. */
			buffer = dec_alloc(1 + DeeInt_GetUlebMaxSize(self));
			if unlikely(!buffer)
				goto err;
			*buffer++ = DTYPE_ULEB; /* Type byte */
			buffer    = DeeInt_GetUleb(self, buffer);
		}
		/* Save the actual end of the LEB-encoded integer. */
		dec_curr->ds_iter = buffer;
		goto done;
	}
	if (tp_self == &DeeString_Type) {
		uint8_t *strptr;
		uint32_t straddr;
		struct dec_section *oldsec;
		char *utf8_data;
		if (dec_putb(DTYPE_STRING))
			goto err;
		utf8_data = DeeString_AsUtf8(self);
		if unlikely(!utf8_data)
			goto err;
		if (dec_putptr((uint32_t)WSTR_LENGTH(utf8_data)))
			goto err;
		/* Special case: an empty string doesn't have an address pointer. */
		if (DeeString_IsEmpty(self))
			goto done;
		/* Emit the string within the string section. */
		oldsec   = dec_curr;
		dec_curr = SC_STRING;
		strptr = dec_allocstr(utf8_data,
		                      WSTR_LENGTH(utf8_data) *
		                      sizeof(char));
		if unlikely(!strptr)
			goto err;
		straddr  = dec_ptr2addr(strptr);
		dec_curr = oldsec;
		/* Emit the address of the string. */
		if (dec_putptr(straddr))
			goto err;
		goto done;
	}
	if (tp_self == &DeeTuple_Type) {
		size_t i, length = DeeTuple_SIZE(self);
		if (dec_putb(DTYPE_TUPLE))
			goto err;
		if (dec_putptr((uint32_t)length))
			goto err;
		/* Encode all of the tuple's elements. */
		for (i = 0; i < length; ++i)
			if (dec_putobj(DeeTuple_GET(self, i)))
				goto err;
		goto done;
	}
	if (tp_self == &DeeList_Type) {
		size_t i, length;
		if (dec_recursion_check(self))
			goto err;
		if (dec_putb(DTYPE_LIST))
			goto err;
		DeeList_LockRead(self);
		length = DeeList_SIZE(self);
		DeeList_LockEndRead(self);
		if (dec_putptr((uint32_t)length))
			goto err;
		DEC_RECURSION_BEGIN(self);
		/* Encode all of the list's elements. */
		DeeList_LockRead(self);
		for (i = 0; i < length; ++i) {
			DREF DeeObject *obj;
			int error;
			/* Must re-validate the list's length, because it may have changed. */
			obj = (i < DeeList_SIZE(self) ? DeeList_GET(self, i) : Dee_None);
			Dee_Incref(obj);
			DeeList_LockEndRead(self);
			/* Emit the list item. */
			error = dec_putobj(obj);
			Dee_Decref(obj);
			if unlikely(error) {
				DEC_RECURSION_BREAK();
				goto err;
			}
			DeeList_LockRead(self);
		}
		DeeList_LockEndRead(self);
		DEC_RECURSION_END();
		goto done;
	}
	if (tp_self == &DeeHashSet_Type) {
		size_t i, length, written;
		DeeHashSetObject *me = (DeeHashSetObject *)self;
		if (dec_recursion_check(self))
			goto err;
		if (dec_putb((DTYPE16_HASHSET & 0xff00) >> 8))
			goto err;
		if (dec_putb(DTYPE16_HASHSET & 0xff))
			goto err;
		DeeHashSet_LockRead(me);
		length = me->s_used;
		DeeHashSet_LockEndRead(me);
		if (dec_putptr((uint32_t)length))
			goto err;
		DEC_RECURSION_BEGIN(self);
		/* Encode all of the set's elements. */
		written = 0;
		DeeHashSet_LockRead(me);
		for (i = 0; i <= me->s_mask; ++i) {
			DREF DeeObject *obj;
			int error;
			obj = me->s_elem[i].si_key;
			if (!obj)
				continue;
			Dee_Incref(obj);
			DeeHashSet_LockEndRead(me);
			if unlikely(written >= length) {
				Dee_Decref(obj);
				break;
			}
			/* Emit the set item. */
			error = dec_putobj(obj);
			Dee_Decref(obj);
			if unlikely(error) {
				DEC_RECURSION_BREAK();
				goto err;
			}
			++written;
			DeeHashSet_LockRead(me);
		}
		DeeHashSet_LockEndRead(me);
		DEC_RECURSION_END();
		/* For anything that couldn't be written, write `none' */
		while unlikely(written < length) {
			if (dec_putb(DTYPE_NONE))
				goto err;
			++written;
		}
		goto done;
	}
	if (tp_self == &DeeDict_Type) {
		size_t i, length, written;
		DeeDictObject *me = (DeeDictObject *)self;
		if (dec_recursion_check(self))
			goto err;
		if (dec_putb((DTYPE16_DICT & 0xff00) >> 8))
			goto err;
		if (dec_putb(DTYPE16_DICT & 0xff))
			goto err;
		DeeDict_LockRead(me);
		length = me->d_used;
		DeeDict_LockEndRead(me);
		if (dec_putptr((uint32_t)length))
			goto err;
		DEC_RECURSION_BEGIN(self);
		/* Encode all of the Dict's elements. */
		written = 0;
		DeeDict_LockRead(me);
		for (i = 0; i <= me->d_mask; ++i) {
			DREF DeeObject *key, *value;
			int error;
			key = me->d_elem[i].di_key;
			if (!key)
				continue;
			value = me->d_elem[i].di_value;
			ASSERT_OBJECT(value);
			Dee_Incref(key);
			Dee_Incref(value);
			DeeDict_LockEndRead(me);
			if unlikely(written >= length) {
				Dee_Decref(value);
				Dee_Decref(key);
				break;
			}
			/* Emit the Dict key + value pair. */
			error = dec_putobj(key);
			if (!error)
				error = dec_putobj(value);
			Dee_Decref(value);
			Dee_Decref(key);
			if unlikely(error) {
				DEC_RECURSION_BREAK();
				goto err;
			}
			++written;
			DeeDict_LockRead(me);
		}
		DeeDict_LockEndRead(me);
		DEC_RECURSION_END();
		/* For anything that couldn't be written, write a `none:none' pair. */
		while unlikely(written < length) {
			if (dec_putb(DTYPE_NONE))
				goto err;
			if (dec_putb(DTYPE_NONE))
				goto err;
			++written;
		}
		goto done;
	}
	if (tp_self == &DeeRoDict_Type) {
		size_t i;
		DeeRoDictObject *me = (DeeRoDictObject *)self;
#ifndef NDEBUG
		size_t num_written = 0;
#endif /* !NDEBUG */
		if (dec_putb((DTYPE16_RODICT & 0xff00) >> 8))
			goto err;
		if (dec_putb(DTYPE16_RODICT & 0xff))
			goto err;
		if (dec_putptr((uint32_t)me->rd_size))
			goto err;
		/* Encode all of the ro-Dict's elements. */
		for (i = 0; i <= me->rd_mask; ++i) {
			int error;
			if (!me->rd_elem[i].di_key)
				continue;
			/* Emit the Dict key + value pair. */
			error = dec_putobj(me->rd_elem[i].di_key);
			if (!error)
				error = dec_putobj(me->rd_elem[i].di_value);
			if unlikely(error)
				goto err;
#ifndef NDEBUG
			++num_written;
#endif /* !NDEBUG */
		}
#ifndef NDEBUG
		ASSERTF(num_written == me->rd_size,
		        "Incorrect number of object written for rodict:\n"
		        "Written  = %Iu\n"
		        "Required = %Iu",
		        num_written, me->rd_size);
#endif /* !NDEBUG */
		goto done;
	}
	if (tp_self == &DeeRoSet_Type) {
		size_t i;
		DeeRoSetObject *me = (DeeRoSetObject *)self;
#ifndef NDEBUG
		size_t num_written = 0;
#endif /* !NDEBUG */
		if (dec_putb((DTYPE16_ROSET & 0xff00) >> 8))
			goto err;
		if (dec_putb(DTYPE16_ROSET & 0xff))
			goto err;
		if (dec_putptr((uint32_t)me->rs_size))
			goto err;
		for (i = 0; i <= me->rs_mask; ++i) {
			if (!me->rs_elem[i].si_key)
				continue;
			/* Emit the set item. */
			if (dec_putobj(me->rs_elem[i].si_key))
				goto err;
#ifndef NDEBUG
			++num_written;
#endif /* !NDEBUG */
		}
#ifndef NDEBUG
		ASSERTF(num_written == me->rs_size,
		        "Incorrect number of object written for roset:\n"
		        "Written  = %Iu\n"
		        "Required = %Iu",
		        num_written, me->rs_size);
#endif /* !NDEBUG */
		goto done;
	}
	if (tp_self == &DeeCell_Type) {
		DREF DeeObject *cell_item;
		int error;
		if (dec_recursion_check(self))
			goto err;
		if (dec_putb((DTYPE16_CELL & 0xff00) >> 8))
			goto err;
		if (dec_putb(DTYPE16_CELL & 0xff))
			goto err;
		cell_item = DeeCell_TryGet(self);
		/* Protect against the potential recursion of cells. */
		DEC_RECURSION_BEGIN(self);
		error = dec_putobj(cell_item);
		DEC_RECURSION_END();
		Dee_XDecref(cell_item);
		return error;
	}
	/* Emit a class descriptor. */
	if (tp_self == &DeeClassDescriptor_Type)
		return dec_putclassdesc((DeeClassDescriptorObject *)self);
	/* Emit a keywords descriptor. */
	if (tp_self == &DeeKwds_Type)
		return dec_putkwds((DeeKwdsObject *)self);
	/* Emit a code object. */
	if (tp_self == &DeeCode_Type) {
		if (dec_putb(DTYPE_CODE))
			goto err;
		return dec_putcode((DeeCodeObject *)self);
	}
	/* Emit a function object. */
	if (tp_self == &DeeFunction_Type) {
		uint16_t i, refc;
		if (dec_putb(DTYPE_FUNCTION))
			goto err;
		if (dec_putcode(((DeeFunctionObject *)self)->fo_code))
			goto err;
		/* Emit referenced objects. */
		refc = ((DeeFunctionObject *)self)->fo_code->co_refc;
		for (i = 0; i < refc; ++i) {
			if (dec_putobj(((DeeFunctionObject *)self)->fo_refv[i]))
				goto err;
		}
		return 0;
	}

	/* Fallback: try to encode a builtin object. */
	builtin_id = Dec_BuiltinID(self);
	if unlikely(builtin_id == DEC_BUILTINID_UNKNOWN)
		goto err_unsupported;
	/* Encode a builtin object ID. */
	if likely(DEC_BUILTINID_SETOF(builtin_id) ==
	          current_dec.dw_objset) {
		/* The object is part of the main object set
		 * >> We can encode it as a single-byte DTYPE. */
		if (dec_putb(DEC_BUILTINID_IDOF(builtin_id)))
			goto err;
	} else {
		/* Not apart of the main object set.
		 * >> Must be encoded as an extended object-set builtin. */
		uint8_t *buffer = dec_alloc(3);
		if unlikely(!buffer)
			goto err;
		buffer[0] = (uint8_t)((DTYPE16_BUILTIN_MIN & 0xff00) >> 8);
		buffer[1] = (uint8_t)(DEC_BUILTINID_SETOF(builtin_id) + (DTYPE16_BUILTIN_MIN & 0xff));
		buffer[2] = (uint8_t)(DEC_BUILTINID_IDOF(builtin_id));
	}
done:
	return 0;
err_unsupported:
	DeeError_Throwf(&DeeError_NotImplemented,
	                "Instance %r or type %k cannot be encoded in a DEC file",
	                self, Dee_TYPE(self));
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) bool DCALL
allow_8bit_code(DeeCodeObject *__restrict self) {
	uint16_t i;
	/* In big-file mode, we can never emit an 8-bit
	 * image default file pointers may (will because
	 * of the text (aka. assembly) pointer) be used. */
	if (current_dec.dw_flags & DEC_WRITE_FBIGFILE)
		goto nope;

	/* Check the bounds on some fields that are more
	 * restrictive if we choose to allow this. */
	if (self->co_localc > UINT8_MAX)
		goto nope;
	if (self->co_refc > UINT8_MAX)
		goto nope;
	if (self->co_argc_min > UINT8_MAX)
		goto nope;
	if (((self->co_framesize / sizeof(DeeObject *)) - self->co_localc) > UINT8_MAX)
		goto nope; /* stack_max */
	if (self->co_codebytes > UINT16_MAX)
		goto nope;
	if (self->co_exceptc > UINT8_MAX)
		goto nope;
	for (i = 0; i < self->co_exceptc; ++i) {
		if (self->co_exceptv[i].eh_start > UINT16_MAX)
			goto nope;
		if (self->co_exceptv[i].eh_end > UINT16_MAX)
			goto nope;
		if (self->co_exceptv[i].eh_addr > UINT16_MAX)
			goto nope;
		if (self->co_exceptv[i].eh_stack > UINT8_MAX)
			goto nope;
	}
	/* Check if DDI information is located in 8-bit bounds. */
	if (!(current_dec.dw_flags & DEC_WRITE_FNODEBUG)) {
		if (self->co_ddi->d_ddisize > UINT16_MAX)
			goto nope;
		if (self->co_ddi->d_ddiinit > UINT8_MAX)
			goto nope;
	}
	if (self->co_keywords) {
		uint16_t i;
		for (i = 0; i < self->co_argc_max; ++i) {
			DeeStringObject *kw = self->co_keywords[i];
			if (strlen(kw->s_str) != kw->s_len)
				goto nope; /* Argument name can't simply be encoded as nul-terminated */
		}
	}
	return true;
nope:
	return false;
}

/* Emit a DDI-compatible string table. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dec_do_putddi_strtab(DeeDDIObject *__restrict self,
                     uint32_t const *__restrict vec,
                     uint32_t length) {
	struct dec_section *ddi_sec;
	uint32_t straddr;
	uint32_t i;
	char *str;
	uint8_t *strptr;
	if (length >= (uint16_t)-1) {
		if (dec_putw(0xffff))
			goto err; /* Dec_Strmap.i_len */
		if (dec_putl(length))
			goto err; /* Dec_Strmap.i_len */
	} else {
		if (dec_putw((uint16_t)length))
			goto err; /* Dec_Strmap.i_len */
	}
	for (i = 0; i < length; ++i) {
		ddi_sec = dec_curr;
		/* Emit the DDI string within the string section. */
		dec_curr = SC_STRING;
		str      = DeeString_STR(self->d_strtab) + vec[i];
		strptr   = dec_allocstr(str, (strlen(str) + 1) * sizeof(char));
		if unlikely(!strptr)
			goto err;
		straddr  = dec_ptr2addr(strptr);
		dec_curr = ddi_sec;
		/* Emit the string pointer within the DDI string vector. */
		if (dec_putptr(straddr))
			goto err;
	}
	return 0;
err:
	return -1;
}

/* Emit a DDI-compatible string table and return a symbol for its starting address. */
PRIVATE struct dec_sym *DCALL
dec_putddi_strtab(DeeDDIObject *__restrict self,
                  uint32_t const *__restrict vec,
                  uint32_t length) {
	struct dec_section *result, *old_sec;
	/* Create a new section within which debug data will be placed. */
	result = dec_newsection_after(SC_DEBUG_DATA);
	if unlikely(!result)
		goto err;
	old_sec  = dec_curr;
	dec_curr = result;
	/* Actually emit the debug data. */
	if unlikely(dec_do_putddi_strtab(self, vec, length))
		goto err;
	/* Switch back to the old section. */
	dec_curr = old_sec;
	/* Return a pointer to a symbol describing the start of the sub-section. */
	return &result->ds_start;
err:
	return NULL;
}

/* Generate a DDI-compatible string table and emit a pointer to it. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dec_putddi_strtab_ptr(DeeDDIObject *__restrict self,
                      uint32_t const *__restrict vec,
                      uint32_t length, bool use_16bit) {
	struct dec_sym *sym;
	if (length) {
		sym = dec_putddi_strtab(self, vec, length);
		if unlikely(!sym)
			goto err;
		if (dec_putrel(use_16bit ? DECREL_ABS16 : DECREL_ABS32, sym))
			goto err;
	} else {
		/* Special case: no table (just emit a NULL-pointer) */
	}
	if (use_16bit ? dec_putw(0) : dec_putl(0))
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
dec_putddi_xdat_ptr(DeeDDIObject *__restrict ddi,
                    struct Dee_ddi_exdat const *self,
                    bool use_16bit) {
	if (self && self->dx_size != 0) {
		uint8_t *buf, *iter, *end;
		size_t size;
		struct dec_section *xsect, *old_sec;
		/* Create a new section within which debug data will be placed. */
		xsect = dec_newsection_after(SC_DEBUG_DATA);
		if unlikely(!xsect)
			goto err;
		old_sec  = dec_curr;
		dec_curr = xsect;
		if (dec_putw(0))
			goto err; /* Overwritten later */
		end = (iter = (uint8_t *)self->dx_data) + self->dx_size;
		while (iter < end) {
			uint8_t op = *iter++;
			size_t opsize;
			switch (op & DDI_EXDAT_OPMASK) {

			case DDI_EXDAT_OP8:
				opsize = 1 + 1;
				break;

			case DDI_EXDAT_OP16:
				opsize = 2 + 2;
				break;

			case DDI_EXDAT_OP32:
				opsize = 2 + 4;
				break;

			default:
				opsize = 0;
				break;
			}
			switch (op & ~DDI_EXDAT_OPMASK) {

			case DDI_EXDAT_O_RNAM:
			case DDI_EXDAT_O_SNAM: {
				uint32_t string_offset;
				uint16_t symbol_id;
				char const *string;
				switch (opsize) {

				case 1 + 1:
					symbol_id     = *(uint8_t *)(iter + 0);
					string_offset = *(uint8_t *)(iter + 1);
					break;

				case 2 + 2:
					symbol_id     = UNALIGNED_GETLE16((uint16_t *)(iter + 0));
					string_offset = UNALIGNED_GETLE16((uint16_t *)(iter + 2));
					break;

				case 2 + 4:
					symbol_id     = UNALIGNED_GETLE16((uint16_t *)(iter + 0));
					string_offset = UNALIGNED_GETLE32((uint32_t *)(iter + 2));
					break;

				default: goto defl_xdat;
				}
				string   = DeeString_STR(ddi->d_strtab) + string_offset;
				dec_curr = SC_STRING;
				string   = (char *)dec_allocstr(string, (strlen(string) + 1) * sizeof(char));
				if unlikely(!string)
					goto err;
				string_offset = dec_ptr2addr((uint8_t *)string);
				dec_curr      = xsect;
				if (string_offset <= UINT8_MAX && symbol_id <= UINT8_MAX) {
					if (dec_putb((op & ~DDI_EXDAT_OPMASK) | DDI_EXDAT_OP8))
						goto err;
					if (dec_putb((uint8_t)symbol_id))
						goto err;
					if (dec_putb((uint8_t)string_offset))
						goto err;
				} else if (string_offset <= UINT16_MAX) {
					if (dec_putb((op & ~DDI_EXDAT_OPMASK) | DDI_EXDAT_OP16))
						goto err;
					if (dec_putw(symbol_id))
						goto err;
					if (dec_putw((uint16_t)string_offset))
						goto err;
				} else {
					if (dec_putb((op & ~DDI_EXDAT_OPMASK) | DDI_EXDAT_OP32))
						goto err;
					if (dec_putw(symbol_id))
						goto err;
					if (dec_putl(string_offset))
						goto err;
				}
			}	break;

			default:
defl_xdat:
				buf = dec_alloc(1 + opsize);
				if unlikely(!buf)
					goto err;
				memcpy(buf, buf - 1, 1 + opsize);
				break;
			}
			iter += opsize;
		}
		size = (size_t)(xsect->ds_iter - xsect->ds_begin) - 2;
		if (size <= UINT16_MAX) {
			UNALIGNED_SETLE16((uint16_t *)xsect->ds_begin, (uint16_t)size);
		} else {
			if (dec_putl(0))
				goto err;
			memmove(xsect->ds_begin + 6,
			        xsect->ds_begin + 2,
			        size);
			UNALIGNED_SET16((uint16_t *)(xsect->ds_begin + 0), 0xffff);
			UNALIGNED_SETLE32((uint32_t *)(xsect->ds_begin + 2), (uint32_t)size);
		}

		/* Switch back to the old section. */
		dec_curr = old_sec;
		if (dec_putrel(use_16bit ? DECREL_ABS16 : DECREL_ABS32, &xsect->ds_start))
			goto err;
	} else {
		/* Special case: no table (just emit a NULL-pointer) */
	}
	if (use_16bit ? dec_putw(0) : dec_putl(0))
		goto err;
	return 0;
err:
	return -1;
}



INTERN int (DCALL dec_putcode)(DeeCodeObject *__restrict self) {
	bool use_8bit;
	Dec_Code descr;
	uint8_t *ptr;
	struct dec_sym *text_sym;
	struct dec_sym *static_sym   = NULL;
	struct dec_sym *except_sym   = NULL;
	struct dec_sym *default_sym  = NULL;
	struct dec_sym *ddi_sym      = NULL;
	struct dec_sym *kwd_sym      = NULL;
	struct dec_section *code_sec = dec_curr;
	/* Fill in a code object descriptor. */
	descr.co_flags      = self->co_flags;
	descr.co_localc     = self->co_localc;
	descr.co_refc       = self->co_refc;
	descr.co_argc_min   = self->co_argc_min;
	descr.co_stackmax   = (uint16_t)(self->co_framesize / sizeof(DeeObject *)) - self->co_localc;
	descr.co_pad        = 0;
	descr.co_staticoff  = 0;
	descr.co_exceptoff  = 0;
	descr.co_defaultoff = 0;
	descr.co_ddioff     = 0;
	descr.co_kwdoff     = 0;
	descr.co_textsiz    = self->co_codebytes;
	descr.co_textoff    = 0;

	/* Figure out if we can emit an 8-bit code image. */
	use_8bit = allow_8bit_code(self);

	/* Set the 8-bit code flag is we're allowed
	 * to encode the code object as 8 bits. */
	if (use_8bit)
		descr.co_flags |= DEC_CODE_F8BIT;

	/* First of all: Allocate the code's text segment. */
	dec_curr = SC_TEXT;
	text_sym = dec_newsym();
	if unlikely(!text_sym)
		goto err;
	ptr = dec_allocstr(self->co_code,
	                   self->co_codebytes);
	if unlikely(!ptr)
		goto err;
	dec_defsymat(text_sym, dec_ptr2addr(ptr));

	if (self->co_staticc) {
		/* Generate the static/constant variable vector. */
		struct dec_section *static_sec;
		static_sec = dec_newsection_after(SC_ROOT);
		if unlikely(!static_sec)
			goto err;
		dec_curr   = static_sec;
		static_sym = &static_sec->ds_start; /* This is where static object data starts. */
		/* Generate static object vector. */
		if (dec_putobjv(self->co_staticc, self->co_staticv))
			goto err;
	}

	if (self->co_exceptc) {
		struct dec_section *except_sec;
		struct except_handler *iter, *end;
		except_sec = dec_newsection_after(SC_ROOT);
		if unlikely(!except_sec)
			goto err;
		dec_curr   = except_sec;
		except_sym = &except_sec->ds_start; /* This is where exception data starts. */
		end        = (iter = self->co_exceptv) + self->co_exceptc;
		/* Generate exception descriptor vector. */
		if (use_8bit) {
			if (dec_putb((uint8_t)self->co_exceptc))
				goto err; /* Dec_8BitCodeExceptions.ces_len */
			for (; iter != end; ++iter) {
				if (dec_putw(iter->eh_flags))
					goto err; /* Dec_8BitCodeExcept.ce_flags */
				if (dec_putw((uint16_t)iter->eh_start))
					goto err; /* Dec_8BitCodeExcept.ce_begin */
				if (dec_putw((uint16_t)iter->eh_end))
					goto err; /* Dec_8BitCodeExcept.ce_end */
				if (dec_putw((uint16_t)iter->eh_addr))
					goto err; /* Dec_8BitCodeExcept.ce_addr */
				if (dec_putb((uint8_t)iter->eh_stack))
					goto err; /* Dec_8BitCodeExcept.ce_stack */
				if (dec_putobj((DeeObject *)iter->eh_mask))
					goto err; /* Dec_8BitCodeExcept.ce_mask */
			}
		} else {
			if (dec_putw(self->co_exceptc))
				goto err; /* Dec_CodeExceptions.ces_len */
			for (; iter != end; ++iter) {
				if (dec_putw(iter->eh_flags))
					goto err; /* Dec_CodeExcept.ce_flags */
				if (dec_putl(iter->eh_start))
					goto err; /* Dec_CodeExcept.ce_begin */
				if (dec_putl(iter->eh_end))
					goto err; /* Dec_CodeExcept.ce_end */
				if (dec_putl(iter->eh_addr))
					goto err; /* Dec_CodeExcept.ce_addr */
				if (dec_putw(iter->eh_stack))
					goto err; /* Dec_CodeExcept.ce_stack */
				if (dec_putobj((DeeObject *)iter->eh_mask))
					goto err; /* Dec_CodeExcept.ce_mask */
			}
		}
	}

	ASSERT(self->co_argc_max >= self->co_argc_min);
	if (self->co_argc_min < self->co_argc_max) {
		/* Create the argument descriptor data. */
		struct dec_section *args_sec;
		args_sec = dec_newsection_after(SC_ROOT);
		if unlikely(!args_sec)
			goto err;
		dec_curr    = args_sec;
		default_sym = &args_sec->ds_start; /* This is where argument data starts. */
		/* Generate default argument data. */
		if (dec_putobjv(self->co_argc_max - self->co_argc_min,
		                (DeeObject **)self->co_defaultv))
			goto err;
	}

	if (!(current_dec.dw_flags & DEC_WRITE_FNODEBUG)) {
		/* Emit debug information if there are some */
		DeeDDIObject *ddi = self->co_ddi;
		/* Don't emit any debug information, if the DDI object doesn't have any text. */
		if (ddi->d_ddisize ||
		    (ddi->d_exdat && ddi->d_exdat->dx_size)) {
			uint8_t *tempptr;
			struct dec_sym *tempsym;
			struct dec_section *ddi_section = dec_newsection_after(SC_DEBUG);
			if unlikely(!ddi_section)
				goto err;
			dec_curr = ddi_section;
			ddi_sym  = &ddi_section->ds_start;
			/* Start emitting the DDI object. */
			if (dec_putddi_strtab_ptr(ddi,
			                          ddi->d_strings,
			                          ddi->d_nstring,
			                          use_8bit))
				goto err; /* Dec_CodeDDI.cd_strings */
			/* Emit the DDI text into the debug-text section. */
			dec_curr = SC_DEBUG_TEXT;
			tempptr  = dec_allocstr(ddi->d_ddi, ddi->d_ddisize);
			if unlikely(!tempptr)
				goto err;
			tempsym = dec_newsym();
			if unlikely(!tempsym)
				goto err;
			dec_defsymat(tempsym, dec_ptr2addr(tempptr));
			dec_curr = ddi_section;
			if unlikely(dec_putddi_xdat_ptr(ddi, ddi->d_exdat, use_8bit))
				goto err; /* Dec_CodeDDI.cd_ddixdat */

			/* Now emit the size of, and a pointer to the DDI's text. */
			if (use_8bit) {
				if (dec_putrel(DECREL_ABS16, tempsym))
					goto err;
				if (dec_putw(0))
					goto err; /* Dec_8BitCodeDDI.cd_ddiaddr */
				if (dec_putw((uint16_t)ddi->d_ddisize))
					goto err; /* Dec_8BitCodeDDI.cd_ddisize */
				if (dec_putb((uint8_t)ddi->d_ddiinit))
					goto err; /* Dec_8BitCodeDDI.cd_ddiinit */
			} else {
				if (dec_putrel(DECREL_ABS32, tempsym))
					goto err;
				if (dec_putl(0))
					goto err; /* Dec_CodeDDI.cd_ddiaddr */
				if (dec_putl(ddi->d_ddisize))
					goto err; /* Dec_CodeDDI.cd_ddisize */
				if (dec_putw(ddi->d_ddiinit))
					goto err; /* Dec_CodeDDI.cd_ddiinit */
			}

			/* Save the initial register state (`cd_regs'). */
			if (dec_putw(ddi->d_start.dr_flags))
				goto err; /* Dec_DDIRegStart.rs_flags */
			if (dec_putuleb(ddi->d_start.dr_uip))
				goto err; /* Dec_DDIRegStart.rs_uip */
			if (dec_putuleb(ddi->d_start.dr_usp))
				goto err; /* Dec_DDIRegStart.rs_usp */
			if (dec_putuleb(ddi->d_start.dr_path))
				goto err; /* Dec_DDIRegStart.rs_path */
			if (dec_putuleb(ddi->d_start.dr_file))
				goto err; /* Dec_DDIRegStart.rs_file */
			if (dec_putuleb(ddi->d_start.dr_name))
				goto err; /* Dec_DDIRegStart.rs_name */
			if (dec_putsleb(ddi->d_start.dr_col))
				goto err; /* Dec_DDIRegStart.rs_col */
			if (dec_putsleb(ddi->d_start.dr_lno))
				goto err; /* Dec_DDIRegStart.rs_lno */
		}
	}

	/* Encode keyword names. */
	if (self->co_keywords != NULL) {
		struct dec_section *kwd_section;
		uint16_t i;
		kwd_section = dec_newsection_after(SC_ROOT);
		if unlikely(!kwd_section)
			goto err;
		kwd_sym = &kwd_section->ds_start;
		if (use_8bit) {
			for (i = 0; i < self->co_argc_max; ++i) {
				uint8_t *name;
				uint32_t addr;
				dec_curr = SC_STRING;
				name     = (uint8_t *)DeeString_AsUtf8((DeeObject *)self->co_keywords[i]);
				if unlikely(!name)
					goto err;
				name = dec_allocstr(name, (WSTR_LENGTH(name) + 1) * sizeof(char));
				if unlikely(!name)
					goto err;
				addr     = dec_ptr2addr(name);
				dec_curr = kwd_section;
				if (dec_putptr(addr))
					goto err;
			}
		} else {
			for (i = 0; i < self->co_argc_max; ++i) {
				uint8_t *name;
				uint32_t addr;
				size_t len;
				name = (uint8_t *)DeeString_AsUtf8((DeeObject *)self->co_keywords[i]);
				if unlikely(!name)
					goto err;
				len = WSTR_LENGTH(name);
				if (len) {
					dec_curr = SC_STRING;
					name     = dec_allocstr(name, len * sizeof(char));
					if unlikely(!name)
						goto err;
					addr     = dec_ptr2addr(name);
					dec_curr = kwd_section;
					if (dec_putptr((uint32_t)len))
						goto err;
					if (dec_putptr(addr))
						goto err;
				} else {
					dec_curr = kwd_section; /* Unnamed */
					if (dec_putptr(0))
						goto err;
				}
			}
		}
	}

	dec_curr = code_sec;
	/* Finally, emit the code object itself! */
	if (use_8bit) {
		Dec_8BitCode *pdesc8;
		uint32_t code_addr = dec_addr;
		/* Allocate and copy descriptor data. */
		pdesc8 = (Dec_8BitCode *)dec_alloc(sizeof(Dec_8BitCode));
		if unlikely(!pdesc8)
			goto err;
		UNALIGNED_SETLE16(&pdesc8->co_flags, descr.co_flags);                     /* Dec_Code.co_flags */
		pdesc8->co_localc   = (uint8_t)descr.co_localc;                           /* Dec_Code.co_localc */
		pdesc8->co_refc     = (uint8_t)descr.co_refc;                             /* Dec_Code.co_refc */
		pdesc8->co_argc_min = (uint8_t)descr.co_argc_min;                         /* Dec_Code.co_argc_min */
		pdesc8->co_stackmax = (uint8_t)descr.co_stackmax;                         /* Dec_Code.co_stackmax */
		UNALIGNED_SETLE16(&pdesc8->co_staticoff, (uint16_t)descr.co_staticoff);   /* Dec_Code.co_staticoff */
		UNALIGNED_SETLE16(&pdesc8->co_exceptoff, (uint16_t)descr.co_exceptoff);   /* Dec_Code.co_exceptoff */
		UNALIGNED_SETLE16(&pdesc8->co_defaultoff, (uint16_t)descr.co_defaultoff); /* Dec_Code.co_defaultoff */
		UNALIGNED_SETLE16(&pdesc8->co_ddioff, (uint16_t)descr.co_ddioff);         /* Dec_Code.co_ddioff */
		UNALIGNED_SETLE16(&pdesc8->co_kwdoff, (uint16_t)descr.co_kwdoff);         /* Dec_Code.co_ddioff */
		UNALIGNED_SETLE16(&pdesc8->co_textsiz, (uint16_t)descr.co_textsiz);       /* Dec_Code.co_textsiz */
		UNALIGNED_SETLE16(&pdesc8->co_textoff, (uint16_t)descr.co_textoff);       /* Dec_Code.co_textoff */

		/* Create relocations. */
		if (static_sym && dec_putrelat(code_addr + offsetof(Dec_8BitCode, co_staticoff), DECREL_ABS16, static_sym))
			goto err;
		if (except_sym && dec_putrelat(code_addr + offsetof(Dec_8BitCode, co_exceptoff), DECREL_ABS16, except_sym))
			goto err;
		if (default_sym && dec_putrelat(code_addr + offsetof(Dec_8BitCode, co_defaultoff), DECREL_ABS16, default_sym))
			goto err;
		if (ddi_sym && dec_putrelat(code_addr + offsetof(Dec_8BitCode, co_ddioff), DECREL_ABS16, ddi_sym))
			goto err;
		if (kwd_sym && dec_putrelat(code_addr + offsetof(Dec_8BitCode, co_kwdoff), DECREL_ABS16, kwd_sym))
			goto err;
		if (dec_putrelat(code_addr + offsetof(Dec_8BitCode, co_textoff), DECREL_ABS16, text_sym))
			goto err;
	} else {
		Dec_Code *pdesc;
		uint32_t code_addr = dec_addr;
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
		/* Convert endian before writing data. */
		descr.co_flags      = HTOLE16(descr.co_flags);      /* Dec_Code.co_flags */
		descr.co_localc     = HTOLE16(descr.co_localc);     /* Dec_Code.co_localc */
		descr.co_refc       = HTOLE16(descr.co_refc);       /* Dec_Code.co_refc */
		descr.co_argc_min   = HTOLE16(descr.co_argc_min);   /* Dec_Code.co_argc_min */
		descr.co_stackmax   = HTOLE16(descr.co_stackmax);   /* Dec_Code.co_stackmax */
		descr.co_staticoff  = HTOLE32(descr.co_staticoff);  /* Dec_Code.co_staticoff */
		descr.co_exceptoff  = HTOLE32(descr.co_exceptoff);  /* Dec_Code.co_exceptoff */
		descr.co_defaultoff = HTOLE32(descr.co_defaultoff); /* Dec_Code.co_defaultoff */
		descr.co_ddioff     = HTOLE32(descr.co_ddioff);     /* Dec_Code.co_ddioff */
		descr.co_kwdoff     = HTOLE32(descr.co_kwdoff);     /* Dec_Code.co_kwdoff */
		descr.co_textsiz    = HTOLE32(descr.co_textsiz);    /* Dec_Code.co_textsiz */
		descr.co_textoff    = HTOLE32(descr.co_textoff);    /* Dec_Code.co_textoff */
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
		/* Allocate and copy descriptor data. */
		pdesc = (Dec_Code *)dec_alloc(sizeof(Dec_Code));
		if unlikely(!pdesc)
			goto err;
		memcpy(pdesc, &descr, sizeof(Dec_Code));
		/* Create relocations. */
		if (static_sym) {
			if (dec_putrelat(code_addr + offsetof(Dec_Code, co_staticoff), DECREL_ABS32, static_sym))
				goto err;
		}
		if (except_sym) {
			if (dec_putrelat(code_addr + offsetof(Dec_Code, co_exceptoff), DECREL_ABS32, except_sym))
				goto err;
		}
		if (default_sym) {
			if (dec_putrelat(code_addr + offsetof(Dec_Code, co_defaultoff), DECREL_ABS32, default_sym))
				goto err;
		}
		if (ddi_sym) {
			if (dec_putrelat(code_addr + offsetof(Dec_Code, co_ddioff), DECREL_ABS32, ddi_sym))
				goto err;
		}
		if (kwd_sym) {
			if (dec_putrelat(code_addr + offsetof(Dec_Code, co_kwdoff), DECREL_ABS32, kwd_sym))
				goto err;
		}
		if (dec_putrelat(code_addr + offsetof(Dec_Code, co_textoff), DECREL_ABS32, text_sym))
			goto err;
	}
	return 0;
err:
	return -1;
}



/* Generate DEC code for compiling the given module.
 * @throw NotImplemented Somewhere during generation process, a constant
 *                       could not be encoded a DTYPE expression.
 * @return:  0: Successfully populated DEC sections.
 * @return: -1: An error occurred. */
INTERN int (DCALL dec_generate)(DeeModuleObject *__restrict self) {
	Dec_Ehdr *header;
	ASSERT_OBJECT_TYPE(self, &DeeModule_Type);
	dec_curr = SC_HEADER;
	header   = (Dec_Ehdr *)dec_alloc(sizeof(Dec_Ehdr));
	if unlikely(!header)
		goto err;

	/* Initialize the magic header. */
	header->e_ident[DI_MAG0] = DECMAG0;
	header->e_ident[DI_MAG1] = DECMAG1;
	header->e_ident[DI_MAG2] = DECMAG2;
	header->e_ident[DI_MAG3] = DECMAG3;
	header->e_builtinset     = current_dec.dw_objset;
	header->e_size           = sizeof(Dec_Ehdr);
	header->e_version        = DVERSION_CUR;
	header->e_impoff         = 0;
	header->e_depoff         = 0;
	header->e_globoff        = 0;
	header->e_rootoff        = 0;
	header->e_stroff         = 0;
	header->e_strsiz         = 0;

	{
		uint64_t comtm;
		comtm = DeeModule_GetCTime((DeeObject *)self);
		if unlikely(comtm == (uint64_t)-1)
			goto err;
		/* Fill in the original timestamp of when the module was compiled. */
		header->e_timestamp_lo = HTOLE32((uint32_t)comtm);
		header->e_timestamp_hi = HTOLE32((uint32_t)(comtm >> 32));
	}

	/* Create initial relocations against other sections. */
	if (dec_putrelat(offsetof(Dec_Ehdr, e_impoff), DECREL_ABS32_NULL, &SC_IMPORTS->ds_start))
		goto err;
	if (dec_putrelat(offsetof(Dec_Ehdr, e_depoff), DECREL_ABS32_NULL, &SC_DEPS->ds_start))
		goto err;
	if (dec_putrelat(offsetof(Dec_Ehdr, e_globoff), DECREL_ABS32_NULL, &SC_GLOBALS->ds_start))
		goto err;
	if (dec_putrelat(offsetof(Dec_Ehdr, e_rootoff), DECREL_ABS32, &SC_ROOT->ds_start))
		goto err;
	if (dec_putrelat(offsetof(Dec_Ehdr, e_stroff), DECREL_ABS32, &SC_STRING->ds_start))
		goto err;
	if (dec_putrelat(offsetof(Dec_Ehdr, e_strsiz), DECREL_SIZE32, &SC_STRING->ds_start))
		goto err;

	/* Save the module import table. */
	if unlikely(decgen_imports(self))
		goto err;

	/* TODO: Save additional dependencies (files that were included by the source file of this module) */

	/* Save the global variable table. */
	if unlikely(decgen_globals(self))
		goto err;

	/* Emit the root code object. */
	dec_curr = SC_ROOT;
	if likely(self->mo_root) {
		if unlikely(dec_putcode(self->mo_root))
			goto err;
	} else {
		/* Shouldn't happen, but the specs allow `mo_root' to be NULL. */
		if unlikely(dec_putcode(&empty_code))
			goto err;
	}

	return 0;
err:
	return -1;
}


DECL_END

#endif /* !CONFIG_NO_DEC */

#endif /* !GUARD_DEEMON_COMPILER_GENDEC_C */
