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
#ifndef GUARD_DEX_FS_PRINTINSTR_C
#define GUARD_DEX_FS_PRINTINSTR_C 1

#include <deemon/api.h>
#include <deemon/dex.h>
#include <deemon/object.h>
#include <deemon/asm.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/string.h>
#include <deemon/code.h>
#include <deemon/class.h>

#include "libdisasm.h"

#include <hybrid/minmax.h>

#include <hybrid/unaligned.h>
#include <hybrid/byteswap.h>
#include <hybrid/byteorder.h>

#include <string.h>

DECL_BEGIN


#define UNKNOWN_MNEMONIC    "?"
#define PREFIX_ADDRESS      "" /* "+" */
#define PREFIX_ADDRESS_SPC  "" /* " " */
#define PREFIX_STACKEFFECT  "#" /* Prefix for operands that affect the stack-effect */
#define PREFIX_INTEGERAL    "$" /* Prefix for operands that are integral immediate values. */
#define PREFIX_VARNAME      "@" /* Prefix for variable names. */
#define PREFIX_CONSTANT     "@" /* Prefix for constant expressions. */
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
    function strip_tail(x,y) {
        if (x.endswith(y) && x.isspace(#x-(#y+1)))
            return x[:#x-#y];
        return x;
    }
    short_desc = strip_tail(short_desc,"ref");
    short_desc = strip_tail(short_desc,"arg");
    short_desc = strip_tail(short_desc,"const");
    short_desc = strip_tail(short_desc,"static");
    short_desc = strip_tail(short_desc,"extern");
    short_desc = strip_tail(short_desc,"global");
    short_desc = strip_tail(short_desc,"module");
    short_desc = strip_tail(short_desc,"local");
    //if (short_desc.endswith(" const"))
    //    short_desc = short_desc[:#short_desc-#"const"];
    if (short_desc.endswith("$ ") ||
        short_desc.endswith("# "))
        short_desc = short_desc[:#short_desc-1];
    if (short_desc.endswith(",") ||
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
    /* 0x05 */ "setret pop", /* `ASM_SETRET' */
    /* 0x06 */ "end    catch", /* `ASM_ENDCATCH' */
    /* 0x07 */ "end    finally", /* `ASM_ENDFINALLY' */
    /* 0x08 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x09 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x0a */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x0b */ "call   top, " PREFIX_STACKEFFECT, /* `ASM_CALL_KW' */
    /* 0x0c */ "call   top, pop..., ", /* `ASM_CALL_TUPLE_KW' */
    /* 0x0d */ "push   bound ", /* `ASM_PUSH_BND_EXTERN' */
    /* 0x0e */ "push   bound ", /* `ASM_PUSH_BND_GLOBAL' */
    /* 0x0f */ "push   bound ", /* `ASM_PUSH_BND_LOCAL' */
    /* 0x10 */ "jf     pop, ", /* `ASM_JF' */
    /* 0x11 */ "jf     pop, ", /* `ASM_JF16' */
    /* 0x12 */ "jt     pop, ", /* `ASM_JT' */
    /* 0x13 */ "jt     pop, ", /* `ASM_JT16' */
    /* 0x14 */ "jmp    ", /* `ASM_JMP' */
    /* 0x15 */ "jmp    ", /* `ASM_JMP16' */
    /* 0x16 */ "foreach top, ", /* `ASM_FOREACH' */
    /* 0x17 */ "foreach top, ", /* `ASM_FOREACH16' */
    /* 0x18 */ "jmp    pop", /* `ASM_JMP_POP' */
    /* 0x19 */ "op     top, " PREFIX_INTEGERAL, /* `ASM_OPERATOR' */
    /* 0x1a */ "op     top, " PREFIX_INTEGERAL, /* `ASM_OPERATOR_TUPLE' */
    /* 0x1b */ "call   top, " PREFIX_STACKEFFECT, /* `ASM_CALL' */
    /* 0x1c */ "call   top, pop...", /* `ASM_CALL_TUPLE' */
    /* 0x1d */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x1e */ "del    ", /* `ASM_DEL_GLOBAL' */
    /* 0x1f */ "del    ", /* `ASM_DEL_LOCAL' */
    /* 0x20 */ "swap", /* `ASM_SWAP' */
    /* 0x21 */ "lrot   " PREFIX_STACKEFFECT, /* `ASM_LROT' */
    /* 0x22 */ "rrot   " PREFIX_STACKEFFECT, /* `ASM_RROT' */
    /* 0x23 */ "dup", /* `ASM_DUP' */
    /* 0x24 */ "dup    #SP - ", /* `ASM_DUP_N' */
    /* 0x25 */ "pop", /* `ASM_POP' */
    /* 0x26 */ "pop    #SP - ", /* `ASM_POP_N' */
    /* 0x27 */ "adjstack #SP ", /* `ASM_ADJSTACK' */
    /* 0x28 */ "super  top, pop", /* `ASM_SUPER' */
    /* 0x29 */ "push   super this, ", /* `ASM_SUPER_THIS_R' */
    /* 0x2a */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x2b */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x2c */ "pop    ", /* `ASM_POP_STATIC' */
    /* 0x2d */ "pop    ", /* `ASM_POP_EXTERN' */
    /* 0x2e */ "pop    ", /* `ASM_POP_GLOBAL' */
    /* 0x2f */ "pop    ", /* `ASM_POP_LOCAL' */
    /* 0x30 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x31 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x32 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x33 */ "push   none", /* `ASM_PUSH_NONE' */
    /* 0x34 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x35 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x36 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x37 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x38 */ "push   ", /* `ASM_PUSH_MODULE' */
    /* 0x39 */ "push   ", /* `ASM_PUSH_ARG' */
    /* 0x3a */ "push   ", /* `ASM_PUSH_CONST' */
    /* 0x3b */ "push   ", /* `ASM_PUSH_REF' */
    /* 0x3c */ "push   ", /* `ASM_PUSH_STATIC' */
    /* 0x3d */ "push   ", /* `ASM_PUSH_EXTERN' */
    /* 0x3e */ "push   ", /* `ASM_PUSH_GLOBAL' */
    /* 0x3f */ "push   ", /* `ASM_PUSH_LOCAL' */
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
    /* 0x59 */ "boundattr top, pop", /* `ASM_BOUNDATTR' */
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
    /* 0x66 */ "class  top, ", /* `ASM_CLASS_C' */
    /* 0x67 */ "push   class ", /* `ASM_CLASS_GC' */
    /* 0x68 */ "push   class ", /* `ASM_CLASS_EC' */
    /* 0x69 */ "defcmember top, " PREFIX_INTEGERAL, /* `ASM_DEFCMEMBER' */
    /* 0x6a */ "push   getcmember ", /* `ASM_GETCMEMBER_R' */
    /* 0x6b */ "push   callcmember this, ", /* `ASM_CALLCMEMBER_THIS_R' */
    /* 0x6c */ UNKNOWN_MNEMONIC, /* --- */
    /* 0x6d */ UNKNOWN_MNEMONIC, /* --- */
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
    /* 0xa5 */ UNKNOWN_MNEMONIC, /* --- */
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
    /* 0xb0 */ "contains top, pop", /* `ASM_CONTAINS' */
    /* 0xb1 */ "push   contains ", /* `ASM_CONTAINS_C' */
    /* 0xb2 */ "getitem top, pop", /* `ASM_GETITEM' */
    /* 0xb3 */ "getitem top, " PREFIX_INTEGERAL, /* `ASM_GETITEM_I' */
    /* 0xb4 */ "getitem top, ", /* `ASM_GETITEM_C' */
    /* 0xb5 */ "getsize top", /* `ASM_GETSIZE' */
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
    /* 0xce */ "callattr top, ", /* `ASM_CALLATTR_C_KW' */
    /* 0xcf */ "callattr top, ", /* `ASM_CALLATTR_C_TUPLE_KW' */
    /* 0xd0 */ "callattr top, pop, " PREFIX_STACKEFFECT, /* `ASM_CALLATTR' */
    /* 0xd1 */ "callattr top, pop, pop...", /* `ASM_CALLATTR_TUPLE' */
    /* 0xd2 */ "callattr top, ", /* `ASM_CALLATTR_C' */
    /* 0xd3 */ "callattr top, ", /* `ASM_CALLATTR_C_TUPLE' */
    /* 0xd4 */ "push   callattr this, ", /* `ASM_CALLATTR_THIS_C' */
    /* 0xd5 */ "push   callattr this, ", /* `ASM_CALLATTR_THIS_C_TUPLE' */
    /* 0xd6 */ "callattr top, ", /* `ASM_CALLATTR_C_SEQ' */
    /* 0xd7 */ "callattr top, ", /* `ASM_CALLATTR_C_MAP' */
    /* 0xd8 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xd9 */ "push   getmember this, ", /* `ASM_GETMEMBER_THIS_R' */
    /* 0xda */ "delmember this, ", /* `ASM_DELMEMBER_THIS_R' */
    /* 0xdb */ "setmember this, ", /* `ASM_SETMEMBER_THIS_R' */
    /* 0xdc */ "push   boundmember this, ", /* `ASM_BOUNDMEMBER_THIS_R' */
    /* 0xdd */ "push   call ", /* `ASM_CALL_EXTERN' */
    /* 0xde */ "push   call ", /* `ASM_CALL_GLOBAL' */
    /* 0xdf */ "push   call ", /* `ASM_CALL_LOCAL' */
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
PRIVATE char const mnemonic_names_f0[256][32] = {
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
    /* 0xf00b */ "call   top, " PREFIX_STACKEFFECT, /* `ASM16_CALL_KW' */
    /* 0xf00c */ "call   top, pop..., ", /* `ASM16_CALL_TUPLE_KW' */
    /* 0xf00d */ "push   bnd ", /* `ASM16_PUSH_BND_EXTERN' */
    /* 0xf00e */ "push   bnd ", /* `ASM16_PUSH_BND_GLOBAL' */
    /* 0xf00f */ "push   bnd ", /* `ASM16_PUSH_BND_LOCAL' */
    /* 0xf010 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf011 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf012 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf013 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf014 */ "jmp    ", /* `ASM32_JMP' */
    /* 0xf015 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf016 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf017 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf018 */ "jmp    pop, #pop", /* `ASM_JMP_POP_POP' */
    /* 0xf019 */ "op     top, " PREFIX_INTEGERAL, /* `ASM16_OPERATOR' */
    /* 0xf01a */ "op     top, " PREFIX_INTEGERAL, /* `ASM16_OPERATOR_TUPLE' */
    /* 0xf01b */ "call   top, [" PREFIX_STACKEFFECT, /* `ASM_CALL_SEQ' */
    /* 0xf01c */ "call   top, {" PREFIX_STACKEFFECT, /* `ASM_CALL_MAP' */
    /* 0xf01d */ "call   top, pop, pop...", /* `ASM_THISCALL_TUPLE' */
    /* 0xf01e */ "del    ", /* `ASM16_DEL_GLOBAL' */
    /* 0xf01f */ "del    ", /* `ASM16_DEL_LOCAL' */
    /* 0xf020 */ "call   top, pop..., pop", /* `ASM_CALL_TUPLE_KWDS' */
    /* 0xf021 */ "lrot   " PREFIX_STACKEFFECT, /* `ASM16_LROT' */
    /* 0xf022 */ "rrot   " PREFIX_STACKEFFECT, /* `ASM16_RROT' */
    /* 0xf023 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf024 */ "dup    #SP - ", /* `ASM16_DUP_N' */
    /* 0xf025 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf026 */ "pop    #SP - ", /* `ASM16_POP_N' */
    /* 0xf027 */ "adjstack #SP ", /* `ASM16_ADJSTACK' */
    /* 0xf028 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf029 */ "push   super this, ", /* `ASM16_SUPER_THIS_R' */
    /* 0xf02a */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf02b */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf02c */ "pop    ", /* `ASM16_POP_STATIC' */
    /* 0xf02d */ "pop    ", /* `ASM16_POP_EXTERN' */
    /* 0xf02e */ "pop    ", /* `ASM16_POP_GLOBAL' */
    /* 0xf02f */ "pop    ", /* `ASM16_POP_LOCAL' */
    /* 0xf030 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf031 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf032 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf033 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf034 */ "push   except", /* `ASM_PUSH_EXCEPT' */
    /* 0xf035 */ "push   this", /* `ASM_PUSH_THIS' */
    /* 0xf036 */ "push   this_module", /* `ASM_PUSH_THIS_MODULE' */
    /* 0xf037 */ "push   this_function", /* `ASM_PUSH_THIS_FUNCTION' */
    /* 0xf038 */ "push   ", /* `ASM16_PUSH_MODULE' */
    /* 0xf039 */ "push   ", /* `ASM16_PUSH_ARG' */
    /* 0xf03a */ "push   ", /* `ASM16_PUSH_CONST' */
    /* 0xf03b */ "push   ", /* `ASM16_PUSH_REF' */
    /* 0xf03c */ "push   ", /* `ASM16_PUSH_STATIC' */
    /* 0xf03d */ "push   ", /* `ASM16_PUSH_EXTERN' */
    /* 0xf03e */ "push   ", /* `ASM16_PUSH_GLOBAL' */
    /* 0xf03f */ "push   ", /* `ASM16_PUSH_LOCAL' */
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
    /* 0xf059 */ "bounditem top, pop", /* `ASM_BOUNDITEM' */
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
    /* 0xf064 */ "getcmember top, " PREFIX_INTEGERAL, /* `ASM16_GETCMEMBER' */
    /* 0xf065 */ "class  top, pop", /* `ASM_CLASS' */
    /* 0xf066 */ "class  top, ", /* `ASM16_CLASS_C' */
    /* 0xf067 */ "push   class ", /* `ASM16_CLASS_GC' */
    /* 0xf068 */ "push   class ", /* `ASM16_CLASS_EC' */
    /* 0xf069 */ "defcmember top, " PREFIX_INTEGERAL, /* `ASM16_DEFCMEMBER' */
    /* 0xf06a */ "push   getcmember ", /* `ASM16_GETCMEMBER_R' */
    /* 0xf06b */ "push   callcmember this, ", /* `ASM16_CALLCMEMBER_THIS_R' */
    /* 0xf06c */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf06d */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf06e */ "push   function ", /* `ASM16_FUNCTION_C' */
    /* 0xf06f */ "push   function ", /* `ASM16_FUNCTION_C_16' */
    /* 0xf070 */ "push   getattr this, ", /* `ASM_SUPERGETATTR_THIS_RC' */
    /* 0xf071 */ "push   getattr this, ", /* `ASM16_SUPERGETATTR_THIS_RC' */
    /* 0xf072 */ "push   callattr this, ", /* `ASM_SUPERCALLATTR_THIS_RC' */
    /* 0xf073 */ "push   callattr this, ", /* `ASM16_SUPERCALLATTR_THIS_RC' */
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
    /* 0xf0a4 */ "push   range $0, " PREFIX_INTEGERAL, /* `ASM_RANGE_0_I32' */
    /* 0xf0a5 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0a6 */ "unpack varargs, " PREFIX_STACKEFFECT, /* `ASM_VARARGS_UNPACK' */
    /* 0xf0a7 */ "call   top, ", /* `ASM_CALL_ARGSFWD' */
    /* 0xf0a8 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0a9 */ "print  top, ", /* `ASM16_FPRINT_C' */
    /* 0xf0aa */ "print  top, ", /* `ASM16_FPRINT_C_SP' */
    /* 0xf0ab */ "print  top, ", /* `ASM16_FPRINT_C_NL' */
    /* 0xf0ac */ "push   cmp eq, #varargs, " PREFIX_INTEGERAL, /* `ASM_VARARGS_CMP_EQ_SZ' */
    /* 0xf0ad */ "push   cmp gr, #varargs, " PREFIX_INTEGERAL, /* `ASM_VARARGS_CMP_GR_SZ' */
    /* 0xf0ae */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0af */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0b0 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0b1 */ "push   contains ", /* `ASM16_CONTAINS_C' */
    /* 0xf0b2 */ "getitem varargs, top", /* `ASM_VARARGS_GETITEM' */
    /* 0xf0b3 */ "push   getitem varargs, " PREFIX_INTEGERAL, /* `ASM_VARARGS_GETITEM_I' */
    /* 0xf0b4 */ "getitem top, ", /* `ASM16_GETITEM_C' */
    /* 0xf0b5 */ "push   getsize varargs", /* `ASM_VARARGS_GETSIZE' */
    /* 0xf0b6 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0b7 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0b8 */ "setitem pop, ", /* `ASM16_SETITEM_C' */
    /* 0xf0b9 */ "iternext top", /* `ASM_ITERNEXT' */
    /* 0xf0ba */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0bb */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0bc */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0bd */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0be */ "getmember top, pop, " PREFIX_INTEGERAL, /* `ASM_GETMEMBER' */
    /* 0xf0bf */ "getmember top, pop, " PREFIX_INTEGERAL, /* `ASM16_GETMEMBER' */
    /* 0xf0c0 */ "delmember pop, pop, " PREFIX_INTEGERAL, /* `ASM_DELMEMBER' */
    /* 0xf0c1 */ "delmember pop, pop, " PREFIX_INTEGERAL, /* `ASM16_DELMEMBER' */
    /* 0xf0c2 */ "setmember pop, pop, " PREFIX_INTEGERAL, /* `ASM_SETMEMBER' */
    /* 0xf0c3 */ "setmember pop, pop, " PREFIX_INTEGERAL, /* `ASM16_SETMEMBER' */
    /* 0xf0c4 */ "boundmember top, pop, " PREFIX_INTEGERAL, /* `ASM_BOUNDMEMBER' */
    /* 0xf0c5 */ "boundmember top, pop, " PREFIX_INTEGERAL, /* `ASM16_BOUNDMEMBER' */
    /* 0xf0c6 */ "push   getmember this, pop, " PREFIX_INTEGERAL, /* `ASM_GETMEMBER_THIS' */
    /* 0xf0c7 */ "push   getmember this, pop, " PREFIX_INTEGERAL, /* `ASM16_GETMEMBER_THIS' */
    /* 0xf0c8 */ "delmember this, pop, " PREFIX_INTEGERAL, /* `ASM_DELMEMBER_THIS' */
    /* 0xf0c9 */ "delmember this, pop, " PREFIX_INTEGERAL, /* `ASM16_DELMEMBER_THIS' */
    /* 0xf0ca */ "setmember this, pop, " PREFIX_INTEGERAL, /* `ASM_SETMEMBER_THIS' */
    /* 0xf0cb */ "setmember this, pop, " PREFIX_INTEGERAL, /* `ASM16_SETMEMBER_THIS' */
    /* 0xf0cc */ "push   boundmember this, pop, " PREFIX_INTEGERAL, /* `ASM_BOUNDMEMBER_THIS' */
    /* 0xf0cd */ "push   boundmember this, pop, " PREFIX_INTEGERAL, /* `ASM16_BOUNDMEMBER_THIS' */
    /* 0xf0ce */ "callattr top, ", /* `ASM16_CALLATTR_C_KW' */
    /* 0xf0cf */ "callattr top, ", /* `ASM16_CALLATTR_C_TUPLE_KW' */
    /* 0xf0d0 */ "callattr top, pop, " PREFIX_STACKEFFECT, /* `ASM_CALLATTR_KWDS' */
    /* 0xf0d1 */ "callattr top, pop, pop..., pop", /* `ASM_CALLATTR_TUPLE_KWDS' */
    /* 0xf0d2 */ "callattr top, ", /* `ASM16_CALLATTR_C' */
    /* 0xf0d3 */ "callattr top, ", /* `ASM16_CALLATTR_C_TUPLE' */
    /* 0xf0d4 */ "callattr this, ", /* `ASM16_CALLATTR_THIS_C' */
    /* 0xf0d5 */ "callattr this, ", /* `ASM16_CALLATTR_THIS_C_TUPLE' */
    /* 0xf0d6 */ "callattr top, ", /* `ASM16_CALLATTR_C_SEQ' */
    /* 0xf0d7 */ "callattr top, ", /* `ASM16_CALLATTR_C_MAP' */
    /* 0xf0d8 */ UNKNOWN_MNEMONIC, /* --- */
    /* 0xf0d9 */ "push   getmember this, ", /* `ASM16_GETMEMBER_THIS_R' */
    /* 0xf0da */ "delmember this, ", /* `ASM16_DELMEMBER_THIS_R' */
    /* 0xf0db */ "setmember this, ", /* `ASM16_SETMEMBER_THIS_R' */
    /* 0xf0dc */ "push   boundmember this, ", /* `ASM16_BOUNDMEMBER_THIS_R' */
    /* 0xf0dd */ "push   call ", /* `ASM16_CALL_EXTERN' */
    /* 0xf0de */ "push   call ", /* `ASM16_CALL_GLOBAL' */
    /* 0xf0df */ "push   call ", /* `ASM16_CALL_LOCAL' */
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
    /* 0xf0fb */ "stack  " PREFIX_STACKEFFECT, /* `ASM16_STACK' */
    /* 0xf0fc */ "static ", /* `ASM16_STATIC' */
    /* 0xf0fd */ "extern ", /* `ASM16_EXTERN' */
    /* 0xf0fe */ "global ", /* `ASM16_GLOBAL' */
    /* 0xf0ff */ "local  ", /* `ASM16_LOCAL' */
};
//[[[end]]]


PRIVATE dssize_t DCALL
libdisasm_printconst(dformatprinter printer, void *arg,
                     uint16_t cid, DeeCodeObject *code,
                     unsigned int flags,
                     bool print_interal) {
 if (code) {
  DeeObject *constval;
  if (cid >= code->co_staticc) {
   if (flags & PCODE_FNOBADCOMMENT)
       goto print_generic;
   return Dee_FormatPrintf(printer,arg,"const %u /* invalid cid */",(unsigned int)cid);
  }
  rwlock_read(&code->co_static_lock);
  constval = code->co_staticv[cid];
  Dee_Incref(constval);
  rwlock_endread(&code->co_static_lock);
  if (DeeInt_Check(constval)) {
   dssize_t temp,result = 0;
   unsigned int numsys;
   uint32_t value;
   if (print_interal) {
    temp = (*printer)(arg,PREFIX_INTEGERAL,COMPILER_STRLEN(PREFIX_INTEGERAL));
   } else {
    temp = (*printer)(arg,PREFIX_CONSTANT,COMPILER_STRLEN(PREFIX_CONSTANT));
   }
   if unlikely(temp < 0) return temp;
   result += temp;
   numsys = 16;
   if (((DeeIntObject *)constval)->ob_size < 0 ||
       (DeeInt_TryAsU32(constval,&value) && value <= UINT16_MAX))
        numsys = 10;
   temp = DeeInt_Print(constval,
                       DEEINT_PRINT(numsys,DEEINT_PRINT_FNUMSYS),
                       printer,arg);
   if (temp < 0) return temp;
   result += temp;
   Dee_Decref(constval);
   return result;
  }
  /* TODO: Add the names of code/function/class objects as comments. */
  if (!DeeCode_CheckExact(constval) &&
      !DeeFunction_CheckExact(constval) &&
      !DeeClassDescriptor_Check(constval))
       return Dee_FormatPrintf(printer,arg,PREFIX_CONSTANT "%R",constval);
  Dee_Decref(constval);
 }
print_generic:
 return Dee_FormatPrintf(printer,arg,"const %u",(unsigned int)cid);
}
PRIVATE dssize_t DCALL
libdisasm_printstatic(dformatprinter printer, void *arg,
                      uint16_t sid, bool readonly,
                      DeeCodeObject *code, unsigned int flags) {
#if 1
 if (readonly)
     return libdisasm_printconst(printer,arg,sid,code,flags,false);
#endif
 if (code) {
  char *name;
  if ((name = DeeCode_GetSSymbolName((DeeObject *)code,sid)) != NULL)
      return Dee_FormatPrintf(printer,arg,"static " PREFIX_VARNAME "%s",name);
  if (sid >= code->co_staticc) {
   if (flags & PCODE_FNOBADCOMMENT)
       goto print_generic;
   return Dee_FormatPrintf(printer,arg,"static %u /* invalid sid */",(unsigned int)sid);
  }
#if 0
  if (readonly && !(flags & PCODE_FNOARGCOMMENT)) {
   DREF DeeObject *init;
   rwlock_read(&code->co_static_lock);
   init = code->co_staticv[sid];
   Dee_Incref(init);
   rwlock_endread(&code->co_static_lock);
   return Dee_FormatPrintf(printer,arg,"static %u /* %R */",sid,init);
  }
#endif
 }
print_generic:
 return Dee_FormatPrintf(printer,arg,"static %u",(unsigned int)sid);
}
PRIVATE dssize_t DCALL
libdisasm_printlocal(dformatprinter printer, void *arg,
                     uint16_t lid, struct ddi_state *ddi,
                     DeeCodeObject *code, unsigned int flags) {
 if (code) {
  /* Use DDI information to lookup the name of the variable. */
  if (ddi && lid < ddi->rs_xregs.dx_lcnamc) {
   char *name;
   if ((name = DeeCode_GetDDIString((DeeObject *)code,ddi->rs_xregs.dx_lcnamv[lid])) != NULL)
        return Dee_FormatPrintf(printer,arg,"local " PREFIX_VARNAME "%s",name);
  } 
  if (lid >= code->co_localc) {
   if (flags & PCODE_FNOBADCOMMENT)
       goto print_generic;
   return Dee_FormatPrintf(printer,arg,"local %u /* invalid lid */",(unsigned int)lid);
  }
 }
print_generic:
 return Dee_FormatPrintf(printer,arg,"local %u",(unsigned int)lid);
}
PRIVATE dssize_t DCALL
libdisasm_printstack(dformatprinter printer, void *arg,
                     uint16_t soff, bool is_prefix, struct ddi_state *ddi,
                     DeeCodeObject *code, unsigned int flags) {
 if (code) {
  if (soff >= DeeCode_StackDepth(code)) {
   if (flags & PCODE_FNOBADCOMMENT)
       goto print_generic;
   return Dee_FormatPrintf(printer,arg,"%s%I16u /* invalid stack-offset */",
                           is_prefix ? "stack #" : "#",soff);
  }
  /* Use DDI information to lookup the name of the variable. */
  if (ddi && soff < MIN(ddi->rs_xregs.dx_base.dr_usp,ddi->rs_xregs.dx_spnama)) {
   char *name;
   if ((name = DeeCode_GetDDIString((DeeObject *)code,ddi->rs_xregs.dx_spnamv[soff])) != NULL)
        return Dee_FormatPrintf(printer,arg,"stack " PREFIX_VARNAME "%s",name);
  } 
 }
print_generic:
 return Dee_FormatPrintf(printer,arg,"%s%I16u",
                         is_prefix ? "stack #" : "#",soff);
}
PRIVATE dssize_t DCALL
libdisasm_printrelstack(dformatprinter printer, void *arg,
                        uint16_t stacksz, uint16_t sp_sub,
                        struct ddi_state *ddi, DeeCodeObject *code,
                        unsigned int flags) {
 uint16_t soff;
 if (!sp_sub) goto invalid_offset;
 if (stacksz == (uint16_t)-1) {
print_generic:
  return Dee_FormatPrintf(printer,arg,"#SP - %I16u",sp_sub);
 }
 if (sp_sub > stacksz) goto invalid_offset;
 soff = stacksz - sp_sub;
 if (code) {
  if (soff >= DeeCode_StackDepth(code))
      goto invalid_offset;
  /* Use DDI information to lookup the name of the variable. */
  if (ddi && soff < MIN(ddi->rs_xregs.dx_base.dr_usp,ddi->rs_xregs.dx_spnama)) {
   char *name = DeeCode_GetDDIString((DeeObject *)code,
                                      ddi->rs_xregs.dx_spnamv[soff]);
   if (name) {
    if (!(flags & PCODE_FALTCOMMENT))
        return Dee_FormatPrintf(printer,arg,"stack " PREFIX_VARNAME "%s",name);
    return Dee_FormatPrintf(printer,arg,"stack " PREFIX_VARNAME "%s /* #%I16u / #SP - %I16u */",
                            name,soff,sp_sub);
   }
  } 
 }
 if (!(flags & PCODE_FALTCOMMENT))
       return Dee_FormatPrintf(printer,arg,"#%I16u",soff);
 return Dee_FormatPrintf(printer,arg,"#%I16u /* #SP - %I16u */",soff,sp_sub);
invalid_offset:
 if (flags & PCODE_FNOBADCOMMENT)
     goto print_generic;
 return Dee_FormatPrintf(printer,arg,"#SP - %I16u /* invalid stack-offset */",sp_sub);
}
PRIVATE dssize_t DCALL
libdisasm_printglobal(dformatprinter printer, void *arg,
                      uint16_t gid, DeeCodeObject *code,
                      unsigned int flags) {
 if (code) {
  char const *name;
  if (gid >= code->co_module->mo_globalc) {
   if (flags & PCODE_FNOBADCOMMENT)
       goto print_generic;
   return Dee_FormatPrintf(printer,arg,"global %u /* invalid gid */",(unsigned int)gid);
  }
  name = DeeModule_GlobalName((DeeObject *)code->co_module,gid);
  if (name) return Dee_FormatPrintf(printer,arg,"global " PREFIX_VARNAME "%s",name);
 }
print_generic:
 return Dee_FormatPrintf(printer,arg,"global %u",(unsigned int)gid);
}
PRIVATE dssize_t DCALL
libdisasm_printmodule(dformatprinter printer, void *arg,
                      uint16_t mid, DeeCodeObject *code,
                      unsigned int flags) {
 if (code) {
  DeeStringObject *name;
  if (mid >= code->co_module->mo_importc) {
   if (flags & PCODE_FNOBADCOMMENT)
       goto print_generic;
   return Dee_FormatPrintf(printer,arg,"module %u /* invalid mid */",(unsigned int)mid);
  }
  name = code->co_module->mo_importv[mid]->mo_name;
  return Dee_FormatPrintf(printer,arg,"module " PREFIX_VARNAME "%k",name);
 }
print_generic:
 return Dee_FormatPrintf(printer,arg,"module %u",(unsigned int)mid);
}
PRIVATE dssize_t DCALL
libdisasm_printextern(dformatprinter printer, void *arg,
                      uint16_t mid, uint16_t gid,
                      DeeCodeObject *code, unsigned int flags) {
 if (code) {
  char const *name;
  DeeModuleObject *module;
  if (mid >= code->co_module->mo_importc) {
   if (flags & PCODE_FNOBADCOMMENT)
       goto print_generic;
   return Dee_FormatPrintf(printer,arg,"extern %u:%u /* invalid mid */",(unsigned int)mid,(unsigned int)gid);
  }
  module = code->co_module->mo_importv[mid];
  if (gid >= module->mo_globalc) {
   if (flags & PCODE_FNOBADCOMMENT)
       goto print_unknown_name;
   return Dee_FormatPrintf(printer,arg,"extern " PREFIX_VARNAME "%k:%u /* invalid gid */",module->mo_name,(unsigned int)gid);
  }
  name = DeeModule_GlobalName((DeeObject *)module,gid);
  if (!name) {
print_unknown_name:
   return Dee_FormatPrintf(printer,arg,"extern " PREFIX_VARNAME "%k:%u",module->mo_name,(unsigned int)gid);
  }
  return Dee_FormatPrintf(printer,arg,"extern " PREFIX_VARNAME "%k:" PREFIX_VARNAME "%s",module->mo_name,name);
 }
print_generic:
 return Dee_FormatPrintf(printer,arg,"extern %u:%u",(unsigned int)mid,(unsigned int)gid);
}
PRIVATE dssize_t DCALL
libdisasm_printref(dformatprinter printer, void *arg,
                   uint16_t rid, DeeCodeObject *code,
                   unsigned int flags) {
 if (code) {
  char *name;
  if ((name = DeeCode_GetRSymbolName((DeeObject *)code,rid)) != NULL)
      return Dee_FormatPrintf(printer,arg,"ref " PREFIX_VARNAME "%s",name);
  if (rid >= code->co_refc) {
   if (flags & PCODE_FNOBADCOMMENT)
       goto print_generic;
   return Dee_FormatPrintf(printer,arg,"ref %u /* invalid rid */",(unsigned int)rid);
  }
      
 }
print_generic:
 return Dee_FormatPrintf(printer,arg,"ref %u",(unsigned int)rid);
}
PRIVATE dssize_t DCALL
libdisasm_printarg(dformatprinter printer, void *arg,
                   uint16_t aid, DeeCodeObject *code,
                   unsigned int flags) {
 if (code) {
  char *name = DeeCode_GetASymbolName((DeeObject *)code,aid);
  if (name)
      return Dee_FormatPrintf(printer,arg,"arg " PREFIX_VARNAME "%s",name);
  if (aid == code->co_argc_max && (code->co_flags & CODE_FVARARGS))
      return (*printer)(arg,"varargs",COMPILER_STRLEN("varargs"));
  if (aid >= code->co_argc_max && !(flags & PCODE_FNOBADCOMMENT))
      return Dee_FormatPrintf(printer,arg,"arg %u /* invalid aid */",(unsigned int)aid);
 }
 return Dee_FormatPrintf(printer,arg,"arg %u",(unsigned int)aid);
}
PRIVATE dssize_t DCALL
libdisasm_printprefix(dformatprinter printer, void *arg,
                      instruction_t *__restrict instr_start, bool readonly,
                      struct ddi_state *ddi, DeeCodeObject *code,
                      unsigned int flags) {
 uint16_t opcode;
 instruction_t *iter = instr_start;
 uint16_t imm,imm2;
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
  return libdisasm_printstack(printer,arg,imm,true,ddi,code,flags);

 case ASM16_STATIC:
  imm = *(uint16_t *)(iter + 0);
  goto do_static_prefix;
 case ASM_STATIC:
  imm = *(uint8_t *)(iter + 0);
do_static_prefix:
  return libdisasm_printstatic(printer,arg,imm,readonly,code,flags);

 case ASM16_GLOBAL:
  imm = *(uint16_t *)(iter + 0);
  goto do_global_prefix;
 case ASM_GLOBAL:
  imm = *(uint8_t *)(iter + 0);
do_global_prefix:
  return libdisasm_printglobal(printer,arg,imm,code,flags);

 case ASM16_EXTERN:
  imm  = *(uint16_t *)(iter + 0);
  imm2 = *(uint16_t *)(iter + 2);
  goto do_extern_prefix;
 case ASM_EXTERN:
  imm  = *(uint8_t *)(iter + 0);
  imm2 = *(uint8_t *)(iter + 1);
do_extern_prefix:
  return libdisasm_printextern(printer,arg,imm,imm2,code,flags);

 case ASM16_LOCAL:
  imm = *(uint16_t *)(iter + 0);
  goto do_local_prefix;
 case ASM_LOCAL:
  imm = *(uint8_t *)(iter + 0);
do_local_prefix:
  return libdisasm_printlocal(printer,arg,imm,ddi,code,flags);

 default: break;
 }
 return 0;
}

PRIVATE char const class_flag_names[4][10] = {
    /* [TP_FFINAL]     = */"FINAL",
    /* [TP_FTRUNCATE]  = */"TRUNCATE",
    /* [TP_FINTERRUPT] = */"INTERRUPT",
    /* [0x8]           = */""
};


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
#define READ_imm16(ip)            UNALIGNED_GETLE16(ip.u16++)
#define READ_Simm16(ip) ((int16_t)UNALIGNED_GETLE16(ip.u16++))
#define READ_imm32(ip)            UNALIGNED_GETLE32(ip.u32++)
#define READ_Simm32(ip) ((int32_t)UNALIGNED_GETLE32(ip.u32++))
} ip_t;


INTERN dssize_t DCALL
libdisasm_printinstr(dformatprinter printer, void *arg,
                     instruction_t *__restrict instr_start, uint16_t stacksz,
                     struct ddi_state *ddi, DeeCodeObject *code,
                     unsigned int flags) {
#define INVOKE(expr)              do{ if ((temp = expr) < 0) goto err; result += temp; }__WHILE0
#define print(p,s)                INVOKE((*printer)(arg,p,s))
#define PRINT(s)                  print(s,COMPILER_STRLEN(s))
#define printf(...)               INVOKE(Dee_FormatPrintf(printer,arg,__VA_ARGS__))
 ip_t iter;
 dssize_t temp,result = 0;
 uint16_t opcode,imm,imm2;
 char const *mnemonic;
 int32_t jump_offset;
 iter.ptr = instr_start;
 opcode = *iter.ptr++;
 if (ASM_ISEXTENDED(opcode))
     opcode = (opcode << 8) | *iter.ptr++;
 if (opcode <= UINT8_MAX) {
  mnemonic = mnemonic_names[opcode];
 } else {
  ASSERT(opcode >= (ASM_EXTENDED1 << 8));
  ASSERT(opcode <= (ASM_EXTENDED1 << 8)+0xff);
  /* XXX: If more opcode extension tables get added, they must be made available here! */
  mnemonic = mnemonic_names_f0[opcode & 0xff];
 }
 /* Special case for prefix instructions */
 if (ASM_ISPREFIX(opcode & 0xff)) {
  switch (opcode) {
  case ASM_STACK:
  case ASM_STATIC:
  case ASM_GLOBAL:
  case ASM_LOCAL:
   iter.ptr += 1;
   break;
  case ASM_EXTERN:
  case ASM16_STACK:
  case ASM16_STATIC:
  case ASM16_GLOBAL:
  case ASM16_LOCAL:
   iter.ptr += 2;
   break;
  case ASM16_EXTERN:
   iter.ptr += 4;
   break;
  default: break;
  }
  opcode = *iter.ptr++;
  if (ASM_ISEXTENDED(opcode))
      opcode = (opcode << 8) | *iter.ptr++;
  if (opcode <= UINT8_MAX) {
   mnemonic = mnemonic_names[opcode];
  } else {
   ASSERT(opcode >= (ASM_EXTENDED1 << 8));
   ASSERT(opcode <= (ASM_EXTENDED1 << 8)+0xff);
   /* XXX: If more opcode extension tables get added, they must be made available here! */
   mnemonic = mnemonic_names_f0[opcode & 0xff];
  }
  /* Special representations for when a prefix is being used. */
  switch (opcode) {

  case ASM_RET:
   mnemonic = "ret    ";
   if (code && code->co_flags & CODE_FYIELDING)
       mnemonic = "yield  ";
   goto do_mnemonic_prefix_readonly;
  case ASM_YIELDALL:
   mnemonic = "yield  foreach, ";
   goto do_mnemonic_prefix_readonly;
  case ASM_POP:
   mnemonic = "mov    top, ";
   goto do_mnemonic_prefix_readonly;
  case ASM_THROW:
   mnemonic = "throw  ";
do_mnemonic_prefix_readonly:
   print(mnemonic,strlen(mnemonic));
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,true,ddi,code,flags));
   goto done;
  case ASM_SWAP:
   mnemonic = "swap   top, ";
do_mnemonic_prefix:
   print(mnemonic,strlen(mnemonic));
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   goto done;

  case ASM_JF16:
  case ASM_JT16:
  case ASM_FOREACH16:
   jump_offset = READ_Simm16(iter);
   goto prefix_do_jmp_target;
  case ASM_JF:
  case ASM_JT:
  case ASM_FOREACH:
   jump_offset = READ_Simm8(iter);
prefix_do_jmp_target:
   switch (opcode & 0xff) {
   case ASM_JT:
   case ASM_JT16:
    PRINT("jt     ");
    break;
   case ASM_JF:
   case ASM_JF16:
    PRINT("jf     ");
    break;
   case ASM_FOREACH:
   case ASM_FOREACH16:
    PRINT("foreach ");
    break;
   default: __builtin_unreachable();
   }
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,true,ddi,code,flags));
   PRINT(", ");
   goto do_jmp_target;

  case ASM16_POP_N:
   imm = READ_imm16(iter)+2;
   goto do_pop_as_mov;
  case ASM_POP_N:
   imm = READ_imm8(iter)+2;
do_pop_as_mov:
   PRINT("mov    ");
   INVOKE(libdisasm_printrelstack(printer,arg,stacksz,imm,ddi,code,flags));
do_print_prefix:
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,true,ddi,code,flags));
   goto done;


  case ASM16_DUP_N:
   imm = READ_imm16(iter)+2;
   goto do_dup_as_mov;
  case ASM_DUP_N:
   imm = READ_imm8(iter)+2;
