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
#ifdef __DEEMON__

/* Accepted code generation template parameters. */
#define FILE_PARAMS                                   \
	{                                                 \
		{ "DeeObject" },                              \
		{ "DefaultNew", "DefaultNewKw" },             \
		{ "Arg0", "ArgN", "ArgN0", "ArgK", "ArgK0" }, \
		{ "Free0", "Free1", "FreeX" },                \
		{ "HeapType0", "HeapType1", "HeapTypeX" },    \
		{ "GC0", "GC1", "GCX" },                      \
	}

/* Helper for inline deemon scripts: enumerate
 * all possible template parameter permutations. */
#define getPermutations()                                                       \
	(()->{                                                                      \
		local PARAMS  = FILE_PARAMS;                                            \
		local indices = [({ 0 } * #PARAMS)...];                                 \
		for (;;) {                                                              \
			yield (for (local i, v : indices.enumerate()) PARAMS[i][v]).frozen; \
			for (local i = #indices - 1;; --i) {                                \
				if (i < 0)                                                      \
					return;                                                     \
				local c = indices[i] + 1;                                       \
				local m = #PARAMS[i];                                           \
				if (c >= m) {                                                   \
					indices[i] = 0;                                             \
				} else {                                                        \
					indices[i] = c;                                             \
					break;                                                      \
				}                                                               \
			}                                                                   \
		}                                                                       \
	})()
#endif /* __DEEMON__ */

#if !defined(__DEEMON__) || !defined(__FORMAT_SCRIPT__)

/*[[[deemon
print('#ifdef __INTELLISENSE__');
print('#include "new.c"');
local DEFAULT = "DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX";
for (local params: getPermutations()) {
	local key = f"DEFINE_{"_".join(params)}";
	print(key == DEFAULT ? "#define " : "//#define ", key);
}
print('#endif /' '* __INTELLISENSE__ *' '/');
]]]*/
#ifdef __INTELLISENSE__
#include "new.c"
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC1
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GCX
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC0
//#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC1
#define DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX
#endif /* __INTELLISENSE__ */
/*[[[end]]]*/

#include <deemon/api.h>

#include <deemon/alloc.h>  /* DeeObject_* */
#include <deemon/gc.h>     /* DeeGCObject_Free, DeeGCObject_Malloc, DeeGC_Track */
#include <deemon/kwds.h>   /* DeeKwds_Check, DeeKwds_SIZE */
#include <deemon/object.h> /* DREF, DeeObject, DeeObject_Size, DeeTypeObject, Dee_Decref_unlikely, Dee_TYPE, OBJECT_HEAD */
#include <deemon/type.h>   /* DeeObject_*, DeeType_IsGC, DeeType_IsHeapType */

#include <stddef.h> /* NULL, size_t */

/*[[[deemon
print("#if (", " + \\\n     ".join(
	for (local params: getPermutations())
		f"defined(DEFINE_{"_".join(params)})"
), ") != 1");
print('#error "Must #define exactly one of these"');
print('#endif /' '* ... *' '/');
]]]*/
#if (defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GCX) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC0) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC1) + \
     defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX)) != 1
#error "Must #define exactly one of these"
#endif /* ... */
/*[[[end]]]*/

/*[[[deemon
local isFirst = true;
for (local params: getPermutations()) {
	local key = f"DEFINE_{"_".join(params)}";
	print(isFirst ? f"#ifdef {key}" : f"#elif defined({key})");
	print("#define LOCAL_DeeObject_DefaultNew ", "_".join(params));
	for (local param: params) {
		if (param.startswith("Arg")) {
			print("#define LOCAL_PARAM_Arg DEFAULT_NEW_F_", param);
		} else if (param.endswith("0") || param.endswith("1")) {
			print("#define LOCAL_PARAM_", param[:-1], " ", param[-1:]);
		} else if (param == "DefaultNewKw") {
			print("#define LOCAL_PARAM_Kw 1");
		}
	}
	isFirst = false;
}
print('#else /' '* ... *' '/');
print('#error "Invalid configuration"');
print('#endif /' '* !... *' '/');
]]]*/
#ifdef DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free0_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free0_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free1_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free1_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free0_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free0_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free1_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free1_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free0_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free0_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free1_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free1_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC0
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_Free 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_Arg0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_Free 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_Free 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgN0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_Free 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_Free 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_HeapType 0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_HeapType 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_HeapType 1
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_HeapType 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC0)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC0
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_GC 0
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC1)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC1
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#define LOCAL_PARAM_GC 1
#elif defined(DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX)
#define LOCAL_DeeObject_DefaultNew DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX
#define LOCAL_PARAM_Kw 1
#define LOCAL_PARAM_Arg DEFAULT_NEW_F_ArgK0
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
/*[[[end]]]*/

