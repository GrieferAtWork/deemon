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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_C_INL 1

#ifdef __INTELLISENSE__
#include "string_functions.c"
#endif /* !__INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/bytes.h>              /* DeeBytes* */
#include <deemon/computed-operators.h>
#include <deemon/int.h>                /* DeeInt_NewSize */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_AssertTypeExact, DeeTypeObject, Dee_AsObject, Dee_COMPARE_ERR, Dee_Decref, Dee_Incref, Dee_Incref_n, Dee_TYPE, Dee_hash_t, Dee_return_compareT, Dee_ssize_t, Dee_visit_t, ITER_DONE, OBJECT_HEAD_INIT */
#include <deemon/regex.h>              /* DeeRegex*, DeeString_GetRegex, DeeString_GetRegexFlags, Dee_RE_* */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_Type */
#include <deemon/serial.h>             /* DeeSerial*, Dee_seraddr_t */
#include <deemon/string.h>             /* DeeString*, Dee_EmptyString, STRING_ERROR_FSTRICT */
#include <deemon/system-features.h>    /* bzero, memcpy */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_GetName, DeeType_Type, METHOD_FNOREFESCAPE, STRUCT_*, TF_NONLOOPING, TP_F*, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* atomic_read, atomic_write */
#include <deemon/util/lock.h>          /* Dee_atomic_rwlock_* */

#include "../../runtime/runtime_error.h" /* err_invalid_argc */
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "regroups.h"
#include "string_functions.h"

#include <stddef.h> /* NULL, offsetof, size_t */
#include <stdint.h> /* uintptr_t */


/* Proxy sequence objects for `string.refindall',
 * `string.relocateall' and `string.resplit' */

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


#define _Dee_RE_COMPILE_MASK \
	(Dee_RE_COMPILE_NORMAL | Dee_RE_COMPILE_ICASE | Dee_RE_COMPILE_NOUTF8)
STATIC_ASSERT_MSG(_Dee_RE_COMPILE_MASK <= 0xf,
                  "Keep this low, since no valid pointer must overlap with this range");

/* Return the `struct DeeRegexCode *' of `self'
 * @return: * :   The regex code of `self'
 * @return: NULL: An error was thrown */
#define DeeRegexBaseExec_GetCode(self) \
	(likely((uintptr_t)(self)->rx_code > _Dee_RE_COMPILE_MASK) ? (self)->rx_code : _DeeRegexBaseExec_LoadCode(self))
PRIVATE ATTR_COLD ATTR_NOINLINE WUNUSED NONNULL((1)) struct DeeRegexCode const *DCALL
_DeeRegexBaseExec_LoadCode(struct DeeRegexBaseExec *__restrict self) {
	struct DeeRegexCode const *result = atomic_read(&self->rx_code);
	if unlikely((uintptr_t)result > _Dee_RE_COMPILE_MASK)
		return result; /* Already loaded */
	/* Lazily re-load after serialization */
	result = DeeString_GetRegex((DeeObject *)self->rx_pattern,
	                            (unsigned int)(uintptr_t)result,
	                            NULL);
	if likely(result)
		atomic_write(&self->rx_code, result);
	return result;
}


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

STATIC_ASSERT(offsetof(ReSequenceIterator, rsi_data) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(ReSequenceIterator, rsi_data) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(ReSequenceIterator, rsi_exec.rx_pattern) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(ReSequenceIterator, rsi_exec.rx_pattern) == offsetof(ProxyObject2, po_obj2));
#define ReSequenceIterator__fini  generic_proxy2__fini
#define ReSequenceIterator__visit generic_proxy2__visit

STATIC_ASSERT(offsetof(ReSequence, rs_data) == offsetof(ReSequenceIterator, rsi_data));
STATIC_ASSERT(offsetof(ReSequence, rs_exec) == offsetof(ReSequenceIterator, rsi_exec));
#define ReSequence__fini  generic_proxy2__fini
#define ReSequence__visit generic_proxy2__visit


