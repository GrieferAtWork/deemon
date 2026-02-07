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
/*!export **/
/*!export DeeSeqOne**/
/*!export DeeSeqPair**/
/*!export DeeSeq_Of**/
/*!export DeeSeq_**/
/*!export CONFIG_ENABLE_SEQ_*_TYPE*/
#ifndef GUARD_DEEMON_PAIR_H
#define GUARD_DEEMON_PAIR_H 1 /*!export-*/

#include "api.h"

#include "types.h" /* DREF, DeeObject, DeeTypeObject, Dee_AsObject, Dee_OBJECT_HEAD */

#ifdef CONFIG_BUILDING_DEEMON
/* Configure which dedicated sequence types should be enabled
 * (and which should just be implemented using "Tuple") */
#if (!defined(CONFIG_ENABLE_SEQ_ONE_TYPE) && \
     !defined(CONFIG_DISABLE_SEQ_ONE_TYPE))
#if !defined(__OPTIMIZE_SIZE__) && 1
#define CONFIG_ENABLE_SEQ_ONE_TYPE /* Use dedicated type for `{item}' */
#else /* ... */
#define CONFIG_DISABLE_SEQ_ONE_TYPE /* Use Tuple for `{item}' */
#endif /* !... */
#endif /* !CONFIG_ENABLE_SEQ_ONE_TYPE && !CONFIG_DISABLE_SEQ_ONE_TYPE */

#if (!defined(CONFIG_ENABLE_SEQ_PAIR_TYPE) && \
     !defined(CONFIG_DISABLE_SEQ_PAIR_TYPE))
#if !defined(__OPTIMIZE_SIZE__) && 0 /* TODO: Not fully implemented */
#define CONFIG_ENABLE_SEQ_PAIR_TYPE /* Use dedicated type for `{a, b}' */
#else /* ... */
#define CONFIG_DISABLE_SEQ_PAIR_TYPE /* Use Tuple for `{a, b}' */
#endif /* !... */
#endif /* !CONFIG_ENABLE_SEQ_PAIR_TYPE && !CONFIG_DISABLE_SEQ_PAIR_TYPE */

#if !defined(CONFIG_ENABLE_SEQ_ONE_TYPE) || !defined(CONFIG_ENABLE_SEQ_PAIR_TYPE)
#ifndef __INTELLISENSE__
#include "alloc.h"  /* DeeObject_FREE, DeeObject_MALLOC */
#include "object.h" /* Dee_DecrefNokill, Dee_Incref */
#include "tuple.h"  /* DeeTuple* */
#include "type.h"   /* DeeObject_Init */
#endif /* !__INTELLISENSE__ */
#endif /* !CONFIG_ENABLE_SEQ_ONE_TYPE || !CONFIG_ENABLE_SEQ_PAIR_TYPE */
#endif /* CONFIG_BUILDING_DEEMON */

DECL_BEGIN

/************************************************************************/
/* Dedicated type for `{ item }'                                        */
/************************************************************************/
#ifdef CONFIG_BUILDING_DEEMON
#ifdef CONFIG_ENABLE_SEQ_ONE_TYPE
typedef struct {
	Dee_OBJECT_HEAD
	DREF DeeObject *so_item; /* [1..1][const] The sequence's first and only item. */
} DeeSeqOneObject;
#define DeeSeqOne_GET(self) Dee_REQUIRES_TYPE(DeeSeqOneObject const *, self)->so_item

