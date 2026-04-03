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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_SEGMENTS_H
#define GUARD_DEEMON_OBJECTS_UNICODE_SEGMENTS_H 1

#include <deemon/api.h>

#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject */
#include <deemon/string.h> /* DeeStringObject */

#include "../generic-proxy.h"

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint8_t */

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeStringObject, s_str)   /* [1..1][const] The string that is being segmented. */
	size_t                                s_siz;   /* [!0][const] The size of a single segment. */
	DWEAK uint8_t                        *s_ptr;   /* [1..1][in(DeeString_WSTR(s_str))] Pointer to the start of the next segment. */
	uint8_t                              *s_end;   /* [1..1][== DeeString_WEND(s_str)] End pointer. */
	unsigned int                          s_width; /* [const] The width of a single character. */
} StringSegmentsIterator;

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeStringObject, s_str) /* [1..1][const] The string that is being segmented. */
	size_t                                s_siz; /* [!0][const] The size of a single segment. */
} StringSegments;

INTDEF DeeTypeObject StringSegmentsIterator_Type;
INTDEF DeeTypeObject StringSegments_Type;
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_Segments(DeeStringObject *__restrict self,
                   size_t segment_size);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_SEGMENTS_H */
