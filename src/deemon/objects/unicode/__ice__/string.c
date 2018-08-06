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
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/seq.h>
#include <deemon/tuple.h>
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

#ifndef CONFIG_USE_NEW_STRING_API
STATIC_ASSERT(DeeEnc_Size(STRING_WIDTH_1BYTE)  == 1);
STATIC_ASSERT(DeeEnc_Size(STRING_WIDTH_2BYTE) == 2);
STATIC_ASSERT(DeeEnc_Size(STRING_WIDTH_4BYTE) == 4);
#ifdef CONFIG_WCHAR_STRINGS
STATIC_ASSERT(DeeEnc_Size(STRING_WIDTH_WCHAR) == __SIZEOF_WCHAR_T__);
STATIC_ASSERT(DeeEnc_Size(STRING_WIDTH_WCHAR) == sizeof(wchar_t));
#endif /* CONFIG_WCHAR_STRINGS */
#endif /* !CONFIG_USE_NEW_STRING_API */

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
  haystack_len = ((uintptr_t)haystack+haystack_len)-(uintptr_t)candidate;
  haystack     = (void const *)((uintptr_t)candidate+1);
 }
 return NULL;
}
#endif /* !__USE_GNU */

PUBLIC void DCALL
string_printer_release(void *self, size_t datalen) {
 struct string_printer *me;
 ASSERT(self);
 me = (struct string_printer *)self;
 ASSERT(me->sp_length >= datalen);
 /* This's actually all that needs to be
  * done with the current implementation. */
 me->sp_length -= datalen;
}
PUBLIC char *DCALL
string_printer_alloc(void *self, size_t datalen) {
 struct string_printer *me;
 String *string;
 size_t alloc_size;
 char *result;
 ASSERT(self);
 me = (struct string_printer *)self;
 if ((string = me->sp_string) == NULL) {
  /* Make sure not to allocate a string when the used length remains ZERO.
   * >> Must be done to assure the expectation of `if(sp_length == 0) sp_string == NULL' */
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
  me->sp_string = string;
  string->s_len = alloc_size;
  me->sp_length = datalen;
  return string->s_str;
 }
 alloc_size = string->s_len;
 ASSERT(alloc_size >= me->sp_length);
 alloc_size -= me->sp_length;
 if unlikely(alloc_size < datalen) {
  size_t min_alloc = me->sp_length+datalen;
  alloc_size = (min_alloc+63) & ~63;
realloc_again:
  string = (String *)DeeObject_TryRealloc(string,offsetof(String,s_str)+
                                         (alloc_size+1)*sizeof(char));
  if unlikely(!string) {
   string = me->sp_string;
   if (alloc_size != min_alloc) { alloc_size = min_alloc; goto realloc_again; }
   if (Dee_CollectMemory(offsetof(String,s_str)+
                        (alloc_size+1)*sizeof(char)))
       goto realloc_again;
   return NULL;
  }
  me->sp_string = string;
  string->s_len = alloc_size;
 }
 /* Append text at the end. */
 result = string->s_str+me->sp_length;
 me->sp_length += datalen;
 return result;
}
PUBLIC int DCALL
string_printer_putc(void *self, char ch) {
 struct string_printer *me;
 ASSERT(self);
 me = (struct string_printer *)self;
 /* Quick check: Can we print to an existing buffer. */
 if (me->sp_string &&
     me->sp_length < me->sp_string->s_len) {
  me->sp_string->s_str[me->sp_length++] = ch;
  goto done;
 }
 /* Fallback: go the long route. */
 if (string_printer_print(self,&ch,1) < 0)
     goto err;
done:
 return 0;
err:
 return -1;
}
PUBLIC dssize_t DCALL
string_printer_print(void *self,
                     char const *__restrict data,
                     size_t datalen) {
 struct string_printer *me;
 String *string;
 size_t alloc_size;
 ASSERT(self);
 ASSERT(data || !datalen);
 me = (struct string_printer *)self;
 if ((string = me->sp_string) == NULL) {
  /* Make sure not to allocate a string when the used length remains ZERO.
   * >> Must be done to assure the expectation of `if(sp_length == 0) sp_string == NULL' */
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
  me->sp_string = string;
  string->s_len = alloc_size;
  memcpy(string->s_str,data,datalen*sizeof(char));
  me->sp_length = datalen;
  goto done;
 }
 alloc_size = string->s_len;
 ASSERT(alloc_size >= me->sp_length);
 alloc_size -= me->sp_length;
 if unlikely(alloc_size < datalen) {
  size_t min_alloc = me->sp_length+datalen;
  alloc_size = (min_alloc+63) & ~63;
realloc_again:
  string = (String *)DeeObject_TryRealloc(string,offsetof(String,s_str)+
                                         (alloc_size+1)*sizeof(char));
  if unlikely(!string) {
   string = me->sp_string;
   if (alloc_size != min_alloc) { alloc_size = min_alloc; goto realloc_again; }
   if (Dee_CollectMemory(offsetof(String,s_str)+
                        (alloc_size+1)*sizeof(char)))
       goto realloc_again;
   return -1;
  }
  me->sp_string = string;
  string->s_len = alloc_size;
 }
 /* Copy text into the dynamic string. */
 /*DEE_DPRINTF("PRINT: %IX - `%.*s'\n",datalen,(int)datalen,data);*/
 memcpy(string->s_str+me->sp_length,
        data,datalen*sizeof(char));
 me->sp_length += datalen;
done:
 return (dssize_t)datalen;
}