INTDEF DeeTypeObject DeeSeqOne_Type;
#ifdef __INTELLISENSE__
#define DeeSeq_InitOne_inplace(self) (void)0
#define DeeSeq_FiniOne_inplace(self) (void)0
#else /* __INTELLISENSE__ */
#define DeeSeq_InitOne_inplace(self) (void)DeeObject_Init(self, &DeeSeqOne_Type)
#define DeeSeq_FiniOne_inplace(self) (void)Dee_DecrefNokill(&DeeSeqOne_Type)
#endif /* !__INTELLISENSE__ */
#else /* CONFIG_ENABLE_SEQ_ONE_TYPE */
#ifdef __INTELLISENSE__
typedef struct { Dee_OBJECT_HEAD DREF DeeObject *t_elem[1]; } DeeSeqOneObject;
#define DeeSeqOne_GET(self) Dee_REQUIRES_TYPE(DeeSeqOneObject const *, self)->t_elem[0]
#else /* __INTELLISENSE__ */
typedef DeeTupleObject DeeSeqOneObject;
#define DeeSeqOne_GET(self) DeeTuple_GET(self, 0)
#endif /* !__INTELLISENSE__ */
#define DeeSeq_InitOne_inplace(self) (void)0
#define DeeSeq_FiniOne_inplace(self) (void)0
#endif /* !CONFIG_ENABLE_SEQ_ONE_TYPE */
#else /* CONFIG_BUILDING_DEEMON */
typedef struct {
	Dee_OBJECT_HEAD /* Implementation hidden... */
} DeeSeqOneObject;
#endif /* !CONFIG_BUILDING_DEEMON */

DFUNDEF WUNUSED DREF DeeSeqOneObject *(DCALL DeeSeq_NewOneUninitialized)(void);
DFUNDEF NONNULL((1)) void (DCALL DeeSeq_FreeOneUninitialized)(DREF DeeSeqOneObject *__restrict self);
#if !defined(CONFIG_BUILDING_DEEMON) || defined(__OPTIMIZE_SIZE__)
DFUNDEF ATTR_RETNONNULL NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSeq_InitOne)(/*inherit(always)*/ DREF DeeSeqOneObject *__restrict self,
                       DeeObject *__restrict item);
DFUNDEF ATTR_RETNONNULL NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeSeq_InitOneInherited)(/*inherit(always)*/ DREF DeeSeqOneObject *__restrict self,
                                /*inherit(always)*/ DREF DeeObject *__restrict item);
#elif defined(CONFIG_ENABLE_SEQ_ONE_TYPE)
#ifdef __INTELLISENSE__
#define DeeSeq_NewOneUninitialized()      ((DeeSeqOneObject *)sizeof(DeeSeqOneObject))
#define DeeSeq_FreeOneUninitialized(self) (void)Dee_REQUIRES_TYPE(DeeSeqOneObject *, self)
#define DeeSeq_InitOneInherited(self, item) \
	((self)->so_item = (item), Dee_AsObject(self))
#define DeeSeq_InitOne(self, item) \
	((self)->so_item = (item), Dee_AsObject(self))
#else /* __INTELLISENSE__ */
#define DeeSeq_NewOneUninitialized()      DeeObject_MALLOC(DeeSeqOneObject)
#define DeeSeq_FreeOneUninitialized(self) DeeObject_FREE(Dee_REQUIRES_TYPE(DeeSeqOneObject *, self))
#define DeeSeq_InitOneInherited(self, item) \
	((self)->so_item = (item), DeeObject_Init(self, &DeeSeqOne_Type), Dee_AsObject(self))
#define DeeSeq_InitOne(self, item)  \
	(Dee_Incref(item), (self)->so_item = (item), \
	 DeeObject_Init(self, &DeeSeqOne_Type), Dee_AsObject(self))
#endif /* !__INTELLISENSE__ */
#else /* CONFIG_ENABLE_SEQ_ONE_TYPE */
#define DeeSeq_InitOneInherited(self, item) \
	((self)->t_elem[0] = (item), Dee_AsObject(self))
#ifdef __INTELLISENSE__
#define DeeSeq_NewOneUninitialized()      ((DeeSeqOneObject *)sizeof(DeeSeqOneObject))
#define DeeSeq_FreeOneUninitialized(self) (void)Dee_REQUIRES_TYPE(DeeSeqOneObject *, self)
#define DeeSeq_InitOne(self, item) DeeSeq_InitOneInherited(self, item)
#else /* __INTELLISENSE__ */
#define DeeSeq_NewOneUninitialized()      DeeTuple_NewUninitialized(1)
#define DeeSeq_FreeOneUninitialized(self) DeeTuple_FreeUninitialized(self)
#define DeeSeq_InitOne(self, item) \
	(Dee_Incref(item), (self)->t_elem[0] = (item), Dee_AsObject(self))
