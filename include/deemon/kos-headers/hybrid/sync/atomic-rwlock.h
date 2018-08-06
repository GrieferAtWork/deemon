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
#ifndef __GUARD_HYBRID_SYNC_ATOMIC_RWLOCK_H
#define __GUARD_HYBRID_SYNC_ATOMIC_RWLOCK_H 1

#include <hybrid/compiler.h>
#include <stdbool.h>
#include <assert.h>
#include <hybrid/atomic.h>
#include <hybrid/typecore.h>
#include <hybrid/sched/yield.h>
#ifdef __KOS__
#include <hybrid/critical.h>
#endif
#ifdef __ASSEMBLER__
#include <hybrid/asm.h>
#endif

#ifndef __assertf
#define __GUARD_HYBRID_SYNC_DEFINES_ASSERTF 1
#define __assertf(expr,...) assert(expr)
#endif

DECL_BEGIN

#define ATOMIC_RWLOCK_RMASK 0x7fffffff
#define ATOMIC_RWLOCK_WFLAG 0x80000000
#define ATOMIC_RWLOCK_SIZE  __SIZEOF_POINTER__

#ifdef __CC__
typedef union PACKED {
 __UINT32_TYPE__    arw_lock;
 __UINTPTR_TYPE__ __arw_align;
} atomic_rwlock_t;

#define ATOMIC_RWLOCK_INIT              {0}
#define ATOMIC_RWLOCK_INIT_READ         {1}
#define ATOMIC_RWLOCK_INIT_WRITE        {ATOMIC_RWLOCK_WFLAG}
#define atomic_rwlock_cinit(self)       (void)assert((self)->arw_lock == 0)
#define atomic_rwlock_cinit_read(self)  (void)(assert((self)->arw_lock == 0),(self)->arw_lock = 1)
#define atomic_rwlock_cinit_write(self) (void)(assert((self)->arw_lock == 0),(self)->arw_lock = ATOMIC_RWLOCK_WFLAG)
#define atomic_rwlock_init(self)        (void)((self)->arw_lock = 0)
#define atomic_rwlock_init_read(self)   (void)((self)->arw_lock = 1)
#define atomic_rwlock_init_write(self)  (void)((self)->arw_lock = ATOMIC_RWLOCK_WFLAG)
#define DEFINE_ATOMIC_RWLOCK(name)       atomic_rwlock_t name = ATOMIC_RWLOCK_INIT

#define atomic_rwlock_reading(x)   (ATOMIC_READ((x)->arw_lock) != 0)
#define atomic_rwlock_writing(x)   (ATOMIC_READ((x)->arw_lock)&ATOMIC_RWLOCK_WFLAG)

/* Acquire an exclusive read/write lock. */
LOCAL bool KCALL atomic_rwlock_tryread(atomic_rwlock_t *__restrict self);
LOCAL bool KCALL atomic_rwlock_trywrite(atomic_rwlock_t *__restrict self);
LOCAL void KCALL atomic_rwlock_read(atomic_rwlock_t *__restrict self);
LOCAL void KCALL atomic_rwlock_write(atomic_rwlock_t *__restrict self);

/* Try to upgrade a read-lock to a write-lock. Return `FALSE' upon failure. */
LOCAL bool KCALL atomic_rwlock_tryupgrade(atomic_rwlock_t *__restrict self);

/* NOTE: The lock is always upgraded, but when `FALSE' is returned, no lock
 *       may have been held temporarily, meaning that the caller should
 *       re-load local copies of affected resources. */
LOCAL bool KCALL atomic_rwlock_upgrade(atomic_rwlock_t *__restrict self);

/* Downgrade a write-lock to a read-lock (Always succeeds). */
LOCAL void KCALL atomic_rwlock_downgrade(atomic_rwlock_t *__restrict self);

/* End reading/writing/either.
 * @return: true:  The lock has become free.
 * @return: false: The lock is still held by something. */
LOCAL void KCALL atomic_rwlock_endwrite(atomic_rwlock_t *__restrict self);
LOCAL bool KCALL atomic_rwlock_endread(atomic_rwlock_t *__restrict self);
LOCAL bool KCALL atomic_rwlock_end(atomic_rwlock_t *__restrict self);





