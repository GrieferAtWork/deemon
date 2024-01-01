/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_UTIL_ONCE_H
#define GUARD_DEEMON_UTIL_ONCE_H 1

#include "../api.h"

/*
 * Dee_once_t: Perform some operation exactly once
 */

#ifdef CONFIG_NO_THREADS
#include <stdbool.h>

DECL_BEGIN

typedef bool Dee_once_t;
#define DEE_ONCE_INIT              false
#define Dee_once_init(self)        (void)(*(self) = false)
#define Dee_once_init_didrun(self) (void)(*(self) = true)
#define Dee_once_begin(self)       (*(self) ? 0 : (*(self) = true, 1))
#define Dee_once_begin_noint(self) (*(self) ? false : (*(self) = true, true))
#define Dee_once_trybegin(self)    (*(self) ? 0 : (*(self) = true, 1))
#define Dee_once_commit(self)      (void)(*(self) = true)
#define Dee_once_abort(self)       (void)(*(self) = false)
#define Dee_once_hasrun(self)      (!!*(self))
#define Dee_once_isrunning(self)   false
#define Dee_once_ispending(self)   (!*(self))

DECL_END

#else /* CONFIG_NO_THREADS */

#include <hybrid/atomic.h>
#include <hybrid/typecore.h>

#include <stdint.h>

DECL_BEGIN

#define DEE_ONCE_COMPLETED_THRESHOLD UINT32_C(0x80000000)
typedef struct {
	uint32_t oc_didrun; /* 0: Not run
	                     * 1: running
	                     * [2, DEE_ONCE_COMPLETED_THRESHOLD - 1]: Running, and threads are waiting for completion 
	                     * [DEE_ONCE_COMPLETED_THRESHOLD, (uint32_t)-1]: Did run */
} Dee_once_t;
#define DEE_ONCE_INIT { 0 }
#define Dee_once_init(self)        (void)((self)->oc_didrun = 0)
#define Dee_once_init_didrun(self) (void)((self)->oc_didrun = DEE_ONCE_COMPLETED_THRESHOLD)

/* Enter the once-block
 * @return: 1 : You're now responsible for executing the once-function
 * @return: 0 : The once-function has already been executed
 * @return: -1: An error was thrown */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL Dee_once_begin)(Dee_once_t *__restrict self);

/* Enter the once-block
 * @return: true:  You're now responsible for executing the once-function
 * @return: false: The once-function has already been executed */
DFUNDEF WUNUSED NONNULL((1)) bool
(DCALL Dee_once_begin_noint)(Dee_once_t *__restrict self);

/* Finish the once-block successfully */
DFUNDEF NONNULL((1)) void (DCALL Dee_once_commit)(Dee_once_t *__restrict self);

/* Finish the once-block with an error (causing it to be re-attempted the next time) */
DFUNDEF NONNULL((1)) void (DCALL Dee_once_abort)(Dee_once_t *__restrict self);

/* Check if `self' has already been executed. */
#define Dee_once_hasrun(self) \
	(__hybrid_atomic_load(&(self)->oc_didrun, __ATOMIC_ACQUIRE) >= DEE_ONCE_COMPLETED_THRESHOLD)
#define Dee_once_isrunning(self) \
	((uint32_t)(__hybrid_atomic_load(&(self)->oc_didrun, __ATOMIC_ACQUIRE) - 1) <= (DEE_ONCE_COMPLETED_THRESHOLD - 2))
#define Dee_once_ispending(self) \
	(__hybrid_atomic_load(&(self)->oc_didrun, __ATOMIC_ACQUIRE) == 0)

/* Try to begin execution of a once-block
 * @return: 0 : The once-function has already been executed
 * @return: 1 : You're now responsible for executing the once-function
 * @return: 2 : Another thread is currently executing the once-function (NO ERROR WAS THROWN) */
DFUNDEF WUNUSED NONNULL((1)) int (DCALL Dee_once_trybegin)(Dee_once_t *__restrict self);

#ifndef __OPTIMIZE_SIZE__
#define Dee_once_begin(self) \
	(Dee_once_hasrun(self) ? 0 : (Dee_once_begin)(self))
#define Dee_once_begin_noint(self) \
	(Dee_once_hasrun(self) ? false : (Dee_once_begin_noint)(self))
#endif /* !__OPTIMIZE_SIZE__ */

DECL_END
#endif /* !CONFIG_NO_THREADS */

/* Helper macro to perform some action exactly once. */
#define Dee_ONCE(...)                            \
	do {                                         \
		static Dee_once_t _once = DEE_ONCE_INIT; \
		if (Dee_once_begin_noint(&_once)) {      \
			__VA_ARGS__;                         \
			Dee_once_commit(&_once);             \
		}                                        \
	}	__WHILE0

#endif /* !GUARD_DEEMON_UTIL_ONCE_H */
