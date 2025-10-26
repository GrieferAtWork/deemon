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
#ifndef __GUARD_HYBRID_SCHED___ATOMIC_RWLOCK_H
#define __GUARD_HYBRID_SCHED___ATOMIC_RWLOCK_H 1

#include "../../__stdinc.h"
#include "../typecore.h"
#include "../__atomic.h"
#include "../__assert.h"
#ifndef __INTELLISENSE__
#include "__yield.h"
#endif /* !__INTELLISENSE__ */

#ifdef __KOS_SYSTEM_HEADERS__
#include <kos/anno.h>
#endif /* __KOS_SYSTEM_HEADERS__ */

#ifndef __THROWS
#ifdef __PREPROCESSOR_HAVE_VA_ARGS
#define __THROWS(...)
#elif defined(__PREPROCESSOR_HAVE_NAMED_VA_ARGS)
#define __THROWS(e...)
#else /* ... */
#define __THROWS(e)
#endif /* !... */
#endif /* !__THROWS */

#define __SIZEOF_HYBRID_ATOMIC_RWLOCK __SIZEOF_POINTER__

#if defined(__CC__) || defined(__DEEMON__)
__DECL_BEGIN

struct __hybrid_atomic_rwlock {
	__UINTPTR_TYPE__ arw_lock; /* Lock word. */
};

#define __HYBRID_ATOMIC_RWLOCK_MAX_READERS         ((__UINTPTR_TYPE__)-2)
#define __HYBRID_ATOMIC_RWLOCK_INIT                { 0 }
#define __HYBRID_ATOMIC_RWLOCK_INIT_READ(n)        { n }
#define __HYBRID_ATOMIC_RWLOCK_INIT_WRITE          { (__UINTPTR_TYPE__)-1 }
#define __hybrid_atomic_rwlock_cinit(self)         (void)__hybrid_assert((self)->arw_lock == 0)
#define __hybrid_atomic_rwlock_cinit_read(self, n) (void)(__hybrid_assert((self)->arw_lock == 0), (self)->arw_lock = (n))
#define __hybrid_atomic_rwlock_cinit_write(self)   (void)(__hybrid_assert((self)->arw_lock == 0), (self)->arw_lock = (__UINTPTR_TYPE__)-1)
#define __hybrid_atomic_rwlock_init(self)          (void)((self)->arw_lock = 0)
#define __hybrid_atomic_rwlock_init_read(self, n)  (void)((self)->arw_lock = (n))
#define __hybrid_atomic_rwlock_init_write(self)    (void)((self)->arw_lock = (__UINTPTR_TYPE__)-1)

#define __hybrid_atomic_rwlock_reading(self)     (__hybrid_atomic_load(&(self)->arw_lock, __ATOMIC_ACQUIRE) != 0)
#define __hybrid_atomic_rwlock_writing(self)     (__hybrid_atomic_load(&(self)->arw_lock, __ATOMIC_ACQUIRE) == (__UINTPTR_TYPE__)-1)
#define __hybrid_atomic_rwlock_canread(self)     __likely(__hybrid_atomic_load(&(self)->arw_lock, __ATOMIC_ACQUIRE) != (__UINTPTR_TYPE__)-1)
#define __hybrid_atomic_rwlock_canwrite(self)    __likely(__hybrid_atomic_load(&(self)->arw_lock, __ATOMIC_ACQUIRE) == 0)
#define __hybrid_atomic_rwlock_canupgrade(self)  __likely(__hybrid_atomic_load(&(self)->arw_lock, __ATOMIC_ACQUIRE) == 1)
#define __hybrid_atomic_rwlock_canend(self)      __likely(__hybrid_atomic_load(&(self)->arw_lock, __ATOMIC_ACQUIRE) != 0)
#define __hybrid_atomic_rwlock_canendread(self)  __likely((__UINTPTR_TYPE__)(__hybrid_atomic_load(&(self)->arw_lock, __ATOMIC_ACQUIRE) + 1) >= 2)
#define __hybrid_atomic_rwlock_canendwrite(self) __likely(__hybrid_atomic_load(&(self)->arw_lock, __ATOMIC_ACQUIRE) == (__UINTPTR_TYPE__)-1)

/* Try to acquire a read lock. */
__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1)) __BOOL
__NOTHROW(__hybrid_atomic_rwlock_tryread)(struct __hybrid_atomic_rwlock *__restrict __self);

