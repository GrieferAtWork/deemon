/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_COMPILER_ASM_USERASM_C
#define GUARD_DEEMON_COMPILER_ASM_USERASM_C 1

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/tpp.h>
#include <deemon/error.h>
#include <deemon/system-features.h> /* memmoveupc(), ... */

#ifndef CONFIG_LANGUAGE_NO_ASM
#include <deemon/bool.h>
#include <deemon/dict.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/tuple.h>

#include <hybrid/byteswap.h>
#include <hybrid/unaligned.h>

#include <stdint.h> /* UINT8_MAX, ... */

#include "../../runtime/strings.h"
#endif /* !CONFIG_LANGUAGE_NO_ASM */

DECL_BEGIN

#ifndef CONFIG_LANGUAGE_NO_ASM
INTERN WUNUSED NONNULL((1, 2)) dssize_t DCALL
asm_invoke_operand_print(struct asm_invoke_operand *__restrict self,
                         struct ascii_printer *__restrict printer) {
	dssize_t temp, result = 0;
	char const *raw_operand_string = NULL;
	if (self->io_class & OPERAND_CLASS_FBRACKETFLAG) {
		temp = ascii_printer_putc(printer, '[');
		if unlikely(temp)
			goto err;
		++result;
	}
	if (self->io_class & OPERAND_CLASS_FBRACEFLAG) {
		temp = ascii_printer_putc(printer, '{');
		if unlikely(temp)
			goto err;
		++result;
	}
	if (self->io_class & OPERAND_CLASS_FIMMVAL) {
		temp = ascii_printer_putc(printer, '$');
		if unlikely(temp)
			goto err;
		++result;
	}
	if (self->io_class & OPERAND_CLASS_FSTACKFLAG) {
		temp = ascii_printer_putc(printer, '#');
		if unlikely(temp)
			goto err;
		++result;
	}
	if (self->io_class & OPERAND_CLASS_FSTACKFLAG2) {
		temp = ASCII_PRINTER_PRINT(printer, " #");
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	switch (self->io_class & OPERAND_CLASS_FMASK) {

	case OPERAND_CLASS_POP:
		raw_operand_string = DeeString_STR(&str_pop);
		goto do_raw_string;

	case OPERAND_CLASS_TOP:
		raw_operand_string = "top";
		goto do_raw_string;

	case OPERAND_CLASS_POP_OR_TOP:
		raw_operand_string = "~pop_or_top~";
		goto do_raw_string;

	case OPERAND_CLASS_REF:
		temp = ascii_printer_printf(printer, "ref %u",
		                            (unsigned int)self->io_symid);
		break;

	case OPERAND_CLASS_ARG:
		temp = ascii_printer_printf(printer, "arg %u",
		                            (unsigned int)self->io_symid);
		break;

	case OPERAND_CLASS_CONST:
		temp = ascii_printer_printf(printer, "const %u",
		                            (unsigned int)self->io_symid);
		break;

	case OPERAND_CLASS_STATIC:
		temp = ascii_printer_printf(printer, "static %u",
		                            (unsigned int)self->io_symid);
		break;

	case OPERAND_CLASS_MODULE:
		temp = ascii_printer_printf(printer, "module %u",
		                            (unsigned int)self->io_symid);
		break;

	case OPERAND_CLASS_EXTERN:
		temp = ascii_printer_printf(printer, "extern %u:%u",
		                            (unsigned int)self->io_extern.io_modid,
		                            (unsigned int)self->io_extern.io_symid);
		break;

	case OPERAND_CLASS_GLOBAL:
		temp = ascii_printer_printf(printer, "global %u",
		                            (unsigned int)self->io_symid);
		break;

	case OPERAND_CLASS_LOCAL:
		temp = ascii_printer_printf(printer, "local %u",
		                            (unsigned int)self->io_symid);
		break;

	case OPERAND_CLASS_SDISP8:
	case OPERAND_CLASS_SDISP16:
	case OPERAND_CLASS_SDISP32:
	case OPERAND_CLASS_DISP8:
	case OPERAND_CLASS_DISP16:
	case OPERAND_CLASS_DISP32:
	case OPERAND_CLASS_DISP_EQ_N2:
	case OPERAND_CLASS_DISP_EQ_N1:
	case OPERAND_CLASS_DISP_EQ_0:
	case OPERAND_CLASS_DISP_EQ_1:
	case OPERAND_CLASS_DISP_EQ_2:
	case OPERAND_CLASS_DISP8_HALF:
	case OPERAND_CLASS_DISP16_HALF:
		if (self->io_intexpr.ie_sym) {
			struct TPPKeyword *name;
			char const *mode   = ".PC";
			char const *suffix = " + ";
			name = self->io_intexpr.ie_sym->as_uname;
			if (self->io_intexpr.ie_rel == ASM_OVERLOAD_FSTKABS ||
			    self->io_intexpr.ie_rel == ASM_OVERLOAD_FSTKDSP)
				mode = ".SP";
			if (self->io_intexpr.ie_rel == (uint16_t)-1)
				mode = "";
			if (self->io_intexpr.ie_val == 0)
				suffix = "";
			if (name) {
				temp = ascii_printer_printf(printer, "%s%s%s",
				                            name->k_name, mode, suffix);
			} else {
				temp = ascii_printer_printf(printer, ".L<%p>%s%s",
				                            self->io_intexpr.ie_sym,
				                            mode, suffix);
			}
			if (!self->io_intexpr.ie_val)
				break;
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		if ((self->io_class & OPERAND_CLASS_FMASK) == OPERAND_CLASS_DISP8 ||
		    (self->io_class & OPERAND_CLASS_FMASK) == OPERAND_CLASS_DISP16 ||
		    (self->io_class & OPERAND_CLASS_FMASK) == OPERAND_CLASS_DISP32) {
			temp = ascii_printer_printf(printer, "%I64u", self->io_intexpr.ie_val);
		} else {
			temp = ascii_printer_printf(printer, "%I64d", self->io_intexpr.ie_val);
		}
		break;

	case OPERAND_CLASS_NONE:
		raw_operand_string = DeeString_STR(&str_none);
		goto do_raw_string;

	case OPERAND_CLASS_FOREACH:
		raw_operand_string = "foreach";
		goto do_raw_string;

	case OPERAND_CLASS_EXCEPT:
		raw_operand_string = DeeString_STR(&str_except);
		goto do_raw_string;

	case OPERAND_CLASS_CATCH:
		raw_operand_string = "catch";
		goto do_raw_string;

	case OPERAND_CLASS_FINALLY:
		raw_operand_string = "finally";
		goto do_raw_string;

	case OPERAND_CLASS_THIS:
		raw_operand_string = DeeString_STR(&str_this);
		goto do_raw_string;

	case OPERAND_CLASS_THIS_MODULE:
		raw_operand_string = DeeString_STR(&str_this_module);
		goto do_raw_string;

	case OPERAND_CLASS_THIS_FUNCTION:
		raw_operand_string = DeeString_STR(&str_this_function);
		goto do_raw_string;

	case OPERAND_CLASS_TRUE:
		raw_operand_string = DeeString_STR(&str_true);
		goto do_raw_string;

	case OPERAND_CLASS_FALSE:
		raw_operand_string = DeeString_STR(&str_false);
		goto do_raw_string;

	case OPERAND_CLASS_LIST:
		raw_operand_string = DeeString_STR(&str_List);
		goto do_raw_string;

	case OPERAND_CLASS_TUPLE:
		raw_operand_string = DeeString_STR(&str_Tuple);
		goto do_raw_string;

	case OPERAND_CLASS_HASHSET:
		raw_operand_string = DeeString_STR(&str_HashSet);
		goto do_raw_string;

	case OPERAND_CLASS_DICT:
		raw_operand_string = DeeString_STR(&str_Dict);
		goto do_raw_string;

	case OPERAND_CLASS_INT:
		raw_operand_string = DeeString_STR(&str_int);
		goto do_raw_string;

	case OPERAND_CLASS_BOOL:
		raw_operand_string = DeeString_STR(&str_bool);
		goto do_raw_string;

	case OPERAND_CLASS_EQ:
		raw_operand_string = "eq";
		goto do_raw_string;

	case OPERAND_CLASS_NE:
		raw_operand_string = "ne";
		goto do_raw_string;

	case OPERAND_CLASS_LO:
		raw_operand_string = "lo";
		goto do_raw_string;

	case OPERAND_CLASS_LE:
		raw_operand_string = "le";
		goto do_raw_string;

	case OPERAND_CLASS_GR:
		raw_operand_string = "gr";
		goto do_raw_string;

	case OPERAND_CLASS_GE:
		raw_operand_string = "ge";
		goto do_raw_string;

	case OPERAND_CLASS_SO:
		raw_operand_string = "so";
		goto do_raw_string;

	case OPERAND_CLASS_DO:
		raw_operand_string = "do";
		goto do_raw_string;

	case OPERAND_CLASS_BREAK:
		raw_operand_string = "break";
		goto do_raw_string;

	case OPERAND_CLASS_MIN:
		raw_operand_string = "min";
		goto do_raw_string;

	case OPERAND_CLASS_MAX:
		raw_operand_string = "max";
		goto do_raw_string;

	case OPERAND_CLASS_SUM:
		raw_operand_string = "sum";
		goto do_raw_string;

	case OPERAND_CLASS_ANY:
		raw_operand_string = "any";
		goto do_raw_string;

	case OPERAND_CLASS_ALL:
		raw_operand_string = "all";
		goto do_raw_string;

	case OPERAND_CLASS_SP:
		raw_operand_string = "sp";
		goto do_raw_string;

	case OPERAND_CLASS_NL:
		raw_operand_string = "nl";
		goto do_raw_string;

	case OPERAND_CLASS_MOVE:
		raw_operand_string = "move";
		goto do_raw_string;

	case OPERAND_CLASS_DEFAULT:
		raw_operand_string = "default";
		goto do_raw_string;

	case OPERAND_CLASS_VARARGS:
		raw_operand_string = "varargs";
		goto do_raw_string;

	case OPERAND_CLASS_VARKWDS:
		raw_operand_string = "varkwds";
do_raw_string:
		temp = ascii_printer_print(printer,
		                           raw_operand_string,
		                           strlen(raw_operand_string));
		break;

	default:
		temp = ascii_printer_printf(printer, "??" "?(%u)",
		                            (unsigned int)self->io_class);
		break;
	}
	if unlikely(temp < 0)
		goto err;
	result += temp;
	if (self->io_class & OPERAND_CLASS_FDOTSFLAG) {
		temp = ASCII_PRINTER_PRINT(printer, "...");
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	if (self->io_class & OPERAND_CLASS_FBRACEFLAG) {
		temp = ascii_printer_putc(printer, '}');
		if unlikely(temp)
			goto err;
		++result;
	}
	if (self->io_class & OPERAND_CLASS_FBRACKETFLAG) {
		temp = ascii_printer_putc(printer, ']');
		if unlikely(temp)
			goto err;
		++result;
	}
	return result;
err:
	return temp;
}

INTERN WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
asm_invocation_print(struct asm_invocation *__restrict self,
                     struct asm_mnemonic *__restrict instr,
                     struct ascii_printer *__restrict printer) {
	dssize_t temp, result = 0;
	unsigned int i;
	if (self->ai_flags & INVOKE_FPUSH) {
		temp = ASCII_PRINTER_PRINT(printer, "push ");
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	if (self->ai_flags & INVOKE_FPREFIX) {
		switch (self->ai_prefix) {

		case ASM_STACK:
			temp = ascii_printer_printf(printer, "stack #%u: ",
			                            (unsigned int)self->ai_prefix_id1);
			break;

		case ASM_STATIC:
			temp = ascii_printer_printf(printer, "static %u: ",
			                            (unsigned int)self->ai_prefix_id1);
			break;

		case ASM_EXTERN:
			temp = ascii_printer_printf(printer, "extern %u:%u: ",
			                            (unsigned int)self->ai_prefix_id1,
			                            (unsigned int)self->ai_prefix_id2);
			break;

		case ASM_GLOBAL:
			temp = ascii_printer_printf(printer, "global %u: ",
			                            (unsigned int)self->ai_prefix_id1);
			break;

		case ASM_LOCAL:
			temp = ascii_printer_printf(printer, "local %u: ",
			                            (unsigned int)self->ai_prefix_id1);
			break;

		default:
			temp = ascii_printer_printf(printer, "??" "?(%u) %u:%u: ",
			                            (unsigned int)self->ai_prefix,
			                            (unsigned int)self->ai_prefix_id1,
			                            (unsigned int)self->ai_prefix_id2);
			break;
		}
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	temp = ascii_printer_print(printer,
	                           instr->am_name,
	                           strlen(instr->am_name));
	if unlikely(temp < 0)
		goto err;
	result += temp;
	for (i = 0; i < self->ai_opcount; ++i) {
		if (i == 0) {
			temp = ascii_printer_putc(printer, ' ');
			if unlikely(temp)
				goto err;
			++result;
		} else {
			temp = ASCII_PRINTER_PRINT(printer, ", ");
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		temp = asm_invoke_operand_print(&self->ai_ops[i], printer);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
asm_invocation_tostring(struct asm_invocation *__restrict self,
                        struct asm_mnemonic *__restrict instr) {
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	if unlikely(asm_invocation_print(self, instr, &printer) < 0)
		goto err;
	return ascii_printer_pack(&printer);
err:
	ascii_printer_fini(&printer);
	return NULL;
}




/* @param: ao_flags: Set of `ASM_OVERLOAD_F*' */
PRIVATE bool FCALL
compatible_operand(struct asm_invoke_operand   const *__restrict iop,
                   struct asm_overload_operand const *__restrict oop,
                   uint16_t ao_flags) {
	tint_t imm_val;
	uint16_t imm_rel;
#define OPERAND_CLASS_FFLAGMASK \
	(~OPERAND_CLASS_FMASK & ~(OPERAND_CLASS_FSUBSP | OPERAND_CLASS_FSPSUB | OPERAND_CLASS_FSPADD))

	/* Match context flags (aka. flags set by a prefix such as `$' or `{...}') */
	if ((iop->io_class & OPERAND_CLASS_FFLAGMASK) !=
	    (UNALIGNED_GET16(&oop->aoo_class) & OPERAND_CLASS_FFLAGMASK)) {
		if (((UNALIGNED_GET16(&oop->aoo_class) & OPERAND_CLASS_FMASK) == OPERAND_CLASS_TOP ||
		     (UNALIGNED_GET16(&oop->aoo_class) & OPERAND_CLASS_FMASK) == OPERAND_CLASS_POP) &&
		    (iop->io_class & OPERAND_CLASS_FMASK) == OPERAND_CLASS_POP_OR_TOP)
			;
		else if ((UNALIGNED_GET16(&oop->aoo_class) & OPERAND_CLASS_FMASK) == OPERAND_CLASS_PREFIX)
			;
#if 0
		else if (OPERAND_CLASS_ISDISP(UNALIGNED_GET16(&oop->aoo_class)) &&
		         OPERAND_CLASS_ISDISP(iop->io_class))
			;
#endif
		else {
			goto nope;
		}
	}
	imm_val = iop->io_intexpr.ie_val;
	imm_rel = iop->io_intexpr.ie_rel;
	if (imm_rel == (uint16_t)-1) {
		imm_rel = ao_flags & ASM_OVERLOAD_FRELMSK;
	} else if (ao_flags & ASM_OVERLOAD_FREL_DSPBIT) {
		/* Toggle the relative displacement bit. */
		imm_rel ^= ASM_OVERLOAD_FREL_DSPBIT;
	}
	/* The stack-prefix instruction uses absolute addressing. */
	switch (UNALIGNED_GET16(&oop->aoo_class) & (OPERAND_CLASS_FSPADD |
	                                            OPERAND_CLASS_FSPSUB)) {

	case OPERAND_CLASS_FSPADD: /* `SP + imm' */
		imm_val -= current_assembler.a_stackcur;
		break;

	case OPERAND_CLASS_FSPSUB: /* `SP - imm' */
		imm_val = current_assembler.a_stackcur - imm_val;
		break;

	case OPERAND_CLASS_FSUBSP: /* `imm - SP' */
		imm_val += current_assembler.a_stackcur;
		break;

	default: break;
	}

	imm_val += oop->aoo_disp;
#if 0
	/* Relative stack displacements work in reverse. */
	if (imm_rel == ASM_OVERLOAD_FSTKDSP)
		imm_val = -imm_val;
#endif
	switch (UNALIGNED_GET16(&oop->aoo_class) & OPERAND_CLASS_FMASK) {

	case OPERAND_CLASS_VARARGS:
		if ((iop->io_class & OPERAND_CLASS_FMASK) == OPERAND_CLASS_VARARGS)
			break;
		/* Special case: If Allow the use of the last argument as replacement for `varargs' */
		if (((iop->io_class & OPERAND_CLASS_FMASK) == OPERAND_CLASS_ARG) &&
		    current_basescope->bs_varargs &&
		    iop->io_symid == current_basescope->bs_varargs->s_symid)
			break;
		goto nope;

	case OPERAND_CLASS_VARKWDS:
		if ((iop->io_class & OPERAND_CLASS_FMASK) == OPERAND_CLASS_VARKWDS)
			break;
		/* Special case: If Allow the use of the last argument as replacement for `varargs' */
		if (((iop->io_class & OPERAND_CLASS_FMASK) == OPERAND_CLASS_ARG) &&
		    current_basescope->bs_varkwds &&
		    iop->io_symid == current_basescope->bs_varkwds->s_symid)
			break;
		goto nope;

	case OPERAND_CLASS_ARG:
		if ((iop->io_class & OPERAND_CLASS_FMASK) != OPERAND_CLASS_ARG)
			goto nope;
		if (iop->io_symid > UINT8_MAX && !(ao_flags & (ASM_OVERLOAD_F16BIT | ASM_OVERLOAD_FF0)))
			goto nope;
		if (current_basescope->bs_varargs &&
		    current_basescope->bs_varargs->s_symid == iop->io_symid)
			goto nope;
		if (current_basescope->bs_varkwds &&
		    current_basescope->bs_varkwds->s_symid == iop->io_symid)
			goto nope;
		break;

	case OPERAND_CLASS_SDISP8:
		if (!OPERAND_CLASS_ISDISP(iop->io_class))
			goto nope;
		if (iop->io_intexpr.ie_sym)
			goto nope;
		if (imm_val < INT8_MIN ||
		    imm_val > INT8_MAX)
			goto nope;
		break;

	case OPERAND_CLASS_SDISP16:
		if (!OPERAND_CLASS_ISDISP(iop->io_class))
			goto nope;
		if (iop->io_intexpr.ie_sym &&
		    (imm_rel == ASM_OVERLOAD_FRELABS ||
		     imm_rel == ASM_OVERLOAD_FRELDSP) &&
		    (current_assembler.a_flag & ASM_FBIGCODE))
			goto nope;
		if (imm_val < INT16_MIN ||
		    imm_val > INT16_MAX)
			goto nope;
		break;

	case OPERAND_CLASS_SDISP32:
		if (!OPERAND_CLASS_ISDISP(iop->io_class))
			goto nope;
		if (imm_val < INT32_MIN ||
		    imm_val > INT32_MAX)
			goto nope;
		break;

	case OPERAND_CLASS_DISP8:
		if (!OPERAND_CLASS_ISDISP(iop->io_class))
			goto nope;
		if (iop->io_intexpr.ie_sym)
			goto nope;
		if (imm_val < 0 ||
		    imm_val > UINT8_MAX)
			goto nope;
		break;

	case OPERAND_CLASS_DISP16:
		if (!OPERAND_CLASS_ISDISP(iop->io_class))
			goto nope;
		if (iop->io_intexpr.ie_sym &&
		    (imm_rel == ASM_OVERLOAD_FRELABS ||
		     imm_rel == ASM_OVERLOAD_FRELDSP) &&
		    (current_assembler.a_flag & ASM_FBIGCODE))
			goto nope;
		if (imm_val < 0 ||
		    imm_val > UINT16_MAX)
			goto nope;
		break;

	case OPERAND_CLASS_DISP32:
		if (!OPERAND_CLASS_ISDISP(iop->io_class))
			goto nope;
		if (imm_val < 0 ||
		    imm_val > UINT32_MAX)
			goto nope;
		break;

	case OPERAND_CLASS_DISP8_HALF:
		if (!OPERAND_CLASS_ISDISP(iop->io_class))
			goto nope;
		if (iop->io_intexpr.ie_sym)
			goto nope;
		if (imm_val < 0 || (imm_val & 1) || ((imm_val >> 1) > UINT8_MAX))
			goto nope;
		break;

	case OPERAND_CLASS_DISP16_HALF:
		if (!OPERAND_CLASS_ISDISP(iop->io_class))
			goto nope;
		if (iop->io_intexpr.ie_sym)
			goto nope;
		if (imm_val < 0 || (imm_val & 1) || ((imm_val >> 1) > UINT16_MAX))
			goto nope;
		break;

	case OPERAND_CLASS_DISP_EQ_2:
	case OPERAND_CLASS_DISP_EQ_1:
	case OPERAND_CLASS_DISP_EQ_0:
	case OPERAND_CLASS_DISP_EQ_N1:
	case OPERAND_CLASS_DISP_EQ_N2:
		if (!OPERAND_CLASS_ISDISP(iop->io_class))
			goto nope;
		if (iop->io_intexpr.ie_sym)
			goto nope;
		if (OPERAND_CLASS_DISP_EQ_VALUE(UNALIGNED_GET16(&oop->aoo_class)) != imm_val)
			goto nope;
		break;

	case OPERAND_CLASS_PREFIX:
		switch (iop->io_class & OPERAND_CLASS_FMASK) {

		case OPERAND_CLASS_POP:
		case OPERAND_CLASS_TOP:
		case OPERAND_CLASS_POP_OR_TOP:
			/* Encoded as stack-top operand. */
			break;

		case OPERAND_CLASS_CONST:
			if (!(ao_flags & ASM_OVERLOAD_FPREFIX_RO))
				goto nope; /* The overload doesn't accept a read-only operand. */
			ATTR_FALLTHROUGH
		case OPERAND_CLASS_STATIC:
		case OPERAND_CLASS_EXTERN:
		case OPERAND_CLASS_GLOBAL:
		case OPERAND_CLASS_LOCAL:
			break;

		case OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_SDISP8:
		case OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_SDISP16:
		case OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_SDISP32:
		case OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_DISP8:
		case OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_DISP16:
		case OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_DISP32:
		case OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_DISP_EQ_N2:
		case OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_DISP_EQ_N1:
		case OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_DISP_EQ_0:
		case OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_DISP_EQ_1:
		case OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_DISP_EQ_2:
		case OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_DISP8_HALF:
		case OPERAND_CLASS_FSTACKFLAG | OPERAND_CLASS_DISP16_HALF:
			/* Stack operand. */
			if (iop->io_intexpr.ie_val < 0)
				goto nope;
			if (iop->io_intexpr.ie_val > UINT16_MAX)
				goto nope;
			break;

		default: goto nope;
		}
		break;

	case OPERAND_CLASS_TOP:
	case OPERAND_CLASS_POP:
		if ((iop->io_class & OPERAND_CLASS_FMASK) == OPERAND_CLASS_POP_OR_TOP)
			break;
		goto do_default_class_check;

		/* For symbol-style operands, make sure that the FF0 flag is
		 * set when their index lies beyond the 8-bit address range. */
	case OPERAND_CLASS_EXTERN:
		if (iop->io_extern.io_modid > UINT8_MAX &&
		    !(ao_flags & (ASM_OVERLOAD_F16BIT | ASM_OVERLOAD_FF0)))
			goto nope;
		ATTR_FALLTHROUGH
	case OPERAND_CLASS_REF:
	case OPERAND_CLASS_CONST:
	case OPERAND_CLASS_STATIC:
	case OPERAND_CLASS_MODULE:
	case OPERAND_CLASS_GLOBAL:
	case OPERAND_CLASS_LOCAL:
		if (iop->io_symid > UINT8_MAX &&
		    !(ao_flags & (ASM_OVERLOAD_F16BIT | ASM_OVERLOAD_FF0)))
			goto nope;
		ATTR_FALLTHROUGH
	default:
		/* Fallback: confirm exact classification match. */
do_default_class_check:
		if ((iop->io_class & OPERAND_CLASS_FMASK) !=
		    (UNALIGNED_GET16(&oop->aoo_class) & OPERAND_CLASS_FMASK))
			goto nope;
		break;
	}
	return true;
nope:
	return false;
}

struct reldesc {
	uint16_t rd_rel8;  /* 8-bit relocation */
	uint16_t rd_rel16; /* 16-bit relocation */
	uint16_t rd_rel32; /* 32-bit relocation */
};

/* Relocation type descriptors.
 * The index is one of `ASM_OVERLOAD_FREL*' or `ASM_OVERLOAD_FSTK*'
 * Unsupported relocations are encoded as `R_DMN_NONE' */
PRIVATE struct reldesc const reldescs[ASM_OVERLOAD_FRELMSK + 1] = {
	/* [ASM_OVERLOAD_FRELABS] = */ { R_DMN_ABS8, R_DMN_ABS16, R_DMN_ABS32 },
	/* [ASM_OVERLOAD_FRELDSP] = */ { R_DMN_DISP8, R_DMN_DISP16, R_DMN_DISP32 },
	/* [ASM_OVERLOAD_FSTKABS] = */ { R_DMN_STCK8, R_DMN_STCK16, R_DMN_NONE },
	/* [ASM_OVERLOAD_FSTKDSP] = */ { R_DMN_STCKA8, R_DMN_STCKA16, R_DMN_NONE },
};

/* @param: flags: Set of `ASM_OVERLOAD_F*'
 * @param: bits:  One of `8', `16' or `32' */
#define asm_putrel_f(flags, sym, bits) \
	asm_putrel(reldescs[(flags) & ASM_OVERLOAD_FRELMSK].rd_rel##bits, sym, 0)


INTERN WUNUSED NONNULL((1, 2)) int FCALL
uasm_invoke(struct asm_mnemonic *__restrict instr,
            struct asm_invocation *__restrict invoc) {
	struct asm_overload *iter, *end;
	unsigned int i;
	bool stackabs_translated = false;
retry:
	if (invoc->ai_flags & INVOKE_FPUSH && invoc->ai_opcount &&
	    invoc->ai_ops[0].io_class == OPERAND_CLASS_POP) {
		/* Translate: `push add pop, pop' --> `add top, pop' */
		invoc->ai_flags &= ~INVOKE_FPUSH;
		invoc->ai_ops[0].io_class = OPERAND_CLASS_TOP;
	}
	iter = instr->am_overloads;
	end  = iter + instr->am_num_overloads;
	for (; iter != end; ++iter) {
		/* Search for a suitable overload. */
		if (iter->ao_opcount != invoc->ai_opcount)
			continue;
			/* Match behavioral flags. */
#if (INVOKE_FPUSH == ASM_OVERLOAD_FPUSH) && (INVOKE_FPREFIX == ASM_OVERLOAD_FPREFIX)
		if ((invoc->ai_flags & (INVOKE_FPUSH | INVOKE_FPREFIX)) !=
		    (iter->ao_flags & (ASM_OVERLOAD_FPUSH | ASM_OVERLOAD_FPREFIX)))
			continue;
#else /* (INVOKE_FPUSH == ASM_OVERLOAD_FPUSH) && (INVOKE_FPREFIX == ASM_OVERLOAD_FPREFIX) */
		if (!!(invoc->ai_flags & INVOKE_FPUSH) !=
		    !!(iter->ao_flags & ASM_OVERLOAD_FPUSH))
			continue;
		if (!!(invoc->ai_flags & INVOKE_FPREFIX) !=
		    !!(iter->ao_flags & ASM_OVERLOAD_FPREFIX))
			continue;
#endif /* (INVOKE_FPUSH != ASM_OVERLOAD_FPUSH) || (INVOKE_FPREFIX != ASM_OVERLOAD_FPREFIX) */
		/* Make sure that read-only prefix operands can only
		 * be used with read-only prefix instructions. */
		if (!(iter->ao_flags & ASM_OVERLOAD_FPREFIX_RO) &&
		    (invoc->ai_flags & INVOKE_FPREFIX_RO))
			continue;
		/* If the overload is only applicable to yielding/non-yielding
		 * functions, validate that we match that requirement. */
		if ((iter->ao_flags & (ASM_OVERLOAD_FRET | ASM_OVERLOAD_FYLD)) &&
		    !!(iter->ao_flags & ASM_OVERLOAD_FYLD) !=
		    !!(current_basescope->bs_flags & CODE_FYIELDING))
			continue;
		/* Match operands against each other. */
		for (i = 0; i < iter->ao_opcount; ++i) {
			if (!compatible_operand(&invoc->ai_ops[i], &iter->ao_ops[i],
			                        iter->ao_flags))
				goto next_overload;
		}
		goto got_overload;
	next_overload:;
	}
/*check_stackabs_translated:*/
	if (!stackabs_translated) {
		uint16_t immediate_stackdepth;
		stackabs_translated = true;
		/* Translate absolute stack addresses in operands:
		 * >> add top, #1
		 * Translate to this when the previous stack-alignment was `#2'
		 * >> add top, pop */
		i                    = invoc->ai_opcount;
		immediate_stackdepth = current_assembler.a_stackcur;
		while (i--) {
			if (!(invoc->ai_ops[i].io_class & OPERAND_CLASS_FSTACKFLAG))
				continue;
			if (!OPERAND_CLASS_ISDISP(invoc->ai_ops[i].io_class))
				goto err_no_overload;
			if (invoc->ai_ops[i].io_intexpr.ie_sym)
				goto err_no_overload;
			--immediate_stackdepth;
			if (invoc->ai_ops[i].io_intexpr.ie_val == immediate_stackdepth) {
				invoc->ai_ops[i].io_class &= ~(OPERAND_CLASS_FMASK | OPERAND_CLASS_FSTACKFLAG);
				invoc->ai_ops[i].io_class |= OPERAND_CLASS_POP_OR_TOP;
				if (invoc->ai_ops[i].io_class & OPERAND_CLASS_FSTACKFLAG2) {
					invoc->ai_ops[i].io_class &= ~OPERAND_CLASS_FSTACKFLAG2;
					invoc->ai_ops[i].io_class |= OPERAND_CLASS_FSTACKFLAG;
				}
				goto retry;
			}
		}
	}
err_no_overload:
	DeeError_Throwf(&DeeError_CompilerError,
	                "No overload of `%s' matches the operands and prefix in `%K'",
	                instr->am_name, asm_invocation_tostring(invoc, instr));
err:
	return -1;
got_overload: {
	/* Got the overload that should be printed.
	 * Now to generate the assembly and update the assembler state. */
	code_addr_t instr_start = asm_ip();
	uint16_t old_sp, sp_add, sp_sub;
	instruction_t *instr_end;
	bool emit_sym16 = false; /* Emit symbols as 16-bit operands */
	bool emit_imm16 = false; /* Emit integers as 16-bit operands */
	/* Special case: discard emission of DELOP instructions. */
	if (((instruction_t)iter->ao_instr) == ASM_DELOP)
		goto done;

	if ((current_userasm.ua_flags & AST_FASSEMBLY_VOLATILE) &&
	    (current_assembler.a_flag & ASM_FPEEPHOLE)) {
		/* Create a fake symbol to prevent peephole
		 * optimization from tinkering with the generated code. */
		struct asm_sym *volatile_sym = asm_newsym();
		if unlikely(!volatile_sym)
			goto err;
		asm_defsym(volatile_sym);
		++volatile_sym->as_used; /* Intentionally left dangling. */
	}
	/* Emit a prefix. */
	if (invoc->ai_flags & INVOKE_FPREFIX) {
		bool is_extended = false;
		if (invoc->ai_prefix != ASM_EXTERN)
			invoc->ai_prefix_id2 = 0;
		/* Check if the prefix needs to be extended. */
		if (invoc->ai_prefix_id1 > UINT8_MAX ||
		    invoc->ai_prefix_id2 > UINT8_MAX) {
			is_extended = true;
			if (asm_put(ASM_EXTENDED1))
				goto err;
		}
		if (asm_put(invoc->ai_prefix))
			goto err;
		if (is_extended) {
			if (asm_put_data16(invoc->ai_prefix_id1))
				goto err;
			if (invoc->ai_prefix == ASM_EXTERN) {
				if (asm_put_data16(invoc->ai_prefix_id2))
					goto err;
			}
		} else {
			if (asm_put_data8((uint8_t)invoc->ai_prefix_id1))
				goto err;
			if (invoc->ai_prefix == ASM_EXTERN) {
				if (asm_put_data8((uint8_t)invoc->ai_prefix_id2))
					goto err;
			}
		}
	} else {
		/* Emit prefix operands. */
		for (i = 0; i < iter->ao_opcount; ++i) {
			if ((UNALIGNED_GET16(&iter->ao_ops[i].aoo_class) & OPERAND_CLASS_FMASK) != OPERAND_CLASS_PREFIX)
				continue;
			switch (invoc->ai_ops[i].io_class & OPERAND_CLASS_FMASK) {

			case OPERAND_CLASS_POP:
			case OPERAND_CLASS_TOP:
			case OPERAND_CLASS_POP_OR_TOP:
				/* stack top: ... */
				invoc->ai_ops[i].io_intexpr.ie_val = current_assembler.a_stackcur - 1;
				ATTR_FALLTHROUGH
			case OPERAND_CLASS_SDISP8:
			case OPERAND_CLASS_SDISP16:
			case OPERAND_CLASS_SDISP32:
			case OPERAND_CLASS_DISP8:
			case OPERAND_CLASS_DISP16:
			case OPERAND_CLASS_DISP32:
			case OPERAND_CLASS_DISP_EQ_N2:
			case OPERAND_CLASS_DISP_EQ_N1:
			case OPERAND_CLASS_DISP_EQ_0:
			case OPERAND_CLASS_DISP_EQ_1:
			case OPERAND_CLASS_DISP_EQ_2:
			case OPERAND_CLASS_DISP8_HALF:
			case OPERAND_CLASS_DISP16_HALF:
				if (invoc->ai_ops[i].io_intexpr.ie_val > UINT8_MAX) {
					if (asm_put(ASM_EXTENDED1))
						goto err;
					if (asm_putimm16(ASM_STACK, (uint16_t)invoc->ai_ops[i].io_intexpr.ie_val))
						goto err;
				} else {
					if (asm_putimm8(ASM_STACK, (uint8_t)invoc->ai_ops[i].io_intexpr.ie_val))
						goto err;
				}
				break;

			case OPERAND_CLASS_CONST:
			case OPERAND_CLASS_STATIC:
				if (invoc->ai_ops[i].io_symid > UINT8_MAX) {
					if (asm_put(ASM_EXTENDED1))
						goto err;
					if (asm_putimm16(ASM_STATIC, invoc->ai_ops[i].io_symid))
						goto err;
				} else {
					if (asm_putimm8(ASM_STATIC, (uint8_t)invoc->ai_ops[i].io_symid))
						goto err;
				}
				break;

			case OPERAND_CLASS_EXTERN:
				if (invoc->ai_ops[i].io_extern.io_modid > UINT8_MAX ||
				    invoc->ai_ops[i].io_extern.io_symid > UINT8_MAX) {
					if (asm_put(ASM_EXTENDED1))
						goto err;
					if (asm_putimm16(ASM_EXTERN, invoc->ai_ops[i].io_extern.io_modid))
						goto err;
					if (asm_put_data16(invoc->ai_ops[i].io_extern.io_symid))
						goto err;
				} else {
					if (asm_putimm8(ASM_EXTERN, (uint8_t)invoc->ai_ops[i].io_extern.io_modid))
						goto err;
					if (asm_put_data8((uint8_t)invoc->ai_ops[i].io_extern.io_symid))
						goto err;
				}
				break;

			case OPERAND_CLASS_GLOBAL:
				if (invoc->ai_ops[i].io_symid > UINT8_MAX) {
					if (asm_put(ASM_EXTENDED1))
						goto err;
					if (asm_putimm16(ASM_GLOBAL, invoc->ai_ops[i].io_symid))
						goto err;
				} else {
					if (asm_putimm8(ASM_GLOBAL, (uint8_t)invoc->ai_ops[i].io_symid))
						goto err;
				}
				break;

			case OPERAND_CLASS_LOCAL:
				if (invoc->ai_ops[i].io_symid > UINT8_MAX) {
					if (asm_put(ASM_EXTENDED1))
						goto err;
					if (asm_putimm16(ASM_LOCAL, invoc->ai_ops[i].io_symid))
						goto err;
				} else {
					if (asm_putimm8(ASM_LOCAL, (uint8_t)invoc->ai_ops[i].io_symid))
						goto err;
				}
				break;

			default: __builtin_unreachable();
			}
		}
	}

	/* Check if the instruction must be prefixed by F0 */
	for (i = 0; i < iter->ao_opcount; ++i) {
		switch (UNALIGNED_GET16(&iter->ao_ops[i].aoo_class) & OPERAND_CLASS_FMASK) {

		case OPERAND_CLASS_EXTERN: /* `extern <io_modid>:<io_symid>' */
			if (invoc->ai_ops[i].io_extern.io_modid > UINT8_MAX)
				goto do_emit_f0_prefix;
			ATTR_FALLTHROUGH
		case OPERAND_CLASS_REF:
		case OPERAND_CLASS_ARG:
		case OPERAND_CLASS_CONST:
		case OPERAND_CLASS_STATIC:
		case OPERAND_CLASS_MODULE:
		case OPERAND_CLASS_GLOBAL:
		case OPERAND_CLASS_LOCAL:
			if (invoc->ai_ops[i].io_symid <= UINT8_MAX)
				break;
do_emit_f0_prefix:
			if (!(iter->ao_flags & ASM_OVERLOAD_F16BIT)) {
				if (asm_put(ASM_EXTENDED1))
					goto err;
			}
			emit_sym16 = true;
			emit_imm16 = !!(iter->ao_flags & ASM_OVERLOAD_FF0_IMM);
			goto do_emit_instruction;

		default:
			/* Check if a constant-encoded immediate operand needs extension. */
			if (current_assembler.a_constc > UINT8_MAX &&
			    (iter->ao_flags & ASM_OVERLOAD_FCONSTIMM) &&
			    OPERAND_CLASS_ISDISP(UNALIGNED_GET16(&iter->ao_ops[i].aoo_class)))
				goto do_emit_f0_prefix;
			break;
		}
	}
	if (iter->ao_flags & ASM_OVERLOAD_F16BIT)
		emit_sym16 = true;

	/* Now emit the instruction itself. */
do_emit_instruction:
	if ((iter->ao_instr & 0xff00) &&
	    asm_put((instruction_t)(iter->ao_instr >> 8)))
		goto err;
	if (asm_put((instruction_t)iter->ao_instr))
		goto err;

	/* Now emit operands in ascending order. */
	for (i = 0; i < iter->ao_opcount; ++i) {
		tint_t imm_val;
		struct asm_sym *imm_sym;
		uint16_t imm_rel;
		imm_val = invoc->ai_ops[i].io_intexpr.ie_val;
		imm_sym = invoc->ai_ops[i].io_intexpr.ie_sym;
		imm_rel = invoc->ai_ops[i].io_intexpr.ie_rel;
		if (imm_rel == (uint16_t)-1) {
			imm_rel = iter->ao_flags;
		} else if (iter->ao_flags & ASM_OVERLOAD_FREL_DSPBIT) {
			/* Toggle the relative displacement bit. */
			imm_rel ^= ASM_OVERLOAD_FREL_DSPBIT;
		}
		switch (UNALIGNED_GET16(&iter->ao_ops[i].aoo_class) &
		        (OPERAND_CLASS_FSPADD | OPERAND_CLASS_FSPSUB)) {

		case OPERAND_CLASS_FSPADD: /* `SP + imm' */
			imm_val -= current_assembler.a_stackcur;
			break;

		case OPERAND_CLASS_FSPSUB: /* `SP - imm' */
			imm_val = current_assembler.a_stackcur - imm_val;
			break;

		case OPERAND_CLASS_FSUBSP: /* `imm - SP' */
			imm_val += current_assembler.a_stackcur;
			break;

		default: break;
		}
		imm_val += iter->ao_ops[i].aoo_disp;
#if 0
		/* Relative stack displacements work in reverse. */
		if (imm_rel == ASM_OVERLOAD_FSTKDSP)
			imm_val = -imm_val;
#endif
		if ((iter->ao_flags & ASM_OVERLOAD_FCONSTIMM) &&
		    OPERAND_CLASS_ISDISP(UNALIGNED_GET16(&iter->ao_ops[i].aoo_class))) {
			int32_t cid;
			/* Encode as a relocation-enabled integer constant. */
			switch (imm_rel & ASM_OVERLOAD_FRELMSK) {

			case ASM_OVERLOAD_FRELABS:
				break;

			case ASM_OVERLOAD_FRELDSP:
				imm_val -= asm_ip();
				break;

			case ASM_OVERLOAD_FSTKABS:
				break;

			case ASM_OVERLOAD_FSTKDSP:
				imm_val = -imm_val;
				break;

			default: __builtin_unreachable();
			}
			cid = asm_newrelint(imm_sym, imm_val,
			                    imm_rel & ASM_OVERLOAD_FREL_STKBIT
			                    ? RELINT_MODE_FSTCK
			                    : RELINT_MODE_FADDR);
			if unlikely(cid < 0)
				goto err;
			if (emit_sym16 ? asm_put_data16((uint16_t)cid)
			               : asm_put_data8((uint8_t)cid))
				goto err;
		} else {
			switch (UNALIGNED_GET16(&iter->ao_ops[i].aoo_class) & OPERAND_CLASS_FMASK) {

				/* Symbol operands. */
			case OPERAND_CLASS_EXTERN:
				if (emit_sym16 ? asm_put_data16((uint16_t)invoc->ai_ops[i].io_extern.io_modid)
				               : asm_put_data8((uint8_t)invoc->ai_ops[i].io_extern.io_modid))
					goto err;
				ATTR_FALLTHROUGH
			case OPERAND_CLASS_REF:
			case OPERAND_CLASS_CONST:
			case OPERAND_CLASS_STATIC:
			case OPERAND_CLASS_MODULE:
			case OPERAND_CLASS_GLOBAL:
			case OPERAND_CLASS_LOCAL:
			case OPERAND_CLASS_ARG:
/*do_emit_symid_816:*/
				if (emit_sym16 ? asm_put_data16((uint16_t)invoc->ai_ops[i].io_symid)
				               : asm_put_data8((uint8_t)invoc->ai_ops[i].io_symid))
					goto err;
				break;

				/* Immediate operands (of any kind). */
			case OPERAND_CLASS_SDISP8:
			case OPERAND_CLASS_DISP8:
				if (!emit_imm16) {
					if (imm_sym && asm_putrel_f(imm_rel, imm_sym, 8))
						goto err;
					if (asm_put_data8((uint8_t)imm_val))
						goto err;
					break;
				}
				ATTR_FALLTHROUGH
			case OPERAND_CLASS_SDISP16:
			case OPERAND_CLASS_DISP16:
				if (imm_sym && asm_putrel_f(imm_rel, imm_sym, 16))
					goto err;
				if (asm_put_data16((uint16_t)imm_val))
					goto err;
				break;

			case OPERAND_CLASS_SDISP32:
			case OPERAND_CLASS_DISP32:
				if (imm_sym && asm_putrel_f(imm_rel, imm_sym, 32))
					goto err;
				if (asm_put_data32((uint32_t)imm_val))
					goto err;
				break;

			case OPERAND_CLASS_DISP8_HALF:
				if (!emit_imm16) {
					ASSERTF(!imm_sym, "DISP8_HALF cannot be used with symbols");
					if (asm_put_data8((uint8_t)((uint16_t)imm_val >> 1)))
						goto err;
					break;
				}
				ATTR_FALLTHROUGH
			case OPERAND_CLASS_DISP16_HALF:
				ASSERTF(!imm_sym, "DISP16_HALF cannot be used with symbols");
				if (asm_put_data16((uint16_t)((uint32_t)imm_val >> 1)))
					goto err;
				break;

			default: break;
			}
		}
	}
/*done_instruction:*/
	/* Save the last-written instruction. */
	current_userasm.ua_lasti = iter->ao_instr;

	/* Finally, update the stack effect. */
	old_sp    = current_assembler.a_stackcur;
	instr_end = DeeAsm_NextInstrEf(current_assembler.a_curr->sec_begin + instr_start,
	                             &current_assembler.a_stackcur, &sp_add, &sp_sub);
	ASSERTF(instr_end == current_assembler.a_curr->sec_iter,
	        "Generated instruction does not terminate properly");
	if (current_userasm.ua_mode & USER_ASM_FSTKINV && (sp_add || sp_sub)) {
		DeeError_Throwf(&DeeError_CompilerError,
		                "Instruction `%s' with stack effect used while the stack is in an undefined state",
		                instr->am_name);
		goto err;
	}

	if unlikely(sp_sub > old_sp) {
		/* Negative stack offset? */
#if 1
		DeeError_Throwf(&DeeError_CompilerError,
		                "Negative stack effect during `%s' instruction",
		                instr->am_name);
		goto err;
#else
		/* Peephole optimization would not be able to deal with this... */
		current_assembler.a_flag &= ~ASM_FPEEPHOLE;
#endif
	}
	/* Update stack requirements. */
	if (current_assembler.a_stackmax < current_assembler.a_stackcur)
		current_assembler.a_stackmax = current_assembler.a_stackcur;
	/* Enter undefined-stack mode if adjstack is used with a relocatable symbol. */
	if ((iter->ao_instr & 0xff) == ASM_ADJSTACK &&
	    invoc->ai_ops[0].io_intexpr.ie_sym != NULL) {
#if 1 /* Disable peephole, because who knows what the user might do to define this symbol... */
		current_assembler.a_flag &= ~ASM_FPEEPHOLE;
#endif
		current_userasm.ua_mode |= USER_ASM_FSTKINV;
	}
}

	/* Make sure to always set the assembly flag
	 * for code objects containing user-assembly. */
	current_basescope->bs_flags |= CODE_FASSEMBLY;
done:
	return 0;
}


#define get_label_repr(label_number) \
	DeeString_Newf(USERLABEL_PREFIX "%Iu", (size_t)(label_number))
#define OPNAME1(a)          (uint32_t)((a))
#define OPNAME2(a, b)       (uint32_t)((b) | (a) << 8)
#define OPNAME3(a, b, c)    (uint32_t)((c) | (b) << 8 | (a) << 16)
#define OPNAME4(a, b, c, d) (uint32_t)((d) | (c) << 8 | (b) << 16 | (a) << 24)

struct assembler_state {
	uint16_t as_handlerc; /* The old amount of active catch/finally handlers (`a_handlerc'). */
	uint16_t as_stackcur; /* The old stack alignment before user-assembly began compiling (`a_stackcur'). */
};

/* User-assembly operand identifiers. */
#define ASM_OP_INOUT         OPNAME1('+')           /* Switch operand mode to input/output. */
#define ASM_OP_OUT           OPNAME1('=')           /* Switch operand mode to output. */
#define ASM_OP_NONE          OPNAME1('n')           /* `none'                       The `none' builtin constant. */
#define ASM_OP_STACK         OPNAME1('s')           /* `stack #7'                   An absolute position on the stack, located at the top. */
#define ASM_OP_ABSSTACK      OPNAME1('S')           /* `stack #7'                   An absolute position on the stack. */
#define ASM_OP_EXCEPT        OPNAME1('E')           /* `except'                     The currently active exception. */
#define ASM_OP_MODULE        OPNAME1('M')           /* `module <imm8|16>'           An imported module descriptor. */
#define ASM_OP_THIS          OPNAME2('T', 'i')      /* `this'                       The this-argument of a this-class function. */
#define ASM_OP_THIS_MODULE   OPNAME2('T', 'm')      /* `this_module'                The calling module. */
#define ASM_OP_THIS_FUNCTION OPNAME2('T', 'f')      /* `this_function'              The calling function. */
#define ASM_OP_REF           OPNAME1('r')           /* `ref <imm8|16>'              A referenced variable. */
#define ASM_OP_REF_GEN       OPNAME1('R')           /* `ref <imm8|16>'              A referenced variable (create new references for variables where that is possible). */
#define ASM_OP_ARG           OPNAME1('a')           /* `arg <imm8|16>'              An argument variable. */
#define ASM_OP_VARG          OPNAME2('A', 'v')      /* `varargs'                    Variable positional arguments. */
#define ASM_OP_VKWD          OPNAME2('A', 'k')      /* `varkwds'                    Variable keyword arguments. */
#define ASM_OP_CONST         OPNAME1('c')           /* `const <imm8|16>'            A constant expression. */
#define ASM_OP_STATIC        OPNAME2('C', 's')      /* `static <imm8|16>'           A static expression. */
#define ASM_OP_EXTERN        OPNAME1('e')           /* `extern <imm8|16>:<imm8|16>' An external symbol. */
#define ASM_OP_GLOBAL        OPNAME1('g')           /* `global <imm8|16>'           A global symbol. */
#define ASM_OP_LOCAL         OPNAME1('l')           /* `local <imm8|16>'            A local symbol. */
#define ASM_OP_LOCAL_GEN     OPNAME1('L')           /* `local <imm8|16>'            A local symbol (allow the use of an anonymous local). */
#define ASM_OP_TRUE          OPNAME2('T', 'T')      /* `true'                       A constant true. */
#define ASM_OP_FALSE         OPNAME2('F', 'F')      /* `false'                      A constant false. */
#define ASM_OP_IMMZERO       OPNAME2('I', 'z')      /* `$0'                         An integer equal to zero. */
#define ASM_OP_SIMM8         OPNAME2('I', '8')      /* `$42'                        Any signed 8-bit integer. */
#define ASM_OP_SIMM16        OPNAME3('I', '1', '6') /* `$42'                        Any signed 16-bit integer. */
#define ASM_OP_SIMM32        OPNAME3('I', '3', '2') /* `$42'                        Any signed 32-bit integer. */
#define ASM_OP_SIMM64        OPNAME3('I', '6', '4') /* `$42'                        Any signed 64-bit integer. */
#define ASM_OP_IMM8          OPNAME2('N', '8')      /* `$42'                        Any unsigned 8-bit integer. */
#define ASM_OP_IMM16         OPNAME3('N', '1', '6') /* `$42'                        Any unsigned 16-bit integer. */
#define ASM_OP_IMM32         OPNAME3('N', '3', '2') /* `$42'                        Any unsigned 32-bit integer. */
#define ASM_OP_IMM64         OPNAME3('N', '6', '4') /* `$42'                        Any unsigned 64-bit integer. */
#define ASM_OP_CAST          OPNAME2('C', 'a')      /* `...'                        A valid operand for the `cast' instruction. */
#define ASM_OP_SEQ_SEQ       OPNAME2('Q', 's')      /* `[#...]'                     A sequence-operand, as accepted by the `call' instruction. */
#define ASM_OP_SEQ_MAP       OPNAME2('Q', 'm')      /* `{#...}'                     A Dict-compatible sequence-operand, as accepted by the `call' instruction. */
#define ASM_OP_PREFIX        OPNAME1('p')           /* Input operand: Same as `ceglCsS' (Any prefix operand, including const-as-static)
                                                     * Output operand: Same as `eglCsS' (Any prefix operand, including const-as-static) */
#define ASM_OP_INTEGER       OPNAME1('i')           /* Same as `I32N32' (Any integer operand). */
#define ASM_OP_SYMBOL        OPNAME1('v')           /* Same as `raAvAkcCseglRS' (Any symbol, potentially immutable). */
#define ASM_OP_VARIABLE      OPNAME1('V')           /* Same as `eglS' (Any symbol, must be mutable). */
#define ASM_OP_BINDABLE      OPNAME1('b')           /* Same as `egl' (Any symbol, must be able to be unbound). */
#define ASM_OP_PUSH          OPNAME1('P')           /* Input operand:  Same as `nEMTiTmTfraAvAkcCseglTTFFTcI64N64SR' (All operands accepted by the `push' instruction)
                                                     * Output operand: Same as `eglCsS' (All operands accepted by the `pop' instruction)
                                                     * In/out operand: Same as output operand (All operands accepted by both the `push' and `pop' instructions) */
#define ASM_OP_ANYTHING      OPNAME1('x')           /* Any kind of operand (only really meaningful for artificial dependencies). */

#define OPTION_MODE_UNDEF    0
#define OPTION_MODE_INPUT    1
#define OPTION_MODE_OUTPUT   2
#define OPTION_MODE_INOUT    3
#define OPTION_MODE_TRY   0x80

PRIVATE uint32_t DCALL fix_option_name(uint32_t name) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	if (name & 0xff000000)
		;
	else if (name & 0x00ff0000)
		name = name << 8;
	else if (name & 0x0000ff00)
		name = name << 16;
	else {
		name = name << 24;
	}
#else /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */
	if (name & 0xff000000) {
		name = BSWAP32(name);
	} else if (name & 0x00ff0000) {
		name = BSWAP32(name) >> 8;
	} else if (name & 0x0000ff00) {
		name = (uint32_t)BSWAP16((uint16_t)name);
	}
#endif /* __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ */
	return name;
}

struct cleanup_mode {
	uint16_t cm_kind;   /* The kind of cleanup (One of `CLEANUP_MODE_F*') */
#define CLEANUP_MODE_FNONE      0x0000 /* No special cleanup required. */
#define CLEANUP_MODE_FSTACK     0x0001 /* Cleanup by popping a value from the stack. */
#define CLEANUP_MODE_FLOCAL     0x0002 /* Cleanup by deleting a local variable. */
#define CLEANUP_MODE_FLOCAL_POP 0x0003 /* Cleanup by loading from a local variable & deleting it. */
	uint16_t cm_value;  /* For `CLEANUP_MODE_FLOCAL*': The local variable ID */
};


#ifndef NDEBUG
#define DBG_INITIALIZE_FAKE_LOCAL_SYMBOL(sym) \
	memset(sym, 0xcc, sizeof(struct symbol))
#else /* NDEBUG */
#define DBG_INITIALIZE_FAKE_LOCAL_SYMBOL(sym) (void)0
#endif /* !NDEBUG */
#define INITIALIZE_FAKE_LOCAL_SYMBOL(sym, lid)               \
	(DBG_INITIALIZE_FAKE_LOCAL_SYMBOL(sym),                  \
	 (sym)->s_decl.l_file = NULL,                            \
	 (sym)->s_scope   = (DeeScopeObject *)current_basescope, \
	 (sym)->s_nread   = (sym)->s_nwrite = 1,                 \
	 (sym)->s_nbound  = 0,                                   \
	 (sym)->s_type    = SYMBOL_TYPE_LOCAL,                   \
	 (sym)->s_flag    = SYMBOL_FALLOC,                       \
	 (sym)->s_symid   = (lid))


PRIVATE WUNUSED NONNULL((1, 2, 4, 5)) DREF DeeObject *DCALL
get_assembly_formatter_oprepr(struct ast *__restrict self,
                              char const *__restrict format, int mode,
                              struct cleanup_mode *__restrict cleanup,
                              struct assembler_state const *__restrict init_state) {
	DREF DeeObject *result;
	uint32_t option;
	struct symbol *sym;
	char const *format_start;
	bool try_repr;
	format_start = format;
	try_repr     = (mode & OPTION_MODE_TRY) != 0;
	mode &= ~OPTION_MODE_TRY;
next_option:
	option = 0;
cont_option:
	if (option & 0xff000000) {
unknown_encoding:
		option = fix_option_name(option);
		DeeError_Throwf(&DeeError_CompilerError,
		                "Unknown operand encoding %.4q",
		                &option);
		goto err;
	}
	/* Check for format end. */
	if (!*format) {
		if (option)
			goto unknown_encoding;
		if (try_repr)
			return ITER_DONE;
		DeeError_Throwf(&DeeError_CompilerError,
		                "Operand for %q is not compatible with any possible encoding",
		                format_start);
		goto err;
	}
	/* Extend the active option. */
	option <<= 8;
	option |= (uint8_t)*format++;
	switch (option) {

		/* Separators. */
	case ' ':
	case ',':
		goto next_option;

	case ASM_OP_INOUT:
		if (mode != OPTION_MODE_UNDEF)
			goto err_illegal_option;
		mode = OPTION_MODE_INOUT;
		goto next_option;

	case ASM_OP_OUT:
		if (mode != OPTION_MODE_UNDEF)
			goto err_illegal_option;
		mode = OPTION_MODE_OUTPUT;
		goto next_option;

	case ASM_OP_NONE:
		/* None-operand */
		if (!AST_ISNONE(self))
			goto next_option;
		if (ast_genasm(self, ASM_G_FNORMAL))
			goto err;
		result = (DeeObject *)&str_none;
		Dee_Incref(result);
		break;

	case ASM_OP_ABSSTACK:
abs_stack_any:
		if ((mode == OPTION_MODE_INOUT || mode == OPTION_MODE_INPUT) &&
		    self->a_type == AST_SYM) {
			sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
			if (sym->s_type == SYMBOL_TYPE_STACK &&
			    !SYMBOL_MUST_REFERENCE_TYPEMAY(sym) &&
			    (sym->s_flag & SYMBOL_FALLOC)) {
				/* in/out stack-operand */
				result = DeeString_Newf("stack #%I16u", SYMBOL_STACK_OFFSET(sym));
				break;
			}
		}
		ATTR_FALLTHROUGH
	case ASM_OP_STACK:
		/* Stack operand. */
		if (mode == OPTION_MODE_UNDEF)
			goto err_undefined_mode;
		/* Must pop the operand if it isn't input-only. */
		if (mode != OPTION_MODE_INPUT)
			cleanup->cm_kind = CLEANUP_MODE_FSTACK;
		if (mode != OPTION_MODE_OUTPUT) {
			if (ast_genasm(self, ASM_G_FPUSHRES))
				goto err;
		} else if (option != ASM_OP_STACK) {
			/* Since this is a stack-operand, we _must_ provide an initial
			 * value despite the fact that it is an output-only operation. */
			if (asm_gpush_none())
				goto err;
		} else {
			/* Output-only stack-top operand (user-assembly must leave the value ontop of the stack) */
			return_empty_string;
		}
		result = DeeString_Newf("stack #%I16u", current_assembler.a_stackcur - 1);
		if unlikely(!result)
			goto err;
		break;

	case ASM_OP_EXCEPT:
		if (self->a_type != AST_SYM)
			goto next_option;
		sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
		if (sym->s_type != SYMBOL_TYPE_EXCEPT)
			goto next_option;
		if (SYMBOL_MUST_REFERENCE_TYPEMAY(sym))
			goto next_option;
		result = (DeeObject *)&str_except;
		Dee_Incref(result);
		break;

	case ASM_OP_MODULE: {
		int32_t mid;
		if (self->a_type != AST_SYM)
			goto next_option;
		sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
		if (sym->s_type != SYMBOL_TYPE_MODULE)
			goto next_option;
		mid = asm_msymid(sym);
		if unlikely(mid < 0)
			goto err;
		result = DeeString_Newf("module %I16u", (uint16_t)mid);
	}	break;

	case ASM_OP_THIS:
		if (self->a_type != AST_SYM)
			goto next_option;
		sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
		if (sym->s_type != SYMBOL_TYPE_THIS)
			goto next_option;
		if (SYMBOL_MUST_REFERENCE_TYPEMAY(sym))
			goto next_option;
		result = (DeeObject *)&str_this;
		Dee_Incref(result);
		break;

	case ASM_OP_THIS_MODULE:
		if (self->a_type != AST_SYM)
			goto next_option;
		sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
		if (sym->s_type != SYMBOL_TYPE_MYMOD)
			goto next_option;
		result = (DeeObject *)&str_this_module;
		Dee_Incref(result);
		break;

	case ASM_OP_THIS_FUNCTION:
		if (self->a_type != AST_SYM)
			goto next_option;
		sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
		if (sym->s_type != SYMBOL_TYPE_MYFUNC)
			goto next_option;
		if (SYMBOL_MUST_REFERENCE_TYPEMAY(sym))
			goto next_option;
		result = (DeeObject *)&str_this_function;
		Dee_Incref(result);
		break;

	case ASM_OP_REF: {
		int32_t rid;
		if (self->a_type != AST_SYM)
			goto next_option;
		sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
		if (!SYMBOL_MUST_REFERENCE(sym))
			goto next_option;
		rid = asm_rsymid(sym);
		if unlikely(rid < 0)
			goto err;
		result = DeeString_Newf("ref %I16u", (uint16_t)rid);
	}	break;

	case ASM_OP_REF_GEN: {
		int32_t rid;
		if (self->a_type == AST_CONSTEXPR &&
		    current_basescope != (DeeBaseScopeObject *)current_rootscope) {
			/* Check if the object is being exported from `deemon' */
			sym = asm_bind_deemon_export(self->a_constexpr);
			if unlikely(!sym)
				goto err;
			if (sym == ASM_BIND_DEEMON_EXPORT_NOTFOUND)
				goto next_option;
		} else {
			if (self->a_type != AST_SYM)
				goto next_option;
			sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
			/* Check for may-reference, thus allowing anything that ~could~ be referenced. */
			if (!SYMBOL_MAY_REFERENCE(sym))
				goto next_option;
		}
		rid = asm_rsymid(sym);
		if unlikely(rid < 0)
			goto err;
		result = DeeString_Newf("ref %I16u", (uint16_t)rid);
	}	break;

	case ASM_OP_ARG:
		if (self->a_type != AST_SYM)
			goto next_option;
		sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
		if (sym->s_type != SYMBOL_TYPE_ARG)
			goto next_option;
		if (SYMBOL_MUST_REFERENCE_TYPEMAY(sym))
			goto next_option;
		if (DeeBaseScope_IsVarargs(current_basescope, sym) ||
		    DeeBaseScope_IsVarkwds(current_basescope, sym))
			goto next_option;
		result = DeeString_Newf("arg %I16u", sym->s_symid);
		break;

	case ASM_OP_VARG:
		if (self->a_type != AST_SYM)
			goto next_option;
		sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
		if (sym->s_type != SYMBOL_TYPE_ARG)
			goto next_option;
		if (SYMBOL_MUST_REFERENCE_TYPEMAY(sym))
			goto next_option;
		if (!DeeBaseScope_IsVarargs(current_basescope, sym))
			goto next_option;
		result = DeeString_New("varargs");
		break;

	case ASM_OP_VKWD:
		if (self->a_type != AST_SYM)
			goto next_option;
		sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
		if (sym->s_type != SYMBOL_TYPE_ARG)
			goto next_option;
		if (SYMBOL_MUST_REFERENCE_TYPEMAY(sym))
			goto next_option;
		if (!DeeBaseScope_IsVarkwds(current_basescope, sym))
			goto next_option;
		result = DeeString_New("varkwds");
		break;

	case ASM_OP_CONST: {
		int32_t cid;
		if (self->a_type != AST_CONSTEXPR)
			goto next_option;
		if (!asm_allowconst(self->a_constexpr))
			goto next_option;
		cid = asm_newconst(self->a_constexpr);
		if unlikely(cid < 0)
			goto err;
		result = DeeString_Newf("const %I16u", (uint16_t)cid);
	}	break;

	case ASM_OP_STATIC: {
		int32_t sid;
		if (self->a_type != AST_SYM)
			goto next_option;
		sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
		if (sym->s_type != SYMBOL_TYPE_STATIC)
			goto next_option;
		if (SYMBOL_MUST_REFERENCE_TYPEMAY(sym))
			goto next_option;
		if (mode == OPTION_MODE_INPUT)
			sid = asm_ssymid_for_read(sym, self);
		else {
			sid = asm_ssymid(sym);
		}
		if unlikely(sid < 0)
			goto err;
		result = DeeString_Newf("static %I16u", (uint16_t)sid);
	}	break;

	case ASM_OP_EXTERN: {
		int32_t eid;
		if (self->a_type != AST_SYM)
			goto next_option;
		sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
		if (sym->s_type != SYMBOL_TYPE_EXTERN)
			goto next_option;
		if (SYMBOL_EXTERN_SYMBOL(sym)->ss_flags & MODSYM_FPROPERTY)
			goto next_option;
		eid = asm_esymid(sym);
		if unlikely(eid < 0)
			goto err;
		result = DeeString_Newf("extern %I16u:%I16u", (uint16_t)eid,
		                        SYMBOL_EXTERN_SYMBOL(sym)->ss_index);
	}	break;

	case ASM_OP_GLOBAL: {
		int32_t gid;
		if (self->a_type != AST_SYM)
			goto next_option;
		sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
		if (sym->s_type != SYMBOL_TYPE_GLOBAL)
			goto next_option;
		if (mode == OPTION_MODE_INPUT)
			gid = asm_gsymid_for_read(sym, self);
		else {
			gid = asm_gsymid(sym);
		}
		if unlikely(gid < 0)
			goto err;
		result = DeeString_Newf("global %I16u", (uint16_t)gid);
	}	break;

	case ASM_OP_LOCAL: {
		int32_t lid;
		if (self->a_type != AST_SYM)
			goto next_option;
		sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
		if (sym->s_type != SYMBOL_TYPE_LOCAL)
			goto next_option;
		if (SYMBOL_MUST_REFERENCE_TYPEMAY(sym))
			goto next_option;
write_regular_local:
		if (mode == OPTION_MODE_INPUT)
			lid = asm_lsymid_for_read(sym, self);
		else {
			lid = asm_lsymid(sym);
		}
		if unlikely(lid < 0)
			goto err;
		result = DeeString_Newf("local %I16u", (uint16_t)lid);
	}	break;

	case ASM_OP_LOCAL_GEN: {
		int32_t lid;
		if (self->a_type == AST_SYM) {
			sym = SYMBOL_UNWIND_ALIAS(self->a_sym);
			if (sym->s_type == SYMBOL_TYPE_LOCAL &&
			    !SYMBOL_MUST_REFERENCE_TYPEMAY(sym))
				goto write_regular_local;
		}
		lid = asm_newlocal();
		if unlikely(lid < 0)
			goto err;
		if (mode != OPTION_MODE_OUTPUT) {
			struct symbol temp;
			INITIALIZE_FAKE_LOCAL_SYMBOL(&temp, (uint16_t)lid);
			if (asm_gmov_sym_ast(&temp, self, self))
				goto err;
		}
		cleanup->cm_kind = CLEANUP_MODE_FLOCAL;
		if (mode != OPTION_MODE_INPUT)
			cleanup->cm_kind = CLEANUP_MODE_FLOCAL_POP;
		cleanup->cm_value = (uint16_t)lid;
		result            = DeeString_Newf("local %I16u", (uint16_t)lid);
	}	break;

	case ASM_OP_TRUE:
		if (self->a_type != AST_CONSTEXPR)
			goto next_option;
		if (self->a_constexpr != Dee_True)
			goto next_option;
		result = (DeeObject *)&str_true;
		Dee_Incref(result);
		break;

	case ASM_OP_FALSE:
		if (self->a_type != AST_CONSTEXPR)
			goto next_option;
		if (self->a_constexpr != Dee_False)
			goto next_option;
		result = (DeeObject *)&str_false;
		Dee_Incref(result);
		break;

	case ASM_OP_IMMZERO: {
		uint32_t value;
		if (self->a_type != AST_CONSTEXPR)
			goto next_option;
		if (!DeeInt_Check(self->a_constexpr))
			goto next_option;
		if (!DeeInt_TryAsU32(self->a_constexpr, &value))
			goto next_option;
		if (value != 0)
			goto next_option;
		result = DeeString_New("$0");
	}	break;

	case ASM_OP_SEQ_MAP:
		if (self->a_type == AST_MULTIPLE &&
		    self->a_flag != AST_FMULTIPLE_KEEPLAST &&
		    AST_FMULTIPLE_ISDICT(self->a_flag)) {
			size_t i, length = self->a_multiple.m_astc & ~1;
			for (i = 0; i < length; ++i) {
				if (ast_genasm_one(self->a_multiple.m_astv[i], ASM_G_FPUSHRES))
					goto err;
			}
			result = DeeString_Newf("{#%u}", length);
			break;
		}
		if (self->a_type == AST_CONSTEXPR &&
		    DeeDict_Check(self->a_constexpr)) {
			/* Push the key-item pairs of a Dict. */
			size_t i, length = 0;
			DeeDictObject *d = (DeeDictObject *)self->a_constexpr;
			DeeDict_LockRead(d);
			if (d->d_used > UINT8_MAX) {
				DeeDict_LockEndRead(d);
				goto next_option;
			}
			for (i = 0; i <= d->d_mask; ++i) {
				int error;
				DREF DeeObject *key, *item;
				key = d->d_elem[i].di_key;
				if (!key || key == &DeeDict_Dummy)
					continue;
				item = d->d_elem[i].di_value;
				Dee_Incref(key);
				Dee_Incref(item);
				DeeDict_LockEndRead(d);
				error = asm_gpush_constexpr(key);
				if likely(!error)
					error = asm_gpush_constexpr(item);
				Dee_Decref(key);
				Dee_Decref(item);
				if unlikely(error)
					goto err;
				++length;
				DeeDict_LockRead(d);
			}
			DeeDict_LockEndRead(d);
			if unlikely(length > UINT8_MAX) {
				/* Shouldn't really happen, but is required due to race conditions.
				 * Peephole optimization should be able to get rid of this later... */
				length *= 2;
				while (length--)
					if (asm_gpop())
						goto err;
				goto next_option;
			}
			result = DeeString_Newf("{#%u}", length * 2);
			break;
		}
		goto next_option;

	case ASM_OP_SEQ_SEQ:
		if (self->a_type == AST_MULTIPLE &&
		    self->a_flag != AST_FMULTIPLE_KEEPLAST &&
		    !AST_FMULTIPLE_ISDICT(self->a_flag)) {
			size_t i, length = self->a_multiple.m_astc;
			for (i = 0; i < length; ++i) {
				if (ast_genasm_one(self->a_multiple.m_astv[i], ASM_G_FPUSHRES))
					goto err;
			}
			result = DeeString_Newf("[#%u]", length);
			break;
		}
		if (self->a_type == AST_CONSTEXPR) {
			DREF DeeObject *iter, *elem;
			size_t length;
			/* Check the length of the sequence. */
			length = DeeObject_Size(self->a_constexpr);
			if unlikely(length == (size_t)-1) {
				if (DeeError_Catch(&DeeError_NotImplemented))
					goto next_option;
				goto err;
			}
			if (length > UINT8_MAX)
				goto next_option;
			iter = DeeObject_IterSelf(self->a_constexpr);
			if unlikely(!iter) {
				if (DeeError_Catch(&DeeError_NotImplemented))
					goto next_option;
				goto err;
			}
			/* Push all the elements of the sequence. */
			length = 0;
			while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
				int error;
				error = asm_gpush_constexpr(elem);
				Dee_Decref(elem);
				if unlikely(!error) {
					elem = NULL;
					break;
				}
				++length;
			}
			Dee_Decref(iter);
			if unlikely(!elem)
				goto err;
			if unlikely(length > UINT8_MAX) {
				/* Shouldn't really happen, but is required due to race conditions.
				 * Peephole optimization should be able to get rid of this later... */
				while (length--)
					if (asm_gpop())
						goto err;
				goto next_option;
			}
			result = DeeString_Newf("{#%u}", length);
			break;
		}
		goto next_option;

	{
		int64_t intval, intmin, intmax;
		__IF0 {
	case ASM_OP_SIMM8:
			intmin = INT8_MIN;
			intmax = INT8_MAX;
		}
		__IF0 {
	case ASM_OP_SIMM16:
			intmin = INT16_MIN;
			intmax = INT16_MAX;
		}
		__IF0 {
	case ASM_OP_SIMM32:
			intmin = INT32_MIN;
			intmax = INT32_MAX;
		}
		__IF0 {
	case ASM_OP_SIMM64:
			intmin = INT64_MIN;
			intmax = INT64_MAX;
		}
		if (self->a_type != AST_CONSTEXPR)
			goto next_option;
		if (!DeeInt_Check(self->a_constexpr))
			goto next_option;
		if (!DeeInt_TryAsS64(self->a_constexpr, &intval))
			goto next_option;
		if (intval < intmin || intval > intmax)
			goto next_option;
		result = DeeString_Newf("$%I64d", intval);
	}	break;

	{
		uint64_t intval, intmax;
		__IF0 {
	case ASM_OP_IMM8:
			intmax = UINT8_MAX;
		}
		__IF0 {
	case ASM_OP_IMM16:
			intmax = UINT16_MAX;
		}
		__IF0 {
	case ASM_OP_IMM32:
			intmax = UINT32_MAX;
		}
		__IF0 {
	case ASM_OP_IMM64:
			intmax = UINT64_MAX;
		}
		if (self->a_type != AST_CONSTEXPR)
			goto next_option;
		if (!DeeInt_Check(self->a_constexpr))
			goto next_option;
		if (!DeeInt_TryAsU64(self->a_constexpr, &intval))
			goto next_option;
		if (intval > intmax)
			goto next_option;
		result = DeeString_Newf("$%I64u", intval);
	}	break;

	case ASM_OP_CAST:
		if (self->a_type != AST_CONSTEXPR)
			goto next_option;
		if (self->a_constexpr == (DeeObject *)&DeeTuple_Type) {
			result = (DeeObject *)&str_Tuple;
			Dee_Incref(result);
			break;
		}
		if (self->a_constexpr == (DeeObject *)&DeeList_Type) {
			result = (DeeObject *)&str_List;
			Dee_Incref(result);
			break;
		}
		if (self->a_constexpr == (DeeObject *)&DeeDict_Type) {
			result = (DeeObject *)&str_Dict;
			Dee_Incref(result);
			break;
		}
		if (self->a_constexpr == (DeeObject *)&DeeHashSet_Type) {
			result = (DeeObject *)&str_HashSet;
			Dee_Incref(result);
			break;
		}
		if (self->a_constexpr == (DeeObject *)&DeeInt_Type) {
			result = (DeeObject *)&str_int;
			Dee_Incref(result);
			break;
		}
		if (self->a_constexpr == (DeeObject *)&DeeBool_Type) {
			result = (DeeObject *)&str_bool;
			Dee_Incref(result);
			break;
		}
		goto next_option;

	case ASM_OP_PREFIX:
		if (mode == OPTION_MODE_INPUT) {
			result = get_assembly_formatter_oprepr(self, "ceglCsS", mode | OPTION_MODE_TRY,
			                                       cleanup, init_state);
		} else {
			result = get_assembly_formatter_oprepr(self, "eglCsS", mode | OPTION_MODE_TRY,
			                                       cleanup, init_state);
		}
		if (!result)
			goto err;
		if (result != ITER_DONE)
			break;
		goto next_option;

	case ASM_OP_INTEGER:
		result = get_assembly_formatter_oprepr(self, "I32N32", mode | OPTION_MODE_TRY,
		                                       cleanup, init_state);
		if (!result)
			goto err;
		if (result != ITER_DONE)
			break;
		goto next_option;

	case ASM_OP_SYMBOL:
		result = get_assembly_formatter_oprepr(self, "raAvAkcCseglRS", mode | OPTION_MODE_TRY,
		                                       cleanup, init_state);
		if (!result)
			goto err;
		if (result != ITER_DONE)
			break;
		goto next_option;

	case ASM_OP_VARIABLE:
		result = get_assembly_formatter_oprepr(self, "eglS", mode | OPTION_MODE_TRY,
		                                       cleanup, init_state);
		if (!result)
			goto err;
		if (result != ITER_DONE)
			break;
		goto next_option;

	case ASM_OP_BINDABLE:
		result = get_assembly_formatter_oprepr(self, "egl", mode | OPTION_MODE_TRY,
		                                       cleanup, init_state);
		if (!result)
			goto err;
		if (result != ITER_DONE)
			break;
		goto next_option;

	case ASM_OP_PUSH:
		if (mode == OPTION_MODE_UNDEF)
			goto err_undefined_mode;
		if (mode == OPTION_MODE_INPUT)
			result = get_assembly_formatter_oprepr(self, "nEMTiTmTfraAvAkcCseglTTFFTcI64N64SR",
			                                       mode | OPTION_MODE_TRY,
			                                       cleanup, init_state);
		else {
			result = get_assembly_formatter_oprepr(self, "eglCsS",
			                                       mode | OPTION_MODE_TRY,
			                                       cleanup, init_state);
		}
		if (!result)
			goto err;
		if (result != ITER_DONE)
			break;
		goto next_option;

	case ASM_OP_ANYTHING:
		result = get_assembly_formatter_oprepr(self, "nEMTiTmTfraAvAkcCseglTTFFI64N64CaQsQm",
		                                       mode | OPTION_MODE_TRY,
		                                       cleanup, init_state);
		if (!result)
			goto err;
		if (result != ITER_DONE)
			break;
		/* Fallback: use the stack to pass the value. */
		goto abs_stack_any;

		/* Continue reading in the option name. */
	default: goto cont_option;
	}
	return result;
err:
	return NULL;
err_illegal_option:
	option = fix_option_name(option);
	DeeError_Throwf(&DeeError_CompilerError,
	                "Illegal operand encoding %.4q in %q",
	                &option, format_start);
	goto err;
err_undefined_mode:
	DeeError_Throwf(&DeeError_CompilerError,
	                "No operand mode has been defined by %q",
	                format_start);
	goto err;
}

struct assembly_formatter {
	struct ast            *af_ast;     /* [1..1] The user-assembly ast. */
	struct ascii_printer   af_printer; /* Printer for the resulting assembly text. */
	DREF DeeStringObject **af_opreprv; /* [1..1][af_ast->a_assembly.as_opc][owned]
	                                    *  Vector of pre-allocated operand representations. */
};

PRIVATE NONNULL((1)) void DCALL
assembly_formatter_fini(struct assembly_formatter *__restrict self) {
	DREF DeeStringObject **iter, **end;
	end = (iter = self->af_opreprv) + self->af_ast->a_assembly.as_opc;
	/* NOTE: Technically, representations are [1..1], the the caller uses this function
	 *       to cleanup partially constructed operand representations allocated through
	 *       calloc(). */
	for (; iter != end; ++iter)
		Dee_XDecref(*iter);
	Dee_Free(self->af_opreprv);
	ascii_printer_fini(&self->af_printer);
}




PRIVATE WUNUSED NONNULL((1, 2)) /*ref*/ struct TPPString *DCALL
assembly_formatter_format(struct assembly_formatter *__restrict self,
                          struct TPPString *__restrict input) {
#define print(p, s)                                                   \
	do {                                                              \
		if unlikely(ascii_printer_print(&self->af_printer, p, s) < 0) \
			goto err;                                                 \
	}	__WHILE0
#define printf(...)                                                           \
	do {                                                                      \
		if unlikely(ascii_printer_printf(&self->af_printer, __VA_ARGS__) < 0) \
			goto err;                                                         \
	}	__WHILE0
	char const *iter, *end, *flush_start;
	char ch;
	/*ref*/ struct TPPString *result;
	bool has_paren;
	end = (iter = flush_start = input->s_text) + input->s_size;
next:
	ASSERT(iter <= end);
	ch = *iter++;
	switch (ch) {

	case '\0':
		ASSERT(iter <= end + 1);
		if (iter == end + 1)
			break;
		goto next;

	case '%':
		/* Flush everything up to this point. */
		if (flush_start != iter - 1)
			print(flush_start, (size_t)(iter - flush_start) - 1);
		ch        = *iter++;
		has_paren = false;
		if (ch == '(') {
			/* To prevent ambiguity, we allow format options to be surrounded by (...) */
			has_paren = true;
			iter      = utf8_skipspace(iter, end);
			ch        = *iter++;
		}
		if (ch != '%') {
			size_t opno;
			char mod = 0;
			DeeStringObject *oprepr;
			if (!ch && iter == end)
				break;
			if (ch == '=') {
				/* Expand to a unique integer. */
				printf("%Iu", current_userasm.ua_asmuid);
				++current_userasm.ua_asmuid;
				goto done_special;
			}
			if (ch == 'l') {
				mod = ch;
				ch  = *iter++;
			}

			/* Determine the referenced operand. */
			if (ch == '[') {
				char const *name_start;
				struct TPPKeyword *name;
				struct asm_operand *op_iter, *op_end;
				iter = utf8_skipspace(iter, end);
				ch   = *iter++;
				if (!DeeUni_IsSymStrt(ch)) {
					DeeError_Throwf(&DeeError_CompilerError,
					                "Expected an identifier after `%[' in user-assembly text");
					goto err;
				}
				name_start = iter - 1;
				do {
					ch = *iter++;
				} while (DeeUni_IsSymCont(ch));
				/* Lookup the name of the operand. */
				name = TPPLexer_LookupKeyword(name_start, (size_t)(iter - name_start) - 1, 0);
				if unlikely(!name) {
err_unknown_operand:
					DeeError_Throwf(&DeeError_CompilerError,
					                "No operand known under the name %$q",
					                (size_t)(iter - name_start) - 1, name);
					goto err;
				}
				op_end = (op_iter = self->af_ast->a_assembly.as_opv) +
				         self->af_ast->a_assembly.as_opc;
				/* Search for an operand matching the given name. */
				for (opno = 0; op_iter != op_end; ++op_iter, ++opno) {
					if (op_iter->ao_name == name)
						goto has_operand; /* Found it! */
				}
				goto err_unknown_operand;
has_operand:
				iter = utf8_skipspace(iter - 1, end);
				ch   = *iter++;
				if (ch != ']') {
					DeeError_Throwf(&DeeError_CompilerError,
					                "Expected `]' after `%[' in user-assembly text");
					goto err;
				}
			} else {
				if (!DeeUni_IsDecimal(ch)) {
					DeeError_Throwf(&DeeError_CompilerError,
					                "Expected `[' or a digit after `%%' in user-assembly text");
					goto err;
				}
				opno = DeeUni_AsDigit(ch);
				ch   = *iter++;
				while (DeeUni_IsDecimal(ch)) {
					opno *= 10;
					opno += DeeUni_AsDigit(ch);
					ch = *iter++;
				}
				if unlikely(opno > self->af_ast->a_assembly.as_opc) {
					DeeError_Throwf(&DeeError_CompilerError,
					                "Expected `]' after `%[' in user-assembly text");
					goto err;
				}
				--iter;
			}
			ASSERT(opno <= self->af_ast->a_assembly.as_opc);
			/* Label operands must be prefixed with `l' */
			if ((opno >= (self->af_ast->a_assembly.as_num_i +
			              self->af_ast->a_assembly.as_num_o)) !=
			    (mod == 'l')) {

				DeeError_Throwf(&DeeError_CompilerError,
				                mod == 'l' ? "Only label operands may be prefixed by `l'"
				                           : "A label operand must be prefixed by `l'");
				goto err;
			}
			/* Load the representation used for this operand. */
			oprepr = self->af_opreprv[opno];
			/* Print the operand's representation. */
			print(DeeString_STR(oprepr), DeeString_SIZE(oprepr));
		}
done_special:
		if (has_paren) {
			iter = utf8_skipspace(iter, end);
			if (*iter != ')') {
				DeeError_Throwf(&DeeError_CompilerError,
				                "Expected `)' after `%(' in user-assembly text");
				goto err;
			}
			++iter;
		}
		flush_start = iter;
		goto next;

	default: goto next;
	}
	--iter;
	ASSERT(!*iter);
	if (flush_start != iter)
		print(flush_start, (size_t)(iter - flush_start));

	/* Special case: empty assembly text. */
	if (!self->af_printer.ap_length) {
		DeeObject_Free(self->af_printer.ap_string);
		self->af_printer.ap_string = NULL;
		return TPPString_NewEmpty();
	}

#ifdef CONFIG_OBJECT_HEAP_ISNT_MALLOC
	return TPPString_New(self->af_printer.ap_string->s_str,
	                     self->af_printer.ap_length);
#else /* CONFIG_OBJECT_HEAP_ISNT_MALLOC */
	/* Convert the string printer into a TPP-style string. */
	result = (struct TPPString *)self->af_printer.ap_string;
	__STATIC_IF(offsetof(struct TPPString, s_text) > offsetof(DeeStringObject, s_str)) {
		memmoveupc((void *)((uintptr_t)result + offsetof(struct TPPString, s_text)),
		           (void *)((uintptr_t)result + offsetof(DeeStringObject, s_str)),
		           self->af_printer.ap_length + 1, sizeof(char));
	}
	__STATIC_IF(offsetof(struct TPPString, s_text) < offsetof(DeeStringObject, s_str)) {
		memmovedownc((void *)((uintptr_t)result + offsetof(struct TPPString, s_text)),
		             (void *)((uintptr_t)result + offsetof(DeeStringObject, s_str)),
		             self->af_printer.ap_length + 1, sizeof(char));
	}
	result->s_refcnt = 1;
	result->s_size = self->af_printer.ap_length;
	result = (struct TPPString *)Dee_TryRealloc(result, offsetof(struct TPPString, s_text) +
	                                                    (self->af_printer.ap_length + 1) * sizeof(char));
	if unlikely(!result)
		result = (struct TPPString *)self->af_printer.ap_string;
	self->af_printer.ap_string = NULL;
	result->s_text[result->s_size] = '\0';
	return result;
#endif /* !CONFIG_OBJECT_HEAP_ISNT_MALLOC */
err:
	return NULL;
#undef printf
#undef print
}


INTERN WUNUSED NONNULL((1)) int DCALL
ast_genasm_userasm(struct ast *__restrict self) {
	struct assembler_state old_state;
	int result;
	/*ref*/ struct TPPString *assembly_text;
	/*ref*/ struct TPPFile *assembly_file, *old_eob;
	struct asm_operand *iter;
	size_t i, count;
	struct cleanup_mode *cleanup_actions = NULL, *cleanup_dst;
	uint32_t old_lexer_flags, old_lexer_tokens;
	/* Save the assembler state before user-assembly is processed. */
	old_state.as_handlerc = current_assembler.a_handlerc;
	old_state.as_stackcur = current_assembler.a_stackcur;
	ASSERT(self->a_type == AST_ASSEMBLY);
	ASSERT(self->a_assembly.as_text.at_text);
	ASSERT(self->a_assembly.as_text.at_text->s_refcnt);
	/* Keep track of operands that must be popped during cleanup. */
	if (self->a_assembly.as_num_o ||
	    self->a_assembly.as_num_i) {
		cleanup_actions = (struct cleanup_mode *)Dee_Calloc((self->a_assembly.as_num_o +
		                                                     self->a_assembly.as_num_i) *
		                                                    sizeof(struct cleanup_mode));
		if unlikely(!cleanup_actions)
			goto err;
	}

	if (self->a_flag & AST_FASSEMBLY_FORMAT) {
		struct assembly_formatter formatter;
		DREF DeeStringObject **dst;
		/* Setup the formatter for user-assembly text. */
		formatter.af_ast     = self;
		formatter.af_opreprv = (DREF DeeStringObject **)Dee_Calloc(self->a_assembly.as_opc *
		                                                           sizeof(DREF DeeStringObject *));
		if unlikely(!formatter.af_opreprv)
			goto err;
		ascii_printer_init(&formatter.af_printer);
		/* Generate text representations of assembly operands. */
		dst         = formatter.af_opreprv;
		iter        = self->a_assembly.as_opv;
		count       = self->a_assembly.as_num_o;
		cleanup_dst = cleanup_actions;
		/* Format output operands. */
		for (; count; --count, ++dst, ++iter, ++cleanup_dst) {
			*dst = (DREF DeeStringObject *)get_assembly_formatter_oprepr(iter->ao_expr,
			                                                             iter->ao_type->s_text,
			                                                             OPTION_MODE_UNDEF,
			                                                             cleanup_dst, &old_state);
			if unlikely(!*dst)
				goto err_formatter;
		}
		/* Format output operands. */
		count = self->a_assembly.as_num_i;
		for (; count; --count, ++dst, ++iter, ++cleanup_dst) {
			*dst = (DREF DeeStringObject *)get_assembly_formatter_oprepr(iter->ao_expr,
			                                                             iter->ao_type->s_text,
			                                                             OPTION_MODE_INPUT,
			                                                             cleanup_dst, &old_state);
			if unlikely(!*dst)
				goto err_formatter;
		}
		/* Format label operands. */
		count = self->a_assembly.as_num_l;
		for (i = 0; i < count; ++i, ++dst) {
			*dst = (DREF DeeStringObject *)get_label_repr(i);
			if unlikely(!*dst)
				goto err_formatter;
		}

		/* Format the input text to what will then be processed. */
		assembly_text = assembly_formatter_format(&formatter,
		                                          self->a_assembly.as_text.at_text);
		assembly_formatter_fini(&formatter);
		if unlikely(!assembly_text)
			goto err;
		goto create_assembly_file;
err_formatter:
		assembly_formatter_fini(&formatter);
		goto err;
	}
	/* Must still evaluate operands, even when not formatting the text.
	 * NOTE: Usually, there shouldn't be any operands, as their presence
	 *       requires formatting to be enabled by default, but the specs
	 *       don't state that this _must_ be the case, so we must still
	 *       evaluate all the operands. */
	cleanup_dst = cleanup_actions;
	iter        = self->a_assembly.as_opv;
	count       = self->a_assembly.as_num_o;
	/* Format output operands. */
	for (; count; --count, ++iter, ++cleanup_dst) {
		DREF DeeStringObject *temp;
		temp = (DREF DeeStringObject *)get_assembly_formatter_oprepr(iter->ao_expr,
		                                                             iter->ao_type->s_text,
		                                                             OPTION_MODE_UNDEF,
		                                                             cleanup_dst, &old_state);
		if unlikely(!temp)
			goto err;
		Dee_Decref(temp);
	}
	count = self->a_assembly.as_num_i;
	/* Format input operands. */
	for (; count; --count, ++iter, ++cleanup_dst) {
		DREF DeeStringObject *temp;
		temp = (DREF DeeStringObject *)get_assembly_formatter_oprepr(iter->ao_expr,
		                                                             iter->ao_type->s_text,
		                                                             OPTION_MODE_INPUT,
		                                                             cleanup_dst, &old_state);
		if unlikely(!temp)
			goto err;
		Dee_Decref(temp);
	}
	/* Emit debug information for user-assembly. */
	if (asm_putddi(self))
		goto err;

	assembly_text = self->a_assembly.as_text.at_text;
	TPPString_Incref(assembly_text);
create_assembly_file:
	assembly_file = TPPFile_NewExplicitInherited(assembly_text);
	if unlikely(!assembly_file)
		goto err_text;
	/* Push out assembly file. */
	TPPLexer_PushFileInherited(assembly_file);
	/* Configure the lexer so that it will not attempt to pop our file, or
	 * even try to read more data from it (considering it isn't a stream). */
	old_eob                      = TPPLexer_Current->l_eob_file;
	TPPLexer_Current->l_eob_file = assembly_file;

	/* Configure the lexer for assembly mode. */
	old_lexer_flags  = TPPLexer_Current->l_flags;
	old_lexer_tokens = TPPLexer_Current->l_extokens;
	TPPLexer_Current->l_flags &= (TPPLEXER_FLAG_MSVC_MESSAGEFORMAT |
	                              TPPLEXER_FLAG_MERGEMASK);
	TPPLexer_Current->l_flags |= (TPPLEXER_FLAG_WANTLF |
	                              TPPLEXER_FLAG_TERMINATE_STRING_LF |
	                              /*TPPLEXER_FLAG_NO_MACROS|
	                              TPPLEXER_FLAG_NO_DIRECTIVES|*/
	                              TPPLEXER_FLAG_ASM_COMMENTS);

	/* Enable the $-token, as well as C and C++ comments. */
	TPPLexer_Current->l_extokens = (TPPLEXER_TOKEN_DOLLAR |
	                                TPPLEXER_TOKEN_C_COMMENT |
	                                TPPLEXER_TOKEN_CPP_COMMENT);

	/* Reset various parts of the active lexer
	 * context, such as user-defined macros, etc.
	 * NOTE: Since the caller won't actually be using macros and the like
	 *       any more, we are safe to do this without concerns about other
	 *       parts of the compilation process (which are already done)
	 *       With that in mind, resetting all of that stuff here will
	 *       make it look like every user-assembly component is being
	 *       executed in a kind-of sub-space that is independent from 
	 *       all the other parts. */
	TPPLexer_Reset(TPPLexer_Current,
	               (TPPLEXER_RESET_ESTATE | TPPLEXER_RESET_ESTACK |
	                TPPLEXER_RESET_WSTATE | TPPLEXER_RESET_WSTACK |
	                TPPLEXER_RESET_MACRO | TPPLEXER_RESET_ASSERT |
	                TPPLEXER_RESET_KWDFLAGS | TPPLEXER_RESET_COUNTER |
	                TPPLEXER_RESET_FONCE));

	/* Clear out the last-written user-assembly instruction. */
	current_userasm.ua_lasti = ASM_DELOP;

	/* Actually parse user-assembly. */
	BEGIN_PARSER_CALLBACK();
	{
		struct asm_sec *old_section;
		/* Configure to use user-labels defined through operands. */
		uint16_t old_flags        = current_userasm.ua_flags;
		DeeScopeObject *old_scope = current_scope;
		/* Re-activate the scope of this branch. */
		current_scope = self->a_scope;
		ASSERT(current_basescope == current_scope->s_base);
		ASSERT(current_rootscope == current_basescope->bs_root);
		current_userasm.ua_flags = self->a_flag;
		/* Save the old current-section. */
		old_section = current_assembler.a_curr;
		{
			size_t old_user_label_c;
			struct asm_operand *old_user_label_v;
			old_user_label_c          = current_userasm.ua_labelc;
			old_user_label_v          = current_userasm.ua_labelv;
			current_userasm.ua_labelc = self->a_assembly.as_num_l;
			current_userasm.ua_labelv = self->a_assembly.as_opv +
			                            (self->a_assembly.as_num_i +
			                             self->a_assembly.as_num_o);
			parser_start();
			if unlikely(yield() < 0)
				result = -1;
			else {
				result = uasm_parse();
			}
			current_userasm.ua_labelc = old_user_label_c;
			current_userasm.ua_labelv = old_user_label_v;
		}
		/* Emit one last symbol to prevent peephole at the end
		 * of user-assembly when the `volatile' bit is set. */
		if ((current_userasm.ua_flags & AST_FASSEMBLY_VOLATILE) &&
		    (current_assembler.a_flag & ASM_FPEEPHOLE) && !result) {
			struct asm_sym *volatile_sym = asm_newsym();
			if unlikely(!volatile_sym)
				result = -1;
			asm_defsym(volatile_sym);
			++volatile_sym->as_used; /* Intentionally left dangling. */
		}
		if (!result &&
		    current_assembler.a_curr != old_section) {
			/* Generate a jump to the proper section. */
			struct asm_sym *temp = asm_newsym();
			if unlikely(!temp)
				result = -1;
			else {
				result                   = asm_gjmp(ASM_JMP, temp);
				current_assembler.a_curr = old_section;
				asm_defsym(temp);
			}
		}
		current_userasm.ua_flags = old_flags;
		current_scope            = old_scope;
	}
	if (parser_rethrow(result != 0))
		result = -1;
	END_PARSER_CALLBACK();

	/* Restore old lexer flags. */
	TPPLexer_Current->l_flags &= TPPLEXER_FLAG_MERGEMASK;
	TPPLexer_Current->l_flags |= old_lexer_flags;
	TPPLexer_Current->l_extokens = old_lexer_tokens;

	/* Pop all files leading up to our assembly file. */
	while (TPPLexer_GetFile() != assembly_file &&
	       TPPLexer_GetFile() != old_eob &&
	       TPPLexer_GetFile() != &TPPFile_Empty)
		TPPLexer_PopFile();
	/* Restore the old end-of-block file. */
	TPPLexer_Current->l_eob_file = old_eob;
	/* Pop our assembly file. */
	if (TPPLexer_GetFile() == assembly_file)
		TPPLexer_PopFile();

	/* Check for errors during processing of user-assembly. */
	if unlikely(result)
		goto err;

	/* Check if the assembler is still in an undefined state. */
	if (current_userasm.ua_mode & USER_ASM_FSTKINV) {
		DeeError_Throwf(&DeeError_CompilerError,
		                "User-assembly left the stack in an undefined state");
		goto err;
	}

	/* Pop operands according to `must_pop_ops' */
	count = (self->a_assembly.as_num_o +
	         self->a_assembly.as_num_i);
	while (count--) {
		struct ast *operand;
		switch (cleanup_actions[count].cm_kind) {

		case CLEANUP_MODE_FSTACK:
			operand = self->a_assembly.as_opv[count].ao_expr;
			if unlikely(current_assembler.a_stackcur <= old_state.as_stackcur) {
				/* The user broke stack alignment (just evaluate the operand). */
				if (self->a_assembly.as_opv[count].ao_name) {
					if (WARN(W_UASM_CANNOT_POP_ASSEMBLY_OUTPUT_EXPRESSION,
					         self->a_assembly.as_opv[count].ao_name->k_name))
						goto err;
				} else {
					char buffer[32];
					Dee_sprintf(buffer, "%%%Iu", (size_t)count);
					if (WARN(W_UASM_CANNOT_POP_ASSEMBLY_OUTPUT_EXPRESSION, buffer))
						goto err;
				}
				if (ast_genasm(operand, ASM_G_FNORMAL))
					goto err;
			} else {
				/* Pop the stack-top expression into this operand. */
				uint16_t expected = current_assembler.a_stackcur - 1;
				if (asm_gpop_expr(operand))
					goto err;
				/* Account of stack-slot re-use in stack displacement mode. */
				ASSERT(current_assembler.a_stackcur == expected ||
				       current_assembler.a_flag & ASM_FSTACKDISP);
				old_state.as_stackcur += current_assembler.a_stackcur - expected;
			}
			break;

		case CLEANUP_MODE_FLOCAL_POP: {
			struct symbol temp;
			operand = self->a_assembly.as_opv[count].ao_expr;
			INITIALIZE_FAKE_LOCAL_SYMBOL(&temp, cleanup_actions[count].cm_value);
			if (asm_gmov_ast_sym(operand, &temp, operand))
				goto err;
		}	ATTR_FALLTHROUGH
		case CLEANUP_MODE_FLOCAL:
			asm_dellocal(cleanup_actions[count].cm_value);
			break;

		default: break;
		}
	}

	/* Re-adjust the stack depth to what the caller expects. */
	ASSERT(old_state.as_handlerc == current_assembler.a_handlerc);
	if (old_state.as_stackcur != current_assembler.a_stackcur) {
		/* NOTE: Omit any warnings when `SP' was set as a clobber operand. */
		if (!(self->a_flag & AST_FASSEMBLY_CLOBSP)) {
			if (old_state.as_stackcur < current_assembler.a_stackcur) {
				if (WARN(W_UASM_DOESNT_CLEANUP_STACK,
				         (unsigned long)(current_assembler.a_stackcur - old_state.as_stackcur)))
					goto err;
			} else {
				if (WARN(W_UASM_POPPED_UNRELATED_ITEMS,
				         (unsigned long)(old_state.as_stackcur - current_assembler.a_stackcur)))
					goto err;
			}
		}
		if (asm_gsetstack(old_state.as_stackcur))
			goto err;
	}
	Dee_Free(cleanup_actions);
	return result;
err_text:
	TPPString_Decref(assembly_text);
err:
	Dee_Free(cleanup_actions);
	return -1;
}

#else /* !CONFIG_LANGUAGE_NO_ASM */

PRIVATE WUNUSED NONNULL((1)) int DCALL
compile_operator(struct asm_operand *__restrict op, bool is_output) {
	char const *format;
	struct ast *expr = op->ao_expr;
	ASSERT(op->ao_type);
	format = op->ao_type->s_text;
	if (is_output) {
		/* Since there is no portable way of providing a values, `=x', while
		 * having an explicitly defined meaning, can't actually be used, since
		 * there would be nothing to store into it.
		 * However, `+x' is portable, and has the meaning of `op = op', which
		 * may actually have side-effects for non-symbol operands. */
		if (strcmp(format, "+x") != 0) {
			if (*format != '+' && *format != '=')
				goto err_undefined_mode;
			goto err_illegal_mode;
		}
		if (expr->a_type == AST_SYM) {
			SYMBOL_INPLACE_UNWIND_ALIAS(expr->a_sym);
			if (expr->a_sym->s_type != SYMBOL_TYPE_GETSET)
				goto done;
		}
		if (ast_genasm(expr, ASM_G_FPUSHRES))
			goto err;
		if (asm_gpop_expr(expr))
			goto err;
	} else {
		if (strcmp(format, "x") != 0)
			goto err_illegal_mode;
		if (ast_genasm(expr, ASM_G_FNORMAL))
			goto err;
	}
done:
	return 0;
err:
	return -1;
err_illegal_mode:
	return DeeError_Throwf(&DeeError_CompilerError,
	                       "Illegal operand encoding %q",
	                       format);
err_undefined_mode:
	return DeeError_Throwf(&DeeError_CompilerError,
	                       "No operand mode has been defined by %q",
	                       format);
}

INTERN WUNUSED NONNULL((1)) int DCALL
ast_genasm_userasm(struct ast *__restrict self) {
	size_t i;
	ASSERT(self->a_type == AST_ASSEMBLY);
	/* Still compile assembly operands. */
	for (i = 0; i < self->a_assembly.as_num_o; ++i) {
		if (compile_operator(&self->a_assembly.as_opv[i], true))
			goto err;
	}
	for (i = 0; i < self->a_assembly.as_num_i; ++i) {
		if (compile_operator(&self->a_assembly.as_opv[self->a_assembly.as_num_o + i], false))
			goto err;
	}
	return 0;
err:
	return -1;
}
#endif /* !CONFIG_LANGUAGE_NO_ASM */

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_USERASM_C */
