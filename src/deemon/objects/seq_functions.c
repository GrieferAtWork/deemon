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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_C
#define GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_C 1

#include "seq_functions.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/minmax.h>

#include "../runtime/runtime_error.h"

DECL_BEGIN

#ifndef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
/* NOTE: Technically, all of these functions can be used on any type of object,
 *       but all objects derived from `DeeSeq_Type' automatically implement
 *       all of them as member functions.
 *       With that in mind, any type implementing the `tp_seq' interface
 *       with the intention of behaving as an Iterable, should probably
 *       be derived from `DeeSeq_Type' as this allows usercode to query
 *       for a general purpose sequence by writing `x is Sequence from deemon' */
INTERN WUNUSED NONNULL((1)) size_t DCALL DeeSeq_Size(DeeObject *__restrict self) {
	DREF DeeObject *iter, *elem;
	size_t result = 0;
	/* Count the number of elements, given an iterator. */
	iter = DeeObject_Iter(self);
	if unlikely(!iter) {
		if (DeeError_Catch(&DeeError_NotImplemented))
			err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SIZE);
		goto err;
	}
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		Dee_Decref(elem);
		if unlikely(result == (size_t)-2) {
			err_integer_overflow_i(sizeof(size_t) * 8, true);
			goto err;
		}
		++result;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	Dee_Decref(iter);
	if unlikely(!elem)
		goto err;
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
iterator_get_nth(DeeObject *__restrict self,
                 DeeObject *__restrict sequence,
                 size_t index) {
	DREF DeeObject *elem;
	size_t current_index = 0;
	while (ITER_ISOK(elem = DeeObject_IterNext(self))) {
		if (current_index == (size_t)index)
			return elem;
		Dee_Decref(elem);
		++current_index;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if unlikely(!elem)
		goto err;
	err_index_out_of_bounds(sequence, index, current_index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_GetItem(DeeObject *__restrict self, size_t index) {
	DeeTypeObject *tp_self;
	DREF DeeObject *result;
	DeeTypeMRO mro;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if unlikely(tp_self == &DeeSeq_Type) {
		err_index_out_of_bounds(self, index, 0);
		goto err;
	}
	DeeTypeMRO_Init(&mro, tp_self);
	for (;;) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi const *nsi;
			if ((nsi = seq->tp_nsi) != NULL &&
			    nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				if (nsi->nsi_seqlike.nsi_getitem)
					return (*nsi->nsi_seqlike.nsi_getitem)(self, index);
				if (nsi->nsi_seqlike.nsi_getitem_fast) {
					size_t mylen = (*nsi->nsi_common.nsi_getsize)(self);
					if unlikely(mylen == (size_t)-1)
						goto err;
					if unlikely(index >= mylen) {
						err_index_out_of_bounds(self, index, mylen);
						goto err;
					}
					result = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, index);
					if unlikely(!result)
						err_unbound_index(self, index);
					return result;
				}
				if (nsi->nsi_seqlike.nsi_getrange) {
					result = (*nsi->nsi_seqlike.nsi_getrange)(self, index, index + 1);
					if unlikely(!result)
						goto err;
					goto return_result_first;
				}
			}
			if (has_noninherited_getitem(tp_self, seq)) {
				DREF DeeObject *index_ob;
				index_ob = DeeInt_NewSize(index);
				if unlikely(!index_ob)
					goto err;
				result = (*seq->tp_getitem)(self, index_ob);
				Dee_Decref(index_ob);
				return result;
			}
			if (has_noninherited_getrange(tp_self, seq)) {
				DREF DeeObject *real_result;
				DREF DeeObject *index_ob, *index_plus1_ob;
				index_ob = DeeInt_NewSize(index);
				if unlikely(!index_ob)
					goto err;
				index_plus1_ob = DeeInt_NewSize(index + 1);
				if unlikely(!index_plus1_ob) {
					Dee_Decref(index_ob);
					goto err;
				}
				result = (*seq->tp_getrange)(self, index_ob, index_plus1_ob);
				Dee_Decref(index_plus1_ob);
				Dee_Decref(index_ob);
				if unlikely(!result)
					goto err;
return_result_first:
				real_result = DeeSeq_Front(result);
				Dee_Decref(result);
				if unlikely(!real_result) {
					/* Translate the empty-sequence error into an index-out-of-bounds */
					if (DeeError_Catch(&DeeError_ValueError)) {
						size_t mylen = DeeObject_Size(self);
						if unlikely(mylen == (size_t)-1)
							goto err;
						err_index_out_of_bounds(self, index, mylen);
					}
				}
				return real_result;
			}
			if (seq->tp_iter) {
				DREF DeeObject *iterator;
				iterator = (*seq->tp_iter)(self);
				if unlikely(!iterator)
					goto err;
				result = iterator_get_nth(iterator, self, index);
				Dee_Decref(iterator);
				return result;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
		if (tp_self == &DeeSeq_Type)
			break;
	}
	err_unimplemented_operator3(Dee_TYPE(self),
	                            OPERATOR_GETITEM,
	                            OPERATOR_GETRANGE,
	                            OPERATOR_ITER);
err:
	return NULL;
}

INTERN ATTR_COLD int DCALL
err_no_generic_sequence(DeeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Neither `%k.__getitem__' and `%k.__size__', nor `%k.__iter__' are implemented",
	                       Dee_TYPE(self), Dee_TYPE(self), Dee_TYPE(self));
}




INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_NonEmpty(DeeObject *__restrict self) {
	DeeTypeObject *tp_self;
	DeeTypeMRO mro;
	int result;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if unlikely(tp_self == &DeeSeq_Type)
		return 0;
	DeeTypeMRO_Init(&mro, tp_self);
	for (;;) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi const *nsi;
			DREF DeeObject *temp;
			if ((nsi = seq->tp_nsi) != NULL &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				size_t length = (*nsi->nsi_common.nsi_getsize)(self);
				if unlikely(length == (size_t)-1)
					goto err;
				return length != 0;
			}
			if (has_noninherited_size(tp_self, seq)) {
				if (has_noninherited_bool(tp_self))
					return (*tp_self->tp_cast.tp_bool)(self);
				temp = (*seq->tp_sizeob)(self);
				if unlikely(!temp)
					goto err;
				result = DeeObject_Bool(temp);
				Dee_Decref(temp);
				if unlikely(result < 0)
					goto err;
				return result;
			}
			if (seq->tp_iter) {
				DREF DeeObject *elem;
				temp = (*seq->tp_iter)(self);
				if unlikely(!temp)
					goto err;
				elem = DeeObject_IterNext(temp);
				Dee_Decref(temp);
				if (elem == ITER_DONE)
					return 0;
				if unlikely(!elem)
					goto err;
				Dee_Decref(elem);
				return 1;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
		if (tp_self == &DeeSeq_Type)
			break;
	}
	err_unimplemented_operator2(Dee_TYPE(self),
	                            OPERATOR_SIZE,
	                            OPERATOR_ITER);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Front(DeeObject *__restrict self) {
	DeeTypeObject *tp_self;
	DREF DeeObject *result;
	DeeTypeMRO mro;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi const *nsi;
			DREF DeeObject *temp;
			if ((nsi = seq->tp_nsi) != NULL &&
			    nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    nsi->nsi_seqlike.nsi_getitem &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				result = (*nsi->nsi_seqlike.nsi_getitem)(self, 0);
				if unlikely(!result) {
					if (DeeError_Catch(&DeeError_IndexError))
						goto err_empty;
				}
				return result;
			}
			if (has_noninherited_getitem(tp_self, seq)) {
				result = (*seq->tp_getitem)(self, DeeInt_Zero);
				if unlikely(!result) {
					if (DeeError_Catch(&DeeError_IndexError))
						goto err_empty;
				}
				return result;
			}
			if (seq->tp_iter) {
				temp = (*seq->tp_iter)(self);
				if unlikely(!temp)
					goto err;
				result = DeeObject_IterNext(temp);
				Dee_Decref(temp);
				if (result == ITER_DONE)
					goto err_empty;
				return result;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_unimplemented_operator2(Dee_TYPE(self),
	                            OPERATOR_GETITEM,
	                            OPERATOR_ITER);
err:
	return NULL;
err_empty:
	err_empty_sequence(self);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Back(DeeObject *__restrict self) {
	DeeTypeObject *tp_self;
	size_t seq_length;
	DREF DeeObject *result, *temp;
	DeeTypeMRO mro;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi const *nsi;
			if ((nsi = seq->tp_nsi) != NULL &&
			    nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				if (nsi->nsi_seqlike.nsi_getitem) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if unlikely(!seq_length)
						goto err_empty;
					if (nsi->nsi_seqlike.nsi_getitem_fast)
						return (*nsi->nsi_seqlike.nsi_getitem_fast)(self, seq_length - 1);
					return (*nsi->nsi_seqlike.nsi_getitem)(self, seq_length - 1);
				}
				if (has_noninherited_getitem(tp_self, seq)) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if unlikely(!seq_length)
						goto err_empty;
					temp = DeeInt_NewSize(seq_length - 1);
					if unlikely(!temp)
						goto err;
					result = (*seq->tp_getitem)(self, temp);
					Dee_Decref(temp);
					return result;
				}
			}
			if (has_noninherited_getitem(tp_self, seq) &&
			    has_noninherited_size(tp_self, seq)) {
				int size_is_nonzero;
				temp = (*seq->tp_sizeob)(self);
				if unlikely(!temp)
					goto err;
				size_is_nonzero = DeeObject_Bool(temp);
				if unlikely(size_is_nonzero <= 0) {
					if unlikely(size_is_nonzero < 0)
						goto err_temp;
					Dee_Decref(temp);
					goto err_empty;
				}
				if (DeeObject_Dec(&temp))
					goto err_temp;
				result = (*seq->tp_getitem)(self, temp);
				Dee_Decref(temp);
				return result;
			}
			if (seq->tp_iter) {
				DREF DeeObject *next;
				temp = (*seq->tp_iter)(self);
				if unlikely(!temp)
					goto err;
				result = NULL;
				while (ITER_ISOK(next = DeeObject_IterNext(temp))) {
					Dee_XDecref(result);
					result = next;
					if (DeeThread_CheckInterrupt()) {
						Dee_Decref(result);
						goto err;
					}
				}
				Dee_Decref(temp);
				if unlikely(!next) {
					Dee_XClear(result);
				} else if unlikely(!result) {
					goto err_empty;
				}
				return result;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_no_generic_sequence(self);
err:
	return NULL;
err_empty:
	err_empty_sequence(self);
	return NULL;
err_temp:
	Dee_Decref(temp);
	goto err;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
iterator_find(DeeObject *__restrict iterator,
              DeeObject *__restrict keyed_search_item,
              size_t start, size_t end,
              DeeObject *key) {
	DREF DeeObject *elem;
	size_t index = 0;
	int temp;
	size_t search_size = end - start;
	while (start) {
		elem = DeeObject_IterNext(iterator);
		if (!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			goto notfound;
		}
		Dee_Decref(elem);
		--start;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		temp = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, elem, key);
		Dee_Decref(elem);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			/* Found it! */
			return index;
		}
		++index;
		if unlikely(index == (size_t)-2) {
			err_integer_overflow_i(sizeof(size_t) * 8, true);
			goto err;
		}
		if (!--search_size)
			goto notfound; /* End of search size */
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if unlikely(!elem)
		goto err;
notfound:
	return (size_t)-1; /* Not found. */
err:
	return (size_t)-2;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
iterator_rfind(DeeObject *__restrict iterator,
               DeeObject *__restrict keyed_search_item,
               size_t start, size_t end,
               DeeObject *key) {
	DREF DeeObject *elem;
	size_t index = 0;
	int temp;
	size_t search_size = end - start;
	size_t result      = (size_t)-1;
	while (start) {
		elem = DeeObject_IterNext(iterator);
		if (!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			goto notfound;
		}
		Dee_Decref(elem);
		--start;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		temp = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, elem, key);
		Dee_Decref(elem);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			/* Found it! */
			result = index;
		}
		++index;
		if unlikely(index == (size_t)-2) {
			err_integer_overflow_i(sizeof(size_t) * 8, true);
			goto err;
		}
		if (!--search_size)
			return result; /* End of search size */
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if unlikely(!elem)
		goto err;
	return result;
err:
	return (size_t)-2;
notfound:
	return (size_t)-1;
}


INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
DeeSeq_Find(DeeObject *self,
            size_t start, size_t end,
            DeeObject *keyed_search_item,
            DeeObject *key) {
	DREF DeeObject *iterator;
	size_t result;
	DeeTypeObject *tp_self;
	size_t i, seq_length;
	DREF DeeObject *temp;
	DeeTypeMRO mro;
	int error;
	ASSERT_OBJECT(self);
	if unlikely(start >= end)
		goto notfound;
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq = tp_self->tp_seq;
		if (seq) {
			struct type_nsi const *nsi = seq->tp_nsi;
			if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				if (nsi->nsi_seqlike.nsi_find)
					return (*nsi->nsi_seqlike.nsi_find)(self, start, end, keyed_search_item, key);
				if (nsi->nsi_seqlike.nsi_getitem_fast) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if (start >= seq_length)
						goto notfound;
					if (end > seq_length)
						end = seq_length;
					for (i = start; i < end; ++i) {
						temp = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, i);
						if unlikely(!temp)
							continue;
						error = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, temp, key);
						Dee_Decref(temp);
						if (error != 0) {
							if unlikely(error < 0)
								goto err;
							if unlikely(i == (size_t)-2 || i == (size_t)-1) {
								err_integer_overflow_i(sizeof(size_t) * 8, true);
								goto err;
							}
							return i;
						}
					}
					goto notfound;
				}
				if (nsi->nsi_seqlike.nsi_getitem) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if (start >= seq_length)
						goto notfound;
					if (end > seq_length)
						end = seq_length;
					for (i = start; i < end; ++i) {
						temp = (*nsi->nsi_seqlike.nsi_getitem)(self, i);
						if unlikely(!temp) {
							if (DeeError_Catch(&DeeError_UnboundItem))
								continue;
							goto err;
						}
						error = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, temp, key);
						Dee_Decref(temp);
						if (error != 0) {
							if unlikely(error < 0)
								goto err;
							if unlikely(i == (size_t)-2 || i == (size_t)-1) {
								err_integer_overflow_i(sizeof(size_t) * 8, true);
								goto err;
							}
							return i;
						}
					}
					goto notfound;
				}
				if (has_noninherited_getitem(tp_self, seq)) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
