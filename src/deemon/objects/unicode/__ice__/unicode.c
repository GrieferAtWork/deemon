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
#include <deemon/tuple.h>
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

#ifndef CONFIG_NO_THREADS
#include <hybrid/atomic.h>
#endif

#ifdef CONFIG_WCHAR_STRINGS
#ifdef CONFIG_HOST_WINDOWS
#    include <Windows.h>
#else /* CONFIG_HOST_WINDOWS */
#    include <locale.h>
#    include <wchar.h>
#ifndef CONFIG_NO_STDIO
#    include <stdio.h>
#endif /* !CONFIG_NO_STDIO */
#endif /* !CONFIG_HOST_WINDOWS */
#endif /* CONFIG_WCHAR_STRINGS */

DECL_BEGIN

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

INTERN DREF DeeObject *DCALL
DeeString_New1Char(uint8_t ch) {
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




#ifdef CONFIG_USE_NEW_STRING_API

PRIVATE uint8_t const utf8_trailing_bytes[256] = {
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5};

PRIVATE uint8_t const utf8_sequence_len[256] = {
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


PRIVATE uint16_t *DCALL
utf8_to_utf16(uint8_t *__restrict str, size_t length) {
 uint16_t *result,*dst; size_t i;
 result = DeeString_NewUtf16Buffer(length);
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
  } else if (ch32 >= 0x10ffff) {
   *dst++ = '?'; /* Invalid utf-16 character (unicode character is too large) */
  } else {
   /* Character needs a low, and a high surrogate. */
   uint16_t high,low;
   ch32 -= 0x10000;
   high = UTF16_HIGH_SURROGATE_MIN+(uint16_t)(ch32 >> 10);
   low  = UTF16_LOW_SURROGATE_MIN+(uint16_t)(ch32 & 0x3ff);
   *dst++ = high;
   *dst++ = low;
  }
 }
 *dst = 0;
 i = (size_t)(dst-result);
 if (i != length) {
  dst = DeeString_TryResizeUtf16Buffer(result,i);
  if likely(dst) result = dst;
 }
 return result;
err:
 return NULL;
}

PRIVATE uint32_t *DCALL
utf8_to_utf32(uint8_t *__restrict str, size_t length) {
 uint32_t *result,*dst; size_t i;
 result = DeeString_NewUtf32Buffer(length);
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
  dst = DeeString_TryResizeUtf32Buffer(result,i);
  if likely(dst) result = dst;
 }
 return result;
err:
 return NULL;
}

#ifdef CONFIG_HOST_WINDOWS
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
#else
#error TODO
#endif




