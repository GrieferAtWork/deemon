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
#ifndef GUARD_DEX_FS_PRINTCODE_C
#define GUARD_DEX_FS_PRINTCODE_C 1
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/dex.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* strlen(), bzero(), memcpy(), ... */

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/debug-alignment.h>
#include <hybrid/minmax.h>
#include <hybrid/unaligned.h>

/**/
#include "libdisasm.h"
/**/

DECL_BEGIN

/* How many number of code bytes printed before mnemonics.
 * Remaining bytes are then printed in the following line. */
#define LINE_MAXBYTES 4

#if 0
#define DIRECTIVE_DEDENT_WIDTH 0 /* Doesn't really look that good... */
#else
#define DIRECTIVE_DEDENT_WIDTH 4
#endif


PRIVATE char const whitespace[] = "                                ";
PRIVATE char const question[] = "?" "?" "?" "?" "?" "?" "?";

#define INVOKE(expr)                   \
	do {                               \
		if unlikely((temp = expr) < 0) \
			goto err;                  \
		result += temp;                \
	}	__WHILE0
#define print(p, s) INVOKE((*printer)(arg, p, s))
#define PRINT(s)    INVOKE((*printer)(arg, s, COMPILER_STRLEN(s)))
#define printf(...) INVOKE(DeeFormat_Printf(printer, arg, __VA_ARGS__))

PRIVATE Dee_ssize_t DCALL
print_sp_transition(Dee_formatprinter_t printer, void *arg,
                    uint16_t old_sp, uint16_t new_sp,
                    unsigned int sp_width) {
	Dee_ssize_t temp, result = 0;
	if (old_sp == new_sp) {
		if (old_sp == (uint16_t)-1) {
			print(whitespace, (sp_width * 2) + 7);
		} else {
			printf("[%.*d]", sp_width, (unsigned int)old_sp);
			print(whitespace, sp_width + 5);
		}
	} else if (old_sp != (uint16_t)-1 && new_sp != (uint16_t)-1) {
		printf("[%.*d -> %.*d] ",
		       sp_width, (unsigned int)old_sp,
		       sp_width, (unsigned int)new_sp);
	} else if (old_sp == (uint16_t)-1) {
		PRINT("[");
		print(question, sp_width);
		printf(" -> %.*d] ", sp_width, (unsigned int)new_sp);
	} else {
		printf("[%.*d -> ", sp_width, (unsigned int)old_sp);
		print(question, sp_width);
		PRINT("] ");
	}
	return result;
err:
	return temp;
}

struct textjump {
	code_addr_t tj_origin; /* Origin instruction address. */
#define TEXTJUMP_ABSTRACT_TARGET ((code_addr_t)-1)
	code_addr_t tj_target; /* Target instruction address.
	                        * Set to `TEXTJUMP_ABSTRACT_TARGET' for abstract jump targets. */
	uint16_t    tj_level;  /* The display level of this jump. */
	uint16_t    tj_render; /* The number of times that this jump has been rendered. */
};

struct textjumps {
	size_t           tj_cnt; /* Actual number of text jumps */
	size_t           tj_alc; /* Allocated number of text jumps */
	struct textjump *tj_vec; /* [0..tj_cnt|ALLOC(tj_alc)][owned] Vector of textjumps. */
	size_t           tj_max; /* The max number of non-abstract overlapping jumps that are ever concurrently active. */
};

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
textjumps_istarget(struct textjumps const *__restrict self,
                   code_addr_t addr) {
	size_t i;
	for (i = 0; i < self->tj_cnt; ++i) {
		if (self->tj_vec[i].tj_target == addr)
			return true;
	}
	return false;
}

