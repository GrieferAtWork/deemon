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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FUNCTIONS_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FUNCTIONS_C_INL 1

#include "string_functions.h"

#include <deemon/bytes.h>
#include <deemon/arg.h>
#include <deemon/int.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/seq.h>
#include <deemon/tuple.h>

DECL_BEGIN

typedef DeeBytesObject Bytes;

typedef struct {
    uint8_t *n_data;
    size_t   n_size;
    uint8_t _n_buf[sizeof(size_t)];
} Needle;

#ifndef __NO_builtin_expect
#define get_needle(self,ob) __builtin_expect(get_needle(self,ob),0)
#endif

PRIVATE int (DCALL get_needle)(Needle *__restrict self, DeeObject *__restrict ob) {
 if (DeeString_Check(ob)) {
  self->n_data = DeeString_AsBytes(ob,false);
  if unlikely(!self->n_data) goto err;
  self->n_size = WSTR_LENGTH(self->n_data)*sizeof(char);
 } else if (DeeBytes_Check(ob)) {
  self->n_data = DeeBytes_DATA(ob);
  self->n_size = DeeBytes_SIZE(ob);
 } else {
  /* Convert to an integer (to-be used as a single byte). */
  if (DeeObject_AsUInt8(ob,self->_n_buf))
      goto err;
  self->n_data = self->_n_buf;
  self->n_size = 1;
 }
 return 0;
err:
 return -1;
}

INTERN DREF DeeObject *DCALL
bytes_contains(Bytes *__restrict self,
               DeeObject *__restrict find_ob) {
 Needle needle;
 if (get_needle(&needle,find_ob))
     return NULL;
 return_bool(memmemb(DeeBytes_DATA(self),
                     DeeBytes_SIZE(self),
                     needle.n_data,
                     needle.n_size) != NULL);
}

PRIVATE DREF DeeObject *DCALL
bytes_find(Bytes *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle;
 size_t start = 0,end = (size_t)-1; uint8_t *result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:find",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
  result = NULL;
 else {
  result = (uint8_t *)memmemb(DeeBytes_DATA(self) + start,
                              end - start,
                              needle.n_data,
                              needle.n_size);
 }
 if (!result) return_reference_(&DeeInt_MinusOne);
 return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
}

PRIVATE DREF DeeObject *DCALL
bytes_casefind(Bytes *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle;
 size_t start = 0,end = (size_t)-1; uint8_t *result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:casefind",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
  result = NULL;
 else {
  result = (uint8_t *)memcasememb(DeeBytes_DATA(self) + start,
                                  end - start,
                                  needle.n_data,
                                  needle.n_size);
 }
 if (!result) return_reference_(&DeeInt_MinusOne);
 return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
}

PRIVATE DREF DeeObject *DCALL
bytes_rfind(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle;
 size_t start = 0,end = (size_t)-1; uint8_t *result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:rfind",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
  result = NULL;
 else {
  result = (uint8_t *)memrmemb(DeeBytes_DATA(self) + start,
                               end - start,
                               needle.n_data,
                               needle.n_size);
 }
 if (!result) return_reference_(&DeeInt_MinusOne);
 return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
}

PRIVATE DREF DeeObject *DCALL
bytes_caserfind(Bytes *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle;
 size_t start = 0,end = (size_t)-1; uint8_t *result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caserfind",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
  result = NULL;
 else {
  result = (uint8_t *)memcasermemb(DeeBytes_DATA(self) + start,
                                   end - start,
                                   needle.n_data,
                                   needle.n_size);
 }
 if (!result) return_reference_(&DeeInt_MinusOne);
 return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
}

PRIVATE DREF DeeObject *DCALL
bytes_index(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle;
 size_t start = 0,end = (size_t)-1; uint8_t *result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:index",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
  result = NULL;
 else {
  result = (uint8_t *)memmemb(DeeBytes_DATA(self) + start,
                              end - start,
                              needle.n_data,
                              needle.n_size);
 }
 if (!result) {
  err_index_not_found((DeeObject *)self,find_ob);
  return NULL;
 }
 return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
}

PRIVATE DREF DeeObject *DCALL
bytes_caseindex(Bytes *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle;
 size_t start = 0,end = (size_t)-1; uint8_t *result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caseindex",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
  result = NULL;
 else {
  result = (uint8_t *)memcasememb(DeeBytes_DATA(self) + start,
                                  end - start,
                                  needle.n_data,
                                  needle.n_size);
 }
 if (!result) {
  err_index_not_found((DeeObject *)self,find_ob);
  return NULL;
 }
 return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
}

PRIVATE DREF DeeObject *DCALL
bytes_rindex(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle;
 size_t start = 0,end = (size_t)-1; uint8_t *result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:rindex",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
  result = NULL;
 else {
  result = (uint8_t *)memrmemb(DeeBytes_DATA(self) + start,
                               end - start,
                               needle.n_data,
                               needle.n_size);
 }
 if (!result) {
  err_index_not_found((DeeObject *)self,find_ob);
  return NULL;
 }
 return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
}

PRIVATE DREF DeeObject *DCALL
bytes_caserindex(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle;
 size_t start = 0,end = (size_t)-1; uint8_t *result;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caserindex",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
  result = NULL;
 else {
  result = (uint8_t *)memcasermemb(DeeBytes_DATA(self) + start,
                                   end - start,
                                   needle.n_data,
                                   needle.n_size);
 }
 if (!result) {
  err_index_not_found((DeeObject *)self,find_ob);
  return NULL;
 }
 return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
}