PRIVATE WUNUSED NONNULL((1)) int DCALL
refaiter_ctor(ReSequenceIterator *__restrict self) {
	bzero(&self->rsi_exec, sizeof(self->rsi_exec));
	self->rsi_exec.rx_code = DeeString_GetRegex(Dee_EmptyString, Dee_RE_COMPILE_NORMAL, NULL);
	if unlikely(!self->rsi_exec.rx_code)
		return -1;
	self->rsi_data = Dee_EmptyString;
	self->rsi_exec.rx_pattern = (DREF String *)Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	Dee_atomic_rwlock_init(&self->rsi_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rebfaiter_ctor(ReSequenceIterator *__restrict self) {
	bzero(&self->rsi_exec, sizeof(self->rsi_exec));
	self->rsi_exec.rx_code = DeeString_GetRegex(Dee_EmptyString, Dee_RE_COMPILE_NORMAL, NULL);
	if unlikely(!self->rsi_exec.rx_code)
		return -1;
	self->rsi_data = DeeBytes_NewEmpty();
	self->rsi_exec.rx_pattern = (DREF String *)DeeString_NewEmpty();
	Dee_atomic_rwlock_init(&self->rsi_lock);
	return 0;
}

#define ReSequenceIterator__deep ReSequenceIterator__copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ReSequenceIterator__copy(ReSequenceIterator *__restrict self,
                         ReSequenceIterator *__restrict other) {
	ReSequenceIterator_LockRead(other);
	memcpy(&self->rsi_exec, &other->rsi_exec, sizeof(self->rsi_exec));
	ReSequenceIterator_LockEndRead(other);
	self->rsi_data = other->rsi_data;
	Dee_Incref(self->rsi_data);
	Dee_Incref(self->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&self->rsi_lock);
	return 0;
}

#define ReSequence__deep ReSequence__copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ReSequence__copy(ReSequence *__restrict self,
                 ReSequence *__restrict other) {
	self->rs_data = other->rs_data;
	memcpy(&self->rs_exec, &other->rs_exec, sizeof(self->rs_exec));
	Dee_Incref(self->rs_data);
	Dee_Incref(self->rs_exec.rx_pattern);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeRegexBaseExec_Serialize(struct DeeRegexBaseExec *__restrict self,
                           DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(struct DeeRegexBaseExec, field))
	unsigned int compile_flags;
	struct DeeRegexBaseExec *out;
	if (DeeSerial_PutObject(writer, ADDROF(rx_pattern), self->rx_pattern))
		goto err;
	if ((uintptr_t)self->rx_code <= _Dee_RE_COMPILE_MASK) {
		compile_flags = (unsigned int)(uintptr_t)self->rx_code;
	} else {
		compile_flags = DeeString_GetRegexFlags((DeeObject *)self->rx_pattern, self->rx_code);
	}
	out = DeeSerial_Addr2Mem(writer, addr, struct DeeRegexBaseExec);
	/* Set "rx_code" to the flags that were used to compile it.
	 * After deserialization, this will be detected by `DeeRegexBaseExec_GetCode()',
	 * which will then re-compile the regex, meaning it doesn't need to be serialized
	 * as well! */
	out->rx_code     = (struct DeeRegexCode const *)(uintptr_t)compile_flags;
	out->rx_inbase   = NULL;
	out->rx_insize   = self->rx_insize;
	out->rx_startoff = self->rx_startoff;
	out->rx_endoff   = self->rx_endoff;
	out->rx_eflags   = self->rx_eflags;
	return DeeSerial_PutPointer(writer, ADDROF(rx_inbase), self->rx_inbase);
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ReSequenceIterator__serialize(ReSequenceIterator *__restrict self,
                              DeeSerial *__restrict writer, Dee_seraddr_t addr) {
	struct DeeRegexBaseExec self__rsi_exec;
#define ADDROF(field) (addr + offsetof(ReSequenceIterator, field))
	ReSequenceIterator *out;
	if (DeeSerial_PutObject(writer, ADDROF(rsi_data), self->rsi_data))
		goto err;
	out = DeeSerial_Addr2Mem(writer, addr, ReSequenceIterator);
	Dee_atomic_rwlock_init(&out->rsi_lock);
	ReSequenceIterator_LockRead(self);
	memcpy(&self__rsi_exec, &self->rsi_exec, sizeof(self->rsi_exec));
	ReSequenceIterator_LockEndRead(self);
	return DeeRegexBaseExec_Serialize(&self__rsi_exec, writer, ADDROF(rsi_exec));
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ReSequence__serialize(ReSequence *__restrict self,
                      DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(ReSequence, field))
	int result = DeeSerial_PutObject(writer, ADDROF(rs_data), self->rs_data);
	if likely(result == 0)
		result = DeeRegexBaseExec_Serialize(&self->rs_exec, writer, ADDROF(rs_exec));
	return result;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ReSequenceIterator__init(ReSequenceIterator *__restrict self,
                         size_t argc, DeeObject *const *argv) {
	ReSequence *reseq;
	DeeTypeObject *tp_seq, *tp_self = Dee_TYPE(self);
	if unlikely(argc != 1)
		return err_invalid_argc(DeeType_GetName(tp_self), argc, 1, 1);
#define MATCH(Titer, Tseq)  \
	if (tp_self == Titer) { \
		tp_seq = Tseq;      \
	} else 
	MATCH(&ReFindAllIterator_Type, &ReFindAll_Type)
	MATCH(&RegFindAllIterator_Type, &RegFindAll_Type)
	MATCH(&ReLocateAllIterator_Type, &ReLocateAll_Type)
	MATCH(&RegLocateAllIterator_Type, &RegLocateAll_Type)
	MATCH(&ReSplitIterator_Type, &ReSplit_Type)
	MATCH(&ReBytesFindAllIterator_Type, &ReBytesFindAll_Type)
	MATCH(&RegBytesFindAllIterator_Type, &RegBytesFindAll_Type)
	MATCH(&ReBytesLocateAllIterator_Type, &ReBytesLocateAll_Type)
	MATCH(&RegBytesLocateAllIterator_Type, &RegBytesLocateAll_Type)
	MATCH(&ReBytesSplitIterator_Type, &ReBytesSplit_Type)
	{
		__builtin_unreachable();
	}
#undef MATCH
	reseq = (ReSequence *)argv[0];
	if (DeeObject_AssertTypeExact(reseq, tp_seq))
		goto err;
	memcpy(&self->rsi_data, &reseq->rs_data,
	       sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	Dee_Incref(self->rsi_data);
	Dee_Incref(self->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&self->rsi_lock);
	return 0;
err:
	return -1;
}

#define refaiter_init      ReSequenceIterator__init
#define refaiter_copy      ReSequenceIterator__copy
#define refaiter_deep      ReSequenceIterator__deep
#define refaiter_serialize ReSequenceIterator__serialize
#define refaiter_fini      ReSequenceIterator__fini
#define refaiter_visit     ReSequenceIterator__visit

#define rebfaiter_init      ReSequenceIterator__init
#define rebfaiter_copy      ReSequenceIterator__copy
#define rebfaiter_deep      ReSequenceIterator__deep /* XXX: Should actually deep-copy the underlying Bytes (since those are mutable...) */
#define rebfaiter_serialize ReSequenceIterator__serialize
#define rebfaiter_fini      ReSequenceIterator__fini
#define rebfaiter_visit     ReSequenceIterator__visit

#define rebfaiter_bool  refaiter_bool
PRIVATE WUNUSED NONNULL((1)) int DCALL
refaiter_bool(ReSequenceIterator *__restrict self) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	struct DeeRegexCode const *code;
	code = DeeRegexBaseExec_GetCode(&self->rsi_exec);
	if unlikely(!code)
		goto err;
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, code, &exec, 0, NULL);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err;
	return result != Dee_RE_STATUS_NOMATCH &&
	       match_size != 0; /* Prevent infinite loop on epsilon-match */
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
refaiter_nextpair(ReSequenceIterator *__restrict self, DREF DeeObject *pair[2]) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	struct DeeRegexCode const *code;
	code = DeeRegexBaseExec_GetCode(&self->rsi_exec);
	if unlikely(!code)
		goto err;
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, code, &exec, 0, NULL);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err;
	if (result == Dee_RE_STATUS_NOMATCH)
		return 1;
	if (match_size == 0)
		return 1; /* Prevent infinite loop on epsilon-match */
	ReSequenceIterator_LockWrite(self);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		ReSequenceIterator_LockEndWrite(self);
		goto again;
	}
	self->rsi_exec.rx_startoff = (size_t)result + match_size;
	ReSequenceIterator_LockEndWrite(self);
	match_size = string_bytecnt2charcnt((DREF String *)self->rsi_data, (size_t)match_size, (char const *)exec.rx_inbase + (size_t)result);
	result     = string_bytecnt2charcnt((DREF String *)self->rsi_data, (size_t)result, (char const *)exec.rx_inbase);
	match_size += (size_t)result;
	pair[0] = DeeInt_NewSize((size_t)result);
	if unlikely(!pair[0])
		goto err;
	pair[1] = DeeInt_NewSize((size_t)match_size);
	if unlikely(!pair[1])
		goto err_pair_0;
	return 0;
err_pair_0:
	Dee_Decref(pair[0]);
err:
	return -1;
}

PRIVATE struct type_iterator refaiter_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&refaiter_nextpair,
	/* .tp_nextkey   = */ DEFIMPL(&default__nextkey__with__nextpair),
	/* .tp_nextvalue = */ DEFIMPL(&default__nextvalue__with__nextpair),
	/* .tp_advance   = */ DEFIMPL(&default__advance__with__nextpair),
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rebfaiter_nextpair(ReSequenceIterator *__restrict self, DREF DeeObject *pair[2]) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	struct DeeRegexCode const *code;
	code = DeeRegexBaseExec_GetCode(&self->rsi_exec);
	if unlikely(!code)
		goto err;
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, code, &exec, 0, NULL);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err;
	if (result == Dee_RE_STATUS_NOMATCH)
		return 1;
	if (match_size == 0)
		return 1; /* Prevent infinite loop on epsilon-match */
	ReSequenceIterator_LockWrite(self);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		ReSequenceIterator_LockEndWrite(self);
		goto again;
	}
	self->rsi_exec.rx_startoff = (size_t)result + match_size;
	ReSequenceIterator_LockEndWrite(self);
	match_size += (size_t)result;
	pair[0] = DeeInt_NewSize((size_t)result);
	if unlikely(!pair[0])
		goto err;
	pair[1] = DeeInt_NewSize((size_t)match_size);
	if unlikely(!pair[1])
		goto err_pair_0;
	return 0;
err_pair_0:
	Dee_Decref(pair[0]);
err:
	return -1;
}

