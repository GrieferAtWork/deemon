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
#ifndef GUARD_DEEMON_RUNTIME_BUILD_VALUE_C
#define GUARD_DEEMON_RUNTIME_BUILD_VALUE_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/object.h>
#include <deemon/none.h>
#include <deemon/error.h>
#include <deemon/thread.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/format.h>
#include <deemon/tuple.h>

#include <stddef.h>
#include <string.h>

#include "runtime_error.h"

DECL_BEGIN

#if defined(__x86_64__) || defined(__VA_LIST_IS_ARRAY)
#define CONFIG_VA_LIST_IS_ARRAY 1
#endif

#ifdef CONFIG_VA_LIST_IS_ARRAY
#define VALIST_ADDR(x) (&(x))
//#define VALIST_ADDR(x)   (x)
//#define VALIST_ADDR(x) (&(x)[0])
#else
#define VALIST_ADDR(x) (&(x))
#endif

PRIVATE size_t DCALL
count_pack_args(char const *__restrict format) {
 size_t result = 0;
 for (;;) {
  char ch = *format++;
  switch (ch) {
   /* Length modifiers / ignored. */
  case ',':
  case 'h':
  case 'l':
  case 'I':
  case 'L':
  case '.':
  case '*':
  case '?':
  case '$':
   break;
  case 'n': case '-': /* none */
  case 'o': case 'O': /* object */
  case 'f': case 'D': /* float */
  case 'd': case 'i': /* signed int */
  case 'u': case 'x': /* unsigned int */
  case 'b':           /* bool */
  case 's':           /* string */
   ++result;
   break;

  {
   char tagr;
   unsigned int recursion;
  case '[':
  case '(':
  case '{':
  case '<':
   recursion = 1;
   tagr = ch+2;
   if (ch == '(') --tagr;
   for (; *format; ++format) {
    /* */if (*format == ch) ++recursion;
    else if (*format == tagr) { if (!--recursion) { ++format; break; } }
    else if (!*format) break;
   }
   ++result;
  } break;

  case '\0':
   goto done;

  default:
   if (ch >= '0' && ch <= '9')
       break;
   goto done;
  }
 }
done:
 return result;
}

PRIVATE size_t DCALL
count_unpack_args(char const **__restrict pformat) {
 size_t result = 0;
 char const *format = *pformat;
 while (*format) {
  switch (*format++) {
   /* Length modifiers / ignored. */
  case ',':
  case 'h':
  case 'l':
  case 'I':
  case 'L':
  case 'U':
   break;
  case 'n': case '-': /* none */
  case 'o':           /* object */
  case 's':           /* string */
  case 'f': case 'D': /* float */
  case 'd': case 'i': /* signed int */
  case 'u': case 'x': /* unsigned int */
   ++result;
   break;

  {
   unsigned int recursion;
  case '(':
   recursion = 1;
   for (; *format; ++format) {
    /* */if (*format == '(') ++recursion;
    else if (*format == ')') { if (!--recursion) { ++format; break; } }
    else if (!*format) break;
   }
   ++result;
  } break;

  default:
   if (format[-1] >= '0' && format[-1] <= '9')
       break;
   ATTR_FALLTHROUGH
  case '\0':
   --format;
   goto done;
  }
 }
done:
 *pformat = format;
 return result;
}



PUBLIC void DCALL
Dee_VPPackf_Cleanup(char const *__restrict format, va_list args) {
again:
 switch (*format++) {

 case '(':
 case '[':
 case '{':
 case '<': /* Recursion (Can be ignored here) */
 case ',': /* Separator. */
 case 'n': /* `none' */
 case '-': goto again;
 case 'o': va_arg(args,DeeObject *); goto again;

 case 'L':
  ASSERTF(*format == 'D',"Invalid format: `%s'",format);
  va_arg(args,long double);
  goto again;

 case 'f':
 case 'D':
  va_arg(args,double); /* NOTE: C promotes `float' to double in varargs. */
  goto again;

 case '.':
  if (*format == '*') {
   va_arg(args,unsigned int);
   ++format;
  } else if (*format == '?') {
 case '$':
   va_arg(args,size_t);
   ++format;
  } else {
   while (*format >= '0' && *format <= '9') ++format;
  }
  ASSERTF(*format == 's',"Invalid format: `%s'",format);
  goto again;

 {
  unsigned int length;
 case 'l':
  length = sizeof(long);
  if (*format == 'l') {
#ifdef __SIZEOF_LONG_LONG__
   length = __SIZEOF_LONG_LONG__;
#else
   length = 8;
#endif
   ++format;
  }
  goto do_int;
 case 'h':
  length = sizeof(short);
  if (*format == 'h') { length = sizeof(char); ++format; }
  ++format;
  goto do_int;
 case 'I':
  length = sizeof(size_t);
  /* */if (*format == '8') { length = 1; format += 1; }
  else if (*format == '1') { ASSERTF(format[1] == '6',"Invalid format: `%s'",format); length = 2; format += 2; }
  else if (*format == '3') { ASSERTF(format[1] == '2',"Invalid format: `%s'",format); length = 4; format += 2; }
  else if (*format == '6') { ASSERTF(format[1] == '4',"Invalid format: `%s'",format); length = 8; format += 2; }
  ++format;
  goto do_int;
 case 'd': case 'u': case 'i': case 'x':
 case 'b':
  length = sizeof(int);
do_int:
  ASSERTF(format[-1] == 'd' || *format == 'u' ||
          format[-1] == 'i' || *format == 'x' ||
          format[-1] == 'b',"Invalid format: `%s'",format);
  if (length <= sizeof(int))
      va_arg(args,int);
#if __SIZEOF_INT__ < 2
  else if (length == 2) va_arg(args,int16_t);
#endif
#if __SIZEOF_INT__ < 4
  else if (length == 4) va_arg(args,int32_t);
#endif
  else va_arg(args,int64_t);
  goto again;
 }

 {
  DeeObject *temp;
 case 'O': /* _Always_ inherit reference to `O' operands. */
  temp = va_arg(args,DeeObject *);
  Dee_XDecref(temp);
  goto again;
 }

 default:
  ASSERTF(!*format,"Invalid format: `%s'",format);
  break;
 }
}


