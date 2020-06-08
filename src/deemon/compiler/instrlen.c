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
#ifndef GUARD_DEEMON_COMPILER_INSTRLEN_C
#define GUARD_DEEMON_COMPILER_INSTRLEN_C 1

#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>

#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/unaligned.h>

DECL_BEGIN

#if 1
#define STACK_EFFECT_UNDEF     0x00
#else
#define STACK_EFFECT_UNDEF     0xff
#endif
#define STACK_EFFECT(down, up) ((down) << 4 | (up))
#define STACK_EFFECT_UP(x)     ((x)&0xf)
#define STACK_EFFECT_DOWN(x)   (((x)&0xf0) >> 4)
#define STACK_EFFECT_SUM(x)    (STACK_EFFECT_UP(x) - STACK_EFFECT_DOWN(x))

/* Length of basic instructions */
/*[[[deemon
#include <file>
#include <util>
#include <fs>
local codes = list([none] * 256);
local codes_f0 = list([none] * 256);
fs::chdir(fs::path::head(__FILE__));
local longest_name = 0;

for (local l: file.open("../../../include/deemon/asm.h")) {
	local name, code, length, misc;
	try {
		name, code, length, misc = l.scanf(" # define ASM%[^ ] 0x%[0-9a-fA-F] /" "* [ %[^ \\]] ] %[^]")...;
	} catch (...) {
		continue;
	}
	local updown, desc;
	try {
		updown, desc = misc.scanf(" [ %[^\\]] ] ` %[^\'] '")...;
	} catch (...) {
		desc = "---";
		try {
			updown = misc.scanf(" [ %[^\\]] ]")...;
		} catch (...) {
			updown = ",";
		}
	}
	code = (int)("0x" + code);
	local data = pack(name, length, updown, desc);
	if (code < 256) {
		codes[code] = data;
	} else {
		if (code >= 0xf000 && code <= 0xf0ff) {
			codes_f0[code - 0xf000] = data;
		}
	}
	if (longest_name < #name)
		longest_name = #name;
}

function print_codes(codes, zero_f0, unknown_length) {
	for (local id, data: util::enumerate(codes)) {
			if (data is none) {
				local length = unknown_length;
				if (id >= 0xf0 && zero_f0)
					length = 0;
				print ("\t/" "* 0x%.2I8x *" "/" % id) + " " + length + ", /" "* --- *" "/";
			} else {
				local name, length, none, desc = data...;
				print ("\t/" "* 0x%.2I8x *" "/" % id) + " " + length + ", /" "* `ASM" + name + "':" +
				      (" " * (longest_name - #name))+" `"+desc+"' *" "/";
			}
	}
}

function print_stack_effect(codes) {
	for (local id, data: util::enumerate(codes)) {
			if (data is none) {
				print ("\t/" "* 0x%.2I8x *" "/" % id) + " STACK_EFFECT_UNDEF, /" "* --- *" "/";
			} else {
				local name, none, updown, desc = data...;
				if (updown == "-n,+n+1")
					updown = "-0,+1"; // Cheat a bit...
				if (updown == "-n-1,+n")
					updown = "-1,+0"; // Cheat a bit...

				if ("n" in updown || "?" in updown) {
					print ("\t/" "* 0x%.2I8x *" "/" % id) + " STACK_EFFECT_UNDEF, /" "* `ASM" + name + "':" +
					       (" " * (longest_name - #name)) + " `" + desc + "' *" "/";
				} else {
					local down, up = updown.split(",")...;
					down = down.strip(" -");
					up   = up.strip(" +");
					if ("|" in up)
						up = up.partition("|")[0];
					print ("\t/" "* 0x%.2I8x *" "/" % id) + " STACK_EFFECT(" + down + ", " + up + "), /" "* `ASM" + name + "':" +
							(" " * (longest_name - #name)) + " `" + desc + "' *" "/";
				}
			}
	}
}
print "PRIVATE uint8_t const intr_len[256] = {";
print_codes(codes, true, 1);
print "};";
print "";

print "PRIVATE uint8_t const intr_len_f0[256] = {";
print_codes(codes_f0, false, 2);
print "};";
print "";

print "PRIVATE uint8_t const stack_effect[256] = {";
print_stack_effect(codes);
print "};";

print "";
print "PRIVATE uint8_t const stack_effect_f0[256] = {";
print_stack_effect(codes_f0);
print "};";
]]]*/
PRIVATE uint8_t const intr_len[256] = {
	/* 0x00 */ 1, /* `ASM_RET_NONE':                `ret' */
	/* 0x01 */ 1, /* `ASM_YIELD':                   `yield pop' */
	/* 0x02 */ 1, /* `ASM_YIELDALL':                `yield foreach, pop' */
	/* 0x03 */ 1, /* `ASM_THROW':                   `throw pop' */
	/* 0x04 */ 1, /* `ASM_RETHROW':                 `throw except' */
	/* 0x05 */ 1, /* `ASM_SETRET':                  `setret pop' */
	/* 0x06 */ 1, /* `ASM_ENDCATCH':                `end catch' */
	/* 0x07 */ 1, /* `ASM_ENDFINALLY':              `end finally' */
	/* 0x08 */ 3, /* `ASM_CALL_KW':                 `call top, #<imm8>, const <imm8>' */
	/* 0x09 */ 2, /* `ASM_CALL_TUPLE_KW':           `call top, pop..., const <imm8>' */
	/* 0x0a */ 1, /* --- */
	/* 0x0b */ 2, /* `ASM_PUSH_BND_ARG':            `push bound arg <imm8>' */
	/* 0x0c */ 3, /* `ASM_PUSH_BND_EXTERN':         `push bound extern <imm8>:<imm8>' */
	/* 0x0d */ 1, /* --- */
	/* 0x0e */ 2, /* `ASM_PUSH_BND_GLOBAL':         `push bound global <imm8>' */
	/* 0x0f */ 2, /* `ASM_PUSH_BND_LOCAL':          `push bound local <imm8>' */
	/* 0x10 */ 2, /* `ASM_JF':                      `jf pop, <Sdisp8>' */
	/* 0x11 */ 3, /* `ASM_JF16':                    `jf pop, <Sdisp16>' */
	/* 0x12 */ 2, /* `ASM_JT':                      `jt pop, <Sdisp8>' */
	/* 0x13 */ 3, /* `ASM_JT16':                    `jt pop, <Sdisp16>' */
	/* 0x14 */ 2, /* `ASM_JMP':                     `jmp <Sdisp8>' */
	/* 0x15 */ 3, /* `ASM_JMP16':                   `jmp <Sdisp16>' */
	/* 0x16 */ 2, /* `ASM_FOREACH':                 `foreach top, <Sdisp8>' */
	/* 0x17 */ 3, /* `ASM_FOREACH16':               `foreach top, <Sdisp16>' */
	/* 0x18 */ 1, /* `ASM_JMP_POP':                 `jmp pop' */
	/* 0x19 */ 3, /* `ASM_OPERATOR':                `op top, $<imm8>, #<imm8>' */
	/* 0x1a */ 2, /* `ASM_OPERATOR_TUPLE':          `op top, $<imm8>, pop...' */
	/* 0x1b */ 2, /* `ASM_CALL':                    `call top, #<imm8>' */
	/* 0x1c */ 1, /* `ASM_CALL_TUPLE':              `call top, pop...' */
	/* 0x1d */ 1, /* --- */
	/* 0x1e */ 2, /* `ASM_DEL_GLOBAL':              `del global <imm8>' */
	/* 0x1f */ 2, /* `ASM_DEL_LOCAL':               `del local <imm8>' */
	/* 0x20 */ 1, /* `ASM_SWAP':                    `swap' */
	/* 0x21 */ 2, /* `ASM_LROT':                    `lrot #<imm8>+3' */
	/* 0x22 */ 2, /* `ASM_RROT':                    `rrot #<imm8>+3' */
	/* 0x23 */ 1, /* `ASM_DUP':                     `dup' */
	/* 0x24 */ 2, /* `ASM_DUP_N':                   `dup #SP - <imm8> - 2' */
	/* 0x25 */ 1, /* `ASM_POP':                     `pop' */
	/* 0x26 */ 2, /* `ASM_POP_N':                   `pop #SP - <imm8> - 2' */
	/* 0x27 */ 2, /* `ASM_ADJSTACK':                `adjstack #SP +/- <Simm8>' */
	/* 0x28 */ 1, /* `ASM_SUPER':                   `super top, pop' */
	/* 0x29 */ 2, /* `ASM_SUPER_THIS_R':            `push super this, ref <imm8>' */
	/* 0x2a */ 1, /* --- */
	/* 0x2b */ 2, /* `ASM_POP_STATIC':              `pop static <imm8>' */
	/* 0x2c */ 3, /* `ASM_POP_EXTERN':              `pop extern <imm8>:<imm8>' */
	/* 0x2d */ 1, /* --- */
	/* 0x2e */ 2, /* `ASM_POP_GLOBAL':              `pop global <imm8>' */
	/* 0x2f */ 2, /* `ASM_POP_LOCAL':               `pop local <imm8>' */
	/* 0x30 */ 1, /* --- */
	/* 0x31 */ 1, /* --- */
	/* 0x32 */ 1, /* --- */
	/* 0x33 */ 1, /* `ASM_PUSH_NONE':               `push none' */
	/* 0x34 */ 1, /* `ASM_PUSH_VARARGS':            `push varargs' */
	/* 0x35 */ 1, /* `ASM_PUSH_VARKWDS':            `push varkwds' */
	/* 0x36 */ 2, /* `ASM_PUSH_MODULE':             `push module <imm8>' */
	/* 0x37 */ 2, /* `ASM_PUSH_ARG':                `push arg <imm8>' */
	/* 0x38 */ 2, /* `ASM_PUSH_CONST':              `push const <imm8>' */
	/* 0x39 */ 2, /* `ASM_PUSH_REF':                `push ref <imm8>' */
	/* 0x3a */ 1, /* --- */
	/* 0x3b */ 2, /* `ASM_PUSH_STATIC':             `push static <imm8>' */
	/* 0x3c */ 3, /* `ASM_PUSH_EXTERN':             `push extern <imm8>:<imm8>' */
	/* 0x3d */ 1, /* --- */
	/* 0x3e */ 2, /* `ASM_PUSH_GLOBAL':             `push global <imm8>' */
	/* 0x3f */ 2, /* `ASM_PUSH_LOCAL':              `push local <imm8>' */
	/* 0x40 */ 1, /* `ASM_CAST_TUPLE':              `cast top, Tuple' */
	/* 0x41 */ 1, /* `ASM_CAST_LIST':               `cast top, List' */
	/* 0x42 */ 2, /* `ASM_PACK_TUPLE':              `push pack Tuple, #<imm8>' */
	/* 0x43 */ 2, /* `ASM_PACK_LIST':               `push pack List, #<imm8>' */
	/* 0x44 */ 1, /* --- */
	/* 0x45 */ 1, /* --- */
	/* 0x46 */ 2, /* `ASM_UNPACK':                  `unpack pop, #<imm8>' */
	/* 0x47 */ 1, /* `ASM_CONCAT':                  `concat top, pop' */
	/* 0x48 */ 2, /* `ASM_EXTEND':                  `extend top, #<imm8>' */
	/* 0x49 */ 1, /* `ASM_TYPEOF':                  `typeof top' */
	/* 0x4a */ 1, /* `ASM_CLASSOF':                 `classof top' */
	/* 0x4b */ 1, /* `ASM_SUPEROF':                 `superof top' */
	/* 0x4c */ 1, /* `ASM_INSTANCEOF':              `instanceof top, pop' */
	/* 0x4d */ 1, /* `ASM_STR':                     `str top' */
	/* 0x4e */ 1, /* `ASM_REPR':                    `repr top' */
	/* 0x4f */ 1, /* --- */
	/* 0x50 */ 1, /* `ASM_BOOL':                    `bool top' */
	/* 0x51 */ 1, /* `ASM_NOT':                     `not top' */
	/* 0x52 */ 1, /* `ASM_ASSIGN':                  `assign pop, pop' */
	/* 0x53 */ 1, /* `ASM_MOVE_ASSIGN':             `assign move, pop, pop' */
	/* 0x54 */ 1, /* `ASM_COPY':                    `copy top' */
	/* 0x55 */ 1, /* `ASM_DEEPCOPY':                `deepcopy top' */
	/* 0x56 */ 1, /* `ASM_GETATTR':                 `getattr top, pop' */
	/* 0x57 */ 1, /* `ASM_DELATTR':                 `delattr pop, pop' */
	/* 0x58 */ 1, /* `ASM_SETATTR':                 `setattr pop, pop, pop' */
	/* 0x59 */ 1, /* `ASM_BOUNDATTR':               `boundattr top, pop' */
	/* 0x5a */ 2, /* `ASM_GETATTR_C':               `getattr top, const <imm8>' */
	/* 0x5b */ 2, /* `ASM_DELATTR_C':               `delattr pop, const <imm8>' */
	/* 0x5c */ 2, /* `ASM_SETATTR_C':               `setattr pop, const <imm8>, pop' */
	/* 0x5d */ 2, /* `ASM_GETATTR_THIS_C':          `push getattr this, const <imm8>' */
	/* 0x5e */ 2, /* `ASM_DELATTR_THIS_C':          `delattr this, const <imm8>' */
	/* 0x5f */ 2, /* `ASM_SETATTR_THIS_C':          `setattr this, const <imm8>, pop' */
	/* 0x60 */ 1, /* `ASM_CMP_EQ':                  `cmp eq, top, pop' */
	/* 0x61 */ 1, /* `ASM_CMP_NE':                  `cmp ne, top, pop' */
	/* 0x62 */ 1, /* `ASM_CMP_GE':                  `cmp ge, top, pop' */
	/* 0x63 */ 1, /* `ASM_CMP_LO':                  `cmp lo, top, pop' */
	/* 0x64 */ 1, /* `ASM_CMP_LE':                  `cmp le, top, pop' */
	/* 0x65 */ 1, /* `ASM_CMP_GR':                  `cmp gr, top, pop' */
	/* 0x66 */ 2, /* `ASM_CLASS_C':                 `class top, const <imm8>' */
	/* 0x67 */ 3, /* `ASM_CLASS_GC':                `push class global <imm8>, const <imm8>' */
	/* 0x68 */ 4, /* `ASM_CLASS_EC':                `push class extern <imm8>:<imm8>, const <imm8>' */
	/* 0x69 */ 2, /* `ASM_DEFCMEMBER':              `defcmember top, $<imm8>, pop' */
	/* 0x6a */ 3, /* `ASM_GETCMEMBER_R':            `push getcmember ref <imm8>, $<imm8>' */
	/* 0x6b */ 4, /* `ASM_CALLCMEMBER_THIS_R':      `push callcmember this, ref <imm8>, $<imm8>, #<imm8>' */
	/* 0x6c */ 1, /* --- */
	/* 0x6d */ 1, /* --- */
	/* 0x6e */ 3, /* `ASM_FUNCTION_C':              `push function const <imm8>, #<imm8>+1' */
	/* 0x6f */ 4, /* `ASM_FUNCTION_C_16':           `push function const <imm8>, #<imm16>+1' */
	/* 0x70 */ 1, /* `ASM_CAST_INT':                `cast top, int' */
	/* 0x71 */ 1, /* `ASM_INV':                     `inv top' */
	/* 0x72 */ 1, /* `ASM_POS':                     `pos top' */
	/* 0x73 */ 1, /* `ASM_NEG':                     `neg top' */
	/* 0x74 */ 1, /* `ASM_ADD':                     `add top, pop' */
	/* 0x75 */ 1, /* `ASM_SUB':                     `sub top, pop' */
	/* 0x76 */ 1, /* `ASM_MUL':                     `mul top, pop' */
	/* 0x77 */ 1, /* `ASM_DIV':                     `div top, pop' */
	/* 0x78 */ 1, /* `ASM_MOD':                     `mod top, pop' */
	/* 0x79 */ 1, /* `ASM_SHL':                     `shl top, pop' */
	/* 0x7a */ 1, /* `ASM_SHR':                     `shr top, pop' */
	/* 0x7b */ 1, /* `ASM_AND':                     `and top, pop' */
	/* 0x7c */ 1, /* `ASM_OR':                      `or top, pop' */
	/* 0x7d */ 1, /* `ASM_XOR':                     `xor top, pop' */
	/* 0x7e */ 1, /* `ASM_POW':                     `pow top, pop' */
	/* 0x7f */ 1, /* `ASM_INC':                     `inc' */
	/* 0x80 */ 1, /* `ASM_DEC':                     `dec' */
	/* 0x81 */ 2, /* `ASM_ADD_SIMM8':               `add top, $<Simm8>' */
	/* 0x82 */ 5, /* `ASM_ADD_IMM32':               `add top, $<imm32>' */
	/* 0x83 */ 2, /* `ASM_SUB_SIMM8':               `sub top, $<Simm8>' */
	/* 0x84 */ 5, /* `ASM_SUB_IMM32':               `sub top, $<imm32>' */
	/* 0x85 */ 2, /* `ASM_MUL_SIMM8':               `mul top, $<Simm8>' */
	/* 0x86 */ 2, /* `ASM_DIV_SIMM8':               `div top, $<Simm8>' */
	/* 0x87 */ 2, /* `ASM_MOD_SIMM8':               `mod top, $<Simm8>' */
	/* 0x88 */ 2, /* `ASM_SHL_IMM8':                `shl top, $<imm8>' */
	/* 0x89 */ 2, /* `ASM_SHR_IMM8':                `shr top, $<imm8>' */
	/* 0x8a */ 5, /* `ASM_AND_IMM32':               `and top, $<imm32>' */
	/* 0x8b */ 5, /* `ASM_OR_IMM32':                `or top, $<imm32>' */
	/* 0x8c */ 5, /* `ASM_XOR_IMM32':               `xor top, $<imm32>' */
	/* 0x8d */ 1, /* `ASM_ISNONE':                  `instanceof top, none' */
	/* 0x8e */ 1, /* --- */
	/* 0x8f */ 1, /* `ASM_DELOP':                   `---' */
	/* 0x90 */ 1, /* `ASM_NOP':                     `nop' */
	/* 0x91 */ 1, /* `ASM_PRINT':                   `print pop' */
	/* 0x92 */ 1, /* `ASM_PRINT_SP':                `print pop, sp' */
	/* 0x93 */ 1, /* `ASM_PRINT_NL':                `print pop, nl' */
	/* 0x94 */ 1, /* `ASM_PRINTNL':                 `print nl' */
	/* 0x95 */ 1, /* `ASM_PRINTALL':                `print pop...' */
	/* 0x96 */ 1, /* `ASM_PRINTALL_SP':             `print pop..., sp' */
	/* 0x97 */ 1, /* `ASM_PRINTALL_NL':             `print pop..., nl' */
	/* 0x98 */ 1, /* --- */
	/* 0x99 */ 1, /* `ASM_FPRINT':                  `print top, pop' */
	/* 0x9a */ 1, /* `ASM_FPRINT_SP':               `print top, pop, sp' */
	/* 0x9b */ 1, /* `ASM_FPRINT_NL':               `print top, pop, nl' */
	/* 0x9c */ 1, /* `ASM_FPRINTNL':                `print top, nl' */
	/* 0x9d */ 1, /* `ASM_FPRINTALL':               `print top, pop...' */
	/* 0x9e */ 1, /* `ASM_FPRINTALL_SP':            `print top, pop..., sp' */
	/* 0x9f */ 1, /* `ASM_FPRINTALL_NL':            `print top, pop..., nl' */
	/* 0xa0 */ 1, /* --- */
	/* 0xa1 */ 2, /* `ASM_PRINT_C':                 `print const <imm8>' */
	/* 0xa2 */ 2, /* `ASM_PRINT_C_SP':              `print const <imm8>, sp' */
	/* 0xa3 */ 2, /* `ASM_PRINT_C_NL':              `print const <imm8>, nl' */
	/* 0xa4 */ 3, /* `ASM_RANGE_0_I16':             `push range $0, $<imm16>' */
	/* 0xa5 */ 1, /* --- */
	/* 0xa6 */ 1, /* `ASM_ENTER':                   `enter top' */
	/* 0xa7 */ 1, /* `ASM_LEAVE':                   `leave pop' */
	/* 0xa8 */ 1, /* --- */
	/* 0xa9 */ 2, /* `ASM_FPRINT_C':                `print top, const <imm8>' */
	/* 0xaa */ 2, /* `ASM_FPRINT_C_SP':             `print top, const <imm8>, sp' */
	/* 0xab */ 2, /* `ASM_FPRINT_C_NL':             `print top, const <imm8>, nl' */
	/* 0xac */ 1, /* `ASM_RANGE':                   `range top, pop' */
	/* 0xad */ 1, /* `ASM_RANGE_DEF':               `push range default, pop' */
	/* 0xae */ 1, /* `ASM_RANGE_STEP':              `range top, pop, pop' */
	/* 0xaf */ 1, /* `ASM_RANGE_STEP_DEF':          `push range default, pop, pop' */
	/* 0xb0 */ 1, /* `ASM_CONTAINS':                `contains top, pop' */
	/* 0xb1 */ 2, /* `ASM_CONTAINS_C':              `push contains const <imm8>, pop' */
	/* 0xb2 */ 1, /* `ASM_GETITEM':                 `getitem top, pop' */
	/* 0xb3 */ 3, /* `ASM_GETITEM_I':               `getitem top, $<Simm16>' */
	/* 0xb4 */ 2, /* `ASM_GETITEM_C':               `getitem top, const <imm8>' */
	/* 0xb5 */ 1, /* `ASM_GETSIZE':                 `getsize top' */
	/* 0xb6 */ 1, /* `ASM_SETITEM':                 `setitem pop, pop, pop' */
	/* 0xb7 */ 3, /* `ASM_SETITEM_I':               `setitem pop, $<Simm16>, pop' */
	/* 0xb8 */ 2, /* `ASM_SETITEM_C':               `setitem pop, const <imm8>, pop' */
	/* 0xb9 */ 1, /* `ASM_ITERSELF':                `iterself top' */
	/* 0xba */ 1, /* `ASM_DELITEM':                 `delitem pop, pop' */
	/* 0xbb */ 1, /* `ASM_GETRANGE':                `getrange top, pop, pop' */
	/* 0xbc */ 1, /* `ASM_GETRANGE_PN':             `getrange top, pop, none' */
	/* 0xbd */ 1, /* `ASM_GETRANGE_NP':             `getrange top, none, pop' */
	/* 0xbe */ 3, /* `ASM_GETRANGE_PI':             `getrange top, pop, $<Simm16>' */
	/* 0xbf */ 3, /* `ASM_GETRANGE_IP':             `getrange top, $<Simm16>, pop' */
	/* 0xc0 */ 3, /* `ASM_GETRANGE_NI':             `getrange top, none, $<Simm16>' */
	/* 0xc1 */ 3, /* `ASM_GETRANGE_IN':             `getrange top, $<Simm16>, none' */
	/* 0xc2 */ 5, /* `ASM_GETRANGE_II':             `getrange top, $<Simm16>, $<Simm16>' */
	/* 0xc3 */ 1, /* `ASM_DELRANGE':                `delrange pop, pop, pop' */
	/* 0xc4 */ 1, /* `ASM_SETRANGE':                `setrange pop, pop, pop, pop' */
	/* 0xc5 */ 1, /* `ASM_SETRANGE_PN':             `setrange pop, pop, none, pop' */
	/* 0xc6 */ 1, /* `ASM_SETRANGE_NP':             `setrange pop, none, pop, pop' */
	/* 0xc7 */ 3, /* `ASM_SETRANGE_PI':             `setrange pop, pop, $<Simm16>, pop' */
	/* 0xc8 */ 3, /* `ASM_SETRANGE_IP':             `setrange pop, $<Simm16>, pop, pop' */
	/* 0xc9 */ 3, /* `ASM_SETRANGE_NI':             `setrange pop, none, $<Simm16>, pop' */
	/* 0xca */ 3, /* `ASM_SETRANGE_IN':             `setrange pop, $<Simm16>, none, pop' */
	/* 0xcb */ 5, /* `ASM_SETRANGE_II':             `setrange pop, $<Simm16>, $<Simm16>, pop' */
	/* 0xcc */ 1, /* `ASM_BREAKPOINT':              `debug break' */
	/* 0xcd */ 1, /* `ASM_UD':                      `ud' */
	/* 0xce */ 4, /* `ASM_CALLATTR_C_KW':           `callattr top, const <imm8>, #<imm8>, const <imm8>' */
	/* 0xcf */ 3, /* `ASM_CALLATTR_C_TUPLE_KW':     `callattr top, const <imm8>, pop..., const <imm8>' */
	/* 0xd0 */ 2, /* `ASM_CALLATTR':                `callattr top, pop, #<imm8>' */
	/* 0xd1 */ 1, /* `ASM_CALLATTR_TUPLE':          `callattr top, pop, pop...' */
	/* 0xd2 */ 3, /* `ASM_CALLATTR_C':              `callattr top, const <imm8>, #<imm8>' */
	/* 0xd3 */ 2, /* `ASM_CALLATTR_C_TUPLE':        `callattr top, const <imm8>, pop...' */
	/* 0xd4 */ 3, /* `ASM_CALLATTR_THIS_C':         `push callattr this, const <imm8>, #<imm8>' */
	/* 0xd5 */ 2, /* `ASM_CALLATTR_THIS_C_TUPLE':   `push callattr this, const <imm8>, pop...' */
	/* 0xd6 */ 3, /* `ASM_CALLATTR_C_SEQ':          `callattr top, const <imm8>, [#<imm8>]' */
	/* 0xd7 */ 3, /* `ASM_CALLATTR_C_MAP':          `callattr top, const <imm8>, {#<imm8>*2}' */
	/* 0xd8 */ 1, /* --- */
	/* 0xd9 */ 3, /* `ASM_GETMEMBER_THIS_R':        `push getmember this, ref <imm8>, $<imm8>' */
	/* 0xda */ 3, /* `ASM_DELMEMBER_THIS_R':        `delmember this, ref <imm8>, $<imm8>' */
	/* 0xdb */ 3, /* `ASM_SETMEMBER_THIS_R':        `setmember this, ref <imm8>, $<imm8>, pop' */
	/* 0xdc */ 3, /* `ASM_BOUNDMEMBER_THIS_R':      `push boundmember this, ref <imm8>, $<imm8>' */
	/* 0xdd */ 4, /* `ASM_CALL_EXTERN':             `push call extern <imm8>:<imm8>, #<imm8>' */
	/* 0xde */ 3, /* `ASM_CALL_GLOBAL':             `push call global <imm8>, #<imm8>' */
	/* 0xdf */ 3, /* `ASM_CALL_LOCAL':              `push call local <imm8>, #<imm8>' */
	/* 0xe0 */ 1, /* --- */
	/* 0xe1 */ 1, /* --- */
	/* 0xe2 */ 1, /* --- */
	/* 0xe3 */ 1, /* --- */
	/* 0xe4 */ 1, /* --- */
	/* 0xe5 */ 1, /* --- */
	/* 0xe6 */ 1, /* --- */
	/* 0xe7 */ 1, /* --- */
	/* 0xe8 */ 1, /* --- */
	/* 0xe9 */ 1, /* --- */
	/* 0xea */ 1, /* --- */
	/* 0xeb */ 1, /* --- */
	/* 0xec */ 1, /* --- */
	/* 0xed */ 1, /* --- */
	/* 0xee */ 1, /* --- */
	/* 0xef */ 1, /* --- */
	/* 0xf0 */ 0, /* --- */
	/* 0xf1 */ 0, /* --- */
	/* 0xf2 */ 0, /* --- */
	/* 0xf3 */ 0, /* --- */
	/* 0xf4 */ 0, /* --- */
	/* 0xf5 */ 0, /* --- */
	/* 0xf6 */ 0, /* --- */
	/* 0xf7 */ 0, /* --- */
	/* 0xf8 */ 0, /* --- */
	/* 0xf9 */ 0, /* --- */
	/* 0xfa */ 0, /* --- */
	/* 0xfb */ 0, /* --- */
	/* 0xfc */ 0, /* --- */
	/* 0xfd */ 0, /* --- */
	/* 0xfe */ 0, /* --- */
	/* 0xff */ 0, /* --- */
};