PRIVATE struct type_iterator rebfaiter_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&rebfaiter_nextpair,
	/* .tp_nextkey   = */ DEFIMPL(&default__nextkey__with__nextpair),
	/* .tp_nextvalue = */ DEFIMPL(&default__nextvalue__with__nextpair),
	/* .tp_advance   = */ DEFIMPL(&default__advance__with__nextpair),
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
refaiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	ReSequenceIterator_LockRead(self);
	memcpy(&args_copy, &self->rsi_exec, sizeof(struct DeeRegexBaseExec));
	ReSequenceIterator_LockEndRead(self);
	return string_re_findall((DeeStringObject *)self->rsi_data, &args_copy);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rebfaiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	ReSequenceIterator_LockRead(self);
	memcpy(&args_copy, &self->rsi_exec, sizeof(struct DeeRegexBaseExec));
	ReSequenceIterator_LockEndRead(self);
	return bytes_re_findall((DeeBytesObject *)self->rsi_data, &args_copy);
}

#define REITER_GETDATAPTR(x) atomic_read(&(x)->rsi_exec.rx_startoff)

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
refa_hash(ReSequenceIterator *self) {
	return (Dee_hash_t)REITER_GETDATAPTR(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
refa_compare(ReSequenceIterator *self, ReSequenceIterator *other) {
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	Dee_return_compareT(size_t, REITER_GETDATAPTR(self),
	                    /*   */ REITER_GETDATAPTR(other));
err:
	return Dee_COMPARE_ERR;
}


#define rebfaiter_cmp refaiter_cmp
PRIVATE struct type_cmp refaiter_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&refa_hash,
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&refa_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


PRIVATE struct type_getset tpconst refaiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &refaiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:ReFindAll"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst rebfaiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &rebfaiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:ReBytesFindAll"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst refaiter_members[] = {
#define rebfaiter_members refaiter_members
	TYPE_MEMBER_FIELD_DOC("__input__", STRUCT_OBJECT, offsetof(ReSequenceIterator, rsi_data), "->?X2?Dstring?DBytes"),
	TYPE_MEMBER_FIELD_DOC("__pattern__", STRUCT_OBJECT, offsetof(ReSequenceIterator, rsi_exec.rx_pattern), "->?Dstring"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReSequenceIterator, rsi_exec.rx_startoff)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReSequenceIterator, rsi_exec.rx_endoff)),
	TYPE_MEMBER_BITFIELD("__notbol__", STRUCT_CONST, ReSequenceIterator, rsi_exec.rx_eflags, Dee_RE_EXEC_NOTBOL),
	TYPE_MEMBER_BITFIELD("__noteol__", STRUCT_CONST, ReSequenceIterator, rsi_exec.rx_eflags, Dee_RE_EXEC_NOTEOL),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject ReFindAllIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReFindAllIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReFindAll)\n"
	                         "\n"
	                         "next->?X2?T2?Dint?Dint?N"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequenceIterator,
			/* tp_ctor:        */ &refaiter_ctor,
			/* tp_copy_ctor:   */ &refaiter_copy,
			/* tp_deep_ctor:   */ &refaiter_deep,
			/* tp_any_ctor:    */ &refaiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &refaiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&refaiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&refaiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&refaiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &refaiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &refaiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ refaiter_getsets,
	/* .tp_members       = */ refaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject ReBytesFindAllIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReBytesFindAllIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReBytesFindAll)\n"
	                         "\n"
	                         "next->?X2?T2?Dint?Dint?N"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequenceIterator,
			/* tp_ctor:        */ &rebfaiter_ctor,
			/* tp_copy_ctor:   */ &rebfaiter_copy,
			/* tp_deep_ctor:   */ &rebfaiter_deep,
			/* tp_any_ctor:    */ &rebfaiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rebfaiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rebfaiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rebfaiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rebfaiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &rebfaiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &rebfaiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ rebfaiter_getsets,
	/* .tp_members       = */ rebfaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


#define regfaiter_ctor      refaiter_ctor
#define regfaiter_copy      refaiter_copy
#define regfaiter_deep      refaiter_deep
#define regfaiter_serialize refaiter_serialize
#define regfaiter_init      refaiter_init
#define regfaiter_fini      refaiter_fini
#define regfaiter_bool      refaiter_bool
#define regfaiter_visit     refaiter_visit
#define regfaiter_cmp       refaiter_cmp
#define regfaiter_members   refaiter_members

#define regbfaiter_ctor      rebfaiter_ctor
#define regbfaiter_copy      rebfaiter_copy
#define regbfaiter_deep      rebfaiter_deep
#define regbfaiter_serialize rebfaiter_serialize
#define regbfaiter_init      rebfaiter_init
#define regbfaiter_fini      rebfaiter_fini
#define regbfaiter_bool      rebfaiter_bool
#define regbfaiter_visit     rebfaiter_visit
#define regbfaiter_cmp       rebfaiter_cmp
#define regbfaiter_members   rebfaiter_members

PRIVATE WUNUSED NONNULL((1)) DREF ReGroups *DCALL
regfaiter_next(ReSequenceIterator *__restrict self) {
	DREF ReGroups *groups;
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	struct DeeRegexCode const *code;
	code = DeeRegexBaseExec_GetCode(&self->rsi_exec);
	if unlikely(!code)
		goto err;
	groups = ReGroups_Malloc(1 + code->rc_ngrps);
	if unlikely(!groups)
		goto err;
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, code, &exec,
	                      code->rc_ngrps, groups->rg_groups + 1);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err_g;
	if (result == Dee_RE_STATUS_NOMATCH)
		goto return_ITER_DONE;
	if (match_size == 0)
		goto return_ITER_DONE; /* Prevent infinite loop on epsilon-match */
	ReSequenceIterator_LockWrite(self);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		ReSequenceIterator_LockEndWrite(self);
		goto again;
	}
	self->rsi_exec.rx_startoff = (size_t)result + match_size;
	ReSequenceIterator_LockEndWrite(self);
	string_bytecnt2charcnt_v((String *)self->rsi_data, (char const *)exec.rx_inbase,
	                         groups->rg_groups + 1, code->rc_ngrps);
	match_size = string_bytecnt2charcnt((String *)self->rsi_data, (size_t)match_size, (char const *)exec.rx_inbase + (size_t)result);
	result     = string_bytecnt2charcnt((String *)self->rsi_data, (size_t)result, (char const *)exec.rx_inbase);
	match_size += (size_t)result;
	groups->rg_groups[0].rm_so = result;
	groups->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(groups, 1 + code->rc_ngrps);
	return groups;
return_ITER_DONE:
	ReGroups_Free(groups);
	return (DREF ReGroups *)ITER_DONE;
err_g:
	ReGroups_Free(groups);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF ReGroups *DCALL
regbfaiter_next(ReSequenceIterator *__restrict self) {
	DREF ReGroups *groups;
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	struct DeeRegexCode const *code;
	code = DeeRegexBaseExec_GetCode(&self->rsi_exec);
	if unlikely(!code)
		goto err;
	groups = ReGroups_Malloc(1 + code->rc_ngrps);
	if unlikely(!groups)
		goto err;
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, code, &exec,
	                      code->rc_ngrps, groups->rg_groups + 1);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err_g;
	if (result == Dee_RE_STATUS_NOMATCH)
		goto return_ITER_DONE;
	if (match_size == 0)
		goto return_ITER_DONE; /* Prevent infinite loop on epsilon-match */
	ReSequenceIterator_LockWrite(self);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		ReSequenceIterator_LockEndWrite(self);
		goto again;
	}
	self->rsi_exec.rx_startoff = (size_t)result + match_size;
	ReSequenceIterator_LockEndWrite(self);
	match_size += (size_t)result;
	groups->rg_groups[0].rm_so = result;
	groups->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(groups, 1 + code->rc_ngrps);
	return groups;
return_ITER_DONE:
	ReGroups_Free(groups);
	return (DREF ReGroups *)ITER_DONE;
