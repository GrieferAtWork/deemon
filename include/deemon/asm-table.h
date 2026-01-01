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
#ifndef DEE_ASM_WANT_TABLE
#define DEE_ASM_WANT_TABLE(prefix_byte_or_0) (prefix_byte_or_0 == 0)
#endif /* !DEE_ASM_WANT_TABLE */
#ifndef DEE_ASM_BEGIN
#define DEE_ASM_BEGIN(table_prefix) /* nothing */
#endif /* !DEE_ASM_BEGIN */
#ifndef DEE_ASM_END
#define DEE_ASM_END(table_prefix) /* nothing */
#endif /* !DEE_ASM_END */
#ifndef DEE_ASM_UNDEFINED
#define DEE_ASM_UNDEFINED(table_prefix, opcode_byte, instr_len) /* nothing */
#endif /* !DEE_ASM_UNDEFINED */
#ifndef DEE_ASM_OPCODE
#define DEE_ASM_OPCODE(table_prefix, opcode_byte, instr_len, name, sp_sub, sp_add, mnemonic) \
	DEE_ASM_UNDEFINED(table_prefix, opcode_byte, instr_len)
#endif /* !DEE_ASM_OPCODE */
#ifndef DEE_ASM_UNDEFINED_PREFIX
#define DEE_ASM_UNDEFINED_PREFIX(table_prefix, opcode_byte, instr_len) \
	DEE_ASM_UNDEFINED(table_prefix, opcode_byte, instr_len)
#endif /* !DEE_ASM_UNDEFINED_PREFIX */
#ifndef DEE_ASM_OPCODE_P
#define DEE_ASM_OPCODE_P(table_prefix, opcode_byte, instr_len, name, sp_sub, sp_add, mnemonic, prefix_sp_sub, prefix_sp_add, prefix_mnemonic) \
	DEE_ASM_OPCODE(table_prefix, opcode_byte, instr_len, name, sp_sub, sp_add, mnemonic)
#endif /* !DEE_ASM_OPCODE_P */
#ifndef DEE_ASM_EXTENDED
#define DEE_ASM_EXTENDED(table_prefix, opcode_byte, instr_len, name) \
	DEE_ASM_UNDEFINED(table_prefix, opcode_byte, instr_len)
#endif /* !DEE_ASM_EXTENDED */
#ifndef DEE_ASM_PREFIX
#define DEE_ASM_PREFIX(table_prefix, opcode_byte, instr_len, name) \
	DEE_ASM_UNDEFINED(table_prefix, opcode_byte, instr_len)
#endif /* !DEE_ASM_PREFIX */


