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
#ifndef GUARD_DEEMON_STRING_H
#define GUARD_DEEMON_STRING_H 1

#define CONFIG_USE_NEW_STRING_API 1

#ifdef CONFIG_USE_NEW_STRING_API
#include "string2.h"

#undef CONFIG_WCHAR_STRINGS
#define CONFIG_WCHAR_STRINGS 1

#define su_enc                u_data
#define su_pref               u_width
#define ENCODING_SIZE(x)      WSTR_LENGTH(x)
#define DeeString_UTF(x)      ((DeeStringObject *)(x))->s_data
#define DeeEnc_Size(x)        STRING_SIZEOF_WIDTH(x)
#define DeeString_UtfEnc(x)   DeeString_WIDTH(x)
#define DeeString_UtfLen(x)   DeeString_WLEN(x)
#define DeeString_UtfStr(x)   DeeString_WSTR(x)
#define DeeEnc_Get(enc,base,i) STRING_WIDTH_GETCHAR(enc,base,i)
#define DeeEnc_Set(enc,base,i,v) STRING_WIDTH_SETCHAR(enc,base,i,v)
#define DeeString_AsEncoding(self,enc) DeeString_AsWidth(self,enc)
#define DeeString_UtfGet(self,i) DeeString_GetChar(self,i)
#define DeeString_UtfSet(self,i,v) DeeString_SetChar(self,i,v)
#define DeeString_NewChar     DeeString_New4Char
#define DeeEnc_Is1Byte(x)   ((x) == STRING_WIDTH_1BYTE)
#if __SIZEOF_WCHAR_T__ == 2
#define DeeEnc_Is2Byte(x)   ((x) == STRING_WIDTH_2BYTE || (x) == STRING_WIDTH_WCHAR)
#define DeeEnc_Is4Byte(x)   ((x) == STRING_WIDTH_4BYTE)
#else
#define DeeEnc_Is2Byte(x)   ((x) == STRING_WIDTH_2BYTE)
#define DeeEnc_Is4Byte(x)   ((x) == STRING_WIDTH_4BYTE || (x) == STRING_WIDTH_WCHAR)
#endif

#define string_printer_pack_enc(self,encoding) string_printer_pack(self)
#define DeeString_NewSizedWithEncoding(base,length,enc) \
        DeeString_NewWithWidth(base,length,enc,DEE_STRING_CODEC_FREPLAC)
#define DeeString_UtfForeach(self,ibegin,iend,iter,end,...) \
        DeeString_Foreach(self,ibegin,iend,iter,end,__VA_ARGS__)
#define DeeString_DelBufferEncoding DeeString_FreeWidth

#else /* CONFIG_USE_NEW_STRING_API */

#include "api.h"
#include "object.h"
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

DECL_BEGIN

typedef struct string_object DeeStringObject;

#ifndef __SIZEOF_WCHAR_T__
#ifdef CONFIG_HOST_WINDOWS
#   define __SIZEOF_WCHAR_T__ 2
#else
#   define __SIZEOF_WCHAR_T__ 4
#endif
#endif

/* String encoding codes. */
#define STRING_WIDTH_1BYTE  (-1) /* Special encoding. */
#define STRING_WIDTH_2BYTE   0
#define STRING_WIDTH_4BYTE   1
#ifdef CONFIG_WCHAR_STRINGS
#   define STRING_WIDTH_WCHAR 2
#   define STRING_WIDTH_COUNT 3
#else /* CONFIG_WCHAR_STRINGS */
#if __SIZEOF_WCHAR_T__ == 2
#   define STRING_WIDTH_WCHAR STRING_WIDTH_2BYTE
#else
#   define STRING_WIDTH_WCHAR STRING_WIDTH_4BYTE
#endif
#   define STRING_WIDTH_COUNT 4
#endif /* !CONFIG_WCHAR_STRINGS */

