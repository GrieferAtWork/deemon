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
#ifndef GUARD_DEEMON_OBJECTS_STRING_C
#define GUARD_DEEMON_OBJECTS_STRING_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1 /* memmem() */

#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/thread.h>
#include <deemon/format.h>
#include <deemon/bytes.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/seq.h>
#include <deemon/arg.h>
#include <deemon/int.h>
#include <deemon/util/cache.h>

#include <hybrid/minmax.h>

#include <stddef.h>
#include <string.h>

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

#ifndef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX  __SSIZE_MAX__
#endif


DECL_BEGIN

DEFINE_STRUCT_CACHE(utf,struct string_utf,256)
typedef DeeStringObject String;

PUBLIC struct string_utf *
(DCALL string_utf_alloc)(void) {
 return utf_alloc();
}
#ifndef NDEBUG
PUBLIC struct string_utf *
(DCALL string_utf_alloc_d)(char const *file,
                           int line) {
 return utf_dbgalloc(file,line);
}
#else
PUBLIC struct string_utf *
(DCALL string_utf_alloc_d)(char const *UNUSED(file),
                           int UNUSED(line)) {
 return utf_alloc();
}
#endif
PUBLIC void
(DCALL string_utf_free)(struct string_utf *__restrict self) {
 utf_free(self);
}

#ifndef __USE_GNU
#define memmem  dee_memmem
LOCAL void *dee_memmem(void const *__restrict haystack, size_t haystack_len,
                       void const *__restrict needle, size_t needle_len) {
 void const *candidate; uint8_t marker;
 if unlikely(!needle_len || needle_len > haystack_len)
    return NULL;
 haystack_len -= needle_len,marker = *(uint8_t *)needle;
 while ((candidate = memchr(haystack,marker,haystack_len)) != NULL) {
  if (memcmp(candidate,needle,needle_len) == 0)
      return (void *)candidate;
  haystack_len = ((uintptr_t)haystack+haystack_len)-((uintptr_t)candidate)+1;
  haystack     = (void const *)((uintptr_t)candidate+1);
 }
 return NULL;
}
#endif /* !__USE_GNU */

PUBLIC void
(DCALL ascii_printer_release)(struct ascii_printer *__restrict self, size_t datalen) {
 ASSERT(self);
 ASSERT(self->ap_length >= datalen);
 /* This's actually all that needs to be
  * done with the current implementation. */
 self->ap_length -= datalen;
}
PUBLIC char *
(DCALL ascii_printer_alloc)(struct ascii_printer *__restrict self, size_t datalen) {
 String *string;
 size_t alloc_size;
 char *result;
 ASSERT(self);
 if ((string = self->ap_string) == NULL) {
  /* Make sure not to allocate a string when the used length remains ZERO.
   * >> Must be done to assure the expectation of `if(ap_length == 0) ap_string == NULL' */
  if unlikely(!datalen) return 0;
  /* Allocate the initial string. */
  alloc_size = 8;
  while (alloc_size < datalen) alloc_size *= 2;
alloc_again:
  string = (String *)DeeObject_TryMalloc(offsetof(String,s_str)+
                                        (alloc_size+1)*sizeof(char));
  if unlikely(!string) {
   if (alloc_size != datalen) { alloc_size = datalen; goto alloc_again; }
   if (Dee_CollectMemory(offsetof(String,s_str)+
                        (alloc_size+1)*sizeof(char))) goto alloc_again;
   return NULL;
  }
  self->ap_string = string;
  string->s_len = alloc_size;
  self->ap_length = datalen;
  return string->s_str;
 }
 alloc_size = string->s_len;
 ASSERT(alloc_size >= self->ap_length);
 alloc_size -= self->ap_length;
 if unlikely(alloc_size < datalen) {
  size_t min_alloc = self->ap_length+datalen;
  alloc_size = (min_alloc+63) & ~63;
realloc_again:
  string = (String *)DeeObject_TryRealloc(string,offsetof(String,s_str)+
                                         (alloc_size+1)*sizeof(char));
  if unlikely(!string) {
   string = self->ap_string;
   if (alloc_size != min_alloc) { alloc_size = min_alloc; goto realloc_again; }
   if (Dee_CollectMemory(offsetof(String,s_str)+
                        (alloc_size+1)*sizeof(char)))
       goto realloc_again;
   return NULL;
  }
  self->ap_string = string;
  string->s_len = alloc_size;
 }
 /* Append text at the end. */
 result = string->s_str+self->ap_length;
 self->ap_length += datalen;
 return result;
}
PUBLIC int
(DCALL ascii_printer_putc)(struct ascii_printer *__restrict self, char ch) {
 ASSERT(self);
 /* Quick check: Can we print to an existing buffer. */
 if (self->ap_string &&
     self->ap_length < self->ap_string->s_len) {
  self->ap_string->s_str[self->ap_length++] = ch;
  goto done;
 }
 /* Fallback: go the long route. */
 if (ascii_printer_print(self,&ch,1) < 0)
     goto err;
done:
 return 0;
err:
 return -1;
}
PUBLIC dssize_t
(DCALL ascii_printer_print)(struct ascii_printer *__restrict self,
                            char const *__restrict data,
                            size_t datalen) {
 String *string;
 size_t alloc_size;
 ASSERT(self);
 ASSERT(data || !datalen);
 if ((string = self->ap_string) == NULL) {
  /* Make sure not to allocate a string when the used length remains ZERO.
   * >> Must be done to assure the expectation of `if(ap_length == 0) ap_string == NULL' */
  if unlikely(!datalen) return 0;
  /* Allocate the initial string. */
  alloc_size = 8;
  while (alloc_size < datalen) alloc_size *= 2;
alloc_again:
  string = (String *)DeeObject_TryMalloc(offsetof(String,s_str)+
                                        (alloc_size+1)*sizeof(char));
  if unlikely(!string) {
   if (alloc_size != datalen) { alloc_size = datalen; goto alloc_again; }
   if (Dee_CollectMemory(offsetof(String,s_str)+
                        (alloc_size+1)*sizeof(char))) goto alloc_again;
   return -1;
  }
  self->ap_string = string;
  string->s_len = alloc_size;
  memcpy(string->s_str,data,datalen*sizeof(char));
  self->ap_length = datalen;
  goto done;
 }
 alloc_size = string->s_len;
 ASSERT(alloc_size >= self->ap_length);
 alloc_size -= self->ap_length;
 if unlikely(alloc_size < datalen) {
  size_t min_alloc = self->ap_length+datalen;
  alloc_size = (min_alloc+63) & ~63;
realloc_again:
  string = (String *)DeeObject_TryRealloc(string,offsetof(String,s_str)+
                                         (alloc_size+1)*sizeof(char));
  if unlikely(!string) {
   string = self->ap_string;
   if (alloc_size != min_alloc) { alloc_size = min_alloc; goto realloc_again; }
   if (Dee_CollectMemory(offsetof(String,s_str)+
                        (alloc_size+1)*sizeof(char)))
       goto realloc_again;
   return -1;
  }
  self->ap_string = string;
  string->s_len = alloc_size;
 }
 /* Copy text into the dynamic string. */
 /*DEE_DPRINTF("PRINT: %IX - `%.*s'\n",datalen,(int)datalen,data);*/
 memcpy(string->s_str+self->ap_length,
        data,datalen*sizeof(char));
 self->ap_length += datalen;
done:
 return (dssize_t)datalen;
}

