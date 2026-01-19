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
#ifndef GUARD_DEEMON_RUNTIME_MD5_C
#define GUARD_DEEMON_RUNTIME_MD5_C 1

#include <deemon/api.h>

#include <deemon/util/md5.h>

#include <hybrid/byteswap.h>
#include <hybrid/typecore.h>

#include <stdint.h>

#ifndef UINT32_C
#define UINT32_C __UINT32_C
#endif /* !UINT32_C */

#undef byte_t
#define byte_t __BYTE_TYPE__
#undef shift_t
#define shift_t __SHIFT_TYPE__

DECL_BEGIN

/* Based on https://github.com/Zunawe/md5-c  (which is public domain) */

#define A UINT32_C(0x67452301)
#define B UINT32_C(0xefcdab89)
#define C UINT32_C(0x98badcfe)
#define D UINT32_C(0x10325476)

/* clang-format off */
PRIVATE shift_t const S[] = {
	7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
	5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
	4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
	6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};
/* clang-format on */

PRIVATE uint32_t const K[] = {
	UINT32_C(0xd76aa478), UINT32_C(0xe8c7b756), UINT32_C(0x242070db), UINT32_C(0xc1bdceee),
	UINT32_C(0xf57c0faf), UINT32_C(0x4787c62a), UINT32_C(0xa8304613), UINT32_C(0xfd469501),
	UINT32_C(0x698098d8), UINT32_C(0x8b44f7af), UINT32_C(0xffff5bb1), UINT32_C(0x895cd7be),
	UINT32_C(0x6b901122), UINT32_C(0xfd987193), UINT32_C(0xa679438e), UINT32_C(0x49b40821),
	UINT32_C(0xf61e2562), UINT32_C(0xc040b340), UINT32_C(0x265e5a51), UINT32_C(0xe9b6c7aa),
	UINT32_C(0xd62f105d), UINT32_C(0x02441453), UINT32_C(0xd8a1e681), UINT32_C(0xe7d3fbc8),
	UINT32_C(0x21e1cde6), UINT32_C(0xc33707d6), UINT32_C(0xf4d50d87), UINT32_C(0x455a14ed),
	UINT32_C(0xa9e3e905), UINT32_C(0xfcefa3f8), UINT32_C(0x676f02d9), UINT32_C(0x8d2a4c8a),
	UINT32_C(0xfffa3942), UINT32_C(0x8771f681), UINT32_C(0x6d9d6122), UINT32_C(0xfde5380c),
	UINT32_C(0xa4beea44), UINT32_C(0x4bdecfa9), UINT32_C(0xf6bb4b60), UINT32_C(0xbebfbc70),
	UINT32_C(0x289b7ec6), UINT32_C(0xeaa127fa), UINT32_C(0xd4ef3085), UINT32_C(0x04881d05),
	UINT32_C(0xd9d4d039), UINT32_C(0xe6db99e5), UINT32_C(0x1fa27cf8), UINT32_C(0xc4ac5665),
	UINT32_C(0xf4292244), UINT32_C(0x432aff97), UINT32_C(0xab9423a7), UINT32_C(0xfc93a039),
	UINT32_C(0x655b59c3), UINT32_C(0x8f0ccc92), UINT32_C(0xffeff47d), UINT32_C(0x85845dd1),
	UINT32_C(0x6fa87e4f), UINT32_C(0xfe2ce6e0), UINT32_C(0xa3014314), UINT32_C(0x4e0811a1),
	UINT32_C(0xf7537e82), UINT32_C(0xbd3af235), UINT32_C(0x2ad7d2bb), UINT32_C(0xeb86d391)
};

