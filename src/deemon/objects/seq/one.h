/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_ONE_H
#define GUARD_DEEMON_OBJECTS_SEQ_ONE_H 1

#include <deemon/api.h>
#include <deemon/seq.h>

DECL_BEGIN

typedef struct {
	Dee_OBJECT_HEAD
	DREF DeeObject *soi_item; /* [0..1][lock(ATOMIC && CLEAR_ONCE)]
	                           * The item to yield (or "ITER_DONE" if that already
	                           * happened, or "NULL" if temporarily locked) */
} SeqOneIterator;

INTDEF DeeTypeObject SeqOneIterator_Type;

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_ONE_H */
