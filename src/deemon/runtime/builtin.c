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
#ifndef GUARD_DEEMON_RUNTIME_BUILTIN_C
#define GUARD_DEEMON_RUNTIME_BUILTIN_C 1

#include <deemon/api.h>

#include <deemon/gc.h>
#include <deemon/dex.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
/**/

#include "builtin.h"
#include "strings.h"

#include <stdint.h> /* UINT32_C */

DECL_BEGIN

enum {
#define BUILTIN(name, object, flags) _id_##name,
#define BUILTIN_NO_INCLUDES
#include "builtins.def"
#undef BUILTIN_NO_INCLUDES
	_NUM_BUILTINS_SYM
};

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
PRIVATE DREF DeeObject *builtin_object_vector[num_builtins_obj] = {
#define BUILTIN(name, object, flags) (DeeObject *)object,
#define BUILTIN_NO_INCLUDES
#include "builtins.def"
#undef BUILTIN_NO_INCLUDES
};
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

/* Define a second time here since the deemon script below needs it.
 * We internally don't wrap with `#ifndef' since the C standard says
 * that a macro can be re-defined with a duplicate definition without
 * causing a compiler warning.
 *
 * So if someone ends up changing the definition in headers, they'll
 * get a compiler warning here (which they have to fix by adjusting
 * this macro here, then running `deemon -F builtin.c') */