/* Try to acquire a write lock. */
#ifdef __OPTIMIZE_SIZE__
#define __hybrid_atomic_rwlock_trywrite(self) ___hybrid_atomic_rwlock_trywrite_Os(self)
#else /* __OPTIMIZE_SIZE__ */
#define __hybrid_atomic_rwlock_trywrite(self) ___hybrid_atomic_rwlock_trywrite_O2(self)
#endif /* !__OPTIMIZE_SIZE__ */
#define ___hybrid_atomic_rwlock_trywrite_O2(self) \
	__likely(__hybrid_atomic_rwlock_canwrite(self) && __hybrid_atomic_cmpxch(&(self)->arw_lock, 0, (__UINTPTR_TYPE__)-1, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
#define ___hybrid_atomic_rwlock_trywrite_Os(self) \
	__likely(__hybrid_atomic_cmpxch(&(self)->arw_lock, 0, (__UINTPTR_TYPE__)-1, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))


/* Acquire a read/write lock. */
__LOCAL __ATTR_NONNULL((1)) void (__hybrid_atomic_rwlock_read)(struct __hybrid_atomic_rwlock *__restrict __self) __THROWS(E_WOULDBLOCK_PREEMPTED);
__LOCAL __ATTR_NONNULL((1)) void (__hybrid_atomic_rwlock_write)(struct __hybrid_atomic_rwlock *__restrict __self) __THROWS(E_WOULDBLOCK_PREEMPTED);
__LOCAL __ATTR_NONNULL((1)) void (__hybrid_atomic_rwlock_waitread)(struct __hybrid_atomic_rwlock *__restrict __self) __THROWS(E_WOULDBLOCK_PREEMPTED);
__LOCAL __ATTR_NONNULL((1)) void (__hybrid_atomic_rwlock_waitwrite)(struct __hybrid_atomic_rwlock *__restrict __self) __THROWS(E_WOULDBLOCK_PREEMPTED);
#if defined(__KERNEL__) && defined(__KOS_VERSION__) && __KOS_VERSION__ >= 400
__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1)) __BOOL __NOTHROW(__hybrid_atomic_rwlock_read_nx)(struct __hybrid_atomic_rwlock *__restrict __self);
__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1)) __BOOL __NOTHROW(__hybrid_atomic_rwlock_write_nx)(struct __hybrid_atomic_rwlock *__restrict __self);
__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1)) __BOOL __NOTHROW(__hybrid_atomic_rwlock_waitread_nx)(struct __hybrid_atomic_rwlock *__restrict __self);
__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1)) __BOOL __NOTHROW(__hybrid_atomic_rwlock_waitwrite_nx)(struct __hybrid_atomic_rwlock *__restrict __self);
#endif /* __KERNEL__ && __KOS_VERSION__ >= 400 */

/* Try to upgrade a read-lock to a write-lock. Return `FALSE' upon failure. */
#ifdef __OPTIMIZE_SIZE__
#define __hybrid_atomic_rwlock_tryupgrade(self) ___hybrid_atomic_rwlock_tryupgrade_Os(self)
#else /* __OPTIMIZE_SIZE__ */
#define __hybrid_atomic_rwlock_tryupgrade(self) ___hybrid_atomic_rwlock_tryupgrade_O2(self)
#endif /* !__OPTIMIZE_SIZE__ */
#define ___hybrid_atomic_rwlock_tryupgrade_O2(self) \
	__likely(__hybrid_atomic_rwlock_canupgrade(self) && __hybrid_atomic_cmpxch(&(self)->arw_lock, 1, (__UINTPTR_TYPE__)-1, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED))
#define ___hybrid_atomic_rwlock_tryupgrade_Os(self) \
	__likely(__hybrid_atomic_cmpxch(&(self)->arw_lock, 1, (__UINTPTR_TYPE__)-1, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED))

/* NOTE: The lock is always upgraded, but when `FALSE' is returned, no lock
 *       may  have been  held temporarily,  meaning that  the caller should
 *       re-load local copies of affected resources. */
#define __hybrid_atomic_rwlock_upgrade(self)    \
	(__hybrid_atomic_rwlock_tryupgrade(self) || \
	 (__hybrid_atomic_rwlock_endread(self),     \
	  __hybrid_atomic_rwlock_write(self), 0))

#if defined(__KERNEL__) && defined(__KOS_VERSION__) && __KOS_VERSION__ >= 400
/* NOTE: The lock is always upgraded for `return != 0', but when `2' is returned,
 *       no lock may have been held  temporarily, meaning that the caller  should
 *       re-load local copies of affected resources.
 * NOTE: When `0' is returned, the original read-lock created by the caller has
 *       already been released. */
__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1)) unsigned int
__NOTHROW(__hybrid_atomic_rwlock_upgrade_nx)(struct __hybrid_atomic_rwlock *__restrict __self);
#endif /* __KERNEL__ && __KOS_VERSION__ >= 400 */

