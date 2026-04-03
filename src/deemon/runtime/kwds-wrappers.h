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
#ifndef GUARD_DEEMON_RUNTIME_KWDS_WRAPPERS_H
#define GUARD_DEEMON_RUNTIME_KWDS_WRAPPERS_H 1

#include <deemon/api.h>

#include <deemon/kwds.h>   /* DeeBlackListKwObject, DeeBlackListKwdsObject, Dee_kwds_entry */
#include <deemon/object.h> /* DeeObject, DeeTypeObject */

#include "../objects/generic-proxy.h"


DECL_BEGIN

/* ======================== DeeBlackListKwdsObject ======================== */
typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeBlackListKwdsObject, blki_map); /* [1..1][const] The associated keywords mapping. */
	struct Dee_kwds_entry                       *blki_iter; /* [1..1][lock(ATOMIC)] The next entry to iterate. */
	struct Dee_kwds_entry                       *blki_end;  /* [1..1][const] Pointer to the end of the associated keywords table. */
} DeeBlackListKwdsIterator;

INTDEF DeeTypeObject DeeBlackListKwdsIterator_Type;


/* ======================== DeeBlackListKwObject ======================== */
typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeObject,            mi_iter, /* [1..1][const] An iterator for the underlying `mi_map->blkw_kw'. */
	                      DeeBlackListKwObject, mi_map); /* [1..1][const] The general-purpose blacklist mapping being iterated. */
} DeeBlackListKwIterator;

INTDEF DeeTypeObject DeeBlackListKwIterator_Type;

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_KWDS_WRAPPERS_H */
