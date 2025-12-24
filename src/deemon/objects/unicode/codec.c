/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_CODEC_C
#define GUARD_DEEMON_OBJECTS_UNICODE_CODEC_C 1

#include <deemon/api.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/util/lock.h>

#include <hybrid/byteorder.h>
#include <hybrid/unaligned.h>
#include <hybrid/typecore.h>
#include <hybrid/wordbits.h>
/**/

#include "../../runtime/strings.h"
#include "codec.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint8_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCodec_NormalizeName(DeeObject *__restrict name) {
	char const *iter, *end, *str;
	size_t length;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	length = DeeString_SIZE(name);
	str    = DeeString_STR(name);
	iter   = str;
	end    = iter + length;
	for (; iter < end; ++iter) {
		/* TODO: Use case folding to normalize codec names! */
		/* TODO: When `DeeString_STR_ISUTF8()' is true, must `DeeString_SetUtf8()' the result! */
		if (*iter == '_' || DeeUni_IsUpper(*iter)) {
			char *dst;
			result = DeeString_NewBuffer(length);
			if unlikely(!result)
				goto err;
			dst = (char *)mempcpyc(DeeString_GetBuffer(result), str,
			                       (size_t)(iter - str), sizeof(char));
			for (; iter < end; ++iter) {
				if (*iter == '_') {
					*dst++ = '-';
				} else if (DeeUni_IsUpper(*iter)) {
					*dst++ = (uint8_t)DeeUni_ToLower(*iter);
				} else {
					*dst++ = *iter;
				}
			}
			ASSERT(dst == DeeString_GetBuffer(result) + DeeString_SIZE(result));
			if (length >= 4 &&
			    UNALIGNED_GET32(DeeString_GetBuffer(result)) == ENCODE_INT32('i', 's', 'o', '-')) {
				--((DeeStringObject *)result)->s_len;
				memmovedownc(DeeString_GetBuffer(result) + 3,
				             DeeString_GetBuffer(result) + 4,
				             length - 4,
				             sizeof(char));
				DeeString_GetBuffer(result)[length - 2] = '\0';
			} else if (length >= 3 &&
			           UNALIGNED_GET16(DeeString_GetBuffer(result)) == ENCODE_INT16('c', 'p') &&
			           DeeString_GetBuffer(result)[2] == '-') {
				--((DeeStringObject *)result)->s_len;
				memmovedownc(DeeString_GetBuffer(result) + 2,
				             DeeString_GetBuffer(result) + 3,
				             length - 3,
				             sizeof(char));
				DeeString_GetBuffer(result)[length - 2] = '\0';
			}
			return result;
		}
	}
	if (length >= 4 && UNALIGNED_GET32(str) == ENCODE_INT32('i', 's', 'o', '-')) {
		result = DeeString_NewBuffer(length - 1);
		if unlikely(!result)
			goto err;
		UNALIGNED_SET16(DeeString_GetBuffer(result), ENCODE_INT16('i', 's'));
		DeeString_GetBuffer(result)[3] = 'o';
		memcpyc(DeeString_GetBuffer(result) + 3,
		        str + 4,
		        length - 4,
		        sizeof(char));
		return result;
	}
	if (length >= 3 &&
	    UNALIGNED_GET16(str) == ENCODE_INT16('c', 'p') && str[2] == '-') {
		result = DeeString_NewBuffer(length - 1);
		if unlikely(!result)
			goto err;
		UNALIGNED_SET16(DeeString_GetBuffer(result), ENCODE_INT16('c', 'p'));
		memcpyc(DeeString_GetBuffer(result) + 2,
		        str + 3,
		        length - 3,
		        sizeof(char));
		return result;
	}
	return_reference_(name);
err:
	return NULL;
}



PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_unknown_codec(DeeObject *__restrict name) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Unknown codec %r",
	                       name);
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_expected_string_or_bytes(DeeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Expected a string or Bytes object, but got an instance of %k",
	                       self->ob_type);
}


