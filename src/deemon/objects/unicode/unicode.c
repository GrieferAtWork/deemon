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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_C
#define GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/util/cache.h>
#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#include <hybrid/minmax.h>
#ifndef CONFIG_NO_THREADS
#include <hybrid/atomic.h>
#endif

#if 0
#ifdef CONFIG_HOST_WINDOWS
#    include <Windows.h>
#else /* CONFIG_HOST_WINDOWS */
#    include <locale.h>
#    include <wchar.h>
#ifndef CONFIG_NO_STDIO
#    include <stdio.h>
#endif /* !CONFIG_NO_STDIO */
#endif /* !CONFIG_HOST_WINDOWS */
#endif

DECL_BEGIN

STATIC_ASSERT_MSG(sizeof(char) == sizeof(uint8_t),"Probably won't work...");
STATIC_ASSERT(STRING_SIZEOF_WIDTH(STRING_WIDTH_1BYTE) == 1);
STATIC_ASSERT(STRING_SIZEOF_WIDTH(STRING_WIDTH_2BYTE) == 2);
STATIC_ASSERT(STRING_SIZEOF_WIDTH(STRING_WIDTH_4BYTE) == 4);


#ifndef __USE_KOS
#define memcpyb(dst,src,n)             memcpy(dst,src,n)
#if defined(_MSC_VER) && (defined(__i386__) || defined(__x86_64__))
#ifdef __x86_64__
extern void __movsw(unsigned short *, unsigned short const *, unsigned __int64);
extern void __movsd(unsigned long *, unsigned long const *, unsigned __int64);
#else
extern void __movsw(unsigned short *, unsigned short const *, unsigned long);
extern void __movsd(unsigned long *, unsigned long const *, unsigned long);
#endif
#pragma intrinsic(__movsw)
#pragma intrinsic(__movsd)
#define memcpyw(dst,src,n) __movsw((unsigned short *)(dst),(unsigned short const *)(src),(size_t)(n))
#define memcpyl(dst,src,n) __movsd((unsigned long *)(dst),(unsigned long const *)(src),(size_t)(n))
#else
#define memcpyw(dst,src,n)             memcpy(dst,src,(n)*2)
#define memcpyl(dst,src,n)             memcpy(dst,src,(n)*4)
#endif
#endif /* __USE_KOS */


PUBLIC uint8_t const utf8_sequence_len[256] = {
    /* ASCII */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x00-0x0f */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x10-0x1f */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x20-0x2f */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x30-0x3f */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x40-0x4f */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x50-0x5f */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x60-0x6f */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x70-0x7f */
    /* Unicode follow-up word (`0b10??????'). */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x80-0x8f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x90-0x9f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xa0-0xaf */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xb0-0xbf */
    /* `0b110?????' */
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* 0xc0-0xcf */
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* 0xd0-0xdf */
    /* `0b1110????' */
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, /* 0xe0-0xef */
    /* `0b11110???' */
    4,4,4,4,4,4,4,4,
    5,5,5,5,
    6,6,
    7,
    8
};


/* (Theoretical) utf-8 unicode sequence ranges:
 *  - 1-byte    -- 7               = 7 bits
 *  - 2-byte    -- 5+6             = 11 bits
 *  - 3-byte    -- 4+6+6           = 16 bits
 *  - 4-byte    -- 3+6+6+6         = 21 bits
 *  - 5-byte    -- 2+6+6+6+6       = 26 bits (Not valid unicode characters)
 *  - 6-byte    -- 1+6+6+6+6+6     = 31 bits (Not valid unicode characters)
 *  - 7-byte    --   6+6+6+6+6+6   = 36 bits (Not valid unicode characters)
 *  - 8-byte    --   6+6+6+6+6+6+6 = 42 bits (Not valid unicode characters)
 */
#define UTF8_1BYTE_MAX   (((uint32_t)1 << 7)-1)
#define UTF8_2BYTE_MAX   (((uint32_t)1 << 11)-1)
#define UTF8_3BYTE_MAX   (((uint32_t)1 << 16)-1)
#define UTF8_4BYTE_MAX   (((uint32_t)1 << 21)-1)
#define UTF8_5BYTE_MAX   (((uint32_t)1 << 26)-1)
#define UTF8_6BYTE_MAX   (((uint32_t)1 << 31)-1)


#define UTF16_HIGH_SURROGATE_MIN 0xd800
#define UTF16_HIGH_SURROGATE_MAX 0xdbff
#define UTF16_LOW_SURROGATE_MIN  0xdc00
#define UTF16_LOW_SURROGATE_MAX  0xdfff
#define UTF16_SURROGATE_SHIFT    0x10000

#define UTF16_COMBINE_SURROGATES(high,low) \
     ((((uint32_t)(high) - UTF16_HIGH_SURROGATE_MIN) << 10 | \
       ((uint32_t)(low) - UTF16_LOW_SURROGATE_MIN)) + UTF16_SURROGATE_SHIFT)





/* String functions related to unicode. */

DECLARE_STRUCT_CACHE(utf,struct string_utf)
#ifndef NDEBUG
#define utf_alloc()  utf_dbgalloc(__FILE__,__LINE__)
#endif
typedef DeeStringObject String;


/* Preallocated latin1 character table.
 * Elements are lazily allocated. */
PRIVATE DREF String *latin1_chars[256] = { NULL, };
#ifndef CONFIG_NO_THREADS
PRIVATE rwlock_t latin1_chars_lock = RWLOCK_INIT;
#endif

INTERN size_t DCALL
latincache_clear(size_t max_clear) {
 size_t result = 0;
 DREF String **iter = latin1_chars;
#ifndef CONFIG_NO_THREADS
again:
 rwlock_write(&latin1_chars_lock);
#endif
 for (; iter != COMPILER_ENDOF(latin1_chars); ++iter) {
  DREF String *ob = *iter;
  if (!ob) continue;
  *iter = NULL;
  if (!Dee_DecrefIfNotOne(ob)) {
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&latin1_chars_lock);
#endif
   result += offsetof(String,s_str)+2*sizeof(char);
   Dee_Decref(ob);
   if (result >= max_clear) goto done;
#ifndef CONFIG_NO_THREADS
   goto again;
#endif
  }
 }
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&latin1_chars_lock);
#endif
done:
 return result;
}

PUBLIC DREF DeeObject *
(DCALL _DeeString_Chr8)(uint8_t ch) {
 DREF String *result;
#ifndef CONFIG_NO_THREADS
 rwlock_read(&latin1_chars_lock);
#endif
 result = latin1_chars[ch];
 if (result) {
  Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&latin1_chars_lock);
#endif
 } else {
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&latin1_chars_lock);
#endif
  result = (DREF String *)DeeString_NewBuffer(1);
  if unlikely(!result) return NULL;
  result->s_str[0] = (char)ch;
#ifndef CONFIG_NO_THREADS
  rwlock_write(&latin1_chars_lock);
#endif
  if unlikely(latin1_chars[ch] != NULL) {
   DREF String *new_result = latin1_chars[ch];
   /* Special case: The string has been created in the mean time.
    * This can even happen when threading is disabled, in case
    * a GC callback invoked during allocation of our string did
    * the job. */
   Dee_Incref(new_result);
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&latin1_chars_lock);
#endif
   Dee_Decref(result);
   return (DREF DeeObject *)new_result;
  }
  Dee_Incref(result); /* The reference stored in `latin1_chars' */
  latin1_chars[ch] = result;
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&latin1_chars_lock);
#endif
 }
 return (DREF DeeObject *)result;
}


PUBLIC uint16_t *(DCALL DeeString_As2Byte)(DeeObject *__restrict self) {
 struct string_utf *utf;
again:
 utf = ((String *)self)->s_data;
 if (!utf) {
  utf = string_utf_alloc();
  if unlikely(!utf) goto err;
  memset(utf,0,sizeof(struct string_utf));
  utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)DeeString_STR(self);
#if STRING_WIDTH_1BYTE != 0
  utf->u_width = STRING_WIDTH_1BYTE;
#endif
  if (!ATOMIC_CMPXCH(((String *)self)->s_data,NULL,utf)) {
   utf_free(utf);
   goto again;
  }
  Dee_UntrackAlloc(utf);
 }
 ASSERT(utf->u_width <= STRING_WIDTH_2BYTE);
 if likely(!utf->u_data[STRING_WIDTH_2BYTE]) {
  uint16_t *result; size_t i,length; uint8_t *data;
  ASSERT(utf->u_width != STRING_WIDTH_2BYTE);
  ASSERT(utf->u_width == STRING_WIDTH_1BYTE);
  data   = (uint8_t *)DeeString_STR(self);
  length = DeeString_SIZE(self);
  result = DeeString_NewBuffer16(length);
  if unlikely(!result) goto err;
  for (i = 0; i < length; ++i)
      result[i] = data[i];
  result[length] = 0;
  if likely(ATOMIC_CMPXCH(utf->u_data[STRING_WIDTH_2BYTE],NULL,result)) {
   Dee_UntrackAlloc((size_t *)result - 1);
   return result;
  }
  Dee_Free(result);
 }
 return (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE];
err:
 return NULL;
}
PUBLIC uint32_t *(DCALL DeeString_As4Byte)(DeeObject *__restrict self) {
 struct string_utf *utf;
again:
 utf = ((String *)self)->s_data;
 if (!utf) {
  utf = string_utf_alloc();
  if unlikely(!utf) goto err;
  memset(utf,0,sizeof(struct string_utf));
  utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)DeeString_STR(self);
#if STRING_WIDTH_1BYTE != 0
  utf->u_width = STRING_WIDTH_1BYTE;
#endif
  if (!ATOMIC_CMPXCH(((String *)self)->s_data,NULL,utf)) {
   utf_free(utf);
   goto again;
  }
  Dee_UntrackAlloc(utf);
 }
 ASSERT(utf->u_width <= STRING_WIDTH_4BYTE);
 if likely(!utf->u_data[STRING_WIDTH_4BYTE]) {
  uint32_t *result; size_t i,length;
  ASSERT(utf->u_width != STRING_WIDTH_4BYTE);
  if (utf->u_width == STRING_WIDTH_1BYTE) {
   uint8_t *data;
   data   = (uint8_t *)DeeString_STR(self);
   length = DeeString_SIZE(self);
   result = DeeString_NewBuffer32(length);
   if unlikely(!result) goto err;
   for (i = 0; i < length; ++i)
       result[i] = data[i];
  } else {
   uint16_t *data;
   ASSERT(utf->u_width == STRING_WIDTH_2BYTE);
   ASSERT(utf->u_data[STRING_WIDTH_2BYTE]);
   data   = (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE];
   length = WSTR_LENGTH(data);
   result = DeeString_NewBuffer32(length);
   if unlikely(!result) goto err;
   for (i = 0; i < length; ++i)
       result[i] = data[i];
  }
  result[length] = 0;
  if likely(ATOMIC_CMPXCH(utf->u_data[STRING_WIDTH_4BYTE],NULL,result)) {
   Dee_UntrackAlloc((size_t *)result - 1);
   return result;
  }
  Dee_Free(result);
 }
 return (uint32_t *)utf->u_data[STRING_WIDTH_4BYTE];
err:
 return NULL;
}



PUBLIC char *DCALL
DeeString_AsUtf8(DeeObject *__restrict self) {
 struct string_utf *utf;
 uint8_t *iter,*end;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
again:
 utf = ((String *)self)->s_data;
 if (utf) {
  if (utf->u_utf8)
      return utf->u_utf8;
  if ((utf->u_flags & STRING_UTF_FASCII) ||
      (utf->u_width != STRING_WIDTH_1BYTE)) {
set_utf8_and_return_1byte:
   ATOMIC_WRITE(utf->u_utf8,DeeString_STR(self));
   return DeeString_STR(self);
  }
 }
 /* We are either a LATIN1, or an ASCII string. */
 if (!utf) {
  /* Allocate the UTF-buffer. */
  utf = utf_alloc();
  if unlikely(!utf) goto err;
  memset(utf,0,sizeof(struct string_utf));
  utf->u_width                    = STRING_WIDTH_1BYTE;
  utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)DeeString_STR(self);
  if (!ATOMIC_CMPXCH(((String *)self)->s_data,NULL,utf)) {
   utf_free(utf);
   goto again;
  }
  Dee_UntrackAlloc(utf);
 }
 iter = (uint8_t *)DeeString_STR(self);
 end = iter + DeeString_SIZE(self);
 for (; iter < end; ++iter) {
  uint8_t ch = *iter;
  size_t result_length;
  uint8_t *iter2,*result,*dst;
  if (ch <= 0x7f) continue; /* ASCII character. */
  /* Well... This string _does_ contain some latin1 characters. */
  result_length = DeeString_SIZE(self)+1;
  iter2 = iter+1;
  for (; iter2 < end; ++iter2) {
   if (*iter2 >= 0x80) ++result_length;
  }
  result = (uint8_t *)Dee_Malloc(sizeof(size_t)+
                                (result_length+1)*
                                 sizeof(uint8_t));
  if unlikely(!result) goto err;
  *(size_t *)result = result_length;
  result += sizeof(size_t);
  /* Copy leading ASCII-only data. */
  memcpy(result,DeeString_STR(self),
        (size_t)(iter-(uint8_t *)DeeString_STR(self))*
         sizeof(uint8_t));
  dst = result + (size_t)(iter-(uint8_t *)DeeString_STR(self));
  for (; iter < end; ++iter) {
   ch = *iter;
   if (ch <= 0x7f)
    *dst++ = ch;
   else {
    /* Encode the LATIN-1 character in UTF-8 */
    *dst++ = 0xc0 | ((ch & 0xc0) >> 6);
    *dst++ = 0x80 | (ch & 0x3f);
   }
  }
  ASSERT(WSTR_LENGTH(result) == result_length);
  ASSERT(dst == result + result_length);
  *dst = '\0';
  /* Save the generated UTF-8 string in the string's UTF cache. */
  if (!ATOMIC_CMPXCH(utf->u_utf8,NULL,(char *)result)) {
   Dee_Free((size_t *)result - 1);
   ASSERT(utf->u_utf8 != NULL);
   return utf->u_utf8;
  }
  Dee_UntrackAlloc((size_t *)result - 1);
  return (char *)result;
 }
 /* No latin1 characters here! */
 ATOMIC_FETCHOR(utf->u_flags,STRING_UTF_FASCII);
 goto set_utf8_and_return_1byte;
err:
 return NULL;
}

PUBLIC uint8_t *DCALL
DeeString_AsBytes(DeeObject *__restrict self, bool allow_invalid) {
 struct string_utf *utf; void *str; bool contains_invalid;
 uint8_t *result; size_t i,length;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 utf = ((String *)self)->s_data;
 /* Simple case: No UTF data, or 1-byte string means that the string's
  *              1-byte variant has no special encoding (aka. is LATIN-1). */
 if (!utf || utf->u_width == STRING_WIDTH_1BYTE)
      return (uint8_t *)DeeString_STR(self);
 /* The single-byte variant of the string was already allocated.
  * Check how that variant's INV-byte flag compares to the caller's
  * allow_invalid option. */
 if (!allow_invalid && (utf->u_flags & STRING_UTF_FINVBYT))
     goto err_invalid_string;
 /* Check for a cached single-byte variant. */
 result = (uint8_t *)utf->u_data[STRING_WIDTH_1BYTE];
 if (result) return result;
 /* Since strings are allowed to use a wider default width than they
  * may actually need, `self' may still only contain characters that
  * fit into the 00-FF unicode range, so regardless of `allow_invalid'
  * (which controls the behavior for characters outside that range) */
 str = utf->u_data[utf->u_width];
 ASSERT(utf->u_width != STRING_WIDTH_1BYTE);
 length = WSTR_LENGTH(str);
 ASSERT(length <= DeeString_SIZE(self));
 if (length == DeeString_SIZE(self)) {
  /* The actual length of the string matches the length of of its single-byte
   * variant, in other words meaning that all of its characters could fit into
   * that range, and that the string consists only of ASCII characters. */
  ATOMIC_FETCHOR(utf->u_flags,STRING_UTF_FASCII);
  utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)DeeString_STR(self);
  return (uint8_t *)DeeString_STR(self);
 }
 /* Try to construct the single-byte variant. */
 result = (uint8_t *)Dee_Malloc(sizeof(size_t)+
                               (length+1)*sizeof(uint8_t));
 if unlikely(!result) goto err;
 *(size_t *)result = length;
 result += sizeof(size_t);
 result[length] = 0;
 contains_invalid = false;
 SWITCH_SIZEOF_WIDTH(utf->u_width) {
 case STRING_WIDTH_2BYTE:
  for (i = 0; i < length; ++i) {
   uint16_t ch = ((uint16_t *)str)[i];
   if (ch > 0xff) {
    if (!allow_invalid)
         goto err_result;
    contains_invalid = true;
    ch = '?';
   }
   result[i] = (uint8_t)ch;
  }
  break;
 case STRING_WIDTH_4BYTE:
  for (i = 0; i < length; ++i) {
   uint32_t ch = ((uint32_t *)str)[i];
   if (ch > 0xff) {
    if (!allow_invalid)
         goto err_result;
    contains_invalid = true;
    ch = '?';
   }
   result[i] = (uint8_t)ch;
  }
  break;
 default: __builtin_unreachable();
 }
 /* All right. - We've managed to construct the single-byte variant. */
 if (contains_invalid)
     ATOMIC_FETCHOR(utf->u_flags,STRING_UTF_FINVBYT);
 /* Deal with race conditions. */
 if (!ATOMIC_CMPXCH(utf->u_data[STRING_WIDTH_1BYTE],NULL,(size_t *)result)) {
  Dee_Free((size_t *)result - 1);
  return (uint8_t *)utf->u_data[STRING_WIDTH_1BYTE];
 }
 Dee_UntrackAlloc((size_t *)result - 1);
 return result;
err_result:
 Dee_Free((size_t *)result - 1);
err_invalid_string:
 DeeError_Throwf(&DeeError_ValueError,
                 "The string contains characters that don't fit into a single byte");
err:
 return NULL;
}



PUBLIC uint16_t *DCALL
DeeString_AsUtf16(DeeObject *__restrict self, unsigned int error_mode) {
 uint16_t *result;
 struct string_utf *utf;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 utf = ((String *)self)->s_data;
 if (utf) {
  if (utf->u_utf16)
      return (uint16_t *)utf->u_utf16;
  if (utf->u_width == STRING_WIDTH_1BYTE) {
   /* A single-byte string has no characters within the surrogate-range,
    * meaning we can simply request the string's 2-byte variant, and we'll
    * automatically be given a valid utf-16 string! */
load_2byte_width:
   result = (uint16_t *)DeeString_As2Byte(self);
   if unlikely(!result) goto err;
   utf = ((String *)self)->s_data;
   ASSERT(utf);
   ASSERT(result == (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE]);
   *(uint16_t **)&utf->u_utf16 = result;
   return result;
  }
 } else {
  /* Without a UTF descriptor, we know that all characters are within
   * the U+0000 ... U+00FF range, meaning we can simply request the 2-byte
   * variant of the string, and automatically be given a valid utf-16 string. */
  goto load_2byte_width;
 }
 /* The complicated case: the string probably
  * contains characters that need to be escaped. */
 ASSERT(utf);
 ASSERT(utf->u_width != STRING_WIDTH_1BYTE);
 SWITCH_SIZEOF_WIDTH(utf->u_width) {
 {
  size_t i,length;
  uint16_t *str,*result,*dst;
 CASE_WIDTH_2BYTE:
  str    = (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE];
  length = WSTR_LENGTH(str);
  /* Search if the string contains surrogate-characters. */
  for (i = 0; i < length; ++i) {
   uint16_t ch = str[i];
   if ((ch < UTF16_HIGH_SURROGATE_MIN || ch > UTF16_HIGH_SURROGATE_MAX) &&
       (ch < UTF16_LOW_SURROGATE_MIN || ch > UTF16_LOW_SURROGATE_MAX))
        continue;
   if (!(error_mode & (STRING_ERROR_FREPLAC|STRING_ERROR_FIGNORE))) {
    DeeError_Throwf(&DeeError_UnicodeEncodeError,
                    "Invalid UTF-16 character U+%.4I16X",ch);
    goto err;
   }
   /* Must generate a fixed variant. */
   result = DeeString_NewBuffer16(length);
   if unlikely(!result) goto err;
   memcpyw(result,str,i);
   dst = result + 1;
   if (!(error_mode & STRING_ERROR_FIGNORE))
         *dst++ = '?';
   while (++i < length) {
    ch = str[i];
    if ((ch >= UTF16_HIGH_SURROGATE_MIN && ch <= UTF16_HIGH_SURROGATE_MAX) ||
        (ch >= UTF16_LOW_SURROGATE_MIN && ch <= UTF16_LOW_SURROGATE_MAX)) {
     if (error_mode & STRING_ERROR_FIGNORE)
         continue;
     ch = '?';
    }
    *dst++ = ch;
   }
   *dst = 0;
   if (error_mode & STRING_ERROR_FIGNORE) {
    uint16_t *new_result;
    new_result = DeeString_TryResizeBuffer16(result,(size_t)(dst - result));
    if likely(new_result) result = new_result;
   }
   /* Save the utf-16 encoded string. */
   if (!ATOMIC_CMPXCH(*(uint16_t **)&utf->u_utf16,NULL,result)) {
    DeeString_Free2ByteBuffer(result);
    return (uint16_t *)utf->u_utf16;
   }
   Dee_UntrackAlloc((size_t *)result - 1);
   return result;
  }
  /* The 2-byte variant doesn't contain any illegal characters,
   * so we can simply re-use it as the utf-16 variant. */
  ATOMIC_CMPXCH(*(uint16_t **)&utf->u_utf16,NULL,str);
  return (uint16_t *)utf->u_utf16;
 } break;

 {
  size_t i,length,result_length;
  uint32_t *str; uint16_t *result,*dst;
 CASE_WIDTH_4BYTE:
  /* The string is full-on UTF-32.
   * -> Count the number of characters that need surrogates */
  str = (uint32_t *)utf->u_data[STRING_WIDTH_4BYTE];
  result_length = length = WSTR_LENGTH(str);
  for (i = 0; i < length; ++i) {
   uint32_t ch = str[i];
   if (ch <= 0xffff) {
    if ((ch >= UTF16_HIGH_SURROGATE_MIN && ch <= UTF16_HIGH_SURROGATE_MAX) ||
        (ch >= UTF16_LOW_SURROGATE_MIN && ch <= UTF16_LOW_SURROGATE_MAX))
         goto err_invalid_unicode;
    continue;
   } else if (ch > 0x10ffff) {
err_invalid_unicode:
    if (!(error_mode & (STRING_ERROR_FREPLAC|STRING_ERROR_FIGNORE))) {
     DeeError_Throwf(&DeeError_UnicodeEncodeError,
                     "Invalid unicode character U+%.4I32X",ch);
     goto err;
    }
    if (error_mode & STRING_ERROR_FIGNORE)
        --result_length;
    continue;
   }
   ++result_length;
  }
  result = DeeString_NewBuffer16(result_length);
  if unlikely(!result) goto err;
  dst = result;
  for (i = 0; i < length; ++i) {
   uint32_t ch = str[i];
   if (ch <= 0xffff) {
    if ((ch >= UTF16_HIGH_SURROGATE_MIN && ch <= UTF16_HIGH_SURROGATE_MAX) ||
        (ch >= UTF16_LOW_SURROGATE_MIN && ch <= UTF16_LOW_SURROGATE_MAX)) {
     if (error_mode & STRING_ERROR_FIGNORE)
         continue;
     ch = '?';
    }
    *dst++ = (uint16_t)ch;
   } else if (ch > 0x10ffff) {
    if (error_mode & STRING_ERROR_FIGNORE)
        continue;
    *dst++ = '?';
   } else {
    /* Must encode as a high/low surrogate pair. */
    *dst++ = UTF16_HIGH_SURROGATE_MIN+(uint16_t)(ch >> 10);
    *dst++ = UTF16_LOW_SURROGATE_MIN+(uint16_t)(ch & 0x3ff);
   }
  }
  ASSERT(dst == result + result_length);
  *dst = 0;
  /* Save the utf-16 encoded string. */
  if (!ATOMIC_CMPXCH(*(uint16_t **)&utf->u_utf16,NULL,result)) {
   DeeString_Free2ByteBuffer(result);
   return (uint16_t *)utf->u_utf16;
  }
  Dee_UntrackAlloc((size_t *)result - 1);
  return result;
 } break;

 default: __builtin_unreachable();
 }
err:
 return NULL;
}



PUBLIC dssize_t DCALL
unicode_printer_printstring(struct unicode_printer *__restrict self,
                            DeeObject *__restrict string) {
 struct string_utf *utf; void *str;
 ASSERT_OBJECT_TYPE_EXACT(string,&DeeString_Type);
 utf = ((String *)string)->s_data;
 /* Try to optimize, by printing character data directly. */
 if (!utf) {
  /* Raw, 8-bit character (aka. LATIN-1) */
  return unicode_printer_print8(self,
                               (uint8_t *)DeeString_STR(string),
                                DeeString_SIZE(string));
 }
 /* Directly print sized string data (without the need to convert to & from UTF-8). */
 str = utf->u_data[utf->u_width];
 SWITCH_SIZEOF_WIDTH(utf->u_width) {
 CASE_WIDTH_1BYTE:
  return unicode_printer_print8(self,(uint8_t *)str,WSTR_LENGTH(str));
 CASE_WIDTH_2BYTE:
  return unicode_printer_print16(self,(uint16_t *)str,WSTR_LENGTH(str));
 CASE_WIDTH_4BYTE:
  return unicode_printer_print32(self,(uint32_t *)str,WSTR_LENGTH(str));
 }
}


