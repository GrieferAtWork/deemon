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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_CODEC_C
#define GUARD_DEEMON_OBJECTS_UNICODE_CODEC_C 1

#include <deemon/api.h>
#include <deemon/bytes.h>
#include <deemon/string.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/module.h>

#include "codec.h"
#include "../../runtime/strings.h"

DECL_BEGIN

#ifdef CONFIG_LITTLE_ENDIAN
#define ENCODE2(a,b)     ((b)<<8|(a))
#define ENCODE4(a,b,c,d) ((d)<<24|(c)<<16|(b)<<8|(a))
#else
#define ENCODE2(a,b)     ((b)|(a)<<8)
#define ENCODE4(a,b,c,d) ((d)|(c)<<8|(b)<<16|(a)<<24)
#endif

INTERN DREF DeeObject *DCALL
DeeCodec_NormalizeName(DeeObject *__restrict name) {
 char *iter,*end,*str; size_t length;
 DREF DeeObject *result;
 ASSERT_OBJECT_TYPE_EXACT(name,&DeeString_Type);
 length = DeeString_SIZE(name);
 iter = str = DeeString_STR(name);
 end  = iter + length;
 for (; iter < end; ++iter) {
  /* TODO: Use case folding to normalize codec names! */
  if (*iter == '_' || DeeUni_IsUpper(*iter)) {
   char *dst;
   result = DeeString_NewBuffer(length);
   if unlikely(!result) goto err;
   memcpy(DeeString_STR(result),str,(size_t)(iter - str)*sizeof(char));
   dst = DeeString_STR(result) + (size_t)(iter - str);
   for (; iter < end; ++iter) {
    if (*iter == '_')
     *dst++ = '-';
    else if (DeeUni_IsUpper(*iter))
     *dst++ = (uint8_t)DeeUni_ToLower(*iter);
    else {
     *dst++ = *iter;
    }
   }
   ASSERT(dst == DeeString_STR(result) + DeeString_SIZE(result));
   if (length >= 4 &&
     *(uint32_t *)DeeString_STR(result) == ENCODE4('i','s','o','-')) {
    --DeeString_SIZE(result);
    memmove(DeeString_STR(result)+3,
            DeeString_STR(result)+4,
           (length - 4)*sizeof(char));
    DeeString_STR(result)[length-2] = '\0';
   } else if (length >= 3 &&
            *(uint16_t *)DeeString_STR(result) == ENCODE2('c','p') &&
              DeeString_STR(result)[2] == '-') {
    --DeeString_SIZE(result);
    memmove(DeeString_STR(result)+2,
            DeeString_STR(result)+3,
           (length - 3)*sizeof(char));
    DeeString_STR(result)[length-2] = '\0';
   }
   return result;
  }
 }
 if (length >= 4 && *(uint32_t *)str == ENCODE4('i','s','o','-')) {
  result = DeeString_NewBuffer(length - 1);
  if unlikely(!result) goto err;
  *(uint16_t *)DeeString_STR(result) = ENCODE2('i','s');
  DeeString_STR(result)[3] = 'o';
  memcpy(DeeString_STR(result) + 3,str + 4,(length - 4)*sizeof(char));
  return result;
 }
 if (length >= 3 && *(uint16_t *)str == ENCODE2('c','p') && str[2] == '-') {
  result = DeeString_NewBuffer(length - 1);
  if unlikely(!result) goto err;
  *(uint16_t *)DeeString_STR(result) = ENCODE2('c','p');
  memcpy(DeeString_STR(result) + 2,str + 3,(length - 3)*sizeof(char));
  return result;
 }
 return_reference_(name);
err:
 return NULL;
}



PRIVATE ATTR_COLD int DCALL
err_unknown_codec(DeeObject *__restrict name) {
 return DeeError_Throwf(&DeeError_ValueError,
                        "Unknown codec %r",
                        name);
}
PRIVATE ATTR_COLD int DCALL
err_expected_string_or_bytes(DeeObject *__restrict self) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "Expected a string or bytes object, but got an instance of %k",
                        self->ob_type);
}


