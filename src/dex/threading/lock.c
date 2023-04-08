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
#ifndef GUARD_DEX_THREADING_LOCK_C
#define GUARD_DEX_THREADING_LOCK_C 1
#define CONFIG_BUILDING_LIBTHREADING
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>
#include <deemon/util/rlock.h>

#include <hybrid/overflow.h>
#include <hybrid/sched/yield.h>

#include <stdbool.h>
#include <stddef.h>

#include "libthreading.h"

DECL_BEGIN

#if __SIZEOF_POINTER__ >= 8
#define POINTER_BITS 64
#elif __SIZEOF_POINTER__ >= 4
#define POINTER_BITS 32
#elif __SIZEOF_POINTER__ >= 2
#define POINTER_BITS 16
#else /* __SIZEOF_POINTER__ >= ... */
#define POINTER_BITS 8
#endif /* __SIZEOF_POINTER__ < ... */

typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t l_lock; /* Managed lock */
#endif /* !CONFIG_NO_THREADS */
} DeeAtomicLockObject;

typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_shared_lock_t l_lock; /* Managed lock */
#endif /* !CONFIG_NO_THREADS */
} DeeSharedLockObject;

typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t rwl_lock; /* Managed lock */
#endif /* !CONFIG_NO_THREADS */
} DeeAtomicRWLockObject;

typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_shared_rwlock_t rwl_lock; /* Managed lock */
#endif /* !CONFIG_NO_THREADS */
} DeeSharedRWLockObject;

typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_ratomic_lock_t l_lock; /* Managed lock */
#endif /* !CONFIG_NO_THREADS */
} DeeRAtomicLockObject;

typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_rshared_lock_t l_lock; /* Managed lock */
#endif /* !CONFIG_NO_THREADS */
} DeeRSharedLockObject;

typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_ratomic_rwlock_t rwl_lock; /* Managed lock */
#endif /* !CONFIG_NO_THREADS */
} DeeRAtomicRWLockObject;

typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_rshared_rwlock_t rwl_lock; /* Managed lock */
#endif /* !CONFIG_NO_THREADS */
} DeeRSharedRWLockObject;

typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_semaphore_t sem_semaphore; /* Managed semaphore */
#endif /* !CONFIG_NO_THREADS */
} DeeSemaphoreObject;

typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_event_t e_event; /* Managed event */
#endif /* !CONFIG_NO_THREADS */
} DeeEventObject;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *grwl_lock; /* [1..1][const] Pointed-to rwlock object. */
} DeeGenericRWLockProxyObject;

#define sizeof_field(T, s) sizeof(((T *)0)->s)

/* Assert that shared lock objects are binary-compatible with atomic lock objects. */
#define ASSERT_BINARY_COMPATIBLE_FIELDS(T1, f1, T2, f2)          \
	STATIC_ASSERT(sizeof_field(T1, f1) == sizeof_field(T2, f2)); \
	STATIC_ASSERT(offsetof(T1, f1) == offsetof(T2, f2))
