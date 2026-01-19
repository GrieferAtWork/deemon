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
#ifndef GUARD_DEX_FS_PRINTINSTR_C
#define GUARD_DEX_FS_PRINTINSTR_C 1
#define DEE_SOURCE

#include "libdisasm.h"
/**/

#include <deemon/api.h>

#include <deemon/asm.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* strlen(), ... */

#include <hybrid/byteswap.h>
#include <hybrid/minmax.h>
#include <hybrid/typecore.h>
#include <hybrid/unaligned.h>

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* UINT16_MAX, int8_t, int16_t, int32_t, uint8_t, uint16_t, uint32_t */

DECL_BEGIN

#ifndef UINT16_MAX
#define UINT16_MAX __UINT16_MAX__
#endif /* !UINT16_MAX */

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

#ifndef CONFIG_HAVE_strstr
#define CONFIG_HAVE_strstr
#undef strstr
#define strstr dee_strstr
LOCAL WUNUSED NONNULL((1, 2)) char *
dee_strstr(char const *haystack, char const *needle) {
	char ch, needle_start = *needle++;
	while ((ch = *haystack++) != '\0') {
		if (ch == needle_start) {
			char const *hay2, *ned_iter;
			hay2     = haystack;
			ned_iter = needle;
			while ((ch = *ned_iter++) != '\0') {
				if (*hay2++ != ch)
					goto miss;
			}
			return (char *)haystack - 1;
		}
miss:
		;
	}
	return NULL;
}
#endif /* !CONFIG_HAVE_strstr */

#define PREFIX_VARNAME  "@" /* Prefix for variable names. */
#define PREFIX_CONSTANT "@" /* Prefix for constant expressions. */

PRIVATE Dee_ssize_t DCALL
libdisasm_printconst(Dee_formatprinter_t printer, void *arg,
                     uint16_t cid, DeeCodeObject *code,
                     unsigned int flags) {
	if (code) {
		DeeObject *constval;
		if (cid >= code->co_constc) {
			if (flags & PCODE_FNOBADCOMMENT)
				goto print_generic;
			return DeeFormat_Printf(printer, arg, "const %u /* invalid cid */", (unsigned int)cid);
		}
		constval = code->co_constv[cid];
		if (DeeInt_Check(constval)) {
			Dee_ssize_t temp, result = 0;
			unsigned int numsys;
			uint32_t value;
			temp = (*printer)(arg, PREFIX_CONSTANT, COMPILER_STRLEN(PREFIX_CONSTANT));
			if unlikely(temp < 0)
				return temp;
			result += temp;
			numsys = 16;
			if (((DeeIntObject *)constval)->ob_size < 0 ||
			    (DeeInt_TryAsUInt32(constval, &value) && value <= UINT16_MAX))
				numsys = 10;
			temp = DeeInt_Print(constval,
			                    DEEINT_PRINT(numsys, DEEINT_PRINT_FNUMSYS),
			                    0, printer, arg);
			if unlikely(temp < 0)
				return temp;
			result += temp;
			return result;
		}
		/* TODO: Add the names of code/function/class objects as comments. */
		if (!DeeCode_CheckExact(constval) &&
		    !DeeFunction_CheckExact(constval) &&
		    !DeeClassDescriptor_Check(constval))
			return DeeFormat_Printf(printer, arg, PREFIX_CONSTANT "%r", constval);
	}
print_generic:
	return DeeFormat_Printf(printer, arg, "const %u", (unsigned int)cid);
}

PRIVATE Dee_ssize_t DCALL
libdisasm_printstatic(Dee_formatprinter_t printer, void *arg, uint16_t sid,
                      DeeCodeObject *code, unsigned int flags) {
	if (code) {
		char const *name;
		if ((name = DeeCode_GetRSymbolName(Dee_AsObject(code), sid)) != NULL)
			return DeeFormat_Printf(printer, arg, "static " PREFIX_VARNAME "%s", name);
		if (sid >= code->co_constc) {
			if (flags & PCODE_FNOBADCOMMENT)
				goto print_generic;
			return DeeFormat_Printf(printer, arg, "static %u /* invalid sid */", (unsigned int)sid);
		}
#if 0
		if (readonly && !(flags & PCODE_FNOARGCOMMENT)) {
			DREF DeeObject *init;
			DeeCode_ConstLockRead(code);
			init = code->co_constv[sid];
			Dee_Incref(init);
			DeeCode_ConstLockEndRead(code);
			return DeeFormat_Printf(printer, arg, "static %u /* %R */", sid, init);
		}
#endif
	}
print_generic:
	return DeeFormat_Printf(printer, arg, "static %u", (unsigned int)sid);
}

PRIVATE Dee_ssize_t DCALL
libdisasm_printlocal(Dee_formatprinter_t printer, void *arg,
                     uint16_t lid, struct ddi_state *ddi,
                     DeeCodeObject *code, unsigned int flags) {
	if (code) {
		/* Use DDI information to lookup the name of the variable. */
		if (ddi) {
			struct ddi_xregs *iter;
			DDI_STATE_DO(iter, ddi) {
				if (lid < iter->dx_lcnamc) {
					char const *name;
					if ((name = DeeCode_GetDDIString(Dee_AsObject(code), iter->dx_lcnamv[lid])) != NULL)
						return DeeFormat_Printf(printer, arg, "local " PREFIX_VARNAME "%s", name);
				}
			}
			DDI_STATE_WHILE(iter, ddi);
		}
		if (lid >= code->co_localc) {
			if (flags & PCODE_FNOBADCOMMENT)
				goto print_generic;
			return DeeFormat_Printf(printer, arg, "local %u /* invalid lid */", (unsigned int)lid);
		}
	}
print_generic:
	return DeeFormat_Printf(printer, arg, "local %u", (unsigned int)lid);
}

PRIVATE Dee_ssize_t DCALL
libdisasm_printstack(Dee_formatprinter_t printer, void *arg,
                     uint16_t soff, bool is_prefix, struct ddi_state *ddi,
                     DeeCodeObject *code, unsigned int flags) {
	if (code) {
		if (ddi) {
			/* Use DDI information to lookup the name of the variable. */
			struct ddi_xregs *iter;
			DDI_STATE_DO(iter, ddi) {
				if (soff < MIN(iter->dx_base.dr_usp, iter->dx_spnama)) {
					char const *name;
					if ((name = DeeCode_GetDDIString(Dee_AsObject(code), iter->dx_spnamv[soff])) != NULL)
						return DeeFormat_Printf(printer, arg, "stack " PREFIX_VARNAME "%s", name);
				}
			}
			DDI_STATE_WHILE(iter, ddi);
		}
		if (soff >= DeeCode_StackDepth(code)) {
			if (flags & PCODE_FNOBADCOMMENT)
				goto print_generic;
			return DeeFormat_Printf(printer, arg, "%s%" PRFu16 " /* invalid stack-offset */",
			                        is_prefix ? "stack #" : "#", soff);
		}
	}
print_generic:
	return DeeFormat_Printf(printer, arg, "%s%" PRFu16,
	                        is_prefix ? "stack #" : "#", soff);
}

