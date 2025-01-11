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
#ifndef GUARD_DEEMON_RUNTIME_KWLIST_H
#define GUARD_DEEMON_RUNTIME_KWLIST_H 1

#include <deemon/api.h>
#include <deemon/arg.h>

#ifndef DECLARE_KWLIST
#define DECLARE_KWLIST(name, ...) INTDEF struct Dee_keyword name[];
#endif /* !DECLARE_KWLIST */

DECL_BEGIN

/*[[[deemon
import * from deemon;
import rt.hash as rtHash;
local kw_lists = {
	{"thisarg"},
	{"thisarg", "value"},
	{"message", "inner"},
	{"radix", "precision", "mode"},
	{"precision"},
	{"length", "byteorder", "signed"},
	{"bytes", "byteorder", "signed"},
	{"signed"},
	{"other"},
	{"name", "argc"},
	{"attr"},
	{"attr", "value"},
	{"attr", "allow_missing"},
	{"lhs", "rhs"},
	{"base", "name"},
	{"expr", "globals"},
	{"module"},
	{"value", "scope", "loc"},
	{"sym", "scope", "loc"},
	{"expr", "scope", "loc"},
	{"guard", "handlers", "scope", "loc"},
	{"elem", "iter", "loop", "scope", "loc"},
	{"cond", "next", "loop", "scope", "loc"},
	{"branches", "typing", "scope", "loc"},
	{"isbreak", "scope", "loc"},
	{"cond", "tt", "ff", "flags", "scope", "loc"},
	{"expr", "negate", "scope", "loc"},
	{"code", "scope", "loc"},
	{"name", "binding", "scope", "loc"},
	{"name", "a", "b", "c", "d", "flags", "scope", "loc"},
	{"name", "a", "b", "c", "mustrun", "scope", "loc"},
	{"name", "create"},
	{"name", "value", "builtin"},
	{"predicate", "answer"},
	{"extend", "binary", "nonblocking"},
	{"head"},
	{"lookupmode"},
	{"nonblocking"},
	{"end"},
	{"name", "requirenew", "loc"},
	{"tuple", "kwds", "async"},
	{"text", "module", "constants", "except", "nlocal", "nstack", "nref", "nstatic", "argc", "keywords", "defaults", "flags", "ddi"},
	{"yfunc", "argc", "ridstart", "ridend"},
	{"frame", "argc", "ridstart", "ridend", "localc", "stackc"},
	{"index", "count"},
	{"index", "item"},
	{"index", "items"},
	{"index"},
	{"combine", "start", "end", "init"},
	{"item", "start", "end"},
	{"item", "start", "end", "key"},
	{"item", "start", "end", "max", "key"},
	{"match", "start", "end", "def"},
	{"should", "start", "end", "max"},
	{"size", "filler"},
	{"start", "end", "values"},
	{"start", "end", "filler"},
	{"start", "end"},
	{"start", "end", "key"},
	{"cb", "start", "end"},
	{"needle", "start", "end"},
	{"find", "replace", "max"},
	{"pattern", "replace", "max", "rules"},
	{"pattern", "start", "end", "rules"},
	{"pattern", "start", "end", "range", "rules"},
	{"needle", "max"},
	{"mask", "max"},
	{"codec", "errors"},
	{"ob", "name", "flagmask", "flagval", "decl"},
	{"name", "doc", "flags", "operators", "iattr", "cattr", "isize", "csize"},
	{"path", "oflags", "mode"},
	{"maxbytes", "readall"},
	{"dst", "readall"},
	{"data", "writeall"},
	{"pos", "maxbytes", "readall"},
	{"dst", "pos", "readall"},
	{"data", "pos", "writeall"},
	{"off", "whence"},
	{"minbytes", "maxbytes", "offset", "nulbytes", "readall", "mustmmap", "mapshared"},
	{"mode", "size"},
	{"data", "start", "end", "pos"},
	{"fd", "minbytes", "maxbytes", "offset", "nulbytes", "readall", "mustmmap", "mapshared"},
	{"func", "thisarg"},
	{"getter", "delete", "setter"},
	{"key"},
	{"template"},
	{"item", "key"},
	{"obj", "callback"},
	{"code", "positional", "kwargs", "kwds"},
	{"code", "positional", "kwds"},
	{"r", "cached"},
	{"fully"},
	{"keys", "value", "valuefor"},
	{"total", "more"},
	{"total", "more", "weak"},

	{"#ifdef CONFIG_HOST_WINDOWS", "message", "inner", "errno", "nterr_np"},
	{"#ifndef CONFIG_HOST_WINDOWS", "message", "inner", "errno"},
};

local conditional = HashSet(for (local x: kw_lists) if (x.first.startswith("#")) x);
local unconditional = HashSet(for (local x: kw_lists) if (!x.first.startswith("#")) x);
local conditionalByCondition: {string: {{string...}...}} = Dict();
for (local args: conditional.sorted())
	conditionalByCondition.setdefault(args.first, HashSet()).insert(args[1:]);
for (local args: unconditional.sorted()) {
	print("DECLARE_KWLIST(kwlist__", "_".join(args), ", { "),;
	for (local arg: args) {
		print(f"KEX({repr arg}, {rtHash.hash32(arg).hex()}, {rtHash.hash64(arg).hex()}), "),;
	}
	print("KEND });");
}
for (local cond, blocks: conditionalByCondition.sorted()) {
	print(cond);
	for (local args: blocks.sorted()) {
		print("DECLARE_KWLIST(kwlist__", "_".join(args), ", { "),;
		for (local arg: args) {
			print(f"KEX({repr arg}, {rtHash.hash32(arg).hex()}, {rtHash.hash64(arg).hex()}), "),;
		}
		print("KEND });");
	}
	print("#endif /" "* ... *" "/");
}
]]]*/
DECLARE_KWLIST(kwlist__attr, { KEX("attr", 0x55cfee3, 0xe4311a2c8443755d), KEND });
DECLARE_KWLIST(kwlist__attr_allow_missing, { KEX("attr", 0x55cfee3, 0xe4311a2c8443755d), KEX("allow_missing", 0xc2a212fa, 0xf80df0d65cca5cc9), KEND });
DECLARE_KWLIST(kwlist__attr_value, { KEX("attr", 0x55cfee3, 0xe4311a2c8443755d), KEX("value", 0xd9093f6e, 0x69e7413ae0c88471), KEND });
DECLARE_KWLIST(kwlist__base_name, { KEX("base", 0xc3cb0590, 0x56fd8eccbdfdd7a7), KEX("name", 0xdbaf43f0, 0x8bcdb293dc3cbddc), KEND });
DECLARE_KWLIST(kwlist__branches_typing_scope_loc, { KEX("branches", 0xb5d662bc, 0xc0df755730133cbf), KEX("typing", 0x83a69128, 0x4a1266cd5553de66), KEX("scope", 0x8b65b2f8, 0x52824a24d6447e5), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__bytes_byteorder_signed, { KEX("bytes", 0x998eb0c, 0x88b08b49ba3c5d43), KEX("byteorder", 0x7b88a38d, 0x7fef2c9253a29a88), KEX("signed", 0x17a15883, 0x58a245b6f802625f), KEND });
DECLARE_KWLIST(kwlist__cb_start_end, { KEX("cb", 0x75ffadba, 0x2501dbb50208b92e), KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEND });
DECLARE_KWLIST(kwlist__code_positional_kwargs_kwds, { KEX("code", 0x6e753b06, 0xdb2cd7ddab7a1dec), KEX("positional", 0x514b31fa, 0xd12fff1a2457ecd1), KEX("kwargs", 0xe88e435b, 0x4292c60f7a499445), KEX("kwds", 0x6dfae69b, 0x66fee9115d75f3ef), KEND });
DECLARE_KWLIST(kwlist__code_positional_kwds, { KEX("code", 0x6e753b06, 0xdb2cd7ddab7a1dec), KEX("positional", 0x514b31fa, 0xd12fff1a2457ecd1), KEX("kwds", 0x6dfae69b, 0x66fee9115d75f3ef), KEND });
DECLARE_KWLIST(kwlist__code_scope_loc, { KEX("code", 0x6e753b06, 0xdb2cd7ddab7a1dec), KEX("scope", 0x8b65b2f8, 0x52824a24d6447e5), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__codec_errors, { KEX("codec", 0x91dfc790, 0x678d4474a4f58564), KEX("errors", 0xd327c5ea, 0x88b9782b6de95122), KEND });
DECLARE_KWLIST(kwlist__combine_start_end_init, { KEX("combine", 0x6ee686b7, 0x2224a65df53ea995), KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("init", 0x4179122, 0x8c337193ca26e208), KEND });
DECLARE_KWLIST(kwlist__cond_next_loop_scope_loc, { KEX("cond", 0xd089ebf5, 0xe82f5d755b5a03e7), KEX("next", 0x7e5e1569, 0x2a31f6a650652d33), KEX("loop", 0xe44e70af, 0xadc137e48b7293ee), KEX("scope", 0x8b65b2f8, 0x52824a24d6447e5), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__cond_tt_ff_flags_scope_loc, { KEX("cond", 0xd089ebf5, 0xe82f5d755b5a03e7), KEX("tt", 0xb3c7c4a6, 0xa7c060e25efbe4e5), KEX("ff", 0x83c00a2, 0xcba1f9680f16ea23), KEX("flags", 0xd9e40622, 0x6afda85728fae70d), KEX("scope", 0x8b65b2f8, 0x52824a24d6447e5), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__data_pos_writeall, { KEX("data", 0x3af4b6d3, 0xb0164401a9853128), KEX("pos", 0xb1aecbb4, 0x277b6d36f75741ae), KEX("writeall", 0x1b1ebbd8, 0x90da47dac64003d1), KEND });
DECLARE_KWLIST(kwlist__data_start_end_pos, { KEX("data", 0x3af4b6d3, 0xb0164401a9853128), KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("pos", 0xb1aecbb4, 0x277b6d36f75741ae), KEND });
DECLARE_KWLIST(kwlist__data_writeall, { KEX("data", 0x3af4b6d3, 0xb0164401a9853128), KEX("writeall", 0x1b1ebbd8, 0x90da47dac64003d1), KEND });
DECLARE_KWLIST(kwlist__dst_pos_readall, { KEX("dst", 0x2c96daf8, 0xb9f356c1d6facfce), KEX("pos", 0xb1aecbb4, 0x277b6d36f75741ae), KEX("readall", 0x331ceae8, 0x8ad608764266c76d), KEND });
DECLARE_KWLIST(kwlist__dst_readall, { KEX("dst", 0x2c96daf8, 0xb9f356c1d6facfce), KEX("readall", 0x331ceae8, 0x8ad608764266c76d), KEND });
DECLARE_KWLIST(kwlist__elem_iter_loop_scope_loc, { KEX("elem", 0x1aacf22d, 0x705652c4aed9308a), KEX("iter", 0xa249e6cd, 0xcad5071f49906e5f), KEX("loop", 0xe44e70af, 0xadc137e48b7293ee), KEX("scope", 0x8b65b2f8, 0x52824a24d6447e5), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__end, { KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEND });
DECLARE_KWLIST(kwlist__expr_globals, { KEX("expr", 0x391ad037, 0x9928df37efec67d5), KEX("globals", 0x98e98592, 0x188be45e73afd7e), KEND });
DECLARE_KWLIST(kwlist__expr_negate_scope_loc, { KEX("expr", 0x391ad037, 0x9928df37efec67d5), KEX("negate", 0x2fa02a47, 0x141c7912ed8219b0), KEX("scope", 0x8b65b2f8, 0x52824a24d6447e5), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__expr_scope_loc, { KEX("expr", 0x391ad037, 0x9928df37efec67d5), KEX("scope", 0x8b65b2f8, 0x52824a24d6447e5), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__extend_binary_nonblocking, { KEX("extend", 0x960b75e7, 0xba076858e3adb055), KEX("binary", 0x338ee80d, 0x2a346109a4d10240), KEX("nonblocking", 0xd2f4eb1f, 0xdf83fd2b5752ab51), KEND });
DECLARE_KWLIST(kwlist__fd_minbytes_maxbytes_offset_nulbytes_readall_mustmmap_mapshared, { KEX("fd", 0x10561ad6, 0xce2e588d84c6793), KEX("minbytes", 0xa6327ad1, 0x36d31d85046534af), KEX("maxbytes", 0x3b196fc0, 0x93d3b3615fcacd4a), KEX("offset", 0xa97063e7, 0x2381bd4159ebe8a7), KEX("nulbytes", 0xf6256f, 0x164a29d88f8ebaa3), KEX("readall", 0x331ceae8, 0x8ad608764266c76d), KEX("mustmmap", 0x720e751d, 0x10d9f32721af9f8d), KEX("mapshared", 0xcb3dd03e, 0xc9af3c3e0ad0fc85), KEND });
DECLARE_KWLIST(kwlist__find_replace_max, { KEX("find", 0x9e66372, 0x2b65fe03bbdde5b2), KEX("replace", 0x54b94882, 0x2d4ba4f8cfd63bc6), KEX("max", 0xc293979b, 0x822bd5c706bd9850), KEND });
DECLARE_KWLIST(kwlist__frame_argc_ridstart_ridend_localc_stackc, { KEX("frame", 0x84d7f989, 0x2d577bebe8cb9d1f), KEX("argc", 0xe5c6c120, 0xd96a642eb89eed13), KEX("ridstart", 0x1948fbda, 0xf5df625781b6632b), KEX("ridend", 0x815ea972, 0x9fd191cbcc2b1731), KEX("localc", 0x37633c82, 0x192c5e324b6944a3), KEX("stackc", 0x758638f3, 0x8336c39ba9094fa8), KEND });
DECLARE_KWLIST(kwlist__fully, { KEX("fully", 0xe8a99a63, 0xa054d5734992b1fc), KEND });
DECLARE_KWLIST(kwlist__func_thisarg, { KEX("func", 0x85f1f7f7, 0x33c744efe63d8c2d), KEX("thisarg", 0xfeb6b4f, 0xd3f418e6f91d6ac1), KEND });
DECLARE_KWLIST(kwlist__getter_delete_setter, { KEX("getter", 0x3b6f31af, 0x2c85740dee910984), KEX("delete", 0x26f7366c, 0xe5950d7b4ca209c3), KEX("setter", 0x27892c4f, 0x699b04a416d42834), KEND });
DECLARE_KWLIST(kwlist__guard_handlers_scope_loc, { KEX("guard", 0x8338047e, 0xa7c4a82949b93392), KEX("handlers", 0x840e9780, 0x552e754df097389b), KEX("scope", 0x8b65b2f8, 0x52824a24d6447e5), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__head, { KEX("head", 0x95a53fc9, 0xd2803db826dcba8a), KEND });
DECLARE_KWLIST(kwlist__index, { KEX("index", 0x77f34f0, 0x440d5888c0ff3081), KEND });
DECLARE_KWLIST(kwlist__index_count, { KEX("index", 0x77f34f0, 0x440d5888c0ff3081), KEX("count", 0x54eac164, 0xbd66b5980d54babb), KEND });
DECLARE_KWLIST(kwlist__index_item, { KEX("index", 0x77f34f0, 0x440d5888c0ff3081), KEX("item", 0x91b22efe, 0xe78210b41247a693), KEND });
DECLARE_KWLIST(kwlist__index_items, { KEX("index", 0x77f34f0, 0x440d5888c0ff3081), KEX("items", 0x8858badc, 0xa52ef5f42baafa42), KEND });
DECLARE_KWLIST(kwlist__isbreak_scope_loc, { KEX("isbreak", 0xdb6a737a, 0xf4916531d0a5abe7), KEX("scope", 0x8b65b2f8, 0x52824a24d6447e5), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__item_key, { KEX("item", 0x91b22efe, 0xe78210b41247a693), KEX("key", 0xe29c6a44, 0x612dd31212e90587), KEND });
DECLARE_KWLIST(kwlist__item_start_end, { KEX("item", 0x91b22efe, 0xe78210b41247a693), KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEND });
DECLARE_KWLIST(kwlist__item_start_end_key, { KEX("item", 0x91b22efe, 0xe78210b41247a693), KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("key", 0xe29c6a44, 0x612dd31212e90587), KEND });
DECLARE_KWLIST(kwlist__item_start_end_max_key, { KEX("item", 0x91b22efe, 0xe78210b41247a693), KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("max", 0xc293979b, 0x822bd5c706bd9850), KEX("key", 0xe29c6a44, 0x612dd31212e90587), KEND });
DECLARE_KWLIST(kwlist__key, { KEX("key", 0xe29c6a44, 0x612dd31212e90587), KEND });
DECLARE_KWLIST(kwlist__keys_value_valuefor, { KEX("keys", 0x97e36be1, 0x654d31bc4825131c), KEX("value", 0xd9093f6e, 0x69e7413ae0c88471), KEX("valuefor", 0xc2d5a98f, 0x8c111daeb6d08b14), KEND });
DECLARE_KWLIST(kwlist__length_byteorder_signed, { KEX("length", 0xecef0c1, 0x2993e8eb119cab21), KEX("byteorder", 0x7b88a38d, 0x7fef2c9253a29a88), KEX("signed", 0x17a15883, 0x58a245b6f802625f), KEND });
DECLARE_KWLIST(kwlist__lhs_rhs, { KEX("lhs", 0x4778c168, 0x4c6b9df6ea0934de), KEX("rhs", 0xc6d67ad, 0x7f4a7fedc9040cd3), KEND });
DECLARE_KWLIST(kwlist__lookupmode, { KEX("lookupmode", 0x5919ca40, 0x2f6c38dd63e00f55), KEND });
DECLARE_KWLIST(kwlist__mask_max, { KEX("mask", 0xc3b4302b, 0x933f153b40dd4379), KEX("max", 0xc293979b, 0x822bd5c706bd9850), KEND });
DECLARE_KWLIST(kwlist__match_start_end_def, { KEX("match", 0x69faa058, 0xb0704984f0078f40), KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("def", 0xf5797de2, 0x6cea9b604fa07583), KEND });
DECLARE_KWLIST(kwlist__maxbytes_readall, { KEX("maxbytes", 0x3b196fc0, 0x93d3b3615fcacd4a), KEX("readall", 0x331ceae8, 0x8ad608764266c76d), KEND });
DECLARE_KWLIST(kwlist__message_inner, { KEX("message", 0x14820755, 0xbeaa4b97155366df), KEX("inner", 0x20e82985, 0x4f20d07bb803c1fe), KEND });
DECLARE_KWLIST(kwlist__minbytes_maxbytes_offset_nulbytes_readall_mustmmap_mapshared, { KEX("minbytes", 0xa6327ad1, 0x36d31d85046534af), KEX("maxbytes", 0x3b196fc0, 0x93d3b3615fcacd4a), KEX("offset", 0xa97063e7, 0x2381bd4159ebe8a7), KEX("nulbytes", 0xf6256f, 0x164a29d88f8ebaa3), KEX("readall", 0x331ceae8, 0x8ad608764266c76d), KEX("mustmmap", 0x720e751d, 0x10d9f32721af9f8d), KEX("mapshared", 0xcb3dd03e, 0xc9af3c3e0ad0fc85), KEND });
DECLARE_KWLIST(kwlist__mode_size, { KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEX("size", 0xed8917fa, 0x3fe8023bdf261c0f), KEND });
DECLARE_KWLIST(kwlist__module, { KEX("module", 0xae3684a4, 0xbb78a82535e5801e), KEND });
DECLARE_KWLIST(kwlist__name_a_b_c_d_flags_scope_loc, { KEX("name", 0xdbaf43f0, 0x8bcdb293dc3cbddc), KEX("a", 0x3c2569b2, 0x4292cee227b9150a), KEX("b", 0x95de7e03, 0x88fbb133f8576bd5), KEX("c", 0xe132d65f, 0x32c769ee5c5509b0), KEX("d", 0x27191473, 0x7a452383aa9bf7c4), KEX("flags", 0xd9e40622, 0x6afda85728fae70d), KEX("scope", 0x8b65b2f8, 0x52824a24d6447e5), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__name_a_b_c_mustrun_scope_loc, { KEX("name", 0xdbaf43f0, 0x8bcdb293dc3cbddc), KEX("a", 0x3c2569b2, 0x4292cee227b9150a), KEX("b", 0x95de7e03, 0x88fbb133f8576bd5), KEX("c", 0xe132d65f, 0x32c769ee5c5509b0), KEX("mustrun", 0x7c653ecc, 0x40c10c84cd362cc7), KEX("scope", 0x8b65b2f8, 0x52824a24d6447e5), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__name_argc, { KEX("name", 0xdbaf43f0, 0x8bcdb293dc3cbddc), KEX("argc", 0xe5c6c120, 0xd96a642eb89eed13), KEND });
DECLARE_KWLIST(kwlist__name_binding_scope_loc, { KEX("name", 0xdbaf43f0, 0x8bcdb293dc3cbddc), KEX("binding", 0xff219a41, 0x7287dc3022834c20), KEX("scope", 0x8b65b2f8, 0x52824a24d6447e5), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__name_create, { KEX("name", 0xdbaf43f0, 0x8bcdb293dc3cbddc), KEX("create", 0x2fe30bdf, 0x4ca754f2ad18c6ae), KEND });
DECLARE_KWLIST(kwlist__name_doc_flags_operators_iattr_cattr_isize_csize, { KEX("name", 0xdbaf43f0, 0x8bcdb293dc3cbddc), KEX("doc", 0xd5a05523, 0x96758c759cfa17a6), KEX("flags", 0xd9e40622, 0x6afda85728fae70d), KEX("operators", 0xd4b6b76c, 0xc8d5b5ae0eb7316e), KEX("iattr", 0x94ebb8ec, 0x46ea695e3f817c99), KEX("cattr", 0xb088469c, 0x573747fd43f3c1e1), KEX("isize", 0xc9f18430, 0xe4a758dbfdeaa90c), KEX("csize", 0x6b6b1f1e, 0x21f992162cf30ec3), KEND });
DECLARE_KWLIST(kwlist__name_requirenew_loc, { KEX("name", 0xdbaf43f0, 0x8bcdb293dc3cbddc), KEX("requirenew", 0xd30f277a, 0xfd3369926891ad), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__name_value_builtin, { KEX("name", 0xdbaf43f0, 0x8bcdb293dc3cbddc), KEX("value", 0xd9093f6e, 0x69e7413ae0c88471), KEX("builtin", 0x8047ac18, 0x860b479ebf90d5d3), KEND });
DECLARE_KWLIST(kwlist__needle_max, { KEX("needle", 0xc332b2c9, 0xac5d4abaf21e8ae1), KEX("max", 0xc293979b, 0x822bd5c706bd9850), KEND });
DECLARE_KWLIST(kwlist__needle_start_end, { KEX("needle", 0xc332b2c9, 0xac5d4abaf21e8ae1), KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEND });
DECLARE_KWLIST(kwlist__nonblocking, { KEX("nonblocking", 0xd2f4eb1f, 0xdf83fd2b5752ab51), KEND });
DECLARE_KWLIST(kwlist__ob_name_flagmask_flagval_decl, { KEX("ob", 0xdfa5fee2, 0x80a90888850ad043), KEX("name", 0xdbaf43f0, 0x8bcdb293dc3cbddc), KEX("flagmask", 0x1ad95b0e, 0xb243c89ffa422745), KEX("flagval", 0x364af42c, 0x4e190d94378badaa), KEX("decl", 0x95fe81e2, 0xdc35fdc1dce5cffc), KEND });
DECLARE_KWLIST(kwlist__obj_callback, { KEX("obj", 0x477b06c1, 0xffb577570f61bd03), KEX("callback", 0x3b9dd39e, 0x1e7dd8df6e98f4c6), KEND });
DECLARE_KWLIST(kwlist__off_whence, { KEX("off", 0xe95f4e95, 0x499726573b82dfda), KEX("whence", 0x41d86ee7, 0x560314d7be8806a5), KEND });
DECLARE_KWLIST(kwlist__other, { KEX("other", 0x887ca3a8, 0x353f92c20a856fc5), KEND });
DECLARE_KWLIST(kwlist__path_oflags_mode, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("oflags", 0xbe92b5be, 0x4f84e498f7c9d171), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEND });
DECLARE_KWLIST(kwlist__pattern_replace_max_rules, { KEX("pattern", 0x6ea88732, 0x829c0a5f2fb04920), KEX("replace", 0x54b94882, 0x2d4ba4f8cfd63bc6), KEX("max", 0xc293979b, 0x822bd5c706bd9850), KEX("rules", 0xa825949d, 0x78d6949b972cfd25), KEND });
DECLARE_KWLIST(kwlist__pattern_start_end_range_rules, { KEX("pattern", 0x6ea88732, 0x829c0a5f2fb04920), KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("range", 0x4c77a395, 0x566c6985947d9bbd), KEX("rules", 0xa825949d, 0x78d6949b972cfd25), KEND });
DECLARE_KWLIST(kwlist__pattern_start_end_rules, { KEX("pattern", 0x6ea88732, 0x829c0a5f2fb04920), KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("rules", 0xa825949d, 0x78d6949b972cfd25), KEND });
DECLARE_KWLIST(kwlist__pos_maxbytes_readall, { KEX("pos", 0xb1aecbb4, 0x277b6d36f75741ae), KEX("maxbytes", 0x3b196fc0, 0x93d3b3615fcacd4a), KEX("readall", 0x331ceae8, 0x8ad608764266c76d), KEND });
DECLARE_KWLIST(kwlist__precision, { KEX("precision", 0xb3c93bdd, 0x3d6866f78be60b), KEND });
DECLARE_KWLIST(kwlist__predicate_answer, { KEX("predicate", 0x3c672fb4, 0xb5f52435e811385), KEX("answer", 0x63c165df, 0x76fd5eeb2e58020), KEND });
DECLARE_KWLIST(kwlist__r_cached, { KEX("r", 0x5c9373f1, 0x6ece84440d42ecf6), KEX("cached", 0x915e175e, 0xddfd408a14eae4b4), KEND });
DECLARE_KWLIST(kwlist__radix_precision_mode, { KEX("radix", 0xb10d4185, 0xbf2dcb32c6415f32), KEX("precision", 0xb3c93bdd, 0x3d6866f78be60b), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEND });
DECLARE_KWLIST(kwlist__should_start_end_max, { KEX("should", 0x28877b82, 0xbb3cb749df0a8b51), KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("max", 0xc293979b, 0x822bd5c706bd9850), KEND });
DECLARE_KWLIST(kwlist__signed, { KEX("signed", 0x17a15883, 0x58a245b6f802625f), KEND });
DECLARE_KWLIST(kwlist__size_filler, { KEX("size", 0xed8917fa, 0x3fe8023bdf261c0f), KEX("filler", 0xb990988f, 0x6067d27b1e35cd17), KEND });
DECLARE_KWLIST(kwlist__start_end, { KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEND });
DECLARE_KWLIST(kwlist__start_end_filler, { KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("filler", 0xb990988f, 0x6067d27b1e35cd17), KEND });
DECLARE_KWLIST(kwlist__start_end_key, { KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("key", 0xe29c6a44, 0x612dd31212e90587), KEND });
DECLARE_KWLIST(kwlist__start_end_values, { KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("values", 0x33b551c8, 0xf6e3e991b86d1574), KEND });
DECLARE_KWLIST(kwlist__sym_scope_loc, { KEX("sym", 0x17fd993c, 0x1c0e6e19c328844b), KEX("scope", 0x8b65b2f8, 0x52824a24d6447e5), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__template, { KEX("template", 0xa21c009f, 0xb466762d86160f07), KEND });
DECLARE_KWLIST(kwlist__text_module_constants_except_nlocal_nstack_nref_nstatic_argc_keywords_defaults_flags_ddi, { KEX("text", 0xc624ae24, 0x2a28a0084dd3a743), KEX("module", 0xae3684a4, 0xbb78a82535e5801e), KEX("constants", 0x8d73036e, 0xba127eb7623ae369), KEX("except", 0x8aae072b, 0xac70487e4861a6f3), KEX("nlocal", 0xae97b3ed, 0x23bb6e6e6190e9d7), KEX("nstack", 0xb92ec26e, 0x80220c290516f55), KEX("nref", 0x7951b608, 0xb8d28d6292d2f6cb), KEX("nstatic", 0xf17d253b, 0x6229886d9780e339), KEX("argc", 0xe5c6c120, 0xd96a642eb89eed13), KEX("keywords", 0xe751d20, 0x68e3cfd5fbfbc77a), KEX("defaults", 0xd48dea84, 0x2f0dce9444829201), KEX("flags", 0xd9e40622, 0x6afda85728fae70d), KEX("ddi", 0xe141111d, 0x958d321a1dacb4ed), KEND });
DECLARE_KWLIST(kwlist__thisarg, { KEX("thisarg", 0xfeb6b4f, 0xd3f418e6f91d6ac1), KEND });
DECLARE_KWLIST(kwlist__thisarg_value, { KEX("thisarg", 0xfeb6b4f, 0xd3f418e6f91d6ac1), KEX("value", 0xd9093f6e, 0x69e7413ae0c88471), KEND });
DECLARE_KWLIST(kwlist__total_more, { KEX("total", 0x48601e35, 0xd63cdba353c9c786), KEX("more", 0x8a39c376, 0xff9278cf274776cc), KEND });
DECLARE_KWLIST(kwlist__total_more_weak, { KEX("total", 0x48601e35, 0xd63cdba353c9c786), KEX("more", 0x8a39c376, 0xff9278cf274776cc), KEX("weak", 0x41118332, 0xc8855a0749fb5152), KEND });
DECLARE_KWLIST(kwlist__tuple_kwds_async, { KEX("tuple", 0x8abf5ca6, 0x32af754eabcdc75a), KEX("kwds", 0x6dfae69b, 0x66fee9115d75f3ef), KEX("async", 0x4499f053, 0xcc6733737627be1e), KEND });
DECLARE_KWLIST(kwlist__value_scope_loc, { KEX("value", 0xd9093f6e, 0x69e7413ae0c88471), KEX("scope", 0x8b65b2f8, 0x52824a24d6447e5), KEX("loc", 0x4f1e6896, 0xc8a6c6e417ce00f9), KEND });
DECLARE_KWLIST(kwlist__yfunc_argc_ridstart_ridend, { KEX("yfunc", 0xdedea038, 0xc47ce4fa3cfab784), KEX("argc", 0xe5c6c120, 0xd96a642eb89eed13), KEX("ridstart", 0x1948fbda, 0xf5df625781b6632b), KEX("ridend", 0x815ea972, 0x9fd191cbcc2b1731), KEND });
#ifdef CONFIG_HOST_WINDOWS
DECLARE_KWLIST(kwlist__message_inner_errno_nterr_np, { KEX("message", 0x14820755, 0xbeaa4b97155366df), KEX("inner", 0x20e82985, 0x4f20d07bb803c1fe), KEX("errno", 0x6df11216, 0x9ded64cf31236f55), KEX("nterr_np", 0x4766570c, 0xd41540e86b5d9a0a), KEND });
#endif /* ... */
#ifndef CONFIG_HOST_WINDOWS
DECLARE_KWLIST(kwlist__message_inner_errno, { KEX("message", 0x14820755, 0xbeaa4b97155366df), KEX("inner", 0x20e82985, 0x4f20d07bb803c1fe), KEX("errno", 0x6df11216, 0x9ded64cf31236f55), KEND });
#endif /* ... */
/*[[[end]]]*/

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_KWLIST_H */