PRIVATE ATTR_COLD int DCALL err_invalid_ascii(uint32_t ch, bool is_decode) {
 return DeeError_Throwf(is_decode ? &DeeError_UnicodeDecodeError
                                  : &DeeError_UnicodeEncodeError,
                        "Invalid ASCII character U+%.4I32X",ch);

}

PRIVATE DREF DeeObject *DCALL
convert_ascii(DeeObject *__restrict self, unsigned int error_mode, bool is_decode) {
 DREF DeeObject *result;
 size_t i,j,size;
 if (DeeBytes_Check(self)) {
  uint8_t *data;
  if (error_mode == STRING_ERROR_FIGNORE)
      goto return_self;
  data = DeeBytes_DATA(self);
  size = DeeBytes_SIZE(self);
  for (i = 0; i < size; ++i) {
   if (data[i] <= 0x7f) continue;
   if (error_mode == STRING_ERROR_FSTRICT) {
    err_invalid_ascii(data[i],is_decode);
    goto err;
   }
   result = DeeBytes_NewBufferUninitialized(size);
   if unlikely(!result) goto err;
   memcpy(DeeBytes_DATA(result),data,i);
   for (; i < size; ++i) {
    uint8_t ch = data[i];
    if (ch > 0x7f) ch = '?';
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
  {
   uint8_t *data;
  CASE_WIDTH_1BYTE:
   data = DeeString_Get1Byte(self);
   size = WSTR_LENGTH(data);
   for (i = 0; i < size; ++i) {
    if (data[i] <= 0x7f) continue;
    if (error_mode == STRING_ERROR_FSTRICT) {
     err_invalid_ascii(data[i],is_decode);
     goto err;
    }
    result = DeeString_NewBuffer(size);
    if unlikely(!result) goto err;
    memcpy(DeeString_STR(result),data,i);
    for (; i < size; ++i) {
     uint8_t ch = data[i];
     if (ch > 0x7f) ch = '?';
     DeeString_STR(result)[i] = (char)ch;
    }
    return result;
   }
   goto return_self;
  } break;
  {
   uint16_t *data;
  CASE_WIDTH_2BYTE:
   data = DeeString_Get2Byte(self);
   size = WSTR_LENGTH(data);
   for (i = 0; i < size; ++i) {
    if (data[i] <= 0x7f) continue;
    if (error_mode == STRING_ERROR_FSTRICT) {
     err_invalid_ascii(data[i],is_decode);
     goto err;
    }
    result = DeeString_NewBuffer(size);
    if unlikely(!result) goto err;
    for (j = 0; j < i; ++j)
       DeeString_STR(result)[j] = (uint8_t)data[j];
    for (; i < size; ++i) {
     uint16_t ch = data[i];
     if (ch > 0x7f) ch = '?';
     DeeString_STR(result)[i] = (char)(uint8_t)ch;
    }
    return result;
   }
   goto return_self;
  } break;
  {
   uint32_t *data;
  CASE_WIDTH_4BYTE:
   data = DeeString_Get4Byte(self);
   size = WSTR_LENGTH(data);
   for (i = 0; i < size; ++i) {
    if (data[i] <= 0x7f) continue;
    if (error_mode == STRING_ERROR_FSTRICT) {
     err_invalid_ascii(data[i],is_decode);
     goto err;
    }
    result = DeeString_NewBuffer(size);
    if unlikely(!result) goto err;
    for (j = 0; j < i; ++j)
       DeeString_STR(result)[j] = (uint8_t)data[j];
    for (; i < size; ++i) {
     uint32_t ch = data[i];
     if (ch > 0x7f) ch = '?';
     DeeString_STR(result)[i] = (char)(uint8_t)ch;
    }
    return result;
   }
   goto return_self;
  } break;
  }
 }
 err_expected_string_or_bytes(self);
err:
 return NULL;
return_self:
 return_reference_(self);
}

PRIVATE DREF DeeObject *DCALL
convert_latin1(DeeObject *__restrict self, unsigned int error_mode, bool is_decode) {
 DREF DeeObject *result;
 size_t i,j,size;
 if (DeeBytes_Check(self))
     goto return_self;
 if (DeeString_Check(self)) {
  if (error_mode == STRING_ERROR_FIGNORE)
      goto return_self;
  SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
  CASE_WIDTH_1BYTE:
   goto return_self;
  {
   uint16_t *data;
  CASE_WIDTH_2BYTE:
   data = DeeString_Get2Byte(self);
   size = WSTR_LENGTH(data);
   for (i = 0; i < size; ++i) {
    if (data[i] <= 0xff) continue;
    if (error_mode == STRING_ERROR_FSTRICT) {
     err_invalid_ascii(data[i],is_decode);
     goto err;
    }
    result = DeeString_NewBuffer(size);
    if unlikely(!result) goto err;
    for (j = 0; j < i; ++j)
       DeeString_STR(result)[j] = (uint8_t)data[j];
    for (; i < size; ++i) {
     uint16_t ch = data[i];
     if (ch > 0xff) ch = '?';
     DeeString_STR(result)[i] = (char)(uint8_t)ch;
    }
    return result;
   }
   goto return_self;
  } break;
  {
   uint32_t *data;
  CASE_WIDTH_4BYTE:
   data = DeeString_Get4Byte(self);
   size = WSTR_LENGTH(data);
   for (i = 0; i < size; ++i) {
    if (data[i] <= 0xff) continue;
    if (error_mode == STRING_ERROR_FSTRICT) {
     err_invalid_ascii(data[i],is_decode);
     goto err;
    }
    result = DeeString_NewBuffer(size);
    if unlikely(!result) goto err;
    for (j = 0; j < i; ++j)
       DeeString_STR(result)[j] = (uint8_t)data[j];
    for (; i < size; ++i) {
     uint32_t ch = data[i];
     if (ch > 0xff) ch = '?';
     DeeString_STR(result)[i] = (char)(uint8_t)ch;
    }
    return result;
   }
   goto return_self;
  } break;
  }
 }
 err_expected_string_or_bytes(self);
err:
 return NULL;
return_self:
 return_reference_(self);
}


