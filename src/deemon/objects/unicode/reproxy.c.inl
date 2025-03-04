/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_C_INL 1

#ifdef __INTELLISENSE__
#include "string_functions.c"
#endif /* !__INTELLISENSE__ */

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bytes.h>
#include <deemon/computed-operators.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/regex.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memcpy() */
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "regroups.h"
#include "string_functions.h"
/**/

#include <stddef.h> /* size_t, offsetof */


/* Proxy sequence objects for `string.refindall',
 * `string.relocateall' and `string.resplit' */

DECL_BEGIN

INTDEF DeeTypeObject ReFindAll_Type;
INTDEF DeeTypeObject ReFindAllIterator_Type;
INTDEF DeeTypeObject RegFindAll_Type;
INTDEF DeeTypeObject RegFindAllIterator_Type;
INTDEF DeeTypeObject ReLocateAll_Type;
INTDEF DeeTypeObject ReLocateAllIterator_Type;
INTDEF DeeTypeObject ReSplit_Type;
INTDEF DeeTypeObject ReSplitIterator_Type;
INTDEF DeeTypeObject ReBytesFindAll_Type;
INTDEF DeeTypeObject ReBytesFindAllIterator_Type;
INTDEF DeeTypeObject RegBytesFindAll_Type;
INTDEF DeeTypeObject RegBytesFindAllIterator_Type;
INTDEF DeeTypeObject ReBytesLocateAll_Type;
INTDEF DeeTypeObject ReBytesLocateAllIterator_Type;
INTDEF DeeTypeObject ReBytesSplit_Type;
INTDEF DeeTypeObject ReBytesSplitIterator_Type;