do_dup_as_mov:
   PRINT("mov    ");
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   PRINT(", ");
   INVOKE(libdisasm_printrelstack(printer,arg,stacksz,imm,ddi,code,flags));
   goto done;
  case ASM_DUP:
   PRINT("mov    ");
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   PRINT(", top");
   goto done;


  case ASM16_LROT:
  case ASM16_RROT:
   imm = READ_imm16(iter)+2;
   goto do_lr_rot;
  case ASM_LROT:
  case ASM_RROT:
   imm = READ_imm8(iter)+2;
do_lr_rot:
   printf("%crot   #%u, ",(opcode & 0xff) == ASM_LROT ? 'l' : 'r',imm);
   goto do_print_prefix;

  case ASM16_POP_STATIC:
   imm = READ_imm16(iter);
   goto do_prefix_pop_static;
  case ASM_POP_STATIC:
   imm = READ_imm8(iter);
do_prefix_pop_static:
   PRINT("mov    ");
   INVOKE(libdisasm_printstatic(printer,arg,imm,false,code,flags));
do_print_comma_then_prefix:
   PRINT(", ");
   goto do_print_prefix;

  case ASM16_POP_EXTERN:
   imm  = READ_imm16(iter);
   imm2 = READ_imm16(iter);
   goto do_prefix_pop_extern;
  case ASM_POP_EXTERN:
   imm  = READ_imm8(iter);
   imm2 = READ_imm8(iter);