PRIVATE Dee_ssize_t DCALL
libdisasm_printrelstack(Dee_formatprinter_t printer, void *arg,
                        uint16_t stacksz, uint32_t sp_sub,
                        struct ddi_state *ddi, DeeCodeObject *code,
                        unsigned int flags) {
	uint16_t soff;
	if (!sp_sub)
		goto invalid_offset;
	if (stacksz == (uint16_t)-1) {
print_generic:
		return DeeFormat_Printf(printer, arg, "SP - %" PRFu32, sp_sub);
	}
	if (sp_sub > (uint32_t)stacksz)
		goto invalid_offset;
	soff = stacksz - (uint16_t)sp_sub;
	if (code) {
		if (soff >= DeeCode_StackDepth(code))
			goto invalid_offset;
		/* Use DDI information to lookup the name of the variable. */
		if (ddi) {
			struct ddi_xregs *iter;
			DDI_STATE_DO(iter, ddi) {
				if (soff < MIN(iter->dx_base.dr_usp, iter->dx_spnama)) {
					char const *name;
					if ((name = DeeCode_GetDDIString(Dee_AsObject(code), iter->dx_spnamv[soff])) != NULL) {
						if (!(flags & PCODE_FALTCOMMENT))
							return DeeFormat_Printf(printer, arg, "stack " PREFIX_VARNAME "%s", name);
						return DeeFormat_Printf(printer, arg,
						                        "stack " PREFIX_VARNAME "%s "
						                        "/* #%" PRFu16 " / #SP - %" PRFu32 " */",
						                        name, soff, sp_sub);
					}
				}
			}
			DDI_STATE_WHILE(iter, ddi);
		}
	}
	if (!(flags & PCODE_FALTCOMMENT))
		return DeeFormat_Printf(printer, arg, "#%" PRFu16, soff);
	return DeeFormat_Printf(printer, arg, "#%" PRFu16 " /* #SP - %" PRFu32 " */", soff, sp_sub);
invalid_offset:
	if (flags & PCODE_FNOBADCOMMENT)
		goto print_generic;
	return DeeFormat_Printf(printer, arg, "#SP - %" PRFu32 " /* invalid stack-offset */", sp_sub);
}

PRIVATE Dee_ssize_t DCALL
libdisasm_printglobal(Dee_formatprinter_t printer, void *arg,
                      uint16_t gid, DeeCodeObject *code,
                      unsigned int flags) {
	if (code) {
		char const *name;
		if (gid >= code->co_module->mo_globalc) {
			if (flags & PCODE_FNOBADCOMMENT)
				goto print_generic;
			return DeeFormat_Printf(printer, arg, "global %u /* invalid gid */", (unsigned int)gid);
		}
		name = DeeModule_GlobalName(code->co_module, gid);
		if (name)
			return DeeFormat_Printf(printer, arg, "global " PREFIX_VARNAME "%s", name);
	}
print_generic:
	return DeeFormat_Printf(printer, arg, "global %u", (unsigned int)gid);
}

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
print_module_name(Dee_formatprinter_t printer, void *arg,
                  DeeModuleObject *__restrict mod) {
	Dee_ssize_t result;
	DREF DeeObject *libname = DeeModule_GetLibName(mod, 0);
	if (ITER_ISOK(libname)) {
		result = DeeString_PrintUtf8(libname, printer, arg);
		Dee_Decref_unlikely(libname);
	} else if unlikely(!libname) {
		result = -1;
	} else if (mod->mo_absname)  {
		result = DeeFormat_Printf(printer, arg, "%q", mod->mo_absname);
	} else {
		result = DeeFormat_PRINT(printer, arg, "<anonymous module>");
	}
	return result;
}
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
#define print_module_name(printer, arg, mod) \
	DeeString_PrintUtf8((DeeObject *)(mod)->mo_name, printer, arg)
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
libdisasm_printmodule(Dee_formatprinter_t printer, void *arg,
                      uint16_t mid, DeeCodeObject *code,
                      unsigned int flags) {
	if (code) {
		Dee_ssize_t result, temp;
		DeeModuleObject *mod;
		if (mid >= code->co_module->mo_importc) {
			if (flags & PCODE_FNOBADCOMMENT)
				goto print_generic;
			return DeeFormat_Printf(printer, arg, "module %u /* invalid mid */", (unsigned int)mid);
		}
		mod = code->co_module->mo_importv[mid];
		result = DeeFormat_PRINT(printer, arg, "module " PREFIX_VARNAME);
		if likely(result >= 0) {
			temp = print_module_name(printer, arg, mod);
			if unlikely(temp < 0) {
				result = temp;
			} else {
				result += temp;
			}
		}
		return result;
	}
print_generic:
	return DeeFormat_Printf(printer, arg, "module %u", (unsigned int)mid);
}

PRIVATE Dee_ssize_t DCALL
print_extern_symbol(Dee_formatprinter_t printer, void *arg,
                    DeeModuleObject *mod, struct module_symbol *symbol,
                    char const *suffix) {
	Dee_ssize_t result, temp;
	result = DeeFormat_PRINT(printer, arg, "extern " PREFIX_VARNAME);
	if unlikely(result < 0)
		return result;
	temp = print_module_name(printer, arg, mod);
	if unlikely(temp < 0)
		return temp;
	result += temp;
	if (suffix) {
		temp = DeeFormat_Printf(printer, arg, ":" PREFIX_VARNAME "%s+%s", symbol->ss_name, suffix);
	} else {
		temp = DeeFormat_Printf(printer, arg, ":" PREFIX_VARNAME "%s", symbol->ss_name);
	}
	if unlikely(temp < 0)
		return temp;
	result += temp;
	return result;
}

