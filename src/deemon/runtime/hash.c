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
#ifndef GUARD_DEEMON_RUNTIME_MISC_C
#define GUARD_DEEMON_RUNTIME_MISC_C 1

#include <deemon/api.h>

#include <deemon/object.h>          /* Dee_SIZEOF_HASH_T, Dee_hash_t, _Dee_HashSelect */
#include <deemon/string.h>          /* DeeUni_ToLower */
#include <deemon/stringutils.h>     /* Dee_unicode_readutf8_n */
#include <deemon/system-features.h> /* strlen */

#include <hybrid/byteorder.h> /* __BYTE_ORDER__, __ORDER_LITTLE_ENDIAN__ */
#include <hybrid/byteswap.h>  /* UNALIGNED_GETLE32, UNALIGNED_GETLE64 */
#include <hybrid/typecore.h>  /* __BYTE_TYPE__ */
#include <hybrid/unaligned.h> /* UNALIGNED_GETLE32, UNALIGNED_GETLE64 */

#include <stddef.h> /* size_t */
#include <stdint.h> /* UINT16_C, UINT32_C, UINT64_C, uint8_t, uint16_t, uint32_t */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

#define packw(x) \
	(((x)&UINT16_C(0xff)) ^ (((x)&UINT16_C(0xff00)) >> 8))
#define packl(x)                                            \
	(((x)&UINT32_C(0xff)) ^ (((x)&UINT32_C(0xff00)) >> 8) ^ \
	 (((x)&UINT32_C(0xff0000)) >> 16) ^ (((x)&UINT32_C(0xff000000)) >> 24))


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define ESEL(l, b) l
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
#define ESEL(l, b) b
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */



/* Combine 2 hash values into 1, while losing as little entropy
 * from either as possible. Note that this function tries to
 * include the order of arguments in the result, meaning that:
 * >> Dee_HashCombine(a, b) != Dee_HashCombine(b, a) */
PUBLIC ATTR_CONST WUNUSED Dee_hash_t
(DFCALL Dee_HashCombine)(Dee_hash_t a, Dee_hash_t b) {
	/* Taken from https://stackoverflow.com/a/27952689/3296587 */
#if Dee_SIZEOF_HASH_T >= 8
	a ^= b + UINT64_C(0x517cc1b727220a95) + (a << 6) + (a >> 2);
#else /* Dee_SIZEOF_HASH_T >= 8 */
	a ^= b + UINT32_C(0x9e3779b9) + (a << 6) + (a >> 2);
#endif /* Dee_SIZEOF_HASH_T < 8 */
	return a;
}


#if _Dee_HashSelect(32, 64) == 32
// This Hash function is based on code from here:
// https://en.wikipedia.org/wiki/MurmurHash
// It was referenced as pretty good here:
// http://programmers.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed

#define ROT32(x, y) ((x << y) | (x >> (32 - y))) // avoid effort
#define c1 UINT32_C(0xcc9e2d51)
#define c2 UINT32_C(0x1b873593)
#define r1 15
#define r2 13
#define m  5
#define n  UINT32_C(0xe6546b64)
PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_HashPtr(void const *__restrict ptr, size_t n_bytes) {
	uint8_t const *tail;
	uint32_t k1;
	size_t i;
	size_t const nblocks   = n_bytes >> 2;
	uint32_t const *blocks = (uint32_t const *)ptr;
	uint32_t hash          = 0;
	uint32_t k;
	for (i = 0; i < nblocks; ++i) {
		k = UNALIGNED_GETLE32(&blocks[i]);
		k *= c1;
		k = ROT32(k, r1);
		k *= c2;
		hash ^= k;
		hash = ROT32(hash, r2) * m + n;
	}
	tail = ((uint8_t const *)ptr) + (nblocks << 2);
	k1   = 0;
	switch (n_bytes & 3) {
	case 3:
		k1 ^= (uint32_t)tail[2] << 16;
		ATTR_FALLTHROUGH
	case 2:
		k1 ^= (uint32_t)tail[1] << 8;
		ATTR_FALLTHROUGH
	case 1:
		k1 ^= (uint32_t)tail[0];
		k1 *= c1;
		k1 = ROT32(k1, r1);
		k1 *= c2;
		hash ^= k1;
		break;
	default: break;
	}
	hash ^= n_bytes;
	hash ^= (hash >> 16);
	hash *= UINT32_C(0x85ebca6b);
	hash ^= (hash >> 13);
	hash *= UINT32_C(0xc2b2ae35);
	hash ^= (hash >> 16);
	return hash;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_Hash2Byte(uint16_t const *__restrict ptr, size_t n_words) {
	uint16_t const *tail;
	uint32_t k1;
	size_t i;
	size_t const nblocks = n_words >> 2;
	uint32_t hash        = 0;
	for (i = 0; i < nblocks; ++i) {
		uint32_t k;
		uint16_t ch;
		ch = ptr[(i << 2) + 0];
		k  = packw(ch) << ESEL(0, 24);
		ch = ptr[(i << 2) + 1];
		k |= packw(ch) << ESEL(8, 16);
		ch = ptr[(i << 2) + 2];
		k |= packw(ch) << ESEL(16, 8);
		ch = ptr[(i << 2) + 3];
		k |= packw(ch) << ESEL(24, 0);
		k *= c1;
		k = ROT32(k, r1);
		k *= c2;
		hash ^= k;
		hash = ROT32(hash, r2) * m + n;
	}
	tail = ((uint16_t const *)ptr) + (nblocks << 2);
	k1   = 0;
	switch (n_words & 3) {
	case 3:
		k1 ^= packw(tail[2]) << 16;
		ATTR_FALLTHROUGH
	case 2:
		k1 ^= packw(tail[1]) << 8;
		ATTR_FALLTHROUGH
	case 1:
		k1 ^= packw(tail[0]);
		k1 *= c1;
		k1 = ROT32(k1, r1);
		k1 *= c2;
		hash ^= k1;
		break;
	default: break;
	}
	hash ^= n_words;
	hash ^= (hash >> 16);
	hash *= UINT32_C(0x85ebca6b);
	hash ^= (hash >> 13);
	hash *= UINT32_C(0xc2b2ae35);
	hash ^= (hash >> 16);
	return hash;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_Hash4Byte(uint32_t const *__restrict ptr, size_t n_dwords) {
	uint32_t const *tail;
	uint32_t k1;
	size_t i;
	size_t const nblocks = n_dwords >> 2;
	uint32_t hash        = 0;
	uint32_t k;
	uint32_t ch;
	for (i = 0; i < nblocks; ++i) {
		ch = ptr[(i << 2) + 0];
		k  = packl(ch) << ESEL(0, 24);
		ch = ptr[(i << 2) + 1];
		k |= packl(ch) << ESEL(8, 16);
		ch = ptr[(i << 2) + 2];
		k |= packl(ch) << ESEL(16, 8);
		ch = ptr[(i << 2) + 3];
		k |= packl(ch) << ESEL(24, 0);
		k *= c1;
		k = ROT32(k, r1);
		k *= c2;
		hash ^= k;
		hash = ROT32(hash, r2) * m + n;
	}
	tail = ((uint32_t const *)ptr) + (nblocks << 2);
	k1   = 0;
	switch (n_dwords & 3) {
	case 3:
		ch = tail[2];
		k1 ^= packl(ch) << 16;
		ATTR_FALLTHROUGH
	case 2:
		ch = tail[1];
		k1 ^= packl(ch) << 8;
		ATTR_FALLTHROUGH
	case 1:
		ch = tail[0];
		k1 ^= packl(ch);
		k1 *= c1;
		k1 = ROT32(k1, r1);
		k1 *= c2;
		hash ^= k1;
		ATTR_FALLTHROUGH
	default:
		break;
	}
	hash ^= n_dwords;
	hash ^= (hash >> 16);
	hash *= UINT32_C(0x85ebca6b);
	hash ^= (hash >> 13);
	hash *= UINT32_C(0xc2b2ae35);
	hash ^= (hash >> 16);
	return hash;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_HashCasePtr(void const *__restrict ptr, size_t n_bytes) {
	uint8_t const *tail;
	uint32_t k1;
	size_t i;
	size_t const nblocks   = n_bytes >> 2;
	uint32_t const *blocks = (uint32_t const *)ptr;
	uint32_t hash          = 0;
	uint32_t k;
	for (i = 0; i < nblocks; ++i) {
		union {
			uint32_t block;
			char part[4];
		} b;
		b.block   = UNALIGNED_GETLE32(&blocks[i]);
		b.part[0] = (char)DeeUni_ToLower(b.part[0]);
		b.part[1] = (char)DeeUni_ToLower(b.part[1]);
		b.part[2] = (char)DeeUni_ToLower(b.part[2]);
		b.part[3] = (char)DeeUni_ToLower(b.part[3]);
		k         = b.block;
		k *= c1;
		k = ROT32(k, r1);
		k *= c2;
		hash ^= k;
		hash = ROT32(hash, r2) * m + n;
	}
	tail = ((uint8_t const *)ptr) + (nblocks << 2);
	k1   = 0;
	switch (n_bytes & 3) {
	case 3:
		k1 ^= (uint8_t)DeeUni_ToLower(tail[2]) << 16;
		ATTR_FALLTHROUGH
	case 2:
		k1 ^= (uint8_t)DeeUni_ToLower(tail[1]) << 8;
		ATTR_FALLTHROUGH
	case 1:
		k1 ^= (uint8_t)DeeUni_ToLower(tail[0]);
		k1 *= c1;
		k1 = ROT32(k1, r1);
		k1 *= c2;
		hash ^= k1;
		ATTR_FALLTHROUGH
	default: break;
	}
	hash ^= n_bytes;
	hash ^= (hash >> 16);
	hash *= UINT32_C(0x85ebca6b);
	hash ^= (hash >> 13);
	hash *= UINT32_C(0xc2b2ae35);
	hash ^= (hash >> 16);
	return hash;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_HashCase2Byte(uint16_t const *__restrict ptr, size_t n_words) {
	uint16_t const *tail;
	uint32_t k1;
	size_t i;
	size_t const nblocks = n_words >> 2;
	uint32_t hash        = 0;
	uint32_t k;
	uint32_t ch;
	for (i = 0; i < nblocks; ++i) {
		ch = DeeUni_ToLower(ptr[(i << 2) + 0]);
		k  = packl(ch) << ESEL(0, 24);
		ch = DeeUni_ToLower(ptr[(i << 2) + 1]);
		k |= packl(ch) << ESEL(8, 16);
		ch = DeeUni_ToLower(ptr[(i << 2) + 2]);
		k |= packl(ch) << ESEL(16, 8);
		ch = DeeUni_ToLower(ptr[(i << 2) + 3]);
		k |= packl(ch) << ESEL(24, 0);
		k *= c1;
		k = ROT32(k, r1);
		k *= c2;
		hash ^= k;
		hash = ROT32(hash, r2) * m + n;
	}
	tail = ((uint16_t const *)ptr) + (nblocks << 2);
	k1   = 0;
	switch (n_words & 3) {
	case 3:
		k1 ^= packw(DeeUni_ToLower(tail[2])) << 16;
		ATTR_FALLTHROUGH
	case 2:
		k1 ^= packw(DeeUni_ToLower(tail[1])) << 8;
		ATTR_FALLTHROUGH
	case 1:
		k1 ^= packw(DeeUni_ToLower(tail[0]));
		k1 *= c1;
		k1 = ROT32(k1, r1);
		k1 *= c2;
		hash ^= k1;
		ATTR_FALLTHROUGH
	default: break;
	}
	hash ^= n_words;
	hash ^= (hash >> 16);
	hash *= UINT32_C(0x85ebca6b);
	hash ^= (hash >> 13);
	hash *= UINT32_C(0xc2b2ae35);
	hash ^= (hash >> 16);
	return hash;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_HashCase4Byte(uint32_t const *__restrict ptr, size_t n_dwords) {
	uint32_t const *tail;
	uint32_t k1;
	size_t i;
	size_t const nblocks = n_dwords >> 2;
	uint32_t hash        = 0;
	uint32_t k;
	uint32_t ch;
	for (i = 0; i < nblocks; ++i) {
		ch = DeeUni_ToLower(ptr[(i << 2) + 0]);
		k  = packl(ch) << ESEL(0, 24);
		ch = DeeUni_ToLower(ptr[(i << 2) + 1]);
		k |= packl(ch) << ESEL(8, 16);
		ch = DeeUni_ToLower(ptr[(i << 2) + 2]);
		k |= packl(ch) << ESEL(16, 8);
		ch = DeeUni_ToLower(ptr[(i << 2) + 3]);
		k |= packl(ch) << ESEL(24, 0);
		k *= c1;
		k = ROT32(k, r1);
		k *= c2;
		hash ^= k;
		hash = ROT32(hash, r2) * m + n;
	}
	tail = ((uint32_t const *)ptr) + (nblocks << 2);
	k1   = 0;
	switch (n_dwords & 3) {
	case 3:
		ch = DeeUni_ToLower(tail[2]);
		k1 ^= packl(ch) << 16;
		ATTR_FALLTHROUGH
	case 2:
		ch = DeeUni_ToLower(tail[1]);
		k1 ^= packl(ch) << 8;
		ATTR_FALLTHROUGH
	case 1:
		ch = DeeUni_ToLower(tail[0]);
		k1 ^= packl(ch);
		k1 *= c1;
		k1 = ROT32(k1, r1);
		k1 *= c2;
		hash ^= k1;
		ATTR_FALLTHROUGH
	default: break;
	}
	hash ^= n_dwords;
	hash ^= (hash >> 16);
	hash *= UINT32_C(0x85ebca6b);
	hash ^= (hash >> 13);
	hash *= UINT32_C(0xc2b2ae35);
	hash ^= (hash >> 16);
	return hash;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_HashUtf8(char const *__restrict ptr, size_t n_bytes) {
	char const *end = ptr + n_bytes;
	uint32_t k1     = 0;
	uint32_t block[4];
	uint32_t hash = 0;
	uint32_t k;
	size_t n_chars = 0;
	for (;;) {
		if unlikely(ptr >= end)
			goto done;
		block[0] = Dee_unicode_readutf8_n(&ptr, end);
		if unlikely(ptr >= end)
			goto do_tail_1;
		block[1] = Dee_unicode_readutf8_n(&ptr, end);
		if unlikely(ptr >= end)
			goto do_tail_2;
		block[2] = Dee_unicode_readutf8_n(&ptr, end);
		if unlikely(ptr >= end)
			goto do_tail_3;
		block[3] = Dee_unicode_readutf8_n(&ptr, end);
		n_chars += 4;
		k = packl(block[0]) << ESEL(0, 24);
		k |= packl(block[1]) << ESEL(8, 16);
		k |= packl(block[2]) << ESEL(16, 8);
		k |= packl(block[3]) << ESEL(24, 0);
		k *= c1;
		k = ROT32(k, r1);
		k *= c2;
		hash ^= k;
		hash = ROT32(hash, r2) * m + n;
	}
	goto done;
do_tail_3:
	++n_chars;
	k1 ^= packl(block[2]) << 16;
do_tail_2:
	++n_chars;
	k1 ^= packl(block[1]) << 8;
do_tail_1:
	++n_chars;
	k1 ^= packl(block[0]);
	k1 *= c1;
	k1 = ROT32(k1, r1);
	k1 *= c2;
	hash ^= k1;
done:
	hash ^= n_chars;
	hash ^= (hash >> 16);
	hash *= UINT32_C(0x85ebca6b);
	hash ^= (hash >> 13);
	hash *= UINT32_C(0xc2b2ae35);
	hash ^= (hash >> 16);
	return hash;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_HashCaseUtf8(char const *__restrict ptr, size_t n_bytes) {
	char const *end = ptr + n_bytes;
	uint32_t k1     = 0;
	uint32_t block[4];
	uint32_t hash = 0;
	uint32_t k;
	size_t n_chars = 0;
	for (;;) {
		if unlikely(ptr >= end)
			goto done;
		block[0] = Dee_unicode_readutf8_n(&ptr, end);
		block[0] = DeeUni_ToLower(block[0]);
		if unlikely(ptr >= end)
			goto do_tail_1;
		block[1] = Dee_unicode_readutf8_n(&ptr, end);
		block[1] = DeeUni_ToLower(block[1]);
		if unlikely(ptr >= end)
			goto do_tail_2;
		block[2] = Dee_unicode_readutf8_n(&ptr, end);
		block[2] = DeeUni_ToLower(block[2]);
		if unlikely(ptr >= end)
			goto do_tail_3;
		block[3] = Dee_unicode_readutf8_n(&ptr, end);
		block[3] = DeeUni_ToLower(block[3]);
		n_chars += 4;
		k = packl(block[0]) << ESEL(0, 24);
		k |= packl(block[1]) << ESEL(8, 16);
		k |= packl(block[2]) << ESEL(16, 8);
		k |= packl(block[3]) << ESEL(24, 0);
		k *= c1;
		k = ROT32(k, r1);
		k *= c2;
		hash ^= k;
		hash = ROT32(hash, r2) * m + n;
	}
	goto done;
do_tail_3:
	++n_chars;
	k1 ^= packl(block[2]) << 16;
do_tail_2:
	++n_chars;
	k1 ^= packl(block[1]) << 8;
do_tail_1:
	++n_chars;
	k1 ^= packl(block[0]);
	k1 *= c1;
	k1 = ROT32(k1, r1);
	k1 *= c2;
	hash ^= k1;
done:
	hash ^= n_chars;
	hash ^= (hash >> 16);
	hash *= UINT32_C(0x85ebca6b);
	hash ^= (hash >> 13);
	hash *= UINT32_C(0xc2b2ae35);
	hash ^= (hash >> 16);
	return hash;
}


#undef c1
#undef c2
#undef r1
#undef r2
#undef m
#undef n

#elif _Dee_HashSelect(32, 64) == 64

#define m    UINT64_C(0xc6a4a7935bd1e995)
#define r    47
//#define seed 0xe17a1465
PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_HashPtr(void const *__restrict ptr, size_t n_bytes) {
#ifdef seed
	Dee_hash_t h = seed ^ (n_bytes * m);
#else /* seed */
	Dee_hash_t h = 0;
#endif /* !seed */
	size_t len8 = n_bytes >> 3;
	while (len8--) {
		Dee_hash_t k;
		k = UNALIGNED_GETLE64((Dee_hash_t *)ptr);
		ptr = (void const *)((byte_t *)ptr + sizeof(Dee_hash_t));
		k *= m;
		k ^= k >> r;
		k *= m;
		h ^= k;
		h *= m;
	}
	switch (n_bytes & 7) {
	case 7:
		h ^= (Dee_hash_t)((uint8_t *)ptr)[6] << 48;
		ATTR_FALLTHROUGH
	case 6:
		h ^= (Dee_hash_t)((uint8_t *)ptr)[5] << 40;
		ATTR_FALLTHROUGH
	case 5:
		h ^= (Dee_hash_t)((uint8_t *)ptr)[4] << 32;
		ATTR_FALLTHROUGH
	case 4:
		h ^= (Dee_hash_t)((uint8_t *)ptr)[3] << 24;
		ATTR_FALLTHROUGH
	case 3:
		h ^= (Dee_hash_t)((uint8_t *)ptr)[2] << 16;
		ATTR_FALLTHROUGH
	case 2:
		h ^= (Dee_hash_t)((uint8_t *)ptr)[1] << 8;
		ATTR_FALLTHROUGH
	case 1:
		h ^= (Dee_hash_t)((uint8_t *)ptr)[0];
		h *= m;
		break;
	default: break;
	}
	h ^= h >> r;
	h *= m;
	h ^= h >> r;
	return h;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_Hash2Byte(uint16_t const *__restrict ptr, size_t n_words) {
#ifdef seed
	Dee_hash_t h = seed ^ (n_words * m);
#else /* seed */
	Dee_hash_t h = 0;
#endif /* !seed */
	size_t len8 = n_words >> 3;
	while (len8--) {
		Dee_hash_t k;
		k = (Dee_hash_t)packw(ptr[0]) << ESEL(0, 56);
		k |= (Dee_hash_t)packw(ptr[1]) << ESEL(8, 48);
		k |= (Dee_hash_t)packw(ptr[2]) << ESEL(16, 40);
		k |= (Dee_hash_t)packw(ptr[3]) << ESEL(24, 32);
		k |= (Dee_hash_t)packw(ptr[4]) << ESEL(32, 24);
		k |= (Dee_hash_t)packw(ptr[5]) << ESEL(40, 16);
		k |= (Dee_hash_t)packw(ptr[6]) << ESEL(48, 8);
		k |= (Dee_hash_t)packw(ptr[7]) << ESEL(56, 0);
		ptr += 8;
		k *= m;
		k ^= k >> r;
		k *= m;
		h ^= k;
		h *= m;
	}
	switch (n_words & 7) {
	case 7:
		h ^= (Dee_hash_t)packw(ptr[6]) << 48;
		ATTR_FALLTHROUGH
	case 6:
		h ^= (Dee_hash_t)packw(ptr[5]) << 40;
		ATTR_FALLTHROUGH
	case 5:
		h ^= (Dee_hash_t)packw(ptr[4]) << 32;
		ATTR_FALLTHROUGH
	case 4:
		h ^= (Dee_hash_t)packw(ptr[3]) << 24;
		ATTR_FALLTHROUGH
	case 3:
		h ^= (Dee_hash_t)packw(ptr[2]) << 16;
		ATTR_FALLTHROUGH
	case 2:
		h ^= (Dee_hash_t)packw(ptr[1]) << 8;
		ATTR_FALLTHROUGH
	case 1:
		h ^= (Dee_hash_t)packw(ptr[0]);
		h *= m;
		break;
	default: break;
	}
	h ^= h >> r;
	h *= m;
	h ^= h >> r;
	return h;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_Hash4Byte(uint32_t const *__restrict ptr, size_t n_dwords) {
#ifdef seed
	Dee_hash_t h = seed ^ (n_dwords * m);
#else /* seed */
	Dee_hash_t h = 0;
#endif /* !seed */
	size_t len8 = n_dwords >> 3;
	while (len8--) {
		Dee_hash_t k;
		k = (Dee_hash_t)packl(ptr[0]) << ESEL(0, 56);
		k |= (Dee_hash_t)packl(ptr[1]) << ESEL(8, 48);
		k |= (Dee_hash_t)packl(ptr[2]) << ESEL(16, 40);
		k |= (Dee_hash_t)packl(ptr[3]) << ESEL(24, 32);
		k |= (Dee_hash_t)packl(ptr[4]) << ESEL(32, 24);
		k |= (Dee_hash_t)packl(ptr[5]) << ESEL(40, 16);
		k |= (Dee_hash_t)packl(ptr[6]) << ESEL(48, 8);
		k |= (Dee_hash_t)packl(ptr[7]) << ESEL(56, 0);
		ptr += 8;
		k *= m;
		k ^= k >> r;
		k *= m;
		h ^= k;
		h *= m;
	}
	switch (n_dwords & 7) {
	case 7:
		h ^= (Dee_hash_t)packl(ptr[6]) << 48;
		ATTR_FALLTHROUGH
	case 6:
		h ^= (Dee_hash_t)packl(ptr[5]) << 40;
		ATTR_FALLTHROUGH
	case 5:
		h ^= (Dee_hash_t)packl(ptr[4]) << 32;
		ATTR_FALLTHROUGH
	case 4:
		h ^= (Dee_hash_t)packl(ptr[3]) << 24;
		ATTR_FALLTHROUGH
	case 3:
		h ^= (Dee_hash_t)packl(ptr[2]) << 16;
		ATTR_FALLTHROUGH
	case 2:
		h ^= (Dee_hash_t)packl(ptr[1]) << 8;
		ATTR_FALLTHROUGH
	case 1:
		h ^= (Dee_hash_t)packl(ptr[0]);
		h *= m;
		break;
	default: break;
	}
	h ^= h >> r;
	h *= m;
	h ^= h >> r;
	return h;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_HashCasePtr(void const *__restrict ptr, size_t n_bytes) {
#ifdef seed
	Dee_hash_t h = seed ^ (n_bytes * m);
#else /* seed */
	Dee_hash_t h = 0;
#endif /* !seed */
	size_t len8 = n_bytes >> 3;
	while (len8--) {
		union {
			char ch[8];
			Dee_hash_t x;
		} k;
		k.x = UNALIGNED_GETLE64((Dee_hash_t *)ptr);
		ptr = (void const *)((byte_t *)ptr + sizeof(Dee_hash_t));
		k.ch[0] = (char)DeeUni_ToLower(k.ch[0]);
		k.ch[1] = (char)DeeUni_ToLower(k.ch[1]);
		k.ch[2] = (char)DeeUni_ToLower(k.ch[2]);
		k.ch[3] = (char)DeeUni_ToLower(k.ch[3]);
		k.ch[4] = (char)DeeUni_ToLower(k.ch[4]);
		k.ch[5] = (char)DeeUni_ToLower(k.ch[5]);
		k.ch[6] = (char)DeeUni_ToLower(k.ch[6]);
		k.ch[7] = (char)DeeUni_ToLower(k.ch[7]);
		k.x *= m;
		k.x ^= k.x >> r;
		k.x *= m;
		h ^= k.x;
		h *= m;
	}
	switch (n_bytes & 7) {
	case 7:
		h ^= (Dee_hash_t)DeeUni_ToLower(((uint8_t *)ptr)[6]) << 48;
		ATTR_FALLTHROUGH
	case 6:
		h ^= (Dee_hash_t)DeeUni_ToLower(((uint8_t *)ptr)[5]) << 40;
		ATTR_FALLTHROUGH
	case 5:
		h ^= (Dee_hash_t)DeeUni_ToLower(((uint8_t *)ptr)[4]) << 32;
		ATTR_FALLTHROUGH
	case 4:
		h ^= (Dee_hash_t)DeeUni_ToLower(((uint8_t *)ptr)[3]) << 24;
		ATTR_FALLTHROUGH
	case 3:
		h ^= (Dee_hash_t)DeeUni_ToLower(((uint8_t *)ptr)[2]) << 16;
		ATTR_FALLTHROUGH
	case 2:
		h ^= (Dee_hash_t)DeeUni_ToLower(((uint8_t *)ptr)[1]) << 8;
		ATTR_FALLTHROUGH
	case 1:
		h ^= (Dee_hash_t)DeeUni_ToLower(((uint8_t *)ptr)[0]);
		h *= m;
		break;
	default: break;
	}
	h ^= h >> r;
	h *= m;
	h ^= h >> r;
	return h;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_HashCase2Byte(uint16_t const *__restrict ptr, size_t n_words) {
#ifdef seed
	Dee_hash_t h = seed ^ (n_words * m);
#else /* seed */
	Dee_hash_t h = 0;
#endif /* !seed */
	size_t len8 = n_words >> 3;
	uint32_t ch;
	while (len8--) {
		Dee_hash_t k;
		ch = DeeUni_ToLower(ptr[0]);
		k  = (Dee_hash_t)packl(ch) << ESEL(0, 56);
		ch = DeeUni_ToLower(ptr[1]);
		k |= (Dee_hash_t)packl(ch) << ESEL(8, 48);
		ch = DeeUni_ToLower(ptr[2]);
		k |= (Dee_hash_t)packl(ch) << ESEL(16, 40);
		ch = DeeUni_ToLower(ptr[3]);
		k |= (Dee_hash_t)packl(ch) << ESEL(24, 32);
		ch = DeeUni_ToLower(ptr[4]);
		k |= (Dee_hash_t)packl(ch) << ESEL(32, 24);
		ch = DeeUni_ToLower(ptr[5]);
		k |= (Dee_hash_t)packl(ch) << ESEL(40, 16);
		ch = DeeUni_ToLower(ptr[6]);
		k |= (Dee_hash_t)packl(ch) << ESEL(48, 8);
		ch = DeeUni_ToLower(ptr[7]);
		k |= (Dee_hash_t)packl(ch) << ESEL(56, 0);
		ptr += 8;
		k *= m;
		k ^= k >> r;
		k *= m;
		h ^= k;
		h *= m;
	}
	switch (n_words & 7) {
	case 7:
		ch = DeeUni_ToLower(ptr[6]);
		h ^= (Dee_hash_t)packl(ch) << 48;
		ATTR_FALLTHROUGH
	case 6:
		ch = DeeUni_ToLower(ptr[5]);
		h ^= (Dee_hash_t)packl(ch) << 40;
		ATTR_FALLTHROUGH
	case 5:
		ch = DeeUni_ToLower(ptr[4]);
		h ^= (Dee_hash_t)packl(ch) << 32;
		ATTR_FALLTHROUGH
	case 4:
		ch = DeeUni_ToLower(ptr[3]);
		h ^= (Dee_hash_t)packl(ch) << 24;
		ATTR_FALLTHROUGH
	case 3:
		ch = DeeUni_ToLower(ptr[2]);
		h ^= (Dee_hash_t)packl(ch) << 16;
		ATTR_FALLTHROUGH
	case 2:
		ch = DeeUni_ToLower(ptr[1]);
		h ^= (Dee_hash_t)packl(ch) << 8;
		ATTR_FALLTHROUGH
	case 1:
		ch = DeeUni_ToLower(ptr[0]);
		h ^= (Dee_hash_t)packl(ch);
		h *= m;
		break;
	default: break;
	}
	h ^= h >> r;
	h *= m;
	h ^= h >> r;
	return h;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_HashCase4Byte(uint32_t const *__restrict ptr, size_t n_dwords) {
#ifdef seed
	Dee_hash_t h   = seed ^ (n_dwords * m);
#else /* seed */
	Dee_hash_t h = 0;
#endif /* !seed */
	size_t len8 = n_dwords >> 3;
	uint32_t ch;
	while (len8--) {
		Dee_hash_t k;
		ch = DeeUni_ToLower(ptr[0]);
		k  = (Dee_hash_t)packl(ch) << ESEL(0, 56);
		ch = DeeUni_ToLower(ptr[1]);
		k |= (Dee_hash_t)packl(ch) << ESEL(8, 48);
		ch = DeeUni_ToLower(ptr[2]);
		k |= (Dee_hash_t)packl(ch) << ESEL(16, 40);
		ch = DeeUni_ToLower(ptr[3]);
		k |= (Dee_hash_t)packl(ch) << ESEL(24, 32);
		ch = DeeUni_ToLower(ptr[4]);
		k |= (Dee_hash_t)packl(ch) << ESEL(32, 24);
		ch = DeeUni_ToLower(ptr[5]);
		k |= (Dee_hash_t)packl(ch) << ESEL(40, 16);
		ch = DeeUni_ToLower(ptr[6]);
		k |= (Dee_hash_t)packl(ch) << ESEL(48, 8);
		ch = DeeUni_ToLower(ptr[7]);
		k |= (Dee_hash_t)packl(ch) << ESEL(56, 0);
		ptr += 8;
		k *= m;
		k ^= k >> r;
		k *= m;
		h ^= k;
		h *= m;
	}
	switch (n_dwords & 7) {
	case 7:
		ch = DeeUni_ToLower(ptr[6]);
		h ^= (Dee_hash_t)packl(ch) << 48;
		ATTR_FALLTHROUGH
	case 6:
		ch = DeeUni_ToLower(ptr[5]);
		h ^= (Dee_hash_t)packl(ch) << 40;
		ATTR_FALLTHROUGH
	case 5:
		ch = DeeUni_ToLower(ptr[4]);
		h ^= (Dee_hash_t)packl(ch) << 32;
		ATTR_FALLTHROUGH
	case 4:
		ch = DeeUni_ToLower(ptr[3]);
		h ^= (Dee_hash_t)packl(ch) << 24;
		ATTR_FALLTHROUGH
	case 3:
		ch = DeeUni_ToLower(ptr[2]);
		h ^= (Dee_hash_t)packl(ch) << 16;
		ATTR_FALLTHROUGH
	case 2:
		ch = DeeUni_ToLower(ptr[1]);
		h ^= (Dee_hash_t)packl(ch) << 8;
		ATTR_FALLTHROUGH
	case 1:
		ch = DeeUni_ToLower(ptr[0]);
		h ^= (Dee_hash_t)packl(ch);
		h *= m;
		break;
	default: break;
	}
	h ^= h >> r;
	h *= m;
	h ^= h >> r;
	return h;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_HashUtf8(char const *__restrict ptr, size_t n_bytes) {
#ifdef seed
	Dee_hash_t h = seed ^ (n_bytes * m); /* XXX: num_characters */
#else /* seed */
	Dee_hash_t h = 0;
#endif /* !seed */
	char const *end = ptr + n_bytes;
	uint32_t block[8];
	for (;;) {
		Dee_hash_t k;
		if unlikely(ptr >= end)
			goto done;
		block[0] = Dee_unicode_readutf8_n(&ptr, end);
		if unlikely(ptr >= end)
			goto do_tail_1;
		block[1] = Dee_unicode_readutf8_n(&ptr, end);
		if unlikely(ptr >= end)
			goto do_tail_2;
		block[2] = Dee_unicode_readutf8_n(&ptr, end);
		if unlikely(ptr >= end)
			goto do_tail_3;
		block[3] = Dee_unicode_readutf8_n(&ptr, end);
		if unlikely(ptr >= end)
			goto do_tail_4;
		block[4] = Dee_unicode_readutf8_n(&ptr, end);
		if unlikely(ptr >= end)
			goto do_tail_5;
		block[5] = Dee_unicode_readutf8_n(&ptr, end);
		if unlikely(ptr >= end)
			goto do_tail_6;
		block[6] = Dee_unicode_readutf8_n(&ptr, end);
		if unlikely(ptr >= end)
			goto do_tail_7;
		block[7] = Dee_unicode_readutf8_n(&ptr, end);
		k        = (Dee_hash_t)packl(block[0]) << ESEL(0, 56);
		k |= (Dee_hash_t)packl(block[1]) << ESEL(8, 48);
		k |= (Dee_hash_t)packl(block[2]) << ESEL(16, 40);
		k |= (Dee_hash_t)packl(block[3]) << ESEL(24, 32);
		k |= (Dee_hash_t)packl(block[4]) << ESEL(32, 24);
		k |= (Dee_hash_t)packl(block[5]) << ESEL(40, 16);
		k |= (Dee_hash_t)packl(block[6]) << ESEL(48, 8);
		k |= (Dee_hash_t)packl(block[7]) << ESEL(56, 0);
		k *= m;
		k ^= k >> r;
		k *= m;
		h ^= k;
		h *= m;
	}
	goto done;
do_tail_7:
	h ^= (Dee_hash_t)packl(block[6]) << 48;
do_tail_6:
	h ^= (Dee_hash_t)packl(block[5]) << 40;
do_tail_5:
	h ^= (Dee_hash_t)packl(block[4]) << 32;
do_tail_4:
	h ^= (Dee_hash_t)packl(block[3]) << 24;
do_tail_3:
	h ^= (Dee_hash_t)packl(block[2]) << 16;
do_tail_2:
	h ^= (Dee_hash_t)packl(block[1]) << 8;
do_tail_1:
	h ^= (Dee_hash_t)packl(block[0]);
	h *= m;
done:
	h ^= h >> r;
	h *= m;
	h ^= h >> r;
	return h;
}

PUBLIC ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t DCALL
Dee_HashCaseUtf8(char const *__restrict ptr, size_t n_bytes) {
#ifdef seed
	Dee_hash_t h = seed ^ (n_bytes * m); /* XXX: num_characters */
#else /* seed */
	Dee_hash_t h = 0;
#endif /* !seed */
	char const *end = ptr + n_bytes;
	uint32_t block[8];
	for (;;) {
		Dee_hash_t k;
		if unlikely(ptr >= end)
			goto done;
		block[0] = Dee_unicode_readutf8_n(&ptr, end);
		block[0] = DeeUni_ToLower(block[0]);
		if unlikely(ptr >= end)
			goto do_tail_1;
		block[1] = Dee_unicode_readutf8_n(&ptr, end);
		block[1] = DeeUni_ToLower(block[1]);
		if unlikely(ptr >= end)
			goto do_tail_2;
		block[2] = Dee_unicode_readutf8_n(&ptr, end);
		block[2] = DeeUni_ToLower(block[2]);
		if unlikely(ptr >= end)
			goto do_tail_3;
		block[3] = Dee_unicode_readutf8_n(&ptr, end);
		block[3] = DeeUni_ToLower(block[3]);
		if unlikely(ptr >= end)
			goto do_tail_4;
		block[4] = Dee_unicode_readutf8_n(&ptr, end);
		block[4] = DeeUni_ToLower(block[4]);
		if unlikely(ptr >= end)
			goto do_tail_5;
		block[5] = Dee_unicode_readutf8_n(&ptr, end);
		block[5] = DeeUni_ToLower(block[5]);
		if unlikely(ptr >= end)
			goto do_tail_6;
		block[6] = Dee_unicode_readutf8_n(&ptr, end);
		block[6] = DeeUni_ToLower(block[6]);
		if unlikely(ptr >= end)
			goto do_tail_7;
		block[7] = Dee_unicode_readutf8_n(&ptr, end);
		block[7] = DeeUni_ToLower(block[7]);
		k = (Dee_hash_t)packl(block[0]) << ESEL(0, 56);
		k |= (Dee_hash_t)packl(block[1]) << ESEL(8, 48);
		k |= (Dee_hash_t)packl(block[2]) << ESEL(16, 40);
		k |= (Dee_hash_t)packl(block[3]) << ESEL(24, 32);
		k |= (Dee_hash_t)packl(block[4]) << ESEL(32, 24);
		k |= (Dee_hash_t)packl(block[5]) << ESEL(40, 16);
		k |= (Dee_hash_t)packl(block[6]) << ESEL(48, 8);
		k |= (Dee_hash_t)packl(block[7]) << ESEL(56, 0);
		k *= m;
		k ^= k >> r;
		k *= m;
		h ^= k;
		h *= m;
	}
	goto done;
do_tail_7:
	h ^= (Dee_hash_t)packl(block[6]) << 48;
do_tail_6:
	h ^= (Dee_hash_t)packl(block[5]) << 40;
do_tail_5:
	h ^= (Dee_hash_t)packl(block[4]) << 32;
do_tail_4:
	h ^= (Dee_hash_t)packl(block[3]) << 24;
do_tail_3:
	h ^= (Dee_hash_t)packl(block[2]) << 16;
do_tail_2:
	h ^= (Dee_hash_t)packl(block[1]) << 8;
do_tail_1:
	h ^= (Dee_hash_t)packl(block[0]);
	h *= m;
done:
	h ^= h >> r;
	h *= m;
	h ^= h >> r;
	return h;
}

#undef seed
#undef r
#undef m

#else /* ... */
#error "Unable to determine desired hash algorithm"
#endif /* !... */

PUBLIC ATTR_PURE WUNUSED ATTR_IN(1) Dee_hash_t DCALL
Dee_HashStr(char const *__restrict str) {
	return Dee_HashPtr(str, strlen(str));
}

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_MISC_C */