PRIVATE uint8_t const intr_len_f0[256] = {
	/* 0x00 */ 2, /* --- */
	/* 0x01 */ 2, /* --- */
	/* 0x02 */ 2, /* --- */
	/* 0x03 */ 2, /* --- */
	/* 0x04 */ 2, /* --- */
	/* 0x05 */ 2, /* --- */
	/* 0x06 */ 3, /* `ASM_ENDCATCH_N':              `end catch, #<imm8>+1' */
	/* 0x07 */ 3, /* `ASM_ENDFINALLY_N':            `end finally, #<imm8>+1' */
	/* 0x08 */ 5, /* `ASM16_CALL_KW':               `call top, #<imm8>, const <imm16>' */
	/* 0x09 */ 4, /* `ASM16_CALL_TUPLE_KW':         `call top, pop..., const <imm16>' */
	/* 0x0a */ 2, /* --- */
	/* 0x0b */ 4, /* `ASM16_PUSH_BND_ARG':          `push bound arg <imm8>' */
	/* 0x0c */ 6, /* `ASM16_PUSH_BND_EXTERN':       `push bound extern <imm16>:<imm16>' */
	/* 0x0d */ 2, /* --- */
	/* 0x0e */ 4, /* `ASM16_PUSH_BND_GLOBAL':       `push bound global <imm16>' */
	/* 0x0f */ 4, /* `ASM16_PUSH_BND_LOCAL':        `push bound local <imm16>' */
	/* 0x10 */ 2, /* --- */
	/* 0x11 */ 2, /* --- */
	/* 0x12 */ 2, /* --- */
	/* 0x13 */ 2, /* --- */
	/* 0x14 */ 6, /* `ASM32_JMP':                   `jmp <Sdisp32>' */
	/* 0x15 */ 2, /* --- */
	/* 0x16 */ 2, /* --- */
	/* 0x17 */ 2, /* --- */
	/* 0x18 */ 2, /* `ASM_JMP_POP_POP':             `jmp pop, #pop' */
	/* 0x19 */ 5, /* `ASM16_OPERATOR':              `op top, $<imm16>, #<imm8>' */
	/* 0x1a */ 4, /* `ASM16_OPERATOR_TUPLE':        `op top, $<imm16>, pop' */
	/* 0x1b */ 3, /* `ASM_CALL_SEQ':                `call top, [#<imm8>]' */
	/* 0x1c */ 3, /* `ASM_CALL_MAP':                `call top, {#<imm8>*2}' */
	/* 0x1d */ 2, /* `ASM_THISCALL_TUPLE':          `call top, pop, pop...' */
	/* 0x1e */ 4, /* `ASM16_DEL_GLOBAL':            `del global <imm16>' */
	/* 0x1f */ 4, /* `ASM16_DEL_LOCAL':             `del local <imm16>' */
	/* 0x20 */ 2, /* `ASM_CALL_TUPLE_KWDS':         `call top, pop..., pop' */
	/* 0x21 */ 4, /* `ASM16_LROT':                  `lrot #<imm16>+3' */
	/* 0x22 */ 4, /* `ASM16_RROT':                  `rrot #<imm16>+3' */
	/* 0x23 */ 2, /* --- */
	/* 0x24 */ 4, /* `ASM16_DUP_N':                 `dup #SP - <imm16> - 2' */
	/* 0x25 */ 2, /* --- */
	/* 0x26 */ 4, /* `ASM16_POP_N':                 `pop #SP - <imm16> - 2' */
	/* 0x27 */ 4, /* `ASM16_ADJSTACK':              `adjstack #SP +/- <Simm16>' */
	/* 0x28 */ 2, /* --- */
	/* 0x29 */ 4, /* `ASM16_SUPER_THIS_R':          `push super this, ref <imm16>' */
	/* 0x2a */ 2, /* --- */
	/* 0x2b */ 4, /* `ASM16_POP_STATIC':            `pop static <imm16>' */
	/* 0x2c */ 6, /* `ASM16_POP_EXTERN':            `pop extern <imm16>:<imm16>' */
	/* 0x2d */ 2, /* --- */
	/* 0x2e */ 4, /* `ASM16_POP_GLOBAL':            `pop global <imm16>' */
	/* 0x2f */ 4, /* `ASM16_POP_LOCAL':             `pop local <imm16>' */
	/* 0x30 */ 2, /* --- */
	/* 0x31 */ 2, /* --- */
	/* 0x32 */ 2, /* `ASM_PUSH_EXCEPT':             `push except' */
	/* 0x33 */ 2, /* `ASM_PUSH_THIS':               `push this' */
	/* 0x34 */ 2, /* `ASM_PUSH_THIS_MODULE':        `push this_module' */
	/* 0x35 */ 2, /* `ASM_PUSH_THIS_FUNCTION':      `push this_function' */
	/* 0x36 */ 4, /* `ASM16_PUSH_MODULE':           `push module <imm16>' */
	/* 0x37 */ 4, /* `ASM16_PUSH_ARG':              `push arg <imm16>' */
	/* 0x38 */ 4, /* `ASM16_PUSH_CONST':            `push const <imm16>' */
	/* 0x39 */ 4, /* `ASM16_PUSH_REF':              `push ref <imm16>' */
	/* 0x3a */ 2, /* --- */
	/* 0x3b */ 4, /* `ASM16_PUSH_STATIC':           `push static <imm16>' */
	/* 0x3c */ 6, /* `ASM16_PUSH_EXTERN':           `push extern <imm16>:<imm16>' */
	/* 0x3d */ 2, /* --- */
	/* 0x3e */ 4, /* `ASM16_PUSH_GLOBAL':           `push global <imm16>' */
	/* 0x3f */ 4, /* `ASM16_PUSH_LOCAL':            `push local <imm16>' */
	/* 0x40 */ 2, /* `ASM_CAST_HASHSET':            `cast top, HashSet' */
	/* 0x41 */ 2, /* `ASM_CAST_DICT':               `cast top, Dict' */
	/* 0x42 */ 4, /* `ASM16_PACK_TUPLE':            `push pack Tuple, #<imm16>' */
	/* 0x43 */ 4, /* `ASM16_PACK_LIST':             `push pack List, #<imm16>' */
	/* 0x44 */ 2, /* --- */
	/* 0x45 */ 2, /* --- */
	/* 0x46 */ 4, /* `ASM16_UNPACK':                `unpack pop, #<imm16>' */
	/* 0x47 */ 2, /* --- */
	/* 0x48 */ 2, /* --- */
	/* 0x49 */ 2, /* --- */
	/* 0x4a */ 2, /* --- */
	/* 0x4b */ 2, /* --- */
	/* 0x4c */ 2, /* --- */
	/* 0x4d */ 2, /* --- */
	/* 0x4e */ 2, /* --- */
	/* 0x4f */ 2, /* --- */
	/* 0x50 */ 2, /* `ASM_PUSH_TRUE':               `push true' */
	/* 0x51 */ 2, /* `ASM_PUSH_FALSE':              `push false' */
	/* 0x52 */ 3, /* `ASM_PACK_HASHSET':            `push pack HashSet, #<imm8>' */
	/* 0x53 */ 3, /* `ASM_PACK_DICT':               `push pack Dict, #<imm8>*2' */
	/* 0x54 */ 2, /* --- */
	/* 0x55 */ 2, /* --- */
	/* 0x56 */ 2, /* --- */
	/* 0x57 */ 2, /* --- */
	/* 0x58 */ 2, /* --- */
	/* 0x59 */ 2, /* `ASM_BOUNDITEM':               `bounditem top, pop' */
	/* 0x5a */ 4, /* `ASM16_GETATTR_C':             `getattr top, const <imm16>' */
	/* 0x5b */ 4, /* `ASM16_DELATTR_C':             `delattr pop, const <imm16>' */
	/* 0x5c */ 4, /* `ASM16_SETATTR_C':             `setattr pop, const <imm16>, pop' */
	/* 0x5d */ 4, /* `ASM16_GETATTR_THIS_C':        `push getattr this, const <imm16>' */
	/* 0x5e */ 4, /* `ASM16_DELATTR_THIS_C':        `delattr this, const <imm16>' */
	/* 0x5f */ 4, /* `ASM16_SETATTR_THIS_C':        `setattr this, const <imm16>, pop' */
	/* 0x60 */ 2, /* `ASM_CMP_SO':                  `cmp so, top, pop' */
	/* 0x61 */ 2, /* `ASM_CMP_DO':                  `cmp do, top, pop' */
	/* 0x62 */ 4, /* `ASM16_PACK_HASHSET':          `push pack HashSet, #<imm16>' */
	/* 0x63 */ 4, /* `ASM16_PACK_DICT':             `push pack Dict, #<imm16>*2' */
	/* 0x64 */ 4, /* `ASM16_GETCMEMBER':            `getcmember top, $<imm16>' */
	/* 0x65 */ 2, /* `ASM_CLASS':                   `class top, pop' */
	/* 0x66 */ 4, /* `ASM16_CLASS_C':               `class top, const <imm16>' */
	/* 0x67 */ 6, /* `ASM16_CLASS_GC':              `push class global <imm16>, const <imm16>' */
	/* 0x68 */ 8, /* `ASM16_CLASS_EC':              `push class extern <imm16>:<imm16>, const <imm16>' */
	/* 0x69 */ 4, /* `ASM16_DEFCMEMBER':            `defcmember top, $<imm16>, pop' */
	/* 0x6a */ 6, /* `ASM16_GETCMEMBER_R':          `push getcmember ref <imm16>, $<imm16>' */
	/* 0x6b */ 7, /* `ASM16_CALLCMEMBER_THIS_R':    `push callcmember this, ref <imm16>, $<imm16>, #<imm8>' */
	/* 0x6c */ 2, /* --- */
	/* 0x6d */ 2, /* --- */
	/* 0x6e */ 5, /* `ASM16_FUNCTION_C':            `push function const <imm16>, #<imm8>+1' */
	/* 0x6f */ 6, /* `ASM16_FUNCTION_C_16':         `push function const <imm16>, #<imm16>+1' */
	/* 0x70 */ 4, /* `ASM_SUPERGETATTR_THIS_RC':    `push getattr this, ref <imm8>, const <imm8>' */
	/* 0x71 */ 6, /* `ASM16_SUPERGETATTR_THIS_RC':  `push getattr this, ref <imm16>, const <imm16>' */
	/* 0x72 */ 5, /* `ASM_SUPERCALLATTR_THIS_RC':   `push callattr this, ref <imm8>, const <imm8>, #<imm8>' */
	/* 0x73 */ 7, /* `ASM16_SUPERCALLATTR_THIS_RC': `push callattr this, ref <imm16>, const <imm16>, #<imm8>' */
	/* 0x74 */ 2, /* --- */
	/* 0x75 */ 2, /* --- */
	/* 0x76 */ 2, /* --- */
	/* 0x77 */ 2, /* --- */
	/* 0x78 */ 2, /* --- */
	/* 0x79 */ 2, /* --- */
	/* 0x7a */ 2, /* --- */
	/* 0x7b */ 2, /* --- */
	/* 0x7c */ 2, /* --- */
	/* 0x7d */ 2, /* --- */
	/* 0x7e */ 2, /* --- */
	/* 0x7f */ 2, /* `ASM_INCPOST':                 `push inc' */
	/* 0x80 */ 2, /* `ASM_DECPOST':                 `push dec' */
	/* 0x81 */ 2, /* --- */
	/* 0x82 */ 2, /* --- */
	/* 0x83 */ 2, /* --- */
	/* 0x84 */ 2, /* --- */
	/* 0x85 */ 2, /* --- */
	/* 0x86 */ 2, /* --- */
	/* 0x87 */ 2, /* --- */
	/* 0x88 */ 2, /* --- */
	/* 0x89 */ 2, /* --- */
	/* 0x8a */ 2, /* --- */
	/* 0x8b */ 2, /* --- */
	/* 0x8c */ 2, /* --- */
	/* 0x8d */ 2, /* --- */
	/* 0x8e */ 2, /* --- */
	/* 0x8f */ 2, /* `ASM16_DELOP':                 `---' */
	/* 0x90 */ 2, /* `ASM16_NOP':                   `nop16' */
	/* 0x91 */ 2, /* `ASM_REDUCE_MIN':              `reduce top, min' */
	/* 0x92 */ 2, /* `ASM_REDUCE_MAX':              `reduce top, max' */
	/* 0x93 */ 2, /* `ASM_REDUCE_SUM':              `reduce top, sum' */
	/* 0x94 */ 2, /* `ASM_REDUCE_ANY':              `reduce top, any' */
	/* 0x95 */ 2, /* `ASM_REDUCE_ALL':              `reduce top, all' */
	/* 0x96 */ 2, /* --- */
	/* 0x97 */ 2, /* --- */
	/* 0x98 */ 2, /* --- */
	/* 0x99 */ 2, /* --- */
	/* 0x9a */ 2, /* --- */
	/* 0x9b */ 2, /* --- */
	/* 0x9c */ 2, /* --- */
	/* 0x9d */ 2, /* --- */
	/* 0x9e */ 2, /* --- */
	/* 0x9f */ 2, /* --- */
	/* 0xa0 */ 2, /* --- */
	/* 0xa1 */ 4, /* `ASM16_PRINT_C':               `print const <imm16>' */
	/* 0xa2 */ 4, /* `ASM16_PRINT_C_SP':            `print const <imm16>, sp' */
	/* 0xa3 */ 4, /* `ASM16_PRINT_C_NL':            `print const <imm16>, nl' */
	/* 0xa4 */ 6, /* `ASM_RANGE_0_I32':             `push range $0, $<imm32>' */
	/* 0xa5 */ 2, /* --- */
	/* 0xa6 */ 3, /* `ASM_VARARGS_UNPACK':          `unpack varargs, #<imm8>' */
	/* 0xa7 */ 2, /* `ASM_PUSH_VARKWDS_NE':         `push bool varkwds' */
	/* 0xa8 */ 2, /* --- */
	/* 0xa9 */ 4, /* `ASM16_FPRINT_C':              `print top, const <imm16>' */
	/* 0xaa */ 4, /* `ASM16_FPRINT_C_SP':           `print top, const <imm16>, sp' */
	/* 0xab */ 4, /* `ASM16_FPRINT_C_NL':           `print top, const <imm16>, nl' */
	/* 0xac */ 3, /* `ASM_VARARGS_CMP_EQ_SZ':       `push cmp eq, #varargs, $<imm8>' */
	/* 0xad */ 3, /* `ASM_VARARGS_CMP_GR_SZ':       `push cmp gr, #varargs, $<imm8>' */
	/* 0xae */ 2, /* --- */
	/* 0xaf */ 2, /* --- */
	/* 0xb0 */ 2, /* --- */
	/* 0xb1 */ 3, /* `ASM16_CONTAINS_C':            `push contains const <imm16>, pop' */
	/* 0xb2 */ 2, /* `ASM_VARARGS_GETITEM':         `getitem varargs, top' */
	/* 0xb3 */ 3, /* `ASM_VARARGS_GETITEM_I':       `push getitem varargs, $<imm8>' */
	/* 0xb4 */ 4, /* `ASM16_GETITEM_C':             `getitem top, const <imm16>' */
	/* 0xb5 */ 2, /* `ASM_VARARGS_GETSIZE':         `push getsize varargs' */
	/* 0xb6 */ 2, /* --- */
	/* 0xb7 */ 2, /* --- */
	/* 0xb8 */ 4, /* `ASM16_SETITEM_C':             `setitem pop, const <imm16>, pop' */
	/* 0xb9 */ 2, /* `ASM_ITERNEXT':                `iternext top' */
	/* 0xba */ 2, /* --- */
	/* 0xbb */ 2, /* --- */
	/* 0xbc */ 2, /* --- */
	/* 0xbd */ 2, /* --- */
	/* 0xbe */ 3, /* `ASM_GETMEMBER':               `getmember top, pop, $<imm8>' */
	/* 0xbf */ 4, /* `ASM16_GETMEMBER':             `getmember top, pop, $<imm16>' */
	/* 0xc0 */ 3, /* `ASM_DELMEMBER':               `delmember pop, pop, $<imm8>' */
	/* 0xc1 */ 4, /* `ASM16_DELMEMBER':             `delmember pop, pop, $<imm16>' */
	/* 0xc2 */ 3, /* `ASM_SETMEMBER':               `setmember pop, pop, $<imm8>, pop' */
	/* 0xc3 */ 4, /* `ASM16_SETMEMBER':             `setmember pop, pop, $<imm16>, pop' */
	/* 0xc4 */ 3, /* `ASM_BOUNDMEMBER':             `boundmember top, pop, $<imm8>' */
	/* 0xc5 */ 4, /* `ASM16_BOUNDMEMBER':           `boundmember top, pop, $<imm16>' */
	/* 0xc6 */ 3, /* `ASM_GETMEMBER_THIS':          `push getmember this, pop, $<imm8>' */
	/* 0xc7 */ 4, /* `ASM16_GETMEMBER_THIS':        `push getmember this, pop, $<imm16>' */
	/* 0xc8 */ 3, /* `ASM_DELMEMBER_THIS':          `delmember this, pop, $<imm8>' */
	/* 0xc9 */ 4, /* `ASM16_DELMEMBER_THIS':        `delmember this, pop, $<imm16>' */
	/* 0xca */ 3, /* `ASM_SETMEMBER_THIS':          `setmember this, pop, $<imm8>, pop' */
	/* 0xcb */ 4, /* `ASM16_SETMEMBER_THIS':        `setmember this, pop, $<imm16>, pop' */
	/* 0xcc */ 3, /* `ASM_BOUNDMEMBER_THIS':        `push boundmember this, pop, $<imm8>' */
	/* 0xcd */ 4, /* `ASM16_BOUNDMEMBER_THIS':      `push boundmember this, pop, $<imm16>' */
	/* 0xce */ 7, /* `ASM16_CALLATTR_C_KW':         `callattr top, const <imm16>, #<imm8>, const <imm16>' */
	/* 0xcf */ 6, /* `ASM16_CALLATTR_C_TUPLE_KW':   `callattr top, const <imm16>, pop..., const <imm16>' */
	/* 0xd0 */ 2, /* `ASM_CALLATTR_KWDS':           `callattr top, pop, #<imm8>, pop' */
	/* 0xd1 */ 2, /* `ASM_CALLATTR_TUPLE_KWDS':     `callattr top, pop, pop..., pop' */
	/* 0xd2 */ 5, /* `ASM16_CALLATTR_C':            `callattr top, const <imm16>, #<imm8>' */
	/* 0xd3 */ 4, /* `ASM16_CALLATTR_C_TUPLE':      `callattr top, const <imm16>, pop' */
	/* 0xd4 */ 5, /* `ASM16_CALLATTR_THIS_C':       `callattr this, const <imm16>, #<imm8>' */
	/* 0xd5 */ 4, /* `ASM16_CALLATTR_THIS_C_TUPLE': `callattr this, const <imm16>, pop' */
	/* 0xd6 */ 5, /* `ASM16_CALLATTR_C_SEQ':        `callattr top, const <imm16>, [#<imm8>]' */
	/* 0xd7 */ 5, /* `ASM16_CALLATTR_C_MAP':        `callattr top, const <imm16>, {#<imm8>*2}' */
	/* 0xd8 */ 2, /* --- */
	/* 0xd9 */ 6, /* `ASM16_GETMEMBER_THIS_R':      `push getmember this, ref <imm16>, $<imm16>' */
	/* 0xda */ 6, /* `ASM16_DELMEMBER_THIS_R':      `delmember this, ref <imm16>, $<imm16>' */
	/* 0xdb */ 6, /* `ASM16_SETMEMBER_THIS_R':      `setmember this, ref <imm16>, $<imm16>, pop' */
	/* 0xdc */ 6, /* `ASM16_BOUNDMEMBER_THIS_R':    `push boundmember this, ref <imm16>, $<imm16>' */
	/* 0xdd */ 7, /* `ASM16_CALL_EXTERN':           `push call extern <imm16>:<imm16>, #<imm8>' */
	/* 0xde */ 5, /* `ASM16_CALL_GLOBAL':           `push call global <imm16>, #<imm8>' */
	/* 0xdf */ 5, /* `ASM16_CALL_LOCAL':            `push call local <imm16>, #<imm8>' */
	/* 0xe0 */ 2, /* --- */
	/* 0xe1 */ 2, /* --- */
	/* 0xe2 */ 2, /* --- */
	/* 0xe3 */ 2, /* --- */
	/* 0xe4 */ 2, /* --- */
	/* 0xe5 */ 2, /* --- */
	/* 0xe6 */ 2, /* --- */
	/* 0xe7 */ 2, /* --- */
	/* 0xe8 */ 2, /* --- */
	/* 0xe9 */ 2, /* --- */
	/* 0xea */ 2, /* --- */
	/* 0xeb */ 2, /* --- */
	/* 0xec */ 2, /* --- */
	/* 0xed */ 2, /* --- */
	/* 0xee */ 2, /* --- */
	/* 0xef */ 2, /* --- */
	/* 0xf0 */ 2, /* --- */
	/* 0xf1 */ 2, /* --- */
	/* 0xf2 */ 2, /* --- */
	/* 0xf3 */ 2, /* --- */
	/* 0xf4 */ 2, /* --- */
	/* 0xf5 */ 2, /* --- */
	/* 0xf6 */ 2, /* --- */
	/* 0xf7 */ 2, /* --- */
	/* 0xf8 */ 2, /* --- */
	/* 0xf9 */ 2, /* --- */
	/* 0xfa */ 2, /* --- */
	/* 0xfb */ 2, /* --- */
	/* 0xfc */ 2, /* --- */
	/* 0xfd */ 2, /* --- */
	/* 0xfe */ 2, /* --- */
	/* 0xff */ 2, /* --- */
};

