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
#include <deemon/types.h>       /* DeeObject, DeeTypeObject, Dee_OBJECT_HEAD */
#include <deemon/util/nrlock.h> /* Dee_nrshared_lock_t */

#include "iconv.h"

#include <stddef.h> /* size_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

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
/* ENCODER                                                              */
/************************************************************************/

typedef struct {
	/* Encoder: convert UTF-8 --> <some codec> */
	Dee_FILE_OBJECT_HEAD
//	DREF DeeObject      *ive_out;      /* [const][1..1] Output file for output of `ive_encoder' (== ive_encoder.ice_output.ii_arg) */
	Dee_nrshared_lock_t  ive_lock;     /* Lock for `ive_encoder' */
	bool                 ive_closeout; /* [const] Propagate "operator sync()" and "operator close()" to `ive_out' */
	struct iconv_printer ive_input;    /* [lock(ive_lock)] Input printer */
	struct iconv_encode  ive_encoder;  /* [lock(ive_lock)] Underlying encoder */
} IconvEncoder;

INTDEF DeeFileTypeObject IconvEncoder_Type;
#define IconvEncoder_GetOut(self) ((DeeFileObject *)(self)->ive_encoder.ice_output.ii_arg)



/************************************************************************/
/* DECODE-WRITER                                                        */
/************************************************************************/

typedef struct {
	/* Decode-writer: convert <some codec> --> UTF-8 */
	Dee_FILE_OBJECT_HEAD
//	DREF DeeObject      *ivdw_out;      /* [const][1..1] Output file for output of `ivdw_decoder' (== ivdw_decoder.icd_output.ii_arg) */
	Dee_nrshared_lock_t  ivdw_lock;     /* Lock for `ivdw_decoder' */
	bool                 ivdw_closeout; /* [const] Propagate "operator sync()" and "operator close()" to `ive_out' */
	struct iconv_printer ivdw_input;    /* [lock(ivdw_lock)] Input printer */
	struct iconv_decode  ivdw_decoder;  /* [lock(ivdw_lock)] Underlying decoder */
} IconvDecodeWriter;

INTDEF DeeFileTypeObject IconvDecodeWriter_Type;
#define IconvDecodeWriter_GetOut(self) ((DeeFileObject *)(self)->ivdw_decoder.icd_output.ii_arg)



/************************************************************************/
/* TRANSCODE-WRITER                                                     */
/************************************************************************/

typedef struct {
	/* Transcoder: convert <some codec> --> <some codec> */
	Dee_FILE_OBJECT_HEAD
//	DREF DeeObject      *ivtw_out;      /* [const][1..1] Output file for output of `ivtw_encoder' (== ivtw_encoder.ice_output.ii_arg) */
	Dee_nrshared_lock_t  ivtw_lock;     /* Lock for `ivtw_encoder' */
	bool                 ivtw_closeout; /* [const] Propagate "operator sync()" and "operator close()" to `ive_out' */
	struct iconv_printer ivtw_input;    /* [lock(ivtw_lock)] Input printer */
	struct iconv_encode  ivtw_encoder;  /* [lock(ivtw_lock)] Underlying encoder */
	struct iconv_decode  ivtw_decoder;  /* [lock(ivtw_lock)] Underlying decoder */
} IconvTranscodeWriter;

INTDEF DeeFileTypeObject IconvTranscodeWriter_Type;
#define IconvTranscodeWriter_GetOut(self) ((DeeFileObject *)(self)->ivtw_encoder.ice_output.ii_arg)




/************************************************************************/
/* DECODER                                                              */
/************************************************************************/

typedef struct {
	/* Decode-reader: convert <some codec> --> UTF-8 */
	Dee_FILE_OBJECT_HEAD
	Dee_nrshared_lock_t  ivd_lock;    /* Lock for `ivd_decoder' */
	bool                 ivd_closein; /* [const] Propagate "operator sync()" and "operator close()" to `ivd_in' */
	DREF DeeObject      *ivd_in;      /* [const][1..1] Input file containing encoded data */
	byte_t              *ivd_bufbase; /* [0..ivd_pendsize][owned][lock(ivd_lock)] Buffer for decoded, but not-yet-read data */
	size_t               ivd_bufsize; /* [lock(ivd_lock)] Allocated size of `ivd_bufbase' */
	byte_t              *ivd_pndbase; /* [0..ivd_bufbase][lock(ivd_lock)] Pointer to first unread byte in `ivd_bufbase' */
	size_t               ivd_pndsize; /* [lock(ivd_lock)] # of decoded, but not-yet-read bytes starting at `ivd_pndbase' */
	size_t               ivd_chnksiz; /* [lock(ivd_lock)] Max. block size when reading from `ivd_in' */
	struct iconv_printer ivd_input;   /* [lock(ivd_lock)] Input printer */
	struct iconv_decode  ivd_decoder; /* [lock(ivd_lock)] Underlying decoder */
} IconvDecoder;

INTDEF DeeFileTypeObject IconvDecoder_Type;






INTDEF ATTR_COLD int DCALL err_unicode_decode_error(iconv_codec_t codec, Dee_pos_t offset);
INTDEF ATTR_COLD int DCALL err_unicode_encode_error(iconv_codec_t codec, Dee_pos_t offset);
INTDEF ATTR_COLD int DCALL err_unicode_reencode_error(iconv_codec_t codec);
INTDEF ATTR_COLD int DCALL err_unknown_codec(iconv_codec_t codec);


/* @return: ICONV_CODEC_UNKNOWN: An error was thrown */
INTDEF WUNUSED NONNULL((1, 3)) iconv_codec_t DCALL
deemon_iconv_parse_codec_name_and_error_mode(DeeObject *codec, DeeObject *errors,
                                             uintptr_half_t *__restrict p_flags);

DECL_END

#endif /* !GUARD_DEX_ICONV_LIBICONV_H */
