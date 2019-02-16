/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_UTIL_RWLOCK_H
#define GUARD_DEEMON_UTIL_RWLOCK_H 1

#include "../api.h"
#ifdef CONFIG_NO_THREADS
typedef int Dee_rwlock_t;
#define Dee_RWLOCK_INIT                   0
#define Dee_rwlock_cinit(self)      (void)0
#define Dee_rwlock_init(self)       (void)0
#define Dee_DEFINE_RWLOCK(name)      Dee_rwlock_t name = 0
#define Dee_rwlock_reading(x)             1
#define Dee_rwlock_writing(x)             1
#define Dee_rwlock_tryread(self)          1
#define Dee_rwlock_trywrite(self)         1
#define Dee_rwlock_read(self)       (void)0
#define Dee_rwlock_write(self)      (void)0
#define Dee_rwlock_tryupgrade(self)       1
#define Dee_rwlock_upgrade(self)          1
#define Dee_rwlock_downgrade(self)  (void)0
#define Dee_rwlock_endwrite(self)   (void)0
#define Dee_rwlock_endread(self)    (void)0
#define Dee_rwlock_end(self)        (void)0
#else
#include <hybrid/sync/atomic-rwlock.h>

typedef atomic_rwlock_t Dee_rwlock_t;
#define Dee_RWLOCK_INIT             ATOMIC_RWLOCK_INIT
#define Dee_rwlock_cinit(self)      atomic_rwlock_cinit(self)
#define Dee_rwlock_init(self)       atomic_rwlock_init(self)
#define Dee_DEFINE_RWLOCK(name)     DEFINE_ATOMIC_RWLOCK(name)
#define Dee_rwlock_reading(x)       atomic_rwlock_reading(x)
#define Dee_rwlock_writing(x)       atomic_rwlock_writing(x)
#define Dee_rwlock_tryread(self)    atomic_rwlock_tryread(self)
#define Dee_rwlock_trywrite(self)   atomic_rwlock_trywrite(self)
#define Dee_rwlock_read(self)       atomic_rwlock_read(self)
#define Dee_rwlock_write(self)      atomic_rwlock_write(self)
#define Dee_rwlock_tryupgrade(self) atomic_rwlock_tryupgrade(self)
#define Dee_rwlock_upgrade(self)    atomic_rwlock_upgrade(self)
#define Dee_rwlock_downgrade(self)  atomic_rwlock_downgrade(self)
#define Dee_rwlock_endwrite(self)   atomic_rwlock_endwrite(self)
#define Dee_rwlock_endread(self)    atomic_rwlock_endread(self)
#define Dee_rwlock_end(self)        atomic_rwlock_end(self)
#endif

#ifdef DEE_SOURCE
typedef Dee_rwlock_t rwlock_t;
#define RWLOCK_INIT       Dee_RWLOCK_INIT
#define rwlock_cinit      Dee_rwlock_cinit
#define rwlock_init       Dee_rwlock_init
#define DEFINE_RWLOCK     Dee_DEFINE_RWLOCK
#define rwlock_reading    Dee_rwlock_reading
#define rwlock_writing    Dee_rwlock_writing
#define rwlock_tryread    Dee_rwlock_tryread
#define rwlock_trywrite   Dee_rwlock_trywrite
#define rwlock_read       Dee_rwlock_read
#define rwlock_write      Dee_rwlock_write
#define rwlock_tryupgrade Dee_rwlock_tryupgrade
#define rwlock_upgrade    Dee_rwlock_upgrade
#define rwlock_downgrade  Dee_rwlock_downgrade
#define rwlock_endwrite   Dee_rwlock_endwrite
#define rwlock_endread    Dee_rwlock_endread
#define rwlock_end        Dee_rwlock_end
#endif /* DEE_SOURCE */

#endif /* !GUARD_DEEMON_UTIL_RWLOCK_H */
