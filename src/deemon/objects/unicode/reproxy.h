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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_H
#define GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_H 1

#include <deemon/api.h>

#include <deemon/bytes.h>     /* DeeBytesObject */
#include <deemon/object.h>    /* DREF, DeeObject, DeeTypeObject */
#include <deemon/string.h>    /* DeeStringObject */
#include <deemon/util/lock.h> /* Dee_atomic_rwlock_* */

#include "../generic-proxy.h"
#include "regex.h"

DECL_BEGIN

INTDEF DeeTypeObject ReFindAll_Type;
INTDEF DeeTypeObject ReFindAllIterator_Type;
INTDEF DeeTypeObject RegFindAll_Type;
INTDEF DeeTypeObject RegFindAllIterator_Type;
INTDEF DeeTypeObject ReLocateAll_Type;
INTDEF DeeTypeObject ReLocateAllIterator_Type;
INTDEF DeeTypeObject RegLocateAll_Type;
INTDEF DeeTypeObject RegLocateAllIterator_Type;
INTDEF DeeTypeObject ReSplit_Type;
INTDEF DeeTypeObject ReSplitIterator_Type;
INTDEF DeeTypeObject ReBytesFindAll_Type;
INTDEF DeeTypeObject ReBytesFindAllIterator_Type;
INTDEF DeeTypeObject RegBytesFindAll_Type;
INTDEF DeeTypeObject RegBytesFindAllIterator_Type;
INTDEF DeeTypeObject RegBytesLocateAll_Type;
INTDEF DeeTypeObject RegBytesLocateAllIterator_Type;
INTDEF DeeTypeObject ReBytesLocateAll_Type;
INTDEF DeeTypeObject ReBytesLocateAllIterator_Type;
INTDEF DeeTypeObject ReBytesSplit_Type;
INTDEF DeeTypeObject ReBytesSplitIterator_Type;

#define DeeRegexBaseExec_Load(self, code, result, nmatch, pmatch) \
	(void)((result)->rx_code     = (code),                        \
	       (result)->rx_inbase   = (self)->rx_inbase,             \
	       (result)->rx_insize   = (self)->rx_insize,             \
	       (result)->rx_startoff = (self)->rx_startoff,           \
	       (result)->rx_endoff   = (self)->rx_endoff,             \
	       (result)->rx_eflags   = (self)->rx_eflags,             \
	       (result)->rx_nmatch   = (nmatch),                      \
	       (result)->rx_pmatch   = (pmatch))

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_findall(DeeBytesObject *__restrict self,
                 struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_reg_findall(DeeBytesObject *__restrict self,
                 struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_locateall(DeeBytesObject *__restrict self,
                   struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_reg_locateall(DeeBytesObject *__restrict self,
                    struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_split(DeeBytesObject *__restrict self,
               struct DeeRegexBaseExec const *__restrict exec);


INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_re_findall(DeeStringObject *__restrict self,
                  struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_reg_findall(DeeStringObject *__restrict self,
                   struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_reg_locateall(DeeStringObject *__restrict self,
                     struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_re_locateall(DeeStringObject *__restrict self,
                    struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_re_split(DeeStringObject *__restrict self,
                struct DeeRegexBaseExec const *__restrict exec);


typedef struct {
	PROXY_OBJECT_HEAD      (rs_data) /* [const][1..1] Data string or Bytes. */
	struct DeeRegexBaseExec rs_exec; /* [const] Regex arguments. */
} ReSequence;


typedef struct {
	PROXY_OBJECT_HEAD      (rsi_data) /* [const][1..1] Data string or Bytes. */
	struct DeeRegexBaseExec rsi_exec; /* [lock(rsi_lock)] Regex arguments. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t     rsi_lock; /* Lock used during iteration. */
#endif /* !CONFIG_NO_THREADS */
} ReSequenceIterator;

#define ReSequenceIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->rsi_lock)
#define ReSequenceIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->rsi_lock)
#define ReSequenceIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->rsi_lock)
#define ReSequenceIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->rsi_lock)
#define ReSequenceIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->rsi_lock)
#define ReSequenceIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->rsi_lock)
#define ReSequenceIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->rsi_lock)
#define ReSequenceIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->rsi_lock)
#define ReSequenceIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->rsi_lock)
#define ReSequenceIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->rsi_lock)
#define ReSequenceIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->rsi_lock)
#define ReSequenceIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->rsi_lock)
#define ReSequenceIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->rsi_lock)
#define ReSequenceIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->rsi_lock)
#define ReSequenceIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->rsi_lock)
#define ReSequenceIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->rsi_lock)

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_H */
