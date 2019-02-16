/* Copyright (c) 2019 Griefer@Work                                            *
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

#ifdef DEE_SOURCE
#ifdef __INTELLISENSE__
#define Dee_utf8_sequence_len  utf8_sequence_len
#define Dee_utf8_readchar      utf8_readchar
#define Dee_utf8_readchar_u    utf8_readchar_u
#define Dee_utf8_readchar_rev  utf8_readchar_rev
#define Dee_utf8_writechar     utf8_writechar
#define Dee_utf8_skipspace     utf8_skipspace
#define Dee_utf8_skipspace_rev utf8_skipspace_rev
#else
#define utf8_sequence_len  Dee_utf8_sequence_len
#define utf8_readchar      Dee_utf8_readchar
#define utf8_readchar_u    Dee_utf8_readchar_u
#define utf8_readchar_rev  Dee_utf8_readchar_rev
#define utf8_writechar     Dee_utf8_writechar
#define utf8_skipspace     Dee_utf8_skipspace
#define utf8_skipspace_rev Dee_utf8_skipspace_rev
#endif
#define UTF8_MAX_MBLEN     Dee_UTF8_MAX_MBLEN
#endif

/* UTF-8 helper API */
DDATDEF uint8_t const Dee_utf8_sequence_len[256];
DFUNDEF uint32_t (DCALL Dee_utf8_readchar)(char const **__restrict piter, char const *__restrict end);
DFUNDEF uint32_t (DCALL Dee_utf8_readchar_u)(char const **__restrict piter);
DFUNDEF uint32_t (DCALL Dee_utf8_readchar_rev)(char const **__restrict pend, char const *__restrict begin);
DFUNDEF char *(DCALL Dee_utf8_writechar)(char *__restrict buffer, uint32_t ch); /* Up to `UTF8_MAX_MBLEN' bytes may be used in `buffer' */
#define Dee_UTF8_MAX_MBLEN  8 /* The max length of a UTF-8 multi-byte sequence (100% future-proof,
                               * as this is the theoretical limit. - The actual limit would be `4') */


#ifdef __INTELLISENSE__
extern "C++" {
size_t (DCALL DeeUni_FoldedLength)(uint8_t const *__restrict text, size_t length);
size_t (DCALL DeeUni_FoldedLength)(uint16_t const *__restrict text, size_t length);
size_t (DCALL DeeUni_FoldedLength)(uint32_t const *__restrict text, size_t length);
}
#else
#define DeeUni_FoldedLength(text,length) \
    (sizeof(*(text)) == 1 ? _DeeUni_FoldedLength_b((uint8_t *)(text),length) : \
     sizeof(*(text)) == 2 ? _DeeUni_FoldedLength_w((uint16_t *)(text),length) : \
                            _DeeUni_FoldedLength_l((uint32_t *)(text),length))
LOCAL size_t
(DCALL _DeeUni_FoldedLength_b)(uint8_t *__restrict text, size_t length) {
 size_t i,result = 0;
 uint32_t buf[Dee_UNICODE_FOLDED_MAX];
 for (i = 0; i < length; ++i)
     result += DeeUni_ToFolded(text[i],buf);
 return result;
}
LOCAL size_t
(DCALL _DeeUni_FoldedLength_w)(uint16_t *__restrict text, size_t length) {
 size_t i,result = 0;
 uint32_t buf[Dee_UNICODE_FOLDED_MAX];
 for (i = 0; i < length; ++i)
     result += DeeUni_ToFolded(text[i],buf);
 return result;
}
LOCAL size_t
(DCALL _DeeUni_FoldedLength_l)(uint32_t *__restrict text, size_t length) {
 size_t i,result = 0;
 uint32_t buf[Dee_UNICODE_FOLDED_MAX];
 for (i = 0; i < length; ++i)
     result += DeeUni_ToFolded(text[i],buf);
 return result;
}
#endif


LOCAL char *
(DCALL Dee_utf8_skipspace)(char const *__restrict str,
                           char const *__restrict end) {
 char *result;
 for (;;) {
  uint32_t chr;
  result = (char *)str;
  chr = Dee_utf8_readchar((char const **)&str,end);
  if (!DeeUni_IsSpace(chr)) break;
 }
 return result;
}

LOCAL char *
(DCALL Dee_utf8_skipspace_rev)(char const *__restrict end,
                               char const *__restrict begin) {
 char *result;
 for (;;) {
  uint32_t chr;
  result = (char *)end;
  chr = Dee_utf8_readchar_rev((char const **)&end,begin);
  if (!DeeUni_IsSpace(chr)) break;
 }
 return result;
}


/* Get/Set a character, given its index within the string. */
#define DeeString_GetChar(self,index)       DeeString_GetChar((DeeStringObject *)Dee_REQUIRES_OBJECT(self),index)
#define DeeString_SetChar(self,index,value) DeeString_SetChar((DeeStringObject *)Dee_REQUIRES_OBJECT(self),index,value)
DFUNDEF uint32_t (DCALL DeeString_GetChar)(DeeStringObject *__restrict self, size_t index);
DFUNDEF void (DCALL DeeString_SetChar)(DeeStringObject *__restrict self, size_t index, uint32_t value);


/* Move `num_chars' characters from `src' to `dst' */
#define DeeString_Memmove(self,dst,src,num_chars) \
        DeeString_Memmove((DeeStringObject *)Dee_REQUIRES_OBJECT(self),dst,src,num_chars)
DFUNDEF void (DCALL DeeString_Memmove)(DeeStringObject *__restrict self,
                                       size_t dst, size_t src, size_t num_chars);

/* Pop the last character of a string, which must be an ASCII character. */
#define DeeString_PopbackAscii(self) \
        DeeString_PopbackAscii((DeeStringObject *)Dee_REQUIRES_OBJECT(self))
DFUNDEF void (DCALL DeeString_PopbackAscii)(DeeStringObject *__restrict self);



#define DeeString_Foreach(self,ibegin,iend,iter,end,...) \
do{ void *_str_ = DeeString_WSTR(self); \
    size_t _len_ = WSTR_LENGTH(_str_); \
    if (_len_ > (iend)) \
        _len_ = (iend); \
    if ((ibegin) < _len_) { \
        Dee_SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) { \
        { \
            char *iter,*end; \
        Dee_CASE_WIDTH_1BYTE: \
            iter = (char *)_str_; \
            end = (char *)_str_ + _len_; \
            for (; iter != end; ++iter) do __VA_ARGS__ __WHILE0; \
        } break; \
        { \
            uint16_t *iter,*end; \
        Dee_CASE_WIDTH_2BYTE: \
            iter = (uint16_t *)_str_; \
            end = (uint16_t *)_str_ + _len_; \
            for (; iter != end; ++iter) do __VA_ARGS__ __WHILE0; \
        } break; \
        { \
            uint32_t *iter,*end; \
        Dee_CASE_WIDTH_4BYTE: \
            iter = (uint32_t *)_str_; \
            end = (uint32_t *)_str_ + _len_; \
            for (; iter != end; ++iter) do __VA_ARGS__ __WHILE0; \
        } break; \
        } \
    } \
}__WHILE0



DECL_END

#endif /* !GUARD_DEEMON_STRINGUTILS_H */
