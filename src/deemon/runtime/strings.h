/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_STRINGS_H
#define GUARD_DEEMON_RUNTIME_STRINGS_H 1

#include <deemon/api.h>
#include <deemon/file.h>
#include <deemon/object.h>
#include <deemon/string.h>

DECL_BEGIN

#ifndef DEF_STRING
#define DEF_STRING(name, str, hash32, hash64) INTDEF DeeStringObject name;
#endif /* !DEF_STRING */

/*[[[begin]]]*/
/*[[[deemon
import * from deemon;
import rt.hash as rtHash;
function defString(name, value?) {
	if (value !is bound) {
		value = name;
		name = f"str_{name}";
	}
	print("DEF_STRING(", name, ", ", repr value, ", ",
	      rtHash.hash32(value).hex(), ", ",
	      rtHash.hash64(value).hex(), ")");
	print("#define STR_", name.lsstrip("str_"), " DeeString_STR(&", name, ")");
}

local STRINGS = {
	"Signal",
	"Error",

	"Attribute",
	"Bytes",
	"Callable",
	"Cell",
	"Dict",
	"File",
	"Frame",
	"Function",
	"HashSet",
	"InstanceMethod",
	"Iterator",
	"Joined",
	"List",
	"Mapping",
	"Module",
	"Numeric",
	"Object",
	"Property",
	"Sequence",
	"Set",
	"Super",
	"Thread",
	"Traceback",
	"Tuple",
	"Type",
	"WeakRef",
	"WeakRefAble",

	"bool",
	"string",
	"int",
	"float",

	"none",
	"true",
	"false",
	"deemon",
	"seq",
	"operators",
	"files",
	"_jit",
	"codecs",
	"__encode",
	"__decode",
	"strict",
	"replace",
	"ignore",
	"posix",
	"environ",
	"get",
	"set",
	"enumattr",
	"gc",
	"import",
	"exec",
	"isatty",
	"isfloat",
	"tostr",
	"pop",
	"remove",
	"rremove",
	"removeall",
	"removeif",
	"erase",
	"insert",
	"clear",
	"append",
	"extend",
	"insertall",
	"xch",
	"resize",
	"pushfront",
	"pushback",
	"popfront",
	"popback",
	"frozen",

	"revert",
	"advance",
	"index",
	"prev",
	"hasprev",
	"hasnext",
	"rewind",
	"peek",

	"format",
	"__format__",
	"__name__",
	"__doc__",
	"__type__",
	"__kwds__",
	"__module__",
	"first",
	"last",

	"size",
	"filename",
	("str_tab", "\t"),

	// Misc strings needed for builtins
	"hasattr",
	"hasitem",
	"boundattr",
	"bounditem",
	"compare",
	"__pooad",
	"__neosb",
	"__giosi",
	"__grosr",
	"__gaosa",
	"__roloc",
	"__assert",
	"__badcall",

	"rt.d200",

	("str_lt_anonymous_gr", "<anonymous>"),
	("str_nomemory", "allocation failed"),
	("str_dots", "..."),

	// Strings used for internal AST branches
	"constexpr",
	"sym",
	"unbind",
	"bound",
	"multiple",
	"return",
	"yield",
	"throw",
	"try",
	"loop",
	"loopctl",
	"conditional",
	"bool",
	"expand",
	"function",
	"operatorfunc",
	"operator",
	"action",
	"class",
	"label",
	"goto",
	"switch",
	"assembly",

	// Strings used for internal symbol classes
	"none",
	"global",
	"extern",
	"module",
	"mymod",
	"getset",
	"ifield",
	"cfield",
	"alias",
	"arg",
	"local",
	"stack",
	"static",
	"except",
	"myfunc",
	"this",
	"ambig",
	"fwd",
	"const",
};

local generatedStrings = HashSet();
for (local s: STRINGS) {
	if (s is string)
		s = { f"str_{s.replace(".", "_")}", s };
	if (s.last in generatedStrings)
		continue;
	generatedStrings.insert(s.last);
	defString(s...);
}

print("#ifdef DeeSysFD_IS_HANDLE");
defString("str_getsysfd", "osfhandle_np");
print("#elif defined(DeeSysFD_IS_INT)");
defString("str_getsysfd", "fileno_np");
print("#endif /" "* ... *" "/");
print;

print("#ifndef CONFIG_NO_THREADS");
defString("run");
print("#endif /" "* !CONFIG_NO_THREADS *" "/");
print;

print("#ifndef CONFIG_LANGUAGE_NO_ASM");
defString("this_module");
defString("this_function");
defString("str_cell_empty", "cell empty");
print("#endif /" "* !CONFIG_LANGUAGE_NO_ASM *" "/");
]]]*/
DEF_STRING(str_Signal, "Signal", 0x9b300d86, 0x966027ef8153891b)
#define STR_Signal DeeString_STR(&str_Signal)
DEF_STRING(str_Error, "Error", 0xa9956e41, 0xf32cf15e8c80bfdb)
#define STR_Error DeeString_STR(&str_Error)
DEF_STRING(str_Attribute, "Attribute", 0xa08b731, 0x2e763a5308721ff3)
#define STR_Attribute DeeString_STR(&str_Attribute)
DEF_STRING(str_Bytes, "Bytes", 0xa7a9ebde, 0xe8f459d5fa21062)
#define STR_Bytes DeeString_STR(&str_Bytes)
DEF_STRING(str_Callable, "Callable", 0xeb0130c3, 0xa323908e8099518d)
#define STR_Callable DeeString_STR(&str_Callable)
DEF_STRING(str_Cell, "Cell", 0x152b23f5, 0xee2fd6b5878a788c)
#define STR_Cell DeeString_STR(&str_Cell)
DEF_STRING(str_Dict, "Dict", 0xb2f1a21, 0x848e5b9886ecb76)
#define STR_Dict DeeString_STR(&str_Dict)
DEF_STRING(str_File, "File", 0x5209cbaf, 0xd32410b9199632d1)
#define STR_File DeeString_STR(&str_File)
DEF_STRING(str_Frame, "Frame", 0x162dc154, 0xe232866e91505426)
#define STR_Frame DeeString_STR(&str_Frame)
DEF_STRING(str_Function, "Function", 0x3614abe4, 0xacccb3e026a8a35a)
#define STR_Function DeeString_STR(&str_Function)
DEF_STRING(str_HashSet, "HashSet", 0x19f6e15e, 0xbe7c97fdd78092a1)
#define STR_HashSet DeeString_STR(&str_HashSet)
DEF_STRING(str_InstanceMethod, "InstanceMethod", 0xa7929fea, 0x6aaa126ce9665d53)
#define STR_InstanceMethod DeeString_STR(&str_InstanceMethod)
DEF_STRING(str_Iterator, "Iterator", 0xfce46883, 0x3c33c9d5c64ebfff)
#define STR_Iterator DeeString_STR(&str_Iterator)
DEF_STRING(str_Joined, "Joined", 0x18f2ed, 0x97e484d348a54ca4)
#define STR_Joined DeeString_STR(&str_Joined)
DEF_STRING(str_List, "List", 0xab47f22, 0xfd145407ab974fb5)
#define STR_List DeeString_STR(&str_List)
DEF_STRING(str_Mapping, "Mapping", 0x3465ead6, 0xacacbeb01821219a)
#define STR_Mapping DeeString_STR(&str_Mapping)
DEF_STRING(str_Module, "Module", 0x5d2db7f4, 0x75bd883e9fa8a946)
#define STR_Module DeeString_STR(&str_Module)
DEF_STRING(str_Numeric, "Numeric", 0x634f8f67, 0x768d9e160bb13cf)
#define STR_Numeric DeeString_STR(&str_Numeric)
DEF_STRING(str_Object, "Object", 0xfa8141c1, 0x6769a374488a3c81)
#define STR_Object DeeString_STR(&str_Object)
DEF_STRING(str_Property, "Property", 0xd4f3688b, 0x1cf12720947fcc55)
#define STR_Property DeeString_STR(&str_Property)
DEF_STRING(str_Sequence, "Sequence", 0xe5937b14, 0xd04bba0a4444f063)
#define STR_Sequence DeeString_STR(&str_Sequence)
DEF_STRING(str_Set, "Set", 0xf18ec750, 0xe64a97dec556c73c)
#define STR_Set DeeString_STR(&str_Set)
DEF_STRING(str_Super, "Super", 0xa0d5169e, 0x62d0e5c69f132edf)
#define STR_Super DeeString_STR(&str_Super)
DEF_STRING(str_Thread, "Thread", 0x81f0afeb, 0xa13eeae86b00fdd)
#define STR_Thread DeeString_STR(&str_Thread)
DEF_STRING(str_Traceback, "Traceback", 0x65f6383d, 0xa37936341573a7ab)
#define STR_Traceback DeeString_STR(&str_Traceback)
DEF_STRING(str_Tuple, "Tuple", 0xf10ca66f, 0x5075ee57ac96f2a1)
#define STR_Tuple DeeString_STR(&str_Tuple)
DEF_STRING(str_Type, "Type", 0x746585e5, 0x6e282d042ac80ffb)
#define STR_Type DeeString_STR(&str_Type)
DEF_STRING(str_WeakRef, "WeakRef", 0x1968c00b, 0x412a8970996e9e0c)
#define STR_WeakRef DeeString_STR(&str_WeakRef)
DEF_STRING(str_WeakRefAble, "WeakRefAble", 0x3b8e11b6, 0x22c298dff7d3200f)
#define STR_WeakRefAble DeeString_STR(&str_WeakRefAble)
DEF_STRING(str_bool, "bool", 0x8fd0d24a, 0x78e45f7b558db28e)
#define STR_bool DeeString_STR(&str_bool)
DEF_STRING(str_string, "string", 0xad217aab, 0xa027864469ad4382)
#define STR_string DeeString_STR(&str_string)
DEF_STRING(str_int, "int", 0xce831ddf, 0xb7ad4ebe928a1ef0)
#define STR_int DeeString_STR(&str_int)
DEF_STRING(str_float, "float", 0x95fb9fe8, 0x19ab2ca7919bffe4)
#define STR_float DeeString_STR(&str_float)
DEF_STRING(str_none, "none", 0xde6dda00, 0xaf534e37b60e6020)
#define STR_none DeeString_STR(&str_none)
DEF_STRING(str_true, "true", 0x34edcf72, 0xbd1e3bcd102bfbc4)
#define STR_true DeeString_STR(&str_true)
DEF_STRING(str_false, "false", 0xd5619e17, 0x83cada938c98c156)
#define STR_false DeeString_STR(&str_false)
DEF_STRING(str_deemon, "deemon", 0x4579666d, 0xeb3bb684d0ec756)
#define STR_deemon DeeString_STR(&str_deemon)
DEF_STRING(str_seq, "seq", 0x232af2b7, 0x80a0b0950a5a5251)
#define STR_seq DeeString_STR(&str_seq)
DEF_STRING(str_operators, "operators", 0xd4b6b76c, 0xc8d5b5ae0eb7316e)
#define STR_operators DeeString_STR(&str_operators)
DEF_STRING(str_files, "files", 0x908e29de, 0x41e984160894009c)
#define STR_files DeeString_STR(&str_files)
DEF_STRING(str__jit, "_jit", 0x6f3e4261, 0xbcf5fb303015dc89)
#define STR__jit DeeString_STR(&str__jit)
DEF_STRING(str_codecs, "codecs", 0x341958d7, 0x51cf434bd995d8ac)
#define STR_codecs DeeString_STR(&str_codecs)
DEF_STRING(str___encode, "__encode", 0xe31efed3, 0xf1bfd986648273b9)
#define STR___encode DeeString_STR(&str___encode)
DEF_STRING(str___decode, "__decode", 0x1c21cb81, 0x55817bd8d69ec3f5)
#define STR___decode DeeString_STR(&str___decode)
DEF_STRING(str_strict, "strict", 0xc77e2a15, 0xffd127a282d4a0f0)
#define STR_strict DeeString_STR(&str_strict)
DEF_STRING(str_replace, "replace", 0x54b94882, 0x2d4ba4f8cfd63bc6)
#define STR_replace DeeString_STR(&str_replace)
DEF_STRING(str_ignore, "ignore", 0xbd41ea58, 0xd951d03dedff60d2)
#define STR_ignore DeeString_STR(&str_ignore)
DEF_STRING(str_posix, "posix", 0x8a12ee56, 0xfc8c64936b261e96)
#define STR_posix DeeString_STR(&str_posix)
DEF_STRING(str_environ, "environ", 0xd8ecb380, 0x8d2a0a9c2432f221)
#define STR_environ DeeString_STR(&str_environ)
DEF_STRING(str_get, "get", 0x3b6d35a2, 0x7c8e1568eac4979f)
#define STR_get DeeString_STR(&str_get)
DEF_STRING(str_set, "set", 0x5ecc6fe8, 0xe706aa03fdbe04fa)
#define STR_set DeeString_STR(&str_set)
DEF_STRING(str_enumattr, "enumattr", 0x767e1f86, 0x6b627a9d4ba17e37)
#define STR_enumattr DeeString_STR(&str_enumattr)
DEF_STRING(str_gc, "gc", 0x73e7fc4c, 0x5369f38dbb7cb94e)
#define STR_gc DeeString_STR(&str_gc)
DEF_STRING(str_import, "import", 0x1a3f5a1f, 0x5a525def3865fbed)
#define STR_import DeeString_STR(&str_import)
DEF_STRING(str_exec, "exec", 0x6b42be28, 0x2efd876517f0e883)
#define STR_exec DeeString_STR(&str_exec)
DEF_STRING(str_isatty, "isatty", 0xab82818e, 0x66fe4c97c9502a99)
#define STR_isatty DeeString_STR(&str_isatty)
DEF_STRING(str_isfloat, "isfloat", 0xe3da5546, 0x96c4dbe16a19d65d)
#define STR_isfloat DeeString_STR(&str_isfloat)
DEF_STRING(str_tostr, "tostr", 0x6b502fcb, 0x9cdf9482b472ce73)
#define STR_tostr DeeString_STR(&str_tostr)
DEF_STRING(str_pop, "pop", 0x960361ff, 0x666fb01461b0a0eb)
#define STR_pop DeeString_STR(&str_pop)
DEF_STRING(str_remove, "remove", 0x3d2727dd, 0xe9f313a03e2051a)
#define STR_remove DeeString_STR(&str_remove)
DEF_STRING(str_rremove, "rremove", 0x37ef1152, 0x199975a7908f6d6)
#define STR_rremove DeeString_STR(&str_rremove)
DEF_STRING(str_removeall, "removeall", 0x902407ed, 0x97879af70abc9349)
#define STR_removeall DeeString_STR(&str_removeall)
DEF_STRING(str_removeif, "removeif", 0x156aa732, 0x96ad85f728d8a11e)
#define STR_removeif DeeString_STR(&str_removeif)
DEF_STRING(str_erase, "erase", 0x6f5916cf, 0x65f9c8b6514af4e5)
#define STR_erase DeeString_STR(&str_erase)
DEF_STRING(str_insert, "insert", 0x71d74a66, 0x5e168c86241590d7)
#define STR_insert DeeString_STR(&str_insert)
DEF_STRING(str_clear, "clear", 0x7857faae, 0x22a34b6f82b3b83c)
#define STR_clear DeeString_STR(&str_clear)
DEF_STRING(str_append, "append", 0x5f19594f, 0x8c2b7c1aba65d5ee)
#define STR_append DeeString_STR(&str_append)
DEF_STRING(str_extend, "extend", 0x960b75e7, 0xba076858e3adb055)
#define STR_extend DeeString_STR(&str_extend)
DEF_STRING(str_insertall, "insertall", 0xbf9bc3a9, 0x4f85971d093a27f2)
#define STR_insertall DeeString_STR(&str_insertall)
DEF_STRING(str_xch, "xch", 0x818ce38a, 0x6bb37305be1b0321)
#define STR_xch DeeString_STR(&str_xch)
DEF_STRING(str_resize, "resize", 0x36fcb308, 0x573f3d2e97212b34)
#define STR_resize DeeString_STR(&str_resize)
DEF_STRING(str_pushfront, "pushfront", 0xc682cfdf, 0x5933eb9a387ff882)
#define STR_pushfront DeeString_STR(&str_pushfront)
DEF_STRING(str_pushback, "pushback", 0xad1e1509, 0x4cfafd84a12923bd)
#define STR_pushback DeeString_STR(&str_pushback)
DEF_STRING(str_popfront, "popfront", 0x46523911, 0x22a469cc52318bba)
#define STR_popfront DeeString_STR(&str_popfront)
DEF_STRING(str_popback, "popback", 0xd84577aa, 0xb77f74a49a9cc289)
#define STR_popback DeeString_STR(&str_popback)
DEF_STRING(str_frozen, "frozen", 0x82311b77, 0x7b55e2e6e642b6fd)
#define STR_frozen DeeString_STR(&str_frozen)
DEF_STRING(str_revert, "revert", 0x98ca826, 0x626b4fca0d39dcf2)
#define STR_revert DeeString_STR(&str_revert)
DEF_STRING(str_advance, "advance", 0xdd1157a0, 0x8667ad2c6ab8d35d)
#define STR_advance DeeString_STR(&str_advance)
DEF_STRING(str_index, "index", 0x77f34f0, 0x440d5888c0ff3081)
#define STR_index DeeString_STR(&str_index)
DEF_STRING(str_prev, "prev", 0xeb31683d, 0x7487ec947044729e)
#define STR_prev DeeString_STR(&str_prev)
DEF_STRING(str_hasprev, "hasprev", 0xe7e8f3c, 0x17b364986c9ecd3b)
#define STR_hasprev DeeString_STR(&str_hasprev)
DEF_STRING(str_hasnext, "hasnext", 0xae2186a8, 0x19d7bd95854b765f)
#define STR_hasnext DeeString_STR(&str_hasnext)
DEF_STRING(str_rewind, "rewind", 0x2ab1b235, 0xa35b8bb3941ca25f)
#define STR_rewind DeeString_STR(&str_rewind)
DEF_STRING(str_peek, "peek", 0xb2ae48a2, 0xcc667a4d924a91f8)
#define STR_peek DeeString_STR(&str_peek)
DEF_STRING(str_format, "format", 0x9ddde74c, 0xc575d5e219281e76)
#define STR_format DeeString_STR(&str_format)
DEF_STRING(str___format__, "__format__", 0xb59a1ae2, 0xdf14ed3788cde344)
#define STR___format__ DeeString_STR(&str___format__)
DEF_STRING(str___name__, "__name__", 0x27a6cbdf, 0x9004f0806b170f3f)
#define STR___name__ DeeString_STR(&str___name__)
DEF_STRING(str___doc__, "__doc__", 0xd5eefba, 0x9e1c0e198ad451ff)
#define STR___doc__ DeeString_STR(&str___doc__)
DEF_STRING(str___type__, "__type__", 0xc25dc337, 0xd3fa545616840a4e)
#define STR___type__ DeeString_STR(&str___type__)
DEF_STRING(str___kwds__, "__kwds__", 0xd3926a14, 0xa90825b224a7262b)
#define STR___kwds__ DeeString_STR(&str___kwds__)
DEF_STRING(str___module__, "__module__", 0x3bea6c9f, 0x183a20d7d6c28dbb)
#define STR___module__ DeeString_STR(&str___module__)
DEF_STRING(str_first, "first", 0xa9f0e818, 0x9d12a485470a29a7)
#define STR_first DeeString_STR(&str_first)
DEF_STRING(str_last, "last", 0x185a4f9a, 0x760894ca6d41e4dc)
#define STR_last DeeString_STR(&str_last)
DEF_STRING(str_size, "size", 0xed8917fa, 0x3fe8023bdf261c0f)
#define STR_size DeeString_STR(&str_size)
DEF_STRING(str_filename, "filename", 0x199d68d3, 0x4a5d0431e1a3caed)
#define STR_filename DeeString_STR(&str_filename)
DEF_STRING(str_tab, "\t", 0xe99b2213, 0x510da8cb03a6ba5d)
#define STR_tab DeeString_STR(&str_tab)
DEF_STRING(str_hasattr, "hasattr", 0xa37d5291, 0xad2ec658de4b00d3)
#define STR_hasattr DeeString_STR(&str_hasattr)
DEF_STRING(str_hasitem, "hasitem", 0xfd5ab4fe, 0x754610f6171d3ff9)
#define STR_hasitem DeeString_STR(&str_hasitem)
DEF_STRING(str_boundattr, "boundattr", 0xbe67bf95, 0x64616add8dce0b74)
#define STR_boundattr DeeString_STR(&str_boundattr)
DEF_STRING(str_bounditem, "bounditem", 0xbe8d5e6f, 0x95383275aec67fa5)
#define STR_bounditem DeeString_STR(&str_bounditem)
DEF_STRING(str_compare, "compare", 0x84b4e5c0, 0x9165e5178389f3e4)
#define STR_compare DeeString_STR(&str_compare)
DEF_STRING(str___pooad, "__pooad", 0x38ba68c9, 0xd5562c36880fcfa0)
#define STR___pooad DeeString_STR(&str___pooad)
DEF_STRING(str___neosb, "__neosb", 0x3a5bef0, 0x18de2c5f371aa921)
#define STR___neosb DeeString_STR(&str___neosb)
DEF_STRING(str___giosi, "__giosi", 0x30dd03a8, 0x347263f7fbfcea2b)
#define STR___giosi DeeString_STR(&str___giosi)
DEF_STRING(str___grosr, "__grosr", 0xee715568, 0x4b4bdaef29d9b42b)
#define STR___grosr DeeString_STR(&str___grosr)
DEF_STRING(str___gaosa, "__gaosa", 0xbdd31637, 0x31b6aef35b2a1f6)
#define STR___gaosa DeeString_STR(&str___gaosa)
DEF_STRING(str___roloc, "__roloc", 0xf4874267, 0xa00f637868f0b44e)
#define STR___roloc DeeString_STR(&str___roloc)
DEF_STRING(str___assert, "__assert", 0xd4715fbd, 0xdf7c220c44eeb5a4)
#define STR___assert DeeString_STR(&str___assert)
DEF_STRING(str___badcall, "__badcall", 0x9795b98b, 0xc9e3cd8eadb2ee72)
#define STR___badcall DeeString_STR(&str___badcall)
DEF_STRING(str_rt_d200, "rt.d200", 0x6b64a89c, 0x798d95b52acb68ab)
#define STR_rt_d200 DeeString_STR(&str_rt_d200)
DEF_STRING(str_lt_anonymous_gr, "<anonymous>", 0xd5f7311c, 0x536d809a7d1c1aab)
#define STR_lt_anonymous_gr DeeString_STR(&str_lt_anonymous_gr)
DEF_STRING(str_nomemory, "allocation failed", 0xbef65010, 0x6315c4e658da5e37)
#define STR_nomemory DeeString_STR(&str_nomemory)
DEF_STRING(str_dots, "...", 0x1a086252, 0xf5eff0465042ef13)
#define STR_dots DeeString_STR(&str_dots)
DEF_STRING(str_constexpr, "constexpr", 0x58c650ca, 0xf0a0c2270200c1f0)
#define STR_constexpr DeeString_STR(&str_constexpr)
DEF_STRING(str_sym, "sym", 0x17fd993c, 0x1c0e6e19c328844b)
#define STR_sym DeeString_STR(&str_sym)
DEF_STRING(str_unbind, "unbind", 0x5a30901a, 0xa4d534ebcf828a7a)
#define STR_unbind DeeString_STR(&str_unbind)
DEF_STRING(str_bound, "bound", 0x5d3fd85e, 0x9200603171c0f3b0)
#define STR_bound DeeString_STR(&str_bound)
DEF_STRING(str_multiple, "multiple", 0x298dfbdb, 0x28f61f03af08a3ca)
#define STR_multiple DeeString_STR(&str_multiple)
DEF_STRING(str_return, "return", 0x553dfc89, 0x2a644c3d30df4872)
#define STR_return DeeString_STR(&str_return)
DEF_STRING(str_yield, "yield", 0x96c58e93, 0x17e5134b02b40024)
#define STR_yield DeeString_STR(&str_yield)
DEF_STRING(str_throw, "throw", 0x17f397c6, 0x1267633ecc33c2c)
#define STR_throw DeeString_STR(&str_throw)
DEF_STRING(str_try, "try", 0xb3e4fc45, 0xeb7c790414e244ab)
#define STR_try DeeString_STR(&str_try)
DEF_STRING(str_loop, "loop", 0xe44e70af, 0xadc137e48b7293ee)
#define STR_loop DeeString_STR(&str_loop)
DEF_STRING(str_loopctl, "loopctl", 0xa389d8bc, 0x2d55b845bc0a547d)
#define STR_loopctl DeeString_STR(&str_loopctl)
DEF_STRING(str_conditional, "conditional", 0x4445d225, 0xb0965c593399d415)
#define STR_conditional DeeString_STR(&str_conditional)
DEF_STRING(str_expand, "expand", 0x7708077c, 0x793fc0a371216076)
#define STR_expand DeeString_STR(&str_expand)
DEF_STRING(str_function, "function", 0x3d75dc7a, 0xf36d912a52403931)
#define STR_function DeeString_STR(&str_function)
DEF_STRING(str_operatorfunc, "operatorfunc", 0x4c528db6, 0x4cdc8fe9282f4a88)
#define STR_operatorfunc DeeString_STR(&str_operatorfunc)
DEF_STRING(str_operator, "operator", 0xa5f184dd, 0x970564e28323bb4)
#define STR_operator DeeString_STR(&str_operator)
DEF_STRING(str_action, "action", 0x3679cc25, 0x4cca7f022ef68151)
#define STR_action DeeString_STR(&str_action)
DEF_STRING(str_class, "class", 0xf769f29e, 0xc5397fe5c82dbd12)
#define STR_class DeeString_STR(&str_class)
DEF_STRING(str_label, "label", 0x8c7dd24d, 0x10ab80c11491baad)
#define STR_label DeeString_STR(&str_label)
DEF_STRING(str_goto, "goto", 0x6e4777d3, 0x75d33f290274060e)
#define STR_goto DeeString_STR(&str_goto)
DEF_STRING(str_switch, "switch", 0xfb63ec5e, 0x70bce0e305461cce)
#define STR_switch DeeString_STR(&str_switch)
DEF_STRING(str_assembly, "assembly", 0x88542c16, 0x9d1c909a08c788a3)
#define STR_assembly DeeString_STR(&str_assembly)
DEF_STRING(str_global, "global", 0x42ebe077, 0x4b19ecd5b61b5296)
#define STR_global DeeString_STR(&str_global)
DEF_STRING(str_extern, "extern", 0xc801935e, 0x8b47f0a1d76ecba3)
#define STR_extern DeeString_STR(&str_extern)
DEF_STRING(str_module, "module", 0xae3684a4, 0xbb78a82535e5801e)
#define STR_module DeeString_STR(&str_module)
DEF_STRING(str_mymod, "mymod", 0x85c64d4c, 0xf4aa94c3b47cfe72)
#define STR_mymod DeeString_STR(&str_mymod)
DEF_STRING(str_getset, "getset", 0x4be0a05b, 0xd5b87464b9cb7503)
#define STR_getset DeeString_STR(&str_getset)
DEF_STRING(str_ifield, "ifield", 0xce5e0d5c, 0x86dab707bb32c291)
#define STR_ifield DeeString_STR(&str_ifield)
DEF_STRING(str_cfield, "cfield", 0x12ffac49, 0xb7298fd50b333835)
#define STR_cfield DeeString_STR(&str_cfield)
DEF_STRING(str_alias, "alias", 0xca8b8108, 0x5826aeb387c6da71)
#define STR_alias DeeString_STR(&str_alias)
DEF_STRING(str_arg, "arg", 0xd22385c4, 0x72e1ee0d651e56d8)
#define STR_arg DeeString_STR(&str_arg)
DEF_STRING(str_local, "local", 0x1d3dfa1, 0x3ede3ae259b4a7d2)
#define STR_local DeeString_STR(&str_local)
DEF_STRING(str_stack, "stack", 0x30b318, 0xd084e637b870262b)
#define STR_stack DeeString_STR(&str_stack)
DEF_STRING(str_static, "static", 0x6758a24b, 0x9f5751e5f08d205d)
#define STR_static DeeString_STR(&str_static)
DEF_STRING(str_except, "except", 0x8aae072b, 0xac70487e4861a6f3)
#define STR_except DeeString_STR(&str_except)
DEF_STRING(str_myfunc, "myfunc", 0x67667409, 0x45b51811d7de12b2)
#define STR_myfunc DeeString_STR(&str_myfunc)
DEF_STRING(str_this, "this", 0x142984e, 0x19d213c79c35abf3)
#define STR_this DeeString_STR(&str_this)
DEF_STRING(str_ambig, "ambig", 0x963ab94d, 0x62fe7ee447f767bd)
#define STR_ambig DeeString_STR(&str_ambig)
DEF_STRING(str_fwd, "fwd", 0x4d05936a, 0x468b9d355ef7e041)
#define STR_fwd DeeString_STR(&str_fwd)
DEF_STRING(str_const, "const", 0x95daec48, 0x2e9cb1cd0ec552da)
#define STR_const DeeString_STR(&str_const)
#ifdef DeeSysFD_IS_HANDLE
DEF_STRING(str_getsysfd, "osfhandle_np", 0x75b169b6, 0x74235841d2ace4f0)
#define STR_getsysfd DeeString_STR(&str_getsysfd)
#elif defined(DeeSysFD_IS_INT)
DEF_STRING(str_getsysfd, "fileno_np", 0xe3e546ab, 0x38c7dbc48e44183)
#define STR_getsysfd DeeString_STR(&str_getsysfd)
#endif /* ... */

