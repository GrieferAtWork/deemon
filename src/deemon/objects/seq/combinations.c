/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_COMBINATIONS_C
#define GUARD_DEEMON_OBJECTS_SEQ_COMBINATIONS_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/lock.h>

#include "combinations.h"
/**/

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

DECL_BEGIN

#ifndef CONFIG_HAVE_memsetp
#define CONFIG_HAVE_memsetp
#define memsetp(dst, pointer, num_pointers) \
	dee_memsetp(dst, (__UINTPTR_TYPE__)(pointer), num_pointers)
DeeSystem_DEFINE_memsetp(dee_memsetp)
#endif /* !CONFIG_HAVE_memsetp */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Combinations_GetSeqItem(Combinations *__restrict self, size_t index) {
	DREF DeeObject *temp, *result;
	struct type_nsi const *nsi;
	ASSERT(index < self->c_seqlen);
	if (self->c_elem)
		return_reference_(self->c_elem[index]);
	ASSERT(self->c_getitem != NULL);
	ASSERT(self->c_getitem->tp_getitem != NULL);
	nsi = self->c_getitem->tp_nsi;
	if (nsi &&
	    nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
	    nsi->nsi_seqlike.nsi_getitem)
		return (*nsi->nsi_seqlike.nsi_getitem)(self->c_seq, index);
	temp = DeeInt_NewSize(index);
	if unlikely(!temp)
		goto err;
	result = DeeType_invoke_seq_tp_getitem(self->c_getitem_tp,
	                                       self->c_getitem->tp_getitem,
	                                       self->c_seq, temp);
	Dee_Decref(temp);
	return result;
err:
	return NULL;
}