/* Downgrade a write-lock to a read-lock (Always succeeds). */
#if defined(NDEBUG) || defined(NDEBUG_SYNC)
#define __hybrid_atomic_rwlock_downgrade(self) ___hybrid_atomic_rwlock_downgrade_NDEBUG(self)
#else /* NDEBUG || NDEBUG_SYNC */
#define __hybrid_atomic_rwlock_downgrade(self)              \
	(__hybrid_assertf(__hybrid_atomic_rwlock_writing(self), \
	                  "Lock isn't in write-mode (%x)",      \
	                  (self)->arw_lock),                    \
	 ___hybrid_atomic_rwlock_downgrade_NDEBUG(self))
#endif /* !NDEBUG || !NDEBUG_SYNC */
#define ___hybrid_atomic_rwlock_downgrade_NDEBUG(self) \
	__hybrid_atomic_store(&(self)->arw_lock, 1, __ATOMIC_RELEASE)

/* End reading or reading/writing. */
#define ___hybrid_atomic_rwlock_endread_NDEBUG(self) \
	__hybrid_atomic_dec(&(self)->arw_lock, __ATOMIC_RELEASE)
#define ___hybrid_atomic_rwlock_end_NDEBUG(self)                \
	(!__hybrid_atomic_rwlock_writing(self)                      \
	 ? __hybrid_atomic_dec(&(self)->arw_lock, __ATOMIC_RELEASE) \
	 : __hybrid_atomic_store(&(self)->arw_lock, 0, __ATOMIC_RELEASE))
#if defined(NDEBUG) || defined(NDEBUG_SYNC)
#define __hybrid_atomic_rwlock_endread(self) ___hybrid_atomic_rwlock_endread_NDEBUG(self)
#define __hybrid_atomic_rwlock_end(self)     ___hybrid_atomic_rwlock_end_NDEBUG(self)
#else /* NDEBUG || NDEBUG_SYNC */
#define __hybrid_atomic_rwlock_endread(self)                                                                  \
	(__hybrid_assertf(!__hybrid_atomic_rwlock_writing(self), "Lock is in write-mode (%x)", (self)->arw_lock), \
	 __hybrid_assertf(__hybrid_atomic_rwlock_reading(self), "Lock isn't held by anyone"),                     \
	 ___hybrid_atomic_rwlock_endread_NDEBUG(self))
#define __hybrid_atomic_rwlock_end(self)                                                   \
	(!__hybrid_atomic_rwlock_writing(self)                                                 \
	 ? (__hybrid_assertf(__hybrid_atomic_rwlock_reading(self), "No remaining read-locks"), \
	    __hybrid_atomic_dec(&(self)->arw_lock, __ATOMIC_RELEASE))                          \
	 : __hybrid_atomic_store(&(self)->arw_lock, 0, __ATOMIC_RELEASE))
#endif /* !NDEBUG || !NDEBUG_SYNC */


/* End reading or reading/writing.
 * @return: true:  The lock may have become free.
 * @return: false: The lock is still held by someone. */
#define ___hybrid_atomic_rwlock_endread_ex_NDEBUG(self) \
	(__hybrid_atomic_decfetch(&(self)->arw_lock, __ATOMIC_RELEASE) == 0)
#define ___hybrid_atomic_rwlock_end_ex_NDEBUG(self)    \
	(!__hybrid_atomic_rwlock_writing(self)             \
	 ? ___hybrid_atomic_rwlock_endread_ex_NDEBUG(self) \
	 : (___hybrid_atomic_rwlock_endwrite_NDEBUG(self), 1))
#if defined(NDEBUG) || defined(NDEBUG_SYNC)
#define __hybrid_atomic_rwlock_endread_ex(self) ___hybrid_atomic_rwlock_endread_ex_NDEBUG(self)
#define __hybrid_atomic_rwlock_end_ex(self)     ___hybrid_atomic_rwlock_end_ex_NDEBUG(self)
#else /* NDEBUG || NDEBUG_SYNC */
#define __hybrid_atomic_rwlock_endread_ex(self)                                                               \
	(__hybrid_assertf(!__hybrid_atomic_rwlock_writing(self), "Lock is in write-mode (%x)", (self)->arw_lock), \
	 __hybrid_assertf(__hybrid_atomic_rwlock_reading(self), "Lock isn't held by anyone"),                     \
	 ___hybrid_atomic_rwlock_endread_ex_NDEBUG(self))
