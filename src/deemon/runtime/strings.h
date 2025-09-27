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
import _Dee_HashSelect from rt.gen.hash;
function defString(name, value?) {
	if (value !is bound) {
		value = name;
		name = f"str_{name}";
	}
	print(f"#define Dee_HashStr__{name[4:]} {_Dee_HashSelect(value)}");
	print("DEF_STRING(", name, ", ", repr value, ", ",
	      rtHash.hash32(value).hex(), ", ",
	      rtHash.hash64(value).hex(), ")");
	print("#define STR_", name.lsstrip("str_"), " DeeString_STR(&", name, ")");
}

local STRINGS = List {
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
	"KeysIterator",
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
	"ignore",
	"posix",
	"environ",
	"get",
	"set",
	"enumattr",
	"gc",
	"__import__",
	"hash",
	"exec",
	"isatty",
	"isfloat",
	"tostr",

	"index",
	"revert",
	"advance",
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

	"size",
	"filename",
	("str_tab", "\t"),

	// Misc strings needed for builtins
	"hasattr",
	"hasitem",
	"boundattr",
	"bounditem",
	"compare",
	"equals",
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

	// Messages
	"No active exception",
};

// Append strings needed for method hints
import getAllMethodHintAttributes from "..method-hints.method-hints";
STRINGS.extend(getAllMethodHintAttributes());

local generatedStrings = HashSet();
for (local s: STRINGS
	.map(e -> e is string ? { f"str_{e.rereplace(r"[.\s]", "_")}", e } : e)
//	.sorted()
) {
	if (generatedStrings.insert(s.last))
		defString(s...);
}