PRIVATE uint8_t const stack_effect[256] = {
	/* 0x00 */ STACK_EFFECT(0, 0), /* `ASM_RET_NONE':                `ret' */
	/* 0x01 */ STACK_EFFECT(1, 0), /* `ASM_YIELD':                   `yield pop' */
	/* 0x02 */ STACK_EFFECT(1, 0), /* `ASM_YIELDALL':                `yield foreach, pop' */
	/* 0x03 */ STACK_EFFECT(1, 0), /* `ASM_THROW':                   `throw pop' */
	/* 0x04 */ STACK_EFFECT(0, 0), /* `ASM_RETHROW':                 `throw except' */
	/* 0x05 */ STACK_EFFECT(1, 0), /* `ASM_SETRET':                  `setret pop' */
	/* 0x06 */ STACK_EFFECT(0, 0), /* `ASM_ENDCATCH':                `end catch' */
	/* 0x07 */ STACK_EFFECT(0, 0), /* `ASM_ENDFINALLY':              `end finally' */
	/* 0x08 */ STACK_EFFECT_UNDEF, /* `ASM_CALL_KW':                 `call top, #<imm8>, const <imm8>' */
	/* 0x09 */ STACK_EFFECT(2, 1), /* `ASM_CALL_TUPLE_KW':           `call top, pop..., const <imm8>' */
	/* 0x0a */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x0b */ STACK_EFFECT(0, 1), /* `ASM_PUSH_BND_ARG':            `push bound arg <imm8>' */
	/* 0x0c */ STACK_EFFECT(0, 1), /* `ASM_PUSH_BND_EXTERN':         `push bound extern <imm8>:<imm8>' */
	/* 0x0d */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x0e */ STACK_EFFECT(0, 1), /* `ASM_PUSH_BND_GLOBAL':         `push bound global <imm8>' */
	/* 0x0f */ STACK_EFFECT(0, 1), /* `ASM_PUSH_BND_LOCAL':          `push bound local <imm8>' */
	/* 0x10 */ STACK_EFFECT(1, 0), /* `ASM_JF':                      `jf pop, <Sdisp8>' */
	/* 0x11 */ STACK_EFFECT(1, 0), /* `ASM_JF16':                    `jf pop, <Sdisp16>' */
	/* 0x12 */ STACK_EFFECT(1, 0), /* `ASM_JT':                      `jt pop, <Sdisp8>' */
	/* 0x13 */ STACK_EFFECT(1, 0), /* `ASM_JT16':                    `jt pop, <Sdisp16>' */
	/* 0x14 */ STACK_EFFECT(0, 0), /* `ASM_JMP':                     `jmp <Sdisp8>' */
	/* 0x15 */ STACK_EFFECT(0, 0), /* `ASM_JMP16':                   `jmp <Sdisp16>' */
	/* 0x16 */ STACK_EFFECT(1, 2), /* `ASM_FOREACH':                 `foreach top, <Sdisp8>' */
	/* 0x17 */ STACK_EFFECT(1, 2), /* `ASM_FOREACH16':               `foreach top, <Sdisp16>' */
	/* 0x18 */ STACK_EFFECT(1, 0), /* `ASM_JMP_POP':                 `jmp pop' */
	/* 0x19 */ STACK_EFFECT_UNDEF, /* `ASM_OPERATOR':                `op top, $<imm8>, #<imm8>' */
	/* 0x1a */ STACK_EFFECT(2, 1), /* `ASM_OPERATOR_TUPLE':          `op top, $<imm8>, pop...' */
	/* 0x1b */ STACK_EFFECT_UNDEF, /* `ASM_CALL':                    `call top, #<imm8>' */
	/* 0x1c */ STACK_EFFECT(2, 1), /* `ASM_CALL_TUPLE':              `call top, pop...' */
	/* 0x1d */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x1e */ STACK_EFFECT(0, 0), /* `ASM_DEL_GLOBAL':              `del global <imm8>' */
	/* 0x1f */ STACK_EFFECT(0, 0), /* `ASM_DEL_LOCAL':               `del local <imm8>' */
	/* 0x20 */ STACK_EFFECT(2, 2), /* `ASM_SWAP':                    `swap' */
	/* 0x21 */ STACK_EFFECT_UNDEF, /* `ASM_LROT':                    `lrot #<imm8>+3' */
	/* 0x22 */ STACK_EFFECT_UNDEF, /* `ASM_RROT':                    `rrot #<imm8>+3' */
	/* 0x23 */ STACK_EFFECT(1, 2), /* `ASM_DUP':                     `dup' */
	/* 0x24 */ STACK_EFFECT(0, 1), /* `ASM_DUP_N':                   `dup #SP - <imm8> - 2' */
	/* 0x25 */ STACK_EFFECT(1, 0), /* `ASM_POP':                     `pop' */
	/* 0x26 */ STACK_EFFECT(1, 0), /* `ASM_POP_N':                   `pop #SP - <imm8> - 2' */
	/* 0x27 */ STACK_EFFECT_UNDEF, /* `ASM_ADJSTACK':                `adjstack #SP +/- <Simm8>' */
	/* 0x28 */ STACK_EFFECT(2, 1), /* `ASM_SUPER':                   `super top, pop' */
	/* 0x29 */ STACK_EFFECT(0, 1), /* `ASM_SUPER_THIS_R':            `push super this, ref <imm8>' */
	/* 0x2a */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x2b */ STACK_EFFECT(1, 0), /* `ASM_POP_STATIC':              `pop static <imm8>' */
	/* 0x2c */ STACK_EFFECT(1, 0), /* `ASM_POP_EXTERN':              `pop extern <imm8>:<imm8>' */
	/* 0x2d */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x2e */ STACK_EFFECT(1, 0), /* `ASM_POP_GLOBAL':              `pop global <imm8>' */
	/* 0x2f */ STACK_EFFECT(1, 0), /* `ASM_POP_LOCAL':               `pop local <imm8>' */
	/* 0x30 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x31 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x32 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x33 */ STACK_EFFECT(0, 1), /* `ASM_PUSH_NONE':               `push none' */
	/* 0x34 */ STACK_EFFECT(0, 1), /* `ASM_PUSH_VARARGS':            `push varargs' */
	/* 0x35 */ STACK_EFFECT(0, 1), /* `ASM_PUSH_VARKWDS':            `push varkwds' */
	/* 0x36 */ STACK_EFFECT(0, 1), /* `ASM_PUSH_MODULE':             `push module <imm8>' */
	/* 0x37 */ STACK_EFFECT(0, 1), /* `ASM_PUSH_ARG':                `push arg <imm8>' */
	/* 0x38 */ STACK_EFFECT(0, 1), /* `ASM_PUSH_CONST':              `push const <imm8>' */
	/* 0x39 */ STACK_EFFECT(0, 1), /* `ASM_PUSH_REF':                `push ref <imm8>' */
	/* 0x3a */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x3b */ STACK_EFFECT(0, 1), /* `ASM_PUSH_STATIC':             `push static <imm8>' */
	/* 0x3c */ STACK_EFFECT(0, 1), /* `ASM_PUSH_EXTERN':             `push extern <imm8>:<imm8>' */
	/* 0x3d */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x3e */ STACK_EFFECT(0, 1), /* `ASM_PUSH_GLOBAL':             `push global <imm8>' */
	/* 0x3f */ STACK_EFFECT(0, 1), /* `ASM_PUSH_LOCAL':              `push local <imm8>' */
	/* 0x40 */ STACK_EFFECT(1, 1), /* `ASM_CAST_TUPLE':              `cast top, Tuple' */
	/* 0x41 */ STACK_EFFECT(1, 1), /* `ASM_CAST_LIST':               `cast top, List' */
	/* 0x42 */ STACK_EFFECT_UNDEF, /* `ASM_PACK_TUPLE':              `push pack Tuple, #<imm8>' */
	/* 0x43 */ STACK_EFFECT_UNDEF, /* `ASM_PACK_LIST':               `push pack List, #<imm8>' */
	/* 0x44 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x45 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x46 */ STACK_EFFECT_UNDEF, /* `ASM_UNPACK':                  `unpack pop, #<imm8>' */
	/* 0x47 */ STACK_EFFECT(2, 1), /* `ASM_CONCAT':                  `concat top, pop' */
	/* 0x48 */ STACK_EFFECT_UNDEF, /* `ASM_EXTEND':                  `extend top, #<imm8>' */
	/* 0x49 */ STACK_EFFECT(1, 1), /* `ASM_TYPEOF':                  `typeof top' */
	/* 0x4a */ STACK_EFFECT(1, 1), /* `ASM_CLASSOF':                 `classof top' */
	/* 0x4b */ STACK_EFFECT(1, 1), /* `ASM_SUPEROF':                 `superof top' */
	/* 0x4c */ STACK_EFFECT(2, 1), /* `ASM_INSTANCEOF':              `instanceof top, pop' */
	/* 0x4d */ STACK_EFFECT(1, 1), /* `ASM_STR':                     `str top' */
	/* 0x4e */ STACK_EFFECT(1, 1), /* `ASM_REPR':                    `repr top' */
	/* 0x4f */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x50 */ STACK_EFFECT(1, 1), /* `ASM_BOOL':                    `bool top' */
	/* 0x51 */ STACK_EFFECT(1, 1), /* `ASM_NOT':                     `not top' */
	/* 0x52 */ STACK_EFFECT(2, 0), /* `ASM_ASSIGN':                  `assign pop, pop' */
	/* 0x53 */ STACK_EFFECT(2, 0), /* `ASM_MOVE_ASSIGN':             `assign move, pop, pop' */
	/* 0x54 */ STACK_EFFECT(1, 1), /* `ASM_COPY':                    `copy top' */
	/* 0x55 */ STACK_EFFECT(1, 1), /* `ASM_DEEPCOPY':                `deepcopy top' */
	/* 0x56 */ STACK_EFFECT(2, 1), /* `ASM_GETATTR':                 `getattr top, pop' */
	/* 0x57 */ STACK_EFFECT(2, 0), /* `ASM_DELATTR':                 `delattr pop, pop' */
	/* 0x58 */ STACK_EFFECT(3, 0), /* `ASM_SETATTR':                 `setattr pop, pop, pop' */
	/* 0x59 */ STACK_EFFECT(2, 1), /* `ASM_BOUNDATTR':               `boundattr top, pop' */
	/* 0x5a */ STACK_EFFECT(1, 1), /* `ASM_GETATTR_C':               `getattr top, const <imm8>' */
	/* 0x5b */ STACK_EFFECT(1, 0), /* `ASM_DELATTR_C':               `delattr pop, const <imm8>' */
	/* 0x5c */ STACK_EFFECT(2, 0), /* `ASM_SETATTR_C':               `setattr pop, const <imm8>, pop' */
	/* 0x5d */ STACK_EFFECT(0, 1), /* `ASM_GETATTR_THIS_C':          `push getattr this, const <imm8>' */
	/* 0x5e */ STACK_EFFECT(0, 0), /* `ASM_DELATTR_THIS_C':          `delattr this, const <imm8>' */
	/* 0x5f */ STACK_EFFECT(1, 0), /* `ASM_SETATTR_THIS_C':          `setattr this, const <imm8>, pop' */
	/* 0x60 */ STACK_EFFECT(2, 1), /* `ASM_CMP_EQ':                  `cmp eq, top, pop' */
	/* 0x61 */ STACK_EFFECT(2, 1), /* `ASM_CMP_NE':                  `cmp ne, top, pop' */
	/* 0x62 */ STACK_EFFECT(2, 1), /* `ASM_CMP_GE':                  `cmp ge, top, pop' */
	/* 0x63 */ STACK_EFFECT(2, 1), /* `ASM_CMP_LO':                  `cmp lo, top, pop' */
	/* 0x64 */ STACK_EFFECT(2, 1), /* `ASM_CMP_LE':                  `cmp le, top, pop' */
	/* 0x65 */ STACK_EFFECT(2, 1), /* `ASM_CMP_GR':                  `cmp gr, top, pop' */
	/* 0x66 */ STACK_EFFECT(1, 1), /* `ASM_CLASS_C':                 `class top, const <imm8>' */
	/* 0x67 */ STACK_EFFECT(0, 1), /* `ASM_CLASS_GC':                `push class global <imm8>, const <imm8>' */
	/* 0x68 */ STACK_EFFECT(0, 1), /* `ASM_CLASS_EC':                `push class extern <imm8>:<imm8>, const <imm8>' */
	/* 0x69 */ STACK_EFFECT(2, 1), /* `ASM_DEFCMEMBER':              `defcmember top, $<imm8>, pop' */
	/* 0x6a */ STACK_EFFECT(0, 1), /* `ASM_GETCMEMBER_R':            `push getcmember ref <imm8>, $<imm8>' */
	/* 0x6b */ STACK_EFFECT_UNDEF, /* `ASM_CALLCMEMBER_THIS_R':      `push callcmember this, ref <imm8>, $<imm8>, #<imm8>' */
	/* 0x6c */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x6d */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x6e */ STACK_EFFECT_UNDEF, /* `ASM_FUNCTION_C':              `push function const <imm8>, #<imm8>+1' */
	/* 0x6f */ STACK_EFFECT_UNDEF, /* `ASM_FUNCTION_C_16':           `push function const <imm8>, #<imm16>+1' */
	/* 0x70 */ STACK_EFFECT(1, 1), /* `ASM_CAST_INT':                `cast top, int' */
	/* 0x71 */ STACK_EFFECT(1, 1), /* `ASM_INV':                     `inv top' */
	/* 0x72 */ STACK_EFFECT(1, 1), /* `ASM_POS':                     `pos top' */
	/* 0x73 */ STACK_EFFECT(1, 1), /* `ASM_NEG':                     `neg top' */
	/* 0x74 */ STACK_EFFECT(2, 1), /* `ASM_ADD':                     `add top, pop' */
	/* 0x75 */ STACK_EFFECT(2, 1), /* `ASM_SUB':                     `sub top, pop' */
	/* 0x76 */ STACK_EFFECT(2, 1), /* `ASM_MUL':                     `mul top, pop' */
	/* 0x77 */ STACK_EFFECT(2, 1), /* `ASM_DIV':                     `div top, pop' */
	/* 0x78 */ STACK_EFFECT(2, 1), /* `ASM_MOD':                     `mod top, pop' */
	/* 0x79 */ STACK_EFFECT(2, 1), /* `ASM_SHL':                     `shl top, pop' */
	/* 0x7a */ STACK_EFFECT(2, 1), /* `ASM_SHR':                     `shr top, pop' */
	/* 0x7b */ STACK_EFFECT(2, 1), /* `ASM_AND':                     `and top, pop' */
	/* 0x7c */ STACK_EFFECT(2, 1), /* `ASM_OR':                      `or top, pop' */
	/* 0x7d */ STACK_EFFECT(2, 1), /* `ASM_XOR':                     `xor top, pop' */
	/* 0x7e */ STACK_EFFECT(2, 1), /* `ASM_POW':                     `pow top, pop' */
	/* 0x7f */ STACK_EFFECT(0, 0), /* `ASM_INC':                     `inc' */
	/* 0x80 */ STACK_EFFECT(0, 0), /* `ASM_DEC':                     `dec' */
	/* 0x81 */ STACK_EFFECT(1, 1), /* `ASM_ADD_SIMM8':               `add top, $<Simm8>' */
	/* 0x82 */ STACK_EFFECT(1, 1), /* `ASM_ADD_IMM32':               `add top, $<imm32>' */
	/* 0x83 */ STACK_EFFECT(1, 1), /* `ASM_SUB_SIMM8':               `sub top, $<Simm8>' */
	/* 0x84 */ STACK_EFFECT(1, 1), /* `ASM_SUB_IMM32':               `sub top, $<imm32>' */
	/* 0x85 */ STACK_EFFECT(1, 1), /* `ASM_MUL_SIMM8':               `mul top, $<Simm8>' */
	/* 0x86 */ STACK_EFFECT(1, 1), /* `ASM_DIV_SIMM8':               `div top, $<Simm8>' */
	/* 0x87 */ STACK_EFFECT(1, 1), /* `ASM_MOD_SIMM8':               `mod top, $<Simm8>' */
	/* 0x88 */ STACK_EFFECT(1, 1), /* `ASM_SHL_IMM8':                `shl top, $<imm8>' */
	/* 0x89 */ STACK_EFFECT(1, 1), /* `ASM_SHR_IMM8':                `shr top, $<imm8>' */
	/* 0x8a */ STACK_EFFECT(1, 1), /* `ASM_AND_IMM32':               `and top, $<imm32>' */
	/* 0x8b */ STACK_EFFECT(1, 1), /* `ASM_OR_IMM32':                `or top, $<imm32>' */
	/* 0x8c */ STACK_EFFECT(1, 1), /* `ASM_XOR_IMM32':               `xor top, $<imm32>' */
	/* 0x8d */ STACK_EFFECT(1, 1), /* `ASM_ISNONE':                  `instanceof top, none' */
	/* 0x8e */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x8f */ STACK_EFFECT(0, 0), /* `ASM_DELOP':                   `---' */
	/* 0x90 */ STACK_EFFECT(0, 0), /* `ASM_NOP':                     `nop' */
	/* 0x91 */ STACK_EFFECT(1, 0), /* `ASM_PRINT':                   `print pop' */
	/* 0x92 */ STACK_EFFECT(1, 0), /* `ASM_PRINT_SP':                `print pop, sp' */
	/* 0x93 */ STACK_EFFECT(1, 0), /* `ASM_PRINT_NL':                `print pop, nl' */
	/* 0x94 */ STACK_EFFECT(0, 0), /* `ASM_PRINTNL':                 `print nl' */
	/* 0x95 */ STACK_EFFECT(1, 0), /* `ASM_PRINTALL':                `print pop...' */
	/* 0x96 */ STACK_EFFECT(1, 0), /* `ASM_PRINTALL_SP':             `print pop..., sp' */
	/* 0x97 */ STACK_EFFECT(1, 0), /* `ASM_PRINTALL_NL':             `print pop..., nl' */
	/* 0x98 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x99 */ STACK_EFFECT(2, 1), /* `ASM_FPRINT':                  `print top, pop' */
	/* 0x9a */ STACK_EFFECT(2, 1), /* `ASM_FPRINT_SP':               `print top, pop, sp' */
	/* 0x9b */ STACK_EFFECT(2, 1), /* `ASM_FPRINT_NL':               `print top, pop, nl' */
	/* 0x9c */ STACK_EFFECT(1, 1), /* `ASM_FPRINTNL':                `print top, nl' */
	/* 0x9d */ STACK_EFFECT(2, 1), /* `ASM_FPRINTALL':               `print top, pop...' */
	/* 0x9e */ STACK_EFFECT(2, 1), /* `ASM_FPRINTALL_SP':            `print top, pop..., sp' */
	/* 0x9f */ STACK_EFFECT(2, 1), /* `ASM_FPRINTALL_NL':            `print top, pop..., nl' */
	/* 0xa0 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xa1 */ STACK_EFFECT(0, 0), /* `ASM_PRINT_C':                 `print const <imm8>' */
	/* 0xa2 */ STACK_EFFECT(0, 0), /* `ASM_PRINT_C_SP':              `print const <imm8>, sp' */
	/* 0xa3 */ STACK_EFFECT(0, 0), /* `ASM_PRINT_C_NL':              `print const <imm8>, nl' */
	/* 0xa4 */ STACK_EFFECT(0, 1), /* `ASM_RANGE_0_I16':             `push range $0, $<imm16>' */
	/* 0xa5 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xa6 */ STACK_EFFECT(1, 1), /* `ASM_ENTER':                   `enter top' */
	/* 0xa7 */ STACK_EFFECT(1, 0), /* `ASM_LEAVE':                   `leave pop' */
	/* 0xa8 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xa9 */ STACK_EFFECT(1, 1), /* `ASM_FPRINT_C':                `print top, const <imm8>' */
	/* 0xaa */ STACK_EFFECT(1, 1), /* `ASM_FPRINT_C_SP':             `print top, const <imm8>, sp' */
	/* 0xab */ STACK_EFFECT(1, 1), /* `ASM_FPRINT_C_NL':             `print top, const <imm8>, nl' */
	/* 0xac */ STACK_EFFECT(2, 1), /* `ASM_RANGE':                   `range top, pop' */
	/* 0xad */ STACK_EFFECT(1, 1), /* `ASM_RANGE_DEF':               `push range default, pop' */
	/* 0xae */ STACK_EFFECT(3, 1), /* `ASM_RANGE_STEP':              `range top, pop, pop' */
	/* 0xaf */ STACK_EFFECT(2, 1), /* `ASM_RANGE_STEP_DEF':          `push range default, pop, pop' */
	/* 0xb0 */ STACK_EFFECT(2, 1), /* `ASM_CONTAINS':                `contains top, pop' */
	/* 0xb1 */ STACK_EFFECT(1, 1), /* `ASM_CONTAINS_C':              `push contains const <imm8>, pop' */
	/* 0xb2 */ STACK_EFFECT(2, 1), /* `ASM_GETITEM':                 `getitem top, pop' */
	/* 0xb3 */ STACK_EFFECT(1, 1), /* `ASM_GETITEM_I':               `getitem top, $<Simm16>' */
	/* 0xb4 */ STACK_EFFECT(1, 1), /* `ASM_GETITEM_C':               `getitem top, const <imm8>' */
	/* 0xb5 */ STACK_EFFECT(1, 1), /* `ASM_GETSIZE':                 `getsize top' */
	/* 0xb6 */ STACK_EFFECT(3, 0), /* `ASM_SETITEM':                 `setitem pop, pop, pop' */
	/* 0xb7 */ STACK_EFFECT(2, 0), /* `ASM_SETITEM_I':               `setitem pop, $<Simm16>, pop' */
	/* 0xb8 */ STACK_EFFECT(2, 0), /* `ASM_SETITEM_C':               `setitem pop, const <imm8>, pop' */
	/* 0xb9 */ STACK_EFFECT(1, 1), /* `ASM_ITERSELF':                `iterself top' */
	/* 0xba */ STACK_EFFECT(2, 0), /* `ASM_DELITEM':                 `delitem pop, pop' */
	/* 0xbb */ STACK_EFFECT(3, 1), /* `ASM_GETRANGE':                `getrange top, pop, pop' */
	/* 0xbc */ STACK_EFFECT(2, 1), /* `ASM_GETRANGE_PN':             `getrange top, pop, none' */
	/* 0xbd */ STACK_EFFECT(2, 1), /* `ASM_GETRANGE_NP':             `getrange top, none, pop' */
	/* 0xbe */ STACK_EFFECT(2, 1), /* `ASM_GETRANGE_PI':             `getrange top, pop, $<Simm16>' */
	/* 0xbf */ STACK_EFFECT(2, 1), /* `ASM_GETRANGE_IP':             `getrange top, $<Simm16>, pop' */
	/* 0xc0 */ STACK_EFFECT(1, 1), /* `ASM_GETRANGE_NI':             `getrange top, none, $<Simm16>' */
	/* 0xc1 */ STACK_EFFECT(1, 1), /* `ASM_GETRANGE_IN':             `getrange top, $<Simm16>, none' */
	/* 0xc2 */ STACK_EFFECT(1, 1), /* `ASM_GETRANGE_II':             `getrange top, $<Simm16>, $<Simm16>' */
	/* 0xc3 */ STACK_EFFECT(3, 0), /* `ASM_DELRANGE':                `delrange pop, pop, pop' */
	/* 0xc4 */ STACK_EFFECT(4, 0), /* `ASM_SETRANGE':                `setrange pop, pop, pop, pop' */
	/* 0xc5 */ STACK_EFFECT(3, 0), /* `ASM_SETRANGE_PN':             `setrange pop, pop, none, pop' */
	/* 0xc6 */ STACK_EFFECT(3, 0), /* `ASM_SETRANGE_NP':             `setrange pop, none, pop, pop' */
	/* 0xc7 */ STACK_EFFECT(3, 0), /* `ASM_SETRANGE_PI':             `setrange pop, pop, $<Simm16>, pop' */
	/* 0xc8 */ STACK_EFFECT(3, 0), /* `ASM_SETRANGE_IP':             `setrange pop, $<Simm16>, pop, pop' */
	/* 0xc9 */ STACK_EFFECT(2, 0), /* `ASM_SETRANGE_NI':             `setrange pop, none, $<Simm16>, pop' */
	/* 0xca */ STACK_EFFECT(2, 0), /* `ASM_SETRANGE_IN':             `setrange pop, $<Simm16>, none, pop' */
	/* 0xcb */ STACK_EFFECT(2, 0), /* `ASM_SETRANGE_II':             `setrange pop, $<Simm16>, $<Simm16>, pop' */
	/* 0xcc */ STACK_EFFECT(0, 0), /* `ASM_BREAKPOINT':              `debug break' */
	/* 0xcd */ STACK_EFFECT(0, 0), /* `ASM_UD':                      `ud' */
	/* 0xce */ STACK_EFFECT_UNDEF, /* `ASM_CALLATTR_C_KW':           `callattr top, const <imm8>, #<imm8>, const <imm8>' */
	/* 0xcf */ STACK_EFFECT(2, 1), /* `ASM_CALLATTR_C_TUPLE_KW':     `callattr top, const <imm8>, pop..., const <imm8>' */
	/* 0xd0 */ STACK_EFFECT_UNDEF, /* `ASM_CALLATTR':                `callattr top, pop, #<imm8>' */
	/* 0xd1 */ STACK_EFFECT(3, 1), /* `ASM_CALLATTR_TUPLE':          `callattr top, pop, pop...' */
	/* 0xd2 */ STACK_EFFECT_UNDEF, /* `ASM_CALLATTR_C':              `callattr top, const <imm8>, #<imm8>' */
	/* 0xd3 */ STACK_EFFECT(2, 1), /* `ASM_CALLATTR_C_TUPLE':        `callattr top, const <imm8>, pop...' */
	/* 0xd4 */ STACK_EFFECT_UNDEF, /* `ASM_CALLATTR_THIS_C':         `push callattr this, const <imm8>, #<imm8>' */
	/* 0xd5 */ STACK_EFFECT(1, 1), /* `ASM_CALLATTR_THIS_C_TUPLE':   `push callattr this, const <imm8>, pop...' */
	/* 0xd6 */ STACK_EFFECT_UNDEF, /* `ASM_CALLATTR_C_SEQ':          `callattr top, const <imm8>, [#<imm8>]' */
	/* 0xd7 */ STACK_EFFECT_UNDEF, /* `ASM_CALLATTR_C_MAP':          `callattr top, const <imm8>, {#<imm8>*2}' */
	/* 0xd8 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xd9 */ STACK_EFFECT(0, 1), /* `ASM_GETMEMBER_THIS_R':        `push getmember this, ref <imm8>, $<imm8>' */
	/* 0xda */ STACK_EFFECT(0, 0), /* `ASM_DELMEMBER_THIS_R':        `delmember this, ref <imm8>, $<imm8>' */
	/* 0xdb */ STACK_EFFECT(1, 0), /* `ASM_SETMEMBER_THIS_R':        `setmember this, ref <imm8>, $<imm8>, pop' */
	/* 0xdc */ STACK_EFFECT(0, 1), /* `ASM_BOUNDMEMBER_THIS_R':      `push boundmember this, ref <imm8>, $<imm8>' */
	/* 0xdd */ STACK_EFFECT_UNDEF, /* `ASM_CALL_EXTERN':             `push call extern <imm8>:<imm8>, #<imm8>' */
	/* 0xde */ STACK_EFFECT_UNDEF, /* `ASM_CALL_GLOBAL':             `push call global <imm8>, #<imm8>' */
	/* 0xdf */ STACK_EFFECT_UNDEF, /* `ASM_CALL_LOCAL':              `push call local <imm8>, #<imm8>' */
	/* 0xe0 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe1 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe2 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe3 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe4 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe5 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe6 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe7 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe8 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe9 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xea */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xeb */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xec */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xed */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xee */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xef */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf0 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf1 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf2 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf3 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf4 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf5 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf6 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf7 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf8 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf9 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xfa */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xfb */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xfc */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xfd */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xfe */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xff */ STACK_EFFECT_UNDEF, /* --- */
};

