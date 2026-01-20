/* Copyright (c) 2016 - deemon by Griefer@Work                                    *
 *                                                                                *
 * Permission is hereby granted, free of charge, to any person obtaining a copy   *
 * of this software and associated documentation files (the "Software"), to deal  *
 * in the Software without restriction, including without limitation the rights   *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      *
 * copies of the Software, and to permit persons to whom the Software is          *
 * furnished to do so, subject to the following conditions:                       *
 *                                                                                *
 * The above copyright notice and this permission notice shall be included in all *
 * copies or substantial portions of the Software.                                *
 *                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  *
 * SOFTWARE.                                                                      *
 */

#ifndef WIDTH
#error "Must '#define WIDTH' before #including this file"
#endif
#ifndef IN_REFLECTED
#error "Must '#define IN_REFLECTED' before #including this file"
#endif
#ifndef OUT_REFLECTED
#error "Must '#define OUT_REFLECTED' before #including this file"
#endif
#if IN_REFLECTED != 0 && IN_REFLECTED != 1
#error "'IN_REFLECTED' must be defined as 0|1"
#endif
#if OUT_REFLECTED != 0 && OUT_REFLECTED != 1
#error "'OUT_REFLECTED' must be defined as 0|1"
#endif

#include "libhash.h"
/**/

#include <deemon/api.h>

#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintN_t */

#if WIDTH > 32
#define SIZE   64
#define HASH_T uint64_t
#elif WIDTH > 16
#define SIZE   32
#define HASH_T uint32_t
#elif WIDTH > 8
#define SIZE   16
#define HASH_T uint16_t
#else
#define SIZE   8
#define HASH_T uint8_t
#endif

#if IN_REFLECTED && OUT_REFLECTED
#define NAME PP_CAT3(_hashimpl_, WIDTH, _ioref)
#elif OUT_REFLECTED
#define NAME PP_CAT3(_hashimpl_, WIDTH, _oref)
#elif OUT_REFLECTED
#define NAME PP_CAT3(_hashimpl_, WIDTH, _oref)
#else
#define NAME PP_CAT2(_hashimpl_, WIDTH)
#endif

#define ALGO PP_CAT2(dhashalgo, SIZE)


PRIVATE HASH_T DCALL
NAME(struct ALGO const *self, HASH_T start, void const *data, size_t datasize) {
	__BYTE_TYPE__ *p = (__BYTE_TYPE__ *)data;
	ASSERT(self);
	ASSERT(self->ha_base.ha_width == WIDTH);
	ASSERT(self->ha_base.ha_size == SIZE / 8);
	ASSERT(!datasize || p);
#if SIZE != WIDTH
	start <<= (SIZE - WIDTH);
#endif
#if SIZE > 8
#if OUT_REFLECTED
	while (datasize--)
		start = (HASH_T)(self->ha_table[(uint8_t)(((start >> (SIZE - WIDTH)) ^ *p++) & 0xFF)] ^ (start >> 8));
#if !IN_REFLECTED && 0
	start >>= (SIZE - WIDTH);
	{
		unsigned int i;
		for (i = 0; i < 12; i++) {
			if ((start & 0x800) != 0) {
				start <<= 1;
				start ^= 0x80f;
			} else {
				start <<= 1;
			}
		}
		/*start = crc_reflect(start,12);*/
		return start & 0xFFF;
	}
#endif
#else /* OUT_REFLECTED */
	while (datasize--) {
		start = (HASH_T)(self->ha_table[(uint8_t)((start >> ((SIZE - WIDTH) + (WIDTH - 8))) ^ *p++)] ^ (start << 8));
	}
#endif /* !OUT_REFLECTED */
#else /* SIZE > 8 */
#if OUT_REFLECTED && (SIZE != WIDTH)
	while (datasize--) {
		start = (HASH_T)(self->ha_table[(uint8_t)((start >> (SIZE - WIDTH)) ^ *p++)]);
	}
#else /* OUT_REFLECTED && (SIZE != WIDTH) */
	while (datasize--) {
		start = (HASH_T)(self->ha_table[start ^ *p++]);
	}
#endif /* !OUT_REFLECTED || (SIZE == WIDTH) */
#endif /* SIZE <= 8 */
#if SIZE != WIDTH
	start >>= (SIZE - WIDTH);
#endif /* SIZE != WIDTH */
	return start;
}





#undef ALGO
#undef NAME
#undef HASH_T
#undef SIZE
#undef WIDTH
#undef IN_REFLECTED
#undef OUT_REFLECTED