do_prefix_pop_extern:
   PRINT("mov    ");
   INVOKE(libdisasm_printextern(printer,arg,imm,imm2,code,flags));
   goto do_print_comma_then_prefix;

  case ASM16_POP_GLOBAL:
   imm = READ_imm16(iter);
   goto do_prefix_pop_global;
  case ASM_POP_GLOBAL:
   imm = READ_imm8(iter);
do_prefix_pop_global:
   PRINT("mov    ");
   INVOKE(libdisasm_printglobal(printer,arg,imm,code,flags));
   goto do_print_comma_then_prefix;

  case ASM16_POP_LOCAL:
   imm = READ_imm16(iter);
   goto do_prefix_pop_local;
  case ASM_POP_LOCAL:
   imm = READ_imm8(iter);
do_prefix_pop_local:
   PRINT("mov    ");
   INVOKE(libdisasm_printlocal(printer,arg,imm,ddi,code,flags));
   goto do_print_comma_then_prefix;

  case ASM16_PUSH_MODULE:
   imm = READ_imm16(iter);
   goto do_prefix_push_module;
  case ASM_PUSH_MODULE:
   imm = READ_imm8(iter);
do_prefix_push_module:
   PRINT("mov    ");
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   PRINT(", ");
   INVOKE(libdisasm_printmodule(printer,arg,imm,code,flags));
   goto done;

  case ASM16_PUSH_REF:
   imm = READ_imm16(iter);
   goto do_prefix_push_ref;
  case ASM_PUSH_REF:
   imm = READ_imm8(iter);
