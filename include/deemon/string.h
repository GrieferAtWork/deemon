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

#include "api.h"
#include "object.h"
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <hybrid/limits.h>

DECL_BEGIN

/* Explanation of WIDTH-STRINGS vs. UTF-STRINGS:
 *
 *  - 1-byte string: 100% the same as LATIN-1
 *    Only able to encode the unicode character range U+00-U+FF
 *    Generated and returned by `DeeString_Bytes()', which
 *    will encode characters outside that range as '?'
 *  - UTF-8 string:
 *    A multi-byte / variable-character-width string capable
 *    of encoding the entirety of the unicode range, as well
 *    as share binary compatibility to ASCII.
 *    For strings containing only characters <= 0x7F, this is
 *    identical to a 1-byte string.
 *
 *  - 2-byte string:
 *    Encodes all characters in the address range U+0000-U+FFFF.
 *    Unlike for 1-byte strings, there is no way to force this
 *    representation to be generated (although such a function
 *    could easily be added as an afterthought)
 *    Strings may indicate their width to be 2-bytes, even if
 *    all contained characters fit into the 1-byte range.
 *    In this case, `DeeString_Bytes()' still functions normally,
 *    and will succeed.
 *  - UTF-16 string:
 *    A 1/2-word / variable-character-width string that can encode
 *    the unicode range U+000000 - U+10FFFF. Characters outside that
 *    range cannot be represented, despite being able to be referred
 *    to by UTF-8
 *
 *  - 4-byte string:
 *    All characters are encoded in the address range U+00000000-U+FFFFFFFF.
 *    Strings may indicate their width to be 4-bytes, even if
 *    all contained characters fit into the 2-, or 1-byte range.
 *    In this case, `DeeString_Bytes()' still functions normally,
 *    and will succeed for 1-byte ranges.
 *  - UTF-32 string:
 *    Literally the same as a 4-byte string.
 *
 */



struct unicode_printer;
typedef struct string_object DeeStringObject;

#ifndef __SIZEOF_WCHAR_T__
#ifdef CONFIG_HOST_WINDOWS
#   define __SIZEOF_WCHAR_T__ 2
#else
#   define __SIZEOF_WCHAR_T__ 4
#endif
#endif

#if defined(_NATIVE_WCHAR_T_DEFINED) || \
    defined(_WCHAR_T_DEFINED) || \
    defined(__wchar_t_defined)
typedef wchar_t dwchar_t;
#elif __SIZEOF_WCHAR_T__ == 2
typedef uint16_t dwchar_t;
#else
typedef uint32_t dwchar_t;
#endif

union dcharptr {
    void     *ptr;
    uint8_t  *cp8;
    uint16_t *cp16;
    uint32_t *cp32;
    char     *cp_char;
    dwchar_t *cp_wchar;
};


#define STRING_WIDTH_1BYTE  0u /* All characters are within the range U+0000 - U+00FF (LATIN-1) */
#define STRING_WIDTH_2BYTE  1u /* All characters are within the range U+0000 - U+FFFF (BMP) */
#define STRING_WIDTH_4BYTE  2u /* All characters are within the range U+0000 - U+10FFFF (Full unicode; UTF-32) */
#define STRING_WIDTH_COUNT  3u /* the number of of known string width encodings. */


/* 
 *   00 | 00  -> 00 == a|b
 *   00 | 01  -> 01 == a|b
 *   00 | 10  -> 10 == a|b
 *   01 | 00  -> 01 == a|b
 *   01 | 01  -> 01 == a|b
 *   01 | 10  -> 10 == a|b & ~1
 *   10 | 00  -> 10 == a|b
 *   10 | 01  -> 10 == a|b & ~1
 *   10 | 10  -> 10 == a|b
 */
#ifndef __NO_XBLOCK
#define STRING_WIDTH_COMMON(x,y) \
    XBLOCK({ unsigned int _x=(x),_y=(y); XRETURN _x >= _y ? _x : _y; })
#define STRING_WIDTH_COMMON3(x,y,z) \
    XBLOCK({ unsigned int _x=(x),_y=(y),_z(z); XRETURN _x >= _y ? (_x >= _z ? _x : _z) : (_y >= _z ? _y : _z); })
#elif !defined(__NO_ATTR_FORCEINLINE) && (!defined(_MSC_VER) || defined(NDEBUG))
FORCELOCAL ATTR_CONST unsigned int DCALL
string_width_common(unsigned int x, unsigned int y) {
 return x >= y ? x : y;
}
FORCELOCAL ATTR_CONST unsigned int DCALL
string_width_common3(unsigned int x, unsigned int y, unsigned int z) {
 return x >= y ? (x >= z ? x : z) : (y >= z ? y : z);
}
#define STRING_WIDTH_COMMON(x,y)    string_width_common(x,y)
#define STRING_WIDTH_COMMON3(x,y,z) string_width_common3(x,y,z)
#else
#define STRING_WIDTH_COMMON(x,y)    ((x)>=(y)?(x):(y))
#define STRING_WIDTH_COMMON3(x,y,z) ((x)>=(y)?STRING_WIDTH_COMMON(x,z):STRING_WIDTH_COMMON(y,z))
#endif


/* Encoding error flags. */
#define STRING_ERROR_FNORMAL 0x0000 /* Normal string decoding. */
#define STRING_ERROR_FSTRICT 0x0000 /* Throw `Error.UnicodeError.UnicodeDecodeError' when something goes wrong. */
#define STRING_ERROR_FREPLAC 0x0001 /* Replace bad characters with `?'. */
#define STRING_ERROR_FIGNORE 0x0002 /* Ignore (truncate or drop) bad characters. */

/* Returns the length of a width-string (which is any string obtained from a `string' object) */
#define WSTR_LENGTH(x) (((size_t *)(x))[-1])

/* Given the character-width `width', the base address `base', and the
 * index `index', return the unicode character found at that location */
#define STRING_WIDTH_GETCHAR(width,base,index) \
   (likely((width) == STRING_WIDTH_1BYTE) ? (uint32_t)((uint8_t *)(base))[index] : \
          ((width) == STRING_WIDTH_2BYTE) ? (uint32_t)((uint16_t *)(base))[index] : \
                                                      ((uint32_t *)(base))[index])

/* Given the character-width `width', the base address `base', and the
 * index `index', set the unicode character found at that location to `value' */
#define STRING_WIDTH_SETCHAR(width,base,index,value) \
   (likely((width) == STRING_WIDTH_1BYTE) ? (void)(((uint8_t *)(base))[index] = (uint8_t)(value)) : \
          ((width) == STRING_WIDTH_2BYTE) ? (void)(((uint16_t *)(base))[index] = (uint16_t)(value)) : \
                                            (void)(((uint32_t *)(base))[index] = (uint32_t)(value)))

/* Return the width (in bytes) of a string width `width' */
#define STRING_SIZEOF_WIDTH(width)       ((size_t)1 << (width))
/* Return the result of `x * STRING_SIZEOF_WIDTH(width)' */
#define STRING_MUL_SIZEOF_WIDTH(x,width) ((size_t)(x) << (width))


#ifndef __NO_builtin_expect
#define SWITCH_SIZEOF_WIDTH(x) switch (__builtin_expect(x,STRING_WIDTH_1BYTE))
#else
#define SWITCH_SIZEOF_WIDTH(x) switch (x)
#endif

#ifndef __NO_builtin_unreachable
#define CASE_WIDTH_1BYTE       default: __builtin_unreachable(); \
                               case STRING_WIDTH_1BYTE
#else
#define CASE_WIDTH_1BYTE       default
#endif
#define CASE_WIDTH_2BYTE       case STRING_WIDTH_2BYTE
#define CASE_WIDTH_4BYTE       case STRING_WIDTH_4BYTE


#define STRING_UTF_FNORMAL 0x0000 /* Normal UTF flags */
#define STRING_UTF_FASCII  0x0001 /* FLAG: The string contains no character outside the ASCII range.
                                   * NOTE: This flag isn't required to be set, even if there are only ASCII characters. */
#define STRING_UTF_FINVBYT 0x0002 /* FLAG: The string in `u_data[STRING_WIDTH_1BYTE]' contains truncated characters (represented as `?') */

struct string_utf {
#if __SIZEOF_POINTER__ > 4
    uint32_t   u_width; /* [const] The minimum encoding size (One of `STRING_WIDTH_*').
                         *         Also used as index into the encoding-data vector below.
                         *         NOTE: The data-representation indexed by this is _always_ allocated! */
    uint32_t   u_flags; /* [const] UTF flags (Set of `STRING_UTF_F*') */
#else
    uint16_t   u_width; /* [const] The minimum encoding size (One of `STRING_WIDTH_*').
                         *         Also used as index into the encoding-data vector below.
                         *         NOTE: The data-representation indexed by this is _always_ allocated! */
    uint16_t   u_flags; /* [const] UTF flags (Set of `STRING_UTF_F*') */
#endif
    size_t    *u_data[STRING_WIDTH_COUNT]; /* [0..1][owned][lock(WRITE_ONCE)][*]
                                            * Multi-byte string variant.
                                            * Variant at indices `>= u_width' can always be accessed without
                                            * any problems, with the version at `u_width' even being guarantied
                                            * to be 1..1 (that version is returned by `DeeString_WSTR()').
                                            * Accessing lower-order variants required the string to be cast into
                                            * that width-class, with the result being that characters which don't fit
                                            * into that class are replaced by `?', potentially causing a DecodeError.
                                            * >> ASSERT(u_data[u_width] != NULL);
                                            * >> ASSERT(!u_data[STRING_WIDTH_1BYTE] || WSTR_LENGTH(u_data[u_width]) == WSTR_LENGTH(u_data[STRING_WIDTH_1BYTE]));
                                            * >> ASSERT(!u_data[STRING_WIDTH_2BYTE] || WSTR_LENGTH(u_data[u_width]) == WSTR_LENGTH(u_data[STRING_WIDTH_2BYTE]));
                                            * >> ASSERT(!u_data[STRING_WIDTH_4BYTE] || WSTR_LENGTH(u_data[u_width]) == WSTR_LENGTH(u_data[STRING_WIDTH_4BYTE]));
                                            * >> if (u_width == STRING_WIDTH_1BYTE) {
                                            * >>     ASSERT(u_data[STRING_WIDTH_1BYTE] == :s_str);
                                            * >>     ASSERT(!u_utf16 || u_utf16 == u_data[STRING_WIDTH_2BYTE]);
                                            * >> }
                                            * >> if (u_flags & STRING_UTF_FASCII)
                                            * >>     ASSERT(u_utf8 == :s_str);
                                            */
    char      *u_utf8;  /* [0..1][lock(WRITE_ONCE)][owned_if(!= :s_str)]
                         * A lazily allocated width-string (meaning you can use `WSTR_LENGTH' to
                         * determine its length), representing the UTF-8 variant of this string. */
#if __SIZEOF_WCHAR_T__ == 2
    dwchar_t  *u_utf16; /* [0..1][lock(WRITE_ONCE)][owned_if(!= u_data[STRING_WIDTH_2BYTE])]
                         * A lazily allocated width-string (meaning you can use `WSTR_LENGTH' to
                         * determine its length), representing the UTF-16 variant of this string. */
#else
    uint16_t  *u_utf16; /* [0..1][lock(WRITE_ONCE)][owned_if(!= u_data[STRING_WIDTH_2BYTE])]
                         * A lazily allocated width-string (meaning you can use `WSTR_LENGTH' to
                         * determine its length), representing the UTF-16 variant of this string. */
#endif
};
#define string_utf_fini(self,str) \
do{ unsigned int i; \
    for (i = 1; i < STRING_WIDTH_COUNT; ++i) \
        if ((self)->u_data[i]) \
            Dee_Free((self)->u_data[i]-1); \
    if ((self)->u_data[STRING_WIDTH_1BYTE] && \
        (self)->u_data[STRING_WIDTH_1BYTE] != (size_t *)DeeString_STR(str)) \
        Dee_Free(((size_t *)(self)->u_data[STRING_WIDTH_1BYTE])-1); \
    if ((self)->u_utf8 && (self)->u_utf8 != (char *)DeeString_STR(str) && \
        (self)->u_utf8 != (char *)(self)->u_data[STRING_WIDTH_1BYTE]) \
        Dee_Free(((size_t *)(self)->u_utf8)-1); \
    if ((self)->u_utf16 && (uint16_t *)(self)->u_utf16 != (uint16_t *)(self)->u_data[STRING_WIDTH_2BYTE]) \
        Dee_Free(((size_t *)(self)->u_utf16)-1); \
}__WHILE0


