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
#ifndef __GUARD_HYBRID_SYNC_THREADID_H
#define __GUARD_HYBRID_SYNC_THREADID_H 1

#include <hybrid/compiler.h>

#ifdef __KOS__
#ifdef __KERNEL__
#if __KOS_VERSION__ >= 300
#include <sched/task.h>
#else
#include <sched/percpu.h>
#endif

DECL_BEGIN

#define THREADID_SIZE  __SIZEOF_POINTER__
typedef struct task *threadid_t;
#define THREADID_INVALID_IS_ZERO 1
#define THREADID_INVALID         __NULLPTR
#define THREADID_SELF()          THIS_TASK

DECL_END

#else
#include <kos/types.h>
DECL_BEGIN

#ifdef __BUILDING_LIBC
__INTDEF /*ATTR_CONST*/ pid_t (__LIBCCALL libc_gettid)(void);
#define THREADID_SELF()        libc_gettid()
#else /* __BUILDING_LIBC */
__LIBC ATTR_CONST pid_t (__LIBCCALL __gettid)(void);
#define THREADID_SELF()        __gettid()
#endif /* !__BUILDING_LIBC */

#define THREADID_SIZE  __SIZEOF_PID_T__
typedef pid_t threadid_t;
#if 0
#define THREADID_INVALID       (-1)
#else
#define THREADID_INVALID_IS_ZERO 1 /* Not always, but good enough? */
#define THREADID_INVALID         0
#endif

DECL_END
#endif
#elif (defined(__DOS_COMPAT__) && !defined(__CRT_KOS)) || \
      (defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__MINGW32__) || defined(__WINDOWS__) || \
       defined(_WIN16) || defined(WIN16) || defined(_WIN32) || defined(WIN32) || \
       defined(_WIN64) || defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
       defined(_WIN32_WCE) || defined(WIN32_WCE))
#include <hybrid/typecore.h>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4559) /* Suppress warning about our addition of `ATTR_CONST' */
#endif
DECL_BEGIN
typedef __ULONG32_TYPE__ threadid_t;
extern ATTR_CONST ATTR_DLLIMPORT __ULONG32_TYPE__ ATTR_STDCALL GetCurrentThreadId(void);
DECL_END
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#define THREADID_SIZE            4
#define THREADID_SELF()          GetCurrentThreadId()
#define THREADID_INVALID_IS_ZERO 1
#define THREADID_INVALID         0
#elif defined(__unix__) || defined(__unix) || defined(unix)
#include <unistd.h>
#ifdef SYS_gettid
#ifdef __SIZEOF_PID_T__
#   define THREADID_SIZE            __SIZEOF_PID_T__
#elif defined(__SIZEOF_PID_T)
#   define THREADID_SIZE            __SIZEOF_PID_T
#else
#   define THREADID_SIZE            __SIZEOF_POINTER__
STATIC_ASSERT_MSG(sizeof(pid_t) == THREADID_SIZE,"Please adjust");
#endif
typedef pid_t threadid_t;
#define THREADID_SELF()          syscall(SYS_gettid)
#define THREADID_INVALID_IS_ZERO 1
#define THREADID_INVALID         0
#endif /* !SYS_gettid */
#endif

#ifndef THREADID_SELF
#if __has_include(<pthread.h>) || \
    defined(__unix__) || defined(__unix) || defined(unix)
#include <pthread.h>
#ifdef __SIZEOF_PTHREAD_T__
#   define THREADID_SIZE  __SIZEOF_PTHREAD_T__
#elif defined(__SIZEOF_PTHREAD_T)
#   define THREADID_SIZE  __SIZEOF_PTHREAD_T
#else
#   include <hybrid/typecore.h>
#   define THREADID_SIZE  __SIZEOF_POINTER__
STATIC_ASSERT_MSG(sizeof(pthread_t) == THREADID_SIZE,"Please adjust");
#endif
typedef pthread_t threadid_t;
#define THREADID_SELF()          pthread_self()
#define THREADID_INVALID_IS_ZERO 1 /* Not always, but good enough? */
#define THREADID_INVALID         0
#define THREADID_SAME(a,b)       pthread_equal(a,b)
#else
#include <pthread.h>
#include <hybrid/typecore.h>
#define THREADID_SIZE  __SIZEOF_INT__
typedef int threadid_t;
#define THREADID_SELF()          1 /* ??? */
#define THREADID_INVALID_IS_ZERO 1 /* Not always, but good enough? */
#define THREADID_INVALID         0
#endif
#endif /* !THREADID_SELF */

#ifndef THREADID_SAME
#define THREADID_SAME(a,b)     ((a) == (b))
#endif /* !THREADID_SAME */


#endif /* !__GUARD_HYBRID_SYNC_THREADID_H */
