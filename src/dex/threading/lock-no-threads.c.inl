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
#ifndef GUARD_DEEMON_RUNTIME_LOCK_NO_THREADS_C_INL
#define GUARD_DEEMON_RUNTIME_LOCK_NO_THREADS_C_INL

#include "libthreading.h"
/**/

#include <deemon/api.h>

#include <deemon/error.h>
#include <deemon/thread.h>
#include <deemon/util/lock.h>  /* Dee_ATOMIC_LOCK_INIT, Dee_ATOMIC_LOCK_INIT_ACQUIRED, Dee_ATOMIC_RWLOCK_*, Dee_EVENT_INIT, Dee_EVENT_INIT_SET, Dee_SEMAPHORE_INIT, Dee_SHARED_LOCK_INIT, Dee_SHARED_LOCK_INIT_ACQUIRED, Dee_SHARED_RWLOCK_*, Dee_atomic_lock_*, Dee_atomic_rwlock_*, Dee_event_*, Dee_semaphore_*, Dee_shared_lock_*, Dee_shared_rwlock_*, _Dee_atomic_lock_release_NDEBUG, _Dee_atomic_rwlock_*, _Dee_shared_rwlock_* */
#include <deemon/util/rlock.h> /* Dee_RATOMIC_LOCK_INIT, Dee_RATOMIC_RWLOCK_INIT, Dee_RSHARED_LOCK_INIT, Dee_RSHARED_RWLOCK_INIT, Dee_ratomic_lock_*, Dee_ratomic_rwlock_*, Dee_rshared_lock_*, Dee_rshared_rwlock_*, _Dee_ratomic_lock_release_NDEBUG, _Dee_ratomic_lock_release_ex_NDEBUG, _Dee_ratomic_rwlock_*, _Dee_rshared_lock_release_NDEBUG, _Dee_rshared_lock_release_ex_NDEBUG, _Dee_rshared_rwlock_* */

#include <hybrid/typecore.h> /* __CHAR_BIT__ */

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint8_t, uint64_t, uintptr_t */

DECL_BEGIN

#ifndef CHAR_BIT
#define CHAR_BIT __CHAR_BIT__
#endif /* !CHAR_BIT */

/* Delete definitions */
#undef Dee_atomic_lock_t
#undef Dee_ATOMIC_LOCK_INIT
#undef Dee_ATOMIC_LOCK_INIT_ACQUIRED
#undef Dee_atomic_lock_init
#undef Dee_atomic_lock_init_acquired
#undef Dee_atomic_lock_cinit
#undef Dee_atomic_lock_cinit_acquired
#undef Dee_atomic_lock_available
#undef Dee_atomic_lock_acquired
#undef Dee_atomic_lock_tryacquire
#undef Dee_atomic_lock_acquire
#undef Dee_atomic_lock_waitfor
#undef Dee_atomic_lock_release
#undef _Dee_atomic_lock_release_NDEBUG

#undef Dee_atomic_rwlock_t
#undef Dee_ATOMIC_RWLOCK_MAX_READERS
#undef Dee_ATOMIC_RWLOCK_INIT
#undef Dee_ATOMIC_RWLOCK_INIT_READ
#undef Dee_ATOMIC_RWLOCK_INIT_WRITE
#undef Dee_atomic_rwlock_cinit
#undef Dee_atomic_rwlock_cinit_read
#undef Dee_atomic_rwlock_cinit_write
#undef Dee_atomic_rwlock_init
#undef Dee_atomic_rwlock_init_read
#undef Dee_atomic_rwlock_init_write
#undef Dee_atomic_rwlock_reading
#undef Dee_atomic_rwlock_writing
#undef Dee_atomic_rwlock_tryread
#undef Dee_atomic_rwlock_trywrite
#undef Dee_atomic_rwlock_canread
#undef Dee_atomic_rwlock_canwrite
#undef Dee_atomic_rwlock_canendread
#undef Dee_atomic_rwlock_canendwrite
#undef Dee_atomic_rwlock_canend
#undef Dee_atomic_rwlock_waitread
#undef Dee_atomic_rwlock_waitwrite
#undef Dee_atomic_rwlock_read
#undef Dee_atomic_rwlock_write
#undef Dee_atomic_rwlock_tryupgrade
#undef Dee_atomic_rwlock_upgrade
#undef Dee_atomic_rwlock_downgrade
#undef _Dee_atomic_rwlock_downgrade_NDEBUG
#undef Dee_atomic_rwlock_endwrite
#undef Dee_atomic_rwlock_endread
#undef Dee_atomic_rwlock_end
#undef Dee_atomic_rwlock_endread_ex
#undef Dee_atomic_rwlock_end_ex
#undef _Dee_atomic_rwlock_endwrite_NDEBUG
#undef _Dee_atomic_rwlock_endread_NDEBUG
#undef _Dee_atomic_rwlock_end_NDEBUG
#undef _Dee_atomic_rwlock_endread_ex_NDEBUG
#undef _Dee_atomic_rwlock_end_ex_NDEBUG