/*[[[deemon
import * from deemon;
import util;
import functools;

#include "asm.h"

global final VERCMP_KEY = functools.predcmp2key((a, b) -> a.vercompare(b));

class Opcode {
	this = default;
	public final member name: string;            // ASM_RET_NONE
	public final member opcode: int;             // 0x00
	public final member length: int;             // 1             (Total length in bytes)
	public final member sp_sub: string;          // SP_SUB0       (Stack effect minus, or none if varying)
	public final member sp_add: string;          // SP_ADD0       (Stack effect plus, or none if varying)
	public final member mnemonic: string;        // "\"ret\""
	public final member prefixed: Opcode | none; // Representation when given a prefix
};

global opcodes: {int: Opcode} = Dict();
global usedSpFormats: {string...} = HashSet();
global usedFFormats: {string...} = HashSet();

function replaceAll(x, y, z) {
	for (;;) {
		local r = x.replace(y, z);
		if (r == x)
			return r;
		x = r;
	}
}

function parseMnemonic(mnemonic: Bytes): string {
	local result = mnemonic.decode("utf-8");
	result = replaceAll(result, " -", "-");
	result = replaceAll(result, "- ", "-");
	result = replaceAll(result, " +", "+");
	result = replaceAll(result, "+ ", "+");
	result = replaceAll(result, "  ", " ");
	result = result.replace("PREFIX", '" F_PREFIX "');
	result = replaceAll(result, "<Sdisp8>", '" F_SDISP8 "');
	result = replaceAll(result, "<Sdisp16>", '" F_SDISP16 "');
	result = replaceAll(result, "<Sdisp32>", '" F_SDISP32 "');
	result = replaceAll(result, "const <imm8>", '" F_CONST8 "');
	result = replaceAll(result, "const <imm16>", '" F_CONST16 "');
	result = replaceAll(result, "static <imm8>", '" F_STATIC8 "');
	result = replaceAll(result, "static <imm16>", '" F_STATIC16 "');
	result = replaceAll(result, "ref <imm8>", '" F_REF8 "');
	result = replaceAll(result, "ref <imm16>", '" F_REF16 "');
	result = replaceAll(result, "arg <imm8>", '" F_ARG8 "');
	result = replaceAll(result, "arg <imm16>", '" F_ARG16 "');
	result = replaceAll(result, "local <imm8>", '" F_LOCAL8 "');
	result = replaceAll(result, "local <imm16>", '" F_LOCAL16 "');
	result = replaceAll(result, "global <imm8>", '" F_GLOBAL8 "');
	result = replaceAll(result, "global <imm16>", '" F_GLOBAL16 "');
	result = replaceAll(result, "extern <imm8>:<imm8>", '" F_EXTERN8 "');
	result = replaceAll(result, "extern <imm16>:<imm16>", '" F_EXTERN16 "');
	result = replaceAll(result, "module <imm8>", '" F_MODULE8 "');
	result = replaceAll(result, "module <imm16>", '" F_MODULE16 "');
	result = replaceAll(result, "#SP+/-<Simm8>", '" F_SP_PLUSS8 "');
	result = replaceAll(result, "#SP+/-<Simm16>", '" F_SP_PLUSS16 "');
	result = replaceAll(result, "#SP-<imm16>-2", '" F_SP_SUB16_SUB2 "');
	result = replaceAll(result, "#SP-<imm8>-2", '" F_SP_SUB8_SUB2 "');
	result = replaceAll(result, "<imm8>+1", '" F_IMM8_PLUS1 "');
	result = replaceAll(result, "<imm16>+1", '" F_IMM16_PLUS1 "');
	result = replaceAll(result, "<imm8>+2", '" F_IMM8_PLUS2 "');
	result = replaceAll(result, "<imm16>+2", '" F_IMM16_PLUS2 "');
	result = replaceAll(result, "<imm8>+3", '" F_IMM8_PLUS3 "');
	result = replaceAll(result, "<imm16>+3", '" F_IMM16_PLUS3 "');
	result = replaceAll(result, "<imm8>*2", '" F_IMM8_X2 "');
	result = replaceAll(result, "<imm16>*2", '" F_IMM16_X2 "');
	result = replaceAll(result, "<imm8>", '" F_IMM8 "');
	result = replaceAll(result, "<Simm8>", '" F_SIMM8 "');
	result = replaceAll(result, "<imm16>", '" F_IMM16 "');
	result = replaceAll(result, "<Simm16>", '" F_SIMM16 "');
	result = replaceAll(result, "<imm32>", '" F_IMM32 "');
	result = replaceAll(result, "<Simm32>", '" F_SIMM32 "');
	result = replaceAll(result, ' ""', " ");
	result = replaceAll(result, "  ", " ");
	if (!result.startswith('"')) {
		local base, none, args = result.partition(" ")...;
		if (!args)
			return repr base;
		result = f'"{base}" F_PAD "{args}"';
	} else {
		result = f'"{result}"';
	}
	result = replaceAll(result, ' ""', " ");
	result = replaceAll(result, '"" ', " ");
	result = replaceAll(result, "  ", " ");
	result = result.strip();
	usedFFormats.insertall(result.relocateall(r"F_[^ ]+"));
	return result;
}

function formatSpEffect(x: string, mnemonic: string, operandsOffset: int, neg: bool = false): string {
	x = x.partition("|").first; // Special case for ASM_FOREACH
	local r = try int(x) catch (...) none;
	if (r !is none) {
		local result = neg ? f"SP_SUB{-r}" : f"SP_ADD{r}";
		usedSpFormats.insert(result);
		return result;
	}
	local n_result = none;
	switch (x) {
	case "n":
	case "+n":
		if (!neg) n_result = "SP_ADDIMMn";
		break;
	case "1+n": case "n+1":
	case "+1+n": case "+n+1":
		if (!neg) n_result = "SP_ADDIMMn_PLUS1";
		break;
	case "2+n": case "n+2":
	case "+2+n": case "+n+2":
		if (!neg) n_result = "SP_ADDIMMn_PLUS2";
		break;
	case "3+n": case "n+3":
	case "+3+n": case "+n+3":
		if (!neg) n_result = "SP_ADDIMMn_PLUS3";
		break;
	case "-n":
		if (neg) n_result = "SP_SUBIMMn";
		break;
	case "-n*2":
	case "-2*n":
		if (neg) n_result = "SP_SUBIMMnX2";
		break;
	case "-1-n": case "-n-1":
		if (neg) n_result = "SP_SUBIMMn_MINUS1";
		break;
	case "-2-n": case "-n-2":
		if (neg) n_result = "SP_SUBIMMn_MINUS2";
		break;
	case "-3-n": case "-n-3":
		if (neg) n_result = "SP_SUBIMMn_MINUS3";
		break;
	case "-1-n*2": case "-n*2-1":
	case "-1-2*n": case "-2*n-1":
		if (neg) n_result = "SP_SUBIMMnX2_MINUS1";
		break;
	case "-2-n*2": case "-n*2-2":
	case "-2-2*n": case "-2*n-2":
		if (neg) n_result = "SP_SUBIMMnX2_MINUS2";
		break;
	case "-3-n*2": case "-n*2-3":
	case "-3-2*n": case "-2*n-3":
		if (neg) n_result = "SP_SUBIMMnX2_MINUS3";
		break;
	case "*":
	case "+*":
		if (!neg) n_result = "SP_ADDADJSTACKn";
		break;
	case "-*":
		if (neg) n_result = "SP_SUBADJSTACKn";
		break;
	default: break;
	}
	if (n_result !is none) {
		local temp = mnemonic.replace("F_PREFIX", "");
		local temp = temp.replace("F_PAD", "");
		local immMatches = Tuple(temp.relocateall("F_[^ ]+"));
		local offset = operandsOffset;
		local result = none;
		for (local x: immMatches) {
			local width = int(x.relocate("([0-9]+)"));
			if (x.startswith("F_IMM") || x.startswith("F_SP")) {
				result = n_result.replace("n", f"{offset}N{width}");
				usedSpFormats.insert(result);
			}
			if (x.startswith("F_EXTERN"))
				width *= 2;
			offset += width / 8;
		}
		if (result !is none)
			return result;
	}
	throw Error(f"Unknown stack effect: {repr x}");
}

local blob = File.open("./asm.h", "rb").read().unifylines();
local i = 0, end = #blob;
while (i < end) {
	local none, match = blob.refind(r"#\s*define\s+ASM", i, end)...;
	if (match is none)
		break;
	i = match;
	local eol = blob.find("\n", i, end);
	if (eol < 0)
		eol = end;
	while (eol > i && blob.isspace(eol - 1))
		--eol;
	local name, opcode, length, sp_sub, sp_add, mnemonic = none...;
	try {
		name, opcode, length, sp_sub, sp_add, mnemonic =
			blob.rescanf(r"([^\s]+)\s+([0-9a-fA-FxX]+)\s*" r"/\*\s*\[([^]]+)\]\s*\[([^,\]]+)\s*,\s*([^\]]+)\s*\]\s*`([^']*)'", i, eol)...;
	} catch (...) {
		try {
			name, opcode, length, sp_sub, sp_add =
				blob.rescanf(r"([^\s]+)\s+([0-9a-fA-FxX]+)\s*" r"/\*\s*\[([^]]+)\]\s*\[([^,\]]+)\s*,\s*([^\]]+)\s*\]", i, eol)...;
		} catch (...) {
			try {
				name, opcode, length =
					blob.rescanf(r"([^\s]+)\s+([0-9a-fA-FxX]+)\s*" r"/\*\s*\[([^]]+)\]", i, eol)...;
			} catch (...) {
				continue;
			}
		}
	}
	opcode = int(opcode);
	local opcodeLength = opcode > 0xff ? 2 : 1;
	local prefixed: Opcode | none = none;
	if (!blob.endswith("*" "/", i, eol)) {
		local nextLine = blob.index("\n", eol, end) + 1;
		local nextEol = blob.find("\n", nextLine, end);
		if (nextEol < 0)
			nextEol = end;
		local p_length, p_sp_sub, p_sp_add, p_mnemonic = none...;
		try {
			p_length, p_sp_sub, p_sp_add, p_mnemonic =
				blob.rescanf(r"\s*\*\s*\[([^]]+)\]\s*\[([^,\]]+)\s*,\s*([^\]]+)\s*\]\s*`([^']*)'", nextLine, nextEol)...;
		} catch (...) {
			try {
				p_length, p_sp_sub, p_sp_add =
					blob.rescanf(r"\s*\*\s*\[([^]]+)\]\s*\[([^,\]]+)\s*,\s*([^\]]+)\s*\]", nextLine, nextEol)...;
			} catch (...) {
			}
		}
		if (p_length !is none) {
			p_sp_sub = p_sp_sub.strip();
			p_sp_add = p_sp_add.strip();
			p_mnemonic = p_mnemonic is none ? none : parseMnemonic(p_mnemonic);
			prefixed = Opcode(
				length: int(p_length),
				sp_sub: formatSpEffect(p_sp_sub.decode("utf-8"), p_mnemonic, opcodeLength, true),
				sp_add: formatSpEffect(p_sp_add.decode("utf-8"), p_mnemonic, opcodeLength),
				mnemonic: p_mnemonic,
			);
		}
		eol = nextEol;
	}
	sp_sub = sp_sub.strip();
	sp_add = sp_add.strip();
	mnemonic = mnemonic is none ? none : parseMnemonic(mnemonic);
	local op = Opcode(
		name: name.decode("utf-8"),
		opcode: opcode,
		length: int(length),
		sp_sub: formatSpEffect(sp_sub.decode("utf-8"), mnemonic, opcodeLength, true),
		sp_add: formatSpEffect(sp_add.decode("utf-8"), mnemonic, opcodeLength),
		mnemonic: mnemonic,
		prefixed: prefixed,
	);
	if (prefixed !is none && prefixed.length != op.length)
		throw Error(f"Miss-matching prefix vs. non-prefix length in: {repr op}");
	opcodes.setdefault(opcode, op);
	eol = blob.find("\n", eol, end);
	if (eol < 0)
		eol = end;
	i = eol + 1;
}

print("/" "* Define control codes for disassembly representation format strings. *" "/");
print(r'#ifndef F_PAD');
usedFFormats.remove("F_PAD");
usedFFormats = usedFFormats.sorted(key: VERCMP_KEY);
local macros: {(string, string)...} = [
	("F_PAD",     r'"\t"'),
	("F_PAD_C",   r"'\t'"),
	("F_MINCODE", f"0x80"),
	("F_MAXCODE", f"{(0x80 + #usedFFormats - 1).hex(2)}"),
];
for (local i, x: usedFFormats.enumerate()) {
	local f_code = 0x80 + i;
	macros.append((f"{x}",   f'"\\x{f_code.tostr(16, 2)}"'));
	macros.append((f"{x}_C", f'{f_code.hex()}'));
}
local longestNameLen = macros.each.first.length > ...;
for (local name, value: macros)
	print(f"#define {name.ljust(longestNameLen)} {value}");
print('#endif /' '* !F_PAD *' '/');

print("/" "* Define control codes for negative/positive stack effects *" "/");
print(f'#ifndef SP_ADD0');
local addCodes = HashSet(for (local x: usedSpFormats) if (x.startswith("SP_ADD")) x).sorted(key: VERCMP_KEY);
local subCodes = HashSet(for (local x: usedSpFormats) if (!x.startswith("SP_ADD")) x).sorted(key: VERCMP_KEY);
local macros: {(string, string)...} = [
	("SP_ADD_COUNT", f'{#addCodes}'),
	("SP_SUB_COUNT", f'{#subCodes}'),
];
for (local i, x: addCodes.enumerate())
	macros.append((f"{x}", f"{i}"));
for (local i, x: subCodes.enumerate())
	macros.append((f"{x}", f"{i}"));
local longestNameLen = macros.each.first.length > ...;
for (local name, value: macros)
	print(f"#define {name.ljust(longestNameLen)} {value}");
print(f'#endif /' f'* !SP_ADD0 *' f'/');
print();

print("/" "* DEE_SP_SUB(code, expr) -- Define expressions to calculate negative stack effects. *" "/");
print(f'#ifdef DEE_SP_SUB');
for (local x: subCodes) {
	local a, b, c, d = none...;
#define TRY_MATCH(re...) ({ try { re; true; } catch (...) false; })
	print(f'DEE_SP_SUB({x}, '),;
	if (TRY_MATCH(a, b, c, d = x.rescanf(r'SP_SUBIMM([0-9]+)N([0-9]+)X([0-9]+)_MINUS([0-9]+)')...)) {
		a = int(a);
		print(a ? f'UNALIGNED_GETLE{b}(pc + {a}) * {c} + {d}'
		        : f'UNALIGNED_GETLE{b}(pc) * {c} + {d}'),;
	} else if (TRY_MATCH(a, b, c = x.rescanf(r'SP_SUBIMM([0-9]+)N([0-9]+)_MINUS([0-9]+)')...)) {
		a = int(a);
		print(a ? f'UNALIGNED_GETLE{b}(pc + {a}) + {c}'
		        : f'UNALIGNED_GETLE{b}(pc) + {c}'),;
	} else if (TRY_MATCH(a, b, c = x.rescanf(r'SP_SUBIMM([0-9]+)N([0-9]+)X([0-9]+)')...)) {
		a = int(a);
		print(a ? f'UNALIGNED_GETLE{b}(pc + {a}) * {c}'
		        : f'UNALIGNED_GETLE{b}(pc)' * {c}),;
	} else if (TRY_MATCH(a, b = x.rescanf(r'SP_SUBIMM([0-9]+)N([0-9]+)')...)) {
		a = int(a);
		print(a ? f'UNALIGNED_GETLE{b}(pc + {a})'
		        : f'UNALIGNED_GETLE{b}(pc)'),;
	} else if (TRY_MATCH(a, b = x.rescanf(r'SP_SUBADJSTACK([0-9]+)N([0-9]+)')...)) {
		a = int(a);
		print(a ? f'(int{b}_t)UNALIGNED_GETLE{b}(pc + {a}) >= 0 ? 0 : -(int{b}_t)UNALIGNED_GETLE{b}(pc + {a})'
		        : f'(int{b}_t)UNALIGNED_GETLE{b}(pc) >= 0 ? 0 : -(int{b}_t)UNALIGNED_GETLE{b}(pc)'),;
	} else if (TRY_MATCH(a = x.rescanf(r'SP_SUB([0-9]+)')...)) {
		print(f'{a}'),;
	} else {
		throw Error(f"Unable to decode sp-sub-code: {repr x}");
	}
	print(f')');
}
print(f'#endif /' f'* !DEE_SP_SUB *' f'/');
print();

print("/" "* DEE_SP_ADD(code, expr) -- Define expressions to calculate positive stack effects. *" "/");
print(f'#ifdef DEE_SP_ADD');
for (local x: addCodes) {
	local a, b, c, d = none...;
	print(f'DEE_SP_ADD({x}, '),;
	if (TRY_MATCH(a, b, c, d = x.rescanf(r'SP_ADDIMM([0-9]+)N([0-9]+)X([0-9]+)_PLUS([0-9]+)')...)) {
		a = int(a);
		print(a ? f'UNALIGNED_GETLE{b}(pc + {a}) * {c} + {d}'
		        : f'UNALIGNED_GETLE{b}(pc) * {c} + {d}'),;
	} else if (TRY_MATCH(a, b, c = x.rescanf(r'SP_ADDIMM([0-9]+)N([0-9]+)_PLUS([0-9]+)')...)) {
		a = int(a);
		print(a ? f'UNALIGNED_GETLE{b}(pc + {a}) + {c}'
		        : f'UNALIGNED_GETLE{b}(pc) + {c}'),;
	} else if (TRY_MATCH(a, b, c = x.rescanf(r'SP_ADDIMM([0-9]+)N([0-9]+)X([0-9]+)')...)) {
		a = int(a);
		print(a ? f'UNALIGNED_GETLE{b}(pc + {a}) * {c}'
		        : f'UNALIGNED_GETLE{b}(pc)' * {c}),;
	} else if (TRY_MATCH(a, b = x.rescanf(r'SP_ADDIMM([0-9]+)N([0-9]+)')...)) {
		a = int(a);
		print(a ? f'UNALIGNED_GETLE{b}(pc + {a})'
		        : f'UNALIGNED_GETLE{b}(pc)'),;
	} else if (TRY_MATCH(a, b = x.rescanf(r'SP_ADDADJSTACK([0-9]+)N([0-9]+)')...)) {
		a = int(a);
		print(a ? f'(int{b}_t)UNALIGNED_GETLE{b}(pc + {a}) <= 0 ? 0 : (int{b}_t)UNALIGNED_GETLE{b}(pc + {a})'
		        : f'(int{b}_t)UNALIGNED_GETLE{b}(pc) <= 0 ? 0 : (int{b}_t)UNALIGNED_GETLE{b}(pc)'),;
	} else if (TRY_MATCH(a = x.rescanf(r'SP_ADD([0-9]+)')...)) {
		print(f'{a}'),;
	} else {
		throw Error(f"Unable to decode sp-add-code: {repr x}");
	}
	print(f')');
}
print(f'#endif /' f'* !DEE_SP_ADD *' f'/');
print();

local prefixBytes = HashSet(for (local opcode: opcodes.keys) (opcode & 0xff00) >> 8);
for (local prefix: prefixBytes) {
	print(f"#if DEE_ASM_WANT_TABLE({prefix.hex(2)})");
	print(f"DEE_ASM_BEGIN({prefix.hex(2)})");
	for (local opByte: [:256]) {
		local code = (prefix << 8) | opByte;
		local c = opcodes.get(code);
		if (c !is none) {
			if (ASM_ISPREFIX(opByte)) {
				print(f"DEE_ASM_PREFIX({prefix.hex(2)}, {opByte.hex(2)}, {c.length}, {c.name})");
			} else {
				print(f"DEE_ASM_OPCODE{c.prefixed is none ? "" : "_P"}({prefix.hex(2)}, {opByte.hex(2)}, "
				      f"{c.length}, {c.name}, {c.sp_sub}, {c.sp_add}, {c.mnemonic ?? '""'}"),;
				if (c.prefixed !is none)
					print(f", {c.prefixed.sp_sub}, {c.prefixed.sp_add}, {c.prefixed.mnemonic ?? '""'}"),;
				print(f")");
			}
		} else {
			local length = prefix ? 2 : 1;
			if (ASM_ISPREFIX(opByte)) {
				print(f"DEE_ASM_UNDEFINED_PREFIX({prefix.hex(2)}, {opByte.hex(2)}, {length})");
			} else if (ASM_ISEXTENDED(opByte) && prefix == 0x00) {
				print(f"DEE_ASM_EXTENDED({prefix.hex(2)}, {opByte.hex(2)}, {length}, _EXTENDED{opByte - ASM_EXTENDEDMIN + 1})");
			} else {
				print(f"DEE_ASM_UNDEFINED({prefix.hex(2)}, {opByte.hex(2)}, {length})");
			}
		}
	}
	print(f"DEE_ASM_END({prefix.hex(2)})");
	print(f"#endif /" f"* DEE_ASM_WANT_TABLE({prefix.hex(2)}) *" f"/");
	print(f"");
}
]]]*/
/* Define control codes for disassembly representation format strings. */
#ifndef F_PAD
#define F_PAD             "\t"
#define F_PAD_C           '\t'
#define F_MINCODE         0x80
#define F_MAXCODE         0xa3
#define F_ARG8            "\x80"
#define F_ARG8_C          0x80
#define F_ARG16           "\x81"
#define F_ARG16_C         0x81
#define F_CONST8          "\x82"
#define F_CONST8_C        0x82
#define F_CONST16         "\x83"
#define F_CONST16_C       0x83
#define F_EXTERN8         "\x84"
#define F_EXTERN8_C       0x84
#define F_EXTERN16        "\x85"
#define F_EXTERN16_C      0x85
#define F_GLOBAL8         "\x86"
#define F_GLOBAL8_C       0x86
#define F_GLOBAL16        "\x87"
#define F_GLOBAL16_C      0x87
#define F_IMM8            "\x88"
#define F_IMM8_C          0x88
#define F_IMM8_PLUS1      "\x89"
#define F_IMM8_PLUS1_C    0x89
#define F_IMM8_PLUS2      "\x8a"
#define F_IMM8_PLUS2_C    0x8a
#define F_IMM8_PLUS3      "\x8b"
#define F_IMM8_PLUS3_C    0x8b
#define F_IMM8_X2         "\x8c"
#define F_IMM8_X2_C       0x8c
#define F_IMM16           "\x8d"
#define F_IMM16_C         0x8d
#define F_IMM16_PLUS2     "\x8e"
#define F_IMM16_PLUS2_C   0x8e
#define F_IMM16_PLUS3     "\x8f"
#define F_IMM16_PLUS3_C   0x8f
#define F_IMM16_X2        "\x90"
#define F_IMM16_X2_C      0x90
#define F_IMM32           "\x91"
#define F_IMM32_C         0x91
#define F_LOCAL8          "\x92"
#define F_LOCAL8_C        0x92
#define F_LOCAL16         "\x93"
#define F_LOCAL16_C       0x93
#define F_MODULE8         "\x94"
#define F_MODULE8_C       0x94
#define F_MODULE16        "\x95"
#define F_MODULE16_C      0x95
#define F_PREFIX          "\x96"
#define F_PREFIX_C        0x96
#define F_REF8            "\x97"
#define F_REF8_C          0x97
#define F_REF16           "\x98"
#define F_REF16_C         0x98
#define F_SDISP8          "\x99"
#define F_SDISP8_C        0x99
#define F_SDISP16         "\x9a"
#define F_SDISP16_C       0x9a
#define F_SDISP32         "\x9b"
#define F_SDISP32_C       0x9b
#define F_SIMM8           "\x9c"
#define F_SIMM8_C         0x9c
#define F_SIMM16          "\x9d"
#define F_SIMM16_C        0x9d
#define F_SP_PLUSS8       "\x9e"
#define F_SP_PLUSS8_C     0x9e
#define F_SP_PLUSS16      "\x9f"
#define F_SP_PLUSS16_C    0x9f
#define F_SP_SUB8_SUB2    "\xa0"
#define F_SP_SUB8_SUB2_C  0xa0
#define F_SP_SUB16_SUB2   "\xa1"
#define F_SP_SUB16_SUB2_C 0xa1
#define F_STATIC8         "\xa2"
#define F_STATIC8_C       0xa2
#define F_STATIC16        "\xa3"
#define F_STATIC16_C      0xa3
#endif /* !F_PAD */
/* Define control codes for negative/positive stack effects */
#ifndef SP_ADD0
#define SP_ADD_COUNT          15
#define SP_SUB_COUNT          27
#define SP_ADD0               0
#define SP_ADD1               1
#define SP_ADD2               2
#define SP_ADD3               3
#define SP_ADDADJSTACK1N8     4
#define SP_ADDADJSTACK2N16    5
#define SP_ADDIMM1N8          6
#define SP_ADDIMM1N8_PLUS1    7
#define SP_ADDIMM1N8_PLUS2    8
#define SP_ADDIMM1N8_PLUS3    9
#define SP_ADDIMM2N8          10
#define SP_ADDIMM2N16         11
#define SP_ADDIMM2N16_PLUS1   12
#define SP_ADDIMM2N16_PLUS2   13
#define SP_ADDIMM2N16_PLUS3   14
#define SP_SUB0               0
#define SP_SUB1               1
#define SP_SUB2               2
#define SP_SUB3               3
#define SP_SUB4               4
#define SP_SUBADJSTACK1N8     5
#define SP_SUBADJSTACK2N16    6
#define SP_SUBIMM1N8          7
#define SP_SUBIMM1N8_MINUS1   8
#define SP_SUBIMM1N8_MINUS2   9
#define SP_SUBIMM1N8_MINUS3   10
#define SP_SUBIMM2N8          11
#define SP_SUBIMM2N8_MINUS1   12
#define SP_SUBIMM2N8X2        13
#define SP_SUBIMM2N8X2_MINUS1 14
#define SP_SUBIMM2N8_MINUS3   15
#define SP_SUBIMM2N16         16
#define SP_SUBIMM2N16_MINUS1  17
#define SP_SUBIMM2N16_MINUS2  18
#define SP_SUBIMM2N16_MINUS3  19
#define SP_SUBIMM2N16X2       20
#define SP_SUBIMM3N8          21
#define SP_SUBIMM4N8          22
#define SP_SUBIMM4N8X2_MINUS1 23
#define SP_SUBIMM4N8_MINUS1   24
#define SP_SUBIMM4N16         25
#define SP_SUBIMM6N8          26
#endif /* !SP_ADD0 */