/* Encoding helper macros. */
#ifdef __NO_builtin_expect
#define SWITCH_SIZEOF_WIDTH(x)  switch (x)
#else
#define SWITCH_SIZEOF_WIDTH(x)  switch (__builtin_expect(x,STRING_WIDTH_1BYTE))
#endif
#define CASE_WIDTH_1BYTE      case STRING_WIDTH_1BYTE
#define DeeEnc_Index(x)     (x)
#define DeeEnc_Is1Byte(x)   ((x) == STRING_WIDTH_1BYTE)
#ifdef CONFIG_WCHAR_STRINGS
#define DeeEnc_Size(x)      ((x) == STRING_WIDTH_WCHAR ? __SIZEOF_WCHAR_T__ : 1 << ((x)+1))
#if __SIZEOF_WCHAR_T__ == 2
#define CASE_WIDTH_2BYTE      case STRING_WIDTH_2BYTE: case STRING_WIDTH_WCHAR
#define CASE_WIDTH_4BYTE      case STRING_WIDTH_4BYTE
#define DeeEnc_Is2Byte(x)   ((x) == STRING_WIDTH_2BYTE || (x) == STRING_WIDTH_WCHAR)
#define DeeEnc_Is4Byte(x)   ((x) == STRING_WIDTH_4BYTE)
#define DeeEnc_Get(enc,base,i) \
   (DeeEnc_Is1Byte(enc) ? (uint32_t)((uint8_t *)(base))[i] : \
    DeeEnc_Is4Byte(enc) ?           ((uint32_t *)(base))[i] : \
                          (uint32_t)((uint16_t *)(base))[i])
#define DeeEnc_Set(enc,base,i,v) \
   (DeeEnc_Is1Byte(enc) ? (void)(((uint8_t *)(base))[i] = (uint8_t)(v)) : \
    DeeEnc_Is4Byte(enc) ? (void)(((uint32_t *)(base))[i] = (uint32_t)(v)) : \
                          (void)(((uint16_t *)(base))[i] = (uint16_t)(v)))
#else
#define DeeEnc_Size(x)      (1 << ((x)+1))
#define CASE_WIDTH_2BYTE      case STRING_WIDTH_2BYTE
#define CASE_WIDTH_4BYTE      case STRING_WIDTH_4BYTE: case STRING_WIDTH_WCHAR
#define DeeEnc_Is2Byte(x)   ((x) == STRING_WIDTH_2BYTE)
#define DeeEnc_Is4Byte(x)   ((x) == STRING_WIDTH_4BYTE || (x) == STRING_WIDTH_WCHAR)
#endif
#else
#define CASE_WIDTH_2BYTE      case STRING_WIDTH_2BYTE
#define CASE_WIDTH_4BYTE      case STRING_WIDTH_4BYTE
#define DeeEnc_Is2Byte(x)   ((x) == STRING_WIDTH_2BYTE)
#define DeeEnc_Is4Byte(x)   ((x) == STRING_WIDTH_4BYTE)
#endif

#ifndef DeeEnc_Get
#define DeeEnc_Get(enc,base,i) \
   (DeeEnc_Is1Byte(enc) ? (uint32_t)((uint8_t *)(base))[i] : \
    DeeEnc_Is2Byte(enc) ? (uint32_t)((uint16_t *)(base))[i] : \
                                    ((uint32_t *)(base))[i])
#define DeeEnc_Set(enc,base,i,v) \
   (DeeEnc_Is1Byte(enc) ? (void)(((uint8_t *)(base))[i] = (uint8_t)(v)) : \
    DeeEnc_Is2Byte(enc) ? (void)(((uint16_t *)(base))[i] = (uint16_t)(v)) : \
                          (void)(((uint32_t *)(base))[i] = (uint32_t)(v)))
#endif /* !DeeEnc_Get */


struct string_utf {
    int                su_pref;  /* [const] The preferred encoding of this string. (One of `STRING_ENCODING_*')
                                  *         When present, this field affects the behavior of character positions
                                  *         and the length of the string when viewed from usercode.
                                  *         Unless otherwise specified, this field defaults to `STRING_WIDTH_1BYTE'
                                  *   NOTE: Users may assume that the encoding specified by this field is already
                                  *         already allocated (meaning that `DeeString_AsEncoding()' for this
                                  *         encoding will never fail). */
    union {
        struct {
            /* The string in various other encodings.
             * NOTE: All of these pointers (when set) actually point to the
             *      `text' member of a structure that looks as follows:
             *    >> struct utfXX {
             *    >>     size_t   size;    // Size of `text' in `uintXX_t' integrals.
             *    >>     uintXX_t text[1]; // [0..size] Vector of `uintXX_t' characters.
             *    >> } */
            uint16_t  *su_2byte; /* [0..1][owned][const_if(!= NULL)][lock(WRITE_ONCE)] UTF-16, host-endian. */
            uint32_t  *su_4byte; /* [0..1][owned][const_if(!= NULL)][lock(WRITE_ONCE)] UTF-32, host-endian. */
#ifdef CONFIG_WCHAR_STRINGS
            wchar_t   *su_wchar; /* [0..1][owned][const_if(!= NULL)][lock(WRITE_ONCE)] Wide-character representation.
                                  * HINT: This representation is generated using `mbtowc' or `MultiByteToWideChar' */
#endif /* CONFIG_WCHAR_STRINGS */
        }              su_utf;   /* UTF encodings by name. */
        size_t        *su_enc[STRING_WIDTH_COUNT]; /* Generated string encodings. */
    };
};