#if !defined(LOCAL_PARAM_Arg) && !defined(__DEEMON__)
#error "Missing parameter: 'LOCAL_PARAM_Arg'"
#endif /* !LOCAL_PARAM_Arg && !__DEEMON__ */
#ifndef LOCAL_PARAM_Kw
#define LOCAL_PARAM_Kw 0
#endif /* !LOCAL_PARAM_Kw */
#ifndef LOCAL_PARAM_Free
#define LOCAL_PARAM_Free (-1)
#endif /* !LOCAL_PARAM_Free */
#ifndef LOCAL_PARAM_HeapType
#define LOCAL_PARAM_HeapType (-1)
#endif /* !LOCAL_PARAM_HeapType */
#ifndef LOCAL_PARAM_GC
#define LOCAL_PARAM_GC (-1)
#endif /* !LOCAL_PARAM_GC */

DECL_BEGIN

#ifndef GenericObject_DEFINED
#define GenericObject_DEFINED
typedef struct {
	OBJECT_HEAD
} GenericObject;
#endif /* !GenericObject_DEFINED */


/* Allocate+initialize a new instance of "tp_self"
 *
 * This function template represents the default implementation for:
 * - struct type_init::tp_new
 * - struct type_init::tp_new_kw
 * - struct type_init::tp_new_copy
 *
 * ... and forms the bridge between APIs like "DeeObject_New()", and
 * low-level type operators like "tp_ctor" / "tp_any_ctor"...
 */
PRIVATE NONNULL((1)) ATTR_INS(3, 2) DREF DeeObject *DCALL
LOCAL_DeeObject_DefaultNew(DeeTypeObject *tp_self,
                           size_t argc,
                           DeeObject *const *argv
#if LOCAL_PARAM_Kw
                           , DeeObject *kw
#define LOCAL_kw kw
#else /* LOCAL_PARAM_Kw */
#define LOCAL_kw NULL
#endif /* !LOCAL_PARAM_Kw */
                           ) {
	int error;
	DREF DeeObject *result;

	/* Check arguments when only the 0-arg constructor is present */
#if LOCAL_PARAM_Arg == DEFAULT_NEW_F_Arg0
	if unlikely(argc != 0) {
#if LOCAL_PARAM_Kw
		err_unimplemented_constructor_kw(tp_self, argc, argv, kw);
#else /* LOCAL_PARAM_Kw */
		err_unimplemented_constructor(tp_self, argc, argv);
#endif /* !LOCAL_PARAM_Kw */
		goto err;
	}
#endif /* LOCAL_PARAM_Arg == DEFAULT_NEW_F_Arg0 */

	/* Assert that "kw" is empty when no keyword-constructors are present */
#if (LOCAL_PARAM_Kw && (LOCAL_PARAM_Arg == DEFAULT_NEW_F_Arg0 || \
                        LOCAL_PARAM_Arg == DEFAULT_NEW_F_ArgN || \
                        LOCAL_PARAM_Arg == DEFAULT_NEW_F_ArgN0))
	if (kw) {
		if (DeeKwds_Check(kw)) {
			if (DeeKwds_SIZE(kw) != 0)
				goto err_no_keywords;
#define NEED_err_no_keywords
		} else {
			size_t kw_size = DeeObject_Size(kw);
			if unlikely(kw_size == (size_t)-1)
				goto err;
			if (kw_size != 0)
				goto err_no_keywords;
#define NEED_err_no_keywords
		}
	}
#endif /* LOCAL_PARAM_Kw */

	/* Allocate storage for object copy */
#if LOCAL_PARAM_Free < 0
	if (tp_self->tp_init.tp_alloc.tp_free) {
		result = (DREF DeeObject *)((*tp_self->tp_init.tp_alloc.tp_alloc)());
	} else
#endif /* LOCAL_PARAM_Free < 0 */
	{
#if LOCAL_PARAM_Free > 0
		result = (DREF DeeObject *)((*tp_self->tp_init.tp_alloc.tp_alloc)());
#elif LOCAL_PARAM_GC < 0
		result = DeeType_IsGC(tp_self)
		         ? (DREF DeeObject *)DeeGCObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size)
		         : (DREF DeeObject *)DeeObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
#elif LOCAL_PARAM_GC
		result = (DREF DeeObject *)DeeGCObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
#else /* ... */
		result = (DREF DeeObject *)DeeObject_Malloc(tp_self->tp_init.tp_alloc.tp_instance_size);
#endif /* !... */
	}
	if unlikely(!result)
		goto err;

	/* Initialize common object header of copy */
#if LOCAL_PARAM_HeapType > 0
	DeeObject_InitHeap((GenericObject *)result, tp_self);
#elif LOCAL_PARAM_HeapType == 0
	DeeObject_InitStatic((GenericObject *)result, tp_self);
#else /* ... */
	DeeObject_Init((GenericObject *)result, tp_self);
#endif /* !... */

	/* Invoke user-defined constructor (based on configured 'LOCAL_PARAM_Arg') */
#if LOCAL_PARAM_Arg == DEFAULT_NEW_F_Arg0
	error = (*tp_self->tp_init.tp_alloc.tp_ctor)(result);
