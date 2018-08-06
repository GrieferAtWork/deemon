/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SUBRANGE_H
#define GUARD_DEEMON_OBJECTS_SEQ_SUBRANGE_H 1

#include <deemon/api.h>
#include <deemon/object.h>

DECL_BEGIN

typedef struct {
    OBJECT_HEAD
    DREF DeeObject *sr_iter; /* [1..1][const] Underlying iterator. */
    size_t          sr_size; /* Max remaining number of items to-be yielded. */
} SubRangeIterator;

#ifdef CONFIG_NO_THREADS
#define READ_SIZE(x)            ((x)->sr_size)
#else
#define READ_SIZE(x) ATOMIC_READ((x)->sr_size)
#endif


typedef struct {
    OBJECT_HEAD
    DREF DeeObject *sr_seq;   /* [1..1][const] Underlying sequence. */
    size_t          sr_begin; /* [const] Amount of items discarded at the beginning. */
    size_t          sr_size;  /* [const] Max amount of items yielded. */
} SubRange;

typedef struct {
    OBJECT_HEAD
    DREF DeeObject *sr_seq;   /* [1..1][const] Underlying sequence. */
    size_t          sr_begin; /* [const] Amount of items discarded at the beginning. */
} SubRangeN;

INTDEF DeeTypeObject DeeSubRangeIterator_Type;
INTDEF DeeTypeObject DeeSubRange_Type;
INTDEF DeeTypeObject DeeSubRangeN_Type;

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SUBRANGE_H */