DFUNDEF struct string_utf *(DCALL string_utf_alloc)(void);
DFUNDEF void (DCALL string_utf_free)(struct string_utf *__restrict self);
#ifndef NDEBUG
DFUNDEF struct string_utf *(DCALL string_utf_alloc_d)(char const *file, int line);
#define string_utf_alloc() string_utf_alloc_d(__FILE__,__LINE__)
#endif


struct string_object {
    OBJECT_HEAD
    struct string_utf *s_data;      /* [0..1][owned][const_if(!= NULL)][lock(WRITE_ONCE)] Non-UTF-8-encoded string data. */
#define DEE_STRING_HASH_UNSET ((dhash_t)-1) /* A hash-representation of the string. */
    dhash_t            s_hash;      /* [const_if(!= -1)][valid_if(!= -1)][lock(WRITE_ONCE)] Hash value of this string. */
    /* HINT: Because we place `s_len' before `s_str', using `ENCODING_SIZE(s_str)' still
     *       works, which is also why we are able to provide `STRING_WIDTH_1BYTE', too. */
    size_t             s_len;       /* [const] String length. (In UTF-8 bytes) */
    char               s_str[1024]; /* [const] Actual string data (generic byte-sequence, or UTF-8 string). */
};


struct string_printer {
    size_t           sp_length; /* Used string length. */
    DeeStringObject *sp_string; /* [0..1][owned] Generated string object. */
};
#define STRING_PRINTER_INIT  {0,NULL}
#define string_printer_init(self) \
 (void)((self)->sp_length = 0,(self)->sp_string = NULL)
#define string_printer_fini(self) \
         DeeObject_Free((self)->sp_string)
#define STRING_PRINTER_STR(self) ((self)->sp_string->s_str)
#define STRING_PRINTER_LEN(self) ((self)->sp_length)

/* Append the given data to a string printer. (HINT: Use this one as a `dformatprinter') */
DFUNDEF dssize_t DCALL string_printer_print(void *self, char const *__restrict data, size_t datalen);
DFUNDEF char *DCALL string_printer_alloc(void *self, size_t datalen);
/* Release the last `datalen' bytes from the printer to be
 * re-used in subsequent calls, or be truncated eventually. */
DFUNDEF void DCALL string_printer_release(void *self, size_t datalen);
#define STRING_PRINTER_PRINT(self,S)                string_printer_print(self,S,COMPILER_STRLEN(S))
#define string_printer_printf(self,...)             Dee_FormatPrintf(&string_printer_print,self,__VA_ARGS__)
#define string_printer_vprintf(self,format,args)    Dee_VFormatPrintf(&string_printer_print,self,format,args)
/* Print a single character, returning -1 on error or 0 on success. */
DFUNDEF int DCALL string_printer_putc(void *self, char ch);

/* Pack together data from a string printer and return the generated contained string.
 * NOTE: Upon success, `self' will be reset to no longer contain any heap-data, meaning
 *       that a call to `string_printer_fini()' can be omitted in the event of success. */
DFUNDEF DREF DeeObject *DCALL string_printer_pack(struct string_printer *__restrict self);
DFUNDEF DREF DeeObject *DCALL string_printer_pack_enc(struct string_printer *__restrict self, int encoding);
/* Similar to `string_printer_pack()', but always finalize `self',
 * regardless of whether or not the function successed. */
DFUNDEF DREF DeeObject *DCALL string_printer_packfini(struct string_printer *__restrict self);

/* Search the buffer that has already been created for an existing instance
 * of `str...+=length' and if found, return a pointer to its location.
 * Otherwise, append the given string and return a pointer to that location.
 * Upon error (append failed to allocate more memory), NULL is returned.
 * HINT: This function is very useful when creating
 *       string tables for NUL-terminated strings:
 *       >> string_printer_allocstr("foobar\0"); // Table is now `foobar\0'
 *       >> string_printer_allocstr("foo\0");    // Table is now `foobar\0foo\0'
 *       >> string_printer_allocstr("bar\0");    // Table is still `foobar\0foo\0' - `bar\0' points into `foobar\0'
 */