/* DEE_SP_SUB(code, expr) -- Define expressions to calculate negative stack effects. */
#ifdef DEE_SP_SUB
DEE_SP_SUB(SP_SUB0, 0)
DEE_SP_SUB(SP_SUB1, 1)
DEE_SP_SUB(SP_SUB2, 2)
DEE_SP_SUB(SP_SUB3, 3)
DEE_SP_SUB(SP_SUB4, 4)
DEE_SP_SUB(SP_SUBADJSTACK1N8, (int8_t)UNALIGNED_GETLE8(pc + 1) >= 0 ? 0 : -(int8_t)UNALIGNED_GETLE8(pc + 1))
DEE_SP_SUB(SP_SUBADJSTACK2N16, (int16_t)UNALIGNED_GETLE16(pc + 2) >= 0 ? 0 : -(int16_t)UNALIGNED_GETLE16(pc + 2))
DEE_SP_SUB(SP_SUBIMM1N8, UNALIGNED_GETLE8(pc + 1))
DEE_SP_SUB(SP_SUBIMM1N8_MINUS1, UNALIGNED_GETLE8(pc + 1) + 1)
DEE_SP_SUB(SP_SUBIMM1N8_MINUS2, UNALIGNED_GETLE8(pc + 1) + 2)
DEE_SP_SUB(SP_SUBIMM1N8_MINUS3, UNALIGNED_GETLE8(pc + 1) + 3)
DEE_SP_SUB(SP_SUBIMM2N8, UNALIGNED_GETLE8(pc + 2))
DEE_SP_SUB(SP_SUBIMM2N8_MINUS1, UNALIGNED_GETLE8(pc + 2) + 1)
DEE_SP_SUB(SP_SUBIMM2N8X2, UNALIGNED_GETLE8(pc + 2) * 2)
DEE_SP_SUB(SP_SUBIMM2N8X2_MINUS1, UNALIGNED_GETLE8(pc + 2) * 2 + 1)
DEE_SP_SUB(SP_SUBIMM2N8_MINUS3, UNALIGNED_GETLE8(pc + 2) + 3)
DEE_SP_SUB(SP_SUBIMM2N16, UNALIGNED_GETLE16(pc + 2))
DEE_SP_SUB(SP_SUBIMM2N16_MINUS1, UNALIGNED_GETLE16(pc + 2) + 1)
DEE_SP_SUB(SP_SUBIMM2N16_MINUS2, UNALIGNED_GETLE16(pc + 2) + 2)
DEE_SP_SUB(SP_SUBIMM2N16_MINUS3, UNALIGNED_GETLE16(pc + 2) + 3)
DEE_SP_SUB(SP_SUBIMM2N16X2, UNALIGNED_GETLE16(pc + 2) * 2)
DEE_SP_SUB(SP_SUBIMM3N8, UNALIGNED_GETLE8(pc + 3))
DEE_SP_SUB(SP_SUBIMM4N8, UNALIGNED_GETLE8(pc + 4))
DEE_SP_SUB(SP_SUBIMM4N8X2_MINUS1, UNALIGNED_GETLE8(pc + 4) * 2 + 1)
DEE_SP_SUB(SP_SUBIMM4N8_MINUS1, UNALIGNED_GETLE8(pc + 4) + 1)
DEE_SP_SUB(SP_SUBIMM4N16, UNALIGNED_GETLE16(pc + 4))
DEE_SP_SUB(SP_SUBIMM6N8, UNALIGNED_GETLE8(pc + 6))
#endif /* !DEE_SP_SUB */

/* DEE_SP_ADD(code, expr) -- Define expressions to calculate positive stack effects. */
#ifdef DEE_SP_ADD
DEE_SP_ADD(SP_ADD0, 0)
DEE_SP_ADD(SP_ADD1, 1)
DEE_SP_ADD(SP_ADD2, 2)
DEE_SP_ADD(SP_ADD3, 3)
DEE_SP_ADD(SP_ADDADJSTACK1N8, (int8_t)UNALIGNED_GETLE8(pc + 1) <= 0 ? 0 : (int8_t)UNALIGNED_GETLE8(pc + 1))
DEE_SP_ADD(SP_ADDADJSTACK2N16, (int16_t)UNALIGNED_GETLE16(pc + 2) <= 0 ? 0 : (int16_t)UNALIGNED_GETLE16(pc + 2))
DEE_SP_ADD(SP_ADDIMM1N8, UNALIGNED_GETLE8(pc + 1))
DEE_SP_ADD(SP_ADDIMM1N8_PLUS1, UNALIGNED_GETLE8(pc + 1) + 1)
DEE_SP_ADD(SP_ADDIMM1N8_PLUS2, UNALIGNED_GETLE8(pc + 1) + 2)
DEE_SP_ADD(SP_ADDIMM1N8_PLUS3, UNALIGNED_GETLE8(pc + 1) + 3)
DEE_SP_ADD(SP_ADDIMM2N8, UNALIGNED_GETLE8(pc + 2))
DEE_SP_ADD(SP_ADDIMM2N16, UNALIGNED_GETLE16(pc + 2))
DEE_SP_ADD(SP_ADDIMM2N16_PLUS1, UNALIGNED_GETLE16(pc + 2) + 1)
DEE_SP_ADD(SP_ADDIMM2N16_PLUS2, UNALIGNED_GETLE16(pc + 2) + 2)
DEE_SP_ADD(SP_ADDIMM2N16_PLUS3, UNALIGNED_GETLE16(pc + 2) + 3)
#endif /* !DEE_SP_ADD */