LOCAL void (KCALL atomic_rwlock_endwrite)(atomic_rwlock_t *__restrict self) {
 COMPILER_BARRIER();
#ifdef NDEBUG
#ifdef __KOS__
 assert(TASK_ISSAFE());
#endif /* __KOS__ */
 ATOMIC_STORE(self->arw_lock,0);
#else
 __UINT32_TYPE__ f;
#ifdef __KOS__
 assert(TASK_ISSAFE());
#endif /* __KOS__ */
 do {
  f = ATOMIC_READ(self->arw_lock);
  __assertf(f&ATOMIC_RWLOCK_WFLAG,"Lock isn't in write-mode (%x)",f);
 } while (!ATOMIC_CMPXCH_WEAK(self->arw_lock,f,0));
#endif
}
LOCAL bool (KCALL atomic_rwlock_endread)(atomic_rwlock_t *__restrict self) {
 COMPILER_READ_BARRIER();
#ifdef NDEBUG
#ifdef __KOS__
 assert(TASK_ISSAFE());
#endif /* __KOS__ */
 return ATOMIC_DECFETCH(self->arw_lock) == 0;
#else
 __UINT32_TYPE__ f;
#ifdef __KOS__
 assert(TASK_ISSAFE());
#endif /* __KOS__ */
 do {
  f = ATOMIC_READ(self->arw_lock);
  __assertf(!(f&ATOMIC_RWLOCK_WFLAG),"Lock is in write-mode (%x)",f);
  __assertf(f != 0,"Lock isn't held by anyone");
 } while (!ATOMIC_CMPXCH_WEAK(self->arw_lock,f,f-1));
 return f == 1;
#endif
}
LOCAL bool (KCALL atomic_rwlock_end)(atomic_rwlock_t *__restrict self) {
 __UINT32_TYPE__ temp,newval;
#ifdef __KOS__
 assert(TASK_ISSAFE());
#endif /* __KOS__ */
 COMPILER_BARRIER();
 do {
  temp = ATOMIC_READ(self->arw_lock);
  if (temp&ATOMIC_RWLOCK_WFLAG) {
   assert(!(temp&ATOMIC_RWLOCK_RMASK));
   newval = 0;
  } else {
   __assertf(temp != 0,"No remaining read-locks");
   newval = temp-1;
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->arw_lock,temp,newval));
 return newval == 0;
}
LOCAL bool (KCALL atomic_rwlock_tryread)(atomic_rwlock_t *__restrict self) {
 __UINT32_TYPE__ temp;
#ifdef __KOS__
 assert(TASK_ISSAFE());
#endif /* __KOS__ */
 do {
  temp = ATOMIC_READ(self->arw_lock);
  if (temp&ATOMIC_RWLOCK_WFLAG) return false;
  assert((temp&ATOMIC_RWLOCK_RMASK) != ATOMIC_RWLOCK_RMASK);
 } while (!ATOMIC_CMPXCH_WEAK(self->arw_lock,temp,temp+1));
 COMPILER_READ_BARRIER();
 return true;
}
LOCAL bool (KCALL atomic_rwlock_trywrite)(atomic_rwlock_t *__restrict self) {
#ifdef __KOS__
 assert(TASK_ISSAFE());
#endif /* __KOS__ */
 if (!ATOMIC_CMPXCH(self->arw_lock,0,ATOMIC_RWLOCK_WFLAG))
      return false;
 COMPILER_BARRIER();
 return true;
}
LOCAL void (KCALL atomic_rwlock_read)(atomic_rwlock_t *__restrict self) {
 while (!atomic_rwlock_tryread(self)) SCHED_YIELD();
 COMPILER_READ_BARRIER();
}
LOCAL void (KCALL atomic_rwlock_write)(atomic_rwlock_t *__restrict self) {
 while (!atomic_rwlock_trywrite(self)) SCHED_YIELD();
 COMPILER_BARRIER();
}
LOCAL bool (KCALL atomic_rwlock_tryupgrade)(atomic_rwlock_t *__restrict self) {
 __UINT32_TYPE__ temp;
#ifdef __KOS__
 assert(TASK_ISSAFE());
#endif /* __KOS__ */
 do {
  temp = ATOMIC_READ(self->arw_lock);
  if (temp != 1) return false;
 } while (!ATOMIC_CMPXCH_WEAK(self->arw_lock,temp,ATOMIC_RWLOCK_WFLAG));
 COMPILER_WRITE_BARRIER();
 return true;
}
LOCAL bool (KCALL atomic_rwlock_upgrade)(atomic_rwlock_t *__restrict self) {
 if (atomic_rwlock_tryupgrade(self)) return true;
 atomic_rwlock_endread(self);
 atomic_rwlock_write(self);
 return false;
}
LOCAL void (KCALL atomic_rwlock_downgrade)(atomic_rwlock_t *__restrict self) {
#ifdef NDEBUG
#ifdef __KOS__
 assert(TASK_ISSAFE());
#endif /* __KOS__ */
 ATOMIC_WRITE(self->arw_lock,1);
#else
 __UINT32_TYPE__ f;
#ifdef __KOS__
 assert(TASK_ISSAFE());
#endif /* __KOS__ */
 COMPILER_WRITE_BARRIER();
 do {
  f = ATOMIC_READ(self->arw_lock);
  __assertf(f == ATOMIC_RWLOCK_WFLAG,"Lock not in write-mode (%x)",f);
 } while (!ATOMIC_CMPXCH_WEAK(self->arw_lock,f,1));
#endif
}
#endif /* __CC__ */

