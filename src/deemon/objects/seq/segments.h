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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SEGMENTS_H
#define GUARD_DEEMON_OBJECTS_SEQ_SEGMENTS_H 1

#include <deemon/api.h>
#include <deemon/object.h>
/**/

#include "../generic-proxy.h"
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD(s_seq) /* [1..1][const] The underlying sequence that is being segmented. */
	size_t            s_len; /* [const][!0] The (max) length of a single segment. */
} Segments;

typedef struct {
	PROXY_OBJECT_HEAD(si_iter) /* [1..1][const] An iterator for the sequence being segmented. */
	size_t            si_len;  /* [const][!0] The (max) length of a single segment. */
} SegmentsIterator;

INTDEF DeeTypeObject SeqSegmentsIterator_Type;
INTDEF DeeTypeObject SeqSegments_Type;

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_Segments(DeeObject *__restrict self, size_t segsize);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SEGMENTS_H */