#undef Dee_shared_lock_t
#undef Dee_SHARED_LOCK_INIT
#undef Dee_SHARED_LOCK_INIT_ACQUIRED
#undef Dee_shared_lock_cinit
#undef Dee_shared_lock_init
#undef Dee_shared_lock_cinit_acquired
#undef Dee_shared_lock_init_acquired
#undef Dee_shared_lock_available
#undef Dee_shared_lock_acquired
#undef Dee_shared_lock_tryacquire
#undef Dee_shared_lock_release
#undef _Dee_shared_lock_release_NDEBUG
#undef Dee_shared_lock_acquire
#undef Dee_shared_lock_acquire_noint
#undef Dee_shared_lock_waitfor
#undef Dee_shared_lock_waitfor_noint
#undef Dee_shared_lock_acquire_timed
#undef Dee_shared_lock_acquire_noint_timed
#undef Dee_shared_lock_waitfor_timed
#undef Dee_shared_lock_waitfor_noint_timed

#undef Dee_shared_rwlock_t
#undef Dee_SHARED_RWLOCK_MAX_READERS
#undef Dee_SHARED_RWLOCK_INIT
#undef Dee_SHARED_RWLOCK_INIT_READ
#undef Dee_SHARED_RWLOCK_INIT_WRITE
#undef Dee_shared_rwlock_init
#undef Dee_shared_rwlock_init_read
#undef Dee_shared_rwlock_init_write
#undef Dee_shared_rwlock_cinit
#undef Dee_shared_rwlock_cinit_read
#undef Dee_shared_rwlock_cinit_write
#undef Dee_shared_rwlock_reading
#undef Dee_shared_rwlock_writing
#undef Dee_shared_rwlock_canread
#undef Dee_shared_rwlock_canwrite
#undef Dee_shared_rwlock_canendread
#undef Dee_shared_rwlock_canendwrite
#undef Dee_shared_rwlock_canend
#undef Dee_shared_rwlock_tryupgrade
#undef Dee_shared_rwlock_trywrite
#undef Dee_shared_rwlock_tryread
#undef _Dee_shared_rwlock_downgrade_NDEBUG
#undef Dee_shared_rwlock_downgrade
#undef Dee_shared_rwlock_upgrade
#undef Dee_shared_rwlock_upgrade_noint
#undef _Dee_shared_rwlock_upgrade_NDEBUG
#undef _Dee_shared_rwlock_upgrade_noint_NDEBUG
#undef Dee_shared_rwlock_endread
#undef Dee_shared_rwlock_endwrite
#undef Dee_shared_rwlock_end
#undef _Dee_shared_rwlock_endread_NDEBUG
#undef _Dee_shared_rwlock_endwrite_NDEBUG
#undef _Dee_shared_rwlock_end_NDEBUG
#undef Dee_shared_rwlock_endread_ex
#undef Dee_shared_rwlock_end_ex
#undef _Dee_shared_rwlock_endread_ex_NDEBUG
#undef _Dee_shared_rwlock_end_ex_NDEBUG
#undef Dee_shared_rwlock_read
#undef Dee_shared_rwlock_read_noint
#undef Dee_shared_rwlock_write
#undef Dee_shared_rwlock_write_noint
#undef Dee_shared_rwlock_waitread
#undef Dee_shared_rwlock_waitread_noint
#undef Dee_shared_rwlock_waitwrite
#undef Dee_shared_rwlock_waitwrite_noint
#undef Dee_shared_rwlock_read_timed
#undef Dee_shared_rwlock_read_noint_timed
#undef Dee_shared_rwlock_write_timed
#undef Dee_shared_rwlock_write_noint_timed
#undef Dee_shared_rwlock_waitread_timed
#undef Dee_shared_rwlock_waitread_noint_timed
#undef Dee_shared_rwlock_waitwrite_timed
#undef Dee_shared_rwlock_waitwrite_noint_timed

