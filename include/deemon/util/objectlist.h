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
#ifndef GUARD_DEEMON_UTIL_OBJECTLIST_H
#define GUARD_DEEMON_UTIL_OBJECTLIST_H 1

#include "../api.h"

#include "../alloc.h"
#include "../object.h"
#include "../seq.h"
#include "../system-features.h" /* memchrp */
#include "../tuple.h"

#include <stdint.h> /* uintptr_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_objectlist           objectlist
#define OBJECTLIST_INIT          DEE_OBJECTLIST_INIT
#define objectlist_init          Dee_objectlist_init
#define objectlist_cinit         Dee_objectlist_cinit
#define objectlist_fini          Dee_objectlist_fini
#define objectlist_initseq       Dee_objectlist_initseq
#define objectlist_alloc         Dee_objectlist_alloc
#define objectlist_append        Dee_objectlist_append
#define objectlist_extendseq     Dee_objectlist_extendseq
#define objectlist_packlist      Dee_objectlist_packlist
#define objectlist_packtuple     Dee_objectlist_packtuple
#define objectlist_contains_byid Dee_objectlist_contains_byid
#define objectlist_setallocated  Dee_objectlist_setallocated
#endif /* DEE_SOURCE */

/* Helpers to alloc/realloc object vectors. */
#define Dee_objectlist_elemv_trymalloc(elemc)              ((DREF DeeObject **)Dee_TryMallocc(elemc, sizeof(DREF DeeObject *)))
#define Dee_objectlist_elemv_trycalloc(elemc)              ((DREF DeeObject **)Dee_TryCallocc(elemc, sizeof(DREF DeeObject *)))
#define Dee_objectlist_elemv_tryrealloc(elemv, elemc)      ((DREF DeeObject **)Dee_TryReallocc(elemv, elemc, sizeof(DREF DeeObject *)))
#define Dee_objectlist_elemv_malloc(elemc)                 ((DREF DeeObject **)Dee_Mallocc(elemc, sizeof(DREF DeeObject *)))
#define Dee_objectlist_elemv_calloc(elemc)                 ((DREF DeeObject **)Dee_Callocc(elemc, sizeof(DREF DeeObject *)))
#define Dee_objectlist_elemv_realloc(elemv, elemc)         ((DREF DeeObject **)Dee_Reallocc(elemv, elemc, sizeof(DREF DeeObject *)))
#define Dee_objectlist_elemv_trymalloc_safe(elemc)         ((DREF DeeObject **)Dee_TryMalloccSafe(elemc, sizeof(DREF DeeObject *)))
#define Dee_objectlist_elemv_trycalloc_safe(elemc)         ((DREF DeeObject **)Dee_TryCalloccSafe(elemc, sizeof(DREF DeeObject *)))
#define Dee_objectlist_elemv_tryrealloc_safe(elemv, elemc) ((DREF DeeObject **)Dee_TryRealloccSafe(elemv, elemc, sizeof(DREF DeeObject *)))
#define Dee_objectlist_elemv_malloc_safe(elemc)            ((DREF DeeObject **)Dee_MalloccSafe(elemc, sizeof(DREF DeeObject *)))
#define Dee_objectlist_elemv_calloc_safe(elemc)            ((DREF DeeObject **)Dee_CalloccSafe(elemc, sizeof(DREF DeeObject *)))
#define Dee_objectlist_elemv_realloc_safe(elemv, elemc)    ((DREF DeeObject **)Dee_RealloccSafe(elemv, elemc, sizeof(DREF DeeObject *)))
#define Dee_objectlist_elemv_free(elemv)                   Dee_Free(elemv)
#ifdef Dee_MallocUsableSize
#define Dee_objectlist_elemv_usable_size(elemv) (Dee_MallocUsableSize(elemv) / sizeof(DREF DeeObject *))
#endif /* Dee_MallocUsableSize */

#define DEE_OBJECTLIST_MINALLOC             8
#define DEE_OBJECTLIST_MOREALLOC(old_alloc) ((old_alloc) * 2)

struct Dee_objectlist {
	DREF DeeObject **ol_elemv; /* [1..1][0..ol_elemc|ALLOC(ol_elema)][owned] Vector of objects. */
	size_t           ol_elemc; /* Number of used entries. */
#ifndef Dee_MallocUsableSize
#define DEE_OBJECTLIST_HAVE_ELEMA
	size_t           ol_elema; /* Number of allocated entries. */
#define Dee_objectlist_getalloc(self)     ((self)->ol_elema)
#define _Dee_objectlist_setalloc(self, v) (void)((self)->ol_elema = (v))
#else /* !Dee_MallocUsableSize */
#define Dee_objectlist_getalloc(self)     (Dee_objectlist_elemv_usable_size((self)->ol_elemv))
#define _Dee_objectlist_setalloc(self, v) (void)0
#endif /* Dee_MallocUsableSize */
};


#ifdef DEE_OBJECTLIST_HAVE_ELEMA
#define DEE_OBJECTLIST_INIT { NULL, 0, 0 }
#define Dee_objectlist_init(self) \
	(void)((self)->ol_elemv = NULL, (self)->ol_elemc = (self)->ol_elema = 0)