DFUNDEF struct string_utf *(DCALL string_utf_alloc)(void);
DFUNDEF void (DCALL string_utf_free)(struct string_utf *__restrict self);
#ifndef NDEBUG
DFUNDEF struct string_utf *(DCALL string_utf_alloc_d)(char const *file, int line);
#define string_utf_alloc() string_utf_alloc_d(__FILE__,__LINE__)
#endif


struct string_object {
    OBJECT_HEAD
    struct string_utf *s_data;      /* [0..1][owned][lock(WRITE_ONCE)]
                                     * Extended string data, as well as unicode information & lazily allocated caches. */
#define DEE_STRING_HASH_UNSET ((dhash_t)-1) /* A hash-representation of the string. */
    dhash_t            s_hash;      /* [valid_if(!= #define)][lock(WRITE_ONCE)] The string's hash. */
    size_t             s_len;       /* [const] The number of bytes found in the single-byte string text. */
    /*unsigned*/char   s_str[1024]; /* [const][s_len] The single-byte string text.
                                     * This string is either encoded as UTF-8, when the string's character
                                     * width isn't 1 byte, or encoded as LATIN-1, if all characters fit
                                     * within the unicode range U+0000 - U+00FF */
};

#define DEFINE_STRING(name,str) \
struct { \
    OBJECT_HEAD \
    struct string_utf *s_data; \
    dhash_t s_hash; \
    size_t s_len; \
    char s_str[sizeof(str)/sizeof(char)]; \
    struct string_utf s_utf; \
} name = { \
    OBJECT_HEAD_INIT(&DeeString_Type), \
    &name.s_utf, \
    DEE_STRING_HASH_UNSET, \
   (sizeof(str)/sizeof(char))-1, \
    str, \
    { \
        STRING_WIDTH_1BYTE, \
        STRING_UTF_FASCII, \
        { (size_t *)name.s_str }, \
        name.s_str \
    } \
}

#ifdef GUARD_DEEMON_OBJECTS_STRING_C
struct empty_string { OBJECT_HEAD struct string_utf *s_data; dhash_t s_hash; size_t s_len; char s_zero; };
DDATDEF struct empty_string DeeString_Empty;
#define Dee_EmptyString ((DeeObject *)&DeeString_Empty)
#else
DDATDEF DeeObject           DeeString_Empty;
#define Dee_EmptyString   (&DeeString_Empty)
#endif
#define return_empty_string  return_reference_(Dee_EmptyString)


DDATDEF DeeTypeObject DeeString_Type;
#define DeeString_Check(x)      DeeObject_InstanceOfExact(x,&DeeString_Type)
#define DeeString_CheckExact(x) DeeObject_InstanceOfExact(x,&DeeString_Type)


/* Return a pointer to the multi-byte encoded variant of the given string.
 * Characters <= 127 follow ASCII characters, while the meaning of characters
 * above this depends on `DeeString_WIDTH()':
 *   - STRING_WIDTH_1BYTE:
 *      - If there are no characters above 127, the string is pure ASCII
 *      - Otherwise, the string may have been created from raw, non-decoded
 *        data, and might potentially not even contain readable text.
 *      - Another possibility is that the string follows a host-specific
 *        ANSI character set, where characters above 127 have host-specific
 *        meaning.
 *     -> In either case, any character above 127 should be treated as part
 *        of the unicode character range U+0080...U+00FF, meaning that the
 *        string should behave like any ordinary unicode string.
 *   - STRING_WIDTH_2BYTE:
 *      - `DeeString_STR()' is the multi-byte, UTF-8 form of a UTF-16 string
 *        found in `x->s_data->u_data[STRING_WIDTH_2BYTE]', while `DeeString_SIZE()'
 *        refers to the number of bytes used by the multi-byte string.
 *   - STRING_WIDTH_4BYTE:
 *      - `DeeString_STR()' is the multi-byte, UTF-8 form of a UTF-32 string
 *        found in `x->s_data->u_data[STRING_WIDTH_4BYTE]', while `DeeString_SIZE()'
 *        refers to the number of bytes used by the multi-byte string.
 */
#define DeeString_STR(x)   (((DeeStringObject *)REQUIRES_OBJECT(x))->s_str)
#define DeeString_SIZE(x)  (((DeeStringObject *)REQUIRES_OBJECT(x))->s_len)
#define DeeString_END(x)   (((DeeStringObject *)REQUIRES_OBJECT(x))->s_str+((DeeStringObject *)(x))->s_len)


#define DeeString_STR8(x)  (((DeeStringObject *)REQUIRES_OBJECT(x))->s_data ? (uint8_t *)((DeeStringObject *)(x))->s_data->u_data[STRING_WIDTH_1BYTE] : (uint8_t *)((DeeStringObject *)(x))->s_str)
#define DeeString_STR16(x) ((uint16_t *)((DeeStringObject *)REQUIRES_OBJECT(x))->s_data->u_data[STRING_WIDTH_2BYTE])
#define DeeString_STR32(x) ((uint32_t *)((DeeStringObject *)REQUIRES_OBJECT(x))->s_data->u_data[STRING_WIDTH_4BYTE])
#define DeeString_LEN8(x)  (((DeeStringObject *)REQUIRES_OBJECT(x))->s_data ? WSTR_LENGTH(((DeeStringObject *)(x))->s_data->u_data[STRING_WIDTH_1BYTE]) : ((DeeStringObject *)(x))->s_len)
#define DeeString_LEN16(x)  WSTR_LENGTH(DeeString_STR16(x))
#define DeeString_LEN32(x)  WSTR_LENGTH(DeeString_STR32(x))

/* Returns true if `DeeString_STR()' is encoded in UTF-8 */
#define DeeString_STR_ISUTF8(x) \
      (((DeeStringObject *)self)->s_data && \
     ((((DeeStringObject *)self)->s_data->u_flags & STRING_UTF_FASCII) || \
       ((DeeStringObject *)self)->s_data->u_width != STRING_WIDTH_1BYTE))

/* Returns true if `DeeString_STR()' is encoded in LATIN1, or ASCII */
#define DeeString_STR_ISLATIN1(x) (!DeeString_STR_ISUTF8(x))


/* Check if the given string object `x' has its hash calculated. */
#define DeeString_HASHOK(x)  (((DeeStringObject *)REQUIRES_OBJECT(x))->s_hash != (dhash_t)-1)
#define DeeString_HASH(x)     ((DeeStringObject *)REQUIRES_OBJECT(x))->s_hash

/* Check if the given string object `x' is an (the) empty string. */
#define DeeString_IsEmpty(x)  (((DeeStringObject *)REQUIRES_OBJECT(x))->s_len == 0)


#define DeeString_EQUALS_ASCII(x,ascii_str) \
       (DeeString_SIZE(x) == COMPILER_STRLEN(ascii_str) && \
        memcmp(DeeString_STR(x),ascii_str,sizeof(ascii_str)-sizeof(char)) == 0)

/* Return the unicode character-width of characters found in the given string `x' */
#define DeeString_WIDTH(x) \
     (((DeeStringObject *)REQUIRES_OBJECT(x))->s_data ? \
      ((DeeStringObject *)REQUIRES_OBJECT(x))->s_data->u_width : \
        STRING_WIDTH_1BYTE)

#define DeeString_Is1Byte(x) (!((DeeStringObject *)REQUIRES_OBJECT(x))->s_data || ((DeeStringObject *)(x))->s_data->u_width == STRING_WIDTH_1BYTE)
#define DeeString_Is2Byte(x) (((DeeStringObject *)REQUIRES_OBJECT(x))->s_data && ((DeeStringObject *)(x))->s_data->u_width == STRING_WIDTH_2BYTE)
#define DeeString_Is4Byte(x) (((DeeStringObject *)REQUIRES_OBJECT(x))->s_data && ((DeeStringObject *)(x))->s_data->u_width == STRING_WIDTH_4BYTE)

/* Return a pointer to the unicode character-array for which all
 * characters can be fitted into the same number of bytes, the
 * amount of which can be determined by `WSTR_LENGTH(DeeString_WSTR(x))',
 * or `DeeString_WLEN(x)', with their individual size in bytes determinable
 * as `STRING_SIZEOF_WIDTH(DeeString_WSTR(x))'.
 * HINT: The string length returned by `operator size' is `DeeString_WLEN()' */
#define DeeString_WSTR(x)   _DeeString_WStr((DeeStringObject *)REQUIRES_OBJECT(x))
#define DeeString_WLEN(x)   _DeeString_WLen((DeeStringObject *)REQUIRES_OBJECT(x))
#define DeeString_WEND(x)   _DeeString_WEnd((DeeStringObject *)REQUIRES_OBJECT(x))
#define DeeString_WSIZ(x)   _DeeString_WSiz((DeeStringObject *)REQUIRES_OBJECT(x))

FORCELOCAL size_t *DCALL _DeeString_WStr(DeeStringObject *__restrict x) {
 if (x->s_data) return x->s_data->u_data[x->s_data->u_width];
 return (size_t *)x->s_str;
}
FORCELOCAL void *DCALL _DeeString_WEnd(DeeStringObject *__restrict x) {
 struct string_utf *utf;
 if ((utf = x->s_data) != NULL) {
  SWITCH_SIZEOF_WIDTH(utf->u_width) {
  {
   uint8_t *result;
  CASE_WIDTH_1BYTE:
   result = (uint8_t *)utf->u_data[STRING_WIDTH_1BYTE];
   return result + WSTR_LENGTH(result);
  }
  {
   uint16_t *result;
  CASE_WIDTH_2BYTE:
   result = (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE];
   return result + WSTR_LENGTH(result);
  }
  {
   uint32_t *result;
  CASE_WIDTH_4BYTE:
   result = (uint32_t *)utf->u_data[STRING_WIDTH_4BYTE];
   return result + WSTR_LENGTH(result);
  }
  }
 }
 return x->s_str + x->s_len;
}
FORCELOCAL size_t DCALL _DeeString_WLen(DeeStringObject *__restrict x) {
 if (x->s_data)
     return x->s_data->u_data[x->s_data->u_width][-1];
 return x->s_len;
}
FORCELOCAL size_t DCALL _DeeString_WSiz(DeeStringObject *__restrict x) {
 if (x->s_data)
     return STRING_MUL_SIZEOF_WIDTH(x->s_data->u_data[x->s_data->u_width][-1],
                                    x->s_data->u_width);
 return x->s_len;
}

#ifdef CONFIG_BUILDING_DEEMON
/* Return the hash of `self', or calculate it if it wasn't already. */
INTDEF dhash_t DCALL DeeString_Hash(DeeObject *__restrict self);
#else
#define DeeString_Hash(self)   DeeObject_Hash(self)
#endif
DFUNDEF dhash_t DCALL DeeString_HashCase(DeeObject *__restrict self);

/* Delete cached buffer encodings from a given 1-byte string. */
PUBLIC void DCALL DeeString_FreeWidth(DeeObject *__restrict self);

/* Return the given string's character as a byte-array.
 * Characters above 0xFF either cause `NULL' to be returned, alongside a
 * ValueError being thrown, or cause them to be replaced with '?'.
 * NOTE: This function differs from `DeeString_STR()' (or `DeeString_AsWidth(,STRING_WIDTH_1BYTE)'),
 *       which leaves the encoding of its return value undefined (as either LATIN-1, or UTF-8),
 *       and is the LATIN-1 counterpart of `DeeString_AsUtf8()', used to enforce a certain type of
 *       encoding.
 * @return: * :   The Bytes-data of the given string `self' (encoded as a width-string)
 *                NOTE: The length of this block also matches `DeeString_WLEN(self)'
 * @return: NULL: An error occurred. */