PUBLIC DREF DeeObject *DCALL
Dee_VPPackf(char const **__restrict pformat,
            struct va_list_struct *__restrict pargs) {
 DREF DeeObject *result;
 char const *format = *pformat;
again:
 switch (*format++) {

 case ',':
  goto again;

 case '\0':
  --format;
  ATTR_FALLTHROUGH
 case 'n':
 case '-':
  result = Dee_None;
  Dee_Incref(result);
  break;

 case 'o':
 case 'O':
  result = va_arg(pargs->vl_ap,DeeObject *);
  if (format[-1] == 'o') {
   ASSERTF(result,"NULL-object passed as `o' operand");
   Dee_Incref(result);
  }
  break;

 {
  int length;
  union { int32_t i32; uint32_t u32; int64_t i64; uint64_t u64; } data;
 case 'h':
  length = sizeof(short);
  if (*format++ == 'h') { ++format; length = sizeof(char); }
  goto has_length;
 case 'l':
  length = sizeof(long);
#ifdef __SIZEOF_LONG_LONG__
  if (*format++ == 'l') { ++format; length = __SIZEOF_LONG_LONG__; }
#else
  if (*format++ == 'l') { ++format; length = 8; }
#endif
  goto has_length;
 case 'I':
  length = sizeof(size_t);
  /* */if (*format == '8') { ++format; length = 1; }
  else if (*format == '1') { ASSERTF(format[1] == '6',"Invalid format: `%s' (`%s')",format,*pformat); format += 2; length = 2; }
  else if (*format == '3') { ASSERTF(format[1] == '2',"Invalid format: `%s' (`%s')",format,*pformat); format += 2; length = 4; }
  else if (*format == '6') { ASSERTF(format[1] == '4',"Invalid format: `%s' (`%s')",format,*pformat); format += 2; length = 8; }
  ++format;
  goto has_length;
 case 'd': case 'i': case 'u': case 'x': case 'b':
  length = sizeof(int);
has_length:
  ASSERTF(format[-1] == 'd' || format[-1] == 'u' ||
          format[-1] == 'i' || format[-1] == 'x' ||
          format[-1] == 'b',"Invalid format: `%s'",format);
#if __SIZEOF_INT__ < 2
  if (length == 1)
   data.u32 = (uint32_t)va_arg(pargs->vl_ap,uint8_t);
  else
#endif
#if __SIZEOF_INT__ < 4
  if (length == 2)
   data.u32 = (uint32_t)va_arg(pargs->vl_ap,uint16_t);
  else
#endif
#if __SIZEOF_INT__ < 8
  if (length == 4)
   data.u32 = va_arg(pargs->vl_ap,uint32_t);
  else
#endif
  { data.u64 = va_arg(pargs->vl_ap,uint64_t); }
  if (format[-1] == 'b') {
   /* Boolean. */
   result = DeeBool_For(length > 4 ? (data.u64 != 0) : (data.u32 != 0));
   Dee_Incref(result);
  } else if (format[-1] == 'd' || format[-1] == 'i') {
   /* Signed integer. */
   if (length > 4) result = DeeInt_NewS64(data.i64);
   else            result = DeeInt_NewS32(data.i32);
  } else {
   /* Unsigned integer. */
   if (length > 4) result = DeeInt_NewU64(data.u64);
   else            result = DeeInt_NewU32(data.u32);
  }
 } break;

 {
  size_t string_length;
  char *string;
 case '$':
  ASSERTF(*format == 's',"Invalid format: `%s' (`%s')",format,*pformat);
  ++format;
  string_length = va_arg(pargs->vl_ap,size_t);
  string = va_arg(pargs->vl_ap,char *);
  goto do_string;
case 's':
  string = va_arg(pargs->vl_ap,char *);
  string_length = strlen(string);
  goto do_string;
 case '.':
#if !defined(__SIZEOF_INT__) || !defined(__SIZEOF_SIZE_T__) || \
            (__SIZEOF_INT__ != __SIZEOF_SIZE_T__)
  if (*format == '*') {
   string_length = (size_t)va_arg(pargs->vl_ap,unsigned int);
   goto do_strnlen;
  } else if (*format == '?')
#else
  if (*format == '*' || *format == '?')
#endif
  {
   string_length = va_arg(pargs->vl_ap,size_t);
do_strnlen:
   string = va_arg(pargs->vl_ap,char *);
   string_length = strnlen(string,string_length);
  } else {
   ASSERTF(*format >= '0' && *format <= '9',
           "Invalid format: `%s' (`%s')",format,*pformat);
   string_length = 0;
   do string_length = (string_length*10)+(*format - '0');
   while ((++format,*format >= '0' && *format <= '9'));
   goto do_strnlen;
  }
do_string:
  ASSERTF(format[-1] == 's',"Invalid format: `%s' (`%s')",format,*pformat);
  result = DeeString_NewSized(string,string_length);
 } break;


 {
  size_t num_args;
  DeeObject **iter;
 case '(':
  num_args = count_pack_args(format);
  result = DeeTuple_NewUninitialized(num_args);
  if unlikely(!result) break;
  iter = DeeTuple_ELEM(result);
  while (num_args--) {
   if unlikely((*iter++ = Dee_VPPackf(&format,pargs)) == NULL) {
    /* Propagate an error. */
    while (--iter != DeeTuple_ELEM(result)) Dee_Decref(*iter);
    DeeTuple_FreeUninitialized(result);
    result = NULL;
    goto end;
   }
  }
  ASSERTF(*format == ')',"Invalid format: `%s' (`%s')",format,*pformat);
  ++format;
 } break;

 /* TODO: tuple, set, cell */
 /* TODO: float */

 default:
  ASSERTF(0,"Invalid format: `%s' (`%s')",format,*pformat);
  __builtin_unreachable();
  break;

 }
end:
 *pformat = format;
 return result;
}


