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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FINDER_H
#define GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FINDER_H 1

#include <deemon/api.h>

#include <deemon/bytes.h>  /* DeeBytesObject */
#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject */

#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "../generic-proxy.h"
#include "bytes_needle.h"

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeBytesObject, bf_bytes,  /* [1..1][const] The bytes that is being searched. */
	                      DeeObject,      bf_other); /* [1..1][const] The needle object. */
	Needle                                bf_needle; /* [const] The needle being searched for. */
	byte_t const                         *bf_start;  /* [1..1][const] Starting pointer. */
	byte_t const                         *bf_end;    /* [1..1][const] End pointer. */
	bool                                  bf_ovrlap; /* [const] When true, "bfi_find_delta = 1", else "bfi_find_delta = max(bf_needle.n_size, 1)" */
} BytesFind;

typedef struct {
	PROXY_OBJECT_HEAD_EX(BytesFind, bfi_find)       /* [1..1][const] The underlying find-controller. */
	byte_t const                   *bfi_start;      /* [1..1][const] Starting pointer. */
	DWEAK byte_t const             *bfi_ptr;        /* [1..1] Pointer to the start of data left to be searched. */
	byte_t const                   *bfi_end;        /* [1..1][const] End pointer. */
	byte_t const                   *bfi_needle_ptr; /* [1..1][const] Starting pointer of the needle being searched. */
	size_t                          bfi_needle_len; /* [const] Length of the needle being searched. */
	size_t                          bfi_find_delta; /* [const] Delta added to `sfi_ptr' after each match */
} BytesFindIterator;

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_FindAll(DeeBytesObject *self, DeeObject *needle,
                 size_t start, size_t end,
                 bool overlapping);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_CaseFindAll(DeeBytesObject *self, DeeObject *needle,
                     size_t start, size_t end,
                     bool overlapping);

INTDEF DeeTypeObject BytesFindIterator_Type;
INTDEF DeeTypeObject BytesFind_Type;
INTDEF DeeTypeObject BytesCaseFindIterator_Type;
INTDEF DeeTypeObject BytesCaseFind_Type;

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FINDER_H */