PRIVATE uint8_t const stack_effect_f0[256] = {
	/* 0x00 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x01 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x02 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x03 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x04 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x05 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x06 */ STACK_EFFECT(0, 0), /* `ASM_ENDCATCH_N':              `end catch, #<imm8>+1' */
	/* 0x07 */ STACK_EFFECT(0, 0), /* `ASM_ENDFINALLY_N':            `end finally, #<imm8>+1' */
	/* 0x08 */ STACK_EFFECT_UNDEF, /* `ASM16_CALL_KW':               `call top, #<imm8>, const <imm16>' */
	/* 0x09 */ STACK_EFFECT(2, 1), /* `ASM16_CALL_TUPLE_KW':         `call top, pop..., const <imm16>' */
	/* 0x0a */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x0b */ STACK_EFFECT(0, 1), /* `ASM16_PUSH_BND_ARG':          `push bound arg <imm8>' */
	/* 0x0c */ STACK_EFFECT(0, 1), /* `ASM16_PUSH_BND_EXTERN':       `push bound extern <imm16>:<imm16>' */
	/* 0x0d */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x0e */ STACK_EFFECT(0, 1), /* `ASM16_PUSH_BND_GLOBAL':       `push bound global <imm16>' */
	/* 0x0f */ STACK_EFFECT(0, 1), /* `ASM16_PUSH_BND_LOCAL':        `push bound local <imm16>' */
	/* 0x10 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x11 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x12 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x13 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x14 */ STACK_EFFECT(0, 0), /* `ASM32_JMP':                   `jmp <Sdisp32>' */
	/* 0x15 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x16 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x17 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x18 */ STACK_EFFECT(2, 0), /* `ASM_JMP_POP_POP':             `jmp pop, #pop' */
	/* 0x19 */ STACK_EFFECT_UNDEF, /* `ASM16_OPERATOR':              `op top, $<imm16>, #<imm8>' */
	/* 0x1a */ STACK_EFFECT(2, 1), /* `ASM16_OPERATOR_TUPLE':        `op top, $<imm16>, pop' */
	/* 0x1b */ STACK_EFFECT_UNDEF, /* `ASM_CALL_SEQ':                `call top, [#<imm8>]' */
	/* 0x1c */ STACK_EFFECT_UNDEF, /* `ASM_CALL_MAP':                `call top, {#<imm8>*2}' */
	/* 0x1d */ STACK_EFFECT(3, 1), /* `ASM_THISCALL_TUPLE':          `call top, pop, pop...' */
	/* 0x1e */ STACK_EFFECT(0, 0), /* `ASM16_DEL_GLOBAL':            `del global <imm16>' */
	/* 0x1f */ STACK_EFFECT(0, 0), /* `ASM16_DEL_LOCAL':             `del local <imm16>' */
	/* 0x20 */ STACK_EFFECT(3, 1), /* `ASM_CALL_TUPLE_KWDS':         `call top, pop..., pop' */
	/* 0x21 */ STACK_EFFECT_UNDEF, /* `ASM16_LROT':                  `lrot #<imm16>+3' */
	/* 0x22 */ STACK_EFFECT_UNDEF, /* `ASM16_RROT':                  `rrot #<imm16>+3' */
	/* 0x23 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x24 */ STACK_EFFECT(0, 1), /* `ASM16_DUP_N':                 `dup #SP - <imm16> - 2' */
	/* 0x25 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x26 */ STACK_EFFECT(1, 0), /* `ASM16_POP_N':                 `pop #SP - <imm16> - 2' */
	/* 0x27 */ STACK_EFFECT_UNDEF, /* `ASM16_ADJSTACK':              `adjstack #SP +/- <Simm16>' */
	/* 0x28 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x29 */ STACK_EFFECT(0, 1), /* `ASM16_SUPER_THIS_R':          `push super this, ref <imm16>' */
	/* 0x2a */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x2b */ STACK_EFFECT(1, 0), /* `ASM16_POP_STATIC':            `pop static <imm16>' */
	/* 0x2c */ STACK_EFFECT(1, 0), /* `ASM16_POP_EXTERN':            `pop extern <imm16>:<imm16>' */
	/* 0x2d */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x2e */ STACK_EFFECT(1, 0), /* `ASM16_POP_GLOBAL':            `pop global <imm16>' */
	/* 0x2f */ STACK_EFFECT(1, 0), /* `ASM16_POP_LOCAL':             `pop local <imm16>' */
	/* 0x30 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x31 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x32 */ STACK_EFFECT(0, 1), /* `ASM_PUSH_EXCEPT':             `push except' */
	/* 0x33 */ STACK_EFFECT(0, 1), /* `ASM_PUSH_THIS':               `push this' */
	/* 0x34 */ STACK_EFFECT(0, 1), /* `ASM_PUSH_THIS_MODULE':        `push this_module' */
	/* 0x35 */ STACK_EFFECT(0, 1), /* `ASM_PUSH_THIS_FUNCTION':      `push this_function' */
	/* 0x36 */ STACK_EFFECT(0, 1), /* `ASM16_PUSH_MODULE':           `push module <imm16>' */
	/* 0x37 */ STACK_EFFECT(0, 1), /* `ASM16_PUSH_ARG':              `push arg <imm16>' */
	/* 0x38 */ STACK_EFFECT(0, 1), /* `ASM16_PUSH_CONST':            `push const <imm16>' */
	/* 0x39 */ STACK_EFFECT(0, 1), /* `ASM16_PUSH_REF':              `push ref <imm16>' */
	/* 0x3a */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x3b */ STACK_EFFECT(0, 1), /* `ASM16_PUSH_STATIC':           `push static <imm16>' */
	/* 0x3c */ STACK_EFFECT(0, 0), /* `ASM16_PUSH_EXTERN':           `push extern <imm16>:<imm16>' */
	/* 0x3d */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x3e */ STACK_EFFECT(0, 1), /* `ASM16_PUSH_GLOBAL':           `push global <imm16>' */
	/* 0x3f */ STACK_EFFECT(0, 1), /* `ASM16_PUSH_LOCAL':            `push local <imm16>' */
	/* 0x40 */ STACK_EFFECT(1, 1), /* `ASM_CAST_HASHSET':            `cast top, HashSet' */
	/* 0x41 */ STACK_EFFECT(1, 1), /* `ASM_CAST_DICT':               `cast top, Dict' */
	/* 0x42 */ STACK_EFFECT_UNDEF, /* `ASM16_PACK_TUPLE':            `push pack Tuple, #<imm16>' */
	/* 0x43 */ STACK_EFFECT_UNDEF, /* `ASM16_PACK_LIST':             `push pack List, #<imm16>' */
	/* 0x44 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x45 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x46 */ STACK_EFFECT_UNDEF, /* `ASM16_UNPACK':                `unpack pop, #<imm16>' */
	/* 0x47 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x48 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x49 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x4a */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x4b */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x4c */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x4d */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x4e */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x4f */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x50 */ STACK_EFFECT(0, 1), /* `ASM_PUSH_TRUE':               `push true' */
	/* 0x51 */ STACK_EFFECT(0, 1), /* `ASM_PUSH_FALSE':              `push false' */
	/* 0x52 */ STACK_EFFECT_UNDEF, /* `ASM_PACK_HASHSET':            `push pack HashSet, #<imm8>' */
	/* 0x53 */ STACK_EFFECT_UNDEF, /* `ASM_PACK_DICT':               `push pack Dict, #<imm8>*2' */
	/* 0x54 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x55 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x56 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x57 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x58 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x59 */ STACK_EFFECT(2, 1), /* `ASM_BOUNDITEM':               `bounditem top, pop' */
	/* 0x5a */ STACK_EFFECT(1, 1), /* `ASM16_GETATTR_C':             `getattr top, const <imm16>' */
	/* 0x5b */ STACK_EFFECT(1, 0), /* `ASM16_DELATTR_C':             `delattr pop, const <imm16>' */
	/* 0x5c */ STACK_EFFECT(2, 0), /* `ASM16_SETATTR_C':             `setattr pop, const <imm16>, pop' */
	/* 0x5d */ STACK_EFFECT(0, 1), /* `ASM16_GETATTR_THIS_C':        `push getattr this, const <imm16>' */
	/* 0x5e */ STACK_EFFECT(0, 0), /* `ASM16_DELATTR_THIS_C':        `delattr this, const <imm16>' */
	/* 0x5f */ STACK_EFFECT(1, 0), /* `ASM16_SETATTR_THIS_C':        `setattr this, const <imm16>, pop' */
	/* 0x60 */ STACK_EFFECT(2, 1), /* `ASM_CMP_SO':                  `cmp so, top, pop' */
	/* 0x61 */ STACK_EFFECT(2, 1), /* `ASM_CMP_DO':                  `cmp do, top, pop' */
	/* 0x62 */ STACK_EFFECT_UNDEF, /* `ASM16_PACK_HASHSET':          `push pack HashSet, #<imm16>' */
	/* 0x63 */ STACK_EFFECT_UNDEF, /* `ASM16_PACK_DICT':             `push pack Dict, #<imm16>*2' */
	/* 0x64 */ STACK_EFFECT(1, 1), /* `ASM16_GETCMEMBER':            `getcmember top, $<imm16>' */
	/* 0x65 */ STACK_EFFECT(2, 1), /* `ASM_CLASS':                   `class top, pop' */
	/* 0x66 */ STACK_EFFECT(1, 1), /* `ASM16_CLASS_C':               `class top, const <imm16>' */
	/* 0x67 */ STACK_EFFECT(0, 1), /* `ASM16_CLASS_GC':              `push class global <imm16>, const <imm16>' */
	/* 0x68 */ STACK_EFFECT(0, 1), /* `ASM16_CLASS_EC':              `push class extern <imm16>:<imm16>, const <imm16>' */
	/* 0x69 */ STACK_EFFECT(2, 1), /* `ASM16_DEFCMEMBER':            `defcmember top, $<imm16>, pop' */
	/* 0x6a */ STACK_EFFECT(0, 1), /* `ASM16_GETCMEMBER_R':          `push getcmember ref <imm16>, $<imm16>' */
	/* 0x6b */ STACK_EFFECT_UNDEF, /* `ASM16_CALLCMEMBER_THIS_R':    `push callcmember this, ref <imm16>, $<imm16>, #<imm8>' */
	/* 0x6c */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x6d */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x6e */ STACK_EFFECT_UNDEF, /* `ASM16_FUNCTION_C':            `push function const <imm16>, #<imm8>+1' */
	/* 0x6f */ STACK_EFFECT_UNDEF, /* `ASM16_FUNCTION_C_16':         `push function const <imm16>, #<imm16>+1' */
	/* 0x70 */ STACK_EFFECT(0, 1), /* `ASM_SUPERGETATTR_THIS_RC':    `push getattr this, ref <imm8>, const <imm8>' */
	/* 0x71 */ STACK_EFFECT(0, 1), /* `ASM16_SUPERGETATTR_THIS_RC':  `push getattr this, ref <imm16>, const <imm16>' */
	/* 0x72 */ STACK_EFFECT_UNDEF, /* `ASM_SUPERCALLATTR_THIS_RC':   `push callattr this, ref <imm8>, const <imm8>, #<imm8>' */
	/* 0x73 */ STACK_EFFECT_UNDEF, /* `ASM16_SUPERCALLATTR_THIS_RC': `push callattr this, ref <imm16>, const <imm16>, #<imm8>' */
	/* 0x74 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x75 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x76 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x77 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x78 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x79 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x7a */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x7b */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x7c */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x7d */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x7e */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x7f */ STACK_EFFECT(0, 1), /* `ASM_INCPOST':                 `push inc' */
	/* 0x80 */ STACK_EFFECT(0, 1), /* `ASM_DECPOST':                 `push dec' */
	/* 0x81 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x82 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x83 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x84 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x85 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x86 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x87 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x88 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x89 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x8a */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x8b */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x8c */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x8d */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x8e */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x8f */ STACK_EFFECT(0, 0), /* `ASM16_DELOP':                 `---' */
	/* 0x90 */ STACK_EFFECT(0, 0), /* `ASM16_NOP':                   `nop16' */
	/* 0x91 */ STACK_EFFECT(1, 1), /* `ASM_REDUCE_MIN':              `reduce top, min' */
	/* 0x92 */ STACK_EFFECT(1, 1), /* `ASM_REDUCE_MAX':              `reduce top, max' */
	/* 0x93 */ STACK_EFFECT(1, 1), /* `ASM_REDUCE_SUM':              `reduce top, sum' */
	/* 0x94 */ STACK_EFFECT(1, 1), /* `ASM_REDUCE_ANY':              `reduce top, any' */
	/* 0x95 */ STACK_EFFECT(1, 1), /* `ASM_REDUCE_ALL':              `reduce top, all' */
	/* 0x96 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x97 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x98 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x99 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x9a */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x9b */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x9c */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x9d */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x9e */ STACK_EFFECT_UNDEF, /* --- */
	/* 0x9f */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xa0 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xa1 */ STACK_EFFECT(0, 0), /* `ASM16_PRINT_C':               `print const <imm16>' */
	/* 0xa2 */ STACK_EFFECT(0, 0), /* `ASM16_PRINT_C_SP':            `print const <imm16>, sp' */
	/* 0xa3 */ STACK_EFFECT(0, 0), /* `ASM16_PRINT_C_NL':            `print const <imm16>, nl' */
	/* 0xa4 */ STACK_EFFECT(0, 1), /* `ASM_RANGE_0_I32':             `push range $0, $<imm32>' */
	/* 0xa5 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xa6 */ STACK_EFFECT_UNDEF, /* `ASM_VARARGS_UNPACK':          `unpack varargs, #<imm8>' */
	/* 0xa7 */ STACK_EFFECT(0, 1), /* `ASM_PUSH_VARKWDS_NE':         `push bool varkwds' */
	/* 0xa8 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xa9 */ STACK_EFFECT(1, 1), /* `ASM16_FPRINT_C':              `print top, const <imm16>' */
	/* 0xaa */ STACK_EFFECT(1, 1), /* `ASM16_FPRINT_C_SP':           `print top, const <imm16>, sp' */
	/* 0xab */ STACK_EFFECT(1, 1), /* `ASM16_FPRINT_C_NL':           `print top, const <imm16>, nl' */
	/* 0xac */ STACK_EFFECT(0, 1), /* `ASM_VARARGS_CMP_EQ_SZ':       `push cmp eq, #varargs, $<imm8>' */
	/* 0xad */ STACK_EFFECT(0, 1), /* `ASM_VARARGS_CMP_GR_SZ':       `push cmp gr, #varargs, $<imm8>' */
	/* 0xae */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xaf */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xb0 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xb1 */ STACK_EFFECT(1, 1), /* `ASM16_CONTAINS_C':            `push contains const <imm16>, pop' */
	/* 0xb2 */ STACK_EFFECT(1, 1), /* `ASM_VARARGS_GETITEM':         `getitem varargs, top' */
	/* 0xb3 */ STACK_EFFECT(0, 1), /* `ASM_VARARGS_GETITEM_I':       `push getitem varargs, $<imm8>' */
	/* 0xb4 */ STACK_EFFECT(1, 1), /* `ASM16_GETITEM_C':             `getitem top, const <imm16>' */
	/* 0xb5 */ STACK_EFFECT(0, 1), /* `ASM_VARARGS_GETSIZE':         `push getsize varargs' */
	/* 0xb6 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xb7 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xb8 */ STACK_EFFECT(2, 0), /* `ASM16_SETITEM_C':             `setitem pop, const <imm16>, pop' */
	/* 0xb9 */ STACK_EFFECT(1, 1), /* `ASM_ITERNEXT':                `iternext top' */
	/* 0xba */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xbb */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xbc */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xbd */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xbe */ STACK_EFFECT(2, 1), /* `ASM_GETMEMBER':               `getmember top, pop, $<imm8>' */
	/* 0xbf */ STACK_EFFECT(2, 1), /* `ASM16_GETMEMBER':             `getmember top, pop, $<imm16>' */
	/* 0xc0 */ STACK_EFFECT(2, 0), /* `ASM_DELMEMBER':               `delmember pop, pop, $<imm8>' */
	/* 0xc1 */ STACK_EFFECT(2, 0), /* `ASM16_DELMEMBER':             `delmember pop, pop, $<imm16>' */
	/* 0xc2 */ STACK_EFFECT(3, 0), /* `ASM_SETMEMBER':               `setmember pop, pop, $<imm8>, pop' */
	/* 0xc3 */ STACK_EFFECT(3, 0), /* `ASM16_SETMEMBER':             `setmember pop, pop, $<imm16>, pop' */
	/* 0xc4 */ STACK_EFFECT(2, 1), /* `ASM_BOUNDMEMBER':             `boundmember top, pop, $<imm8>' */
	/* 0xc5 */ STACK_EFFECT(2, 1), /* `ASM16_BOUNDMEMBER':           `boundmember top, pop, $<imm16>' */
	/* 0xc6 */ STACK_EFFECT(1, 1), /* `ASM_GETMEMBER_THIS':          `push getmember this, pop, $<imm8>' */
	/* 0xc7 */ STACK_EFFECT(1, 1), /* `ASM16_GETMEMBER_THIS':        `push getmember this, pop, $<imm16>' */
	/* 0xc8 */ STACK_EFFECT(1, 0), /* `ASM_DELMEMBER_THIS':          `delmember this, pop, $<imm8>' */
	/* 0xc9 */ STACK_EFFECT(1, 0), /* `ASM16_DELMEMBER_THIS':        `delmember this, pop, $<imm16>' */
	/* 0xca */ STACK_EFFECT(2, 0), /* `ASM_SETMEMBER_THIS':          `setmember this, pop, $<imm8>, pop' */
	/* 0xcb */ STACK_EFFECT(2, 0), /* `ASM16_SETMEMBER_THIS':        `setmember this, pop, $<imm16>, pop' */
	/* 0xcc */ STACK_EFFECT(1, 1), /* `ASM_BOUNDMEMBER_THIS':        `push boundmember this, pop, $<imm8>' */
	/* 0xcd */ STACK_EFFECT(1, 1), /* `ASM16_BOUNDMEMBER_THIS':      `push boundmember this, pop, $<imm16>' */
	/* 0xce */ STACK_EFFECT_UNDEF, /* `ASM16_CALLATTR_C_KW':         `callattr top, const <imm16>, #<imm8>, const <imm16>' */
	/* 0xcf */ STACK_EFFECT(2, 1), /* `ASM16_CALLATTR_C_TUPLE_KW':   `callattr top, const <imm16>, pop..., const <imm16>' */
	/* 0xd0 */ STACK_EFFECT_UNDEF, /* `ASM_CALLATTR_KWDS':           `callattr top, pop, #<imm8>, pop' */
	/* 0xd1 */ STACK_EFFECT(4, 1), /* `ASM_CALLATTR_TUPLE_KWDS':     `callattr top, pop, pop..., pop' */
	/* 0xd2 */ STACK_EFFECT_UNDEF, /* `ASM16_CALLATTR_C':            `callattr top, const <imm16>, #<imm8>' */
	/* 0xd3 */ STACK_EFFECT(2, 1), /* `ASM16_CALLATTR_C_TUPLE':      `callattr top, const <imm16>, pop' */
	/* 0xd4 */ STACK_EFFECT_UNDEF, /* `ASM16_CALLATTR_THIS_C':       `callattr this, const <imm16>, #<imm8>' */
	/* 0xd5 */ STACK_EFFECT(1, 1), /* `ASM16_CALLATTR_THIS_C_TUPLE': `callattr this, const <imm16>, pop' */
	/* 0xd6 */ STACK_EFFECT_UNDEF, /* `ASM16_CALLATTR_C_SEQ':        `callattr top, const <imm16>, [#<imm8>]' */
	/* 0xd7 */ STACK_EFFECT_UNDEF, /* `ASM16_CALLATTR_C_MAP':        `callattr top, const <imm16>, {#<imm8>*2}' */
	/* 0xd8 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xd9 */ STACK_EFFECT(0, 1), /* `ASM16_GETMEMBER_THIS_R':      `push getmember this, ref <imm16>, $<imm16>' */
	/* 0xda */ STACK_EFFECT(0, 0), /* `ASM16_DELMEMBER_THIS_R':      `delmember this, ref <imm16>, $<imm16>' */
	/* 0xdb */ STACK_EFFECT(1, 0), /* `ASM16_SETMEMBER_THIS_R':      `setmember this, ref <imm16>, $<imm16>, pop' */
	/* 0xdc */ STACK_EFFECT(0, 1), /* `ASM16_BOUNDMEMBER_THIS_R':    `push boundmember this, ref <imm16>, $<imm16>' */
	/* 0xdd */ STACK_EFFECT_UNDEF, /* `ASM16_CALL_EXTERN':           `push call extern <imm16>:<imm16>, #<imm8>' */
	/* 0xde */ STACK_EFFECT_UNDEF, /* `ASM16_CALL_GLOBAL':           `push call global <imm16>, #<imm8>' */
	/* 0xdf */ STACK_EFFECT_UNDEF, /* `ASM16_CALL_LOCAL':            `push call local <imm16>, #<imm8>' */
	/* 0xe0 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe1 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe2 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe3 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe4 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe5 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe6 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe7 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe8 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xe9 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xea */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xeb */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xec */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xed */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xee */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xef */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf0 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf1 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf2 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf3 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf4 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf5 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf6 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf7 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf8 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xf9 */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xfa */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xfb */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xfc */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xfd */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xfe */ STACK_EFFECT_UNDEF, /* --- */
	/* 0xff */ STACK_EFFECT_UNDEF, /* --- */
};
//[[[end]]]