#define __hybrid_atomic_rwlock_end_ex(self)                                                \
	(!__hybrid_atomic_rwlock_writing(self)                                                 \
	 ? (__hybrid_assertf(__hybrid_atomic_rwlock_reading(self), "No remaining read-locks"), \
	    ___hybrid_atomic_rwlock_endread_ex_NDEBUG(self))                                   \
	 : (___hybrid_atomic_rwlock_endwrite_NDEBUG(self), 1))
#endif /* !NDEBUG || !NDEBUG_SYNC */


/* End writing. (returns `void') */
#define ___hybrid_atomic_rwlock_endwrite_NDEBUG(self) \
	__hybrid_atomic_store(&(self)->arw_lock, 0, __ATOMIC_RELEASE)
#if defined(NDEBUG) || defined(NDEBUG_SYNC)
#define __hybrid_atomic_rwlock_endwrite ___hybrid_atomic_rwlock_endwrite_NDEBUG
#else /* NDEBUG || NDEBUG_SYNC */
#define __hybrid_atomic_rwlock_endwrite(self)                   \
	(__hybrid_assertf(__hybrid_atomic_rwlock_writing(self),     \
	                  "Lock isn't in write-mode (%x)", \
	                  (self)->arw_lock),               \
	 ___hybrid_atomic_rwlock_endwrite_NDEBUG(self))
#endif /* !NDEBUG || !NDEBUG_SYNC */


#ifndef __INTELLISENSE__
__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1)) __BOOL
__NOTHROW(__hybrid_atomic_rwlock_tryread)(struct __hybrid_atomic_rwlock *__restrict __self) {
	__UINTPTR_TYPE__ __temp;
	do {
		__temp = __hybrid_atomic_load(&__self->arw_lock, __ATOMIC_ACQUIRE);
		if (__temp == (__UINTPTR_TYPE__)-1)
			return 0;
		__hybrid_assert(__temp != (__UINTPTR_TYPE__)-2);
	} while (!__hybrid_atomic_cmpxch_weak(&__self->arw_lock, __temp, __temp + 1,
	                                      __ATOMIC_ACQUIRE, __ATOMIC_RELAXED));
	return 1;
}

__LOCAL __ATTR_NONNULL((1)) void
(__hybrid_atomic_rwlock_read)(struct __hybrid_atomic_rwlock *__restrict __self)
		__THROWS(E_WOULDBLOCK_PREEMPTED) {
	while (!__hybrid_atomic_rwlock_tryread(__self))
		__hybrid_yield();
}

__LOCAL __ATTR_NONNULL((1)) void
(__hybrid_atomic_rwlock_write)(struct __hybrid_atomic_rwlock *__restrict __self)
		__THROWS(E_WOULDBLOCK_PREEMPTED) {
	while (!__hybrid_atomic_rwlock_trywrite(__self))
		__hybrid_yield();
}

__LOCAL __ATTR_NONNULL((1)) void
(__hybrid_atomic_rwlock_waitread)(struct __hybrid_atomic_rwlock *__restrict __self)
		__THROWS(E_WOULDBLOCK_PREEMPTED) {
	while (!__hybrid_atomic_rwlock_canread(__self))
		__hybrid_yield();
}

__LOCAL __ATTR_NONNULL((1)) void
(__hybrid_atomic_rwlock_waitwrite)(struct __hybrid_atomic_rwlock *__restrict __self)
		__THROWS(E_WOULDBLOCK_PREEMPTED) {
	while (!__hybrid_atomic_rwlock_canwrite(__self))
		__hybrid_yield();
}

#if defined(__KERNEL__) && defined(__KOS_VERSION__) && __KOS_VERSION__ >= 400
__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1)) __BOOL
__NOTHROW(__hybrid_atomic_rwlock_read_nx)(struct __hybrid_atomic_rwlock *__restrict __self) {
	while (!__hybrid_atomic_rwlock_tryread(__self)) {
		if __unlikely(!__hybrid_yield_nx())
			return 0;
	}
	return 1;
}

__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1)) __BOOL
__NOTHROW(__hybrid_atomic_rwlock_write_nx)(struct __hybrid_atomic_rwlock *__restrict __self) {
	while (!__hybrid_atomic_rwlock_trywrite(__self)) {
		if __unlikely(!__hybrid_yield_nx())
			return 0;
	}
	return 1;
}

__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1)) __BOOL
__NOTHROW(__hybrid_atomic_rwlock_waitread_nx)(struct __hybrid_atomic_rwlock *__restrict __self) {
	while (!__hybrid_atomic_rwlock_canread(__self)) {
		if __unlikely(!__hybrid_yield_nx())
			return 0;
	}
	return 1;
}

