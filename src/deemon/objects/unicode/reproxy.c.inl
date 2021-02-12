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

typedef struct {
	OBJECT_HEAD
	DREF String   *re_data;    /* [const][1..1] Data string. */
	DREF String   *re_pattern; /* [const][1..1] Pattern string. */
	struct re_args re_args;    /* [const] Regex arguments. */
} ReSequence;


typedef struct {
	OBJECT_HEAD
	DREF String   *re_data;    /* [const][1..1] Data string. */
	DREF String   *re_pattern; /* [const][1..1] Pattern string. */
	struct re_args re_args;    /* Regex arguments.
	                            * NOTE: Only `re_dataptr' and `re_datalen' are mutable! */
#ifndef CONFIG_NO_THREADS
	rwlock_t       re_lock;    /* Lock used during iteration. */
#endif /* !CONFIG_NO_THREADS */
} ReSequenceIterator;

STATIC_ASSERT(COMPILER_OFFSETOF(ReSequence, re_data) ==
              COMPILER_OFFSETOF(ReSequenceIterator, re_data));
STATIC_ASSERT(COMPILER_OFFSETOF(ReSequence, re_pattern) ==
              COMPILER_OFFSETOF(ReSequenceIterator, re_pattern));
STATIC_ASSERT(COMPILER_OFFSETOF(ReSequence, re_args) ==
              COMPILER_OFFSETOF(ReSequenceIterator, re_args));



