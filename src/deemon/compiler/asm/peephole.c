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
#ifndef GUARD_DEEMON_COMPILER_ASM_PEEPHOLE_C
#define GUARD_DEEMON_COMPILER_ASM_PEEPHOLE_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_Free, Dee_TryCalloc */
#include <deemon/asm.h>                /* ASM16_*, ASM32_JMP, ASM_*, CASE_ASM_EXTENDED, DeeAsm_*, instruction_t */
#include <deemon/code.h>               /* Dee_CODE_FYIELDING, code_addr_t, code_saddr_t, code_size_t, instruction_t */
#include <deemon/compiler/assembler.h> /* ASM_*, R_DMN_NONE, asm_*, current_assembler, ddi_checkpoint */
#include <deemon/compiler/symbol.h>    /* current_basescope */
#include <deemon/format.h>             /* PRF* */
#include <deemon/system-features.h>    /* abort, memcpyc, memmoveupc, memset */

#include <hybrid/byteswap.h>      /* UNALIGNED_GETLE*, UNALIGNED_SETLE* */
#include <hybrid/sequence/list.h> /* SLIST_FOREACH */
#include <hybrid/unaligned.h>     /* UNALIGNED_GETLE*, UNALIGNED_SETLE* */

#include "../../runtime/strings.h"

#include <stdarg.h>  /* va_end, va_list, va_start */
#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* INT8_MAX, INT8_MIN, INT16_MAX, INT16_MIN, INT32_MAX, INT32_MIN, UINT8_MAX, int8_t, int16_t, int32_t, uint8_t, uint16_t, uint32_t */

DECL_BEGIN


#define sc_main   current_assembler.a_sect[0]

/* Given an instruction that is known to be a jump (`IP_ISJMP()'), check the operand size. */
#define JMP_IS32(x)   ((x)[0] == ASM_EXTENDED1 && (x)[1] == (ASM32_JMP & 0xff))
#define JMP_IS16(x)   (((x)[0] & 1) || ((x)[0] == ASM_EXTENDED1 && ((x)[1] & 1)))
/*#define JMP_IS8(x)  (!((x)[0] & 1))*/
STATIC_ASSERT(ASM_JF16 & 1);
STATIC_ASSERT(ASM_JT16 & 1);
STATIC_ASSERT(ASM_JMP16 & 1);
STATIC_ASSERT(ASM_FOREACH16 & 1);
STATIC_ASSERT(ASM_FOREACH_KEY16 & 1);
STATIC_ASSERT(ASM_FOREACH_VALUE16 & 1);
STATIC_ASSERT(ASM_FOREACH_PAIR16 & 1);
STATIC_ASSERT(!(ASM_JF & 1));
STATIC_ASSERT(!(ASM_JT & 1));
STATIC_ASSERT(!(ASM_JMP & 1));
STATIC_ASSERT(!(ASM_FOREACH & 1));
STATIC_ASSERT(!(ASM_FOREACH_KEY & 1));
STATIC_ASSERT(!(ASM_FOREACH_VALUE & 1));
STATIC_ASSERT(!(ASM_FOREACH_PAIR & 1));

#define IP_ISJMP(x)                              \
	((x)[0] == ASM_JMP || (x)[0] == ASM_JMP16 || \
	 ((x)[0] == ASM_EXTENDED1 && (x)[1] == (ASM32_JMP & 0xff)))
#define OP_ISCJMP(opcode) \
	((opcode) >= ASM_JF && (opcode) <= ASM_JT16)
#define IP_ISCJMP(x) OP_ISCJMP(*(x))


/* Similar to `DeeAsm_NextInstr()', but skip `ASM_DELOP'
 * WARNING: Do _NOT_ use this one for walking instructions for
 *          inter-opcode optimizations.
 *          While ASM_DELOP instruction will be deleted before too long,
 *          they can still be protected instructions that you're normally
 *          not allowed to inter-optimize between.
 *       -> Just use this one sparingly, and remember that ASM_DELOP instructions
 *          will be deleted before peephole will be invoked another time, so
 *          your optimization will still get its chance, even if you don't
 *          use `next_instr()' and instead use `DeeAsm_NextInstr()' everywhere. */
LOCAL instruction_t *DCALL
next_instr(instruction_t *__restrict iter) {
	do {
		iter = DeeAsm_NextInstr(iter);
	} while (*iter == ASM_DELOP);
	return iter;
}

LOCAL instruction_t *DCALL
next_instr_sp(instruction_t *__restrict iter, uint16_t *__restrict p_stacksz) {
#if 1
	return DeeAsm_NextInstrSp(iter, p_stacksz);
#else
	do {
		iter = DeeAsm_NextInstrSp(iter, p_stacksz);
	} while (*iter == ASM_DELOP);
	return iter;
#endif
}

LOCAL struct asm_sym *DCALL
symbol_at(code_addr_t addr) {
	struct asm_sym *iter;
	SLIST_FOREACH (iter, &current_assembler.a_syms, as_link) {
		if (iter->as_addr == addr)
			return iter;
	}
	return NULL;
}

LOCAL struct asm_rel *DCALL
relocation_at(code_addr_t addr) {
	struct asm_rel *iter = sc_main.sec_relv;
	struct asm_rel *end  = iter + sc_main.sec_relc;
	for (; iter < end; ++iter) {
		if (iter->ar_addr != addr)
			continue;
		if (iter->ar_type == R_DMN_NONE)
			continue;
		return iter;
	}
	return NULL;
}

LOCAL bool DCALL
delete_assembly(code_addr_t begin, code_size_t size) {
	/* Now delete everything that is in-between. */
	struct asm_rel *rel_iter = sc_main.sec_relv;
	struct asm_rel *rel_end  = rel_iter + sc_main.sec_relc;
	code_addr_t end          = begin + size;
	bool result              = false;
	instruction_t *iter, *iter_end;
	/* Delete all relocations within the area we're about to delete. */
	while (rel_iter < rel_end && rel_iter->ar_addr < begin)
		++rel_iter;
	while (rel_iter < rel_end && rel_iter->ar_addr < end) {
		result = true;
		asm_reldel(rel_iter);
		++rel_iter;
	}
	iter     = sc_main.sec_begin + begin;
	iter_end = iter + size;
	while (iter < iter_end) {
		if (*iter != ASM_DELOP)
			result = true;
		*iter++ = ASM_DELOP;
	}
	return result;
}


/* Follow a jump instruction */
LOCAL ATTR_RETNONNULL instruction_t *DCALL
follow_jmp(instruction_t *__restrict jmp,
           struct asm_rel **p_rel) {
	struct asm_rel *rel;
	code_addr_t rel_addr;
	code_addr_t result_addr;
	if (*jmp == ASM_EXTENDED1)
		++jmp;
	if (JMP_IS32(jmp)) { /* 32-bit jump */
		jmp += 2;
		rel_addr    = (code_addr_t)(jmp - sc_main.sec_begin);
		result_addr = (code_addr_t)((code_saddr_t)(int32_t)UNALIGNED_GETLE32(jmp));
		jmp += 4;
	} else if (JMP_IS16(jmp)) { /* 16-bit jump */
		jmp += 1;
		rel_addr    = (code_addr_t)(jmp - sc_main.sec_begin);
		result_addr = (code_addr_t)((code_saddr_t)(int16_t)UNALIGNED_GETLE16(jmp));
		jmp += 2;
	} else { /* 8-bit jump */
		jmp += 1;
		rel_addr    = (code_addr_t)(jmp - sc_main.sec_begin);
		result_addr = (code_addr_t)((code_saddr_t) * (int8_t *)jmp);
		jmp += 1;
	}
	/* Jump offsets are relative to the following instruction. */
	result_addr += (code_addr_t)(jmp - sc_main.sec_begin);
	result_addr -= rel_addr;
	/* NOTE: These may not be a relocation if the jump was hard-coded. */
	if ((rel = relocation_at(rel_addr)) != NULL) {
		/* Add a relocation offset. */
		ASSERT(rel->ar_sym);
		ASSERT(ASM_SYM_DEFINED(rel->ar_sym));
		result_addr += rel->ar_sym->as_addr;
		if (p_rel)
			*p_rel = rel;
	} else {
		if (p_rel)
			*p_rel = NULL;
	}
	return sc_main.sec_begin + result_addr;
}

#undef CONFIG_LOG_PEEPHOLE_OPTS
#undef CONFIG_VALIDATE_STACK_DEPTH
#if !defined(NDEBUG) && 1
#define CONFIG_LOG_PEEPHOLE_OPTS
#define CONFIG_LOG_PEEPHOLE_SOURCE
#define CONFIG_VALIDATE_STACK_DEPTH
#endif /* !NDEBUG */

#if defined(CONFIG_LOG_PEEPHOLE_OPTS) || defined(CONFIG_VALIDATE_STACK_DEPTH)
PRIVATE ATTR_NOINLINE void DCALL
print_ddi_file_and_line(instruction_t *addr_ptr) {
	struct ddi_checkpoint *iter, *end;
	code_addr_t addr = (code_addr_t)(addr_ptr - sc_main.sec_begin);
	iter             = current_assembler.a_ddi.da_checkv;
	end              = iter + current_assembler.a_ddi.da_checkc;
	for (; iter < end; ++iter) {
		if (iter->dc_sym->as_addr <= addr)
			continue;
		if (iter > current_assembler.a_ddi.da_checkv) {
do_print:
			--iter;
		}
		Dee_DPRINTF("%s(%d,%d) : ",
		            iter->dc_loc.l_file->f_name,
		            iter->dc_loc.l_line + 1,
		            iter->dc_loc.l_col + 1);
		return;
	}
	if (current_assembler.a_ddi.da_checkc &&
	    iter[-1].dc_sym->as_addr == addr)
		goto do_print;
	Dee_DPRINT("?(?) : ");
}
#endif /* CONFIG_LOG_PEEPHOLE_OPTS || CONFIG_VALIDATE_STACK_DEPTH */


#ifdef CONFIG_VALIDATE_STACK_DEPTH
/* Use symbols to validate our expected stack-depth. */
PRIVATE void DCALL
validate_stack_depth(code_addr_t ip, uint16_t stacksz) {
	struct asm_sym *iter;
	SLIST_FOREACH (iter, &current_assembler.a_syms, as_link) {
		if (!ASM_SYM_DEFINED(iter))
			continue;
		if (iter->as_sect != 0)
			continue;
		if (iter->as_addr != ip)
			continue;
		/* Ignore symbols with an undefined stack-depth. */
		if (iter->as_stck == ASM_SYM_STCK_INVALID)
			continue;
		if (!iter->as_used)
			continue;
		if unlikely(iter->as_stck != stacksz) {
			Dee_DPRINT_SET_ENABLED(true);
			Dee_DPRINTF("\n\n"
			            "FATAL ERROR: Invalid stack-depth at %.4X (expected %u, but got %u)\n"
			            "%s(%d) : See symbol allocated here\n",
			            (unsigned)ip, (unsigned)iter->as_stck,
			            (unsigned)stacksz, iter->as_file, iter->as_line);
			print_ddi_file_and_line(sc_main.sec_begin + ip);
			Dee_DPRINTF("See reference to nearest DDI checkpoint\n");
			Dee_BREAKPOINT();
			abort();
		}
	}
}
#else /* CONFIG_VALIDATE_STACK_DEPTH */
#define validate_stack_depth(ip, stacksz) (void)0
#endif /* !CONFIG_VALIDATE_STACK_DEPTH */


#ifdef CONFIG_LOG_PEEPHOLE_OPTS

#if 0
#ifdef CONFIG_LOG_PEEPHOLE_SOURCE
PRIVATE ATTR_NOINLINE void DCALL
peephole_opt(instruction_t *addr_ptr)
#else /* CONFIG_LOG_PEEPHOLE_SOURCE */
PRIVATE ATTR_NOINLINE void DCALL
peephole_opt(char const *file, int line, instruction_t *addr_ptr)
#endif /* !CONFIG_LOG_PEEPHOLE_SOURCE */
{
#ifdef CONFIG_LOG_PEEPHOLE_SOURCE
	print_ddi_file_and_line(addr_ptr);
	Dee_DPRINTF("Peephole at +%.4I32X\n",
	            (code_addr_t)(addr_ptr - sc_main.sec_begin));
#else /* CONFIG_LOG_PEEPHOLE_SOURCE */
	Dee_DPRINTF("%s(%d) : Peephole at +%.4I32X\n", file, line,
	            (code_addr_t)(addr_ptr - sc_main.sec_begin));
#endif /* !CONFIG_LOG_PEEPHOLE_SOURCE */
}
#endif

#ifdef CONFIG_LOG_PEEPHOLE_SOURCE
PRIVATE ATTR_NOINLINE void DCALL
peephole_optf(instruction_t *addr_ptr,
              char const *__restrict format, ...)
#else /* CONFIG_LOG_PEEPHOLE_SOURCE */
PRIVATE ATTR_NOINLINE void DCALL
peephole_optf(char const *file, int line, instruction_t *addr_ptr,
              char const *__restrict format, ...)
#endif /* !CONFIG_LOG_PEEPHOLE_SOURCE */
{
	va_list args;
#ifdef CONFIG_LOG_PEEPHOLE_SOURCE
	print_ddi_file_and_line(addr_ptr);
	Dee_DPRINTF("Peephole at +%.4I32X : ",
	            (code_addr_t)(addr_ptr - sc_main.sec_begin));
#else /* CONFIG_LOG_PEEPHOLE_SOURCE */
	Dee_DPRINTF("%s(%d) : Peephole at +%.4I32X : ", file, line,
	            (code_addr_t)(addr_ptr - sc_main.sec_begin));
#endif /* !CONFIG_LOG_PEEPHOLE_SOURCE */
	va_start(args, format);
	Dee_VDPRINTF(format, args);
	va_end(args);
	Dee_DPRINT("\n");
}