__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1)) __BOOL
__NOTHROW(__hybrid_atomic_rwlock_waitwrite_nx)(struct __hybrid_atomic_rwlock *__restrict __self) {
	while (!__hybrid_atomic_rwlock_canwrite(__self)) {
		if __unlikely(!__hybrid_yield_nx())
			return 0;
	}
	return 1;
}
#endif /* __KERNEL__ && __KOS_VERSION__ >= 400 */

#if defined(__KERNEL__) && defined(__KOS_VERSION__) && __KOS_VERSION__ >= 400
__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1)) unsigned int
__NOTHROW(__hybrid_atomic_rwlock_upgrade_nx)(struct __hybrid_atomic_rwlock *__restrict __self) {
	if __untraced(__hybrid_atomic_rwlock_tryupgrade(__self))
		return 1;
	__hybrid_atomic_rwlock_endread(__self);
	if __unlikely(!__hybrid_atomic_rwlock_write_nx(__self))
		return 0;
	return 2;
}
#endif /* __KERNEL__ && __KOS_VERSION__ >= 400 */
#endif /* !__INTELLISENSE__ */

/*[[[deemon
for (local S, N: {
	(1, 8),
	(2, 16),
	(4, 32),
	(8, 64),
	(16, 128),
}) {
	local T = f"__UINT{N}_TYPE__";
	if (N >= 64)
		print("#ifdef ", T);
	print("#if __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC >= ", S);
	print("#define __hybrid_atomic_load", N, "_with_atomic_rwlock(p, order, self) __hybrid_atomic_load", N, "(p, order)");
	print("#else /" "* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC >= ", S, " *" "/");
	print("#define __hybrid_atomic_load", N, "_with_atomic_rwlock __hybrid_atomic_load", N, "_with_atomic_rwlock");
	print("__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1, 3)) ", T);
	print("__NOTHROW(__hybrid_atomic_load", N, "_with_atomic_rwlock)(", T, " const *__restrict __p, int __order,");
	print("                                                    struct __hybrid_atomic_rwlock *__restrict __self) {");
	print("	", T, " __result;");
	print("	__hybrid_atomic_rwlock_read(__self);");
	print("	__result = __hybrid_atomic_load", N, "(__p, __order);");
	print("	__hybrid_atomic_rwlock_endread(__self);");
	print("	return __result;");
	print("}");
	print("#endif /" "* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC < ", S, " *" "/");
	if (N >= 64)
		print("#endif /" "* ", T, " *" "/");
}
]]]*/
#if __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC >= 1
#define __hybrid_atomic_load8_with_atomic_rwlock(p, order, self) __hybrid_atomic_load8(p, order)
#else /* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC >= 1 */
#define __hybrid_atomic_load8_with_atomic_rwlock __hybrid_atomic_load8_with_atomic_rwlock
__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1, 3)) __UINT8_TYPE__
__NOTHROW(__hybrid_atomic_load8_with_atomic_rwlock)(__UINT8_TYPE__ const *__restrict __p, int __order,
                                                    struct __hybrid_atomic_rwlock *__restrict __self) {
	__UINT8_TYPE__ __result;
	__hybrid_atomic_rwlock_read(__self);
	__result = __hybrid_atomic_load8(__p, __order);
	__hybrid_atomic_rwlock_endread(__self);
	return __result;
}
#endif /* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC < 1 */
#if __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC >= 2
#define __hybrid_atomic_load16_with_atomic_rwlock(p, order, self) __hybrid_atomic_load16(p, order)
#else /* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC >= 2 */
#define __hybrid_atomic_load16_with_atomic_rwlock __hybrid_atomic_load16_with_atomic_rwlock
__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1, 3)) __UINT16_TYPE__
__NOTHROW(__hybrid_atomic_load16_with_atomic_rwlock)(__UINT16_TYPE__ const *__restrict __p, int __order,
                                                    struct __hybrid_atomic_rwlock *__restrict __self) {
	__UINT16_TYPE__ __result;
	__hybrid_atomic_rwlock_read(__self);
	__result = __hybrid_atomic_load16(__p, __order);
	__hybrid_atomic_rwlock_endread(__self);
	return __result;
}
#endif /* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC < 2 */
#if __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC >= 4
#define __hybrid_atomic_load32_with_atomic_rwlock(p, order, self) __hybrid_atomic_load32(p, order)
#else /* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC >= 4 */
#define __hybrid_atomic_load32_with_atomic_rwlock __hybrid_atomic_load32_with_atomic_rwlock
__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1, 3)) __UINT32_TYPE__
__NOTHROW(__hybrid_atomic_load32_with_atomic_rwlock)(__UINT32_TYPE__ const *__restrict __p, int __order,
                                                    struct __hybrid_atomic_rwlock *__restrict __self) {
	__UINT32_TYPE__ __result;
	__hybrid_atomic_rwlock_read(__self);
	__result = __hybrid_atomic_load32(__p, __order);
	__hybrid_atomic_rwlock_endread(__self);
	return __result;
}
#endif /* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC < 4 */
#ifdef __UINT64_TYPE__
#if __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC >= 8
#define __hybrid_atomic_load64_with_atomic_rwlock(p, order, self) __hybrid_atomic_load64(p, order)
#else /* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC >= 8 */
#define __hybrid_atomic_load64_with_atomic_rwlock __hybrid_atomic_load64_with_atomic_rwlock
__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1, 3)) __UINT64_TYPE__
__NOTHROW(__hybrid_atomic_load64_with_atomic_rwlock)(__UINT64_TYPE__ const *__restrict __p, int __order,
                                                    struct __hybrid_atomic_rwlock *__restrict __self) {
	__UINT64_TYPE__ __result;
	__hybrid_atomic_rwlock_read(__self);
	__result = __hybrid_atomic_load64(__p, __order);
	__hybrid_atomic_rwlock_endread(__self);
	return __result;
}
#endif /* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC < 8 */
#endif /* __UINT64_TYPE__ */
#ifdef __UINT128_TYPE__
#if __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC >= 16
#define __hybrid_atomic_load128_with_atomic_rwlock(p, order, self) __hybrid_atomic_load128(p, order)
#else /* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC >= 16 */
#define __hybrid_atomic_load128_with_atomic_rwlock __hybrid_atomic_load128_with_atomic_rwlock
__LOCAL __ATTR_WUNUSED __ATTR_NONNULL((1, 3)) __UINT128_TYPE__
__NOTHROW(__hybrid_atomic_load128_with_atomic_rwlock)(__UINT128_TYPE__ const *__restrict __p, int __order,
                                                    struct __hybrid_atomic_rwlock *__restrict __self) {
	__UINT128_TYPE__ __result;
	__hybrid_atomic_rwlock_read(__self);
	__result = __hybrid_atomic_load128(__p, __order);
	__hybrid_atomic_rwlock_endread(__self);
	return __result;
}
#endif /* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC < 16 */
#endif /* __UINT128_TYPE__ */
/*[[[end]]]*/