PRIVATE ATTR_NOINLINE int DCALL
textjumps_add(struct textjumps *__restrict self,
              code_addr_t origin, code_addr_t target) {
	size_t index;
	code_addr_t min_addr;
	ASSERT(self->tj_cnt <= self->tj_alc);
	if (self->tj_cnt == self->tj_alc) {
		struct textjump *newvec;
		size_t newalloc = self->tj_alc * 2;
		if (!newalloc)
			newalloc = 4;
		newvec = (struct textjump *)Dee_TryReallocc(self->tj_vec, newalloc,
		                                            sizeof(struct textjump));
		if unlikely(!newvec) {
			newalloc = self->tj_cnt + 1;
			newvec = (struct textjump *)Dee_Reallocc(self->tj_vec, newalloc,
			                                         sizeof(struct textjump));
			if unlikely(!newvec)
				goto err;
		}
		self->tj_vec = newvec;
		self->tj_alc = newalloc;
	}
	index    = self->tj_cnt;
	min_addr = MIN(origin, target);
	while (index && MIN(self->tj_vec[index - 1].tj_origin,
	                    self->tj_vec[index - 1].tj_target) > min_addr)
		--index;
	memmoveupc(self->tj_vec + index + 1,
	           self->tj_vec + index,
	           self->tj_cnt - index,
	           sizeof(struct textjump));
	self->tj_vec[index].tj_origin = origin;
	self->tj_vec[index].tj_target = target;
	self->tj_vec[index].tj_level  = 0;
	self->tj_vec[index].tj_render = 0;
	++self->tj_cnt;
	if (target != TEXTJUMP_ABSTRACT_TARGET) {
		if (!self->tj_max) {
			self->tj_max = 1;
		} else {
			code_addr_t addr_min = MIN(origin, target);
			code_addr_t addr_max = MAX(origin, target);
			uint8_t *used_levels;
			size_t i;
			used_levels = (uint8_t *)Dee_Calloca((self->tj_max + 7) / 8);
			if unlikely(!used_levels)
				goto err;
			for (i = 0; i < self->tj_cnt; ++i) {
				code_addr_t slot_min;
				code_addr_t slot_max;
				if (i == index)
					continue;
				if (self->tj_vec[i].tj_target == TEXTJUMP_ABSTRACT_TARGET)
					continue;
				slot_min = MIN(self->tj_vec[i].tj_origin, self->tj_vec[i].tj_target);
				slot_max = MAX(self->tj_vec[i].tj_origin, self->tj_vec[i].tj_target);
				if (slot_min <= addr_max && slot_max >= addr_min) {
					uint16_t level = self->tj_vec[i].tj_level;
					ASSERT(level < self->tj_max);
					used_levels[level / 8] |= 1 << (level % 8);
				}
			}
			for (i = 0; i < self->tj_max; ++i) {
				if (!(used_levels[i / 8] & (1 << (i % 8))))
					break;
			}
			Dee_Freea(used_levels);
			if (self->tj_max < i + 1)
				self->tj_max = i + 1;
			self->tj_vec[index].tj_level = (uint16_t)i;
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
textjumps_collect(struct textjumps *__restrict self,
                  instruction_t *__restrict instr_start,
                  instruction_t *__restrict instr_end,
                  instruction_t *__restrict start_addr) {
	instruction_t *iter_start;
	instruction_t *iter = instr_start;
fast_continue:
	for (; iter < instr_end; iter = DeeAsm_NextInstr(iter)) {
		uint16_t opcode;
		int32_t offset;
		iter_start = iter;
do_switch_on_iter:
		opcode = *iter;
do_switch_on_opcode:
		switch (opcode) {

		CASE_ASM_EXTENDED:
			opcode <<= 8;
			opcode |= iter[1];
			goto do_switch_on_opcode;

		case ASM_LOCAL:
		case ASM_GLOBAL:
		case ASM_STATIC:
		case ASM_STACK:
			iter += 2;
			goto do_switch_on_iter;

		case ASM_EXTERN:
			iter += 3;
			goto do_switch_on_iter;

		case ASM16_LOCAL:
		case ASM16_GLOBAL:
		case ASM16_STATIC:
		case ASM16_STACK:
			iter += 4;
			goto do_switch_on_iter;

		case ASM16_EXTERN:
			iter += 6;
			goto do_switch_on_iter;

		case ASM_JF:
		case ASM_JT:
		case ASM_JMP:
		case ASM_FOREACH:
		case ASM_FOREACH_KEY:
		case ASM_FOREACH_VALUE:
		case ASM_FOREACH_PAIR:
			offset = (int8_t)UNALIGNED_GETLE8(iter + 1);
do_relative_jump:
			iter = DeeAsm_NextInstr(iter);
			if (textjumps_add(self, (code_addr_t)(iter_start - start_addr),
			                  (code_addr_t)(iter - start_addr) + offset))
				goto err;
			goto fast_continue;

		case ASM_JF16:
		case ASM_JT16:
		case ASM_JMP16:
		case ASM_FOREACH16:
		case ASM_FOREACH_KEY16:
		case ASM_FOREACH_VALUE16:
		case ASM_FOREACH_PAIR16:
			offset = (int16_t)UNALIGNED_GETLE16(iter + 1);
			goto do_relative_jump;

		case ASM32_JMP:
			offset = (int32_t)UNALIGNED_GETLE32(iter + 2);
			goto do_relative_jump;

		case ASM_JMP_POP:
		case ASM_JMP_POP_POP:
			if (textjumps_add(self, (code_addr_t)(iter - start_addr),
			                  TEXTJUMP_ABSTRACT_TARGET))
				goto err;
			break;

		default: break;
		}
	}
	return 0;
err:
	return -1;
}


#undef MIRROR_LINES
#define MIRROR_LINES 1


#if 1
#define CORNER_TOP     0x6c
#define LINE_VERT      0x78
#define CORNER_BOTTOM  0x6d
#define LINE_HORI      0x71

#if 0
#define ARROW_LEFT     '<'
#define ARROW_RIGHT    '>'
#define ARROW_UP       '^'
#define ARROW_DOWN     'v'
#endif

PRIVATE unsigned char const corner_top[]    = { 0xe2, 0x94, 0x8c };
PRIVATE unsigned char const line_vert[]     = { 0xe2, 0x94, 0x82 };
PRIVATE unsigned char const corner_bottom[] = { 0xe2, 0x94, 0x94 };
PRIVATE unsigned char const line_hori[]     = { 0xe2, 0x94, 0x80 };

#ifdef ARROW_LEFT
PRIVATE unsigned char const arrow_left[]  = { 0xe2, 0x86, 0x90 };
PRIVATE unsigned char const arrow_up[]    = { 0xe2, 0x86, 0x91 };
PRIVATE unsigned char const arrow_right[] = { 0xe2, 0x86, 0x92 };
PRIVATE unsigned char const arrow_down[]  = { 0xe2, 0x86, 0x93 };

//PRIVATE unsigned char const arrow_left[]  = { 0xf0, 0x9f, 0xa2, 0x80 };
//PRIVATE unsigned char const arrow_up[]    = { 0xf0, 0x9f, 0xa2, 0x81 };
//PRIVATE unsigned char const arrow_right[] = { 0xf0, 0x9f, 0xa2, 0x82 };
//PRIVATE unsigned char const arrow_down[]  = { 0xf0, 0x9f, 0xa2, 0x83 };
#endif /* ARROW_LEFT */

#define HAVE_PRINT_BOX 1
PRIVATE Dee_ssize_t DCALL
print_box(Dee_formatprinter_t printer, void *arg,
          unsigned char *__restrict text, size_t length) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	for (i = 0; i < length; ++i) {
		switch (text[i]) {

		case CORNER_TOP:
			temp = (*printer)(arg, (char *)corner_top, COMPILER_LENOF(corner_top));
			break;

		case LINE_VERT:
			temp = (*printer)(arg, (char *)line_vert, COMPILER_LENOF(line_vert));
			break;

		case CORNER_BOTTOM:
			temp = (*printer)(arg, (char *)corner_bottom, COMPILER_LENOF(corner_bottom));
			break;

		case LINE_HORI:
			temp = (*printer)(arg, (char *)line_hori, COMPILER_LENOF(line_hori));
			break;

#ifdef ARROW_LEFT
		case ARROW_LEFT:
			temp = (*printer)(arg, (char *)arrow_left, COMPILER_LENOF(arrow_left));
			break;

		case ARROW_RIGHT:
			temp = (*printer)(arg, (char *)arrow_right, COMPILER_LENOF(arrow_right));
			break;

		case ARROW_UP:
			temp = (*printer)(arg, (char *)arrow_up, COMPILER_LENOF(arrow_up));
			break;

		case ARROW_DOWN:
			temp = (*printer)(arg, (char *)arrow_down, COMPILER_LENOF(arrow_down));
			break;
#endif /* ARROW_LEFT */

		default:
			temp = (*printer)(arg, (char *)&text[i], 1);
			break;
		}
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}

#elif 0
#define CORNER_TOP    '/'
#define LINE_VERT     '|'
#define CORNER_BOTTOM '\\'
#define LINE_HORI     '-'
#else
#define CORNER_TOP    '+'
#define LINE_VERT     '|'
#define CORNER_BOTTOM '+'
#define LINE_HORI     '-'
#endif

#ifndef ARROW_LEFT
#define ARROW_LEFT    '<'
#define ARROW_RIGHT   '>'
#define ARROW_UP      '^'
#define ARROW_DOWN    'v'
#endif /* !ARROW_LEFT */


PRIVATE Dee_ssize_t DCALL
textjumps_print(Dee_formatprinter_t printer, void *arg,
                struct textjumps *__restrict self,
                code_addr_t curr_uip, code_addr_t next_uip) {
	Dee_ssize_t temp, result = 0;
	size_t i, line_length;
	bool has_origin          = false;
	bool has_target          = false;
	bool has_abstract_target = false;
	unsigned char *lines;
	if (!self->tj_cnt)
		return 0;
	line_length = self->tj_max ? self->tj_max * 2 - 1 : 0;
#if DIRECTIVE_DEDENT_WIDTH == 0
	lines = (unsigned char *)Dee_Mallocac(line_length + 4, sizeof(unsigned char));
#else /* DIRECTIVE_DEDENT_WIDTH == 0 */
	lines = (unsigned char *)Dee_Mallocac(line_length + 3, sizeof(unsigned char));
#endif /* DIRECTIVE_DEDENT_WIDTH != 0 */
	if unlikely(!lines)
		goto err;
	memset(lines, ' ', line_length * sizeof(unsigned char));
	for (i = 0; i < self->tj_cnt; ++i) {
		code_addr_t jmp_min;
		code_addr_t jmp_max;
		bool is_origin = false;
		bool is_target = false;
		unsigned char *line;
		bool is_downdir;
		if (self->tj_vec[i].tj_target == TEXTJUMP_ABSTRACT_TARGET) {
			if (curr_uip <= self->tj_vec[i].tj_origin &&
			    next_uip > self->tj_vec[i].tj_origin)
				has_origin = has_abstract_target = true;
			continue;
		}
		if (curr_uip <= self->tj_vec[i].tj_origin &&
		    next_uip > self->tj_vec[i].tj_origin)
			has_origin = is_origin = true;
		if (curr_uip <= self->tj_vec[i].tj_target &&
		    next_uip > self->tj_vec[i].tj_target)
			has_target = is_target = true;
		jmp_min = MIN(self->tj_vec[i].tj_origin, self->tj_vec[i].tj_target);
		jmp_max = MAX(self->tj_vec[i].tj_origin, self->tj_vec[i].tj_target);
		if (!(curr_uip <= jmp_max && next_uip > jmp_min))
			continue; /* No intersection */
		ASSERT(self->tj_vec[i].tj_level < self->tj_max);
#ifdef MIRROR_LINES
		line = &lines[((self->tj_max - 1) - self->tj_vec[i].tj_level) * 2];
#else /* MIRROR_LINES */
		line = &lines[self->tj_vec[i].tj_level * 2];
#endif /* !MIRROR_LINES */
		is_downdir = self->tj_vec[i].tj_target > self->tj_vec[i].tj_origin;
		if (is_origin) {
			*line = is_downdir ? CORNER_TOP : CORNER_BOTTOM;
		} else if (is_target) {
			*line = is_downdir ? CORNER_BOTTOM : CORNER_TOP;
		} else {
			*line = LINE_VERT;
			if ((self->tj_vec[i].tj_render & 7) == 7)
				*line = is_downdir ? ARROW_DOWN : ARROW_UP;
		}
		++self->tj_vec[i].tj_render;
	}
	i = 0;
	for (; i < line_length; ++i) {
		if (lines[i] != CORNER_TOP &&
		    lines[i] != CORNER_BOTTOM)
			continue;
		/* Replace all whitespace with `-' */
		++i;
		while (i < line_length) {
			if (lines[i] == ' ')
				lines[i] = LINE_HORI;
			/* Force the use of `|' here, so-as to keep
			 * intersecting lines as clear as possible. */
			if (lines[i] == ARROW_UP || lines[i] == ARROW_DOWN)
				lines[i] = LINE_VERT;
			++i;
		}
		break;
	}
	if (has_origin && has_target) {
		lines[line_length + 0] = LINE_HORI;
		lines[line_length + 1] = ARROW_LEFT;
		lines[line_length + 2] = ARROW_RIGHT;
	} else if (has_origin) {
		lines[line_length + 0] = has_abstract_target ? ' ' : LINE_HORI;
		lines[line_length + 1] = has_abstract_target ? '?' : LINE_HORI;
		lines[line_length + 2] = ARROW_LEFT;
	} else if (has_target) {
		lines[line_length + 0] = LINE_HORI;
		lines[line_length + 1] = LINE_HORI;
		lines[line_length + 2] = ARROW_RIGHT;
	} else {
		lines[line_length + 0] = ' ';
		lines[line_length + 1] = ' ';
		lines[line_length + 2] = ' ';
	}
#if DIRECTIVE_DEDENT_WIDTH == 0
	lines[line_length + 3] = ' ';
	line_length += 4;
#else /* DIRECTIVE_DEDENT_WIDTH == 0 */
	line_length += 3;
#endif /* DIRECTIVE_DEDENT_WIDTH != 0 */
#ifdef HAVE_PRINT_BOX
	temp = print_box(printer, arg, lines, line_length);
	if unlikely(temp < 0)
		goto fail_with_temp_err_lines;
	result += temp;
#else /* HAVE_PRINT_BOX */
	print((char *)lines, line_length);
#endif /* !HAVE_PRINT_BOX */
done:
	Dee_Freea(lines);
	return result;
fail_with_temp_err_lines:
	result = temp;
	goto done;
err:
	return -1;
}


struct typeflag {
	char     name[14];
	uint16_t mask;
};

PRIVATE struct typeflag const typeflags[] = {
	{ "final", TP_FFINAL },
	{ "truncate", TP_FTRUNCATE },
	{ "interrupt", TP_FINTERRUPT },
	{ "moveany", TP_FMOVEANY },
	{ "inheritctor", TP_FINHERITCTOR },
};

struct attributeflag {
	char     name[14];
	uint16_t mask;
};

PRIVATE struct attributeflag const attributeflags[] = {
	{ "private", CLASS_ATTRIBUTE_FPRIVATE },
	{ "final", CLASS_ATTRIBUTE_FFINAL },
	{ "readonly", CLASS_ATTRIBUTE_FREADONLY },
	{ "method", CLASS_ATTRIBUTE_FMETHOD },
	{ "getset", CLASS_ATTRIBUTE_FGETSET },
	{ "classmem", CLASS_ATTRIBUTE_FCLASSMEM },
};

INTERN Dee_ssize_t DCALL
libdisasm_printclassattribute(Dee_formatprinter_t printer, void *arg,
                              struct class_attribute *__restrict self,
                              char const *line_prefix, uint16_t addr_size) {
	Dee_ssize_t temp, result;
	unsigned int i;
	result = DeeFormat_Printf(printer, arg,
	                          "%s        attribute %k, %" PRFu16,
	                          line_prefix, self->ca_name,
	                          self->ca_addr);
	if unlikely(result < 0)
		goto done;
	for (i = 0; i < COMPILER_LENOF(attributeflags); ++i) {
		if (!(self->ca_flag & attributeflags[i].mask))
			continue;
		INVOKE(DeeFormat_Printf(printer, arg, ", @%s", attributeflags[i].name));
	}
	if (self->ca_addr >= addr_size)
		INVOKE(DeeFormat_PRINT(printer, arg, " /* Invalid address */"));
	if (self->ca_flag & ~CLASS_ATTRIBUTE_FMASK)
		INVOKE(DeeFormat_Printf(printer, arg, " /* Invalid flags %#" PRFx16 " */", self->ca_flag));
	INVOKE((*printer)(arg, "\n", 1));
done:
	return result;
err:
	return temp;
}

/* Return the S-name (e.g. `add') of an operator.
 * Returns `NULL' when the name cannot be determined. */
INTERN WUNUSED char const *DCALL
libdisasm_get_operator_sname(Dee_operator_t operator_id) {
	char const *result;
	struct opinfo const *info;
	info = DeeTypeType_GetOperatorById(&DeeType_Type, operator_id);
	if (info) {
		result = info->oi_sname;
	} else {
		result = NULL;
		switch (operator_id) {
		case CLASS_OPERATOR_SUPERARGS:
			result = "superargs";
			break;
		case CLASS_OPERATOR_PRINT:
			result = "print";
			break;
		case CLASS_OPERATOR_PRINTREPR:
			result = "printrepr";
			break;
		default: break;
		}
	}
	return result;
}


INTERN Dee_ssize_t DCALL
libdisasm_printclass(Dee_formatprinter_t printer, void *arg,
                     DeeClassDescriptorObject *__restrict self,
                     size_t const_index, char const *line_prefix) {
	Dee_ssize_t temp, result;
	size_t i;
	if (!line_prefix)
		line_prefix = "";
	result = DeeFormat_Printf(printer, arg,
	                          "%s.const %" PRFuSIZ " = class %k%s{\n"
	                          "%s    /* INSTANCESIZE = %" PRFu16 " */\n"
	                          "%s    /* CLASSSIZE    = %" PRFu16 " */\n",
	                          line_prefix, const_index,
	                          self->cd_name ? self->cd_name : (DeeStringObject *)Dee_EmptyString,
	                          self->cd_name ? " " : "",
	                          line_prefix, self->cd_imemb_size,
	                          line_prefix, self->cd_cmemb_size);
	if unlikely(result < 0)
		goto done;
	for (i = 0; i < COMPILER_LENOF(typeflags); ++i) {
		if (!(self->cd_flags & typeflags[i].mask))
			continue;
		INVOKE(DeeFormat_Printf(printer, arg,
		                        "%s    @%s\n",
		                        line_prefix,
		                        typeflags[i].name));
	}
	{
		bool has_operators = false;
		for (i = 0; i <= self->cd_clsop_mask; ++i) {
			Dee_operator_t op_name = self->cd_clsop_list[i].co_name;
			char const *op_name_str;
			if (op_name == (Dee_operator_t)-1)
				continue;
			if (!has_operators) {
				INVOKE(DeeFormat_Printf(printer, arg, "%s    operators = {\n", line_prefix));
				has_operators = true;
			}
			op_name_str = libdisasm_get_operator_sname(op_name);
			INVOKE(op_name_str ? DeeFormat_Printf(printer, arg, "%s        %s = %" PRFu16 "\n",
			                                      line_prefix, op_name_str, self->cd_clsop_list[i].co_addr)
			                   : DeeFormat_Printf(printer, arg, "%s        %#" PRFx16 " = %" PRFu16 "\n",
			                                      line_prefix, op_name, self->cd_clsop_list[i].co_addr));
		}
		if (has_operators)
			INVOKE(DeeFormat_Printf(printer, arg, "%s    }\n", line_prefix));
	}
	{
		bool has_class_attributes = false;
		for (i = 0; i <= self->cd_cattr_mask; ++i) {
			struct class_attribute *attr;
			attr = &self->cd_cattr_list[i];
			if (!attr->ca_name)
				continue;
			if (!has_class_attributes) {
				INVOKE(DeeFormat_Printf(printer, arg, "%s    class_attributes = {\n", line_prefix));
				has_class_attributes = true;
			}
			INVOKE(libdisasm_printclassattribute(printer, arg, attr, line_prefix, self->cd_cmemb_size));
		}
		if (has_class_attributes)
			INVOKE(DeeFormat_Printf(printer, arg, "%s    }\n", line_prefix));
	}
	{
		bool has_instance_attributes = false;
		for (i = 0; i <= self->cd_iattr_mask; ++i) {
			struct class_attribute *attr;
			attr = &self->cd_iattr_list[i];
			if (!attr->ca_name)
				continue;
			if (!has_instance_attributes) {
				INVOKE(DeeFormat_Printf(printer, arg, "%s    instance_attributes = {\n", line_prefix));
				has_instance_attributes = true;
			}
			INVOKE(libdisasm_printclassattribute(printer, arg, attr, line_prefix,
			                                     attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM
			                                     ? self->cd_cmemb_size
			                                     : self->cd_imemb_size));
		}
		if (has_instance_attributes)
			INVOKE(DeeFormat_Printf(printer, arg, "%s    }\n", line_prefix));
	}
	INVOKE(DeeFormat_Printf(printer, arg, "%s}\n", line_prefix));
done:
	return result;
err:
	return temp;
}

struct codeflag {
	uint16_t cf_flag;
	char     cf_name[14];
};

PRIVATE struct codeflag const codeflag_names[] = {
	{ CODE_FYIELDING,    "yielding" },
	{ CODE_FCOPYABLE,    "copyable" },
	{ CODE_FASSEMBLY,    "assembly" },
	{ CODE_FLENIENT,     "lenient" },
	{ CODE_FVARARGS,     "varargs" },
	{ CODE_FVARKWDS,     "varkwds" },
	{ CODE_FTHISCALL,    "thiscall" },
	{ CODE_FHEAPFRAME,   "heapframe" },
	{ CODE_FFINALLY,     "finally" },
	{ CODE_FCONSTRUCTOR, "constructor" }
};
#define CODEFLAG_NAMEDMASK                              \
	(CODE_FYIELDING | CODE_FCOPYABLE | CODE_FASSEMBLY | \
	 CODE_FLENIENT | CODE_FVARARGS | CODE_FVARKWDS |    \
	 CODE_FTHISCALL | CODE_FHEAPFRAME | CODE_FFINALLY | \
	 CODE_FCONSTRUCTOR)


INTERN WUNUSED NONNULL((1, 3, 4)) Dee_ssize_t DCALL
libdisasm_printcode(Dee_formatprinter_t printer, void *arg,
                    instruction_t *instr_start,
                    instruction_t *instr_end,
                    DeeCodeObject *code,
                    char const *line_prefix,
                    unsigned int flags) {
	struct textjumps jumps = { 0, 0, NULL };
	Dee_ssize_t temp, result = 0;
	uint16_t stacksz = 0, new_stacksz;
	instruction_t *iter, *next;
	instruction_t *start_addr = code ? code->co_code : instr_start;
	size_t prefix_len = line_prefix ? strlen(line_prefix) : 0;
	uint16_t code_flags = code ? code->co_flags : 0;
	struct ddi_state ddi;
	uint8_t *ddi_ip = DDI_NEXT_DONE;
	struct ddi_regs last_print_ddi;
	unsigned int sp_width = 1;
	if ((flags & (PCODE_FNOJUMPARROW | PCODE_FNOLABELS)) !=
	    (PCODE_FNOJUMPARROW | PCODE_FNOLABELS) &&
	    textjumps_collect(&jumps, instr_start, instr_end, start_addr))
		goto err_n1;
	memset(&last_print_ddi, 0xff, sizeof(last_print_ddi));
	if (code) {
		uint16_t stack_max;
		if ((ddi_ip = Dee_ddi_state_init(&ddi, (DeeObject *)code, DDI_STATE_FNORMAL)) == DDI_NEXT_ERR)
			goto err_n1;
		stack_max = (uint16_t)DeeCode_StackDepth(code);
		if (stack_max >= 10000) {
			sp_width = 5;
		} else if (stack_max >= 1000) {
			sp_width = 4;
		} else if (stack_max >= 100) {
			sp_width = 3;
		} else if (stack_max >= 10) {
			sp_width = 2;
		}
	} else {
		bzero(&ddi, sizeof(ddi));
	}

	if ((code_flags & CODEFLAG_NAMEDMASK) &&
	    !(flags & PCODE_FNOCOFLAGS)) {
		uint16_t named_flags = code_flags & CODEFLAG_NAMEDMASK;
		uint16_t i;
		if (prefix_len)
			print(line_prefix, prefix_len);
		if (!(flags & PCODE_FNOADDRESS))
			print(whitespace, 7);
		if (!(flags & PCODE_FNOBYTES))
			print(whitespace, LINE_MAXBYTES * 3);
		if (!(flags & PCODE_FNODEPTH))
			print(whitespace, (sp_width * 2) + 7);
		if (!(flags & PCODE_FNOJUMPARROW) && jumps.tj_max) {
			size_t line_length;
			line_length = jumps.tj_max * 2 - 1;
#if DIRECTIVE_DEDENT_WIDTH == 0
			line_length += 4;
#else /* DIRECTIVE_DEDENT_WIDTH == 0 */
			line_length += 3;
#endif /* DIRECTIVE_DEDENT_WIDTH != 0 */
			temp = DeeFormat_Repeat(printer, arg, ' ', line_length);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		PRINT(".code @");
		for (i = 0; i < COMPILER_LENOF(codeflag_names); ++i) {
			if (!(named_flags & codeflag_names[i].cf_flag))
				continue;
			print(codeflag_names[i].cf_name, strlen(codeflag_names[i].cf_name));
			named_flags &= ~codeflag_names[i].cf_flag;
			if (!named_flags)
				break;
			PRINT(", @");
		}
		PRINT("\n");
	}

	for (iter = instr_start;
	     iter < instr_end;
	     iter = next, stacksz = new_stacksz) {
		code_addr_t code_ip = (code_addr_t)(iter - start_addr);
		while ((ddi_ip != DDI_NEXT_DONE) && ddi.rs_regs.dr_uip < code_ip) {
			ddi_ip = Dee_ddi_next_state(ddi_ip, &ddi, DDI_STATE_FNORMAL);
			if unlikely(ddi_ip == DDI_NEXT_ERR)
				goto err_n1;
		}
		if (ddi_ip && ddi.rs_regs.dr_uip == code_ip) {
			if (flags & PCODE_FDDI) {
				bool is_first = true;
				if (prefix_len)
					print(line_prefix, prefix_len);
				if (!(flags & PCODE_FNOADDRESS))
					print(whitespace, 7);
				if (!(flags & PCODE_FNOBYTES))
					print(whitespace, LINE_MAXBYTES * 3);
				if (!(flags & PCODE_FNODEPTH))
					print(whitespace, (sp_width * 2) + 7);
				if (!(flags & PCODE_FNOJUMPARROW)) {
					temp = textjumps_print(printer, arg, &jumps, code_ip, code_ip);
					if unlikely(temp < 0)
						goto err;
					result += temp;
				}
				PRINT(".ddi ");
				if (last_print_ddi.dr_name != ddi.rs_regs.dr_name) {
					char *name = DeeCode_GetDDIString((DeeObject *)code,
					                                  ddi.rs_regs.dr_name);
					if (name) {
						printf("@name(%q)", name);
					} else {
						PRINT("@name(none)");
					}
					is_first = false;
				}
				/* TODO: DDI commands for local/stack name bindings. */
				if (last_print_ddi.dr_path != ddi.rs_regs.dr_path ||
				    last_print_ddi.dr_file != ddi.rs_regs.dr_file) {
					char const *path = NULL, *file;
					if (ddi.rs_regs.dr_path != 0)
						path = DeeCode_GetDDIString((DeeObject *)code, ddi.rs_regs.dr_path - 1);
					file = DeeCode_GetDDIString((DeeObject *)code, ddi.rs_regs.dr_file);
					if (!is_first)
						PRINT(", ");
					printf("\"%#q%s%#q\"", path ? path : "", path ? "/" : "", file ? file : "");
					is_first = false;
				}
				if (!is_first)
					PRINT(", ");
				printf("%d", ddi.rs_regs.dr_lno + 1);
				if (last_print_ddi.dr_col != ddi.rs_regs.dr_col)
					printf(", %d", ddi.rs_regs.dr_col + 1);
				PRINT("\n");
				memcpy(&last_print_ddi, &ddi.rs_regs, sizeof(last_print_ddi));
			}
			stacksz = ddi.rs_regs.dr_usp;
		}
		if (stacksz == (uint16_t)-1) {
			/* TODO: Make use of textjumps to mirror the stack-depth of the jump
			 *       origin if the current instruction is the target of a a jump.
			 * >> 00AC   10 0B       [1 -> 0] |               +-|─|--<    jf     pop, 00B9
			 * >> 00AE   3B 0D       [0 -> 1] |               | | |       push   @"()"
			 * >> 00B0   FF 00 74    [1 -> 0] |               | | |       add    local 0, pop
			 * >> 00B3   FF 01 3F 05 [0]      |               | | |       mov    local 1, local 5
			 * >> 00B7   14 3F       [0 -> ?] |             +-|-|-|--<    jmp    00F8
			 * >> 00B9   3F 01                |             | +-+-|-->    push   local 1 // We can re-use `00AC.SP - 1' as SP for this instruction.
			 * >> 00BB   3B 09                |             |     v       push   @"("
			 */
		}

		new_stacksz = stacksz;
		if (new_stacksz == (uint16_t)-1) {
get_next_instruction_without_stack:
			next = DeeAsm_NextInstr(iter);
		} else {
			uint16_t opcode = *iter;
			if (ASM_ISEXTENDED(opcode))
				opcode = (opcode << 8) | iter[1];
			if (DeeAsm_IsNoreturn(opcode, code_flags)) {
				new_stacksz = (uint16_t)-1;
				goto get_next_instruction_without_stack;
			}
			next = DeeAsm_NextInstrSp(iter, &new_stacksz);
		}
		/* Print jump labels */
		if (!(flags & PCODE_FNOLABELS) && code) {
			size_t i;
			code_addr_t instr_min = (code_addr_t)(iter - start_addr);
			code_addr_t instr_max = (code_addr_t)(next - start_addr);
			for (i = 0; i < jumps.tj_cnt; ++i) {
				struct textjump *jmp;
				jmp = &jumps.tj_vec[i];
				if (jmp->tj_target < instr_max && jmp->tj_target >= instr_min) {
					instruction_t *orig_pc;
					uint16_t opcode;
					orig_pc = code->co_code + jmp->tj_origin;
					orig_pc = DeeAsm_SkipPrefix(orig_pc); /* Jump instruction can use prefixes */
					opcode  = *orig_pc;
					if (ASM_ISEXTENDED(opcode))
						opcode = (opcode << 8) | orig_pc[1];
					if (prefix_len)
						print(line_prefix, prefix_len);
					if (!(flags & PCODE_FNOADDRESS))
						print(whitespace, 7);
					if (!(flags & PCODE_FNOBYTES))
						print(whitespace, LINE_MAXBYTES * 3);
					if (!(flags & PCODE_FNODEPTH))
						print(whitespace, (sp_width * 2) + 7);
					if (!(flags & PCODE_FNOJUMPARROW)) {
						temp = textjumps_print(printer, arg, &jumps, code_ip, code_ip);
						if unlikely(temp < 0)
							goto err;
						result += temp;
					}
					temp = libdisasm_printlabel(printer, arg, opcode,
					                            jmp->tj_origin,
					                            jmp->tj_target);
					if unlikely(temp < 0)
						goto err;
					result += temp;
					PRINT(":\n");
				}
			}
		}
		/* Print exception labels & directives. */
#if 1
		if (!(flags & PCODE_FNOEXCEPT) && code) {
			uint16_t i;
			code_addr_t instr_min = (code_addr_t)(iter - start_addr);
			code_addr_t instr_max = (code_addr_t)(next - start_addr);
			for (i = 0; i < code->co_exceptc; ++i) {
				size_t j;
				for (j = 0; j < 3; ++j) {
					struct except_handler *hand = &code->co_exceptv[i];
					code_addr_t hip             = (&hand->eh_start)[j];
					if (hip < instr_max && hip >= instr_min) {
						PRIVATE char const except_type[3][6] = { "start", "end", "entry" };
						PRIVATE char const except_name[2][8] = { "except", "finally" };
#if EXCEPTION_HANDLER_FFINALLY == 1
						char const *name = except_name[hand->eh_flags & EXCEPTION_HANDLER_FFINALLY];
#else /* EXCEPTION_HANDLER_FFINALLY == 1 */
						char const *name = except_name[hand->eh_flags & EXCEPTION_HANDLER_FFINALLY ? 1 : 0];
#endif /* EXCEPTION_HANDLER_FFINALLY != 1 */
						char const *type = except_type[j];
						/* Found an overlap! */
prefix_except_prefix:
						if (prefix_len)
							print(line_prefix, prefix_len);
						if (!(flags & PCODE_FNOADDRESS))
							print(whitespace, 7);
						if (!(flags & PCODE_FNOBYTES))
							print(whitespace, LINE_MAXBYTES * 3);
						if (!(flags & PCODE_FNODEPTH))
							print(whitespace, (sp_width * 2) + 7);
						if (!(flags & PCODE_FNOJUMPARROW)) {
							temp = textjumps_print(printer, arg, &jumps, code_ip, code_ip);
							if unlikely(temp < 0)
								goto err;
							result += temp;
						}
						if (j == 2) {
							/* Entry */
							if (stacksz == (uint16_t)-1) {
								/* Make use of handler stack information */
								new_stacksz = stacksz = hand->eh_stack;
								DeeAsm_NextInstrSp(iter, &new_stacksz);
							}
							printf(".except .L%s_%" PRFu16 "_start, .L%s_%" PRFu16 "_end, .L%s_%" PRFu16 "_entry", name, i, name, i, name, i);
							if (hand->eh_flags & EXCEPTION_HANDLER_FFINALLY)
								PRINT(", @finally");
							if (hand->eh_flags & EXCEPTION_HANDLER_FINTERPT)
								PRINT(", @interrupt");
							if (hand->eh_flags & EXCEPTION_HANDLER_FHANDLED)
								PRINT(", @handled");
							if (hand->eh_mask)
								printf(", @mask(%k)", hand->eh_mask);
							PRINT("\n");
							j = 3; /* Prevent recursion. */
							goto prefix_except_prefix;
						}
						/* start / end */
						printf(".L%s_%" PRFu16 "_%s:\n", name, i, type);
					}
				}
			}
		}
#endif

		/* Skip DELOP opcodes if they aren't jump targets. */
		if (!(flags & PCODE_FNOSKIPDELOP) && *iter == ASM_DELOP) {
			if (flags & PCODE_FNOJUMPARROW)
				continue;
			if (!textjumps_istarget(&jumps, code_ip))
				continue;
		}

		if (prefix_len)
			print(line_prefix, prefix_len);

		/* Print the instruction address. */
		if (!(flags & PCODE_FNOADDRESS))
			printf("%.4I32X   ", (code_addr_t)(iter - start_addr));

		/* Print instruction bytes. */
		if (!(flags & PCODE_FNOBYTES)) {
			char bytes[INSTRLEN_MAX * 3];
			size_t i, num_bytes;
			num_bytes = (size_t)(next - iter);
			if unlikely(num_bytes > INSTRLEN_MAX)
				num_bytes = INSTRLEN_MAX;
			for (i = 0; i < num_bytes; ++i) {
				uint8_t byte = iter[i];
				bytes[(i * 3) + 0] = DeeAscii_ItoaUpperDigit(byte >> 4);
				bytes[(i * 3) + 1] = DeeAscii_ItoaUpperDigit(byte & 0xf);
				bytes[(i * 3) + 2] = ' ';
			}
			i = MIN(num_bytes, (size_t)LINE_MAXBYTES);
			print(bytes, i * 3);
			num_bytes -= i;
			i = LINE_MAXBYTES - i;
			while (i--)
				PRINT("   ");
			if (!(flags & PCODE_FNODEPTH)) {
				temp = print_sp_transition(printer, arg, stacksz, new_stacksz, sp_width);
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
			if (!(flags & PCODE_FNOJUMPARROW)) {
				temp = textjumps_print(printer, arg, &jumps, code_ip,
				                       (code_addr_t)(next - start_addr));
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
#if DIRECTIVE_DEDENT_WIDTH != 0
			if ((flags & (PCODE_FDDI | PCODE_FNOEXCEPT)) !=
			    (PCODE_FNOEXCEPT))
				print(whitespace, DIRECTIVE_DEDENT_WIDTH);
#endif /* DIRECTIVE_DEDENT_WIDTH != 0 */
			/* Print the actual instruction. */
			temp = libdisasm_printinstr(printer, arg, iter, stacksz,
			                            ddi_ip ? &ddi : NULL,
			                            code, flags);
			if unlikely(temp < 0)
				goto err;
			result += temp;
			PRINT("\n");
#if INSTRLEN_MAX > LINE_MAXBYTES * 2
			{
				size_t offset = LINE_MAXBYTES;
				while (num_bytes) {
					size_t line_bytes = num_bytes;
					if (line_bytes > LINE_MAXBYTES)
						line_bytes = LINE_MAXBYTES;
					if (prefix_len)
						print(line_prefix, prefix_len);
					if (!(flags & PCODE_FNOADDRESS))
						print(whitespace, 7);
					if (flags & PCODE_FNOJUMPARROW) {
						bytes[offset * 3 + (line_bytes * 3) - 1] = '\n';
						print(bytes + offset * 3, line_bytes * 3);
					} else {
						print(bytes + offset * 3, line_bytes * 3);
						if (LINE_MAXBYTES != line_bytes)
							print(whitespace, (LINE_MAXBYTES - line_bytes) * 3);
						if (!(flags & PCODE_FNODEPTH))
							print(whitespace, (sp_width * 2) + 7);
						temp = textjumps_print(printer, arg, &jumps,
						                       (code_addr_t)(next - start_addr),
						                       (code_addr_t)(next - start_addr));
						if unlikely(temp < 0)
							goto err;
						result += temp;
						PRINT("\n");
					}
					num_bytes -= line_bytes;
					offset += line_bytes;
				}
			}
#else /* INSTRLEN_MAX > LINE_MAXBYTES*2 */
			if (num_bytes) {
				if (prefix_len)
					print(line_prefix, prefix_len);
				if (!(flags & PCODE_FNOADDRESS))
					print(whitespace, 7);
				if (flags & PCODE_FNOJUMPARROW) {
					bytes[LINE_MAXBYTES * 3 + (num_bytes * 3) - 1] = '\n';
					print(bytes + LINE_MAXBYTES * 3, num_bytes * 3);
				} else {
					ASSERT(LINE_MAXBYTES >= num_bytes);
					print(bytes + LINE_MAXBYTES * 3, num_bytes * 3);
					if (LINE_MAXBYTES != num_bytes)
						print(whitespace, (LINE_MAXBYTES - num_bytes) * 3);
					if (!(flags & PCODE_FNODEPTH))
						print(whitespace, (sp_width * 2) + 7);
					temp = textjumps_print(printer, arg, &jumps,
					                       (code_addr_t)(next - start_addr),
					                       (code_addr_t)(next - start_addr));
					if unlikely(temp < 0)
						goto err;
					result += temp;
					PRINT("\n");
				}
			}
#endif /* INSTRLEN_MAX < LINE_MAXBYTES*2 */
		} else {
			if (!(flags & PCODE_FNODEPTH)) {
				temp = print_sp_transition(printer, arg, stacksz, new_stacksz, sp_width);
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
			if (!(flags & PCODE_FNOJUMPARROW)) {
				temp = textjumps_print(printer, arg, &jumps, code_ip,
				                       (code_addr_t)(next - start_addr));
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
#if DIRECTIVE_DEDENT_WIDTH != 0
			if ((flags & (PCODE_FDDI | PCODE_FNOEXCEPT)) !=
			    (PCODE_FNOEXCEPT))
				print(whitespace, DIRECTIVE_DEDENT_WIDTH);
#endif /* DIRECTIVE_DEDENT_WIDTH != 0 */
			/* Print the actual instruction. */
			temp = libdisasm_printinstr(printer, arg, iter, stacksz,
			                            ddi_ip ? &ddi : NULL,
			                            code, flags);
			if unlikely(temp < 0)
				goto err;
			result += temp;
			PRINT("\n");
		}
	}
	if (!(flags & PCODE_FNOINNER) && code) {
		size_t i;
		for (i = 0; i < code->co_constc; ++i) {
			DREF DeeCodeObject *inner_code;
			char const *kind = "code";
			inner_code       = (DREF DeeCodeObject *)code->co_constv[i];
			if (!DeeCode_Check(inner_code)) {
				if (!DeeFunction_Check(inner_code)) {
					if (DeeClassDescriptor_Check(inner_code)) {
						Dee_Incref(inner_code);
						temp = libdisasm_printclass(printer, arg,
						                            (DeeClassDescriptorObject *)inner_code,
						                            i, line_prefix);
						Dee_Decref(inner_code);
						if unlikely(temp < 0)
							goto err;
						result += temp;
					}
					continue;
				}
				inner_code = ((DREF DeeFunctionObject *)inner_code)->fo_code;
				kind       = "function";
			}
			Dee_Incref(inner_code);
			temp = DeeFormat_Printf(printer, arg,
			                        "%s.const %" PRFuSIZ " = %s {\n",
			                        line_prefix ? line_prefix : "",
			                        i,
			                        kind);
			if likely(temp >= 0) {
				char *inner_prefix;
				result += temp;
				inner_prefix = (char *)Dee_Mallocc(prefix_len + 5, sizeof(char));
				if unlikely(!inner_prefix) {
					temp = -1;
				} else {
					memset(inner_prefix, ' ', (prefix_len + 4) * sizeof(char));
					inner_prefix[prefix_len + 4] = '\0';
					temp = libdisasm_printcode(printer, arg,
					                           inner_code->co_code,
					                           inner_code->co_code + inner_code->co_codebytes,
					                           inner_code,
					                           inner_prefix,
					                           flags);
					DBG_ALIGNMENT_ENABLE();
					if likely(temp >= 0) {
						temp = DeeFormat_Printf(printer, arg, "%s}\n", line_prefix ? line_prefix : "");
						if likely(temp >= 0)
							result += temp;
					}
					Dee_Free(inner_prefix);
				}
			}
			Dee_Decref(inner_code);
			if unlikely(temp < 0)
				goto err;
		}
	}
done:
	if (code)
		Dee_ddi_state_fini(&ddi);
	Dee_Free(jumps.tj_vec);
	DBG_ALIGNMENT_DISABLE();
	return result;
err:
	result = temp;
	goto done;
err_n1:
	result = -1;
	goto done;
}


DECL_END

#endif /* !GUARD_DEX_FS_PRINTCODE_C */