/* Pack together data from a string printer and return the generated contained string. */
PUBLIC DREF DeeObject *DCALL
string_printer_pack(struct string_printer *__restrict self) {
 DREF String *result = self->sp_string;
 if unlikely(!result) return_reference_(Dee_EmptyString);
 /* Deallocate unused memory. */
 if likely(self->sp_length != result->s_len) {
  DREF String *reloc;
  reloc = (DREF String *)DeeObject_TryRealloc(result,offsetof(String,s_str)+
                                             (self->sp_length+1)*sizeof(char));
  if likely(reloc) result = reloc;
  result->s_len = self->sp_length;
 }
 /* Make sure to terminate the c-string representation. */
 result->s_str[self->sp_length] = '\0';
 /* Do final object initialization. */
 DeeObject_Init(result,&DeeString_Type);
 result->s_hash = (dhash_t)-1;
 result->s_data = NULL;
 self->sp_string = NULL;
 return (DREF DeeObject *)result;
}

PUBLIC DREF DeeObject *DCALL
string_printer_packfini(struct string_printer *__restrict self) {
 DREF String *result = self->sp_string;
 if unlikely(!result) return_reference_(Dee_EmptyString);
 /* Deallocate unused memory. */
 if likely(self->sp_length != result->s_len) {
  DREF String *reloc;
  reloc = (DREF String *)DeeObject_TryRealloc(result,offsetof(String,s_str)+
                                             (self->sp_length+1)*sizeof(char));
  if likely(reloc) result = reloc;
  result->s_len = self->sp_length;
 }
 /* Make sure to terminate the c-string representation. */
 result->s_str[self->sp_length] = '\0';
 /* Do final object initialization. */
 DeeObject_Init(result,&DeeString_Type);
 result->s_hash = (dhash_t)-1;
 result->s_data = NULL;
 return (DREF DeeObject *)result;
}

PUBLIC char *DCALL
string_printer_allocstr(struct string_printer *__restrict self,
                        char const *__restrict str, size_t length) {
 char *result; dssize_t error;
 if (self->sp_string) {
  result = (char *)memmem(self->sp_string->s_str,
                          self->sp_length,str,length);
  if (result) return result;
 }
 /* Append a new string. */
 error = string_printer_print(self,str,length);
 if unlikely(error < 0) return NULL;
 ASSERT(self->sp_string);
 ASSERT(self->sp_length >= length);
 return self->sp_string->s_str+(self->sp_length-length);
}