/* Helper for doing  atomic reads of  aligned works, either  through
 * use of CPU-intrinsic atomics, or (if regular writes to such words
 * are  not atomic on  the host CPU, acquire  the mentioned lock and
 * perform a normal reading while said lock is being held. */
#if __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC >= (defined(__UINT128_TYPE__) ? 16 : (defined(__UINT64_TYPE__) ? 8 : 4))
#define __hybrid_atomic_load_with_atomic_rwlock(p, order, rwlock) __hybrid_atomic_load(p, order)
#else /* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC >= ... */
#ifdef __cplusplus
extern "C++" {
#ifndef __HYBRID_PRIVATE_ATOMIC_ENABLE_IF_DEFINED
#define __HYBRID_PRIVATE_ATOMIC_ENABLE_IF_DEFINED
__NAMESPACE_INT_BEGIN
template<bool __V, class __T> struct __hybrid_atomic_enable_if { typedef __T __type; };
template<class __T> struct __hybrid_atomic_enable_if<false, __T> {};
__NAMESPACE_INT_END
#endif /* __HYBRID_PRIVATE_ATOMIC_ENABLE_IF_DEFINED */
#define __hybrid_atomic_load_with_atomic_rwlock __hybrid_atomic_load_with_atomic_rwlock
template<class __T> inline __ATTR_ARTIFICIAL __ATTR_WUNUSED __ATTR_NONNULL((1)) typename __NAMESPACE_INT_SYM __hybrid_atomic_enable_if<false
	|| sizeof(__T) == 1
	|| sizeof(__T) == 2
	|| sizeof(__T) == 4
#ifdef __hybrid_atomic_load64_with_atomic_rwlock
	|| sizeof(__T) == 8
#endif /* __hybrid_atomic_load64_with_atomic_rwlock */
#ifdef __hybrid_atomic_load128_with_atomic_rwlock
	|| sizeof(__T) == 16
#endif /* __hybrid_atomic_load128_with_atomic_rwlock */
, __T>::__type __NOTHROW_NCX(__hybrid_atomic_load_with_atomic_rwlock)(__T const *__p, int __order, struct __hybrid_atomic_rwlock *__restrict __self) {
	(void)__order;
	(void)__self;
	__STATIC_IF(sizeof(__T) == 1) { return (__T)__hybrid_atomic_load8_with_atomic_rwlock((__UINT8_TYPE__ const *)__p, __order, __self); }
	__STATIC_ELSE(sizeof(__T) == 1) {
	__STATIC_IF(sizeof(__T) == 2) { return (__T)__hybrid_atomic_load16_with_atomic_rwlock((__UINT16_TYPE__ const *)__p, __order, __self); }
	__STATIC_ELSE(sizeof(__T) == 2) {
	__STATIC_IF(sizeof(__T) == 4) { return (__T)__hybrid_atomic_load32_with_atomic_rwlock((__UINT32_TYPE__ const *)__p, __order, __self); }
	__STATIC_ELSE(sizeof(__T) == 4) {
#ifdef __hybrid_atomic_load64_with_atomic_rwlock
	__STATIC_IF(sizeof(__T) == 8) { return (__T)__hybrid_atomic_load64_with_atomic_rwlock((__UINT64_TYPE__ const *)__p, __order, __self); }
	__STATIC_ELSE(sizeof(__T) == 8)
#endif /* __hybrid_atomic_load64_with_atomic_rwlock */
	{
#ifdef __hybrid_atomic_load128_with_atomic_rwlock
	__STATIC_IF(sizeof(__T) == 16) { return (__T)__hybrid_atomic_load128_with_atomic_rwlock((__UINT128_TYPE__ const *)__p, __order, __self); }
	__STATIC_ELSE(sizeof(__T) == 16)
#endif /* __hybrid_atomic_load128_with_atomic_rwlock */
	{ __builtin_unreachable(); }}}}}
}
} /* extern "C++" */
#else /* __cplusplus */
#ifndef __HYBRID_ATOMIC_RECAST
#ifdef __COMPILER_HAVE_TYPEOF
#define __HYBRID_ATOMIC_RECAST(p, y) ((__typeof__(*(p)))(y))
#elif 1
#define __HYBRID_ATOMIC_RECAST(p, y) (1 ? (y) : *(p))
#else /* ... */
#define __HYBRID_ATOMIC_RECAST(p, y) (y)
#endif /* !... */
#endif /* !__HYBRID_ATOMIC_RECAST */