#ifdef __ASSEMBLER__
#if defined(__i386__) || defined(__x86_64__)
/* Clobber: \clobber, %eax
 * @return: true:  ZF=1
 * @return: false: ZF=0 */
.macro atomic_rwlock_trywrite self, clobber=%ecx, eax_is_zero=0
.if \eax_is_zero == 0
    xorl  %eax, %eax
.endif
    movl  $(ATOMIC_RWLOCK_WFLAG), \clobber
    LOCK_PREFIX cmpxchgl \clobber, \self
.endm

/* WARNING: Clobber: \clobber, %eax
 * @return: true:  ZF=1
 * @return: false: ZF=0 */
.macro atomic_rwlock_tryread self, clobber=%ecx
995:
    movl  \self, %eax
    testl $(ATOMIC_RWLOCK_WFLAG), %eax
    jnz   994f
    leal  1(%eax), \clobber
    LOCK_PREFIX cmpxchgl \clobber, \self
    jnz   995b
994:
.endm


/* WARNING: Clobber: \clobber, %eax */
.macro atomic_rwlock_write self, clobber=%ecx, eax_is_zero=0, yield=''
996:
    atomic_rwlock_trywrite \self, \clobber, \eax_is_zero
.ifc \yield,''
    jnz    996b
.else
    jz     997f
    call   \yield
    jmp    996b
997:
.endif
.endm

/* WARNING: Clobber: \clobber, %eax */
.macro atomic_rwlock_read self, clobber=%ecx, yield=''
996:
    atomic_rwlock_tryread \self, \clobber
.ifc \yield,''
    jnz    996b
.else
    jz     997f
    call   \yield
    jmp    996b
997:
.endif
.endm

.macro atomic_rwlock_endread self
    LOCK_PREFIX decl \self
.endm
.macro atomic_rwlock_endwrite self
    movl   $0, \self
.endm
#endif
#endif

#if !defined(__INTELLISENSE__) && !defined(__NO_builtin_expect)
#define atomic_rwlock_tryread(self)    __builtin_expect(atomic_rwlock_tryread(self),true)
#define atomic_rwlock_trywrite(self)   __builtin_expect(atomic_rwlock_trywrite(self),true)
#define atomic_rwlock_tryupgrade(self) __builtin_expect(atomic_rwlock_tryupgrade(self),true)
#define atomic_rwlock_upgrade(self)    __builtin_expect(atomic_rwlock_upgrade(self),true)
#endif

#ifdef __GUARD_HYBRID_SYNC_DEFINES_ASSERTF
#undef __GUARD_HYBRID_SYNC_DEFINES_ASSERTF
#undef __assertf
#endif

DECL_END

#endif /* !__GUARD_HYBRID_SYNC_ATOMIC_RWLOCK_H */