INTERN dssize_t DCALL
DeeString_PrintUtf8(DeeObject *__restrict self,
                    dformatprinter printer,
                    void *arg) {
 struct string_utf *utf;
 uint8_t *iter,*end,*flush_start;
 dssize_t temp,result;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 /* `DeeString_STR()' is the single-byte variant.
  * That only means UTF-8 if the string is an ASCII
  * string, or if its character with is greater than
  * one (in which case the single-byte variant is the
  * UTF-8 version of the string) */
again:
 utf = ((String *)self)->s_data;
 if (printer == (dformatprinter)&unicode_printer_print) {
  void *str;
  /* Try to optimize, by printing character data directly. */
  if (!utf) {
   /* Raw, 8-bit character (aka. LATIN-1) */
   return unicode_printer_print8((struct unicode_printer *)arg,
                                 (uint8_t *)DeeString_STR(self),
                                  DeeString_SIZE(self));
  }
  /* Directly print sized string data (without the need to convert to & from UTF-8). */
  str = utf->u_data[utf->u_width];
  SWITCH_SIZEOF_WIDTH(utf->u_width) {
  CASE_WIDTH_1BYTE:
   return unicode_printer_print8((struct unicode_printer *)arg,(uint8_t *)str,WSTR_LENGTH(str));
  CASE_WIDTH_2BYTE:
   return unicode_printer_print16((struct unicode_printer *)arg,(uint16_t *)str,WSTR_LENGTH(str));
  CASE_WIDTH_4BYTE:
   return unicode_printer_print32((struct unicode_printer *)arg,(uint32_t *)str,WSTR_LENGTH(str));
  }
 }
 if (utf) {
  /* The string has a pre-allocated UTF-8 variant. */
  if (utf->u_utf8)
      return (*printer)(arg,utf->u_utf8,WSTR_LENGTH(utf->u_utf8));
  if ((utf->u_flags & STRING_UTF_FASCII) ||
      (utf->u_width != STRING_WIDTH_1BYTE)) {
   /* The single-byte variant is known to be encoded in UTF-8, or ASCII */
print_ascii:
   ATOMIC_WRITE(utf->u_utf8,DeeString_STR(self));
   return (*printer)(arg,DeeString_STR(self),DeeString_SIZE(self));
  }
 }
 /* The UTF-8 string is encoded in LATIN-1, or ASCII */
 if (!utf) {
  /* Allocate the UTF-buffer. */
  utf = utf_alloc();
  if unlikely(!utf) goto err;
  memset(utf,0,sizeof(struct string_utf));
  utf->u_width = STRING_WIDTH_1BYTE;
  utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)DeeString_STR(self);
  if (!ATOMIC_CMPXCH(((String *)self)->s_data,NULL,utf)) {
   utf_free(utf);
   goto again;
  }
  Dee_UntrackAlloc(utf);
 }
 result = 0;
 iter = flush_start = (uint8_t *)DeeString_STR(self);
 end = iter+DeeString_SIZE(self);
 for (; iter < end; ++iter) {
  uint8_t utf8_encoded[2];
  uint8_t ch = *iter;
  if (ch <= 0x7f) continue; /* ASCII character. */
  /* This is a true LATIN-1 character, which we
   * must manually encode as a 2-byte UTF-8 string. */
  if (iter != flush_start) {
   temp = (*printer)(arg,(char *)flush_start,
                    (size_t)((char *)iter-(char *)flush_start));
   if unlikely(temp < 0) goto err_temp;
   result += temp;
  }
  /* Encode as: 110---xx 10xxxxxx */
  utf8_encoded[0] = 0xc0 | ((ch & 0xc0) >> 6);
  utf8_encoded[1] = 0x80 | (ch & 0x3f);
  temp = (*printer)(arg,(char *)utf8_encoded,2*sizeof(char));
  if unlikely(temp < 0) goto err_temp;
  result += temp;
  flush_start = iter+1;
 }
 if (flush_start == (uint8_t *)DeeString_STR(self)) {
  /* The entire string is ASCII (remember this fact!) */
  ATOMIC_FETCHOR(utf->u_flags,STRING_UTF_FASCII);
  goto print_ascii;
 }
 /* Flush the remainder. */
 if (flush_start != iter) {
  temp = (*printer)(arg,(char *)flush_start,
                   (size_t)((char *)iter-(char *)flush_start));
  if unlikely(temp < 0) goto err_temp;
  result += temp;
 }
 return result;
err_temp:
 return temp;
err:
 return -1;
}

PUBLIC dssize_t DCALL
DeeString_PrintRepr(DeeObject *__restrict self,
                    dformatprinter printer,
                    void *arg, unsigned int flags) {
 void *str;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 str = DeeString_WSTR(self);
 SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
 CASE_WIDTH_1BYTE:
  return Dee_FormatQuote8(printer,arg,(uint8_t *)str,WSTR_LENGTH(str),flags);
 CASE_WIDTH_2BYTE:
  return Dee_FormatQuote16(printer,arg,(uint16_t *)str,WSTR_LENGTH(str),flags);
 CASE_WIDTH_4BYTE:
  return Dee_FormatQuote32(printer,arg,(uint32_t *)str,WSTR_LENGTH(str),flags);
 }
}




#if 0
PRIVATE uint16_t *DCALL
utf8_to_utf16(uint8_t *__restrict str, size_t length) {
 uint16_t *result,*dst; size_t i;
 result = DeeString_NewBuffer16(length);
 if unlikely(!result) goto err;
 dst = result;
 i = 0;
 while (i < length) {
  uint8_t ch = str[i];
  uint8_t chlen = utf8_sequence_len[ch];
  uint32_t ch32;
  if ((size_t)chlen > length-i) {
   *dst++ = '?';
   ++i;
   continue;
  }
  switch (chlen) {
  case 0: /* Just to safely deal with this type of error... */
   *dst++ = '?';
   ++i;
   continue;
  case 1:
   *dst++ = ch;
   ++i;
   continue;
  case 2:
   ch32  = (ch & 0x1f) << 6;
   ch32 |= (str[i+1] & 0x3f);
   i += 2;
   break;
  case 3:
   if likely((str[i+1] & 0xc0) == 0x80 &&
             (str[i+1] & 0xc0) == 0x80) {
    ch32  = (ch & 0x0f) << 12;
    ch32 |= (str[i+1] & 0x3f) << 6;
    ch32 |= (str[i+2] & 0x3f);
   } else {
    ch32 = '?';
   }
   i += 3;
   break;
  case 4:
   if likely((str[i+1] & 0xc0) == 0x80 &&
             (str[i+2] & 0xc0) == 0x80 &&
             (str[i+3] & 0xc0) == 0x80) {
    ch32  = (ch & 0x07) << 18;
    ch32 |= (str[i+1] & 0x3f) << 12;
    ch32 |= (str[i+2] & 0x3f) << 6;
    ch32 |= (str[i+3] & 0x3f);
   } else {
    ch32 = '?';
   }
   i += 4;
   break;
  case 5:
   if likely((str[i+1] & 0xc0) == 0x80 &&
             (str[i+2] & 0xc0) == 0x80 &&
             (str[i+3] & 0xc0) == 0x80 &&
             (str[i+4] & 0xc0) == 0x80) {
    ch32  = (ch & 0x03) << 24;
    ch32 |= (str[i+1] & 0x3f) << 18;
    ch32 |= (str[i+2] & 0x3f) << 12;
    ch32 |= (str[i+3] & 0x3f) << 6;
    ch32 |= (str[i+4] & 0x3f);
   } else {
    ch32 = '?';
   }
   i += 5;
   break;
  case 6:
   if likely((str[i+1] & 0xc0) == 0x80 &&
             (str[i+2] & 0xc0) == 0x80 &&
             (str[i+3] & 0xc0) == 0x80 &&
             (str[i+4] & 0xc0) == 0x80 &&
             (str[i+5] & 0xc0) == 0x80) {
    ch32  = (ch & 0x01) << 30;
    ch32 |= (str[i+1] & 0x3f) << 24;
    ch32 |= (str[i+2] & 0x3f) << 18;
    ch32 |= (str[i+3] & 0x3f) << 12;
    ch32 |= (str[i+4] & 0x3f) << 6;
    ch32 |= (str[i+5] & 0x3f);
   } else {
    ch32 = '?';
   }
   i += 6;
   break;
  case 7:
   if likely((str[i+1] & 0xc0) == 0x80 &&
             (str[i+2] & 0xc0) == 0x80 &&
             (str[i+3] & 0xc0) == 0x80 &&
             (str[i+4] & 0xc0) == 0x80 &&
             (str[i+5] & 0xc0) == 0x80 &&
             (str[i+6] & 0xc0) == 0x80) {
    ch32  = (str[i+1] & 0x03/*0x3f*/) << 30;
    ch32 |= (str[i+2] & 0x3f) << 24;
    ch32 |= (str[i+3] & 0x3f) << 18;
    ch32 |= (str[i+4] & 0x3f) << 12;
    ch32 |= (str[i+5] & 0x3f) << 6;
    ch32 |= (str[i+6] & 0x3f);
   } else {
    ch32 = '?';
   }
   i += 7;
   break;
  case 8:
   if likely((str[i+1] & 0xc0) == 0x80 &&
             (str[i+2] & 0xc0) == 0x80 &&
             (str[i+3] & 0xc0) == 0x80 &&
             (str[i+4] & 0xc0) == 0x80 &&
             (str[i+5] & 0xc0) == 0x80 &&
             (str[i+6] & 0xc0) == 0x80 &&
             (str[i+7] & 0xc0) == 0x80) {
    /*ch32 = (str[i+1] & 0x3f) << 36;*/
    ch32  = (str[i+2] & 0x03/*0x3f*/) << 30;
    ch32 |= (str[i+3] & 0x3f) << 24;
    ch32 |= (str[i+4] & 0x3f) << 18;
    ch32 |= (str[i+5] & 0x3f) << 12;
    ch32 |= (str[i+6] & 0x3f) << 6;
    ch32 |= (str[i+7] & 0x3f);
   } else {
    ch32 = '?';
   }
   i += 8;
   break;
  default: __builtin_unreachable();
  }
  if ((ch32 >= UTF16_HIGH_SURROGATE_MIN && ch32 <= UTF16_HIGH_SURROGATE_MAX) ||
      (ch32 >= UTF16_LOW_SURROGATE_MIN && ch32 <= UTF16_LOW_SURROGATE_MAX)) {
   *dst++ = '?'; /* Invalid utf-16 character (surrogate) */
  } else if (ch32 <= 0xffff) {
   *dst++ = (uint16_t)ch32;
  } else if (ch32 > 0x10ffff) {
   *dst++ = '?'; /* Invalid utf-16 character (unicode character is too large) */
  } else {
   /* Character needs a low, and a high surrogate. */
   ch32 -= 0x10000;
   *dst++ = UTF16_HIGH_SURROGATE_MIN+(uint16_t)(ch32 >> 10);
   *dst++ = UTF16_LOW_SURROGATE_MIN+(uint16_t)(ch32 & 0x3ff);
  }
 }
 *dst = 0;
 i = (size_t)(dst-result);
 if (i != length) {
  dst = DeeString_TryResizeBuffer16(result,i);
  if likely(dst) result = dst;
 }
 return result;
err:
 return NULL;
}

PRIVATE uint32_t *DCALL
utf8_to_utf32(uint8_t *__restrict str, size_t length) {
 uint32_t *result,*dst; size_t i;
 result = DeeString_NewBuffer32(length);
 if unlikely(!result) goto err;
 dst = result;
 i = 0;
 while (i < length) {
  uint8_t ch = str[i];
  uint8_t chlen = utf8_sequence_len[ch];
  uint32_t ch32;
  if ((size_t)chlen > length-i) {
   *dst++ = '?';
   ++i;
   continue;
  }
  switch (chlen) {
  case 0: /* Just to safely deal with this type of error... */
   *dst++ = '?';
   ++i;
   continue;
  case 1:
   *dst++ = ch;
   ++i;
   break;
  case 2:
   ch32  = (ch & 0x1f) << 6;
   ch32 |= (str[i+1] & 0x3f);
   *dst++ = ch32;
   i += 2;
   break;
  case 3:
   if likely((str[i+1] & 0xc0) == 0x80 &&
             (str[i+1] & 0xc0) == 0x80) {
    ch32  = (ch & 0x0f) << 12;
    ch32 |= (str[i+1] & 0x3f) << 6;
    ch32 |= (str[i+2] & 0x3f);
    *dst++ = ch32;
   } else {
    *dst++ = '?';
   }
   i += 3;
   break;
  case 4:
   if likely((str[i+1] & 0xc0) == 0x80 &&
             (str[i+2] & 0xc0) == 0x80 &&
             (str[i+3] & 0xc0) == 0x80) {
    ch32  = (ch & 0x07) << 18;
    ch32 |= (str[i+1] & 0x3f) << 12;
    ch32 |= (str[i+2] & 0x3f) << 6;
    ch32 |= (str[i+3] & 0x3f);
    *dst++ = ch32;
   } else {
    *dst++ = '?';
   }
   i += 4;
   break;
  case 5:
   if likely((str[i+1] & 0xc0) == 0x80 &&
             (str[i+2] & 0xc0) == 0x80 &&
             (str[i+3] & 0xc0) == 0x80 &&
             (str[i+4] & 0xc0) == 0x80) {
    ch32  = (ch & 0x03) << 24;
    ch32 |= (str[i+1] & 0x3f) << 18;
    ch32 |= (str[i+2] & 0x3f) << 12;
    ch32 |= (str[i+3] & 0x3f) << 6;
    ch32 |= (str[i+4] & 0x3f);
    *dst++ = ch32;
   } else {
    *dst++ = '?';
   }
   i += 5;
   break;
  case 6:
   if likely((str[i+1] & 0xc0) == 0x80 &&
             (str[i+2] & 0xc0) == 0x80 &&
             (str[i+3] & 0xc0) == 0x80 &&
             (str[i+4] & 0xc0) == 0x80 &&
             (str[i+5] & 0xc0) == 0x80) {
    ch32  = (ch & 0x01) << 30;
    ch32 |= (str[i+1] & 0x3f) << 24;
    ch32 |= (str[i+2] & 0x3f) << 18;
    ch32 |= (str[i+3] & 0x3f) << 12;
    ch32 |= (str[i+4] & 0x3f) << 6;
    ch32 |= (str[i+5] & 0x3f);
    *dst++ = ch32;
   } else {
    *dst++ = '?';
   }
   i += 6;
   break;
  case 7:
   if likely((str[i+1] & 0xc0) == 0x80 &&
             (str[i+2] & 0xc0) == 0x80 &&
             (str[i+3] & 0xc0) == 0x80 &&
             (str[i+4] & 0xc0) == 0x80 &&
             (str[i+5] & 0xc0) == 0x80 &&
             (str[i+6] & 0xc0) == 0x80) {
    ch32  = (str[i+1] & 0x03/*0x3f*/) << 30;
    ch32 |= (str[i+2] & 0x3f) << 24;
    ch32 |= (str[i+3] & 0x3f) << 18;
    ch32 |= (str[i+4] & 0x3f) << 12;
    ch32 |= (str[i+5] & 0x3f) << 6;
    ch32 |= (str[i+6] & 0x3f);
    *dst++ = ch32;
   } else {
    *dst++ = '?';
   }
   i += 7;
   break;
  case 8:
   if likely((str[i+1] & 0xc0) == 0x80 &&
             (str[i+2] & 0xc0) == 0x80 &&
             (str[i+3] & 0xc0) == 0x80 &&
             (str[i+4] & 0xc0) == 0x80 &&
             (str[i+5] & 0xc0) == 0x80 &&
             (str[i+6] & 0xc0) == 0x80 &&
             (str[i+7] & 0xc0) == 0x80) {
    /*ch32 = (str[i+1] & 0x3f) << 36;*/
    ch32  = (str[i+2] & 0x03/*0x3f*/) << 30;
    ch32 |= (str[i+3] & 0x3f) << 24;
    ch32 |= (str[i+4] & 0x3f) << 18;
    ch32 |= (str[i+5] & 0x3f) << 12;
    ch32 |= (str[i+6] & 0x3f) << 6;
    ch32 |= (str[i+7] & 0x3f);
    *dst++ = ch32;
   } else {
    *dst++ = '?';
   }
   i += 8;
   break;
  default: __builtin_unreachable();
  }
 }
 *dst = 0;
 i = (size_t)(dst-result);
 if (i != length) {
  dst = DeeString_TryResizeBuffer32(result,i);
  if likely(dst) result = dst;
 }
 return result;
err:
 return NULL;
}
#endif

#if defined(CONFIG_HOST_WINDOWS) && 0
#define mbcs_to_wide(str,length) nt_MultiByteToWideChar(CP_ACP,str,length)
PRIVATE dwchar_t *DCALL
nt_MultiByteToWideChar(DWORD codepage, uint8_t *__restrict str, size_t length) {
 dwchar_t *result,*new_result; size_t result_length;
 assert(length != 0);
 result = DeeString_NewWideBuffer(length);
 result_length = (size_t)(DWORD)MultiByteToWideChar(codepage,
                                                    0,
                                                   (LPCCH)str,
                                                   (int)(DWORD)length,
                                                   (LPWSTR)result,
                                                   (int)(DWORD)length);
 if unlikely(!result_length || result_length > length) {
  size_t new_length;
  result_length = (size_t)(DWORD)MultiByteToWideChar(codepage,
                                                     0,
                                                    (LPCCH)str,
                                                    (int)(DWORD)length,
                                                     NULL,
                                                     0);
  if unlikely(!result_length) {
   size_t i;
fallback:
   /* Fallback: Simply up-cast the string. */
   for (i = 0; i < length; ++i)
       result[i] = (dwchar_t)str[i];
   result[length] = 0;
   return result;
  }
  new_length = result_length;
  new_result = DeeString_ResizeWideBuffer(result,new_length);
  if unlikely(!new_result) goto err_r;
  result = new_result;
  result_length = (size_t)(DWORD)MultiByteToWideChar(codepage,
                                                     0,
                                                    (LPCCH)str,
                                                    (int)(DWORD)length,
                                                    (LPWSTR)result,
                                                    (int)(DWORD)new_length);
  if unlikely(!result_length || result_length > new_length) {
   /* ... What?!? */
   new_result = DeeString_ResizeWideBuffer(result,length);
   if unlikely(!new_result) goto err_r;
   result = new_result;
   goto fallback;
  }
  length = new_length; /* For the comparison below... */
 }
 result[result_length] = 0;
 if unlikely(result_length != length) {
  new_result = DeeString_TryResizeWideBuffer(result,result_length);
  if likely(new_result) result = new_result;
 }
 return result;
err_r:
 DeeString_FreeWideBuffer(result);
 return NULL;
}
#endif





LOCAL void DCALL
char16_to_utf8(uint16_t *__restrict src, size_t src_len,
               uint8_t *__restrict dst) {
 size_t i;
 for (i = 0; i < src_len; ++i) {
  uint16_t ch = src[i];
  if (ch <= UTF8_1BYTE_MAX) {
   *dst++ = (uint8_t)ch;
  } else if (ch <= UTF8_2BYTE_MAX) {
   *dst++ = 0xc0 | (uint8_t)((ch >> 6)/* & 0x1f*/);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  } else {
   *dst++ = 0xe0 | (uint8_t)((ch >> 12)/* & 0x0f*/);
   *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  }
 }
}

LOCAL void DCALL
char16_to_utf8_fast(uint16_t *__restrict src, size_t src_len,
                    uint8_t *__restrict dst) {
 size_t i;
 for (i = 0; i < src_len; ++i) {
  uint16_t ch = src[i];
  ASSERT(ch <= UTF8_1BYTE_MAX);
  *dst++ = (uint8_t)ch;
 }
}

LOCAL void DCALL
utf16_to_utf8(uint16_t *__restrict src, size_t src_len,
              uint8_t *__restrict dst) {
 size_t i;
 for (i = 0; i < src_len; ++i) {
  uint32_t ch = src[i];
  if (ch >= UTF16_HIGH_SURROGATE_MIN &&
      ch <= UTF16_HIGH_SURROGATE_MAX) {
   ASSERT(i < src_len-1);
   ch = (ch - UTF16_HIGH_SURROGATE_MIN) << 10;
   ++i;
   ASSERT(src[i] >= UTF16_LOW_SURROGATE_MIN);
   ASSERT(src[i] <= UTF16_LOW_SURROGATE_MAX);
   ch |= src[i] - UTF16_LOW_SURROGATE_MIN;
   ch += UTF16_SURROGATE_SHIFT;
  }
  ASSERT(ch <= 0x10FFFF);
  if (ch <= UTF8_1BYTE_MAX) {
   *dst++ = (uint8_t)ch;
  } else if (ch <= UTF8_2BYTE_MAX) {
   *dst++ = 0xc0 | (uint8_t)((ch >> 6)/* & 0x1f*/);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  } else if (ch <= UTF8_3BYTE_MAX) {
   *dst++ = 0xe0 | (uint8_t)((ch >> 12)/* & 0x0f*/);
   *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  } else {
   *dst++ = 0xf0 | (uint8_t)((ch >> 18)/* & 0x07*/);
   *dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  }
 }
}

LOCAL void DCALL
utf16_to_utf8_char16(uint16_t *__restrict src, size_t src_len,
                     uint8_t *__restrict dst,
                     uint16_t *__restrict dst_c16) {
 size_t i;
 for (i = 0; i < src_len; ++i) {
  uint32_t ch = src[i];
  if (ch >= UTF16_HIGH_SURROGATE_MIN &&
      ch <= UTF16_HIGH_SURROGATE_MAX) {
   ASSERT(i < src_len-1);
   ch = (ch - UTF16_HIGH_SURROGATE_MIN) << 10;
   ++i;
   ASSERT(src[i] >= UTF16_LOW_SURROGATE_MIN);
   ASSERT(src[i] <= UTF16_LOW_SURROGATE_MAX);
   ch |= src[i] - UTF16_LOW_SURROGATE_MIN;
   ch += UTF16_SURROGATE_SHIFT;
  }
  ASSERT(ch <= 0xFFFF);
  *dst_c16++ = (uint16_t)ch;
  if (ch <= UTF8_1BYTE_MAX) {
   *dst++ = (uint8_t)ch;
  } else if (ch <= UTF8_2BYTE_MAX) {
   *dst++ = 0xc0 | (uint8_t)((ch >> 6)/* & 0x1f*/);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  } else {
   *dst++ = 0xe0 | (uint8_t)((ch >> 12)/* & 0x0f*/);
   *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  }
 }
}

LOCAL void DCALL
utf16_to_utf8_char32(uint16_t *__restrict src, size_t src_len,
                     uint8_t *__restrict dst,
                     uint32_t *__restrict dst_c32) {
 size_t i;
 for (i = 0; i < src_len; ++i) {
  uint32_t ch = src[i];
  if (ch >= UTF16_HIGH_SURROGATE_MIN &&
      ch <= UTF16_HIGH_SURROGATE_MAX) {
   ASSERT(i < src_len-1);
   ch = (ch - UTF16_HIGH_SURROGATE_MIN) << 10;
   ++i;
   ASSERT(src[i] >= UTF16_LOW_SURROGATE_MIN);
   ASSERT(src[i] <= UTF16_LOW_SURROGATE_MAX);
   ch |= src[i] - UTF16_LOW_SURROGATE_MIN;
   ch += UTF16_SURROGATE_SHIFT;
  }
  ASSERT(ch <= 0x10FFFF);
  *dst_c32++ = ch;
  if (ch <= UTF8_1BYTE_MAX) {
   *dst++ = (uint8_t)ch;
  } else if (ch <= UTF8_2BYTE_MAX) {
   *dst++ = 0xc0 | (uint8_t)((ch >> 6)/* & 0x1f*/);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  } else if (ch <= UTF8_3BYTE_MAX) {
   *dst++ = 0xe0 | (uint8_t)((ch >> 12)/* & 0x0f*/);
   *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  } else {
   *dst++ = 0xf0 | (uint8_t)((ch >> 18)/* & 0x07*/);
   *dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  }
 }
}

LOCAL void DCALL
utf32_to_utf8(uint32_t *__restrict src, size_t src_len,
              uint8_t *__restrict dst) {
 size_t i;
 for (i = 0; i < src_len; ++i) {
  uint32_t ch = src[i];
  if (ch <= UTF8_1BYTE_MAX) {
   *dst++ = (uint8_t)ch;
  } else if (ch <= UTF8_2BYTE_MAX) {
   *dst++ = 0xc0 | (uint8_t)((ch >> 6)/* & 0x1f*/);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  } else if (ch <= UTF8_3BYTE_MAX) {
   *dst++ = 0xe0 | (uint8_t)((ch >> 12)/* & 0x0f*/);
   *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  } else if (ch <= UTF8_4BYTE_MAX) {
   *dst++ = 0xf0 | (uint8_t)((ch >> 18)/* & 0x07*/);
   *dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  } else if (ch <= UTF8_5BYTE_MAX) {
   *dst++ = 0xf8 | (uint8_t)((ch >> 24)/* & 0x03*/);
   *dst++ = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  } else if (ch <= UTF8_6BYTE_MAX) {
   *dst++ = 0xfc | (uint8_t)((ch >> 30)/* & 0x01*/);
   *dst++ = 0x80 | (uint8_t)((ch >> 24) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  } else {
   *dst++ = 0xfe;
   *dst++ = 0x80 | (uint8_t)((ch >> 30) & 0x03/* & 0x3f*/);
   *dst++ = 0x80 | (uint8_t)((ch >> 24) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
   *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
  }
 }
}


PUBLIC DREF DeeObject *DCALL
DeeString_Pack2ByteBuffer(/*inherit(always)*/uint16_t *__restrict text) {
 size_t i,length,utf8_length;
 DREF String *result; struct string_utf *utf;
 ASSERT(text);
 length = ((size_t *)text)[-1];
 if unlikely(!length) {
  Dee_Free((size_t *)text-1);
  return_empty_string;
 }
 text[length] = 0;
 utf8_length = 0;
 for (i = 0; i < length; ++i) {
  uint16_t ch = text[i];
  if (ch <= UTF8_1BYTE_MAX) {
   utf8_length += 1;
  } else if (ch <= UTF8_2BYTE_MAX) {
   utf8_length += 2;
  } else {
   utf8_length += 3;
  }
 }
 ASSERT(utf8_length >= length);
 result = (DREF String *)DeeObject_Malloc(COMPILER_OFFSETOF(String,s_str)+
                                         (utf8_length+1)*sizeof(char));
 if unlikely(!result) goto err;
 result->s_len = utf8_length;
 utf = utf_alloc();
 if unlikely(!utf) goto err_r;
 memset(utf,0,sizeof(struct string_utf));
 if (utf8_length == length) {
  utf->u_width = STRING_WIDTH_1BYTE;
  utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
 } else {
  utf->u_width = STRING_WIDTH_2BYTE;
 }
 char16_to_utf8(text,length,(uint8_t *)result->s_str);
 utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)text; /* Inherit data */
 result->s_hash = DEE_STRING_HASH_UNSET;
 result->s_data = utf;
 Dee_UntrackAlloc(utf);
 result->s_str[utf8_length] = '\0';
 DeeObject_Init(result,&DeeString_Type);
 Dee_UntrackAlloc((size_t *)text - 1);
 return (DREF DeeObject *)result;
err_r:
 DeeObject_Free(result);
err:
 Dee_Free((size_t *)text-1);
 return NULL;
}

PUBLIC DREF DeeObject *DCALL
DeeString_TryPack2ByteBuffer(/*inherit(on_success)*/uint16_t *__restrict text) {
 size_t i,length,utf8_length;
 DREF String *result; struct string_utf *utf;
 ASSERT(text);
 length = ((size_t *)text)[-1];
 if unlikely(!length) {
  Dee_Free((size_t *)text-1);
  return_empty_string;
 }
 text[length] = 0;
 utf8_length = 0;
 for (i = 0; i < length; ++i) {
  uint16_t ch = text[i];
  if (ch <= UTF8_1BYTE_MAX) {
   utf8_length += 1;
  } else if (ch <= UTF8_2BYTE_MAX) {
   utf8_length += 2;
  } else {
   utf8_length += 3;
  }
 }
 ASSERT(utf8_length >= length);
 result = (DREF String *)DeeObject_TryMalloc(COMPILER_OFFSETOF(String,s_str)+
                                            (utf8_length+1)*sizeof(char));
 if unlikely(!result) goto err;
 result->s_len = utf8_length;
 utf = utf_alloc();
 if unlikely(!utf) goto err_r;
 memset(utf,0,sizeof(struct string_utf));
 if (utf8_length == length) {
  utf->u_width = STRING_WIDTH_1BYTE;
  utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
  char16_to_utf8_fast(text,length,(uint8_t *)result->s_str);
 } else {
  utf->u_width = STRING_WIDTH_2BYTE;
  char16_to_utf8(text,length,(uint8_t *)result->s_str);
 }
 utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)text; /* Inherit data */
 result->s_hash = DEE_STRING_HASH_UNSET;
 result->s_data = utf;
 Dee_UntrackAlloc(utf);
 result->s_str[utf8_length] = '\0';
 DeeObject_Init(result,&DeeString_Type);
 Dee_UntrackAlloc((size_t *)text - 1);
 return (DREF DeeObject *)result;
err_r:
 DeeObject_Free(result);
err:
 return NULL;
}