PRIVATE uint16_t *DCALL
utf8_to_utf16_raw(uint8_t *__restrict src, size_t src_len,
                  uint16_t *__restrict dst) {
 size_t i = 0;
 while (i < src_len) {
  uint8_t ch = src[i];
  uint8_t len = utf8_sequence_len[ch];
  uint16_t ch16;
  ASSERT(len != 0);
  ASSERT(len <= 3);
  switch (len) {
  case 1:
   *dst++ = ch;
   ++i;
   continue;
  case 2:
   ASSERT((src[i+1] & 0xc0) == 0x80);
   ch16  = (ch & 0x1f) << 6;
   ch16 |= (src[i+1] & 0x3f);
   *dst++ = ch16;
   i += 2;
   break;
  case 3:
   ASSERT((src[i+1] & 0xc0) == 0x80);
   ASSERT((src[i+2] & 0xc0) == 0x80);
   ch16  = (ch & 0x0f) << 12;
   ch16 |= (src[i+1] & 0x3f) << 6;
   ch16 |= (src[i+2] & 0x3f);
   *dst++ = ch16;
   i += 3;
   break;
  default: __builtin_unreachable();
  }
 }
 ASSERT(i == src_len);
 return dst;
}
PRIVATE uint32_t *DCALL
utf8_to_utf32_raw(uint8_t *__restrict src, size_t src_len,
                  uint32_t *__restrict dst) {
 size_t i = 0;
 while (i < src_len) {
  uint8_t ch = src[i];
  uint8_t len = utf8_sequence_len[ch];
  uint32_t ch32;
  ASSERT(len != 0);
  switch (len) {
  case 1:
   *dst++ = ch;
   ++i;
   continue;
  case 2:
   ASSERT((src[i+1] & 0xc0) == 0x80);
   ch32  = (ch & 0x1f) << 6;
   ch32 |= (src[i+1] & 0x3f);
   *dst++ = ch32;
   i += 2;
   break;
  case 3:
   ASSERT((src[i+1] & 0xc0) == 0x80);
   ASSERT((src[i+2] & 0xc0) == 0x80);
   ch32  = (ch & 0x0f) << 12;
   ch32 |= (src[i+1] & 0x3f) << 6;
   ch32 |= (src[i+2] & 0x3f);
   *dst++ = ch32;
   i += 3;
   break;
  case 4:
   ASSERT((src[i+1] & 0xc0) == 0x80);
   ASSERT((src[i+2] & 0xc0) == 0x80);
   ASSERT((src[i+3] & 0xc0) == 0x80);
   ch32  = (ch & 0x07) << 18;
   ch32 |= (src[i+1] & 0x3f) << 12;
   ch32 |= (src[i+2] & 0x3f) << 6;
   ch32 |= (src[i+3] & 0x3f);
   *dst++ = ch32;
   i += 4;
   break;
  case 5:
   ASSERT((src[i+1] & 0xc0) == 0x80);
   ASSERT((src[i+2] & 0xc0) == 0x80);
   ASSERT((src[i+3] & 0xc0) == 0x80);
   ASSERT((src[i+4] & 0xc0) == 0x80);
   ch32  = (ch & 0x03) << 24;
   ch32 |= (src[i+1] & 0x3f) << 18;
   ch32 |= (src[i+2] & 0x3f) << 12;
   ch32 |= (src[i+3] & 0x3f) << 6;
   ch32 |= (src[i+4] & 0x3f);
   *dst++ = ch32;
   i += 5;
   break;
  case 6:
   ASSERT((src[i+1] & 0xc0) == 0x80);
   ASSERT((src[i+2] & 0xc0) == 0x80);
   ASSERT((src[i+3] & 0xc0) == 0x80);
   ASSERT((src[i+4] & 0xc0) == 0x80);
   ASSERT((src[i+5] & 0xc0) == 0x80);
   ch32  = (ch & 0x01) << 30;
   ch32 |= (src[i+1] & 0x3f) << 24;
   ch32 |= (src[i+2] & 0x3f) << 18;
   ch32 |= (src[i+3] & 0x3f) << 12;
   ch32 |= (src[i+4] & 0x3f) << 6;
   ch32 |= (src[i+5] & 0x3f);
   *dst++ = ch32;
   i += 6;
   break;
  case 7:
   ASSERT((src[i+1] & 0xc0) == 0x80);
   ASSERT((src[i+2] & 0xc0) == 0x80);
   ASSERT((src[i+3] & 0xc0) == 0x80);
   ASSERT((src[i+4] & 0xc0) == 0x80);
   ASSERT((src[i+5] & 0xc0) == 0x80);
   ASSERT((src[i+6] & 0xc0) == 0x80);
   ch32  = (src[i+1] & 0x03/*0x3f*/) << 30;
   ch32 |= (src[i+2] & 0x3f) << 24;
   ch32 |= (src[i+3] & 0x3f) << 18;
   ch32 |= (src[i+4] & 0x3f) << 12;
   ch32 |= (src[i+5] & 0x3f) << 6;
   ch32 |= (src[i+6] & 0x3f);
   *dst++ = ch32;
   i += 7;
   break;
  case 8:
   ASSERT((src[i+1] & 0xc0) == 0x80);
   ASSERT((src[i+2] & 0xc0) == 0x80);
   ASSERT((src[i+3] & 0xc0) == 0x80);
   ASSERT((src[i+4] & 0xc0) == 0x80);
   ASSERT((src[i+5] & 0xc0) == 0x80);
   ASSERT((src[i+6] & 0xc0) == 0x80);
   ASSERT((src[i+7] & 0xc0) == 0x80);
   /*ch32 = (src[i+1] & 0x3f) << 36;*/
   ch32  = (src[i+2] & 0x03/*0x3f*/) << 30;
   ch32 |= (src[i+3] & 0x3f) << 24;
   ch32 |= (src[i+4] & 0x3f) << 18;
   ch32 |= (src[i+5] & 0x3f) << 12;
   ch32 |= (src[i+6] & 0x3f) << 6;
   ch32 |= (src[i+7] & 0x3f);
   *dst++ = ch32;
   i += 8;
   break;
  default: __builtin_unreachable();
  }
 }
 ASSERT(i == src_len);
 return dst;
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
DeeString_PackUtf16Buffer(/*inherit(always)*/uint16_t *__restrict text,
                          unsigned int error_mode) {
 size_t i,length,utf8_length;
 DREF String *result;
 ASSERT(text);
 length = ((size_t *)text)[-1];
 if unlikely(!length) {
  Dee_Free((size_t *)text-1);
  return_empty_string;
 }
 text[length] = 0;
 utf8_length = 0;
 i = 0;
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
    if (error_mode & DEE_STRING_CODEC_FREPLAC) {
     text[i] = '?';
     break;
    }
    if (error_mode & DEE_STRING_CODEC_FIGNORE) {
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
    if (error_mode & DEE_STRING_CODEC_FREPLAC) {
     text[i-1] = '?';
     --length;
     memmove(&text[i],&text[i+1],(length-i)*sizeof(uint16_t));
     goto read_text_i;
    }
    if (error_mode & DEE_STRING_CODEC_FIGNORE) {
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
  }
  ASSERT(ch <= 0x10FFFF);
  if (ch <= UTF8_1BYTE_MAX) {
   utf8_length += 1;
  } else if (ch <= UTF8_2BYTE_MAX) {
   utf8_length += 2;
  } else if (ch <= UTF8_3BYTE_MAX) {
   utf8_length += 3;
  } else {
   utf8_length += 4;
  }
 }
 ASSERT(utf8_length >= length);
 result = (DREF String *)DeeObject_Malloc(COMPILER_OFFSETOF(String,s_str)+
                                         (utf8_length+1)*sizeof(char));
 if unlikely(!result) goto err;
 result->s_len = utf8_length;
 result->s_data = utf_alloc();
 if unlikely(!result->s_data) goto err_r;
 memset(result->s_data,0,sizeof(struct string_utf));
 result->s_data->u_width = utf8_length == length
                         ? STRING_WIDTH_1BYTE
                         : STRING_WIDTH_2BYTE
                         ;
 result->s_data->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
 result->s_data->u_data[STRING_WIDTH_2BYTE] = (size_t *)text; /* Inherit data */
 result->s_hash = DEE_STRING_HASH_UNSET;
 utf16_to_utf8(text,length,(uint8_t *)result->s_str);
 result->s_str[utf8_length] = '\0';
 return (DREF DeeObject *)result;
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
 DREF String *result;
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
   if (error_mode & DEE_STRING_CODEC_FREPLAC) {
    /* Replace with a question mark. */
    text[i] = '?';
    utf8_length += 1;
   } else if (error_mode & DEE_STRING_CODEC_FIGNORE) {
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
 result->s_data = utf_alloc();
 if unlikely(!result->s_data) goto err_r;
 memset(result->s_data,0,sizeof(struct string_utf));
 result->s_data->u_width = utf8_length == length
                         ? STRING_WIDTH_1BYTE
                           /* The string might fit into 2-bytes, but
                            * we already have the 4-byte variant... */
                         : STRING_WIDTH_4BYTE
                         ;
 result->s_data->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
 result->s_data->u_data[STRING_WIDTH_4BYTE] = (size_t *)text; /* Inherit data */
 result->s_hash = DEE_STRING_HASH_UNSET;
 utf32_to_utf8(text,length,(uint8_t *)result->s_str);
 result->s_str[utf8_length] = '\0';
 return (DREF DeeObject *)result;
err_r:
 DeeObject_Free(result);
err:
 Dee_Free((size_t *)text-1);
 return NULL;
}
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
 if (!(error_mode&(DEE_STRING_CODEC_FIGNORE|
                   DEE_STRING_CODEC_FREPLAC)))
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
   if (error_mode & (DEE_STRING_CODEC_FIGNORE|DEE_STRING_CODEC_FREPLAC)) {
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
 else result->s_data->u_width = STRING_WIDTH_WCHAR;
 result->s_data->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
 result->s_data->u_flags                    = STRING_UTF_FNORMAL;
 result->s_data->u_data[STRING_WIDTH_WCHAR] = (size_t *)text; /* Inherit data. */
 result->s_len                              = mbcs_length;
 result->s_hash                             = DEE_STRING_HASH_UNSET;
 result->s_str[mbcs_length]                 = '\0';
 DeeObject_Init(result,&DeeString_Type);
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



/* After having populated a single-byte string buffer, interpret its data (`DeeString_STR()')
 * in accordance to `codec' and initialize `s_data' as appropriate for the strings content.
 * e.g.: `DeeString_NewBuffer()' may have been used to construct a UTF-8 string,
 *        after which `DeeString_SetBufferEncoding(...,DEE_STRING_CODEC_UTF8)' should
 *        be called.
 *        Doing so will then go through the string and check if it contains any bytes
 *        outside the ASCII character range, which are then interpreted as unicode
 *        sequences to either fill in the `STRING_WIDTH_4BYTE', or the `STRING_WIDTH_2BYTE'
 *        fields, dependent on what the greatest unicode character found was.
 * @param: codec: One of `DEE_STRING_CODEC_*', optionally be or'd with `DEE_STRING_CODEC_F*'
 * @return:  0: Successfully set the buffer's encoding.
 * @return: -1: An error occurred. */
PUBLIC int DCALL
DeeString_SetBufferEncoding(DeeObject *__restrict self,
                            unsigned int codec) {
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT(codec < DEE_STRING_CODEC_COUNT);
 switch (codec) {

 case DEE_STRING_CODEC_LATIN1:
  return 0; /* Nothing to do here! */

 {
  size_t i,length; uint8_t *str;
 case DEE_STRING_CODEC_ASCII:
  /* Assert that there are no characters > 127 */
  str    = (uint8_t *)DeeString_STR(self);
  length = DeeString_SIZE(self);
  i = 0;
continue_ascii_fast:
  for (; i < length; ++i) {
   if (str[i] <= 0x7f) continue;
   if (codec & DEE_STRING_CODEC_FIGNORE) {
    --DeeString_SIZE(self);
    --length;
    memmove(&str[i],&str[i+1],(length-i)*sizeof(char));
    str[length] = '\0';
    goto continue_ascii_fast;
   }
   if (codec & DEE_STRING_CODEC_FREPLAC) {
    str[i] = '?';
    continue;
   }
   DeeError_Throwf(&DeeError_UnicodeEncodeError,
                   "Non-ascii character character U+%.4X",
                  (unsigned int)str[i]);
   goto err;
  }
  return 0;
 }

 {
  size_t i,length; uint8_t *str;
 case DEE_STRING_CODEC_UTF8:
  /* Decode a UTF-8 string.
   * If the string doesn't contain any characters > 255, don't do anything.
   * If all characters fit into U+0000 ... U+FFFF, use a 2-byte string.
   * Otherwise, use a 4-byte string. */
  str    = (uint8_t *)DeeString_STR(self);
  length = DeeString_SIZE(self);
  i = 0;
continue_utf8_fast:
  for (; i < length; ++i) {
   uint8_t ch = str[i];
   uint8_t chlen = utf8_sequence_len[ch];
   size_t j,unicode_length;
   struct string_utf *utf;
   bool unicode_is_32bit;
   if (chlen == 1) continue; /* ASCII character */
   if unlikely(chlen == 0) {
    /* Unicode follow-up character (invalid on its own) */
    if (codec & DEE_STRING_CODEC_FIGNORE) {
     --DeeString_SIZE(self);
     --length;
     memmove(&str[i],&str[i+1],(length-i)*sizeof(char));
     str[length] = '\0';
     goto continue_utf8_fast;
    }
    if (codec & DEE_STRING_CODEC_FREPLAC) {
     str[i] = '?';
     continue;
    }
err_utf8_invalid_follow_up:
    DeeError_Throwf(&DeeError_UnicodeEncodeError,
                    "UTF-8 follow-up byte 0x%.2X in invalid location",
                   (unsigned int)str[i]);
    goto err;
   }
   /* Found a unicode character. */
   unicode_length   = j = i;
   unicode_is_32bit = chlen > 3;
   /* Parse the remainder of the string in respect to unicode characters. */
   for (;;) {
    size_t ch_start = j;
    ASSERT(chlen != 0);
    ASSERT(j < length);
    if (j+chlen > length) {
     if (codec & DEE_STRING_CODEC_FIGNORE) {
      DeeString_SIZE(self) = j;
      length = j;
      str[length] = '\0';
      break;
     }
     if (codec & DEE_STRING_CODEC_FREPLAC) {
      str[j] = '?';
      chlen = 1;
     } else {
      DeeError_Throwf(&DeeError_UnicodeEncodeError,
                      "Incomplete UTF-8 sequence (missing %Iu bytes)",
                      j+chlen-length);
      goto err;
     }
    }
    ASSERT(chlen);
    while ((++j,--chlen)) {
     if ((str[j] & 0xc0) != 0x80) {
      /* Invalid follow-up byte. */
      if (codec & DEE_STRING_CODEC_FIGNORE) {
       chlen   = (uint8_t)((j-ch_start)+1);
       j       = ch_start;
       length -= chlen;
       memmove(&str[j],&str[j+chlen],(length-j)*sizeof(char));
       str[length] = '\0';
       goto utf8_unicode_read_j_check;
      }
      if (codec & DEE_STRING_CODEC_FREPLAC) {
       chlen   = (uint8_t)(j-ch_start);
       j       = ch_start;
       length -= chlen;
       memmove(&str[j+1],&str[j+chlen],(length-j)*sizeof(char));
       str[ch_start] = '?';
       str[length] = '\0';
       ASSERT(j < length);
       goto utf8_unicode_read_j;
      }
      DeeError_Throwf(&DeeError_UnicodeEncodeError,
                      "Missing UTF-8 follow-up byte #%u is 0x%.2X, which doesn't match 0b10xxxxxx",
                     (unsigned int)(j-(ch_start+1)),
                     (unsigned int)str[j]);
      goto err;
     }
    }
    ++unicode_length;
utf8_unicode_read_j_check:
    if (j >= length) break;
utf8_unicode_read_j:
    ch    = str[j];
    chlen = utf8_sequence_len[ch];
    if (!chlen) {
     if (codec & DEE_STRING_CODEC_FIGNORE) {
      --DeeString_SIZE(self);
      --length;
      memmove(&str[j],&str[j+1],(length-j)*sizeof(char));
      str[length] = '\0';
      goto utf8_unicode_read_j;
     }
     if (codec & DEE_STRING_CODEC_FREPLAC) {
      str[j] = '?';
      chlen = 1;
     } else {
      i = j;
      goto err_utf8_invalid_follow_up;
     }
    }
    if (chlen > 3)
        unicode_is_32bit = true;
   }
   /* Create the 2-byte/4-byte unicode string. */
   ASSERT(!DeeObject_IsShared(self));
   ASSERT(!((String *)self)->s_data);
   utf = utf_alloc();
   if unlikely(!utf) goto err;
   memset(utf,0,sizeof(struct string_utf));
   if (unicode_is_32bit) {
    uint32_t *encoded_string;
    encoded_string = DeeString_NewUtf32Buffer(unicode_length);
    if unlikely(!encoded_string) goto err;
    /* The first couple of characters are pure UTF-8 */
    for (j = 0; j < i; ++j)
        encoded_string[j] = str[j];
    utf8_to_utf32_raw(str+i,length-i,encoded_string+i);
    encoded_string[unicode_length] = 0;
    utf->u_width = STRING_WIDTH_4BYTE;
    utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)encoded_string;
   } else {
    uint16_t *encoded_string;
    encoded_string = DeeString_NewUtf16Buffer(unicode_length);
    if unlikely(!encoded_string) goto err;
    /* The first couple of characters are pure UTF-8 */
    for (j = 0; j < i; ++j)
        encoded_string[j] = str[j];
    utf8_to_utf16_raw(str+i,length-i,encoded_string+i);
    encoded_string[unicode_length] = 0;
    utf->u_width = STRING_WIDTH_2BYTE;
    utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)encoded_string;
   }
   utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)((String *)self)->s_str;
   ((String *)self)->s_data = utf;
   break;
  }
  return 0;
 }

 {
  struct string_utf *data;
 case DEE_STRING_CODEC_MBCS:
  if (DeeString_IsEmpty(self)) return 0;
  ASSERT(!DeeObject_IsShared(self));
  ASSERT(!((String *)self)->s_data);
  data = utf_alloc();
  if unlikely(!data) goto err;
  memset(data,0,sizeof(struct string_utf));
  data->u_width = STRING_WIDTH_WCHAR;
  data->u_data[STRING_WIDTH_WCHAR] = (size_t *)mbcs_to_wide((uint8_t *)DeeString_STR(self),
                                                             DeeString_SIZE(self));
  if unlikely(!data->u_data[STRING_WIDTH_WCHAR]) { utf_free(data); goto err; }
  data->u_data[STRING_WIDTH_1BYTE] = (size_t *)((String *)self)->s_str;
  ((String *)self)->s_data = data;
  return 0;
 }

 default: __builtin_unreachable();
 }
err:
 return -1;
}

