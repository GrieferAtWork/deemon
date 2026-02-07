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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_PAIR_C
#define GUARD_DEEMON_OBJECTS_SEQ_PAIR_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>  /* DeeObject_FREE, DeeObject_MALLOC */
#include <deemon/object.h> /* ASSERT_OBJECT_TYPE_EXACT, DREF, DeeObject, Dee_AsObject, Dee_Decref*, Dee_Incref */
#include <deemon/pair.h>   /* CONFIG_ENABLE_SEQ_PAIR_TYPE, DeeSeqPairObject, DeeSeqPair_Type */
#include <deemon/tuple.h>  /* DeeTuple* */
#include <deemon/type.h>   /* DeeObject_Init, DeeObject_IsShared */

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "concat.h"
#include "default-compare.h"
#include "repeat.h"

#include <stddef.h> /* NULL */

DECL_BEGIN

#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE

typedef DeeSeqPairObject SeqPair;

/* TODO */

#endif /* CONFIG_ENABLE_SEQ_PAIR_TYPE */



PUBLIC WUNUSED DREF DeeSeqPairObject *
(DCALL DeeSeq_NewPairUninitialized)(void) {
#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE
	return DeeObject_MALLOC(DeeSeqPairObject);
#else /* CONFIG_ENABLE_SEQ_PAIR_TYPE */
	return (DREF DeeSeqPairObject *)DeeTuple_NewUninitialized(1);
#endif /* !CONFIG_ENABLE_SEQ_PAIR_TYPE */
}

PUBLIC NONNULL((1)) void
(DCALL DeeSeq_FreePairUninitialized)(DREF DeeSeqPairObject *__restrict self) {
#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE
	DeeObject_FREE(self);
#else /* CONFIG_ENABLE_SEQ_PAIR_TYPE */
	DeeTuple_FreeUninitialized((DeeTupleObject *)self);
#endif /* !CONFIG_ENABLE_SEQ_PAIR_TYPE */
}

PUBLIC ATTR_RETNONNULL NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeSeq_InitPairUninitialized)(/*inherit(always)*/ DREF DeeSeqPairObject *__restrict self,
                                     DeeObject *a, DeeObject *b) {
	ASSERT(a != Dee_AsObject(self));
	ASSERT(b != Dee_AsObject(self));
#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE
	Dee_Incref(a);
	self->sp_items[0] = a;
	Dee_Incref(b);
	self->sp_items[1] = b;
	DeeObject_Init(self, &DeeSeqPair_Type);
#else /* CONFIG_ENABLE_SEQ_PAIR_TYPE */
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeTuple_Type);
	ASSERT(DeeTuple_SIZE(self) == 2);
	Dee_Incref(a);
	DeeTuple_SET(self, 0, a);
	Dee_Incref(b);
	DeeTuple_SET(self, 1, b);
#endif /* !CONFIG_ENABLE_SEQ_PAIR_TYPE */
	return Dee_AsObject(self);
}

PUBLIC ATTR_RETNONNULL NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeSeq_InitPairUninitializedInherited)(/*inherit(always)*/ DREF DeeSeqPairObject *__restrict self,
                                              /*inherit(always)*/ DREF DeeObject *a,
                                              /*inherit(always)*/ DREF DeeObject *b) {
	ASSERT(a != Dee_AsObject(self));
	ASSERT(b != Dee_AsObject(self));
#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE
	self->sp_items[0] = a; /* Inherited */
	self->sp_items[1] = b; /* Inherited */
	DeeObject_Init(self, &DeeSeqPair_Type);
#else /* CONFIG_ENABLE_SEQ_PAIR_TYPE */
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeTuple_Type);
	ASSERT(DeeTuple_SIZE(self) == 2);
	DeeTuple_SET(self, 0, a); /* Inherited */
	DeeTuple_SET(self, 1, b); /* Inherited */
#endif /* !CONFIG_ENABLE_SEQ_PAIR_TYPE */
	return Dee_AsObject(self);
}



/* Construct a new 2-element sequence */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_OfPair(DeeObject *__restrict a,
              DeeObject *__restrict b) {
	DREF DeeSeqPairObject *result = DeeSeq_NewPairUninitialized();
	if unlikely(!result)
		goto err;
	return DeeSeq_InitPairUninitialized(result, a, b);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_OfPairInherited(/*inherit(always)*/ DREF DeeObject *__restrict a,
                       /*inherit(always)*/ DREF DeeObject *__restrict b) {
	DREF DeeSeqPairObject *result = DeeSeq_NewPairUninitialized();
	if unlikely(!result)
		goto err;
	return DeeSeq_InitPairUninitializedInherited(result, a, b);
err:
	Dee_Decref(b); /* Inherited */
	Dee_Decref(a); /* Inherited */
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_OfPairInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict a,
                                /*inherit(on_success)*/ DREF DeeObject *__restrict b) {
	DREF DeeSeqPairObject *result = DeeSeq_NewPairUninitialized();
	if unlikely(!result)
		goto err;
	return DeeSeq_InitPairUninitializedInherited(result, a, b);
err:
	return NULL;
}


/* Pack a 2-item sequence using a symbolic reference */
PUBLIC NONNULL((1)) void DCALL
DeeSeqPair_DecrefSymbolic(DREF DeeObject *__restrict self) {
#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE
	SeqPair *me = (SeqPair *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeSeqPair_Type);
	if (!DeeObject_IsShared(me)) {
		DeeObject_FREE(me);
		Dee_DecrefNokill(&DeeSeqPair_Type);
	} else {
		Dee_Incref(me->sp_items[1]);
		Dee_Incref(me->sp_items[0]);
		Dee_Decref_unlikely(me);
	}
#else /* CONFIG_ENABLE_SEQ_PAIR_TYPE */
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeTuple_Type);
	ASSERT(DeeTuple_SIZE(self) == 2);
	DeeTuple_DecrefSymbolic(self);
#endif /* !CONFIG_ENABLE_SEQ_PAIR_TYPE */
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_PAIR_C */
