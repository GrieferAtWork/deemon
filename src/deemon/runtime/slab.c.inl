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
#ifndef GUARD_DEEMON_RUNTIME_SLAB_C_INL
#define GUARD_DEEMON_RUNTIME_SLAB_C_INL 1

#include "misc.c"

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/gc.h>

DECL_BEGIN

#undef Dee_Malloc
#undef Dee_Calloc
#undef Dee_Realloc
#undef Dee_TryMalloc
#undef Dee_TryCalloc
#undef Dee_TryRealloc
#undef Dee_Free
#undef DeeObject_Free

DECL_END

#ifndef __INTELLISENSE__
/*[[[deemon
for (local x = 20; x >= 2; x = x - 1) {
    print "#if DEEMON_SLAB_HASSIZE(" + x + ")";
    print "#define SIZE",x;
    print "#include \"slab-impl.c.inl\"";
    print "#define NEXT_LARGER",x;
    print "#endif /" "* DEEMON_SLAB_HASSIZE(" + x + ") *" "/";
}
print "#undef NEXT_LARGER";
]]]*/
#if DEEMON_SLAB_HASSIZE(20)
#define SIZE 20
#include "slab-impl.c.inl"
#define NEXT_LARGER 20
#endif /* DEEMON_SLAB_HASSIZE(20) */
#if DEEMON_SLAB_HASSIZE(19)
#define SIZE 19
#include "slab-impl.c.inl"
#define NEXT_LARGER 19
#endif /* DEEMON_SLAB_HASSIZE(19) */
#if DEEMON_SLAB_HASSIZE(18)
#define SIZE 18
#include "slab-impl.c.inl"
#define NEXT_LARGER 18
#endif /* DEEMON_SLAB_HASSIZE(18) */
#if DEEMON_SLAB_HASSIZE(17)
#define SIZE 17
#include "slab-impl.c.inl"
#define NEXT_LARGER 17
#endif /* DEEMON_SLAB_HASSIZE(17) */
#if DEEMON_SLAB_HASSIZE(16)
#define SIZE 16
#include "slab-impl.c.inl"
#define NEXT_LARGER 16
#endif /* DEEMON_SLAB_HASSIZE(16) */
#if DEEMON_SLAB_HASSIZE(15)
#define SIZE 15
#include "slab-impl.c.inl"
#define NEXT_LARGER 15
#endif /* DEEMON_SLAB_HASSIZE(15) */
#if DEEMON_SLAB_HASSIZE(14)
#define SIZE 14
#include "slab-impl.c.inl"
#define NEXT_LARGER 14
#endif /* DEEMON_SLAB_HASSIZE(14) */
#if DEEMON_SLAB_HASSIZE(13)
#define SIZE 13
#include "slab-impl.c.inl"
#define NEXT_LARGER 13
#endif /* DEEMON_SLAB_HASSIZE(13) */
#if DEEMON_SLAB_HASSIZE(12)
#define SIZE 12
#include "slab-impl.c.inl"
#define NEXT_LARGER 12
#endif /* DEEMON_SLAB_HASSIZE(12) */
#if DEEMON_SLAB_HASSIZE(11)
#define SIZE 11
#include "slab-impl.c.inl"
#define NEXT_LARGER 11
#endif /* DEEMON_SLAB_HASSIZE(11) */
#if DEEMON_SLAB_HASSIZE(10)
#define SIZE 10
#include "slab-impl.c.inl"
#define NEXT_LARGER 10
#endif /* DEEMON_SLAB_HASSIZE(10) */
#if DEEMON_SLAB_HASSIZE(9)
#define SIZE 9
#include "slab-impl.c.inl"
#define NEXT_LARGER 9
#endif /* DEEMON_SLAB_HASSIZE(9) */
#if DEEMON_SLAB_HASSIZE(8)
#define SIZE 8
#include "slab-impl.c.inl"
#define NEXT_LARGER 8
#endif /* DEEMON_SLAB_HASSIZE(8) */
#if DEEMON_SLAB_HASSIZE(7)
#define SIZE 7
#include "slab-impl.c.inl"
#define NEXT_LARGER 7
#endif /* DEEMON_SLAB_HASSIZE(7) */
#if DEEMON_SLAB_HASSIZE(6)
#define SIZE 6
#include "slab-impl.c.inl"
#define NEXT_LARGER 6
#endif /* DEEMON_SLAB_HASSIZE(6) */
#if DEEMON_SLAB_HASSIZE(5)
#define SIZE 5
#include "slab-impl.c.inl"
#define NEXT_LARGER 5
#endif /* DEEMON_SLAB_HASSIZE(5) */
#if DEEMON_SLAB_HASSIZE(4)
#define SIZE 4
#include "slab-impl.c.inl"
#define NEXT_LARGER 4
#endif /* DEEMON_SLAB_HASSIZE(4) */
#if DEEMON_SLAB_HASSIZE(3)
#define SIZE 3
#include "slab-impl.c.inl"
#define NEXT_LARGER 3
#endif /* DEEMON_SLAB_HASSIZE(3) */
#if DEEMON_SLAB_HASSIZE(2)
#define SIZE 2
#include "slab-impl.c.inl"
#define NEXT_LARGER 2
#endif /* DEEMON_SLAB_HASSIZE(2) */
#undef NEXT_LARGER
//[[[end]]]
#endif

#endif /* !GUARD_DEEMON_RUNTIME_SLAB_C_INL */