do_prefix_push_ref:
   PRINT("mov    ");
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   PRINT(", ");
   INVOKE(libdisasm_printref(printer,arg,imm,code,flags));
   goto done;

  case ASM16_PUSH_ARG:
   imm = READ_imm16(iter);
   goto do_prefix_push_arg;
  case ASM_PUSH_ARG:
   imm = READ_imm8(iter);
do_prefix_push_arg:
   PRINT("mov    ");
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   PRINT(", ");
   INVOKE(libdisasm_printarg(printer,arg,imm,code,flags));
   goto done;

  case ASM16_PUSH_CONST:
   imm = READ_imm16(iter);
   goto do_prefix_push_const;
  case ASM_PUSH_CONST:
   imm = READ_imm8(iter);
do_prefix_push_const:
   PRINT("mov    ");
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   PRINT(", ");
   INVOKE(libdisasm_printconst(printer,arg,imm,code,flags,true));
   goto done;

  case ASM16_PUSH_STATIC:
   imm = READ_imm16(iter);
   goto do_prefix_push_static;
  case ASM_PUSH_STATIC:
   imm = READ_imm8(iter);
do_prefix_push_static:
   PRINT("mov    ");
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   PRINT(", ");
#if 1
   INVOKE(libdisasm_printstatic(printer,arg,imm,false,code,flags));