DFUNDEF char *DCALL
string_printer_allocstr(struct string_printer *__restrict self,
                        char const *__restrict str, size_t length);

#define DEFINE_STRING(name,str) \
struct { OBJECT_HEAD struct string_utf *s_data; dhash_t s_hash; size_t s_len; char s_str[sizeof(str)/sizeof(char)]; } \
name = { OBJECT_HEAD_INIT(&DeeString_Type),NULL,(dhash_t)-1,(sizeof(str)/sizeof(char))-1,str }

#ifdef GUARD_DEEMON_OBJECTS_STRING_C
struct empty_string { OBJECT_HEAD struct string_utf *s_data; dhash_t s_hash; size_t s_len; char s_zero; };
DDATDEF struct empty_string DeeString_Empty;
#define Dee_EmptyString ((DeeObject *)&DeeString_Empty)
#else
DDATDEF DeeObject           DeeString_Empty;
#define Dee_EmptyString   (&DeeString_Empty)
#endif
#define return_empty_string  return_reference_(Dee_EmptyString)


#define DeeString_UTF(x)      ((DeeStringObject *)(x))->s_data
#define DeeString_HASHOK(x)  (((DeeStringObject *)(x))->s_hash != (dhash_t)-1)
#define DeeString_HASH(x)     ((DeeStringObject *)(x))->s_hash
#define DeeString_SIZE(x)     ((DeeStringObject *)(x))->s_len
#define DeeString_STR(x)      ((DeeStringObject *)(x))->s_str
#define DeeString_END(x)       (DeeString_STR(x)+DeeString_SIZE(x))
#define DeeString_IsEmpty(x)  (!DeeString_SIZE(x))
#define DeeString_Check(x)      DeeObject_InstanceOfExact(x,&DeeString_Type) /* `string' is `final'. */
#define DeeString_CheckExact(x) DeeObject_InstanceOfExact(x,&DeeString_Type)
DDATDEF DeeTypeObject DeeString_Type;

/* Create new UTF-8 strings. */
DFUNDEF DREF DeeObject *DCALL DeeString_New(char const *__restrict str);
DFUNDEF DREF DeeObject *DCALL DeeString_NewSized(char const *__restrict str, size_t length);
DFUNDEF DREF DeeObject *DCALL DeeString_NewBuffer(size_t length);
DFUNDEF DREF DeeObject *DeeString_Newf(char const *__restrict format, ...);
DFUNDEF DREF DeeObject *DCALL DeeString_VNewf(char const *__restrict format, va_list args);
/* Delete or reload cached buffer encodings from a given UTF-8 string. */
DFUNDEF void DCALL DeeString_DelBufferEncoding(DeeObject *__restrict self);

/* Ensure that the given string has an exact length of `length',
 * potentially re-allocating it to fit that length, though ignoring
 * failures of doing so and simply re-returning `self' after setting
 * its length to `length' regardless.
 * The caller must ensure that `length <= DeeString_UtfLen(self)' upon entry,
 * and the implementation guaranties that `length == DeeString_UtfLen(return)'
 * upon successful exit. */
DFUNDEF ATTR_RETNONNULL DREF DeeObject *DCALL
DeeString_SetBufferWithLength(DREF DeeObject *__restrict self, size_t length);

/* Resize the given string buffer, inheriting the (only) reference
 * to `self' upon success, if `self' was non-NULL upon entry.
 * If `self' is passed as NULL, the call is identical to `DeeString_NewBuffer()' */
DFUNDEF DREF DeeObject *DCALL DeeString_ResizeBuffer(DREF DeeObject *self, size_t length);

/* Return a string containing a single character. */
DFUNDEF DREF DeeObject *DCALL DeeString_NewChar(uint32_t ch);

/* Unicode encoding/errors pair.
 * HINT: If the default error handling `UNICODE_ERRORS_STRICT' is used,
 *       use of these macros can be omitted in favor of directly passing
 *       one of `STRING_ENCODING_*'
 * @param: encoding: One of `STRING_ENCODING_*'
 * @param: errors:   One of `UNICODE_ERRORS_*' */
