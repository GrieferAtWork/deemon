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
#ifndef GUARD_DEX_COLLECTIONS_KWLIST_H
#define GUARD_DEX_COLLECTIONS_KWLIST_H 1

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
	{"item", "start", "end", "key"},
	{"item", "start", "end", "max", "key"},
	{"should", "start", "end", "max"},
	{"start", "end", "filler"},
	{"start", "end"},
	{"index", "value"},
	{"index", "item"},
	{"index", "count"},
	{"index"},
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
DECLARE_KWLIST(kwlist__index, { KEX("index", 0x77f34f0, 0x440d5888c0ff3081), KEND });
DECLARE_KWLIST(kwlist__index_count, { KEX("index", 0x77f34f0, 0x440d5888c0ff3081), KEX("count", 0x54eac164, 0xbd66b5980d54babb), KEND });
DECLARE_KWLIST(kwlist__index_item, { KEX("index", 0x77f34f0, 0x440d5888c0ff3081), KEX("item", 0x91b22efe, 0xe78210b41247a693), KEND });
DECLARE_KWLIST(kwlist__index_value, { KEX("index", 0x77f34f0, 0x440d5888c0ff3081), KEX("value", 0xd9093f6e, 0x69e7413ae0c88471), KEND });
DECLARE_KWLIST(kwlist__item_start_end_key, { KEX("item", 0x91b22efe, 0xe78210b41247a693), KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("key", 0xe29c6a44, 0x612dd31212e90587), KEND });
DECLARE_KWLIST(kwlist__item_start_end_max_key, { KEX("item", 0x91b22efe, 0xe78210b41247a693), KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("max", 0xc293979b, 0x822bd5c706bd9850), KEX("key", 0xe29c6a44, 0x612dd31212e90587), KEND });
DECLARE_KWLIST(kwlist__should_start_end_max, { KEX("should", 0x28877b82, 0xbb3cb749df0a8b51), KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("max", 0xc293979b, 0x822bd5c706bd9850), KEND });
DECLARE_KWLIST(kwlist__start_end, { KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEND });
DECLARE_KWLIST(kwlist__start_end_filler, { KEX("start", 0xa2ed6890, 0x80b621ce3c3982d5), KEX("end", 0x37fb4a05, 0x6de935c204dc3d01), KEX("filler", 0xb990988f, 0x6067d27b1e35cd17), KEND });
/*[[[end]]]*/

DECL_END

#endif /* !GUARD_DEX_COLLECTIONS_KWLIST_H */
