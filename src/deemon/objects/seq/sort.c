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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SORT_C
#define GUARD_DEEMON_OBJECTS_SEQ_SORT_C 1

#include "sort.h"

#ifndef __INTELLISENSE__
#define DEFINE_DeeSeq_SortVector
#include "sort-impl.c.inl"
#define DEFINE_DeeSeq_SortVectorWithKey
#include "sort-impl.c.inl"
#define DEFINE_DeeSeq_SortGetItemIndexFast
#include "sort-impl.c.inl"
#define DEFINE_DeeSeq_SortGetItemIndexFastWithKey
#include "sort-impl.c.inl"
#define DEFINE_DeeSeq_SortTryGetItemIndex
#include "sort-impl.c.inl"
#define DEFINE_DeeSeq_SortTryGetItemIndexWithKey
#include "sort-impl.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SORT_C */
