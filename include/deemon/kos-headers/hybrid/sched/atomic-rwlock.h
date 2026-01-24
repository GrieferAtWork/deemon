/* Copyright (c) 2019-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2019-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
/*!export atomic_rwlock*/
/*!export atomic_rwlock_**/
/*!export ATOMIC_LOLOCK_**/
/*!export _atomic_rwlock**/
#ifndef __GUARD_HYBRID_SCHED_ATOMIC_RWLOCK_H
#define __GUARD_HYBRID_SCHED_ATOMIC_RWLOCK_H 1

#include "../../__stdinc.h"
#include "__atomic-rwlock.h"

#define __SIZEOF_ATOMIC_RWLOCK __SIZEOF_HYBRID_ATOMIC_RWLOCK

#if defined(__CC__) || defined(__DEEMON__)
#define atomic_rwlock                    __hybrid_atomic_rwlock
#define ATOMIC_RWLOCK_MAX_READERS        __HYBRID_ATOMIC_RWLOCK_MAX_READERS
#define ATOMIC_RWLOCK_INIT               __HYBRID_ATOMIC_RWLOCK_INIT
#define ATOMIC_RWLOCK_INIT_READ          __HYBRID_ATOMIC_RWLOCK_INIT_READ
#define ATOMIC_RWLOCK_INIT_WRITE         __HYBRID_ATOMIC_RWLOCK_INIT_WRITE
#define atomic_rwlock_cinit              __hybrid_atomic_rwlock_cinit
#define atomic_rwlock_cinit_read         __hybrid_atomic_rwlock_cinit_read
#define atomic_rwlock_cinit_write        __hybrid_atomic_rwlock_cinit_write
#define atomic_rwlock_init               __hybrid_atomic_rwlock_init
#define atomic_rwlock_init_read          __hybrid_atomic_rwlock_init_read
#define atomic_rwlock_init_write         __hybrid_atomic_rwlock_init_write
#define atomic_rwlock_reading            __hybrid_atomic_rwlock_reading
#define atomic_rwlock_writing            __hybrid_atomic_rwlock_writing
#define atomic_rwlock_canread            __hybrid_atomic_rwlock_canread
#define atomic_rwlock_canwrite           __hybrid_atomic_rwlock_canwrite
#define atomic_rwlock_canupgrade         __hybrid_atomic_rwlock_canupgrade
#define atomic_rwlock_canend             __hybrid_atomic_rwlock_canend
#define atomic_rwlock_canendread         __hybrid_atomic_rwlock_canendread
#define atomic_rwlock_canendwrite        __hybrid_atomic_rwlock_canendwrite
#define atomic_rwlock_tryread            __hybrid_atomic_rwlock_tryread
#define atomic_rwlock_trywrite           __hybrid_atomic_rwlock_trywrite
#define _atomic_rwlock_trywrite_O2       ___hybrid_atomic_rwlock_trywrite_O2
#define _atomic_rwlock_trywrite_Os       ___hybrid_atomic_rwlock_trywrite_Os
#define atomic_rwlock_read               __hybrid_atomic_rwlock_read
#define atomic_rwlock_write              __hybrid_atomic_rwlock_write
#define atomic_rwlock_waitread           __hybrid_atomic_rwlock_waitread
#define atomic_rwlock_waitwrite          __hybrid_atomic_rwlock_waitwrite
#define atomic_rwlock_tryupgrade         __hybrid_atomic_rwlock_tryupgrade
#define _atomic_rwlock_tryupgrade_O2     ___hybrid_atomic_rwlock_tryupgrade_O2
#define _atomic_rwlock_tryupgrade_Os     ___hybrid_atomic_rwlock_tryupgrade_Os
#define atomic_rwlock_upgrade            __hybrid_atomic_rwlock_upgrade
#define atomic_rwlock_downgrade          __hybrid_atomic_rwlock_downgrade
#define _atomic_rwlock_downgrade_NDEBUG  ___hybrid_atomic_rwlock_downgrade_NDEBUG
#define _atomic_rwlock_endread_NDEBUG    ___hybrid_atomic_rwlock_endread_NDEBUG
#define _atomic_rwlock_end_NDEBUG        ___hybrid_atomic_rwlock_end_NDEBUG
#define atomic_rwlock_endread            __hybrid_atomic_rwlock_endread
#define atomic_rwlock_end                __hybrid_atomic_rwlock_end
#define _atomic_rwlock_endread_ex_NDEBUG ___hybrid_atomic_rwlock_endread_ex_NDEBUG
#define _atomic_rwlock_end_ex_NDEBUG     ___hybrid_atomic_rwlock_end_ex_NDEBUG
#define atomic_rwlock_endread_ex         __hybrid_atomic_rwlock_endread_ex
#define atomic_rwlock_end_ex             __hybrid_atomic_rwlock_end_ex
#define _atomic_rwlock_endwrite_NDEBUG   ___hybrid_atomic_rwlock_endwrite_NDEBUG
#define atomic_rwlock_endwrite           __hybrid_atomic_rwlock_endwrite
#if defined(__KERNEL__) && defined(__KOS_VERSION__) && __KOS_VERSION__ >= 400
#define atomic_rwlock_read_nx      __hybrid_atomic_rwlock_read_nx
#define atomic_rwlock_write_nx     __hybrid_atomic_rwlock_write_nx
#define atomic_rwlock_waitread_nx  __hybrid_atomic_rwlock_waitread_nx
#define atomic_rwlock_waitwrite_nx __hybrid_atomic_rwlock_waitwrite_nx
#define atomic_rwlock_upgrade_nx   __hybrid_atomic_rwlock_upgrade_nx
#endif /* __KERNEL__ && __KOS_VERSION__ >= 400 */
#endif /* __CC__ */