PUBLIC int (DCALL Dee_VPUnpackf)(DeeObject *__restrict self,
                                 char const **__restrict pformat,
                                 struct va_list_struct *__restrict pargs) {
 char const *format = *pformat;
again:
 switch (*format++) {

 case ',':
  goto again;

 {
  DREF DeeObject *iterator,*elem; size_t argc;
  bool is_optional; char const *fmt_start; int temp;
 case '(':
  /* Unpack a sequence. */
  iterator = DeeObject_IterSelf(self);
  if unlikely(!iterator) return -1;
  is_optional = false,fmt_start = format,argc = 0;
  while (*format) {
   if (*format == '|') { is_optional = true; ++format; }
   elem = DeeObject_IterNext(iterator);
   if unlikely(!elem) return -1;
   if (elem == ITER_DONE) { if (!is_optional && *format && *format != ')') goto invalid_argc2; break; }
   else if (*format == ')') goto invalid_argc;
   if (!*format || *format == ')') { if (!is_optional) goto invalid_argc; break; }
   /* Recursively unpacked contained objects. */
   temp = Dee_VPUnpackf(elem,&format,pargs);
   Dee_Decref(elem);
   if unlikely(temp) return temp;
   ++argc;
  }
  Dee_Decref(iterator);
  if (*format == ')') ++format;
  break;
invalid_argc:
  /* Count the remaining arguments for the error message. */
  do {
   Dee_Decref(elem);
   ++argc;
   if (DeeThread_CheckInterrupt())
       goto err_iter;
  } while (ITER_ISOK(elem = DeeObject_IterNext(iterator)));
  if unlikely(!elem) {
err_iter:
   Dee_Decref(iterator);
   return -1;
  }
invalid_argc2:
  Dee_Decref(iterator);
  {
   size_t argc_min;
   size_t argc_max;
   format = fmt_start;
   argc_min = argc_max = count_unpack_args(&format);
   if (*format == '|') {
    ++format;
    argc_max += count_unpack_args(&format);
   }
   err_invalid_argc_unpack(self,argc,argc_min,argc_max);
  }
  return -1;
 } break;

  /* Ignore this object. */
 case 'n':
 case '-':
  break;

 case 'o': /* Store the object as-is. */
  *va_arg(pargs->vl_ap,DeeObject **) = self;
  break;

 {
  void *str;
 case 'U': /* Store a unicode string. */
  ASSERTF((format[0] == '1' && format[1] == '6') ||
          (format[0] == '3' && format[1] == '2'),
          "Invalid format: `%s' (`%s')",format,*pformat);
  ASSERTF(format[2] == 's',"Invalid format: `%s' (`%s')",format,*pformat);
  str = format[0] == '1'
      ? (void *)DeeString_As2Byte(self)
      : (void *)DeeString_As4Byte(self)
      ;
  if unlikely(!str) return -1;
  format += 3;
  *va_arg(pargs->vl_ap,void **) = str;
 } break;


 case 's': /* Store a string. */
  if (DeeObject_AssertTypeExact(self,&DeeString_Type))
      return -1;
  *va_arg(pargs->vl_ap,char **) = DeeString_STR(self);
  break;

 {
  void *str;
 case '$': /* Store a string, including its length. */
  if (DeeObject_AssertTypeExact(self,&DeeString_Type))
      return -1;
  if (*format == 'U') {
   ASSERTF((format[1] == '1' && format[2] == '6') ||
           (format[1] == '3' && format[2] == '2'),
           "Invalid format: `%s' (`%s')",format,*pformat);
   format += 3;
   str = format[-1] == '2'
       ? (void *)DeeString_As2Byte(self)
       : (void *)DeeString_As4Byte(self)
       ;
  } else {
   str = DeeString_AsUtf8(self);
  }
  if unlikely(!str) return -1;
  ASSERTF(*format == 's',"Invalid format: `%s' (`%s')",format,*pformat);
  ++format;
  *va_arg(pargs->vl_ap,size_t *) = WSTR_LENGTH(str);
  *va_arg(pargs->vl_ap,void **)  = str;
 } break;

 {
  double value;
 case 'L':
  ASSERTF(*format == 'D',"Invalid format: `%s' (`%s')",format,*pformat);
  ++format;
  ATTR_FALLTHROUGH
 case 'f':
 case 'D':
  if (DeeObject_AsDouble(self,&value))
      return -1;
  if (format[-1] == 'f')
       *va_arg(pargs->vl_ap,float *) = (float)value;
  else if (format[-2] != 'L')
       *va_arg(pargs->vl_ap,double *) = value;
  else *va_arg(pargs->vl_ap,long double *) = (long double)value;
 } break;

 {
  int temp;
 /* Bool */
 case 'b':
  temp = DeeObject_Bool(self);
  if unlikely(temp < 0) return temp;
  *va_arg(pargs->vl_ap,bool *) = !!temp;
 } break;

 /* Int */
 {
  int length,temp;
#define LEN_INT_IB1 0
#define LEN_INT_IB2 1
#define LEN_INT_IB4 2
#define LEN_INT_IB8 3
#define LEN_INT2(n) LEN_INT_IB##n
#define LEN_INT(n) LEN_INT2(n)
 case 'c':
  length = 1;
  goto do_integer_format;
 case 'l':
  if (*format == 's') {
   void *str;
   /* Wide-character string. */
   if (DeeObject_AssertTypeExact(self,&DeeString_Type))
       return -1;
   str = DeeString_AsWide(self);
   if unlikely(!str) return -1;
   *va_arg(pargs->vl_ap,void **) = str;
   break;
  }
  ATTR_FALLTHROUGH
 case 'I': case 'h':
 case 'd': case 'u': case 'i': case 'x':
  length = LEN_INT(__SIZEOF_INT__);
do_integer_format:
  if (format[-1] == 'I') {
   if (*format == '8') {
    length = LEN_INT(1);
    format += 2;
   } else if (*format == '1') {
    ASSERTF(format[1] == '6',"Invalid format: `%s' (`%s')",format,*pformat);
    length = LEN_INT(2);
    format += 3;
   } else if (*format == '3') {
    ASSERTF(format[1] == '2',"Invalid format: `%s' (`%s')",format,*pformat);
    length = LEN_INT(4);
    format += 3;
   } else if (*format == '6') {
    ASSERTF(format[1] == '4',"Invalid format: `%s' (`%s')",format,*pformat);
    length = LEN_INT(8);
    format += 3;
   } else {
    length = LEN_INT(__SIZEOF_SIZE_T__);
    format += 1;
   }
  } else if (format[-1] == 'h') {
   length = LEN_INT(__SIZEOF_SHORT__);
   if (*format == 'h') { ++format; length = LEN_INT(__SIZEOF_CHAR__); }
   ++format;
  } else if (format[-1] == 'l') {
   length = LEN_INT(__SIZEOF_LONG__);
   if (*format == 'l') {
    ++format;
#ifdef __SIZEOF_LONG_LONG__
    length = LEN_INT(__SIZEOF_LONG_LONG__);
#else
    length = LEN_INT(8);
#endif
   }
   ++format;
  }
  if (format[-1] == 'd' || format[-1] == 'i') {
   switch (length) { /* signed int */
   case LEN_INT_IB1: temp = DeeObject_AsInt8 (self,va_arg(pargs->vl_ap,int8_t  *)); break;
   case LEN_INT_IB2: temp = DeeObject_AsInt16(self,va_arg(pargs->vl_ap,int16_t *)); break;
   case LEN_INT_IB4: temp = DeeObject_AsInt32(self,va_arg(pargs->vl_ap,int32_t *)); break;
   default:          temp = DeeObject_AsInt64(self,va_arg(pargs->vl_ap,int64_t *)); break;
   }
  } else if (format[-1] == 'u' || format[-1] == 'x') {
parse_unsigned_int:
   switch (length) { /* unsigned int */
   case LEN_INT_IB1: temp = DeeObject_AsUInt8 (self,va_arg(pargs->vl_ap,uint8_t  *)); break;
   case LEN_INT_IB2: temp = DeeObject_AsUInt16(self,va_arg(pargs->vl_ap,uint16_t *)); break;
   case LEN_INT_IB4: temp = DeeObject_AsUInt32(self,va_arg(pargs->vl_ap,uint32_t *)); break;
   default:          temp = DeeObject_AsUInt64(self,va_arg(pargs->vl_ap,uint64_t *)); break;
   }
  } else {
   ASSERTF(format[-1] == 'c',"Invalid format: `%s' (`%s')",format,*pformat);
   /* Unicode character (either a single-character string/bytes object, or an integer). */
   if (DeeString_Check(self)) {
    uint32_t ch;
    if (DeeString_WLEN(self) != 1)
        err_expected_single_character_string(self);
    ch = DeeString_GetChar(self,0);
    switch (length) { /* unsigned int */
    case LEN_INT_IB1:
     if (ch > 0xff) { err_integer_overflow_i(8,true); goto err; }
     *va_arg(pargs->vl_ap,uint8_t *) = (uint8_t)ch;
     break;
    case LEN_INT_IB2:
     if (ch > 0xffff) { err_integer_overflow_i(16,true); goto err; }
     *va_arg(pargs->vl_ap,uint16_t *) = (uint16_t)ch;
     break;
    case LEN_INT_IB4:
     *va_arg(pargs->vl_ap,uint32_t *) = (uint32_t)ch;
     break;
    default:
     *va_arg(pargs->vl_ap,uint64_t *) = (uint64_t)ch;
     break;
    }
    temp = 0;
   } else {
    goto parse_unsigned_int;
   }
  }
  if unlikely(temp < 0) return temp;
 } break;

 default:
  ASSERTF(!*format,"Invalid format: `%s' (`%s')",format,*pformat);
  break;
 }
 *pformat = format;
 return 0;
err:
 return -1;
}