#undef Dee_semaphore_t
#undef Dee_SEMAPHORE_INIT
#undef Dee_semaphore_init
#undef Dee_semaphore_cinit
#undef Dee_semaphore_haswaiting
#undef Dee_semaphore_hastickets
#undef Dee_semaphore_gettickets
#undef Dee_semaphore_release
#undef Dee_semaphore_tryacquire
#undef Dee_semaphore_waitfor
#undef Dee_semaphore_waitfor_timed
#undef Dee_semaphore_acquire
#undef Dee_semaphore_acquire_timed
#undef Dee_semaphore_waitfor_noint
#undef Dee_semaphore_waitfor_noint_timed
#undef Dee_semaphore_acquire_noint
#undef Dee_semaphore_acquire_noint_timed

#undef Dee_event_t
#undef Dee_EVENT_INIT_SET
#undef Dee_EVENT_INIT
#undef Dee_event_init_set
#undef Dee_event_init
#undef Dee_event_cinit_set
#undef Dee_event_cinit
#undef Dee_event_get
#undef Dee_event_set
#undef Dee_event_clear
#undef Dee_event_set_ex
#undef Dee_event_clear_ex
#undef Dee_event_waitfor
#undef Dee_event_waitfor_timed
#undef Dee_event_waitfor_noint
#undef Dee_event_waitfor_noint_timed

#undef Dee_ratomic_lock_t
#undef Dee_RATOMIC_LOCK_INIT
#undef Dee_ratomic_lock_init
#undef Dee_ratomic_lock_cinit
#undef Dee_ratomic_lock_available
#undef Dee_ratomic_lock_acquired
#undef Dee_ratomic_lock_tryacquire
#undef Dee_ratomic_lock_acquire
#undef Dee_ratomic_lock_waitfor
#undef _Dee_ratomic_lock_release_NDEBUG
#undef _Dee_ratomic_lock_release_ex_NDEBUG
#undef Dee_ratomic_lock_release
#undef Dee_ratomic_lock_release_ex

#undef Dee_rshared_lock_t
#undef Dee_RSHARED_LOCK_INIT
#undef Dee_rshared_lock_init
#undef Dee_rshared_lock_cinit
#undef Dee_rshared_lock_available
#undef Dee_rshared_lock_acquired
#undef Dee_rshared_lock_tryacquire
#undef Dee_rshared_lock_acquire
#undef Dee_rshared_lock_acquire_noint
#undef Dee_rshared_lock_waitfor
#undef Dee_rshared_lock_waitfor_noint
#undef Dee_rshared_lock_acquire_timed
#undef Dee_rshared_lock_acquire_noint_timed
#undef Dee_rshared_lock_waitfor_timed
#undef Dee_rshared_lock_waitfor_noint_timed
#undef _Dee_rshared_lock_release_NDEBUG
#undef _Dee_rshared_lock_release_ex_NDEBUG
#undef Dee_rshared_lock_release
#undef Dee_rshared_lock_release_ex