#endif /* !__INTELLISENSE__ */
#endif /* !CONFIG_ENABLE_SEQ_ONE_TYPE */

/* Construct a new 1-element sequence */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_OfOne(DeeObject *__restrict item);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_OfOneInherited(/*inherit(always)*/ DREF DeeObject *__restrict item);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_OfOneInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict item);

/* Pack a single-item sequence using a symbolic reference */
#define DeeSeq_OfOneSymbolic(item) DeeSeq_OfOneInheritedOnSuccess(item)
DFUNDEF NONNULL((1)) void DCALL DeeSeqOne_DecrefSymbolic(DREF DeeObject *__restrict self);




/************************************************************************/
/* Dedicated type for `{ a, b }'                                        */
/************************************************************************/
#ifdef CONFIG_BUILDING_DEEMON
#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE
typedef struct {
	Dee_OBJECT_HEAD
	DREF DeeObject *sp_items[2]; /* [1..1][const] The sequence's items. */
} DeeSeqPairObject;
#define DeeSeqPair_ELEM(self) Dee_REQUIRES_TYPE(DeeSeqPairObject *, self)->sp_items

INTDEF DeeTypeObject DeeSeqPair_Type;
#ifdef __INTELLISENSE__
#define DeeSeq_InitPair_inplace(self) (void)0
#define DeeSeq_FiniPair_inplace(self) (void)0
#else /* __INTELLISENSE__ */
#define DeeSeq_InitPair_inplace(self) (void)DeeObject_Init(self, &DeeSeqPair_Type)
#define DeeSeq_FiniPair_inplace(self) (void)Dee_DecrefNokill(&DeeSeqPair_Type)
#endif /* !__INTELLISENSE__ */
#else /* CONFIG_ENABLE_SEQ_PAIR_TYPE */
#ifdef __INTELLISENSE__
typedef struct { Dee_OBJECT_HEAD DREF DeeObject *t_elem[2]; } DeeSeqPairObject;
#define DeeSeqPair_ELEM(self) Dee_REQUIRES_TYPE(DeeSeqPairObject *, self)->t_elem
#else /* __INTELLISENSE__ */
typedef DeeTupleObject DeeSeqPairObject;
#define DeeSeqPair_ELEM(self) DeeTuple_ELEM(self)
#endif /* !__INTELLISENSE__ */
#define DeeSeq_InitPair_inplace(self) (void)0
#define DeeSeq_FiniPair_inplace(self) (void)0
#endif /* !CONFIG_ENABLE_SEQ_PAIR_TYPE */
#else /* CONFIG_BUILDING_DEEMON */
typedef struct {
	Dee_OBJECT_HEAD /* Implementation hidden... */
} DeeSeqPairObject;
#endif /* !CONFIG_BUILDING_DEEMON */

DFUNDEF WUNUSED DREF DeeSeqPairObject *(DCALL DeeSeq_NewPairUninitialized)(void);
DFUNDEF NONNULL((1)) void (DCALL DeeSeq_FreePairUninitialized)(DREF DeeSeqPairObject *__restrict self);
#if !defined(CONFIG_BUILDING_DEEMON) || defined(__OPTIMIZE_SIZE__)
DFUNDEF ATTR_RETNONNULL NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeSeq_InitPair)(/*inherit(always)*/ DREF DeeSeqPairObject *__restrict self,
                        DeeObject *a, DeeObject *b);
DFUNDEF ATTR_RETNONNULL NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeSeq_InitPairInherited)(/*inherit(always)*/ DREF DeeSeqPairObject *__restrict self,
                                 /*inherit(always)*/ DREF DeeObject *a,
                                 /*inherit(always)*/ DREF DeeObject *b);
#elif defined(CONFIG_ENABLE_SEQ_PAIR_TYPE)
#ifdef __INTELLISENSE__
#define DeeSeq_NewPairUninitialized()      ((DeeSeqPairObject *)sizeof(DeeSeqPairObject))
#define DeeSeq_FreePairUninitialized(self) (void)Dee_REQUIRES_TYPE(DeeSeqPairObject *, self)
#define DeeSeq_InitPairInherited(self, a, b) \
	((self)->sp_items[0] = (a), (self)->sp_items[1] = (b), Dee_AsObject(self))
