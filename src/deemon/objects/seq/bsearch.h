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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_BSEARCH_H
#define GUARD_DEEMON_OBJECTS_SEQ_BSEARCH_H 1

#include <deemon/api.h>
#include <deemon/object.h>

DECL_BEGIN

/* Binary search for `keyed_search_item'
 * In case multiple elements match `keyed_search_item', the returned index
 * will be that for one of them, though it is undefined which one specifically.
 * @return: (size_t)-1: Not found.
 * @return: (size_t)-2: Error. */
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL
DeeSeq_BFind(DeeObject *self, size_t start, size_t end,
             DeeObject *keyed_search_item, DeeObject *key);

/* Find the index-range of all items, such that:
 * >> for (elem: self[*p_startindex:*p_endindex])
 * >>     assert keyed_search_item == key(elem);
 * @return: 0: Success
 * @return: -1: Error. */
INTDEF WUNUSED NONNULL((1, 4, 6, 7)) int DCALL
DeeSeq_BFindRange(DeeObject *self, size_t start, size_t end,
                  DeeObject *keyed_search_item, DeeObject *key,
                  size_t *__restrict p_startindex,
                  size_t *__restrict p_endindex);

/* Same as `DeeSeq_BFind()', but return index where `keyed_search_item'
 * should go in case no matching item already exists in `self'
 * @return: (size_t)-1: Error. */
INTDEF WUNUSED NONNULL((1, 4)) size_t DCALL
DeeSeq_BFindPosition(DeeObject *self, size_t start, size_t end,
                     DeeObject *keyed_search_item, DeeObject *key);

/* Returns `self[DeeSeq_BFind(self, keyed_search_item, key)]'
 * In case multiple elements match `keyed_search_item', the
 * returned item will be one of them, though which one is
 * undefined.
 * @return: NULL: Error, or not found (and `defl' is NULL). */
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
DeeSeq_BLocate(DeeObject *self, size_t start, size_t end,
               DeeObject *keyed_search_item, DeeObject *key,
               DeeObject *defl);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_BSEARCH_H */
