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
#ifndef GUARD_DEEMON_TRACEBACK_H
#define GUARD_DEEMON_TRACEBACK_H 1

#include "api.h"
#include "object.h"
#include "code.h"
#include <stddef.h>
#include <stdarg.h>

#ifndef CONFIG_NO_THREADS
#include "util/rwlock.h"
#endif

DECL_BEGIN

typedef struct traceback_object DeeTracebackObject;
struct thread_object;
struct code_frame;

struct traceback_object {
    OBJECT_HEAD /* GC object. */
    DREF struct thread_object *tb_thread;    /* [0..1][const] The thread for which this is a traceback. */
#ifndef CONFIG_NO_THREADS
    rwlock_t                   tb_lock;      /* Lock for accessing this traceback. */
#endif
    uint16_t                   tb_numframes; /* [const] The amount of allocated frames. */
    uint16_t                   tb_padding[3];/* ... */
    struct code_frame          tb_frames[1]; /* [0..tb_numframes][lock(tb_lock)]
                                              * [OVERRIDE([*].cf_frame,[0..1][owned])] May randomly be NULL if duplication failed.
                                              * [OVERRIDE([*].cf_func,DREF [1..1])]
                                              * [OVERRIDE([*].cf_argv,DREF [1..1][0..cf_argc][owned])]
                                              * [OVERRIDE([*].cf_this,DREF [0..1])]
                                              * [OVERRIDE([*].cf_vargs,DREF [0..1])]
                                              * [OVERRIDE([*].cf_result,DREF [0..1])]
                                              * [OVERRIDE([*].cf_prev,[?..?])]
                                              * [OVERRIDE([*].cf_flags,[valid])]
                                              * [OVERRIDE([*].cf_sp,[(!= NULL) == (cf_stack != NULL)][== cf_stack+cf_stacksz])] May randomly remain NULL if duplication failed.
                                              * [OVERRIDE([*].cf_stack,[(!= NULL) == (cf_stacksz == 0)][0..cf_stacksz][owned])]
                                              * [OVERRIDE([*].cf_stacksz,[(!= 0) == (cf_sp != NULL)])] Vector of copied frames.
                                              * NOTE: The stack vectors of frames are duplicated as the stack is unwound.
                                              *       Frames who's stack has yet to be duplicated have a `cf_sp = cf_stack = NULL', `cf_stacksz = 0'. */
};

#ifdef CONFIG_BUILDING_DEEMON
#ifdef GUARD_DEEMON_OBJECTS_TRACEBACK_C
struct empty_traceback_object {
    OBJECT_HEAD
    DREF struct thread_object *tb_thread;
#ifndef CONFIG_NO_THREADS
    rwlock_t                   tb_lock;
#endif
    uint16_t                   tb_numframes;
    uint16_t                   tb_padding[3];
};
INTDEF struct empty_traceback_object empty_traceback;
#else
INTDEF DeeTracebackObject empty_traceback;
#endif
#endif

DDATDEF DeeTypeObject DeeTraceback_Type;
#define DeeTraceback_Check(ob)      DeeObject_InstanceOfExact(ob,&DeeTraceback_Type) /* `traceback' is `final' */
#define DeeTraceback_CheckExact(ob) DeeObject_InstanceOfExact(ob,&DeeTraceback_Type)


#ifdef CONFIG_BUILDING_DEEMON
/* Fill in stack information in the given traceback for `frame'. */
INTDEF void DCALL DeeTraceback_AddFrame(DeeTracebackObject *__restrict self,
                                        struct code_frame *__restrict frame,
                                        uint16_t frame_id);
/* Try to create a new traceback, but don't throw
 * an error and return `NULL' if doing so failed.
 * NOTE: The given `thread' must be the caller's. */
INTDEF DREF DeeTracebackObject *DCALL
DeeTraceback_New(struct thread_object *__restrict thread);
#endif


typedef struct frame_object DeeFrameObject;
struct frame_object {
    OBJECT_HEAD /* More of a frame-reference object. */
    DREF DeeObject    *f_owner; /* [0..1][const] Owner of the frame (Required to prevent the frame from being destroyed). */
    struct code_frame *f_frame; /* [lock(*f_plock)][0..1][lock(f_lock)]
                                 * The actual frame that is being referenced. */
#ifndef CONFIG_NO_THREADS
    union {
        rwlock_t      *f_plock;       /* [0..1][valid_if(!DEEFRAME_FRECLOCK)][const]
                                       * Lock that must be acquired when accessing the frame. */
        recursive_rwlock_t *f_prlock; /* [1..1][valid_if(DEEFRAME_FRECLOCK)][const]
                                       * Lock that must be acquired when accessing the frame. */
    };
    rwlock_t           f_lock;  /* Lock for accessing fields of this frame object. */
#endif /* !CONFIG_NO_THREADS */
#define DEEFRAME_FNORMAL   0x0000 /* Normal frame flags. */
#define DEEFRAME_FREADONLY 0x0000 /* Contents of the frame may not be modified. */
#define DEEFRAME_FWRITABLE 0x0001 /* Contents of the frame may be modified. */
#define DEEFRAME_FUNDEFSP  0x0002 /* The stack-pointer of the frame is undefined.
                                   * When `DEEFRAME_FUNDEFSP2' isn't set, the correct stack
                                   * pointer may be obtainable from DDI information, as well
                                   * as use of the current IP, alongside further validation. */
#define DEEFRAME_FUNDEFSP2 0x0004 /* The stack-pointer of the frame is always undefined. */
#ifndef CONFIG_NO_THREADS
#define DEEFRAME_FRECLOCK  0x8000 /* The frame uses a recursive lock. */
#else /* !CONFIG_NO_THREADS */
#define DEEFRAME_FRECLOCK  0x0000 /* Ignored. */
#endif /* CONFIG_NO_THREADS */
    uint16_t           f_flags; /* [const] Contents of the frame may be modified. */
};

DDATDEF DeeTypeObject DeeFrame_Type;

/* Construct a frame object owned by `owner'
 * The intended use of this is for tracebacks and yield_function-iterators.
 * @param: flags: Set of `DEEFRAME_F*' */
#ifndef CONFIG_NO_THREADS
DFUNDEF DREF DeeObject *
(DCALL DeeFrame_NewReferenceWithLock)(DeeObject *owner,
                                      struct code_frame *__restrict frame,
                                      uint16_t flags, void *lock);
#define DeeFrame_NewReference(owner,frame,flags) \
        DeeFrame_NewReferenceWithLock(owner,frame,flags,NULL)
#else
DFUNDEF DREF DeeObject *
(DCALL DeeFrame_NewReference)(DeeObject *owner,
                              struct code_frame *__restrict frame,
                              uint16_t flags);
#define DeeFrame_NewReferenceWithLock(owner,frame,flags,lock) \
        DeeFrame_NewReference(owner,frame,flags)
#endif

/* Construct a shared frame object, which can be manually
 * invalidated once the caller calls `DeeFrame_DecrefShared()'.
 * The intended use of this is for user-code handling of breakpoints.
 * @param: flags: Set of `DEEFRAME_F*' */
#define DeeFrame_NewSharedWithLock(frame,flags,lock) \
        DeeFrame_NewReferenceWithLock(NULL,frame,flags,lock)
#define DeeFrame_NewShared(frame,flags) \
        DeeFrame_NewReference(NULL,frame,flags)
DFUNDEF void DCALL DeeFrame_DecrefShared(DREF DeeObject *__restrict self);


DECL_END

#endif /* !GUARD_DEEMON_TRACEBACK_H */
