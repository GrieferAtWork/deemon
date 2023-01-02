/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_C_INL 1

#ifdef __INTELLISENSE__
#include "string_functions.c"
#endif /* !__INTELLISENSE__ */

#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memcpy() */

#include <hybrid/atomic.h>

#include "../../runtime/strings.h"

/* Proxy sequence objects for `string.refindall',
 * `string.relocateall' and `string.resplit' */

DECL_BEGIN

INTDEF DeeTypeObject ReFindAll_Type;
INTDEF DeeTypeObject ReFindAllIterator_Type;
INTDEF DeeTypeObject ReLocateAll_Type;
INTDEF DeeTypeObject ReLocateAllIterator_Type;
INTDEF DeeTypeObject ReSplit_Type;
INTDEF DeeTypeObject ReSplitIterator_Type;

#define DeeRegexBaseExec_Load(self, result, nmatch, pmatch) \
	(void)((result)->rx_code     = (self)->rx_code,         \
	       (result)->rx_inbase   = (self)->rx_inbase,       \
	       (result)->rx_insize   = (self)->rx_insize,       \
	       (result)->rx_startoff = (self)->rx_startoff,     \
	       (result)->rx_endoff   = (self)->rx_endoff,       \
	       (result)->rx_eflags   = (self)->rx_eflags)



typedef struct {
	OBJECT_HEAD
	DREF String            *rs_data;    /* [const][1..1] Data string. */
	struct DeeRegexBaseExec rs_exec;    /* [const] Regex arguments. */
} ReSequence;


typedef struct {
	OBJECT_HEAD
	DREF String            *rsi_data;    /* [const][1..1] Data string. */
	struct DeeRegexBaseExec rsi_exec;    /* Regex arguments. */
#ifndef CONFIG_NO_THREADS
	rwlock_t                rsi_lock;    /* Lock used during iteration. */
#endif /* !CONFIG_NO_THREADS */
} ReSequenceIterator;

STATIC_ASSERT(offsetof(ReSequence, rs_data) == offsetof(ReSequenceIterator, rsi_data));
STATIC_ASSERT(offsetof(ReSequence, rs_exec) == offsetof(ReSequenceIterator, rsi_exec));



