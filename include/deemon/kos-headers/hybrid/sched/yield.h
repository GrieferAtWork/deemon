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
#ifndef __GUARD_HYBRID_SCHED_YIELD_H
#define __GUARD_HYBRID_SCHED_YIELD_H 1

#include <hybrid/compiler.h>
#include <hybrid/typecore.h>
#ifdef __KOS_SYSTEM_HEADERS__
#include <features.h>
#endif

DECL_BEGIN

#ifdef __CC__
#ifdef __KERNEL__

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef __task_yield_defined
#define __task_yield_defined 1

#if __KOS_VERSION__ < 300
/* Yield the remainder of the caller's quantum to the next
 * scheduled task (no-op if no task to switch to exists).
 * HINT: All registers but EAX are preserved across a call to this function.
 * @return: -EOK:       Another task was executed before this function returned.
 * @return: -EAGAIN:    There was no other task to switch to. */
FUNDEF errno_t (KCALL task_yield)(void);

#if !defined(__NO_XBLOCK) && defined(__COMPILER_HAVE_GCC_ASM) && \
   (!defined(__x86_64__) || defined(CONFIG_BUILDING_KERNEL_CORE)) && \
     defined(__i386__)
/* Take advantage of the fact that `task_yield()' doesn't clobber anything. */
#define task_yield() \
 __XBLOCK({ register errno_t __y_err; \
            __asm__ __volatile__("call task_yield\n" : "=a" (__y_err)); \
            __XRETURN __y_err; \
 })
#endif
#else
FUNDEF void (KCALL task_yield)(void);
#endif
#endif /* !__task_yield_defined */
#define SCHED_YIELD() task_yield()
#elif (defined(__DOS_COMPAT__) && !defined(__CRT_KOS)) || \
      (defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__MINGW32__) || defined(__WINDOWS__) || \
       defined(_WIN16) || defined(WIN16) || defined(_WIN32) || defined(WIN32) || \
       defined(_WIN64) || defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
       defined(_WIN32_WCE) || defined(WIN32_WCE))
__NAMESPACE_INT_BEGIN
__IMPDEF __ULONG32_TYPE__ __ATTR_STDCALL SleepEx(__ULONG32_TYPE__ __msec, __INT32_TYPE__ __alertable);
__NAMESPACE_INT_END
#   define SCHED_YIELD() (__NAMESPACE_INT_SYM SleepEx(20,0))
#elif defined(__linux__) || defined(__linux) || defined(linux) || \
      defined(__LINUX__) || defined(__LINUX) || defined(LINUX) || \
     !defined(__KOS_SYSTEM_HEADERS__)
#include <sched.h>
#define SCHED_YIELD() sched_yield()
#elif defined(__CRT_GLC)
#ifdef __BUILDING_LIBC
__INTDEF int (LIBCCALL libc_sched_yield)(void);
#define SCHED_YIELD() libc_sched_yield()
#else
__LIBC int (LIBCCALL sched_yield)(void);
#define SCHED_YIELD() sched_yield()
#endif
#elif defined(__unix__)
__LIBC int (LIBCCALL pthread_yield)(void);
#define SCHED_YIELD() pthread_yield()
#else
#define SCHED_YIELD() (void)0
#endif
#endif /* __CC__ */

DECL_END

#endif /* !__GUARD_HYBRID_SCHED_YIELD_H */