PRIVATE WUNUSED NONNULL((1)) int DCALL
refaiter_ctor(ReSequenceIterator *__restrict self) {
	self->re_data    = (DREF String *)Dee_EmptyString;
	self->re_pattern = (DREF String *)Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	self->re_args.re_dataptr    = DeeString_STR(Dee_EmptyString);
	self->re_args.re_patternptr = DeeString_STR(Dee_EmptyString);
	self->re_args.re_datalen    = 0;
	self->re_args.re_patternlen = 0;
	self->re_args.re_offset     = 0;
	self->re_args.re_flags      = Dee_REGEX_FNORMAL;
	rwlock_init(&self->re_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
refaiter_copy(ReSequenceIterator *__restrict self,
              ReSequenceIterator *__restrict other) {
	self->re_data    = other->re_data;
	self->re_pattern = other->re_pattern;
	Dee_Incref(self->re_data);
	Dee_Incref(self->re_pattern);
	self->re_args.re_patternptr = other->re_args.re_patternptr;
	self->re_args.re_patternlen = other->re_args.re_patternlen;
	self->re_args.re_offset     = other->re_args.re_offset;
	self->re_args.re_flags      = other->re_args.re_flags;
	rwlock_init(&self->re_lock);
	rwlock_read(&other->re_lock);
	COMPILER_READ_BARRIER();
	self->re_args.re_dataptr = other->re_args.re_dataptr;
	self->re_args.re_datalen = other->re_args.re_datalen;
	COMPILER_READ_BARRIER();
	rwlock_endread(&other->re_lock);
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
	memcpy(&self->re_data, &reseq->re_data,
	       sizeof(ReSequence) - offsetof(ReSequence, re_data));
	Dee_Incref(self->re_data);
	Dee_Incref(self->re_pattern);
	rwlock_init(&self->re_lock);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
refaiter_fini(ReSequenceIterator *__restrict self) {
	Dee_Decref(self->re_data);
	Dee_Decref(self->re_pattern);
}

PRIVATE NONNULL((1, 2)) void DCALL
refaiter_visit(ReSequenceIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->re_data);
	Dee_Visit(self->re_pattern);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
refaiter_bool(ReSequenceIterator *__restrict self) {
	struct regex_range_ptr range;
	char *dataptr;
	size_t datalen;
	rwlock_read(&self->re_lock);
	dataptr = self->re_args.re_dataptr;
	datalen = self->re_args.re_datalen;
	rwlock_endread(&self->re_lock);
	/* Check if there is at least a single match. */
	return DeeRegex_FindPtr(dataptr,
	                        datalen,
	                        self->re_args.re_patternptr,
	                        self->re_args.re_patternlen,
	                        &range,
	                        self->re_args.re_flags);
}

/* Assert binary compatibility between the index-variants
 * of `struct regex_range' and `struct regex_range_ex' */
STATIC_ASSERT((COMPILER_OFFSETOF(struct regex_range, rr_end) -
               COMPILER_OFFSETOF(struct regex_range, rr_start)) ==
              (COMPILER_OFFSETOF(struct regex_range_ex, rr_end) -
               COMPILER_OFFSETOF(struct regex_range_ex, rr_start)));

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
refaiter_next(ReSequenceIterator *__restrict self) {
	struct regex_range_ex range;
	char *dataptr;
	size_t datalen;
	int error;
	size_t offset;
again:
	rwlock_read(&self->re_lock);
	dataptr = self->re_args.re_dataptr;
	datalen = self->re_args.re_datalen;
	rwlock_endread(&self->re_lock);
	if (!datalen)
		return ITER_DONE;
	error = DeeRegex_FindEx(dataptr, datalen,
	                        self->re_args.re_patternptr,
	                        self->re_args.re_patternlen,
	                        &range,
	                        self->re_args.re_flags);
	if unlikely(error < 0)
		goto err;
	if (!error) {
		rwlock_write(&self->re_lock);
		if (dataptr != self->re_args.re_dataptr ||
		    datalen != self->re_args.re_datalen) {
			rwlock_endwrite(&self->re_lock);
			goto again;
		}
		self->re_args.re_datalen = 0;
		rwlock_endwrite(&self->re_lock);
		return ITER_DONE;
	}
	ASSERT(range.rr_start < range.rr_end);
	rwlock_write(&self->re_lock);
	if (dataptr != self->re_args.re_dataptr ||
	    datalen != self->re_args.re_datalen) {
		rwlock_endwrite(&self->re_lock);
		goto again;
	}
	offset = self->re_args.re_offset;
	self->re_args.re_datalen -= (size_t)(range.rr_end_ptr - self->re_args.re_dataptr);
	self->re_args.re_dataptr = range.rr_end_ptr;
	self->re_args.re_offset += range.rr_end;
	rwlock_endwrite(&self->re_lock);
	return regex_pack_range(offset,
	                        (struct regex_range *)&range.rr_start);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
refaiter_getseq(ReSequenceIterator *__restrict self) {
	struct re_args args_copy;
	rwlock_read(&self->re_lock);
	memcpy(&args_copy, &self->re_args, sizeof(struct re_args));
	rwlock_endread(&self->re_lock);
	return string_re_findall(self->re_data,
	                         self->re_pattern,
	                         &args_copy);
}

#define REITER_GETDATAPTR(x) ATOMIC_READ((x)->re_args.re_dataptr)

#define DEFINE_REFA_COMPARE(name, op)                                      \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                  \
	name(ReSequenceIterator *self, ReSequenceIterator *other) {            \
		char *x, *y;                                                       \
		if (DeeObject_AssertTypeExact(other, Dee_TYPE(self))) \
			goto err;                                                      \
		x = REITER_GETDATAPTR(self);                                       \
		y = REITER_GETDATAPTR(other);                                      \
		if (!x)                                                            \
			x = (char *)(uintptr_t)-1;                                     \
		if (!y)                                                            \
			y = (char *)(uintptr_t)-1;                                     \
		return_bool(x op y);                                               \
	err:                                                                   \
		return NULL;                                                       \
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


PRIVATE struct type_getset refaiter_getsets[] = {
	{ DeeString_STR(&str_seq),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&refaiter_getseq,
	  NULL,
	  NULL,
	  DOC("->?Ert:ReFindAll") },
	{ NULL }
};

PRIVATE struct type_member refaiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(ReSequenceIterator, re_data), "->?Dstring"),
	TYPE_MEMBER_FIELD_DOC("__pattern__", STRUCT_OBJECT, offsetof(ReSequenceIterator, re_pattern), "->?Dstring"),
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
				/* .tp_ctor      = */ (void *)&refaiter_ctor,
				/* .tp_copy_ctor = */ (void *)&refaiter_copy,
				/* .tp_deep_ctor = */ (void *)&refaiter_copy,
				/* .tp_any_ctor  = */ (void *)&refaiter_init,
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
	if (DeeArg_Unpack(argc, argv, "o:_ReLocateAllIterator", &reseq) ||
	    DeeObject_AssertTypeExact(reseq, &ReLocateAll_Type))
		goto err;
	memcpy(&self->re_data, &reseq->re_data,
	       sizeof(ReSequence) - offsetof(ReSequence, re_data));
	Dee_Incref(self->re_data);
	Dee_Incref(self->re_pattern);
	rwlock_init(&self->re_lock);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
relaiter_getseq(ReSequenceIterator *__restrict self) {
	struct re_args args_copy;
	rwlock_read(&self->re_lock);
	memcpy(&args_copy, &self->re_args, sizeof(struct re_args));
	rwlock_endread(&self->re_lock);
	return string_re_locateall(self->re_data,
	                           self->re_pattern,
	                           &args_copy);
}

PRIVATE struct type_getset relaiter_getsets[] = {
	{ DeeString_STR(&str_seq),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&relaiter_getseq,
	  NULL,
	  NULL,
	  DOC("->?Ert:ReLocateAll") },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
relaiter_next(ReSequenceIterator *__restrict self) {
	struct regex_range_ptr range;
	char *dataptr;
	size_t datalen;
	int error;
again:
	rwlock_read(&self->re_lock);
	dataptr = self->re_args.re_dataptr;
	datalen = self->re_args.re_datalen;
	rwlock_endread(&self->re_lock);
	if (!datalen)
		return ITER_DONE;
	error = DeeRegex_FindPtr(dataptr, datalen,
	                         self->re_args.re_patternptr,
	                         self->re_args.re_patternlen,
	                         &range,
	                         self->re_args.re_flags);
	if unlikely(error < 0)
		return NULL;
	if (!error) {
		rwlock_write(&self->re_lock);
		if (dataptr != self->re_args.re_dataptr ||
		    datalen != self->re_args.re_datalen) {
			rwlock_endwrite(&self->re_lock);
			goto again;
		}
		self->re_args.re_datalen = 0;
		rwlock_endwrite(&self->re_lock);
		return ITER_DONE;
	}
	ASSERT(range.rr_start < range.rr_end);
	rwlock_write(&self->re_lock);
	if (dataptr != self->re_args.re_dataptr ||
	    datalen != self->re_args.re_datalen) {
		rwlock_endwrite(&self->re_lock);
		goto again;
	}
	self->re_args.re_datalen -= (size_t)(range.rr_end - self->re_args.re_dataptr);
	self->re_args.re_dataptr = range.rr_end;
	rwlock_endwrite(&self->re_lock);
	/* Return the sub-string matched by the range. */
	return DeeString_NewUtf8(range.rr_start,
	                         (size_t)(range.rr_end - range.rr_start),
	                         STRING_ERROR_FIGNORE);
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
				/* .tp_ctor      = */ (void *)&relaiter_ctor,
				/* .tp_copy_ctor = */ (void *)&relaiter_copy,
				/* .tp_deep_ctor = */ (void *)&relaiter_copy,
				/* .tp_any_ctor  = */ (void *)&relaiter_init,
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
	if (DeeArg_Unpack(argc, argv, "o:_ReSplitIterator", &reseq) ||
	    DeeObject_AssertTypeExact(reseq, &ReSplit_Type))
		goto err;
	memcpy(&self->re_data, &reseq->re_data,
	       sizeof(ReSequence) - offsetof(ReSequence, re_data));
	if (!self->re_args.re_datalen)
		self->re_args.re_dataptr = NULL;
	Dee_Incref(self->re_data);
	Dee_Incref(self->re_pattern);
	rwlock_init(&self->re_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
respiter_getseq(ReSequenceIterator *__restrict self) {
	struct re_args args_copy;
	rwlock_read(&self->re_lock);
	memcpy(&args_copy, self->re_data, sizeof(struct re_args));
	rwlock_endread(&self->re_lock);
	return string_re_split(self->re_data,
	                       self->re_pattern,
	                       &args_copy);
}

PRIVATE struct type_getset respiter_getsets[] = {
	{ DeeString_STR(&str_seq),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&respiter_getseq,
	  NULL,
	  NULL,
	  DOC("->?Ert:ReSplit") },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
respiter_next(ReSequenceIterator *__restrict self) {
	struct regex_range_ptr range;
	char *dataptr;
	size_t datalen;
	int error;
again:
	rwlock_read(&self->re_lock);
	dataptr = self->re_args.re_dataptr;
	datalen = self->re_args.re_datalen;
	rwlock_endread(&self->re_lock);
	if (!dataptr)
		return ITER_DONE;
	error = DeeRegex_FindPtr(dataptr, datalen,
	                         self->re_args.re_patternptr,
	                         self->re_args.re_patternlen,
	                         &range,
	                         self->re_args.re_flags);
	if unlikely(error < 0)
		return NULL;
	if (!error) {
		rwlock_write(&self->re_lock);
		if (dataptr != self->re_args.re_dataptr ||
		    datalen != self->re_args.re_datalen) {
			rwlock_endwrite(&self->re_lock);
			goto again;
		}
		self->re_args.re_dataptr = NULL;
		rwlock_endwrite(&self->re_lock);
		/* The last sub-string (aka. all the trailing data) */
		return DeeString_NewUtf8(dataptr, datalen, STRING_ERROR_FIGNORE);
	}
	ASSERT(range.rr_start < range.rr_end);
	rwlock_write(&self->re_lock);
	if (dataptr != self->re_args.re_dataptr ||
	    datalen != self->re_args.re_datalen) {
		rwlock_endwrite(&self->re_lock);
		goto again;
	}
	self->re_args.re_datalen -= (size_t)(range.rr_end - self->re_args.re_dataptr);
	self->re_args.re_dataptr = range.rr_end;
	rwlock_endwrite(&self->re_lock);
	/* Return the sub-string found between this match and the previous one. */
	return DeeString_NewUtf8(dataptr,
	                         (size_t)(range.rr_start - dataptr),
	                         STRING_ERROR_FIGNORE);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
respiter_bool(ReSequenceIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return self->re_args.re_dataptr != NULL;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->re_args.re_dataptr) != NULL;
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
				/* .tp_ctor      = */ (void *)&respiter_ctor,
				/* .tp_copy_ctor = */ (void *)&respiter_copy,
				/* .tp_deep_ctor = */ (void *)&respiter_copy,
				/* .tp_any_ctor  = */ (void *)&respiter_init,
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
	self->re_data    = (DREF String *)Dee_EmptyString;
	self->re_pattern = (DREF String *)Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	self->re_args.re_dataptr    = DeeString_STR(Dee_EmptyString);
	self->re_args.re_patternptr = DeeString_STR(Dee_EmptyString);
	self->re_args.re_datalen    = 0;
	self->re_args.re_patternlen = 0;
	self->re_args.re_offset     = 0;
	self->re_args.re_flags      = Dee_REGEX_FNORMAL;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
refa_init(ReSequence *__restrict self,
          size_t argc, DeeObject *const *argv) {
	String *rules = NULL;
	if (DeeArg_Unpack(argc, argv, "oo|o" /*":_ReFindAll"*/,
	                  &self->re_data, &self->re_pattern, &rules) ||
	    DeeObject_AssertTypeExact(self->re_data, &DeeString_Type) ||
	    DeeObject_AssertTypeExact(self->re_pattern, &DeeString_Type))
		goto err;
	self->re_args.re_flags = Dee_REGEX_FNORMAL;
	if (rules) {
		if (DeeObject_AssertTypeExact(rules, &DeeString_Type))
			goto err;
		if (regex_get_rules(DeeString_STR(rules), &self->re_args.re_flags))
			goto err;
	}
	self->re_args.re_dataptr = DeeString_AsUtf8((DeeObject *)self->re_data);
	if unlikely(!self->re_args.re_dataptr)
		goto err;
	self->re_args.re_patternptr = DeeString_AsUtf8((DeeObject *)self->re_pattern);
	if unlikely(!self->re_args.re_patternptr)
		goto err;
	self->re_args.re_datalen    = WSTR_LENGTH(self->re_args.re_dataptr);
	self->re_args.re_patternlen = WSTR_LENGTH(self->re_args.re_patternptr);
	Dee_Incref(self->re_data);
	Dee_Incref(self->re_pattern);
	self->re_args.re_offset = 0;
	return 0;
err:
	return -1;
}

#define refa_fini  refaiter_fini
#define refa_visit refaiter_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
refa_bool(ReSequence *__restrict self) {
	struct regex_range_ptr range;
	/* Check if there is at least a single match. */
	return DeeRegex_FindPtr(self->re_args.re_dataptr,
	                        self->re_args.re_datalen,
	                        self->re_args.re_patternptr,
	                        self->re_args.re_patternlen,
	                        &range,
	                        self->re_args.re_flags);
}

PRIVATE WUNUSED NONNULL((1)) DREF ReSequenceIterator *DCALL
refa_iter(ReSequence *__restrict self) {
	DREF ReSequenceIterator *result;
	result = DeeObject_MALLOC(ReSequenceIterator);
	if unlikely(!result)
		goto done;
	memcpy(&result->re_data, &self->re_data,
	       sizeof(ReSequence) - offsetof(ReSequence, re_data));
	Dee_Incref(result->re_data);
	Dee_Incref(result->re_pattern);
	DeeObject_Init(result, &ReFindAllIterator_Type);
	rwlock_init(&result->re_lock);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
refa_nsi_getsize(ReSequence *__restrict self) {
	int error;
	size_t result;
	struct regex_range_ptr range;
	char *dataptr;
	size_t datalen;
	result  = 0;
	dataptr = self->re_args.re_dataptr;
	datalen = self->re_args.re_datalen;
	for (;;) {
		error = DeeRegex_FindPtr(dataptr,
		                         datalen,
		                         self->re_args.re_patternptr,
		                         self->re_args.re_patternlen,
		                         &range,
		                         self->re_args.re_flags);
		if unlikely(error < 0)
			goto err;
		if (!error)
			break;
		++result;
		datalen -= (size_t)(range.rr_end - dataptr);
		dataptr = range.rr_end;
	}
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
refa_size(ReSequence *__restrict self) {
	size_t result = refa_nsi_getsize(self);
	if unlikely(result == (size_t)-1)
		return NULL;
	return DeeInt_NewSize(result);
}


PRIVATE struct type_nsi refa_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&refa_nsi_getsize,
			/* .nsi_getsize_fast = */ (void *)NULL,
			/* .nsi_getitem      = */ (void *)NULL,
			/* .nsi_delitem      = */ (void *)NULL,
			/* .nsi_setitem      = */ (void *)NULL,
			/* .nsi_getitem_fast = */ (void *)NULL,
			/* .nsi_getrange     = */ (void *)NULL,
			/* .nsi_getrange_n   = */ (void *)NULL,
			/* .nsi_setrange     = */ (void *)NULL,
			/* .nsi_setrange_n   = */ (void *)NULL,
			/* .nsi_find         = */ (void *)NULL,
			/* .nsi_rfind        = */ (void *)NULL,
			/* .nsi_xch          = */ (void *)NULL,
			/* .nsi_insert       = */ (void *)NULL,
			/* .nsi_insertall    = */ (void *)NULL,
			/* .nsi_insertvec    = */ (void *)NULL,
			/* .nsi_pop          = */ (void *)NULL,
			/* .nsi_erase        = */ (void *)NULL,
			/* .nsi_remove       = */ (void *)NULL,
			/* .nsi_rremove      = */ (void *)NULL,
			/* .nsi_removeall    = */ (void *)NULL
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

PRIVATE struct type_member refa_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(ReSequence, re_data), "->?Dstring"),
	TYPE_MEMBER_FIELD_DOC("__pattern__", STRUCT_OBJECT, offsetof(ReSequence, re_pattern), "->?Dstring"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member refa_class_members[] = {
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
				/* .tp_ctor      = */ (void *)&refa_ctor,
				/* .tp_copy_ctor = */ (void *)NULL,
				/* .tp_deep_ctor = */ (void *)NULL,
				/* .tp_any_ctor  = */ (void *)&refa_init,
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
	memcpy(&result->re_data, &self->re_data,
	       sizeof(ReSequence) - offsetof(ReSequence, re_data));
	Dee_Incref(result->re_data);
	Dee_Incref(result->re_pattern);
	DeeObject_Init(result, &ReLocateAllIterator_Type);
	rwlock_init(&result->re_lock);
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

PRIVATE struct type_member rela_class_members[] = {
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
				/* .tp_ctor      = */ (void *)&rela_ctor,
				/* .tp_copy_ctor = */ (void *)NULL,
				/* .tp_deep_ctor = */ (void *)NULL,
				/* .tp_any_ctor  = */ (void *)&rela_init,
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
	int result               = refa_ctor(self);
	self->re_args.re_dataptr = NULL;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
resp_init(ReSequence *__restrict self,
          size_t argc, DeeObject *const *argv) {
	int result = refa_init(self, argc, argv);
	if (!self->re_args.re_datalen)
		self->re_args.re_dataptr = NULL;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
resp_bool(ReSequence *__restrict self) {
	return self->re_args.re_dataptr != NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF ReSequenceIterator *DCALL
resp_iter(ReSequence *__restrict self) {
	DREF ReSequenceIterator *result;
	result = DeeObject_MALLOC(ReSequenceIterator);
	if unlikely(!result)
		goto done;
	memcpy(&result->re_data, &self->re_data,
	       sizeof(ReSequence) - offsetof(ReSequence, re_data));
	ASSERT(result->re_args.re_datalen ? 1 : result->re_args.re_dataptr == NULL);
	Dee_Incref(result->re_data);
	Dee_Incref(result->re_pattern);
	DeeObject_Init(result, &ReSplitIterator_Type);
	rwlock_init(&result->re_lock);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
resp_nsi_getsize(ReSequence *__restrict self) {
	size_t result;
	if (!self->re_args.re_dataptr)
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
		return NULL;
	return DeeInt_NewSize(result);
}


PRIVATE struct type_nsi resp_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&resp_nsi_getsize,
			/* .nsi_getsize_fast = */ (void *)NULL,
			/* .nsi_getitem      = */ (void *)NULL,
			/* .nsi_delitem      = */ (void *)NULL,
			/* .nsi_setitem      = */ (void *)NULL,
			/* .nsi_getitem_fast = */ (void *)NULL,
			/* .nsi_getrange     = */ (void *)NULL,
			/* .nsi_getrange_n   = */ (void *)NULL,
			/* .nsi_setrange     = */ (void *)NULL,
			/* .nsi_setrange_n   = */ (void *)NULL,
			/* .nsi_find         = */ (void *)NULL,
			/* .nsi_rfind        = */ (void *)NULL,
			/* .nsi_xch          = */ (void *)NULL,
			/* .nsi_insert       = */ (void *)NULL,
			/* .nsi_insertall    = */ (void *)NULL,
			/* .nsi_insertvec    = */ (void *)NULL,
			/* .nsi_pop          = */ (void *)NULL,
			/* .nsi_erase        = */ (void *)NULL,
			/* .nsi_remove       = */ (void *)NULL,
			/* .nsi_rremove      = */ (void *)NULL,
			/* .nsi_removeall    = */ (void *)NULL
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

PRIVATE struct type_member resp_class_members[] = {
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
				/* .tp_ctor      = */ (void *)&resp_ctor,
				/* .tp_copy_ctor = */ (void *)NULL,
				/* .tp_deep_ctor = */ (void *)NULL,
				/* .tp_any_ctor  = */ (void *)&resp_init,
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




INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
string_re_findall(String *__restrict self,
                  String *__restrict pattern,
                  struct re_args const *__restrict args) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->re_data    = self;
	result->re_pattern = pattern;
	memcpy(&result->re_args, args, sizeof(struct re_args));
	Dee_Incref(self);
	Dee_Incref(pattern);
	DeeObject_Init(result, &ReFindAll_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
string_re_locateall(String *__restrict self,
                    String *__restrict pattern,
                    struct re_args const *__restrict args) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->re_data    = self;
	result->re_pattern = pattern;
	memcpy(&result->re_args, args, sizeof(struct re_args));
	Dee_Incref(self);
	Dee_Incref(pattern);
	DeeObject_Init(result, &ReLocateAll_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
string_re_split(String *__restrict self,
                String *__restrict pattern,
                struct re_args const *__restrict args) {
	DREF ReSequence *result;
	result = DeeObject_MALLOC(ReSequence);
	if unlikely(!result)
		goto done;
	result->re_data    = self;
	result->re_pattern = pattern;
	memcpy(&result->re_args, args, sizeof(struct re_args));
	if (!result->re_args.re_datalen)
		result->re_args.re_dataptr = NULL;
	Dee_Incref(self);
	Dee_Incref(pattern);
	DeeObject_Init(result, &ReSplit_Type);
done:
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_C_INL */
