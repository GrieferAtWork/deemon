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
#ifndef GUARD_DEEMON_UTIL_BYTEWRITER_H
#define GUARD_DEEMON_UTIL_BYTEWRITER_H 1

#include <string.h>

#include "../alloc.h"
#include "../api.h"
#include "../object.h"

DECL_BEGIN

struct bytewriter {
	uint8_t *bw_base;  /* [0..1][owned] Base address. */
	size_t   bw_size;  /* used size */
	size_t   bw_alloc; /* allocated size */
};
#define BYTEWRITER_INIT     { NULL, 0, 0 }
#define bytewriter_init(x)  (void)((x)->bw_base = NULL, (x)->bw_size = (x)->bw_alloc = 0)
#define bytewriter_cinit(x) (void)(Dee_ASSERT((x)->bw_base == NULL), Dee_ASSERT((x)->bw_size == 0), Dee_ASSERT((x)->bw_alloc == 0))
#define bytewriter_fini(x)  Dee_Free((x)->bw_base)

/* Reserve memory for at least `n_bytes'  */
LOCAL uint8_t *DCALL
bytewriter_alloc(struct bytewriter *__restrict self, size_t n_bytes) {
	uint8_t *result;
	Dee_ASSERT(self->bw_size <= self->bw_alloc);
	result = self->bw_base;
	if (n_bytes > (self->bw_alloc - self->bw_size)) {
		size_t new_alloc = self->bw_alloc * 2;
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
		self->bw_base  = result;
		self->bw_alloc = new_alloc;
	}
	result += self->bw_size;
	self->bw_size += n_bytes;
	return result;
err:
	return NULL;
}


/* Append a single byte/word/dword or qword, returning -1 on error and 0 on success */
#define DEFINE_BYTEWRITER_PUTX(name, T, x)                             \
	LOCAL int (DCALL name)(struct bytewriter * __restrict self, T x) { \
		T *buf;                                                        \
		buf = (T *)bytewriter_alloc(self, sizeof(T));                  \
		if unlikely(!buf)                                              \
			goto err;                                                  \
		*buf = x;                                                      \
		return 0;                                                      \
	err:                                                               \
		return -1;                                                     \
	}
DEFINE_BYTEWRITER_PUTX(bytewriter_putb,uint8_t,byte)
DEFINE_BYTEWRITER_PUTX(bytewriter_putw,uint16_t,word)
DEFINE_BYTEWRITER_PUTX(bytewriter_putl,uint32_t,dword)
DEFINE_BYTEWRITER_PUTX(bytewriter_putq,uint64_t,qword)
#undef DEFINE_BYTEWRITER_PUTX

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define bytewriter_putb(self, byte)  __builtin_expect(bytewriter_putb(self, byte), 0)
#define bytewriter_putw(self, word)  __builtin_expect(bytewriter_putw(self, word), 0)
#define bytewriter_putl(self, dword) __builtin_expect(bytewriter_putl(self, dword), 0)
#define bytewriter_putq(self, qword) __builtin_expect(bytewriter_putq(self, qword), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */


/* Deallocate all unused bytes, and return a heap-allocated pointer to what has been written.
 * When nothing has been written, `NULL' may be returned, which does not indicate an error,
 * as this function will never throw any. */
LOCAL uint8_t *DCALL
bytewriter_flush(struct bytewriter *__restrict self) {
	Dee_ASSERT(self->bw_alloc >= self->bw_size);
	if (self->bw_alloc > self->bw_size) {
		uint8_t *newbase;
		newbase = (uint8_t *)Dee_TryRealloc(self->bw_base,
		                                    self->bw_size);
		if likely(newbase) {
			self->bw_base  = newbase;
			self->bw_alloc = self->bw_size;
		}
	}
	return self->bw_base;
}


DECL_END

#endif /* !GUARD_DEEMON_UTIL_BYTEWRITER_H */