PRIVATE ATTR_COLD int DCALL err_invalid_ascii(uint32_t ch, bool is_decode) {
	return DeeError_Throwf(is_decode ? &DeeError_UnicodeDecodeError
	                                 : &DeeError_UnicodeEncodeError,
	                       "Invalid ASCII character U+%.4I32X", ch);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
convert_ascii(DeeObject *__restrict self, unsigned int error_mode, bool is_decode) {
	DREF DeeObject *result;
	size_t i, j, size;
	if (DeeBytes_Check(self)) {
		byte_t *data;
		if (error_mode == STRING_ERROR_FIGNORE)
			goto return_self;
		data = DeeBytes_DATA(self);
		size = DeeBytes_SIZE(self);
		for (i = 0; i < size; ++i) {
			if (data[i] <= 0x7f)
				continue;
			if (error_mode == STRING_ERROR_FSTRICT) {
				err_invalid_ascii(data[i], is_decode);
				goto err;
			}
			result = DeeBytes_NewBufferUninitialized(size);
			if unlikely(!result)
				goto err;
			memcpy(DeeBytes_DATA(result), data, i);
			for (; i < size; ++i) {
				uint8_t ch = data[i];
				if (ch > 0x7f)
					ch = '?';
				DeeBytes_DATA(result)[i] = (char)ch;
			}
			return result;
		}
		goto return_self;
	}
	if (DeeString_Check(self)) {
		if (error_mode == STRING_ERROR_FIGNORE)
			goto return_self;
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE: {
			uint8_t *dest;
			uint8_t const *data;
			data = DeeString_Get1Byte(self);
			size = WSTR_LENGTH(data);
			for (i = 0; i < size; ++i) {
				if (data[i] <= 0x7f)
					continue;
				if (error_mode == STRING_ERROR_FSTRICT) {
					err_invalid_ascii(data[i], is_decode);
					goto err;
				}
				dest = DeeString_New1ByteBuffer(size);
				if unlikely(!dest)
					goto err;
				memcpy(dest, data, i);
				for (; i < size; ++i) {
					uint8_t ch = data[i];
					if (ch > 0x7f)
						ch = '?';
					dest[i] = (char)ch;
				}
				return DeeString_Pack1ByteBuffer(dest);
			}
			goto return_self;
		}	break;

		CASE_WIDTH_2BYTE: {
			uint16_t const *data;
			uint8_t *dest;
			data = DeeString_Get2Byte(self);
			size = WSTR_LENGTH(data);
			for (i = 0; i < size; ++i) {
				if (data[i] <= 0x7f)
					continue;
				if (error_mode == STRING_ERROR_FSTRICT) {
					err_invalid_ascii(data[i], is_decode);
					goto err;
				}
				dest = DeeString_New1ByteBuffer(size);
				if unlikely(!dest)
					goto err;
				for (j = 0; j < i; ++j)
					dest[j] = (uint8_t)data[j];
				for (; i < size; ++i) {
					uint16_t ch = data[i];
					if (ch > 0x7f)
						ch = '?';
					dest[i] = (char)(uint8_t)ch;
				}
				return DeeString_Pack1ByteBuffer(dest);
			}
			goto return_self;
		}	break;

		CASE_WIDTH_4BYTE: {
			uint32_t const *data;
			data = DeeString_Get4Byte(self);
			size = WSTR_LENGTH(data);
			for (i = 0; i < size; ++i) {
				if (data[i] <= 0x7f)
					continue;
				if (error_mode == STRING_ERROR_FSTRICT) {
					err_invalid_ascii(data[i], is_decode);
					goto err;
				}
				result = DeeString_NewBuffer(size);
				if unlikely(!result)
					goto err;
				for (j = 0; j < i; ++j)
					DeeString_GetBuffer(result)[j] = (uint8_t)data[j];
				for (; i < size; ++i) {
					uint32_t ch = data[i];
					if (ch > 0x7f)
						ch = '?';
					DeeString_GetBuffer(result)[i] = (char)(uint8_t)ch;
				}
				return result;
			}
			goto return_self;
		}	break;
		}
	}
	err_expected_string_or_bytes(self);
err:
	return NULL;
return_self:
	return_reference_(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
convert_latin1(DeeObject *__restrict self, unsigned int error_mode, bool is_decode) {
	DREF DeeObject *result;
	size_t i, j, size;
	if (DeeBytes_Check(self))
		goto return_self;
	if (DeeString_Check(self)) {
		if (error_mode == STRING_ERROR_FIGNORE)
			goto return_self;
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			goto return_self;

		CASE_WIDTH_2BYTE: {
			uint16_t const *data;
			data = DeeString_Get2Byte(self);
			size = WSTR_LENGTH(data);
			for (i = 0; i < size; ++i) {
				if (data[i] <= 0xff)
					continue;
				if (error_mode == STRING_ERROR_FSTRICT) {
					err_invalid_ascii(data[i], is_decode);
					goto err;
				}
				result = DeeString_NewBuffer(size);
				if unlikely(!result)
					goto err;
				for (j = 0; j < i; ++j)
					DeeString_GetBuffer(result)[j] = (uint8_t)data[j];
				for (; i < size; ++i) {
					uint16_t ch = data[i];
					if (ch > 0xff)
						ch = '?';
					DeeString_GetBuffer(result)[i] = (char)(uint8_t)ch;
				}
				return result;
			}
			goto return_self;
		}	break;

		CASE_WIDTH_4BYTE: {
			uint32_t const *data;
			data = DeeString_Get4Byte(self);
			size = WSTR_LENGTH(data);
			for (i = 0; i < size; ++i) {
				if (data[i] <= 0xff)
					continue;
				if (error_mode == STRING_ERROR_FSTRICT) {
					err_invalid_ascii(data[i], is_decode);
					goto err;
				}
				result = DeeString_NewBuffer(size);
				if unlikely(!result)
					goto err;
				for (j = 0; j < i; ++j)
					DeeString_GetBuffer(result)[j] = (uint8_t)data[j];
				for (; i < size; ++i) {
					uint32_t ch = data[i];
					if (ch > 0xff)
						ch = '?';
					DeeString_GetBuffer(result)[i] = (char)(uint8_t)ch;
				}
				return result;
			}
			goto return_self;
		}	break;
		}
	}
	err_expected_string_or_bytes(self);
err:
	return NULL;
return_self:
	return_reference_(self);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
decode_c_escape(DeeObject *__restrict self, unsigned int error_mode) {
	char const *text;
	size_t size;
	if (DeeBytes_Check(self)) {
		text = (char *)DeeBytes_DATA(self);
		size = DeeBytes_SIZE(self);
	} else if (DeeString_Check(self)) {
		text = DeeString_AsUtf8(self);
		if unlikely(!text)
			goto err;
		size = WSTR_LENGTH(text);
	} else {
		err_expected_string_or_bytes(self);
		goto err;
	}
	/* If the string starts and ends with the same quotation mark, remove them. */
	if (size >= 2 &&
	    text[0] == text[size - 1] &&
	    (text[0] == '\"' || text[0] == '\'')) {
		++text;
		size -= 2;
	}
	return DeeString_FromBackslashEscaped(text, size, error_mode);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
encode_c_escape(DeeObject *__restrict self) {
	Dee_ssize_t error;
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	if (DeeBytes_Check(self)) {
		error = DeeFormat_QuoteBytes(&ascii_printer_print, &printer,
		                             (uint8_t const *)DeeBytes_DATA(self),
		                             DeeBytes_SIZE(self));
	} else if (DeeString_Check(self)) {
		union dcharptr_const str;
		str.ptr = DeeString_WSTR(self);
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			error = DeeFormat_Quote8(&ascii_printer_print, &printer,
			                         str.cp8, WSTR_LENGTH(str.cp8));
			break;

		CASE_WIDTH_2BYTE:
			error = DeeFormat_Quote16(&ascii_printer_print, &printer,
			                          str.cp16, WSTR_LENGTH(str.cp16));
			break;

		CASE_WIDTH_4BYTE:
			error = DeeFormat_Quote32(&ascii_printer_print, &printer,
			                          str.cp32, WSTR_LENGTH(str.cp32));
			break;
		}
	} else {
		err_expected_string_or_bytes(self);
		goto err_ascii_printer;
	}
	if unlikely(error < 0)
		goto err_ascii_printer;
	return ascii_printer_pack(&printer);
err_ascii_printer:
	ascii_printer_fini(&printer);
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
decode_utf16(DeeObject *__restrict self,
             unsigned int error_mode,
             bool little_endian) {
	byte_t const *data;
	size_t size;
	if (DeeBytes_Check(self)) {
		data = DeeBytes_DATA(self);
		size = DeeBytes_SIZE(self);
	} else if (DeeString_Check(self)) {
		data = DeeString_AsBytes(self, error_mode != STRING_ERROR_FSTRICT);
		if unlikely(!data)
			goto err;
		size = WSTR_LENGTH(data);
	} else {
		err_expected_string_or_bytes(self);
err:
		return NULL;
	}
	if (size & 1) {
		/* Uneven (the last byte would not be used) */
		if (error_mode == STRING_ERROR_FSTRICT) {
			DeeError_Throwf(&DeeError_UnicodeDecodeError,
			                "Cannot encode unmatched trailing byte in data block of %" PRFuSIZ " "
			                "bytes, when an number of bytes divisible by 2 was required",
			                size);
			goto err;
		}
	}
	return little_endian
	       ? DeeString_NewUtf16Le((uint16_t *)data, size / 2, error_mode)
	       : DeeString_NewUtf16Be((uint16_t *)data, size / 2, error_mode);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
decode_utf32(DeeObject *__restrict self,
             unsigned int error_mode,
             bool little_endian) {
	byte_t const *data;
	size_t size;
	if (DeeBytes_Check(self)) {
		data = DeeBytes_DATA(self);
		size = DeeBytes_SIZE(self);
	} else if (DeeString_Check(self)) {
		data = DeeString_AsBytes(self, error_mode != STRING_ERROR_FSTRICT);
		if unlikely(!data)
			goto err;
		size = WSTR_LENGTH(data);
	} else {
		err_expected_string_or_bytes(self);
err:
		return NULL;
	}
	if (size & 3) {
		/* Uneven (the last byte would not be used) */
		if (error_mode == STRING_ERROR_FSTRICT) {
			DeeError_Throwf(&DeeError_UnicodeDecodeError,
			                "Cannot encode unmatched trailing byte in data block of %" PRFuSIZ " "
			                "bytes, when an number of bytes divisible by 4 was required",
			                size);
			goto err;
		}
	}
	return little_endian
	       ? DeeString_NewUtf32Le((uint32_t *)data, size / 4, error_mode)
	       : DeeString_NewUtf32Be((uint32_t *)data, size / 4, error_mode);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
encode_utf16(DeeObject *__restrict self,
             unsigned int error_mode) {
	if (DeeBytes_Check(self)) {
		DREF DeeObject *result;
		byte_t const *data;
		size_t size;
		uint16_t *dst;
		data   = DeeBytes_DATA(self);
		size   = DeeBytes_SIZE(self);
		result = DeeBytes_NewBufferUninitialized(size * 2);
		if unlikely(!result)
			goto err;
		dst = (uint16_t *)DeeBytes_DATA(result);
		while (size--)
			UNALIGNED_SET16(dst++, (uint16_t)*data++);
		return result;
	}
	if (DeeString_Check(self)) {
		uint16_t const *data = DeeString_AsUtf16(self, error_mode);
		if unlikely(!data)
			goto err;
		/* Return a bytes-view for the UTF-16 variant of the given string. */
		return DeeBytes_NewViewRo(self, data, WSTR_LENGTH(data) * 2);
	}
	err_expected_string_or_bytes(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
encode_utf32(DeeObject *__restrict self) {
	if (DeeBytes_Check(self)) {
		DREF DeeObject *result;
		byte_t const *data;
		size_t size;
		uint32_t *dst;
		data   = DeeBytes_DATA(self);
		size   = DeeBytes_SIZE(self);
		result = DeeBytes_NewBufferUninitialized(size * 4);
		if unlikely(!result)
			goto err;
		dst = (uint32_t *)DeeBytes_DATA(result);
		while (size--)
			UNALIGNED_SET32(dst++, (uint32_t)*data++);
		return result;
	}
	if (DeeString_Check(self)) {
		uint32_t const *data = DeeString_AsUtf32(self);
		if unlikely(!data)
			goto err;
		/* Return a bytes-view for the UTF-32 variant of the given string. */
		return DeeBytes_NewViewRo(self, data, WSTR_LENGTH(data) * 4);
	}
	err_expected_string_or_bytes(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
encode_utf16_alt(DeeObject *__restrict self,
                 unsigned int error_mode) {
	if (DeeBytes_Check(self)) {
		DREF DeeObject *result;
		byte_t const *data;
		size_t size;
		uint16_t *dst;
		data   = DeeBytes_DATA(self);
		size   = DeeBytes_SIZE(self);
		result = DeeBytes_NewBufferUninitialized(size * 2);
		if unlikely(!result)
			goto err;
		dst = (uint16_t *)DeeBytes_DATA(result);
		while (size--)
			UNALIGNED_SET16_SWAP(dst++, (uint16_t)*data++);
		return result;
	}
	if (DeeString_Check(self)) {
		DREF DeeObject *result;
		uint16_t const *data;
		size_t size;
		uint16_t *dst;
		data = DeeString_AsUtf16(self, error_mode);
		if unlikely(!data)
			goto err;
		size   = WSTR_LENGTH(data);
		result = DeeBytes_NewBufferUninitialized(size * 2);
		if unlikely(!result)
			goto err;
		dst = (uint16_t *)DeeBytes_DATA(result);
		while (size--)
			UNALIGNED_SET16_SWAP(dst++, UNALIGNED_GET16(data++));
		return result;
	}
	err_expected_string_or_bytes(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
encode_utf32_alt(DeeObject *__restrict self) {
	if (DeeBytes_Check(self)) {
		DREF DeeObject *result;
		byte_t const *data;
		size_t size;
		uint32_t *dst;
		data   = DeeBytes_DATA(self);
		size   = DeeBytes_SIZE(self);
		result = DeeBytes_NewBufferUninitialized(size * 4);
		if unlikely(!result)
			goto err;
		dst = (uint32_t *)DeeBytes_DATA(result);
		while (size--)
			UNALIGNED_SET32_SWAP(dst++, (uint32_t)*data++);
		return result;
	}
	if (DeeString_Check(self)) {
		DREF DeeObject *result;
		uint32_t const *data;
		size_t size;
		uint32_t *dst;
		data = DeeString_AsUtf32(self);
		if unlikely(!data)
			goto err;
		size   = WSTR_LENGTH(data);
		result = DeeBytes_NewBufferUninitialized(size * 4);
		if unlikely(!result)
			goto err;
		dst = (uint32_t *)DeeBytes_DATA(result);
		while (size--)
			UNALIGNED_SET32_SWAP(dst++, UNALIGNED_GET32(data++));
		return result;
	}
	err_expected_string_or_bytes(self);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
decode_utf8(DeeObject *__restrict self,
            unsigned int error_mode) {
	byte_t const *data;
	size_t size;
	if (DeeBytes_Check(self)) {
		data = DeeBytes_DATA(self);
		size = DeeBytes_SIZE(self);
	} else if (DeeString_Check(self)) {
		data = DeeString_AsBytes(self, error_mode != STRING_ERROR_FSTRICT);
		if unlikely(!data)
			goto err;
		size = WSTR_LENGTH(data);
	} else {
		err_expected_string_or_bytes(self);
err:
		return NULL;
	}
	return DeeString_NewUtf8((char *)data, size, error_mode);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
encode_utf8(DeeObject *__restrict self) {
	if (DeeBytes_Check(self)) {
		/* LATIN-1 to UTF-8 */
		DREF DeeObject *result;
		uint8_t const *data;
		size_t size;
		byte_t *dst;
		data   = DeeBytes_DATA(self);
		size   = DeeBytes_SIZE(self);
		result = DeeBytes_NewBufferUninitialized(size * 2);
		if unlikely(!result)
			goto err;
		dst = DeeBytes_DATA(result);
		while (size--) {
			uint8_t ch = *data++;
			if (ch >= 0x80) {
				/* Must encode as a byte-pair */
				*dst++ = 0xc0 | ((ch & 0xc0) >> 6);
				*dst++ = 0x80 | (ch & 0x3f);
			} else {
				*dst++ = ch;
			}
		}
		size = (size_t)(dst - DeeBytes_DATA(result));
		ASSERT(size <= DeeBytes_SIZE(result));
		return DeeBytes_TruncateBuffer(result, size);
	}
	if (DeeString_Check(self)) {
		char const *data = DeeString_AsUtf8(self);
		if unlikely(!data)
			goto err;
		/* Return a bytes-view for the UTF-8 variant of the given string. */
		return DeeBytes_NewViewRo(self, data, WSTR_LENGTH(data));
	}
	err_expected_string_or_bytes(self);
err:
	return NULL;
}


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define IS_LITTLE_ENDIAN true
#define encode_utf16_le  encode_utf16
#define encode_utf16_be  encode_utf16_alt
#define encode_utf32_le  encode_utf32
#define encode_utf32_be  encode_utf32_alt
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
#define IS_LITTLE_ENDIAN false
#define encode_utf16_le  encode_utf16_alt
#define encode_utf16_be  encode_utf16
#define encode_utf32_le  encode_utf32_alt
#define encode_utf32_be  encode_utf32
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */

PRIVATE DREF DeeObject *g_libcodecs = NULL;
#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t libcodecs_lock = Dee_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define libcodecs_lock_reading()    Dee_atomic_rwlock_reading(&libcodecs_lock)
#define libcodecs_lock_writing()    Dee_atomic_rwlock_writing(&libcodecs_lock)
#define libcodecs_lock_tryread()    Dee_atomic_rwlock_tryread(&libcodecs_lock)
#define libcodecs_lock_trywrite()   Dee_atomic_rwlock_trywrite(&libcodecs_lock)
#define libcodecs_lock_canread()    Dee_atomic_rwlock_canread(&libcodecs_lock)
#define libcodecs_lock_canwrite()   Dee_atomic_rwlock_canwrite(&libcodecs_lock)
#define libcodecs_lock_waitread()   Dee_atomic_rwlock_waitread(&libcodecs_lock)
#define libcodecs_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&libcodecs_lock)
#define libcodecs_lock_read()       Dee_atomic_rwlock_read(&libcodecs_lock)
#define libcodecs_lock_write()      Dee_atomic_rwlock_write(&libcodecs_lock)
#define libcodecs_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&libcodecs_lock)
#define libcodecs_lock_upgrade()    Dee_atomic_rwlock_upgrade(&libcodecs_lock)
#define libcodecs_lock_downgrade()  Dee_atomic_rwlock_downgrade(&libcodecs_lock)
#define libcodecs_lock_endwrite()   Dee_atomic_rwlock_endwrite(&libcodecs_lock)
#define libcodecs_lock_endread()    Dee_atomic_rwlock_endread(&libcodecs_lock)
#define libcodecs_lock_end()        Dee_atomic_rwlock_end(&libcodecs_lock)

INTERN bool DCALL libcodecs_shutdown(void) {
	DREF DeeObject *old_lib;
	libcodecs_lock_write();
	old_lib     = g_libcodecs;
	g_libcodecs = NULL;
	libcodecs_lock_endwrite();
	if (!old_lib)
		return false;
	Dee_Decref(old_lib);
	return true;
}


PRIVATE WUNUSED DREF DeeObject *DCALL libcodecs_get(void) {
	DREF DeeObject *result;
	libcodecs_lock_read();
	result = g_libcodecs;
	if (result) {
		Dee_Incref(result);
		libcodecs_lock_endread();
		return result;
	}
	libcodecs_lock_endread();
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	result = DeeModule_Import((DeeObject *)&str_codecs, NULL, DeeModule_IMPORT_F_NORMAL);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	result = DeeModule_OpenGlobal((DeeObject *)&str_codecs, NULL, true);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	if likely(result) {
		libcodecs_lock_write();
		ASSERT(!g_libcodecs || g_libcodecs == result);
		if (!g_libcodecs) {
			Dee_Incref(result);
			g_libcodecs = result;
		}
		libcodecs_lock_endwrite();
#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		if unlikely(DeeModule_RunInit(result) < 0)
			Dee_Clear(result);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	}
	return result;
}

PRIVATE DeeStringObject *tpconst error_module_names[] = {
	/* [STRING_ERROR_FSTRICT] = */ &str_strict,
	/* [STRING_ERROR_FREPLAC] = */ &str_replace,
	/* [STRING_ERROR_FIGNORE] = */ &str_ignore
};

struct codec_error {
	char name[8];
	int  flags;
};

PRIVATE struct codec_error const codec_error_db[] = {
	{ "strict",  STRING_ERROR_FSTRICT },
	{ "replace", STRING_ERROR_FREPLAC },
	{ "ignore",  STRING_ERROR_FIGNORE }
};

INTERN WUNUSED NONNULL((1)) unsigned int DCALL
DeeCodec_GetErrorMode(char const *__restrict errors) {
	size_t i;
	for (i = 0; i < COMPILER_LENOF(codec_error_db); ++i) {
		if (strcmp(codec_error_db[i].name, errors) != 0)
			continue;
		return codec_error_db[i].flags;
	}
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid error code %q",
	                errors);
	return (unsigned int)-1;
}



/* @return: ITER_DONE: Not an internal codec. */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeCodec_DecodeIntern(DeeObject *self, DeeObject *name,
                      unsigned int error_mode) {
	char const *name_str;
	ASSERT(error_mode <= COMPILER_LENOF(error_module_names));
	name_str = DeeString_STR(name);
	SWITCH_BUILTIN_CODECS(name_str,
	                      return convert_ascii(self, error_mode, true),
	                      return decode_c_escape(self, error_mode),
	                      return convert_latin1(self, error_mode, true),
	                      return decode_utf16(self, error_mode, IS_LITTLE_ENDIAN),
	                      return decode_utf16(self, error_mode, false),
	                      return decode_utf16(self, error_mode, true),
	                      return decode_utf32(self, error_mode, IS_LITTLE_ENDIAN),
	                      return decode_utf32(self, error_mode, false),
	                      return decode_utf32(self, error_mode, true),
	                      return decode_utf8(self, error_mode));
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCodec_EncodeIntern(DeeObject *self, DeeObject *name,
                      unsigned int error_mode) {
	char const *name_str;
	ASSERT(error_mode <= COMPILER_LENOF(error_module_names));
	name_str = DeeString_STR(name);
	SWITCH_BUILTIN_CODECS(name_str,
	                      return convert_ascii(self, error_mode, false),
	                      return encode_c_escape(self),
	                      return convert_latin1(self, error_mode, false),
	                      return encode_utf16(self, error_mode),
	                      return encode_utf16_be(self, error_mode),
	                      return encode_utf16_le(self, error_mode),
	                      return encode_utf32(self),
	                      return encode_utf32_be(self),
	                      return encode_utf32_le(self),
	                      return encode_utf8(self));
	return ITER_DONE;
}


/* Encode/decode `self' (usually a bytes- or string-object) to/from a codec `name'.
 * These functions will start by normalizing `name', checking if it refers to
 * one of the builtin codecs, and if it doesn't, make an external function
 * call to `encode from codecs' / `decode from codecs':
 * >> name = name.casefold().replace("_", "-");
 * >> if (name.startswith("iso-"))
 * >>     name = "iso" + name[4:];
 * >> else if (name.startswith("cp-")) {
 * >>     name = "cp" + name[3:];
 * >> }
 * >> if (has_builtin_codec(name))
 * >>     return builtin_encode(self, name, error_mode); // or decode...
 * The following is a list of the recognized builtin codecs.
 *  - "ascii", "646", "us-ascii"
 *  - "latin-1", "iso8859-1", "iso8859", "8859", "cp819", "latin", "latin1", "l1"
 *  - "utf-8", "utf8", "u8", "utf"
 *  - "utf-16", "utf16", "u16"
 *  - "utf-16-le", "utf16-le", "u16-le", "utf-16le", "utf16le", "u16le"
 *  - "utf-16-be", "utf16-be", "u16-be", "utf-16be", "utf16be", "u16be"
 *  - "utf-32", "utf32", "u32"
 *  - "utf-32-le", "utf32-le", "u32-le", "utf-32le", "utf32le", "u32le"
 *  - "utf-32-be", "utf32-be", "u32-be", "utf-32be", "utf32be", "u32be"
 *  - "string-escape", "backslash-escape", "c-escape"
 * @throw: ValueError: The given `name' is not a recognized codec name.
 * @param: error_mode: One of `STRING_ERROR_F*'
 * @return: * :   The encoded/decoded variant of `self'
 *                The type of this object is unrelated to `self', but rather
 *                depends on `self' and is usually a bytes, or string object.
 *                In most cases, `DeeCodec_Decode()' returns a string object,
 *                while `DeeCodec_Encode()' returns a Bytes object.
 * @return: NULL: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeCodec_Decode(DeeObject *self, DeeObject *name,
                unsigned int error_mode) {
	DREF DeeObject *result, *libcodecs;
	ASSERT(error_mode <= COMPILER_LENOF(error_module_names));
	name = DeeCodec_NormalizeName(name);
	if unlikely(!name)
		goto err;
	result = DeeCodec_DecodeIntern(self, name, error_mode);
	if (result != ITER_DONE)
		goto done;
	libcodecs = libcodecs_get();
	if unlikely(!libcodecs) {
		if (DeeError_Catch(&DeeError_FileNotFound))
			goto err_unknown; /* Codec library not found */ /* TODO: Pass orig error as "cause" */
		goto err_name;
	}
	result = DeeObject_CallAttrPack(libcodecs,
	                                (DeeObject *)&str___decode,
	                                3,
	                                self,
	                                name,
	                                error_module_names[error_mode]);
	Dee_Decref(libcodecs);
#if 0
	if unlikely(!result) {
		/* Translate any kind of value error into an unknown-codec error.
		 * This includes things such as key-errors thrown by the codec library,
		 * as is likely to be the case when a Dict is used by the implementation. */
		if (DeeError_Catch(&DeeError_ValueError))
			goto err_unknown; /* TODO: Pass orig error as "cause" */
	}
#endif
done:
	Dee_Decref(name);
	return result;
err_unknown:
	err_unknown_codec(name);
err_name:
	Dee_Decref(name);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeCodec_Encode(DeeObject *self, DeeObject *name,
                unsigned int error_mode) {
	DREF DeeObject *result, *libcodecs;
	char const *name_str;
	ASSERT(error_mode < COMPILER_LENOF(error_module_names));
	name = DeeCodec_NormalizeName(name);
	if unlikely(!name)
		goto err;
	name_str = DeeString_STR(name);
	SWITCH_BUILTIN_CODECS(
	name_str,
	{ result = convert_ascii(self, error_mode, false); goto done; },
	{ result = encode_c_escape(self); goto done; },
	{ result = convert_latin1(self, error_mode, false); goto done; },
	{ result = encode_utf16(self, error_mode); goto done; },
	{ result = encode_utf16_be(self, error_mode); goto done; },
	{ result = encode_utf16_le(self, error_mode); goto done; },
	{ result = encode_utf32(self); goto done; },
	{ result = encode_utf32_be(self); goto done; },
	{ result = encode_utf32_le(self); goto done; },
	{ result = encode_utf8(self); goto done; });
	libcodecs = libcodecs_get();
	if unlikely(!libcodecs) {
		if (DeeError_Catch(&DeeError_FileNotFound))
			goto err_unknown; /* Codec library not found */ /* TODO: Pass orig error as "cause" */
		goto err_name;
	}
	result = DeeObject_CallAttrPack(libcodecs,
	                                (DeeObject *)&str___encode,
	                                3,
	                                self,
	                                name,
	                                error_module_names[error_mode]);
	Dee_Decref(libcodecs);
#if 0
	if unlikely(!result) {
		/* Translate any kind of value error into an unknown-codec error.
		 * This includes things such as key-errors thrown by the codec library,
		 * as is likely to be the case when a Dict is used by the implementation. */
		if (DeeError_Catch(&DeeError_ValueError))
			goto err_unknown; /* TODO: Pass orig error as "cause" */
	}
#endif
done:
	Dee_Decref(name);
	return result;
err_unknown:
	err_unknown_codec(name);
err_name:
	Dee_Decref(name);
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_CODEC_C */
