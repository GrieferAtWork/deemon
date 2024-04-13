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
#ifndef GUARD_DEEMON_TRACEBACK_H
#define GUARD_DEEMON_TRACEBACK_H 1

#include "api.h"

#include <stdarg.h>
#include <stddef.h>

#include "code.h"
#include "object.h"
#include "util/lock.h"
#include "util/rlock.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_traceback_object traceback_object
#define Dee_thread_object    thread_object
#define Dee_code_frame       code_frame
#endif /* DEE_SOURCE */

typedef struct Dee_traceback_object DeeTracebackObject;
struct Dee_thread_object;
struct Dee_code_frame;

struct Dee_traceback_object {
	Dee_OBJECT_HEAD /* GC object. */
	DREF struct Dee_thread_object                 *tb_thread;     /* [0..1][const] The thread for which this is a traceback. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t                              tb_lock;       /* Lock for accessing this traceback. */
#endif /* !CONFIG_NO_THREADS */
	uint16_t                                       tb_numframes;  /* [const] The amount of allocated frames. */
	uint16_t                                       tb_padding[3]; /* ... */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_code_frame, tb_frames);    /* [0..tb_numframes][lock(tb_lock)]
	                                                               * [OVERRIDE([*].cf_frame, [0..1][owned])] May randomly be NULL if duplication failed.
	                                                               * [OVERRIDE([*].cf_func, DREF [1..1])]
	                                                               * [OVERRIDE([*].cf_argv, DREF [1..1][0..cf_argc][owned])]
	                                                               * [OVERRIDE([*].cf_this, DREF [0..1])]
	                                                               * [OVERRIDE([*].cf_vargs, DREF [0..1])]
	                                                               * [OVERRIDE([*].cf_result, DREF [0..1])]
	                                                               * [OVERRIDE([*].cf_prev, [?..?])]
	                                                               * [OVERRIDE([*].cf_flags, [valid])]
	                                                               * [OVERRIDE([*].cf_sp, [(!= NULL) == (cf_stack != NULL)][== cf_stack+cf_stacksz])] May randomly remain NULL if duplication failed.
	                                                               * [OVERRIDE([*].cf_stack, [(!= NULL) == (cf_stacksz == 0)][0..cf_stacksz][owned])]
	                                                               * [OVERRIDE([*].cf_stacksz, [(!= 0) == (cf_sp != NULL)])] Vector of copied frames.
	                                                               * NOTE: The stack vectors of frames are duplicated as the stack is unwound.
	                                                               *       Frames whose stack has yet to be duplicated have `cf_sp = cf_stack = NULL', `cf_stacksz = 0'. */
};

#define DeeTraceback_LockAvailable(self)  Dee_atomic_lock_available(&(self)->tb_lock)
#define DeeTraceback_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->tb_lock)
#define DeeTraceback_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->tb_lock)
#define DeeTraceback_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->tb_lock)
#define DeeTraceback_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->tb_lock)
#define DeeTraceback_LockRelease(self)    Dee_atomic_lock_release(&(self)->tb_lock)


#ifdef CONFIG_BUILDING_DEEMON
#ifdef GUARD_DEEMON_OBJECTS_TRACEBACK_C
struct empty_traceback_object {
	Dee_OBJECT_HEAD
	DREF struct Dee_thread_object *tb_thread;
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t              tb_lock;
#endif /* !CONFIG_NO_THREADS */
	uint16_t                       tb_numframes;
	uint16_t                       tb_padding[3];
};
INTDEF struct empty_traceback_object DeeTraceback_Empty;
#else /* GUARD_DEEMON_OBJECTS_TRACEBACK_C */
INTDEF DeeTracebackObject DeeTraceback_Empty;
#endif /* !GUARD_DEEMON_OBJECTS_TRACEBACK_C */
#endif /* CONFIG_BUILDING_DEEMON */

DDATDEF DeeTypeObject DeeTraceback_Type;
#define DeeTraceback_Check(ob)      DeeObject_InstanceOfExact(ob, &DeeTraceback_Type) /* `Traceback' is final */
#define DeeTraceback_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeTraceback_Type)


#ifdef CONFIG_BUILDING_DEEMON
/* Fill in stack information in the given traceback for `frame'. */
INTDEF NONNULL((1, 2)) void DCALL
DeeTraceback_AddFrame(DeeTracebackObject *__restrict self,
                      struct Dee_code_frame *__restrict frame,
                      uint16_t frame_id);

