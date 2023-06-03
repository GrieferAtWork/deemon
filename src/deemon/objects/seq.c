/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_C
#define GUARD_DEEMON_OBJECTS_SEQ_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/lock.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "seq/each.h"
#include "seq/simpleproxy.h"
#include "seq/svec.h"
#include "seq_functions.h"

#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN


/* Lookup the closest NSI descriptor for `tp', or return `NULL'
 * if the top-most type implementing any sequence operator doesn't
 * expose NSI functionality. */
PUBLIC WUNUSED NONNULL((1)) struct type_nsi const *DCALL
DeeType_NSI(DeeTypeObject *__restrict tp) {
	ASSERT_OBJECT_TYPE(tp, &DeeType_Type);
	do {
		if (tp->tp_seq)
			return tp->tp_seq->tp_nsi;
	} while (type_inherit_nsi(tp));
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_size(DeeObject *__restrict self) {
	size_t result;
	result = DeeSeq_Size(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seq_tpcontains(DeeObject *self, DeeObject *elem) {
	int result;
	result = DeeSeq_Contains(self, elem, NULL);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seq_getitem(DeeObject *self, DeeObject *index) {
	size_t i;
	if (DeeObject_AsSize(index, &i))
		goto err;
	return DeeSeq_GetItem(self, i);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
seq_getrange(DeeObject *self, DeeObject *start, DeeObject *end) {
	dssize_t i_begin, i_end;
	if (DeeObject_AsSSize(start, &i_begin))
		goto err;
	if (DeeNone_Check(end)) {
		if unlikely(i_begin < 0) {
			size_t seq_len = DeeObject_Size(self);
			if unlikely(seq_len == (size_t)-1)
				goto err;
			if (i_begin < 0)
				i_begin += seq_len;
		}
		return DeeSeq_GetRangeN(self, (size_t)i_begin);
	}
	if (DeeObject_AsSSize(end, &i_end))
		goto err;
	if unlikely(i_begin < 0 || i_end < 0) {
		size_t seq_len = DeeObject_Size(self);
		if unlikely(seq_len == (size_t)-1)
			goto err;
		if (i_begin < 0)
			i_begin += seq_len;
		if (i_end < 0)
			i_end += seq_len;
	}
	return DeeSeq_GetRange(self,
	                       (size_t)i_begin,
	                       (size_t)i_end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_nsi_getrange(DeeObject *__restrict self, dssize_t i_begin, dssize_t i_end) {
	if unlikely(i_begin < 0 || i_end < 0) {
		size_t seq_len;
		seq_len = DeeObject_Size(self);
		if unlikely(seq_len == (size_t)-1)
			goto err;
		if (i_begin < 0)
			i_begin += seq_len;
		if (i_end < 0)
			i_end += seq_len;
	}
	return DeeSeq_GetRange(self,
	                       (size_t)i_begin,
	                       (size_t)i_end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_nsi_getrange_n(DeeObject *__restrict self, dssize_t i_begin) {
	if unlikely(i_begin < 0) {
		size_t seq_len;
		seq_len = DeeObject_Size(self);
		if unlikely(seq_len == (size_t)-1)
			goto err;
		if (i_begin < 0)
			i_begin += seq_len;
	}
	return DeeSeq_GetRangeN(self, (size_t)i_begin);
err:
	return NULL;
}

typedef struct {
	OBJECT_HEAD
	/* TODO: Integrate NSI optimizations. */
	DREF DeeObject *(DCALL *si_getitem)(DeeObject *self, DeeObject *index);
	DREF DeeObject         *si_seq;   /* [1..1][const] The Sequence being iterated. */
	DREF DeeObject         *si_size;  /* [1..1][const] The size of the Sequence. */
	DREF DeeObject         *si_index; /* [1..1][lock(si_lock)] The current index (`int' object). */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t     si_lock;  /* Lock for accessing `si_index' */
#endif /* !CONFIG_NO_THREADS */
} SeqIterator;

#define SeqIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->si_lock)
#define SeqIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->si_lock)
#define SeqIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->si_lock)
#define SeqIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->si_lock)
#define SeqIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->si_lock)
#define SeqIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->si_lock)
#define SeqIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->si_lock)
#define SeqIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->si_lock)
#define SeqIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->si_lock)
#define SeqIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->si_lock)
#define SeqIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->si_lock)
#define SeqIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->si_lock)
#define SeqIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->si_lock)
#define SeqIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->si_lock)
#define SeqIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->si_lock)
#define SeqIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->si_lock)

PRIVATE /*WUNUSED*/ NONNULL((1)) int DCALL
seqiterator_ctor(SeqIterator *__restrict self) {
	self->si_getitem = &seq_getitem;
	self->si_seq     = Dee_EmptySeq;
	self->si_index   = DeeInt_Zero;
	self->si_size    = DeeInt_Zero;
	Dee_atomic_rwlock_init(&self->si_lock);
	Dee_Incref(Dee_EmptySeq);
	Dee_Incref(DeeInt_Zero);
	Dee_Incref(DeeInt_Zero);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seqiterator_copy(SeqIterator *__restrict self,
                 SeqIterator *__restrict other) {
	self->si_getitem = other->si_getitem;
	self->si_seq     = other->si_seq;
	self->si_size    = other->si_size;
	Dee_Incref(self->si_seq);
	Dee_Incref(self->si_size);
	Dee_atomic_rwlock_init(&self->si_lock);
	SeqIterator_LockRead(other);
	self->si_index = other->si_index;
	Dee_Incref(self->si_index);
	SeqIterator_LockEndRead(other);
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
seqiterator_fini(SeqIterator *__restrict self) {
	Dee_Decref(self->si_seq);
	Dee_Decref(self->si_size);
	Dee_Decref(self->si_index);
}

PRIVATE NONNULL((1, 2)) void DCALL
seqiterator_visit(SeqIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->si_seq);
	Dee_Visit(self->si_size);
	SeqIterator_LockRead(self);
	Dee_Visit(self->si_index);
	SeqIterator_LockEndRead(self);
}

PRIVATE WUNUSED NONNULL((1)) dssize_t DCALL
seqiterator_printrepr(SeqIterator *__restrict self,
                      dformatprinter printer, void *arg) {
	dssize_t result;
	DREF DeeObject *index_ob;
	SeqIterator_LockRead(self);
	index_ob = self->si_index;
	Dee_Incref(index_ob);
	SeqIterator_LockEndRead(self);
	result = DeeFormat_Printf(printer, arg,
	                          "GenericIterator(%r, %r /* of %r */)",
	                          self->si_seq, index_ob, self->si_size);
	Dee_Decref(index_ob);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seqiterator_next(SeqIterator *__restrict self) {
	DREF DeeObject *old_index;
	int error;
	DREF DeeObject *result, *new_index;
again:
	SeqIterator_LockRead(self);
	old_index = self->si_index;
	Dee_Incref(old_index);
	SeqIterator_LockEndRead(self);
	/* Check if the Iterator has been exhausted. */
	error = DeeObject_CompareGe(old_index, self->si_size);
	if unlikely(error < 0)
		goto err_old_index;
	if (error)
		goto eof_old_index;
	/* Check if the index has changed during the comparison. */
	if (old_index != self->si_index)
		goto old_index_again;
	/* Lookup the item that's going to be returned. */
	result = (*self->si_getitem)(self->si_seq, old_index);
	if unlikely(!result) {
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err_old_index;
		/* Unbound item (just skip it!). */
		new_index = old_index;
		Dee_Incref(new_index);
		for (;;) {
			if (DeeObject_Inc(&new_index))
				goto err_new_index;
			error = DeeObject_CompareGe(new_index, self->si_size);
			if unlikely(error < 0)
				goto err_new_index;
			if (error)
				goto eof_new_index;
			/* Check if the index has changed during the comparison. */
			if (old_index != self->si_index)
				goto new_index_again;
			result = (*self->si_getitem)(self->si_seq, new_index);
			if likely(result)
				goto set_new_index_plus;
			if (!DeeError_Catch(&DeeError_UnboundItem))
				goto err_new_index;
		}
	}
	/* Increment the index. */
	new_index = old_index;
	Dee_Incref(new_index);
set_new_index_plus:
	if (DeeObject_Inc(&new_index)) {
		Dee_Decref(result);
		goto err_new_index;
	}
	SeqIterator_LockWrite(self);
	COMPILER_READ_BARRIER();
	if (old_index != self->si_index) {
		/* The index was changed in the mean time. */
		SeqIterator_LockEndWrite(self);
		Dee_Decref(result);
		old_index = new_index;
old_index_again:
		Dee_Decref(old_index);
		goto again;
	}
	self->si_index = new_index; /* Override reference. */
	SeqIterator_LockEndWrite(self);
	/* Drop the old-index reference we've inherited
	 * when overriding `self->si_index' */
	Dee_Decref(old_index);
end:
	/* Drop our temporary reference from the old index. */
	Dee_Decref(old_index);
	return result;
new_index_again:
	Dee_Decref(new_index);
	goto old_index_again;
eof_new_index:
	Dee_Decref(new_index);
eof_old_index:
	result = ITER_DONE;
	goto end;
err_new_index:
	Dee_Decref(new_index);
err_old_index:
	result = NULL;
	goto end;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seqiterator_init(SeqIterator *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeTypeObject *tp_iter;
	self->si_index = DeeInt_Zero;
	if (DeeArg_Unpack(argc, argv, "o|o:_GenericIterator", &self->si_seq, &self->si_index))
		goto err;
	if (DeeObject_AssertTypeExact(self->si_index, &DeeInt_Type))
		goto err;
	tp_iter = Dee_TYPE(self->si_seq);
	if (!tp_iter->tp_seq || !tp_iter->tp_seq->tp_get) {
		if (!type_inherit_getitem(tp_iter))
			goto err_not_implemented;
	}
	ASSERT(tp_iter->tp_seq);
	ASSERT(tp_iter->tp_seq->tp_get);
	self->si_getitem = tp_iter->tp_seq->tp_get;
	self->si_size    = DeeObject_SizeObject(self->si_seq);
	if unlikely(!self->si_size)
		goto err;
	Dee_atomic_rwlock_init(&self->si_lock);
	Dee_Incref(self->si_seq);
	Dee_Incref(self->si_index);
	return 0;
err_not_implemented:
	err_unimplemented_operator(tp_iter, OPERATOR_GETITEM);
err:
	return -1;
}


INTDEF DeeTypeObject DeeGenericIterator_Type;


#define DEFINE_SEQITERATOR_COMPARE(name, cmp_name)                 \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL          \
	name(SeqIterator *self, SeqIterator *other) {                  \
		DREF DeeObject *lindex, *rindex, *result;                  \
		if (DeeObject_AssertType(other, &DeeGenericIterator_Type)) \
			return NULL;                                           \
		SeqIterator_LockRead(self);                                \
		lindex = self->si_index;                                   \
		Dee_Incref(lindex);                                        \
		SeqIterator_LockEndRead(self);                             \
		SeqIterator_LockRead(other);                               \
		rindex = other->si_index;                                  \
		Dee_Incref(rindex);                                        \
		SeqIterator_LockEndRead(other);                            \
		result = cmp_name(lindex, rindex);                         \
		Dee_Decref(rindex);                                        \
		Dee_Decref(lindex);                                        \
		return result;                                             \
	}
DEFINE_SEQITERATOR_COMPARE(seqiterator_eq, DeeObject_CompareEqObject)
DEFINE_SEQITERATOR_COMPARE(seqiterator_ne, DeeObject_CompareNeObject)
DEFINE_SEQITERATOR_COMPARE(seqiterator_lo, DeeObject_CompareLoObject)
DEFINE_SEQITERATOR_COMPARE(seqiterator_le, DeeObject_CompareLeObject)
DEFINE_SEQITERATOR_COMPARE(seqiterator_gr, DeeObject_CompareGrObject)
DEFINE_SEQITERATOR_COMPARE(seqiterator_ge, DeeObject_CompareGeObject)
#undef DEFINE_SEQITERATOR_COMPARE



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seqiterator_index_get(SeqIterator *__restrict self) {
	DREF DeeObject *result;
	SeqIterator_LockRead(self);
	result = self->si_index;
	Dee_Incref(result);
	SeqIterator_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seqiterator_index_del(SeqIterator *__restrict self) {
	DREF DeeObject *old_ob;
	Dee_Incref(DeeInt_Zero);
	SeqIterator_LockWrite(self);
	old_ob         = self->si_index;
	self->si_index = DeeInt_Zero;
	SeqIterator_LockEndWrite(self);
	Dee_Decref(old_ob);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seqiterator_index_set(SeqIterator *self, DeeObject *new_index) {
	DREF DeeObject *old_ob;
	if (DeeObject_AssertTypeExact(new_index, &DeeInt_Type))
		goto err;
	Dee_Incref(new_index);
	SeqIterator_LockWrite(self);
	old_ob         = self->si_index;
	self->si_index = new_index;
	SeqIterator_LockEndWrite(self);
	Dee_Decref(old_ob);
	return 0;
err:
	return -1;
}

PRIVATE struct type_getset tpconst seqiterator_getsets[] = {
	TYPE_GETSET("__index__",
	            &seqiterator_index_get,
	            &seqiterator_index_del,
	            &seqiterator_index_set,
	            "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst seqiterator_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(SeqIterator, si_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__size__", STRUCT_OBJECT, offsetof(SeqIterator, si_size), "->?X2?DInt?O"),
	TYPE_MEMBER_END
};

PRIVATE struct type_cmp seqiterator_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&seqiterator_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&seqiterator_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&seqiterator_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&seqiterator_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&seqiterator_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&seqiterator_ge
};

INTERN DeeTypeObject DeeGenericIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_GenericIterator",
	/* .tp_doc      = */ DOC("(seq:?DSequence,index=!0)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&seqiterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&seqiterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&seqiterator_init,
				TYPE_FIXED_ALLOCATOR(SeqIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&seqiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&seqiterator_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&seqiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &seqiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&seqiterator_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ seqiterator_getsets,
	/* .tp_members       = */ seqiterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN WUNUSED DREF DeeObject *DCALL new_empty_sequence_iterator(void) {
	DREF SeqIterator *result;
	result = DeeObject_MALLOC(SeqIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeGenericIterator_Type);
	/* Default-construct the Iterator.
	 * HINT: This function is implemented above and can never fail,
	 *       which is why we don't check it for errors here. */
	seqiterator_ctor(result);
done:
	return (DREF DeeObject *)result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_iterself(DeeObject *__restrict self) {
	int found              = 0;
	DeeTypeObject *tp_iter = Dee_TYPE(self);
	/* To prevent recursion, we must manually search for the proper
	 * callback in order to check if it isn't `seq_getitem' / `seq_size'. */
	while (tp_iter != &DeeSeq_Type) {
		struct type_seq *seq;
		if ((seq = tp_iter->tp_seq) != NULL) {
			if (seq->tp_size && seq->tp_size != &seq_size)
				found |= 1;
			if (seq->tp_get && seq->tp_get != &seq_getitem)
				found |= 2;
			if (found == (1 | 2)) {
				/* Yes, this one's OK! */
				DREF SeqIterator *result;
				result = DeeObject_MALLOC(SeqIterator);
				if unlikely(!result)
					goto err;
				/* Save the getitem operator. */
				result->si_getitem = tp_iter->tp_seq->tp_get;
				if unlikely(!result->si_getitem) {
					tp_iter = Dee_TYPE(self);
					/* TODO: Make use of operator inheritance. */
					while (!tp_iter->tp_seq &&
					       (!tp_iter->tp_seq->tp_get ||
					        tp_iter->tp_seq->tp_get == &seq_getitem))
						tp_iter = DeeType_Base(tp_iter);
					result->si_getitem = tp_iter->tp_seq->tp_get;
				}
				if unlikely(!tp_iter->tp_seq->tp_size) {
					tp_iter = Dee_TYPE(self);
					/* TODO: Make use of operator inheritance. */
					while (!tp_iter->tp_seq &&
					       (!tp_iter->tp_seq->tp_size ||
					        tp_iter->tp_seq->tp_size == &seq_size))
						tp_iter = DeeType_Base(tp_iter);
				}
				/* Figure out the size of the Sequence. */
				result->si_size = (*tp_iter->tp_seq->tp_size)(self);
				if unlikely(!result->si_size) {
					DeeObject_FREE(result);
					goto err;
				}
				/* Assign the initial Iterator index. */
				result->si_index = DeeInt_Zero;
				Dee_Incref(DeeInt_Zero);
				/* Save a reference to the associated Sequence. */
				result->si_seq = self;
				Dee_Incref(self);
				Dee_atomic_rwlock_init(&result->si_lock);
				DeeObject_Init(result, &DeeGenericIterator_Type);
				return (DREF DeeObject *)result;
			}
		}
		tp_iter = DeeType_Base(tp_iter);
	}
	if unlikely(Dee_TYPE(self) == &DeeSeq_Type) {
		/* Special case: Create an empty Iterator.
		 * >> This can happen when someone tries to iterate a symbolic empty-Sequence object. */
		return new_empty_sequence_iterator();
	}
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITERSELF);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
seq_nsi_setrange(DeeObject *self, dssize_t i_begin, dssize_t i_end,
                 DeeObject *values) {
	if unlikely(i_begin < 0 || i_end < 0) {
		size_t seq_len = DeeObject_Size(self);
		if unlikely(seq_len == (size_t)-1)
			goto err;
		if (i_begin < 0)
			i_begin += seq_len;
		if (i_end < 0)
			i_end += seq_len;
	}
	if (DeeNone_Check(values)) {
		return DeeSeq_DelRange(self,
		                       (size_t)i_begin,
		                       (size_t)i_end);
	}
	return DeeSeq_SetRange(self,
	                       (size_t)i_begin,
	                       (size_t)i_end,
	                       values);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
seq_nsi_setrange_n(DeeObject *self, dssize_t i_begin,
                   DeeObject *values) {
	if unlikely(i_begin < 0) {
		size_t seq_len = DeeObject_Size(self);
		if unlikely(seq_len == (size_t)-1)
			goto err;
		i_begin += seq_len;
	}
	if (DeeNone_Check(values))
		return DeeSeq_DelRangeN(self, (size_t)i_begin);
	return DeeSeq_SetRangeN(self, (size_t)i_begin, values);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
seq_delrange(DeeObject *self, DeeObject *start, DeeObject *end) {
	dssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end)) {
		if unlikely(start_index < 0) {
			size_t seq_len = DeeObject_Size(self);
			if unlikely(seq_len == (size_t)-1)
				goto err;
			start_index += seq_len;
		}
		return DeeSeq_DelRangeN(self, (size_t)start_index);
	}
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	if unlikely(start_index < 0 || end_index < 0) {
		size_t seq_len = DeeObject_Size(self);
		if unlikely(seq_len == (size_t)-1)
			goto err;
		if (start_index < 0)
			start_index += seq_len;
		if (end_index < 0)
			end_index += seq_len;
	}
	return DeeSeq_DelRange(self,
	                       (size_t)start_index,
	                       (size_t)end_index);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
seq_setrange(DeeObject *self, DeeObject *start,
             DeeObject *end, DeeObject *values) {
	dssize_t start_index, end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return seq_nsi_setrange_n(self, start_index, values);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return seq_nsi_setrange(self, start_index, end_index, values);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
seq_nsi_insert_vec(DeeObject *self, size_t index,
                   size_t objc, DeeObject *const *objv) {
	DeeObject *shared_vector;
	int result;
	shared_vector = DeeSharedVector_NewShared(objc, objv);
	if unlikely(!shared_vector)
		goto err;
	result = DeeSeq_InsertAll(self, index, shared_vector);
	DeeSharedVector_Decref(shared_vector);
	return result;
err:
	return -1;
}

PRIVATE struct type_nsi tpconst seq_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&DeeSeq_Size,
			/* .nsi_getsize_fast = */ (dfunptr_t)NULL,
			/* .nsi_getitem      = */ (dfunptr_t)&DeeSeq_GetItem,
			/* .nsi_delitem      = */ (dfunptr_t)&DeeSeq_DelItem,
			/* .nsi_setitem      = */ (dfunptr_t)&DeeSeq_SetItem,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)&seq_nsi_getrange,
			/* .nsi_getrange_n   = */ (dfunptr_t)&seq_nsi_getrange_n,
			/* .nsi_setrange     = */ (dfunptr_t)&seq_nsi_setrange,
			/* .nsi_setrange_n   = */ (dfunptr_t)&seq_nsi_setrange_n,
			/* .nsi_find         = */ (dfunptr_t)&DeeSeq_Find,
			/* .nsi_rfind        = */ (dfunptr_t)&DeeSeq_RFind,
			/* .nsi_xch          = */ (dfunptr_t)&DeeSeq_XchItem,
			/* .nsi_insert       = */ (dfunptr_t)&DeeSeq_Insert,
			/* .nsi_insertall    = */ (dfunptr_t)&DeeSeq_InsertAll,
			/* .nsi_insertvec    = */ (dfunptr_t)&seq_nsi_insert_vec,
			/* .nsi_pop          = */ (dfunptr_t)&DeeSeq_PopItem,
			/* .nsi_erase        = */ (dfunptr_t)&DeeSeq_Erase,
			/* .nsi_remove       = */ (dfunptr_t)&DeeSeq_Remove,
			/* .nsi_rremove      = */ (dfunptr_t)&DeeSeq_RRemove,
			/* .nsi_removeall    = */ (dfunptr_t)&DeeSeq_RemoveAll,
			/* .nsi_removeif     = */ (dfunptr_t)&DeeSeq_RemoveIf
		}
	}
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seq_delitem(DeeObject *self, DeeObject *index) {
	size_t i;
	if (DeeObject_AsSize(index, &i))
		goto err;
	return DeeSeq_DelItem(self, i);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
seq_setitem(DeeObject *self, DeeObject *index, DeeObject *value) {
	size_t i;
	if (DeeObject_AsSize(index, &i))
		goto err;
	return DeeSeq_SetItem(self, i, value);
err:
	return -1;
}

PRIVATE struct type_seq seq_seq = {
	/* .tp_iter_self = */ &seq_iterself,
	/* .tp_size      = */ &seq_size,
	/* .tp_contains  = */ &seq_tpcontains,
	/* .tp_get       = */ &seq_getitem,
	/* .tp_del       = */ &seq_delitem,
	/* .tp_set       = */ &seq_setitem,
	/* .tp_range_get = */ &seq_getrange,
	/* .tp_range_del = */ &seq_delrange,
	/* .tp_range_set = */ &seq_setrange,
	/* .tp_nsi       = */ &seq_nsi,
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_Eq(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *lhs_iter;
	size_t lhs_size;
	if ((lhs_size = DeeFastSeq_GetSize(lhs)) != DEE_FASTSEQ_NOTFAST)
		return DeeSeq_EqFS(lhs, lhs_size, rhs);
	lhs_iter = DeeObject_IterSelf(lhs);
	if unlikely(!lhs_iter)
		return -1;
	result = DeeSeq_EqIS(lhs_iter, rhs);
	Dee_Decref(lhs_iter);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_Compare(DeeObject *lhs, DeeObject *rhs) {
	int result;
	DREF DeeObject *lhs_iter;
	size_t lhs_size;
	if ((lhs_size = DeeFastSeq_GetSize(lhs)) != DEE_FASTSEQ_NOTFAST)
		return DeeSeq_CompareFS(lhs, lhs_size, rhs);
	lhs_iter = DeeObject_IterSelf(lhs);
	if unlikely(!lhs_iter)
		return -2;
	result = DeeSeq_CompareIS(lhs_iter, rhs);
	Dee_Decref(lhs_iter);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
seq_hash(DeeObject *__restrict self) {
	dhash_t result;
	DREF DeeObject *iter, *elem;
	iter = DeeObject_IterSelf(self);
	if unlikely(!iter)
		goto err;
	elem = DeeObject_IterNext(iter);
	if (!ITER_ISOK(elem)) {
		Dee_Decref(iter);
		if (!elem)
			goto err;
		return 0; /* Empty sequence hash */
	}
	result = DeeObject_Hash(elem);
	Dee_Decref(elem);
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		result = Dee_HashCombine(result, DeeObject_Hash(elem));
		Dee_Decref(elem);
	}
	Dee_Decref(iter);
	if unlikely(!elem)
		goto err;
	return result;
err:
	DeeError_Print("Unhandled error in `Set.operator hash'\n",
	               ERROR_PRINT_DOHANDLE);
	return DeeObject_HashGeneric(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seq_eq(DeeObject *self, DeeObject *other) {
	int result = DeeSeq_Eq(self, other);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seq_ne(DeeObject *self, DeeObject *other) {
	int result = DeeSeq_Eq(self, other);
	if unlikely(result < 0)
		goto err;
	return_bool_(!result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seq_lo(DeeObject *self, DeeObject *other) {
	int result = DeeSeq_Compare(self, other);
	if unlikely(result == -2)
		goto err;
	return_bool_(result < 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seq_le(DeeObject *self, DeeObject *other) {
	int result = DeeSeq_Compare(self, other);
	if unlikely(result == -2)
		goto err;
	return_bool_(result <= 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seq_gr(DeeObject *self, DeeObject *other) {
	int result = DeeSeq_Compare(self, other);
	if unlikely(result == -2)
		goto err;
	return_bool_(result > 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seq_ge(DeeObject *self, DeeObject *other) {
	int result = DeeSeq_Compare(self, other);
	if unlikely(result == -2)
		goto err;
	return_bool_(result >= 0);
err:
	return NULL;
}

PRIVATE struct type_cmp seq_cmp = {
	/* .tp_hash = */ &seq_hash,
	/* .tp_eq   = */ &seq_eq,
	/* .tp_ne   = */ &seq_ne,
	/* .tp_lo   = */ &seq_lo,
	/* .tp_le   = */ &seq_le,
	/* .tp_gr   = */ &seq_gr,
	/* .tp_ge   = */ &seq_ge
};

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seq_mul(DeeObject *self, DeeObject *countob) {
	size_t count;
	if (DeeObject_AsSize(countob, &count))
		goto err;
	return DeeSeq_Repeat(self, count);
err:
	return NULL;
}

PRIVATE struct type_math seq_math = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
	/* .tp_inv         = */ NULL,
	/* .tp_pos         = */ NULL,
	/* .tp_neg         = */ NULL,
	/* .tp_add         = */ &DeeSeq_Concat,
	/* .tp_sub         = */ NULL,
	/* .tp_mul         = */ &seq_mul,
	/* .tp_div         = */ NULL,
	/* .tp_mod         = */ NULL,
	/* .tp_shl         = */ NULL,
	/* .tp_shr         = */ NULL,
	/* .tp_and         = */ NULL,
	/* .tp_or          = */ NULL,
	/* .tp_xor         = */ NULL,
	/* .tp_pow         = */ NULL,
	/* .tp_inc         = */ NULL,
	/* .tp_dec         = */ NULL,
	/* .tp_inplace_add = */ &DeeSeq_InplaceExtend,
	/* .tp_inplace_sub = */ NULL,
	/* .tp_inplace_mul = */ &DeeSeq_InplaceRepeat,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL,
	/* .tp_inplace_shr = */ NULL,
	/* .tp_inplace_and = */ NULL,
	/* .tp_inplace_or  = */ NULL,
	/* .tp_inplace_xor = */ NULL,
	/* .tp_inplace_pow = */ NULL,
};


PRIVATE WUNUSED NONNULL((1)) bool DCALL
sequence_should_use_getitem(DeeTypeObject *__restrict self) {
	DeeTypeObject *iter, *base;
	int found;
	if (self == &DeeSeq_Type)
		return false;
	if (DeeType_IsInherited(self, &DeeMapping_Type))
		return false;
	iter  = self;
	found = 0;
	do {
		struct type_seq *seq;
		base = DeeType_Base(iter);
		if ((seq = iter->tp_seq) != NULL) {
			if (seq->tp_get && seq->tp_get != &seq_getitem &&
			    (!base || !base->tp_seq || seq->tp_get != base->tp_seq->tp_get))
				found |= 1;
			if (seq->tp_size && seq->tp_size != &seq_size &&
			    (!base || !base->tp_seq || seq->tp_size != base->tp_seq->tp_size))
				found |= 2;
			if (found == (1 | 2))
				return true;
			if (seq->tp_iter_self &&
			    (!base || !base->tp_seq || seq->tp_iter_self != base->tp_seq->tp_iter_self))
				break;
		}
	} while (base && (iter = base) != &DeeSeq_Type);
	return false;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
seq_printrepr(DeeObject *__restrict self, dformatprinter printer, void *arg) {
#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0
	dssize_t temp, result = 0;
	bool is_first;
	DREF DeeObject *iterator, *elem;
	if (sequence_should_use_getitem(Dee_TYPE(self))) {
		size_t i, size;
		size = DeeObject_Size(self);
		if unlikely(size == (size_t)-1) {
			/* If the size operator isn't actually implemented, try to use iterators instead! */
			if (DeeError_Catch(&DeeError_NotImplemented))
				goto do_try_iterators;
			goto err_m1;
		}
		if (!size) {
			DO(err, DeeFormat_PRINT(printer, arg, "{ }"));
		} else {
			DO(err, DeeFormat_PRINT(printer, arg, "{ "));
			for (i = 0; i < size; ++i) {
				if (i != 0)
					DO(err, DeeFormat_PRINT(printer, arg, ", "));
				elem = DeeObject_GetItemIndex(self, i);
				if likely(elem) {
					DO(err_elem, DeeFormat_PrintObjectRepr(printer, arg, elem));
					Dee_Decref(elem);
				} else if (DeeError_Catch(&DeeError_UnboundItem)) {
					DO(err, DeeFormat_PRINT(printer, arg, "<unbound>"));
				} else if (DeeError_Catch(&DeeError_IndexError)) {
					/* Assume that the Sequence got re-sized while we were iterating it. */
					if unlikely(!i) {
						DO(err, DeeFormat_PRINT(printer, arg, "}"));
						goto done;
					}
					break;
				} else {
					goto err_m1;
				}
			}
			DO(err, DeeFormat_PRINT(printer, arg, " }"));
		}
	} else {
do_try_iterators:
		iterator = DeeObject_IterSelf(self);
		if unlikely(!iterator)
			goto err_m1;
		DO(err_iterator, DeeFormat_PRINT(printer, arg, "{ "));
		is_first = true;
		while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
			if (!is_first)
				DO(err_iterator_elem, DeeFormat_PRINT(printer, arg, ", "));
			DO(err_iterator_elem, DeeFormat_PrintObjectRepr(printer, arg, elem));
			Dee_Decref(elem);
			is_first = false;
			if (DeeThread_CheckInterrupt())
				goto err_m1_iterator;
		}
		if unlikely(!elem)
			goto err_m1_iterator;
		Dee_Decref(iterator);
		if (is_first) {
			DO(err, DeeFormat_PRINT(printer, arg, "}"));
		} else {
			DO(err, DeeFormat_PRINT(printer, arg, " }"));
		}
	}
done:
	return result;
err_elem:
	Dee_Decref(elem);
	goto err;
err_iterator_elem:
	Dee_Decref(elem);
err_iterator:
	Dee_Decref(iterator);
err:
	return temp;
err_m1_iterator:
	temp = -1;
	goto err_iterator;
err_m1:
	temp = -1;
	goto err;
#undef DO
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
seq_iterator_get(DeeTypeObject *__restrict self) {
	DeeTypeObject *iter, *base;
	int found;
	/* Special case: Accessing the `Iterator' field of the raw `Sequence' type
	 *               will yield the (intended) base-class for all iterators. */
	if (self == &DeeSeq_Type)
		return_reference_(&DeeIterator_Type);
	iter = self, found = 0;
	do {
		struct type_seq *seq;
		base = DeeType_Base(iter);
		if ((seq = iter->tp_seq) != NULL) {
			/* If sub-classes override both the get+size operators, then our stub-version is used. */
			if (seq->tp_get && seq->tp_get != &seq_getitem &&
			    (!base || !base->tp_seq || seq->tp_get != base->tp_seq->tp_get))
				found |= 1;
			if (seq->tp_size && seq->tp_size != &seq_size &&
			    (!base || !base->tp_seq || seq->tp_size != base->tp_seq->tp_size))
				found |= 2;
			/* If any sub-class that isn't the Sequence type itself implements
			 * the Iterator interface, then it should be responsible for providing
			 * the `Iterator' class member.
			 * With that in mind, us being here probably indicates that such a member is
			 * missing, meaning that we should fail and act as though there's no such field. */
			if (seq->tp_iter_self &&
			    (!base || !base->tp_seq || seq->tp_iter_self != base->tp_seq->tp_iter_self))
				goto fail;
		}
	} while (base && (iter = base) != &DeeSeq_Type);
	/* If we've found everything that's need to implement
	 * the `generic_iterator' type, then that's the one! */
	if (found == (1 | 2))
		return_reference_(&DeeGenericIterator_Type);
fail:
	err_unknown_attribute(self, STR_Iterator, ATTR_ACCESS_GET);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
seq_frozen_get(DeeTypeObject *__restrict self) {
	int error;
	DREF DeeTypeObject *result;
	struct attribute_info info;
	struct attribute_lookup_rules rules;
	rules.alr_name       = "Frozen";
	rules.alr_hash       = Dee_HashPtr("Frozen", COMPILER_STRLEN("Frozen"));
	rules.alr_decl       = NULL;
	rules.alr_perm_mask  = ATTR_PERMGET | ATTR_IMEMBER;
	rules.alr_perm_value = ATTR_PERMGET | ATTR_IMEMBER;
	error = DeeAttribute_Lookup(Dee_TYPE(self),
	                            (DeeObject *)self,
	                            &info,
	                            &rules);
	if unlikely(error < 0)
		goto err;
	if (error != 0) {
		/* If the type doesn't provide its own override for `Frozen', the default
		 * implementation provided by us will return information via a tuple instead. */
		return_reference_(&DeeTuple_Type);
	}
	if (info.a_attrtype) {
		result = info.a_attrtype;
		Dee_Incref(result);
	} else if (info.a_decl == (DeeObject *)&DeeSeq_Type) {
		/* We've the ones implementing the attribute, and since we know that we're
		 * already implementing it by casting ourself to a tuple, inform the caller
		 * of exactly that. */
		result = &DeeTuple_Type;
		Dee_Incref(&DeeTuple_Type);
	} else {
		if (info.a_doc) {
			/* TODO: Use doc meta-information to determine the return type! */
		}
		/* Fallback: just tell the caller what they already know: a Sequence will be returned... */
		result = &DeeSeq_Type;
		Dee_Incref(&DeeSeq_Type);
	}
	attribute_info_fini(&info);
	return result;
err:
	return NULL;
}

PRIVATE struct type_getset tpconst seq_class_getsets[] = {
	TYPE_GETTER(STR_Iterator, &seq_iterator_get,
	            "->?DType\n"
	            "Returns the Iterator class used by instances of @this Sequence type\n"
	            "Should a sub-class implement its own Iterator, this attribute should be overwritten"),
	TYPE_GETTER("Frozen", &seq_frozen_get,
	            "->?DType\n"
	            "Returns the type of Sequence returned by the #i:frozen property"),
	TYPE_GETSET_END
};


/* === General-purpose Sequence methods. === */
#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_at_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *index;
	if (DeeArg_Unpack(argc, argv, "o:at", &index))
		goto err;
	return DeeObject_GetItem(self, index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_empty_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":empty"))
		goto err;
	result = DeeObject_Bool(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_non_empty_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":non_empty"))
		goto err;
	result = DeeObject_Bool(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result != 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_front_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":front"))
		goto err;
	return DeeObject_GetAttr(self, (DeeObject *)&str_first);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_back_deprecated(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":back"))
		goto err;
	return DeeObject_GetAttr(self, (DeeObject *)&str_last);
err:
	return NULL;
}
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_reduce(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *combine, *init = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:reduce", &combine, &init))
		goto err;
	return DeeSeq_Reduce(self, combine, init);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_filter(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *pred_keep;
	if (DeeArg_Unpack(argc, argv, "o:filter", &pred_keep))
		goto err;
	return DeeSeq_Filter(self, pred_keep);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_sum(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":sum"))
		goto err;
	return DeeSeq_Sum(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_any(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":any"))
		goto err;
	result = DeeSeq_Any(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_all(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":all"))
		goto err;
	result = DeeSeq_All(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_parity(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":parity"))
		goto err;
	result = DeeSeq_Parity(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN DEFINE_KWLIST(seq_sort_kwlist, { K(key), KEND });
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_min(DeeObject *self, size_t argc,
        DeeObject *const *argv, DeeObject *kw) {
	DeeObject *key = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_sort_kwlist, "|o:min", &key))
		goto err;
	if (DeeNone_Check(key))
		key = NULL;
	return DeeSeq_Min(self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_max(DeeObject *self, size_t argc,
        DeeObject *const *argv, DeeObject *kw) {
	DeeObject *key = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_sort_kwlist, "|o:max", &key))
		goto err;
	if (DeeNone_Check(key))
		key = NULL;
	return DeeSeq_Max(self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_count(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *elem, *key = Dee_None;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "o|o:count", &elem, &key))
		goto err;
	if (DeeNone_Check(key)) {
		result = DeeSeq_Count(self, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_Count(self, elem, key);
		Dee_Decref(elem);
	}
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_locate(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *elem, *key = Dee_None;
	DREF DeeObject *result;
	if (DeeArg_Unpack(argc, argv, "o|o:locate", &elem, &key))
		goto err;
	if (DeeNone_Check(key)) {
		result = DeeSeq_Locate(self, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_Locate(self, elem, key);
		Dee_Decref(elem);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_rlocate(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *elem, *key = Dee_None;
	DREF DeeObject *result;
	if (DeeArg_Unpack(argc, argv, "o|o:rlocate", &elem, &key))
		goto err;
	if (DeeNone_Check(key)) {
		result = DeeSeq_RLocate(self, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_RLocate(self, elem, key);
		Dee_Decref(elem);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_locateall(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *elem, *key = Dee_None;
	DREF DeeObject *result;
	if (DeeArg_Unpack(argc, argv, "o|o:locateall", &elem, &key))
		goto err;
	if (DeeNone_Check(key)) {
		result = DeeSeq_LocateAll(self, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_LocateAll(self, elem, key);
		Dee_Decref(elem);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_transform(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *transformation;
	if (DeeArg_Unpack(argc, argv, "o:transform", &transformation))
		goto err;
	return DeeSeq_Transform(self, transformation);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_contains(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *elem, *key = Dee_None;
	int result;
	if (DeeArg_Unpack(argc, argv, "o|o:contains", &elem, &key))
		goto err;
	/* Without a key function, invoke the regular contains-operator. */
	if (DeeNone_Check(key))
		return DeeObject_ContainsObject(self, elem);
	elem = DeeObject_Call(key, 1, &elem);
	if unlikely(!elem)
		goto err;
	result = DeeSeq_Contains(self, elem, key);
	Dee_Decref(elem);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_startswith(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *elem, *key = Dee_None;
	int result;
	if (DeeArg_Unpack(argc, argv, "o|o:startswith", &elem, &key))
		goto err;
	if (DeeNone_Check(key)) {
		result = DeeSeq_StartsWith(self, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_StartsWith(self, elem, key);
		Dee_Decref(elem);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_endswith(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *elem, *key = Dee_None;
	int result;
	if (DeeArg_Unpack(argc, argv, "o|o:endswith", &elem, &key))
		goto err;
	if (DeeNone_Check(key)) {
		result = DeeSeq_EndsWith(self, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_EndsWith(self, elem, key);
		Dee_Decref(elem);
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

#ifdef __OPTIMIZE_SIZE__
#define get_sequence_find_args(name, argc, argv, p_elem, p_key, p_start, p_end) \
	get_sequence_find_args_kw(name, argc, argv, NULL, p_elem, p_key, p_start, p_end)
#else /* __OPTIMIZE_SIZE__ */
PRIVATE WUNUSED NONNULL((1, 4, 5, 6, 7)) int DCALL
get_sequence_find_args(char const *__restrict name,
                       size_t argc, DeeObject *const *argv,
                       DeeObject **__restrict p_elem,
                       DeeObject **__restrict p_key,
                       size_t *__restrict p_start,
                       size_t *__restrict p_end) {
	switch (argc) {

	case 1:
		*p_elem  = argv[0];
		*p_key   = NULL;
		*p_start = 0;
		*p_end   = (size_t)-1;
		break;

	case 2:
		if (DeeInt_Check(argv[1])) {
			if (DeeObject_AsSSize(argv[1], (dssize_t *)p_start))
				goto err;
			*p_key = NULL;
		} else {
			*p_key   = argv[1];
			*p_start = 0;
			if (DeeNone_Check(*p_key))
				*p_key = NULL;
		}
		*p_elem = argv[0];
		*p_end  = (size_t)-1;
		break;

	case 3:
		if (DeeObject_AsSSize(argv[1], (dssize_t *)p_start))
			goto err;
		if (DeeInt_Check(argv[2])) {
			if (DeeObject_AsSSize(argv[2], (dssize_t *)p_end))
				goto err;
			*p_key = NULL;
		} else {
			*p_key = argv[2];
			*p_end = (size_t)-1;
			if (DeeNone_Check(*p_key))
				*p_key = NULL;
		}
		*p_elem = argv[0];
		break;

	case 4:
		if (DeeObject_AsSSize(argv[1], (dssize_t *)p_start))
			goto err;
		if (DeeObject_AsSSize(argv[2], (dssize_t *)p_end))
			goto err;
		*p_elem = argv[0];
		*p_key  = argv[3];
		if (DeeNone_Check(*p_key))
			*p_key = NULL;
		break;

	default:
		err_invalid_argc(name, argc, 1, 4);
err:
		return -1;
	}
	return 0;
}
#endif /* !__OPTIMIZE_SIZE__ */

/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
print define_Dee_HashStr("elem");
print define_Dee_HashStr("start");
print define_Dee_HashStr("end");
print define_Dee_HashStr("key");
print define_Dee_HashStr("defl");
]]]*/
#define Dee_HashStr__elem _Dee_HashSelect(UINT32_C(0x1aacf22d), UINT64_C(0x705652c4aed9308a))
#define Dee_HashStr__start _Dee_HashSelect(UINT32_C(0xa2ed6890), UINT64_C(0x80b621ce3c3982d5))
#define Dee_HashStr__end _Dee_HashSelect(UINT32_C(0x37fb4a05), UINT64_C(0x6de935c204dc3d01))
#define Dee_HashStr__key _Dee_HashSelect(UINT32_C(0xe29c6a44), UINT64_C(0x612dd31212e90587))
#define Dee_HashStr__defl _Dee_HashSelect(UINT32_C(0x4353f18), UINT64_C(0x655c26b85fe0c07b))
/*[[[end]]]*/

/* (elem,key:?DCallable=!N)
 * (elem,start:?Dint,key:?DCallable=!N)
 * (elem,start:?Dint,end:?Dint,key:?DCallable=!N) */
PRIVATE WUNUSED NONNULL((1, 5, 6, 7, 8)) int DCALL
get_sequence_find_args_kw(char const *__restrict name,
                          size_t argc, DeeObject *const *argv, DeeObject *kw,
                          DeeObject **__restrict p_elem,
                          DeeObject **__restrict p_key,
                          size_t *__restrict p_start,
                          size_t *__restrict p_end) {
	DREF DeeObject *temp;
	DeeKwArgs kwargs;
#ifndef __OPTIMIZE_SIZE__
	if (!kw) {
		/* Fastpass */
		return get_sequence_find_args(name, argc, argv,
		                              p_elem, p_key,
		                              p_start, p_end);
	}
#endif /* !__OPTIMIZE_SIZE__ */
	if (DeeKwArgs_Init(&kwargs, &argc, argv, kw))
		goto err;
	switch (argc) {

	case 0:
		if unlikely((*p_elem = DeeKwArgs_GetString(&kwargs, "elem", Dee_HashStr__elem)) == NULL)
			goto err;
		Dee_DecrefNokill(*p_elem); /* FIXME: This can break for custom mapping types! */
check_kw_start_end_key:
		if unlikely((temp = DeeKwArgs_GetStringDef(&kwargs, "start", Dee_HashStr__start, DeeInt_Zero)) == NULL)
			goto err;
		if (DeeObject_AsSSize(temp, (dssize_t *)p_start))
			goto err_temp;
		Dee_Decref(temp);
check_kw_end_key:
		if unlikely((temp = DeeKwArgs_GetStringDef(&kwargs, "end", Dee_HashStr__end, DeeInt_MinusOne)) == NULL)
			goto err;
		if (DeeObject_AsSSize(temp, (dssize_t *)p_end))
			goto err_temp;
		Dee_Decref(temp);
/*check_kw_key:*/
		if unlikely((*p_key = DeeKwArgs_GetStringDef(&kwargs, "key", Dee_HashStr__key, Dee_None)) == NULL)
			goto err;
		if (DeeNone_Check(*p_key)) {
			Dee_DecrefNokill(Dee_None);
			*p_key = NULL;
		} else {
			Dee_DecrefNokill(*p_key); /* FIXME: This can break for custom mapping types! */
		}
		break;

	case 1:
		*p_elem = argv[0];
		goto check_kw_start_end_key;

	case 2:
		*p_elem = argv[0];
		if (DeeInt_Check(argv[1])) {
			if (DeeObject_AsSSize(argv[1], (dssize_t *)p_start))
				goto err;
			goto check_kw_end_key;
		}
		*p_key   = argv[1];
		*p_start = 0;
		if (DeeNone_Check(*p_key))
			*p_key = NULL;
check_kw_end:
		if unlikely((temp = DeeKwArgs_GetStringDef(&kwargs, "end", Dee_HashStr__end, DeeInt_MinusOne)) == NULL)
			goto err;
		if (DeeObject_AsSSize(temp, (dssize_t *)p_end))
			goto err_temp;
		Dee_Decref(temp);
		break;

	case 3:
		*p_elem = argv[0];
		if (DeeObject_AsSSize(argv[1], (dssize_t *)p_start))
			goto err;
		if (DeeInt_Check(argv[2])) {
			if (DeeObject_AsSSize(argv[2], (dssize_t *)p_end))
				goto err;
			goto check_kw_end_key;
		}
		*p_key = argv[2];
		if (DeeNone_Check(*p_key))
			*p_key = NULL;
		goto check_kw_end;

	case 4:
		if (DeeObject_AsSSize(argv[1], (dssize_t *)p_start))
			goto err;
		if (DeeObject_AsSSize(argv[2], (dssize_t *)p_end))
			goto err;
		*p_elem = argv[0];
		*p_key  = argv[3];
		if (DeeNone_Check(*p_key))
			*p_key = NULL;
		break;

	default:
		err_invalid_argc(name, argc, 1, 4);
		goto err;
	}
	return DeeKwArgs_Done(&kwargs, argc, name);
err_temp:
	Dee_Decref(temp);
err:
	return -1;
}


/* (elem,key:?DCallable=!N,defl?)
 * (elem,start:?Dint,key:?DCallable=!N,defl?)
 * (elem,start:?Dint,end:?Dint,key:?DCallable=!N,defl?) */
PRIVATE WUNUSED NONNULL((1, 5, 6, 7, 8, 9)) int DCALL
get_sequence_find_defl_args_kw(char const *__restrict name,
                               size_t argc, DeeObject *const *argv, DeeObject *kw,
                               DeeObject **__restrict p_elem,
                               DeeObject **__restrict p_key,
                               size_t *__restrict p_start,
                               size_t *__restrict p_end,
                               DeeObject **__restrict p_defl) {
	DREF DeeObject *temp;
	DeeKwArgs kwargs;
	if (DeeKwArgs_Init(&kwargs, &argc, argv, kw))
		goto err;
	switch (argc) {

	case 0:
		if unlikely((*p_elem = DeeKwArgs_GetString(&kwargs, "elem", Dee_HashStr__elem)) == NULL)
			goto err;
		Dee_DecrefNokill(*p_elem); /* FIXME: This can break for custom mapping types! */
check_kw_start_end_key_defl:
		if unlikely((temp = DeeKwArgs_GetStringDef(&kwargs, "start", Dee_HashStr__start, DeeInt_Zero)) == NULL)
			goto err;
		if (DeeObject_AsSSize(temp, (dssize_t *)p_start))
			goto err_temp;
		Dee_Decref(temp);
check_kw_end_key_defl:
		if unlikely((temp = DeeKwArgs_GetStringDef(&kwargs, "end", Dee_HashStr__end, DeeInt_MinusOne)) == NULL)
			goto err;
		if (DeeObject_AsSSize(temp, (dssize_t *)p_end))
			goto err_temp;
		Dee_Decref(temp);
/*check_kw_key_defl:*/
		if unlikely((*p_key = DeeKwArgs_GetStringDef(&kwargs, "key", Dee_HashStr__key, Dee_None)) == NULL)
			goto err;
		if (DeeNone_Check(*p_key)) {
			Dee_DecrefNokill(Dee_None);
			*p_key = NULL;
		} else {
			Dee_DecrefNokill(*p_key); /* FIXME: This can break for custom mapping types! */
		}
check_kw_defl:
		if unlikely((*p_defl = DeeKwArgs_GetStringDef(&kwargs, "defl", Dee_HashStr__defl, ITER_DONE)) == NULL)
			goto err;
		if (*p_defl == ITER_DONE) {
			*p_defl = NULL;
		} else {
			Dee_DecrefNokill(*p_defl); /* FIXME: This can break for custom mapping types! */
		}
		break;

	case 1:
		*p_elem = argv[0];
		goto check_kw_start_end_key_defl;

	case 2:
		*p_elem = argv[0];
		if (DeeInt_Check(argv[1])) {
			if (DeeObject_AsSSize(argv[1], (dssize_t *)p_start))
				goto err;
			goto check_kw_end_key_defl;
		}
		*p_key   = argv[1];
		*p_start = 0;
		if (DeeNone_Check(*p_key))
			*p_key = NULL;
check_kw_end_defl:
		if unlikely((temp = DeeKwArgs_GetStringDef(&kwargs, "end", Dee_HashStr__end, DeeInt_MinusOne)) == NULL)
			goto err;
		if (DeeObject_AsSSize(temp, (dssize_t *)p_end))
			goto err_temp;
		Dee_Decref(temp);
		goto check_kw_defl;

	case 3:
		*p_elem = argv[0];
		if (!DeeInt_Check(argv[1])) {
			/* (elem,key:?DCallable=!N,defl?) */
			*p_key  = argv[1];
			*p_defl = argv[2];
			break;
		}
		if (DeeObject_AsSSize(argv[1], (dssize_t *)p_start))
			goto err;
		if (DeeInt_Check(argv[2])) {
			if (DeeObject_AsSSize(argv[2], (dssize_t *)p_end))
				goto err;
			goto check_kw_end_key_defl;
		}
		*p_key = argv[2];
		if (DeeNone_Check(*p_key))
			*p_key = NULL;
		goto check_kw_end_defl;

	case 4:
		if (DeeObject_AsSSize(argv[1], (dssize_t *)p_start))
			goto err;
		if (DeeObject_AsSSize(argv[2], (dssize_t *)p_end))
			goto err;
		*p_elem = argv[0];
		*p_key  = argv[3];
		if (DeeNone_Check(*p_key))
			*p_key = NULL;
		goto check_kw_defl;

	case 5:
		if (DeeObject_AsSSize(argv[1], (dssize_t *)p_start))
			goto err;
		if (DeeObject_AsSSize(argv[2], (dssize_t *)p_end))
			goto err;
		*p_elem = argv[0];
		*p_key  = argv[3];
		if (DeeNone_Check(*p_key))
			*p_key = NULL;
		*p_defl = argv[4];
		break;

	default:
		err_invalid_argc(name, argc, 1, 4);
		goto err;
	}
	return DeeKwArgs_Done(&kwargs, argc, name);
err_temp:
	Dee_Decref(temp);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_find(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *elem, *key;
	size_t result, start, end;
	if (get_sequence_find_args_kw("find", argc, argv, kw, &elem, &key, &start, &end))
		goto err;
	if (!key) {
		result = DeeSeq_Find(self, start, end, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_Find(self, start, end, elem, key);
		Dee_Decref(elem);
	}
	if ((dssize_t)result < 0) {
		if unlikely(result == (size_t)-2)
			goto err;
		if unlikely(result == (size_t)-1)
			return_reference_(DeeInt_MinusOne);
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_rfind(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *elem, *key;
	size_t result, start, end;
	if (get_sequence_find_args_kw("rfind", argc, argv, kw, &elem, &key, &start, &end))
		goto err;
	if (!key) {
		result = DeeSeq_RFind(self, start, end, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_RFind(self, start, end, elem, key);
		Dee_Decref(elem);
	}
	if ((dssize_t)result < 0) {
		if unlikely(result == (size_t)-2)
			goto err;
		if unlikely(result == (size_t)-1)
			return_reference_(DeeInt_MinusOne);
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_index(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *elem, *key;
	size_t result, start, end;
	if (get_sequence_find_args_kw(STR_index, argc, argv, kw, &elem, &key, &start, &end))
		goto err;
	if (!key) {
		result = DeeSeq_Find(self, start, end, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_Find(self, start, end, elem, key);
		Dee_Decref(elem);
	}
	if unlikely((dssize_t)result < 0) {
		if unlikely(result == (size_t)-2)
			goto err;
		if unlikely(result == (size_t)-1)
			goto err_not_found;
	}
	return DeeInt_NewSize(result);
err_not_found:
	err_item_not_found(self, elem);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_rindex(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *elem, *key;
	size_t result, start, end;
	if (get_sequence_find_args_kw("rindex", argc, argv, kw, &elem, &key, &start, &end))
		goto err;
	if (!key) {
		result = DeeSeq_RFind(self, start, end, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_RFind(self, start, end, elem, key);
		Dee_Decref(elem);
	}
	if unlikely((dssize_t)result < 0) {
		if unlikely(result == (size_t)-2)
			goto err;
		if unlikely(result == (size_t)-1)
			goto err_not_found;
	}
	return DeeInt_NewSize(result);
err_not_found:
	err_item_not_found(self, elem);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_reversed(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":reversed"))
		goto err;
	return DeeSeq_Reversed(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_sorted(DeeObject *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	DeeObject *key = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_sort_kwlist, "|o:sorted", &key))
		goto err;
	if (DeeNone_Check(key))
		key = NULL;
	return DeeSeq_Sorted(self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_segments(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t segsize;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":segments", &segsize))
		goto err;
	if unlikely(!segsize)
		goto err_invalid_segsize;
	return DeeSeq_Segments(self, segsize);
err_invalid_segsize:
	err_invalid_segment_size(segsize);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_distribute(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t segsize, mylen;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":distribute", &segsize))
		goto err;
	if unlikely(!segsize)
		goto err_invalid_segsize;
	mylen = DeeObject_Size(self);
	if unlikely(mylen == (size_t)-1)
		goto err;
	mylen += segsize - 1;
	mylen /= segsize;
	if unlikely(!mylen)
		return_empty_seq;
	return DeeSeq_Segments(self, mylen);
err_invalid_segsize:
	err_invalid_distribution_count(segsize);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_combinations(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t r;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":combinations", &r))
		goto err;
	return DeeSeq_Combinations(self, r);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_repeatcombinations(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t r;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":repeatcombinations", &r))
		goto err;
	return DeeSeq_RepeatCombinations(self, r);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_permutations(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t r;
	DeeObject *arg = Dee_None;
	if (DeeArg_Unpack(argc, argv, "|o:permutations", &arg))
		goto err;
	if (DeeNone_Check(arg))
		return DeeSeq_Permutations(self);
	if (DeeObject_AsSize(arg, &r))
		goto err;
	return DeeSeq_Permutations2(self, r);
err:
	return NULL;
}


/* Mutable-Sequence functions */
INTERN DEFINE_KWLIST(seq_insert_kwlist, { K(index), K(item), KEND });
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_insert(DeeObject *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *item;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_insert_kwlist, UNPuSIZ "o:insert", &index, &item))
		goto err;
	if (DeeSeq_Insert(self, index, item))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN DEFINE_KWLIST(seq_insertall_kwlist, { K(index), K(items), KEND });
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_insertall(DeeObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *items;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_insertall_kwlist, UNPuSIZ "o:insertall", &index, &items))
		goto err;
	if (DeeSeq_InsertAll(self, index, items))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN DEFINE_KWLIST(seq_erase_kwlist, { K(index), K(count), KEND });
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_erase(DeeObject *self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	size_t index, count = 1, result;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_erase_kwlist, UNPuSIZ "|" UNPuSIZ ":erase", &index, &count))
		goto err;
	result = DeeSeq_Erase(self, index, count);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN DEFINE_KWLIST(seq_xch_kwlist, { K(index), K(value), KEND });
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_xch(DeeObject *self, size_t argc,
        DeeObject *const *argv, DeeObject *kw) {
	size_t index;
	DeeObject *value;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_xch_kwlist, UNPuSIZ "o:xch", &index, &value))
		goto err;
	return DeeSeq_XchItem(self, index, value);
err:
	return NULL;
}

INTERN DEFINE_KWLIST(seq_pop_kwlist, { K(index), KEND });
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_pop(DeeObject *self, size_t argc,
        DeeObject *const *argv, DeeObject *kw) {
	dssize_t index = -1;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_pop_kwlist, "|" UNPdSIZ ":pop", &index))
		goto err;
	return DeeSeq_PopItem(self, index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_popfront(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *args[1], *result;
	if (DeeArg_Unpack(argc, argv, ":popfront"))
		goto err;
	args[0] = DeeInt_Zero;
	result  = DeeObject_CallAttr(self, (DeeObject *)&str_pop, 1, args);
	if unlikely(!result && DeeError_Catch(&DeeError_IndexError))
		err_empty_sequence(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_popback(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *args[1], *result;
	if (DeeArg_Unpack(argc, argv, ":popback"))
		goto err;
	args[0] = DeeInt_MinusOne;
	result  = DeeObject_CallAttr(self, (DeeObject *)&str_pop, 1, args);
	if unlikely(!result && DeeError_Catch(&DeeError_IndexError))
		err_empty_sequence(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_pushfront(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *args[2];
	if (DeeArg_Unpack(argc, argv, "o:pushfront", &args[1]))
		goto err;
	args[0] = DeeInt_Zero;
	return DeeObject_CallAttr(self, (DeeObject *)&str_insert, 2, args);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_append(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *obj;
	if (DeeArg_Unpack(argc, argv, "o:append", &obj))
		goto err;
	if (DeeSeq_Append(self, obj))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_extend(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *values;
	if (DeeArg_Unpack(argc, argv, "o:extend", &values))
		goto err;
	if (DeeSeq_Extend(self, values))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_pushback(DeeObject *self, size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttr(self, (DeeObject *)&str_append, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_remove(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *elem, *key;
	int result;
	size_t start, end;
	if (get_sequence_find_args_kw(STR_remove, argc, argv,
	                              kw, &elem, &key, &start, &end))
		goto err;
	result = DeeSeq_Remove(self, start, end, elem, key);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_rremove(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *elem, *key;
	int result;
	size_t start, end;
	if (get_sequence_find_args_kw(STR_rremove, argc, argv,
	                              kw, &elem, &key, &start, &end))
		goto err;
	result = DeeSeq_RRemove(self, start, end, elem, key);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_removeall(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *elem, *key;
	size_t result;
	size_t start, end;
	if (get_sequence_find_args_kw(STR_removeall, argc, argv,
	                              kw, &elem, &key, &start, &end))
		goto err;
	result = DeeSeq_RemoveAll(self, start, end, elem, key);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN DEFINE_KWLIST(seq_removeif_kwlist, { K(should), K(start), K(end), KEND });
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_removeif(DeeObject *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	DeeObject *should;
	size_t result, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_removeif_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":removeif", &should, &start, &end))
		goto err;
	result = DeeSeq_RemoveIf(self, start, end, should);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_clear(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	if (DeeObject_SetRange(self, Dee_None, Dee_None, Dee_None))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_fill(DeeObject *self, size_t argc,
         DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1, result;
	DeeObject *filler = Dee_None;
	PRIVATE DEFINE_KWLIST(kwlist, { K(start), K(end), K(filler), KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist,
	                    "|" UNPdSIZ UNPdSIZ "o:fill",
	                    &start, &end, &filler))
		goto err;
	result = DeeSeq_Fill(self, start, end, filler);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN DEFINE_KWLIST(seq_resize_kwlist, { K(newsize), K(filler), KEND });
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_resize(DeeObject *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	size_t newsize, oldsize;
	DeeObject *filler = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_resize_kwlist,
	                    UNPuSIZ "|o:resize", &newsize, &filler))
		goto err;
	if (!newsize) {
		result = DeeObject_CallAttr(self, (DeeObject *)&str_clear, 0, NULL);
	} else {
		oldsize = DeeObject_Size(self);
		if unlikely(oldsize == (size_t)-1)
			goto err;
		if (newsize < oldsize) {
			result = DeeObject_CallAttrf(self, (DeeObject *)&str_erase,
			                             PCKuSIZ "o", newsize, DeeInt_MinusOne);
		} else if (newsize > oldsize) {
			DREF DeeObject *seq_extension;
			seq_extension = DeeSeq_RepeatItem(filler, newsize - oldsize);
			if unlikely(!seq_extension)
				goto err;
			result = DeeObject_CallAttr(self, (DeeObject *)&str_extend, 1, &seq_extension);
			Dee_Decref(seq_extension);
		} else {
			goto do_return_none;
		}
	}
	if unlikely(result != Dee_None) {
		if unlikely(!result)
			goto err;
		Dee_Decref(result);
do_return_none:
		result = Dee_None;
		Dee_Incref(Dee_None);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_reverse(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":reverse"))
		goto err;
	if (DeeSeq_Reverse(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_sort(DeeObject *self, size_t argc,
         DeeObject *const *argv, DeeObject *kw) {
	DeeObject *key = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_sort_kwlist, "|o:sort", &key))
		goto err;
	if (DeeNone_Check(key))
		key = NULL;
	if (DeeSeq_Sort(self, key))
		goto err;
	return_none;
err:
	return NULL;
}


DOC_DEF(seq_byhash_doc,
        "(template:?O)->?DSequence\n"
        "#ptemplate{The object who's hash should be used to search for collisions}"
        "Find all objects apart of @this sequence who's hash matches that of @template\n"
        "Note that when hashing ?Dint objects, integers who's value lies within the range "
        /**/ "of valid hash values get hashed to their original value, meaning that the following "
        /**/ "two uses of this function are identical (because ${x.operator hash() == x} when $x "
        /**/ "is an integer that contains a valid hash value):\n"
        "${"
        /**/ "local a = seq.byhash(\"foo\");\n"
        /**/ "local b = seq.byhash(\"foo\".operator hash());\n"
        "}\n"
        "The intended use-case is to query contained elements of mappings/sets by-hash, "
        /**/ "rather than by-key, thus allowing user-code more control in regards to is-contained/"
        /**/ "lookup-element, specifically in scenarios where objects are used as keys that are "
        /**/ "expensive to construct, such that "
        /**/ "${return myDict[ConstructNewCopyOfKey(values...)]} "
        /**/ "can be written more efficiently as "
        /**/ "${for (local e: myDict.byhash(hashOfKeyFromValues(values...))) if (e.equals(values...)) return e;}");
INTERN DEFINE_KWLIST(seq_byhash_kwlist, { K(template), KEND });
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_byhash(DeeObject *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	DeeObject *template_;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_byhash_kwlist, "o:byhash", &template_))
		goto err;
	return DeeSeq_HashFilter(self, DeeObject_Hash(template_));
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_bfind(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *elem, *key;
	size_t result, start, end;
	if (get_sequence_find_args_kw("bfind", argc, argv, kw, &elem, &key, &start, &end))
		goto err;
	if (!key) {
		result = DeeSeq_BFind(self, start, end, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_BFind(self, start, end, elem, key);
		Dee_Decref(elem);
	}
	if ((dssize_t)result < 0) {
		if unlikely(result == (size_t)-2)
			goto err;
		if unlikely(result == (size_t)-1)
			return_none;
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_bcontains(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *elem, *key;
	size_t result, start, end;
	if (get_sequence_find_args_kw("bcontains", argc, argv, kw, &elem, &key, &start, &end))
		goto err;
	if (!key) {
		result = DeeSeq_BFind(self, start, end, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_BFind(self, start, end, elem, key);
		Dee_Decref(elem);
	}
	if ((dssize_t)result < 0) {
		if unlikely(result == (size_t)-2)
			goto err;
		if unlikely(result == (size_t)-1)
			return_false;
	}
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_bindex(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *elem, *key;
	size_t result, start, end;
	if (get_sequence_find_args_kw("bindex", argc, argv, kw, &elem, &key, &start, &end))
		goto err;
	if (!key) {
		result = DeeSeq_BFind(self, start, end, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_BFind(self, start, end, elem, key);
		Dee_Decref(elem);
	}
	if unlikely((dssize_t)result < 0) {
		if unlikely(result == (size_t)-2)
			goto err;
		if unlikely(result == (size_t)-1)
			goto err_not_found;
	}
	return DeeInt_NewSize(result);
err_not_found:
	err_item_not_found(self, elem);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_bposition(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *elem, *key;
	size_t result, start, end;
	if (get_sequence_find_args_kw("bposition", argc, argv, kw, &elem, &key, &start, &end))
		goto err;
	if (!key) {
		result = DeeSeq_BFindPosition(self, start, end, elem, NULL);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_BFindPosition(self, start, end, elem, key);
		Dee_Decref(elem);
	}
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_brange(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *elem, *key;
	size_t result_start, result_end;
	size_t start, end;
	int error;
	if (get_sequence_find_args_kw("brange", argc, argv, kw, &elem, &key, &start, &end))
		goto err;
	if (!key) {
		error = DeeSeq_BFindRange(self, start, end, elem, NULL, &result_start, &result_end);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		error = DeeSeq_BFindRange(self, start, end, elem, key, &result_start, &result_end);
		Dee_Decref(elem);
	}
	if unlikely(error)
		goto err;
	return DeeTuple_Newf(PCKuSIZ
	                     PCKuSIZ,
	                     result_start,
	                     result_end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_blocate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeObject *elem, *key, *defl;
	size_t start, end;
	if (get_sequence_find_defl_args_kw("blocate", argc, argv, kw,
	                                   &elem, &key, &start, &end, &defl))
		goto err;
	if (key == NULL) {
		result = DeeSeq_BLocate(self, start, end, elem, NULL, defl);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		result = DeeSeq_BLocate(self, start, end, elem, key, defl);
		Dee_Decref(elem);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_blocateall(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *elem, *key;
	size_t result_start, result_end;
	size_t start, end;
	int error;
	if (get_sequence_find_args_kw("blocateall", argc, argv, kw, &elem, &key, &start, &end))
		goto err;
	if (!key) {
		error = DeeSeq_BFindRange(self, start, end, elem, NULL, &result_start, &result_end);
	} else {
		elem = DeeObject_Call(key, 1, &elem);
		if unlikely(!elem)
			goto err;
		error = DeeSeq_BFindRange(self, start, end, elem, key, &result_start, &result_end);
		Dee_Decref(elem);
	}
	if unlikely(error)
		goto err;
	return DeeSeq_GetRange(self, result_start, result_end);
err:
	return NULL;
}

INTERN DEFINE_KWLIST(seq_binsert_kwlist, { K(elem), K(key), KEND });
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_binsert(DeeObject *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *index, *result;
	DeeObject *args[2]; /* elem, key */
	args[1] = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_binsert_kwlist, "o|o:binsert", &args[0], &args[1]))
		goto err;
	index = DeeObject_CallAttrString(self, "bposition", 2, args);
	if unlikely(!index)
		goto err;
	args[1] = args[0]; /* elem */
	args[0] = index;   /* index */
	result = DeeObject_CallAttr(self, (DeeObject *)&str_insert, 2, args);
	Dee_Decref(args[0]);
	return result;
err:
	return NULL;
}



INTDEF struct type_method tpconst seq_methods[];
INTERN_TPCONST struct type_method tpconst seq_methods[] = {
	TYPE_METHOD("reduce", &seq_reduce,
	            "(merger:?DCallable)->\n"
	            "(merger:?DCallable,init)->\n"
	            "Combines consecutive elements of @this Sequence by passing them as pairs of 2 to @merger, "
	            /**/ "then re-using its return value in the next invocation, before finally returning its last "
	            /**/ "return value. If the Sequence consists of only 1 element, @merger is never invoked.\n"
	            "If the Sequence is empty, ?N is returned\n"
	            "When given, @init is used as the initial lhs-operand, "
	            /**/ "rather than the first element of the Sequence\n"
	            "${"
	            /**/ "function reduce(merger: Callable, init?: Object): Object | none {\n"
	            /**/ "	for (local x: this) {\n"
	            /**/ "		if (init !is bound) {\n"
	            /**/ "			init = x;\n"
	            /**/ "		} else {\n"
	            /**/ "			init = merger(init, x);\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "	if (init is bound)\n"
	            /**/ "		return init;\n"
	            /**/ "	return none;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("filter", &seq_filter,
	            "(keep:?DCallable)->?DSequence\n"
	            "#pkeep{A key function which is called for each element of @this Sequence"
	            /**/ "Returns a sub-Sequence of all elements for which ${keep(elem)} evaluates to ?t}"
	            "Semantically, this is identical to ${(for (local x: this) if (keep(x)) x)}\n"
	            "${"
	            /**/ "function filter(keep): Sequence {\n"
	            /**/ "	for (local x: this)\n"
	            /**/ "		if (keep(x))\n"
	            /**/ "			yield x;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("sum", &seq_sum,
	            "->\nReturns the sum of all elements, or ?N if the Sequence is empty\n"
	            "This, alongside :string.join is the preferred way of merging lists of strings "
	            /**/ "into a single string\n"
	            "${"
	            /**/ "function sum() {\n"
	            /**/ "	local result;\n"
	            /**/ "	for (local x: this) {\n"
	            /**/ "		if (result is bound) {\n"
	            /**/ "			result = result + x;\n"
	            /**/ "		} else {\n"
	            /**/ "			result = x;\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "	return result is bound ? result : none;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("any", &seq_any,
	            "->?Dbool\n"
	            "Returns ?t if any element of @this Sequence evaluates to ?t\n"
	            "If @this Sequence is empty, ?f is returned\n"
	            "This function has the same effect as ${this || ...}\n"
	            "${"
	            /**/ "function any(): bool {\n"
	            /**/ "	for (local x: this)\n"
	            /**/ "		if (x)\n"
	            /**/ "			return true;\n"
	            /**/ "	return false;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("all", &seq_all,
	            "->?Dbool\n"
	            "Returns ?t if all elements of @this Sequence evaluate to ?t\n"
	            "If @this Sequence is empty, ?t is returned\n"
	            "This function has the same effect as ${this && ...}\n"
	            "${"
	            /**/ "function all(): bool {\n"
	            /**/ "	for (local x: this)\n"
	            /**/ "		if (!x)\n"
	            /**/ "			return false;\n"
	            /**/ "	return true;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("parity", &seq_parity,
	            "->?Dbool\n"
	            "Returns ?t or ?f indicative of the parity of Sequence elements that are ?t\n"
	            "If @this Sequence is empty, ?f is returned\n"
	            "Parity here refers to ${##this.filter(x -\\> !!x) % 2}\n"
	            "${"
	            /**/ "function parity(): bool {\n"
	            /**/ "	local result = false;\n"
	            /**/ "	for (local x: this)\n"
	            /**/ "		if (x)\n"
	            /**/ "			result = !result;\n"
	            /**/ "	return result;\n"
	            /**/ "}"
	            "}"),
	TYPE_KWMETHOD("min", &seq_min,
	              "(key:?DCallable=!N)->\n"
	              "#pkey{A key function for transforming Sequence elements}"
	              "Returns the smallest element of @this Sequence\n"
	              "If @this Sequence is empty, ?N is returned\n"
	              "When no @key is given, this function has the same effect as ${this < ...}\n"
	              "${"
	              /**/ "function min(key: Callable = none): Object {\n"
	              /**/ "	local result;\n"
	              /**/ "	local key_result;\n"
	              /**/ "	for (local x: this) {\n"
	              /**/ "		if (result !is bound) {\n"
	              /**/ "			result = x;\n"
	              /**/ "		} else if (key !is none) {\n"
	              /**/ "			if (key_result !is bound)\n"
	              /**/ "				key_result = key(result);\n"
	              /**/ "			local key_x = key(x);\n"
	              /**/ "			if (!(key_result < key_x)) {\n"
	              /**/ "				key_result = key_x;\n"
	              /**/ "				result = x;\n"
	              /**/ "			}\n"
	              /**/ "		} else if (!(result < x)) {\n"
	              /**/ "			result = x;\n"
	              /**/ "		}\n"
	              /**/ "	}\n"
	              /**/ "	if (result !is bound)\n"
	              /**/ "		result = none;\n"
	              /**/ "	return result;\n"
	              /**/ "}"
	              "}"),
	TYPE_KWMETHOD("max", &seq_max,
	              "(key:?DCallable=!N)->\n"
	              "#pkey{A key function for transforming Sequence elements}"
	              "Returns the greatest element of @this Sequence\n"
	              "If @this Sequence is empty, ?N is returned\n"
	              "This function has the same effect as ${this > ...}\n"
	              "${"
	              /**/ "function max(key: Callable = none): Object {\n"
	              /**/ "	local result;\n"
	              /**/ "	for (local x: this) {\n"
	              /**/ "		if (result !is bound) {\n"
	              /**/ "			result = x;\n"
	              /**/ "		} else if (key !is none) {\n"
	              /**/ "			if (key_result !is bound)\n"
	              /**/ "				key_result = key(result);\n"
	              /**/ "			local key_x = key(x);\n"
	              /**/ "			if (key_result < key_x) {\n"
	              /**/ "				key_result = key_x;\n"
	              /**/ "				result = x;\n"
	              /**/ "			}\n"
	              /**/ "		} else if (result < x) {\n"
	              /**/ "			result = x;\n"
	              /**/ "		}\n"
	              /**/ "	}\n"
	              /**/ "	if (result !is bound)\n"
	              /**/ "		result = none;\n"
	              /**/ "	return result;\n"
	              /**/ "}"
	              "}"),
	TYPE_METHOD("count", &seq_count,
	            "(elem,key:?DCallable=!N)->?Dint\n"
	            "#pelem{The element to search for}"
	            "#pkey{A key function for transforming Sequence elements}"
	            "Returns the number of instances of a given object @elem in @this Sequence\n"
	            "${"
	            /**/ "function count(elem: Object, key: Callable = none): int {\n"
	            /**/ "	local result: int = 0;\n"
	            /**/ "	if (key !is none)\n"
	            /**/ "		elem = key(elem);\n"
	            /**/ "	for (local x: this) {\n"
	            /**/ "		if (key !is none) {\n"
	            /**/ "			if (key(x) == elem)\n"
	            /**/ "				++result;\n"
	            /**/ "		} else {\n"
	            /**/ "			if (x == elem)\n"
	            /**/ "				++result;\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "	return result;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("locate", &seq_locate,
	            /* TODO:
	             * "(elem,key:?DCallable=!N,defl?)->\n"
	             * "(elem,start:?Dint,key:?DCallable=!N,defl?)->\n"
	             * "(elem,start:?Dint,end:?Dint,key:?DCallable=!N,defl?)->\n" */
	            "(elem,key:?DCallable=!N)->\n"
	            "#pelem{The element to search for}"
	            "#pkey{A key function for transforming Sequence elements}"
	            "#tValueError{The Sequence does not contain an element matching @elem}"
	            "Returns the first item equal to @elem\n"
	            "${"
	            /**/ "function locate(elem: Object, key: Callable = none): Object {\n"
	            /**/ "	import Error from deemon;\n"
	            /**/ "	if (key !is none)\n"
	            /**/ "		elem = key(elem);\n"
	            /**/ "	for (local x: this) {\n"
	            /**/ "		if (key !is none) {\n"
	            /**/ "			if (elem == key(x))\n"
	            /**/ "				return x;\n"
	            /**/ "		} else {\n"
	            /**/ "			if (elem == x)\n"
	            /**/ "				return x;\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "	throw Error.ValueError(\"Item not found...\")\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("rlocate", &seq_rlocate,
	            /* TODO:
	             * "(elem,key:?DCallable=!N,defl?)->\n"
	             * "(elem,start:?Dint,key:?DCallable=!N,defl?)->\n"
	             * "(elem,start:?Dint,end:?Dint,key:?DCallable=!N,defl?)->\n" */
	            "(elem,key:?DCallable=!N)->\n"
	            "#pelem{The element to search for}"
	            "#pkey{A key function for transforming Sequence elements}"
	            "#tValueError{The Sequence does not contain an element matching @elem}"
	            "Returns the last item equal to @elem\n"
	            "${"
	            /**/ "function rlocate(elem, key: Callable) {\n"
	            /**/ "	import Error from deemon;\n"
	            /**/ "	local result;\n"
	            /**/ "	if (key !is none)\n"
	            /**/ "		elem = key(elem);\n"
	            /**/ "	for (local x: this) {\n"
	            /**/ "		if (key !is none) {\n"
	            /**/ "			if (elem == key(x))\n"
	            /**/ "				result = x;\n"
	            /**/ "		} else {\n"
	            /**/ "			if (elem == x)\n"
	            /**/ "				result = x;\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "	if (result is bound)\n"
	            /**/ "		return result;\n"
	            /**/ "	throw Error.ValueError(\"Item not found...\")\n"
	            /**/ "}"
	            "}"),
	/* TODO: findall:
	 * "(elem,key:?DCallable=!N)->?S?Dint\n"
	 * "(elem,start:?Dint,key:?DCallable=!N)->?S?Dint\n"
	 * "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?S?Dint\n" */
	TYPE_METHOD("locateall", &seq_locateall,
	            /* TODO:
	             * "(elem,key:?DCallable=!N)->?S?O\n"
	             * "(elem,start:?Dint,key:?DCallable=!N)->?S?O\n"
	             * "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?S?O\n" */
	            "(elem,key:?DCallable=!N)->?S?O\n"
	            "#pelem{The element to search for}"
	            "#pkey{A key function for transforming Sequence elements}"
	            "Returns a Sequence of items equal to @elem\n"
	            "${"
	            /**/ "function locateall(elem: Object, key: Callable): Sequence {\n"
	            /**/ "	import Error from deemon;\n"
	            /**/ "	if (key !is none)\n"
	            /**/ "		elem = key(elem);\n"
	            /**/ "	for (local x: this) {\n"
	            /**/ "		if (key !is none) {\n"
	            /**/ "			if (elem == key(x))\n"
	            /**/ "				yield x;\n"
	            /**/ "		} else {\n"
	            /**/ "			if (elem == x)\n"
	            /**/ "				yield x;\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("transform", &seq_transform,
	            "(transformation:?DCallable)->?DSequence\n"
	            "#ptransformation{A key function invoked to transform members of @this Sequence}"
	            "Returns a Sequence that is a transformation of @this, with each element passed "
	            "to @transformation for processing before being returned\n"
	            "${"
	            /**/ "function transform(transformation: Callable): Sequence {\n"
	            /**/ "	for (local x: this)\n"
	            /**/ "		yield transformation(x);\n"
	            /**/ "}"
	            "}\n"
	            "Hint: The python equivalent of this function is #A{map|https://docs.python.org/3/library/functions.html##map}"),
	TYPE_METHOD("contains", &seq_contains,
	            "(elem,key:?DCallable=!N)->?Dbool\n"
	            "#pelem{The element to search for}"
	            "#pkey{A key function for transforming Sequence elements}"
	            "Returns ?t if @this Sequence contains an element matching @elem\n"
	            "${"
	            /**/ "function contains(elem: Object, key: Callable): bool {\n"
	            /**/ "	if (key is none)\n"
	            /**/ "		return elem in this;\n"
	            /**/ "	elem = key(elem);\n"
	            /**/ "	for (local x: this) {\n"
	            /**/ "		if (elem == key(x))\n"
	            /**/ "			return true;\n"
	            /**/ "	}\n"
	            /**/ "	return false;\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD("startswith", &seq_startswith,
	            "(elem,key:?DCallable=!N)->?Dbool\n"
	            "#pelem{The element to compare against}"
	            "#pkey{A key function for transforming Sequence elements}"
	            "Returns ?t / ?f indicative of @this Sequence's first element matching :elem\n"
	            "The implementation of this is derived from #first, where the found is then compared "
	            /**/ "against @elem, potentially through use of @{key}: ${key(first) == key(elem)} or ${first == elem}, "
	            /**/ "however instead of throwing a :ValueError when the Sequence is empty, ?f is returned"),
	TYPE_METHOD("endswith", &seq_endswith,
	            "(elem,key:?DCallable=!N)->?Dbool\n"
	            "#pelem{The element to compare against}"
	            "#pkey{A key function for transforming Sequence elements}"
	            "Returns ?t / ?f indicative of @this Sequence's last element matching :elem\n"
	            "The implementation of this is derived from #last, where the found is then compared "
	            /**/ "against @elem, potentially through use of @{key}: ${key(last) == key(elem)} or ${last == elem}, "
	            /**/ "however instead of throwing a :ValueError when the Sequence is empty, ?f is returned"),
	TYPE_KWMETHOD("find", &seq_find,
	              "(elem,key:?DCallable=!N)->?Dint\n"
	              "(elem,start:?Dint,key:?DCallable=!N)->?Dint\n"
	              "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?Dint\n"
	              "#pelem{The element to search for}"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#pstart{The start index for a sub-range to search (clamped by ${##this})}"
	              "#pend{The end index for a sub-range to search (clamped by ${##this})}"
	              "Search for the first element matching @elem and return its index\n"
	              "If no such element exists, return ${-1} instead\n"
	              "Depending on the nearest implemented group of operators, "
	              /**/ "one of the following implementations is chosen\n"
	              "For ${operator []} and ${operator ##}:\n"
	              "${"
	              /**/ "function find(elem: int, start: int, end: int, key: Callable = none): int {\n"
	              /**/ "	import int, Error from deemon;\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	if (key !is none)\n"
	              /**/ "		elem = key(elem);\n"
	              /**/ "	if (start >= end)\n"
	              /**/ "		return -1;\n"
	              /**/ "	local my_size = ##this;\n"
	              /**/ "	if (start >= my_size)\n"
	              /**/ "		return -1;\n"
	              /**/ "	if (end > my_size)\n"
	              /**/ "		end = my_size;\n"
	              /**/ "	for (local index = start; index < end; ++index) {\n"
	              /**/ "		local my_elem;\n"
	              /**/ "		try {\n"
	              /**/ "			my_elem = this[index];\n"
	              /**/ "		} catch (Error.ValueError.IndexError.UnboundItem) {\n"
	              /**/ "			continue;\n"
	              /**/ "		}\n"
	              /**/ "		if (key !is none) {\n"
	              /**/ "			if (elem == key(my_elem))\n"
	              /**/ "				return index;\n"
	              /**/ "		} else {\n"
	              /**/ "			if (elem == my_elem)\n"
	              /**/ "				return index;\n"
	              /**/ "		}\n"
	              /**/ "	}\n"
	              /**/ "	return -1;\n"
	              /**/ "}"
	              "}\n"
	              "For ${operator iter}:\n"
	              "${"
	              /**/ "function find(elem: int, start: int, end: int, key: Callable = none): int {\n"
	              /**/ "	import Signal, int from deemon;\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	if (key !is none)\n"
	              /**/ "		elem = key(elem);\n"
	              /**/ "	if (start >= end)\n"
	              /**/ "		return -1;\n"
	              /**/ "	local it = this.operator iter();\n"
	              /**/ "	local search_size = end - start;\n"
	              /**/ "	while (start) {\n"
	              /**/ "		try {\n"
	              /**/ "			it.operator next();\n"
	              /**/ "		} catch (Signal.StopIteration) {\n"
	              /**/ "			return -1;\n"
	              /**/ "		}\n"
	              /**/ "		--start;\n"
	              /**/ "	}\n"
	              /**/ "	local index = 0;\n"
	              /**/ "	while (search_size) {\n"
	              /**/ "		local my_elem;\n"
	              /**/ "		try {\n"
	              /**/ "			my_elem = it.operator next();\n"
	              /**/ "		} catch (Signal.StopIteration) {\n"
	              /**/ "			return -1;\n"
	              /**/ "		}\n"
	              /**/ "		if (key !is none) {\n"
	              /**/ "			if (elem == key(my_elem))\n"
	              /**/ "				return index;\n"
	              /**/ "		} else {\n"
	              /**/ "			if (elem == my_elem)\n"
	              /**/ "				return index;\n"
	              /**/ "		}\n"
	              /**/ "		--search_size;\n"
	              /**/ "		++index;\n"
	              /**/ "	}\n"
	              /**/ "	return -1;\n"
	              /**/ "}"
	              "}"),
	TYPE_KWMETHOD("rfind", &seq_rfind,
	              "(elem,key:?DCallable=!N)->?Dint\n"
	              "(elem,start:?Dint,key:?DCallable=!N)->?Dint\n"
	              "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?Dint\n"
	              "#pelem{The element to search for}"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#pstart{The start index for a sub-range to search (clamped by ${##this})}"
	              "#pend{The end index for a sub-range to search (clamped by ${##this})}"
	              "Search for the last element matching @elem and return its index\n"
	              "If no such element exists, return ${-1} instead\n"
	              "Depending on the nearest implemented group of operators, "
	              /**/ "one of the following implementations is chosen\n"
	              "For ${operator []} and ${operator ##}:\n"
	              "${"
	              /**/ "function rfind(elem: int, start: int, end: int, key: Callable = none): int {\n"
	              /**/ "	import int from deemon;\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	if (key !is none)\n"
	              /**/ "		elem = key(elem);\n"
	              /**/ "	if (start >= end)\n"
	              /**/ "		return -1;\n"
	              /**/ "	local my_size = ##this;\n"
	              /**/ "	if (start >= my_size)\n"
	              /**/ "		return -1;\n"
	              /**/ "	if (end > my_size)\n"
	              /**/ "		end = my_size;\n"
	              /**/ "	local index = end-1;\n"
	              /**/ "	if (key !is none)\n"
	              /**/ "		elem = key(elem);\n"
	              /**/ "	for (;;) {\n"
	              /**/ "		local my_elem;\n"
	              /**/ "		try {\n"
	              /**/ "			my_elem = this[index];\n"
	              /**/ "		} catch (Error.ValueError.IndexError.UnboundItem) {\n"
	              /**/ "			goto next_item;\n"
	              /**/ "		}\n"
	              /**/ "		if (key !is none) {\n"
	              /**/ "			if (key(my_elem) == elem)\n"
	              /**/ "				return index;\n"
	              /**/ "		} else {\n"
	              /**/ "			if (my_elem == elem)\n"
	              /**/ "				return index;\n"
	              /**/ "		}\n"
	              /**/ "next_item:\n"
	              /**/ "		if (index == start)\n"
	              /**/ "			break;\n"
	              /**/ "		--index;\n"
	              /**/ "	}\n"
	              /**/ "	return -1;\n"
	              /**/ "}"
	              "}\n"
	              "For ${operator iter}:\n"
	              "${"
	              /**/ "function find(elem: int, start: int, end: int, key: Callable = none): int {\n"
	              /**/ "	import Signal, int from deemon;\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	if (start >= end) return -1;\n"
	              /**/ "	local it = this.operator iter();\n"
	              /**/ "	local search_size = end - start;\n"
	              /**/ "	while (start) {\n"
	              /**/ "		try {\n"
	              /**/ "			it.operator next();\n"
	              /**/ "		} catch (Signal.StopIteration) {\n"
	              /**/ "			return -1;\n"
	              /**/ "		}\n"
	              /**/ "		--start;\n"
	              /**/ "	}\n"
	              /**/ "	local index = 0;\n"
	              /**/ "	local result = -1;\n"
	              /**/ "	if (key !is none)\n"
	              /**/ "		elem = key(elem);\n"
	              /**/ "	while (search_size) {\n"
	              /**/ "		local my_elem;\n"
	              /**/ "		try {\n"
	              /**/ "			my_elem = it.operator next();\n"
	              /**/ "		} catch (Signal.StopIteration) {\n"
	              /**/ "			break;\n"
	              /**/ "		}\n"
	              /**/ "		if (key !is none) {\n"
	              /**/ "			if (key(my_elem) == elem)\n"
	              /**/ "				result = index;\n"
	              /**/ "		} else {\n"
	              /**/ "			if (my_elem == elem)\n"
	              /**/ "				result = index;\n"
	              /**/ "		}\n"
	              /**/ "		--search_size;\n"
	              /**/ "		++index;\n"
	              /**/ "	}\n"
	              /**/ "	return result;\n"
	              /**/ "}"
	              "}"),
	TYPE_KWMETHOD(STR_index, &seq_index,
	              "(elem,key:?DCallable=!N)->?Dint\n"
	              "(elem,start:?Dint,key:?DCallable=!N)->?Dint\n"
	              "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?Dint\n"
	              "#pelem{The element to search for}"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#pstart{The start index for a sub-range to search (clamped by ${##this})}"
	              "#pend{The end index for a sub-range to search (clamped by ${##this})}"
	              "#tValueError{The Sequence does not contain an element matching @elem}"
	              "Search for the first element matching @elem and return its index\n"
	              "This function is implemented as:\n"
	              "${"
	              /**/ "function index(elem: Object, start: int, end: int, key: Callable = none): int {\n"
	              /**/ "	import Sequence, Error from deemon;\n"
	              /**/ "	local result = (this as Sequence).find(elem, start, end, key);\n"
	              /**/ "	if (result == -1)\n"
	              /**/ "		throw Error.ValueError(\"...\");\n"
	              /**/ "	return result;\n"
	              /**/ "}"
	              "}"),
	TYPE_KWMETHOD("rindex", &seq_rindex,
	              "(elem,key:?DCallable=!N)->?Dint\n"
	              "(elem,start:?Dint,key:?DCallable=!N)->?Dint\n"
	              "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?Dint\n"
	              "#pelem{The element to search for}"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#pstart{The start index for a sub-range to search (clamped by ${##this})}"
	              "#pend{The end index for a sub-range to search (clamped by ${##this})}"
	              "#tValueError{The Sequence does not contain an element matching @elem}"
	              "Search for the last element matching @elem and return its index\n"
	              "This function is implemented as:\n"
	              "${"
	              /**/ "function index(elem: Object, start: int, end: int, key: Callable = none): int {\n"
	              /**/ "	import Sequence, Error from deemon;\n"
	              /**/ "	local result = (this as Sequence).rfind(elem, start, end, key);\n"
	              /**/ "	if (result == -1)\n"
	              /**/ "		throw Error.ValueError(\"...\");\n"
	              /**/ "	return result;\n"
	              /**/ "}"
	              "}"),
	TYPE_METHOD("reversed",
	            &seq_reversed,
	            "->?DSequence\n"
	            "Return a Sequence that contains the elements of @this Sequence in reverse order\n"
	            "The point at which @this Sequence is enumerated is implementation-defined"),
	TYPE_KWMETHOD("sorted", &seq_sorted,
	              "(key:?DCallable=!N)->?DSequence\n"
	              "Return a Sequence that contains all elements from @this Sequence, "
	              /**/ "but sorted in ascending order, or in accordance to @key\n"
	              "The point at which @this Sequence is enumerated is implementation-defined"),
	TYPE_METHOD("segments",
	            &seq_segments,
	            "(segment_size:?Dint)->?S?DSequence\n"
	            "#tIntegerOverflow{@segment_size is negative, or too large}"
	            "#tValueError{The given @segment_size is zero}"
	            "Return a Sequence of sequences contains all elements from @this Sequence, "
	            /**/ "with the first n sequences all consisting of @segment_size elements, before "
	            /**/ "the last one contains the remainder of up to @segment_size elements"),
	TYPE_METHOD("distribute",
	            &seq_distribute,
	            "(bucket_count:?Dint)->?S?DSequence\n"
	            "#tIntegerOverflow{@segment_size is negative, or too large}"
	            "#tValueError{The given @segment_size is zero}"
	            "Re-distribute the elements of @this Sequence to form @bucket_count similarly-sized "
	            /**/ "buckets of objects, with the last bucket containing the remaining elements, making "
	            /**/ "its length a little bit shorter than the other buckets\n"
	            "This is similar to #segments, however rather than having the caller specify the "
	            /**/ "size of the a bucket, the number of buckets is specified instead."),
	TYPE_METHOD("combinations",
	            &seq_combinations,
	            "(r:?Dint)->?S?DSequence\n"
	            "#tIntegerOverflow{@r is negative, or too large}"
	            "Returns a Sequence of r-long sequences representing all possible (ordered) "
	            /**/ "combinations of elements retrieved from @this\n"
	            "${"
	            /**/ "/* { (\"A\", \"B\"), (\"A\", \"C\"), (\"A\", \"D\"),\n"
	            /**/ " *   (\"B\", \"C\"), (\"B\", \"D\"), (\"C\", \"D\") } */\n"
	            /**/ "print repr \"ABCD\".combinations(2);"
	            "}\n"
	            "Notice that a combination such as $\"BA\" is not produced, as only possible "
	            /**/ "combinations with their original element order still in tact may be returned\n"
	            "When @this Sequence implements ?#{op:getitem} and ?#{op:size}, those will be invoked "
	            /**/ "as items are retrieved by index. Otherwise, all elements from @this Sequence "
	            /**/ "are loaded at once when #combinations is called first.\n"
	            "When @r is greater than ${##this}, an empty Sequence is returned (${{}})\n"
	            "Hint: The python equivalent of this function is "
	            /**/ "#A{itertools.combinations|https://docs.python.org/3/library/itertools.html##itertools.combinations}"),
	TYPE_METHOD("repeatcombinations",
	            &seq_repeatcombinations,
	            "(r:?Dint)->?S?DSequence\n"
	            "#tIntegerOverflow{@r is negative, or too large}"
	            "Same as #combinations, however elements of @this Sequence may be repeated (though element order is still enforced)\n"
	            "${"
	            /**/ "/* { (\"A\", \"A\"), (\"A\", \"B\"), (\"A\", \"C\"),\n"
	            /**/ " *   (\"B\", \"B\"), (\"B\", \"C\"), (\"C\", \"C\") } */\n"
	            /**/ "print repr \"ABC\".repeatcombinations(2);"
	            "}\n"
	            "When @this Sequence implements ?#{op:getitem} and ?#{op:size}, those will be invoked "
	            /**/ "as items are retrieved by index. Otherwise, all elements from @this Sequence "
	            /**/ "are loaded at once when #repeatcombinations is called first.\n"
	            "When @r is $0, a Sequence containing a single, empty Sequence is returned (${{{}}})\n"
	            "When ${##this} is zero, an empty Sequence is returned (${{}})\n"
	            "Hint: The python equivalent of this function is "
	            /**/ "#A{itertools.combinations_with_replacement|https://docs.python.org/3/library/itertools.html##itertools.combinations_with_replacement}"),
	TYPE_METHOD("permutations",
	            &seq_permutations,
	            "(r:?Dint=!N)->?S?DSequence\n"
	            "#tIntegerOverflow{@r is negative, or too large}"
	            "Same as #combinations, however the order of elements must "
	            /**/ "not be enforced, though indices may not be repeated\n"
	            "When @r is ?N, ${##this} is used instead\n"
	            "${"
	            /**/ "/* { (\"A\", \"B\"), (\"A\", \"C\"), (\"B\", \"A\"),\n"
	            /**/ " *   (\"B\", \"C\"), (\"C\", \"A\"), (\"C\", \"B\") } */\n"
	            /**/ "print repr \"ABC\".permutations(2);"
	            "}\n"
	            "When @this Sequence implements ?#{op:getitem} and ?#{op:size}, those will be invoked "
	            /**/ "as items are retrieved by index. Otherwise, all elements from @this Sequence "
	            /**/ "are loaded at once when #repeatcombinations is called first.\n"
	            "When @r is $0, a Sequence containing a single, empty Sequence is returned (${{{}}})\n"
	            "When ${##this} is zero, an empty Sequence is returned (${{}})\n"
	            "Hint: The python equivalent of this function is "
	            /**/ "#A{itertools.permutations|https://docs.python.org/3/library/itertools.html##itertools.permutations}"),

	/* TODO: unique(key:?DCallable=!N)->?DSequence
	 * Returns a generic Sequence proxy that contains all of the elements from @this Sequence,
	 * however will only enumerate the first of n consecutive objects for which ${key(first) == key(nth)}
	 * evaluates to true (essentially removing all duplicate, neighboring items)
	 * When @key is none, the behavior is the same as though @key was `identity from functools'
	 * >> local items = collect_items();
	 * >> local unique_items = items.sorted(Object.id).unique(Object.id);
	 * >> // `unique_items' now contains no object more than once
	 */

	/* TODO: join(items: {Sequence...}): Sequence */
	/* TODO: strip(item: Object, key: Callable = none): Sequence */
	/* TODO: lstrip(item: Object, key: Callable = none): Sequence */
	/* TODO: rstrip(item: Object, key: Callable = none): Sequence */
	/* TODO: split(sep: Object, key: Callable = none): Sequence */

	/* TODO: countseq(seq: Sequence, key: Callable = none): int */
	/* TODO: partition(item: Object, key: Callable = none): (Sequence, (item), Sequence) */
	/* TODO: rpartition(item: Object, key: Callable = none): (Sequence, (item), Sequence) */
	/* TODO: partitionseq(seq: Sequence, key: Callable = none): (Sequence, seq, Sequence) */
	/* TODO: rpartitionseq(seq: Sequence, key: Callable = none): (Sequence, seq, Sequence) */
	/* TODO: startswithseq(seq: Sequence, key: Callable = none): bool */
	/* TODO: endswithseq(seq: Sequence, key: Callable = none): bool */
	/* TODO: findseq(seq: Sequence, key: Callable = none): int */
	/* TODO: rfindseq(seq: Sequence, key: Callable = none): int */
	/* TODO: indexseq(seq: Sequence, key: Callable = none): int */
	/* TODO: rindexseq(seq: Sequence, key: Callable = none): int */
	/* TODO: stripseq(items: Sequence, key: Callable = none): Sequence */
	/* TODO: lstripseq(items: Sequence, key: Callable = none): Sequence */
	/* TODO: rstripseq(items: Sequence, key: Callable = none): Sequence */
	/* TODO: splitseq(seq: Sequence, key: Callable = none): Sequence */


	/* Functions for mutable sequences. */
	TYPE_KWMETHOD(STR_insert, &seq_insert,
	              "(index:?Dint,item)\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "When @index is negative, it will refer to the end of the Sequence\n"
	              "For mutable sequences only: Insert the given @item under @index\n"
	              "When this function isn't defined by a sub-class, the following "
	              /**/ "default-implementation is provided by ?.:\n"
	              "${"
	              /**/ "function insert(index: int, item: Object) {\n"
	              /**/ "	import int, Error, Sequence from deemon;\n"
	              /**/ "	index = index.operator int();\n"
	              /**/ "	for (local tp = type(this); tp !is none && tp !== Sequence; tp = tp.__base__) {\n"
	              /**/ "		if (index < 0) {\n"
	              /**/ "			if (tp.hasprivateattribute(\"append\")) */ {\n"
	              /**/ "				this.append(item);\n"
	              /**/ "				return;\n"
	              /**/ "			}\n"
	              /**/ "			if (tp.hasprivateattribute(\"extend\")) {\n"
	              /**/ "				this.extend(TYPE_METHOD(item });\n"
	              /**/ "				return;\n"
	              /**/ "			}\n"
	              /**/ "		}\n"
	              /**/ "		if (tp.hasprivateattribute(\"insertall\")) {\n"
	              /**/ "			this.insertall(index, TYPE_METHOD(item });\n"
	              /**/ "			return;\n"
	              /**/ "		}\n"
	              /**/ "		if (tp.hasprivateoperator(\"setrange\")) {\n"
	              /**/ "			if (index < 0)\n"
	              /**/ "				index = int.SIZE_MAX;\n"
	              /**/ "			this[index:index] = TYPE_METHOD(item };\n"
	              /**/ "			return;\n"
	              /**/ "		}\n"
	              /**/ "		if (index < 0) {\n"
	              /**/ "			if (tp.hasprivateoperator(\"assign\")) {\n"
	              /**/ "				local addend = TYPE_METHOD(item };\n"
	              /**/ "				this := (this as Sequence) + addend;\n"
	              /**/ "				return;\n"
	              /**/ "			}\n"
	              /**/ "		}\n"
	              /**/ "	}\n"
	              /**/ "	throw Error.ValueError.SequenceError(\"Sequence not resizable\");\n"
	              /**/ "}"
	              "}"),
	TYPE_KWMETHOD(STR_insertall, &seq_insertall,
	              "(index:?Dint,items:?DSequence)\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "For mutable sequences only: Insert all elements from @items at @index\n"
	              "When this function isn't defined by a sub-class, the following "
	              /**/ "default-implementation is provided by ?.:\n"
	              "${"
	              /**/ "function insertall(index: int, items: {Object...}) {\n"
	              /**/ "	import int, Error, Sequence from deemon;\n"
	              /**/ "	index = index.operator int();\n"
	              /**/ "	for (local tp = type(this); tp !is none && tp !== Sequence; tp = tp.__base__) {\n"
	              /**/ "		if (index < 0) {\n"
	              /**/ "			if (tp.hasprivateattribute(\"extend\")) {\n"
	              /**/ "				this.extend(items);\n"
	              /**/ "				return;\n"
	              /**/ "			}\n"
	              /**/ "		}\n"
	              /**/ "		if (tp.hasprivateoperator(\"setrange\")) {\n"
	              /**/ "			if (index < 0) index = int.SIZE_MAX;\n"
	              /**/ "			this[index:index] = items;\n"
	              /**/ "			return;\n"
	              /**/ "		}\n"
	              /**/ "		if (index < 0) {\n"
	              /**/ "			if (tp.hasprivateattribute(\"append\")) {\n"
	              /**/ "				for (local x: items)\n"
	              /**/ "					this.append(x);\n"
	              /**/ "				return;\n"
	              /**/ "			}\n"
	              /**/ "		}\n"
	              /**/ "		if (tp.hasprivateattribute(\"insert\")) {\n"
	              /**/ "			for (local x: items: {Object...}) {\n"
	              /**/ "				this.insert(index, x);\n"
	              /**/ "				if (index >= 0) ++index;\n"
	              /**/ "			}\n"
	              /**/ "			return;\n"
	              /**/ "		}\n"
	              /**/ "		if (index < 0) {\n"
	              /**/ "			if (tp.hasprivateoperator(\"assign\")) {\n"
	              /**/ "				this := (this as Sequence) + items;\n"
	              /**/ "				return;\n"
	              /**/ "			}\n"
	              /**/ "		}\n"
	              /**/ "	}\n"
	              /**/ "	throw Error.ValueError.SequenceError(\"Sequence not resizable\");\n"
	              /**/ "}"
	              "}"),
	TYPE_METHOD(STR_append, &seq_append,
	            "(item)\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "For mutable sequences only: Append the given @item at the end of @this Sequence\n"
	            "When this function isn't defined by a sub-class, the following "
	            /**/ "default-implementation is provided by ?.:\n"
	            "${"
	            /**/ "function append(item: Object) {\n"
	            /**/ "	import int, Error, Sequence from deemon;\n"
	            /**/ "	for (local tp = type(this); tp !is none && tp !== Sequence; tp = tp.__base__) {\n"
	            /**/ "		if (tp.hasprivateattribute(\"extend\")) {\n"
	            /**/ "			this.extend(TYPE_METHOD(item });\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"insert\")) {\n"
	            /**/ "			this.insert(-1, item);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"insertall\")) {\n"
	            /**/ "			this.insertall(-1, TYPE_METHOD(item });\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"setrange\")) {\n"
	            /**/ "			this[int.SIZE_MAX:int.SIZE_MAX] = TYPE_METHOD(item };\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"assign\":=)) {\n"
	            /**/ "			local addend = TYPE_METHOD(item };\n"
	            /**/ "			this := (this as Sequence) + addend;\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "	throw Error.ValueError.SequenceError(\"Sequence not resizable\");\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD(STR_extend, &seq_extend,
	            "(items:?DSequence)\n"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "For mutable sequences only: Append all elements from @items at the end of @this Sequence\n"
	            "When this function isn't defined by a sub-class, the following "
	            /**/ "default-implementation is provided by ?.:\n"
	            "${"
	            /**/ "function extend(items: {Object...}) {\n"
	            /**/ "	import int, Error, Sequence from deemon;\n"
	            /**/ "	for (local tp = type(this); tp !is none && tp !== Sequence; tp = tp.__base__) {\n"
	            /**/ "		if (tp.hasprivateattribute(\"insertall\")) {\n"
	            /**/ "			this.insertall(-1, items);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"setrange\")) {\n"
	            /**/ "			this[int.SIZE_MAX:int.SIZE_MAX] = items;\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"append\")) {\n"
	            /**/ "			for (local x: items)\n"
	            /**/ "				this.append(x);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateattribute(\"insert\")) {\n"
	            /**/ "			for (local x: items)\n"
	            /**/ "				this.insert(-1, x);\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "		if (tp.hasprivateoperator(\"assign\")) {\n"
	            /**/ "			this := (this as Sequence) + items;\n"
	            /**/ "			return;\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "	throw Error.ValueError.SequenceError(\"Sequence not resizable\");\n"
	            /**/ "}"
	            "}"),
	TYPE_KWMETHOD(STR_erase, &seq_erase,
	              "(index:?Dint,count=!1)\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "For mutable sequences only: Erase up to @count elements starting at @index, "
	              /**/ "and return the actual number of erased items (Less than @count may be erased "
	              /**/ "when ${index + count > ##this})\n"
	              "When this function isn't defined by a sub-class, the following "
	              /**/ "default-implementation is provided by ?.:\n"
	              "${"
	              /**/ "function erase(index: int, count: int = 1) {\n"
	              /**/ "	import int, Error from deemon;\n"
	              /**/ "	index = index.operator int();\n"
	              /**/ "	count = count.operator int();\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (index >= mylen)\n"
	              /**/ "		throw Error.ValueError.IndexError(\"...\");\n"
	              /**/ "	if (index + count > mylen)\n"
	              /**/ "		count = mylen - index;\n"
	              /**/ "	for (local tp = type(this); tp !is none && tp !== Sequence; tp = tp.__base__) {\n"
	              /**/ "		if (tp.hasprivateoperator(\"delrange\")) {\n"
	              /**/ "			del this[index:index + count];\n"
	              /**/ "			return count;\n"
	              /**/ "		}\n"
	              /**/ "		if (tp.hasprivateoperator(\"setrange\")) {\n"
	              /**/ "			this[index:index + count] = none;\n"
	              /**/ "			return count;\n"
	              /**/ "		}\n"
	              /**/ "		if (tp.hasprivateoperator(\"delitem\")) {\n"
	              /**/ "			if (count) {\n"
	              /**/ "				local i = index + count;\n"
	              /**/ "				do {\n"
	              /**/ "					--i;\n"
	              /**/ "					del this[index];\n"
	              /**/ "				} while (i > index);\n"
	              /**/ "			}\n"
	              /**/ "			return count;\n"
	              /**/ "		}\n"
	              /**/ "	}\n"
	              /**/ "	throw Error.ValueError.SequenceError(\"Sequence not resizable\");\n"
	              /**/ "}"
	              "}"),
	TYPE_KWMETHOD(STR_xch, &seq_xch,
	              "(index:?Dint,value)->\n"
	              "#tIntegerOverflow{The given @index is negative, or too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "For mutable sequences only: Exchange the @index'th element of @this Sequence "
	              /**/ "with the given @value, returning the old element found under that index\n"
	              /**/ "When this function isn't defined by a sub-class, the following "
	              /**/ "default-implementation is provided by ?.:\n"
	              "${"
	              /**/ "function xch(index: int, value: Object) {\n"
	              /**/ "	import int, Error from deemon;\n"
	              /**/ "	index = index.operator int();\n"
	              /**/ "	for (local tp = type(this); tp !is none && tp !== Sequence; tp = tp.__base__) {\n"
	              /**/ "		if (tp.hasprivateoperator(\"setitem\")) {\n"
	              /**/ "			local result = this[index];\n"
	              /**/ "			this[index] = value;\n"
	              /**/ "			return result;\n"
	              /**/ "		}\n"
	              /**/ "		if (tp.hasprivateoperator(\"setrange\")) {\n"
	              /**/ "			local result = this[index];\n"
	              /**/ "			this[index:index + 1] = value;\n"
	              /**/ "			return result;\n"
	              /**/ "		}\n"
	              /**/ "	}\n"
	              /**/ "	throw Error.ValueError.SequenceError(\"Sequence not mutable\");\n"
	              /**/ "}"
	              "}"),
	TYPE_KWMETHOD(STR_pop, &seq_pop,
	              "(index=!-1)->\n"
	              "#tIntegerOverflow{The given @index is too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "#tSequenceError{@this Sequence cannot be resized}"
	              "For mutable sequences only: Pop the @index'th element of @this Sequence and "
	              /**/ "return it. When @index is lower than $0, add ${##this} prior to index selection\n"
	              "When this function isn't defined by a sub-class, the following "
	              /**/ "default-implementation is provided by ?.:\n"
	              "${"
	              /**/ "function pop(index: int = -1): Object {\n"
	              /**/ "	import int, Error from deemon;\n"
	              /**/ "	index = index.operator int();\n"
	              /**/ "	if (index < 0)\n"
	              /**/ "		index += ##this;\n"
	              /**/ "	for (local tp = type(this); tp !is none && tp !== Sequence; tp = tp.__base__) {\n"
	              /**/ "		if (tp.hasprivateoperator(\"delitem\")) {\n"
	              /**/ "			local result = this[index];\n"
	              /**/ "			del this[index];\n"
	              /**/ "			return result;\n"
	              /**/ "		}\n"
	              /**/ "		if (tp.hasprivateoperator(\"delrange\")) {\n"
	              /**/ "			local result = this[index];\n"
	              /**/ "			del this[index:index + 1];\n"
	              /**/ "			return result;\n"
	              /**/ "		}\n"
	              /**/ "		if (tp.hasprivateoperator(\"setrange\")) {\n"
	              /**/ "			local result = this[index];\n"
	              /**/ "			this[index:index + 1] = none;\n"
	              /**/ "			return result;\n"
	              /**/ "		}\n"
	              /**/ "	}\n"
	              /**/ "	throw Error.ValueError.SequenceError(\"Sequence not mutable\");\n"
	              /**/ "}"
	              "}"),
	TYPE_METHOD(STR_popfront, &seq_popfront,
	            "->\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "For mutable sequences only: Convenience wrapper for #pop\n"
	            "${"
	            /**/ "function popfront(): Object {\n"
	            /**/ "	import Error from deemon;\n"
	            /**/ "	try {\n"
	            /**/ "		return this.pop(0);\n"
	            /**/ "	} catch (Error.ValueError.IndexError) {\n"
	            /**/ "		throw Error.ValueError(\"Empty Sequence...\");\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD(STR_popback, &seq_popback,
	            "->\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "For mutable sequences only: Convenience wrapper for #pop\n"
	            "${"
	            /**/ "function popfront(): Object {\n"
	            /**/ "	import Error from deemon;\n"
	            /**/ "	try {\n"
	            /**/ "		return this.pop(-1);\n"
	            /**/ "	} catch (Error.ValueError.IndexError) {\n"
	            /**/ "		throw Error.ValueError(\"Empty Sequence...\");\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD(STR_pushfront, &seq_pushfront,
	            "(item)\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "For mutable sequences only: Convenience wrapper for #insert at position $0\n"
	            "${"
	            /**/ "function pushfront(item: Object) {\n"
	            /**/ "	return this.insert(0, item);\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD(STR_pushback, &seq_pushback,
	            "(item)\n"
	            "#tIndexError{The given @index is out of bounds}"
	            "#tSequenceError{@this Sequence cannot be resized}"
	            "For mutable sequences only: Convenience wrapper for #append\n"
	            "${"
	            /**/ "function pushback(item: Object) {\n"
	            /**/ "	return this.append(item);\n"
	            /**/ "}"
	            "}"),
	TYPE_KWMETHOD(STR_remove, &seq_remove,
	              "(elem,key:?DCallable=!N)->?Dbool\n"
	              "(elem,start:?Dint,key:?DCallable=!N)->?Dbool\n"
	              "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?Dbool\n"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#tSequenceError{@this Sequence is immutable}"
	              "For mutable sequences only: Find the first instance of @elem and remove it, "
	              /**/ "returning ?t if an element got removed, or ?f if @elem could not be found\n"
	              "Depending on the nearest implemented group of operators, "
	              /**/ "one of the following implementations is chosen\n"
	              "For ${operator del[]}:\n"
	              "${"
	              /**/ "function remove(elem: int, start: int, end: int, key: Callable): bool {\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (end > mylen)\n"
	              /**/ "		end = mylen;\n"
	              /**/ "	if (start < end) {\n"
	              /**/ "		if (key !is none)\n"
	              /**/ "			elem = key(elem);\n"
	              /**/ "		for (local i = start; i < end; ++i) {\n"
	              /**/ "			local item = this[i];\n"
	              /**/ "			if (key !is none) {\n"
	              /**/ "				if (!(elem == key(item)))\n"
	              /**/ "					continue;\n"
	              /**/ "			} else {\n"
	              /**/ "				if (!(elem == item))\n"
	              /**/ "					continue;\n"
	              /**/ "			}\n"
	              /**/ "			del this[i];\n"
	              /**/ "			return true;\n"
	              /**/ "		}\n"
	              /**/ "	}\n"
	              /**/ "	return false;\n"
	              /**/ "}"
	              "}\n"
	              "For ${erase(index, count)}:\n"
	              "${"
	              /**/ "function remove(elem: int, start: int, end: int, key: Callable): bool {\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (end > mylen)\n"
	              /**/ "		end = mylen;\n"
	              /**/ "	if (start < end) {\n"
	              /**/ "		if (key !is none)\n"
	              /**/ "			elem = key(elem);\n"
	              /**/ "		for (local i = start; i < end; ++i) {\n"
	              /**/ "			local item = this[i];\n"
	              /**/ "			if (key !is none) {\n"
	              /**/ "				if (!(elem == key(item)))\n"
	              /**/ "					continue;\n"
	              /**/ "			} else {\n"
	              /**/ "				if (!(elem == item))\n"
	              /**/ "					continue;\n"
	              /**/ "			}\n"
	              /**/ "			this.erase(i, 1);\n"
	              /**/ "			return true;\n"
	              /**/ "		}\n"
	              /**/ "	}\n"
	              /**/ "	return false;\n"
	              /**/ "}"
	              "}\n"
	              "For ${operator del[:]}:\n"
	              "${"
	              /**/ "function remove(elem: Object, start: int, end: int, key: Callable): bool {\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (end > mylen)\n"
	              /**/ "		end = mylen;\n"
	              /**/ "	if (start < end) {\n"
	              /**/ "		if (key !is none)\n"
	              /**/ "			elem = key(elem);\n"
	              /**/ "		for (local i = start; i < end; ++i) {\n"
	              /**/ "			local item = this[i];\n"
	              /**/ "			if (key !is none) {\n"
	              /**/ "				if (!(elem == key(item)))\n"
	              /**/ "					continue;\n"
	              /**/ "			} else {\n"
	              /**/ "				if (!(elem == item))\n"
	              /**/ "					continue;\n"
	              /**/ "			}\n"
	              /**/ "			del this[i:i+1];\n"
	              /**/ "			return true;\n"
	              /**/ "		}\n"
	              /**/ "	}\n"
	              /**/ "	return false;\n"
	              /**/ "}"
	              "}"),
	TYPE_KWMETHOD(STR_rremove, &seq_rremove,
	              "(elem,key:?DCallable=!N)->?Dbool\n"
	              "(elem,start:?Dint,key:?DCallable=!N)->?Dbool\n"
	              "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?Dbool\n"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#tSequenceError{@this Sequence is immutable}"
	              "For mutable sequences only: Find the last instance of @elem and remove it, "
	              /**/ "returning ?t if an element got removed, or ?f if @elem could not be found\n"
	              "Depending on the nearest implemented group of operators, "
	              /**/ "one of the following implementations is chosen\n"
	              "For ${operator del[]}:\n"
	              "${"
	              /**/ "function rremove(elem: int, start: int, end: int, key: Callable): bool {\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (end > mylen)\n"
	              /**/ "		end = mylen;\n"
	              /**/ "	if (end > start) {\n"
	              /**/ "		local i = end;\n"
	              /**/ "		if (key !is none)\n"
	              /**/ "			elem = key(elem);\n"
	              /**/ "		do {\n"
	              /**/ "			--i;\n"
	              /**/ "			local item = this[i];\n"
	              /**/ "			if (key !is none) {\n"
	              /**/ "				if (!(elem == key(item)))\n"
	              /**/ "					continue;\n"
	              /**/ "			} else {\n"
	              /**/ "				if (!(elem == item))\n"
	              /**/ "					continue;\n"
	              /**/ "			}\n"
	              /**/ "			del this[i];\n"
	              /**/ "			return true;\n"
	              /**/ "		} while (i > start);\n"
	              /**/ "	}\n"
	              /**/ "	return false;\n"
	              /**/ "}"
	              "}\n"
	              "For ${erase(index, count)}:\n"
	              "${"
	              /**/ "function rremove(elem: int, start: int, end: int, key: Callable): bool {\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (end > mylen)\n"
	              /**/ "		end = mylen;\n"
	              /**/ "	if (end > start) {\n"
	              /**/ "		local i = end;\n"
	              /**/ "		if (key !is none)\n"
	              /**/ "			elem = key(elem);\n"
	              /**/ "		do {\n"
	              /**/ "			--i;\n"
	              /**/ "			local item = this[i];\n"
	              /**/ "			if (key !is none) {\n"
	              /**/ "				if (!(elem == key(item)))\n"
	              /**/ "					continue;\n"
	              /**/ "			} else {\n"
	              /**/ "				if (!(elem == item))\n"
	              /**/ "					continue;\n"
	              /**/ "			}\n"
	              /**/ "			this.erase(i, 1);\n"
	              /**/ "			return true;\n"
	              /**/ "		} while (i > start);\n"
	              /**/ "	}\n"
	              /**/ "	return false;\n"
	              /**/ "}"
	              "}\n"
	              "For ${operator del[:]}:\n"
	              "${"
	              /**/ "function rremove(elem: int, start: int, end: int, key: Callable): bool {\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (end > mylen)\n"
	              /**/ "		end = mylen;\n"
	              /**/ "	if (end > start) {\n"
	              /**/ "		local i = end;\n"
	              /**/ "		if (key !is none)\n"
	              /**/ "			elem = key(elem);\n"
	              /**/ "		do {\n"
	              /**/ "			--i;\n"
	              /**/ "			local item = this[i];\n"
	              /**/ "			if (key !is none) {\n"
	              /**/ "				if (!(elem == key(item)))\n"
	              /**/ "					continue;\n"
	              /**/ "			} else {\n"
	              /**/ "				if (!(elem == item))\n"
	              /**/ "					continue;\n"
	              /**/ "			}\n"
	              /**/ "			del this[i:i+1];\n"
	              /**/ "			return true;\n"
	              /**/ "		} while (i > start);\n"
	              /**/ "	}\n"
	              /**/ "	return false;\n"
	              /**/ "}"
	              "}"),
	TYPE_KWMETHOD(STR_removeall, &seq_removeall,
	              "(elem,key:?DCallable=!N)->?Dint\n"
	              "(elem,start:?Dint,key:?DCallable=!N)->?Dint\n"
	              "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?Dint\n"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#tSequenceError{@this Sequence is immutable}"
	              "For mutable sequences only: Find all instance of @elem and remove "
	              /**/ "them, returning the number of instances found (and consequently removed)\n"
	              "Depending on the nearest implemented group of operators, "
	              /**/ "one of the following implementations is chosen\n"
	              "For ${removeif(should, start, end)}:\n"
	              "${"
	              /**/ "function removeall(elem: Object, start: int, end: int, key: Callable): int {\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	if (start >= end) return 0;\n"
	              /**/ "	if (key is none)\n"
	              /**/ "		return this.removeif(x -\\> elem == x, start, end).operator int();\n"
	              /**/ "	elem = key(elem);\n"
	              /**/ "	return this.removeif(x -\\> elem == key(x), start, end).operator int();\n"
	              /**/ "}"
	              "}\n"
	              "For ${rremove(elem, start, end, key)}:\n"
	              "${"
	              /**/ "function removeall(elem: Object, start: int, end: int, key: Callable): int {\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	local count = 0;\n"
	              /**/ "	while (end > start) {\n"
	              /**/ "		if (key is none) {\n"
	              /**/ "			if (!this.rremove(elem, start, end))\n"
	              /**/ "				break;\n"
	              /**/ "		} else {\n"
	              /**/ "			if (!this.rremove(elem, start, end, key))\n"
	              /**/ "				break;\n"
	              /**/ "		}\n"
	              /**/ "		++count;\n"
	              /**/ "		--end;\n"
	              /**/ "	}\n"
	              /**/ "	return count;\n"
	              /**/ "}"
	              "}\n"
	              "For ${remove(elem, start, end, key)}:\n"
	              "${"
	              /**/ "function removeall(elem: Object, start: int, end: int, key: Callable): int {\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	local count = 0;\n"
	              /**/ "	while (end > start) {\n"
	              /**/ "		if (key is none) {\n"
	              /**/ "			if (!this.remove(elem, start, end))\n"
	              /**/ "				break;\n"
	              /**/ "		} else {\n"
	              /**/ "			if (!this.remove(elem, start, end, key))\n"
	              /**/ "				break;\n"
	              /**/ "		}\n"
	              /**/ "		++count;\n"
	              /**/ "		--end;\n"
	              /**/ "	}\n"
	              /**/ "	return count;\n"
	              /**/ "}"
	              "}\n"
	              "For ${operator del[]}:\n"
	              "${"
	              /**/ "function removeall(elem: Object, start: int, end: int, key: Callable): int {\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	local count = 0;\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (end > mylen)\n"
	              /**/ "		end = mylen;\n"
	              /**/ "	if (end > start) {\n"
	              /**/ "		local i = end;\n"
	              /**/ "		if (key !is none)\n"
	              /**/ "			elem = key(elem);\n"
	              /**/ "		do {\n"
	              /**/ "			--i;\n"
	              /**/ "			local item = this[i];\n"
	              /**/ "			if (key !is none) {\n"
	              /**/ "				if (!(elem == key(item)))\n"
	              /**/ "					continue;\n"
	              /**/ "			} else {\n"
	              /**/ "				if (!(elem == item))\n"
	              /**/ "					continue;\n"
	              /**/ "			}\n"
	              /**/ "			del this[i];\n"
	              /**/ "			++count;\n"
	              /**/ "		} while (i > start);\n"
	              /**/ "	}\n"
	              /**/ "	return count;\n"
	              /**/ "}"
	              "}\n"
	              "For ${erase(index, count)}:\n"
	              "${"
	              /**/ "function removeall(elem: Object, start: int, end: int, key: Callable): int {\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	local count = 0;\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (end > mylen)\n"
	              /**/ "		end = mylen;\n"
	              /**/ "	if (end > start) {\n"
	              /**/ "		local i = end;\n"
	              /**/ "		if (key !is none)\n"
	              /**/ "			elem = key(elem);\n"
	              /**/ "		do {\n"
	              /**/ "			--i;\n"
	              /**/ "			local item = this[i];\n"
	              /**/ "			if (key !is none) {\n"
	              /**/ "				if (!(elem == key(item)))\n"
	              /**/ "					continue;\n"
	              /**/ "			} else {\n"
	              /**/ "				if (!(elem == item))\n"
	              /**/ "					continue;\n"
	              /**/ "			}\n"
	              /**/ "			this.erase(i, 1);\n"
	              /**/ "			++count;\n"
	              /**/ "		} while (i > start);\n"
	              /**/ "	}\n"
	              /**/ "	return count;\n"
	              /**/ "}"
	              "}\n"
	              "For ${operator del[:]}:\n"
	              "${"
	              /**/ "function removeall(elem: Object, start: int, end: int, key: Callable): int {\n"
	              /**/ "	start = start.operator int();\n"
	              /**/ "	end = end.operator int();\n"
	              /**/ "	local count = 0;\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (end > mylen)\n"
	              /**/ "		end = mylen;\n"
	              /**/ "	if (end > start) {\n"
	              /**/ "		local i = end;\n"
	              /**/ "		if (key !is none)\n"
	              /**/ "			elem = key(elem);\n"
	              /**/ "		do {\n"
	              /**/ "			--i;\n"
	              /**/ "			local item = this[i];\n"
	              /**/ "			if (key !is none) {\n"
	              /**/ "				if (!(elem == key(item)))\n"
	              /**/ "					continue;\n"
	              /**/ "			} else {\n"
	              /**/ "				if (!(elem == item))\n"
	              /**/ "					continue;\n"
	              /**/ "			}\n"
	              /**/ "			del this[i:i+1];\n"
	              /**/ "			++count;\n"
	              /**/ "		} while (i > start);\n"
	              /**/ "	}\n"
	              /**/ "	return count;\n"
	              /**/ "}"
	              "}"),
	TYPE_KWMETHOD(STR_removeif, &seq_removeif,
	              "(should:?DCallable,start=!0,end=!-1)->?Dint\n"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#tSequenceError{@this Sequence is immutable}"
	              "For mutable sequences only: Remove all elements within the given sub-range, "
	              /**/ "for which ${should(elem)} evaluates to ?t, and return the number "
	              /**/ "of elements found (and consequently removed)\n"
	              "Depending on the nearest implemented group of operators, "
	              /**/ "one of the following implementations is chosen\n"
	              "For ${removeall(should, start, end)}:\n"
	              "${"
	              /**/ "function removeif(should: Callable, start: int, end: int): int {\n"
	              /**/ "	if (start >= end)\n"
	              /**/ "		return 0;\n"
	              /**/ "	return this.removeall(true, start, end, should).operator int();\n"
	              /**/ "}"
	              "}\n"
	              "For ${rremove(elem, start, end, key)}:\n"
	              "${"
	              /**/ "function removeif(should: Callable, start: int, end: int): int {\n"
	              /**/ "	local count = 0;\n"
	              /**/ "	while (end > start) {\n"
	              /**/ "		if (!this.rremove(true, start, end, should))\n"
	              /**/ "			break;\n"
	              /**/ "		++count;\n"
	              /**/ "		--end;\n"
	              /**/ "	}\n"
	              /**/ "	return count;\n"
	              /**/ "}"
	              "}\n"
	              "For ${remove(elem, start, end, key)}:\n"
	              "${"
	              /**/ "function removeif(should: Callable, start: int, end: int): int {\n"
	              /**/ "	local count = 0;\n"
	              /**/ "	while (end > start) {\n"
	              /**/ "		if (!this.remove(true, start, end, should))\n"
	              /**/ "			break;\n"
	              /**/ "		++count;\n"
	              /**/ "		--end;\n"
	              /**/ "	}\n"
	              /**/ "	return count;\n"
	              /**/ "}"
	              "}\n"
	              "For ${operator del[]}:\n"
	              "${"
	              /**/ "function removeif(should: Callable, start: int, end: int): int {\n"
	              /**/ "	local count = 0;\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (end > mylen)\n"
	              /**/ "		end = mylen;\n"
	              /**/ "	if (end > start) {\n"
	              /**/ "		local i = end;\n"
	              /**/ "		do {\n"
	              /**/ "			--i;\n"
	              /**/ "			local item = this[i];\n"
	              /**/ "			if (!should(item))\n"
	              /**/ "				continue;\n"
	              /**/ "			del this[i];\n"
	              /**/ "			++count;\n"
	              /**/ "		} while (i > start);\n"
	              /**/ "	}\n"
	              /**/ "	return count;\n"
	              /**/ "}"
	              "}\n"
	              "For ${erase(index, count)}:\n"
	              "${"
	              /**/ "function removeif(should: Callable, start: int, end: int): int {\n"
	              /**/ "	local count = 0;\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (end > mylen)\n"
	              /**/ "		end = mylen;\n"
	              /**/ "	if (end > start) {\n"
	              /**/ "		local i = end;\n"
	              /**/ "		do {\n"
	              /**/ "			--i;\n"
	              /**/ "			local item = this[i];\n"
	              /**/ "			if (!should(item))\n"
	              /**/ "				continue;\n"
	              /**/ "			this.erase(i, 1);\n"
	              /**/ "			++count;\n"
	              /**/ "		} while (i > start);\n"
	              /**/ "	}\n"
	              /**/ "	return count;\n"
	              /**/ "}"
	              "}\n"
	              "For ${operator del[:]}:\n"
	              "${"
	              /**/ "function removeif(should: Callable, start: int, end: int): int {\n"
	              /**/ "	local count = 0;\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (end > mylen)\n"
	              /**/ "		end = mylen;\n"
	              /**/ "	if (end > start) {\n"
	              /**/ "		local i = end;\n"
	              /**/ "		do {\n"
	              /**/ "			--i;\n"
	              /**/ "			local item = this[i];\n"
	              /**/ "			if (!should(item))\n"
	              /**/ "				continue;\n"
	              /**/ "			del this[i:i+1];\n"
	              /**/ "			++count;\n"
	              /**/ "		} while (i > start);\n"
	              /**/ "	}\n"
	              /**/ "	return count;\n"
	              /**/ "}"
	              "}"),
	TYPE_METHOD(STR_clear, &seq_clear,
	            "()\n"
	            "#tSequenceError{@this Sequence is immutable}"
	            "For mutable sequences only: Clear all elements from the Sequence\n"
	            "When not implemented by a sub-class, ?. implements "
	            "this function as follows (s.a. ?#{op:setrange}):\n"
	            "${"
	            /**/ "function clear() {\n"
	            /**/ "	this[:] = none;\n"
	            /**/ "}"
	            "}"),
	TYPE_KWMETHOD(STR_resize, &seq_resize,
	              "(int newsize,filler=!N)\n"
	              "#tSequenceError{@this Sequence isn't resizable}"
	              "Resize @this Sequence to have a new length of @newsize "
	              /**/ "items, using @filler to initialize newly added entries\n"
	              "When not implemented by a sub-class, ?. "
	              /**/ "implements this function as follows:\n"
	              "${"
	              /**/ "function resize(newsize: int, filler: Object = none) {\n"
	              /**/ "	import int, Sequence from deemon;\n"
	              /**/ "	newsize = newsize.operator int();\n"
	              /**/ "	if (!newsize) {\n"
	              /**/ "		this.clear();\n"
	              /**/ "	} else {\n"
	              /**/ "		local oldsize = (##this).operator int();\n"
	              /**/ "		if (newsize < oldsize) {\n"
	              /**/ "			this.erase(newsize, -1);\n"
	              /**/ "		} else if (newsize > oldsize) {\n"
	              /**/ "			this.extend(Sequence.repeat(filler, newsize - oldsize));\n"
	              /**/ "		}\n"
	              /**/ "	}\n"
	              /**/ "}"
	              "}"),
	TYPE_KWMETHOD("fill", &seq_fill,
	              "(start=!0,end=!-1,filler=!N)->?Dint\n"
	              "#tSequenceError{@this Sequence is immutable}"
	              "For mutable sequences only: Assign @filler to all elements within "
	              /**/ "the given sub-range, and return the number of written indices\n"
	              "Depending on the nearest implemented group of operators, "
	              /**/ "one of the following implementations is chosen\n"
	              "For ${operator setrange}:\n"
	              "${"
	              /**/ "function fill(start: int = 0, end: int = -1, filler: Object = none) {\n"
	              /**/ "	import Sequence from deemon;\n"
	              /**/ "	if (start >= end)\n"
	              /**/ "		return 0;\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (start >= mylen)\n"
	              /**/ "		return 0;\n"
	              /**/ "	if (end > mylen)\n"
	              /**/ "		end = mylen;\n"
	              /**/ "	local result = end - start;\n"
	              /**/ "	this[start:end] = Sequence.repeat(filler, result);\n"
	              /**/ "	return result;\n"
	              /**/ "}"
	              "}\n"
	              "For ${operator setitem}:\n"
	              "${"
	              /**/ "function fill(start: int = 0, end: int = -1, filler: Object = none) {\n"
	              /**/ "	import Sequence from deemon;\n"
	              /**/ "	if (start >= end)\n"
	              /**/ "		return 0;\n"
	              /**/ "	local mylen = ##this;\n"
	              /**/ "	if (start >= mylen)\n"
	              /**/ "		return 0;\n"
	              /**/ "	if (end > mylen)\n"
	              /**/ "		end = mylen;\n"
	              /**/ "	for (local i = start; i < end; ++i)\n"
	              /**/ "		this[i] = filler;\n"
	              /**/ "	return end - start;\n"
	              /**/ "}"
	              "}"),
	TYPE_METHOD("reverse", &seq_reverse,
	            "()\n"
	            "#tSequenceError{@this Sequence is immutable}"
	            "For mutable sequences only: Reverse the order of all elements\n"
	            "When not implemented by a sub-class, ?. implements "
	            /**/ "this function as follows (s.a. ?#{op:assign}):\n"
	            "${"
	            /**/ "function reverse() {\n"
	            /**/ "	this := (this as Sequence from deemon).reversed();\n"
	            /**/ "}"
	            "}"),
	TYPE_KWMETHOD("sort", &seq_sort,
	              "(key:?DCallable=!N)\n"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#tSequenceError{@this Sequence is immutable}"
	              "For mutable sequences only: Sort the elements of @this Sequence\n"
	              "When not implemented by a sub-class, ?. implements "
	              /**/ "this function as follows (s.a. ?#{op:assign}):\n"
	              "${"
	              /**/ "function sort(key: Callable = none) {\n"
	              /**/ "	this := (this as Sequence from deemon).sorted(key);\n"
	              /**/ "}"
	              "}"),
	TYPE_KWMETHOD("byhash", &seq_byhash, DOC_GET(seq_byhash_doc)),

	/* Binary search API */
	TYPE_KWMETHOD("bfind", &seq_bfind,
	              "(elem,key:?DCallable=!N)->?X2?Dint?N\n"
	              "(elem,start:?Dint,key:?DCallable=!N)->?X2?Dint?N\n"
	              "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?X2?Dint?N\n"
	              "#pelem{The element to search for}"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#pstart{The start index for a sub-range to search (clamped by ${##this})}"
	              "#pend{The end index for a sub-range to search (clamped by ${##this})}"
	              "Do a binary search (requiring @this to be sorted via @key) for @elem\n"
	              "In case multiple elements match @elem, the returned index will be "
	              /**/ "that for one of them, though it is undefined which one specifically.\n"
	              "When no elements of @this match, ?N is returned."),
	TYPE_KWMETHOD("bcontains", &seq_bcontains,
	              "(elem,key:?DCallable=!N)->?Dbool\n"
	              "(elem,start:?Dint,key:?DCallable=!N)->?Dbool\n"
	              "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?Dbool\n"
	              "#pelem{The element to search for}"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#pstart{The start index for a sub-range to search (clamped by ${##this})}"
	              "#pend{The end index for a sub-range to search (clamped by ${##this})}"
	              "Wrapper around ?#bfind that simply returns ${this.bfind(...) !is none}"),
	TYPE_KWMETHOD("bindex", &seq_bindex,
	              "(elem,key:?DCallable=!N)->?Dint\n"
	              "(elem,start:?Dint,key:?DCallable=!N)->?Dint\n"
	              "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?Dint\n"
	              "#pelem{The element to search for}"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#pstart{The start index for a sub-range to search (clamped by ${##this})}"
	              "#pend{The end index for a sub-range to search (clamped by ${##this})}"
	              "#tValueError{The Sequence does not contain an element matching @elem}"
	              "Same as ?#bfind, but throw an :ValueError instead of returning ?N."),
	TYPE_KWMETHOD("bposition", &seq_bposition,
	              "(elem,key:?DCallable=!N)->?Dint\n"
	              "(elem,start:?Dint,key:?DCallable=!N)->?Dint\n"
	              "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?Dint\n"
	              "#pelem{The element to search for}"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#pstart{The start index for a sub-range to search (clamped by ${##this})}"
	              "#pend{The end index for a sub-range to search (clamped by ${##this})}"
	              "Same as ?#bfind, but return (an) index where @elem should be inserted, rather "
	              /**/ "than ?N when @this doesn't contain any matching object"),
	TYPE_KWMETHOD("brange", &seq_brange,
	              "(elem,key:?DCallable=!N)->?T2?Dint?Dint\n"
	              "(elem,start:?Dint,key:?DCallable=!N)->?T2?Dint?Dint\n"
	              "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?T2?Dint?Dint\n"
	              "#pelem{The element to search for}"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#pstart{The start index for a sub-range to search (clamped by ${##this})}"
	              "#pend{The end index for a sub-range to search (clamped by ${##this})}"
	              "Similar to ?#bfind, but return a tuple ${[begin,end)} of integers representing "
	              /**/ "the lower and upper bound of indices for elements from @this matching @elem.\n"
	              "NOTE: The returned tuple is allowed to be an ASP, meaning that its elements may "
	              /**/ "be calculated lazily, and are prone to change as the result of @this changing."),
	TYPE_KWMETHOD("blocate", &seq_blocate,
	              "(elem,key:?DCallable=!N,defl?)->\n"
	              "(elem,start:?Dint,key:?DCallable=!N,defl?)->\n"
	              "(elem,start:?Dint,end:?Dint,key:?DCallable=!N,defl?)->\n"
	              "#pelem{The element to search for}"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#tValueError{The Sequence does not contain an element matching @elem}"
	              "Same as ?#bfind, but return the matching element, rather than its index"),
	TYPE_KWMETHOD("blocateall", &seq_blocateall,
	              "(elem,key:?DCallable=!N)->?S?O\n"
	              "(elem,start:?Dint,key:?DCallable=!N)->?S?O\n"
	              "(elem,start:?Dint,end:?Dint,key:?DCallable=!N)->?S?O\n"
	              "#pelem{The element to search for}"
	              "#pkey{A key function for transforming Sequence elements}"
	              "#pstart{The start index for a sub-range to search (clamped by ${##this})}"
	              "#pend{The end index for a sub-range to search (clamped by ${##this})}"
	              "Return the sub-range from @this of elements matching @elem, as returned by ?#brange\n"
	              "${"
	              /**/ "function blocateall(args..., **kwds) {\n"
	              /**/ "	import Sequence from deemon;\n"
	              /**/ "	local begin, end = this.brange(args..., **kwds)...;\n"
	              /**/ "	return (this as Sequence)[begin:end];\n"
	              /**/ "}"
	              "}\n"
	              "Here is a really neat usage-example for this function: find all strings within "
	              /**/ "a sorted sequence of strings that start with a given prefix-string:\n"
	              "${"
	              /**/ "local lines: {string...} = ...; /* Must be sorted! */\n"
	              /**/ "local prefix: string     = ...;\n"
	              /**/ "/* The process of looking up relevant lines here is O(log(##lines))! */\n"
	              /**/ "for (local l: lines.blocateall(prefix, s -#> s.substr(0, ##prefix)))\n"
	              /**/ "	print l;\n"
	              "}"),
	TYPE_KWMETHOD("binsert", &seq_binsert,
	              "(elem,key:?DCallable=!N)\n"
	              "Helper wrapper for ?#insert and ?#bposition that automatically determines "
	              /**/ "the index where a given @elem should be inserted to ensure that @this sequence "
	              /**/ "remains sorted according to @key. Note that this function makes virtual calls as "
	              /**/ "seen in the following template, meaning it usually doesn't need to be overwritten "
	              /**/ "by sub-classes.\n"
	              "${"
	              /**/ "function binsert(elem: Object, key: Callable = none) {\n"
	              /**/ "	local index = this.bposition(elem, key);\n"
	              /**/ "	return this.insert(index, elem);\n"
	              /**/ "}"
	              "}"),


/* Old function names/deprecated functions. */
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_METHOD("front", &seq_front_deprecated,
	            "->\n"
	            "Deprecated alias for ?#first"),
	TYPE_METHOD("back", &seq_back_deprecated,
	            "->\n"
	            "Deprecated alias for ?#last"),
	TYPE_METHOD("empty", &seq_empty_deprecated,
	            "->?Dbool\n"
	            "Deprecated alias for ?#isempty"),
	TYPE_METHOD("non_empty", &seq_non_empty_deprecated,
	            "->?Dbool\n"
	            "Deprecated alias for ?#isnonempty"),
	TYPE_METHOD("at", &seq_at_deprecated,
	            "(index:?Dint)->\n"
	            "Deprecated alias for ${this[index]}"),
	TYPE_METHOD(STR_get, &seq_at_deprecated,
	            "(index:?Dint)->\n"
	            "Deprecated alias for ${this[index]}\n"
	            "In older versions of deemon, this function (as well as ${operator []}) "
	            /**/ "would modulate the given @index by the length of the Sequence. Starting "
	            /**/ "with deemon 200, this behavior no longer exists, and neither is it still "
	            /**/ "supported"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_METHOD_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_length_get(DeeObject *__restrict self) {
	return DeeObject_SizeObject(self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seq_del_first(DeeObject *__restrict self) {
	int result;
	result = DeeObject_DelItemIndex(self, 0);
	if (result < 0 && DeeError_Catch(&DeeError_IndexError))
		err_empty_sequence(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seq_set_first(DeeObject *__restrict self, DeeObject *__restrict value) {
	int result;
	result = DeeObject_SetItemIndex(self, 0, value);
	if (result < 0 && DeeError_Catch(&DeeError_IndexError))
		err_empty_sequence(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seq_del_last(DeeObject *__restrict self) {
	size_t mylen = DeeObject_Size(self);
	if unlikely(mylen == (size_t)-1)
		goto err;
	if unlikely(!mylen)
		goto err_empty;
	return DeeObject_DelItemIndex(self, mylen - 1);
err_empty:
	err_empty_sequence(self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seq_set_last(DeeObject *__restrict self, DeeObject *__restrict value) {
	size_t mylen = DeeObject_Size(self);
	if unlikely(mylen == (size_t)-1)
		goto err;
	if unlikely(!mylen)
		goto err_empty;
	return DeeObject_SetItemIndex(self, mylen - 1, value);
err_empty:
	err_empty_sequence(self);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_get_ismutable(DeeObject *__restrict self) {
	int result = DeeSeq_IsMutable(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_get_isresizable(DeeObject *__restrict self) {
	int result = DeeSeq_IsResizable(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_get_isfrozen(DeeObject *__restrict self) {
	return_bool(Dee_TYPE(self) == &DeeSeq_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_get_isempty(DeeObject *__restrict self) {
	int result = DeeSeq_NonEmpty(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(!result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_get_isnonempty(DeeObject *__restrict self) {
	int result = DeeSeq_NonEmpty(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}




PRIVATE struct type_getset tpconst seq_getsets[] = {
	TYPE_GETTER("length", &seq_length_get, "->?Dint\nAlias for ${##this}"),
	TYPE_GETSET(STR_first, &DeeSeq_Front, &seq_del_first, &seq_set_first,
	            "->\n"
	            "Access the first item of the Sequence\n"
	            "Depending on the nearest implemented group of operators, "
	            /**/ "one of the following implementations is chosen\n"
	            "For ${operator []}:\n"
	            "${"
	            /**/ "property first: Object = {\n"
	            /**/ "	get(): Object {\n"
	            /**/ "		import Error from deemon;\n"
	            /**/ "		try {\n"
	            /**/ "			return this[0];\n"
	            /**/ "		} catch (Error.ValueError.IndexError) {\n"
	            /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "	del() {\n"
	            /**/ "		try {\n"
	            /**/ "			del this[0]; // If `operator delitem' doesn't exist, throw NotImplemented\n"
	            /**/ "		} catch (Error.ValueError.IndexError) {\n"
	            /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "	set(value: Object) {\n"
	            /**/ "		try {\n"
	            /**/ "			this[0] = value; /* If `operator setitem' doesn't exist, throw NotImplemented */\n"
	            /**/ "		} catch (Error.ValueError.IndexError) {\n"
	            /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}\n"
	            "For ${operator iter}:\n"
	            "${"
	            /**/ "property first: Object = {\n"
	            /**/ "	get(): Object {\n"
	            /**/ "		import Error, Signal from deemon;\n"
	            /**/ "		local it = this.operator iter();\n"
	            /**/ "		try {\n"
	            /**/ "			return it.operator next();\n"
	            /**/ "		} catch (Signal.StopIteration) {\n"
	            /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	            /**/ "		}\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETSET(STR_last, &DeeSeq_Back, &seq_del_last, &seq_set_last,
	            "->\n"
	            "Access the last item of the Sequence\n"
	            "Depending on the nearest implemented group of operators, "
	            /**/ "one of the following implementations is chosen\n"
	            "For ${operator []} and ${operator ##}:\n"
	            "${"
	            /**/ "property last: Object = {\n"
	            /**/ "	get(): Object {\n"
	            /**/ "		import Error from deemon;\n"
	            /**/ "		local mylen = ##this;\n"
	            /**/ "		if (!mylen)\n"
	            /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	            /**/ "		return this[mylen - 1];\n"
	            /**/ "	}\n"
	            /**/ "	del() {\n"
	            /**/ "		import Error from deemon;\n"
	            /**/ "		local mylen = ##this;\n"
	            /**/ "		if (!mylen)\n"
	            /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	            /**/ "		del this[mylen - 1];\n"
	            /**/ "	}\n"
	            /**/ "	set(value: Object) {\n"
	            /**/ "		import Error from deemon;\n"
	            /**/ "		local mylen = ##this;\n"
	            /**/ "		if (!mylen)\n"
	            /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	            /**/ "		this[mylen - 1] = value;\n"
	            /**/ "	}\n"
	            /**/ "}}\n"
	            /**/ "For ${operator iter}:\n"
	            /**/ "${"
	            /**/ "property last: Object = {\n"
	            /**/ "	get(): Object {\n"
	            /**/ "		import Error, Signal from deemon;\n"
	            /**/ "		local it = this.operator iter();\n"
	            /**/ "		local result;\n"
	            /**/ "		try {\n"
	            /**/ "			for (;;)\n"
	            /**/ "				result = it.operator next();\n"
	            /**/ "		} catch (Signal.StopIteration) {\n"
	            /**/ "		}\n"
	            /**/ "		if (result !is bound)\n"
	            /**/ "			throw Error.ValueError(\"Empty Sequence...\");\n"
	            /**/ "		return result;\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("ismutable", &seq_get_ismutable,
	            "->?Dbool\n"
	            "Try to determine if @this Sequence is mutable by looking at operators and "
	            /**/ "member functions implemented by sub-classes. Note however that this property "
	            /**/ "indicating ?t does not necessarily guaranty that elements of the Sequence "
	            /**/ "can actually be manipulated, only that a sub-class provides special behavior "
	            /**/ "for at least one of the following: ?#{op:setitem}, ?#{op:delitem}, ?#{op:setrange}, "
	            /**/ "?#{op:delrange}, ?#append, ?#extend, ?#insert, ?#insertall, ?#erase, ?#remove, "
	            /**/ "?#rremove, ?#removeall, ?#removeif, ?#pop, ?#xch, ?#resize, ?#clear, ?#pushfront, "
	            /**/ "?#pushback, ?#popfront or ?#popback"),
	TYPE_GETTER("isresizable", &seq_get_isresizable,
	            "->?Dbool\n"
	            "Similar to ?#ismutable, but tries to determine if @this Sequence is resizable by "
	            /**/ "looking at member functions implemented by sub-classes. Note however that this "
	            /**/ "property indicating ?t does not necessarily guaranty that elements of the "
	            /**/ "Sequence can actually be manipulated, only that a sub-class provides special "
	            /**/ "behavior for at least one of the following: ?#append, ?#extend, ?#insert, ?#insertall, "
	            /**/ "?#erase, ?#pop, ?#resize, ?#pushfront, ?#pushback, ?#popfront or ?#popback"),
	TYPE_GETTER("each", &DeeSeq_Each,
	            "->?S?O\n"
	            "Returns a special proxy object that mirrors any operation performed on "
	            /**/ "it onto each element of @this Sequence, evaluating to another proxy object "
	            /**/ "that allows the same, but also allows being used as a regular Sequence:\n"
	            "${"
	            /**/ "local locks = { get_lock(\"a\"), get_lock(\"b\") };\n"
	            /**/ "with (locks.each) { ... }\n"
	            /**/ "local strings = { \"foo\", \"bar\", \"foobar\" };\n"
	            /**/ "for (local x: strings.each.upper())\n"
	            /**/ "	print x; /* \"FOO\", \"BAR\", \"FOOBAR\" */\n"
	            /**/ "local lists = { [10, 20, 30], [1, 2, 3], [19, 41, 57] };\n"
	            /**/ "del lists.each[0];\n"
	            /**/ "print repr lists; /* { [20, 30], [2, 3], [41, 57] } */"
	            "}\n"
	            "WARNING: When invoking member functions, be sure to expand the generated "
	            /**/ "Sequence to ensure that the operator actually gets applied. "
	            /**/ "The only exception to this rule are operators that don't have an "
	            /**/ "actual return value and thus cannot be used in expand expressions:\n"
	            "${"
	            /**/ "local lists = { [10, 20, 30], [1, 2, 3], [19, 41, 57] };\n"
	            /**/ "lists.each.insert(0, 9)...; /* Expand the wrapped Sequence to ensure invocation */\n"
	            /**/ "lists.each[0] = 8;          /* No need for expand in this case */\n"
	            /**/ "del lists.each[0];          /* No need for expand in this case */"
	            "}"),
	TYPE_GETTER("ids", &SeqIds_New,
	            "->?S?Dint\n"
	            "Returns a special proxy object for accessing the ids of Sequence elements\n"
	            "This is equivalent to ${this.transform(x -\\> Object.id(x))}"),
	TYPE_GETTER("types", &SeqTypes_New,
	            "->?S?DType\n"
	            "Returns a special proxy object for accessing the types of Sequence elements\n"
	            "This is equivalent to ${this.transform(x -\\> type(x))}"),
	TYPE_GETTER("classes", &SeqClasses_New,
	            "->?S?DType\n"
	            "Returns a special proxy object for accessing the classes of Sequence elements\n"
	            "This is equivalent to ${this.transform(x -\\> x.class)}"),
	TYPE_GETTER("isempty", &seq_get_isempty,
	            "->?Dbool\n"
	            "Returns ?t if @this Sequence is empty\n"
	            "Implemented as (s.a. ?#{op:bool}):\n"
	            "${"
	            /**/ "property isempty: bool = {\n"
	            /**/ "	get(): bool {\n"
	            /**/ "		return !this.operator bool();\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	TYPE_GETTER("isnonempty", &seq_get_isnonempty,
	            "->?Dbool\n"
	            "Returns ?t if @this Sequence is non-empty\n"
	            "Implemented as (s.a. ?#{op:bool}):\n"
	            "${"
	            /**/ "property isnonempty: bool = {\n"
	            /**/ "	get(): bool {\n"
	            /**/ "		return this.operator bool();\n"
	            /**/ "	}\n"
	            /**/ "}"
	            "}"),
	/* TODO: itemtype->?DType
	 *       Check if the type of @this overrides the ?#ItemType class attribute.
	 *       If so, return its value; else, return the common base-class of all
	 *       items in @this ?.. When @this is empty, ?O is returned. */

	/* TODO: ItemType->?DType   (class property)
	 *       When this type of ?. only allows items of a certain ?DType,
	 *       this class attribute is overwritten with that ?DType. Else,
	 *       it simply evaluates to ?O */

	TYPE_GETTER("isfrozen", &seq_get_isfrozen,
	            "->?Dbool\n"
	            "Evaluates to true if the ?Aid?Os of elements of "
	            /**/ "@this Sequence can never change though use of any non-implementation-"
	            /**/ "specific functions/attributes (i.e. anything that doesn't match #C{__*__}).\n"
	            "This differs from the inverse of ?#ismutable, as in the case "
	            /**/ "of a proxy Sequence, this property depends on the underlying "
	            /**/ "Sequence, and the kind of transformation applied to it, rather "
	            /**/ "than what is exposed by the proxy itself"),
	TYPE_GETTER(STR_frozen, &DeeTuple_FromSequence,
	            "->?#Frozen\n"
	            "Returns a copy of @this Sequence, with all of its current elements, as well as "
	            /**/ "their current order frozen in place, constructing a snapshot of the Sequence's "
	            /**/ "current elements. - The actual type of Sequence returned is implementation- "
	            /**/ "and type- specific, but a guaranty is made that nothing no non-implementation-"
	            /**/ "specific functions/attributes (i.e. anything that doesn't match #C{__*__}) will "
	            /**/ "be able to change the elements of the returned sequence.\n"
	            "By default, this attribute simply casts @this Sequence into a ?DTuple, but sequence "
	            /**/ "types that are known to already be immutable can override this attribute such "
	            /**/ "that they simple re-return themselves.\n"
	            "Note that this attributes does NOT perform a deep copy, and does NOT protect from "
	            "potential changes made to the state of the elements of @this sequence. It ONLY makes "
	            "a snapshot of the sequence itself. If you want to construct a frozen deep copy, you "
	            "should do the following instead (assuming that elements of the sequence support being "
	            "deep-copied):\n"
	            "${"
	            /**/ "local s = getSeq();\n"
	            /**/ "local s = (for (local x: s) deepcopy x).frozen;"
	            "}"),
	TYPE_GETSET_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_class_range(DeeObject *UNUSED(self),
                size_t argc, DeeObject *const *argv) {
	/*  Offering the same functionality as the legacy `util::range()',
	 * `Sequence.range()' is the new builtin way of getting this
	 *  behavior from a core function (since `Sequence' is a
	 *  builtin type like `List', `Tuple', etc.). */
	DeeObject *start, *end = NULL, *step = NULL, *result;
	if (DeeArg_Unpack(argc, argv, "o|oo:range", &start, &end, &step))
		goto err;
	if (end)
		return DeeRange_New(start, end, step);
	/* Use a default-constructed instance of `type(start)' for the real start. */
	end = DeeObject_NewDefault(Dee_TYPE(start));
	if unlikely(!end)
		goto err;
	result = DeeRange_New(end, start, step);
	Dee_Decref(end);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_class_repeat(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
	DeeObject *obj;
	size_t count;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":repeat", &obj, &count))
		goto err;
	return DeeSeq_RepeatItem(obj, count);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
seq_class_repeatseq(DeeObject *__restrict UNUSED(self),
                    size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	size_t count;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":repeatseq", &seq, &count))
		goto err;
	return DeeSeq_Repeat(seq, count);
err:
	return NULL;
}

INTDEF DeeTypeObject SeqConcat_Type;

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seq_class_concat(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	if (!argc)
		return_empty_seq;
	if (argc == 1)
		return_reference_(argv[0]);
	result = DeeTuple_NewVector(argc, argv);
	if likely(result) {
		ASSERT(result->ob_type == &DeeTuple_Type);
		Dee_DecrefNokill(&DeeTuple_Type);
		Dee_Incref(&SeqConcat_Type);
		result->ob_type = &SeqConcat_Type;
	}
	return result;
}

PRIVATE struct type_method tpconst seq_class_methods[] = {
	TYPE_METHOD("range", &seq_class_range,
	            "(end:?Dint)->?S?Dint\n"
	            "(end)->?DSequence\n"
	            "(start:?Dint,end:?Dint)->?S?Dint\n"
	            "(start,end)->?DSequence\n"
	            "(start:?Dint,end:?Dint,step:?Dint)->?S?Dint\n"
	            "(start,end,step)->?DSequence\n"
	            "Create a new Sequence object for enumeration of indices. "
	            /**/ "This function is a simple wrapper for the same "
	            /**/ "functionality available through the following usercode:\n"
	            "${"
	            /**/ "local x = [:end];\n"
	            /**/ "local x = [start:end];\n"
	            /**/ "local x = [start:end, step];"
	            "}"),
	TYPE_METHOD("repeat", &seq_class_repeat,
	            "(obj,count:?Dint)->?DSequence\n"
	            "#tIntegerOverflow{@count is negative}"
	            "Create a proxy-Sequence that yields @obj a total of @count times\n"
	            "The main purpose of this function is to construct large sequences "
	            /**/ "to be used as initializers for mutable sequences such as ?DList"),
	TYPE_METHOD("repeatseq", &seq_class_repeatseq,
	            "(seq:?DSequence,count:?Dint)->?DSequence\n"
	            "#tIntegerOverflow{@count is negative}"
	            "Repeat all the elements from @seq a total of @count times\n"
	            "This is the same as ${(seq as Sequence from deemon) * count}"),
	TYPE_METHOD("concat", &seq_class_concat,
	            "(seqs!:?DSequence)->?DSequence\n"
	            "Returns a proxy-Sequence describing the concatenation of all of the given sequences\n"
	            "When only 1 Sequence is given, that Sequence is forwarded directly.\n"
	            "When no sequences are given, an empty Sequence is returned\n"
	            "Hint: The python equivalent of this function is "
	            /**/ "#A{itertools.chain|https://docs.python.org/3/library/itertools.html##itertools.chain}"),
	TYPE_METHOD_END
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seq_assign(DeeObject *self, DeeObject *other) {
	int result;
	result = DeeObject_SetRange(self, Dee_None, Dee_None, other);
	if unlikely(result < 0 && DeeError_Catch(&DeeError_NotImplemented))
		err_immutable_sequence(self);
	return result;
}


INTDEF int DCALL none_i1(void *UNUSED(a));
INTDEF int DCALL none_i2(void *UNUSED(a), void *UNUSED(b));


/* `Sequence from deemon' */
PUBLIC DeeTypeObject DeeSeq_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Sequence),
	/* .tp_doc      = */ DOC("A recommended abstract base class for any Sequence "
	                         /**/ "type that wishes to implement a Sequence protocol\n"
	                         "When derived from, ?. implements numerous Sequence-related "
	                         /**/ "member functions, such as ?#find or ?#reduce, as well as "
	                         /**/ "operators such as ?#{op:add} and ?#{op:eq}\n"
	                         "An object derived from this class should implement at at least one of:\n"
	                         "#L-{"
	                         /**/ "${operator iter}, aka. ${operator for} (?#{op:iter})|"
	                         /**/ "${operator [] (index: int): Object} (?#{op:getitem}) and ${operator ## (): int} (?#{op:size})"
	                         "}\n"
	                         "The abstract declaration syntax for a generic Sequence is ${{object...}}\n"
	                         "Also note that a Sequence is considered mutable when implementing "
	                         /**/ "${operator []=} (?#{op:setitem}) and/or ${operator [:]=} (?#{op:setrange}), at which "
	                         /**/ "point an extended set of Sequence functionality becomes available for use.\n"
	                         "Functions that fall under this category are documented as only applying to mutable sequences.\n"
	                         "\n"

	                         "()\n"
	                         "A no-op default constructor that is implicitly called by sub-classes\n"
	                         "When invoked manually, an empty, general-purpose Sequence is returned\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t/?f indicative of @this Sequence being non-empty\n"
	                         "Same as the ?#isnonempty attribute\n"
	                         "Depending on the nearest implemented group of operators, "
	                         /**/ "one of the following implementations is chosen\n"
	                         "For ${operator ##}:\n"
	                         "${"
	                         /**/ "operator bool() {\n"
	                         /**/ "	return !!##this;\n"
	                         /**/ "}"
	                         "}\n"
	                         "For ${operator iter}:\n"
	                         "${"
	                         /**/ "operator bool() {\n"
	                         /**/ "	import Signal from deemon;\n"
	                         /**/ "	local it = this.operator iter();\n"
	                         /**/ "	try {\n"
	                         /**/ "		it.operator next();\n"
	                         /**/ "	} catch (Signal.StopIteration) {\n"
	                         /**/ "		return false;\n"
	                         /**/ "	}\n"
	                         /**/ "	return true;\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "repr->\n"
	                         "Returns the representation of all Sequence elements, using "
	                         /**/ "abstract Sequence syntax\n"
	                         "e.g.: ${{ 10, 20, \"foo\" }}\n"
	                         "${"
	                         /**/ "operator repr() {\n"
	                         /**/ "	import File from deemon;\n"
	                         /**/ "	local fp = File.Writer();\n"
	                         /**/ "	fp << \"{ \";\n"
	                         /**/ "	local is_first = true;\n"
	                         /**/ "	for (local elem: this) {\n"
	                         /**/ "		if (!is_first)\n"
	                         /**/ "			fp << \", \"\n"
	                         /**/ "		is_first = false;\n"
	                         /**/ "		fp << repr elem;\n"
	                         /**/ "	}\n"
	                         /**/ "	return fp.string;\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "+(other:?DSequence)->\n"
	                         "Returns a proxy Sequence for accessing elements from @this "
	                         /**/ "Sequence, and those from @other in a seamless stream of items\n"
	                         "This operator is implemented similar to the following, however the actual "
	                         /**/ "return type may be a proxy Sequence that further optimizes the iteration "
	                         /**/ "strategy used, based on which operators have been implemented by sub-classes, as well "
	                         /**/ "as how the sub-range is accessed (i.e. ${##(this + other)} will invoke ${(##this).operator int() + (#other).operator int()}).\n"
	                         "${"
	                         /**/ "operator + (other) {\n"
	                         /**/ "	yield this...;\n"
	                         /**/ "	yield other...;\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "*(count:?Dint)->\n"
	                         "#tIntegerOverflow{@count is negative, or larger than :int.SIZE_MAX}"
	                         "Returns a proxy Sequence for accessing the elements of @this "
	                         /**/ "Sequence, consecutively repeated a total of @count times\n"
	                         "The implementation is allowed to assume that the number and value of "
	                         /**/ "items of @this Sequence don't change between multiple iterations\n"
	                         "This operator is implemented similar to the following, however the actual "
	                         /**/ "return type may be a proxy Sequence that further optimizes the iteration "
	                         /**/ "strategy used, based on which operators have been implemented by sub-classes, as well "
	                         /**/ "as how the sub-range is accessed (i.e. ${##(this * 4)} will invoke ${(##this).operator int() * 4}).\n"
	                         "${"
	                         /**/ "operator * (count) {\n"
	                         /**/ "	import int, Error from deemon;\n"
	                         /**/ "	count = count.operator int();\n"
	                         /**/ "	if (count < 0)\n"
	                         /**/ "		throw Error.ValueError.ArithmeticError.IntegerOverflow();\n"
	                         /**/ "	while (count) {\n"
	                         /**/ "		--count;\n"
	                         /**/ "		yield this...;\n"
	                         /**/ "	}\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "<->\n"
	                         "<=->\n"
	                         "==->\n"
	                         "!=->\n"
	                         ">->\n"
	                         ">=->\n"
	                         "Returns ?t/?f indicative of a lexicographical comparison between @this and @other\n"
	                         "Note that the operators ${this != other} is implemented as ${!(this == other)}, "
	                         /**/ "${this > other} as ${!(this <= other)}, and ${this >= other} and ${!(this < other)}, "
	                         /**/ "meaning that it is sufficient to only implement ${==}, ${<} and ${<=} for full "
	                         /**/ "compare-support, should the user with to provide that kind of functionality\n"
	                         "The lexicographical comparison will makes use of iterators to compare sequences\n"
	                         "\n"

	                         "[]->\n"
	                         "#tOverflowError{The given @index is negative}"
	                         "#tIndexError{The given @index is greater than the length of @this Sequence (${##this})}"
	                         "#tUnboundItem{The item associated with @index is unbound}"
	                         "Returns the @{index}th element of @this Sequence, as determinable by enumeration\n"
	                         "Depending on the nearest implemented group of operators, "
	                         /**/ "one of the following implementations is chosen\n"
	                         "For ${operator [:]}:\n"
	                         "${"
	                         /**/ "operator [] (index: int) {\n"
	                         /**/ "	import int, Sequence, Error from deemon;\n"
	                         /**/ "	index = index.operator int();\n"
	                         /**/ "	local result = this[index:index + 1];\n"
	                         /**/ "	try {\n"
	                         /**/ "		return (result as Sequence).first;\n"
	                         /**/ "	} catch (Error.ValueError) {\n"
	                         /**/ "		throw Error.ValueError.IndexError(\"Index out of bounds\");\n"
	                         /**/ "	}\n"
	                         /**/ "}"
	                         "}\n"
	                         "For ${operator iter}:\n"
	                         "${"
	                         /**/ "operator [] (index: int) {\n"
	                         /**/ "	import int, Error, Signal from deemon;\n"
	                         /**/ "	index = index.operator int();\n"
	                         /**/ "	local it = this.operator iter();\n"
	                         /**/ "	for (;;) {\n"
	                         /**/ "		local result;\n"
	                         /**/ "		try {\n"
	                         /**/ "			result = it.operator next();\n"
	                         /**/ "		} catch (Signal.StopIteration) {\n"
	                         /**/ "			throw Error.ValueError.IndexError(\"Index out of bounds\");\n"
	                         /**/ "		}\n"
	                         /**/ "		if (!index)\n"
	                         /**/ "			return result;\n"
	                         /**/ "		--index;\n"
	                         /**/ "	}\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "#->\n"
	                         "Returns the length of @this Sequence, as determinable by enumeration\n"
	                         "When not implemented by a sub-class, this operator is implemented by ?. as follows\n"
	                         "${"
	                         /**/ "operator # () {\n"
	                         /**/ "	local result = 0;\n"
	                         /**/ "	for (none: this)\n"
	                         /**/ "		++result;\n"
	                         /**/ "	return result;\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "contains->\n"
	                         "Returns ?t/?f indicative of @item being apart of @this Sequence\n"
	                         "This operator is an alias for the #contains member function, "
	                         /**/ "which allows the use of an additional key function\n"
	                         "When not implemented by a sub-class, this operator is implemented by ?. as follows\n"
	                         "${"
	                         /**/ "operator contains(item: Object) {\n"
	                         /**/ "	for (local my_elem: this) {\n"
	                         /**/ "		if (my_elem == item)\n"
	                         /**/ "			return true;\n"
	                         /**/ "	}\n"
	                         /**/ "	return false;\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "[:](start:?Dint,end:?Dint)->\n"
	                         "Returns a sub-range of @this Sequence, spanning across all elements from @start to @end\n"
	                         "If either @start or @end is smaller than ${0}, ${##this} is added once to either\n"
	                         "If @end is greater than the length of @this Sequence, it is clamped to its length\n"
	                         "When @start is greater than, or equal to @end or ${##this}, an empty Sequence is returned\n"
	                         "This operator is implemented similar to the following, however the actual "
	                         /**/ "return type may be a proxy Sequence that further optimizes the iteration "
	                         /**/ "strategy used, based on which operators have been implemented by sub-classes, as well "
	                         /**/ "as how the sub-range is accessed (i.e. ${this[10:20][3]} will invoke ${this[13]}).\n"
	                         "${"
	                         /**/ "operator [:](start: int, end: int): Sequence {\n"
	                         /**/ "	import int, Signal from deemon;\n"
	                         /**/ "	start = start.operator int();\n"
	                         /**/ "	if (end is none) {\n"
	                         /**/ "		local it = this.operator iter();\n"
	                         /**/ "		if (start < 0)\n"
	                         /**/ "			start += ##this;\n"
	                         /**/ "		/* Implementation-specific-variant: */\n"
	                         /**/ "		while (start) {\n"
	                         /**/ "			try {\n"
	                         /**/ "				it.operator next();\n"
	                         /**/ "			} catch(Signal.StopIteration) {\n"
	                         /**/ "				return;\n"
	                         /**/ "			}\n"
	                         /**/ "			--start;\n"
	                         /**/ "		}\n"
	                         /**/ "		foreach (local elem: it)\n"
	                         /**/ "			yield elem;\n"
	                         /**/ "	} else {\n"
	                         /**/ "		end = start.operator int();\n"
	                         /**/ "		if (start < 0 || end < 0) {\n"
	                         /**/ "			local mylen = ##this;\n"
	                         /**/ "			if (start < 0)\n"
	                         /**/ "				start += mylen;\n"
	                         /**/ "			if (end < 0)\n"
	                         /**/ "				end += mylen;\n"
	                         /**/ "		}\n"
	                         /**/ "		if (start >= end)\n"
	                         /**/ "			return Sequence(); /* Empty Sequence */\n"
	                         /**/ "		local it = this.operator iter();\n"
	                         /**/ "		local count = end - start;\n"
	                         /**/ "		while (start) {\n"
	                         /**/ "			try {\n"
	                         /**/ "				it.operator next();\n"
	                         /**/ "			} catch(Signal.StopIteration) {\n"
	                         /**/ "				return;\n"
	                         /**/ "			}\n"
	                         /**/ "			--start;\n"
	                         /**/ "		}\n"
	                         /**/ "		while (count) {\n"
	                         /**/ "			local elem;\n"
	                         /**/ "			try {\n"
	                         /**/ "				elem = it.operator next();\n"
	                         /**/ "			} catch(Signal.StopIteration) {\n"
	                         /**/ "				return;\n"
	                         /**/ "			}\n"
	                         /**/ "			yield elem;\n"
	                         /**/ "			--count;\n"
	                         /**/ "		}\n"
	                         /**/ "	}\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns a general-purpose Iterator using ${operator []} (getitem) and ${operator ##} (size) "
	                         /**/ "to enumerate a Sequence that is implemented using a size+index approach\n"
	                         "${"
	                         /**/ "class Iterator: Iterator from deemon {\n"
	                         /**/ "	private m_idx = 0;\n"
	                         /**/ "	private m_seq;\n"
	                         /**/ "	private m_len;\n"
	                         /**/ "	this(seq: Sequence) {\n"
	                         /**/ "		m_seq = seq;\n"
	                         /**/ "		m_len = ##seq;\n"
	                         /**/ "	}\n"
	                         /**/ "	operator next(): Object {\n"
	                         /**/ "		import Error, Signal from deemon;\n"
	                         /**/ "		for (;;) {\n"
	                         /**/ "			local index = m_idx;\n"
	                         /**/ "			if (index >= m_len)\n"
	                         /**/ "				throw Signal.StopIteration();\n"
	                         /**/ "			++m_idx;\n"
	                         /**/ "			try {\n"
	                         /**/ "				return m_seq[index];\n"
	                         /**/ "			} catch (Error.ValueError.IndexError.UnboundItem) {\n"
	                         /**/ "			}\n"
	                         /**/ "		}\n"
	                         /**/ "	}\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "+=(other:?DSequence)->\n"
	                         "Highly similar to #extend, however if that fails, or if the nearest "
	                         /**/ "matching sub-class provides an implementation for ${operator +}, replace "
	                         /**/ "the caller's storage symbol with ${this + other}, meaning that similar "
	                         /**/ "to when `operator +=' is used with strings, the original Sequence remains "
	                         /**/ "unmodified, but the caller's symbol used to access that Sequence is changed\n"
	                         "${"
	                         /**/ "operator += (items: Sequence) {\n"
	                         /**/ "	import int, Error, Sequence from deemon;\n"
	                         /**/ "	for (local tp = type(this); tp !is none && tp !== Sequence; tp = tp.__base__) {\n"
	                         /**/ "		if (tp.hasprivateattribute(\"extend\")) {\n"
	                         /**/ "			this.extend(items);\n"
	                         /**/ "			return this;\n"
	                         /**/ "		}\n"
	                         /**/ "		if (tp.hasprivateattribute(\"insertall\")) {\n"
	                         /**/ "			this.insertall(-1, items);\n"
	                         /**/ "			return this;\n"
	                         /**/ "		}\n"
	                         /**/ "		if (tp.hasprivateoperator(this, \"setrange\")) {\n"
	                         /**/ "			this[int.SIZE_MAX:int.SIZE_MAX] = items;\n"
	                         /**/ "			return this;\n"
	                         /**/ "		}\n"
	                         /**/ "		if (tp.hasprivateattribute(\"append\")) {\n"
	                         /**/ "			for (local x: items)\n"
	                         /**/ "				this.append(x);\n"
	                         /**/ "			return this;\n"
	                         /**/ "		}\n"
	                         /**/ "		if (tp.hasprivateattribute(\"insert\")) {\n"
	                         /**/ "			for (local x: items)\n"
	                         /**/ "				this.insert(-1, x);\n"
	                         /**/ "			return this;\n"
	                         /**/ "		}\n"
	                         /**/ "		if (tp.hasprivateoperator(\"assign\")) {\n"
	                         /**/ "			this := (this as Sequence) + items;\n"
	                         /**/ "			return this;\n"
	                         /**/ "		}\n"
	                         /**/ "		if (tp.hasprivateoperator(\"add\"))\n"
	                         /**/ "			return this + items;\n"
	                         /**/ "	}\n"
	                         /**/ "	return (this as Sequence) + items;\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "*=(count:?Dint)->\n"
	                         "Inplace-repeat the elements of @this Sequence\n"
	                         "If @this Sequence isn't mutable, replace the caller's symbol with ${this * count}\n"
	                         "${"
	                         /**/ "operator *= (count: int) {\n"
	                         /**/ "	import Sequence from deemon;\n"
	                         /**/ "	for (local tp = type(this); tp !is none && tp !== Sequence; tp = tp.__base__) {\n"
	                         /**/ "		if (tp.hasprivateoperator(\"assign\")) {\n"
	                         /**/ "			count = count.operator int();\n"
	                         /**/ "			if (count != 1) {\n"
	                         /**/ "				this := (this as Sequence) * count;\n"
	                         /**/ "			}\n"
	                         /**/ "			return this;\n"
	                         /**/ "		}\n"
	                         /**/ "		if (tp.hasprivateoperator(\"setrange\")) {\n"
	                         /**/ "			count = count.operator int();\n"
	                         /**/ "			if (count != 1) {\n"
	                         /**/ "				this[:] = (this as Sequence) * count;\n"
	                         /**/ "			}\n"
	                         /**/ "			return this;\n"
	                         /**/ "		}\n"
	                         /**/ "		if (tp.hasprivateoperator(\"mul\"))\n"
	                         /**/ "			return this * count;\n"
	                         /**/ "	}\n"
	                         /**/ "	return (this as Sequence) * count;\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         ":=->\n"
	                         "#tSequenceError{@this Sequence is immutable}"
	                         "For mutable sequences only: Assign the contents from @other to @this Sequence\n"
	                         "When this operator is lacking, the following "
	                         /**/ "default-implementation is provided by ?.:\n"
	                         "${"
	                         /**/ "operator := (other: Sequence) {\n"
	                         /**/ "	import Error from deemon;\n"
	                         /**/ "	try {\n"
	                         /**/ "		this[:] = other; /* Implemented using `setrange(none, none, other)' */\n"
	                         /**/ "	} catch (Error.RuntimeError.NotImplemented) {\n"
	                         /**/ "		throw Error.ValueError.SequenceError(\"Immutable Sequence\");\n"
	                         /**/ "	}\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "del[]->\n"
	                         "#tIntegerOverflow{The given @index is negative, or too large}"
	                         "#tIndexError{The given @index is out of bounds}"
	                         "#tUnboundItem{The item associated with @index had already been unbound}"
	                         "#tSequenceError{@this Sequence is immutable}"
	                         "For mutable sequences only: Remove the item found at @index, either "
	                         /**/ "removing it from the Sequence, or changing it to being unbound\n"
	                         "When this operator is lacking, but ${operator del[:]}, ${operator [:]=} or "
	                         /**/ "${erase(index: int, count: int = 1)} are available, the following "
	                         /**/ "default-implementation is provided by ?.:\n"
	                         "${"
	                         /**/ "operator del[] (index: int) {\n"
	                         /**/ "	import int, Error from deemon;\n"
	                         /**/ "	index = index.operator int();\n"
	                         /**/ "	if (index < 0)\n"
	                         /**/ "		throw Error.ValueError.ArithmeticError.IntegerOverflow();\n"
	                         /**/ "	for (local tp = type(this); tp !is none && tp !== Sequence; tp = tp.__base__) {\n"
	                         /**/ "		if (tp.hasprivateattribute(\"erase\")) {\n"
	                         /**/ "			this.erase(index, 1);\n"
	                         /**/ "			return;\n"
	                         /**/ "		}\n"
	                         /**/ "		if (tp.hasprivateoperator(\"delrange\")) {\n"
	                         /**/ "			if (index >= ##this)\n"
	                         /**/ "				throw Error.ValueError.IndexError();\n"
	                         /**/ "			del self[index:index + 1];\n"
	                         /**/ "			return;\n"
	                         /**/ "		}\n"
	                         /**/ "		if (tp.hasprivateoperator(\"setrange\")) {\n"
	                         /**/ "			if (index >= ##this)\n"
	                         /**/ "				throw Error.ValueError.IndexError();\n"
	                         /**/ "			self[index:index + 1] = none;\n"
	                         /**/ "			return;\n"
	                         /**/ "		}\n"
	                         /**/ "	}\n"
	                         /**/ "	throw Error.ValueError.SequenceError(\"Immutable Sequence\");\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "[]=->\n"
	                         "#tIntegerOverflow{The given @index is negative, or too large}"
	                         "#tIndexError{The given @index is out of bounds}"
	                         "#tSequenceError{@this Sequence is immutable}"
	                         "For mutable sequences only: Override the item found at @index with @value\n"
	                         "When this operator is lacking, but ${operator [:]=} is available, the following "
	                         /**/ "default-implementation is provided by ?.:\n"
	                         "${"
	                         /**/ "operator []= (index: int, value) {\n"
	                         /**/ "	import int, Error from deemon;\n"
	                         /**/ "	index = index.operator int();\n"
	                         /**/ "	if (index < 0)\n"
	                         /**/ "		throw Error.ValueError.ArithmeticError.IntegerOverflow();\n"
	                         /**/ "	if (index >= ##this)\n"
	                         /**/ "		throw Error.ValueError.IndexError();\n"
	                         /**/ "	self[index:index + 1] = { value };\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "del[:]->\n"
	                         "#tIntegerOverflow{@start or @end are too large}"
	                         "#tSequenceError{@this Sequence cannot be resized}"
	                         "For mutable sequences only: Delete, or unbind all items within the given range\n"
	                         "When this operator is lacking, the following default-implementation is "
	                         /**/ "provided by ?. when the required operators and members are present.\n"
	                         "Also note that in order to properly function in resizable sequences, the member function "
	                         /**/ "${erase(index: int, count: int = 1)} must be defined by a sub-class\n"
	                         "${"
	                         /**/ "operator del[:] (start: int, end: int) {\n"
	                         /**/ "	import int, Error, Signal from deemon;\n"
	                         /**/ "	start = start.operator int();\n"
	                         /**/ "	if (end is none) {\n"
	                         /**/ "		if (start < 0)\n"
	                         /**/ "			start += (##this).operator int();\n"
	                         /**/ "		for (local tp = type(this); tp !is none && tp !== Sequence; tp = tp.__base__) {\n"
	                         /**/ "			if (start == 0) {\n"
	                         /**/ "				if (!tp.hasprivateattribute(\"clear\")) {\n"
	                         /**/ "					this.clear();\n"
	                         /**/ "					return;\n"
	                         /**/ "				}\n"
	                         /**/ "			}\n"
	                         /**/ "			if (tp.hasprivateoperator(\"setrange\")) {\n"
	                         /**/ "				this[start:] = none;\n"
	                         /**/ "				return;\n"
	                         /**/ "			}\n"
	                         /**/ "			if (tp.hasprivateattribute(\"erase\")) {\n"
	                         /**/ "				local mylen = ##this;\n"
	                         /**/ "				if (start > mylen)\n"
	                         /**/ "					start = mylen;\n"
	                         /**/ "				this.erase(start, mylen - start);\n"
	                         /**/ "				return;\n"
	                         /**/ "			}\n"
	                         /**/ "			if (tp.hasprivateoperator(\"del[]\")) {\n"
	                         /**/ "				while (end > start) {\n"
	                         /**/ "					--end;\n"
	                         /**/ "					del this[end];\n"
	                         /**/ "				}\n"
	                         /**/ "				return;\n"
	                         /**/ "			}\n"
	                         /**/ "		}\n"
	                         /**/ "		throw Error.ValueError.SequenceError(\"Sequence not resizable\");\n"
	                         /**/ "	} else {\n"
	                         /**/ "		end = end.operator int();\n"
	                         /**/ "		if (start < 0 || end < 0) {\n"
	                         /**/ "			local mylen = (##this).operator int();\n"
	                         /**/ "			if (start < 0)\n"
	                         /**/ "				start += mylen;\n"
	                         /**/ "			if (end < 0)\n"
	                         /**/ "				end += mylen;\n"
	                         /**/ "		}\n"
	                         /**/ "		for (local tp = type(this); tp !is none && tp !== Sequence; tp = tp.__base__) {\n"
	                         /**/ "			if (tp.hasprivateoperator(\"setrange\")) {\n"
	                         /**/ "				this[start:end] = none;\n"
	                         /**/ "				return;\n"
	                         /**/ "			}\n"
	                         /**/ "			if (tp.hasprivateattribute(\"erase\")) {\n"
	                         /**/ "				local mylen = ##this;\n"
	                         /**/ "				if (start > mylen)\n"
	                         /**/ "					start = mylen;\n"
	                         /**/ "				if (end < start)\n"
	                         /**/ "					end = start;\n"
	                         /**/ "				this.erase(start, end - start);\n"
	                         /**/ "				return;\n"
	                         /**/ "			}\n"
	                         /**/ "			if (tp.hasprivateoperator(\"delitem\")) {\n"
	                         /**/ "				while (end > start) {\n"
	                         /**/ "					--end;\n"
	                         /**/ "					del this[end];\n"
	                         /**/ "				}\n"
	                         /**/ "				return;\n"
	                         /**/ "			}\n"
	                         /**/ "		}\n"
	                         /**/ "		throw Error.ValueError.SequenceError(\"Sequence not resizable\");\n"
	                         /**/ "	}\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "[:]=->\n"
	                         "#tIntegerOverflow{@start or @end are too large}"
	                         "#tSequenceError{@this Sequence is immutable, or cannot be resized}"
	                         "For mutable sequences only: Override the given range with items from @{values}:\n"
	                         "When this operator is lacking, the following default-implementation is "
	                         /**/ "provided by ?. when the required operators and members are present.\n"
	                         "Also note that in order to properly function in resizable sequences, 2 member functions "
	                         /**/ "${insert(index: int, item: Object)} and ${erase(index: int, count: int = 1)} must be defined "
	                         /**/ "by a sub-class\n"
	                         "${"
	                         /**/ "operator [:]= (start: int, end: int, values: Sequence) {\n"
	                         /**/ "	import int, Error, Signal, Iterator, Sequence from deemon;\n"
	                         /**/ "	start = start.operator int();\n"
	                         /**/ "	if (values is none) {\n"
	                         /**/ "		del this[start:end];\n"
	                         /**/ "		return;\n"
	                         /**/ "	}\n"
	                         /**/ "	if (start < 0 || (end is none || ((end = end.operator int()) < 0))) {\n"
	                         /**/ "		local mylen = ##this;\n"
	                         /**/ "		if (start < 0)\n"
	                         /**/ "			start += mylen;\n"
	                         /**/ "		if (end is none) {\n"
	                         /**/ "			end = mylen;\n"
	                         /**/ "		} else if (end < 0) {\n"
	                         /**/ "			end += mylen;\n"
	                         /**/ "		}\n"
	                         /**/ "	}\n"
	                         /**/ "	if (start >= end) {\n"
	                         /**/ "		local mylen = ##this;\n"
	                         /**/ "		if (start > mylen)\n"
	                         /**/ "			start = mylen;\n"
	                         /**/ "		try {\n"
	                         /**/ "			this.insertall(start, values);\n"
	                         /**/ "		} catch (Error.AttributeError | Error.RuntimeError.NotImplemented) {\n"
	                         /**/ "			if (!##values)\n"
	                         /**/ "				return;\n"
	                         /**/ "			throw Error.ValueError.SequenceError(\"Sequence not resizable\");\n"
	                         /**/ "		}\n"
	                         /**/ "		return;\n"
	                         /**/ "	}\n"
	                         /**/ "	local values_is_empty;\n"
	                         /**/ "	for (local tp = type(this); tp !is none && tp !== Sequence; tp = tp.__base__) {\n"
	                         /**/ "		if (tp.hasprivateoperator(\"setitem\")) {\n"
	                         /**/ "			local it = values.operator iter();\n"
	                         /**/ "			/* Override existing / Delete trailing */\n"
	                         /**/ "			while (start < end) {\n"
	                         /**/ "				local elem;\n"
	                         /**/ "				try {\n"
	                         /**/ "					elem = it.operator next();\n"
	                         /**/ "				} catch (Signal.StopIteration) {\n"
	                         /**/ "					try {\n"
	                         /**/ "						this.erase(start, end - start);\n"
	                         /**/ "					} catch (Error.AttributeError | Error.RuntimeError.NotImplemented) {\n"
	                         /**/ "						throw Error.ValueError.SequenceError(\"Sequence not resizable\");\n"
	                         /**/ "					}\n"
	                         /**/ "					return;\n"
	                         /**/ "				}\n"
	                         /**/ "				this[start] = elem;\n"
	                         /**/ "				++start;\n"
	                         /**/ "			}\n"
	                         /**/ "			/* Insert remainder */\n"
	                         /**/ "			try {\n"
	                         /**/ "				this.insertall(start, (it as Iterator).pending);\n"
	                         /**/ "			} catch (Error.AttributeError | Error.RuntimeError.NotImplemented) {\n"
	                         /**/ "				/* If the input-Sequence was fully iterated, then\n"
	                         /**/ "				 * we don't actually need to resize the Sequence */\n"
	                         /**/ "				try {\n"
	                         /**/ "					it.operator next();\n"
	                         /**/ "				} catch (Signal.StopIteration) {\n"
	                         /**/ "					return;\n"
	                         /**/ "				}\n"
	                         /**/ "				throw Error.ValueError.SequenceError(\"Sequence not resizable\");\n"
	                         /**/ "			}\n"
	                         /**/ "			return;\n"
	                         /**/ "		}\n"
	                         /**/ "		if (values_is_empty !is bound)\n"
	                         /**/ "			values_is_empty = !(##values).operator int();\n"
	                         /**/ "		if (values_is_empty) {\n"
	                         /**/ "			if (tp.hasprivateoperator(\"delrange\")) {\n"
	                         /**/ "				del this[start:end];\n"
	                         /**/ "				return;\n"
	                         /**/ "			}\n"
	                         /**/ "			if (tp.hasprivateattribute(\"erase\")) {\n"
	                         /**/ "				this.erase(start, end - start);\n"
	                         /**/ "				return;\n"
	                         /**/ "			}\n"
	                         /**/ "			if (tp.hasprivateoperator(\"delitem\")) {\n"
	                         /**/ "				do {\n"
	                         /**/ "					--end;\n"
	                         /**/ "					del this[end];\n"
	                         /**/ "				} while (end > start);\n"
	                         /**/ "				return;\n"
	                         /**/ "			}\n"
	                         /**/ "		}\n"
	                         /**/ "	}\n"
	                         /**/ "	throw Error.ValueError.SequenceError(\"Immutable Sequence\");\n"
	                         /**/ "}"
	                         "}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT | TP_FNAMEOBJECT, /* Generic base class type. */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&none_i1, /* Allow default-construction of Sequence objects. */
				/* .tp_copy_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_deep_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ &seq_assign,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ &DeeSeq_NonEmpty,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ &seq_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &seq_math,
	/* .tp_cmp           = */ &seq_cmp,
	/* .tp_seq           = */ &seq_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ seq_methods,
	/* .tp_getsets       = */ seq_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ seq_class_methods,
	/* .tp_class_getsets = */ seq_class_getsets,
	/* .tp_class_members = */ NULL
};

/* An empty instance of a generic sequence object.
 * NOTE: This is _NOT_ a singleton. - Usercode may create more by
 *       calling the constructor of `DeeSeq_Type' with no arguments.
 *       Though this statically allocated instance is used by most
 *       internal sequence functions.
 * HINT: Any exact instance of `DeeSeq_Type' should be considered stub/empty,
 *       but obviously something like an empty tuple is also an empty sequence. */
PUBLIC DeeObject DeeSeq_EmptyInstance = {
	OBJECT_HEAD_INIT(&DeeSeq_Type)
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_C */