#undef Dee_ratomic_rwlock_t
#undef Dee_RATOMIC_RWLOCK_INIT
#undef Dee_ratomic_rwlock_init
#undef Dee_ratomic_rwlock_cinit
#undef Dee_ratomic_rwlock_tryread
#undef Dee_ratomic_rwlock_waitread
#undef Dee_ratomic_rwlock_read
#undef Dee_ratomic_rwlock_endread
#undef _Dee_ratomic_rwlock_endread_NDEBUG
#undef Dee_ratomic_rwlock_endread_ex
#undef _Dee_ratomic_rwlock_endread_ex_NDEBUG
#undef Dee_ratomic_rwlock_trywrite
#undef Dee_ratomic_rwlock_reading
#undef Dee_ratomic_rwlock_writing
#undef Dee_ratomic_rwlock_canread
#undef Dee_ratomic_rwlock_canwrite
#undef Dee_ratomic_rwlock_canendread
#undef Dee_ratomic_rwlock_canend
#undef Dee_ratomic_rwlock_canendwrite
#undef Dee_ratomic_rwlock_write
#undef Dee_ratomic_rwlock_waitwrite
#undef Dee_ratomic_rwlock_tryupgrade
#undef Dee_ratomic_rwlock_upgrade
#undef Dee_ratomic_rwlock_downgrade
#undef _Dee_ratomic_rwlock_downgrade_NDEBUG
#undef _Dee_ratomic_rwlock_endwrite_NDEBUG
#undef _Dee_ratomic_rwlock_endwrite_ex_NDEBUG
#undef Dee_ratomic_rwlock_endwrite
#undef Dee_ratomic_rwlock_endwrite_ex
#undef Dee_ratomic_rwlock_end
#undef Dee_ratomic_rwlock_end_ex
#undef _Dee_ratomic_rwlock_end_NDEBUG
#undef _Dee_ratomic_rwlock_end_ex_NDEBUG

#undef Dee_rshared_rwlock_t
#undef Dee_RSHARED_RWLOCK_INIT
#undef Dee_rshared_rwlock_init
#undef Dee_rshared_rwlock_cinit
#undef Dee_rshared_rwlock_reading
#undef Dee_rshared_rwlock_writing
#undef Dee_rshared_rwlock_tryread
#undef Dee_rshared_rwlock_trywrite
#undef Dee_rshared_rwlock_canread
#undef Dee_rshared_rwlock_canwrite
#undef Dee_rshared_rwlock_canendread
#undef Dee_rshared_rwlock_canendwrite
#undef Dee_rshared_rwlock_canend
#undef Dee_rshared_rwlock_tryupgrade
#undef Dee_rshared_rwlock_downgrade
#undef _Dee_rshared_rwlock_downgrade_NDEBUG
#undef Dee_rshared_rwlock_endwrite_ex
#undef Dee_rshared_rwlock_endwrite
#undef _Dee_rshared_rwlock_endwrite_ex_NDEBUG
#undef _Dee_rshared_rwlock_endwrite_NDEBUG
#undef Dee_rshared_rwlock_endread
#undef Dee_rshared_rwlock_endread_ex
#undef _Dee_rshared_rwlock_endread_NDEBUG
#undef _Dee_rshared_rwlock_endread_ex_NDEBUG
#undef Dee_rshared_rwlock_end
#undef Dee_rshared_rwlock_end_ex
#undef _Dee_rshared_rwlock_end_NDEBUG
#undef _Dee_rshared_rwlock_end_ex_NDEBUG
#undef Dee_rshared_rwlock_read
#undef Dee_rshared_rwlock_read_noint
#undef Dee_rshared_rwlock_write
#undef Dee_rshared_rwlock_write_noint
#undef Dee_rshared_rwlock_waitread
#undef Dee_rshared_rwlock_waitread_noint
#undef Dee_rshared_rwlock_waitwrite
#undef Dee_rshared_rwlock_waitwrite_noint
#undef Dee_rshared_rwlock_read_timed
#undef Dee_rshared_rwlock_read_noint_timed
#undef Dee_rshared_rwlock_write_timed
#undef Dee_rshared_rwlock_write_noint_timed
#undef Dee_rshared_rwlock_waitread_timed
#undef Dee_rshared_rwlock_waitread_noint_timed
#undef Dee_rshared_rwlock_waitwrite_timed
#undef Dee_rshared_rwlock_waitwrite_noint_timed
#undef Dee_rshared_rwlock_upgrade
#undef Dee_rshared_rwlock_upgrade_noint


