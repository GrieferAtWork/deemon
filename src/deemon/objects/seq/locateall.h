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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_LOCATE_ALL_H
#define GUARD_DEEMON_OBJECTS_SEQ_LOCATE_ALL_H 1

#include <deemon/api.h>
#include <deemon/object.h>

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *l_seq;  /* [1..1][const] The sequence being transformed. */
	DREF DeeObject *l_elem; /* [1..1][const] The element being searched. */
	DREF DeeObject *l_pred; /* [0..1][const] The key function invoked to transform elements. */
} Locator;


typedef struct {
	OBJECT_HEAD
	DREF DeeObject *li_iter; /* [1..1][const] The iterator in which items are being located. */
	DREF DeeObject *li_elem; /* [1..1][const] The element being searched. */
	DREF DeeObject *li_pred; /* [0..1][const] The key function invoked to transform elements. */
} LocatorIterator;

INTDEF DeeTypeObject SeqLocator_Type;
INTDEF DeeTypeObject SeqLocatorIterator_Type;

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_LocateAll(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_LOCATE_ALL_H */
