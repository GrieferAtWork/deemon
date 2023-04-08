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
#ifndef GUARD_DEEMON_UTIL_FUTEX_H
#define GUARD_DEEMON_UTIL_FUTEX_H 1

#include "../api.h"

#ifdef CONFIG_NO_THREADS
/* No-op out the futex API (with only a single thread, the below is
 * actually equivalent, so-long as you assume that dead-locks are
 * impossible, too) */
#define DeeFutex_WakeOne(addr)                                     (void)0
#define DeeFutex_WakeAll(addr)                                     (void)0
#define DeeFutex_Wait32(addr, expected)                            0
#define DeeFutex_Wait32Timed(addr, expected, timeout_nanoseconds)  0
#define DeeFutex_WaitPtr(addr, expected)                           0
#define DeeFutex_WaitPtrTimed(addr, expected, timeout_nanoseconds) 0
#define DeeFutex_WaitInt(addr, expected)                           0
#define DeeFutex_WaitIntTimed(addr, expected, timeout_nanoseconds) 0
#if __SIZEOF_POINTER__ >= 8
#define DeeFutex_Wait64(addr, expected)                           0
#define DeeFutex_Wait64Timed(addr, expected, timeout_nanoseconds) 0
#endif /* __SIZEOF_POINTER__ >= 8 */

#else /* CONFIG_NO_THREADS */

#include <stdint.h>

DECL_BEGIN

/************************************************************************/
/* Low-level, futex-based wait/wake scheduling.                         */
/************************************************************************/

/* Wake up 1, or all waiting threads at a given address. */
DFUNDEF NONNULL((1)) void (DCALL DeeFutex_WakeOne)(void *addr);
DFUNDEF NONNULL((1)) void (DCALL DeeFutex_WakeAll)(void *addr);

/* Blocking wait if `*(uint32_t *)addr == expected', until someone calls `DeeFutex_Wake*(addr)'
 * @return: 1 : [DeeFutex_Wait32Timed] The given `timeout_nanoseconds' expired.
 * @return: 0 : Success (someone called `DeeFutex_Wake*(addr)', or `*addr != expected', or spurious wake-up)
 * @return: -1: Error (an error was thrown) */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeFutex_Wait32)(void *addr, uint32_t expected);
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeFutex_Wait32Timed)(void *addr, uint32_t expected,
                             uint64_t timeout_nanoseconds);

#if __SIZEOF_POINTER__ >= 8
/* Same as above, but do a 64-bit equals-comparison test. */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeFutex_Wait64)(void *addr, uint64_t expected);
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeFutex_Wait64Timed)(void *addr, uint64_t expected,
                             uint64_t timeout_nanoseconds);
#define DeeFutex_WaitPtr      DeeFutex_Wait64
#define DeeFutex_WaitPtrTimed DeeFutex_Wait64Timed
#else /* __SIZEOF_POINTER__ >= 8 */
#define DeeFutex_WaitPtr      DeeFutex_Wait32
#define DeeFutex_WaitPtrTimed DeeFutex_Wait32Timed
#endif /* __SIZEOF_POINTER__ < 8 */


/* Same as above, but don't check for interrupts (WARNING: Don't abuse this,
 * and don't expose this to user-code. If this is abused, CTRL+C may not be
 * able to kill deemon!)
 * @return: 1 : Timeout expired (`*Timed' only)
 * @return: 0 : Success (`*Timed' only) */
DFUNDEF WUNUSED NONNULL((1)) void
(DCALL DeeFutex_Wait32NoInt)(void *addr, uint32_t expected);
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeFutex_Wait32NoIntTimed)(void *addr, uint32_t expected,
                                  uint64_t timeout_nanoseconds);
#if __SIZEOF_POINTER__ >= 8
/* Same as above, but do a 64-bit equals-comparison test. */
DFUNDEF WUNUSED NONNULL((1)) void
(DCALL DeeFutex_Wait64NoInt)(void *addr, uint64_t expected);
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeFutex_Wait64NoIntTimed)(void *addr, uint64_t expected,
                                  uint64_t timeout_nanoseconds);
#define DeeFutex_WaitPtrNoInt      DeeFutex_Wait64NoInt
#define DeeFutex_WaitPtrNoIntTimed DeeFutex_Wait64NoIntTimed
#else /* __SIZEOF_POINTER__ >= 8 */
#define DeeFutex_WaitPtrNoInt      DeeFutex_Wait32NoInt
#define DeeFutex_WaitPtrNoIntTimed DeeFutex_Wait32NoIntTimed
#endif /* __SIZEOF_POINTER__ < 8 */

#if __SIZEOF_INT__ <= 4
#define DeeFutex_WaitInt           DeeFutex_Wait32
#define DeeFutex_WaitIntTimed      DeeFutex_Wait32Timed
#define DeeFutex_WaitIntNoInt      DeeFutex_Wait32NoInt
#define DeeFutex_WaitIntNoIntTimed DeeFutex_Wait32NoIntTimed
#else /* __SIZEOF_INT__ <= 4 */
#define DeeFutex_WaitInt           DeeFutex_Wait64
#define DeeFutex_WaitIntTimed      DeeFutex_Wait64Timed
#define DeeFutex_WaitIntNoInt      DeeFutex_Wait64NoInt
#define DeeFutex_WaitIntNoIntTimed DeeFutex_Wait64NoIntTimed
#endif /* __SIZEOF_INT__ > 4 */

DECL_END
#endif /* !CONFIG_NO_THREADS */

#endif /* !GUARD_DEEMON_UTIL_FUTEX_H */
