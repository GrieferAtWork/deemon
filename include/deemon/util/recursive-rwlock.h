/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_UTIL_RECURSIVE_RWLOCK_H
#define GUARD_DEEMON_UTIL_RECURSIVE_RWLOCK_H 1

#include "../api.h"

#ifdef CONFIG_NO_THREADS
typedef int Dee_recursive_rwlock_t;
#define Dee_RECURSIVE_RWLOCK_INIT                   0
#define Dee_recursive_rwlock_cinit(self)      (void)0
#define Dee_recursive_rwlock_init(self)       (void)0
#define Dee_DEFINE_RECURSIVE_RWLOCK(name)      Dee_recursive_rwlock_t name = 0
#define Dee_recursive_rwlock_reading(x)             1
#define Dee_recursive_rwlock_writing(x)             1
#define Dee_recursive_rwlock_tryread(self)          1
#define Dee_recursive_rwlock_trywrite(self)         1
#define Dee_recursive_rwlock_read(self)       (void)0
#define Dee_recursive_rwlock_write(self)      (void)0
#define Dee_recursive_rwlock_tryupgrade(self)       1
#define Dee_recursive_rwlock_upgrade(self)          1
#define Dee_recursive_rwlock_downgrade(self)  (void)0
#define Dee_recursive_rwlock_endwrite(self)   (void)0
#define Dee_recursive_rwlock_endread(self)    (void)0
#define Dee_recursive_rwlock_end(self)        (void)0
#else /* CONFIG_NO_THREADS */
#include <hybrid/sync/atomic-owner-rwlock.h>

typedef struct atomic_owner_rwlock Dee_recursive_rwlock_t;
#define Dee_RECURSIVE_RWLOCK_INIT             ATOMIC_OWNER_RWLOCK_INIT
#define Dee_recursive_rwlock_cinit(self)      atomic_owner_rwlock_cinit(self)
#define Dee_recursive_rwlock_init(self)       atomic_owner_rwlock_init(self)
#define Dee_DEFINE_RECURSIVE_RWLOCK(name)     Dee_recursive_rwlock_t name = Dee_RECURSIVE_RWLOCK_INIT
#define Dee_recursive_rwlock_reading(x)       atomic_owner_rwlock_reading(x)
#define Dee_recursive_rwlock_writing(x)       atomic_owner_rwlock_writing(x)
#define Dee_recursive_rwlock_tryread(self)    atomic_owner_rwlock_tryread(self)
#define Dee_recursive_rwlock_trywrite(self)   atomic_owner_rwlock_trywrite(self)
#define Dee_recursive_rwlock_read(self)       atomic_owner_rwlock_read(self)
#define Dee_recursive_rwlock_write(self)      atomic_owner_rwlock_write(self)
#define Dee_recursive_rwlock_tryupgrade(self) atomic_owner_rwlock_tryupgrade(self)
#define Dee_recursive_rwlock_upgrade(self)    atomic_owner_rwlock_upgrade(self)
#define Dee_recursive_rwlock_downgrade(self)  atomic_owner_rwlock_downgrade(self)
#define Dee_recursive_rwlock_endwrite(self)   atomic_owner_rwlock_endwrite(self)
#define Dee_recursive_rwlock_endread(self)    atomic_owner_rwlock_endread(self)
#define Dee_recursive_rwlock_end(self)        atomic_owner_rwlock_end(self)
#endif /* !CONFIG_NO_THREADS */

#ifdef DEE_SOURCE
typedef Dee_recursive_rwlock_t recursive_rwlock_t;
#define RECURSIVE_RWLOCK_INIT       Dee_RECURSIVE_RWLOCK_INIT
#define recursive_rwlock_cinit      Dee_recursive_rwlock_cinit
#define recursive_rwlock_init       Dee_recursive_rwlock_init
#define DEFINE_RECURSIVE_RWLOCK     Dee_DEFINE_RECURSIVE_RWLOCK
#define recursive_rwlock_reading    Dee_recursive_rwlock_reading
#define recursive_rwlock_writing    Dee_recursive_rwlock_writing
#define recursive_rwlock_tryread    Dee_recursive_rwlock_tryread
#define recursive_rwlock_trywrite   Dee_recursive_rwlock_trywrite
#define recursive_rwlock_read       Dee_recursive_rwlock_read
#define recursive_rwlock_write      Dee_recursive_rwlock_write
#define recursive_rwlock_tryupgrade Dee_recursive_rwlock_tryupgrade
#define recursive_rwlock_upgrade    Dee_recursive_rwlock_upgrade
#define recursive_rwlock_downgrade  Dee_recursive_rwlock_downgrade
#define recursive_rwlock_endwrite   Dee_recursive_rwlock_endwrite
#define recursive_rwlock_endread    Dee_recursive_rwlock_endread
#define recursive_rwlock_end        Dee_recursive_rwlock_end
#endif /* DEE_SOURCE */

#endif /* !GUARD_DEEMON_UTIL_RECURSIVE_RWLOCK_H */