do_lookup_tpget:
					if (start >= seq_length)
						goto notfound;
					if (end > seq_length)
						end = seq_length;
					for (i = start; i < end; ++i) {
						DREF DeeObject *index_ob;
						index_ob = DeeInt_NewSize(i);
						if unlikely(!index_ob)
							goto err;
						temp = (*seq->tp_getitem)(self, index_ob);
						Dee_Decref(index_ob);
						if unlikely(!temp) {
							if (DeeError_Catch(&DeeError_UnboundItem))
								continue;
							goto err;
						}
						error = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, temp, key);
						Dee_Decref(temp);
						if (error != 0) {
							if unlikely(error < 0)
								goto err;
							if unlikely(i == (size_t)-2 || i == (size_t)-1) {
								err_integer_overflow_i(sizeof(size_t) * 8, true);
								goto err;
							}
							return i;
						}
					}
					goto notfound;
				}
			}
			if (has_noninherited_getitem(tp_self, seq) &&
			    has_noninherited_size(tp_self, seq)) {
				temp = (*seq->tp_sizeob)(self);
				if unlikely(!temp)
					goto err;
				if (DeeObject_AsSize(temp, &seq_length))
					goto err_temp;
				Dee_Decref(temp);
				goto do_lookup_tpget;
			}
			if (seq->tp_iter) {
				/* Use iterators */
				if unlikely((iterator = (*seq->tp_iter)(self)) == NULL)
					goto err;
				result = iterator_find(iterator, keyed_search_item, start, end, key);
				Dee_Decref(iterator);
				return result;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_no_generic_sequence(self);
err:
	return (size_t)-2;
err_temp:
	Dee_Decref(temp);
	goto err;
notfound:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
DeeSeq_RFind(DeeObject *self,
             size_t start, size_t end,
             DeeObject *keyed_search_item,
             DeeObject *key) {
	DREF DeeObject *iterator;
	size_t result;
	DeeTypeObject *tp_self;
	size_t i, seq_length;
	DREF DeeObject *temp;
	DeeTypeMRO mro;
	int error;
	ASSERT_OBJECT(self);
	if unlikely(start >= end)
		goto notfound;
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq = tp_self->tp_seq;
		if (seq) {
			struct type_nsi const *nsi = seq->tp_nsi;
			if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				if (nsi->nsi_seqlike.nsi_rfind)
					return (*nsi->nsi_seqlike.nsi_rfind)(self, start, end, keyed_search_item, key);
				if (nsi->nsi_seqlike.nsi_getitem_fast) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if (start > seq_length)
						start = seq_length;
					if (end > seq_length)
						end = seq_length;
					i = end - 1;
					do {
						temp = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, i);
						if unlikely(!temp)
							continue;
						error = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, temp, key);
						Dee_Decref(temp);
						if (error != 0) {
							if unlikely(error < 0)
								goto err;
							if unlikely(i == (size_t)-2 || i == (size_t)-1) {
								err_integer_overflow_i(sizeof(size_t) * 8, true);
								goto err;
							}
							return i;
						}
					} while (i-- > start);
					goto notfound;
				}
				if (nsi->nsi_seqlike.nsi_getitem) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if (start > seq_length)
						start = seq_length;
					if (end > seq_length)
						end = seq_length;
					i = end - 1;
					do {
						temp = (*nsi->nsi_seqlike.nsi_getitem)(self, i);
						if unlikely(!temp) {
							if (DeeError_Catch(&DeeError_UnboundItem))
								continue;
							goto err;
						}
						error = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, temp, key);
						Dee_Decref(temp);
						if (error != 0) {
							if unlikely(error < 0)
								goto err;
							if unlikely(i == (size_t)-2 || i == (size_t)-1) {
								err_integer_overflow_i(sizeof(size_t) * 8, true);
								goto err;
							}
							return i;
						}
					} while (i-- > start);
					goto notfound;
				}
				if (has_noninherited_getitem(tp_self, seq)) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