PRIVATE char const *DCALL
Dee_VPUnpackfSkip(char const *__restrict format,
                  struct va_list_struct *__restrict pargs) {
again:
 switch (*format++) {

 case ',':
  goto again;

 case '(':
  /* Unpack a sequence. */
  while (*format && *format != ')')
      format = Dee_VPUnpackfSkip(format,pargs);
  break;

  /* Ignore this object. */
 case 'n':
 case '-':
  break;

 case 'L':
  ASSERTF(*format == 'D',"Invalid format: `%s'",format);
  ++format;
  ATTR_FALLTHROUGH
 case 'f':
 case 'D':
 case 'o':
 case 's':
 case 'b':
  va_arg(pargs->vl_ap,void **);
  break;

 case 'U': /* Store a unicode string. */
  ASSERTF((format[0] == '1' && format[1] == '6') ||
          (format[0] == '3' && format[1] == '2'),
          "Invalid format: `%s'",format);
  ASSERTF(format[2] == 's',"Invalid format: `%s'",format);
  format += 3;
  va_arg(pargs->vl_ap,void **);
  break;

 case '$': /* Store a string, including its length. */
  if (*format == 'U') {
   ASSERTF((format[1] == '1' && format[2] == '6') ||
           (format[1] == '3' && format[2] == '2'),
           "Invalid format: `%s'",format);
   format += 3;
  }
  ASSERTF(*format == 's',"Invalid format: `%s'",format);
  ++format;
  va_arg(pargs->vl_ap,size_t *);
  va_arg(pargs->vl_ap,void **);
  break;

 /* Int */
 case 'l':
  if (*format == 's') {
   va_arg(pargs->vl_ap,void **);
   break;
  }
  ATTR_FALLTHROUGH
 case 'I': case 'h':
 case 'd': case 'u': case 'i': case 'x':
 case 'c':
  if (format[-1] == 'I') {
   if (*format == '8') {
    format += 2;
   } else if (*format == '1') {
    ASSERTF(format[1] == '6',"Invalid format: `%s'",format);
    format += 3;
   } else if (*format == '3') {
    ASSERTF(format[1] == '2',"Invalid format: `%s'",format);
    format += 3;
   } else if (*format == '6') {
    ASSERTF(format[1] == '4',"Invalid format: `%s'",format);
    format += 3;
   } else {
    format += 1;
   }
  } else if (format[-1] == 'h') {
   if (*format == 'h') ++format;
   ++format;
  } else if (format[-1] == 'l') {
   if (*format == 'l') ++format;
   ++format;
  }
  va_arg(pargs->vl_ap,void **);
  break;

 default:
  ASSERTF(!*format,"Invalid format: `%s'",format);
  break;
 }
 return format;
}







