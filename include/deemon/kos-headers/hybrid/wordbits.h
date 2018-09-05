/* Copyright (c) 2018 Griefer@Work                                            *
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
#ifndef __GUARD_HYBRID_WORDBITS_H
#define __GUARD_HYBRID_WORDBITS_H 1

#include "compiler.h"
#include "__byteorder.h"

DECL_BEGIN

/* Return the ith byte/word/dword of a given 16/32/64-bit integer. */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define INT16_I8(x,i)                 (((x) >> (i)*8) & 0xff)
#define INT32_I8(x,i)                 (((x) >> (i)*8) & 0xff)
#define INT64_I8(x,i)                 (((x) >> (i)*8) & 0xff)
#define INT32_I16(x,i)                (((x) >> (i)*16) & 0xffff)
#define INT64_I16(x,i)                (((x) >> (i)*16) & 0xffff)
#define INT64_I32(x,i)                (((x) >> (i)*32) & 0xffffffff)
#define ENCODE_INT16(a,b)              ((b)<<8|(a))
#define ENCODE_INT32(a,b,c,d)          ((d)<<24|(c)<<16|(b)<<8|(a))
#define ENCODE_INT64(a,b,c,d,e,f,g,h)  ((h)<<56|(g)<<48|(f)<<40|(e)<<32|(d)<<24|(c)<<16|(b)<<8|(a))
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define INT16_I8(x,i)                 (((x) >> (8-(i)*8)) & 0xff)
#define INT32_I8(x,i)                 (((x) >> (24-(i)*8)) & 0xff)
#define INT64_I8(x,i)                 (((x) >> (56-(i)*8)) & 0xff)
#define INT32_I16(x,i)                (((x) >> (16-(i)*16)) & 0xffff)
#define INT64_I16(x,i)                (((x) >> (48-(i)*16)) & 0xffff)
#define INT64_I32(x,i)                (((x) >> (32-(i)*32)) & 0xffffffff)
#define ENCODE_INT16(a,b)              ((b)|(a)<<8)
#define ENCODE_INT32(a,b,c,d)          ((d)|(c)<<8|(b)<<16|(a)<<24)
#define ENCODE_INT64(a,b,c,d,e,f,g,h)  ((h)|(g)<<8|(f)<<16|(e)<<24|(d)<<32|(c)<<40|(b)<<48|(a)<<56)
#endif

/* Commonly used aliases. */
#define INT16_BYTE(x,i)  INT16_I8(x,i)
#define INT32_BYTE(x,i)  INT32_I8(x,i)
#define INT64_BYTE(x,i)  INT64_I8(x,i)
#define INT32_WORD(x,i)  INT32_I16(x,i)
#define INT64_WORD(x,i)  INT64_I16(x,i)
#define INT64_DWORD(x,i) INT64_I32(x,i)

DECL_END

#endif /* !__GUARD_HYBRID_WORDBITS_H */
