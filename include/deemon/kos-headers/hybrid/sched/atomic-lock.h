/* Copyright (c) 2019-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2019-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef __GUARD_HYBRID_SCHED_ATOMIC_LOCK_H
#define __GUARD_HYBRID_SCHED_ATOMIC_LOCK_H 1

#include "../../__stdinc.h"
#include "__atomic-lock.h"

#define __SIZEOF_ATOMIC_LOCK __SIZEOF_HYBRID_ATOMIC_LOCK

#if defined(__CC__) || defined(__DEEMON__)
#define atomic_lock                 __hybrid_atomic_lock
#define ATOMIC_LOCK_INIT            __HYBRID_ATOMIC_LOCK_INIT
#define ATOMIC_LOCK_INIT_ACQUIRED   __HYBRID_ATOMIC_LOCK_INIT_ACQUIRED
#define atomic_lock_init            __hybrid_atomic_lock_init
#define atomic_lock_init_acquired   __hybrid_atomic_lock_init_acquired
#define atomic_lock_cinit           __hybrid_atomic_lock_cinit
#define atomic_lock_cinit_acquired  __hybrid_atomic_lock_cinit_acquired
#define atomic_lock_acquired        __hybrid_atomic_lock_acquired
#define atomic_lock_available       __hybrid_atomic_lock_available
#define atomic_lock_tryacquire      __hybrid_atomic_lock_tryacquire
#define atomic_lock_release         __hybrid_atomic_lock_release
#define _atomic_lock_release_NDEBUG ___hybrid_atomic_lock_release_NDEBUG
#define atomic_lock_acquire_nopr    __hybrid_atomic_lock_acquire_nopr
#define atomic_lock_waitfor_nopr    __hybrid_atomic_lock_waitfor_nopr
#define atomic_lock_acquire         __hybrid_atomic_lock_acquire
#define atomic_lock_waitfor         __hybrid_atomic_lock_waitfor
#define atomic_lock_release_nopr    __hybrid_atomic_lock_release_nopr
#define atomic_lock_acquire_smp_r   __hybrid_atomic_lock_acquire_smp_r
#define atomic_lock_release_smp_r   __hybrid_atomic_lock_release_smp_r
#define atomic_lock_acquire_smp_b   __hybrid_atomic_lock_acquire_smp_b
#define atomic_lock_release_smp_b   __hybrid_atomic_lock_release_smp_b
#define atomic_lock_acquire_smp     __hybrid_atomic_lock_acquire_smp
#define atomic_lock_release_smp     __hybrid_atomic_lock_release_smp
#if defined(__KERNEL__) && defined(__KOS_VERSION__) && __KOS_VERSION__ >= 400
#define atomic_lock_acquire_nx __hybrid_atomic_lock_acquire_nx
#define atomic_lock_waitfor_nx __hybrid_atomic_lock_waitfor_nx
#endif /* __KERNEL__ && __KOS_VERSION__ >= 400 */
#endif /* __CC__ */

#endif /* !__GUARD_HYBRID_SCHED_ATOMIC_LOCK_H */