PRIVATE void DCALL
free_utf(struct string_utf *self) {
 unsigned int i;
#ifdef CONFIG_USE_NEW_STRING_API
 for (i = 1; i < STRING_WIDTH_COUNT; ++i)
#else
 for (i = 0; i < STRING_WIDTH_COUNT; ++i)
#endif
 {
  if (!self->su_enc[i]) continue;
  Dee_Free(self->su_enc[i]-1);
 }
 utf_free(self);
}

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
#ifdef CONFIG_USE_NEW_STRING_API
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
#endif
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
#ifndef CONFIG_USE_NEW_STRING_API
PUBLIC void DCALL
DeeString_DelBufferEncoding(DeeObject *__restrict self) {
 struct string_utf *utf; unsigned int i;
 ASSERTF(DeeString_Check(self),"Not a string buffer");
 ASSERTF(!DeeObject_IsShared(self),"String buffers cannot be shared");
 if ((utf = DeeString_UTF(self)) == NULL) return;
 ASSERTF(utf->su_pref == STRING_WIDTH_1BYTE,"This isn't a UTF-8 buffer");
 for (i = 0; i < STRING_WIDTH_COUNT; ++i) {
  if (!utf->su_enc[i]) continue;
  Dee_Free(utf->su_enc[i]-1);
  utf->su_enc[i] = NULL;
 }
}
PUBLIC ATTR_RETNONNULL DREF DeeObject *DCALL
DeeString_SetBufferWithLength(DREF DeeObject *__restrict self, size_t length) {
 DREF String *result = (DREF String *)self;
 ASSERTF(DeeString_Check(self),"Not a string buffer");
 ASSERTF(!DeeObject_IsShared(self),"String buffers cannot be shared");
 ASSERTF(length <= DeeString_SIZE(self),"The buffer isn't large enough");
 if (length != result->s_len) {
  result = (DREF String *)DeeObject_TryRealloc(self,offsetof(String,s_str)+
                                              (length+1)*sizeof(char));
  if unlikely(!result) result = (DREF String *)self;
  result->s_len         = length;
  result->s_str[length] = '\0';
 }
 return (DREF DeeObject *)result;
}
#else /* !CONFIG_USE_NEW_STRING_API */
PUBLIC void DCALL
DeeString_FreeWidth(DeeObject *__restrict self) {
 struct string_utf *utf; unsigned int i;
 ASSERTF(DeeString_Check(self),"Not a string buffer");
 ASSERTF(!DeeObject_IsShared(self),"String buffers cannot be shared");
 if ((utf = ((DeeStringObject *)self)->s_data) == NULL) return;
 ASSERTF(utf->u_width == STRING_WIDTH_1BYTE,"This isn't a 1-byte string");
 for (i = 0; i < STRING_WIDTH_COUNT; ++i) {
  if (!utf->u_data[i]) continue;
  Dee_Free(utf->u_data[i]-1);
  utf->u_data[i] = NULL;
 }
 ((DeeStringObject *)self)->s_data = NULL;
 string_utf_free(utf);
}
#endif /* !CONFIG_USE_NEW_STRING_API */

PUBLIC DREF DeeObject *DCALL
DeeString_NewSized(char const *__restrict str, size_t length) {
 DREF DeeObject *result;
 /* Optimization: use pre-allocated latin1 strings
  *               for single-character sequences. */
 switch (length) {
 case 0: return_empty_string;
 case 1: return DeeString_New1Char((uint8_t)str[0]);
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
string_new_empty(DeeTypeObject *__restrict UNUSED(tp)) {
 return_empty_string;
}

PRIVATE DREF DeeObject *DCALL
string_new(DeeTypeObject *__restrict UNUSED(tp),
           size_t argc, DeeObject **__restrict argv) {
 DeeObject *ob;
 if (DeeArg_Unpack(argc,argv,"o:string",&ob)) return NULL;
 return DeeObject_Str(ob);
}

PUBLIC DREF DeeObject *DCALL
DeeString_VNewf(/*unsigned*/char const *__restrict format, va_list args) {
 struct string_printer printer = STRING_PRINTER_INIT;
 if (string_printer_vprintf(&printer,format,args) < 0) goto err;
 return string_printer_pack(&printer);
err:
 string_printer_fini(&printer);
 return NULL;
}

PUBLIC DREF DeeObject *
DeeString_Newf(/*unsigned*/char const *__restrict format, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,format);
 result = DeeString_VNewf(format,args);
 va_end(args);
 return result;
}




INTERN DREF DeeObject *DCALL
DeeString_CatInherited(DREF DeeObject *__restrict lhs,
                       DREF DeeObject *__restrict rhs) {
 String *result;
 size_t total_size,alloc_size,lhs_size;
 ASSERT_OBJECT(lhs);
 ASSERT_OBJECT(rhs);
 ASSERT(DeeString_Check(lhs));
 ASSERT(DeeString_Check(rhs));
 if ((total_size = DeeString_SIZE(rhs)) == 0) { Dee_Decref(rhs); return lhs; }
 if (!DeeString_SIZE(lhs)) { Dee_Decref(lhs); return rhs; }
 total_size += (lhs_size = DeeString_SIZE(lhs));
 alloc_size  = (total_size+1)*sizeof(char);
 alloc_size += offsetof(String,s_str);
 if (!DeeObject_IsShared(lhs)) {
  /* Re-use 'a' (and append 'b') */
  result = (String *)DeeObject_Realloc(lhs,alloc_size);
  if unlikely(!result) goto err;
  if (result->s_data) free_utf(result->s_data);
copy_result2:
  memcpy(result->s_str+lhs_size,
         DeeString_STR(rhs),
         DeeString_SIZE(rhs)*sizeof(char));
  Dee_Decref(rhs);
 } else if (!DeeObject_IsShared(rhs)) {
  /* Re-use 'b' (and insert 'a') */
  result = (String *)DeeObject_Realloc(rhs,alloc_size);
  if unlikely(!result) goto err;
  memmove(result->s_str+lhs_size,result->s_str,
         (total_size-lhs_size)*sizeof(char));
  memcpy(result->s_str,DeeString_STR(lhs),lhs_size*sizeof(char));
  if (result->s_data) free_utf(result->s_data);
  Dee_Decref(lhs);
 } else {
  result = (String *)DeeObject_Malloc(alloc_size);
  if unlikely(!result) goto err;
  DeeObject_Init(result,&DeeString_Type);
  memcpy(result->s_str,DeeString_STR(lhs),
         lhs_size*sizeof(char));
  Dee_Decref(lhs);
  goto copy_result2;
 }
 result->s_data = NULL;
 result->s_hash = (dhash_t)-1;
 result->s_len  = total_size;
 result->s_str[total_size] = '\0';
 return (DREF DeeObject *)result;
err:
 return NULL;
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
  result = hash_ptr(DeeString_STR(self),
                    DeeString_SIZE(self));
  DeeString_HASH(self) = result;
 }
 return result;
}