#define DeeRegexBaseExec_Load(self, result, nmatch, pmatch) \
	(void)((result)->rx_code     = (self)->rx_code,         \
	       (result)->rx_inbase   = (self)->rx_inbase,       \
	       (result)->rx_insize   = (self)->rx_insize,       \
	       (result)->rx_startoff = (self)->rx_startoff,     \
	       (result)->rx_endoff   = (self)->rx_endoff,       \
	       (result)->rx_eflags   = (self)->rx_eflags,       \
	       (result)->rx_nmatch   = (nmatch),                \
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

STATIC_ASSERT(offsetof(ReSequence, rs_data) == offsetof(ReSequenceIterator, rsi_data));
STATIC_ASSERT(offsetof(ReSequence, rs_exec) == offsetof(ReSequenceIterator, rsi_exec));



PRIVATE WUNUSED NONNULL((1)) int DCALL
refaiter_ctor(ReSequenceIterator *__restrict self) {
	bzero(&self->rsi_exec, sizeof(self->rsi_exec));
	self->rsi_exec.rx_code = DeeString_GetRegex(Dee_EmptyString, DEE_REGEX_COMPILE_NORMAL, NULL);
	if unlikely(!self->rsi_exec.rx_code)
		return -1;
	self->rsi_data            = Dee_EmptyString;
	self->rsi_exec.rx_pattern = (DREF String *)Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	Dee_atomic_rwlock_init(&self->rsi_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rebfaiter_ctor(ReSequenceIterator *__restrict self) {
	bzero(&self->rsi_exec, sizeof(self->rsi_exec));
	self->rsi_exec.rx_code = DeeString_GetRegex(Dee_EmptyString, DEE_REGEX_COMPILE_NORMAL, NULL);
	if unlikely(!self->rsi_exec.rx_code)
		return -1;
	self->rsi_data            = Dee_EmptyBytes;
	self->rsi_exec.rx_pattern = (DREF String *)Dee_EmptyString;
	Dee_Incref(Dee_EmptyBytes);
	Dee_Incref(Dee_EmptyString);
	Dee_atomic_rwlock_init(&self->rsi_lock);
	return 0;
}

#define rebfaiter_copy refaiter_copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
refaiter_copy(ReSequenceIterator *__restrict self,
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
refaiter_init(ReSequenceIterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	ReSequence *reseq;
	if (DeeArg_Unpack(argc, argv, "o:_ReFindAllIterator", &reseq))
		goto err;
	if (DeeObject_AssertTypeExact(reseq, &ReFindAll_Type))
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
rebfaiter_init(ReSequenceIterator *__restrict self,
               size_t argc, DeeObject *const *argv) {
	ReSequence *reseq;
	if (DeeArg_Unpack(argc, argv, "o:_ReBytesFindAllIterator", &reseq))
		goto err;
	if (DeeObject_AssertTypeExact(reseq, &ReBytesFindAll_Type))
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

STATIC_ASSERT(offsetof(ReSequenceIterator, rsi_data) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(ReSequenceIterator, rsi_data) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(ReSequenceIterator, rsi_exec.rx_pattern) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(ReSequenceIterator, rsi_exec.rx_pattern) == offsetof(ProxyObject2, po_obj2));
#define rebfaiter_fini  generic_proxy2__fini
#define refaiter_fini   generic_proxy2__fini
#define rebfaiter_visit generic_proxy2__visit
#define refaiter_visit  generic_proxy2__visit

#define rebfaiter_bool refaiter_bool
PRIVATE WUNUSED NONNULL((1)) int DCALL
refaiter_bool(ReSequenceIterator *__restrict self) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, &exec, 0, NULL);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	return result != DEE_RE_STATUS_NOMATCH &&
	       match_size != 0; /* Prevent infinite loop on epsilon-match */
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
refaiter_nextpair(ReSequenceIterator *__restrict self, DREF DeeObject *pair[2]) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, &exec, 0, NULL);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
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
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, &exec, 0, NULL);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
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
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&refa_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&refa_compare,
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

#define rebfaiter_members refaiter_members
PRIVATE struct type_member tpconst refaiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__input__", STRUCT_OBJECT, offsetof(ReSequenceIterator, rsi_data), "->?X2?Dstring?DBytes"),
	TYPE_MEMBER_FIELD_DOC("__pattern__", STRUCT_OBJECT, offsetof(ReSequenceIterator, rsi_exec.rx_pattern), "->?Dstring"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReSequenceIterator, rsi_exec.rx_startoff)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReSequenceIterator, rsi_exec.rx_endoff)),
	TYPE_MEMBER_BITFIELD("__notbol__", STRUCT_CONST, ReSequenceIterator, rsi_exec.rx_eflags, DEE_RE_EXEC_NOTBOL),
	TYPE_MEMBER_BITFIELD("__noteol__", STRUCT_CONST, ReSequenceIterator, rsi_exec.rx_eflags, DEE_RE_EXEC_NOTEOL),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject ReFindAllIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReFindAllIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReFindAll)\n"
	                         "\n"
	                         "iter->?X2?Dint?Dint"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&refaiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&refaiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&refaiter_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&refaiter_init,
				TYPE_FIXED_ALLOCATOR(ReSequenceIterator)
			}
		},
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
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&refaiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &refaiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &refaiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ refaiter_getsets,
	/* .tp_members       = */ refaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};

INTERN DeeTypeObject ReBytesFindAllIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReBytesFindAllIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReBytesFindAll)\n"
	                         "\n"
	                         "iter->?X2?Dint?Dint"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rebfaiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&rebfaiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&rebfaiter_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&rebfaiter_init,
				TYPE_FIXED_ALLOCATOR(ReSequenceIterator)
			}
		},
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
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rebfaiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &rebfaiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &rebfaiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ rebfaiter_getsets,
	/* .tp_members       = */ rebfaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};


#define regfaiter_ctor    refaiter_ctor
#define regfaiter_copy    refaiter_copy
#define regfaiter_init    refaiter_init
#define regfaiter_fini    refaiter_fini
#define regfaiter_bool    refaiter_bool
#define regfaiter_visit   refaiter_visit
#define regfaiter_cmp     refaiter_cmp
#define regfaiter_members refaiter_members