PRIVATE WUNUSED NONNULL((1)) int DCALL
refaiter_ctor(ReSequenceIterator *__restrict self) {
	bzero(&self->rsi_exec, sizeof(self->rsi_exec));
	self->rsi_exec.rx_code = DeeString_GetRegex(Dee_EmptyString, DEE_REGEX_COMPILE_NORMAL);
	if unlikely(!self->rsi_exec.rx_code)
		return -1;
	self->rsi_data            = (DREF String *)Dee_EmptyString;
	self->rsi_exec.rx_pattern = (DREF String *)Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	rwlock_init(&self->rsi_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
refaiter_copy(ReSequenceIterator *__restrict self,
              ReSequenceIterator *__restrict other) {
	rwlock_read(&other->rsi_lock);
	memcpy(&self->rsi_exec, &other->rsi_exec, sizeof(self->rsi_exec));
	rwlock_endread(&other->rsi_lock);
	self->rsi_data = other->rsi_data;
	Dee_Incref(self->rsi_data);
	Dee_Incref(self->rsi_exec.rx_pattern);
	rwlock_init(&self->rsi_lock);
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
	rwlock_init(&self->rsi_lock);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
refaiter_fini(ReSequenceIterator *__restrict self) {
	Dee_Decref(self->rsi_data);
	Dee_Decref(self->rsi_exec.rx_pattern);
}

PRIVATE NONNULL((1, 2)) void DCALL
refaiter_visit(ReSequenceIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->rsi_data);
	Dee_Visit(self->rsi_exec.rx_pattern);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
refaiter_bool(ReSequenceIterator *__restrict self) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	rwlock_read(&self->rsi_lock);
	DeeRegexBaseExec_Load(&self->rsi_exec, &exec, 0, NULL);
	rwlock_endread(&self->rsi_lock);
	result = DeeRegex_Search(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	return result != DEE_RE_STATUS_NOMATCH &&
	       match_size != 0; /* Prevent infinite loop on epsilon-match */
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
refaiter_next(ReSequenceIterator *__restrict self) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
again:
	rwlock_read(&self->rsi_lock);
	DeeRegexBaseExec_Load(&self->rsi_exec, &exec, 0, NULL);
	rwlock_endread(&self->rsi_lock);
	result = DeeRegex_Search(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return ITER_DONE;
	if (match_size == 0)
		return ITER_DONE; /* Prevent infinite loop on epsilon-match */
	rwlock_write(&self->rsi_lock);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		rwlock_endwrite(&self->rsi_lock);
		goto again;
	}
	self->rsi_exec.rx_startoff = (size_t)result + match_size;
	rwlock_endwrite(&self->rsi_lock);
	match_size = string_bytecnt2charcnt(self->rsi_data, (size_t)match_size, (char const *)exec.rx_inbase + (size_t)result);
	result     = string_bytecnt2charcnt(self->rsi_data, (size_t)result, (char const *)exec.rx_inbase);
	match_size += result;
	return DeeTuple_Newf(PCKuSIZ PCKuSIZ, (size_t)result, (size_t)match_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
refaiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	rwlock_read(&self->rsi_lock);
	memcpy(&args_copy, &self->rsi_exec, sizeof(struct DeeRegexBaseExec));
	rwlock_endread(&self->rsi_lock);
	return string_re_findall(self->rsi_data, &args_copy);
}

#define REITER_GETDATAPTR(x) ATOMIC_READ((x)->rsi_exec.rx_startoff)

#define DEFINE_REFA_COMPARE(name, op)                           \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL       \
	name(ReSequenceIterator *self, ReSequenceIterator *other) { \
		size_t x, y;                                            \
		if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))   \
			goto err;                                           \
		x = REITER_GETDATAPTR(self);                            \
		y = REITER_GETDATAPTR(other);                           \
		return_bool(x op y);                                    \
	err:                                                        \
		return NULL;                                            \
	}
DEFINE_REFA_COMPARE(refa_eq, ==)
DEFINE_REFA_COMPARE(refa_ne, !=)
DEFINE_REFA_COMPARE(refa_lo, <)
DEFINE_REFA_COMPARE(refa_le, <=)
DEFINE_REFA_COMPARE(refa_gr, >)
DEFINE_REFA_COMPARE(refa_ge, >=)
#undef DEFINE_REFA_COMPARE


PRIVATE struct type_cmp refaiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&refa_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&refa_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&refa_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&refa_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&refa_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&refa_ge
};


PRIVATE struct type_getset tpconst refaiter_getsets[] = {
	{ DeeString_STR(&str_seq),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&refaiter_getseq,
	  NULL,
	  NULL,
	  DOC("->?Ert:ReFindAll") },
	{ NULL }
};

PRIVATE struct type_member tpconst refaiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(ReSequenceIterator, rsi_data), "->?Dstring"),
	TYPE_MEMBER_FIELD_DOC("__pattern__", STRUCT_OBJECT, offsetof(ReSequenceIterator, rsi_exec.rx_pattern), "->?Dstring"),
	TYPE_MEMBER_FIELD("__startoff__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReSequenceIterator, rsi_exec.rx_startoff)),
	TYPE_MEMBER_FIELD("__endoff__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReSequenceIterator, rsi_exec.rx_endoff)),
	TYPE_MEMBER_BITFIELD("__notbol__", STRUCT_CONST, ReSequenceIterator, rsi_exec.rx_eflags, DEE_RE_EXEC_NOTBOL),
	TYPE_MEMBER_BITFIELD("__noteol__", STRUCT_CONST, ReSequenceIterator, rsi_exec.rx_eflags, DEE_RE_EXEC_NOTEOL),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject ReFindAllIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReFindAllIterator",
	/* .tp_doc      = */ NULL,
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&refaiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&refaiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &refaiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&refaiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ refaiter_getsets,
	/* .tp_members       = */ refaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


#define relaiter_ctor    refaiter_ctor
#define relaiter_copy    refaiter_copy
#define relaiter_fini    refaiter_fini
#define relaiter_bool    refaiter_bool
#define relaiter_cmp     refaiter_cmp
#define relaiter_members refaiter_members

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
	rwlock_init(&self->rsi_lock);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
relaiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	rwlock_read(&self->rsi_lock);
	memcpy(&args_copy, &self->rsi_exec, sizeof(struct DeeRegexBaseExec));
	rwlock_endread(&self->rsi_lock);
	return string_re_locateall(self->rsi_data, &args_copy);
}

PRIVATE struct type_getset tpconst relaiter_getsets[] = {
	{ DeeString_STR(&str_seq),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&relaiter_getseq,
	  NULL,
	  NULL,
	  DOC("->?Ert:ReLocateAll") },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
relaiter_next(ReSequenceIterator *__restrict self) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
again:
	rwlock_read(&self->rsi_lock);
	DeeRegexBaseExec_Load(&self->rsi_exec, &exec, 0, NULL);
	rwlock_endread(&self->rsi_lock);
	result = DeeRegex_Search(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return ITER_DONE;
	if (match_size == 0)
		return ITER_DONE; /* Prevent infinite loop on epsilon-match */
	rwlock_write(&self->rsi_lock);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		rwlock_endwrite(&self->rsi_lock);
		goto again;
	}
	self->rsi_exec.rx_startoff = (size_t)result + match_size;
	rwlock_endwrite(&self->rsi_lock);
	return DeeString_NewUtf8((char const *)exec.rx_inbase + (size_t)result,
	                         match_size, STRING_ERROR_FSTRICT);
err:
	return NULL;
}

INTERN DeeTypeObject ReLocateAllIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReLocateAllIterator",
	/* .tp_doc      = */ NULL,
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&relaiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&relaiter_fini,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &relaiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&relaiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ relaiter_getsets,
	/* .tp_members       = */ relaiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


#define respiter_ctor    refaiter_ctor
#define respiter_copy    refaiter_copy
#define respiter_fini    refaiter_fini
#define respiter_visit   refaiter_visit
#define respiter_cmp     refaiter_cmp
#define respiter_members refaiter_members

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
	rwlock_init(&self->rsi_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
respiter_getseq(ReSequenceIterator *__restrict self) {
	struct DeeRegexBaseExec args_copy;
	rwlock_read(&self->rsi_lock);
	memcpy(&args_copy, self->rsi_data, sizeof(struct DeeRegexBaseExec));
	rwlock_endread(&self->rsi_lock);
	return string_re_split(self->rsi_data, &args_copy);
}

PRIVATE struct type_getset tpconst respiter_getsets[] = {
	{ DeeString_STR(&str_seq),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&respiter_getseq,
	  NULL,
	  NULL,
	  DOC("->?Ert:ReSplit") },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
respiter_next(ReSequenceIterator *__restrict self) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
again:
	rwlock_read(&self->rsi_lock);
	DeeRegexBaseExec_Load(&self->rsi_exec, &exec, 0, NULL);
	rwlock_endread(&self->rsi_lock);
	if (!exec.rx_inbase)
		return ITER_DONE;
	result = DeeRegex_Search(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return ITER_DONE;
	if (match_size == 0)
		return ITER_DONE; /* Prevent infinite loop on epsilon-match */
	rwlock_write(&self->rsi_lock);
	if unlikely(self->rsi_exec.rx_startoff != exec.rx_startoff) {
		rwlock_endwrite(&self->rsi_lock);
		goto again;
	}
	self->rsi_exec.rx_startoff = (size_t)result + match_size;
	rwlock_endwrite(&self->rsi_lock);
	return DeeString_NewUtf8((char const *)exec.rx_inbase + exec.rx_startoff,
	                         (size_t)result - exec.rx_startoff,
	                         STRING_ERROR_FSTRICT);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
respiter_bool(ReSequenceIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return self->rsi_exec.rx_inbase != NULL;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->rsi_exec.rx_inbase) != NULL;
#endif /* !CONFIG_NO_THREADS */
}


INTERN DeeTypeObject ReSplitIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ReSplitIterator",
	/* .tp_doc      = */ NULL,
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&respiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&respiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &respiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&respiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ respiter_getsets,
	/* .tp_members       = */ respiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
refa_ctor(ReSequence *__restrict self) {
	bzero(&self->rs_exec, sizeof(self->rs_exec));
	self->rs_exec.rx_code = DeeString_GetRegex(Dee_EmptyString, DEE_REGEX_COMPILE_NORMAL);
	if unlikely(!self->rs_exec.rx_code)
		return -1;
	self->rs_data            = (DREF String *)Dee_EmptyString;
	self->rs_exec.rx_pattern = (DREF String *)Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	return 0;
}

#define refa_fini  refaiter_fini
#define refa_visit refaiter_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
refa_bool(ReSequence *__restrict self) {
	/* Check if there is at least a single match. */
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	DeeRegexBaseExec_Load(&self->rs_exec, &exec, 0, NULL);
	result = DeeRegex_Search(&exec, (size_t)-1, &match_size);
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
	rwlock_init(&result->rsi_lock);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
refa_nsi_getsize(ReSequence *__restrict self) {
	size_t count = 0;
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	DeeRegexBaseExec_Load(&self->rs_exec, &exec, 0, NULL);
	/* Count the # of matches */
	while (exec.rx_startoff < exec.rx_endoff) {
		result = DeeRegex_Search(&exec, (size_t)-1, &match_size);
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
refa_size(ReSequence *__restrict self) {
	size_t result = refa_nsi_getsize(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}


PRIVATE struct type_nsi tpconst refa_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&refa_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)NULL,
			/* .nsi_getitem      = */ (dfunptr_t)NULL,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)NULL,
			/* .nsi_rfind        = */ (dfunptr_t)NULL,
			/* .nsi_xch          = */ (dfunptr_t)NULL,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)NULL,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)NULL,
			/* .nsi_rremove      = */ (dfunptr_t)NULL,
			/* .nsi_removeall    = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_seq refa_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&refa_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&refa_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &refa_nsi
};

PRIVATE struct type_member tpconst refa_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(ReSequence, rs_data), "->?Dstring"),
	TYPE_MEMBER_FIELD_DOC("__pattern__", STRUCT_OBJECT, offsetof(ReSequence, rs_exec.rx_pattern), "->?Dstring"),
	TYPE_MEMBER_FIELD("__startoff__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReSequence, rs_exec.rx_startoff)),
	TYPE_MEMBER_FIELD("__endoff__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(ReSequence, rs_exec.rx_endoff)),
	TYPE_MEMBER_BITFIELD("__notbol__", STRUCT_CONST, ReSequence, rs_exec.rx_eflags, DEE_RE_EXEC_NOTBOL),
	TYPE_MEMBER_BITFIELD("__noteol__", STRUCT_CONST, ReSequence, rs_exec.rx_eflags, DEE_RE_EXEC_NOTEOL),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst refa_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &ReFindAllIterator_Type),
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&refa_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&refa_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &refa_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ refa_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ refa_class_members
};


#define rela_ctor    refa_ctor
#define rela_init    refa_init
#define rela_fini    refa_fini
#define rela_visit   refa_visit
#define rela_bool    refa_bool
#define rela_members refa_members
#define rela_size    refa_size
#define rela_nsi     refa_nsi

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
	rwlock_init(&result->rsi_lock);
done:
	return result;
}

PRIVATE struct type_seq rela_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rela_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rela_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &rela_nsi
};

PRIVATE struct type_member tpconst rela_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &ReLocateAllIterator_Type),
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rela_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rela_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &rela_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rela_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rela_class_members
};

#define resp_fini    refa_fini
#define resp_visit   refa_visit
#define resp_members refa_members

PRIVATE WUNUSED NONNULL((1)) int DCALL
resp_ctor(ReSequence *__restrict self) {
	int result = refa_ctor(self);
	self->rs_exec.rx_inbase = NULL;
	return result;
}

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
	rwlock_init(&result->rsi_lock);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
resp_nsi_getsize(ReSequence *__restrict self) {
	size_t result;
	if (!self->rs_exec.rx_inbase)
		return 0;
	result = refa_nsi_getsize(self);
	if (result != (size_t)-1)
		++result;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
resp_size(ReSequence *__restrict self) {
	size_t result = resp_nsi_getsize(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}


PRIVATE struct type_nsi tpconst resp_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&resp_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)NULL,
			/* .nsi_getitem      = */ (dfunptr_t)NULL,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)NULL,
			/* .nsi_rfind        = */ (dfunptr_t)NULL,
			/* .nsi_xch          = */ (dfunptr_t)NULL,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)NULL,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)NULL,
			/* .nsi_rremove      = */ (dfunptr_t)NULL,
			/* .nsi_removeall    = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_seq resp_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&resp_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&resp_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &resp_nsi
};

PRIVATE struct type_member tpconst resp_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &ReSplitIterator_Type),
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&resp_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&resp_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &resp_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ resp_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ resp_class_members
};




INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_re_findall(String *__restrict self,
                  struct DeeRegexBaseExec const *__restrict exec) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->rs_data = self;
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &ReFindAll_Type);
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
	result->rs_data = self;
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
	result->rs_data    = self;
	memcpy(&result->rs_exec, exec, sizeof(struct DeeRegexBaseExec));
	if (result->rs_exec.rx_startoff >= result->rs_exec.rx_endoff)
		result->rs_exec.rx_inbase = NULL;
	Dee_Incref(self);
	Dee_Incref(result->rs_exec.rx_pattern);
	DeeObject_Init(result, &ReSplit_Type);
done:
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_C_INL */