PRIVATE DREF DeeObject *DCALL
string_repr(DeeObject *__restrict self) {
 struct string_printer printer = STRING_PRINTER_INIT;
#ifdef CONFIG_USE_NEW_STRING_API
 void *str = DeeString_WSTR(self);
 SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {
 CASE_WIDTH_1BYTE:
  if unlikely(Dee_FormatQuote(&string_printer_print,&printer,
                             (char *)str,WSTR_LENGTH(str),
                              FORMAT_QUOTE_FNORMAL) < 0)
     goto err;
  break;
 CASE_WIDTH_2BYTE:
  if unlikely(Dee_FormatQuote16(&string_printer_print,&printer,
                               (uint16_t *)str,WSTR_LENGTH(str),
                                FORMAT_QUOTE_FNORMAL) < 0)
     goto err;
  break;
 CASE_WIDTH_4BYTE:
  if unlikely(Dee_FormatQuote32(&string_printer_print,&printer,
                               (uint32_t *)str,WSTR_LENGTH(str),
                                FORMAT_QUOTE_FNORMAL) < 0)
     goto err;
  break;
 }
#else
 if unlikely(Dee_FormatQuote(&string_printer_print,&printer,
                              DeeString_STR(self),
                              DeeString_SIZE(self),
                              FORMAT_QUOTE_FNORMAL) < 0)
    goto err;
#endif
 return string_printer_pack(&printer);
err:
 string_printer_fini(&printer);
 return NULL;
}

PRIVATE void DCALL
string_fini(String *__restrict self) {
 /* Clean up UTF data. */
 if (self->s_data)
     free_utf(self->s_data);
}


PRIVATE DREF DeeObject *DCALL
string_eq(String *__restrict self, DeeObject *__restrict some_object) {
 /* Basic checks for same-object. */
 if (self == (String *)some_object) return_true;
 if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
     return NULL;
 /* Basic checks for size and hash. */
 /* NOTE: We don't check preferred encoding! */
 if (DeeString_SIZE(self) != DeeString_SIZE(some_object) ||
    (DeeString_HASHOK(self) && DeeString_HASHOK(some_object) &&
     DeeString_HASH(self) != DeeString_HASH(some_object)))
     return_false;
 return_bool(memcmp(DeeString_STR(self),DeeString_STR(some_object),
                    DeeString_SIZE(self)*sizeof(char)) == 0);
}
PRIVATE DREF DeeObject *DCALL
string_ne(String *__restrict self, DeeObject *__restrict some_object) {
 /* Basic checks for same-object. */
 if (self == (String *)some_object) return_false;
 if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
     return NULL;
 /* Basic checks for size and hash. */
 /* NOTE: We don't check preferred encoding! */
 if (DeeString_SIZE(self) != DeeString_SIZE(some_object) ||
    (DeeString_HASHOK(self) && DeeString_HASHOK(some_object) &&
     DeeString_HASH(self) != DeeString_HASH(some_object)))
     return_true;
 return_bool(memcmp(DeeString_STR(self),DeeString_STR(some_object),
                    DeeString_SIZE(self)*sizeof(char)) != 0);
}