err_g:
	ReGroups_Free(groups);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
regfaiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	ReSequenceIterator_LockRead(self);
	memcpy(&args_copy, &self->rsi_exec, sizeof(struct DeeRegexBaseExec));
	ReSequenceIterator_LockEndRead(self);
	return string_reg_findall((String *)self->rsi_data, &args_copy);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
regbfaiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	ReSequenceIterator_LockRead(self);
	memcpy(&args_copy, &self->rsi_exec, sizeof(struct DeeRegexBaseExec));
	ReSequenceIterator_LockEndRead(self);
	return bytes_reg_findall((DeeBytesObject *)self->rsi_data, &args_copy);
}

PRIVATE struct type_getset tpconst regfaiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &regfaiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:RegFindAll"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst regbfaiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &regbfaiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:RegBytesFindAll"),
	TYPE_GETSET_END
};

INTERN DeeTypeObject RegFindAllIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RegFindAllIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReFindAll)\n"
	                         "\n"
	                         "next->?Ert:ReGroups"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequenceIterator,
			/* tp_ctor:        */ &regfaiter_ctor,
			/* tp_copy_ctor:   */ &regfaiter_copy,
			/* tp_deep_ctor:   */ &regfaiter_deep,
			/* tp_any_ctor:    */ &regfaiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &regfaiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&regfaiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&regfaiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&regfaiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &regfaiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&regfaiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ regfaiter_getsets,
	/* .tp_members       = */ regfaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject RegBytesFindAllIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RegBytesFindAllIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReBytesFindAll)\n"
	                         "\n"
	                         "next->?Ert:ReGroups"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequenceIterator,
			/* tp_ctor:        */ &regbfaiter_ctor,
			/* tp_copy_ctor:   */ &regbfaiter_copy,
			/* tp_deep_ctor:   */ &regbfaiter_deep,
			/* tp_any_ctor:    */ &regbfaiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &regbfaiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&regbfaiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&regbfaiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&regbfaiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &regbfaiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&regbfaiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ regbfaiter_getsets,
	/* .tp_members       = */ regbfaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};



/* LocateAllIterator wrapper (based on FindAll) */
#define reglaiter_ctor      regfaiter_ctor
#define reglaiter_copy      regfaiter_copy
#define reglaiter_deep      regfaiter_deep
#define reglaiter_serialize regfaiter_serialize
#define reglaiter_init      regfaiter_init
#define reglaiter_fini      regfaiter_fini
#define reglaiter_bool      regfaiter_bool
#define reglaiter_visit     regfaiter_visit
#define reglaiter_cmp       regfaiter_cmp
#define reglaiter_members   regfaiter_members

#define regblaiter_ctor      regbfaiter_ctor
#define regblaiter_copy      regbfaiter_copy
#define regblaiter_deep      regbfaiter_deep
#define regblaiter_serialize regbfaiter_serialize
#define regblaiter_init      regbfaiter_init
#define regblaiter_fini      regbfaiter_fini
#define regblaiter_bool      regbfaiter_bool
#define regblaiter_visit     regbfaiter_visit
#define regblaiter_cmp       regbfaiter_cmp
#define regblaiter_members   regbfaiter_members

PRIVATE WUNUSED NONNULL((1)) DREF ReSubStrings *DCALL
reglaiter_next(ReSequenceIterator *__restrict self) {
	DREF ReSubStrings *substrings;
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	struct DeeRegexCode const *code;
	code = DeeRegexBaseExec_GetCode(&self->rsi_exec);
	if unlikely(!code)
		goto err;
	substrings = ReSubStrings_Malloc(1 + code->rc_ngrps);
	if unlikely(!substrings)
		goto err;
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, code, &exec,
	                      code->rc_ngrps, substrings->rss_groups + 1);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err_g;
	if (result == Dee_RE_STATUS_NOMATCH)
		goto return_ITER_DONE;
	if (match_size == 0)
		goto return_ITER_DONE; /* Prevent infinite loop on epsilon-match */
	ReSequenceIterator_LockWrite(self);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		ReSequenceIterator_LockEndWrite(self);
		goto again;
	}
	self->rsi_exec.rx_startoff = (size_t)result + match_size;
	ReSequenceIterator_LockEndWrite(self);
	substrings->rss_groups[0].rm_so = (size_t)result;
	substrings->rss_groups[0].rm_eo = (size_t)result + match_size;
	ReSubStrings_Init(substrings, self->rsi_data, exec.rx_inbase,
	                  1 + code->rc_ngrps);
	return substrings;
return_ITER_DONE:
	ReSubStrings_Free(substrings);
	return (DREF ReSubStrings *)ITER_DONE;
err_g:
	ReSubStrings_Free(substrings);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF ReSubBytes *DCALL
regblaiter_next(ReSequenceIterator *__restrict self) {
	DREF ReSubBytes *subbytes;
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	struct DeeRegexCode const *code;
	code = DeeRegexBaseExec_GetCode(&self->rsi_exec);
	if unlikely(!code)
		goto err;
	subbytes = ReSubBytes_Malloc(1 + code->rc_ngrps);
	if unlikely(!subbytes)
		goto err;
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, code, &exec,
	                      code->rc_ngrps, subbytes->rss_groups + 1);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err_g;
	if (result == Dee_RE_STATUS_NOMATCH)
		goto return_ITER_DONE;
	if (match_size == 0)
		goto return_ITER_DONE; /* Prevent infinite loop on epsilon-match */
	ReSequenceIterator_LockWrite(self);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		ReSequenceIterator_LockEndWrite(self);
		goto again;
	}
	self->rsi_exec.rx_startoff = (size_t)result + match_size;
	ReSequenceIterator_LockEndWrite(self);
	subbytes->rss_groups[0].rm_so = (size_t)result;
	subbytes->rss_groups[0].rm_eo = (size_t)result + match_size;
	ReSubBytes_Init(subbytes, self->rsi_data, exec.rx_inbase,
	                1 + code->rc_ngrps);
	return subbytes;
return_ITER_DONE:
	ReSubBytes_Free(subbytes);
	return (DREF ReSubBytes *)ITER_DONE;
err_g:
	ReSubBytes_Free(subbytes);
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
reglaiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	ReSequenceIterator_LockRead(self);
	memcpy(&args_copy, &self->rsi_exec, sizeof(struct DeeRegexBaseExec));
	ReSequenceIterator_LockEndRead(self);
	return string_reg_locateall((String *)self->rsi_data, &args_copy);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
regblaiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	ReSequenceIterator_LockRead(self);
	memcpy(&args_copy, &self->rsi_exec, sizeof(struct DeeRegexBaseExec));
	ReSequenceIterator_LockEndRead(self);
	return bytes_reg_locateall((DeeBytesObject *)self->rsi_data, &args_copy);
}


PRIVATE struct type_getset tpconst reglaiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &reglaiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:RegLocateAll"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst regblaiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &regblaiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:RegBytesLocateAll"),
	TYPE_GETSET_END
};


INTERN DeeTypeObject RegLocateAllIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RegLocateAllIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReLocateAll)\n"
	                         "\n"
	                         "next->?Ert:ReSubStrings"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequenceIterator,
			/* tp_ctor:        */ &reglaiter_ctor,
			/* tp_copy_ctor:   */ &reglaiter_copy,
			/* tp_deep_ctor:   */ &reglaiter_deep,
			/* tp_any_ctor:    */ &reglaiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &reglaiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&reglaiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&reglaiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&reglaiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &reglaiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&reglaiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ reglaiter_getsets,
	/* .tp_members       = */ reglaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject RegBytesLocateAllIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RegBytesLocateAllIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReBytesLocateAll)\n"
	                         "\n"
	                         "next->?Ert:ReSubBytes"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequenceIterator,
			/* tp_ctor:        */ &regblaiter_ctor,
			/* tp_copy_ctor:   */ &regblaiter_copy,
			/* tp_deep_ctor:   */ &regblaiter_deep,
			/* tp_any_ctor:    */ &regblaiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &regblaiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&regblaiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&regblaiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&regblaiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &regblaiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&regblaiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ regblaiter_getsets,
	/* .tp_members       = */ regblaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


