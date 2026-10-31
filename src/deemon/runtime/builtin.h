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
#ifndef GUARD_DEEMON_RUNTIME_BUILTIN_H
#define GUARD_DEEMON_RUNTIME_BUILTIN_H 1

#include <deemon/api.h>

#define BUILTIN(name, object, flags)
#include "builtins.def"

DECL_BEGIN

/* Define a second time here since the deemon script below needs it.
 * We internally don't wrap with `#ifndef` since the C standard says
 * that a macro can be re-defined with a duplicate definition without
 * causing a compiler warning.
 *
 * So if someone ends up changing the definition in headers, they'll
 * get a compiler warning here (which they have to fix by adjusting
 * this macro here, then running `deemon -F builtin.c`) */
#define Dee_MODULE_HASHNX(hs, perturb) \
	(void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5` is tunable. */



enum {
	/* Global object ids for builtin objects, as exported by the `deemon` module. */
#define BUILTIN(name, object, flags)    id_##name,
#define BUILTIN_ALIAS(name, alt, flags) /* nothing */
#include "builtins.def"
	num_builtins_obj
};

INTDEF struct Dee_module_symbol builtin_deemon_symbols[];

/*[[[deemon
import * from deemon;
import rtHash = rt.hash;

@@List of (name, flags, doc) tuples
local builtins: {(string, string, string | none)...} = [];
{
#define BUILTIN(name, object, flags)          BUILTIN_DOC(name, object, flags, none)
#define BUILTIN_DOC(name, object, flags, doc) builtins.append((#name, (#flags).rereplace("[ \t\n]+", " "), doc));
#define BUILTIN_RUNTIME_DEFINED
#define BUILTIN_NO_INCLUDES
#include "builtins.def"
}
#undef NUM_BUILTINS_SYM
#undef NUM_BUILTINS_SPC
#undef BUILTINS_HASHMASK
#define void none

local NUM_BUILTINS_SYM  = #builtins;
local NUM_BUILTINS_SPC  = NUM_BUILTINS_SYM + (NUM_BUILTINS_SYM / 3);
local BUILTINS_HASHMASK = (1 << (NUM_BUILTINS_SPC - 1).fls) - 1;

// Construct the hash-table of builtin deemon objects.
local hashValues32 = Dict();
local hashValues64 = Dict();
for (local hashValues, hashof, ppCond, uintNN_C: {
	(hashValues32, rtHash.hash32, "_Dee_HashSelect(32, 64) == 32", 32),
	(hashValues64, rtHash.hash64, "_Dee_HashSelect(32, 64) == 64", 64)
}) {
	local hashtab: List with (string, string, string | none) = List([none] * (BUILTINS_HASHMASK + 1));
	for (local name, flags, doc: builtins) {
		local hash = hashof(name), i, perturb;
		perturb = i = hash & BUILTINS_HASHMASK;
		for (;; Dee_MODULE_HASHNX(i, perturb)) {
			local index = i & BUILTINS_HASHMASK;
			if (hashtab[index] !is none)
				continue;
			hashtab[index] = (name, flags, doc);
			break;
		}
	}
	for (local i, item: hashtab.enumerate()) {
		local name, flags, doc = item...;
		if (name !is none)
			hashValues[name] = i;
	}
}
print("STATIC_ASSERT_MSG(num_builtins_obj == ", NUM_BUILTINS_SYM, ", \"You need to re-run `deemon -F src/deemon/runtime/builtin.h`\");");
print("INTDEF struct Dee_module_symbol deemon_symbols[", BUILTINS_HASHMASK + 1, "];");
local longestNameLen = hashValues32.keys.each.length > ...;
for (local name, hashIndex32: hashValues32) {
	print("#define deemon_modsym_", name.ljust(longestNameLen),
		" deemon_symbols[_Dee_HashSelect(",
			hashIndex32, ", ", hashValues64[name], ")]");
}
]]]*/
STATIC_ASSERT_MSG(num_builtins_obj == 52, "You need to re-run `deemon -F src/deemon/runtime/builtin.h`");
INTDEF struct Dee_module_symbol deemon_symbols[128];
#define deemon_modsym_none           deemon_symbols[_Dee_HashSelect(0, 32)]
#define deemon_modsym_Iterator       deemon_symbols[_Dee_HashSelect(3, 127)]
#define deemon_modsym_hasattr        deemon_symbols[_Dee_HashSelect(4, 66)]
#define deemon_modsym_Signal         deemon_symbols[_Dee_HashSelect(6, 27)]
#define deemon_modsym_Object         deemon_symbols[_Dee_HashSelect(7, 1)]
#define deemon_modsym___roloc        deemon_symbols[_Dee_HashSelect(8, 44)]
#define deemon_modsym_WeakRef        deemon_symbols[_Dee_HashSelect(11, 73)]
#define deemon_modsym_Property       deemon_symbols[_Dee_HashSelect(17, 85)]
#define deemon_modsym_Sequence       deemon_symbols[_Dee_HashSelect(20, 99)]
#define deemon_modsym_boundattr      deemon_symbols[_Dee_HashSelect(21, 116)]
#define deemon_modsym_bounditem      deemon_symbols[_Dee_HashSelect(27, 37)]
#define deemon_modsym_Super          deemon_symbols[_Dee_HashSelect(30, 95)]
#define deemon_modsym_Dict           deemon_symbols[_Dee_HashSelect(33, 118)]
#define deemon_modsym_List           deemon_symbols[_Dee_HashSelect(34, 53)]
#define deemon_modsym_enumattr       deemon_symbols[_Dee_HashSelect(37, 55)]
#define deemon_modsym___import__     deemon_symbols[_Dee_HashSelect(38, 68)]
#define deemon_modsym_exec           deemon_symbols[_Dee_HashSelect(40, 3)]
#define deemon_modsym_string         deemon_symbols[_Dee_HashSelect(43, 2)]
#define deemon_modsym___assert       deemon_symbols[_Dee_HashSelect(45, 36)]
#define deemon_modsym_File           deemon_symbols[_Dee_HashSelect(47, 81)]
#define deemon_modsym_Attribute      deemon_symbols[_Dee_HashSelect(49, 115)]
#define deemon_modsym_HashSet        deemon_symbols[_Dee_HashSelect(53, 71)]
#define deemon_modsym_WeakRefAble    deemon_symbols[_Dee_HashSelect(54, 15)]
#define deemon_modsym___gaosa        deemon_symbols[_Dee_HashSelect(55, 69)]
#define deemon_modsym___grosr        deemon_symbols[_Dee_HashSelect(57, 86)]
#define deemon_modsym_Traceback      deemon_symbols[_Dee_HashSelect(61, 43)]
#define deemon_modsym_compare        deemon_symbols[_Dee_HashSelect(64, 89)]
#define deemon_modsym_Error          deemon_symbols[_Dee_HashSelect(65, 91)]
#define deemon_modsym_Callable       deemon_symbols[_Dee_HashSelect(67, 13)]
#define deemon_modsym_equals         deemon_symbols[_Dee_HashSelect(69, 16)]
#define deemon_modsym___pooad        deemon_symbols[_Dee_HashSelect(73, 65)]
#define deemon_modsym_bool           deemon_symbols[_Dee_HashSelect(74, 14)]
#define deemon_modsym_gc             deemon_symbols[_Dee_HashSelect(76, 78)]
#define deemon_modsym_Set            deemon_symbols[_Dee_HashSelect(80, 60)]
#define deemon_modsym_Frame          deemon_symbols[_Dee_HashSelect(84, 38)]
#define deemon_modsym_Mapping        deemon_symbols[_Dee_HashSelect(86, 26)]
#define deemon_modsym_Bytes          deemon_symbols[_Dee_HashSelect(94, 98)]
#define deemon_modsym_int            deemon_symbols[_Dee_HashSelect(95, 112)]
#define deemon_modsym_Function       deemon_symbols[_Dee_HashSelect(100, 90)]
#define deemon_modsym_Type           deemon_symbols[_Dee_HashSelect(101, 123)]
#define deemon_modsym_Numeric        deemon_symbols[_Dee_HashSelect(103, 79)]
#define deemon_modsym_float          deemon_symbols[_Dee_HashSelect(104, 100)]
#define deemon_modsym_InstanceMethod deemon_symbols[_Dee_HashSelect(106, 83)]
#define deemon_modsym_Thread         deemon_symbols[_Dee_HashSelect(107, 93)]
#define deemon_modsym___badcall      deemon_symbols[_Dee_HashSelect(108, 114)]
#define deemon_modsym_Tuple          deemon_symbols[_Dee_HashSelect(111, 33)]
#define deemon_modsym___neosb        deemon_symbols[_Dee_HashSelect(112, 101)]
#define deemon_modsym___giosi        deemon_symbols[_Dee_HashSelect(113, 17)]
#define deemon_modsym_Module         deemon_symbols[_Dee_HashSelect(116, 70)]
#define deemon_modsym_Cell           deemon_symbols[_Dee_HashSelect(117, 12)]
#define deemon_modsym_hash           deemon_symbols[_Dee_HashSelect(123, 103)]
#define deemon_modsym_hasitem        deemon_symbols[_Dee_HashSelect(126, 121)]
/*[[[end]]]*/

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_BUILTIN_H */