DFUNDEF uint8_t *DCALL DeeString_AsBytes(DeeObject *__restrict self, bool allow_invalid);
#define DeeString_AsLatin1(self,allow_invalid)  DeeString_AsBytes(self,allow_invalid)

/* Return the characters of the given string `self', encoded with a width of `width'
 * @param: width: One of `STRING_WIDTH_*'
 * @return: * :   A pointer to the first character matching `width'.
 *                The number of characters 
 * @return: NULL: An error occurred. */
DFUNDEF void *DCALL DeeString_AsWidth(DeeObject *__restrict self, unsigned int width);


/* Quickly access the 1,2 or 4-byte variants of a given string, allowing
 * for the assumption that all characters of the string are guarantied to
 * fit the requested amount of bytes. */
DFUNDEF uint16_t *(DCALL DeeString_As2Byte)(DeeObject *__restrict self);
DFUNDEF uint32_t *(DCALL DeeString_As4Byte)(DeeObject *__restrict self);
#ifdef __INTELLISENSE__
ATTR_RETNONNULL uint8_t *(DeeString_As1Byte)(DeeObject *__restrict self);
#endif

#ifdef __INTELLISENSE__
ATTR_RETNONNULL uint8_t *(DeeString_Get1Byte)(DeeObject *__restrict self);
ATTR_RETNONNULL uint16_t *(DeeString_Get2Byte)(DeeObject *__restrict self);
ATTR_RETNONNULL uint32_t *(DeeString_Get4Byte)(DeeObject *__restrict self);
#else
#define DeeString_Get1Byte(self) DeeString_As1Byte(self)
#define DeeString_Get2Byte(self) \
                (ASSERTF(((DeeStringObject *)(self))->s_data && \
                         ((DeeStringObject *)(self))->s_data->u_data[STRING_WIDTH_2BYTE], \
                           "The 2-byte variant hasn't been allocated"), \
                (uint16_t *)((DeeStringObject *)(self))->s_data->u_data[STRING_WIDTH_2BYTE])
#define DeeString_Get4Byte(self) \
                (ASSERTF(((DeeStringObject *)(self))->s_data && \
                         ((DeeStringObject *)(self))->s_data->u_data[STRING_WIDTH_4BYTE], \
                           "The 4-byte variant hasn't been allocated"), \
                (uint32_t *)((DeeStringObject *)(self))->s_data->u_data[STRING_WIDTH_4BYTE])
#define DeeString_As1Byte(self) \
        (ASSERTF(!((DeeStringObject *)(self))->s_data || \
                  ((DeeStringObject *)(self))->s_data->u_width == STRING_WIDTH_1BYTE, \
                   "The string is too large to be view its 1-byte variant"),\
        (uint8_t *)DeeString_STR(self))
#define DeeString_As2Byte(self) \
                (((DeeStringObject *)(self))->s_data && \
        (ASSERTF(((DeeStringObject *)(self))->s_data->u_width <= STRING_WIDTH_2BYTE, \
                   "The string is too large to be view its 2-byte variant"), \
                 ((DeeStringObject *)(self))->s_data->u_data[STRING_WIDTH_2BYTE]) ? \
     (uint16_t *)((DeeStringObject *)(self))->s_data->u_data[STRING_WIDTH_2BYTE] : \
                   DeeString_As2Byte(self))
#define DeeString_As4Byte(self) \
               (((DeeStringObject *)(self))->s_data && \
                ((DeeStringObject *)(self))->s_data->u_data[STRING_WIDTH_4BYTE] ? \
    (uint32_t *)((DeeStringObject *)(self))->s_data->u_data[STRING_WIDTH_4BYTE] : \
                  DeeString_As4Byte(self))
#endif


/* Return the UTF-8 variant of the given string.
 * The returned string can be used as a width-string, meaning that
 * upon success (return != NULL), you can determine its length by
 * using `WSTR_LENGTH(return)'.
 * @return: * :   A pointer to the UTF-8 variant-string of `self'
 * @return: NULL: An error occurred. */
DFUNDEF char *DCALL DeeString_AsUtf8(DeeObject *__restrict self);

/* Returns the UTF-16 variant of the given string (as a width-string). */
DFUNDEF uint16_t *DCALL DeeString_AsUtf16(DeeObject *__restrict self, unsigned int error_mode);

/* Returns the UTF-32 variant of the given string (as a width-string). */
#ifdef __INTELLISENSE__
uint32_t *DeeString_AsUtf32(DeeObject *__restrict self);
#else
#define DeeString_AsUtf32(self)  DeeString_As4Byte(self)
#endif

/* Returns the Wide-string variant of the given string (as a width-string). */
#ifdef __INTELLISENSE__
dwchar_t *DeeString_AsWide(DeeObject *__restrict self);
#elif __SIZEOF_WCHAR_T__ == 2
#define DeeString_AsWide(self) ((dwchar_t *)DeeString_AsUtf16(self,STRING_ERROR_FREPLAC))
#else
#define DeeString_AsWide(self) ((dwchar_t *)DeeString_AsUtf32(self))
#endif




/* ================================================================================= */
/*   STRING BUFFER API                                                               */
/* ================================================================================= */

/* Construct an uninitialized single-byte string,
 * capable of representing up to `num_bytes' bytes of text. */
DFUNDEF DREF DeeObject *DCALL DeeString_NewBuffer(size_t num_bytes);
/* Resize a single-byte string to have a length of `num_bytes' bytes.
 * You may pass `NULL' for `self', or a reference to `Dee_EmptyString'
 * in order to allocate and return a new buffer. */
DFUNDEF DREF DeeObject *DCALL DeeString_ResizeBuffer(DREF DeeObject *self, size_t num_bytes);
DFUNDEF DREF DeeObject *DCALL DeeString_TryResizeBuffer(DREF DeeObject *self, size_t num_bytes);

#ifdef CONFIG_BUILDING_DEEMON
/* Print the text of `self' to `printer', encoded as a UTF-8 string.
 * NOTE: If `printer' is `&unicode_printer_print', special optimization
 *       is done, meaning that this is the preferred method of printing
 *       an object to a unicode printer.
 * NOTE: This optimization is also done when `DeeObject_Print' is used. */
INTDEF dssize_t DCALL
DeeString_PrintUtf8(DeeObject *__restrict self,
                    dformatprinter printer,
                    void *arg);
#else
#define DeeString_PrintUtf8 DeeObject_Print
#endif

/* Print the escape-encoded variant of `self'
 * @param: flags: Set of `FORMAT_QUOTE_F*' (from <deemon/format.h>) */
DFUNDEF dssize_t DCALL
DeeString_PrintRepr(DeeObject *__restrict self,
                    dformatprinter printer,
                    void *arg, unsigned int flags);


/* Construct a string from the given escape-sequence.
 * This function automatically deals with escaped characters above
 * the single-byte range, and expects the input to be structured as
 * UTF-8 text.
 *  - Surrounding quotation marks should be stripped before calling this function.
 *  - Escaped linefeeds are implicitly parsed, too.
 * @param: error_mode: One of `STRING_ERROR_F*' */
DFUNDEF DREF DeeObject *DCALL
DeeString_FromBackslashEscaped(/*utf-8*/char const *__restrict start,
                               size_t length, unsigned int error_mode);
DFUNDEF int DCALL
DeeString_DecodeBackslashEscaped(struct unicode_printer *__restrict printer,
                                 /*utf-8*/char const *__restrict start,
                                 size_t length, unsigned int error_mode);


/* Allocate / pack UTF16, UTF32 and Wide-character strings from buffers. */
FORCELOCAL uint16_t *DCALL
DeeString_NewBuffer16(size_t num_chars) {
 size_t *result = (size_t *)Dee_Malloc(sizeof(size_t)+(num_chars+1)*2);
 if likely(result) *result++ = num_chars;
 return (uint16_t *)result;
}
FORCELOCAL uint32_t *DCALL
DeeString_NewBuffer32(size_t num_chars) {
 size_t *result = (size_t *)Dee_Malloc(sizeof(size_t)+(num_chars+1)*4);
 if likely(result) *result++ = num_chars;
 return (uint32_t *)result;
}
FORCELOCAL uint16_t *DCALL
DeeString_TryNewBuffer16(size_t num_chars) {
 size_t *result = (size_t *)Dee_TryMalloc(sizeof(size_t)+(num_chars+1)*2);
 if likely(result) *result++ = num_chars;
 return (uint16_t *)result;
}
FORCELOCAL uint32_t *DCALL
DeeString_TryNewBuffer32(size_t num_chars) {
 size_t *result = (size_t *)Dee_TryMalloc(sizeof(size_t)+(num_chars+1)*4);
 if likely(result) *result++ = num_chars;
 return (uint32_t *)result;
}
FORCELOCAL uint16_t *DCALL
DeeString_ResizeBuffer16(uint16_t *buffer, size_t num_chars) {
 size_t *result = buffer ? (size_t *)buffer-1 : NULL;
 result = (size_t *)Dee_Realloc(result,sizeof(size_t)+(num_chars+1)*2);
 if likely(result) *result++ = num_chars;
 return (uint16_t *)result;
}
FORCELOCAL uint32_t *DCALL
DeeString_ResizeBuffer32(uint32_t *buffer, size_t num_chars) {
 size_t *result = buffer ? (size_t *)buffer-1 : NULL;
 result = (size_t *)Dee_Realloc(result,sizeof(size_t)+(num_chars+1)*4);
 if likely(result) *result++ = num_chars;
 return (uint32_t *)result;
}
FORCELOCAL uint16_t *DCALL
DeeString_TryResizeBuffer16(uint16_t *buffer, size_t num_chars) {
 size_t *result = buffer ? (size_t *)buffer-1 : NULL;
 result = (size_t *)Dee_TryRealloc(result,sizeof(size_t)+(num_chars+1)*2);
 if likely(result) *result++ = num_chars;
 else if (num_chars < WSTR_LENGTH(buffer)) {
  WSTR_LENGTH(buffer) = num_chars;
  result = (size_t *)buffer;
 }
 return (uint16_t *)result;
}
FORCELOCAL uint32_t *DCALL
DeeString_TryResizeBuffer32(uint32_t *buffer, size_t num_chars) {
 size_t *result = buffer ? (size_t *)buffer-1 : NULL;
 result = (size_t *)Dee_TryRealloc(result,sizeof(size_t)+(num_chars+1)*4);
 if likely(result) *result++ = num_chars;
 else if (num_chars < WSTR_LENGTH(buffer)) {
  WSTR_LENGTH(buffer) = num_chars;
  result = (size_t *)buffer;
 }
 return (uint32_t *)result;
}
FORCELOCAL void DeeString_Free2ByteBuffer(uint16_t *buffer) {
 if (buffer) Dee_Free((size_t *)buffer-1);
}
#define DeeString_Free4ByteBuffer(buffer) DeeString_Free2ByteBuffer((uint16_t *)(buffer)) 
#define DeeString_FreeWideBuffer(buffer)  DeeString_Free2ByteBuffer((uint16_t *)(buffer))
DFUNDEF DREF DeeObject *DCALL DeeString_Pack2ByteBuffer(/*inherit(always)*/uint16_t *__restrict text);
DFUNDEF DREF DeeObject *DCALL DeeString_TryPack2ByteBuffer(/*inherit(on_success)*/uint16_t *__restrict text);
#define DeeString_Pack4ByteBuffer(text)    DeeString_PackUtf32Buffer(text,STRING_ERROR_FIGNORE)
#define DeeString_TryPack4ByteBuffer(text) DeeString_TryPackUtf32Buffer(text)