#else
   INVOKE(libdisasm_printstatic(printer,arg,imm,true,code,flags));
#endif
   goto done;

  case ASM16_PUSH_EXTERN:
   imm  = READ_imm16(iter);
   imm2 = READ_imm16(iter);
   goto do_prefix_push_extern;
  case ASM_PUSH_EXTERN:
   imm  = READ_imm8(iter);
   imm2 = READ_imm8(iter);
do_prefix_push_extern:
   PRINT("mov    ");
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   PRINT(", ");
   INVOKE(libdisasm_printextern(printer,arg,imm,imm2,code,flags));
   goto done;

  case ASM16_PUSH_GLOBAL:
   imm = READ_imm16(iter);
   goto do_prefix_push_global;
  case ASM_PUSH_GLOBAL:
   imm = READ_imm8(iter);
do_prefix_push_global:
   PRINT("mov    ");
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   PRINT(", ");
   INVOKE(libdisasm_printglobal(printer,arg,imm,code,flags));
   goto done;

  case ASM16_PUSH_LOCAL:
   imm = READ_imm16(iter);
   goto do_prefix_push_local;
  case ASM_PUSH_LOCAL:
   imm = READ_imm8(iter);
do_prefix_push_local:
   PRINT("mov    ");
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   PRINT(", ");
   INVOKE(libdisasm_printlocal(printer,arg,imm,ddi,code,flags));
   goto done;

  case ASM16_UNPACK:
   imm = READ_imm16(iter);
   goto do_prefix_unpack;
  case ASM_UNPACK:
   imm = READ_imm8(iter);