PUBLIC DREF DeeObject *
(DCALL ascii_printer_pack)(struct ascii_printer *__restrict self) {
 DREF String *result = self->ap_string;
 if unlikely(!result) return_reference_(Dee_EmptyString);
 /* Deallocate unused memory. */
 if likely(self->ap_length != result->s_len) {
  DREF String *reloc;
  reloc = (DREF String *)DeeObject_TryRealloc(result,offsetof(String,s_str)+
                                             (self->ap_length+1)*sizeof(char));
  if likely(reloc) result = reloc;
  result->s_len = self->ap_length;
 }
 /* Make sure to terminate the c-string representation. */
 result->s_str[self->ap_length] = '\0';
 /* Do final object initialization. */
 DeeObject_Init(result,&DeeString_Type);
 result->s_hash = (dhash_t)-1;
 result->s_data = NULL;
#ifndef NDEBUG
 memset(self,0xcc,sizeof(*self));
#endif
 return (DREF DeeObject *)result;
}

PUBLIC char *
(DCALL ascii_printer_allocstr)(struct ascii_printer *__restrict self,
                               char const *__restrict str, size_t length) {
 char *result; dssize_t error;
 if (self->ap_string) {
  result = (char *)memmem(self->ap_string->s_str,
                          self->ap_length,str,length);
  if (result) return result;
 }
 /* Append a new string. */
 error = ascii_printer_print(self,str,length);
 if unlikely(error < 0) return NULL;
 ASSERT(self->ap_string);
 ASSERT(self->ap_length >= length);
 return self->ap_string->s_str+(self->ap_length-length);
}

STATIC_ASSERT(STRING_WIDTH_1BYTE < 1);

PUBLIC DREF DeeObject *DCALL
DeeString_ResizeBuffer(DREF DeeObject *self, size_t num_bytes) {
 DREF String *result;
 if unlikely(self == Dee_EmptyString) self = NULL;
 ASSERTF(!self || DeeString_Check(self),"Not a string buffer");
 ASSERTF(!self || !DeeObject_IsShared(self),"String buffers cannot be shared");
 /* Special case: Resize-to-zero. */
 if unlikely(!num_bytes) {
  if (self) Dee_DecrefDokill(self);
  Dee_Incref(Dee_EmptyString);
  return Dee_EmptyString;
 }
 /* Re-allocate the buffer. */
 result = (DREF String *)DeeObject_Realloc(self,offsetof(String,s_str)+
                                          (num_bytes+1)*sizeof(char));
 if likely(result) {
  if (!self) {
   /* Do the initial init when `self' was `NULL'. */
   DeeObject_Init(result,&DeeString_Type);
   result->s_data = NULL;
   result->s_hash = (dhash_t)-1;
  }
  result->s_len         = num_bytes;
  result->s_str[num_bytes] = '\0';
 }
 return (DREF DeeObject *)result;
}
PUBLIC DREF DeeObject *DCALL
DeeString_TryResizeBuffer(DREF DeeObject *self, size_t num_bytes) {
 DREF String *result;
 if unlikely(self == Dee_EmptyString) self = NULL;
 ASSERTF(!self || DeeString_Check(self),"Not a string buffer");
 ASSERTF(!self || !DeeObject_IsShared(self),"String buffers cannot be shared");
 /* Special case: Resize-to-zero. */
 if unlikely(!num_bytes) {
  if (self) Dee_DecrefDokill(self);
  Dee_Incref(Dee_EmptyString);
  return Dee_EmptyString;
 }
 /* Re-allocate the buffer. */
 result = (DREF String *)DeeObject_TryRealloc(self,offsetof(String,s_str)+
                                             (num_bytes+1)*sizeof(char));
 if likely(result) {
  if (!self) {
   /* Do the initial init when `self' was `NULL'. */
   DeeObject_Init(result,&DeeString_Type);
   result->s_data = NULL;
   result->s_hash = (dhash_t)-1;
  }
  result->s_len         = num_bytes;
  result->s_str[num_bytes] = '\0';
 }
 return (DREF DeeObject *)result;
}
PUBLIC DREF DeeObject *DCALL
DeeString_NewBuffer(size_t num_bytes) {
 DREF String *result;
 if unlikely(!num_bytes) {
  Dee_Incref(Dee_EmptyString);
  return Dee_EmptyString;
 }
 result = (DREF String *)DeeObject_Malloc(offsetof(String,s_str)+
                                         (num_bytes+1)*sizeof(char));
 if likely(result) {
  DeeObject_Init(result,&DeeString_Type);
  result->s_data        = NULL;
  result->s_hash        = (dhash_t)-1;
  result->s_len         = num_bytes;
  result->s_str[num_bytes] = '\0';
 }
 return (DREF DeeObject *)result;
}
PUBLIC void DCALL
DeeString_FreeWidth(DeeObject *__restrict self) {
 struct string_utf *utf;
 ASSERTF(DeeString_Check(self),"Not a string buffer");
 ASSERTF(!DeeObject_IsShared(self),"String buffers cannot be shared");
 if ((utf = ((DeeStringObject *)self)->s_data) == NULL) return;
 ASSERTF(utf->u_width == STRING_WIDTH_1BYTE,"This isn't a 1-byte string");
 string_utf_fini(utf,self);
 ((DeeStringObject *)self)->s_data = NULL;
 string_utf_free(utf);
}

