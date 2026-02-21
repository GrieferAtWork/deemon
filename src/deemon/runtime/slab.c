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
#ifndef GUARD_DEEMON_RUNTIME_SLAB_C
#define GUARD_DEEMON_RUNTIME_SLAB_C 1

#include <deemon/api.h>

#if defined(CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR) || defined(__DEEMON__)
#include <deemon/alloc.h>
#include <deemon/gc.h>
#include <hybrid/host.h>
#include <hybrid/align.h>
#include <deemon/util/slab.h>
#include <deemon/util/slab-config.h>

#if defined(Dee_SLAB_CHUNKSIZE_MAX) || defined(__DEEMON__)
DECL_BEGIN

#define _DeeSlab_EXISTS_CB(n, N) || n == (N)
#define DeeSlab_EXISTS(N)    (0 Dee_SLAB_CHUNKSIZE_FOREACH(_DeeSlab_EXISTS_CB, N))
#define DeeSlab_GC_EXISTS(N) (0 Dee_SLAB_CHUNKSIZE_GC_FOREACH(_DeeSlab_EXISTS_CB, N))

/* Do some sanity assertions on the slab configuration */
STATIC_ASSERT(IS_POWER_OF_TWO(Dee_SLAB_PAGESIZE));
STATIC_ASSERT(Dee_SLAB_PAGESIZE > Dee_SLAB_CHUNKSIZE_MAX);


/* TODO: Slab page allocator / free function (try to use os-specific
 *       mmap() / munmap() if allowed by configured `Dee_SLAB_PAGESIZE',
 *       but fall back to simply using `Dee_TryMemalign()' / `Dee_free()') */



/* TODO: Define slab GC allocators (use "Dee_SLAB_CHUNKSIZE_GC_FOREACH()" to
 *       enumerate and call-forward to the relevant, matching slab function) */

DECL_END

