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
#ifndef GUARD_DEEMON_STRINGUTILS_H
#define GUARD_DEEMON_STRINGUTILS_H 1

#include "api.h"
#include "object.h"
#include "string.h"
#include "util/string.h"

DECL_BEGIN


/* UTF-8 helper API */
DDATDEF uint8_t const utf8_sequence_len[256];
DFUNDEF uint32_t (DCALL utf8_readchar)(char const **__restrict piter, char const *__restrict end);
DFUNDEF uint32_t (DCALL utf8_readchar_u)(char const **__restrict piter);
DFUNDEF uint32_t (DCALL utf8_readchar_rev)(char const **__restrict pend, char const *__restrict begin);
DFUNDEF char *(DCALL utf8_writechar)(char *__restrict buffer, uint32_t ch); /* Up to `UTF8_MAX_MBLEN' bytes may be used in `buffer' */
#define UTF8_MAX_MBLEN  8 /* The max length of a UTF-8 multi-byte sequence (100% future-proof,
                           * as this is the theoretical limit. - The actual limit would be `4') */


LOCAL char *
(DCALL utf8_skipspace)(char const *__restrict str,
                       char const *__restrict end) {
 char *result;
 for (;;) {
  uint32_t chr;
  result = (char *)str;
  chr = utf8_readchar((char const **)&str,end);
  if (!DeeUni_IsSpace(chr)) break;
 }
 return result;
}

LOCAL char *
(DCALL utf8_skipspace_rev)(char const *__restrict end,
                           char const *__restrict begin) {
 char *result;
 for (;;) {
  uint32_t chr;
  result = (char *)end;
  chr = utf8_readchar_rev((char const **)&end,begin);
  if (!DeeUni_IsSpace(chr)) break;
 }
 return result;
}


/* Get/Set a character, given its index within the string. */
#define DeeString_GetChar(self,index)       _DeeString_GetChar((DeeStringObject *)REQUIRES_OBJECT(self),index)
#define DeeString_SetChar(self,index,value) _DeeString_SetChar((DeeStringObject *)REQUIRES_OBJECT(self),index,value)

FORCELOCAL uint32_t DCALL
_DeeString_GetChar(DeeStringObject *__restrict self, size_t index) {
 size_t *str; struct string_utf *utf = self->s_data;
 if (!utf) {
  ASSERT(index <= self->s_len);
  return ((uint8_t *)self->s_str)[index];
 }
 str = utf->u_data[utf->u_width];
 ASSERT(index <= WSTR_LENGTH(str));
 SWITCH_SIZEOF_WIDTH(utf->u_width) {
 CASE_WIDTH_1BYTE: return ((uint8_t *)str)[index];
 CASE_WIDTH_2BYTE: return ((uint16_t *)str)[index];
 CASE_WIDTH_4BYTE: return ((uint32_t *)str)[index];
 }
}
FORCELOCAL void DCALL
_DeeString_SetChar(DeeStringObject *__restrict self,
                   size_t index, uint32_t value) {
 size_t *str;
 struct string_utf *utf = self->s_data;
 if (!utf) {
  ASSERT((index < self->s_len) ||
         (index == self->s_len && !value));
  self->s_str[index] = (uint8_t)value;
 } else {
  str = utf->u_data[utf->u_width];
  ASSERT((index < WSTR_LENGTH(str)) ||
         (index == WSTR_LENGTH(str) && !value));
  SWITCH_SIZEOF_WIDTH(utf->u_width) {
  CASE_WIDTH_1BYTE: ((uint8_t *)str)[index] = (uint8_t)value; break;
  CASE_WIDTH_2BYTE: ((uint16_t *)str)[index] = (uint16_t)value; break;
  CASE_WIDTH_4BYTE: ((uint32_t *)str)[index] = (uint32_t)value; break;
  }
 }
}

#define DeeString_Memmove(self,dst,src,num_chars) \
       _DeeString_Memmove((DeeStringObject *)REQUIRES_OBJECT(self),dst,src,num_chars)
LOCAL void DCALL
_DeeString_Memmove(DeeStringObject *__restrict self,
                   size_t dst, size_t src, size_t num_chars) {
 union dcharptr str;
 struct string_utf *utf = self->s_data;
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
      utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE]) {
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
    if (utf->u_utf16 == (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE])
        utf->u_utf16 = NULL;
    Dee_Free(((size_t *)utf->u_data[STRING_WIDTH_2BYTE])-1);
    utf->u_data[STRING_WIDTH_2BYTE] = NULL;
   }
   if (utf->u_utf16) {
    ASSERT(utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE]);
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

/* Pop the last character of a string, which must be an ASCII character. */
#define DeeString_PopbackAscii(self) \
       _DeeString_PopbackAscii((DeeStringObject *)REQUIRES_OBJECT(self))
LOCAL void DCALL
_DeeString_PopbackAscii(DeeStringObject *__restrict self) {
 struct string_utf *utf = self->s_data;
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
      utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE])
      DEE_PRIVATE_WSTR_DECLEN(utf->u_utf16);
 }
}



#define DeeString_Foreach(self,ibegin,iend,iter,end,...) \
do{ void *_str_ = DeeString_WSTR(self); \
    size_t _len_ = WSTR_LENGTH(_str_); \
    if (_len_ > (iend)) \
        _len_ = (iend); \
    if ((ibegin) < _len_) { \
        switch (DeeString_WIDTH(self)) { \
        { \
            char *iter,*end; \
        CASE_WIDTH_1BYTE: \
            iter = (char *)_str_; \
            end = (char *)_str_ + _len_; \
            for (; iter != end; ++iter) do __VA_ARGS__ __WHILE0; \
        } break; \
        { \
            uint16_t *iter,*end; \
        CASE_WIDTH_2BYTE: \
            iter = (uint16_t *)_str_; \
            end = (uint16_t *)_str_ + _len_; \
            for (; iter != end; ++iter) do __VA_ARGS__ __WHILE0; \
        } break; \
        { \
            uint32_t *iter,*end; \
        CASE_WIDTH_4BYTE: \
            iter = (uint32_t *)_str_; \
            end = (uint32_t *)_str_ + _len_; \
            for (; iter != end; ++iter) do __VA_ARGS__ __WHILE0; \
        } break; \
        } \
    } \
}__WHILE0



DECL_END

#endif /* !GUARD_DEEMON_STRINGUTILS_H */
