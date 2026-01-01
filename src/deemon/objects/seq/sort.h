/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SORT_H
#define GUARD_DEEMON_OBJECTS_SEQ_SORT_H 1

#include <deemon/api.h>
#include <deemon/object.h>
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

/* Generic sorting functions */
INTDEF WUNUSED ATTR_OUTS(2, 1) ATTR_INS(3, 1) int DCALL DeeSeq_SortVector(size_t objc, DREF DeeObject **__restrict dst, /*inherit(on_success)*/ DREF DeeObject *const *__restrict src); /* source elements are never unbound */
INTDEF WUNUSED ATTR_OUTS(2, 1) ATTR_INS(3, 1) NONNULL((4)) int DCALL DeeSeq_SortVectorWithKey(size_t objc, DREF DeeObject **__restrict dst, /*inherit(on_success)*/ DREF DeeObject *const *__restrict src, DeeObject *key); /* source elements are never unbound */
INTDEF WUNUSED ATTR_OUTS(2, 1) NONNULL((3, 5)) int DCALL DeeSeq_SortGetItemIndexFast(size_t objc, DREF DeeObject **__restrict dst, DeeObject *src, size_t src_start, DREF DeeObject *(DCALL *src_getitem_index_fast)(DeeObject *__restrict self, size_t index));
INTDEF WUNUSED ATTR_OUTS(2, 1) NONNULL((3, 5, 6)) int DCALL DeeSeq_SortGetItemIndexFastWithKey(size_t objc, DREF DeeObject **__restrict dst, DeeObject *src, size_t src_start, DREF DeeObject *(DCALL *src_getitem_index_fast)(DeeObject *__restrict self, size_t index), DeeObject *key);
INTDEF WUNUSED ATTR_OUTS(2, 1) NONNULL((3, 5)) int DCALL DeeSeq_SortTryGetItemIndex(size_t objc, DREF DeeObject **__restrict dst, DeeObject *src, size_t src_start, DREF DeeObject *(DCALL *src_trygetitem_index)(DeeObject *__restrict self, size_t index));
INTDEF WUNUSED ATTR_OUTS(2, 1) NONNULL((3, 5, 6)) int DCALL DeeSeq_SortTryGetItemIndexWithKey(size_t objc, DREF DeeObject **__restrict dst, DeeObject *src, size_t src_start, DREF DeeObject *(DCALL *src_trygetitem_index)(DeeObject *__restrict self, size_t index), DeeObject *key);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SORT_H */