/************************************************************************/
/* Emulate semantics of lock objects in a single-threaded environment   */
/************************************************************************/

#define _DeeError_ThrowWouldBlock() \
	DeeError_Throwf(&DeeError_ValueError, "Operation would block, and threading is disabled")
#if 1 /* Actually sleep so-long as timeout isn't infinite? */
#define _DeeLock_SleepFor(timeout_nanoseconds) \
	((timeout_nanoseconds) == 0                \
	 ? 1                                       \
	 : (timeout_nanoseconds) == (uint64_t)-1   \
	   ? _DeeError_ThrowWouldBlock()           \
	   : (DeeThread_Sleep((timeout_nanoseconds) / 1000) ? -1 : 1))
#else
#define _DeeLock_SleepFor(timeout_nanoseconds) \
	((timeout_nanoseconds) == 0 ? 1 : _DeeError_ThrowWouldBlock())
#endif


/************************************************************************/
/* <deemon/util/lock.h>                                                 */
/************************************************************************/
#define Dee_shared_lock_t                                        uint8_t
#define Dee_shared_lock_cinit(self)                              Dee_ASSERT(*(self) == 0)
#define Dee_shared_lock_init(self)                               (void)(*(self) = 0)
#define Dee_shared_lock_cinit_acquired(self)                     (void)(*(self) = 1)
#define Dee_shared_lock_init_acquired(self)                      (void)(*(self) = 1)
#define Dee_shared_lock_available(self)                          (*(self) == 0)
#define Dee_shared_lock_acquired(self)                           (*(self) != 0)
#define Dee_shared_lock_tryacquire(self)                         (*(self) ? 0 : (*(self) = 1, 1))
#define _Dee_shared_lock_release_NDEBUG(self)                    (void)(*(self) = 0)
#define Dee_shared_lock_acquire(self)                            (Dee_shared_lock_tryacquire(self) ? 0 : _DeeError_ThrowWouldBlock())
#define Dee_shared_lock_waitfor(self)                            (Dee_shared_lock_available(self) ? 0 : _DeeError_ThrowWouldBlock())
#define Dee_shared_lock_acquire_timed(self, timeout_nanoseconds) (Dee_shared_lock_tryacquire(self) ? 0 : _DeeLock_SleepFor(timeout_nanoseconds))
#define Dee_shared_lock_waitfor_timed(self, timeout_nanoseconds) (Dee_shared_lock_available(self) ? 0 : _DeeLock_SleepFor(timeout_nanoseconds))
/* Aliases */
#define Dee_atomic_lock_cinit                                    Dee_shared_lock_cinit
#define Dee_atomic_lock_init                                     Dee_shared_lock_init
#define Dee_atomic_lock_cinit_acquired                           Dee_shared_lock_cinit_acquired
#define Dee_atomic_lock_init_acquired                            Dee_shared_lock_init_acquired

