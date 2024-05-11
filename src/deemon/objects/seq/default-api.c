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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_C 1

#include "default-api.h"

#include <deemon/api.h>
#include <deemon/bytes.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>

/**/
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

#ifndef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
#include "../seq_functions.h"
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */

DECL_BEGIN

#define DeeType_RequireBool(tp_self)           (((tp_self)->tp_cast.tp_bool) || DeeType_InheritBool(tp_self))
#define DeeType_RequireSize(tp_self)           (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_size && (tp_self)->tp_seq->tp_sizeob) || DeeType_InheritSize(tp_self))
#define DeeType_RequireIter(tp_self)           (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_iter) || DeeType_InheritIter(tp_self))
#define DeeType_RequireForeach(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_foreach) || DeeType_InheritIter(tp_self))
#define DeeType_RequireEnumerateIndex(tp_self) (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_enumerate_index) || (DeeType_InheritIter(tp_self) && (tp_self)->tp_seq->tp_enumerate_index))
#define DeeType_RequireGetItem(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_getitem && (tp_self)->tp_seq->tp_getitem_index) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireBoundItem(tp_self)      (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_bounditem && (tp_self)->tp_seq->tp_bounditem_index) || DeeType_InheritGetItem(tp_self))
#define DeeType_RequireDelItem(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_delitem && (tp_self)->tp_seq->tp_delitem_index) || DeeType_InheritDelItem(tp_self))
#define DeeType_RequireSetItem(tp_self)        (((tp_self)->tp_seq && (tp_self)->tp_seq->tp_setitem && (tp_self)->tp_seq->tp_setitem_index) || DeeType_InheritSetItem(tp_self))

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_size(struct type_seq const *__restrict self) {
	return (self->tp_size != NULL) &&
	       !DeeType_IsDefaultSize(self->tp_size);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_sizeob(struct type_seq const *__restrict self) {
	return (self->tp_sizeob != NULL) &&
	       !DeeType_IsDefaultSizeOb(self->tp_sizeob);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_foreach(struct type_seq const *__restrict self) {
	return (self->tp_foreach != NULL) &&
	       !DeeType_IsDefaultForeach(self->tp_foreach);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_getitem_index(struct type_seq const *__restrict self) {
	return (self->tp_getitem_index != NULL) &&
	       !DeeType_IsDefaultGetItemIndex(self->tp_getitem_index);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_bounditem_index(struct type_seq const *__restrict self) {
	return (self->tp_bounditem_index != NULL) &&
	       !DeeType_IsDefaultBoundItemIndex(self->tp_bounditem_index);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_delitem_index(struct type_seq const *__restrict self) {
	return (self->tp_delitem_index != NULL) &&
	       !DeeType_IsDefaultDelItemIndex(self->tp_delitem_index);
}
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_type_seq_has_custom_tp_setitem_index(struct type_seq const *__restrict self) {
	return (self->tp_setitem_index != NULL) &&
	       !DeeType_IsDefaultSetItemIndex(self->tp_setitem_index);
}

PRIVATE WUNUSED NONNULL((1)) struct Dee_type_seq_cache *DCALL
DeeType_TryRequireSeqCache(DeeTypeObject *__restrict self) {
	struct Dee_type_seq_cache *sc;
	ASSERT(self->tp_seq);
	sc = self->tp_seq->_tp_seqcache;
	if (sc)
		return sc;
	sc = (struct Dee_type_seq_cache *)Dee_TryCalloc(sizeof(struct Dee_type_seq_cache));
	if likely(sc) {
		if unlikely(!atomic_cmpxch(&self->tp_seq->_tp_seqcache, NULL, sc)) {
			Dee_Free(sc);
			return self->tp_seq->_tp_seqcache;
		}

		/* Don't treat "sc" as a memory leak if it isn't freed. */
		sc = (struct Dee_type_seq_cache *)Dee_UntrackAlloc(sc);
	}
	return sc;
}




INTERN WUNUSED NONNULL((1)) Dee_tsc_foreach_reverse_t DCALL
DeeType_SeqCache_TryRequireForeachReverse(DeeTypeObject *__restrict self) {
	struct Dee_type_seq *seq = self->tp_seq;
	if likely(seq) {
		struct Dee_type_seq_cache *sc;
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_foreach_reverse)
			return sc->tsc_foreach_reverse;
	}
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		Dee_tsc_foreach_reverse_t result = NULL;
		if (DeeType_RequireSize(self) && DeeType_RequireGetItem(self)) {
			bool has_size = !DeeType_IsDefaultSize(self->tp_seq->tp_size);
			if (has_size && self->tp_seq->tp_getitem_index_fast) {
				result = &DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndexFast;
			} else if (has_size && !DeeType_IsDefaultGetItemIndex(self->tp_seq->tp_getitem_index)) {
				result = &DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndex;
			} else if (has_size && !DeeType_IsDefaultTryGetItemIndex(self->tp_seq->tp_trygetitem_index)) {
				result = &DeeSeq_DefaultForeachReverseWithSizeAndTryGetItemIndex;
			} else if (!DeeType_IsDefaultSizeOb(self->tp_seq->tp_sizeob) &&
			           !DeeType_IsDefaultGetItem(self->tp_seq->tp_getitem)) {
				result = &DeeSeq_DefaultForeachReverseWithSizeObAndGetItem;
			}
		}
		if (result) {
			struct Dee_type_seq_cache *sc;
			sc = DeeType_TryRequireSeqCache(self);
			if likely(sc)
				atomic_write(&sc->tsc_foreach_reverse, result);
		}
		return result;
	}
	return NULL; /* Not possible... */
}

INTERN WUNUSED NONNULL((1)) Dee_tsc_enumerate_index_reverse_t DCALL
DeeType_SeqCache_TryRequireEnumerateIndexReverse(DeeTypeObject *__restrict self) {
	struct Dee_type_seq *seq = self->tp_seq;
	if likely(seq) {
		struct Dee_type_seq_cache *sc;
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_enumerate_index_reverse)
			return sc->tsc_enumerate_index_reverse;
	}
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		Dee_tsc_enumerate_index_reverse_t result = NULL;
		if (DeeType_RequireSize(self) && DeeType_RequireGetItem(self)) {
			bool has_size = !DeeType_IsDefaultSize(self->tp_seq->tp_size);
			if (has_size && self->tp_seq->tp_getitem_index_fast) {
				result = &DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndexFast;
			} else if (has_size && !DeeType_IsDefaultGetItemIndex(self->tp_seq->tp_getitem_index)) {
				result = &DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndex;
			} else if (has_size && !DeeType_IsDefaultTryGetItemIndex(self->tp_seq->tp_trygetitem_index)) {
				result = &DeeSeq_DefaultEnumerateIndexReverseWithSizeAndTryGetItemIndex;
			} else if (!DeeType_IsDefaultSizeOb(self->tp_seq->tp_sizeob) &&
			           !DeeType_IsDefaultGetItem(self->tp_seq->tp_getitem)) {
				result = &DeeSeq_DefaultEnumerateIndexReverseWithSizeObAndGetItem;
			}
		}
		if (result) {
			struct Dee_type_seq_cache *sc;
			sc = DeeType_TryRequireSeqCache(self);
			if likely(sc)
				atomic_write(&sc->tsc_enumerate_index_reverse, result);
		}
		return result;
	}
	return NULL; /* Not possible... */
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultEnumerateIndexWithError(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                      void *arg, size_t start, size_t end) {
	(void)self;
	(void)proc;
	(void)arg;
	(void)start;
	(void)end;
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_enumerate_index_t DCALL
DeeType_SeqCache_RequireEnumerateIndex(DeeTypeObject *__restrict self) {
	Dee_tsc_enumerate_index_t result;
	struct Dee_type_seq_cache *sc;
	if likely(self->tp_seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_enumerate_index)
			return sc->tsc_enumerate_index;
	}
	if (DeeType_RequireEnumerateIndex(self)) {
		result = self->tp_seq->tp_enumerate_index;
	} else if (DeeType_RequireForeach(self) && !DeeType_IsDefaultForeach(self->tp_seq->tp_foreach)) {
		result = &DeeSeq_DefaultEnumerateIndexWithCounterAndForeach;
	} else if (DeeType_RequireIter(self) && !DeeType_IsDefaultIter(self->tp_seq->tp_iter)) {
		result = &DeeSeq_DefaultEnumerateIndexWithCounterAndIter;
	} else if (self->tp_seq && self->tp_seq->tp_foreach) {
		result = &DeeSeq_DefaultEnumerateIndexWithCounterAndForeachDefault;
	} else {
		result = &DeeSeq_DefaultEnumerateIndexWithError;
	}
	sc = DeeType_TryRequireSeqCache(self);
	if likely(sc)
		atomic_write(&sc->tsc_enumerate_index, result);
	return result;
}

INTDEF WUNUSED NONNULL((1, 2)) int DCALL generic_seq_bool(DeeObject *self);

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_nonempty_t DCALL
DeeType_SeqCache_RequireNonEmpty(DeeTypeObject *__restrict self) {
	Dee_tsc_nonempty_t result;
	struct Dee_type_seq_cache *sc;
	if likely(self->tp_seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->tsc_nonempty)
			return sc->tsc_nonempty;
	}
	result = &DeeSeq_DefaultNonEmptyWithError;
	if (DeeType_GetSeqClass(self) != Dee_SEQCLASS_NONE) {
		if (DeeType_RequireBool(self) && self->tp_cast.tp_bool != &generic_seq_bool) {
			result = self->tp_cast.tp_bool;
		} else if (Dee_type_seq_has_custom_tp_size(self->tp_seq)) {
			result = &DeeSeq_DefaultBoolWithSize;
		} else if (Dee_type_seq_has_custom_tp_sizeob(self->tp_seq)) {
			result = &DeeSeq_DefaultBoolWithSizeOb;
		} else if (Dee_type_seq_has_custom_tp_foreach(self->tp_seq)) {
			result = &DeeSeq_DefaultBoolWithForeach;
		} else if (self->tp_cmp && self->tp_cmp->tp_compare_eq &&
		           !DeeType_IsDefaultCompareEq(self->tp_cmp->tp_compare_eq) &&
		           !DeeType_IsDefaultCompare(self->tp_cmp->tp_compare_eq)) {
			result = &DeeSeq_DefaultBoolWithCompareEq;
		} else if (self->tp_cmp && self->tp_cmp->tp_eq && !DeeType_IsDefaultEq(self->tp_cmp->tp_eq)) {
			result = &DeeSeq_DefaultBoolWithEq;
		} else if (self->tp_cmp && self->tp_cmp->tp_ne && !DeeType_IsDefaultNe(self->tp_cmp->tp_ne)) {
			result = &DeeSeq_DefaultBoolWithNe;
		} else if (self->tp_seq->tp_foreach || DeeType_InheritIter(self)) {
			result = &DeeSeq_DefaultBoolWithForeachDefault;
		}
	} else if (DeeType_RequireForeach(self)) {
		result = &DeeSeq_DefaultBoolWithForeach;
	}
	sc = DeeType_TryRequireSeqCache(self);
	if likely(sc)
		atomic_write(&sc->tsc_nonempty, result);
	return result;
}






/* Possible implementations for sequence cache functions. */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndexFast(DeeObject *__restrict self,
                                                        Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	while (size) {
		DREF DeeObject *elem;
		--size;
		elem = (*seq->tp_getitem_index_fast)(self, size);
		if likely(elem) {
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndex(DeeObject *__restrict self,
                                                    Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	while (size) {
		DREF DeeObject *elem;
		--size;
		elem = (*seq->tp_getitem_index)(self, size);
		if likely(elem) {
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		} else {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultForeachReverseWithSizeAndTryGetItemIndex(DeeObject *__restrict self,
                                                       Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	while (size) {
		DREF DeeObject *elem;
		--size;
		elem = (*seq->tp_trygetitem_index)(self, size);
		if unlikely(!elem)
			goto err;
		if likely(elem != ITER_DONE) {
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultForeachReverseWithSizeObAndGetItem(DeeObject *__restrict self,
                                                 Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	DREF DeeObject *sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	for (;;) {
		DREF DeeObject *elem;
		int size_is_nonzero = DeeObject_Bool(sizeob);
		if unlikely(size_is_nonzero < 0)
			goto err_sizeob;
		if (!size_is_nonzero)
			break;
		if (DeeObject_Dec(&sizeob))
			goto err_sizeob;
		elem = (*seq->tp_getitem)(self, sizeob);
		if likely(elem) {
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp_sizeob;
			result += temp;
		} else {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob;
		}
		if (DeeThread_CheckInterrupt())
			goto err_sizeob;
	}
	Dee_Decref(sizeob);
	return result;
err_temp_sizeob:
	Dee_Decref(sizeob);
/*err_temp:*/
	return temp;
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                                               void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	while (size > start) {
		DREF DeeObject *elem;
		--size;
		elem = (*seq->tp_getitem_index_fast)(self, size);
		temp = (*proc)(arg, size, elem);
		Dee_XDecref(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndex(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                                           void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	while (size > start) {
		DREF DeeObject *elem;
		--size;
		elem = (*seq->tp_getitem_index)(self, size);
		if unlikely(!elem) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err;
		}
		temp = (*proc)(arg, size, elem);
		Dee_XDecref(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultEnumerateIndexReverseWithSizeAndTryGetItemIndex(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                                              void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if (size > end)
		size = end;
	while (size > start) {
		DREF DeeObject *elem;
		--size;
		elem = (*seq->tp_trygetitem_index)(self, size);
		if unlikely(!elem)
			goto err;
		if likely(elem != ITER_DONE) {
			temp = (*proc)(arg, size, elem);
			Dee_Decref(elem);
		} else {
			temp = (*proc)(arg, size, NULL);
		}
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_DefaultEnumerateIndexReverseWithSizeObAndGetItem(DeeObject *__restrict self, Dee_enumerate_index_t proc,
                                                        void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	struct type_seq *seq = Dee_TYPE(self)->tp_seq;
	DREF DeeObject *startob = NULL;
	DREF DeeObject *sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	if (end != (size_t)-1) {
		int error;
		DREF DeeObject *wanted_end;
		wanted_end = DeeInt_NewSize(end);
		if unlikely(!wanted_end)
			goto err_sizeob;
		/* if (sizeob > wanted_end)
		 *     sizeob = wanted_end; */
		error = DeeObject_CmpGrAsBool(sizeob, wanted_end);
		if unlikely(error == 0) {
			Dee_Decref(wanted_end);
		} else {
			Dee_Decref(sizeob);
			sizeob = wanted_end;
			if unlikely(error < 0)
				goto err_sizeob;
		}
	}
	if (start != 0) {
		startob = DeeInt_NewSize(start);
		if unlikely(!startob)
			goto err_sizeob;
	}
	for (;;) {
		size_t index_value;
		DREF DeeObject *elem;
		int size_is_greater_start;
		if (startob) {
			size_is_greater_start = DeeObject_CmpGrAsBool(sizeob, startob);
		} else {
			size_is_greater_start = DeeObject_Bool(sizeob);
		}
		if unlikely(size_is_greater_start < 0)
			goto err_sizeob_startob;
		if (!size_is_greater_start)
			break;
		if (DeeObject_Dec(&sizeob))
			goto err_sizeob_startob;
		elem = (*seq->tp_getitem)(self, sizeob);
		if unlikely(!elem) {
			if (!DeeError_Catch(&DeeError_IndexError) &&
			    !DeeError_Catch(&DeeError_UnboundItem))
				goto err_sizeob_startob;
		}
		if unlikely(DeeObject_AsSize(sizeob, &index_value))
			goto err_sizeob_startob;
		temp = 0;
		if likely(index_value >= start && index_value < end)
			temp = (*proc)(arg, index_value, elem);
		Dee_XDecref(elem);
		if unlikely(temp < 0)
			goto err_temp_sizeob_startob;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_sizeob_startob;
	}
	Dee_XDecref(startob);
	Dee_Decref(sizeob);
	return result;
err_temp_sizeob_startob:
	Dee_XDecref(startob);
/*err_temp_sizeob:*/
	Dee_Decref(sizeob);
/*err_temp:*/
	return temp;
err_sizeob_startob:
	Dee_XDecref(startob);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}



INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetFirstWithGetAttr(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_first);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetFirstWithGetItem(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, DeeInt_Zero);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetFirstWithGetItemIndex(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_getitem_index)(self, 0);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_default_getfirst_with_foreach_cb(void *arg, DeeObject *elem) {
	Dee_Incref(elem);
	*(DREF DeeObject **)arg = elem;
	return -2;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetFirstWithForeachDefault(DeeObject *__restrict self) {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &seq_default_getfirst_with_foreach_cb, &result);
	if likely(foreach_status == -2)
		return result;
	if (foreach_status == 0)
		err_empty_sequence(self);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundFirstWithBoundAttr(DeeObject *__restrict self) {
	return DeeObject_BoundAttr(self, (DeeObject *)&str_first);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundFirstWithBoundItem(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_bounditem)(self, DeeInt_Zero);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundFirstWithBoundItemIndex(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_bounditem_index)(self, 0);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_default_boundfirst_with_foreach_cb(void *arg, DeeObject *elem) {
	(void)arg;
	(void)elem;
	return -2;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundFirstWithForeachDefault(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &seq_default_boundfirst_with_foreach_cb, NULL);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if likely(foreach_status == -2)
		return 1;
	if unlikely(foreach_status == -1)
		return -1;
	return -2;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelFirstWithDelAttr(DeeObject *__restrict self) {
	return DeeObject_DelAttr(self, (DeeObject *)&str_first);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelFirstWithDelItem(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_delitem)(self, DeeInt_Zero);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelFirstWithDelItemIndex(DeeObject *__restrict self) {
	return (*Dee_TYPE(self)->tp_seq->tp_delitem_index)(self, 0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelFirstWithError(DeeObject *__restrict self) {
	return err_cant_access_attribute(Dee_TYPE(self), &str_first, ATTR_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetFirstWithSetAttr(DeeObject *self, DeeObject *value) {
	return DeeObject_SetAttr(self, (DeeObject *)&str_first, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetFirstWithSetItem(DeeObject *self, DeeObject *value) {
	return (*Dee_TYPE(self)->tp_seq->tp_setitem)(self, DeeInt_Zero, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetFirstWithSetItemIndex(DeeObject *self, DeeObject *value) {
	return (*Dee_TYPE(self)->tp_seq->tp_setitem_index)(self, 0, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetFirstWithError(DeeObject *self, DeeObject *value) {
	(void)value;
	return err_cant_access_attribute(Dee_TYPE(self), &str_first, ATTR_ACCESS_SET);
}




INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithGetAttr(DeeObject *__restrict self) {
	return DeeObject_GetAttr(self, (DeeObject *)&str_last);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithSizeAndGetItemIndexFast(DeeObject *__restrict self) {
	DREF DeeObject *result;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	result = (*seq->tp_getitem_index_fast)(self, size);
	if unlikely(!result)
		err_unbound_index(self, size);
	return result;
err_isempty:
	err_empty_sequence(self);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithSizeAndGetItemIndex(DeeObject *__restrict self) {
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	return (*seq->tp_getitem_index)(self, size);
err_isempty:
	err_empty_sequence(self);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithSizeObAndGetItem(DeeObject *__restrict self) {
	int temp;
	DREF DeeObject *result, *sizeob;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	temp = DeeObject_Bool(sizeob);
	if unlikely(temp < 0)
		goto err_sizeob;
	if unlikely(!temp)
		goto err_sizeob_isempty;
	if unlikely(DeeObject_Dec(&sizeob))
		goto err_sizeob;
	result = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, sizeob);
	Dee_Decref(sizeob);
	return result;
err_sizeob_isempty:
	err_empty_sequence(self);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
seq_default_last_with_foreach_cb(void *arg, DeeObject *elem) {
	Dee_Incref(elem);
	Dee_Decref_unlikely(*(DREF DeeObject **)arg);
	*(DREF DeeObject **)arg = elem;
	return 1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_DefaultGetLastWithForeachDefault(DeeObject *__restrict self) {
	DREF DeeObject *result;
	Dee_ssize_t foreach_status;
	Dee_Incref(Dee_None);
	result = Dee_None;
	foreach_status = DeeObject_Foreach(self, &seq_default_last_with_foreach_cb, &result);
	if likely(foreach_status > 0)
		return result;
	Dee_Decref_unlikely(result);
	if (foreach_status == 0)
		err_empty_sequence(self);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundLastWithSizeAndGetItemIndexFast(DeeObject *__restrict self) {
	DREF DeeObject *result;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	result = (*seq->tp_getitem_index_fast)(self, size);
	if unlikely(!result)
		return 0;
	Dee_Decref(result);
	return 1;
err_isempty:
	return -2;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundLastWithSizeAndBoundItemIndex(DeeObject *__restrict self) {
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	return (*seq->tp_bounditem_index)(self, size);
err_isempty:
	err_empty_sequence(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundLastWithBoundAttr(DeeObject *__restrict self) {
	return DeeObject_BoundAttr(self, (DeeObject *)&str_last);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultBoundLastWithSizeObAndBoundItem(DeeObject *__restrict self) {
	int temp, result;
	DREF DeeObject *sizeob;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	temp = DeeObject_Bool(sizeob);
	if unlikely(temp < 0)
		goto err_sizeob;
	if unlikely(!temp)
		goto err_sizeob_isempty;
	if unlikely(DeeObject_Dec(&sizeob))
		goto err_sizeob;
	result = (*Dee_TYPE(self)->tp_seq->tp_bounditem)(self, sizeob);
	Dee_Decref(sizeob);
	return result;
err_sizeob_isempty:
	err_empty_sequence(self);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelLastWithDelAttr(DeeObject *__restrict self) {
	return DeeObject_DelAttr(self, (DeeObject *)&str_last);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelLastWithSizeAndDelItemIndex(DeeObject *__restrict self) {
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	return (*seq->tp_delitem_index)(self, size);
err_isempty:
	err_empty_sequence(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelLastWithSizeObAndDelItem(DeeObject *__restrict self) {
	int temp, result;
	DREF DeeObject *sizeob;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	temp = DeeObject_Bool(sizeob);
	if unlikely(temp < 0)
		goto err_sizeob;
	if unlikely(!temp)
		goto err_sizeob_isempty;
	if unlikely(DeeObject_Dec(&sizeob))
		goto err_sizeob;
	result = (*Dee_TYPE(self)->tp_seq->tp_delitem)(self, sizeob);
	Dee_Decref(sizeob);
	return result;
err_sizeob_isempty:
	err_empty_sequence(self);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultDelLastWithError(DeeObject *__restrict self) {
	return err_cant_access_attribute(Dee_TYPE(self), &str_last, ATTR_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetLastWithSetAttr(DeeObject *self, DeeObject *value) {
	return DeeObject_SetAttr(self, (DeeObject *)&str_last, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetLastWithSizeAndSetItemIndex(DeeObject *self, DeeObject *value) {
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	size_t size = (*seq->tp_size)(self);
	if unlikely(size == (size_t)-1)
		goto err;
	if unlikely(size == 0)
		goto err_isempty;
	--size;
	return (*seq->tp_setitem_index)(self, size, value);
err_isempty:
	err_empty_sequence(self);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetLastWithSizeObAndSetItem(DeeObject *self, DeeObject *value) {
	int temp, result;
	DREF DeeObject *sizeob;
	struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
	sizeob = (*seq->tp_sizeob)(self);
	if unlikely(!sizeob)
		goto err;
	temp = DeeObject_Bool(sizeob);
	if unlikely(temp < 0)
		goto err_sizeob;
	if unlikely(!temp)
		goto err_sizeob_isempty;
	if unlikely(DeeObject_Dec(&sizeob))
		goto err_sizeob;
	result = (*Dee_TYPE(self)->tp_seq->tp_setitem)(self, sizeob, value);
	Dee_Decref(sizeob);
	return result;
err_sizeob_isempty:
	err_empty_sequence(self);
err_sizeob:
	Dee_Decref(sizeob);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_DefaultSetLastWithError(DeeObject *self, DeeObject *value) {
	(void)value;
	return err_cant_access_attribute(Dee_TYPE(self), &str_last, ATTR_ACCESS_SET);
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_DefaultNonEmptyWithError(DeeObject *__restrict self) {
	return err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
}








/* Generic sequence function hooks (used as function pointers of `type_method' / `type_getset' of Sequence/Set/Mapping) */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_getfirst(DeeObject *__restrict self) {
	return DeeSeq_GetFirst(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_seq_boundfirst(DeeObject *__restrict self) {
	return DeeSeq_BoundFirst(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_seq_delfirst(DeeObject *__restrict self) {
	return DeeSeq_DelFirst(self);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_seq_setfirst(DeeObject *self, DeeObject *value) {
	return DeeSeq_SetFirst(self, value);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_getlast(DeeObject *__restrict self) {
	return DeeSeq_GetLast(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_seq_boundlast(DeeObject *__restrict self) {
	return DeeSeq_BoundLast(self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_seq_dellast(DeeObject *__restrict self) {
	return DeeSeq_DelLast(self);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_seq_setlast(DeeObject *self, DeeObject *value) {
	return DeeSeq_SetLast(self, value);
}









/* Uncached default sequence operations. */

struct generic_seq_sum_data {
#define GENERIC_SEQ_SUM_MODE_FIRST  0 /* Mode not yet determined (still at first item) */
#define GENERIC_SEQ_SUM_MODE_OBJECT 1 /* Generic object sum mode (using "operator +") */
#define GENERIC_SEQ_SUM_MODE_STRING 2 /* Use a unicode printer */
#define GENERIC_SEQ_SUM_MODE_BYTES  3 /* Use a bytes printer */
	uintptr_t gss_mode; /* Sum-mode (one of `GENERIC_SEQ_SUM_MODE_*') */
	union {
		DREF DeeObject        *v_object; /* GENERIC_SEQ_SUM_MODE_OBJECT */
		struct unicode_printer v_string; /* GENERIC_SEQ_SUM_MODE_STRING */
		struct bytes_printer   v_bytes;  /* GENERIC_SEQ_SUM_MODE_BYTES */
	} gss_value;
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
generic_seq_sum_cb(void *arg, DeeObject *elem) {
	struct generic_seq_sum_data *data;
	data = (struct generic_seq_sum_data *)arg;
	switch (data->gss_mode) {

	case GENERIC_SEQ_SUM_MODE_FIRST: {
		DeeTypeObject *tp_elem = Dee_TYPE(elem);
		if (tp_elem == &DeeString_Type) {
			data->gss_mode = GENERIC_SEQ_SUM_MODE_STRING;
			unicode_printer_init(&data->gss_value.v_string);
			goto do_print_string;
		} else if (tp_elem == &DeeBytes_Type) {
			data->gss_mode = GENERIC_SEQ_SUM_MODE_BYTES;
			bytes_printer_init(&data->gss_value.v_bytes);
			goto do_print_bytes;
		} else {
			data->gss_mode = GENERIC_SEQ_SUM_MODE_OBJECT;
			data->gss_value.v_object = elem;
			Dee_Incref(elem);
		}
	}	break;

	case GENERIC_SEQ_SUM_MODE_OBJECT: {
		DREF DeeObject *result;
		result = DeeObject_Add(data->gss_value.v_object, elem);
		if unlikely(!result)
			goto err;
		Dee_Decref(data->gss_value.v_object);
		data->gss_value.v_object = result;
	}	break;

	case GENERIC_SEQ_SUM_MODE_STRING:
do_print_string:
		return unicode_printer_printobject(&data->gss_value.v_string, elem);

	case GENERIC_SEQ_SUM_MODE_BYTES:
do_print_bytes:
		return bytes_printer_printobject(&data->gss_value.v_bytes, elem);

	default: __builtin_unreachable();
	}
	return 0;
err:
	return -1;
}

/* Functions used to implement special sequence expressions,
 * such as `x + ...' (as `DeeSeq_Sum'), etc. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Sum(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	struct generic_seq_sum_data data;
	data.gss_mode = GENERIC_SEQ_SUM_MODE_FIRST;
	foreach_status = DeeObject_Foreach(self, &generic_seq_sum_cb, &data);
	if unlikely(foreach_status < 0)
		goto err;
	switch (data.gss_mode) {
	case GENERIC_SEQ_SUM_MODE_FIRST:
		return_none;
	case GENERIC_SEQ_SUM_MODE_OBJECT:
		return data.gss_value.v_object;
	case GENERIC_SEQ_SUM_MODE_STRING:
		return unicode_printer_pack(&data.gss_value.v_string);
	case GENERIC_SEQ_SUM_MODE_BYTES:
		return bytes_printer_pack(&data.gss_value.v_bytes);
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
err:
	switch (data.gss_mode) {
	case GENERIC_SEQ_SUM_MODE_FIRST:
		break;
	case GENERIC_SEQ_SUM_MODE_OBJECT:
		Dee_Decref(data.gss_value.v_object);
		break;
	case GENERIC_SEQ_SUM_MODE_STRING:
		unicode_printer_fini(&data.gss_value.v_string);
		break;
	case GENERIC_SEQ_SUM_MODE_BYTES:
		bytes_printer_fini(&data.gss_value.v_bytes);
		break;
	default: __builtin_unreachable();
	}
	return NULL;
}


PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
generic_seq_any_cb(void *arg, DeeObject *elem) {
	int temp;
	(void)arg;
	temp = DeeObject_Bool(elem);
	if (temp > 0)
		temp = -2;
	return temp;
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeSeq_Any(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &generic_seq_any_cb, NULL);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
generic_seq_all_cb(void *arg, DeeObject *elem) {
	int temp;
	(void)arg;
	temp = DeeObject_Bool(elem);
	if (temp == 0)
		temp = -2;
	return temp;
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeSeq_All(DeeObject *__restrict self) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &generic_seq_all_cb, NULL);
	ASSERT(foreach_status >= 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		return 0;
	if (foreach_status >= 0)
		return 1;
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
generic_seq_min_cb(void *arg, DeeObject *elem) {
	int temp;
	DeeObject *current = *(DeeObject **)arg;
	if (!current) {
		*(DeeObject **)arg = elem;
		Dee_Incref(elem);
		return 0;
	}
	temp = DeeObject_CmpLoAsBool(current, elem);
	if (temp == 0) {
		ASSERT(*(DeeObject **)arg == current);
		Dee_Decref(current);
		Dee_Incref(elem);
		*(DeeObject **)arg = elem;
	}
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
generic_seq_max_cb(void *arg, DeeObject *elem) {
	int temp;
	DeeObject *current = *(DeeObject **)arg;
	if (!current) {
		*(DeeObject **)arg = elem;
		Dee_Incref(elem);
		return 0;
	}
	temp = DeeObject_CmpLoAsBool(current, elem);
	if (temp > 0) {
		ASSERT(*(DeeObject **)arg == current);
		Dee_Decref(current);
		Dee_Incref(elem);
		*(DeeObject **)arg = elem;
	}
	return temp;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Min(DeeObject *self) {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &generic_seq_min_cb, &result);
	if unlikely(foreach_status < 0)
		goto err_r;
	if (result == NULL)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Max(DeeObject *self) {
	DREF DeeObject *result = NULL;
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &generic_seq_max_cb, &result);
	if unlikely(foreach_status < 0)
		goto err_r;
	if (result == NULL)
		return_none;
	return result;
err_r:
	Dee_XDecref(result);
	return NULL;
}



struct generic_seq_reduce_data {
	DeeObject      *gsr_combine; /* [1..1] Combinatory predicate (invoke as `gsr_combine(gsr_init, elem)') */
	DREF DeeObject *gsr_result;  /* [0..1] Current reduction result, or NULL if no init given and at first item. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
generic_seq_reduce_cb(void *arg, DeeObject *elem) {
	struct generic_seq_reduce_data *data;
	data = (struct generic_seq_reduce_data *)arg;
	if (!data->gsr_result) {
		data->gsr_result = elem;
		Dee_Incref(elem);
	} else {
		DeeObject *args[2];
		DREF DeeObject *reduced;
		args[0] = data->gsr_result;
		args[1] = elem;
		reduced = DeeObject_Call(data->gsr_combine, 2, args);
		if unlikely(!reduced)
			goto err;
		Dee_Decref(data->gsr_result);
		data->gsr_result = reduced;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_seq_reduce(DeeObject *self, DeeObject *combine, /*nullable*/ DeeObject *init) {
	Dee_ssize_t foreach_status;
	struct generic_seq_reduce_data data;
	data.gsr_combine = combine;
	data.gsr_result  = init;
	foreach_status = DeeObject_Foreach(self, &generic_seq_reduce_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data_result;
	if unlikely(!data.gsr_result)
		return_none;
	return data.gsr_result;
err_data_result:
	Dee_XDecref(data.gsr_result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
generic_seq_parity_cb(void *arg, DeeObject *elem) {
	(void)arg;
	return (Dee_ssize_t)DeeObject_Bool(elem);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_seq_parity(DeeObject *self) {
	Dee_ssize_t foreach_status; /* foreach_status is the number of elements considered "true" */
	foreach_status = DeeObject_Foreach(self, &generic_seq_parity_cb, NULL);
	if (foreach_status > 1)
		foreach_status = foreach_status & 1;
	return (int)foreach_status;
}

struct generic_seq_minmax_with_key_data {
	DeeObject      *gsmmwk_key;     /* [1..1] The key predicate to apply to elements. */
	DREF DeeObject *gsmmwk_result;  /* [0..1] The current result without a key applied (or NULL if at the first element) */
	DREF DeeObject *gsmmwk_kresult; /* [0..1] The current result with the key applied (or NULL if at the first or second element) */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
generic_seq_min_with_key_cb(void *arg, DeeObject *elem) {
	int temp;
	DREF DeeObject *kelem;
	struct generic_seq_minmax_with_key_data *data;
	data = (struct generic_seq_minmax_with_key_data *)arg;
	if (!data->gsmmwk_result) {
		Dee_Incref(elem);
		data->gsmmwk_result = elem; /* Inherit reference */
		return 0;
	}
	if unlikely(!data->gsmmwk_kresult) {
		DREF DeeObject *keyed;
		keyed = DeeObject_Call(data->gsmmwk_key, 1, &data->gsmmwk_result);
		if unlikely(!keyed)
			goto err;
		data->gsmmwk_kresult = keyed; /* Inherit reference */
	}
	kelem = DeeObject_Call(data->gsmmwk_key, 1, &elem);
	if unlikely(!kelem)
		goto err;
	temp = DeeObject_CmpLoAsBool(data->gsmmwk_kresult, kelem);
	if (temp > 0) {
		Dee_Decref(kelem);
		return 0;
	}
	if unlikely(temp < 0)
		goto err_kelem;
	Dee_Decref(data->gsmmwk_result);
	Dee_Decref(data->gsmmwk_kresult);
	Dee_Incref(elem);
	data->gsmmwk_result  = elem;  /* Inherit reference */
	data->gsmmwk_kresult = kelem; /* Inherit reference */
	return 0;
err_kelem:
	Dee_Decref(kelem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
generic_seq_max_with_key_cb(void *arg, DeeObject *elem) {
	int temp;
	DREF DeeObject *kelem;
	struct generic_seq_minmax_with_key_data *data;
	data = (struct generic_seq_minmax_with_key_data *)arg;
	if (!data->gsmmwk_result) {
		Dee_Incref(elem);
		data->gsmmwk_result = elem; /* Inherit reference */
		return 0;
	}
	if unlikely(!data->gsmmwk_kresult) {
		DREF DeeObject *keyed;
		keyed = DeeObject_Call(data->gsmmwk_key, 1, &data->gsmmwk_result);
		if unlikely(!keyed)
			goto err;
		data->gsmmwk_kresult = keyed; /* Inherit reference */
	}
	kelem = DeeObject_Call(data->gsmmwk_key, 1, &elem);
	if unlikely(!kelem)
		goto err;
	temp = DeeObject_CmpLoAsBool(data->gsmmwk_kresult, kelem);
	if (temp == 0) {
		Dee_Decref(kelem);
		return 0;
	}
	if unlikely(temp < 0)
		goto err_kelem;
	Dee_Decref(data->gsmmwk_result);
	Dee_Decref(data->gsmmwk_kresult);
	Dee_Incref(elem);
	data->gsmmwk_result  = elem;  /* Inherit reference */
	data->gsmmwk_kresult = kelem; /* Inherit reference */
	return 0;
err_kelem:
	Dee_Decref(kelem);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_seq_min_with_key(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct generic_seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = DeeObject_Foreach(self, &generic_seq_min_with_key_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data;
	if (!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_seq_max_with_key(DeeObject *self, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct generic_seq_minmax_with_key_data data;
	data.gsmmwk_key     = key;
	data.gsmmwk_result  = NULL;
	data.gsmmwk_kresult = NULL;
	foreach_status = DeeObject_Foreach(self, &generic_seq_max_with_key_cb, &data);
	if unlikely(foreach_status < 0)
		goto err_data;
	if (!data.gsmmwk_result) {
		ASSERT(!data.gsmmwk_kresult);
		return_none;
	}
	Dee_XDecref(data.gsmmwk_kresult);
	return data.gsmmwk_result;
err_data:
	if (data.gsmmwk_kresult) {
		ASSERT(data.gsmmwk_result);
		Dee_Decref(data.gsmmwk_kresult);
		Dee_Decref(data.gsmmwk_result);
	} else if (data.gsmmwk_result) {
		Dee_Decref(data.gsmmwk_result);
	}
	return NULL;
}



PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
generic_seq_count_cb(void *arg, DeeObject *key) {
	int temp = DeeObject_TryCompareEq((DeeObject *)arg, key);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? 1 : 0;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
generic_seq_count(DeeObject *self, DeeObject *elem) {
	return (size_t)DeeObject_Foreach(self, &generic_seq_count_cb, elem);
}

struct generic_seq_count_with_key_data {
	DeeObject *gscwk_key;   /* [1..1] Key predicate */
	DeeObject *gscwk_kelem; /* [1..1] Keyed search element. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
generic_seq_count_with_key_cb(void *arg, DeeObject *elem) {
	int temp;
	struct generic_seq_count_with_key_data *data;
	data = (struct generic_seq_count_with_key_data *)arg;
	temp = DeeObject_TryCompareKeyEq(data->gscwk_kelem, elem, data->gscwk_key);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
generic_seq_contains_with_key_cb(void *arg, DeeObject *elem) {
	int temp;
	struct generic_seq_count_with_key_data *data;
	data = (struct generic_seq_count_with_key_data *)arg;
	temp = DeeObject_TryCompareKeyEq(data->gscwk_kelem, elem, data->gscwk_key);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	return temp == 0 ? -2 : 0;
}

INTERN WUNUSED NONNULL((1, 2, 3)) size_t DCALL
generic_seq_count_with_key(DeeObject *self, DeeObject *elem, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct generic_seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &elem);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = DeeObject_Foreach(self, &generic_seq_count_with_key_cb, &data);
	Dee_Decref(data.gscwk_kelem);
	return (size_t)foreach_status;
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
generic_seq_contains_with_key(DeeObject *self, DeeObject *elem, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct generic_seq_count_with_key_data data;
	data.gscwk_key   = key;
	data.gscwk_kelem = DeeObject_Call(key, 1, &elem);
	if unlikely(!data.gscwk_kelem)
		goto err;
	foreach_status = DeeObject_Foreach(self, &generic_seq_contains_with_key_cb, &data);
	Dee_Decref(data.gscwk_kelem);
	ASSERT(foreach_status == 0 ||
	       foreach_status == -1 ||
	       foreach_status == -2);
	if (foreach_status == -2)
		foreach_status = 1;
	return (int)foreach_status;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
generic_seq_locate_cb(void *arg, DeeObject *elem) {
	DeeObject *elem_to_locate = *(DeeObject **)arg;
	int temp = DeeObject_TryCompareEq(elem_to_locate, elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		return -1;
	if (temp == 0) {
		Dee_Incref(elem);
		*(DeeObject **)arg = elem;
		return -2;
	}
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_seq_locate(DeeObject *self, DeeObject *elem) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self, &generic_seq_locate_cb, &elem);
	if likely(foreach_status == -2)
		return elem;
	if (foreach_status == 0)
		err_item_not_found(self, elem);
	return NULL;
}

struct generic_seq_locate_with_key_data {
	DeeObject *gslwk_kelem; /* [1..1] Keyed search element. */
	DeeObject *gslwk_key;   /* [1..1][in] Search key predicate
	                         * [1..1][out:DREF] Located element. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
generic_seq_locate_with_key_cb(void *arg, DeeObject *elem) {
	int temp;
	struct generic_seq_locate_with_key_data *data;
	data = (struct generic_seq_locate_with_key_data *)arg;
	temp = DeeObject_TryCompareKeyEq(data->gslwk_kelem, elem, data->gslwk_key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		Dee_Incref(elem);
		data->gslwk_key = elem;
		return -2;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
generic_seq_locate_with_key(DeeObject *self, DeeObject *elem, DeeObject *key) {
	Dee_ssize_t foreach_status;
	struct generic_seq_locate_with_key_data data;
	data.gslwk_kelem = DeeObject_Call(key, 1, &elem);
	if unlikely(!data.gslwk_kelem)
		goto err;
	data.gslwk_key = key;
	foreach_status = DeeObject_Foreach(self, &generic_seq_locate_with_key_cb, &data);
	Dee_Decref(data.gslwk_kelem);
	if likely(foreach_status == -2)
		return data.gslwk_key;
	if (foreach_status == 0)
		err_item_not_found(self, elem);
err:
	return NULL;
}


struct generic_seq_rlocate_with_foreach_data {
	DeeObject      *gsrlwf_elem;   /* [1..1] Element to search for. */
	DREF DeeObject *gsrlwf_result; /* [0..1] Most recent match. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
generic_seq_rlocate_with_foreach_cb(void *arg, DeeObject *elem) {
	int temp;
	struct generic_seq_rlocate_with_foreach_data *data;
	data = (struct generic_seq_rlocate_with_foreach_data *)arg;
	temp = DeeObject_TryCompareEq(data->gsrlwf_elem, elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		Dee_Incref(elem);
		Dee_XDecref(data->gsrlwf_result);
		data->gsrlwf_result = elem;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_seq_rlocate(DeeObject *self, DeeObject *elem) {
	Dee_ssize_t foreach_status;
	Dee_tsc_foreach_reverse_t op;
	if ((op = DeeType_SeqCache_TryRequireForeachReverse(Dee_TYPE(self))) != NULL) {
		foreach_status = (*op)(self, &generic_seq_locate_cb, &elem);
		if likely(foreach_status == -2)
			return elem;
		if (foreach_status == 0)
			err_item_not_found(self, elem);
	} else {
		struct generic_seq_rlocate_with_foreach_data data;
		data.gsrlwf_elem   = elem;
		data.gsrlwf_result = NULL;
		foreach_status = DeeObject_Foreach(self, &generic_seq_rlocate_with_foreach_cb, &data);
		if likely(foreach_status == 0) {
			if (data.gsrlwf_result)
				return data.gsrlwf_result;
			err_item_not_found(self, elem);
		}
	}
	return NULL;
}

struct generic_seq_rlocate_with_key_and_foreach_data {
	DeeObject      *gsrlwkf_kelem;  /* [1..1] Keyed element to search for. */
	DREF DeeObject *gsrlwkf_result; /* [0..1] Most recent match. */
	DeeObject      *gsrlwkf_key;    /* [1..1] Search key. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
generic_seq_rlocate_with_key_and_foreach_cb(void *arg, DeeObject *elem) {
	int temp;
	struct generic_seq_rlocate_with_key_and_foreach_data *data;
	data = (struct generic_seq_rlocate_with_key_and_foreach_data *)arg;
	temp = DeeObject_TryCompareKeyEq(data->gsrlwkf_kelem, elem, data->gsrlwkf_key);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	if (temp == 0) {
		Dee_Incref(elem);
		Dee_XDecref(data->gsrlwkf_result);
		data->gsrlwkf_result = elem;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
generic_seq_rlocate_with_key(DeeObject *self, DeeObject *elem, DeeObject *key) {
	Dee_tsc_foreach_reverse_t op;
	Dee_ssize_t foreach_status;
	struct generic_seq_locate_with_key_data data;
	data.gslwk_kelem = DeeObject_Call(key, 1, &elem);
	if unlikely(!data.gslwk_kelem)
		goto err;
	data.gslwk_key = key;
	if ((op = DeeType_SeqCache_TryRequireForeachReverse(Dee_TYPE(self))) != NULL) {
		foreach_status = (*op)(self, &generic_seq_locate_with_key_cb, &data);
		Dee_Decref(data.gslwk_kelem);
		if likely(foreach_status == -2)
			return data.gslwk_key;
		if (foreach_status == 0)
			err_item_not_found(self, elem);
	} else {
		struct generic_seq_rlocate_with_key_and_foreach_data rdata;
		rdata.gsrlwkf_kelem  = data.gslwk_kelem;
		rdata.gsrlwkf_key    = data.gslwk_key;
		rdata.gsrlwkf_result = NULL;
		foreach_status = DeeObject_Foreach(self, &generic_seq_rlocate_with_key_and_foreach_cb, &rdata);
		Dee_Decref(rdata.gsrlwkf_kelem);
		if likely(foreach_status == 0) {
			if (rdata.gsrlwkf_result)
				return rdata.gsrlwkf_result;
			err_item_not_found(self, elem);
		}
	}
err:
	return NULL;
}

/* Returns ITER_DONE if item doesn't exist. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_trygetfirst(DeeObject *self) {
	DREF DeeObject *result;
	Dee_tsc_getfirst_t getfirst;
	DeeTypeObject *tp_self = Dee_TYPE(self);
	getfirst = DeeType_SeqCache_RequireGetFirst(tp_self);
	if (getfirst == &DeeSeq_DefaultGetFirstWithGetItemIndex) {
		size_t size = (*tp_self->tp_seq->tp_size)(self);
		if unlikely(size == (size_t)-1)
			goto err;
		if (size == 0)
			return ITER_DONE;
	} else if (getfirst == &DeeSeq_DefaultGetFirstWithGetItem) {
		int temp;
		DREF DeeObject *sizeob;
		sizeob = (*tp_self->tp_seq->tp_sizeob)(self);
		if unlikely(sizeob == NULL)
			goto err;
		temp = DeeObject_Bool(sizeob);
		Dee_Decref(sizeob);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			return ITER_DONE;
	} else if (getfirst == &DeeSeq_DefaultGetFirstWithForeachDefault) {
		Dee_ssize_t foreach_status;
		foreach_status = DeeObject_Foreach(self, &seq_default_getfirst_with_foreach_cb, &result);
		if likely(foreach_status == -2)
			return result;
		if (foreach_status == 0)
			return ITER_DONE;
		goto err;
	}
	result = (*getfirst)(self);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}

/* Returns ITER_DONE if item doesn't exist. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_seq_trygetlast(DeeObject *self) {
	DREF DeeObject *result;
	Dee_tsc_getlast_t getlast;
	DeeTypeObject *tp_self = Dee_TYPE(self);
	getlast = DeeType_SeqCache_RequireGetLast(tp_self);
	if (getlast == &DeeSeq_DefaultGetLastWithSizeAndGetItemIndexFast) {
		size_t size = (*tp_self->tp_seq->tp_size)(self);
		if unlikely(size == (size_t)-1)
			goto err;
		if (size == 0)
			return ITER_DONE;
		result = (*tp_self->tp_seq->tp_getitem_index_fast)(self, size - 1);
		if (!result)
			return ITER_DONE;
		return result;
	} else if (getlast == &DeeSeq_DefaultGetLastWithSizeAndGetItemIndex) {
		size_t size = (*tp_self->tp_seq->tp_size)(self);
		if unlikely(size == (size_t)-1)
			goto err;
		if (size == 0)
			return ITER_DONE;
		result = (*tp_self->tp_seq->tp_getitem_index)(self, size - 1);
		if (result)
			return result;
		if (DeeError_Catch(&DeeError_UnboundItem))
			return ITER_DONE;
		goto err;
	} else if (getlast == &DeeSeq_DefaultGetLastWithSizeObAndGetItem) {
		int temp;
		DREF DeeObject *sizeob;
		struct Dee_type_seq *seq = Dee_TYPE(self)->tp_seq;
		sizeob = (*seq->tp_sizeob)(self);
		if unlikely(!sizeob)
			goto err;
		temp = DeeObject_Bool(sizeob);
		if unlikely(temp < 0)
			goto err_sizeob;
		if unlikely(!temp) {
			Dee_Decref(sizeob);
			return ITER_DONE;
		}
		if unlikely(DeeObject_Dec(&sizeob))
			goto err_sizeob;
		result = (*Dee_TYPE(self)->tp_seq->tp_getitem)(self, sizeob);
		Dee_Decref(sizeob);
		if (result)
			return result;
		if (DeeError_Catch(&DeeError_UnboundItem))
			return ITER_DONE;
		goto err;
err_sizeob:
		Dee_Decref(sizeob);
		goto err;
	} else if (getlast == &DeeSeq_DefaultGetLastWithForeachDefault) {
		Dee_ssize_t foreach_status;
		Dee_Incref(Dee_None);
		result = Dee_None;
		foreach_status = DeeObject_Foreach(self, &seq_default_last_with_foreach_cb, &result);
		if likely(foreach_status > 0)
			return result;
		Dee_Decref_unlikely(result);
		if (foreach_status == 0)
			return ITER_DONE;
		goto err;
	}
	result = (*getlast)(self);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_seq_startswith(DeeObject *self, DeeObject *elem) {
	int result;
	DREF DeeObject *first = generic_seq_trygetfirst(self);
	if unlikely(!first)
		goto err;
	if (first == ITER_DONE)
		return 0;
	result = DeeObject_TryCompareEq(elem, first);
	Dee_Decref(first);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
generic_seq_startswith_with_key(DeeObject *self, DeeObject *elem, DeeObject *key) {
	int result;
	DREF DeeObject *first = generic_seq_trygetfirst(self);
	if unlikely(!first)
		goto err;
	if (first == ITER_DONE)
		return 0;
	elem = DeeObject_Call(key, 1, &elem);
	if unlikely(!elem)
		goto err_elem;
	result = DeeObject_TryCompareKeyEq(elem, first, key);
	Dee_Decref(elem);
	Dee_Decref(first);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err_elem:
	Dee_Decref(elem);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_seq_endswith(DeeObject *self, DeeObject *elem) {
	int result;
	DREF DeeObject *last = generic_seq_trygetlast(self);
	if unlikely(!last)
		goto err;
	if (last == ITER_DONE)
		return 0;
	result = DeeObject_TryCompareEq(elem, last);
	Dee_Decref(last);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
generic_seq_endswith_with_key(DeeObject *self, DeeObject *elem, DeeObject *key) {
	int result;
	DREF DeeObject *last = generic_seq_trygetlast(self);
	if unlikely(!last)
		goto err;
	if (last == ITER_DONE)
		return 0;
	elem = DeeObject_Call(key, 1, &elem);
	if unlikely(!elem)
		goto err_elem;
	result = DeeObject_TryCompareKeyEq(elem, last, key);
	Dee_Decref(elem);
	Dee_Decref(last);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 1 : 0;
err_elem:
	Dee_Decref(elem);
err:
	return -1;
}


union generic_seq_find_data {
	DeeObject *gsfd_elem;  /* [in][1..1] Element to search for */
	size_t     gsfd_index; /* [out] Located index */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
generic_seq_find_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	union generic_seq_find_data *data;
	data = (union generic_seq_find_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareEq(data->gsfd_elem, value);
	if (cmp == 0) {
		/* Found the index! */
		data->gsfd_index = index;
		return -2;
	}
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
generic_seq_find(DeeObject *self, DeeObject *elem, size_t start, size_t end) {
	Dee_ssize_t status;
	union generic_seq_find_data data;
	data.gsfd_elem = elem;
	status = DeeSeq_EnumerateIndex(self, &generic_seq_find_cb, &data, start, end);
	if likely(status == -2) {
		if unlikely(data.gsfd_index == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return data.gsfd_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}

struct generic_seq_find_with_key_data {
	union generic_seq_find_data gsfwk_base; /* Base find data */
	DeeObject                  *gsfwk_key;  /* Find element key */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
generic_seq_find_with_key_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	struct generic_seq_find_with_key_data *data;
	data = (struct generic_seq_find_with_key_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareKeyEq(data->gsfwk_base.gsfd_elem, value, data->gsfwk_key);
	if (cmp == 0) {
		/* Found the index! */
		data->gsfwk_base.gsfd_index = index;
		return -2;
	}
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
generic_seq_find_with_key(DeeObject *self, DeeObject *elem,
                          size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t status;
	struct generic_seq_find_with_key_data data;
	data.gsfwk_base.gsfd_elem = DeeObject_Call(key, 1, &elem);
	if unlikely(!data.gsfwk_base.gsfd_elem)
		goto err;
	data.gsfwk_key = key;
	status = DeeSeq_EnumerateIndex(self, &generic_seq_find_with_key_cb, &data, start, end);
	Dee_Decref(data.gsfwk_base.gsfd_elem);
	if likely(status == -2) {
		if unlikely(data.gsfwk_base.gsfd_index == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return data.gsfwk_base.gsfd_index;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}


struct generic_seq_rfind_data {
	DeeObject *gsrfd_elem;   /* [1..1] The element to search for */
	size_t     gsrfd_result; /* The last-matched index. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
generic_seq_rfind_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	struct generic_seq_rfind_data *data;
	data = (struct generic_seq_rfind_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareEq(data->gsrfd_elem, value);
	if (cmp == 0)
		data->gsrfd_result = index;
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
generic_seq_rfind(DeeObject *self, DeeObject *elem, size_t start, size_t end) {
	Dee_ssize_t status;
	union generic_seq_find_data data;
	Dee_tsc_enumerate_index_reverse_t renum;
	data.gsfd_elem = elem;
	if ((renum = DeeType_SeqCache_TryRequireEnumerateIndexReverse(Dee_TYPE(self))) != NULL) {
		status = (*renum)(self, &generic_seq_find_cb, &data, start, end);
		if likely(status == -2) {
			if unlikely(data.gsfd_index == (size_t)Dee_COMPARE_ERR)
				err_integer_overflow_i(sizeof(size_t) * 8, true);
			return data.gsfd_index;
		}
	} else {
		struct generic_seq_rfind_data rdata;
		rdata.gsrfd_elem   = elem;
		rdata.gsrfd_result = (size_t)-1;
		status = DeeSeq_EnumerateIndex(self, &generic_seq_rfind_cb, &rdata, start, end);
		ASSERT(status == 0 || status == -1);
		if unlikely(status == -1)
			goto err;
		if unlikely(rdata.gsrfd_result == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return rdata.gsrfd_result;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}

struct generic_seq_rfind_with_key_data {
	DeeObject *gsrfwkd_kelem;   /* [1..1] The element to search for */
	size_t     gsrfwkd_result; /* The last-matched index. */
	DeeObject *gsrfwkd_key;    /* [1..1] Search key. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
generic_seq_rfind_with_key_cb(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	int cmp;
	struct generic_seq_rfind_with_key_data *data;
	data = (struct generic_seq_rfind_with_key_data *)arg;
	if (!value)
		return 0;
	cmp = DeeObject_TryCompareEq(data->gsrfwkd_kelem, value);
	if (cmp == 0)
		data->gsrfwkd_result = index;
	if unlikely(cmp == Dee_COMPARE_ERR)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
generic_seq_rfind_with_key(DeeObject *self, DeeObject *elem,
                           size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t status;
	struct generic_seq_find_with_key_data data;
	Dee_tsc_enumerate_index_reverse_t renum;
	data.gsfwk_base.gsfd_elem = DeeObject_Call(key, 1, &elem);
	if unlikely(!data.gsfwk_base.gsfd_elem)
		goto err;
	if ((renum = DeeType_SeqCache_TryRequireEnumerateIndexReverse(Dee_TYPE(self))) != NULL) {
		data.gsfwk_key = key;
		status = (*renum)(self, &generic_seq_find_with_key_cb, &data, start, end);
		Dee_Decref(data.gsfwk_base.gsfd_elem);
		if likely(status == -2) {
			if unlikely(data.gsfwk_base.gsfd_index == (size_t)Dee_COMPARE_ERR)
				err_integer_overflow_i(sizeof(size_t) * 8, true);
			return data.gsfwk_base.gsfd_index;
		}
	} else {
		struct generic_seq_rfind_with_key_data rdata;
		rdata.gsrfwkd_kelem  = data.gsfwk_base.gsfd_elem;
		rdata.gsrfwkd_result = (size_t)-1;
		status = DeeSeq_EnumerateIndex(self, &generic_seq_rfind_with_key_cb, &rdata, start, end);
		Dee_Decref(rdata.gsrfwkd_kelem);
		ASSERT(status == 0 || status == -1);
		if unlikely(status == -1)
			goto err;
		if unlikely(rdata.gsrfwkd_result == (size_t)Dee_COMPARE_ERR)
			err_integer_overflow_i(sizeof(size_t) * 8, true);
		return rdata.gsrfwkd_result;
	}
	if unlikely(status == -1)
		goto err;
	return (size_t)-1;
err:
	return (size_t)Dee_COMPARE_ERR;
}



DECL_END

#ifndef __INTELLISENSE__
#define DEFINE_DeeType_SeqCache_RequireGetX
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireBoundX
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireDelX
#include "default-api-require-impl.c.inl"
#define DEFINE_DeeType_SeqCache_RequireSetX
#include "default-api-require-impl.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_C */