/* Try to create a new traceback, but don't throw
 * an error and return `NULL' if doing so failed.
 * NOTE: The given `thread' must be the caller's. */
INTDEF WUNUSED NONNULL((1)) DREF DeeTracebackObject *DCALL
DeeTraceback_New(struct Dee_thread_object *__restrict thread);

/* Same as `DeeTraceback_New()', but throw errors when returning NULL. */
INTDEF WUNUSED NONNULL((1)) DREF DeeTracebackObject *DCALL
DeeTraceback_NewWithException(struct Dee_thread_object *__restrict thread);
#endif /* CONFIG_BUILDING_DEEMON */


typedef struct frame_object DeeFrameObject;
struct frame_object {
	Dee_OBJECT_HEAD /* More of a frame-reference object. */
	DREF DeeObject        *f_owner; /* [0..1][const] Owner of the frame (Required to prevent the frame from being destroyed). */
	struct Dee_code_frame *f_frame; /* [lock(*f_palock)][0..1][lock(f_lock)] The actual frame that is being referenced. */
#ifndef CONFIG_NO_THREADS
	union { /* Lock that must be acquired when accessing the frame. */
		Dee_atomic_rwlock_t  *f_palock;  /* [0..1][valid_if(!DEEFRAME_FRECLOCK && !DEEFRAME_FSHRLOCK)][const] */
		Dee_ratomic_rwlock_t *f_pralock; /* [0..1][valid_if(DEEFRAME_FRECLOCK && !DEEFRAME_FSHRLOCK)][const] */
		Dee_shared_rwlock_t  *f_pslock;  /* [0..1][valid_if(!DEEFRAME_FRECLOCK && DEEFRAME_FSHRLOCK)][const] */
		Dee_rshared_rwlock_t *f_prslock; /* [0..1][valid_if(DEEFRAME_FRECLOCK && DEEFRAME_FSHRLOCK)][const] */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define f_palock   _dee_aunion.f_palock
#define f_pralock  _dee_aunion.f_pralock
#define f_pslock   _dee_aunion.f_pslock
#define f_prslock  _dee_aunion.f_prslock
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	Dee_atomic_rwlock_t f_lock;  /* Lock for accessing fields of this frame object. */
#endif /* !CONFIG_NO_THREADS */
#define DEEFRAME_FNORMAL   0x0000 /* Normal frame flags. */
#define DEEFRAME_FREADONLY 0x0000 /* [const] Contents of the frame may not be modified. */
#define DEEFRAME_FNORESULT 0x0010 /* [const] Ignore the value of `cf_result' -- the frame has no return value. */
#define DEEFRAME_FWRITABLE 0x1000 /* [lock(READ(f_lock) && CLEAR_ONCE)] Contents of the frame may be modified. */
#define DEEFRAME_FUNDEFSP  0x2000 /* [lock(READ(f_lock) && ATOMIC && CLEAR_ONCE)] The stack-pointer of the frame is undefined.
                                   * When `DEEFRAME_FUNDEFSP2' isn't set, the correct stack pointer may be
                                   * obtainable from DDI information and the current PC.
                                   * This flag may NOT be set when `DEEFRAME_FWRITABLE' is set. */
#define DEEFRAME_FUNDEFSP2 0x4000 /* [lock(READ(f_lock) && ATOMIC && WRITE_ONCE)] The stack-pointer of the frame could not be determined. */
#define DEEFRAME_FREGENGSP 0x8000 /* [lock(READ(f_lock) && ATOMIC && WRITE_ONCE)] The SP pointer was reverse engineered and stored in `f_revsp' */
#ifndef CONFIG_NO_THREADS
#define DEEFRAME_FSHRLOCK  0x0001 /* The frame uses a shared lock. */
#define DEEFRAME_FRECLOCK  0x0002 /* The frame uses a recursive lock. */
#else /* !CONFIG_NO_THREADS */
#define DEEFRAME_FSHRLOCK  0x0000 /* Ignored. */
#define DEEFRAME_FRECLOCK  0x0000 /* Ignored. */
#endif /* CONFIG_NO_THREADS */
	uint16_t           f_flags; /* [const] Contents of the frame may be modified. */
	uint16_t           f_revsp; /* [lock(f_lock)][valid_if(DEEFRAME_FREGENGSP)] Reverse engineered SP. */
};

#ifdef CONFIG_NO_THREADS
#define _DeeFrame_PLockOp(self,                                        \
                          Dee_atomic_rwlock_op, Dee_ratomic_rwlock_op, \
                          Dee_shared_rwlock_op, Dee_rshared_rwlock_op) \
	Dee_atomic_rwlock_op(~)