#define regbfaiter_ctor    rebfaiter_ctor
#define regbfaiter_copy    rebfaiter_copy
#define regbfaiter_init    rebfaiter_init
#define regbfaiter_fini    rebfaiter_fini
#define regbfaiter_bool    rebfaiter_bool
#define regbfaiter_visit   rebfaiter_visit
#define regbfaiter_cmp     rebfaiter_cmp
#define regbfaiter_members rebfaiter_members

PRIVATE WUNUSED NONNULL((1)) DREF ReGroups *DCALL
regfaiter_next(ReSequenceIterator *__restrict self) {
	DREF ReGroups *groups;
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	groups = ReGroups_Malloc(1 + self->rsi_exec.rx_code->rc_ngrps);
	if unlikely(!groups)
		goto err;
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, &exec,
	                      self->rsi_exec.rx_code->rc_ngrps,
	                      groups->rg_groups + 1);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err_g;
	if (result == DEE_RE_STATUS_NOMATCH)
		return (DREF ReGroups *)ITER_DONE;
	if (match_size == 0)
		return (DREF ReGroups *)ITER_DONE; /* Prevent infinite loop on epsilon-match */
	ReSequenceIterator_LockWrite(self);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		ReSequenceIterator_LockEndWrite(self);
		goto again;
	}
	self->rsi_exec.rx_startoff = (size_t)result + match_size;
	ReSequenceIterator_LockEndWrite(self);
	string_bytecnt2charcnt_v((String *)self->rsi_data, (char const *)exec.rx_inbase,
	                         groups->rg_groups + 1, self->rsi_exec.rx_code->rc_ngrps);
	match_size = string_bytecnt2charcnt((String *)self->rsi_data, (size_t)match_size, (char const *)exec.rx_inbase + (size_t)result);
	result     = string_bytecnt2charcnt((String *)self->rsi_data, (size_t)result, (char const *)exec.rx_inbase);
	match_size += (size_t)result;
	groups->rg_groups[0].rm_so = result;
	groups->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(groups, 1 + self->rsi_exec.rx_code->rc_ngrps);
	return groups;
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
	groups = ReGroups_Malloc(1 + self->rsi_exec.rx_code->rc_ngrps);
	if unlikely(!groups)
		goto err;
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, &exec,
	                      self->rsi_exec.rx_code->rc_ngrps,
	                      groups->rg_groups + 1);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err_g;
	if (result == DEE_RE_STATUS_NOMATCH)
		return (DREF ReGroups *)ITER_DONE;
	if (match_size == 0)
		return (DREF ReGroups *)ITER_DONE; /* Prevent infinite loop on epsilon-match */
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
	ReGroups_Init(groups, 1 + self->rsi_exec.rx_code->rc_ngrps);
	return groups;
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
	                         "iter->?Ert:ReGroups"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&regfaiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&regfaiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&regfaiter_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&regfaiter_init,
				TYPE_FIXED_ALLOCATOR(ReSequenceIterator)
			}
		},
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
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&regfaiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &regfaiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&regfaiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ regfaiter_getsets,
	/* .tp_members       = */ regfaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};

INTERN DeeTypeObject RegBytesFindAllIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RegBytesFindAllIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReBytesFindAll)\n"
	                         "\n"
	                         "iter->?Ert:ReGroups"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&regbfaiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&regbfaiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&regbfaiter_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&regbfaiter_init,
				TYPE_FIXED_ALLOCATOR(ReSequenceIterator)
			}
		},
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
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&regbfaiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &regbfaiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&regbfaiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ regbfaiter_getsets,
	/* .tp_members       = */ regbfaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};


#define relaiter_ctor    refaiter_ctor
#define relaiter_copy    refaiter_copy
#define relaiter_fini    refaiter_fini
#define relaiter_bool    refaiter_bool
#define relaiter_cmp     refaiter_cmp
#define relaiter_members refaiter_members

#define reblaiter_ctor    refaiter_ctor
#define reblaiter_copy    refaiter_copy
#define reblaiter_fini    refaiter_fini
#define reblaiter_bool    refaiter_bool
#define reblaiter_cmp     refaiter_cmp
#define reblaiter_members refaiter_members