PUBLIC DREF DeeObject *DCALL
DeeString_NewSized(char const *__restrict str, size_t length) {
 DREF DeeObject *result;
 /* Optimization: use pre-allocated latin1 strings
  *               for single-character sequences. */
 switch (length) {
 case 0: return_empty_string;
 case 1: return DeeString_Chr((uint8_t)str[0]);
 default: break;
 }
 result = DeeString_NewBuffer(length);
 if (result) memcpy(DeeString_STR(result),str,length*sizeof(char));
 return result;
}

PUBLIC DREF DeeObject *DCALL
DeeString_New(/*unsigned*/char const *__restrict str) {
 return DeeString_NewSized(str,strlen(str));
}


PRIVATE DREF DeeObject *DCALL
string_new_empty(void) {
 return_empty_string;
}

PRIVATE DREF DeeObject *DCALL
string_new(size_t argc, DeeObject **__restrict argv) {
 DeeObject *ob;
 if (DeeArg_Unpack(argc,argv,"o:string",&ob)) return NULL;
 return DeeObject_Str(ob);
}

PUBLIC DREF DeeObject *DCALL
DeeString_VNewf(/*utf-8*/char const *__restrict format, va_list args) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 if unlikely(unicode_printer_vprintf(&printer,format,args) < 0)
    goto err;
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}

PUBLIC DREF DeeObject *
DeeString_Newf(/*utf-8*/char const *__restrict format, ...) {
 va_list args;
 DREF DeeObject *result;
 va_start(args,format);
 result = DeeString_VNewf(format,args);
 va_end(args);
 return result;
}


PRIVATE int DCALL
string_bool(String *__restrict self) {
 return !DeeString_IsEmpty(self);
}

INTERN dhash_t DCALL
DeeString_Hash(DeeObject *__restrict self) {
 dhash_t result;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 result = DeeString_HASH(self);
 if (result == (dhash_t)-1) {
  SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
  CASE_WIDTH_1BYTE:
   result = hash_ptr(DeeString_STR(self),
                     DeeString_SIZE(self));
   break;
  {
   uint16_t *str;
  CASE_WIDTH_2BYTE:
   str = DeeString_Get2Byte(self);
   result = hash_ptrw(str,WSTR_LENGTH(str));
  } break;
  {
   uint32_t *str;
  CASE_WIDTH_4BYTE:
   str = DeeString_Get4Byte(self);
   result = hash_ptrl(str,WSTR_LENGTH(str));
  } break;
  }
  DeeString_HASH(self) = result;
 }
 return result;
}

PUBLIC dhash_t DCALL
DeeString_HashCase(DeeObject *__restrict self) {
 dhash_t result;
 ASSERT_OBJECT_TYPE_EXACT(self,&DeeString_Type);
 SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
 CASE_WIDTH_1BYTE:
  result = hash_caseptr(DeeString_STR(self),
                        DeeString_SIZE(self));
  break;
 {
  uint16_t *str;
 CASE_WIDTH_2BYTE:
  str = DeeString_Get2Byte(self);
  result = hash_caseptrw(str,WSTR_LENGTH(str));
 } break;
 {
  uint32_t *str;
 CASE_WIDTH_4BYTE:
  str = DeeString_Get4Byte(self);
  result = hash_caseptrl(str,WSTR_LENGTH(str));
 } break;
 }
 return result;
}

PRIVATE DREF DeeObject *DCALL
string_repr(DeeObject *__restrict self) {
 struct ascii_printer printer = ASCII_PRINTER_INIT;
 void *str = DeeString_WSTR(self);
 SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
 CASE_WIDTH_1BYTE:
  if unlikely(Dee_FormatQuote8((dformatprinter)&ascii_printer_print,&printer,
                               (uint8_t *)str,WSTR_LENGTH(str),
                                FORMAT_QUOTE_FNORMAL) < 0)
     goto err;
  break;
 CASE_WIDTH_2BYTE:
  if unlikely(Dee_FormatQuote16((dformatprinter)&ascii_printer_print,&printer,
                                (uint16_t *)str,WSTR_LENGTH(str),
                                 FORMAT_QUOTE_FNORMAL) < 0)
     goto err;
  break;
 CASE_WIDTH_4BYTE:
  if unlikely(Dee_FormatQuote32((dformatprinter)&ascii_printer_print,&printer,
                                (uint32_t *)str,WSTR_LENGTH(str),
                                 FORMAT_QUOTE_FNORMAL) < 0)
     goto err;
  break;
 }
 return ascii_printer_pack(&printer);