/* @param: error_mode: One of `STRING_ERROR_F*' */
DFUNDEF DREF DeeObject *DCALL DeeString_PackUtf16Buffer(/*inherit(always)*/uint16_t *__restrict text, unsigned int error_mode);
DFUNDEF DREF DeeObject *DCALL DeeString_PackUtf32Buffer(/*inherit(always)*/uint32_t *__restrict text, unsigned int error_mode);
DFUNDEF DREF DeeObject *DCALL DeeString_TryPackUtf32Buffer(/*inherit(on_success)*/uint32_t *__restrict text);

#ifdef __INTELLISENSE__
dwchar_t *DeeString_NewWideBuffer(size_t num_chars);
dwchar_t *DeeString_ResizeWideBuffer(dwchar_t *buffer, size_t num_chars);
dwchar_t *DeeString_TryResizeWideBuffer(dwchar_t *buffer, size_t num_chars);
DREF DeeObject *DeeString_PackWideBuffer(/*inherit(always)*/dwchar_t *__restrict text, unsigned int error_mode);
#elif __SIZEOF_WCHAR_T__ == 2
#define DeeString_NewWideBuffer(num_chars)              ((dwchar_t *)DeeString_NewBuffer16(num_chars))
#define DeeString_ResizeWideBuffer(buffer,num_chars)    ((dwchar_t *)DeeString_ResizeBuffer16((uint16_t *)(buffer),num_chars))
#define DeeString_TryResizeWideBuffer(buffer,num_chars) ((dwchar_t *)DeeString_TryResizeBuffer16((uint16_t *)(buffer),num_chars))
#define DeeString_PackWideBuffer(buf,error_mode)          DeeString_PackUtf16Buffer((uint16_t *)(buf),error_mode)
#else
#define DeeString_NewWideBuffer(num_chars)              ((dwchar_t *)DeeString_NewBuffer32(num_chars))
#define DeeString_ResizeWideBuffer(buffer,num_chars)    ((dwchar_t *)DeeString_ResizeBuffer32((uint32_t *)(buffer),num_chars))
#define DeeString_TryResizeWideBuffer(buffer,num_chars) ((dwchar_t *)DeeString_TryResizeBuffer32((uint32_t *)(buffer),num_chars))
#define DeeString_PackWideBuffer(buf,error_mode)          DeeString_PackUtf32Buffer((uint32_t *)(buf),error_mode)
#endif

FORCELOCAL void *DCALL
DeeString_NewWidthBuffer(size_t num_chars, unsigned int width) {
 if (width == STRING_WIDTH_1BYTE) {
  DeeStringObject *result;
  result = (DeeStringObject *)DeeObject_Malloc(COMPILER_OFFSETOF(DeeStringObject,s_str)+
                                              (num_chars+1)*sizeof(char));
  if likely(result) result->s_len = num_chars;
  return result->s_str;
 } else {
  size_t *result;
  result = (size_t *)Dee_Malloc(sizeof(size_t)+(num_chars+1)*
                                STRING_SIZEOF_WIDTH(width));
  if likely(result) *result++ = num_chars;
  return result;
 }
}
FORCELOCAL void *DCALL
DeeString_ResizeWidthBuffer(void *buffer, size_t num_chars, unsigned int width) {
 if (width == STRING_WIDTH_1BYTE) {
  DeeStringObject *result;
  result = buffer ? COMPILER_CONTAINER_OF((char *)buffer,DeeStringObject,s_str) : NULL;
  result = (DeeStringObject *)DeeObject_Realloc(result,
                                                COMPILER_OFFSETOF(DeeStringObject,s_str)+
                                               (num_chars+1)*sizeof(char));
  if likely(result) result->s_len = num_chars;
  return result->s_str;
 } else {
  size_t *result;
  result = buffer ? (size_t *)buffer-1 : NULL;
  result = (size_t *)Dee_Realloc(result,sizeof(size_t)+(num_chars+1)*
                                 STRING_SIZEOF_WIDTH(width));
  if likely(result) *result++ = num_chars;
  return result;
 }
}
FORCELOCAL void *DCALL
DeeString_TryResizeWidthBuffer(void *buffer, size_t num_chars, unsigned int width) {
 if (width == STRING_WIDTH_1BYTE) {
  DeeStringObject *result;
  result = buffer ? COMPILER_CONTAINER_OF((char *)buffer,DeeStringObject,s_str) : NULL;
  result = (DeeStringObject *)DeeObject_TryRealloc(result,
                                                   COMPILER_OFFSETOF(DeeStringObject,s_str)+
                                                  (num_chars+1)*sizeof(char));
  if likely(result) result->s_len = num_chars;
  return result->s_str;
 } else {
  size_t *result;
  result = buffer ? (size_t *)buffer-1 : NULL;
  result = (size_t *)Dee_TryRealloc(result,sizeof(size_t)+(num_chars+1)*
                                    STRING_SIZEOF_WIDTH(width));
  if likely(result) *result++ = num_chars;
  return result;
 }
}
/* @param: error_mode: One of `STRING_ERROR_F*' (Usually `STRING_ERROR_FIGNORE') */
FORCELOCAL DREF DeeObject *DCALL
DeeString_PackWidthBuffer(/*inherit(always)*/void *buffer,
                          unsigned int width) {
 SWITCH_SIZEOF_WIDTH(width) {
 {
  DREF DeeStringObject *result;
 CASE_WIDTH_1BYTE:
  result = COMPILER_CONTAINER_OF((char *)buffer,DeeStringObject,s_str);
  result->s_data = NULL;
  result->s_hash = DEE_STRING_HASH_UNSET;
  result->s_str[result->s_len] = '\0';
  DeeObject_Init(result,&DeeString_Type);
  return (DREF DeeObject *)result;
 } break;

 CASE_WIDTH_2BYTE:
  return DeeString_Pack2ByteBuffer((uint16_t *)buffer);
 CASE_WIDTH_4BYTE:
  return DeeString_Pack4ByteBuffer((uint32_t *)buffer);
 }
}
FORCELOCAL DREF DeeObject *DCALL
DeeString_TryPackWidthBuffer(/*inherit(on_success)*/void *buffer,
                             unsigned int width) {
 SWITCH_SIZEOF_WIDTH(width) {
 {
  DREF DeeStringObject *result;
 CASE_WIDTH_1BYTE:
  result = COMPILER_CONTAINER_OF((char *)buffer,DeeStringObject,s_str);
  result->s_data = NULL;
  result->s_hash = DEE_STRING_HASH_UNSET;
  result->s_str[result->s_len] = '\0';
  DeeObject_Init(result,&DeeString_Type);
  return (DREF DeeObject *)result;
 } break;

 CASE_WIDTH_2BYTE:
  return DeeString_TryPack2ByteBuffer((uint16_t *)buffer);
 CASE_WIDTH_4BYTE:
  return DeeString_TryPack4ByteBuffer((uint32_t *)buffer);
 }
}
FORCELOCAL void DCALL
DeeString_FreeWidthBuffer(void *buffer, unsigned int width) {
 if (buffer) {
  if (width == STRING_WIDTH_1BYTE) {
   DeeObject_Free(COMPILER_CONTAINER_OF((char *)buffer,DeeStringObject,s_str));
  } else {
   Dee_Free((size_t *)buffer-1);
  }
 }
}




/* Construct a new, non-decoded single-byte-per-character string `str'.
 * The string itself may contain characters above 127, which are then
 * interpreted as part of the unicode character-range U+0080...U+00FF. */
DFUNDEF DREF DeeObject *DCALL DeeString_New(/*unsigned*/char const *__restrict str);
#define DeeString_NewWithHash(str,hash) DeeString_New(str) /* XXX: Take advantage of this? */


/* Check if `str' is the `DeeString_STR()' of a string object, and return
 * a pointer to that object, only if it being that can be guarantied.
 * Otherwise return `NULL'.
 * WARNING: In order to prevent race conditions, only use this function
 *          with statically allocated strings, as a heap implementation
 *          that doesn't fill free memory and uses a block-size field in
 *          an unfortunate location could falsely trigger a match. */
#ifdef PAGESIZE
LOCAL DeeObject *DCALL
DeeString_IsObject(/*unsigned*/char const *__restrict str) {
 DeeStringObject *base;
 base = COMPILER_CONTAINER_OF(str,DeeStringObject,s_str);
 /* Check if the string object base would be part of the same page
  * as the string pointer we were given. - Only if it is, we can
  * safely access the supposed string object to check if it matches
  * what we'd expect of a string instance. */
 if (((uintptr_t)base & ~(PAGESIZE-1)) ==
     ((uintptr_t)str & ~(PAGESIZE-1))) {
  /* Most important check: Does the object type indicate that it's a string. */
  if (base->ob_type == &DeeString_Type) {
   /* Check that the object's reference counter is non-zero.
    * The combination of these 2 checks can verify a string object
    * allowed by any kind of allocator, regardless of what may be
    * done to freed objects:
    * #1: If the heap does some debug stuff to memset() free memory,
    *     and the given `str' is allocated within such a region, ob_type
    *     would not be a pointer to `DeeString_Type'
    * #2: If the heap doesn't do debug-memset()-stuff, and we were unlucky
    *     enough to be given an `str' allocated at the exact location where
    *     then `ob_refcnt' would still be zero.
    * #3: UNLUCKY: Same as in #2, but `ob_refcnt' is the heap-size field,
    *              in which case we'd get an invalid match by assuming
    *              that the heap-size field was our reference counter.
    * -> Because of case #3, this function can't be used for heap-allocated strings. */
   if (base->ob_refcnt != 0)
       return (DeeObject *)base;
  }
 }
 return NULL;
}
LOCAL DREF DeeObject *DCALL
DeeString_NewAuto(/*unsigned*/char const *__restrict str) {
 DeeObject *result;
 result = DeeString_IsObject(str);
 if (result)
  Dee_Incref(result);
 else {
  result = DeeString_New(str);
 }
 return result;
}
LOCAL DREF DeeObject *DCALL
DeeString_NewAutoWithHash(/*unsigned*/char const *__restrict str, dhash_t hash) {
 DeeObject *result;
 result = DeeString_IsObject(str);
 if (result) {
  dhash_t str_hash = ((DeeStringObject *)result)->s_hash;
  if (str_hash != hash) {
   if (str_hash != DEE_STRING_HASH_UNSET)
       goto return_new_string;
   ((DeeStringObject *)result)->s_hash = hash;
  }
  Dee_Incref(result);
 } else {
return_new_string:
  result = DeeString_NewWithHash(str,hash);
 }
 return result;
}
#else
#define DeeString_IsObject(str)            ((DeeObject *)NULL)
#define DeeString_NewAuto(str)               DeeString_New(str)
#define DeeString_NewAutoWithHash(str,hash)  DeeString_NewWithHash(str,hash)
#endif

/* Construct a new string using printf-like (and deemon-enhanced) format-arguments. */
DFUNDEF DREF DeeObject *DeeString_Newf(/*utf-8*/char const *__restrict format, ...);
DFUNDEF DREF DeeObject *DCALL DeeString_VNewf(/*utf-8*/char const *__restrict format, va_list args);

/* Construct strings with basic width-data. */
DFUNDEF DREF DeeObject *DCALL DeeString_NewSized(/*unsigned*/char const *__restrict str, size_t length);
DFUNDEF DREF DeeObject *DCALL DeeString_New2Byte(uint16_t const *__restrict str, size_t length);
DFUNDEF DREF DeeObject *DCALL DeeString_New4Byte(uint32_t const *__restrict str, size_t length);

#ifdef __INTELLISENSE__
DREF DeeObject *DeeString_New1Byte(uint8_t const *__restrict str, size_t length);
#else
#define DeeString_New1Byte(str,length) DeeString_NewSized((char const *)(str),length)
#endif


/* Construct a string from a UTF-8 character sequence. */
DFUNDEF DREF DeeObject *DCALL DeeString_NewUtf8(/*utf-8*/char const *__restrict str, size_t length,
                                                unsigned int error_mode);

/* Construct strings from UTF-16/32 encoded content.
 * @param: error_mode: One of `STRING_ERROR_F*' */