PUBLIC DREF DeeObject *DCALL
DeeString_PackUtf16Buffer(/*inherit(always)*/uint16_t *__restrict text,
                          unsigned int error_mode) {
 size_t i,length,utf8_length; struct string_utf *utf;
 DREF String *result; size_t character_count;
 int kind = 0; /* 0: UTF-16 w/o surrogates
                * 1: UTF-16 w surrogates
                * 2: UTF-16 w surrogates that produce character > 0xffff */
 ASSERT(text);
 length = ((size_t *)text)[-1];
 if unlikely(!length) {
  Dee_Free((size_t *)text-1);
  return_empty_string;
 }
 text[length] = 0;
 utf8_length = 0;
 i = 0;
 character_count = 0;
continue_at_i:
 for (; i < length; ++i) {
  uint32_t ch;
read_text_i:
  ch = text[i];
  if (ch >= UTF16_HIGH_SURROGATE_MIN &&
      ch <= UTF16_HIGH_SURROGATE_MAX) {
   uint16_t low_value;
   /* Surrogate pair. */
   if unlikely(i >= length-1) {
    /* Missing high surrogate. */
    if (error_mode & STRING_ERROR_FREPLAC) {
     text[i] = '?';
     break;
    }
    if (error_mode & STRING_ERROR_FIGNORE) {
     text[i] = 0;
     --length;
     break;
    }
    DeeError_Throwf(&DeeError_UnicodeDecodeError,
                    "Missing low surrogate for high surrogate U+%I32X",
                    ch);
    goto err;
   }
   ch = (ch - UTF16_HIGH_SURROGATE_MIN) << 10;
   ++i;
   low_value = text[i];
   if unlikely(low_value < UTF16_LOW_SURROGATE_MIN ||
               low_value > UTF16_LOW_SURROGATE_MAX) {
    /* Invalid low surrogate. */
    if (error_mode & STRING_ERROR_FREPLAC) {
     text[i-1] = '?';
     --length;
     memmove(&text[i],&text[i+1],(length-i)*sizeof(uint16_t));
     goto read_text_i;
    }
    if (error_mode & STRING_ERROR_FIGNORE) {
     --i;
     length -= 2;
     ASSERT(length >= i);
     memmove(&text[i],&text[i+2],(length-i)*sizeof(uint16_t));
     goto continue_at_i;
    }
    DeeError_Throwf(&DeeError_UnicodeDecodeError,
                    "Invalid low surrogate U+%I16X paired with high surrogate U+%I32X",
                    low_value,ch);
    goto err;
   }
   ch |= low_value - UTF16_LOW_SURROGATE_MIN;
   ch += UTF16_SURROGATE_SHIFT;
   if (!kind) kind = 1;
  }
  ++character_count;
  ASSERT(ch <= 0x10FFFF);
  if (ch <= UTF8_1BYTE_MAX) {
   utf8_length += 1;
  } else if (ch <= UTF8_2BYTE_MAX) {
   utf8_length += 2;
  } else if (ch <= UTF8_3BYTE_MAX) {
   utf8_length += 3;
  } else {
   kind = 2;
   utf8_length += 4;
  }
 }
 ASSERT(utf8_length >= length);
 result = (DREF String *)DeeObject_Malloc(COMPILER_OFFSETOF(String,s_str)+
                                         (utf8_length+1)*sizeof(char));
 if unlikely(!result) goto err;
 result->s_len = utf8_length;
 utf = utf_alloc();
 if unlikely(!utf) goto err_r;
 memset(utf,0,sizeof(struct string_utf));
 *(uint16_t **)&utf->u_utf16 = (uint16_t *)text; /* Inherit data */
 if (utf8_length == length) {
  size_t i;
  /* Pure UTF-16 in ASCII range. */
  utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
  utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)text;
  Dee_UntrackAlloc((size_t *)text - 1);
  utf->u_width                    = STRING_WIDTH_1BYTE;
  ASSERT(character_count == utf8_length);
  ASSERT(character_count == length);
  ASSERT(character_count == WSTR_LENGTH(text));
  for (i = 0; i < length; ++i)
      result->s_str[i] = (char)(uint8_t)text[i];
 } else {
  switch (kind) {
  case 0: /* Pure UTF-16 (without surrogates) */
   utf->u_width = STRING_WIDTH_2BYTE;
   utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)text;
   ASSERT(character_count == WSTR_LENGTH(text));
   char16_to_utf8(text,length,(uint8_t *)result->s_str);
   break;
  {
   uint16_t *buffer;
  case 1:
   /* Regular UTF-16 in 2-byte range (with surrogates)
    * -> Must convert the UTF-16 text into a 2-byte string. */
   buffer = DeeString_NewBuffer16(character_count);
   if unlikely(!buffer) goto err_r_utf;
   buffer[character_count] = 0;
   utf->u_width = STRING_WIDTH_2BYTE;
   utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)buffer;
   Dee_UntrackAlloc((size_t *)buffer - 1);
   ASSERT(character_count < WSTR_LENGTH(text));
   utf16_to_utf8_char16(text,length,(uint8_t *)result->s_str,buffer);
  } break;
  {
   uint32_t *buffer;
  case 2:
   /* Regular UTF-16 outside the 2-byte range (with surrogates)
    * -> Must convert the UTF-16 text into a 4-byte string. */
   buffer = DeeString_NewBuffer32(character_count);
   if unlikely(!buffer) goto err_r_utf;
   buffer[character_count] = 0;
   utf->u_width = STRING_WIDTH_4BYTE;
   utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)buffer;
   Dee_UntrackAlloc((size_t *)buffer - 1);
   ASSERT(character_count < WSTR_LENGTH(text));
   utf16_to_utf8_char32(text,length,(uint8_t *)result->s_str,buffer);
  } break;
  default: __builtin_unreachable();
  }
 }
 result->s_data = utf;
 Dee_UntrackAlloc(utf);
 result->s_hash = DEE_STRING_HASH_UNSET;
 result->s_str[utf8_length] = '\0';
 DeeObject_Init(result,&DeeString_Type);
 Dee_UntrackAlloc(utf);
 return (DREF DeeObject *)result;
err_r_utf:
 utf_free(utf);
err_r:
 DeeObject_Free(result);
err:
 Dee_Free((size_t *)text-1);
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeString_PackUtf32Buffer(/*inherit(always)*/uint32_t *__restrict text,
                          unsigned int error_mode) {
 size_t i,length,utf8_length;
 DREF String *result; struct string_utf *utf;
 ASSERT(text);
 length = ((size_t *)text)[-1];
 if unlikely(!length) {
  Dee_Free((size_t *)text-1);
  return_empty_string;
 }
 text[length] = 0;
 utf8_length = 0;
 for (i = 0; i < length; ++i) {
  uint32_t ch = text[i];
  /* */if (ch <= UTF8_1BYTE_MAX) utf8_length += 1;
  else if (ch <= UTF8_2BYTE_MAX) utf8_length += 2;
  else if (ch <= UTF8_3BYTE_MAX) utf8_length += 3;
  else if (ch <= UTF8_4BYTE_MAX) utf8_length += 4;
  else {
   /* Not actually a valid unicode character, but could be encoded! */
   if (error_mode & STRING_ERROR_FREPLAC) {
    /* Replace with a question mark. */
    text[i] = '?';
    utf8_length += 1;
   } else if (error_mode & STRING_ERROR_FIGNORE) {
    /* Just encode it... */
    /* */if (ch <= UTF8_5BYTE_MAX) utf8_length += 5;
    else if (ch <= UTF8_6BYTE_MAX) utf8_length += 6;
    else utf8_length += 7;
   } else {
    DeeError_Throwf(&DeeError_UnicodeDecodeError,
                    "Invalid unicode character U+%I32X",
                    ch);
    goto err;
   }
  }
 }
 ASSERT(utf8_length >= length);
 result = (DREF String *)DeeObject_Malloc(COMPILER_OFFSETOF(String,s_str)+
                                         (utf8_length+1)*sizeof(char));
 if unlikely(!result) goto err;
 result->s_len = utf8_length;
 utf = utf_alloc();
 if unlikely(!utf) goto err_r;
 memset(utf,0,sizeof(struct string_utf));
 utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)text; /* Inherit data */
 utf32_to_utf8(text,length,(uint8_t *)result->s_str);
 if (utf8_length == length) {
  /* Pure UTF-32 in ASCII range. */
  utf->u_width = STRING_WIDTH_1BYTE;
  utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
 } else {
  utf->u_width = STRING_WIDTH_4BYTE;
 }
 result->s_data = utf;
 Dee_UntrackAlloc(utf);
 result->s_hash = DEE_STRING_HASH_UNSET;
 result->s_str[utf8_length] = '\0';
 DeeObject_Init(result,&DeeString_Type);
 Dee_UntrackAlloc((size_t *)text - 1);
 Dee_UntrackAlloc(utf);
 return (DREF DeeObject *)result;
err_r:
 DeeObject_Free(result);
err:
 Dee_Free((size_t *)text-1);
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeString_TryPackUtf32Buffer(/*inherit(on_success)*/uint32_t *__restrict text) {
 size_t i,length,utf8_length;
 DREF String *result; struct string_utf *utf;
 ASSERT(text);
 length = ((size_t *)text)[-1];
 if unlikely(!length) {
  Dee_Free((size_t *)text-1);
  return_empty_string;
 }
 text[length] = 0;
 utf8_length = 0;
 for (i = 0; i < length; ++i) {
  uint32_t ch = text[i];
  /* */if (ch <= UTF8_1BYTE_MAX) utf8_length += 1;
  else if (ch <= UTF8_2BYTE_MAX) utf8_length += 2;
  else if (ch <= UTF8_3BYTE_MAX) utf8_length += 3;
  else if (ch <= UTF8_4BYTE_MAX) utf8_length += 4;
  else if (ch <= UTF8_5BYTE_MAX) utf8_length += 5;
  else if (ch <= UTF8_6BYTE_MAX) utf8_length += 6;
  else utf8_length += 7;
 }
 ASSERT(utf8_length >= length);
 result = (DREF String *)DeeObject_TryMalloc(COMPILER_OFFSETOF(String,s_str)+
                                            (utf8_length+1)*sizeof(char));
 if unlikely(!result) goto err;
 result->s_len = utf8_length;
 utf = utf_alloc();
 if unlikely(!utf) goto err_r;
 memset(utf,0,sizeof(struct string_utf));
 utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)text; /* Inherit data */
 utf32_to_utf8(text,length,(uint8_t *)result->s_str);
 if (utf8_length == length) {
  /* Pure UTF-32 in ASCII range. */
  utf->u_width = STRING_WIDTH_1BYTE;
  utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
 } else {
  utf->u_width = STRING_WIDTH_4BYTE;
 }
 result->s_data = utf;
 Dee_UntrackAlloc(utf);
 result->s_hash = DEE_STRING_HASH_UNSET;
 result->s_str[utf8_length] = '\0';
 DeeObject_Init(result,&DeeString_Type);
 Dee_UntrackAlloc((size_t *)text - 1);
 Dee_UntrackAlloc(utf);
 return (DREF DeeObject *)result;
err_r:
 DeeObject_Free(result);
err:
 return NULL;
}

#if 0 /* TODO: Expose as a codepage decoder function */
PUBLIC DREF DeeObject *DCALL
DeeString_PackWideBuffer(/*inherit(always)*/dwchar_t *__restrict text,
                         unsigned int error_mode) {
#ifdef CONFIG_HOST_WINDOWS
 size_t length,mbcs_length; DWORD flags;
 DREF String *result,*new_result;
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS 0x00000080
#endif
 static DWORD wincall_flags = WC_ERR_INVALID_CHARS;
 ASSERT(text);
 length = ((size_t *)text)[-1];
 if unlikely(!length) {
  Dee_Free((size_t *)text-1);
  return_empty_string;
 }
 text[length] = 0;
 /* Convert to a MBSC string. */
 result = (DREF String *)DeeObject_Malloc(COMPILER_OFFSETOF(String,s_str)+
                                         (length+1)*sizeof(char));
 if unlikely(!result) goto err;
restart:
 flags = 0;
 if (!(error_mode&(STRING_ERROR_FIGNORE|
                   STRING_ERROR_FREPLAC)))
       flags |= wincall_flags;
 mbcs_length = (size_t)(DWORD)WideCharToMultiByte(CP_UTF8,
                                                  flags,
                                                  text,
                                                 (int)(DWORD)length,
                                                  result->s_str,
                                                 (int)(DWORD)length,
                                                  NULL,
                                                  NULL);
 if (!mbcs_length) {
  /* Probably just insufficient buffer memory... */
  mbcs_length = (size_t)(DWORD)WideCharToMultiByte(CP_UTF8,
                                                   flags,
                                                   text,
                                                  (int)(DWORD)length,
                                                   NULL,
                                                   0,
                                                   NULL,
                                                   NULL);
  if unlikely(!mbcs_length) {
   DWORD error;
handle_decode_error:
   error = GetLastError();
   if (error == ERROR_INVALID_FLAGS &&
      (flags & WC_ERR_INVALID_CHARS)) {
    wincall_flags = 0;
    goto restart;
   }
   if (error_mode & (STRING_ERROR_FIGNORE|STRING_ERROR_FREPLAC)) {
    size_t i;
    mbcs_length = length;
    for (i = 0; i < length; ++i)
        result->s_str[i] = (char)((uint16_t)text[i] & 0x7f);
    goto done;
   }
   DeeError_Throwf(&DeeError_UnicodeDecodeError,
                   "Invalid unicode sequence");
   goto err_r;
  }
  new_result = (DREF String *)DeeObject_Realloc(result,COMPILER_OFFSETOF(String,s_str)+
                                               (mbcs_length+1)*sizeof(char));
  if unlikely(!new_result) goto err;
  result = new_result;
  mbcs_length = (size_t)(DWORD)WideCharToMultiByte(CP_UTF8,
                                                   flags,
                                                   text,
                                                  (int)(DWORD)length,
                                                   result->s_str,
                                                  (int)(DWORD)mbcs_length,
                                                   NULL,
                                                   NULL);
  if unlikely(!mbcs_length) goto handle_decode_error;
 }
 if unlikely(mbcs_length < length) {
  new_result = (DREF String *)DeeObject_TryRealloc(result,
                                                   COMPILER_OFFSETOF(String,s_str)+
                                                  (length+1)*sizeof(char));
  if likely(new_result) result = new_result;
 }
done:
 /* Fill in the unicode data of the resulting object. */
 result->s_data = utf_alloc();
 if unlikely(!result->s_data) goto err_r;
 memset(result->s_data,0,sizeof(struct string_utf));
 if (mbcs_length <= length)
      result->s_data->u_width = STRING_WIDTH_1BYTE;
 else result->s_data->u_width = STRING_WIDTH_2BYTE;
 result->s_data->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
 result->s_data->u_flags                    = STRING_UTF_FNORMAL;
 result->s_data->u_data[STRING_WIDTH_2BYTE] = (size_t *)text; /* Inherit data. */
 result->s_len                              = mbcs_length;
 result->s_hash                             = DEE_STRING_HASH_UNSET;
 result->s_str[mbcs_length]                 = '\0';
 DeeObject_Init(result,&DeeString_Type);
 Dee_UntrackAlloc((size_t *)text - 1);
 Dee_UntrackAlloc(result->s_data);
 return (DREF DeeObject *)result;
err_r:
 DeeObject_Free(result);
err:
 Dee_Free((size_t *)text-1);
 return NULL;
#else
#error TODO
#endif
}
#endif



#if 0
 {
  size_t i,length; uint8_t *str;
 case STRING_ENCODING_---ASCII:
  /* Assert that there are no characters > 127 */
  str    = (uint8_t *)DeeString_STR(self);
  length = DeeString_SIZE(self);
  i = 0;
continue_ascii_fast:
  for (; i < length; ++i) {
   if (str[i] <= 0x7f) continue;
   if (encoding & STRING_ERROR_FIGNORE) {
    --DeeString_SIZE(self);
    --length;
    memmove(&str[i],&str[i+1],(length-i)*sizeof(char));
    str[length] = '\0';
    goto continue_ascii_fast;
   }
   if (encoding & STRING_ERROR_FREPLAC) {
    str[i] = '?';
    continue;
   }
   DeeError_Throwf(&DeeError_UnicodeEncodeError,
                   "Non-ascii character character U+%.4X",
                  (unsigned int)str[i]);
   goto err;
  }
  return self;
 }
#endif

#if 0
 {
  struct string_utf *data;
 case STRING_ENCODING_---MBCS:
  if (DeeString_IsEmpty(self)) return self;
  ASSERT(!DeeObject_IsShared(self));
  ASSERT(!((String *)self)->s_data);
  data = utf_alloc();
  if unlikely(!data) goto err;
  memset(data,0,sizeof(struct string_utf));
#if __SIZEOF_WCHAR_T__ == 2
  data->u_width = STRING_WIDTH_2BYTE;
  data->u_data[STRING_WIDTH_2BYTE] = (size_t *)mbcs_to_wide((uint8_t *)DeeString_STR(self),
                                                             DeeString_SIZE(self));
  if unlikely(!data->u_data[STRING_WIDTH_2BYTE]) { utf_free(data); goto err; }
#else
  data->u_width = STRING_WIDTH_4BYTE;
  data->u_data[STRING_WIDTH_4BYTE] = (size_t *)mbcs_to_wide((uint8_t *)DeeString_STR(self),
                                                             DeeString_SIZE(self));
  if unlikely(!data->u_data[STRING_WIDTH_4BYTE]) { utf_free(data); goto err; }
#endif
  ((String *)self)->s_data = data;
  Dee_UntrackAlloc(data);
  return self;
 }

#endif

INTDEF uint32_t DCALL
utf8_getchar(uint8_t const *__restrict base, uint8_t seqlen);

PUBLIC DREF DeeObject *DCALL
DeeString_NewUtf8(char const *__restrict str, size_t length,
                  unsigned int error_mode) {
 DREF String *result; uint8_t *iter,*end;
 struct string_utf *utf = NULL;
 uint32_t *buffer32,*dst32;
 size_t i,simple_length,utf_length;
 result = (DREF String *)DeeObject_Malloc(COMPILER_OFFSETOF(String,s_str)+
                                         (length + 1)*sizeof(char));
 if unlikely(!result) goto err;
 memcpy(result->s_str,str,length*sizeof(char));
 /* Search for multi-byte character sequences. */
 end = (iter = (uint8_t *)result->s_str)+length;
 while (iter < end) {
  uint8_t seqlen,ch = *iter;
  uint32_t ch32;
  if (ch <= 0x7f) { ++iter; continue; }
  seqlen = utf8_sequence_len[ch];
  if unlikely(!seqlen || iter + seqlen > end) {
   /* Invalid UTF-8 character */
   if (error_mode == STRING_ERROR_FREPLAC) {
    *iter = '?';
    ++iter;
    continue;
   }
   if (error_mode == STRING_ERROR_FIGNORE) {
    --end;
    memmove(iter,iter + 1,(end - iter)*sizeof(char));
    continue;
   }
   DeeError_Throwf(&DeeError_UnicodeDecodeError,
                   "Invalid utf-8 character byte 0x%.2I8x",
                   ch);
   goto err_r;
  }
  ch32 = utf8_getchar(iter,seqlen);
  if unlikely(ch32 <= 0x7f) {
   *iter = (uint8_t)ch32;
   memmove(iter + 1,
           iter + seqlen,
          (end - (iter + seqlen))*
           sizeof(char));
   ++iter;
   continue;
  }
  if (ch32 > 0xffff) {
   /* Must decode into a 4-byte string */
   buffer32 = DeeString_NewBuffer32(length - (seqlen - 1));
   if unlikely(!buffer32) goto err_r;
   dst32 = buffer32;
   simple_length = iter - (uint8_t *)result->s_str;
   for (i = 0; i < simple_length; ++i)
       *dst32++ = (uint32_t)(uint8_t)result->s_str[i];
   *dst32++ = ch32;
   iter += seqlen;
use_buffer32:
   while (iter < end) {
    ch = *iter;
    if (ch <= 0x7f) {
     ++iter;
     *dst32++ = ch;
     continue;
    }
    seqlen = utf8_sequence_len[ch];
    if unlikely(!seqlen || iter + seqlen > end) {
     /* Invalid UTF-8 character */
     if (error_mode == STRING_ERROR_FREPLAC) {
      *iter++ = '?';
      *dst32++  = '?';
      continue;
     }
     if (error_mode == STRING_ERROR_FIGNORE) {
      --end;
      memmove(iter,iter + 1,(end - iter)*sizeof(char));
      continue;
     }
     DeeError_Throwf(&DeeError_UnicodeDecodeError,
                     "Invalid utf-8 character byte 0x%.2I8x",
                     ch);
err_buffer32:
     DeeString_Free4ByteBuffer(buffer32);
     goto err_r;
    }
    ch32 = utf8_getchar(iter,seqlen);
    *dst32++ = ch32;
    iter += seqlen;
   }
   utf_length = (size_t)(dst32 - buffer32);
   ASSERT(utf_length <= WSTR_LENGTH(buffer32));
   if (utf_length != WSTR_LENGTH(buffer32)) {
    uint32_t *new_buffer32;
    new_buffer32 = DeeString_TryResizeBuffer32(buffer32,utf_length);
    if likely(new_buffer32) buffer32 = new_buffer32;
   }
   utf = utf_alloc();
   if unlikely(!utf) goto err_buffer32;
   memset(utf,0,sizeof(*utf));
   utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)buffer32; /* Inherit data */
   Dee_UntrackAlloc((size_t *)buffer32 - 1);
   utf->u_width = STRING_WIDTH_4BYTE;
   utf->u_utf8  = DeeString_STR(result);
  } else {
   uint16_t *buffer16,*dst16;
   /* Must decode into a 2/4-byte string */
   buffer16 = DeeString_NewBuffer16(length - (seqlen - 1));
   if unlikely(!buffer16) goto err_r;
   dst16 = buffer16;
   simple_length = iter - (uint8_t *)result->s_str;
   for (i = 0; i < simple_length; ++i)
       *dst16++ = (uint16_t)(uint8_t)result->s_str[i];
   *dst16++ = (uint16_t)ch32;
   iter += seqlen;
   while (iter < end) {
    ch = *iter;
    if (ch <= 0x7f) {
     ++iter;
     *dst16++ = ch;
     continue;
    }
    seqlen = utf8_sequence_len[ch];
    if unlikely(!seqlen || iter + seqlen > end) {
     /* Invalid UTF-8 character */
     if (error_mode == STRING_ERROR_FREPLAC) {
      *iter++ = '?';
      *dst16++  = '?';
      continue;
     }
     if (error_mode == STRING_ERROR_FIGNORE) {
      --end;
      memmove(iter,iter + 1,(end - iter)*sizeof(char));
      continue;
     }
     DeeError_Throwf(&DeeError_UnicodeDecodeError,
                     "Invalid utf-8 character byte 0x%.2I8x",
                     ch);
err_buffer16:
     DeeString_Free2ByteBuffer(buffer16);
     goto err_r;
    }
    ch32 = utf8_getchar(iter,seqlen);
    if unlikely(ch32 > 0xffff) {
     iter += seqlen;
     /* Must upgrade to use a 4-byte buffer. */
     buffer32 = DeeString_NewBuffer32((dst16 - buffer16) + 1 + (end - iter));
     if unlikely(!buffer32) goto err_buffer16;
     simple_length = (size_t)(dst16 - buffer16);
     for (i = 0; i < simple_length; ++i)
         buffer32[i] = buffer16[i];
     DeeString_Free2ByteBuffer(buffer16);
     dst32 = buffer32 + simple_length;
     *dst32++ = ch32;
     goto use_buffer32;
    }
    *dst16++ = (uint16_t)ch32;
    iter += seqlen;
   }
   utf_length = (size_t)(dst16 - buffer16);
   ASSERT(utf_length <= WSTR_LENGTH(buffer16));
   if (utf_length != WSTR_LENGTH(buffer16)) {
    uint16_t *new_buffer16;
    new_buffer16 = DeeString_TryResizeBuffer16(buffer16,utf_length);
    if likely(new_buffer16) buffer16 = new_buffer16;
   }
   utf = utf_alloc();
   if unlikely(!utf) goto err_buffer16;
   memset(utf,0,sizeof(*utf));
   utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)buffer16; /* Inherit data */
   Dee_UntrackAlloc((size_t *)buffer16 - 1);
   utf->u_width = STRING_WIDTH_2BYTE;
   utf->u_utf8  = DeeString_STR(result);
  }
  break;
 }
 result->s_data = utf;
 Dee_UntrackAlloc(utf);
 result->s_hash = DEE_STRING_HASH_UNSET;
 result->s_len  = length;
 result->s_str[length] = '\0';
 DeeObject_Init(result,&DeeString_Type);
 return (DREF DeeObject *)result;
err_r:
 DeeObject_Free(result);
err:
 return NULL;
}