PRIVATE Dee_ssize_t DCALL
libdisasm_printextern(Dee_formatprinter_t printer, void *arg,
                      uint16_t mid, uint16_t gid,
                      DeeCodeObject *code, unsigned int flags) {
	if (code) {
		struct module_symbol *symbol;
		DeeModuleObject *module;
		if (mid >= code->co_module->mo_importc) {
			if (flags & PCODE_FNOBADCOMMENT)
				goto print_generic;
			return DeeFormat_Printf(printer, arg, "extern %u:%u /* invalid mid */", (unsigned int)mid, (unsigned int)gid);
		}
		module = code->co_module->mo_importv[mid];
		if (gid >= module->mo_globalc) {
			Dee_ssize_t result, temp;
			if (flags & PCODE_FNOBADCOMMENT)
				goto print_unknown_name;
			result = DeeFormat_PRINT(printer, arg, "extern " PREFIX_VARNAME);
			if unlikely(result < 0)
				return result;
			temp = print_module_name(printer, arg, module);
			if unlikely(temp < 0)
				return temp;
			result += temp;
			temp = DeeFormat_Printf(printer, arg, ":%u /* invalid gid */", (unsigned int)gid);
			if unlikely(temp < 0)
				return temp;
			result += temp;
			return result;
		}
		symbol = DeeModule_GetSymbolID(module, gid);
		if (symbol) {
			char const *suffix = (symbol->ss_flags & MODSYM_FPROPERTY) ? "getter" : NULL;
			return print_extern_symbol(printer, arg, module, symbol, suffix);
		}
		if (gid >= MODULE_PROPERTY_DEL) {
			symbol = DeeModule_GetSymbolID(module, gid - MODULE_PROPERTY_DEL);
			if (symbol && (symbol->ss_flags & (MODSYM_FPROPERTY | MODSYM_FREADONLY)) == MODSYM_FPROPERTY)
				return print_extern_symbol(printer, arg, module, symbol, "delete");
			if (gid >= MODULE_PROPERTY_SET) {
				symbol = DeeModule_GetSymbolID(module, gid - MODULE_PROPERTY_SET);
				if (symbol && (symbol->ss_flags & (MODSYM_FPROPERTY | MODSYM_FREADONLY)) == MODSYM_FPROPERTY)
					return print_extern_symbol(printer, arg, module, symbol, "setter");
			}
		}
		{
			Dee_ssize_t result, temp;
print_unknown_name:
			result = DeeFormat_PRINT(printer, arg, "extern " PREFIX_VARNAME);
			if unlikely(result < 0)
				return result;
			temp = print_module_name(printer, arg, module);
			if unlikely(temp < 0)
				return temp;
			result += temp;
			temp = DeeFormat_Printf(printer, arg, ":%u", (unsigned int)gid);
			if unlikely(temp < 0)
				return temp;
			result += temp;
			return result;
		}
	}
print_generic:
	return DeeFormat_Printf(printer, arg, "extern %u:%u", (unsigned int)mid, (unsigned int)gid);
}