DFUNDEF DREF DeeObject *DCALL DeeString_NewUtf16(uint16_t const *__restrict str, size_t length, unsigned int error_mode);
DFUNDEF DREF DeeObject *DCALL DeeString_NewUtf32(uint32_t const *__restrict str, size_t length, unsigned int error_mode);
DFUNDEF DREF DeeObject *DCALL DeeString_NewUtf16AltEndian(uint16_t const *__restrict str, size_t length, unsigned int error_mode);
DFUNDEF DREF DeeObject *DCALL DeeString_NewUtf32AltEndian(uint32_t const *__restrict str, size_t length, unsigned int error_mode);

#ifdef __INTELLISENSE__
DREF DeeObject *DeeString_NewWide(dwchar_t const *__restrict str, size_t length, unsigned int error_mode);
DREF DeeObject *DeeString_NewWideAltEndian(dwchar_t const *__restrict str, size_t length, unsigned int error_mode);
#elif __SIZEOF_WCHAR_T__ == 2
#define DeeString_NewWide(str,length,error_mode)          DeeString_NewUtf16((uint16_t *)(str),length,error_mode)
#define DeeString_NewWideAltEndian(str,length,error_mode) DeeString_NewUtf16AltEndian((uint16_t *)(str),length,error_mode)
#else
#define DeeString_NewWide(str,length,error_mode)          DeeString_NewUtf32((uint16_t *)(str),length,error_mode)
#define DeeString_NewWideAltEndian(str,length,error_mode) DeeString_NewUtf32AltEndian((uint16_t *)(str),length,error_mode)
#endif

#ifdef CONFIG_LITTLE_ENDIAN
#define DeeString_NewUtf16Le  DeeString_NewUtf16
#define DeeString_NewUtf32Le  DeeString_NewUtf32
#define DeeString_NewUtf16Be  DeeString_NewUtf16AltEndian
#define DeeString_NewUtf32Be  DeeString_NewUtf32AltEndian
#define DeeString_NewWideLe   DeeString_NewWide
#define DeeString_NewWideBe   DeeString_NewWideAltEndian
#else
#define DeeString_NewUtf16Le  DeeString_NewUtf16AltEndian
#define DeeString_NewUtf32Le  DeeString_NewUtf32AltEndian
#define DeeString_NewUtf16Be  DeeString_NewUtf16
#define DeeString_NewUtf32Be  DeeString_NewUtf32
#define DeeString_NewWideLe   DeeString_NewWideAltEndian
#define DeeString_NewWideBe   DeeString_NewWide
#endif


FORCELOCAL DREF DeeObject *DCALL
DeeString_NewWithWidth(void const *__restrict str,
                       size_t length, unsigned int width) {
 SWITCH_SIZEOF_WIDTH(width) {
 CASE_WIDTH_1BYTE: return DeeString_NewSized((char *)str,length);
 CASE_WIDTH_2BYTE: return DeeString_New2Byte((uint16_t *)str,length);
 CASE_WIDTH_4BYTE: return DeeString_New4Byte((uint32_t *)str,length);
 }
}



/* Return a string containing a single character. */
#if defined(__INTELLISENSE__) && defined(__cplusplus)
extern "C++" {
DREF DeeObject *DeeString_Chr(uint8_t ch);
DREF DeeObject *DeeString_Chr(uint16_t ch);
DREF DeeObject *DeeString_Chr(uint32_t ch);
}
#else
#define DeeString_Chr(ch) \
       (sizeof(ch) == 1 ? _DeeString_Chr8((uint8_t)(ch)) : \
        sizeof(ch) == 2 ? _DeeString_Chr16((uint16_t)(ch)) : \
                          _DeeString_Chr32((uint32_t)(ch)))
DFUNDEF DREF DeeObject *DCALL _DeeString_Chr8(uint8_t ch);
DFUNDEF DREF DeeObject *DCALL _DeeString_Chr16(uint16_t ch);
DFUNDEF DREF DeeObject *DCALL _DeeString_Chr32(uint32_t ch);
#endif




#define UNICODE_FPRINT   0x0001 /* The character is printable, or SPC (` '). */
#define UNICODE_FALPHA   0x0002 /* The character is alphabetic. */
#define UNICODE_FSPACE   0x0004 /* The character is a space-character. */
#define UNICODE_FLF      0x0008 /* Line-feed/line-break character. */
#define UNICODE_FLOWER   0x0010 /* Lower-case. */
#define UNICODE_FUPPER   0x0020 /* Upper-case. */
#define UNICODE_FTITLE   0x0040 /* Title-case. */
#define UNICODE_FCNTRL   0x0080 /* Control character. */
#define UNICODE_FDIGIT   0x0100 /* The character is a digit. e.g.: `2' (ascii; `ut_digit' is `2') */
#define UNICODE_FDECIMAL 0x0200 /* The character is a decimal. e.g: `²' (sqare; `ut_digit' is `2') */
#define UNICODE_FSYMSTRT 0x0400 /* The character can be used as the start of an identifier. */
#define UNICODE_FSYMCONT 0x0800 /* The character can be used to continue an identifier. */
/*      UNICODE_F        0x1000 */
/*      UNICODE_F        0x2000 */
/*      UNICODE_F        0x4000 */
/*      UNICODE_F        0x8000 */
typedef uint16_t uniflag_t;
/* Character flags for the first 256 unicode characters
 * -> Using this, we can greatly optimize unicode database
 *    lookups by checking the size of a character at compile-
 *    time, meaning that when working with 8-bit characters,
 *    no call to `DeeUni_Descriptor()' needs to be assembled. */
DDATDEF uniflag_t const DeeAscii_Flags[256];

#define DeeAscii_IsUpper(ch)  (DeeAscii_Flags[(uint8_t)(ch)]&UNICODE_FUPPER)
#define DeeAscii_IsLower(ch)  (DeeAscii_Flags[(uint8_t)(ch)]&UNICODE_FLOWER)
#define DeeAscii_ToLower(ch)  (DeeAscii_IsUpper(ch) ? (uint8_t)(ch)+0x20 : (uint8_t)(ch))
#define DeeAscii_ToUpper(ch)  (DeeAscii_IsLower(ch) ? (uint8_t)(ch)-0x20 : (uint8_t)(ch))
#define DeeAscii_ToTitle(ch)  (DeeAscii_IsLower(ch) ? (uint8_t)(ch)-0x20 : (uint8_t)(ch))
#define DeeAscii_AsDigit(ch)  ((uint8_t)(ch)-0x30)

struct unitraits {
    uniflag_t const ut_flags; /* Character flags (Set of `UNICODE_F*') */
    uint8_t   const ut_digit; /* Digit/decimal value (`DeeUni_IsNumeric'), or 0. */
    uint8_t   const ut_fold;  /* Unicode fold extension index, or `0xff'. */
    int32_t   const ut_lower; /* Delta added to the character to convert it to lowercase, or 0. */
    int32_t   const ut_upper; /* Delta added to the character to convert it to uppercase, or 0. */
    int32_t   const ut_title; /* Delta added to the character to convert it to titlecase, or 0. */
};

/* Unicode character traits database access. */
DFUNDEF ATTR_RETNONNULL ATTR_CONST struct unitraits *(DCALL DeeUni_Descriptor)(uint32_t ch);

#define UNICODE_FOLDED_MAX 3

/* case-fold the given unicode character `ch', and
 * return the number of resulting folded characters.
 * @assume(return >= 1 && return <= UNICODE_FOLDED_MAX); */
DFUNDEF ATTR_PURE size_t (DCALL DeeUni_ToFolded)(uint32_t ch, uint32_t buf[UNICODE_FOLDED_MAX]);


#if 0
#define DeeUni_Flags(ch)        (DeeUni_Descriptor(ch)->ut_flags)
#else
#define DeeUni_Flags(ch)        (sizeof(ch) == 1 ? DeeAscii_Flags[(uint8_t)(ch)] : DeeUni_Descriptor(ch)->ut_flags)
#endif

/* ctype-style character conversion. */
#define DeeUni_ToLower(ch)      (sizeof(ch) == 1 ? (uint32_t)DeeAscii_ToLower(ch) : (uint32_t)((ch)+DeeUni_Descriptor(ch)->ut_lower))
#define DeeUni_ToUpper(ch)      (sizeof(ch) == 1 ? (uint32_t)DeeAscii_ToUpper(ch) : (uint32_t)((ch)+DeeUni_Descriptor(ch)->ut_upper))
#define DeeUni_ToTitle(ch)      (sizeof(ch) == 1 ? (uint32_t)DeeAscii_ToTitle(ch) : (uint32_t)((ch)+DeeUni_Descriptor(ch)->ut_title))
#define DeeUni_AsDigit(ch)      (sizeof(ch) == 1 ? DeeAscii_AsDigit(ch) : DeeUni_Descriptor(ch)->ut_digit)
#define DeeUni_Convert(ch,kind) ((uint32_t)((ch)+*(int32_t *)((uintptr_t)DeeUni_Descriptor(ch)+(kind))))
#define UNICODE_CONVERT_LOWER     offsetof(struct unitraits,ut_lower)
#define UNICODE_CONVERT_UPPER     offsetof(struct unitraits,ut_upper)
#define UNICODE_CONVERT_TITLE     offsetof(struct unitraits,ut_title)

#define DeeUni_SwapCase(ch) \
       (sizeof(ch) == 1 ? (uint32_t)_DeeUni_SwapCase8((uint8_t)(ch)) \
                        : _DeeUni_SwapCase(ch))
FORCELOCAL uint32_t DCALL _DeeUni_SwapCase(uint32_t ch) {
 struct unitraits *record = DeeUni_Descriptor(ch);
 return (uint32_t)(ch+((record->ut_flags&UNICODE_FUPPER) ? record->ut_lower : record->ut_upper));
}
FORCELOCAL uint8_t DCALL _DeeUni_SwapCase8(uint8_t ch) {
 if (ch >= 0x41 && ch <= 0x5a) return ch + 0x20;
 if (ch >= 0x61 && ch <= 0x7a) return ch - 0x20;
 return ch;
}

/* ctype-style character traits testing. */
#define DeeUni_IsAlpha(ch)    (DeeUni_Flags(ch)&UNICODE_FALPHA)
#define DeeUni_IsLower(ch)    (DeeUni_Flags(ch)&UNICODE_FLOWER)
#define DeeUni_IsUpper(ch)    (DeeUni_Flags(ch)&UNICODE_FUPPER)
#define DeeUni_IsAlnum(ch)    (DeeUni_Flags(ch)&(UNICODE_FALPHA|UNICODE_FDIGIT))
#define DeeUni_IsSpace(ch)    (DeeUni_Flags(ch)&UNICODE_FSPACE)
#define DeeUni_IsTab(ch)      ((ch) == 9)
#define DeeUni_IsLF(ch)       (DeeUni_Flags(ch)&UNICODE_FLF)
#define DeeUni_IsPrint(ch)    (DeeUni_Flags(ch)&UNICODE_FPRINT)
#define DeeUni_IsDigit(ch)    (DeeUni_Flags(ch)&UNICODE_FDIGIT)
#define DeeUni_IsDecimal(ch)  (DeeUni_Flags(ch)&UNICODE_FDECIMAL)
#define DeeUni_IsNumeric(ch)  (DeeUni_Flags(ch)&(UNICODE_FDIGIT|UNICODE_FDECIMAL))
#define DeeUni_IsTitle(ch)    (DeeUni_Flags(ch)&(UNICODE_FTITLE|UNICODE_FUPPER))
#define DeeUni_IsSymStrt(ch)  (DeeUni_Flags(ch)&UNICODE_FSYMSTRT)
#define DeeUni_IsSymCont(ch)  (DeeUni_Flags(ch)&UNICODE_FSYMCONT)
#define DeeUni_IsDigitX(ch,x) \
       (sizeof(ch) == 1 ? ((uint8_t)(ch) == (uint8_t)('0'+(x))) : \
                          ((ch) == '0'+(x) || _DeeUni_IsDigitX(ch,x)))