err:
 ascii_printer_fini(&printer);
 return NULL;
}

PRIVATE void DCALL
string_fini(String *__restrict self) {
 struct string_utf *utf;
 /* Clean up UTF data. */
 if ((utf = self->s_data) != NULL) {
  string_utf_fini(utf,self);
  Dee_Free(utf);
 }
}


INTERN int DCALL
compare_string_bytes(String *__restrict lhs,
                     DeeBytesObject *__restrict rhs) {
 size_t lhs_len;
 size_t rhs_len;
 if (!lhs->s_data ||
      lhs->s_data->u_width == STRING_WIDTH_1BYTE) {
  uint8_t *lhs_str; int result;
  /* Compare against single-byte string. */
  lhs_str = (uint8_t *)lhs->s_str;
  lhs_len = lhs->s_len;
  rhs_len = DeeBytes_SIZE(rhs);
  /* Most simple case: compare ascii/single-byte strings. */
  result = memcmp(lhs_str,DeeBytes_DATA(rhs),MIN(lhs_len,rhs_len));
  if (result != 0) return result;
 } else {
  uint8_t *rhs_str;
  struct string_utf *lhs_utf;
  /* Compare against single-byte string. */
  rhs_str = DeeBytes_DATA(rhs);
  rhs_len = DeeBytes_SIZE(rhs);
  lhs_utf = lhs->s_data;
  switch (lhs_utf->u_width) {

  {
   uint16_t *lhs_str;
   size_t i,common_len;
  CASE_WIDTH_2BYTE:
   lhs_str = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
   lhs_len = WSTR_LENGTH(lhs_str);
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
   common_len = MIN(rhs_len,lhs_len);
   for (i = 0; i < common_len; ++i) {
    if (lhs_str[i] == (uint32_t)(rhs_str[i]))
        continue;
    return lhs_str[i] < (uint32_t)(rhs_str[i]) ? -1 : 1;
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
compare_strings(String *__restrict lhs,
                String *__restrict rhs) {
 size_t lhs_len;
 size_t rhs_len;
 if (!lhs->s_data ||
      lhs->s_data->u_width == STRING_WIDTH_1BYTE) {
  uint8_t *lhs_str;
  /* Compare against single-byte string. */
  lhs_str = (uint8_t *)lhs->s_str;
  lhs_len = lhs->s_len;
  if (!rhs->s_data ||
       rhs->s_data->u_width == STRING_WIDTH_1BYTE) {
   int result;
   rhs_len = rhs->s_len;
   /* Most simple case: compare ascii/single-byte strings. */
   result = memcmp(lhs_str,rhs->s_str,MIN(lhs_len,rhs_len));
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
  lhs_utf = lhs->s_data;
  switch (lhs_utf->u_width) {

  {
   uint16_t *lhs_str;
   size_t i,common_len;
  CASE_WIDTH_2BYTE:
   lhs_str = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
   lhs_len = WSTR_LENGTH(lhs_str);
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
   switch (rhs_utf->u_width) {

   {
    uint16_t *rhs_str;
    size_t common_len;
   CASE_WIDTH_2BYTE:
    rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
    rhs_len = WSTR_LENGTH(rhs_str);
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
   switch (rhs_utf->u_width) {

   {
    uint16_t *rhs_str;
    size_t i,common_len;
   CASE_WIDTH_2BYTE:
    rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
    rhs_len = WSTR_LENGTH(rhs_str);
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


PRIVATE DREF DeeObject *DCALL
string_lo(String *__restrict self, DeeObject *__restrict some_object) {
 int result;
 if (DeeBytes_Check(some_object))
     result = compare_string_bytes(self,(DeeBytesObject *)some_object);
 else {
  if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
      return NULL;
 }
 result = compare_strings(self,(String *)some_object);
 return_bool_(result < 0);
}
PRIVATE DREF DeeObject *DCALL
string_le(String *__restrict self, DeeObject *__restrict some_object) {
 int result;
 if (DeeBytes_Check(some_object))
     result = compare_string_bytes(self,(DeeBytesObject *)some_object);
 else {
  if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
      return NULL;
 }
 result = compare_strings(self,(String *)some_object);
 return_bool_(result <= 0);
}
PRIVATE DREF DeeObject *DCALL
string_gr(String *__restrict self, DeeObject *__restrict some_object) {
 int result;
 if (DeeBytes_Check(some_object))
     result = compare_string_bytes(self,(DeeBytesObject *)some_object);
 else {
  if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
      return NULL;
 }
 result = compare_strings(self,(String *)some_object);
 return_bool_(result > 0);
}
PRIVATE DREF DeeObject *DCALL
string_ge(String *__restrict self, DeeObject *__restrict some_object) {
 int result;
 if (DeeBytes_Check(some_object))
     result = compare_string_bytes(self,(DeeBytesObject *)some_object);
 else {
  if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
      return NULL;
 }
 result = compare_strings(self,(String *)some_object);
 return_bool_(result >= 0);
}


INTDEF bool DCALL
string_eq_bytes(String *__restrict self,
                DeeBytesObject *__restrict other);
PRIVATE DREF DeeObject *DCALL
string_eq(String *__restrict self, DeeObject *__restrict some_object) {
 /* Basic checks for same-object. */
 if (self == (String *)some_object)
     return_true;
 if (DeeBytes_Check(some_object))
     return_bool(string_eq_bytes(self,(DeeBytesObject *)some_object));
 if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
     return NULL;
 if (DeeString_Hash((DeeObject *)self) !=
     DeeString_Hash((DeeObject *)some_object))
     return_false;
 return_bool(compare_strings(self,(String *)some_object) == 0);
}
PRIVATE DREF DeeObject *DCALL
string_ne(String *__restrict self, DeeObject *__restrict some_object) {
 /* Basic checks for same-object. */
 if (self == (String *)some_object) return_false;
 if (DeeBytes_Check(some_object))
     return_bool(!string_eq_bytes(self,(DeeBytesObject *)some_object));
 if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
     return NULL;
 if (DeeString_Hash((DeeObject *)self) !=
     DeeString_Hash((DeeObject *)some_object))
     return_true;
 /* XXX: This could be optimized! */
 return_bool(compare_strings(self,(String *)some_object) != 0);
}




typedef struct {
    OBJECT_HEAD
    DREF String   *si_string; /* [1..1][const] The string that is being iterated. */
    union dcharptr si_iter;   /* [1..1][weak] The current iterator position. */
    union dcharptr si_end;    /* [1..1][const] The string end pointer. */
    unsigned int   si_width;  /* [const] The stirng width used during iteration (One of `STRING_WIDTH_*'). */
} StringIterator;

PRIVATE void DCALL
stringiter_fini(StringIterator *__restrict self) {
 Dee_Decref(self->si_string);
}
PRIVATE DREF DeeObject *DCALL
stringiter_next(StringIterator *__restrict self) {
 DREF DeeObject *result;
 union dcharptr pos; uint8_t *new_pos;
 /* Consume one character (atomically) */
 do {
  pos.ptr = self->si_iter.ptr;
  if (pos.ptr >= self->si_end.ptr)
      return ITER_DONE;
  new_pos = (uint8_t *)pos.ptr+STRING_SIZEOF_WIDTH(self->si_width);
 } while (!ATOMIC_CMPXCH(self->si_iter.ptr,pos.ptr,(void *)new_pos));
 /* Create the single-character string. */
 SWITCH_SIZEOF_WIDTH(self->si_width) {
 CASE_WIDTH_1BYTE: result = DeeString_Chr(*pos.cp8); break;
 CASE_WIDTH_2BYTE: result = DeeString_Chr(*pos.cp16); break;
 CASE_WIDTH_4BYTE: result = DeeString_Chr(*pos.cp32); break;
 }
 return result;
}
PRIVATE int DCALL
stringiter_ctor(StringIterator *__restrict self) {
 self->si_string   = (DREF String *)Dee_EmptyString;
 self->si_iter.ptr = DeeString_STR(Dee_EmptyString);
 self->si_end.ptr  = DeeString_STR(Dee_EmptyString);
 self->si_width    = STRING_WIDTH_1BYTE;
 Dee_Incref(Dee_EmptyString);
 return 0;
}
PRIVATE int DCALL
stringiter_copy(StringIterator *__restrict self,
                StringIterator *__restrict other) {
 self->si_string = other->si_string;
#ifdef CONFIG_NO_THREADS
 self->si_iter.ptr = other->si_iter.ptr;
#else
 self->si_iter.ptr = ATOMIC_READ(other->si_iter.ptr);
#endif
 self->si_end    = other->si_end;
 self->si_width  = other->si_width;
 Dee_Incref(self->si_string);
 return 0;
}
PRIVATE int DCALL
stringiter_init(StringIterator *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 String *str;
 if (DeeArg_Unpack(argc,argv,"o:string.iterator",&str) ||
     DeeObject_AssertTypeExact((DeeObject *)str,&DeeString_Type))
     return -1;
 self->si_string = str;
 Dee_Incref(str);
 self->si_width = DeeString_WIDTH(str);
 self->si_iter.ptr = self->si_width == STRING_WIDTH_1BYTE
                   ? (void *)DeeString_STR(str)
                   : (void *)str->s_data->u_data[self->si_width];
 self->si_end.ptr  = (void *)((uintptr_t)self->si_iter.ptr+
                               WSTR_LENGTH(self->si_iter.ptr)*
                               STRING_SIZEOF_WIDTH(self->si_width));
 return 0;
}

PRIVATE struct type_member stringiter_members[] = {
    TYPE_MEMBER_FIELD("seq",STRUCT_OBJECT,offsetof(StringIterator,si_string)),
    TYPE_MEMBER_FIELD("__width__",STRUCT_CONST|STRUCT_INT,offsetof(StringIterator,si_width)),
    TYPE_MEMBER_END
};

INTDEF DeeTypeObject StringIterator_Type;

PRIVATE DREF DeeObject *DCALL
stringiter_eq(StringIterator *__restrict self,
              StringIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&StringIterator_Type))
     return NULL;
 return_bool(self->si_string == other->si_string &&
             self->si_iter.ptr == other->si_iter.ptr);
}
PRIVATE DREF DeeObject *DCALL
stringiter_ne(StringIterator *__restrict self,
              StringIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&StringIterator_Type))
     return NULL;
 return_bool(self->si_string != other->si_string ||
             self->si_iter.ptr != other->si_iter.ptr);
}
PRIVATE DREF DeeObject *DCALL
stringiter_lo(StringIterator *__restrict self,
              StringIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&StringIterator_Type))
     return NULL;
 return_bool(self->si_string < other->si_string ||
            (self->si_string == other->si_string &&
             self->si_iter.ptr < other->si_iter.ptr));
}
PRIVATE DREF DeeObject *DCALL
stringiter_le(StringIterator *__restrict self,
              StringIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&StringIterator_Type))
     return NULL;
 return_bool(self->si_string < other->si_string ||
            (self->si_string == other->si_string &&
             self->si_iter.ptr <= other->si_iter.ptr));
}
PRIVATE DREF DeeObject *DCALL
stringiter_gr(StringIterator *__restrict self,
              StringIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&StringIterator_Type))
     return NULL;
 return_bool(self->si_string > other->si_string ||
            (self->si_string == other->si_string &&
             self->si_iter.ptr > other->si_iter.ptr));
}
PRIVATE DREF DeeObject *DCALL
stringiter_ge(StringIterator *__restrict self,
              StringIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&StringIterator_Type))
     return NULL;
 return_bool(self->si_string > other->si_string ||
            (self->si_string == other->si_string &&
             self->si_iter.ptr >= other->si_iter.ptr));
}

PRIVATE struct type_cmp stringiter_cmp = {
    /* Compare operators. */
    /* .tp_hash = */NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&stringiter_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&stringiter_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&stringiter_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&stringiter_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&stringiter_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&stringiter_ge,
};

INTERN DeeTypeObject StringIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"string.iterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&stringiter_ctor,
                /* .tp_copy_ctor = */&stringiter_copy,
                /* .tp_deep_ctor = */&stringiter_copy,
                /* .tp_any_ctor  = */&stringiter_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(StringIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&stringiter_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&stringiter_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stringiter_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */stringiter_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};



PRIVATE DREF DeeObject *DCALL
stringiter_self(String *__restrict self) {
 DREF StringIterator *result;
 result = (DREF StringIterator *)DeeObject_Malloc(sizeof(StringIterator));
 if unlikely(!result) return NULL;
 DeeObject_Init(result,&StringIterator_Type);
 result->si_string = self;
 Dee_Incref(self);
 result->si_width = DeeString_WIDTH(self);
 result->si_iter.ptr = result->si_width == STRING_WIDTH_1BYTE
                     ? (void *)DeeString_STR(self)
                     : (void *)self->s_data->u_data[result->si_width];
 result->si_end.ptr  = (void *)((uintptr_t)result->si_iter.ptr+
                                 WSTR_LENGTH(result->si_iter.ptr)*
                                 STRING_SIZEOF_WIDTH(result->si_width));
 return (DREF DeeObject *)result;
}
INTDEF DREF DeeObject *DCALL
string_contains(String *__restrict self,
                DeeObject *__restrict some_object);

PRIVATE DREF DeeObject *DCALL
string_size(String *__restrict self) {
 size_t result = DeeString_WLEN(self);
 return DeeInt_NewSize(result);
}
PRIVATE DREF DeeObject *DCALL
string_get(String *__restrict self,
           DeeObject *__restrict index) {
 int width = DeeString_WIDTH(self);
 union dcharptr str; size_t i,len;
 str.ptr = DeeString_WSTR(self);
 len     = WSTR_LENGTH(str.ptr);
 if (DeeObject_AsSize(index,&i))
     return NULL;
 if unlikely((size_t)i >= len) {
  err_index_out_of_bounds((DeeObject *)self,i,len);
  return NULL;
 }
 SWITCH_SIZEOF_WIDTH(width) {
 CASE_WIDTH_1BYTE:
  return DeeString_Chr(str.cp8[i]);
 CASE_WIDTH_2BYTE:
  return DeeString_Chr(str.cp16[i]);
 CASE_WIDTH_4BYTE:
  return DeeString_Chr(str.cp32[i]);
 }
}
INTERN DREF DeeObject *DCALL
string_range_get(String *__restrict self,
                 DeeObject *__restrict begin,
                 DeeObject *__restrict end) {
 void *str = DeeString_WSTR(self);
 int width = DeeString_WIDTH(self);
 size_t len = WSTR_LENGTH(str);
 dssize_t i_begin,i_end = (dssize_t)len;
 if (DeeObject_AsSSize(begin,&i_begin) ||
    (!DeeNone_Check(end) && DeeObject_AsSSize(end,&i_end)))
     return NULL;
 if (i_begin < 0) i_begin += len;
 if (i_end < 0) i_end += len;
 if unlikely((size_t)i_begin >= len || (size_t)i_begin >= (size_t)i_end)
    return_empty_string;
 if unlikely((size_t)i_end > len) i_end = (dssize_t)len;
 return DeeString_NewWithWidth((uint8_t *)str+
                              ((size_t)i_begin*STRING_SIZEOF_WIDTH(width)),
                               (size_t)i_end-(size_t)i_begin,
                                width);
}


PRIVATE struct type_cmp string_cmp = {
    /* .tp_hash = */&DeeString_Hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&string_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&string_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&string_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&string_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&string_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&string_ge,
};

PRIVATE size_t DCALL
string_nsi_getsize(String *__restrict self) {
 ASSERT(DeeString_WLEN(self) != (size_t)-1);
 return DeeString_WLEN(self);
}

PRIVATE DREF DeeObject *DCALL
string_nsi_getitem(String *__restrict self, size_t index) {
 if unlikely(index >= DeeString_WLEN(self)) {
  err_index_out_of_bounds((DeeObject *)self,index,DeeString_WLEN(self));
  return NULL;
 }
 return DeeString_Chr(DeeString_GetChar(self,index));
}

PRIVATE DREF DeeObject *DCALL
string_nsi_getitem_fast(String *__restrict self, size_t index) {
 ASSERT(index < DeeString_WLEN(self));
 return DeeString_Chr(DeeString_GetChar(self,index));
}

PRIVATE struct type_nsi string_nsi = {
    /* .nsi_class   = */TYPE_SEQX_CLASS_SEQ,
    /* .nsi_flags   = */TYPE_SEQX_FNORMAL,
    {
        /* .nsi_seqlike = */{
            /* .nsi_getsize      = */(void *)&string_nsi_getsize,
            /* .nsi_getsize_fast = */(void *)&string_nsi_getsize,
            /* .nsi_getitem      = */(void *)&string_nsi_getitem,
            /* .nsi_delitem      = */(void *)NULL,
            /* .nsi_setitem      = */(void *)NULL,
            /* .nsi_getitem_fast = */(void *)&string_nsi_getitem_fast,
            /* .nsi_getrange     = */(void *)NULL,
            /* .nsi_getrange_n   = */(void *)NULL,
            /* .nsi_setrange     = */(void *)NULL,
            /* .nsi_setrange_n   = */(void *)NULL,
            /* .nsi_find         = */(void *)NULL,
            /* .nsi_rfind        = */(void *)NULL,
            /* .nsi_xch          = */(void *)NULL,
            /* .nsi_insert       = */(void *)NULL,
            /* .nsi_insertall    = */(void *)NULL,
            /* .nsi_insertvec    = */(void *)NULL,
            /* .nsi_pop          = */(void *)NULL,
            /* .nsi_erase        = */(void *)NULL,
            /* .nsi_remove       = */(void *)NULL,
            /* .nsi_rremove      = */(void *)NULL,
            /* .nsi_removeall    = */(void *)NULL,
            /* .nsi_removeif     = */(void *)NULL
        }
    }
};

PRIVATE struct type_seq string_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stringiter_self,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&string_size,
    /* .tp_contains  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&string_contains,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&string_get,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&string_range_get,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&string_nsi
};

PRIVATE struct type_member string_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&StringIterator_Type),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
string_class_chr(DeeObject *__restrict UNUSED(self),
                 size_t argc, DeeObject **__restrict argv) {
 uint32_t ch;
 if (DeeArg_Unpack(argc,argv,"I32u:chr",&ch))
     return NULL;
 return DeeString_Chr(ch);
}

PRIVATE DREF DeeObject *DCALL
string_class_fromseq(DeeObject *__restrict UNUSED(self),
                     size_t argc, DeeObject **__restrict argv) {
 DeeObject *seq;
 if (DeeArg_Unpack(argc,argv,"o:fromseq",&seq))
     goto err;
 /* XXX: Fast-sequence optimizations? */
 {
  struct unicode_printer printer = UNICODE_PRINTER_INIT;
  DREF DeeObject *iter,*elem;
  iter = DeeObject_IterSelf(seq);
  if unlikely(!iter) goto err_printer;
  while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
   uint32_t chr; int temp;
   temp = DeeObject_AsUInt32(elem,&chr);
   Dee_Decref(elem);
   if unlikely(temp) goto err_iter;
   if unlikely(unicode_printer_putc(&printer,chr))
      goto err_iter;
   if (DeeThread_CheckInterrupt())
       goto err_iter;
  }
  if unlikely(!elem) goto err_iter;
  Dee_Decref(iter);
  return unicode_printer_pack(&printer);
err_iter:
  Dee_Decref(iter);
err_printer:
  unicode_printer_fini(&printer);
 }
err:
 return NULL;
}

