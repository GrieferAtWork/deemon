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

#include <deemon/file.h>        /* DeeFileObject, DeeFileTypeObject, Dee_FILE_OBJECT_HEAD */
#include <deemon/types.h>       /* DeeTypeObject, Dee_OBJECT_HEAD */
#include <deemon/util/nrlock.h> /* Dee_nrshared_lock_* */

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


/************************************************************************/
/* DECODER                                                              */
/************************************************************************/

typedef struct {
	/* Decoder: convert <some codec> --> UTF-8 */
	Dee_FILE_OBJECT_HEAD
//	DREF DeeObject      *ivd_out;     /* [const][1..1] Output file for output of `ivd_decoder' (== ivd_decoder.icd_output.ii_arg) */
	Dee_nrshared_lock_t  ivd_lock;    /* Lock for `ivd_decoder' */
	struct iconv_printer ivd_input;   /* [lock(ivd_lock)] Input printer */
	struct iconv_decode  ivd_decoder; /* [lock(ivd_lock)] Underlying decoder */
} IconvDecoder;

INTDEF DeeFileTypeObject IconvDecoder_Type;
#define IconvDecoder_GetOut(self) ((DeeFileObject *)(self)->ivd_decoder.icd_output.ii_arg)



/************************************************************************/
/* ENCODER                                                              */
/************************************************************************/

typedef struct {
	/* Encoder: convert UTF-8 --> <some codec> */
	Dee_FILE_OBJECT_HEAD
//	DREF DeeObject      *ive_out;     /* [const][1..1] Output file for output of `ive_encoder' (== ive_encoder.ice_output.ii_arg) */
	Dee_nrshared_lock_t  ive_lock;    /* Lock for `ive_encoder' */
	struct iconv_printer ive_input;   /* [lock(ive_lock)] Input printer */
	struct iconv_encode  ive_encoder; /* [lock(ive_lock)] Underlying encoder */
} IconvEncoder;

INTDEF DeeFileTypeObject IconvEncoder_Type;
#define IconvEncoder_GetOut(self) ((DeeFileObject *)(self)->ive_encoder.ice_output.ii_arg)



/************************************************************************/
/* TRANSCODER                                                           */
/************************************************************************/

typedef struct {
	/* Transcoder: convert <some codec> --> <some codec> */
	Dee_FILE_OBJECT_HEAD
//	DREF DeeObject      *ivt_out;     /* [const][1..1] Output file for output of `ivt_encoder' (== ivt_encoder.ice_output.ii_arg) */
	Dee_nrshared_lock_t  ivt_lock;    /* Lock for `ivt_encoder' */
	struct iconv_printer ivt_input;   /* [lock(ivt_lock)] Input printer */
	struct iconv_encode  ivt_encoder; /* [lock(ivt_lock)] Underlying encoder */
	struct iconv_decode  ivt_decoder; /* [lock(ivt_lock)] Underlying decoder */
} IconvTranscoder;

INTDEF DeeFileTypeObject IconvTranscoder_Type;
#define IconvTranscoder_GetOut(self) ((DeeFileObject *)(self)->ivt_encoder.ice_output.ii_arg)

INTDEF ATTR_COLD int DCALL err_unicode_decode_error(iconv_codec_t codec, size_t offset);
INTDEF ATTR_COLD int DCALL err_unicode_encode_error(iconv_codec_t codec, size_t offset);
INTDEF ATTR_COLD int DCALL err_unicode_reencode_error(iconv_codec_t codec);
INTDEF ATTR_COLD int DCALL err_unknown_codec(iconv_codec_t codec);

/* @return: ICONV_CODEC_UNKNOWN: An error was thrown */
INTDEF WUNUSED NONNULL((1, 3)) iconv_codec_t DCALL
deemon_iconv_parse_codec_name_and_error_mode(DeeObject *codec, DeeObject *errors,
                                             uintptr_half_t *__restrict p_flags);

DECL_END

#endif /* !GUARD_DEX_ICONV_LIBICONV_H */
