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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_BYTES_SPLIT_H
#define GUARD_DEEMON_OBJECTS_UNICODE_BYTES_SPLIT_H 1

#include <deemon/api.h>

#include <deemon/bytes.h>  /* DeeBytesObject */
#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject */

#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "../generic-proxy.h"

#include <stddef.h> /* size_t */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

INTDEF DeeTypeObject BytesSplitIterator_Type;
INTDEF DeeTypeObject BytesSplit_Type;
INTDEF DeeTypeObject BytesCaseSplitIterator_Type;
INTDEF DeeTypeObject BytesCaseSplit_Type;
INTDEF DeeTypeObject BytesLineSplitIterator_Type;
INTDEF DeeTypeObject BytesLineSplit_Type;

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeBytesObject, bs_bytes,     /* [1..1][const] The Bytes object being split. */
	                      DeeObject,      bs_sep_owner) /* [0..1][const] The owner of the split sequence. */
	byte_t const                         *bs_sep_ptr;   /* [const] Pointer to the effective separation sequence. */
	size_t                                bs_sep_len;   /* [const][!0] Length of the separation sequence (in bytes). */
	byte_t                                bs_sep_buf[sizeof(void *)]; /* A small inline-buffer used for single-byte splits. */
} BytesSplit;

typedef struct {
	PROXY_OBJECT_HEAD_EX(BytesSplit, bsi_split)    /* [1..1][const] The underlying split controller. */
	DWEAK byte_t const              *bsi_iter;     /* [0..1] Pointer to the start of the next split (When NULL, iteration is complete). */
	byte_t const                    *bsi_end;      /* [1..1][== DeeBytes_END(bsi_split->bs_bytes)] Pointer to the end of input data. */
	DeeBytesObject                  *bsi_bytes;    /* [1..1][const][== bsi_split->bs_bytes] The Bytes object being split. */
	byte_t const                    *bsi_sep_ptr;  /* [const][== bsi_split->bs_sep_ptr] Pointer to the effective separation sequence. */
	size_t                           bsi_sep_len;  /* [const][== bsi_split->bs_sep_len] Length of the separation sequence (in bytes). */
} BytesSplitIterator;


INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_SplitByte(DeeBytesObject *__restrict self, byte_t sep);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_Split(DeeBytesObject *self, DeeObject *sep);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_CaseSplitByte(DeeBytesObject *__restrict self, byte_t sep);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeBytes_CaseSplit(DeeBytesObject *self, DeeObject *sep);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_SplitLines(DeeBytesObject *__restrict self, bool keepends);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_BYTES_SPLIT_H */