PRIVATE uint8_t const prefix_length[8] = {
	/* [0xf8       - ASM_PREFIXMIN] */ 1,
	/* [0xf9       - ASM_PREFIXMIN] */ 1,
	/* [ASM_STACK  - ASM_PREFIXMIN] */ 2,
	/* [ASM_STATIC - ASM_PREFIXMIN] */ 2,
	/* [ASM_EXTERN - ASM_PREFIXMIN] */ 3,
	/* [0xfd       - ASM_PREFIXMIN] */ 1,
	/* [ASM_GLOBAL - ASM_PREFIXMIN] */ 2,
	/* [ASM_LOCAL  - ASM_PREFIXMIN] */ 2
};

PRIVATE uint8_t const prefix_length_f0[8] = {
	/* [0xf0f8       - 0xf000 | ASM_PREFIXMIN] */ 2,
	/* [0xf0f9       - 0xf000 | ASM_PREFIXMIN] */ 2,
	/* [ASM16_STACK  - 0xf000 | ASM_PREFIXMIN] */ 4,
	/* [ASM16_STATIC - 0xf000 | ASM_PREFIXMIN] */ 4,
	/* [ASM16_EXTERN - 0xf000 | ASM_PREFIXMIN] */ 6,
	/* [0xf0fd       - 0xf000 | ASM_PREFIXMIN] */ 2,
	/* [ASM16_GLOBAL - 0xf000 | ASM_PREFIXMIN] */ 4,
	/* [ASM16_LOCAL  - 0xf000 | ASM_PREFIXMIN] */ 4
};