FORCELOCAL bool DCALL _DeeUni_IsDigitX(uint32_t ch, uint8_t x) {
 struct unitraits *record = DeeUni_Descriptor(ch);
 return (record->ut_flags & UNICODE_FDIGIT) && record->ut_digit == x;
}


/* ================================================================================= */
/*   ASCII / LATIN-1 PRINTER API (Formerly `string_printer')                         */
/*   Superseded by `unicode_printer'                                                 */
/* ================================================================================= */
struct ascii_printer {
    size_t           ap_length; /* Used string length. */
    DeeStringObject *ap_string; /* [0..1][owned] Generated string object. */
};
#define ASCII_PRINTER_INIT  {0,NULL}
#define ascii_printer_init(self) \
 (void)((self)->ap_length = 0,(self)->ap_string = NULL)
#define ascii_printer_fini(self) \
        DeeObject_Free((self)->ap_string)
#define ASCII_PRINTER_STR(self) ((self)->ap_string->s_str)
#define ASCII_PRINTER_LEN(self) ((self)->ap_length)

/* Append the given data to a string printer. (HINT: Use this one as a `dformatprinter') */
DFUNDEF dssize_t DCALL ascii_printer_print(struct ascii_printer *__restrict self, char const *__restrict data, size_t datalen);

DFUNDEF char *DCALL ascii_printer_alloc(struct ascii_printer *__restrict self, size_t datalen);
/* Release the last `datalen' bytes from the printer to be
 * re-used in subsequent calls, or be truncated eventually. */
DFUNDEF void DCALL ascii_printer_release(struct ascii_printer *__restrict self, size_t datalen);

#ifdef __INTELLISENSE__
dssize_t ascii_printer_printf(struct ascii_printer *__restrict self, char const *__restrict format, ...);
dssize_t ascii_printer_vprintf(struct ascii_printer *__restrict self, char const *__restrict format, va_list args);
#define ASCII_PRINTER_PRINT(self,S)                ascii_printer_print(self,S,COMPILER_STRLEN(S))
#else
#define ASCII_PRINTER_PRINT(self,S)                ascii_printer_print(self,S,COMPILER_STRLEN(S))
#define ascii_printer_printf(self,...)             Dee_FormatPrintf((dformatprinter)&ascii_printer_print,self,__VA_ARGS__)
#define ascii_printer_vprintf(self,format,args)    Dee_VFormatPrintf((dformatprinter)&ascii_printer_print,self,format,args)
#endif

/* Print a single character, returning -1 on error or 0 on success. */
DFUNDEF int DCALL ascii_printer_putc(struct ascii_printer *__restrict self, char ch);
/* Search the buffer that has already been created for an existing instance
 * of `str...+=length' and if found, return a pointer to its location.
 * Otherwise, append the given string and return a pointer to that location.
 * Upon error (append failed to allocate more memory), NULL is returned.
 * HINT: This function is very useful when creating
 *       string tables for NUL-terminated strings:
 *       >> ascii_printer_allocstr("foobar\0"); // Table is now `foobar\0'
 *       >> ascii_printer_allocstr("foo\0");    // Table is now `foobar\0foo\0'
 *       >> ascii_printer_allocstr("bar\0");    // Table is still `foobar\0foo\0' - `bar\0' points into `foobar\0'
 */
DFUNDEF char *DCALL
ascii_printer_allocstr(struct ascii_printer *__restrict self,
                       char const *__restrict str, size_t length);

/* Pack together data from a string printer and return the generated contained string.
 * Upon success, as well as upon failure, the state of `self' is undefined upon return. */
DFUNDEF DREF DeeObject *DCALL ascii_printer_pack(/*inherit(always)*/struct ascii_printer *__restrict self);

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define ascii_printer_putc(self,ch) __builtin_expect(ascii_printer_putc(self,ch),0)
#endif
#endif




/* ================================================================================= */
/*   UNICODE PRINTER API                                                             */
/* ================================================================================= */
struct unicode_printer {
    size_t        up_length; /* The length (in characters) of the string printed thus far. */
#if 1
    union {
        void     *up_buffer; /* [0..1][owned(DeeString_FreeWidthBuffer)] The current width-buffer. */
        char     *_up_str;   /* Only here, show the printer's string can be viewed in a debugger. */
        dwchar_t *_up_wstr;  /* Only here, show the printer's string can be viewed in a debugger. */
    };
#else
    void         *up_buffer; /* [0..1][owned(DeeString_FreeWidthBuffer)] The current width-buffer. */
#endif
#define UNICODE_PRINTER_FWIDTH   0x0f /* Mask of the string-width produced by this printer. */
#define UNICODE_PRINTER_FPENDING 0xf0 /* Mask for the number of pending UTF-8 characters. */
#define UNICODE_PRINTER_FPENDING_SHFT 4 /* Shift for the number of pending UTF-8 characters. */
    unsigned char up_flags;  /* The currently required max-width of the resulting string.  */
    unsigned char up_pend[7];/* Pending UTF-8 characters. */
};
#define UNICODE_PRINTER_INIT { 0, { NULL }, STRING_WIDTH_1BYTE }

#ifdef __INTELLISENSE__
void unicode_printer_init(struct unicode_printer *__restrict self);
void unicode_printer_fini(struct unicode_printer *__restrict self);
#else
#define unicode_printer_init(self) ((self)->up_length = 0,(self)->up_buffer = NULL,(self)->up_flags = STRING_WIDTH_1BYTE)
#define unicode_printer_fini(self) DeeString_FreeWidthBuffer((self)->up_buffer,UNICODE_PRINTER_WIDTH(self))
#endif

#define UNICODE_PRINTER_LENGTH(x)      ((x)->up_length) /* Used length */
#define UNICODE_PRINTER_BUFSIZE(x)     ((x)->up_buffer ? WSTR_LENGTH((x)->up_buffer) : NULL) /* Allocated length */
#define UNICODE_PRINTER_WIDTH(x)       ((x)->up_flags&UNICODE_PRINTER_FWIDTH) /* Current width */
#define UNICODE_PRINTER_GETCHAR(x,i)    STRING_WIDTH_GETCHAR(UNICODE_PRINTER_WIDTH(x),(x)->up_buffer,i) /* Get a character */
#define UNICODE_PRINTER_SETCHAR(x,i,v)  STRING_WIDTH_SETCHAR(UNICODE_PRINTER_WIDTH(x),(x)->up_buffer,i,v) /* Replace a character (`v' must fit into the buffer's current width) */

#ifndef NDEBUG
#define unicode_printer_truncate(self,len) \
 (void)(ASSERT((len) <= (self)->up_length), \
       (self)->up_length ? (void)UNICODE_PRINTER_SETCHAR(self,(len),0) : (void)0, \
       (self)->up_length = (len))
#else
#define unicode_printer_truncate(self,len) \
 (void)(ASSERT((len) <= (self)->up_length), \
       (self)->up_length = (len))
#endif

/* _Always_ inherit all string data (even upon error) saved in
 * `self', and construct a new string from all that data, before
 * returning a reference to that string.
 * NOTE: A pending, incomplete UTF-8 character sequence is discarded.
 *      ---> Regardless of return value, `self' is finalized and left
 *           in an undefined state, the same way it would have been
 *           after a call to `unicode_printer_fini()'
 * @return: * :   A reference to the packed string.
 * @return: NULL: An error occurred. */
DFUNDEF DREF DeeObject *DCALL
unicode_printer_pack(/*inherit(always)*/struct unicode_printer *__restrict self);

/* Same as `unicode_printer_pack()', but don't throw errors upon failure, but
 * simply return `NULL' and leave `self' in a valid state, ready for the call
 * to be repeated at a later time. */
DFUNDEF DREF DeeObject *DCALL
unicode_printer_trypack(/*inherit(on_success)*/struct unicode_printer *__restrict self);

/* Try to pre-allocate memory for `num_chars' characters.
 * NOTE: This function merely acts as a hint, and calls may even be ignored. */
DFUNDEF void DCALL
unicode_printer_allocate(struct unicode_printer *__restrict self,
                         size_t num_chars);

/* Append a single character to the given printer.
 * If `ch' can't fit the currently set `up_width', copy already
 * written data into a larger representation before appending `ch'.
 * @return:  0: Successfully appended the character.
 * @return: -1: An error occurred. */
DFUNDEF int (DCALL unicode_printer_putc)(struct unicode_printer *__restrict self, uint32_t ch);
/* Append an ASCII character. */
DFUNDEF int (DCALL unicode_printer_putascii)(struct unicode_printer *__restrict self, char ch);
/* Append a UTF-8 character. */
DFUNDEF int (DCALL unicode_printer_pututf8)(struct unicode_printer *__restrict self, char ch);
/* Append a UTF-16 character. */
DFUNDEF int (DCALL unicode_printer_pututf16)(struct unicode_printer *__restrict self, uint16_t ch);

/* Append a UTF-32 character. */
#ifdef __INTELLISENSE__
int (unicode_printer_pututf32)(struct unicode_printer *__restrict self, uint32_t ch);
#else
#define unicode_printer_pututf32(self,ch)  unicode_printer_putc(self,ch)
#endif

/* Append an 8-, 16-, or 32-bit unicode character. */
#ifdef __INTELLISENSE__
int (unicode_printer_put8)(struct unicode_printer *__restrict self, uint8_t ch);
int (unicode_printer_put16)(struct unicode_printer *__restrict self, uint16_t ch);
int (unicode_printer_put32)(struct unicode_printer *__restrict self, uint32_t ch);
#else
#define unicode_printer_put8(self,ch)   unicode_printer_putc(self,ch)
#define unicode_printer_put16(self,ch)  unicode_printer_putc(self,ch)
#define unicode_printer_put32(self,ch)  unicode_printer_putc(self,ch)
#endif

/* Append UTF-8 text to the back of the given printer.
 * An incomplete UTF-8 sequences can be completed by future uses of this function.
 * HINT: This function is intentionally designed as compatible with `dformatprinter'
 *       >> DeeObject_Print(ob,(dformatprinter)&unicode_printer_print,&printer);
 * @return: textlen: Successfully appended the string.
 * @return: -1:      Failed to append the string. */
DFUNDEF dssize_t
(DCALL unicode_printer_print)(struct unicode_printer *__restrict self,
                              /*utf-8*/char const *__restrict text,
                              size_t textlen);

/* Append UTF-16 text to the back of the given printer.
 * @return: textlen: Successfully appended the string.
 * @return: -1:      Failed to append the string. */
DFUNDEF dssize_t
(DCALL unicode_printer_printutf16)(struct unicode_printer *__restrict self,
                                   /*utf-16*/uint16_t const *__restrict text,
                                   size_t textlen);

/* Explicitly print utf-8/utf-32 text. */
#ifdef __INTELLISENSE__
DFUNDEF dssize_t
(DCALL unicode_printer_printascii)(struct unicode_printer *__restrict self,
                                   /*ascii*/char const *__restrict text,
                                   size_t textlen);
DFUNDEF dssize_t
(DCALL unicode_printer_printutf8)(struct unicode_printer *__restrict self,
                                  /*utf-8*/char const *__restrict text,
                                  size_t textlen);
DFUNDEF dssize_t
(DCALL unicode_printer_printutf32)(struct unicode_printer *__restrict self,
                                   /*utf-32*/uint32_t const *__restrict text,
                                   size_t textlen);
#else
#define unicode_printer_printascii(self,text,textlen) \
        unicode_printer_print8(self,(uint8_t *)(text),textlen)
#define unicode_printer_printutf8(self,text,textlen) \
        unicode_printer_print(self,text,textlen)
#define unicode_printer_printutf32(self,text,textlen) \
        unicode_printer_print32(self,text,textlen)
#endif

/* Append the ASCII character `ch' a total of `num_repetitions' times. */
DFUNDEF dssize_t
(DCALL unicode_printer_repeatascii)(struct unicode_printer *__restrict self,
                                    char ch, size_t num_repetitions);