#if DEE_ASM_WANT_TABLE(0x00)
DEE_ASM_BEGIN(0x00)
DEE_ASM_OPCODE(0x00, 0x00, 1, _RET_NONE, SP_SUB0, SP_ADD0, "ret")
DEE_ASM_OPCODE_P(0x00, 0x01, 1, _RET, SP_SUB1, SP_ADD0, "ret" F_PAD "pop", SP_SUB0, SP_ADD0, "ret" F_PAD F_PREFIX)
DEE_ASM_OPCODE_P(0x00, 0x02, 1, _YIELDALL, SP_SUB1, SP_ADD0, "yield" F_PAD "foreach, pop", SP_SUB0, SP_ADD0, "yield" F_PAD "foreach, " F_PREFIX)
DEE_ASM_OPCODE_P(0x00, 0x03, 1, _THROW, SP_SUB1, SP_ADD0, "throw" F_PAD "pop", SP_SUB0, SP_ADD0, "throw" F_PAD F_PREFIX)
DEE_ASM_OPCODE(0x00, 0x04, 1, _RETHROW, SP_SUB0, SP_ADD0, "throw" F_PAD "except")
DEE_ASM_OPCODE(0x00, 0x05, 1, _SETRET, SP_SUB1, SP_ADD0, "setret" F_PAD "pop")
DEE_ASM_OPCODE(0x00, 0x06, 1, _ENDCATCH, SP_SUB0, SP_ADD0, "end" F_PAD "catch")
DEE_ASM_OPCODE(0x00, 0x07, 1, _ENDFINALLY, SP_SUB0, SP_ADD0, "end" F_PAD "finally")
DEE_ASM_OPCODE(0x00, 0x08, 3, _CALL_KW, SP_SUBIMM1N8_MINUS1, SP_ADD1, "call" F_PAD "top, #" F_IMM8 ", " F_CONST8)
DEE_ASM_OPCODE(0x00, 0x09, 2, _CALL_TUPLE_KW, SP_SUB2, SP_ADD1, "call" F_PAD "top, pop..., " F_CONST8)
DEE_ASM_UNDEFINED(0x00, 0x0a, 1)
DEE_ASM_OPCODE(0x00, 0x0b, 2, _PUSH_BND_ARG, SP_SUB0, SP_ADD1, "push" F_PAD "bound " F_ARG8)
DEE_ASM_OPCODE(0x00, 0x0c, 3, _PUSH_BND_EXTERN, SP_SUB0, SP_ADD1, "push" F_PAD "bound " F_EXTERN8)
DEE_ASM_OPCODE(0x00, 0x0d, 2, _PUSH_BND_STATIC, SP_SUB0, SP_ADD1, "push" F_PAD "bound " F_STATIC8)
DEE_ASM_OPCODE(0x00, 0x0e, 2, _PUSH_BND_GLOBAL, SP_SUB0, SP_ADD1, "push" F_PAD "bound " F_GLOBAL8)
DEE_ASM_OPCODE(0x00, 0x0f, 2, _PUSH_BND_LOCAL, SP_SUB0, SP_ADD1, "push" F_PAD "bound " F_LOCAL8)
DEE_ASM_OPCODE_P(0x00, 0x10, 2, _JF, SP_SUB1, SP_ADD0, "jf" F_PAD "pop, " F_SDISP8, SP_SUB0, SP_ADD0, "jf" F_PAD F_PREFIX ", " F_SDISP8)
DEE_ASM_OPCODE_P(0x00, 0x11, 3, _JF16, SP_SUB1, SP_ADD0, "jf" F_PAD "pop, " F_SDISP16, SP_SUB0, SP_ADD0, "jf" F_PAD F_PREFIX ", " F_SDISP16)
DEE_ASM_OPCODE_P(0x00, 0x12, 2, _JT, SP_SUB1, SP_ADD0, "jt" F_PAD "pop, " F_SDISP8, SP_SUB0, SP_ADD0, "jt" F_PAD F_PREFIX ", " F_SDISP8)
DEE_ASM_OPCODE_P(0x00, 0x13, 3, _JT16, SP_SUB1, SP_ADD0, "jt" F_PAD "pop, " F_SDISP16, SP_SUB0, SP_ADD0, "jt" F_PAD F_PREFIX ", " F_SDISP16)
DEE_ASM_OPCODE(0x00, 0x14, 2, _JMP, SP_SUB0, SP_ADD0, "jmp" F_PAD F_SDISP8)
DEE_ASM_OPCODE(0x00, 0x15, 3, _JMP16, SP_SUB0, SP_ADD0, "jmp" F_PAD F_SDISP16)
DEE_ASM_OPCODE_P(0x00, 0x16, 2, _FOREACH, SP_SUB1, SP_ADD2, "foreach" F_PAD "top, " F_SDISP8, SP_SUB0, SP_ADD1, "foreach" F_PAD F_PREFIX ", " F_SDISP8)
DEE_ASM_OPCODE_P(0x00, 0x17, 3, _FOREACH16, SP_SUB1, SP_ADD2, "foreach" F_PAD "top, " F_SDISP16, SP_SUB0, SP_ADD1, "foreach" F_PAD F_PREFIX ", " F_SDISP16)
DEE_ASM_OPCODE(0x00, 0x18, 1, _JMP_POP, SP_SUB1, SP_ADD0, "jmp" F_PAD "pop")
DEE_ASM_OPCODE_P(0x00, 0x19, 3, _OPERATOR, SP_SUBIMM2N8_MINUS1, SP_ADD1, "op" F_PAD "top, $" F_IMM8 ", #" F_IMM8, SP_SUBIMM2N8, SP_ADD1, F_PREFIX ": push op $" F_IMM8 ", #" F_IMM8)
DEE_ASM_OPCODE_P(0x00, 0x1a, 2, _OPERATOR_TUPLE, SP_SUB2, SP_ADD1, "op" F_PAD "top, $" F_IMM8 ", pop...", SP_SUB1, SP_ADD1, F_PREFIX ": push op $" F_IMM8 ", pop...")
DEE_ASM_OPCODE(0x00, 0x1b, 2, _CALL, SP_SUBIMM1N8_MINUS1, SP_ADD1, "call" F_PAD "top, #" F_IMM8)
DEE_ASM_OPCODE(0x00, 0x1c, 1, _CALL_TUPLE, SP_SUB2, SP_ADD1, "call" F_PAD "top, pop...")
DEE_ASM_OPCODE(0x00, 0x1d, 2, _DEL_STATIC, SP_SUB0, SP_ADD0, "del" F_PAD F_STATIC8)
DEE_ASM_OPCODE(0x00, 0x1e, 2, _DEL_GLOBAL, SP_SUB0, SP_ADD0, "del" F_PAD F_GLOBAL8)
DEE_ASM_OPCODE(0x00, 0x1f, 2, _DEL_LOCAL, SP_SUB0, SP_ADD0, "del" F_PAD F_LOCAL8)
DEE_ASM_OPCODE_P(0x00, 0x20, 1, _SWAP, SP_SUB2, SP_ADD2, "swap", SP_SUB1, SP_ADD1, "swap" F_PAD "top, " F_PREFIX)
DEE_ASM_OPCODE_P(0x00, 0x21, 2, _LROT, SP_SUBIMM1N8_MINUS3, SP_ADDIMM1N8_PLUS3, "lrot" F_PAD "#" F_IMM8_PLUS3, SP_SUBIMM1N8_MINUS2, SP_ADDIMM1N8_PLUS2, "lrot" F_PAD "#" F_IMM8_PLUS2 ", " F_PREFIX)
DEE_ASM_OPCODE_P(0x00, 0x22, 2, _RROT, SP_SUBIMM1N8, SP_ADDIMM1N8, "rrot" F_PAD "#" F_IMM8_PLUS3, SP_SUBIMM1N8, SP_ADDIMM1N8, "rrot" F_PAD "#" F_IMM8_PLUS2 ", " F_PREFIX)
DEE_ASM_OPCODE_P(0x00, 0x23, 1, _DUP, SP_SUB1, SP_ADD2, "dup", SP_SUB1, SP_ADD1, "mov" F_PAD F_PREFIX ", top")
DEE_ASM_OPCODE_P(0x00, 0x24, 2, _DUP_N, SP_SUBIMM1N8, SP_ADDIMM1N8_PLUS1, "dup" F_PAD F_SP_SUB8_SUB2, SP_SUBIMM1N8, SP_ADDIMM1N8, "mov" F_PAD F_PREFIX ", " F_SP_SUB8_SUB2)
DEE_ASM_OPCODE_P(0x00, 0x25, 1, _POP, SP_SUB1, SP_ADD0, "pop", SP_SUB0, SP_ADD0, "mov" F_PAD "top, " F_PREFIX)
DEE_ASM_OPCODE_P(0x00, 0x26, 2, _POP_N, SP_SUBIMM1N8_MINUS1, SP_ADDIMM1N8, "pop" F_PAD F_SP_SUB8_SUB2, SP_SUB0, SP_ADD0, "mov" F_PAD F_SP_SUB8_SUB2 ", " F_PREFIX)
DEE_ASM_OPCODE(0x00, 0x27, 2, _ADJSTACK, SP_SUBADJSTACK1N8, SP_ADDADJSTACK1N8, "adjstack" F_PAD F_SP_PLUSS8)
DEE_ASM_OPCODE(0x00, 0x28, 1, _SUPER, SP_SUB2, SP_ADD1, "super" F_PAD "top, pop")
DEE_ASM_OPCODE(0x00, 0x29, 2, _SUPER_THIS_R, SP_SUB0, SP_ADD1, "push" F_PAD "super this, " F_REF8)
DEE_ASM_UNDEFINED(0x00, 0x2a, 1)
DEE_ASM_UNDEFINED(0x00, 0x2b, 1)
DEE_ASM_OPCODE_P(0x00, 0x2c, 3, _POP_EXTERN, SP_SUB1, SP_ADD0, "pop" F_PAD F_EXTERN8, SP_SUB0, SP_ADD0, "mov" F_PAD F_EXTERN8 ", " F_PREFIX)
DEE_ASM_OPCODE_P(0x00, 0x2d, 2, _POP_STATIC, SP_SUB1, SP_ADD0, "pop" F_PAD F_STATIC8, SP_SUB0, SP_ADD0, "mov" F_PAD F_STATIC8 ", " F_PREFIX)
DEE_ASM_OPCODE_P(0x00, 0x2e, 2, _POP_GLOBAL, SP_SUB1, SP_ADD0, "pop" F_PAD F_GLOBAL8, SP_SUB0, SP_ADD0, "mov" F_PAD F_GLOBAL8 ", " F_PREFIX)
DEE_ASM_OPCODE_P(0x00, 0x2f, 2, _POP_LOCAL, SP_SUB1, SP_ADD0, "pop" F_PAD F_LOCAL8, SP_SUB0, SP_ADD0, "mov" F_PAD F_LOCAL8 ", " F_PREFIX)
DEE_ASM_UNDEFINED(0x00, 0x30, 1)
DEE_ASM_UNDEFINED(0x00, 0x31, 1)
DEE_ASM_UNDEFINED(0x00, 0x32, 1)
DEE_ASM_OPCODE_P(0x00, 0x33, 1, _PUSH_NONE, SP_SUB0, SP_ADD1, "push" F_PAD "none", SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", none")
DEE_ASM_OPCODE_P(0x00, 0x34, 1, _PUSH_VARARGS, SP_SUB0, SP_ADD1, "push" F_PAD "varargs", SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", varargs")
DEE_ASM_OPCODE_P(0x00, 0x35, 1, _PUSH_VARKWDS, SP_SUB0, SP_ADD1, "push" F_PAD "varkwds", SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", varkwds")
DEE_ASM_OPCODE_P(0x00, 0x36, 2, _PUSH_MODULE, SP_SUB0, SP_ADD1, "push" F_PAD F_MODULE8, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_MODULE8)
DEE_ASM_OPCODE_P(0x00, 0x37, 2, _PUSH_ARG, SP_SUB0, SP_ADD1, "push" F_PAD F_ARG8, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_ARG8)
DEE_ASM_OPCODE_P(0x00, 0x38, 2, _PUSH_CONST, SP_SUB0, SP_ADD1, "push" F_PAD F_CONST8, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_CONST8)
DEE_ASM_OPCODE_P(0x00, 0x39, 2, _PUSH_REF, SP_SUB0, SP_ADD1, "push" F_PAD F_REF8, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_REF8)
DEE_ASM_UNDEFINED(0x00, 0x3a, 1)
DEE_ASM_UNDEFINED(0x00, 0x3b, 1)
DEE_ASM_OPCODE_P(0x00, 0x3c, 3, _PUSH_EXTERN, SP_SUB0, SP_ADD1, "push" F_PAD F_EXTERN8, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_EXTERN8)
DEE_ASM_OPCODE_P(0x00, 0x3d, 2, _PUSH_STATIC, SP_SUB0, SP_ADD1, "push" F_PAD F_STATIC8, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_STATIC8)
DEE_ASM_OPCODE_P(0x00, 0x3e, 2, _PUSH_GLOBAL, SP_SUB0, SP_ADD1, "push" F_PAD F_GLOBAL8, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_GLOBAL8)
DEE_ASM_OPCODE_P(0x00, 0x3f, 2, _PUSH_LOCAL, SP_SUB0, SP_ADD1, "push" F_PAD F_LOCAL8, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_LOCAL8)
DEE_ASM_OPCODE(0x00, 0x40, 1, _CAST_TUPLE, SP_SUB1, SP_ADD1, "cast" F_PAD "top, Tuple")
DEE_ASM_OPCODE(0x00, 0x41, 1, _CAST_LIST, SP_SUB1, SP_ADD1, "cast" F_PAD "top, List")
DEE_ASM_OPCODE(0x00, 0x42, 2, _PACK_TUPLE, SP_SUBIMM1N8, SP_ADD1, "push" F_PAD "pack Tuple, #" F_IMM8)
DEE_ASM_OPCODE(0x00, 0x43, 2, _PACK_LIST, SP_SUBIMM1N8, SP_ADD1, "push" F_PAD "pack List, #" F_IMM8)
DEE_ASM_OPCODE(0x00, 0x44, 1, _CAST_VARKWDS, SP_SUB1, SP_ADD1, "cast" F_PAD "top, varkwds")
DEE_ASM_OPCODE(0x00, 0x45, 1, _PACK_ONE, SP_SUB1, SP_ADD1, "push" F_PAD "pack Sequence, #1")
DEE_ASM_OPCODE_P(0x00, 0x46, 2, _UNPACK, SP_SUB1, SP_ADDIMM1N8, "unpack" F_PAD "pop, #" F_IMM8, SP_SUB0, SP_ADDIMM1N8, "unpack" F_PAD F_PREFIX ", #" F_IMM8)
DEE_ASM_OPCODE(0x00, 0x47, 1, _CONCAT, SP_SUB2, SP_ADD1, "concat" F_PAD "top, pop")
DEE_ASM_OPCODE(0x00, 0x48, 2, _EXTEND, SP_SUBIMM1N8_MINUS1, SP_ADD1, "extend" F_PAD "top, #" F_IMM8)
DEE_ASM_OPCODE(0x00, 0x49, 1, _TYPEOF, SP_SUB1, SP_ADD1, "typeof" F_PAD "top")
DEE_ASM_OPCODE(0x00, 0x4a, 1, _CLASSOF, SP_SUB1, SP_ADD1, "classof" F_PAD "top")
DEE_ASM_OPCODE(0x00, 0x4b, 1, _SUPEROF, SP_SUB1, SP_ADD1, "superof" F_PAD "top")
DEE_ASM_OPCODE(0x00, 0x4c, 1, _INSTANCEOF, SP_SUB2, SP_ADD1, "instanceof" F_PAD "top, pop")
DEE_ASM_OPCODE(0x00, 0x4d, 1, _IMPLEMENTS, SP_SUB2, SP_ADD1, "implements" F_PAD "top, pop")
DEE_ASM_OPCODE(0x00, 0x4e, 1, _STR, SP_SUB1, SP_ADD1, "str" F_PAD "top")
DEE_ASM_OPCODE(0x00, 0x4f, 1, _REPR, SP_SUB1, SP_ADD1, "repr" F_PAD "top")
DEE_ASM_OPCODE(0x00, 0x50, 1, _BOOL, SP_SUB1, SP_ADD1, "bool" F_PAD "top")
DEE_ASM_OPCODE(0x00, 0x51, 1, _NOT, SP_SUB1, SP_ADD1, "not" F_PAD "top")
DEE_ASM_OPCODE(0x00, 0x52, 1, _ASSIGN, SP_SUB2, SP_ADD0, "assign" F_PAD "pop, pop")
DEE_ASM_OPCODE(0x00, 0x53, 1, _MOVE_ASSIGN, SP_SUB2, SP_ADD0, "assign" F_PAD "move, pop, pop")
DEE_ASM_OPCODE(0x00, 0x54, 1, _COPY, SP_SUB1, SP_ADD1, "copy" F_PAD "top")
DEE_ASM_OPCODE(0x00, 0x55, 1, _DEEPCOPY, SP_SUB1, SP_ADD1, "deepcopy" F_PAD "top")
DEE_ASM_OPCODE(0x00, 0x56, 1, _GETATTR, SP_SUB2, SP_ADD1, "getattr" F_PAD "top, pop")
DEE_ASM_OPCODE(0x00, 0x57, 1, _DELATTR, SP_SUB2, SP_ADD0, "delattr" F_PAD "pop, pop")
DEE_ASM_OPCODE(0x00, 0x58, 1, _SETATTR, SP_SUB3, SP_ADD0, "setattr" F_PAD "pop, pop, pop")
DEE_ASM_OPCODE(0x00, 0x59, 1, _BOUNDATTR, SP_SUB2, SP_ADD1, "boundattr" F_PAD "top, pop")
DEE_ASM_OPCODE(0x00, 0x5a, 2, _GETATTR_C, SP_SUB1, SP_ADD1, "getattr" F_PAD "top, " F_CONST8)
DEE_ASM_OPCODE(0x00, 0x5b, 2, _DELATTR_C, SP_SUB1, SP_ADD0, "delattr" F_PAD "pop, " F_CONST8)
DEE_ASM_OPCODE(0x00, 0x5c, 2, _SETATTR_C, SP_SUB2, SP_ADD0, "setattr" F_PAD "pop, " F_CONST8 ", pop")
DEE_ASM_OPCODE(0x00, 0x5d, 2, _GETATTR_THIS_C, SP_SUB0, SP_ADD1, "push" F_PAD "getattr this, " F_CONST8)
DEE_ASM_OPCODE(0x00, 0x5e, 2, _DELATTR_THIS_C, SP_SUB0, SP_ADD0, "delattr" F_PAD "this, " F_CONST8)
DEE_ASM_OPCODE(0x00, 0x5f, 2, _SETATTR_THIS_C, SP_SUB1, SP_ADD0, "setattr" F_PAD "this, " F_CONST8 ", pop")
DEE_ASM_OPCODE(0x00, 0x60, 1, _CMP_EQ, SP_SUB2, SP_ADD1, "cmp" F_PAD "eq, top, pop")
DEE_ASM_OPCODE(0x00, 0x61, 1, _CMP_NE, SP_SUB2, SP_ADD1, "cmp" F_PAD "ne, top, pop")
DEE_ASM_OPCODE(0x00, 0x62, 1, _CMP_GE, SP_SUB2, SP_ADD1, "cmp" F_PAD "ge, top, pop")
DEE_ASM_OPCODE(0x00, 0x63, 1, _CMP_LO, SP_SUB2, SP_ADD1, "cmp" F_PAD "lo, top, pop")
DEE_ASM_OPCODE(0x00, 0x64, 1, _CMP_LE, SP_SUB2, SP_ADD1, "cmp" F_PAD "le, top, pop")
DEE_ASM_OPCODE(0x00, 0x65, 1, _CMP_GR, SP_SUB2, SP_ADD1, "cmp" F_PAD "gr, top, pop")
DEE_ASM_OPCODE(0x00, 0x66, 2, _CLASS_C, SP_SUB1, SP_ADD1, "class" F_PAD "top, " F_CONST8)
DEE_ASM_OPCODE(0x00, 0x67, 3, _CLASS_GC, SP_SUB0, SP_ADD1, "push" F_PAD "class " F_GLOBAL8 ", " F_CONST8)
DEE_ASM_OPCODE(0x00, 0x68, 4, _CLASS_EC, SP_SUB0, SP_ADD1, "push" F_PAD "class " F_EXTERN8 ", " F_CONST8)
DEE_ASM_OPCODE(0x00, 0x69, 2, _DEFCMEMBER, SP_SUB2, SP_ADD1, "defcmember" F_PAD "top, $" F_IMM8 ", pop")
DEE_ASM_OPCODE(0x00, 0x6a, 3, _GETCMEMBER_R, SP_SUB0, SP_ADD1, "push" F_PAD "getcmember " F_REF8 ", $" F_IMM8)
DEE_ASM_OPCODE(0x00, 0x6b, 4, _CALLCMEMBER_THIS_R, SP_SUBIMM3N8, SP_ADD1, "push" F_PAD "callcmember this, " F_REF8 ", $" F_IMM8 ", #" F_IMM8)
DEE_ASM_UNDEFINED(0x00, 0x6c, 1)
DEE_ASM_UNDEFINED(0x00, 0x6d, 1)
DEE_ASM_OPCODE_P(0x00, 0x6e, 3, _FUNCTION_C, SP_SUBIMM2N8, SP_ADD1, "push" F_PAD "function " F_CONST8 ", #" F_IMM8, SP_SUBIMM2N8, SP_ADD0, F_PREFIX ": function " F_CONST8 ", #" F_IMM8)
DEE_ASM_OPCODE_P(0x00, 0x6f, 4, _FUNCTION_C_16, SP_SUBIMM2N16, SP_ADD1, "push" F_PAD "function " F_CONST8 ", #" F_IMM16, SP_SUBIMM2N16, SP_ADD0, F_PREFIX ": function " F_CONST8 ", #" F_IMM16)
DEE_ASM_OPCODE(0x00, 0x70, 1, _CAST_INT, SP_SUB1, SP_ADD1, "cast" F_PAD "top, int")
DEE_ASM_OPCODE(0x00, 0x71, 1, _INV, SP_SUB1, SP_ADD1, "inv" F_PAD "top")
DEE_ASM_OPCODE(0x00, 0x72, 1, _POS, SP_SUB1, SP_ADD1, "pos" F_PAD "top")
DEE_ASM_OPCODE(0x00, 0x73, 1, _NEG, SP_SUB1, SP_ADD1, "neg" F_PAD "top")
DEE_ASM_OPCODE_P(0x00, 0x74, 1, _ADD, SP_SUB2, SP_ADD1, "add" F_PAD "top, pop", SP_SUB1, SP_ADD0, "add" F_PAD F_PREFIX ", pop")
DEE_ASM_OPCODE_P(0x00, 0x75, 1, _SUB, SP_SUB2, SP_ADD1, "sub" F_PAD "top, pop", SP_SUB1, SP_ADD0, "sub" F_PAD F_PREFIX ", pop")
DEE_ASM_OPCODE_P(0x00, 0x76, 1, _MUL, SP_SUB2, SP_ADD1, "mul" F_PAD "top, pop", SP_SUB1, SP_ADD0, "mul" F_PAD F_PREFIX ", pop")
DEE_ASM_OPCODE_P(0x00, 0x77, 1, _DIV, SP_SUB2, SP_ADD1, "div" F_PAD "top, pop", SP_SUB1, SP_ADD0, "div" F_PAD F_PREFIX ", pop")
DEE_ASM_OPCODE_P(0x00, 0x78, 1, _MOD, SP_SUB2, SP_ADD1, "mod" F_PAD "top, pop", SP_SUB1, SP_ADD0, "mod" F_PAD F_PREFIX ", pop")
DEE_ASM_OPCODE_P(0x00, 0x79, 1, _SHL, SP_SUB2, SP_ADD1, "shl" F_PAD "top, pop", SP_SUB1, SP_ADD0, "shl" F_PAD F_PREFIX ", pop")
DEE_ASM_OPCODE_P(0x00, 0x7a, 1, _SHR, SP_SUB2, SP_ADD1, "shr" F_PAD "top, pop", SP_SUB1, SP_ADD0, "shr" F_PAD F_PREFIX ", pop")
DEE_ASM_OPCODE_P(0x00, 0x7b, 1, _AND, SP_SUB2, SP_ADD1, "and" F_PAD "top, pop", SP_SUB1, SP_ADD0, "and" F_PAD F_PREFIX ", pop")
DEE_ASM_OPCODE_P(0x00, 0x7c, 1, _OR, SP_SUB2, SP_ADD1, "or" F_PAD "top, pop", SP_SUB1, SP_ADD0, "or" F_PAD F_PREFIX ", pop")
DEE_ASM_OPCODE_P(0x00, 0x7d, 1, _XOR, SP_SUB2, SP_ADD1, "xor" F_PAD "top, pop", SP_SUB1, SP_ADD0, "xor" F_PAD F_PREFIX ", pop")
DEE_ASM_OPCODE_P(0x00, 0x7e, 1, _POW, SP_SUB2, SP_ADD1, "pow" F_PAD "top, pop", SP_SUB1, SP_ADD0, "pow" F_PAD F_PREFIX ", pop")
DEE_ASM_OPCODE_P(0x00, 0x7f, 1, _INC, SP_SUB0, SP_ADD0, "inc", SP_SUB0, SP_ADD0, "inc" F_PAD F_PREFIX)
DEE_ASM_OPCODE_P(0x00, 0x80, 1, _DEC, SP_SUB0, SP_ADD0, "dec", SP_SUB0, SP_ADD0, "dec" F_PAD F_PREFIX)
DEE_ASM_OPCODE_P(0x00, 0x81, 2, _ADD_SIMM8, SP_SUB1, SP_ADD1, "add" F_PAD "top, $" F_SIMM8, SP_SUB0, SP_ADD0, "add" F_PAD F_PREFIX ", $" F_SIMM8)
DEE_ASM_OPCODE_P(0x00, 0x82, 5, _ADD_IMM32, SP_SUB1, SP_ADD1, "add" F_PAD "top, $" F_IMM32, SP_SUB0, SP_ADD0, "add" F_PAD F_PREFIX ", $" F_IMM32)
DEE_ASM_OPCODE_P(0x00, 0x83, 2, _SUB_SIMM8, SP_SUB1, SP_ADD1, "sub" F_PAD "top, $" F_SIMM8, SP_SUB0, SP_ADD0, "sub" F_PAD F_PREFIX ", $" F_SIMM8)
DEE_ASM_OPCODE_P(0x00, 0x84, 5, _SUB_IMM32, SP_SUB1, SP_ADD1, "sub" F_PAD "top, $" F_IMM32, SP_SUB0, SP_ADD0, "sub" F_PAD F_PREFIX ", $" F_IMM32)
DEE_ASM_OPCODE_P(0x00, 0x85, 2, _MUL_SIMM8, SP_SUB1, SP_ADD1, "mul" F_PAD "top, $" F_SIMM8, SP_SUB0, SP_ADD0, "mul" F_PAD F_PREFIX ", $" F_SIMM8)
DEE_ASM_OPCODE_P(0x00, 0x86, 2, _DIV_SIMM8, SP_SUB1, SP_ADD1, "div" F_PAD "top, $" F_SIMM8, SP_SUB0, SP_ADD0, "div" F_PAD F_PREFIX ", $" F_SIMM8)
DEE_ASM_OPCODE_P(0x00, 0x87, 2, _MOD_SIMM8, SP_SUB1, SP_ADD1, "mod" F_PAD "top, $" F_SIMM8, SP_SUB0, SP_ADD0, "mod" F_PAD F_PREFIX ", $" F_SIMM8)
DEE_ASM_OPCODE_P(0x00, 0x88, 2, _SHL_IMM8, SP_SUB1, SP_ADD1, "shl" F_PAD "top, $" F_IMM8, SP_SUB0, SP_ADD0, "shl" F_PAD F_PREFIX ", $" F_IMM8)
DEE_ASM_OPCODE_P(0x00, 0x89, 2, _SHR_IMM8, SP_SUB1, SP_ADD1, "shr" F_PAD "top, $" F_IMM8, SP_SUB0, SP_ADD0, "shr" F_PAD F_PREFIX ", $" F_IMM8)
DEE_ASM_OPCODE_P(0x00, 0x8a, 5, _AND_IMM32, SP_SUB1, SP_ADD1, "and" F_PAD "top, $" F_IMM32, SP_SUB0, SP_ADD0, "and" F_PAD F_PREFIX ", $" F_IMM32)
DEE_ASM_OPCODE_P(0x00, 0x8b, 5, _OR_IMM32, SP_SUB1, SP_ADD1, "or" F_PAD "top, $" F_IMM32, SP_SUB0, SP_ADD0, "or" F_PAD F_PREFIX ", $" F_IMM32)
DEE_ASM_OPCODE_P(0x00, 0x8c, 5, _XOR_IMM32, SP_SUB1, SP_ADD1, "xor" F_PAD "top, $" F_IMM32, SP_SUB0, SP_ADD0, "xor" F_PAD F_PREFIX ", $" F_IMM32)
DEE_ASM_OPCODE(0x00, 0x8d, 1, _ISNONE, SP_SUB1, SP_ADD1, "instanceof" F_PAD "top, none")
DEE_ASM_UNDEFINED(0x00, 0x8e, 1)
DEE_ASM_OPCODE(0x00, 0x8f, 1, _DELOP, SP_SUB0, SP_ADD0, "")
DEE_ASM_OPCODE_P(0x00, 0x90, 1, _NOP, SP_SUB0, SP_ADD0, "nop", SP_SUB0, SP_ADD0, "nop" F_PAD F_PREFIX)
DEE_ASM_OPCODE(0x00, 0x91, 1, _PRINT, SP_SUB1, SP_ADD0, "print" F_PAD "pop")
DEE_ASM_OPCODE(0x00, 0x92, 1, _PRINT_SP, SP_SUB1, SP_ADD0, "print" F_PAD "pop, sp")
DEE_ASM_OPCODE(0x00, 0x93, 1, _PRINT_NL, SP_SUB1, SP_ADD0, "print" F_PAD "pop, nl")
DEE_ASM_OPCODE(0x00, 0x94, 1, _PRINTNL, SP_SUB0, SP_ADD0, "print" F_PAD "nl")
DEE_ASM_OPCODE(0x00, 0x95, 1, _PRINTALL, SP_SUB1, SP_ADD0, "print" F_PAD "pop...")
DEE_ASM_OPCODE(0x00, 0x96, 1, _PRINTALL_SP, SP_SUB1, SP_ADD0, "print" F_PAD "pop..., sp")
DEE_ASM_OPCODE(0x00, 0x97, 1, _PRINTALL_NL, SP_SUB1, SP_ADD0, "print" F_PAD "pop..., nl")
DEE_ASM_UNDEFINED(0x00, 0x98, 1)
DEE_ASM_OPCODE(0x00, 0x99, 1, _FPRINT, SP_SUB2, SP_ADD1, "print" F_PAD "top, pop")
DEE_ASM_OPCODE(0x00, 0x9a, 1, _FPRINT_SP, SP_SUB2, SP_ADD1, "print" F_PAD "top, pop, sp")
DEE_ASM_OPCODE(0x00, 0x9b, 1, _FPRINT_NL, SP_SUB2, SP_ADD1, "print" F_PAD "top, pop, nl")
DEE_ASM_OPCODE(0x00, 0x9c, 1, _FPRINTNL, SP_SUB1, SP_ADD1, "print" F_PAD "top, nl")
DEE_ASM_OPCODE(0x00, 0x9d, 1, _FPRINTALL, SP_SUB2, SP_ADD1, "print" F_PAD "top, pop...")
DEE_ASM_OPCODE(0x00, 0x9e, 1, _FPRINTALL_SP, SP_SUB2, SP_ADD1, "print" F_PAD "top, pop..., sp")
DEE_ASM_OPCODE(0x00, 0x9f, 1, _FPRINTALL_NL, SP_SUB2, SP_ADD1, "print" F_PAD "top, pop..., nl")
DEE_ASM_UNDEFINED(0x00, 0xa0, 1)
DEE_ASM_OPCODE(0x00, 0xa1, 2, _PRINT_C, SP_SUB0, SP_ADD0, "print" F_PAD F_CONST8)
DEE_ASM_OPCODE(0x00, 0xa2, 2, _PRINT_C_SP, SP_SUB0, SP_ADD0, "print" F_PAD F_CONST8 ", sp")
DEE_ASM_OPCODE(0x00, 0xa3, 2, _PRINT_C_NL, SP_SUB0, SP_ADD0, "print" F_PAD F_CONST8 ", nl")
DEE_ASM_OPCODE(0x00, 0xa4, 3, _RANGE_0_I16, SP_SUB0, SP_ADD1, "push" F_PAD "range $0, $" F_IMM16)
DEE_ASM_UNDEFINED(0x00, 0xa5, 1)
DEE_ASM_OPCODE(0x00, 0xa6, 1, _ENTER, SP_SUB1, SP_ADD1, "enter" F_PAD "top")
DEE_ASM_OPCODE(0x00, 0xa7, 1, _LEAVE, SP_SUB1, SP_ADD0, "leave" F_PAD "pop")
DEE_ASM_UNDEFINED(0x00, 0xa8, 1)
DEE_ASM_OPCODE(0x00, 0xa9, 2, _FPRINT_C, SP_SUB1, SP_ADD1, "print" F_PAD "top, " F_CONST8)
DEE_ASM_OPCODE(0x00, 0xaa, 2, _FPRINT_C_SP, SP_SUB1, SP_ADD1, "print" F_PAD "top, " F_CONST8 ", sp")
DEE_ASM_OPCODE(0x00, 0xab, 2, _FPRINT_C_NL, SP_SUB1, SP_ADD1, "print" F_PAD "top, " F_CONST8 ", nl")
DEE_ASM_OPCODE(0x00, 0xac, 1, _RANGE, SP_SUB2, SP_ADD1, "range" F_PAD "top, pop")
DEE_ASM_OPCODE(0x00, 0xad, 1, _RANGE_DEF, SP_SUB1, SP_ADD1, "push" F_PAD "range default, pop")
DEE_ASM_OPCODE(0x00, 0xae, 1, _RANGE_STEP, SP_SUB3, SP_ADD1, "range" F_PAD "top, pop, pop")
DEE_ASM_OPCODE(0x00, 0xaf, 1, _RANGE_STEP_DEF, SP_SUB2, SP_ADD1, "push" F_PAD "range default, pop, pop")
DEE_ASM_OPCODE(0x00, 0xb0, 1, _CONTAINS, SP_SUB2, SP_ADD1, "contains" F_PAD "top, pop")
DEE_ASM_OPCODE(0x00, 0xb1, 2, _CONTAINS_C, SP_SUB1, SP_ADD1, "push" F_PAD "contains " F_CONST8 ", pop")
DEE_ASM_OPCODE(0x00, 0xb2, 1, _GETITEM, SP_SUB2, SP_ADD1, "getitem" F_PAD "top, pop")
DEE_ASM_OPCODE(0x00, 0xb3, 3, _GETITEM_I, SP_SUB1, SP_ADD1, "getitem" F_PAD "top, $" F_IMM16)
DEE_ASM_OPCODE(0x00, 0xb4, 2, _GETITEM_C, SP_SUB1, SP_ADD1, "getitem" F_PAD "top, " F_CONST8)
DEE_ASM_OPCODE(0x00, 0xb5, 1, _GETSIZE, SP_SUB1, SP_ADD1, "getsize" F_PAD "top")
DEE_ASM_OPCODE(0x00, 0xb6, 1, _SETITEM, SP_SUB3, SP_ADD0, "setitem" F_PAD "pop, pop, pop")
DEE_ASM_OPCODE(0x00, 0xb7, 3, _SETITEM_I, SP_SUB2, SP_ADD0, "setitem" F_PAD "pop, $" F_IMM16 ", pop")
DEE_ASM_OPCODE(0x00, 0xb8, 2, _SETITEM_C, SP_SUB2, SP_ADD0, "setitem" F_PAD "pop, " F_CONST8 ", pop")
DEE_ASM_OPCODE(0x00, 0xb9, 1, _ITERSELF, SP_SUB1, SP_ADD1, "iterself" F_PAD "top")
DEE_ASM_OPCODE(0x00, 0xba, 1, _DELITEM, SP_SUB2, SP_ADD0, "delitem" F_PAD "pop, pop")
DEE_ASM_OPCODE(0x00, 0xbb, 1, _GETRANGE, SP_SUB3, SP_ADD1, "getrange" F_PAD "top, pop, pop")
DEE_ASM_OPCODE(0x00, 0xbc, 1, _GETRANGE_PN, SP_SUB2, SP_ADD1, "getrange" F_PAD "top, pop, none")
DEE_ASM_OPCODE(0x00, 0xbd, 1, _GETRANGE_NP, SP_SUB2, SP_ADD1, "getrange" F_PAD "top, none, pop")
DEE_ASM_OPCODE(0x00, 0xbe, 3, _GETRANGE_PI, SP_SUB2, SP_ADD1, "getrange" F_PAD "top, pop, $" F_SIMM16)
DEE_ASM_OPCODE(0x00, 0xbf, 3, _GETRANGE_IP, SP_SUB2, SP_ADD1, "getrange" F_PAD "top, $" F_SIMM16 ", pop")
DEE_ASM_OPCODE(0x00, 0xc0, 3, _GETRANGE_NI, SP_SUB1, SP_ADD1, "getrange" F_PAD "top, none, $" F_SIMM16)
DEE_ASM_OPCODE(0x00, 0xc1, 3, _GETRANGE_IN, SP_SUB1, SP_ADD1, "getrange" F_PAD "top, $" F_SIMM16 ", none")
DEE_ASM_OPCODE(0x00, 0xc2, 5, _GETRANGE_II, SP_SUB1, SP_ADD1, "getrange" F_PAD "top, $" F_SIMM16 ", $" F_SIMM16)
DEE_ASM_OPCODE(0x00, 0xc3, 1, _DELRANGE, SP_SUB3, SP_ADD0, "delrange" F_PAD "pop, pop, pop")
DEE_ASM_OPCODE(0x00, 0xc4, 1, _SETRANGE, SP_SUB4, SP_ADD0, "setrange" F_PAD "pop, pop, pop, pop")
DEE_ASM_OPCODE(0x00, 0xc5, 1, _SETRANGE_PN, SP_SUB3, SP_ADD0, "setrange" F_PAD "pop, pop, none, pop")
DEE_ASM_OPCODE(0x00, 0xc6, 1, _SETRANGE_NP, SP_SUB3, SP_ADD0, "setrange" F_PAD "pop, none, pop, pop")
DEE_ASM_OPCODE(0x00, 0xc7, 3, _SETRANGE_PI, SP_SUB3, SP_ADD0, "setrange" F_PAD "pop, pop, $" F_SIMM16 ", pop")
DEE_ASM_OPCODE(0x00, 0xc8, 3, _SETRANGE_IP, SP_SUB3, SP_ADD0, "setrange" F_PAD "pop, $" F_SIMM16 ", pop, pop")
DEE_ASM_OPCODE(0x00, 0xc9, 3, _SETRANGE_NI, SP_SUB2, SP_ADD0, "setrange" F_PAD "pop, none, $" F_SIMM16 ", pop")
DEE_ASM_OPCODE(0x00, 0xca, 3, _SETRANGE_IN, SP_SUB2, SP_ADD0, "setrange" F_PAD "pop, $" F_SIMM16 ", none, pop")
DEE_ASM_OPCODE(0x00, 0xcb, 5, _SETRANGE_II, SP_SUB2, SP_ADD0, "setrange" F_PAD "pop, $" F_SIMM16 ", $" F_SIMM16 ", pop")
DEE_ASM_OPCODE(0x00, 0xcc, 1, _BREAKPOINT, SP_SUB0, SP_ADD0, "debug" F_PAD "break")
DEE_ASM_OPCODE(0x00, 0xcd, 1, _UD, SP_SUB0, SP_ADD0, "ud")
DEE_ASM_OPCODE(0x00, 0xce, 4, _CALLATTR_C_KW, SP_SUBIMM2N8_MINUS1, SP_ADD1, "callattr" F_PAD "top, " F_CONST8 ", #" F_IMM8 ", " F_CONST8)
DEE_ASM_OPCODE(0x00, 0xcf, 3, _CALLATTR_C_TUPLE_KW, SP_SUB2, SP_ADD1, "callattr" F_PAD "top, " F_CONST8 ", pop..., " F_CONST8)
DEE_ASM_OPCODE(0x00, 0xd0, 2, _CALLATTR, SP_SUBIMM1N8_MINUS2, SP_ADD1, "callattr" F_PAD "top, pop, #" F_IMM8)
DEE_ASM_OPCODE(0x00, 0xd1, 1, _CALLATTR_TUPLE, SP_SUB3, SP_ADD1, "callattr" F_PAD "top, pop, pop...")
DEE_ASM_OPCODE(0x00, 0xd2, 3, _CALLATTR_C, SP_SUBIMM2N8_MINUS1, SP_ADD1, "callattr" F_PAD "top, " F_CONST8 ", #" F_IMM8)
DEE_ASM_OPCODE(0x00, 0xd3, 2, _CALLATTR_C_TUPLE, SP_SUB2, SP_ADD1, "callattr" F_PAD "top, " F_CONST8 ", pop...")
DEE_ASM_OPCODE(0x00, 0xd4, 3, _CALLATTR_THIS_C, SP_SUBIMM2N8, SP_ADD1, "push" F_PAD "callattr this, " F_CONST8 ", #" F_IMM8)
DEE_ASM_OPCODE(0x00, 0xd5, 2, _CALLATTR_THIS_C_TUPLE, SP_SUB1, SP_ADD1, "push" F_PAD "callattr this, " F_CONST8 ", pop...")
DEE_ASM_OPCODE(0x00, 0xd6, 3, _CALLATTR_C_SEQ, SP_SUBIMM2N8_MINUS1, SP_ADD1, "callattr" F_PAD "top, " F_CONST8 ", [#" F_IMM8 "]")
DEE_ASM_OPCODE(0x00, 0xd7, 3, _CALLATTR_C_MAP, SP_SUBIMM2N8X2_MINUS1, SP_ADD1, "callattr" F_PAD "top, " F_CONST8 ", {#" F_IMM8_X2 "}")
DEE_ASM_OPCODE(0x00, 0xd8, 3, _GETMEMBER_THIS_R, SP_SUB0, SP_ADD1, "push" F_PAD "getmember this, " F_REF8 ", $" F_IMM8)
DEE_ASM_OPCODE(0x00, 0xd9, 3, _DELMEMBER_THIS_R, SP_SUB0, SP_ADD0, "delmember" F_PAD "this, " F_REF8 ", $" F_IMM8)
DEE_ASM_OPCODE(0x00, 0xda, 3, _SETMEMBER_THIS_R, SP_SUB1, SP_ADD0, "setmember" F_PAD "this, " F_REF8 ", $" F_IMM8 ", pop")
DEE_ASM_OPCODE(0x00, 0xdb, 3, _SETMEMBERI_THIS_R, SP_SUB1, SP_ADD0, "setmemberi" F_PAD "this, " F_REF8 ", $" F_IMM8 ", pop")
DEE_ASM_OPCODE(0x00, 0xdc, 3, _BOUNDMEMBER_THIS_R, SP_SUB0, SP_ADD1, "push" F_PAD "boundmember this, " F_REF8 ", $" F_IMM8)
DEE_ASM_OPCODE(0x00, 0xdd, 4, _CALL_EXTERN, SP_SUBIMM3N8, SP_ADD1, "push" F_PAD "call " F_EXTERN8 ", #" F_IMM8)
DEE_ASM_OPCODE(0x00, 0xde, 3, _CALL_GLOBAL, SP_SUBIMM2N8, SP_ADD1, "push" F_PAD "call " F_GLOBAL8 ", #" F_IMM8)
DEE_ASM_OPCODE(0x00, 0xdf, 3, _CALL_LOCAL, SP_SUBIMM2N8, SP_ADD1, "push" F_PAD "call " F_LOCAL8 ", #" F_IMM8)
DEE_ASM_UNDEFINED(0x00, 0xe0, 1)
DEE_ASM_UNDEFINED(0x00, 0xe1, 1)
DEE_ASM_UNDEFINED(0x00, 0xe2, 1)
DEE_ASM_UNDEFINED(0x00, 0xe3, 1)
DEE_ASM_UNDEFINED(0x00, 0xe4, 1)
DEE_ASM_UNDEFINED(0x00, 0xe5, 1)
DEE_ASM_UNDEFINED(0x00, 0xe6, 1)
DEE_ASM_UNDEFINED(0x00, 0xe7, 1)
DEE_ASM_UNDEFINED(0x00, 0xe8, 1)
DEE_ASM_UNDEFINED(0x00, 0xe9, 1)
DEE_ASM_UNDEFINED(0x00, 0xea, 1)
DEE_ASM_UNDEFINED(0x00, 0xeb, 1)
DEE_ASM_UNDEFINED(0x00, 0xec, 1)
DEE_ASM_UNDEFINED(0x00, 0xed, 1)
DEE_ASM_UNDEFINED(0x00, 0xee, 1)
DEE_ASM_UNDEFINED(0x00, 0xef, 1)
DEE_ASM_EXTENDED(0x00, 0xf0, 1, _EXTENDED1)
DEE_ASM_UNDEFINED(0x00, 0xf1, 1)
DEE_ASM_UNDEFINED(0x00, 0xf2, 1)
DEE_ASM_UNDEFINED(0x00, 0xf3, 1)
DEE_ASM_UNDEFINED(0x00, 0xf4, 1)
DEE_ASM_UNDEFINED(0x00, 0xf5, 1)
DEE_ASM_UNDEFINED(0x00, 0xf6, 1)
DEE_ASM_UNDEFINED(0x00, 0xf7, 1)
DEE_ASM_UNDEFINED_PREFIX(0x00, 0xf8, 1)
DEE_ASM_UNDEFINED_PREFIX(0x00, 0xf9, 1)
DEE_ASM_PREFIX(0x00, 0xfa, 2, _STACK)
DEE_ASM_UNDEFINED_PREFIX(0x00, 0xfb, 1)
DEE_ASM_PREFIX(0x00, 0xfc, 3, _EXTERN)
DEE_ASM_PREFIX(0x00, 0xfd, 2, _STATIC)
DEE_ASM_PREFIX(0x00, 0xfe, 2, _GLOBAL)
DEE_ASM_PREFIX(0x00, 0xff, 2, _LOCAL)
DEE_ASM_END(0x00)
#endif /* DEE_ASM_WANT_TABLE(0x00) */

