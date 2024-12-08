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

DECL_BEGIN

typedef struct {
	OBJECT_HEAD /* GC Object */
	DREF DeeObject                     *ui_iter;        /* [1..1][const] Underlying iterator */
	/* [1..1][const] Callback to load the next item from `ui_iter'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *ui_tp_next)(DeeObject *self);
	struct Dee_simple_hashset_with_lock ui_encountered; /* Set of objects previously encountered objects */
} UniqueIterator;

INTDEF DeeTypeObject UniqueIterator_Type;

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_UNIQUE_ITERATOR_H */