/* Reserve `num_chars' characters, to-be either using
 * `UNICODE_PRINTER_SETCHAR()', or `DeeString_SetChar()'.
 * The return value of this function is the starting index of the reservation,
 * which is made at the end of the currently printed portion of text.
 * @return: * : The starting index of the reservation.
 * @return: -1: An error occurred. */
DFUNDEF dssize_t
(DCALL unicode_printer_reserve)(struct unicode_printer *__restrict self,
                                size_t num_chars);


/* Print the entire contents of `self' into `printer'
 * NOTE: This function is allowed to erase characters from `self' if `printer'
 *       is recognized to be another unicode-printer. However, regardless of
 *       whether of not this is done, the caller must still finalize `self'! */
DFUNDEF dssize_t
(DCALL unicode_printer_printinto)(struct unicode_printer *__restrict self,
                                  dformatprinter printer, void *arg);

/* Append the characters from the given string object `string' at the end of `self' */
DFUNDEF dssize_t
(DCALL unicode_printer_printstring)(struct unicode_printer *__restrict self,
                                    DeeObject *__restrict string);


/* Print raw 8,16 or 32-bit-per-character sequences, consisting of unicode characters.
 *  - `unicode_printer_print8' prints characters from the range U+0000 .. U+00FF (aka. latin-1)
 *  - `unicode_printer_print16' prints characters from the range U+0000 .. U+FFFF
 *  - `unicode_printer_print32' prints characters from the range U+0000 .. U+10FFFF (really is FFFFFFFF)
 * @return: textlen: Successfully appended the string.
 * @return: -1:      Failed to append the string. */
DFUNDEF dssize_t (DCALL unicode_printer_print8)(struct unicode_printer *__restrict self, uint8_t const *__restrict text, size_t textlen);
DFUNDEF dssize_t (DCALL unicode_printer_print16)(struct unicode_printer *__restrict self, uint16_t const *__restrict text, size_t textlen);
DFUNDEF dssize_t (DCALL unicode_printer_print32)(struct unicode_printer *__restrict self, uint32_t const *__restrict text, size_t textlen);

/* Search for existing occurrences of, or append a new instance of a given string.
 * Upon success, return the index (offset from the base) of the string (in characters).
 * @return: * : The offset from the base of string being printed, where the given `str' can be found.
 * @return: -1: Failed to allocate the string. */
DFUNDEF dssize_t (DCALL unicode_printer_reuse)(struct unicode_printer *__restrict self, /*utf-8*/char const *__restrict str, size_t length);
DFUNDEF dssize_t (DCALL unicode_printer_reuse8)(struct unicode_printer *__restrict self, uint8_t const *__restrict str, size_t length);
DFUNDEF dssize_t (DCALL unicode_printer_reuse16)(struct unicode_printer *__restrict self, uint16_t const *__restrict str, size_t length);
DFUNDEF dssize_t (DCALL unicode_printer_reuse32)(struct unicode_printer *__restrict self, uint32_t const *__restrict str, size_t length);


/* Allocate buffers for UTF-8 with the intent of appending them to the end of the unicode printer.
 * Under specific circumstances, these functions allow the printer to allocate
 * the utf-8 string as in-line to the string being generated, with `unicode_printer_confirm_utf8()'
 * then checking if the buffer contains non-ascii characters, in which case the string would be
 * up-cast. However, if the buffer cannot be allocated in-line, it is allocated on the heap, and
 * a later call to `unicode_printer_confirm_utf8()' will append it normally.
 * Note however that when a UTF-8 buffer has been allocated, no text may be printed to the printer
 * before that buffer is either confirmed, or freed. However, this shouldn't be a problem,
 * considering the intended usage case in something like this:
 * >> dssize_t print_pwd(struct unicode_printer *__restrict printer) {
 * >>     size_t bufsize = 256;
 * >>     char *buffer,*new_buffer;
 * >>     buffer = unicode_printer_alloc_utf8(printer,bufsize);
 * >>     if unlikely(!buffer) goto err;
 * >>     while unlikely(!getcwd(buffer,bufsize)) {
 * >>         if (errno != ERANGE) {
 * >>             ...
 * >>             goto err_buffer;
 * >>         }
 * >>         bufsize *= 2;
 * >>         new_buffer = unicode_printer_resize_utf8(printer,buffer,bufsize);
 * >>         if unlikely(!new_buffer) goto err_buffer;
 * >>         buffer = new_buffer;
 * >>     }
 * >>     return unicode_printer_confirm_utf8(printer,buffer,strlen(buffer));
 * >> err_buffer:
 * >>     unicode_printer_free_utf8(printer,buffer);
 * >> err:
 * >>     return -1;
 * >> }
 * HINT: The unicode printer always allocates 1 byte more than you requested, allowing
 *       you to write 1 past the end (usually meant for some trailing \0-character that
 *       you don't really care about)
 * NOTE: All functions operate identical to those one would expect to find in a
 *       heap-API, with `unicode_printer_confirm_utf8()' acting as a semantically
 *       similar behavior to `unicode_printer_free_utf8()', which will ensure that
 *       the contents of the buffer are decoded and appended at the end of the string
 *       that is being printed.
 * NOTE: Passing `NULL' for `buf' to `unicode_printer_resize_utf8()' or
 *       `unicode_printer_tryresize_utf8()' will allocate a new buffer, the same
 *       way `unicode_printer_alloc_utf8()' and `unicode_printer_tryalloc_utf8()' would have)
 * NOTE: Passing `NULL' for `buf' to `unicode_printer_free_utf8()' is a no-op
 * NOTE: Passing `NULL' for `buf' to `unicode_printer_confirm_utf8()' is a no-op and causes `0' to be returned.
 * @return[unicode_printer_alloc_utf8]:     * :   The pointer to the base of a utf-8 buffer consisting of `length' bytes.
 * @return[unicode_printer_alloc_utf8]:     NULL: An error occurred.
 * @return[unicode_printer_tryalloc_utf8]:  * :   Failed to allocate the buffer.
 * @return[unicode_printer_tryalloc_utf8]:  NULL: An error occurred.
 * @return[unicode_printer_resize_utf8]:    * :   The reallocated base pointer.
 * @return[unicode_printer_resize_utf8]:    NULL: An error occurred.
 * @return[unicode_printer_tryresize_utf8]: * :   The reallocated base pointer.
 * @return[unicode_printer_tryresize_utf8]: NULL: Failed to allocate / resize the buffer.
 * @return[unicode_printer_confirm_utf8]:   * :   The number of printed characters.
 * @return[unicode_printer_confirm_utf8]:   < 0 : An error occurred. */
DFUNDEF char *(DCALL unicode_printer_alloc_utf8)(struct unicode_printer *__restrict self, size_t length);                     /* Dee_Malloc()-like */
DFUNDEF char *(DCALL unicode_printer_tryalloc_utf8)(struct unicode_printer *__restrict self, size_t length);                  /* Dee_Malloc()-like */
DFUNDEF char *(DCALL unicode_printer_resize_utf8)(struct unicode_printer *__restrict self, char *buf, size_t new_length);     /* Dee_Realloc()-like */
DFUNDEF char *(DCALL unicode_printer_tryresize_utf8)(struct unicode_printer *__restrict self, char *buf, size_t new_length);  /* Dee_TryRealloc()-like */
DFUNDEF void (DCALL unicode_printer_free_utf8)(struct unicode_printer *__restrict self, char *buf);                           /* Dee_Free()-like */
DFUNDEF dssize_t (DCALL unicode_printer_confirm_utf8)(struct unicode_printer *__restrict self, /*inherit(always)*/char *buf, size_t final_length);

/* Same as the functions above, however used to allocate utf-16 character buffers. */
DFUNDEF uint16_t *(DCALL unicode_printer_alloc_utf16)(struct unicode_printer *__restrict self, size_t length);                        /* Dee_Malloc()-like */
DFUNDEF uint16_t *(DCALL unicode_printer_tryalloc_utf16)(struct unicode_printer *__restrict self, size_t length);                     /* Dee_Malloc()-like */
DFUNDEF uint16_t *(DCALL unicode_printer_resize_utf16)(struct unicode_printer *__restrict self, uint16_t *buf, size_t new_length);    /* Dee_Realloc()-like */
DFUNDEF uint16_t *(DCALL unicode_printer_tryresize_utf16)(struct unicode_printer *__restrict self, uint16_t *buf, size_t new_length); /* Dee_TryRealloc()-like */
DFUNDEF void (DCALL unicode_printer_free_utf16)(struct unicode_printer *__restrict self, uint16_t *buf);                              /* Dee_Free()-like */
DFUNDEF dssize_t (DCALL unicode_printer_confirm_utf16)(struct unicode_printer *__restrict self, /*inherit(always)*/uint16_t *buf, size_t final_length);

#ifdef __INTELLISENSE__
dwchar_t *(unicode_printer_alloc_wchar)(struct unicode_printer *__restrict self, size_t length);                        /* Dee_Malloc()-like */
dwchar_t *(unicode_printer_tryalloc_wchar)(struct unicode_printer *__restrict self, size_t length);                     /* Dee_Malloc()-like */
dwchar_t *(unicode_printer_resize_wchar)(struct unicode_printer *__restrict self, dwchar_t *buf, size_t new_length);    /* Dee_Realloc()-like */
dwchar_t *(unicode_printer_tryresize_wchar)(struct unicode_printer *__restrict self, dwchar_t *buf, size_t new_length); /* Dee_TryRealloc()-like */
void (unicode_printer_free_wchar)(struct unicode_printer *__restrict self, dwchar_t *buf);                              /* Dee_Free()-like */
dssize_t (unicode_printer_confirm_wchar)(struct unicode_printer *__restrict self, /*inherit(always)*/dwchar_t *buf, size_t final_length);
#elif __SIZEOF_WCHAR_T__ == 2
#define unicode_printer_alloc_wchar(self,length)             ((dwchar_t *)unicode_printer_alloc_utf16(self,length))
#define unicode_printer_tryalloc_wchar(self,length)          ((dwchar_t *)unicode_printer_tryalloc_utf16(self,length))
#define unicode_printer_resize_wchar(self,buf,new_length)    ((dwchar_t *)unicode_printer_resize_utf16(self,(uint16_t *)(buf),new_length))
#define unicode_printer_tryresize_wchar(self,buf,new_length) ((dwchar_t *)unicode_printer_tryresize_utf16(self,(uint16_t *)(buf),new_length))
#define unicode_printer_free_wchar(self,buf)                   unicode_printer_free_utf16(self,(uint16_t *)(buf))
#define unicode_printer_confirm_wchar(self,buf,final_length)   unicode_printer_confirm_utf16(self,(uint16_t *)(buf),final_length)
#else
#define unicode_printer_alloc_wchar(self,length)             ((dwchar_t *)unicode_printer_alloc_utf32(self,length))
#define unicode_printer_tryalloc_wchar(self,length)          ((dwchar_t *)unicode_printer_tryalloc_utf32(self,length))
#define unicode_printer_resize_wchar(self,buf,new_length)    ((dwchar_t *)unicode_printer_resize_utf32(self,(uint32_t *)(buf),new_length))
#define unicode_printer_tryresize_wchar(self,buf,new_length) ((dwchar_t *)unicode_printer_tryresize_utf32(self,(uint32_t *)(buf),new_length))
#define unicode_printer_free_wchar(self,buf)                   unicode_printer_free_utf32(self,(uint32_t *)(buf))
#define unicode_printer_confirm_wchar(self,buf,final_length)   unicode_printer_confirm_utf32(self,(uint32_t *)(buf),final_length)
#endif