PRIVATE WUNUSED NONNULL((1)) int DCALL
relaiter_init(ReSequenceIterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	ReSequence *reseq;
	if (DeeArg_Unpack(argc, argv, "o:_ReLocateAllIterator", &reseq))
		goto err;
	if (DeeObject_AssertTypeExact(reseq, &ReLocateAll_Type))
		goto err;
	memcpy(&self->rsi_data, &reseq->rs_data, sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	Dee_Incref(self->rsi_data);
	Dee_Incref(self->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&self->rsi_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
reblaiter_init(ReSequenceIterator *__restrict self,
               size_t argc, DeeObject *const *argv) {
	ReSequence *reseq;
	if (DeeArg_Unpack(argc, argv, "o:_ReBytesLocateAllIterator", &reseq))
		goto err;
	if (DeeObject_AssertTypeExact(reseq, &ReBytesLocateAll_Type))
		goto err;
	memcpy(&self->rsi_data, &reseq->rs_data, sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	Dee_Incref(self->rsi_data);
	Dee_Incref(self->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&self->rsi_lock);
	return 0;
err:
	return -1;
}


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
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, &exec, 0, NULL);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
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
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, &exec, 0, NULL);
	ReSequenceIterator_LockEndRead(self);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
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
	                         "iter->?Dstring"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&relaiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&relaiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&relaiter_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&relaiter_init,
				TYPE_FIXED_ALLOCATOR(ReSequenceIterator)
			}
		},
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
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&relaiter_fini,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &relaiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&relaiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ relaiter_getsets,
	/* .tp_members       = */ relaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};

INTERN DeeTypeObject ReBytesLocateAllIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReBytesLocateAllIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReBytesLocateAll)\n"
	                         "\n"
	                         "iter->?DBytes"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&reblaiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&reblaiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&reblaiter_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&reblaiter_init,
				TYPE_FIXED_ALLOCATOR(ReSequenceIterator)
			}
		},
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
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&reblaiter_fini,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &reblaiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&reblaiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ reblaiter_getsets,
	/* .tp_members       = */ reblaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};


#define respiter_ctor    refaiter_ctor
#define respiter_copy    refaiter_copy
#define respiter_fini    refaiter_fini
#define respiter_visit   refaiter_visit
#define respiter_cmp     refaiter_cmp
#define respiter_members refaiter_members

#define rebspiter_ctor    refaiter_ctor
#define rebspiter_copy    refaiter_copy
#define rebspiter_fini    refaiter_fini
#define rebspiter_visit   refaiter_visit
#define rebspiter_cmp     refaiter_cmp
#define rebspiter_members refaiter_members