#define relaiter_ctor      refaiter_ctor
#define relaiter_copy      refaiter_copy
#define relaiter_deep      refaiter_deep
#define relaiter_serialize refaiter_serialize
#define relaiter_fini      refaiter_fini
#define relaiter_bool      refaiter_bool
#define relaiter_cmp       refaiter_cmp
#define relaiter_members   refaiter_members

#define reblaiter_ctor      rebfaiter_ctor
#define reblaiter_copy      rebfaiter_copy
#define reblaiter_deep      rebfaiter_deep
#define reblaiter_serialize rebfaiter_serialize
#define reblaiter_fini      rebfaiter_fini
#define reblaiter_bool      rebfaiter_bool
#define reblaiter_cmp       rebfaiter_cmp
#define reblaiter_members   rebfaiter_members

#define relaiter_init  ReSequenceIterator__init
#define reblaiter_init ReSequenceIterator__init

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
relaiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	ReSequenceIterator_LockRead(self);
	memcpy(&args_copy, &self->rsi_exec, sizeof(struct DeeRegexBaseExec));
	ReSequenceIterator_LockEndRead(self);
	return string_re_locateall((String *)self->rsi_data, &args_copy);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
reblaiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	ReSequenceIterator_LockRead(self);
	memcpy(&args_copy, &self->rsi_exec, sizeof(struct DeeRegexBaseExec));
	ReSequenceIterator_LockEndRead(self);
	return bytes_re_locateall((DeeBytesObject *)self->rsi_data, &args_copy);
}

PRIVATE struct type_getset tpconst relaiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &relaiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:ReLocateAll"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst reblaiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &reblaiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:ReBytesLocateAll"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
relaiter_next(ReSequenceIterator *__restrict self) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	struct DeeRegexCode const *code;
	code = DeeRegexBaseExec_GetCode(&self->rsi_exec);
	if unlikely(!code)
		goto err;
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, code, &exec, 0, NULL);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err;
	if (result == Dee_RE_STATUS_NOMATCH)
		return ITER_DONE;
	if (match_size == 0)
		return ITER_DONE; /* Prevent infinite loop on epsilon-match */
	ReSequenceIterator_LockWrite(self);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		ReSequenceIterator_LockEndWrite(self);
		goto again;
	}
	self->rsi_exec.rx_startoff = (size_t)result + match_size;
	ReSequenceIterator_LockEndWrite(self);
	return DeeString_NewUtf8((char const *)exec.rx_inbase + (size_t)result,
	                         match_size, STRING_ERROR_FSTRICT);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
reblaiter_next(ReSequenceIterator *__restrict self) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	struct DeeRegexCode const *code;
	code = DeeRegexBaseExec_GetCode(&self->rsi_exec);
	if unlikely(!code)
		goto err;
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, code, &exec, 0, NULL);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err;
	if (result == Dee_RE_STATUS_NOMATCH)
		return ITER_DONE;
	if (match_size == 0)
		return ITER_DONE; /* Prevent infinite loop on epsilon-match */
	ReSequenceIterator_LockWrite(self);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		ReSequenceIterator_LockEndWrite(self);
		goto again;
	}
	self->rsi_exec.rx_startoff = (size_t)result + match_size;
	ReSequenceIterator_LockEndWrite(self);
	return DeeBytes_NewSubView(self->rsi_data,
	                           (void *)((char const *)exec.rx_inbase + (size_t)result),
	                           match_size);
err:
	return NULL;
}

INTERN DeeTypeObject ReLocateAllIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReLocateAllIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReLocateAll)\n"
	                         "\n"
	                         "next->?Dstring"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequenceIterator,
			/* tp_ctor:        */ &relaiter_ctor,
			/* tp_copy_ctor:   */ &relaiter_copy,
			/* tp_deep_ctor:   */ &relaiter_deep,
			/* tp_any_ctor:    */ &relaiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &relaiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&relaiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&relaiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&relaiter_fini,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &relaiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&relaiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ relaiter_getsets,
	/* .tp_members       = */ relaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject ReBytesLocateAllIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReBytesLocateAllIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReBytesLocateAll)\n"
	                         "\n"
	                         "next->?DBytes"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequenceIterator,
			/* tp_ctor:        */ &reblaiter_ctor,
			/* tp_copy_ctor:   */ &reblaiter_copy,
			/* tp_deep_ctor:   */ &reblaiter_deep,
			/* tp_any_ctor:    */ &reblaiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &reblaiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&reblaiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&reblaiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&reblaiter_fini,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &reblaiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&reblaiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ reblaiter_getsets,
	/* .tp_members       = */ reblaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


#define respiter_ctor      refaiter_ctor
#define respiter_copy      refaiter_copy
#define respiter_deep      refaiter_deep
#define respiter_serialize refaiter_serialize
#define respiter_fini      refaiter_fini
#define respiter_visit     refaiter_visit
#define respiter_cmp       refaiter_cmp
#define respiter_members   refaiter_members

#define rebspiter_ctor      rebfaiter_ctor
#define rebspiter_copy      rebfaiter_copy
#define rebspiter_deep      rebfaiter_deep
#define rebspiter_serialize rebfaiter_serialize
#define rebspiter_fini      rebfaiter_fini
#define rebspiter_visit     rebfaiter_visit
#define rebspiter_cmp       rebfaiter_cmp
#define rebspiter_members   rebfaiter_members

#define rebspiter_init respiter_init
PRIVATE WUNUSED NONNULL((1)) int DCALL
respiter_init(ReSequenceIterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	int result = ReSequenceIterator__init(self, argc, argv);
	if (!self->rsi_exec.rx_insize)
		self->rsi_exec.rx_inbase = NULL;
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
respiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	ReSequenceIterator_LockRead(self);
	memcpy(&args_copy, &self->rsi_exec, sizeof(args_copy));
	ReSequenceIterator_LockEndRead(self);
	return string_re_split((String *)self->rsi_data, &args_copy);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rebspiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	ReSequenceIterator_LockRead(self);
	memcpy(&args_copy, &self->rsi_exec, sizeof(args_copy));
	ReSequenceIterator_LockEndRead(self);
	return bytes_re_split((DeeBytesObject *)self->rsi_data, &args_copy);
}