PRIVATE struct type_method string_class_methods[] = {
    { "chr", &string_class_chr,
      DOC("(int ch)->string\n"
          "@throw IntegerOverflow @ch is negative or greater than the greatest unicode-character\n"
          "@return A single-character string matching the unicode-character @ch") },
    { "fromseq", &string_class_fromseq,
      DOC("({int...} ordinals)->string\n"
          "@throw IntegerOverflow One of the ordinals is negative, or greater than $0xffffffff\n"
          "Construct a new string object from a sequence of ordinal values") },
    { NULL }
};

INTDEF struct type_method string_methods[];
INTDEF struct type_math string_math;

INTDEF DREF DeeObject *DCALL
DeeString_Ordinals(DeeObject *__restrict self);

PRIVATE DREF DeeObject *DCALL
string_hashed(String *__restrict self) {
 return_bool(self->s_hash != DEE_STRING_HASH_UNSET);
}

PRIVATE DREF DeeObject *DCALL
string_getfirst(String *__restrict self) {
 void *str = DeeString_WSTR(self);
 int width = DeeString_WIDTH(self);
 if unlikely(!WSTR_LENGTH(str)) {
  err_empty_sequence((DeeObject *)self);
  return NULL;
 }
 return DeeString_Chr(STRING_WIDTH_GETCHAR(width,str,0));
}

