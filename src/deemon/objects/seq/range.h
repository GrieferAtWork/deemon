/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_RANGE_H
#define GUARD_DEEMON_OBJECTS_SEQ_RANGE_H 1

#include <deemon/api.h>
#include <deemon/object.h>

#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif

DECL_BEGIN

typedef struct {
    OBJECT_HEAD
    /* NOTE: Iteration stops only when `index >= ir_end'.
     *       This is feasible because of our infinite-precision integer library,
     *       meaning that user-code shouldn't need to worry about overflows in
     *       standard arithmetical operations. */
    DREF DeeObject *r_start; /* [1..1][const] Starting index. */
    DREF DeeObject *r_end;   /* [1..1][const] Ending index. */
    DREF DeeObject *r_step;  /* [0..1][const] Step size (or NULL when `tp_inc()' should be used). */
    bool            r_rev;   /* [const] True if `r_step' is non-NULL and negative. */
} Range;

typedef struct {
    OBJECT_HEAD
    DREF DeeObject *ri_index; /* [1..1][lock(ri_lock)] The current index operated on using using `tp_inplace_add()' or `tp_inc()'. */
    DREF Range     *ri_range; /* [1..1][const] The underlying range object. */
    DeeObject      *ri_end;   /* [1..1][const][== ri_range->r_end] Ending index. */
    DeeObject      *ri_step;  /* [0..1][const][== ri_range->r_step] Step size (or NULL when `tp_inc()' should be used). */
    bool            ri_first; /* [lock(ri_lock)] Only true during the first iteration to skip the initial modification. */
#ifndef CONFIG_NO_THREADS
    rwlock_t        ri_lock;  /* Lock for synchronizing access to ri_index. */
#endif
} RangeIterator;

INTDEF DeeTypeObject SeqRangeIterator_Type;
INTDEF DeeTypeObject SeqRange_Type;




typedef struct {
    OBJECT_HEAD
    /* NOTE: Iteration stops when `index >= ir_end' (ir_step > 0) / `index <= ir_end' (ir_step < 0) or
     *      `index += ir_step' would roll over. (returning `ITER_DONE' immediately) */
    dssize_t ir_start; /* [const] Starting index. */
    dssize_t ir_end;   /* [const] Ending index. */
    dssize_t ir_step;  /* [const][!0] Step size (may be negative). */
} IntRange;

typedef struct {
    OBJECT_HEAD
    dssize_t        iri_index; /* [atomic] The current index operated on using using `tp_inplace_add()' or `tp_inc()'. */
    dssize_t        iri_end;   /* [weak(const)] Ending index. */
    dssize_t        iri_step;  /* [weak(const)] Step size (may be negative). */
    DREF IntRange  *iri_range; /* [1..1][const] The underlying range object. */
} IntRangeIterator;

INTDEF DeeTypeObject SeqIntRangeIterator_Type;
INTDEF DeeTypeObject SeqIntRange_Type;


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_RANGE_H */
