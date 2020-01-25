/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_UTIL_OBJECTLIST_H
#define GUARD_DEEMON_UTIL_OBJECTLIST_H 1

#include <stddef.h>

#include "../api.h"
#include "../gc.h"
#include "../list.h"
#include "../object.h"
#include "../seq.h"
#include "../tuple.h"
#include "string.h"

DECL_BEGIN

struct objectlist {
	size_t           ol_size;  /* Number of used entries. */
	size_t           ol_alloc; /* Number of allocated entries. */
	DREF DeeObject **ol_list;  /* [1..1][0..ol_size|ALLOC(ol_alloc)][owned] Vector of objects. */
};

#define OBJECTLIST_INIT       { 0, 0, NULL }
#define objectlist_init(self) \
	((self)->ol_size = (self)->ol_alloc = 0, (self)->ol_list = NULL)
#define objectlist_cinit(self)          \
	(Dee_ASSERT((self)->ol_size == 0),  \
	 Dee_ASSERT((self)->ol_alloc == 0), \
	 Dee_ASSERT((self)->ol_list == NULL))

/* Finalize the given object list. */
LOCAL void DCALL
objectlist_fini(struct objectlist *__restrict self) {
	size_t i;
	for (i = 0; i < self->ol_size; ++i)
		Dee_Decref(self->ol_list[i]);
	Dee_Free(self->ol_list);
}

/* Allocate memory for at least `num_objects' entries and return a pointer to
 * the first of them, leaving it up to the caller to initialize that memory.
 * Upon error, `NULL' is returned instead.
 * The caller is responsible to ensure that `num_objects' is non-zero. */
LOCAL WUNUSED DREF DeeObject **DCALL
objectlist_alloc(struct objectlist *__restrict self, size_t num_objects) {
	DREF DeeObject **result;
	size_t min_alloc = self->ol_size + num_objects;
	Dee_ASSERT(num_objects != 0);
	Dee_ASSERT(self->ol_size <= self->ol_alloc);
	if (min_alloc > self->ol_alloc) {
		DREF DeeObject **new_list;
		size_t new_alloc = self->ol_alloc * 2;
		if (!new_alloc)
			new_alloc = 2;
		while (new_alloc < min_alloc)
			new_alloc *= 2;
		Dee_ASSERT(new_alloc > self->ol_size);
		new_list = (DREF DeeObject **)Dee_TryRealloc(self->ol_list,
		                                             new_alloc *
		                                             sizeof(DREF DeeObject *));
		if unlikely(!new_list) {
			new_alloc = min_alloc;
			new_list = (DREF DeeObject **)Dee_Realloc(self->ol_list,
			                                          new_alloc *
			                                          sizeof(DREF DeeObject *));
			if unlikely(!new_list)
				goto err;
		}
		self->ol_list  = new_list;
		self->ol_alloc = new_alloc;
	}
	result = self->ol_list + self->ol_size;
	self->ol_size += num_objects;
	return result;
err:
	return NULL;
}

/* Append the given @value onto @self, returning -1 on error and 0 on success. */
LOCAL int DCALL
objectlist_append(struct objectlist *__restrict self,
                  DeeObject *__restrict ob) {
	Dee_ASSERT(self->ol_size <= self->ol_alloc);
	if (self->ol_size >= self->ol_alloc) {
		DREF DeeObject **new_list;
		size_t new_alloc = self->ol_alloc * 2;
		if (!new_alloc)
			new_alloc = 2;
		Dee_ASSERT(new_alloc > self->ol_size);
		new_list = (DREF DeeObject **)Dee_TryRealloc(self->ol_list,
		                                             new_alloc *
		                                             sizeof(DREF DeeObject *));
		if unlikely(!new_list) {
			new_alloc = self->ol_size + 1;
			new_list = (DREF DeeObject **)Dee_Realloc(self->ol_list,
			                                          new_alloc *
			                                          sizeof(DREF DeeObject *));
			if unlikely(!new_list)
				goto err;
		}
		self->ol_list  = new_list;
		self->ol_alloc = new_alloc;
	}
	self->ol_list[self->ol_size] = ob;
	Dee_Incref(ob);
	++self->ol_size;
	return 0;
err:
	return -1;
}

/* Append all objects from a given `sequence'
 * @return: * :         The number of items appended.
 * @return: (size_t)-1: An error occurred. */
LOCAL size_t DCALL
objectlist_extendseq(struct objectlist *__restrict self,
                     DeeObject *__restrict sequence) {
	size_t more;
	more = DeeSeq_AsHeapVectorWithAllocReuseOffset(sequence,
	                                               &self->ol_list,
	                                               &self->ol_alloc,
	                                               self->ol_size);
	if likely(more != (size_t)-1)
		self->ol_size += more;
	Dee_ASSERT(self->ol_alloc >= self->ol_size);
	return more;
}


/* Pack the given objectlist into a List.
 * Upon success, `self' will have been finalized. */
LOCAL WUNUSED DREF DeeObject *DCALL
objectlist_packlist(struct objectlist *__restrict self) {
	return DeeList_NewVectorInheritedHeap(self->ol_alloc,
	                                      self->ol_size,
	                                      self->ol_list);
}

/* Pack the given objectlist into a Tuple.
 * Upon success, `self' will have been finalized. */
LOCAL WUNUSED DREF DeeObject *DCALL
objectlist_packtuple(struct objectlist *__restrict self) {
	DREF DeeObject *result;
	result = DeeTuple_NewVectorSymbolic(self->ol_size, self->ol_list);
	if likely(result)
		Dee_Free(self->ol_list);
	return result;
}




DECL_END

#endif /* !GUARD_DEEMON_UTIL_OBJECTLIST_H */