PUBLIC DREF DeeObject *DCALL
DeeTuple_VNewf(char const *__restrict format, va_list args) {
 struct va_list_struct *pargs; DREF DeeObject *result,**iter;
 size_t tuple_size = count_pack_args(format);
 pargs = (struct va_list_struct *)VALIST_ADDR(args);
 result = DeeTuple_NewUninitialized(tuple_size);
 if unlikely(!result) return NULL;
 iter = DeeTuple_ELEM(result);
 while (tuple_size--) {
  if ((*iter++ = Dee_VPPackf((char const **)&format,pargs)) == NULL)
        goto err;
 }
 ASSERTF(!*format,"Invalid format: `%s'",format);
 return result;
err:
 while (--iter != DeeTuple_ELEM(result)) Dee_Decref(*iter);
 DeeTuple_FreeUninitialized(result);
 Dee_VPPackf_Cleanup(format,pargs->vl_ap);
 return NULL;
}

PUBLIC int (DCALL DeeArg_VUnpack)(size_t argc, DeeObject **__restrict argv,
                                  char const *__restrict format, va_list args) {
 char const *fmt_start = format; bool is_optional = false; int temp;
 struct va_list_struct *pargs = (struct va_list_struct *)VALIST_ADDR(args);
 DeeObject **iter,**end;
 ASSERT(format);
 end = (iter = argv)+argc;
 for (;;) {
  if (*format == '|') { is_optional = true; ++format; }
  if (iter == end) { if (!is_optional && *format && *format != ':') goto invalid_argc; break; }
  if (!*format || *format == ':') goto invalid_argc;
  temp = Dee_VPUnpackf(*iter++,(char const **)&format,pargs);
  if unlikely(temp) return temp;
 }
 return 0;
invalid_argc:
 {
  size_t argc_min;
  size_t argc_max;
  format = fmt_start;
  argc_min = argc_max = count_unpack_args((char const **)&format);
  if (*format == '|') {
   ++format;
   argc_max += count_unpack_args((char const **)&format);
  }
  if (*format == ':') ++format;
  else format = NULL;
  err_invalid_argc(format,argc,argc_min,argc_max);
 }
 return -1;
}
PUBLIC int (DeeArg_Unpack)(size_t argc, DeeObject **__restrict argv,
                           char const *__restrict format, ...) {
 int result; va_list args;
 va_start(args,format);
 result = DeeArg_VUnpack(argc,argv,format,args);
 va_end(args);
 return result;
}