print("#ifdef Dee_fd_t_IS_HANDLE");
defString("str_getsysfd", "osfhandle_np");
print("#elif defined(Dee_fd_t_IS_int)");
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
print("#endif /" "* !CONFIG_LANGUAGE_NO_ASM *" "/");
]]]*/
#define Dee_HashStr__Signal _Dee_HashSelectC(0x9b300d86, 0x966027ef8153891b)
DEF_STRING(str_Signal, "Signal", 0x9b300d86, 0x966027ef8153891b)
#define STR_Signal DeeString_STR(&str_Signal)
#define Dee_HashStr__Error _Dee_HashSelectC(0xa9956e41, 0xf32cf15e8c80bfdb)
DEF_STRING(str_Error, "Error", 0xa9956e41, 0xf32cf15e8c80bfdb)
#define STR_Error DeeString_STR(&str_Error)
#define Dee_HashStr__Attribute _Dee_HashSelectC(0xa08b731, 0x2e763a5308721ff3)
DEF_STRING(str_Attribute, "Attribute", 0xa08b731, 0x2e763a5308721ff3)
#define STR_Attribute DeeString_STR(&str_Attribute)
#define Dee_HashStr__Bytes _Dee_HashSelectC(0xa7a9ebde, 0xe8f459d5fa21062)
DEF_STRING(str_Bytes, "Bytes", 0xa7a9ebde, 0xe8f459d5fa21062)
#define STR_Bytes DeeString_STR(&str_Bytes)
#define Dee_HashStr__Callable _Dee_HashSelectC(0xeb0130c3, 0xa323908e8099518d)
DEF_STRING(str_Callable, "Callable", 0xeb0130c3, 0xa323908e8099518d)
#define STR_Callable DeeString_STR(&str_Callable)
#define Dee_HashStr__Cell _Dee_HashSelectC(0x152b23f5, 0xee2fd6b5878a788c)
DEF_STRING(str_Cell, "Cell", 0x152b23f5, 0xee2fd6b5878a788c)
#define STR_Cell DeeString_STR(&str_Cell)
#define Dee_HashStr__Dict _Dee_HashSelectC(0xb2f1a21, 0x848e5b9886ecb76)
DEF_STRING(str_Dict, "Dict", 0xb2f1a21, 0x848e5b9886ecb76)
#define STR_Dict DeeString_STR(&str_Dict)
#define Dee_HashStr__File _Dee_HashSelectC(0x5209cbaf, 0xd32410b9199632d1)
DEF_STRING(str_File, "File", 0x5209cbaf, 0xd32410b9199632d1)
#define STR_File DeeString_STR(&str_File)
#define Dee_HashStr__Frame _Dee_HashSelectC(0x162dc154, 0xe232866e91505426)
DEF_STRING(str_Frame, "Frame", 0x162dc154, 0xe232866e91505426)
#define STR_Frame DeeString_STR(&str_Frame)
#define Dee_HashStr__Function _Dee_HashSelectC(0x3614abe4, 0xacccb3e026a8a35a)
DEF_STRING(str_Function, "Function", 0x3614abe4, 0xacccb3e026a8a35a)
#define STR_Function DeeString_STR(&str_Function)
#define Dee_HashStr__HashSet _Dee_HashSelectC(0x19f6e15e, 0xbe7c97fdd78092a1)
DEF_STRING(str_HashSet, "HashSet", 0x19f6e15e, 0xbe7c97fdd78092a1)
#define STR_HashSet DeeString_STR(&str_HashSet)
#define Dee_HashStr__InstanceMethod _Dee_HashSelectC(0xa7929fea, 0x6aaa126ce9665d53)
DEF_STRING(str_InstanceMethod, "InstanceMethod", 0xa7929fea, 0x6aaa126ce9665d53)
#define STR_InstanceMethod DeeString_STR(&str_InstanceMethod)
#define Dee_HashStr__Iterator _Dee_HashSelectC(0xfce46883, 0x3c33c9d5c64ebfff)
DEF_STRING(str_Iterator, "Iterator", 0xfce46883, 0x3c33c9d5c64ebfff)
#define STR_Iterator DeeString_STR(&str_Iterator)
#define Dee_HashStr__KeysIterator _Dee_HashSelectC(0x4414d7ed, 0xb21fd2b052003297)
DEF_STRING(str_KeysIterator, "KeysIterator", 0x4414d7ed, 0xb21fd2b052003297)
#define STR_KeysIterator DeeString_STR(&str_KeysIterator)
#define Dee_HashStr__Joined _Dee_HashSelectC(0x18f2ed, 0x97e484d348a54ca4)
DEF_STRING(str_Joined, "Joined", 0x18f2ed, 0x97e484d348a54ca4)
#define STR_Joined DeeString_STR(&str_Joined)
#define Dee_HashStr__List _Dee_HashSelectC(0xab47f22, 0xfd145407ab974fb5)
DEF_STRING(str_List, "List", 0xab47f22, 0xfd145407ab974fb5)
#define STR_List DeeString_STR(&str_List)
#define Dee_HashStr__Mapping _Dee_HashSelectC(0x3465ead6, 0xacacbeb01821219a)
DEF_STRING(str_Mapping, "Mapping", 0x3465ead6, 0xacacbeb01821219a)
#define STR_Mapping DeeString_STR(&str_Mapping)
#define Dee_HashStr__Module _Dee_HashSelectC(0x5d2db7f4, 0x75bd883e9fa8a946)
DEF_STRING(str_Module, "Module", 0x5d2db7f4, 0x75bd883e9fa8a946)
#define STR_Module DeeString_STR(&str_Module)
#define Dee_HashStr__Numeric _Dee_HashSelectC(0x634f8f67, 0x768d9e160bb13cf)
DEF_STRING(str_Numeric, "Numeric", 0x634f8f67, 0x768d9e160bb13cf)
#define STR_Numeric DeeString_STR(&str_Numeric)
#define Dee_HashStr__Object _Dee_HashSelectC(0xfa8141c1, 0x6769a374488a3c81)
DEF_STRING(str_Object, "Object", 0xfa8141c1, 0x6769a374488a3c81)
#define STR_Object DeeString_STR(&str_Object)
#define Dee_HashStr__Property _Dee_HashSelectC(0xd4f3688b, 0x1cf12720947fcc55)
DEF_STRING(str_Property, "Property", 0xd4f3688b, 0x1cf12720947fcc55)
#define STR_Property DeeString_STR(&str_Property)
#define Dee_HashStr__Sequence _Dee_HashSelectC(0xe5937b14, 0xd04bba0a4444f063)
DEF_STRING(str_Sequence, "Sequence", 0xe5937b14, 0xd04bba0a4444f063)
#define STR_Sequence DeeString_STR(&str_Sequence)
#define Dee_HashStr__Set _Dee_HashSelectC(0xf18ec750, 0xe64a97dec556c73c)
DEF_STRING(str_Set, "Set", 0xf18ec750, 0xe64a97dec556c73c)
#define STR_Set DeeString_STR(&str_Set)
#define Dee_HashStr__Super _Dee_HashSelectC(0xa0d5169e, 0x62d0e5c69f132edf)
DEF_STRING(str_Super, "Super", 0xa0d5169e, 0x62d0e5c69f132edf)
#define STR_Super DeeString_STR(&str_Super)
#define Dee_HashStr__Thread _Dee_HashSelectC(0x81f0afeb, 0xa13eeae86b00fdd)
DEF_STRING(str_Thread, "Thread", 0x81f0afeb, 0xa13eeae86b00fdd)
#define STR_Thread DeeString_STR(&str_Thread)
#define Dee_HashStr__Traceback _Dee_HashSelectC(0x65f6383d, 0xa37936341573a7ab)
DEF_STRING(str_Traceback, "Traceback", 0x65f6383d, 0xa37936341573a7ab)
#define STR_Traceback DeeString_STR(&str_Traceback)
#define Dee_HashStr__Tuple _Dee_HashSelectC(0xf10ca66f, 0x5075ee57ac96f2a1)
DEF_STRING(str_Tuple, "Tuple", 0xf10ca66f, 0x5075ee57ac96f2a1)
#define STR_Tuple DeeString_STR(&str_Tuple)
#define Dee_HashStr__Type _Dee_HashSelectC(0x746585e5, 0x6e282d042ac80ffb)
DEF_STRING(str_Type, "Type", 0x746585e5, 0x6e282d042ac80ffb)
#define STR_Type DeeString_STR(&str_Type)
#define Dee_HashStr__WeakRef _Dee_HashSelectC(0x1968c00b, 0x412a8970996e9e0c)
DEF_STRING(str_WeakRef, "WeakRef", 0x1968c00b, 0x412a8970996e9e0c)
#define STR_WeakRef DeeString_STR(&str_WeakRef)
#define Dee_HashStr__WeakRefAble _Dee_HashSelectC(0x3b8e11b6, 0x22c298dff7d3200f)
DEF_STRING(str_WeakRefAble, "WeakRefAble", 0x3b8e11b6, 0x22c298dff7d3200f)
#define STR_WeakRefAble DeeString_STR(&str_WeakRefAble)
#define Dee_HashStr__bool _Dee_HashSelectC(0x8fd0d24a, 0x78e45f7b558db28e)
DEF_STRING(str_bool, "bool", 0x8fd0d24a, 0x78e45f7b558db28e)
#define STR_bool DeeString_STR(&str_bool)
#define Dee_HashStr__string _Dee_HashSelectC(0xad217aab, 0xa027864469ad4382)
DEF_STRING(str_string, "string", 0xad217aab, 0xa027864469ad4382)
#define STR_string DeeString_STR(&str_string)
#define Dee_HashStr__int _Dee_HashSelectC(0xce831ddf, 0xb7ad4ebe928a1ef0)
DEF_STRING(str_int, "int", 0xce831ddf, 0xb7ad4ebe928a1ef0)
#define STR_int DeeString_STR(&str_int)
#define Dee_HashStr__float _Dee_HashSelectC(0x95fb9fe8, 0x19ab2ca7919bffe4)
DEF_STRING(str_float, "float", 0x95fb9fe8, 0x19ab2ca7919bffe4)
#define STR_float DeeString_STR(&str_float)
#define Dee_HashStr__none _Dee_HashSelectC(0xde6dda00, 0xaf534e37b60e6020)
DEF_STRING(str_none, "none", 0xde6dda00, 0xaf534e37b60e6020)
#define STR_none DeeString_STR(&str_none)
#define Dee_HashStr__true _Dee_HashSelectC(0x34edcf72, 0xbd1e3bcd102bfbc4)
DEF_STRING(str_true, "true", 0x34edcf72, 0xbd1e3bcd102bfbc4)
#define STR_true DeeString_STR(&str_true)
#define Dee_HashStr__false _Dee_HashSelectC(0xd5619e17, 0x83cada938c98c156)
DEF_STRING(str_false, "false", 0xd5619e17, 0x83cada938c98c156)
#define STR_false DeeString_STR(&str_false)
#define Dee_HashStr__deemon _Dee_HashSelectC(0x4579666d, 0xeb3bb684d0ec756)
DEF_STRING(str_deemon, "deemon", 0x4579666d, 0xeb3bb684d0ec756)
#define STR_deemon DeeString_STR(&str_deemon)
#define Dee_HashStr__seq _Dee_HashSelectC(0x232af2b7, 0x80a0b0950a5a5251)
DEF_STRING(str_seq, "seq", 0x232af2b7, 0x80a0b0950a5a5251)
#define STR_seq DeeString_STR(&str_seq)
#define Dee_HashStr__operators _Dee_HashSelectC(0xd4b6b76c, 0xc8d5b5ae0eb7316e)
DEF_STRING(str_operators, "operators", 0xd4b6b76c, 0xc8d5b5ae0eb7316e)
#define STR_operators DeeString_STR(&str_operators)
#define Dee_HashStr__files _Dee_HashSelectC(0x908e29de, 0x41e984160894009c)
DEF_STRING(str_files, "files", 0x908e29de, 0x41e984160894009c)
#define STR_files DeeString_STR(&str_files)
#define Dee_HashStr___jit _Dee_HashSelectC(0x6f3e4261, 0xbcf5fb303015dc89)
DEF_STRING(str__jit, "_jit", 0x6f3e4261, 0xbcf5fb303015dc89)
#define STR__jit DeeString_STR(&str__jit)
#define Dee_HashStr__codecs _Dee_HashSelectC(0x341958d7, 0x51cf434bd995d8ac)
DEF_STRING(str_codecs, "codecs", 0x341958d7, 0x51cf434bd995d8ac)
#define STR_codecs DeeString_STR(&str_codecs)
#define Dee_HashStr____encode _Dee_HashSelectC(0xe31efed3, 0xf1bfd986648273b9)
DEF_STRING(str___encode, "__encode", 0xe31efed3, 0xf1bfd986648273b9)
#define STR___encode DeeString_STR(&str___encode)
#define Dee_HashStr____decode _Dee_HashSelectC(0x1c21cb81, 0x55817bd8d69ec3f5)
DEF_STRING(str___decode, "__decode", 0x1c21cb81, 0x55817bd8d69ec3f5)
#define STR___decode DeeString_STR(&str___decode)
#define Dee_HashStr__strict _Dee_HashSelectC(0xc77e2a15, 0xffd127a282d4a0f0)
DEF_STRING(str_strict, "strict", 0xc77e2a15, 0xffd127a282d4a0f0)
#define STR_strict DeeString_STR(&str_strict)
#define Dee_HashStr__replace _Dee_HashSelectC(0x54b94882, 0x2d4ba4f8cfd63bc6)
DEF_STRING(str_replace, "replace", 0x54b94882, 0x2d4ba4f8cfd63bc6)
#define STR_replace DeeString_STR(&str_replace)
#define Dee_HashStr__ignore _Dee_HashSelectC(0xbd41ea58, 0xd951d03dedff60d2)
DEF_STRING(str_ignore, "ignore", 0xbd41ea58, 0xd951d03dedff60d2)
#define STR_ignore DeeString_STR(&str_ignore)
#define Dee_HashStr__posix _Dee_HashSelectC(0x8a12ee56, 0xfc8c64936b261e96)
DEF_STRING(str_posix, "posix", 0x8a12ee56, 0xfc8c64936b261e96)
#define STR_posix DeeString_STR(&str_posix)
#define Dee_HashStr__environ _Dee_HashSelectC(0xd8ecb380, 0x8d2a0a9c2432f221)
DEF_STRING(str_environ, "environ", 0xd8ecb380, 0x8d2a0a9c2432f221)
#define STR_environ DeeString_STR(&str_environ)
#define Dee_HashStr__get _Dee_HashSelectC(0x3b6d35a2, 0x7c8e1568eac4979f)
DEF_STRING(str_get, "get", 0x3b6d35a2, 0x7c8e1568eac4979f)
#define STR_get DeeString_STR(&str_get)
#define Dee_HashStr__set _Dee_HashSelectC(0x5ecc6fe8, 0xe706aa03fdbe04fa)
DEF_STRING(str_set, "set", 0x5ecc6fe8, 0xe706aa03fdbe04fa)
#define STR_set DeeString_STR(&str_set)
#define Dee_HashStr__enumattr _Dee_HashSelectC(0x767e1f86, 0x6b627a9d4ba17e37)
DEF_STRING(str_enumattr, "enumattr", 0x767e1f86, 0x6b627a9d4ba17e37)
#define STR_enumattr DeeString_STR(&str_enumattr)
#define Dee_HashStr__gc _Dee_HashSelectC(0x73e7fc4c, 0x5369f38dbb7cb94e)
DEF_STRING(str_gc, "gc", 0x73e7fc4c, 0x5369f38dbb7cb94e)
#define STR_gc DeeString_STR(&str_gc)
#define Dee_HashStr____import__ _Dee_HashSelectC(0x5ace85a6, 0x9083cbce4d7003c2)
DEF_STRING(str___import__, "__import__", 0x5ace85a6, 0x9083cbce4d7003c2)
#define STR___import__ DeeString_STR(&str___import__)
#define Dee_HashStr__hash _Dee_HashSelectC(0x56c454fb, 0x4436b8a58bf97c51)
DEF_STRING(str_hash, "hash", 0x56c454fb, 0x4436b8a58bf97c51)
#define STR_hash DeeString_STR(&str_hash)
#define Dee_HashStr__exec _Dee_HashSelectC(0x6b42be28, 0x2efd876517f0e883)
DEF_STRING(str_exec, "exec", 0x6b42be28, 0x2efd876517f0e883)
#define STR_exec DeeString_STR(&str_exec)
#define Dee_HashStr__isatty _Dee_HashSelectC(0xab82818e, 0x66fe4c97c9502a99)
DEF_STRING(str_isatty, "isatty", 0xab82818e, 0x66fe4c97c9502a99)
#define STR_isatty DeeString_STR(&str_isatty)
#define Dee_HashStr__isfloat _Dee_HashSelectC(0xe3da5546, 0x96c4dbe16a19d65d)
DEF_STRING(str_isfloat, "isfloat", 0xe3da5546, 0x96c4dbe16a19d65d)
#define STR_isfloat DeeString_STR(&str_isfloat)
#define Dee_HashStr__tostr _Dee_HashSelectC(0x6b502fcb, 0x9cdf9482b472ce73)
DEF_STRING(str_tostr, "tostr", 0x6b502fcb, 0x9cdf9482b472ce73)
#define STR_tostr DeeString_STR(&str_tostr)
#define Dee_HashStr__index _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081)
DEF_STRING(str_index, "index", 0x77f34f0, 0x440d5888c0ff3081)
#define STR_index DeeString_STR(&str_index)
#define Dee_HashStr__revert _Dee_HashSelectC(0x98ca826, 0x626b4fca0d39dcf2)
DEF_STRING(str_revert, "revert", 0x98ca826, 0x626b4fca0d39dcf2)
#define STR_revert DeeString_STR(&str_revert)
#define Dee_HashStr__advance _Dee_HashSelectC(0xdd1157a0, 0x8667ad2c6ab8d35d)
DEF_STRING(str_advance, "advance", 0xdd1157a0, 0x8667ad2c6ab8d35d)
#define STR_advance DeeString_STR(&str_advance)
#define Dee_HashStr__prev _Dee_HashSelectC(0xeb31683d, 0x7487ec947044729e)
DEF_STRING(str_prev, "prev", 0xeb31683d, 0x7487ec947044729e)
#define STR_prev DeeString_STR(&str_prev)
#define Dee_HashStr__hasprev _Dee_HashSelectC(0xe7e8f3c, 0x17b364986c9ecd3b)
DEF_STRING(str_hasprev, "hasprev", 0xe7e8f3c, 0x17b364986c9ecd3b)
#define STR_hasprev DeeString_STR(&str_hasprev)
#define Dee_HashStr__hasnext _Dee_HashSelectC(0xae2186a8, 0x19d7bd95854b765f)
DEF_STRING(str_hasnext, "hasnext", 0xae2186a8, 0x19d7bd95854b765f)
#define STR_hasnext DeeString_STR(&str_hasnext)
#define Dee_HashStr__rewind _Dee_HashSelectC(0x2ab1b235, 0xa35b8bb3941ca25f)
DEF_STRING(str_rewind, "rewind", 0x2ab1b235, 0xa35b8bb3941ca25f)
#define STR_rewind DeeString_STR(&str_rewind)
#define Dee_HashStr__peek _Dee_HashSelectC(0xb2ae48a2, 0xcc667a4d924a91f8)
DEF_STRING(str_peek, "peek", 0xb2ae48a2, 0xcc667a4d924a91f8)
#define STR_peek DeeString_STR(&str_peek)
#define Dee_HashStr__format _Dee_HashSelectC(0x9ddde74c, 0xc575d5e219281e76)
DEF_STRING(str_format, "format", 0x9ddde74c, 0xc575d5e219281e76)
#define STR_format DeeString_STR(&str_format)
#define Dee_HashStr____format__ _Dee_HashSelectC(0xb59a1ae2, 0xdf14ed3788cde344)
DEF_STRING(str___format__, "__format__", 0xb59a1ae2, 0xdf14ed3788cde344)
#define STR___format__ DeeString_STR(&str___format__)
#define Dee_HashStr____name__ _Dee_HashSelectC(0x27a6cbdf, 0x9004f0806b170f3f)
DEF_STRING(str___name__, "__name__", 0x27a6cbdf, 0x9004f0806b170f3f)
#define STR___name__ DeeString_STR(&str___name__)
#define Dee_HashStr____doc__ _Dee_HashSelectC(0xd5eefba, 0x9e1c0e198ad451ff)
DEF_STRING(str___doc__, "__doc__", 0xd5eefba, 0x9e1c0e198ad451ff)
#define STR___doc__ DeeString_STR(&str___doc__)
#define Dee_HashStr____type__ _Dee_HashSelectC(0xc25dc337, 0xd3fa545616840a4e)
DEF_STRING(str___type__, "__type__", 0xc25dc337, 0xd3fa545616840a4e)
#define STR___type__ DeeString_STR(&str___type__)
#define Dee_HashStr____kwds__ _Dee_HashSelectC(0xd3926a14, 0xa90825b224a7262b)
DEF_STRING(str___kwds__, "__kwds__", 0xd3926a14, 0xa90825b224a7262b)
#define STR___kwds__ DeeString_STR(&str___kwds__)
#define Dee_HashStr____module__ _Dee_HashSelectC(0x3bea6c9f, 0x183a20d7d6c28dbb)
DEF_STRING(str___module__, "__module__", 0x3bea6c9f, 0x183a20d7d6c28dbb)
#define STR___module__ DeeString_STR(&str___module__)
#define Dee_HashStr__size _Dee_HashSelectC(0xed8917fa, 0x3fe8023bdf261c0f)
DEF_STRING(str_size, "size", 0xed8917fa, 0x3fe8023bdf261c0f)
#define STR_size DeeString_STR(&str_size)
#define Dee_HashStr__filename _Dee_HashSelectC(0x199d68d3, 0x4a5d0431e1a3caed)
DEF_STRING(str_filename, "filename", 0x199d68d3, 0x4a5d0431e1a3caed)
#define STR_filename DeeString_STR(&str_filename)
#define Dee_HashStr__tab _Dee_HashSelectC(0xe99b2213, 0x510da8cb03a6ba5d)
DEF_STRING(str_tab, "\t", 0xe99b2213, 0x510da8cb03a6ba5d)
#define STR_tab DeeString_STR(&str_tab)
#define Dee_HashStr__hasattr _Dee_HashSelectC(0xa37d5291, 0xad2ec658de4b00d3)
DEF_STRING(str_hasattr, "hasattr", 0xa37d5291, 0xad2ec658de4b00d3)
#define STR_hasattr DeeString_STR(&str_hasattr)
#define Dee_HashStr__hasitem _Dee_HashSelectC(0xfd5ab4fe, 0x754610f6171d3ff9)
DEF_STRING(str_hasitem, "hasitem", 0xfd5ab4fe, 0x754610f6171d3ff9)
#define STR_hasitem DeeString_STR(&str_hasitem)
#define Dee_HashStr__boundattr _Dee_HashSelectC(0xbe67bf95, 0x64616add8dce0b74)
DEF_STRING(str_boundattr, "boundattr", 0xbe67bf95, 0x64616add8dce0b74)
#define STR_boundattr DeeString_STR(&str_boundattr)
#define Dee_HashStr__bounditem _Dee_HashSelectC(0xbe8d5e6f, 0x95383275aec67fa5)
DEF_STRING(str_bounditem, "bounditem", 0xbe8d5e6f, 0x95383275aec67fa5)
#define STR_bounditem DeeString_STR(&str_bounditem)
#define Dee_HashStr__compare _Dee_HashSelectC(0x84b4e5c0, 0x9165e5178389f3e4)
DEF_STRING(str_compare, "compare", 0x84b4e5c0, 0x9165e5178389f3e4)
#define STR_compare DeeString_STR(&str_compare)
#define Dee_HashStr__equals _Dee_HashSelectC(0xcf48fdb6, 0x8ff48babe6a36c10)
DEF_STRING(str_equals, "equals", 0xcf48fdb6, 0x8ff48babe6a36c10)
#define STR_equals DeeString_STR(&str_equals)
#define Dee_HashStr____pooad _Dee_HashSelectC(0x38ba68c9, 0xd5562c36880fcfa0)
DEF_STRING(str___pooad, "__pooad", 0x38ba68c9, 0xd5562c36880fcfa0)
#define STR___pooad DeeString_STR(&str___pooad)
#define Dee_HashStr____neosb _Dee_HashSelectC(0x3a5bef0, 0x18de2c5f371aa921)
DEF_STRING(str___neosb, "__neosb", 0x3a5bef0, 0x18de2c5f371aa921)
#define STR___neosb DeeString_STR(&str___neosb)
#define Dee_HashStr____giosi _Dee_HashSelectC(0x30dd03a8, 0x347263f7fbfcea2b)
DEF_STRING(str___giosi, "__giosi", 0x30dd03a8, 0x347263f7fbfcea2b)
#define STR___giosi DeeString_STR(&str___giosi)
#define Dee_HashStr____grosr _Dee_HashSelectC(0xee715568, 0x4b4bdaef29d9b42b)
DEF_STRING(str___grosr, "__grosr", 0xee715568, 0x4b4bdaef29d9b42b)
#define STR___grosr DeeString_STR(&str___grosr)
#define Dee_HashStr____gaosa _Dee_HashSelectC(0xbdd31637, 0x31b6aef35b2a1f6)
DEF_STRING(str___gaosa, "__gaosa", 0xbdd31637, 0x31b6aef35b2a1f6)
#define STR___gaosa DeeString_STR(&str___gaosa)
#define Dee_HashStr____roloc _Dee_HashSelectC(0xf4874267, 0xa00f637868f0b44e)
DEF_STRING(str___roloc, "__roloc", 0xf4874267, 0xa00f637868f0b44e)
#define STR___roloc DeeString_STR(&str___roloc)
#define Dee_HashStr____assert _Dee_HashSelectC(0xd4715fbd, 0xdf7c220c44eeb5a4)
DEF_STRING(str___assert, "__assert", 0xd4715fbd, 0xdf7c220c44eeb5a4)
#define STR___assert DeeString_STR(&str___assert)
#define Dee_HashStr____badcall _Dee_HashSelectC(0x9795b98b, 0xc9e3cd8eadb2ee72)
DEF_STRING(str___badcall, "__badcall", 0x9795b98b, 0xc9e3cd8eadb2ee72)
#define STR___badcall DeeString_STR(&str___badcall)
#define Dee_HashStr__rt_d200 _Dee_HashSelectC(0x6b64a89c, 0x798d95b52acb68ab)
DEF_STRING(str_rt_d200, "rt.d200", 0x6b64a89c, 0x798d95b52acb68ab)
#define STR_rt_d200 DeeString_STR(&str_rt_d200)
#define Dee_HashStr__lt_anonymous_gr _Dee_HashSelectC(0xd5f7311c, 0x536d809a7d1c1aab)
DEF_STRING(str_lt_anonymous_gr, "<anonymous>", 0xd5f7311c, 0x536d809a7d1c1aab)
#define STR_lt_anonymous_gr DeeString_STR(&str_lt_anonymous_gr)
#define Dee_HashStr__nomemory _Dee_HashSelectC(0xbef65010, 0x6315c4e658da5e37)
DEF_STRING(str_nomemory, "allocation failed", 0xbef65010, 0x6315c4e658da5e37)
#define STR_nomemory DeeString_STR(&str_nomemory)
#define Dee_HashStr__dots _Dee_HashSelectC(0x1a086252, 0xf5eff0465042ef13)
DEF_STRING(str_dots, "...", 0x1a086252, 0xf5eff0465042ef13)
#define STR_dots DeeString_STR(&str_dots)
#define Dee_HashStr__constexpr _Dee_HashSelectC(0x58c650ca, 0xf0a0c2270200c1f0)
DEF_STRING(str_constexpr, "constexpr", 0x58c650ca, 0xf0a0c2270200c1f0)
#define STR_constexpr DeeString_STR(&str_constexpr)
#define Dee_HashStr__sym _Dee_HashSelectC(0x17fd993c, 0x1c0e6e19c328844b)
DEF_STRING(str_sym, "sym", 0x17fd993c, 0x1c0e6e19c328844b)
#define STR_sym DeeString_STR(&str_sym)
#define Dee_HashStr__unbind _Dee_HashSelectC(0x5a30901a, 0xa4d534ebcf828a7a)
DEF_STRING(str_unbind, "unbind", 0x5a30901a, 0xa4d534ebcf828a7a)
#define STR_unbind DeeString_STR(&str_unbind)
#define Dee_HashStr__bound _Dee_HashSelectC(0x5d3fd85e, 0x9200603171c0f3b0)
DEF_STRING(str_bound, "bound", 0x5d3fd85e, 0x9200603171c0f3b0)
#define STR_bound DeeString_STR(&str_bound)
#define Dee_HashStr__multiple _Dee_HashSelectC(0x298dfbdb, 0x28f61f03af08a3ca)
DEF_STRING(str_multiple, "multiple", 0x298dfbdb, 0x28f61f03af08a3ca)
#define STR_multiple DeeString_STR(&str_multiple)
#define Dee_HashStr__return _Dee_HashSelectC(0x553dfc89, 0x2a644c3d30df4872)
DEF_STRING(str_return, "return", 0x553dfc89, 0x2a644c3d30df4872)
#define STR_return DeeString_STR(&str_return)
#define Dee_HashStr__yield _Dee_HashSelectC(0x96c58e93, 0x17e5134b02b40024)
DEF_STRING(str_yield, "yield", 0x96c58e93, 0x17e5134b02b40024)
#define STR_yield DeeString_STR(&str_yield)
#define Dee_HashStr__throw _Dee_HashSelectC(0x17f397c6, 0x1267633ecc33c2c)
DEF_STRING(str_throw, "throw", 0x17f397c6, 0x1267633ecc33c2c)
#define STR_throw DeeString_STR(&str_throw)
#define Dee_HashStr__try _Dee_HashSelectC(0xb3e4fc45, 0xeb7c790414e244ab)
DEF_STRING(str_try, "try", 0xb3e4fc45, 0xeb7c790414e244ab)
#define STR_try DeeString_STR(&str_try)
#define Dee_HashStr__loop _Dee_HashSelectC(0xe44e70af, 0xadc137e48b7293ee)
DEF_STRING(str_loop, "loop", 0xe44e70af, 0xadc137e48b7293ee)
#define STR_loop DeeString_STR(&str_loop)
#define Dee_HashStr__loopctl _Dee_HashSelectC(0xa389d8bc, 0x2d55b845bc0a547d)
DEF_STRING(str_loopctl, "loopctl", 0xa389d8bc, 0x2d55b845bc0a547d)
#define STR_loopctl DeeString_STR(&str_loopctl)
#define Dee_HashStr__conditional _Dee_HashSelectC(0x4445d225, 0xb0965c593399d415)
DEF_STRING(str_conditional, "conditional", 0x4445d225, 0xb0965c593399d415)
#define STR_conditional DeeString_STR(&str_conditional)
#define Dee_HashStr__expand _Dee_HashSelectC(0x7708077c, 0x793fc0a371216076)
DEF_STRING(str_expand, "expand", 0x7708077c, 0x793fc0a371216076)
#define STR_expand DeeString_STR(&str_expand)
#define Dee_HashStr__function _Dee_HashSelectC(0x3d75dc7a, 0xf36d912a52403931)
DEF_STRING(str_function, "function", 0x3d75dc7a, 0xf36d912a52403931)
#define STR_function DeeString_STR(&str_function)
#define Dee_HashStr__operatorfunc _Dee_HashSelectC(0x4c528db6, 0x4cdc8fe9282f4a88)
DEF_STRING(str_operatorfunc, "operatorfunc", 0x4c528db6, 0x4cdc8fe9282f4a88)
#define STR_operatorfunc DeeString_STR(&str_operatorfunc)
#define Dee_HashStr__operator _Dee_HashSelectC(0xa5f184dd, 0x970564e28323bb4)
DEF_STRING(str_operator, "operator", 0xa5f184dd, 0x970564e28323bb4)
#define STR_operator DeeString_STR(&str_operator)
#define Dee_HashStr__action _Dee_HashSelectC(0x3679cc25, 0x4cca7f022ef68151)
DEF_STRING(str_action, "action", 0x3679cc25, 0x4cca7f022ef68151)
#define STR_action DeeString_STR(&str_action)
#define Dee_HashStr__class _Dee_HashSelectC(0xf769f29e, 0xc5397fe5c82dbd12)
DEF_STRING(str_class, "class", 0xf769f29e, 0xc5397fe5c82dbd12)
#define STR_class DeeString_STR(&str_class)
#define Dee_HashStr__label _Dee_HashSelectC(0x8c7dd24d, 0x10ab80c11491baad)
DEF_STRING(str_label, "label", 0x8c7dd24d, 0x10ab80c11491baad)
#define STR_label DeeString_STR(&str_label)
#define Dee_HashStr__goto _Dee_HashSelectC(0x6e4777d3, 0x75d33f290274060e)
DEF_STRING(str_goto, "goto", 0x6e4777d3, 0x75d33f290274060e)
#define STR_goto DeeString_STR(&str_goto)
#define Dee_HashStr__switch _Dee_HashSelectC(0xfb63ec5e, 0x70bce0e305461cce)
DEF_STRING(str_switch, "switch", 0xfb63ec5e, 0x70bce0e305461cce)
#define STR_switch DeeString_STR(&str_switch)
#define Dee_HashStr__assembly _Dee_HashSelectC(0x88542c16, 0x9d1c909a08c788a3)
DEF_STRING(str_assembly, "assembly", 0x88542c16, 0x9d1c909a08c788a3)
#define STR_assembly DeeString_STR(&str_assembly)
#define Dee_HashStr__global _Dee_HashSelectC(0x42ebe077, 0x4b19ecd5b61b5296)
DEF_STRING(str_global, "global", 0x42ebe077, 0x4b19ecd5b61b5296)
#define STR_global DeeString_STR(&str_global)
#define Dee_HashStr__extern _Dee_HashSelectC(0xc801935e, 0x8b47f0a1d76ecba3)
DEF_STRING(str_extern, "extern", 0xc801935e, 0x8b47f0a1d76ecba3)
#define STR_extern DeeString_STR(&str_extern)
#define Dee_HashStr__module _Dee_HashSelectC(0xae3684a4, 0xbb78a82535e5801e)
DEF_STRING(str_module, "module", 0xae3684a4, 0xbb78a82535e5801e)
#define STR_module DeeString_STR(&str_module)
#define Dee_HashStr__mymod _Dee_HashSelectC(0x85c64d4c, 0xf4aa94c3b47cfe72)
DEF_STRING(str_mymod, "mymod", 0x85c64d4c, 0xf4aa94c3b47cfe72)
#define STR_mymod DeeString_STR(&str_mymod)
#define Dee_HashStr__getset _Dee_HashSelectC(0x4be0a05b, 0xd5b87464b9cb7503)
DEF_STRING(str_getset, "getset", 0x4be0a05b, 0xd5b87464b9cb7503)
#define STR_getset DeeString_STR(&str_getset)
#define Dee_HashStr__ifield _Dee_HashSelectC(0xce5e0d5c, 0x86dab707bb32c291)
DEF_STRING(str_ifield, "ifield", 0xce5e0d5c, 0x86dab707bb32c291)
#define STR_ifield DeeString_STR(&str_ifield)
#define Dee_HashStr__cfield _Dee_HashSelectC(0x12ffac49, 0xb7298fd50b333835)
DEF_STRING(str_cfield, "cfield", 0x12ffac49, 0xb7298fd50b333835)
#define STR_cfield DeeString_STR(&str_cfield)
#define Dee_HashStr__alias _Dee_HashSelectC(0xca8b8108, 0x5826aeb387c6da71)
DEF_STRING(str_alias, "alias", 0xca8b8108, 0x5826aeb387c6da71)
#define STR_alias DeeString_STR(&str_alias)
#define Dee_HashStr__arg _Dee_HashSelectC(0xd22385c4, 0x72e1ee0d651e56d8)
DEF_STRING(str_arg, "arg", 0xd22385c4, 0x72e1ee0d651e56d8)
#define STR_arg DeeString_STR(&str_arg)
#define Dee_HashStr__local _Dee_HashSelectC(0x1d3dfa1, 0x3ede3ae259b4a7d2)
DEF_STRING(str_local, "local", 0x1d3dfa1, 0x3ede3ae259b4a7d2)
#define STR_local DeeString_STR(&str_local)
#define Dee_HashStr__stack _Dee_HashSelectC(0x30b318, 0xd084e637b870262b)
DEF_STRING(str_stack, "stack", 0x30b318, 0xd084e637b870262b)
#define STR_stack DeeString_STR(&str_stack)
#define Dee_HashStr__static _Dee_HashSelectC(0x6758a24b, 0x9f5751e5f08d205d)
DEF_STRING(str_static, "static", 0x6758a24b, 0x9f5751e5f08d205d)
#define STR_static DeeString_STR(&str_static)
#define Dee_HashStr__except _Dee_HashSelectC(0x8aae072b, 0xac70487e4861a6f3)
DEF_STRING(str_except, "except", 0x8aae072b, 0xac70487e4861a6f3)
#define STR_except DeeString_STR(&str_except)
#define Dee_HashStr__myfunc _Dee_HashSelectC(0x67667409, 0x45b51811d7de12b2)
DEF_STRING(str_myfunc, "myfunc", 0x67667409, 0x45b51811d7de12b2)
#define STR_myfunc DeeString_STR(&str_myfunc)
#define Dee_HashStr__this _Dee_HashSelectC(0x142984e, 0x19d213c79c35abf3)
DEF_STRING(str_this, "this", 0x142984e, 0x19d213c79c35abf3)
#define STR_this DeeString_STR(&str_this)
#define Dee_HashStr__ambig _Dee_HashSelectC(0x963ab94d, 0x62fe7ee447f767bd)
DEF_STRING(str_ambig, "ambig", 0x963ab94d, 0x62fe7ee447f767bd)
#define STR_ambig DeeString_STR(&str_ambig)
#define Dee_HashStr__fwd _Dee_HashSelectC(0x4d05936a, 0x468b9d355ef7e041)
DEF_STRING(str_fwd, "fwd", 0x4d05936a, 0x468b9d355ef7e041)
#define STR_fwd DeeString_STR(&str_fwd)
#define Dee_HashStr__const _Dee_HashSelectC(0x95daec48, 0x2e9cb1cd0ec552da)
DEF_STRING(str_const, "const", 0x95daec48, 0x2e9cb1cd0ec552da)
#define STR_const DeeString_STR(&str_const)
#define Dee_HashStr__No_active_exception _Dee_HashSelectC(0xa6c0d0c8, 0x132615532aa858b6)
DEF_STRING(str_No_active_exception, "No active exception", 0xa6c0d0c8, 0x132615532aa858b6)
#define STR_No_active_exception DeeString_STR(&str_No_active_exception)
#define Dee_HashStr____seq_bool__ _Dee_HashSelectC(0x385ee23, 0x45c831bf3a70ded9)
DEF_STRING(str___seq_bool__, "__seq_bool__", 0x385ee23, 0x45c831bf3a70ded9)
#define STR___seq_bool__ DeeString_STR(&str___seq_bool__)
#define Dee_HashStr____seq_size__ _Dee_HashSelectC(0xa1c99855, 0xdd68fbbf0a1a68a1)
DEF_STRING(str___seq_size__, "__seq_size__", 0xa1c99855, 0xdd68fbbf0a1a68a1)
#define STR___seq_size__ DeeString_STR(&str___seq_size__)
#define Dee_HashStr____seq_iter__ _Dee_HashSelectC(0x2fb16a47, 0x49d80da3961f157e)
DEF_STRING(str___seq_iter__, "__seq_iter__", 0x2fb16a47, 0x49d80da3961f157e)
#define STR___seq_iter__ DeeString_STR(&str___seq_iter__)
#define Dee_HashStr____seq_getitem__ _Dee_HashSelectC(0x4c346166, 0x8b3bf00bdee10ba0)
DEF_STRING(str___seq_getitem__, "__seq_getitem__", 0x4c346166, 0x8b3bf00bdee10ba0)
#define STR___seq_getitem__ DeeString_STR(&str___seq_getitem__)
#define Dee_HashStr____seq_delitem__ _Dee_HashSelectC(0x3b8e2105, 0x11a0e21507457e51)
DEF_STRING(str___seq_delitem__, "__seq_delitem__", 0x3b8e2105, 0x11a0e21507457e51)
#define STR___seq_delitem__ DeeString_STR(&str___seq_delitem__)
#define Dee_HashStr____seq_setitem__ _Dee_HashSelectC(0x60928657, 0x54b9696d799c0ea8)
DEF_STRING(str___seq_setitem__, "__seq_setitem__", 0x60928657, 0x54b9696d799c0ea8)
#define STR___seq_setitem__ DeeString_STR(&str___seq_setitem__)
#define Dee_HashStr____seq_getrange__ _Dee_HashSelectC(0x705f1d4b, 0x5978059426c1c6d2)
DEF_STRING(str___seq_getrange__, "__seq_getrange__", 0x705f1d4b, 0x5978059426c1c6d2)
#define STR___seq_getrange__ DeeString_STR(&str___seq_getrange__)
#define Dee_HashStr____seq_delrange__ _Dee_HashSelectC(0x21e5566, 0xdc5706e8f19d6eea)
DEF_STRING(str___seq_delrange__, "__seq_delrange__", 0x21e5566, 0xdc5706e8f19d6eea)
#define STR___seq_delrange__ DeeString_STR(&str___seq_delrange__)
#define Dee_HashStr____seq_setrange__ _Dee_HashSelectC(0x57041adb, 0x27f8dd423284ce91)
DEF_STRING(str___seq_setrange__, "__seq_setrange__", 0x57041adb, 0x27f8dd423284ce91)
#define STR___seq_setrange__ DeeString_STR(&str___seq_setrange__)
#define Dee_HashStr____seq_assign__ _Dee_HashSelectC(0xd4de11e1, 0x9b9726754969edfe)
DEF_STRING(str___seq_assign__, "__seq_assign__", 0xd4de11e1, 0x9b9726754969edfe)
#define STR___seq_assign__ DeeString_STR(&str___seq_assign__)
#define Dee_HashStr____seq_hash__ _Dee_HashSelectC(0x7c9aba93, 0xa285aa7898698c52)
DEF_STRING(str___seq_hash__, "__seq_hash__", 0x7c9aba93, 0xa285aa7898698c52)
#define STR___seq_hash__ DeeString_STR(&str___seq_hash__)
#define Dee_HashStr____seq_compare__ _Dee_HashSelectC(0xd06efa37, 0x1f9459ac2c6feba4)
DEF_STRING(str___seq_compare__, "__seq_compare__", 0xd06efa37, 0x1f9459ac2c6feba4)
#define STR___seq_compare__ DeeString_STR(&str___seq_compare__)
#define Dee_HashStr____seq_compare_eq__ _Dee_HashSelectC(0xa3f084a6, 0x857f75efb347d0ac)
DEF_STRING(str___seq_compare_eq__, "__seq_compare_eq__", 0xa3f084a6, 0x857f75efb347d0ac)
#define STR___seq_compare_eq__ DeeString_STR(&str___seq_compare_eq__)
#define Dee_HashStr____seq_eq__ _Dee_HashSelectC(0xf8ecf7d5, 0xaf5f106254d65f83)
DEF_STRING(str___seq_eq__, "__seq_eq__", 0xf8ecf7d5, 0xaf5f106254d65f83)
#define STR___seq_eq__ DeeString_STR(&str___seq_eq__)
#define Dee_HashStr____seq_ne__ _Dee_HashSelectC(0x507d96b7, 0xed7d4079819f9114)
DEF_STRING(str___seq_ne__, "__seq_ne__", 0x507d96b7, 0xed7d4079819f9114)
#define STR___seq_ne__ DeeString_STR(&str___seq_ne__)
#define Dee_HashStr____seq_lo__ _Dee_HashSelectC(0xf25cdb01, 0x646b4a6d46be1da7)
DEF_STRING(str___seq_lo__, "__seq_lo__", 0xf25cdb01, 0x646b4a6d46be1da7)
#define STR___seq_lo__ DeeString_STR(&str___seq_lo__)
#define Dee_HashStr____seq_le__ _Dee_HashSelectC(0x2352e2a4, 0x9451a774e98e2630)
DEF_STRING(str___seq_le__, "__seq_le__", 0x2352e2a4, 0x9451a774e98e2630)
#define STR___seq_le__ DeeString_STR(&str___seq_le__)
#define Dee_HashStr____seq_gr__ _Dee_HashSelectC(0x7803c9, 0x402d902d292b4105)
DEF_STRING(str___seq_gr__, "__seq_gr__", 0x7803c9, 0x402d902d292b4105)
#define STR___seq_gr__ DeeString_STR(&str___seq_gr__)
#define Dee_HashStr____seq_ge__ _Dee_HashSelectC(0xa1fbc092, 0x6717358f09330a99)
DEF_STRING(str___seq_ge__, "__seq_ge__", 0xa1fbc092, 0x6717358f09330a99)
#define STR___seq_ge__ DeeString_STR(&str___seq_ge__)
#define Dee_HashStr____seq_add__ _Dee_HashSelectC(0xfabd2799, 0x1aca855ff1f30c1d)
DEF_STRING(str___seq_add__, "__seq_add__", 0xfabd2799, 0x1aca855ff1f30c1d)
#define STR___seq_add__ DeeString_STR(&str___seq_add__)
#define Dee_HashStr____seq_mul__ _Dee_HashSelectC(0x36e749ff, 0x78910f4cca503d4e)
DEF_STRING(str___seq_mul__, "__seq_mul__", 0x36e749ff, 0x78910f4cca503d4e)
#define STR___seq_mul__ DeeString_STR(&str___seq_mul__)
#define Dee_HashStr____seq_inplace_add__ _Dee_HashSelectC(0x9de83f91, 0x93e6c1c0cbd77d91)
DEF_STRING(str___seq_inplace_add__, "__seq_inplace_add__", 0x9de83f91, 0x93e6c1c0cbd77d91)
#define STR___seq_inplace_add__ DeeString_STR(&str___seq_inplace_add__)
#define Dee_HashStr____seq_inplace_mul__ _Dee_HashSelectC(0x2091bae6, 0xddf45474ad12649)
DEF_STRING(str___seq_inplace_mul__, "__seq_inplace_mul__", 0x2091bae6, 0xddf45474ad12649)
#define STR___seq_inplace_mul__ DeeString_STR(&str___seq_inplace_mul__)
#define Dee_HashStr____seq_enumerate__ _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247)
DEF_STRING(str___seq_enumerate__, "__seq_enumerate__", 0x2daa9e99, 0xca96f5a312eda247)
#define STR___seq_enumerate__ DeeString_STR(&str___seq_enumerate__)
#define Dee_HashStr____seq_enumerate_items__ _Dee_HashSelectC(0x657daa4b, 0x490569f74ec83bbf)
DEF_STRING(str___seq_enumerate_items__, "__seq_enumerate_items__", 0x657daa4b, 0x490569f74ec83bbf)
#define STR___seq_enumerate_items__ DeeString_STR(&str___seq_enumerate_items__)
#define Dee_HashStr__unpack _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9)
DEF_STRING(str_unpack, "unpack", 0x579a82d1, 0x2714bac446bf9c9)
#define STR_unpack DeeString_STR(&str_unpack)
#define Dee_HashStr____seq_unpack__ _Dee_HashSelectC(0x37857c93, 0xe09c2983d8662e7d)
DEF_STRING(str___seq_unpack__, "__seq_unpack__", 0x37857c93, 0xe09c2983d8662e7d)
#define STR___seq_unpack__ DeeString_STR(&str___seq_unpack__)
#define Dee_HashStr__unpackub _Dee_HashSelectC(0x7ed69d27, 0x1cc0c8cb043c32ce)
DEF_STRING(str_unpackub, "unpackub", 0x7ed69d27, 0x1cc0c8cb043c32ce)
#define STR_unpackub DeeString_STR(&str_unpackub)
#define Dee_HashStr____seq_unpackub__ _Dee_HashSelectC(0x95f8a84e, 0x51662d156144f8cd)
DEF_STRING(str___seq_unpackub__, "__seq_unpackub__", 0x95f8a84e, 0x51662d156144f8cd)
#define STR___seq_unpackub__ DeeString_STR(&str___seq_unpackub__)
#define Dee_HashStr__first _Dee_HashSelectC(0xa9f0e818, 0x9d12a485470a29a7)
DEF_STRING(str_first, "first", 0xa9f0e818, 0x9d12a485470a29a7)
#define STR_first DeeString_STR(&str_first)
#define Dee_HashStr____seq_first__ _Dee_HashSelectC(0xba07bcf8, 0xf64baada0fea7f04)
DEF_STRING(str___seq_first__, "__seq_first__", 0xba07bcf8, 0xf64baada0fea7f04)
#define STR___seq_first__ DeeString_STR(&str___seq_first__)
#define Dee_HashStr__last _Dee_HashSelectC(0x185a4f9a, 0x760894ca6d41e4dc)
DEF_STRING(str_last, "last", 0x185a4f9a, 0x760894ca6d41e4dc)
#define STR_last DeeString_STR(&str_last)
#define Dee_HashStr____seq_last__ _Dee_HashSelectC(0x2bf501b7, 0x87a676ceeac240a5)
DEF_STRING(str___seq_last__, "__seq_last__", 0x2bf501b7, 0x87a676ceeac240a5)
#define STR___seq_last__ DeeString_STR(&str___seq_last__)
#define Dee_HashStr__cached _Dee_HashSelectC(0x915e175e, 0xddfd408a14eae4b4)
DEF_STRING(str_cached, "cached", 0x915e175e, 0xddfd408a14eae4b4)
#define STR_cached DeeString_STR(&str_cached)
#define Dee_HashStr____seq_cached__ _Dee_HashSelectC(0x13338d67, 0x497ba149d490b121)
DEF_STRING(str___seq_cached__, "__seq_cached__", 0x13338d67, 0x497ba149d490b121)
#define STR___seq_cached__ DeeString_STR(&str___seq_cached__)
#define Dee_HashStr__frozen _Dee_HashSelectC(0x82311b77, 0x7b55e2e6e642b6fd)
DEF_STRING(str_frozen, "frozen", 0x82311b77, 0x7b55e2e6e642b6fd)
#define STR_frozen DeeString_STR(&str_frozen)
#define Dee_HashStr____seq_frozen__ _Dee_HashSelectC(0xf0763bf2, 0x3af9b99de3330ab)
DEF_STRING(str___seq_frozen__, "__seq_frozen__", 0xf0763bf2, 0x3af9b99de3330ab)
#define STR___seq_frozen__ DeeString_STR(&str___seq_frozen__)
#define Dee_HashStr__any _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef)
DEF_STRING(str_any, "any", 0x8125d3c6, 0xb4b817103f9d84ef)
#define STR_any DeeString_STR(&str_any)
#define Dee_HashStr____seq_any__ _Dee_HashSelectC(0xdbc18402, 0x809971dbd2fbe45)
DEF_STRING(str___seq_any__, "__seq_any__", 0xdbc18402, 0x809971dbd2fbe45)
#define STR___seq_any__ DeeString_STR(&str___seq_any__)
#define Dee_HashStr__all _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc)
DEF_STRING(str_all, "all", 0xb7e328b, 0xa812dcfcacc34ebc)
#define STR_all DeeString_STR(&str_all)
#define Dee_HashStr____seq_all__ _Dee_HashSelectC(0xd1aed67c, 0xba447c34c699709f)
DEF_STRING(str___seq_all__, "__seq_all__", 0xd1aed67c, 0xba447c34c699709f)
#define STR___seq_all__ DeeString_STR(&str___seq_all__)
#define Dee_HashStr__parity _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28)
DEF_STRING(str_parity, "parity", 0x92f3da0a, 0x254564db48674f28)
#define STR_parity DeeString_STR(&str_parity)
#define Dee_HashStr____seq_parity__ _Dee_HashSelectC(0xcda8343, 0x4134274571b0743b)
DEF_STRING(str___seq_parity__, "__seq_parity__", 0xcda8343, 0x4134274571b0743b)
#define STR___seq_parity__ DeeString_STR(&str___seq_parity__)
#define Dee_HashStr__reduce _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d)
DEF_STRING(str_reduce, "reduce", 0x907b2992, 0xb7e66094a8a9409d)
#define STR_reduce DeeString_STR(&str_reduce)
#define Dee_HashStr____seq_reduce__ _Dee_HashSelectC(0x212ddcd3, 0x2f18d73fc1a6b44a)
DEF_STRING(str___seq_reduce__, "__seq_reduce__", 0x212ddcd3, 0x2f18d73fc1a6b44a)
#define STR___seq_reduce__ DeeString_STR(&str___seq_reduce__)
#define Dee_HashStr__min _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca)
DEF_STRING(str_min, "min", 0xde957cfa, 0xe6ffc7365e2d00ca)
#define STR_min DeeString_STR(&str_min)
#define Dee_HashStr____seq_min__ _Dee_HashSelectC(0xaf4e340b, 0x94879a203902a8ca)
DEF_STRING(str___seq_min__, "__seq_min__", 0xaf4e340b, 0x94879a203902a8ca)
#define STR___seq_min__ DeeString_STR(&str___seq_min__)
#define Dee_HashStr__max _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850)
DEF_STRING(str_max, "max", 0xc293979b, 0x822bd5c706bd9850)
#define STR_max DeeString_STR(&str_max)
#define Dee_HashStr____seq_max__ _Dee_HashSelectC(0x670b4224, 0xd6ab9494e5312cec)
DEF_STRING(str___seq_max__, "__seq_max__", 0x670b4224, 0xd6ab9494e5312cec)
#define STR___seq_max__ DeeString_STR(&str___seq_max__)
#define Dee_HashStr__sum _Dee_HashSelectC(0xfdc548bb, 0x69e7b5a62f6dbc17)
DEF_STRING(str_sum, "sum", 0xfdc548bb, 0x69e7b5a62f6dbc17)
#define STR_sum DeeString_STR(&str_sum)
#define Dee_HashStr____seq_sum__ _Dee_HashSelectC(0x18e84724, 0xacd793c141691611)
DEF_STRING(str___seq_sum__, "__seq_sum__", 0x18e84724, 0xacd793c141691611)
#define STR___seq_sum__ DeeString_STR(&str___seq_sum__)
#define Dee_HashStr__count _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb)
DEF_STRING(str_count, "count", 0x54eac164, 0xbd66b5980d54babb)
#define STR_count DeeString_STR(&str_count)
#define Dee_HashStr____seq_count__ _Dee_HashSelectC(0x71175b03, 0xb54ae2ee1fe7d0c9)
DEF_STRING(str___seq_count__, "__seq_count__", 0x71175b03, 0xb54ae2ee1fe7d0c9)
#define STR___seq_count__ DeeString_STR(&str___seq_count__)
#define Dee_HashStr__contains _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209)
DEF_STRING(str_contains, "contains", 0x9338eec0, 0x1e0128373382a209)
#define STR_contains DeeString_STR(&str_contains)
#define Dee_HashStr____seq_contains__ _Dee_HashSelectC(0x9acccdfe, 0x991d55e25b1a82b4)
DEF_STRING(str___seq_contains__, "__seq_contains__", 0x9acccdfe, 0x991d55e25b1a82b4)
#define STR___seq_contains__ DeeString_STR(&str___seq_contains__)
#define Dee_HashStr__locate _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258)
DEF_STRING(str_locate, "locate", 0x72fc7691, 0xa3c641c56f3d6258)
#define STR_locate DeeString_STR(&str_locate)
#define Dee_HashStr____seq_locate__ _Dee_HashSelectC(0xb6ab1ac3, 0xe01f18461033d66c)
DEF_STRING(str___seq_locate__, "__seq_locate__", 0xb6ab1ac3, 0xe01f18461033d66c)
#define STR___seq_locate__ DeeString_STR(&str___seq_locate__)
#define Dee_HashStr__rlocate _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b)
DEF_STRING(str_rlocate, "rlocate", 0xe233056a, 0xf4759389157e74b)
#define STR_rlocate DeeString_STR(&str_rlocate)
#define Dee_HashStr____seq_rlocate__ _Dee_HashSelectC(0xf40f4306, 0x5e1336153e18b450)
DEF_STRING(str___seq_rlocate__, "__seq_rlocate__", 0xf40f4306, 0x5e1336153e18b450)
#define STR___seq_rlocate__ DeeString_STR(&str___seq_rlocate__)
#define Dee_HashStr__startswith _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d)
DEF_STRING(str_startswith, "startswith", 0x58e22b, 0x6251da2c5cdb654d)
#define STR_startswith DeeString_STR(&str_startswith)
#define Dee_HashStr____seq_startswith__ _Dee_HashSelectC(0xe2020e14, 0xda8ef94f534073e9)
DEF_STRING(str___seq_startswith__, "__seq_startswith__", 0xe2020e14, 0xda8ef94f534073e9)
#define STR___seq_startswith__ DeeString_STR(&str___seq_startswith__)
#define Dee_HashStr__endswith _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1)
DEF_STRING(str_endswith, "endswith", 0x8bdeff05, 0xca6abed3214345e1)
#define STR_endswith DeeString_STR(&str_endswith)
#define Dee_HashStr____seq_endswith__ _Dee_HashSelectC(0x7a3367e0, 0x67cfbe5dc4038c84)
DEF_STRING(str___seq_endswith__, "__seq_endswith__", 0x7a3367e0, 0x67cfbe5dc4038c84)
#define STR___seq_endswith__ DeeString_STR(&str___seq_endswith__)
#define Dee_HashStr__find _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2)
DEF_STRING(str_find, "find", 0x9e66372, 0x2b65fe03bbdde5b2)
#define STR_find DeeString_STR(&str_find)
#define Dee_HashStr____seq_find__ _Dee_HashSelectC(0x6efdc0ae, 0x527acacc0183caca)
DEF_STRING(str___seq_find__, "__seq_find__", 0x6efdc0ae, 0x527acacc0183caca)
#define STR___seq_find__ DeeString_STR(&str___seq_find__)
#define Dee_HashStr__rfind _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d)
DEF_STRING(str_rfind, "rfind", 0xfb368ca, 0x8b40ffa7172dc59d)
#define STR_rfind DeeString_STR(&str_rfind)
#define Dee_HashStr____seq_rfind__ _Dee_HashSelectC(0x6a572f0c, 0x93860c7078813ca)
DEF_STRING(str___seq_rfind__, "__seq_rfind__", 0x6a572f0c, 0x93860c7078813ca)
#define STR___seq_rfind__ DeeString_STR(&str___seq_rfind__)
#define Dee_HashStr__erase _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5)
DEF_STRING(str_erase, "erase", 0x6f5916cf, 0x65f9c8b6514af4e5)
#define STR_erase DeeString_STR(&str_erase)
#define Dee_HashStr____seq_erase__ _Dee_HashSelectC(0xf418fec, 0xaa4716e6a7c90a6e)
DEF_STRING(str___seq_erase__, "__seq_erase__", 0xf418fec, 0xaa4716e6a7c90a6e)
#define STR___seq_erase__ DeeString_STR(&str___seq_erase__)
#define Dee_HashStr__insert _Dee_HashSelectC(0x71d74a66, 0x5e168c86241590d7)
DEF_STRING(str_insert, "insert", 0x71d74a66, 0x5e168c86241590d7)
#define STR_insert DeeString_STR(&str_insert)
#define Dee_HashStr____seq_insert__ _Dee_HashSelectC(0x108c61ac, 0xe94b2ec29ead79d1)
DEF_STRING(str___seq_insert__, "__seq_insert__", 0x108c61ac, 0xe94b2ec29ead79d1)
#define STR___seq_insert__ DeeString_STR(&str___seq_insert__)
#define Dee_HashStr__insertall _Dee_HashSelectC(0xbf9bc3a9, 0x4f85971d093a27f2)
DEF_STRING(str_insertall, "insertall", 0xbf9bc3a9, 0x4f85971d093a27f2)
#define STR_insertall DeeString_STR(&str_insertall)
#define Dee_HashStr____seq_insertall__ _Dee_HashSelectC(0xae7ace11, 0x8510df225525bacc)
DEF_STRING(str___seq_insertall__, "__seq_insertall__", 0xae7ace11, 0x8510df225525bacc)
#define STR___seq_insertall__ DeeString_STR(&str___seq_insertall__)
#define Dee_HashStr__pushfront _Dee_HashSelectC(0xc682cfdf, 0x5933eb9a387ff882)
DEF_STRING(str_pushfront, "pushfront", 0xc682cfdf, 0x5933eb9a387ff882)
#define STR_pushfront DeeString_STR(&str_pushfront)
#define Dee_HashStr____seq_pushfront__ _Dee_HashSelectC(0xe30e92a5, 0x69ae18cfaba44b5a)
DEF_STRING(str___seq_pushfront__, "__seq_pushfront__", 0xe30e92a5, 0x69ae18cfaba44b5a)
#define STR___seq_pushfront__ DeeString_STR(&str___seq_pushfront__)
#define Dee_HashStr__append _Dee_HashSelectC(0x5f19594f, 0x8c2b7c1aba65d5ee)
DEF_STRING(str_append, "append", 0x5f19594f, 0x8c2b7c1aba65d5ee)
#define STR_append DeeString_STR(&str_append)
#define Dee_HashStr__pushback _Dee_HashSelectC(0xad1e1509, 0x4cfafd84a12923bd)
DEF_STRING(str_pushback, "pushback", 0xad1e1509, 0x4cfafd84a12923bd)
#define STR_pushback DeeString_STR(&str_pushback)
#define Dee_HashStr____seq_append__ _Dee_HashSelectC(0x43ea1331, 0x383f299606f81ebe)
DEF_STRING(str___seq_append__, "__seq_append__", 0x43ea1331, 0x383f299606f81ebe)
#define STR___seq_append__ DeeString_STR(&str___seq_append__)
#define Dee_HashStr__extend _Dee_HashSelectC(0x960b75e7, 0xba076858e3adb055)
DEF_STRING(str_extend, "extend", 0x960b75e7, 0xba076858e3adb055)
#define STR_extend DeeString_STR(&str_extend)
#define Dee_HashStr____seq_extend__ _Dee_HashSelectC(0x9bea054c, 0xe1bd8880fb31a9ee)
DEF_STRING(str___seq_extend__, "__seq_extend__", 0x9bea054c, 0xe1bd8880fb31a9ee)
#define STR___seq_extend__ DeeString_STR(&str___seq_extend__)
#define Dee_HashStr__xchitem _Dee_HashSelectC(0xc89decce, 0x16e81f00d8d95d57)
DEF_STRING(str_xchitem, "xchitem", 0xc89decce, 0x16e81f00d8d95d57)
#define STR_xchitem DeeString_STR(&str_xchitem)
#define Dee_HashStr____seq_xchitem__ _Dee_HashSelectC(0x4eea0d1, 0x6238b16fe217a6ed)
DEF_STRING(str___seq_xchitem__, "__seq_xchitem__", 0x4eea0d1, 0x6238b16fe217a6ed)
#define STR___seq_xchitem__ DeeString_STR(&str___seq_xchitem__)
#define Dee_HashStr__clear _Dee_HashSelectC(0x7857faae, 0x22a34b6f82b3b83c)
DEF_STRING(str_clear, "clear", 0x7857faae, 0x22a34b6f82b3b83c)
#define STR_clear DeeString_STR(&str_clear)
#define Dee_HashStr____seq_clear__ _Dee_HashSelectC(0x9d9e937d, 0xfdca7540ed524b50)
DEF_STRING(str___seq_clear__, "__seq_clear__", 0x9d9e937d, 0xfdca7540ed524b50)
#define STR___seq_clear__ DeeString_STR(&str___seq_clear__)
#define Dee_HashStr__pop _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb)
DEF_STRING(str_pop, "pop", 0x960361ff, 0x666fb01461b0a0eb)
#define STR_pop DeeString_STR(&str_pop)
#define Dee_HashStr____seq_pop__ _Dee_HashSelectC(0xbc856b3, 0x292be45738029ef3)
DEF_STRING(str___seq_pop__, "__seq_pop__", 0xbc856b3, 0x292be45738029ef3)
#define STR___seq_pop__ DeeString_STR(&str___seq_pop__)
#define Dee_HashStr__remove _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a)
DEF_STRING(str_remove, "remove", 0x3d2727dd, 0xe9f313a03e2051a)
#define STR_remove DeeString_STR(&str_remove)
#define Dee_HashStr____seq_remove__ _Dee_HashSelectC(0xf3973265, 0xf66c3dc2f794b6f6)
DEF_STRING(str___seq_remove__, "__seq_remove__", 0xf3973265, 0xf66c3dc2f794b6f6)
#define STR___seq_remove__ DeeString_STR(&str___seq_remove__)
#define Dee_HashStr__rremove _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6)
DEF_STRING(str_rremove, "rremove", 0x37ef1152, 0x199975a7908f6d6)
#define STR_rremove DeeString_STR(&str_rremove)
#define Dee_HashStr____seq_rremove__ _Dee_HashSelectC(0x45433c7f, 0xb9a93a98d3d74233)
DEF_STRING(str___seq_rremove__, "__seq_rremove__", 0x45433c7f, 0xb9a93a98d3d74233)
#define STR___seq_rremove__ DeeString_STR(&str___seq_rremove__)
#define Dee_HashStr__removeall _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349)
DEF_STRING(str_removeall, "removeall", 0x902407ed, 0x97879af70abc9349)
#define STR_removeall DeeString_STR(&str_removeall)
#define Dee_HashStr____seq_removeall__ _Dee_HashSelectC(0x8b6a674f, 0xe2b4538c5c43da51)
DEF_STRING(str___seq_removeall__, "__seq_removeall__", 0x8b6a674f, 0xe2b4538c5c43da51)
#define STR___seq_removeall__ DeeString_STR(&str___seq_removeall__)
#define Dee_HashStr__removeif _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e)
DEF_STRING(str_removeif, "removeif", 0x156aa732, 0x96ad85f728d8a11e)
#define STR_removeif DeeString_STR(&str_removeif)
#define Dee_HashStr____seq_removeif__ _Dee_HashSelectC(0x304fcae9, 0x5c2fb6757251a6dd)
DEF_STRING(str___seq_removeif__, "__seq_removeif__", 0x304fcae9, 0x5c2fb6757251a6dd)
#define STR___seq_removeif__ DeeString_STR(&str___seq_removeif__)
#define Dee_HashStr__resize _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34)
DEF_STRING(str_resize, "resize", 0x36fcb308, 0x573f3d2e97212b34)
#define STR_resize DeeString_STR(&str_resize)
#define Dee_HashStr____seq_resize__ _Dee_HashSelectC(0x3f5efd3b, 0xbb48ab62bf1c52f9)
DEF_STRING(str___seq_resize__, "__seq_resize__", 0x3f5efd3b, 0xbb48ab62bf1c52f9)
#define STR___seq_resize__ DeeString_STR(&str___seq_resize__)
#define Dee_HashStr__fill _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4)
DEF_STRING(str_fill, "fill", 0xbd501461, 0x7b3ed649c1abacf4)
#define STR_fill DeeString_STR(&str_fill)
#define Dee_HashStr____seq_fill__ _Dee_HashSelectC(0x804baa4b, 0x8c022361158e60b5)
DEF_STRING(str___seq_fill__, "__seq_fill__", 0x804baa4b, 0x8c022361158e60b5)
#define STR___seq_fill__ DeeString_STR(&str___seq_fill__)
#define Dee_HashStr__reverse _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a)
DEF_STRING(str_reverse, "reverse", 0xd8d820f4, 0x278cbcd3a230313a)
#define STR_reverse DeeString_STR(&str_reverse)
#define Dee_HashStr____seq_reverse__ _Dee_HashSelectC(0x6b430a0f, 0x1b0d1d614c68adb6)
DEF_STRING(str___seq_reverse__, "__seq_reverse__", 0x6b430a0f, 0x1b0d1d614c68adb6)
#define STR___seq_reverse__ DeeString_STR(&str___seq_reverse__)
#define Dee_HashStr__reversed _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f)
DEF_STRING(str_reversed, "reversed", 0x78e8d950, 0xde2071a9b2ca405f)
#define STR_reversed DeeString_STR(&str_reversed)
#define Dee_HashStr____seq_reversed__ _Dee_HashSelectC(0xfc04d16d, 0x86f352c014e6952e)
DEF_STRING(str___seq_reversed__, "__seq_reversed__", 0xfc04d16d, 0x86f352c014e6952e)
#define STR___seq_reversed__ DeeString_STR(&str___seq_reversed__)
#define Dee_HashStr__sort _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1)
DEF_STRING(str_sort, "sort", 0xde7868af, 0x58835b3b7416f7f1)
#define STR_sort DeeString_STR(&str_sort)
#define Dee_HashStr____seq_sort__ _Dee_HashSelectC(0xada4b872, 0xd6269336d74985fa)
DEF_STRING(str___seq_sort__, "__seq_sort__", 0xada4b872, 0xd6269336d74985fa)
#define STR___seq_sort__ DeeString_STR(&str___seq_sort__)
#define Dee_HashStr__sorted _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860)
DEF_STRING(str_sorted, "sorted", 0x93fb6d4, 0x2fc60c43cfaf0860)
#define STR_sorted DeeString_STR(&str_sorted)
#define Dee_HashStr____seq_sorted__ _Dee_HashSelectC(0x5c289684, 0x5916b598f6f698c3)
DEF_STRING(str___seq_sorted__, "__seq_sorted__", 0x5c289684, 0x5916b598f6f698c3)
#define STR___seq_sorted__ DeeString_STR(&str___seq_sorted__)
#define Dee_HashStr__bfind _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7)
DEF_STRING(str_bfind, "bfind", 0xdb39cc6c, 0x5ec07aef149314c7)
#define STR_bfind DeeString_STR(&str_bfind)
#define Dee_HashStr____seq_bfind__ _Dee_HashSelectC(0x14f1087e, 0xf99331ec53fd5237)
DEF_STRING(str___seq_bfind__, "__seq_bfind__", 0x14f1087e, 0xf99331ec53fd5237)
#define STR___seq_bfind__ DeeString_STR(&str___seq_bfind__)
#define Dee_HashStr__bposition _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2)
DEF_STRING(str_bposition, "bposition", 0xba99f013, 0xc8f6389c9f293cb2)
#define STR_bposition DeeString_STR(&str_bposition)
#define Dee_HashStr____seq_bposition__ _Dee_HashSelectC(0x9405a8c, 0x3af79077b899240c)
DEF_STRING(str___seq_bposition__, "__seq_bposition__", 0x9405a8c, 0x3af79077b899240c)
#define STR___seq_bposition__ DeeString_STR(&str___seq_bposition__)
#define Dee_HashStr__brange _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2)
DEF_STRING(str_brange, "brange", 0xb132222e, 0xfed8bb16d0ac0dd2)
#define STR_brange DeeString_STR(&str_brange)
#define Dee_HashStr____seq_brange__ _Dee_HashSelectC(0xdce22547, 0x72bf9e7241379d7)
DEF_STRING(str___seq_brange__, "__seq_brange__", 0xdce22547, 0x72bf9e7241379d7)
#define STR___seq_brange__ DeeString_STR(&str___seq_brange__)
#define Dee_HashStr____set_iter__ _Dee_HashSelectC(0xbd188c7a, 0x5b5d9e6145ece3c4)
DEF_STRING(str___set_iter__, "__set_iter__", 0xbd188c7a, 0x5b5d9e6145ece3c4)
#define STR___set_iter__ DeeString_STR(&str___set_iter__)
#define Dee_HashStr____set_size__ _Dee_HashSelectC(0x9810fd1, 0x72462ed60fe76f39)
DEF_STRING(str___set_size__, "__set_size__", 0x9810fd1, 0x72462ed60fe76f39)
#define STR___set_size__ DeeString_STR(&str___set_size__)
#define Dee_HashStr____set_hash__ _Dee_HashSelectC(0xbc4a1737, 0xdac29ed35783587)
DEF_STRING(str___set_hash__, "__set_hash__", 0xbc4a1737, 0xdac29ed35783587)
#define STR___set_hash__ DeeString_STR(&str___set_hash__)
#define Dee_HashStr____set_compare_eq__ _Dee_HashSelectC(0x559b95bd, 0x79ecbfb17bdfb615)
DEF_STRING(str___set_compare_eq__, "__set_compare_eq__", 0x559b95bd, 0x79ecbfb17bdfb615)
#define STR___set_compare_eq__ DeeString_STR(&str___set_compare_eq__)
#define Dee_HashStr____set_eq__ _Dee_HashSelectC(0x1a668259, 0x517690c0f0238162)
DEF_STRING(str___set_eq__, "__set_eq__", 0x1a668259, 0x517690c0f0238162)
#define STR___set_eq__ DeeString_STR(&str___set_eq__)
#define Dee_HashStr____set_ne__ _Dee_HashSelectC(0x6c1e8688, 0xef0260a014a3a89c)
DEF_STRING(str___set_ne__, "__set_ne__", 0x6c1e8688, 0xef0260a014a3a89c)
#define STR___set_ne__ DeeString_STR(&str___set_ne__)
#define Dee_HashStr____set_lo__ _Dee_HashSelectC(0x192ae081, 0x26f27730229e1e)
DEF_STRING(str___set_lo__, "__set_lo__", 0x192ae081, 0x26f27730229e1e)
#define STR___set_lo__ DeeString_STR(&str___set_lo__)
#define Dee_HashStr__issubset _Dee_HashSelectC(0xac6aa1c0, 0x49ece9bed26428cf)
DEF_STRING(str_issubset, "issubset", 0xac6aa1c0, 0x49ece9bed26428cf)
#define STR_issubset DeeString_STR(&str_issubset)
#define Dee_HashStr____set_le__ _Dee_HashSelectC(0xf82dac68, 0x1edea9fde6d1891a)
DEF_STRING(str___set_le__, "__set_le__", 0xf82dac68, 0x1edea9fde6d1891a)
#define STR___set_le__ DeeString_STR(&str___set_le__)
#define Dee_HashStr____set_gr__ _Dee_HashSelectC(0x50e1afbc, 0x146ab10a7b327a34)
DEF_STRING(str___set_gr__, "__set_gr__", 0x50e1afbc, 0x146ab10a7b327a34)
#define STR___set_gr__ DeeString_STR(&str___set_gr__)
#define Dee_HashStr__issuperset _Dee_HashSelectC(0x55780f5f, 0x7f578be05d081a7f)
DEF_STRING(str_issuperset, "issuperset", 0x55780f5f, 0x7f578be05d081a7f)
#define STR_issuperset DeeString_STR(&str_issuperset)
#define Dee_HashStr____set_ge__ _Dee_HashSelectC(0xbe017980, 0x72b0e6d930f8cda)
DEF_STRING(str___set_ge__, "__set_ge__", 0xbe017980, 0x72b0e6d930f8cda)
#define STR___set_ge__ DeeString_STR(&str___set_ge__)
#define Dee_HashStr____set_inv__ _Dee_HashSelectC(0x95bdcbff, 0x5ec843324068dcae)
DEF_STRING(str___set_inv__, "__set_inv__", 0x95bdcbff, 0x5ec843324068dcae)
#define STR___set_inv__ DeeString_STR(&str___set_inv__)
#define Dee_HashStr__union _Dee_HashSelectC(0x23b88b9b, 0x3b416e7d690babb2)
DEF_STRING(str_union, "union", 0x23b88b9b, 0x3b416e7d690babb2)
#define STR_union DeeString_STR(&str_union)
#define Dee_HashStr____set_add__ _Dee_HashSelectC(0xa0f90ae6, 0xfd0cb80b82558f78)
DEF_STRING(str___set_add__, "__set_add__", 0xa0f90ae6, 0xfd0cb80b82558f78)
#define STR___set_add__ DeeString_STR(&str___set_add__)
#define Dee_HashStr__difference _Dee_HashSelectC(0xe944baff, 0x6add28d83d2e1f6e)
DEF_STRING(str_difference, "difference", 0xe944baff, 0x6add28d83d2e1f6e)
#define STR_difference DeeString_STR(&str_difference)
#define Dee_HashStr____set_sub__ _Dee_HashSelectC(0x11a9c48, 0xdb604349181c79d5)
DEF_STRING(str___set_sub__, "__set_sub__", 0x11a9c48, 0xdb604349181c79d5)
#define STR___set_sub__ DeeString_STR(&str___set_sub__)
#define Dee_HashStr__intersection _Dee_HashSelectC(0xabaa0afa, 0xc72d025e185198b7)
DEF_STRING(str_intersection, "intersection", 0xabaa0afa, 0xc72d025e185198b7)
#define STR_intersection DeeString_STR(&str_intersection)
#define Dee_HashStr____set_and__ _Dee_HashSelectC(0x4b12fb6a, 0x25ba13e3e8334037)
DEF_STRING(str___set_and__, "__set_and__", 0x4b12fb6a, 0x25ba13e3e8334037)
#define STR___set_and__ DeeString_STR(&str___set_and__)
#define Dee_HashStr__symmetric_difference _Dee_HashSelectC(0x9a1e5057, 0x17b1425a414674d3)
DEF_STRING(str_symmetric_difference, "symmetric_difference", 0x9a1e5057, 0x17b1425a414674d3)
#define STR_symmetric_difference DeeString_STR(&str_symmetric_difference)
#define Dee_HashStr____set_xor__ _Dee_HashSelectC(0x59e8993a, 0x3feb800b0e1df4cd)
DEF_STRING(str___set_xor__, "__set_xor__", 0x59e8993a, 0x3feb800b0e1df4cd)
#define STR___set_xor__ DeeString_STR(&str___set_xor__)
#define Dee_HashStr____set_inplace_add__ _Dee_HashSelectC(0xfcb7e907, 0x964e87b39bdf6679)
DEF_STRING(str___set_inplace_add__, "__set_inplace_add__", 0xfcb7e907, 0x964e87b39bdf6679)
#define STR___set_inplace_add__ DeeString_STR(&str___set_inplace_add__)
#define Dee_HashStr____set_inplace_sub__ _Dee_HashSelectC(0x9ca00ddc, 0xfe43b45a3259fddb)
DEF_STRING(str___set_inplace_sub__, "__set_inplace_sub__", 0x9ca00ddc, 0xfe43b45a3259fddb)
#define STR___set_inplace_sub__ DeeString_STR(&str___set_inplace_sub__)
#define Dee_HashStr____set_inplace_and__ _Dee_HashSelectC(0x7f8a59c6, 0x810fa0b4d590ed16)
DEF_STRING(str___set_inplace_and__, "__set_inplace_and__", 0x7f8a59c6, 0x810fa0b4d590ed16)
#define STR___set_inplace_and__ DeeString_STR(&str___set_inplace_and__)
#define Dee_HashStr____set_inplace_xor__ _Dee_HashSelectC(0x4a81dcf1, 0x8cdbea199590f870)
DEF_STRING(str___set_inplace_xor__, "__set_inplace_xor__", 0x4a81dcf1, 0x8cdbea199590f870)
#define STR___set_inplace_xor__ DeeString_STR(&str___set_inplace_xor__)
#define Dee_HashStr____set_frozen__ _Dee_HashSelectC(0xb1972c98, 0xcf2786c52076fc2c)
DEF_STRING(str___set_frozen__, "__set_frozen__", 0xb1972c98, 0xcf2786c52076fc2c)
#define STR___set_frozen__ DeeString_STR(&str___set_frozen__)
#define Dee_HashStr__unify _Dee_HashSelectC(0x3cce686e, 0x4c0c9bdcc8d95cc7)
DEF_STRING(str_unify, "unify", 0x3cce686e, 0x4c0c9bdcc8d95cc7)
#define STR_unify DeeString_STR(&str_unify)
#define Dee_HashStr____set_unify__ _Dee_HashSelectC(0x72612949, 0xabb8cb45cdbd4ab4)
DEF_STRING(str___set_unify__, "__set_unify__", 0x72612949, 0xabb8cb45cdbd4ab4)
#define STR___set_unify__ DeeString_STR(&str___set_unify__)
#define Dee_HashStr____set_insert__ _Dee_HashSelectC(0x50fd52f7, 0x3fc463c8885bc57c)
DEF_STRING(str___set_insert__, "__set_insert__", 0x50fd52f7, 0x3fc463c8885bc57c)
#define STR___set_insert__ DeeString_STR(&str___set_insert__)
#define Dee_HashStr____set_insertall__ _Dee_HashSelectC(0x551cdddf, 0x8b13942a0ba095b4)
DEF_STRING(str___set_insertall__, "__set_insertall__", 0x551cdddf, 0x8b13942a0ba095b4)
#define STR___set_insertall__ DeeString_STR(&str___set_insertall__)
#define Dee_HashStr____set_remove__ _Dee_HashSelectC(0x7cff2ae1, 0xddd5656e54907429)
DEF_STRING(str___set_remove__, "__set_remove__", 0x7cff2ae1, 0xddd5656e54907429)
#define STR___set_remove__ DeeString_STR(&str___set_remove__)
#define Dee_HashStr____set_removeall__ _Dee_HashSelectC(0x427677be, 0x694ed4e8774b5ac6)
DEF_STRING(str___set_removeall__, "__set_removeall__", 0x427677be, 0x694ed4e8774b5ac6)
#define STR___set_removeall__ DeeString_STR(&str___set_removeall__)
#define Dee_HashStr____set_pop__ _Dee_HashSelectC(0xf29d444d, 0x747e5186fe18cdfb)
DEF_STRING(str___set_pop__, "__set_pop__", 0xf29d444d, 0x747e5186fe18cdfb)
#define STR___set_pop__ DeeString_STR(&str___set_pop__)
#define Dee_HashStr____map_iter__ _Dee_HashSelectC(0x5e1cf789, 0xef658451365f669e)
DEF_STRING(str___map_iter__, "__map_iter__", 0x5e1cf789, 0xef658451365f669e)
#define STR___map_iter__ DeeString_STR(&str___map_iter__)
#define Dee_HashStr____map_size__ _Dee_HashSelectC(0x96e29e1e, 0xd9031fef7c89a17d)
DEF_STRING(str___map_size__, "__map_size__", 0x96e29e1e, 0xd9031fef7c89a17d)
#define STR___map_size__ DeeString_STR(&str___map_size__)
#define Dee_HashStr____map_hash__ _Dee_HashSelectC(0x5ae7b9a0, 0xd3a036d48e067ebb)
DEF_STRING(str___map_hash__, "__map_hash__", 0x5ae7b9a0, 0xd3a036d48e067ebb)
#define STR___map_hash__ DeeString_STR(&str___map_hash__)
#define Dee_HashStr____map_getitem__ _Dee_HashSelectC(0x5de8e44c, 0x364d046254541982)
DEF_STRING(str___map_getitem__, "__map_getitem__", 0x5de8e44c, 0x364d046254541982)
#define STR___map_getitem__ DeeString_STR(&str___map_getitem__)
#define Dee_HashStr____map_delitem__ _Dee_HashSelectC(0xfd22048d, 0xa2d895d289b25417)
DEF_STRING(str___map_delitem__, "__map_delitem__", 0xfd22048d, 0xa2d895d289b25417)
#define STR___map_delitem__ DeeString_STR(&str___map_delitem__)
#define Dee_HashStr____map_setitem__ _Dee_HashSelectC(0x6843cc01, 0xdc449c7dc30d4d99)
DEF_STRING(str___map_setitem__, "__map_setitem__", 0x6843cc01, 0xdc449c7dc30d4d99)
#define STR___map_setitem__ DeeString_STR(&str___map_setitem__)
#define Dee_HashStr____map_contains__ _Dee_HashSelectC(0x6e6ab73e, 0x8de609e1509485a7)
DEF_STRING(str___map_contains__, "__map_contains__", 0x6e6ab73e, 0x8de609e1509485a7)
#define STR___map_contains__ DeeString_STR(&str___map_contains__)
#define Dee_HashStr__keys _Dee_HashSelectC(0x97e36be1, 0x654d31bc4825131c)
DEF_STRING(str_keys, "keys", 0x97e36be1, 0x654d31bc4825131c)
#define STR_keys DeeString_STR(&str_keys)
#define Dee_HashStr____map_keys__ _Dee_HashSelectC(0x14a40c86, 0xd1bc83542c560ec3)
DEF_STRING(str___map_keys__, "__map_keys__", 0x14a40c86, 0xd1bc83542c560ec3)
#define STR___map_keys__ DeeString_STR(&str___map_keys__)
#define Dee_HashStr__iterkeys _Dee_HashSelectC(0x62bd6adc, 0x535ac8ab28094ab3)
DEF_STRING(str_iterkeys, "iterkeys", 0x62bd6adc, 0x535ac8ab28094ab3)
#define STR_iterkeys DeeString_STR(&str_iterkeys)
#define Dee_HashStr____map_iterkeys__ _Dee_HashSelectC(0x1447f394, 0xe79c875bc390a418)
DEF_STRING(str___map_iterkeys__, "__map_iterkeys__", 0x1447f394, 0xe79c875bc390a418)
#define STR___map_iterkeys__ DeeString_STR(&str___map_iterkeys__)
#define Dee_HashStr__values _Dee_HashSelectC(0x33b551c8, 0xf6e3e991b86d1574)
DEF_STRING(str_values, "values", 0x33b551c8, 0xf6e3e991b86d1574)
#define STR_values DeeString_STR(&str_values)
#define Dee_HashStr____map_values__ _Dee_HashSelectC(0x8afb96e3, 0x6b716639763fd995)
DEF_STRING(str___map_values__, "__map_values__", 0x8afb96e3, 0x6b716639763fd995)
#define STR___map_values__ DeeString_STR(&str___map_values__)
#define Dee_HashStr__itervalues _Dee_HashSelectC(0xcb00bab3, 0xe9a89082a994930a)
DEF_STRING(str_itervalues, "itervalues", 0xcb00bab3, 0xe9a89082a994930a)
#define STR_itervalues DeeString_STR(&str_itervalues)
#define Dee_HashStr____map_itervalues__ _Dee_HashSelectC(0x225dfc75, 0xe6cfc129bcb6d0af)
DEF_STRING(str___map_itervalues__, "__map_itervalues__", 0x225dfc75, 0xe6cfc129bcb6d0af)
#define STR___map_itervalues__ DeeString_STR(&str___map_itervalues__)
#define Dee_HashStr____map_enumerate__ _Dee_HashSelectC(0x54761193, 0xd176d65465588e02)
DEF_STRING(str___map_enumerate__, "__map_enumerate__", 0x54761193, 0xd176d65465588e02)
#define STR___map_enumerate__ DeeString_STR(&str___map_enumerate__)
#define Dee_HashStr____map_enumerate_items__ _Dee_HashSelectC(0xf1946528, 0x2a3f812751074a29)
DEF_STRING(str___map_enumerate_items__, "__map_enumerate_items__", 0xf1946528, 0x2a3f812751074a29)
#define STR___map_enumerate_items__ DeeString_STR(&str___map_enumerate_items__)
#define Dee_HashStr____map_compare_eq__ _Dee_HashSelectC(0x9bf0e702, 0x608f8c3e4758f040)
DEF_STRING(str___map_compare_eq__, "__map_compare_eq__", 0x9bf0e702, 0x608f8c3e4758f040)
#define STR___map_compare_eq__ DeeString_STR(&str___map_compare_eq__)
#define Dee_HashStr____map_eq__ _Dee_HashSelectC(0x4d3488c2, 0x79c50f5f0604d6e3)
DEF_STRING(str___map_eq__, "__map_eq__", 0x4d3488c2, 0x79c50f5f0604d6e3)
#define STR___map_eq__ DeeString_STR(&str___map_eq__)
#define Dee_HashStr____map_ne__ _Dee_HashSelectC(0xaf578d0b, 0x4b5687687dc9dd55)
DEF_STRING(str___map_ne__, "__map_ne__", 0xaf578d0b, 0x4b5687687dc9dd55)
#define STR___map_ne__ DeeString_STR(&str___map_ne__)
#define Dee_HashStr____map_lo__ _Dee_HashSelectC(0x4d3830d7, 0x3f6eb05ce7835edf)
DEF_STRING(str___map_lo__, "__map_lo__", 0x4d3830d7, 0x3f6eb05ce7835edf)
#define STR___map_lo__ DeeString_STR(&str___map_lo__)
#define Dee_HashStr____map_le__ _Dee_HashSelectC(0x6a57d80d, 0x339b43fb9012632d)
DEF_STRING(str___map_le__, "__map_le__", 0x6a57d80d, 0x339b43fb9012632d)
#define STR___map_le__ DeeString_STR(&str___map_le__)
#define Dee_HashStr____map_gr__ _Dee_HashSelectC(0x3dabc7f8, 0xbf0d28fc9ca466fa)
DEF_STRING(str___map_gr__, "__map_gr__", 0x3dabc7f8, 0xbf0d28fc9ca466fa)
#define STR___map_gr__ DeeString_STR(&str___map_gr__)
#define Dee_HashStr____map_ge__ _Dee_HashSelectC(0x1ec135ba, 0xe3d8e69a9b6efffd)
DEF_STRING(str___map_ge__, "__map_ge__", 0x1ec135ba, 0xe3d8e69a9b6efffd)
#define STR___map_ge__ DeeString_STR(&str___map_ge__)
#define Dee_HashStr____map_add__ _Dee_HashSelectC(0x42065b0e, 0xd9b50b90683d4ae3)
DEF_STRING(str___map_add__, "__map_add__", 0x42065b0e, 0xd9b50b90683d4ae3)
#define STR___map_add__ DeeString_STR(&str___map_add__)
#define Dee_HashStr____map_sub__ _Dee_HashSelectC(0x82697e96, 0x88464d274bb6d009)
DEF_STRING(str___map_sub__, "__map_sub__", 0x82697e96, 0x88464d274bb6d009)
#define STR___map_sub__ DeeString_STR(&str___map_sub__)
#define Dee_HashStr____map_and__ _Dee_HashSelectC(0xa4839bff, 0xa0d5a1d0c3e9480)
DEF_STRING(str___map_and__, "__map_and__", 0xa4839bff, 0xa0d5a1d0c3e9480)
#define STR___map_and__ DeeString_STR(&str___map_and__)
#define Dee_HashStr____map_xor__ _Dee_HashSelectC(0x3fe194f4, 0xb9506e27fedd0894)
DEF_STRING(str___map_xor__, "__map_xor__", 0x3fe194f4, 0xb9506e27fedd0894)
#define STR___map_xor__ DeeString_STR(&str___map_xor__)
#define Dee_HashStr____map_inplace_add__ _Dee_HashSelectC(0x15d5f614, 0xe7a8b998befc67d9)
DEF_STRING(str___map_inplace_add__, "__map_inplace_add__", 0x15d5f614, 0xe7a8b998befc67d9)
#define STR___map_inplace_add__ DeeString_STR(&str___map_inplace_add__)
#define Dee_HashStr____map_inplace_sub__ _Dee_HashSelectC(0x7453a011, 0xa983a68c91082d2b)
DEF_STRING(str___map_inplace_sub__, "__map_inplace_sub__", 0x7453a011, 0xa983a68c91082d2b)
#define STR___map_inplace_sub__ DeeString_STR(&str___map_inplace_sub__)
#define Dee_HashStr____map_inplace_and__ _Dee_HashSelectC(0xa702b418, 0xd94c11e78b08868)
DEF_STRING(str___map_inplace_and__, "__map_inplace_and__", 0xa702b418, 0xd94c11e78b08868)
#define STR___map_inplace_and__ DeeString_STR(&str___map_inplace_and__)
#define Dee_HashStr____map_inplace_xor__ _Dee_HashSelectC(0xd0f2955f, 0x65663c20e89156c7)
DEF_STRING(str___map_inplace_xor__, "__map_inplace_xor__", 0xd0f2955f, 0x65663c20e89156c7)
#define STR___map_inplace_xor__ DeeString_STR(&str___map_inplace_xor__)
#define Dee_HashStr____map_frozen__ _Dee_HashSelectC(0xaca0a2f, 0xbc89db44c2452160)
DEF_STRING(str___map_frozen__, "__map_frozen__", 0xaca0a2f, 0xbc89db44c2452160)
#define STR___map_frozen__ DeeString_STR(&str___map_frozen__)
#define Dee_HashStr__setold _Dee_HashSelectC(0xb02a28d9, 0xe69353d27a45da0c)
DEF_STRING(str_setold, "setold", 0xb02a28d9, 0xe69353d27a45da0c)
#define STR_setold DeeString_STR(&str_setold)
#define Dee_HashStr____map_setold__ _Dee_HashSelectC(0x98133a06, 0x9f85f07238f15ea8)
DEF_STRING(str___map_setold__, "__map_setold__", 0x98133a06, 0x9f85f07238f15ea8)
#define STR___map_setold__ DeeString_STR(&str___map_setold__)
#define Dee_HashStr__setold_ex _Dee_HashSelectC(0xf8b4d68b, 0x73d8fdc770be1ae)
DEF_STRING(str_setold_ex, "setold_ex", 0xf8b4d68b, 0x73d8fdc770be1ae)
#define STR_setold_ex DeeString_STR(&str_setold_ex)
#define Dee_HashStr____map_setold_ex__ _Dee_HashSelectC(0x2ff74f04, 0x4bf3a474c83f3c6e)
DEF_STRING(str___map_setold_ex__, "__map_setold_ex__", 0x2ff74f04, 0x4bf3a474c83f3c6e)
#define STR___map_setold_ex__ DeeString_STR(&str___map_setold_ex__)
#define Dee_HashStr__setnew _Dee_HashSelectC(0xb6040b2, 0xde8a8697e7aca93d)
DEF_STRING(str_setnew, "setnew", 0xb6040b2, 0xde8a8697e7aca93d)
#define STR_setnew DeeString_STR(&str_setnew)
#define Dee_HashStr____map_setnew__ _Dee_HashSelectC(0x3d51899e, 0x7cc7fad3b094d5ff)
DEF_STRING(str___map_setnew__, "__map_setnew__", 0x3d51899e, 0x7cc7fad3b094d5ff)
#define STR___map_setnew__ DeeString_STR(&str___map_setnew__)
#define Dee_HashStr__setnew_ex _Dee_HashSelectC(0x3f694391, 0x104d84a2d9986bc5)
DEF_STRING(str_setnew_ex, "setnew_ex", 0x3f694391, 0x104d84a2d9986bc5)
#define STR_setnew_ex DeeString_STR(&str_setnew_ex)
#define Dee_HashStr____map_setnew_ex__ _Dee_HashSelectC(0xdf4ed868, 0x57b1501ab5df4980)
DEF_STRING(str___map_setnew_ex__, "__map_setnew_ex__", 0xdf4ed868, 0x57b1501ab5df4980)
#define STR___map_setnew_ex__ DeeString_STR(&str___map_setnew_ex__)
#define Dee_HashStr__setdefault _Dee_HashSelectC(0x947d5cce, 0x7cbcb4f64ace9cbc)
DEF_STRING(str_setdefault, "setdefault", 0x947d5cce, 0x7cbcb4f64ace9cbc)
#define STR_setdefault DeeString_STR(&str_setdefault)
#define Dee_HashStr____map_setdefault__ _Dee_HashSelectC(0xc24f2597, 0xe662ab907613d2e1)
DEF_STRING(str___map_setdefault__, "__map_setdefault__", 0xc24f2597, 0xe662ab907613d2e1)
#define STR___map_setdefault__ DeeString_STR(&str___map_setdefault__)
#define Dee_HashStr__update _Dee_HashSelectC(0xdf8e9237, 0x41c79529f2460018)
DEF_STRING(str_update, "update", 0xdf8e9237, 0x41c79529f2460018)
#define STR_update DeeString_STR(&str_update)
#define Dee_HashStr____map_update__ _Dee_HashSelectC(0x7999cbe, 0xfeac53cf88774cdb)
DEF_STRING(str___map_update__, "__map_update__", 0x7999cbe, 0xfeac53cf88774cdb)
#define STR___map_update__ DeeString_STR(&str___map_update__)
#define Dee_HashStr____map_remove__ _Dee_HashSelectC(0xaaf1d3a3, 0xa18455f39b93dac2)
DEF_STRING(str___map_remove__, "__map_remove__", 0xaaf1d3a3, 0xa18455f39b93dac2)
#define STR___map_remove__ DeeString_STR(&str___map_remove__)
#define Dee_HashStr__removekeys _Dee_HashSelectC(0x85b72988, 0xb92131e1a60492b4)
DEF_STRING(str_removekeys, "removekeys", 0x85b72988, 0xb92131e1a60492b4)
#define STR_removekeys DeeString_STR(&str_removekeys)
#define Dee_HashStr____map_removekeys__ _Dee_HashSelectC(0x41b7c204, 0xdbdd6785608b9e21)
DEF_STRING(str___map_removekeys__, "__map_removekeys__", 0x41b7c204, 0xdbdd6785608b9e21)
#define STR___map_removekeys__ DeeString_STR(&str___map_removekeys__)
#define Dee_HashStr____map_pop__ _Dee_HashSelectC(0x280f151f, 0xd722ef001b673063)
DEF_STRING(str___map_pop__, "__map_pop__", 0x280f151f, 0xd722ef001b673063)
#define STR___map_pop__ DeeString_STR(&str___map_pop__)
#define Dee_HashStr__popitem _Dee_HashSelectC(0x40b249f3, 0x131a404a88439bc0)
DEF_STRING(str_popitem, "popitem", 0x40b249f3, 0x131a404a88439bc0)
#define STR_popitem DeeString_STR(&str_popitem)
#define Dee_HashStr____map_popitem__ _Dee_HashSelectC(0x66a1fe6, 0x6456db8617f2ac73)
DEF_STRING(str___map_popitem__, "__map_popitem__", 0x66a1fe6, 0x6456db8617f2ac73)
#define STR___map_popitem__ DeeString_STR(&str___map_popitem__)
#ifdef Dee_fd_t_IS_HANDLE
#define Dee_HashStr__getsysfd _Dee_HashSelectC(0x75b169b6, 0x74235841d2ace4f0)
DEF_STRING(str_getsysfd, "osfhandle_np", 0x75b169b6, 0x74235841d2ace4f0)
#define STR_getsysfd DeeString_STR(&str_getsysfd)
#elif defined(Dee_fd_t_IS_int)
#define Dee_HashStr__getsysfd _Dee_HashSelectC(0xe3e546ab, 0x38c7dbc48e44183)
DEF_STRING(str_getsysfd, "fileno_np", 0xe3e546ab, 0x38c7dbc48e44183)
#define STR_getsysfd DeeString_STR(&str_getsysfd)
#endif /* ... */