PRIVATE DREF DeeObject *DCALL
decode_c_escape(DeeObject *__restrict self, unsigned int error_mode) {
 char const *text; size_t size;
 if (DeeBytes_Check(self)) {
  text = (char *)DeeBytes_DATA(self);
  size = DeeBytes_SIZE(self);
 } else if (DeeString_Check(self)) {
  text = DeeString_AsUtf8(self);
  if unlikely(!text) return NULL;
  size = WSTR_LENGTH(text);
 } else {
  err_expected_string_or_bytes(self);
  return NULL;
 }
 /* If the string starts and ends with the same quotation mark, remove them. */
 if (size >= 2 &&
     text[0] == text[size-1] &&
    (text[0] == '\"' || text[0] == '\''))
     ++text,size -= 2;
 return DeeString_FromBackslashEscaped(text,size,error_mode);
}
PRIVATE DREF DeeObject *DCALL
encode_c_escape(DeeObject *__restrict self) {
 if (DeeBytes_Check(self)) {
  struct bytes_printer printer = BYTES_PRINTER_INIT;
  if unlikely(Dee_FormatQuote((dformatprinter)&bytes_printer_print,&printer,
                             (char *)DeeBytes_DATA(self),
                              DeeBytes_SIZE(self),
                              FORMAT_QUOTE_FNORMAL|
                              FORMAT_QUOTE_FPRINTRAW) < 0)
     goto err_bytes_printer;
  return bytes_printer_pack(&printer);
err_bytes_printer:
  bytes_printer_fini(&printer);
  return NULL;
 }
 if (DeeString_Check(self)) {
  struct ascii_printer printer = ASCII_PRINTER_INIT;
  void *str = DeeString_WSTR(self);
  SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
  CASE_WIDTH_1BYTE:
   if unlikely(Dee_FormatQuote8((dformatprinter)&ascii_printer_print,&printer,
                                (uint8_t *)str,WSTR_LENGTH(str),
                                 FORMAT_QUOTE_FNORMAL|
                                 FORMAT_QUOTE_FPRINTRAW) < 0)
      goto err_ascii_printer;
   break;
  CASE_WIDTH_2BYTE:
   if unlikely(Dee_FormatQuote16((dformatprinter)&ascii_printer_print,&printer,
                                 (uint16_t *)str,WSTR_LENGTH(str),
                                  FORMAT_QUOTE_FNORMAL|
                                  FORMAT_QUOTE_FPRINTRAW) < 0)
      goto err_ascii_printer;
   break;
  CASE_WIDTH_4BYTE:
   if unlikely(Dee_FormatQuote32((dformatprinter)&ascii_printer_print,&printer,
                                 (uint32_t *)str,WSTR_LENGTH(str),
                                  FORMAT_QUOTE_FNORMAL|
                                  FORMAT_QUOTE_FPRINTRAW) < 0)
      goto err_ascii_printer;
   break;
  }
  return ascii_printer_pack(&printer);
err_ascii_printer:
  ascii_printer_fini(&printer);
  return NULL;
 }
 err_expected_string_or_bytes(self);
 return NULL;
}



