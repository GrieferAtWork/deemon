/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_HASHFILTER_H
#define GUARD_DEEMON_OBJECTS_SEQ_HASHFILTER_H 1

#include <deemon/api.h>
#include <deemon/object.h>

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *fi_iter; /* [1..1][const] The iterator who's elements are being filtered. */
	dhash_t         fi_hash; /* [const] The hash for which we're filtering. */
} HashFilterIterator;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *f_seq;  /* [1..1][const] The sequence being filtered. */
	dhash_t         f_hash; /* [const] The hash for which we're filtering. */
} HashFilter;

INTDEF DeeTypeObject SeqHashFilter_Type;
INTDEF DeeTypeObject SeqHashFilterIterator_Type;

INTDEF DeeTypeObject MapHashFilter_Type;
INTDEF DeeTypeObject MapHashFilterIterator_Type;

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_HASHFILTER_H */
