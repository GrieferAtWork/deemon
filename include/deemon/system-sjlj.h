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
#ifndef GUARD_DEEMON_SYSTEM_SJLF_H
#define GUARD_DEEMON_SYSTEM_SJLF_H 1

#include "api.h"
/**/

#include "system-features.h"

#undef DeeSystem_JmpBuf
#undef DeeSystem_SetJmp
#undef DeeSystem_LongJmp
#ifdef CONFIG_HAVE_SETJMP_H
#if defined(__USE_FORTIFY_LEVEL) && (__USE_FORTIFY_LEVEL + 0) > 0
/* Work around a problem with `__longjmp_chk()', which compares stack-pointers
 * to see if we try to jump up the stack. However, we use setjmp to jump to an
 * entirely different stack, which may have been allocated such that it appears
 * at a lower address than the source (making __longjmp_chk think that we did
 * an invalid jump when that isn't actually the case)
 * 
 * The bug here is that __longjmp_chk always simply does:
 * >> assert(TARGET_SP >= MY_SP);
 *
 * When it should do this:
 * >> assert(TARGET_SP >= MY_SP || !IN_STACK_BOUNDS(pthread_getstack(pthread_self()), TARGET_SP));
 *
 * Reminder: we just setjmp/longjmp to switch between 2 stacks!
 *
 * As such, we have to disable __USE_FORTIFY_LEVEL for setjmp.h */
#ifdef _SETJMP_H
#ifdef CONFIG_HAVE_longjmp
#undef longjmp
__COMPILER_REDIRECT_VOID(extern,__ATTR_NOTHROW,,,__real_longjmp,(jmp_buf __env, int __val),longjmp,(__env,__val))
#define longjmp __real_longjmp
#endif /* CONFIG_HAVE_longjmp */
#ifdef CONFIG_HAVE__longjmp
#undef _longjmp
__COMPILER_REDIRECT_VOID(extern,__ATTR_NOTHROW,,,__real__longjmp,(jmp_buf __env, int __val),_longjmp,(__env,__val))
#define _longjmp __real__longjmp
#endif /* CONFIG_HAVE__longjmp */
#ifdef CONFIG_HAVE_siglongjmp
#undef siglongjmp
__COMPILER_REDIRECT_VOID(extern,__ATTR_NOTHROW,,,__real_siglongjmp,(jmp_buf __env, int __val),siglongjmp,(__env,__val))
#define siglongjmp __real_siglongjmp
#endif /* CONFIG_HAVE_siglongjmp */
#else /* _SETJMP_H */
#ifndef _FEATURES_H
#error "How wasn't this included already? I mean: we did include <deemon/system-features.h>"
#endif /* !_FEATURES_H */
#if __USE_FORTIFY_LEVEL == 2
#define __PRIVATE_FORTIFY_LEVEL_WAS_2
#endif /* __USE_FORTIFY_LEVEL == 2 */
#undef __USE_FORTIFY_LEVEL
#define __USE_FORTIFY_LEVEL 0
#include <setjmp.h>
#undef __USE_FORTIFY_LEVEL
#ifdef __PRIVATE_FORTIFY_LEVEL_WAS_2
#undef __PRIVATE_FORTIFY_LEVEL_WAS_2
#define __USE_FORTIFY_LEVEL 2
#else /* __PRIVATE_FORTIFY_LEVEL_WAS_2 */
#define __USE_FORTIFY_LEVEL 1
#endif /* !__PRIVATE_FORTIFY_LEVEL_WAS_2 */
#endif /* !_SETJMP_H */
#else /* __USE_FORTIFY_LEVEL > 0 */
#include <setjmp.h>
#endif /* __USE_FORTIFY_LEVEL <= 0  */

/* Figure out which sjlj-pair we want to use. For this purpose,
 * try to select whatever will be the easiest on the runtime. */
#if defined(CONFIG_HAVE__longjmp) && defined(CONFIG_HAVE__setjmp)
#define DeeSystem_JmpBuf            jmp_buf
#define DeeSystem_SetJmp(env)       _setjmp(env)
#define DeeSystem_LongJmp(env, sig) _longjmp(env, sig)
#elif defined(CONFIG_HAVE_siglongjmp) && defined(CONFIG_HAVE_sigsetjmp)
#define DeeSystem_JmpBuf            sigjmp_buf
#define DeeSystem_SetJmp(env)       sigsetjmp(env, 0)
#define DeeSystem_LongJmp(env, sig) siglongjmp(env, sig)
#elif defined(CONFIG_HAVE_longjmp) && defined(CONFIG_HAVE_setjmp)
#define DeeSystem_JmpBuf            jmp_buf
#define DeeSystem_SetJmp(env)       setjmp(env)
#define DeeSystem_LongJmp(env, sig) longjmp(env, sig)
#endif /* ... */

#ifdef _MSC_VER
#pragma warning(disable: 4611) /* Some nonsensical warning about how setjmp() is evil... */
#endif /* _MSC_VER */

#endif /* CONFIG_HAVE_SETJMP_H */


#endif /* !GUARD_DEEMON_SYSTEM_SJLF_H */