#ifdef __hybrid_atomic_load128_with_atomic_rwlock
#ifdef __NO_builtin_choose_expr
#define __hybrid_atomic_load_with_atomic_rwlock(p, order, self) \
	__HYBRID_ATOMIC_RECAST(p, sizeof(*(p)) <= 1 ? __hybrid_atomic_load8_with_atomic_rwlock((__UINT8_TYPE__ const *)(p), order, self) : \
	                          sizeof(*(p)) <= 2 ? __hybrid_atomic_load16_with_atomic_rwlock((__UINT16_TYPE__ const *)(p), order, self) : \
	                          sizeof(*(p)) <= 4 ? __hybrid_atomic_load32_with_atomic_rwlock((__UINT32_TYPE__ const *)(p), order, self) : \
	                          sizeof(*(p)) <= 8 ? __hybrid_atomic_load64_with_atomic_rwlock((__UINT64_TYPE__ const *)(p), order, self) : \
	                                              __hybrid_atomic_load128_with_atomic_rwlock((__UINT128_TYPE__ const *)(p), order, self))
#else /* __NO_builtin_choose_expr */
#define __hybrid_atomic_load_with_atomic_rwlock(p, order, self) \
	__HYBRID_ATOMIC_RECAST(p, __builtin_choose_expr(sizeof(*(p)) <= 1, __hybrid_atomic_load8_with_atomic_rwlock((__UINT8_TYPE__ const *)(p), order, self), \
	                          __builtin_choose_expr(sizeof(*(p)) <= 2, __hybrid_atomic_load16_with_atomic_rwlock((__UINT16_TYPE__ const *)(p), order, self), \
	                          __builtin_choose_expr(sizeof(*(p)) <= 4, __hybrid_atomic_load32_with_atomic_rwlock((__UINT32_TYPE__ const *)(p), order, self), \
	                          __builtin_choose_expr(sizeof(*(p)) <= 8, __hybrid_atomic_load64_with_atomic_rwlock((__UINT64_TYPE__ const *)(p), order, self), \
	                                                                   __hybrid_atomic_load128_with_atomic_rwlock((__UINT128_TYPE__ const *)(p), order, self))))))