#ifdef CONFIG_USE_NEW_STRING_API
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
     if ((uint16_t)lhs_str[i] == rhs_str[i])
         continue;
     return (uint16_t)lhs_str[i] < rhs_str[i] ? -1 : 1;
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
     if ((uint32_t)lhs_str[i] == rhs_str[i])
         continue;
     return (uint32_t)lhs_str[i] < rhs_str[i] ? -1 : 1;
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
    if (lhs_str[i] == (uint16_t)rhs_str[i])
        continue;
    return lhs_str[i] < (uint16_t)rhs_str[i] ? -1 : 1;
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
    if (lhs_str[i] == (uint32_t)rhs_str[i])
        continue;
    return lhs_str[i] < (uint32_t)rhs_str[i] ? -1 : 1;
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
     if ((uint32_t)lhs_str[i] == rhs_str[i])
         continue;
     return (uint32_t)lhs_str[i] < rhs_str[i] ? -1 : 1;
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
     if (lhs_str[i] == (uint32_t)rhs_str[i])
         continue;
     return lhs_str[i] < (uint32_t)rhs_str[i] ? -1 : 1;
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
 if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
     return NULL;
 result = compare_strings(self,(String *)some_object);
 return_bool_(result < 0);
}
PRIVATE DREF DeeObject *DCALL
string_le(String *__restrict self, DeeObject *__restrict some_object) {
 int result;
 if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
     return NULL;
 result = compare_strings(self,(String *)some_object);
 return_bool_(result <= 0);
}
PRIVATE DREF DeeObject *DCALL
string_gr(String *__restrict self, DeeObject *__restrict some_object) {
 int result;
 if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
     return NULL;
 result = compare_strings(self,(String *)some_object);
 return_bool_(result > 0);
}
PRIVATE DREF DeeObject *DCALL
string_ge(String *__restrict self, DeeObject *__restrict some_object) {
 int result;
 if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
     return NULL;
 result = compare_strings(self,(String *)some_object);
 return_bool_(result >= 0);
}
#else

PRIVATE dssize_t strxcmpb(size_t a_len, int8_t const *a,
                          size_t b_len, int8_t const *b) {
 int8_t const *a_end = a+a_len,*b_end = b+b_len;
 while (a != a_end && b != b_end) {
  int8_t temp = *a++ - *b++;
  if (temp)
      return temp;
 }
 return (dssize_t)((a_end-a)-(b_end-b));
}

PRIVATE DREF DeeObject *DCALL
string_lo(String *__restrict self, DeeObject *__restrict some_object) {
 if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
     return NULL;
 return_bool(strxcmpb(DeeString_SIZE(self),(int8_t *)DeeString_STR(self),
                      DeeString_SIZE(some_object),(int8_t *)DeeString_STR(some_object)) < 0);
}
PRIVATE DREF DeeObject *DCALL
string_le(String *__restrict self, DeeObject *__restrict some_object) {
 if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
     return NULL;
 return_bool(strxcmpb(DeeString_SIZE(self),(int8_t *)DeeString_STR(self),
                      DeeString_SIZE(some_object),(int8_t *)DeeString_STR(some_object)) <= 0);
}
PRIVATE DREF DeeObject *DCALL
string_gr(String *__restrict self, DeeObject *__restrict some_object) {
 if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
     return NULL;
 return_bool(strxcmpb(DeeString_SIZE(self),(int8_t *)DeeString_STR(self),
                      DeeString_SIZE(some_object),(int8_t *)DeeString_STR(some_object)) > 0);
}
PRIVATE DREF DeeObject *DCALL
string_ge(String *__restrict self, DeeObject *__restrict some_object) {
 if (DeeObject_AssertTypeExact(some_object,&DeeString_Type))
     return NULL;
 return_bool(strxcmpb(DeeString_SIZE(self),(int8_t *)DeeString_STR(self),
                      DeeString_SIZE(some_object),(int8_t *)DeeString_STR(some_object)) >= 0);
}
#endif




union unichar {
    void     *any;
    uint8_t  *ch8;
    uint16_t *ch16;
    uint32_t *ch32;
#ifdef CONFIG_WCHAR_STRINGS
    wchar_t  *wchar;
#endif /* CONFIG_WCHAR_STRINGS */
};

