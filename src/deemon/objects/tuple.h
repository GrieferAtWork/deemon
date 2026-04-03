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
#ifndef GUARD_DEEMON_OBJECTS_TUPLE_H
#define GUARD_DEEMON_OBJECTS_TUPLE_H 1

#include <deemon/api.h>

#include <deemon/method-hints.h> /* Dee_seq_enumerate_index_t */
#include <deemon/object.h>       /* DeeTypeObject, Dee_foreach_t, Dee_ssize_t */
#include <deemon/tuple.h>        /* DeeTupleObject */

#include "generic-proxy.h"

#include <stddef.h> /* size_t */

DECL_BEGIN

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* From "../tuple.c" */
tuple_mh_foreach_reverse(DeeTupleObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* From "../tuple.c" */
tuple_mh_enumerate_index_reverse(DeeTupleObject *__restrict self, Dee_seq_enumerate_index_t proc,
                                 void *arg, size_t start, size_t end);


/*  ====== `Tuple.Iterator' type implementation ======  */
typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeTupleObject, ti_tuple); /* [1..1][const] Referenced tuple. */
	DWEAK size_t                         ti_index;  /* [<= ti_tuple->t_size] Next-element index. */
} TupleIterator;

INTDEF DeeTypeObject DeeTupleIterator_Type;

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_TUPLE_H */