#define Dee_objectlist_cinit(self)               \
	(void)(Dee_ASSERT((self)->ol_elemv == NULL), \
	       Dee_ASSERT((self)->ol_elemc == 0),    \
	       Dee_ASSERT((self)->ol_elema == 0))
#else /* DEE_OBJECTLIST_HAVE_ELEMA */
#define DEE_OBJECTLIST_INIT { NULL, 0 }
#define Dee_objectlist_init(self) \
	(void)((self)->ol_elemv = NULL, (self)->ol_elemc = 0)
#define Dee_objectlist_cinit(self)               \
	(void)(Dee_ASSERT((self)->ol_elemv == NULL), \
	       Dee_ASSERT((self)->ol_elemc == 0))
#endif /* !DEE_OBJECTLIST_HAVE_ELEMA */

/* Initialize `self' from the contents of a given `seq' */
#ifdef DEE_OBJECTLIST_HAVE_ELEMA
#define Dee_objectlist_init_fromseq(self, seq) \
	__builtin_expect(((self)->ol_elemv = DeeSeq_AsHeapVectorWithAlloc(seq, &(self)->ol_elemc, &(self)->ol_elema)) != NULL ? 0 : -1, 0)
#else /* DEE_OBJECTLIST_HAVE_ELEMA */
#define Dee_objectlist_init_fromseq(self, seq) \
	__builtin_expect(((self)->ol_elemv = DeeSeq_AsHeapVectorWithAlloc2(seq, &(self)->ol_elemc)) != NULL ? 0 : -1, 0)
#endif /* !DEE_OBJECTLIST_HAVE_ELEMA */


/* Finalize the given object list. */
#define Dee_objectlist_fini(self)                     \
	(Dee_Decrefv((self)->ol_elemv, (self)->ol_elemc), \
	 Dee_objectlist_elemv_free((self)->ol_elemv))

/* Finalize the given object list, but don't drop references. */
#define Dee_objectlist_fini_nodecref(self) \
	Dee_objectlist_elemv_free((self)->ol_elemv)

/* Initialize a given object-self `self' with the elements of `sequence'
 * @return: * :         The number of items appended.
 * @return: (size_t)-1: An error occurred. */
LOCAL WUNUSED NONNULL((1, 2)) int
(DCALL Dee_objectlist_initseq)(struct Dee_objectlist *__restrict self,
                               DeeObject *__restrict sequence) {
#ifdef DEE_OBJECTLIST_HAVE_ELEMA
	self->ol_elemv = DeeSeq_AsHeapVectorWithAlloc(sequence, &self->ol_elemc, &self->ol_elema);
#else /* DEE_OBJECTLIST_HAVE_ELEMA */
	self->ol_elemv = DeeSeq_AsHeapVectorWithAlloc2(sequence, &self->ol_elemc);
#endif /* !DEE_OBJECTLIST_HAVE_ELEMA */
	return self->ol_elemv ? 0 : -1;
}


/* Allocate memory for at least `num_objects' entries and return a pointer to
 * the first of them, leaving it up to the caller to initialize that memory.
 * Upon error, `NULL' is returned instead.
 * The caller is responsible to ensure that `num_objects' is non-zero. */
LOCAL WUNUSED NONNULL((1)) DREF DeeObject **DCALL
Dee_objectlist_alloc(struct Dee_objectlist *__restrict self,
                     size_t num_objects) {
	DREF DeeObject **result;
	size_t avail     = Dee_objectlist_getalloc(self);
	size_t min_alloc = self->ol_elemc + num_objects;
	Dee_ASSERT(num_objects != 0);
	Dee_ASSERT(self->ol_elemc <= avail);
	if (min_alloc > avail) {
		DREF DeeObject **new_list;
		size_t new_alloc = DEE_OBJECTLIST_MOREALLOC(avail);
		if (new_alloc < DEE_OBJECTLIST_MINALLOC)
			new_alloc = DEE_OBJECTLIST_MINALLOC;
		while (new_alloc < min_alloc)
			new_alloc = DEE_OBJECTLIST_MOREALLOC(new_alloc);
		Dee_ASSERT(new_alloc > self->ol_elemc);
		new_list = Dee_objectlist_elemv_tryrealloc(self->ol_elemv, new_alloc);
		if unlikely(!new_list) {
			new_alloc = min_alloc;
			new_list = Dee_objectlist_elemv_realloc(self->ol_elemv, new_alloc);
			if unlikely(!new_list)
				goto err;
		}
		self->ol_elemv = new_list;
		_Dee_objectlist_setalloc(self, new_alloc);
	}
	result = self->ol_elemv + self->ol_elemc;
	self->ol_elemc += num_objects;
	return result;
err:
	return NULL;
}

