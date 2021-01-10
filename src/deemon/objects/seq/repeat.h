/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_REPEAT_H
#define GUARD_DEEMON_OBJECTS_SEQ_REPEAT_H 1

#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/util/rwlock.h>

#include "../../runtime/runtime_error.h"

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *r_seq; /* [1..1][const] The sequence being repeated. */
	size_t          r_num; /* [!0][const] The number of times by which to repeat the sequence. */
} Repeat;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *ri_obj; /* [1..1][const] The object being repeated. */
	size_t          ri_num; /* [const] The number of times by which to repeat the object. */
} RepeatItem;

typedef struct {
	OBJECT_HEAD
	DREF Repeat    *ri_rep;  /* [1..1][const] The underlying repeat-proxy-sequence. */
	DREF DeeObject *ri_iter; /* [1..1][lock(ri_lock)] The current repeat-iterator. */
	size_t          ri_num;  /* [lock(ri_lock)] The remaining number of times to repeat the sequence. */
#ifndef CONFIG_NO_THREADS
	rwlock_t        ri_lock; /* Lock for accessing the variable fields above. */
#endif /* !CONFIG_NO_THREADS */
} RepeatIterator;

typedef struct {
	OBJECT_HEAD
	DREF RepeatItem   *rii_rep; /* [1..1][const] The underlying repeat-proxy-sequence. */
	DeeObject         *rii_obj; /* [1..1][const][== rii_rep->r_obj] The object being repeated. */
	DWEAK size_t       rii_num; /* The remaining number of repetitions. */
} RepeatItemIterator;

INTDEF DeeTypeObject SeqRepeat_Type;
INTDEF DeeTypeObject SeqItemRepeat_Type;
INTDEF DeeTypeObject SeqRepeatIterator_Type;
INTDEF DeeTypeObject SeqItemRepeatIterator_Type;


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_REPEAT_H */
