/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_H
#define GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_H 1

#include <deemon/api.h>
#include <deemon/object.h>

/**/
#include "../generic-proxy.h"

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD(sp_seq) /* [1..1][const] The underlying sequence. */
} SeqSimpleProxy;

typedef struct {
	PROXY_OBJECT_HEAD(si_iter) /* [1..1][const] The underlying iterator. */
} SeqSimpleProxyIterator;


INTDEF DeeTypeObject SeqIds_Type;             /* sequence.ids */
INTDEF DeeTypeObject SeqTypes_Type;           /* sequence.types */
INTDEF DeeTypeObject SeqClasses_Type;         /* sequence.classes */
INTDEF DeeTypeObject SeqIdsIterator_Type;     /* sequence.ids.Iterator */
INTDEF DeeTypeObject SeqTypesIterator_Type;   /* sequence.types.Iterator */
INTDEF DeeTypeObject SeqClassesIterator_Type; /* sequence.classes.Iterator */

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL SeqIds_New(DeeObject *__restrict seq);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL SeqTypes_New(DeeObject *__restrict seq);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL SeqClasses_New(DeeObject *__restrict seq);


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_H */