#define UNICODE_ENC(encoding,errors)          ((errors) << 16 | (uint16_t)(int16_t)(encoding))
#define UNICODE_ERRORS(encoding_and_errors)   ((uint32_t)(encoding_and_errors) >> 16)
#define UNICODE_ENCODING(encoding_and_errors) ((int16_t)(encoding_and_errors))
#define UNICODE_ERRORS_STRICT  0 /* Raise an `Error.UnicodeError.UnicodeDecodeError' (_MUST_ always be ZERO(0)!) */
#define UNICODE_ERRORS_REPLACE 1 /* Replace with a `?' character. */
#define UNICODE_ERRORS_IGNORE  2 /* Ignore (discard) errors. */

/* Create a new string given its representation in
 * encoded as `UNICODE_ENCODING(encoding_and_errors)'
 * NOTE: The returned string will prefer the passed encoding. */
DFUNDEF DREF DeeObject *DCALL
DeeString_NewSizedWithEncoding(void const *__restrict str, size_t length,
                               int encoding_and_errors);
DFUNDEF DREF DeeObject *DCALL
DeeString_NewWithEncoding(void const *__restrict str,
                          int encoding_and_errors);

#ifdef CONFIG_BUILDING_DEEMON
INTDEF dhash_t DCALL DeeString_Hash(DeeObject *__restrict self);
#else
#define DeeString_Hash(self)   DeeObject_Hash(self)
#endif
#define DeeString_HashCase(self) \
    hash_caseptr(DeeString_STR(self),DeeString_SIZE(self))

/* Allocate a new string buffer using the given encoding.
 * Once the string pointed to by `*pencstr' has been initialized
 * by the caller, `DeeString_SetEncodingBuffer()' should be called
 * to generate the mandatory UTF-8 representation.
 * NOTE: `DeeString_SetEncodingBuffer()' _always_ inherits a reference
 *        from the given string object, returning `NULL' on error, or
 *        the string containing the requested data on success.
 * NOTE:  In case a larger buffer than is available is need to fit the
 *        UTF-8 representation when `DeeString_SetEncodingBuffer()' is
 *        called, the passed string is automatically re-allocated to
 *        fit the required size, meaning that the return pointer doesn't
 *        necessarily equal the given `self' pointer.
 * NOTE:  The caller is responsible to not start sharing the string until
 *        `DeeString_SetEncodingBuffer()' has been called, which expects
 *        that `DeeObject_IsShared(self)' is `false'.
 * HINT:  If non-NULL is passed, upon success `*pencstr' is equal to `DeeString_UtfStr(return)'
 */
DFUNDEF DREF DeeObject *DCALL DeeString_NewEncodingBuffer(size_t length, int encoding, void **pencstr);
DFUNDEF DREF DeeObject *DCALL DeeString_SetEncodingBuffer(DREF DeeObject *__restrict self);
/* Same as `DeeString_SetEncodingBuffer()', but given a `length' that may be smaller
 * than the length of the given string, try to truncate the string to fit that length,
 * yet don't throw an error if this re-allocation fails.
 * The caller must ensure that `length <= DeeString_UtfLen(self)' upon entry,
 * and the implementation guaranties that `length == DeeString_UtfLen(return)'
 * upon successful exit. */
DFUNDEF DREF DeeObject *DCALL DeeString_SetEncodingBufferWithLength(DREF DeeObject *__restrict self, size_t length);
/* Resize an encoding buffer to the given `length' and write the new encoding pointer to `pencstr'. */
DFUNDEF DREF DeeObject *DCALL DeeString_RezEncodingBuffer(DREF DeeObject *self, size_t length, int encoding, void **pencstr);

/* Return the pointer to the encoding representation of a
 * given string, creating missing encodings on-the-fly.
 * @return: * :   A pointer to the starting address of the encoded string.
 *                The actual character-type of this string depends on `encoding'.
 *                HINT: The size (in characters) of the encoded string can quickly be
 *                      determined by passing the returned pointer to `ENCODING_SIZE()'
 * @return: NULL: Failed to generate the encoding (memory/unicode-error; An error was thrown). */
DFUNDEF void *DCALL DeeString_AsEncoding(DeeObject *__restrict self, int encoding_and_errors);
#define ENCODING_SIZE(x)   (((size_t *)(x))[-1])