typedef struct {
    OBJECT_HEAD
    DREF String  *si_string; /* [1..1][const] The string that is being iterated. */
    union unichar si_iter;   /* [1..1][weak] The current iterator position. */
    union unichar si_end;    /* [1..1][const] The string end pointer. */
    unsigned int  si_width;  /* [const] The stirng width used during iteration (One of `STRING_WIDTH_*'). */
} StringIterator;

PRIVATE void DCALL
string_iterator_fini(StringIterator *__restrict self) {
 Dee_Decref(self->si_string);
}
PRIVATE DREF DeeObject *DCALL
string_iterator_next(StringIterator *__restrict self) {
 DREF DeeObject *result;
 union unichar pos; uint8_t *new_pos;
 /* Consume one character (atomically) */
 do {
  pos = self->si_iter;
  if (pos.any == self->si_end.any)
      return ITER_DONE;
  new_pos = (uint8_t *)pos.any+DeeEnc_Size(self->si_width);
 } while (!ATOMIC_CMPXCH(self->si_iter.any,pos.any,(void *)new_pos));
 /* Create the single-character string. */
 SWITCH_ENC_SIZE(self->si_width) {
 CASE_ENC_1BYTE: result = DeeString_New1Char(*pos.ch8); break;
 CASE_ENC_2BYTE: result = DeeString_New2Char(*pos.ch16); break;
 CASE_ENC_4BYTE: result = DeeString_New4Char(*pos.ch32); break;
 }
 return result;
}
PRIVATE int DCALL
string_iter_ctor(DeeTypeObject *__restrict UNUSED(tp_self),
                 StringIterator *__restrict self) {
 self->si_string   = (DREF String *)Dee_EmptyString;
 self->si_iter.any = DeeString_STR(Dee_EmptyString);
 self->si_end.any  = DeeString_STR(Dee_EmptyString);
 self->si_width    = STRING_WIDTH_1BYTE;
 Dee_Incref(Dee_EmptyString);
 return 0;
}
PRIVATE int DCALL
string_iter_copy(DeeTypeObject *__restrict UNUSED(tp_self),
                 StringIterator *__restrict self,
                 StringIterator *__restrict other) {
 self->si_string = other->si_string;
 self->si_iter   = other->si_iter;
 self->si_end    = other->si_end;
 self->si_width  = other->si_width;
 Dee_Incref(self->si_string);
 return 0;
}
PRIVATE int DCALL
string_iter_init(DeeTypeObject *__restrict UNUSED(tp_self),
                 StringIterator *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 String *str;
 if (DeeArg_Unpack(argc,argv,"o:string.iterator",&str) ||
     DeeObject_AssertTypeExact((DeeObject *)str,&DeeString_Type))
     return -1;
 self->si_string = str;
 Dee_Incref(str);
 self->si_width = DeeString_UtfEnc(str);
 self->si_iter.any = self->si_width == STRING_WIDTH_1BYTE
                   ? (void *)DeeString_STR(str)
                   : (void *)str->s_data->su_enc[self->si_width];
 self->si_end.any  = (void *)((uintptr_t)self->si_iter.any+
                               ENCODING_SIZE(self->si_iter.any)*
                               DeeEnc_Size(self->si_width));
 return 0;
}

PRIVATE struct type_member stringiterator_members[] = {
    TYPE_MEMBER_FIELD("seq",STRUCT_CONST|STRUCT_OBJECT,offsetof(StringIterator,si_string)),
    TYPE_MEMBER_FIELD("__iter__",STRUCT_CONST|STRUCT_UINTPTR_T,offsetof(StringIterator,si_iter)),
    TYPE_MEMBER_FIELD("__end__",STRUCT_CONST|STRUCT_UINTPTR_T,offsetof(StringIterator,si_end)),
    TYPE_MEMBER_FIELD("__encoding__",STRUCT_CONST|STRUCT_INT,offsetof(StringIterator,si_width)),
    TYPE_MEMBER_END
};