#define _DeeFrame_PLockOp2(self, atomic_retval,                         \
                           Dee_atomic_rwlock_op, Dee_ratomic_rwlock_op, \
                           Dee_shared_rwlock_op, Dee_rshared_rwlock_op) \
	atomic_retval
#else /* CONFIG_NO_THREADS */
#define _DeeFrame_PLockOp(self,                                        \
                          Dee_atomic_rwlock_op, Dee_ratomic_rwlock_op, \
                          Dee_shared_rwlock_op, Dee_rshared_rwlock_op) \
	((self)->f_flags & DEEFRAME_FSHRLOCK                               \
	 ? ((self)->f_flags & DEEFRAME_FRECLOCK                            \
	    ? Dee_rshared_rwlock_op((self)->f_prslock)                     \
	    : Dee_shared_rwlock_op((self)->f_pslock))                      \
	 : ((self)->f_flags & DEEFRAME_FRECLOCK                            \
	    ? Dee_ratomic_rwlock_op((self)->f_pralock)                     \
	    : Dee_atomic_rwlock_op((self)->f_palock)))
#define _DeeFrame_PLockOp2(self, atomic_retval,                         \
                           Dee_atomic_rwlock_op, Dee_ratomic_rwlock_op, \
                           Dee_shared_rwlock_op, Dee_rshared_rwlock_op) \
	((self)->f_flags & DEEFRAME_FSHRLOCK                                \
	 ? ((self)->f_flags & DEEFRAME_FRECLOCK                             \
	    ? Dee_rshared_rwlock_op((self)->f_prslock)                      \
	    : Dee_shared_rwlock_op((self)->f_pslock))                       \
	 : ((self)->f_flags & DEEFRAME_FRECLOCK                             \
	    ? Dee_ratomic_rwlock_op((self)->f_pralock)                      \
	    : Dee_atomic_rwlock_op((self)->f_palock),                       \
	    atomic_retval))
#endif /* !CONFIG_NO_THREADS */

#define DeeFrame_CanWrite(self) ((self)->f_flags & DEEFRAME_FWRITABLE)