PUBLIC DREF DeeObject *DCALL
DeeString_SetUtf8(/*inherit(always)*/DREF DeeObject *__restrict self,
                  unsigned int error_mode) {
 DREF String *result; uint8_t *iter,*end;
 struct string_utf *utf = NULL;
 uint32_t *buffer32,*dst32;
 size_t i,simple_length,utf_length;
 result = (DREF String *)self;
 /* Search for multi-byte character sequences. */
 end = (iter = (uint8_t *)result->s_str)+result->s_len;
 while (iter < end) {
  uint8_t seqlen,ch = *iter;
  uint32_t ch32;
  if (ch <= 0x7f) { ++iter; continue; }
  seqlen = utf8_sequence_len[ch];
  if unlikely(!seqlen || iter + seqlen > end) {
   /* Invalid UTF-8 character */
   if (error_mode == STRING_ERROR_FREPLAC) {
    *iter = '?';
    ++iter;
    continue;
   }
   if (error_mode == STRING_ERROR_FIGNORE) {
    --end;
    memmove(iter,iter + 1,(end - iter)*sizeof(char));
    continue;
   }
   DeeError_Throwf(&DeeError_UnicodeDecodeError,
                   "Invalid utf-8 character byte 0x%.2I8x",
                   ch);
   goto err_r;
  }
  ch32 = utf8_getchar(iter,seqlen);
  if unlikely(ch32 <= 0x7f) {
   *iter = (uint8_t)ch32;
   memmove(iter + 1,
           iter + seqlen,
          (end - (iter + seqlen))*
           sizeof(char));
   ++iter;
   continue;
  }
  if (ch32 > 0xffff) {
   /* Must decode into a 4-byte string */
   buffer32 = DeeString_NewBuffer32(result->s_len - (seqlen - 1));
   if unlikely(!buffer32) goto err_r;
   dst32 = buffer32;
   simple_length = iter - (uint8_t *)result->s_str;
   for (i = 0; i < simple_length; ++i)
       *dst32++ = (uint32_t)(uint8_t)result->s_str[i];
   *dst32++ = ch32;
   iter += seqlen;
use_buffer32:
   while (iter < end) {
    ch = *iter;
    if (ch <= 0x7f) {
     ++iter;
     *dst32++ = ch;
     continue;
    }
    seqlen = utf8_sequence_len[ch];
    if unlikely(!seqlen || iter + seqlen > end) {
     /* Invalid UTF-8 character */
     if (error_mode == STRING_ERROR_FREPLAC) {
      *iter++ = '?';
      *dst32++  = '?';
      continue;
     }
     if (error_mode == STRING_ERROR_FIGNORE) {
      --end;
      memmove(iter,iter + 1,(end - iter)*sizeof(char));
      continue;
     }
     DeeError_Throwf(&DeeError_UnicodeDecodeError,
                     "Invalid utf-8 character byte 0x%.2I8x",
                     ch);
err_buffer32:
     DeeString_Free4ByteBuffer(buffer32);
     goto err_r;
    }
    ch32 = utf8_getchar(iter,seqlen);
    *dst32++ = ch32;
    iter += seqlen;
   }
   utf_length = (size_t)(dst32 - buffer32);
   ASSERT(utf_length <= WSTR_LENGTH(buffer32));
   if (utf_length != WSTR_LENGTH(buffer32)) {
    uint32_t *new_buffer32;
    new_buffer32 = DeeString_TryResizeBuffer32(buffer32,utf_length);
    if likely(new_buffer32) buffer32 = new_buffer32;
   }
   utf = utf_alloc();
   if unlikely(!utf) goto err_buffer32;
   memset(utf,0,sizeof(*utf));
   utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)buffer32; /* Inherit data */
   Dee_UntrackAlloc((size_t *)buffer32 - 1);
   utf->u_width = STRING_WIDTH_4BYTE;
   utf->u_utf8  = DeeString_STR(result);
  } else {
   uint16_t *buffer16,*dst16;
   /* Must decode into a 2/4-byte string */
   buffer16 = DeeString_NewBuffer16(result->s_len - (seqlen - 1));
   if unlikely(!buffer16) goto err_r;
   dst16 = buffer16;
   simple_length = iter - (uint8_t *)result->s_str;
   for (i = 0; i < simple_length; ++i)
       *dst16++ = (uint16_t)(uint8_t)result->s_str[i];
   *dst16++ = (uint16_t)ch32;
   iter += seqlen;
   while (iter < end) {
    ch = *iter;
    if (ch <= 0x7f) {
     ++iter;
     *dst16++ = ch;
     continue;
    }
    seqlen = utf8_sequence_len[ch];
    if unlikely(!seqlen || iter + seqlen > end) {
     /* Invalid UTF-8 character */
     if (error_mode == STRING_ERROR_FREPLAC) {
      *iter++ = '?';
      *dst16++  = '?';
      continue;
     }
     if (error_mode == STRING_ERROR_FIGNORE) {
      --end;
      memmove(iter,iter + 1,(end - iter)*sizeof(char));
      continue;
     }
     DeeError_Throwf(&DeeError_UnicodeDecodeError,
                     "Invalid utf-8 character byte 0x%.2I8x",
                     ch);
err_buffer16:
     DeeString_Free2ByteBuffer(buffer16);
     goto err_r;
    }
    ch32 = utf8_getchar(iter,seqlen);
    if unlikely(ch32 > 0xffff) {
     iter += seqlen;
     /* Must upgrade to use a 4-byte buffer. */
     buffer32 = DeeString_NewBuffer32((dst16 - buffer16) + 1 + (end - iter));
     if unlikely(!buffer32) goto err_buffer16;
     simple_length = (size_t)(dst16 - buffer16);
     for (i = 0; i < simple_length; ++i)
         buffer32[i] = buffer16[i];
     DeeString_Free2ByteBuffer(buffer16);
     dst32 = buffer32 + simple_length;
     *dst32++ = ch32;
     goto use_buffer32;
    }
    *dst16++ = (uint16_t)ch32;
    iter += seqlen;
   }
   utf_length = (size_t)(dst16 - buffer16);
   ASSERT(utf_length <= WSTR_LENGTH(buffer16));
   if (utf_length != WSTR_LENGTH(buffer16)) {
    uint16_t *new_buffer16;
    new_buffer16 = DeeString_TryResizeBuffer16(buffer16,utf_length);
    if likely(new_buffer16) buffer16 = new_buffer16;
   }
   utf = utf_alloc();
   if unlikely(!utf) goto err_buffer16;
   memset(utf,0,sizeof(*utf));
   utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)buffer16; /* Inherit data */
   Dee_UntrackAlloc((size_t *)buffer16 - 1);
   utf->u_width = STRING_WIDTH_2BYTE;
   utf->u_utf8  = DeeString_STR(result);
  }
  break;
 }
 ASSERT(!DeeObject_IsShared(result));
 ASSERT(!result->s_data);
 result->s_data = utf;
 Dee_UntrackAlloc(utf);
 return (DREF DeeObject *)result;
err_r:
 DeeObject_Free(result);
 return NULL;
}


PUBLIC DREF DeeObject *(DCALL _DeeString_Chr16)(uint16_t ch) {
 uint16_t *buffer;
 if (ch <= 0xff)
     return _DeeString_Chr8((uint8_t)ch);
 buffer = DeeString_NewBuffer16(1);
 if unlikely(!buffer) return NULL;
 buffer[0] = ch;
 return DeeString_Pack2ByteBuffer(buffer);
}
PUBLIC DREF DeeObject *(DCALL _DeeString_Chr32)(uint32_t ch) {
 if (ch <= 0xff)
     return _DeeString_Chr8((uint8_t)ch);
 if (ch <= 0xffff) {
  uint16_t *buffer;
  buffer = DeeString_NewBuffer16(1);
  if unlikely(!buffer) return NULL;
  buffer[0] = (uint16_t)ch;
  return DeeString_Pack2ByteBuffer(buffer);
 } else {
  uint32_t *buffer;
  buffer = DeeString_NewBuffer32(1);
  if unlikely(!buffer) return NULL;
  buffer[0] = ch;
  return DeeString_Pack4ByteBuffer(buffer);
 }
}


INTERN DREF DeeObject *DCALL
DeeString_Convert(DeeObject *__restrict self,
                  size_t start, size_t end,
                  uintptr_t kind) {
 unsigned int width; void *str,*result;
 size_t i,length;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 width  = DeeString_WIDTH(self);
 str    = DeeString_WSTR(self);
 length = WSTR_LENGTH(str);
 if (start > length) start = length;
 if (end > length) end = length;
 if (start >= end) return_empty_string;
 end -= start;
 result = DeeString_NewWidthBuffer(end,width);
 if unlikely(!result) return NULL;
 SWITCH_SIZEOF_WIDTH(width) {
 CASE_WIDTH_1BYTE:
  for (i = 0; i < end; ++i)
     ((uint8_t *)result)[i] = (uint8_t)DeeUni_Convert(((uint8_t *)str)[start + i],kind);
  break;
 CASE_WIDTH_2BYTE:
  for (i = 0; i < end; ++i)
     ((uint16_t *)result)[i] = (uint16_t)DeeUni_Convert(((uint16_t *)str)[start + i],kind);
  break;
 CASE_WIDTH_4BYTE:
  for (i = 0; i < end; ++i)
     ((uint32_t *)result)[i] = (uint32_t)DeeUni_Convert(((uint32_t *)str)[start + i],kind);
  break;
 }
 return DeeString_PackWidthBuffer(result,width);
}

INTERN DREF DeeObject *DCALL
DeeString_ToTitle(DeeObject *__restrict self, size_t start, size_t end) {
 uintptr_t kind = UNICODE_CONVERT_TITLE;
 unsigned int width; void *str,*result;
 size_t i,length;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 width  = DeeString_WIDTH(self);
 str    = DeeString_WSTR(self);
 length = WSTR_LENGTH(str);
 if (start > length) start = length;
 if (end > length) end = length;
 if (start >= end) return_empty_string;
 end -= start;
 result = DeeString_NewWidthBuffer(end,width);
 if unlikely(!result) return NULL;
 SWITCH_SIZEOF_WIDTH(width) {
 CASE_WIDTH_1BYTE:
  for (i = 0; i < end; ++i) {
   uint8_t ch = ((uint8_t *)str)[start + i];
   struct unitraits *desc = DeeUni_Descriptor(ch);
   ((uint8_t *)result)[i] = (uint8_t)(ch + *(int32_t *)((uintptr_t)desc + kind));
   kind = (desc->ut_flags & UNICODE_FSPACE) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
  }
  break;
 CASE_WIDTH_2BYTE:
  for (i = 0; i < end; ++i) {
   uint16_t ch = ((uint16_t *)str)[start + i];
   struct unitraits *desc = DeeUni_Descriptor(ch);
   ((uint16_t *)result)[i] = (uint16_t)(ch + *(int32_t *)((uintptr_t)desc + kind));
   kind = (desc->ut_flags & UNICODE_FSPACE) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
  }
  break;
 CASE_WIDTH_4BYTE:
  for (i = 0; i < end; ++i) {
   uint32_t ch = ((uint32_t *)str)[start + i];
   struct unitraits *desc = DeeUni_Descriptor(ch);
   ((uint32_t *)result)[i] = (uint32_t)(ch + *(int32_t *)((uintptr_t)desc + kind));
   kind = (desc->ut_flags & UNICODE_FSPACE) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
  }
  break;
 }
 return DeeString_PackWidthBuffer(result,width);
}
INTERN DREF DeeObject *DCALL
DeeString_Capitalize(DeeObject *__restrict self, size_t start, size_t end) {
 unsigned int width; void *str,*result;
 size_t i,length;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 width  = DeeString_WIDTH(self);
 str    = DeeString_WSTR(self);
 length = WSTR_LENGTH(str);
 if (start > length) start = length;
 if (end > length) end = length;
 if (start >= end) return_empty_string;
 end -= start;
 result = DeeString_NewWidthBuffer(end,width);
 if unlikely(!result) return NULL;
 SWITCH_SIZEOF_WIDTH(width) {
 CASE_WIDTH_1BYTE:
  ((uint8_t *)result)[0] = (uint8_t)DeeUni_ToUpper(((uint8_t *)str)[start]);
  for (i = 1; i < end; ++i)
     ((uint8_t *)result)[i] = (uint8_t)DeeUni_ToLower(((uint8_t *)str)[start + i]);
  break;
 CASE_WIDTH_2BYTE:
  ((uint16_t *)result)[0] = (uint16_t)DeeUni_ToUpper(((uint16_t *)str)[start]);
  for (i = 1; i < end; ++i)
     ((uint16_t *)result)[i] = (uint16_t)DeeUni_ToLower(((uint16_t *)str)[start + i]);
  break;
 CASE_WIDTH_4BYTE:
  ((uint32_t *)result)[0] = (uint32_t)DeeUni_ToUpper(((uint32_t *)str)[start]);
  for (i = 1; i < end; ++i)
     ((uint32_t *)result)[i] = (uint32_t)DeeUni_ToLower(((uint32_t *)str)[start + i]);
  break;
 }
 return DeeString_PackWidthBuffer(result,width);
}
INTERN DREF DeeObject *DCALL
DeeString_Swapcase(DeeObject *__restrict self, size_t start, size_t end) {
 unsigned int width; void *str,*result;
 size_t i,length;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 width  = DeeString_WIDTH(self);
 str    = DeeString_WSTR(self);
 length = WSTR_LENGTH(str);
 if (start > length) start = length;
 if (end > length) end = length;
 if (start >= end) return_empty_string;
 end -= start;
 result = DeeString_NewWidthBuffer(end,width);
 if unlikely(!result) return NULL;
 SWITCH_SIZEOF_WIDTH(width) {
 CASE_WIDTH_1BYTE:
  for (i = 0; i < end; ++i)
     ((uint8_t *)result)[i] = (uint8_t)DeeUni_SwapCase(((uint8_t *)str)[start + i]);
  break;
 CASE_WIDTH_2BYTE:
  for (i = 0; i < end; ++i)
     ((uint16_t *)result)[i] = (uint16_t)DeeUni_SwapCase(((uint16_t *)str)[start + i]);
  break;
 CASE_WIDTH_4BYTE:
  for (i = 0; i < end; ++i)
     ((uint32_t *)result)[i] = (uint32_t)DeeUni_SwapCase(((uint32_t *)str)[start + i]);
  break;
 }
 return DeeString_PackWidthBuffer(result,width);
}




INTERN uint32_t DCALL
utf8_getchar(uint8_t const *__restrict base, uint8_t seqlen) {
 uint32_t result;
 switch (seqlen) {
 case 0:
  result = 0;
  break;
 case 1:
  result = base[0];
  break;
 case 2:
  result  = (base[0] & 0x1f) << 6;
  result |= (base[1] & 0x3f);
  break;
 case 3:
  result  = (base[0] & 0x0f) << 12;
  result |= (base[1] & 0x3f) << 6;
  result |= (base[2] & 0x3f);
  break;
 case 4:
  result  = (base[0] & 0x07) << 18;
  result |= (base[1] & 0x3f) << 12;
  result |= (base[2] & 0x3f) << 6;
  result |= (base[3] & 0x3f);
  break;
 case 5:
  result  = (base[0] & 0x03) << 24;
  result |= (base[1] & 0x3f) << 18;
  result |= (base[2] & 0x3f) << 12;
  result |= (base[3] & 0x3f) << 6;
  result |= (base[4] & 0x3f);
  break;
 case 6:
  result  = (base[0] & 0x01) << 30;
  result |= (base[1] & 0x3f) << 24;
  result |= (base[2] & 0x3f) << 18;
  result |= (base[3] & 0x3f) << 12;
  result |= (base[4] & 0x3f) << 6;
  result |= (base[5] & 0x3f);
  break;
 case 7:
  result  = (base[1] & 0x03/*0x3f*/) << 30;
  result |= (base[2] & 0x3f) << 24;
  result |= (base[3] & 0x3f) << 18;
  result |= (base[4] & 0x3f) << 12;
  result |= (base[5] & 0x3f) << 6;
  result |= (base[6] & 0x3f);
  break;
 case 8:
  /*result = (base[0] & 0x3f) << 42;*/
  /*result = (base[1] & 0x3f) << 36;*/
  result  = (base[2] & 0x03/*0x3f*/) << 30;
  result |= (base[3] & 0x3f) << 24;
  result |= (base[4] & 0x3f) << 18;
  result |= (base[5] & 0x3f) << 12;
  result |= (base[6] & 0x3f) << 6;
  result |= (base[7] & 0x3f);
  break;
 default: __builtin_unreachable();
 }
 return result;
}

LOCAL uint16_t DCALL
utf8_getchar16(uint8_t const *__restrict base, uint8_t seqlen) {
 uint16_t result;
 ASSERT(seqlen <= 3);
 switch (seqlen) {
 case 0:
  result = 0;
  break;
 case 1:
  result = base[0];
  break;
 case 2:
  result  = (base[0] & 0x1f) << 6;
  result |= (base[1] & 0x3f);
  break;
 case 3:
  result  = (base[0] & 0x0f) << 12;
  result |= (base[1] & 0x3f) << 6;
  result |= (base[2] & 0x3f);
  break;
 default: __builtin_unreachable();
 }
 return result;
}

PUBLIC uint32_t DCALL
utf8_readchar(char const **__restrict piter,
              char const *__restrict end) {
 uint32_t result;
 char const *iter = *piter;
 if (iter >= end) return 0;
 result = *iter++;
 if (result >= 0xc0) {
  uint8_t len;
  len = utf8_sequence_len[result];
  if (iter + len-1 >= end)
      len = (uint8_t)(end - iter)+1;
  switch (len) {
  case 0:
  case 1: break;
  case 2:
   result  = (result & 0x1f) << 6;
   result |= (iter[0] & 0x3f);
   iter += 2;
   break;
  case 3:
   result  = (result & 0x0f) << 12;
   result |= (iter[0] & 0x3f) << 6;
   result |= (iter[1] & 0x3f);
   iter += 3;
   break;
  case 4:
   result  = (result & 0x07) << 18;
   result |= (iter[0] & 0x3f) << 12;
   result |= (iter[1] & 0x3f) << 6;
   result |= (iter[2] & 0x3f);
   iter += 4;
   break;
  case 5:
   result  = (result & 0x03) << 24;
   result |= (iter[0] & 0x3f) << 18;
   result |= (iter[1] & 0x3f) << 12;
   result |= (iter[2] & 0x3f) << 6;
   result |= (iter[3] & 0x3f);
   iter += 5;
   break;
  case 6:
   result  = (result & 0x01) << 30;
   result |= (iter[0] & 0x3f) << 24;
   result |= (iter[1] & 0x3f) << 18;
   result |= (iter[2] & 0x3f) << 12;
   result |= (iter[3] & 0x3f) << 6;
   result |= (iter[4] & 0x3f);
   iter += 6;
   break;
  case 7:
   result  = (iter[0] & 0x03/*0x3f*/) << 30;
   result |= (iter[1] & 0x3f) << 24;
   result |= (iter[2] & 0x3f) << 18;
   result |= (iter[3] & 0x3f) << 12;
   result |= (iter[4] & 0x3f) << 6;
   result |= (iter[5] & 0x3f);
   iter += 7;
   break;
  case 8:
   /*result = (result & 0x3f) << 36;*/
   result  = (iter[1] & 0x03/*0x3f*/) << 30;
   result |= (iter[2] & 0x3f) << 24;
   result |= (iter[3] & 0x3f) << 18;
   result |= (iter[4] & 0x3f) << 12;
   result |= (iter[5] & 0x3f) << 6;
   result |= (iter[6] & 0x3f);
   iter += 8;
   break;
  default: __builtin_unreachable();
  }
 }
 *piter = iter;
 return result;
}
PUBLIC uint32_t DCALL
utf8_readchar_u(char const **__restrict piter) {
 uint32_t result;
 char const *iter = *piter;
 result = *iter++;
 if (result >= 0xc0) {
  uint8_t len;
  len = utf8_sequence_len[result];
  switch (len) {
  case 0:
  case 1: break;
  case 2:
   result  = (result & 0x1f) << 6;
   result |= (iter[0] & 0x3f);
   iter += 2;
   break;
  case 3:
   result  = (result & 0x0f) << 12;
   result |= (iter[0] & 0x3f) << 6;
   result |= (iter[1] & 0x3f);
   iter += 3;
   break;
  case 4:
   result  = (result & 0x07) << 18;
   result |= (iter[0] & 0x3f) << 12;
   result |= (iter[1] & 0x3f) << 6;
   result |= (iter[2] & 0x3f);
   iter += 4;
   break;
  case 5:
   result  = (result & 0x03) << 24;
   result |= (iter[0] & 0x3f) << 18;
   result |= (iter[1] & 0x3f) << 12;
   result |= (iter[2] & 0x3f) << 6;
   result |= (iter[3] & 0x3f);
   iter += 5;
   break;
  case 6:
   result  = (result & 0x01) << 30;
   result |= (iter[0] & 0x3f) << 24;
   result |= (iter[1] & 0x3f) << 18;
   result |= (iter[2] & 0x3f) << 12;
   result |= (iter[3] & 0x3f) << 6;
   result |= (iter[4] & 0x3f);
   iter += 6;
   break;
  case 7:
   result  = (iter[0] & 0x03/*0x3f*/) << 30;
   result |= (iter[1] & 0x3f) << 24;
   result |= (iter[2] & 0x3f) << 18;
   result |= (iter[3] & 0x3f) << 12;
   result |= (iter[4] & 0x3f) << 6;
   result |= (iter[5] & 0x3f);
   iter += 7;
   break;
  case 8:
   /*result = (result & 0x3f) << 36;*/
   result  = (iter[1] & 0x03/*0x3f*/) << 30;
   result |= (iter[2] & 0x3f) << 24;
   result |= (iter[3] & 0x3f) << 18;
   result |= (iter[4] & 0x3f) << 12;
   result |= (iter[5] & 0x3f) << 6;
   result |= (iter[6] & 0x3f);
   iter += 8;
   break;
  default: __builtin_unreachable();
  }
 }
 *piter = iter;
 return result;
}
PUBLIC uint32_t DCALL
utf8_readchar_rev(char const **__restrict pend,
                  char const *__restrict begin) {
 uint32_t result;
 char const *end = *pend;
 uint8_t seqlen = 1;
 while (end > begin &&
       ((unsigned char)*--end & 0xc0) == 0x80 &&
        seqlen < 8) ++seqlen;
 result = utf8_getchar((uint8_t *)end,seqlen);
 *pend = end;
 return result;
}



PUBLIC char *DCALL
utf8_writechar(char *__restrict buffer, uint32_t ch) {
 uint8_t *dst = (uint8_t *)buffer;
 if (ch <= UTF8_1BYTE_MAX) {
  *dst++ = (uint8_t)ch;
 } else if (ch <= UTF8_2BYTE_MAX) {
  *dst++ = 0xc0 | (uint8_t)((ch >> 6)/* & 0x1f*/);
  *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
 } else if (ch <= UTF8_3BYTE_MAX) {
  *dst++ = 0xe0 | (uint8_t)((ch >> 12)/* & 0x0f*/);
  *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
  *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
 } else if (ch <= UTF8_4BYTE_MAX) {
  *dst++ = 0xf0 | (uint8_t)((ch >> 18)/* & 0x07*/);
  *dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
  *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
  *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
 } else if (ch <= UTF8_5BYTE_MAX) {
  *dst++ = 0xf8 | (uint8_t)((ch >> 24)/* & 0x03*/);
  *dst++ = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
  *dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
  *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
  *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
 } else if (ch <= UTF8_6BYTE_MAX) {
  *dst++ = 0xfc | (uint8_t)((ch >> 30)/* & 0x01*/);
  *dst++ = 0x80 | (uint8_t)((ch >> 24) & 0x3f);
  *dst++ = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
  *dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
  *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
  *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
 } else {
  *dst++ = 0xfe;
  *dst++ = 0x80 | (uint8_t)((ch >> 30) & 0x03/* & 0x3f*/);
  *dst++ = 0x80 | (uint8_t)((ch >> 24) & 0x3f);
  *dst++ = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
  *dst++ = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
  *dst++ = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
  *dst++ = 0x80 | (uint8_t)((ch) & 0x3f);
 }
 return (char *)dst;
}





#undef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
#define CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION 1



/* Unicode printer API */
PUBLIC bool DCALL
unicode_printer_allocate(struct unicode_printer *__restrict self,
                         size_t num_chars, unsigned int width) {
 ASSERT(!(width & ~UNICODE_PRINTER_FWIDTH));
 if unlikely(!num_chars) goto done;
 if (!self->up_buffer) {
  /* Allocate an initial buffer. */
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  if likely(!self->up_length) {
   self->up_length = num_chars;
   self->up_flags |= (unsigned char)width;
  } else {
   width = (unsigned int)STRING_WIDTH_COMMON((unsigned int)(self->up_flags & UNICODE_PRINTER_FWIDTH),
                                              width);
   self->up_length += num_chars;
   self->up_flags  &= ~UNICODE_PRINTER_FWIDTH;
   self->up_flags  |= width;
  }
#else
  ASSERT(!self->up_length);
  self->up_buffer = DeeString_TryNewWidthBuffer(num_chars,width);
  if (!self->up_buffer) goto err;
  self->up_flags = (unsigned char)width;
#endif
 } else {
  /* Check if sufficient memory is already available. */
  union dcharptr buffer; size_t avail,required;
  buffer.ptr = self->up_buffer;
  avail = WSTR_LENGTH(buffer.ptr);
  required = self->up_length + num_chars;
  ASSERT(self->up_length <= avail);
  if (avail < required) {
   /* Try to pre-allocate memory by extending upon what has already been written. */
   SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {
   CASE_WIDTH_1BYTE:
    if (width == STRING_WIDTH_1BYTE) {
     DeeStringObject *new_string;
     new_string = (DeeStringObject *)DeeObject_TryRealloc(COMPILER_CONTAINER_OF(buffer.ptr,
                                                                                DeeStringObject,
                                                                                s_str),
                                                          COMPILER_OFFSETOF(DeeStringObject,s_str) +
                                                         (required + 1)*sizeof(char));
     if unlikely(!new_string) goto err;
     new_string->s_len = required;
     self->up_buffer = new_string->s_str;
    } else if (width == STRING_WIDTH_2BYTE) {
     /* Upgrade to a 2-byte string. */
     uint16_t *new_buffer; size_t i;
     new_buffer = DeeString_TryNewBuffer16(required);
     if unlikely(!new_buffer) goto err;
     for (i = 0; i < self->up_length; ++i)
         new_buffer[i] = (uint16_t)buffer.cp8[i];
     DeeObject_Free(COMPILER_CONTAINER_OF(buffer.ptr,
                                          DeeStringObject,
                                          s_str));
     self->up_buffer = new_buffer;
#if STRING_WIDTH_1BYTE != 0
     self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif
     self->up_flags |= STRING_WIDTH_2BYTE;
    } else {
     /* Upgrade to a 4-byte string. */
     uint32_t *new_buffer; size_t i;
     new_buffer = DeeString_TryNewBuffer32(required);
     if unlikely(!new_buffer) goto err;
     for (i = 0; i < self->up_length; ++i)
         new_buffer[i] = (uint32_t)buffer.cp8[i];
     DeeObject_Free(COMPILER_CONTAINER_OF(buffer.ptr,
                                          DeeStringObject,
                                          s_str));
     self->up_buffer = new_buffer;
#if STRING_WIDTH_1BYTE != 0
     self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif
     self->up_flags |= STRING_WIDTH_4BYTE;
    }
    break;
   CASE_WIDTH_2BYTE:
    if (width <= STRING_WIDTH_2BYTE) {
     uint16_t *new_buffer;
     /* Extend the 2-byte buffer. */
     new_buffer = DeeString_TryResizeBuffer16(buffer.cp16,required);
     if unlikely(!new_buffer) goto err;
     self->up_buffer = new_buffer;
    } else {
     /* Upgrade to a 4-byte buffer. */
     uint32_t *new_buffer; size_t i;
     new_buffer = DeeString_TryNewBuffer32(required);
     if unlikely(!new_buffer) goto err;
     for (i = 0; i < self->up_length; ++i)
         new_buffer[i] = (uint32_t)buffer.cp16[i];
     DeeString_Free2ByteBuffer(buffer.cp16);
     self->up_buffer = new_buffer;
     self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
     self->up_flags |= STRING_WIDTH_4BYTE;
    }
    break;
   {
    uint32_t *new_buffer;
   CASE_WIDTH_4BYTE:
    /* Extend the 4-byte buffer. */
    new_buffer = DeeString_TryResizeBuffer32(buffer.cp32,required);
    if unlikely(!new_buffer) goto err;
    self->up_buffer = new_buffer;
   } break;
   }
  }
 }
done:
 return true;
err:
 return false;
}