PRIVATE DREF DeeObject *DCALL
decode_utf16(DeeObject *__restrict self,
             unsigned int error_mode,
             bool little_endian) {
 uint8_t *data; size_t size;
 if (DeeBytes_Check(self)) {
  data = DeeBytes_DATA(self);
  size = DeeBytes_SIZE(self);
 } else if (DeeString_Check(self)) {
  data = DeeString_AsBytes(self,error_mode != STRING_ERROR_FSTRICT);
  if unlikely(!data) goto err;
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
                   "Cannot encode unmatched trailing byte in data block of %Iu bytes, "
                   "when an number of bytes divisible by 2 was required",
                   size);
   goto err;
  }
 }
 return little_endian
      ? DeeString_NewUtf16Le((uint16_t *)data,size / 2,error_mode)
      : DeeString_NewUtf16Be((uint16_t *)data,size / 2,error_mode)
      ;
}
PRIVATE DREF DeeObject *DCALL
decode_utf32(DeeObject *__restrict self,
             unsigned int error_mode,
             bool little_endian) {
 uint8_t *data; size_t size;
 if (DeeBytes_Check(self)) {
  data = DeeBytes_DATA(self);
  size = DeeBytes_SIZE(self);
 } else if (DeeString_Check(self)) {
  data = DeeString_AsBytes(self,error_mode != STRING_ERROR_FSTRICT);
  if unlikely(!data) goto err;
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
                   "Cannot encode unmatched trailing byte in data block of %Iu bytes, "
                   "when an number of bytes divisible by 4 was required",
                   size);
   goto err;
  }
 }
 return little_endian
      ? DeeString_NewUtf32Le((uint32_t *)data,size / 4,error_mode)
      : DeeString_NewUtf32Be((uint32_t *)data,size / 4,error_mode)
      ;
}