do_lookup_tpget:
					if (start > seq_length)
						start = seq_length;
					if (end > seq_length)
						end = seq_length;
					i = end - 1;
					do {
						DREF DeeObject *index_ob;
						index_ob = DeeInt_NewSize(i);
						if unlikely(!index_ob)
							goto err;
						temp = (*seq->tp_getitem)(self, index_ob);
						Dee_Decref(index_ob);
						if unlikely(!temp) {
							if (DeeError_Catch(&DeeError_UnboundItem))
								continue;
							goto err;
						}
						error = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, temp, key);
						Dee_Decref(temp);
						if (error != 0) {
							if unlikely(error < 0)
								goto err;
							if unlikely(i == (size_t)-2 || i == (size_t)-1) {
								err_integer_overflow_i(sizeof(size_t) * 8, true);
								goto err;
							}
							return i;
						}
					} while (i-- > start);
					goto notfound;
				}
			}
			if (has_noninherited_getitem(tp_self, seq) &&
			    has_noninherited_size(tp_self, seq)) {
				temp = (*seq->tp_sizeob)(self);
				if unlikely(!temp)
					goto err;
				if (DeeObject_AsSize(temp, &seq_length))
					goto err_temp;
				Dee_Decref(temp);
				goto do_lookup_tpget;
			}
			if (seq->tp_iter) {
				/* Use iterators */
				if unlikely((iterator = (*seq->tp_iter)(self)) == NULL)
					goto err;
				result = iterator_rfind(iterator, keyed_search_item, start, end, key);
				Dee_Decref(iterator);
				return result;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_no_generic_sequence(self);
err:
	return (size_t)-2;
err_temp:
	Dee_Decref(temp);
	goto err;
notfound:
	return (size_t)-1;
}
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */





/************************************************************************/
/* Sequence compare                                                     */
/************************************************************************/

/* @return: == Dee_COMPARE_ERR: An error occurred.
 * @return: == -1: `self < some_object'
 * @return: == 0:  Objects compare as equal
 * @return: == 1:  `self > some_object' */
INTERN WUNUSED int DCALL
DeeSeq_CompareVV(DeeObject *const *lhsv, size_t lhsc,
                 DeeObject *const *rhsv, size_t rhsc) {
	size_t i, common = MIN(lhsc, rhsc);
	for (i = 0; i < common; ++i) {
		int diff;
		diff = DeeObject_Compare(lhsv[i], rhsv[i]);
		if (diff != 0)
			return diff;
	}
	return lhsc < rhsc ? -1 : lhsc > rhsc ? 1 : 0;
}

INTERN WUNUSED NONNULL((3)) int DCALL
DeeSeq_CompareVF(DeeObject *const *lhsv, size_t lhsc,
                 DeeObject *rhs, size_t rhsc) {
	size_t i, common = MIN(lhsc, rhsc);
	for (i = 0; i < common; ++i) {
		int diff;
		DREF DeeObject *rhs_item;
		rhs_item = DeeFastSeq_GetItem_deprecated(rhs, i);
		if unlikely(!rhs_item)
			return Dee_COMPARE_ERR;
		diff = DeeObject_Compare(lhsv[i], rhs_item);
		Dee_Decref(rhs_item);
		if (diff != 0)
			return diff;
	}
	return lhsc < rhsc ? -1 : lhsc > rhsc ? 1 : 0;
}