/* _Always_ inherit all string data (even upon error) saved in
 * `self', and construct a new string from all that data, before
 * returning a reference to that string.
 * NOTE: A pending, incomplete UTF-8 character sequence is discarded.
 *      ---> Regardless of return value, `self' is finalized and left
 *           in an undefined state, the same way it would have been
 *           after a call to `unicode_printer_fini()'
 * @return: * :   A reference to the packed string.
 * @return: NULL: An error occurred. */
PUBLIC DREF DeeObject *DCALL
unicode_printer_pack(/*inherit(always)*/struct unicode_printer *__restrict self) {
 void *result_buffer = self->up_buffer;
 if unlikely(!result_buffer)
    return_empty_string;
 if (self->up_length < WSTR_LENGTH(result_buffer)) {
  void *new_buffer;
  new_buffer = DeeString_TryResizeWidthBuffer(result_buffer,
                                              self->up_length,
                                              self->up_flags & UNICODE_PRINTER_FWIDTH);
  if likely(new_buffer) result_buffer = new_buffer;
 }
#ifdef NDEBUG
 return DeeString_PackWidthBuffer(result_buffer,
                                  self->up_flags & UNICODE_PRINTER_FWIDTH);
#else
 {
  unsigned char flags = self->up_flags;
  memset(self,0xcc,sizeof(*self));
  return DeeString_PackWidthBuffer(result_buffer,
                                   flags & UNICODE_PRINTER_FWIDTH);
 }
#endif
}
PUBLIC DREF DeeObject *DCALL
unicode_printer_trypack(/*inherit(on_success)*/struct unicode_printer *__restrict self) {
 void *result_buffer = self->up_buffer;
 if unlikely(!result_buffer)
    return_empty_string;
 ASSERT(self->up_length <= WSTR_LENGTH(result_buffer));
 if (self->up_length < WSTR_LENGTH(result_buffer)) {
  void *new_buffer;
  new_buffer = DeeString_TryResizeWidthBuffer(result_buffer,
                                              self->up_length,
                                              self->up_flags & UNICODE_PRINTER_FWIDTH);
  if likely(new_buffer) result_buffer = new_buffer;
 }
#ifdef NDEBUG
 return DeeString_TryPackWidthBuffer(result_buffer,
                                     self->up_flags & UNICODE_PRINTER_FWIDTH);
#else
 {
  DREF DeeObject *result;
  result = DeeString_TryPackWidthBuffer(result_buffer,
                                        self->up_flags & UNICODE_PRINTER_FWIDTH);
  if likely(result)
     memset(self,0xcc,sizeof(*self));
  return result;
 }
#endif
}

#define UNICODE_PRINTER_INITIAL_BUFSIZE 64