#define Dee_shared_rwlock_t                                          uintptr_t
#define Dee_SHARED_RWLOCK_MAX_READERS                                ((uintptr_t)-2)
#define _DEE_SHARED_RWLOCK_WRITING                                   ((uintptr_t)-1)
#define Dee_shared_rwlock_init(self)                                 (void)(*(self) = 0)
#define Dee_shared_rwlock_init_read(self, n)                         (void)(*(self) = (n))
#define Dee_shared_rwlock_init_write(self)                           (void)(*(self) = _DEE_SHARED_RWLOCK_WRITING)
#define Dee_shared_rwlock_cinit(self)                                (void)Dee_ASSERT(*(self) == 0)
#define Dee_shared_rwlock_cinit_read(self)                           (void)(*(self) = (n))
#define Dee_shared_rwlock_cinit_write(self)                          (void)(*(self) = _DEE_SHARED_RWLOCK_WRITING)
#define Dee_shared_rwlock_reading(self)                              (*(self) != 0)
#define Dee_shared_rwlock_writing(self)                              (*(self) == _DEE_SHARED_RWLOCK_WRITING)
#define Dee_shared_rwlock_canread(self)                              (*(self) != _DEE_SHARED_RWLOCK_WRITING)
#define Dee_shared_rwlock_canwrite(self)                             (*(self) == 0)
#define Dee_shared_rwlock_canendread(self)                           (*(self) != 0 && *(self) != _DEE_SHARED_RWLOCK_WRITING)
#define Dee_shared_rwlock_canendwrite(self)                          (*(self) == _DEE_SHARED_RWLOCK_WRITING)
#define Dee_shared_rwlock_canend(self)                               (*(self) != 0)
#define Dee_shared_rwlock_tryupgrade(self)                           (*(self) == 1 ? (*(self) = _DEE_SHARED_RWLOCK_WRITING, 1) : 0)
#define Dee_shared_rwlock_trywrite(self)                             (*(self) == 0 ? (*(self) = _DEE_SHARED_RWLOCK_WRITING, 1) : 0)
#define Dee_shared_rwlock_tryread(self)                              (*(self) != _DEE_SHARED_RWLOCK_WRITING ? (++*(self), 1) : 0)
#define _Dee_shared_rwlock_downgrade_NDEBUG(self)                    (void)(*(self) = 1)
#define _Dee_shared_rwlock_endread_NDEBUG(self)                      (void)(--*(self))
#define _Dee_shared_rwlock_endwrite_NDEBUG(self)                     (void)(*(self) = 0)
#define _Dee_shared_rwlock_end_NDEBUG(self)                          (*(self) == _DEE_SHARED_RWLOCK_WRITING ? (void)(*(self) = 0) : (void)(--*(self)))
#define _Dee_shared_rwlock_endread_ex_NDEBUG(self)                   (_Dee_shared_rwlock_endread_NDEBUG(self), 1)
#define _Dee_shared_rwlock_end_ex_NDEBUG(self)                       (_Dee_shared_rwlock_end_NDEBUG(self), 1)
#define Dee_shared_rwlock_read(self)                                 (Dee_shared_rwlock_tryread(self) ? 0 : _DeeError_ThrowWouldBlock())
#define Dee_shared_rwlock_write(self)                                (Dee_shared_rwlock_trywrite(self) ? 0 : _DeeError_ThrowWouldBlock())
#define Dee_shared_rwlock_waitread(self)                             (Dee_shared_rwlock_canread(self) ? 0 : _DeeError_ThrowWouldBlock())
#define Dee_shared_rwlock_waitwrite(self)                            (Dee_shared_rwlock_canwrite(self) ? 0 : _DeeError_ThrowWouldBlock())
#define Dee_shared_rwlock_read_timed(self, timeout_nanoseconds)      (Dee_shared_rwlock_tryread(self) ? 0 : _DeeLock_SleepFor(timeout_nanoseconds))
#define Dee_shared_rwlock_write_timed(self, timeout_nanoseconds)     (Dee_shared_rwlock_trywrite(self) ? 0 : _DeeLock_SleepFor(timeout_nanoseconds))
#define Dee_shared_rwlock_waitread_timed(self, timeout_nanoseconds)  (Dee_shared_rwlock_canread(self) ? 0 : _DeeLock_SleepFor(timeout_nanoseconds))
#define Dee_shared_rwlock_waitwrite_timed(self, timeout_nanoseconds) (Dee_shared_rwlock_canwrite(self) ? 0 : _DeeLock_SleepFor(timeout_nanoseconds))
/* Aliases */
#define Dee_ATOMIC_RWLOCK_MAX_READERS                                Dee_SHARED_RWLOCK_MAX_READERS
#define Dee_atomic_rwlock_init                                       Dee_shared_rwlock_init
#define Dee_atomic_rwlock_init_read                                  Dee_shared_rwlock_init_read
#define Dee_atomic_rwlock_init_write                                 Dee_shared_rwlock_init_write
#define Dee_atomic_rwlock_cinit                                      Dee_shared_rwlock_cinit
#define Dee_atomic_rwlock_cinit_read                                 Dee_shared_rwlock_cinit_read
#define Dee_atomic_rwlock_cinit_write                                Dee_shared_rwlock_cinit_write




