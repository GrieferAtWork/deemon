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
/*!export **/
/*!export Dee_atomic_lock_**/
/*!export Dee_atomic_rwlock_**/
/*!export _Dee_generic_lock_**/
#ifndef GUARD_DEEMON_UTIL_LOCK_UTILS_H
#define GUARD_DEEMON_UTIL_LOCK_UTILS_H 1 /*!export-*/

#include "../api.h"

#include <stdint.h> /* uint64_t */

#ifdef CONFIG_NO_THREADS
#define Dee_atomic_lock_acquire_p(self, err_label)                                               (void)0
#define Dee_atomic_lock_waitfor_p(self, err_label)                                               (void)0
#define Dee_atomic_lock_acquire_timed_p(self, timeout_nanoseconds, err_label, timeout_label)     (void)0
#define Dee_atomic_lock_waitfor_timed_p(self, timeout_nanoseconds, err_label, timeout_label)     (void)0
#define Dee_atomic_rwlock_read_p(self, err_label)                                                (void)0
#define Dee_atomic_rwlock_write_p(self, err_label)                                               (void)0
#define Dee_atomic_rwlock_waitread_p(self, err_label)                                            (void)0
#define Dee_atomic_rwlock_waitwrite_p(self, err_label)                                           (void)0
#define Dee_atomic_rwlock_read_timed_p(self, timeout_nanoseconds, err_label, timeout_label)      (void)0
#define Dee_atomic_rwlock_write_timed_p(self, timeout_nanoseconds, err_label, timeout_label)     (void)0
#define Dee_atomic_rwlock_waitread_timed_p(self, timeout_nanoseconds, err_label, timeout_label)  (void)0
#define Dee_atomic_rwlock_waitwrite_timed_p(self, timeout_nanoseconds, err_label, timeout_label) (void)0
#else /* CONFIG_NO_THREADS */

#include <hybrid/__overflow.h>    /* __hybrid_overflow_uadd, __hybrid_overflow_usub */
#include <hybrid/sched/__yield.h> /* __hybrid_yield */

#include "../thread.h" /* DeeThread_CheckInterrupt, DeeThread_GetTimeMicroSeconds */
#include "lock.h"      /* Dee_atomic_lock_available, Dee_atomic_lock_tryacquire, Dee_atomic_rwlock_* */

DECL_BEGIN

#define _Dee_generic_lock_spinuntil_p(ok, err_label) \
	do {                                             \
		while (!(ok)) {                              \
			if (DeeThread_CheckInterrupt())          \
				goto err_label;                      \
			__hybrid_yield();                        \
		}                                            \
	}	__WHILE0
#define _Dee_generic_lock_spinuntil_timed_p(ok, timeout_nanoseconds, err_label, timeout_label) \
	do {                                                                                       \
		if (!(ok)) {                                                                           \
			uint64_t _now_microseconds, _then_microseconds;                                    \
			if ((timeout_nanoseconds == (uint64_t)-1) ||                                       \
			    (_now_microseconds = DeeThread_GetTimeMicroSeconds(),                          \
			     __hybrid_overflow_uadd(_now_microseconds,                                     \
			                            timeout_nanoseconds / 1000,                            \
			                            &_then_microseconds))) {                               \
				_Dee_generic_lock_spinuntil_p(ok, err_label);                                  \
			} else {                                                                           \
				while (!(ok)) {                                                                \
					_now_microseconds = DeeThread_GetTimeMicroSeconds();                       \
					if (__hybrid_overflow_usub(_then_microseconds, _now_microseconds,          \
					                           &timeout_nanoseconds))                          \
						goto timeout_label; /* Timeout */                                      \
					timeout_nanoseconds *= 1000;                                               \
					if (DeeThread_CheckInterrupt())                                            \
						goto err;                                                              \
					__hybrid_yield();                                                          \
				}                                                                              \
			}                                                                                  \
		}                                                                                      \
	}	__WHILE0

/* Atomic lock acquire with interrupt checks (+ optional timeout) */
#define Dee_atomic_lock_acquire_p(self, err_label) \
	_Dee_generic_lock_spinuntil_p(Dee_atomic_lock_tryacquire(self), err_label)
#define Dee_atomic_lock_waitfor_p(self, err_label) \
	_Dee_generic_lock_spinuntil_p(Dee_atomic_lock_available(self), err_label)
#define Dee_atomic_lock_acquire_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	_Dee_generic_lock_spinuntil_timed_p(Dee_atomic_lock_tryacquire(self), timeout_nanoseconds, err_label, timeout_label)
#define Dee_atomic_lock_waitfor_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	_Dee_generic_lock_spinuntil_timed_p(Dee_atomic_lock_available(self), timeout_nanoseconds, err_label, timeout_label)

/* Atomic rwlock acquire with interrupt checks (+ optional timeout) */
#define Dee_atomic_rwlock_read_p(self, err_label) \
	_Dee_generic_lock_spinuntil_p(Dee_atomic_rwlock_tryread(self), err_label)
#define Dee_atomic_rwlock_write_p(self, err_label) \
	_Dee_generic_lock_spinuntil_p(Dee_atomic_rwlock_trywrite(self), err_label)
#define Dee_atomic_rwlock_waitread_p(self, err_label) \
	_Dee_generic_lock_spinuntil_p(Dee_atomic_rwlock_canread(self), err_label)
#define Dee_atomic_rwlock_waitwrite_p(self, err_label) \
	_Dee_generic_lock_spinuntil_p(Dee_atomic_rwlock_canwrite(self), err_label)
#define Dee_atomic_rwlock_read_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	_Dee_generic_lock_spinuntil_timed_p(Dee_atomic_rwlock_tryread(self), timeout_nanoseconds, err_label, timeout_label)
#define Dee_atomic_rwlock_write_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	_Dee_generic_lock_spinuntil_timed_p(Dee_atomic_rwlock_trywrite(self), timeout_nanoseconds, err_label, timeout_label)
#define Dee_atomic_rwlock_waitread_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	_Dee_generic_lock_spinuntil_timed_p(Dee_atomic_rwlock_canread(self), timeout_nanoseconds, err_label, timeout_label)
#define Dee_atomic_rwlock_waitwrite_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	_Dee_generic_lock_spinuntil_timed_p(Dee_atomic_rwlock_canwrite(self), timeout_nanoseconds, err_label, timeout_label)

DECL_END
#endif /* !CONFIG_NO_THREADS */

#endif /* !GUARD_DEEMON_UTIL_LOCK_UTILS_H */