#define Dee_MODULE_HASHNX(hs, perturb) \
	(void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */


#ifdef CONFIG_NO_DOC
#define DOCOF(x) NULL
#else /* CONFIG_NO_DOC */
#define BUILTIN(name, object, flags) /* nothing */
#define BUILTIN_DOC(name, object, flags, doc) \
	PRIVATE char const DOCOF_##name[] = doc;
#include "builtins.def"
#define DOCOF(x) DOCOF_##x
#endif /* !CONFIG_NO_DOC */


#define MODSYM(name, doc, name_hash, flags, index) \
	{ name, doc, name_hash, flags, 0, index }

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
print("#define NUM_BUILTINS_SYM  ", NUM_BUILTINS_SYM);
print("#define NUM_BUILTINS_SPC  ", NUM_BUILTINS_SPC);
print("#define BUILTINS_HASHMASK ", BUILTINS_HASHMASK.hex());
print("STATIC_ASSERT_MSG(_NUM_BUILTINS_SYM == ", NUM_BUILTINS_SYM, ", \"You need to re-run `deemon -F src/deemon/runtime/builtin.c'\");");
print("PRIVATE struct module_symbol deemon_symbols[", BUILTINS_HASHMASK + 1, "] = {");

// Construct the hash-table of builtin deemon objects.
local isFirst = true;
for (local hashof, ppCond, uintNN_C: {
	(rtHash.hash32, "_Dee_HashSelect(32, 64) == 32", 32),
	(rtHash.hash64, "_Dee_HashSelect(32, 64) == 64", 64)
}) {
	local hashtab: List with (string, string, string | none) = [none] * (BUILTINS_HASHMASK + 1);
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
	print(isFirst ? "#if " : "#elif ", ppCond);
	isFirst = false;
	for (local name, flags, doc: hashtab) {
		print("	MODSYM("),;
		if (name is none) {
			print("NULL, "),;
		} else {
			print("DeeString_STR(&str_", name, "), "),;
		}
		if (doc is none) {
			print("NULL, "),;
		} else {
			print("DOCOF(", name, "), "),;
		}
		if (name is none) {
			print("0, "),;
		} else {
			print("UINT", uintNN_C, "_C(", hashof(name).hex(), "), "),;
		}
		if (flags is none) {
			print(name is none ? "0, " : "MODSYM_FNAMEOBJ, "),;
		} else {
			print(flags, " | MODSYM_FNAMEOBJ, "),;
		}
		if (name is none) {
			print("0"),;
		} else {
			print("id_", name),;
		}
		print("),");
	}
}
print("#elif !defined(__DEEMON__) /" "* ... *" "/");
print("#error \"Unsupported hash configuration\"");
print("#endif /" "* !... *" "/");
print("};");
]]]*/
#define NUM_BUILTINS_SYM  52
#define NUM_BUILTINS_SPC  69
#define BUILTINS_HASHMASK 0x7f
STATIC_ASSERT_MSG(_NUM_BUILTINS_SYM == 52, "You need to re-run `deemon -F src/deemon/runtime/builtin.c'");
PRIVATE struct module_symbol deemon_symbols[128] = {
#if _Dee_HashSelect(32, 64) == 32
	MODSYM(DeeString_STR(&str_none), NULL, UINT32_C(0xde6dda00), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_none),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Iterator), NULL, UINT32_C(0xfce46883), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Iterator),
	MODSYM(DeeString_STR(&str_hasattr), DOCOF(hasattr), UINT32_C(0xa37d5291), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_hasattr),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Signal), NULL, UINT32_C(0x9b300d86), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Signal),
	MODSYM(DeeString_STR(&str_Object), NULL, UINT32_C(0xfa8141c1), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Object),
	MODSYM(DeeString_STR(&str___roloc), NULL, UINT32_C(0xf4874267), MODSYM_FREADONLY | MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___roloc),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_WeakRef), NULL, UINT32_C(0x1968c00b), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_WeakRef),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Property), NULL, UINT32_C(0xd4f3688b), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Property),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Sequence), NULL, UINT32_C(0xe5937b14), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Sequence),
	MODSYM(DeeString_STR(&str_boundattr), DOCOF(boundattr), UINT32_C(0xbe67bf95), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_boundattr),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_bounditem), DOCOF(bounditem), UINT32_C(0xbe8d5e6f), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_bounditem),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Super), NULL, UINT32_C(0xa0d5169e), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Super),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Dict), NULL, UINT32_C(0xb2f1a21), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Dict),
	MODSYM(DeeString_STR(&str_List), NULL, UINT32_C(0xab47f22), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_List),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_enumattr), NULL, UINT32_C(0x767e1f86), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_enumattr),
	MODSYM(DeeString_STR(&str___import__), DOCOF(__import__), UINT32_C(0x5ace85a6), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id___import__),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_exec), DOCOF(exec), UINT32_C(0x6b42be28), MODSYM_FNORMAL | MODSYM_FNAMEOBJ, id_exec),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_string), NULL, UINT32_C(0xad217aab), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_string),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str___assert), NULL, UINT32_C(0xd4715fbd), MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___assert),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_File), NULL, UINT32_C(0x5209cbaf), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_File),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Attribute), NULL, UINT32_C(0xa08b731), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Attribute),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_HashSet), NULL, UINT32_C(0x19f6e15e), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_HashSet),
	MODSYM(DeeString_STR(&str_WeakRefAble), NULL, UINT32_C(0x3b8e11b6), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_WeakRefAble),
	MODSYM(DeeString_STR(&str___gaosa), NULL, UINT32_C(0xbdd31637), MODSYM_FREADONLY | MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___gaosa),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str___grosr), NULL, UINT32_C(0xee715568), MODSYM_FREADONLY | MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___grosr),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Traceback), NULL, UINT32_C(0x65f6383d), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Traceback),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_compare), DOCOF(compare), UINT32_C(0x84b4e5c0), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_compare),
	MODSYM(DeeString_STR(&str_Error), NULL, UINT32_C(0xa9956e41), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Error),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Callable), NULL, UINT32_C(0xeb0130c3), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Callable),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_equals), DOCOF(equals), UINT32_C(0xcf48fdb6), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_equals),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str___pooad), NULL, UINT32_C(0x38ba68c9), MODSYM_FREADONLY | MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___pooad),
	MODSYM(DeeString_STR(&str_bool), NULL, UINT32_C(0x8fd0d24a), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_bool),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_gc), DOCOF(gc), UINT32_C(0x73e7fc4c), MODSYM_FREADONLY | MODSYM_FNAMEOBJ, id_gc),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Set), NULL, UINT32_C(0xf18ec750), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Set),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Frame), NULL, UINT32_C(0x162dc154), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Frame),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Mapping), NULL, UINT32_C(0x3465ead6), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Mapping),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Bytes), NULL, UINT32_C(0xa7a9ebde), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Bytes),
	MODSYM(DeeString_STR(&str_int), NULL, UINT32_C(0xce831ddf), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_int),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Function), NULL, UINT32_C(0x3614abe4), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Function),
	MODSYM(DeeString_STR(&str_Type), NULL, UINT32_C(0x746585e5), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Type),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Numeric), NULL, UINT32_C(0x634f8f67), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Numeric),
	MODSYM(DeeString_STR(&str_float), NULL, UINT32_C(0x95fb9fe8), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_float),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_InstanceMethod), NULL, UINT32_C(0xa7929fea), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_InstanceMethod),
	MODSYM(DeeString_STR(&str_Thread), NULL, UINT32_C(0x81f0afeb), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Thread),
	MODSYM(DeeString_STR(&str___badcall), NULL, UINT32_C(0x9795b98b), MODSYM_FREADONLY | MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___badcall),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Tuple), NULL, UINT32_C(0xf10ca66f), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Tuple),
	MODSYM(DeeString_STR(&str___neosb), NULL, UINT32_C(0x3a5bef0), MODSYM_FREADONLY | MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___neosb),
	MODSYM(DeeString_STR(&str___giosi), NULL, UINT32_C(0x30dd03a8), MODSYM_FREADONLY | MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___giosi),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Module), NULL, UINT32_C(0x5d2db7f4), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Module),
	MODSYM(DeeString_STR(&str_Cell), NULL, UINT32_C(0x152b23f5), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Cell),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_hash), DOCOF(hash), UINT32_C(0x56c454fb), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_hash),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_hasitem), DOCOF(hasitem), UINT32_C(0xfd5ab4fe), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_hasitem),
	MODSYM(NULL, NULL, 0, 0, 0),