PRIVATE struct type_getset tpconst respiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &respiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:ReSplit"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst rebspiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &rebspiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:ReBytesSplit"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
respiter_next(ReSequenceIterator *__restrict self) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	struct DeeRegexCode const *code;
	code = DeeRegexBaseExec_GetCode(&self->rsi_exec);
	if unlikely(!code)
		goto err;
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, code, &exec, 0, NULL);
	ReSequenceIterator_LockEndRead(self);
	if (!exec.rx_inbase)
		return ITER_DONE;
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err;
	ReSequenceIterator_LockWrite(self);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		ReSequenceIterator_LockEndWrite(self);
		goto again;
	}
	if (result == Dee_RE_STATUS_NOMATCH ||
	    match_size == 0) { /* Prevent infinite loop on epsilon-match */
		self->rsi_exec.rx_startoff = (size_t)-1;
		self->rsi_exec.rx_inbase   = NULL;
		result = exec.rx_endoff;
	} else {
		self->rsi_exec.rx_startoff = (size_t)result + match_size;
	}
	ReSequenceIterator_LockEndWrite(self);
	return DeeString_NewUtf8((char const *)exec.rx_inbase + exec.rx_startoff,
	                         (size_t)result - exec.rx_startoff,
	                         STRING_ERROR_FSTRICT);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rebspiter_next(ReSequenceIterator *__restrict self) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	struct DeeRegexCode const *code;
	code = DeeRegexBaseExec_GetCode(&self->rsi_exec);
	if unlikely(!code)
		goto err;
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, code, &exec, 0, NULL);
	ReSequenceIterator_LockEndRead(self);
	if (!exec.rx_inbase)
		return ITER_DONE;
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err;
	ReSequenceIterator_LockWrite(self);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		ReSequenceIterator_LockEndWrite(self);
		goto again;
	}
	if (result == Dee_RE_STATUS_NOMATCH ||
	    match_size == 0) { /* Prevent infinite loop on epsilon-match */
		self->rsi_exec.rx_startoff = (size_t)-1;
		self->rsi_exec.rx_inbase   = NULL;
		result = exec.rx_endoff;
	} else {
		self->rsi_exec.rx_startoff = (size_t)result + match_size;
	}
	ReSequenceIterator_LockEndWrite(self);
	return DeeBytes_NewSubView(self->rsi_data,
	                           (void *)((char const *)exec.rx_inbase + exec.rx_startoff),
	                           (size_t)result - exec.rx_startoff);
err:
	return NULL;
}

#define rebspiter_bool respiter_bool
PRIVATE WUNUSED NONNULL((1)) int DCALL
respiter_bool(ReSequenceIterator *__restrict self) {
	return atomic_read(&self->rsi_exec.rx_inbase) != NULL;
}


INTERN DeeTypeObject ReSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReSplitIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReSplit)\n"
	                         "\n"
	                         "next->?Dstring"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequenceIterator,
			/* tp_ctor:        */ &respiter_ctor,
			/* tp_copy_ctor:   */ &respiter_copy,
			/* tp_deep_ctor:   */ &respiter_deep,
			/* tp_any_ctor:    */ &respiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &respiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&respiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&respiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&respiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &respiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&respiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ respiter_getsets,
	/* .tp_members       = */ respiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject ReBytesSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReBytesSplitIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReBytesSplit)\n"
	                         "\n"
	                         "next->?DBytes"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequenceIterator,
			/* tp_ctor:        */ &rebspiter_ctor,
			/* tp_copy_ctor:   */ &rebspiter_copy,
			/* tp_deep_ctor:   */ &rebspiter_deep,
			/* tp_any_ctor:    */ &rebspiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rebspiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rebspiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rebspiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rebspiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &rebspiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rebspiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ rebspiter_getsets,
	/* .tp_members       = */ rebspiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
refa_ctor(ReSequence *__restrict self) {
	bzero(&self->rs_exec, sizeof(self->rs_exec));
	self->rs_exec.rx_code = DeeString_GetRegex(Dee_EmptyString, Dee_RE_COMPILE_NORMAL, NULL);
	if unlikely(!self->rs_exec.rx_code)
		return -1;
	self->rs_data = Dee_EmptyString;
	self->rs_exec.rx_pattern = (DREF String *)Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	return 0;
}

#define refa_copy       ReSequence__copy
#define refa_deep       ReSequence__deep
#define refa_serialize  ReSequence__serialize
#define rebfa_copy      ReSequence__copy
#define rebfa_deep      ReSequence__deep /* TODO: Should actually deep-copy the underlying Bytes (since those are mutable...) */
#define rebfa_serialize ReSequence__serialize

PRIVATE WUNUSED NONNULL((1)) int DCALL
rebfa_ctor(ReSequence *__restrict self) {
	bzero(&self->rs_exec, sizeof(self->rs_exec));
	self->rs_exec.rx_code = DeeString_GetRegex(Dee_EmptyString, Dee_RE_COMPILE_NORMAL, NULL);
	if unlikely(!self->rs_exec.rx_code)
		return -1;
	self->rs_data = DeeBytes_NewEmpty();
	self->rs_exec.rx_pattern = (DREF String *)DeeString_NewEmpty();
	return 0;
}

#define refa_fini  refaiter_fini
#define refa_visit refaiter_visit

#define rebfa_fini  rebfaiter_fini
#define rebfa_visit rebfaiter_visit

