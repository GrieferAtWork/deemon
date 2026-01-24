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
/*!export Dee_ratomic_lock_**/
/*!export Dee_ratomic_rwlock_**/
/*!export _Dee_ratomic_lock_**/
/*!export _Dee_ratomic_rwlock_**/
#ifndef GUARD_DEEMON_UTIL_RLOCK_UTILS_H
#define GUARD_DEEMON_UTIL_RLOCK_UTILS_H 1

#include "../api.h"

#ifdef CONFIG_NO_THREADS
#define Dee_ratomic_lock_acquire_p(self, err_label)                                               (void)0
#define Dee_ratomic_lock_waitfor_p(self, err_label)                                               (void)0
#define Dee_ratomic_lock_acquire_timed_p(self, timeout_nanoseconds, err_label, timeout_label)     (void)0
#define Dee_ratomic_lock_waitfor_timed_p(self, timeout_nanoseconds, err_label, timeout_label)     (void)0
#define Dee_ratomic_rwlock_read_p(self, err_label)                                                (void)0
#define Dee_ratomic_rwlock_write_p(self, err_label)                                               (void)0
#define Dee_ratomic_rwlock_waitread_p(self, err_label)                                            (void)0
#define Dee_ratomic_rwlock_waitwrite_p(self, err_label)                                           (void)0
#define Dee_ratomic_rwlock_read_timed_p(self, timeout_nanoseconds, err_label, timeout_label)      (void)0
#define Dee_ratomic_rwlock_write_timed_p(self, timeout_nanoseconds, err_label, timeout_label)     (void)0
#define Dee_ratomic_rwlock_waitread_timed_p(self, timeout_nanoseconds, err_label, timeout_label)  (void)0
#define Dee_ratomic_rwlock_waitwrite_timed_p(self, timeout_nanoseconds, err_label, timeout_label) (void)0
#else /* CONFIG_NO_THREADS */

#include <hybrid/__atomic.h>       /* __ATOMIC_ACQUIRE, __hybrid_atomic_* */
#include <hybrid/__overflow.h>     /* __hybrid_overflow_uadd, __hybrid_overflow_usub */
#include <hybrid/sched/__gettid.h> /* __hybrid_gettid, __hybrid_gettid_iscaller */
#include <hybrid/sched/__yield.h>  /* __hybrid_yield */

#include "../thread.h"
#include "rlock.h"     /* Dee_ratomic_lock_t, Dee_ratomic_rwlock_t */

#include <stdint.h> /* uint64_t, uintptr_t */

DECL_BEGIN

/************************************************************************/
/* Dee_ratomic_lock_t                                                   */
/************************************************************************/