#define _DeeFrame_PLockPresent(self)        ((self)->f_palock != NULL)
#define _DeeFrame_PLockReading(self)        _DeeFrame_PLockOp(self, Dee_atomic_rwlock_reading, Dee_ratomic_rwlock_reading, Dee_shared_rwlock_reading, Dee_rshared_rwlock_reading)
#define _DeeFrame_PLockWriting(self)        _DeeFrame_PLockOp(self, Dee_atomic_rwlock_writing, Dee_ratomic_rwlock_writing, Dee_shared_rwlock_writing, Dee_rshared_rwlock_writing)
#define _DeeFrame_PLockTryRead(self)        _DeeFrame_PLockOp(self, Dee_atomic_rwlock_tryread, Dee_ratomic_rwlock_tryread, Dee_shared_rwlock_tryread, Dee_rshared_rwlock_tryread)
#define _DeeFrame_PLockTryWrite(self)       _DeeFrame_PLockOp(self, Dee_atomic_rwlock_trywrite, Dee_ratomic_rwlock_trywrite, Dee_shared_rwlock_trywrite, Dee_rshared_rwlock_trywrite)
#define _DeeFrame_PLockCanRead(self)        _DeeFrame_PLockOp(self, Dee_atomic_rwlock_canread, Dee_ratomic_rwlock_canread, Dee_shared_rwlock_canread, Dee_rshared_rwlock_canread)
#define _DeeFrame_PLockCanWrite(self)       _DeeFrame_PLockOp(self, Dee_atomic_rwlock_canwrite, Dee_ratomic_rwlock_canwrite, Dee_shared_rwlock_canwrite, Dee_rshared_rwlock_canwrite)
#define _DeeFrame_PLockWaitRead(self)       _DeeFrame_PLockOp2(self, 0, Dee_atomic_rwlock_waitread, Dee_ratomic_rwlock_waitread, Dee_shared_rwlock_waitread, Dee_rshared_rwlock_waitread)
#define _DeeFrame_PLockWaitReadNoInt(self)  _DeeFrame_PLockOp2(self, 0, Dee_atomic_rwlock_waitread, Dee_ratomic_rwlock_waitread, Dee_shared_rwlock_waitread_noint, Dee_rshared_rwlock_waitread_noint)
#define _DeeFrame_PLockWaitWrite(self)      _DeeFrame_PLockOp2(self, 0, Dee_atomic_rwlock_waitwrite, Dee_ratomic_rwlock_waitwrite, Dee_shared_rwlock_waitwrite, Dee_rshared_rwlock_waitwrite)
#define _DeeFrame_PLockWaitWriteNoInt(self) _DeeFrame_PLockOp2(self, 0, Dee_atomic_rwlock_waitwrite, Dee_ratomic_rwlock_waitwrite, Dee_shared_rwlock_waitwrite_noint, Dee_rshared_rwlock_waitwrite_noint)
#define _DeeFrame_PLockRead(self)           _DeeFrame_PLockOp2(self, 0, Dee_atomic_rwlock_read, Dee_ratomic_rwlock_read, Dee_shared_rwlock_read, Dee_rshared_rwlock_read)
#define _DeeFrame_PLockReadNoInt(self)      _DeeFrame_PLockOp2(self, 0, Dee_atomic_rwlock_read, Dee_ratomic_rwlock_read, Dee_shared_rwlock_read_noint, Dee_rshared_rwlock_read_noint)
#define _DeeFrame_PLockWrite(self)          _DeeFrame_PLockOp2(self, 0, Dee_atomic_rwlock_write, Dee_ratomic_rwlock_write, Dee_shared_rwlock_write, Dee_rshared_rwlock_write)
#define _DeeFrame_PLockWriteNoInt(self)     _DeeFrame_PLockOp2(self, 0, Dee_atomic_rwlock_write, Dee_ratomic_rwlock_write, Dee_shared_rwlock_write_noint, Dee_rshared_rwlock_write_noint)
#define _DeeFrame_PLockTryUpgrade(self)     _DeeFrame_PLockOp(self, Dee_atomic_rwlock_tryupgrade, Dee_ratomic_rwlock_tryupgrade, Dee_shared_rwlock_tryupgrade, Dee_rshared_rwlock_tryupgrade)
#define _DeeFrame_PLockUpgrade(self)        _DeeFrame_PLockOp(self, Dee_atomic_rwlock_upgrade, Dee_ratomic_rwlock_upgrade, Dee_shared_rwlock_upgrade, Dee_rshared_rwlock_upgrade)
#define _DeeFrame_PLockUpgradeNoInt(self)   _DeeFrame_PLockOp(self, Dee_atomic_rwlock_upgrade, Dee_ratomic_rwlock_upgrade, Dee_shared_rwlock_upgrade_noint, Dee_rshared_rwlock_upgrade_noint)
#define _DeeFrame_PLockDowngrade(self)      _DeeFrame_PLockOp(self, Dee_atomic_rwlock_downgrade, Dee_ratomic_rwlock_downgrade, Dee_shared_rwlock_downgrade, Dee_rshared_rwlock_downgrade)
#define _DeeFrame_PLockEndWrite(self)       _DeeFrame_PLockOp(self, Dee_atomic_rwlock_endwrite, Dee_ratomic_rwlock_endwrite, Dee_shared_rwlock_endwrite, Dee_rshared_rwlock_endwrite)
#define _DeeFrame_PLockEndRead(self)        _DeeFrame_PLockOp(self, Dee_atomic_rwlock_endread, Dee_ratomic_rwlock_endread, Dee_shared_rwlock_endread, Dee_rshared_rwlock_endread)
#define _DeeFrame_PLockEnd(self)            _DeeFrame_PLockOp(self, Dee_atomic_rwlock_end, Dee_ratomic_rwlock_end, Dee_shared_rwlock_end, Dee_rshared_rwlock_end)

