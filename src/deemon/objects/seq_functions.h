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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_H
#define GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_H 1

#include <deemon/api.h>
#include <deemon/object.h>

DECL_BEGIN

#ifndef CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS
/* TODO: All of this stuff here breaks when used on types with multiple bases.
 *
 * Solution:
 * - Instead of implementing (e.g.) "Sequence.find()" to check how it should be
 *   implemented every time it is used on a type, types should have a pointer to
 *   a NSI implementation cache region that contains function pointers describing
 *   how a specific NSI function should be used with a given type.
 * - The first time a function is then called, it should figure out how to implement
 *   itself, and then store a method pointer in the cache that will then simply be
 *   called as-is the next time the function is called.
 *
 * For reference, see the way `Sequence.operator iter()' operates.
 */
#define has_noninherited_seqfield(tp, seq, field)       \
	((seq)->field != NULL &&                            \
	 (!DeeType_Base(tp) || !DeeType_Base(tp)->tp_seq || \
	  DeeType_Base(tp)->tp_seq->field != (seq)->field))

#define has_noninherited_field(tp, field) \
	((tp)->field != NULL &&               \
	 (!DeeType_Base(tp) || DeeType_Base(tp)->field != (tp)->field))

#define is_noninherited_nsi(tp, seq, nsi) \
	(!DeeType_Base(tp) || DeeType_Base(tp)->tp_seq != (seq))

#define has_noninherited_getrange(tp, seq) has_noninherited_seqfield(tp, seq, tp_getrange)
#define has_noninherited_delrange(tp, seq) has_noninherited_seqfield(tp, seq, tp_delrange)
#define has_noninherited_setrange(tp, seq) has_noninherited_seqfield(tp, seq, tp_setrange)
#define has_noninherited_getitem(tp, seq)  has_noninherited_seqfield(tp, seq, tp_getitem)
#define has_noninherited_delitem(tp, seq)  has_noninherited_seqfield(tp, seq, tp_delitem)
#define has_noninherited_setitem(tp, seq)  has_noninherited_seqfield(tp, seq, tp_setitem)
#define has_noninherited_size(tp, seq)     has_noninherited_seqfield(tp, seq, tp_sizeob)
#define has_noninherited_bool(tp)          has_noninherited_field(tp, tp_cast.tp_bool)

/* Mutable-sequence API */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DelItem(DeeObject *__restrict self, size_t index);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_SetItem(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_XchItem(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DelRange(DeeObject *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_SetRange(DeeObject *self, size_t start, size_t end, DeeObject *values);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DelRangeN(DeeObject *__restrict self, size_t start);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_SetRangeN(DeeObject *self, size_t start, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_Insert(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_InsertAll(DeeObject *self, size_t index, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_Append(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_Extend(DeeObject *self, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_InplaceExtend(DREF DeeObject **__restrict p_self, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_InplaceRepeat(DREF DeeObject **__restrict p_self, DeeObject *count);
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_Erase(DeeObject *__restrict self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_PopItem(DeeObject *__restrict self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_Remove(DeeObject *self, size_t start, size_t end, DeeObject *elem, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_RRemove(DeeObject *self, size_t start, size_t end, DeeObject *elem, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL DeeSeq_RemoveAll(DeeObject *self, size_t start, size_t end, DeeObject *elem, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL DeeSeq_RemoveIf(DeeObject *self, size_t start, size_t end, DeeObject *should);
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL DeeSeq_Fill(DeeObject *self, size_t start, size_t end, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_Reverse(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_Sort(DeeObject *self, DeeObject *key);

/* Determine if a given sequence is mutable or resizable.
 * @return: 1:  The sequence is mutable or resizable.
 * @return: 0:  The sequence isn't mutable or resizable.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_IsMutable(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_IsResizable(DeeObject *__restrict self);

/* NOTE: Technically, all of these functions can be used on any type of object,
 *       but all objects derived from `DeeSeq_Type' automatically implement
 *       all of them as member functions.
 *       With that in mind, any type implementing the `tp_seq' interface
 *       with the intention of behaving as an Iterable, should probably
 *       be derived from `DeeSeq_Type' as this allows usercode to query
 *       for a general purpose sequence by writing `x is Sequence from deemon' */
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_Size(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_GetItem(DeeObject *__restrict self, size_t index);
#endif /* !CONFIG_EXPERIMENTAL_NEW_SEQUENCE_OPERATORS */


/* TODO: All of the following also needs to go eventually... */


/* Return the sequence associated with the iterator, or NULL on error.
 * NOTE: Alternatively, a getset/member `seq' may be defined for this. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeIterator_GetSeq(DeeObject *__restrict self);

/* Get the iterator's position
 * @return: * :         The iterator's current position, where the a starting position is 0
 * @return: (size_t)-2: The position is indeterminate (the iterator may have become detached
 *                      from its sequence, as can happen in linked lists when the iterator's
 *                      link entry gets removed)
 * @return: (size_t)-1: Error */
INTDEF WUNUSED NONNULL((1)) size_t DCALL
DeeIterator_GetIndex(DeeObject *__restrict self);

/* Set the iterator's position
 * If the given `new_index' is greater than the max allowed index,
 * the iterator is set to an exhausted state (i.e. points at the
 * end of the associated sequence)
 * @return:  0: Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeIterator_SetIndex(DeeObject *__restrict self, size_t new_index);

/* Rewind the iterator to its starting position
 * @return:  0: Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeIterator_Rewind(DeeObject *__restrict self);

/* Revert the iterator by at most `step' (When `step' is too large, same as `rewind')
 * @return:  0: Success (new relative position wasn't determined)
 * @return:  1: Success (the iterator has reached its starting position)
 * @return:  2: Success (the iterator hasn't reached its starting position)
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeIterator_Revert(DeeObject *__restrict self, size_t step);

/* Advance the iterator by at most `step' (When `step' is too large, exhaust the iterator)
 * @return:  0: Success (new relative position wasn't determined)
 * @return:  1: Success (the iterator has become exhausted)
 * @return:  2: Success (the iterator hasn't become exhausted)
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeIterator_Advance(DeeObject *__restrict self, size_t step);

/* Decrement the iterator by 1.
 * @return:  0: Success
 * @return:  1: The iterator was already at its starting location
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeIterator_Prev(DeeObject *__restrict self);

/* Increment the iterator, but don't generate a value
 * NOTE: Unlike `tp_iter_next()', this operator shouldn't skip unbound entries,
 *       meaning that (also unlike `tp_iter_next()'), the iterator's index should
 *       only ever be incremented by 1.
 * @return:  0: Success
 * @return:  1: The iterator had already been exhausted
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeIterator_Next(DeeObject *__restrict self);

/* Check if the iterator is at its starting location
 * @return:  0: No, it isn't
 * @return:  1: Yes, it is
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1)) int DCALL
DeeIterator_HasPrev(DeeObject *__restrict self);

/* Peek the next iterator value, but don't actually advance the iterator.
 * @return: ITER_DONE: The iterator has already been exhausted. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeIterator_Peek(DeeObject *__restrict self);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_H */
