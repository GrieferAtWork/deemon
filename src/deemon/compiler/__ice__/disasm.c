/* Copyright (c) 2018 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_COMPILER_DISASM_C
#define GUARD_DEEMON_COMPILER_DISASM_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/format.h>
#include <deemon/string.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/code.h>
#include <deemon/module.h>
#include <deemon/int.h>
#include <deemon/tuple.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/assembler.h>
#include <deemon/class.h>
#include <hybrid/minmax.h>

#include <string.h>

DECL_BEGIN

/* TODO: This function should eventually be moved into a DEX. */

#define UNKNOWN_MNEMONIC    "?"
#define PREFIX_ADDRESS      "" /* "+" */
#define PREFIX_ADDRESS_SPC  "" /* " " */
#define PREFIX_STACKEFFECT  "#" /* Prefix for operands that affect the stack-effect */
#define PREFIX_INTEGERAL    "$" /* Prefix for operands that are integral immediate values. */
#define PREFIX_VARNAME      "@" /* Prefix for variable names. */
#define PREFIX_CONSTANT     "@" /* Prefix for constant expressions. */
#define COMMENT(s)          "/* " s " */" /* Wrap `s' within an assembly comment. */
#define MNEMONIC_MINWIDTH   6

/*[[[deemon
#include <file>
#include <util>
#include <fs>
local codes = list([none] * 256);
local codes_f0 = list([none] * 256);
fs::chdir(fs::path::head(__FILE__));
local longest_name = 0;
local longest_name_f0 = 0;

for (local l: file.open("../../../include/deemon/asm.h")) {
    local name,code,misc;
    try name,code,none,misc = l.scanf(" # define ASM%[^ ] 0x%[0-9a-fA-F] /" "* [ %[^ \\]] ] %[^]")...;
    catch (e...) {
        try name,code,misc = l.scanf(" # define ASM%[^ ] 0x%[0-9a-fA-F] /" "* %[^]")...;
        catch (...) continue;
    }
    local desc;
    try none,desc = misc.scanf(" [ %[^\\]] ] ` %[^\'] '")...;
    catch (...) {
        try desc = misc.scanf(" ` %[^\'] '")...;
        catch (...) continue;
    }
    code = (int)("0x"+code);
    desc = desc.strip();
    local short_desc = desc;
    if ("<" in short_desc) {
        short_desc = desc.partition("<")[0];
    }
    short_desc = short_desc.strip();
    if (short_desc.endswith("+/-")) short_desc = short_desc[:#short_desc-3].rstrip();
    if (desc != short_desc) short_desc = short_desc+" ";
    if (" " in short_desc) {
        local a,none,b = short_desc.partition(" ")...;
        short_desc = a.strip().ljust(MNEMONIC_MINWIDTH)+" "+b.strip();
    }
    if (short_desc.endswith(" const"))
        short_desc = short_desc[:#short_desc-#"const"];
    if (short_desc.endswith("$ ") ||
        short_desc.endswith("# "))
        short_desc = short_desc[:#short_desc-1];
    if (short_desc.endswith(",") ||
        short_desc.endswith("ref") ||
        short_desc.endswith("arg") ||
        short_desc.endswith("const") ||
        short_desc.endswith("static") ||
        short_desc.endswith("extern") ||
        short_desc.endswith("global") ||
        short_desc.endswith("module") ||
        short_desc.endswith("local") ||
        short_desc.endswith("#SP") ||
        short_desc.endswith("#SP +") ||
        short_desc.endswith("#SP -"))
        short_desc = short_desc+" ";
    local name_length = #short_desc;
    if (short_desc.endswith("#")) {
        name_length = (name_length-1)+#PREFIX_STACKEFFECT;
        short_desc = (repr short_desc[:#short_desc-1])+" PREFIX_STACKEFFECT";
    } else if (short_desc.endswith("$")) {
        name_length = (name_length-1)+#PREFIX_INTEGERAL;
        short_desc = (repr short_desc[:#short_desc-1])+" PREFIX_INTEGERAL";
    } else {
        short_desc = repr short_desc;
    }
    local data = pack(name,short_desc);
    if (code < 256) {
        if (codes[code] is none) {
            codes[code] = data;
            if (longest_name < name_length)
                longest_name = name_length;
        }
    } else {
        if (code >= 0xf000 && code <= 0xf0ff) {
            codes_f0[code - 0xf000] = data;
            if (longest_name_f0 < name_length)
                longest_name_f0 = name_length;
        }
    }
}
print "PRIVATE char const mnemonic_names[256]["+(longest_name+1)+"] = {";
for (local id,data: util.enumerate(codes)) {
     if (data is none) {
        print ("    /" "* 0x%.2I8x *" "/" % id)+" UNKNOWN_MNEMONIC, /" "* --- *" "/";
     } else {
        local name,desc = data...;
        print ("    /" "* 0x%.2I8x *" "/" % id)+" "+desc+", /" "* `ASM"+name+"' *" "/";
     }
}
print "};";
print "PRIVATE char const mnemonic_names_f0[256]["+(longest_name_f0+1)+"] = {";
for (local id,data: util.enumerate(codes_f0)) {
     if (data is none) {
        print ("    /" "* 0xf0%.2I8x *" "/" % id)+" UNKNOWN_MNEMONIC, /" "* --- *" "/";
     } else {
        local name,desc = data...;
        print ("    /" "* 0xf0%.2I8x *" "/" % id)+" "+desc+", /" "* `ASM"+name+"' *" "/";
     }
}
print "};";
]]]*/
PRIVATE char const mnemonic_names[256][31] = {
    /* 0x00 */ "ret", /* `ASM_RET_NONE' */
    /* 0x01 */ "ret    pop", /* `ASM_RET' */
    /* 0x02 */ "yield  foreach, pop", /* `ASM_YIELDALL' */
    /* 0x03 */ "throw  pop", /* `ASM_THROW' */
    /* 0x04 */ "throw  except", /* `ASM_RETHROW' */
    /* 0x05 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x06 */ "end    catch", /* `ASM_ENDCATCH' */
    /* 0x07 */ "end    finally", /* `ASM_ENDFINALLY' */
    /* 0x08 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x09 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x0a */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x0b */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x0c */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x0d */ "push   bound extern ", /* `ASM_PUSH_BND_EXTERN' */
    /* 0x0e */ "push   bound global ", /* `ASM_PUSH_BND_GLOBAL' */
    /* 0x0f */ "push   bound local ", /* `ASM_PUSH_BND_LOCAL' */
    /* 0x10 */ "jf     pop, ", /* `ASM_JF' */
    /* 0x11 */ "jf     pop, ", /* `ASM_JF16' */
    /* 0x12 */ "jt     pop, ", /* `ASM_JT' */
    /* 0x13 */ "jt     pop, ", /* `ASM_JT16' */
    /* 0x14 */ "jmp    ", /* `ASM_JMP' */
    /* 0x15 */ "jmp    ", /* `ASM_JMP16' */
    /* 0x16 */ "foreach top, ", /* `ASM_FOREACH' */
    /* 0x17 */ "foreach top, ", /* `ASM_FOREACH16' */
    /* 0x18 */ "jmp    pop", /* `ASM_JMP_POP' */
    /* 0x19 */ "call   top, " PREFIX_STACKEFFECT, /* `ASM_CALL' */
    /* 0x1a */ "call   top, pop", /* `ASM_CALL_TUPLE' */
    /* 0x1b */ "op     top, " PREFIX_INTEGERAL, /* `ASM_OPERATOR' */
    /* 0x1c */ "op     top, " PREFIX_INTEGERAL, /* `ASM_OPERATOR_TUPLE' */
    /* 0x1d */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x1e */ "del    global ", /* `ASM_DEL_GLOBAL' */
    /* 0x1f */ "del    local ", /* `ASM_DEL_LOCAL' */
    /* 0x20 */ "swap", /* `ASM_SWAP' */
    /* 0x21 */ "lrot   " PREFIX_STACKEFFECT, /* `ASM_LROT' */
    /* 0x22 */ "rrot   " PREFIX_STACKEFFECT, /* `ASM_RROT' */
    /* 0x23 */ "dup", /* `ASM_DUP' */
    /* 0x24 */ "dup    #SP - ", /* `ASM_DUP_N' */
    /* 0x25 */ "pop", /* `ASM_POP' */
    /* 0x26 */ "pop    #SP - ", /* `ASM_POP_N' */
    /* 0x27 */ "adjstack #SP ", /* `ASM_ADJSTACK' */
    /* 0x28 */ "super  top, pop", /* `ASM_SUPER' */
    /* 0x29 */ "push   super ref ", /* `ASM_SUPER_THIS_R' */
    /* 0x2a */ "push   super global ", /* `ASM_SUPER_THIS_G' */
    /* 0x2b */ "push   super extern ", /* `ASM_SUPER_THIS_E' */
    /* 0x2c */ "pop    static ", /* `ASM_POP_STATIC' */
    /* 0x2d */ "pop    extern ", /* `ASM_POP_EXTERN' */
    /* 0x2e */ "pop    global ", /* `ASM_POP_GLOBAL' */
    /* 0x2f */ "pop    local ", /* `ASM_POP_LOCAL' */
    /* 0x30 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x31 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x32 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x33 */ "push   none", /* `ASM_PUSH_NONE' */
    /* 0x34 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x35 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x36 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x37 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x38 */ "push   module ", /* `ASM_PUSH_MODULE' */
    /* 0x39 */ "push   ref ", /* `ASM_PUSH_REF' */
    /* 0x3a */ "push   arg ", /* `ASM_PUSH_ARG' */
    /* 0x3b */ "push   ", /* `ASM_PUSH_CONST' */
    /* 0x3c */ "push   static ", /* `ASM_PUSH_STATIC' */
    /* 0x3d */ "push   extern ", /* `ASM_PUSH_EXTERN' */
    /* 0x3e */ "push   global ", /* `ASM_PUSH_GLOBAL' */
    /* 0x3f */ "push   local ", /* `ASM_PUSH_LOCAL' */
    /* 0x40 */ "cast   top, tuple", /* `ASM_CAST_TUPLE' */
    /* 0x41 */ "cast   top, list", /* `ASM_CAST_LIST' */
    /* 0x42 */ "push   pack tuple, " PREFIX_STACKEFFECT, /* `ASM_PACK_TUPLE' */
    /* 0x43 */ "push   pack list, " PREFIX_STACKEFFECT, /* `ASM_PACK_LIST' */
    /* 0x44 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x45 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x46 */ "unpack pop, " PREFIX_STACKEFFECT, /* `ASM_UNPACK' */
    /* 0x47 */ "concat top, pop", /* `ASM_CONCAT' */
    /* 0x48 */ "extend top, " PREFIX_STACKEFFECT, /* `ASM_EXTEND' */
    /* 0x49 */ "typeof top", /* `ASM_TYPEOF' */
    /* 0x4a */ "classof top", /* `ASM_CLASSOF' */
    /* 0x4b */ "superof top", /* `ASM_SUPEROF' */
    /* 0x4c */ "instanceof top, pop", /* `ASM_INSTANCEOF' */
    /* 0x4d */ "str    top", /* `ASM_STR' */
    /* 0x4e */ "repr   top", /* `ASM_REPR' */
    /* 0x4f */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x50 */ "bool   top", /* `ASM_BOOL' */
    /* 0x51 */ "not    top", /* `ASM_NOT' */
    /* 0x52 */ "assign pop, pop", /* `ASM_ASSIGN' */
    /* 0x53 */ "assign move, pop, pop", /* `ASM_MOVE_ASSIGN' */
    /* 0x54 */ "copy   top", /* `ASM_COPY' */
    /* 0x55 */ "deepcopy top", /* `ASM_DEEPCOPY' */
    /* 0x56 */ "getattr top, pop", /* `ASM_GETATTR' */
    /* 0x57 */ "delattr pop, pop", /* `ASM_DELATTR' */
    /* 0x58 */ "setattr pop, pop, pop", /* `ASM_SETATTR' */
    /* 0x59 */ "hasattr top, pop", /* `ASM_HASATTR' */
    /* 0x5a */ "getattr top, ", /* `ASM_GETATTR_C' */
    /* 0x5b */ "delattr pop, ", /* `ASM_DELATTR_C' */
    /* 0x5c */ "setattr pop, ", /* `ASM_SETATTR_C' */
    /* 0x5d */ "push   getattr this, ", /* `ASM_GETATTR_THIS_C' */
    /* 0x5e */ "delattr this, ", /* `ASM_DELATTR_THIS_C' */
    /* 0x5f */ "setattr this, ", /* `ASM_SETATTR_THIS_C' */
    /* 0x60 */ "cmp    eq, top, pop", /* `ASM_CMP_EQ' */
    /* 0x61 */ "cmp    ne, top, pop", /* `ASM_CMP_NE' */
    /* 0x62 */ "cmp    lo, top, pop", /* `ASM_CMP_LO' */
    /* 0x63 */ "cmp    le, top, pop", /* `ASM_CMP_LE' */
    /* 0x64 */ "cmp    gr, top, pop", /* `ASM_CMP_GR' */
    /* 0x65 */ "cmp    ge, top, pop", /* `ASM_CMP_GE' */
    /* 0x66 */ "push   class " PREFIX_INTEGERAL, /* `ASM_CLASS' */
    /* 0x67 */ "push   class " PREFIX_INTEGERAL, /* `ASM_CLASS_C' */
    /* 0x68 */ "push   class " PREFIX_INTEGERAL, /* `ASM_CLASS_CBL' */
    /* 0x69 */ "push   class " PREFIX_INTEGERAL, /* `ASM_CLASS_CBG' */
    /* 0x6a */ "defmember top, " PREFIX_INTEGERAL, /* `ASM_DEFMEMBER' */
    /* 0x6b */ "defop  top, " PREFIX_INTEGERAL, /* `ASM_DEFOP' */
    /* 0x6c */ "function top, pop", /* `ASM_FUNCTION' */
    /* 0x6d */ "push   function ", /* `ASM_FUNCTION_C_0' */
    /* 0x6e */ "push   function ", /* `ASM_FUNCTION_C' */
    /* 0x6f */ "push   function ", /* `ASM_FUNCTION_C_16' */
    /* 0x70 */ "cast   top, int", /* `ASM_CAST_INT' */
    /* 0x71 */ "inv    top", /* `ASM_INV' */
    /* 0x72 */ "pos    top", /* `ASM_POS' */
    /* 0x73 */ "neg    top", /* `ASM_NEG' */
    /* 0x74 */ "add    top, pop", /* `ASM_ADD' */
    /* 0x75 */ "sub    top, pop", /* `ASM_SUB' */
    /* 0x76 */ "mul    top, pop", /* `ASM_MUL' */
    /* 0x77 */ "div    top, pop", /* `ASM_DIV' */
    /* 0x78 */ "mod    top, pop", /* `ASM_MOD' */
    /* 0x79 */ "shl    top, pop", /* `ASM_SHL' */
    /* 0x7a */ "shr    top, pop", /* `ASM_SHR' */
    /* 0x7b */ "and    top, pop", /* `ASM_AND' */
    /* 0x7c */ "or     top, pop", /* `ASM_OR' */
    /* 0x7d */ "xor    top, pop", /* `ASM_XOR' */
    /* 0x7e */ "pow    top, pop", /* `ASM_POW' */
    /* 0x7f */ "inc", /* `ASM_INC' */
    /* 0x80 */ "dec", /* `ASM_DEC' */
    /* 0x81 */ "add    top, " PREFIX_INTEGERAL, /* `ASM_ADD_SIMM8' */
    /* 0x82 */ "add    top, " PREFIX_INTEGERAL, /* `ASM_ADD_IMM32' */
    /* 0x83 */ "sub    top, " PREFIX_INTEGERAL, /* `ASM_SUB_SIMM8' */
    /* 0x84 */ "sub    top, " PREFIX_INTEGERAL, /* `ASM_SUB_IMM32' */
    /* 0x85 */ "mul    top, " PREFIX_INTEGERAL, /* `ASM_MUL_SIMM8' */
    /* 0x86 */ "div    top, " PREFIX_INTEGERAL, /* `ASM_DIV_SIMM8' */
    /* 0x87 */ "mod    top, " PREFIX_INTEGERAL, /* `ASM_MOD_SIMM8' */
    /* 0x88 */ "shl    top, " PREFIX_INTEGERAL, /* `ASM_SHL_IMM8' */
    /* 0x89 */ "shr    top, " PREFIX_INTEGERAL, /* `ASM_SHR_IMM8' */
    /* 0x8a */ "and    top, " PREFIX_INTEGERAL, /* `ASM_AND_IMM32' */
    /* 0x8b */ "or     top, " PREFIX_INTEGERAL, /* `ASM_OR_IMM32' */
    /* 0x8c */ "xor    top, " PREFIX_INTEGERAL, /* `ASM_XOR_IMM32' */
    /* 0x8d */ "instanceof top, none", /* `ASM_ISNONE' */
    /* 0x8e */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x8f */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x90 */ "nop", /* `ASM_NOP' */
    /* 0x91 */ "print  pop", /* `ASM_PRINT' */
    /* 0x92 */ "print  pop, sp", /* `ASM_PRINT_SP' */
    /* 0x93 */ "print  pop, nl", /* `ASM_PRINT_NL' */
    /* 0x94 */ "print  nl", /* `ASM_PRINTNL' */
    /* 0x95 */ "print  pop...", /* `ASM_PRINTALL' */
    /* 0x96 */ "print  pop..., sp", /* `ASM_PRINTALL_SP' */
    /* 0x97 */ "print  pop..., nl", /* `ASM_PRINTALL_NL' */
    /* 0x98 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x99 */ "print  top, pop", /* `ASM_FPRINT' */
    /* 0x9a */ "print  top, pop, sp", /* `ASM_FPRINT_SP' */
    /* 0x9b */ "print  top, pop, nl", /* `ASM_FPRINT_NL' */
    /* 0x9c */ "print  top, nl", /* `ASM_FPRINTNL' */
    /* 0x9d */ "print  top, pop...", /* `ASM_FPRINTALL' */
    /* 0x9e */ "print  top, pop..., sp", /* `ASM_FPRINTALL_SP' */
    /* 0x9f */ "print  top, pop..., nl", /* `ASM_FPRINTALL_NL' */
    /* 0xa0 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xa1 */ "print  ", /* `ASM_PRINT_C' */
    /* 0xa2 */ "print  ", /* `ASM_PRINT_C_SP' */
    /* 0xa3 */ "print  ", /* `ASM_PRINT_C_NL' */
    /* 0xa4 */ "push   range $0, " PREFIX_INTEGERAL, /* `ASM_RANGE_0_I16' */
    /* 0xa5 */ "push   range $0, " PREFIX_INTEGERAL, /* `ASM_RANGE_0_I32' */
    /* 0xa6 */ "enter  top", /* `ASM_ENTER' */
    /* 0xa7 */ "leave  pop", /* `ASM_LEAVE' */
    /* 0xa8 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xa9 */ "print  top, ", /* `ASM_FPRINT_C' */
    /* 0xaa */ "print  top, ", /* `ASM_FPRINT_C_SP' */
    /* 0xab */ "print  top, ", /* `ASM_FPRINT_C_NL' */
    /* 0xac */ "range  top, pop", /* `ASM_RANGE' */
    /* 0xad */ "push   range default, pop", /* `ASM_RANGE_DEF' */
    /* 0xae */ "range  top, pop, pop", /* `ASM_RANGE_STEP' */
    /* 0xaf */ "push   range default, pop, pop", /* `ASM_RANGE_STEP_DEF' */
    /* 0xb0 */ "getsize top", /* `ASM_GETSIZE' */
    /* 0xb1 */ "contains top, pop", /* `ASM_CONTAINS' */
    /* 0xb2 */ "getitem top, pop", /* `ASM_GETITEM' */
    /* 0xb3 */ "getitem top, " PREFIX_INTEGERAL, /* `ASM_GETITEM_I' */
    /* 0xb4 */ "getitem top, ", /* `ASM_GETITEM_C' */
    /* 0xb5 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xb6 */ "setitem pop, pop, pop", /* `ASM_SETITEM' */
    /* 0xb7 */ "setitem pop, " PREFIX_INTEGERAL, /* `ASM_SETITEM_I' */
    /* 0xb8 */ "setitem pop, ", /* `ASM_SETITEM_C' */
    /* 0xb9 */ "iterself top", /* `ASM_ITERSELF' */
    /* 0xba */ "delitem pop, pop", /* `ASM_DELITEM' */
    /* 0xbb */ "getrange top, pop, pop", /* `ASM_GETRANGE' */
    /* 0xbc */ "getrange top, pop, none", /* `ASM_GETRANGE_PN' */
    /* 0xbd */ "getrange top, none, pop", /* `ASM_GETRANGE_NP' */
    /* 0xbe */ "getrange top, pop, " PREFIX_INTEGERAL, /* `ASM_GETRANGE_PI' */
    /* 0xbf */ "getrange top, " PREFIX_INTEGERAL, /* `ASM_GETRANGE_IP' */
    /* 0xc0 */ "getrange top, none, " PREFIX_INTEGERAL, /* `ASM_GETRANGE_NI' */
    /* 0xc1 */ "getrange top, " PREFIX_INTEGERAL, /* `ASM_GETRANGE_IN' */
    /* 0xc2 */ "getrange top, " PREFIX_INTEGERAL, /* `ASM_GETRANGE_II' */
    /* 0xc3 */ "delrange pop, pop, pop", /* `ASM_DELRANGE' */
    /* 0xc4 */ "setrange pop, pop, pop, pop", /* `ASM_SETRANGE' */
    /* 0xc5 */ "setrange pop, pop, none, pop", /* `ASM_SETRANGE_PN' */
    /* 0xc6 */ "setrange pop, none, pop, pop", /* `ASM_SETRANGE_NP' */
    /* 0xc7 */ "setrange pop, pop, " PREFIX_INTEGERAL, /* `ASM_SETRANGE_PI' */
    /* 0xc8 */ "setrange pop, " PREFIX_INTEGERAL, /* `ASM_SETRANGE_IP' */
    /* 0xc9 */ "setrange pop, none, " PREFIX_INTEGERAL, /* `ASM_SETRANGE_NI' */
    /* 0xca */ "setrange pop, " PREFIX_INTEGERAL, /* `ASM_SETRANGE_IN' */
    /* 0xcb */ "setrange pop, " PREFIX_INTEGERAL, /* `ASM_SETRANGE_II' */
    /* 0xcc */ "debug  break", /* `ASM_BREAKPOINT' */
    /* 0xcd */ "ud", /* `ASM_UD' */
    /* 0xce */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xcf */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xd0 */ "callattr top, pop, " PREFIX_STACKEFFECT, /* `ASM_CALLATTR' */
    /* 0xd1 */ "callattr top, pop, pop", /* `ASM_CALLATTR_TUPLE' */
    /* 0xd2 */ "callattr top, ", /* `ASM_CALLATTR_C' */
    /* 0xd3 */ "callattr top, ", /* `ASM_CALLATTR_TUPLE_C' */
    /* 0xd4 */ "push   callattr this, ", /* `ASM_CALLATTR_THIS_C' */
    /* 0xd5 */ "push   callattr this, ", /* `ASM_CALLATTR_THIS_TUPLE_C' */
    /* 0xd6 */ "push   getmember this, pop, " PREFIX_INTEGERAL, /* `ASM_GETMEMBER' */
    /* 0xd7 */ "push   getmember this, ref ", /* `ASM_GETMEMBER_R' */
    /* 0xd8 */ "delmember this, pop, " PREFIX_INTEGERAL, /* `ASM_DELMEMBER' */
    /* 0xd9 */ "delmember this, ref ", /* `ASM_DELMEMBER_R' */
    /* 0xda */ "setmember this, pop, " PREFIX_INTEGERAL, /* `ASM_SETMEMBER' */
    /* 0xdb */ "setmember this, ref ", /* `ASM_SETMEMBER_R' */
    /* 0xdc */ "push   hasmember this, pop, " PREFIX_INTEGERAL, /* `ASM_HASMEMBER' */
    /* 0xdd */ "push   call extern ", /* `ASM_CALL_EXTERN' */
    /* 0xde */ "push   call global ", /* `ASM_CALL_GLOBAL' */
    /* 0xdf */ "push   call local ", /* `ASM_CALL_LOCAL' */
    /* 0xe0 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xe1 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xe2 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xe3 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xe4 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xe5 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xe6 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xe7 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xe8 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xe9 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xea */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xeb */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xec */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xed */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xee */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xef */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf1 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf2 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf3 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf4 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf5 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf6 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf7 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf8 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf9 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xfa */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xfb */ "stack  " PREFIX_STACKEFFECT, /* `ASM_STACK' */
    /* 0xfc */ "static ", /* `ASM_STATIC' */
    /* 0xfd */ "extern ", /* `ASM_EXTERN' */
    /* 0xfe */ "global ", /* `ASM_GLOBAL' */
    /* 0xff */ "local  ", /* `ASM_LOCAL' */
};
PRIVATE char const mnemonic_names_f0[256][30] = {
    /* 0xf000 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf001 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf002 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf003 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf004 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf005 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf006 */ "end    catch, " PREFIX_STACKEFFECT, /* `ASM_ENDCATCH_N' */
    /* 0xf007 */ "end    finally, " PREFIX_STACKEFFECT, /* `ASM_ENDFINALLY_N' */
    /* 0xf008 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf009 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf00a */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf00b */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf00c */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf00d */ "push   bnd extern ", /* `ASM16_PUSH_BND_EXTERN' */
    /* 0xf00e */ "push   bnd global ", /* `ASM16_PUSH_BND_GLOBAL' */
    /* 0xf00f */ "push   bnd local ", /* `ASM16_PUSH_BND_LOCAL' */
    /* 0xf010 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf011 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf012 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf013 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf014 */ "jmp    ", /* `ASM32_JMP' */
    /* 0xf015 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf016 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf017 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf018 */ "jmp    pop, #pop", /* `ASM_JMP_POP_POP' */
    /* 0xf019 */ "call   top, [" PREFIX_STACKEFFECT, /* `ASM_CALL_SEQ' */
    /* 0xf01a */ "call   top, {" PREFIX_STACKEFFECT, /* `ASM_CALL_MAP' */
    /* 0xf01b */ "op     top, " PREFIX_INTEGERAL, /* `ASM16_OPERATOR' */
    /* 0xf01c */ "op     top, " PREFIX_INTEGERAL, /* `ASM16_OPERATOR_TUPLE' */
    /* 0xf01d */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf01e */ "del    global ", /* `ASM16_DEL_GLOBAL' */
    /* 0xf01f */ "del    local ", /* `ASM16_DEL_LOCAL' */
    /* 0xf020 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf021 */ "lrot   " PREFIX_STACKEFFECT, /* `ASM16_LROT' */
    /* 0xf022 */ "rrot   " PREFIX_STACKEFFECT, /* `ASM16_RROT' */
    /* 0xf023 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf024 */ "dup    #SP - ", /* `ASM16_DUP_N' */
    /* 0xf025 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf026 */ "pop    #SP - ", /* `ASM16_POP_N' */
    /* 0xf027 */ "adjstack #SP ", /* `ASM16_ADJSTACK' */
    /* 0xf028 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf029 */ "push   super ref ", /* `ASM16_SUPER_THIS_R' */
    /* 0xf02a */ "push   super global ", /* `ASM16_SUPER_THIS_G' */
    /* 0xf02b */ "push   super extern ", /* `ASM16_SUPER_THIS_E' */
    /* 0xf02c */ "pop    static ", /* `ASM16_POP_STATIC' */
    /* 0xf02d */ "pop    extern ", /* `ASM16_POP_EXTERN' */
    /* 0xf02e */ "pop    global ", /* `ASM16_POP_GLOBAL' */
    /* 0xf02f */ "pop    local ", /* `ASM16_POP_LOCAL' */
    /* 0xf030 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf031 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf032 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf033 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf034 */ "push   except", /* `ASM_PUSH_EXCEPT' */
    /* 0xf035 */ "push   this", /* `ASM_PUSH_THIS' */
    /* 0xf036 */ "push   this_module ", /* `ASM_PUSH_THIS_MODULE' */
    /* 0xf037 */ "push   this_function", /* `ASM_PUSH_THIS_FUNCTION' */
    /* 0xf038 */ "push   module ", /* `ASM16_PUSH_MODULE' */
    /* 0xf039 */ "push   ref ", /* `ASM16_PUSH_REF' */
    /* 0xf03a */ "push   arg ", /* `ASM16_PUSH_ARG' */
    /* 0xf03b */ "push   ", /* `ASM16_PUSH_CONST' */
    /* 0xf03c */ "push   static ", /* `ASM16_PUSH_STATIC' */
    /* 0xf03d */ "push   extern ", /* `ASM16_PUSH_EXTERN' */
    /* 0xf03e */ "push   global ", /* `ASM16_PUSH_GLOBAL' */
    /* 0xf03f */ "push   local ", /* `ASM16_PUSH_LOCAL' */
    /* 0xf040 */ "cast   top, hashset", /* `ASM_CAST_HASHSET' */
    /* 0xf041 */ "cast   top, dict", /* `ASM_CAST_DICT' */
    /* 0xf042 */ "push   pack tuple, " PREFIX_STACKEFFECT, /* `ASM16_PACK_TUPLE' */
    /* 0xf043 */ "push   pack list, " PREFIX_STACKEFFECT, /* `ASM16_PACK_LIST' */
    /* 0xf044 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf045 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf046 */ "unpack pop, " PREFIX_STACKEFFECT, /* `ASM16_UNPACK' */
    /* 0xf047 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf048 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf049 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf04a */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf04b */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf04c */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf04d */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf04e */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf04f */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf050 */ "push   true", /* `ASM_PUSH_TRUE' */
    /* 0xf051 */ "push   false", /* `ASM_PUSH_FALSE' */
    /* 0xf052 */ "push   pack hashset, " PREFIX_STACKEFFECT, /* `ASM_PACK_HASHSET' */
    /* 0xf053 */ "push   pack dict, " PREFIX_STACKEFFECT, /* `ASM_PACK_DICT' */
    /* 0xf054 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf055 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf056 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf057 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf058 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf059 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf05a */ "getattr top, ", /* `ASM16_GETATTR_C' */
    /* 0xf05b */ "delattr pop, ", /* `ASM16_DELATTR_C' */
    /* 0xf05c */ "setattr pop, ", /* `ASM16_SETATTR_C' */
    /* 0xf05d */ "push   getattr this, ", /* `ASM16_GETATTR_THIS_C' */
    /* 0xf05e */ "delattr this, ", /* `ASM16_DELATTR_THIS_C' */
    /* 0xf05f */ "setattr this, ", /* `ASM16_SETATTR_THIS_C' */
    /* 0xf060 */ "cmp    so, top, pop", /* `ASM_CMP_SO' */
    /* 0xf061 */ "cmp    do, top, pop", /* `ASM_CMP_DO' */
    /* 0xf062 */ "push   pack hashset, " PREFIX_STACKEFFECT, /* `ASM16_PACK_HASHSET' */
    /* 0xf063 */ "push   pack dict, " PREFIX_STACKEFFECT, /* `ASM16_PACK_DICT' */
    /* 0xf064 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf065 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf066 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf067 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf068 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf069 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf06a */ "defmember top, " PREFIX_INTEGERAL, /* `ASM16_DEFMEMBER' */
    /* 0xf06b */ "defop  top, " PREFIX_INTEGERAL, /* `ASM16_DEFOP' */
    /* 0xf06c */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf06d */ "push   function ", /* `ASM16_FUNCTION_C_0' */
    /* 0xf06e */ "push   function ", /* `ASM16_FUNCTION_C' */
    /* 0xf06f */ "push   function ", /* `ASM16_FUNCTION_C_16' */
    /* 0xf070 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf071 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf072 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf073 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf074 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf075 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf076 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf077 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf078 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf079 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf07a */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf07b */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf07c */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf07d */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf07e */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf07f */ "push   inc", /* `ASM_INCPOST' */
    /* 0xf080 */ "push   dec", /* `ASM_DECPOST' */
    /* 0xf081 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf082 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf083 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf084 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf085 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf086 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf087 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf088 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf089 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf08a */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf08b */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf08c */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf08d */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf08e */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf08f */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf090 */ "nop16", /* `ASM16_NOP' */
    /* 0xf091 */ "reduce top, min", /* `ASM_REDUCE_MIN' */
    /* 0xf092 */ "reduce top, max", /* `ASM_REDUCE_MAX' */
    /* 0xf093 */ "reduce top, sum", /* `ASM_REDUCE_SUM' */
    /* 0xf094 */ "reduce top, any", /* `ASM_REDUCE_ANY' */
    /* 0xf095 */ "reduce top, all", /* `ASM_REDUCE_ALL' */
    /* 0xf096 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf097 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf098 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf099 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf09a */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf09b */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf09c */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf09d */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf09e */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf09f */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0a0 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0a1 */ "print  ", /* `ASM16_PRINT_C' */
    /* 0xf0a2 */ "print  ", /* `ASM16_PRINT_C_SP' */
    /* 0xf0a3 */ "print  ", /* `ASM16_PRINT_C_NL' */
    /* 0xf0a4 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0a5 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0a6 */ "unpack varargs, " PREFIX_STACKEFFECT, /* `ASM_VARARGS_UNPACK' */
    /* 0xf0a7 */ "call   top, arg ", /* `ASM_CALL_ARGSFWD' */
    /* 0xf0a8 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0a9 */ "print  top, ", /* `ASM16_FPRINT_C' */
    /* 0xf0aa */ "print  top, ", /* `ASM16_FPRINT_C_SP' */
    /* 0xf0ab */ "print  top, ", /* `ASM16_FPRINT_C_NL' */
    /* 0xf0ac */ "push   cmp eq, #varargs, " PREFIX_INTEGERAL, /* `ASM_VARARGS_CMP_EQ_SZ' */
    /* 0xf0ad */ "push   cmp gr, #varargs, " PREFIX_INTEGERAL, /* `ASM_VARARGS_CMP_GR_SZ' */
    /* 0xf0ae */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0af */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0b0 */ "push   getsize varargs", /* `ASM_VARARGS_GETSIZE' */
    /* 0xf0b1 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0b2 */ "getitem varargs, top", /* `ASM_VARARGS_GETITEM' */
    /* 0xf0b3 */ "push   getitem varargs, " PREFIX_INTEGERAL, /* `ASM_VARARGS_GETITEM_I' */
    /* 0xf0b4 */ "getitem top, ", /* `ASM16_GETITEM_C' */
    /* 0xf0b5 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0b6 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0b7 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0b8 */ "setitem pop, ", /* `ASM16_SETITEM_C' */
    /* 0xf0b9 */ "iternext top", /* `ASM_ITERNEXT' */
    /* 0xf0ba */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0bb */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0bc */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0bd */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0be */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0bf */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0c0 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0c1 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0c2 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0c3 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0c4 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0c5 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0c6 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0c7 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0c8 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0c9 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0ca */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0cb */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0cc */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0cd */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0ce */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0cf */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0d0 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0d1 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0d2 */ "callattr top, ", /* `ASM16_CALLATTR_C' */
    /* 0xf0d3 */ "callattr top, ", /* `ASM16_CALLATTR_TUPLE_C' */
    /* 0xf0d4 */ "callattr this, ", /* `ASM16_CALLATTR_THIS_C' */
    /* 0xf0d5 */ "callattr this, ", /* `ASM16_CALLATTR_THIS_TUPLE_C' */
    /* 0xf0d6 */ "push   getmember this, pop, " PREFIX_INTEGERAL, /* `ASM16_GETMEMBER' */
    /* 0xf0d7 */ "push   getmember this, ref ", /* `ASM16_GETMEMBER_R' */
    /* 0xf0d8 */ "delmember this, pop, " PREFIX_INTEGERAL, /* `ASM16_DELMEMBER' */
    /* 0xf0d9 */ "delmember this, ref ", /* `ASM16_DELMEMBER_R' */
    /* 0xf0da */ "setmember this, pop, " PREFIX_INTEGERAL, /* `ASM16_SETMEMBER' */
    /* 0xf0db */ "setmember this, ref ", /* `ASM16_SETMEMBER_R' */
    /* 0xf0dc */ "push   hasmember this, pop, " PREFIX_INTEGERAL, /* `ASM16_HASMEMBER' */
    /* 0xf0dd */ "push   call extern ", /* `ASM16_CALL_EXTERN' */
    /* 0xf0de */ "push   call global ", /* `ASM16_CALL_GLOBAL' */
    /* 0xf0df */ "push   call local ", /* `ASM16_CALL_LOCAL' */
    /* 0xf0e0 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0e1 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0e2 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0e3 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0e4 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0e5 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0e6 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0e7 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0e8 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0e9 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0ea */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0eb */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0ec */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0ed */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0ee */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0ef */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0f0 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0f1 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0f2 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0f3 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0f4 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0f5 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0f6 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0f7 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0f8 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0f9 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0fa */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0fb */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0fc */ "static ", /* `ASM16_STATIC' */
    /* 0xf0fd */ "extern ", /* `ASM16_EXTERN' */
    /* 0xf0fe */ "global ", /* `ASM16_GLOBAL' */
    /* 0xf0ff */ "local  ", /* `ASM16_LOCAL' */
};
//[[[end]]]