#define _DeeFrame_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->f_lock)
#define _DeeFrame_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->f_lock)
#define _DeeFrame_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->f_lock)
#define _DeeFrame_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->f_lock)
#define _DeeFrame_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->f_lock)
#define _DeeFrame_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->f_lock)
#define _DeeFrame_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->f_lock)
#define _DeeFrame_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->f_lock)
#define _DeeFrame_LockRead(self)       Dee_atomic_rwlock_read(&(self)->f_lock)
#define _DeeFrame_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->f_lock)
#define _DeeFrame_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->f_lock)
#define _DeeFrame_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->f_lock)
#define _DeeFrame_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->f_lock)
#define _DeeFrame_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->f_lock)
#define _DeeFrame_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->f_lock)
#define _DeeFrame_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->f_lock)

DDATDEF DeeTypeObject DeeFrame_Type;

/* Construct a frame object owned by `owner'
 * The intended use of this is for tracebacks and yield_function-iterators.
 * @param: flags: Set of `DEEFRAME_F*' */
DFUNDEF WUNUSED NONNULL((2)) DREF DeeObject *
(DCALL DeeFrame_NewReferenceWithLock)(/*[0..1]*/ DeeObject *owner,
                                      struct Dee_code_frame *__restrict frame,
                                      uint16_t flags, void *lock);
#ifdef CONFIG_NO_THREADS
#define DeeFrame_NewReferenceWithLock(owner, frame, flags, lock) \
	DeeFrame_NewReferenceWithLock(owner, frame, flags, NULL)
#endif /* CONFIG_NO_THREADS */
#define DeeFrame_NewReference(owner, frame, flags) \
	DeeFrame_NewReferenceWithLock(owner, frame, flags, NULL)

/* Construct a shared frame object, which can be manually
 * invalidated once the caller calls `DeeFrame_DecrefShared()'.
 * The intended use of this is for user-code handling of breakpoints.
 * @param: flags: Set of `DEEFRAME_F*' */
#define DeeFrame_NewSharedWithLock(frame, flags, lock) \
	DeeFrame_NewReferenceWithLock(NULL, frame, flags, lock)
#define DeeFrame_NewShared(frame, flags) \
	DeeFrame_NewReference(NULL, frame, flags)
DFUNDEF NONNULL((1)) void DCALL
DeeFrame_DecrefShared(DREF DeeObject *__restrict self);


/* Acquire locks to the frame that is underlying to `self'
 * NOTE: When aquiring for writing, these functions also check
 *       `DeeFrame_CanWrite()' and will throw an error if writing
 *       isn't allowed.
 * @return: * :   The underlying code frame.
 * @return: NULL: An error was thrown. */
DFUNDEF WUNUSED NONNULL((1)) struct Dee_code_frame const *DCALL DeeFrame_LockRead(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) struct Dee_code_frame *DCALL DeeFrame_LockWrite(DeeObject *__restrict self);
#ifdef CONFIG_NO_THREADS
#define DeeFrame_LockEndRead(self)  (void)0
#define DeeFrame_LockEndWrite(self) (void)0
#else /* CONFIG_NO_THREADS */
DFUNDEF NONNULL((1)) void DCALL DeeFrame_LockEndRead(DeeObject *__restrict self);
DFUNDEF NONNULL((1)) void DCALL DeeFrame_LockEndWrite(DeeObject *__restrict self);
#endif /* !CONFIG_NO_THREADS */

/* Same as `DeeFrame_LockWrite()', but also set the "CODE_FASSEMBLY"
 * flag for the underlying code object (if not set already). */
DFUNDEF WUNUSED NONNULL((1)) struct Dee_code_frame *DCALL
DeeFrame_LockWriteAssembly(DeeObject *__restrict self);

/* Same as above, but return `Dee_CODE_FRAME_DEAD' if the
 * frame is dead, rather than throw a `ReferenceError'.
 * @return: * :                  The underlying code frame.
 * @return: NULL:                An error was thrown.
 * @return: Dee_CODE_FRAME_DEAD: The underlying code frame is dead (no error was thrown). */
DFUNDEF WUNUSED NONNULL((1)) struct Dee_code_frame const *DCALL DeeFrame_LockReadIfNotDead(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) struct Dee_code_frame *DCALL DeeFrame_LockWriteIfNotDead(DeeObject *__restrict self);
#define Dee_CODE_FRAME_DEAD ((struct Dee_code_frame *)ITER_DONE)


DECL_END

#endif /* !GUARD_DEEMON_TRACEBACK_H */
