/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_FILES_LIBFILES_H
#define GUARD_DEX_FILES_LIBFILES_H 1

#include <deemon/api.h>
#include <deemon/dex.h>
#include <deemon/file.h>
#include <deemon/object.h>
#include <deemon/string.h>

DECL_BEGIN

typedef struct {
	/* Joined files are created when `operator |' is used on
	 * a type derived from `File', in which case the 2 files
	 * are used to create a so-called joined file type. */
	FILE_OBJECT_HEAD
	size_t                                    j_count;  /* [const] Amount of files that are being joined together. */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, j_files); /* [j_count][const] Vector of joined files.
	                                                     * NOTE: This vector is accessible as an
	                                                     *       abstract sequence type named `files' */
} Joined;

INTDEF DeeFileTypeObject Joined_Type; /* TODO: Not implemented. */



typedef struct {
	/* General-purpose file data de/en-coder
	 *
	 * This file type is used as an intermitten wrapper type
	 * for `File.open' when the file is being opened in text-mode.
	 *
	 * It is not only used to automatically detect the encoding of a
	 * file, but also be able to encode/decode its contents to/from UTF-8. */
	FILE_OBJECT_HEAD
	DREF DeeObject *d_file;         /* [1..1][const] The underlying file stream. */
#define DECODER_ENCODING_UNSET    0 /* Automatically determine the encoding upon first access. */
#define DECODER_ENCODING_UTF8     1 /* UTF-8 or ASCII */
#define DECODER_ENCODING_UTF16_LE 2 /* UTF-16 (little-endian) */
#define DECODER_ENCODING_UTF16_BE 3 /* UTF-16 (big-endian) */
#define DECODER_ENCODING_UTF32_LE 4 /* UTF-32 (little-endian) */
#define DECODER_ENCODING_UTF32_BE 5 /* UTF-32 (big-endian) */
	uint16_t        d_encoding;     /* The encoding of `d_file' (One of `DECODER_ENCODING_*') */
} Decoder;

INTDEF DeeFileTypeObject Decoder_Type; /* TODO: Not implemented. */


DECL_END

#endif /* !GUARD_DEX_FILES_LIBFILES_H */