ASSERT_BINARY_COMPATIBLE_FIELDS(DeeAtomicLockObject, l_lock, DeeSharedLockObject, l_lock.s_lock);
ASSERT_BINARY_COMPATIBLE_FIELDS(DeeAtomicRWLockObject, rwl_lock, DeeSharedRWLockObject, rwl_lock.srw_lock);
ASSERT_BINARY_COMPATIBLE_FIELDS(DeeRAtomicLockObject, l_lock, DeeRSharedLockObject, l_lock.rs_lock);
ASSERT_BINARY_COMPATIBLE_FIELDS(DeeRAtomicRWLockObject, rwl_lock, DeeRSharedRWLockObject, rwl_lock.rsrw_lock);
#undef ASSERT_BINARY_COMPATIBLE_FIELDS

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_lock_not_acquired(DeeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_ValueError, "Lock not acquired: %k", self);
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_read_lock_not_acquired(DeeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_ValueError, "Read-lock not acquired: %k", self);
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_write_lock_not_acquired(DeeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_ValueError, "Write-lock not acquired: %k", self);
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_read_or_write_lock_not_acquired(DeeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_ValueError, "Read- or write-lock not acquired: %k", self);
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_rwlock_with_readers_and_writers(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Cannot initialize RWLock with both readers and writers");
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_rwlock_too_many_readers(uintptr_t readers) {
	return DeeError_Throwf(&DeeError_IntegerOverflow,
	                       "Specified reader count %" PRFuPTR " is too large",
	                       readers);
}


/*[[[deemon
local STRINGS = {
	"tryacquire",
	"timedacquire",
	"acquire",
	"release",
	"acquired",
	"available",
	"waitfor",
	"timedwaitfor",
	"tryread",
	"trywrite",
	"endread",
	"endwrite",
	"reading",
	"writing",
	"read",
	"write",
	"timedread",
	"timedwrite",
	"waitread",
	"waitwrite",
	"timedwaitread",
	"timedwaitwrite",
	"canread",
	"canwrite",
	"exclusive",
	"shared",
	"tryupgrade",
	"upgrade",
	"downgrade",
	"end",
};
for (local s: STRINGS)
	(PRIVATE_DEFINE_STRING from rt.gen.string)(f"str_{s}", s);
for (local s: STRINGS)
	print("#define STR_", s, " " * ((STRINGS.each.length > ...) - #s), " DeeString_STR(&str_", s, ")");
]]]*/
PRIVATE DEFINE_STRING_EX(str_tryacquire, "tryacquire", 0x77c23503, 0xb4a4aa2e5f48e165);
PRIVATE DEFINE_STRING_EX(str_timedacquire, "timedacquire", 0x60c85ce4, 0xcb5311708bc2b051);
PRIVATE DEFINE_STRING_EX(str_acquire, "acquire", 0xb87de1e4, 0x4555fd7fb6829a7e);
PRIVATE DEFINE_STRING_EX(str_release, "release", 0x14b4fc94, 0x5231029cc695388d);
PRIVATE DEFINE_STRING_EX(str_acquired, "acquired", 0xe789fa25, 0x4e0e81b1ea53d7e3);
PRIVATE DEFINE_STRING_EX(str_available, "available", 0xc2366131, 0xee9ea76d7339ffee);
PRIVATE DEFINE_STRING_EX(str_waitfor, "waitfor", 0xf161c6ee, 0xd7ad4effa0ea131f);
PRIVATE DEFINE_STRING_EX(str_timedwaitfor, "timedwaitfor", 0xb57d86c4, 0x7610288c0ed17e72);
PRIVATE DEFINE_STRING_EX(str_tryread, "tryread", 0x1b8a76ae, 0x8467e68ce4595d83);
PRIVATE DEFINE_STRING_EX(str_trywrite, "trywrite", 0x4d7785ca, 0x623f338c3fd5339);
PRIVATE DEFINE_STRING_EX(str_endread, "endread", 0xd19c7b3a, 0x6a2ff12bd6ec0458);
PRIVATE DEFINE_STRING_EX(str_endwrite, "endwrite", 0x26a5fe08, 0xaf386d76d55b2335);
PRIVATE DEFINE_STRING_EX(str_reading, "reading", 0xf13b3c3d, 0x432d315647d3c71a);
PRIVATE DEFINE_STRING_EX(str_writing, "writing", 0x4b0a2f5f, 0x6ee13ac60d55f503);
PRIVATE DEFINE_STRING_EX(str_read, "read", 0xcda20821, 0xb4f41304e896cb09);
PRIVATE DEFINE_STRING_EX(str_write, "write", 0xbd1dfa8c, 0xcf6faa4fbd36ac24);
PRIVATE DEFINE_STRING_EX(str_timedread, "timedread", 0xbe2dcc67, 0x4d983722643f1f37);
PRIVATE DEFINE_STRING_EX(str_timedwrite, "timedwrite", 0xcfe33f1c, 0xab3a8edc8619a3a4);
PRIVATE DEFINE_STRING_EX(str_waitread, "waitread", 0x808ae63c, 0xca59370da2c72e5f);
PRIVATE DEFINE_STRING_EX(str_waitwrite, "waitwrite", 0x51b7ddd3, 0x1f7ca64da4739f8f);
PRIVATE DEFINE_STRING_EX(str_timedwaitread, "timedwaitread", 0xfee13f70, 0x1c54db2080c11bc3);
PRIVATE DEFINE_STRING_EX(str_timedwaitwrite, "timedwaitwrite", 0x683b59e9, 0x75ce39f41579afec);
PRIVATE DEFINE_STRING_EX(str_canread, "canread", 0xa7dcc052, 0xf575d512b203f6ce);
PRIVATE DEFINE_STRING_EX(str_canwrite, "canwrite", 0x6dee47e, 0xe7d41485f26ddd04);
PRIVATE DEFINE_STRING_EX(str_exclusive, "exclusive", 0x4df8c743, 0xc4bc49ba4e9b74b);
PRIVATE DEFINE_STRING_EX(str_shared, "shared", 0x15563460, 0x25b1ae5177c3628b);
PRIVATE DEFINE_STRING_EX(str_tryupgrade, "tryupgrade", 0xa9586b25, 0xbd25f6075bfb97b6);
PRIVATE DEFINE_STRING_EX(str_upgrade, "upgrade", 0xe72664e6, 0xd934e23ca6727afa);
PRIVATE DEFINE_STRING_EX(str_downgrade, "downgrade", 0x70347eef, 0x5a7fee27c4b1e0f);
PRIVATE DEFINE_STRING_EX(str_end, "end", 0x37fb4a05, 0x6de935c204dc3d01);
#define STR_tryacquire     DeeString_STR(&str_tryacquire)
#define STR_timedacquire   DeeString_STR(&str_timedacquire)
#define STR_acquire        DeeString_STR(&str_acquire)
#define STR_release        DeeString_STR(&str_release)
#define STR_acquired       DeeString_STR(&str_acquired)
#define STR_available      DeeString_STR(&str_available)
#define STR_waitfor        DeeString_STR(&str_waitfor)
#define STR_timedwaitfor   DeeString_STR(&str_timedwaitfor)
#define STR_tryread        DeeString_STR(&str_tryread)
#define STR_trywrite       DeeString_STR(&str_trywrite)
#define STR_endread        DeeString_STR(&str_endread)
#define STR_endwrite       DeeString_STR(&str_endwrite)
#define STR_reading        DeeString_STR(&str_reading)
#define STR_writing        DeeString_STR(&str_writing)
#define STR_read           DeeString_STR(&str_read)
#define STR_write          DeeString_STR(&str_write)
#define STR_timedread      DeeString_STR(&str_timedread)
#define STR_timedwrite     DeeString_STR(&str_timedwrite)
#define STR_waitread       DeeString_STR(&str_waitread)
#define STR_waitwrite      DeeString_STR(&str_waitwrite)
#define STR_timedwaitread  DeeString_STR(&str_timedwaitread)
#define STR_timedwaitwrite DeeString_STR(&str_timedwaitwrite)
#define STR_canread        DeeString_STR(&str_canread)
#define STR_canwrite       DeeString_STR(&str_canwrite)
#define STR_exclusive      DeeString_STR(&str_exclusive)
#define STR_shared         DeeString_STR(&str_shared)
#define STR_tryupgrade     DeeString_STR(&str_tryupgrade)
#define STR_upgrade        DeeString_STR(&str_upgrade)
#define STR_downgrade      DeeString_STR(&str_downgrade)
#define STR_end            DeeString_STR(&str_end)
/*[[[end]]]*/


/* Pre-defined doc strings for lock-like objects */
DOC_DEF(doc_lock_acquire,
        "()\n"
        "@interrupt\n"
        "Acquire @this lock, blocking until that becomes possible");
DOC_DEF(doc_lock_release,
        "()\n"
        "@throws ValueError You're not holding this lock\n"
        "Release a lock previously acquired by ?#acquire or some other means");
DOC_DEF(doc_lock_tryacquire,
        "->?Dbool\n"
        "Try to acquire @this lock, returning !t on success, and !f if doing so would block");
DOC_DEF(doc_lock_timedacquire,
        "(timeout_nanoseconds:?Dint)->?Dbool\n"
        "@interrupt\n"
        "Same as ?#acquire, returning !t on success, but block for at "
        /**/ "most @timeout_nanoseconds. Once that amount of time has elapsed, "
        /**/ "stop trying to acquire the lock and fail by returning !f instead.");
DOC_DEF(doc_lock_waitfor,
        "()\n"
        "@interrupt\n"
        "Wait until @this lock can be acquired without blocking\n"
        "Note that by the time this function returns, trying to acquire the "
        /**/ "lock might already block once again.");
DOC_DEF(doc_lock_timedwaitfor,
        "(timeout_nanoseconds:?Dint)->?Dbool\n"
        "@interrupt\n"
        "Same as ?#waitfor, returning !t on success, but block for at "
        /**/ "most @timeout_nanoseconds. Once that amount of time has elapsed, "
        /**/ "stop trying to wait for the lock to become available, and fail "
        /**/ "by returning !f instead.");
DOC_DEF(doc_lock_available,
        "->?Dbool\n"
        "Check if @this lock could currently be acquired without blocking");
DOC_DEF(doc_lock_acquired,
        "->?Dbool\n"
        "Check if @this lock is currently being held");


/* Pre-defined doc strings for rwlock-like objects */
DOC_DEF(doc_rwlock_tryread,
        "->?Dbool\n"
        "Try to acquire a shared lock to @this.\n"
        "Same as ${this.shared.tryacquire()}");
DOC_DEF(doc_rwlock_trywrite,
        "->?Dbool\n"
        "Try to acquire an exclusive lock to @this.\n"
        "Same as ${this.exclusive.tryacquire()}");
DOC_DEF(doc_rwlock_read,
        "()\n"
        "@interrupt\n"
        "Acquire a shared lock to @this, blocking until that becomes possible.\n"
        "Same as ${this.shared.tryacquire()}");
DOC_DEF(doc_rwlock_timedread,
        "(timeout_nanoseconds:?Dint)->?Dbool\n"
        "@interrupt\n"
        "Same as ?#read, returning !t on success, but block for at "
        /**/ "most @timeout_nanoseconds. Once that amount of time has elapsed, "
        /**/ "stop trying to acquire a shared lock and fail by returning !f instead.\n"
        "Same as ${this.shared.timedacquire(timeout_nanoseconds)}");
DOC_DEF(doc_rwlock_write,
        "()\n"
        "@interrupt\n"
        "Acquire an exclusive lock to @this, blocking until that becomes possible.\n"
        "Same as ${this.exclusive.tryacquire()}");
DOC_DEF(doc_rwlock_timedwrite,
        "(timeout_nanoseconds:?Dint)->?Dbool\n"
        "@interrupt\n"
        "Same as ?#write, returning !t on success, but block for at "
        /**/ "most @timeout_nanoseconds. Once that amount of time has elapsed, "
        /**/ "stop trying to acquire an exclusive lock and fail by returning !f instead.\n"
        "Same as ${this.exclusive.timedacquire(timeout_nanoseconds)}");
DOC_DEF(doc_rwlock_waitread,
        "()\n"
        "@interrupt\n"
        "Wait until a shared lock can be acquired without blocking\n"
        "Note that by the time this function returns, trying to acquire a shared "
        /**/ "lock might already block once again.\n"
        "Same as ${this.shared.waitfor()}");
DOC_DEF(doc_rwlock_timedwaitread,
        "(timeout_nanoseconds:?Dint)->?Dbool\n"
        "@interrupt\n"
        "Same as ?#waitread, returning !t on success, but block for at "
        /**/ "most @timeout_nanoseconds. Once that amount of time has elapsed, "
        /**/ "stop trying to wait for a shared lock to become available, and "
        /**/ "fail by returning !f instead.\n"
        "Same as ${this.shared.timedwaitfor(timeout_nanoseconds)}");
DOC_DEF(doc_rwlock_waitwrite,
        "()\n"
        "@interrupt\n"
        "Wait until an exclusive lock can be acquired without blocking\n"
        "Note that by the time this function returns, trying to acquire an exclusive "
        /**/ "lock might already block once again.\n"
        "Same as ${this.exclusive.waitfor()}");
DOC_DEF(doc_rwlock_timedwaitwrite,
        "(timeout_nanoseconds:?Dint)->?Dbool\n"
        "@interrupt\n"
        "Same as ?#waitwrite, returning !t on success, but block for at "
        /**/ "most @timeout_nanoseconds. Once that amount of time has elapsed, "
        /**/ "stop trying to wait for an exclusive lock to become available, and "
        /**/ "fail by returning !f instead.\n"
        "Same as ${this.exclusive.timedwaitfor(timeout_nanoseconds)}");
DOC_DEF(doc_rwlock_endread,
        "()\n"
        "@throws ValueError No shared lock is currently held\n"
        "Release a shared lock from @this, previously acquired by ?#read or some other function.\n"
        "Same as ${this.shared.release()}");
DOC_DEF(doc_rwlock_endwrite,
        "()\n"
        "@throws ValueError No exclusive lock is currently held\n"
        "Release an exclusive lock from @this, previously acquired by ?#write or some other function.\n"
        "Same as ${this.exclusive.release()}");
DOC_DEF(doc_rwlock_end,
        "()\n"
        "@throws ValueError No lock of any kind is currently held\n"
        "Release either an exclusive, or a shared lock from @this");
DOC_DEF(doc_rwlock_tryupgrade,
        "->?DBool\n"
        "Try to upgrade a shared lock (s.a. ?#read) into an exclusive lock (s.a. ?#write)\n"
        "If the lock was upgraded, return !t. Otherwise, the shared lock is kept, and !f is returned.");
DOC_DEF(doc_rwlock_upgrade,
        "->?DBool\n"
        "@interrupt\n"
        "@throws ValueError Not holding a shared lock at the moment\n"
        "@return true Lock upgrade could be performed atomically (without dropping the shared lock temporarily)\n"
        "@return false Lock upgrade was performed by temporarily dropping the shared lock, before acquiring an exclusive lock\n"
        "Upgrade a shared lock into an exclusive lock (if an interrupt error is thrown, the shared lock was released)");
DOC_DEF(doc_rwlock_downgrade,
        "()\n"
        "@throws ValueError Not holding an exclusive lock at the moment\n"
        "Downgrade an exclusive lock into a shared lock, but maintain some kind of lock at every point in time");
DOC_DEF(doc_rwlock_reading,
        "->?Dbool\n"
        "Returns !t if either a shared- or an exclusive lock is being held");
DOC_DEF(doc_rwlock_writing,
        "->?Dbool\n"
        "Returns !t if some thread is holding an exclusive lock");
DOC_DEF(doc_rwlock_canread,
        "->?Dbool\n"
        "Returns !t so-long as no-one is holding an exclusive lock (inverse of ?#writing)");
DOC_DEF(doc_rwlock_canwrite,
        "->?Dbool\n"
        "Returns !t so-long as no shared- and no exclusive locks are being held (inverse of ?#reading)");
DOC_DEF(doc_rwlock_shared,
        "->?GRWLockSharedLock\n"
        "Return a ?GLock-compatible wrapper object around @this read/write-lock "
        /**/ "that can be used to acquire a shared (read) locks");
DOC_DEF(doc_rwlock_exclusive,
        "->?GRWLockExclusiveLock\n"
        "Return a ?GLock-compatible wrapper object around @this read/write-lock "
        /**/ "that can be used to acquire an exclusive (write) locks");









/************************************************************************/
/* Abstract Lock API                                                    */
/************************************************************************/
PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_ctor(DeeObject *__restrict self) {
	(void)self;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_enter(DeeObject *__restrict self) {
	DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_acquire, 0, NULL);
	Dee_XDecref(result);
	return likely(result) ? 0 : -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_leave(DeeObject *__restrict self) {
	DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_release, 0, NULL);
	Dee_XDecref(result);
	return likely(result) ? 0 : -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_tryenter(DeeObject *__restrict self) {
	int result;
	DeeObject *result_ob;
	result_ob = DeeObject_CallAttr(self, (DeeObject *)&str_tryacquire, 0, NULL);
	if unlikely(!result_ob)
		goto err;
	result = DeeObject_Bool(result_ob);
	Dee_Decref(result_ob);
	return result;
err:
	return -1;
}

PRIVATE struct type_with lock_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&lock_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&lock_leave
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_acquire(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":acquire"))
		goto err;
	for (;;) {
		int error = lock_tryenter(self);
		if unlikely(error < 0)
			goto err;
		if (error)
			break;
		if (DeeThread_CheckInterrupt())
			goto err;
		SCHED_YIELD();
	}
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_timedacquire(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	uint64_t timeout_nanoseconds;
	uint64_t now_microseconds, then_microseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedacquire", &timeout_nanoseconds))
		goto err;
	error = lock_tryenter(self);
	if unlikely(error < 0)
		goto err;
	if (error)
		goto ok;
	if (timeout_nanoseconds == (uint64_t)-1) {
		DREF DeeObject *result;
do_infinite_timeout:
		result = DeeObject_CallAttr(self, (DeeObject *)&str_acquire, 0, NULL);
		if unlikely(!result)
			goto err;
		Dee_Decref(result);
		goto ok;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	error = lock_tryenter(self);
	if unlikely(error < 0)
		goto err;
	if (error)
		goto ok;
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return_false; /* Timeout */
	timeout_nanoseconds *= 1000;
	if (DeeThread_CheckInterrupt())
		goto err;
	SCHED_YIELD();
	goto do_wait_with_timeout;
ok:
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_is_acquired(DeeObject *__restrict self) {
	int result;
	DeeObject *result_ob;
	result_ob = DeeObject_GetAttr(self, (DeeObject *)&str_acquired);
	if unlikely(!result_ob)
		goto err;
	result = DeeObject_Bool(result_ob);
	Dee_Decref(result_ob);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_available_get(DeeObject *__restrict self) {
	int result = lock_is_acquired(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(!result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_waitfor(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":waitfor"))
		goto err;
	for (;;) {
		int error = lock_is_acquired(self);
		if unlikely(error < 0)
			goto err;
		if (!error)
			break;
		if (DeeThread_CheckInterrupt())
			goto err;
		SCHED_YIELD();
	}
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_timedwaitfor(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	uint64_t timeout_nanoseconds;
	uint64_t now_microseconds, then_microseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedwaitfor", &timeout_nanoseconds))
		goto err;
	error = lock_is_acquired(self);
	if unlikely(error < 0)
		goto err;
	if (!error)
		goto ok;
	if (timeout_nanoseconds == (uint64_t)-1) {
		DREF DeeObject *result;
do_infinite_timeout:
		result = DeeObject_CallAttr(self, (DeeObject *)&str_waitfor, 0, NULL);
		if unlikely(!result)
			goto err;
		Dee_Decref(result);
		goto ok;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	error = lock_is_acquired(self);
	if unlikely(error < 0)
		goto err;
	if (!error)
		goto ok;
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return_false; /* Timeout */
	timeout_nanoseconds *= 1000;
	if (DeeThread_CheckInterrupt())
		goto err;
	SCHED_YIELD();
	goto do_wait_with_timeout;
ok:
	return_true;
err:
	return NULL;
}

PRIVATE struct type_method lock_methods[] = {
	TYPE_METHOD(STR_acquire, &lock_acquire, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD(STR_timedacquire, &lock_timedacquire, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD(STR_waitfor, &lock_waitfor, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD(STR_timedwaitfor, &lock_timedwaitfor, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset lock_getsets[] = {
	TYPE_GETTER(STR_available, &lock_available_get, DOC_GET(doc_lock_available)),
	TYPE_GETSET_END
};

INTERN DeeTypeObject DeeLock_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Lock",
	/* .tp_doc      = */ DOC("Abstract base class for lock-objects.\n"
	                         "Sub-classes of this type must implement at least the following members:\n"
	                         "${"
	                         /**/ "function tryacquire(): bool;\n"
	                         /**/ "function release();\n"
	                         /**/ "property acquired: bool = { get(): bool; };"
	                         "}\n"
	                         "Unless overwritten by a sub-class, this type implements the "
	                         "following functions through use of the above attributes:\n"
	                         "${"
	                         /**/ "function acquire();\n"
	                         /**/ "function timedacquire(timeout_nanoseconds: int): bool;\n"
	                         /**/ "function waitfor();\n"
	                         /**/ "function timedwaitfor(timeout_nanoseconds: int): bool;\n"
	                         /**/ "property available: bool = { get(): bool; };"
	                         "}\n"
	                         "This base-class also implements ?#{op:enter} and ?#{op:leave} such that they "
	                         "will call the #Cacquire and #Crelease member functions of the sub-class."),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&lock_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ &lock_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ lock_methods,
	/* .tp_getsets       = */ lock_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};









/************************************************************************/
/* Abstract RWLock API                                                  */
/************************************************************************/
#define rwlock_ctor lock_ctor
PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_tryread(DeeObject *self) {
	int result;
	DeeObject *result_ob;
	result_ob = DeeObject_CallAttr(self, (DeeObject *)&str_tryread, 0, NULL);
	if unlikely(!result_ob)
		goto err;
	result = DeeObject_Bool(result_ob);
	Dee_Decref(result_ob);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_trywrite(DeeObject *self) {
	int result;
	DeeObject *result_ob;
	result_ob = DeeObject_CallAttr(self, (DeeObject *)&str_trywrite, 0, NULL);
	if unlikely(!result_ob)
		goto err;
	result = DeeObject_Bool(result_ob);
	Dee_Decref(result_ob);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_tryupgrade(DeeObject *self) {
	int result;
	DeeObject *result_ob;
	result_ob = DeeObject_CallAttr(self, (DeeObject *)&str_tryupgrade, 0, NULL);
	if unlikely(!result_ob)
		goto err;
	result = DeeObject_Bool(result_ob);
	Dee_Decref(result_ob);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_endread(DeeObject *self) {
	DeeObject *result_ob;
	result_ob = DeeObject_CallAttr(self, (DeeObject *)&str_endread, 0, NULL);
	if unlikely(!result_ob)
		goto err;
	Dee_Decref(result_ob);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_endwrite(DeeObject *self) {
	DeeObject *result_ob;
	result_ob = DeeObject_CallAttr(self, (DeeObject *)&str_endwrite, 0, NULL);
	if unlikely(!result_ob)
		goto err;
	Dee_Decref(result_ob);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_reading(DeeObject *self) {
	int result;
	DeeObject *result_ob;
	result_ob = DeeObject_GetAttr(self, (DeeObject *)&str_reading);
	if unlikely(!result_ob)
		goto err;
	result = DeeObject_Bool(result_ob);
	Dee_Decref(result_ob);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_writing(DeeObject *self) {
	int result;
	DeeObject *result_ob;
	result_ob = DeeObject_GetAttr(self, (DeeObject *)&str_writing);
	if unlikely(!result_ob)
		goto err;
	result = DeeObject_Bool(result_ob);
	Dee_Decref(result_ob);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_read(DeeObject *self) {
	for (;;) {
		int error = rwlock_do_tryread(self);
		if unlikely(error < 0)
			goto err;
		if (error)
			break;
		if (DeeThread_CheckInterrupt())
			goto err;
		SCHED_YIELD();
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_write(DeeObject *self) {
	for (;;) {
		int error = rwlock_do_trywrite(self);
		if unlikely(error < 0)
			goto err;
		if (error)
			break;
		if (DeeThread_CheckInterrupt())
			goto err;
		SCHED_YIELD();
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_waitread(DeeObject *self) {
	for (;;) {
		int error = rwlock_do_writing(self);
		if unlikely(error < 0)
			goto err;
		if (!error)
			break;
		if (DeeThread_CheckInterrupt())
			goto err;
		SCHED_YIELD();
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_waitwrite(DeeObject *self) {
	for (;;) {
		int error = rwlock_do_reading(self);
		if unlikely(error < 0)
			goto err;
		if (!error)
			break;
		if (DeeThread_CheckInterrupt())
			goto err;
		SCHED_YIELD();
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_timedread(DeeObject *self, uint64_t timeout_nanoseconds) {
	int error;
	uint64_t now_microseconds, then_microseconds;
	error = rwlock_do_tryread(self);
	if (error <= 0)
		return error; /* Error or success */
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		return rwlock_do_read(self);
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	error = rwlock_do_tryread(self);
	if (error <= 0)
		return error; /* Error or success */
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	if (DeeThread_CheckInterrupt())
		goto err;
	SCHED_YIELD();
	goto do_wait_with_timeout;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_timedwrite(DeeObject *self, uint64_t timeout_nanoseconds) {
	int error;
	uint64_t now_microseconds, then_microseconds;
	error = rwlock_do_trywrite(self);
	if (error <= 0)
		return error; /* Error or success */
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		return rwlock_do_write(self);
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	error = rwlock_do_trywrite(self);
	if (error <= 0)
		return error; /* Error or success */
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	if (DeeThread_CheckInterrupt())
		goto err;
	SCHED_YIELD();
	goto do_wait_with_timeout;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_timedwaitread(DeeObject *self, uint64_t timeout_nanoseconds) {
	int error;
	uint64_t now_microseconds, then_microseconds;
	error = rwlock_do_writing(self);
	if (error <= 0)
		return error; /* Error or success */
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		return rwlock_do_waitread(self);
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	error = rwlock_do_writing(self);
	if (error <= 0)
		return error; /* Error or success */
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	if (DeeThread_CheckInterrupt())
		goto err;
	SCHED_YIELD();
	goto do_wait_with_timeout;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_timedwaitwrite(DeeObject *self, uint64_t timeout_nanoseconds) {
	int error;
	uint64_t now_microseconds, then_microseconds;
	error = rwlock_do_reading(self);
	if (error <= 0)
		return error; /* Error or success */
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		return rwlock_do_waitwrite(self);
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	error = rwlock_do_reading(self);
	if (error <= 0)
		return error; /* Error or success */
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
		return 1; /* Timeout */
	timeout_nanoseconds *= 1000;
	if (DeeThread_CheckInterrupt())
		goto err;
	SCHED_YIELD();
	goto do_wait_with_timeout;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_read(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":read"))
		goto err;
	if unlikely(rwlock_do_read(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_write(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":write"))
		goto err;
	if unlikely(rwlock_do_write(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_end(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	if (DeeArg_Unpack(argc, argv, ":end"))
		goto err;
	error = rwlock_do_writing(self);
	if unlikely(error < 0)
		goto err;
	if (error) {
		error = rwlock_do_endwrite(self);
	} else {
		error = rwlock_do_endread(self);
	}
	if unlikely(error)
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_upgrade(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	if (DeeArg_Unpack(argc, argv, ":upgrade"))
		goto err;
	error = rwlock_do_tryupgrade(self);
	if unlikely(error < 0)
		goto err;
	if (error)
		return_true;
	if unlikely(rwlock_do_endread(self))
		goto err;
	if unlikely(rwlock_do_write(self))
		goto err;
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_timedread(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	uint64_t timeout_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedread", &timeout_nanoseconds))
		goto err;
	error = rwlock_do_timedread(self, timeout_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_timedwrite(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	uint64_t timeout_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedwrite", &timeout_nanoseconds))
		goto err;
	error = rwlock_do_timedwrite(self, timeout_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_waitread(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":waitread"))
		goto err;
	if unlikely(rwlock_do_waitread(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_waitwrite(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":waitwrite"))
		goto err;
	if unlikely(rwlock_do_waitwrite(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_timedwaitread(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	uint64_t timeout_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedwaitread", &timeout_nanoseconds))
		goto err;
	error = rwlock_do_timedwaitread(self, timeout_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_timedwaitwrite(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	uint64_t timeout_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedwaitwrite", &timeout_nanoseconds))
		goto err;
	error = rwlock_do_timedwaitwrite(self, timeout_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_canread_get(DeeObject *__restrict self) {
	int error = rwlock_do_writing(self);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_canwrite_get(DeeObject *__restrict self) {
	int error = rwlock_do_reading(self);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeGenericRWLockProxyObject *DCALL
rwlock_shared_get(DeeObject *__restrict self) {
	DREF DeeGenericRWLockProxyObject *result;
	result = DeeObject_MALLOC(DeeGenericRWLockProxyObject);
	if unlikely(!result)
		goto done;
	result->grwl_lock = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeRWLockSharedLock_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeGenericRWLockProxyObject *DCALL
rwlock_exclusive_get(DeeObject *__restrict self) {
	DREF DeeGenericRWLockProxyObject *result;
	result = DeeObject_MALLOC(DeeGenericRWLockProxyObject);
	if unlikely(!result)
		goto done;
	result->grwl_lock = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeRWLockExclusiveLock_Type);
done:
	return result;
}

PRIVATE struct type_method rwlock_methods[] = {
	TYPE_METHOD(STR_read, &rwlock_read, DOC_GET(doc_rwlock_read)),
	TYPE_METHOD(STR_write, &rwlock_write, DOC_GET(doc_rwlock_write)),
	TYPE_METHOD(STR_end, &rwlock_end, DOC_GET(doc_rwlock_end)),
	TYPE_METHOD(STR_upgrade, &rwlock_upgrade, DOC_GET(doc_rwlock_upgrade)),
	TYPE_METHOD(STR_timedread, &rwlock_timedread, DOC_GET(doc_rwlock_timedread)),
	TYPE_METHOD(STR_timedwrite, &rwlock_timedwrite, DOC_GET(doc_rwlock_timedwrite)),
	TYPE_METHOD(STR_waitread, &rwlock_waitread, DOC_GET(doc_rwlock_waitread)),
	TYPE_METHOD(STR_waitwrite, &rwlock_waitwrite, DOC_GET(doc_rwlock_waitwrite)),
	TYPE_METHOD(STR_timedwaitread, &rwlock_timedwaitread, DOC_GET(doc_rwlock_timedwaitread)),
	TYPE_METHOD(STR_timedwaitwrite, &rwlock_timedwaitwrite, DOC_GET(doc_rwlock_timedwaitwrite)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset rwlock_getsets[] = {
	TYPE_GETTER(STR_canread, &rwlock_canread_get, DOC_GET(doc_rwlock_canread)),
	TYPE_GETTER(STR_canwrite, &rwlock_canwrite_get, DOC_GET(doc_rwlock_canwrite)),
	TYPE_GETTER(STR_shared, &rwlock_shared_get, DOC_GET(doc_rwlock_shared)),
	TYPE_GETTER(STR_exclusive, &rwlock_exclusive_get, DOC_GET(doc_rwlock_exclusive)),
	TYPE_GETSET_END
};

INTERN DeeTypeObject DeeRWLock_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "RWLock",
	/* .tp_doc      = */ DOC("Abstract base class for read/write lock-objects.\n"
	                         "Sub-classes of this type must implement at least the following member functions:\n"
	                         "${"
	                         /**/ "function tryread(): bool;\n"
	                         /**/ "function trywrite(): bool;\n"
	                         /**/ "function tryupgrade(): bool;\n"
	                         /**/ "function endread();\n"
	                         /**/ "function endwrite();\n"
	                         /**/ "function downgrade();\n"
	                         /**/ "property reading: bool = { get(): bool; };\n"
	                         /**/ "property writing: bool = { get(): bool; };"
	                         "}\n"
	                         "Unless overwritten by a sub-class, this type implements the "
	                         "following functions through use of the above attributes:\n"
	                         "${"
	                         /**/ "function read();\n"
	                         /**/ "function write();\n"
	                         /**/ "function end();\n"
	                         /**/ "function upgrade(): bool;\n"
	                         /**/ "function timedread(timeout_nanoseconds: int): bool;\n"
	                         /**/ "function timedwrite(timeout_nanoseconds: int): bool;\n"
	                         /**/ "function waitread();\n"
	                         /**/ "function waitwrite();\n"
	                         /**/ "function timedwaitread(timeout_nanoseconds: int): bool;\n"
	                         /**/ "function timedwaitwrite(timeout_nanoseconds: int): bool;\n"
	                         /**/ "property canread: bool = { get(): bool; };\n"
	                         /**/ "property canwrite: bool = { get(): bool; };\n"
	                         /**/ "property shared: RWLockSharedLock = { get(): RWLockSharedLock; };\n"
	                         /**/ "property exclusive: RWLockExclusiveLock = { get(): RWLockExclusiveLock; };"
	                         "}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rwlock_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rwlock_methods,
	/* .tp_getsets       = */ rwlock_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_shared_init(DeeGenericRWLockProxyObject *__restrict self,
                   size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:RWLockSharedLock", &self->grwl_lock))
		goto err;
	if (DeeObject_AssertType(self->grwl_lock, &DeeLock_Type))
		goto err;
	Dee_Incref(self->grwl_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_exclusive_init(DeeGenericRWLockProxyObject *__restrict self,
                      size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:RWLockExclusiveLock", &self->grwl_lock))
		goto err;
	if (DeeObject_AssertType(self->grwl_lock, &DeeLock_Type))
		goto err;
	Dee_Incref(self->grwl_lock);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
rwlock_proxy_fini(DeeGenericRWLockProxyObject *__restrict self) {
	Dee_Decref(self->grwl_lock);
}

PRIVATE NONNULL((1, 2)) void DCALL
rwlock_proxy_visit(DeeGenericRWLockProxyObject *__restrict self,
                   Dee_visit_t proc, void *arg) {
	Dee_Visit(self->grwl_lock);
}

PRIVATE struct type_member rwlock_proxy_members[] = {
	TYPE_MEMBER_FIELD_DOC("rwlock", STRUCT_OBJECT, offsetof(DeeGenericRWLockProxyObject, grwl_lock), "->?GRWLock"),
	TYPE_MEMBER_END
};

#define rwlock_shared_fini       rwlock_proxy_fini
#define rwlock_shared_visit      rwlock_proxy_visit
#define rwlock_shared_members    rwlock_proxy_members
#define rwlock_exclusive_fini    rwlock_proxy_fini
#define rwlock_exclusive_visit   rwlock_proxy_visit
#define rwlock_exclusive_members rwlock_proxy_members

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rwlock_shared_print(DeeGenericRWLockProxyObject *__restrict self,
                    dformatprinter printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<Shared Lock for %k>", self->grwl_lock);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rwlock_exclusive_print(DeeGenericRWLockProxyObject *__restrict self,
                       dformatprinter printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<Exclusive Lock for %k>", self->grwl_lock);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rwlock_shared_printrepr(DeeGenericRWLockProxyObject *__restrict self,
                        dformatprinter printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "%r.shared", self->grwl_lock);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rwlock_exclusive_printrepr(DeeGenericRWLockProxyObject *__restrict self,
                           dformatprinter printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "%r.exclusive", self->grwl_lock);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_shared_enter(DeeGenericRWLockProxyObject *__restrict self) {
	DREF DeeObject *temp;
	temp = DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_read, 0, NULL);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_shared_leave(DeeGenericRWLockProxyObject *__restrict self) {
	DREF DeeObject *temp;
	temp = DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_endread, 0, NULL);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_exclusive_enter(DeeGenericRWLockProxyObject *__restrict self) {
	DREF DeeObject *temp;
	temp = DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_write, 0, NULL);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_exclusive_leave(DeeGenericRWLockProxyObject *__restrict self) {
	DREF DeeObject *temp;
	temp = DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_endwrite, 0, NULL);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_shared_tryacquire(DeeGenericRWLockProxyObject *self,
                         size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_tryread, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_exclusive_tryacquire(DeeGenericRWLockProxyObject *self,
                            size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_trywrite, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_shared_acquire(DeeGenericRWLockProxyObject *self,
                      size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_read, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_exclusive_acquire(DeeGenericRWLockProxyObject *self,
                         size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_write, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_shared_release(DeeGenericRWLockProxyObject *self,
                      size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_endread, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_exclusive_release(DeeGenericRWLockProxyObject *self,
                         size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_endwrite, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_shared_timedacquire(DeeGenericRWLockProxyObject *self,
                           size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_timedread, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_exclusive_timedacquire(DeeGenericRWLockProxyObject *self,
                              size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_timedwrite, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_shared_waitfor(DeeGenericRWLockProxyObject *self,
                      size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_waitread, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_exclusive_waitfor(DeeGenericRWLockProxyObject *self,
                         size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_waitwrite, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_shared_timedwaitfor(DeeGenericRWLockProxyObject *self,
                           size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_timedwaitread, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_exclusive_timedwaitfor(DeeGenericRWLockProxyObject *self,
                              size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, (DeeObject *)&str_timedwaitwrite, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_shared_available_get(DeeGenericRWLockProxyObject *__restrict self) {
	return DeeObject_GetAttr(self->grwl_lock, (DeeObject *)&str_canread);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_exclusive_available_get(DeeGenericRWLockProxyObject *__restrict self) {
	return DeeObject_GetAttr(self->grwl_lock, (DeeObject *)&str_canwrite);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_shared_acquired_get(DeeGenericRWLockProxyObject *__restrict self) {
	return DeeObject_GetAttr(self->grwl_lock, (DeeObject *)&str_reading);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_exclusive_acquired_get(DeeGenericRWLockProxyObject *__restrict self) {
	return DeeObject_GetAttr(self->grwl_lock, (DeeObject *)&str_writing);
}



PRIVATE struct type_with rwlock_shared_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&rwlock_shared_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&rwlock_shared_leave
};

PRIVATE struct type_with rwlock_exclusive_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&rwlock_exclusive_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&rwlock_exclusive_leave
};

PRIVATE struct type_method rwlock_shared_methods[] = {
	TYPE_METHOD(STR_tryacquire, &rwlock_shared_tryacquire, DOC_GET(doc_lock_tryacquire)),
	TYPE_METHOD(STR_acquire, &rwlock_shared_acquire, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD(STR_release, &rwlock_shared_release, DOC_GET(doc_lock_release)),
	TYPE_METHOD(STR_timedacquire, &rwlock_shared_timedacquire, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD(STR_waitfor, &rwlock_shared_waitfor, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD(STR_timedwaitfor, &rwlock_shared_timedwaitfor, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};

PRIVATE struct type_method rwlock_exclusive_methods[] = {
	TYPE_METHOD(STR_tryacquire, &rwlock_exclusive_tryacquire, DOC_GET(doc_lock_tryacquire)),
	TYPE_METHOD(STR_acquire, &rwlock_exclusive_acquire, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD(STR_release, &rwlock_exclusive_release, DOC_GET(doc_lock_release)),
	TYPE_METHOD(STR_timedacquire, &rwlock_exclusive_timedacquire, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD(STR_waitfor, &rwlock_exclusive_waitfor, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD(STR_timedwaitfor, &rwlock_exclusive_timedwaitfor, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset rwlock_shared_getsets[] = {
	TYPE_GETTER(STR_available, &rwlock_shared_available_get, DOC_GET(doc_lock_available)),
	TYPE_GETTER(STR_acquired, &rwlock_shared_acquired_get, DOC_GET(doc_lock_acquired)),
	TYPE_GETSET_END
};

PRIVATE struct type_getset rwlock_exclusive_getsets[] = {
	TYPE_GETTER(STR_available, &rwlock_exclusive_available_get, DOC_GET(doc_lock_available)),
	TYPE_GETTER(STR_acquired, &rwlock_exclusive_acquired_get, DOC_GET(doc_lock_acquired)),
	TYPE_GETSET_END
};

INTERN DeeTypeObject DeeRWLockSharedLock_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "RWLockSharedLock",
	/* .tp_doc      = */ DOC("Wrapper for a ?GRWLock's shared set of locking functions\n"
	                         "\n"
	                         "(lock:?GRWLock)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeLock_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&rwlock_shared_init,
				TYPE_FIXED_ALLOCATOR(DeeGenericRWLockProxyObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rwlock_shared_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&rwlock_shared_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&rwlock_shared_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rwlock_shared_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ &rwlock_shared_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rwlock_shared_methods,
	/* .tp_getsets       = */ rwlock_shared_getsets,
	/* .tp_members       = */ rwlock_shared_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DeeRWLockExclusiveLock_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "RWLockExclusiveLock",
	/* .tp_doc      = */ DOC("Wrapper for a ?GRWLock's exclusive set of locking functions\n"
	                         "\n"
	                         "(lock:?GRWLock)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeLock_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&rwlock_exclusive_init,
				TYPE_FIXED_ALLOCATOR(DeeGenericRWLockProxyObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rwlock_exclusive_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&rwlock_exclusive_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&rwlock_exclusive_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rwlock_exclusive_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ &rwlock_exclusive_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rwlock_exclusive_methods,
	/* .tp_getsets       = */ rwlock_exclusive_getsets,
	/* .tp_members       = */ rwlock_exclusive_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};









/************************************************************************/
/* Semaphore                                                            */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1)) int DCALL
semaphore_init(DeeSemaphoreObject *__restrict self,
               size_t argc, DeeObject *const *argv) {
	uintptr_t tickets = 0;
	if (DeeArg_Unpack(argc, argv, "|" UNPuPTR ":Semaphore", &tickets))
		goto err;
	Dee_semaphore_init(&self->sem_semaphore, tickets);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
semaphore_printrepr(DeeSemaphoreObject *__restrict self,
                    dformatprinter printer, void *arg) {
	uintptr_t tickets = atomic_read(&self->sem_semaphore.se_tickets);
	return DeeFormat_Printf(printer, arg, "Semaphore(%" PRFuPTR ")", tickets);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
semaphore_do_release(DeeSemaphoreObject *__restrict self, uintptr_t count) {
	uintptr_t old_count;
	for (;;) {
		uintptr_t new_count;
		old_count = atomic_read(&self->sem_semaphore.se_tickets);
		if (OVERFLOW_UADD(old_count, count, &new_count))
			goto err_overflow;
		if (atomic_cmpxch_weak_explicit(&self->sem_semaphore.se_tickets,
		                                old_count, new_count,
		                                __ATOMIC_SEQ_CST, __ATOMIC_RELAXED))
			break;
	}
	if (Dee_semaphore_haswaiting(&self->sem_semaphore)) {
		if (count == 1) {
			DeeFutex_WakeOne(&self->sem_semaphore.se_tickets);
		} else {
			/* Technically, it'd be enough to only wake `count'
			 * threads, but that can't be done portably... */
			DeeFutex_WakeAll(&self->sem_semaphore.se_tickets);
		}
	}
	return 0;
err_overflow:
	return DeeError_Throwf(&DeeError_IntegerOverflow,
	                       "positive integer overflow after " PP_STR(POINTER_BITS) " bits");
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
semaphore_enter(DeeSemaphoreObject *__restrict self) {
	return Dee_semaphore_acquire(&self->sem_semaphore);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
semaphore_leave(DeeSemaphoreObject *__restrict self) {
	return semaphore_do_release(self, 1);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
semaphore_tryacquire(DeeSemaphoreObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":tryacquire"))
		goto err;
	return_bool(Dee_semaphore_tryacquire(&self->sem_semaphore));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
semaphore_acquire(DeeSemaphoreObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":acquire"))
		goto err;
	if unlikely(Dee_semaphore_acquire(&self->sem_semaphore))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
semaphore_release(DeeSemaphoreObject *self, size_t argc, DeeObject *const *argv) {
	uintptr_t count = 1;
	if (DeeArg_Unpack(argc, argv, "|" UNPuPTR ":release", &count))
		goto err;
	if unlikely(semaphore_do_release(self, count))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
semaphore_timedacquire(DeeSemaphoreObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	uint64_t timeout_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedacquire", &timeout_nanoseconds))
		goto err;
	error = Dee_semaphore_acquire_timed(&self->sem_semaphore, timeout_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
semaphore_waitfor(DeeSemaphoreObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":waitfor"))
		goto err;
	if unlikely(Dee_semaphore_waitfor(&self->sem_semaphore))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
semaphore_timedwaitfor(DeeSemaphoreObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	uint64_t timeout_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedwaitfor", &timeout_nanoseconds))
		goto err;
	error = Dee_semaphore_waitfor_timed(&self->sem_semaphore, timeout_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
semaphore_available_get(DeeSemaphoreObject *__restrict self) {
	return_bool(Dee_semaphore_hastickets(&self->sem_semaphore));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
semaphore_acquired_get(DeeSemaphoreObject *__restrict self) {
	return_bool(!Dee_semaphore_hastickets(&self->sem_semaphore));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
semaphore_tickets_get(DeeSemaphoreObject *__restrict self) {
	uintptr_t result;
	result = Dee_semaphore_gettickets(&self->sem_semaphore);
	return DeeInt_NewUIntptr(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
semaphore_tickets_set(DeeSemaphoreObject *self, DeeObject *value) {
	uintptr_t new_tickets, old_tickets;
	if (DeeObject_AsUIntptr(value, &new_tickets))
		goto err;
	old_tickets = atomic_xch(&self->sem_semaphore.se_tickets, new_tickets);
	if (new_tickets > old_tickets) {
		if (Dee_semaphore_haswaiting(&self->sem_semaphore)) {
			uintptr_t extra = new_tickets - old_tickets;
			if (extra == 1) {
				DeeFutex_WakeOne(&self->sem_semaphore.se_tickets);
			} else {
				/* Technically, it'd be enough to only wake `extra'
				 * threads, but that can't be done portably... */
				DeeFutex_WakeAll(&self->sem_semaphore.se_tickets);
			}
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE struct type_with semaphore_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&semaphore_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&semaphore_leave
};

PRIVATE struct type_method semaphore_methods[] = {
	TYPE_METHOD(STR_tryacquire, &semaphore_tryacquire, DOC_GET(doc_lock_tryacquire)),
	TYPE_METHOD(STR_acquire, &semaphore_acquire, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD(STR_release, &semaphore_release,
	            "(tickets=!1)\n"
	            "@throw IntegerOverflow The new total number of tickets would be too large\n"
	            "Release @tickets tickets to the semaphore"),
	TYPE_METHOD(STR_timedacquire, &semaphore_timedacquire, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD(STR_waitfor, &semaphore_waitfor, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD(STR_timedwaitfor, &semaphore_timedwaitfor, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset semaphore_getsets[] = {
	TYPE_GETTER(STR_available, &semaphore_available_get, DOC_GET(doc_lock_available)),
	TYPE_GETTER(STR_acquired, &semaphore_acquired_get, DOC_GET(doc_lock_acquired)),
	TYPE_GETSET("tickets",
	            &semaphore_tickets_get, NULL, &semaphore_tickets_set,
	            "->?Dint\n"
	            "Get or set the number of available semaphore tickets"),
	TYPE_GETSET_END
};

INTERN DeeTypeObject DeeSemaphore_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Semaphore",
	/* .tp_doc      = */ DOC("(tickets=!0)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeLock_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&semaphore_init,
				TYPE_FIXED_ALLOCATOR(DeeSemaphoreObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&semaphore_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ &semaphore_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ semaphore_methods,
	/* .tp_getsets       = */ semaphore_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};









/************************************************************************/
/* Event                                                            */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1)) int DCALL
event_init_kw(DeeEventObject *__restrict self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	PRIVATE DEFINE_KWLIST(event_kwlist, { K(isset), KEND });
	bool isset = false;
	if (DeeArg_UnpackKw(argc, argv, kw, event_kwlist, "|b:Event", &isset))
		goto err;
	if (isset) {
		Dee_event_init_set(&self->e_event);
	} else {
		Dee_event_init(&self->e_event);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
event_printrepr(DeeEventObject *__restrict self,
                dformatprinter printer, void *arg) {
	bool isset = Dee_event_get(&self->e_event);
	return DeeFormat_Printf(printer, arg,
	                        "Event(isset: %s)",
	                        isset ? "true" : "false");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
event_waitfor(DeeEventObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":waitfor"))
		goto err;
	if unlikely(Dee_event_waitfor(&self->e_event))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
event_timedwaitfor(DeeEventObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	uint64_t timeout_nanoseconds;
	if (DeeArg_Unpack(argc, argv, UNPu64 ":timedwaitfor", &timeout_nanoseconds))
		goto err;
	error = Dee_event_waitfor_timed(&self->e_event, timeout_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
event_set(DeeEventObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":set"))
		goto err;
	Dee_event_set(&self->e_event);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
event_clear(DeeEventObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	Dee_event_clear(&self->e_event);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
event_isset_get(DeeEventObject *__restrict self) {
	return_bool(Dee_event_get(&self->e_event));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
event_isset_set(DeeEventObject *self, DeeObject *value) {
	int val = DeeObject_Bool(value);
	if unlikely(val < 0)
		goto err;
	if (val) {
		Dee_event_set(&self->e_event);
	} else {
		Dee_event_clear(&self->e_event);
	}
	return 0;
err:
	return -1;
}

PRIVATE struct type_method event_methods[] = {
	TYPE_METHOD(STR_waitfor, &event_waitfor,
	            "()\n"
	            "Wait until the event becomes set"),
	TYPE_METHOD(STR_timedwaitfor, &event_timedwaitfor,
	            "(timeout_nanoseconds:?Dint)->?Dbool\n"
	            "Same as ?#waitfor, returning !t on success, but block for at "
	            /**/ "most @timeout_nanoseconds. Once that amount of time has elapsed, "
	            /**/ "stop trying to wait for the event to become set, and fail by "
	            /**/ "returning !f instead."),
	TYPE_METHOD("set", &event_set,
	            "()\n"
	            "Trigger the event as having taken place"),
	TYPE_METHOD("clear", &event_clear,
	            "()\n"
	            "Clear the event as not having taken place"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset event_getsets[] = {
	TYPE_GETSET("isset", &event_isset_get, NULL, &event_isset_set,
	            "->?Dbool\n"
	            "Get or set the has-taken-place status of the event"),
	TYPE_GETSET_END
};

INTERN DeeTypeObject DeeEvent_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Event",
	/* .tp_doc      = */ DOC("(isset=!f)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeEventObject),
				/* .tp_any_ctor_kw = */ (dfunptr_t)&event_init_kw
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&event_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ event_methods,
	/* .tp_getsets       = */ event_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

/* Define generic lock types */
#ifndef __INTELLISENSE__
#define DEFINE_DeeAtomicLock_Type__AND__DeeAtomicRWLock_Type
#include "lock.c.inl"
#define DEFINE_DeeSharedLock_Type__AND__DeeSharedRWLock_Type
#include "lock.c.inl"
#define DEFINE_RDeeAtomicLock_Type__AND__DeeRAtomicRWLock_Type
#include "lock.c.inl"
#define DEFINE_RDeeSharedLock_Type__AND__DeeRSharedRWLock_Type
#include "lock.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEX_THREADING_LOCK_C */
