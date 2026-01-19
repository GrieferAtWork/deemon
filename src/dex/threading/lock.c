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
#ifndef GUARD_DEX_THREADING_LOCK_C
#define GUARD_DEX_THREADING_LOCK_C 1
#define CONFIG_BUILDING_LIBTHREADING
#define DEE_SOURCE

#include "libthreading.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none-operator.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>
#include <deemon/serial.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memset(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/futex.h>
#include <deemon/util/lock.h>
#include <deemon/util/rlock.h>

#include <hybrid/overflow.h>
#include <hybrid/sched/yield.h>
#include <hybrid/typecore.h>

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uint64_t */

#ifdef CONFIG_NO_THREADS
/* Override the lock-API to emulate semantically correct LOCK functions. */
#include "lock-no-threads.c.inl"
#endif /* CONFIG_NO_THREADS */

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

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
	Dee_shared_lock_t l_lock; /* Managed lock */
} DeeSharedLockObject;

typedef struct {
	OBJECT_HEAD
	Dee_shared_rwlock_t rwl_lock; /* Managed lock */
} DeeSharedRWLockObject;

typedef struct {
	OBJECT_HEAD
	Dee_rshared_lock_t l_lock; /* Managed lock */
} DeeRSharedLockObject;

typedef struct {
	OBJECT_HEAD
	Dee_rshared_rwlock_t rwl_lock; /* Managed lock */
} DeeRSharedRWLockObject;

typedef struct {
	OBJECT_HEAD
	Dee_semaphore_t sem_semaphore; /* Managed semaphore */
} DeeSemaphoreObject;

typedef struct {
	OBJECT_HEAD
	Dee_event_t e_event; /* Managed event */
} DeeEventObject;

#ifdef CONFIG_NO_THREADS
#define DeeAtomicLockObject    DeeSharedLockObject
#define DeeAtomicRWLockObject  DeeSharedRWLockObject
#define DeeRAtomicLockObject   DeeRSharedLockObject
#define DeeRAtomicRWLockObject DeeRSharedRWLockObject
#else /* CONFIG_NO_THREADS */
typedef struct {
	OBJECT_HEAD
	Dee_atomic_lock_t l_lock; /* Managed lock */
} DeeAtomicLockObject;

typedef struct {
	OBJECT_HEAD
	Dee_atomic_rwlock_t rwl_lock; /* Managed lock */
} DeeAtomicRWLockObject;

typedef struct {
	OBJECT_HEAD
	Dee_ratomic_lock_t l_lock; /* Managed lock */
} DeeRAtomicLockObject;

typedef struct {
	OBJECT_HEAD
	Dee_ratomic_rwlock_t rwl_lock; /* Managed lock */
} DeeRAtomicRWLockObject;
#endif /* !CONFIG_NO_THREADS */



typedef struct {
	OBJECT_HEAD
	DREF DeeObject *grwl_lock; /* [1..1][const] Pointed-to rwlock object. */
} DeeGenericRWLockProxyObject;

#undef sizeof_field
#define sizeof_field(T, s) sizeof(((T *)0)->s)

/* Assert that shared lock objects are binary-compatible with atomic lock objects. */
#define ASSERT_BINARY_COMPATIBLE_FIELDS(T1, f1, T2, f2)          \
	STATIC_ASSERT(sizeof_field(T1, f1) == sizeof_field(T2, f2)); \
	STATIC_ASSERT(offsetof(T1, f1) == offsetof(T2, f2))
#ifndef CONFIG_NO_THREADS
ASSERT_BINARY_COMPATIBLE_FIELDS(DeeAtomicLockObject, l_lock, DeeSharedLockObject, l_lock.s_lock);
ASSERT_BINARY_COMPATIBLE_FIELDS(DeeAtomicRWLockObject, rwl_lock, DeeSharedRWLockObject, rwl_lock.srw_lock);
ASSERT_BINARY_COMPATIBLE_FIELDS(DeeRAtomicLockObject, l_lock, DeeRSharedLockObject, l_lock.rs_lock);
ASSERT_BINARY_COMPATIBLE_FIELDS(DeeRAtomicRWLockObject, rwl_lock, DeeRSharedRWLockObject, rwl_lock.rsrw_lock);
#endif /* !CONFIG_NO_THREADS */
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

PRIVATE ATTR_COLD int DCALL
err_rwlock_with_readers_and_writers(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Cannot initialize RWLock with both readers and writers");
}

PRIVATE ATTR_COLD int DCALL
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
	"readlock",
	"writelock",
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
PRIVATE DEFINE_STRING_EX(str_readlock, "readlock", 0x683410eb, 0x385a4ec1a5f8ad29);
PRIVATE DEFINE_STRING_EX(str_writelock, "writelock", 0xb24b92d9, 0xc033e9b77132dacf);
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
#define STR_readlock       DeeString_STR(&str_readlock)
#define STR_writelock      DeeString_STR(&str_writelock)
#define STR_tryupgrade     DeeString_STR(&str_tryupgrade)
#define STR_upgrade        DeeString_STR(&str_upgrade)
#define STR_downgrade      DeeString_STR(&str_downgrade)
#define STR_end            DeeString_STR(&str_end)
/*[[[end]]]*/


/* Pre-defined doc strings for lock-like objects */
DOC_DEF(doc_lock_acquire,
        "()\n"
        "#t{:Interrupt}"
        "Acquire @this lock, blocking until that becomes possible");
DOC_DEF(doc_lock_release,
        "()\n"
        "#tValueError{You're not holding this lock}"
        "Release a lock previously acquired by ?#acquire or some other means");
DOC_DEF(doc_lock_tryacquire,
        "->?Dbool\n"
        "Try to acquire @this lock, returning ?t on success, and ?f if doing so would block");
DOC_DEF(doc_lock_timedacquire,
        "(timeout_nanoseconds:?Dint)->?Dbool\n"
        "#t{:Interrupt}"
        "Same as ?#acquire, returning ?t on success, but block for at "
        /**/ "most @timeout_nanoseconds. Once that amount of time has elapsed, "
        /**/ "stop trying to acquire the lock and fail by returning ?f instead.");
DOC_DEF(doc_lock_waitfor,
        "()\n"
        "#t{:Interrupt}"
        "Wait until @this lock can be acquired without blocking\n"
        "Note that by the time this function returns, trying to acquire the "
        /**/ "lock might already block once again.");
DOC_DEF(doc_lock_timedwaitfor,
        "(timeout_nanoseconds:?Dint)->?Dbool\n"
        "#t{:Interrupt}"
        "Same as ?#waitfor, returning ?t on success, but block for at "
        /**/ "most @timeout_nanoseconds. Once that amount of time has elapsed, "
        /**/ "stop trying to wait for the lock to become available, and fail "
        /**/ "by returning ?f instead.");
DOC_DEF(doc_lock_available,
        "->?Dbool\n"
        "Check if @this lock could currently be acquired without blocking\n"
        "Same as ${this.timedwaitfor(0)}");
DOC_DEF(doc_lock_acquired,
        "->?Dbool\n"
        "Check if @this lock is currently being held");


/* Pre-defined doc strings for rwlock-like objects */
DOC_DEF(doc_rwlock_tryread,
        "->?Dbool\n"
        "Try to acquire a shared (read) lock to @this.\n"
        "Same as ${this.readlock.tryacquire()}");
DOC_DEF(doc_rwlock_trywrite,
        "->?Dbool\n"
        "Try to acquire an exclusive (write) lock to @this.\n"
        "Same as ${this.writelock.tryacquire()}");
DOC_DEF(doc_rwlock_read,
        "()\n"
        "#t{:Interrupt}"
        "Acquire a shared (read) lock to @this, blocking until that becomes possible.\n"
        "Same as ${this.readlock.tryacquire()}");
DOC_DEF(doc_rwlock_timedread,
        "(timeout_nanoseconds:?Dint)->?Dbool\n"
        "#t{:Interrupt}"
        "Same as ?#read, returning ?t on success, but block for at "
        /**/ "most @timeout_nanoseconds. Once that amount of time has elapsed, "
        /**/ "stop trying to acquire a shared (read) lock and fail by returning ?f instead.\n"
        "Same as ${this.readlock.timedacquire(timeout_nanoseconds)}");