#elif _Dee_HashSelect(32, 64) == 64
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Object), NULL, UINT64_C(0x6769a374488a3c81), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Object),
	MODSYM(DeeString_STR(&str_string), NULL, UINT64_C(0xa027864469ad4382), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_string),
	MODSYM(DeeString_STR(&str_exec), DOCOF(exec), UINT64_C(0x2efd876517f0e883), MODSYM_FNORMAL | MODSYM_FNAMEOBJ, id_exec),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Cell), NULL, UINT64_C(0xee2fd6b5878a788c), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Cell),
	MODSYM(DeeString_STR(&str_Callable), NULL, UINT64_C(0xa323908e8099518d), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Callable),
	MODSYM(DeeString_STR(&str_bool), NULL, UINT64_C(0x78e45f7b558db28e), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_bool),
	MODSYM(DeeString_STR(&str_WeakRefAble), NULL, UINT64_C(0x22c298dff7d3200f), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_WeakRefAble),
	MODSYM(DeeString_STR(&str_equals), DOCOF(equals), UINT64_C(0x8ff48babe6a36c10), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_equals),
	MODSYM(DeeString_STR(&str___giosi), NULL, UINT64_C(0x347263f7fbfcea2b), MODSYM_FREADONLY | MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___giosi),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Mapping), NULL, UINT64_C(0xacacbeb01821219a), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Mapping),
	MODSYM(DeeString_STR(&str_Signal), NULL, UINT64_C(0x966027ef8153891b), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Signal),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_none), NULL, UINT64_C(0xaf534e37b60e6020), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_none),
	MODSYM(DeeString_STR(&str_Tuple), NULL, UINT64_C(0x5075ee57ac96f2a1), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Tuple),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str___assert), NULL, UINT64_C(0xdf7c220c44eeb5a4), MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___assert),
	MODSYM(DeeString_STR(&str_bounditem), DOCOF(bounditem), UINT64_C(0x95383275aec67fa5), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_bounditem),
	MODSYM(DeeString_STR(&str_Frame), NULL, UINT64_C(0xe232866e91505426), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Frame),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Traceback), NULL, UINT64_C(0xa37936341573a7ab), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Traceback),
	MODSYM(DeeString_STR(&str___roloc), NULL, UINT64_C(0xa00f637868f0b44e), MODSYM_FREADONLY | MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___roloc),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_List), NULL, UINT64_C(0xfd145407ab974fb5), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_List),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_enumattr), NULL, UINT64_C(0x6b627a9d4ba17e37), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_enumattr),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Set), NULL, UINT64_C(0xe64a97dec556c73c), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Set),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str___pooad), NULL, UINT64_C(0xd5562c36880fcfa0), MODSYM_FREADONLY | MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___pooad),
	MODSYM(DeeString_STR(&str_hasattr), DOCOF(hasattr), UINT64_C(0xad2ec658de4b00d3), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_hasattr),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str___import__), DOCOF(__import__), UINT64_C(0x9083cbce4d7003c2), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id___import__),
	MODSYM(DeeString_STR(&str___gaosa), NULL, UINT64_C(0x31b6aef35b2a1f6), MODSYM_FREADONLY | MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___gaosa),
	MODSYM(DeeString_STR(&str_Module), NULL, UINT64_C(0x75bd883e9fa8a946), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Module),
	MODSYM(DeeString_STR(&str_HashSet), NULL, UINT64_C(0xbe7c97fdd78092a1), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_HashSet),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_WeakRef), NULL, UINT64_C(0x412a8970996e9e0c), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_WeakRef),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_gc), DOCOF(gc), UINT64_C(0x5369f38dbb7cb94e), MODSYM_FREADONLY | MODSYM_FNAMEOBJ, id_gc),
	MODSYM(DeeString_STR(&str_Numeric), NULL, UINT64_C(0x768d9e160bb13cf), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Numeric),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_File), NULL, UINT64_C(0xd32410b9199632d1), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_File),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_InstanceMethod), NULL, UINT64_C(0x6aaa126ce9665d53), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_InstanceMethod),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Property), NULL, UINT64_C(0x1cf12720947fcc55), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Property),
	MODSYM(DeeString_STR(&str___grosr), NULL, UINT64_C(0x4b4bdaef29d9b42b), MODSYM_FREADONLY | MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___grosr),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_compare), DOCOF(compare), UINT64_C(0x9165e5178389f3e4), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_compare),
	MODSYM(DeeString_STR(&str_Function), NULL, UINT64_C(0xacccb3e026a8a35a), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Function),
	MODSYM(DeeString_STR(&str_Error), NULL, UINT64_C(0xf32cf15e8c80bfdb), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Error),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Thread), NULL, UINT64_C(0xa13eeae86b00fdd), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Thread),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Super), NULL, UINT64_C(0x62d0e5c69f132edf), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Super),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Bytes), NULL, UINT64_C(0xe8f459d5fa21062), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Bytes),
	MODSYM(DeeString_STR(&str_Sequence), NULL, UINT64_C(0xd04bba0a4444f063), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Sequence),
	MODSYM(DeeString_STR(&str_float), NULL, UINT64_C(0x19ab2ca7919bffe4), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_float),
	MODSYM(DeeString_STR(&str___neosb), NULL, UINT64_C(0x18de2c5f371aa921), MODSYM_FREADONLY | MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___neosb),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_hash), DOCOF(hash), UINT64_C(0x4436b8a58bf97c51), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_hash),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_int), NULL, UINT64_C(0xb7ad4ebe928a1ef0), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_int),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str___badcall), NULL, UINT64_C(0xc9e3cd8eadb2ee72), MODSYM_FREADONLY | MODSYM_FHIDDEN | MODSYM_FNAMEOBJ, id___badcall),
	MODSYM(DeeString_STR(&str_Attribute), NULL, UINT64_C(0x2e763a5308721ff3), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Attribute),
	MODSYM(DeeString_STR(&str_boundattr), DOCOF(boundattr), UINT64_C(0x64616add8dce0b74), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_boundattr),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Dict), NULL, UINT64_C(0x848e5b9886ecb76), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Dict),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_hasitem), DOCOF(hasitem), UINT64_C(0x754610f6171d3ff9), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_hasitem),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Type), NULL, UINT64_C(0x6e282d042ac80ffb), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Type),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(NULL, NULL, 0, 0, 0),
	MODSYM(DeeString_STR(&str_Iterator), NULL, UINT64_C(0x3c33c9d5c64ebfff), MODSYM_FREADONLY | MODSYM_FCONSTEXPR | MODSYM_FNAMEOBJ, id_Iterator),