/* Slab implementations... */
#ifndef __INTELLISENSE__
/*[[[deemon
local minsize = 12;
local maxsize = 80;
print("#if Dee_SLAB_CHUNKSIZE_MIN < ", minsize);
print("#error \"Configured value 'Dee_SLAB_CHUNKSIZE_MIN' is less than supported minimum ", minsize, "\"");
print("#endif /" "* Dee_SLAB_CHUNKSIZE_MIN < ", minsize, " *" "/");
print("#if Dee_SLAB_CHUNKSIZE_MAX > ", maxsize);
print("#error \"Configured value 'Dee_SLAB_CHUNKSIZE_MAX' is greater than supported maximum ", maxsize, "\"");
print("#endif /" "* Dee_SLAB_CHUNKSIZE_MAX > ", maxsize, " *" "/");
for (local n: [minsize:maxsize+1]) {
	print("#if DeeSlab_EXISTS(", n, ")");
	print("#define DEFINE_CHUNK_SIZE ", n);
	print("#include \"slab-sized.c.inl\"");
	print("#endif /" "* DeeSlab_EXISTS(", n, ") *"  "/");
}
]]]*/
#if Dee_SLAB_CHUNKSIZE_MIN < 12
#error "Configured value 'Dee_SLAB_CHUNKSIZE_MIN' is less than supported minimum 12"
#endif /* Dee_SLAB_CHUNKSIZE_MIN < 12 */
#if Dee_SLAB_CHUNKSIZE_MAX > 80
#error "Configured value 'Dee_SLAB_CHUNKSIZE_MAX' is greater than supported maximum 80"
#endif /* Dee_SLAB_CHUNKSIZE_MAX > 80 */
#if DeeSlab_EXISTS(12)
#define DEFINE_CHUNK_SIZE 12
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(12) */
#if DeeSlab_EXISTS(13)
#define DEFINE_CHUNK_SIZE 13
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(13) */
#if DeeSlab_EXISTS(14)
#define DEFINE_CHUNK_SIZE 14
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(14) */
#if DeeSlab_EXISTS(15)
#define DEFINE_CHUNK_SIZE 15
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(15) */
#if DeeSlab_EXISTS(16)
#define DEFINE_CHUNK_SIZE 16
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(16) */
#if DeeSlab_EXISTS(17)
#define DEFINE_CHUNK_SIZE 17
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(17) */
#if DeeSlab_EXISTS(18)
#define DEFINE_CHUNK_SIZE 18
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(18) */
#if DeeSlab_EXISTS(19)
#define DEFINE_CHUNK_SIZE 19
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(19) */
#if DeeSlab_EXISTS(20)
#define DEFINE_CHUNK_SIZE 20
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(20) */
#if DeeSlab_EXISTS(21)
#define DEFINE_CHUNK_SIZE 21
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(21) */
#if DeeSlab_EXISTS(22)
#define DEFINE_CHUNK_SIZE 22
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(22) */
#if DeeSlab_EXISTS(23)
#define DEFINE_CHUNK_SIZE 23
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(23) */
#if DeeSlab_EXISTS(24)
#define DEFINE_CHUNK_SIZE 24
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(24) */
#if DeeSlab_EXISTS(25)
#define DEFINE_CHUNK_SIZE 25
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(25) */
#if DeeSlab_EXISTS(26)
#define DEFINE_CHUNK_SIZE 26
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(26) */
#if DeeSlab_EXISTS(27)
#define DEFINE_CHUNK_SIZE 27
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(27) */
#if DeeSlab_EXISTS(28)
#define DEFINE_CHUNK_SIZE 28
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(28) */
#if DeeSlab_EXISTS(29)
#define DEFINE_CHUNK_SIZE 29
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(29) */
#if DeeSlab_EXISTS(30)
#define DEFINE_CHUNK_SIZE 30
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(30) */
#if DeeSlab_EXISTS(31)
#define DEFINE_CHUNK_SIZE 31
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(31) */
#if DeeSlab_EXISTS(32)
#define DEFINE_CHUNK_SIZE 32
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(32) */
#if DeeSlab_EXISTS(33)
#define DEFINE_CHUNK_SIZE 33
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(33) */
#if DeeSlab_EXISTS(34)
#define DEFINE_CHUNK_SIZE 34
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(34) */
#if DeeSlab_EXISTS(35)
#define DEFINE_CHUNK_SIZE 35
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(35) */
#if DeeSlab_EXISTS(36)
#define DEFINE_CHUNK_SIZE 36
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(36) */
#if DeeSlab_EXISTS(37)
#define DEFINE_CHUNK_SIZE 37
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(37) */
#if DeeSlab_EXISTS(38)
#define DEFINE_CHUNK_SIZE 38
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(38) */
#if DeeSlab_EXISTS(39)
#define DEFINE_CHUNK_SIZE 39
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(39) */
#if DeeSlab_EXISTS(40)
#define DEFINE_CHUNK_SIZE 40
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(40) */
#if DeeSlab_EXISTS(41)
#define DEFINE_CHUNK_SIZE 41
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(41) */
#if DeeSlab_EXISTS(42)
#define DEFINE_CHUNK_SIZE 42
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(42) */
#if DeeSlab_EXISTS(43)
#define DEFINE_CHUNK_SIZE 43
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(43) */
#if DeeSlab_EXISTS(44)
#define DEFINE_CHUNK_SIZE 44
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(44) */
#if DeeSlab_EXISTS(45)
#define DEFINE_CHUNK_SIZE 45
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(45) */
#if DeeSlab_EXISTS(46)
#define DEFINE_CHUNK_SIZE 46
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(46) */
#if DeeSlab_EXISTS(47)
#define DEFINE_CHUNK_SIZE 47
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(47) */
#if DeeSlab_EXISTS(48)
#define DEFINE_CHUNK_SIZE 48
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(48) */
#if DeeSlab_EXISTS(49)
#define DEFINE_CHUNK_SIZE 49
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(49) */
#if DeeSlab_EXISTS(50)
#define DEFINE_CHUNK_SIZE 50
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(50) */
#if DeeSlab_EXISTS(51)
#define DEFINE_CHUNK_SIZE 51
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(51) */
#if DeeSlab_EXISTS(52)
#define DEFINE_CHUNK_SIZE 52
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(52) */
#if DeeSlab_EXISTS(53)
#define DEFINE_CHUNK_SIZE 53
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(53) */
#if DeeSlab_EXISTS(54)
#define DEFINE_CHUNK_SIZE 54
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(54) */
#if DeeSlab_EXISTS(55)
#define DEFINE_CHUNK_SIZE 55
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(55) */
#if DeeSlab_EXISTS(56)
#define DEFINE_CHUNK_SIZE 56
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(56) */
#if DeeSlab_EXISTS(57)
#define DEFINE_CHUNK_SIZE 57
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(57) */
#if DeeSlab_EXISTS(58)
#define DEFINE_CHUNK_SIZE 58
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(58) */
#if DeeSlab_EXISTS(59)
#define DEFINE_CHUNK_SIZE 59
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(59) */
#if DeeSlab_EXISTS(60)
#define DEFINE_CHUNK_SIZE 60
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(60) */
#if DeeSlab_EXISTS(61)
#define DEFINE_CHUNK_SIZE 61
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(61) */
#if DeeSlab_EXISTS(62)
#define DEFINE_CHUNK_SIZE 62
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(62) */
#if DeeSlab_EXISTS(63)
#define DEFINE_CHUNK_SIZE 63
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(63) */
#if DeeSlab_EXISTS(64)
#define DEFINE_CHUNK_SIZE 64
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(64) */
#if DeeSlab_EXISTS(65)
#define DEFINE_CHUNK_SIZE 65
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(65) */
#if DeeSlab_EXISTS(66)
#define DEFINE_CHUNK_SIZE 66
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(66) */
#if DeeSlab_EXISTS(67)
#define DEFINE_CHUNK_SIZE 67
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(67) */
#if DeeSlab_EXISTS(68)
#define DEFINE_CHUNK_SIZE 68
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(68) */
#if DeeSlab_EXISTS(69)
#define DEFINE_CHUNK_SIZE 69
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(69) */
#if DeeSlab_EXISTS(70)
#define DEFINE_CHUNK_SIZE 70
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(70) */
#if DeeSlab_EXISTS(71)
#define DEFINE_CHUNK_SIZE 71
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(71) */
#if DeeSlab_EXISTS(72)
#define DEFINE_CHUNK_SIZE 72
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(72) */
#if DeeSlab_EXISTS(73)
#define DEFINE_CHUNK_SIZE 73
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(73) */
#if DeeSlab_EXISTS(74)
#define DEFINE_CHUNK_SIZE 74
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(74) */
#if DeeSlab_EXISTS(75)
#define DEFINE_CHUNK_SIZE 75
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(75) */
#if DeeSlab_EXISTS(76)
#define DEFINE_CHUNK_SIZE 76
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(76) */
#if DeeSlab_EXISTS(77)
#define DEFINE_CHUNK_SIZE 77
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(77) */
#if DeeSlab_EXISTS(78)
#define DEFINE_CHUNK_SIZE 78
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(78) */
#if DeeSlab_EXISTS(79)
#define DEFINE_CHUNK_SIZE 79
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(79) */
#if DeeSlab_EXISTS(80)
#define DEFINE_CHUNK_SIZE 80
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(80) */
/*[[[end]]]*/
#endif /* !__INTELLISENSE__ */
#endif /* Dee_SLAB_CHUNKSIZE_MAX */

#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

#endif /* !GUARD_DEEMON_RUNTIME_SLAB_C */
