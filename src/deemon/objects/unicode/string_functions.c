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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_C
#define GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_C 1

#include "string_functions.h"

#include <deemon/arg.h>
#include <deemon/tuple.h>
#include <deemon/int.h>
#include <deemon/bool.h>
#include <deemon/seq.h>
#include <deemon/thread.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/bytes.h>

DECL_BEGIN

INTERN DREF DeeObject *DCALL
DeeString_StripSpc(DeeObject *__restrict self) {
 union dcharptr str,new_str; int width; size_t len,new_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 width = DeeString_WIDTH(self);
 str.ptr = new_str.ptr = DeeString_WSTR(self);
 len = new_len = WSTR_LENGTH(str.ptr);
 SWITCH_SIZEOF_WIDTH(width) {
 CASE_WIDTH_1BYTE:
  while (new_len && DeeUni_IsSpace(new_str.cp8[0])) ++new_str.cp8,--new_len;
  while (new_len && DeeUni_IsSpace(new_str.cp8[new_len-1])) --new_len;
  if (new_len == len) goto return_self;
  return DeeString_New1Byte(new_str.cp8,new_len);
 CASE_WIDTH_2BYTE:
  while (new_len && DeeUni_IsSpace(new_str.cp16[0])) ++new_str.cp16,--new_len;
  while (new_len && DeeUni_IsSpace(new_str.cp16[new_len-1])) --new_len;
  if (new_len == len) goto return_self;
  return DeeString_New2Byte(new_str.cp16,new_len);
 CASE_WIDTH_4BYTE:
  while (new_len && DeeUni_IsSpace(new_str.cp32[0])) ++new_str.cp32,--new_len;
  while (new_len && DeeUni_IsSpace(new_str.cp32[new_len-1])) --new_len;
  if (new_len == len) goto return_self;
  return DeeString_New4Byte(new_str.cp32,new_len);
 }
return_self:
 return_reference_(self);
}
INTERN DREF DeeObject *DCALL
DeeString_LStripSpc(DeeObject *__restrict self) {
 union dcharptr str,new_str; int width; size_t len,new_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 width = DeeString_WIDTH(self);
 str.ptr = new_str.ptr = DeeString_WSTR(self);
 len = new_len = WSTR_LENGTH(str.ptr);
 switch (width) {
 CASE_WIDTH_1BYTE:
  while (new_len && DeeUni_IsSpace(new_str.cp8[0])) ++new_str.cp8,--new_len;
  if (new_len == len) goto return_self;
  return DeeString_New1Byte(new_str.cp8,new_len);
 CASE_WIDTH_2BYTE:
  while (new_len && DeeUni_IsSpace(new_str.cp16[0])) ++new_str.cp16,--new_len;
  if (new_len == len) goto return_self;
  return DeeString_New2Byte(new_str.cp16,new_len);
 CASE_WIDTH_4BYTE:
  while (new_len && DeeUni_IsSpace(new_str.cp32[0])) ++new_str.cp32,--new_len;
  if (new_len == len) goto return_self;
  return DeeString_New4Byte(new_str.cp32,new_len);
 }
return_self:
 return_reference_(self);
}
INTERN DREF DeeObject *DCALL
DeeString_RStripSpc(DeeObject *__restrict self) {
 union dcharptr str; int width; size_t len,new_len;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 width = DeeString_WIDTH(self);
 str.ptr = DeeString_WSTR(self);
 len = new_len = WSTR_LENGTH(str.ptr);
 SWITCH_SIZEOF_WIDTH(width) {
 CASE_WIDTH_1BYTE:
  while (new_len && DeeUni_IsSpace(str.cp8[new_len-1])) --new_len;
  if (new_len == len) goto return_self;
  return DeeString_New1Byte(str.cp8,new_len);
 CASE_WIDTH_2BYTE:
  while (new_len && DeeUni_IsSpace(str.cp16[new_len-1])) --new_len;
  if (new_len == len) goto return_self;
  return DeeString_New2Byte(str.cp16,new_len);
 CASE_WIDTH_4BYTE:
  while (new_len && DeeUni_IsSpace(str.cp32[new_len-1])) --new_len;
  if (new_len == len) goto return_self;
  return DeeString_New4Byte(str.cp32,new_len);
 }
return_self:
 return_reference_(self);
}

INTERN DREF DeeObject *DCALL
DeeString_StripMask(DeeObject *__restrict self,
                    DeeObject *__restrict mask) {
 union dcharptr mystr,newstr,mskstr;
 size_t mylen,newlen,msklen;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(mask,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(mask))) {
 CASE_WIDTH_1BYTE:
  mystr.cp8  = DeeString_As1Byte(self);
  mskstr.cp8 = DeeString_As1Byte(mask);
  mylen      = WSTR_LENGTH(mystr.cp8);
  msklen     = WSTR_LENGTH(mskstr.cp8);
  newstr.cp8 = mystr.cp8,newlen = mylen;
  while (newlen && memchrb(mskstr.cp8,newstr.cp8[0],msklen)) ++newstr.cp8,--newlen;
  while (newlen && memchrb(mskstr.cp8,newstr.cp8[newlen-1],msklen)) --newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New1Byte(newstr.cp8,newlen);
 CASE_WIDTH_2BYTE:
  mystr.cp16  = DeeString_As2Byte(self);
  if unlikely(!mystr.cp16) goto err;
  mskstr.cp16 = DeeString_As2Byte(mask);
  if unlikely(!mskstr.cp16) goto err;
  mylen       = WSTR_LENGTH(mystr.cp16);
  msklen      = WSTR_LENGTH(mskstr.cp16);
  newstr.cp16 = mystr.cp16,newlen = mylen;
  while (newlen && memchrw(mskstr.cp16,newstr.cp16[0],msklen)) ++newstr.cp16,--newlen;
  while (newlen && memchrw(mskstr.cp16,newstr.cp16[newlen-1],msklen)) --newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New2Byte(newstr.cp16,newlen);
 CASE_WIDTH_4BYTE:
  mystr.cp32  = DeeString_As4Byte(self);
  if unlikely(!mystr.cp32) goto err;
  mskstr.cp32 = DeeString_As4Byte(mask);
  if unlikely(!mskstr.cp32) goto err;
  mylen       = WSTR_LENGTH(mystr.cp32);
  msklen      = WSTR_LENGTH(mskstr.cp32);
  newstr.cp32 = mystr.cp32,newlen = mylen;
  while (newlen && memchrl(mskstr.cp32,newstr.cp32[0],msklen)) ++newstr.cp32,--newlen;
  while (newlen && memchrl(mskstr.cp32,newstr.cp32[newlen-1],msklen)) --newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New4Byte(newstr.cp32,newlen);
 }
retself:
 return_reference_(self);
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_LStripMask(DeeObject *__restrict self, DeeObject *__restrict mask) {
 union dcharptr mystr,newstr,mskstr;
 size_t mylen,newlen,msklen;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(mask,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(mask))) {
 CASE_WIDTH_1BYTE:
  mystr.cp8  = DeeString_As1Byte(self);
  mskstr.cp8 = DeeString_As1Byte(mask);
  mylen      = WSTR_LENGTH(mystr.cp8);
  msklen     = WSTR_LENGTH(mskstr.cp8);
  newstr.cp8 = mystr.cp8,newlen = mylen;
  while (newlen && memchrb(mskstr.cp8,newstr.cp8[0],msklen)) ++newstr.cp8,--newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New1Byte(newstr.cp8,newlen);
 CASE_WIDTH_2BYTE:
  mystr.cp16  = DeeString_As2Byte(self);
  if unlikely(!mystr.cp16) goto err;
  mskstr.cp16 = DeeString_As2Byte(mask);
  if unlikely(!mskstr.cp16) goto err;
  mylen       = WSTR_LENGTH(mystr.cp16);
  msklen      = WSTR_LENGTH(mskstr.cp16);
  newstr.cp16 = mystr.cp16,newlen = mylen;
  while (newlen && memchrw(mskstr.cp16,newstr.cp16[0],msklen)) ++newstr.cp16,--newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New2Byte(newstr.cp16,newlen);
 CASE_WIDTH_4BYTE:
  mystr.cp32  = DeeString_As4Byte(self);
  if unlikely(!mystr.cp32) goto err;
  mskstr.cp32 = DeeString_As4Byte(mask);
  if unlikely(!mskstr.cp32) goto err;
  mylen       = WSTR_LENGTH(mystr.cp32);
  msklen      = WSTR_LENGTH(mskstr.cp32);
  newstr.cp32 = mystr.cp32,newlen = mylen;
  while (newlen && memchrl(mskstr.cp32,newstr.cp32[0],msklen)) ++newstr.cp32,--newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New4Byte(newstr.cp32,newlen);
 }
retself:
 return_reference_(self);
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_RStripMask(DeeObject *__restrict self, DeeObject *__restrict mask) {
 union dcharptr mystr,mskstr;
 size_t mylen,newlen,msklen;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(mask,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(mask))) {
 CASE_WIDTH_1BYTE:
  mystr.cp8  = DeeString_As1Byte(self);
  mskstr.cp8 = DeeString_As1Byte(mask);
  mylen      = WSTR_LENGTH(mystr.cp8);
  msklen     = WSTR_LENGTH(mskstr.cp8);
  newlen     = mylen;
  while (newlen && memchrb(mskstr.cp8,mystr.cp8[newlen-1],msklen)) --newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New1Byte(mystr.cp8,newlen);
 CASE_WIDTH_2BYTE:
  mystr.cp16  = DeeString_As2Byte(self);
  if unlikely(!mystr.cp16) goto err;
  mskstr.cp16 = DeeString_As2Byte(mask);
  if unlikely(!mskstr.cp16) goto err;
  mylen       = WSTR_LENGTH(mystr.cp16);
  msklen      = WSTR_LENGTH(mskstr.cp16);
  newlen      = mylen;
  while (newlen && memchrw(mskstr.cp16,mystr.cp16[newlen-1],msklen)) --newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New2Byte(mystr.cp16,newlen);
 CASE_WIDTH_4BYTE:
  mystr.cp32  = DeeString_As4Byte(self);
  if unlikely(!mystr.cp32) goto err;
  mskstr.cp32 = DeeString_As4Byte(mask);
  if unlikely(!mskstr.cp32) goto err;
  mylen       = WSTR_LENGTH(mystr.cp32);
  msklen      = WSTR_LENGTH(mskstr.cp32);
  newlen      = mylen;
  while (newlen && memchrl(mskstr.cp32,mystr.cp32[newlen-1],msklen)) --newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New4Byte(mystr.cp32,newlen);
 }
retself:
 return_reference_(self);
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_CaseStripMask(DeeObject *__restrict self,
                        DeeObject *__restrict mask) {
 union dcharptr mystr,newstr,mskstr;
 size_t mylen,newlen,msklen;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(mask,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(mask))) {
 CASE_WIDTH_1BYTE:
  mystr.cp8  = DeeString_As1Byte(self);
  mskstr.cp8 = DeeString_As1Byte(mask);
  mylen      = WSTR_LENGTH(mystr.cp8);
  msklen     = WSTR_LENGTH(mskstr.cp8);
  newstr.cp8 = mystr.cp8,newlen = mylen;
  while (newlen && memcasechrb(mskstr.cp8,newstr.cp8[0],msklen)) ++newstr.cp8,--newlen;
  while (newlen && memcasechrb(mskstr.cp8,newstr.cp8[newlen-1],msklen)) --newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New1Byte(newstr.cp8,newlen);
 CASE_WIDTH_2BYTE:
  mystr.cp16  = DeeString_As2Byte(self);
  if unlikely(!mystr.cp16) goto err;
  mskstr.cp16 = DeeString_As2Byte(mask);
  if unlikely(!mskstr.cp16) goto err;
  mylen       = WSTR_LENGTH(mystr.cp16);
  msklen      = WSTR_LENGTH(mskstr.cp16);
  newstr.cp16 = mystr.cp16,newlen = mylen;
  while (newlen && memcasechrw(mskstr.cp16,newstr.cp16[0],msklen)) ++newstr.cp16,--newlen;
  while (newlen && memcasechrw(mskstr.cp16,newstr.cp16[newlen-1],msklen)) --newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New2Byte(newstr.cp16,newlen);
 CASE_WIDTH_4BYTE:
  mystr.cp32  = DeeString_As4Byte(self);
  if unlikely(!mystr.cp32) goto err;
  mskstr.cp32 = DeeString_As4Byte(mask);
  if unlikely(!mskstr.cp32) goto err;
  mylen       = WSTR_LENGTH(mystr.cp32);
  msklen      = WSTR_LENGTH(mskstr.cp32);
  newstr.cp32 = mystr.cp32,newlen = mylen;
  while (newlen && memcasechrl(mskstr.cp32,newstr.cp32[0],msklen)) ++newstr.cp32,--newlen;
  while (newlen && memcasechrl(mskstr.cp32,newstr.cp32[newlen-1],msklen)) --newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New4Byte(newstr.cp32,newlen);
 }
retself:
 return_reference_(self);
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_CaseLStripMask(DeeObject *__restrict self, DeeObject *__restrict mask) {
 union dcharptr mystr,newstr,mskstr;
 size_t mylen,newlen,msklen;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(mask,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(mask))) {
 CASE_WIDTH_1BYTE:
  mystr.cp8  = DeeString_As1Byte(self);
  mskstr.cp8 = DeeString_As1Byte(mask);
  mylen      = WSTR_LENGTH(mystr.cp8);
  msklen     = WSTR_LENGTH(mskstr.cp8);
  newstr.cp8 = mystr.cp8,newlen = mylen;
  while (newlen && memcasechrb(mskstr.cp8,newstr.cp8[0],msklen)) ++newstr.cp8,--newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New1Byte(newstr.cp8,newlen);
 CASE_WIDTH_2BYTE:
  mystr.cp16  = DeeString_As2Byte(self);
  if unlikely(!mystr.cp16) goto err;
  mskstr.cp16 = DeeString_As2Byte(mask);
  if unlikely(!mskstr.cp16) goto err;
  mylen       = WSTR_LENGTH(mystr.cp16);
  msklen      = WSTR_LENGTH(mskstr.cp16);
  newstr.cp16 = mystr.cp16,newlen = mylen;
  while (newlen && memcasechrw(mskstr.cp16,newstr.cp16[0],msklen)) ++newstr.cp16,--newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New2Byte(newstr.cp16,newlen);
 CASE_WIDTH_4BYTE:
  mystr.cp32  = DeeString_As4Byte(self);
  if unlikely(!mystr.cp32) goto err;
  mskstr.cp32 = DeeString_As4Byte(mask);
  if unlikely(!mskstr.cp32) goto err;
  mylen       = WSTR_LENGTH(mystr.cp32);
  msklen      = WSTR_LENGTH(mskstr.cp32);
  newstr.cp32 = mystr.cp32,newlen = mylen;
  while (newlen && memcasechrl(mskstr.cp32,newstr.cp32[0],msklen)) ++newstr.cp32,--newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New4Byte(newstr.cp32,newlen);
 }
retself:
 return_reference_(self);
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_CaseRStripMask(DeeObject *__restrict self, DeeObject *__restrict mask) {
 union dcharptr mystr,mskstr;
 size_t mylen,newlen,msklen;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(mask,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(mask))) {
 CASE_WIDTH_1BYTE:
  mystr.cp8  = DeeString_As1Byte(self);
  mskstr.cp8 = DeeString_As1Byte(mask);
  mylen      = WSTR_LENGTH(mystr.cp8);
  msklen     = WSTR_LENGTH(mskstr.cp8);
  newlen     = mylen;
  while (newlen && memcasechrb(mskstr.cp8,mystr.cp8[newlen-1],msklen)) --newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New1Byte(mystr.cp8,newlen);
 CASE_WIDTH_2BYTE:
  mystr.cp16  = DeeString_As2Byte(self);
  if unlikely(!mystr.cp16) goto err;
  mskstr.cp16 = DeeString_As2Byte(mask);
  if unlikely(!mskstr.cp16) goto err;
  mylen       = WSTR_LENGTH(mystr.cp16);
  msklen      = WSTR_LENGTH(mskstr.cp16);
  newlen      = mylen;
  while (newlen && memcasechrw(mskstr.cp16,mystr.cp16[newlen-1],msklen)) --newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New2Byte(mystr.cp16,newlen);
 CASE_WIDTH_4BYTE:
  mystr.cp32  = DeeString_As4Byte(self);
  if unlikely(!mystr.cp32) goto err;
  mskstr.cp32 = DeeString_As4Byte(mask);
  if unlikely(!mskstr.cp32) goto err;
  mylen       = WSTR_LENGTH(mystr.cp32);
  msklen      = WSTR_LENGTH(mskstr.cp32);
  newlen      = mylen;
  while (newlen && memcasechrl(mskstr.cp32,mystr.cp32[newlen-1],msklen)) --newlen;
  if (newlen == mylen) goto retself;
  return DeeString_New4Byte(mystr.cp32,newlen);
 }
retself:
 return_reference_(self);
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_Reversed(DeeObject *__restrict self,
                   size_t begin, size_t end) {
 DREF DeeObject *result; int width;
 uint8_t *my_str,*dst; size_t flip_size;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 width = DeeString_WIDTH(self);
 my_str   = (uint8_t *)DeeString_WSTR(self);
 if (end > WSTR_LENGTH(my_str))
     end = WSTR_LENGTH(my_str);
 if (end <= begin) return_empty_string; /* Empty string area. */
 flip_size = (size_t)(end-begin);
 ASSERT(flip_size);
 /* Actually perform the search for the given string. */
 SWITCH_SIZEOF_WIDTH (width) {
 CASE_WIDTH_1BYTE:
  result = DeeString_NewBuffer(flip_size);
  if unlikely(!result) goto err;
  dst = (uint8_t *)DeeString_STR(result);
  do {
   --flip_size;
   *(uint8_t *)dst = ((uint8_t *)my_str)[flip_size];
   dst += 1;
  } while (flip_size);
  break;
 {
  uint16_t *buf;
 CASE_WIDTH_2BYTE:
  buf = DeeString_NewBuffer16(flip_size);
  if unlikely(!buf) goto err;
  dst = (uint8_t *)buf;
  do {
   --flip_size;
   *(uint16_t *)dst = ((uint16_t *)my_str)[flip_size];
   dst += 2;
  } while (flip_size);
  result = DeeString_Pack2ByteBuffer(buf);
 } break;
 {
  uint32_t *buf;
 CASE_WIDTH_4BYTE:
  buf = DeeString_NewBuffer32(flip_size);
  if unlikely(!buf) goto err;
  dst = (uint8_t *)buf;
  do {
   --flip_size;
   *(uint32_t *)dst = ((uint32_t *)my_str)[flip_size];
   dst += 4;
  } while (flip_size);
  result = DeeString_Pack4ByteBuffer(buf);
 } break;
 }
 return result;
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_ExpandTabs(DeeObject *__restrict self, size_t tab_width) {
 union dcharptr iter,end,flush_start; size_t line_inset = 0;
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
 CASE_WIDTH_1BYTE:
  iter.cp8 = DeeString_Get1Byte(self);
  end.cp8  = iter.cp8 + WSTR_LENGTH(iter.cp8);
  flush_start.cp8 = iter.cp8;
  for (; iter.cp8 < end.cp8; ++iter.cp8) {
   uint8_t ch = *iter.cp8;
   if (!DeeUni_IsTab(ch)) {
    ++line_inset;
    if (DeeUni_IsLF(ch))
        line_inset = 0; /* Reset insets at line starts. */
    continue;
   }
   if (unicode_printer_print8(&printer,flush_start.cp8,
                             (size_t)(iter.cp8-flush_start.cp8)) < 0)
       goto err;
   /* Replace with white-space. */
   if likely(tab_width) {
    line_inset = tab_width - (line_inset % tab_width);
    if (unicode_printer_repeatascii(&printer,UNICODE_SPACE,line_inset) < 0)
        goto err;
    line_inset = 0;
   }
   flush_start.cp8 = iter.cp8+1;
  }
  if (!UNICODE_PRINTER_LENGTH(&printer))
       goto retself;
  if (unicode_printer_print8(&printer,flush_start.cp8,
                            (size_t)(iter.cp8-flush_start.cp8)) < 0)
      goto err;
  break;
 CASE_WIDTH_2BYTE:
  iter.cp16 = DeeString_Get2Byte(self);
  end.cp16  = iter.cp16 + WSTR_LENGTH(iter.cp16);
  flush_start.cp16 = iter.cp16;
  for (; iter.cp16 < end.cp16; ++iter.cp16) {
   uint16_t ch = *iter.cp16;
   if (!DeeUni_IsTab(ch)) {
    ++line_inset;
    if (DeeUni_IsLF(ch))
        line_inset = 0; /* Reset insets at line starts. */
    continue;
   }
   if (unicode_printer_print16(&printer,flush_start.cp16,
                              (size_t)(iter.cp16-flush_start.cp16)) < 0)
       goto err;
   /* Replace with white-space. */
   if likely(tab_width) {
    line_inset = tab_width - (line_inset % tab_width);
    if (unicode_printer_repeatascii(&printer,UNICODE_SPACE,line_inset) < 0)
        goto err;
    line_inset = 0;
   }
   flush_start.cp16 = iter.cp16+1;
  }
  if (!UNICODE_PRINTER_LENGTH(&printer))
       goto retself;
  if (unicode_printer_print16(&printer,flush_start.cp16,
                             (size_t)(iter.cp16-flush_start.cp16)) < 0)
      goto err;
  break;
 CASE_WIDTH_4BYTE:
  iter.cp32 = DeeString_Get4Byte(self);
  end.cp32  = iter.cp32 + WSTR_LENGTH(iter.cp32);
  flush_start.cp32 = iter.cp32;
  for (; iter.cp32 < end.cp32; ++iter.cp32) {
   uint32_t ch = *iter.cp32;
   if (!DeeUni_IsTab(ch)) {
    ++line_inset;
    if (DeeUni_IsLF(ch))
        line_inset = 0; /* Reset insets at line starts. */
    continue;
   }
   if (unicode_printer_print32(&printer,flush_start.cp32,
                              (size_t)(iter.cp32-flush_start.cp32)) < 0)
       goto err;
   /* Replace with white-space. */
   if likely(tab_width) {
    line_inset = tab_width - (line_inset % tab_width);
    if (unicode_printer_repeatascii(&printer,UNICODE_SPACE,line_inset) < 0)
        goto err;
    line_inset = 0;
   }
   flush_start.cp32 = iter.cp32+1;
  }
  if (!UNICODE_PRINTER_LENGTH(&printer))
       goto retself;
  if (unicode_printer_print32(&printer,flush_start.cp32,
                             (size_t)(iter.cp32-flush_start.cp32)) < 0)
      goto err;
  break;
 }
 return unicode_printer_pack(&printer);
retself:
 unicode_printer_fini(&printer);
 return_reference_(self);
err:
 unicode_printer_fini(&printer);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_UnifyLines(DeeObject *__restrict self,
                     DeeObject *__restrict replacement) {
 union dcharptr iter,end,flush_start;
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
 CASE_WIDTH_1BYTE:
  iter.cp8 = DeeString_Get1Byte(self);
  end.cp8  = iter.cp8 + WSTR_LENGTH(iter.cp8);
  flush_start.cp8 = iter.cp8;
  for (; iter.cp8 < end.cp8; ++iter.cp8) {
   uint8_t ch = *iter.cp8;
   if (!DeeUni_IsLF(ch)) continue;
   if (unicode_printer_print8(&printer,flush_start.cp8,
                             (size_t)(iter.cp8-flush_start.cp8)) < 0 ||
       unicode_printer_printstring(&printer,replacement) < 0)
       goto err;
   if (ch == UNICODE_CR && iter.cp8[1] == UNICODE_LF) ++iter.cp8;
   flush_start.cp8 = iter.cp8+1;
  }
  if (!UNICODE_PRINTER_LENGTH(&printer))
       goto retself;
  if (unicode_printer_print8(&printer,flush_start.cp8,
                            (size_t)(iter.cp8-flush_start.cp8)) < 0)
      goto err;
  break;
 CASE_WIDTH_2BYTE:
  iter.cp16 = DeeString_Get2Byte(self);
  end.cp16  = iter.cp16 + WSTR_LENGTH(iter.cp16);
  flush_start.cp16 = iter.cp16;
  for (; iter.cp16 < end.cp16; ++iter.cp16) {
   uint16_t ch = *iter.cp16;
   if (!DeeUni_IsLF(ch)) continue;
   if (unicode_printer_print16(&printer,flush_start.cp16,
                              (size_t)(iter.cp16-flush_start.cp16)) < 0 ||
       unicode_printer_printstring(&printer,replacement) < 0)
       goto err;
   if (ch == UNICODE_CR && iter.cp16[1] == UNICODE_LF) ++iter.cp16;
   flush_start.cp16 = iter.cp16+1;
  }
  if (!UNICODE_PRINTER_LENGTH(&printer))
       goto retself;
  if (unicode_printer_print16(&printer,flush_start.cp16,
                             (size_t)(iter.cp16-flush_start.cp16)) < 0)
      goto err;
  break;
 CASE_WIDTH_4BYTE:
  iter.cp32 = DeeString_Get4Byte(self);
  end.cp32  = iter.cp32 + WSTR_LENGTH(iter.cp32);
  flush_start.cp32 = iter.cp32;
  for (; iter.cp32 < end.cp32; ++iter.cp32) {
   uint32_t ch = *iter.cp32;
   if (!DeeUni_IsLF(ch)) continue;
   if (unicode_printer_print32(&printer,flush_start.cp32,
                              (size_t)(iter.cp32-flush_start.cp32)) < 0 ||
       unicode_printer_printstring(&printer,replacement) < 0)
       goto err;
   if (ch == UNICODE_CR && iter.cp32[1] == UNICODE_LF) ++iter.cp32;
   flush_start.cp32 = iter.cp32+1;
  }
  if (!UNICODE_PRINTER_LENGTH(&printer))
       goto retself;
  if (unicode_printer_print32(&printer,flush_start.cp32,
                             (size_t)(iter.cp32-flush_start.cp32)) < 0)
      goto err;
  break;
 }
 return unicode_printer_pack(&printer);
retself:
 unicode_printer_fini(&printer);
 return_reference_(self);
err:
 unicode_printer_fini(&printer);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_UnifyLinesLf(DeeObject *__restrict self) {
 union dcharptr iter,end,flush_start;
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
 CASE_WIDTH_1BYTE:
  iter.cp8 = DeeString_Get1Byte(self);
  end.cp8  = iter.cp8 + WSTR_LENGTH(iter.cp8);
  flush_start.cp8 = iter.cp8;
  for (; iter.cp8 < end.cp8; ++iter.cp8) {
   uint8_t ch = *iter.cp8;
   if (!DeeUni_IsLF(ch)) continue;
   if (ch == UNICODE_LF) continue;
   if (unicode_printer_print8(&printer,flush_start.cp8,
                             (size_t)(iter.cp8-flush_start.cp8)) < 0 ||
       unicode_printer_putascii(&printer,UNICODE_LF) < 0)
       goto err;
   if (ch == UNICODE_CR && iter.cp8[1] == UNICODE_LF) ++iter.cp8;
   flush_start.cp8 = iter.cp8+1;
  }
  if (!UNICODE_PRINTER_LENGTH(&printer))
       goto retself;
  if (unicode_printer_print8(&printer,flush_start.cp8,
                            (size_t)(iter.cp8-flush_start.cp8)) < 0)
      goto err;
  break;
 CASE_WIDTH_2BYTE:
  iter.cp16 = DeeString_Get2Byte(self);
  end.cp16  = iter.cp16 + WSTR_LENGTH(iter.cp16);
  flush_start.cp16 = iter.cp16;
  for (; iter.cp16 < end.cp16; ++iter.cp16) {
   uint16_t ch = *iter.cp16;
   if (!DeeUni_IsLF(ch)) continue;
   if (ch == UNICODE_LF) continue;
   if (unicode_printer_print16(&printer,flush_start.cp16,
                              (size_t)(iter.cp16-flush_start.cp16)) < 0 ||
       unicode_printer_putascii(&printer,UNICODE_LF) < 0)
       goto err;
   if (ch == UNICODE_CR && iter.cp16[1] == UNICODE_LF) ++iter.cp16;
   flush_start.cp16 = iter.cp16+1;
  }
  if (!UNICODE_PRINTER_LENGTH(&printer))
       goto retself;
  if (unicode_printer_print16(&printer,flush_start.cp16,
                             (size_t)(iter.cp16-flush_start.cp16)) < 0)
      goto err;
  break;
 CASE_WIDTH_4BYTE:
  iter.cp32 = DeeString_Get4Byte(self);
  end.cp32  = iter.cp32 + WSTR_LENGTH(iter.cp32);
  flush_start.cp32 = iter.cp32;
  for (; iter.cp32 < end.cp32; ++iter.cp32) {
   uint32_t ch = *iter.cp32;
   if (!DeeUni_IsLF(ch)) continue;
   if (ch == UNICODE_LF) continue;
   if (unicode_printer_print32(&printer,flush_start.cp32,
                              (size_t)(iter.cp32-flush_start.cp32)) < 0 ||
       unicode_printer_putascii(&printer,UNICODE_LF) < 0)
       goto err;
   if (ch == UNICODE_CR && iter.cp32[1] == UNICODE_LF) ++iter.cp32;
   flush_start.cp32 = iter.cp32+1;
  }
  if (!UNICODE_PRINTER_LENGTH(&printer))
       goto retself;
  if (unicode_printer_print32(&printer,flush_start.cp32,
                             (size_t)(iter.cp32-flush_start.cp32)) < 0)
      goto err;
  break;
 }
 return unicode_printer_pack(&printer);
retself:
 unicode_printer_fini(&printer);
 return_reference_(self);
err:
 unicode_printer_fini(&printer);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_Join(DeeObject *__restrict self, DeeObject *__restrict seq) {
 DREF DeeObject *iter,*elem;
 bool is_first = true; size_t fast_size;
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 fast_size = DeeFastSeq_GetSize(seq);
 if (fast_size != DEE_FASTSEQ_NOTFAST) {
  /* Fast-sequence optimizations. */
  size_t i;
  for (i = 0; i < fast_size; ++i) {
   /* Print `self' prior to every object, starting with the 2nd one. */
   if unlikely(!is_first &&
                unicode_printer_printstring(&printer,self) < 0)
      goto err;
   elem = DeeFastSeq_GetItem(seq,i);
   if unlikely(!elem) goto err;
   if unlikely(unicode_printer_printobject(&printer,elem) < 0)
      goto err_elem_noiter;
   Dee_Decref(elem);
   is_first = false;
  }
 } else {
  iter = DeeObject_IterSelf(seq);
  if unlikely(!iter) goto err;
  while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
   /* Print `self' prior to every object, starting with the 2nd one. */
   if unlikely(!is_first &&
                unicode_printer_printstring(&printer,self) < 0)
      goto err_elem;
   if unlikely(unicode_printer_printobject(&printer,elem) < 0)
      goto err_elem;
   Dee_Decref(elem);
   is_first = false;
   if (DeeThread_CheckInterrupt())
       goto err_iter;
  }
  if unlikely(!elem) goto err_iter;
  Dee_Decref(iter);
 }
 return unicode_printer_pack(&printer);
err_elem_noiter:
 Dee_Decref(elem);
 goto err;
err_elem:
 Dee_Decref(elem);
err_iter:
 Dee_Decref(iter);
err:
 unicode_printer_fini(&printer);
 return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_SStrip(DeeObject *__restrict self,
                 DeeObject *__restrict other) {
 union dcharptr mystr,newstr,otherstr;
 size_t mylen,newlen,otherlen;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  mystr.cp8    = DeeString_As1Byte(self);
  otherstr.cp8 = DeeString_As1Byte(other);
  mylen        = WSTR_LENGTH(mystr.cp8);
  otherlen     = WSTR_LENGTH(otherstr.cp8);
  if unlikely(!otherlen) goto retself;
  newstr.cp8 = mystr.cp8,newlen = mylen;
  while (newlen >= otherlen &&
         MEMEQB(newstr.cp8,otherstr.cp8,otherlen))
         newstr.cp8 += otherlen,newlen -= otherlen;
  while (newlen >= otherlen &&
         MEMEQB(newstr.cp8 + (newlen - otherlen),
                otherstr.cp8,otherlen))
         newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  return DeeString_New1Byte(newstr.cp8,newlen);
 CASE_WIDTH_2BYTE:
  mystr.cp16    = DeeString_As2Byte(self);
  if unlikely(!mystr.cp16) goto err;
  otherstr.cp16 = DeeString_As2Byte(other);
  if unlikely(!otherstr.cp16) goto err;
  mylen         = WSTR_LENGTH(mystr.cp16);
  otherlen      = WSTR_LENGTH(otherstr.cp16);
  if unlikely(!otherlen) goto retself;
  newstr.cp16 = mystr.cp16,newlen = mylen;
  while (newlen >= otherlen &&
         MEMEQB(newstr.cp16,otherstr.cp16,otherlen))
         newstr.cp16 += otherlen,newlen -= otherlen;
  while (newlen >= otherlen &&
         MEMEQB(newstr.cp16 + (newlen - otherlen),
                otherstr.cp16,otherlen))
         newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
      return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp16 - mystr.cp16),newlen);
  return DeeString_New2Byte(newstr.cp16,newlen);
 CASE_WIDTH_4BYTE:
  mystr.cp32    = DeeString_As4Byte(self);
  if unlikely(!mystr.cp32) goto err;
  otherstr.cp32 = DeeString_As4Byte(other);
  if unlikely(!otherstr.cp32) goto err;
  mylen         = WSTR_LENGTH(mystr.cp32);
  otherlen      = WSTR_LENGTH(otherstr.cp32);
  if unlikely(!otherlen) goto retself;
  newstr.cp32 = mystr.cp32,newlen = mylen;
  while (newlen >= otherlen &&
         MEMEQB(newstr.cp32,otherstr.cp32,otherlen))
         newstr.cp32 += otherlen,newlen -= otherlen;
  while (newlen >= otherlen &&
         MEMEQB(newstr.cp32 + (newlen - otherlen),
                otherstr.cp32,otherlen))
         newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
      return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp32 - mystr.cp32),newlen);
  if (DeeString_WIDTH(self) == STRING_WIDTH_2BYTE)
      return DeeString_New2Byte(DeeString_Get2Byte(self) + (newstr.cp32 - mystr.cp32),newlen);
  return DeeString_New4Byte(newstr.cp32,newlen);
 }
retself:
 return_reference_(self);
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_LSStrip(DeeObject *__restrict self,
                  DeeObject *__restrict other) {
 union dcharptr mystr,newstr,otherstr;
 size_t mylen,newlen,otherlen;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  mystr.cp8    = DeeString_As1Byte(self);
  otherstr.cp8 = DeeString_As1Byte(other);
  mylen        = WSTR_LENGTH(mystr.cp8);
  otherlen     = WSTR_LENGTH(otherstr.cp8);
  if unlikely(!otherlen) goto retself;
  newstr.cp8 = mystr.cp8,newlen = mylen;
  while (newlen >= otherlen &&
         MEMEQB(newstr.cp8,otherstr.cp8,otherlen))
         newstr.cp8 += otherlen,newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  return DeeString_New1Byte(newstr.cp8,newlen);
 CASE_WIDTH_2BYTE:
  mystr.cp16    = DeeString_As2Byte(self);
  if unlikely(!mystr.cp16) goto err;
  otherstr.cp16 = DeeString_As2Byte(other);
  if unlikely(!otherstr.cp16) goto err;
  mylen         = WSTR_LENGTH(mystr.cp16);
  otherlen      = WSTR_LENGTH(otherstr.cp16);
  if unlikely(!otherlen) goto retself;
  newstr.cp16 = mystr.cp16,newlen = mylen;
  while (newlen >= otherlen &&
         MEMEQB(newstr.cp16,otherstr.cp16,otherlen))
         newstr.cp16 += otherlen,newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
      return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp16 - mystr.cp16),newlen);
  return DeeString_New2Byte(newstr.cp16,newlen);
 CASE_WIDTH_4BYTE:
  mystr.cp32    = DeeString_As4Byte(self);
  if unlikely(!mystr.cp32) goto err;
  otherstr.cp32 = DeeString_As4Byte(other);
  if unlikely(!otherstr.cp32) goto err;
  mylen         = WSTR_LENGTH(mystr.cp32);
  otherlen      = WSTR_LENGTH(otherstr.cp32);
  if unlikely(!otherlen) goto retself;
  newstr.cp32 = mystr.cp32,newlen = mylen;
  while (newlen >= otherlen &&
         MEMEQB(newstr.cp32,otherstr.cp32,otherlen))
         newstr.cp32 += otherlen,newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
      return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp32 - mystr.cp32),newlen);
  if (DeeString_WIDTH(self) == STRING_WIDTH_2BYTE)
      return DeeString_New2Byte(DeeString_Get2Byte(self) + (newstr.cp32 - mystr.cp32),newlen);
  return DeeString_New4Byte(newstr.cp32,newlen);
 }
retself:
 return_reference_(self);
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_RSStrip(DeeObject *__restrict self,
                  DeeObject *__restrict other) {
 union dcharptr mystr,otherstr;
 size_t mylen,newlen,otherlen;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  mystr.cp8    = DeeString_As1Byte(self);
  otherstr.cp8 = DeeString_As1Byte(other);
  mylen        = WSTR_LENGTH(mystr.cp8);
  otherlen     = WSTR_LENGTH(otherstr.cp8);
  if unlikely(!otherlen) goto retself;
  newlen       = mylen;
  while (newlen >= otherlen &&
         MEMEQB(mystr.cp8 + (newlen - otherlen),
                otherstr.cp8,otherlen))
         newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  return DeeString_New1Byte(mystr.cp8,newlen);
 CASE_WIDTH_2BYTE:
  mystr.cp16    = DeeString_As2Byte(self);
  if unlikely(!mystr.cp16) goto err;
  otherstr.cp16 = DeeString_As2Byte(other);
  if unlikely(!otherstr.cp16) goto err;
  mylen         = WSTR_LENGTH(mystr.cp16);
  otherlen      = WSTR_LENGTH(otherstr.cp16);
  if unlikely(!otherlen) goto retself;
  newlen        = mylen;
  while (newlen >= otherlen &&
         MEMEQW(mystr.cp16 + (newlen - otherlen),
                otherstr.cp16,otherlen))
         newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
      return DeeString_New1Byte(DeeString_Get1Byte(self) + (mystr.cp16 - mystr.cp16),newlen);
  return DeeString_New2Byte(mystr.cp16,newlen);
 CASE_WIDTH_4BYTE:
  mystr.cp32    = DeeString_As4Byte(self);
  if unlikely(!mystr.cp32) goto err;
  otherstr.cp32 = DeeString_As4Byte(other);
  if unlikely(!otherstr.cp32) goto err;
  mylen         = WSTR_LENGTH(mystr.cp32);
  otherlen      = WSTR_LENGTH(otherstr.cp32);
  if unlikely(!otherlen) goto retself;
  newlen        = mylen;
  while (newlen >= otherlen &&
         MEMEQL(mystr.cp32 + (newlen - otherlen),
                otherstr.cp32,otherlen))
         newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
      return DeeString_New1Byte(DeeString_Get1Byte(self) + (mystr.cp32 - mystr.cp32),newlen);
  if (DeeString_WIDTH(self) == STRING_WIDTH_2BYTE)
      return DeeString_New2Byte(DeeString_Get2Byte(self) + (mystr.cp32 - mystr.cp32),newlen);
  return DeeString_New4Byte(mystr.cp32,newlen);
 }
retself:
 return_reference_(self);
err:
 return NULL;
}



INTERN DREF DeeObject *DCALL
DeeString_CaseSStrip(DeeObject *__restrict self,
                     DeeObject *__restrict other) {
 union dcharptr mystr,newstr,otherstr;
 size_t mylen,newlen,otherlen;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  mystr.cp8    = DeeString_As1Byte(self);
  otherstr.cp8 = DeeString_As1Byte(other);
  mylen        = WSTR_LENGTH(mystr.cp8);
  otherlen     = WSTR_LENGTH(otherstr.cp8);
  if unlikely(!otherlen) goto retself;
  newstr.cp8 = mystr.cp8,newlen = mylen;
  while (newlen >= otherlen &&
         MEMCASEEQB(newstr.cp8,otherstr.cp8,otherlen))
         newstr.cp8 += otherlen,newlen -= otherlen;
  while (newlen >= otherlen &&
         MEMCASEEQB(newstr.cp8 + (newlen - otherlen),
                    otherstr.cp8,otherlen))
         newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  return DeeString_New1Byte(newstr.cp8,newlen);
 CASE_WIDTH_2BYTE:
  mystr.cp16    = DeeString_As2Byte(self);
  if unlikely(!mystr.cp16) goto err;
  otherstr.cp16 = DeeString_As2Byte(other);
  if unlikely(!otherstr.cp16) goto err;
  mylen         = WSTR_LENGTH(mystr.cp16);
  otherlen      = WSTR_LENGTH(otherstr.cp16);
  if unlikely(!otherlen) goto retself;
  newstr.cp16 = mystr.cp16,newlen = mylen;
  while (newlen >= otherlen &&
         MEMCASEEQB(newstr.cp16,otherstr.cp16,otherlen))
         newstr.cp16 += otherlen,newlen -= otherlen;
  while (newlen >= otherlen &&
         MEMCASEEQB(newstr.cp16 + (newlen - otherlen),
                    otherstr.cp16,otherlen))
         newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
      return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp16 - mystr.cp16),newlen);
  return DeeString_New2Byte(newstr.cp16,newlen);
 CASE_WIDTH_4BYTE:
  mystr.cp32    = DeeString_As4Byte(self);
  if unlikely(!mystr.cp32) goto err;
  otherstr.cp32 = DeeString_As4Byte(other);
  if unlikely(!otherstr.cp32) goto err;
  mylen         = WSTR_LENGTH(mystr.cp32);
  otherlen      = WSTR_LENGTH(otherstr.cp32);
  if unlikely(!otherlen) goto retself;
  newstr.cp32 = mystr.cp32,newlen = mylen;
  while (newlen >= otherlen &&
         MEMCASEEQB(newstr.cp32,otherstr.cp32,otherlen))
         newstr.cp32 += otherlen,newlen -= otherlen;
  while (newlen >= otherlen &&
         MEMCASEEQB(newstr.cp32 + (newlen - otherlen),
                    otherstr.cp32,otherlen))
         newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
      return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp32 - mystr.cp32),newlen);
  if (DeeString_WIDTH(self) == STRING_WIDTH_2BYTE)
      return DeeString_New2Byte(DeeString_Get2Byte(self) + (newstr.cp32 - mystr.cp32),newlen);
  return DeeString_New4Byte(newstr.cp32,newlen);
 }
retself:
 return_reference_(self);
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_CaseLSStrip(DeeObject *__restrict self,
                      DeeObject *__restrict other) {
 union dcharptr mystr,newstr,otherstr;
 size_t mylen,newlen,otherlen;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  mystr.cp8    = DeeString_As1Byte(self);
  otherstr.cp8 = DeeString_As1Byte(other);
  mylen        = WSTR_LENGTH(mystr.cp8);
  otherlen     = WSTR_LENGTH(otherstr.cp8);
  if unlikely(!otherlen) goto retself;
  newstr.cp8 = mystr.cp8,newlen = mylen;
  while (newlen >= otherlen &&
         MEMCASEEQB(newstr.cp8,otherstr.cp8,otherlen))
         newstr.cp8 += otherlen,newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  return DeeString_New1Byte(newstr.cp8,newlen);
 CASE_WIDTH_2BYTE:
  mystr.cp16    = DeeString_As2Byte(self);
  if unlikely(!mystr.cp16) goto err;
  otherstr.cp16 = DeeString_As2Byte(other);
  if unlikely(!otherstr.cp16) goto err;
  mylen         = WSTR_LENGTH(mystr.cp16);
  otherlen      = WSTR_LENGTH(otherstr.cp16);
  if unlikely(!otherlen) goto retself;
  newstr.cp16 = mystr.cp16,newlen = mylen;
  while (newlen >= otherlen &&
         MEMCASEEQB(newstr.cp16,otherstr.cp16,otherlen))
         newstr.cp16 += otherlen,newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
      return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp16 - mystr.cp16),newlen);
  return DeeString_New2Byte(newstr.cp16,newlen);
 CASE_WIDTH_4BYTE:
  mystr.cp32    = DeeString_As4Byte(self);
  if unlikely(!mystr.cp32) goto err;
  otherstr.cp32 = DeeString_As4Byte(other);
  if unlikely(!otherstr.cp32) goto err;
  mylen         = WSTR_LENGTH(mystr.cp32);
  otherlen      = WSTR_LENGTH(otherstr.cp32);
  if unlikely(!otherlen) goto retself;
  newstr.cp32 = mystr.cp32,newlen = mylen;
  while (newlen >= otherlen &&
         MEMCASEEQB(newstr.cp32,otherstr.cp32,otherlen))
         newstr.cp32 += otherlen,newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
      return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp32 - mystr.cp32),newlen);
  if (DeeString_WIDTH(self) == STRING_WIDTH_2BYTE)
      return DeeString_New2Byte(DeeString_Get2Byte(self) + (newstr.cp32 - mystr.cp32),newlen);
  return DeeString_New4Byte(newstr.cp32,newlen);
 }
retself:
 return_reference_(self);
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_CaseRSStrip(DeeObject *__restrict self,
                      DeeObject *__restrict other) {
 union dcharptr mystr,otherstr;
 size_t mylen,newlen,otherlen;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(other,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  mystr.cp8    = DeeString_As1Byte(self);
  otherstr.cp8 = DeeString_As1Byte(other);
  mylen        = WSTR_LENGTH(mystr.cp8);
  otherlen     = WSTR_LENGTH(otherstr.cp8);
  if unlikely(!otherlen) goto retself;
  newlen       = mylen;
  while (newlen >= otherlen &&
         MEMCASEEQB(mystr.cp8 + (newlen - otherlen),
                    otherstr.cp8,otherlen))
         newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  return DeeString_New1Byte(mystr.cp8,newlen);
 CASE_WIDTH_2BYTE:
  mystr.cp16    = DeeString_As2Byte(self);
  if unlikely(!mystr.cp16) goto err;
  otherstr.cp16 = DeeString_As2Byte(other);
  if unlikely(!otherstr.cp16) goto err;
  mylen         = WSTR_LENGTH(mystr.cp16);
  otherlen      = WSTR_LENGTH(otherstr.cp16);
  if unlikely(!otherlen) goto retself;
  newlen        = mylen;
  while (newlen >= otherlen &&
         MEMCASEEQW(mystr.cp16 + (newlen - otherlen),
                    otherstr.cp16,otherlen))
         newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
      return DeeString_New1Byte(DeeString_Get1Byte(self) + (mystr.cp16 - mystr.cp16),newlen);
  return DeeString_New2Byte(mystr.cp16,newlen);
 CASE_WIDTH_4BYTE:
  mystr.cp32    = DeeString_As4Byte(self);
  if unlikely(!mystr.cp32) goto err;
  otherstr.cp32 = DeeString_As4Byte(other);
  if unlikely(!otherstr.cp32) goto err;
  mylen         = WSTR_LENGTH(mystr.cp32);
  otherlen      = WSTR_LENGTH(otherstr.cp32);
  if unlikely(!otherlen) goto retself;
  newlen        = mylen;
  while (newlen >= otherlen &&
         MEMCASEEQL(mystr.cp32 + (newlen - otherlen),
                    otherstr.cp32,otherlen))
         newlen -= otherlen;
  if (newlen == mylen)
      goto retself;
  if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
      return DeeString_New1Byte(DeeString_Get1Byte(self) + (mystr.cp32 - mystr.cp32),newlen);
  if (DeeString_WIDTH(self) == STRING_WIDTH_2BYTE)
      return DeeString_New2Byte(DeeString_Get2Byte(self) + (mystr.cp32 - mystr.cp32),newlen);
  return DeeString_New4Byte(mystr.cp32,newlen);
 }
retself:
 return_reference_(self);
err:
 return NULL;
}

#define DeeString_IsPrint(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FPRINT)
#define DeeString_IsAlpha(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FALPHA)
#define DeeString_IsSpace(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FSPACE)
#define DeeString_IsLF(self,start,end)         DeeString_TestTrait(self,start,end,UNICODE_FLF)
#define DeeString_IsLower(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FLOWER)
#define DeeString_IsUpper(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FUPPER)
#define DeeString_IsCntrl(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FCNTRL)
#define DeeString_IsDigit(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FDIGIT)
#define DeeString_IsDecimal(self,start,end)    DeeString_TestTrait(self,start,end,UNICODE_FDECIMAL)
#define DeeString_IsSymStrt(self,start,end)    DeeString_TestTrait(self,start,end,UNICODE_FSYMSTRT)
#define DeeString_IsSymCont(self,start,end)    DeeString_TestTrait(self,start,end,UNICODE_FSYMCONT)
#define DeeString_IsAlnum(self,start,end)      DeeString_TestTrait(self,start,end,UNICODE_FALPHA|UNICODE_FDIGIT)
#define DeeString_IsNumeric(self,start,end)    DeeString_TestTrait(self,start,end,UNICODE_FDIGIT|UNICODE_FDECIMAL)
#define DeeString_IsAnyPrint(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FPRINT)
#define DeeString_IsAnyAlpha(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FALPHA)
#define DeeString_IsAnySpace(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FSPACE)
#define DeeString_IsAnyLF(self,start,end)      DeeString_TestAnyTrait(self,start,end,UNICODE_FLF)
#define DeeString_IsAnyLower(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FLOWER)
#define DeeString_IsAnyUpper(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FUPPER)
#define DeeString_IsAnyTitle(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FTITLE)
#define DeeString_IsAnyCntrl(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FCNTRL)
#define DeeString_IsAnyDigit(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FDIGIT)
#define DeeString_IsAnyDecimal(self,start,end) DeeString_TestAnyTrait(self,start,end,UNICODE_FDECIMAL)
#define DeeString_IsAnySymStrt(self,start,end) DeeString_TestAnyTrait(self,start,end,UNICODE_FSYMSTRT)
#define DeeString_IsAnySymCont(self,start,end) DeeString_TestAnyTrait(self,start,end,UNICODE_FSYMCONT)
#define DeeString_IsAnyAlnum(self,start,end)   DeeString_TestAnyTrait(self,start,end,UNICODE_FALPHA|UNICODE_FDIGIT)
#define DeeString_IsAnyNumeric(self,start,end) DeeString_TestAnyTrait(self,start,end,UNICODE_FDIGIT|UNICODE_FDECIMAL)

INTERN bool DCALL
DeeString_TestTrait(DeeObject *__restrict self,
                    size_t start_index,
                    size_t end_index,
                    uniflag_t flags) {
 DeeString_Foreach(self,start_index,end_index,iter,end,{
  if (!(DeeUni_Flags(*iter)&flags))
        return false;
 });
 return true;
}
INTERN bool DCALL
DeeString_TestAnyTrait(DeeObject *__restrict self,
                       size_t start_index,
                       size_t end_index,
                       uniflag_t flags) {
 DeeString_Foreach(self,start_index,end_index,iter,end,{
  if (DeeUni_Flags(*iter)&flags)
      return true;
 });
 return false;
}


INTERN bool DCALL
DeeString_IsTitle(DeeObject *__restrict self,
                  size_t start_index,
                  size_t end_index) {
 bool was_space = false;
 DeeString_Foreach(self,start_index,end_index,iter,end,{
  uniflag_t f = DeeUni_Flags(*iter);
  if (f & UNICODE_FSPACE)
   was_space = true;
  else if (was_space) {
   was_space = false;
   /* Space must be followed by title- or upper-case */
   if (!(f & (UNICODE_FTITLE|UNICODE_FUPPER)))
         return false;
  } else {
   /* Title- or upper-case anywhere else is illegal */
   if (f & (UNICODE_FTITLE|UNICODE_FUPPER))
       return false;
  }
 });
 return true;
}
INTERN bool DCALL
DeeString_IsSymbol(DeeObject *__restrict self,
                   size_t start_index,
                   size_t end_index) {
 uniflag_t flags = (UNICODE_FSYMSTRT|UNICODE_FALPHA);
 DeeString_Foreach(self,start_index,end_index,iter,end,{
  if (!(DeeUni_Flags(*iter)&flags))
        return false;
  flags |= (UNICODE_FSYMCONT|UNICODE_FDIGIT);
 });
 return true;
}



INTERN DREF DeeObject *DCALL
DeeString_Indent(DeeObject *__restrict self,
                 DeeObject *__restrict filler) {
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(filler,&DeeString_Type);
 /* Simple case: if the filler, or self-string are
  *              empty, nothing would get inserted! */
 if unlikely(DeeString_IsEmpty(filler) || DeeString_IsEmpty(self))
    return_reference_(self);
 {
  struct unicode_printer printer = UNICODE_PRINTER_INIT;
  union dcharptr flush_start,iter,end;
  /* Start by inserting the initial, unconditional indentation at the start. */
  if (unicode_printer_printstring(&printer,filler) < 0)
      goto err;
  SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
  CASE_WIDTH_1BYTE:
   iter.cp8 = DeeString_Get1Byte(self);
   end.cp8 = iter.cp8 + WSTR_LENGTH(iter.cp8);
   flush_start.cp8 = iter.cp8;
   while (iter.cp8 < end.cp8) {
    uint8_t ch = *iter.cp8;
    if (DeeUni_IsLF(ch)) {
     ++iter.cp8;
     /* Deal with windows-style linefeeds. */
     if (ch == UNICODE_CR && *iter.cp8 == UNICODE_LF) ++iter.cp8;
     /* Flush all unwritten data up to this point. */
     if (unicode_printer_print8(&printer,flush_start.cp8,
                               (size_t)(iter.cp8-flush_start.cp8)) < 0)
         goto err;
     flush_start.cp8 = iter.cp8;
     /* Insert the filler just before the linefeed. */
     if (unicode_printer_printobject(&printer,filler) < 0)
         goto err;
     continue;
    }
    ++iter.cp8;
   }
   if (iter.cp8 == flush_start.cp8) {
    /* Either the string is empty, ends with a line-feed.
     * In either case, we must remove `filler' from its end,
     * because we're not supposed to have the resulting
     * string include it as trailing memory. */
    ASSERT(UNICODE_PRINTER_LENGTH(&printer) >= DeeString_WLEN(filler));
    unicode_printer_truncate(&printer,
                             UNICODE_PRINTER_LENGTH(&printer) -
                             DeeString_WLEN(filler));
   } else {
    /* Flush the remainder. */
    if (unicode_printer_print8(&printer,flush_start.cp8,
                              (size_t)(iter.cp8-flush_start.cp8)) < 0)
        goto err;
   }
   break;
  CASE_WIDTH_2BYTE:
   iter.cp16 = DeeString_Get2Byte(self);
   end.cp16 = iter.cp16 + WSTR_LENGTH(iter.cp16);
   flush_start.cp16 = iter.cp16;
   while (iter.cp16 < end.cp16) {
    uint16_t ch = *iter.cp16;
    if (DeeUni_IsLF(ch)) {
     ++iter.cp16;
     if (ch == UNICODE_CR && *iter.cp16 == UNICODE_LF) ++iter.cp16;
     if (unicode_printer_print16(&printer,flush_start.cp16,
                               (size_t)(iter.cp16-flush_start.cp16)) < 0)
         goto err;
     flush_start.cp16 = iter.cp16;
     if (unicode_printer_printobject(&printer,filler) < 0)
         goto err;
     continue;
    }
    ++iter.cp16;
   }
   if (iter.cp16 == flush_start.cp16) {
    ASSERT(UNICODE_PRINTER_LENGTH(&printer) >= DeeString_WLEN(filler));
    unicode_printer_truncate(&printer,
                             UNICODE_PRINTER_LENGTH(&printer) -
                             DeeString_WLEN(filler));
   } else {
    if (unicode_printer_print16(&printer,flush_start.cp16,
                              (size_t)(iter.cp16-flush_start.cp16)) < 0)
        goto err;
   }
   break;
  CASE_WIDTH_4BYTE:
   iter.cp32 = DeeString_Get4Byte(self);
   end.cp32 = iter.cp32 + WSTR_LENGTH(iter.cp32);
   flush_start.cp32 = iter.cp32;
   while (iter.cp32 < end.cp32) {
    uint32_t ch = *iter.cp32;
    if (DeeUni_IsLF(ch)) {
     ++iter.cp32;
     if (ch == UNICODE_CR && *iter.cp32 == UNICODE_LF) ++iter.cp32;
     if (unicode_printer_print32(&printer,flush_start.cp32,
                               (size_t)(iter.cp32-flush_start.cp32)) < 0)
         goto err;
     flush_start.cp32 = iter.cp32;
     if (unicode_printer_printobject(&printer,filler) < 0)
         goto err;
     continue;
    }
    ++iter.cp32;
   }
   if (iter.cp32 == flush_start.cp32) {
    ASSERT(UNICODE_PRINTER_LENGTH(&printer) >= DeeString_WLEN(filler));
    unicode_printer_truncate(&printer,
                             UNICODE_PRINTER_LENGTH(&printer) -
                             DeeString_WLEN(filler));
   } else {
    if (unicode_printer_print32(&printer,flush_start.cp32,
                              (size_t)(iter.cp32-flush_start.cp32)) < 0)
        goto err;
   }
   break;
  }
  return unicode_printer_pack(&printer);
err:
  unicode_printer_fini(&printer);
  return NULL;
 }
}

PRIVATE bool DCALL
mask_containsb(DeeObject *__restrict self, uint8_t ch) {
 union dcharptr str;
 SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
 CASE_WIDTH_1BYTE:
  str.cp8 = DeeString_Get1Byte(self);
  return memchrb(str.cp8,ch,WSTR_LENGTH(str.cp8)) != NULL;
 CASE_WIDTH_2BYTE:
  str.cp16 = DeeString_Get2Byte(self);
  return memchrw(str.cp16,ch,WSTR_LENGTH(str.cp16)) != NULL;
 CASE_WIDTH_4BYTE:
  str.cp32 = DeeString_Get4Byte(self);
  return memchrl(str.cp32,ch,WSTR_LENGTH(str.cp32)) != NULL;
 }
}
PRIVATE bool DCALL
mask_containsw(DeeObject *__restrict self, uint16_t ch) {
 union dcharptr str;
 SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
 CASE_WIDTH_1BYTE:
  if (ch > 0xff) return false;
  str.cp8 = DeeString_Get1Byte(self);
  return memchrb(str.cp8,(uint8_t)ch,WSTR_LENGTH(str.cp8)) != NULL;
 CASE_WIDTH_2BYTE:
  str.cp16 = DeeString_Get2Byte(self);
  return memchrw(str.cp16,ch,WSTR_LENGTH(str.cp16)) != NULL;
 CASE_WIDTH_4BYTE:
  str.cp32 = DeeString_Get4Byte(self);
  return memchrl(str.cp32,ch,WSTR_LENGTH(str.cp32)) != NULL;
 }
}
PRIVATE bool DCALL
mask_containsl(DeeObject *__restrict self, uint32_t ch) {
 union dcharptr str;
 SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
 CASE_WIDTH_1BYTE:
  if (ch > 0xff) return false;
  str.cp8 = DeeString_Get1Byte(self);
  return memchrb(str.cp8,(uint8_t)ch,WSTR_LENGTH(str.cp8)) != NULL;
 CASE_WIDTH_2BYTE:
  if (ch > 0xffff) return false;
  str.cp16 = DeeString_Get2Byte(self);
  return memchrw(str.cp16,(uint16_t)ch,WSTR_LENGTH(str.cp16)) != NULL;
 CASE_WIDTH_4BYTE:
  str.cp32 = DeeString_Get4Byte(self);
  return memchrl(str.cp32,ch,WSTR_LENGTH(str.cp32)) != NULL;
 }
}


INTERN DREF DeeObject *DCALL
DeeString_Dedent(DeeObject *__restrict self,
                 size_t max_chars,
                 DeeObject *__restrict mask) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 union dcharptr flush_start,iter,end; size_t i;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT(mask,&DeeString_Type);
 /* Simple case: Nothing should be removed. */
 if unlikely(!max_chars)
    return_reference_(self);
 SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
 CASE_WIDTH_1BYTE:
  iter.cp8 = DeeString_Get1Byte(self);
  end.cp8 = iter.cp8 + WSTR_LENGTH(iter.cp8);
  /* Remove leading characters. */
  for (i = 0; i < max_chars && mask_containsb(mask,*iter.cp8); ++i) ++iter.cp8;
  flush_start.cp8 = iter.cp8;
  while (iter.cp8 < end.cp8) {
   uint8_t ch = *iter.cp8;
   if (DeeUni_IsLF(ch)) {
    ++iter.cp8;
    if (ch == UNICODE_CR && *iter.cp8 == UNICODE_LF) ++iter.cp8;
    /* Flush all unwritten data up to this point. */
    if (unicode_printer_print8(&printer,flush_start.cp8,
                              (size_t)(iter.cp8-flush_start.cp8)) < 0)
        goto err;
    /* Skip up to `max_chars' characters after a linefeed. */
    for (i = 0; i < max_chars && mask_containsb(mask,*iter.cp8); ++i) ++iter.cp8;
    flush_start = iter;
    continue;
   }
   ++iter.cp8;
  }
  /* Flush the remainder. */
  if (unicode_printer_print8(&printer,flush_start.cp8,
                            (size_t)(iter.cp8-flush_start.cp8)) < 0)
      goto err;
  break;
 CASE_WIDTH_2BYTE:
  iter.cp16 = DeeString_Get2Byte(self);
  end.cp16 = iter.cp16 + WSTR_LENGTH(iter.cp16);
  for (i = 0; i < max_chars && mask_containsw(mask,*iter.cp16); ++i) ++iter.cp16;
  flush_start.cp16 = iter.cp16;
  while (iter.cp16 < end.cp16) {
   uint16_t ch = *iter.cp16;
   if (DeeUni_IsLF(ch)) {
    ++iter.cp16;
    if (ch == UNICODE_CR && *iter.cp16 == UNICODE_LF) ++iter.cp16;
    if (unicode_printer_print16(&printer,flush_start.cp16,
                               (size_t)(iter.cp16-flush_start.cp16)) < 0)
        goto err;
    for (i = 0; i < max_chars && mask_containsw(mask,*iter.cp16); ++i) ++iter.cp16;
    flush_start = iter;
    continue;
   }
   ++iter.cp16;
  }
  if (unicode_printer_print16(&printer,flush_start.cp16,
                             (size_t)(iter.cp16-flush_start.cp16)) < 0)
      goto err;
  break;
 CASE_WIDTH_4BYTE:
  iter.cp32 = DeeString_Get4Byte(self);
  end.cp32 = iter.cp32 + WSTR_LENGTH(iter.cp32);
  for (i = 0; i < max_chars && mask_containsl(mask,*iter.cp32); ++i) ++iter.cp32;
  flush_start.cp32 = iter.cp32;
  while (iter.cp32 < end.cp32) {
   uint32_t ch = *iter.cp32;
   if (DeeUni_IsLF(ch)) {
    ++iter.cp32;
    if (ch == UNICODE_CR && *iter.cp32 == UNICODE_LF) ++iter.cp32;
    if (unicode_printer_print32(&printer,flush_start.cp32,
                               (size_t)(iter.cp32-flush_start.cp32)) < 0)
        goto err;
    for (i = 0; i < max_chars && mask_containsl(mask,*iter.cp32); ++i) ++iter.cp32;
    flush_start = iter;
    continue;
   }
   ++iter.cp32;
  }
  if (unicode_printer_print32(&printer,flush_start.cp32,
                             (size_t)(iter.cp32-flush_start.cp32)) < 0)
      goto err;
  break;
 }
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeString_DedentSpc(DeeObject *__restrict self,
                    size_t max_chars) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 union dcharptr flush_start,iter,end; size_t i;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 /* Simple case: Nothing should be removed. */
 if unlikely(!max_chars)
    return_reference_(self);
 SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
 CASE_WIDTH_1BYTE:
  iter.cp8 = DeeString_Get1Byte(self);
  end.cp8 = iter.cp8 + WSTR_LENGTH(iter.cp8);
  /* Remove leading characters. */
  for (i = 0; i < max_chars && DeeUni_IsSpace(*iter.cp8); ++i) ++iter.cp8;
  flush_start.cp8 = iter.cp8;
  while (iter.cp8 < end.cp8) {
   uint8_t ch = *iter.cp8;
   if (DeeUni_IsLF(ch)) {
    ++iter.cp8;
    if (ch == UNICODE_CR && *iter.cp8 == UNICODE_LF) ++iter.cp8;
    /* Flush all unwritten data up to this point. */
    if (unicode_printer_print8(&printer,flush_start.cp8,
                              (size_t)(iter.cp8-flush_start.cp8)) < 0)
        goto err;
    /* Skip up to `max_chars' characters after a linefeed. */
    for (i = 0; i < max_chars && DeeUni_IsSpace(*iter.cp8); ++i) ++iter.cp8;
    flush_start = iter;
    continue;
   }
   ++iter.cp8;
  }
  /* Flush the remainder. */
  if (unicode_printer_print8(&printer,flush_start.cp8,
                            (size_t)(iter.cp8-flush_start.cp8)) < 0)
      goto err;
  break;
 CASE_WIDTH_2BYTE:
  iter.cp16 = DeeString_Get2Byte(self);
  end.cp16 = iter.cp16 + WSTR_LENGTH(iter.cp16);
  for (i = 0; i < max_chars && DeeUni_IsSpace(*iter.cp16); ++i) ++iter.cp16;
  flush_start.cp16 = iter.cp16;
  while (iter.cp16 < end.cp16) {
   uint16_t ch = *iter.cp16;
   if (DeeUni_IsLF(ch)) {
    ++iter.cp16;
    if (ch == UNICODE_CR && *iter.cp16 == UNICODE_LF) ++iter.cp16;
    if (unicode_printer_print16(&printer,flush_start.cp16,
                               (size_t)(iter.cp16-flush_start.cp16)) < 0)
        goto err;
    for (i = 0; i < max_chars && DeeUni_IsSpace(*iter.cp16); ++i) ++iter.cp16;
    flush_start = iter;
    continue;
   }
   ++iter.cp16;
  }
  if (unicode_printer_print16(&printer,flush_start.cp16,
                             (size_t)(iter.cp16-flush_start.cp16)) < 0)
      goto err;
  break;
 CASE_WIDTH_4BYTE:
  iter.cp32 = DeeString_Get4Byte(self);
  end.cp32 = iter.cp32 + WSTR_LENGTH(iter.cp32);
  for (i = 0; i < max_chars && DeeUni_IsSpace(*iter.cp32); ++i) ++iter.cp32;
  flush_start.cp32 = iter.cp32;
  while (iter.cp32 < end.cp32) {
   uint32_t ch = *iter.cp32;
   if (DeeUni_IsLF(ch)) {
    ++iter.cp32;
    if (ch == UNICODE_CR && *iter.cp32 == UNICODE_LF) ++iter.cp32;
    if (unicode_printer_print32(&printer,flush_start.cp32,
                               (size_t)(iter.cp32-flush_start.cp32)) < 0)
        goto err;
    for (i = 0; i < max_chars && DeeUni_IsSpace(*iter.cp32); ++i) ++iter.cp32;
    flush_start = iter;
    continue;
   }
   ++iter.cp32;
  }
  if (unicode_printer_print32(&printer,flush_start.cp32,
                             (size_t)(iter.cp32-flush_start.cp32)) < 0)
      goto err;
  break;
 }
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}

PUBLIC DREF DeeObject *DCALL
DeeString_New2Byte(uint16_t const *__restrict str,
                   size_t length) {
 uint16_t *buffer;
 buffer = DeeString_NewBuffer16(length);
 if unlikely(!buffer) return NULL;
 memcpyw(buffer,str,length);
 return DeeString_Pack2ByteBuffer(buffer);
}
PUBLIC DREF DeeObject *DCALL
DeeString_New4Byte(uint32_t const *__restrict str,
                   size_t length) {
 uint32_t *buffer;
 buffer = DeeString_NewBuffer32(length);
 if unlikely(!buffer) return NULL;
 memcpyl(buffer,str,length);
 return DeeString_Pack4ByteBuffer(buffer);
}
PUBLIC DREF DeeObject *DCALL
DeeString_NewUtf16(uint16_t const *__restrict str,
                   size_t length,
                   unsigned int error_mode) {
 uint16_t *buffer;
 buffer = DeeString_NewBuffer16(length);
 if unlikely(!buffer) return NULL;
 memcpyw(buffer,str,length);
 return DeeString_PackUtf16Buffer(buffer,error_mode);
}
PUBLIC DREF DeeObject *DCALL
DeeString_NewUtf32(uint32_t const *__restrict str,
                   size_t length,
                   unsigned int error_mode) {
 uint32_t *buffer;
 buffer = DeeString_NewBuffer32(length);
 if unlikely(!buffer) return NULL;
 memcpyl(buffer,str,length);
 return DeeString_PackUtf32Buffer(buffer,error_mode);
}
PUBLIC DREF DeeObject *DCALL
DeeString_NewUtf16AltEndian(uint16_t const *__restrict str,
                            size_t length,
                            unsigned int error_mode) {
 uint16_t *buffer; size_t i;
 buffer = DeeString_NewBuffer16(length);
 if unlikely(!buffer) return NULL;
 for (i = 0; i < length; ++i)
     buffer[i] = DEE_BSWAP16(str[i]);
 return DeeString_PackUtf16Buffer(buffer,error_mode);
}
PUBLIC DREF DeeObject *DCALL
DeeString_NewUtf32AltEndian(uint32_t const *__restrict str,
                            size_t length,
                            unsigned int error_mode) {
 uint32_t *buffer; size_t i;
 buffer = DeeString_NewBuffer32(length);
 if unlikely(!buffer) return NULL;
 for (i = 0; i < length; ++i)
     buffer[i] = DEE_BSWAP32(str[i]);
 return DeeString_PackUtf32Buffer(buffer,error_mode);
}




PRIVATE DREF String *DCALL
string_replace(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 String *find,*replace; size_t max_count = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"oo|Iu:replace",&find,&replace,&max_count) ||
     DeeObject_AssertTypeExact((DeeObject *)find,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)replace,&DeeString_Type))
     goto err;
 if unlikely(!max_count)
    goto retself;
 {
  struct unicode_printer p = UNICODE_PRINTER_INIT;
  union dcharptr ptr,mystr,begin,end,findstr; size_t findlen;
  SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                          DeeString_WIDTH(find))) {
  CASE_WIDTH_1BYTE:
   mystr.cp8   = DeeString_As1Byte((DeeObject *)self);
   findstr.cp8 = DeeString_As1Byte((DeeObject *)find);
   findlen     = WSTR_LENGTH(findstr.cp8);
   /* Handle special cases. */
   if unlikely(findlen > WSTR_LENGTH(mystr.cp8))
      goto retself;
   if unlikely(!findlen)
      goto retrepl_if_self;
   begin.cp8 = mystr.cp8;
   end.cp8   = begin.cp8+WSTR_LENGTH(mystr.cp8);
   while ((ptr.cp8 = memmemb(begin.cp8,end.cp8 - begin.cp8,
                             findstr.cp8,findlen)) != NULL) {
    /* Found one */
    if (unlikely(unicode_printer_print8(&p,begin.cp8,(size_t)(ptr.cp8-begin.cp8)) < 0) ||
        unlikely(unicode_printer_printstring(&p,(DeeObject *)replace) < 0))
        goto err_printer;
    if unlikely(!--max_count) break;
    begin.cp8 = ptr.cp8 + findlen;
   }
   /* If we never found `find', our printer will still be empty.
    * >> In that case we don't need to write the entire string to it,
    *    but can simply return a reference to the original string,
    *    saving on memory and speeding up the function by a lot. */
   if (UNICODE_PRINTER_LENGTH(&p) == 0 && begin.cp8 == mystr.cp8)
       goto retself;
   if unlikely(unicode_printer_print8(&p,begin.cp8,(size_t)(end.cp8-begin.cp8)) < 0)
      goto err_printer;
   break;
  CASE_WIDTH_2BYTE:
   mystr.cp16   = DeeString_As2Byte((DeeObject *)self);
   if unlikely(!mystr.cp16) goto err_printer;
   findstr.cp16 = DeeString_As2Byte((DeeObject *)find);
   if unlikely(!findstr.cp16) goto err_printer;
   findlen      = WSTR_LENGTH(findstr.cp16);
   if unlikely(findlen > WSTR_LENGTH(mystr.cp16))
      goto retself;
   if unlikely(!findlen)
      goto retrepl_if_self;
   begin.cp16 = mystr.cp16;
   end.cp16   = begin.cp16+WSTR_LENGTH(mystr.cp16);
   while ((ptr.cp16 = memmemw(begin.cp16,end.cp16 - begin.cp16,
                              findstr.cp16,findlen)) != NULL) {
    if (unlikely(unicode_printer_print16(&p,begin.cp16,(size_t)(ptr.cp16-begin.cp16)) < 0) ||
        unlikely(unicode_printer_printstring(&p,(DeeObject *)replace) < 0))
        goto err_printer;
    if unlikely(!--max_count) break;
    begin.cp16 = ptr.cp16 + findlen;
   }
   if (UNICODE_PRINTER_LENGTH(&p) == 0 && begin.cp16 == mystr.cp16)
       goto retself;
   if unlikely(unicode_printer_print16(&p,begin.cp16,(size_t)(end.cp16-begin.cp16)) < 0)
      goto err_printer;
   break;
  CASE_WIDTH_4BYTE:
   mystr.cp32   = DeeString_As4Byte((DeeObject *)self);
   if unlikely(!mystr.cp32) goto err_printer;
   findstr.cp32 = DeeString_As4Byte((DeeObject *)find);
   if unlikely(!findstr.cp32) goto err_printer;
   findlen      = WSTR_LENGTH(findstr.cp32);
   if unlikely(findlen > WSTR_LENGTH(mystr.cp32))
      goto retself;
   if unlikely(!findlen)
      goto retrepl_if_self;
   begin.cp32 = mystr.cp32;
   end.cp32   = begin.cp32+WSTR_LENGTH(mystr.cp32);
   while ((ptr.cp32 = memmeml(begin.cp32,end.cp32 - begin.cp32,
                              findstr.cp32,findlen)) != NULL) {
    if (unlikely(unicode_printer_print32(&p,begin.cp32,(size_t)(ptr.cp32-begin.cp32)) < 0) ||
        unlikely(unicode_printer_printstring(&p,(DeeObject *)replace) < 0))
        goto err_printer;
    if unlikely(!--max_count) break;
    begin.cp32 = ptr.cp32 + findlen;
   }
   if (UNICODE_PRINTER_LENGTH(&p) == 0 && begin.cp32 == mystr.cp32)
       goto retself;
   if unlikely(unicode_printer_print32(&p,begin.cp32,(size_t)(end.cp32-begin.cp32)) < 0)
      goto err_printer;
   break;
  }
  return (DREF String *)unicode_printer_pack(&p);
err_printer:
  unicode_printer_fini(&p);
 }
err:
 return NULL;
retrepl_if_self:
 if (!DeeString_IsEmpty(self))
      goto retself;
 return_reference_(replace);
retself:
 return_reference_(self);
}

PRIVATE DREF String *DCALL
string_casereplace(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 String *find,*replace; size_t max_count = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"oo|Iu:casereplace",&find,&replace,&max_count) ||
     DeeObject_AssertTypeExact((DeeObject *)find,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)replace,&DeeString_Type))
     goto err;
 if unlikely(!max_count)
    goto retself;
 {
  struct unicode_printer p = UNICODE_PRINTER_INIT;
  union dcharptr ptr,mystr,begin,end,findstr; size_t findlen;
  SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                          DeeString_WIDTH(find))) {
  CASE_WIDTH_1BYTE:
   mystr.cp8   = DeeString_As1Byte((DeeObject *)self);
   findstr.cp8 = DeeString_As1Byte((DeeObject *)find);
   findlen     = WSTR_LENGTH(findstr.cp8);
   /* Handle special cases. */
   if unlikely(findlen > WSTR_LENGTH(mystr.cp8))
      goto retself;
   if unlikely(!findlen)
      goto retrepl_if_self;
   begin.cp8 = mystr.cp8;
   end.cp8   = begin.cp8+WSTR_LENGTH(mystr.cp8);
   while ((ptr.cp8 = memcasememb(begin.cp8,end.cp8 - begin.cp8,
                                 findstr.cp8,findlen)) != NULL) {
    /* Found one */
    if (unlikely(unicode_printer_print8(&p,begin.cp8,(size_t)(ptr.cp8-begin.cp8)) < 0) ||
        unlikely(unicode_printer_printstring(&p,(DeeObject *)replace) < 0))
        goto err_printer;
    if unlikely(!--max_count) break;
    begin.cp8 = ptr.cp8 + findlen;
   }
   /* If we never found `find', our printer will still be empty.
    * >> In that case we don't need to write the entire string to it,
    *    but can simply return a reference to the original string,
    *    saving on memory and speeding up the function by a lot. */
   if (UNICODE_PRINTER_LENGTH(&p) == 0 && begin.cp8 == mystr.cp8)
       goto retself;
   if unlikely(unicode_printer_print8(&p,begin.cp8,(size_t)(end.cp8-begin.cp8)) < 0)
      goto err_printer;
   break;
  CASE_WIDTH_2BYTE:
   mystr.cp16   = DeeString_As2Byte((DeeObject *)self);
   if unlikely(!mystr.cp16) goto err_printer;
   findstr.cp16 = DeeString_As2Byte((DeeObject *)find);
   if unlikely(!findstr.cp16) goto err_printer;
   findlen      = WSTR_LENGTH(findstr.cp16);
   if unlikely(findlen > WSTR_LENGTH(mystr.cp16))
      goto retself;
   if unlikely(!findlen)
      goto retrepl_if_self;
   begin.cp16 = mystr.cp16;
   end.cp16   = begin.cp16+WSTR_LENGTH(mystr.cp16);
   while ((ptr.cp16 = memcasememw(begin.cp16,end.cp16 - begin.cp16,
                                  findstr.cp16,findlen)) != NULL) {
    if (unlikely(unicode_printer_print16(&p,begin.cp16,(size_t)(ptr.cp16-begin.cp16)) < 0) ||
        unlikely(unicode_printer_printstring(&p,(DeeObject *)replace) < 0))
        goto err_printer;
    if unlikely(!--max_count) break;
    begin.cp16 = ptr.cp16 + findlen;
   }
   if (UNICODE_PRINTER_LENGTH(&p) == 0 && begin.cp16 == mystr.cp16)
       goto retself;
   if unlikely(unicode_printer_print16(&p,begin.cp16,(size_t)(end.cp16-begin.cp16)) < 0)
      goto err_printer;
   break;
  CASE_WIDTH_4BYTE:
   mystr.cp32   = DeeString_As4Byte((DeeObject *)self);
   if unlikely(!mystr.cp32) goto err_printer;
   findstr.cp32 = DeeString_As4Byte((DeeObject *)find);
   if unlikely(!findstr.cp32) goto err_printer;
   findlen      = WSTR_LENGTH(findstr.cp32);
   if unlikely(findlen > WSTR_LENGTH(mystr.cp32))
      goto retself;
   if unlikely(!findlen)
      goto retrepl_if_self;
   begin.cp32 = mystr.cp32;
   end.cp32   = begin.cp32+WSTR_LENGTH(mystr.cp32);
   while ((ptr.cp32 = memcasememl(begin.cp32,end.cp32 - begin.cp32,
                                  findstr.cp32,findlen)) != NULL) {
    if (unlikely(unicode_printer_print32(&p,begin.cp32,(size_t)(ptr.cp32-begin.cp32)) < 0) ||
        unlikely(unicode_printer_printstring(&p,(DeeObject *)replace) < 0))
        goto err_printer;
    if unlikely(!--max_count) break;
    begin.cp32 = ptr.cp32 + findlen;
   }
   if (UNICODE_PRINTER_LENGTH(&p) == 0 && begin.cp32 == mystr.cp32)
       goto retself;
   if unlikely(unicode_printer_print32(&p,begin.cp32,(size_t)(end.cp32-begin.cp32)) < 0)
      goto err_printer;
   break;
  }
  return (DREF String *)unicode_printer_pack(&p);
err_printer:
  unicode_printer_fini(&p);
 }
err:
 return NULL;
retrepl_if_self:
 if (!DeeString_IsEmpty(self))
      goto retself;
 return_reference_(replace);
retself:
 return_reference_(self);
}

PRIVATE DREF DeeObject *DCALL
string_ord(String *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 size_t index = 0;
 if (argc) {
  if (DeeArg_Unpack(argc,argv,"Iu:ord",&index))
      goto err;
  if (index >= DeeString_WLEN(self)) {
   err_index_out_of_bounds((DeeObject *)self,
                            index,
                            DeeString_WLEN(self));
   goto err;
  }
 } else if unlikely(DeeString_WLEN(self) != 1) {
  err_expected_single_character_string((DeeObject *)self);
  goto err;
 }
 return DeeInt_NewU32(DeeString_GetChar((DeeObject *)self,index));
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_bytes(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 bool allow_invalid = false;
 size_t start = 0,end = (size_t)-1;
 uint8_t *my_bytes;
 if (argc == 1) {
  int temp = DeeObject_Bool(argv[0]);
  if unlikely(temp < 0) goto err;
  allow_invalid = !!temp;
 } else {
  if (DeeArg_Unpack(argc,argv,"|IdIdb:bytes",&start,&end,&allow_invalid))
      goto err;
 }
 my_bytes = DeeString_AsBytes((DeeObject *)self,allow_invalid);
 if unlikely(!my_bytes) goto err;
 if (start > WSTR_LENGTH(my_bytes)) start = WSTR_LENGTH(my_bytes);
 if (end > WSTR_LENGTH(my_bytes)) end = WSTR_LENGTH(my_bytes);
 return DeeBytes_NewView((DeeObject *)self,
                          my_bytes + start,
                          end - start,
                          DEE_BUFFER_FREADONLY);
err:
 return NULL;
}

#define DEFINE_STRING_TRAIT(name,function,flag) \
PRIVATE DREF DeeObject *DCALL \
string_##name(String *__restrict self, \
              size_t argc, DeeObject **__restrict argv) { \
 size_t start = 0,end = (size_t)-1; \
 if (argc == 1) { \
  uint32_t ch; \
  if (DeeObject_AsSize(argv[0],&start)) \
      goto err; \
  if unlikely(start >= DeeString_WLEN(self)) { \
   err_index_out_of_bounds((DeeObject *)self, \
                            start, \
                            DeeString_WLEN(self)); \
   goto err; \
  } \
  ch = DeeString_GetChar(self,start); \
  return_bool(DeeUni_Flags(ch) & (flag)); \
 } else { \
  if (DeeArg_Unpack(argc,argv,"|IdId" #name,&start,&end)) \
      goto err; \
  return_bool(function((DeeObject *)self,start,end)); \
 } \
err: \
 return NULL; \
}
#define DEFINE_ANY_STRING_TRAIT(name,function) \
PRIVATE DREF DeeObject *DCALL \
string_##name(String *__restrict self, \
              size_t argc, DeeObject **__restrict argv) { \
 size_t start = 0,end = (size_t)-1; \
 if (DeeArg_Unpack(argc,argv,"|IdId" #name,&start,&end)) \
     return NULL; \
 return_bool(function((DeeObject *)self,start,end)); \
}
DEFINE_STRING_TRAIT(isprint,DeeString_IsPrint,UNICODE_FPRINT)
DEFINE_STRING_TRAIT(isalpha,DeeString_IsAlpha,UNICODE_FALPHA)
DEFINE_STRING_TRAIT(isspace,DeeString_IsSpace,UNICODE_FSPACE)
DEFINE_STRING_TRAIT(islf,DeeString_IsLF,UNICODE_FLF)
DEFINE_STRING_TRAIT(islower,DeeString_IsLower,UNICODE_FLOWER)
DEFINE_STRING_TRAIT(isupper,DeeString_IsUpper,UNICODE_FUPPER)
DEFINE_STRING_TRAIT(iscntrl,DeeString_IsCntrl,UNICODE_FCNTRL)
DEFINE_STRING_TRAIT(isdigit,DeeString_IsDigit,UNICODE_FDIGIT)
DEFINE_STRING_TRAIT(isdecimal,DeeString_IsDecimal,UNICODE_FDECIMAL)
DEFINE_STRING_TRAIT(issymstrt,DeeString_IsSymStrt,UNICODE_FSYMSTRT)
DEFINE_STRING_TRAIT(issymcont,DeeString_IsSymCont,UNICODE_FSYMCONT)
DEFINE_STRING_TRAIT(isalnum,DeeString_IsAlnum,UNICODE_FALPHA|UNICODE_FDIGIT)
DEFINE_STRING_TRAIT(isnumeric,DeeString_IsNumeric,UNICODE_FDECIMAL|UNICODE_FDIGIT)
DEFINE_STRING_TRAIT(istitle,DeeString_IsTitle,UNICODE_FTITLE)
DEFINE_STRING_TRAIT(issymbol,DeeString_IsSymbol,UNICODE_FSYMSTRT)
DEFINE_ANY_STRING_TRAIT(isanyprint,DeeString_IsAnyPrint)
DEFINE_ANY_STRING_TRAIT(isanyalpha,DeeString_IsAnyAlpha)
DEFINE_ANY_STRING_TRAIT(isanyspace,DeeString_IsAnySpace)
DEFINE_ANY_STRING_TRAIT(isanylf,DeeString_IsAnyLF)
DEFINE_ANY_STRING_TRAIT(isanylower,DeeString_IsAnyLower)
DEFINE_ANY_STRING_TRAIT(isanyupper,DeeString_IsAnyUpper)
DEFINE_ANY_STRING_TRAIT(isanycntrl,DeeString_IsAnyCntrl)
DEFINE_ANY_STRING_TRAIT(isanydigit,DeeString_IsAnyDigit)
DEFINE_ANY_STRING_TRAIT(isanydecimal,DeeString_IsAnyDecimal)
DEFINE_ANY_STRING_TRAIT(isanysymstrt,DeeString_IsAnySymStrt)
DEFINE_ANY_STRING_TRAIT(isanysymcont,DeeString_IsAnySymCont)
DEFINE_ANY_STRING_TRAIT(isanyalnum,DeeString_IsAnyAlnum)
DEFINE_ANY_STRING_TRAIT(isanynumeric,DeeString_IsAnyNumeric)
DEFINE_ANY_STRING_TRAIT(isanytitle,DeeString_IsAnyTitle)
#undef DEFINE_ANY_STRING_TRAIT
#undef DEFINE_STRING_TRAIT

INTDEF DREF DeeObject *DCALL DeeString_Convert(DeeObject *__restrict self, size_t start, size_t end, uintptr_t kind);
INTDEF DREF DeeObject *DCALL DeeString_ToTitle(DeeObject *__restrict self, size_t start, size_t end);
INTDEF DREF DeeObject *DCALL DeeString_Capitalize(DeeObject *__restrict self, size_t start, size_t end);
INTDEF DREF DeeObject *DCALL DeeString_Swapcase(DeeObject *__restrict self, size_t start, size_t end);
#define DeeString_ToLower(self,start,end) DeeString_Convert(self,start,end,UNICODE_CONVERT_LOWER)
#define DeeString_ToUpper(self,start,end) DeeString_Convert(self,start,end,UNICODE_CONVERT_UPPER)

PRIVATE DREF DeeObject *DCALL
string_lower(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:lower",&start,&end))
     return NULL;
 return DeeString_ToLower((DeeObject *)self,start,end);
}
PRIVATE DREF DeeObject *DCALL
string_upper(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:upper",&start,&end))
     return NULL;
 return DeeString_ToUpper((DeeObject *)self,start,end);
}
PRIVATE DREF DeeObject *DCALL
string_title(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:title",&start,&end))
     return NULL;
 return DeeString_ToTitle((DeeObject *)self,start,end);
}
PRIVATE DREF DeeObject *DCALL
string_capitalize(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:capitalize",&start,&end))
     return NULL;
 return DeeString_Capitalize((DeeObject *)self,start,end);
}
PRIVATE DREF DeeObject *DCALL
string_swapcase(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:swapcase",&start,&end))
     return NULL;
 return DeeString_Swapcase((DeeObject *)self,start,end);
}
PRIVATE DREF DeeObject *DCALL
string_find(String *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 dssize_t result; union dcharptr ptr,lhs,rhs; size_t mylen;
 if (DeeArg_Unpack(argc,argv,"o|IdId:find",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 result = -1;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  ptr.cp8 = memmemb(lhs.cp8 + begin,end - begin,
                    rhs.cp8,WSTR_LENGTH(rhs.cp8));
  if (ptr.cp8) result = ptr.cp8 - lhs.cp8;
  break;
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  ptr.cp16 = memmemw(lhs.cp16 + begin,end - begin,
                     rhs.cp16,WSTR_LENGTH(rhs.cp16));
  if (ptr.cp16) result = ptr.cp16 - lhs.cp16;
  break;
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  ptr.cp32 = memmeml(lhs.cp32 + begin,end - begin,
                     rhs.cp32,WSTR_LENGTH(rhs.cp32));
  if (ptr.cp32) result = ptr.cp32 - lhs.cp32;
  break;
 }
 return DeeInt_NewSSize(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_rfind(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 dssize_t result; union dcharptr ptr,lhs,rhs; size_t mylen;
 if (DeeArg_Unpack(argc,argv,"o|IdId:rfind",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 result = -1;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  ptr.cp8 = memrmemb(lhs.cp8 + begin,end - begin,
                     rhs.cp8,WSTR_LENGTH(rhs.cp8));
  if (ptr.cp8) result = ptr.cp8 - lhs.cp8;
  break;
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  ptr.cp16 = memrmemw(lhs.cp16 + begin,end - begin,
                      rhs.cp16,WSTR_LENGTH(rhs.cp16));
  if (ptr.cp16) result = ptr.cp16 - lhs.cp16;
  break;
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  ptr.cp32 = memrmeml(lhs.cp32 + begin,end - begin,
                      rhs.cp32,WSTR_LENGTH(rhs.cp32));
  if (ptr.cp32) result = ptr.cp32 - lhs.cp32;
  break;
 }
 return DeeInt_NewSSize(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_index(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 size_t result; union dcharptr ptr,lhs,rhs; size_t mylen;
 if (DeeArg_Unpack(argc,argv,"o|IdId:index",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_found;
  ptr.cp8 = memmemb(lhs.cp8 + begin,end - begin,
                    rhs.cp8,WSTR_LENGTH(rhs.cp8));
  if (!ptr.cp8) goto not_found;
  result = (size_t)(ptr.cp8 - lhs.cp8);
  break;
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_found;
  ptr.cp16 = memmemw(lhs.cp16 + begin,end - begin,
                     rhs.cp16,WSTR_LENGTH(rhs.cp16));
  if (!ptr.cp16) goto not_found;
  result = (size_t)(ptr.cp16 - lhs.cp16);
  break;
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_found;
  ptr.cp32 = memmeml(lhs.cp32 + begin,end - begin,
                     rhs.cp32,WSTR_LENGTH(rhs.cp32));
  if (!ptr.cp32) goto not_found;
  result = (size_t)(ptr.cp32 - lhs.cp32);
  break;
 }
 return DeeInt_NewSize(result);
not_found:
 err_index_not_found((DeeObject *)self,
                     (DeeObject *)other);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_rindex(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 size_t result; union dcharptr ptr,lhs,rhs; size_t mylen;
 if (DeeArg_Unpack(argc,argv,"o|IdId:rindex",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_found;
  ptr.cp8 = memrmemb(lhs.cp8 + begin,end - begin,
                     rhs.cp8,WSTR_LENGTH(rhs.cp8));
  if (!ptr.cp8) goto not_found;
  result = (size_t)(ptr.cp8 - lhs.cp8);
  break;
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_found;
  ptr.cp16 = memrmemw(lhs.cp16 + begin,end - begin,
                      rhs.cp16,WSTR_LENGTH(rhs.cp16));
  if (!ptr.cp16) goto not_found;
  result = (size_t)(ptr.cp16 - lhs.cp16);
  break;
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_found;
  ptr.cp32 = memrmeml(lhs.cp32 + begin,end - begin,
                      rhs.cp32,WSTR_LENGTH(rhs.cp32));
  if (!ptr.cp32) goto not_found;
  result = (size_t)(ptr.cp32 - lhs.cp32);
  break;
 }
 return DeeInt_NewSize(result);
not_found:
 err_index_not_found((DeeObject *)self,
                     (DeeObject *)other);
err:
 return NULL;
}
INTDEF DREF DeeObject *DCALL
DeeString_FindAll(String *__restrict self,
                  String *__restrict other,
                  size_t start, size_t end);
INTDEF DREF DeeObject *DCALL
DeeString_CaseFindAll(String *__restrict self,
                      String *__restrict other,
                      size_t start, size_t end);
PRIVATE DREF DeeObject *DCALL
string_findall(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 String *other; size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:findall",&other,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 return DeeString_FindAll(self,other,start,end);
}
PRIVATE DREF DeeObject *DCALL
string_casefindall(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 String *other; size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:casefindall",&other,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     return NULL;
 return DeeString_CaseFindAll(self,other,start,end);
}
PRIVATE DREF DeeObject *DCALL
string_casefind(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 String *other; size_t start = 0,end = (size_t)-1;
 dssize_t result; union dcharptr ptr,lhs,rhs; size_t mylen;
 if (DeeArg_Unpack(argc,argv,"o|IdId:casefind",&other,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 result = -1;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(start > mylen) start = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= start) break;
  ptr.cp8 = memcasememb(lhs.cp8 + start,end - start,
                        rhs.cp8,WSTR_LENGTH(rhs.cp8));
  if (ptr.cp8) result = ptr.cp8 - lhs.cp8;
  break;
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(start > mylen) start = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= start) break;
  ptr.cp16 = memcasememw(lhs.cp16 + start,end - start,
                         rhs.cp16,WSTR_LENGTH(rhs.cp16));
  if (ptr.cp16) result = ptr.cp16 - lhs.cp16;
  break;
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(start > mylen) start = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= start) break;
  ptr.cp32 = memcasememl(lhs.cp32 + start,end - start,
                         rhs.cp32,WSTR_LENGTH(rhs.cp32));
  if (ptr.cp32) result = ptr.cp32 - lhs.cp32;
  break;
 }
 return DeeInt_NewSSize(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_caserfind(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 dssize_t result; union dcharptr ptr,lhs,rhs; size_t mylen;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caserfind",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 result = -1;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  ptr.cp8 = memcasermemb(lhs.cp8 + begin,end - begin,
                         rhs.cp8,WSTR_LENGTH(rhs.cp8));
  if (ptr.cp8) result = ptr.cp8 - lhs.cp8;
  break;
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  ptr.cp16 = memcasermemw(lhs.cp16 + begin,end - begin,
                          rhs.cp16,WSTR_LENGTH(rhs.cp16));
  if (ptr.cp16) result = ptr.cp16 - lhs.cp16;
  break;
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  ptr.cp32 = memcasermeml(lhs.cp32 + begin,end - begin,
                          rhs.cp32,WSTR_LENGTH(rhs.cp32));
  if (ptr.cp32) result = ptr.cp32 - lhs.cp32;
  break;
 }
 return DeeInt_NewSSize(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_caseindex(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 size_t result; union dcharptr ptr,lhs,rhs; size_t mylen;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caseindex",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_found;
  ptr.cp8 = memcasememb(lhs.cp8 + begin,end - begin,
                        rhs.cp8,WSTR_LENGTH(rhs.cp8));
  if (!ptr.cp8) goto not_found;
  result = (size_t)(ptr.cp8 - lhs.cp8);
  break;
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_found;
  ptr.cp16 = memcasememw(lhs.cp16 + begin,end - begin,
                         rhs.cp16,WSTR_LENGTH(rhs.cp16));
  if (!ptr.cp16) goto not_found;
  result = (size_t)(ptr.cp16 - lhs.cp16);
  break;
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_found;
  ptr.cp32 = memcasememl(lhs.cp32 + begin,end - begin,
                         rhs.cp32,WSTR_LENGTH(rhs.cp32));
  if (!ptr.cp32) goto not_found;
  result = (size_t)(ptr.cp32 - lhs.cp32);
  break;
 }
 return DeeInt_NewSize(result);
not_found:
 err_index_not_found((DeeObject *)self,
                     (DeeObject *)other);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_caserindex(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 size_t result; union dcharptr ptr,lhs,rhs; size_t mylen;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caserindex",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_found;
  ptr.cp8 = memcasermemb(lhs.cp8 + begin,end - begin,
                         rhs.cp8,WSTR_LENGTH(rhs.cp8));
  if (!ptr.cp8) goto not_found;
  result = (size_t)(ptr.cp8 - lhs.cp8);
  break;
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_found;
  ptr.cp16 = memcasermemw(lhs.cp16 + begin,end - begin,
                          rhs.cp16,WSTR_LENGTH(rhs.cp16));
  if (!ptr.cp16) goto not_found;
  result = (size_t)(ptr.cp16 - lhs.cp16);
  break;
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_found;
  ptr.cp32 = memcasermeml(lhs.cp32 + begin,end - begin,
                          rhs.cp32,WSTR_LENGTH(rhs.cp32));
  if (!ptr.cp32) goto not_found;
  result = (size_t)(ptr.cp32 - lhs.cp32);
  break;
 }
 return DeeInt_NewSize(result);
not_found:
 err_index_not_found((DeeObject *)self,
                     (DeeObject *)other);
err:
 return NULL;
}
PRIVATE DREF String *DCALL
string_getsubstr_ib(String *__restrict self,
                    size_t start, size_t end) {
 DREF String *result;
 void *str = DeeString_WSTR(self);
 size_t len = WSTR_LENGTH(str);
 ASSERT(start <= end);
 ASSERT(start <= len);
 ASSERT(end <= len);
 if (start == 0 && end >= len) {
  result = self;
  Dee_Incref(result);
 } else {
  int width = DeeString_WIDTH(self);
  result = (DREF String *)DeeString_NewWithWidth((uint8_t *)str+
                                                 (start*STRING_SIZEOF_WIDTH(width)),
                                                  end-(size_t)start,
                                                  width);
 }
 return result;
}
PRIVATE DREF String *DCALL
string_getsubstr(String *__restrict self,
                 size_t start, size_t end) {
 DREF String *result;
 void *str = DeeString_WSTR(self);
 size_t len = WSTR_LENGTH(str);
 if (start == 0 && end >= len) {
  result = self;
  Dee_Incref(result);
 } else {
  if (end >= len) end = len;
  if (start >= end) {
   result = (DREF String *)Dee_EmptyString;
   Dee_Incref(Dee_EmptyString);
  } else {
   int width = DeeString_WIDTH(self);
   result = (DREF String *)DeeString_NewWithWidth((uint8_t *)str+
                                                  (start*STRING_SIZEOF_WIDTH(width)),
                                                   end-(size_t)start,
                                                   width);
  }
 }
 return result;
}

PRIVATE DREF DeeObject *DCALL
string_substr(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:substr",&start,&end))
     return NULL;
 return (DREF DeeObject *)string_getsubstr(self,start,end);
}
PRIVATE DREF DeeObject *DCALL
string_strip(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:strip",&mask))
     return NULL;
 return mask ? (DeeObject_AssertTypeExact(mask,&DeeString_Type) ? NULL :
                DeeString_StripMask((DeeObject *)self,mask))
             :  DeeString_StripSpc((DeeObject *)self);
}
PRIVATE DREF DeeObject *DCALL
string_lstrip(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:lstrip",&mask))
     return NULL;
 return mask ? (DeeObject_AssertTypeExact(mask,&DeeString_Type) ? NULL :
                DeeString_LStripMask((DeeObject *)self,mask))
             :  DeeString_LStripSpc((DeeObject *)self);
}
PRIVATE DREF DeeObject *DCALL
string_rstrip(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:rstrip",&mask))
     return NULL;
 return mask ? (DeeObject_AssertTypeExact(mask,&DeeString_Type) ? NULL :
                DeeString_RStripMask((DeeObject *)self,mask))
             :  DeeString_RStripSpc((DeeObject *)self);
}
PRIVATE DREF DeeObject *DCALL
string_casestrip(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:casestrip",&mask))
     return NULL;
 return mask ? (DeeObject_AssertTypeExact(mask,&DeeString_Type) ? NULL :
                DeeString_CaseStripMask((DeeObject *)self,mask))
             :  DeeString_StripSpc((DeeObject *)self);
}
PRIVATE DREF DeeObject *DCALL
string_caselstrip(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:caselstrip",&mask))
     return NULL;
 return mask ? (DeeObject_AssertTypeExact(mask,&DeeString_Type) ? NULL :
                DeeString_CaseLStripMask((DeeObject *)self,mask))
             :  DeeString_LStripSpc((DeeObject *)self);
}
PRIVATE DREF DeeObject *DCALL
string_caserstrip(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:caserstrip",&mask))
     return NULL;
 return mask ? (DeeObject_AssertTypeExact(mask,&DeeString_Type) ? NULL :
                DeeString_CaseRStripMask((DeeObject *)self,mask))
             :  DeeString_RStripSpc((DeeObject *)self);
}

#undef CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS
#define CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS 1

PRIVATE DREF DeeObject *DCALL
string_startswith(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 union dcharptr my_str,ot_str; size_t my_len,ot_len;
 if (DeeArg_Unpack(argc,argv,"o|IdId:startswith",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
#ifdef CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS
 if (begin == 0 && end >= DeeString_WLEN(self) &&
     /* NOTE: This checks that `DeeString_STR()' being either LATIN-1, or
      *       UTF-8 is the same for both our own, and the `other' string. */
    (DeeString_STR_ISUTF8(self) == DeeString_STR_ISUTF8(other))) {
  /* Special case: Since we don't have to count characters, we can simply
   *               match the UTF-8 representations against each other. */
  if (DeeString_SIZE(other) > DeeString_SIZE(self) ||
     !MEMEQB(DeeString_STR(self),DeeString_STR(other),
             DeeString_SIZE(other)))
      goto nope;
  return_true;
 }
#endif /* CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS */
 /* Must decode the other string in order to match its contents
  * against data from our string at a specific offset. */
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (my_len > end) my_len = end;
  if (my_len <= begin) break;
  my_len -= begin;
  if (ot_len > my_len) goto nope;
  return_bool(MEMEQB(my_str.cp8+begin,ot_str.cp8,ot_len));
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)other);
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (my_len > end) my_len = end;
  if (my_len <= begin) break;
  my_len -= begin;
  if (ot_len > my_len) goto nope;
  return_bool(MEMEQW(my_str.cp16+begin,ot_str.cp16,ot_len));
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)other);
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (my_len > end) my_len = end;
  if (my_len <= begin) break;
  my_len -= begin;
  if (ot_len > my_len) goto nope;
  return_bool(MEMEQL(my_str.cp32+begin,ot_str.cp32,ot_len));
 }
 return_bool_(ot_len == 0);
nope:
 return_false;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_endswith(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 union dcharptr my_str,ot_str; size_t my_len,ot_len;
 if (DeeArg_Unpack(argc,argv,"o|IdId:endswith",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
#ifdef CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS
 if (begin == 0 && end >= DeeString_WLEN(self) &&
     /* NOTE: This checks that `DeeString_STR()' being either LATIN-1, or
      *       UTF-8 is the same for both our own, and the `other' string. */
    (DeeString_STR_ISUTF8(self) == DeeString_STR_ISUTF8(other))) {
  /* Special case: Since we don't have to count characters, we can simply
   *               match the UTF-8 representations against each other. */
  if (DeeString_SIZE(other) > DeeString_SIZE(self) ||
     !MEMEQB(DeeString_STR(self)+
            (DeeString_SIZE(self)-DeeString_SIZE(other)),
             DeeString_STR(other),DeeString_SIZE(other)))
      goto nope;
  return_true;
 }
#endif /* CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS */
 /* Must decode the other string in order to match its contents
  * against data from our string at a specific offset. */
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (my_len > end) my_len = end;
  if (my_len <= begin) break;
  my_len -= begin;
  if (ot_len > my_len) goto nope;
  begin += my_len;
  begin -= ot_len;
  return_bool(MEMEQB(my_str.cp8+begin,ot_str.cp8,ot_len));
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)other);
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (my_len > end) my_len = end;
  if (my_len <= begin) break;
  my_len -= begin;
  if (ot_len > my_len) goto nope;
  begin += my_len;
  begin -= ot_len;
  return_bool(MEMEQW(my_str.cp16+begin,ot_str.cp16,ot_len));
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)other);
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (my_len > end) my_len = end;
  if (my_len <= begin) break;
  my_len -= begin;
  if (ot_len > my_len) goto nope;
  begin += my_len;
  begin -= ot_len;
  return_bool(MEMEQL(my_str.cp32+begin,ot_str.cp32,ot_len));
 }
 return_bool_(ot_len == 0);
nope:
 return_false;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_casestartswith(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 union dcharptr my_str,ot_str; size_t my_len,ot_len;
 if (DeeArg_Unpack(argc,argv,"o|IdId:casestartswith",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
#ifdef CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS
 if (begin == 0 && end >= DeeString_WLEN(self) &&
     /* NOTE: This checks that `DeeString_STR()' being either LATIN-1, or
      *       UTF-8 is the same for both our own, and the `other' string. */
    (DeeString_STR_ISUTF8(self) == DeeString_STR_ISUTF8(other))) {
  /* Special case: Since we don't have to count characters, we can simply
   *               match the UTF-8 representations against each other. */
  if (DeeString_SIZE(other) > DeeString_SIZE(self) ||
     !MEMCASEEQB(DeeString_STR(self),DeeString_STR(other),
                 DeeString_SIZE(other)))
      goto nope;
  return_true;
 }
#endif /* CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS */
 /* Must decode the other string in order to match its contents
  * against data from our string at a specific offset. */
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (my_len > end) my_len = end;
  if (my_len <= begin) break;
  my_len -= begin;
  if (ot_len > my_len) goto nope;
  return_bool(MEMCASEEQB(my_str.cp8+begin,ot_str.cp8,ot_len));
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)other);
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (my_len > end) my_len = end;
  if (my_len <= begin) break;
  my_len -= begin;
  if (ot_len > my_len) goto nope;
  return_bool(MEMCASEEQW(my_str.cp16+begin,ot_str.cp16,ot_len));
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)other);
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (my_len > end) my_len = end;
  if (my_len <= begin) break;
  my_len -= begin;
  if (ot_len > my_len) goto nope;
  return_bool(MEMCASEEQL(my_str.cp32+begin,ot_str.cp32,ot_len));
 }
 return_bool_(ot_len == 0);
nope:
 return_false;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_caseendswith(String *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 union dcharptr my_str,ot_str; size_t my_len,ot_len;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caseendswith",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
#ifdef CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS
 if (begin == 0 && end >= DeeString_WLEN(self) &&
     /* NOTE: This checks that `DeeString_STR()' being either LATIN-1, or
      *       UTF-8 is the same for both our own, and the `other' string. */
    (DeeString_STR_ISUTF8(self) == DeeString_STR_ISUTF8(other))) {
  /* Special case: Since we don't have to count characters, we can simply
   *               match the UTF-8 representations against each other. */
  if (DeeString_SIZE(other) > DeeString_SIZE(self) ||
     !MEMCASEEQB(DeeString_STR(self)+
                (DeeString_SIZE(self)-DeeString_SIZE(other)),
                 DeeString_STR(other),DeeString_SIZE(other)))
      goto nope;
  return_true;
 }
#endif /* CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS */
 /* Must decode the other string in order to match its contents
  * against data from our string at a specific offset. */
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (my_len > end) my_len = end;
  if (my_len <= begin) break;
  my_len -= begin;
  if (ot_len > my_len) goto nope;
  begin += my_len;
  begin -= ot_len;
  return_bool(MEMCASEEQB(my_str.cp8+begin,ot_str.cp8,ot_len));
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)other);
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (my_len > end) my_len = end;
  if (my_len <= begin) break;
  my_len -= begin;
  if (ot_len > my_len) goto nope;
  begin += my_len;
  begin -= ot_len;
  return_bool(MEMCASEEQW(my_str.cp16+begin,ot_str.cp16,ot_len));
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)other);
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (my_len > end) my_len = end;
  if (my_len <= begin) break;
  my_len -= begin;
  if (ot_len > my_len) goto nope;
  begin += my_len;
  begin -= ot_len;
  return_bool(MEMCASEEQL(my_str.cp32+begin,ot_str.cp32,ot_len));
 }
 return_bool_(ot_len == 0);
nope:
 return_false;
err:
 return NULL;
}


struct codec_error {
    char name[8];
    int  flags;
};

PRIVATE struct codec_error const codec_error_db[] = {
    { "strict", STRING_ERROR_FSTRICT },
    { "replace", STRING_ERROR_FREPLAC },
    { "ignore", STRING_ERROR_FIGNORE }
};


INTERN DREF DeeObject *DCALL
string_decode(DeeObject *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeObject *name; char *errors = NULL;
 unsigned int error_mode = STRING_ERROR_FSTRICT;
 if (DeeArg_Unpack(argc,argv,"o|s:decode",&name,&errors) ||
     DeeObject_AssertTypeExact(name,&DeeString_Type))
     goto err;
 if (errors) {
  size_t i;
  for (i = 0; i < COMPILER_LENOF(codec_error_db); ++i) {
   if (strcmp(codec_error_db[i].name,errors) != 0)
       continue;
   error_mode = codec_error_db[i].flags;
   goto got_errors;
  }
  DeeError_Throwf(&DeeError_ValueError,
                  "Invalid error code %q",
                  errors);
  goto err;
 }
got_errors:
 return DeeCodec_Decode(self,name,error_mode);
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
string_encode(DeeObject *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeObject *name; char *errors = NULL;
 unsigned int error_mode = STRING_ERROR_FSTRICT;
 if (DeeArg_Unpack(argc,argv,"o|s:encode",&name,&errors) ||
     DeeObject_AssertTypeExact(name,&DeeString_Type))
     goto err;
 if (errors) {
  size_t i;
  for (i = 0; i < COMPILER_LENOF(codec_error_db); ++i) {
   if (strcmp(codec_error_db[i].name,errors) != 0)
       continue;
   error_mode = codec_error_db[i].flags;
   goto got_errors;
  }
  DeeError_Throwf(&DeeError_ValueError,
                  "Invalid error code %q",
                  errors);
  goto err;
 }
got_errors:
 return DeeCodec_Encode(self,name,error_mode);
err:
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_front(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 void *str = DeeString_WSTR(self);
 int width = DeeString_WIDTH(self);
 if (DeeArg_Unpack(argc,argv,":front"))
     return NULL;
 if unlikely(!WSTR_LENGTH(str)) {
  err_empty_sequence((DeeObject *)self);
  return NULL;
 }
 return DeeString_Chr(STRING_WIDTH_GETCHAR(width,str,0));
}

PRIVATE DREF DeeObject *DCALL
string_back(String *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 void *str = DeeString_WSTR(self);
 int width = DeeString_WIDTH(self);
 size_t length = WSTR_LENGTH(str);
 if (DeeArg_Unpack(argc,argv,":front"))
     return NULL;
 if unlikely(!length) {
  err_empty_sequence((DeeObject *)self);
  return NULL;
 }
 return DeeString_Chr(STRING_WIDTH_GETCHAR(width,str,length-1));
}

INTDEF DREF DeeObject *DCALL
DeeString_Segments(DeeObject *__restrict self, size_t segment_size);

PRIVATE DREF DeeObject *DCALL
string_segments(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 size_t segment_size;
 if (DeeArg_Unpack(argc,argv,"Iu:segments",&segment_size))
     return NULL;
 if unlikely(!segment_size) {
  err_invalid_segment_size(segment_size);
  return NULL;
 }
 return DeeString_Segments((DeeObject *)self,segment_size);
}

PRIVATE ATTR_COLD int DCALL err_empty_filler(void) {
 return DeeError_Throwf(&DeeError_ValueError,
                        "Empty filler");
}

PRIVATE DREF DeeObject *DCALL
string_center(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 size_t result_length,my_len,fl_len;
 size_t fill_front,fill_back;
 DeeObject *filler_ob = NULL;
 DREF DeeObject *result;
 union dcharptr dst,buf,my_str,fl_str;
 if (DeeArg_Unpack(argc,argv,"Iu|o:center",&result_length,&filler_ob))
     goto err;
 my_len = DeeString_WLEN(self);
 if (result_length <= my_len)
     return_reference_((DeeObject *)self);
 fill_front  = (result_length - my_len);
 fill_back   = fill_front/2;
 fill_front -= fill_back;
 if (filler_ob) {
  if (DeeObject_AssertTypeExact(filler_ob,&DeeString_Type))
      goto err;
  SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                          DeeString_WIDTH(filler_ob))) {
  CASE_WIDTH_1BYTE:
   my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
   fl_str.cp8 = DeeString_As1Byte((DeeObject *)filler_ob);
   fl_len     = WSTR_LENGTH(fl_str.cp8);
   if unlikely(!fl_len) goto empty_filler;
   result = DeeString_NewBuffer(result_length);
   if unlikely(!result) goto err;
   dst.cp8 = DeeString_As1Byte(result);
   memfilb(dst.cp8,fill_front,fl_str.cp8,fl_len);
   dst.cp8 += fill_front;
   memcpyb(dst.cp8,my_str.cp8,my_len);
   dst.cp8 += my_len;
   memfilb(dst.cp8,fill_back,fl_str.cp8,fl_len);
   break;
  CASE_WIDTH_2BYTE:
   my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
   if unlikely(!my_str.cp16) goto err;
   fl_str.cp16 = DeeString_As2Byte((DeeObject *)filler_ob);
   if unlikely(!fl_str.cp16) goto err;
   fl_len = WSTR_LENGTH(fl_str.cp8);
   if unlikely(!fl_len) goto empty_filler;
   dst.cp16 = buf.cp16 = DeeString_NewBuffer16(result_length);
   if unlikely(!buf.cp16) goto err;
   memfilw(dst.cp16,fill_front,fl_str.cp16,fl_len);
   dst.cp16 += fill_front;
   memcpyw(dst.cp16,my_str.cp16,my_len);
   dst.cp16 += my_len;
   memfilw(dst.cp16,fill_back,fl_str.cp16,fl_len);
   result = DeeString_Pack2ByteBuffer(buf.cp16);
   break;
  CASE_WIDTH_4BYTE:
   my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
   if unlikely(!my_str.cp32) goto err;
   fl_str.cp32 = DeeString_As4Byte((DeeObject *)filler_ob);
   if unlikely(!fl_str.cp32) goto err;
   fl_len = WSTR_LENGTH(fl_str.cp8);
   if unlikely(!fl_len) goto empty_filler;
   dst.cp32 = buf.cp32 = DeeString_NewBuffer32(result_length);
   if unlikely(!buf.cp32) goto err;
   memfill(dst.cp32,fill_front,fl_str.cp32,fl_len);
   dst.cp32 += fill_front;
   memcpyl(dst.cp32,my_str.cp32,my_len);
   dst.cp32 += my_len;
   memfill(dst.cp32,fill_back,fl_str.cp32,fl_len);
   result = DeeString_Pack4ByteBuffer(buf.cp32);
   break;
  }
 } else {
  SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
  CASE_WIDTH_1BYTE:
   my_str.cp8 = DeeString_Get1Byte((DeeObject *)self);
   result = DeeString_NewBuffer(result_length);
   if unlikely(!result) goto err;
   dst.cp8 = DeeString_As1Byte(result);
   memsetb(dst.cp8,UNICODE_SPACE,fill_front);
   dst.cp8 += fill_front;
   memcpyb(dst.cp8,my_str.cp8,my_len);
   dst.cp8 += my_len;
   memsetb(dst.cp8,UNICODE_SPACE,fill_back);
   break;
  CASE_WIDTH_2BYTE:
   my_str.cp16 = DeeString_Get2Byte((DeeObject *)self);
   dst.cp16 = buf.cp16 = DeeString_NewBuffer16(result_length);
   if unlikely(!buf.cp16) goto err;
   memsetw(dst.cp16,UNICODE_SPACE,fill_front);
   dst.cp16 += fill_front;
   memcpyw(dst.cp16,my_str.cp16,my_len);
   dst.cp16 += my_len;
   memsetw(dst.cp16,UNICODE_SPACE,fill_back);
   result = DeeString_Pack2ByteBuffer(buf.cp16);
   break;
  CASE_WIDTH_4BYTE:
   my_str.cp32 = DeeString_Get4Byte((DeeObject *)self);
   dst.cp32 = buf.cp32 = DeeString_NewBuffer32(result_length);
   if unlikely(!buf.cp32) goto err;
   memsetl(dst.cp32,UNICODE_SPACE,fill_front);
   dst.cp32 += fill_front;
   memcpyl(dst.cp32,my_str.cp32,my_len);
   dst.cp32 += my_len;
   memsetl(dst.cp32,UNICODE_SPACE,fill_back);
   result = DeeString_Pack4ByteBuffer(buf.cp32);
   break;
  }
 }
 return result;
empty_filler:
 err_empty_filler();
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_ljust(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 size_t result_length,my_len,fl_len,fill_back;
 DeeObject *filler_ob = NULL;
 DREF DeeObject *result;
 union dcharptr dst,buf,my_str,fl_str;
 if (DeeArg_Unpack(argc,argv,"Iu|o:ljust",&result_length,&filler_ob))
     goto err;
 my_len = DeeString_WLEN(self);
 if (result_length <= my_len)
     return_reference_((DeeObject *)self);
 fill_back = (result_length - my_len);
 if (filler_ob) {
  if (DeeObject_AssertTypeExact(filler_ob,&DeeString_Type))
      goto err;
  SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                          DeeString_WIDTH(filler_ob))) {
  CASE_WIDTH_1BYTE:
   my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
   fl_str.cp8 = DeeString_As1Byte((DeeObject *)filler_ob);
   fl_len     = WSTR_LENGTH(fl_str.cp8);
   if unlikely(!fl_len) goto empty_filler;
   result = DeeString_NewBuffer(result_length);
   if unlikely(!result) goto err;
   dst.cp8 = DeeString_As1Byte(result);
   memcpyb(dst.cp8,my_str.cp8,my_len);
   dst.cp8 += my_len;
   memfilb(dst.cp8,fill_back,fl_str.cp8,fl_len);
   break;
  CASE_WIDTH_2BYTE:
   my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
   if unlikely(!my_str.cp16) goto err;
   fl_str.cp16 = DeeString_As2Byte((DeeObject *)filler_ob);
   if unlikely(!fl_str.cp16) goto err;
   fl_len = WSTR_LENGTH(fl_str.cp8);
   if unlikely(!fl_len) goto empty_filler;
   dst.cp16 = buf.cp16 = DeeString_NewBuffer16(result_length);
   if unlikely(!buf.cp16) goto err;
   memcpyw(dst.cp16,my_str.cp16,my_len);
   dst.cp16 += my_len;
   memfilw(dst.cp16,fill_back,fl_str.cp16,fl_len);
   result = DeeString_Pack2ByteBuffer(buf.cp16);
   break;
  CASE_WIDTH_4BYTE:
   my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
   if unlikely(!my_str.cp32) goto err;
   fl_str.cp32 = DeeString_As4Byte((DeeObject *)filler_ob);
   if unlikely(!fl_str.cp32) goto err;
   fl_len = WSTR_LENGTH(fl_str.cp8);
   if unlikely(!fl_len) goto empty_filler;
   dst.cp32 = buf.cp32 = DeeString_NewBuffer32(result_length);
   if unlikely(!buf.cp32) goto err;
   memcpyl(dst.cp32,my_str.cp32,my_len);
   dst.cp32 += my_len;
   memfill(dst.cp32,fill_back,fl_str.cp32,fl_len);
   result = DeeString_Pack4ByteBuffer(buf.cp32);
   break;
  }
 } else {
  SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
  CASE_WIDTH_1BYTE:
   my_str.cp8 = DeeString_Get1Byte((DeeObject *)self);
   result = DeeString_NewBuffer(result_length);
   if unlikely(!result) goto err;
   dst.cp8 = DeeString_As1Byte(result);
   memcpyb(dst.cp8,my_str.cp8,my_len);
   dst.cp8 += my_len;
   memsetb(dst.cp8,UNICODE_SPACE,fill_back);
   break;
  CASE_WIDTH_2BYTE:
   my_str.cp16 = DeeString_Get2Byte((DeeObject *)self);
   dst.cp16 = buf.cp16 = DeeString_NewBuffer16(result_length);
   if unlikely(!buf.cp16) goto err;
   memcpyw(dst.cp16,my_str.cp16,my_len);
   dst.cp16 += my_len;
   memsetw(dst.cp16,UNICODE_SPACE,fill_back);
   result = DeeString_Pack2ByteBuffer(buf.cp16);
   break;
  CASE_WIDTH_4BYTE:
   my_str.cp32 = DeeString_Get4Byte((DeeObject *)self);
   dst.cp32 = buf.cp32 = DeeString_NewBuffer32(result_length);
   if unlikely(!buf.cp32) goto err;
   memcpyl(dst.cp32,my_str.cp32,my_len);
   dst.cp32 += my_len;
   memsetl(dst.cp32,UNICODE_SPACE,fill_back);
   result = DeeString_Pack4ByteBuffer(buf.cp32);
   break;
  }
 }
 return result;
empty_filler:
 err_empty_filler();
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_rjust(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 size_t result_length,my_len,fl_len,fill_front;
 DeeObject *filler_ob = NULL;
 DREF DeeObject *result;
 union dcharptr dst,buf,my_str,fl_str;
 if (DeeArg_Unpack(argc,argv,"Iu|o:rjust",&result_length,&filler_ob))
     goto err;
 my_len = DeeString_WLEN(self);
 if (result_length <= my_len)
     return_reference_((DeeObject *)self);
 fill_front = (result_length - my_len);
 if (filler_ob) {
  if (DeeObject_AssertTypeExact(filler_ob,&DeeString_Type))
      goto err;
  SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                          DeeString_WIDTH(filler_ob))) {
  CASE_WIDTH_1BYTE:
   my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
   fl_str.cp8 = DeeString_As1Byte((DeeObject *)filler_ob);
   fl_len     = WSTR_LENGTH(fl_str.cp8);
   if unlikely(!fl_len) goto empty_filler;
   result = DeeString_NewBuffer(result_length);
   if unlikely(!result) goto err;
   dst.cp8 = DeeString_As1Byte(result);
   memfilb(dst.cp8,fill_front,fl_str.cp8,fl_len);
   dst.cp8 += fill_front;
   memcpyb(dst.cp8,my_str.cp8,my_len);
   break;
  CASE_WIDTH_2BYTE:
   my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
   if unlikely(!my_str.cp16) goto err;
   fl_str.cp16 = DeeString_As2Byte((DeeObject *)filler_ob);
   if unlikely(!fl_str.cp16) goto err;
   fl_len = WSTR_LENGTH(fl_str.cp8);
   if unlikely(!fl_len) goto empty_filler;
   dst.cp16 = buf.cp16 = DeeString_NewBuffer16(result_length);
   if unlikely(!buf.cp16) goto err;
   memfilw(dst.cp16,fill_front,fl_str.cp16,fl_len);
   dst.cp16 += fill_front;
   memcpyw(dst.cp16,my_str.cp16,my_len);
   result = DeeString_Pack2ByteBuffer(buf.cp16);
   break;
  CASE_WIDTH_4BYTE:
   my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
   if unlikely(!my_str.cp32) goto err;
   fl_str.cp32 = DeeString_As4Byte((DeeObject *)filler_ob);
   if unlikely(!fl_str.cp32) goto err;
   fl_len = WSTR_LENGTH(fl_str.cp8);
   if unlikely(!fl_len) goto empty_filler;
   dst.cp32 = buf.cp32 = DeeString_NewBuffer32(result_length);
   if unlikely(!buf.cp32) goto err;
   memfill(dst.cp32,fill_front,fl_str.cp32,fl_len);
   dst.cp32 += fill_front;
   memcpyl(dst.cp32,my_str.cp32,my_len);
   result = DeeString_Pack4ByteBuffer(buf.cp32);
   break;
  }
 } else {
  SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
  CASE_WIDTH_1BYTE:
   my_str.cp8 = DeeString_Get1Byte((DeeObject *)self);
   result = DeeString_NewBuffer(result_length);
   if unlikely(!result) goto err;
   dst.cp8 = DeeString_As1Byte(result);
   memsetb(dst.cp8,UNICODE_SPACE,fill_front);
   dst.cp8 += fill_front;
   memcpyb(dst.cp8,my_str.cp8,my_len);
   break;
  CASE_WIDTH_2BYTE:
   my_str.cp16 = DeeString_Get2Byte((DeeObject *)self);
   dst.cp16 = buf.cp16 = DeeString_NewBuffer16(result_length);
   if unlikely(!buf.cp16) goto err;
   memsetw(dst.cp16,UNICODE_SPACE,fill_front);
   dst.cp16 += fill_front;
   memcpyw(dst.cp16,my_str.cp16,my_len);
   result = DeeString_Pack2ByteBuffer(buf.cp16);
   break;
  CASE_WIDTH_4BYTE:
   my_str.cp32 = DeeString_Get4Byte((DeeObject *)self);
   dst.cp32 = buf.cp32 = DeeString_NewBuffer32(result_length);
   if unlikely(!buf.cp32) goto err;
   memsetl(dst.cp32,UNICODE_SPACE,fill_front);
   dst.cp32 += fill_front;
   memcpyl(dst.cp32,my_str.cp32,my_len);
   result = DeeString_Pack4ByteBuffer(buf.cp32);
   break;
  }
 }
 return result;
empty_filler:
 err_empty_filler();
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_count(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 String *other; size_t result = 0;
 size_t begin = 0,end = (size_t)-1;
 union dcharptr lhs,rhs,endptr; size_t mylen;
 if (DeeArg_Unpack(argc,argv,"o|IdId:count",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen   = DeeString_SIZE(self);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  endptr.cp8 = lhs.cp8 + end;
  lhs.cp8 += begin;
  while ((lhs.cp8 = memmemb(lhs.cp8,endptr.cp8 - lhs.cp8,
                            rhs.cp8,WSTR_LENGTH(rhs.cp8))) != NULL)
        ++result,lhs.cp8 += WSTR_LENGTH(rhs.cp8);
  break;
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen    = WSTR_LENGTH(lhs.cp16);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  endptr.cp16 = lhs.cp16 + end;
  lhs.cp16 += begin;
  while ((lhs.cp16 = memmemw(lhs.cp16,endptr.cp16 - lhs.cp16,
                             rhs.cp16,WSTR_LENGTH(rhs.cp16))) != NULL)
        ++result,lhs.cp16 += WSTR_LENGTH(rhs.cp16);
  break;
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen    = WSTR_LENGTH(lhs.cp32);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  endptr.cp32 = lhs.cp32 + end;
  lhs.cp32 += begin;
  while ((lhs.cp32 = memmeml(lhs.cp32,endptr.cp32 - lhs.cp32,
                             rhs.cp32,WSTR_LENGTH(rhs.cp32))) != NULL)
        ++result,lhs.cp32 += WSTR_LENGTH(rhs.cp32);
  break;
 }
 return DeeInt_NewSize(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_casecount(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 String *other; size_t result = 0;
 size_t begin = 0,end = (size_t)-1;
 union dcharptr lhs,rhs,endptr; size_t mylen;
 if (DeeArg_Unpack(argc,argv,"o|IdId:casecount",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen   = DeeString_SIZE(self);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  endptr.cp8 = lhs.cp8 + end;
  lhs.cp8 += begin;
  while ((lhs.cp8 = memcasememb(lhs.cp8,endptr.cp8 - lhs.cp8,
                                rhs.cp8,WSTR_LENGTH(rhs.cp8))) != NULL)
        ++result,lhs.cp8 += WSTR_LENGTH(rhs.cp8);
  break;
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen    = WSTR_LENGTH(lhs.cp16);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  endptr.cp16 = lhs.cp16 + end;
  lhs.cp16 += begin;
  while ((lhs.cp16 = memcasememw(lhs.cp16,endptr.cp16 - lhs.cp16,
                                 rhs.cp16,WSTR_LENGTH(rhs.cp16))) != NULL)
        ++result,lhs.cp16 += WSTR_LENGTH(rhs.cp16);
  break;
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen    = WSTR_LENGTH(lhs.cp32);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) break;
  endptr.cp32 = lhs.cp32 + end;
  lhs.cp32 += begin;
  while ((lhs.cp32 = memcasememl(lhs.cp32,endptr.cp32 - lhs.cp32,
                                 rhs.cp32,WSTR_LENGTH(rhs.cp32))) != NULL)
        ++result,lhs.cp32 += WSTR_LENGTH(rhs.cp32);
  break;
 }
 return DeeInt_NewSize(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_contains_f(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 union dcharptr lhs,rhs; size_t mylen;
 if (DeeArg_Unpack(argc,argv,"o|IdId:contains",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen   = DeeString_SIZE(self);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto nope;
  if (!memmemb(lhs.cp8 + begin,end - begin,
               rhs.cp8,WSTR_LENGTH(rhs.cp8)))
       goto nope;
  break;
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen    = WSTR_LENGTH(lhs.cp16);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto nope;
  if (!memmemw(lhs.cp16 + begin,end - begin,
               rhs.cp16,WSTR_LENGTH(rhs.cp16)))
       goto nope;
  break;
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen    = WSTR_LENGTH(lhs.cp32);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto nope;
  if (!memmeml(lhs.cp32 + begin,end - begin,
               rhs.cp32,WSTR_LENGTH(rhs.cp32)))
       goto nope;
  break;
 }
 return_true;
nope:
 return_false;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_casecontains_f(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 String *other; size_t begin = 0,end = (size_t)-1;
 union dcharptr lhs,rhs; size_t mylen;
 if (DeeArg_Unpack(argc,argv,"o|IdId:contains",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen   = DeeString_SIZE(self);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto nope;
  if (!memcasememb(lhs.cp8 + begin,end - begin,
                   rhs.cp8,WSTR_LENGTH(rhs.cp8)))
       goto nope;
  break;
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen    = WSTR_LENGTH(lhs.cp16);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto nope;
  if (!memcasememw(lhs.cp16 + begin,end - begin,
                   rhs.cp16,WSTR_LENGTH(rhs.cp16)))
       goto nope;
  break;
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen    = WSTR_LENGTH(lhs.cp32);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto nope;
  if (!memcasememl(lhs.cp32 + begin,end - begin,
                   rhs.cp32,WSTR_LENGTH(rhs.cp32)))
       goto nope;
  break;
 }
 return_true;
nope:
 return_false;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_zfill(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 size_t result_length,my_len,fl_len,fill_front;
 DeeObject *filler_ob = NULL;
 DREF DeeObject *result;
 union dcharptr dst,buf,my_str,fl_str;
 if (DeeArg_Unpack(argc,argv,"Iu|o:zfill",&result_length,&filler_ob))
     goto err;
 my_len = DeeString_WLEN(self);
 if (result_length <= my_len)
     return_reference_((DeeObject *)self);
 fill_front = (result_length - my_len);
 if (filler_ob) {
  if (DeeObject_AssertTypeExact(filler_ob,&DeeString_Type))
      goto err;
  SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                          DeeString_WIDTH(filler_ob))) {
  CASE_WIDTH_1BYTE:
   my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
   fl_str.cp8 = DeeString_As1Byte((DeeObject *)filler_ob);
   fl_len     = WSTR_LENGTH(fl_str.cp8);
   if unlikely(!fl_len) goto empty_filler;
   result = DeeString_NewBuffer(result_length);
   if unlikely(!result) goto err;
   dst.cp8 = DeeString_As1Byte(result);
   while (my_len && DeeUni_IsSign(my_str.cp8[0])) {
    *dst.cp8++ = *my_str.cp8++;
    --my_len;
   }
   memfilb(dst.cp8,fill_front,fl_str.cp8,fl_len);
   dst.cp8 += fill_front;
   memcpyb(dst.cp8,my_str.cp8,my_len);
   break;
  CASE_WIDTH_2BYTE:
   my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
   if unlikely(!my_str.cp16) goto err;
   fl_str.cp16 = DeeString_As2Byte((DeeObject *)filler_ob);
   if unlikely(!fl_str.cp16) goto err;
   fl_len = WSTR_LENGTH(fl_str.cp8);
   if unlikely(!fl_len) goto empty_filler;
   dst.cp16 = buf.cp16 = DeeString_NewBuffer16(result_length);
   if unlikely(!buf.cp16) goto err;
   while (my_len && DeeUni_IsSign(my_str.cp16[0])) {
    *dst.cp16++ = *my_str.cp16++;
    --my_len;
   }
   memfilw(dst.cp16,fill_front,fl_str.cp16,fl_len);
   dst.cp16 += fill_front;
   memcpyw(dst.cp16,my_str.cp16,my_len);
   result = DeeString_Pack2ByteBuffer(buf.cp16);
   break;
  CASE_WIDTH_4BYTE:
   my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
   if unlikely(!my_str.cp32) goto err;
   fl_str.cp32 = DeeString_As4Byte((DeeObject *)filler_ob);
   if unlikely(!fl_str.cp32) goto err;
   fl_len = WSTR_LENGTH(fl_str.cp8);
   if unlikely(!fl_len) goto empty_filler;
   dst.cp32 = buf.cp32 = DeeString_NewBuffer32(result_length);
   if unlikely(!buf.cp32) goto err;
   while (my_len && DeeUni_IsSign(my_str.cp32[0])) {
    *dst.cp32++ = *my_str.cp32++;
    --my_len;
   }
   memfill(dst.cp32,fill_front,fl_str.cp32,fl_len);
   dst.cp32 += fill_front;
   memcpyl(dst.cp32,my_str.cp32,my_len);
   result = DeeString_Pack4ByteBuffer(buf.cp32);
   break;
  }
 } else {
  SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
  CASE_WIDTH_1BYTE:
   my_str.cp8 = DeeString_Get1Byte((DeeObject *)self);
   result = DeeString_NewBuffer(result_length);
   if unlikely(!result) goto err;
   dst.cp8 = DeeString_As1Byte(result);
   while (my_len && DeeUni_IsSign(my_str.cp8[0])) {
    *dst.cp8++ = *my_str.cp8++;
    --my_len;
   }
   memsetb(dst.cp8,UNICODE_ZERO,fill_front);
   dst.cp8 += fill_front;
   memcpyb(dst.cp8,my_str.cp8,my_len);
   break;
  CASE_WIDTH_2BYTE:
   my_str.cp16 = DeeString_Get2Byte((DeeObject *)self);
   dst.cp16 = buf.cp16 = DeeString_NewBuffer16(result_length);
   if unlikely(!buf.cp16) goto err;
   while (my_len && DeeUni_IsSign(my_str.cp16[0])) {
    *dst.cp16++ = *my_str.cp16++;
    --my_len;
   }
   memsetw(dst.cp16,UNICODE_ZERO,fill_front);
   dst.cp16 += fill_front;
   memcpyw(dst.cp16,my_str.cp16,my_len);
   result = DeeString_Pack2ByteBuffer(buf.cp16);
   break;
  CASE_WIDTH_4BYTE:
   my_str.cp32 = DeeString_Get4Byte((DeeObject *)self);
   dst.cp32 = buf.cp32 = DeeString_NewBuffer32(result_length);
   if unlikely(!buf.cp32) goto err;
   while (my_len && DeeUni_IsSign(my_str.cp32[0])) {
    *dst.cp32++ = *my_str.cp32++;
    --my_len;
   }
   memsetl(dst.cp32,UNICODE_ZERO,fill_front);
   dst.cp32 += fill_front;
   memcpyl(dst.cp32,my_str.cp32,my_len);
   result = DeeString_Pack4ByteBuffer(buf.cp32);
   break;
  }
 }
 return result;
empty_filler:
 err_empty_filler();
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_reversed(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 size_t begin = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:reversed",&begin,&end))
     return NULL;
 return DeeString_Reversed((DeeObject *)self,begin,end);
}
PRIVATE DREF DeeObject *DCALL
string_expandtabs(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 size_t tab_width = 8;
 if (DeeArg_Unpack(argc,argv,"|Iu:expandtabs",&tab_width))
     return NULL;
 return DeeString_ExpandTabs((DeeObject *)self,tab_width);
}
PRIVATE DREF DeeObject *DCALL
string_join(String *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DeeObject *items;
 if (DeeArg_Unpack(argc,argv,"o:join",&items))
     return NULL;
 return DeeString_Join((DeeObject *)self,items);
}
PRIVATE DREF DeeObject *DCALL
string_unifylines(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 DeeObject *replacement = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:unifylines",&replacement))
     return NULL;
 if likely(!replacement)
    return DeeString_UnifyLinesLf((DeeObject *)self);
 return DeeString_UnifyLines((DeeObject *)self,replacement);
}
PRIVATE DREF DeeObject *DCALL
partition_pack_notfoundb(String *__restrict self,
                         uint8_t *__restrict lhs,
                         size_t lhs_length) {
 DREF String *lhs_string;
 DREF DeeObject *result;
 result = DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 if (lhs == DeeString_Get1Byte((DeeObject *)self) &&
     WSTR_LENGTH(lhs) == lhs_length) {
  lhs_string = self;
  Dee_Incref(self);
 } else {
  lhs_string = (DREF String *)DeeString_New1Byte(lhs,lhs_length);
  if unlikely(!lhs_string) goto err_r;
 }
 Dee_Incref_n(Dee_EmptyString,2);
 DeeTuple_SET(result,0,(DeeObject *)lhs_string);
 DeeTuple_SET(result,1,(DeeObject *)Dee_EmptyString);
 DeeTuple_SET(result,2,(DeeObject *)Dee_EmptyString);
 return result;
err_r:
 DeeTuple_FreeUninitialized(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
partition_pack_notfoundw(String *__restrict self,
                         uint16_t *__restrict lhs,
                         size_t lhs_length) {
 DREF String *lhs_string;
 DREF DeeObject *result;
 result = DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 if (lhs == DeeString_Get2Byte((DeeObject *)self) &&
     WSTR_LENGTH(lhs) == lhs_length) {
  lhs_string = self;
  Dee_Incref(self);
 } else {
  lhs_string = (DREF String *)DeeString_New2Byte(lhs,lhs_length);
  if unlikely(!lhs_string) goto err_r;
 }
 Dee_Incref_n(Dee_EmptyString,2);
 DeeTuple_SET(result,0,(DeeObject *)lhs_string);
 DeeTuple_SET(result,1,(DeeObject *)Dee_EmptyString);
 DeeTuple_SET(result,2,(DeeObject *)Dee_EmptyString);
 return result;
err_r:
 DeeTuple_FreeUninitialized(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
partition_pack_notfoundl(String *__restrict self,
                         uint32_t *__restrict lhs,
                         size_t lhs_length) {
 DREF String *lhs_string;
 DREF DeeObject *result;
 result = DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 if (lhs == DeeString_Get4Byte((DeeObject *)self) &&
     WSTR_LENGTH(lhs) == lhs_length) {
  lhs_string = self;
  Dee_Incref(self);
 } else {
  lhs_string = (DREF String *)DeeString_New4Byte(lhs,lhs_length);
  if unlikely(!lhs_string) goto err_r;
 }
 Dee_Incref_n(Dee_EmptyString,2);
 DeeTuple_SET(result,0,(DeeObject *)lhs_string);
 DeeTuple_SET(result,1,(DeeObject *)Dee_EmptyString);
 DeeTuple_SET(result,2,(DeeObject *)Dee_EmptyString);
 return result;
err_r:
 DeeTuple_FreeUninitialized(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
partition_packb(String *__restrict other,
                uint8_t *__restrict lhs, size_t lhs_length,
                uint8_t *__restrict ptr, size_t ptr_length) {
 DREF String *prestr,*poststr;
 DREF DeeObject *result;
 result = DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 if (ptr == lhs) {
  uint8_t *post_ptr = ptr + ptr_length;
  poststr = (DREF String *)DeeString_New1Byte(post_ptr,
                                              lhs_length - ptr_length);
  if unlikely(!poststr) goto err_r;
  prestr = (DREF String *)Dee_EmptyString;
  Dee_Incref(prestr);
 } else if (ptr == lhs + lhs_length - ptr_length) {
  prestr = (DREF String *)DeeString_New1Byte(lhs,
                                             lhs_length - ptr_length);
  if unlikely(!prestr) goto err_r;
  poststr = (DREF String *)Dee_EmptyString;
  Dee_Incref(poststr);
 } else {
  uint8_t *post_ptr = ptr + ptr_length;
  prestr = (DREF String *)DeeString_New1Byte(lhs,ptr - lhs);
  if unlikely(!prestr) goto err_r;
  poststr = (DREF String *)DeeString_New1Byte(post_ptr,(lhs + lhs_length) - post_ptr);
  if unlikely(!prestr) goto err_r_pre;
 }
 DeeTuple_SET(result,0,(DeeObject *)prestr);
 DeeTuple_SET(result,1,(DeeObject *)other);
 DeeTuple_SET(result,2,(DeeObject *)poststr);
 Dee_Incref(other);
 return result;
err_r_pre:
 Dee_Decref_likely(prestr);
err_r:
 DeeTuple_FreeUninitialized(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
partition_packw(String *__restrict other,
                uint16_t *__restrict lhs, size_t lhs_length,
                uint16_t *__restrict ptr, size_t ptr_length) {
 DREF String *prestr,*poststr;
 DREF DeeObject *result;
 result = DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 if (ptr == lhs) {
  uint16_t *post_ptr = ptr + ptr_length;
  poststr = (DREF String *)DeeString_New2Byte(post_ptr,
                                               lhs_length - ptr_length);
  if unlikely(!poststr) goto err_r;
  prestr = (DREF String *)Dee_EmptyString;
  Dee_Incref(prestr);
 } else if (ptr == lhs + lhs_length - ptr_length) {
  prestr = (DREF String *)DeeString_New2Byte(lhs,
                                              lhs_length - ptr_length);
  if unlikely(!prestr) goto err_r;
  poststr = (DREF String *)Dee_EmptyString;
  Dee_Incref(poststr);
 } else {
  uint16_t *post_ptr = ptr + ptr_length;
  prestr = (DREF String *)DeeString_New2Byte(lhs,ptr - lhs);
  if unlikely(!prestr) goto err_r;
  poststr = (DREF String *)DeeString_New2Byte(post_ptr,(lhs + lhs_length) - post_ptr);
  if unlikely(!prestr) goto err_r_pre;
 }
 DeeTuple_SET(result,0,(DeeObject *)prestr);
 DeeTuple_SET(result,1,(DeeObject *)other);
 DeeTuple_SET(result,2,(DeeObject *)poststr);
 Dee_Incref(other);
 return result;
err_r_pre:
 Dee_Decref_likely(prestr);
err_r:
 DeeTuple_FreeUninitialized(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
partition_packl(String *__restrict other,
                uint32_t *__restrict lhs, size_t lhs_length,
                uint32_t *__restrict ptr, size_t ptr_length) {
 DREF String *prestr,*poststr;
 DREF DeeObject *result;
 result = DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 if (ptr == lhs) {
  uint32_t *post_ptr = ptr + ptr_length;
  poststr = (DREF String *)DeeString_New4Byte(post_ptr,
                                               lhs_length - ptr_length);
  if unlikely(!poststr) goto err_r;
  prestr = (DREF String *)Dee_EmptyString;
  Dee_Incref(prestr);
 } else if (ptr == lhs + lhs_length - ptr_length) {
  prestr = (DREF String *)DeeString_New4Byte(lhs,
                                              lhs_length - ptr_length);
  if unlikely(!prestr) goto err_r;
  poststr = (DREF String *)Dee_EmptyString;
  Dee_Incref(poststr);
 } else {
  uint32_t *post_ptr = ptr + ptr_length;
  prestr = (DREF String *)DeeString_New4Byte(lhs,ptr - lhs);
  if unlikely(!prestr) goto err_r;
  poststr = (DREF String *)DeeString_New4Byte(post_ptr,(lhs + lhs_length) - post_ptr);
  if unlikely(!prestr) goto err_r_pre;
 }
 DeeTuple_SET(result,0,(DeeObject *)prestr);
 DeeTuple_SET(result,1,(DeeObject *)other);
 DeeTuple_SET(result,2,(DeeObject *)poststr);
 Dee_Incref(other);
 return result;
err_r_pre:
 Dee_Decref_likely(prestr);
err_r:
 DeeTuple_FreeUninitialized(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_parition(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 String *other; union dcharptr lhs,rhs,ptr;
 size_t mylen,begin = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:parition",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_foundb_zero;
  end -= begin;
  lhs.cp8 += begin;
  ptr.cp8 = memmemb(lhs.cp8,end,
                    rhs.cp8,WSTR_LENGTH(rhs.cp8));
  if (!ptr.cp8) goto not_foundb;
  return partition_packb(other,
                         lhs.cp8,end,
                         ptr.cp8,WSTR_LENGTH(rhs.cp8));
not_foundb_zero:
  end = 0;
not_foundb:
  return partition_pack_notfoundb(self,lhs.cp8,end);
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_foundw_zero;
  end -= begin;
  lhs.cp16 += begin;
  ptr.cp16 = memmemw(lhs.cp16,end,
                     rhs.cp16,WSTR_LENGTH(rhs.cp16));
  if (!ptr.cp16) goto not_foundw;
  return partition_packw(other,
                         lhs.cp16,end,
                         ptr.cp16,WSTR_LENGTH(rhs.cp16));
not_foundw_zero:
  end = 0;
not_foundw:
  return partition_pack_notfoundw(self,lhs.cp16,end);
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_foundl_zero;
  end -= begin;
  lhs.cp32 += begin;
  ptr.cp32 = memmeml(lhs.cp32,end,
                     rhs.cp32,WSTR_LENGTH(rhs.cp32));
  if (!ptr.cp32) goto not_foundl;
  return partition_packl(other,
                         lhs.cp32,end,
                         ptr.cp32,WSTR_LENGTH(rhs.cp32));
not_foundl_zero:
  end = 0;
not_foundl:
  return partition_pack_notfoundl(self,lhs.cp32,end);
 }
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_rparition(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 String *other; union dcharptr lhs,rhs,ptr;
 size_t mylen,begin = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:rparition",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_foundb_zero;
  end -= begin;
  lhs.cp8 += begin;
  ptr.cp8 = memrmemb(lhs.cp8,end,
                     rhs.cp8,WSTR_LENGTH(rhs.cp8));
  if (!ptr.cp8) goto not_foundb;
  return partition_packb(other,
                         lhs.cp8,end,
                         ptr.cp8,WSTR_LENGTH(rhs.cp8));
not_foundb_zero:
  end = 0;
not_foundb:
  return partition_pack_notfoundb(self,lhs.cp8,end);
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_foundw_zero;
  end -= begin;
  lhs.cp16 += begin;
  ptr.cp16 = memrmemw(lhs.cp16,end,
                      rhs.cp16,WSTR_LENGTH(rhs.cp16));
  if (!ptr.cp16) goto not_foundw;
  return partition_packw(other,
                         lhs.cp16,end,
                         ptr.cp16,WSTR_LENGTH(rhs.cp16));
not_foundw_zero:
  end = 0;
not_foundw:
  return partition_pack_notfoundw(self,lhs.cp16,end);
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_foundl_zero;
  end -= begin;
  lhs.cp32 += begin;
  ptr.cp32 = memrmeml(lhs.cp32,end,
                      rhs.cp32,WSTR_LENGTH(rhs.cp32));
  if (!ptr.cp32) goto not_foundl;
  return partition_packl(other,
                         lhs.cp32,end,
                         ptr.cp32,WSTR_LENGTH(rhs.cp32));
not_foundl_zero:
  end = 0;
not_foundl:
  return partition_pack_notfoundl(self,lhs.cp32,end);
 }
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_caseparition(String *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 String *other; union dcharptr lhs,rhs,ptr;
 size_t mylen,begin = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caseparition",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_foundb_zero;
  end -= begin;
  lhs.cp8 += begin;
  ptr.cp8 = memcasememb(lhs.cp8,end,
                        rhs.cp8,WSTR_LENGTH(rhs.cp8));
  if (!ptr.cp8) goto not_foundb;
  return partition_packb(other,
                         lhs.cp8,end,
                         ptr.cp8,WSTR_LENGTH(rhs.cp8));
not_foundb_zero:
  end = 0;
not_foundb:
  return partition_pack_notfoundb(self,lhs.cp8,end);
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_foundw_zero;
  end -= begin;
  lhs.cp16 += begin;
  ptr.cp16 = memcasememw(lhs.cp16,end,
                         rhs.cp16,WSTR_LENGTH(rhs.cp16));
  if (!ptr.cp16) goto not_foundw;
  return partition_packw(other,
                         lhs.cp16,end,
                         ptr.cp16,WSTR_LENGTH(rhs.cp16));
not_foundw_zero:
  end = 0;
not_foundw:
  return partition_pack_notfoundw(self,lhs.cp16,end);
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_foundl_zero;
  end -= begin;
  lhs.cp32 += begin;
  ptr.cp32 = memcasememl(lhs.cp32,end,
                         rhs.cp32,WSTR_LENGTH(rhs.cp32));
  if (!ptr.cp32) goto not_foundl;
  return partition_packl(other,
                         lhs.cp32,end,
                         ptr.cp32,WSTR_LENGTH(rhs.cp32));
not_foundl_zero:
  end = 0;
not_foundl:
  return partition_pack_notfoundl(self,lhs.cp32,end);
 }
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_caserparition(String *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
 String *other; union dcharptr lhs,rhs,ptr;
 size_t mylen,begin = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caserparition",&other,&begin,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)other,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(other))) {
 CASE_WIDTH_1BYTE:
  lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
  rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_foundb_zero;
  end -= begin;
  lhs.cp8 += begin;
  ptr.cp8 = memcasermemb(lhs.cp8,end,
                         rhs.cp8,WSTR_LENGTH(rhs.cp8));
  if (!ptr.cp8) goto not_foundb;
  return partition_packb(other,
                         lhs.cp8,end,
                         ptr.cp8,WSTR_LENGTH(rhs.cp8));
not_foundb_zero:
  end = 0;
not_foundb:
  return partition_pack_notfoundb(self,lhs.cp8,end);
 CASE_WIDTH_2BYTE:
  lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!lhs.cp16) goto err;
  rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
  if unlikely(!rhs.cp16) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_foundw_zero;
  end -= begin;
  lhs.cp16 += begin;
  ptr.cp16 = memcasermemw(lhs.cp16,end,
                          rhs.cp16,WSTR_LENGTH(rhs.cp16));
  if (!ptr.cp16) goto not_foundw;
  return partition_packw(other,
                         lhs.cp16,end,
                         ptr.cp16,WSTR_LENGTH(rhs.cp16));
not_foundw_zero:
  end = 0;
not_foundw:
  return partition_pack_notfoundw(self,lhs.cp16,end);
 CASE_WIDTH_4BYTE:
  lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!lhs.cp32) goto err;
  rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
  if unlikely(!rhs.cp32) goto err;
  mylen = WSTR_LENGTH(lhs.ptr);
  if unlikely(begin > mylen) begin = mylen;
  if likely(end > mylen) end = mylen;
  if unlikely(end <= begin) goto not_foundl_zero;
  end -= begin;
  lhs.cp32 += begin;
  ptr.cp32 = memcasermeml(lhs.cp32,end,
                          rhs.cp32,WSTR_LENGTH(rhs.cp32));
  if (!ptr.cp32) goto not_foundl;
  return partition_packl(other,
                         lhs.cp32,end,
                         ptr.cp32,WSTR_LENGTH(rhs.cp32));
not_foundl_zero:
  end = 0;
not_foundl:
  return partition_pack_notfoundl(self,lhs.cp32,end);
 }
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_sstrip(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:sstrip",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_SStrip((DeeObject *)self,other);
}
PRIVATE DREF DeeObject *DCALL
string_lsstrip(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:lsstrip",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_LSStrip((DeeObject *)self,other);
}
PRIVATE DREF DeeObject *DCALL
string_rsstrip(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:rsstrip",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_RSStrip((DeeObject *)self,other);
}
PRIVATE DREF DeeObject *DCALL
string_casesstrip(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:casesstrip",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_CaseSStrip((DeeObject *)self,other);
}
PRIVATE DREF DeeObject *DCALL
string_caselsstrip(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:caselsstrip",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_CaseLSStrip((DeeObject *)self,other);
}
PRIVATE DREF DeeObject *DCALL
string_casersstrip(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:casersstrip",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_CaseRSStrip((DeeObject *)self,other);
}

struct compare_args {
    DeeObject *other;
    size_t     my_start;
    size_t     my_end;
    size_t     ot_start;
    size_t     ot_end;
};

PRIVATE int DCALL
get_compare_args(struct compare_args *__restrict args,
                 size_t argc, DeeObject **__restrict argv,
                 char const *__restrict funname) {
 args->my_start = 0;
 args->my_end   = (size_t)-1;
 args->ot_start = 0;
 args->ot_end   = (size_t)-1;
 switch (argc) {
 case 1:
  args->other = argv[0];
  if (DeeObject_AssertTypeExact(args->other,&DeeString_Type))
      goto err;
  break;
 case 2:
  if (DeeString_Check(argv[0])) {
   args->other = argv[0];
   if (DeeObject_AsSSize(argv[1],(dssize_t *)&args->ot_start)) goto err;
  } else {
   if (DeeObject_AsSSize(argv[0],(dssize_t *)&args->my_start)) goto err;
   args->other = argv[1];
   if (DeeObject_AssertTypeExact(args->other,&DeeString_Type))
       goto err;
  }
  break;
 case 3:
  if (DeeString_Check(argv[0])) {
   args->other = argv[0];
   if (DeeObject_AsSSize(argv[1],(dssize_t *)&args->ot_start)) goto err;
   if (DeeObject_AsSSize(argv[2],(dssize_t *)&args->ot_end)) goto err;
  } else if (DeeString_Check(argv[1])) {
   if (DeeObject_AsSSize(argv[0],(dssize_t *)&args->my_start)) goto err;
   args->other = argv[1];
   if (DeeObject_AsSSize(argv[2],(dssize_t *)&args->ot_start)) goto err;
  } else {
   if (DeeObject_AsSSize(argv[0],(dssize_t *)&args->my_start)) goto err;
   if (DeeObject_AsSSize(argv[1],(dssize_t *)&args->my_end)) goto err;
   args->other = argv[2];
   if (DeeObject_AssertTypeExact(args->other,&DeeString_Type))
       goto err;
  }
  break;
 case 4:
  if (DeeObject_AsSSize(argv[0],(dssize_t *)&args->my_start)) goto err;
  if (DeeString_Check(argv[1])) {
   args->other = argv[1];
   if (DeeObject_AsSSize(argv[2],(dssize_t *)&args->ot_start)) goto err;
   if (DeeObject_AsSSize(argv[3],(dssize_t *)&args->ot_end)) goto err;
  } else {
   if (DeeObject_AsSSize(argv[1],(dssize_t *)&args->my_end)) goto err;
   args->other = argv[2];
   if (DeeObject_AsSSize(argv[3],(dssize_t *)&args->ot_start)) goto err;
   if (DeeObject_AssertTypeExact(args->other,&DeeString_Type))
       goto err;
  }
  break;
 case 5:
  if (DeeObject_AsSSize(argv[0],(dssize_t *)&args->my_start)) goto err;
  if (DeeObject_AsSSize(argv[1],(dssize_t *)&args->my_end)) goto err;
  args->other = argv[2];
  if (DeeObject_AssertTypeExact(args->other,&DeeString_Type))
      goto err;
  if (DeeObject_AsSSize(argv[3],(dssize_t *)&args->ot_start)) goto err;
  if (DeeObject_AsSSize(argv[4],(dssize_t *)&args->ot_end)) goto err;
  break;
 default:
  err_invalid_argc(funname,argc,1,5);
  goto err;
 }
 return 0;
err:
 return -1;
}


PRIVATE int DCALL
compare_strings_ex(String *__restrict lhs, size_t lhs_start, size_t lhs_end,
                   String *__restrict rhs, size_t rhs_start, size_t rhs_end) {
 size_t lhs_len;
 size_t rhs_len;
 if (!lhs->s_data ||
      lhs->s_data->u_width == STRING_WIDTH_1BYTE) {
  uint8_t *lhs_str;
  /* Compare against single-byte string. */
  lhs_str = (uint8_t *)lhs->s_str;
  lhs_len = lhs->s_len;
  if (lhs_end > lhs_len)
      lhs_end = lhs_len;
  if (lhs_start >= lhs_end)
      lhs_len = 0;
  else {
   lhs_str += lhs_start;
   lhs_len  = lhs_end - lhs_start;
  }
  if (!rhs->s_data ||
       rhs->s_data->u_width == STRING_WIDTH_1BYTE) {
   int result;
   rhs_len = rhs->s_len;
   if (rhs_end > rhs_len)
       rhs_end = rhs_len;
   if (rhs_start >= rhs_end)
       rhs_len = 0;
   else {
    rhs_len  = rhs_end - rhs_start;
   }
   /* Most simple case: compare ascii/single-byte strings. */
   result = memcmp(lhs_str,rhs->s_str + rhs_start,MIN(lhs_len,rhs_len));
   if (result != 0) return result;
  } else {
   struct string_utf *rhs_utf = rhs->s_data;
   switch (rhs_utf->u_width) {

   {
    uint16_t *rhs_str;
    size_t i,common_len;
   CASE_WIDTH_2BYTE:
    rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
    rhs_len = WSTR_LENGTH(rhs_str);
    if (rhs_end > rhs_len)
        rhs_end = rhs_len;
    if (rhs_start >= rhs_end)
        rhs_len = 0;
    else {
     rhs_str += rhs_start;
     rhs_len  = rhs_end - rhs_start;
    }
    common_len = MIN(lhs_len,rhs_len);
    for (i = 0; i < common_len; ++i) {
     if ((uint16_t)(lhs_str[i]) == rhs_str[i])
         continue;
     return (uint16_t)(lhs_str[i]) < rhs_str[i] ? -1 : 1;
    }
   } break;

   {
    uint32_t *rhs_str;
    size_t i,common_len;
   CASE_WIDTH_4BYTE:
    rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
    rhs_len = WSTR_LENGTH(rhs_str);
    if (rhs_end > rhs_len)
        rhs_end = rhs_len;
    if (rhs_start >= rhs_end)
        rhs_len = 0;
    else {
     rhs_str += rhs_start;
     rhs_len  = rhs_end - rhs_start;
    }
    common_len = MIN(lhs_len,rhs_len);
    for (i = 0; i < common_len; ++i) {
     if ((uint32_t)(lhs_str[i]) == rhs_str[i])
         continue;
     return (uint32_t)(lhs_str[i]) < rhs_str[i] ? -1 : 1;
    }
   } break;

   default: __builtin_unreachable();
   }
  }
 } else if (!rhs->s_data ||
             rhs->s_data->u_width == STRING_WIDTH_1BYTE) {
  uint8_t *rhs_str;
  struct string_utf *lhs_utf;
  /* Compare against single-byte string. */
  rhs_str = (uint8_t *)rhs->s_str;
  rhs_len = rhs->s_len;
  if (rhs_end > rhs_len)
      rhs_end = rhs_len;
  if (rhs_start >= rhs_end)
      rhs_len = 0;
  else {
   rhs_str += rhs_start;
   rhs_len  = rhs_end - rhs_start;
  }
  lhs_utf = lhs->s_data;
  switch (lhs_utf->u_width) {

  {
   uint16_t *lhs_str;
   size_t i,common_len;
  CASE_WIDTH_2BYTE:
   lhs_str = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
   lhs_len = WSTR_LENGTH(lhs_str);
   if (lhs_end > lhs_len)
       lhs_end = lhs_len;
   if (lhs_start >= lhs_end)
       lhs_len = 0;
   else {
    lhs_str += lhs_start;
    lhs_len  = lhs_end - lhs_start;
   }
   common_len = MIN(rhs_len,lhs_len);
   for (i = 0; i < common_len; ++i) {
    if (lhs_str[i] == (uint16_t)(rhs_str[i]))
        continue;
    return lhs_str[i] < (uint16_t)(rhs_str[i]) ? -1 : 1;
   }
  } break;

  {
   uint32_t *lhs_str;
   size_t i,common_len;
  CASE_WIDTH_4BYTE:
   lhs_str = (uint32_t *)lhs_utf->u_data[lhs_utf->u_width];
   lhs_len = WSTR_LENGTH(lhs_str);
   if (lhs_end > lhs_len)
       lhs_end = lhs_len;
   if (lhs_start >= lhs_end)
       lhs_len = 0;
   else {
    lhs_str += lhs_start;
    lhs_len  = lhs_end - lhs_start;
   }
   common_len = MIN(rhs_len,lhs_len);
   for (i = 0; i < common_len; ++i) {
    if (lhs_str[i] == (uint32_t)(rhs_str[i]))
        continue;
    return lhs_str[i] < (uint32_t)(rhs_str[i]) ? -1 : 1;
   }
  } break;

  default: __builtin_unreachable();
  }
 } else {
  struct string_utf *lhs_utf;
  struct string_utf *rhs_utf;
  lhs_utf = lhs->s_data;
  rhs_utf = rhs->s_data;
  ASSERT(lhs_utf);
  ASSERT(rhs_utf);
  ASSERT(lhs_utf->u_width != STRING_WIDTH_1BYTE);
  ASSERT(rhs_utf->u_width != STRING_WIDTH_1BYTE);
  /* Complex string comparison. */
  switch (lhs_utf->u_width) {

  {
   uint16_t *lhs_str;
  CASE_WIDTH_2BYTE:
   lhs_str = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
   lhs_len = WSTR_LENGTH(lhs_str);
   if (lhs_end > lhs_len)
       lhs_end = lhs_len;
   if (lhs_start >= lhs_end)
       lhs_len = 0;
   else {
    lhs_str += lhs_start;
    lhs_len  = lhs_end - lhs_start;
   }
   switch (rhs_utf->u_width) {

   {
    uint16_t *rhs_str;
    size_t common_len;
   CASE_WIDTH_2BYTE:
    rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
    rhs_len = WSTR_LENGTH(rhs_str);
    if (rhs_end > rhs_len)
        rhs_end = rhs_len;
    if (rhs_start >= rhs_end)
        rhs_len = 0;
    else {
     rhs_str += rhs_start;
     rhs_len  = rhs_end - rhs_start;
    }
    common_len = MIN(lhs_len,rhs_len);
#ifdef __USE_KOS
    {
     int result;
     result = memcmpw(lhs_str,rhs_str,common_len);
     if (result != 0) return result;
    }
#else
    {
     size_t i;
     for (i = 0; i < common_len; ++i) {
      if (lhs_str[i] == rhs_str[i])
          continue;
      return lhs_str[i] < rhs_str[i] ? -1 : 1;
     }
    }
#endif
   } break;

   {
    uint32_t *rhs_str;
    size_t i,common_len;
   CASE_WIDTH_4BYTE:
    rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
    rhs_len = WSTR_LENGTH(rhs_str);
    if (rhs_end > rhs_len)
        rhs_end = rhs_len;
    if (rhs_start >= rhs_end)
        rhs_len = 0;
    else {
     rhs_str += rhs_start;
     rhs_len  = rhs_end - rhs_start;
    }
    common_len = MIN(lhs_len,rhs_len);
    for (i = 0; i < common_len; ++i) {
     if ((uint32_t)(lhs_str[i]) == rhs_str[i])
         continue;
     return (uint32_t)(lhs_str[i]) < rhs_str[i] ? -1 : 1;
    }
   } break;

   default: __builtin_unreachable();
   }
  } break;

  {
   uint32_t *lhs_str;
  CASE_WIDTH_4BYTE:
   lhs_str = (uint32_t *)lhs_utf->u_data[lhs_utf->u_width];
   lhs_len = WSTR_LENGTH(lhs_str);
   if (lhs_end > lhs_len)
       lhs_end = lhs_len;
   if (lhs_start >= lhs_end)
       lhs_len = 0;
   else {
    lhs_str += lhs_start;
    lhs_len  = lhs_end - lhs_start;
   }
   switch (rhs_utf->u_width) {

   {
    uint16_t *rhs_str;
    size_t i,common_len;
   CASE_WIDTH_2BYTE:
    rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
    rhs_len = WSTR_LENGTH(rhs_str);
    if (rhs_end > rhs_len)
        rhs_end = rhs_len;
    if (rhs_start >= rhs_end)
        rhs_len = 0;
    else {
     rhs_str += rhs_start;
     rhs_len  = rhs_end - rhs_start;
    }
    common_len = MIN(lhs_len,rhs_len);
    for (i = 0; i < common_len; ++i) {
     if (lhs_str[i] == (uint32_t)(rhs_str[i]))
         continue;
     return lhs_str[i] < (uint32_t)(rhs_str[i]) ? -1 : 1;
    }
   } break;

   {
    uint32_t *rhs_str;
    size_t common_len;
   CASE_WIDTH_4BYTE:
    rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
    rhs_len = WSTR_LENGTH(rhs_str);
    if (rhs_end > rhs_len)
        rhs_end = rhs_len;
    if (rhs_start >= rhs_end)
        rhs_len = 0;
    else {
     rhs_str += rhs_start;
     rhs_len  = rhs_end - rhs_start;
    }
    common_len = MIN(lhs_len,rhs_len);
#ifdef __USE_KOS
    {
     int result;
     result = memcmpl(lhs_str,rhs_str,common_len);
     if (result != 0) return result;
    }
#else
    {
     size_t i;
     for (i = 0; i < common_len; ++i) {
      if (lhs_str[i] == rhs_str[i])
          continue;
      return lhs_str[i] < rhs_str[i] ? -1 : 1;
     }
    }
#endif
   } break;

   default: __builtin_unreachable();
   }
  } break;

  default: __builtin_unreachable();
  }
 }

 /* If string contents are identical, leave off by comparing their lengths. */
 if (lhs_len == rhs_len)
     return 0;
 if (lhs_len < rhs_len)
     return -1;
 return 1;
}

PRIVATE int DCALL
casecompare_strings_ex(String *__restrict lhs, size_t lhs_start, size_t lhs_end,
                       String *__restrict rhs, size_t rhs_start, size_t rhs_end) {
 size_t lhs_len;
 size_t rhs_len;
 if (!lhs->s_data ||
      lhs->s_data->u_width == STRING_WIDTH_1BYTE) {
  uint8_t *lhs_str;
  /* Compare against single-byte string. */
  lhs_str = (uint8_t *)lhs->s_str;
  lhs_len = lhs->s_len;
  if (lhs_end > lhs_len)
      lhs_end = lhs_len;
  if (lhs_start >= lhs_end)
      lhs_len = 0;
  else {
   lhs_str += lhs_start;
   lhs_len  = lhs_end - lhs_start;
  }
  if (!rhs->s_data ||
       rhs->s_data->u_width == STRING_WIDTH_1BYTE) {
   int result;
   rhs_len = rhs->s_len;
   if (rhs_end > rhs_len)
       rhs_end = rhs_len;
   if (rhs_start >= rhs_end)
       rhs_len = 0;
   else {
    rhs_len  = rhs_end - rhs_start;
   }
   /* Most simple case: compare ascii/single-byte strings. */
   result = memcasecmp(lhs_str,rhs->s_str + rhs_start,MIN(lhs_len,rhs_len));
   if (result != 0) return result;
  } else {
   struct string_utf *rhs_utf = rhs->s_data;
   switch (rhs_utf->u_width) {

   {
    uint16_t *rhs_str;
    size_t i,common_len;
   CASE_WIDTH_2BYTE:
    rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
    rhs_len = WSTR_LENGTH(rhs_str);
    if (rhs_end > rhs_len)
        rhs_end = rhs_len;
    if (rhs_start >= rhs_end)
        rhs_len = 0;
    else {
     rhs_str += rhs_start;
     rhs_len  = rhs_end - rhs_start;
    }
    common_len = MIN(lhs_len,rhs_len);
    for (i = 0; i < common_len; ++i) {
     uint16_t a,b;
     if ((uint16_t)(lhs_str[i]) == rhs_str[i])
         continue;
     a = (uint16_t)DeeUni_ToLower(lhs_str[i]);
     b = (uint16_t)DeeUni_ToLower(rhs_str[i]);
     if (a == b)
         continue;
     return a < b ? -1 : 1;
    }
   } break;

   {
    uint32_t *rhs_str;
    size_t i,common_len;
   CASE_WIDTH_4BYTE:
    rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
    rhs_len = WSTR_LENGTH(rhs_str);
    if (rhs_end > rhs_len)
        rhs_end = rhs_len;
    if (rhs_start >= rhs_end)
        rhs_len = 0;
    else {
     rhs_str += rhs_start;
     rhs_len  = rhs_end - rhs_start;
    }
    common_len = MIN(lhs_len,rhs_len);
    for (i = 0; i < common_len; ++i) {
     uint32_t a,b;
     if ((uint32_t)(lhs_str[i]) == rhs_str[i])
         continue;
     a = (uint32_t)DeeUni_ToLower(lhs_str[i]);
     b = (uint32_t)DeeUni_ToLower(rhs_str[i]);
     if (a == b)
         continue;
     return a < b ? -1 : 1;
    }
   } break;

   default: __builtin_unreachable();
   }
  }
 } else if (!rhs->s_data ||
             rhs->s_data->u_width == STRING_WIDTH_1BYTE) {
  uint8_t *rhs_str;
  struct string_utf *lhs_utf;
  /* Compare against single-byte string. */
  rhs_str = (uint8_t *)rhs->s_str;
  rhs_len = rhs->s_len;
  if (rhs_end > rhs_len)
      rhs_end = rhs_len;
  if (rhs_start >= rhs_end)
      rhs_len = 0;
  else {
   rhs_str += rhs_start;
   rhs_len  = rhs_end - rhs_start;
  }
  lhs_utf = lhs->s_data;
  switch (lhs_utf->u_width) {

  {
   uint16_t *lhs_str;
   size_t i,common_len;
  CASE_WIDTH_2BYTE:
   lhs_str = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
   lhs_len = WSTR_LENGTH(lhs_str);
   if (lhs_end > lhs_len)
       lhs_end = lhs_len;
   if (lhs_start >= lhs_end)
       lhs_len = 0;
   else {
    lhs_str += lhs_start;
    lhs_len  = lhs_end - lhs_start;
   }
   common_len = MIN(rhs_len,lhs_len);
   for (i = 0; i < common_len; ++i) {
    uint16_t a,b;
    if (lhs_str[i] == (uint16_t)(rhs_str[i]))
        continue;
    a = (uint16_t)DeeUni_ToLower(lhs_str[i]);
    b = (uint16_t)DeeUni_ToLower(rhs_str[i]);
    if (a == b)
        continue;
    return a < b ? -1 : 1;
   }
  } break;

  {
   uint32_t *lhs_str;
   size_t i,common_len;
  CASE_WIDTH_4BYTE:
   lhs_str = (uint32_t *)lhs_utf->u_data[lhs_utf->u_width];
   lhs_len = WSTR_LENGTH(lhs_str);
   if (lhs_end > lhs_len)
       lhs_end = lhs_len;
   if (lhs_start >= lhs_end)
       lhs_len = 0;
   else {
    lhs_str += lhs_start;
    lhs_len  = lhs_end - lhs_start;
   }
   common_len = MIN(rhs_len,lhs_len);
   for (i = 0; i < common_len; ++i) {
    uint32_t a,b;
    if (lhs_str[i] == (uint32_t)(rhs_str[i]))
        continue;
    a = (uint32_t)DeeUni_ToLower(lhs_str[i]);
    b = (uint32_t)DeeUni_ToLower(rhs_str[i]);
    if (a == b)
        continue;
    return a < b ? -1 : 1;
   }
  } break;

  default: __builtin_unreachable();
  }
 } else {
  struct string_utf *lhs_utf;
  struct string_utf *rhs_utf;
  lhs_utf = lhs->s_data;
  rhs_utf = rhs->s_data;
  ASSERT(lhs_utf);
  ASSERT(rhs_utf);
  ASSERT(lhs_utf->u_width != STRING_WIDTH_1BYTE);
  ASSERT(rhs_utf->u_width != STRING_WIDTH_1BYTE);
  /* Complex string comparison. */
  switch (lhs_utf->u_width) {

  {
   uint16_t *lhs_str;
  CASE_WIDTH_2BYTE:
   lhs_str = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
   lhs_len = WSTR_LENGTH(lhs_str);
   if (lhs_end > lhs_len)
       lhs_end = lhs_len;
   if (lhs_start >= lhs_end)
       lhs_len = 0;
   else {
    lhs_str += lhs_start;
    lhs_len  = lhs_end - lhs_start;
   }
   switch (rhs_utf->u_width) {

   {
    uint16_t *rhs_str;
    size_t common_len;
   CASE_WIDTH_2BYTE:
    rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
    rhs_len = WSTR_LENGTH(rhs_str);
    if (rhs_end > rhs_len)
        rhs_end = rhs_len;
    if (rhs_start >= rhs_end)
        rhs_len = 0;
    else {
     rhs_str += rhs_start;
     rhs_len  = rhs_end - rhs_start;
    }
    common_len = MIN(lhs_len,rhs_len);
    {
     size_t i;
     for (i = 0; i < common_len; ++i) {
      uint16_t a,b;
      if (lhs_str[i] == rhs_str[i])
          continue;
      a = (uint16_t)DeeUni_ToLower(lhs_str[i]);
      b = (uint16_t)DeeUni_ToLower(rhs_str[i]);
      if (a == b)
          continue;
      return a < b ? -1 : 1;
     }
    }
   } break;

   {
    uint32_t *rhs_str;
    size_t i,common_len;
   CASE_WIDTH_4BYTE:
    rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
    rhs_len = WSTR_LENGTH(rhs_str);
    if (rhs_end > rhs_len)
        rhs_end = rhs_len;
    if (rhs_start >= rhs_end)
        rhs_len = 0;
    else {
     rhs_str += rhs_start;
     rhs_len  = rhs_end - rhs_start;
    }
    common_len = MIN(lhs_len,rhs_len);
    for (i = 0; i < common_len; ++i) {
     uint32_t a,b;
     if ((uint32_t)(lhs_str[i]) == rhs_str[i])
         continue;
     a = (uint32_t)DeeUni_ToLower(lhs_str[i]);
     b = (uint32_t)DeeUni_ToLower(rhs_str[i]);
     if (a == b)
         continue;
     return a < b ? -1 : 1;
    }
   } break;

   default: __builtin_unreachable();
   }
  } break;

  {
   uint32_t *lhs_str;
  CASE_WIDTH_4BYTE:
   lhs_str = (uint32_t *)lhs_utf->u_data[lhs_utf->u_width];
   lhs_len = WSTR_LENGTH(lhs_str);
   if (lhs_end > lhs_len)
       lhs_end = lhs_len;
   if (lhs_start >= lhs_end)
       lhs_len = 0;
   else {
    lhs_str += lhs_start;
    lhs_len  = lhs_end - lhs_start;
   }
   switch (rhs_utf->u_width) {

   {
    uint16_t *rhs_str;
    size_t i,common_len;
   CASE_WIDTH_2BYTE:
    rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
    rhs_len = WSTR_LENGTH(rhs_str);
    if (rhs_end > rhs_len)
        rhs_end = rhs_len;
    if (rhs_start >= rhs_end)
        rhs_len = 0;
    else {
     rhs_str += rhs_start;
     rhs_len  = rhs_end - rhs_start;
    }
    common_len = MIN(lhs_len,rhs_len);
    for (i = 0; i < common_len; ++i) {
     uint32_t a,b;
     if (lhs_str[i] == (uint32_t)(rhs_str[i]))
         continue;
     a = (uint32_t)DeeUni_ToLower(lhs_str[i]);
     b = (uint32_t)DeeUni_ToLower(rhs_str[i]);
     if (a == b)
         continue;
     return a < b ? -1 : 1;
    }
   } break;

   {
    uint32_t *rhs_str;
    size_t common_len;
   CASE_WIDTH_4BYTE:
    rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
    rhs_len = WSTR_LENGTH(rhs_str);
    if (rhs_end > rhs_len)
        rhs_end = rhs_len;
    if (rhs_start >= rhs_end)
        rhs_len = 0;
    else {
     rhs_str += rhs_start;
     rhs_len  = rhs_end - rhs_start;
    }
    common_len = MIN(lhs_len,rhs_len);
    {
     size_t i;
     for (i = 0; i < common_len; ++i) {
      uint32_t a,b;
      if (lhs_str[i] == rhs_str[i])
          continue;
      a = (uint32_t)DeeUni_ToLower(lhs_str[i]);
      b = (uint32_t)DeeUni_ToLower(rhs_str[i]);
      if (a == b)
          continue;
      return a < b ? -1 : 1;
     }
    }
   } break;

   default: __builtin_unreachable();
   }
  } break;

  default: __builtin_unreachable();
  }
 }

 /* If string contents are identical, leave off by comparing their lengths. */
 if (lhs_len == rhs_len)
     return 0;
 if (lhs_len < rhs_len)
     return -1;
 return 1;
}



PRIVATE DREF DeeObject *DCALL
string_compare(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 int result; struct compare_args args;
 if (get_compare_args(&args,argc,argv,"compare"))
     goto err;
 result = compare_strings_ex(self,args.my_start,args.my_end,
                            (String *)args.other,args.ot_start,args.ot_end);
 return DeeInt_NewInt(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casecompare(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 int result; struct compare_args args;
 if (get_compare_args(&args,argc,argv,"compare"))
     goto err;
 result = casecompare_strings_ex(self,args.my_start,args.my_end,
                                (String *)args.other,args.ot_start,args.ot_end);
 return DeeInt_NewInt(result);
err:
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_vercompare(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 union dcharptr my_str,ot_str;
 size_t my_len,ot_len; int32_t result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"vercompare"))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(args.other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (args.my_end  > my_len)
      args.my_end  = my_len;
  if (args.my_end <= args.my_start) my_len = 0;
  else my_str.cp8 += args.my_start,
       my_len      = args.my_end - args.my_start;
  if (args.ot_end  > ot_len)
      args.ot_end  = ot_len;
  if (args.ot_end <= args.ot_start) ot_len = 0;
  else ot_str.cp8 += args.ot_start,
       ot_len      = args.ot_end - args.ot_start;
  result = dee_strverscmpb(my_str.cp8,my_len,
                           ot_str.cp8,ot_len);
  break;
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp16) goto err;
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp16 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp16 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = dee_strverscmpw(my_str.cp16,my_len,
                           ot_str.cp16,ot_len);
  break;
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp32) goto err;
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp32 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp32 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = dee_strverscmpl(my_str.cp32,my_len,
                           ot_str.cp32,ot_len);
  break;
 }
 return DeeInt_NewS32(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
string_casevercompare(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 union dcharptr my_str,ot_str;
 size_t my_len,ot_len; int32_t result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"casevercompare"))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(args.other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (args.my_end  > my_len)
      args.my_end  = my_len;
  if (args.my_end <= args.my_start) my_len = 0;
  else my_str.cp8 += args.my_start,
       my_len      = args.my_end - args.my_start;
  if (args.ot_end  > ot_len)
      args.ot_end  = ot_len;
  if (args.ot_end <= args.ot_start) ot_len = 0;
  else ot_str.cp8 += args.ot_start,
       ot_len      = args.ot_end - args.ot_start;
  result = dee_strcaseverscmpb(my_str.cp8,my_len,
                               ot_str.cp8,ot_len);
  break;
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp16) goto err;
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp16 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp16 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = dee_strcaseverscmpw(my_str.cp16,my_len,
                               ot_str.cp16,ot_len);
  break;
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp32) goto err;
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp32 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp32 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = dee_strcaseverscmpl(my_str.cp32,my_len,
                               ot_str.cp32,ot_len);
  break;
 }
 return DeeInt_NewS32(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_fuzzycompare(String *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 union dcharptr my_str,ot_str;
 size_t my_len,ot_len; dssize_t result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"fuzzycompare"))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(args.other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (args.my_end  > my_len)
      args.my_end  = my_len;
  if (args.my_end <= args.my_start) my_len = 0;
  else my_str.cp8 += args.my_start,
       my_len      = args.my_end - args.my_start;
  if (args.ot_end  > ot_len)
      args.ot_end  = ot_len;
  if (args.ot_end <= args.ot_start) ot_len = 0;
  else ot_str.cp8 += args.ot_start,
       ot_len      = args.ot_end - args.ot_start;
  result = fuzzy_compareb(my_str.cp8,my_len,
                          ot_str.cp8,ot_len);
  break;
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp16) goto err;
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp16 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp16 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = fuzzy_comparew(my_str.cp16,my_len,
                          ot_str.cp16,ot_len);
  break;
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp32) goto err;
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp32 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp32 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = fuzzy_comparel(my_str.cp32,my_len,
                          ot_str.cp32,ot_len);
  break;
 }
 if unlikely(result == (dssize_t)-1) goto err;
 return DeeInt_NewSize((size_t)result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casefuzzycompare(String *__restrict self,
                        size_t argc, DeeObject **__restrict argv) {
 union dcharptr my_str,ot_str;
 size_t my_len,ot_len; dssize_t result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"casefuzzycompare"))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(args.other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (args.my_end  > my_len)
      args.my_end  = my_len;
  if (args.my_end <= args.my_start) my_len = 0;
  else my_str.cp8 += args.my_start,
       my_len      = args.my_end - args.my_start;
  if (args.ot_end  > ot_len)
      args.ot_end  = ot_len;
  if (args.ot_end <= args.ot_start) ot_len = 0;
  else ot_str.cp8 += args.ot_start,
       ot_len      = args.ot_end - args.ot_start;
  result = fuzzy_casecompareb(my_str.cp8,my_len,
                              ot_str.cp8,ot_len);
  break;
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp16) goto err;
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp16 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp16 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = fuzzy_casecomparew(my_str.cp16,my_len,
                              ot_str.cp16,ot_len);
  break;
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp32) goto err;
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp32 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp32 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = fuzzy_casecomparel(my_str.cp32,my_len,
                              ot_str.cp32,ot_len);
  break;
 }
 if unlikely(result == (dssize_t)-1) goto err;
 return DeeInt_NewSize((size_t)result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_common(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 union dcharptr my_str,ot_str;
 size_t my_len,ot_len,result = 0;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"common"))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(args.other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (args.my_end  > my_len)
      args.my_end  = my_len;
  if (args.my_end <= args.my_start) my_len = 0;
  else my_str.cp8 += args.my_start,
       my_len      = args.my_end - args.my_start;
  if (args.ot_end  > ot_len)
      args.ot_end  = ot_len;
  if (args.ot_end <= args.ot_start) ot_len = 0;
  else ot_str.cp8 += args.ot_start,
       ot_len      = args.ot_end - args.ot_start;
  while (ot_len && my_len) {
   uint8_t a = *my_str.cp8;
   uint8_t b = *ot_str.cp8;
   if (a != b) break;
   ++my_str.cp8;
   ++ot_str.cp8;
   --my_len;
   --ot_len;
   ++result;
  }
  break;
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp16) goto err;
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp16 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp16 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  while (ot_len && my_len) {
   uint16_t a = *my_str.cp16;
   uint16_t b = *ot_str.cp16;
   if (a != b) break;
   ++my_str.cp16;
   ++ot_str.cp16;
   --my_len;
   --ot_len;
   ++result;
  }
  break;
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp32) goto err;
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp32 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp32 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  while (ot_len && my_len) {
   uint32_t a = *my_str.cp32;
   uint32_t b = *ot_str.cp32;
   if (a != b) break;
   ++my_str.cp32;
   ++ot_str.cp32;
   --my_len;
   --ot_len;
   ++result;
  }
  break;
 }
 return DeeInt_NewSize(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rcommon(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 union dcharptr my_str,ot_str;
 size_t my_len,ot_len,result = 0;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"rcommon"))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(args.other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (args.my_end  > my_len)
      args.my_end  = my_len;
  if (args.my_end <= args.my_start) my_len = 0;
  else my_str.cp8 += args.my_start,
       my_len      = args.my_end - args.my_start;
  if (args.ot_end  > ot_len)
      args.ot_end  = ot_len;
  if (args.ot_end <= args.ot_start) ot_len = 0;
  else ot_str.cp8 += args.ot_start,
       ot_len      = args.ot_end - args.ot_start;
  while (ot_len && my_len) {
   uint8_t a;
   uint8_t b;
   --my_len;
   --ot_len;
   a = my_str.cp8[my_len];
   b = ot_str.cp8[ot_len];
   if (a != b) break;
   ++my_str.cp8;
   ++ot_str.cp8;
   ++result;
  }
  break;
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp16) goto err;
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp16 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp16 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  while (ot_len && my_len) {
   uint16_t a;
   uint16_t b;
   --my_len;
   --ot_len;
   a = my_str.cp16[my_len];
   b = ot_str.cp16[ot_len];
   if (a != b) break;
   ++my_str.cp16;
   ++ot_str.cp16;
   ++result;
  }
  break;
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp32) goto err;
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp32 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp32 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  while (ot_len && my_len) {
   uint32_t a;
   uint32_t b;
   --my_len;
   --ot_len;
   a = my_str.cp32[my_len];
   b = ot_str.cp32[ot_len];
   if (a != b) break;
   ++my_str.cp32;
   ++ot_str.cp32;
   ++result;
  }
  break;
 }
 return DeeInt_NewSize(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casecommon(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 union dcharptr my_str,ot_str;
 size_t my_len,ot_len,result = 0;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"casecommon"))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(args.other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (args.my_end  > my_len)
      args.my_end  = my_len;
  if (args.my_end <= args.my_start) my_len = 0;
  else my_str.cp8 += args.my_start,
       my_len      = args.my_end - args.my_start;
  if (args.ot_end  > ot_len)
      args.ot_end  = ot_len;
  if (args.ot_end <= args.ot_start) ot_len = 0;
  else ot_str.cp8 += args.ot_start,
       ot_len      = args.ot_end - args.ot_start;
  while (ot_len && my_len) {
   uint8_t a = *my_str.cp8;
   uint8_t b = *ot_str.cp8;
   if (a != b) {
    a = (uint8_t)DeeUni_ToLower(a);
    b = (uint8_t)DeeUni_ToLower(b);
    if (a != b) break;
   }
   ++my_str.cp8;
   ++ot_str.cp8;
   --my_len;
   --ot_len;
   ++result;
  }
  break;
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp16) goto err;
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp16 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp16 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  while (ot_len && my_len) {
   uint16_t a = *my_str.cp16;
   uint16_t b = *ot_str.cp16;
   if (a != b) {
    a = (uint16_t)DeeUni_ToLower(a);
    b = (uint16_t)DeeUni_ToLower(b);
    if (a != b) break;
   }
   ++my_str.cp16;
   ++ot_str.cp16;
   --my_len;
   --ot_len;
   ++result;
  }
  break;
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp32) goto err;
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp32 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp32 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  while (ot_len && my_len) {
   uint32_t a = *my_str.cp32;
   uint32_t b = *ot_str.cp32;
   if (a != b) {
    a = (uint32_t)DeeUni_ToLower(a);
    b = (uint32_t)DeeUni_ToLower(b);
    if (a != b) break;
   }
   ++my_str.cp32;
   ++ot_str.cp32;
   --my_len;
   --ot_len;
   ++result;
  }
  break;
 }
 return DeeInt_NewSize(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casercommon(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 union dcharptr my_str,ot_str;
 size_t my_len,ot_len,result = 0;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"rcommon"))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(args.other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (args.my_end  > my_len)
      args.my_end  = my_len;
  if (args.my_end <= args.my_start) my_len = 0;
  else my_str.cp8 += args.my_start,
       my_len      = args.my_end - args.my_start;
  if (args.ot_end  > ot_len)
      args.ot_end  = ot_len;
  if (args.ot_end <= args.ot_start) ot_len = 0;
  else ot_str.cp8 += args.ot_start,
       ot_len      = args.ot_end - args.ot_start;
  while (ot_len && my_len) {
   uint8_t a;
   uint8_t b;
   --my_len;
   --ot_len;
   a = my_str.cp8[my_len];
   b = ot_str.cp8[ot_len];
   if (a != b) {
    a = (uint8_t)DeeUni_ToLower(a);
    b = (uint8_t)DeeUni_ToLower(b);
    if (a != b) break;
   }
   ++my_str.cp8;
   ++ot_str.cp8;
   ++result;
  }
  break;
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp16) goto err;
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp16 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp16 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  while (ot_len && my_len) {
   uint16_t a;
   uint16_t b;
   --my_len;
   --ot_len;
   a = my_str.cp16[my_len];
   b = ot_str.cp16[ot_len];
   if (a != b) {
    a = (uint16_t)DeeUni_ToLower(a);
    b = (uint16_t)DeeUni_ToLower(b);
    if (a != b) break;
   }
   ++my_str.cp16;
   ++ot_str.cp16;
   ++result;
  }
  break;
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp32) goto err;
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp32 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp32 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  while (ot_len && my_len) {
   uint32_t a;
   uint32_t b;
   --my_len;
   --ot_len;
   a = my_str.cp32[my_len];
   b = ot_str.cp32[ot_len];
   if (a != b) {
    a = (uint32_t)DeeUni_ToLower(a);
    b = (uint32_t)DeeUni_ToLower(b);
    if (a != b) break;
   }
   ++my_str.cp32;
   ++ot_str.cp32;
   ++result;
  }
  break;
 }
 return DeeInt_NewSize(result);
err:
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_wildcompare(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 union dcharptr my_str,ot_str;
 size_t my_len,ot_len; int64_t result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"wildcompare"))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(args.other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (args.my_end  > my_len)
      args.my_end  = my_len;
  if (args.my_end <= args.my_start) my_len = 0;
  else my_str.cp8 += args.my_start,
       my_len      = args.my_end - args.my_start;
  if (args.ot_end  > ot_len)
      args.ot_end  = ot_len;
  if (args.ot_end <= args.ot_start) ot_len = 0;
  else ot_str.cp8 += args.ot_start,
       ot_len      = args.ot_end - args.ot_start;
  result = wildcompareb(my_str.cp8,my_len,
                        ot_str.cp8,ot_len);
  break;
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp16) goto err;
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp16 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp16 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = wildcomparew(my_str.cp16,my_len,
                        ot_str.cp16,ot_len);
  break;
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp32) goto err;
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp32 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp32 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = wildcomparel(my_str.cp32,my_len,
                        ot_str.cp32,ot_len);
  break;
 }
 return DeeInt_NewS64(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_wmatch(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 union dcharptr my_str,ot_str;
 size_t my_len,ot_len; bool result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"wmatch"))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(args.other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (args.my_end  > my_len)
      args.my_end  = my_len;
  if (args.my_end <= args.my_start) my_len = 0;
  else my_str.cp8 += args.my_start,
       my_len      = args.my_end - args.my_start;
  if (args.ot_end  > ot_len)
      args.ot_end  = ot_len;
  if (args.ot_end <= args.ot_start) ot_len = 0;
  else ot_str.cp8 += args.ot_start,
       ot_len      = args.ot_end - args.ot_start;
  result = wildcompareb(my_str.cp8,my_len,
                        ot_str.cp8,ot_len) == 0;
  break;
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp16) goto err;
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp16 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp16 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = wildcomparew(my_str.cp16,my_len,
                        ot_str.cp16,ot_len) == 0;
  break;
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp32) goto err;
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp32 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp32 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = wildcomparel(my_str.cp32,my_len,
                        ot_str.cp32,ot_len) == 0;
  break;
 }
 return_bool_(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casewildcompare(String *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
 union dcharptr my_str,ot_str;
 size_t my_len,ot_len; int64_t result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"casewildcompare"))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(args.other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (args.my_end  > my_len)
      args.my_end  = my_len;
  if (args.my_end <= args.my_start) my_len = 0;
  else my_str.cp8 += args.my_start,
       my_len      = args.my_end - args.my_start;
  if (args.ot_end  > ot_len)
      args.ot_end  = ot_len;
  if (args.ot_end <= args.ot_start) ot_len = 0;
  else ot_str.cp8 += args.ot_start,
       ot_len      = args.ot_end - args.ot_start;
  result = wildcasecompareb(my_str.cp8,my_len,
                            ot_str.cp8,ot_len);
  break;
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp16) goto err;
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp16 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp16 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = wildcasecomparew(my_str.cp16,my_len,
                            ot_str.cp16,ot_len);
  break;
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp32) goto err;
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp32 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp32 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = wildcasecomparel(my_str.cp32,my_len,
                            ot_str.cp32,ot_len);
  break;
 }
 return DeeInt_NewS64(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casewmatch(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 union dcharptr my_str,ot_str;
 size_t my_len,ot_len; bool result;
 struct compare_args args;
 if (get_compare_args(&args,argc,argv,"casewmatch"))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
                                         DeeString_WIDTH(args.other))) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
  my_len     = WSTR_LENGTH(my_str.cp8);
  ot_len     = WSTR_LENGTH(ot_str.cp8);
  if (args.my_end  > my_len)
      args.my_end  = my_len;
  if (args.my_end <= args.my_start) my_len = 0;
  else my_str.cp8 += args.my_start,
       my_len      = args.my_end - args.my_start;
  if (args.ot_end  > ot_len)
      args.ot_end  = ot_len;
  if (args.ot_end <= args.ot_start) ot_len = 0;
  else ot_str.cp8 += args.ot_start,
       ot_len      = args.ot_end - args.ot_start;
  result = wildcasecompareb(my_str.cp8,my_len,
                            ot_str.cp8,ot_len) == 0;
  break;
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!my_str.cp16) goto err;
  ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp16) goto err;
  my_len      = WSTR_LENGTH(my_str.cp16);
  ot_len      = WSTR_LENGTH(ot_str.cp16);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp16 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp16 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = wildcasecomparew(my_str.cp16,my_len,
                            ot_str.cp16,ot_len) == 0;
  break;
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!my_str.cp32) goto err;
  ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
  if unlikely(!ot_str.cp32) goto err;
  my_len      = WSTR_LENGTH(my_str.cp32);
  ot_len      = WSTR_LENGTH(ot_str.cp32);
  if (args.my_end   > my_len)
      args.my_end   = my_len;
  if (args.my_end  <= args.my_start) my_len = 0;
  else my_str.cp32 += args.my_start,
       my_len       = args.my_end - args.my_start;
  if (args.ot_end   > ot_len)
      args.ot_end   = ot_len;
  if (args.ot_end  <= args.ot_start) ot_len = 0;
  else ot_str.cp32 += args.ot_start,
       ot_len       = args.ot_end - args.ot_start;
  result = wildcasecomparel(my_str.cp32,my_len,
                            ot_str.cp32,ot_len) == 0;
  break;
 }
 return_bool_(result);
err:
 return NULL;
}

INTDEF DREF DeeObject *DCALL
DeeString_Split(DeeObject *__restrict self, DeeObject *__restrict seperator);
INTDEF DREF DeeObject *DCALL
DeeString_CaseSplit(DeeObject *__restrict self, DeeObject *__restrict seperator);

PRIVATE DREF DeeObject *DCALL
string_split(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:split",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_Split((DeeObject *)self,other);
}
PRIVATE DREF DeeObject *DCALL
string_casesplit(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *other;
 if (DeeArg_Unpack(argc,argv,"o:casesplit",&other) ||
     DeeObject_AssertTypeExact(other,&DeeString_Type))
     return NULL;
 return DeeString_CaseSplit((DeeObject *)self,other);
}

INTDEF DREF DeeObject *DCALL
DeeString_SplitLines(DeeObject *__restrict self, bool keepends);

PRIVATE DREF DeeObject *DCALL
string_splitlines(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 bool keepends = false;
 if (DeeArg_Unpack(argc,argv,"|b:splitlines",&keepends))
     return NULL;
 return DeeString_SplitLines((DeeObject *)self,keepends);
}

PRIVATE DREF DeeObject *DCALL
string_indent(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeObject *filler = &str_tab;
 if (DeeArg_Unpack(argc,argv,"|o:indent",&filler) ||
     DeeObject_AssertTypeExact(filler,&DeeString_Type))
     return NULL;
 return DeeString_Indent((DeeObject *)self,filler);
}

PRIVATE DREF DeeObject *DCALL
string_dedent(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 size_t max_chars = 1; DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|Iuo:dedent",&max_chars,&mask))
     return NULL;
 return mask
      ? DeeString_Dedent((DeeObject *)self,max_chars,mask)
      : DeeString_DedentSpc((DeeObject *)self,max_chars);
}


INTDEF dssize_t DCALL
DeeString_Format(dformatprinter printer, void *arg,
                 /*utf-8*/char const *__restrict format,
                 size_t format_len, DeeObject *__restrict args);

PRIVATE DREF DeeObject *DCALL
string_format(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeObject *args; char *utf8_repr;
 if (DeeArg_Unpack(argc,argv,"o:format",&args))
     goto err;
 utf8_repr = DeeString_AsUtf8((DeeObject *)self);
 if unlikely(!utf8_repr) goto err;
 {
  struct unicode_printer printer = UNICODE_PRINTER_INIT;
  if unlikely(DeeString_Format((dformatprinter)&unicode_printer_print,
                               &printer,
                                utf8_repr,
                                WSTR_LENGTH(utf8_repr),
                                args) < 0)
     goto err_printer;
  return unicode_printer_pack(&printer);
err_printer:
  unicode_printer_fini(&printer);
 }
err:
 return NULL;
}

INTDEF DREF DeeObject *DCALL
DeeString_Scanf(DeeObject *__restrict self,
                DeeObject *__restrict format);
PRIVATE DREF DeeObject *DCALL
string_scanf(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 DeeObject *format;
 if (DeeArg_Unpack(argc,argv,"o:scanf",&format) ||
     DeeObject_AssertTypeExact(format,&DeeString_Type))
     return NULL;
 return DeeString_Scanf((DeeObject *)self,format);
}



PRIVATE DREF DeeObject *DCALL
string_findmatch(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 union dcharptr scan_str,open_str,clos_str,ptr;
 size_t scan_len,open_len,clos_len; size_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:findmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
                                          DeeString_WIDTH(s_open),
                                          DeeString_WIDTH(s_clos))) {
 CASE_WIDTH_1BYTE:
  scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
  clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
  open_len = WSTR_LENGTH(open_str.cp8);
  clos_len = WSTR_LENGTH(clos_str.cp8);
  if (end > WSTR_LENGTH(scan_str.cp8))
      end = WSTR_LENGTH(scan_str.cp8);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp8 = find_matchb(scan_str.cp8 + start,scan_len,
                        open_str.cp8,open_len,
                        clos_str.cp8,clos_len);
  if unlikely(!ptr.cp8) goto err_not_found;
  result = (size_t)(ptr.cp8 - scan_str.cp8);
  break;
 CASE_WIDTH_2BYTE:
  scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!scan_str.cp16) goto err;
  open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp16) goto err;
  clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp16) goto err;
  open_len = WSTR_LENGTH(open_str.cp16);
  clos_len = WSTR_LENGTH(clos_str.cp16);
  if (end > WSTR_LENGTH(scan_str.cp16))
      end = WSTR_LENGTH(scan_str.cp16);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp16 = find_matchw(scan_str.cp16 + start,scan_len,
                         open_str.cp16,open_len,
                         clos_str.cp16,clos_len);
  if unlikely(!ptr.cp16) goto err_not_found;
  result = (size_t)(ptr.cp16 - scan_str.cp16);
  break;
 CASE_WIDTH_4BYTE:
  scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!scan_str.cp32) goto err;
  open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp32) goto err;
  clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp32) goto err;
  open_len = WSTR_LENGTH(open_str.cp32);
  clos_len = WSTR_LENGTH(clos_str.cp32);
  if (end > WSTR_LENGTH(scan_str.cp32))
      end = WSTR_LENGTH(scan_str.cp32);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp32 = find_matchl(scan_str.cp32 + start,scan_len,
                         open_str.cp32,open_len,
                         clos_str.cp32,clos_len);
  if unlikely(!ptr.cp32) goto err_not_found;
  result = (size_t)(ptr.cp32 - scan_str.cp32);
  break;
 }
 return DeeInt_NewSize(result);
err_not_found:
 return_reference_(&DeeInt_MinusOne);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_indexmatch(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 union dcharptr scan_str,open_str,clos_str,ptr;
 size_t scan_len,open_len,clos_len; size_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:findmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
                                          DeeString_WIDTH(s_open),
                                          DeeString_WIDTH(s_clos))) {
 CASE_WIDTH_1BYTE:
  scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
  clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
  open_len = WSTR_LENGTH(open_str.cp8);
  clos_len = WSTR_LENGTH(clos_str.cp8);
  if (end > WSTR_LENGTH(scan_str.cp8))
      end = WSTR_LENGTH(scan_str.cp8);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp8 = find_matchb(scan_str.cp8 + start,scan_len,
                        open_str.cp8,open_len,
                        clos_str.cp8,clos_len);
  if unlikely(!ptr.cp8) goto err_not_found;
  result = (size_t)(ptr.cp8 - scan_str.cp8);
  break;
 CASE_WIDTH_2BYTE:
  scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!scan_str.cp16) goto err;
  open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp16) goto err;
  clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp16) goto err;
  open_len = WSTR_LENGTH(open_str.cp16);
  clos_len = WSTR_LENGTH(clos_str.cp16);
  if (end > WSTR_LENGTH(scan_str.cp16))
      end = WSTR_LENGTH(scan_str.cp16);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp16 = find_matchw(scan_str.cp16 + start,scan_len,
                         open_str.cp16,open_len,
                         clos_str.cp16,clos_len);
  if unlikely(!ptr.cp16) goto err_not_found;
  result = (size_t)(ptr.cp16 - scan_str.cp16);
  break;
 CASE_WIDTH_4BYTE:
  scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!scan_str.cp32) goto err;
  open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp32) goto err;
  clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp32) goto err;
  open_len = WSTR_LENGTH(open_str.cp32);
  clos_len = WSTR_LENGTH(clos_str.cp32);
  if (end > WSTR_LENGTH(scan_str.cp32))
      end = WSTR_LENGTH(scan_str.cp32);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp32 = find_matchl(scan_str.cp32 + start,scan_len,
                         open_str.cp32,open_len,
                         clos_str.cp32,clos_len);
  if unlikely(!ptr.cp32) goto err_not_found;
  result = (size_t)(ptr.cp32 - scan_str.cp32);
  break;
 }
 return DeeInt_NewSize(result);
err_not_found:
 err_index_not_found((DeeObject *)self,
                     (DeeObject *)s_clos);
err:
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_casefindmatch(String *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 union dcharptr scan_str,open_str,clos_str,ptr;
 size_t scan_len,open_len,clos_len; size_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:casefindmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
                                          DeeString_WIDTH(s_open),
                                          DeeString_WIDTH(s_clos))) {
 CASE_WIDTH_1BYTE:
  scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
  clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
  open_len = WSTR_LENGTH(open_str.cp8);
  clos_len = WSTR_LENGTH(clos_str.cp8);
  if (end > WSTR_LENGTH(scan_str.cp8))
      end = WSTR_LENGTH(scan_str.cp8);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp8 = find_casematchb(scan_str.cp8 + start,scan_len,
                            open_str.cp8,open_len,
                            clos_str.cp8,clos_len);
  if unlikely(!ptr.cp8) goto err_not_found;
  result = (size_t)(ptr.cp8 - scan_str.cp8);
  break;
 CASE_WIDTH_2BYTE:
  scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!scan_str.cp16) goto err;
  open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp16) goto err;
  clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp16) goto err;
  open_len = WSTR_LENGTH(open_str.cp16);
  clos_len = WSTR_LENGTH(clos_str.cp16);
  if (end > WSTR_LENGTH(scan_str.cp16))
      end = WSTR_LENGTH(scan_str.cp16);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp16 = find_casematchw(scan_str.cp16 + start,scan_len,
                             open_str.cp16,open_len,
                             clos_str.cp16,clos_len);
  if unlikely(!ptr.cp16) goto err_not_found;
  result = (size_t)(ptr.cp16 - scan_str.cp16);
  break;
 CASE_WIDTH_4BYTE:
  scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!scan_str.cp32) goto err;
  open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp32) goto err;
  clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp32) goto err;
  open_len = WSTR_LENGTH(open_str.cp32);
  clos_len = WSTR_LENGTH(clos_str.cp32);
  if (end > WSTR_LENGTH(scan_str.cp32))
      end = WSTR_LENGTH(scan_str.cp32);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp32 = find_casematchl(scan_str.cp32 + start,scan_len,
                             open_str.cp32,open_len,
                             clos_str.cp32,clos_len);
  if unlikely(!ptr.cp32) goto err_not_found;
  result = (size_t)(ptr.cp32 - scan_str.cp32);
  break;
 }
 return DeeInt_NewSize(result);
err_not_found:
 return_reference_(&DeeInt_MinusOne);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caseindexmatch(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 union dcharptr scan_str,open_str,clos_str,ptr;
 size_t scan_len,open_len,clos_len; size_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:caseindexmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
                                          DeeString_WIDTH(s_open),
                                          DeeString_WIDTH(s_clos))) {
 CASE_WIDTH_1BYTE:
  scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
  clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
  open_len = WSTR_LENGTH(open_str.cp8);
  clos_len = WSTR_LENGTH(clos_str.cp8);
  if (end > WSTR_LENGTH(scan_str.cp8))
      end = WSTR_LENGTH(scan_str.cp8);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp8 = find_casematchb(scan_str.cp8 + start,scan_len,
                            open_str.cp8,open_len,
                            clos_str.cp8,clos_len);
  if unlikely(!ptr.cp8) goto err_not_found;
  result = (size_t)(ptr.cp8 - scan_str.cp8);
  break;
 CASE_WIDTH_2BYTE:
  scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!scan_str.cp16) goto err;
  open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp16) goto err;
  clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp16) goto err;
  open_len = WSTR_LENGTH(open_str.cp16);
  clos_len = WSTR_LENGTH(clos_str.cp16);
  if (end > WSTR_LENGTH(scan_str.cp16))
      end = WSTR_LENGTH(scan_str.cp16);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp16 = find_casematchw(scan_str.cp16 + start,scan_len,
                             open_str.cp16,open_len,
                             clos_str.cp16,clos_len);
  if unlikely(!ptr.cp16) goto err_not_found;
  result = (size_t)(ptr.cp16 - scan_str.cp16);
  break;
 CASE_WIDTH_4BYTE:
  scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!scan_str.cp32) goto err;
  open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp32) goto err;
  clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp32) goto err;
  open_len = WSTR_LENGTH(open_str.cp32);
  clos_len = WSTR_LENGTH(clos_str.cp32);
  if (end > WSTR_LENGTH(scan_str.cp32))
      end = WSTR_LENGTH(scan_str.cp32);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp32 = find_casematchl(scan_str.cp32 + start,scan_len,
                             open_str.cp32,open_len,
                             clos_str.cp32,clos_len);
  if unlikely(!ptr.cp32) goto err_not_found;
  result = (size_t)(ptr.cp32 - scan_str.cp32);
  break;
 }
 return DeeInt_NewSize(result);
err_not_found:
 err_index_not_found((DeeObject *)self,
                     (DeeObject *)s_clos);
err:
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_rfindmatch(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 union dcharptr scan_str,open_str,clos_str,ptr;
 size_t scan_len,open_len,clos_len; size_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:rfindmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
                                          DeeString_WIDTH(s_open),
                                          DeeString_WIDTH(s_clos))) {
 CASE_WIDTH_1BYTE:
  scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
  clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
  open_len = WSTR_LENGTH(open_str.cp8);
  clos_len = WSTR_LENGTH(clos_str.cp8);
  if (end > WSTR_LENGTH(scan_str.cp8))
      end = WSTR_LENGTH(scan_str.cp8);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp8 = rfind_matchb(scan_str.cp8 + start,scan_len,
                         open_str.cp8,open_len,
                         clos_str.cp8,clos_len);
  if unlikely(!ptr.cp8) goto err_not_found;
  result = (size_t)(ptr.cp8 - scan_str.cp8);
  break;
 CASE_WIDTH_2BYTE:
  scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!scan_str.cp16) goto err;
  open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp16) goto err;
  clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp16) goto err;
  open_len = WSTR_LENGTH(open_str.cp16);
  clos_len = WSTR_LENGTH(clos_str.cp16);
  if (end > WSTR_LENGTH(scan_str.cp16))
      end = WSTR_LENGTH(scan_str.cp16);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp16 = rfind_matchw(scan_str.cp16 + start,scan_len,
                          open_str.cp16,open_len,
                          clos_str.cp16,clos_len);
  if unlikely(!ptr.cp16) goto err_not_found;
  result = (size_t)(ptr.cp16 - scan_str.cp16);
  break;
 CASE_WIDTH_4BYTE:
  scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!scan_str.cp32) goto err;
  open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp32) goto err;
  clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp32) goto err;
  open_len = WSTR_LENGTH(open_str.cp32);
  clos_len = WSTR_LENGTH(clos_str.cp32);
  if (end > WSTR_LENGTH(scan_str.cp32))
      end = WSTR_LENGTH(scan_str.cp32);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp32 = rfind_matchl(scan_str.cp32 + start,scan_len,
                          open_str.cp32,open_len,
                          clos_str.cp32,clos_len);
  if unlikely(!ptr.cp32) goto err_not_found;
  result = (size_t)(ptr.cp32 - scan_str.cp32);
  break;
 }
 return DeeInt_NewSize(result);
err_not_found:
 return_reference_(&DeeInt_MinusOne);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rindexmatch(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 union dcharptr scan_str,open_str,clos_str,ptr;
 size_t scan_len,open_len,clos_len; size_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:rindexmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
                                          DeeString_WIDTH(s_open),
                                          DeeString_WIDTH(s_clos))) {
 CASE_WIDTH_1BYTE:
  scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
  clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
  open_len = WSTR_LENGTH(open_str.cp8);
  clos_len = WSTR_LENGTH(clos_str.cp8);
  if (end > WSTR_LENGTH(scan_str.cp8))
      end = WSTR_LENGTH(scan_str.cp8);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp8 = rfind_matchb(scan_str.cp8 + start,scan_len,
                         open_str.cp8,open_len,
                         clos_str.cp8,clos_len);
  if unlikely(!ptr.cp8) goto err_not_found;
  result = (size_t)(ptr.cp8 - scan_str.cp8);
  break;
 CASE_WIDTH_2BYTE:
  scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!scan_str.cp16) goto err;
  open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp16) goto err;
  clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp16) goto err;
  open_len = WSTR_LENGTH(open_str.cp16);
  clos_len = WSTR_LENGTH(clos_str.cp16);
  if (end > WSTR_LENGTH(scan_str.cp16))
      end = WSTR_LENGTH(scan_str.cp16);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp16 = rfind_matchw(scan_str.cp16 + start,scan_len,
                          open_str.cp16,open_len,
                          clos_str.cp16,clos_len);
  if unlikely(!ptr.cp16) goto err_not_found;
  result = (size_t)(ptr.cp16 - scan_str.cp16);
  break;
 CASE_WIDTH_4BYTE:
  scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!scan_str.cp32) goto err;
  open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp32) goto err;
  clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp32) goto err;
  open_len = WSTR_LENGTH(open_str.cp32);
  clos_len = WSTR_LENGTH(clos_str.cp32);
  if (end > WSTR_LENGTH(scan_str.cp32))
      end = WSTR_LENGTH(scan_str.cp32);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp32 = rfind_matchl(scan_str.cp32 + start,scan_len,
                          open_str.cp32,open_len,
                          clos_str.cp32,clos_len);
  if unlikely(!ptr.cp32) goto err_not_found;
  result = (size_t)(ptr.cp32 - scan_str.cp32);
  break;
 }
 return DeeInt_NewSize(result);
err_not_found:
 err_index_not_found((DeeObject *)self,
                     (DeeObject *)s_clos);
err:
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_caserfindmatch(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 union dcharptr scan_str,open_str,clos_str,ptr;
 size_t scan_len,open_len,clos_len; size_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:caserfindmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
                                          DeeString_WIDTH(s_open),
                                          DeeString_WIDTH(s_clos))) {
 CASE_WIDTH_1BYTE:
  scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
  clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
  open_len = WSTR_LENGTH(open_str.cp8);
  clos_len = WSTR_LENGTH(clos_str.cp8);
  if (end > WSTR_LENGTH(scan_str.cp8))
      end = WSTR_LENGTH(scan_str.cp8);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp8 = rfind_casematchb(scan_str.cp8 + start,scan_len,
                             open_str.cp8,open_len,
                             clos_str.cp8,clos_len);
  if unlikely(!ptr.cp8) goto err_not_found;
  result = (size_t)(ptr.cp8 - scan_str.cp8);
  break;
 CASE_WIDTH_2BYTE:
  scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!scan_str.cp16) goto err;
  open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp16) goto err;
  clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp16) goto err;
  open_len = WSTR_LENGTH(open_str.cp16);
  clos_len = WSTR_LENGTH(clos_str.cp16);
  if (end > WSTR_LENGTH(scan_str.cp16))
      end = WSTR_LENGTH(scan_str.cp16);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp16 = rfind_casematchw(scan_str.cp16 + start,scan_len,
                              open_str.cp16,open_len,
                              clos_str.cp16,clos_len);
  if unlikely(!ptr.cp16) goto err_not_found;
  result = (size_t)(ptr.cp16 - scan_str.cp16);
  break;
 CASE_WIDTH_4BYTE:
  scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!scan_str.cp32) goto err;
  open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp32) goto err;
  clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp32) goto err;
  open_len = WSTR_LENGTH(open_str.cp32);
  clos_len = WSTR_LENGTH(clos_str.cp32);
  if (end > WSTR_LENGTH(scan_str.cp32))
      end = WSTR_LENGTH(scan_str.cp32);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp32 = rfind_casematchl(scan_str.cp32 + start,scan_len,
                              open_str.cp32,open_len,
                              clos_str.cp32,clos_len);
  if unlikely(!ptr.cp32) goto err_not_found;
  result = (size_t)(ptr.cp32 - scan_str.cp32);
  break;
 }
 return DeeInt_NewSize(result);
err_not_found:
 return_reference_(&DeeInt_MinusOne);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caserindexmatch(String *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 union dcharptr scan_str,open_str,clos_str,ptr;
 size_t scan_len,open_len,clos_len; size_t result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:caserindexmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
                                          DeeString_WIDTH(s_open),
                                          DeeString_WIDTH(s_clos))) {
 CASE_WIDTH_1BYTE:
  scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
  clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
  open_len = WSTR_LENGTH(open_str.cp8);
  clos_len = WSTR_LENGTH(clos_str.cp8);
  if (end > WSTR_LENGTH(scan_str.cp8))
      end = WSTR_LENGTH(scan_str.cp8);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp8 = rfind_casematchb(scan_str.cp8 + start,scan_len,
                             open_str.cp8,open_len,
                             clos_str.cp8,clos_len);
  if unlikely(!ptr.cp8) goto err_not_found;
  result = (size_t)(ptr.cp8 - scan_str.cp8);
  break;
 CASE_WIDTH_2BYTE:
  scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!scan_str.cp16) goto err;
  open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp16) goto err;
  clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp16) goto err;
  open_len = WSTR_LENGTH(open_str.cp16);
  clos_len = WSTR_LENGTH(clos_str.cp16);
  if (end > WSTR_LENGTH(scan_str.cp16))
      end = WSTR_LENGTH(scan_str.cp16);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp16 = rfind_casematchw(scan_str.cp16 + start,scan_len,
                              open_str.cp16,open_len,
                              clos_str.cp16,clos_len);
  if unlikely(!ptr.cp16) goto err_not_found;
  result = (size_t)(ptr.cp16 - scan_str.cp16);
  break;
 CASE_WIDTH_4BYTE:
  scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!scan_str.cp32) goto err;
  open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp32) goto err;
  clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp32) goto err;
  open_len = WSTR_LENGTH(open_str.cp32);
  clos_len = WSTR_LENGTH(clos_str.cp32);
  if (end > WSTR_LENGTH(scan_str.cp32))
      end = WSTR_LENGTH(scan_str.cp32);
  if unlikely(end <= start) goto err_not_found; /* Empty search area. */
  scan_len = end - start;
  ptr.cp32 = rfind_casematchl(scan_str.cp32 + start,scan_len,
                              open_str.cp32,open_len,
                              clos_str.cp32,clos_len);
  if unlikely(!ptr.cp32) goto err_not_found;
  result = (size_t)(ptr.cp32 - scan_str.cp32);
  break;
 }
 return DeeInt_NewSize(result);
err_not_found:
 err_index_not_found((DeeObject *)self,
                     (DeeObject *)s_clos);
err:
 return NULL;
}



PRIVATE DREF DeeObject *DCALL
string_partitionmatch(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 union dcharptr scan_str,open_str,clos_str,match_start,match_end;
 size_t scan_len,open_len,clos_len; DREF DeeTupleObject *result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:partitionmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
#define SET_STRING(a,b,c) \
 do{ \
   if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) goto err_r_0; \
   if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) goto err_r_1; \
   if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) goto err_r_2; \
 }__WHILE0
 result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
                                          DeeString_WIDTH(s_open),
                                          DeeString_WIDTH(s_clos))) {
 CASE_WIDTH_1BYTE:
  scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
  clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
  open_len = WSTR_LENGTH(open_str.cp8);
  clos_len = WSTR_LENGTH(clos_str.cp8);
  if (end > WSTR_LENGTH(scan_str.cp8))
      end = WSTR_LENGTH(scan_str.cp8);
  if unlikely(end <= start)
     goto match_not_found; /* Empty search area. */
  scan_len = end - start;
  match_start.cp8 = memmemb(scan_str.cp8 + start,scan_len,
                            open_str.cp8,open_len);
  if unlikely(!match_start.cp8) goto match_not_found;
  match_end.cp8 = find_matchb(match_start.cp8 + open_len,scan_len -
                             (match_start.cp8 - (scan_str.cp8 + start)),
                              open_str.cp8,open_len,
                              clos_str.cp8,clos_len);
  if unlikely(!match_end.cp8) goto match_not_found;
  SET_STRING(DeeString_New1Byte(scan_str.cp8,
                               (size_t)(match_start.cp8-scan_str.cp8)),
             DeeString_New1Byte(match_start.cp8,
                               (match_end.cp8 + clos_len) -
                                match_start.cp8),
             DeeString_New1Byte(match_end.cp8 + clos_len,
                               (size_t)(scan_str.cp8 + end) -
                               (size_t)(match_end.cp8 + clos_len)));
  break;
 CASE_WIDTH_2BYTE:
  scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!scan_str.cp16) goto err_r_0;
  open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp16) goto err_r_0;
  clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp16) goto err_r_0;
  open_len = WSTR_LENGTH(open_str.cp16);
  clos_len = WSTR_LENGTH(clos_str.cp16);
  if (end > WSTR_LENGTH(scan_str.cp16))
      end = WSTR_LENGTH(scan_str.cp16);
  if unlikely(end <= start)
     goto match_not_found; /* Empty search area. */
  scan_len = end - start;
  match_start.cp16 = memmemw(scan_str.cp16 + start,scan_len,
                             open_str.cp16,open_len);
  if unlikely(!match_start.cp16) goto match_not_found;
  match_end.cp16 = find_matchw(match_start.cp16 + open_len,scan_len -
                              (match_start.cp16 - (scan_str.cp16 + start)),
                               open_str.cp16,open_len,
                               clos_str.cp16,clos_len);
  if unlikely(!match_end.cp16) goto match_not_found;
  SET_STRING(DeeString_New2Byte(scan_str.cp16,
                               (size_t)(match_start.cp16-scan_str.cp16)),
             DeeString_New2Byte(match_start.cp16,
                               (match_end.cp16 + clos_len) -
                                match_start.cp16),
             DeeString_New2Byte(match_end.cp16 + clos_len,
                               (size_t)(scan_str.cp16 + end) -
                               (size_t)(match_end.cp16 + clos_len)));
  break;
 CASE_WIDTH_4BYTE:
  scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!scan_str.cp32) goto err_r_0;
  open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp32) goto err_r_0;
  clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp32) goto err_r_0;
  open_len = WSTR_LENGTH(open_str.cp32);
  clos_len = WSTR_LENGTH(clos_str.cp32);
  if (end > WSTR_LENGTH(scan_str.cp32))
      end = WSTR_LENGTH(scan_str.cp32);
  if unlikely(end <= start)
     goto match_not_found; /* Empty search area. */
  scan_len = end - start;
  match_start.cp32 = memmeml(scan_str.cp32 + start,scan_len,
                             open_str.cp32,open_len);
  if unlikely(!match_start.cp32) goto match_not_found;
  match_end.cp32 = find_matchl(match_start.cp32 + open_len,scan_len -
                              (match_start.cp32 - (scan_str.cp32 + start)),
                               open_str.cp32,open_len,
                               clos_str.cp32,clos_len);
  if unlikely(!match_end.cp32) goto match_not_found;
  SET_STRING(DeeString_New4Byte(scan_str.cp32,
                               (size_t)(match_start.cp32-scan_str.cp32)),
             DeeString_New4Byte(match_start.cp32,
                               (match_end.cp32 + clos_len) -
                                match_start.cp32),
             DeeString_New4Byte(match_end.cp32 + clos_len,
                               (size_t)(scan_str.cp32 + end) -
                               (size_t)(match_end.cp32 + clos_len)));
  break;
 }
#undef SET_STRING
done:
 return (DREF DeeObject *)result;
match_not_found:
 result->t_elem[0] = (DREF DeeObject *)string_getsubstr(self,start,end);
 if unlikely(!result->t_elem[0]) goto err_r_0;
 result->t_elem[1] = Dee_EmptyString;
 result->t_elem[2] = Dee_EmptyString;
#ifdef CONFIG_NO_THREADS
 Dee_EmptyString->ob_refcnt += 2;
#else
 ATOMIC_FETCHADD(Dee_EmptyString->ob_refcnt,2);
#endif
 goto done;
err_r_2:
 Dee_DecrefDokill(result->t_elem[1]);
err_r_1:
 Dee_DecrefDokill(result->t_elem[0]);
err_r_0:
 DeeTuple_FreeUninitialized((DeeObject *)result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rpartitionmatch(String *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 union dcharptr scan_str,open_str,clos_str,match_start,match_end;
 size_t scan_len,open_len,clos_len; DREF DeeTupleObject *result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:rpartitionmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
#define SET_STRING(a,b,c) \
 do{ \
   if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) goto err_r_0; \
   if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) goto err_r_1; \
   if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) goto err_r_2; \
 }__WHILE0
 result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
                                          DeeString_WIDTH(s_open),
                                          DeeString_WIDTH(s_clos))) {
 CASE_WIDTH_1BYTE:
  scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
  clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
  open_len = WSTR_LENGTH(open_str.cp8);
  clos_len = WSTR_LENGTH(clos_str.cp8);
  if (end > WSTR_LENGTH(scan_str.cp8))
      end = WSTR_LENGTH(scan_str.cp8);
  if unlikely(end <= start)
     goto match_not_found; /* Empty search area. */
  scan_len = end - start;
  match_end.cp8 = memrmemb(scan_str.cp8 + start,scan_len,
                           clos_str.cp8,clos_len);
  if unlikely(!match_end.cp8) goto match_not_found;
  match_start.cp8 = rfind_matchb(scan_str.cp8 + start,
                                (size_t)(match_end.cp8 - (scan_str.cp8 + start)),
                                 open_str.cp8,open_len,
                                 clos_str.cp8,clos_len);
  if unlikely(!match_start.cp8) goto match_not_found;
  SET_STRING(DeeString_New1Byte(scan_str.cp8,
                               (size_t)(match_start.cp8 - scan_str.cp8)),
             DeeString_New1Byte(match_start.cp8,
                               (size_t)((match_end.cp8 + clos_len) -
                                         match_start.cp8)),
             DeeString_New1Byte(match_end.cp8 + clos_len,
                               (size_t)(scan_str.cp8 + end)-
                               (size_t)(match_end.cp8 + clos_len)));
  break;
 CASE_WIDTH_2BYTE:
  scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!scan_str.cp16) goto err_r_0;
  open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp16) goto err_r_0;
  clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp16) goto err_r_0;
  open_len = WSTR_LENGTH(open_str.cp16);
  clos_len = WSTR_LENGTH(clos_str.cp16);
  if (end > WSTR_LENGTH(scan_str.cp16))
      end = WSTR_LENGTH(scan_str.cp16);
  if unlikely(end <= start)
     goto match_not_found; /* Empty search area. */
  scan_len = end - start;
  match_end.cp16 = memrmemw(scan_str.cp16 + start,scan_len,
                            clos_str.cp16,clos_len);
  if unlikely(!match_end.cp16) goto match_not_found;
  match_start.cp16 = rfind_matchw(scan_str.cp16 + start,
                                 (size_t)(match_end.cp16 - (scan_str.cp16 + start)),
                                  open_str.cp16,open_len,
                                  clos_str.cp16,clos_len);
  if unlikely(!match_start.cp16) goto match_not_found;
  SET_STRING(DeeString_New2Byte(scan_str.cp16,
                               (size_t)(match_start.cp16 - scan_str.cp16)),
             DeeString_New2Byte(match_start.cp16,
                               (size_t)((match_end.cp16 + clos_len) -
                                         match_start.cp16)),
             DeeString_New2Byte(match_end.cp16 + clos_len,
                               (size_t)(scan_str.cp16 + end)-
                               (size_t)(match_end.cp16 + clos_len)));
  break;
 CASE_WIDTH_4BYTE:
  scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!scan_str.cp32) goto err_r_0;
  open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp32) goto err_r_0;
  clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp32) goto err_r_0;
  open_len = WSTR_LENGTH(open_str.cp32);
  clos_len = WSTR_LENGTH(clos_str.cp32);
  if (end > WSTR_LENGTH(scan_str.cp32))
      end = WSTR_LENGTH(scan_str.cp32);
  if unlikely(end <= start)
     goto match_not_found; /* Empty search area. */
  scan_len = end - start;
  match_end.cp32 = memrmeml(scan_str.cp32 + start,scan_len,
                            clos_str.cp32,clos_len);
  if unlikely(!match_end.cp32) goto match_not_found;
  match_start.cp32 = rfind_matchl(scan_str.cp32 + start,
                                 (size_t)(match_end.cp32 - (scan_str.cp32 + start)),
                                  open_str.cp32,open_len,
                                  clos_str.cp32,clos_len);
  if unlikely(!match_start.cp32) goto match_not_found;
  SET_STRING(DeeString_New4Byte(scan_str.cp32,
                               (size_t)(match_start.cp32 - scan_str.cp32)),
             DeeString_New4Byte(match_start.cp32,
                               (size_t)((match_end.cp32 + clos_len) -
                                         match_start.cp32)),
             DeeString_New4Byte(match_end.cp32 + clos_len,
                               (size_t)(scan_str.cp32 + end)-
                               (size_t)(match_end.cp32 + clos_len)));
  break;
 }
#undef SET_STRING
done:
 return (DREF DeeObject *)result;
match_not_found:
 result->t_elem[0] = (DREF DeeObject *)string_getsubstr(self,start,end);
 if unlikely(!result->t_elem[0]) goto err_r_0;
 result->t_elem[1] = Dee_EmptyString;
 result->t_elem[2] = Dee_EmptyString;
#ifdef CONFIG_NO_THREADS
 Dee_EmptyString->ob_refcnt += 2;
#else
 ATOMIC_FETCHADD(Dee_EmptyString->ob_refcnt,2);
#endif
 goto done;
err_r_2:
 Dee_DecrefDokill(result->t_elem[1]);
err_r_1:
 Dee_DecrefDokill(result->t_elem[0]);
err_r_0:
 DeeTuple_FreeUninitialized((DeeObject *)result);
err:
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_casepartitionmatch(String *__restrict self,
                          size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 union dcharptr scan_str,open_str,clos_str,match_start,match_end;
 size_t scan_len,open_len,clos_len; DREF DeeTupleObject *result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:casepartitionmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
#define SET_STRING(a,b,c) \
 do{ \
   if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) goto err_r_0; \
   if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) goto err_r_1; \
   if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) goto err_r_2; \
 }__WHILE0
 result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
                                          DeeString_WIDTH(s_open),
                                          DeeString_WIDTH(s_clos))) {
 CASE_WIDTH_1BYTE:
  scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
  clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
  open_len = WSTR_LENGTH(open_str.cp8);
  clos_len = WSTR_LENGTH(clos_str.cp8);
  if (end > WSTR_LENGTH(scan_str.cp8))
      end = WSTR_LENGTH(scan_str.cp8);
  if unlikely(end <= start)
     goto match_not_found; /* Empty search area. */
  scan_len = end - start;
  match_start.cp8 = memcasememb(scan_str.cp8 + start,scan_len,
                                open_str.cp8,open_len);
  if unlikely(!match_start.cp8) goto match_not_found;
  match_end.cp8 = find_casematchb(match_start.cp8 + open_len,scan_len -
                                 (match_start.cp8 - (scan_str.cp8 + start)),
                                  open_str.cp8,open_len,
                                  clos_str.cp8,clos_len);
  if unlikely(!match_end.cp8) goto match_not_found;
  SET_STRING(DeeString_New1Byte(scan_str.cp8,
                               (size_t)(match_start.cp8-scan_str.cp8)),
             DeeString_New1Byte(match_start.cp8,
                               (match_end.cp8 + clos_len) -
                                match_start.cp8),
             DeeString_New1Byte(match_end.cp8 + clos_len,
                               (size_t)(scan_str.cp8 + end) -
                               (size_t)(match_end.cp8 + clos_len)));
  break;
 CASE_WIDTH_2BYTE:
  scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!scan_str.cp16) goto err_r_0;
  open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp16) goto err_r_0;
  clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp16) goto err_r_0;
  open_len = WSTR_LENGTH(open_str.cp16);
  clos_len = WSTR_LENGTH(clos_str.cp16);
  if (end > WSTR_LENGTH(scan_str.cp16))
      end = WSTR_LENGTH(scan_str.cp16);
  if unlikely(end <= start)
     goto match_not_found; /* Empty search area. */
  scan_len = end - start;
  match_start.cp16 = memcasememw(scan_str.cp16 + start,scan_len,
                                 open_str.cp16,open_len);
  if unlikely(!match_start.cp16) goto match_not_found;
  match_end.cp16 = find_casematchw(match_start.cp16 + open_len,scan_len -
                                  (match_start.cp16 - (scan_str.cp16 + start)),
                                   open_str.cp16,open_len,
                                   clos_str.cp16,clos_len);
  if unlikely(!match_end.cp16) goto match_not_found;
  SET_STRING(DeeString_New2Byte(scan_str.cp16,
                               (size_t)(match_start.cp16-scan_str.cp16)),
             DeeString_New2Byte(match_start.cp16,
                               (match_end.cp16 + clos_len) -
                                match_start.cp16),
             DeeString_New2Byte(match_end.cp16 + clos_len,
                               (size_t)(scan_str.cp16 + end) -
                               (size_t)(match_end.cp16 + clos_len)));
  break;
 CASE_WIDTH_4BYTE:
  scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!scan_str.cp32) goto err_r_0;
  open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp32) goto err_r_0;
  clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp32) goto err_r_0;
  open_len = WSTR_LENGTH(open_str.cp32);
  clos_len = WSTR_LENGTH(clos_str.cp32);
  if (end > WSTR_LENGTH(scan_str.cp32))
      end = WSTR_LENGTH(scan_str.cp32);
  if unlikely(end <= start)
     goto match_not_found; /* Empty search area. */
  scan_len = end - start;
  match_start.cp32 = memcasememl(scan_str.cp32 + start,scan_len,
                                 open_str.cp32,open_len);
  if unlikely(!match_start.cp32) goto match_not_found;
  match_end.cp32 = find_casematchl(match_start.cp32 + open_len,scan_len -
                                  (match_start.cp32 - (scan_str.cp32 + start)),
                                   open_str.cp32,open_len,
                                   clos_str.cp32,clos_len);
  if unlikely(!match_end.cp32) goto match_not_found;
  SET_STRING(DeeString_New4Byte(scan_str.cp32,
                               (size_t)(match_start.cp32-scan_str.cp32)),
             DeeString_New4Byte(match_start.cp32,
                               (match_end.cp32 + clos_len) -
                                match_start.cp32),
             DeeString_New4Byte(match_end.cp32 + clos_len,
                               (size_t)(scan_str.cp32 + end) -
                               (size_t)(match_end.cp32 + clos_len)));
  break;
 }
#undef SET_STRING
done:
 return (DREF DeeObject *)result;
match_not_found:
 result->t_elem[0] = (DREF DeeObject *)string_getsubstr(self,start,end);
 if unlikely(!result->t_elem[0]) goto err_r_0;
 result->t_elem[1] = Dee_EmptyString;
 result->t_elem[2] = Dee_EmptyString;
#ifdef CONFIG_NO_THREADS
 Dee_EmptyString->ob_refcnt += 2;
#else
 ATOMIC_FETCHADD(Dee_EmptyString->ob_refcnt,2);
#endif
 goto done;
err_r_2:
 Dee_DecrefDokill(result->t_elem[1]);
err_r_1:
 Dee_DecrefDokill(result->t_elem[0]);
err_r_0:
 DeeTuple_FreeUninitialized((DeeObject *)result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caserpartitionmatch(String *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
 String *s_open,*s_clos; size_t start = 0,end = (size_t)-1;
 union dcharptr scan_str,open_str,clos_str,match_start,match_end;
 size_t scan_len,open_len,clos_len; DREF DeeTupleObject *result;
 if (DeeArg_Unpack(argc,argv,"oo|IdId:caserpartitionmatch",&s_open,&s_clos,&start,&end) ||
     DeeObject_AssertTypeExact((DeeObject *)s_open,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)s_clos,&DeeString_Type))
     goto err;
#define SET_STRING(a,b,c) \
 do{ \
   if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) goto err_r_0; \
   if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) goto err_r_1; \
   if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) goto err_r_2; \
 }__WHILE0
 result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
                                          DeeString_WIDTH(s_open),
                                          DeeString_WIDTH(s_clos))) {
 CASE_WIDTH_1BYTE:
  scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
  open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
  clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
  open_len = WSTR_LENGTH(open_str.cp8);
  clos_len = WSTR_LENGTH(clos_str.cp8);
  if (end > WSTR_LENGTH(scan_str.cp8))
      end = WSTR_LENGTH(scan_str.cp8);
  if unlikely(end <= start)
     goto match_not_found; /* Empty search area. */
  scan_len = end - start;
  match_end.cp8 = memcasermemb(scan_str.cp8 + start,scan_len,
                               clos_str.cp8,clos_len);
  if unlikely(!match_end.cp8) goto match_not_found;
  match_start.cp8 = rfind_casematchb(scan_str.cp8 + start,
                                    (size_t)(match_end.cp8 - (scan_str.cp8 + start)),
                                     open_str.cp8,open_len,
                                     clos_str.cp8,clos_len);
  if unlikely(!match_start.cp8) goto match_not_found;
  SET_STRING(DeeString_New1Byte(scan_str.cp8,
                               (size_t)(match_start.cp8 - scan_str.cp8)),
             DeeString_New1Byte(match_start.cp8,
                               (size_t)((match_end.cp8 + clos_len) -
                                         match_start.cp8)),
             DeeString_New1Byte(match_end.cp8 + clos_len,
                               (size_t)(scan_str.cp8 + end)-
                               (size_t)(match_end.cp8 + clos_len)));
  break;
 CASE_WIDTH_2BYTE:
  scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
  if unlikely(!scan_str.cp16) goto err_r_0;
  open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp16) goto err_r_0;
  clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp16) goto err_r_0;
  open_len = WSTR_LENGTH(open_str.cp16);
  clos_len = WSTR_LENGTH(clos_str.cp16);
  if (end > WSTR_LENGTH(scan_str.cp16))
      end = WSTR_LENGTH(scan_str.cp16);
  if unlikely(end <= start)
     goto match_not_found; /* Empty search area. */
  scan_len = end - start;
  match_end.cp16 = memcasermemw(scan_str.cp16 + start,scan_len,
                                clos_str.cp16,clos_len);
  if unlikely(!match_end.cp16) goto match_not_found;
  match_start.cp16 = rfind_casematchw(scan_str.cp16 + start,
                                     (size_t)(match_end.cp16 - (scan_str.cp16 + start)),
                                      open_str.cp16,open_len,
                                      clos_str.cp16,clos_len);
  if unlikely(!match_start.cp16) goto match_not_found;
  SET_STRING(DeeString_New2Byte(scan_str.cp16,
                               (size_t)(match_start.cp16 - scan_str.cp16)),
             DeeString_New2Byte(match_start.cp16,
                               (size_t)((match_end.cp16 + clos_len) -
                                         match_start.cp16)),
             DeeString_New2Byte(match_end.cp16 + clos_len,
                               (size_t)(scan_str.cp16 + end)-
                               (size_t)(match_end.cp16 + clos_len)));
  break;
 CASE_WIDTH_4BYTE:
  scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
  if unlikely(!scan_str.cp32) goto err_r_0;
  open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
  if unlikely(!open_str.cp32) goto err_r_0;
  clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
  if unlikely(!clos_str.cp32) goto err_r_0;
  open_len = WSTR_LENGTH(open_str.cp32);
  clos_len = WSTR_LENGTH(clos_str.cp32);
  if (end > WSTR_LENGTH(scan_str.cp32))
      end = WSTR_LENGTH(scan_str.cp32);
  if unlikely(end <= start)
     goto match_not_found; /* Empty search area. */
  scan_len = end - start;
  match_end.cp32 = memcasermeml(scan_str.cp32 + start,scan_len,
                                clos_str.cp32,clos_len);
  if unlikely(!match_end.cp32) goto match_not_found;
  match_start.cp32 = rfind_casematchl(scan_str.cp32 + start,
                                     (size_t)(match_end.cp32 - (scan_str.cp32 + start)),
                                      open_str.cp32,open_len,
                                      clos_str.cp32,clos_len);
  if unlikely(!match_start.cp32) goto match_not_found;
  SET_STRING(DeeString_New4Byte(scan_str.cp32,
                               (size_t)(match_start.cp32 - scan_str.cp32)),
             DeeString_New4Byte(match_start.cp32,
                               (size_t)((match_end.cp32 + clos_len) -
                                         match_start.cp32)),
             DeeString_New4Byte(match_end.cp32 + clos_len,
                               (size_t)(scan_str.cp32 + end)-
                               (size_t)(match_end.cp32 + clos_len)));
  break;
 }
#undef SET_STRING
done:
 return (DREF DeeObject *)result;
match_not_found:
 result->t_elem[0] = (DREF DeeObject *)string_getsubstr(self,start,end);
 if unlikely(!result->t_elem[0]) goto err_r_0;
 result->t_elem[1] = Dee_EmptyString;
 result->t_elem[2] = Dee_EmptyString;
#ifdef CONFIG_NO_THREADS
 Dee_EmptyString->ob_refcnt += 2;
#else
 ATOMIC_FETCHADD(Dee_EmptyString->ob_refcnt,2);
#endif
 goto done;
err_r_2:
 Dee_DecrefDokill(result->t_elem[1]);
err_r_1:
 Dee_DecrefDokill(result->t_elem[0]);
err_r_0:
 DeeTuple_FreeUninitialized((DeeObject *)result);
err:
 return NULL;
}


struct re_args {
    char    *re_dataptr;    /* Starting pointer to regex input data (in UTF-8). */
    size_t   re_datalen;    /* Number of _bytes_ of regex input data. */
    char    *re_patternptr; /* Starting pointer to regex pattern data (in UTF-8). */
    size_t   re_patternlen; /* Number of _bytes_ of regex pattern data. */
    uint16_t re_flags;      /* Regex flags. */
    uint16_t re_pad[(sizeof(void *)-2)/2]; /* ... */
    size_t   re_offset;     /* Starting character-offset into the matched string. */
};

struct re_args_ex {
    char    *re_dataptr;    /* Starting pointer to regex input data (in UTF-8). */
    size_t   re_datalen;    /* Number of _bytes_ of regex input data. */
    char    *re_patternptr; /* Starting pointer to regex pattern data (in UTF-8). */
    size_t   re_patternlen; /* Number of _bytes_ of regex pattern data. */
    uint16_t re_flags;      /* Regex flags. */
    uint16_t re_pad[(sizeof(void *)-2)/2]; /* ... */
    size_t   re_offset;     /* Starting character-offset into the matched string. */
    size_t   re_endindex;   /* Ending character-offset within the matched string. */
};

#ifndef __NO_builtin_expect
#define regex_getargs_generic(self,function_name,argc,argv,result) \
        __builtin_expect(regex_getargs_generic(self,function_name,argc,argv,result),0)
#define regex_getargs_generic_ex(self,function_name,argc,argv,result) \
        __builtin_expect(regex_getargs_generic_ex(self,function_name,argc,argv,result),0)
#define regex_get_rules(rules_str,result) \
        __builtin_expect(regex_get_rules(rules_str,result),0)
#endif

struct regex_rule_name {
    char     name[14];
    uint16_t flag;
};

PRIVATE struct regex_rule_name const regex_rule_names[] = {
    { "dotall",    DEE_REGEX_FDOTALL },
    { "multiline", DEE_REGEX_FMULTILINE },
    { "nocase",    DEE_REGEX_FNOCASE }
};


PRIVATE int
(DCALL regex_get_rules)(char const *__restrict rules_str,
                        uint16_t *__restrict result) {
 if (*rules_str == '.' || *rules_str == '\n' ||
     *rules_str == 'l' || *rules_str == 'c') {
  /* Flag-mode */
  for (;;) {
   char ch = *rules_str++;
   if (!ch) break;
   /* */if (ch == '.')  *result |= DEE_REGEX_FDOTALL;
   else if (ch == '\n') *result |= DEE_REGEX_FMULTILINE;
   else if (ch == 'c')  *result |= DEE_REGEX_FNOCASE;
   else {
    DeeError_Throwf(&DeeError_ValueError,
                    "Invalid regex rules string flag %:1q",
                    rules_str-1);
    goto err;
   }
  }
 } else for (;;) {
  /* Comma-mode */
  char *name_start = (char *)rules_str;
  while (*rules_str && *rules_str != ',')
    ++rules_str;
  if (rules_str != name_start) {
   size_t rule_length = (size_t)(rules_str - name_start);
   size_t i;
   if (rule_length < COMPILER_LENOF(regex_rule_names[0].name)) {
    for (i = 0; i < COMPILER_LENOF(regex_rule_names); ++i) {
     if (!MEMCASEEQB(regex_rule_names[i].name,name_start,rule_length))
          continue;
     *result |= regex_rule_names[i].flag;
     goto got_rule_name;
    }
   }
   DeeError_Throwf(&DeeError_ValueError,
                   "Invalid regex rules name %$q",
                   rule_length,name_start);
   goto err;
  }
got_rule_name:
  if (!*rules_str) break;
  ++rules_str;
 }
 return 0;
err:
 return -1;
}


PRIVATE int
(DCALL regex_getargs_generic)(String *__restrict self,
                              char const *__restrict function_name,
                              size_t argc, DeeObject **__restrict argv,
                              struct re_args *__restrict result) {
 DeeObject *pattern;
 result->re_flags  = DEE_REGEX_FNORMAL;
 result->re_offset = 0;
 switch (argc) {
 case 1:
do_full_data_match:
   result->re_dataptr = DeeString_AsUtf8((DeeObject *)self);
   if unlikely(!result->re_dataptr) goto err;
   result->re_datalen = WSTR_LENGTH(result->re_dataptr);
   break;
 case 2:
  if (DeeString_Check(argv[1])) {
   /* Rules. */
   if (regex_get_rules(DeeString_STR(argv[1]),&result->re_flags))
       goto err;
   goto do_full_data_match;
  }
do_start_offset_only:
  /* Start offset. */
  if (DeeObject_AsSSize(argv[1],(dssize_t *)&result->re_offset))
      goto err;
  if (result->re_offset >= DeeString_WLEN(self)) {
do_empty_scan:
   result->re_offset  = DeeString_WLEN(self);
   result->re_dataptr = "";
   result->re_datalen = 0;
  } else {
   result->re_dataptr = DeeString_AsUtf8((DeeObject *)self);
   if unlikely(!result->re_dataptr) goto err;
   result->re_datalen = WSTR_LENGTH(result->re_dataptr);
   if (result->re_dataptr == DeeString_STR(self)) {
    /* ASCII string. */
    result->re_dataptr += result->re_offset;
    result->re_datalen -= result->re_offset;
   } else {
    size_t i; char *iter,*end;
    /* UTF-8 string (manually adjust). */
    end = (iter = result->re_dataptr)+result->re_datalen;
    for (i = 0; i < result->re_offset; ++i) {
     if (iter >= end) break;
     utf8_readchar((char const **)&iter,end);
    }
    result->re_dataptr = iter;
    result->re_datalen = (size_t)(end - iter);
   }
  }
  break;
 case 4:
  if (DeeObject_AssertTypeExact(argv[3],&DeeString_Type))
      goto err;
  if (regex_get_rules(DeeString_STR(argv[3]),&result->re_flags))
      goto err;
  ATTR_FALLTHROUGH
 {
  size_t end_offset;
 case 3:
  if (DeeObject_AsSSize(argv[2],(dssize_t *)&end_offset))
      goto err;
  if (end_offset >= DeeString_WLEN(self))
      goto do_start_offset_only;
  if (DeeObject_AsSSize(argv[1],(dssize_t *)&result->re_offset))
      goto err;
  if (result->re_offset >= end_offset)
      goto do_empty_scan;
  /* Do a sub-string scan. */
  result->re_dataptr = DeeString_AsUtf8((DeeObject *)self);
  if unlikely(!result->re_dataptr) goto err;
  result->re_datalen = WSTR_LENGTH(result->re_dataptr);
  if (result->re_dataptr == DeeString_STR(self)) {
   /* ASCII string. */
   result->re_dataptr += result->re_offset;
   result->re_datalen = end_offset - result->re_offset;
  } else {
   size_t i; char *iter,*end;
   /* UTF-8 string (manually adjust). */
   end = (iter = result->re_dataptr)+result->re_datalen;
   for (i = 0; i < result->re_offset; ++i) {
    if (iter >= end) break;
    utf8_readchar((char const **)&iter,end);
   }
   result->re_dataptr = iter;
   /* Continue scanning until the full sub-string has been indexed. */
   for (; i < end_offset; ++i) {
    if (iter >= end) break;
    utf8_readchar((char const **)&iter,end);
   }
   result->re_datalen = (size_t)(iter - result->re_dataptr);
  }
 } break;

 default:
  err_invalid_argc(function_name,argc,1,4);
  goto err;
 }
 pattern = argv[0];
 if (DeeObject_AssertTypeExact(pattern,&DeeString_Type))
     goto err;
 result->re_patternptr = DeeString_AsUtf8(pattern);
 if unlikely(!result->re_patternptr) goto err;
 result->re_patternlen = WSTR_LENGTH(result->re_patternptr);
 return 0;
err:
 return -1;
}

PRIVATE int
(DCALL regex_getargs_generic_ex)(String *__restrict self,
                                 char const *__restrict function_name,
                                 size_t argc, DeeObject **__restrict argv,
                                 struct re_args_ex *__restrict result) {
 DeeObject *pattern;
 result->re_flags    = DEE_REGEX_FNORMAL;
 result->re_offset   = 0;
 result->re_endindex = DeeString_WLEN(self);
 switch (argc) {
 case 1:
do_full_data_match:
   result->re_dataptr = DeeString_AsUtf8((DeeObject *)self);
   if unlikely(!result->re_dataptr) goto err;
   result->re_datalen = WSTR_LENGTH(result->re_dataptr);
   break;
 case 2:
  if (DeeString_Check(argv[1])) {
   /* Rules. */
   if (regex_get_rules(DeeString_STR(argv[1]),&result->re_flags))
       goto err;
   goto do_full_data_match;
  }
do_start_offset_only:
  /* Start offset. */
  if (DeeObject_AsSSize(argv[1],(dssize_t *)&result->re_offset))
      goto err;
  if (result->re_offset >= DeeString_WLEN(self)) {
do_empty_scan:
   result->re_offset   = DeeString_WLEN(self);
   result->re_endindex = result->re_offset;
   result->re_dataptr  = "";
   result->re_datalen  = 0;
  } else {
   result->re_dataptr = DeeString_AsUtf8((DeeObject *)self);
   if unlikely(!result->re_dataptr) goto err;
   result->re_datalen = WSTR_LENGTH(result->re_dataptr);
   if (result->re_dataptr == DeeString_STR(self)) {
    /* ASCII string. */
    result->re_dataptr += result->re_offset;
    result->re_datalen -= result->re_offset;
   } else {
    size_t i; char *iter,*end;
    /* UTF-8 string (manually adjust). */
    end = (iter = result->re_dataptr)+result->re_datalen;
    for (i = 0; i < result->re_offset; ++i) {
     if (iter >= end) break;
     utf8_readchar((char const **)&iter,end);
    }
    result->re_dataptr = iter;
    result->re_datalen = (size_t)(end - iter);
   }
  }
  break;
 case 4:
  if (DeeObject_AssertTypeExact(argv[3],&DeeString_Type))
      goto err;
  if (regex_get_rules(DeeString_STR(argv[3]),&result->re_flags))
      goto err;
  ATTR_FALLTHROUGH
 case 3:
  if (DeeObject_AsSSize(argv[2],(dssize_t *)&result->re_endindex))
      goto err;
  if (result->re_endindex >= DeeString_WLEN(self)) {
   result->re_endindex = DeeString_WLEN(self);
   goto do_start_offset_only;
  }
  if (DeeObject_AsSSize(argv[1],(dssize_t *)&result->re_offset))
      goto err;
  if (result->re_offset >= result->re_endindex)
      goto do_empty_scan;
  /* Do a sub-string scan. */
  result->re_dataptr = DeeString_AsUtf8((DeeObject *)self);
  if unlikely(!result->re_dataptr) goto err;
  result->re_datalen = WSTR_LENGTH(result->re_dataptr);
  if (result->re_dataptr == DeeString_STR(self)) {
   /* ASCII string. */
   result->re_dataptr += result->re_offset;
   result->re_datalen = result->re_endindex - result->re_offset;
  } else {
   size_t i; char *iter,*end;
   /* UTF-8 string (manually adjust). */
   end = (iter = result->re_dataptr)+result->re_datalen;
   for (i = 0; i < result->re_offset; ++i) {
    if (iter >= end) break;
    utf8_readchar((char const **)&iter,end);
   }
   result->re_dataptr = iter;
   /* Continue scanning until the full sub-string has been indexed. */
   for (; i < result->re_endindex; ++i) {
    if (iter >= end) break;
    utf8_readchar((char const **)&iter,end);
   }
   result->re_datalen = (size_t)(iter - result->re_dataptr);
  }
  break;

 default:
  err_invalid_argc(function_name,argc,1,4);
  goto err;
 }
 pattern = argv[0];
 if (DeeObject_AssertTypeExact(pattern,&DeeString_Type))
     goto err;
 result->re_patternptr = DeeString_AsUtf8(pattern);
 if unlikely(!result->re_patternptr) goto err;
 result->re_patternlen = WSTR_LENGTH(result->re_patternptr);
 return 0;
err:
 return -1;
}

PRIVATE DREF DeeObject *DCALL
string_rematch(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 struct re_args args; size_t result;
 if (regex_getargs_generic(self,"rematch",argc,argv,&args))
     goto err;
 result = DeeRegex_Matches(args.re_dataptr,
                           args.re_datalen,
                           args.re_patternptr,
                           args.re_patternlen,
                           args.re_flags);
 if unlikely(result == (size_t)-1) goto err;
 return DeeInt_NewSize(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
regex_pack_range(size_t offset,
                 struct regex_range const *__restrict range) {
 DREF DeeObject *result,*temp;
 result = DeeTuple_NewUninitialized(2);
 if unlikely(!result) goto err;
 temp = DeeInt_NewSize(offset + range->rr_start);
 if unlikely(!temp) goto err_r;
 DeeTuple_SET(result,0,temp);
 temp = DeeInt_NewSize(offset + range->rr_end);
 if unlikely(!temp) goto err_r_0;
 DeeTuple_SET(result,1,temp);
 return result;
err_r_0:
 Dee_Decref_likely(DeeTuple_GET(result,0));
err_r:
 DeeTuple_FreeUninitialized(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_refind(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 struct re_args args; int error;
 struct regex_range result_range;
 if (regex_getargs_generic(self,"refind",argc,argv,&args))
     goto err;
 error = DeeRegex_Find(args.re_dataptr,
                       args.re_datalen,
                       args.re_patternptr,
                       args.re_patternlen,
                      &result_range,
                       args.re_flags);
 if unlikely(error < 0) goto err;
 if (!error) return_none;
 return regex_pack_range(args.re_offset,&result_range);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rerfind(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 struct re_args args; int error;
 struct regex_range result_range;
 if (regex_getargs_generic(self,"rerfind",argc,argv,&args))
     goto err;
 error = DeeRegex_RFind(args.re_dataptr,
                        args.re_datalen,
                        args.re_patternptr,
                        args.re_patternlen,
                       &result_range,
                        args.re_flags);
 if unlikely(error < 0) goto err;
 if (!error) return_none;
 return regex_pack_range(args.re_offset,&result_range);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_reindex(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 struct re_args args; int error;
 struct regex_range result_range;
 if (regex_getargs_generic(self,"reindex",argc,argv,&args))
     goto err;
 error = DeeRegex_Find(args.re_dataptr,
                       args.re_datalen,
                       args.re_patternptr,
                       args.re_patternlen,
                      &result_range,
                       args.re_flags);
 if unlikely(error < 0) goto err;
 if (!error) goto err_not_found;
 return regex_pack_range(args.re_offset,&result_range);
err_not_found:
 err_index_not_found((DeeObject *)self,argv[0]);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rerindex(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 struct re_args args; int error;
 struct regex_range result_range;
 if (regex_getargs_generic(self,"rerindex",argc,argv,&args))
     goto err;
 error = DeeRegex_RFind(args.re_dataptr,
                        args.re_datalen,
                        args.re_patternptr,
                        args.re_patternlen,
                       &result_range,
                        args.re_flags);
 if unlikely(error < 0) goto err;
 if (!error) goto err_not_found;
 return regex_pack_range(args.re_offset,&result_range);
err_not_found:
 err_index_not_found((DeeObject *)self,argv[0]);
err:
 return NULL;
}

PRIVATE DREF String *DCALL
string_relocate(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 struct re_args args; int error;
 struct regex_range result_range;
 if (regex_getargs_generic(self,"relocate",argc,argv,&args))
     goto err;
 error = DeeRegex_Find(args.re_dataptr,
                       args.re_datalen,
                       args.re_patternptr,
                       args.re_patternlen,
                      &result_range,
                       args.re_flags);
 if unlikely(error < 0) goto err;
 if (!error) return_reference_((String *)Dee_EmptyString);
 return string_getsubstr_ib(self,
                            args.re_offset + result_range.rr_start,
                            args.re_offset + result_range.rr_end);
err:
 return NULL;
}

PRIVATE DREF String *DCALL
string_rerlocate(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 struct re_args args; int error;
 struct regex_range result_range;
 if (regex_getargs_generic(self,"rerlocate",argc,argv,&args))
     goto err;
 error = DeeRegex_RFind(args.re_dataptr,
                        args.re_datalen,
                        args.re_patternptr,
                        args.re_patternlen,
                       &result_range,
                        args.re_flags);
 if unlikely(error < 0) goto err;
 if (!error) return_reference_((String *)Dee_EmptyString);
 return string_getsubstr_ib(self,
                            args.re_offset + result_range.rr_start,
                            args.re_offset + result_range.rr_end);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
regex_pack_partition(String *__restrict self, size_t offset,
                     struct regex_range const *__restrict range) {
 DREF DeeObject *result;
 DREF String *temp;
 result = DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto err;
 if (!range->rr_start && !offset) {
  DeeTuple_SET(result,0,Dee_EmptyString);
  Dee_Incref(Dee_EmptyString);
  if (range->rr_end != DeeString_WLEN(self))
      goto allocate_matched_substring;
  DeeTuple_SET(result,1,(DeeObject *)self);
  Dee_Incref(self);
  DeeTuple_SET(result,2,Dee_EmptyString);
  Dee_Incref(Dee_EmptyString);
  return result;
 }
 temp = string_getsubstr_ib(self,
                            0,
                            offset + range->rr_start);
 if unlikely(!temp) goto err_r;
 DeeTuple_SET(result,0,(DeeObject *)temp); /* Inherit reference. */
allocate_matched_substring:
 temp = string_getsubstr_ib(self,
                            offset + range->rr_start,
                            offset + range->rr_end);
 if unlikely(!temp) goto err_r_0;
 DeeTuple_SET(result,1,(DeeObject *)temp); /* Inherit reference. */
 if (offset + range->rr_end == 0) {
  DeeTuple_SET(result,2,(DeeObject *)self);
  Dee_Incref(self);
 } else if (offset + range->rr_end == DeeString_WLEN(self)) {
  DeeTuple_SET(result,2,Dee_EmptyString);
  Dee_Incref(Dee_EmptyString);
 } else {
  temp = string_getsubstr(self,
                          offset + range->rr_end,
                         (size_t)-1);
  if unlikely(!temp) goto err_r_1;
  DeeTuple_SET(result,2,(DeeObject *)temp); /* Inherit reference. */
 }
 return result;
err_r_1:
 Dee_Decref_likely(DeeTuple_GET(result,1));
err_r_0:
 Dee_Decref_likely(DeeTuple_GET(result,0));
err_r:
 DeeTuple_FreeUninitialized(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_repartition(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 struct re_args args; int error;
 struct regex_range result_range;
 if (regex_getargs_generic(self,"repartition",argc,argv,&args))
     goto err;
 error = DeeRegex_Find(args.re_dataptr,
                       args.re_datalen,
                       args.re_patternptr,
                       args.re_patternlen,
                      &result_range,
                       args.re_flags);
 if unlikely(error < 0) goto err;
 if (!error) return DeeTuple_Pack(3,self,Dee_EmptyString,Dee_EmptyString);
 return regex_pack_partition(self,args.re_offset,&result_range);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rerpartition(String *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 struct re_args args; int error;
 struct regex_range result_range;
 if (regex_getargs_generic(self,"rerpartition",argc,argv,&args))
     goto err;
 error = DeeRegex_RFind(args.re_dataptr,
                        args.re_datalen,
                        args.re_patternptr,
                        args.re_patternlen,
                       &result_range,
                        args.re_flags);
 if unlikely(error < 0) goto err;
 if (!error) return DeeTuple_Pack(3,self,Dee_EmptyString,Dee_EmptyString);
 return regex_pack_partition(self,args.re_offset,&result_range);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rereplace(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 String *find_pattern,*replace;
 DeeObject *opt1 = NULL,*opt2 = NULL;
 size_t max_count = (size_t)-1;
 uint16_t re_flags = DEE_REGEX_FNORMAL;
 char *pattern_ptr; size_t pattern_len;
 char *data_ptr; size_t data_len;
 if (DeeArg_Unpack(argc,argv,"oo|oo",&find_pattern,&replace,&opt1,&opt2) ||
     DeeObject_AssertTypeExact((DeeObject *)find_pattern,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)replace,&DeeString_Type))
     goto err;
 if (opt1) {
  if (DeeString_Check(opt1)) {
   if (regex_get_rules(DeeString_STR(opt1),&re_flags))
       goto err;
   if (opt2 && DeeObject_AsSize(opt2,&max_count))
       goto err;
  } else {
   if (DeeObject_AsSize(opt1,&max_count))
       goto err;
   if (opt2 &&
      (DeeObject_AssertTypeExact(opt2,&DeeString_Type) ||
       regex_get_rules(DeeString_STR(opt2),&re_flags)))
       goto err;
  }
 }
 pattern_ptr = DeeString_AsUtf8((DeeObject *)find_pattern);
 if unlikely(!pattern_ptr) goto err;
 data_ptr = DeeString_AsUtf8((DeeObject *)self);
 if unlikely(!data_ptr) goto err;
 pattern_len = WSTR_LENGTH(pattern_ptr);
 data_len    = WSTR_LENGTH(data_ptr);
 {
  struct unicode_printer printer = UNICODE_PRINTER_INIT;
  char *iter,*end,*flush_start;
  end = (flush_start = iter = data_ptr) + data_len;
  while (iter < end) {
   struct regex_range_ptr range; int error;
   error = DeeRegex_FindPtr(iter,
                           (size_t)(end - iter),
                            pattern_ptr,
                            pattern_len,
                           &range,
                            re_flags);
   if unlikely(error < 0) goto err_printer;
   if (!error) break;
   if (flush_start < range.rr_start) {
    if (unicode_printer_printutf8(&printer,(unsigned char *)flush_start,
                                 (size_t)(range.rr_start - flush_start)) < 0)
        goto err_printer;
   }
   if (unicode_printer_printstring(&printer,(DeeObject *)replace) < 0)
       goto err_printer;
   flush_start = range.rr_end;
   if (!max_count--) break;
   iter = range.rr_end;
  }
  /* Flush the remainder. */
  if (unicode_printer_printutf8(&printer,(unsigned char *)flush_start,
                               (size_t)(end - flush_start)) < 0)
      goto err_printer;
  return unicode_printer_pack(&printer);
err_printer:
  unicode_printer_fini(&printer);
 }
err:
 return NULL;
}

INTDEF DREF DeeObject *DCALL string_re_findall(String *__restrict self, String *__restrict pattern, struct re_args const *__restrict args);
INTDEF DREF DeeObject *DCALL string_re_locateall(String *__restrict self, String *__restrict pattern, struct re_args const *__restrict args);
INTDEF DREF DeeObject *DCALL string_re_split(String *__restrict self, String *__restrict pattern, struct re_args const *__restrict args);

PRIVATE DREF DeeObject *DCALL
string_refindall(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 struct re_args args;
 if (regex_getargs_generic(self,"refindall",argc,argv,&args))
     goto err;
 return string_re_findall(self,(String *)argv[0],&args);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_relocateall(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 struct re_args args;
 if (regex_getargs_generic(self,"relocateall",argc,argv,&args))
     goto err;
 return string_re_locateall(self,(String *)argv[0],&args);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_resplit(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 struct re_args args;
 if (regex_getargs_generic(self,"resplit",argc,argv,&args))
     goto err;
 return string_re_split(self,(String *)argv[0],&args);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_restartswith(String *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 struct re_args args; size_t result;
 if (regex_getargs_generic(self,"restartswith",argc,argv,&args))
     goto err;
 result = DeeRegex_Matches(args.re_dataptr,
                           args.re_datalen,
                           args.re_patternptr,
                           args.re_patternlen,
                           args.re_flags);
 if unlikely(result == (size_t)-1) goto err;
 return_bool_(result != 0);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_reendswith(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 struct re_args args; int error;
 struct regex_range_ptr range;
 if (regex_getargs_generic(self,"reendswith",argc,argv,&args))
     goto err;
 error = DeeRegex_RFindPtr(args.re_dataptr,
                           args.re_datalen,
                           args.re_patternptr,
                           args.re_patternlen,
                          &range,
                           args.re_flags);
 if unlikely(error < 0) goto err;
 return_bool_(error &&
              range.rr_end == args.re_dataptr +
                              args.re_datalen);
err:
 return NULL;
}

PRIVATE DREF String *DCALL
string_relstrip(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 struct re_args args; size_t temp;
 if (regex_getargs_generic(self,"relstrip",argc,argv,&args))
     goto err;
 for (;;) {
  char *data_end;
  temp = DeeRegex_MatchesPtr(args.re_dataptr,
                             args.re_datalen,
                             args.re_patternptr,
                             args.re_patternlen,
                            (char const **)&data_end,
                             args.re_flags);
  if unlikely(temp == (size_t)-1) goto err;
  if (!temp) break;
  args.re_offset += temp;
  args.re_datalen -= (size_t)(data_end - args.re_dataptr);
  args.re_dataptr  = data_end;
 }
 return string_getsubstr(self,args.re_offset,(size_t)-1);
err:
 return NULL;
}

PRIVATE DREF String *DCALL
string_rerstrip(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 struct re_args_ex args; int error;
 struct regex_range_ex range;
 if (regex_getargs_generic_ex(self,"rerstrip",argc,argv,&args))
     goto err;
 for (;;) {
  error = DeeRegex_RFindEx(args.re_dataptr,
                           args.re_datalen,
                           args.re_patternptr,
                           args.re_patternlen,
                          &range,
                           args.re_flags);
  if unlikely(error < 0) goto err;
  if (!error) break;
  if (range.rr_end_ptr != args.re_dataptr + args.re_datalen) break;
  ASSERT(range.rr_start < range.rr_end);
  ASSERT(range.rr_start_ptr < range.rr_end_ptr);
  args.re_datalen  = (size_t)(range.rr_start_ptr - args.re_dataptr);
  args.re_endindex = args.re_offset + range.rr_start;
 }
 return string_getsubstr_ib(self,args.re_offset,args.re_endindex);
err:
 return NULL;
}

PRIVATE DREF String *DCALL
string_restrip(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 struct re_args_ex args; int error;
 struct regex_range_ex range;
 if (regex_getargs_generic_ex(self,"restrip",argc,argv,&args))
     goto err;
 for (;;) {
  char *data_end; size_t temp;
  temp = DeeRegex_MatchesPtr(args.re_dataptr,
                             args.re_datalen,
                             args.re_patternptr,
                             args.re_patternlen,
                            (char const **)&data_end,
                             args.re_flags);
  if unlikely(temp == (size_t)-1) goto err;
  if (!temp) break;
  args.re_offset  += temp;
  args.re_datalen -= (size_t)(data_end - args.re_dataptr);
  args.re_dataptr  = data_end;
 }
 for (;;) {
  error = DeeRegex_RFindEx(args.re_dataptr,
                           args.re_datalen,
                           args.re_patternptr,
                           args.re_patternlen,
                          &range,
                           args.re_flags);
  if unlikely(error < 0) goto err;
  if (!error) break;
  if (range.rr_end_ptr != args.re_dataptr + args.re_datalen) break;
  ASSERT(range.rr_start < range.rr_end);
  ASSERT(range.rr_start_ptr < range.rr_end_ptr);
  args.re_datalen  = (size_t)(range.rr_start_ptr - args.re_dataptr);
  args.re_endindex = args.re_offset + range.rr_start;
 }
 return string_getsubstr_ib(self,args.re_offset,args.re_endindex);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_recontains(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 struct re_args_ex args; int error;
 struct regex_range_ptr range;
 if (regex_getargs_generic_ex(self,"recontains",argc,argv,&args))
     goto err;
 error = DeeRegex_FindPtr(args.re_dataptr,
                          args.re_datalen,
                          args.re_patternptr,
                          args.re_patternlen,
                         &range,
                          args.re_flags);
 if unlikely(error < 0) goto err;
 return_bool_(error);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_recount(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 struct re_args_ex args; int error; size_t result;
 struct regex_range_ptr range;
 if (regex_getargs_generic_ex(self,"recount",argc,argv,&args))
     goto err;
 result = 0;
 for (;;) {
  error = DeeRegex_FindPtr(args.re_dataptr,
                           args.re_datalen,
                           args.re_patternptr,
                           args.re_patternlen,
                          &range,
                           args.re_flags);
  if unlikely(error < 0) goto err;
  if (!error) break;
  ++result;
  args.re_datalen -= (size_t)(range.rr_end - args.re_dataptr);
  args.re_dataptr  = range.rr_end;
 }
 return DeeInt_NewSize(result);
err:
 return NULL;
}


INTERN struct type_method string_methods[] = {

    /* String encode/decode functions */
    { "decode", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_decode,
      DOC("(string codec,string errors=\"strict\")->string\n"
          "(string codec,string errors=\"strict\")->object\n"
          "@throw ValueError The given @codec or @errors wasn't recognized\n"
          "@throw UnicodeDecodeError @this string could not be decoded as @codec and @errors was set to $\"strict\"\n"
          "@param errors The way that decode-errors are handled as one of $\"strict\", $\"replace\" or $\"ignore\"\n"
          "Decode @this string, re-interpreting its underlying character bytes as @codec\n"
          "Prior to processing, @codec is normalized as follows:\n"
          ">name = name.lower().replace(\"_\",\"-\");\n"
          ">if (name.startswith(\"iso-\"))\n"
          "> name = \"iso\"+name[4:];\n"
          ">else if (name.startswith(\"cp-\")) {\n"
          "> name = \"cp\"+name[3:];\n"
          ">}\n"
          "Following that, @codec is compared against the following list of builtin codecs\n"
          "%{table Codec Name|Aliases|Return type|Description\n"
          "$\"ascii\"|$\"646\", $\"us-ascii\"|Same as $this|"
              "Validate that all character of @this are apart of the unicode range U+0000 - U+007F\n"
          "$\"latin-1\"|$\"iso8859-1\", $\"iso8859\", $\"8859\", $\"cp819\", $\"latin\", $\"latin1\", $\"l1\"|Same as $this|"
              "Validate that all character of @this are apart of the unicode range U+0000 - U+00FF\n"
          "$\"utf-8\"|$\"utf8\", $\"u8\", $\"utf\"|:string|Decode ${this.bytes()} as a UTF-8 encoded byte sequence\n"
          "$\"utf-16\"|$\"utf16\", $\"u16\"|:string|Decode ${this.bytes()} as a UTF-16 sequence, encoded in host-endian\n"
          "$\"utf-16-le\"|$\"utf16-le\", $\"u16-le\", $\"utf-16le\", $\"utf16le\", $\"u16le\"|:string|Decode ${this.bytes()} as a UTF-16 sequence, encoded in little-endian\n"
          "$\"utf-16-be\"|$\"utf16-be\", $\"u16-be\", $\"utf-16be\", $\"utf16be\", $\"u16be\"|:string|Decode ${this.bytes()} as a UTF-16 sequence, encoded in big-endian\n"
          "$\"utf-32\"|$\"utf32\", $\"u32\"|:string|Decode ${this.bytes()} as a UTF-32 sequence, encoded in host-endian\n"
          "$\"utf-32-le\"|$\"utf32-le\", $\"u32-le\", $\"utf-32le\", $\"utf32le\", $\"u32le\"|:string|Decode ${this.bytes()} as a UTF-32 sequence, encoded in little-endian\n"
          "$\"utf-32-be\"|$\"utf32-be\", $\"u32-be\", $\"utf-32be\", $\"utf32be\", $\"u32be\"|:string|Decode ${this.bytes()} as a UTF-32 sequence, encoded in big-endian\n"
          "$\"string-escape\"|$\"backslash-escape\", $\"c-escape\"|:string|Decode a backslash-escaped string after stripping an optional leading and trailing $\"\\\"\" or $\"\\\'\" character\n"
          "}\n"
          "If the given @codec is not apart of this list, a call is made to :codecs:decode") },
    { "encode", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_encode,
      DOC("(string codec,string errors=\"strict\")->bytes\n"
          "(string codec,string errors=\"strict\")->string\n"
          "(string codec,string errors=\"strict\")->object\n"
          "@throw ValueError The given @codec or @errors wasn't recognized\n"
          "@throw UnicodeEncodeError @this string could not be decoded as @codec and @errors was set to $\"strict\"\n"
          "@param errors The way that decode-errors are handled as one of $\"strict\", $\"replace\" or $\"ignore\"\n"
          "Encode @this string, re-interpreting its underlying character bytes as @codec\n"
          "Prior to processing, @codec is normalized as follows:\n"
          ">name = name.lower().replace(\"_\",\"-\");\n"
          ">if (name.startswith(\"iso-\"))\n"
          "> name = \"iso\"+name[4:];\n"
          ">else if (name.startswith(\"cp-\")) {\n"
          "> name = \"cp\"+name[3:];\n"
          ">}\n"
          "Following that, @codec is compared against the following list of builtin codecs\n"
          "%{table Codec Name|Aliases|Return type|Description\n"
          "$\"ascii\"|$\"646\", $\"us-ascii\"|Same as $this|"
              "Validate that all character of @this are apart of the unicode range U+0000 - U+007F\n"
          "$\"latin-1\"|$\"iso8859-1\", $\"iso8859\", $\"8859\", $\"cp819\", $\"latin\", $\"latin1\", $\"l1\"|Same as $this|"
              "Validate that all character of @this are apart of the unicode range U+0000 - U+00FF\n"
          "$\"utf-8\"|$\"utf8\", $\"u8\", $\"utf\"|:bytes|Encode character of @this string as a UTF-8 encoded byte sequence\n"
          "$\"utf-16\"|$\"utf16\", $\"u16\"|:bytes|Encode 'as a UTF-16 sequence, encoded in host-endian\n"
          "$\"utf-16-le\"|$\"utf16-le\", $\"u16-le\", $\"utf-16le\", $\"utf16le\", $\"u16le\"|:bytes|Encode @this string as a UTF-16 sequence, encoded in little-endian\n"
          "$\"utf-16-be\"|$\"utf16-be\", $\"u16-be\", $\"utf-16be\", $\"utf16be\", $\"u16be\"|:bytes|Encode @this string as a UTF-16 sequence, encoded in big-endian\n"
          "$\"utf-32\"|$\"utf32\", $\"u32\"|:bytes|Encode @this string as a UTF-32 sequence, encoded in host-endian\n"
          "$\"utf-32-le\"|$\"utf32-le\", $\"u32-le\", $\"utf-32le\", $\"utf32le\", $\"u32le\"|:bytes|Encode @this string as a UTF-32 sequence, encoded in little-endian\n"
          "$\"utf-32-be\"|$\"utf32-be\", $\"u32-be\", $\"utf-32be\", $\"utf32be\", $\"u32be\"|:bytes|Encode @this string as a UTF-32 sequence, encoded in big-endian\n"
          "$\"string-escape\"|$\"backslash-escape\", $\"c-escape\"|:string|Encode @this string as a backslash-escaped string. This is similar to #op:repr, however the string is not surrounded by $\"\\\"\"-characters\n"
          "}\n"
          "If the given @codec is not apart of this list, a call is made to :codecs:encode") },
    { "bytes", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_bytes,
      DOC("(bool allow_invalid=false)->bytes\n"
          "(int start,int end,bool allow_invalid=false)->bytes\n"
          "@throw ValueError @allow_invalid is :false, and @this string contains characters above $0xff\n"
          "Returns a read-only bytes representation of the characters within ${this.substr(start,end)}, "
          "using a single byte per character. A character greater than $0xff either causes : ValueError "
          "to be thrown (when @allow_invalid is false), or is replaced with the ASCII character "
          "$\"?\" in the returned bytes object") },
    { "ord", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_ord,
      DOC("->int\n"
          "@throw ValueError The length of @this string is not equal to ${1}\n"
          "Return the ordinal integral value of @this single-character string\n"
          "\n"
          "(int index)->int\n"
          "@throw IntegerOverflow The given @index is lower than $0\n"
          "@throw IndexError The given @index is greater than ${#this}\n"
          "Returns the ordinal integral value of the @index'th character of @this string") },

    /* String formatting / scanning. */
    { "format", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_format,
      DOC("(sequence args)->string\n"
          "Format @this string using @args:\n"
          "This kind of formating is the most powerful variant of string formatting available in deemon.\n"
          "Like most other ways of formatting strings, all text outside of format specifiers is simply "
          "copied into the output string. Special rules are applied to text found inside or curly-braces ($\"{...}\")\n"
          "In order to escape either $\"{\" or $\"}\" characters, use $\"{{\" and $\"}}\" respectively\n"
          "Processing of text inside of curly-brace is split into 2 parts, both of which are optional and seperated by $\":\":\n"
          "-The object selection expression part\n"
          "-The object spec string portion (which may contain further $\"{...}\"-blocks that are expanded beforehand)\n"
          "%{table Selection expression|Description\n"
          "$\"{}\"|Lazily create an iterator $iter as ${args.operator iter()} when encountered the "
                  "first time, then invoke ${iter.operator next()} and use its return value as format object\n"
          "$\"{foo}\"|Use ${args[\"foo\"]} as format object\n"
          "$\"{42}\"|Use ${args[42]} as format object\n"
          "$\"{(x)}\"|Alias for $\"{x}\"\n"
          "$\"{x.<expr>}\"|With $x being another selection expression, use ${x.operator . (<expr>)} (whitespace before and after $\".\" is ignored)\n"
          "$\"{x[<expr>]}\"|With $x being another selection expression, use ${x.operator [] (<expr>)} (whitespace before and after $\"[\" and $\"]\" is ignored)\n"
          "$\"{x[<expr>:]}\"|With $x being another selection expression, use ${x.operator [:] (<expr>,none)} (whitespace before and after $\"[\" and $\"]\" is ignored)\n"
          "$\"{x[:<expr>]}\"|With $x being another selection expression, use ${x.operator [:] (none,<expr>)} (whitespace before and after $\"[\" and $\"]\" is ignored)\n"
          "$\"{x[<expr1>:<expr2>]}\"|With $x being another selection expression, use ${x.operator [:] (<expr1>,<expr2>)} (whitespace before and after $\"[\", $\":\" and $\"]\" is ignored)\n"
          "$\"{x(<expr1>,<expr2>,[...])}\"|With $x being another selection expression, use ${x(<expr1>,<expr2>,[...])} (whitespace before and after $\"(\", $\",\" and $\")\" is ignored)\n"
          "$\"{x(<expr1>,<expr2>...)}\"|With $x being another selection expression, use ${x(<expr1>,<expr2>...)} (i.e. you're able to use expand expressions here) (whitespace before and after $\"(\", $\",\", $\"...\" and $\")\" is ignored)\n"
          "$\"{x ? <expr1> : <expr2>}\"|With $x being another selection expression, use sub-expression <expr1> if ${x.operator bool()} is true, or <expr2> otherwise (whitespace before and after $\"?\" and $\":\" is ignored)\n"
          "$\"{x ? : <expr2>}\"|Re-use $x as true-result, similar to $\"{x ? {x} : <expr2>}\" (whitespace before and after $\"?\" and $\":\" is ignored)\n"
          "$\"{x ? <expr1>}\"|Use :none as false-result\" (whitespace before and after $\"?\" is ignored)\n"
          "}\n"
          "Sub-expressions in selections strings (the ${<expr>} above). "
          "Note however that the angle brackets are not part of the syntax, "
          "but used to highlight association:\n"
          "%{table Sub-expression|Description\n"
          "$\"42\"|Evaluates to ${int(42)}\n"
          "$\"foobar\"|Evaluates to ${\"foobar\"}\n"
          "$\"{x}\"|Evaluates to the object selected by another selection expression $x}\n"
          "Once an object to-be formatted has been selected, the way in which it should "
          "then be formatted can be altered through use of spec string portion\n"
          "If a spec portion is not present, ${str selected_object} is simply appended "
          "to the resulting string. Otherwise, ${selected_object.__format__(spec_string)} "
          "is invoked, and the resulting object is appended instead\n"
          "For this purpose, :object implements a function :object.__format__ that provides "
          "some basic spec options, which are also used for types not derived from :object, "
          "or ones overwriting ${operator .}, where invocationg with $\"__format__\" throws "
          "either a :NotImplemented or :AttributeError error.\n"
          "When used, :object.__format__ provides the following functionality, with a "
          ":ValueError being thrown for anything else, or anything not matching these "
          "criteria\n"
          "%{table Spec option|Description\n"
          "$\"{:42}\"|Will append ${selected_object.operator str().ljust(42)} to the resulting string (s.a. #ljust)\n"
          "$\"{:<42}\"|Same as $\"{:42}\"\n"
          "$\"{:>42}\"|Will append ${selected_object.operator str().rjust(42)} to the resulting string (s.a. #rjust)\n"
          "$\"{:^42}\"|Will append ${selected_object.operator str().center(42)} to the resulting string (s.a. #center)\n"
          "$\"{:=42}\"|Will append ${selected_object.operator str().zfill(42)} to the resulting string (s.a. #zfill)\n"
          "$\"{:42:foo}\"|Will append ${selected_object.operator str().ljust(42,\"foo\")} to the resulting string (s.a. #ljust)\n"
          "$\"{:<42:foo}\"|Same as $\"{:42:foo}\"\n"
          "$\"{:>42:foo}\"|Will append ${selected_object.operator str().rjust(42,\"foo\")} to the resulting string (s.a. #rjust)\n"
          "$\"{:^42:foo}\"|Will append ${selected_object.operator str().center(42,\"foo\")} to the resulting string (s.a. #center)\n"
          "$\"{:=42:foo}\"|Will append ${selected_object.operator str().zfill(42,\"foo\")} to the resulting string (s.a. #zfill)}\n"
          ) },
    { "scanf", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_scanf,
      DOC("(string format)->sequence\n"
          "@throw ValueError The given @format is malformed\n"
          "@throw ValueError Conversion to an integer failed\n"
          "Scan @this string using a scanf-like format string @format\n"
          "No major changes have been made from C's scanf function, however regex-like range "
          "expressions are supported, and the returned sequence ends as soon as either @format "
          "or @this has been exhausted, or a miss-match has occurred\n"
          "Scanf command blocks are structured as ${%[*][width]pattern}\n"
          "Besides this, for convenience and better unicode integration, the following changes "
          "have been made to C's regular scanf function:\n"
          "%{table Format pattern|Yielded type|Description\n"
          "$\" \"|-|Skip any number of characters from input data for which #isspace returns :true (${r\"\\s*\"})\n"
          "$\"\\n\"|-|Skip any kind of line-feed, including $\"\\r\\n\", as well as any character for which #islf returns :true (${r\"\\n\"})\n"
          "$\"%o\"|:int|Match up to `width' characters with ${r\"[+-]*(?\\d<8)+\"} and yield the result as an octal integer\n"
          "$\"%d\"|:int|Match up to `width' characters with ${r\"[+-]*(?\\d<10)+\"} and yield the result as an decimal integer\n"
          "$\"%x\", $\"%p\"|:int|Match up to `width' characters with ${r\"[+-]*((?\\d<16)|[a-fA-F])+\"} and yield the result as a hexadecimal integer\n"
          "$\"%i\", $\"%u\"|:int|Match up to `width' characters with ${r\"[+-]*((?\\d=0)([xX](?\\d<16)+|[bB](?\\d<2)+)|(?\\d<10)+)\"} and yield the result as an integer with automatic radix\n"
          "$\"%s\"|:string|Match up to `width' characters with ${r\"\\S+\"} and return them as a string\n"
          "$\"%c\"|:string|Consume exactly `width' (see above) or one characters and return them as a string\n"
          "$\"%[...]\"|:string|Similar to the regex (s.a. #rematch) range function (e.g. $\"%[^xyz]\", $\"%[abc]\", $\"%[a-z]\", $\"%[^\\]]\")}\n"
          "Integer-width modifiers ($\"h\", $\"hh\", $\"l\", $\"ll\", $\"j\", $\"z\", "
          "$\"t\", $\"L\", $\"I\", $\"I8\", $\"I16\", $\"I32\" and $\"I64\") are ignored\n"
          ) },
    /* What about something like this?:
     * >> print "You name is $your_name, and I'm ${my_name}"
     * >>       .substitute({ .your_name = "foo", .my_name = "bar" });
     * >> print "You owe $guy $$10 dollar!"
     * >>       .substitute({ .guy = "me" });
     */


    /* String/Character traits */
    { "isprint", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isprint,
      DOC("->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all "
          "characters in ${this.substr(start,end)} are printable") },
    { "isalpha", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isalpha,
      DOC("->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all characters "
          "in ${this.substr(start,end)} are alphabetical") },
    { "isspace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isspace,
      DOC("->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all characters "
          "in ${this.substr(start,end)} are space-characters") },
    { "islf", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_islf,
      DOC("->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all characters "
          "in ${this.substr(start,end)} are line-feeds") },
    { "islower", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_islower,
      DOC("->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all characters "
          "in ${this.substr(start,end)} are lower-case") },
    { "isupper", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isupper,
      DOC("->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all characters "
          "in ${this.substr(start,end)} are upper-case") },
    { "iscntrl", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_iscntrl,
      DOC("->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all characters "
          "in ${this.substr(start,end)} are control characters") },
    { "isdigit", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isdigit,
      DOC("->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all characters "
          "in ${this.substr(start,end)} are digits") },
    { "isdecimal", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isdecimal,
      DOC("->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all characters "
          "in ${this.substr(start,end)} are dicimal characters") },
    { "issymstrt", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_issymstrt,
      DOC("->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all characters "
          "in ${this.substr(start,end)} can be used to start a symbol name") },
    { "issymcont", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_issymcont,
      DOC("->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all characters "
          "in ${this.substr(start,end)} can be used to continue a symbol name") },
    { "isalnum", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isalnum,
      DOC("->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all characters "
          "in ${this.substr(start,end)} are alpha-numerical") },
    { "isnumeric", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isnumeric,
      DOC("->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all characters "
          "in ${this.substr(start,end)} qualify as digit or decimal characters") },
    { "istitle", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_istitle,
      DOC("(int index)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if the character at ${this[index]} has title-casing\n"
          "\n"
          "->bool\n"
          "(int start=0,int end=-1)->bool\n"
          "Returns :true if $this, or the sub-string ${this.substr(start,end)} "
          "follows title-casing, meaning that space is followed by title-case, "
          "with all remaining characters not being title-case") },
    { "issymbol", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_issymbol,
      DOC("(int index)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if the character at ${this[index]} can be used to start a symbol name\n"
          "\n"
          "->bool\n"
          "(int start,int end)->bool\n"
          "Returns :true if $this, or the sub-string ${this.substr(start,end)} "
          "is a valid symbol name") },

    { "isanyprint", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanyprint,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is printable") },
    { "isanyalpha", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanyalpha,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is alphabetical") },
    { "isanyspace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanyspace,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is a space character") },
    { "isanylf", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanylf,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is a line-feeds") },
    { "isanylower", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanylower,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is lower-case") },
    { "isanyupper", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanyupper,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is upper-case") },
    { "isanycntrl", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanycntrl,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is a control character") },
    { "isanydigit", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanydigit,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is a digit") },
    { "isanydecimal", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanydecimal,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is a dicimal character") },
    { "isanysymstrt", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanysymstrt,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} can be used to start a symbol name") },
    { "isanysymcont", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanysymcont,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} can be used to continue a symbol name") },
    { "isanyalnum", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanyalnum,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is alpha-numerical") },
    { "isanynumeric", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanynumeric,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} qualifies as digit or decimal characters") },
    { "isanytitle", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_isanytitle,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} has title-casing") },

    /* String conversion */
    { "lower", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_lower,
      DOC("(int start=0,int end=-1)->string\n"
          "Returns @this string converted to lower-case") },
    { "upper", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_upper,
      DOC("(int start=0,int end=-1)->string\n"
          "Returns @this string converted to upper-case") },
    { "title", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_title,
      DOC("(int start=0,int end=-1)->string\n"
          "Returns @this string converted to title-casing") },
    { "capitalize", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_capitalize,
      DOC("(int start=0,int end=-1)->string\n"
          "Returns @this string with each word capitalized") },
    { "swapcase", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_swapcase,
      DOC("(int start=0,int end=-1)->string\n"
          "Returns @this string with the casing of each "
          "character that has two different casings swapped") },

    /* Case-sensitive query functions */
    { "replace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_replace,
      DOC("(string find_str,string replace_str,int max_count=int.SIZE_MAX)->string\n"
          "Find up to @max_count occurrances of @find_str and replace each with @replace_str, then return the resulting string") },
    { "find", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_find,
      DOC("(string needle,int start=0,int end=-1)->int\n"
          "Find the first instance of @needle within ${this.substr(start,end)}, "
          "and return its starting index, or ${-1} if no such position exists") },
    { "rfind", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rfind,
      DOC("(string needle,int start=0,int end=-1)->int\n"
          "Find the last instance of @needle within ${this.substr(start,end)}, "
          "and return its starting index, or ${-1} if no such position exists") },
    { "index", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_index,
      DOC("(string needle,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @needle can be found within ${this.substr(start,end)}\n"
          "Find the first instance of @needle within ${this.substr(start,end)}, "
          "and return its starting index") },
    { "rindex", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rindex,
      DOC("(string needle,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @needle can be found within ${this.substr(start,end)}\n"
          "Find the last instance of @needle within ${this.substr(start,end)}, "
          "and return its starting index") },
    { "findall", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_findall,
       DOC("(string needle,int start=0,int end=-1)->{int...}\n"
          "Find all instances of @needle within ${this.substr(start,end)}, "
          "and return their starting indeces as a sequence") },
    { "count", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_count,
      DOC("(string needle,int start=0,int end=-1)->int\n"
          "Count the number of instances of @needle that exist within ${this.substr(start,end)}, "
          "and return now many were found") },
    { "contains", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_contains_f,
      DOC("(string needle,int start=0,int end=-1)->bool\n"
          "Check if @needle can be found within ${this.substr(start,end)}, and return a boolean indicative of that") },
    { "substr", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_substr,
      DOC("(int start=0,int end=-1)->string\n"
          "Similar to ${this[start:end]}, however only integer-convertible objects may "
          "be passed (passing :none will invoke ${(int)none}, which results in $0), and "
          "passing negative values for either @start or @end will cause :int.SIZE_MAX to "
          "be used for that argument:\n"
          ">s = \"foo bar foobar\";\n"
          ">print repr s.substr(0,1);    /* \"f\" */\n"
          ">print repr s[0:1];           /* \"f\" */\n"
          ">print repr s.substr(0,#s);   /* \"foo bar foobar\" */\n"
          ">print repr s[0:#s];          /* \"foo bar foobar\" */\n"
          ">print repr s.substr(0,1234); /* \"foo bar foobar\" */\n"
          ">print repr s[0:1234];        /* \"foo bar foobar\" */\n"
          ">print repr s.substr(0,-1);   /* \"foo bar foobar\" -- Negative indices intentionally underflow into positive infinity */\n"
          ">print repr s[0:-1];          /* \"foo bar fooba\" */\n"
          "Also note that this way of interpreting integer indices is mirrored by all other "
          "string functions that allow start/end-style arguments, including #find, #compare, "
          "as well as many others") },
    { "strip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_strip,
      DOC("->string\n"
          "(string mask)->string\n"
          "Strip all leading and trailing whitespace-characters, or "
          "characters apart of @mask, and return the resulting string") },
    { "lstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_lstrip,
      DOC("->string\n"
          "(string mask)->string\n"
          "Strip all leading whitespace-characters, or "
          "characters apart of @mask, and return the resulting string") },
    { "rstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rstrip,
      DOC("->string\n"
          "(string mask)->string\n"
          "Strip all trailing whitespace-characters, or "
          "characters apart of @mask, and return the resulting string") },
    { "sstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_sstrip,
      DOC("(string other)->string\n"
          "Strip all leading and trailing instances of @other from @this string\n"
          ">local result = this;\n"
          ">while (result.startswith(other))\n"
          ">       result = result[#other:];\n"
          ">while (result.endswith(other))\n"
          ">       result = result[:#result-#other];\n") },
    { "lsstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_lsstrip,
      DOC("(string other)->string\n"
          "Strip all leading instances of @other from @this string\n"
          ">local result = this;\n"
          ">while (result.startswith(other))\n"
          ">       result = result[#other:];\n") },
    { "rsstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rsstrip,
      DOC("(string other)->string\n"
          "Strip all trailing instances of @other from @this string\n"
          ">local result = this;\n"
          ">while (result.endswith(other))\n"
          ">       result = result[:#result-#other];\n") },
    { "startswith", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_startswith,
      DOC("(string needle,int start=0,int end=-1)->bool\n"
          "Return :true if the sub-string ${this.substr(start,end)} starts with @other") },
    { "endswith", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_endswith,
      DOC("(string needle,int start=0,int end=-1)->bool\n"
          "Return :true if the sub-string ${this.substr(start,end)} ends with @other") },
    { "partition", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_parition,
      DOC("(string needle,int start=0,int end=-1)->(string,string,string)\n"
          "Search for the first instance of @needle within ${this.substr(start,end)} and "
          "return a 3-element sequence of strings ${(this[:pos],needle,this[pos+#needle:])}.\n"
          "If @needle could not be found, ${(this,\"\",\"\")} is returned") },
    { "rpartition", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rparition,
      DOC("(string needle,int start=0,int end=-1)->(string,string,string)\n"
          "Search for the last instance of @needle within ${this.substr(start,end)} and "
          "return a 3-element sequence of strings ${(this[:pos],needle,this[pos+#needle:])}.\n"
          "If @needle could not be found, ${(this,\"\",\"\")} is returned") },
    { "compare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_compare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Compare the sub-string ${left = this.substr(my_start,my_end)} with ${right = other.substr(other_start,other_end)}, returning "
          "${< 0} if ${left < right}, ${> 0} if ${left > right}, or ${== 0} if they are equal") },
    { "vercompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_vercompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Performs a version-string comparison. This is similar to #compare, but rather than "
          "performing a strict lexicographical comparison, the numbers found in the strings "
          "being compared are comparsed as a whole, solving the common problem seen in applications "
          "such as file navigators showing a file order of `foo1.txt', `foo10.txt', `foo11.txt', `foo2.txt', etc...\n"
          "This function is a portable implementation of the GNU function "
          "%{link https://linux.die.net/man/3/strverscmp strverscmp}, "
          "for which you may follow the link for further details") },
    { "wildcompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_wildcompare,
      DOC("(string pattern,int pattern_start=0,int pattern_end=-1)->int\n"
          "(int my_start,string pattern,int pattern_start=0,int pattern_end=-1)->int\n"
          "(int my_start,int my_end,string pattern,int pattern_start=0,int pattern_end=-1)->int\n"
          "Perform a wild-character-enabled comparising of the sub-string ${left = this.substr(my_start,my_end)} "
          "with ${right = pattern.substr(pattern_start,pattern_end)}, returning ${< 0} if ${left < right}, ${> 0} "
          "if ${left > right}, or ${== 0} if they are equal\n"
          "Wild-compare characters are only parsed from @pattern, allowing $\"?\" to "
          "be matched with any single character from @this, and $\"*\" to be matched to "
          "any number of characters") },
    { "fuzzycompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_fuzzycompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Perform a fuzzy string comparison between ${this.substr(my_start,my_end)} and ${other.substr(other_start,other_end)}\n"
          "The return value is a similarty-factor that can be used to score how close the two strings look alike.\n"
          "How exactly the scoring is done is implementation-specific, however a score of $0 is reserved for two "
          "strings that are perfectly identical, any two differing strings always have a score ${> 0}, and the closer "
          "the score is to $0, the more alike they are\n"
          "The intended use of this function is for auto-completion, as well as warning "
          "messages and recommendations in the sense of I-dont-know-foo-but-did-you-mean-bar\n"
          "Note that there is another version #casefuzzycompare that also ignores casing") },
    { "wmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_wmatch,
      DOC("(string other,int other_start=0,int other_end=-1)->bool\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->bool\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->bool\n"
          "Same as #wildcompare, returning :true where #wildcompare would return $0, and :false in all other cases") },

    /* Case-insensitive query functions */
    { "casereplace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casereplace,
      DOC("(string find_str,string replace_str,int max_count=int.SIZE_MAX)->int\n"
          "Same as #replace, however casing is ignored during character comparisons") },
    { "casefind", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casefind,
      DOC("(string needle,int start=0,int end=-1)->int\n"
          "Same as #find, however casing is ignored during character comparisons") },
    { "caserfind", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caserfind,
      DOC("(string needle,int start=0,int end=-1)->int\n"
          "Same as #rfind, however casing is ignored during character comparisons") },
    { "caseindex", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caseindex,
      DOC("(string needle,int start=0,int end=-1)->int\n"
          "Same as #index, however casing is ignored during character comparisons") },
    { "caserindex", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caserindex,
      DOC("(string needle,int start=0,int end=-1)->int\n"
          "Same as #rindex, however casing is ignored during character comparisons") },
    { "casefindall", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casefindall,
       DOC("(string needle,int start=0,int end=-1)->{int...}\n"
          "Same as #findall, however casing is ignored during character comparisons") },
    { "casecount", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casecount,
      DOC("(string needle,int start=0,int end=-1)->int\n"
          "Same as #count, however casing is ignored during character comparisons") },
    { "casecontains", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casecontains_f,
      DOC("(string needle,int start=0,int end=-1)->bool\n"
          "Same as #contains, however casing is ignored during character comparisons") },
    { "casestrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casestrip,
      DOC("->string\n"
          "(string mask)->string\n"
          "Same as #strip, however casing is ignored during character comparisons when @mask is given") },
    { "caselstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caselstrip,
      DOC("->string\n"
          "(string mask)->string\n"
          "Same as #lstrip, however casing is ignored during character comparisons when @mask is given") },
    { "caserstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caserstrip,
      DOC("->string\n"
          "(string mask)->string\n"
          "Same as #rstrip, however casing is ignored during character comparisons when @mask is given") },
    { "casesstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casesstrip,
      DOC("(string other)->string\n"
          "Same as #sstrip, however casing is ignored during character comparisons") },
    { "caselsstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caselsstrip,
      DOC("(string other)->string\n"
          "Same as #lsstrip, however casing is ignored during character comparisons") },
    { "casersstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casersstrip,
      DOC("(string other)->string\n"
          "Same as #rsstrip, however casing is ignored during character comparisons") },
    { "casestartswith", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casestartswith,
      DOC("(string other,int start=0,int end=-1)->bool\n"
          "Same as #startswith, however casing is ignored during character comparisons") },
    { "caseendswith", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caseendswith,
      DOC("(string other,int start=0,int end=-1)->bool\n"
          "Same as #endswith, however casing is ignored during character comparisons") },
    { "casepartition", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caseparition,
      DOC("(string needle,int start=0,int end=-1)->(string,string,string)\n"
          "Same as #partition, however casing is ignored during character comparisons") },
    { "caserpartition", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caserparition,
      DOC("(string needle,int start=0,int end=-1)->(string,string,string)\n"
          "Same as #rpartition, however casing is ignored during character comparisons") },
    { "casecompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casecompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Same as #compare, however casing is ignored during character comparisons") },
    { "casevercompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casevercompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Same as #vercompare, however casing is ignored during character comparisons") },
    { "casewildcompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casewildcompare,
      DOC("(string pattern,int pattern_start=0,int pattern_end=-1)->int\n"
          "(int my_start,string pattern,int pattern_start=0,int pattern_end=-1)->int\n"
          "(int my_start,int my_end,string pattern,int pattern_start=0,int pattern_end=-1)->int\n"
          "Same as #wildcompare, however casing is ignored during character comparisons") },
    { "casefuzzycompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casefuzzycompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Same as #fuzzycompare, however casing is ignored during character comparisons") },
    { "casewmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casewmatch,
      DOC("(string pattern,int pattern_start=0,int pattern_end=-1)->bool\n"
          "(int my_start,string pattern,int pattern_start=0,int pattern_end=-1)->bool\n"
          "(int my_start,int my_end,string pattern,int pattern_start=0,int pattern_end=-1)->bool\n"
          "Same as #wmatch, however casing is ignored during character comparisons") },

    /* String alignment functions. */
    { "center", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_center,
      DOC("(int width,string filler=\" \")->string\n"
          "Use @this string as result, then evenly insert @filler at "
          "the front and back to pad its length to @width characters") },
    { "ljust", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_ljust,
      DOC("(int width,string filler=\" \")->string\n"
          "Use @this string as result, then insert @filler "
          "at the back to pad its length to @width characters") },
    { "rjust", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rjust,
      DOC("(int width,string filler=\" \")->string\n"
          "Use @this string as result, then insert @filler "
          "at the front to pad its length to @width characters") },
    { "zfill", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_zfill,
      DOC("(int width,string filler=\"0\")->string\n"
          "Skip leading ${\'+\'} and ${\'-\'} characters, then insert @filler "
          "to pad the resulting string to a length of @width characters") },
    { "reversed", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_reversed,
      DOC("(int start=0,int end=-1)->string\n"
          "Return the sub-string ${this.substr(start,end)} with its character order reversed") },
    { "expandtabs", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_expandtabs,
      DOC("(int tabwidth=8)->string\n"
          "Expand tab characters with whitespace offset from the start of "
          "their respective line at multiples of @tabwidth") },
    { "unifylines", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_unifylines,
      DOC("(string replacement=\"\\n\")->string\n"
          "Unify all linefeed character sequences found in @this string to "
          "make exclusive use of @replacement") },

    /* String -- sequence interaction. */
    { "join", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_join,
      DOC("(sequence seq)->string\n"
          "Iterate @seq and convert all items into string, inserting @this "
          "string before each element, starting only with the second") },
    { "split", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_split,
      DOC("(string sep)->{string...}\n"
          "Split @this string at each instance of @sep, returning a sequence of the resulting parts") },
    { "casesplit", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casesplit,
      DOC("(string sep)->{string...}\n"
          "Same as #split, however casing is ignored during character comparisons") },
    { "splitlines", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_splitlines,
      DOC("(bool keepends=false)->{string...}\n"
          "Split @this string at each linefeed, returning a sequence of all contained lines\n"
          "When @keepends is :false, this is identical to ${this.unifylines().split(\"\\n\")}\n"
          "When @keepends is :true, items found in the returned sequence will still have their "
          "original, trailing line-feed appended") },

    /* String indentation. */
    { "indent", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_indent,
      DOC("(string filler=\"\\t\")->string\n"
          "Using @this string as result, insert @filler at the front, as well as after "
          "every linefeed with the exception of one that may be located at its end\n"
          "The inteded use is for generating strings from structured data, such as HTML:\n"
          ">text = get_html();\n"
          ">text = \"<html>\n{}\n</html>\".format({ text.strip().indent() });\n") },
    { "dedent", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_dedent,
      DOC("(int max_chars=1)->string\n"
          "(int max_chars=1,string mask)->string\n"
          "Using @this string as result, remove up to @max_chars whitespace "
          "(s.a. #isspace) characters, or if given: characters apart of @mask "
          "from the front, as well as following any linefeed") },

    /* Common-character search functions. */
    { "common", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_common,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Returns the number of common leading characters shared between @this and @other, "
          "or in other words: the lowest index $i for which ${this[i] != other[i]} is true") },
    { "rcommon", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rcommon,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Returns the number of common trailing characters shared between @this and @other") },
    { "casecommon", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casecommon,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Same as #common, however casing is ignored during character comparisons") },
    { "casercommon", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casercommon,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "Same as #rcommon, however casing is ignored during character comparisons") },

    /* Find match character sequences */
    { "findmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_findmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "Similar to #find, but do a recursive search for the "
          "first @close that doesn't have a match @{open}:\n"
          ">s = \"foo(bar(),baz(42),7).strip()\";\n"
          ">lcol = s.find(\"(\");\n"
          ">print lcol; /* 3 */\n"
          ">mtch = s.findmatch(\"(\",\")\",lcol+1);\n"
          ">print repr s[lcol:mtch+1]; /* \"(bar(),baz(42),7)\" */\n"
          "If no @close without a match @open exists, ${-1} is returned\n"
          "Note that @open and @close are not restricted to single-character "
          "strings, are allowed to be of any length") },
    { "indexmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_indexmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @close without a match @open exists within ${this.substr(start,end)}\n"
          "Same as #findmatch, but throw an :IndexError instead of "
          "returning ${-1} if no @close without a match @open exists") },
    { "casefindmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casefindmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "Same as :findmatch, however casing is ignored during character comparisons") },
    { "caseindexmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caseindexmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @close without a match @open exists within ${this.substr(start,end)}\n"
          "Same as :indexmatch, however casing is ignored during character comparisons") },
    { "rfindmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rfindmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "Similar to #findmatch, but operate in a mirrored fashion, searching for the "
          "last instance of @open that has no match @close within ${this.substr(start,end)}:\n"
          ">s = \"get_string().foo(bar(),baz(42),7).length\";\n"
          ">lcol = s.find(\")\");\n"
          ">print lcol; /* 19 */\n"
          ">mtch = s.rfindmatch(\"(\",\")\",0,lcol);\n"
          ">print repr s[mtch:lcol+1]; /* \"(bar(),baz(42),7)\" */\n"
          "If no @open without a match @close exists, ${-1} is returned") },
    { "rindexmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rindexmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @open without a match @close exists within ${this.substr(start,end)}\n"
          "Same as #rfindmatch, but throw an :IndexError instead of "
          "returning ${-1} if no @open without a match @close exists") },
    { "caserfindmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caserfindmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "Same as :rfindmatch, however casing is ignored during character comparisons") },
    { "caserindexmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caserindexmatch,
      DOC("(string open,string close,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @open without a match @close exists within ${this.substr(start,end)}\n"
          "Same as :rindexmatch, however casing is ignored during character comparisons") },

    /* Using the find-match functionality, also provide a partitioning version */
    { "partitionmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_partitionmatch,
      DOC("(string open,string close,int start=0,int end=-1)->(string,string,string)\n"
          "A hybrid between #find, #findmatch and #partition that returns the strings surrounding "
          "the matched string portion, the first being the substring prior to the match, "
          "the second being the matched string itself (including the @open and @close strings), "
          "and the third being the substring after the match:\n"
          ">s = \"foo {x,y,{13,19,42,{}},w} -- tail {}\";\n"
          ">print repr s.partitionmatch(\"{\",\"\"); /* { \"foo \", \"{x,y,{13,19,42,{}},w}\", \" -- tail {}\" } */\n"
          "If no matching @open + @close pair could be found, ${(this[start:end],\"\",\"\")} is returned\n"
          ">function partitionmatch(open, close, start = 0, end = -1) {\n"
          "> local j;\n"
          "> local i = this.find(open,start,end);\n"
          "> if (i < 0 || (j = this.findmatch(open,close,i+#open,end)) < 0)\n"
          ">  return (this.substr(start,end),\"\",\"\");\n"
          "> return (this.substr(start,i),this.substr(i,j+#close),this.substr(j+#close,end))\n"
          ">}") },
    { "rpartitionmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rpartitionmatch,
      DOC("(string open,string close,int start=0,int end=-1)->(string,string,string)\n"
          "A hybrid between #rfind, #rfindmatch and #rpartition that returns the strings surrounding "
          "the matched string portion, the first being the substring prior to the match, "
          "the second being the matched string itself (including the @open and @close strings), "
          "and the third being the substring after the match:\n"
          ">s = \"{} foo {x,y,{13,19,42,{}},w} -- tail\";\n"
          ">print repr s.rpartitionmatch(\"{\",\"\"); /* { \"{} foo \", \"{x,y,{13,19,42,{}},w}\", \" -- tail\" } */\n"
          "If no matching @open + @close pair could be found, ${(this[start:end],\"\",\"\")} is returned\n"
          ">function rpartitionmatch(open, close, start = 0, end = -1) {\n"
          ">    local i;\n"
          ">    local j = this.rfind(close,start,end);\n"
          ">    if (j < 0 || (i = this.rfindmatch(open,close,start,j)) < 0)\n"
          ">        return (this.substr(start,end),\"\",\"\");\n"
          ">    return (this.substr(start,i),this.substr(i,j+#close),this.substr(j+#close,end))\n"
          ">}") },
    { "casepartitionmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_casepartitionmatch,
      DOC("(string open,string close,int start=0,int end=-1)->(string,string,string)\n"
          "Same as #partitionmatch, however casing is ignored during character comparisons") },
    { "caserpartitionmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_caserpartitionmatch,
      DOC("(string open,string close,int start=0,int end=-1)->(string,string,string)\n"
          "Same as #rpartitionmatch, however casing is ignored during character comparisons") },

    { "segments", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_segments,
      DOC("(int count)->{string...}\n"
          "Split @this string into segments, each exactly @count characters long, with the "
          "last segment containing the remaining characters and having a length of between "
          "$1 and @count characters.") },

    /* Regex functions. */
    { "rematch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rematch,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->int\n"
          "(string pattern,string rules,int start=0,int end=-1)->int\n"
          "@throw ValueError The given @pattern is malformed\n"
          "@return The number of leading characters in ${this.substr(start,end)} "
          "matched by @pattern, or $0 if @pattern cannot be fully matched\n"
          "Check if ${this.substr(start,end)} string matches the given regular expression @pattern\n"
          "When specified, @rules must be a comma-seperated and case-insensitive string "
          "consisting of a set of the following Name-options, or a tightly packed set of the "
          "following Short-options:\n"
          "%{table Name|Short|Inline|Description\n"
          "$\"DOTALL\"|$\"s\"|$\"s\"|The $\".\" regex meta-character matches anything (including new-lines, which otherwise wouldn't be matched)\n"
          "$\"MULTILINE\"|$\"m\"|$\"m\"|Allow $\"^\" to match immediately after a line-feed, rather than just at the start of the string\n"
          "$\"NOCASE\"|$\"i\"|$\"i\"|Ignore casing when matching single characters, as well as characters in ranges (e.g.: $\"[a-z]\")}\n"
          "The builtin regular expression API for strings spans across the following functions:\n"
          "%{table Function|Non-regex Variant|Description\n"
          "#rematch|#common|Count how many character at the start of a sub-string match a regex pattern\n"
          "#refind|#find|Find the first sub-range matched by a regex pattern\n"
          "#rerfind|#rfind|Find the last sub-range matched by a regex pattern\n"
          "#reindex|#index|Same as #refind, but throws an error if not found\n"
          "#rerindex|#rindex|Same as #rerfind, but throws an error if not found\n"
          "#relocate|-|Same as #refind, but return the sub-string that was matched, rather than its indices\n"
          "#rerlocate|-|Same as #rerfind, but return the sub-string that was matched, rather than its indices\n"
          "#repartition|#partition|Same as #relocate, but return a 3-tuple of strings (before_match,match,after_match)\n"
          "#rerpartition|#rpartition|Same as #rerlocate, but return a 3-tuple of strings (before_match,match,after_match)\n"
          "#rereplace|#replace|Find and replace all sub-ranges matched by a regex pattern with a different string\n"
          "#refindall|#findall|Enumerate all sub-ranges matched by a regex pattern in ascending order\n"
          "#relocateall|-|Enumerate all sub-strings matched by a regex pattern in ascending order\n"
          "#resplit|#split|Enumerate all sub-strings matched by a regex pattern in ascending order\n"
          "#restartswith|#startswith|Check if @this string starts with a regular expression\n"
          "#reendswith|#endswith|Check if @this string ends with a regular expression\n"
          "#recontains|#contains|Check if @this stirng contains a regular expression anywhere\n"
          "#recount|#count|Count the number of occurances of a regular expression\n"
          "#restrip|#strip|Strip all leading and trailing regular expression matches\n"
          "#relstrip|#lstrip|Strip all leading regular expression matches\n"
          "#rerstrip|#rstrip|Strip all trailing regular expression matches\n"
          "}\n"
          "Deemon implements support for the following regex matching functions:\n"
          "%{table Feature|Description\n"
          "${r\".\"}|Match anything except for newlines. When $\"DOTALL\" is enabled, match anything including newlines\n"
          "${r\"^\"}|Match at the start of the string. When $\"MULTILINE\", also match at the start of lines (${r\"(?<=\\A|\\n)\"})\n"
          "${r\"$\"}|Match at the end of the string. When $\"MULTILINE\", also match at the end of lines (${r\"(?=\\Z|\\n)\"})\n"
          "${r\"\\A\"}|Match only at the start of the string\n"
          "${r\"\\Z\"}|Match only at the end of the string\n"
          "${r\"(x...|y...)\"}|Match either `x...' or `y...' (Note special behavior when repeated)\n"
          "${r\"(?...)\"}|Regex extension (see below)\n"
          "${r\"[...]\"}|Match any character apart of `...' (also accept the `a-z' notation, as well as any of the ${r\"\\...\"}) functions below\n"
          "${r\"\\d\"}|Match any character $ch with ${ch.isdigit()}\n"
          "${r\"\\D\"}|Match any character $ch with ${!ch.isdigit()}\n"
          "${r\"\\s\"}|Match any character $ch with ${ch.isspace()}\n"
          "${r\"\\S\"}|Match any character $ch with ${!ch.isspace()}\n"
          "${r\"\\w\"}|Match any character $ch with ${ch.issymstrt() || ch.issymcont()}\n"
          "${r\"\\W\"}|Match any character $ch with ${!ch.issymstrt() && !ch.issymcont()}\n"
          "${r\"\\n\"}|Match any character $ch with ${ch.islf()} (NOTE: deemon-specific extension)\n"
          "${r\"\\N\"}|Match any character $ch with ${!ch.islf()} (NOTE: deemon-specific extension)\n"
          "${r\"\\a\"}|Match the character ${string.chr(0x07)} aka $\"\\a\"\n"
          "${r\"\\b\"}|Match the character ${string.chr(0x08)} aka $\"\\b\"\n"
          "${r\"\\f\"}|Match the character ${string.chr(0x0c)} aka $\"\\f\"\n"
          "${r\"\\r\"}|Match the character ${string.chr(0x0d)} aka $\"\\r\"\n"
          "${r\"\\t\"}|Match the character ${string.chr(0x09)} aka $\"\\t\"\n"
          "${r\"\\v\"}|Match the character ${string.chr(0x0b)} aka $\"\\v\"\n"
          "${r\"\\e\"}|Match the character ${string.chr(0x1b)} aka $\"\\e\"\n"
          "${r\"\\...\"}|For anything else, match `...' exactly\n"
          "${r\"...\"}|Match the given character `...' exactly}\n"
          "Deemon implements support for the following regex repetition suffixes:\n"
          "%{table Suffix|Min|Max|Greedy\n"
          "${r\"*\"}|$0|$INF|$true\n"
          "${r\"*?\"}|$0|$INF|$false\n"
          "${r\"+\"}|$1|$INF|$true\n"
          "${r\"+?\"}|$1|$INF|$false\n"
          "${r\"?\"}|$0|$1|$true\n"
          "${r\"??\"}|$0|$1|$false\n"
          "${r\"{m}\"}|$m|$m|$true\n"
          "${r\"{m}?\"}|$m|$m|$false\n"
          "${r\"{,n}\"}|$0|$n|$true\n"
          "${r\"{,n}?\"}|$0|$n|$false\n"
          "${r\"{m,}\"}|$m|$INF|$true\n"
          "${r\"{m,}?\"}|$m|$INF|$false\n"
          "${r\"{m,n}\"}|$m|$n|$true\n"
          "${r\"{m,n}?\"}|$m|$n|$false}\n"
          "Deemon implements support for the following regex extensions (which also include some deemon-specific ones):\n"
          "%{table Extension|Description\n"
          "${r\"(?<=...)\"}|Positive look-behind assertion (Ensure that the current data position is preceded by `...')\n"
          "${r\"(?<!...)\"}|Negative look-behind assertion (Ensure that the current data position isn't preceded by `...')\n"
          "${r\"(?=...)\"}|Positive look-ahead assertion (Ensure that `...' follows the current data position)\n"
          "${r\"(?!...)\"}|Negative look-ahead assertion (Ensure that `...' doesn't follow the current data position)\n"
          "${r\"(?~ims)\"}|Set/delete regex flags (each character corresponding to the `Inline' column above). "
                          "Each rule may be prefixed by `~' in order to unset that flag. e.g. $\"(?i~m)\" or $\"(?~mi~s)\"\n"
          "${r\"(?#...)\"}|Comment\n"
          "${r\"(?\\d=7)\"}|Match any unicode digit-like character with a value equal to $7\n"
          "${r\"(?\\d>2)\"}, ${r\"(?\\d>=3)\"}|Match any unicode digit-like character with a value of at least $3\n"
          "${r\"(?\\d<7)\"}, ${r\"(?\\d<=6)\"}|Match any unicode digit-like character with a value of at most $6\n"
          "${r\"(?\\d>=2<=6)\"}, ${r\"(?\\d<=6>=2)\"}, ${r\"(?\\d>1<7)\"}, ${r\"(?\\d<7>1)\"}|Match any unicode digit-like character with a value between $2 and $6}\n"
          "In order to improve performance for repetitious data, deemon's regex implementation "
          "has a minor quirk when it comes to groups containing $\"|\" to indicate multiple variants "
          "in situations where that group is repeated more than once.\n"
          "If one of the variants is found to match the data string, instead of restarting from the "
          "beginning when checking if the variant matches more than once, the variant already matched "
          "(then called the primary variant) is re-tried first. Only if it cannot be used to match "
          "another data block will other variants be tried again in ascending order, but skipping the "
          "old primary variant. If one of the secondary variants then ends up being a match, it will "
          "become the new primary variant, and the process is repeated.\n"
          "This design decision was done on purpose in order to more efficiently match repetitious data, "
          "however when presented with multiple variants in a repeating context, it will not always be "
          "the first variant that gets matched.\n"
          ">local data = \"bar,  foobar\";\n"
          ">/* When faced by multiple variants, deemon will always\n"
          "> * preferr a variant that matched before the current:\n"
          "> *  - Available variants: r\",\", r\" f\", r\" \"\n"
          "> *  - ...\n"
          "> *  - Try variant #0\n"
          "> *    Match: \"bar,  foobar\"\n"
          "> *               ^ Variant #0\n"
          "> *    Remaining data: \"  foobar\"\n"
          "> *    Set primary variant: #0\n"
          "> *  - Try variant #0\n"
          "> *    Missmatch (Primary variant)\n"
          "> *    Search for new primary variant\n"
          "> *  - Try variant #1\n"
          "> *    Missmatch (\"  foobar\".startswith(\" f\") == false)\n"
          "> *  - Try variant #2\n"
          "> *    Match\n"
          "> *    Set primary variant: #3\n"
          "> *    Remaining data: \" foobar\"\n"
          "> *  - Try variant #2\n"
          "> *    Match\n"
          "> *    Remaining data: \"foobar\"\n"
          "> *  - Try variant #2\n"
          "> *    Missmatch (Primary variant)\n"
          "> *    Search for new primary variant\n"
          "> *  - ...\n"
          "> * -> At this point, none of the variants continue to match\n"
          "> *    and the resulting match-range doesn't include the `f'\n"
          "> *    because variant #1 (despite having a higher precedence\n"
          "> *    than variant #2) was never checked for \" foobar\"\n"
          "> * -> This quirk can only ever be a problem in badly written\n"
          "> *    expressions that contain multiple variants where a variant\n"
          "> *    with a lower index starts with the same condition as another\n"
          "> *    with a greater index\n"
          "> *    This example should really be written as r\"(,| f?)+\", which\n"
          "> *    wold never even run into this problem in the first place. */\n"
          ">print repr data.relocateall(r\"(,| f| )+\"); /* { \",  \" } */\n"
          ">\n"
          ">/* This regular expression doesn't have the quirk due to the double-parenthesis, \n"
          "> * which leaves the outter (repeating) group to only have 1 variant, which it will\n"
          "> * then always execute linearly, meaning that for each repetition, `,' is checked\n"
          "> * first, followed by ` f', and finally ` '\n"
          "> *  - ...\n"
          "> *  - Try variant #0\n"
          "> *    - Try variant #0.1\n"
          "> *    - Try variant #0.2\n"
          "> *    - Try variant #0.3\n"
          "> *  - ...\n"
          "> * -> Since The repeating group has only 1 variant, that variant\n"
          "> *    is always the primary one, and checked as a whole when being\n"
          "> *    repeated */\n"
          ">print repr data.relocateall(r\"((,| f| ))+\"); /* { \",  f\" } */") },
    { "refind", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_refind,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->(int,int)\n"
          "(string pattern,int start=0,int end=-1,string rules=\"\")->none\n"
          "(string pattern,string rules,int start=0,int end=-1)->(int,int)\n"
          "(string pattern,string rules,int start=0,int end=-1)->none\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Find the first sub-string matched by @pattern, and return its start/end indices, or :none if no match exists\n"
          "Note that using :none in an expand expression will result in whatever number of targets are required:\n"
          ">/* If the pattern count not be matched, both `start' and `end' will be `none' singleton */\n"
          ">local start,end = data.refind(r\"\\b\\w\\b\")...;\n"
          ">/* Since `none' is equal to `0' when casted to `int', calling `substr' with none-arguments\n"
          "> * is the same as calling `data.substr(0,0)', meaning that this an empty string will be\n"
          "> * returned when `refind' didn't manage to find anything.\n"
          "> * Note however that `operator [:]' functions differently, as it interprets `none' as a\n"
          "> * placeholder for either `0' of `#data', so calling `data[start:end]' would re-produce\n"
          "> * `data' itself in the event of `refind' having failed. */\n"
          ">print repr data.substr(start,end);") },
    { "rerfind", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rerfind,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->(int,int)\n"
          "(string pattern,int start=0,int end=-1,string rules=\"\")->none\n"
          "(string pattern,string rules,int start=0,int end=-1)->(int,int)\n"
          "(string pattern,string rules,int start=0,int end=-1)->none\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Find the last sub-string matched by @pattern, and return its start/end indices, or :none if no match exists (s.a. #refind)") },
    { "reindex", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_reindex,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->(int,int)\n"
          "(string pattern,string rules,int start=0,int end=-1)->(int,int)\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "@throw IndexError No substring matching the given @pattern could be found\n"
          "Same as #refind, but throw an :IndexError when no match can be found") },
    { "rerindex", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rerindex,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->(int,int)\n"
          "(string pattern,string rules,int start=0,int end=-1)->(int,int)\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "@throw IndexError No substring matching the given @pattern could be found\n"
          "Same as #rerfind, but throw an :IndexError when no match can be found") },
    { "relocate", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_relocate,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->string\n"
          "(string pattern,string rules,int start=0,int end=-1)->string\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Same as ${this.substr(this.refind(pattern,start,end,rules)...)}\n"
          "In other words: return the first sub-string matched by the "
          "given regular expression, or an empty string if not found\n"
          "This function has nothing to do with relocations! - it's pronounced R.E. locate") },
    { "rerlocate", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rerlocate,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->string\n"
          "(string pattern,string rules,int start=0,int end=-1)->string\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Same as ${this.substr(this.rerfind(pattern,start,end,rules)...)}\n"
          "In other words: return the last sub-string matched by the "
          "given regular expression, or an empty string if not found") },
    { "repartition", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_repartition,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->(string,string,string)\n"
          "(string pattern,string rules,int start=0,int end=-1)->(string,string,string)\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "A hybrid between #refind and #partition\n"
          ">function repartition(pattern,start,end,rules) {\n"
          "> local start,end = this.refind(pattern,start,end,rules)...;\n"
          "> if (start is none) return (this,\"\",\"\");\n"
          "> return (this.substr(0,start),this.substr(start,end),this.substr(end,-1));\n"
          ">}") },
    { "rerpartition", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rerpartition,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->(string,string,string)\n"
          "(string pattern,string rules,int start=0,int end=-1)->(string,string,string)\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "A hybrid between #rrefind and #rpartition\n"
          ">function rerpartition(pattern,start,end,rules) {\n"
          "> local start,end = this.rerfind(pattern,start,end,rules)...;\n"
          "> if (start is none) return (this,\"\",\"\");\n"
          "> return (this.substr(0,start),this.substr(start,end),this.substr(end,-1));\n"
          ">}") },
    { "rereplace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rereplace,
      DOC("(string pattern,string replace_str,int max_count=int.SIZE_MAX,string rules=\"\")->string\n"
          "(string pattern,string replace_str,string rules=\"\",int max_count=int.SIZE_MAX)->string\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Similar to #replace, however the string to search for is implemented as a regular expression "
          "pattern, with the sub-string matched by it then getting replaced by @replace_str") },
    { "refindall", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_refindall,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->{(int,int)...}\n"
          "(string pattern,string rules,int start=0,int end=-1)->{(int,int)...}\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Similar to #refind, but return a sequence of all matches found within ${this.substr(start,end)}\n"
          "Note that the matches returned are ordered ascendingly") },
    { "relocateall", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_relocateall,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->{string...}\n"
          "(string pattern,string rules,int start=0,int end=-1)->{string...}\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Similar to #relocate, but return a sequence of all matched "
          "sub-strings found within ${this.substr(start,end)}\n"
          "Note that the matches returned are ordered ascendingly\n"
          "This function has nothing to do with relocations! - it's pronounced R.E. locate all") },
    { "resplit", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_resplit,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->{string...}\n"
          "(string pattern,string rules,int start=0,int end=-1)->{string...}\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Similar to #split, but use a regular expression in order to "
          "express the sections of the string around which to perform the split\n"
          ">local data = \"10 , 20,30 40, 50\";\n"
          ">for (local x: data.resplit(\"(\\\\s*,?)+\\\\s*\"))\n"
          "> print x; /* `10' `20' `30' `40' `50' */\n"
          "If you wish to do the reverse and enumerate matches, rather than the "
          "strings between matches, use #relocateall instead, which also behaves "
          "as a sequence") },
    { "restartswith", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_restartswith,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->bool\n"
          "(string pattern,string rules,int start=0,int end=-1)->bool\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Check if @this string starts with a regular expression described by @pattern (s.a. #startswith)\n"
          ">function restartswith(pattern) {\n"
          "> return this.rematch(pattern) != 0;\n"
          ">}") },
    { "reendswith", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_reendswith,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->bool\n"
          "(string pattern,string rules,int start=0,int end=-1)->bool\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Check if @this string ends with a regular expression described by @pattern (s.a. #endswith)\n"
          ">function restartswith(pattern) {\n"
          "> local rpos = this.rerfind(pattern);\n"
          "> return rpos !is none && rpos[1] == #this;\n"
          ">}") },
    { "restrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_restrip,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->string\n"
          "(string pattern,string rules,int start=0,int end=-1)->string\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Strip all leading and trailing matches for @pattern from @this string and return the result (s.a. #strip)") },
    { "relstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_relstrip,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->string\n"
          "(string pattern,string rules,int start=0,int end=-1)->string\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Strip all leading matches for @pattern from @this string and return the result (s.a. #lstrip)") },
    { "rerstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_rerstrip,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->string\n"
          "(string pattern,string rules,int start=0,int end=-1)->string\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Strip all trailing matches for @pattern from @this string and return the result (s.a. #lstrip)") },
    { "recount", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_recount,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->int\n"
          "(string pattern,string rules,int start=0,int end=-1)->int\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Count the number of matches of a given regular expression @pattern (s.a. #count)\n"
          "Hint: This is the same as ${#this.refindall(pattern)} or ${#this.relocateall(pattern)}") },
    { "recontains", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_recontains,
      DOC("(string pattern,int start=0,int end=-1,string rules=\"\")->int\n"
          "(string pattern,string rules,int start=0,int end=-1)->int\n"
          "@param pattern The regular expression patterm (s.a. #rematch)\n"
          "@param rules The regular expression rules (s.a. #rematch)\n"
          "@throw ValueError The given @pattern is malformed\n"
          "Check if @this contains a match for the given regular expression @pattern (s.a. #contains)\n"
          "Hint: This is the same as ${!!this.refindall(pattern)} or ${!!this.relocateall(pattern)}") },


    /* String optimizations for standard sequence functions. */
    { "front", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_front },
    { "back", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_back },

    /* Deprecated functions. */
    { "reverse", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_reversed,
      DOC("(int start=0,int end=-1)->string\nDeprecated alias for #reversed") },
    { NULL }
};

PRIVATE DREF String *DCALL
string_cat(String *__restrict self, DeeObject *__restrict other) {
 /* Simple case: `self' is an empty string, so just cast `other' into a string. */
 if (DeeString_IsEmpty(self))
     return (DREF String *)DeeObject_Str(other);
 if (DeeString_Check(other)) {
  /* In the likely case of `other' also being a string, we can
   * try to perform some optimizations by looking that the common,
   * required character width, and creating the resulting string in
   * accordance to what _it_ requires (bypassing the need of for printer). */
  struct string_utf *lhs_utf;
  struct string_utf *rhs_utf;
  /* Simple case: `other' is an empty string, so just re-use `self'. */
  if (DeeString_IsEmpty(other))
      return_reference_(self);
  lhs_utf = self->s_data;
  rhs_utf = ((String *)other)->s_data;
  if (!lhs_utf || lhs_utf->u_width == STRING_WIDTH_1BYTE) {
   if (!rhs_utf || rhs_utf->u_width == STRING_WIDTH_1BYTE) {
    DREF String *result;
    size_t total_length = self->s_len + DeeString_SIZE(other);
    /* Most likely case: both strings use 1-byte characters,
     * so we don't even need to use a multi-byte buffer! */
    result = (DREF String *)DeeObject_Malloc(COMPILER_OFFSETOF(String,s_str)+
                                            (total_length+1)*sizeof(char));
    if unlikely(!result) goto err;
    result->s_len = total_length;
    /* Copy characters into the resulting string. */
    memcpy(result->s_str,self->s_str,self->s_len*sizeof(char));
    memcpy(result->s_str+self->s_len,
           DeeString_STR(other),
           DeeString_SIZE(other)*sizeof(char));
    /* finalize the resulting string. */
    result->s_str[total_length] = '\0';
    result->s_hash = DEE_STRING_HASH_UNSET;
    result->s_data = NULL;
    DeeObject_Init(result,&DeeString_Type);
    return result;
   }
  } else if (rhs_utf && rhs_utf->u_width != STRING_WIDTH_1BYTE) {
   /* >> 2/4-byte + 2/4-byte
    * This case we can optimize as well, because both the left,
    * as well as the right string already feature their UTF-8
    * representations, meaning that while we will have to generate
    * a 2/4-byte string, we don't have to painfully convert that
    * string into UTF-8, since we can simply generate the UTF-8
    * sequence by concat-ing the 2 we already got! */
   DREF String *result;
   struct string_utf *result_utf;
   size_t total_length = self->s_len + DeeString_SIZE(other);
   /* Most likely case: both strings use 1-byte characters,
    * so we don't even need to use a multi-byte buffer! */
   result = (DREF String *)DeeObject_Malloc(COMPILER_OFFSETOF(String,s_str)+
                                           (total_length+1)*sizeof(char));
   if unlikely(!result) goto err;
   result_utf = string_utf_alloc();
   if unlikely(!result_utf) goto err_r_2_4;
   memset(result_utf,0,sizeof(struct string_utf));

   /* Determine the common width of the left and right string,
    * then construct a 16-bit, or 32-bit character string. */
   ASSERT(lhs_utf->u_width == STRING_WIDTH_2BYTE || lhs_utf->u_width == STRING_WIDTH_4BYTE);
   ASSERT(rhs_utf->u_width == STRING_WIDTH_2BYTE || rhs_utf->u_width == STRING_WIDTH_4BYTE);
   if (lhs_utf->u_width == STRING_WIDTH_2BYTE) {
    if (rhs_utf->u_width == STRING_WIDTH_2BYTE) {
     /* 2-byte + 2-byte --> 2-byte */
     uint16_t *result_string; size_t lhs_len,rhs_len;
     uint16_t *lhs_str = (uint16_t *)lhs_utf->u_data[STRING_WIDTH_2BYTE];
     uint16_t *rhs_str = (uint16_t *)rhs_utf->u_data[STRING_WIDTH_2BYTE];
     lhs_len = WSTR_LENGTH(lhs_str);
     rhs_len = WSTR_LENGTH(rhs_str);
     result_string = DeeString_NewBuffer16(lhs_len+rhs_len);
     if unlikely(!result_string) goto err_r_2_4_utf;
     result_string[lhs_len+rhs_len] = 0;
     memcpyw(result_string,lhs_str,lhs_len);
     memcpyw(result_string+lhs_len,rhs_str,rhs_len);
     result_utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)result_string;
     result_utf->u_width = STRING_WIDTH_2BYTE;
    } else {
     /* 2-byte + 4-byte --> 4-byte */
     uint32_t *result_string; size_t i,lhs_len,rhs_len;
     uint16_t *lhs_str = (uint16_t *)lhs_utf->u_data[STRING_WIDTH_2BYTE];
     uint32_t *rhs_str = (uint32_t *)rhs_utf->u_data[STRING_WIDTH_4BYTE];
     lhs_len = WSTR_LENGTH(lhs_str);
     rhs_len = WSTR_LENGTH(rhs_str);
     result_string = DeeString_NewBuffer32(lhs_len+rhs_len);
     if unlikely(!result_string) goto err_r_2_4_utf;
     result_string[lhs_len+rhs_len] = 0;
     for (i = 0; i < lhs_len; ++i)
         result_string[i] = lhs_str[i];
     memcpyl(result_string+lhs_len,rhs_str,rhs_len);
     result_utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)result_string;
     result_utf->u_width = STRING_WIDTH_4BYTE;
    }
   } else {
    if (rhs_utf->u_width == STRING_WIDTH_2BYTE) {
     /* 4-byte + 2-byte --> 4-byte */
     uint32_t *result_string; size_t i,rhs_len,lhs_len;
     uint32_t *lhs_str = (uint32_t *)lhs_utf->u_data[STRING_WIDTH_4BYTE];
     uint16_t *rhs_str = (uint16_t *)rhs_utf->u_data[STRING_WIDTH_2BYTE];
     lhs_len = WSTR_LENGTH(lhs_str);
     rhs_len = WSTR_LENGTH(rhs_str);
     result_string = DeeString_NewBuffer32(lhs_len+rhs_len);
     if unlikely(!result_string) goto err_r_2_4_utf;
     result_string[lhs_len+rhs_len] = 0;
     memcpyl(result_string,lhs_str,lhs_len);
     for (i = 0; i < rhs_len; ++i)
         result_string[lhs_len+i] = rhs_str[i];
     result_utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)result_string;
     result_utf->u_width = STRING_WIDTH_4BYTE;
    } else {
     /* 4-byte + 4-byte --> 4-byte */
     uint32_t *result_string; size_t rhs_len,lhs_len;
     uint16_t *lhs_str = (uint16_t *)lhs_utf->u_data[STRING_WIDTH_4BYTE];
     uint32_t *rhs_str = (uint32_t *)rhs_utf->u_data[STRING_WIDTH_4BYTE];
     lhs_len = WSTR_LENGTH(lhs_str);
     rhs_len = WSTR_LENGTH(rhs_str);
     result_string = DeeString_NewBuffer32(lhs_len+rhs_len);
     if unlikely(!result_string) goto err_r_2_4_utf;
     result_string[lhs_len+rhs_len] = 0;
     memcpyl(result_string,lhs_str,lhs_len);
     memcpyl(result_string+lhs_len,rhs_str,rhs_len);
     result_utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)result_string;
     result_utf->u_width = STRING_WIDTH_4BYTE;
    }
   }
   result->s_len = total_length;
   /* Copy characters into the resulting string. */
   memcpy(result->s_str,self->s_str,self->s_len*sizeof(char));
   memcpy(result->s_str+self->s_len,
          DeeString_STR(other),
          DeeString_SIZE(other)*sizeof(char));
   /* finalize the resulting string. */
   result->s_str[total_length] = '\0';
   result->s_hash = DEE_STRING_HASH_UNSET;
   if (lhs_utf->u_data[STRING_WIDTH_1BYTE] == (size_t *)DeeString_STR(self) &&
       rhs_utf->u_data[STRING_WIDTH_1BYTE] == (size_t *)DeeString_STR(other))
       result_utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
   result->s_data = result_utf;
   DeeObject_Init(result,&DeeString_Type);
   return result;
err_r_2_4_utf:
   string_utf_free(result_utf);
err_r_2_4:
   DeeObject_Free(result);
   return NULL;
  }
 }
 /* Fallback: use a string printer to append `other' to a copy of `self'. */
 {
  struct unicode_printer printer = UNICODE_PRINTER_INIT;
  /* Print our own string. */
  if unlikely(unicode_printer_printstring(&printer,(DeeObject *)self) < 0)
     goto err_printer;
  /* Print the other object (as a string). */
  if unlikely(DeeObject_Print(other,(dformatprinter)&unicode_printer_print,&printer) < 0)
     goto err_printer;
  return (DREF String *)unicode_printer_pack(&printer);
err_printer:
  unicode_printer_fini(&printer);
 }
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_mul(String *__restrict self, DeeObject *__restrict other) {
 size_t my_length,total_length,repeat; unsigned int width;
 if (DeeObject_AsSize(other,&repeat))
     return NULL;
 if (DeeString_IsEmpty(self) || !repeat)
     return_empty_string;
 if (repeat == 1)
     return_reference_((DeeObject *)self);
 width = DeeString_WIDTH(self);
 SWITCH_SIZEOF_WIDTH(width) {
 {
  DREF DeeObject *result;
  uint8_t *dst,*src;
 CASE_WIDTH_1BYTE:
  my_length = DeeString_SIZE(self);
  total_length = my_length*repeat;
  if unlikely(total_length < my_length ||
              total_length < repeat)
     goto err_overflow;
  result = DeeString_NewBuffer(total_length);
  if unlikely(!result) goto err;
  src = (uint8_t *)DeeString_STR(self);
  dst = (uint8_t *)DeeString_STR(result);
  while (repeat--) {
   memcpyb(dst,src,my_length);
   dst += my_length;
  }
  return result;
 } break;
 {
  uint16_t *dst,*src,*str;
 CASE_WIDTH_2BYTE:
  src = DeeString_Get2Byte((DeeObject *)self);
  my_length = WSTR_LENGTH(src);
  total_length = my_length*repeat;
  if unlikely(total_length < my_length ||
              total_length < repeat)
     goto err_overflow;
  dst = str = DeeString_NewBuffer16(total_length);
  if unlikely(!str) goto err;
  while (repeat--) {
   memcpyw(dst,src,my_length);
   dst += my_length;
  }
  return DeeString_Pack2ByteBuffer(str);
 } break;
 {
  uint32_t *dst,*src,*str;
 CASE_WIDTH_4BYTE:
  src = DeeString_Get4Byte((DeeObject *)self);
  my_length = WSTR_LENGTH(src);
  total_length = my_length*repeat;
  if unlikely(total_length < my_length ||
              total_length < repeat)
     goto err_overflow;
  dst = str = DeeString_NewBuffer32(total_length);
  if unlikely(!str) goto err;
  while (repeat--) {
   memcpyl(dst,src,my_length);
   dst += my_length;
  }
  return DeeString_Pack4ByteBuffer(str);
 } break;
 }
err_overflow:
 err_integer_overflow_i(sizeof(size_t)*8,true);
err:
 return NULL;
}


INTDEF dssize_t DCALL DeeString_CFormat(dformatprinter printer,
                                        dformatprinter format_printer, void *arg,
                                        /*utf-8*/char const *__restrict format, size_t format_len,
                                        size_t argc, DeeObject **__restrict argv);

PRIVATE DREF String *DCALL
string_mod(String *__restrict self, DeeObject *__restrict args) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 DeeObject **argv; size_t argc; char *format_str;
 /* C-style string formating */
 if (DeeTuple_Check(args)) {
  argv = DeeTuple_ELEM(args);
  argc = DeeTuple_SIZE(args);
 } else {
  argv = (DeeObject **)&args;
  argc = 1;
 }
 format_str = DeeString_AsUtf8((DeeObject *)self);
 if unlikely(!format_str) goto err;
 if unlikely(DeeString_CFormat((dformatprinter)&unicode_printer_print,
                               (dformatprinter)&unicode_printer_print,
                               &printer,
                                format_str,
                                WSTR_LENGTH(format_str),
                                argc,
                                argv) < 0)
    goto err;
 return (DREF String *)unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}

INTERN struct type_math string_math = {
    /* .tp_int32  = */NULL,
    /* .tp_int64  = */NULL,
    /* .tp_double = */NULL,
    /* .tp_int    = */NULL,
    /* .tp_inv    = */NULL,
    /* .tp_pos    = */NULL,
    /* .tp_neg    = */NULL,
    /* .tp_add    = */(DREF DeeObject *(DCALL*)(DeeObject *__restrict,DeeObject *__restrict))&string_cat,
    /* .tp_sub    = */NULL,
    /* .tp_mul    = */(DREF DeeObject *(DCALL*)(DeeObject *__restrict,DeeObject *__restrict))&string_mul,
    /* .tp_div    = */NULL,
    /* .tp_mod    = */(DREF DeeObject *(DCALL*)(DeeObject *__restrict,DeeObject *__restrict))&string_mod,
    /* .tp_shl    = */NULL,
    /* .tp_shr    = */NULL,
    /* .tp_and    = */NULL,
    /* .tp_or     = */NULL,
    /* .tp_xor    = */NULL,
    /* .tp_pow    = */NULL
};


INTERN bool DCALL
string_eq_bytes(String *__restrict self,
                DeeBytesObject *__restrict other) {
 union dcharptr my_str; uint8_t *bytes_data;
 size_t bytes_size;
 bytes_data = DeeBytes_DATA(other);
 bytes_size = DeeBytes_SIZE(other);
 SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
 CASE_WIDTH_1BYTE:
  my_str.cp8 = DeeString_Get1Byte((DeeObject *)self);
  if (WSTR_LENGTH(my_str.cp8) != bytes_size)
      break;
  return MEMEQB(my_str.cp8,bytes_data,bytes_size);
 CASE_WIDTH_2BYTE:
  my_str.cp16 = DeeString_Get2Byte((DeeObject *)self);
  if (bytes_size != WSTR_LENGTH(my_str.cp16)) break;
  while (bytes_size--) {
   if (my_str.cp16[bytes_size] != (uint16_t)(bytes_data[bytes_size]))
       goto nope;
  }
  return true;
 CASE_WIDTH_4BYTE:
  my_str.cp32 = DeeString_Get4Byte((DeeObject *)self);
  if (bytes_size != WSTR_LENGTH(my_str.cp32)) break;
  while (bytes_size--) {
   if (my_str.cp32[bytes_size] != (uint32_t)(bytes_data[bytes_size]))
       goto nope;
  }
  return true;
 }
nope:
 return false;
}


INTERN DREF DeeObject *DCALL
string_contains(String *__restrict self,
                DeeObject *__restrict some_object) {
 void *me,*other,*ptr; int width;
 if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
     return NULL;
 /* Search for an occurrence of `some_object' */
 width   = DeeString_WIDTH(self);
 me    = DeeString_WSTR(self);
 other = DeeString_AsWidth(some_object,width);
 if unlikely(!other) return NULL;
 SWITCH_SIZEOF_WIDTH(width) {
 default:
  ptr = memmemb((uint8_t *)me,WSTR_LENGTH(me),(uint8_t *)other,WSTR_LENGTH(other));
  break;
 CASE_WIDTH_2BYTE:
  ptr = memmemw((uint16_t *)me,WSTR_LENGTH(me),(uint16_t *)other,WSTR_LENGTH(other));
  break;
 CASE_WIDTH_4BYTE:
  ptr = memmeml((uint32_t *)me,WSTR_LENGTH(me),(uint32_t *)other,WSTR_LENGTH(other));
  break;
 }
 return_bool_(ptr != NULL);
}


PUBLIC dssize_t DCALL
unicode_printer_memchr(struct unicode_printer *__restrict self,
                       uint32_t chr, size_t start, size_t length) {
 void *ptr,*str = self->up_buffer; size_t result;
 ASSERT(start+length <= (str ? WSTR_LENGTH(str) : 0));
 if unlikely(!str) goto not_found;
 SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {
 CASE_WIDTH_1BYTE:
  if (chr > 0xff) goto not_found;
  ptr = memchrb((uint8_t *)str + start,
                (uint8_t)chr,length);
  if (!ptr) goto not_found;
  result = (size_t)((uint8_t *)ptr - (uint8_t *)str);
  break;
 CASE_WIDTH_2BYTE:
  if (chr > 0xffff) goto not_found;
  ptr = memchrw((uint16_t *)str + start,
                (uint16_t)chr,length);
  if (!ptr) goto not_found;
  result = (size_t)((uint16_t *)ptr - (uint16_t *)str);
  break;
 CASE_WIDTH_4BYTE:
  ptr = memchrl((uint32_t *)str + start,
                (uint32_t)chr,length);
  if (!ptr) goto not_found;
  result = (size_t)((uint32_t *)ptr - (uint32_t *)str);
  break;
 }
 return (dssize_t)result;
not_found:
 return -1;
}
PUBLIC dssize_t DCALL
unicode_printer_memrchr(struct unicode_printer *__restrict self,
                        uint32_t chr, size_t start, size_t length) {
 void *ptr,*str = self->up_buffer; size_t result;
 ASSERT(start+length <= (str ? WSTR_LENGTH(str) : 0));
 if unlikely(!str) goto not_found;
 SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {
 CASE_WIDTH_1BYTE:
  if (chr > 0xff) goto not_found;
  ptr = memrchrb((uint8_t *)str + start,
                 (uint8_t)chr,length);
  if (!ptr) goto not_found;
  result = (size_t)((uint8_t *)ptr - (uint8_t *)str);
  break;
 CASE_WIDTH_2BYTE:
  if (chr > 0xffff) goto not_found;
  ptr = memrchrw((uint16_t *)str + start,
                 (uint16_t)chr,length);
  if (!ptr) goto not_found;
  result = (size_t)((uint16_t *)ptr - (uint16_t *)str);
  break;
 CASE_WIDTH_4BYTE:
  ptr = memrchrl((uint32_t *)str + start,
                 (uint32_t)chr,length);
  if (!ptr) goto not_found;
  result = (size_t)((uint32_t *)ptr - (uint32_t *)str);
  break;
 }
 return (dssize_t)result;
not_found:
 return -1;
}

DECL_END

#ifndef __INTELLISENSE__
#include "ordinals.c.inl"
#include "split.c.inl"
#include "segments.c.inl"
#include "reproxy.c.inl"
#include "finder.c.inl"

/* Include this last! */
#include "bytes_functions.c.inl"
#endif

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_C */