PRIVATE DREF DeeObject *DCALL
string_getlast(String *__restrict self) {
 void *str = DeeString_WSTR(self);
 int width = DeeString_WIDTH(self);
 size_t length = WSTR_LENGTH(str);
 if unlikely(!length) {
  err_empty_sequence((DeeObject *)self);
  return NULL;
 }
 return DeeString_Chr(STRING_WIDTH_GETCHAR(width,str,length-1));
}


PRIVATE struct type_getset string_getsets[] = {
    { "ordinals", &DeeString_Ordinals, NULL, NULL,
      DOC("->{int...}\n"
          "Returns a proxy view for the characters of @this string as a sequence of "
          "integers referring to the ordinal values of each character (s.a. #ord)") },
    { "__hashed__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&string_hashed, NULL, NULL,
      DOC("->bool\n"
          "Evaluates to :true if @this string has been hashed") },
    { "first",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&string_getfirst, NULL, NULL,
      DOC("->string\n"
          "@throw ValueError @this string is empty\n"
          "Returns the first character of @this string") },
    { "last",
      (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&string_getlast, NULL, NULL,
      DOC("->string\n"
          "@throw ValueError @this string is empty\n"
          "Returns the last character of @this string") },
    { NULL }
};


PRIVATE int DCALL
string_getbuf(String *__restrict self,
              DeeBuffer *__restrict info,
              unsigned int UNUSED(flags)) {
 info->bb_base = DeeString_AsBytes((DeeObject *)self,false);
 if unlikely(!info->bb_base) return -1;
 info->bb_size = WSTR_LENGTH(info->bb_base);
 return 0;
}

PRIVATE struct type_buffer string_buffer = {
    /* .tp_getbuf       = */(int(DCALL *)(DeeObject *__restrict,DeeBuffer *__restrict,unsigned int))&string_getbuf,
    /* .tp_putbuf       = */NULL,
    /* .tp_buffer_flags = */DEE_BUFFER_TYPE_FREADONLY
};


PUBLIC DeeTypeObject DeeString_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_string),
    /* .tp_doc      = */DOC("An encoding-neutral, immutable sequence of characters\n"
                            "\n"
                            "()\n"
                            "Returns an empty string $\"\"\n"
                            "\n"
                            "(object ob)\n"
                            "Same as ${str ob}, returning the string representation of @ob\n"
                            "\n"
                            "str->\n"
                            "Simply re-return @this string\n"
                            "\n"
                            "repr->\n"
                            "Returns @this string as a C-style escaped string\n"
                            ">operator repr() {\n"
                            "> return \"\\\"{}\\\"\".format({ this.encode(\"c-escape\") });\n"
                            ">}\n"
                            "\n"
                            "bool->\n"
                            "Returns :true if @this string is non-empty\n"
                            ">operator bool() {\n"
                            "> return #this != 0;\n"
                            ">}\n"
                            "\n"
                            "+(string other)->string\n"
                            "+(object other)->string\n"
                            "Return a new string that is the concatenation of @this and ${str other}\n"
                            ">operator + (other) {\n"
                            "> return \"{}{}\".format({ this, other });\n"
                            ">}\n"
                            "\n"
                            "*(int times)->string\n"
                            "@throw IntegerOverflow @times is negative, or too large\n"
                            "Returns @this string repeated @times number of times\n"
                            "\n"
                            "%(tuple args)->string\n"
                            "%(object arg)->string\n"
                            "Using @this string as a printf-style format string, use a tuple found "
                            "in @args to format it into a new string, which is then returned\n"
                            "If @arg isn't a tuple, it is packed into one and the call is identical "
                            "to ${this.operator % (pack(arg))}\n"
                            ">local x = 42;\n"
                            ">print \"x = %d\" % x; /* \"x = 42\" */\n"
                            "\n"
                            "<(string other)\n"
                            "<(bytes other)\n"
                            "<=(string other)\n"
                            "<=(bytes other)\n"
                            "==(string other)\n"
                            "==(bytes other)\n"
                            "!=(string other)\n"
                            "!=(bytes other)\n"
                            ">(string other)\n"
                            ">(bytes other)\n"
                            ">=(string other)\n"
                            ">=(bytes other)\n"
                            "Perform a lexicographical comparison between @this string and @other, and return the result\n"
                            "\n"
                            "iter->\n"
                            "Return a string iterator that can be used to enumerate each of the string's characters individually\n"
                            "\n"
                            "#->\n"
                            "Returns the length of @this string in characters\n"
                            "\n"
                            "contains(string substr)\n"
                            "Returns :true if @substr is apart of @this string\n"
                            ">print \"foo\" in \"bar\";    /* false */\n"
                            ">print \"foo\" in \"foobar\"; /* true */\n"
                            "\n"
                            "[](int index)->string\n"
                            "@throw IntegerOverflow @index is negative\n"
                            "@throw IndexError @index is greater than ${#this}\n"
                            "Returns the @{index}th character of @this string\n"
                            ">print \"foo\"[0]; /* \"f\" */\n"
                            ">print \"foo\"[1]; /* \"o\" */\n"
                            "\n"
                            "[:](int start,int end)->string\n"
                            "Return a sub-string of @this, that starts at @start and ends at @end\n"
                            "If @end is greater than ${#this}, it is truncated to that value\n"
                            "If @start is greater than, or equal to @end, an empty string is returned\n"
                            "If either @start or @end is negative, ${#this} is added before "
                            "further index transformations are performed\n"
                            "As per convention, :none may be passed for @end as an alias for ${#this}\n"
                            ">print \"foo\"[:-1];      /* \"fo\" */\n"
                            ">print \"bar\"[1:];       /* \"ar\" */\n"
                            ">print \"foobar\"[3:123]; /* \"bar\" */\n"
                            ">print \"bizbuz\"[5:4];   /* \"\" */"),
    /* .tp_flags    = */TP_FNORMAL|TP_FVARIABLE|TP_FNAMEOBJECT,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_var = */{
                /* .tp_ctor      = */&string_new_empty,
                /* .tp_copy_ctor = */&DeeObject_NewRef,
                /* .tp_deep_ctor = */&DeeObject_NewRef,
                /* .tp_any_ctor  = */&string_new
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&string_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */&DeeObject_NewRef,
        /* .tp_repr = */&string_repr,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&string_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */&string_math,
    /* .tp_cmp           = */&string_cmp,
    /* .tp_seq           = */&string_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */&string_buffer,
    /* .tp_methods       = */string_methods,
    /* .tp_getsets       = */string_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */string_class_methods,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */string_class_members
};

PRIVATE struct { size_t len; uint32_t zero; } empty_utf = { 0, 0 };
PRIVATE struct string_utf empty_string_utf = {
    /* .u_width = */STRING_WIDTH_1BYTE, /* Every character fits into a single byte (because there are no characters) */
    /* .u_flags = */STRING_UTF_FASCII,
    /* .u_data  = */{
        /* [STRING_WIDTH_1BYTE] = */(size_t *)&DeeString_Empty.s_zero,
        /* [STRING_WIDTH_2BYTE] = */(size_t *)&empty_utf.zero,
        /* [STRING_WIDTH_4BYTE] = */(size_t *)&empty_utf.zero
    },
    /* .u_utf8  = */&DeeString_Empty.s_zero
};

PUBLIC struct empty_string
DeeString_Empty = {
    OBJECT_HEAD_INIT(&DeeString_Type),
    /* .s_data = */&empty_string_utf,
    /* .s_hash = */0,
    /* .s_len  = */0,
    /* .s_zero = */'\0'
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_STRING_C */