#endif /* !__NO_builtin_choose_expr */
#elif defined(__hybrid_atomic_load64_with_atomic_rwlock)
#ifdef __NO_builtin_choose_expr
#define __hybrid_atomic_load_with_atomic_rwlock(p, order, self) \
	__HYBRID_ATOMIC_RECAST(p, sizeof(*(p)) <= 1 ? __hybrid_atomic_load8_with_atomic_rwlock((__UINT8_TYPE__ const *)(p), order, self) : \
	                          sizeof(*(p)) <= 2 ? __hybrid_atomic_load16_with_atomic_rwlock((__UINT16_TYPE__ const *)(p), order, self) : \
	                          sizeof(*(p)) <= 4 ? __hybrid_atomic_load32_with_atomic_rwlock((__UINT32_TYPE__ const *)(p), order, self) : \
	                                              __hybrid_atomic_load64_with_atomic_rwlock((__UINT64_TYPE__ const *)(p), order, self))
#else /* __NO_builtin_choose_expr */
#define __hybrid_atomic_load_with_atomic_rwlock(p, order, self) \
	__HYBRID_ATOMIC_RECAST(p, __builtin_choose_expr(sizeof(*(p)) <= 1, __hybrid_atomic_load8_with_atomic_rwlock((__UINT8_TYPE__ const *)(p), order, self), \
	                          __builtin_choose_expr(sizeof(*(p)) <= 2, __hybrid_atomic_load16_with_atomic_rwlock((__UINT16_TYPE__ const *)(p), order, self), \
	                          __builtin_choose_expr(sizeof(*(p)) <= 4, __hybrid_atomic_load32_with_atomic_rwlock((__UINT32_TYPE__ const *)(p), order, self), \
	                                                                   __hybrid_atomic_load64_with_atomic_rwlock((__UINT64_TYPE__ const *)(p), order, self)))))
#endif /* !__NO_builtin_choose_expr */
#elif defined(__hybrid_atomic_load32_with_atomic_rwlock)
#ifdef __NO_builtin_choose_expr
#define __hybrid_atomic_load_with_atomic_rwlock(p, order, self) \
	__HYBRID_ATOMIC_RECAST(p, sizeof(*(p)) <= 1 ? __hybrid_atomic_load8_with_atomic_rwlock((__UINT8_TYPE__ const *)(p), order, self) : \
	                          sizeof(*(p)) <= 2 ? __hybrid_atomic_load16_with_atomic_rwlock((__UINT16_TYPE__ const *)(p), order, self) : \
	                                              __hybrid_atomic_load32_with_atomic_rwlock((__UINT32_TYPE__ const *)(p), order, self))
#else /* __NO_builtin_choose_expr */
#define __hybrid_atomic_load_with_atomic_rwlock(p, order, self) \
	__HYBRID_ATOMIC_RECAST(p, __builtin_choose_expr(sizeof(*(p)) <= 1, __hybrid_atomic_load8_with_atomic_rwlock((__UINT8_TYPE__ const *)(p), order, self), \
	                          __builtin_choose_expr(sizeof(*(p)) <= 2, __hybrid_atomic_load16_with_atomic_rwlock((__UINT16_TYPE__ const *)(p), order, self), \
	                                                                   __hybrid_atomic_load32_with_atomic_rwlock((__UINT32_TYPE__ const *)(p), order, self))))
#endif /* !__NO_builtin_choose_expr */
#elif defined(__hybrid_atomic_load16_with_atomic_rwlock)
#ifdef __NO_builtin_choose_expr
#define __hybrid_atomic_load_with_atomic_rwlock(p, order, self) \
	__HYBRID_ATOMIC_RECAST(p, sizeof(*(p)) <= 1 ? __hybrid_atomic_load8_with_atomic_rwlock((__UINT8_TYPE__ const *)(p), order, self) : \
	                                              __hybrid_atomic_load16_with_atomic_rwlock((__UINT16_TYPE__ const *)(p), order, self))
#else /* __NO_builtin_choose_expr */
#define __hybrid_atomic_load_with_atomic_rwlock(p, order, self) \
	__HYBRID_ATOMIC_RECAST(p, __builtin_choose_expr(sizeof(*(p)) <= 1, __hybrid_atomic_load8_with_atomic_rwlock((__UINT8_TYPE__ const *)(p), order, self), \
	                                                                   __hybrid_atomic_load16_with_atomic_rwlock((__UINT16_TYPE__ const *)(p), order, self)))
#endif /* !__NO_builtin_choose_expr */
#else /* ... */
#define __hybrid_atomic_load_with_atomic_rwlock(p, order, self) \
	__HYBRID_ATOMIC_RECAST(p, __hybrid_atomic_load8_with_atomic_rwlock((__UINT8_TYPE__ const *)(p), order, self))
#endif /* !... */
#endif /* !__cplusplus */
#endif /* __ARCH_HAVE_ALIGNED_WRITES_ARE_ATOMIC < ... */

__DECL_END
#endif /* __CC__ */

#endif /* !__GUARD_HYBRID_SCHED___ATOMIC_RWLOCK_H */