PUBLIC void *DCALL
DeeString_AsWidth(DeeObject *__restrict self, unsigned int width) {
 struct string_utf *utf; size_t *result;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT(width < STRING_WIDTH_COUNT);
 if (width == STRING_WIDTH_1BYTE)
     return DeeString_STR(self);
again:
 utf = ((String *)self)->s_data;
 /* Generate missing width-representations of the string.
  * NOTE: Remember that for this we must convert WSTR, not STR,
  *       as STR is just the multi-byte string, while WSTR contains
  *       the actual characters!
  * e.g.: The 4-byte variant of a 1-byte string is simply an
  *       up-cast of all the 1-byte characters, rather than
  *       needing to use an actual unicode version! */
 if (utf) {
  result = utf->u_data[width];
  if (result)
      return result;
  if (utf->u_width == STRING_WIDTH_1BYTE)
      goto cast_from_1byte;
 } else {
  size_t i,length;
  utf = utf_alloc();
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
cast_from_1byte:
  length = DeeString_SIZE(self);
  SWITCH_SIZEOF_WIDTH(width) {
  CASE_WIDTH_2BYTE:
   result = (size_t *)Dee_Malloc(sizeof(size_t)+
                                (length+1)*2);
   if unlikely(!result) goto err;
   *result++ = length;
   for (i = 0; i < length; ++i)
      ((uint16_t *)result)[i] = (uint16_t)(uint8_t)DeeString_STR(self)[i];
   ((uint16_t *)result)[length] = 0;
   if (!ATOMIC_CMPXCH(utf->u_data[width],NULL,result)) {
    Dee_Free(result-1);
    goto again;
   }
   break;
  CASE_WIDTH_4BYTE:
   result = (size_t *)Dee_Malloc(sizeof(size_t)+
                                (length+1)*4);
   if unlikely(!result) goto err;
   *result++ = length;
   for (i = 0; i < length; ++i)
      ((uint32_t *)result)[i] = (uint32_t)(uint8_t)DeeString_STR(self)[i];
   ((uint32_t *)result)[length] = 0;
   if (!ATOMIC_CMPXCH(utf->u_data[width],NULL,result)) {
    Dee_Free(result-1);
    goto again;
   }
   break;
  default: __builtin_unreachable();
  }
  return result;
 }

 ASSERT(utf != NULL);
 ASSERT(utf->u_width != width);
 ASSERT(utf->u_width != STRING_WIDTH_1BYTE);
 ASSERT(width        != STRING_WIDTH_1BYTE);
 switch (width) {

 case STRING_WIDTH_2BYTE:
  ASSERT(utf->u_width == STRING_WIDTH_4BYTE ||
         utf->u_width == STRING_WIDTH_WCHAR);
  /* The 1-byte variant is encoded in UTF-8. - So create the
   * 2-byte variant by doing a `utf8_to_utf16' conversion. */
  result = (size_t *)utf8_to_utf16((uint8_t *)DeeString_STR(self),
                                    DeeString_SIZE(self));
  if unlikely(!result) goto err;
  if (!ATOMIC_CMPXCH(utf->u_data[STRING_WIDTH_2BYTE],NULL,result)) {
   Dee_Free(result-1);
   goto again;
  }
  break;

 case STRING_WIDTH_4BYTE:
  ASSERT(utf->u_width == STRING_WIDTH_2BYTE ||
         utf->u_width == STRING_WIDTH_WCHAR);
  if (utf->u_width == STRING_WIDTH_2BYTE) {
   uint16_t *str; size_t i,length;
   /* Can simply up-cast the utf16 string.
    * The fact that our own string's UTF width is 2 bytes means
    * that all characters fit into U+0000 ... U+FFFF (excluding surrogates),
    * which we can directly map onto the utf32 variant. */
   str = (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE];
   ASSERT(str);
   length = ((size_t *)str)[-1];
   result = (size_t *)Dee_Malloc(sizeof(size_t)+(length+1)*4);
   if unlikely(!result) goto err;
   *result++ = length;
   for (i = 0; i < length; ++i)
      ((uint32_t *)result)[i] = (uint32_t)str[i];
   ((uint32_t *)result)[length] = 0;
   if (!ATOMIC_CMPXCH(utf->u_data[STRING_WIDTH_4BYTE],NULL,result)) {
    Dee_Free(result-1);
    goto again;
   }
  } else {
   /* Must convert the utf-8 string stored in the 1-byte variant to utf-32 */
   result = (size_t *)utf8_to_utf32((uint8_t *)DeeString_STR(self),
                                     DeeString_SIZE(self));
   if unlikely(!result) goto err;
   if (!ATOMIC_CMPXCH(utf->u_data[STRING_WIDTH_4BYTE],NULL,result)) {
    Dee_Free(result-1);
    goto again;
   }
  }
  break;

 case STRING_WIDTH_WCHAR:
  ASSERT(utf->u_width == STRING_WIDTH_2BYTE ||
         utf->u_width == STRING_WIDTH_4BYTE);
  ASSERT(!DeeString_IsEmpty(self));
  ASSERT(DeeString_SIZE(self) != 0);
  /* The 1-byte variant is a UTF-8 string. */
  result = (size_t *)mbcs_to_wide((uint8_t *)DeeString_STR(self),
                                   DeeString_SIZE(self));
  if unlikely(!result) goto err;
  if (!ATOMIC_CMPXCH(utf->u_data[STRING_WIDTH_WCHAR],NULL,result)) {
   Dee_Free(result-1);
   goto again;
  }
  break;

 default: __builtin_unreachable();
 }
 return result;
err:
 return NULL;
}

DFUNDEF DREF DeeObject *DCALL
DeeString_NewWithCodec(unsigned char const *__restrict str,
                       size_t length, unsigned int codec) {
 DREF DeeObject *result;
 result = DeeString_NewSized((char *)str,length);
 if (likely(result) && result != Dee_EmptyString &&
     unlikely(DeeString_SetBufferEncoding(result,codec)))
     Dee_Clear(result);
 return result;
}

PUBLIC DREF DeeObject *DCALL DeeString_New2Char(uint16_t ch) {
 uint16_t *buffer;
 if (ch < 0xff)
     return DeeString_New1Char((uint8_t)ch);
 buffer = DeeString_NewUtf16Buffer(1);
 if unlikely(!buffer) return NULL;
 buffer[0] = ch;
 return DeeString_PackUtf16Buffer(buffer,DEE_STRING_CODEC_FREPLAC);
}
PUBLIC DREF DeeObject *DCALL DeeString_New4Char(uint32_t ch) {
 uint32_t *buffer;
 if (ch < 0xff)
     return DeeString_New1Char((uint8_t)ch);
 if (ch < 0xffff)
     return DeeString_New2Char((uint16_t)ch);
 buffer = DeeString_NewUtf32Buffer(1);
 if unlikely(!buffer) return NULL;
 buffer[0] = ch;
 return DeeString_PackUtf32Buffer(buffer,DEE_STRING_CODEC_FREPLAC);
}


INTERN DREF DeeObject *DCALL
DeeString_Convert(DeeObject *__restrict self, uintptr_t kind) {
 unsigned int width; void *str,*result;
 size_t i,length;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 width  = DeeString_WIDTH(self);
 str    = DeeString_WSTR(self);
 length = WSTR_LENGTH(str);
 result = DeeString_NewWidthBuffer(length,width);
 if unlikely(!result) return NULL;
 SWITCH_SIZEOF_WIDTH(width) {
 CASE_WIDTH_1BYTE:
  for (i = 0; i < length; ++i)
     ((uint8_t *)result)[i] = (uint8_t)DeeUni_Convert(((uint8_t *)str)[i],kind);
  break;
 CASE_WIDTH_2BYTE:
  for (i = 0; i < length; ++i)
     ((uint16_t *)result)[i] = (uint16_t)DeeUni_Convert(((uint16_t *)str)[i],kind);
  break;
 CASE_WIDTH_4BYTE:
  for (i = 0; i < length; ++i)
     ((uint32_t *)result)[i] = (uint32_t)DeeUni_Convert(((uint32_t *)str)[i],kind);
  break;
 }
 return DeeString_PackWidthBuffer(result,width,DEE_STRING_CODEC_FREPLAC);
}

INTERN DREF DeeObject *DCALL
DeeString_ToTitle(DeeObject *__restrict self) {
 uintptr_t kind = UNICODE_CONVERT_TITLE;
 unsigned int width; void *str,*result;
 size_t i,length;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 width  = DeeString_WIDTH(self);
 str    = DeeString_WSTR(self);
 length = WSTR_LENGTH(str);
 if (!length) return_empty_string;
 result = DeeString_NewWidthBuffer(length,width);
 if unlikely(!result) return NULL;
 SWITCH_SIZEOF_WIDTH(width) {
 CASE_WIDTH_1BYTE:
  for (i = 0; i < length; ++i) {
   uint8_t ch = ((uint8_t *)str)[i];
   struct unitraits *desc = DeeUni_Descriptor(ch);
   ((uint8_t *)result)[i] = (uint8_t)(ch + *(int32_t *)((uintptr_t)desc + kind));
   kind = (desc->ut_flags & UNICODE_FSPACE) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
  }
  break;
 CASE_WIDTH_2BYTE:
  for (i = 0; i < length; ++i) {
   uint16_t ch = ((uint16_t *)str)[i];
   struct unitraits *desc = DeeUni_Descriptor(ch);
   ((uint16_t *)result)[i] = (uint16_t)(ch + *(int32_t *)((uintptr_t)desc + kind));
   kind = (desc->ut_flags & UNICODE_FSPACE) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
  }
  break;
 CASE_WIDTH_4BYTE:
  for (i = 0; i < length; ++i) {
   uint32_t ch = ((uint32_t *)str)[i];
   struct unitraits *desc = DeeUni_Descriptor(ch);
   ((uint32_t *)result)[i] = (uint32_t)(ch + *(int32_t *)((uintptr_t)desc + kind));
   kind = (desc->ut_flags & UNICODE_FSPACE) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
  }
  break;
 }
 return DeeString_PackWidthBuffer(result,width,DEE_STRING_CODEC_FREPLAC);
}
INTERN DREF DeeObject *DCALL
DeeString_Capitalize(DeeObject *__restrict self) {
 unsigned int width; void *str,*result;
 size_t i,length;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 width  = DeeString_WIDTH(self);
 str    = DeeString_WSTR(self);
 length = WSTR_LENGTH(str);
 if (!length) return_empty_string;
 result = DeeString_NewWidthBuffer(length,width);
 if unlikely(!result) return NULL;
 SWITCH_SIZEOF_WIDTH(width) {
 CASE_WIDTH_1BYTE:
  ((uint8_t *)result)[0] = (uint8_t)DeeUni_ToUpper(((uint8_t *)str)[0]);
  for (i = 1; i < length; ++i)
     ((uint8_t *)result)[i] = (uint8_t)DeeUni_ToLower(((uint8_t *)str)[i]);
  break;
 CASE_WIDTH_2BYTE:
  ((uint16_t *)result)[0] = (uint16_t)DeeUni_ToUpper(((uint16_t *)str)[0]);
  for (i = 1; i < length; ++i)
     ((uint16_t *)result)[i] = (uint16_t)DeeUni_ToLower(((uint16_t *)str)[i]);
  break;
 CASE_WIDTH_4BYTE:
  ((uint32_t *)result)[0] = (uint32_t)DeeUni_ToUpper(((uint32_t *)str)[0]);
  for (i = 1; i < length; ++i)
     ((uint32_t *)result)[i] = (uint32_t)DeeUni_ToLower(((uint32_t *)str)[i]);
  break;
 }
 return DeeString_PackWidthBuffer(result,width,DEE_STRING_CODEC_FREPLAC);
}
INTERN DREF DeeObject *DCALL
DeeString_Swapcase(DeeObject *__restrict self) {
 unsigned int width; void *str,*result;
 size_t i,length;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 width  = DeeString_WIDTH(self);
 str    = DeeString_WSTR(self);
 length = WSTR_LENGTH(str);
 result = DeeString_NewWidthBuffer(length,width);
 if unlikely(!result) return NULL;
 SWITCH_SIZEOF_WIDTH(width) {
 CASE_WIDTH_1BYTE:
  for (i = 0; i < length; ++i)
     ((uint8_t *)result)[i] = (uint8_t)DeeUni_SwapCase(((uint8_t *)str)[i]);
  break;
 CASE_WIDTH_2BYTE:
  for (i = 0; i < length; ++i)
     ((uint16_t *)result)[i] = (uint16_t)DeeUni_SwapCase(((uint16_t *)str)[i]);
  break;
 CASE_WIDTH_4BYTE:
  for (i = 0; i < length; ++i)
     ((uint32_t *)result)[i] = (uint32_t)DeeUni_SwapCase(((uint32_t *)str)[i]);
  break;
 }
 return DeeString_PackWidthBuffer(result,width,DEE_STRING_CODEC_FREPLAC);
}