PRIVATE DREF DeeObject *DCALL
bytes_count(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle; size_t result;
 size_t start = 0,end = (size_t)-1;
 uint8_t *iter; size_t size;
 if (DeeArg_Unpack(argc,argv,"o|IdId:count",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 iter = DeeBytes_DATA(self);
 size = DeeBytes_SIZE(self);
 if (end > size)
     end = size;
 result = 0;
 if (start < end) {
  end -= start;
  iter += start;
  while (end >= needle.n_size) {
   if (MEMEQB(iter,needle.n_data,needle.n_size))
       ++result;
   --end;
   ++iter;
  }
 }
 return DeeInt_NewSize(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_casecount(Bytes *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle; size_t result;
 size_t start = 0,end = (size_t)-1;
 uint8_t *iter; size_t size;
 if (DeeArg_Unpack(argc,argv,"o|IdId:casecount",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 iter = DeeBytes_DATA(self);
 size = DeeBytes_SIZE(self);
 if (end > size)
     end = size;
 result = 0;
 if (start < end) {
  end -= start;
  iter += start;
  while (end >= needle.n_size) {
   if (MEMCASEEQB(iter,needle.n_data,needle.n_size))
       ++result;
   --end;
   ++iter;
  }
 }
 return DeeInt_NewSize(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_contains_f(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle;
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:contains",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
     return_false;
 return_bool(memmemb(DeeBytes_DATA(self) + start,
                     end - start,
                     needle.n_data,
                     needle.n_size) != NULL);
}

PRIVATE DREF DeeObject *DCALL
bytes_casecontains_f(Bytes *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle;
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:casecontains",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
     return_false;
 return_bool(memcasememb(DeeBytes_DATA(self) + start,
                         end - start,
                         needle.n_data,
                         needle.n_size) != NULL);
}

INTDEF dssize_t DCALL
DeeBytes_Format(dformatprinter printer,
                dformatprinter format_printer, void *arg,
                char const *__restrict format,
                size_t format_len, DeeObject *__restrict args);

PRIVATE DREF DeeObject *DCALL
bytes_format(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 DeeObject *args;
 if (DeeArg_Unpack(argc,argv,"o:format",&args))
     goto err;
 {
  struct bytes_printer printer = BYTES_PRINTER_INIT;
  if unlikely(DeeBytes_Format((dformatprinter)&bytes_printer_print,
                              (dformatprinter)&bytes_printer_append,
                              &printer,
                              (char *)DeeBytes_DATA(self),
                               DeeBytes_SIZE(self),
                               args) < 0)
     goto err_printer;
  return bytes_printer_pack(&printer);
err_printer:
  bytes_printer_fini(&printer);
 }
err:
 return NULL;
}



PRIVATE DREF Bytes *DCALL
bytes_substr(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:substr",&start,&end))
     goto err;
 if (end >= DeeBytes_SIZE(self)) {
  if (start == 0)
      return_reference_(self);
  end = DeeBytes_SIZE(self);
 }
 if (start >= end)
     return_reference_((DREF Bytes *)Dee_EmptyBytes);
 return (DREF Bytes *)DeeBytes_NewView(self->b_orig,
                                       self->b_base + (size_t)start,
                                      (size_t)(end-start),
                                       self->b_flags);
err:
 return NULL;
}

PRIVATE DREF Bytes *DCALL
bytes_resized(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DREF Bytes *result;
 size_t new_size;
 if (argc == 1) {
  if (DeeObject_AsSize(argv[0],&new_size))
      goto err;
  result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(new_size);
  if unlikely(!result) goto err;
  memcpy(result->b_data,
         DeeBytes_DATA(self),
         MIN(DeeBytes_SIZE(self),new_size));
 } else {
  uint8_t init;
  if (DeeArg_Unpack(argc,argv,"Iu|I8u:resized",&new_size,&init))
      goto err;
  result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(new_size);
  if unlikely(!result) goto err;
  if (new_size <= DeeBytes_SIZE(self)) {
   memcpy(result->b_data,DeeBytes_DATA(self),new_size);
  } else {
   size_t old_size = DeeBytes_SIZE(self);
   memcpy(result->b_data,DeeBytes_DATA(self),old_size);
   memset(result->b_data+old_size,init,new_size-old_size);
  }
 }
 return result;
err:
 return NULL;
}

PRIVATE DREF Bytes *DCALL
bytes_reversed(Bytes *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 DREF Bytes *result;
 uint8_t *data,*dst;
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:reversed",&start,&end))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (end <= start)
     return_reference_((DREF Bytes *)Dee_EmptyBytes);
 end -= start;
 result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(end);
 if unlikely(!result) goto done;
 data = DeeBytes_DATA(self);
 dst  = DeeBytes_DATA(result);
 do {
  --end;
  *dst++ = ((uint8_t *)data)[end];
 } while (end);
done:
 return result;
}

PRIVATE DREF Bytes *DCALL
bytes_reverse(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 uint8_t *data,*dst;
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:reverse",&start,&end))
     goto err;
 if unlikely(!DeeBytes_WRITABLE(self)) {
  err_bytes_not_writable((DeeObject *)self);
  goto err;
 }
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (end <= start)
     goto done;
 data = DeeBytes_DATA(self) + start;
 dst  = DeeBytes_DATA(self) + end;
 while (data < dst) {
  uint8_t temp = *data;
  *data++ = *--dst;
  *dst = temp;
 }
done:
 return_reference_(self);
err:
 return NULL;
}


PRIVATE DREF DeeObject *DCALL
bytes_ord(Bytes *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 size_t index = 0;
 if (argc) {
  if (DeeArg_Unpack(argc,argv,"Iu:ord",&index))
      goto err;
  if (index >= DeeBytes_SIZE(self)) {
   err_index_out_of_bounds((DeeObject *)self,
                            index,
                            DeeBytes_SIZE(self));
   goto err;
  }
 } else if unlikely(DeeBytes_SIZE(self) != 1) {
  err_expected_single_character_string((DeeObject *)self);
  goto err;
 }
 return DeeInt_NewU8(DeeBytes_DATA(self)[index]);
err:
 return NULL;
}

INTDEF DREF DeeObject *DCALL
DeeString_Scanf(DeeObject *__restrict self,
                DeeObject *__restrict format);
PRIVATE DREF DeeObject *DCALL
bytes_scanf(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DeeObject *format;
 if (DeeArg_Unpack(argc,argv,"o:scanf",&format) ||
     DeeObject_AssertTypeExact(format,&DeeString_Type))
     return NULL;
 return DeeString_Scanf((DeeObject *)self,format);
}


#define DeeBytes_IsPrint(self,start,end)      DeeBytes_TestTrait(self,start,end,UNICODE_FPRINT)
#define DeeBytes_IsAlpha(self,start,end)      DeeBytes_TestTrait(self,start,end,UNICODE_FALPHA)
#define DeeBytes_IsSpace(self,start,end)      DeeBytes_TestTrait(self,start,end,UNICODE_FSPACE)
#define DeeBytes_IsLF(self,start,end)         DeeBytes_TestTrait(self,start,end,UNICODE_FLF)
#define DeeBytes_IsLower(self,start,end)      DeeBytes_TestTrait(self,start,end,UNICODE_FLOWER)
#define DeeBytes_IsUpper(self,start,end)      DeeBytes_TestTrait(self,start,end,UNICODE_FUPPER)
#define DeeBytes_IsCntrl(self,start,end)      DeeBytes_TestTrait(self,start,end,UNICODE_FCNTRL)
#define DeeBytes_IsDigit(self,start,end)      DeeBytes_TestTrait(self,start,end,UNICODE_FDIGIT)
#define DeeBytes_IsDecimal(self,start,end)    DeeBytes_TestTrait(self,start,end,UNICODE_FDECIMAL)
#define DeeBytes_IsSymStrt(self,start,end)    DeeBytes_TestTrait(self,start,end,UNICODE_FSYMSTRT)
#define DeeBytes_IsSymCont(self,start,end)    DeeBytes_TestTrait(self,start,end,UNICODE_FSYMCONT)
#define DeeBytes_IsAlnum(self,start,end)      DeeBytes_TestTrait(self,start,end,UNICODE_FALPHA|UNICODE_FDIGIT)
#define DeeBytes_IsNumeric(self,start,end)    DeeBytes_TestTrait(self,start,end,UNICODE_FDIGIT|UNICODE_FDECIMAL)
#define DeeBytes_IsAnyPrint(self,start,end)   DeeBytes_TestAnyTrait(self,start,end,UNICODE_FPRINT)
#define DeeBytes_IsAnyAlpha(self,start,end)   DeeBytes_TestAnyTrait(self,start,end,UNICODE_FALPHA)
#define DeeBytes_IsAnySpace(self,start,end)   DeeBytes_TestAnyTrait(self,start,end,UNICODE_FSPACE)
#define DeeBytes_IsAnyLF(self,start,end)      DeeBytes_TestAnyTrait(self,start,end,UNICODE_FLF)
#define DeeBytes_IsAnyLower(self,start,end)   DeeBytes_TestAnyTrait(self,start,end,UNICODE_FLOWER)
#define DeeBytes_IsAnyUpper(self,start,end)   DeeBytes_TestAnyTrait(self,start,end,UNICODE_FUPPER)
#define DeeBytes_IsAnyTitle(self,start,end)   DeeBytes_TestAnyTrait(self,start,end,UNICODE_FTITLE)
#define DeeBytes_IsAnyCntrl(self,start,end)   DeeBytes_TestAnyTrait(self,start,end,UNICODE_FCNTRL)
#define DeeBytes_IsAnyDigit(self,start,end)   DeeBytes_TestAnyTrait(self,start,end,UNICODE_FDIGIT)
#define DeeBytes_IsAnyDecimal(self,start,end) DeeBytes_TestAnyTrait(self,start,end,UNICODE_FDECIMAL)
#define DeeBytes_IsAnySymStrt(self,start,end) DeeBytes_TestAnyTrait(self,start,end,UNICODE_FSYMSTRT)
#define DeeBytes_IsAnySymCont(self,start,end) DeeBytes_TestAnyTrait(self,start,end,UNICODE_FSYMCONT)
#define DeeBytes_IsAnyAlnum(self,start,end)   DeeBytes_TestAnyTrait(self,start,end,UNICODE_FALPHA|UNICODE_FDIGIT)
#define DeeBytes_IsAnyNumeric(self,start,end) DeeBytes_TestAnyTrait(self,start,end,UNICODE_FDIGIT|UNICODE_FDECIMAL)

INTERN bool DCALL
DeeBytes_TestTrait(Bytes *__restrict self,
                   size_t start_index, size_t end_index,
                   uniflag_t flags) {
 uint8_t *iter;
 if (start_index > DeeBytes_SIZE(self))
     start_index = DeeBytes_SIZE(self);
 if (end_index > DeeBytes_SIZE(self))
     end_index = DeeBytes_SIZE(self);
 iter = DeeBytes_DATA(self);
 while (start_index < end_index) {
  if (!(DeeUni_Flags(*iter) & flags))
        return false;
  ++iter,++start_index;
 }
 return true;
}
INTERN bool DCALL
DeeBytes_TestAnyTrait(Bytes *__restrict self,
                      size_t start_index,
                      size_t end_index,
                      uniflag_t flags) {
 uint8_t *iter;
 if (end_index > DeeBytes_SIZE(self))
     end_index = DeeBytes_SIZE(self);
 if (start_index < end_index) {
  iter = DeeBytes_DATA(self) + start_index;
  while (start_index < end_index) {
   if (DeeUni_Flags(*iter) & flags)
        return true;
   ++iter;
   ++start_index;
  }
 }
 return false;
}


INTERN bool DCALL
DeeBytes_IsTitle(Bytes *__restrict self,
                 size_t start_index,
                 size_t end_index) {
 uniflag_t flags = (UNICODE_FTITLE|UNICODE_FUPPER|UNICODE_FSPACE);
 uint8_t *iter;
 if (end_index > DeeBytes_SIZE(self))
     end_index = DeeBytes_SIZE(self);
 if (start_index < end_index) {
  iter = DeeBytes_DATA(self) + start_index;
  while (start_index < end_index) {
   uniflag_t f = DeeUni_Flags(*iter);
   if (!(f & flags)) return false;
   flags = (f&UNICODE_FSPACE) ? (UNICODE_FTITLE|UNICODE_FUPPER|UNICODE_FSPACE)
                              : (UNICODE_FLOWER|UNICODE_FSPACE);
   ++iter;
   ++start_index;
  }
 }
 return true;
}
INTERN bool DCALL
DeeBytes_IsSymbol(Bytes *__restrict self,
                  size_t start_index,
                  size_t end_index) {
 uniflag_t flags = (UNICODE_FSYMSTRT|UNICODE_FALPHA);
 uint8_t *iter;
 if (end_index > DeeBytes_SIZE(self))
     end_index = DeeBytes_SIZE(self);
 if (start_index < end_index) {
  iter = DeeBytes_DATA(self) + start_index;
  while (start_index < end_index) {
   if (!(DeeUni_Flags(*iter) & flags))
         return false;
   flags |= (UNICODE_FSYMCONT|UNICODE_FDIGIT);
   ++iter;
   ++start_index;
  }
 }
 return true;
}

#define DEFINE_BYTES_TRAIT(name,function,flag) \
PRIVATE DREF DeeObject *DCALL \
bytes_##name(Bytes *__restrict self, \
             size_t argc, DeeObject **__restrict argv) { \
 size_t start = 0,end = (size_t)-1; \
 if (argc == 1) { \
  uint8_t ch; \
  if (DeeObject_AsSize(argv[0],&start)) \
      goto err; \
  if unlikely(start >= DeeBytes_SIZE(self)) { \
   err_index_out_of_bounds((DeeObject *)self, \
                            start, \
                            DeeBytes_SIZE(self)); \
   goto err; \
  } \
  ch = DeeBytes_DATA(self)[start]; \
  return_bool(DeeUni_Flags(ch) & (flag)); \
 } else { \
  if (DeeArg_Unpack(argc,argv,"|IdId" #name,&start,&end)) \
      goto err; \
  return_bool(function(self,start,end)); \
 } \
err: \
 return NULL; \
}
#define DEFINE_ANY_BYTES_TRAIT(name,function) \
PRIVATE DREF DeeObject *DCALL \
bytes_##name(Bytes *__restrict self, \
             size_t argc, DeeObject **__restrict argv) { \
 size_t start = 0,end = (size_t)-1; \
 if (DeeArg_Unpack(argc,argv,"|IdId" #name,&start,&end)) \
     return NULL; \
 return_bool(function(self,start,end)); \
}
DEFINE_BYTES_TRAIT(isprint,DeeBytes_IsPrint,UNICODE_FPRINT)
DEFINE_BYTES_TRAIT(isalpha,DeeBytes_IsAlpha,UNICODE_FALPHA)
DEFINE_BYTES_TRAIT(isspace,DeeBytes_IsSpace,UNICODE_FSPACE)
DEFINE_BYTES_TRAIT(islf,DeeBytes_IsLF,UNICODE_FLF)
DEFINE_BYTES_TRAIT(islower,DeeBytes_IsLower,UNICODE_FLOWER)
DEFINE_BYTES_TRAIT(isupper,DeeBytes_IsUpper,UNICODE_FUPPER)
DEFINE_BYTES_TRAIT(iscntrl,DeeBytes_IsCntrl,UNICODE_FCNTRL)
DEFINE_BYTES_TRAIT(isdigit,DeeBytes_IsDigit,UNICODE_FDIGIT)
DEFINE_BYTES_TRAIT(isdecimal,DeeBytes_IsDecimal,UNICODE_FDECIMAL)
DEFINE_BYTES_TRAIT(issymstrt,DeeBytes_IsSymStrt,UNICODE_FSYMSTRT)
DEFINE_BYTES_TRAIT(issymcont,DeeBytes_IsSymStrt,UNICODE_FSYMCONT)
DEFINE_BYTES_TRAIT(isalnum,DeeBytes_IsAlnum,UNICODE_FALPHA|UNICODE_FDIGIT)
DEFINE_BYTES_TRAIT(isnumeric,DeeBytes_IsNumeric,UNICODE_FDECIMAL|UNICODE_FDIGIT)
DEFINE_BYTES_TRAIT(istitle,DeeBytes_IsTitle,UNICODE_FTITLE)
DEFINE_BYTES_TRAIT(issymbol,DeeBytes_IsSymbol,UNICODE_FSYMSTRT)
DEFINE_ANY_BYTES_TRAIT(isanyprint,DeeBytes_IsAnyPrint)
DEFINE_ANY_BYTES_TRAIT(isanyalpha,DeeBytes_IsAnyAlpha)
DEFINE_ANY_BYTES_TRAIT(isanyspace,DeeBytes_IsAnySpace)
DEFINE_ANY_BYTES_TRAIT(isanylf,DeeBytes_IsAnyLF)
DEFINE_ANY_BYTES_TRAIT(isanylower,DeeBytes_IsAnyLower)
DEFINE_ANY_BYTES_TRAIT(isanyupper,DeeBytes_IsAnyUpper)
DEFINE_ANY_BYTES_TRAIT(isanycntrl,DeeBytes_IsAnyCntrl)
DEFINE_ANY_BYTES_TRAIT(isanydigit,DeeBytes_IsAnyDigit)
DEFINE_ANY_BYTES_TRAIT(isanydecimal,DeeBytes_IsAnyDecimal)
DEFINE_ANY_BYTES_TRAIT(isanysymstrt,DeeBytes_IsAnySymStrt)
DEFINE_ANY_BYTES_TRAIT(isanysymcont,DeeBytes_IsAnySymStrt)
DEFINE_ANY_BYTES_TRAIT(isanyalnum,DeeBytes_IsAnyAlnum)
DEFINE_ANY_BYTES_TRAIT(isanynumeric,DeeBytes_IsAnyNumeric)
DEFINE_ANY_BYTES_TRAIT(isanytitle,DeeBytes_IsAnyTitle)
#undef DEFINE_ANY_BYTES_TRAIT
#undef DEFINE_BYTES_TRAIT


PRIVATE DREF Bytes *DCALL
bytes_lower(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 size_t i,start = 0,end = (size_t)-1; DREF Bytes *result;
 if (DeeArg_Unpack(argc,argv,"|IdId:lower",&start,&end))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
     return_reference_((DREF Bytes *)Dee_EmptyBytes);
 end -= start;
 result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(end);
 if unlikely(!result) goto done;
 for (i = 0; i < end; ++i)
     DeeBytes_DATA(result)[i] = (uint8_t)DeeUni_ToLower(DeeBytes_DATA(self)[start + i]);
done:
 return result;
}
PRIVATE DREF Bytes *DCALL
bytes_upper(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 size_t i,start = 0,end = (size_t)-1; DREF Bytes *result;
 if (DeeArg_Unpack(argc,argv,"|IdId:upper",&start,&end))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
     return_reference_((DREF Bytes *)Dee_EmptyBytes);
 end -= start;
 result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(end);
 if unlikely(!result) goto done;
 for (i = 0; i < end; ++i)
     DeeBytes_DATA(result)[i] = (uint8_t)DeeUni_ToUpper(DeeBytes_DATA(self)[start + i]);
done:
 return result;
}
PRIVATE DREF Bytes *DCALL
bytes_title(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 uintptr_t kind = UNICODE_CONVERT_TITLE;
 size_t i,start = 0,end = (size_t)-1; DREF Bytes *result;
 if (DeeArg_Unpack(argc,argv,"|IdId:title",&start,&end))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
     return_reference_((DREF Bytes *)Dee_EmptyBytes);
 end -= start;
 result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(end);
 if unlikely(!result) goto done;
 for (i = 0; i < end; ++i) {
  uint8_t ch = DeeBytes_DATA(self)[start + i];
  DeeBytes_DATA(result)[i] = kind == UNICODE_CONVERT_TITLE
                           ? (uint8_t)DeeUni_ToTitle(ch)
                           : (uint8_t)DeeUni_ToLower(ch)
                           ;
  kind = DeeUni_IsSpace(ch) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
 }
done:
 return result;
}
PRIVATE DREF Bytes *DCALL
bytes_capitalize(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 size_t i,start = 0,end = (size_t)-1; DREF Bytes *result;
 if (DeeArg_Unpack(argc,argv,"|IdId:capitalize",&start,&end))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
     return_reference_((DREF Bytes *)Dee_EmptyBytes);
 end -= start;
 result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(end);
 if unlikely(!result) goto done;
 DeeBytes_DATA(result)[0] = (uint8_t)DeeUni_ToUpper(DeeBytes_DATA(self)[start]);
 for (i = 1; i < end; ++i)
     DeeBytes_DATA(result)[i] = (uint8_t)DeeUni_ToLower(DeeBytes_DATA(self)[start + i]);
done:
 return result;
}
PRIVATE DREF Bytes *DCALL
bytes_swapcase(Bytes *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 size_t i,start = 0,end = (size_t)-1; DREF Bytes *result;
 if (DeeArg_Unpack(argc,argv,"|IdId:swapcase",&start,&end))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
     return_reference_((DREF Bytes *)Dee_EmptyBytes);
 end -= start;
 result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(end);
 if unlikely(!result) goto done;
 for (i = 0; i < end; ++i)
     DeeBytes_DATA(result)[i] = (uint8_t)DeeUni_SwapCase(DeeBytes_DATA(self)[start + i]);
done:
 return result;
}



PRIVATE DREF Bytes *DCALL
bytes_tolower(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 size_t i,start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:tolower",&start,&end))
     goto err;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if unlikely(!DeeBytes_WRITABLE(self) && start < end) {
  err_bytes_not_writable((DeeObject *)self);
  goto err;
 }
 for (i = start; i < end; ++i)
     DeeBytes_DATA(self)[i] = (uint8_t)DeeUni_ToLower(DeeBytes_DATA(self)[i]);
 return_reference_(self);
err:
 return NULL;
}
PRIVATE DREF Bytes *DCALL
bytes_toupper(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 size_t i,start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:toupper",&start,&end))
     goto err;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if unlikely(!DeeBytes_WRITABLE(self) && start < end) {
  err_bytes_not_writable((DeeObject *)self);
  goto err;
 }
 for (i = start; i < end; ++i)
     DeeBytes_DATA(self)[i] = (uint8_t)DeeUni_ToUpper(DeeBytes_DATA(self)[i]);
 return_reference_(self);
err:
 return NULL;
}
PRIVATE DREF Bytes *DCALL
bytes_totitle(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 uintptr_t kind = UNICODE_CONVERT_TITLE;
 size_t i,start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:totitle",&start,&end))
     goto err;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if unlikely(!DeeBytes_WRITABLE(self) && start < end) {
  err_bytes_not_writable((DeeObject *)self);
  goto err;
 }
 for (i = start; i < end; ++i) {
  uint8_t ch = DeeBytes_DATA(self)[i];
  DeeBytes_DATA(self)[i] = kind == UNICODE_CONVERT_TITLE
                         ? (uint8_t)DeeUni_ToTitle(ch)
                         : (uint8_t)DeeUni_ToLower(ch)
                         ;
  kind = DeeUni_IsSpace(ch) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
 }
 return_reference_(self);
err:
 return NULL;
}
PRIVATE DREF Bytes *DCALL
bytes_tocapitalize(Bytes *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:tocapitalize",&start,&end))
     goto err;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start < end) {
  size_t i = start;
  if unlikely(!DeeBytes_WRITABLE(self)) {
   err_bytes_not_writable((DeeObject *)self);
   goto err;
  }
  DeeBytes_DATA(self)[i] = (uint8_t)DeeUni_ToUpper(DeeBytes_DATA(self)[i]);
  ++i;
  for (; i < end; ++i)
      DeeBytes_DATA(self)[i] = (uint8_t)DeeUni_ToLower(DeeBytes_DATA(self)[i]);
 }
 return_reference_(self);
err:
 return NULL;
}
PRIVATE DREF Bytes *DCALL
bytes_toswapcase(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 size_t i,start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|IdId:toswapcase",&start,&end))
     goto err;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if unlikely(!DeeBytes_WRITABLE(self) && start < end) {
  err_bytes_not_writable((DeeObject *)self);
  goto err;
 }
 for (i = start; i < end; ++i)
     DeeBytes_DATA(self)[i] = (uint8_t)DeeUni_SwapCase(DeeBytes_DATA(self)[i]);
 return_reference_(self);
err:
 return NULL;
}


PRIVATE DREF Bytes *DCALL
bytes_replace(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DREF Bytes *result; uint8_t *begin,*end,*block_begin;
 DeeObject *find_ob,*replace_ob; size_t max_count = (size_t)-1;
 Needle find_needle,replace_needle; struct bytes_printer printer;
 if (DeeArg_Unpack(argc,argv,"oo|Iu:replace",&find_ob,&replace_ob,&max_count) ||
     get_needle(&find_needle,find_ob) || get_needle(&replace_needle,replace_ob))
     return NULL;
 /* Handle special cases. */
 if unlikely(find_needle.n_size > DeeBytes_SIZE(self))
    goto return_self;
 if unlikely(!find_needle.n_size) {
  if (DeeBytes_SIZE(self)) goto return_self;
  result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(replace_needle.n_size);
  if likely(result) memcpy(result->b_data,replace_needle.n_data,replace_needle.n_size);
  return result;
 }
 bytes_printer_init(&printer);
 end = (begin = DeeBytes_DATA(self))+(DeeBytes_SIZE(self)-(find_needle.n_size-1));
 block_begin = begin;
 if likely(max_count) while (begin <= end) {
  if (MEMEQB(begin,find_needle.n_data,find_needle.n_size)) {
   /* Found one */
   if (unlikely(bytes_printer_append(&printer,block_begin,(size_t)(begin-block_begin)) < 0) ||
       unlikely(bytes_printer_append(&printer,replace_needle.n_data,replace_needle.n_size) < 0))
       goto err;
   begin += find_needle.n_size;
   block_begin = begin;
   if (begin >= end) break;
   if unlikely(!--max_count) break;
   continue;
  }
  ++begin;
 }
 if unlikely(bytes_printer_append(&printer,block_begin,
                                 (size_t)((end-block_begin)+
                                          (find_needle.n_size-1))) < 0)
    goto err;
 /* Pack together a bytes object. */
 return (DREF Bytes *)bytes_printer_pack(&printer);
err:
 bytes_printer_fini(&printer);
 return NULL;
return_self:
 result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(DeeBytes_SIZE(self));
 if likely(result) memcpy(result->b_data,DeeBytes_DATA(self),DeeBytes_SIZE(self));
 return result;
}

PRIVATE DREF Bytes *DCALL
bytes_casereplace(Bytes *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 DREF Bytes *result; uint8_t *begin,*end,*block_begin;
 DeeObject *find_ob,*replace_ob; size_t max_count = (size_t)-1;
 Needle find_needle,replace_needle; struct bytes_printer printer;
 if (DeeArg_Unpack(argc,argv,"oo|Iu:casereplace",&find_ob,&replace_ob,&max_count) ||
     get_needle(&find_needle,find_ob) || get_needle(&replace_needle,replace_ob))
     return NULL;
 /* Handle special cases. */
 if unlikely(find_needle.n_size > DeeBytes_SIZE(self))
    goto return_self;
 if unlikely(!find_needle.n_size) {
  if (DeeBytes_SIZE(self)) goto return_self;
  result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(replace_needle.n_size);
  if likely(result) memcpy(result->b_data,replace_needle.n_data,replace_needle.n_size);
  return result;
 }
 bytes_printer_init(&printer);
 end = (begin = DeeBytes_DATA(self))+(DeeBytes_SIZE(self)-(find_needle.n_size-1));
 block_begin = begin;
 if likely(max_count) while (begin <= end) {
  if (MEMCASEEQB(begin,find_needle.n_data,find_needle.n_size)) {
   /* Found one */
   if (unlikely(bytes_printer_append(&printer,block_begin,(size_t)(begin-block_begin)) < 0) ||
       unlikely(bytes_printer_append(&printer,replace_needle.n_data,replace_needle.n_size) < 0))
       goto err;
   begin += find_needle.n_size;
   block_begin = begin;
   if (begin >= end) break;
   if unlikely(!--max_count) break;
   continue;
  }
  ++begin;
 }
 if unlikely(bytes_printer_append(&printer,block_begin,
                                 (size_t)((end-block_begin)+
                                          (find_needle.n_size-1))) < 0)
    goto err;
 /* Pack together a bytes object. */
 return (DREF Bytes *)bytes_printer_pack(&printer);
err:
 bytes_printer_fini(&printer);
 return NULL;
return_self:
 result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(DeeBytes_SIZE(self));
 if likely(result) memcpy(result->b_data,DeeBytes_DATA(self),DeeBytes_SIZE(self));
 return result;
}


PRIVATE DREF Bytes *DCALL
bytes_toreplace(Bytes *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob,*replace_ob; size_t max_count = (size_t)-1;
 Needle find_needle,replace_needle; uint8_t *begin,*end;
 if (DeeArg_Unpack(argc,argv,"oo|Iu:toreplace",&find_ob,&replace_ob,&max_count) ||
     get_needle(&find_needle,find_ob) || get_needle(&replace_needle,replace_ob))
     goto err;
 if unlikely(find_needle.n_size != replace_needle.n_size) {
  DeeError_Throwf(&DeeError_ValueError,
                  "Find(%Iu) and replace(%Iu) needles have different sizes",
                  find_needle.n_size,replace_needle.n_size);
  goto err;
 }
 if unlikely(!DeeBytes_WRITABLE(self)) {
  err_bytes_not_writable((DeeObject *)self);
  goto err;
 }

 /* Handle special cases. */
 if unlikely(find_needle.n_size > DeeBytes_SIZE(self))
    goto done;
 if unlikely(!find_needle.n_size)
    goto done;

 end = (begin = DeeBytes_DATA(self))+(DeeBytes_SIZE(self)-(find_needle.n_size-1));
 if likely(max_count) while (begin <= end) {
  if (MEMEQB(begin,find_needle.n_data,find_needle.n_size)) {
   /* Found one */
   memcpy(begin,replace_needle.n_data,replace_needle.n_size);
   begin += find_needle.n_size;
   if (begin >= end) break;
   if unlikely(!--max_count) break;
   continue;
  }
  ++begin;
 }
done:
 return_reference_(self);
err:
 return NULL;
}

PRIVATE DREF Bytes *DCALL
bytes_tocasereplace(Bytes *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob,*replace_ob; size_t max_count = (size_t)-1;
 Needle find_needle,replace_needle; uint8_t *begin,*end;
 if (DeeArg_Unpack(argc,argv,"oo|Iu:tocasereplace",&find_ob,&replace_ob,&max_count) ||
     get_needle(&find_needle,find_ob) || get_needle(&replace_needle,replace_ob))
     goto err;
 if unlikely(find_needle.n_size != replace_needle.n_size) {
  DeeError_Throwf(&DeeError_ValueError,
                  "Find(%Iu) and replace(%Iu) needles have different sizes",
                  find_needle.n_size,replace_needle.n_size);
  goto err;
 }
 if unlikely(!DeeBytes_WRITABLE(self)) {
  err_bytes_not_writable((DeeObject *)self);
  goto err;
 }

 /* Handle special cases. */
 if unlikely(find_needle.n_size > DeeBytes_SIZE(self))
    goto done;
 if unlikely(!find_needle.n_size)
    goto done;

 end = (begin = DeeBytes_DATA(self))+(DeeBytes_SIZE(self)-(find_needle.n_size-1));
 if likely(max_count) while (begin <= end) {
  if (MEMCASEEQB(begin,find_needle.n_data,find_needle.n_size)) {
   /* Found one */
   memcpy(begin,replace_needle.n_data,replace_needle.n_size);
   begin += find_needle.n_size;
   if (begin >= end) break;
   if unlikely(!--max_count) break;
   continue;
  }
  ++begin;
 }
done:
 return_reference_(self);
err:
 return NULL;
}


/* The string decode() and encode() member functions also function for `bytes' objects.
 * As a matter of fact: they'd work for any kind of object, however built-in
 *                      codecs only function for bytes and string objects! */
INTDEF DREF DeeObject *DCALL
string_decode(DeeObject *__restrict self,
              size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL
string_encode(DeeObject *__restrict self,
              size_t argc, DeeObject **__restrict argv);


INTDEF DREF DeeObject *DCALL DeeBytes_SplitByte(Bytes *__restrict self, uint8_t sep);
INTDEF DREF DeeObject *DCALL DeeBytes_Split(Bytes *__restrict self, DeeObject *__restrict sep);
INTDEF DREF DeeObject *DCALL DeeBytes_CaseSplitByte(Bytes *__restrict self, uint8_t sep);
INTDEF DREF DeeObject *DCALL DeeBytes_CaseSplit(Bytes *__restrict self, DeeObject *__restrict sep);
INTDEF DREF DeeObject *DCALL DeeBytes_SplitLines(Bytes *__restrict self, bool keepends);


PRIVATE DREF DeeObject *DCALL
bytes_findall(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 Needle needle; DeeObject *arg;
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:findall",&arg,&start,&end) ||
     get_needle(&needle,arg))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
     return_empty_seq;
 /* TODO: Proxy-sequence for finding all needles in self[start:end] */
 DERROR_NOTIMPLEMENTED();
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casefindall(Bytes *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 Needle needle; DeeObject *arg;
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:casefindall",&arg,&start,&end) ||
     get_needle(&needle,arg))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
     return_empty_seq;
 /* TODO: Proxy-sequence for finding all needles in self[start:end] */
 DERROR_NOTIMPLEMENTED();
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_split(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DeeObject *other; uint8_t sep;
 if (DeeArg_Unpack(argc,argv,"o:split",&other))
     goto err;
 if (DeeString_Check(other) || DeeBytes_Check(other))
     return DeeBytes_Split(self,other);
 if (DeeObject_AsUInt8(other,&sep))
     goto err;
 return DeeBytes_SplitByte(self,sep);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casesplit(Bytes *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 DeeObject *other; uint8_t sep;
 if (DeeArg_Unpack(argc,argv,"o:casesplit",&other))
     goto err;
 if (DeeString_Check(other) || DeeBytes_Check(other))
     return DeeBytes_CaseSplit(self,other);
 if (DeeObject_AsUInt8(other,&sep))
     goto err;
 return DeeBytes_CaseSplitByte(self,sep);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_splitlines(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 bool keepends = false;
 if (DeeArg_Unpack(argc,argv,"|b:splitlines",&keepends))
     return NULL;
 return DeeBytes_SplitLines(self,keepends);
}

PRIVATE DREF DeeObject *DCALL
bytes_startswith(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 Needle needle; DeeObject *arg;
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:startswith",&arg,&start,&end) ||
     get_needle(&needle,arg))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start > end ||
    (end -= start) < needle.n_size)
     return_false;
 return_bool(MEMEQB(DeeBytes_DATA(self) + start,
                    needle.n_data,needle.n_size));
}

PRIVATE DREF DeeObject *DCALL
bytes_casestartswith(Bytes *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
 Needle needle; DeeObject *arg;
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:casestartswith",&arg,&start,&end) ||
     get_needle(&needle,arg))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start > end ||
    (end -= start) < needle.n_size)
     return_false;
 return_bool(MEMCASEEQB(DeeBytes_DATA(self) + start,
                        needle.n_data,needle.n_size));
}

PRIVATE DREF DeeObject *DCALL
bytes_endswith(Bytes *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 Needle needle; DeeObject *arg;
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:endswith",&arg,&start,&end) ||
     get_needle(&needle,arg))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start > end ||
    (end - start) < needle.n_size)
     return_false;
 return_bool(MEMEQB(DeeBytes_DATA(self) +
                   (end - needle.n_size),
                    needle.n_data,needle.n_size));
}

PRIVATE DREF DeeObject *DCALL
bytes_caseendswith(Bytes *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 Needle needle; DeeObject *arg;
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caseendswith",&arg,&start,&end) ||
     get_needle(&needle,arg))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start > end ||
    (end - start) < needle.n_size)
     return_false;
 return_bool(MEMCASEEQB(DeeBytes_DATA(self) +
                       (end - needle.n_size),
                        needle.n_data,needle.n_size));
}

PRIVATE DREF DeeObject *DCALL
bytes_pack_partition(Bytes *__restrict self, uint8_t *find_ptr,
                     uint8_t *__restrict start_ptr, size_t search_size,
                     size_t needle_len) {
 DREF DeeObject *result,*temp;
 if (!find_ptr)
      return DeeTuple_Pack(3,self,Dee_EmptyBytes,Dee_EmptyBytes);
 result = DeeTuple_NewUninitialized(3);
 if unlikely(!result) goto done;
 temp = DeeBytes_NewView(self->b_orig,start_ptr,
                        (size_t)(find_ptr - start_ptr),
                         self->b_flags);
 if unlikely(!temp) goto err_r_0;
 DeeTuple_SET(result,0,temp); /* Inherit reference. */
 temp = DeeBytes_NewView(self->b_orig,find_ptr,
                         needle_len,self->b_flags);
 if unlikely(!temp) goto err_r_1;
 DeeTuple_SET(result,1,temp); /* Inherit reference. */
 find_ptr += needle_len;
 temp = DeeBytes_NewView(self->b_orig,find_ptr,
                        (start_ptr + search_size) - find_ptr,
                         self->b_flags);
 if unlikely(!temp) goto err_r_2;
 DeeTuple_SET(result,2,temp); /* Inherit reference. */
done:
 return result;
err_r_2:
 Dee_Decref(DeeTuple_GET(result,1));
err_r_1:
 Dee_Decref(DeeTuple_GET(result,0));
err_r_0:
 DeeTuple_FreeUninitialized(result);
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_parition(Bytes *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle;
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:partition",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
     return DeeTuple_Pack(3,Dee_EmptyBytes,Dee_EmptyBytes,Dee_EmptyBytes);
 end -= start;
 return bytes_pack_partition(self,
                             memmemb(DeeBytes_DATA(self) + start,
                                     end,
                                     needle.n_data,
                                     needle.n_size),
                             DeeBytes_DATA(self) + start,
                             end,
                             needle.n_size);
}
PRIVATE DREF DeeObject *DCALL
bytes_caseparition(Bytes *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle;
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:casepartition",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
     return DeeTuple_Pack(3,Dee_EmptyBytes,Dee_EmptyBytes,Dee_EmptyBytes);
 end -= start;
 return bytes_pack_partition(self,
                             memcasememb(DeeBytes_DATA(self) + start,
                                         end,
                                         needle.n_data,
                                         needle.n_size),
                             DeeBytes_DATA(self) + start,
                             end,
                             needle.n_size);
}
PRIVATE DREF DeeObject *DCALL
bytes_rparition(Bytes *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle;
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:rpartition",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
     return DeeTuple_Pack(3,Dee_EmptyBytes,Dee_EmptyBytes,Dee_EmptyBytes);
 end -= start;
 return bytes_pack_partition(self,
                             memrmemb(DeeBytes_DATA(self) + start,
                                      end,
                                      needle.n_data,
                                      needle.n_size),
                             DeeBytes_DATA(self) + start,
                             end,
                             needle.n_size);
}
PRIVATE DREF DeeObject *DCALL
bytes_caserparition(Bytes *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 DeeObject *find_ob; Needle needle;
 size_t start = 0,end = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|IdId:caserpartition",&find_ob,&start,&end) ||
     get_needle(&needle,find_ob))
     return NULL;
 if (end > DeeBytes_SIZE(self))
     end = DeeBytes_SIZE(self);
 if (start >= end)
     return DeeTuple_Pack(3,Dee_EmptyBytes,Dee_EmptyBytes,Dee_EmptyBytes);
 end -= start;
 return bytes_pack_partition(self,
                             memcasermemb(DeeBytes_DATA(self) + start,
                                          end,
                                          needle.n_data,
                                          needle.n_size),
                             DeeBytes_DATA(self) + start,
                             end,
                             needle.n_size);
}

PRIVATE DREF DeeObject *DCALL
bytes_strip(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 uint8_t *begin,*end; DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:strip",&mask))
     goto err;
 begin = DeeBytes_DATA(self);
 end  = begin + DeeBytes_SIZE(self);
 if (mask) {
  Needle needle;
  if (get_needle(&needle,mask))
      goto err;
  while (begin < end && memchr(needle.n_data,*begin,needle.n_size)) ++begin;
  while (end > begin && memchr(needle.n_data,end[-1],needle.n_size)) --end;
 } else {
  while (begin < end && DeeUni_IsSpace(*begin)) ++begin;
  while (end > begin && DeeUni_IsSpace(end[-1])) --end;
 }
 if (begin == DeeBytes_DATA(self) &&
     end  == begin + DeeBytes_SIZE(self))
     return_reference_((DeeObject *)self);
 return DeeBytes_NewView(self->b_orig,
                         begin,
                        (size_t)(end - begin),
                         self->b_flags);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casestrip(Bytes *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 uint8_t *begin,*end; DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:casestrip",&mask))
     goto err;
 begin = DeeBytes_DATA(self);
 end  = begin + DeeBytes_SIZE(self);
 if (mask) {
  Needle needle;
  if (get_needle(&needle,mask))
      goto err;
  while (begin < end && memcasechr(needle.n_data,*begin,needle.n_size)) ++begin;
  while (end > begin && memcasechr(needle.n_data,end[-1],needle.n_size)) --end;
 } else {
  while (begin < end && DeeUni_IsSpace(*begin)) ++begin;
  while (end > begin && DeeUni_IsSpace(end[-1])) --end;
 }
 if (begin == DeeBytes_DATA(self) &&
     end  == begin + DeeBytes_SIZE(self))
     return_reference_((DeeObject *)self);
 return DeeBytes_NewView(self->b_orig,
                         begin,
                        (size_t)(end - begin),
                         self->b_flags);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_lstrip(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 uint8_t *begin,*end; DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:lstrip",&mask))
     goto err;
 begin = DeeBytes_DATA(self);
 end  = begin + DeeBytes_SIZE(self);
 if (mask) {
  Needle needle;
  if (get_needle(&needle,mask))
      goto err;
  while (begin < end && memchr(needle.n_data,*begin,needle.n_size)) ++begin;
 } else {
  while (begin < end && DeeUni_IsSpace(*begin)) ++begin;
 }
 if (begin == DeeBytes_DATA(self))
     return_reference_((DeeObject *)self);
 return DeeBytes_NewView(self->b_orig,
                         begin,
                        (size_t)(end - begin),
                         self->b_flags);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_caselstrip(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 uint8_t *begin,*end; DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:caselstrip",&mask))
     goto err;
 begin = DeeBytes_DATA(self);
 end  = begin + DeeBytes_SIZE(self);
 if (mask) {
  Needle needle;
  if (get_needle(&needle,mask))
      goto err;
  while (begin < end && memcasechr(needle.n_data,*begin,needle.n_size)) ++begin;
 } else {
  while (begin < end && DeeUni_IsSpace(*begin)) ++begin;
 }
 if (begin == DeeBytes_DATA(self))
     return_reference_((DeeObject *)self);
 return DeeBytes_NewView(self->b_orig,
                         begin,
                        (size_t)(end - begin),
                         self->b_flags);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_rstrip(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 uint8_t *begin,*end; DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:rstrip",&mask))
     goto err;
 begin = DeeBytes_DATA(self);
 end  = begin + DeeBytes_SIZE(self);
 if (mask) {
  Needle needle;
  if (get_needle(&needle,mask))
      goto err;
  while (end > begin && memchr(needle.n_data,end[-1],needle.n_size)) --end;
 } else {
  while (end > begin && DeeUni_IsSpace(end[-1])) --end;
 }
 if (end == begin + DeeBytes_SIZE(self))
     return_reference_((DeeObject *)self);
 return DeeBytes_NewView(self->b_orig,
                         begin,
                        (size_t)(end - begin),
                         self->b_flags);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_caserstrip(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 uint8_t *begin,*end; DeeObject *mask = NULL;
 if (DeeArg_Unpack(argc,argv,"|o:caserstrip",&mask))
     goto err;
 begin = DeeBytes_DATA(self);
 end  = begin + DeeBytes_SIZE(self);
 if (mask) {
  Needle needle;
  if (get_needle(&needle,mask))
      goto err;
  while (end > begin && memcasechr(needle.n_data,end[-1],needle.n_size)) --end;
 } else {
  while (end > begin && DeeUni_IsSpace(end[-1])) --end;
 }
 if (end == begin + DeeBytes_SIZE(self))
     return_reference_((DeeObject *)self);
 return DeeBytes_NewView(self->b_orig,
                         begin,
                        (size_t)(end - begin),
                         self->b_flags);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_sstrip(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 uint8_t *begin; DeeObject *mask;
 Needle needle; size_t size;
 if (DeeArg_Unpack(argc,argv,"o:sstrip",&mask))
     goto err;
 if (get_needle(&needle,mask))
     goto err;
 if (needle.n_size) goto retself;
 begin = DeeBytes_DATA(self);
 size  = DeeBytes_SIZE(self);
 while (size >= needle.n_size) {
  if (!MEMEQB(begin,needle.n_data,needle.n_size))
       break;
  begin += needle.n_size;
  size  -= needle.n_size;
 }
 while (size >= needle.n_size) {
  if (!MEMEQB(begin + size - needle.n_size,needle.n_data,needle.n_size))
       break;
  size -= needle.n_size;
 }
 if (begin == DeeBytes_DATA(self) &&
     size  == DeeBytes_SIZE(self))
     goto retself;
 return DeeBytes_NewView(self->b_orig,
                         begin,size,
                         self->b_flags);
retself:
 return_reference_((DeeObject *)self);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casesstrip(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 uint8_t *begin; DeeObject *mask;
 Needle needle; size_t size;
 if (DeeArg_Unpack(argc,argv,"o:casesstrip",&mask))
     goto err;
 if (get_needle(&needle,mask))
     goto err;
 if (needle.n_size) goto retself;
 begin = DeeBytes_DATA(self);
 size  = DeeBytes_SIZE(self);
 while (size >= needle.n_size) {
  if (!MEMCASEEQB(begin,needle.n_data,needle.n_size))
       break;
  begin += needle.n_size;
  size  -= needle.n_size;
 }
 while (size >= needle.n_size) {
  if (!MEMCASEEQB(begin + size - needle.n_size,needle.n_data,needle.n_size))
       break;
  size -= needle.n_size;
 }
 if (begin == DeeBytes_DATA(self) &&
     size  == DeeBytes_SIZE(self))
     goto retself;
 return DeeBytes_NewView(self->b_orig,
                         begin,size,
                         self->b_flags);
retself:
 return_reference_((DeeObject *)self);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_lsstrip(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 uint8_t *begin; DeeObject *mask;
 Needle needle; size_t size;
 if (DeeArg_Unpack(argc,argv,"o:lsstrip",&mask))
     goto err;
 if (get_needle(&needle,mask))
     goto err;
 if (needle.n_size) goto retself;
 begin = DeeBytes_DATA(self);
 size  = DeeBytes_SIZE(self);
 while (size >= needle.n_size) {
  if (!MEMEQB(begin,needle.n_data,needle.n_size))
       break;
  begin += needle.n_size;
  size  -= needle.n_size;
 }
 if (begin == DeeBytes_DATA(self))
     goto retself;
 return DeeBytes_NewView(self->b_orig,
                         begin,size,
                         self->b_flags);
retself:
 return_reference_((DeeObject *)self);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_caselsstrip(Bytes *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 uint8_t *begin; DeeObject *mask;
 Needle needle; size_t size;
 if (DeeArg_Unpack(argc,argv,"o:caselsstrip",&mask))
     goto err;
 if (get_needle(&needle,mask))
     goto err;
 if (needle.n_size) goto retself;
 begin = DeeBytes_DATA(self);
 size  = DeeBytes_SIZE(self);
 while (size >= needle.n_size) {
  if (!MEMCASEEQB(begin,needle.n_data,needle.n_size))
       break;
  begin += needle.n_size;
  size  -= needle.n_size;
 }
 if (begin == DeeBytes_DATA(self))
     goto retself;
 return DeeBytes_NewView(self->b_orig,
                         begin,size,
                         self->b_flags);
retself:
 return_reference_((DeeObject *)self);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_rsstrip(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 uint8_t *begin; DeeObject *mask;
 Needle needle; size_t size;
 if (DeeArg_Unpack(argc,argv,"o:rsstrip",&mask))
     goto err;
 if (get_needle(&needle,mask))
     goto err;
 if (needle.n_size) goto retself;
 begin = DeeBytes_DATA(self);
 size  = DeeBytes_SIZE(self);
 while (size >= needle.n_size) {
  if (!MEMEQB(begin + size - needle.n_size,needle.n_data,needle.n_size))
       break;
  size -= needle.n_size;
 }
 if (size  == DeeBytes_SIZE(self))
     goto retself;
 return DeeBytes_NewView(self->b_orig,
                         begin,size,
                         self->b_flags);
retself:
 return_reference_((DeeObject *)self);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casersstrip(Bytes *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 uint8_t *begin; DeeObject *mask;
 Needle needle; size_t size;
 if (DeeArg_Unpack(argc,argv,"o:casersstrip",&mask))
     goto err;
 if (get_needle(&needle,mask))
     goto err;
 if (needle.n_size) goto retself;
 begin = DeeBytes_DATA(self);
 size  = DeeBytes_SIZE(self);
 while (size >= needle.n_size) {
  if (!MEMCASEEQB(begin + size - needle.n_size,needle.n_data,needle.n_size))
       break;
  size -= needle.n_size;
 }
 if (size  == DeeBytes_SIZE(self))
     goto retself;
 return DeeBytes_NewView(self->b_orig,
                         begin,size,
                         self->b_flags);
retself:
 return_reference_((DeeObject *)self);
err:
 return NULL;
}

struct bcompare_args {
    DeeObject *other;   /* [1..1] String or bytes object. */
    uint8_t   *lhs_ptr; /* [0..my_len] Starting pointer of lhs. */
    size_t     lhs_len; /* Number of bytes in lhs. */
    uint8_t   *rhs_ptr; /* [0..my_len] Starting pointer of rhs. */
    size_t     rhs_len; /* Number of bytes in rhs. */
};

PRIVATE int DCALL
get_bcompare_args(Bytes *__restrict self,
                  struct bcompare_args *__restrict args,
                  size_t argc, DeeObject **__restrict argv,
                  char const *__restrict funname) {
 DeeObject *other; size_t temp,temp2;
 args->lhs_ptr = DeeBytes_DATA(self);
 args->lhs_len = DeeBytes_SIZE(self);
 switch (argc) {
 case 1:
  args->other = other = argv[0];
  if (DeeBytes_Check(other)) {
   args->rhs_ptr = DeeBytes_DATA(other);
   args->rhs_len = DeeBytes_SIZE(other);
  } else {
   if unlikely(!DeeString_Check(other))
      goto err_type_other;
   args->rhs_ptr = DeeString_AsBytes(other,false);
   if unlikely(!args->rhs_ptr) goto err;
   args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
  }
  break;
 case 2:
  if (DeeBytes_Check(argv[0])) {
   args->other = other = argv[0];
   if (DeeObject_AsSSize(argv[1],(dssize_t *)&temp)) goto err;
   args->rhs_ptr = DeeBytes_DATA(other);
   args->rhs_len = DeeBytes_SIZE(other);
   if unlikely(temp >= args->rhs_len) {
    args->rhs_len = 0;
   } else {
    args->rhs_ptr += temp;
    args->rhs_len -= temp;
   }
  } else if (DeeString_Check(argv[0])) {
   args->other = other = argv[0];
   if (DeeObject_AsSSize(argv[1],(dssize_t *)&temp)) goto err;
   args->rhs_ptr = DeeString_AsBytes(other,false);
   if unlikely(!args->rhs_ptr) goto err;
   args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
   if unlikely(temp >= args->rhs_len) {
    args->rhs_len = 0;
   } else {
    args->rhs_ptr += temp;
    args->rhs_len -= temp;
   }
  } else {
   if (DeeObject_AsSSize(argv[0],(dssize_t *)&temp)) goto err;
   if unlikely(temp >= args->lhs_len) {
    args->lhs_len = 0;
   } else {
    args->lhs_ptr += temp;
    args->lhs_len -= temp;
   }
   args->other = other = argv[1];
   if (DeeBytes_Check(other)) {
    args->rhs_ptr = DeeBytes_DATA(other);
    args->rhs_len = DeeBytes_SIZE(other);
   } else {
    if unlikely(!DeeString_Check(other))
       goto err_type_other;
    args->rhs_ptr = DeeString_AsBytes(other,false);
    if unlikely(!args->rhs_ptr) goto err;
    args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
   }
  }
  break;
 case 3:
  if (DeeBytes_Check(argv[0])) {
   args->other = other = argv[0];
   args->rhs_ptr = DeeBytes_DATA(other);
   args->rhs_len = DeeBytes_SIZE(other);
   if (DeeObject_AsSSize(argv[1],(dssize_t *)&temp)) goto err;
   if (DeeObject_AsSSize(argv[2],(dssize_t *)&temp2)) goto err;
   if unlikely(temp >= args->rhs_len) {
    args->rhs_len = 0;
   } else {
    if (temp2 > args->rhs_len)
        temp2 = args->rhs_len;
    args->rhs_ptr += temp;
    args->rhs_len  = temp2 - temp;
   }
  } else if (DeeString_Check(argv[0])) {
   args->other = other = argv[0];
   args->rhs_ptr = DeeString_AsBytes(other,true);
   if unlikely(!args->rhs_ptr) goto err;
   args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
   if (DeeObject_AsSSize(argv[1],(dssize_t *)&temp)) goto err;
   if (DeeObject_AsSSize(argv[2],(dssize_t *)&temp2)) goto err;
   if unlikely(temp >= args->rhs_len) {
    args->rhs_len = 0;
   } else {
    if (temp2 > args->rhs_len)
        temp2 = args->rhs_len;
    args->rhs_ptr += temp;
    args->rhs_len  = temp2 - temp;
   }
  } else if (DeeBytes_Check(argv[1])) {
   if (DeeObject_AsSSize(argv[0],(dssize_t *)&temp)) goto err;
   if unlikely(temp >= args->lhs_len) {
    args->lhs_len = 0;
   } else {
    args->lhs_ptr += temp;
    args->lhs_len -= temp;
   }
   args->other = other = argv[1];
   args->rhs_ptr = DeeBytes_DATA(other);
   args->rhs_len = DeeBytes_SIZE(other);
   if (DeeObject_AsSSize(argv[2],(dssize_t *)&temp)) goto err;
   if unlikely(temp >= args->rhs_len) {
    args->rhs_len = 0;
   } else {
    args->rhs_ptr += temp;
    args->rhs_len -= temp;
   }
  } else if (DeeString_Check(argv[1])) {
   if (DeeObject_AsSSize(argv[0],(dssize_t *)&temp)) goto err;
   if unlikely(temp >= args->lhs_len) {
    args->lhs_len = 0;
   } else {
    args->lhs_ptr += temp;
    args->lhs_len -= temp;
   }
   args->other = other = argv[1];
   args->rhs_ptr = DeeString_AsBytes(other,false);
   if unlikely(!args->rhs_ptr) goto err;
   args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
   if (DeeObject_AsSSize(argv[2],(dssize_t *)&temp)) goto err;
   if unlikely(temp >= args->rhs_len) {
    args->rhs_len = 0;
   } else {
    args->rhs_ptr += temp;
    args->rhs_len -= temp;
   }
  } else {
   if (DeeObject_AsSSize(argv[0],(dssize_t *)&temp)) goto err;
   if (DeeObject_AsSSize(argv[1],(dssize_t *)&temp2)) goto err;
   if (temp >= args->lhs_len) {
    args->lhs_len = 0;
   } else {
    if (temp2 > args->lhs_len)
        temp2 = args->lhs_len;
    args->lhs_ptr += temp;
    args->lhs_len  = temp2 - temp;
   }
   args->other = other = argv[2];
   if (DeeBytes_Check(other)) {
    args->rhs_ptr = DeeBytes_DATA(other);
    args->rhs_len = DeeBytes_SIZE(other);
   } else {
    if unlikely(!DeeString_Check(other))
       goto err_type_other;
    args->rhs_ptr = DeeString_AsBytes(other,false);
    if unlikely(!args->rhs_ptr) goto err;
    args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
   }
  }
  break;
 case 4:
  if (DeeObject_AsSSize(argv[0],(dssize_t *)&temp)) goto err;
  if (DeeBytes_Check(argv[1])) {
   if unlikely(temp >= args->lhs_len) {
    args->lhs_len = 0;
   } else {
    args->lhs_ptr += temp;
    args->lhs_len -= temp;
   }
   args->other = other = argv[1];
   if (DeeObject_AsSSize(argv[2],(dssize_t *)&temp)) goto err;
   if (DeeObject_AsSSize(argv[3],(dssize_t *)&temp2)) goto err;
   args->rhs_ptr = DeeBytes_DATA(other);
   args->rhs_len = DeeBytes_SIZE(other);
   if (temp >= args->rhs_len) {
    args->rhs_len = 0;
   } else {
    if (temp2 > args->rhs_len)
        temp2 = args->rhs_len;
    args->rhs_ptr += temp;
    args->rhs_len  = temp2 - temp;
   }
  } else if (DeeString_Check(argv[1])) {
   if unlikely(temp >= args->lhs_len) {
    args->lhs_len = 0;
   } else {
    args->lhs_ptr += temp;
    args->lhs_len -= temp;
   }
   args->other = other = argv[1];
   if (DeeObject_AsSSize(argv[2],(dssize_t *)&temp)) goto err;
   if (DeeObject_AsSSize(argv[3],(dssize_t *)&temp2)) goto err;
   args->rhs_ptr = DeeString_AsBytes(other,false);
   if unlikely(!args->rhs_ptr) goto err;
   args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
   if (temp >= args->rhs_len) {
    args->rhs_len = 0;
   } else {
    if (temp2 > args->rhs_len)
        temp2 = args->rhs_len;
    args->rhs_ptr += temp;
    args->rhs_len  = temp2 - temp;
   }
  } else {
   if (DeeObject_AsSSize(argv[1],(dssize_t *)&temp2)) goto err;
   if unlikely(temp >= args->lhs_len) {
    args->lhs_len = 0;
   } else {
    if (temp2 > args->lhs_len)
        temp2 = args->lhs_len;
    args->lhs_ptr += temp;
    args->lhs_len  = temp2 - temp;
   }
   args->other = other = argv[2];
   if unlikely(!DeeString_Check(other))
      goto err_type_other;
   if (DeeObject_AsSSize(argv[3],(dssize_t *)&temp)) goto err;
   args->rhs_ptr = DeeString_AsBytes(other,false);
   if unlikely(!args->rhs_ptr) goto err;
   args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
   if (temp >= args->rhs_len) {
    args->rhs_len = 0;
   } else {
    args->rhs_ptr += temp;
    args->rhs_len -= temp;
   }
  }
  break;
 case 5:
  if (DeeObject_AsSSize(argv[0],(dssize_t *)&temp)) goto err;
  if (DeeObject_AsSSize(argv[1],(dssize_t *)&temp2)) goto err;
  if (temp >= args->lhs_len) {
   args->lhs_len = 0;
  } else {
   if (temp2 > args->lhs_len)
       temp2 = args->lhs_len;
   args->lhs_ptr += temp;
   args->lhs_len  = temp2 - temp;
  }
  args->other = other = argv[2];
  if (DeeBytes_Check(other)) {
   args->rhs_ptr = DeeBytes_DATA(other);
   args->rhs_len = DeeBytes_SIZE(other);
  } else {
   if unlikely(!DeeString_Check(other))
      goto err_type_other;
   args->rhs_ptr = DeeString_AsBytes(other,false);
   if unlikely(!args->rhs_ptr) goto err;
   args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
  }
  if (DeeObject_AsSSize(argv[3],(dssize_t *)&temp)) goto err;
  if (DeeObject_AsSSize(argv[4],(dssize_t *)&temp2)) goto err;
  if (temp >= args->rhs_len) {
   args->rhs_len = 0;
  } else {
   if (temp2 > args->rhs_len)
       temp2 = args->rhs_len;
   args->rhs_ptr += temp;
   args->rhs_len  = temp2 - temp;
  }
  break;
 default:
  err_invalid_argc(funname,argc,1,5);
  goto err;
 }
 return 0;
err_type_other:
 DeeObject_TypeAssertFailed(other,&DeeBytes_Type);
err:
 return -1;
}

PRIVATE DREF DeeObject *DCALL
bytes_compare(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 struct bcompare_args args; int result;
 if (get_bcompare_args(self,&args,argc,argv,"compare"))
     return NULL;
 if (args.lhs_len < args.rhs_len) {
  result = memcmp(args.lhs_ptr,args.rhs_ptr,args.lhs_len);
  if (result == 0) return_reference_(&DeeInt_MinusOne);
 } else if (args.lhs_len > args.rhs_len) {
  result = memcmp(args.lhs_ptr,args.rhs_ptr,args.rhs_len);
  if (result == 0) return_reference_(&DeeInt_One);
 } else {
  result = memcmp(args.lhs_ptr,args.rhs_ptr,args.lhs_len);
 }
 return DeeInt_NewInt(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_vercompare(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 struct bcompare_args args; int8_t result;
 if (get_bcompare_args(self,&args,argc,argv,"vercompare"))
     return NULL;
 result = dee_strverscmpb(args.lhs_ptr,args.lhs_len,
                          args.rhs_ptr,args.lhs_len);
 return DeeInt_NewS8(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_wildcompare(Bytes *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 struct bcompare_args args; int result;
 if (get_bcompare_args(self,&args,argc,argv,"wildcompare"))
     return NULL;
 result = wildcompareb(args.lhs_ptr,args.lhs_len,
                       args.rhs_ptr,args.lhs_len);
 return DeeInt_NewInt(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_fuzzycompare(Bytes *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 struct bcompare_args args; dssize_t result;
 if (get_bcompare_args(self,&args,argc,argv,"fuzzycompare"))
     goto err;
 result = fuzzy_compareb(args.lhs_ptr,args.lhs_len,
                         args.rhs_ptr,args.lhs_len);
 if unlikely(result == (dssize_t)-1) goto err;
 return DeeInt_NewSize((size_t)result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_wmatch(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 struct bcompare_args args; int result;
 if (get_bcompare_args(self,&args,argc,argv,"wmatch"))
     return NULL;
 result = wildcompareb(args.lhs_ptr,args.lhs_len,
                       args.rhs_ptr,args.lhs_len);
 return_bool_(result == 0);
}

PRIVATE DREF DeeObject *DCALL
bytes_casecompare(Bytes *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
 struct bcompare_args args; int result;
 if (get_bcompare_args(self,&args,argc,argv,"casecompare"))
     return NULL;
 if (args.lhs_len < args.rhs_len) {
  result = memcasecmp(args.lhs_ptr,args.rhs_ptr,args.lhs_len);
  if (result == 0) return_reference_(&DeeInt_MinusOne);
 } else if (args.lhs_len > args.rhs_len) {
  result = memcasecmp(args.lhs_ptr,args.rhs_ptr,args.rhs_len);
  if (result == 0) return_reference_(&DeeInt_One);
 } else {
  result = memcasecmp(args.lhs_ptr,args.rhs_ptr,args.lhs_len);
 }
 return DeeInt_NewInt(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_casevercompare(Bytes *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
 struct bcompare_args args; int8_t result;
 if (get_bcompare_args(self,&args,argc,argv,"casevercompare"))
     return NULL;
 result = dee_strcaseverscmpb(args.lhs_ptr,args.lhs_len,
                              args.rhs_ptr,args.lhs_len);
 return DeeInt_NewS8(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_casewildcompare(Bytes *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 struct bcompare_args args; int result;
 if (get_bcompare_args(self,&args,argc,argv,"casewildcompare"))
     return NULL;
 result = wildcasecompareb(args.lhs_ptr,args.lhs_len,
                           args.rhs_ptr,args.lhs_len);
 return DeeInt_NewInt(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_casefuzzycompare(Bytes *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
 struct bcompare_args args; dssize_t result;
 if (get_bcompare_args(self,&args,argc,argv,"casefuzzycompare"))
     goto err;
 result = fuzzy_casecompareb(args.lhs_ptr,args.lhs_len,
                             args.rhs_ptr,args.lhs_len);
 if unlikely(result == (dssize_t)-1) goto err;
 return DeeInt_NewSize((size_t)result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casewmatch(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 struct bcompare_args args; int result;
 if (get_bcompare_args(self,&args,argc,argv,"casewmatch"))
     return NULL;
 result = wildcasecompareb(args.lhs_ptr,args.lhs_len,
                           args.rhs_ptr,args.lhs_len);
 return_bool_(result == 0);
}

PRIVATE DREF DeeObject *DCALL
bytes_center(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result; size_t width;
 DeeObject *filler_ob = NULL; Needle filler;
 if (DeeArg_Unpack(argc,argv,"Iu|o:center",&width,&filler_ob))
     goto err;
 if (filler_ob) {
  if (get_needle(&filler,filler_ob))
      goto err;
 } else {
  filler.n_data    = filler._n_buf;
  filler.n_size    = 1;
  filler._n_buf[0] = 0x20; /* ' ' */
 }
 if (width <= DeeBytes_SIZE(self)) {
  result = DeeBytes_NewBufferData(DeeBytes_DATA(self),
                                  DeeBytes_SIZE(self));
 } else {
  size_t fill_front,fill_back;
  result = DeeBytes_NewBufferUninitialized(width);
  if unlikely(!result) goto err;
  fill_front  = (width - DeeBytes_SIZE(self));
  fill_back   = fill_front/2;
  fill_front -= fill_back;
  memfilb(DeeBytes_DATA(result) + 0,fill_front,filler.n_data,filler.n_size);
  memcpyb(DeeBytes_DATA(result) + fill_front,
          DeeBytes_DATA(self),DeeBytes_SIZE(self));
  memfilb(DeeBytes_DATA(result) + fill_front + DeeBytes_SIZE(self),
          fill_back,filler.n_data,filler.n_size);
 }
 return result;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_ljust(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result; size_t width;
 DeeObject *filler_ob = NULL; Needle filler;
 if (DeeArg_Unpack(argc,argv,"Iu|o:ljust",&width,&filler_ob))
     goto err;
 if (filler_ob) {
  if (get_needle(&filler,filler_ob))
      goto err;
 } else {
  filler.n_data    = filler._n_buf;
  filler.n_size    = 1;
  filler._n_buf[0] = 0x20; /* ' ' */
 }
 if (width <= DeeBytes_SIZE(self)) {
  result = DeeBytes_NewBufferData(DeeBytes_DATA(self),
                                  DeeBytes_SIZE(self));
 } else {
  size_t fill_back;
  result = DeeBytes_NewBufferUninitialized(width);
  if unlikely(!result) goto err;
  fill_back = (width - DeeBytes_SIZE(self));
  memcpyb(DeeBytes_DATA(result) + 0,
          DeeBytes_DATA(self),DeeBytes_SIZE(self));
  memfilb(DeeBytes_DATA(result) + DeeBytes_SIZE(self),
          fill_back,filler.n_data,filler.n_size);
 }
 return result;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_rjust(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result; size_t width;
 DeeObject *filler_ob = NULL; Needle filler;
 if (DeeArg_Unpack(argc,argv,"Iu|o:rjust",&width,&filler_ob))
     goto err;
 if (filler_ob) {
  if (get_needle(&filler,filler_ob))
      goto err;
 } else {
  filler.n_data    = filler._n_buf;
  filler.n_size    = 1;
  filler._n_buf[0] = 0x20; /* ' ' */
 }
 if (width <= DeeBytes_SIZE(self)) {
  result = DeeBytes_NewBufferData(DeeBytes_DATA(self),
                                  DeeBytes_SIZE(self));
 } else {
  size_t fill_front;
  result = DeeBytes_NewBufferUninitialized(width);
  if unlikely(!result) goto err;
  fill_front  = (width - DeeBytes_SIZE(self));
  memfilb(DeeBytes_DATA(result) + 0,fill_front,filler.n_data,filler.n_size);
  memcpyb(DeeBytes_DATA(result) + fill_front,
          DeeBytes_DATA(self),DeeBytes_SIZE(self));
 }
 return result;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_zfill(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result; size_t width;
 DeeObject *filler_ob = NULL; Needle filler;
 if (DeeArg_Unpack(argc,argv,"Iu|o:zfill",&width,&filler_ob))
     goto err;
 if (filler_ob) {
  if (get_needle(&filler,filler_ob))
      goto err;
 } else {
  filler.n_data    = filler._n_buf;
  filler.n_size    = 1;
  filler._n_buf[0] = 0x30; /* '0' */
 }
 if (width <= DeeBytes_SIZE(self)) {
  result = DeeBytes_NewBufferData(DeeBytes_DATA(self),
                                  DeeBytes_SIZE(self));
 } else {
  size_t fill_front,src_len; uint8_t *dst,*src;
  result = DeeBytes_NewBufferUninitialized(width);
  if unlikely(!result) goto err;
  dst        = DeeBytes_DATA(result);
  src        = DeeBytes_DATA(self);
  src_len    = DeeBytes_SIZE(self);
  fill_front = (width - src_len);
  while (src_len && DeeUni_IsSign(src[0])) {
   *dst++ = *src++;
   --src_len;
  }
  memfilb(dst + 0,fill_front,filler.n_data,filler.n_size);
  memcpyb(dst + fill_front,src,src_len);
 }
 return result;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_expandtabs(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 size_t tab_width = 8;
 if (DeeArg_Unpack(argc,argv,"|Iu:expandtabs",&tab_width))
     goto err;
 {
  struct bytes_printer printer = BYTES_PRINTER_INIT;
  uint8_t *iter,*end,*flush_start;
  size_t line_inset = 0;
  iter = DeeBytes_DATA(self);
  end  = iter + DeeBytes_SIZE(iter);
  flush_start = iter;
  for (; iter < end; ++iter) {
   uint8_t ch = *iter;
   if (!DeeUni_IsTab(ch)) {
    ++line_inset;
    if (DeeUni_IsLF(ch))
        line_inset = 0; /* Reset insets at line starts. */
    continue;
   }
   if (bytes_printer_append(&printer,flush_start,
                           (size_t)(iter-flush_start)) < 0)
       goto err_printer;
   /* Replace with white-space. */
   if likely(tab_width) {
    line_inset = tab_width - (line_inset % tab_width);
    if (bytes_printer_repeat(&printer,ASCII_SPACE,line_inset) < 0)
        goto err_printer;
    line_inset = 0;
   }
   flush_start = iter+1;
  }
  if (!BYTES_PRINTER_SIZE(&printer))
       goto retself;
  if (bytes_printer_append(&printer,flush_start,
                          (size_t)(iter-flush_start)) < 0)
      goto err_printer;
  return bytes_printer_pack(&printer);
retself:
  bytes_printer_fini(&printer);
  return_reference_((DeeObject *)self);
err_printer:
  bytes_printer_fini(&printer);
 }
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_unifylines(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 DeeObject *replace_ob; Needle replace;
 if (DeeArg_Unpack(argc,argv,"|o:unifylines",&replace_ob))
     goto err;
 if (replace_ob) {
  if (get_needle(&replace,replace_ob))
      goto err;
 } else {
  replace.n_data    = replace._n_buf;
  replace.n_size    = 1;
  replace._n_buf[0] = '\n';
 }
 {
  struct bytes_printer printer = BYTES_PRINTER_INIT;
  uint8_t *iter,*end,*flush_start;
  iter = DeeBytes_DATA(self);
  end  = iter + DeeBytes_SIZE(iter);
  flush_start = iter;
  for (; iter < end; ++iter) {
   uint8_t ch = *iter;
   if (ch != ASCII_CR && ch != ASCII_LF)
       continue; /* Not a line-feed character */
   if (replace.n_size == 1 && ch == replace.n_data[0]) {
    if (ch != ASCII_CR)
        continue; /* No-op replacement. */
    if (iter + 1 >= end)
        continue; /* Cannot be CRLF */
    if (iter[1] != ASCII_LF)
        continue; /* Isn't CRLF */
   }
   if (bytes_printer_append(&printer,flush_start,
                           (size_t)(iter-flush_start)) < 0)
       goto err_printer;
   if (bytes_printer_append(&printer,
                             replace.n_data,
                             replace.n_size) < 0)
       goto err_printer;
   if (ch == ASCII_CR && iter + 1 < end && iter[1] == ASCII_LF)
       ++iter;
   flush_start = iter+1;
  }
  if (!BYTES_PRINTER_SIZE(&printer))
       goto retself;
  if (bytes_printer_append(&printer,flush_start,
                          (size_t)(iter-flush_start)) < 0)
      goto err_printer;
  return bytes_printer_pack(&printer);
retself:
  bytes_printer_fini(&printer);
  return_reference_((DeeObject *)self);
err_printer:
  bytes_printer_fini(&printer);
 }
err:
 return NULL;
}


INTERN struct type_method bytes_methods[] = {
    { "decode", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_decode,
      DOC("(string codec,string errors=\"strict\")->string\n"
          "(string codec,string errors=\"strict\")->object\n"
          "@throw ValueError The given @codec or @errors wasn't recognized\n"
          "@throw UnicodeDecodeError @this string could not be decoded as @codec and @errors was set to $\"strict\"\n"
          "@param errors The way that decode-errors are handled as one of $\"strict\", $\"replace\" or $\"ignore\"\n"
          "Same as :string.decode, but instead use the data of @this bytes object as characters to decode") },
    { "encode", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&string_encode,
      DOC("(string codec,string errors=\"strict\")->bytes\n"
          "(string codec,string errors=\"strict\")->string\n"
          "(string codec,string errors=\"strict\")->object\n"
          "@throw ValueError The given @codec or @errors wasn't recognized\n"
          "@throw UnicodeEncodeError @this string could not be decoded as @codec and @errors was set to $\"strict\"\n"
          "@param errors The way that decode-errors are handled as one of $\"strict\", $\"replace\" or $\"ignore\"\n"
          "Same as :string.encode, but instead use the data of @this bytes object as characters to decode") },
    { "bytes", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_substr,
      DOC("(int start=0,int end=-1)->bytes\n"
          "Same as #substr (here for ABI compatibility with :string.bytes)") },
    { "ord", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_ord,
      DOC("()->int\n"
          "@throw ValueError The length of @this bytes object is not equal to ${1}\n"
          "Same as ${this[0]}\n"
          "\n"
          "(int index)->int\n"
          "@throw IntegerOverflow The given @index is lower than $0\n"
          "@throw IndexError The given @index is greater than ${#this}\n"
          "Same as ${this[index]}") },

    /* Bytes formatting / scanning. */
    { "format", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_format,
      DOC("(sequence args)->bytes") },
    { "scanf", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_scanf,
      DOC("(string format)->sequence") },

    /* String/Character traits */
    { "isprint", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isprint,
      DOC("()->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all "
          "characters in ${this.substr(start,end)} are printable") },
    { "isalpha", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isalpha,
      DOC("()->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
          "in ${this.substr(start,end)} are alphabetical") },
    { "isspace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isspace,
      DOC("()->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
          "in ${this.substr(start,end)} are space-characters") },
    { "islf", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_islf,
      DOC("()->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
          "in ${this.substr(start,end)} are line-feeds") },
    { "islower", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_islower,
      DOC("()->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
          "in ${this.substr(start,end)} are lower-case") },
    { "isupper", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isupper,
      DOC("()->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
          "in ${this.substr(start,end)} are upper-case") },
    { "iscntrl", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_iscntrl,
      DOC("()->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
          "in ${this.substr(start,end)} are control characters") },
    { "isdigit", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isdigit,
      DOC("()->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
          "in ${this.substr(start,end)} are digits") },
    { "isdecimal", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isdecimal,
      DOC("()->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
          "in ${this.substr(start,end)} are dicimal characters") },
    { "issymstrt", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_issymstrt,
      DOC("()->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
          "in ${this.substr(start,end)} can be used to start a symbol name") },
    { "issymcont", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_issymcont,
      DOC("()->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
          "in ${this.substr(start,end)} can be used to continue a symbol name") },
    { "isalnum", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isalnum,
      DOC("()->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
          "in ${this.substr(start,end)} are alpha-numerical") },
    { "isnumeric", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isnumeric,
      DOC("()->bool\n"
          "(int index)->bool\n"
          "(int start,int end)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
          "in ${this.substr(start,end)} qualify as digit or decimal characters") },
    { "istitle", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_istitle,
      DOC("(int index)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if the character at ${this[index]} has title-casing\n"
          "\n"
          "()->bool\n"
          "(int start=0,int end=-1)->bool\n"
          "Returns :true if $this, or the sub-string ${this.substr(start,end)} "
          "follows title-casing, meaning that space is followed by upper-case") },
    { "issymbol", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_issymbol,
      DOC("(int index)->bool\n"
          "@throw IndexError The given @index is larger than ${#this}\n"
          "@throw IntegerOverflow The given @index is negative or too large\n"
          "Returns :true if the character at ${this[index]} can be used to start a symbol name\n"
          "\n"
          "()->bool\n"
          "(int start,int end)->bool\n"
          "Returns :true if $this, or the sub-string ${this.substr(start,end)} "
          "is a valid symbol name") },

    { "isanyprint", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isanyprint,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is printable") },
    { "isanyalpha", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isanyalpha,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is alphabetical") },
    { "isanyspace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isanyspace,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is a space character") },
    { "isanylf", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isanylf,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is a line-feeds") },
    { "isanylower", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isanylower,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is lower-case") },
    { "isanyupper", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isanyupper,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is upper-case") },
    { "isanycntrl", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isanycntrl,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is a control character") },
    { "isanydigit", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isanydigit,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is a digit") },
    { "isanydecimal", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isanydecimal,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is a dicimal character") },
    { "isanysymstrt", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isanysymstrt,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} can be used to start a symbol name") },
    { "isanysymcont", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isanysymcont,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} can be used to continue a symbol name") },
    { "isanyalnum", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isanyalnum,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} is alpha-numerical") },
    { "isanynumeric", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isanynumeric,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} qualifies as digit or decimal characters") },
    { "isanytitle", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_isanytitle,
      DOC("(int start=0,int end=-1)->bool\n"
          "Returns :true if any character in "
          "${this.substr(start,end)} has title-casing") },

    /* Bytes conversion functions */
    { "lower", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_lower,
      DOC("(int start=0,int end=-1)->bytes\n"
          "Returns a writable copy of @this bytes object converted to lower-case (when interpreted as ASCII)") },
    { "upper", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_upper,
      DOC("(int start=0,int end=-1)->bytes\n"
          "Returns a writable copy of @this bytes object converted to upper-case (when interpreted as ASCII)") },
    { "title", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_title,
      DOC("(int start=0,int end=-1)->bytes\n"
          "Returns a writable copy of @this bytes object converted to title-casing (when interpreted as ASCII)") },
    { "capitalize", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_capitalize,
      DOC("(int start=0,int end=-1)->bytes\n"
          "Returns a writable copy of @this bytes object with each word capitalized (when interpreted as ASCII)") },
    { "swapcase", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_swapcase,
      DOC("(int start=0,int end=-1)->bytes\n"
          "Returns a writable copy of @this bytes object with the casing of each "
          "character that has two different casings swapped (when interpreted as ASCII)") },

    /* Inplace variants of bytes conversion functions */
    { "tolower", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_tolower,
      DOC("(int start=0,int end=-1)->bytes\n"
          "@throw BufferError @this bytes object is not writable\n"
          "Same as #lower, but character modifications are performed in-place, and @this bytes object is re-returned") },
    { "toupper", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_toupper,
      DOC("(int start=0,int end=-1)->bytes\n"
          "@throw BufferError @this bytes object is not writable\n"
          "Same as #upper, but character modifications are performed in-place, and @this bytes object is re-returned") },
    { "totitle", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_totitle,
      DOC("(int start=0,int end=-1)->bytes\n"
          "@throw BufferError @this bytes object is not writable\n"
          "Same as #title, but character modifications are performed in-place, and @this bytes object is re-returned") },
    { "tocapitalize", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_tocapitalize,
      DOC("(int start=0,int end=-1)->bytes\n"
          "@throw BufferError @this bytes object is not writable\n"
          "Same as #capitalize, but character modifications are performed in-place, and @this bytes object is re-returned") },
    { "toswapcase", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_toswapcase,
      DOC("(int start=0,int end=-1)->bytes\n"
          "@throw BufferError @this bytes object is not writable\n"
          "Same as #swapcase, but character modifications are performed in-place, and @this bytes object is re-returned") },

    /* Case-sensitive query functions */
    { "replace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_replace,
      DOC("(string find_str,string replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(string find_str,bytes replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(string find_str,int replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(bytes find_str,string replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(bytes find_str,bytes replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(bytes find_str,int replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(int find_str,string replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(int find_str,bytes replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(int find_str,int replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "@throw ValueError The given @find_str or @replace_str is a string containing characters ${> 0xff}\n"
          "@throw IntegerOverflow The given @find_str or @replace_str is an integer lower than $0, or greater than $0xff\n"
          "Find up to @max_count occurrances of @find_str and replace each with @replace_str, then return the resulting data as a writable bytes object") },
    { "toreplace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_toreplace,
      DOC("(string find_str,string replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(string find_str,bytes replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(string find_str,int replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(bytes find_str,string replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(bytes find_str,bytes replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(bytes find_str,int replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(int find_str,string replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(int find_str,bytes replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(int find_str,int replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "@throw ValueError The given @find_str or @replace_str is a string containing characters ${> 0xff}\n"
          "@throw IntegerOverflow The given @find_str or @replace_str is an integer lower than $0, or greater than $0xff\n"
          "@throw ValueError The number of bytes specified by @find_str and @replace_str are not identical\n"
          "@throw BufferError @this bytes object is not writable\n"
          "Same as #replace, but the bytes object is modified in-place, and @this is re-returned") },
    { "find", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_find,
      DOC("(bytes needle,int start=0,int end=-1)->int\n"
          "(string needle,int start=0,int end=-1)->int\n"
          "(int needle,int start=0,int end=-1)->int\n"
          "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
          "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
          "Find the first instance of @needle that exists within ${this.substr(start,end)}, "
          "and return its starting index, or ${-1} if no such position exists") },
    { "rfind", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_rfind,
      DOC("(bytes needle,int start=0,int end=-1)->int\n"
          "(string needle,int start=0,int end=-1)->int\n"
          "(int needle,int start=0,int end=-1)->int\n"
          "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
          "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
          "Find the first instance of @needle that exists within ${this.substr(start,end)}, "
          "and return its starting index, or ${-1} if no such position exists") },
    { "index", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_index,
      DOC("(bytes needle,int start=0,int end=-1)->int\n"
          "(string needle,int start=0,int end=-1)->int\n"
          "(int needle,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @needle can be found within ${this.substr(start,end)}\n"
          "Find the first instance of @needle that exists within ${this.substr(start,end)}, "
          "and return its starting index") },
    { "rindex", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_rindex,
      DOC("(bytes needle,int start=0,int end=-1)->int\n"
          "(string needle,int start=0,int end=-1)->int\n"
          "(int needle,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @needle can be found within ${this.substr(start,end)}\n"
          "Find the last instance of @needle that exists within ${this.substr(start,end)}, "
          "and return its starting index") },
    { "findall", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_findall,
      DOC("(bytes needle,int start=0,int end=-1)->{int...}\n"
          "(string needle,int start=0,int end=-1)->{int...}\n"
          "(int needle,int start=0,int end=-1)->{int...}\n"
          "Find all instances of @needle within ${this.substr(start,end)}, "
          "and return their starting indeces as a sequence") },
    { "count", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_count,
      DOC("(bytes needle,int start=0,int end=-1)->int\n"
          "(string needle,int start=0,int end=-1)->int\n"
          "(int needle,int start=0,int end=-1)->int\n"
          "Count the number of instances of @needle that exists within ${this.substr(start,end)}, "
          "and return now many were found") },
    { "contains", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_contains_f,
      DOC("(bytes needle,int start=0,int end=-1)->bool\n"
          "(string needle,int start=0,int end=-1)->bool\n"
          "(int needle,int start=0,int end=-1)->bool\n"
          "Check if @needle can be found within ${this.substr(start,end)}, and return a boolean indicative of that") },
    { "substr", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_substr,
      DOC("(int start=0,int end=-1)->bytes\n"
          "Similar to ${this[start:end]}, and semantically equialent to :string.substr\n"
          "This function can be used to view a sub-set of bytes from @this bytes object\n"
          "Modifications then made to the returned bytes object will affect the same memory already described by @this bytes object\n") },
    { "strip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_strip,
      DOC("->bytes\n"
          "(string mask)->bytes\n"
          "(bytes mask)->bytes\n"
          "(int mask)->bytes\n"
          "Strip all leading and trailing whitespace-characters, or "
          "characters apart of @mask, and return a sub-view of @this bytes object") },
    { "lstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_lstrip,
      DOC("->bytes\n"
          "(string mask)->bytes\n"
          "(bytes mask)->bytes\n"
          "(int mask)->bytes\n"
          "Strip all leading whitespace-characters, or characters "
          "apart of @mask, and return a sub-view of @this bytes object") },
    { "rstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_rstrip,
      DOC("->bytes\n"
          "(string mask)->bytes\n"
          "(bytes mask)->bytes\n"
          "(int mask)->bytes\n"
          "Strip all trailing whitespace-characters, or characters "
          "apart of @mask, and return a sub-view of @this bytes object") },
    { "sstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_sstrip,
      DOC("(string other)->bytes\n"
          "(bytes other)->bytes\n"
          "(int other)->bytes\n"
          "Strip all leading and trailing instances of @other from @this string\n"
          ">local result = this;\n"
          ">while (result.startswith(other))\n"
          ">       result = result[#other:];\n"
          ">while (result.endswith(other))\n"
          ">       result = result[:#result-#other];\n") },
    { "lsstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_lsstrip,
      DOC("(string other)->bytes\n"
          "(bytes other)->bytes\n"
          "(int other)->bytes\n"
          "Strip all leading instances of @other from @this string\n"
          ">local result = this;\n"
          ">while (result.startswith(other))\n"
          ">       result = result[#other:];\n") },
    { "rsstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_rsstrip,
      DOC("(string other)->bytes\n"
          "(bytes other)->bytes\n"
          "(int other)->bytes\n"
          "Strip all trailing instances of @other from @this string\n"
          ">local result = this;\n"
          ">while (result.endswith(other))\n"
          ">       result = result[:#result-#other];\n") },
    { "startswith", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_startswith,
      DOC("(string needle,int start=0,int end=-1)->bool\n"
          "(bytes needle,int start=0,int end=-1)->bool\n"
          "(int needle,int start=0,int end=-1)->bool\n"
          "Return :true if the sub-string ${this.substr(start,end)} starts with @other") },
    { "endswith", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_endswith,
      DOC("(string needle,int start=0,int end=-1)->bool\n"
          "(bytes needle,int start=0,int end=-1)->bool\n"
          "(int needle,int start=0,int end=-1)->bool\n"
          "Return :true if the sub-string ${this.substr(start,end)} ends with @other") },
    { "partition", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_parition,
      DOC("(string needle,int start=0,int end=-1)->(bytes,bytes,bytes)\n"
          "(bytes needle,int start=0,int end=-1)->(bytes,bytes,bytes)\n"
          "(int needle,int start=0,int end=-1)->(bytes,bytes,bytes)\n"
          "Search for the first instance of @needle within ${this.substr(start,end)} and "
          "return a 3-element sequence of byte objects ${(this[:pos],needle,this[pos+#needle:])}.\n"
          "If @needle could not be found, ${(this,\"\".bytes(),\"\".bytes())} is returned") },
    { "rpartition", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_rparition,
      DOC("(string needle,int start=0,int end=-1)->(bytes,bytes,bytes)\n"
          "(bytes needle,int start=0,int end=-1)->(bytes,bytes,bytes)\n"
          "(int needle,int start=0,int end=-1)->(bytes,bytes,bytes)\n"
          "Search for the last instance of @needle within ${this.substr(start,end)} and "
          "return a 3-element sequence of strings ${(this[:pos],needle,this[pos+#needle:])}.\n"
          "If @needle could not be found, ${(this,\"\".bytes(),\"\".bytes())} is returned") },
    { "compare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_compare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,bytes other,int other_start=0,int other_end=-1)->int\n"
          "Compare the sub-string ${left = this.substr(my_start,my_end)} with ${right = other.substr(other_start,other_end)}, "
          "returning ${< 0} if ${left < right}, ${> 0} if ${left > right}, or ${== 0} if they are equal") },
    { "vercompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_vercompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,bytes other,int other_start=0,int other_end=-1)->int\n"
          "Performs a version-string comparison. This is similar to #compare, but rather than "
          "performing a strict lexicographical comparison, the numbers found in the strings "
          "being compared are comparsed as a whole, solving the common problem seen in applications "
          "such as file navigators showing a file order of `foo1.txt', `foo10.txt', `foo11.txt', `foo2.txt', etc...\n"
          "This function is a portable implementation of the GNU function "
          "%{link https://linux.die.net/man/3/strverscmp strverscmp}, "
          "for which you may follow the link for further details") },
    { "wildcompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_wildcompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,bytes other,int other_start=0,int other_end=-1)->int\n"
          "Perform a wild-character-enabled comparising of the sub-string ${left = this.substr(my_start,my_end)} "
          "with ${right = pattern.substr(pattern_start,pattern_end)}, returning ${< 0} if ${left < right}, ${> 0} "
          "if ${left > right}, or ${== 0} if they are equal\n"
          "Wild-compare characters are only parsed from @pattern, allowing $\"?\" to "
          "be matched with any single character from @this, and $\"*\" to be matched to "
          "any number of characters") },
    { "fuzzycompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_fuzzycompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,bytes other,int other_start=0,int other_end=-1)->int\n"
          "Perform a fuzzy string comparison between ${this.substr(my_start,my_end)} and ${other.substr(other_start,other_end)}\n"
          "The return value is a similarty-factor that can be used to score how close the two strings look alike.\n"
          "How exactly the scoring is done is implementation-specific, however a score of $0 is reserved for two "
          "strings that are perfectly identical, any two differing strings always have a score ${> 0}, and the closer "
          "the score is to $0, the more alike they are\n"
          "The intended use of this function is for auto-completion, as well as warning "
          "messages and recommendations in the sense of I-dont-know-foo-but-did-you-mean-bar\n"
          "Note that there is another version #casefuzzycompare that also ignores casing") },
    { "wmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_wmatch,
      DOC("(string other,int other_start=0,int other_end=-1)->bool\n"
          "(bytes other,int other_start=0,int other_end=-1)->bool\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->bool\n"
          "(int my_start,bytes other,int other_start=0,int other_end=-1)->bool\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->bool\n"
          "(int my_start,int my_end,bytes other,int other_start=0,int other_end=-1)->bool\n"
          "Same as #wildcompare, returning :true where #wildcompare would return $0, and :false in all other cases") },

    /* Case-insensitive query functions */
    { "casereplace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casereplace,
      DOC("(string find_str,string replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(string find_str,bytes replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(string find_str,int replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(bytes find_str,string replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(bytes find_str,bytes replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(bytes find_str,int replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(int find_str,string replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(int find_str,bytes replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(int find_str,int replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "@throw ValueError The given @find_str or @replace_str is a string containing characters ${> 0xff}\n"
          "@throw IntegerOverflow The given @find_str or @replace_str is an integer lower than $0, or greater than $0xff\n"
          "Same as #replace, however ascii-casing is ignored during character comparisons") },
    { "tocasereplace", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_tocasereplace,
      DOC("(string find_str,string replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(string find_str,bytes replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(string find_str,int replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(bytes find_str,string replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(bytes find_str,bytes replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(bytes find_str,int replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(int find_str,string replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(int find_str,bytes replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "(int find_str,int replace_str,int max_count=int.SIZE_MAX)->bytes\n"
          "@throw ValueError The given @find_str or @replace_str is a string containing characters ${> 0xff}\n"
          "@throw IntegerOverflow The given @find_str or @replace_str is an integer lower than $0, or greater than $0xff\n"
          "@throw ValueError The number of bytes specified by @find_str and @replace_str are not identical\n"
          "@throw BufferError @this bytes object is not writable\n"
          "Same as #toreplace, however ascii-casing is ignored during character comparisons") },
    { "casefind", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casefind,
      DOC("(bytes needle,int start=0,int end=-1)->int\n"
          "(string needle,int start=0,int end=-1)->int\n"
          "(int needle,int start=0,int end=-1)->int\n"
          "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
          "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
          "Same as #find, however ascii-casing is ignored during character comparisons") },
    { "caserfind", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_caserfind,
      DOC("(bytes needle,int start=0,int end=-1)->int\n"
          "(string needle,int start=0,int end=-1)->int\n"
          "(int needle,int start=0,int end=-1)->int\n"
          "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
          "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
          "Same as #rfind, however ascii-casing is ignored during character comparisons") },
    { "caseindex", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_caseindex,
      DOC("(bytes needle,int start=0,int end=-1)->int\n"
          "(string needle,int start=0,int end=-1)->int\n"
          "(int needle,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @needle can be found within ${this.substr(start,end)}\n"
          "Same as #index, however ascii-casing is ignored during character comparisons") },
    { "caserindex", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_caserindex,
      DOC("(bytes needle,int start=0,int end=-1)->int\n"
          "(string needle,int start=0,int end=-1)->int\n"
          "(int needle,int start=0,int end=-1)->int\n"
          "@throw IndexError No instance of @needle can be found within ${this.substr(start,end)}\n"
          "Same as #rindex, however ascii-casing is ignored during character comparisons") },
    { "casefindall", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casefindall,
      DOC("(bytes needle,int start=0,int end=-1)->{int...}\n"
          "(string needle,int start=0,int end=-1)->{int...}\n"
          "(int needle,int start=0,int end=-1)->{int...}\n"
          "Same as #findall, however ascii-casing is ignored during character comparisons") },
    { "casecount", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casecount,
      DOC("(bytes needle,int start=0,int end=-1)->int\n"
          "(string needle,int start=0,int end=-1)->int\n"
          "(int needle,int start=0,int end=-1)->int\n"
          "Same as #count, however ascii-casing is ignored during character comparisons") },
    { "casecontains", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casecontains_f,
      DOC("(bytes needle,int start=0,int end=-1)->bool\n"
          "(string needle,int start=0,int end=-1)->bool\n"
          "(int needle,int start=0,int end=-1)->bool\n"
          "Same as #contains, however ascii-casing is ignored during character comparisons") },
    { "casestrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casestrip,
      DOC("->bytes\n"
          "(string mask)->bytes\n"
          "(bytes mask)->bytes\n"
          "(int mask)->bytes\n"
          "Same as #strip, however ascii-casing is ignored during character comparisons") },
    { "caselstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_caselstrip,
      DOC("->bytes\n"
          "(string mask)->bytes\n"
          "(bytes mask)->bytes\n"
          "(int mask)->bytes\n"
          "Same as #lstrip, however ascii-casing is ignored during character comparisons") },
    { "caserstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_caserstrip,
      DOC("->bytes\n"
          "(string mask)->bytes\n"
          "(bytes mask)->bytes\n"
          "(int mask)->bytes\n"
          "Same as #rstrip, however ascii-casing is ignored during character comparisons") },
    { "casesstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casesstrip,
      DOC("(string other)->bytes\n"
          "(bytes other)->bytes\n"
          "(int other)->bytes\n"
          "Same as #sstrip, however ascii-casing is ignored during character comparisons") },
    { "caselsstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_caselsstrip,
      DOC("(string other)->bytes\n"
          "(bytes other)->bytes\n"
          "(int other)->bytes\n"
          "Same as #lsstrip, however ascii-casing is ignored during character comparisons") },
    { "casersstrip", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casersstrip,
      DOC("(string other)->bytes\n"
          "(bytes other)->bytes\n"
          "(int other)->bytes\n"
          "Same as #rsstrip, however ascii-casing is ignored during character comparisons") },
    { "casestartswith", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casestartswith,
      DOC("(string needle,int start=0,int end=-1)->bool\n"
          "(bytes needle,int start=0,int end=-1)->bool\n"
          "(int needle,int start=0,int end=-1)->bool\n"
          "Same as #startswith, however ascii-casing is ignored during character comparisons") },
    { "caseendswith", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_caseendswith,
      DOC("(string needle,int start=0,int end=-1)->bool\n"
          "(bytes needle,int start=0,int end=-1)->bool\n"
          "(int needle,int start=0,int end=-1)->bool\n"
          "Same as #endswith, however ascii-casing is ignored during character comparisons") },
    { "casepartition", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_caseparition,
      DOC("(string needle,int start=0,int end=-1)->(bytes,bytes,bytes)\n"
          "(bytes needle,int start=0,int end=-1)->(bytes,bytes,bytes)\n"
          "(int needle,int start=0,int end=-1)->(bytes,bytes,bytes)\n"
          "Same as #partition, however ascii-casing is ignored during character comparisons") },
    { "caserpartition", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_caserparition,
      DOC("(string needle,int start=0,int end=-1)->(bytes,bytes,bytes)\n"
          "(bytes needle,int start=0,int end=-1)->(bytes,bytes,bytes)\n"
          "(int needle,int start=0,int end=-1)->(bytes,bytes,bytes)\n"
          "Same as #rpartition, however ascii-casing is ignored during character comparisons") },
    { "casecompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casecompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,bytes other,int other_start=0,int other_end=-1)->int\n"
          "Same as #compare, however ascii-casing is ignored during character comparisons") },
    { "casevercompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casevercompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,bytes other,int other_start=0,int other_end=-1)->int\n"
          "Same as #vercompare, however ascii-casing is ignored during character comparisons") },
    { "casewildcompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casewildcompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,bytes other,int other_start=0,int other_end=-1)->int\n"
          "Same as #wildcompare, however ascii-casing is ignored during character comparisons") },
    { "casefuzzycompare", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casefuzzycompare,
      DOC("(string other,int other_start=0,int other_end=-1)->int\n"
          "(bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,bytes other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->int\n"
          "(int my_start,int my_end,bytes other,int other_start=0,int other_end=-1)->int\n"
          "Same as #fuzzycompare, however ascii-casing is ignored during character comparisons") },
    { "casewmatch", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casewmatch,
      DOC("(string other,int other_start=0,int other_end=-1)->bool\n"
          "(bytes other,int other_start=0,int other_end=-1)->bool\n"
          "(int my_start,string other,int other_start=0,int other_end=-1)->bool\n"
          "(int my_start,bytes other,int other_start=0,int other_end=-1)->bool\n"
          "(int my_start,int my_end,string other,int other_start=0,int other_end=-1)->bool\n"
          "(int my_start,int my_end,bytes other,int other_start=0,int other_end=-1)->bool\n"
          "Same as #casewmatch, however ascii-casing is ignored during character comparisons") },

    /* Bytes alignment functions. */
    { "center", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_center,
      DOC("(int width,string filler=\" \")->string\n"
          "(int width,bytes filler)->string\n"
          "(int width,int filler)->string\n"
          "Use a writable copy of @this bytes object as result, then evenly "
          "insert @filler at the front and back to pad its length to @width bytes") },
    { "ljust", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_ljust,
      DOC("(int width,string filler=\" \")->string\n"
          "Use a writable copy of @this bytes object as result, then "
          "insert @filler at the back to pad its length to @width bytes") },
    { "rjust", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_rjust,
      DOC("(int width,string filler=\" \")->string\n"
          "Use a writable copy of @this bytes object as result, then "
          "insert @filler at the front to pad its length to @width bytes") },
    { "zfill", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_zfill,
      DOC("(int width,string filler=\"0\")->string\n"
          "Skip leading ${\'+\'} and ${\'-\'} ascii-characters, then insert @filler "
          "to pad the resulting string to a length of @width bytes") },
    { "reversed", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_reversed,
      DOC("(int start=0,int end=-1)->bytes\n"
          "Return a copy of the sub-string ${this.substr(start,end)} with its byte order reversed") },
    { "expandtabs", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_expandtabs,
      DOC("(int tabwidth=8)->bytes\n"
          "Expand tab characters with whitespace offset from the "
          "start of their respective line at multiples of @tabwidth\n"
          "Note that in the event of no tabs being found, @this bytes object may be re-returned") },
    { "unifylines", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_unifylines,
      DOC("(string replacement=\"\\n\")->bytes\n"
          "(bytes replacement)->bytes\n"
          "(int replacement)->bytes\n"
          "Unify all ascii-linefeed character sequences ($\"\\n\", $\"\\r\" and $\"\\r\\n\") "
          "found in @this bytes object to make exclusive use of @replacement\n"
          "Note that in the event of no line-feeds differing from @replacement being found, "
          "@this bytes object may be re-returned") },

    /* Bytes splitter functions. */
    { "split", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_split,
      DOC("(bytes needle)->{bytes...}\n"
          "(string needle)->{bytes...}\n"
          "(int needle)->{bytes...}\n"
          "Split @this bytes object at each instance of @sep, "
          "returning a sequence of the resulting parts\n"
          "The returned bytes objects are views of @this byte object, meaning they "
          "have the same #iswritable characteristics as @this, and refer to the same "
          "memory") },
    { "casesplit", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_casesplit,
      DOC("(bytes needle)->{bytes...}\n"
          "(string needle)->{bytes...}\n"
          "(int needle)->{bytes...}\n"
          "Same as #split, however ascii-casing is ignored during character comparisons\n"
          "The returned bytes objects are views of @this byte object, meaning they "
          "have the same #iswritable characteristics as @this, and refer to the same "
          "memory") },
    { "splitlines", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_splitlines,
      DOC("(bool keepends=false)->{bytes...}\n"
          "Split @this bytes object at each linefeed, returning a sequence of all contained lines\n"
          "When @keepends is :false, this is identical to ${this.unifylines().split(\"\\n\")}\n"
          "When @keepends is :true, items found in the returned sequence will still have their "
          "original, trailing line-feed appended\n"
          "This function recognizes $\"\\n\", $\"\\r\" and $\"\\r\\n\" as linefeed sequences\n"
          "The returned bytes objects are views of @this byte object, meaning they "
          "have the same #iswritable characteristics as @this, and refer to the same "
          "memory") },

    /* Bytes-specific functions. */
    { "resized", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_resized,
      DOC("(int new_size)->bytes\n"
          "(int new_size,int filler)->bytes\n"
          "Return a new writable bytes object with a length of @new_size, and its "
          "first ${(#this,new_size) < ...} bytes initialized from ${this.substr(0,new_size)}, "
          "with the remainder then either left uninitialized, or initialized to @filler\n"
          "Note that because a bytes object cannot be resized in-line, code using this function "
          "must make use of the returned bytes object:\n"
          ">local x = \"foobar\";\n"
          ">local y = x.bytes();\n"
          ">print repr y; /* \"foobar\" */\n"
          ">y = y.resized(16,\"?\".ord());\n"
          ">print repr y; /* \"foobar??" "??" "??" "??" "??\" */") },
    { "reverse", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&bytes_reverse,
      DOC("(int start=0,int end=-1)->bytes\n"
          "@throw BufferError @this bytes object is not writable\n"
          "Same as #reversed, but modifications are performed "
          "in-line, before @this bytes object is re-returned") },


    { NULL }
};


DECL_END

#ifndef __INTELLISENSE__
#include "bytes_split.c.inl"
#endif

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FUNCTIONS_C_INL */
