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
#ifndef GUARD_DEEMON_UTIL_MD5_H
#define GUARD_DEEMON_UTIL_MD5_H 1

#include "../api.h"
/**/

#include <hybrid/typecore.h>

#include <stdint.h>

DECL_BEGIN

/* Based on https://github.com/Zunawe/md5-c  (which is public domain) */

/* Usage:
 * >> DeeMD5_Context ctx;
 * >> DeeMD5_Init(&ctx);
 * >> DeeMD5_Update(&ctx, DATA1, NUM_BYTES1);
 * >> DeeMD5_Update(&ctx, DATA2, NUM_BYTES2);
 * >> DeeMD5_Update(&ctx, DATA3, NUM_BYTES3);
 * >> DeeMD5_Finalize(&ctx);
 * >> memcpy(result, ctx.digest, 16);
 */
typedef struct {
	uint64_t      dmd5_size;       /* Size of input in bytes */
	uint32_t      dmd5_buffer[4];  /* Current accumulation of hash */
	__BYTE_TYPE__ dmd5_input[64];  /* Input to be used in the next step */
} DeeMD5_Context;

DFUNDEF NONNULL((1)) void DCALL
DeeMD5_Init(DeeMD5_Context *__restrict ctx);
DFUNDEF NONNULL((1)) void DCALL
DeeMD5_Update(DeeMD5_Context *__restrict ctx, void const *input, size_t input_len);
DFUNDEF ATTR_OUT(2) NONNULL((1)) void DCALL
DeeMD5_Finalize(DeeMD5_Context *__restrict ctx, uint32_t result[4]);

DECL_END

#endif /* !GUARD_DEEMON_UTIL_MD5_H */