/* Return a pointer to the instruction following `pc' */
PUBLIC ATTR_RETNONNULL WUNUSED NONNULL((1)) instruction_t *DCALL
DeeAsm_NextInstr(instruction_t const *__restrict pc) {
	uint8_t length;
again:
	length = intr_len[*pc];
	if unlikely(!length) {
		/* Prefix instruction. */
		if (*pc >= ASM_PREFIXMIN) {
			pc += prefix_length[*pc - ASM_PREFIXMIN];
			goto again;
		}
		/* Extended instruction sets. */
		if (*pc == ASM_EXTENDED1) {
			if (pc[1] >= ASM_PREFIXMIN) {
				pc += prefix_length_f0[pc[1] - ASM_PREFIXMIN];
				goto again;
			}
			/* First extended instruction set. */
			length = intr_len_f0[pc[1]];
		} else {
			/* Fallback: advance by one. */
			length = 1;
		}
	}
	return (instruction_t *)(pc + length);
}



/* Return if the given `instr' doesn't return normally (i.e. doesn't fall
 * through to its successor instruction) when executed in a context where
 * `code_flags' (which is the set of CODE_F* flags) is active. */
PUBLIC WUNUSED bool DCALL
DeeAsm_IsNoreturn(uint16_t instr, uint16_t code_flags) {
	bool result = false;
	switch (instr) {

	case ASM_RET:
		/* In yielding code, the `ret' instruction does actually
		 * return, since it would actually be the `yield' instruction. */
		if (code_flags & CODE_FYIELDING)
			break;
		ATTR_FALLTHROUGH
	case ASM_RET_NONE:
	case ASM_THROW:
	case ASM_RETHROW:
	case ASM_JMP:
	case ASM_JMP16:
	case ASM32_JMP:
	case ASM_JMP_POP:
	case ASM_JMP_POP_POP:
		result = true;
		break;

	default: break;
	}
	return result;
}

