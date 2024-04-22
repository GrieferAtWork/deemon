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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_H
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/util/lock.h>

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *digi_seq;   /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `digi_seq'. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *digi_tp_getitem_index)(DeeObject *self, size_t index);
	size_t          digi_index; /* Next index to enumerate. */
	/* NOTE: Rewind start index is always `0' */
} DefaultIterator_WithGetItemIndex;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *disgi_seq;   /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `disgi_seq'.
	 * This is either a `tp_getitem_index' or `tp_getitem_index_fast' operator. */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *disgi_tp_getitem_index)(DeeObject *self, size_t index);
	size_t          disgi_index; /* Next index to enumerate. */
	size_t          disgi_size;  /* [const] Iteration stop index. */
	size_t          disgi_start; /* [const] Iteration start index (for rewind) */
} DefaultIterator_WithSizeAndGetItemIndex;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject   *dig_seq;   /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `dig_seq'. */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *dig_tp_getitem)(DeeObject *self, DeeObject *index);
	DREF DeeObject   *dig_index; /* [1..1][lock(dig_lock)] Next index to enumerate. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t dig_lock;  /* Lock for `dig_index' */
#endif /* !CONFIG_NO_THREADS */
	/* NOTE: Rewind start index is always `DeeInt_Zero' */
} DefaultIterator_WithGetItem;

typedef struct {
	OBJECT_HEAD
	DREF DeeObject   *disg_seq;   /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `disg_seq'. */
	WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *disg_tp_getitem)(DeeObject *self, DeeObject *index);
	DREF DeeObject   *disg_index; /* [1..1][lock(disg_lock)] Next index to enumerate. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t disg_lock;  /* Lock for `disg_index' */
#endif /* !CONFIG_NO_THREADS */
	DREF DeeObject   *disg_size;  /* [1..1][const] Iteration stop index. */
	/* NOTE: Rewind start index is always `DeeInt_Zero' */
} DefaultIterator_WithSizeAndGetItem;

typedef struct {
	OBJECT_HEAD
	DeeTypeObject    *ditsg_tp_seq; /* [1..1][const] The type to pass to `ditsg_tp_tgetitem'. */
	DREF DeeObject   *ditsg_seq;    /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `ditsg_seq'. */
	WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *ditsg_tp_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
	DREF DeeObject   *ditsg_index;  /* [1..1][lock(ditsg_lock)] Next index to enumerate. */
	DREF DeeObject   *ditsg_size;   /* [1..1][const] Iteration stop index. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t ditsg_lock;   /* Lock for `ditsg_index' */
#endif /* !CONFIG_NO_THREADS */
} DefaultIterator_TWithSizeAndGetItem;

typedef struct {
	OBJECT_HEAD
	DeeTypeObject    *ditg_tp_seq; /* [1..1][const] The type to pass to `ditg_tp_tgetitem'. */
	DREF DeeObject   *ditg_seq;    /* [1..1][const] The sequence being iterated. */
	/* [1..1][const] Callback to load the `index'th element of `ditg_seq'. */
	WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *ditg_tp_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
	DREF DeeObject   *ditg_index;  /* [1..1][lock(ditg_lock)] Next index to enumerate. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t ditg_lock;   /* Lock for `ditg_index' */
#endif /* !CONFIG_NO_THREADS */
} DefaultIterator_TWithGetItem;

INTDEF DeeTypeObject DefaultIterator_WithGetItemIndex_Type;            /* DefaultIterator_WithGetItemIndex */
INTDEF DeeTypeObject DefaultIterator_WithSizeAndGetItemIndex_Type;     /* DefaultIterator_WithSizeAndGetItemIndex */
INTDEF DeeTypeObject DefaultIterator_WithSizeAndGetItemIndexFast_Type; /* DefaultIterator_WithSizeAndGetItemIndex */

INTDEF DeeTypeObject DefaultIterator_WithGetItem_Type;         /* DefaultIterator_WithGetItem */
INTDEF DeeTypeObject DefaultIterator_WithSizeAndGetItem_Type;  /* DefaultIterator_WithSizeAndGetItem */
INTDEF DeeTypeObject DefaultIterator_TWithSizeAndGetItem_Type; /* DefaultIterator_TWithSizeAndGetItem */
INTDEF DeeTypeObject DefaultIterator_TWithGetItem_Type;        /* DefaultIterator_TWithGetItem */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_H */