#define DeeSeq_InitPair(self, a, b) \
	((self)->sp_items[0] = (a), (self)->sp_items[1] = (b), Dee_AsObject(self))
#else /* __INTELLISENSE__ */
#define DeeSeq_NewPairUninitialized()      DeeObject_MALLOC(DeeSeqPairObject)
#define DeeSeq_FreePairUninitialized(self) DeeObject_FREE(Dee_REQUIRES_TYPE(DeeSeqPairObject *, self))
#define DeeSeq_InitPairInherited(self, a, b)               \
	((self)->sp_items[0] = (a), (self)->sp_items[1] = (b), \
	 DeeObject_Init(self, &DeeSeqPair_Type), Dee_AsObject(self))
#define DeeSeq_InitPair(self, a, b)            \
	(Dee_Incref(a), (self)->sp_items[0] = (a), \
	 Dee_Incref(b), (self)->sp_items[1] = (b), \
	 DeeObject_Init(self, &DeeSeqPair_Type), Dee_AsObject(self))
#endif /* !__INTELLISENSE__ */
#else /* CONFIG_ENABLE_SEQ_PAIR_TYPE */
#define DeeSeq_InitPairInherited(self, a, b) \
	((self)->t_elem[0] = (a), (self)->t_elem[1] = (b), Dee_AsObject(self))
#ifdef __INTELLISENSE__
#define DeeSeq_NewPairUninitialized()      ((DeeSeqPairObject *)sizeof(DeeSeqPairObject))
#define DeeSeq_FreePairUninitialized(self) (void)Dee_REQUIRES_TYPE(DeeSeqPairObject *, self)
#define DeeSeq_InitPair(self, a, b)        DeeSeq_InitPairInherited(self, a, b)
#else /* __INTELLISENSE__ */
#define DeeSeq_NewPairUninitialized()      DeeTuple_NewUninitialized(2)
#define DeeSeq_FreePairUninitialized(self) DeeTuple_FreeUninitialized(self)
#define DeeSeq_InitPair(self, a, b)          \
	(Dee_Incref(a), (self)->t_elem[0] = (a), \
	 Dee_Incref(b), (self)->t_elem[1] = (b), \
	 Dee_AsObject(self))
#endif /* !__INTELLISENSE__ */
#endif /* !CONFIG_ENABLE_SEQ_PAIR_TYPE */
#define DeeSeq_InitPairv(self, items) \
	DeeSeq_InitPair(self, (items)[0], (items)[1])
#define DeeSeq_InitPairvInherited(self, items) \
	DeeSeq_InitPairInherited(self, (items)[0], (items)[1])

/* Construct a new 2-element sequence */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_OfPair(DeeObject *a, DeeObject *b);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_OfPairInherited(/*inherit(always)*/ DREF DeeObject *a, /*inherit(always)*/ DREF DeeObject *b);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_OfPairInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *a, /*inherit(on_success)*/ DREF DeeObject *b);
#if 0
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_OfPairv(DeeObject *const items[2]);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_OfPairvInherited(/*inherit(always)*/ DREF DeeObject *const items[2]);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_OfPairvInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *const items[2]);
#else
#define DeeSeq_OfPairv(items)                   DeeSeq_OfPair((items)[0], (items)[1])
#define DeeSeq_OfPairvInherited(items)          DeeSeq_OfPairInherited((items)[0], (items)[1])
#define DeeSeq_OfPairvInheritedOnSuccess(items) DeeSeq_OfPairInheritedOnSuccess((items)[0], (items)[1])
#endif

/* Pack a 2-item sequence using symbolic references */
#define DeeSeq_OfPairSymbolic(a, b)   DeeSeq_OfPairInheritedOnSuccess(a, b)
#define DeeSeq_OfPairvSymbolic(items) DeeSeq_OfPairvInheritedOnSuccess(items)
DFUNDEF NONNULL((1)) void DCALL DeeSeqPair_DecrefSymbolic(DREF DeeObject *__restrict self);

DECL_END

#endif /* !GUARD_DEEMON_PAIR_H */