PRIVATE ATTR_NOINLINE void DCALL
peephole_opt2(char const *file, int line, instruction_t *addr_ptr, instruction_t *addr_ptr2) {
	Dee_DPRINTF("%s(%d) : Peephole at +%.4I32x and +%.4I32x\n", file, line,
	            (code_addr_t)(addr_ptr - sc_main.sec_begin),
	            (code_addr_t)(addr_ptr2 - sc_main.sec_begin));
}
#endif /* CONFIG_LOG_PEEPHOLE_OPTS */


PRIVATE void DCALL
decrement_stack_referenes(instruction_t *start,
                          instruction_t *end,
                          uint16_t old_stacksz) {
	instruction_t *iter = start;
	uint16_t stacksz    = old_stacksz;
	uint16_t abs_stackaddr;
	while (iter < end) {
		instruction_t *iiter = iter;
		uint16_t opcode;
read_opcode:
		opcode = *iiter++;
switch_on_opcode:
		switch (opcode) {

		case ASM_STACK:
			abs_stackaddr = *(uint8_t *)(iiter + 0);
			if (abs_stackaddr >= old_stacksz - 1)
				*(uint8_t *)(iiter + 0) = (uint8_t)(abs_stackaddr - 1);
			iiter += 1;
			goto read_opcode;

		case ASM16_STACK:
			abs_stackaddr = UNALIGNED_GETLE16(iiter + 0);
			if (abs_stackaddr >= old_stacksz - 1)
				UNALIGNED_SETLE16(iiter + 0, (uint16_t)(abs_stackaddr - 1));
			iiter += 2;
			goto read_opcode;

		case ASM_POP_N:
			abs_stackaddr = (stacksz - (*(uint8_t *)(iiter + 0) + 2));
			if (abs_stackaddr <= old_stacksz) {
				if ((*(uint8_t *)(iiter + 0))-- == 0) {
					/* Replace with a regular `pop' instruction */
					memset(iter + 1, ASM_DELOP, (size_t)(iiter - iter));
					iter[0] = ASM_POP;
				}
			}
			--stacksz;
			iter = iiter + 1;
			break;

		case ASM16_POP_N:
			abs_stackaddr = (stacksz - UNALIGNED_GETLE16(iiter + 0) + 2);
			if (abs_stackaddr <= old_stacksz) {
				uint16_t old_val;
				old_val = UNALIGNED_GETLE16(iiter + 0);
				UNALIGNED_SETLE16(iiter + 0, old_val - 1);
				if (old_val == 0) {
					/* Replace with a regular `pop' instruction */
					memset(iter, ASM_DELOP, (size_t)(iiter - iter) + 2);
					iter[0] = ASM_POP;
				}
			}
			--stacksz;
			iter = iiter + 2;
			break;

		case ASM_DUP_N:
			abs_stackaddr = (stacksz - (*(uint8_t *)(iiter + 0) + 2));
			if (abs_stackaddr <= old_stacksz) {
				if ((*(uint8_t *)(iiter + 0))-- == 0) {
					/* Replace with a regular `dup' instruction */
					memset(iter + 1, ASM_DELOP, (size_t)(iiter - iter));
					iter[0] = ASM_DUP;
				}
			}
			++stacksz;
			iter = iiter + 1;
			break;

		case ASM16_DUP_N:
			abs_stackaddr = (stacksz - UNALIGNED_GETLE16(iiter + 0) + 2);
			if (abs_stackaddr <= old_stacksz) {
				uint16_t old_val;
				old_val = UNALIGNED_GETLE16(iiter + 0);
				UNALIGNED_SETLE16(iiter + 0, old_val - 1);
				if (old_val == 0) {
					/* Replace with a regular `dup' instruction */
					memset(iter, ASM_DELOP, (size_t)(iiter - iter) + 2);
					iter[0] = ASM_DUP;
				}
			}
			++stacksz;
			iter = iiter + 2;
			break;

		CASE_ASM_EXTENDED:
			opcode <<= 8;
			opcode |= *iiter++;
			goto switch_on_opcode;

		case ASM_JMP:
		case ASM_JMP16:
		case ASM32_JMP: {
			instruction_t *next;
			next = follow_jmp(iter, NULL);
			ASSERT(next >= iter); /* The caller is required to ensure this. */
			iter = next;
		}	break;

		default:
			iter = DeeAsm_NextInstrSp(iter, &stacksz);
			ASSERT(stacksz >= old_stacksz - 1);
			break;
		}
	}
}


#if 0 /* Enable debug instruction logging */
#define HAVE_get_mnemonic

#define F_PAD             "\t"
#define F_ARG8            "ARG8"
#define F_ARG16           "ARG16"
#define F_CONST8          "CONST8"
#define F_CONST16         "CONST16"
#define F_EXTERN8         "EXTERN8"
#define F_EXTERN16        "EXTERN16"
#define F_GLOBAL8         "GLOBAL8"
#define F_GLOBAL16        "GLOBAL16"
#define F_IMM8            "IMM8"
#define F_IMM8_PLUS1      "IMM8_PLUS1"
#define F_IMM8_PLUS2      "IMM8_PLUS2"
#define F_IMM8_PLUS3      "IMM8_PLUS3"
#define F_IMM8_X2         "IMM8_X2"
#define F_IMM16           "IMM16"
#define F_IMM16_PLUS2     "IMM16_PLUS2"
#define F_IMM16_PLUS3     "IMM16_PLUS3"
#define F_IMM16_X2        "IMM16_X2"
#define F_IMM32           "IMM32"
#define F_LOCAL8          "LOCAL8"
#define F_LOCAL16         "LOCAL16"
#define F_MODULE8         "MODULE8"
#define F_MODULE16        "MODULE16"
#define F_PREFIX          "PREFIX"
#define F_REF8            "REF8"
#define F_REF16           "REF16"
#define F_SDISP8          "SDISP8"
#define F_SDISP16         "SDISP16"
#define F_SDISP32         "SDISP32"
#define F_SIMM8           "SIMM8"
#define F_SIMM16          "SIMM16"
#define F_SP_PLUSS8       "SP_PLUSS8"
#define F_SP_PLUSS16      "SP_PLUSS16"
#define F_SP_SUB8_SUB2    "SP_SUB8_SUB2"
#define F_SP_SUB16_SUB2   "SP_SUB16_SUB2"
#define F_STATIC8         "STATIC8"
#define F_STATIC16        "STATIC16"

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
#define DEE_ASM_BEGIN(table_prefix)          PRIVATE char const mnemonics_##table_prefix[256][sizeof(union mnemonic_fmt_maxlen_##table_prefix)] {
#define DEE_ASM_END(table_prefix)            };
#define DEE_ASM_UNDEFINED(table_prefix, opcode_byte, instr_len)                              "",
#define DEE_ASM_OPCODE(table_prefix, opcode_byte, instr_len, name, sp_sub, sp_add, mnemonic) mnemonic,
#include <deemon/asm-table.h>

#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) 1
#define DEE_ASM_BEGIN(table_prefix)          PRIVATE char const mnemonics_p_##table_prefix[256][sizeof(union mnemonic_p_fmt_maxlen_##table_prefix)] {
#define DEE_ASM_END(table_prefix)            };
#define DEE_ASM_UNDEFINED(table_prefix, opcode_byte, instr_len)                              "",
#define DEE_ASM_OPCODE_P(table_prefix, opcode_byte, instr_len, name, sp_sub, sp_add, mnemonic, prefix_sp_sub, prefix_sp_add, prefix_mnemonic) prefix_mnemonic,
#include <deemon/asm-table.h>