#if 0
/* Allocate raw unicode character buffer. */
DFUNDEF uint8_t *DCALL unicode_printer_alloc8(struct unicode_printer *__restrict self, size_t length);                       /* Dee_Malloc()-like */
DFUNDEF uint8_t *DCALL unicode_printer_tryalloc8(struct unicode_printer *__restrict self, size_t length);                    /* Dee_Malloc()-like */
DFUNDEF uint8_t *DCALL unicode_printer_resize8(struct unicode_printer *__restrict self, uint8_t *buf, size_t new_length);    /* Dee_Realloc()-like */
DFUNDEF uint8_t *DCALL unicode_printer_tryresize8(struct unicode_printer *__restrict self, uint8_t *buf, size_t new_length); /* Dee_TryRealloc()-like */
DFUNDEF void DCALL unicode_printer_free8(struct unicode_printer *__restrict self, uint8_t *buf);                             /* Dee_Free()-like */
DFUNDEF dssize_t DCALL unicode_printer_confirm8(struct unicode_printer *__restrict self, uint8_t *buf, size_t final_length);
DFUNDEF uint16_t *DCALL unicode_printer_alloc16(struct unicode_printer *__restrict self, size_t length);                       /* Dee_Malloc()-like */
DFUNDEF uint16_t *DCALL unicode_printer_tryalloc16(struct unicode_printer *__restrict self, size_t length);                    /* Dee_Malloc()-like */
DFUNDEF uint16_t *DCALL unicode_printer_resize16(struct unicode_printer *__restrict self, uint16_t *buf, size_t new_length);    /* Dee_Realloc()-like */
DFUNDEF uint16_t *DCALL unicode_printer_tryresize16(struct unicode_printer *__restrict self, uint16_t *buf, size_t new_length); /* Dee_TryRealloc()-like */
DFUNDEF void DCALL unicode_printer_free16(struct unicode_printer *__restrict self, uint16_t *buf);                             /* Dee_Free()-like */
DFUNDEF dssize_t DCALL unicode_printer_confirm16(struct unicode_printer *__restrict self, uint16_t *buf, size_t final_length);
DFUNDEF uint32_t *DCALL unicode_printer_alloc32(struct unicode_printer *__restrict self, size_t length);                       /* Dee_Malloc()-like */
DFUNDEF uint32_t *DCALL unicode_printer_tryalloc32(struct unicode_printer *__restrict self, size_t length);                    /* Dee_Malloc()-like */
DFUNDEF uint32_t *DCALL unicode_printer_resize32(struct unicode_printer *__restrict self, uint32_t *buf, size_t new_length);    /* Dee_Realloc()-like */
DFUNDEF uint32_t *DCALL unicode_printer_tryresize32(struct unicode_printer *__restrict self, uint32_t *buf, size_t new_length); /* Dee_TryRealloc()-like */
DFUNDEF void DCALL unicode_printer_free32(struct unicode_printer *__restrict self, uint32_t *buf);                             /* Dee_Free()-like */
DFUNDEF dssize_t DCALL unicode_printer_confirm32(struct unicode_printer *__restrict self, uint32_t *buf, size_t final_length);
#endif



/* Find a given unicode character within the specified index-range.
 * @return: * : The index of the character, offset from the start of the printer.
 * @return: -1: The character wasn't found. */
DFUNDEF dssize_t (DCALL unicode_printer_memchr)(struct unicode_printer *__restrict self, uint32_t chr, size_t start, size_t length);
DFUNDEF dssize_t (DCALL unicode_printer_memrchr)(struct unicode_printer *__restrict self, uint32_t chr, size_t start, size_t length);



#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define unicode_printer_putc(self,ch)     __builtin_expect(unicode_printer_putc(self,ch),0)
#define unicode_printer_putascii(self,ch) __builtin_expect(unicode_printer_putascii(self,ch),0)
#define unicode_printer_pututf8(self,ch)  __builtin_expect(unicode_printer_pututf8(self,ch),0)
#define unicode_printer_pututf16(self,ch) __builtin_expect(unicode_printer_pututf16(self,ch),0)
#endif
#endif



/* Helper macros for printing compile-time strings, of printf-style data. */
#define UNICODE_PRINTER_PRINT(self,S) \
        unicode_printer_print8(self,(uint8_t *)(S),COMPILER_STRLEN(S))
#ifdef __INTELLISENSE__
dssize_t (unicode_printer_printf)(struct unicode_printer *__restrict self, char const *__restrict format, ...);
dssize_t (unicode_printer_vprintf)(struct unicode_printer *__restrict self, char const *__restrict format, va_list args);
dssize_t (unicode_printer_printobject)(struct unicode_printer *__restrict self, DeeObject *__restrict ob);
dssize_t (unicode_printer_printobjectrepr)(struct unicode_printer *__restrict self, DeeObject *__restrict ob);
#else
#define unicode_printer_printf(self,...)          Dee_FormatPrintf((dformatprinter)&unicode_printer_print,self,__VA_ARGS__)
#define unicode_printer_vprintf(self,format,args) Dee_VFormatPrintf((dformatprinter)&unicode_printer_print,self,format,args)
#define unicode_printer_printobject(self,ob)      DeeObject_Print(ob,(dformatprinter)&unicode_printer_print,self)
#define unicode_printer_printobjectrepr(self,ob)  DeeObject_PrintRepr(ob,(dformatprinter)&unicode_printer_print,self)
#endif





/* Encode/decode `self' (usually a bytes- or string-object) to/from a codec `name'.
 * These functions will start by normalize `name', checking if it refers to
 * one of the builtin codecs, and if it doesn't, make an external function
 * call to `encode from codecs' / `decode from codecs':
 * >> name = name.lower().replace("_","-");
 * >> if (name.startswith("iso-"))
 * >>  name = "iso"+name[4:];
 * >> else if (name.startswith("cp-")) {
 * >>  name = "cp"+name[3:];
 * >> }
 * >> if (has_builtin_codec(name))
 * >>     return builtin_encode(self,name,error_mode); // or decode...
 * The following is a list of the recognized builtin codecs,
 * as well as the types of objects returned by either.
 *  - "ascii", "646", "us-ascii"
 *  - "latin-1", "iso8859-1", "iso8859", "8859", "cp819", "latin", "latin1", "l1"
 *  - "utf-8", "utf8", "u8", "utf"
 *  - "utf-16", "utf16", "u16"
 *  - "utf-16-le", "utf16-le", "u16-le", "utf-16le", "utf16le", "u16le"
 *  - "utf-16-be", "utf16-be", "u16-be", "utf-16be", "utf16be", "u16be"
 *  - "utf-32", "utf32", "u32"
 *  - "utf-32-le", "utf32-le", "u32-le", "utf-32le", "utf32le", "u32le"
 *  - "utf-32-be", "utf32-be", "u32-be", "utf-32be", "utf32be", "u32be"
 *  - "string-escape", "backslash-escape", "c-escape"
 * @param: error_mode: One of `STRING_ERROR_F*'
 * @return: * :   The encoded/decoded variant of `self'
 *                The type of this object is unrelated to `self', but rather
 *                depends on `self' and is usually a bytes, or string object.
 *                In most cases, `DeeCodec_Decode()' returns a string object,
 *                while `DeeCodec_Encode()' returns a bytes object.
 * @return: NULL: An error occurred.
 */
DFUNDEF DREF DeeObject *DCALL
DeeCodec_Decode(DeeObject *__restrict self,
                DeeObject *__restrict name,
                unsigned int error_mode);
DFUNDEF DREF DeeObject *DCALL
DeeCodec_Encode(DeeObject *__restrict self,
                DeeObject *__restrict name,
                unsigned int error_mode);



/* Given a regular expression `pattern', check if it
 * matches the string found in `data', returning the
 * number of bytes in `data' that are being matched,
 * or `0' if the pattern doesn't match.
 * @param: datalen:     Number of bytes (not characters) in data.
 * @param: patternlen:  Number of bytes (not characters) in pattern.
 * @return: * :         Number of characters (not bytes) matched in `data'.
 * @return: 0 :         Pattern not found.
 * @return: (size_t)-1: Error. */
DFUNDEF size_t DCALL
DeeRegex_Matches(/*utf-8*/char const *__restrict data, size_t datalen,
                 /*utf-8*/char const *__restrict pattern, size_t patternlen,
                 uint16_t flags);
#define DEE_REGEX_FNORMAL    0x0000 /* Normal regex flags. */
#define DEE_REGEX_FDOTALL    0x0001 /* [NAME("DOTALL")] The `.' regex matches anything (including new-lines) */
#define DEE_REGEX_FMULTILINE 0x0002 /* [NAME("MULTILINE")] Allow `^' to match not only at the start of input data, but also immediately after a line-feed. */
#define DEE_REGEX_FNOCASE    0x0004 /* [NAME("NOCASE")] Ignore case when matching single characters, or character ranges. */

/* Same as `DeeRegex_Matches()', but also store a pointer to the end of
 * consumed data in `pdataend'. Because input data is formatted in UTF-8,
 * this isn't position would only be equal to `data + return' if all input
 * data was ASCII only, meaning that in the universal case, this function
 * becomes useful when dealing with unicode data.
 * @param: pdataend:    Upon success (return != 0 && return != (size_t)-1),
 *                      save a pointer to the end of consumed data here.
 * @param: datalen:     Number of bytes (not characters) in data.
 * @param: patternlen:  Number of bytes (not characters) in pattern.
 * @return: * :         Number of characters (not bytes) matched in `data'.
 * @return: 0 :         Pattern not found.
 * @return: (size_t)-1: Error. */
DFUNDEF size_t DCALL
DeeRegex_MatchesPtr(/*utf-8*/char const *__restrict data, size_t datalen,
                    /*utf-8*/char const *__restrict pattern, size_t patternlen,
                    /*utf-8*/char const **__restrict pdataend,
                    uint16_t flags);


struct regex_range {
    size_t rr_start; /* Starting character index. */
    size_t rr_end;   /* End character index. */
};
struct regex_range_ex {
    size_t rr_start;     /* Starting character index. */
    size_t rr_end;       /* End character index. */
    char  *rr_start_ptr; /* Starting character pointer. */
    char  *rr_end_ptr;   /* End character pointer. */
};
struct regex_range_ptr {
    char  *rr_start; /* Starting character pointer. */
    char  *rr_end;   /* End character pointer. */
};

/* Find the first instance matching `pattern' and store the
 * character indices (not byte offsets) in `*pstart' and `*pend'
 * @return: 1:  Pattern was found.
 * @return: 0:  Pattern not found.
 * @return: -1: Error. */
DFUNDEF int DCALL
DeeRegex_Find(/*utf-8*/char const *__restrict data, size_t datalen,
              /*utf-8*/char const *__restrict pattern, size_t patternlen,
              struct regex_range *__restrict presult, uint16_t flags);
DFUNDEF int DCALL
DeeRegex_RFind(/*utf-8*/char const *__restrict data, size_t datalen,
               /*utf-8*/char const *__restrict pattern, size_t patternlen,
               struct regex_range *__restrict presult, uint16_t flags);
DFUNDEF int DCALL
DeeRegex_FindEx(/*utf-8*/char const *__restrict data, size_t datalen,
                /*utf-8*/char const *__restrict pattern, size_t patternlen,
                struct regex_range_ex *__restrict presult, uint16_t flags);
DFUNDEF int DCALL
DeeRegex_RFindEx(/*utf-8*/char const *__restrict data, size_t datalen,
                 /*utf-8*/char const *__restrict pattern, size_t patternlen,
                 struct regex_range_ex *__restrict presult, uint16_t flags);

/* Same as the functions above, but return character pointers, rather than indices. */
DFUNDEF int DCALL
DeeRegex_FindPtr(/*utf-8*/char const *__restrict data, size_t datalen,
                 /*utf-8*/char const *__restrict pattern, size_t patternlen,
                 struct regex_range_ptr *__restrict presult, uint16_t flags);
DFUNDEF int DCALL
DeeRegex_RFindPtr(/*utf-8*/char const *__restrict data, size_t datalen,
                  /*utf-8*/char const *__restrict pattern, size_t patternlen,
                  struct regex_range_ptr *__restrict presult, uint16_t flags);

DECL_END

#endif /* !GUARD_DEEMON_STRING_H */