/* Current-encoding string access. */
#define DeeString_UtfEnc(x)   (!DeeString_UTF(x) ? STRING_WIDTH_1BYTE : DeeString_UTF(x)->su_pref)
#define DeeString_UtfStr(x)   ((!DeeString_UTF(x) || DeeString_UTF(x)->su_pref == STRING_WIDTH_1BYTE) ? (void *)DeeString_STR(x) : (void *)DeeString_UTF(x)->su_enc[DeeEnc_Index(DeeString_UTF(x)->su_pref)])
#define DeeString_UtfLen(x)   ((!DeeString_UTF(x) || DeeString_UTF(x)->su_pref == STRING_WIDTH_1BYTE) ? DeeString_SIZE(x) : DeeString_UTF(x)->su_enc[DeeEnc_Index(DeeString_UTF(x)->su_pref)][-1])

LOCAL uint32_t DCALL
DeeString_UtfGet(DeeObject *__restrict self, size_t index) {
 void *str = DeeString_UtfStr(self);
 int enc = DeeString_UtfEnc(self);
 return DeeEnc_Get(enc,str,index);
}
LOCAL void DCALL
DeeString_UtfSet(DeeObject *__restrict self, size_t index, uint32_t value) {
 void *str = DeeString_UtfStr(self);
 int enc = DeeString_UtfEnc(self);
 DeeEnc_Set(enc,str,index,value);
}


#ifdef CONFIG_BUILDING_DEEMON
/* Concat the 2 given strings, inheriting a reference from both on success. */
INTDEF DREF DeeObject *DCALL
DeeString_CatInherited(DREF DeeObject *__restrict lhs,
                       DREF DeeObject *__restrict rhs);
#endif