INTERN WUNUSED NONNULL((3)) int DCALL
DeeSeq_CompareVI(DeeObject *const *lhsv, size_t lhsc,
                 DeeObject *rhs) {
	size_t i;
	DREF DeeObject *rhs_item;
	for (i = 0; i < lhsc; ++i) {
		int diff;
		rhs_item = DeeObject_IterNext(rhs);
		if unlikely(!ITER_ISOK(rhs_item)) {
			if (rhs_item == ITER_DONE)
				return 1; /* COUNT(LHS) > COUNT(RHS) */
			return Dee_COMPARE_ERR;
		}
		diff = DeeObject_Compare(lhsv[i], rhs_item);
		Dee_Decref(rhs_item);
		if (diff != 0)
			return diff;
	}
	rhs_item = DeeObject_IterNext(rhs);
	if (!ITER_ISOK(rhs_item)) {
		if (rhs_item == ITER_DONE)
			return 0; /* COUNT(LHS) == COUNT(RHS) */
		return Dee_COMPARE_ERR;
	}
	Dee_Decref(rhs_item);
	return -1; /* COUNT(LHS) < COUNT(RHS) */
}

INTERN WUNUSED NONNULL((3)) int DCALL
DeeSeq_CompareVS(DeeObject *const *lhsv, size_t lhsc,
                 DeeObject *rhs) {
	int result;
	size_t rhs_size;
	if (DeeTuple_Check(rhs)) {
		result = DeeSeq_CompareVV(lhsv, lhsc, DeeTuple_ELEM(rhs), DeeTuple_SIZE(rhs));
	} else if ((rhs_size = DeeFastSeq_GetSize_deprecated(rhs)) != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		result = DeeSeq_CompareVF(lhsv, lhsc, rhs, rhs_size);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter)
			return Dee_COMPARE_ERR;
		result = DeeSeq_CompareVI(lhsv, lhsc, rhs_iter);
		Dee_Decref(rhs_iter);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_CompareFV(DeeObject *lhs, size_t lhsc,
                 DeeObject *const *rhsv, size_t rhsc) {
	size_t i, common = MIN(lhsc, rhsc);
	for (i = 0; i < common; ++i) {
		int diff;
		DREF DeeObject *lhs_item;
		lhs_item = DeeFastSeq_GetItem_deprecated(lhs, i);
		if unlikely(!lhs_item)
			return Dee_COMPARE_ERR;
		diff = DeeObject_Compare(lhs_item, rhsv[i]);
		Dee_Decref(lhs_item);
		if (diff != 0)
			return diff;
	}
	return lhsc < rhsc ? -1 : lhsc > rhsc ? 1 : 0;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_CompareFF(DeeObject *lhs, size_t lhsc, DeeObject *rhs, size_t rhsc) {
	size_t i, common = MIN(lhsc, rhsc);
	for (i = 0; i < common; ++i) {
		int diff;
		DREF DeeObject *lhs_item;
		DREF DeeObject *rhs_item;
		lhs_item = DeeFastSeq_GetItem_deprecated(lhs, i);
		if unlikely(!lhs_item)
			return Dee_COMPARE_ERR;
		rhs_item = DeeFastSeq_GetItem_deprecated(rhs, i);
		if unlikely(!rhs_item) {
			Dee_Decref(lhs_item);
			return Dee_COMPARE_ERR;
		}
		diff = DeeObject_Compare(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff != 0)
			return diff;
	}
	return lhsc < rhsc ? -1 : lhsc > rhsc ? 1 : 0;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_CompareFI(DeeObject *lhs, size_t lhsc, DeeObject *rhs) {
	size_t i;
	DREF DeeObject *rhs_item;
	for (i = 0; i < lhsc; ++i) {
		DREF DeeObject *lhs_item;
		int diff;
		lhs_item = DeeFastSeq_GetItem_deprecated(lhs, i);
		if unlikely(!lhs_item)
			return Dee_COMPARE_ERR;
		rhs_item = DeeObject_IterNext(rhs);
		if unlikely(!ITER_ISOK(rhs_item)) {
			Dee_Decref(lhs_item);
			if (rhs_item == ITER_DONE)
				return 1; /* COUNT(LHS) > COUNT(RHS) */
			return Dee_COMPARE_ERR;
		}
		diff = DeeObject_Compare(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff != 0)
			return diff;
	}
	rhs_item = DeeObject_IterNext(rhs);
	if (!ITER_ISOK(rhs_item)) {
		if (rhs_item == ITER_DONE)
			return 0; /* COUNT(LHS) == COUNT(RHS) */
		return Dee_COMPARE_ERR;
	}
	Dee_Decref(rhs_item);
	return -1; /* COUNT(LHS) < COUNT(RHS) */
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_CompareFS(DeeObject *lhs, size_t lhsc, DeeObject *rhs) {
	int result;
	size_t rhs_size;
	if (DeeTuple_Check(rhs)) {
		result = DeeSeq_CompareFV(lhs, lhsc, DeeTuple_ELEM(rhs), DeeTuple_SIZE(rhs));
	} else if ((rhs_size = DeeFastSeq_GetSize_deprecated(rhs)) != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		result = DeeSeq_CompareFF(lhs, lhsc, rhs, rhs_size);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter)
			return Dee_COMPARE_ERR;
		result = DeeSeq_CompareFI(lhs, lhsc, rhs_iter);
		Dee_Decref(rhs_iter);
	}
	return result;
}


INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_CompareIV(DeeObject *lhs,
                 DeeObject *const *rhsv, size_t rhsc) {
	size_t i;
	DREF DeeObject *lhs_item;
	for (i = 0; i < rhsc; ++i) {
		int diff;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if (lhs_item == ITER_DONE)
				return -1; /* COUNT(LHS) < COUNT(RHS) */
			return Dee_COMPARE_ERR;
		}
		diff = DeeObject_Compare(lhs_item, rhsv[i]);
		Dee_Decref(lhs_item);
		if (diff != 0)
			return diff;
	}
	lhs_item = DeeObject_IterNext(lhs);
	if (!ITER_ISOK(lhs_item)) {
		if (lhs_item == ITER_DONE)
			return 0; /* COUNT(LHS) == COUNT(RHS) */
		return Dee_COMPARE_ERR;
	}
	Dee_Decref(lhs_item);
	return 1; /* COUNT(LHS) > COUNT(RHS) */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_CompareIF(DeeObject *lhs,
                 DeeObject *rhs, size_t rhsc) {
	size_t i;
	DREF DeeObject *lhs_item;
	for (i = 0; i < rhsc; ++i) {
		int diff;
		DREF DeeObject *rhs_item;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if (lhs_item == ITER_DONE)
				return -1; /* COUNT(LHS) < COUNT(RHS) */
			return Dee_COMPARE_ERR;
		}
		rhs_item = DeeFastSeq_GetItem_deprecated(rhs, i);
		if unlikely(!rhs_item) {
			Dee_Decref(lhs_item);
			return Dee_COMPARE_ERR;
		}
		diff = DeeObject_Compare(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff != 0)
			return diff;
	}
	lhs_item = DeeObject_IterNext(lhs);
	if (!ITER_ISOK(lhs_item)) {
		if (lhs_item == ITER_DONE)
			return 0; /* COUNT(LHS) == COUNT(RHS) */
		return Dee_COMPARE_ERR;
	}
	Dee_Decref(lhs_item);
	return 1; /* COUNT(LHS) > COUNT(RHS) */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_CompareII(DeeObject *lhs,
                 DeeObject *rhs) {
	for (;;) {
		int diff;
		DREF DeeObject *lhs_item;
		DREF DeeObject *rhs_item;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if unlikely(lhs_item != ITER_DONE)
				return Dee_COMPARE_ERR;
			rhs_item = DeeObject_IterNext(rhs);
			if (!ITER_ISOK(rhs_item)) {
				if unlikely(rhs_item != ITER_DONE)
					return Dee_COMPARE_ERR;
				return 0; /* COUNT(LHS) == COUNT(RHS) */
			}
			Dee_Decref(rhs_item);
			return -1; /* COUNT(LHS) < COUNT(RHS) */
		}
		rhs_item = DeeObject_IterNext(rhs);
		if unlikely(!ITER_ISOK(rhs_item)) {
			Dee_Decref(lhs_item);
			if unlikely(rhs_item != ITER_DONE)
				return Dee_COMPARE_ERR;
			return 1; /* COUNT(LHS) > COUNT(RHS) */
		}
		diff = DeeObject_Compare(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff != 0)
			return diff;
	}
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_CompareIS(DeeObject *lhs,
                 DeeObject *rhs) {
	int result;
	size_t rhs_size;
	if (DeeTuple_Check(rhs)) {
		result = DeeSeq_CompareIV(lhs, DeeTuple_ELEM(rhs), DeeTuple_SIZE(rhs));
	} else if ((rhs_size = DeeFastSeq_GetSize_deprecated(rhs)) != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		result = DeeSeq_CompareIF(lhs, rhs, rhs_size);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter)
			return Dee_COMPARE_ERR;
		result = DeeSeq_CompareII(lhs, rhs_iter);
		Dee_Decref(rhs_iter);
	}
	return result;
}



/* @return: == -1: An error occurred.
 * @return: == 0:  Sequences differ
 * @return: == 1:  Sequences are equal */
INTERN WUNUSED int DCALL
DeeSeq_EqVV(DeeObject *const *lhsv,
            DeeObject *const *rhsv, size_t elemc) {
	size_t i;
	for (i = 0; i < elemc; ++i) {
		int diff;
		diff = DeeObject_TryCmpEqAsBool(lhsv[i], rhsv[i]);
		if (diff <= 0)
			return diff;
	}
	return 1;
}

INTERN WUNUSED NONNULL((2)) int DCALL
DeeSeq_EqVF(DeeObject *const *lhsv,
            DeeObject *rhs, size_t elemc) {
	size_t i;
	for (i = 0; i < elemc; ++i) {
		int diff;
		DREF DeeObject *rhs_item;
		rhs_item = DeeFastSeq_GetItem_deprecated(rhs, i);
		if unlikely(!rhs_item)
			return -1;
		diff = DeeObject_TryCmpEqAsBool(lhsv[i], rhs_item);
		Dee_Decref(rhs_item);
		if (diff <= 0)
			return diff;
	}
	return 1;
}

INTERN WUNUSED NONNULL((3)) int DCALL
DeeSeq_EqVI(DeeObject *const *lhsv, size_t lhsc,
            DeeObject *rhs) {
	size_t i;
	DREF DeeObject *rhs_item;
	for (i = 0; i < lhsc; ++i) {
		int diff;
		rhs_item = DeeObject_IterNext(rhs);
		if unlikely(!ITER_ISOK(rhs_item)) {
			if (rhs_item == ITER_DONE)
				return 0; /* COUNT(LHS) > COUNT(RHS) */
			return -1;
		}
		diff = DeeObject_TryCmpEqAsBool(lhsv[i], rhs_item);
		Dee_Decref(rhs_item);
		if (diff <= 0)
			return diff;
	}
	rhs_item = DeeObject_IterNext(rhs);
	if (!ITER_ISOK(rhs_item)) {
		if (rhs_item == ITER_DONE)
			return 1; /* COUNT(LHS) == COUNT(RHS) */
		return -1;
	}
	Dee_Decref(rhs_item);
	return 0; /* COUNT(LHS) < COUNT(RHS) */
}

INTERN WUNUSED NONNULL((3)) int DCALL
DeeSeq_EqVS(DeeObject *const *lhsv, size_t lhsc,
            DeeObject *rhs) {
	int result;
	size_t rhs_size;
	if (DeeTuple_Check(rhs)) {
		if (lhsc != DeeTuple_SIZE(rhs))
			return 0;
		result = DeeSeq_EqVV(lhsv, DeeTuple_ELEM(rhs), lhsc);
	} else if ((rhs_size = DeeFastSeq_GetSize_deprecated(rhs)) != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		if (lhsc != rhs_size)
			return 0;
		result = DeeSeq_EqVF(lhsv, rhs, rhs_size);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter) {
			if (DeeError_Catch(&DeeError_NotImplemented))
				return 0;
			return -1;
		}
		result = DeeSeq_EqVI(lhsv, lhsc, rhs_iter);
		Dee_Decref(rhs_iter);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_EqFV(DeeObject *lhs,
            DeeObject *const *rhsv, size_t elemc) {
	size_t i;
	for (i = 0; i < elemc; ++i) {
		int diff;
		DREF DeeObject *lhs_item;
		lhs_item = DeeFastSeq_GetItem_deprecated(lhs, i);
		if unlikely(!lhs_item)
			return -1;
		diff = DeeObject_TryCmpEqAsBool(lhs_item, rhsv[i]);
		Dee_Decref(lhs_item);
		if (diff <= 0)
			return diff;
	}
	return 1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_EqFF(DeeObject *lhs, DeeObject *rhs, size_t elemc) {
	size_t i;
	for (i = 0; i < elemc; ++i) {
		int diff;
		DREF DeeObject *lhs_item;
		DREF DeeObject *rhs_item;
		lhs_item = DeeFastSeq_GetItem_deprecated(lhs, i);
		if unlikely(!lhs_item)
			return -1;
		rhs_item = DeeFastSeq_GetItem_deprecated(rhs, i);
		if unlikely(!rhs_item) {
			Dee_Decref(lhs_item);
			return -1;
		}
		diff = DeeObject_TryCmpEqAsBool(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff <= 0)
			return diff;
	}
	return 1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_EqFI(DeeObject *lhs, size_t lhsc, DeeObject *rhs) {
	size_t i;
	DREF DeeObject *rhs_item;
	for (i = 0; i < lhsc; ++i) {
		DREF DeeObject *lhs_item;
		int diff;
		lhs_item = DeeFastSeq_GetItem_deprecated(lhs, i);
		if unlikely(!lhs_item)
			return -1;
		rhs_item = DeeObject_IterNext(rhs);
		if unlikely(!ITER_ISOK(rhs_item)) {
			Dee_Decref(lhs_item);
			if (rhs_item == ITER_DONE)
				return 0; /* COUNT(LHS) > COUNT(RHS) */
			return -1;
		}
		diff = DeeObject_TryCmpEqAsBool(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff <= 0)
			return diff;
	}
	rhs_item = DeeObject_IterNext(rhs);
	if (!ITER_ISOK(rhs_item)) {
		if (rhs_item == ITER_DONE)
			return 1; /* COUNT(LHS) == COUNT(RHS) */
		return -1;
	}
	Dee_Decref(rhs_item);
	return 0; /* COUNT(LHS) < COUNT(RHS) */
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_EqFS(DeeObject *lhs, size_t lhsc, DeeObject *rhs) {
	int result;
	size_t rhs_size;
	if (DeeTuple_Check(rhs)) {
		if (DeeTuple_SIZE(rhs) != lhsc)
			return 0;
		result = DeeSeq_EqFV(lhs, DeeTuple_ELEM(rhs), lhsc);
	} else if ((rhs_size = DeeFastSeq_GetSize_deprecated(rhs)) != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		if (rhs_size != lhsc)
			return 0;
		result = DeeSeq_EqFF(lhs, rhs, rhs_size);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter) {
			if (DeeError_Catch(&DeeError_NotImplemented))
				return 0;
			return -1;
		}
		result = DeeSeq_EqFI(lhs, lhsc, rhs_iter);
		Dee_Decref(rhs_iter);
	}
	return result;
}


INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_EqIV(DeeObject *lhs,
            DeeObject *const *rhsv, size_t rhsc) {
	size_t i;
	DREF DeeObject *lhs_item;
	for (i = 0; i < rhsc; ++i) {
		int diff;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if (lhs_item == ITER_DONE)
				return 0; /* COUNT(LHS) < COUNT(RHS) */
			return -1;
		}
		diff = DeeObject_TryCmpEqAsBool(lhs_item, rhsv[i]);
		Dee_Decref(lhs_item);
		if (diff <= 0)
			return diff;
	}
	lhs_item = DeeObject_IterNext(lhs);
	if (!ITER_ISOK(lhs_item)) {
		if (lhs_item == ITER_DONE)
			return 1; /* COUNT(LHS) == COUNT(RHS) */
		return -1;
	}
	Dee_Decref(lhs_item);
	return 0; /* COUNT(LHS) > COUNT(RHS) */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_EqIF(DeeObject *lhs,
            DeeObject *rhs, size_t rhsc) {
	size_t i;
	DREF DeeObject *lhs_item;
	for (i = 0; i < rhsc; ++i) {
		int diff;
		DREF DeeObject *rhs_item;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if (lhs_item == ITER_DONE)
				return 0; /* COUNT(LHS) < COUNT(RHS) */
			return -1;
		}
		rhs_item = DeeFastSeq_GetItem_deprecated(rhs, i);
		if unlikely(!rhs_item) {
			Dee_Decref(lhs_item);
			return -1;
		}
		diff = DeeObject_TryCmpEqAsBool(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff <= 0)
			return diff;
	}
	lhs_item = DeeObject_IterNext(lhs);
	if (!ITER_ISOK(lhs_item)) {
		if (lhs_item == ITER_DONE)
			return 1; /* COUNT(LHS) == COUNT(RHS) */
		return -1;
	}
	Dee_Decref(lhs_item);
	return 0; /* COUNT(LHS) > COUNT(RHS) */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_EqII(DeeObject *lhs, DeeObject *rhs) {
	for (;;) {
		int diff;
		DREF DeeObject *lhs_item;
		DREF DeeObject *rhs_item;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if unlikely(lhs_item != ITER_DONE)
				return -1;
			rhs_item = DeeObject_IterNext(rhs);
			if (!ITER_ISOK(rhs_item)) {
				if unlikely(rhs_item != ITER_DONE)
					return -1;
				return 1; /* COUNT(LHS) == COUNT(RHS) */
			}
			Dee_Decref(rhs_item);
			return 0; /* COUNT(LHS) < COUNT(RHS) */
		}
		rhs_item = DeeObject_IterNext(rhs);
		if unlikely(!ITER_ISOK(rhs_item)) {
			Dee_Decref(lhs_item);
			if unlikely(rhs_item != ITER_DONE)
				return -1;
			return 0; /* COUNT(LHS) > COUNT(RHS) */
		}
		diff = DeeObject_TryCmpEqAsBool(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff <= 0)
			return diff;
	}
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_EqIS(DeeObject *lhs, DeeObject *rhs) {
	int result;
	size_t rhs_size;
	if (DeeTuple_Check(rhs)) {
		result = DeeSeq_EqIV(lhs, DeeTuple_ELEM(rhs), DeeTuple_SIZE(rhs));
	} else if ((rhs_size = DeeFastSeq_GetSize_deprecated(rhs)) != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		result = DeeSeq_EqIF(lhs, rhs, rhs_size);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter) {
			if (DeeError_Catch(&DeeError_NotImplemented))
				return 0;
			return -1;
		}
		result = DeeSeq_EqII(lhs, rhs_iter);
		Dee_Decref(rhs_iter);
	}
	return result;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_C */