DOC_DEF(doc_rwlock_write,
        "()\n"
        "#t{:Interrupt}"
        "Acquire an exclusive (write) lock to @this, blocking until that becomes possible.\n"
        "Same as ${this.writelock.tryacquire()}");
DOC_DEF(doc_rwlock_timedwrite,
        "(timeout_nanoseconds:?Dint)->?Dbool\n"
        "#t{:Interrupt}"
        "Same as ?#write, returning ?t on success, but block for at "
        /**/ "most @timeout_nanoseconds. Once that amount of time has elapsed, "
        /**/ "stop trying to acquire an exclusive (write) lock and fail by returning ?f instead.\n"
        "Same as ${this.writelock.timedacquire(timeout_nanoseconds)}");
DOC_DEF(doc_rwlock_waitread,
        "()\n"
        "#t{:Interrupt}"
        "Wait until a shared (read) lock can be acquired without blocking\n"
        "Note that by the time this function returns, trying to acquire a "
        /**/ "shared (read) lock might already block once again.\n"
        "Same as ${this.readlock.waitfor()}");
DOC_DEF(doc_rwlock_timedwaitread,
        "(timeout_nanoseconds:?Dint)->?Dbool\n"
        "#t{:Interrupt}"
        "Same as ?#waitread, returning ?t on success, but block for at "
        /**/ "most @timeout_nanoseconds. Once that amount of time has elapsed, "
        /**/ "stop trying to wait for a shared (read) lock to become available, and "
        /**/ "fail by returning ?f instead.\n"
        "Same as ${this.readlock.timedwaitfor(timeout_nanoseconds)}");
DOC_DEF(doc_rwlock_waitwrite,
        "()\n"
        "#t{:Interrupt}"
        "Wait until an exclusive (write) lock can be acquired without blocking\n"
        "Note that by the time this function returns, trying to acquire an exclusive (write) "
        /**/ "lock might already block once again.\n"
        "Same as ${this.writelock.waitfor()}");
DOC_DEF(doc_rwlock_timedwaitwrite,
        "(timeout_nanoseconds:?Dint)->?Dbool\n"
        "#t{:Interrupt}"
        "Same as ?#waitwrite, returning ?t on success, but block for at "
        /**/ "most @timeout_nanoseconds. Once that amount of time has elapsed, "
        /**/ "stop trying to wait for an exclusive (write) lock to become available, and "
        /**/ "fail by returning ?f instead.\n"
        "Same as ${this.writelock.timedwaitfor(timeout_nanoseconds)}");
DOC_DEF(doc_rwlock_endread,
        "()\n"
        "#tValueError{No shared (read) lock is currently held}"
        "Release a shared (read) lock from @this, previously acquired by ?#read or some other function.\n"
        "Same as ${this.readlock.release()}");
DOC_DEF(doc_rwlock_endwrite,
        "()\n"
        "#tValueError{No exclusive (write) lock is currently held}"
        "Release an exclusive (write) lock from @this, previously acquired by ?#write or some other function.\n"
        "Same as ${this.writelock.release()}");
DOC_DEF(doc_rwlock_end,
        "()\n"
        "#tValueError{No lock of any kind is currently held}"
        "Release either an exclusive (write), or a shared (read) lock from @this");
DOC_DEF(doc_rwlock_tryupgrade,
        "->?DBool\n"
        "Try to upgrade a shared (read) lock (s.a. ?#read) into an exclusive (write) lock (s.a. ?#write)\n"
        "If the lock was upgraded, return ?t. Otherwise, the shared (read) lock is kept, and ?f is returned.");
DOC_DEF(doc_rwlock_upgrade,
        "->?DBool\n"
        "#t{:Interrupt}"
        "#tValueError{Not holding a shared (read) lock at the moment}"
        "#r{true Lock upgrade could be performed atomically (without dropping the shared (read) lock temporarily)}"
        "#r{false Lock upgrade was performed by temporarily dropping the shared (read) lock, before acquiring an exclusive (write) lock}"
        "Upgrade a shared (read) lock into an exclusive (write) lock (if an interrupt error is thrown, the shared (read) lock was released)");
DOC_DEF(doc_rwlock_downgrade,
        "()\n"
        "#tValueError{Not holding an exclusive (write) lock at the moment}"
        "Downgrade an exclusive (write) lock into a shared (read) lock, but maintain some kind of lock at every point in time");
DOC_DEF(doc_rwlock_reading,
        "->?Dbool\n"
        "Returns ?t if either a shared- (read) or an exclusive (write) lock is being held");
DOC_DEF(doc_rwlock_writing,
        "->?Dbool\n"
        "Returns ?t if some thread is holding an exclusive (write) lock");
DOC_DEF(doc_rwlock_canread,
        "->?Dbool\n"
        "Returns ?t so-long as no-one is holding an exclusive (write) lock (inverse of ?#writing)\n"
        "Same as ${this.timedwaitread(0)}");
DOC_DEF(doc_rwlock_canwrite,
        "->?Dbool\n"
        "Returns ?t so-long as no shared- (read) and no exclusive (write) locks are being held (inverse of ?#reading)\n"
        "Same as ${this.timedwaitwrite(0)}");
DOC_DEF(doc_rwlock_readlock,
        "->?GRWLockReadLock\n"
        "Return a ?GLock-compatible wrapper object around @this read/write-lock "
        /**/ "that can be used to acquire a shared (read) locks");
DOC_DEF(doc_rwlock_writelock,
        "->?GRWLockWriteLock\n"
        "Return a ?GLock-compatible wrapper object around @this read/write-lock "
        /**/ "that can be used to acquire an exclusive (write) locks");