#ifndef CONFIG_NO_THREADS
DEF_STRING(str_run, "run", 0xf1764c48, 0x7b92c951b5e510e3)
#define STR_run DeeString_STR(&str_run)
#endif /* !CONFIG_NO_THREADS */

#ifndef CONFIG_LANGUAGE_NO_ASM
DEF_STRING(str_this_module, "this_module", 0x34998e44, 0x473e02aa4d7eba45)
#define STR_this_module DeeString_STR(&str_this_module)
DEF_STRING(str_this_function, "this_function", 0xe2b69fa3, 0xdf2ba17d58877ece)
#define STR_this_function DeeString_STR(&str_this_function)
DEF_STRING(str_cell_empty, "cell empty", 0x5df2baea, 0x87d3182524393808)
#define STR_cell_empty DeeString_STR(&str_cell_empty)
#endif /* !CONFIG_LANGUAGE_NO_ASM */
/*[[[end]]]*/

#undef DEF_STRING

#ifndef STR_run
#define STR_run "run"
#endif /* !STR_run */
#ifndef STR_this_module
#define STR_this_module "this_module"
#endif /* !STR_this_module */
#ifndef STR_this_function
#define STR_this_function "this_function"
#endif /* !STR_this_function */

/* Some versions of GCC think that using DeeString_STR() on a static
 * string object will result in us reading from out-of-bounds memory,
 * since it doesn't understand that static objects (with flexible array
 * members) can still be larger than the object's minimal size. */
__pragma_GCC_diagnostic_ignored(Wstringop_overflow)

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_STRINGS_H */