#ifndef CONFIG_NO_THREADS
#define Dee_HashStr__run _Dee_HashSelectC(0xf1764c48, 0x7b92c951b5e510e3)
DEF_STRING(str_run, "run", 0xf1764c48, 0x7b92c951b5e510e3)
#define STR_run DeeString_STR(&str_run)
#endif /* !CONFIG_NO_THREADS */

#ifndef CONFIG_LANGUAGE_NO_ASM
#define Dee_HashStr__this_module _Dee_HashSelectC(0x34998e44, 0x473e02aa4d7eba45)
DEF_STRING(str_this_module, "this_module", 0x34998e44, 0x473e02aa4d7eba45)
#define STR_this_module DeeString_STR(&str_this_module)
#define Dee_HashStr__this_function _Dee_HashSelectC(0xe2b69fa3, 0xdf2ba17d58877ece)
DEF_STRING(str_this_function, "this_function", 0xe2b69fa3, 0xdf2ba17d58877ece)
#define STR_this_function DeeString_STR(&str_this_function)
#endif /* !CONFIG_LANGUAGE_NO_ASM */
/*[[[end]]]*/

#undef DEF_STRING

#ifndef STR_index
#define STR_index "index"
#endif /* !STR_index */
#ifndef STR_rindex
#define STR_rindex "rindex"
#endif /* !STR_rindex */
#ifndef STR_popfront
#define STR_popfront "popfront"
#endif /* !STR_popfront */
#ifndef STR_popback
#define STR_popback "popback"
#endif /* !STR_popback */
#ifndef STR_enumerate
#define STR_enumerate "enumerate"
#endif /* !STR_enumerate */
#ifndef STR_run
#define STR_run "run"
#endif /* !STR_run */
#ifndef STR_this_module
#define STR_this_module "this_module"
#endif /* !STR_this_module */
#ifndef STR_this_function
#define STR_this_function "this_function"
#endif /* !STR_this_function */
#ifndef STR_ItemType
#define STR_ItemType "ItemType"
#endif /* !STR_ItemType */
#ifndef STR_Frozen
#define STR_Frozen "Frozen"
#endif /* !STR_Frozen */
#ifndef STR_Keys
#define STR_Keys "Keys"
#endif /* !STR_Keys */
#ifndef STR_Values
#define STR_Values "Values"
#endif /* !STR_Values */
#ifndef STR_IterKeys
#define STR_IterKeys "IterKeys"
#endif /* !STR_IterKeys */
#ifndef STR_IterValues
#define STR_IterValues "IterValues"
#endif /* !STR_IterValues */

/* Some versions of GCC think that using DeeString_STR() on a static
 * string object will result in us reading from out-of-bounds memory,
 * since it doesn't understand that static objects (with flexible array
 * members) can still be larger than the object's minimal size. */
__pragma_GCC_diagnostic_ignored(Wstringop_overflow)

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_STRINGS_H */