PRIVATE Dee_ssize_t DCALL
libdisasm_printref(Dee_formatprinter_t printer, void *arg,
                   uint16_t rid, DeeCodeObject *code,
                   unsigned int flags) {
	if (code) {
		char const *name;
		if ((name = DeeCode_GetRSymbolName(Dee_AsObject(code), rid)) != NULL)
			return DeeFormat_Printf(printer, arg, "ref " PREFIX_VARNAME "%s", name);
		if (rid >= code->co_refc) {
			if (flags & PCODE_FNOBADCOMMENT)
				goto print_generic;
			return DeeFormat_Printf(printer, arg, "ref %u /* invalid rid */", (unsigned int)rid);
		}
	}
print_generic:
	return DeeFormat_Printf(printer, arg, "ref %u", (unsigned int)rid);
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeClassDescriptorObject *DCALL
find_class_descriptor_in_constants(DeeCodeObject *__restrict code,
                                   char const *__restrict class_name) {
	DREF DeeClassDescriptorObject *result;
	uint16_t i;
	bool has_child_code = false;
	for (i = 0; i < code->co_constc; ++i) {
		result = (DeeClassDescriptorObject *)code->co_constv[i];
		if (!DeeClassDescriptor_Check(result)) {
			if (DeeCode_Check(result))
				has_child_code = true;
			continue;
		}
		if (!result->cd_name)
			continue;
		if (strcmp(DeeString_STR(result->cd_name), class_name) != 0)
			continue;
		/* Found it! */
		return result;
	}
	if (has_child_code) {
		/* Also search child-code nodes. */
		DeeCodeObject *child_code;
		for (i = 0; i < code->co_constc; ++i) {
			child_code = (DeeCodeObject *)code->co_constv[i];
			if (!DeeCode_Check(child_code))
				continue;
			result = find_class_descriptor_in_constants(child_code, class_name);
			if (result)
				return result;
		}
	}
	return NULL;
}


PRIVATE Dee_ssize_t DCALL
libdisasm_printmembername(Dee_formatprinter_t printer, void *arg,
                          uint16_t rid, uint16_t mid,
                          DeeCodeObject *code,
                          unsigned int flags,
                          bool is_cmember) {
	(void)flags;
	if (code) {
		char const *class_name;
		class_name = DeeCode_GetRSymbolName(Dee_AsObject(code), rid);
		if (class_name) {
			DeeModuleObject *mod = code->co_module;
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
			if (!DeeInteractiveModule_Check(code->co_module))
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
			{
				struct module_symbol *class_sym;
				class_sym = DeeModule_GetSymbolString(code->co_module, class_name);
				if (!class_sym)
					goto search_module_root_constants;
				if (Dee_module_symbol_getindex(class_sym) < mod->mo_globalc &&
				    !(class_sym->ss_flags & (MODSYM_FPROPERTY | MODSYM_FEXTERN))) {
					DREF DeeObject *class_type;
					DeeClassDescriptorObject *desc;
					DeeModule_LockRead(mod);
					class_type = mod->mo_globalv[Dee_module_symbol_getindex(class_sym)];
					Dee_XIncref(class_type);
					DeeModule_LockEndRead(mod);
					if (!class_type) {
						DREF DeeCodeObject *root;
search_module_root_constants:
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
						root = DeeModule_GetRootCode(mod);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
						DeeModule_LockRead(mod);
						root = mod->mo_root;
						Dee_XIncref(root);
						DeeModule_LockEndRead(mod);
						if (root)
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
						{
							desc = find_class_descriptor_in_constants(root, class_name);
							Dee_Decref(root);
							if (desc) {
								Dee_Incref(desc);
								class_type = Dee_AsObject(desc); /* Inherit reference. (will be dropped later) */
								goto do_search_desc;
							}
						}
					} else {
						if (DeeType_Check(class_type) &&
						    DeeType_IsClass(class_type)) {
							struct class_attribute *attr;
							desc = DeeClass_DESC(class_type)->cd_desc;
do_search_desc:
							if (mid < (is_cmember ? desc->cd_cmemb_size : desc->cd_imemb_size)) {
								size_t i;
								Dee_ssize_t result;
								if (desc->cd_name) {
									class_name = DeeString_STR(desc->cd_name);
								} else if (DeeType_Check(class_type) &&
								           ((DeeTypeObject *)class_type)->tp_name) {
									class_name = ((DeeTypeObject *)class_type)->tp_name;
								}
								if (is_cmember) {
									for (i = 0; i <= desc->cd_cattr_mask; ++i) {
										attr = &desc->cd_cattr_list[i];
										if (!attr->ca_name)
											continue;
										if (mid < attr->ca_addr)
											continue;
										if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
											if (mid > ((attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
											           ? attr->ca_addr
											           : attr->ca_addr + 2))
												continue;
											/* Found it! */
											goto found_it_property;
										} else {
											if (mid > attr->ca_addr)
												continue;
											goto found_it_member;
										}
									}
								}
								for (i = 0; i <= desc->cd_iattr_mask; ++i) {
									attr = &desc->cd_iattr_list[i];
									if (!attr->ca_name)
										continue;
									if (is_cmember
									    ? (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) == 0
									    : (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) != 0)
										continue;
									if (mid < attr->ca_addr)
										continue;
									if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
										if (mid > ((attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
										           ? attr->ca_addr
										           : attr->ca_addr + 2))
											continue;
										/* Found it! */
found_it_property:
										result = DeeFormat_Printf(printer, arg, "%s.%k.%s",
										                          class_name, attr->ca_name,
										                          mid == attr->ca_addr + CLASS_GETSET_GET ? "getter" : mid == attr->ca_addr + CLASS_GETSET_DEL ? "delete" : "setter");
									} else {
										if (mid > attr->ca_addr)
											continue;
found_it_member:
										result = DeeFormat_Printf(printer, arg, "%s.%k",
										                          class_name, attr->ca_name);
									}
									Dee_Decref(class_type);
									return result;
								}
							}
						}
						Dee_Decref(class_type);
					}
				}
			}
		}
	}
	return DeeFormat_Printf(printer, arg, "%" PRFu16, mid);
}

PRIVATE Dee_ssize_t DCALL
libdisasm_printarg(Dee_formatprinter_t printer, void *arg,
                   uint16_t aid, DeeCodeObject *code,
                   unsigned int flags) {
	if (code) {
		char const *name;
		if ((name = DeeCode_GetASymbolName(Dee_AsObject(code), aid)) != NULL)
			return DeeFormat_Printf(printer, arg, "arg " PREFIX_VARNAME "%s", name);
		if (aid >= code->co_argc_max && !(flags & PCODE_FNOBADCOMMENT))
			return DeeFormat_Printf(printer, arg, "arg %u /* invalid aid */", (unsigned int)aid);
	}
	return DeeFormat_Printf(printer, arg, "arg %u", (unsigned int)aid);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
libdisasm_printprefix(Dee_formatprinter_t printer, void *arg,
                      instruction_t *__restrict instr_start,
                      struct ddi_state *ddi, DeeCodeObject *code,
                      unsigned int flags) {
	uint16_t opcode;
	instruction_t *iter = instr_start;
	uint16_t imm, imm2;
	opcode = *iter++;
	if (ASM_ISEXTENDED(opcode))
		opcode = (opcode << 8) | *iter++;
	switch (opcode) {

	case ASM16_STACK:
		imm = *(uint16_t *)(iter + 0);
		goto do_stack_prefix;
	case ASM_STACK:
		imm = *(uint8_t *)(iter + 0);
do_stack_prefix:
		return libdisasm_printstack(printer, arg, imm, true, ddi, code, flags);

	case ASM16_STATIC:
		imm = *(uint16_t *)(iter + 0);
		goto do_static_prefix;
	case ASM_STATIC:
		imm = *(uint8_t *)(iter + 0);
do_static_prefix:
		return libdisasm_printstatic(printer, arg, imm, code, flags);

	case ASM16_GLOBAL:
		imm = *(uint16_t *)(iter + 0);
		goto do_global_prefix;
	case ASM_GLOBAL:
		imm = *(uint8_t *)(iter + 0);
do_global_prefix:
		return libdisasm_printglobal(printer, arg, imm, code, flags);

	case ASM16_EXTERN:
		imm  = *(uint16_t *)(iter + 0);
		imm2 = *(uint16_t *)(iter + 2);
		goto do_extern_prefix;
	case ASM_EXTERN:
		imm  = *(uint8_t *)(iter + 0);
		imm2 = *(uint8_t *)(iter + 1);
do_extern_prefix:
		return libdisasm_printextern(printer, arg, imm, imm2, code, flags);

	case ASM16_LOCAL:
		imm = *(uint16_t *)(iter + 0);
		goto do_local_prefix;
	case ASM_LOCAL:
		imm = *(uint8_t *)(iter + 0);
do_local_prefix:
		return libdisasm_printlocal(printer, arg, imm, ddi, code, flags);

	default: break;
	}
	return 0;
}


INTERN WUNUSED NONNULL((1)) Dee_ssize_t DCALL
libdisasm_printlabel(Dee_formatprinter_t printer, void *arg,
                     uint16_t opcode, code_addr_t source,
                     code_addr_t target) {
	char const *op_name = "";
	switch (opcode) {

	case ASM32_JMP:
		op_name = "jmp32_";
		break;

	case ASM_JF16:
		op_name = "jf16_";
		break;

	case ASM_JT16:
		op_name = "jt16_";
		break;

	case ASM_JMP16:
		op_name = "jmp16_";
		break;

	case ASM_FOREACH16:
		op_name = "fe16_";
		break;

	case ASM_FOREACH_KEY16:
		op_name = "fek16_";
		break;

	case ASM_FOREACH_VALUE16:
		op_name = "fev16_";
		break;

	case ASM_FOREACH_PAIR16:
		op_name = "fep16_";
		break;

	case ASM_JF:
		op_name = "jf_";
		break;

	case ASM_JT:
		op_name = "jt_";
		break;

	case ASM_JMP:
		op_name = "jmp_";
		break;

	case ASM_FOREACH:
		op_name = "fe_";
		break;

	case ASM_FOREACH_KEY:
		op_name = "fek_";
		break;

	case ASM_FOREACH_VALUE:
		op_name = "fev_";
		break;

	case ASM_FOREACH_PAIR:
		op_name = "fep_";
		break;

	default: break;
	}
	return DeeFormat_Printf(printer, arg,
	                        ".L%s%.4" PRFX32 "_%.4" PRFX32,
	                        op_name, source, target);
}


#if 0
PRIVATE char const class_flag_names[4][10] = {
	/* [TP_FFINAL]     = */ "FINAL",
	/* [TP_FTRUNCATE]  = */ "TRUNCATE",
	/* [TP_FINTERRUPT] = */ "INTERRUPT",
	/* [0x8]           = */ ""
};
#endif


typedef union {
	instruction_t *ptr;
	uint8_t       *u8;
	int8_t        *s8;
	uint16_t      *u16;
	uint32_t      *u32;
	int16_t       *s16;
	int32_t       *s32;
#define READ_imm8(ip)   (*ip.u8++)
#define READ_Simm8(ip)  (*ip.s8++)
#define READ_imm16(ip)  UNALIGNED_GETLE16(ip.u16++)
#define READ_Simm16(ip) ((int16_t)UNALIGNED_GETLE16(ip.u16++))
#define READ_imm32(ip)  UNALIGNED_GETLE32(ip.u32++)
#define READ_Simm32(ip) ((int32_t)UNALIGNED_GETLE32(ip.u32++))
} ip_t;


/* Define instruction format tables. */
#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) 1
#define DEE_ASM_BEGIN(table_prefix)          union mnemonic_fmt_maxlen_##table_prefix {
#define DEE_ASM_END(table_prefix)            };
#if __SIZEOF_CHAR__ == 1
#define DEE_ASM_OPCODE(table_prefix, opcode_byte, instr_len, name, sp_sub, sp_add, mnemonic) char l##name[sizeof(mnemonic)];
#else /* __SIZEOF_CHAR__ == 1 */
#define DEE_ASM_OPCODE(table_prefix, opcode_byte, instr_len, name, sp_sub, sp_add, mnemonic) char l##name[sizeof(mnemonic) / __SIZEOF_CHAR__];
#endif /* __SIZEOF_CHAR__ != 1 */
#include <deemon/asm-table.h>

#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) 1
#define DEE_ASM_BEGIN(table_prefix)          union mnemonic_p_fmt_maxlen_##table_prefix {
#define DEE_ASM_END(table_prefix)            };
#if __SIZEOF_CHAR__ == 1
#define DEE_ASM_OPCODE_P(table_prefix, opcode_byte, instr_len, name, sp_sub, sp_add, mnemonic, prefix_sp_sub, prefix_sp_add, prefix_mnemonic) char l##name[sizeof(prefix_mnemonic)];
#else /* __SIZEOF_CHAR__ == 1 */
#define DEE_ASM_OPCODE_P(table_prefix, opcode_byte, instr_len, name, sp_sub, sp_add, mnemonic, prefix_sp_sub, prefix_sp_add, prefix_mnemonic) char l##name[sizeof(prefix_mnemonic) / __SIZEOF_CHAR__];
#endif /* __SIZEOF_CHAR__ != 1 */
#include <deemon/asm-table.h>

#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) 1
#define DEE_ASM_BEGIN(table_prefix)          PRIVATE char const mnemonics_##table_prefix[256][sizeof(union mnemonic_fmt_maxlen_##table_prefix)] = {
#define DEE_ASM_END(table_prefix)            };
#define DEE_ASM_UNDEFINED(table_prefix, opcode_byte, instr_len)                              "",
#define DEE_ASM_OPCODE(table_prefix, opcode_byte, instr_len, name, sp_sub, sp_add, mnemonic) mnemonic,
#include <deemon/asm-table.h>

#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) 1
#define DEE_ASM_BEGIN(table_prefix)          PRIVATE char const mnemonics_p_##table_prefix[256][sizeof(union mnemonic_p_fmt_maxlen_##table_prefix)] = {
#define DEE_ASM_END(table_prefix)            };
#define DEE_ASM_UNDEFINED(table_prefix, opcode_byte, instr_len)                              "",
#define DEE_ASM_OPCODE_P(table_prefix, opcode_byte, instr_len, name, sp_sub, sp_add, mnemonic, prefix_sp_sub, prefix_sp_add, prefix_mnemonic) prefix_mnemonic,
#include <deemon/asm-table.h>


#define DO(err, expr)                  \
	do {                               \
		if unlikely((temp = expr) < 0) \
			goto err;                  \
		result += temp;                \
	}	__WHILE0
#define print(err, p, s) DO(err, (*printer)(arg, p, s))
#define PRINT(err, s)    DO(err, (*printer)(arg, s, COMPILER_STRLEN(s)))
#define printf(err, ...) DO(err, DeeFormat_Printf(printer, arg, __VA_ARGS__))


/* The minimum amount of characters (including at least 1 trailing space) before operands start */
#define MNEMONIC_MINWIDTH 7
PRIVATE char const mnemonic_namepad[MNEMONIC_MINWIDTH - 1] = { ' ', ' ', ' ', ' ', ' ', ' ' };

/* GCC complains that the first "__IF0" within the format switch is unreachable.
 * Yeah: no $hit, dude. That's why it literally a no-op statement.
 *
 * imo: that warning should not be emitted in code like:
 * >> switch (foo) {
 * >>
 * >> {
 * >>     // vvv GCC warnings that this "if (0)" is unreachable
 * >>     if (0) { case 1: a(); }
 * >>     if (0) { case 2: b(); }
 * >>     c();
 * >> }   break;
 * >>
 * >> {
 * >>     // vvv But then it doesn't warn about this one...
 * >>     if (0) { case 3: d(); }
 * >>     if (0) { case 4: e(); }
 * >>     f();
 * >> }   break;
 * >>
 * >> }
 */
__pragma_GCC_diagnostic_push_ignored(Wswitch_unreachable)

PRIVATE WUNUSED NONNULL((1, 3, 4, 6)) Dee_ssize_t DCALL
libdisasm_printinstr_f(Dee_formatprinter_t printer, void *arg,
                       /*[1..1]*/ instruction_t *instr_start,
                       /*[1..1]*/ instruction_t *operands_start,
                       /*[0..1]*/ instruction_t *prefix_start,
                       /*[1..1]*/ char const *format,
                       uint16_t stacksz, struct ddi_state *ddi,
                       DeeCodeObject *code, unsigned int flags) {
	ip_t ip;
	Dee_ssize_t temp, result;
	char const *flush_start;
	char const *padpos;
	char const *iter;
	bool minimal_padding = false;
	ASSERT(*format);

	/* Check for special case: there is a prefix, but the instruction doesn't
	 *                         (appear to?) use it as part of its operands.
	 * In this case, print the prefix using the generic syntax. */
	if (prefix_start && !strchr(format, (int)(unsigned int)(unsigned char)F_PREFIX_C)) {
		result = libdisasm_printprefix(printer, arg, prefix_start, ddi, code, flags);
		if unlikely(result < 0)
			goto done;
		PRINT(err, ": ");
		minimal_padding = true;
		/*prefix_start = NULL;*/
	} else {
		result = 0;
	}

	/* Figure out if we need to pad after the instruction's mnemonic. */
	iter   = format;
	padpos = strchr(iter, (int)(unsigned int)(unsigned char)F_PAD_C);
	if (padpos) {
		size_t padsiz;
		padsiz = (size_t)(padpos - iter);
		result = (*printer)(arg, iter, padsiz);
		if unlikely(result < 0)
			goto done;
		iter = padpos + 1;

		/* Figure out how many space characters to print before operands start */
		if (!minimal_padding && MNEMONIC_MINWIDTH > padsiz) {
			padsiz = MNEMONIC_MINWIDTH - padsiz;
		} else {
			padsiz = 1;
		}
		print(err, mnemonic_namepad, padsiz);
	}

	/* Time to print the actual mnemonic */
	flush_start = iter;
	ip.ptr = operands_start;
	for (;;) {
		uint16_t imm, imm2;
		unsigned char ch = (unsigned char)*iter;
		if (ch < F_MINCODE) {
			if (!ch)
				break;
			++iter;
			continue;
		}
		if (flush_start < iter)
			print(err, flush_start, (size_t)(iter - flush_start));
		ASSERT(ch >= F_MINCODE);
		ASSERT(ch <= F_MAXCODE);
		++iter;
		switch (ch) {

#if defined(F_ARG8_C) || defined(F_ARG16_C)
		{
#ifdef F_ARG8_C
		__IF0 { case F_ARG8_C: imm = READ_imm8(ip); }
#endif /* F_ARG8_C */
#ifdef F_ARG16_C
		__IF0 { case F_ARG16_C: imm = READ_imm16(ip); }
#endif /* F_ARG16_C */
			DO(err, libdisasm_printarg(printer, arg, imm, code, flags));
		}	break;
#endif /* F_ARG8_C || F_ARG16_C */

#if defined(F_CONST8_C) || defined(F_CONST16_C)
		{
#ifdef F_CONST8_C
		__IF0 { case F_CONST8_C: imm = READ_imm8(ip); }
#endif /* F_CONST8_C */
#ifdef F_CONST16_C
		__IF0 { case F_CONST16_C: imm = READ_imm16(ip); }
#endif /* F_CONST16_C */
			DO(err, libdisasm_printconst(printer, arg, imm, code, flags));
		}	break;
#endif /* F_CONST8_C || F_CONST16_C */

#if defined(F_EXTERN8_C) || defined(F_EXTERN16_C)
		{
#ifdef F_EXTERN8_C
		__IF0 { case F_EXTERN8_C: imm = READ_imm8(ip); imm2 = READ_imm8(ip); }
#endif /* F_EXTERN8_C */
#ifdef F_EXTERN16_C
		__IF0 { case F_EXTERN16_C: imm = READ_imm16(ip); imm2 = READ_imm16(ip); }
#endif /* F_EXTERN16_C */
			DO(err, libdisasm_printextern(printer, arg, imm, imm2, code, flags));
		}	break;
#endif /* F_EXTERN8_C || F_EXTERN16_C */

#if defined(F_GLOBAL8_C) || defined(F_GLOBAL16_C)
		{
#ifdef F_GLOBAL8_C
		__IF0 { case F_GLOBAL8_C: imm = READ_imm8(ip); }
#endif /* F_GLOBAL8_C */
#ifdef F_GLOBAL16_C
		__IF0 { case F_GLOBAL16_C: imm = READ_imm16(ip); }
#endif /* F_GLOBAL16_C */
			DO(err, libdisasm_printglobal(printer, arg, imm, code, flags));
		}	break;
#endif /* F_GLOBAL8_C || F_GLOBAL16_C */

#if defined(F_MODULE8_C) || defined(F_MODULE16_C)
		{
#ifdef F_MODULE8_C
		__IF0 { case F_MODULE8_C: imm = READ_imm8(ip); }
#endif /* F_MODULE8_C */
#ifdef F_MODULE16_C
		__IF0 { case F_MODULE16_C: imm = READ_imm16(ip); }
#endif /* F_MODULE16_C */
			DO(err, libdisasm_printmodule(printer, arg, imm, code, flags));
		}	break;
#endif /* F_MODULE8_C || F_MODULE16_C */

#if defined(F_LOCAL8_C) || defined(F_LOCAL16_C)
		{
#ifdef F_LOCAL8_C
		__IF0 { case F_LOCAL8_C: imm = READ_imm8(ip); }
#endif /* F_LOCAL8_C */
#ifdef F_LOCAL16_C
		__IF0 { case F_LOCAL16_C: imm = READ_imm16(ip); }
#endif /* F_LOCAL16_C */
			DO(err, libdisasm_printlocal(printer, arg, imm, ddi, code, flags));
		}	break;
#endif /* F_LOCAL8_C || F_LOCAL16_C */

#if defined(F_REF8_C) || defined(F_REF16_C)
		{
#ifdef F_REF8_C
		__IF0 { case F_REF8_C: imm = READ_imm8(ip); }
#endif /* F_REF8_C */
#ifdef F_REF16_C
		__IF0 { case F_REF16_C: imm = READ_imm16(ip); }
#endif /* F_REF16_C */
			DO(err, libdisasm_printref(printer, arg, imm, code, flags));
			/* Check for special case: try to print the name of a referenced class member slot. */
			if (memcmp(iter, ", $", 3) == 0 &&
			    ((unsigned char)iter[3] == F_IMM8_C ||
			     (unsigned char)iter[3] == F_IMM16_C)) {
				bool is_cmember;
				PRINT(err, ", $");
				iter += 3;
				if ((unsigned char)iter[0] == F_IMM8_C) {
					imm2 = READ_imm8(ip);
				} else {
					imm2 = READ_imm16(ip);
				}
				iter += 1;
				is_cmember = strstr(format, "cmember") != NULL;
				DO(err, libdisasm_printmembername(printer, arg, imm, imm2,
				                                  code, flags, is_cmember));
			}
		}	break;
#endif /* F_REF8_C || F_REF16_C */

#if defined(F_STATIC8_C) || defined(F_STATIC16_C)
		{
#ifdef F_STATIC8_C
		__IF0 { case F_STATIC8_C: imm = READ_imm8(ip); }
#endif /* F_STATIC8_C */
#ifdef F_STATIC16_C
		__IF0 { case F_STATIC16_C: imm = READ_imm16(ip); }
#endif /* F_STATIC16_C */
			DO(err, libdisasm_printstatic(printer, arg, imm, code, flags));
		}	break;
#endif /* F_STATIC8_C || F_STATIC16_C */

#ifdef F_PREFIX_C
		case F_PREFIX_C:  {
			DO(err, libdisasm_printprefix(printer, arg,
			                              prefix_start ? prefix_start : instr_start,
			                              ddi, code, flags));
		}	break;
#endif /* F_PREFIX_C */

#if defined(F_SDISP8_C) || defined(F_SDISP16_C) || defined(F_SDISP32_C)
		{
			int32_t disp;
#ifdef F_SDISP8_C
		__IF0 { case F_SDISP8_C: disp = READ_Simm8(ip); }
#endif /* F_SDISP8_C */
#ifdef F_SDISP16_C
		__IF0 { case F_SDISP16_C: disp = READ_Simm16(ip); }
#endif /* F_SDISP16_C */
#ifdef F_SDISP32_C
		__IF0 { case F_SDISP32_C: disp = READ_Simm32(ip); }
#endif /* F_SDISP32_C */
			if (code) {
				code_addr_t target_ip;
				target_ip = (code_addr_t)(ip.ptr - code->co_code) + disp;
				if (!(flags & PCODE_FNOLABELS) && target_ip < code->co_codebytes) {
					/* Make use of disassembler label names. */
					uint16_t opcode = instr_start[0];
					if (ASM_ISEXTENDED(opcode))
						opcode = (opcode << 8) | instr_start[1];
					DO(err, libdisasm_printlabel(printer, arg, opcode,
					                             (code_addr_t)(instr_start - code->co_code),
					                             target_ip));
				} else {
					printf(err, "%.4" PRFX32, target_ip);
					if (target_ip >= code->co_codebytes)
						PRINT(err, " /* invalid ip */");
				}
			} else {
				printf(err, "PC%+#" PRFX32, disp);
			}
		}	break;
#endif /* F_SDISP8_C || F_SDISP16_C || F_SDISP32_C */

#if (defined(F_IMM8_C) || defined(F_IMM8_PLUS1_C) ||       \
     defined(F_IMM8_PLUS2_C) || defined(F_IMM8_PLUS3_C) || \
     defined(F_IMM8_X2_C) ||                               \
     defined(F_IMM16_C) || defined(F_IMM16_PLUS2_C) ||     \
     defined(F_IMM16_PLUS3_C) || defined(F_IMM16_X2_C) ||  \
     defined(F_IMM32_C))
		{
			uint32_t immval;
#ifdef F_IMM8_C
		__IF0 { case F_IMM8_C: immval = READ_imm8(ip); }
#endif /* F_IMM8_C */
#ifdef F_IMM8_PLUS1_C
		__IF0 { case F_IMM8_PLUS1_C: immval = READ_imm8(ip) + 1; }
#endif /* F_IMM8_PLUS1_C */
#ifdef F_IMM8_PLUS2_C
		__IF0 { case F_IMM8_PLUS2_C: immval = READ_imm8(ip) + 2; }
#endif /* F_IMM8_PLUS2_C */
#ifdef F_IMM8_PLUS3_C
		__IF0 { case F_IMM8_PLUS3_C: immval = READ_imm8(ip) + 3; }
#endif /* F_IMM8_PLUS3_C */
#ifdef F_IMM8_X2_C
		__IF0 { case F_IMM8_X2_C: immval = READ_imm8(ip) * 2; }
#endif /* F_IMM8_X2_C */
#ifdef F_IMM16_C
		__IF0 { case F_IMM16_C: immval = READ_imm16(ip); }
#endif /* F_IMM16_C */
#ifdef F_IMM16_PLUS2_C
		__IF0 { case F_IMM16_PLUS2_C: immval = READ_imm16(ip) + 2; }
#endif /* F_IMM16_PLUS2_C */
#ifdef F_IMM16_PLUS3_C
		__IF0 { case F_IMM16_PLUS3_C: immval = READ_imm16(ip) + 3; }
#endif /* F_IMM16_PLUS3_C */
#ifdef F_IMM16_X2_C
		__IF0 { case F_IMM16_X2_C: immval = READ_imm16(ip) * 2; }
#endif /* F_IMM16_X2_C */
#ifdef F_IMM32_C
		__IF0 { case F_IMM32_C: immval = READ_imm32(ip); }
#endif /* F_IMM32_C */
			printf(err, "%" PRFu32, immval);
		}	break;
#endif /* ... */

#if (defined(F_SIMM8_C) || defined(F_SIMM16_C))
		{
			int32_t Simmval;
#ifdef F_SIMM8_C
		__IF0 { case F_SIMM8_C: Simmval = READ_Simm8(ip); }
#endif /* F_SIMM8_C */
#ifdef F_SIMM16_C
		__IF0 { case F_SIMM16_C: Simmval = READ_Simm16(ip); }
#endif /* F_SIMM16_C */
			printf(err, "%" PRFd32, Simmval);
		}	break;
#endif /* ... */

#if defined(F_SP_PLUSS8_C) || defined(F_SP_PLUSS16_C)
		{
			int16_t Simmval;
			int32_t adjusted_stacksz;
#ifdef F_SP_PLUSS8_C
		__IF0 { case F_SP_PLUSS8_C: Simmval = READ_Simm8(ip); }
#endif /* F_SP_PLUSS8_C */
#ifdef F_SP_PLUSS16_C
		__IF0 { case F_SP_PLUSS16_C: Simmval = READ_Simm16(ip); }
#endif /* F_SP_PLUSS16_C */
			if ((stacksz != (uint16_t)-1) &&
			    (adjusted_stacksz = (int32_t)stacksz + Simmval,
			     adjusted_stacksz >= 0 && adjusted_stacksz <= UINT16_MAX)) {
				printf(err, "%" PRFu16, (uint16_t)adjusted_stacksz);
			} else {
				printf(err, "SP %c %" PRFd16, Simmval < 0 ? '-' : '+', Simmval);
			}
		}	break;
#endif /* ... */

#if defined(F_SP_SUB8_SUB2_C) || defined(F_SP_SUB16_SUB2_C)
		{
			uint32_t immval;
#ifdef F_SP_SUB8_SUB2_C
		__IF0 { case F_SP_SUB8_SUB2_C: immval = READ_imm8(ip) + 2; }
#endif /* F_SP_SUB8_SUB2_C */
#ifdef F_SP_SUB16_SUB2_C
		__IF0 { case F_SP_SUB16_SUB2_C: immval = READ_imm16(ip) + 2; }
#endif /* F_SP_SUB16_SUB2_C */
			DO(err, libdisasm_printrelstack(printer, arg, stacksz, immval, ddi, code, flags));
		}	break;
#endif /* ... */

		default:
#ifdef NDEBUG
			__builtin_unreachable();
#else /* NDEBUG */
			Dee_Fatalf("Unsupported/unknown format code: %#.2" PRFx8, ch);
#endif /* !NDEBUG */
		}
		flush_start = iter;
	}
	/* Flush remainder (if any) */
	if (flush_start < iter)
		print(err, flush_start, (size_t)(iter - flush_start));
done:
	return result;
err:
	return temp;
}

__pragma_GCC_diagnostic_pop_ignored(Wswitch_unreachable)

INTERN WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
libdisasm_printinstr(Dee_formatprinter_t printer, void *arg,
                     instruction_t *__restrict instr_start, uint16_t stacksz,
                     struct ddi_state *ddi, DeeCodeObject *code,
                     unsigned int flags) {
	ip_t ip;
	Dee_ssize_t temp, result;
	instruction_t opcode;
	char const *format;
	ip.ptr = instr_start;
	opcode = *ip.ptr++;
	format = mnemonics_0x00[opcode];
	if (*format) {
		switch (opcode) {
		case ASM_YIELD:
			if (code->co_flags & CODE_FYIELDING)
				format = "yield" F_PAD "pop";
			break;
		default: break;
		}
		return libdisasm_printinstr_f(printer, arg, instr_start, ip.ptr, NULL,
		                              format, stacksz, ddi, code, flags);
	}
	if (ASM_ISEXTENDED(opcode)) {
		instruction_t opcode2 = *ip.ptr++;
		switch (opcode) {
#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) (prefix_byte_or_0 == 0)
#define DEE_ASM_EXTENDED(table_prefix, opcode_byte, instr_len, name) \
		case opcode_byte:                                            \
			format = mnemonics_##opcode_byte[opcode2];               \
			break;
#ifndef __INTELLISENSE__
#include <deemon/asm-table.h>
#endif /* !__INTELLISENSE__ */
		default: break;
		}
		if (*format) {
			return libdisasm_printinstr_f(printer, arg, instr_start, ip.ptr, NULL,
			                              format, stacksz, ddi, code, flags);
		}
		if (ASM_ISPREFIX(opcode2))
			goto do_handle_prefix;
		result = 0;
	} else if (ASM_ISPREFIX(opcode)) {
		instruction_t *after_prefix;
do_handle_prefix:
		ip.ptr = after_prefix = DeeAsm_SkipPrefix(instr_start);
		opcode = *ip.ptr++;
		format = mnemonics_p_0x00[opcode];
		if (*format) {
			switch (opcode) {
			case ASM_YIELD:
				if (code->co_flags & CODE_FYIELDING)
					format = "yield" F_PAD F_PREFIX;
				break;
			default: break;
			}
			return libdisasm_printinstr_f(printer, arg, after_prefix, ip.ptr, instr_start,
			                              format, stacksz, ddi, code, flags);
		}
		if (ASM_ISEXTENDED(opcode)) {
			instruction_t opcode2 = *ip.ptr++;
			switch (opcode) {
#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) (prefix_byte_or_0 == 0)
#define DEE_ASM_EXTENDED(table_prefix, opcode_byte, instr_len, name) \
			case opcode_byte:                                        \
				format = mnemonics_p_##opcode_byte[opcode2];         \
				break;
#ifndef __INTELLISENSE__
#include <deemon/asm-table.h>
#endif /* !__INTELLISENSE__ */
			default: break;
			}
			if (*format) {
				return libdisasm_printinstr_f(printer, arg, after_prefix, ip.ptr, instr_start,
				                              format, stacksz, ddi, code, flags);
			}
			switch (opcode) {
#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) (prefix_byte_or_0 == 0)
#define DEE_ASM_EXTENDED(table_prefix, opcode_byte, instr_len, name) \
			case opcode_byte:                                        \
				format = mnemonics_##opcode_byte[opcode2];           \
				break;
#ifndef __INTELLISENSE__
#include <deemon/asm-table.h>
#endif /* !__INTELLISENSE__ */
			default: break;
			}
		} else {
			format = mnemonics_0x00[opcode];
		}
		/* Special case: prefix on unknown instruction, or instruction that doesn't normally have one. */
		if (*format) {
			return libdisasm_printinstr_f(printer, arg, after_prefix, ip.ptr, instr_start,
			                              format, stacksz, ddi, code, flags);
		}
		/* Special case: prefix on unknown instruction (fallthru to generic ".byte" printing below) */
		result = libdisasm_printprefix(printer, arg, instr_start, ddi, code, flags);
		if unlikely(result < 0)
			goto done;
		PRINT(err, ": ");
		instr_start = ip.ptr;
	} else {
		result = 0;
	}

	/* Fallback: unknown instruction (print as raw bytes) */
	ip.ptr = DeeAsm_NextInstr(instr_start);
	{
		size_t i, num_bytes = (size_t)(ip.ptr - instr_start);
		ASSERT(num_bytes >= 1);
		PRINT(err, ".byte ");
		if unlikely(result < 0)
			goto done;
		for (i = 0; i < num_bytes; ++i) {
			if (i != 0)
				PRINT(err, ", ");
			printf(err, "%#.2" PRFx8, instr_start[i]);
		}
	}
done:
	return result;
err:
	return temp;
}

DECL_END

#endif /* !GUARD_DEX_FS_PRINTINSTR_C */
