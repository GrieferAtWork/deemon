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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_TYPEMRO_H
#define GUARD_DEEMON_OBJECTS_SEQ_TYPEMRO_H 1

#include <deemon/api.h>

#include <deemon/object.h>
#include <deemon/util/lock.h> /* Dee_atomic_lock_* */

#include "../generic-proxy.h"

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DeeTypeMRO        tmi_mro;  /* [lock(tmi_lock)] MRO iterator (holds a reference to `tp_mro_orig'). */
	DeeTypeObject    *tmi_iter; /* [0..1][lock(tmi_lock)] The type that was enumerated previously (when
	                             * set to `NULL' for `TypeMROIterator_Type', then enumeration has yet to
	                             * begin) */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t tmi_lock; /* Lock for `tmi_mro' */
#endif /* !CONFIG_NO_THREADS */
} TypeMROIterator;

#define TypeMROIterator_LockAvailable(self)  Dee_atomic_lock_available(&(self)->tmi_lock)
#define TypeMROIterator_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->tmi_lock)
#define TypeMROIterator_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->tmi_lock)
#define TypeMROIterator_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->tmi_lock)
#define TypeMROIterator_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->tmi_lock)
#define TypeMROIterator_LockRelease(self)    Dee_atomic_lock_release(&(self)->tmi_lock)

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeTypeObject, tm_type); /* [1..1][const] The type whose mro/bases is being queried. */
} TypeMRO;

/* Helper types for enumerating a type's mro/bases */
INTDEF DeeTypeObject TypeMROIterator_Type;
INTDEF DeeTypeObject TypeMRO_Type;
INTDEF DeeTypeObject TypeBasesIterator_Type;
INTDEF DeeTypeObject TypeBases_Type;

/* Construct wrappers for the mro/bases of a given type. */
INTDEF WUNUSED NONNULL((1)) DREF TypeMRO *DCALL TypeMRO_New(DeeTypeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF TypeMRO *DCALL TypeBases_New(DeeTypeObject *__restrict self);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_TYPEMRO_H */