#elif !defined(__DEEMON__) /* ... */
#error "Unsupported hash configuration"
#endif /* !... */
};
/*[[[end]]]*/


#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
struct Dee_deemon_module_struct {
	/* Even though never tracked, static modules still need the GC header for visiting. */
	struct Dee_gc_head_link                   m_head;
	Dee_MODULE_STRUCT(/**/, num_builtins_obj) m_module;
};

DDATDEF struct Dee_deemon_module_struct DeeModule_Deemon;

#ifndef CONFIG_NO_DEX
INTDEF struct Dee_module_dexdata deemon_dexdata;
#endif /* !CONFIG_NO_DEX */

#undef DeeModule_Deemon
PUBLIC struct Dee_deemon_module_struct DeeModule_Deemon = {
	{ _Dee_GC_HEAD_UNTRACKED_INIT }, {
#ifdef CONFIG_NO_DEX
		OBJECT_HEAD_INIT(&DeeModuleDee_Type),
#else /* CONFIG_NO_DEX */
		OBJECT_HEAD_INIT(&DeeModuleDex_Type),
#endif /* !CONFIG_NO_DEX */
		/* .mo_absname = */ NULL,
		/* .mo_absnode = */ { NULL, NULL, NULL },
		/* .mo_libname = */ {
			/* .mle_name = */ (DeeStringObject *)&str_deemon,
			/* .mle_dat  = */ { (DeeModuleObject *)&DeeModule_Deemon.m_module },
			/* .mle_node = */ { NULL, NULL, NULL },
			/* .mle_next = */ NULL
		},
		/* .mo_dir     = */ (DeeTupleObject *)Dee_EmptyTuple,
		/* .mo_init    = */ Dee_MODULE_INIT_INITIALIZED,
		/* .mo_buildid = */ { {0} }, /* Lazily initialized (from `DeeExec_GetTimestamp()') */
		/* .mo_flags   = */ Dee_MODULE_FNORMAL | _Dee_MODULE_FLIBALL,
		/* .mo_importc = */ 0,
		/* .mo_globalc = */ num_builtins_obj,
		/* .mo_bucketm = */ BUILTINS_HASHMASK,
		/* .mo_bucketv = */ deemon_symbols,
		_Dee_MODULE_INIT_mo_lock
		WEAKREF_SUPPORT_INIT,
#ifdef CONFIG_NO_DEX
		/* .mo_moddata = */ Dee_MODULE_MODDATA_INIT_CODE(&DeeCode_Empty),
#else /* CONFIG_NO_DEX */
		/* .mo_moddata = */ { &deemon_dexdata },
#endif /* !CONFIG_NO_DEX */
		/* .mo_importv = */ NULL,
		_Dee_MODULE_DEXDATA_INIT_LOADBOUNDS,
		/* .mo_globalv = */ {
#define BUILTIN(name, object, flags) (DeeObject *)object,
#define BUILTIN_NO_INCLUDES
#include "builtins.def"
#undef BUILTIN_NO_INCLUDES
		}
	}
};
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
#ifdef __INTELLISENSE__
PUBLIC struct Dee_static_module_struct DeeModule_Deemon_real =
#else /* __INTELLISENSE__ */
#undef DeeModule_Deemon
PUBLIC struct Dee_static_module_struct DeeModule_Deemon =
#endif /* !__INTELLISENSE__ */
{
	{
		/* ... */
		NULL,
		NULL
	}, {
		OBJECT_HEAD_INIT(&DeeModule_Type),
		/* .mo_name      = */ (DREF DeeStringObject *)&str_deemon,
		/* .mo_link      = */ LIST_ENTRY_UNBOUND_INITIALIZER,
		/* .mo_path      = */ NULL,
#ifdef DeeSystem_HAVE_FS_ICASE
		/* .mo_pathihash = */ 0,
#endif /* DeeSystem_HAVE_FS_ICASE */
		/* .mo_globlink  = */ LIST_ENTRY_UNBOUND_INITIALIZER,
		/* .mo_importc   = */ 0,
		/* .mo_globalc   = */ num_builtins_obj,
		/* .mo_flags     = */ Dee_MODULE_FLOADING | Dee_MODULE_FDIDLOAD | Dee_MODULE_FINITIALIZING | Dee_MODULE_FDIDINIT,
		/* .mo_bucketm   = */ BUILTINS_HASHMASK,
		/* .mo_bucketv   = */ deemon_symbols,
		/* .mo_importv   = */ NULL,
		/* .mo_globalv   = */ builtin_object_vector,
		/* .mo_root      = */ &DeeCode_Empty,
#ifndef CONFIG_NO_THREADS
		/* .mo_lock      = */ Dee_ATOMIC_RWLOCK_INIT,
		/* .mo_loader    = */ NULL,
#endif /* !CONFIG_NO_THREADS */
#ifndef CONFIG_NO_DEC
		/* .mo_ctime     = */ 0,
#endif /* !CONFIG_NO_DEC */
		WEAKREF_SUPPORT_INIT
	}
};
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_BUILTIN_C */