PUBLIC ATTR_RETNONNULL NONNULL((1, 2)) instruction_t *DCALL
DeeAsm_NextInstrSp(instruction_t const *__restrict pc,
                   uint16_t *__restrict pstacksz) {
	uint16_t sp_add, sp_sub;
	return DeeAsm_NextInstrEf(pc, pstacksz, &sp_add, &sp_sub);
}

PUBLIC ATTR_RETNONNULL NONNULL((1, 2, 3, 4)) instruction_t *DCALL
DeeAsm_NextInstrEf(instruction_t const *__restrict pc,
                   uint16_t *__restrict pstacksz,
                   uint16_t *__restrict psp_add,
                   uint16_t *__restrict psp_sub) {
	instruction_t op = *pc;
	instruction_t const *prefix_ip;
	uint16_t prefix_stack_sub;
#ifndef NDEBUG
	uint16_t old_stacksz = *pstacksz;
#endif /* !NDEBUG */
	switch (op) {

	case ASM_ADJSTACK: {
		int8_t effect;
		effect = *(int8_t *)(pc + 1);
		*pstacksz += effect;
		if (effect >= 0) {
			*psp_add = (uint16_t)effect;
			*psp_sub = 0;
		} else {
			*psp_add = 0;
			*psp_sub = (uint16_t)-effect;
		}
	}	break;

	case ASM_LROT:
	case ASM_RROT:
		*psp_add = *psp_sub = (uint16_t)(*(uint8_t *)(pc + 1) + 3);
		break;

	case ASM_DUP_N:
		*psp_sub = (uint16_t)(*(uint8_t *)(pc + 1) + 2);
		*psp_add = *psp_sub + 1;
		*pstacksz += 1;
		break;

	case ASM_POP_N:
		*psp_add = (uint16_t)(*(uint8_t *)(pc + 1) + 1);
		*psp_sub = *psp_add + 1;
		*pstacksz -= 1;
		break;

	case ASM_PACK_TUPLE:
	case ASM_PACK_LIST:
		*psp_add = 1;
		*psp_sub = *(uint8_t *)(pc + 1);
		*pstacksz -= *(uint8_t *)(pc + 1);
		++*pstacksz;
		break;

	case ASM_CALL:
	case ASM_CALL_KW:
	case ASM_EXTEND:
		*psp_add = 1;
		*psp_sub = 1 + *(uint8_t *)(pc + 1);
		*pstacksz -= *(uint8_t *)(pc + 1);
		break;

	case ASM_CALLCMEMBER_THIS_R:
		*psp_add = 1;
		*psp_sub = *(uint8_t *)(pc + 3);
		*pstacksz -= *(uint8_t *)(pc + 3);
		++*pstacksz;
		break;

	case ASM_UNPACK:
		*psp_sub = 1;
		*psp_add = *(uint8_t *)(pc + 1);
		*pstacksz += *(uint8_t *)(pc + 1);
		--*pstacksz;
		break;

	case ASM_FUNCTION_C_16:
		*psp_add = 1;
		*psp_sub = 1 + UNALIGNED_GETLE16((uint16_t *)(pc + 2));
		*pstacksz -= (*psp_sub - 1);
		break;

	case ASM_CALLATTR:
		*psp_add = 1;
		*psp_sub = 2 + *(uint8_t *)(pc + 1);
		*pstacksz -= 1 + *(uint8_t *)(pc + 1);
		break;

	case ASM_CALLATTR_C_KW:
	case ASM_FUNCTION_C:
	case ASM_CALLATTR_C:
	case ASM_CALLATTR_C_SEQ:
		*psp_add = 1;
		*psp_sub = 1 + *(uint8_t *)(pc + 2);
		*pstacksz -= (*psp_sub - 1);
		break;

	case ASM_CALLATTR_C_MAP:
		*psp_add = 1;
		*psp_sub = 1 + (uint16_t)(*(uint8_t *)(pc + 2) * 2);
		*pstacksz -= (*psp_sub - 1);
		break;

	case ASM_CALLATTR_THIS_C:
	case ASM_CALL_GLOBAL:
	case ASM_CALL_LOCAL:
		*psp_add = 1;
		*psp_sub = *(uint8_t *)(pc + 2);
		*pstacksz -= *(uint8_t *)(pc + 2);
		*pstacksz += 1;
		break;

	case ASM_CALL_EXTERN:
		*psp_add = 1;
		*psp_sub = *(uint8_t *)(pc + 3);
		*pstacksz -= *(uint8_t *)(pc + 3);
		*pstacksz += 1;
		break;

	case ASM_OPERATOR:
		*psp_add = 1;
		*psp_sub = 1 + *(uint8_t *)(pc + 2);
		*pstacksz -= *(uint8_t *)(pc + 2);
		break;

	case ASM_EXTENDED1:
		op = pc[1];
		switch (op) {

		case ASM16_ADJSTACK & 0xff: {
			int16_t effect;
			effect = (int16_t)UNALIGNED_GETLE16((uint16_t *)(pc + 2));
			*pstacksz += effect;
			if (effect >= 0) {
				*psp_add = (uint16_t)effect;
				*psp_sub = 0;
			} else {
				*psp_add = 0;
				*psp_sub = (uint16_t)-effect;
			}
		}	break;

		case ASM16_LROT & 0xff:
		case ASM16_RROT & 0xff:
			*psp_add = *psp_sub = (uint16_t)(UNALIGNED_GETLE16((uint16_t *)(pc + 2)) + 3);
			break;

		case ASM16_DUP_N & 0xff:
			*psp_sub = (uint16_t)(UNALIGNED_GETLE16((uint16_t *)(pc + 2)) + 2);
			*psp_add = *psp_sub + 1;
			*pstacksz += 1;
			break;

		case ASM16_POP_N & 0xff:
			*psp_add = (uint16_t)(UNALIGNED_GETLE16((uint16_t *)(pc + 2)) + 1);
			*psp_sub = *psp_add + 1;
			*pstacksz -= 1;
			break;

		case ASM16_PACK_TUPLE & 0xff:
		case ASM16_PACK_LIST & 0xff:
		case ASM16_PACK_HASHSET & 0xff:
			*psp_add = 1;
			*psp_sub = UNALIGNED_GETLE16((uint16_t *)(pc + 2));
			*pstacksz -= *psp_sub;
			*pstacksz += 1;
			break;

		case ASM16_UNPACK & 0xff:
			*psp_sub = 1;
			*psp_add = UNALIGNED_GETLE16((uint16_t *)(pc + 2));
			*pstacksz += *psp_add;
			*pstacksz -= 1;
			break;

		case ASM16_FUNCTION_C & 0xff:
			*psp_add = 1;
			*psp_sub = 1 + *(uint8_t *)(pc + 4);
			*pstacksz -= *(uint8_t *)(pc + 4);
			break;

		case ASM16_FUNCTION_C_16 & 0xff:
			*psp_add = 1;
			*psp_sub = 1 + UNALIGNED_GETLE16((uint16_t *)(pc + 4));
			*pstacksz -= (*psp_sub - 1);
			break;

		case ASM_CALLATTR_KWDS & 0xff:
			*psp_add = 1;
			*psp_sub = 3 + *(uint8_t *)(pc + 2);
			*pstacksz -= 2 + *(uint8_t *)(pc + 2);
			break;

		case ASM16_CALLATTR_C_KW & 0xff:
		case ASM16_CALLATTR_C & 0xff:
		case ASM16_CALLATTR_C_SEQ & 0xff:
			*psp_add = 1;
			*psp_sub = 1 + *(uint8_t *)(pc + 4);
			*pstacksz -= *(uint8_t *)(pc + 4);
			break;

		case ASM16_CALLATTR_C_MAP & 0xff:
			*psp_add = 1;
			*psp_sub = 1 + (uint16_t)(*(uint8_t *)(pc + 4) * 2);
			*pstacksz -= *(uint8_t *)(pc + 4);
			break;

		case ASM16_CALLATTR_THIS_C & 0xff:
		case ASM16_CALL_GLOBAL & 0xff:
		case ASM16_CALL_LOCAL & 0xff:
			*psp_add = 1;
			*psp_sub = *(uint8_t *)(pc + 4);
			*pstacksz -= *(uint8_t *)(pc + 4);
			*pstacksz += 1;
			break;

		case ASM16_CALL_EXTERN & 0xff:
			*psp_add = 1;
			*psp_sub = *(uint8_t *)(pc + 6);
			*pstacksz -= *(uint8_t *)(pc + 6);
			*pstacksz += 1;
			break;

		case ASM16_CALL_KW & 0xff:
		case ASM_CALL_SEQ & 0xff:
			*psp_add = 1;
			*psp_sub = 1 + *(uint8_t *)(pc + 2);
			*pstacksz -= *(uint8_t *)(pc + 2);
			break;

		case ASM16_CALLCMEMBER_THIS_R & 0xff:
			*psp_add = 1;
			*psp_sub = *(uint8_t *)(pc + 6);
			*pstacksz -= *(uint8_t *)(pc + 6);
			++*pstacksz;
			break;

		case ASM_CALL_MAP & 0xff:
			*psp_add = 1;
			*psp_sub = 1 + ((uint16_t) * (uint8_t *)(pc + 2) * 2);
			*pstacksz -= ((uint16_t) * (uint8_t *)(pc + 2) * 2);
			break;

		case ASM16_OPERATOR & 0xff:
			*psp_add = 1;
			*psp_sub = 1 + *(uint8_t *)(pc + 4);
			*pstacksz -= *(uint8_t *)(pc + 4);
			break;

		case ASM_PACK_HASHSET & 0xff:
			*psp_add = 1;
			*psp_sub = *(uint8_t *)(pc + 2);
			*pstacksz -= *(uint8_t *)(pc + 2);
			*pstacksz += 1;
			break;

		case ASM_PACK_DICT & 0xff:
			*psp_add = 1;
			*psp_sub = *(uint8_t *)(pc + 2) * 2;
			*pstacksz -= *(uint8_t *)(pc + 2) * 2;
			*pstacksz += 1;
			break;

		case ASM16_PACK_DICT & 0xff:
			*psp_add = 1;
			*psp_sub = UNALIGNED_GETLE16((uint16_t *)(pc + 2)) * 2;
			*pstacksz -= *psp_sub;
			*pstacksz += 1;
			break;

		case ASM_VARARGS_UNPACK & 0xff:
			*psp_add = *(uint8_t *)(pc + 2);
			*psp_sub = 0;
			*pstacksz += *psp_add;
			break;

		case ASM_SUPERCALLATTR_THIS_RC & 0xff:
			*psp_add = 1;
			*psp_sub = *(uint8_t *)(pc + 4);
			*pstacksz += 1;
			*pstacksz -= *psp_sub;
			break;

		case ASM16_SUPERCALLATTR_THIS_RC & 0xff:
			*psp_add = 1;
			*psp_sub = *(uint8_t *)(pc + 6);
			*pstacksz += 1;
			*pstacksz -= *psp_sub;
			break;

		case ASM16_STACK & 0xff: {
			uint16_t sp_addr;
			prefix_ip = pc + 4;
			sp_addr   = UNALIGNED_GETLE16((uint16_t *)(pc + 2));
			/* Calculate the relative stack effect for addressing this stack slot. */
			prefix_stack_sub = (*pstacksz - sp_addr);
			goto do_prefix;
		}

		case ASM16_EXTERN & 0xff:
			prefix_ip = pc + 6;
			goto do_prefix_nosp;

		case ASM16_STATIC & 0xff:
		case ASM16_GLOBAL & 0xff:
		case ASM16_LOCAL & 0xff:
			prefix_ip = pc + 4;
			goto do_prefix_nosp;

		default:
			*psp_sub = STACK_EFFECT_DOWN(stack_effect_f0[op]);
			*psp_add = STACK_EFFECT_UP(stack_effect_f0[op]);
			*pstacksz += STACK_EFFECT_SUM(stack_effect_f0[op]);
			break;
		}
		break;

	case ASM_STACK: {
		uint8_t sp_addr;
		prefix_ip = pc + 2;
		sp_addr   = *(uint8_t *)(pc + 1);
		/* Calculate the relative stack effect for addressing this stack slot. */
		prefix_stack_sub = (*pstacksz - sp_addr);
		goto do_prefix;
	}

	case ASM_EXTERN:
		prefix_ip = pc + 3;
		goto do_prefix_nosp;

	case ASM_STATIC:
	case ASM_GLOBAL:
	case ASM_LOCAL:
		prefix_ip = pc + 2;
do_prefix_nosp:
		prefix_stack_sub = 0;
do_prefix:
		op = prefix_ip[0];
		switch (op) {

		case ASM_ADD:
		case ASM_SUB:
		case ASM_MUL:
		case ASM_DIV:
		case ASM_MOD:
		case ASM_SHL:
		case ASM_SHR:
		case ASM_AND:
		case ASM_OR:
		case ASM_XOR:
		case ASM_POW:
			*psp_add = 0;
			*psp_sub = 1;
			--*pstacksz; /* Inplace arithmetic. e.g.: `local @foo add pop' */
			break;

		case ASM_SWAP:
			*psp_add = 1;
			*psp_sub = 1;
			break;

		case ASM_LROT:
		case ASM_RROT:
			*psp_add = prefix_ip[1];
			*psp_sub = *psp_add;
			break;

		case ASM_DUP:
		case ASM_POP:
			*psp_add = 1;
			*psp_sub = 1;
			break;

		case ASM_DUP_N:
		case ASM_POP_N:
			*psp_add = *(uint8_t *)(prefix_ip + 1);
			*psp_sub = *psp_add;
			break;

		case ASM_OPERATOR:
			*psp_add = 1;
			*psp_sub = *(uint8_t *)(prefix_ip + 2);
			*pstacksz -= *psp_sub;
			*pstacksz += 1;
			break;

		case ASM_OPERATOR_TUPLE:
			*psp_add = 1;
			*psp_sub = 1;
			break;

		case ASM_FUNCTION_C:
			*psp_add = 0;
			*psp_sub = 1 + *(uint8_t *)(prefix_ip + 2);
			*pstacksz -= 1 + *(uint8_t *)(prefix_ip + 2);
			break;

		case ASM_FUNCTION_C_16:
			*psp_add = 0;
			*psp_sub = 1 + UNALIGNED_GETLE16((uint16_t *)(prefix_ip + 2));
			*pstacksz -= *psp_sub;
			break;

		case ASM_UNPACK:
			*psp_add = *(uint8_t *)(prefix_ip + 1);
			*psp_sub = 0;
			*pstacksz += *psp_add;
			break;

		case ASM_EXTENDED1:
			op = prefix_ip[1];
			switch (op) {

			case ASM16_OPERATOR & 0xff:
				*psp_add = 1;
				*psp_sub = 1 + *(uint8_t *)(prefix_ip + 4);
				*pstacksz -= *(uint8_t *)(prefix_ip + 4);
				break;

			case ASM16_FUNCTION_C & 0xff:
				*psp_add = 0;
				*psp_sub = 1 + *(uint8_t *)(prefix_ip + 4);
				*pstacksz -= 1 + *(uint8_t *)(prefix_ip + 4);
				break;

			case ASM16_FUNCTION_C_16 & 0xff:
				*psp_add = 0;
				*psp_sub = 1 + UNALIGNED_GETLE16((uint16_t *)(prefix_ip + 4));
				*pstacksz -= *psp_sub;
				break;

			case ASM_INCPOST & 0xff:
			case ASM_DECPOST & 0xff:
				*psp_add = 1;
				*psp_sub = 0;
				*pstacksz += 1;
				break;

			case ASM16_LROT & 0xff:
			case ASM16_RROT & 0xff:
				*psp_add = UNALIGNED_GETLE16((uint16_t *)(prefix_ip + 2));
				*psp_sub = *psp_add;
				break;

			case ASM16_DUP_N & 0xff:
			case ASM16_POP_N & 0xff:
				*psp_add = UNALIGNED_GETLE16((uint16_t *)(prefix_ip + 2));
				*psp_sub = *psp_add;
				break;

			case ASM16_UNPACK & 0xff:
				*psp_add = UNALIGNED_GETLE16((uint16_t *)(prefix_ip + 2));
				*psp_sub = 0;
				*pstacksz += *psp_add;
				break;

			default:
				*psp_add = 0;
				*psp_sub = 0;
				break;
			}
			break;

		default:
			*psp_add = 0;
			*psp_sub = 0;
			break;
		}
		/* Adjust the add/sub pair to take a stack prefix into account. */
		if (prefix_stack_sub > *psp_sub) {
			*psp_add += (prefix_stack_sub - *psp_sub);
			*psp_sub = prefix_stack_sub;
		}
		break;

	default:
		*psp_sub = STACK_EFFECT_DOWN(stack_effect[op]);
		*psp_add = STACK_EFFECT_UP(stack_effect[op]);
		*pstacksz += STACK_EFFECT_SUM(stack_effect[op]);
		break;
	}

#ifndef NDEBUG
	/* Make sure that the stack was adjusted properly. */
	ASSERT(*pstacksz == (uint16_t)((old_stacksz - *psp_sub) + *psp_add));
#endif /* !NDEBUG */
	return DeeAsm_NextInstr(pc);
}

