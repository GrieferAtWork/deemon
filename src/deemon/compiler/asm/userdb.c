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
#ifndef GUARD_DEEMON_COMPILER_ASM_USERDB_C
#define GUARD_DEEMON_COMPILER_ASM_USERDB_C 1

#include <deemon/api.h>

#include <deemon/compiler/assembler.h> /* ASM_MAX_INSTRUCTION_OPERANDS, ASM_MNEMONIC_MAXNAME */

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

#ifndef CONFIG_LANGUAGE_NO_ASM
#if !defined(__INTELLISENSE__)
DECL_BEGIN

#define PRIVATE_EXPAND_OVERLOADS(...)   __VA_ARGS__
#if ASM_MAX_INSTRUCTION_OPERANDS == 4
#define PRIVATE_UNPACK_ARGS_0         { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
#define PRIVATE_UNPACK_ARGS_1(...)    { __VA_ARGS__ }, { 0, 0 }, { 0, 0 }, { 0, 0 }
#define PRIVATE_UNPACK_ARGS_21(...)   { __VA_ARGS__ }, { 0, 0 }, { 0, 0 }
#define PRIVATE_UNPACK_ARGS_2(...)    { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_21
#define PRIVATE_UNPACK_ARGS_31(...)   { __VA_ARGS__ }, { 0, 0 }
#define PRIVATE_UNPACK_ARGS_32(...)   { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_31
#define PRIVATE_UNPACK_ARGS_3(...)    { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_32
#define PRIVATE_UNPACK_ARGS_41(...)   { __VA_ARGS__ }
#define PRIVATE_UNPACK_ARGS_42(...)   { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_41
#define PRIVATE_UNPACK_ARGS_43(...)   { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_42
#define PRIVATE_UNPACK_ARGS_4(...)    { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_43
#elif ASM_MAX_INSTRUCTION_OPERANDS == 5
#define PRIVATE_UNPACK_ARGS_0         { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
#define PRIVATE_UNPACK_ARGS_1(...)    { __VA_ARGS__ }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
#define PRIVATE_UNPACK_ARGS_21(...)   { __VA_ARGS__ }, { 0, 0 }, { 0, 0 }, { 0, 0 }
#define PRIVATE_UNPACK_ARGS_2(...)    { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_21
#define PRIVATE_UNPACK_ARGS_31(...)   { __VA_ARGS__ }, { 0, 0 }, { 0, 0 }
#define PRIVATE_UNPACK_ARGS_32(...)   { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_31
#define PRIVATE_UNPACK_ARGS_3(...)    { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_32
#define PRIVATE_UNPACK_ARGS_41(...)   { __VA_ARGS__ }, { 0, 0 }
#define PRIVATE_UNPACK_ARGS_42(...)   { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_41
#define PRIVATE_UNPACK_ARGS_43(...)   { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_42
#define PRIVATE_UNPACK_ARGS_4(...)    { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_43
#define PRIVATE_UNPACK_ARGS_51(...)   { __VA_ARGS__ }
#define PRIVATE_UNPACK_ARGS_52(...)   { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_51
#define PRIVATE_UNPACK_ARGS_53(...)   { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_52
#define PRIVATE_UNPACK_ARGS_54(...)   { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_53
#define PRIVATE_UNPACK_ARGS_5(...)    { __VA_ARGS__ }, PRIVATE_UNPACK_ARGS_54
#else /* ASM_MAX_INSTRUCTION_OPERANDS == ... */
#error "Unsupported 'ASM_MAX_INSTRUCTION_OPERANDS'"
#endif /* ASM_MAX_INSTRUCTION_OPERANDS != ... */
#define PRIVATE_UNPACK_ARGS(n, args) PRIVATE_UNPACK_ARGS_##n args

#define MNEMONIC_SIZE_IDENTIFIER   PP_CAT2(mnemonicsize_, __LINE__)
#define MNEMONIC_STRUCT_IDENTIFIER PP_CAT2(mnemonicstct_, __LINE__)

/* Figure out the number of overloads for each instruction. */
enum {
#define INSTR(name, overloads) \
	MNEMONIC_SIZE_IDENTIFIER = 0 PRIVATE_EXPAND_OVERLOADS overloads,
#define OVERLOAD(instr_id, flags, n_args, args) +1
#include "userdb.def"
	_trailing_unused_01
};


#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(push, 1)
#endif /* __COMPILER_HAVE_PRAGMA_PACK */
struct ATTR_PACKED mnemonic_db {
#define INSTR(name, overloads)                           \
	struct {                                             \
		char                n[ASM_MNEMONIC_MAXNAME];     \
		uint16_t            c;                           \
		struct asm_overload o[MNEMONIC_SIZE_IDENTIFIER]; \
	} MNEMONIC_STRUCT_IDENTIFIER;
#include "userdb.def"
	char     sentinel_name[ASM_MNEMONIC_MAXNAME];
	uint16_t sentinel_zero;
};


INTDEF struct mnemonic_db const asm_mnemonics;
INTERN struct mnemonic_db const asm_mnemonics = {
#define INSTR(name, overloads) \
	{ name, MNEMONIC_SIZE_IDENTIFIER, { PRIVATE_EXPAND_OVERLOADS overloads } },
#if (ASM_MAX_INSTRUCTION_OPERANDS % 2) == 0
#define OVERLOAD(instr_id, flags, n_args, args) \
	{ instr_id, flags, n_args, { PRIVATE_UNPACK_ARGS(n_args, args) }, 0 },
#else /* (ASM_MAX_INSTRUCTION_OPERANDS % 2) == 0 */
#define OVERLOAD(instr_id, flags, n_args, args) \
	{ instr_id, flags, n_args, { PRIVATE_UNPACK_ARGS(n_args, args) } },
#endif /* (ASM_MAX_INSTRUCTION_OPERANDS % 2) != 0 */
#include "userdb.def"
	/* Sentinel. */
	{ 0, }, 0
};
INTDEF size_t const asm_mnemonics_size;
INTDEF size_t const asm_mnemonics_count;

INTERN size_t const asm_mnemonics_size  = offsetof(struct mnemonic_db, sentinel_name);
INTERN size_t const asm_mnemonics_count = 0
#define INSTR(name, overloads) +1
#include "userdb.def"
;
#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif /* __COMPILER_HAVE_PRAGMA_PACK */

DECL_END
#endif
#endif /* !CONFIG_LANGUAGE_NO_ASM */

#endif /* !GUARD_DEEMON_COMPILER_ASM_USERDB_C */