PRIVATE uint8_t const PADDING[] = {
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define F(X, Y, Z) ((X & Y) | (~X & Z))
#define G(X, Y, Z) ((X & Z) | (Y & ~Z))
#define H(X, Y, Z) (X ^ Y ^ Z)
#define I(X, Y, Z) (Y ^ (X | ~Z))

PRIVATE ATTR_CONST uint32_t DCALL
md5_rol(uint32_t x, shift_t n) {
	return (x << n) | (x >> (32 - n));
}


PUBLIC NONNULL((1)) void DCALL
DeeMD5_Init(DeeMD5_Context *__restrict ctx) {
	ctx->dmd5_size      = 0;
	ctx->dmd5_buffer[0] = A;
	ctx->dmd5_buffer[1] = B;
	ctx->dmd5_buffer[2] = C;
	ctx->dmd5_buffer[3] = D;
}

PRIVATE NONNULL((1, 2)) void DCALL
md5Step(uint32_t *__restrict buffer, uint32_t const input[16]) {
	uint32_t AA = buffer[0];
	uint32_t BB = buffer[1];
	uint32_t CC = buffer[2];
	uint32_t DD = buffer[3];
	uint32_t E;
	unsigned int i, j;

	for (i = 0; i < 64; ++i) {
		uint32_t temp;
		switch (i / 16) {
		case 0:
			E = F(BB, CC, DD);
			j = i;
			break;
		case 1:
			E = G(BB, CC, DD);
			j = ((i * 5) + 1) % 16;
			break;
		case 2:
			E = H(BB, CC, DD);
			j = ((i * 3) + 5) % 16;
			break;
		default:
			E = I(BB, CC, DD);
			j = (i * 7) % 16;
			break;
		}
		temp = DD;
		DD   = CC;
		CC   = BB;
		BB   = BB + md5_rol(AA + E + K[i] + input[j], S[i]);
		AA   = temp;
	}

	buffer[0] += AA;
	buffer[1] += BB;
	buffer[2] += CC;
	buffer[3] += DD;
}

PUBLIC NONNULL((1)) void DCALL
DeeMD5_Update(DeeMD5_Context *__restrict ctx, void const *data, size_t input_len) {
	size_t i;
	unsigned int offset = ctx->dmd5_size & 63;
	ctx->dmd5_size += input_len;
	for (i = 0; i < input_len; ++i) {
		ctx->dmd5_input[offset++] = ((byte_t const *)data)[i];
		if (offset == 64) {
			uint32_t temp[16];
			unsigned int j;
			for (j = 0; j < 16; ++j) {
				byte_t const *base = &ctx->dmd5_input[j * 4];
#if 1
				temp[j] = LETOH32(*(uint32_t const *)base);
#else
				temp[j] = (uint32_t)(base[3]) << 24 |
				          (uint32_t)(base[2]) << 16 |
				          (uint32_t)(base[1]) << 8 |
				          (uint32_t)(base[0]);
#endif
			}
			md5Step(ctx->dmd5_buffer, temp);
			offset = 0;
		}
	}
}

PUBLIC ATTR_OUT(2) NONNULL((1)) void DCALL
DeeMD5_Finalize(DeeMD5_Context *__restrict ctx, uint32_t result[4]) {
	uint32_t temp[16];
	unsigned int i, j;
	unsigned int offset         = ctx->dmd5_size & 63;
	unsigned int padding_length = offset < 56 ? 56 - offset : (56 + 64) - offset;
	DeeMD5_Update(ctx, PADDING, padding_length);
	ctx->dmd5_size -= (uint64_t)padding_length;
	for (j = 0; j < 14; ++j) {
		byte_t const *base = &ctx->dmd5_input[j * 4];
#if 1
		temp[j] = LETOH32(*(uint32_t const *)base);
#else
		temp[j] = (uint32_t)(base[3]) << 24 |
		          (uint32_t)(base[2]) << 16 |
		          (uint32_t)(base[1]) << 8 |
		          (uint32_t)(base[0]);
#endif
	}
	temp[14] = (uint32_t)(ctx->dmd5_size * 8);
	temp[15] = (uint32_t)((ctx->dmd5_size * 8) >> 32);
	md5Step(ctx->dmd5_buffer, temp);

	for (i = 0; i < 4; ++i) {
#if 1
		result[i] = (uint32_t)HTOLE32(ctx->dmd5_buffer[i]);
#else
		union {
			uint32_t w;
			uint8_t b[4];
		} temp;
		temp.b[0] = (uint8_t)((ctx->dmd5_buffer[i] & 0x000000FF));
		temp.b[1] = (uint8_t)((ctx->dmd5_buffer[i] & 0x0000FF00) >> 8);
		temp.b[2] = (uint8_t)((ctx->dmd5_buffer[i] & 0x00FF0000) >> 16);
		temp.b[3] = (uint8_t)((ctx->dmd5_buffer[i] & 0xFF000000) >> 24);
		result[i] = temp.w;
#endif
	}
}

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_MD5_C */
