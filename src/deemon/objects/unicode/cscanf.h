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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_CSCANF_H
#define GUARD_DEEMON_OBJECTS_UNICODE_CSCANF_H 1

#include <deemon/api.h>

#include <deemon/object.h>    /* DREF, DeeObject, DeeTypeObject */
#include <deemon/util/lock.h> /* Dee_atomic_lock_* */

#include "../generic-proxy.h"

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD2(ss_data,   /* [1..1][const] The string data object (either a string, or Bytes object). */
	                   ss_format) /* [1..1][const] The scanner format object (either a string, or Bytes object). */
} StringScanner;

typedef struct {
	PROXY_OBJECT_HEAD_EX(StringScanner, si_scanner) /* [1..1][const] The underlying scanner. */
	char const                         *si_datend;  /* [1..1][const] End address of the input data (dereferences to a NUL-character). */
	char const                         *si_fmtend;  /* [1..1][const] End address of the format string (dereferences to a NUL-character). */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t                   si_lock;    /* Lock for modifying the data and format pointers.
	                                                 * NOTE: Not required to be held when reading those pointers! */
#endif /* !CONFIG_NO_THREADS */
	char const                         *si_datiter; /* [1..1][lock(READ(atomic), WRITE(si_lock))] The current data pointer (UTF-8). */
	char const                         *si_fmtiter; /* [1..1][lock(READ(atomic), WRITE(si_lock))] The current format pointer (UTF-8). */
} StringScanIterator;

#define StringScanIterator_LockAvailable(self)  Dee_atomic_lock_available(&(self)->si_lock)
#define StringScanIterator_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->si_lock)
#define StringScanIterator_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->si_lock)
#define StringScanIterator_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->si_lock)
#define StringScanIterator_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->si_lock)
#define StringScanIterator_LockRelease(self)    Dee_atomic_lock_release(&(self)->si_lock)

INTDEF DeeTypeObject StringScanIterator_Type;
INTDEF DeeTypeObject StringScan_Type;


/* Implement c-style string scanning, using a scanf()-style format string.
 * This functions then returns a sequence of all scanned objects, that is
 * the usually used in an expand expression:
 * >> for (local line: File.stdin) {
 * >>     local a, b, c;
 * >>     try {
 * >>         a, b, c = line.scanf("%s %s %s")...;
 * >>     } catch (...) { // Unpack errors.
 * >>         continue;
 * >>     }
 * >>     print "a:", a;
 * >>     print "b:", b;
 * >>     print "c:", c;
 * >> }
 */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_Scanf(DeeObject *self, DeeObject *format);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_CSCANF_H */