PRIVATE DeeTypeObject stringiterator_type = {
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
                /* .tp_ctor      = */&string_iter_ctor,
                /* .tp_copy_ctor = */&string_iter_copy,
                /* .tp_deep_ctor = */&string_iter_copy,
                /* .tp_any_ctor  = */&string_iter_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(StringIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&string_iterator_fini,
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
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&string_iterator_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */stringiterator_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};



PRIVATE DREF DeeObject *DCALL
string_iter_self(String *__restrict self) {
 DREF StringIterator *result;
 result = (DREF StringIterator *)DeeObject_Malloc(sizeof(StringIterator));
 if unlikely(!result) return NULL;
 DeeObject_Init(result,&stringiterator_type);
 result->si_string = self;
 Dee_Incref(self);
 result->si_width = DeeString_UtfEnc(self);
 result->si_iter.any = result->si_width == STRING_WIDTH_1BYTE
                     ? (void *)DeeString_STR(self)
                     : (void *)self->s_data->su_enc[result->si_width];
 result->si_end.any  = (void *)((uintptr_t)result->si_iter.any+
                                 ENCODING_SIZE(result->si_iter.any)*
                                 DeeEnc_Size(result->si_width));
 return (DREF DeeObject *)result;
}
INTDEF DREF DeeObject *DCALL
string_contains(String *__restrict self,
                DeeObject *__restrict some_object);

PRIVATE DREF DeeObject *DCALL
string_size(String *__restrict self) {
 size_t result = DeeString_UtfLen(self);
 return DeeInt_NewSize(result);
}
PRIVATE DREF DeeObject *DCALL
string_get(String *__restrict self,
           DeeObject *__restrict index) {
 int enc = DeeString_UtfEnc(self);
 void *str = DeeString_UtfStr(self);
 size_t i,len = ENCODING_SIZE(str);
 if (DeeObject_AsSize(index,&i))
     return NULL;
 if unlikely((size_t)i >= len) {
  err_index_out_of_bounds((DeeObject *)self,i,len);
  return NULL;
 }
#ifdef CONFIG_USE_NEW_STRING_API
 SWITCH_SIZEOF_WIDTH(enc) {
 CASE_WIDTH_1BYTE:
  return DeeString_New1Char(((uint8_t *)str)[i]);
 CASE_WIDTH_2BYTE:
  return DeeString_New2Char(((uint16_t *)str)[i]);
 {
  uint32_t ch;
 CASE_WIDTH_4BYTE:
  ch = ((uint32_t *)str)[i];
  if (ch < 0xff)
      return DeeString_New1Char((uint8_t)ch);
  if (ch < 0xffff)
      return DeeString_New2Char((uint16_t)ch);
  return DeeString_New4Char(ch);
 }
 }
#else
 return DeeString_NewChar(DeeEnc_Get(enc,str,(size_t)i));
#endif
}
INTERN DREF DeeObject *DCALL
string_range_get(String *__restrict self,
                 DeeObject *__restrict begin,
                 DeeObject *__restrict end) {
 dssize_t i_begin,i_end = SSIZE_MAX;
 void *str = DeeString_UtfStr(self);
 size_t len = ENCODING_SIZE(str);
 int enc = DeeString_UtfEnc(self);
 if (DeeObject_AsSSize(begin,&i_begin) ||
    (!DeeNone_Check(end) && DeeObject_AsSSize(end,&i_end)))
     return NULL;
 if (i_begin < 0) i_begin += len;
 if (i_end < 0) i_end += len;
 if unlikely((size_t)i_begin >= len || (size_t)i_begin >= (size_t)i_end) {
#ifdef CONFIG_USE_NEW_STRING_API
  return_empty_string;
#else
  return DeeString_NewSizedWithEncoding(NULL,0,enc);
#endif
 }
 if unlikely((size_t)i_end > len) i_end = (dssize_t)len;
 return DeeString_NewSizedWithEncoding((uint8_t *)str+
                                      ((size_t)i_begin*DeeEnc_Size(enc)),
                                       (size_t)i_end-(size_t)i_begin,enc);
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

PRIVATE struct type_seq string_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&string_iter_self,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&string_size,
    /* .tp_contains  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&string_contains,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&string_get,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&string_range_get,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL
};

PRIVATE struct type_member string_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&stringiterator_type),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
string_class_chr(DeeObject *__restrict UNUSED(self),
                 size_t argc, DeeObject **__restrict argv) {
 uint32_t ch;
 if (DeeArg_Unpack(argc,argv,"I32u:chr",&ch))
     return NULL;
 return DeeString_NewChar(ch);
}

PRIVATE struct type_method string_class_methods[] = {
    { "chr", &string_class_chr,
      DOC("(int ch)->string\n"
          "@throw IntegerOverflow @ch is negative or greater than the greatest unicode-character\n"
          "@return A single-character string matching the unicode-character @ch") },
    { NULL }
};

INTDEF struct type_method string_methods[];
INTDEF struct type_math string_math;


PRIVATE int DCALL
string_getbuf(String *__restrict self,
              DeeBuffer *__restrict info,
              unsigned int UNUSED(flags)) {
 info->bb_base = DeeString_STR(self);
 info->bb_size = DeeString_SIZE(self);
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
                            "Returns an empty string ${\"\"}\n"
                            "\n"
                            "(object ob)\n"
                            "Same as ${str ob}, returning the string representation of @ob\n"
                            "\n"
                            "str()\n"
                            "Simply re-return @this string\n"
                            "\n"
                            "repr()\n"
                            "Returns @this string as a C-style escaped string\n"
                            "\n"
                            "bool()\n"
                            "Returns :true if @this string is non-empty\n"
                            "\n"
                            "+(string other)->string\n"
                            "+(object other)->string\n"
                            "Return a new string that is the concantation of @this and ${str other}\n"
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
                            "$local x = 42;\n"
                            "$print \"x = %d\" % x; /* \"x = 42\" */\n"
                            "\n"
                            "<(string other)\n"
                            "<=(string other)\n"
                            "==(string other)\n"
                            "!=(string other)\n"
                            ">(string other)\n"
                            ">=(string other)\n"
                            "Perform a lexicographical comparison between @this string and @other, and return the result\n"
                            "\n"
                            "iter()\n"
                            "Return a string iterator that can be used to enumerate each of the string's characters individually\n"
                            "\n"
                            "#()\n"
                            "Returns the length of @this string in characters\n"
                            "\n"
                            "contains(string substr)\n"
                            "Returns :true if @substr is apart of @this string\n"
                            "$print \"foo\" in \"bar\";    /* false */\n"
                            "$print \"foo\" in \"foobar\"; /* true */\n"
                            "\n"
                            "[](int index)->string\n"
                            "@throw IntegerOverflow @index is negative\n"
                            "@throw IndexError @index is greater than ${#this}\n"
                            "Returns the @{index}th character of @this string\n"
                            "$print \"foo\"[0]; /* \"f\" */\n"
                            "$print \"foo\"[1]; /* \"o\" */\n"
                            "\n"
                            "[:](int start,int end)->string\n"
                            "Return a sub-string of @this, that starts at @start and ends at @end\n"
                            "If @end is greater than ${#this}, it is truncated to that value\n"
                            "If @start is greater than, or equal to @end, and empty "
                            "string with the same encoding as @this string is returned\n"
                            "If either @start or @end is negative, ${#this} is added before "
                            "further index transformations are performed\n"
                            "As per convention, :none may be passed for @end as an alias for ${#this}\n"
                            "$print \"foo\"[:-1];      /* \"fo\" */\n"
                            "$print \"bar\"[1:];       /* \"ar\" */\n"
                            "$print \"foobar\"[3:123]; /* \"bar\" */\n"
                            "$print \"bizbuz\"[5:4];   /* \"\" */"),
    /* .tp_flags    = */TP_FNORMAL|TP_FVARIABLE|TP_FNAMEOBJECT,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_var = */{
                /* .tp_ctor      = */&string_new_empty,
                /* .tp_copy_ctor = */&noop_varcopy,
                /* .tp_deep_ctor = */&noop_varcopy,
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
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */string_class_methods,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */string_class_members
};

PRIVATE struct { size_t len; uint32_t zero; } empty_utf = { 0, 0 };
PRIVATE struct string_utf empty_string_utf = {
#ifdef CONFIG_USE_NEW_STRING_API
    /* .u_width = */STRING_WIDTH_1BYTE, /* Every character fits into a single byte (because there are no characters) */
    /* .u_flags = */STRING_UTF_FASCII,
    /* .u_data  = */{
        /* [STRING_WIDTH_1BYTE] = */(size_t *)&DeeString_Empty.s_zero,
        /* [STRING_WIDTH_2BYTE] = */(size_t *)&empty_utf.zero,
        /* [STRING_WIDTH_4BYTE] = */(size_t *)&empty_utf.zero,
        /* [STRING_WIDTH_WCHAR] = */(size_t *)&empty_utf.zero
    }
#else
    /* .su_pref = */STRING_WIDTH_1BYTE,
    {
        /* .su_utf  = */{
            /* .su_2byte = */(uint16_t *)&empty_utf.zero,
            /* .su_4byte = */(uint32_t *)&empty_utf.zero
#ifdef CONFIG_WCHAR_STRINGS
            ,
            /* .su_wchar = */(wchar_t *)&empty_utf.zero
#endif /* CONFIG_WCHAR_STRINGS */
        }
    }
#endif
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
