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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_H
#define GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_H 1

#include <deemon/api.h>

#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject */

#include "../generic-proxy.h"

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD(sp_seq) /* [1..1][const] The underlying sequence. */
} SeqSimpleProxy;

typedef struct {
	PROXY_OBJECT_HEAD(si_iter) /* [1..1][const] The underlying iterator. */
} SeqSimpleProxyIterator;


INTDEF DeeTypeObject SeqIds_Type;             /* Sequence.Ids */
INTDEF DeeTypeObject SeqTypes_Type;           /* Sequence.Types */
INTDEF DeeTypeObject SeqClasses_Type;         /* Sequence.Classes */
INTDEF DeeTypeObject SeqIdsIterator_Type;     /* Sequence.Ids.Iterator */
INTDEF DeeTypeObject SeqTypesIterator_Type;   /* Sequence.Types.Iterator */
INTDEF DeeTypeObject SeqClassesIterator_Type; /* Sequence.Classes.Iterator */

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL SeqIds_New(DeeObject *__restrict seq);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL SeqTypes_New(DeeObject *__restrict seq);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL SeqClasses_New(DeeObject *__restrict seq);


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_H */