#else /* CONFIG_USE_NEW_STRING_API */

/* Create a unicode string (with its size at `((size_t *)return)[-1]')
 * HINT: The returned pointer is allocated on the heap. */
typedef size_t *(DCALL *unicode_maker_t)(String const *__restrict text, int errors);
typedef char *(DCALL *unicode_get_t)(void const *__restrict data, size_t datalen, char *__restrict dst, char *__restrict end, int errors);

PRIVATE uint8_t const uni_bytemarks[7] = {0x00,0x00,0xC0,0xE0,0xF0,0xF8,0xFC};
PRIVATE uint8_t const utf8_trailing_bytes[256] = {
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5};
PRIVATE uint32_t const utf8_offsets[6] = {0x00000000,0x00003080,0x000E2080,
                                          0x03C82080,0xFA082080,0x82082080};

#define UNICODE_REPLACEMENT      '?'
#define UNI_HALF_BASE            0x0010000
#define UNI_HALF_MASK            0x3FF
#define UNI_HALF_SHIFT           10
#define UNI_MAX_BMP              0x0000FFFF
#define UNI_MAX_LEGAL_UTF32      0x0010FFFF
#define UNI_MAX_UTF16            0x0010FFFF
#define UNI_MAX_UTF32            0x7FFFFFFF
#define UNI_SURROGATE_HIGH_END   0xDBFF
#define UNI_SURROGATE_HIGH_BEGIN 0xD800
#define UNI_SURROGATE_LOW_END    0xDFFF
#define UNI_SURROGATE_LOW_BEGIN  0xDC00

LOCAL bool DCALL
utf8_check(char const *__restrict utf8, size_t utf8chars) {
 uint8_t ch; char const *end = utf8+utf8chars;
 switch (utf8chars) {
 case 4:
  if ((ch = (uint8_t)*--end) < 0x80 || ch > 0xbf) return false;
  ATTR_FALLTHROUGH
 case 3:
  if ((ch = (uint8_t)*--end) < 0x80 || ch > 0xbf) return false;
  ATTR_FALLTHROUGH
 case 2:
  if ((ch = (uint8_t)*--end) < 0x80 || ch > 0xbf) return false;
  switch ((uint8_t)*utf8) {
  case 0xe0: if (ch < 0xa0) return false; break;
  case 0xed: if (ch > 0x9f) return false; break;
  case 0xf0: if (ch < 0x90) return false; break;
  case 0xf4: if (ch > 0x8f) return false; break;
  default:   if (ch < 0x80) return false; break;
  }
  ATTR_FALLTHROUGH
 case 1:
  if ((uint8_t)*utf8 >= 0x80 && (uint8_t)*utf8 < 0xc2)
      return false;
  break;
 default:
  return false;
 }
 if ((uint8_t)*utf8 > 0xF4)
     return false;
 return true;
}

#define ERROR_ENCODE 0
#define ERROR_DECODE 1
PRIVATE DeeTypeObject *error_types[] = {
    /* [ERROR_ENCODE] = */&DeeError_UnicodeEncodeError,
    /* [ERROR_DECODE] = */&DeeError_UnicodeDecodeError
};
PRIVATE void DCALL
unicode_few_characters(size_t required, size_t avail, int error_kind) {
 DeeError_Throwf(error_types[error_kind],
                 "Expected at least %Iu characters when only %Iu were available",
                 required,avail);
}
PRIVATE void DCALL
unicode_invalid_sequence(size_t length, char const *__restrict seq, int error_kind) {
 DeeError_Throwf(error_types[error_kind],
                 "Invalid unicode sequence %$q",
                 length,seq);
}
PRIVATE void DCALL
unicode_invalid_surrogate(uint32_t ch, int error_kind) {
 DeeError_Throwf(error_types[error_kind],
                 "Invalid unicode surrogate %#I32x",ch);
}
PRIVATE void DCALL
unicode_invalid_utf16(uint32_t ch, int error_kind) {
 DeeError_Throwf(error_types[error_kind],
                 "Invalid utf-16 character %#I32x",ch);
}
PRIVATE void DCALL
unicode_invalid_utf32(uint32_t ch, int error_kind) {
 DeeError_Throwf(error_types[error_kind],
                 "Invalid utf-32 character %#I32x",ch);
}

PRIVATE size_t *DCALL
make_utf16(String const *__restrict text, int errors) {
 char const *iter,*end; uint8_t src_size; uint32_t ch;
 size_t *result,*new_result; uint16_t *dst; size_t result_length;
 /* If this leaks, that's ok (encoding buffers of static strings...) */
 result = (size_t *)(Dee_Malloc)(sizeof(size_t)+sizeof(uint16_t)+2*
                                 DeeString_SIZE(text));
 if unlikely(!result) return NULL;
 dst = (uint16_t *)(result+1);
 end = (iter = DeeString_STR(text))+DeeString_SIZE(text);
 while (iter != end) {
  size_t avail = (size_t)(end-iter);
  src_size = utf8_trailing_bytes[(uint8_t)*iter];
  if unlikely((size_t)src_size+1 > avail) {
   /* Not enough available input characters. */
   if (errors == UNICODE_ERRORS_IGNORE) break;
   if (errors == UNICODE_ERRORS_REPLACE) { *dst++ = UNICODE_REPLACEMENT; break; }
   /* Throw en encoding error. */
   unicode_few_characters(src_size+1,avail,ERROR_ENCODE);
   goto err;
  }
  if unlikely(!utf8_check(iter,src_size+1)) {
   if (errors == UNICODE_ERRORS_IGNORE ||
       errors == UNICODE_ERRORS_REPLACE) {
    if (errors == UNICODE_ERRORS_REPLACE) *dst++ = UNICODE_REPLACEMENT;
    iter += src_size+1;
    unicode_invalid_sequence(src_size+1,iter,ERROR_ENCODE);
    continue;
   }
  }
  ch = 0;
  switch (src_size) {
  case 5: ch += (uint8_t)*iter++; ch <<= 6; ATTR_FALLTHROUGH
  case 4: ch += (uint8_t)*iter++; ch <<= 6; ATTR_FALLTHROUGH
  case 3: ch += (uint8_t)*iter++; ch <<= 6; ATTR_FALLTHROUGH
  case 2: ch += (uint8_t)*iter++; ch <<= 6; ATTR_FALLTHROUGH
  case 1: ch += (uint8_t)*iter++; ch <<= 6; ATTR_FALLTHROUGH
  case 0: ch += (uint8_t)*iter++; break;
  }
  ch -= utf8_offsets[src_size];
  if likely(ch <= UNI_MAX_BMP) {
   if unlikely(ch >= UNI_SURROGATE_HIGH_BEGIN &&
               ch <= UNI_SURROGATE_LOW_END) {
    if (errors == UNICODE_ERRORS_IGNORE) continue;
    if (errors == UNICODE_ERRORS_REPLACE) { *dst++ = UNICODE_REPLACEMENT; continue; }
    unicode_invalid_surrogate(ch,ERROR_ENCODE);
    goto err;
   }
   *dst++ = (uint16_t)ch;
  } else if unlikely(ch > UNI_MAX_UTF16) {
   if (errors == UNICODE_ERRORS_IGNORE) continue;
   if (errors == UNICODE_ERRORS_REPLACE) { *dst++ = UNICODE_REPLACEMENT; continue; }
   unicode_invalid_utf16(ch,ERROR_ENCODE);
   goto err;
  } else { /* Range: 0xFFFF - 0x10FFFF. */
   ch -= UNI_HALF_BASE;
   *dst++ = (uint16_t)((ch >> UNI_HALF_SHIFT)+UNI_SURROGATE_HIGH_BEGIN);
   *dst++ = (uint16_t)((ch & UNI_HALF_MASK)+UNI_SURROGATE_LOW_BEGIN);
  }
 }
 *dst = 0;
 result_length = (size_t)(dst-(uint16_t *)(result+1));
 new_result = (size_t *)Dee_TryRealloc(result,sizeof(size_t)+
                                       sizeof(uint16_t)+2*result_length);
 if (new_result) result = new_result;
 *result = result_length;
 return result;
err:
 Dee_Free(result);
 return NULL;
}