LOCAL size_t DCALL
kwds_findstr(DeeKwdsObject *__restrict self,
             char const *__restrict name,
             dhash_t hash) {
 dhash_t i,perturb;
 perturb = i = hash & self->kw_mask;
 for (;; i = (i << 2) + i + perturb + 1,perturb >>= 5) {
  struct kwds_entry *entry;
  entry = &self->kw_map[i & self->kw_mask];
  if (!entry->ke_name) break;
  if (entry->ke_hash != hash) continue;
  if (strcmp(DeeString_STR(entry->ke_name),name) != 0) continue;
  return entry->ke_index;
 }
 return (size_t)-1;
}

PUBLIC int
(DCALL DeeArg_VUnpackKw)(size_t argc, DeeObject **__restrict argv,
                         DeeObject *kw, struct keyword *__restrict kwlist,
                         char const *__restrict format, va_list args) {
 char const *fmt_start; size_t kw_argc;
 bool is_optional; int temp;
 struct va_list_struct *pargs;
 if (!kw) /* Without arguments, do a regular unpack. */
      return DeeArg_VUnpack(argc,argv,format,args);
 fmt_start = format;
 is_optional = false;
 pargs = (struct va_list_struct *)VALIST_ADDR(args);
 if (DeeKwds_Check(kw)) {
  /* Indirect keyword list. */
  size_t pos_argc;
  kw_argc = ((DeeKwdsObject *)kw)->kw_size;
  if unlikely(kw_argc > argc)
     return err_keywords_bad_for_argc(argc,kw_argc);
  /* Parse all argument not passed through keywords. */
  pos_argc = argc - kw_argc;
  while (pos_argc--) {
   if (*format == '|') { is_optional = true; ++format; }
   if (!*format || *format == ':') {
    ASSERTF(!kwlist->k_name,"Keyword list too long");
    goto invalid_argc; /* Too many arguments. */
   }
   temp = Dee_VPUnpackf(*argv++,(char const **)&format,pargs);
   if unlikely(temp) return temp;
   ASSERTF(kwlist->k_name,"Keyword list too short");
   ++kwlist;
  }
  /* All remaining arguments are passed through
   * keywords found in `kwlist .. kwlist + x'. */
  while (kw_argc--) {
   dhash_t keyword_hash; size_t kwd_index;
   if (*format == '|') { is_optional = true; ++format; }
   if (!*format || *format == ':') {
    ASSERTF(!kwlist->k_name,"Keyword list too long");
    goto invalid_argc; /* Too many arguments.
                        * TODO: This can also happen when:
                        * >> function foo(x,bar = none);
                        * >> foo(x: 10, baz: 20);
                        * In this case we should do a fuzzy match and
                        * warn the caller that instead of `baz', they
                        * probably meant `bar' */
   }
   /* Find the matching positional argument. */
   ASSERTF(kwlist->k_name,"Keyword list too short");
   keyword_hash = kwlist->k_hash;
   if (keyword_hash == (dhash_t)-1) {
    /* Lazily calculate the hash the first time around. */
    keyword_hash   = hash_str(kwlist->k_name);
    kwlist->k_hash = keyword_hash;
   }
   kwd_index = kwds_findstr((DeeKwdsObject *)kw,
                             kwlist->k_name,
                             keyword_hash);
   if unlikely(kwd_index == (size_t)-1) {
    /* Argument not given. */
    if (!is_optional) {
     size_t argc_min,argc_max;
     argc_min = argc_max = count_unpack_args((char const **)&format);
     if (*format == '|') { ++format; argc_max += count_unpack_args((char const **)&format); }
     if (*format == ':') ++format;
     else format = NULL;
     if (argc_min == argc_max) {
      return DeeError_Throwf(&DeeError_TypeError,
                             "Missing argument %s in call to function%s%s expecting %Iu arguments when %Iu w%s given",
                             kwlist->k_name,format ? " " : "",format ? format : "",
                             argc_min,argc,argc == 1 ? "as" : "ere");
     } else {
      return DeeError_Throwf(&DeeError_TypeError,
                             "Missing argument %s in call to function%s%s expecting between %Iu and %Iu arguments when %Iu w%s given",
                             kwlist->k_name,format ? " " : "",format ? format : "",
                             argc_min,argc_max,argc,argc == 1 ? "as" : "ere");
     }
    }
    /* The argument is optional, but not given. -> So just skip this one! */
    format = Dee_VPUnpackfSkip(format,pargs);
   } else {
    /* All right! we've got the argument! */
    ASSERT(kwd_index < ((DeeKwdsObject *)kw)->kw_size);
    temp = Dee_VPUnpackf(argv[kwd_index],(char const **)&format,pargs);
    if unlikely(temp) return temp;
   }
   ++kwlist;
  }
  if (*format && *format != ':') {
   /* TODO: This can also happen when:
    * >> function foo(x,bar);
    * >> foo(x: 10, baz: 20);
    * In this case we should do a fuzzy match and
    * warn the caller that instead of `baz', they
    * probably meant `bar' */
   goto invalid_argc; /* Too few arguments. */
  }
  return 0; /* Done! */
 }
 /* Keyword arguments are given, but aren't a `DeeKwds_Type' object.
  * In this situation, we're supposed to interpret them as a mapping-like object!
  * But first off: parse all the positional argument! */
 while (argc--) {
  if (*format == '|') { is_optional = true; ++format; }
  if (!*format || *format == ':') {
   ASSERTF(!kwlist->k_name,"Keyword list too long");
   goto invalid_argc; /* Too many arguments. */
  }
  temp = Dee_VPUnpackf(*argv++,(char const **)&format,pargs);
  if unlikely(temp) return temp;
  ASSERTF(kwlist->k_name,"Keyword list too short");
  ++kwlist;
 }
 /* Now with positional arguments out of the way, move on to the named arguments. */
 kw_argc = 0;
 for (;;) {
  dhash_t keyword_hash;
  DREF DeeObject *keyword_value;
  if (*format == '|') { is_optional = true; ++format; }
  if (!*format || *format == ':') {
   ASSERTF(!kwlist->k_name,"Keyword list too long");
   break; /* End of argument list. */
  }
  /* Find the matching positional argument. */
  ASSERTF(kwlist->k_name,"Keyword list too short");
  keyword_hash = kwlist->k_hash;
  if (keyword_hash == (dhash_t)-1) {
   /* Lazily calculate the hash the first time around. */
   keyword_hash   = hash_str(kwlist->k_name);
   kwlist->k_hash = keyword_hash;
  }
  keyword_value = DeeObject_GetItemStringDef(kw,
                                             kwlist->k_name,
                                             keyword_hash,
                                             ITER_DONE);
  if unlikely(keyword_value == ITER_DONE) {
   /* Argument not given. */
   if (!is_optional) {
    size_t argc_min,argc_max;
    argc_min = argc_max = count_unpack_args((char const **)&format);
    if (*format == '|') { ++format; argc_max += count_unpack_args((char const **)&format); }
    if (*format == ':') ++format;
    else format = NULL;
    if (argc_min == argc_max) {
     return DeeError_Throwf(&DeeError_TypeError,
                            "Missing argument %s in call to function%s%s expecting %Iu arguments when %Iu w%s given",
                            kwlist->k_name,format ? " " : "",format ? format : "",
                            argc_min,argc,argc == 1 ? "as" : "ere");
    } else {
     return DeeError_Throwf(&DeeError_TypeError,
                            "Missing argument %s in call to function%s%s expecting between %Iu and %Iu arguments when %Iu w%s given",
                            kwlist->k_name,format ? " " : "",format ? format : "",
                            argc_min,argc_max,argc,argc == 1 ? "as" : "ere");
    }
   }
   /* The argument is optional, but not given. -> So just skip this one! */
   format = Dee_VPUnpackfSkip(format,pargs);
  } else {
   /* All right! we've got the argument! */
   temp = Dee_VPUnpackf(keyword_value,(char const **)&format,pargs);
   Dee_Decref(keyword_value); /* XXX: This might cause problems for `o'-targets... */
   if unlikely(temp) return temp;
   ++kw_argc;
  }
  ++kwlist;
 }
 /* Make sure that the argument list doesn't contain argument that went unused. */
 {
  size_t kw_size = DeeObject_Size(kw);
  if unlikely(kw_size == (size_t)-1) return -1;
  if (kw_argc != kw_size)
      goto invalid_argc;
 }

 return 0;
invalid_argc:
 {
  size_t argc_min,argc_max;
  format = fmt_start;
  argc_min = argc_max = count_unpack_args((char const **)&format);
  if (*format == '|') {
   ++format;
   argc_max += count_unpack_args((char const **)&format);
  }
  if (*format == ':') ++format;
  else format = NULL;
  return err_invalid_argc(format,argc,argc_min,argc_max);
 }
}
PUBLIC int
(DeeArg_UnpackKw)(size_t argc, DeeObject **__restrict argv,
                  DeeObject *kw, struct keyword *__restrict kwlist,
                  char const *__restrict format, ...) {
 int result; va_list args;
 va_start(args,format);
 result = DeeArg_VUnpackKw(argc,argv,kw,kwlist,format,args);
 va_end(args);
 return result;
}