#define rebfa_bool refa_bool
PRIVATE WUNUSED NONNULL((1)) int DCALL
refa_bool(ReSequence *__restrict self) {
	/* Check if there is at least a single match. */
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	struct DeeRegexCode const *code;
	code = DeeRegexBaseExec_GetCode(&self->rs_exec);
	if unlikely(!code)
		goto err;
	DeeRegexBaseExec_Load(&self->rs_exec, code, &exec, 0, NULL);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == Dee_RE_STATUS_ERROR)
		goto err;
	return result != Dee_RE_STATUS_NOMATCH &&
	       match_size != 0; /* Prevent infinite loop on epsilon-match */
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF ReSequenceIterator *DCALL
refa_iter(ReSequence *__restrict self) {
	DREF ReSequenceIterator *result;
	result = DeeObject_MALLOC(ReSequenceIterator);
	if unlikely(!result)
		goto done;
	memcpy(&result->rsi_data, &self->rs_data,
	       sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	Dee_Incref(result->rsi_data);
	Dee_Incref(result->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&result->rsi_lock);
	DeeObject_Init(result, &ReFindAllIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF ReSequenceIterator *DCALL
rebfa_iter(ReSequence *__restrict self) {
	DREF ReSequenceIterator *result;
	result = DeeObject_MALLOC(ReSequenceIterator);
	if unlikely(!result)
		goto done;
	memcpy(&result->rsi_data, &self->rs_data,
	       sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	Dee_Incref(result->rsi_data);
	Dee_Incref(result->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&result->rsi_lock);
	DeeObject_Init(result, &ReBytesFindAllIterator_Type);
done:
	return result;
}

#define rebfa_size refa_size
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
refa_size(ReSequence *__restrict self) {
	size_t count = 0;
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	struct DeeRegexCode const *code;
	code = DeeRegexBaseExec_GetCode(&self->rs_exec);
	if unlikely(!code)
		goto err;
	DeeRegexBaseExec_Load(&self->rs_exec, code, &exec, 0, NULL);
	/* Count the # of matches */
	while (exec.rx_startoff < exec.rx_endoff) {
		result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
		if unlikely(result == Dee_RE_STATUS_ERROR)
			goto err;
		if (result == Dee_RE_STATUS_NOMATCH)
			break;
		if (match_size == 0)
			break; /* Prevent infinite loop on epsilon-match */
		exec.rx_startoff = (size_t)result + match_size;
		++count;
	}
	return count;
err:
	return (size_t)-1;
}

PRIVATE struct type_seq refa_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&refa_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_hasitem_index),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&refa_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index      = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index    = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_size),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem         = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index   = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_seq rebfa_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rebfa_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_hasitem_index),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&rebfa_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index      = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index    = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_size),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem         = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index   = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_member tpconst refa_members[] = {
#define rebfa_members refa_members
	TYPE_MEMBER_FIELD_DOC("__input__", STRUCT_OBJECT, offsetof(ReSequence, rs_data), "->?X2?Dstring?DBytes"),
	TYPE_MEMBER_FIELD_DOC("__pattern__", STRUCT_OBJECT, offsetof(ReSequence, rs_exec.rx_pattern), "->?Dstring"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReSequence, rs_exec.rx_startoff)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReSequence, rs_exec.rx_endoff)),
	TYPE_MEMBER_BITFIELD("__notbol__", STRUCT_CONST, ReSequence, rs_exec.rx_eflags, Dee_RE_EXEC_NOTBOL),
	TYPE_MEMBER_BITFIELD("__noteol__", STRUCT_CONST, ReSequence, rs_exec.rx_eflags, Dee_RE_EXEC_NOTEOL),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst refa_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ReFindAllIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rebfa_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ReBytesFindAllIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject ReFindAll_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReFindAll",
	/* .tp_doc      = */ DOC("getitem->?X2?T2?Dint?Dint?N"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequence,
			/* tp_ctor:        */ &refa_ctor,
			/* tp_copy_ctor:   */ &refa_copy,
			/* tp_deep_ctor:   */ &refa_deep,
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &refa_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&refa_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&refa_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&refa_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &refa_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ refa_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ refa_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject ReBytesFindAll_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReBytesFindAll",
	/* .tp_doc      = */ DOC("getitem->?X2?T2?Dint?Dint?N"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequence,
			/* tp_ctor:        */ &rebfa_ctor,
			/* tp_copy_ctor:   */ &rebfa_copy,
			/* tp_deep_ctor:   */ &rebfa_deep,
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rebfa_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rebfa_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rebfa_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rebfa_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &rebfa_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rebfa_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rebfa_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


#define regfa_ctor      refa_ctor
#define regfa_copy      refa_copy
#define regfa_deep      refa_deep
#define regfa_serialize refa_serialize
#define regfa_fini      refa_fini
#define regfa_bool      refa_bool
#define regfa_visit     refa_visit
#define regfa_size      refa_size
#define regfa_members   refa_members

#define regbfa_ctor      rebfa_ctor
#define regbfa_copy      rebfa_copy
#define regbfa_deep      rebfa_deep
#define regbfa_serialize rebfa_serialize
#define regbfa_fini      rebfa_fini
#define regbfa_bool      rebfa_bool
#define regbfa_visit     rebfa_visit
#define regbfa_size      rebfa_size
#define regbfa_members   rebfa_members

PRIVATE WUNUSED NONNULL((1)) DREF ReSequenceIterator *DCALL
regfa_iter(ReSequence *__restrict self) {
	DREF ReSequenceIterator *result;
	result = DeeObject_MALLOC(ReSequenceIterator);
	if unlikely(!result)
		goto done;
	memcpy(&result->rsi_data, &self->rs_data,
	       sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	Dee_Incref(result->rsi_data);
	Dee_Incref(result->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&result->rsi_lock);
	DeeObject_Init(result, &RegFindAllIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF ReSequenceIterator *DCALL
regbfa_iter(ReSequence *__restrict self) {
	DREF ReSequenceIterator *result;
	result = DeeObject_MALLOC(ReSequenceIterator);
	if unlikely(!result)
		goto done;
	memcpy(&result->rsi_data, &self->rs_data,
	       sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	Dee_Incref(result->rsi_data);
	Dee_Incref(result->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&result->rsi_lock);
	DeeObject_Init(result, &RegBytesFindAllIterator_Type);
done:
	return result;
}

PRIVATE struct type_seq regfa_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&regfa_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_hasitem_index),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&regfa_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index      = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index    = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_size),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem         = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index   = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_seq regbfa_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&regbfa_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_hasitem_index),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&regbfa_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index      = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index    = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_size),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem         = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index   = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_member tpconst regfa_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &RegFindAllIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &ReGroups_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst regbfa_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &RegBytesFindAllIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &ReGroups_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject RegFindAll_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RegFindAll",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequence,
			/* tp_ctor:        */ &regfa_ctor,
			/* tp_copy_ctor:   */ &regfa_copy,
			/* tp_deep_ctor:   */ &regfa_deep,
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &regfa_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&regfa_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&regfa_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&regfa_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &regfa_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ regfa_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ regfa_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject RegBytesFindAll_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RegBytesFindAll",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequence,
			/* tp_ctor:        */ &regbfa_ctor,
			/* tp_copy_ctor:   */ &regbfa_copy,
			/* tp_deep_ctor:   */ &regbfa_deep,
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &regbfa_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&regbfa_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&regbfa_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&regbfa_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &regbfa_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ regbfa_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ regbfa_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};



/* LocateAll wrapper (based on FindAll) */
#define regla_ctor      regfa_ctor
#define regla_copy      regfa_copy
#define regla_deep      regfa_deep
#define regla_serialize regfa_serialize
#define regla_fini      regfa_fini
#define regla_bool      regfa_bool
#define regla_visit     regfa_visit
#define regla_members   regfa_members
#define regla_size      regfa_size

#define regbla_ctor      regbfa_ctor
#define regbla_copy      regbfa_copy
#define regbla_deep      regbfa_deep
#define regbla_serialize regbfa_serialize
#define regbla_fini      regbfa_fini
#define regbla_bool      regbfa_bool
#define regbla_visit     regbfa_visit
#define regbla_members   regbfa_members
#define regbla_size      regbfa_size


PRIVATE WUNUSED NONNULL((1)) DREF ReSequenceIterator *DCALL
regla_iter(ReSequence *__restrict self) {
	DREF ReSequenceIterator *result;
	result = DeeObject_MALLOC(ReSequenceIterator);
	if unlikely(!result)
		goto done;
	memcpy(&result->rsi_data, &self->rs_data,
	       sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	Dee_Incref(result->rsi_data);
	Dee_Incref(result->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&result->rsi_lock);
	DeeObject_Init(result, &RegLocateAllIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF ReSequenceIterator *DCALL
regbla_iter(ReSequence *__restrict self) {
	DREF ReSequenceIterator *result;
	result = DeeObject_MALLOC(ReSequenceIterator);
	if unlikely(!result)
		goto done;
	memcpy(&result->rsi_data, &self->rs_data,
	       sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	Dee_Incref(result->rsi_data);
	Dee_Incref(result->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&result->rsi_lock);
	DeeObject_Init(result, &RegBytesLocateAllIterator_Type);
done:
	return result;
}

PRIVATE struct type_seq regla_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&regla_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_hasitem_index),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&regla_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_size),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_seq regbla_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&regbla_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_hasitem_index),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&regbla_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_size),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_member tpconst regla_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &RegLocateAllIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &ReSubStrings_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst regbla_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &RegBytesLocateAllIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &ReSubBytes_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject RegLocateAll_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RegLocateAll",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequence,
			/* tp_ctor:        */ &regla_ctor,
			/* tp_copy_ctor:   */ &regla_copy,
			/* tp_deep_ctor:   */ &regla_deep,
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &regla_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&regla_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&regla_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&regla_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &regla_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ regla_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ regla_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject RegBytesLocateAll_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RegBytesLocateAll",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequence,
			/* tp_ctor:        */ &regbla_ctor,
			/* tp_copy_ctor:   */ &regbla_copy,
			/* tp_deep_ctor:   */ &regbla_deep,
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &regbla_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&regbla_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&regbla_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&regbla_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &regbla_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ regbla_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ regbla_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


#define rela_ctor      refa_ctor
#define rela_copy      refa_copy
#define rela_deep      refa_deep
#define rela_serialize refa_serialize
#define rela_fini      refa_fini
#define rela_visit     refa_visit
#define rela_bool      refa_bool
#define rela_members   refa_members
#define rela_size      refa_size

#define rebla_ctor      rebfa_ctor
#define rebla_copy      rebfa_copy
#define rebla_deep      rebfa_deep
#define rebla_serialize rebfa_serialize
#define rebla_fini      rebfa_fini
#define rebla_visit     rebfa_visit
#define rebla_bool      rebfa_bool
#define rebla_members   rebfa_members
#define rebla_size      rebfa_size

PRIVATE WUNUSED NONNULL((1)) DREF ReSequenceIterator *DCALL
rela_iter(ReSequence *__restrict self) {
	DREF ReSequenceIterator *result;
	result = DeeObject_MALLOC(ReSequenceIterator);
	if unlikely(!result)
		goto done;
	memcpy(&result->rsi_data, &self->rs_data,
	       sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	Dee_Incref(result->rsi_data);
	Dee_Incref(result->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&result->rsi_lock);
	DeeObject_Init(result, &ReLocateAllIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF ReSequenceIterator *DCALL
rebla_iter(ReSequence *__restrict self) {
	DREF ReSequenceIterator *result;
	result = DeeObject_MALLOC(ReSequenceIterator);
	if unlikely(!result)
		goto done;
	memcpy(&result->rsi_data, &self->rs_data,
	       sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	Dee_Incref(result->rsi_data);
	Dee_Incref(result->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&result->rsi_lock);
	DeeObject_Init(result, &ReBytesLocateAllIterator_Type);
done:
	return result;
}

PRIVATE struct type_seq rela_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rela_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_hasitem_index),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&rela_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index      = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index    = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_size),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem         = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index   = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_seq rebla_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rebla_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_hasitem_index),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&rebla_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index      = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index    = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_size),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem         = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index   = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_member tpconst rela_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ReLocateAllIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeString_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rebla_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ReBytesLocateAllIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeBytes_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject ReLocateAll_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReLocateAll",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequence,
			/* tp_ctor:        */ &rela_ctor,
			/* tp_copy_ctor:   */ &rela_copy,
			/* tp_deep_ctor:   */ &rela_deep,
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rela_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rela_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rela_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rela_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &rela_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rela_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rela_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject ReBytesLocateAll_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReBytesLocateAll",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequence,
			/* tp_ctor:        */ &rebla_ctor,
			/* tp_copy_ctor:   */ &rebla_copy,
			/* tp_deep_ctor:   */ &rebla_deep,
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rebla_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rebla_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rebla_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rebla_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &rebla_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rebla_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rebla_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

#define resp_ctor      refa_ctor
#define resp_copy      refa_copy
#define resp_deep      refa_deep
#define resp_serialize refa_serialize
#define resp_fini      refa_fini
#define resp_visit     refa_visit
#define resp_members   refa_members

#define rebsp_ctor      rebfa_ctor
#define rebsp_copy      rebfa_copy
#define rebsp_deep      rebfa_deep
#define rebsp_serialize rebfa_serialize
#define rebsp_fini      rebfa_fini
#define rebsp_visit     rebfa_visit
#define rebsp_members   rebfa_members

#define rebsp_bool resp_bool
PRIVATE WUNUSED NONNULL((1)) int DCALL
resp_bool(ReSequence *__restrict self) {
	return self->rs_exec.rx_inbase != NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF ReSequenceIterator *DCALL
resp_iter(ReSequence *__restrict self) {
	DREF ReSequenceIterator *result;
	result = DeeObject_MALLOC(ReSequenceIterator);
	if unlikely(!result)
		goto done;
	memcpy(&result->rsi_data, &self->rs_data,
	       sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	ASSERT((result->rsi_exec.rx_endoff >
	        result->rsi_exec.rx_startoff)
	       ? 1
	       : result->rsi_exec.rx_inbase == NULL);
	Dee_Incref(result->rsi_data);
	Dee_Incref(result->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&result->rsi_lock);
	DeeObject_Init(result, &ReSplitIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF ReSequenceIterator *DCALL
rebsp_iter(ReSequence *__restrict self) {
	DREF ReSequenceIterator *result;
	result = DeeObject_MALLOC(ReSequenceIterator);
	if unlikely(!result)
		goto done;
	memcpy(&result->rsi_data, &self->rs_data,
	       sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	ASSERT((result->rsi_exec.rx_endoff >
	        result->rsi_exec.rx_startoff)
	       ? 1
	       : result->rsi_exec.rx_inbase == NULL);
	Dee_Incref(result->rsi_data);
	Dee_Incref(result->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&result->rsi_lock);
	DeeObject_Init(result, &ReBytesSplitIterator_Type);
done:
	return result;
}

#define rebsp_size resp_size
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
resp_size(ReSequence *__restrict self) {
	size_t result;
	if (!self->rs_exec.rx_inbase)
		return 0;
	result = refa_size(self);
	if (result != (size_t)-1)
		++result;
	return result;
}

PRIVATE struct type_seq resp_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&resp_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_hasitem_index),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&resp_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index      = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index    = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_size),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem         = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index   = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_seq rebsp_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rebsp_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_hasitem_index),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&rebsp_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index      = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index    = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_size),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem         = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index   = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_member tpconst resp_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ReSplitIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeString_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rebsp_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ReBytesSplitIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeBytes_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject ReSplit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReSplit",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequence,
			/* tp_ctor:        */ &resp_ctor,
			/* tp_copy_ctor:   */ &resp_copy,
			/* tp_deep_ctor:   */ &resp_deep,
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &resp_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&resp_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&resp_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&resp_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &resp_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ resp_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ resp_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject ReBytesSplit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReBytesSplit",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ReSequence,
			/* tp_ctor:        */ &rebsp_ctor,
			/* tp_copy_ctor:   */ &rebsp_copy,
			/* tp_deep_ctor:   */ &rebsp_deep,
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rebsp_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rebsp_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rebsp_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rebsp_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &rebsp_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rebsp_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rebsp_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};




INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_re_findall(String *__restrict self,
                  struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = Dee_AsObject(self);
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &ReFindAll_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_reg_findall(String *__restrict self,
                   struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = Dee_AsObject(self);
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &RegFindAll_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_reg_locateall(String *__restrict self,
                     struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = Dee_AsObject(self);
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &RegLocateAll_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_re_locateall(String *__restrict self,
                    struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = Dee_AsObject(self);
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &ReLocateAll_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_re_split(String *__restrict self,
                struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = Dee_AsObject(self);
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	if (result->rs_exec.rx_startoff >= result->rs_exec.rx_endoff)
		result->rs_exec.rx_inbase = NULL;
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &ReSplit_Type);
done:
	return Dee_AsObject(result);
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_findall(DeeBytesObject *__restrict self,
                 struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = Dee_AsObject(self);
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &ReBytesFindAll_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_reg_findall(DeeBytesObject *__restrict self,
                  struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = Dee_AsObject(self);
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &RegBytesFindAll_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_reg_locateall(DeeBytesObject *__restrict self,
                    struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = Dee_AsObject(self);
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &RegBytesLocateAll_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_locateall(DeeBytesObject *__restrict self,
                   struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = Dee_AsObject(self);
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &ReBytesLocateAll_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_split(DeeBytesObject *__restrict self,
               struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = Dee_AsObject(self);
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	if (result->rs_exec.rx_startoff >= result->rs_exec.rx_endoff)
		result->rs_exec.rx_inbase = NULL;
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &ReBytesSplit_Type);
done:
	return Dee_AsObject(result);
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_C_INL */
