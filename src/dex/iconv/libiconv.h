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
#ifndef GUARD_DEX_ICONV_LIBICONV_H
#define GUARD_DEX_ICONV_LIBICONV_H 1

#include <deemon/api.h>

#include <deemon/file.h>
#include <deemon/util/nrlock.h>

#include "iconv.h"

DECL_BEGIN

typedef struct {
	Dee_OBJECT_HEAD
	char const *snai_iter; /* [1..1][lock(ATOMIC)] Pointer to next string, or
	                        * pointer to a \0-character if enumeration finished */
} StrNulArrayIterator;

typedef struct {
	Dee_OBJECT_HEAD
	char const *sna_base; /* [1..1][const] Pointer to start of \0-separated and \0\0-terminated array of strings */
} StrNulArray;

INTDEF DeeTypeObject StrNulArrayIterator_Type;
INTDEF DeeTypeObject StrNulArray_Type;

typedef struct {
	/* Decoder: convert <some codec> --> UTF-8 */
	Dee_FILE_OBJECT_HEAD
//	DREF DeeObject     *ivd_out;     /* [const][1..1] Output file for output of `ivd_decoder' (== ivd_decoder.icd_output.ii_arg) */
	Dee_nrshared_lock_t ivd_lock;    /* Lock for `ivd_decoder' */
	struct iconv_decode ivd_decoder; /* [lock(ivd_lock)] Underlying decoder */
} IconvDecoder;

typedef struct {
	/* Encoder: convert UTF-8 --> <some codec> */
	Dee_FILE_OBJECT_HEAD
//	DREF DeeObject     *ive_out;     /* [const][1..1] Output file for output of `ive_encoder' (== ive_encoder.ice_output.ii_arg) */
	Dee_nrshared_lock_t ive_lock;    /* Lock for `ive_encoder' */
	struct iconv_encode ive_encoder; /* [lock(ive_lock)] Underlying encoder */
} IconvEncoder;

typedef struct {
	/* Transcoder: convert <some codec> --> <some codec> */
	Dee_FILE_OBJECT_HEAD
//	DREF DeeObject     *ivt_out;     /* [const][1..1] Output file for output of `ivt_encoder' (== ivt_encoder.ice_output.ii_arg) */
	Dee_nrshared_lock_t ivt_lock;    /* Lock for `ivt_encoder' */
	struct iconv_encode ivt_decoder; /* [lock(ivt_lock)] Underlying decoder */
	struct iconv_encode ivt_encoder; /* [lock(ivt_lock)] Underlying encoder */
} IconvTranscoder;

INTDEF DeeFileTypeObject IconvDecoder_Type;
INTDEF DeeFileTypeObject IconvEncoder_Type;
INTDEF DeeFileTypeObject IconvTranscoder_Type;

DECL_END

#endif /* !GUARD_DEX_ICONV_LIBICONV_H */