LOCAL WUNUSED NONNULL((1)) int DCALL
_Dee_ratomic_lock_acquire_p_impl(Dee_ratomic_lock_t *__restrict self) {
	if (__hybrid_atomic_load(&self->ra_lock, __ATOMIC_ACQUIRE) == 0) {
		if (!__hybrid_atomic_cmpxch(&self->ra_lock, 0, 1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			goto waitfor;
settid:
		self->ra_tid = __hybrid_gettid();
		return 0;
	}
	if (__hybrid_gettid_iscaller(self->ra_tid)) {
		__hybrid_atomic_inc(&self->ra_lock, __ATOMIC_ACQUIRE);
		return 0;
	}
waitfor:
	while (__hybrid_atomic_load(&self->ra_lock, __ATOMIC_ACQUIRE) != 0) {
		if (DeeThread_CheckInterrupt())
			goto err;
		__hybrid_yield();
	}
	if (__hybrid_atomic_cmpxch(&self->ra_lock, 0, 1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
		goto settid;
	goto waitfor;
err:
	return -1;
}

LOCAL WUNUSED NONNULL((1)) int DCALL
_Dee_ratomic_lock_acquire_timed_p_impl(Dee_ratomic_lock_t *__restrict self,
                                       uint64_t timeout_nanoseconds) {
	uint64_t now_microseconds, then_microseconds;
	if (__hybrid_atomic_load(&self->ra_lock, __ATOMIC_ACQUIRE) == 0) {
		if (!__hybrid_atomic_cmpxch(&self->ra_lock, 0, 1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			goto waitfor;
settid:
		self->ra_tid = __hybrid_gettid();
		return 0;
	}
	if (__hybrid_gettid_iscaller(self->ra_tid)) {
		__hybrid_atomic_inc(&self->ra_lock, __ATOMIC_ACQUIRE);
		return 0;
	}
waitfor:
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		while (__hybrid_atomic_load(&self->ra_lock, __ATOMIC_ACQUIRE) != 0) {
			if (DeeThread_CheckInterrupt())
				goto err;
			__hybrid_yield();
		}
		if (__hybrid_atomic_cmpxch(&self->ra_lock, 0, 1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			goto settid;
		goto do_infinite_timeout;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (__hybrid_overflow_uadd(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	while (__hybrid_atomic_load(&self->ra_lock, __ATOMIC_ACQUIRE) == 0) {
		if (__hybrid_atomic_cmpxch(&self->ra_lock, 0, 1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			goto settid;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (__hybrid_overflow_usub(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	if (DeeThread_CheckInterrupt())
		goto err;
	__hybrid_yield();
	goto do_wait_with_timeout;
err:
	return -1;
}

#define Dee_ratomic_lock_acquire_p(self, err_label)         \
	do {                                                    \
		if unlikely(_Dee_ratomic_lock_acquire_p_impl(self)) \
			goto err_label;                                 \
	}	__WHILE0
#define Dee_ratomic_lock_waitfor_p(self, err_label)                                 \
	do {                                                                            \
		if (__hybrid_atomic_load(&(self)->ra_lock, __ATOMIC_ACQUIRE) != 0 &&        \
		    !__hybrid_gettid_iscaller((self)->ra_tid)) {                            \
			while (__hybrid_atomic_load(&(self)->ra_lock, __ATOMIC_ACQUIRE) != 0) { \
				if (DeeThread_CheckInterrupt())                                     \
					goto err_label;                                                 \
				__hybrid_yield();                                                   \
			}                                                                       \
		}                                                                           \
	}	__WHILE0
#define Dee_ratomic_lock_acquire_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	do {                                                                                      \
		int _status = _Dee_ratomic_lock_acquire_timed_p_impl(self, timeout_nanoseconds);      \
		if unlikely(_status != 0) {                                                           \
			if unlikely(_status < 0)                                                          \
				goto err_label;                                                               \
			goto timeout_label;                                                               \
		}                                                                                     \
	}	__WHILE0
#define Dee_ratomic_lock_waitfor_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	do {                                                                                      \
		if (__hybrid_atomic_load(&(self)->ra_lock, __ATOMIC_ACQUIRE) != 0 &&                  \
		    !__hybrid_gettid_iscaller((self)->ra_tid)) {                                      \
			uint64_t _now_microseconds, _then_microseconds;                                   \
			if ((timeout_nanoseconds == (uint64_t)-1) ||                                      \
			    (_now_microseconds = DeeThread_GetTimeMicroSeconds(),                         \
			     __hybrid_overflow_uadd(_now_microseconds,                                    \
			                            timeout_nanoseconds / 1000,                           \
			                            &_then_microseconds))) {                              \
				while (__hybrid_atomic_load(&(self)->ra_lock, __ATOMIC_ACQUIRE) != 0) {       \
					if (DeeThread_CheckInterrupt())                                           \
						goto err_label;                                                       \
					__hybrid_yield();                                                         \
				}                                                                             \
			} else {                                                                          \
				while (__hybrid_atomic_load(&(self)->ra_lock, __ATOMIC_ACQUIRE) != 0) {       \
					_now_microseconds = DeeThread_GetTimeMicroSeconds();                      \
					if (__hybrid_overflow_usub(_then_microseconds, _now_microseconds,         \
					                           &timeout_nanoseconds))                         \
						goto timeout_label; /* Timeout */                                     \
					timeout_nanoseconds *= 1000;                                              \
					if (DeeThread_CheckInterrupt())                                           \
						goto err;                                                             \
					__hybrid_yield();                                                         \
				}                                                                             \
			}                                                                                 \
		}                                                                                     \
	}	__WHILE0


/************************************************************************/
/* Dee_ratomic_rwlock_t                                                 */
/************************************************************************/
LOCAL NONNULL((1)) int DCALL
_Dee_ratomic_rwlock_read_p_impl(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
again:
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword != (uintptr_t)-1) {
		if (!__hybrid_atomic_cmpxch_weak(&self->rarw_lock.arw_lock,
		                                 lockword, lockword + 1,
		                                 __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			goto again;
		return 0;
	}
	if (__hybrid_gettid_iscaller(self->rarw_tid)) {
		++self->rarw_nwrite; /* read-after-write */
		return 0;
	}
	do {
		if (DeeThread_CheckInterrupt())
			goto err;
		__hybrid_yield();
	} while (!__hybrid_atomic_cmpxch(&self->rarw_lock.arw_lock, 0, 1,
	                                 __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE));
	return 0;
err:
	return -1;
}

LOCAL NONNULL((1)) int DCALL
_Dee_ratomic_rwlock_write_p_impl(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
again:
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0) {
again_lockword_zero:
		if (!__hybrid_atomic_cmpxch_weak(&self->rarw_lock.arw_lock, 0, (uintptr_t)-1,
		                                 __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			goto again;
		self->rarw_tid = __hybrid_gettid();
		return 0;
	}
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rarw_tid)) {
			++self->rarw_nwrite;
			return 0;
		}
	}
	while (__hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE) != 0) {
		if (DeeThread_CheckInterrupt())
			goto err;
		__hybrid_yield();
	}
	goto again_lockword_zero;
err:
	return -1;
}

LOCAL NONNULL((1)) int DCALL
_Dee_ratomic_rwlock_waitwrite_p_impl(Dee_ratomic_rwlock_t *__restrict self) {
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0)
		return 0;
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rarw_tid)) {
			++self->rarw_nwrite;
			return 0;
		}
	}
	while (__hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE) != 0) {
		if (DeeThread_CheckInterrupt())
			goto err;
		__hybrid_yield();
	}
	return 0;
err:
	return -1;
}


LOCAL NONNULL((1)) int DCALL
_Dee_ratomic_rwlock_read_timed_p_impl(Dee_ratomic_rwlock_t *__restrict self,
                                      uint64_t timeout_nanoseconds) {
	uint64_t now_microseconds, then_microseconds;
	uintptr_t lockword;
again:
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword != (uintptr_t)-1) {
		if (!__hybrid_atomic_cmpxch_weak(&self->rarw_lock.arw_lock,
		                                 lockword, lockword + 1,
		                                 __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			goto again;
		return 0;
	}
	if (__hybrid_gettid_iscaller(self->rarw_tid)) {
		++self->rarw_nwrite; /* read-after-write */
		return 0;
	}
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		goto do_infinite_timeout;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (__hybrid_overflow_uadd(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	if (__hybrid_atomic_cmpxch(&self->rarw_lock.arw_lock, 0, 1,
	                           __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
		return 0;
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (__hybrid_overflow_usub(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	if (DeeThread_CheckInterrupt())
		goto err;
	__hybrid_yield();
	goto do_wait_with_timeout;
err:
	return -1;
}

LOCAL NONNULL((1)) int DCALL
_Dee_ratomic_rwlock_write_timed_p_impl(Dee_ratomic_rwlock_t *__restrict self,
                                       uint64_t timeout_nanoseconds) {
	uint64_t now_microseconds, then_microseconds;
	uintptr_t lockword;
again:
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0) {
		if (!__hybrid_atomic_cmpxch_weak(&self->rarw_lock.arw_lock, 0, (uintptr_t)-1,
		                                 __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
			goto again;
settid:
		self->rarw_tid = __hybrid_gettid();
		return 0;
	}
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rarw_tid)) {
			++self->rarw_nwrite;
			return 0;
		}
	}
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		goto do_infinite_timeout;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (__hybrid_overflow_uadd(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	if (!__hybrid_atomic_cmpxch(&self->rarw_lock.arw_lock, 0, (uintptr_t)-1,
	                            __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
		goto settid;
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (__hybrid_overflow_usub(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	if (DeeThread_CheckInterrupt())
		goto err;
	__hybrid_yield();
	goto do_wait_with_timeout;
err:
	return -1;
}

LOCAL NONNULL((1)) int DCALL
_Dee_ratomic_rwlock_waitread_timed_p_impl(Dee_ratomic_rwlock_t *__restrict self,
                                          uint64_t timeout_nanoseconds) {
	uint64_t now_microseconds, then_microseconds;
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword != (uintptr_t)-1)
		return 0;
	if (__hybrid_gettid_iscaller(self->rarw_tid))
		return 0; /* read-after-write */
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		goto do_infinite_timeout;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (__hybrid_overflow_uadd(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	if (__hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE) != (uintptr_t)-1)
		return 0;
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (__hybrid_overflow_usub(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	if (DeeThread_CheckInterrupt())
		goto err;
	__hybrid_yield();
	goto do_wait_with_timeout;
err:
	return -1;
}

LOCAL NONNULL((1)) int DCALL
_Dee_ratomic_rwlock_waitwrite_timed_p_impl(Dee_ratomic_rwlock_t *__restrict self,
                                           uint64_t timeout_nanoseconds) {
	uint64_t now_microseconds, then_microseconds;
	uintptr_t lockword;
	lockword = __hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE);
	if (lockword == 0)
		return 0;
	if (lockword == (uintptr_t)-1) {
		if (__hybrid_gettid_iscaller(self->rarw_tid))
			return 0;
	}
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		goto do_infinite_timeout;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (__hybrid_overflow_uadd(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	if (__hybrid_atomic_load(&self->rarw_lock.arw_lock, __ATOMIC_ACQUIRE) == 0)
		return 0;
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (__hybrid_overflow_usub(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	if (DeeThread_CheckInterrupt())
		goto err;
	__hybrid_yield();
	goto do_wait_with_timeout;
err:
	return -1;
}


#define Dee_ratomic_rwlock_read_p(self, err_label)         \
	do {                                                   \
		if unlikely(_Dee_ratomic_rwlock_read_p_impl(self)) \
			goto err_label;                                \
	}	__WHILE0
#define Dee_ratomic_rwlock_write_p(self, err_label)         \
	do {                                                    \
		if unlikely(_Dee_ratomic_rwlock_write_p_impl(self)) \
			goto err_label;                                 \
	}	__WHILE0
#define Dee_ratomic_rwlock_waitread_p(self, err_label)                                                         \
	do {                                                                                                       \
		if (__hybrid_atomic_load(&(self)->rarw_lock.arw_lock, __ATOMIC_ACQUIRE) == (uintptr_t)-1) {            \
			if (!__hybrid_gettid_iscaller((self)->rarw_tid)) { /* Read-after-write */                          \
				while (__hybrid_atomic_load(&(self)->rarw_lock.arw_lock, __ATOMIC_ACQUIRE) == (uintptr_t)-1) { \
					if (DeeThread_CheckInterrupt())                                                            \
						goto err_label;                                                                        \
					__hybrid_yield();                                                                          \
				}                                                                                              \
			}                                                                                                  \
		}                                                                                                      \
	}	__WHILE0
#define Dee_ratomic_rwlock_waitwrite_p(self, err_label)         \
	do {                                                        \
		if unlikely(_Dee_ratomic_rwlock_waitwrite_p_impl(self)) \
			goto err_label;                                     \
	}	__WHILE0
#define Dee_ratomic_rwlock_read_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	do {                                                                                     \
		int _status = _Dee_ratomic_rwlock_read_timed_p_impl(self, timeout_nanoseconds);      \
		if unlikely(_status != 0) {                                                          \
			if unlikely(_status < 0)                                                         \
				goto err_label;                                                              \
			goto timeout_label;                                                              \
		}                                                                                    \
	}	__WHILE0
#define Dee_ratomic_rwlock_write_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	do {                                                                                      \
		int _status = _Dee_ratomic_rwlock_write_timed_p_impl(self, timeout_nanoseconds);      \
		if unlikely(_status != 0) {                                                           \
			if unlikely(_status < 0)                                                          \
				goto err_label;                                                               \
			goto timeout_label;                                                               \
		}                                                                                     \
	}	__WHILE0
#define Dee_ratomic_rwlock_waitread_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	do {                                                                                         \
		int _status = _Dee_ratomic_rwlock_waitread_timed_p_impl(self, timeout_nanoseconds);      \
		if unlikely(_status != 0) {                                                              \
			if unlikely(_status < 0)                                                             \
				goto err_label;                                                                  \
			goto timeout_label;                                                                  \
		}                                                                                        \
	}	__WHILE0
#define Dee_ratomic_rwlock_waitwrite_timed_p(self, timeout_nanoseconds, err_label, timeout_label) \
	do {                                                                                          \
		int _status = _Dee_ratomic_rwlock_waitwrite_timed_p_impl(self, timeout_nanoseconds);      \
		if unlikely(_status != 0) {                                                               \
			if unlikely(_status < 0)                                                              \
				goto err_label;                                                                   \
			goto timeout_label;                                                                   \
		}                                                                                         \
	}	__WHILE0



DECL_END
#endif /* !CONFIG_NO_THREADS */

#endif /* !GUARD_DEEMON_UTIL_RLOCK_UTILS_H */