do_prefix_unpack:
   PRINT("unpack ");
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,true,ddi,code,flags));;
   printf(", #%u",imm);
   goto done;

  case ASM_ADD:
   mnemonic = "add    ";
do_prefix_mnemonic_pop:
   print(mnemonic,strlen(mnemonic));
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   PRINT(", pop");
   goto done;
  case ASM_SUB:
   mnemonic = "sub    ";
   goto do_prefix_mnemonic_pop;
  case ASM_MUL:
   mnemonic = "mul    ";
   goto do_prefix_mnemonic_pop;
  case ASM_DIV:
   mnemonic = "div    ";
   goto do_prefix_mnemonic_pop;
  case ASM_MOD:
   mnemonic = "mod    ";
   goto do_prefix_mnemonic_pop;
  case ASM_SHL:
   mnemonic = "shl    ";
   goto do_prefix_mnemonic_pop;
  case ASM_SHR:
   mnemonic = "shr    ";
   goto do_prefix_mnemonic_pop;
  case ASM_AND:
   mnemonic = "and    ";
   goto do_prefix_mnemonic_pop;
  case ASM_OR:
   mnemonic = "or     ";
   goto do_prefix_mnemonic_pop;
  case ASM_XOR:
   mnemonic = "xor    ";
   goto do_prefix_mnemonic_pop;
  case ASM_POW:
   mnemonic = "pow    ";
   goto do_prefix_mnemonic_pop;
  case ASM_INC:
   mnemonic = "inc    ";
   goto do_mnemonic_prefix;
  case ASM_DEC:
   mnemonic = "dec    ";
   goto do_mnemonic_prefix;
  case ASM_DECPOST:
   mnemonic = "push   dec ";
   goto do_mnemonic_prefix;
  case ASM_INCPOST:
   mnemonic = "push   inc ";
   goto do_mnemonic_prefix;

  case ASM_ADD_SIMM8:
   mnemonic = "add    ";
do_prefix_mnemonic_simm8:
   print(mnemonic,strlen(mnemonic));
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   printf(", $%d",(int)READ_Simm8(iter));
   goto done;

  case ASM_ADD_IMM32:
   mnemonic = "add    ";
do_prefix_mnemonic_imm32:
   print(mnemonic,strlen(mnemonic));
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   printf(", $%#I32x",READ_imm32(iter));
   goto done;

  case ASM_SUB_SIMM8:
   mnemonic = "sub    ";
   goto do_prefix_mnemonic_simm8;
  case ASM_SUB_IMM32:
   mnemonic = "sub    ";
   goto do_prefix_mnemonic_imm32;
  case ASM_MUL_SIMM8:
   mnemonic = "mul    ";
   goto do_prefix_mnemonic_simm8;
  case ASM_DIV_SIMM8:
   mnemonic = "div    ";
   goto do_prefix_mnemonic_simm8;
  case ASM_MOD_SIMM8:
   mnemonic = "mod    ";
   goto do_prefix_mnemonic_simm8;
  case ASM_SHL_IMM8:
   mnemonic = "shl    ";
do_prefix_mnemonic_imm8:
   print(mnemonic,strlen(mnemonic));
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   printf(", $%u",(unsigned int)READ_imm8(iter));
   goto done;
  case ASM_SHR_IMM8:
   mnemonic = "shr    ";
   goto do_prefix_mnemonic_imm8;
  case ASM_AND_IMM32:
   mnemonic = "and    ";
   goto do_prefix_mnemonic_imm32;
  case ASM_OR_IMM32:
   mnemonic = "or     ";
   goto do_prefix_mnemonic_imm32;
  case ASM_XOR_IMM32:
   mnemonic = "xor    ";
   goto do_prefix_mnemonic_imm32;

  case ASM_NOP:
   mnemonic = "nop    ";
   goto do_mnemonic_prefix_readonly;
  case ASM16_NOP:
   mnemonic = "nop16  ";
   goto do_mnemonic_prefix_readonly;

  case ASM_PUSH_EXCEPT:
   mnemonic = ", except";
do_mov_prefix_mnemonic:
   PRINT("mov    ");
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   print(mnemonic,strlen(mnemonic));
   goto done;
  case ASM_PUSH_THIS:
   mnemonic = ", this";
   goto do_mov_prefix_mnemonic;
  case ASM_PUSH_THIS_MODULE:
   mnemonic = ", this_module";
   goto do_mov_prefix_mnemonic;
  case ASM_PUSH_THIS_FUNCTION:
   mnemonic = ", this_function";
   goto do_mov_prefix_mnemonic;
  case ASM_PUSH_TRUE:
   mnemonic = ", true";
   goto do_mov_prefix_mnemonic;
  case ASM_PUSH_FALSE:
   mnemonic = ", false";
   goto do_mov_prefix_mnemonic;
  case ASM_PUSH_NONE:
   mnemonic = ", none";
   goto do_mov_prefix_mnemonic;

  case ASM_FUNCTION_C:
  case ASM_FUNCTION_C_16:
  case ASM16_FUNCTION_C:
  case ASM16_FUNCTION_C_16:
   mnemonic += MAX(MNEMONIC_MINWIDTH,4)+1; /* Skip `push ' */
   break;

  case ASM_OPERATOR:
  case ASM_OPERATOR_TUPLE:
  case ASM16_OPERATOR:
  case ASM16_OPERATOR_TUPLE:
   INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
   printf(": push op %s",mnemonic+COMPILER_STRLEN("op     top, "));
   goto do_instruction_specific;

  default: break;
  }

  /* fallback: print the default prefix. */
  INVOKE(libdisasm_printprefix(printer,arg,instr_start,false,ddi,code,flags));
  PRINT(": ");
 }

 /* Special opcodes taking stack arguments. */
 if (stacksz != (uint16_t)-1) {
  instruction_t *old_iter = iter.ptr;
  switch (opcode) {

  case ASM_DUP_N:
  case ASM_POP_N:
   imm = READ_imm8(iter)+2;
   goto print_duppop_stack;
  case ASM16_DUP_N:
  case ASM16_POP_N:
   imm = READ_imm16(iter)+2;
print_duppop_stack:
   print(opcode == ASM_DUP_N ||
         opcode == ASM16_DUP_N
         ? "dup    " : "pop    ",
         COMPILER_STRLEN("dup    "));
   INVOKE(libdisasm_printrelstack(printer,arg,stacksz,imm,ddi,code,flags));
   goto done;

  default: break;
  }
  iter.ptr = old_iter;
 }

 if (opcode == ASM_RET && code &&
     code->co_flags & CODE_FYIELDING)
     mnemonic = "yield  pop";
 print(mnemonic,strlen(mnemonic));