#ifndef __DEEMON__
#define print(p,s)  do{ if ((temp = (*printer)(closure,p,s)) < 0) goto err; result += temp; }__WHILE0
#define PRINT(s)    print(s,COMPILER_STRLEN(s))
#define printf(...) do{ if ((temp = Dee_FormatPrintf(printer,closure,__VA_ARGS__)) < 0) goto err; result += temp; }__WHILE0
#endif

/* How many number of code bytes printed before mnemonics.
 * Remaining bytes are then printed in the following line. */
#define LINE_MAXBYTES 4

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4701)
#endif

INTERN dssize_t DCALL
code_disasm(dformatprinter printer, void *closure,
            DeeCodeObject *__restrict self,
            code_addr_t addr_begin, code_addr_t addr_end,
            unsigned int flags) {
 dssize_t temp,result = 0;
 instruction_t const *iter,*end;
 (void)flags;
 ASSERT_OBJECT_TYPE(self,&DeeCode_Type);
 if (addr_end >= self->co_codebytes)
     addr_end = self->co_codebytes;
 if (addr_begin >= addr_end) goto done;
 iter = self->co_code+addr_begin;
 end  = self->co_code+addr_end;
 for (;;) {
  char bytes[INSTRLEN_MAX*3]; size_t i,num_bytes;
  DREF DeeObject *comment_ob = NULL;
  code_addr_t target; uint32_t imm;
  instruction_t const *iiter;
  uint16_t class_imm,id;
  char const *comment = NULL,*mnemonic;
  instruction_t const *next;
  code_addr_t ip_addr;
  ip_addr = (code_addr_t)(iter - self->co_code);
  if (!(flags&DISASM_FNOEXCEPT)) {
   /* Print exception handler labels. */
   uint16_t i,c = self->co_exceptc;
   for (i = 0; i < c; ++i) {
    struct except_handler *item = &self->co_exceptv[i];
    PRIVATE char const except_name[2][8] = { "except", "finally" };
#if EXCEPTION_HANDLER_FFINALLY == 1
    char const *name = except_name[item->eh_flags&EXCEPTION_HANDLER_FFINALLY];
#else
    char const *name = except_name[item->eh_flags&EXCEPTION_HANDLER_FFINALLY ? 1 : 0];
#endif
    if (item->eh_start == ip_addr)
        printf(".L%s_%I16u_start:\n",name,i);
    if (item->eh_end == ip_addr)
        printf(".L%s_%I16u_end:\n",name,i);
    if (item->eh_addr == ip_addr) {
     printf(".except .L%s_%I16u_start, .L%s_%I16u_end, .L%s_%I16u_entry",name,i,name,i,name,i);
     if (item->eh_flags&EXCEPTION_HANDLER_FFINALLY) PRINT(", @finally");
     if (item->eh_flags&EXCEPTION_HANDLER_FINTERPT) PRINT(", @interrupt");
     if (item->eh_flags&EXCEPTION_HANDLER_FHANDLED) PRINT(", @handled");
     if (item->eh_mask) printf(", @mask(%k)",item->eh_mask);
     PRINT("\n");
     printf(".L%s_%I16u_entry:\n",name,i);
    }
   }
  }
  if (iter >= end) break;
#define READ_IMM(T) (*(*(T **)&iiter)++)
#define READ_imm8()                   READ_IMM(uint8_t)
#define READ_imm16()   ASM_BSWAPIMM16(READ_IMM(uint16_t))
#define READ_imm32()   ASM_BSWAPIMM32(READ_IMM(uint32_t))
#define READ_Simm8()                  READ_IMM(int8_t)
#define READ_Simm16() ASM_BSWAPSIMM16(READ_IMM(int16_t))
#define READ_Simm32() ASM_BSWAPSIMM32(READ_IMM(int32_t))
  iiter = iter;
  next = asm_nextinstr(iter);
  id = *iiter++;
  if (id == ASM_DELOP) goto next_instr; /* This one should be omitted */
  printf(PREFIX_ADDRESS "%.4I32X   ",ip_addr);
  num_bytes = (size_t)(next-iter);
  for (i = 0; i < num_bytes; ++i) {
   uint8_t byte;
   bytes[(i*3)+2] = ' ';
   byte = iter[i] & 0x0f;
   bytes[(i*3)+1] = (char)(byte >= 10 ? 'A'+(byte-10) : '0'+byte);
   byte = iter[i] >> 4;
   bytes[(i*3)+0] = (char)(byte >= 10 ? 'A'+(byte-10) : '0'+byte);
  }
  i = MIN(num_bytes,LINE_MAXBYTES);
  print(bytes,i*3);
  num_bytes -= i;
  i = LINE_MAXBYTES-i;
  while (i--) PRINT("   ");
#if 1 /* Print the expected stack depth if known. */
  {
   struct ddi_state state;
   unsigned int sp_width = 1;
   uint16_t stack_max = (uint16_t)(self->co_framesize-(self->co_localc*sizeof(DeeObject *)))/sizeof(DeeObject *);
   /* */if (stack_max >= 100) sp_width = 3;
   else if (stack_max >= 10)  sp_width = 2;
   if (DeeCode_FindDDI((DeeObject *)self,&state,NULL,ip_addr,
                        DDI_STATE_FNOTHROW|DDI_STATE_FNONAMES)) {
    if (state.rs_regs.dr_uip == ip_addr)
         printf("[%.*d] ",sp_width,(unsigned int)state.rs_regs.dr_usp);
    else print("      ",sp_width+3);
    ddi_state_fini(&state);
   } else {
    print("      ",sp_width+3);
   }
  }
#endif
do_instruction:
  if (id == ASM_EXTENDED1) {
   id = (uint16_t)(((uint16_t)ASM_EXTENDED1 << 8) | *iiter++);
do_instruction_f0:
   mnemonic = mnemonic_names_f0[id & 0xff];
   print(mnemonic,strlen(mnemonic));
   goto do_switch_id;
  }
  if (id == ASM_RET && (self->co_flags&CODE_FYIELDING)) {
   /* In yield-functions, ASM_RET is overwritten with a different instruction. */
   PRINT("yield  pop");
   goto done_instr;
  }
  mnemonic = mnemonic_names[id];
do_instruction_mnemonic:
  print(mnemonic,strlen(mnemonic));
do_switch_id:
  switch (id) {

  case ASM_INC:
  case ASM_DEC:
  case ASM_INCPOST:
  case ASM_DECPOST:
   if (!ASM_ISPREFIX(*iter))
        comment = "Illegal due to missing prefix";
   break;

  case ASM_CLASS:
  case ASM_CLASS_C:
  case ASM_CLASS_CBL:
  case ASM_CLASS_CBG:
   class_imm = READ_imm8();
   {
    PRIVATE char const class_flag_names[4][10] = {
     /* TP_FFINAL     */"FINAL",
     /* TP_FTRUNCATE  */"TRUNCATE",
     /* TP_FINTERRUPT */"INTERRUPT",
     /* 0x8           */""
    };
    uint8_t flags = class_imm&0xf;
    uint8_t index = 0; bool is_first = true;
    for (; flags; flags >>= 1,++index) {
     char const *name;
     if (!(flags&1)) continue;
     name = class_flag_names[index];
     if (!is_first) PRINT("|");
     is_first = false;
     if (*name) printf("TP_F%s",name);
     else printf("%#x",(unsigned int)(1 << index));
    }
    if (is_first) PRINT("0");
   }
continue_class_repr:
   if (class_imm&CLASSGEN_FHASBASE) {
    class_imm &= ~CLASSGEN_FHASBASE;
    PRINT(", b:");
    if (id == ASM_CLASS)
     PRINT("pop");
    else {
     imm = READ_imm8();
     if (id == ASM_CLASS_C) goto print_const;
     if (id == ASM_CLASS_CBL) { PRINT("local "); goto print_local; }
     PRINT("global ");
     goto print_global;
    }
   }
   if (class_imm&CLASSGEN_FHASNAME) {
    class_imm &= ~CLASSGEN_FHASNAME;
    PRINT(", n:");
    if (id == ASM_CLASS)
     PRINT("pop");
    else {
     imm = READ_imm8();
     goto print_const;
    }
   }
   if (class_imm&CLASSGEN_FHASIMEM) {
    class_imm &= ~CLASSGEN_FHASIMEM;
    PRINT(", i:");
    if (id == ASM_CLASS)
     PRINT("pop");
    else {
     imm = READ_imm8();
     goto print_const;
    }
   }
   if (class_imm&CLASSGEN_FHASCMEM) {
    class_imm &= ~CLASSGEN_FHASCMEM;
    PRINT(", c:");
    if (id == ASM_CLASS)
     PRINT("pop");
    else {
     imm = READ_imm8();
     goto print_const;
    }
   }
   break;

  case ASM32_JMP:
   target = READ_Simm32();
   goto print_target;
  case ASM_JF16:
  case ASM_JT16:
  case ASM_JMP16:
  case ASM_FOREACH16:
   target = READ_Simm16();
   goto print_target;
  case ASM_JF:
  case ASM_JT:
  case ASM_JMP:
  case ASM_FOREACH:
   target = READ_Simm8();
print_target:
   target += (code_addr_t)(next - self->co_code);
   printf(PREFIX_ADDRESS "%.4I32X",target);
   if (target >= self->co_codebytes) {
    comment = "Invalid ip";
   }
   break;

  case ASM_LROT:
  case ASM_RROT:
   imm = READ_imm8()+3;
   goto print_imm8;
  case ASM_DUP_N:
  case ASM_POP_N:
   imm = READ_imm8()+2;
   goto print_imm8;
  case ASM_CALL_MAP:
   printf("%I16u}",(uint16_t)READ_imm8()*2);
   break;
  case ASM_CALL_SEQ:
   printf("%I8u]",READ_imm8());
   break;
  case ASM_VARARGS_UNPACK:
  case ASM_VARARGS_GETITEM_I:
  case ASM_VARARGS_CMP_EQ_SZ:
  case ASM_VARARGS_CMP_GR_SZ:
   printf("%I8u",READ_imm8());
   break;
  case ASM_CALL:
  case ASM_EXTEND:
  case ASM_PACK_TUPLE:
  case ASM_PACK_LIST:
  case ASM_UNPACK:
  case ASM_SHL_IMM8:
  case ASM_SHR_IMM8:
  case ASM_CALLATTR:
  case ASM_PACK_HASHSET:
  case ASM_GETMEMBER:
  case ASM_DELMEMBER:
   imm = READ_imm8();
print_imm8:
   /* NOTE: We upcast to 16-bit to prevent overflow
    *       of the +2 / +3 addends above. */
   printf("%I16u",(uint16_t)imm);
   break;

  case ASM_PACK_DICT:
   imm = READ_imm8();
   goto print_pack_dict;
  case ASM16_PACK_DICT:
   imm = READ_imm16();
print_pack_dict:
   printf("%I32u",(uint32_t)imm*2);
   goto done_instr;

  case ASM16_PACK_TUPLE:
  case ASM16_PACK_LIST:
  case ASM16_UNPACK:
  case ASM16_LROT:
  case ASM16_RROT:
  case ASM16_DUP_N:
  case ASM16_POP_N:
  case ASM16_PACK_HASHSET:
  case ASM_RANGE_0_I16:
  case ASM16_GETMEMBER:
  case ASM16_DELMEMBER:
   printf("%I16u",READ_imm16());
   break;
  case ASM_RANGE_0_I32:
   printf("%I32u",READ_imm16());
   break;
  case ASM_SETITEM_I:
   printf("%I16u, pop",READ_imm16());
   break;

  case ASM_ADD_SIMM8:
  case ASM_SUB_SIMM8:
  case ASM_MUL_SIMM8:
  case ASM_DIV_SIMM8:
  case ASM_MOD_SIMM8:
   printf("%I8d",READ_Simm8());
   break;

  {
   int16_t val;
  case ASM16_ADJSTACK:
   val = READ_Simm16();
   goto do_print_adjstack;
  case ASM_ADJSTACK:
   val = READ_Simm8();
do_print_adjstack:
   printf("%c %I32d",
          val < 0 ? '-' : '+',
          val < 0 ? (int32_t)-val : (int32_t)val);
  } break;

  {
   struct opinfo *info;
  case ASM16_DEFOP:
   imm = READ_imm16();
   goto do_print_op;
  case ASM_DEFOP:
   imm = READ_imm8();
do_print_op:
   /* Special case: Try to print the name of the operator, rather than its ID. */
   info = Dee_OperatorInfo(NULL,(uint16_t)imm);
   if (info) {
    printf("__%s__, pop",info->oi_sname);
   } else {
    printf("%I16u, pop",(uint16_t)imm);
   }
   break;
  }
  case ASM_DEFMEMBER:
  case ASM_SETMEMBER:
   printf("%I8u, pop",READ_imm8());
   break;
  case ASM16_DEFMEMBER:
  case ASM16_SETMEMBER:
   printf("%I16u, pop",READ_imm16());
   break;

  case ASM_ENDCATCH_N:
  case ASM_ENDFINALLY_N:
   printf("%I16u",READ_imm8()+1);
   break;

  case ASM_ADD_IMM32:
  case ASM_SUB_IMM32:
  case ASM_AND_IMM32:
  case ASM_OR_IMM32:
  case ASM_XOR_IMM32:
   printf("%#I32x",READ_imm32());
   break;

  {
   struct opinfo *info;
  case ASM16_OPERATOR:
   imm = READ_imm16();
   goto do_print_operator;
  case ASM_OPERATOR:
   imm = READ_imm8();
do_print_operator:
   info = Dee_OperatorInfo(NULL,(uint16_t)imm);
   if (info) {
    printf("__%s__, " PREFIX_STACKEFFECT "%I8u",
           info->oi_sname,READ_imm8());
   } else {
    printf("%I16u, " PREFIX_STACKEFFECT "%I8u",
           imm,READ_imm8());
   }
  } break;

  {
   struct opinfo *info;
  case ASM16_OPERATOR_TUPLE:
   imm = READ_imm16();
   goto do_print_operator_tuple;
  case ASM_OPERATOR_TUPLE:
   imm = READ_imm8();
do_print_operator_tuple:
   info = Dee_OperatorInfo(NULL,(uint16_t)imm);
   if (info) {
    printf("__%s__, pop",info->oi_sname);
   } else {
    printf("%I16u, pop",imm);
   }
  } break;

  case ASM_GETRANGE_PI:
  case ASM_GETRANGE_NI:
  case ASM_GETITEM_I:
   printf("%I16d",READ_Simm16());
   break;
  case ASM_GETRANGE_IP:
  case ASM_SETRANGE_PI:
  case ASM_SETRANGE_NI:
   printf("%I16d, pop",READ_Simm16());
   break;

  case ASM_GETRANGE_IN:
   printf("%I16d, none",READ_Simm16());
   break;

  case ASM_SETRANGE_IP:
   printf("%I16d, pop, pop",READ_Simm16());
   break;

  case ASM_SETRANGE_IN:
   printf("%I16d, none, pop",READ_Simm16());
   break;

  case ASM_GETRANGE_II:
  case ASM_SETRANGE_II:
   imm = (uint32_t)(uint16_t)READ_Simm16();
   printf("%I16d, " PREFIX_INTEGERAL "%I16d",(int16_t)(uint16_t)imm,READ_Simm16());
   if (id == ASM_SETRANGE_II) PRINT(", pop");
   break;

  {
   uint16_t imm2;
   DeeModuleObject *impmod;
  case ASM16_PUSH_BND_EXTERN:
  case ASM16_POP_EXTERN:
  case ASM16_PUSH_EXTERN:
  case ASM16_EXTERN:
  case ASM16_CALL_EXTERN:
  case ASM16_SUPER_THIS_E:
   imm  = READ_imm16();
   imm2 = READ_imm16();
   goto print_extern;
  case ASM_PUSH_BND_EXTERN:
  case ASM_POP_EXTERN:
  case ASM_PUSH_EXTERN:
  case ASM_EXTERN:
  case ASM_CALL_EXTERN:
  case ASM_SUPER_THIS_E:
   imm  = READ_imm8();
   imm2 = READ_imm8();
print_extern:
   if (imm >= self->co_module->mo_importc) {
    comment = "Invalid mid";
    printf("%I16u:%I16u",(uint16_t)imm,(uint16_t)imm2);
   } else if (imm2 >= (impmod = self->co_module->mo_importv[imm])->mo_globalc) {
invalid_gid_extern:
    comment = "Invalid gid";
    printf(PREFIX_VARNAME "%k:%I16u",impmod->mo_name,(uint16_t)imm2);
   } else {
    DeeObject *name = DeeModule_GlobalName((DeeObject *)impmod,imm2);
    if (!name) goto invalid_gid_extern;
    printf(PREFIX_VARNAME "%k:" PREFIX_VARNAME "%k",impmod->mo_name,name);
   }
   if (id == ASM_EXTERN || id == ASM16_EXTERN)
       goto do_prefix;
   if (id == ASM_CALL_EXTERN || id == ASM16_CALL_EXTERN)
       printf(", " PREFIX_STACKEFFECT "%I8u",READ_imm8());
   if (id == ASM_SUPER_THIS_E || id == ASM16_SUPER_THIS_E)
       PRINT(", this");
  } break;


  case ASM16_PUSH_MODULE:
   imm = READ_imm16();
   goto print_module;
  case ASM_PUSH_MODULE:
   imm = READ_imm8();
print_module:
   if (imm >= self->co_module->mo_importc) {
    comment = "Invalid mid";
    printf("%I16u",imm);
   } else {
    printf(PREFIX_VARNAME "%k",self->co_module->mo_importv[imm]->mo_name);
   }
   break;

  case ASM16_PUSH_BND_GLOBAL:
  case ASM16_DEL_GLOBAL:
  case ASM16_POP_GLOBAL:
  case ASM16_PUSH_GLOBAL:
  case ASM16_GLOBAL:
  case ASM16_CALL_GLOBAL:
  case ASM16_SUPER_THIS_G:
   imm = READ_imm16();
   goto print_global;
  case ASM_PUSH_BND_GLOBAL:
  case ASM_DEL_GLOBAL:
  case ASM_POP_GLOBAL:
  case ASM_PUSH_GLOBAL:
  case ASM_GLOBAL:
  case ASM_CALL_GLOBAL:
  case ASM_SUPER_THIS_G:
   imm = READ_imm8();
print_global:
   if (imm >= self->co_module->mo_globalc) {
    comment = "Invalid gid";
unknown_gid:
    printf("%I16u",imm);
   } else {
    DeeObject *name = DeeModule_GlobalName((DeeObject *)self->co_module,
                                           (uint16_t)imm);
    if unlikely(!name)
       goto unknown_gid;
    printf(PREFIX_VARNAME "%k",name);
   }
   if (id == ASM_CLASS_CBG)
       goto continue_class_repr;
   if (id == ASM_GLOBAL || id == ASM16_GLOBAL)
       goto do_prefix;
   if (id == ASM_CALL_GLOBAL || id == ASM16_CALL_GLOBAL)
       printf(", " PREFIX_STACKEFFECT "%I8u",READ_imm8());
   if (id == ASM_SUPER_THIS_G || id == ASM16_SUPER_THIS_G)
       PRINT(", this");
   break;

  case ASM16_PUSH_BND_LOCAL:
  case ASM16_DEL_LOCAL:
  case ASM16_POP_LOCAL:
  case ASM16_PUSH_LOCAL:
  case ASM16_LOCAL:
  case ASM16_CALL_LOCAL:
   imm = READ_imm16();
   goto print_local;
  case ASM_PUSH_BND_LOCAL:
  case ASM_DEL_LOCAL:
  case ASM_POP_LOCAL:
  case ASM_PUSH_LOCAL:
  case ASM_LOCAL:
  case ASM_CALL_LOCAL:
   imm = READ_imm8();
print_local:
   if (imm >= self->co_localc) {
    comment = "Invalid lid";
    printf("%I16u",imm);
   } else {
    struct ddi_state state;
    /* Use DDI information to lookup the name of the variable. */
    if (DeeCode_FindDDI((DeeObject *)self,&state,NULL,ip_addr,DDI_STATE_FNOTHROW)) {
     if (imm < state.rs_xregs.dx_lcnamc &&
         DeeDDI_VALID_SYMBOL(self->co_ddi,state.rs_xregs.dx_lcnamv[imm]))
         printf("@%s",DeeDDI_SYMBOL_NAME(self->co_ddi,state.rs_xregs.dx_lcnamv[imm]));
     else printf("%I16u",imm);
     ddi_state_fini(&state);
    } else {
     printf("%I16u",imm);
    }
   }
   if (id == ASM_CLASS_CBL)
       goto continue_class_repr;
   if (id == ASM_LOCAL || id == ASM16_LOCAL)
       goto do_prefix;
   if (id == ASM_CALL_LOCAL || id == ASM16_CALL_LOCAL)
       printf(", " PREFIX_STACKEFFECT "%I8u",READ_imm8());
   break;

  case ASM16_PUSH_REF:
  case ASM16_SUPER_THIS_R:
#ifdef ASM16_CALL_REF
  case ASM16_CALL_REF:
#endif
  case ASM16_GETMEMBER_R:
  case ASM16_DELMEMBER_R:
  case ASM16_SETMEMBER_R:
   imm = READ_imm16();
   goto print_ref;
  case ASM_PUSH_REF:
  case ASM_SUPER_THIS_R:
  case ASM_GETMEMBER_R:
  case ASM_DELMEMBER_R:
  case ASM_SETMEMBER_R:
#ifdef ASM_CALL_REF
  case ASM_CALL_REF:
#endif
   imm = READ_imm8();
print_ref:
   if (imm >= self->co_refc) {
    comment = "Invalid rid";
    printf("%I16u",imm);
   } else if (DeeDDI_HAS_REF(self->co_ddi,imm)) {
    printf(PREFIX_VARNAME "%s",DeeDDI_REF_NAME(self->co_ddi,imm));
   } else {
    printf("%I16u",imm);
   }
   switch (id) {
#ifdef ASM_CALL_REF
   case ASM_CALL_REF:
   case ASM16_CALL_REF:
    printf(", " PREFIX_STACKEFFECT "%I8u",READ_imm8());
    break;
#endif
   case ASM_SUPER_THIS_R:
   case ASM16_SUPER_THIS_R:
    PRINT(", this");
    break;
   case ASM_GETMEMBER_R:
   case ASM_DELMEMBER_R:
   case ASM_SETMEMBER_R:
    printf(", " PREFIX_INTEGERAL "%I8u",READ_imm8());
    if (id == ASM_SETMEMBER_R) PRINT(", pop");
    break;
   case ASM16_GETMEMBER_R:
   case ASM16_DELMEMBER_R:
   case ASM16_SETMEMBER_R:
    printf(", " PREFIX_INTEGERAL "%I16u",READ_imm16());
    if (id == ASM16_SETMEMBER_R) PRINT(", pop");
    break;
   default: break;
   }
   break;

  case ASM16_PUSH_ARG:
   imm = READ_imm16();
   goto print_arg;
  case ASM_PUSH_ARG:
  case ASM_CALL_ARGSFWD:
do_read_and_print_arg:
   imm = READ_imm8();
print_arg:
   if (imm == self->co_argc_max &&
       self->co_flags&CODE_FVARARGS) {
    comment = "Varargs";
    printf("%I16u",imm);
   } else if (imm >= self->co_argc_max) {
    comment = "Invalid aid";
    printf("%I16u",imm);
   } else if (DeeDDI_HAS_ARG(self->co_ddi,imm)) {
    printf(PREFIX_VARNAME "%s",DeeDDI_ARG_NAME(self->co_ddi,imm));
   } else {
    printf("%I16u",imm);
   }
   if (id == ASM_CALL_ARGSFWD) {
    /* Print the second argument index. */
    PRINT(", arg ");
    id = 0;
    goto do_read_and_print_arg;
   }
   break;

  case ASM16_POP_STATIC:
  case ASM16_PUSH_STATIC:
  case ASM16_STATIC:
  case ASM16_PUSH_CONST:
  case ASM16_PRINT_C:
  case ASM16_FPRINT_C:
  case ASM16_GETITEM_C:
  case ASM16_GETATTR_C:
  case ASM16_DELATTR_C:
  case ASM16_GETATTR_THIS_C:
  case ASM16_DELATTR_THIS_C:
  case ASM16_PRINT_C_SP:
  case ASM16_PRINT_C_NL:
  case ASM16_FPRINT_C_SP:
  case ASM16_FPRINT_C_NL:
  case ASM16_SETITEM_C:
  case ASM16_CALLATTR_TUPLE_C:
  case ASM16_CALLATTR_THIS_TUPLE_C:
  case ASM16_SETATTR_C:
  case ASM16_SETATTR_THIS_C:
  case ASM16_CALLATTR_C:
  case ASM16_CALLATTR_THIS_C:
  case ASM16_FUNCTION_C_0:
  case ASM16_FUNCTION_C:
  case ASM16_FUNCTION_C_16:
   imm = READ_imm16();
   goto print_const;
  case ASM_POP_STATIC:
  case ASM_PUSH_STATIC:
  case ASM_STATIC:
  case ASM_PUSH_CONST:
  case ASM_PRINT_C:
  case ASM_FPRINT_C:
  case ASM_GETITEM_C:
  case ASM_GETATTR_C:
  case ASM_DELATTR_C:
  case ASM_GETATTR_THIS_C:
  case ASM_DELATTR_THIS_C:
  case ASM_PRINT_C_SP:
  case ASM_PRINT_C_NL:
  case ASM_FPRINT_C_SP:
  case ASM_FPRINT_C_NL:
  case ASM_SETITEM_C:
  case ASM_CALLATTR_TUPLE_C:
  case ASM_CALLATTR_THIS_TUPLE_C:
  case ASM_SETATTR_C:
  case ASM_SETATTR_THIS_C:
  case ASM_CALLATTR_C:
  case ASM_CALLATTR_THIS_C:
  case ASM_FUNCTION_C_0:
  case ASM_FUNCTION_C:
  case ASM_FUNCTION_C_16:
   imm = READ_imm8();
print_const:
   if (imm >= self->co_staticc) {
    comment = "Invalid sid";
    printf("const %I16u",imm);
   } else {
    /* Print a string representation of a constant's value as a comment. */
    if (id != ASM_POP_STATIC && id != ASM16_POP_STATIC &&
        id != ASM_PUSH_STATIC && id != ASM16_PUSH_STATIC &&
        id != ASM_STATIC && id != ASM16_STATIC) {
     DREF DeeObject *constant_value;
#ifndef CONFIG_NO_THREADS
     rwlock_read(&self->co_static_lock);
#endif
     constant_value = self->co_staticv[imm];
     Dee_XIncref(constant_value);
#ifndef CONFIG_NO_THREADS
     rwlock_endread(&self->co_static_lock);
#endif
     if (constant_value) {
      if (DeeInt_Check(constant_value)) {
       unsigned int numsys;
       uint32_t value;
       PRINT(PREFIX_INTEGERAL);
       numsys = 16;
       if (((DeeIntObject *)constant_value)->ob_size < 0 ||
           (DeeInt_TryGetU32(constant_value,&value) && value <= UINT16_MAX))
            numsys = 10;
       temp = DeeInt_Print(constant_value,
                           DEEINT_PRINT(numsys,DEEINT_PRINT_FNUMSYS),
                           printer,closure);
       if (temp < 0) goto err;
       result += temp;
       Dee_Decref(constant_value);
       goto did_print_constant_value;
      }
#if 1
      if (asm_allowconst(constant_value) &&
         !DeeCode_CheckExact(constant_value) &&
         !DeeMemberTable_CheckExact(constant_value)) {
       printf(PREFIX_CONSTANT "%r",constant_value);
       Dee_Decref(constant_value);
       goto did_print_constant_value;
      }
#endif
      if (!comment_ob) {
       if (DeeCode_Check(constant_value)) {
        /* Don't print representation of code (They don't look good) */
       } else {
        comment_ob = DeeObject_Repr(constant_value);
        if likely(comment_ob) {
         comment = DeeString_STR(comment_ob);
        } else {
         if (!DeeError_Catch(&DeeError_NotImplemented)) {
          temp = -1;
          goto err;
         }
        }
       }
       if (!comment_ob) {
        comment_ob = DeeObject_Str((DeeObject *)Dee_TYPE(constant_value));
        if likely(comment_ob) {
         comment = DeeString_STR(comment_ob);
        } else {
         if (!DeeError_Catch(&DeeError_NotImplemented)) {
          temp = -1;
          goto err;
         }
        }
       }
      } /* comment_ob == NULL */
      Dee_Decref(constant_value);
     } /* constant_value != NULL */
     PRINT("const ");
    }
    if (DeeDDI_HAS_STATIC(self->co_ddi,imm)) {
     printf(PREFIX_VARNAME "%s",DeeDDI_STATIC_NAME(self->co_ddi,imm));
    } else {
     printf("%I16u",imm);
    }
   }
did_print_constant_value:
   switch (id) {
   case ASM_CLASS_C:
   case ASM_CLASS_CBL:
    goto continue_class_repr;
   case ASM_PRINT_C_SP:
   case ASM16_PRINT_C_SP:
   case ASM_FPRINT_C_SP:
   case ASM16_FPRINT_C_SP:
    PRINT(", sp");
    break;
   case ASM_PRINT_C_NL:
   case ASM16_PRINT_C_NL:
   case ASM_FPRINT_C_NL:
   case ASM16_FPRINT_C_NL:
    PRINT(", nl");
    break;
   case ASM_SETITEM_C:
   case ASM16_SETITEM_C:
   case ASM_CALLATTR_TUPLE_C:
   case ASM16_CALLATTR_TUPLE_C:
   case ASM_CALLATTR_THIS_TUPLE_C:
   case ASM16_CALLATTR_THIS_TUPLE_C:
   case ASM_SETATTR_C:
   case ASM16_SETATTR_C:
   case ASM_SETATTR_THIS_C:
   case ASM16_SETATTR_THIS_C:
    PRINT(", pop");
    break;
   case ASM_STATIC:
   case ASM16_STATIC:
    goto do_prefix;
   case ASM_CALLATTR_C:
   case ASM16_CALLATTR_C:
   case ASM_CALLATTR_THIS_C:
   case ASM16_CALLATTR_THIS_C:
    printf(", " PREFIX_STACKEFFECT "%I8u",READ_imm8());
    break;
   case ASM_FUNCTION_C_0:
   case ASM16_FUNCTION_C_0:
    PRINT(", " PREFIX_STACKEFFECT "0");
    break;
   case ASM_FUNCTION_C:
   case ASM16_FUNCTION_C:
   case ASM_FUNCTION_C_16:
   case ASM16_FUNCTION_C_16:
    if (id == ASM_FUNCTION_C_16 ||
        id == ASM16_FUNCTION_C_16) {
     imm = READ_imm16();
    } else {
     imm = READ_imm8();
    }
    printf(", " PREFIX_STACKEFFECT "%I32u",(uint32_t)imm+1);
    break;

   default: break;
   }
   break;

  case ASM16_STACK:
   imm = READ_imm16();
   goto do_prefix_stack;
  case ASM_STACK:
   imm = READ_imm8();
do_prefix_stack:
   printf("%I16u",imm);
   /*ATTR_FALLTHROUGH*/
do_prefix:
   PRINT(": ");
   id = *iiter++;
   if (id == ASM_EXTENDED1) id = (ASM_EXTENDED1 << 8) | *iiter++;
   mnemonic = ((id&0xff00) ? mnemonic_names_f0[id >> 8]
                           : mnemonic_names[id]);
   if ((id >= ASM_FUNCTION_C_0 && id <= ASM_FUNCTION_C_16) ||
       (id >= ASM16_FUNCTION_C_0 && id <= ASM16_FUNCTION_C_16)) {
    mnemonic += MAX(MNEMONIC_MINWIDTH,4)+1; /* Skip `push ' */
    goto do_instruction_mnemonic;
   }
   if (id >= ASM_ADD && id <= ASM_POW) {
    /* Print by removing the `top, ' from the mnemonic. */
    printf("%$s pop",(size_t)(id == ASM_OR ? 3 : 4),mnemonic);
    goto done_instr;
   }
   if (id == ASM_OPERATOR || id == ASM_OPERATOR_TUPLE ||
       id == ASM16_OPERATOR || id == ASM16_OPERATOR_TUPLE) {
    printf("push op %s",mnemonic+COMPILER_STRLEN("op     top, "));
    goto do_switch_id;
   }
   if (id & 0xff00)
       goto do_instruction_f0;
   goto do_instruction;

  default: break;
  }
done_instr:
  if (comment) printf("\t" COMMENT("%s"),comment);
  PRINT("\n");
  if (num_bytes) {
   PRINT(PREFIX_ADDRESS_SPC "       ");
   bytes[LINE_MAXBYTES*3+(num_bytes*3)-1] = '\n';
   print(bytes+LINE_MAXBYTES*3,(num_bytes*3));
  }
next_instr:
  iter = next;
  Dee_XDecref(comment_ob);
 }
done:
 if (!(flags & DISASM_FNOSUBCODE)) {
  uint16_t i;
  for (i = 0; i < self->co_staticc; ++i) {
   DREF DeeObject *constant_value;
#ifndef CONFIG_NO_THREADS
   rwlock_read(&self->co_static_lock);
#endif
   constant_value = self->co_staticv[i];
   if (!constant_value || !DeeCode_Check(constant_value)) {
#ifndef CONFIG_NO_THREADS
    rwlock_endread(&self->co_static_lock);
#endif
    continue;
   }
   Dee_Incref(constant_value);
#ifndef CONFIG_NO_THREADS
   rwlock_endread(&self->co_static_lock);
#endif
   temp = Dee_FormatPrintf(printer,closure,".const %I16u {\n",i);
   if unlikely(temp < 0) { Dee_Decref(constant_value); goto err; }
   result += temp;
   temp = code_disasm(printer,closure,
                     (DeeCodeObject *)constant_value,
                      0,(code_addr_t)-1,flags);
   Dee_Decref(constant_value);
   if unlikely(temp < 0) goto err;
   result += temp;
   temp = (*printer)(closure,"}\n",2);
   if unlikely(temp < 0) goto err;
   result += temp;
  }
 }
 return result;
err:
 return temp;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_DISASM_C */
