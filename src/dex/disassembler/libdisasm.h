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
#ifndef GUARD_DEX_FS_LIBDISASM_H
#define GUARD_DEX_FS_LIBDISASM_H 1

#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/code.h>
#include <deemon/dex.h>
#include <deemon/object.h>

DECL_BEGIN

/* Special meanings for assembly text-bytes. */
#define TEXTBYTE_LABELCLASS_NORMAL       0x00 /* No special meaning. */
#define TEXTBYTE_LABELCLASS_EXCEPT_START 0x01 /* The starting address of an exception handler. - `tl_name' is the index of the exception handler. */
#define TEXTBYTE_LABELCLASS_EXCEPT_END   0x02 /* The ending address of an exception handler. - `tl_name' is the index of the exception handler. */
#define TEXTBYTE_LABELCLASS_EXCEPT_ENTRY 0x03 /* The entry point of an exception handler. - `tl_name' is the index of the exception handler. */
#define TEXTBYTE_LABELCLASS_JMP          0x04 /* The target of a `jmp', `jf', `jt' or `foreach' instruction.
                                               * `tl_name' is the address of the jump-like instruction. */
struct textlabel {
	code_addr_t tl_addr;   /* The absolute address of this text label. */
	uint8_t     tl_class;  /* The type of special text location. */
	uint8_t    _tl_pad[3]; /* ... */
	uint32_t    tl_name;   /* The name of this label (dependent on `tl_class') */
};

typedef struct {
	size_t            d_labelc; /* The number of special text labels. */
	struct textlabel *d_labelv; /* [0..d_labelc][owned] Vector of special text labels. */
} Disassembler;


/* Print the user-assembly-like mnemonic representation of a given instruction
 * NOTE: When the stack-depth is unknown, pass `(uint16_t)-1' for `stacksz'
 * @param: flags: Set of `PCODE_F*' (Only flags masked by `PCODE_FINSTRMASK' are recognized)
 */
INTDEF dssize_t DCALL
libdisasm_printinstr(dformatprinter printer, void *arg,
                     instruction_t *__restrict instr_start,
                     uint16_t stacksz, struct ddi_state *ddi_info,
                     DeeCodeObject *code, unsigned int flags);

INTDEF dssize_t DCALL
libdisasm_printlabel(dformatprinter printer, void *arg,
                     uint16_t opcode, code_addr_t source,
                     code_addr_t target);


/* Print assembly for `code', one instruction per line.
 * When non-NULL, prefix `line_prefix' infront of every
 * line, allowing the caller to specify an indentation. */
INTDEF dssize_t DCALL
libdisasm_printcode(dformatprinter printer, void *arg,
                    instruction_t *__restrict instr_start,
                    instruction_t *__restrict instr_end,
                    DeeCodeObject *code,
                    char const *line_prefix,
                    unsigned int flags);
#define PCODE_FNORMAL       0x0000 /* Normal code printing flags. */
#define PCODE_FDDI          0x0001 /* FLAG: Include DDI directives in output. */
#define PCODE_FNOEXCEPT     0x0002 /* FLAG: Do not include exception handler labels & directives. */
#define PCODE_FNOJUMPARROW  0x0004 /* FLAG: Do not draw arrows indicating the path of jumps. */
#define PCODE_FNOCOFLAGS    0x0008 /* FLAG: Do not include code-flag directives in output. */
#define PCODE_FINSTRMASK    0x00f0 /* MASK: Mask of flags recognized by `libdisasm_printinstr()'. */
#define PCODE_FNOBADCOMMENT 0x0010 /* FLAG: Do not include comments about invalid instructions/operands. */
#define PCODE_FNOARGCOMMENT 0x0020 /* FLAG: Do not include comments about the typing of operands. */
#define PCODE_FALTCOMMENT   0x0040 /* FLAG: Include comments about alternate operand representations. */
#define PCODE_FNOLABELS     0x0400 /* FLAG: Do not print labels, but use immediate addresses. */
#define PCODE_FNOINNER      0x0800 /* FLAG: Do not print the code of inner assembly recursively. */
#define PCODE_FNODEPTH      0x1000 /* FLAG: Do not include stack-depth information. */
#define PCODE_FNOSKIPDELOP  0x2000 /* FLAG: Do not omit ASM_DELOP from output. */
#define PCODE_FNOADDRESS    0x4000 /* FLAG: Do not print instruction addresses. */
#define PCODE_FNOBYTES      0x8000 /* FLAG: Do not include raw text bytes in output. */

DECL_END

#endif /* !GUARD_DEX_FS_LIBDISASM_H */