#define DeeString_UtfForeach(self,ibegin,iend,iter,end,...) \
do{ void *_str_ = DeeString_UtfStr(self); \
    size_t _len_ = ENCODING_SIZE(_str_); \
    if (_len_ > (iend)) \
        _len_ = (iend); \
    if ((ibegin) < _len_) { \
        switch (DeeString_UtfEnc(self)) { \
        { \
            char *iter,*end; \
        default: \
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


#define UNICODE_FPRINT   0x0001 /* The character is printable, or SPC (` '). */
#define UNICODE_FALPHA   0x0002 /* The character is alphabetic. */
#define UNICODE_FSPACE   0x0004 /* The character is a space-character. */
#define UNICODE_FLF      0x0008 /* Line-feed/line-break character. */
#define UNICODE_FLOWER   0x0010 /* Lower-case. */
#define UNICODE_FUPPER   0x0020 /* Upper-case. */
#define UNICODE_FTITLE   0x0040 /* Title-case. */
#define UNICODE_FCONTROL 0x0080 /* Control character. */
#define UNICODE_FDIGIT   0x0100 /* The character is a digit. e.g.: `2' (ascii; `ut_digit' is `2') */
#define UNICODE_FDECIMAL 0x0200 /* The character is a decimal. e.g: `²' (sqare; `ut_digit' is `2') */
#define UNICODE_FSYMSTRT 0x0400 /* The character can be used as the start of an identifier. */
#define UNICODE_FSYMCONT 0x0800 /* The character can be used to continue an identifier. */
/*      UNICODE_F        0x1000 */
/*      UNICODE_F        0x2000 */
/*      UNICODE_F        0x4000 */
/*      UNICODE_F        0x8000 */
typedef uint16_t uniflag_t;

struct unitraits {
    uniflag_t const ut_flags;   /* Character flags (Set of `UNICODE_F*') */
    uint8_t   const ut_digit;   /* Digit/decimal value (`DeeUni_IsNumeric'), or 0. */
    uint8_t   const ut_padding; /* ... */
    int32_t   const ut_lower;   /* Delta added to the character to convert it to lowercase, or 0. */
    int32_t   const ut_upper;   /* Delta added to the character to convert it to uppercase, or 0. */
    int32_t   const ut_title;   /* Delta added to the character to convert it to titlecase, or 0. */
};

/* Unicode character traits database access. */
DFUNDEF ATTR_RETNONNULL ATTR_CONST struct unitraits *DCALL DeeUni_Descriptor(uint32_t ch);
#define DeeUni_Flags(ch)     (DeeUni_Descriptor(ch)->ut_flags)

/* ctype-style character conversion. */
#define DeeUni_ToLower(ch)      ((uint32_t)((ch)+DeeUni_Descriptor(ch)->ut_lower))
#define DeeUni_ToUpper(ch)      ((uint32_t)((ch)+DeeUni_Descriptor(ch)->ut_upper))
#define DeeUni_ToTitle(ch)      ((uint32_t)((ch)+DeeUni_Descriptor(ch)->ut_title))
#define DeeUni_AsDigit(ch)      (DeeUni_Descriptor(ch)->ut_digit)
#define DeeUni_Convert(ch,kind) ((uint32_t)((ch)+*(int32_t *)((uintptr_t)DeeUni_Descriptor(ch)+(kind))))
#define UNICODE_CONVERT_LOWER     offsetof(struct unitraits,ut_lower)
#define UNICODE_CONVERT_UPPER     offsetof(struct unitraits,ut_upper)
#define UNICODE_CONVERT_TITLE     offsetof(struct unitraits,ut_title)
FORCELOCAL uint32_t DCALL DeeUni_SwapCase(uint32_t ch) {
 struct unitraits *record = DeeUni_Descriptor(ch);
 return (uint32_t)(ch+((record->ut_flags&UNICODE_FUPPER) ? record->ut_lower : record->ut_upper));
}

/* ctype-style character traits testing. */
#define DeeUni_IsAlpha(ch)   (DeeUni_Flags(ch)&UNICODE_FALPHA)
#define DeeUni_IsLower(ch)   (DeeUni_Flags(ch)&UNICODE_FLOWER)
#define DeeUni_IsUpper(ch)   (DeeUni_Flags(ch)&UNICODE_FUPPER)
#define DeeUni_IsAlnum(ch)   (DeeUni_Flags(ch)&(UNICODE_FALPHA|UNICODE_FDIGIT))
#define DeeUni_IsSpace(ch)   (DeeUni_Flags(ch)&UNICODE_FSPACE)
#define DeeUni_IsPrint(ch)   (DeeUni_Flags(ch)&UNICODE_FPRINT)
#define DeeUni_IsDigit(ch)   (DeeUni_Flags(ch)&UNICODE_FDIGIT)
#define DeeUni_IsDecimal(ch) (DeeUni_Flags(ch)&UNICODE_FDECIMAL)
#define DeeUni_IsNumeric(ch) (DeeUni_Flags(ch)&(UNICODE_FDIGIT|UNICODE_FDECIMAL))
#define DeeUni_IsTitle(ch)   (DeeUni_Flags(ch)&(UNICODE_FTITLE|UNICODE_FUPPER))
#define DeeUni_IsSymStrt(ch) (DeeUni_Flags(ch)&UNICODE_FSYMSTRT)
#define DeeUni_IsSymCont(ch) (DeeUni_Flags(ch)&UNICODE_FSYMCONT)

#ifdef CONFIG_BUILDING_DEEMON
INTDEF DREF DeeObject *DCALL DeeString_Convert(DeeObject *__restrict self, uintptr_t kind);
INTDEF DREF DeeObject *DCALL DeeString_ToTitle(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeString_Capitalize(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeString_Swapcase(DeeObject *__restrict self);
#define DeeString_ToLower(self) DeeString_Convert(self,UNICODE_CONVERT_LOWER)
#define DeeString_ToUpper(self) DeeString_Convert(self,UNICODE_CONVERT_UPPER)

/* Find a given sub-string, or return -1 if not found, or -2 if an error was thrown.
 * HINT: `end' is automatically reduced to `DeeString_UtfLen(self)' when greater. */
INTDEF dssize_t DCALL DeeString_Find(DeeObject *__restrict self, DeeObject *__restrict other, size_t begin, size_t end);
INTDEF dssize_t DCALL DeeString_RFind(DeeObject *__restrict self, DeeObject *__restrict other, size_t begin, size_t end);
INTDEF dssize_t DCALL DeeString_CaseFind(DeeObject *__restrict self, DeeObject *__restrict other, size_t begin, size_t end);
INTDEF dssize_t DCALL DeeString_CaseRFind(DeeObject *__restrict self, DeeObject *__restrict other, size_t begin, size_t end);

/* Count the number of occurrences of a given sub-string, or return -1 if an error was thrown.
 * HINT: `end' is automatically reduced to `DeeString_UtfLen(self)' when greater. */
INTDEF dssize_t DCALL DeeString_Count(DeeObject *__restrict self, DeeObject *__restrict other, size_t begin, size_t end);
INTDEF dssize_t DCALL DeeString_CaseCount(DeeObject *__restrict self, DeeObject *__restrict other, size_t begin, size_t end);

INTDEF DREF DeeObject *DCALL DeeString_Reversed(DeeObject *__restrict self, size_t begin, size_t end);
INTDEF DREF DeeObject *DCALL DeeString_ExpandTabs(DeeObject *__restrict self, size_t tab_width);
INTDEF DREF DeeObject *DCALL DeeString_Join(DeeObject *__restrict self, DeeObject *__restrict seq);
INTDEF DREF DeeObject *DCALL DeeString_UnifyLines(DeeObject *__restrict self, DeeObject *__restrict replacement);
INTDEF DREF DeeObject *DCALL DeeString_UnifyLinesLf(DeeObject *__restrict self);

/* Strip all space characters from the front and/or back of the given string. */
INTDEF DREF DeeObject *DCALL DeeString_StripSpc(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeString_LStripSpc(DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeString_RStripSpc(DeeObject *__restrict self);
/* Strip all leading and/or trailing characters apart of `mask' from `self' */
INTDEF DREF DeeObject *DCALL DeeString_StripMask(DeeObject *__restrict self, DeeObject *__restrict mask);
INTDEF DREF DeeObject *DCALL DeeString_LStripMask(DeeObject *__restrict self, DeeObject *__restrict mask);
INTDEF DREF DeeObject *DCALL DeeString_RStripMask(DeeObject *__restrict self, DeeObject *__restrict mask);
/* Strip all leading and/or trailing instances of `mask' from `self' */
INTDEF DREF DeeObject *DCALL DeeString_SStrip(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL DeeString_LSStrip(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL DeeString_RSStrip(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL DeeString_CaseSStrip(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL DeeString_CaseLSStrip(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL DeeString_CaseRSStrip(DeeObject *__restrict self, DeeObject *__restrict other);

/* @return: An abstract sequence type for enumerating the segments of a split string. */
INTDEF DREF DeeObject *DCALL DeeString_Split(DeeObject *__restrict self, DeeObject *__restrict seperator);
INTDEF DREF DeeObject *DCALL DeeString_CaseSplit(DeeObject *__restrict self, DeeObject *__restrict seperator);
/* @return: An abstract sequence type for enumerating the segments of a string split into lines. */
INTDEF DREF DeeObject *DCALL DeeString_SplitLines(DeeObject *__restrict self, bool keepends);

/* String in-/de-dentation (basically implementing what one would expect from a text
 * editor when pressing TAB or SHIFT+TAB whilst having multiple lines selected). */
INTDEF DREF DeeObject *DCALL DeeString_Indent(DeeObject *__restrict self, DeeObject *__restrict filler);
INTDEF DREF DeeObject *DCALL DeeString_Dedent(DeeObject *__restrict self, size_t max_chars, DeeObject *__restrict mask);
INTDEF DREF DeeObject *DCALL DeeString_DedentSpc(DeeObject *__restrict self, size_t max_chars);

/* Implement c-style string scanning, using a scanf()-style format string.
 * This functions then returns a sequence of all scanned objects, that is
 * the usually used in an expand expression:
 * >> for (local line: file.stdin) {
 * >>     local a,b,c;
 * >>     try {
 * >>         a,b,c = line.scanf("%s %s %s")...;
 * >>     } catch (...) { // Unpack errors.
 * >>         continue;
 * >>     }
 * >>     print "a:",a;
 * >>     print "b:",b;
 * >>     print "c:",c;
 * >> }
 */
INTDEF DREF DeeObject *DCALL DeeString_Scanf(DeeObject *__restrict self,
                                             DeeObject *__restrict format);

/* Format a given `format' string subject to printf-style formatting rules.
 * NOTE: This is the function called by `operator %' for strings. */
INTDEF DREF DeeObject *DCALL DeeString_CFormat(DeeObject *__restrict format,
                                               size_t argc, DeeObject **__restrict argv);

/* Format a given `format' string subject to {}-style formatting rules.
 * NOTE: This is the function called by `.format' for strings.
 * @param: args: A sequence (usually a dict, or a list) used for
 *               providing input values to the format string. */
INTDEF DREF DeeObject *DCALL DeeString_Format(DeeObject *__restrict format,
                                              DeeObject *__restrict args);

#endif

DECL_END
#endif /* !CONFIG_USE_NEW_STRING_API */

#endif /* !GUARD_DEEMON_STRING_H */