PRIVATE NONNULL((1)) void DCALL
comiter_fini(CombinationsIterator *__restrict self) {
	Dee_Free(self->ci_indices);
	Dee_Decref_likely(self->ci_combi);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
comiter_ctor(CombinationsIterator *__restrict self) {
	self->ci_combi = (DREF Combinations *)DeeObject_NewDefault(&SeqCombinations_Type);
	if unlikely(!self->ci_combi)
		goto err;
	self->ci_indices = (size_t *)Dee_Callocc(self->ci_combi->c_comlen,
	                                         sizeof(size_t));
	if unlikely(!self->ci_indices)
		goto err_combi;
	self->ci_first = true;
	Dee_atomic_rwlock_init(&self->ci_lock);
	return 0;
err_combi:
	Dee_Decref_likely(self->ci_combi);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
comiter_init(CombinationsIterator *__restrict self,
             size_t argc, DeeObject *const *argv) {
	size_t i, comlen;
	if (DeeArg_Unpack(argc, argv, "o:_SeqCombinationsIterator", &self->ci_combi))
		goto err;
	if (DeeObject_AssertTypeExact(self->ci_combi, &SeqCombinations_Type))
		goto err;
	Dee_atomic_rwlock_init(&self->ci_lock);
	comlen           = self->ci_combi->c_comlen;
	self->ci_indices = (size_t *)Dee_Mallocc(comlen, sizeof(size_t));
	if unlikely(!self->ci_indices)
		goto err;
	for (i = 0; i < comlen; ++i)
		self->ci_indices[i] = i;
	self->ci_first = true;
	Dee_Incref(self->ci_combi);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
comiter_copy(CombinationsIterator *__restrict self,
             CombinationsIterator *__restrict other) {
	self->ci_indices = (size_t *)Dee_Mallocc(other->ci_combi->c_comlen,
	                                         sizeof(size_t));
	if unlikely(!self->ci_indices)
		goto err;
	CombinationsIterator_LockRead(other);
	memcpyc(self->ci_indices,
	        other->ci_indices,
	        other->ci_combi->c_comlen,
	        sizeof(size_t));
	self->ci_first = other->ci_first;
	CombinationsIterator_LockEndRead(other);
	Dee_atomic_rwlock_init(&self->ci_lock);
	self->ci_combi = other->ci_combi;
	Dee_Incref(self->ci_combi);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
comiter_deep(CombinationsIterator *__restrict self,
             CombinationsIterator *__restrict other) {
	self->ci_indices = (size_t *)Dee_Mallocc(other->ci_combi->c_comlen,
	                                         sizeof(size_t));
	if unlikely(!self->ci_indices)
		goto err;
	CombinationsIterator_LockRead(other);
	memcpyc(self->ci_indices,
	        other->ci_indices,
	        other->ci_combi->c_comlen,
	        sizeof(size_t));
	self->ci_first = other->ci_first;
	CombinationsIterator_LockEndRead(other);
	Dee_atomic_rwlock_init(&self->ci_lock);
	self->ci_combi = (DREF Combinations *)DeeObject_DeepCopy((DeeObject *)other->ci_combi);
	if unlikely(!self->ci_combi)
		goto err_indices;
	return 0;
err_indices:
	Dee_Free(self->ci_indices);
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) void DCALL
comiter_visit(CombinationsIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ci_combi);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
comiter_next(CombinationsIterator *__restrict self) {
	DREF DeeTupleObject *result;
	size_t *result_indices, i, comlen, seqlen;
	comlen         = self->ci_combi->c_comlen;
	seqlen         = self->ci_combi->c_seqlen;
	result_indices = (size_t *)Dee_Mallocac(comlen, sizeof(size_t));
	if unlikely(!result_indices)
		goto err;
	CombinationsIterator_LockWrite(self);
	if (self->ci_first) {
		self->ci_first = false;
		goto copy_indices;
	}
	for (i = comlen; i;) {
		--i;
		if (self->ci_indices[i] != i + seqlen - comlen)
			goto update_indices;
	}
	/* Signal `ITER_DONE' */
	CombinationsIterator_LockEndWrite(self);
	Dee_Freea(result_indices);
	return (DREF DeeTupleObject *)ITER_DONE;
update_indices:
	++self->ci_indices[i];
	for (++i; i < comlen; ++i)
		self->ci_indices[i] = self->ci_indices[i - 1] + 1;
copy_indices:
	memcpyc(result_indices,
	        self->ci_indices,
	        comlen,
	        sizeof(size_t));
	CombinationsIterator_LockEndWrite(self);
	result = DeeTuple_NewUninitialized(comlen);
	if unlikely(!result)
		goto err_indices;
	for (i = 0; i < comlen; ++i) {
		DREF DeeObject *temp;
		temp = Combinations_GetSeqItem(self->ci_combi,
		                               result_indices[i]);
		if unlikely(!temp)
			goto err_indices_r;
		DeeTuple_SET(result, i, temp); /* Inherit reference. */
	}
	Dee_Freea(result_indices);
	return result;
err_indices_r:
	Dee_Decrefv(DeeTuple_ELEM(result), i);
	DeeTuple_FreeUninitialized(result);
err_indices:
	Dee_Freea(result_indices);
err:
	return NULL;
}

PRIVATE struct type_member tpconst comiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(CombinationsIterator, ci_combi), "->?Ert:SeqCombinations"),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
comiter_compare(CombinationsIterator *self, CombinationsIterator *other) {
	size_t i;
	int result = 0;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	Dee_return_compare_if_ne(self->ci_combi, other->ci_combi);
	DeeLock_Acquire2(CombinationsIterator_LockRead(self),
	                 CombinationsIterator_LockTryRead(self),
	                 CombinationsIterator_LockEndRead(self),
	                 CombinationsIterator_LockRead(other),
	                 CombinationsIterator_LockTryRead(other),
	                 CombinationsIterator_LockEndRead(other));
	for (i = 0; i < self->ci_combi->c_comlen; ++i) {
		size_t lhs_index = self->ci_indices[i];
		size_t rhs_index = other->ci_indices[i];
		if (lhs_index != rhs_index) {
			result = Dee_CompareNe(lhs_index, rhs_index);
			break;
		}
	}
	CombinationsIterator_LockEndRead(other);
	CombinationsIterator_LockEndRead(self);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp comiter_cmp = {
	/* .tp_hash       = */ NULL,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&comiter_compare,
};

INTERN DeeTypeObject SeqCombinationsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqCombinationsIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqCombinations)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&comiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&comiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&comiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&comiter_init,
				TYPE_FIXED_ALLOCATOR(CombinationsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&comiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&comiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &comiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&comiter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ comiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE WUNUSED NONNULL((1)) DREF CombinationsIterator *DCALL
com_iter(Combinations *__restrict self) {
	DREF CombinationsIterator *result;
	size_t i, comlen;
	result = DeeObject_MALLOC(CombinationsIterator);
	if unlikely(!result)
		goto done;
	result->ci_combi = self;
	Dee_atomic_rwlock_init(&result->ci_lock);
	comlen             = self->c_comlen;
	result->ci_indices = (size_t *)Dee_Mallocc(comlen, sizeof(size_t));
	if unlikely(!result->ci_indices)
		goto err_r;
	for (i = 0; i < comlen; ++i)
		result->ci_indices[i] = i;
	result->ci_first = true;
	Dee_Incref(self);
	DeeObject_Init(result, &SeqCombinationsIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE int DCALL
com_bool(Combinations *__restrict UNUSED(self)) {
	/* Combinations are always non-empty.
	 * The creator function `DeeSeq_Combinations()' returns a
	 * different sequence type if we would have been empty. */
	return 1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
com_copy(Combinations *__restrict self,
         Combinations *__restrict other) {
	/* NOTE: `DeeTuple_ELEM()' just returns an invalid
	 *        pointer for any object that isn't a tuple. */
	self->c_elem = other->c_elem;
	if (other->c_elem &&
	    other->c_elem != DeeTuple_ELEM(other->c_seq)) {
		DREF DeeObject **elem_copy;
		ASSERT(other->c_seqlen != 0);
		elem_copy = (DREF DeeObject **)Dee_Mallocc(other->c_seqlen,
		                                           sizeof(DREF DeeObject *));
		if unlikely(!elem_copy)
			goto err;
		self->c_elem = Dee_Movrefv(elem_copy,
		                           other->c_elem,
		                           other->c_seqlen);
	}
	self->c_seq        = other->c_seq;
	self->c_seqlen     = other->c_seqlen;
	self->c_comlen     = other->c_comlen;
	self->c_getitem    = other->c_getitem;
	self->c_getitem_tp = other->c_getitem_tp;
	Dee_Incref(self->c_seq);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
com_deepcopy(Combinations *__restrict self,
             Combinations *__restrict other) {
	/* NOTE: `DeeTuple_ELEM()' just returns an invalid
	 *        pointer for any object that isn't a tuple. */
	self->c_elem   = other->c_elem;
	self->c_comlen = other->c_comlen;
	if (other->c_elem &&
	    other->c_elem != DeeTuple_ELEM(other->c_seq)) {
		size_t i;
		DREF DeeObject **elem_copy;
		ASSERT(other->c_seqlen != 0);
		elem_copy = (DREF DeeObject **)Dee_Mallocc(other->c_seqlen,
		                                           sizeof(DREF DeeObject *));
		if unlikely(!elem_copy)
			goto err;
		for (i = 0; i < other->c_seqlen; ++i) {
			elem_copy[i] = DeeObject_DeepCopy(other->c_elem[i]);
			if unlikely(!elem_copy[i]) {
				Dee_Decrefv(elem_copy, i);
				Dee_Free(elem_copy);
				goto err;
			}
		}
		self->c_elem = elem_copy;
		self->c_seq  = other->c_seq;
		Dee_Incref(self->c_seq);
		self->c_seqlen = other->c_seqlen;
	} else {
		self->c_seq = DeeObject_DeepCopy(other->c_seq);
		if unlikely(!self->c_seq)
			goto err;
		ASSERT(Dee_TYPE(self->c_seq) == Dee_TYPE(other->c_seq));
		if (other->c_elem == DeeTuple_ELEM(other->c_seq)) {
			self->c_elem   = DeeTuple_ELEM(self->c_seq);
			self->c_seqlen = other->c_seqlen;
		} else {
			ASSERT(self->c_elem == NULL);
			ASSERT(other->c_elem == NULL);
			/* Reload the sequence length, as it may have
			 * changed after copying the underlying sequence. */
			self->c_seqlen = DeeObject_Size(self->c_seq);
			if unlikely(self->c_seqlen == (size_t)-1) {
err_seq_len:
				Dee_Decref(self->c_seq);
				goto err;
			}
			if unlikely(self->c_seqlen == 0) {
				err_empty_sequence(self->c_seq);
				goto err_seq_len;
			}
			/* Make sure that the sequence fulfills the minimum length requirements. */
			if unlikely(self->c_comlen >= self->c_seqlen) {
				DeeError_Throwf(&DeeError_ValueError,
				                "Sequence too short after deepcopy (needs at "
				                "least %" PRFuSIZ " items, but only has %" PRFuSIZ ")",
				                self->c_comlen, self->c_seqlen);
				goto err_seq_len;
			}
		}
	}
	self->c_getitem    = other->c_getitem;
	self->c_getitem_tp = other->c_getitem_tp;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
com_ctor(Combinations *__restrict self) {
	self->c_seq = DeeTuple_Pack(1, Dee_None);
	if unlikely(!self->c_seq)
		goto err;
	self->c_elem       = DeeTuple_ELEM(self->c_seq);
	self->c_seqlen     = 1;
	self->c_comlen     = 1;
	self->c_getitem    = DeeTuple_Type.tp_seq;
	self->c_getitem_tp = &DeeTuple_Type;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
com_fini(Combinations *__restrict self) {
	/* NOTE: `DeeTuple_ELEM()' just returns an invalid
	 *        pointer for any object that isn't a tuple. */
	if (self->c_elem &&
	    self->c_elem != DeeTuple_ELEM(self->c_seq)) {
		Dee_Decrefv(self->c_elem, self->c_seqlen);
		Dee_Free(self->c_elem);
	}
	Dee_Decref(self->c_seq);
}

PRIVATE NONNULL((1, 2)) void DCALL
com_visit(Combinations *__restrict self, dvisit_t proc, void *arg) {
	/* NOTE: `DeeTuple_ELEM()' just returns an invalid
	 *        pointer for any object that isn't a tuple. */
	if (self->c_elem &&
	    self->c_elem != DeeTuple_ELEM(self->c_seq))
		Dee_Visitv(self->c_elem, self->c_seqlen);
	Dee_Visit(self->c_seq);
}

PRIVATE struct type_seq com_seq = {
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&com_iter
};

PRIVATE struct type_member tpconst com_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqCombinationsIterator_Type),
	/* TODO: "Frozen" */
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst com_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(Combinations, c_seq), "->?DSequence"),
	/* TODO: GETTER: "frozen" */
	TYPE_MEMBER_END
};


INTERN DeeTypeObject SeqCombinations_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqCombinations",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&com_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&com_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&com_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(Combinations)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&com_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&com_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&com_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &com_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ com_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ com_class_members
};




PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
rcomiter_next(CombinationsIterator *__restrict self) {
	DREF DeeTupleObject *result;
	size_t *result_indices, i, comlen, seqlen, offset;
	comlen = self->ci_combi->c_comlen;
	seqlen = self->ci_combi->c_seqlen;
	if (self->ci_first) {
		CombinationsIterator_LockWrite(self);
		COMPILER_READ_BARRIER();
		if (!self->ci_first) {
			CombinationsIterator_LockEndWrite(self);
		} else {
			DREF DeeObject *elem;
			self->ci_first = false;
			CombinationsIterator_LockEndWrite(self);
			result = DeeTuple_NewUninitialized(comlen);
			if unlikely(!result)
				goto err;
			elem = Combinations_GetSeqItem(self->ci_combi, 0);
			if unlikely(!elem) {
				DeeTuple_FreeUninitialized(result);
				goto err;
			}
			memsetp(DeeTuple_ELEM(result), elem, comlen);
			Dee_Incref_n(elem, comlen);
			return result;
		}
	}
	result_indices = (size_t *)Dee_Mallocac(comlen, sizeof(size_t));
	if unlikely(!result_indices)
		goto err;
	CombinationsIterator_LockWrite(self);
	i = comlen;
	while (i--) {
		if (self->ci_indices[i] != seqlen - 1)
			goto update_indices;
	}
	/* Signal `ITER_DONE' */
	CombinationsIterator_LockEndWrite(self);
	Dee_Freea(result_indices);
	return (DREF DeeTupleObject *)ITER_DONE;
update_indices:
	offset = self->ci_indices[i] + 1;
	for (; i < comlen; ++i)
		self->ci_indices[i] = offset;
	memcpyc(result_indices,
	        self->ci_indices,
	        comlen,
	        sizeof(size_t));
	CombinationsIterator_LockEndWrite(self);
	result = DeeTuple_NewUninitialized(comlen);
	if unlikely(!result)
		goto err_indices;
	for (i = 0; i < comlen; ++i) {
		DREF DeeObject *temp;
		size_t j, index = result_indices[i];
		for (j = 0; j < i; ++j) {
			if (result_indices[j] == index) {
				temp = DeeTuple_GET(result, j);
				Dee_Incref(temp);
				goto set_temp;
			}
		}
		temp = Combinations_GetSeqItem(self->ci_combi, index);
		if unlikely(!temp)
			goto err_indices_r;
set_temp:
		DeeTuple_SET(result, i, temp); /* Inherit reference. */
	}
	Dee_Freea(result_indices);
	return result;
err_indices_r:
	Dee_Decrefv(DeeTuple_ELEM(result), i);
	DeeTuple_FreeUninitialized(result);
err_indices:
	Dee_Freea(result_indices);
err:
	return NULL;
}

PRIVATE struct type_member tpconst rcomiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(CombinationsIterator, ci_combi), "->?Ert:SeqRepeatCombinations"),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
rcomiter_ctor(CombinationsIterator *__restrict self) {
	self->ci_combi = (DREF Combinations *)DeeObject_NewDefault(&SeqRepeatCombinations_Type);
	if unlikely(!self->ci_combi)
		goto err;
	self->ci_indices = (size_t *)Dee_Callocc(self->ci_combi->c_comlen,
	                                         sizeof(size_t));
	if unlikely(!self->ci_indices)
		goto err_combi;
	self->ci_first = true;
	Dee_atomic_rwlock_init(&self->ci_lock);
	return 0;
err_combi:
	Dee_Decref_likely(self->ci_combi);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rcomiter_init(CombinationsIterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	size_t i, comlen;
	if (DeeArg_Unpack(argc, argv, "o:_SeqRepeatCombinationsIterator", &self->ci_combi))
		goto err;
	if (DeeObject_AssertTypeExact(self->ci_combi, &SeqRepeatCombinations_Type))
		goto err;
	Dee_atomic_rwlock_init(&self->ci_lock);
	comlen           = self->ci_combi->c_comlen;
	self->ci_indices = (size_t *)Dee_Mallocc(comlen, sizeof(size_t));
	if unlikely(!self->ci_indices)
		goto err;
	for (i = 0; i < comlen; ++i)
		self->ci_indices[i] = i;
	self->ci_first = true;
	Dee_Incref(self->ci_combi);
	return 0;
err:
	return -1;
}

INTERN DeeTypeObject SeqRepeatCombinationsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRepeatCombinationsIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqRepeatCombinations)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rcomiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&comiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&comiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&rcomiter_init,
				TYPE_FIXED_ALLOCATOR(CombinationsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&comiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&comiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &comiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rcomiter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rcomiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE WUNUSED NONNULL((1)) DREF CombinationsIterator *DCALL
rcom_iter(Combinations *__restrict self) {
	DREF CombinationsIterator *result;
	result = DeeObject_MALLOC(CombinationsIterator);
	if unlikely(!result)
		goto done;
	result->ci_combi = self;
	Dee_atomic_rwlock_init(&result->ci_lock);
	result->ci_indices = (size_t *)Dee_Callocc(self->c_comlen, sizeof(size_t));
	if unlikely(!result->ci_indices)
		goto err_r;
	result->ci_first = true;
	Dee_Incref(self);
	DeeObject_Init(result, &SeqRepeatCombinationsIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE struct type_seq rcom_seq = {
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rcom_iter
};

PRIVATE struct type_member tpconst rcom_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqRepeatCombinationsIterator_Type),
	TYPE_MEMBER_END
};

#define rcom_members com_members

INTERN DeeTypeObject SeqRepeatCombinations_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRepeatCombinations",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&com_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&com_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&com_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(Combinations)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&com_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&com_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&com_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &rcom_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rcom_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rcom_class_members
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
pmutiter_next(CombinationsIterator *__restrict self) {
	DREF DeeTupleObject *result;
	size_t *result_indices, i, comlen, seqlen;
	comlen         = self->ci_combi->c_comlen;
	seqlen         = self->ci_combi->c_seqlen;
	result_indices = (size_t *)Dee_Mallocac(comlen, sizeof(size_t));
	if unlikely(!result_indices)
		goto err;
	CombinationsIterator_LockWrite(self);
	if (self->ci_first) {
		self->ci_first = false;
		goto copy_indices;
	}
	if (self->ci_indices[0] >= seqlen) {
		/* Signal `ITER_DONE' */
signal_done:
		CombinationsIterator_LockEndWrite(self);
		Dee_Freea(result_indices);
		return (DREF DeeTupleObject *)ITER_DONE;
	}
	i = comlen;
	for (;;) {
		size_t j, index;
		--i;
increment_i:
		if (++self->ci_indices[i] >= seqlen) {
			if (i == 0) {
				self->ci_indices[i] = seqlen - 1;
				goto signal_done;
			}
			self->ci_indices[i] = 0;
			continue;
		}
		index = self->ci_indices[i];
		for (j = 0; j < comlen; ++j) {
			if (j == i)
				continue;
			if (self->ci_indices[j] == index)
				goto increment_i;
		}
		break;
	}
copy_indices:
	memcpyc(result_indices,
	        self->ci_indices,
	        comlen,
	        sizeof(size_t));
	CombinationsIterator_LockEndWrite(self);
	result = DeeTuple_NewUninitialized(comlen);
	if unlikely(!result)
		goto err_indices;
	for (i = 0; i < comlen; ++i) {
		DREF DeeObject *temp;
		temp = Combinations_GetSeqItem(self->ci_combi,
		                               result_indices[i]);
		if unlikely(!temp)
			goto err_indices_r;
		DeeTuple_SET(result, i, temp); /* Inherit reference. */
	}
	Dee_Freea(result_indices);
	return result;
err_indices_r:
	Dee_Decrefv(DeeTuple_ELEM(result), i);
	DeeTuple_FreeUninitialized(result);
err_indices:
	Dee_Freea(result_indices);
err:
	return NULL;
}

PRIVATE struct type_member tpconst pmutiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(CombinationsIterator, ci_combi), "->?Ert:SeqPermutations"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
pmutiter_ctor(CombinationsIterator *__restrict self) {
	self->ci_combi = (DREF Combinations *)DeeObject_NewDefault(&SeqPermutations_Type);
	if unlikely(!self->ci_combi)
		goto err;
	self->ci_indices = (size_t *)Dee_Callocc(self->ci_combi->c_comlen,
	                                         sizeof(size_t));
	if unlikely(!self->ci_indices)
		goto err_combi;
	self->ci_first = true;
	Dee_atomic_rwlock_init(&self->ci_lock);
	return 0;
err_combi:
	Dee_Decref_likely(self->ci_combi);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
pmutiter_init(CombinationsIterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	size_t i, comlen;
	if (DeeArg_Unpack(argc, argv, "o:_SeqPermutationsIterator", &self->ci_combi))
		goto err;
	if (DeeObject_AssertTypeExact(self->ci_combi, &SeqPermutations_Type))
		goto err;
	Dee_atomic_rwlock_init(&self->ci_lock);
	comlen           = self->ci_combi->c_comlen;
	self->ci_indices = (size_t *)Dee_Mallocc(comlen, sizeof(size_t));
	if unlikely(!self->ci_indices)
		goto err;
	for (i = 0; i < comlen; ++i)
		self->ci_indices[i] = i;
	self->ci_first = true;
	Dee_Incref(self->ci_combi);
	return 0;
err:
	return -1;
}


INTERN DeeTypeObject SeqPermutationsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqPermutationsIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqPermutations)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&pmutiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&comiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&comiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&pmutiter_init,
				TYPE_FIXED_ALLOCATOR(CombinationsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&comiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&comiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &comiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&pmutiter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ pmutiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE WUNUSED NONNULL((1)) DREF CombinationsIterator *DCALL
pmut_iter(Combinations *__restrict self) {
	DREF CombinationsIterator *result;
	size_t i, comlen;
	result = DeeObject_MALLOC(CombinationsIterator);
	if unlikely(!result)
		goto done;
	result->ci_combi = self;
	Dee_atomic_rwlock_init(&result->ci_lock);
	comlen             = self->c_comlen;
	result->ci_indices = (size_t *)Dee_Mallocc(comlen, sizeof(size_t));
	if unlikely(!result->ci_indices)
		goto err_r;
	for (i = 0; i < comlen; ++i)
		result->ci_indices[i] = i;
	result->ci_first = true;
	Dee_Incref(self);
	DeeObject_Init(result, &SeqPermutationsIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE struct type_seq pmut_seq = {
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&pmut_iter
};

PRIVATE struct type_member tpconst pmut_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqPermutationsIterator_Type),
	TYPE_MEMBER_END
};

#define pmut_members com_members

INTERN DeeTypeObject SeqPermutations_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqPermutations",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&com_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&com_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&com_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(Combinations)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&com_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&com_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&com_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &pmut_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ pmut_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ pmut_class_members
};


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Combinations(DeeObject *__restrict self, size_t r) {
	DREF Combinations *result;
	DeeTypeObject *tp_iter;
	DeeTypeMRO mro;
	result = DeeObject_MALLOC(Combinations);
	if unlikely(!result)
		goto done;

	/* Quickly check if we can use the fast-sequence interface. */
	tp_iter = Dee_TYPE(self);
	if (tp_iter == &DeeTuple_Type) {
		if (r >= DeeTuple_SIZE(self)) {
			DeeObject_FREE(result);
			if (r == DeeTuple_SIZE(self))
				return DeeTuple_Pack(1, self);
			return_empty_seq;
		}
		result->c_getitem = NULL;
		result->c_elem    = DeeTuple_ELEM(self);
		result->c_seqlen  = DeeTuple_SIZE(self);
		goto fill_in_result_2;
	}
	result->c_seqlen = DeeFastSeq_GetSize_deprecated(self);
	if (result->c_seqlen != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		result->c_getitem    = tp_iter->tp_seq;
		result->c_elem       = NULL;
		result->c_getitem_tp = tp_iter;
		ASSERT(result->c_getitem);
		goto fill_in_result;
	}
	DeeTypeMRO_Init(&mro, tp_iter);
	do {
		struct type_seq *seq;
		if ((seq = tp_iter->tp_seq) == NULL)
			continue;
		if (seq->tp_getitem) {
			/* Use the getitem/size variant. */
			result->c_getitem    = seq;
			result->c_getitem_tp = tp_iter;
			result->c_elem       = NULL;
			if (!seq->tp_sizeob) {
				for (;;) {
					tp_iter = DeeTypeMRO_Next(&mro, tp_iter);
					if (!tp_iter) {
						err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SIZE);
						goto err_r;
					}
					seq = tp_iter->tp_seq;
					if (seq && seq->tp_sizeob)
						break;
				}
			}
			goto load_tp_size;
		}
		if (seq->tp_sizeob) {
			/* Use the getitem/size variant. */
			for (;;) {
				tp_iter = DeeTypeMRO_Next(&mro, tp_iter);
				if (!tp_iter) {
					err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
					goto err_r;
				}
				if (tp_iter->tp_seq &&
				    tp_iter->tp_seq->tp_getitem)
					break;
			}
			result->c_getitem    = tp_iter->tp_seq;
			result->c_getitem_tp = tp_iter;
load_tp_size:
			result->c_elem = NULL;
			{
				DREF DeeObject *temp;
				int error;
				temp = (*seq->tp_sizeob)(self);
				if unlikely(!temp)
					goto err_r;
				error = DeeObject_AsSize(temp, &result->c_seqlen);
				Dee_Decref(temp);
				if unlikely(error)
					goto err_r;
			}
			goto fill_in_result;
		}
		if (seq->tp_iter) {
			DREF DeeObject *iterator, *elem;
			DREF DeeObject **elem_v, **new_elem_v;
			size_t elem_c, elem_a;
			/* Use the iterator variant */
			iterator = (*seq->tp_iter)(self);
			if unlikely(!iterator)
				goto err_r;
			elem_c = elem_a = 0;
			elem_v          = NULL;
			while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
				ASSERT(elem_c <= elem_a);
				if (elem_c >= elem_a) {
					size_t new_alloc = elem_a * 2;
					if (!new_alloc)
						new_alloc = 8;
					new_elem_v = (DREF DeeObject **)Dee_TryReallocc(elem_v, new_alloc,
					                                                sizeof(DREF DeeObject *));
					if unlikely(!new_elem_v) {
						new_alloc  = elem_c + 1;
						new_elem_v = (DREF DeeObject **)Dee_TryReallocc(elem_v, new_alloc,
						                                                sizeof(DREF DeeObject *));
						if unlikely(!new_elem_v) {
							Dee_Decref(elem);
err_elem_v:
							Dee_Decrefv(elem_v, elem_c);
							Dee_Free(elem_v);
							goto err_r;
						}
					}
					elem_v = new_elem_v;
					elem_a = new_alloc;
				}
				elem_v[elem_c++] = elem; /* Inherit reference. */
				if (DeeThread_CheckInterrupt())
					goto err_elem_v;
			}
			if unlikely(!elem)
				goto err_elem_v;
			Dee_Decref(iterator);
			if (r >= elem_c) {
				Dee_Decrefv(elem_v, elem_c);
				Dee_Free(elem_v);
				DeeObject_FREE(result);
				if (r == elem_c)
					return DeeTuple_Pack(1, self);
				return_empty_seq;
			}
			if likely(elem_a > elem_c) {
				new_elem_v = (DREF DeeObject **)Dee_TryReallocc(elem_v, elem_c,
				                                                sizeof(DREF DeeObject *));
				if likely(new_elem_v)
					elem_v = new_elem_v;
			}
			result->c_getitem = NULL;
			result->c_elem    = elem_v;
			result->c_seqlen  = elem_c;
			goto fill_in_result_2;
		}
	} while ((tp_iter = DeeTypeMRO_Next(&mro, tp_iter)) != NULL);
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
	goto err_r;
fill_in_result:
	if (r >= result->c_seqlen) {
		size_t seqlen = result->c_seqlen;
		ASSERT(!result->c_elem);
		DeeObject_FREE(result);
		if (r == seqlen)
			return DeeTuple_Pack(1, self);
		return_empty_seq;
	}
fill_in_result_2:
	result->c_seq    = self;
	result->c_comlen = r;
	Dee_Incref(self);
	DeeObject_Init(result, &SeqCombinations_Type);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_RepeatCombinations(DeeObject *__restrict self, size_t r) {
	DREF Combinations *result;
	DeeTypeObject *tp_iter;
	DeeTypeMRO mro;
	if (!r)
		return DeeTuple_Pack(1, Dee_EmptySeq);
	result = DeeObject_MALLOC(Combinations);
	if unlikely(!result)
		goto done;

	/* Quickly check if we can use the fast-sequence interface. */
	tp_iter = Dee_TYPE(self);
	if (tp_iter == &DeeTuple_Type) {
		if (!DeeTuple_SIZE(self)) {
			DeeObject_FREE(result);
			return_empty_seq;
		}
		result->c_getitem = NULL;
		result->c_elem    = DeeTuple_ELEM(self);
		result->c_seqlen  = DeeTuple_SIZE(self);
		goto fill_in_result_2;
	}
	result->c_seqlen = DeeFastSeq_GetSize_deprecated(self);
	if (result->c_seqlen != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		result->c_getitem    = tp_iter->tp_seq;
		result->c_elem       = NULL;
		result->c_getitem_tp = tp_iter;
		ASSERT(result->c_getitem);
		goto fill_in_result;
	}
	DeeTypeMRO_Init(&mro, tp_iter);
	do {
		struct type_seq *seq;
		if ((seq = tp_iter->tp_seq) == NULL)
			continue;
		if (seq->tp_getitem) {
			/* Use the getitem/size variant. */
			result->c_getitem    = seq;
			result->c_getitem_tp = tp_iter;
			result->c_elem       = NULL;
			if (!seq->tp_sizeob) {
				for (;;) {
					tp_iter = DeeTypeMRO_Next(&mro, tp_iter);
					if (!tp_iter) {
						err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SIZE);
						goto err_r;
					}
					seq = tp_iter->tp_seq;
					if (seq && seq->tp_sizeob)
						break;
				}
			}
			goto load_tp_size;
		}
		if (seq->tp_sizeob) {
			/* Use the getitem/size variant. */
			tp_iter     = tp_iter;
			for (;;) {
				tp_iter = DeeTypeMRO_Next(&mro, tp_iter);
				if (!tp_iter) {
					err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
					goto err_r;
				}
				if (tp_iter->tp_seq &&
				    tp_iter->tp_seq->tp_getitem)
					break;
			}
			result->c_getitem    = tp_iter->tp_seq;
			result->c_getitem_tp = tp_iter;
load_tp_size:
			result->c_elem = NULL;
			{
				DREF DeeObject *temp;
				int error;
				temp = (*seq->tp_sizeob)(self);
				if unlikely(!temp)
					goto err_r;
				error = DeeObject_AsSize(temp, &result->c_seqlen);
				Dee_Decref(temp);
				if unlikely(error)
					goto err_r;
			}
			goto fill_in_result;
		}
		if (seq->tp_iter) {
			DREF DeeObject *iterator, *elem;
			DREF DeeObject **elem_v, **new_elem_v;
			size_t elem_c, elem_a;
			/* Use the iterator variant */
			iterator = (*seq->tp_iter)(self);
			if unlikely(!iterator)
				goto err_r;
			elem_c = elem_a = 0;
			elem_v          = NULL;
			while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
				ASSERT(elem_c <= elem_a);
				if (elem_c >= elem_a) {
					size_t new_alloc = elem_a * 2;
					if (!new_alloc)
						new_alloc = 8;
					new_elem_v = (DREF DeeObject **)Dee_TryReallocc(elem_v, new_alloc,
					                                                sizeof(DREF DeeObject *));
					if unlikely(!new_elem_v) {
						new_alloc  = elem_c + 1;
						new_elem_v = (DREF DeeObject **)Dee_TryReallocc(elem_v, new_alloc,
						                                                sizeof(DREF DeeObject *));
						if unlikely(!new_elem_v) {
							Dee_Decref(elem);
err_elem_v:
							Dee_Decrefv(elem_v, elem_c);
							Dee_Free(elem_v);
							goto err_r;
						}
					}
					elem_v = new_elem_v;
					elem_a = new_alloc;
				}
				elem_v[elem_c++] = elem; /* Inherit reference. */
				if (DeeThread_CheckInterrupt())
					goto err_elem_v;
			}
			if unlikely(!elem)
				goto err_elem_v;
			Dee_Decref(iterator);
			if (!elem_c) {
				Dee_Free(elem_v);
				DeeObject_FREE(result);
				return_empty_seq;
			}
			if likely(elem_a > elem_c) {
				new_elem_v = (DREF DeeObject **)Dee_TryReallocc(elem_v, elem_c,
				                                                sizeof(DREF DeeObject *));
				if likely(new_elem_v)
					elem_v = new_elem_v;
			}
			result->c_getitem = NULL;
			result->c_elem    = elem_v;
			result->c_seqlen  = elem_c;
			goto fill_in_result_2;
		}
	} while ((tp_iter = DeeTypeMRO_Next(&mro, tp_iter)) != NULL);
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
	goto err_r;
fill_in_result:
	if (!result->c_seqlen) {
		ASSERT(!result->c_elem);
		DeeObject_FREE(result);
		return_empty_seq;
	}
fill_in_result_2:
	result->c_seq    = self;
	result->c_comlen = r;
	Dee_Incref(self);
	DeeObject_Init(result, &SeqRepeatCombinations_Type);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Permutations(DeeObject *__restrict self) {
	DREF Combinations *result;
	DeeTypeObject *tp_iter;
	DeeTypeMRO mro;
	result = DeeObject_MALLOC(Combinations);
	if unlikely(!result)
		goto done;

	/* Quickly check if we can use the fast-sequence interface. */
	tp_iter = Dee_TYPE(self);
	if (tp_iter == &DeeTuple_Type) {
		if (!DeeTuple_SIZE(self)) {
			DeeObject_FREE(result);
			return DeeTuple_Pack(1, Dee_EmptySeq);
		}
		result->c_getitem = NULL;
		result->c_elem    = DeeTuple_ELEM(self);
		result->c_seqlen  = DeeTuple_SIZE(self);
		goto fill_in_result_2;
	}
	result->c_seqlen = DeeFastSeq_GetSize_deprecated(self);
	if (result->c_seqlen != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		result->c_getitem    = tp_iter->tp_seq;
		result->c_elem       = NULL;
		result->c_getitem_tp = tp_iter;
		ASSERT(result->c_getitem);
		goto fill_in_result;
	}
	DeeTypeMRO_Init(&mro, tp_iter);
	do {
		struct type_seq *seq;
		if ((seq = tp_iter->tp_seq) == NULL)
			continue;
		if (seq->tp_getitem) {
			/* Use the getitem/size variant. */
			result->c_getitem    = seq;
			result->c_getitem_tp = tp_iter;
			result->c_elem       = NULL;
			if (!seq->tp_sizeob) {
				for (;;) {
					tp_iter = DeeTypeMRO_Next(&mro, tp_iter);
					if (!tp_iter) {
						err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SIZE);
						goto err_r;
					}
					seq = tp_iter->tp_seq;
					if (seq && seq->tp_sizeob)
						break;
				}
			}
			goto load_tp_size;
		}
		if (seq->tp_sizeob) {
			/* Use the getitem/size variant. */
			for (;;) {
				tp_iter = DeeTypeMRO_Next(&mro, tp_iter);
				if (!tp_iter) {
					err_unimplemented_operator(Dee_TYPE(self), OPERATOR_GETITEM);
					goto err_r;
				}
				if (tp_iter->tp_seq &&
				    tp_iter->tp_seq->tp_getitem)
					break;
			}
			result->c_getitem    = tp_iter->tp_seq;
			result->c_getitem_tp = tp_iter;
load_tp_size:
			result->c_elem = NULL;
			{
				DREF DeeObject *temp;
				int error;
				temp = (*seq->tp_sizeob)(self);
				if unlikely(!temp)
					goto err_r;
				error = DeeObject_AsSize(temp, &result->c_seqlen);
				Dee_Decref(temp);
				if unlikely(error)
					goto err_r;
			}
			goto fill_in_result;
		}
		if (seq->tp_iter) {
			DREF DeeObject *iterator, *elem;
			DREF DeeObject **elem_v, **new_elem_v;
			size_t elem_c, elem_a;
			/* Use the iterator variant */
			iterator = (*seq->tp_iter)(self);
			if unlikely(!iterator)
				goto err_r;
			elem_a = 0;
			elem_c = 0;
			elem_v = NULL;
			while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
				ASSERT(elem_c <= elem_a);
				if (elem_c >= elem_a) {
					size_t new_alloc = elem_a * 2;
					if (!new_alloc)
						new_alloc = 8;
					new_elem_v = (DREF DeeObject **)Dee_TryReallocc(elem_v, new_alloc,
					                                                sizeof(DREF DeeObject *));
					if unlikely(!new_elem_v) {
						new_alloc  = elem_c + 1;
						new_elem_v = (DREF DeeObject **)Dee_TryReallocc(elem_v, new_alloc,
						                                                sizeof(DREF DeeObject *));
						if unlikely(!new_elem_v) {
							Dee_Decref(elem);
err_elem_v:
							Dee_Decrefv(elem_v, elem_c);
							Dee_Free(elem_v);
							goto err_r;
						}
					}
					elem_v = new_elem_v;
					elem_a = new_alloc;
				}
				elem_v[elem_c++] = elem; /* Inherit reference. */
				if (DeeThread_CheckInterrupt())
					goto err_elem_v;
			}
			if unlikely(!elem)
				goto err_elem_v;
			Dee_Decref(iterator);
			if (!elem_c) {
				Dee_Free(elem_v);
				DeeObject_FREE(result);
				return DeeTuple_Pack(1, Dee_EmptySeq);
			}
			if likely(elem_a > elem_c) {
				new_elem_v = (DREF DeeObject **)Dee_TryReallocc(elem_v, elem_c,
				                                                sizeof(DREF DeeObject *));
				if likely(new_elem_v)
					elem_v = new_elem_v;
			}
			result->c_getitem = NULL;
			result->c_elem    = elem_v;
			result->c_seqlen  = elem_c;
			goto fill_in_result_2;
		}
	} while ((tp_iter = DeeTypeMRO_Next(&mro, tp_iter)) != NULL);
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
	goto err_r;
fill_in_result:
	if (!result->c_seqlen) {
		ASSERT(!result->c_elem);
		DeeObject_FREE(result);
		return DeeTuple_Pack(1, Dee_EmptySeq);
	}
fill_in_result_2:
	result->c_seq    = self;
	result->c_comlen = result->c_seqlen;
	Dee_Incref(self);
	DeeObject_Init(result, &SeqPermutations_Type);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Permutations2(DeeObject *__restrict self, size_t r) {
	DREF Combinations *result;
	if (!r)
		return DeeTuple_Pack(1, Dee_EmptySeq);
	result = (DREF Combinations *)DeeSeq_Permutations(self);
	if likely(result && Dee_TYPE(result) == &SeqPermutations_Type) {
		result->c_comlen = r;
		if (r > result->c_seqlen) {
			Dee_Decref_likely(result);
			result = (DREF Combinations *)Dee_EmptySeq;
			Dee_Incref(Dee_EmptySeq);
		}
	}
	return (DREF DeeObject *)result;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_COMBINATIONS_C */