PRIVATE WUNUSED NONNULL((1)) int DCALL
respiter_init(ReSequenceIterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	ReSequence *reseq;
	if (DeeArg_Unpack(argc, argv, "o:_ReSplitIterator", &reseq))
		goto err;
	if (DeeObject_AssertTypeExact(reseq, &ReSplit_Type))
		goto err;
	memcpy(&self->rsi_data, &reseq->rs_data,
	       sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	if (!self->rsi_exec.rx_insize)
		self->rsi_exec.rx_inbase = NULL;
	Dee_Incref(self->rsi_data);
	Dee_Incref(self->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&self->rsi_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rebspiter_init(ReSequenceIterator *__restrict self,
               size_t argc, DeeObject *const *argv) {
	ReSequence *reseq;
	if (DeeArg_Unpack(argc, argv, "o:_ReBytesSplitIterator", &reseq))
		goto err;
	if (DeeObject_AssertTypeExact(reseq, &ReBytesSplit_Type))
		goto err;
	memcpy(&self->rsi_data, &reseq->rs_data,
	       sizeof(ReSequence) - offsetof(ReSequence, rs_data));
	if (!self->rsi_exec.rx_insize)
		self->rsi_exec.rx_inbase = NULL;
	Dee_Incref(self->rsi_data);
	Dee_Incref(self->rsi_exec.rx_pattern);
	Dee_atomic_rwlock_init(&self->rsi_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
respiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	ReSequenceIterator_LockRead(self);
	memcpy(&args_copy, self->rsi_data, sizeof(struct DeeRegexBaseExec));
	ReSequenceIterator_LockEndRead(self);
	return string_re_split((String *)self->rsi_data, &args_copy);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rebspiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	ReSequenceIterator_LockRead(self);
	memcpy(&args_copy, self->rsi_data, sizeof(struct DeeRegexBaseExec));
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
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, &exec, 0, NULL);
	ReSequenceIterator_LockEndRead(self);
	if (!exec.rx_inbase)
		return ITER_DONE;
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	ReSequenceIterator_LockWrite(self);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		ReSequenceIterator_LockEndWrite(self);
		goto again;
	}
	if (result == DEE_RE_STATUS_NOMATCH ||
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
again:
	ReSequenceIterator_LockRead(self);
	DeeRegexBaseExec_Load(&self->rsi_exec, &exec, 0, NULL);
	ReSequenceIterator_LockEndRead(self);
	if (!exec.rx_inbase)
		return ITER_DONE;
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	ReSequenceIterator_LockWrite(self);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		ReSequenceIterator_LockEndWrite(self);
		goto again;
	}
	if (result == DEE_RE_STATUS_NOMATCH ||
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
	                         "iter->?Dstring"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&respiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&respiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&respiter_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&respiter_init,
				TYPE_FIXED_ALLOCATOR(ReSequenceIterator)
			}
		},
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
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&respiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &respiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&respiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ respiter_getsets,
	/* .tp_members       = */ respiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};

INTERN DeeTypeObject ReBytesSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReBytesSplitIterator",
	/* .tp_doc      = */ DOC("(reseq:?Ert:ReBytesSplit)\n"
	                         "\n"
	                         "iter->?DBytes"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rebspiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&rebspiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&rebspiter_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&rebspiter_init,
				TYPE_FIXED_ALLOCATOR(ReSequenceIterator)
			}
		},
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
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rebspiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__385A9235483A0324),
	/* .tp_cmp           = */ &rebspiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rebspiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ rebspiter_getsets,
	/* .tp_members       = */ rebspiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ DEFIMPL(&default__call_kw__with__call),
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
refa_ctor(ReSequence *__restrict self) {
	bzero(&self->rs_exec, sizeof(self->rs_exec));
	self->rs_exec.rx_code = DeeString_GetRegex(Dee_EmptyString, DEE_REGEX_COMPILE_NORMAL, NULL);
	if unlikely(!self->rs_exec.rx_code)
		return -1;
	self->rs_data            = Dee_EmptyString;
	self->rs_exec.rx_pattern = (DREF String *)Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rebfa_ctor(ReSequence *__restrict self) {
	bzero(&self->rs_exec, sizeof(self->rs_exec));
	self->rs_exec.rx_code = DeeString_GetRegex(Dee_EmptyString, DEE_REGEX_COMPILE_NORMAL, NULL);
	if unlikely(!self->rs_exec.rx_code)
		return -1;
	self->rs_data            = Dee_EmptyBytes;
	self->rs_exec.rx_pattern = (DREF String *)Dee_EmptyString;
	Dee_Incref(Dee_EmptyBytes);
	Dee_Incref(Dee_EmptyString);
	return 0;
}

#define refa_fini  refaiter_fini
#define refa_visit refaiter_visit

#define rebfa_fini  refaiter_fini
#define rebfa_visit refaiter_visit

#define rebfa_bool refa_bool
PRIVATE WUNUSED NONNULL((1)) int DCALL
refa_bool(ReSequence *__restrict self) {
	/* Check if there is at least a single match. */
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	DeeRegexBaseExec_Load(&self->rs_exec, &exec, 0, NULL);
	result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	return result != DEE_RE_STATUS_NOMATCH &&
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
	DeeObject_Init(result, &ReFindAllIterator_Type);
	Dee_atomic_rwlock_init(&result->rsi_lock);
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
	DeeObject_Init(result, &ReBytesFindAllIterator_Type);
	Dee_atomic_rwlock_init(&result->rsi_lock);
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
	DeeRegexBaseExec_Load(&self->rs_exec, &exec, 0, NULL);
	/* Count the # of matches */
	while (exec.rx_startoff < exec.rx_endoff) {
		result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
		if unlikely(result == DEE_RE_STATUS_ERROR)
			goto err;
		if (result == DEE_RE_STATUS_NOMATCH)
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
	/* .tp_enumerate          = */ NULL,
	/* .tp_enumerate_index    = */ NULL,
	/* .tp_iterkeys           = */ NULL,
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_sizeob),
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
	/* .tp_enumerate          = */ NULL,
	/* .tp_enumerate_index    = */ NULL,
	/* .tp_iterkeys           = */ NULL,
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_sizeob),
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

#define rebfa_members refa_members
PRIVATE struct type_member tpconst refa_members[] = {
	TYPE_MEMBER_FIELD_DOC("__input__", STRUCT_OBJECT, offsetof(ReSequence, rs_data), "->?X2?Dstring?DBytes"),
	TYPE_MEMBER_FIELD_DOC("__pattern__", STRUCT_OBJECT, offsetof(ReSequence, rs_exec.rx_pattern), "->?Dstring"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReSequence, rs_exec.rx_startoff)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReSequence, rs_exec.rx_endoff)),
	TYPE_MEMBER_BITFIELD("__notbol__", STRUCT_CONST, ReSequence, rs_exec.rx_eflags, DEE_RE_EXEC_NOTBOL),
	TYPE_MEMBER_BITFIELD("__noteol__", STRUCT_CONST, ReSequence, rs_exec.rx_eflags, DEE_RE_EXEC_NOTEOL),
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
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&refa_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(ReSequence)
			}
		},
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&refa_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__9211580AA9433079),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &refa_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ refa_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ refa_class_members,
};

