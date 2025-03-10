/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_UTIL_BYTEWRITER_H
#define GUARD_DEEMON_UTIL_BYTEWRITER_H 1

#include "../api.h"
/**/

#include "../alloc.h"
#include "../object.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_bytewriter   bytewriter
#define BYTEWRITER_INIT  DEE_BYTEWRITER_INIT
#define bytewriter_init  Dee_bytewriter_init
#define bytewriter_cinit Dee_bytewriter_cinit
#define bytewriter_putb  Dee_bytewriter_putb
#define bytewriter_putw  Dee_bytewriter_putw
#define bytewriter_putl  Dee_bytewriter_putl
#define bytewriter_putq  Dee_bytewriter_putq
#define bytewriter_alloc Dee_bytewriter_alloc
#define bytewriter_flush Dee_bytewriter_flush
#endif /* DEE_SOURCE */

struct Dee_bytewriter {
	uint8_t *bw_base;  /* [0..1][owned] Base address. */
	size_t   bw_size;  /* used size */
#ifndef Dee_MallocUsableSize
	size_t   bw_alloc; /* allocated size */
#define Dee_bytewriter_getalloc(self)     ((self)->bw_alloc)
#define _Dee_bytewriter_setalloc(self, v) (void)((self)->bw_alloc = (v))
#else /* !Dee_MallocUsableSize */
#define Dee_bytewriter_getalloc(self)     Dee_MallocUsableSize((self)->bw_base)
#define _Dee_bytewriter_setalloc(self, v) (void)0
#endif /* Dee_MallocUsableSize */
};
#ifdef Dee_MallocUsableSize
#define DEE_BYTEWRITER_INIT { NULL, 0 }
#define Dee_bytewriter_init(x)  \
	(void)((x)->bw_base = NULL, \
	       (x)->bw_size = 0)
#define Dee_bytewriter_cinit(x)              \
	(void)(Dee_ASSERT((x)->bw_base == NULL), \
	       Dee_ASSERT((x)->bw_size == 0))
#else /* Dee_MallocUsableSize */
#define DEE_BYTEWRITER_INIT { NULL, 0, 0 }
#define Dee_bytewriter_init(x)  \
	(void)((x)->bw_base = NULL, \
	       (x)->bw_size = (x)->bw_alloc = 0)
#define Dee_bytewriter_cinit(x)              \
	(void)(Dee_ASSERT((x)->bw_base == NULL), \
	       Dee_ASSERT((x)->bw_size == 0),    \
	       Dee_ASSERT((x)->bw_alloc == 0))
#endif /* !Dee_MallocUsableSize */
#define bytewriter_fini(x) \
	Dee_Free((x)->bw_base)

/* Reserve memory for at least `n_bytes'  */
LOCAL WUNUSED NONNULL((1)) uint8_t *DCALL
Dee_bytewriter_alloc(struct Dee_bytewriter *__restrict self, size_t n_bytes) {
	uint8_t *result;
	size_t avail = Dee_bytewriter_getalloc(self);
	Dee_ASSERT(self->bw_size <= avail);
	result = self->bw_base;
	if (n_bytes > (avail - self->bw_size)) {
		size_t new_alloc = avail * 2;
		if (!new_alloc)
			new_alloc = 2;
		while (new_alloc < self->bw_size + n_bytes)
			new_alloc *= 2;
		result = (uint8_t *)Dee_TryRealloc(self->bw_base, new_alloc);
		if unlikely(!result) {
			new_alloc = self->bw_size + n_bytes;
			result    = (uint8_t *)Dee_Realloc(self->bw_base, new_alloc);
			if unlikely(!result)
				goto err;
		}
		self->bw_base = result;
		_Dee_bytewriter_setalloc(self, new_alloc);
	}
	result += self->bw_size;
	self->bw_size += n_bytes;
	return result;
err:
	return NULL;
}


/* Append a single byte/word/dword or qword, returning -1 on error and 0 on success */
#define DEE_DEFINE_BYTEWRITER_PUTX(name, T, x)                  \
	LOCAL WUNUSED NONNULL((1)) int                              \
	(DCALL name)(struct Dee_bytewriter *__restrict self, T x) { \
		T *buf;                                                 \
		buf = (T *)Dee_bytewriter_alloc(self, sizeof(T));       \
		if unlikely(!buf)                                       \
			goto err;                                           \
		*buf = x;                                               \
		return 0;                                               \
	err:                                                        \
		return -1;                                              \
	}
DEE_DEFINE_BYTEWRITER_PUTX(Dee_bytewriter_putb, uint8_t, byte)
DEE_DEFINE_BYTEWRITER_PUTX(Dee_bytewriter_putw, uint16_t, word)
DEE_DEFINE_BYTEWRITER_PUTX(Dee_bytewriter_putl, uint32_t, dword)
#ifdef __UINT64_TYPE__
DEE_DEFINE_BYTEWRITER_PUTX(Dee_bytewriter_putq, __UINT64_TYPE__, qword)
#endif /* __UINT64_TYPE__ */
#undef DEE_DEFINE_BYTEWRITER_PUTX

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define Dee_bytewriter_putb(self, byte)  __builtin_expect(Dee_bytewriter_putb(self, byte), 0)
#define Dee_bytewriter_putw(self, word)  __builtin_expect(Dee_bytewriter_putw(self, word), 0)
#define Dee_bytewriter_putl(self, dword) __builtin_expect(Dee_bytewriter_putl(self, dword), 0)
#ifdef __UINT64_TYPE__
#define Dee_bytewriter_putq(self, qword) __builtin_expect(Dee_bytewriter_putq(self, qword), 0)
#endif /* __UINT64_TYPE__ */
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */


/* Deallocate all unused bytes, and return a heap-allocated pointer to what has been written.
 * When nothing has been written, `NULL' may be returned, which does not indicate an error,
 * as this function will never throw any. */
LOCAL NONNULL((1)) uint8_t *DCALL
Dee_bytewriter_flush(struct Dee_bytewriter *__restrict self) {
#ifndef Dee_MallocUsableSize
	Dee_ASSERT(self->bw_alloc >= self->bw_size);
	if (self->bw_alloc > self->bw_size)
#endif /* !Dee_MallocUsableSize */
	{
		uint8_t *newbase;
		newbase = (uint8_t *)Dee_TryRealloc(self->bw_base,
		                                    self->bw_size);
		if likely(newbase) {
			self->bw_base = newbase;
			_Dee_bytewriter_setalloc(self, self->bw_size);
		}
	}
	return self->bw_base;
}


DECL_END

#endif /* !GUARD_DEEMON_UTIL_BYTEWRITER_H */
