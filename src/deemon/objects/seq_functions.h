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
