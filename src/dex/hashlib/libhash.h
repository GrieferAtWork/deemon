/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_HASHLIB_LIBHASH_H
#define GUARD_DEX_HASHLIB_LIBHASH_H 1

#include <deemon/api.h>
#include <deemon/dex.h>
#include <deemon/object.h>
#include <stdint.h>
#include <stdbool.h>

DECL_BEGIN

struct dhashalgo;
struct dhashalgo8;
struct dhashalgo16;
struct dhashalgo32;
struct dhashalgo64;
struct dhashalgon;

/* Prototypes for 1, 2, 4 and 8-byte hash functions. */
typedef WUNUSED_T NONNULL_T((1, 3)) uint8_t (DCALL *dhashfunc8_t)(struct dhashalgo8 const *__restrict self, uint8_t  start, void const *__restrict data, size_t datasize);
typedef WUNUSED_T NONNULL_T((1, 3)) uint16_t (DCALL *dhashfunc16_t)(struct dhashalgo16 const *__restrict self, uint16_t start, void const *__restrict data, size_t datasize);
typedef WUNUSED_T NONNULL_T((1, 3)) uint32_t (DCALL *dhashfunc32_t)(struct dhashalgo32 const *__restrict self, uint32_t start, void const *__restrict data, size_t datasize);
typedef WUNUSED_T NONNULL_T((1, 3)) uint64_t (DCALL *dhashfunc64_t)(struct dhashalgo64 const *__restrict self, uint64_t start, void const *__restrict data, size_t datasize);

/* Hash function prototype for extended-width hash functions. (with hash values of up to 256 bytes) */
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *
(DCALL *dhashfuncn_t)(struct dhashalgon const *__restrict self,
                      DeeObject *__restrict start,
                      void const *__restrict data, size_t datasize);

struct dhashalgo {
	char const        *ha_name;  /* [1..1] Name of the algorithm. */
	char const *const *ha_alias; /* [1..1][0..1] NULL-terminated list of alias names. */
	uint16_t           ha_width; /* Crc bit-width (<= ha_size * 8). */
	uint8_t            ha_size;  /* Size of a hash value in bytes (1|2|4|8|n). */
#define HASHALGO_FNORMAL 0x0000  /* Normal flags. */
	uint8_t            ha_flags; /* Additional flags (set of `HASHALGO_F*') */
	union {
		/* Internal hash algorithms (these don't perform input/output modulation) */
		dhashfuncn_t   ha_hashn;  /* [1..1] */
		dhashfunc8_t   ha_hash8;  /* [1..1] */
		dhashfunc16_t  ha_hash16; /* [1..1] */
		dhashfunc32_t  ha_hash32; /* [1..1] */
		dhashfunc64_t  ha_hash64; /* [1..1] */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define ha_hashn  _dee_aunion.ha_hashn
#define ha_hash8  _dee_aunion.ha_hash8
#define ha_hash16 _dee_aunion.ha_hash16
#define ha_hash32 _dee_aunion.ha_hash32
#define ha_hash64 _dee_aunion.ha_hash64
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

#define DHASHALGO_MEMBERS(T)                                                                         \
	struct dhashalgo ha_base;       /* The underlying algorithm descriptor. */                       \
	T                ha_start;      /* Initial hash value */                                         \
	T                ha_outmod;     /* Xor the given input, and final output hash value with this */ \
	T                ha_table[256]; /* Hash translation table. */

struct dhashalgo8 {
	DHASHALGO_MEMBERS(uint8_t)
};
struct dhashalgo16 {
	DHASHALGO_MEMBERS(uint16_t)
};
struct dhashalgo32 {
	DHASHALGO_MEMBERS(uint32_t)
};
struct dhashalgo64 {
	DHASHALGO_MEMBERS(uint64_t)
};
struct dhashalgon {
	struct dhashalgo ha_base;  /* The underlying algorithm descriptor. */
	DREF DeeObject  *ha_start; /* Initial hash value */
};
#undef DHASHALGO_MEMBERS


/* Execute the given hash algorithm to hash `data...+=datasize'
 * When given, `start' is used as the initial hash value (which
 * may be the hash result of a previous call), but when set to
 * NULL, the algorythm's default start-value is used instead. */
INTDEF WUNUSED NONNULL((1, 3)) DREF /*Int*/ DeeObject *DCALL
dhashalgo_exec(struct dhashalgo const *__restrict self,
               /*Int*/ DeeObject *start,
               void const *__restrict data,
               size_t datasize);

/* Try to find the hash algorithm associated with `name', returning
 * NULL (but not throwing an error) if no such algorithm exists. */
INTDEF WUNUSED NONNULL((1)) struct dhashalgo const *DCALL
dhashalgo_tryfind(char const *__restrict name);
/* Same as `dhashalgo_tryfind()', but throw an error if the algorithm wasn't found. */
INTDEF WUNUSED NONNULL((1)) struct dhashalgo const *DCALL
dhashalgo_find(char const *__restrict name);



DECL_END

#endif /* !GUARD_DEX_HASHLIB_LIBHASH_H */