#undef Dee_semaphore_t
#define Dee_semaphore_t _nosmp_Dee_semaphore_t
typedef struct {
	size_t se_tickets; /* # of tickets currently available (atomic + futex word) */
} Dee_semaphore_t;
#define Dee_semaphore_init(self, n_tickets)                    (void)((self)->se_tickets = (n_tickets))
#define Dee_semaphore_haswaiting(self)                         ((void)(self), 0)
#define Dee_semaphore_hastickets(self)                         ((self)->se_tickets != 0)
#define Dee_semaphore_gettickets(self)                         ((self)->se_tickets)
#define Dee_semaphore_release(self, count)                     (void)++(self)->se_tickets
#define Dee_semaphore_tryacquire(self)                         ((self)->se_tickets ? (--(self)->se_tickets, 1) : 0)
#define Dee_semaphore_waitfor(self)                            (Dee_semaphore_hastickets(self) ? 0 : _DeeError_ThrowWouldBlock())
#define Dee_semaphore_waitfor_timed(self, timeout_nanoseconds) (Dee_semaphore_hastickets(self) ? 0 : _DeeLock_SleepFor(timeout_nanoseconds))
#define Dee_semaphore_acquire(self)                            (Dee_semaphore_tryacquire(self) ? 0 : _DeeError_ThrowWouldBlock())
#define Dee_semaphore_acquire_timed(self, timeout_nanoseconds) (Dee_semaphore_tryacquire(self) ? 0 : _DeeLock_SleepFor(timeout_nanoseconds))

#define Dee_event_t                                        uint8_t
#define Dee_event_init_set(self)                           (void)(*(self) = 1)
#define Dee_event_init(self)                               (void)(*(self) = 0)
#define Dee_event_get(self)                                (*(self))
#define Dee_event_set(self)                                (void)(*(self) = 1)
#define Dee_event_clear(self)                              (void)(*(self) = 0)
#define Dee_event_set_ex(self)                             (*(self) = 1, 1)
#define Dee_event_clear_ex(self)                           (*(self) = 0, 1)
#define Dee_event_waitfor(self)                            (Dee_event_get(self) ? 0 : _DeeError_ThrowWouldBlock())
#define Dee_event_waitfor_timed(self, timeout_nanoseconds) (Dee_event_get(self) ? 0 : _DeeLock_SleepFor(timeout_nanoseconds))



/************************************************************************/
/* <deemon/util/rlock.h>                                                */
/************************************************************************/

#define Dee_rshared_lock_t                                        uintptr_t
#define Dee_rshared_lock_init(self)                               (void)(*(self) = 0)
#define Dee_rshared_lock_cinit(self)                              Dee_ASSERT(*(self) == 0)
#define Dee_rshared_lock_available(self)                          ((void)(self), 1)
#define Dee_rshared_lock_acquired(self)                           (*(self) != 0)
#define Dee_rshared_lock_tryacquire(self)                         (++*(self), 1)
#define Dee_rshared_lock_acquire(self)                            (++*(self), 0)
#define Dee_rshared_lock_waitfor(self)                            ((void)(self), 0)
#define Dee_rshared_lock_acquire_timed(self, timeout_nanoseconds) (++*(self), (void)(timeout_nanoseconds), 0)
#define Dee_rshared_lock_waitfor_timed(self, timeout_nanoseconds) ((void)(self), (void)(timeout_nanoseconds), 0)
#define _Dee_rshared_lock_release_NDEBUG(self)                    (--*(self))