PRIVATE size_t *DCALL
make_utf32(String const *__restrict text, int errors) {
 char const *iter,*end; uint8_t src_size; uint32_t ch;
 size_t *result,*new_result; uint32_t *dst; size_t result_length;
 /* If this leaks, that's ok (encoding buffers of static strings...) */
 result = (size_t *)(Dee_Malloc)(sizeof(size_t)+sizeof(uint32_t)+4*
                                 DeeString_SIZE(text));
 if unlikely(!result) return NULL;
 dst = (uint32_t *)(result+1);
 end = (iter = DeeString_STR(text))+DeeString_SIZE(text);
 while (iter != end) {
  size_t avail = (size_t)(end-iter);
  src_size = utf8_trailing_bytes[(uint8_t)*iter];
  if unlikely((size_t)src_size+1 > avail) {
   /* Not enough available input characters. */
   if (errors == UNICODE_ERRORS_IGNORE) break;
   if (errors == UNICODE_ERRORS_REPLACE) { *dst++ = UNICODE_REPLACEMENT; break; }
   /* Throw en encoding error. */
   unicode_few_characters(src_size+1,avail,ERROR_ENCODE);
   goto err;
  }
  if unlikely(!utf8_check(iter,src_size+1)) {
   if (errors == UNICODE_ERRORS_IGNORE ||
       errors == UNICODE_ERRORS_REPLACE) {
    if (errors == UNICODE_ERRORS_REPLACE) *dst++ = UNICODE_REPLACEMENT;
    iter += src_size+1;
    unicode_invalid_sequence(src_size+1,iter,ERROR_ENCODE);
    continue;
   }
  }
  ch = 0;
  switch (src_size) {
  case 5: ch += (uint8_t)*iter++; ch <<= 6; ATTR_FALLTHROUGH
  case 4: ch += (uint8_t)*iter++; ch <<= 6; ATTR_FALLTHROUGH
  case 3: ch += (uint8_t)*iter++; ch <<= 6; ATTR_FALLTHROUGH
  case 2: ch += (uint8_t)*iter++; ch <<= 6; ATTR_FALLTHROUGH
  case 1: ch += (uint8_t)*iter++; ch <<= 6; ATTR_FALLTHROUGH
  case 0: ch += (uint8_t)*iter++; break;
  }
  ch -= utf8_offsets[src_size];
  if unlikely(ch > UNI_MAX_LEGAL_UTF32) {
   if (errors == UNICODE_ERRORS_IGNORE) continue;
   if (errors == UNICODE_ERRORS_REPLACE) { *dst++ = UNICODE_REPLACEMENT; continue; }
   unicode_invalid_utf32(ch,ERROR_ENCODE);
   goto err;
  }
  if unlikely(ch >= UNI_SURROGATE_HIGH_BEGIN &&
              ch <= UNI_SURROGATE_LOW_END) {
   if (errors == UNICODE_ERRORS_IGNORE) continue;
   if (errors == UNICODE_ERRORS_REPLACE) { *dst++ = UNICODE_REPLACEMENT; continue; }
   unicode_invalid_surrogate(ch,ERROR_ENCODE);
   goto err;
  }
  *dst++ = ch;
 }
 *dst = 0;
 result_length = (size_t)(dst-(uint32_t *)(result+1));
 new_result = (size_t *)Dee_TryRealloc(result,sizeof(size_t)+
                                       sizeof(uint32_t)+4*result_length);
 if (new_result) result = new_result;
 *result = result_length;
 return result;
err:
 Dee_Free(result);
 return NULL;
}

PRIVATE char *DCALL
get_utf16(void const *__restrict data, size_t datalen,
          char *__restrict dst, char *__restrict end, int errors) {
 uint16_t const *iter,*s_end; uint32_t ch;
 s_end = (iter = (uint16_t *)data)+datalen;
 while (iter != s_end) {
  unsigned int dst_size;
  unsigned char *temp;
  ch = *iter++;
  /* Convert surrogate pair to Utf32 */
  if (ch >= UNI_SURROGATE_HIGH_BEGIN &&
      ch <= UNI_SURROGATE_HIGH_END) {
   if likely(iter < s_end) {
    uint32_t ch2 = *iter;
    if likely(ch2 >= UNI_SURROGATE_LOW_BEGIN && ch2 <= UNI_SURROGATE_LOW_END) {
     ch = (((ch-UNI_SURROGATE_HIGH_BEGIN) << UNI_HALF_SHIFT) +
            (ch2-UNI_SURROGATE_LOW_BEGIN)+UNI_HALF_BASE);
     ++iter;
    } else {
     if (errors == UNICODE_ERRORS_IGNORE) continue;
     if (errors == UNICODE_ERRORS_REPLACE) { *dst++ = UNICODE_REPLACEMENT; continue; }
     unicode_invalid_surrogate(ch2,ERROR_DECODE);
     goto err;
    }
   } else {
    if (errors == UNICODE_ERRORS_IGNORE) continue;
    if (errors == UNICODE_ERRORS_REPLACE) { *dst++ = UNICODE_REPLACEMENT; continue; }
    unicode_few_characters(2,2,ERROR_DECODE);
    goto err;
   }
  } else if unlikely(ch >= UNI_SURROGATE_LOW_BEGIN &&
                     ch <= UNI_SURROGATE_LOW_END) {
   if (errors == UNICODE_ERRORS_IGNORE) continue;
   if (errors == UNICODE_ERRORS_REPLACE) { *dst++ = UNICODE_REPLACEMENT; continue; }
   unicode_invalid_surrogate(ch,ERROR_DECODE);
   goto err;
  }
  if likely(ch < (uint32_t)0x80) dst_size = 1;
  else if (ch < (uint32_t)0x800) dst_size = 2;
  else if (ch < (uint32_t)0x10000) dst_size = 3;
  else if (ch < (uint32_t)0x110000) dst_size = 4;
  else {
   if (errors == UNICODE_ERRORS_IGNORE) continue;
   if (errors == UNICODE_ERRORS_REPLACE) { *dst++ = UNICODE_REPLACEMENT; continue; }
   unicode_invalid_utf16(ch,ERROR_DECODE);
   goto err;
  }
  temp = (unsigned char *)(dst += dst_size);
  if (dst < end) switch (dst_size) {
  case 4: *--temp = (unsigned char)((ch|0x80)&0xBF); ch >>= 6; ATTR_FALLTHROUGH
  case 3: *--temp = (unsigned char)((ch|0x80)&0xBF); ch >>= 6; ATTR_FALLTHROUGH
  case 2: *--temp = (unsigned char)((ch|0x80)&0xBF); ch >>= 6; ATTR_FALLTHROUGH
  case 1: *--temp = (unsigned char)(ch|uni_bytemarks[dst_size]);
  }
 }
 return dst;
err:
 return NULL;
}

PRIVATE char *DCALL
get_utf32(void const *__restrict data, size_t datalen,
          char *__restrict dst, char *__restrict end, int errors) {
 uint32_t const *s_iter,*s_end; uint32_t ch;
 s_end = (s_iter = (uint32_t *)data)+datalen;
 while (s_iter != s_end) {
  unsigned int dst_size;
  ch = *s_iter++;
  if unlikely(ch >= UNI_SURROGATE_HIGH_BEGIN &&
              ch <= UNI_SURROGATE_LOW_END) {
   unicode_invalid_surrogate(ch,ERROR_DECODE);;
   goto err;
  }
  if likely(ch < (uint32_t)0x80) dst_size = 1;
  else if (ch < (uint32_t)0x800) dst_size = 2;
  else if (ch < (uint32_t)0x10000) dst_size = 3;
  else if (ch <= UNI_MAX_LEGAL_UTF32) dst_size = 4;
  else {
   if (errors == UNICODE_ERRORS_IGNORE) continue;
   if (errors == UNICODE_ERRORS_REPLACE) { *dst++ = UNICODE_REPLACEMENT; continue; }
   unicode_invalid_utf32(ch,ERROR_DECODE);
   goto err;
  }
  if (dst < end) switch (dst_size) {
  case 4: *dst++ = (char)((unsigned char)((ch|0x80)&0xbf)); ch >>= 6; ATTR_FALLTHROUGH
  case 3: *dst++ = (char)((unsigned char)((ch|0x80)&0xbf)); ch >>= 6; ATTR_FALLTHROUGH
  case 2: *dst++ = (char)((unsigned char)((ch|0x80)&0xbf)); ch >>= 6; ATTR_FALLTHROUGH
  case 1: *dst   = (char)((unsigned char) (ch|uni_bytemarks[dst_size]));
  }
 }
 return dst;
err:
 return NULL;
}



#ifdef CONFIG_WCHAR_STRINGS
#ifndef CONFIG_HOST_WINDOWS
PRIVATE bool DCALL init_locale(int errors) {
 static bool locale_initialized = false;
 if (!locale_initialized) {
  if (!setlocale(LC_ALL,"")) {
   if (errors == UNICODE_ERRORS_STRICT) {
    DeeError_Throwf(&DeeError_UnicodeError,
                    "Failed to initialize the current locale");
    return false;
   }
   return true;
  }
  locale_initialized = true;
 }
 return true;
}
#endif