PRIVATE DREF DeeObject *DCALL
encode_utf16(DeeObject *__restrict self,
             unsigned int error_mode) {
 if (DeeBytes_Check(self)) {
  DREF DeeObject *result;
  uint8_t *data; size_t size;
  uint16_t *dst;
  data = DeeBytes_DATA(self);
  size = DeeBytes_SIZE(self);
  result = DeeBytes_NewBufferUninitialized(size * 2);
  if unlikely(!result) goto err;
  dst = (uint16_t *)DeeBytes_DATA(result);
  while (size--) *dst++ = (uint16_t)*data++;
  return result;
 }
 if (DeeString_Check(self)) {
  uint16_t *data;
  data = DeeString_AsUtf16(self,error_mode);
  if unlikely(!data) goto err;
  /* Return a bytes-view for the UTF-16 variant of the given string. */
  return DeeBytes_NewView(self,
                          data,
                          WSTR_LENGTH(data) * 2,
                          DEE_BUFFER_FREADONLY);
 }
 err_expected_string_or_bytes(self);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
encode_utf32(DeeObject *__restrict self) {
 if (DeeBytes_Check(self)) {
  DREF DeeObject *result;
  uint8_t *data; size_t size;
  uint32_t *dst;
  data = DeeBytes_DATA(self);
  size = DeeBytes_SIZE(self);
  result = DeeBytes_NewBufferUninitialized(size * 4);
  if unlikely(!result) goto err;
  dst = (uint32_t *)DeeBytes_DATA(result);
  while (size--) *dst++ = (uint32_t)*data++;
  return result;
 }
 if (DeeString_Check(self)) {
  uint32_t *data = DeeString_AsUtf32(self);
  if unlikely(!data) goto err;
  /* Return a bytes-view for the UTF-32 variant of the given string. */
  return DeeBytes_NewView(self,
                          data,
                          WSTR_LENGTH(data) * 8,
                          DEE_BUFFER_FREADONLY);
 }
 err_expected_string_or_bytes(self);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
encode_utf16_alt(DeeObject *__restrict self,
                 unsigned int error_mode) {
 if (DeeBytes_Check(self)) {
  DREF DeeObject *result;
  uint8_t *data; size_t size;
  uint16_t *dst;
  data = DeeBytes_DATA(self);
  size = DeeBytes_SIZE(self);
  result = DeeBytes_NewBufferUninitialized(size * 2);
  if unlikely(!result) goto err;
  dst = (uint16_t *)DeeBytes_DATA(result);
  while (size--) *dst++ = DEE_BSWAP16((uint16_t)*data++);
  return result;
 }
 if (DeeString_Check(self)) {
  DREF DeeObject *result;
  uint16_t *data; size_t size;
  uint16_t *dst;
  data = DeeString_AsUtf16(self,error_mode);
  if unlikely(!data) goto err;
  size = WSTR_LENGTH(data);
  result = DeeBytes_NewBufferUninitialized(size * 2);
  if unlikely(!result) goto err;
  dst = (uint16_t *)DeeBytes_DATA(result);
  while (size--) *dst++ = DEE_BSWAP16((uint16_t)*data++);
  return result;
 }
 err_expected_string_or_bytes(self);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
encode_utf32_alt(DeeObject *__restrict self) {
 if (DeeBytes_Check(self)) {
  DREF DeeObject *result;
  uint8_t *data; size_t size;
  uint32_t *dst;
  data = DeeBytes_DATA(self);
  size = DeeBytes_SIZE(self);
  result = DeeBytes_NewBufferUninitialized(size * 4);
  if unlikely(!result) goto err;
  dst = (uint32_t *)DeeBytes_DATA(result);
  while (size--) *dst++ = DEE_BSWAP32((uint32_t)*data++);
  return result;
 }
 if (DeeString_Check(self)) {
  DREF DeeObject *result;
  uint32_t *data; size_t size;
  uint32_t *dst;
  data = DeeString_AsUtf32(self);
  if unlikely(!data) goto err;
  size = WSTR_LENGTH(data);
  result = DeeBytes_NewBufferUninitialized(size * 4);
  if unlikely(!result) goto err;
  dst = (uint32_t *)DeeBytes_DATA(result);
  while (size--) *dst++ = DEE_BSWAP32((uint32_t)*data++);
  return result;
 }
 err_expected_string_or_bytes(self);
err:
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
decode_utf8(DeeObject *__restrict self,
            unsigned int error_mode) {
 uint8_t *data; size_t size;
 if (DeeBytes_Check(self)) {
  data = DeeBytes_DATA(self);
  size = DeeBytes_SIZE(self);
 } else if (DeeString_Check(self)) {
  data = DeeString_AsBytes(self,error_mode != STRING_ERROR_FSTRICT);
  if unlikely(!data) goto err;
  size = WSTR_LENGTH(data);
 } else {
  err_expected_string_or_bytes(self);
err:
  return NULL;
 }
 return DeeString_NewUtf8((char *)data,size,error_mode);
}

PRIVATE DREF DeeObject *DCALL
encode_utf8(DeeObject *__restrict self) {
 if (DeeBytes_Check(self)) {
  /* LATIN-1 to UTF-8 */
  DREF DeeObject *result;
  uint8_t *data; size_t size;
  uint8_t *dst;
  data = DeeBytes_DATA(self);
  size = DeeBytes_SIZE(self);
  result = DeeBytes_NewBufferUninitialized(size*2);
  if unlikely(!result) goto err;
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
  return DeeBytes_TruncateBuffer(result,size);
 }
 if (DeeString_Check(self)) {
  char *data = DeeString_AsUtf8(self);
  if unlikely(!data) goto err;
  /* Return a bytes-view for the UTF-8 variant of the given string. */
  return DeeBytes_NewView(self,
                          data,
                          WSTR_LENGTH(data),
                          DEE_BUFFER_FREADONLY);
 }
 err_expected_string_or_bytes(self);
err:
 return NULL;
}


#ifdef CONFIG_LITTLE_ENDIAN
#define IS_LITTLE_ENDIAN true
#define encode_utf16_le  encode_utf16
#define encode_utf16_be  encode_utf16_alt
#define encode_utf32_le  encode_utf32
#define encode_utf32_be  encode_utf32_alt
#else
#define IS_LITTLE_ENDIAN false
#define encode_utf16_le  encode_utf16_alt
#define encode_utf16_be  encode_utf16
#define encode_utf32_le  encode_utf32_alt
#define encode_utf32_be  encode_utf32
#endif

PRIVATE DREF DeeObject *libcodecs = NULL;
#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(libcodecs_lock);
#endif

INTERN bool DCALL libcodecs_shutdown(void) {
 DREF DeeObject *old_lib;
 rwlock_write(&libcodecs_lock);
 old_lib = libcodecs;
 libcodecs = NULL;
 rwlock_endwrite(&libcodecs_lock);
 if (!old_lib) return false;
 Dee_Decref(old_lib);
 return true;
}


PRIVATE DREF DeeObject *DCALL libcodecs_get(void) {
 DREF DeeObject *result;
 rwlock_read(&libcodecs_lock);
 result = libcodecs;
 if (result) {
  Dee_Incref(result);
  rwlock_endread(&libcodecs_lock);
  return result;
 }
 rwlock_endread(&libcodecs_lock);
 result = DeeModule_Open(&str_codecs,NULL,true);
 if likely(result) {
  rwlock_write(&libcodecs_lock);
  ASSERT(!libcodecs || libcodecs == result);
  if (!libcodecs) {
   Dee_Incref(result);
   libcodecs = result;
  }
  rwlock_endwrite(&libcodecs_lock);
  if unlikely(DeeModule_RunInit(result) < 0)
     Dee_Clear(result);
 }
 return result;
}

PRIVATE DeeObject *error_module_names[] = {
    /* [STRING_ERROR_FSTRICT] = */&str_strict,
    /* [STRING_ERROR_FREPLAC] = */&str_replace,
    /* [STRING_ERROR_FIGNORE] = */&str_ignore
};

struct codec_error {
    char name[8];
    int  flags;
};

PRIVATE struct codec_error const codec_error_db[] = {
    { "strict", STRING_ERROR_FSTRICT },
    { "replace", STRING_ERROR_FREPLAC },
    { "ignore", STRING_ERROR_FIGNORE }
};

INTERN unsigned int DCALL
DeeCodec_GetErrorMode(char const *__restrict errors) {
 size_t i;
 for (i = 0; i < COMPILER_LENOF(codec_error_db); ++i) {
  if (strcmp(codec_error_db[i].name,errors) != 0)
      continue;
  return codec_error_db[i].flags;
 }
 DeeError_Throwf(&DeeError_ValueError,
                 "Invalid error code %q",
                 errors);
 return (unsigned int)-1;
}



/* @return: ITER_DONE: Not an internal codec. */
INTERN DREF DeeObject *DCALL
DeeCodec_DecodeIntern(DeeObject *__restrict self,
                      DeeObject *__restrict name,
                      unsigned int error_mode) {
 char const *name_str;
 ASSERT(error_mode <= COMPILER_LENOF(error_module_names));
 name_str = DeeString_STR(name);
 SWITCH_BUILTIN_CODECS(
  name_str,
  return convert_ascii(self,error_mode,true),
  return decode_c_escape(self,error_mode),
  return convert_latin1(self,error_mode,true),
  return decode_utf16(self,error_mode,IS_LITTLE_ENDIAN),
  return decode_utf16(self,error_mode,false),
  return decode_utf16(self,error_mode,true),
  return decode_utf32(self,error_mode,IS_LITTLE_ENDIAN),
  return decode_utf32(self,error_mode,false),
  return decode_utf32(self,error_mode,true),
  return decode_utf8(self,error_mode)
 );
 return ITER_DONE;
}

INTERN DREF DeeObject *DCALL
DeeCodec_EncodeIntern(DeeObject *__restrict self,
                      DeeObject *__restrict name,
                      unsigned int error_mode) {
 char const *name_str;
 ASSERT(error_mode <= COMPILER_LENOF(error_module_names));
 name_str = DeeString_STR(name);
 SWITCH_BUILTIN_CODECS(
  name_str,
  return convert_ascii(self,error_mode,false),
  return encode_c_escape(self),
  return convert_latin1(self,error_mode,false),
  return encode_utf16(self,error_mode),
  return encode_utf16_be(self,error_mode),
  return encode_utf16_le(self,error_mode),
  return encode_utf32(self),
  return encode_utf32_be(self),
  return encode_utf32_le(self),
  return encode_utf8(self)
 );
 return ITER_DONE;
}


PUBLIC DREF DeeObject *DCALL
DeeCodec_Decode(DeeObject *__restrict self,
                DeeObject *__restrict name,
                unsigned int error_mode) {
 DREF DeeObject *result,*libcodecs;
 ASSERT(error_mode <= COMPILER_LENOF(error_module_names));
 name = DeeCodec_NormalizeName(name);
 if unlikely(!name) goto err;
 result = DeeCodec_DecodeIntern(self,name,error_mode);
 if (result != ITER_DONE) goto done;
 libcodecs = libcodecs_get();
 if unlikely(!libcodecs) {
  if (DeeError_Catch(&DeeError_FileNotFound))
      goto err_unknown; /* Codec library not found */
  goto err_name;
 }
 result = DeeObject_CallAttrPack(libcodecs,
                                &str_decode,
                                 3,
                                 self,
                                 name,
                                 error_module_names[error_mode]);
 Dee_Decref(libcodecs);
 if unlikely(!result) {
  /* Translate any kind of value error into an unknown-codec error.
   * This includes things such as key-errors thrown by the codec library,
   * as is likely to be the case when a dict is used by the implementation. */
  if (DeeError_Catch(&DeeError_ValueError))
      goto err_unknown;
 }
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
PUBLIC DREF DeeObject *DCALL
DeeCodec_Encode(DeeObject *__restrict self,
                DeeObject *__restrict name,
                unsigned int error_mode) {
 DREF DeeObject *result,*libcodecs;
 char const *name_str;
 ASSERT(error_mode <= COMPILER_LENOF(error_module_names));
 name = DeeCodec_NormalizeName(name);
 if unlikely(!name) goto err;
 name_str = DeeString_STR(name);
 SWITCH_BUILTIN_CODECS(
  name_str,
  { result = convert_ascii(self,error_mode,false); goto done; },
  { result = encode_c_escape(self); goto done; },
  { result = convert_latin1(self,error_mode,false); goto done; },
  { result = encode_utf16(self,error_mode); goto done; },
  { result = encode_utf16_be(self,error_mode); goto done; },
  { result = encode_utf16_le(self,error_mode); goto done; },
  { result = encode_utf32(self); goto done; },
  { result = encode_utf32_be(self); goto done; },
  { result = encode_utf32_le(self); goto done; },
  { result = encode_utf8(self); goto done; }
 );
 libcodecs = libcodecs_get();
 if unlikely(!libcodecs) {
  if (DeeError_Catch(&DeeError_FileNotFound))
      goto err_unknown; /* Codec library not found */
  goto err_name;
 }
 result = DeeObject_CallAttrPack(libcodecs,
                                &str_encode,
                                 3,
                                 self,
                                 name,
                                 error_module_names[error_mode]);
 Dee_Decref(libcodecs);
 if unlikely(!result) {
  /* Translate any kind of value error into an unknown-codec error.
   * This includes things such as key-errors thrown by the codec library,
   * as is likely to be the case when a dict is used by the implementation. */
  if (DeeError_Catch(&DeeError_ValueError))
      goto err_unknown;
 }
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
