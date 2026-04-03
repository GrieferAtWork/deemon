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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_FINDER_H
#define GUARD_DEEMON_OBJECTS_UNICODE_FINDER_H 1

#include <deemon/api.h>

#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject */
#include <deemon/string.h> /* DeeStringObject, Dee_charptr_const */

#include "../generic-proxy.h"

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeStringObject, sf_str,    /* [1..1][const] The string that is being searched. */
	                      DeeStringObject, sf_needle) /* [1..1][const] The needle being searched for. */
	size_t                        sf_start;  /* [const] Starting search index. */
	size_t                        sf_end;    /* [const] End search index. */
	bool                          sf_ovrlap; /* [const] When true, "sfi_find_delta = 1", else "sfi_find_delta = max(#sf_needle, 1)" */
} StringFind;

typedef struct {
	PROXY_OBJECT_HEAD_EX(StringFind, sfi_find)       /* [1..1][const] The underlying find-controller. */
	union Dee_charptr_const          sfi_start;      /* [1..1][const] Starting pointer. */
	DWEAK union Dee_charptr_const    sfi_ptr;        /* [1..1] Pointer to the start of data left to be searched. */
	union Dee_charptr_const          sfi_end;        /* [1..1][const] End pointer. */
	union Dee_charptr_const          sfi_needle_ptr; /* [1..1][const] Starting pointer of the needle being searched. */
	size_t                           sfi_needle_len; /* [const] Length of the needle being searched. */
	size_t                           sfi_find_delta; /* [const] Delta added to `sfi_ptr' after each match */
	unsigned int                     sfi_width;      /* [const] The common width of the searched, and needle string. */
} StringFindIterator;

INTDEF DeeTypeObject StringFindIterator_Type;
INTDEF DeeTypeObject StringFind_Type;
INTDEF DeeTypeObject StringCaseFindIterator_Type;
INTDEF DeeTypeObject StringCaseFind_Type;

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_FindAll(DeeStringObject *self, DeeStringObject *other,
                  size_t start, size_t end, bool overlapping);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_CaseFindAll(DeeStringObject *self, DeeStringObject *other,
                      size_t start, size_t end, bool overlapping);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_FINDER_H */