do_instruction_specific:

 /* Also print instruction operands. */
 switch (opcode) {

 case ASM_INC:
 case ASM_DEC:
 case ASM_INCPOST:
 case ASM_DECPOST:
  PRINT(" /* Illegal due to missing prefix */");
  break;

 case ASM32_JMP:
  jump_offset = READ_Simm32(iter);
  goto do_jmp_target;
 case ASM_JF16:
 case ASM_JT16:
 case ASM_JMP16:
 case ASM_FOREACH16:
  jump_offset = READ_Simm16(iter);
  goto do_jmp_target;
 case ASM_JF:
 case ASM_JT:
 case ASM_JMP:
 case ASM_FOREACH:
  jump_offset = READ_Simm8(iter);
do_jmp_target:
  /* TODO: Make use of disassembler label names. */
  if (code) {
   code_addr_t target_ip;
   target_ip = (code_addr_t)(iter.ptr - code->co_code) + jump_offset;
   printf("%.4I32X",target_ip);
   if (target_ip >= code->co_codebytes)
       PRINT(" /* invalid ip */");
  } else {
   printf("PC%+#I32X",jump_offset);
  }
  break;


 case ASM_CLASS_GC:
  imm  = READ_imm8(iter);
  imm2 = READ_imm8(iter);
  goto print_global_const;
 case ASM16_CLASS_GC:
  imm  = READ_imm16(iter);
  imm2 = READ_imm16(iter);
print_global_const:
  INVOKE(libdisasm_printglobal(printer,arg,imm,code,flags));
  PRINT(", ");
  INVOKE(libdisasm_printconst(printer,arg,imm2,code,flags,false));
  break;
 case ASM_CLASS_EC:
  imm  = READ_imm8(iter);
  imm2 = READ_imm8(iter);
  INVOKE(libdisasm_printextern(printer,arg,imm,imm2,code,flags));
  PRINT(", ");
  imm  = READ_imm8(iter);
  INVOKE(libdisasm_printconst(printer,arg,imm,code,flags,false));
  break;
 case ASM16_CLASS_EC:
  imm  = READ_imm16(iter);
  imm2 = READ_imm16(iter);
  INVOKE(libdisasm_printextern(printer,arg,imm,imm2,code,flags));
  PRINT(", ");
  imm  = READ_imm16(iter);
  INVOKE(libdisasm_printconst(printer,arg,imm,code,flags,false));
  break;

 case ASM_DUP_N:
 case ASM_POP_N:
  imm = READ_imm8(iter)+2;
  goto print_imm;
 case ASM16_DUP_N:
 case ASM16_POP_N:
  imm = READ_imm16(iter)+2;
  goto print_imm;
 case ASM_LROT:
 case ASM_RROT:
  imm = READ_imm8(iter)+3;
  goto print_imm;
 case ASM16_LROT:
 case ASM16_RROT:
  imm = READ_imm16(iter)+3;
  goto print_imm;
 case ASM_ENDCATCH_N:
 case ASM_ENDFINALLY_N:
  imm = READ_imm8(iter)+1;
  goto print_imm;
 case ASM_CALL:
 case ASM_EXTEND:
 case ASM_PACK_TUPLE:
 case ASM_PACK_LIST:
 case ASM_UNPACK:
 case ASM_SHL_IMM8:
 case ASM_SHR_IMM8:
 case ASM_CALLATTR:
 case ASM_PACK_HASHSET:
 case ASM_DELMEMBER:
 case ASM_DELMEMBER_THIS:
 case ASM_GETMEMBER:
 case ASM_GETMEMBER_THIS:
 case ASM_BOUNDMEMBER:
 case ASM_BOUNDMEMBER_THIS:
 case ASM_VARARGS_UNPACK:
 case ASM_VARARGS_GETITEM_I:
 case ASM_VARARGS_CMP_EQ_SZ:
 case ASM_VARARGS_CMP_GR_SZ:
  imm = READ_imm8(iter);
print_imm:
  printf("%I16u",imm);
  break;

 case ASM16_CALL_KW:
  imm  = READ_imm8(iter);
  imm2 = READ_imm16(iter);
  goto print_imm_const;
 case ASM_CALL_KW:
  imm  = READ_imm8(iter);
  imm2 = READ_imm8(iter);
print_imm_const:
  printf("%I16u, ",imm);
  INVOKE(libdisasm_printconst(printer,arg,imm2,code,flags,false));
  break;

 case ASM_CALLATTR_C_KW:
  imm = READ_imm8(iter);
  INVOKE(libdisasm_printconst(printer,arg,imm,code,flags,false));
  imm = READ_imm8(iter);
  printf(", #%I8u, ",imm);
  imm = READ_imm8(iter);
  INVOKE(libdisasm_printconst(printer,arg,imm,code,flags,false));
  break;

 case ASM16_CALLATTR_C_KW:
  imm = READ_imm16(iter);
  INVOKE(libdisasm_printconst(printer,arg,imm,code,flags,false));
  imm = READ_imm8(iter);
  printf(", #%I8u, ",imm);
  imm = READ_imm16(iter);
  INVOKE(libdisasm_printconst(printer,arg,imm,code,flags,false));
  break;

 case ASM16_CALLATTR_C_TUPLE_KW:
  imm  = READ_imm16(iter);
  imm2 = READ_imm16(iter);
  goto print_const_popdots_const;
 case ASM_CALLATTR_C_TUPLE_KW:
  imm  = READ_imm8(iter);
  imm2 = READ_imm8(iter);
print_const_popdots_const:
  INVOKE(libdisasm_printconst(printer,arg,imm,code,flags,false));
  PRINT(", pop..., ");
  INVOKE(libdisasm_printconst(printer,arg,imm2,code,flags,false));
  break;

 case ASM_PACK_DICT:
  imm = READ_imm8(iter);
  goto print_pack_dict;
 case ASM16_PACK_DICT:
  imm = READ_imm16(iter);
print_pack_dict:
  printf("%I32u",(uint32_t)imm*2);
  break;

 case ASM_CALL_SEQ:
  printf("%I8u]",READ_imm8(iter));
  break;
 case ASM_CALL_MAP:
  printf("%I16u}",(uint16_t)((uint16_t)READ_imm8(iter) * 2));
  break;

 case ASM16_PACK_TUPLE:
 case ASM16_PACK_LIST:
 case ASM16_UNPACK:
 case ASM16_PACK_HASHSET:
 case ASM_RANGE_0_I16:
 case ASM16_GETCMEMBER:
 case ASM16_GETMEMBER:
 case ASM16_GETMEMBER_THIS:
 case ASM16_DELMEMBER:
 case ASM16_DELMEMBER_THIS:
 case ASM16_BOUNDMEMBER:
 case ASM16_BOUNDMEMBER_THIS:
  imm = READ_imm16(iter);
  goto print_imm;

 case ASM_ADD_SIMM8:
 case ASM_SUB_SIMM8:
 case ASM_MUL_SIMM8:
 case ASM_DIV_SIMM8:
 case ASM_MOD_SIMM8:
  printf("%I8d",READ_Simm8(iter));
  break;

 {
  int16_t val;
 case ASM16_ADJSTACK:
  val = READ_Simm16(iter);
  goto do_print_adjstack;
 case ASM_ADJSTACK:
  val = READ_Simm8(iter);
do_print_adjstack:
  printf("%c %I32d",
         val < 0 ? '-' : '+',
         val < 0 ? (int32_t)-val : (int32_t)val);
 } break;

 {
  struct opinfo *info;
 case ASM16_OPERATOR:
  imm = READ_imm16(iter);
  goto do_print_operator;
 case ASM_OPERATOR:
  imm = READ_imm8(iter);
do_print_operator:
  info = Dee_OperatorInfo(NULL,(uint16_t)imm);
  if (info) {
   printf("__%s__, " PREFIX_STACKEFFECT "%I8u",
          info->oi_sname,READ_imm8(iter));
  } else {
   printf("%I16u, " PREFIX_STACKEFFECT "%I8u",
          imm,READ_imm8(iter));
  }
 } break;

 {
  struct opinfo *info;
 case ASM16_OPERATOR_TUPLE:
  imm = READ_imm16(iter);
  goto do_print_operator_tuple;
 case ASM_OPERATOR_TUPLE:
  imm = READ_imm8(iter);
do_print_operator_tuple:
  info = Dee_OperatorInfo(NULL,(uint16_t)imm);
  if (info) {
   printf("__%s__, pop",info->oi_sname);
  } else {
   printf("%I16u, pop",imm);
  }
 } break;

 case ASM_DEFCMEMBER:
 case ASM_SETMEMBER:
 case ASM_SETMEMBER_THIS:
 case ASM_CALLATTR_KWDS:
  imm = READ_imm8(iter);
  goto print_imm_pop;
 case ASM16_DEFCMEMBER:
 case ASM16_SETMEMBER:
 case ASM16_SETMEMBER_THIS:
  imm = READ_imm16(iter);
print_imm_pop:
  printf("%I16u, pop",imm);
  break;

 case ASM_RANGE_0_I32:
  printf("%I32u",READ_imm32(iter));
  break;

 case ASM_SETITEM_I:
 case ASM_GETRANGE_IP:
 case ASM_SETRANGE_PI:
 case ASM_SETRANGE_NI:
  printf("%I16d, pop",READ_Simm16(iter));
  break;

 case ASM_GETRANGE_PI:
 case ASM_GETRANGE_NI:
 case ASM_GETITEM_I:
  printf("%I16d",READ_Simm16(iter));
  break;

 case ASM_GETRANGE_IN:
  printf("%I16d, none",READ_Simm16(iter));
  break;

 case ASM_SETRANGE_IP:
  printf("%I16d, pop, pop",READ_Simm16(iter));
  break;

 case ASM_SETRANGE_IN:
  printf("%I16d, none, pop",READ_Simm16(iter));
  break;

 case ASM_GETRANGE_II:
 case ASM_SETRANGE_II:
  imm = (uint32_t)(uint16_t)READ_Simm16(iter);
  printf("%I16d, " PREFIX_INTEGERAL "%I16d",
        (int16_t)(uint16_t)imm,READ_Simm16(iter));
  if (opcode == ASM_SETRANGE_II) PRINT(", pop");
  break;

 case ASM_ADD_IMM32:
 case ASM_SUB_IMM32:
 case ASM_AND_IMM32:
 case ASM_OR_IMM32:
 case ASM_XOR_IMM32:
  printf("%#I32x",READ_imm32(iter));
  break;


 {
  uint16_t imm2;
 case ASM16_PUSH_BND_EXTERN:
 case ASM16_POP_EXTERN:
 case ASM16_PUSH_EXTERN:
 case ASM16_CALL_EXTERN:
  imm  = READ_imm16(iter);
  imm2 = READ_imm16(iter);
  goto print_extern;
 case ASM_PUSH_BND_EXTERN:
 case ASM_POP_EXTERN:
 case ASM_PUSH_EXTERN:
 case ASM_CALL_EXTERN:
  imm  = READ_imm8(iter);
  imm2 = READ_imm8(iter);
print_extern:
  INVOKE(libdisasm_printextern(printer,arg,imm,imm2,code,flags));
  if (opcode == ASM_CALL_EXTERN || opcode == ASM16_CALL_EXTERN)
      printf(", " PREFIX_STACKEFFECT "%I8u",READ_imm8(iter));
 } break;

 case ASM16_PUSH_MODULE:
  imm = READ_imm16(iter);
  goto print_module;
 case ASM_PUSH_MODULE:
  imm = READ_imm8(iter);
print_module:
  INVOKE(libdisasm_printmodule(printer,arg,imm,code,flags));
  break;

 case ASM16_PUSH_BND_GLOBAL:
 case ASM16_DEL_GLOBAL:
 case ASM16_POP_GLOBAL:
 case ASM16_PUSH_GLOBAL:
 case ASM16_CALL_GLOBAL:
  imm = READ_imm16(iter);
  goto print_global;
 case ASM_PUSH_BND_GLOBAL:
 case ASM_DEL_GLOBAL:
 case ASM_POP_GLOBAL:
 case ASM_PUSH_GLOBAL:
 case ASM_CALL_GLOBAL:
  imm = READ_imm8(iter);
print_global:
  INVOKE(libdisasm_printglobal(printer,arg,imm,code,flags));
  if (opcode == ASM_CALL_GLOBAL || opcode == ASM16_CALL_GLOBAL)
      printf(", " PREFIX_STACKEFFECT "%I8u",READ_imm8(iter));
  break;

 case ASM16_PUSH_BND_LOCAL:
 case ASM16_DEL_LOCAL:
 case ASM16_POP_LOCAL:
 case ASM16_PUSH_LOCAL:
 case ASM16_CALL_LOCAL:
  imm = READ_imm16(iter);
  goto print_local;
 case ASM_PUSH_BND_LOCAL:
 case ASM_DEL_LOCAL:
 case ASM_POP_LOCAL:
 case ASM_PUSH_LOCAL:
 case ASM_CALL_LOCAL:
  imm = READ_imm8(iter);
print_local:
  INVOKE(libdisasm_printlocal(printer,arg,imm,ddi,code,flags));
  if (opcode == ASM_CALL_LOCAL || opcode == ASM16_CALL_LOCAL)
      printf(", " PREFIX_STACKEFFECT "%I8u",READ_imm8(iter));
  break;

 case ASM16_PUSH_REF:
 case ASM16_SUPER_THIS_R:
 case ASM16_GETMEMBER_THIS_R:
 case ASM16_BOUNDMEMBER_THIS_R:
 case ASM16_DELMEMBER_THIS_R:
 case ASM16_SETMEMBER_THIS_R:
 case ASM16_GETCMEMBER_R:
 case ASM16_CALLCMEMBER_THIS_R:
  imm = READ_imm16(iter);
  goto print_ref;
 case ASM_PUSH_REF:
 case ASM_SUPER_THIS_R:
 case ASM_GETMEMBER_THIS_R:
 case ASM_BOUNDMEMBER_THIS_R:
 case ASM_DELMEMBER_THIS_R:
 case ASM_SETMEMBER_THIS_R:
 case ASM_GETCMEMBER_R:
 case ASM_CALLCMEMBER_THIS_R:
  imm = READ_imm8(iter);
print_ref:
  INVOKE(libdisasm_printref(printer,arg,imm,code,flags));
  switch (opcode) {
  case ASM_GETMEMBER_THIS_R:
  case ASM_BOUNDMEMBER_THIS_R:
  case ASM_DELMEMBER_THIS_R:
  case ASM_SETMEMBER_THIS_R:
  case ASM_GETCMEMBER_R:
  case ASM_CALLCMEMBER_THIS_R:
   printf(", " PREFIX_INTEGERAL "%I8u",READ_imm8(iter));
   if (opcode == ASM_SETMEMBER_THIS_R) PRINT(", pop");
   if (opcode == ASM_CALLCMEMBER_THIS_R)
       printf(", " PREFIX_STACKEFFECT "%I8u",READ_imm8(iter));
   break;
  case ASM16_GETMEMBER_THIS_R:
  case ASM16_BOUNDMEMBER_THIS_R:
  case ASM16_DELMEMBER_THIS_R:
  case ASM16_SETMEMBER_THIS_R:
  case ASM16_GETCMEMBER_R:
  case ASM16_CALLCMEMBER_THIS_R:
   printf(", " PREFIX_INTEGERAL "%I16u",READ_imm16(iter));
   if (opcode == ASM16_SETMEMBER_THIS_R) PRINT(", pop");
   if (opcode == ASM16_CALLCMEMBER_THIS_R)
       printf(", " PREFIX_STACKEFFECT "%I8u",READ_imm8(iter));
   break;
  default: break;
  }
  break;

 case ASM16_PUSH_ARG:
  imm = READ_imm16(iter);
  goto print_arg;
 case ASM_PUSH_ARG:
 case ASM_CALL_ARGSFWD:
do_read_and_print_arg:
  imm = READ_imm8(iter);
print_arg:
  INVOKE(libdisasm_printarg(printer,arg,imm,code,flags));
  if (opcode == ASM_CALL_ARGSFWD) {
   /* Print the second argument index. */
   PRINT(", ");
   opcode = 0;
   goto do_read_and_print_arg;
  }
  break;

 case ASM16_POP_STATIC:
 case ASM16_PUSH_STATIC:
  imm = READ_imm16(iter);
  goto print_static;
 case ASM_POP_STATIC:
 case ASM_PUSH_STATIC:
  imm = READ_imm8(iter);
print_static:
#if 1
  INVOKE(libdisasm_printstatic(printer,arg,imm,false,code,flags));
#else
  INVOKE(libdisasm_printstatic(printer,arg,imm,
                               opcode == ASM_PUSH_STATIC ||
                               opcode == ASM16_PUSH_STATIC,
                               code,flags));
#endif
  break;


 case ASM16_PUSH_CONST:
  imm = READ_imm16(iter);
  goto print_const_int;
 case ASM_PUSH_CONST:
  imm = READ_imm8(iter);
print_const_int:
  INVOKE(libdisasm_printconst(printer,arg,imm,code,flags,true));
  break;

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
 case ASM16_CALLATTR_C_TUPLE:
 case ASM16_CALLATTR_THIS_C_TUPLE:
 case ASM16_SETATTR_C:
 case ASM16_SETATTR_THIS_C:
 case ASM16_CALLATTR_C:
 case ASM16_CALLATTR_C_SEQ:
 case ASM16_CALLATTR_C_MAP:
 case ASM16_CALLATTR_THIS_C:
 case ASM16_FUNCTION_C:
 case ASM16_FUNCTION_C_16:
 case ASM16_CALL_TUPLE_KW:
 case ASM16_CLASS_C:
  imm = READ_imm16(iter);
  goto print_const;
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
 case ASM_CALLATTR_C_TUPLE:
 case ASM_CALLATTR_THIS_C_TUPLE:
 case ASM_SETATTR_C:
 case ASM_SETATTR_THIS_C:
 case ASM_CALLATTR_C:
 case ASM_CALLATTR_C_SEQ:
 case ASM_CALLATTR_C_MAP:
 case ASM_CALLATTR_THIS_C:
 case ASM_FUNCTION_C:
 case ASM_FUNCTION_C_16:
 case ASM_CALL_TUPLE_KW:
 case ASM_CLASS_C:
  imm = READ_imm8(iter);
print_const:
  INVOKE(libdisasm_printconst(printer,arg,imm,code,flags,false));
  switch (opcode) {
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
  case ASM_CALLATTR_C_TUPLE:
  case ASM16_CALLATTR_C_TUPLE:
  case ASM_CALLATTR_THIS_C_TUPLE:
  case ASM16_CALLATTR_THIS_C_TUPLE:
  case ASM_SETATTR_C:
  case ASM16_SETATTR_C:
  case ASM_SETATTR_THIS_C:
  case ASM16_SETATTR_THIS_C:
   PRINT(", pop");
   break;
  case ASM_CALLATTR_C:
  case ASM16_CALLATTR_C:
  case ASM_CALLATTR_THIS_C:
  case ASM16_CALLATTR_THIS_C:
   printf(", " PREFIX_STACKEFFECT "%I8u",READ_imm8(iter));
   break;
  case ASM_CALLATTR_C_SEQ:
  case ASM16_CALLATTR_C_SEQ:
   printf(", [" PREFIX_STACKEFFECT "%I8u]",READ_imm8(iter));
   break;
  case ASM_CALLATTR_C_MAP:
  case ASM16_CALLATTR_C_MAP:
   printf(", {" PREFIX_STACKEFFECT "%I16u}",(uint16_t)((uint16_t)READ_imm8(iter) * 2));
   break;
  case ASM_FUNCTION_C_16:
  case ASM16_FUNCTION_C_16:
   imm = READ_imm16(iter);
   goto do_print_stackeffect_after_const;
  case ASM_FUNCTION_C:
  case ASM16_FUNCTION_C:
   imm = READ_imm8(iter);
do_print_stackeffect_after_const:
   printf(", " PREFIX_STACKEFFECT "%I32u",(uint32_t)imm+1);
   break;
  default: break;
  }
  break;

 case ASM16_SUPERGETATTR_THIS_RC:
 case ASM16_SUPERCALLATTR_THIS_RC:
  imm  = READ_imm16(iter);
  imm2 = READ_imm16(iter);
  goto do_print_superattr_rc;
 case ASM_SUPERGETATTR_THIS_RC:
 case ASM_SUPERCALLATTR_THIS_RC:
  imm  = READ_imm8(iter);
  imm2 = READ_imm8(iter);
do_print_superattr_rc:
  INVOKE(libdisasm_printref(printer,arg,imm,code,flags));
  PRINT(", ");
  INVOKE(libdisasm_printconst(printer,arg,imm2,code,flags,false));
  if (opcode == ASM_SUPERCALLATTR_THIS_RC ||
      opcode == ASM16_SUPERCALLATTR_THIS_RC)
      printf(", #%I8u",READ_imm8(iter));
  break;

 default: break;
 }

done:
 return result;
err:
 return temp;
}




DECL_END

#endif /* !GUARD_DEX_FS_PRINTINSTR_C */