/* Append the given @value onto @self, returning -1 on error and 0 on success. */
LOCAL WUNUSED NONNULL((1, 2)) int DCALL
Dee_objectlist_append(struct Dee_objectlist *__restrict self,
                      DeeObject *__restrict ob) {
	size_t elema = Dee_objectlist_getalloc(self);
	Dee_ASSERT(self->ol_elemc <= elema);
	if (self->ol_elemc >= elema) {
		DREF DeeObject **new_list;
		size_t new_elema = DEE_OBJECTLIST_MOREALLOC(elema);
		if (new_elema < DEE_OBJECTLIST_MINALLOC)
			new_elema = DEE_OBJECTLIST_MINALLOC;
		Dee_ASSERT(new_elema > self->ol_elemc);
		new_list = Dee_objectlist_elemv_tryrealloc(self->ol_elemv, new_elema);
		if unlikely(!new_list) {
			new_elema = self->ol_elemc + 1;
			new_list = Dee_objectlist_elemv_realloc(self->ol_elemv, new_elema);
			if unlikely(!new_list)
				goto err;
		}
		self->ol_elemv = new_list;
		_Dee_objectlist_setalloc(self, new_elema);
	}
	self->ol_elemv[self->ol_elemc] = ob;
	Dee_Incref(ob);
	++self->ol_elemc;
	return 0;
err:
	return -1;
}

/* Append all objects from a given `sequence'
 * @return: * :         The number of items appended.
 * @return: (size_t)-1: An error occurred. */
LOCAL WUNUSED NONNULL((1, 2)) size_t DCALL
Dee_objectlist_extendseq(struct Dee_objectlist *__restrict self,
                         DeeObject *__restrict sequence) {
	size_t more;
#ifdef DEE_OBJECTLIST_HAVE_ELEMA
	more = DeeSeq_AsHeapVectorWithAllocReuseOffset(sequence,
	                                               &self->ol_elemv,
	                                               &self->ol_elema,
	                                               self->ol_elemc);
#else /* DEE_OBJECTLIST_HAVE_ELEMA */
	more = DeeSeq_AsHeapVectorWithAllocReuseOffset2(sequence,
	                                                &self->ol_elemv,
	                                                self->ol_elemc);
#endif /* !DEE_OBJECTLIST_HAVE_ELEMA */
	if likely(more != (size_t)-1)
		self->ol_elemc += more;
	Dee_ASSERT(Dee_objectlist_getalloc(self) >= self->ol_elemc);
	return more;
}


/* Pack the given Dee_objectlist into a List.
 * Upon success, `self' will have been finalized. */
#ifdef __INTELLISENSE__
extern WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_objectlist_packlist(struct Dee_objectlist *__restrict self);
#elif defined(DEE_OBJECTLIST_HAVE_ELEMA)
#define Dee_objectlist_packlist(self) \
	DeeList_NewVectorInheritedHeap((self)->ol_elemv, (self)->ol_elemc, (self)->ol_elema)
#else /* ... */
#define Dee_objectlist_packlist(self) \
	DeeList_NewVectorInheritedHeap2((self)->ol_elemv, (self)->ol_elemc)
#endif /* !... */

/* Pack the given objectlist into a Tuple.
 * Upon success, `self' will have been finalized. */
LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_objectlist_packtuple(struct objectlist *__restrict self) {
	DREF DeeObject *result;
	result = DeeTuple_NewVectorSymbolic(self->ol_elemc, self->ol_elemv);
	if likely(result)
		Dee_objectlist_elemv_free(self->ol_elemv);
	return result;
}

/* Check if `self' contains the *exact* element `elem' */
LOCAL WUNUSED NONNULL((1, 2)) bool DCALL
Dee_objectlist_contains_byid(struct objectlist *__restrict self, DeeObject *elem) {
#ifdef memchrp
	return memchrp(self->ol_elemv, (uintptr_t)elem, self->ol_elemc) != NULL;
#else /* memchrp */
	size_t i;
	for (i = 0; i < self->ol_elemc; ++i) {
		if (self->ol_elemv[i] == elem)
			return true;
	}
	return false;
#endif /* !memchrp */
}


/* Set the exact number of allocated elements. */
LOCAL WUNUSED NONNULL((1)) int
(DCALL Dee_objectlist_setallocated)(struct Dee_objectlist *__restrict self,
                                    size_t num_objects) {
	size_t avail = Dee_objectlist_getalloc(self);
	Dee_ASSERT(num_objects >= self->ol_elemc);
	if (num_objects > avail) {
		DREF DeeObject **new_list;
		new_list = Dee_objectlist_elemv_realloc(self->ol_elemv, num_objects);
		if unlikely(!new_list)
			goto err;
		self->ol_elemv = new_list;
		_Dee_objectlist_setalloc(self, num_objects);
	} else if (num_objects < avail) {
		DREF DeeObject **new_list;
		new_list = Dee_objectlist_elemv_tryrealloc(self->ol_elemv, num_objects);
		if likely(new_list) {
			self->ol_elemv = new_list;
			_Dee_objectlist_setalloc(self, num_objects);
		}
	}
	return 0;
err:
	return -1;
}


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define Dee_objectlist_initseq(self, sequence)         __builtin_expect(Dee_objectlist_initseq(self, sequence), 0)
#define Dee_objectlist_setallocated(self, num_objects) __builtin_expect(Dee_objectlist_setallocated(self, num_objects), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

DECL_END

#endif /* !GUARD_DEEMON_UTIL_OBJECTLIST_H */