/* Wide-character conversion. */
PRIVATE size_t *DCALL
make_wchar(String const *__restrict text, int errors) {
 size_t *result,*new_result;
 /* If this leaks, that's ok (encoding buffers of static strings...) */
 result = (size_t *)(Dee_Malloc)(sizeof(size_t)+sizeof(wchar_t)+
                                 sizeof(wchar_t)*DeeString_SIZE(text));
 if unlikely(!result) return NULL;
 {
#ifdef CONFIG_HOST_WINDOWS
  DWORD flags = 0; int temp;
  if (errors == UNICODE_ERRORS_STRICT) flags |= MB_ERR_INVALID_CHARS;
  temp = MultiByteToWideChar(CP_ACP,flags,text->s_str,(int)text->s_len,
                            (LPWSTR)(result+1),(int)text->s_len);
  if unlikely(unlikely(!temp) && text->s_len && GetLastError() != NO_ERROR) {
   if (errors != UNICODE_ERRORS_STRICT) {
    /* Fallback: Cast the given string to wide-characters. */
    wchar_t *dst; char *src,*end;
    dst = (wchar_t *)(result+1);
    dst[text->s_len] = '\0';
    end = (src = (char *)text->s_str)+text->s_len;
    for (; src != end; ++src,++dst) *dst = (wchar_t)*src;
    *result = text->s_len;
    return result;
   }
   unicode_invalid_sequence(text->s_len,text->s_str,ERROR_ENCODE);
   goto err;
  }
  *result = (size_t)(unsigned int)temp;
  if unlikely((unsigned int)temp > text->s_len) {
   new_result = (size_t *)Dee_Realloc(result,sizeof(size_t)+sizeof(wchar_t)+
                                      sizeof(wchar_t)*(unsigned int)temp);
   if unlikely(!new_result) goto err;
   result = new_result;
   MultiByteToWideChar(CP_ACP,flags,text->s_str,(int)text->s_len,
                      (LPWSTR)(result+1),temp);
  } else if ((unsigned int)temp < text->s_len) {
   new_result = (size_t *)Dee_TryRealloc(result,sizeof(size_t)+sizeof(wchar_t)+
                                         sizeof(wchar_t)*(unsigned int)temp);
   if likely(new_result) result = new_result;
  }
  ((wchar_t *)(result+1))[(unsigned int)temp] = '\0';
#else
  mbstate_t state; wchar_t wc;
  size_t len,result_length;
  char const *iter; size_t src_len;
  wchar_t *dst = (wchar_t *)(result+1);
  iter = text->s_str,src_len = text->s_len;
  if (!init_locale(errors)) return NULL;
  memset(&state,0,sizeof(state));
  while (src_len) {
   len = mbrtowc(&wc,iter,src_len,&state);
   if unlikely(len == (size_t)-1) {
    if (errors == UNICODE_ERRORS_STRICT) {
     unicode_invalid_sequence(1,iter,ERROR_ENCODE);
     goto err;
    }
    if (errors == UNICODE_ERRORS_REPLACE)
       *dst++ = UNICODE_REPLACEMENT;
    len = 1;
   } else if unlikely(len == (size_t)-2) {
    if (errors == UNICODE_ERRORS_STRICT) {
     unicode_few_characters(src_len+1,src_len,ERROR_ENCODE);
     goto err;
    }
    if (errors == UNICODE_ERRORS_REPLACE)
       *dst++ = UNICODE_REPLACEMENT;
    break;
   } else {
    if unlikely(!len) len = 1;
    *dst++ = wc;
   }
   if (len >= src_len) break;
   src_len -= len;
   iter += len;
  }
  *dst = '\0';
  result_length = (size_t)(dst-(wchar_t *)(result+1));
  *result = result_length;
  ASSERT(result_length <= DeeString_SIZE(text));
  if (result_length != DeeString_SIZE(text)) {
   new_result = (size_t *)Dee_TryRealloc(result,sizeof(size_t)+sizeof(wchar_t)+
                                         sizeof(wchar_t)*result_length);
   if likely(new_result) result = new_result;
  }
#endif
 }
 return result;
err:
 Dee_Free(result);
 return NULL;
}
PRIVATE char *DCALL
get_wchar(void const *__restrict data, size_t datalen,
          char *__restrict dst, char *__restrict end, int errors) {
#ifdef CONFIG_HOST_WINDOWS
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS 0x00000080
#endif
 static DWORD wincall_flags = WC_ERR_INVALID_CHARS;
 DWORD flags; int temp;
restart:
 flags = 0;
 /* */if (errors == UNICODE_ERRORS_STRICT)  flags |= wincall_flags;
 else if (errors == UNICODE_ERRORS_REPLACE) flags |= WC_DEFAULTCHAR;
 temp = WideCharToMultiByte(CP_ACP,flags,(LPCWCH)data,
                           (int)datalen,dst,(int)(end-dst),NULL,NULL);
 if unlikely(!temp && datalen) {
  DWORD error = GetLastError();
  if (error == NO_ERROR) goto ok_input;
  if (error == ERROR_INVALID_FLAGS && (flags&WC_ERR_INVALID_CHARS)) {
   wincall_flags &= ~(WC_ERR_INVALID_CHARS);
   goto restart;
  }
  if (error == ERROR_INSUFFICIENT_BUFFER) {
   size_t req_buffer = (size_t)(end-dst);
   if unlikely(!req_buffer) req_buffer = 8;
   return dst+(req_buffer*2);
  }
  if (errors != UNICODE_ERRORS_STRICT) {
   /* Fallback: Cast the given string to wide-characters. */
   wchar_t *src = (wchar_t *)data;
   while (datalen--) *dst++ = (char)*src++;
   goto done;
  }
  unicode_invalid_sequence((datalen*sizeof(wchar_t))/sizeof(char),
                           (char const *)data,ERROR_DECODE);
  goto err;
 }
ok_input:
 dst += (unsigned int)temp;
done:
#else
#define WCHAR_USE_MB_LEN_MAX 1
 wchar_t *iter = (wchar_t *)data;
 mbstate_t state; size_t buflen;
 if (!init_locale(errors)) return NULL;
 memset(&state,0,sizeof(state));
 while (datalen--) {
  buflen = wcrtomb(dst+MB_LEN_MAX <= end ? dst : NULL,*iter++,&state);
  if unlikely(buflen == (size_t)-1) {
   if (errors == UNICODE_ERRORS_IGNORE) continue;
   if (errors == UNICODE_ERRORS_REPLACE) { *dst++ = '?'; continue; }
   unicode_invalid_sequence(datalen*sizeof(wchar_t),(char const *)iter,ERROR_DECODE);
   goto err;
  }
  dst += buflen;
 }
 return 0;
#endif
 return dst;
err:
 return NULL;
}
#endif


/* Unicode generators. */
PRIVATE unicode_maker_t unigen[STRING_WIDTH_COUNT] = {
    /* [STRING_WIDTH_2BYTE] = */&make_utf16,
    /* [STRING_WIDTH_4BYTE] = */&make_utf32
#ifdef CONFIG_WCHAR_STRINGS
    ,
    /* [STRING_WIDTH_WCHAR] = */&make_wchar
#endif
};

/* Unicode generators. */
PRIVATE unicode_get_t uniget[STRING_WIDTH_COUNT] = {
    /* [STRING_WIDTH_2BYTE] = */&get_utf16,
    /* [STRING_WIDTH_4BYTE] = */&get_utf32
#ifdef CONFIG_WCHAR_STRINGS
    ,
    /* [STRING_WIDTH_WCHAR] = */&get_wchar
#endif
};


/* Unicode duplicators. */
PRIVATE unsigned int unisft[STRING_WIDTH_COUNT] = {
    /* [STRING_WIDTH_2BYTE] = */1, /* 1 << 1 == sizeof(uint16_t) */
    /* [STRING_WIDTH_4BYTE] = */2  /* 1 << 2 == sizeof(uint32_t) */
#ifdef CONFIG_WCHAR_STRINGS
    ,
#if __SIZEOF_WCHAR_T__ == 2
    /* [STRING_WIDTH_WCHAR] = */1  /* 1 << 1 == sizeof(wchar_t) */
#else
    /* [STRING_WIDTH_WCHAR] = */2  /* 1 << 2 == sizeof(wchar_t) */
#endif
#endif
};



PUBLIC DREF DeeObject *DCALL
DeeString_RezEncodingBuffer(DREF DeeObject *self, size_t length,
                            int encoding, void **pencstr) {
 DREF String *result; struct string_utf *utf; size_t *str;
 /* Simple case: When the encoding is UTF-8, we can just use buffer-functions. */
 if (encoding == STRING_WIDTH_1BYTE) {
  result = (DREF String *)DeeString_ResizeBuffer(self,length);
  if (pencstr) *pencstr = DeeString_STR(result); /* This won't crash, even if `result == NULL' */
  goto done;
 }
 /* Must not resize the empty string, which is actually allowed as argument! */
 if unlikely(self == Dee_EmptyString) self = NULL;
 /* Special case: Resize-from-NULL --> Create new. */
 if (!self)
      return DeeString_NewEncodingBuffer(length,encoding,pencstr);
 ASSERTF(DeeString_Check(self),"Not a string buffer");
 ASSERTF(!DeeObject_IsShared(self),"String buffers cannot be shared");
 /* Re-use the existing buffer. */
 result = (DREF String *)self;
 utf    = result->s_data;
 ASSERTF(utf && utf->su_pref == encoding,
         "You can't change your encoding when "
         "resizing an encoding-string-buffer");
 str = utf->su_enc[encoding];
 /* Re-allocate the existing string buffer. */
 str = (size_t *)Dee_Realloc(str-1,sizeof(size_t)+
                            (length+1)*DeeEnc_Size(encoding));
 if unlikely(!str) goto err;
 *str++ = length; /* Save the new length. */
 /* Prepare the string by ZERO-terminating its buffer. */
#if __SIZEOF_WCHAR_T__ == 2
 if (DeeEnc_Is4Byte(encoding))
#else
 if (!DeeEnc_Is2Byte(encoding))
#endif
 {
  ((uint32_t *)str)[length] = 0;
 } else {
  ((uint16_t *)str)[length] = 0;
 }
 utf->su_enc[encoding] = str; /* Save the string in the UTF descriptor. */
 if (pencstr) *pencstr = str; /* If requested by the caller, pass it to them as well. */
done:
 return (DREF DeeObject *)result;
err:
 return NULL;
}

