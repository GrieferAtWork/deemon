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
#ifndef GUARD_DEEMON_RUNTIME_KWDS_H
#define GUARD_DEEMON_RUNTIME_KWDS_H 1

#include <deemon/api.h>

#include <deemon/kwds.h>   /* DeeKwdsMappingObject, DeeKwdsObject, Dee_kwds_entry */
#include <deemon/object.h> /* DREF, DeeTypeObject, OBJECT_HEAD */

#include "../objects/generic-proxy.h"

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeKwdsObject, ki_map); /* [1..1][const] The associated keywords table. */
	struct Dee_kwds_entry              *ki_iter; /* [1..1][lock(ATOMIC)] The next entry to iterate. */
	struct Dee_kwds_entry              *ki_end;  /* [1..1][const] Pointer to the end of the associated keywords table. */
} KwdsIterator;

INTDEF DeeTypeObject DeeKwdsIterator_Type;


typedef struct {
	OBJECT_HEAD
	DREF DeeKwdsMappingObject   *ki_map;  /* [1..1][const] The associated keywords mapping. */
	DWEAK struct Dee_kwds_entry *ki_iter; /* [1..1] The next entry to iterate. */
	struct Dee_kwds_entry       *ki_end;  /* [1..1][const] Pointer to the end of the associated keywords table. */
} KmapIterator;

INTDEF DeeTypeObject DeeKwdsMappingIterator_Type;

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_KWDS_H */