/************************************************************************/
/* Abstract Lock API                                                    */
/************************************************************************/
PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_do_acquire(DeeObject *__restrict self) {
	DeeObject *result;
	result = DeeObject_CallAttr(self, Dee_AsObject(&str_acquire), 0, NULL);
	Dee_XDecref_unlikely(result);
	return likely(result) ? 0 : -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_do_release(DeeObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, Dee_AsObject(&str_release), 0, NULL);
	Dee_XDecref_unlikely(result);
	return likely(result) ? 0 : -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_do_tryacquire(DeeObject *__restrict self) {
	DREF DeeObject *result_ob;
	result_ob = DeeObject_CallAttr(self, Dee_AsObject(&str_tryacquire), 0, NULL);
	if unlikely(!result_ob)
		goto err;
	return DeeObject_BoolInherited(result_ob);
err:
	return -1;
}

PRIVATE struct type_with lock_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&lock_do_acquire,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&lock_do_release
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_acquire(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "acquire");
	for (;;) {
		int error = lock_do_tryacquire(self);
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
	uint64_t now_microseconds, then_microseconds;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedacquire", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedacquire", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	error = lock_do_tryacquire(self);
	if unlikely(error < 0)
		goto err;
	if (error)
		goto ok;
	if (args.timeout_nanoseconds == (uint64_t)-1) {
		DREF DeeObject *result;
do_infinite_timeout:
		result = DeeObject_CallAttr(self, Dee_AsObject(&str_acquire), 0, NULL);
		if unlikely(!result)
			goto err;
		Dee_Decref(result);
		goto ok;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, args.timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	error = lock_do_tryacquire(self);
	if unlikely(error < 0)
		goto err;
	if (error)
		goto ok;
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &args.timeout_nanoseconds))
		return_false; /* Timeout */
	args.timeout_nanoseconds *= 1000;
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
	DREF DeeObject *result_ob;
	result_ob = DeeObject_GetAttr(self, Dee_AsObject(&str_acquired));
	if unlikely(!result_ob)
		goto err;
	return DeeObject_BoolInherited(result_ob);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_available_get(DeeObject *__restrict self) {
	int result = lock_is_acquired(self);
	if unlikely(result < 0)
		goto err;
	return_bool(!result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_waitfor(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "waitfor");
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
	uint64_t now_microseconds, then_microseconds;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedwaitfor", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedwaitfor", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	error = lock_is_acquired(self);
	if unlikely(error < 0)
		goto err;
	if (!error)
		goto ok;
	if (args.timeout_nanoseconds == (uint64_t)-1) {
		DREF DeeObject *result;
do_infinite_timeout:
		result = DeeObject_CallAttr(self, Dee_AsObject(&str_waitfor), 0, NULL);
		if unlikely(!result)
			goto err;
		Dee_Decref(result);
		goto ok;
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, args.timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
do_wait_with_timeout:
	error = lock_is_acquired(self);
	if unlikely(error < 0)
		goto err;
	if (!error)
		goto ok;
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &args.timeout_nanoseconds))
		return_false; /* Timeout */
	args.timeout_nanoseconds *= 1000;
	if (DeeThread_CheckInterrupt())
		goto err;
	SCHED_YIELD();
	goto do_wait_with_timeout;
ok:
	return_true;
err:
	return NULL;
}

PRIVATE struct type_method tpconst lock_methods[] = {
	TYPE_METHOD(STR_acquire, &lock_acquire, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD(STR_timedacquire, &lock_timedacquire, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD(STR_waitfor, &lock_waitfor, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD(STR_timedwaitfor, &lock_timedwaitfor, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst lock_getsets[] = {
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ &DeeNone_OperatorCtor,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &DeeNone_OperatorSerialize
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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
PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_tryread(DeeObject *self) {
	DREF DeeObject *result_ob;
	result_ob = DeeObject_CallAttr(self, Dee_AsObject(&str_tryread), 0, NULL);
	if unlikely(!result_ob)
		goto err;
	return DeeObject_BoolInherited(result_ob);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_trywrite(DeeObject *self) {
	DREF DeeObject *result_ob;
	result_ob = DeeObject_CallAttr(self, Dee_AsObject(&str_trywrite), 0, NULL);
	if unlikely(!result_ob)
		goto err;
	return DeeObject_BoolInherited(result_ob);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_tryupgrade(DeeObject *self) {
	DREF DeeObject *result_ob;
	result_ob = DeeObject_CallAttr(self, Dee_AsObject(&str_tryupgrade), 0, NULL);
	if unlikely(!result_ob)
		goto err;
	return DeeObject_BoolInherited(result_ob);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_endread(DeeObject *self) {
	DREF DeeObject *result_ob;
	result_ob = DeeObject_CallAttr(self, Dee_AsObject(&str_endread), 0, NULL);
	if unlikely(!result_ob)
		goto err;
	Dee_Decref(result_ob);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_endwrite(DeeObject *self) {
	DREF DeeObject *result_ob;
	result_ob = DeeObject_CallAttr(self, Dee_AsObject(&str_endwrite), 0, NULL);
	if unlikely(!result_ob)
		goto err;
	Dee_Decref(result_ob);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_reading(DeeObject *self) {
	DREF DeeObject *result_ob;
	result_ob = DeeObject_GetAttr(self, Dee_AsObject(&str_reading));
	if unlikely(!result_ob)
		goto err;
	return DeeObject_BoolInherited(result_ob);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_do_writing(DeeObject *self) {
	DREF DeeObject *result_ob;
	result_ob = DeeObject_GetAttr(self, Dee_AsObject(&str_writing));
	if unlikely(!result_ob)
		goto err;
	return DeeObject_BoolInherited(result_ob);
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
	DeeArg_Unpack0(err, argc, argv, "read");
	if unlikely(rwlock_do_read(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_write(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "write");
	if unlikely(rwlock_do_write(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_end(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
	DeeArg_Unpack0(err, argc, argv, "end");
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
	DeeArg_Unpack0(err, argc, argv, "upgrade");
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
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedread", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedread", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	error = rwlock_do_timedread(self, args.timeout_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_timedwrite(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedwrite", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedwrite", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	error = rwlock_do_timedwrite(self, args.timeout_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_waitread(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "waitread");
	if unlikely(rwlock_do_waitread(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_waitwrite(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "waitwrite");
	if unlikely(rwlock_do_waitwrite(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_timedwaitread(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedwaitread", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedwaitread", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	error = rwlock_do_timedwaitread(self, args.timeout_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_timedwaitwrite(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int error;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedwaitwrite", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedwaitwrite", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	error = rwlock_do_timedwaitwrite(self, args.timeout_nanoseconds);
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
rwlock_readlock_get(DeeObject *__restrict self) {
	DREF DeeGenericRWLockProxyObject *result;
	result = DeeObject_MALLOC(DeeGenericRWLockProxyObject);
	if unlikely(!result)
		goto done;
	result->grwl_lock = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeRWLockReadLock_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeGenericRWLockProxyObject *DCALL
rwlock_writelock_get(DeeObject *__restrict self) {
	DREF DeeGenericRWLockProxyObject *result;
	result = DeeObject_MALLOC(DeeGenericRWLockProxyObject);
	if unlikely(!result)
		goto done;
	result->grwl_lock = self;
	Dee_Incref(self);
	DeeObject_Init(result, &DeeRWLockWriteLock_Type);
done:
	return result;
}

PRIVATE struct type_method tpconst rwlock_methods[] = {
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

PRIVATE struct type_getset tpconst rwlock_getsets[] = {
	TYPE_GETTER(STR_canread, &rwlock_canread_get, DOC_GET(doc_rwlock_canread)),
	TYPE_GETTER(STR_canwrite, &rwlock_canwrite_get, DOC_GET(doc_rwlock_canwrite)),
	TYPE_GETTER(STR_readlock, &rwlock_readlock_get, DOC_GET(doc_rwlock_readlock)),
	TYPE_GETTER(STR_writelock, &rwlock_writelock_get, DOC_GET(doc_rwlock_writelock)),
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
	                         /**/ "property readlock: RWLockReadLock = { get(): RWLockReadLock; };\n"
	                         /**/ "property writelock: RWLockWriteLock = { get(): RWLockWriteLock; };"
	                         "}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ &DeeNone_OperatorCtor,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &DeeNone_OperatorSerialize
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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
rwlock_readlock_init(DeeGenericRWLockProxyObject *__restrict self,
                     size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "RWLockReadLock", &self->grwl_lock);
	if (DeeObject_AssertType(self->grwl_lock, &DeeLock_Type))
		goto err;
	Dee_Incref(self->grwl_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_writelock_init(DeeGenericRWLockProxyObject *__restrict self,
                      size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "RWLockWriteLock", &self->grwl_lock);
	if (DeeObject_AssertType(self->grwl_lock, &DeeLock_Type))
		goto err;
	Dee_Incref(self->grwl_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rwlock_proxy_serialize(DeeGenericRWLockProxyObject *__restrict self,
                       DeeSerial *__restrict writer,
                       Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DeeGenericRWLockProxyObject, field))
	return DeeSerial_PutObject(writer, ADDROF(grwl_lock), self->grwl_lock);
#undef ADDROF
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

PRIVATE struct type_member tpconst rwlock_proxy_members[] = {
	TYPE_MEMBER_FIELD_DOC("rwlock", STRUCT_OBJECT, offsetof(DeeGenericRWLockProxyObject, grwl_lock), "->?GRWLock"),
	TYPE_MEMBER_END
};

#define rwlock_readlock_serialize  rwlock_proxy_serialize
#define rwlock_readlock_fini       rwlock_proxy_fini
#define rwlock_readlock_visit      rwlock_proxy_visit
#define rwlock_readlock_members    rwlock_proxy_members
#define rwlock_writelock_serialize rwlock_proxy_serialize
#define rwlock_writelock_fini      rwlock_proxy_fini
#define rwlock_writelock_visit     rwlock_proxy_visit
#define rwlock_writelock_members   rwlock_proxy_members

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rwlock_readlock_print(DeeGenericRWLockProxyObject *__restrict self,
                      Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<Shared Lock for %k>", self->grwl_lock);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rwlock_writelock_print(DeeGenericRWLockProxyObject *__restrict self,
                       Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<Exclusive Lock for %k>", self->grwl_lock);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rwlock_readlock_printrepr(DeeGenericRWLockProxyObject *__restrict self,
                          Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "%r.readlock", self->grwl_lock);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rwlock_writelock_printrepr(DeeGenericRWLockProxyObject *__restrict self,
                           Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "%r.writelock", self->grwl_lock);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_readlock_enter(DeeGenericRWLockProxyObject *__restrict self) {
	DREF DeeObject *temp;
	temp = DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_read), 0, NULL);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_readlock_leave(DeeGenericRWLockProxyObject *__restrict self) {
	DREF DeeObject *temp;
	temp = DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_endread), 0, NULL);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_writelock_enter(DeeGenericRWLockProxyObject *__restrict self) {
	DREF DeeObject *temp;
	temp = DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_write), 0, NULL);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rwlock_writelock_leave(DeeGenericRWLockProxyObject *__restrict self) {
	DREF DeeObject *temp;
	temp = DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_endwrite), 0, NULL);
	if unlikely(!temp)
		goto err;
	Dee_Decref(temp);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_readlock_tryacquire(DeeGenericRWLockProxyObject *self,
                           size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_tryread), argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_writelock_tryacquire(DeeGenericRWLockProxyObject *self,
                            size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_trywrite), argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_readlock_acquire(DeeGenericRWLockProxyObject *self,
                        size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_read), argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_writelock_acquire(DeeGenericRWLockProxyObject *self,
                         size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_write), argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_readlock_release(DeeGenericRWLockProxyObject *self,
                        size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_endread), argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_writelock_release(DeeGenericRWLockProxyObject *self,
                         size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_endwrite), argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_readlock_timedacquire(DeeGenericRWLockProxyObject *self,
                             size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_timedread), argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_writelock_timedacquire(DeeGenericRWLockProxyObject *self,
                              size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_timedwrite), argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_readlock_waitfor(DeeGenericRWLockProxyObject *self,
                        size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_waitread), argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_writelock_waitfor(DeeGenericRWLockProxyObject *self,
                         size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_waitwrite), argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_readlock_timedwaitfor(DeeGenericRWLockProxyObject *self,
                             size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_timedwaitread), argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_writelock_timedwaitfor(DeeGenericRWLockProxyObject *self,
                              size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self->grwl_lock, Dee_AsObject(&str_timedwaitwrite), argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_readlock_available_get(DeeGenericRWLockProxyObject *__restrict self) {
	return DeeObject_GetAttr(self->grwl_lock, Dee_AsObject(&str_canread));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_writelock_available_get(DeeGenericRWLockProxyObject *__restrict self) {
	return DeeObject_GetAttr(self->grwl_lock, Dee_AsObject(&str_canwrite));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_readlock_acquired_get(DeeGenericRWLockProxyObject *__restrict self) {
	return DeeObject_GetAttr(self->grwl_lock, Dee_AsObject(&str_reading));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rwlock_writelock_acquired_get(DeeGenericRWLockProxyObject *__restrict self) {
	return DeeObject_GetAttr(self->grwl_lock, Dee_AsObject(&str_writing));
}



PRIVATE struct type_with rwlock_readlock_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&rwlock_readlock_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&rwlock_readlock_leave
};

PRIVATE struct type_with rwlock_writelock_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&rwlock_writelock_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&rwlock_writelock_leave
};

PRIVATE struct type_method tpconst rwlock_readlock_methods[] = {
	TYPE_METHOD(STR_tryacquire, &rwlock_readlock_tryacquire, DOC_GET(doc_lock_tryacquire)),
	TYPE_METHOD(STR_acquire, &rwlock_readlock_acquire, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD(STR_release, &rwlock_readlock_release, DOC_GET(doc_lock_release)),
	TYPE_METHOD(STR_timedacquire, &rwlock_readlock_timedacquire, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD(STR_waitfor, &rwlock_readlock_waitfor, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD(STR_timedwaitfor, &rwlock_readlock_timedwaitfor, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};

PRIVATE struct type_method tpconst rwlock_writelock_methods[] = {
	TYPE_METHOD(STR_tryacquire, &rwlock_writelock_tryacquire, DOC_GET(doc_lock_tryacquire)),
	TYPE_METHOD(STR_acquire, &rwlock_writelock_acquire, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD(STR_release, &rwlock_writelock_release, DOC_GET(doc_lock_release)),
	TYPE_METHOD(STR_timedacquire, &rwlock_writelock_timedacquire, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD(STR_waitfor, &rwlock_writelock_waitfor, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD(STR_timedwaitfor, &rwlock_writelock_timedwaitfor, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst rwlock_readlock_getsets[] = {
	TYPE_GETTER(STR_available, &rwlock_readlock_available_get, DOC_GET(doc_lock_available)),
	TYPE_GETTER(STR_acquired, &rwlock_readlock_acquired_get, DOC_GET(doc_lock_acquired)),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst rwlock_writelock_getsets[] = {
	TYPE_GETTER(STR_available, &rwlock_writelock_available_get, DOC_GET(doc_lock_available)),
	TYPE_GETTER(STR_acquired, &rwlock_writelock_acquired_get, DOC_GET(doc_lock_acquired)),
	TYPE_GETSET_END
};

INTERN DeeTypeObject DeeRWLockReadLock_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "RWLockReadLock",
	/* .tp_doc      = */ DOC("Wrapper for a ?GRWLock's shared (read) set of locking functions\n"
	                         "\n"
	                         "(lock:?GRWLock)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeLock_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeGenericRWLockProxyObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &rwlock_readlock_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rwlock_readlock_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rwlock_readlock_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&rwlock_readlock_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&rwlock_readlock_printrepr
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rwlock_readlock_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ &rwlock_readlock_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rwlock_readlock_methods,
	/* .tp_getsets       = */ rwlock_readlock_getsets,
	/* .tp_members       = */ rwlock_readlock_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DeeRWLockWriteLock_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "RWLockWriteLock",
	/* .tp_doc      = */ DOC("Wrapper for a ?GRWLock's exclusive (write) set of locking functions\n"
	                         "\n"
	                         "(lock:?GRWLock)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeLock_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeGenericRWLockProxyObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &rwlock_writelock_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rwlock_writelock_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rwlock_writelock_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&rwlock_writelock_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&rwlock_writelock_printrepr
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rwlock_writelock_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ &rwlock_writelock_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rwlock_writelock_methods,
	/* .tp_getsets       = */ rwlock_writelock_getsets,
	/* .tp_members       = */ rwlock_writelock_members,
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
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("Semaphore", params: """
	size_t tickets = 0;
""", docStringPrefix: "semaphore");]]]*/
#define semaphore_Semaphore_params "tickets=!0"
	struct {
		size_t tickets;
	} args;
	args.tickets = 0;
	DeeArg_Unpack0Or1X(err, argc, argv, "Semaphore", &args.tickets, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	Dee_semaphore_init(&self->sem_semaphore, args.tickets);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
semaphore_serialize(DeeSemaphoreObject *__restrict self,
                    DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DeeSemaphoreObject, field))
	DeeSemaphoreObject *out = DeeSerial_Addr2Mem(writer, addr, DeeSemaphoreObject);
	size_t tickets = Dee_semaphore_gettickets(&self->sem_semaphore);
	Dee_semaphore_init(&out->sem_semaphore, tickets);
	return 0;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
semaphore_printrepr(DeeSemaphoreObject *__restrict self,
                    Dee_formatprinter_t printer, void *arg) {
	size_t tickets = atomic_read(&self->sem_semaphore.se_tickets);
	return DeeFormat_Printf(printer, arg, "Semaphore(%" PRFuPTR ")", tickets);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
semaphore_do_release(DeeSemaphoreObject *__restrict self, size_t count) {
	size_t old_count;
	for (;;) {
		size_t new_count;
		old_count = atomic_read(&self->sem_semaphore.se_tickets);
		if (OVERFLOW_UADD(old_count, count, &new_count))
			goto err_overflow;
		if (atomic_cmpxch_weak_explicit(&self->sem_semaphore.se_tickets,
		                                old_count, new_count,
		                                Dee_ATOMIC_SEQ_CST, Dee_ATOMIC_RELAXED))
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
	DeeArg_Unpack0(err, argc, argv, "tryacquire");
	return_bool(Dee_semaphore_tryacquire(&self->sem_semaphore));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
semaphore_acquire(DeeSemaphoreObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "acquire");
	if unlikely(Dee_semaphore_acquire(&self->sem_semaphore))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
semaphore_release(DeeSemaphoreObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedwrite", params: """
	size_t count = 1;
""");]]]*/
	struct {
		size_t count;
	} args;
	args.count = 1;
	DeeArg_Unpack0Or1X(err, argc, argv, "timedwrite", &args.count, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if unlikely(semaphore_do_release(self, args.count))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
semaphore_timedacquire(DeeSemaphoreObject *self, size_t argc, DeeObject *const *argv) {
	int error;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedacquire", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedacquire", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	error = Dee_semaphore_acquire_timed(&self->sem_semaphore, args.timeout_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
semaphore_waitfor(DeeSemaphoreObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "waitfor");
	if unlikely(Dee_semaphore_waitfor(&self->sem_semaphore))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
semaphore_timedwaitfor(DeeSemaphoreObject *self, size_t argc, DeeObject *const *argv) {
	int error;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedwaitfor", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedwaitfor", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	error = Dee_semaphore_waitfor_timed(&self->sem_semaphore, args.timeout_nanoseconds);
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
	size_t result;
	result = Dee_semaphore_gettickets(&self->sem_semaphore);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
semaphore_tickets_set(DeeSemaphoreObject *self, DeeObject *value) {
	size_t new_tickets, old_tickets;
	if (DeeObject_AsSize(value, &new_tickets))
		goto err;
	old_tickets = atomic_xch(&self->sem_semaphore.se_tickets, new_tickets);
	if (new_tickets > old_tickets) {
		if (Dee_semaphore_haswaiting(&self->sem_semaphore)) {
			size_t extra = new_tickets - old_tickets;
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

PRIVATE struct type_method tpconst semaphore_methods[] = {
	TYPE_METHOD_F(STR_tryacquire, &semaphore_tryacquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_tryacquire)),
	TYPE_METHOD_F(STR_acquire, &semaphore_acquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD_F(STR_release, &semaphore_release, METHOD_FNOREFESCAPE,
	              "(tickets=!1)\n"
	              "#tIntegerOverflow{The new total number of tickets would be too large}"
	              "Release @tickets tickets to the semaphore"),
	TYPE_METHOD_F(STR_timedacquire, &semaphore_timedacquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD_F(STR_waitfor, &semaphore_waitfor, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD_F(STR_timedwaitfor, &semaphore_timedwaitfor, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst semaphore_getsets[] = {
	TYPE_GETTER_F(STR_available, &semaphore_available_get, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_available)),
	TYPE_GETTER_F(STR_acquired, &semaphore_acquired_get, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_acquired)),
	TYPE_GETSET_F("tickets",
	              &semaphore_tickets_get, NULL, &semaphore_tickets_set, METHOD_FNOREFESCAPE,
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeSemaphoreObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &semaphore_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &semaphore_serialize
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&semaphore_printrepr
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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
/* Event                                                                */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
event_serialize(DeeEventObject *__restrict self,
                DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DeeEventObject, field))
	DeeEventObject *out = DeeSerial_Addr2Mem(writer, addr, DeeEventObject);
	bool isset = Dee_event_get(&self->e_event);
	Dee_event_init_ex(&out->e_event, isset);
	return 0;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
event_init_kw(DeeEventObject *__restrict self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("Event", params: "
	bool isset = false;
", docStringPrefix: "event", defineKwList: true);]]]*/
	static DEFINE_KWLIST(Event_kwlist, { KEX("isset", 0xbc9a78a1, 0x5b60f653e62d5e87), KEND });
#define event_Event_params "isset=!f"
	struct {
		bool isset;
	} args;
	args.isset = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, Event_kwlist, "|b:Event", &args))
		goto err;
/*[[[end]]]*/
	if (args.isset) {
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
                Dee_formatprinter_t printer, void *arg) {
	bool isset = Dee_event_get(&self->e_event);
	return DeeFormat_Printf(printer, arg,
	                        "Event(isset: %s)",
	                        isset ? "true" : "false");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
event_waitfor(DeeEventObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "waitfor");
	if unlikely(Dee_event_waitfor(&self->e_event))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
event_timedwaitfor(DeeEventObject *self, size_t argc, DeeObject *const *argv) {
	int error;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedwaitfor", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedwaitfor", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	error = Dee_event_waitfor_timed(&self->e_event, args.timeout_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
event_set(DeeEventObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "set");
	Dee_event_set(&self->e_event);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
event_clear(DeeEventObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "clear");
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

PRIVATE struct type_method tpconst event_methods[] = {
	TYPE_METHOD_F(STR_waitfor, &event_waitfor, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Wait until the event becomes set"),
	TYPE_METHOD_F(STR_timedwaitfor, &event_timedwaitfor, METHOD_FNOREFESCAPE,
	              "(timeout_nanoseconds:?Dint)->?Dbool\n"
	              "Same as ?#waitfor, returning ?t on success, but block for at "
	              /**/ "most @timeout_nanoseconds. Once that amount of time has elapsed, "
	              /**/ "stop trying to wait for the event to become set, and fail by "
	              /**/ "returning ?f instead."),
	TYPE_METHOD_F("set", &event_set, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Trigger the event as having taken place"),
	TYPE_METHOD_F("clear", &event_clear, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Clear the event as not having taken place"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst event_getsets[] = {
	TYPE_GETSET_F("isset", &event_isset_get, NULL, &event_isset_set, METHOD_FNOREFESCAPE,
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeEventObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ &event_init_kw,
			/* tp_serialize:   */ &event_serialize
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&event_printrepr
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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









/************************************************************************/
/* Lock Union                                                           */
/************************************************************************/
typedef struct {
	OBJECT_HEAD
	size_t                                    lu_size;  /* [const][!= 0] # of locks. */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, lu_elem); /* [1..1][const][lu_size] Managed locks. */
} LockUnion;
#define LockUnion_Alloc(size) \
	(LockUnion *)DeeObject_Mallocc(offsetof(LockUnion, lu_elem), size, sizeof(DREF DeeObject *))
#define LockUnion_Realloc(ptr, size) \
	(LockUnion *)DeeObject_Reallocc(ptr, offsetof(LockUnion, lu_elem), size, sizeof(DREF DeeObject *))
#define LockUnion_TryRealloc(ptr, size) \
	(LockUnion *)DeeObject_TryReallocc(ptr, offsetof(LockUnion, lu_elem), size, sizeof(DREF DeeObject *))
#define LockUnion_Free(ptr) DeeObject_Free(ptr)

struct lock_union_allocator {
	size_t     lua_alloc; /* # of allocated lock items */
	LockUnion *lua_union; /* [1..1] Produced lock union */
};

PRIVATE ATTR_COLD int DCALL err_empty_lock_union(void) {
	return DeeError_Throwf(&DeeError_ValueError, "Lock unions cannot be empty");
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_union_allocator_init(struct lock_union_allocator *__restrict self,
                          size_t hint) {
	Dee_ASSERT(hint != 0);
	self->lua_union = LockUnion_Alloc(hint);
	if unlikely(!self->lua_union)
		goto err;
	self->lua_union->lu_size = 0;
	self->lua_alloc           = hint;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
lock_union_allocator_fini(struct lock_union_allocator *__restrict self) {
	Dee_Decrefv_unlikely(self->lua_union->lu_elem,
	                     self->lua_union->lu_size);
	LockUnion_Free(self->lua_union);
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF LockUnion *DCALL
lock_union_allocator_pack(struct lock_union_allocator *__restrict self) {
	DREF LockUnion *result = self->lua_union;
	ASSERT(result->lu_size != 0);
	ASSERT(result->lu_size <= self->lua_alloc);
	if (result->lu_size < self->lua_alloc) {
		result = LockUnion_TryRealloc(result, result->lu_size);
		if unlikely(!result)
			result = self->lua_union;
	}
	DeeObject_Init(result, &DeeLockUnion_Type);
	DBG_memset(self, 0xcc, sizeof(*self));
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
lock_union_allocator_append(struct lock_union_allocator *__restrict self,
                            DeeObject *__restrict lock);
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
lock_union_allocator_append_vector(struct lock_union_allocator *__restrict self,
                                   size_t lock_c, DeeObject *const *lock_v);

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
lock_union_allocator_append(struct lock_union_allocator *__restrict self,
                            DeeObject *__restrict lock) {
	/* Special case for recursive lock unions (which aren't allowed and are inlined) */
	if (DeeObject_InstanceOfExact(lock, &DeeLockUnion_Type)) {
		LockUnion *un = (LockUnion *)lock;
		return lock_union_allocator_append_vector(self, un->lu_size, un->lu_elem);
	}
	ASSERT(self->lua_union->lu_size <= self->lua_alloc);
	if unlikely(self->lua_union->lu_size >= self->lua_alloc) {
		DREF LockUnion *new_union;
		size_t new_alloc = self->lua_alloc * 2;
		ASSERT(new_alloc > self->lua_alloc);
		new_union = LockUnion_TryRealloc(self->lua_union, new_alloc);
		if unlikely(!new_union) {
			new_alloc = self->lua_alloc + 1;
			new_union = LockUnion_Realloc(self->lua_union, new_alloc);
			if unlikely(!new_union)
				goto err;
		}
		self->lua_union = new_union;
		self->lua_alloc = new_alloc;
	}

	/* Append the new lock to the vector used by the union */
	self->lua_union->lu_elem[self->lua_union->lu_size++] = lock;
	Dee_Incref(lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
lock_union_allocator_append_vector(struct lock_union_allocator *__restrict self,
                                   size_t lock_c, DeeObject *const *lock_v) {
	size_t i;
	for (i = 0; i < lock_c; ++i) {
		if unlikely(lock_union_allocator_append(self, lock_v[i]))
			goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((2)) DREF LockUnion *DCALL
LockUnion_FromVector(size_t argc, DeeObject *const *argv) {
	struct lock_union_allocator alloc;
	if unlikely(argc == 0) {
		err_empty_lock_union();
		goto err;
	}
	if unlikely(lock_union_allocator_init(&alloc, argc))
		goto err;
	if unlikely(lock_union_allocator_append_vector(&alloc, argc, argv))
		goto err_alloc;
	return lock_union_allocator_pack(&alloc);
err_alloc:
	lock_union_allocator_fini(&alloc);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((2)) DREF DeeObject *DCALL
libthreading_lockunion_all_f(size_t argc, DeeObject *const *argv) {
	if (argc == 1) /* Special case for when only a single lock was specified */
		return_reference_(argv[0]);
	return Dee_AsObject(LockUnion_FromVector(argc, argv));
}

INTDEF DeeCMethodObject libthreading_lockunion_all;
INTERN DEFINE_CMETHOD(libthreading_lockunion_all, &libthreading_lockunion_all_f, METHOD_FCONSTCALL);

PRIVATE WUNUSED NONNULL((1)) DREF LockUnion *DCALL
LockUnion_FromSequence(DeeObject *__restrict seq) {
	size_t size_hint;
	struct lock_union_allocator alloc;
#ifndef __OPTIMIZE_SIZE__
	if (DeeTuple_Check(seq))
		return LockUnion_FromVector(DeeTuple_SIZE(seq), DeeTuple_ELEM(seq));
#endif /* !__OPTIMIZE_SIZE__ */
	size_hint = DeeObject_SizeFast(seq);
	if (size_hint == (size_t)-1) {
		size_hint = 4;
	} else if unlikely(size_hint == 0) {
		err_empty_lock_union();
		goto err;
	}
	if unlikely(lock_union_allocator_init(&alloc, size_hint))
		goto err;
	if unlikely(DeeObject_Foreach(seq, (Dee_foreach_t)&lock_union_allocator_append, &alloc))
		goto err_alloc;
	if unlikely(alloc.lua_union->lu_size == 0)
		goto err_alloc_empty;
	return lock_union_allocator_pack(&alloc);
err_alloc_empty:
	err_empty_lock_union();
err_alloc:
	lock_union_allocator_fini(&alloc);
err:
	return NULL;
}

PRIVATE WUNUSED DREF LockUnion *DCALL
lock_union_init(size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	DeeArg_Unpack1(err, argc, argv, "LockUnion", &seq);
	return LockUnion_FromSequence(seq);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
lock_union_serialize(LockUnion *__restrict self,
                     DeeSerial *__restrict writer) {
	LockUnion *out;
	size_t sizeof_self = _Dee_MallococBufsize(offsetof(LockUnion, lu_elem),
	                                          self->lu_size,
	                                          sizeof(DREF DeeObject *));
	Dee_seraddr_t out_addr = DeeSerial_ObjectMalloc(writer, sizeof_self, self);
#define ADDROF(field) (out_addr + offsetof(LockUnion, field))
	if unlikely(!Dee_SERADDR_ISOK(out_addr))
		goto err;
	out = DeeSerial_Addr2Mem(writer, out_addr, LockUnion);
	out->lu_size = self->lu_size;
	if (DeeSerial_PutObjectv(writer, ADDROF(lu_elem), self->lu_elem, self->lu_size))
		goto err;
	return out_addr;
err:
	return Dee_SERADDR_INVALID;
#undef ADDROF
}

PRIVATE NONNULL((1)) void DCALL
lock_union_fini(LockUnion *__restrict self) {
	Dee_Decrefv(self->lu_elem, self->lu_size);
}

PRIVATE NONNULL((1, 2)) void DCALL
lock_union_visit(LockUnion *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visitv(self->lu_elem, self->lu_size);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
lock_union_print(LockUnion *__restrict self, Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "<Lock-union for <");
	if unlikely(result < 0)
		goto done;
	for (i = 0; i < self->lu_size; ++i) {
		if (i != 0)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_PrintObject(printer, arg, self->lu_elem[i]));
	}
	DO(err, DeeFormat_PRINT(printer, arg, ">>"));
done:
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
lock_union_printrepr(LockUnion *__restrict self, Dee_formatprinter_t printer, void *arg) {
	size_t i;
	Dee_ssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "LockUnion({ ");
	if unlikely(result < 0)
		goto done;
	for (i = 0; i < self->lu_size; ++i) {
		if (i != 0)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_PrintObjectRepr(printer, arg, self->lu_elem[i]));
	}
	DO(err, DeeFormat_PRINT(printer, arg, " })"));
done:
	return result;
err:
	return temp;
}

PRIVATE NONNULL((1)) void DCALL
lock_do_release_nx(DeeObject *__restrict self) {
	if unlikely(lock_do_release(self))
		DeeError_Print("Failed to release lock after acquire error", ERROR_PRINT_DOHANDLE);
}

PRIVATE NONNULL((1)) void DCALL
lock_union_leave_nx_count(LockUnion *__restrict self, size_t count) {
	ASSERT(count <= self->lu_size);
	while (count) {
		--count;
		lock_do_release_nx(self->lu_elem[count]);
	}
}

PRIVATE NONNULL((1)) int DCALL
lock_union_leave_x1_count(LockUnion *__restrict self, size_t count) {
	ASSERT(count <= self->lu_size);
	while (count) {
		--count;
		if unlikely(lock_do_release(self->lu_elem[count]))
			goto err_count;
	}
	return 0;
err_count:
	lock_union_leave_nx_count(self, count);
	return -1;
}

PRIVATE NONNULL((1)) int DCALL
lock_union_leave_x1_count_except(LockUnion *__restrict self,
                                 size_t count, size_t except_i) {
	ASSERT(count <= self->lu_size);
	while (count) {
		--count;
		if (count != except_i) {
			if unlikely(lock_do_release(self->lu_elem[0]))
				goto err_count;
		}
	}
	return 0;
err_count:
	lock_union_leave_nx_count(self, count);
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_union_do_acquire(LockUnion *__restrict self) {
	size_t i = 1;
	if unlikely(lock_do_acquire(self->lu_elem[0]))
		goto err;
	for (; i < self->lu_size; ++i) {
		int ok = lock_do_tryacquire(self->lu_elem[i]);
		if (ok < 0)
			goto err_release;
		if (ok == 0) {
			size_t already_holding;
			if unlikely(lock_union_leave_x1_count(self, i))
				goto err;
blocking_acquire_lock_i:
			ok = lock_do_acquire(self->lu_elem[i]);
			if unlikely(ok < 0)
				goto err;
			already_holding = i;
			for (i = 0; i < self->lu_size; ++i) {
				if (i == already_holding)
					continue;
				ok = lock_do_tryacquire(self->lu_elem[i]);
				if (ok < 0) {
					lock_union_leave_nx_count(self, i);
					ASSERT(already_holding != i);
					if (already_holding >= i)
						lock_do_release_nx(self->lu_elem[already_holding]);
					goto err;
				}
				if (ok == 0) {
					lock_union_leave_nx_count(self, i);
					ASSERT(already_holding != i);
					if (already_holding >= i)
						lock_do_release_nx(self->lu_elem[already_holding]);

					/* Force at least 1 interrupt-check in here, just in
					 * case none of the lock types we're trying to acquire
					 * perform one, and in case acquiring all locks at once
					 * is impossible due to some lock appearing more than
					 * once (otherwise, it'd be impossible to kill deemon) */
					if (DeeThread_CheckInterrupt())
						goto err;
					goto blocking_acquire_lock_i;
				}
			}
			break;
		}
	}
	return 0;
err_release:
	lock_union_leave_nx_count(self, i);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_union_do_waitfor(LockUnion *__restrict self) {
	size_t i = 0;
	do {
		DREF DeeObject *result;
		result = DeeObject_CallAttr(self->lu_elem[i], Dee_AsObject(&str_waitfor), 0, NULL);
		if unlikely(!result)
			goto err;
		Dee_Decref(result);
	} while (++i < self->lu_size);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
lock_union_do_available_or_acquired(LockUnion *__restrict self,
                                    DeeObject *__restrict attr_name,
                                    size_t start_index) {
	ASSERT(start_index < self->lu_size);
	do {
		int status;
		DREF DeeObject *result;
		result = DeeObject_GetAttr(self->lu_elem[start_index], attr_name);
		if unlikely(!result)
			goto err;
		status = DeeObject_BoolInherited(result);
		if unlikely(status < 0)
			goto err;
		if (!status)
			return 0;
		++start_index;
	} while (start_index < self->lu_size);
	return 1;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_do_timedacquire(DeeObject *__restrict self, uint64_t timeout_nanoseconds) {
	int status;
	DeeObject *result;
	result = DeeObject_CallAttrf(self, Dee_AsObject(&str_timedacquire),
	                             PCKu64, timeout_nanoseconds);
	if unlikely(result == NULL)
		goto err;
	status = DeeObject_BoolInherited(result);
	if unlikely(status >= 0)
		status = status ? 0 : 1;
	return status;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_do_timedwaitfor(DeeObject *__restrict self, uint64_t timeout_nanoseconds) {
	int status;
	DeeObject *result;
	result = DeeObject_CallAttrf(self, Dee_AsObject(&str_timedwaitfor),
	                             PCKu64, timeout_nanoseconds);
	if unlikely(result == NULL)
		goto err;
	status = DeeObject_BoolInherited(result);
	if unlikely(status >= 0)
		status = status ? 0 : 1;
	return status;
err:
	return -1;
}


/* Try to acquire all locks except for the lock at index `already_holding'.
 * @return: 1:  Success
 * @return: 0:  Failure
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_union_tryacquire_except(LockUnion *__restrict self,
                             size_t already_holding) {
	size_t i;
	for (i = 0; i < self->lu_size; ++i) {
		int error;
		if (i == already_holding)
			continue;
		error = lock_do_tryacquire(self->lu_elem[i]);
		if (error > 0)
			continue; /* Success */
		if unlikely(error < 0) {
			lock_union_leave_nx_count(self, i);
			return error;
		}
		/* Acquire failed (release all already-acquired locks) */
		return lock_union_leave_x1_count_except(self, i, already_holding);
	}
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_union_do_acquire_timed(LockUnion *__restrict self,
                            uint64_t timeout_nanoseconds) {
	int ok;
	size_t i;
	uint64_t now_microseconds, then_microseconds;
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		return lock_union_do_acquire(self);
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
	ok = lock_do_timedacquire(self->lu_elem[0], timeout_nanoseconds);
	if (ok != 0)
		return ok; /* Error or timeout */
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds)) {
		/* Timeout */
		if (self->lu_size == 1)
			return 0;
		ok = lock_union_tryacquire_except(self, 0);
		if unlikely(ok < 0) {
			lock_do_release_nx(self->lu_elem[0]);
			goto err;
		}
		if (ok > 0)
			return 0; /* Able to acquire all remaining locks without blocking */
		if unlikely(lock_do_release(self->lu_elem[0]))
			goto err;
		return 1;
	}
	timeout_nanoseconds *= 1000;

	/* First lock has been acquired -> now to acquire the rest of them! */
	for (i = 1; i < self->lu_size; ++i) {
		ok = lock_do_tryacquire(self->lu_elem[i]);
		if (ok < 0)
			goto err_release;
		if (ok == 0) {
			size_t already_holding;
			if unlikely(lock_union_leave_x1_count(self, i))
				goto err;
blocking_acquire_lock_i:
			ok = lock_do_timedacquire(self->lu_elem[i], timeout_nanoseconds);
			if (ok != 0)
				return ok; /* Error or timeout */
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds)) {
				/* Timeout */
				ok = lock_union_tryacquire_except(self, i);
				if unlikely(ok < 0) {
					lock_do_release_nx(self->lu_elem[i]);
					goto err;
				}
				if (ok > 0)
					return 0; /* Able to acquire all remaining locks without blocking */
				if unlikely(lock_do_release(self->lu_elem[i]))
					goto err;
				return 1;
			}
			timeout_nanoseconds *= 1000;
			already_holding = i;
			for (i = 0; i < self->lu_size; ++i) {
				if (i == already_holding)
					continue;
				ok = lock_do_tryacquire(self->lu_elem[i]);
				if (ok < 0) {
					lock_union_leave_nx_count(self, i);
					ASSERT(already_holding != i);
					if (already_holding >= i)
						lock_do_release_nx(self->lu_elem[already_holding]);
					goto err;
				}
				if (ok == 0) {
					lock_union_leave_nx_count(self, i);
					ASSERT(already_holding != i);
					if (already_holding >= i)
						lock_do_release_nx(self->lu_elem[already_holding]);

					/* Force at least 1 interrupt-check in here, just in
					 * case none of the lock types we're trying to acquire
					 * perform one, and in case acquiring all locks at once
					 * is impossible due to some lock appearing more than
					 * once (otherwise, it'd be impossible to kill deemon) */
					if (DeeThread_CheckInterrupt())
						goto err;
					goto blocking_acquire_lock_i;
				}
			}
			break;
		}
	}
	return 0;
err_release:
	lock_union_leave_nx_count(self, i);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_union_do_waitfor_timed(LockUnion *__restrict self,
                            uint64_t timeout_nanoseconds) {
	int ok;
	size_t i;
	uint64_t now_microseconds, then_microseconds;
	if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		return lock_union_do_waitfor(self);
	}
	now_microseconds = DeeThread_GetTimeMicroSeconds();
	if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
		goto do_infinite_timeout;
	for (i = 0; i < self->lu_size; ++i) {
		ok = lock_do_timedwaitfor(self->lu_elem[i], timeout_nanoseconds);
		if (ok != 0)
			return ok; /* Error or timeout */
		now_microseconds = DeeThread_GetTimeMicroSeconds();
		if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds)) {
			/* Timeout */
			ASSERT(i <= self->lu_size - 1);
			if (i >= self->lu_size - 1)
				return 0;
			/* Check if all other locks are currently available. */
			ok = lock_union_do_available_or_acquired(self, Dee_AsObject(&str_available), i + 1);
			if (ok > 0)
				ok = ok ? 0 : 1;
			return ok;
		}
		timeout_nanoseconds *= 1000;
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_union_do_release(LockUnion *__restrict self) {
	size_t i = self->lu_size;
	ASSERT(i != 0);
	do {
		--i;
		if unlikely(lock_do_release(self->lu_elem[i]))
			goto err_i;
	} while (i);
	return 0;
err_i:
	/* Still have to release all other locks, but
	 * discard all exceptions that those might throw. */
	lock_union_leave_nx_count(self, i);
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lock_union_do_tryacquire(LockUnion *__restrict self) {
	size_t i = 0;
	for (; i < self->lu_size; ++i) {
		int ok = lock_do_tryacquire(self->lu_elem[i]);
		if (ok < 0)
			goto err_release;
		if (ok == 0)
			return lock_union_leave_x1_count(self, i);
	}
	return 1;
err_release:
	lock_union_leave_nx_count(self, i);
/*err:*/
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_union_tryacquire(LockUnion *__restrict self,
                      size_t argc, DeeObject *const *argv) {
	int error;
	DeeArg_Unpack0(err, argc, argv, "tryacquire");
	error = lock_union_do_tryacquire(self);
	if unlikely(error < 0)
		goto err;
	return_bool(error != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_union_acquire(LockUnion *__restrict self,
                   size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "acquire");
	if unlikely(lock_union_do_acquire(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_union_release(LockUnion *__restrict self,
                   size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "release");
	if unlikely(lock_union_do_release(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_union_timedacquire(LockUnion *__restrict self,
                        size_t argc, DeeObject *const *argv) {
	int error;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedacquire", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedacquire", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	error = lock_union_do_acquire_timed(self, args.timeout_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_union_waitfor(LockUnion *__restrict self,
                   size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "waitfor");
	if unlikely(lock_union_do_waitfor(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_union_timedwaitfor(LockUnion *__restrict self,
                        size_t argc, DeeObject *const *argv) {
	int error;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedwaitfor", params: """
	uint64_t timeout_nanoseconds;
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedwaitfor", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	error = lock_union_do_waitfor_timed(self, args.timeout_nanoseconds);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_union_available_get(LockUnion *__restrict self) {
	int status = lock_union_do_available_or_acquired(self, Dee_AsObject(&str_available), 0);
	if unlikely(status < 0)
		goto err;
	return_bool(status != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_union_acquired_get(LockUnion *__restrict self) {
	int status = lock_union_do_available_or_acquired(self, Dee_AsObject(&str_acquired), 0);
	if unlikely(status < 0)
		goto err;
	return_bool(status != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
lock_union_locks_get(LockUnion *__restrict self) {
	return DeeRefVector_NewReadonly(Dee_AsObject(self), self->lu_size, self->lu_elem);
}

PRIVATE struct type_with lock_union_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&lock_union_do_acquire,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&lock_union_do_release
};

PRIVATE struct type_method tpconst lock_union_methods[] = {
	TYPE_METHOD_F(STR_tryacquire, &lock_union_tryacquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_tryacquire)),
	TYPE_METHOD_F(STR_acquire, &lock_union_acquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_acquire)),
	TYPE_METHOD_F(STR_release, &lock_union_release, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_release)),
	TYPE_METHOD_F(STR_timedacquire, &lock_union_timedacquire, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_timedacquire)),
	TYPE_METHOD_F(STR_waitfor, &lock_union_waitfor, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_waitfor)),
	TYPE_METHOD_F(STR_timedwaitfor, &lock_union_timedwaitfor, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_timedwaitfor)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst lock_union_getsets[] = {
	TYPE_GETTER_F(STR_available, &lock_union_available_get, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_available)),
	TYPE_GETTER_F(STR_acquired, &lock_union_acquired_get, METHOD_FNOREFESCAPE, DOC_GET(doc_lock_acquired)),
	TYPE_GETTER_F("__locks__", &lock_union_locks_get, METHOD_FNOREFESCAPE,
	              "->?S?GLock\n"
	              "Returns the sequence of locks that are acquired/released by @this ?."),
	TYPE_GETSET_END
};

INTERN DeeTypeObject DeeLockUnion_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "LockUnion",
	/* .tp_doc      = */ DOC("A special type of lock that represents the union of multiple locks. "
	                         /**/ "Using this type, it becomes possible to acquire/release multiple locks all "
	                         /**/ "at the same time, without running the risk of producing a dead-lock as the "
	                         /**/ "result of improper lock ordering. This is achieved by first acquiring the "
	                         /**/ "first lock normally, then using ?Atryacquire?GLock on all other locks. If "
	                         /**/ "during this process some lock cannot be acquired immediately, all locks "
	                         /**/ "that have already been acquired are released, and the lock that could not "
	                         /**/ "be acquired without blocking is now acquired #Bwith bocking. "
	                         /**/ "Once that has been accomplished, all other locks are ?Atryacquire?GLock'd "
	                         /**/ "once again, with the lock that had blocked in the previous iteration now "
	                         /**/ "being treated like the first lock was in the initial iteration. This is "
	                         /**/ "then repeated until all locks could be acquired at the same time.\n"
	                         "By definition, this method of acquiring multiple locks at once prevents dead-locks, "
	                         /**/ "as it never blocks while already holding some lock, which is one of the pre- "
	                         /**/ "conditions of producing a dead-lock (where 2 or more threads are waiting for "
	                         /**/ "a lock that is held by the other thread resp., which is something that can "
	                         /**/ "never happen if threads simply never block while already holding some lock).\n"
	                         "${"
	                         /**/ "@@Acquire multiple locks without ever blocking\n"
	                         /**/ "function acquireAll(locks: {Lock...}) {\n"
	                         /**/ "	local alreadyAcquired = locks.first;\n"
	                         /**/ "again:\n"
	                         /**/ "	alreadyAcquired.acquire();\n"
	                         /**/ "	for (local i = 0; i < #locks; ++i) {\n"
	                         /**/ "		local lock = locks[i];\n"
	                         /**/ "		if (lock === alreadyAcquired)\n"
	                         /**/ "			continue;\n"
	                         /**/ "		if (!lock.tryacquire()) {\n"
	                         /**/ "			/* Release locks already acquired */\n"
	                         /**/ "			while (i > 0) {\n"
	                         /**/ "				--i;\n"
	                         /**/ "				local prevLock = locks[i];\n"
	                         /**/ "				if (prevLock !== alreadyAcquired)\n"
	                         /**/ "					prevLock.release();\n"
	                         /**/ "			}\n"
	                         /**/ "			alreadyAcquired.release();\n"
	                         /**/ "			/* Acquire this lock by blocking */\n"
	                         /**/ "			alreadyAcquired = lock;\n"
	                         /**/ "			goto again;\n"
	                         /**/ "		}\n"
	                         /**/ "	}\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"
	                         "(locks:?S?GLock)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeLock_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &lock_union_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &lock_union_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&lock_union_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&lock_union_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&lock_union_printrepr
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&lock_union_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ &lock_union_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ lock_union_methods,
	/* .tp_getsets       = */ lock_union_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

/* Define generic lock types */
#ifndef __INTELLISENSE__
/* NOTE: Under `CONFIG_NO_THREADS', shared types _must_ come first! */
#define DEFINE_DeeSharedLock_Type__AND__DeeSharedRWLock_Type
#include "lock.c.inl"
#define DEFINE_RDeeSharedLock_Type__AND__DeeRSharedRWLock_Type
#include "lock.c.inl"
/**/

#define DEFINE_DeeAtomicLock_Type__AND__DeeAtomicRWLock_Type
#include "lock.c.inl"
#define DEFINE_RDeeAtomicLock_Type__AND__DeeRAtomicRWLock_Type
#include "lock.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEX_THREADING_LOCK_C */