#if DEE_ASM_WANT_TABLE(0xf0)
DEE_ASM_BEGIN(0xf0)
DEE_ASM_UNDEFINED(0xf0, 0x00, 2)
DEE_ASM_UNDEFINED(0xf0, 0x01, 2)
DEE_ASM_UNDEFINED(0xf0, 0x02, 2)
DEE_ASM_UNDEFINED(0xf0, 0x03, 2)
DEE_ASM_UNDEFINED(0xf0, 0x04, 2)
DEE_ASM_UNDEFINED(0xf0, 0x05, 2)
DEE_ASM_OPCODE(0xf0, 0x06, 3, _ENDCATCH_N, SP_SUB0, SP_ADD0, "end" F_PAD "catch, #" F_IMM8_PLUS1)
DEE_ASM_OPCODE(0xf0, 0x07, 3, _ENDFINALLY_N, SP_SUB0, SP_ADD0, "end" F_PAD "finally, #" F_IMM8_PLUS1)
DEE_ASM_OPCODE(0xf0, 0x08, 5, 16_CALL_KW, SP_SUBIMM2N8_MINUS1, SP_ADD1, "call" F_PAD "top, #" F_IMM8 ", " F_CONST16)
DEE_ASM_OPCODE(0xf0, 0x09, 4, 16_CALL_TUPLE_KW, SP_SUB2, SP_ADD1, "call" F_PAD "top, pop..., " F_CONST16)
DEE_ASM_UNDEFINED(0xf0, 0x0a, 2)
DEE_ASM_OPCODE(0xf0, 0x0b, 4, 16_PUSH_BND_ARG, SP_SUB0, SP_ADD1, "push" F_PAD "bound " F_ARG16)
DEE_ASM_OPCODE(0xf0, 0x0c, 6, 16_PUSH_BND_EXTERN, SP_SUB0, SP_ADD1, "push" F_PAD "bound " F_EXTERN16)
DEE_ASM_OPCODE(0xf0, 0x0d, 2, 16_PUSH_BND_STATIC, SP_SUB0, SP_ADD1, "push" F_PAD "bound " F_STATIC16)
DEE_ASM_OPCODE(0xf0, 0x0e, 4, 16_PUSH_BND_GLOBAL, SP_SUB0, SP_ADD1, "push" F_PAD "bound " F_GLOBAL16)
DEE_ASM_OPCODE(0xf0, 0x0f, 4, 16_PUSH_BND_LOCAL, SP_SUB0, SP_ADD1, "push" F_PAD "bound " F_LOCAL16)
DEE_ASM_OPCODE_P(0xf0, 0x10, 3, _FOREACH_KEY, SP_SUB1, SP_ADD2, "foreach_key" F_PAD "top, " F_SDISP8, SP_SUB0, SP_ADD1, "foreach_key" F_PAD F_PREFIX ", " F_SDISP8)
DEE_ASM_OPCODE_P(0xf0, 0x11, 4, _FOREACH_KEY16, SP_SUB1, SP_ADD2, "foreach_key" F_PAD "top, " F_SDISP16, SP_SUB0, SP_ADD1, "foreach_key" F_PAD F_PREFIX ", " F_SDISP16)
DEE_ASM_OPCODE_P(0xf0, 0x12, 3, _FOREACH_VALUE, SP_SUB1, SP_ADD2, "foreach_value" F_PAD "top, " F_SDISP8, SP_SUB0, SP_ADD1, "foreach_value" F_PAD F_PREFIX ", " F_SDISP8)
DEE_ASM_OPCODE_P(0xf0, 0x13, 4, _FOREACH_VALUE16, SP_SUB1, SP_ADD2, "foreach_value" F_PAD "top, " F_SDISP16, SP_SUB0, SP_ADD1, "foreach_value" F_PAD F_PREFIX ", " F_SDISP16)
DEE_ASM_OPCODE(0xf0, 0x14, 6, 32_JMP, SP_SUB0, SP_ADD0, "jmp" F_PAD F_SDISP32)
DEE_ASM_UNDEFINED(0xf0, 0x15, 2)
DEE_ASM_OPCODE_P(0xf0, 0x16, 3, _FOREACH_PAIR, SP_SUB1, SP_ADD3, "foreach_pair" F_PAD "top, " F_SDISP8, SP_SUB0, SP_ADD2, "foreach_pair" F_PAD F_PREFIX ", " F_SDISP8)
DEE_ASM_OPCODE_P(0xf0, 0x17, 4, _FOREACH_PAIR16, SP_SUB1, SP_ADD3, "foreach_pair" F_PAD "top, " F_SDISP16, SP_SUB0, SP_ADD2, "foreach_pair" F_PAD F_PREFIX ", " F_SDISP16)
DEE_ASM_OPCODE(0xf0, 0x18, 2, _JMP_POP_POP, SP_SUB2, SP_ADD0, "jmp" F_PAD "pop, #pop")
DEE_ASM_OPCODE_P(0xf0, 0x19, 5, 16_OPERATOR, SP_SUBIMM4N8_MINUS1, SP_ADD1, "op" F_PAD "top, $" F_IMM16 ", #" F_IMM8, SP_SUBIMM4N8, SP_ADD1, F_PREFIX ": push op $" F_IMM16 ", #" F_IMM8)
DEE_ASM_OPCODE_P(0xf0, 0x1a, 4, 16_OPERATOR_TUPLE, SP_SUB2, SP_ADD1, "op" F_PAD "top, $" F_IMM16 ", pop...", SP_SUB1, SP_ADD1, F_PREFIX ": push op $" F_IMM16 ", pop...")
DEE_ASM_OPCODE(0xf0, 0x1b, 3, _CALL_SEQ, SP_SUBIMM2N8_MINUS1, SP_ADD1, "call" F_PAD "top, [#" F_IMM8 "]")
DEE_ASM_OPCODE(0xf0, 0x1c, 3, _CALL_MAP, SP_SUBIMM2N8X2_MINUS1, SP_ADD1, "call" F_PAD "top, {#" F_IMM8_X2 "}")
DEE_ASM_OPCODE(0xf0, 0x1d, 4, 16_DEL_STATIC, SP_SUB0, SP_ADD0, "del" F_PAD F_STATIC16)
DEE_ASM_OPCODE(0xf0, 0x1e, 4, 16_DEL_GLOBAL, SP_SUB0, SP_ADD0, "del" F_PAD F_GLOBAL16)
DEE_ASM_OPCODE(0xf0, 0x1f, 4, 16_DEL_LOCAL, SP_SUB0, SP_ADD0, "del" F_PAD F_LOCAL16)
DEE_ASM_OPCODE(0xf0, 0x20, 2, _CALL_TUPLE_KWDS, SP_SUB3, SP_ADD1, "call" F_PAD "top, pop..., pop")
DEE_ASM_OPCODE_P(0xf0, 0x21, 4, 16_LROT, SP_SUBIMM2N16_MINUS3, SP_ADDIMM2N16_PLUS3, "lrot" F_PAD "#" F_IMM16_PLUS3, SP_SUBIMM2N16_MINUS2, SP_ADDIMM2N16_PLUS2, "lrot" F_PAD "#" F_IMM16_PLUS2 ", " F_PREFIX)
DEE_ASM_OPCODE_P(0xf0, 0x22, 4, 16_RROT, SP_SUBIMM2N16, SP_ADDIMM2N16, "rrot" F_PAD "#" F_IMM16_PLUS3, SP_SUBIMM2N16, SP_ADDIMM2N16, "rrot" F_PAD "#" F_IMM16_PLUS2 ", " F_PREFIX)
DEE_ASM_OPCODE(0xf0, 0x23, 2, _THISCALL_TUPLE, SP_SUB3, SP_ADD1, "call" F_PAD "top, pop, pop...")
DEE_ASM_OPCODE_P(0xf0, 0x24, 4, 16_DUP_N, SP_SUBIMM2N16, SP_ADDIMM2N16_PLUS1, "dup" F_PAD F_SP_SUB16_SUB2, SP_SUBIMM2N16_MINUS1, SP_ADDIMM2N16_PLUS1, "mov" F_PAD F_PREFIX ", " F_SP_SUB16_SUB2)
DEE_ASM_UNDEFINED(0xf0, 0x25, 2)
DEE_ASM_OPCODE_P(0xf0, 0x26, 4, 16_POP_N, SP_SUBIMM2N16_MINUS1, SP_ADDIMM2N16, "pop" F_PAD F_SP_SUB16_SUB2, SP_SUBIMM2N16_MINUS1, SP_ADDIMM2N16_PLUS1, "mov" F_PAD F_SP_SUB16_SUB2 ", " F_PREFIX)
DEE_ASM_OPCODE(0xf0, 0x27, 4, 16_ADJSTACK, SP_SUBADJSTACK2N16, SP_ADDADJSTACK2N16, "adjstack" F_PAD F_SP_PLUSS16)
DEE_ASM_UNDEFINED(0xf0, 0x28, 2)
DEE_ASM_OPCODE(0xf0, 0x29, 4, 16_SUPER_THIS_R, SP_SUB0, SP_ADD1, "push" F_PAD "super this, " F_REF16)
DEE_ASM_UNDEFINED(0xf0, 0x2a, 2)
DEE_ASM_UNDEFINED(0xf0, 0x2b, 2)
DEE_ASM_OPCODE_P(0xf0, 0x2c, 6, 16_POP_EXTERN, SP_SUB1, SP_ADD0, "pop" F_PAD F_EXTERN16, SP_SUB0, SP_ADD0, "mov" F_PAD F_EXTERN16 ", " F_PREFIX)
DEE_ASM_OPCODE_P(0xf0, 0x2d, 4, 16_POP_STATIC, SP_SUB1, SP_ADD0, "pop" F_PAD F_STATIC16, SP_SUB0, SP_ADD0, "mov" F_PAD F_STATIC16 ", " F_PREFIX)
DEE_ASM_OPCODE_P(0xf0, 0x2e, 4, 16_POP_GLOBAL, SP_SUB1, SP_ADD0, "pop" F_PAD F_GLOBAL16, SP_SUB0, SP_ADD0, "mov" F_PAD F_GLOBAL16 ", " F_PREFIX)
DEE_ASM_OPCODE_P(0xf0, 0x2f, 4, 16_POP_LOCAL, SP_SUB1, SP_ADD0, "pop" F_PAD F_LOCAL16, SP_SUB0, SP_ADD0, "mov" F_PAD F_LOCAL16 ", " F_PREFIX)
DEE_ASM_UNDEFINED(0xf0, 0x30, 2)
DEE_ASM_UNDEFINED(0xf0, 0x31, 2)
DEE_ASM_OPCODE_P(0xf0, 0x32, 2, _PUSH_EXCEPT, SP_SUB0, SP_ADD1, "push" F_PAD "except", SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", except")
DEE_ASM_OPCODE_P(0xf0, 0x33, 2, _PUSH_THIS, SP_SUB0, SP_ADD1, "push" F_PAD "this", SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", this")
DEE_ASM_OPCODE_P(0xf0, 0x34, 2, _PUSH_THIS_MODULE, SP_SUB0, SP_ADD1, "push" F_PAD "this_module", SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", this_module")
DEE_ASM_OPCODE_P(0xf0, 0x35, 2, _PUSH_THIS_FUNCTION, SP_SUB0, SP_ADD1, "push" F_PAD "this_function", SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", this_function")
DEE_ASM_OPCODE_P(0xf0, 0x36, 4, 16_PUSH_MODULE, SP_SUB0, SP_ADD1, "push" F_PAD F_MODULE16, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_MODULE16)
DEE_ASM_OPCODE_P(0xf0, 0x37, 4, 16_PUSH_ARG, SP_SUB0, SP_ADD1, "push" F_PAD F_ARG16, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_ARG16)
DEE_ASM_OPCODE_P(0xf0, 0x38, 4, 16_PUSH_CONST, SP_SUB0, SP_ADD1, "push" F_PAD F_CONST16, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_CONST16)
DEE_ASM_OPCODE_P(0xf0, 0x39, 4, 16_PUSH_REF, SP_SUB0, SP_ADD1, "push" F_PAD F_REF16, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_REF16)
DEE_ASM_UNDEFINED(0xf0, 0x3a, 2)
DEE_ASM_UNDEFINED(0xf0, 0x3b, 2)
DEE_ASM_OPCODE_P(0xf0, 0x3c, 6, 16_PUSH_EXTERN, SP_SUB0, SP_ADD1, "push" F_PAD F_EXTERN16, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_EXTERN16)
DEE_ASM_OPCODE_P(0xf0, 0x3d, 4, 16_PUSH_STATIC, SP_SUB0, SP_ADD1, "push" F_PAD F_STATIC16, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_STATIC16)
DEE_ASM_OPCODE_P(0xf0, 0x3e, 4, 16_PUSH_GLOBAL, SP_SUB0, SP_ADD1, "push" F_PAD F_GLOBAL16, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_GLOBAL16)
DEE_ASM_OPCODE_P(0xf0, 0x3f, 4, 16_PUSH_LOCAL, SP_SUB0, SP_ADD1, "push" F_PAD F_LOCAL16, SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", " F_LOCAL16)
DEE_ASM_OPCODE(0xf0, 0x40, 2, _CAST_HASHSET, SP_SUB1, SP_ADD1, "cast" F_PAD "top, HashSet")
DEE_ASM_OPCODE(0xf0, 0x41, 2, _CAST_DICT, SP_SUB1, SP_ADD1, "cast" F_PAD "top, Dict")
DEE_ASM_OPCODE(0xf0, 0x42, 4, 16_PACK_TUPLE, SP_SUBIMM2N16, SP_ADD1, "push" F_PAD "pack Tuple, #" F_IMM16)
DEE_ASM_OPCODE(0xf0, 0x43, 4, 16_PACK_LIST, SP_SUBIMM2N16, SP_ADD1, "push" F_PAD "pack List, #" F_IMM16)
DEE_ASM_UNDEFINED(0xf0, 0x44, 2)
DEE_ASM_UNDEFINED(0xf0, 0x45, 2)
DEE_ASM_OPCODE_P(0xf0, 0x46, 4, 16_UNPACK, SP_SUB1, SP_ADDIMM2N16, "unpack" F_PAD "pop, #" F_IMM16, SP_SUB0, SP_ADDIMM2N16, "unpack" F_PAD F_PREFIX ", #" F_IMM16)
DEE_ASM_UNDEFINED(0xf0, 0x47, 2)
DEE_ASM_UNDEFINED(0xf0, 0x48, 2)
DEE_ASM_UNDEFINED(0xf0, 0x49, 2)
DEE_ASM_UNDEFINED(0xf0, 0x4a, 2)
DEE_ASM_UNDEFINED(0xf0, 0x4b, 2)
DEE_ASM_UNDEFINED(0xf0, 0x4c, 2)
DEE_ASM_UNDEFINED(0xf0, 0x4d, 2)
DEE_ASM_UNDEFINED(0xf0, 0x4e, 2)
DEE_ASM_UNDEFINED(0xf0, 0x4f, 2)
DEE_ASM_OPCODE_P(0xf0, 0x50, 2, _PUSH_TRUE, SP_SUB0, SP_ADD1, "push" F_PAD "true", SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", true")
DEE_ASM_OPCODE_P(0xf0, 0x51, 2, _PUSH_FALSE, SP_SUB0, SP_ADD1, "push" F_PAD "false", SP_SUB0, SP_ADD0, "mov" F_PAD F_PREFIX ", false")
DEE_ASM_OPCODE(0xf0, 0x52, 3, _PACK_HASHSET, SP_SUBIMM2N8, SP_ADD1, "push" F_PAD "pack HashSet, #" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0x53, 3, _PACK_DICT, SP_SUBIMM2N8X2, SP_ADD1, "push" F_PAD "pack Dict, #" F_IMM8_X2)
DEE_ASM_UNDEFINED(0xf0, 0x54, 2)
DEE_ASM_UNDEFINED(0xf0, 0x55, 2)
DEE_ASM_UNDEFINED(0xf0, 0x56, 2)
DEE_ASM_UNDEFINED(0xf0, 0x57, 2)
DEE_ASM_UNDEFINED(0xf0, 0x58, 2)
DEE_ASM_OPCODE(0xf0, 0x59, 2, _BOUNDITEM, SP_SUB2, SP_ADD1, "bounditem" F_PAD "top, pop")
DEE_ASM_OPCODE(0xf0, 0x5a, 4, 16_GETATTR_C, SP_SUB1, SP_ADD1, "getattr" F_PAD "top, " F_CONST16)
DEE_ASM_OPCODE(0xf0, 0x5b, 4, 16_DELATTR_C, SP_SUB1, SP_ADD0, "delattr" F_PAD "pop, " F_CONST16)
DEE_ASM_OPCODE(0xf0, 0x5c, 4, 16_SETATTR_C, SP_SUB2, SP_ADD0, "setattr" F_PAD "pop, " F_CONST16 ", pop")
DEE_ASM_OPCODE(0xf0, 0x5d, 4, 16_GETATTR_THIS_C, SP_SUB0, SP_ADD1, "push" F_PAD "getattr this, " F_CONST16)
DEE_ASM_OPCODE(0xf0, 0x5e, 4, 16_DELATTR_THIS_C, SP_SUB0, SP_ADD0, "delattr" F_PAD "this, " F_CONST16)
DEE_ASM_OPCODE(0xf0, 0x5f, 4, 16_SETATTR_THIS_C, SP_SUB1, SP_ADD0, "setattr" F_PAD "this, " F_CONST16 ", pop")
DEE_ASM_OPCODE(0xf0, 0x60, 2, _CMP_SO, SP_SUB2, SP_ADD1, "cmp" F_PAD "so, top, pop")
DEE_ASM_OPCODE(0xf0, 0x61, 2, _CMP_DO, SP_SUB2, SP_ADD1, "cmp" F_PAD "do, top, pop")
DEE_ASM_OPCODE(0xf0, 0x62, 4, 16_PACK_HASHSET, SP_SUBIMM2N16, SP_ADD1, "push" F_PAD "pack HashSet, #" F_IMM16)
DEE_ASM_OPCODE(0xf0, 0x63, 4, 16_PACK_DICT, SP_SUBIMM2N16X2, SP_ADD1, "push" F_PAD "pack Dict, #" F_IMM16_X2)
DEE_ASM_OPCODE(0xf0, 0x64, 4, 16_GETCMEMBER, SP_SUB1, SP_ADD1, "getcmember" F_PAD "top, $" F_IMM16)
DEE_ASM_OPCODE(0xf0, 0x65, 2, _CLASS, SP_SUB2, SP_ADD1, "class" F_PAD "top, pop")
DEE_ASM_OPCODE(0xf0, 0x66, 4, 16_CLASS_C, SP_SUB1, SP_ADD1, "class" F_PAD "top, " F_CONST16)
DEE_ASM_OPCODE(0xf0, 0x67, 6, 16_CLASS_GC, SP_SUB0, SP_ADD1, "push" F_PAD "class " F_GLOBAL16 ", " F_CONST16)
DEE_ASM_OPCODE(0xf0, 0x68, 8, 16_CLASS_EC, SP_SUB0, SP_ADD1, "push" F_PAD "class " F_EXTERN16 ", " F_CONST16)
DEE_ASM_OPCODE(0xf0, 0x69, 4, 16_DEFCMEMBER, SP_SUB2, SP_ADD1, "defcmember" F_PAD "top, $" F_IMM16 ", pop")
DEE_ASM_OPCODE(0xf0, 0x6a, 6, 16_GETCMEMBER_R, SP_SUB0, SP_ADD1, "push" F_PAD "getcmember " F_REF16 ", $" F_IMM16)
DEE_ASM_OPCODE(0xf0, 0x6b, 7, 16_CALLCMEMBER_THIS_R, SP_SUBIMM6N8, SP_ADD1, "push" F_PAD "callcmember this, " F_REF16 ", $" F_IMM16 ", #" F_IMM8)
DEE_ASM_UNDEFINED(0xf0, 0x6c, 2)
DEE_ASM_UNDEFINED(0xf0, 0x6d, 2)
DEE_ASM_OPCODE_P(0xf0, 0x6e, 5, 16_FUNCTION_C, SP_SUBIMM4N8, SP_ADD1, "push" F_PAD "function " F_CONST16 ", #" F_IMM8, SP_SUBIMM4N8, SP_ADD0, F_PREFIX ": function " F_CONST16 ", #" F_IMM8)
DEE_ASM_OPCODE_P(0xf0, 0x6f, 6, 16_FUNCTION_C_16, SP_SUBIMM4N16, SP_ADD1, "push" F_PAD "function " F_CONST16 ", #" F_IMM16, SP_SUBIMM4N16, SP_ADD0, F_PREFIX ": function " F_CONST16 ", #" F_IMM16)
DEE_ASM_OPCODE(0xf0, 0x70, 4, _SUPERGETATTR_THIS_RC, SP_SUB0, SP_ADD1, "push" F_PAD "getattr this, " F_REF8 ", " F_CONST8)
DEE_ASM_OPCODE(0xf0, 0x71, 6, 16_SUPERGETATTR_THIS_RC, SP_SUB0, SP_ADD1, "push" F_PAD "getattr this, " F_REF16 ", " F_CONST16)
DEE_ASM_OPCODE(0xf0, 0x72, 5, _SUPERCALLATTR_THIS_RC, SP_SUBIMM4N8, SP_ADD1, "push" F_PAD "callattr this, " F_REF8 ", " F_CONST8 ", #" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0x73, 7, 16_SUPERCALLATTR_THIS_RC, SP_SUBIMM6N8, SP_ADD1, "push" F_PAD "callattr this, " F_REF16 ", " F_CONST16 ", #" F_IMM8)
DEE_ASM_UNDEFINED(0xf0, 0x74, 2)
DEE_ASM_UNDEFINED(0xf0, 0x75, 2)
DEE_ASM_UNDEFINED(0xf0, 0x76, 2)
DEE_ASM_UNDEFINED(0xf0, 0x77, 2)
DEE_ASM_UNDEFINED(0xf0, 0x78, 2)
DEE_ASM_UNDEFINED(0xf0, 0x79, 2)
DEE_ASM_UNDEFINED(0xf0, 0x7a, 2)
DEE_ASM_UNDEFINED(0xf0, 0x7b, 2)
DEE_ASM_UNDEFINED(0xf0, 0x7c, 2)
DEE_ASM_UNDEFINED(0xf0, 0x7d, 2)
DEE_ASM_UNDEFINED(0xf0, 0x7e, 2)
DEE_ASM_OPCODE_P(0xf0, 0x7f, 2, _INCPOST, SP_SUB0, SP_ADD1, "push" F_PAD "inc", SP_SUB0, SP_ADD1, "push" F_PAD "inc " F_PREFIX)
DEE_ASM_OPCODE_P(0xf0, 0x80, 2, _DECPOST, SP_SUB0, SP_ADD1, "push" F_PAD "dec", SP_SUB0, SP_ADD1, "push" F_PAD "dec " F_PREFIX)
DEE_ASM_UNDEFINED(0xf0, 0x81, 2)
DEE_ASM_UNDEFINED(0xf0, 0x82, 2)
DEE_ASM_UNDEFINED(0xf0, 0x83, 2)
DEE_ASM_UNDEFINED(0xf0, 0x84, 2)
DEE_ASM_UNDEFINED(0xf0, 0x85, 2)
DEE_ASM_UNDEFINED(0xf0, 0x86, 2)
DEE_ASM_UNDEFINED(0xf0, 0x87, 2)
DEE_ASM_UNDEFINED(0xf0, 0x88, 2)
DEE_ASM_UNDEFINED(0xf0, 0x89, 2)
DEE_ASM_UNDEFINED(0xf0, 0x8a, 2)
DEE_ASM_UNDEFINED(0xf0, 0x8b, 2)
DEE_ASM_UNDEFINED(0xf0, 0x8c, 2)
DEE_ASM_UNDEFINED(0xf0, 0x8d, 2)
DEE_ASM_UNDEFINED(0xf0, 0x8e, 2)
DEE_ASM_OPCODE(0xf0, 0x8f, 2, 16_DELOP, SP_SUB0, SP_ADD0, "")
DEE_ASM_OPCODE_P(0xf0, 0x90, 2, 16_NOP, SP_SUB0, SP_ADD0, "nop16", SP_SUB0, SP_ADD0, "nop16" F_PAD F_PREFIX)
DEE_ASM_UNDEFINED(0xf0, 0x91, 2)
DEE_ASM_UNDEFINED(0xf0, 0x92, 2)
DEE_ASM_UNDEFINED(0xf0, 0x93, 2)
DEE_ASM_OPCODE(0xf0, 0x94, 2, _REDUCE_MIN, SP_SUB1, SP_ADD1, "reduce" F_PAD "top, min")
DEE_ASM_OPCODE(0xf0, 0x95, 2, _REDUCE_MAX, SP_SUB1, SP_ADD1, "reduce" F_PAD "top, max")
DEE_ASM_OPCODE(0xf0, 0x96, 2, _REDUCE_SUM, SP_SUB1, SP_ADD1, "reduce" F_PAD "top, sum")
DEE_ASM_OPCODE(0xf0, 0x97, 2, _REDUCE_ANY, SP_SUB1, SP_ADD1, "reduce" F_PAD "top, any")
DEE_ASM_OPCODE(0xf0, 0x98, 2, _REDUCE_ALL, SP_SUB1, SP_ADD1, "reduce" F_PAD "top, all")
DEE_ASM_UNDEFINED(0xf0, 0x99, 2)
DEE_ASM_OPCODE_P(0xf0, 0x9a, 3, _CMPXCH_UB_C, SP_SUB1, SP_ADD1, "cmpxch" F_PAD "top, none, " F_CONST8, SP_SUB0, SP_ADD1, "push" F_PAD "cmpxch " F_PREFIX ", unbound, " F_CONST8)
DEE_ASM_OPCODE_P(0xf0, 0x9b, 4, 16_CMPXCH_UB_C, SP_SUB1, SP_ADD1, "cmpxch" F_PAD "top, none, " F_CONST16, SP_SUB0, SP_ADD1, "push" F_PAD "cmpxch " F_PREFIX ", unbound, " F_CONST16)
DEE_ASM_OPCODE_P(0xf0, 0x9c, 2, _CMPXCH_UB_LOCK, SP_SUB1, SP_ADD1, "cmpxch" F_PAD "top, none, none", SP_SUB0, SP_ADD1, "push" F_PAD "cmpxch " F_PREFIX ", unbound, lock")
DEE_ASM_OPCODE_P(0xf0, 0x9d, 2, _CMPXCH_UB_POP, SP_SUB2, SP_ADD1, "cmpxch" F_PAD "top, none, pop", SP_SUB1, SP_ADD1, "push" F_PAD "cmpxch " F_PREFIX ", unbound, pop")
DEE_ASM_OPCODE_P(0xf0, 0x9e, 2, _CMPXCH_POP_UB, SP_SUB2, SP_ADD1, "cmpxch" F_PAD "top, pop, none", SP_SUB1, SP_ADD1, "push" F_PAD "cmpxch " F_PREFIX ", pop, unbound")
DEE_ASM_OPCODE_P(0xf0, 0x9f, 2, _CMPXCH_POP_POP, SP_SUB3, SP_ADD1, "cmpxch" F_PAD "top, pop, pop", SP_SUB2, SP_ADD1, "push" F_PAD "cmpxch " F_PREFIX ", pop, pop")
DEE_ASM_UNDEFINED(0xf0, 0xa0, 2)
DEE_ASM_OPCODE(0xf0, 0xa1, 4, 16_PRINT_C, SP_SUB0, SP_ADD0, "print" F_PAD F_CONST16)
DEE_ASM_OPCODE(0xf0, 0xa2, 4, 16_PRINT_C_SP, SP_SUB0, SP_ADD0, "print" F_PAD F_CONST16 ", sp")
DEE_ASM_OPCODE(0xf0, 0xa3, 4, 16_PRINT_C_NL, SP_SUB0, SP_ADD0, "print" F_PAD F_CONST16 ", nl")
DEE_ASM_OPCODE(0xf0, 0xa4, 6, _RANGE_0_I32, SP_SUB0, SP_ADD1, "push" F_PAD "range $0, $" F_IMM32)
DEE_ASM_UNDEFINED(0xf0, 0xa5, 2)
DEE_ASM_OPCODE(0xf0, 0xa6, 3, _VARARGS_UNPACK, SP_SUB0, SP_ADDIMM2N8, "unpack" F_PAD "varargs, #" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0xa7, 2, _PUSH_VARKWDS_NE, SP_SUB0, SP_ADD1, "push" F_PAD "bool varkwds")
DEE_ASM_UNDEFINED(0xf0, 0xa8, 2)
DEE_ASM_OPCODE(0xf0, 0xa9, 4, 16_FPRINT_C, SP_SUB1, SP_ADD1, "print" F_PAD "top, " F_CONST16)
DEE_ASM_OPCODE(0xf0, 0xaa, 4, 16_FPRINT_C_SP, SP_SUB1, SP_ADD1, "print" F_PAD "top, " F_CONST16 ", sp")
DEE_ASM_OPCODE(0xf0, 0xab, 4, 16_FPRINT_C_NL, SP_SUB1, SP_ADD1, "print" F_PAD "top, " F_CONST16 ", nl")
DEE_ASM_OPCODE(0xf0, 0xac, 3, _VARARGS_CMP_EQ_SZ, SP_SUB0, SP_ADD1, "push" F_PAD "cmp eq, #varargs, $" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0xad, 3, _VARARGS_CMP_GR_SZ, SP_SUB0, SP_ADD1, "push" F_PAD "cmp gr, #varargs, $" F_IMM8)
DEE_ASM_UNDEFINED(0xf0, 0xae, 2)
DEE_ASM_UNDEFINED(0xf0, 0xaf, 2)
DEE_ASM_UNDEFINED(0xf0, 0xb0, 2)
DEE_ASM_OPCODE(0xf0, 0xb1, 3, 16_CONTAINS_C, SP_SUB1, SP_ADD1, "push" F_PAD "contains " F_CONST16 ", pop")
DEE_ASM_OPCODE(0xf0, 0xb2, 2, _VARARGS_GETITEM, SP_SUB1, SP_ADD1, "getitem" F_PAD "varargs, top")
DEE_ASM_OPCODE(0xf0, 0xb3, 3, _VARARGS_GETITEM_I, SP_SUB0, SP_ADD1, "push" F_PAD "getitem varargs, $" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0xb4, 4, 16_GETITEM_C, SP_SUB1, SP_ADD1, "getitem" F_PAD "top, " F_CONST16)
DEE_ASM_OPCODE(0xf0, 0xb5, 2, _VARARGS_GETSIZE, SP_SUB0, SP_ADD1, "push" F_PAD "getsize varargs")
DEE_ASM_UNDEFINED(0xf0, 0xb6, 2)
DEE_ASM_UNDEFINED(0xf0, 0xb7, 2)
DEE_ASM_OPCODE(0xf0, 0xb8, 4, 16_SETITEM_C, SP_SUB2, SP_ADD0, "setitem" F_PAD "pop, " F_CONST16 ", pop")
DEE_ASM_OPCODE(0xf0, 0xb9, 2, _ITERNEXT, SP_SUB1, SP_ADD1, "iternext" F_PAD "top")
DEE_ASM_OPCODE(0xf0, 0xba, 2, _ENDFINALLY_EXCEPT, SP_SUB0, SP_ADD0, "end" F_PAD "finally, except")
DEE_ASM_UNDEFINED(0xf0, 0xbb, 2)
DEE_ASM_UNDEFINED(0xf0, 0xbc, 2)
DEE_ASM_UNDEFINED(0xf0, 0xbd, 2)
DEE_ASM_OPCODE(0xf0, 0xbe, 3, _GETMEMBER, SP_SUB2, SP_ADD1, "getmember" F_PAD "top, pop, $" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0xbf, 4, 16_GETMEMBER, SP_SUB2, SP_ADD1, "getmember" F_PAD "top, pop, $" F_IMM16)
DEE_ASM_OPCODE(0xf0, 0xc0, 3, _DELMEMBER, SP_SUB2, SP_ADD0, "delmember" F_PAD "pop, pop, $" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0xc1, 4, 16_DELMEMBER, SP_SUB2, SP_ADD0, "delmember" F_PAD "pop, pop, $" F_IMM16)
DEE_ASM_OPCODE(0xf0, 0xc2, 3, _SETMEMBER, SP_SUB3, SP_ADD0, "setmember" F_PAD "pop, pop, $" F_IMM8 ", pop")
DEE_ASM_OPCODE(0xf0, 0xc3, 4, 16_SETMEMBER, SP_SUB3, SP_ADD0, "setmember" F_PAD "pop, pop, $" F_IMM16 ", pop")
DEE_ASM_OPCODE(0xf0, 0xc4, 3, _BOUNDMEMBER, SP_SUB2, SP_ADD1, "boundmember" F_PAD "top, pop, $" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0xc5, 4, 16_BOUNDMEMBER, SP_SUB2, SP_ADD1, "boundmember" F_PAD "top, pop, $" F_IMM16)
DEE_ASM_OPCODE(0xf0, 0xc6, 3, _GETMEMBER_THIS, SP_SUB1, SP_ADD1, "push" F_PAD "getmember this, pop, $" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0xc7, 4, 16_GETMEMBER_THIS, SP_SUB1, SP_ADD1, "push" F_PAD "getmember this, pop, $" F_IMM16)
DEE_ASM_OPCODE(0xf0, 0xc8, 3, _DELMEMBER_THIS, SP_SUB1, SP_ADD0, "delmember" F_PAD "this, pop, $" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0xc9, 4, 16_DELMEMBER_THIS, SP_SUB1, SP_ADD0, "delmember" F_PAD "this, pop, $" F_IMM16)
DEE_ASM_OPCODE(0xf0, 0xca, 3, _SETMEMBER_THIS, SP_SUB2, SP_ADD0, "setmember" F_PAD "this, pop, $" F_IMM8 ", pop")
DEE_ASM_OPCODE(0xf0, 0xcb, 4, 16_SETMEMBER_THIS, SP_SUB2, SP_ADD0, "setmember" F_PAD "this, pop, $" F_IMM16 ", pop")
DEE_ASM_OPCODE(0xf0, 0xcc, 3, _BOUNDMEMBER_THIS, SP_SUB1, SP_ADD1, "push" F_PAD "boundmember this, pop, $" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0xcd, 4, 16_BOUNDMEMBER_THIS, SP_SUB1, SP_ADD1, "push" F_PAD "boundmember this, pop, $" F_IMM16)
DEE_ASM_OPCODE(0xf0, 0xce, 7, 16_CALLATTR_C_KW, SP_SUBIMM4N8_MINUS1, SP_ADD1, "callattr" F_PAD "top, " F_CONST16 ", #" F_IMM8 ", " F_CONST16)
DEE_ASM_OPCODE(0xf0, 0xcf, 6, 16_CALLATTR_C_TUPLE_KW, SP_SUB2, SP_ADD1, "callattr" F_PAD "top, " F_CONST16 ", pop..., " F_CONST16)
DEE_ASM_OPCODE(0xf0, 0xd0, 2, _CALLATTR_KWDS, SP_SUBIMM2N8_MINUS3, SP_ADD1, "callattr" F_PAD "top, pop, #" F_IMM8 ", pop")
DEE_ASM_OPCODE(0xf0, 0xd1, 2, _CALLATTR_TUPLE_KWDS, SP_SUB4, SP_ADD1, "callattr" F_PAD "top, pop, pop..., pop")
DEE_ASM_OPCODE(0xf0, 0xd2, 5, 16_CALLATTR_C, SP_SUBIMM4N8_MINUS1, SP_ADD1, "callattr" F_PAD "top, " F_CONST16 ", #" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0xd3, 4, 16_CALLATTR_C_TUPLE, SP_SUB2, SP_ADD1, "callattr" F_PAD "top, " F_CONST16 ", pop")
DEE_ASM_OPCODE(0xf0, 0xd4, 5, 16_CALLATTR_THIS_C, SP_SUBIMM4N8, SP_ADD1, "callattr" F_PAD "this, " F_CONST16 ", #" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0xd5, 4, 16_CALLATTR_THIS_C_TUPLE, SP_SUB1, SP_ADD1, "callattr" F_PAD "this, " F_CONST16 ", pop")
DEE_ASM_OPCODE(0xf0, 0xd6, 5, 16_CALLATTR_C_SEQ, SP_SUBIMM4N8_MINUS1, SP_ADD1, "callattr" F_PAD "top, " F_CONST16 ", [#" F_IMM8 "]")
DEE_ASM_OPCODE(0xf0, 0xd7, 5, 16_CALLATTR_C_MAP, SP_SUBIMM4N8X2_MINUS1, SP_ADD1, "callattr" F_PAD "top, " F_CONST16 ", {#" F_IMM8_X2 "}")
DEE_ASM_OPCODE(0xf0, 0xd8, 6, 16_GETMEMBER_THIS_R, SP_SUB0, SP_ADD1, "push" F_PAD "getmember this, " F_REF16 ", $" F_IMM16)
DEE_ASM_OPCODE(0xf0, 0xd9, 6, 16_DELMEMBER_THIS_R, SP_SUB0, SP_ADD0, "delmember" F_PAD "this, " F_REF16 ", $" F_IMM16)
DEE_ASM_OPCODE(0xf0, 0xda, 6, 16_SETMEMBER_THIS_R, SP_SUB1, SP_ADD0, "setmember" F_PAD "this, " F_REF16 ", $" F_IMM16 ", pop")
DEE_ASM_OPCODE(0xf0, 0xdb, 6, 16_SETMEMBERI_THIS_R, SP_SUB1, SP_ADD0, "setmemberi" F_PAD "this, " F_REF16 ", $" F_IMM16 ", pop")
DEE_ASM_OPCODE(0xf0, 0xdc, 6, 16_BOUNDMEMBER_THIS_R, SP_SUB0, SP_ADD1, "push" F_PAD "boundmember this, " F_REF16 ", $" F_IMM16)
DEE_ASM_OPCODE(0xf0, 0xdd, 7, 16_CALL_EXTERN, SP_SUBIMM6N8, SP_ADD1, "push" F_PAD "call " F_EXTERN16 ", #" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0xde, 5, 16_CALL_GLOBAL, SP_SUBIMM4N8, SP_ADD1, "push" F_PAD "call " F_GLOBAL16 ", #" F_IMM8)
DEE_ASM_OPCODE(0xf0, 0xdf, 5, 16_CALL_LOCAL, SP_SUBIMM4N8, SP_ADD1, "push" F_PAD "call " F_LOCAL16 ", #" F_IMM8)
DEE_ASM_UNDEFINED(0xf0, 0xe0, 2)
DEE_ASM_UNDEFINED(0xf0, 0xe1, 2)
DEE_ASM_UNDEFINED(0xf0, 0xe2, 2)
DEE_ASM_UNDEFINED(0xf0, 0xe3, 2)
DEE_ASM_UNDEFINED(0xf0, 0xe4, 2)
DEE_ASM_UNDEFINED(0xf0, 0xe5, 2)
DEE_ASM_UNDEFINED(0xf0, 0xe6, 2)
DEE_ASM_UNDEFINED(0xf0, 0xe7, 2)
DEE_ASM_UNDEFINED(0xf0, 0xe8, 2)
DEE_ASM_UNDEFINED(0xf0, 0xe9, 2)
DEE_ASM_UNDEFINED(0xf0, 0xea, 2)
DEE_ASM_UNDEFINED(0xf0, 0xeb, 2)
DEE_ASM_UNDEFINED(0xf0, 0xec, 2)
DEE_ASM_UNDEFINED(0xf0, 0xed, 2)
DEE_ASM_UNDEFINED(0xf0, 0xee, 2)
DEE_ASM_UNDEFINED(0xf0, 0xef, 2)
DEE_ASM_UNDEFINED(0xf0, 0xf0, 2)
DEE_ASM_UNDEFINED(0xf0, 0xf1, 2)
DEE_ASM_UNDEFINED(0xf0, 0xf2, 2)
DEE_ASM_UNDEFINED(0xf0, 0xf3, 2)
DEE_ASM_UNDEFINED(0xf0, 0xf4, 2)
DEE_ASM_UNDEFINED(0xf0, 0xf5, 2)
DEE_ASM_UNDEFINED(0xf0, 0xf6, 2)
DEE_ASM_UNDEFINED(0xf0, 0xf7, 2)
DEE_ASM_UNDEFINED_PREFIX(0xf0, 0xf8, 2)
DEE_ASM_UNDEFINED_PREFIX(0xf0, 0xf9, 2)
DEE_ASM_PREFIX(0xf0, 0xfa, 4, 16_STACK)
DEE_ASM_UNDEFINED_PREFIX(0xf0, 0xfb, 2)
DEE_ASM_PREFIX(0xf0, 0xfc, 6, 16_EXTERN)
DEE_ASM_PREFIX(0xf0, 0xfd, 4, 16_STATIC)
DEE_ASM_PREFIX(0xf0, 0xfe, 4, 16_GLOBAL)
DEE_ASM_PREFIX(0xf0, 0xff, 4, 16_LOCAL)
DEE_ASM_END(0xf0)
#endif /* DEE_ASM_WANT_TABLE(0xf0) */
/*[[[end]]]*/

#undef DEE_SP_SUB
#undef DEE_SP_ADD
#undef DEE_ASM_OPCODE_P
#undef DEE_ASM_OPCODE
#undef DEE_ASM_PREFIX
#undef DEE_ASM_EXTENDED
#undef DEE_ASM_UNDEFINED_PREFIX
#undef DEE_ASM_UNDEFINED
#undef DEE_ASM_END
#undef DEE_ASM_BEGIN
#undef DEE_ASM_WANT_TABLE