PUBLIC DREF DeeObject *DCALL
DeeString_NewEncodingBuffer(size_t length, int encoding, void **pencstr) {
 DREF String *result; struct string_utf *utf; size_t *str;
 /* Start by creating a string buffer matching the length
  * of the string that's going to be encoded within.
  * If the encoding is UTF-8, this is all we'll have to do,
  * and if it isn't, the most common use-case of unicode is
  * to simply represent ASCII characters, in which case a later
  * call to `DeeString_SetEncodingBuffer()' will not be required
  * to realloc the string again. */
 result = (DREF String *)DeeString_NewBuffer(length);
 if (unlikely(!result) || encoding == STRING_WIDTH_1BYTE) {
  if (pencstr) *pencstr = DeeString_STR(result); /* This won't crash, even if `result == NULL' */
  goto done;
 }
 ASSERT(!DeeObject_IsShared(result));
 /* Allocate a UTF descriptor and the actual buffer requested by the caller. */
 if unlikely((utf = utf_alloc()) == NULL) goto err_r;
 memset(utf,0,sizeof(struct string_utf));
 utf->su_pref   = encoding;
 result->s_data = utf;
 /* Allocate the buffer requested by the caller. */
 str = (size_t *)Dee_Malloc(sizeof(size_t)+(length+1)*DeeEnc_Size(encoding));
 if unlikely(!str) goto err_r;
 *str++ = length; /* Store the length below the string. */
 /* Prepare the string by ZERO-terminating its buffer. */
#if __SIZEOF_WCHAR_T__ == 2
 if (DeeEnc_Is4Byte(encoding))
#else
 if (!DeeEnc_Is2Byte(encoding))
#endif
 {
  ((uint32_t *)str)[length] = 0;
 } else {
  ((uint16_t *)str)[length] = 0;
 }
 utf->su_enc[encoding] = str; /* Save the string in the UTF descriptor. */
 if (pencstr) *pencstr = str; /* If requested by the caller, pass it to them as well. */
done:
 return (DREF DeeObject *)result;
err_r:
 ASSERT(!DeeObject_IsShared(result));
 DeeObject_Destroy((DeeObject *)result);
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeString_SetEncodingBuffer(DREF DeeObject *__restrict self) {
 struct string_utf *data; size_t *encoded_string;
 char *iter,*bufend; String *new_result;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERTF(!DeeObject_IsShared(self),"Cannot set the encoding of shared strings.");
 /* Simple case: The string uses UTF-8 encoding. */
 if ((data = DeeString_UTF(self)) == NULL ||
      data->su_pref == STRING_WIDTH_1BYTE)
      return self;
 encoded_string = data->su_enc[data->su_pref];
do_decode:
 /* Decode the string into its UTF-8 representation. */
 bufend = (iter = DeeString_STR(self))+DeeString_SIZE(self);
 iter = (*uniget[data->su_pref])(encoded_string,encoded_string[-1],iter,bufend,0);
 if unlikely(!iter) goto err_r;
 if (iter != bufend) {
  /* Reallocate to fit the exact, required size. */
  size_t required_length = (size_t)(iter-DeeString_STR(self));
  size_t required_size = (offsetof(String,s_str)+sizeof(char))+
                         (required_length*sizeof(char));
do_realloc:
  new_result = (String *)DeeObject_TryRealloc(self,required_size);
  if unlikely(!new_result) {
   /* Failed to reallocate the string. */
   /* Unlikely: We were just trying to truncate it, but whatever... */
   if unlikely(iter < bufend) goto no_realloc;
   /* Collect memory, then try to reallocate the string again. */
   if likely(Dee_CollectMemory(required_size)) goto do_realloc;
   goto err_r;
  }
  /* Assign the new string. */
  new_result->s_len = required_length;
  self = (DREF DeeObject *)new_result;
  /* If the string couldn't be decoded yet, try again. */
  if (iter > bufend) goto do_decode;
  iter = DeeString_END(self);
 }
no_realloc:
 /* Terminate the string; C-style */
 *iter = '\0';

 return self;
err_r:
 /* Always inherit a reference from `self'.
  * Furthermore, since `self' must not be shared, we can simply destroy it on error. */
 ASSERT(!DeeObject_IsShared(self));
 DeeObject_Destroy((DeeObject *)self);
 return NULL;
}
PUBLIC DREF DeeObject *DCALL
DeeString_SetEncodingBufferWithLength(DREF DeeObject *__restrict self, size_t length) {
 struct string_utf *data; size_t *encoded_string;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERTF(!DeeObject_IsShared(self),"Cannot set the encoding of shared strings.");
 /* Simple case: The string uses UTF-8 encoding. */
 if ((data = DeeString_UTF(self)) == NULL ||
      data->su_pref == STRING_WIDTH_1BYTE)
      return DeeString_SetBufferWithLength(self,length);
 encoded_string = data->su_enc[data->su_pref]-1;
 ASSERTF(length <= *encoded_string,"The buffer isn't large enough");
 if (length != *encoded_string) {
  /* Re-allocate to fit. */
  encoded_string = (size_t *)Dee_TryRealloc(encoded_string,sizeof(size_t)+
                                           (length+1)*DeeEnc_Size(data->su_pref));
  if unlikely(!encoded_string)
     encoded_string = data->su_enc[data->su_pref]-1;
  *encoded_string++ = length;
  /* Ensure ZERO-termination of the encoded string. */
#if __SIZEOF_WCHAR_T__ == 2
  if (DeeEnc_Is4Byte(data->su_pref))
#else
  if (!DeeEnc_Is2Byte(data->su_pref))
#endif
  {
   ((uint32_t *)encoded_string)[length] = 0;
  } else {
   ((uint16_t *)encoded_string)[length] = 0;
  }
  data->su_enc[data->su_pref] = encoded_string;
 }
 /* Set the encoding buffer. */
 return DeeString_SetEncodingBuffer(self);
}

PUBLIC DREF DeeObject *DCALL
DeeString_NewWithEncoding(void const *__restrict str,
                          int encoding_and_errors) {
 size_t length = 0;
 switch (UNICODE_ENCODING(encoding_and_errors)) {
#if 1
 default:        length = strlen((char *)str); break;
#else
 default:        while (((uint8_t  *)str)[length]) ++length; break;
#endif
 CASE_ENC_2BYTE: while (((uint16_t *)str)[length]) ++length; break;
 CASE_ENC_4BYTE: while (((uint32_t *)str)[length]) ++length; break;
 }
 return DeeString_NewSizedWithEncoding(str,length,encoding_and_errors);
}
PUBLIC DREF DeeObject *DCALL
DeeString_NewSizedWithEncoding(void const *__restrict str,
                               size_t length, int encoding_and_errors) {
 struct string_utf *utf; char *iter,*bufend;
 DREF String *result,*new_result; size_t result_size;
 int encoding = UNICODE_ENCODING(encoding_and_errors);
 ASSERT((encoding >= 0 && encoding < STRING_WIDTH_COUNT) ||
        (encoding == STRING_WIDTH_1BYTE));
 /* Special case: return UTF-8 encoding. */
 if (encoding == STRING_WIDTH_1BYTE)
     return DeeString_NewSized((char const *)str,length);

 utf = utf_alloc();
 if unlikely(!utf) return NULL;
 memset(utf,0,sizeof(struct string_utf));
 utf->su_pref = encoding; /* Save the preferred encoding. */

 /* Allocate for the worst-case scenario. */
 result_size = offsetof(String,s_str)+sizeof(char)+length*3*sizeof(char);
 if (encoding == STRING_WIDTH_4BYTE)
  result_size += length*sizeof(char);
#ifdef CONFIG_WCHAR_STRINGS
 else if (encoding == STRING_WIDTH_WCHAR) {
  result_size = offsetof(String,s_str)+sizeof(char)+(MB_LEN_MAX*sizeof(char));
 }
#endif

 result = (DREF String *)DeeObject_Malloc(result_size);
 if unlikely(!result) goto err_utf;

 /* Convert the encoding. */
do_convert:
 bufend = result->s_str+(result_size-(offsetof(String,s_str)+sizeof(char)));
 iter = (*uniget[encoding])(str,length,result->s_str,bufend,
                            UNICODE_ERRORS(encoding_and_errors));
 if unlikely(!iter) goto err_r;
 if unlikely(iter > bufend) {
  /* Reallocate to fit into a larger buffer. */
  result_size = (offsetof(String,s_str)+sizeof(char)+
                (size_t)((iter-result->s_str)*sizeof(char)));
  new_result = (DREF String *)DeeObject_Realloc(result,result_size);
  if unlikely(!new_result) goto err_r;
  result = new_result;
  goto do_convert;
 }
 *iter          = '\0';
 result->s_len  = (size_t)(iter-result->s_str);
 result_size    = offsetof(String,s_str)+sizeof(char);
 result_size   += result->s_len;
 result->s_data = utf;
 result->s_hash = (dhash_t)-1;

 /* Try to save the given string as a known encoding.
  * We do this because considering the fact that the passing encoding
  * will be what user-code is going to use when accessing the string,
  * otherwise we'd most likely have to allocate it then if we don't
  * simply use the copy we've got now. - So why do more work then?
  * Though it's OK if this fails. */
 { size_t *duplicate;
#ifdef WIDECHAR_USE_MB_CUR_MAX
#else
#endif
   duplicate = (size_t *)Dee_TryMalloc(sizeof(size_t)+
                                     ((length+1) << unisft[encoding]));
   if likely(duplicate) {
    size_t offset;
    *duplicate++ = length;
    offset = length << unisft[encoding];
    memcpy(duplicate,str,offset);
    memset((uint8_t *)duplicate+offset,0,1 << unisft[encoding]);
    utf->su_enc[encoding] = duplicate;
   }
 }

 /* Preserve memory by freeing up unused trailing data. */
 new_result = (DREF String *)DeeObject_TryRealloc(result,result_size);
 if (new_result) result = new_result;
 DeeObject_Init(result,&DeeString_Type);
 return (DREF DeeObject *)result;
err_r:   DeeObject_Free(result);
err_utf: Dee_Free(utf);
 return NULL;
}


PUBLIC void *DCALL
DeeString_AsEncoding(DeeObject *__restrict self,
                     int encoding_and_errors) {
 struct string_utf *utf; size_t *result;
 int encoding = UNICODE_ENCODING(encoding_and_errors);
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT((encoding >= 0 && encoding < STRING_WIDTH_COUNT) ||
        (encoding == STRING_WIDTH_1BYTE));
 /* Special case: return UTF-8 encoding. */
 if (encoding == STRING_WIDTH_1BYTE)
     return DeeString_STR(self);

 if ((utf = DeeString_UTF(self)) == NULL) {
  /* Allocate the data controller. */
  utf = (utf_alloc)(); /* If this leaks, that's ok (encoding buffers of static strings...) */
  if unlikely(!utf) return NULL;
  memset(utf,0,sizeof(struct string_utf));
  /* Since no UTF controller was allocated before, the
   * string must have been created as a UTF-8 string. */
  utf->su_pref = STRING_WIDTH_1BYTE;
  /* Store the new UTF controller in the string object. */
#ifndef CONFIG_NO_THREADS
  { struct string_utf *old_data;
    old_data = ATOMIC_CMPXCH_VAL(DeeString_UTF(self),NULL,utf);
    if unlikely(old_data) { Dee_Free(utf); utf = old_data; }
  }
#else
  ASSERT(!DeeString_UTF(self));
  DeeString_UTF(self) = utf;
#endif
 }

 /* Lookup the requested encoding. */
 if ((result = utf->su_enc[encoding]) == NULL) {
  /* If this representation hasn't been allocated yet, create it now. */
  result = (*unigen[encoding])((String *)self,UNICODE_ERRORS(encoding_and_errors));
  if unlikely(!result) return NULL;
  ++result; /* Advance to hide the size-prefix. */
#ifndef CONFIG_NO_THREADS
  { size_t *old_result;
    old_result = ATOMIC_CMPXCH_VAL(utf->su_enc[encoding],NULL,result);
    if unlikely(old_result) { Dee_Free(result-1); result = old_result; }
  }
#else
  ASSERT(!utf->su_enc[encoding]);
  utf->su_enc[encoding] = result;
#endif
 }
 return result;
}

PUBLIC DREF DeeObject *DCALL
string_printer_pack_enc(struct string_printer *__restrict self,
                        int encoding) {
 DREF DeeObject *result;
 result = string_printer_pack(self);
 if (likely(result) && encoding != STRING_WIDTH_1BYTE) {
  if unlikely(result == Dee_EmptyString) {
   Dee_Decref(result);
   return DeeString_NewSizedWithEncoding(NULL,0,encoding);
  }
  ASSERT(!DeeObject_IsShared(result));
  if unlikely(!DeeString_AsEncoding(result,encoding)) {
   Dee_Decref(result);
   return NULL;
  }
  ASSERT(DeeString_UTF(result));
  ASSERT(DeeString_UTF(result)->su_pref == STRING_WIDTH_1BYTE);
  DeeString_UTF(result)->su_pref = encoding;
 }
 return result;
}



PUBLIC DREF DeeObject *DCALL DeeString_NewChar(uint32_t ch) {
 /* Optimization for the likely case of ASCII characters. */
 if (ch <= 0xff)
     return (DREF DeeObject *)DeeString_New1Char((uint8_t)ch);
 if (ch <= 0xffff) {
  return DeeString_NewSizedWithEncoding(&ch,1,STRING_WIDTH_2BYTE);
 } else {
  return DeeString_NewSizedWithEncoding(&ch,1,STRING_WIDTH_4BYTE);
 }
}


INTERN DREF DeeObject *DCALL
DeeString_Convert(DeeObject *__restrict self, uintptr_t kind) {
 DREF String *result; int encoding;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 result = (DREF String *)DeeString_NewBuffer(DeeString_SIZE(self));
 if unlikely(!result) return NULL;
 encoding = DeeString_UtfEnc(self);
 if (encoding == STRING_WIDTH_1BYTE) {
  char *src,*iter,*end;
  src = DeeString_STR(self);
  end = (iter = (char *)DeeString_STR(result))+DeeString_SIZE(result);
  for (; iter != end; ++iter,++src) *iter = (char)DeeUni_Convert(*src,kind);
  *iter = '\0'; /* Terminate a c-style string. */
 } else {
  void *str = DeeString_UtfStr(self);
  struct string_utf *utf = utf_alloc();
  size_t *encoded_string,len = ENCODING_SIZE(str);
  if unlikely(!utf) goto err_r;
  memset(utf,0,sizeof(struct string_utf));
  utf->su_pref = encoding; /* Save the preferred encoding. */
  result->s_data = utf;
  encoded_string = (size_t *)Dee_Malloc(sizeof(size_t)+(len+1)*DeeEnc_Size(encoding));
  if unlikely(!encoded_string) goto err_r;
  *encoded_string++ = len; /* Save the length in the encoded string header. */
  utf->su_enc[encoding] = encoded_string;
#if __SIZEOF_WCHAR_T__ == 2
  if (DeeEnc_Is4Byte(encoding))
#else
  if (!DeeEnc_Is2Byte(encoding))
#endif
  { /* UTF-32 */
   uint32_t *src,*iter,*end;
   src = (uint32_t *)str;
   end = (iter = (uint32_t *)encoded_string)+len;
   for (; iter != end; ++iter,++src) *iter = (uint32_t)DeeUni_Convert(*src,kind);
   *iter = 0; /* Terminate a c-style string. */
  } else { /* UTF-16 */
   uint16_t *src,*iter,*end;
   src = (uint16_t *)str;
   end = (iter = (uint16_t *)encoded_string)+len;
   for (; iter != end; ++iter,++src) *iter = (uint16_t)DeeUni_Convert(*src,kind);
   *iter = 0; /* Terminate a c-style string. */
  }
  result = (DREF String *)DeeString_SetEncodingBuffer((DREF DeeObject *)result);
 }
 return (DREF DeeObject *)result;
err_r:
 Dee_Decref(result);
 return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_ToTitle(DeeObject *__restrict self) {
 DREF String *result; int encoding; uintptr_t kind = UNICODE_CONVERT_TITLE;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 result = (DREF String *)DeeString_NewBuffer(DeeString_SIZE(self));
 if unlikely(!result) return NULL;
 encoding = DeeString_UtfEnc(self);
 if (encoding == STRING_WIDTH_1BYTE) {
  char *src,*iter,*end;
  src = DeeString_STR(self);
  end = (iter = (char *)DeeString_STR(result))+DeeString_SIZE(result);
  for (; iter != end; ++iter,++src) {
   char ch = (char)DeeUni_Convert(*src,kind); *iter = ch;
   kind = DeeUni_IsSpace(ch) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
  }
  *iter = '\0'; /* Terminate a c-style string. */
 } else {
  void *str = DeeString_UtfStr(self);
  struct string_utf *utf = utf_alloc();
  size_t *encoded_string,len = ENCODING_SIZE(str);
  if unlikely(!utf) goto err_r;
  memset(utf,0,sizeof(struct string_utf));
  utf->su_pref = encoding; /* Save the preferred encoding. */
  result->s_data = utf;
  encoded_string = (size_t *)Dee_Malloc(sizeof(size_t)+(len+1)*DeeEnc_Size(encoding));
  if unlikely(!encoded_string) goto err_r;
  *encoded_string++ = len; /* Save the length in the encoded string header. */
  utf->su_enc[encoding] = encoded_string;
#if __SIZEOF_WCHAR_T__ == 2
  if (DeeEnc_Is4Byte(encoding))
#else
  if (!DeeEnc_Is2Byte(encoding))
#endif
  { /* UTF-32 */
   uint32_t *src,*iter,*end;
   src = (uint32_t *)str;
   end = (iter = (uint32_t *)encoded_string)+len;
   for (; iter != end; ++iter,++src) {
    uint32_t ch = (uint32_t)DeeUni_Convert(*src,kind); *iter = ch;
    kind = DeeUni_IsSpace(ch) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
   }
   *iter = 0; /* Terminate a c-style string. */
  } else { /* UTF-16 */
   uint16_t *src,*iter,*end;
   src = (uint16_t *)str;
   end = (iter = (uint16_t *)encoded_string)+len;
   for (; iter != end; ++iter,++src) {
    uint16_t ch = (uint16_t)DeeUni_Convert(*src,kind); *iter = ch;
    kind = DeeUni_IsSpace(ch) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
   }
   *iter = 0; /* Terminate a c-style string. */
  }
  result = (DREF String *)DeeString_SetEncodingBuffer((DREF DeeObject *)result);
 }
 return (DREF DeeObject *)result;
err_r:
 Dee_Decref(result);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_Capitalize(DeeObject *__restrict self) {
 DREF String *result; int encoding; uintptr_t kind = UNICODE_CONVERT_UPPER;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 result = (DREF String *)DeeString_NewBuffer(DeeString_SIZE(self));
 if unlikely(!result) return NULL;
 encoding = DeeString_UtfEnc(self);
 if (encoding == STRING_WIDTH_1BYTE) {
  char *src,*iter,*end;
  src = DeeString_STR(self);
  end = (iter = (char *)DeeString_STR(result))+DeeString_SIZE(result);
  for (; iter != end; ++iter,++src) {
   *iter = (char)DeeUni_Convert(*src,kind);
   kind = UNICODE_CONVERT_LOWER;
  }
  *iter = '\0'; /* Terminate a c-style string. */
 } else {
  void *str = DeeString_UtfStr(self);
  struct string_utf *utf = utf_alloc();
  size_t *encoded_string,len = ENCODING_SIZE(str);
  if unlikely(!utf) goto err_r;
  memset(utf,0,sizeof(struct string_utf));
  utf->su_pref = encoding; /* Save the preferred encoding. */
  result->s_data = utf;
  encoded_string = (size_t *)Dee_Malloc(sizeof(size_t)+(len+1)*DeeEnc_Size(encoding));
  if unlikely(!encoded_string) goto err_r;
  *encoded_string++ = len; /* Save the length in the encoded string header. */
  utf->su_enc[encoding] = encoded_string;
#if __SIZEOF_WCHAR_T__ == 2
  if (DeeEnc_Is4Byte(encoding))
#else
  if (!DeeEnc_Is2Byte(encoding))
#endif
  { /* UTF-32 */
   uint32_t *src,*iter,*end;
   src = (uint32_t *)str;
   end = (iter = (uint32_t *)encoded_string)+len;
   for (; iter != end; ++iter,++src) {
    *iter = (uint32_t)DeeUni_Convert(*src,kind);
    kind = UNICODE_CONVERT_LOWER;
   }
   *iter = 0; /* Terminate a c-style string. */
  } else { /* UTF-16 */
   uint16_t *src,*iter,*end;
   src = (uint16_t *)str;
   end = (iter = (uint16_t *)encoded_string)+len;
   for (; iter != end; ++iter,++src) {
    *iter = (uint16_t)DeeUni_Convert(*src,kind);
    kind = UNICODE_CONVERT_LOWER;
   }
   *iter = 0; /* Terminate a c-style string. */
  }
  result = (DREF String *)DeeString_SetEncodingBuffer((DREF DeeObject *)result);
 }
 return (DREF DeeObject *)result;
err_r:
 Dee_Decref(result);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_Swapcase(DeeObject *__restrict self) {
 DREF String *result; int encoding;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 result = (DREF String *)DeeString_NewBuffer(DeeString_SIZE(self));
 if unlikely(!result) return NULL;
 encoding = DeeString_UtfEnc(self);
 if (encoding == STRING_WIDTH_1BYTE) {
  char *src,*iter,*end;
  src = DeeString_STR(self);
  end = (iter = (char *)DeeString_STR(result))+DeeString_SIZE(result);
  for (; iter != end; ++iter,++src) *iter = (char)DeeUni_SwapCase(*src);
  *iter = '\0'; /* Terminate a c-style string. */
 } else {
  void *str = DeeString_UtfStr(self);
  struct string_utf *utf = utf_alloc();
  size_t *encoded_string,len = ENCODING_SIZE(str);
  if unlikely(!utf) goto err_r;
  memset(utf,0,sizeof(struct string_utf));
  utf->su_pref = encoding; /* Save the preferred encoding. */
  result->s_data = utf;
  encoded_string = (size_t *)Dee_Malloc(sizeof(size_t)+(len+1)*DeeEnc_Size(encoding));
  if unlikely(!encoded_string) goto err_r;
  *encoded_string++ = len; /* Save the length in the encoded string header. */
  utf->su_enc[encoding] = encoded_string;
#if __SIZEOF_WCHAR_T__ == 2
  if (DeeEnc_Is4Byte(encoding))
#else
  if (!DeeEnc_Is2Byte(encoding))
#endif
  { /* UTF-32 */
   uint32_t *src,*iter,*end;
   src = (uint32_t *)str;
   end = (iter = (uint32_t *)encoded_string)+len;
   for (; iter != end; ++iter,++src) *iter = (uint32_t)DeeUni_SwapCase(*src);
   *iter = 0; /* Terminate a c-style string. */
  } else { /* UTF-16 */
   uint16_t *src,*iter,*end;
   src = (uint16_t *)str;
   end = (iter = (uint16_t *)encoded_string)+len;
   for (; iter != end; ++iter,++src) *iter = (uint16_t)DeeUni_SwapCase(*src);
   *iter = 0; /* Terminate a c-style string. */
  }
  result = (DREF String *)DeeString_SetEncodingBuffer((DREF DeeObject *)result);
 }
 return (DREF DeeObject *)result;
err_r:
 Dee_Decref(result);
 return NULL;
}
#endif /* !CONFIG_USE_NEW_STRING_API */


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_UNICODE_C */