#if defined(__ASSEMBLER__) && !defined(__INTELLISENSE__)
#if defined(__x86_64__)

#ifndef LOCK_PREFIX
#define LOCK_PREFIX lock;
#endif /* !LOCK_PREFIX */

/* Clobber: \clobber, %rax
 * @return: true:  ZF=1
 * @return: false: ZF=0 */
.macro atomic_rwlock_trywrite __self, clobber=%rcx, rax_is_zero=0
.if \rax_is_zero == 0
	xorq  %rax, %rax
.endif
	movq  $(-1), \clobber
	LOCK_PREFIX cmpxchgq \clobber, \__self
.endm

/* WARNING: Clobber: \clobber, %rax
 * @return: true:  ZF=1
 * @return: false: ZF=0 */
.macro atomic_rwlock_tryread __self, clobber=%rcx
995:
	movq  \__self, %rax
	cmpq  $(-1), %rax
	jnz   994f
	leaq  1(%rax), \clobber
	LOCK_PREFIX cmpxchgq \clobber, \__self
	jnz   995b
994:
.endm


/* WARNING: Clobber: \clobber, %eax */
.macro atomic_rwlock_write __self, clobber=%rcx, rax_is_zero=0, yield=''
996:
	atomic_rwlock_trywrite \__self, \clobber, \rax_is_zero
.ifc \yield,''
	jnz    996b
.else
	jz     997f
	call   \yield
	jmp    996b
997:
.endif
.endm

/* WARNING: Clobber: \clobber, %rax */
.macro atomic_rwlock_read __self, clobber=%rcx, yield=''
996:
	atomic_rwlock_tryread \__self, \clobber
.ifc \yield,''
	jnz    996b
.else
	jz     997f
	call   \yield
	jmp    996b
997:
.endif
.endm

.macro atomic_rwlock_endread __self
	LOCK_PREFIX decq \__self
.endm
.macro atomic_rwlock_endwrite __self
	movq   $(0), \__self
.endm

#elif defined(__i386__)

#ifndef LOCK_PREFIX
#define LOCK_PREFIX lock;
#endif /* !LOCK_PREFIX */


/* Clobber: \clobber, %eax
 * @return: true:  ZF=1
 * @return: false: ZF=0 */
.macro atomic_rwlock_trywrite __self, clobber=%ecx, eax_is_zero=0
.if \eax_is_zero == 0
	xorl  %eax, %eax
.endif
	movl  $(-1), \clobber
	LOCK_PREFIX cmpxchgl \clobber, \__self
.endm

/* WARNING: Clobber: \clobber, %eax
 * @return: true:  ZF=1
 * @return: false: ZF=0 */
.macro atomic_rwlock_tryread __self, clobber=%ecx
995:
	movl  \__self, %eax
	cmpl  $(-1), %eax
	jnz   994f
	leal  1(%eax), \clobber
	LOCK_PREFIX cmpxchgl \clobber, \__self
	jnz   995b
994:
.endm


/* WARNING: Clobber: \clobber, %eax */
.macro atomic_rwlock_write __self, clobber=%ecx, eax_is_zero=0, yield=''
996:
	atomic_rwlock_trywrite \__self, \clobber, \eax_is_zero
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
.macro atomic_rwlock_read __self, clobber=%ecx, yield=''
996:
	atomic_rwlock_tryread \__self, \clobber
.ifc \yield,''
	jnz    996b
.else
	jz     997f
	call   \yield
	jmp    996b
997:
.endif
.endm

.macro atomic_rwlock_endread __self
	LOCK_PREFIX decl \__self
.endm
.macro atomic_rwlock_endwrite __self
	movl   $(0), \__self
.endm
#endif
#endif /* __ASSEMBLER__ && !__INTELLISENSE__ */

#endif /* !__GUARD_HYBRID_SCHED_ATOMIC_RWLOCK_H */