#elif LOCAL_PARAM_Arg == DEFAULT_NEW_F_ArgN
	error = (*tp_self->tp_init.tp_alloc.tp_any_ctor)(result, argc, argv);
#elif LOCAL_PARAM_Arg == DEFAULT_NEW_F_ArgN0
	if (argc == 0) {
		error = (*tp_self->tp_init.tp_alloc.tp_ctor)(result);
	} else {
		error = (*tp_self->tp_init.tp_alloc.tp_any_ctor)(result, argc, argv);
	}
#elif LOCAL_PARAM_Arg == DEFAULT_NEW_F_ArgK
	error = (*tp_self->tp_init.tp_alloc.tp_any_ctor_kw)(result, argc, argv, LOCAL_kw);
#elif LOCAL_PARAM_Arg == DEFAULT_NEW_F_ArgK0
	if (argc == 0) {
#if LOCAL_PARAM_Kw
		if (kw) {
			if (DeeKwds_Check(kw)) {
				if (DeeKwds_SIZE(kw) != 0)
					goto do_invoke_any_ctor_kw;
			} else {
				size_t kw_size = DeeObject_Size(kw);
				if unlikely(kw_size == (size_t)-1)
					goto err_r;
				if (kw_size != 0)
					goto do_invoke_any_ctor_kw;
			}
		}
#endif /* LOCAL_PARAM_Kw */
		error = (*tp_self->tp_init.tp_alloc.tp_ctor)(result);
	} else {
#if LOCAL_PARAM_Kw
do_invoke_any_ctor_kw:
#endif /* LOCAL_PARAM_Kw */
		error = (*tp_self->tp_init.tp_alloc.tp_any_ctor_kw)(result, argc, argv, LOCAL_kw);
	}
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

	/* Check for errors that may have happened during constructor invocation */
	if unlikely(error)
		goto err_r;
	ASSERT(Dee_TYPE(result) == tp_self);

	/* Begin tracking the newly created object. */
#if LOCAL_PARAM_GC > 0
	result = DeeGC_Track(result);
#elif LOCAL_PARAM_GC < 0
	if (DeeType_IsGC(tp_self))
		result = DeeGC_Track(result);
#endif /* LOCAL_PARAM_GC */

	/* Return newly constructed object */
	return result;

err_r:
	/* Free optional debug data (CONFIG_TRACE_REFCHANGES) */
	DeeObject_FreeTracker(result);

	/* Free backing storage of would-have-been result */
	ASSERT(Dee_TYPE(result) == tp_self);
#if LOCAL_PARAM_Free < 0
	if (tp_self->tp_init.tp_alloc.tp_free) {
		((*tp_self->tp_init.tp_alloc.tp_free)(result));
	} else
#endif /* LOCAL_PARAM_Free < 0 */
	{
#if LOCAL_PARAM_Free > 0
		((*tp_self->tp_init.tp_alloc.tp_free)(result));
#elif LOCAL_PARAM_GC < 0
		if (DeeType_IsGC(tp_self)) {
			DeeGCObject_Free(result);
		} else {
			DeeObject_Free(result);
		}
#elif LOCAL_PARAM_GC
		DeeGCObject_Free(result);
#else /* ... */
		DeeObject_Free(result);
#endif /* !... */
	}

	/* Drop reference to "tp_self" (if one was held) */
#if LOCAL_PARAM_HeapType > 0
	Dee_Decref_unlikely(tp_self);
#elif LOCAL_PARAM_HeapType < 0
	if (DeeType_IsHeapType(tp_self))
		Dee_Decref_unlikely(tp_self);
#endif /* !... */

err:
	return NULL;
#ifdef NEED_err_no_keywords
#undef NEED_err_no_keywords
err_no_keywords:
	err_keywords_ctor_not_accepted(tp_self, kw);
	goto err;
#endif /* NEED_err_no_keywords */

#undef LOCAL_kw
}

DECL_END

#undef LOCAL_PARAM_Kw
#undef LOCAL_PARAM_Free
#undef LOCAL_PARAM_HeapType
#undef LOCAL_PARAM_GC
#undef LOCAL_PARAM_Arg
#undef LOCAL_DeeObject_DefaultNew

#endif /* !__DEEMON__ || !__FORMAT_SCRIPT__ */


/*[[[deemon
for (local params: getPermutations())
	print("#undef DEFINE_", "_".join(params));
]]]*/
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free0_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_Arg0_Free1_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_Arg0_FreeX_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free0_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN_Free1_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN_FreeX_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free0_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN0_Free1_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgN0_FreeX_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free0_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK_Free1_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK_FreeX_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free0_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK0_Free1_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNew_ArgK0_FreeX_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free0_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_Free1_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_Arg0_FreeX_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free0_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_Free1_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN_FreeX_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free0_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_Free1_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgN0_FreeX_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free0_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_Free1_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK_FreeX_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free0_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_Free1_HeapTypeX_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType0_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapType1_GCX
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC0
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GC1
#undef DEFINE_DeeObject_DefaultNewKw_ArgK0_FreeX_HeapTypeX_GCX
/*[[[end]]]*/
