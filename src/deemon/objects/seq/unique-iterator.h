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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_UNIQUE_ITERATOR_H
#define GUARD_DEEMON_OBJECTS_SEQ_UNIQUE_ITERATOR_H 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/gc.h>
#include <deemon/object.h>
#include <deemon/util/simple-hashset.h>

#include "../generic-proxy.h"

DECL_BEGIN

typedef struct {
	OBJECT_HEAD /* GC Object */
	DREF DeeObject                     *di_iter;        /* [1..1][const] Underlying iterator */
	/* [1..1][const] Callback to load the next item from `di_iter'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *di_tp_next)(DeeObject *self);
	struct Dee_simple_hashset_with_lock di_encountered; /* Set of objects previously encountered objects */
} DistinctIterator;

INTDEF DeeTypeObject DistinctIterator_Type;

typedef struct {
	OBJECT_HEAD /* GC Object */
	DREF DeeObject                     *diwk_iter;        /* [1..1][const] Underlying iterator */
	/* [1..1][const] Callback to load the next item from `diwk_iter'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *diwk_tp_next)(DeeObject *self);
	struct Dee_simple_hashset_with_lock diwk_encountered; /* Set of objects previously encountered objects */
	DREF DeeObject                     *diwk_key;         /* [1..1][const] unique-ness filter keys */
} DistinctIteratorWithKey;

INTDEF DeeTypeObject DistinctIteratorWithKey_Type;

typedef struct {
	PROXY_OBJECT_HEAD2(dswk_seq, /* [1..1] Sequence being proxied */
	                   dswk_key) /* [1..1] Key to apply to sequence elements prior to uniqueness-check */
} DistinctSetWithKey;

INTDEF DeeTypeObject DistinctSetWithKey_Type;

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_UNIQUE_ITERATOR_H */
