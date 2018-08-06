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
#ifndef __GUARD_HYBRID_ASM_H
#define __GUARD_HYBRID_ASM_H 1

#include "compiler.h"

DECL_BEGIN

#if defined(__i386__) || defined(__x86_64__)
#ifdef CONFIG_NO_SMP
#define LOCK_PREFIX /* Nothing */
#else
#define LOCK_PREFIX lock
#endif
#endif

#if defined(__ASSEMBLY__) || defined(__ASSEMBLER__)
#   define L(...)                  __VA_ARGS__;
#   define GLOBAL_ASM(...)         __VA_ARGS__
#elif defined(__CC__)
#   define __PRIVATE_ASM_LINE(...) #__VA_ARGS__ "\n"
#   define L(...)                  __PRIVATE_ASM_LINE(__VA_ARGS__)
#   define GLOBAL_ASM              __asm__
#else
#   define L(...)                  /* Nothing */
#   define GLOBAL_ASM(...)         /* Nothing */
#endif
#if defined(__ASSEMBLY__) || defined(__TPP_VERSION__)
#define MACROARG(x) \x
#else
/* CPP doesn't seem to escape this backslash when using PP_STR()?
 * Don't know if that's a bug, but I do know TPP does escape it.
 * So... Add the second slash manually to work around that. */
#define MACROARG(x) \\x
#endif
#define DEFINE_PRIVATE(x)        .hidden x; .local x
#define DEFINE_INTERN(x)         .hidden x; .globl x
#define DEFINE_PUBLIC(x)                    .globl x
#define DEFINE_PRIVATE_T(x,kind) .hidden x; .local x; .type x, #kind
#define DEFINE_INTERN_T(x,kind)  .hidden x; .globl x; .type x, #kind
#define DEFINE_PUBLIC_T(x,kind)             .globl x; .type x, #kind
#define SYMEND(x)                .size x, . - x

#define PRIVATE_LABEL(x)         DEFINE_PRIVATE(x); x:
#define INTERN_LABEL(x)          DEFINE_INTERN(x); x:
#define PUBLIC_LABEL(x)          DEFINE_PUBLIC(x); x:
#define PRIVATE_OBJECT(x)        DEFINE_PRIVATE_T(x,object); x:
#define INTERN_OBJECT(x)         DEFINE_INTERN_T(x,object); x:
#define PUBLIC_OBJECT(x)         DEFINE_PUBLIC_T(x,object); x:
#define PRIVATE_ENTRY(x)         DEFINE_PRIVATE_T(x,function); x:
#define INTERN_ENTRY(x)          DEFINE_INTERN_T(x,function); x:
#define PUBLIC_ENTRY(x)          DEFINE_PUBLIC_T(x,function); x:
#define PRIVATE_CONST(x,y)       DEFINE_PRIVATE(x); .set x, y
#define INTERN_CONST(x,y)        DEFINE_INTERN(x); .set x, y
#define PUBLIC_CONST(x,y)        DEFINE_PUBLIC(x); .set x, y

#define PRIVATE_STRING(x,str)    PRIVATE_OBJECT(x) .string str; SYMEND(x)
#define INTERN_STRING(x,str)     INTERN_OBJECT(x)  .string str; SYMEND(x)
#define PUBLIC_STRING(x,str)     PUBLIC_OBJECT(x)  .string str; SYMEND(x)

#define DEFINE_BSS(x,n_bytes)    x: .skip (n_bytes); SYMEND(x)

#ifdef __PIC__
#   define PLT_SYM(x) x@PLT
#else
#   define PLT_SYM(x) x
#endif

DECL_END

#endif /* !__GUARD_HYBRID_ASM_H */