INTERN DeeTypeObject ReBytesFindAll_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReBytesFindAll",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rebfa_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(ReSequence)
			}
		},
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rebfa_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__9211580AA9433079),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &rebfa_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rebfa_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rebfa_class_members,
};


#define regfa_ctor    refa_ctor
#define regfa_fini    refa_fini
#define regfa_bool    refa_bool
#define regfa_visit   refa_visit
#define regfa_size    refa_size
#define regfa_members refa_members

#define regbfa_ctor    refa_ctor
#define regbfa_fini    refa_fini
#define regbfa_bool    refa_bool
#define regbfa_visit   refa_visit
#define regbfa_size    refa_size
#define regbfa_members refa_members

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
	DeeObject_Init(result, &RegFindAllIterator_Type);
	Dee_atomic_rwlock_init(&result->rsi_lock);
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
	DeeObject_Init(result, &RegBytesFindAllIterator_Type);
	Dee_atomic_rwlock_init(&result->rsi_lock);
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
	/* .tp_enumerate          = */ NULL,
	/* .tp_enumerate_index    = */ NULL,
	/* .tp_iterkeys           = */ NULL,
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_sizeob),
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
	/* .tp_enumerate          = */ NULL,
	/* .tp_enumerate_index    = */ NULL,
	/* .tp_iterkeys           = */ NULL,
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_sizeob),
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
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst regbfa_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &RegBytesFindAllIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject RegFindAll_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RegFindAll",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&regfa_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(ReSequence)
			}
		},
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&regfa_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__9211580AA9433079),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &regfa_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ regfa_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ regfa_class_members,
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&regbfa_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(ReSequence)
			}
		},
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&regbfa_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__9211580AA9433079),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &regbfa_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ regbfa_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ regbfa_class_members,
};


#define rela_ctor    refa_ctor
#define rela_fini    refa_fini
#define rela_visit   refa_visit
#define rela_bool    refa_bool
#define rela_members refa_members
#define rela_size    refa_size