LOCAL int
(DCALL unicode_printer_putc8)(struct unicode_printer *__restrict self,
                              uint8_t ch) {
 size_t size_avail;
 void *string = self->up_buffer;
 if (!string) {
  /* Allocate the initial buffer. */
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  if (self->up_length > UNICODE_PRINTER_INITIAL_BUFSIZE ||
     (self->up_flags & UNICODE_PRINTER_FWIDTH) != STRING_WIDTH_1BYTE) {
   self->up_buffer = DeeString_TryNewWidthBuffer(self->up_length,
                                                 self->up_flags & UNICODE_PRINTER_FWIDTH);
   if (!self->up_buffer) goto allocate_initial_normally;
   self->up_length = 1;
   UNICODE_PRINTER_SETCHAR(self,0,ch);
   goto done;
  } else {
   DeeStringObject *buffer;
allocate_initial_normally:
   ASSERT((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
   buffer = (DeeStringObject *)DeeObject_TryMalloc(COMPILER_OFFSETOF(DeeStringObject,s_str)+
                                                  (UNICODE_PRINTER_INITIAL_BUFSIZE+1)*sizeof(char));
   if likely(buffer) {
    buffer->s_len = UNICODE_PRINTER_INITIAL_BUFSIZE;
   } else {
    buffer = (DeeStringObject *)DeeObject_Malloc(COMPILER_OFFSETOF(DeeStringObject,s_str)+
                                                (1+1)*sizeof(char));
    if unlikely(!buffer) goto err;
    buffer->s_len = 1;
   }
   buffer->s_str[0] = (char)ch;
   self->up_buffer = buffer->s_str;
   self->up_length = 1;
   goto done;
  }
#else
  DeeStringObject *buffer;
  ASSERT(!self->up_length);
  ASSERT((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
  buffer = (DeeStringObject *)DeeObject_TryMalloc(COMPILER_OFFSETOF(DeeStringObject,s_str)+
                                                 (UNICODE_PRINTER_INITIAL_BUFSIZE+1)*sizeof(char));
  if likely(buffer) {
   buffer->s_len = UNICODE_PRINTER_INITIAL_BUFSIZE;
  } else {
   buffer = (DeeStringObject *)DeeObject_Malloc(COMPILER_OFFSETOF(DeeStringObject,s_str)+
                                               (1+1)*sizeof(char));
   if unlikely(!buffer) goto err;
   buffer->s_len = 1;
  }
  buffer->s_str[0] = (char)ch;
  self->up_buffer = buffer->s_str;
  self->up_length = 1;
  goto done;
#endif
 }
 size_avail = WSTR_LENGTH(string);
 ASSERT(size_avail >= self->up_length);
 if unlikely(size_avail == self->up_length) {
  /* Must allocate more memory. */
  string = DeeString_TryResizeWidthBuffer(string,
                                          size_avail * 2,
                                          self->up_flags & UNICODE_PRINTER_FWIDTH);
  if unlikely(!string) {
   string = DeeString_ResizeWidthBuffer(self->up_buffer,
                                        size_avail + 1,
                                        self->up_flags & UNICODE_PRINTER_FWIDTH);
   if unlikely(!string) goto err;
  }
  self->up_buffer = string;
 }
 /* Simply append the new character. */
 SWITCH_SIZEOF_WIDTH (self->up_flags & UNICODE_PRINTER_FWIDTH) {
 CASE_WIDTH_1BYTE:
  ((uint8_t *)string)[self->up_length] = ch;
  break;
 CASE_WIDTH_2BYTE:
  ((uint16_t *)string)[self->up_length] = ch;
  break;
 CASE_WIDTH_4BYTE:
  ((uint32_t *)string)[self->up_length] = ch;
  break;
 }
 ++self->up_length;
done:
 return 0;
err:
 return -1;
}

PRIVATE ATTR_NOINLINE uint16_t *DCALL
cast_8to16(DeeStringObject *__restrict buffer, size_t used_length) {
 uint16_t *result;
 size_t i,length = buffer->s_len;
 ASSERT(used_length <= length);
 result = DeeString_NewBuffer16(length);
 if unlikely(!result) goto done;
 for (i = 0; i < used_length; ++i)
     result[i] = (uint16_t)((uint8_t *)buffer->s_str)[i];
 DeeObject_Free(buffer);
done:
 return result;
}
PRIVATE ATTR_NOINLINE uint32_t *DCALL
cast_8to32(DeeStringObject *__restrict buffer,
           size_t used_length) {
 uint32_t *result;
 size_t i,length = buffer->s_len;
 ASSERT(used_length <= length);
 result = DeeString_NewBuffer32(length);
 if unlikely(!result) goto done;
 for (i = 0; i < used_length; ++i)
     result[i] = (uint32_t)((uint8_t *)buffer->s_str)[i];
 DeeObject_Free(buffer);
done:
 return result;
}
PRIVATE ATTR_NOINLINE uint32_t *DCALL
cast_16to32(uint16_t *__restrict buffer,
            size_t used_length) {
 uint32_t *result;
 size_t i,length = WSTR_LENGTH(buffer);
 ASSERT(used_length <= length);
 result = DeeString_NewBuffer32(length);
 if unlikely(!result) goto done;
 for (i = 0; i < used_length; ++i)
     result[i] = (uint32_t)buffer[i];
 Dee_Free((size_t *)buffer - 1);
done:
 return result;
}

PUBLIC int
(DCALL unicode_printer_putascii)(struct unicode_printer *__restrict self, char ch) {
 ASSERTF((uint8_t)ch <= 0x7f,"The given ch (U+%.4I8x) is not an ASCII character",
         (uint8_t)ch);
 return unicode_printer_putc8(self,(uint8_t)ch);
}


/* Append a single character to the given printer.
 * If `ch' can't fit the currently set `up_width', copy already
 * written data into a larger representation before appending `ch'.
 * @return:  0: Successfully appended the character.
 * @return: -1: An error occurred. */
PUBLIC int
(DCALL unicode_printer_putc)(struct unicode_printer *__restrict self, uint32_t ch) {
 void *string;
 if (ch <= 0xff)
     return unicode_printer_putc8(self,(uint8_t)ch);
 string = self->up_buffer;
 if (!string) {
  /* Allocate the initial buffer. */
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  size_t initial_length;
  initial_length = self->up_length;
  if (initial_length < UNICODE_PRINTER_INITIAL_BUFSIZE)
      initial_length = UNICODE_PRINTER_INITIAL_BUFSIZE;
  if ((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_4BYTE)
       goto allocate_initial_as_32;
  if ((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_2BYTE && ch <= 0xffff)
       goto allocate_initial_as_16;
  self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
  if (ch <= 0xffff) {
allocate_initial_as_16:
   string = DeeString_TryNewBuffer16(initial_length);
   if unlikely(!string) {
    string = DeeString_TryNewBuffer16(UNICODE_PRINTER_INITIAL_BUFSIZE);
    if unlikely(!string) {
     string = DeeString_NewBuffer16(1);
     if unlikely(!string) goto err;
    }
   }
   ((uint16_t *)string)[0] = (uint16_t)ch;
   self->up_flags |= STRING_WIDTH_2BYTE;
  } else {
allocate_initial_as_32:
   string = DeeString_TryNewBuffer32(initial_length);
   if unlikely(!string) {
    string = DeeString_TryNewBuffer32(UNICODE_PRINTER_INITIAL_BUFSIZE);
    if unlikely(!string) {
     string = DeeString_NewBuffer32(1);
     if unlikely(!string) goto err;
    }
   }
   ((uint32_t *)string)[0] = ch;
   self->up_flags |= STRING_WIDTH_4BYTE;
  }
  self->up_length = 1;
  self->up_buffer = string;
  goto done;
#else
  ASSERT((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
  if (ch <= 0xffff) {
   string = DeeString_TryNewBuffer16(UNICODE_PRINTER_INITIAL_BUFSIZE);
   if unlikely(!string) {
    string = DeeString_NewBuffer16(1);
    if unlikely(!string) goto err;
   }
   ((uint16_t *)string)[0] = (uint16_t)ch;
   self->up_flags |= STRING_WIDTH_2BYTE;
  } else {
   string = DeeString_TryNewBuffer32(UNICODE_PRINTER_INITIAL_BUFSIZE);
   if unlikely(!string) {
    string = DeeString_NewBuffer32(1);
    if unlikely(!string) goto err;
   }
   ((uint32_t *)string)[0] = ch;
   self->up_flags |= STRING_WIDTH_4BYTE;
  }
  self->up_length = 1;
  self->up_buffer = string;
  goto done;
#endif
 }
 ASSERT(self->up_length <= WSTR_LENGTH(string));
 SWITCH_SIZEOF_WIDTH (self->up_flags & UNICODE_PRINTER_FWIDTH) {
 CASE_WIDTH_1BYTE:
  /* Must always upcast, regardless of where we're at. */
  if (ch <= 0xffff) {
   string = cast_8to16(COMPILER_CONTAINER_OF((char *)string,DeeStringObject,s_str),
                       self->up_length);
   if unlikely(!string) goto err;
   self->up_buffer = string;
#if STRING_WIDTH_1BYTE != 0
   self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif
   self->up_flags |= STRING_WIDTH_2BYTE;
   goto append_2byte;
  } else {
   string = cast_8to32(COMPILER_CONTAINER_OF((char *)string,DeeStringObject,s_str),
                       self->up_length);
   if unlikely(!string) goto err;
   self->up_buffer = string;
#if STRING_WIDTH_1BYTE != 0
   self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif
   self->up_flags |= STRING_WIDTH_4BYTE;
   goto append_4byte;
  }
  break;
 CASE_WIDTH_2BYTE:
  if (ch <= 0xffff) {
   /* No need to cast. */
append_2byte:
   ASSERT(self->up_length <= WSTR_LENGTH(string));
   if (self->up_length == WSTR_LENGTH(string)) {
    /* Must allocate more memory. */
    string = DeeString_TryResizeBuffer16((uint16_t *)string,
                                          WSTR_LENGTH(string)*2);
    if unlikely(!string) {
     string = DeeString_ResizeBuffer16((uint16_t *)self->up_buffer,
                                        self->up_length+1);
     if unlikely(!string) goto err;
    }
    self->up_buffer = string;
   }
   ((uint16_t *)string)[self->up_length] = (uint16_t)ch;
  } else {
   /* Must cast from 2 -> 4 bytes. */
   string = cast_16to32((uint16_t *)string,self->up_length);
   if unlikely(!string) goto err;
   self->up_buffer = string;
   self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
   self->up_flags |= STRING_WIDTH_4BYTE;
   goto append_4byte;
  }
  break;
 CASE_WIDTH_4BYTE:
append_4byte:
  /* We're already at max-size, so just append. */
  ASSERT(self->up_length <= WSTR_LENGTH(string));
  if (self->up_length == WSTR_LENGTH(string)) {
   /* Must allocate more memory. */
   string = DeeString_TryResizeBuffer32((uint32_t *)string,
                                         WSTR_LENGTH(string) * 2);
   if unlikely(!string) {
    string = DeeString_ResizeBuffer32((uint32_t *)self->up_buffer,
                                       self->up_length+1);
    if unlikely(!string) goto err;
   }
   self->up_buffer = string;
  }
  ((uint32_t *)string)[self->up_length] = ch;
  break;
 }
 ++self->up_length;
done:
 return 0;
err:
 return -1;
}

/* Append a given UTF-8 character. */
PUBLIC int
(DCALL unicode_printer_pututf8)(struct unicode_printer *__restrict self, char ch) {
 if (self->up_flags & UNICODE_PRINTER_FPENDING) {
  /* Complete a pending UTF-8 multi-byte sequence. */
  uint8_t curlen,reqlen;
  curlen = (self->up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
  self->up_pend[curlen] = (uint8_t)ch;
  reqlen = utf8_sequence_len[self->up_pend[0]];
  ASSERT(curlen+1 <= reqlen);
  if (curlen+1 == reqlen) {
   /* Append the full character. */
   int result;
   uint32_t ch32 = utf8_getchar((uint8_t *)self->up_pend,reqlen);
   result = unicode_printer_putc(self,ch32);
   if likely(result >= 0)
      self->up_flags &= ~UNICODE_PRINTER_FPENDING;
   return result;
  }
  self->up_flags += 1 << UNICODE_PRINTER_FPENDING_SHFT;
  return 0;
 }
 if ((unsigned char)ch >= 0xc0) {
  /* Start of a multi-byte sequence. */
  self->up_pend[0] = (unsigned char)ch;
  self->up_flags |= 1 << UNICODE_PRINTER_FPENDING_SHFT;
  return 0;
 }
 return unicode_printer_putc8(self,(unsigned char)ch);
}

PUBLIC int
(DCALL unicode_printer_pututf16)(struct unicode_printer *__restrict self, uint16_t ch) {
 if (self->up_flags & UNICODE_PRINTER_FPENDING) {
  uint32_t ch32;
  /* Complete a utf-16 surrogate pair. */
  ch32 = UTF16_COMBINE_SURROGATES(*(uint16_t *)self->up_pend,ch);
  self->up_flags &= ~UNICODE_PRINTER_FPENDING;
  return unicode_printer_putc(self,ch32);
 }
 if (ch >= UTF16_HIGH_SURROGATE_MIN &&
     ch <= UTF16_HIGH_SURROGATE_MAX) {
  /* High surrogate word. */
  *(uint16_t *)self->up_pend = ch;
  self->up_flags |= 1 << UNICODE_PRINTER_FPENDING_SHFT;
  return 0;
 }
 return unicode_printer_putc(self,ch);
}



/* Append UTF-8 text to the back of the given printer.
 * An incomplete UTF-8 sequences can be completed by future uses of this function.
 * HINT: This function is intentionally designed as compatible with `dformatprinter'
 * @return: textlen: Successfully appended the string.
 * @return: -1:      Failed to append the string. */
PUBLIC dssize_t DCALL
unicode_printer_print(struct unicode_printer *__restrict self,
                      /*utf-8*/char const *__restrict text,
                      size_t textlen) {
 size_t result = textlen;
 char *flush_start;
 while (self->up_flags & UNICODE_PRINTER_FPENDING) {
  /* Complete a pending UTF-8 sequence. */
  uint8_t curlen,reqlen;
  if (!textlen) goto done;
  curlen = (self->up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
  self->up_pend[curlen] = (uint8_t)*text++;
  reqlen = utf8_sequence_len[self->up_pend[0]];
  ASSERT(curlen+1 <= reqlen);
  if (curlen+1 == reqlen) {
   /* Append the full character. */
   int error;
   uint32_t ch32 = utf8_getchar((uint8_t *)self->up_pend,reqlen);
   error = unicode_printer_putc(self,ch32);
   if unlikely(error < 0) goto err;
   self->up_flags &= ~UNICODE_PRINTER_FPENDING;
   break;
  }
  self->up_flags += 1 << UNICODE_PRINTER_FPENDING_SHFT;
 }
again_flush:
 flush_start = (char *)text;
 while (textlen && (unsigned char)*text < 0xc0) ++text,--textlen;
 /* Print ASCII text. */
 if (flush_start != text) {
  if unlikely(unicode_printer_print8(self,
                                    (uint8_t *)flush_start,
                                    (size_t)(text-flush_start)) < 0)
     goto err;
 }
 if (textlen) {
  uint8_t seqlen;
  uint32_t ch32;
  ASSERT((unsigned char)*text >= 0xc0);
  seqlen = utf8_sequence_len[(uint8_t)*text];
  if (seqlen > textlen) {
   /* Incomplete sequence! (safe as pending UTF-8) */
   memcpyb(self->up_pend,text,textlen);
   self->up_flags |= (uint8_t)textlen << UNICODE_PRINTER_FPENDING_SHFT;
   goto done;
  }
  /* Expand the utf-8 sequence and emit it as a single character. */
  ch32 = utf8_getchar((uint8_t *)text,seqlen);
  if (unicode_printer_putc(self,ch32))
      goto err;
  text += seqlen;
  textlen -= seqlen;
  goto again_flush;
 }
done:
 return result;
err:
 return -1;
}


PUBLIC dssize_t DCALL
unicode_printer_printutf16(struct unicode_printer *__restrict self,
                           /*utf-16*/uint16_t const *__restrict text,
                           size_t textlen) {
 size_t result = textlen;
 uint16_t *flush_start;
 uint16_t low_surrogate,high_surrogate;
 if (self->up_flags & UNICODE_PRINTER_FPENDING) {
  /* Complete a pending UTF-16 sequence. */
  if (!textlen) goto done;
  high_surrogate = *(uint16_t *)self->up_pend;
  low_surrogate  = text[0];
  if (unicode_printer_putc(self,UTF16_COMBINE_SURROGATES(high_surrogate,low_surrogate)))
      goto err;
  self->up_flags &= ~UNICODE_PRINTER_FPENDING;
  ++text;
  --textlen;
 }
again_flush:
 flush_start = (uint16_t *)text;
 while (textlen &&
       (*text < UTF16_HIGH_SURROGATE_MIN ||
        *text > UTF16_HIGH_SURROGATE_MAX))
       ++text,--textlen;
 /* Print pure UTF-16 text. */
 if (flush_start != text) {
  if unlikely(unicode_printer_print16(self,
                                     (uint16_t *)flush_start,
                                     (size_t)(text-flush_start)) < 0)
     goto err;
 }
 if (textlen) {
  uint32_t ch32;
  if (textlen == 1) {
   /* Incomplete surrogate pair. */
   *(uint16_t *)self->up_pend = text[0];
   self->up_flags |= (1 << UNICODE_PRINTER_FPENDING_SHFT);
   goto done;
  }
  /* Compile the surrogates and print the resulting unicode character. */
  high_surrogate = text[0];
  low_surrogate  = text[1];
  ch32 = UTF16_COMBINE_SURROGATES(high_surrogate,low_surrogate);
  if (unicode_printer_putc(self,ch32))
      goto err;
  text    += 2;
  textlen -= 2;
  goto again_flush;
 }
done:
 return result;
err:
 return -1;
}




/* Print raw 8,16 or 32-bit sequences of unicode characters.
 *  - `unicode_printer_print8' prints characters from the range U+0000 .. U+00FF (aka. latin-1)
 *  - `unicode_printer_print16' prints characters from the range U+0000 .. U+FFFF
 *  - `unicode_printer_print32' prints characters from the range U+0000 .. U+10FFFF (FFFFFFFF)
 * @return: textlen: Successfully appended the string.
 * @return: -1:      Failed to append the string. */
PUBLIC dssize_t DCALL
unicode_printer_print8(struct unicode_printer *__restrict self,
                       uint8_t const *__restrict text,
                       size_t textlen) {
 size_t result = textlen;
 void *string = self->up_buffer;
 if (!string) {
  String *base_string;
  size_t initial_alloc;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  ASSERT(!self->up_length);
  ASSERT((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
#endif
  if unlikely(!textlen) goto done;
  /* Allocate the initial buffer. */
  initial_alloc = textlen;
  if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
      initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  if (initial_alloc < self->up_length)
      initial_alloc = self->up_length;
  self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
  self->up_flags |= STRING_WIDTH_1BYTE;
#endif
#endif
  base_string = (String *)DeeObject_TryMalloc(COMPILER_OFFSETOF(String,s_str)+
                                             (initial_alloc+1)*sizeof(char));
  if unlikely(!base_string) {
   initial_alloc = textlen;
   base_string = (String *)DeeObject_Malloc(COMPILER_OFFSETOF(String,s_str)+
                                           (initial_alloc+1)*sizeof(char));
   if unlikely(!base_string) goto err;
  }
  base_string->s_len = initial_alloc;
  self->up_buffer = string = base_string->s_str;
  self->up_length = textlen;
  memcpyb(string,text,textlen);
  goto done;
 }
 /* Append new string data. */
 SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {
 CASE_WIDTH_1BYTE:
  if (self->up_length + textlen > WSTR_LENGTH(string)) {
   String *new_string; size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + textlen);
   new_string = (String *)DeeObject_TryRealloc(COMPILER_CONTAINER_OF((char *)string,String,s_str),
                                               COMPILER_OFFSETOF(String,s_str)+
                                              (new_alloc+1)*sizeof(char));
   if unlikely(!new_string) {
    new_alloc = self->up_length + textlen;
    new_string = (String *)DeeObject_Realloc(COMPILER_CONTAINER_OF((char *)string,String,s_str),
                                             COMPILER_OFFSETOF(String,s_str)+
                                            (new_alloc+1)*sizeof(char));
    if unlikely(!new_string) goto err;
   }
   new_string->s_len = new_alloc;
   self->up_buffer = string = new_string->s_str;
  }
  memcpyb((uint8_t *)string + self->up_length,
          (uint8_t *)text,textlen);
  self->up_length += textlen;
  break;
 {
  uint16_t *dst;
 CASE_WIDTH_2BYTE:
  if (self->up_length + textlen > WSTR_LENGTH(string)) {
   size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + textlen);
   string = DeeString_TryResizeBuffer16((uint16_t *)string,new_alloc);
   if unlikely(!string) {
    string = DeeString_TryResizeBuffer16((uint16_t *)self->up_buffer,
                                             self->up_length + textlen);
    if unlikely(!string) goto err;
   }
   self->up_buffer = string;
  }
  dst = ((uint16_t *)string) + self->up_length;
  self->up_length += textlen;
  while (textlen--) *dst++ = *text++;
 } break;
 {
  uint32_t *dst;
 CASE_WIDTH_4BYTE:
  if (self->up_length + textlen > WSTR_LENGTH(string)) {
   size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + textlen);
   string = DeeString_TryResizeBuffer32((uint32_t *)string,new_alloc);
   if unlikely(!string) {
    string = DeeString_TryResizeBuffer32((uint32_t *)self->up_buffer,
                                             self->up_length + textlen);
    if unlikely(!string) goto err;
   }
   self->up_buffer = string;
  }
  dst = ((uint32_t *)string) + self->up_length;
  self->up_length += textlen;
  while (textlen--) *dst++ = *text++;
 } break;
 }
done:
 return result;
err:
 return -1;
}


PUBLIC dssize_t DCALL
unicode_printer_repeatascii(struct unicode_printer *__restrict self,
                            char ch, size_t num_chars) {
 size_t result = num_chars;
 void *string = self->up_buffer;
 if (!string) {
  String *base_string;
  size_t initial_alloc;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  ASSERT(!self->up_length);
  ASSERT((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
#endif
  if unlikely(!num_chars) goto done;
  /* Allocate the initial buffer. */
  initial_alloc = num_chars;
  if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
      initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  if (initial_alloc < self->up_length)
      initial_alloc = self->up_length;
  self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
  self->up_flags |= STRING_WIDTH_1BYTE;
#endif
#endif
  base_string = (String *)DeeObject_TryMalloc(COMPILER_OFFSETOF(String,s_str)+
                                             (initial_alloc+1)*sizeof(char));
  if unlikely(!base_string) {
   initial_alloc = num_chars;
   base_string = (String *)DeeObject_Malloc(COMPILER_OFFSETOF(String,s_str)+
                                           (initial_alloc+1)*sizeof(char));
   if unlikely(!base_string) goto err;
  }
  base_string->s_len = initial_alloc;
  self->up_buffer = string = base_string->s_str;
  self->up_length = num_chars;
  memset(string,(uint8_t)ch,num_chars);
  goto done;
 }
 /* Append new string data. */
 SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {
 CASE_WIDTH_1BYTE:
  if (self->up_length + num_chars > WSTR_LENGTH(string)) {
   String *new_string; size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + num_chars);
   new_string = (String *)DeeObject_TryRealloc(COMPILER_CONTAINER_OF((char *)string,String,s_str),
                                               COMPILER_OFFSETOF(String,s_str)+
                                              (new_alloc+1)*sizeof(char));
   if unlikely(!new_string) {
    new_alloc = self->up_length + num_chars;
    new_string = (String *)DeeObject_Realloc(COMPILER_CONTAINER_OF((char *)string,String,s_str),
                                             COMPILER_OFFSETOF(String,s_str)+
                                            (new_alloc+1)*sizeof(char));
    if unlikely(!new_string) goto err;
   }
   new_string->s_len = new_alloc;
   self->up_buffer = string = new_string->s_str;
  }
  memset((uint8_t *)string + self->up_length,(uint8_t)ch,num_chars);
  self->up_length += num_chars;
  break;
 {
  uint16_t *dst;
 CASE_WIDTH_2BYTE:
  if (self->up_length + num_chars > WSTR_LENGTH(string)) {
   size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + num_chars);
   string = DeeString_TryResizeBuffer16((uint16_t *)string,new_alloc);
   if unlikely(!string) {
    string = DeeString_TryResizeBuffer16((uint16_t *)self->up_buffer,
                                             self->up_length + num_chars);
    if unlikely(!string) goto err;
   }
   self->up_buffer = string;
  }
  dst = ((uint16_t *)string) + self->up_length;
  self->up_length += num_chars;
  while (num_chars--) *dst++ = (uint16_t)(uint8_t)ch;
 } break;
 {
  uint32_t *dst;
 CASE_WIDTH_4BYTE:
  if (self->up_length + num_chars > WSTR_LENGTH(string)) {
   size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + num_chars);
   string = DeeString_TryResizeBuffer32((uint32_t *)string,new_alloc);
   if unlikely(!string) {
    string = DeeString_TryResizeBuffer32((uint32_t *)self->up_buffer,
                                             self->up_length + num_chars);
    if unlikely(!string) goto err;
   }
   self->up_buffer = string;
  }
  dst = ((uint32_t *)string) + self->up_length;
  self->up_length += num_chars;
  while (num_chars--) *dst++ = (uint32_t)(uint8_t)ch;
 } break;
 }
done:
 return result;
err:
 return -1;
}


PUBLIC dssize_t DCALL
unicode_printer_print16(struct unicode_printer *__restrict self,
                        uint16_t const *__restrict text,
                        size_t textlen) {
 size_t result = textlen;
 void *string = self->up_buffer;
 if (!string) {
  size_t initial_alloc;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  ASSERT(!self->up_length);
  ASSERT((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
#endif
  if unlikely(!textlen) goto done;
  initial_alloc = textlen;
  if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
      initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  if (initial_alloc < self->up_length)
      initial_alloc = self->up_length;
  self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
  self->up_flags |= STRING_WIDTH_1BYTE;
#endif
#endif
  /* Allocate the initial buffer. */
  string = DeeString_TryNewBuffer16(initial_alloc);
  if unlikely(!string) {
   string = DeeString_NewBuffer16(textlen);
   if unlikely(!string) goto err;
  }
  memcpyw(string,text,textlen);
  self->up_buffer = string;
  self->up_length = textlen;
#if STRING_WIDTH_1BYTE != 0
   self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif
  self->up_flags |= STRING_WIDTH_2BYTE;
  goto done;
 }
 SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {
 {
  uint8_t *dst; size_t i;
 CASE_WIDTH_1BYTE:
  /* Check if there are any characters > 0xff */
  for (i = 0; i < textlen; ++i) {
   if (text[i] <= 0xff) continue;
   /* Must up-cast */
   string = cast_8to16(COMPILER_CONTAINER_OF((char *)string,DeeStringObject,s_str),
                       self->up_length);
   if unlikely(!string) goto err;
   self->up_buffer = string;
#if STRING_WIDTH_1BYTE != 0
   self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif
   self->up_flags |= STRING_WIDTH_2BYTE;
   goto append_2byte;
  }
  if (self->up_length + textlen > WSTR_LENGTH(string)) {
   size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + textlen);
   string = DeeString_TryResizeWidthBuffer(string,new_alloc,
                                           self->up_flags & UNICODE_PRINTER_FWIDTH);
   if unlikely(!string) {
    string = DeeString_ResizeWidthBuffer(self->up_buffer,self->up_length + textlen,
                                         self->up_flags & UNICODE_PRINTER_FWIDTH);
    if unlikely(!string) goto err;
   }
   self->up_buffer = string;
  }
  dst = (uint8_t *)string + self->up_length;
  self->up_length += textlen;
  while (textlen--) *dst++ = (uint8_t)*text++;
 } break;
 CASE_WIDTH_2BYTE:
append_2byte:
  if (self->up_length + textlen > WSTR_LENGTH(string)) {
   size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + textlen);
   string = DeeString_TryResizeBuffer16((uint16_t *)string,new_alloc);
   if unlikely(!string) {
    string = DeeString_TryResizeBuffer16((uint16_t *)self->up_buffer,
                                             self->up_length + textlen);
    if unlikely(!string) goto err;
   }
   self->up_buffer = string;
  }
  memcpyw((uint16_t *)string + self->up_length,text,textlen);
  self->up_length += textlen;
  break;
 {
  uint32_t *dst;
 CASE_WIDTH_4BYTE:
  if (self->up_length + textlen > WSTR_LENGTH(string)) {
   size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + textlen);
   string = DeeString_TryResizeBuffer32((uint32_t *)string,new_alloc);
   if unlikely(!string) {
    string = DeeString_TryResizeBuffer32((uint32_t *)self->up_buffer,
                                             self->up_length + textlen);
    if unlikely(!string) goto err;
   }
   self->up_buffer = string;
  }
  dst = (uint32_t *)string + self->up_length;
  self->up_length += textlen;
  while (textlen--) *dst++ = *text++;
 } break;
 }
done:
 return result;
err:
 return -1;
}

PUBLIC dssize_t DCALL
unicode_printer_print32(struct unicode_printer *__restrict self,
                        uint32_t const *__restrict text,
                        size_t textlen) {
 size_t i,result = textlen;
 void *string = self->up_buffer;
 if (textlen == 1)
     return unicode_printer_putc(self,text[0]) ? -1 : 1;
 if (!string) {
  size_t initial_alloc;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  ASSERT(!self->up_length);
  ASSERT((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
#endif
  if unlikely(!textlen) goto done;
  initial_alloc = textlen;
  if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
      initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  if (initial_alloc < self->up_length)
      initial_alloc = self->up_length;
  self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
  self->up_flags |= STRING_WIDTH_1BYTE;
#endif
#endif
  /* Allocate the initial buffer. */
  string = DeeString_TryNewBuffer32(initial_alloc);
  if unlikely(!string) {
   string = DeeString_NewBuffer32(textlen);
   if unlikely(!string) goto err;
  }
  memcpyl(string,text,textlen);
  self->up_buffer = string;
  self->up_length = textlen;
#if STRING_WIDTH_1BYTE != 0
   self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif
  self->up_flags |= STRING_WIDTH_4BYTE;
  goto done;
 }
 SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {
 {
  uint8_t *dst;
 CASE_WIDTH_1BYTE:
  /* Check if there are any characters > 0xff */
  for (i = 0; i < textlen; ++i) {
   if (text[i] <= 0xff) continue;
   if (text[i] <= 0xffff) {
    /* Must up-cast to 16-bit */
    string = cast_8to16(COMPILER_CONTAINER_OF((char *)string,DeeStringObject,s_str),
                        self->up_length);
    if unlikely(!string) goto err;
    self->up_buffer = string;
#if STRING_WIDTH_1BYTE != 0
    self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif
    self->up_flags |= STRING_WIDTH_2BYTE;
    goto append_2byte;
   }
   /* Must up-cast to 32-bit */
   string = cast_8to32(COMPILER_CONTAINER_OF((char *)string,DeeStringObject,s_str),
                       self->up_length);
   if unlikely(!string) goto err;
   self->up_buffer = string;
#if STRING_WIDTH_1BYTE != 0
   self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif
   self->up_flags |= STRING_WIDTH_4BYTE;
   goto append_4byte;
  }
  if (self->up_length + textlen > WSTR_LENGTH(string)) {
   size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + textlen);
   string = DeeString_TryResizeWidthBuffer(string,new_alloc,
                                           self->up_flags & UNICODE_PRINTER_FWIDTH);
   if unlikely(!string) {
    string = DeeString_ResizeWidthBuffer(self->up_buffer,self->up_length + textlen,
                                         self->up_flags & UNICODE_PRINTER_FWIDTH);
    if unlikely(!string) goto err;
   }
   self->up_buffer = string;
  }
  dst = (uint8_t *)string + self->up_length;
  self->up_length += textlen;
  while (textlen--) *dst++ = (uint8_t)*text++;
 } break;
 {
  uint16_t *dst;
 CASE_WIDTH_2BYTE:
append_2byte:
  /* Check if there are any characters > 0xffff */
  for (i = 0; i < textlen; ++i) {
   if (text[i] <= 0xffff) continue;
   /* Must up-cast to 32-bit */
   string = cast_16to32((uint16_t *)string,
                         self->up_length);
   if unlikely(!string) goto err;
   self->up_buffer = string;
   self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
   self->up_flags |= STRING_WIDTH_4BYTE;
   goto append_4byte;
  }
  if (self->up_length + textlen > WSTR_LENGTH(string)) {
   size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + textlen);
   string = DeeString_TryResizeBuffer16((uint16_t *)string,new_alloc);
   if unlikely(!string) {
    string = DeeString_TryResizeBuffer16((uint16_t *)self->up_buffer,
                                             self->up_length + textlen);
    if unlikely(!string) goto err;
   }
   self->up_buffer = string;
  }
  dst = (uint16_t *)string + self->up_length;
  self->up_length += textlen;
  while (textlen--) *dst++ = (uint16_t)*text++;
 } break;
 CASE_WIDTH_4BYTE:
append_4byte:
  if (self->up_length + textlen > WSTR_LENGTH(string)) {
   size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + textlen);
   string = DeeString_TryResizeBuffer32((uint32_t *)string,new_alloc);
   if unlikely(!string) {
    string = DeeString_TryResizeBuffer32((uint32_t *)self->up_buffer,
                                             self->up_length + textlen);
    if unlikely(!string) goto err;
   }
   self->up_buffer = string;
  }
  memcpyl((uint32_t *)string + self->up_length,text,textlen);
  self->up_length += textlen;
  break;
 }
done:
 return result;
err:
 return -1;
}

PUBLIC dssize_t DCALL
unicode_printer_printinto(struct unicode_printer *__restrict self,
                          dformatprinter printer, void *arg) {
 void *str; size_t length;
 dssize_t temp,result;
 /* Optimization for when the target is another unicode-printer. */
 length = self->up_length;
 if (printer == (dformatprinter)&unicode_printer_print) {
  struct unicode_printer *dst;
  dst = (struct unicode_printer *)arg;
  if (!dst->up_buffer) {
   /* The simplest cast: the target buffer is currently empty, and since
    * we're allocated to modify `self', we can simply move everything! */
   memcpy(dst,self,sizeof(struct unicode_printer));
   self->up_length = 0;
   self->up_buffer = NULL;
   self->up_flags  = STRING_WIDTH_1BYTE;
   return dst->up_length;
  }
  /* Append all data from out buffer.
   * Since we know that the target is a unicode-printer,
   * we don't have to perform any UTF-8 conversions! */
  str = self->up_buffer;
  SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {
  CASE_WIDTH_1BYTE:
   return unicode_printer_print8(dst,(uint8_t *)str,length);
  CASE_WIDTH_2BYTE:
   return unicode_printer_print16(dst,(uint16_t *)str,length);
  CASE_WIDTH_4BYTE:
   return unicode_printer_print32(dst,(uint32_t *)str,length);
  }
 }
 /* Because we must print everything in UTF-8, this part gets a
  * bit complicated since we need to convert everything on-the-fly. */
 str = self->up_buffer;
 result = 0;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
 if (str != NULL)
#endif
 {
  SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {
  {
   char *iter,*end,*flush_start;
  CASE_WIDTH_1BYTE:
   /* If we're a 1-byte string, at the very least
    * we can directly print out ASCII-only data, and
    * make use of a flush-like buffering system. */
   end = (flush_start = iter = (char *)str)+length;
   while (iter < end) {
    uint8_t ut8_encoded[2];
    uint8_t ch = (uint8_t)*iter;
    if likely(ch < 0x80) { ++iter; continue; }
    /* LATIN-1 character (must flush until here, then print its unicode variant) */
    if (flush_start < iter) {
     temp = (*printer)(arg,flush_start,(size_t)(iter-flush_start));
     if unlikely(temp < 0) goto err;
     result += temp;
    }
    ut8_encoded[0] = 0xc0 | ((ch & 0xc0) >> 6);
    ut8_encoded[1] = 0x80 | (ch & 0x3f);
    /* Print the UTF-8 variant. */
    temp = (*printer)(arg,(char *)ut8_encoded,2);
    if unlikely(temp < 0) goto err;
    result += temp;

    flush_start = ++iter;
   }
   /* Flush the remainder. */
   if (flush_start < end) {
    temp = (*printer)(arg,flush_start,(size_t)(end-flush_start));
    if unlikely(temp < 0) goto err;
    result += temp;
   }
  } break;
  {
   size_t i;
  CASE_WIDTH_2BYTE:
   /* Since our string already uses more than 1 byte per character,
    * there'd be no point in trying to buffer anything, since any
    * buffer we'd be using would also have to be allocated by us.
    * Instead, let the caller deal with buffering (if they wish to),
    * and simply print one character at a time. */
   for (i = 0; i < length; ++i) {
    uint8_t utf8_repr[3];
    uint8_t utf8_length;
    uint16_t ch = ((uint16_t *)str)[i];
    if (ch <= UTF8_1BYTE_MAX) {
     utf8_repr[0] = (uint8_t)ch;
     utf8_length = 1;
    } else if (ch <= UTF8_2BYTE_MAX) {
     utf8_repr[0] = 0xc0 | (uint8_t)((ch >> 6)/* & 0x1f*/);
     utf8_repr[1] = 0x80 | (uint8_t)((ch) & 0x3f);
     utf8_length = 2;
    } else {
     utf8_repr[0] = 0xe0 | (uint8_t)((ch >> 12)/* & 0x0f*/);
     utf8_repr[1] = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
     utf8_repr[2] = 0x80 | (uint8_t)((ch) & 0x3f);
     utf8_length = 3;
    }
    temp = (*printer)(arg,(char *)utf8_repr,utf8_length);
    if unlikely(temp < 0) goto err;
    result += temp;
   }
  } break;
  {
   size_t i;
  CASE_WIDTH_4BYTE:
   /* Same as the 2-byte variant: print one character at a time. */
   for (i = 0; i < length; ++i) {
    uint8_t utf8_repr[7];
    uint8_t utf8_length;
    uint32_t ch = ((uint32_t *)str)[i];
    if (ch <= UTF8_1BYTE_MAX) {
     utf8_repr[0] = (uint8_t)ch;
     utf8_length = 1;
    } else if (ch <= UTF8_2BYTE_MAX) {
     utf8_repr[0] = 0xc0 | (uint8_t)((ch >> 6)/* & 0x1f*/);
     utf8_repr[1] = 0x80 | (uint8_t)((ch) & 0x3f);
     utf8_length = 2;
    } else if (ch <= UTF8_3BYTE_MAX) {
     utf8_repr[0] = 0xe0 | (uint8_t)((ch >> 12)/* & 0x0f*/);
     utf8_repr[1] = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
     utf8_repr[2] = 0x80 | (uint8_t)((ch) & 0x3f);
     utf8_length = 3;
    } else if (ch <= UTF8_4BYTE_MAX) {
     utf8_repr[0] = 0xf0 | (uint8_t)((ch >> 18)/* & 0x07*/);
     utf8_repr[1] = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
     utf8_repr[2] = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
     utf8_repr[3] = 0x80 | (uint8_t)((ch) & 0x3f);
     utf8_length = 4;
    } else if (ch <= UTF8_5BYTE_MAX) {
     utf8_repr[0] = 0xf8 | (uint8_t)((ch >> 24)/* & 0x03*/);
     utf8_repr[1] = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
     utf8_repr[2] = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
     utf8_repr[3] = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
     utf8_repr[4] = 0x80 | (uint8_t)((ch) & 0x3f);
     utf8_length = 5;
    } else if (ch <= UTF8_6BYTE_MAX) {
     utf8_repr[0] = 0xfc | (uint8_t)((ch >> 30)/* & 0x01*/);
     utf8_repr[1] = 0x80 | (uint8_t)((ch >> 24) & 0x3f);
     utf8_repr[2] = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
     utf8_repr[3] = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
     utf8_repr[4] = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
     utf8_repr[5] = 0x80 | (uint8_t)((ch) & 0x3f);
     utf8_length = 6;
    } else {
     utf8_repr[0] = 0xfe;
     utf8_repr[1] = 0x80 | (uint8_t)((ch >> 30) & 0x03/* & 0x3f*/);
     utf8_repr[2] = 0x80 | (uint8_t)((ch >> 24) & 0x3f);
     utf8_repr[3] = 0x80 | (uint8_t)((ch >> 18) & 0x3f);
     utf8_repr[4] = 0x80 | (uint8_t)((ch >> 12) & 0x3f);
     utf8_repr[5] = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
     utf8_repr[6] = 0x80 | (uint8_t)((ch) & 0x3f);
     utf8_length = 7;
    }
    temp = (*printer)(arg,(char *)utf8_repr,utf8_length);
    if unlikely(temp < 0) goto err;
    result += temp;
   }
  } break;
  }
 }
 return result;
err:
 return temp;
}

PUBLIC dssize_t DCALL
unicode_printer_reserve(struct unicode_printer *__restrict self,
                        size_t num_chars) {
 size_t result = self->up_length;
 void *str = self->up_buffer;
 if unlikely(!str) {
  String *init_buffer; size_t initial_alloc;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  ASSERT(self->up_length == 0);
  ASSERT(self->up_flags == STRING_WIDTH_1BYTE);
#endif
  /* Allocate the initial buffer. */
  initial_alloc = num_chars;
  if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
      initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  if (initial_alloc < self->up_length)
      initial_alloc = self->up_length;
  self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
  self->up_flags |= STRING_WIDTH_1BYTE;
#endif
#endif
  init_buffer = (String *)DeeObject_TryMalloc(COMPILER_OFFSETOF(String,s_str)+
                                             (initial_alloc+1)*sizeof(char));
  if unlikely(!init_buffer) {
   initial_alloc = num_chars;
   init_buffer = (String *)DeeObject_Malloc(COMPILER_OFFSETOF(String,s_str)+
                                           (initial_alloc+1)*sizeof(char));
   if unlikely(!init_buffer) goto err;
  }
  init_buffer->s_len = initial_alloc;
  self->up_buffer = init_buffer;
  self->up_length = num_chars;
 } else {
  size_t size_avail = WSTR_LENGTH(str);
  size_t new_length = self->up_length + num_chars;
  if (new_length > size_avail) {
   /* Must allocate more memory. */
   size_t new_size = size_avail;
   do new_size *= 2;
   while (new_size < new_length);
   str = DeeString_TryResizeWidthBuffer(str,new_size,
                                        self->up_flags & UNICODE_PRINTER_FWIDTH);
   if unlikely(!str) {
    str = DeeString_ResizeWidthBuffer(str,new_length,
                                      self->up_flags & UNICODE_PRINTER_FWIDTH);
    if unlikely(!str) goto err;
   }
   self->up_buffer = str;
  }
  self->up_length = new_length;
 }
 return (dssize_t)result;
err:
 return -1;
}






PUBLIC dssize_t DCALL
Dee_FormatPutc(dformatprinter printer, void *arg, uint32_t ch) {
 char utf8_repr[UTF8_MAX_MBLEN];
 size_t utf8_len;
 if (printer == (dformatprinter)&unicode_printer_print) {
  if (unicode_printer_putc((struct unicode_printer *)arg,ch))
      return -1;
  return 1;
 }
 utf8_len = (size_t)(utf8_writechar(utf8_repr,ch) - utf8_repr);
 return (*printer)(arg,utf8_repr,utf8_len);
}




/* Search for existing occurrences of, or append a new instance of a given string.
 * Upon success, return the index (offset from the base) of the string (in characters).
 * @return: * : The offset from the base of string being printed, where the given `str' can be found.
 * @return: -1: Failed to allocate the string. */
PUBLIC dssize_t DCALL
unicode_printer_reuse(struct unicode_printer *__restrict self,
                      /*utf-8*/char const *__restrict str, size_t length) {
 size_t result;
 /* TODO: Search for an existing instance. */
 result = self->up_length;
 if unlikely(unicode_printer_print(self,str,length))
    goto err;
 return result;
err:
 return -1;
}
PUBLIC dssize_t DCALL
unicode_printer_reuse8(struct unicode_printer *__restrict self,
                       uint8_t const *__restrict str, size_t length) {
 size_t result;
 /* TODO: Search for an existing instance. */
 result = self->up_length;
 if unlikely(unicode_printer_print8(self,str,length))
    goto err;
 return result;
err:
 return -1;
}
PUBLIC dssize_t DCALL
unicode_printer_reuse16(struct unicode_printer *__restrict self,
                        uint16_t const *__restrict str, size_t length) {
 size_t result;
 /* TODO: Search for an existing instance. */
 result = self->up_length;
 if unlikely(unicode_printer_print16(self,str,length))
    goto err;
 return result;
err:
 return -1;
}
PUBLIC dssize_t DCALL
unicode_printer_reuse32(struct unicode_printer *__restrict self,
                        uint32_t const *__restrict str, size_t length) {
 size_t result;
 /* TODO: Search for an existing instance. */
 result = self->up_length;
 if unlikely(unicode_printer_print32(self,str,length))
    goto err;
 return result;
err:
 return -1;
}


/* Unicode utf-8 buffer API */
PUBLIC char *DCALL
unicode_printer_tryalloc_utf8(struct unicode_printer *__restrict self,
                              size_t length) {
 void *string = self->up_buffer;
 if (!string) {
  String *base_string;
  size_t initial_alloc;
  uint8_t num_pending;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  ASSERT(!self->up_length);
  ASSERT((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
#endif
  num_pending = (self->up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
  length += num_pending;
  /* Allocate the initial buffer. */
  initial_alloc = length;
  if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
      initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  if (initial_alloc < self->up_length)
      initial_alloc = self->up_length;
  self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
  self->up_flags |= STRING_WIDTH_1BYTE;
#endif
#endif
  base_string = (String *)DeeObject_TryMalloc(COMPILER_OFFSETOF(String,s_str)+
                                             (initial_alloc+1)*sizeof(char));
  if unlikely(!base_string) {
   initial_alloc = length;
   base_string = (String *)DeeObject_TryMalloc(COMPILER_OFFSETOF(String,s_str)+
                                              (initial_alloc+1)*sizeof(char));
   if unlikely(!base_string) goto err;
  }
  base_string->s_len = initial_alloc;
  self->up_length = length;
  self->up_buffer = string = base_string->s_str;
  if (!num_pending) return (char *)string;
  memcpy(string,self->up_pend,num_pending);
  return (char *)string + num_pending;
 }
 if ((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
  /* The unicode printer uses a single-byte character width, so we
   * can allocate from that buffer and later check if only UTF-8
   * was written. */
  char *result; uint8_t num_pending;
  num_pending = (self->up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
  length += num_pending;
  if (self->up_length + length > WSTR_LENGTH(string)) {
   String *new_string; size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + length);
   new_string = (String *)DeeObject_TryRealloc(COMPILER_CONTAINER_OF((char *)string,String,s_str),
                                               COMPILER_OFFSETOF(String,s_str)+
                                              (new_alloc+1)*sizeof(char));
   if unlikely(!new_string) {
    new_alloc = self->up_length + length;
    new_string = (String *)DeeObject_TryRealloc(COMPILER_CONTAINER_OF((char *)string,String,s_str),
                                                COMPILER_OFFSETOF(String,s_str)+
                                               (new_alloc+1)*sizeof(char));
    if unlikely(!new_string) goto err;
   }
   new_string->s_len = new_alloc;
   self->up_buffer = string = new_string->s_str;
  }
  result = (char *)((uint8_t *)string + self->up_length);
  self->up_length += length;
  if (!num_pending) return result;
  memcpy(result,self->up_pend,num_pending);
  return result + num_pending;
 }
 /* The unicode printer already uses a character width of more
  * than 1 byte, meaning we can't allocate the buffer in-line.
  * In this case, we allocate the caller-requested buffer separately on the heap. */
 return (char *)Dee_TryMalloc((length+1)*sizeof(char));
err:
 return NULL;
}
PUBLIC char *DCALL
unicode_printer_tryresize_utf8(struct unicode_printer *__restrict self,
                               char *buf, size_t new_length) {
 if ((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
  size_t old_length,total_avail,new_alloc,old_alloc;
  String *new_buffer;
  if (!buf) return unicode_printer_tryalloc_utf8(self,new_length);
  ASSERT(self->up_buffer != NULL);
  ASSERT(buf >= (char *)((uint8_t *)self->up_buffer));
  ASSERT(buf <= (char *)((uint8_t *)self->up_buffer+self->up_length));
  /* The buffer was allocated in-line. */
  old_length  = (size_t)(((uint8_t *)self->up_buffer+self->up_length) - (uint8_t *)buf);
  total_avail = (size_t)(((uint8_t *)self->up_buffer+WSTR_LENGTH(self->up_buffer)) - (uint8_t *)buf);
  ASSERT(total_avail >= old_length);
  if (new_length <= total_avail) {
   /* Update the buffer length within the pre-allocated bounds. */
   self->up_length -= old_length;
   self->up_length += new_length;
   return buf;
  }
  /* Must allocate a new buffer. */
  old_alloc = WSTR_LENGTH(self->up_buffer);
  new_alloc = (self->up_length - old_length) + new_length;
  ASSERT(old_alloc < new_alloc);
  ASSERT(old_alloc != 0);
  do old_alloc *= 2;
  while (old_alloc < new_alloc);
  /* Reallocate the buffer to fit the requested size. */
  new_buffer = (String *)DeeObject_TryRealloc(COMPILER_CONTAINER_OF(self->up_buffer,
                                                                    String,
                                                                    s_str),
                                             (old_alloc+1)*sizeof(char));
  if unlikely(!new_buffer) {
   old_alloc = new_alloc;
   new_buffer = (String *)DeeObject_TryRealloc(COMPILER_CONTAINER_OF(self->up_buffer,
                                                                     String,
                                                                     s_str),
                                              (old_alloc+1)*sizeof(char));
   if unlikely(!new_buffer) goto err;
  }
  /* Install the reallocated buffer. */
  self->up_buffer = new_buffer->s_str;
  new_buffer->s_len = old_alloc;
  self->up_length -= old_length;
  buf = (char *)((uint8_t *)new_buffer->s_str + self->up_length);
  self->up_length += new_length;
  return buf;
 } else {
  /* The buffer is purely heap-allocated. */
  return (char *)Dee_TryRealloc(buf,(new_length+1)*sizeof(char));
 }
err:
 return NULL;
}
PUBLIC char *DCALL
unicode_printer_alloc_utf8(struct unicode_printer *__restrict self,
                           size_t length) {
 char *result;
 do result = unicode_printer_tryalloc_utf8(self,length);
 while (!result && Dee_CollectMemory(length*sizeof(uint8_t)));
 return result;
}
PUBLIC char *DCALL
unicode_printer_resize_utf8(struct unicode_printer *__restrict self,
                            char *buf, size_t new_length) {
 char *result;
 do result = unicode_printer_tryresize_utf8(self,buf,new_length);
 while (!result && Dee_CollectMemory(new_length*sizeof(uint8_t)));
 return result;
}
PUBLIC void DCALL
unicode_printer_free_utf8(struct unicode_printer *__restrict self, char *buf) {
 if ((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
  if (!buf) return;
  /* Deal with pending characters. */
  buf -= (self->up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
  ASSERT(buf >= (char *)((uint8_t *)self->up_buffer));
  ASSERT(buf <= (char *)((uint8_t *)self->up_buffer+self->up_length));
  /* Mark the buffer memory as having been freed again. */
  self->up_length = (size_t)((uint8_t *)buf - (uint8_t *)self->up_buffer);
 } else {
  Dee_Free(buf);
 }
}
PUBLIC dssize_t DCALL
unicode_printer_confirm_utf8(struct unicode_printer *__restrict self,
                             char *buf, size_t final_length) {
 if ((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
  size_t count; uint8_t *iter;
  size_t confirm_length = final_length;
  if (!buf) return 0;
  if (self->up_flags & UNICODE_PRINTER_FPENDING) {
   uint8_t num_pending;
   /* Include pending UTF-8 characters.
    * -> When the buffer was originally allocated, these had been included
    *    as a hidden prefix to the buffer that the user requested. */
   num_pending     = (self->up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
   buf            -= num_pending;
   confirm_length += num_pending;
   self->up_flags &= ~UNICODE_PRINTER_FPENDING;
  }
  ASSERT(buf                >= (char *)((uint8_t *)self->up_buffer));
  ASSERT(buf+confirm_length <= (char *)((uint8_t *)self->up_buffer+self->up_length));
  /* Now that the caller initialized the UTF-8 content, we must
   * check if it contains any unicode sequences that cannot appear
   * as part of a 1-byte string. */
  iter = (uint8_t *)buf,count = confirm_length;
  while (count--) {
   uint8_t ch,utf8_length;
   uint32_t ch32;
   ch = *iter++;
   if (ch < 0xc0) continue; /* Pure ASCII / invalid UTF-8 (which we ignore here) */
   /* Decode the full unicode character. */
   utf8_length = utf8_sequence_len[ch];
   ASSERT(utf8_length >= 2);
   if (utf8_length > count) {
    /* Incomplete, trailing UTF-8 sequence */
    self->up_flags |= (uint8_t)count << UNICODE_PRINTER_FPENDING_SHFT;
    memcpy(self->up_pend,iter-1,count*sizeof(uint8_t));
    confirm_length -= count;
    break;
   }
   /* Decode the unicode character. */
   ASSERT(count != 0);
   ch32 = utf8_getchar(iter-1,utf8_length);
   if (ch32 <= 0xff) {
    /* The character still fits into a single byte! */
    uint8_t *new_iter;
    iter[-1] = (uint8_t)ch32;
    new_iter = iter + utf8_length - 1;
    memmove(iter,new_iter,
           (size_t)(((uint8_t *)buf+confirm_length) - new_iter) *
                      sizeof(uint8_t));
    --count;
    --confirm_length;
    iter = new_iter;
    continue;
   }
   /* Must up-cast to a 16-bit, or 32-bit string. */
   {
    uint8_t *utf8_start = iter-1;
    size_t utf8_convlength = count+1;
    uint8_t *singlebyte_start = (uint8_t *)self->up_buffer;
    size_t singlebyte_length = (size_t)(utf8_start-singlebyte_start);
    if (ch32 <= 0xffff) {
     /* Quick check if all characters still left to parsed can fit into 16 bits. */
     size_t i,w16_length = singlebyte_length+utf8_convlength;
     uint16_t *string16,*dst;
     for (i = utf8_convlength; i < utf8_convlength;) {
      ch = utf8_start[i];
      if (ch <= 0xc0) { ++i; continue; }
      utf8_length = utf8_sequence_len[ch];
      ASSERT(utf8_length >= 2);
      if (i + utf8_length > utf8_convlength) {
       /* Incomplete UTF-8 sequence. */
       uint8_t pendsz = (uint8_t)(utf8_convlength-i);
       memcpy(self->up_pend,&utf8_start[i],pendsz);
       self->up_flags |= pendsz << UNICODE_PRINTER_FPENDING_SHFT;
       utf8_convlength = i;
       break;
      }
      /* All utf-8 sequences of less than 4 characters can fit into 16 bits. */
      if (utf8_length >= 4)
          goto upcast_to_32bit;
      w16_length -= (utf8_length-1);
      i += utf8_length;
     }
     /* Allocate the new 16-bit string. */
     string16 = DeeString_NewBuffer16(w16_length);
     if unlikely(!string16) goto err;
     for (i = 0; i < singlebyte_length; ++i)
         string16[i] = singlebyte_start[i];
     dst = string16 + singlebyte_length;
     for (i = utf8_convlength; i < utf8_convlength;) {
      ch = utf8_start[i];
      if (ch <= 0xc0) { *dst++ = ch; ++i; continue; }
      utf8_length = utf8_sequence_len[ch];
      ASSERT(utf8_length >= 2);
      ASSERT(utf8_length <= 4);
      *dst++ = utf8_getchar16(&utf8_start[i],utf8_length);
      i     += utf8_length;
     }
     ASSERT(dst == string16 + w16_length);
     /* All right! we've got the 16-bit string all created
      * (including the new content from the caller's buffer)
      * Now get rid of the old 8-bit string and upgrade the printer. */
     DeeObject_Free(COMPILER_CONTAINER_OF(self->up_buffer,String,s_str));
     self->up_buffer = string16;
     self->up_length = w16_length;
#if STRING_WIDTH_1BYTE != 0
     self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif
     self->up_flags |= STRING_WIDTH_2BYTE;
     goto return_final_length;
    } else {
     size_t i,w32_length;
     uint32_t *string32,*dst;
upcast_to_32bit:
     w32_length = singlebyte_length+utf8_convlength;
     /* Determine the length of the 32-bit string that we're about to construct. */
     for (i = utf8_convlength; i < utf8_convlength;) {
      ch = utf8_start[i];
      if (ch <= 0xc0) { ++i; continue; }
      utf8_length = utf8_sequence_len[ch];
      ASSERT(utf8_length >= 2);
      w32_length -= (utf8_length-1);
      i          += utf8_length;
     }
     /* Allocate the new 32-bit string. */
     string32 = DeeString_NewBuffer32(w32_length);
     if unlikely(!string32) goto err;
     for (i = 0; i < singlebyte_length; ++i)
         string32[i] = singlebyte_start[i];
     dst = string32 + singlebyte_length;
     for (i = utf8_convlength; i < utf8_convlength;) {
      ch = utf8_start[i];
      if (ch <= 0xc0) { *dst++ = ch; ++i; continue; }
      utf8_length = utf8_sequence_len[ch];
      ASSERT(utf8_length >= 2);
      *dst++ = utf8_getchar(&utf8_start[i],utf8_length);
      i     += utf8_length;
     }
     ASSERT(dst == string32 + w32_length);
     /* All right! we've got the 32-bit string all created
      * (including the new content from the caller's buffer)
      * Now get rid of the old 8-bit string and upgrade the printer. */
     DeeObject_Free(COMPILER_CONTAINER_OF(self->up_buffer,String,s_str));
     self->up_buffer = string32;
     self->up_length = w32_length;
#if STRING_WIDTH_1BYTE != 0
     self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif
     self->up_flags |= STRING_WIDTH_4BYTE;
     goto return_final_length;
    }
   }
  }
  /* Remember the actual length of the buffer. */
  self->up_length = (size_t)(((uint8_t *)buf + confirm_length) - (uint8_t *)self->up_buffer);
return_final_length:
  return (dssize_t)final_length;
 } else {
  dssize_t result;
  /* Simply print the buffer as UTF-8 text. */
  result = unicode_printer_print(self,
                                 buf,
                                 final_length);
  Dee_Free(buf);
  return result;
 }
err:
 return -1;
}




PUBLIC uint16_t *DCALL
unicode_printer_tryalloc_utf16(struct unicode_printer *__restrict self,
                               size_t length) {
 void *string = self->up_buffer;
 if (!string) {
  size_t initial_alloc;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  ASSERT(!self->up_length);
  ASSERT((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
#endif
  if (self->up_flags & UNICODE_PRINTER_FPENDING) ++length;
  /* Allocate the initial buffer. */
  initial_alloc = length;
  if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
      initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  if (initial_alloc < self->up_length)
      initial_alloc = self->up_length;
  self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
  self->up_flags |= STRING_WIDTH_1BYTE;
#endif
#endif
  string = DeeString_TryNewBuffer16(initial_alloc);
  if unlikely(!string) {
   string = DeeString_TryNewBuffer16(length);
   if unlikely(!string) goto err;
  }
  self->up_length = length;
  self->up_buffer = string;
#if STRING_WIDTH_1BYTE != 0
  self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif
  self->up_flags |= STRING_WIDTH_2BYTE;
  if (!(self->up_flags & UNICODE_PRINTER_FPENDING))
        return (uint16_t *)string;
  *(uint16_t *)string = *(uint16_t *)self->up_pend;
  return (uint16_t *)string + 1;
 }
 if ((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_2BYTE) {
  uint16_t *result;
  if (self->up_flags & UNICODE_PRINTER_FPENDING) ++length;
  if (self->up_length + length > WSTR_LENGTH(string)) {
   size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + length);
   string = DeeString_TryResizeBuffer16((uint16_t *)string,new_alloc);
   if unlikely(!string) {
    string = DeeString_TryResizeBuffer16((uint16_t *)string,self->up_length + length);
    if unlikely(!string) goto err;
   }
   self->up_buffer = string;
  }
  result = (uint16_t *)string + self->up_length;
  self->up_length += length;
  if (!(self->up_flags & UNICODE_PRINTER_FPENDING))
        return result;
  *result = *(uint16_t *)self->up_pend;
  return result + 1;
 }
 return (uint16_t *)Dee_TryMalloc((length+1)*sizeof(uint16_t));
err:
 return NULL;
}
PUBLIC uint16_t *DCALL
unicode_printer_tryresize_utf16(struct unicode_printer *__restrict self,
                                uint16_t *buf, size_t new_length) {
 if ((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_2BYTE) {
  uint16_t *string;
  size_t old_length,total_avail,new_alloc,old_alloc;
  if (!buf) return unicode_printer_tryalloc_utf16(self,new_length);
  string = (uint16_t *)self->up_buffer;
  ASSERT(string != NULL);
  ASSERT(buf >= string);
  ASSERT(buf <= string+self->up_length);
  /* The buffer was allocated in-line. */
  old_length  = (size_t)((string+self->up_length) - buf);
  total_avail = (size_t)((string+WSTR_LENGTH(string)) - buf);
  ASSERT(total_avail >= old_length);
  if (new_length <= total_avail) {
   /* Update the buffer length within the pre-allocated bounds. */
   self->up_length -= old_length;
   self->up_length += new_length;
   return buf;
  }
  /* Must allocate a new buffer. */
  old_alloc = WSTR_LENGTH(string);
  new_alloc = (self->up_length - old_length) + new_length;
  ASSERT(old_alloc < new_alloc);
  ASSERT(old_alloc != 0);
  do old_alloc *= 2;
  while (old_alloc < new_alloc);
  /* Reallocate the buffer to fit the requested size. */
  string = DeeString_TryResizeBuffer16(string,old_alloc);
  if unlikely(!string) {
   string = DeeString_TryResizeBuffer16(string,new_alloc);
   if unlikely(!string) goto err;
  }
  /* Install the reallocated buffer. */
  self->up_buffer  = string;
  self->up_length -= old_length;
  buf = string + self->up_length;
  self->up_length += new_length;
  return buf;
 } else {
  /* The buffer is purely heap-allocated. */
  return (uint16_t *)Dee_TryRealloc(buf,(new_length+1)*sizeof(uint16_t));
 }
err:
 return NULL;
}
PUBLIC uint16_t *DCALL
unicode_printer_alloc_utf16(struct unicode_printer *__restrict self,
                            size_t length) {
 uint16_t *result;
 do result = unicode_printer_tryalloc_utf16(self,length);
 while (!result && Dee_CollectMemory(length*sizeof(uint16_t)));
 return result;
}
PUBLIC uint16_t *DCALL
unicode_printer_resize_utf16(struct unicode_printer *__restrict self,
                             uint16_t *buf, size_t new_length) {
 uint16_t *result;
 do result = unicode_printer_tryresize_utf16(self,buf,new_length);
 while (!result && Dee_CollectMemory(new_length*sizeof(uint16_t)));
 return result;
}
PUBLIC void DCALL
unicode_printer_free_utf16(struct unicode_printer *__restrict self,
                           uint16_t *buf) {
 if ((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_2BYTE) {
  if (!buf) return;
  /* Deal with pending characters. */
  buf -= (self->up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
  ASSERT(buf >= (uint16_t *)self->up_buffer);
  ASSERT(buf <= (uint16_t *)self->up_buffer+self->up_length);
  /* Mark the buffer memory as having been freed again. */
  self->up_length = (size_t)((uint16_t *)buf - (uint16_t *)self->up_buffer);
 } else {
  Dee_Free(buf);
 }
}

PUBLIC dssize_t DCALL
unicode_printer_confirm_utf16(struct unicode_printer *__restrict self,
                              uint16_t *buf, size_t final_length) {
 if ((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_2BYTE) {
  size_t count; uint16_t *iter;
  size_t confirm_length = final_length;
  if (!buf) return 0;
  if (self->up_flags & UNICODE_PRINTER_FPENDING) {
   --buf;
   ++confirm_length;
   self->up_flags &= ~UNICODE_PRINTER_FPENDING;
  }
  ASSERT(buf                >= (uint16_t *)self->up_buffer);
  ASSERT(buf+confirm_length <= (uint16_t *)self->up_buffer+self->up_length);
  /* Search for surrogates to see if the string can works as a pure 16-bit string. */
  iter = buf,count = confirm_length;
  while (count--) {
   uint16_t ch = *iter++;
   if (ch < UTF16_HIGH_SURROGATE_MIN ||
       ch > UTF16_HIGH_SURROGATE_MAX)
       continue;
   if (!count) {
    /* Unmatched high surrogate. */
    *(uint16_t *)self->up_pend = ch;
    self->up_flags |= 1 << UNICODE_PRINTER_FPENDING_SHFT;
    break;
   }
   /* Must up-cast to a full 32-bit string. */
   {
    uint32_t *w32_string,*dst;
    uint16_t *utf16_start = iter-1;
    size_t utf16_length = count+1;
    uint16_t *doublebyte_start = (uint16_t *)self->up_buffer;
    size_t doublebyte_length = (size_t)(utf16_start-doublebyte_start);
    size_t i,result_length = doublebyte_length + utf16_length+1;
    goto check_low_surrogate;
    while (count--) {
     ch = *iter++;
     if (ch < UTF16_HIGH_SURROGATE_MIN ||
         ch > UTF16_HIGH_SURROGATE_MAX)
         continue;
check_low_surrogate:
     --result_length; /* High surrogate of surrogate pair. */
     if (!count--) {
      /* Unmatched high surrogate. */
      *(uint16_t *)self->up_pend = ch;
      self->up_flags |= 1 << UNICODE_PRINTER_FPENDING_SHFT;
      break;
     }
     ch = *iter++;
     if (ch < UTF16_LOW_SURROGATE_MIN ||
         ch > UTF16_LOW_SURROGATE_MAX) {
      /* Invalid surrogate pair! */
      iter[-2] = '?';
      memmove(iter-1,iter,count*sizeof(uint16_t));
      continue;
     }
    }
    /* Allocate the combined 32-bit string. */
    w32_string = DeeString_NewBuffer32(result_length);
    if unlikely(!w32_string) goto err;
    dst = w32_string;
    for (i = 0; i < doublebyte_length; ++i)
        *dst++ = doublebyte_start[i];
    /* Decode the utf-16 surrogates */
    for (i = 0; i < utf16_length; ++i) {
     uint16_t high;
     high = utf16_start[i];
     if (high < UTF16_HIGH_SURROGATE_MIN ||
         high > UTF16_HIGH_SURROGATE_MAX) {
      *dst++ = high; /* Regular unicode character. */
     } else {
      ++i;
      ASSERT(i < utf16_length);
      *dst++ = UTF16_COMBINE_SURROGATES(high,utf16_start[i]);
     }
    }
    *dst = 0;
    ASSERT(dst == w32_string + result_length);
    /* Store the new 32-bit string buffer in the printer. */
    Dee_Free((size_t *)self->up_buffer - 1);
    self->up_buffer = w32_string;
    self->up_length = result_length;
    self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
    self->up_flags |= STRING_WIDTH_4BYTE;
    goto return_final_length;
   }
  }
  /* Remember the actual length of the buffer. */
  self->up_length = (size_t)(((uint16_t *)buf + confirm_length) - (uint16_t *)self->up_buffer);
return_final_length:
  return (dssize_t)final_length;
 } else {
  dssize_t result;
  /* Simply print the buffer as UTF-16 text. */
  result = unicode_printer_printutf16(self,
                                      buf,
                                      final_length);
  Dee_Free(buf);
  return result;
 }
err:
 return -1;
}



PUBLIC uint32_t *
(DCALL unicode_printer_tryalloc_utf32)(struct unicode_printer *__restrict self,
                                       size_t length) {
 void *string = self->up_buffer;
 if (!string) {
  size_t initial_alloc;
#ifndef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  ASSERT(!self->up_length);
  ASSERT((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
#endif
  /* Allocate the initial buffer. */
  initial_alloc = length;
  if (initial_alloc < UNICODE_PRINTER_INITIAL_BUFSIZE)
      initial_alloc = UNICODE_PRINTER_INITIAL_BUFSIZE;
#ifdef CONFIG_UNICODE_PRINTER_LAZY_PREALLOCATION
  if (initial_alloc < self->up_length)
      initial_alloc = self->up_length;
  self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#if STRING_WIDTH_1BYTE != 0
  self->up_flags |= STRING_WIDTH_1BYTE;
#endif
#endif
  string = DeeString_TryNewBuffer32(initial_alloc);
  if unlikely(!string) {
   string = DeeString_TryNewBuffer32(length);
   if unlikely(!string) goto err;
  }
  self->up_length = length;
  self->up_buffer = string;
#if STRING_WIDTH_1BYTE != 0
  self->up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif
  self->up_flags |= STRING_WIDTH_4BYTE;
  return (uint32_t *)string;
 }
 if ((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_4BYTE) {
  uint32_t *result;
  if (self->up_length + length > WSTR_LENGTH(string)) {
   size_t new_alloc;
   /* Must allocate more memory. */
   new_alloc = WSTR_LENGTH(string);
   do new_alloc *= 2;
   while (new_alloc < self->up_length + length);
   string = DeeString_TryResizeBuffer32((uint32_t *)string,new_alloc);
   if unlikely(!string) {
    string = DeeString_TryResizeBuffer32((uint32_t *)string,self->up_length + length);
    if unlikely(!string) goto err;
   }
   self->up_buffer = string;
  }
  result = (uint32_t *)string + self->up_length;
  self->up_length += length;
  return result;
 }
 return (uint32_t *)Dee_TryMalloc((length+1)*sizeof(uint32_t));
err:
 return NULL;
}
PUBLIC uint32_t *
(DCALL unicode_printer_tryresize_utf32)(struct unicode_printer *__restrict self,
                                        uint32_t *buf, size_t new_length) {
 if ((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_4BYTE) {
  uint32_t *string;
  size_t old_length,total_avail,new_alloc,old_alloc;
  if (!buf) return unicode_printer_tryalloc_utf32(self,new_length);
  string = (uint32_t *)self->up_buffer;
  ASSERT(string != NULL);
  ASSERT(buf >= string);
  ASSERT(buf <= string+self->up_length);
  /* The buffer was allocated in-line. */
  old_length  = (size_t)((string+self->up_length) - buf);
  total_avail = (size_t)((string+WSTR_LENGTH(string)) - buf);
  ASSERT(total_avail >= old_length);
  if (new_length <= total_avail) {
   /* Update the buffer length within the pre-allocated bounds. */
   self->up_length -= old_length;
   self->up_length += new_length;
   return buf;
  }
  /* Must allocate a new buffer. */
  old_alloc = WSTR_LENGTH(string);
  new_alloc = (self->up_length - old_length) + new_length;
  ASSERT(old_alloc < new_alloc);
  ASSERT(old_alloc != 0);
  do old_alloc *= 2;
  while (old_alloc < new_alloc);
  /* Reallocate the buffer to fit the requested size. */
  string = DeeString_TryResizeBuffer32(string,old_alloc);
  if unlikely(!string) {
   string = DeeString_TryResizeBuffer32(string,new_alloc);
   if unlikely(!string) goto err;
  }
  /* Install the reallocated buffer. */
  self->up_buffer  = string;
  self->up_length -= old_length;
  buf = string + self->up_length;
  self->up_length += new_length;
  return buf;
 } else {
  /* The buffer is purely heap-allocated. */
  return (uint32_t *)Dee_TryRealloc(buf,(new_length+1)*sizeof(uint32_t));
 }
err:
 return NULL;
}
PUBLIC uint32_t *
(DCALL unicode_printer_alloc_utf32)(struct unicode_printer *__restrict self,
                                    size_t length) {
 uint32_t *result;
 do result = unicode_printer_tryalloc_utf32(self,length);
 while (!result && Dee_CollectMemory(length*sizeof(uint32_t)));
 return result;
}
PUBLIC uint32_t *
(DCALL unicode_printer_resize_utf32)(struct unicode_printer *__restrict self,
                                     uint32_t *buf, size_t new_length) {
 uint32_t *result;
 do result = unicode_printer_tryresize_utf32(self,buf,new_length);
 while (!result && Dee_CollectMemory(new_length*sizeof(uint32_t)));
 return result;
}
PUBLIC void
(DCALL unicode_printer_free_utf32)(struct unicode_printer *__restrict self,
                                   uint32_t *buf) {
 if ((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_4BYTE) {
  if (!buf) return;
  ASSERT(buf >= (uint32_t *)self->up_buffer);
  ASSERT(buf <= (uint32_t *)self->up_buffer+self->up_length);
  /* Mark the buffer memory as having been freed again. */
  self->up_length = (size_t)((uint32_t *)buf - (uint32_t *)self->up_buffer);
 } else {
  Dee_Free(buf);
 }
}
PUBLIC dssize_t
(DCALL unicode_printer_confirm_utf32)(struct unicode_printer *__restrict self,
                                      /*inherit(always)*/uint32_t *buf,
                                      size_t final_length) {
 if ((self->up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_2BYTE) {
  if (!buf) return 0;
  ASSERT(buf              >= (uint32_t *)self->up_buffer);
  ASSERT(buf+final_length <= (uint32_t *)self->up_buffer+self->up_length);
  /* Remember the actual length of the buffer. */
  self->up_length = (size_t)(((uint32_t *)buf + final_length) - (uint32_t *)self->up_buffer);
  return (dssize_t)final_length;
 } else {
  dssize_t result;
  /* Simply print the buffer as UTF-16 text. */
  result = unicode_printer_printutf32(self,
                                      buf,
                                      final_length);
  Dee_Free(buf);
  return result;
 }
}







PUBLIC DREF DeeObject *DCALL
DeeString_FromBackslashEscaped(/*utf-8*/char const *__restrict start,
                               size_t length, unsigned int error_mode) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 if unlikely(DeeString_DecodeBackslashEscaped(&printer,start,length,error_mode))
    goto err;
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}

PUBLIC int DCALL
DeeString_DecodeBackslashEscaped(struct unicode_printer *__restrict printer,
                                 /*utf-8*/char const *__restrict start,
                                 size_t length, unsigned int error_mode){
 char const *iter,*end,*flush_start;
 end = (iter = start)+length;
 flush_start = iter;
 while (iter < end) {
  char ch = *iter;
  uint32_t digit_value;
  if (ch != '\\') { ++iter; continue; }
  if (unicode_printer_print(printer,flush_start,
                           (size_t)(iter-flush_start)) < 0)
      goto err;
  ++iter;
  ch = *iter++;
  switch (ch) {

  case '\\': /* Escaped the following character itself. */
  case '\'':
  case '\"':
  case '?':
   flush_start = iter-1;
   continue;

  case '\n':
  case '\r':
   break; /* Escaped line-feed */

  {
   unsigned int count;
   unsigned int max_digits;
  case 'U':
   max_digits = 8;
   goto parse_hex_integer;
  case 'u':
   max_digits = 4;
   goto parse_hex_integer;
  case 'x':
  case 'X':
   max_digits = (unsigned int)-1; /* Unlimited. */
parse_hex_integer:
   count       = 0;
   digit_value = 0;
   while (count < max_digits) {
    struct unitraits *desc;
    uint32_t ch32; uint8_t val;
    char const *old_iter = iter;
    ch32 = utf8_readchar(&iter,end);
    if (ch32 >= 'a' && ch32 <= 'f')
     val = 10 + ((uint8_t)ch32 - 'a');
    else if (ch32 >= 'A' && ch32 <= 'F')
     val = 10 + ((uint8_t)ch32 - 'A');
    else {
     desc = DeeUni_Descriptor(ch32);
     if (!(desc->ut_flags & UNICODE_FDECIMAL) ||
           desc->ut_digit >= 10) {
      iter = old_iter;
      break;
     }
     val = desc->ut_digit;
    }
    digit_value <<= 4;
    digit_value |= val;
    ++count;
   }
   if (!count) {
    if (error_mode & (STRING_ERROR_FIGNORE|
                      STRING_ERROR_FREPLAC))
        goto continue_or_replace;
    DeeError_Throwf(&DeeError_UnicodeDecodeError,
                    "No digits, or hex-chars found after \\x, \\u or \\U");
    goto err;
   }
   if (unicode_printer_putc(printer,digit_value))
       goto err;
  } break;

  case 'a': ch = (char)0x07; goto put_ch;
  case 'b': ch = (char)0x08; goto put_ch;
  case 'f': ch = (char)0x0c; goto put_ch;
  case 'n': ch = (char)0x0a; goto put_ch;
  case 'r': ch = (char)0x0d; goto put_ch;
  case 't': ch = (char)0x09; goto put_ch;
  case 'v': ch = (char)0x0b; goto put_ch;
  case 'e': ch = (char)0x1b; /*goto put_ch;*/
put_ch:
  if (unicode_printer_putc(printer,(uint32_t)(unsigned char)ch))
      goto err;
  break;

  default:
   if (ch >= '0' && ch <= '7') {
    unsigned int count;
    digit_value = (uint32_t)(ch - '0');
parse_oct_integer:
    /* Octal-encoded integer. */
    count = 1;
    while (count < 3) {
     struct unitraits *desc;
     uint32_t ch32;
     char const *old_iter = iter;
     ch32 = utf8_readchar(&iter,end);
     desc = DeeUni_Descriptor(ch32);
     if (!(desc->ut_flags & UNICODE_FDECIMAL) ||
           desc->ut_digit >= 8) {
      iter = old_iter;
      break;
     }
     digit_value <<= 3;
     digit_value |= desc->ut_digit;
     ++count;
    }
    if (unicode_printer_putc(printer,digit_value))
        goto err;
    break;
   }
   if ((unsigned char)ch >= 0xc0) {
    uint32_t ch32;
    struct unitraits *desc;
    --iter;
    ch32 = utf8_readchar(&iter,end);
    desc = DeeUni_Descriptor(ch32);
    if (desc->ut_flags & UNICODE_FLF)
        break; /* Escaped line-feed */
    if ((desc->ut_flags & UNICODE_FDECIMAL) &&
        (desc->ut_digit < 8)) {
     /* Unicode digit character. */
     digit_value = desc->ut_digit;
     goto parse_oct_integer;
    }
   }
   /* Fallback: Disregard the character being escaped, and include
    *           the following character as part of the next flush. */
   flush_start = iter-1;
continue_or_replace:
   if (error_mode & STRING_ERROR_FIGNORE)
       continue;
   if (error_mode & STRING_ERROR_FREPLAC) {
    if (unicode_printer_putc(printer,'?'))
        goto err;
    continue;
   }
   DeeError_Throwf(&DeeError_UnicodeDecodeError,
                   "Unknown escape character %c",
                   ch);
   goto err;
  }
  flush_start = iter;
 }
 /* Flush the remainder. */
 if (unicode_printer_print(printer,flush_start,
                          (size_t)(end-flush_start)) < 0)
     goto err;
 return 0;
err:
 return -1;
}



/* Append the given `text' to the end of the bytes object.
 * This function is intended to be used as the general-purpose
 * dformatprinter-compatible callback for generating data to-be
 * written into a bytes object. */
PUBLIC dssize_t DCALL
bytes_printer_print(struct bytes_printer *__restrict self,
                    /*utf-8*/char const *__restrict text, size_t textlen) {
 uint32_t ch32; char *flush_start;
 size_t result = textlen;
 while (self->bp_numpend) {
  /* Complete a pending UTF-8 sequence. */
  uint8_t reqlen;
  if (!textlen) goto done;
  self->bp_pend[self->bp_numpend] = (uint8_t)*text++;
  reqlen = utf8_sequence_len[self->bp_pend[0]];
  ASSERT(self->bp_numpend+1 <= reqlen);
  if (self->bp_numpend+1 == reqlen) {
   /* Append the full character. */
   ch32 = utf8_getchar(self->bp_pend,reqlen);
   if unlikely(ch32 > 0xff)
      goto err_bytes_too_large;
   if unlikely(bytes_printer_putc(self,(char)(uint8_t)ch32))
      goto err;
   self->bp_numpend = 0;
   break;
  }
  ++self->bp_numpend;
 }
again_flush:
 flush_start = (char *)text;
 while (textlen && (unsigned char)*text < 0xc0) ++text,--textlen;
 /* Print ASCII text. */
 if (flush_start != text) {
  if unlikely(bytes_printer_append(self,
                                  (uint8_t *)flush_start,
                                  (size_t)(text-flush_start)) < 0)
     goto err;
 }
 if (textlen) {
  uint8_t seqlen;
  ASSERT((unsigned char)*text >= 0xc0);
  seqlen = utf8_sequence_len[(uint8_t)*text];
  if (seqlen > textlen) {
   /* Incomplete sequence! (safe as pending UTF-8) */
   memcpyb(self->bp_pend,text,textlen);
   self->bp_numpend = (uint8_t)textlen;
   goto done;
  }
  /* Expand the utf-8 sequence and emit it as a single character. */
  ch32 = utf8_getchar((uint8_t *)text,seqlen);
  if unlikely(ch32 > 0xff)
     goto err_bytes_too_large;
  if unlikely(bytes_printer_putc(self,(char)(uint8_t)ch32))
     goto err;
  text += seqlen;
  textlen -= seqlen;
  goto again_flush;
 }
done:
 return result;
err:
 return -1;
err_bytes_too_large:
 DeeError_Throwf(&DeeError_UnicodeEncodeError,
                 "Unicode character U+%.4I32X cannot fit into a single byte",
                 ch32);
 goto err;
}

PUBLIC int
(DCALL bytes_printer_putc)(struct bytes_printer *__restrict self, char ch) {
 /* Quick check: If the character is apart of the
  *              ASCII range, just append it as a byte. */
 if likely((uint8_t)ch < 0x80)
    return bytes_printer_putb(self,(uint8_t)ch);
 /* Print the character as a UTF-8 string. */
 return bytes_printer_print(self,&ch,1) < 0 ? -1 : 0;
}

PUBLIC dssize_t DCALL
Dee_FormatPrint8(dformatprinter printer, void *arg,
                 uint8_t const *__restrict text,
                 size_t textlen) {
 uint8_t const *iter,*end,*flush_start;
 dssize_t temp,result = 0; uint8_t utf8_buffer[2];
 if (printer == (dformatprinter)&unicode_printer_print)
     return unicode_printer_print8((struct unicode_printer *)arg,text,textlen);
 /* Ascii-only data can simply be forwarded one-on-one */
 end = (iter = flush_start = text) + textlen;
 for (; iter < end; ++iter) {
  if (*iter <= 0x7f) continue;
  /* Flush unwritten data. */
  if (flush_start < iter) {
   /* Flush the remainder. */
   temp = (*printer)(arg,(char const *)flush_start,(size_t)(iter - flush_start));
   if unlikely(temp < 0) goto err;
   result += temp;
  }
  /* Convert to a 2-wide multibyte UTF-8 character. */
  utf8_buffer[0] = 0xc0 | ((*iter & 0xc0) >> 6);
  utf8_buffer[1] = 0x80 | (*iter & 0x3f);
  temp = (*printer)(arg,(char *)utf8_buffer,2);
  if unlikely(temp < 0) goto err;
  result += temp;
  flush_start = iter + 1;
 }
 if (flush_start < end) {
  /* Flush the remainder. */
  temp = (*printer)(arg,(char const *)flush_start,(size_t)(end - flush_start));
  if unlikely(temp < 0) goto err;
  result += temp;
 }
 return result;
err:
 return temp;
}

PUBLIC dssize_t DCALL
Dee_FormatPrint16(dformatprinter printer, void *arg,
                  uint16_t const *__restrict text,
                  size_t textlen) {
 dssize_t temp,result = 0; uint8_t utf8_buffer[3]; size_t utf8_length;
 if (printer == (dformatprinter)&unicode_printer_print)
     return unicode_printer_print16((struct unicode_printer *)arg,text,textlen);
 while (textlen--) {
  uint16_t ch = *text++;
  if (ch <= UTF8_1BYTE_MAX) {
   utf8_buffer[0] = (uint8_t)ch;
   utf8_length    = 1;
  } else if (ch <= UTF8_2BYTE_MAX) {
   utf8_buffer[0] = 0xc0 | (uint8_t)((ch >> 6)/* & 0x1f*/);
   utf8_buffer[1] = 0x80 | (uint8_t)((ch) & 0x3f);
   utf8_length    = 2;
  } else {
   utf8_buffer[0] = 0xe0 | (uint8_t)((ch >> 12)/* & 0x0f*/);
   utf8_buffer[1] = 0x80 | (uint8_t)((ch >> 6) & 0x3f);
   utf8_buffer[2] = 0x80 | (uint8_t)((ch) & 0x3f);
   utf8_length    = 3;
  }
  temp = (*printer)(arg,(char const *)utf8_buffer,utf8_length);
  if unlikely(temp < 0) goto err;
  result += temp;
 }
 return result;
err:
 return temp;
}

PUBLIC dssize_t DCALL
Dee_FormatPrint32(dformatprinter printer, void *arg,
                  uint32_t const *__restrict text, size_t textlen) {
 dssize_t temp,result = 0; size_t utf8_length;
 uint8_t utf8_buffer[UTF8_MAX_MBLEN];
 if (printer == (dformatprinter)&unicode_printer_print)
     return unicode_printer_print32((struct unicode_printer *)arg,text,textlen);
 while (textlen--) {
  uint32_t ch = *text++;
  utf8_length = (size_t)((uint8_t *)utf8_writechar((char *)utf8_buffer,ch) - utf8_buffer);
  temp = (*printer)(arg,(char const *)utf8_buffer,utf8_length);
  if unlikely(temp < 0) goto err;
  result += temp;
 }
 return result;
err:
 return temp;
}



/* Helper functions for manipulating strings that have already been created. */
PUBLIC uint32_t
(DCALL DeeString_GetChar)(DeeStringObject *__restrict self, size_t index) {
 union dcharptr str; struct string_utf *utf;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 utf = self->s_data;
 if (!utf) {
  ASSERT(index <= self->s_len);
  return ((uint8_t *)self->s_str)[index];
 }
 str.ptr = utf->u_data[utf->u_width];
 ASSERT(index <= WSTR_LENGTH(str.ptr));
 SWITCH_SIZEOF_WIDTH(utf->u_width) {
 CASE_WIDTH_1BYTE: return str.cp8[index];
 CASE_WIDTH_2BYTE: return str.cp16[index];
 CASE_WIDTH_4BYTE: return str.cp32[index];
 }
}

PUBLIC void
(DCALL DeeString_SetChar)(DeeStringObject *__restrict self,
                          size_t index, uint32_t value) {
 union dcharptr str;
 struct string_utf *utf;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT(!DeeObject_IsShared(self));
 utf = self->s_data;
 if (!utf) {
  ASSERT((index < self->s_len) ||
         (index == self->s_len && !value));
  self->s_str[index] = (uint8_t)value;
 } else {
  str.ptr = utf->u_data[utf->u_width];
  ASSERT((index < WSTR_LENGTH(str.ptr)) ||
         (index == WSTR_LENGTH(str.ptr) && !value));
  SWITCH_SIZEOF_WIDTH(utf->u_width) {
  CASE_WIDTH_1BYTE:
   ASSERT(value <= 0xff);
   ASSERT(str.cp8 == (uint8_t *)DeeString_STR(self));
   str.cp8[index] = (uint8_t)value;
   if (utf->u_utf8 &&
       utf->u_utf8 != DeeString_STR(self)) {
    ASSERT(utf->u_utf8 != (char *)utf->u_data[STRING_WIDTH_1BYTE]);
    Dee_Free((size_t *)utf->u_utf8 - 1);
    utf->u_utf8 = NULL;
   }
   if (utf->u_data[STRING_WIDTH_2BYTE])
      ((uint16_t *)utf->u_data[STRING_WIDTH_2BYTE])[index] = (uint16_t)value;
   if (utf->u_data[STRING_WIDTH_4BYTE])
      ((uint32_t *)utf->u_data[STRING_WIDTH_4BYTE])[index] = (uint32_t)value;
   if (utf->u_utf16 &&
      (uint16_t *)utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE])
       utf->u_utf16[index] = (uint16_t)value;
   break;
  CASE_WIDTH_2BYTE:
   ASSERT(value <= 0xffff);
   str.cp16[index] = (uint16_t)value;
   if (utf->u_data[STRING_WIDTH_4BYTE]) {
    str.ptr = utf->u_data[STRING_WIDTH_4BYTE];
    str.cp32[index] = (uint32_t)value;
   }
   goto check_1byte;
  CASE_WIDTH_4BYTE:
   str.cp32[index] = (uint32_t)value;
   if (utf->u_data[STRING_WIDTH_2BYTE]) {
    if ((uint16_t *)utf->u_utf16 == (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE])
        utf->u_utf16 = NULL;
    Dee_Free(((size_t *)utf->u_data[STRING_WIDTH_2BYTE])-1);
    utf->u_data[STRING_WIDTH_2BYTE] = NULL;
   }
   if (utf->u_utf16) {
    ASSERT((uint16_t *)utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE]);
    Dee_Free((size_t *)utf->u_utf16 - 1);
    utf->u_utf16 = NULL;
   }
check_1byte:
   if (utf->u_data[STRING_WIDTH_1BYTE]) {
    /* String bytes data. */
    if (utf->u_data[STRING_WIDTH_1BYTE] == (size_t *)DeeString_STR(self)) {
     ASSERT(DeeString_SIZE(self) == WSTR_LENGTH(str.ptr));
     DeeString_STR(self)[index] = (char)(uint8_t)value; /* Data loss here cannot be prevented... */
     return;
    }
    if (utf->u_utf8 == (char *)utf->u_data[STRING_WIDTH_1BYTE])
        utf->u_utf8 = NULL;
    Dee_Free((size_t *)utf->u_data[STRING_WIDTH_1BYTE] - 1);
    utf->u_data[STRING_WIDTH_1BYTE] = NULL;
   }
   if (utf->u_utf8) {
    if (utf->u_utf8 == (char *)DeeString_STR(self)) {
     /* Must update the utf-8 representation. */
     if (DeeString_SIZE(self) == WSTR_LENGTH(str.ptr)) {
      /* No unicode characters. */
      DeeString_STR(self)[index] = (char)(uint8_t)value; /* Data loss here cannot be prevented... */
     } else {
      /* The difficult case. */
      size_t i = index;
      uint8_t *utf8_dst = (uint8_t *)DeeString_STR(self);
      while (i--) utf8_readchar_u((char const **)&utf8_dst);
      switch (utf8_sequence_len[*utf8_dst]) {
      default: *utf8_dst = (uint8_t)value; break;
      case 2:
       utf8_dst[0] = 0xc0 | (uint8_t)((value >> 6)/* & 0x1f*/);
       utf8_dst[1] = 0x80 | (uint8_t)((value) & 0x3f);
       break;
      case 3:
       utf8_dst[0] = 0xe0 | (uint8_t)((value >> 12)/* & 0x0f*/);
       utf8_dst[1] = 0x80 | (uint8_t)((value >> 6) & 0x3f);
       utf8_dst[2] = 0x80 | (uint8_t)((value) & 0x3f);
       break;
      case 4:
       utf8_dst[0] = 0xf0 | (uint8_t)((value >> 18)/* & 0x07*/);
       utf8_dst[1] = 0x80 | (uint8_t)((value >> 12) & 0x3f);
       utf8_dst[2] = 0x80 | (uint8_t)((value >> 6) & 0x3f);
       utf8_dst[3] = 0x80 | (uint8_t)((value) & 0x3f);
       break;
      case 5:
       utf8_dst[0] = 0xf8 | (uint8_t)((value >> 24)/* & 0x03*/);
       utf8_dst[1] = 0x80 | (uint8_t)((value >> 18) & 0x3f);
       utf8_dst[2] = 0x80 | (uint8_t)((value >> 12) & 0x3f);
       utf8_dst[3] = 0x80 | (uint8_t)((value >> 6) & 0x3f);
       utf8_dst[4] = 0x80 | (uint8_t)((value) & 0x3f);
       break;
      case 6:
       utf8_dst[0] = 0xfc | (uint8_t)((value >> 30)/* & 0x01*/);
       utf8_dst[1] = 0x80 | (uint8_t)((value >> 24) & 0x3f);
       utf8_dst[2] = 0x80 | (uint8_t)((value >> 18) & 0x3f);
       utf8_dst[3] = 0x80 | (uint8_t)((value >> 12) & 0x3f);
       utf8_dst[4] = 0x80 | (uint8_t)((value >> 6) & 0x3f);
       utf8_dst[5] = 0x80 | (uint8_t)((value) & 0x3f);
       break;
      case 7:
       utf8_dst[0] = 0xfe;
       utf8_dst[1] = 0x80 | (uint8_t)((value >> 30) & 0x03/* & 0x3f*/);
       utf8_dst[2] = 0x80 | (uint8_t)((value >> 24) & 0x3f);
       utf8_dst[3] = 0x80 | (uint8_t)((value >> 18) & 0x3f);
       utf8_dst[4] = 0x80 | (uint8_t)((value >> 12) & 0x3f);
       utf8_dst[5] = 0x80 | (uint8_t)((value >> 6) & 0x3f);
       utf8_dst[6] = 0x80 | (uint8_t)((value) & 0x3f);
       break;
      case 8:
       utf8_dst[0] = 0xff;
       utf8_dst[1] = 0x80;
       utf8_dst[2] = 0x80 | (uint8_t)((value >> 30) & 0x03/* & 0x3f*/);
       utf8_dst[3] = 0x80 | (uint8_t)((value >> 24) & 0x3f);
       utf8_dst[4] = 0x80 | (uint8_t)((value >> 18) & 0x3f);
       utf8_dst[5] = 0x80 | (uint8_t)((value >> 12) & 0x3f);
       utf8_dst[6] = 0x80 | (uint8_t)((value >> 6) & 0x3f);
       utf8_dst[7] = 0x80 | (uint8_t)((value) & 0x3f);
       break;
      }
     }
    } else {
     ASSERT(utf->u_utf8 != (char *)utf->u_data[STRING_WIDTH_1BYTE]);
     Dee_Free((size_t *)utf->u_utf8 - 1);
     utf->u_utf8 = NULL;
    }
   }
   break;
  }
 }
}

PUBLIC void
(DCALL DeeString_Memmove)(DeeStringObject *__restrict self,
                          size_t dst, size_t src, size_t num_chars) {
 union dcharptr str;
 struct string_utf *utf;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT(!DeeObject_IsShared(self));
 utf = self->s_data;
 if (!utf) {
  ASSERT((dst+num_chars) <= self->s_len+1);
  ASSERT((src+num_chars) <= self->s_len+1);
  memmove(self->s_str + dst,
          self->s_str + src,
          num_chars * sizeof(char));
 } else {
  str.ptr = utf->u_data[utf->u_width];
  ASSERT((dst+num_chars) <= WSTR_LENGTH(str.ptr));
  ASSERT((src+num_chars) <= WSTR_LENGTH(str.ptr));
  if (utf->u_utf8 &&
      utf->u_utf8 != (char *)DeeString_STR(self) &&
      utf->u_utf8 != (char *)utf->u_data[STRING_WIDTH_1BYTE]) {
   ASSERT(str.ptr != utf->u_utf8);
   Dee_Free(((size_t *)utf->u_utf8)-1);
   utf->u_utf8 = NULL;
  }
  if (utf->u_utf16 &&
     (uint16_t *)utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE]) {
   ASSERT(str.ptr != utf->u_utf16);
   Dee_Free(((size_t *)utf->u_utf16) - 1);
   utf->u_utf16 = NULL;
  }
  SWITCH_SIZEOF_WIDTH(utf->u_width) {
  CASE_WIDTH_1BYTE:
   memmove(str.cp8 + dst,str.cp8 + src,num_chars * 1);
   if (utf->u_data[STRING_WIDTH_2BYTE]) {
    str.ptr = utf->u_data[STRING_WIDTH_2BYTE];
    memmove(str.cp16 + dst,str.cp16 + src,num_chars * 2);
   }
   if (utf->u_data[STRING_WIDTH_4BYTE]) {
    str.ptr = utf->u_data[STRING_WIDTH_4BYTE];
    memmove(str.cp32 + dst,str.cp32 + src,num_chars * 4);
   }
   break;
  CASE_WIDTH_2BYTE:
   ASSERT(!utf->u_data[STRING_WIDTH_1BYTE]);
   memmove(str.cp16 + dst,str.cp16 + src,num_chars * 1);
   if (utf->u_data[STRING_WIDTH_4BYTE]) {
    str.ptr = utf->u_data[STRING_WIDTH_4BYTE];
    memmove(str.cp32 + dst,str.cp32 + src,num_chars * 4);
   }
   goto check_1byte;
  CASE_WIDTH_4BYTE:
   if (utf->u_data[STRING_WIDTH_2BYTE]) {
    if ((uint16_t *)utf->u_utf16 == (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE])
        utf->u_utf16 = NULL;
    Dee_Free(((size_t *)utf->u_data[STRING_WIDTH_2BYTE])-1);
    utf->u_data[STRING_WIDTH_2BYTE] = NULL;
   }
   if (utf->u_utf16) {
    ASSERT((uint16_t *)utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE]);
    Dee_Free((size_t *)utf->u_utf16 - 1);
    utf->u_utf16 = NULL;
   }
   memmove(str.cp32 + dst,str.cp32 + src,num_chars * 1);
check_1byte:
   if (utf->u_data[STRING_WIDTH_1BYTE]) {
    /* String bytes data. */
    if (utf->u_data[STRING_WIDTH_1BYTE] == (size_t *)DeeString_STR(self)) {
     ASSERT(DeeString_SIZE(self) == WSTR_LENGTH(str.ptr));
     memmove(DeeString_STR(self) + dst,
             DeeString_STR(self) + src,num_chars * sizeof(char));
     return;
    }
    if (utf->u_utf8 == (char *)utf->u_data[STRING_WIDTH_1BYTE])
        utf->u_utf8 = NULL;
    Dee_Free((size_t *)utf->u_data[STRING_WIDTH_1BYTE] - 1);
    utf->u_data[STRING_WIDTH_1BYTE] = NULL;
   }
   if (utf->u_utf8) {
    if (utf->u_utf8 == (char *)DeeString_STR(self)) {
     /* Must update the utf-8 representation. */
     if (DeeString_SIZE(self) == WSTR_LENGTH(str.ptr)) {
      /* No unicode character. -> We can simply memmove the UTF-8 variable to update it. */
      memmove(DeeString_STR(self) + dst,
              DeeString_STR(self) + src,num_chars * sizeof(char));
     } else {
      /* The difficult case. */
      char *utf8_src,*utf8_dst,*end;
      size_t i;
      if (dst < src) {
       i = dst;
       utf8_dst = DeeString_STR(self);
       while (i--) utf8_readchar_u((char const **)&utf8_dst);
       utf8_src = utf8_dst;
       i = src - dst;
       while (i--) utf8_readchar_u((char const **)&utf8_src);
       end = utf8_src;
       i = num_chars;
       while (i--) utf8_readchar_u((char const **)&end);
      } else {
       i = dst;
       utf8_src = DeeString_STR(self);
       while (i--) utf8_readchar_u((char const **)&utf8_src);
       utf8_dst = utf8_src;
       i = dst - src;
       if (num_chars > i) {
        while (i--) utf8_readchar_u((char const **)&utf8_dst);
        i = num_chars - (dst - src);
        end = utf8_dst;
        while (i--) utf8_readchar_u((char const **)&end);
       } else {
        end = NULL;
        while (i--) {
         if (!num_chars--)
              end = utf8_dst;
         utf8_readchar_u((char const **)&utf8_dst);
        }
        ASSERT(end != NULL);
       }
      }
      memmove(utf8_dst,utf8_src,(size_t)(end - utf8_src));
     }
    } else {
     ASSERT(utf->u_utf8 != (char *)utf->u_data[STRING_WIDTH_1BYTE]);
     Dee_Free((size_t *)utf->u_utf8 - 1);
     utf->u_utf8 = NULL;
    }
   }
   break;
  }
 }
}

#define DEE_PRIVATE_WSTR_DECLEN(x) \
   (ASSERT(WSTR_LENGTH(x)),--WSTR_LENGTH(x),(x)[WSTR_LENGTH(x)] = 0)

PUBLIC void
(DCALL DeeString_PopbackAscii)(DeeStringObject *__restrict self) {
 struct string_utf *utf;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT(!DeeObject_IsShared(self));
 utf = self->s_data;
 DEE_PRIVATE_WSTR_DECLEN(self->s_str);
 if (utf) {
  if (utf->u_data[STRING_WIDTH_1BYTE] &&
      utf->u_data[STRING_WIDTH_1BYTE] != (size_t *)DeeString_STR(self))
      DEE_PRIVATE_WSTR_DECLEN((uint8_t *)utf->u_data[STRING_WIDTH_1BYTE]);
  if (utf->u_data[STRING_WIDTH_2BYTE])
      DEE_PRIVATE_WSTR_DECLEN((uint16_t *)utf->u_data[STRING_WIDTH_2BYTE]);
  if (utf->u_data[STRING_WIDTH_4BYTE])
      DEE_PRIVATE_WSTR_DECLEN((uint32_t *)utf->u_data[STRING_WIDTH_4BYTE]);
  if (utf->u_utf8 &&
      utf->u_utf8 != (char *)utf->u_data[STRING_WIDTH_1BYTE] &&
      utf->u_utf8 != DeeString_STR(self))
      DEE_PRIVATE_WSTR_DECLEN(utf->u_utf8);
  if (utf->u_utf16 &&
     (uint16_t *)utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE])
      DEE_PRIVATE_WSTR_DECLEN(utf->u_utf16);
 }
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_C */