PUBLIC DREF DeeObject *DCALL
Dee_VPackf(char const *__restrict format, va_list args) {
 DREF DeeObject *result;
 result = Dee_VPPackf((char const **)&format,(struct va_list_struct *)VALIST_ADDR(args));
 if unlikely(!result) Dee_VPPackf_Cleanup(format,((struct va_list_struct *)VALIST_ADDR(args))->vl_ap);
 return result;
}
PUBLIC int (DCALL Dee_VUnpackf)(DeeObject *__restrict self,
                                char const *__restrict format,
                                va_list args) {
 return Dee_VPUnpackf(self,(char const **)&format,
                     (struct va_list_struct *)VALIST_ADDR(args));
}


PUBLIC DREF DeeObject *
Dee_Packf(char const *__restrict format, ...) {
 struct va_list_struct args;
 DREF DeeObject *result;
 va_start(args.vl_ap,format);
 result = Dee_VPPackf((char const **)&format,&args);
 va_end(args.vl_ap);
 if unlikely(!result) Dee_VPPackf_Cleanup(format,args.vl_ap);
 return result;
}
PUBLIC DREF DeeObject *
DeeTuple_Newf(char const *__restrict format, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,format);
 result = DeeTuple_VNewf(format,args);
 va_end(args);
 return result;
}

PUBLIC int (Dee_Unpackf)(DeeObject *__restrict self,
                         char const *__restrict format, ...) {
 int result; struct va_list_struct args;
 va_start(args.vl_ap,format);
 result = Dee_VPUnpackf(self,(char const **)&format,&args);
 va_end(args.vl_ap);
 return result;
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_BUILD_VALUE_C */