#define rebla_ctor    refa_ctor
#define rebla_fini    refa_fini
#define rebla_visit   refa_visit
#define rebla_bool    refa_bool
#define rebla_members refa_members
#define rebla_size    refa_size

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
	DeeObject_Init(result, &ReLocateAllIterator_Type);
	Dee_atomic_rwlock_init(&result->rsi_lock);
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
	DeeObject_Init(result, &ReBytesLocateAllIterator_Type);
	Dee_atomic_rwlock_init(&result->rsi_lock);
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
	/* .tp_enumerate          = */ NULL,
	/* .tp_enumerate_index    = */ NULL,
	/* .tp_iterkeys           = */ NULL,
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_sizeob),
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
	/* .tp_enumerate          = */ NULL,
	/* .tp_enumerate_index    = */ NULL,
	/* .tp_iterkeys           = */ NULL,
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_sizeob),
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
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rebla_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ReBytesLocateAllIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject ReLocateAll_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReLocateAll",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rela_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(ReSequence)
			}
		},
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rela_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__9211580AA9433079),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &rela_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rela_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rela_class_members,
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rebla_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(ReSequence)
			}
		},
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rebla_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__9211580AA9433079),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &rebla_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rebla_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rebla_class_members,
};

#define resp_fini    refa_fini
#define resp_visit   refa_visit
#define resp_members refa_members

#define rebsp_fini    refa_fini
#define rebsp_visit   refa_visit
#define rebsp_members refa_members

PRIVATE WUNUSED NONNULL((1)) int DCALL
resp_ctor(ReSequence *__restrict self) {
	int result = refa_ctor(self);
	self->rs_exec.rx_inbase = NULL;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rebsp_ctor(ReSequence *__restrict self) {
	int result = rebfa_ctor(self);
	self->rs_exec.rx_inbase = NULL;
	return result;
}

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
	DeeObject_Init(result, &ReSplitIterator_Type);
	Dee_atomic_rwlock_init(&result->rsi_lock);
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
	DeeObject_Init(result, &ReBytesSplitIterator_Type);
	Dee_atomic_rwlock_init(&result->rsi_lock);
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
	/* .tp_enumerate          = */ NULL,
	/* .tp_enumerate_index    = */ NULL,
	/* .tp_iterkeys           = */ NULL,
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_sizeob),
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
	/* .tp_enumerate          = */ NULL,
	/* .tp_enumerate_index    = */ NULL,
	/* .tp_iterkeys           = */ NULL,
	/* .tp_bounditem          = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_sizeob),
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
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rebsp_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ReBytesSplitIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject ReSplit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReSplit",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&resp_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(ReSequence)
			}
		},
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&resp_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__9211580AA9433079),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &resp_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ resp_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ resp_class_members,
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rebsp_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(ReSequence)
			}
		},
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rebsp_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__9211580AA9433079),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &rebsp_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rebsp_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rebsp_class_members,
};




INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_re_findall(String *__restrict self,
                  struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = (DREF DeeObject *)self;
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &ReFindAll_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_reg_findall(String *__restrict self,
                   struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = (DREF DeeObject *)self;
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &RegFindAll_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_re_locateall(String *__restrict self,
                    struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = (DREF DeeObject *)self;
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &ReLocateAll_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_re_split(String *__restrict self,
                struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = (DREF DeeObject *)self;
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	if (result->rs_exec.rx_startoff >= result->rs_exec.rx_endoff)
		result->rs_exec.rx_inbase = NULL;
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &ReSplit_Type);
done:
	return (DREF DeeObject *)result;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_findall(DeeBytesObject *__restrict self,
                 struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = (DREF DeeObject *)self;
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &ReBytesFindAll_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_reg_findall(DeeBytesObject *__restrict self,
                  struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = (DREF DeeObject *)self;
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &RegBytesFindAll_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_locateall(DeeBytesObject *__restrict self,
                   struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = (DREF DeeObject *)self;
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &ReBytesLocateAll_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_split(DeeBytesObject *__restrict self,
               struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = (DREF DeeObject *)self;
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	if (result->rs_exec.rx_startoff >= result->rs_exec.rx_endoff)
		result->rs_exec.rx_inbase = NULL;
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &ReBytesSplit_Type);
done:
	return (DREF DeeObject *)result;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_C_INL */