PRIVATE char const *DCALL get_mnemonic(instruction_t const *ip) {
	instruction_t opcode;
	char const *format;
	opcode = *ip++;
	format = mnemonics_0x00[opcode];
	if (*format)
		return format;
	if (ASM_ISEXTENDED(opcode)) {
		instruction_t opcode2 = *ip++;
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
		if (*format)
			return format;
		if (ASM_ISPREFIX(opcode2))
			goto do_handle_prefix;
	} else if (ASM_ISPREFIX(opcode)) {
do_handle_prefix:
		ip = DeeAsm_SkipPrefix(ip - 1);
		opcode = *ip++;
		format = mnemonics_p_0x00[opcode];
		if (*format)
			return format;
		if (ASM_ISEXTENDED(opcode)) {
			instruction_t opcode2 = *ip++;
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
			if (*format)
				return format;
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
		if (*format)
			return format;
	}
	return NULL;
}
#endif


INTERN WUNUSED int DCALL asm_peephole(void) {
#ifdef CONFIG_LOG_PEEPHOLE_OPTS
#ifdef CONFIG_LOG_PEEPHOLE_SOURCE
#define SET_RESULT(addr_ptr)             (peephole_opt(addr_ptr), result = true)
#define SET_RESULTF(addr_ptr, ...)       (peephole_optf(addr_ptr, __VA_ARGS__), result = true)
#else /* CONFIG_LOG_PEEPHOLE_SOURCE */
#define SET_RESULT(addr_ptr)             (peephole_opt(__FILE__, __LINE__, addr_ptr), result = true)
#define SET_RESULTF(addr_ptr, ...)       (peephole_optf(__FILE__, __LINE__, addr_ptr, __VA_ARGS__), result = true)
#endif /* !CONFIG_LOG_PEEPHOLE_SOURCE */
#define SET_RESULT2(addr_ptr, addr_ptr2) (peephole_opt2(__FILE__, __LINE__, addr_ptr, addr_ptr2), result = true)
#else /* CONFIG_LOG_PEEPHOLE_OPTS */
#define SET_RESULT(addr_ptr)             (result = true)
#define SET_RESULTF(addr_ptr, ...)       (result = true)
#define SET_RESULT2(addr_ptr, addr_ptr2) (result = true)
#endif /* !CONFIG_LOG_PEEPHOLE_OPTS */

	instruction_t *iter, *end, *after_prefix;
	uint16_t stacksz;
	bool result = false;
	uint8_t *protected_code;
	/* Set to true if the required stack depth should be reloaded. */
	bool should_reload_stack          = false;
	bool should_delete_unused_symbols = false;
	if (!(current_assembler.a_flag & ASM_FPEEPHOLE))
		goto done_raw;

	/* In order to perform peephole optimizations, we must first know
	 * in which places we must not merge operators, those places being
	 * locations where symbols have been defined as targets for jumps,
	 * or for some other use.
	 * NOTE: Because of this, we should probably do a pass where we
	 *       delete all symbols that don't appear in relocations.
	 *       One of these passes should be enough though, as no new
	 *       symbols should be defined once we start peeping into holes...
	 *       Repeatedly doing that and peephole optimization, we should
	 *       be able to delete any unused code so long as it doesn't have
	 *       cross-references. */
	result = asm_delunusedsyms();

	/* Allocate a bitset that describes which
	 * bytes of code should not be optimized. */
	protected_code = (uint8_t *)Dee_TryCalloc(((code_size_t)(sc_main.sec_iter -
	                                                         sc_main.sec_begin) +
	                                           8) /
	                                          8);
	if unlikely(!protected_code)
		goto done_raw;
	{
		struct asm_sym *sym;
		SLIST_FOREACH (sym, &current_assembler.a_syms, as_link) {
			code_addr_t symaddr = sym->as_addr;
			ASSERT(symaddr <= (code_size_t)(sc_main.sec_iter - sc_main.sec_begin));
			protected_code[symaddr / 8] |= 1 << (symaddr % 8);
		}
	}
#define IS_PROTECTED_ADDR(addr) (protected_code[(addr) / 8] & (1 << ((addr) % 8)))
#define IS_PROTECTED(ip)        IS_PROTECTED_ADDR((code_addr_t)((ip) - sc_main.sec_begin))
	stacksz = 0;
	iter    = sc_main.sec_begin;
	end     = sc_main.sec_iter;
continue_at_iter:
	validate_stack_depth((code_addr_t)(iter - sc_main.sec_begin), stacksz);
	for (; iter < end;
	     validate_stack_depth((code_addr_t)(iter - sc_main.sec_begin), stacksz),
	     iter = DeeAsm_NextInstrSp(iter, &stacksz),
	     validate_stack_depth((code_addr_t)(iter - sc_main.sec_begin), stacksz)) {
		uint16_t opcode;
		instruction_t *iiter = iter;
#ifdef HAVE_get_mnemonic
		Dee_DPRINTF("PC %.4" PRFX32 " SP %" PRFu16 " (`%s')\n",
		            (uint32_t)(iter - sc_main.sec_begin), stacksz,
		            get_mnemonic(iter));
#endif /* HAVE_get_mnemonic */
		/* TODO: Eliminate-redundancy-optimization:
		 * >> if (cond) {
		 * >>     print "a";
		 * >>     print "done";
		 * >> } else {
		 * >>     print "b";
		 * >>     print "done";
		 * >> }
		 * ASM:
		 * >>    push  <cond>
		 * >>    jf    pop, 1f
		 * >>    print @"a", nl
		 * >>    print @"done", nl
		 * >>    jmp   2f  // When here, look at the previous instruction and check if
		 * >>              // the instruction immediately before the target is the same.
		 * >>              // If they are, create a new symbol point to the instruction
		 * >>              // before the target, delete the instruction immediately prior
		 * >>              // to the `jmp', and replace the `jmp's target with that new
		 * >>              // symbol.
		 * >>              // It's sufficient if we only optimize one instruction at a
		 * >>              // time using this method, as peephole is repeated indefinitely
		 * >>              // while optimizations are being performed.
		 * >>1:  print @"b", nl
		 * >>    print @"done", nl
		 * >>2:
		 * OPTIMIZED:
		 * >>    push  <cond>
		 * >>    jf    pop, 1f
		 * >>    print @"a", nl
		 * >>    jmp   3f
		 * >>1:  print @"b", nl
		 * >>3:  print @"done", nl
		 */

		/* TODO: Move-to-jump-target-optimization:
		 * >>    push  <cond>
		 * >>    jf    pop, 1f
		 * >>    print @"foo", nl // Move this print...
		 * >>    jmp   2f
		 * >>1:  print @"bar", nl
		 * >>    ret
		 * >>2:  print @"baz", nl // ... before this instruction, because it is not reachable except for the `jmp'
		 * >>    jmp   1b
		 * This may not seem too important, but it would allow us to start
		 * optimizing stack adjustment code generated from jmp-clutches.
		 * -> Imagine it not being a print, but an adjstack.
		 * The the assembly would look like this:
		 * >>    push  <cond>
		 * >>    jf    pop, 1f
		 * >>    jmp   2f
		 * >>1:  print @"bar", nl
		 * >>    ret
		 * >>2:  print @"foo", nl
		 * >>    print @"baz", nl
		 * >>    jmp   1b
		 * At which point we can get rid of the double-jump:
		 * >>    push  <cond>
		 * >>    jt    pop, 2f
		 * >>1:  print @"bar", nl
		 * >>    ret
		 * >>2:  print @"foo", nl
		 * >>    print @"baz", nl
		 * >>    jmp   1b
		 */

		ASSERT(stacksz <= current_assembler.a_stackmax);
		/* Dead code elimination. */
		after_prefix = iter;
		switch (iter[0]) {

		case ASM_ADJSTACK:
			if (!(current_assembler.a_flag & ASM_FOPTIMIZE))
				break;
do_adjstack_optimization:
			/* The `adjstack' instruction is normally optimized by the linker
			 * when `asm_minjmp()' reduces required instruction groupings.
			 * But since peephole optimizations require fully linked stack
			 * alignments, special code has been called to link any stack-related
			 * instruction prematurely, meaning that it falls upon us to optimize
			 * uses of `adjstack', as well as `push $<imm*>', both of which may
			 * be used with an operand that (used to) depend on a relocation
			 * pointing at a stack-address. */
			switch (*(int8_t *)(iter + 1)) {

			case -2:
				iter[0] = ASM_POP;
				iter[1] = ASM_POP;
				SET_RESULTF(iter, "Flatten `adjstack #SP - 2' into `pop; pop'");
				break;

			case -1:
				iter[0] = ASM_POP;
				iter[1] = ASM_DELOP;
				SET_RESULTF(iter, "Flatten `adjstack #SP - 1' into `pop'");
				break;

			case 0:
				iter[0] = ASM_DELOP;
				iter[1] = ASM_DELOP;
				SET_RESULTF(iter, "Flatten `adjstack #SP + 0' into `-'");
				break;

			case 1:
				iter[0] = ASM_PUSH_NONE;
				iter[1] = ASM_DELOP;
				SET_RESULTF(iter, "Flatten `adjstack #SP + 1' into `push none'");
				break;

			case 2:
				iter[0] = ASM_PUSH_NONE;
				iter[1] = ASM_PUSH_NONE;
				SET_RESULTF(iter, "Flatten `adjstack #SP + 2' into `push none; push none'");
				break;

			default: break;
			}
			break;

		case ASM_EXTENDED1: {
			int is_conditional;
			switch (iter[1]) {

			case ASM32_JMP & 0xff:
				goto do_jmpf_unconditional;

			case ASM_FOREACH_KEY & 0xff:
			case ASM_FOREACH_KEY16 & 0xff:
			case ASM_FOREACH_VALUE & 0xff:
			case ASM_FOREACH_VALUE16 & 0xff:
			case ASM_FOREACH_PAIR & 0xff:
			case ASM_FOREACH_PAIR16 & 0xff:
				goto do_jmpf_foreach;

			case ASM_JMP_POP_POP & 0xff:
			case ASM_ENDFINALLY_EXCEPT & 0xff:
				goto do_noreturn_optimization;

			case ASM16_ADJSTACK & 0xff: {
				int16_t adjust;
				if (!(current_assembler.a_flag & ASM_FOPTIMIZE))
					break;
				/* See above: It's our job to try and truncate this instruction. */
				adjust = (int16_t)UNALIGNED_GETLE16(iter + 2);
				if (adjust >= INT8_MIN && adjust <= INT8_MAX) {
					/* Convert to an 8-bit instruction. */
					*iter++               = ASM_DELOP; /* F0 prefix. */
					*(int8_t *)(iter + 1) = (int8_t)adjust;
					*(iter + 2)           = ASM_DELOP; /* Upper 8 bits of old immediate argument. */
					SET_RESULTF(iter, "Trip 16-bit adjstack to 8-bit");
					goto do_adjstack_optimization;
				}
			}	break;

			case ASM16_EXTERN & 0xff:
				after_prefix = iter + 6;
				goto do_basic_optimize_after_prefix;

			case ASM16_LOCAL & 0xff:
			case ASM16_GLOBAL & 0xff:
			case ASM16_STATIC & 0xff:
			case ASM16_STACK & 0xff:
				after_prefix = iter + 4;
				goto do_basic_optimize_after_prefix;

			default: break;
			}
			break;

		case ASM_EXTERN:
			after_prefix = iter + 3;
			goto do_basic_optimize_after_prefix;

		case ASM_LOCAL:
		case ASM_GLOBAL:
		case ASM_STATIC:
		case ASM_STACK:
			after_prefix = iter + 2;
do_basic_optimize_after_prefix:
			if (IP_ISCJMP(after_prefix))
				goto do_jmpf_conditional;
			switch (*after_prefix) {
			case ASM_FOREACH:
			case ASM_FOREACH16:
				goto do_jmpf_foreach;

			case ASM_RET:
				if (!(current_basescope->bs_flags & Dee_CODE_FYIELDING))
					goto do_noreturn_optimization;
				break;
			case ASM_THROW:
				goto do_noreturn_optimization;
			case ASM_EXTENDED1:
				if (after_prefix[1] == (ASM_FOREACH_KEY & 0xff) ||
				    after_prefix[1] == (ASM_FOREACH_KEY16 & 0xff) ||
				    after_prefix[1] == (ASM_FOREACH_VALUE & 0xff) ||
				    after_prefix[1] == (ASM_FOREACH_VALUE16 & 0xff) ||
				    after_prefix[1] == (ASM_FOREACH_PAIR & 0xff) ||
				    after_prefix[1] == (ASM_FOREACH_PAIR16 & 0xff))
					goto do_jmpf_foreach;
				break;
			default: break;
			}
			break;

		case ASM_JF:
		case ASM_JT:
		case ASM_JF16:
		case ASM_JT16:
do_jmpf_conditional:
			is_conditional = 1;
			goto do_jmpf;

		case ASM_FOREACH:
		case ASM_FOREACH16:
do_jmpf_foreach:
			is_conditional = 2;
			goto do_jmpf;

		case ASM_JMP:
		case ASM_JMP16:
do_jmpf_unconditional:
			is_conditional = 0;
do_jmpf:
			/* Forward jump optimization:
			 * >>    jmp 1f
			 * >> 1: jmp 2f
			 * >> 2:
			 * Turn into:
			 * >>    jmp 2f // Optimized here.
			 * >> 1: jmp 2f
			 * >> 2:
			 */
			{
				struct asm_rel *jmp_rel;
				instruction_t *jmp_target;
				jmp_target = follow_jmp(after_prefix, &jmp_rel);
				while (*jmp_target == ASM_DELOP)
					++jmp_target;
					/* Check if the target of this jump is another jump.
					 * NOTE: This however doesn't apply to `foreach' as that one does more than just jump.
					 *       XXX: foreach to the next instruction? That's not even allowed
					 *            because it breaks stack alignment in an unpredictable fashion... */
#if 1
				if (jmp_target == next_instr(after_prefix) &&
				    (after_prefix[0] != ASM_FOREACH &&
				     (after_prefix[0] != ASM_EXTENDED1 || after_prefix[1] != ASM_FOREACH))) {
					/* Jump to the following instruction (aka. no-op jump)
					 * NOTE: To keep any and all side-effects of the original instruction,
					 *       we replace code like `jf 1f; 1:' with `bool top; pop;' */
					if (JMP_IS32(after_prefix)) {
						after_prefix[2] = ASM_DELOP;
						after_prefix[3] = ASM_DELOP;
						after_prefix[4] = ASM_DELOP;
						after_prefix[5] = ASM_DELOP;
					} else if (JMP_IS16(after_prefix)) {
						after_prefix[1] = ASM_DELOP;
						after_prefix[2] = ASM_DELOP;
					}
					if (is_conditional == 2) { /* foreach */
						after_prefix[0] = ASM_POP;
						after_prefix[1] = ASM_DELOP;
					} else if (is_conditional == 1) { /* jcc */
						after_prefix[0] = ASM_BOOL;
						after_prefix[1] = ASM_POP;
					} else {
						after_prefix[0] = ASM_DELOP;
						after_prefix[1] = ASM_DELOP;
					}
					/* Delete the relocation. */
					if (jmp_rel) {
						ASSERT(jmp_rel->ar_sym);
						ASSERT(ASM_SYM_DEFINED(jmp_rel->ar_sym));
						if (jmp_rel->ar_sym->as_used == 1)
							should_delete_unused_symbols = true;
						stacksz = jmp_rel->ar_sym->as_stck;
						asm_reldel(jmp_rel);
					}
					SET_RESULTF(iter, "Remove jump to following instruction");
					iter = jmp_target;
					validate_stack_depth((code_addr_t)(iter - sc_main.sec_begin), stacksz);
					goto continue_at_iter;
				}
#endif
#if 1
				if (jmp_rel && IP_ISJMP(jmp_target)) {
					struct asm_rel *target_rel;
					if (jmp_target[0] == ASM_EXTENDED1) {
						target_rel = relocation_at((code_addr_t)((jmp_target + 2) - sc_main.sec_begin));
					} else {
						target_rel = relocation_at((code_addr_t)((jmp_target + 1) - sc_main.sec_begin));
					}
					if (target_rel && target_rel->ar_sym != jmp_rel->ar_sym) {
						code_saddr_t inplace_disp;
						code_saddr_t relative_target, rel_min, rel_max;
						ASSERT(target_rel->ar_sym);
						ASSERT(ASM_SYM_DEFINED(target_rel->ar_sym));
						/* Since we're overwriting another jump, we must first ensure
						 * that the first jump instruction can even handle the new target.
						 * There is after all the slight possibility that the new target
						 * would cause the relocation to be truncated when overwritten blindly. */
						if (JMP_IS32(after_prefix)) {
							rel_min = INT32_MIN;
							rel_max = INT32_MAX;
						} else if (JMP_IS16(after_prefix)) {
							rel_min = INT16_MIN;
							rel_max = INT16_MAX;
						} else {
							rel_min = INT8_MIN;
							rel_max = INT8_MAX;
						}
						relative_target = (code_addr_t)target_rel->ar_sym->as_addr;
						if (JMP_IS32(jmp_target)) {
							inplace_disp = (code_saddr_t)(int32_t)UNALIGNED_GETLE32(jmp_target + 2) + 4;
						} else if (JMP_IS16(jmp_target)) {
							inplace_disp = (code_saddr_t)(int16_t)UNALIGNED_GETLE16(jmp_target + 1) + 2;
						} else {
							inplace_disp = (code_saddr_t)(int8_t)UNALIGNED_GETLE8(jmp_target + 1) + 1;
						}
						relative_target += inplace_disp;
						relative_target -= (code_saddr_t)(DeeAsm_NextInstr(after_prefix) - sc_main.sec_begin);
						/* `relative_target' now contains the absolute (sign-extended)
						 *  value that will eventually be written to operator of the
						 *  instruction to-be forwarded.
						 *  NOTE: This value can only become smaller when more code may be deleted. */
						if ((relative_target >= rel_min && relative_target <= rel_max) &&
						    (inplace_disp >= rel_min && inplace_disp <= rel_max)) {
							/* Clear the jump offset of the original jump by writing the offsets */
							if (JMP_IS32(after_prefix)) {
								UNALIGNED_SETLE32(after_prefix + 2, (uint32_t)((int32_t)inplace_disp - 4));
							} else if (JMP_IS16(after_prefix)) {
								UNALIGNED_SETLE16(after_prefix + 1, (uint16_t)((int16_t)inplace_disp - 2));
							} else {
								UNALIGNED_SETLE8(after_prefix + 1, (uint8_t)((int8_t)inplace_disp - 1));
							}
							SET_RESULTF(iter, "Follow jump at +%.4I32X to +%.4I32X",
							            jmp_rel->ar_sym->as_addr,
							            target_rel->ar_sym->as_addr);
							/* Track usage of symbols. */
							ASSERT(jmp_rel->ar_sym->as_used);
							if (jmp_rel->ar_sym->as_used == 1)
								should_delete_unused_symbols = true;
							--jmp_rel->ar_sym->as_used;
							++target_rel->ar_sym->as_used;
							/* Simply override the target of this jump with the forward address. */
							jmp_rel->ar_sym = target_rel->ar_sym;
						}
					}
				}
#endif
#if 0 /* Seems to produce less efficient code? */
				/* Optimization:
				 * >>    push ...
				 * >>    jf   pop, 1f
				 * >>    print @"True"
				 * >>    jmp  2f        // The is our instruction
				 * >>1:  print @"False"
				 * >>2:  ret            // This is where it points to
				 * Optimize into:
				 * >>    push ...
				 * >>    jf   pop, 1f
				 * >>    print @"True"
				 * >>    ret
				 * >>1:  print @"False"
				 * >>2:  ret
				 * This way, we have one less indirection to deal with,
				 * and also potentially reduce the overall size of our code.
				 * -> Because any jmp is always at least 2 text bytes, we
				 *    only do this optimization for noreturn target instruction
				 *    that are at most 2 bytes wide. */
				if (!is_conditional &&
				    (*(jmp_target + 0) == ASM_RET_NONE ||
				     (*(jmp_target + 0) == ASM_RET && !(current_basescope->bs_flags & Dee_CODE_FYIELDING)) ||
				     *(jmp_target + 0) == ASM_JMP_POP ||
				     (*(jmp_target + 0) == (ASM_JMP_POP_POP & 0xff00) >> 8 &&
				      *(jmp_target + 1) == (ASM_JMP_POP_POP & 0xff)) ||
				     (!current_assembler.a_exceptc &&
				      /* Only inline instructions such as ASM_THROW or ASM_RETHROW
				       * when the user hasn't defined any exception handlers that
				       * could affect their behavior, based on where they appear. */
				      (*(jmp_target + 0) == ASM_THROW ||
				       *(jmp_target + 0) == ASM_RETHROW)))) {
					if (jmp_rel)
						asm_reldel(jmp_rel);
					if (JMP_IS16(after_prefix))
						*(after_prefix + 2) = ASM_DELOP;
					*(after_prefix + 1) = ASM_DELOP;
					*(after_prefix + 0) = *(jmp_target + 0);
					if (*(after_prefix + 0) == ASM_EXTENDED1)
						*(after_prefix + 1) = *(jmp_target + 1);
					SET_RESULTF(iter, "Inline return/jump-instruction from target site at +%.4I32X",
					            (code_addr_t)(jmp_target - sc_main.sec_begin));
				}
#endif
			}
			/* Don't perform dead code elimination after conditional jumps. */
			if (is_conditional)
				break;
		}	/*ATTR_FALLTHROUGH*/
		__IF0 {
		case ASM_RET:
			if (current_basescope->bs_flags & Dee_CODE_FYIELDING)
				break; /* In yielding code, this instruction behaves differently. */
		}	ATTR_FALLTHROUGH
		case ASM_JMP_POP:
		case ASM_RET_NONE:
		case ASM_THROW:
		case ASM_RETHROW:
		case ASM_UD:
do_noreturn_optimization:
		{
			/* Search for the next defined symbol, continue
			 * there, and delete all intermittent instructions. */
			struct asm_sym *nearest_symbol = NULL;
			struct asm_sym *sym_iter;
			code_addr_t current_symbol_addr;
			code_addr_t nearest_symbol_addr = (code_addr_t)-1;
			code_size_t text_size;
			instruction_t *new_ip;
			iter                     = DeeAsm_NextInstrSp(iter, &stacksz);
			current_symbol_addr      = (code_addr_t)(iter - sc_main.sec_begin);
			SLIST_FOREACH (sym_iter, &current_assembler.a_syms, as_link) {
				if (!ASM_SYM_DEFINED(sym_iter))
					continue;
				if (!sym_iter->as_used)
					continue;
				if (sym_iter->as_sect != 0)
					continue;
				/* Ignore symbols with an undefined stack-depth. */
				if (sym_iter->as_stck == ASM_SYM_STCK_INVALID)
					continue;
#ifdef CONFIG_VALIDATE_STACK_DEPTH
				/* Validate matching stack addresses for multiple
				 * symbols defined for the same address. */
				if (sym_iter->as_addr == nearest_symbol_addr &&
				    sym_iter->as_stck != nearest_symbol->as_stck) {
					Dee_DPRINTF("Conflicting symbol definitions at address +%" PRFx32 ":\n"
					            "%s(%d) : See 1st symbol allocated here (stack %" PRFu16 ")\n"
					            "%s(%d) : See 2nd symbol allocated here (stack %" PRFu16 ")\n",
					            nearest_symbol_addr,
					            nearest_symbol->as_file,
					            nearest_symbol->as_line,
					            nearest_symbol->as_stck,
					            sym_iter->as_file,
					            sym_iter->as_line,
					            sym_iter->as_stck);
					print_ddi_file_and_line(sc_main.sec_begin + nearest_symbol_addr);
					Dee_DPRINTF("See reference to nearest DDI checkpoint\n");
					Dee_BREAKPOINT();
				}
#endif /* CONFIG_VALIDATE_STACK_DEPTH */
				if (sym_iter->as_addr < current_symbol_addr)
					continue;
				if (sym_iter->as_addr >= nearest_symbol_addr)
					continue;
				nearest_symbol      = sym_iter;
				nearest_symbol_addr = sym_iter->as_addr;
			}
			/* Truncate the effective address of the next symbol. */
			text_size = (code_size_t)(sc_main.sec_iter - sc_main.sec_begin);
			if (nearest_symbol_addr > text_size)
				nearest_symbol_addr = text_size;
			new_ip = sc_main.sec_begin + nearest_symbol_addr;
			ASSERT(new_ip >= iter);
			if (new_ip > iter) {
				/* Now delete everything that is in-between. */
				struct asm_rel *rel_iter = sc_main.sec_relv;
				struct asm_rel *rel_end  = rel_iter + sc_main.sec_relc;
				code_addr_t start_ip     = (code_addr_t)(iter - sc_main.sec_begin);
				/* Delete all relocations within the area we're about to delete. */
				while (rel_iter < rel_end && rel_iter->ar_addr < start_ip)
					++rel_iter;
				while (rel_iter < rel_end && rel_iter->ar_addr < nearest_symbol_addr) {
					asm_reldel(rel_iter);
					++rel_iter;
				}
				do {
					if (*iter != ASM_DELOP)
						SET_RESULTF(iter, "Delete unreachable instruction 0x%.2I8x", *iter);
					*iter++ = ASM_DELOP;
				} while (iter < new_ip);
			}
			if (!nearest_symbol)
				goto done;
			/* Continue at the nearest symbol that follows. */
			stacksz = nearest_symbol->as_stck;
			validate_stack_depth((code_addr_t)(iter - sc_main.sec_begin), stacksz);
			goto continue_at_iter;
		}	break;

		default: break;
		}

		/* Cross-instruction optimizations from here on! */

		ASSERT(stacksz <= current_assembler.a_stackmax);
#if 0 /* This shouldn't matter. */
		if (IS_PROTECTED(iiter))
			goto next_opt;
#endif
		opcode = *iiter++;
do_opcode:
		switch (opcode) {

		CASE_ASM_EXTENDED:
			if (opcode & 0xff00)
				break;
			opcode <<= 8;
			opcode |= *iiter++;
			goto do_opcode;

		{
			bool must_invert;
			unsigned int num_instruction;
			/* TODO: Optimize:
			 * >> push  hasmember ...  // Anything that is known to leave a boolean on the stack.
			 * >> bool  top
			 */
		case ASM_INSTANCEOF:
		case ASM_IMPLEMENTS:
			--stacksz;
			ATTR_FALLTHROUGH
		case ASM_ISNONE:
			num_instruction = 0;
			iter            = iiter;
			goto do_push_bool_optimizations;
		case ASM_BOOL:
		case ASM_NOT:
			/* Optimize code patter:
			 * >>    bool      top
			 * >>    jf        pop, TARGET
			 * >>    not       top
			 * >>    jt        pop, TARGET
			 * Into:
			 * >>    jf        pop, TARGET
			 * >>    jf        pop, TARGET
			 */
			num_instruction = 1;
do_push_bool_optimizations:
			must_invert = opcode == ASM_NOT;
			for (;;) {
				if (IS_PROTECTED(iiter)) {
					if (num_instruction > 1)
						break; /* Optimize multiple consecutive bool-instructions. */
					if (*iiter == ASM_DUP) {
						/* Still optimize conditional-jump bool-forwarding, but
						 * just don't include the bool-cast instruction within
						 * the actual optimization range. */
						must_invert     = false;
						iter            = iiter;
						num_instruction = 0;
						goto do_conditional_forward_optimization;
					}
					if (*iiter == ASM_BOOL || *iiter == ASM_NOT) {
						must_invert     = *iiter == ASM_NOT;
						iter            = iiter++;
						num_instruction = 1;
						continue;
					}
					/* Since we're not modifying the conditional-jump instruction itself,
					 * we can still delete bool instruction found before a conditional
					 * jump instruction, since doing so technically isn't a cross-instruction
					 * optimization, since only the (possibly) protected instruction isn't
					 * affected at all.
					 * NOTE: However, since the jump instruction is protected, we must
					 *       not attempt to invert its meaning if the current logical
					 *       answer is negated following an `ASM_NOT' instruction. */
					if (IP_ISCJMP(iiter) && !must_invert && num_instruction != 0)
						goto delete_bool_before_jft;
					goto done_opt_bool;
				}
				if (*iiter != ASM_BOOL && *iiter != ASM_NOT)
					break;
				must_invert ^= *iiter == ASM_NOT;
				++iiter;
				++num_instruction;
			}

			if (*iiter == ASM_DUP) {
				/* Special optimization for conditional-jump boolean-forwarding. */
				instruction_t *instr_jmp;
				instruction_t *instr_pop;
do_conditional_forward_optimization:
				instr_jmp = iiter + 1;
				if (IP_ISCJMP(instr_jmp)) {
					instr_pop = DeeAsm_NextInstr(instr_jmp);
					ASSERT(!IS_PROTECTED(instr_pop));
					if (!IS_PROTECTED(instr_pop) && (*(instr_pop + 0) == ASM_POP ||
					                                 ((*(instr_pop + 0) == ASM_ADJSTACK) && *(int8_t *)(instr_pop + 1) < 0) ||
					                                 ((*(instr_pop + 0) == (ASM16_ADJSTACK & 0xff00) >> 8) &&
					                                  (*(instr_pop + 1) == (ASM16_ADJSTACK & 0xff)) &&
					                                  (int16_t)UNALIGNED_GETLE16(instr_pop + 2) < 0))) {
						/* Found sequence:
						 * >>   bool  top
						 * >>   dup
						 * >>   jf/jt pop, 1f
						 * >>   pop
						 * This sequence is generated for chained conditional
						 * expressions, and can be greatly optimized. */
						struct asm_rel *jmp_reloc;
						instruction_t *jmp_target;
						jmp_target = follow_jmp(instr_jmp, &jmp_reloc);
						if (IP_ISCJMP(jmp_target)) {
							/* Target is another conditional jump:
							 * >>   bool  top
							 * >>   dup
							 * >>   jt    pop, 1f
							 * >>   pop
							 * >>   ...
							 * >>1: jf    pop, 2f
							 * >>   ...
							 * >>2: */
							if ((*instr_jmp & ASM_JX_NOTBIT) != (*jmp_target & ASM_JX_NOTBIT)) {
								/* Redirect to jump after jt/jf instruction. */
								uint8_t target_size = JMP_IS16(jmp_target) ? 3 : 2;
								if (JMP_IS16(instr_jmp)) {
									int32_t new_disp;
									new_disp = (int16_t)UNALIGNED_GETLE16(instr_jmp + 1);
									new_disp += target_size;
									if (new_disp >= INT16_MIN && new_disp <= INT16_MIN) {
										if (jmp_reloc) {
											ASSERT(jmp_reloc->ar_sym);
											ASSERT(ASM_SYM_DEFINED(jmp_reloc->ar_sym));
											ASSERT(jmp_reloc->ar_sym->as_used);
											ASSERT(jmp_reloc->ar_sym->as_stck == stacksz);
											if (jmp_reloc->ar_sym->as_used == 1) {
												jmp_reloc->ar_sym->as_addr += target_size;
												--jmp_reloc->ar_sym->as_stck;
											} else {
												struct asm_sym *newsym = asm_newsym();
												if unlikely(!newsym)
													goto err;
												newsym->as_addr = jmp_reloc->ar_sym->as_addr + target_size;
												newsym->as_stck = jmp_reloc->ar_sym->as_stck - 1;
												newsym->as_sect = jmp_reloc->ar_sym->as_sect;
												newsym->as_hand = jmp_reloc->ar_sym->as_hand;
												newsym->as_used = 1;
												--jmp_reloc->ar_sym->as_used;
												jmp_reloc->ar_sym = newsym;
											}
										} else {
											UNALIGNED_SETLE16(instr_jmp + 1, (uint16_t)(int16_t)new_disp);
										}
conditional_jump_forwarding_ok:
										delete_assembly((code_addr_t)(iter - sc_main.sec_begin),
										                (code_size_t)(instr_jmp - iter));
										/* Apply logic inversion _after_ forwarding checks, because
										 * doing so beforehand would break the branching logic in
										 * expressions like `if (!foo && bar)' */
										if (must_invert) {
											*instr_jmp ^= ASM_JX_NOTBIT;
										} if (*instr_pop == ASM_POP) {
											*instr_pop = ASM_DELOP;
										} else if (*instr_pop == ASM_ADJSTACK) {
											--*(int8_t *)(instr_pop + 1);
										} else {
											UNALIGNED_SETLE16(instr_pop + 2, (uint16_t)((int16_t)UNALIGNED_GETLE16(instr_pop + 2) - 1));
										}
										SET_RESULTF(iter, "Flatten repeated conditional jump at %.4I32X to %.4I32X",
										            (code_addr_t)(instr_jmp - sc_main.sec_begin),
										            (code_addr_t)(jmp_target - sc_main.sec_begin));
										goto done_opt_bool;
									}
								} else {
									int16_t new_disp;
									new_disp = *(int8_t *)(instr_jmp + 1);
									new_disp += target_size;
									if (new_disp >= INT8_MIN && new_disp <= INT8_MAX) {
										if (jmp_reloc) {
											ASSERT(jmp_reloc->ar_sym);
											ASSERT(ASM_SYM_DEFINED(jmp_reloc->ar_sym));
											ASSERT(jmp_reloc->ar_sym->as_used);
											ASSERT(jmp_reloc->ar_sym->as_stck == stacksz);
											if (jmp_reloc->ar_sym->as_used == 1) {
												jmp_reloc->ar_sym->as_addr += target_size;
												--jmp_reloc->ar_sym->as_stck;
											} else {
												struct asm_sym *newsym = asm_newsym();
												if unlikely(!newsym)
													goto err;
												newsym->as_addr = jmp_reloc->ar_sym->as_addr + target_size;
												newsym->as_stck = jmp_reloc->ar_sym->as_stck - 1;
												newsym->as_sect = jmp_reloc->ar_sym->as_sect;
												newsym->as_hand = jmp_reloc->ar_sym->as_hand;
												newsym->as_used = 1;
												--jmp_reloc->ar_sym->as_used;
												jmp_reloc->ar_sym = newsym;
											}
										} else {
											*(int8_t *)(instr_jmp + 1) = (int8_t)new_disp;
										}
										goto conditional_jump_forwarding_ok;
									}
								}
							} else if (jmp_reloc) {
								/* Follow the jump */
								instruction_t *second_target;
								int32_t disp;
								struct asm_rel *second_reloc;
								ASSERT(jmp_reloc->ar_sym);
								ASSERT(ASM_SYM_DEFINED(jmp_reloc->ar_sym));
								ASSERT(jmp_reloc->ar_sym->as_used);
								second_target = follow_jmp(jmp_target, &second_reloc);
								if (second_reloc) {
									disp = (int32_t)(second_target - instr_pop);
									if (JMP_IS16(instr_jmp)
									    ? (disp >= INT16_MIN && disp <= INT16_MAX)
									    : (disp >= INT8_MIN && disp <= INT8_MAX)) {
										if (jmp_reloc->ar_sym->as_used == 1)
											should_delete_unused_symbols = true;
										--jmp_reloc->ar_sym->as_used;
										++second_reloc->ar_sym->as_used;
										jmp_reloc->ar_sym = second_reloc->ar_sym;
										goto conditional_jump_forwarding_ok;
									}
								}
							}
						}
					}
				}
			}

			if (IP_ISCJMP(iiter)) {
				if (num_instruction != 0) {
delete_bool_before_jft:
					/* Merge the bool/not instructions into the conditional-jump instruction. */
					delete_assembly((code_addr_t)(iter - sc_main.sec_begin),
					                (code_size_t)(iiter - iter));
					/* Invert the jump condition if the bool inverted the test. */
					if (must_invert)
						*iiter ^= ASM_JX_NOTBIT;
					SET_RESULTF(iter, "Remove bool-cast(s) preceding to conditional jump at +%.4I32X",
					            (code_addr_t)(iiter - sc_main.sec_begin));
					goto done_opt_bool;
				}
			}
			if (num_instruction > 1) {
				/* Optimize:
				 * >> bool top
				 * >> not  top
				 * >> not  top
				 * >> bool top
				 * >> not  top
				 * Into:
				 * >> not  top
				 */
				delete_assembly((code_addr_t)(iter - sc_main.sec_begin),
				                (code_size_t)(iiter - iter));
				*iter = ASM_BOOL ^ (must_invert ? 1 : 0);
				SET_RESULTF(iter, "Flatten boolean logic repetition to a single `%s'",
				            must_invert ? "not" : STR_bool);
			}
done_opt_bool:
			iter = iiter;
			goto continue_at_iter;
		}	break;


#if 1
		case ASM_JF16:
		case ASM_JT16:
		case ASM_JF:
		case ASM_JT:
do_optimize_conditional_jump:
		{
			/* In order to properly use stack-based variables, all jump instructions must
			 * be generated to adjust the stack before a jump can actually be performed.
			 * Because of that, when stack-variables _are_ used, it gets kind-of complicated
			 * and a lot of stub code must be created in order to adjust the stack.
			 * The most generic case of a regular jump doesn't need to be handled here,
			 * as that one can intrinsically be optimized because it is written as:
			 * >>    adjstack #target.sp
			 * >>    jmp       target.ip
			 * On the other hand, conditional jumps are much more complicated and need a wrapper:
			 * >>    jf           target.ip
			 * Must be compiled as this when stack-variables are used:
			 * >>    jt           1f
			 * >>    adjstack #target.sp
			 * >>    jmp       target.ip
			 * >>1:
			 * We're here to optimize the latter when `asm_minjmp()' was able to
			 * determine the operand of `adjstack' being ZERO(0), allowing it to
			 * be deleted all-together, leaving us with code like this:
			 * >>    jt           1f
			 * >>    jmp          target.ip
			 * >>1:
			 * ... which we want to turn back into this:
			 * >>    jf           target.ip
			 * NOTE: Since the wrapper assembly used by conditional stack-jumps
			 *       is also required when generating 32-bit long jumps, we
			 *       intentionally don't optimize `ASM32_JMP' instructions,
			 *       so-as not to break bigcode mode. */
			iiter = DeeAsm_NextInstr(after_prefix);
			if ((*iiter == ASM_JMP || *iiter == ASM_JMP16) && !IS_PROTECTED(iiter)) {
				struct asm_rel *cond_rel;
				instruction_t *jmp_target = follow_jmp(after_prefix, &cond_rel);
				instruction_t *jmp_next   = next_instr(iiter);
				/* Make sure that the conditional jump points after the unconditional one. */
				if (jmp_target != jmp_next)
					break;
				/* Delete the wrapper jump instruction (and associated relocation). */
				if (cond_rel)
					asm_reldel(cond_rel);
				if (opcode & 1)
					after_prefix[2] = ASM_DELOP;
				after_prefix[1] = ASM_DELOP;
				after_prefix[0] = ASM_DELOP;
				/* Invert the jump condition and move the instruction forward. */
				opcode &= ~1;                     /* Turn into an 8-bit instruction. */
				opcode ^= ASM_JX_NOTBIT;          /* Invert the test. */
				opcode |= iiter[0] & 1;           /* Copy the 16-bit operand flag of the following unconditional jump. */
				iiter[0] = (instruction_t)opcode; /* Change an unconditional jump to one that is conditional. */
				SET_RESULTF(iter, "Merge conditional double-jump around +%.4I32X to +%.4I32X (`j%c 1f; jmp 2f; 1:' -> `j%c 2f')",
				            (code_addr_t)(iiter - sc_main.sec_begin),
				            (code_addr_t)(jmp_next - sc_main.sec_begin),
				            ((opcode & ~1) == ASM_JT) ? 'f' : 't',
				            ((opcode & ~1) == ASM_JT) ? 't' : 'f');
				iter = iiter; /* Continue by parsing the newly overwritten instruction. */
			}
		}	break;
#endif

		case ASM16_EXTERN:
			after_prefix = iiter + 4;
			goto do_optimize_after_prefix;
		case ASM_LOCAL:
		case ASM_GLOBAL:
		case ASM_STATIC:
		case ASM_STACK:
			after_prefix = iiter + 1;
			goto do_optimize_after_prefix;
		case ASM_EXTERN:
		case ASM16_LOCAL:
		case ASM16_GLOBAL:
		case ASM16_STATIC:
		case ASM16_STACK:
			after_prefix = iiter + 2;
do_optimize_after_prefix:
			opcode = *after_prefix++;
do_switch_after_prefix_opcode:
			switch (opcode) {

			CASE_ASM_EXTENDED:
				opcode <<= 8;
				opcode |= *after_prefix++;
				goto do_switch_after_prefix_opcode;

#if 1
			case ASM_JF:
			case ASM_JF16:
			case ASM_JT:
			case ASM_JT16:
				goto do_optimize_conditional_jump;
#endif

			default: break;
			}
			break;


#if 1
		{
			uint16_t last_operand_address, continue_sp, iiter_sp;
			uint16_t positive_instruction_sp;
			instruction_t *positive_instruction, *continue_ip;
			instruction_t *next_instruction;
			bool contains_stack_max;
			/* Instructions without any guarantied side-effect
			 * that have a positive stack-effect of >= `+1'. */
		case ASM_ADJSTACK:
			if (*(int8_t *)(iiter + 0) > 0)
				goto do_unused_operand_optimization;
			goto do_pop_merge_optimization;

		case ASM16_ADJSTACK:
			if ((int16_t)UNALIGNED_GETLE16(iiter + 0) > 0)
				goto do_unused_operand_optimization;
			goto do_pop_merge_optimization;

		case ASM_PUSH_NONE:
		case ASM_PUSH_TRUE:
		case ASM_PUSH_FALSE:
		case ASM_PUSH_EXCEPT:
		case ASM_PUSH_MODULE:
		case ASM16_PUSH_MODULE:
		case ASM_PUSH_THIS:
		case ASM_PUSH_THIS_MODULE:
		case ASM_PUSH_THIS_FUNCTION:
		case ASM_PUSH_REF:
		case ASM16_PUSH_REF:
		case ASM_PUSH_ARG:
		case ASM16_PUSH_ARG:
		case ASM_PUSH_CONST:
		case ASM16_PUSH_CONST:
		case ASM_PUSH_STATIC:
		case ASM16_PUSH_STATIC:
		case ASM_PUSH_EXTERN:
		case ASM16_PUSH_EXTERN:
		case ASM_PUSH_GLOBAL:
		case ASM16_PUSH_GLOBAL:
		case ASM_PUSH_LOCAL:
		case ASM16_PUSH_LOCAL:
			/* Optimization:
			 * >> push arg @foo
			 * >> pop  local @bar
			 * Into:
			 * >> mov  local @bar, arg @foo
			 */
			next_instruction = DeeAsm_NextInstr(iter);
			if (!IS_PROTECTED(next_instruction)) {
				switch (next_instruction[0]) {
					instruction_t temp[12];

				case ASM_POP_LOCAL:
					temp[0] = ASM_LOCAL;
					goto do_optimize_popmov_8bit;

				case ASM_POP_GLOBAL:
					temp[0] = ASM_GLOBAL;
					goto do_optimize_popmov_8bit;

				case ASM_POP_STATIC:
					temp[0] = ASM_STATIC;
do_optimize_popmov_8bit:
					temp[1] = next_instruction[1];
					memcpyc(temp + 2, iter, next_instruction - iter, sizeof(instruction_t));
					memcpyc(iter, temp, 2 + (next_instruction - iter), sizeof(instruction_t));
					SET_RESULTF(iter, "Construct mov-instruction from push+pop");
					goto continue_at_iter;

				case ASM_POP_EXTERN:
					temp[0] = ASM_EXTERN;
					temp[1] = next_instruction[1];
					temp[2] = next_instruction[2];
					memcpyc(temp + 3, iter, next_instruction - iter, sizeof(instruction_t));
					memcpyc(iter, temp, 3 + (next_instruction - iter), sizeof(instruction_t));
					SET_RESULTF(iter, "Construct mov-instruction from push+pop extern");
					goto continue_at_iter;

				case ASM_POP_N: {
					uint16_t abs_sp;
					abs_sp = (stacksz + 1) - (UNALIGNED_GETLE8(next_instruction + 1) + 2);
					if (abs_sp <= UINT8_MAX) {
						temp[0] = ASM_STACK;
						temp[1] = (uint8_t)abs_sp;
						memcpyc(temp + 2, iter, next_instruction - iter, sizeof(instruction_t));
						memcpyc(iter, temp, 2 + (next_instruction - iter), sizeof(instruction_t));
						SET_RESULTF(iter, "Construct mov-instruction from push+pop stack");
						goto continue_at_iter;
					}
				}	break;

				case ASM_EXTENDED1:
					switch (next_instruction[1]) {

					case ASM16_POP_LOCAL & 0xff:
						temp[1] = ASM16_LOCAL & 0xff;
						goto do_optimize_popmov_16bit;

					case ASM16_POP_GLOBAL & 0xff:
						temp[1] = ASM16_GLOBAL & 0xff;
						goto do_optimize_popmov_16bit;

					case ASM16_POP_STATIC & 0xff:
						temp[1] = ASM16_STATIC & 0xff;
do_optimize_popmov_16bit:
						temp[0] = (ASM16_STATIC & 0xff00) >> 8;
						temp[2] = next_instruction[1];
						temp[3] = next_instruction[2];
						memcpyc(temp + 4, iter, next_instruction - iter, sizeof(instruction_t));
						memcpyc(iter, temp, 4 + (next_instruction - iter), sizeof(instruction_t));
						SET_RESULTF(iter, "Construct 16-bit mov-instruction from push+pop");
						goto continue_at_iter;

					case ASM16_POP_EXTERN & 0xff:
						temp[0] = (ASM16_EXTERN & 0xff00) >> 8;
						temp[1] = ASM16_EXTERN & 0xff;
						temp[2] = next_instruction[2];
						temp[3] = next_instruction[3];
						temp[4] = next_instruction[4];
						temp[5] = next_instruction[5];
						memcpyc(temp + 6, iter, next_instruction - iter, sizeof(instruction_t));
						memcpyc(iter, temp, 6 + (next_instruction - iter), sizeof(instruction_t));
						SET_RESULTF(iter, "Construct 16-bit mov-instruction from push+pop extern");
						goto continue_at_iter;

					case ASM16_POP_N & 0xff: {
						uint16_t abs_sp;
						abs_sp  = (stacksz + 1) - (UNALIGNED_GETLE16(next_instruction + 1) + 2);
						temp[0] = (ASM16_STACK & 0xff00) >> 8;
						temp[1] = ASM16_STACK & 0xff;
						temp[2] = abs_sp & 0xff;
						temp[3] = (abs_sp & 0xff00) >> 8;
						memcpyc(temp + 4, iter, next_instruction - iter, sizeof(instruction_t));
						memcpyc(iter, temp, 4 + (next_instruction - iter), sizeof(instruction_t));
						SET_RESULTF(iter, "Construct 16-bit mov-instruction from push+pop stack");
						goto continue_at_iter;
					}	break;

					default: break;
					}
					break;

				default: break;
				}
			}
			ATTR_FALLTHROUGH
		case ASM_PUSH_BND_EXTERN:
		case ASM16_PUSH_BND_EXTERN:
		case ASM_PUSH_BND_GLOBAL:
		case ASM16_PUSH_BND_GLOBAL:
		case ASM_PUSH_BND_STATIC:
		case ASM16_PUSH_BND_STATIC:
		case ASM_PUSH_BND_LOCAL:
		case ASM16_PUSH_BND_LOCAL:
		case ASM_SUPER_THIS_R:
		case ASM16_SUPER_THIS_R:
		case ASM_CLASS_GC:
		case ASM16_CLASS_GC:
		case ASM_CLASS_EC:
		case ASM16_CLASS_EC:
		case ASM_FUNCTION_C:
		case ASM16_FUNCTION_C:
		case ASM_FUNCTION_C_16:
		case ASM16_FUNCTION_C_16:
		case ASM_RANGE_0_I16:
		case ASM_RANGE_0_I32:
		case ASM_RANGE:
		case ASM_RANGE_DEF:
		case ASM_RANGE_STEP:
		case ASM_RANGE_STEP_DEF:
		case ASM_GETMEMBER_THIS:
		case ASM16_GETMEMBER_THIS:
		case ASM_GETMEMBER_THIS_R:
		case ASM16_GETMEMBER_THIS_R:
			/* Optimize the following:
			 *  >> dup
			 *  >> *    // Any sequence of code that doesn't make use of absolute
			 *  >>      // stack offsets >= the stacksz value before `dup' and
			 *  >>      // doesn't access any stack object below dup.
			 *  >>      // With these restrictions, the code must be able to run
			 *  >>      // without regards to the currently stack alignment, so long
			 *  >>      // that it doesn't exceed specifications (max-stack-size).
			 *  >> pop
			 * This is the optimization that implements what is referred to in
			 * the documentation of `ASM_FSTACKDISP' to optimize the following:
			 *  >>    dup      #0      // Copy `x' for print
			 *  >>    print    pop, nl // Print `x'
			 *  >>    adjstack #SP - 1 // Adjust to fix the stack before the following jump. (actually assembled as `pop')
			 * ... Into this:
			 *  >>    print    pop, nl // Print `x'
			 * NOTE: Remember that if `current_assembler.a_stackmax' is located
			 *       within `*' above, then we could re-calculated the max required
			 *       stack size of the entire assembly.
			 * NOTE: This optimization also gets rid of unused pushes such as:
			 *  >>   push      $10
			 *  >>   pop
			 */
do_unused_operand_optimization:
			iiter_sp = stacksz;
			/* Figure out the absolute stack-address of the
			 * greatest operand created by this instruction. */
			validate_stack_depth((code_addr_t)(iter - sc_main.sec_begin), stacksz);
			next_instruction = DeeAsm_NextInstrSp(iter, &stacksz);
			validate_stack_depth((code_addr_t)(next_instruction - sc_main.sec_begin), stacksz);
			/* The address of the last operand.
			 * If the stack argument associated with this address is popped
			 * in a way that discards its value, then we can optimize it away
			 * entirely.
			 * NOTE: We must not perform this optimization when
			 *       encountering a protected instruction.
			 *       We could theoretically handle branching instructions
			 *       and check if both branches discard the value as unused,
			 *       but that would be too complicated for right now... */
			last_operand_address = stacksz - 1;
			__IF0 {
				/* Special handling for DUP instructions.
				 * For these, we already know an origin offset. */
		case ASM_DUP:
				next_instruction = iter + 1;
				ASSERT(stacksz >= 1);
				iiter_sp             = stacksz;
				last_operand_address = stacksz - 1;
				goto do_check_dup_into;
		case ASM_DUP_N:
				next_instruction = iter + 2;
				ASSERT(stacksz >= (UNALIGNED_GETLE8(iter + 1) + 2));
				iiter_sp             = stacksz;
				last_operand_address = stacksz - (UNALIGNED_GETLE8(iter + 1) + 2);
				goto do_check_dup_into;
		case ASM16_DUP_N:
				next_instruction = iter + 4;
				ASSERT(stacksz >= (UNALIGNED_GETLE16(iter + 2) + 2));
				iiter_sp             = stacksz;
				last_operand_address = stacksz - (UNALIGNED_GETLE16(iter + 2) + 2);
do_check_dup_into:
				/* Optimize:
				 * >> dup
				 * >> pop  local @x
				 * into:
				 * >> mov  local @x, top
				 */
				if (!IS_PROTECTED(next_instruction)) {
					instruction_t temp[10];
					switch (next_instruction[0]) {

					case ASM_POP_LOCAL:
						temp[0] = ASM_LOCAL;
						goto do_optimize_dupmov_8bit;
					case ASM_POP_GLOBAL:
						temp[0] = ASM_GLOBAL;
						goto do_optimize_dupmov_8bit;
					case ASM_POP_STATIC:
						temp[0] = ASM_STATIC;
do_optimize_dupmov_8bit:
						temp[1] = next_instruction[1];
						memcpyc(temp + 2, iter, next_instruction - iter, sizeof(instruction_t));
						memcpyc(iter, temp, 2 + (next_instruction - iter), sizeof(instruction_t));
						SET_RESULTF(iter, "Construct mov-instruction from dup+pop");
						goto continue_at_iter;

					case ASM_POP_EXTERN:
						temp[0] = ASM_EXTERN;
						temp[1] = next_instruction[1];
						temp[2] = next_instruction[2];
						memcpyc(temp + 3, iter, next_instruction - iter, sizeof(instruction_t));
						memcpyc(iter, temp, 3 + (next_instruction - iter), sizeof(instruction_t));
						SET_RESULTF(iter, "Construct mov-instruction from dup+pop extern");
						goto continue_at_iter;

					case ASM_POP_N: {
						uint16_t abs_sp;
						ASSERT((stacksz + 1) >= 2);
						abs_sp = (stacksz + 1) - (UNALIGNED_GETLE8(next_instruction + 1) + 2);
						if (abs_sp == last_operand_address) {
							memset(iter, ASM_DELOP, (size_t)((next_instruction + 2) - iter));
							SET_RESULTF(iter, "Remove redundant `dup; pop #SP-2' instruction pair");
							goto continue_at_iter;
						} else if (abs_sp <= UINT8_MAX) {
							temp[0] = ASM_STACK;
							temp[1] = (uint8_t)abs_sp;
							memcpyc(temp + 2, iter, next_instruction - iter, sizeof(instruction_t));
							memcpyc(iter, temp, 2 + (next_instruction - iter), sizeof(instruction_t));
							SET_RESULTF(iter, "Construct mov-instruction from dup+pop stack");
							goto continue_at_iter;
						}
					}	break;

					case ASM_EXTENDED1:
						switch (next_instruction[1]) {

						case ASM16_POP_LOCAL & 0xff:
							temp[1] = ASM16_LOCAL & 0xff;
							goto do_optimize_dupmov_16bit;
						case ASM16_POP_GLOBAL & 0xff:
							temp[1] = ASM16_GLOBAL & 0xff;
							goto do_optimize_dupmov_16bit;
						case ASM16_POP_STATIC & 0xff:
							temp[1] = ASM16_STATIC & 0xff;
do_optimize_dupmov_16bit:
							temp[0] = (ASM16_STATIC & 0xff00) >> 8;
							temp[2] = next_instruction[2];
							temp[3] = next_instruction[3];
							memcpyc(temp + 4, iter, next_instruction - iter, sizeof(instruction_t));
							memcpyc(iter, temp, 4 + (next_instruction - iter), sizeof(instruction_t));
							SET_RESULTF(iter, "Construct 16-bit mov-instruction from dup+pop");
							goto continue_at_iter;

						case ASM16_POP_EXTERN & 0xff:
							temp[0] = (ASM16_EXTERN & 0xff00) >> 8;
							temp[1] = ASM16_EXTERN & 0xff;
							temp[2] = next_instruction[2];
							temp[3] = next_instruction[3];
							temp[4] = next_instruction[4];
							temp[5] = next_instruction[5];
							memcpyc(temp + 6, iter, next_instruction - iter, sizeof(instruction_t));
							memcpyc(iter, temp, 6 + (next_instruction - iter), sizeof(instruction_t));
							SET_RESULTF(iter, "Construct 16-bit mov-instruction from dup+pop extern");
							goto continue_at_iter;

						case ASM16_POP_N & 0xff: {
							uint16_t abs_sp;
							ASSERT((stacksz + 1) >= 2);
							abs_sp = (stacksz + 1) - (*(uint16_t *)(next_instruction + 2) + 2);
							if (abs_sp == last_operand_address) {
								memset(iter, ASM_DELOP, (size_t)((next_instruction + 4) - iter));
								SET_RESULTF(iter, "Remove redundant `dup; pop #SP-2' instruction pair");
							} else {
								temp[0] = (ASM16_STACK & 0xff00) >> 8;
								temp[1] = ASM16_STACK & 0xff;
								temp[2] = abs_sp & 0xff;
								temp[3] = (abs_sp & 0xff00) >> 8;
								memcpyc(temp + 4, iter, next_instruction - iter, sizeof(instruction_t));
								memcpyc(iter, temp, 4 + (next_instruction - iter), sizeof(instruction_t));
								SET_RESULTF(iter, "Construct 16-bit mov-instruction from dup+pop stack");
							}
							goto continue_at_iter;
						}	break;

						default: break;
						}
						break;

					default: break;
					}
				}
				++stacksz;
				goto do_unused_operand_optimization_ex;
			}
do_unused_operand_optimization_ex:
			validate_stack_depth((code_addr_t)(next_instruction - sc_main.sec_begin), stacksz);
			/* TODO: Optimize:
			 * >>    push $42
			 * >>    mov  #SP - 1, $15
			 * Into:
			 * >>    push $15
			 */

#if 1
			/* Do some special handling to optimize the value being discarded immediately. */
			if ((next_instruction[0] == ASM_POP ||
			     (next_instruction[0] == ASM_ADJSTACK && (int8_t)UNALIGNED_GETLE8(next_instruction + 1) < 0) ||
			     (next_instruction[0] == ASM_EXTENDED1 &&
			      next_instruction[1] == ASM_ADJSTACK &&
			      (int16_t)UNALIGNED_GETLE16(next_instruction + 2) < 0)) &&
			    !IS_PROTECTED(next_instruction)) {

				/* Optimize stuff like `dup; pop' */
				int16_t total_adjustment = stacksz - iiter_sp;
				instruction_t *continue_after;
				if (next_instruction[0] == ASM_POP) {
					total_adjustment -= 1;
				} else if (next_instruction[0] == ASM_ADJSTACK) {
					total_adjustment += (int8_t)UNALIGNED_GETLE8(next_instruction + 1);
				} else {
					total_adjustment += (int16_t)UNALIGNED_GETLE16(next_instruction + 2);
				}

				/* Delete existing assembly. */
				continue_after = DeeAsm_NextInstr(next_instruction);
				delete_assembly((code_addr_t)(iter - sc_main.sec_begin),
				                (code_size_t)(continue_after - iter));

				/* Write some new assembly to do this adjustment. */
				switch (total_adjustment) {

				case -2:
					iter[1] = ASM_POP;
					ATTR_FALLTHROUGH
				case -1:
					iter[0] = ASM_POP;
					break;

				case 0:
					break;

				case 2:
					iter[1] = ASM_PUSH_NONE;
					ATTR_FALLTHROUGH
				case 1:
					iter[0] = ASM_PUSH_NONE;
					break;

				default:
					/* Default case: use a stack-alignment instruction. */
					if (total_adjustment >= INT8_MIN &&
					    total_adjustment <= INT8_MAX) {
						iter[0] = ASM_ADJSTACK;
						UNALIGNED_SETLE8(iter + 1, (uint8_t)(int8_t)total_adjustment);
					} else {
						iter[0] = (ASM16_ADJSTACK & 0xff00) >> 8;
						iter[1] = (ASM16_ADJSTACK & 0xff);
						UNALIGNED_SETLE16(iter + 2, (uint16_t)(int16_t)total_adjustment);
					}
					break;
				}
				SET_RESULTF(iter, "Remove unused push instruction");
				stacksz = iiter_sp + total_adjustment;
				iter    = continue_after;
				goto continue_at_iter;
			}
#endif

			continue_ip = next_instruction;
			continue_sp = stacksz;
			validate_stack_depth((code_addr_t)(continue_ip - sc_main.sec_begin), continue_sp);
			ASSERT(stacksz != 0);
			iiter = positive_instruction = iter;
			positive_instruction_sp      = stacksz;
			iter                         = next_instruction;
			contains_stack_max           = false;
			/* Enumerate following instructions. */
			if (opcode == ASM_DUP)
				while (iter < end && !IS_PROTECTED(iter)) {
					uint16_t sp_add, sp_sub;
					instruction_t *next;
					uint16_t old_stacksz;
					/* Handle unconditional jumps. */
					if (IP_ISJMP(iter)) {
						next = follow_jmp(iter, NULL);
						if unlikely(next <= iter)
							break; /* Let's not deal with this... */
						/* NOTE: Since we stop on any conditional jump (see below),
						 *       we are actually allowed to follow unconditional ones. */
						iter = next;
						continue;
					}
					opcode = *iter;
					/* We don't handle conditional branching here (See above) */
					if (OP_ISCJMP(opcode))
						break;
					/* Don't deal with variable jumps. */
					if (opcode == ASM_JMP_POP)
						break;
					/* Don't deal with instructions that don't return. */
					if (opcode == ASM_RET_NONE || opcode == ASM_THROW ||
					    opcode == ASM_UD || opcode == ASM_RETHROW ||
					    (opcode == ASM_RET &&
					     !(current_basescope->bs_flags & Dee_CODE_FYIELDING)))
						break;
					if (opcode == ASM_EXTENDED1) {
						opcode = iter[1];
						/* Don't deal with variable jumps. */
						if (opcode == (ASM_JMP_POP_POP & 0xff))
							break;
					}


					old_stacksz = stacksz;
					/* Figure out the next instruction, as well as the precise the stack-effect of this one.
					 * We need to know this for certain because we need to know if the instruction can
					 * affect the operand we're trying to optimize away. */
					validate_stack_depth((code_addr_t)(iter - sc_main.sec_begin), stacksz);
					next = DeeAsm_NextInstrEf(iter, &stacksz, &sp_add, &sp_sub);
					validate_stack_depth((code_addr_t)(next - sc_main.sec_begin), stacksz);
					ASSERT(stacksz <= current_assembler.a_stackmax);
					ASSERT(old_stacksz >= sp_sub);
					if (stacksz == current_assembler.a_stackmax)
						contains_stack_max = true;
					if (old_stacksz - sp_sub <= last_operand_address) {
						/* Instructions such as `dup' and `dup #n' may use our operand as
						 * an input argument, but for all we are concerned, they don't count as uses.
						 * After all: The main idea here is to optimize assembly like this:
						 * >> push  $10
						 * >> dup           // This is the `dup' in question.
						 * >> print pop, nl
						 * >> pop
						 * Instead, we must interpret the `dup' as being an alias for the `push' above,
						 * and delete it, as well as the `pop' when going to perform the optimization.
						 */
						if (opcode == ASM_EXTENDED1)
							opcode = iter[1];
						if ((opcode == ASM_DUP && (old_stacksz - 1) == last_operand_address)
#if 0 /* This optimization only works for immediate-level operands */
						    ||
						    (opcode == ASM_DUP_N && (old_stacksz - sp_sub) == last_operand_address)
#endif
						    ) {
							/* Inner recursion of symbol aliasing. */
							positive_instruction    = iter;
							next_instruction        = next;
							positive_instruction_sp = stacksz;
							goto continue_unused_operand;
						}
#if 1 /* This could be done, but it would require us to track all instructions     \
       * that use operands beneath our own, so that we can re-adjust their offsets \
       * in the event that code is going to be deleted.                            \
       * Yet since we don't do that, we can't handle this case. */
						if (opcode == ASM_DUP_N && (old_stacksz - sp_sub) != last_operand_address) {
							/* An operand further down the stack got copied.
							 * We do not need to care about this in particular,
							 * because it didn't affect our operand! */
							goto continue_unused_operand;
						}
#endif

						/* Found an instruction that affects stack items
						 * past the one we're trying to optimize.
						 * Now lets check if the instruction discards the argument
						 * without side-effects, and if we can re-write it to where
						 * it simply ignores the operand. */
						if (opcode == ASM_POP_N && last_operand_address == stacksz - 1 &&
						    (iter[0] == ASM_EXTENDED1 ? UNALIGNED_GETLE16(iter + 2) == 0 : (UNALIGNED_GETLE8(iter + 1)) == 0)) {
							/* Optimize something like this by deleting it:
							 * >> dup
							 * >> pop #1 // Replace the second entry (source operand of dup) with the same value (aka. a noop)
							 */
							if (iter[0] == ASM_EXTENDED1) {
								iter[1] = ASM_DELOP;
								iter[2] = ASM_DELOP;
								iter[3] = ASM_DELOP;
							} else {
								iter[1] = ASM_DELOP;
							}
							goto delete_asm_after_pop;
						} else if (opcode == ASM_POP) {
							/* The parade-example: `pop'
							 * In this situation, the assembly may look something like this:
							 * >> push $10
							 * >> dup           // This is the instruction that started all of this
							 * >> print pop, nl
							 * >> pop           // This is where we are right now
							 * As the result of user-code that may look like this:
							 * >> __stack local foo = 10;
							 * >> print foo;
							 * Now we want to optimize it into this:
							 * >> push $10
							 * >> print pop, nl
							 */
delete_asm_after_pop:
							delete_assembly((code_addr_t)(positive_instruction - sc_main.sec_begin),
							                (code_size_t)(next_instruction - positive_instruction));
							iter[0] = ASM_DELOP;
							goto set_result_for_pop;
						} else if (opcode == ASM_ADJSTACK) {
							/* Similar to `pop': `adjstack' with a negative
							 * operand can be used to pop multiple values. */
							delete_assembly((code_addr_t)(positive_instruction - sc_main.sec_begin),
							                (code_size_t)(next_instruction - positive_instruction));
							/* Add +1 to the stack adjustment (which must be negative) to subtract one less stack object */
							if (iter[0] == ASM_EXTENDED1) {
								int16_t offset;
								offset = (int16_t)UNALIGNED_GETLE16(iter + 2) + 1;
								UNALIGNED_SETLE16(iter + 2, (uint16_t)offset);
								if (offset == 0) {
									iter[0] = ASM_DELOP;
									iter[1] = ASM_DELOP;
									iter[2] = ASM_DELOP;
									iter[3] = ASM_DELOP;
								}
							} else {
								if ((*(int8_t *)(iter + 1) += 1) == 0) {
									iter[0] = ASM_DELOP;
									iter[1] = ASM_DELOP;
								}
							}
set_result_for_pop:
							SET_RESULTF(next_instruction,
							            "Refactor stack to localize producer and consumer at #%.4I32X",
							            (code_addr_t)(iter - sc_main.sec_begin));
							should_reload_stack |= contains_stack_max;
							if (iiter == positive_instruction)
								--continue_sp;
							/* Adjust absolute & cross-SP references in the
							 * code block between the 2 modified instructions.
							 * This fixes SP-references */
							decrement_stack_referenes(next_instruction, iter,
							                          positive_instruction_sp);
						}
						break;
					}
continue_unused_operand:
					iter = next;
				}
			/* Continue regular peephole optimization at the address
			 * following the `push' we've failed to optimize away. */
			iter    = continue_ip;
			stacksz = continue_sp;
			validate_stack_depth((code_addr_t)(iter - sc_main.sec_begin), stacksz);
			if (*iiter == ASM_PUSH_NONE) {
				iter = iiter;
				--stacksz;
				goto do_pop_merge_optimization;
			}
			goto continue_at_iter;
		}
		break;
#endif

		{
			int16_t effect_sum;
			instruction_t *effect_start;
			uint16_t effect_instructions;
			/* Merge consecutive adjstack/pop/push instructions.
			 * This kind of thing can easily happen in code like this:
			 * >> {   __stack local a = 10;         // `push $10'
			 * >>     {   __stack local b = 20;     // `push $20'
			 * >>         {   __stack local c = 30; // `push $30'
			 * >>         }                         // `pop'
			 * >>     }                             // `pop'
			 * >> }                                 // `pop'
			 * Optimize `pop;pop;pop' to `adjstack #SP - 3'
			 * ... But even more easily once peephole optimization
			 *     has already performed a bunch of optimizations. */
		case ASM_POP:
do_pop_merge_optimization:
			/* Calculate the sum of all stack alignment instructions,
			 * traversing all instructions until one with a non-zero
			 * sp_sub effect is encountered. */
			effect_sum          = 0;
			effect_start        = iter;
			effect_instructions = 0;
			for (; iter < end; ++effect_instructions) {
				if (IS_PROTECTED(iter))
					break; /* Don't merge protected instructions. */
				opcode = *iter++;
				if (opcode == ASM_PUSH_NONE) {
					/* NOTE: When a `push none' follows after `pop', then we must
					 *       not optimize it away because the assembly may be used
					 *       to first pop some object, then replace it with `none'. */
					if (effect_sum < 0)
						break;
					++effect_sum;
					continue;
				}
				if (opcode == ASM_POP) {
					--effect_sum;
					continue;
				}
				if (opcode == ASM_ADJSTACK) {
					int8_t effect = *(int8_t *)(iter + 0);
					if (effect > 0 && effect_sum < 0)
						break;
					effect_sum += effect;
					++iter;
					continue;
				}
				if (opcode == ASM_EXTENDED1 &&
				    iter[0] == (ASM16_ADJSTACK & 0xff)) {
					int16_t effect = (int16_t)UNALIGNED_GETLE16((uint16_t *)(iter + 1));
					if (effect > 0 && effect_sum < 0)
						break;
					effect_sum += effect;
					iter += 3;
					continue;
				}
				--iter;
				break;
			}
			if (effect_instructions > 1 &&
			    (effect_instructions != 2 || (effect_sum != -2 && effect_sum != 2))) {
				/* With more than one instruction at hand, we can do some merging!
				 * First off: Delete all existing code detailing
				 *            multiple consecutive alignment instructions. */
				delete_assembly((code_addr_t)(effect_start - sc_main.sec_begin),
				                (code_size_t)(iter - effect_start));
				/* Now just write some new code using only
				 * a single instruction to do the same job. */
				switch (effect_sum) {

				case -2:
					*(effect_start + 1) = ASM_POP;
					ATTR_FALLTHROUGH
				case -1:
					*(effect_start + 0) = ASM_POP;
					break;

				case 0:
					break; /* nothing to do here. */

				case 2:
					*(effect_start + 1) = ASM_PUSH_NONE;
					ATTR_FALLTHROUGH
				case 1:
					*(effect_start + 0) = ASM_PUSH_NONE;
					break;

				default:
					if (effect_sum >= INT8_MIN && effect_sum <= INT8_MAX) {
						/* 8-bit stack alignment. */
						*(effect_start + 0)           = ASM_ADJSTACK;
						*(int8_t *)(effect_start + 1) = (int8_t)effect_sum;
					} else {
						/* 16-bit stack alignment. */
						*(effect_start + 0) = (instruction_t)((ASM16_ADJSTACK & 0xff00) >> 8);
						*(effect_start + 1) = (instruction_t)(ASM16_ADJSTACK & 0xff);
						UNALIGNED_SETLE16(effect_start + 2, (uint16_t)effect_sum);
					}
					break;
				}
				SET_RESULTF(effect_start, "Flatten neighboring stack-effect instructions");
			}
			iter = effect_start;
			if (*iter == ASM_PUSH_NONE && !(current_basescope->bs_flags & Dee_CODE_FYIELDING)) {
				instruction_t *next;
				/* Minor optimization: In non-yielding functions, we can
				 *                     optimize `push none; ret pop' to `ret none'
				 *                     In yielding functions however, `ret pop' doesn't
				 *                     exist, meaning that we can't perform the optimization. */
				next = next_instr_sp(iter, &stacksz);
				if (*next == ASM_RET && !IS_PROTECTED(next)) {
					if (stacksz == current_assembler.a_stackmax)
						should_reload_stack = true;
					*iter = ASM_RET_NONE;
					*next = ASM_DELOP;
					SET_RESULT2(iter, next);
					--stacksz;
				}
				iter = next;
				validate_stack_depth((code_addr_t)(iter - sc_main.sec_begin), stacksz);
				goto continue_at_iter;
			}
		}	break;


		{
			instruction_t *next;
			uint16_t next_stacksz;
			uint16_t pop_imma, pop_immb, next_op;
			/* Optimize write+read into dup+write:
			 * >> push  $10
			 * >> pop   local @foo
			 * >> push  local @foo
			 * >> print pop, nl
			 * The associated user-code may look like this:
			 * >> local foo = 10;
			 * >> print foo;
			 *
			 * We can however optimize this assembly to the following:
			 * >> push  $10
			 * >> dup
			 * >> pop   local @foo
			 * >> print pop, nl
			 * ... At which point later optimization passes may notice that the
			 *     local variable `foo' is never read from following the point of
			 *     its assignment (and no DDI information is being created),
			 *     further optimizing the code into this:
			 *     NOTE: Only happens for `local' variables if not read from
			 *           after the assignment, or `static' variables when
			 *           never read from at all.
			 * >> push  $10
			 * >> dup
			 * >> pop
			 * >> print pop, nl
			 *
			 * Which can then be optimized again into:
			 * >> push  $10
			 * >> print pop, nl
			 *
			 * And finally, optimized into:
			 * >> print $10, nl
			 */
		case ASM_POP_EXTERN:
			pop_imma = UNALIGNED_GETLE8(iter + 1);
			pop_immb = *(uint8_t *)(iter + 2);
			goto do_pop_push_optimization2;

		case ASM16_POP_EXTERN:
			pop_imma = UNALIGNED_GETLE16(iter + 2);
			pop_immb = UNALIGNED_GETLE16(iter + 4);
			goto do_pop_push_optimization2;

		case ASM_POP_STATIC:
		case ASM_POP_GLOBAL:
		case ASM_POP_LOCAL:
			pop_imma = UNALIGNED_GETLE8(iter + 1);
			goto do_pop_push_optimization;

		case ASM16_POP_STATIC:
		case ASM16_POP_GLOBAL:
		case ASM16_POP_LOCAL:
			pop_imma = UNALIGNED_GETLE16(iter + 2);
do_pop_push_optimization:
			pop_immb = 0;
do_pop_push_optimization2:
			validate_stack_depth((code_addr_t)(iter - sc_main.sec_begin), stacksz);
			next_stacksz = stacksz;
			next         = next_instr_sp(iter, &next_stacksz);
			validate_stack_depth((code_addr_t)(next - sc_main.sec_begin), next_stacksz);
			/* If the next instruction is protected, then we can't do anything. */
			if (/*IS_PROTECTED(iter) || */ IS_PROTECTED(next))
				goto do_writeonly_symbol_optimization;
			next_op = next[0];
			if (ASM_ISEXTENDED(next_op))
				next_op = (next_op << 8) | next[1];
			if (next_op == opcode + 0x10) {
				uint16_t next_op_imma, next_op_immb = 0;
				if (next_op == ASM16_PUSH_GLOBAL) {
					next_op_imma = UNALIGNED_GETLE16((uint16_t *)(next + 2));
					next_op_immb = UNALIGNED_GETLE16((uint16_t *)(next + 4));
				} else if (next_op == ASM_PUSH_GLOBAL) {
					next_op_imma = *(uint8_t *)(next + 1);
					next_op_immb = *(uint8_t *)(next + 2);
				} else if (next_op & 0xff00) {
					next_op_imma = UNALIGNED_GETLE16((uint16_t *)(next + 2));
				} else {
					next_op_imma = *(uint8_t *)(next + 1);
				}
				if (pop_imma == next_op_imma &&
				    pop_immb == next_op_immb) {
					/* pop ?; push ?; --> dup; pop ?;
					 * NOTE: If the current stack-depth was the old
					 *       maximum, we must increase the max-depth! */
					next = DeeAsm_NextInstr(next);
					delete_assembly((code_addr_t)(iter - sc_main.sec_begin),
					                (code_size_t)(next - iter));
					stacksz      = next_stacksz + 2;
					next_stacksz = stacksz - 1;
					ASSERT(current_assembler.a_stackmax >= stacksz - 1);
					if (current_assembler.a_stackmax == stacksz - 1)
						current_assembler.a_stackmax += 1;
					/* Re-write the new assembly. */
					*iter++ = ASM_DUP;
					if (opcode & 0xff00) {
						*(iter + 0) = (instruction_t)(opcode >> 8);
						*(iter + 1) = (instruction_t)opcode;
						UNALIGNED_SETLE16(iter + 2, pop_imma);
						if ((opcode & 0xff) == (ASM16_POP_EXTERN & 0xff))
							UNALIGNED_SETLE16(iter + 4, pop_immb);
					} else {
						*(iter + 0) = (instruction_t)opcode;
						UNALIGNED_SETLE8(iter + 1, (uint8_t)pop_imma);
						if ((opcode & 0xff) == (ASM16_POP_EXTERN & 0xff))
							UNALIGNED_SETLE8(iter + 2, (uint8_t)pop_immb);
					}
					SET_RESULTF(iter - 1, "Optimize `pop x; push x' into `dup; pop x'");
					validate_stack_depth((code_addr_t)(iter - sc_main.sec_begin), stacksz);
				}
			} else if ((next_op & 0xff00) == (opcode & 0xff00) &&
			           (((next_op & 0xff) == ASM_LOCAL && (opcode & 0xff) == ASM_POP_LOCAL) ||
			            ((next_op & 0xff) == ASM_GLOBAL && (opcode & 0xff) == ASM_POP_GLOBAL) ||
			            ((next_op & 0xff) == ASM_STATIC && (opcode & 0xff) == ASM_POP_STATIC) ||
			            /*((next_op & 0xff) == ASM_STACK && (opcode & 0xff) == ASM_POP_N) ||*/
			            ((next_op & 0xff) == ASM_EXTERN && (opcode & 0xff) == ASM_POP_EXTERN))) {
				uint16_t next_op_imma, next_op_immb = 0;
				instruction_t *next_after_prefix;
				/* The next instruction is a prefix that uses a symbol of the same class.
				 * -> Check if the used symbols are the same, and optimize the next instruction
				 *    in case it also supports an instruction variant that operates with the stack:
				 * >>     pop local @x
				 * >>     jt  local @x, 1f
				 * Into:
				 * >>     dup
				 * >>     pop local @x
				 * >>     jt  pop, 1f
				 * This might seem slower because it's +1 more instruction, however this will
				 * actually run faster, because `jt pop' can operate faster than `jt PREFIX' */
				if (next_op == ASM16_GLOBAL) {
					next_op_imma      = UNALIGNED_GETLE16((uint16_t *)(next + 2));
					next_after_prefix = next + 4;
				} else if (next_op == ASM16_EXTERN) {
					next_op_imma      = UNALIGNED_GETLE16((uint16_t *)(next + 2));
					next_op_immb      = UNALIGNED_GETLE16((uint16_t *)(next + 4));
					next_after_prefix = next + 6;
				} else if (next_op == ASM_EXTERN) {
					next_op_imma      = *(uint8_t *)(next + 1);
					next_op_immb      = *(uint8_t *)(next + 2);
					next_after_prefix = next + 3;
				} else {
					next_op_imma      = *(uint8_t *)(next + 1);
					next_after_prefix = next + 2;
				}
				if (pop_imma == next_op_imma &&
				    pop_immb == next_op_immb) {
					uint16_t next_after_prefix_opcode;
					uint16_t pop_size;    /* Size of `pop ...' instruction found at `iter' */
					uint16_t prefix_size; /* Size of `...:' prefix found at `next' */
					pop_size                 = (uint16_t)(next - iter);
					prefix_size              = (uint16_t)(next_after_prefix - next);
					next_after_prefix_opcode = next_after_prefix[0];
do_switch_next_after_prefix_opcode:
					switch (next_after_prefix_opcode) {

					CASE_ASM_EXTENDED:
						next_after_prefix_opcode <<= 8;
						next_after_prefix_opcode |= next_after_prefix[1];
						goto do_switch_next_after_prefix_opcode;

					case ASM_YIELD:        /* `yield PREFIX' can be translated to `yield pop' */
					case ASM_YIELDALL:     /* `yield foreach, PREFIX' can be translated to `yield foreach, pop' */
					case ASM_THROW:        /* `throw PREFIX' can be translated to `throw pop' */
					case ASM_JT:           /* `jt PREFIX, 1f' can be translated to `jt pop, 1f' */
					case ASM_JT16:         /* ... */
					case ASM_JF:           /* `jf PREFIX, 1f' can be translated to `jf pop, 1f' */
					case ASM_JF16:         /* ... */
					case ASM_POP_STATIC:   /* `mov static, PREFIX' can be translated to `pop static' */
					case ASM16_POP_STATIC: /* ... */
					case ASM_POP_EXTERN:   /* `mov extern, PREFIX' can be translated to `pop extern' */
					case ASM16_POP_EXTERN: /* ... */
					case ASM_POP_GLOBAL:   /* `mov global, PREFIX' can be translated to `pop global' */
					case ASM16_POP_GLOBAL: /* ... */
					case ASM_POP_LOCAL:    /* `mov local, PREFIX' can be translated to `pop local' */
					case ASM16_POP_LOCAL:  /* ... */
						memmoveupc(iter + prefix_size, iter, pop_size, sizeof(instruction_t));
						memset(iter, ASM_DELOP, prefix_size - 1);
						iter[prefix_size - 1] = ASM_DUP;
						SET_RESULTF(iter, "Optimize `pop FOO; j%c FOO, ...' into `dup; pop FOO; j%c pop, ...'",
						            (next_after_prefix_opcode & ~1) == ASM_JT ? 't' : 'f');
						/* The optimized variant uses +1 more stack slots. */
						if (stacksz == current_assembler.a_stackmax)
							++current_assembler.a_stackmax;
						goto continue_at_iter;

					default: break;
					}
				}
			}
do_writeonly_symbol_optimization:
			/* TODO: These optimization should also apply to any instruction
			 *       that only uses a prefix as a write-only target, such as
			 *       mov instructions:
			 *    >> ...
			 *    >> mov local @x, @"foobar"  // When `x' is never used, this should get deleted.
			 *    >> ...
			 */
#if 0 /* TODO: This can only be done when the length of the the source \
       *       operand's lifetime doesn't have any side-effects.       \
       *       When done unconditionally, this optimization breaks the \
       *       assumption that an object may not be destroyed before   \
       *       its associated scope has ended.                         \
       */
			if ((opcode & 0xff) == ASM_POP_LOCAL) {
				instruction_t *scanner = next;
				bool after_jmp         = false;
				/* Optimization:
				 *   - pop local ... (never read after this instruction was executed)
				 *     >> pop */
				for (; scanner < end; scanner = DeeAsm_NextInstr(scanner)) {
					unsigned int how;
					/* NOTE: We can't follow jumps here  */
					if (IP_ISJMP(scanner)) {
						instruction_t *target;
						/* Following unconditional jumps. */
						target = follow_jmp(scanner, NULL);
						if (target <= scanner) {
scan_entire_text_for_local_readers:
							/* Search the entire assembly for assignments to this variable. */
							scanner = sc_main.sec_begin;
							for (; scanner < end; scanner = DeeAsm_NextInstr(scanner)) {
								if (asm_uses_local(scanner, pop_imma) & ASM_USING_READ)
									goto done_pop_optimization;
							}
							break; /* The variable is never read from. */
						}
						/* An unconditional jump was encountered. */
						after_jmp = true;
#if 0
						/* Can't follow absolute jumps due to code like this:
						 * >>     jf pop, 1f
						 * >>     jmp     2f // Due to the previous conditional jump, this one become conditional, too...
						 * >> 1:
						 */
						scanner = target;
						continue;
#endif
					} else {
						switch (scanner[0]) {

						case ASM_JF:
						case ASM_JF16:
						case ASM_JT:
						case ASM_JT16:
						case ASM_FOREACH:
						case ASM_FOREACH16:
							/* A conditional jump instruction was encountered. */
							after_jmp = true;
							break;

						case ASM_JMP_POP:
							/* Unpredictable jump target. */
							goto scan_entire_text_for_local_readers;
						case ASM_EXTENDED1:
							/* Unpredictable jump target. */
							if (scanner[1] == (ASM_JMP_POP_POP & 0xff))
								goto scan_entire_text_for_local_readers;
							if (scanner[1] == (ASM16_LOCAL & 0xff) ||
								scanner[1] == (ASM16_STATIC & 0xff) ||
								scanner[1] == (ASM16_GLOBAL & 0xff) ||
								scanner[1] == (ASM16_STACK & 0xff)) {
								after_prefix = scanner + 4;
							} else if (scanner[1] == (ASM16_EXTERN & 0xff)) {
								after_prefix = scanner + 6;
							} else {
								break;
							}
check_scanner_after_prefix:
							/* A conditional jump instruction was encountered. */
							if (IP_ISCJMP(after_prefix))
								after_jmp = true;
							break;

						case ASM_LOCAL:
						case ASM_GLOBAL:
						case ASM_STATIC:
						case ASM_STACK:
							after_prefix = scanner + 2;
							goto check_scanner_after_prefix;

						case ASM_EXTERN:
							after_prefix = scanner + 3;
							goto check_scanner_after_prefix;

						default:
							break;
						}
					}
					/* Check if the variable is being read from. */
					how = asm_uses_local(scanner, pop_imma);
					if (how & ASM_USING_READ)
						goto done_pop_optimization;
#if 1
					/* If the variable is re-written before being read,
					 * then what we wrote really doesn't matter.
					 * NOTE: If a conditional jump was encountered, do not
					 *       optimize for a double-write to filter code
					 *       like this:
					 * >> local foo = 42;
					 * >> if (use_the_other_foo())
					 * >>     foo = 7;
					 * ASM:
					 * >>     push   $42
					 * >>     pop    local @foo
					 * >>     push   call @global use_the_other_foo, #0
					 * >>     jz     1f
					 * >>     push   $7
					 * >>     pop    local @foo  // This is where we are right now
					 * >> 1:
					 * NOTE: If an unconditional jump was encountered, do
					 *       not optimize for a double-write to filter code
					 *       like this:
					 * >> local foo;
					 * >> if (use_basic_foo()) {
					 * >>     foo = "basic";
					 * >> } else {
					 * >>     foo = "extended";
					 * >> }
					 * ASM:
					 * >>     push   call @global use_the_other_foo, #0
					 * >>     jz     1f
					 * >>     push   @"basic"
					 * >>     pop    local @foo
					 * >>     jmp    2f
					 * >> 1:  push   @"extended"
					 * >>     pop    local @foo  // This is where we are right now
					 * >> 2:
					 */
					if (how & ASM_USING_WRITE) {
						if (after_jmp)
							goto scan_entire_text_for_local_readers;
						break;
					}
#endif
				}
delete_symbol_store:
				/* Yes, we can optimize away this use case. */
				delete_assembly((code_addr_t)(iter - sc_main.sec_begin),
					            (code_size_t)(next - iter));
				ASSERT(next_stacksz == stacksz - 1);
				iter[0] = ASM_POP;
				SET_RESULTF(iter, "Remove unused store to %s %" PRFX16 "",
					        (opcode & 0xff) == ASM_POP_LOCAL ? STR_local : STR_static,
					        pop_imma);
				validate_stack_depth((code_addr_t)(iter - sc_main.sec_begin), stacksz);
				validate_stack_depth((code_addr_t)((iter + 1) - sc_main.sec_begin), stacksz - 1);
				validate_stack_depth((code_addr_t)(next - sc_main.sec_begin), next_stacksz);
				iter    = next;
				stacksz = next_stacksz;
				goto continue_at_iter;
			} else if ((opcode & 0xff) == ASM_POP_STATIC) {
				/* Optimization:
				 *   - pop static ... (never read at all)
				 *     >> pop */
				instruction_t *scanner = sc_main.sec_begin;
				for (; scanner < end; scanner = DeeAsm_NextInstr(scanner)) {
					if (asm_uses_static(scanner, pop_imma) & ASM_USING_READ)
						goto done_pop_optimization;
				}
				/* Yes, we can optimize away this use case. */
				goto delete_symbol_store;
			}
done_pop_optimization:
#endif
			iter    = next;
			stacksz = next_stacksz;
			validate_stack_depth((code_addr_t)(iter - sc_main.sec_begin), stacksz);
			goto continue_at_iter;
		}
		break;

		/* TODO: optimize code like this:
		 *    >> push local @foo
		 *    >> fprint top, @"Hello", nl
		 *    >> pop
		 *    >> push local @foo
		 *    >> fprint top, @"World", nl
		 *    >> pop
		 * Optimize to:
		 *    >> push local @foo
		 *    >> fprint top, @"Hello", nl
		 *    >> fprint top, @"World", nl
		 *    >> pop
		 */

		default: break;
		}
/*next_opt:;*/
		validate_stack_depth((code_addr_t)(iter - sc_main.sec_begin), stacksz);
}
#undef IS_PROTECTED
#undef IS_PROTECTED_ADDR

done:
	if (should_reload_stack) {
		/* TODO: Reload the max required stack depth. */
	}
	if (should_delete_unused_symbols) {
		/* Go through all assembly symbols and delete symbols that are no longer being used. */
		if (asm_delunusedsyms())
			result = true;
	}
	Dee_Free(protected_code);
done_raw:
	return result;
err:
	Dee_Free(protected_code);
	return -1;
}

DECL_END


#endif /* !GUARD_DEEMON_COMPILER_ASM_PEEPHOLE_C */