PRIVATE unsigned int DCALL
prefix_symbol_usage(instruction_t const *__restrict pc) {
	unsigned int result;
	switch (pc[0]) {

	case ASM_ADD:
	case ASM_SUB:
	case ASM_MUL:
	case ASM_DIV:
	case ASM_MOD:
	case ASM_SHL:
	case ASM_SHR:
	case ASM_AND:
	case ASM_OR:
	case ASM_XOR:
	case ASM_POW:
	case ASM_INC:
	case ASM_DEC:
	case ASM_OPERATOR:
	case ASM_OPERATOR_TUPLE:
		/* Inplace operations read & write. */
		result = ASM_USING_READ | ASM_USING_WRITE;
		break;

	case ASM_FUNCTION_C:
	case ASM_FUNCTION_C_16:
		/* These only write to the symbol. */
		result = ASM_USING_WRITE;
		break;

		/* All the new mov-style prefix instruction */
	case ASM_DUP:
	case ASM_DUP_N:
	case ASM_PUSH_NONE:
	case ASM_PUSH_MODULE:
	case ASM_PUSH_REF:
	case ASM_PUSH_ARG:
	case ASM_PUSH_CONST:
	case ASM_PUSH_STATIC:
	case ASM_PUSH_EXTERN:
	case ASM_PUSH_GLOBAL:
	case ASM_PUSH_LOCAL:
		result = ASM_USING_WRITE;
		break;

	case ASM_POP:
	case ASM_POP_N:
	case ASM_POP_STATIC:
	case ASM_POP_EXTERN:
	case ASM_POP_GLOBAL:
	case ASM_POP_LOCAL:
	case ASM_RET:
	case ASM_YIELDALL:
	case ASM_THROW:
	case ASM_JT:
	case ASM_JT16:
	case ASM_JF:
	case ASM_JF16:
	case ASM_FOREACH:
	case ASM_FOREACH16:
		result = ASM_USING_READ;
		break;

	case ASM_EXTENDED1:
		switch (pc[1]) {

		case ASM_INCPOST & 0xff:
		case ASM_DECPOST & 0xff:
		case ASM16_OPERATOR & 0xff:
		case ASM16_OPERATOR_TUPLE & 0xff:
			result = ASM_USING_READ | ASM_USING_WRITE;
			break;

		case ASM16_FUNCTION_C & 0xff:
		case ASM16_FUNCTION_C_16 & 0xff:
			/* These only write to the symbol. */
			result = ASM_USING_WRITE;
			break;

			/* All the new mov-style prefix instruction */
		case ASM16_DUP_N & 0xff:
		case ASM16_PUSH_MODULE & 0xff:
		case ASM16_PUSH_REF & 0xff:
		case ASM16_PUSH_ARG & 0xff:
		case ASM16_PUSH_CONST & 0xff:
		case ASM16_PUSH_STATIC & 0xff:
		case ASM16_PUSH_EXTERN & 0xff:
		case ASM16_PUSH_GLOBAL & 0xff:
		case ASM16_PUSH_LOCAL & 0xff:
		case ASM_PUSH_EXCEPT & 0xff:
		case ASM_PUSH_THIS & 0xff:
		case ASM_PUSH_THIS_MODULE & 0xff:
		case ASM_PUSH_THIS_FUNCTION & 0xff:
		case ASM_PUSH_TRUE & 0xff:
		case ASM_PUSH_FALSE & 0xff:
			result = ASM_USING_WRITE;
			break;

		case ASM16_POP_N & 0xff:
		case ASM16_POP_STATIC & 0xff:
		case ASM16_POP_EXTERN & 0xff:
		case ASM16_POP_GLOBAL & 0xff:
		case ASM16_POP_LOCAL & 0xff:
			result = ASM_USING_READ;
			break;

		default:
			result = ASM_USING_READ | ASM_USING_WRITE;
			break;
		}
		break;

	default:
		result = ASM_USING_READ | ASM_USING_WRITE;
		break;
	}
	return result;
}


INTERN WUNUSED NONNULL((1)) unsigned int DCALL
asm_uses_local(instruction_t const *__restrict pc, uint16_t lid) {
	unsigned int result = 0;
	switch (pc[0]) {

	case ASM_LOCAL:
		if (*(uint8_t *)(pc + 1) == lid)
			result = prefix_symbol_usage(pc + 2);
		pc += 2;
		goto check_prefix_core_usage;

	case ASM_PUSH_BND_LOCAL:
	case ASM_PUSH_LOCAL:
	case ASM_CALL_LOCAL:
		if (*(uint8_t *)(pc + 1) != lid)
			break;
		result = ASM_USING_READ;
		break;

	case ASM_DEL_LOCAL:
	case ASM_POP_LOCAL:
		if (*(uint8_t *)(pc + 1) != lid)
			break;
		result = ASM_USING_WRITE;
		break;

	case ASM_EXTENDED1:
		switch (pc[1]) {

		case ASM16_PUSH_BND_LOCAL & 0xff:
		case ASM16_PUSH_LOCAL & 0xff:
		case ASM16_CALL_LOCAL & 0xff:
			if (UNALIGNED_GETLE16((uint16_t *)(pc + 2)) != lid)
				break;
			result = ASM_USING_READ;
			break;

		case ASM16_DEL_LOCAL & 0xff:
		case ASM16_POP_LOCAL & 0xff:
			if (UNALIGNED_GETLE16((uint16_t *)(pc + 2)) != lid)
				break;
			result = ASM_USING_WRITE;
			break;

		case ASM16_LOCAL & 0xff:
			if (UNALIGNED_GETLE16((uint16_t *)(pc + 2)) == lid)
				result = prefix_symbol_usage(pc + 4);
			pc += 4;
			goto check_prefix_core_usage;

		case ASM16_STACK & 0xff:
		case ASM16_STATIC & 0xff:
		case ASM16_EXTERN & 0xff:
		case ASM16_GLOBAL & 0xff:
			pc += 4;
			goto check_prefix_core_usage;

		default: break;
		}
		break;

	case ASM_STACK:
	case ASM_STATIC:
	case ASM_EXTERN:
	case ASM_GLOBAL:
check_prefix_core_usage:
		switch (*pc) {

		case ASM_PUSH_LOCAL: /* mov PREFIX, local */
			if (*(uint8_t *)(pc + 1) == lid)
				result |= ASM_USING_READ;
			break;

		case ASM_POP_LOCAL: /* mov local, PREFIX */
			if (*(uint8_t *)(pc + 1) == lid)
				result |= ASM_USING_WRITE;
			break;

		case ASM_EXTENDED1:
			switch (pc[1]) {

			case ASM16_PUSH_LOCAL & 0xff: /* mov PREFIX, local */
				if (UNALIGNED_GETLE16((uint16_t *)(pc + 2)) == lid)
					result |= ASM_USING_READ;
				break;

			case ASM16_POP_LOCAL & 0xff: /* mov local, PREFIX */
				if (UNALIGNED_GETLE16((uint16_t *)(pc + 2)) == lid)
					result |= ASM_USING_WRITE;
				break;

			default: break;
			}
			break;

		default: break;
		}
		break;

	default: break;
	}
	return result;
}



INTERN WUNUSED NONNULL((1)) unsigned int DCALL
asm_uses_static(instruction_t const *__restrict pc, uint16_t sid) {
	/* NOTE: This function also allows for tracking of constant variable usage. */
	unsigned int result = 0;
	switch (pc[0]) {

	case ASM_STATIC:
		if (*(uint8_t *)(pc + 1) != sid)
			break;
		result = prefix_symbol_usage(pc + 2);
		result |= asm_uses_static(pc + 2, sid);
		pc += 2;
		goto check_prefix_core_usage;

	case ASM_POP_STATIC:
		if (*(uint8_t *)(pc + 1) != sid)
			break;
		result = ASM_USING_WRITE;
		break;

	case ASM_PUSH_STATIC:
	case ASM_PUSH_CONST:
	case ASM_GETATTR_C:
	case ASM_DELATTR_C:
	case ASM_SETATTR_C:
	case ASM_GETATTR_THIS_C:
	case ASM_DELATTR_THIS_C:
	case ASM_SETATTR_THIS_C:
	case ASM_FUNCTION_C:
	case ASM_FUNCTION_C_16:
	case ASM_PRINT_C:
	case ASM_PRINT_C_SP:
	case ASM_PRINT_C_NL:
	case ASM_FPRINT_C:
	case ASM_FPRINT_C_SP:
	case ASM_FPRINT_C_NL:
	case ASM_GETITEM_C:
	case ASM_SETITEM_C:
	case ASM_CALLATTR_C:
	case ASM_CALLATTR_C_MAP:
	case ASM_CALLATTR_C_SEQ:
	case ASM_CALLATTR_C_TUPLE:
	case ASM_CALLATTR_THIS_C:
	case ASM_CALLATTR_THIS_C_TUPLE:
		if (*(uint8_t *)(pc + 1) != sid)
			break;
		result = ASM_USING_READ;
		break;

	case ASM_CLASS_C:
		if (*(uint8_t *)(pc + 1) == sid) {
			result = ASM_USING_READ;
			break;
		}
		break;

	case ASM_CLASS_GC:
		if (*(uint8_t *)(pc + 2) == sid) {
			result = ASM_USING_READ;
			break;
		}
		break;

	case ASM_CLASS_EC:
		if (*(uint8_t *)(pc + 3) == sid) {
			result = ASM_USING_READ;
			break;
		}
		break;

	case ASM_EXTENDED1:
		switch (pc[1]) {

		case ASM16_CLASS_C & 0xff:
			if (UNALIGNED_GETLE16((uint16_t *)(pc + 2)) == sid) {
				result = ASM_USING_READ;
				break;
			}
			break;

		case ASM16_CLASS_GC & 0xff:
			if (UNALIGNED_GETLE16((uint16_t *)(pc + 4)) == sid) {
				result = ASM_USING_READ;
				break;
			}
			break;

		case ASM16_CLASS_EC & 0xff:
			if (UNALIGNED_GETLE16((uint16_t *)(pc + 6)) == sid) {
				result = ASM_USING_READ;
				break;
			}
			break;

		case ASM16_STATIC & 0xff:
			if (UNALIGNED_GETLE16((uint16_t *)(pc + 2)) != sid)
				break;
			result = prefix_symbol_usage(pc + 4);
			result |= asm_uses_static(pc + 4, sid);
			pc += 4;
			goto check_prefix_core_usage;

		case ASM16_POP_STATIC & 0xff:
			if (UNALIGNED_GETLE16((uint16_t *)(pc + 2)) != sid)
				break;
			result = ASM_USING_WRITE;
			break;

		case ASM16_PUSH_STATIC & 0xff:
		case ASM16_PUSH_CONST & 0xff:
		case ASM16_GETATTR_C & 0xff:
		case ASM16_DELATTR_C & 0xff:
		case ASM16_SETATTR_C & 0xff:
		case ASM16_GETATTR_THIS_C & 0xff:
		case ASM16_DELATTR_THIS_C & 0xff:
		case ASM16_SETATTR_THIS_C & 0xff:
		case ASM16_FUNCTION_C & 0xff:
		case ASM16_FUNCTION_C_16 & 0xff:
		case ASM16_PRINT_C & 0xff:
		case ASM16_PRINT_C_SP & 0xff:
		case ASM16_PRINT_C_NL & 0xff:
		case ASM16_FPRINT_C & 0xff:
		case ASM16_FPRINT_C_SP & 0xff:
		case ASM16_FPRINT_C_NL & 0xff:
		case ASM16_GETITEM_C & 0xff:
		case ASM16_SETITEM_C & 0xff:
		case ASM16_CALLATTR_C & 0xff:
		case ASM16_CALLATTR_C_MAP & 0xff:
		case ASM16_CALLATTR_C_SEQ & 0xff:
		case ASM16_CALLATTR_C_TUPLE & 0xff:
		case ASM16_CALLATTR_THIS_C & 0xff:
		case ASM16_CALLATTR_THIS_C_TUPLE & 0xff:
			if (UNALIGNED_GETLE16((uint16_t *)(pc + 2)) != sid)
				break;
			result = ASM_USING_READ;
			break;

		case ASM16_STACK & 0xff:
		case ASM16_LOCAL & 0xff:
		case ASM16_EXTERN & 0xff:
		case ASM16_GLOBAL & 0xff:
			pc += 4;
			goto check_prefix_core_usage;

		default: break;
		}
		break;

	case ASM_STACK:
	case ASM_LOCAL:
	case ASM_EXTERN:
	case ASM_GLOBAL:
check_prefix_core_usage:
		switch (*pc) {

		case ASM_PUSH_STATIC: /* mov PREFIX, static */
		case ASM_PUSH_CONST:  /* mov PREFIX, const */
			if (*(uint8_t *)(pc + 1) == sid)
				result |= ASM_USING_READ;
			break;
		case ASM_POP_STATIC: /* mov static, PREFIX */
			if (*(uint8_t *)(pc + 1) == sid)
				result |= ASM_USING_WRITE;
			break;

		case ASM_EXTENDED1:
			switch (pc[1]) {

			case ASM16_PUSH_STATIC & 0xff: /* mov PREFIX, static */
			case ASM16_PUSH_CONST & 0xff:  /* mov PREFIX, const */
				if (UNALIGNED_GETLE16((uint16_t *)(pc + 2)) == sid)
					result |= ASM_USING_READ;
				break;
			case ASM16_POP_STATIC & 0xff: /* mov local, PREFIX */
				if (UNALIGNED_GETLE16((uint16_t *)(pc + 2)) == sid)
					result |= ASM_USING_WRITE;
				break;

			default: break;
			}
			break;

		default: break;
		}
		break;

	default: break;
	}
	return result;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INSTRLEN_C */