#define Dee_rshared_rwlock_t                                          uintptr_t
#define _DEE_RSHARED_RWLOCK_WMASK                                     ((uintptr_t)1 << ((sizeof(uintptr_t) * CHAR_BIT) - 1))
#define Dee_rshared_rwlock_init(self)                                 (void)(*(self) = 0)
#define Dee_rshared_rwlock_cinit(self)                                (void)Dee_ASSERT(*(self) == 0)
#define Dee_rshared_rwlock_reading(self)                              (*(self) != 0)
#define Dee_rshared_rwlock_writing(self)                              (*(self)&_DEE_RSHARED_RWLOCK_WMASK)
#define Dee_rshared_rwlock_tryread(self)                              (++*(self), 1)
#define Dee_rshared_rwlock_trywrite(self)                             (Dee_rshared_rwlock_writing(self) ? (++*(self), 1) : (!Dee_rshared_rwlock_reading(self) ? (*(self) = _DEE_RSHARED_RWLOCK_WMASK, 1) : 0))
#define Dee_rshared_rwlock_canread(self)                              ((void)(self), 1)
#define Dee_rshared_rwlock_canwrite(self)                             (Dee_rshared_rwlock_writing(self) || !Dee_rshared_rwlock_reading(self))
#define Dee_rshared_rwlock_canendread(self)                           (Dee_rshared_rwlock_reading(self) && *(self) != _DEE_RSHARED_RWLOCK_WMASK)
#define Dee_rshared_rwlock_canendwrite(self)                          Dee_rshared_rwlock_writing(self)
#define Dee_rshared_rwlock_canend(self)                               Dee_rshared_rwlock_reading(self)
#define Dee_rshared_rwlock_tryupgrade(self)                           (*(self) == 1 ? (*(self) = _DEE_RSHARED_RWLOCK_WMASK, 1) : 0)
#define _Dee_rshared_rwlock_downgrade_NDEBUG(self)                    (void)(*(self) = 1)
#define _Dee_rshared_rwlock_endwrite_NDEBUG(self)                     (*(self) == _DEE_RSHARED_RWLOCK_WMASK ? (void)(*(self) = 0) : (void)(--*(self)))
#define _Dee_rshared_rwlock_endread_NDEBUG(self)                      (*(self) == _DEE_RSHARED_RWLOCK_WMASK ? (void)(*(self) = 0) : (void)(--*(self)))
#define _Dee_rshared_rwlock_end_NDEBUG(self)                          (*(self) == _DEE_RSHARED_RWLOCK_WMASK ? (void)(*(self) = 0) : (void)(--*(self)))
#define _Dee_rshared_rwlock_endread_ex_NDEBUG(self)                   (_Dee_rshared_rwlock_endread_NDEBUG(self), 1)
#define _Dee_rshared_rwlock_end_ex_NDEBUG(self)                       (_Dee_rshared_rwlock_end_NDEBUG(self), 1)
#define Dee_rshared_rwlock_read(self)                                 (++*(self), 0)
#define Dee_rshared_rwlock_write(self)                                (Dee_rshared_rwlock_trywrite(self) ? 0 : _DeeError_ThrowWouldBlock())
#define Dee_rshared_rwlock_waitread(self)                             ((void)(self), 0)
#define Dee_rshared_rwlock_waitwrite(self)                            (Dee_rshared_rwlock_canwrite(self) ? 0 : _DeeError_ThrowWouldBlock())
#define Dee_rshared_rwlock_read_timed(self, timeout_nanoseconds)      ((void)(timeout_nanoseconds), Dee_rshared_rwlock_read(self))
#define Dee_rshared_rwlock_write_timed(self, timeout_nanoseconds)     (Dee_rshared_rwlock_trywrite(self) ? 0 : _DeeLock_SleepFor(timeout_nanoseconds))
#define Dee_rshared_rwlock_waitread_timed(self, timeout_nanoseconds)  ((void)(timeout_nanoseconds), Dee_rshared_rwlock_waitread(self))
#define Dee_rshared_rwlock_waitwrite_timed(self, timeout_nanoseconds) (Dee_rshared_rwlock_canwrite(self) ? 0 : _DeeLock_SleepFor(timeout_nanoseconds))

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_LOCK_NO_THREADS_C_INL */
