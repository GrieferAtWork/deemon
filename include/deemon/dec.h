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
#ifndef GUARD_DEEMON_DEC_H
#define GUARD_DEEMON_DEC_H 1

#include "api.h"
#include <stdint.h>

#ifdef CONFIG_BUILDING_DEEMON
#ifndef CONFIG_NO_DEC
#include "object.h"
#endif /* !CONFIG_NO_DEC */
#endif /* CONFIG_BUILDING_DEEMON */


DECL_BEGIN

#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(push,1)
#endif


/* The max size of a DEC file.
 * If a file is larger than this, the loader is allowed to stop its attempts
 * at parsing it and simply act as though it wasn't a DEC file to begin with. */
#define DFILE_LIMIT  0xffffff


#define DI_MAG0    0    /* File identification byte 0 index */
#define DECMAG0    0x7f /* Magic number byte 0 */
#define DI_MAG1    1    /* File identification byte 1 index */
#define DECMAG1   'D'   /* Magic number byte 1 */
#define DI_MAG2    2    /* File identification byte 2 index */
#define DECMAG2   'E'   /* Magic number byte 2 */
#define DI_MAG3    3    /* File identification byte 3 index */
#define DECMAG3   'C'   /* Magic number byte 3 */
#define DI_NIDENT  4    /* Number of identification bytes */



#define DVERSION_CUR  0 /* The currently active version of the DEC file format. */

typedef struct PACKED {
    uint8_t  e_ident[DI_NIDENT]; /* Identification bytes. (See `DI_*') */
    uint8_t  e_builtinset;       /* Set of builtin object (One of `DBUILTINS_*') */
    uint8_t  e_size;             /* Absolute size of the header (`Dec_Ehdr').
                                  * NOTE: When larger than expected, don't assume errors, but ignore trailing data! */
    uint16_t e_version;          /* DEC version number (One of `DVERSION_*'). */
    uint32_t e_impoff;           /* Absolute file-offset of the import string map (`Dec_Strmap').
                                  * NOTE: When ZERO(0), then there is no import table.
                                  * NOTE: Strings are encoded in relative-import format
                                  *      (as accepted by `DeeModule_OpenRelative') */
    uint32_t e_depoff;           /* When checking if a DEC file has become outdated, the obvious
                                  * candidate that must be checked is the `*.dex' replaced with `*.dee'.
                                  * However because deemon makes use of a C-compatible preprocessor,
                                  * user-code can create additional dependencies that must also be
                                  * able to trigger dismissal and re-generation of `*.dec' files.
                                  * This field describes an absolute file-offset to a `Dec_Strmap'
                                  * structure containing the relative pathnames (using `/' for slashes)
                                  * of all additional dependencies who's timestamps must be checked
                                  * before it can be confirmed that no dependency of this DEC file
                                  * has been modified. */
    uint32_t e_globoff;          /* Absolute file-offset of the global variable table (`Dec_Glbmap').
                                  * NOTE: When ZERO(0), then there are no globals. */
    uint32_t e_rootoff;          /* Absolute file-offset of the root code object (`Dec_Code'). */
    uint32_t e_stroff;           /* Absolute file-offset of the UTF-8 encoded string table. */
    uint32_t e_strsiz;           /* The absolute length of the string table (in bytes). */
    uint32_t e_timestamp_lo;     /* In microseconds since `01.01.1970', the point in
                                  * time when this compiled source file was generated.
                                  * Since DEC files are meant to be used as a shallow
                                  * cache file that is automatically discarded if the
                                  * actual source file has changed since, this is the
                                  * time against which everything must be compared. */
    uint32_t e_timestamp_hi;     /* High 32-bits of the timestamp. */
} Dec_Ehdr;



typedef struct PACKED {
    uint16_t i_len;    /* Number of string pointers.
                        * NOTE: When (uint16_t)-1, a `uint32_t' follows with the actual size. */
    uint8_t  i_map[1]; /* Offsets into the string table (`e_stroff') to zero-terminated string.
                        * NOTE: Individual pointers are decoded using `Dec_DecodePointer()'. */
} Dec_Strmap;

typedef struct PACKED {
    uint16_t   s_flg;       /* Symbol flags (Set of `MODSYM_F*') */
    uint8_t    s_nam[1];    /* Name of the symbol. - offsets into the string table (`e_stroff').
                             * NOTE: Individual pointers are decoded using `Dec_DecodePointer()'.
                             * NOTE: If this value points to an empty string, then this
                             *       symbol hasn't been given a name and neither `s_doclen',
                             *       nor `s_doc' exist. */
    uint8_t    s_doclen[1]; /* Length of the documentation string (decodable using `Dec_DecodePointer()'). */
    uint8_t    s_doc[1];    /* Documentation string of the symbol. - offsets into the string table (`e_stroff').
                             * NOTE: Individual pointers are decoded using `Dec_DecodePointer()'.
                             * NOTE: Only exists when `s_doclen' evaluates to non-zero. */
} Dec_GlbSym;

typedef struct PACKED {
    uint16_t   s_flg;       /* Symbol flags (Set of `MODSYM_F*') */
    uint16_t   s_addr;      /* Symbol address, or module import index, or getter symbol index. */
    uint16_t   s_addr2;     /* [exists_if(s_flg & MODSYM_FEXTERN)] External module symbol index. */
    uint8_t    s_nam[1];    /* Name of the symbol. - offsets into the string table (`e_stroff').
                             * NOTE: Individual pointers are decoded using `Dec_DecodePointer()'.
                             * NOTE: If this value points to an empty string, then this
                             *       symbol hasn't been given a name and neither `s_doclen',
                             *       nor `s_doc' exist. */
    uint8_t    s_doclen[1]; /* Length of the documentation string (decodable using `Dec_DecodePointer()'). */
    uint8_t    s_doc[1];    /* Documentation string of the symbol. - offsets into the string table (`e_stroff').
                             * NOTE: Individual pointers are decoded using `Dec_DecodePointer()'.
                             * NOTE: Only exists when `s_doclen' evaluates to non-zero. */
} Dec_GlbExt;

typedef struct PACKED {
    uint16_t   g_cnt;    /* Number of global variables. */
    uint16_t   g_len;    /* Number of global symbols. */
    Dec_GlbSym g_map[1]; /* [g_cnt] Vector of global variable descriptors. */
    Dec_GlbExt g_ext[1]; /* [g_len-g_cnt] Vector of extended global variable descriptors. */
} Dec_Glbmap;


typedef struct PACKED {
    uint8_t    cs_type;    /* Type code (One of `DTYPE_*') */
    uint8_t    cs_data[1]; /* Type data (depends on `DTYPE_*'; may not actually exist)
                            * NOTE: If or not `DTYPE_NULL' is allowed depends on the usage context. */
} Dec_Object;


/* Code controller data structures. */
typedef struct PACKED {
    uint16_t   os_len;    /* Number of objects. */
    Dec_Object os_vec[1]; /* Vector of objects.
                           * NOTE: Whether or not `DTYPE_NULL' is allowed
                           *       depends on the usage context. */
} Dec_Objects;

typedef struct PACKED {
    uint16_t   ce_flags;  /* Set of `EXCEPTION_HANDLER_F*' */
    uint32_t   ce_begin;  /* [<= ce_end] Exception handler protection start address. */
    uint32_t   ce_end;    /* [>= ce_begin] Exception handler protection end address. */
    uint32_t   ce_addr;   /* [< ce_begin && >= ce_end] Exception handler entry point. */
    uint16_t   ce_stack;  /* Stack depth that must be ensured when this handler is executed. */
    Dec_Object ce_mask;   /* Exception handler mask. (NOTE: `DTYPE_NULL' is allowed) */
} Dec_CodeExcept;
typedef struct PACKED {
    uint16_t       ces_len;    /* Amount of exception descriptors. */
    Dec_CodeExcept ces_vec[1]; /* [ces_len] Vector of exception descriptors. */
} Dec_CodeExceptions;

typedef struct PACKED {
    uint16_t   ce_flags;  /* Set of `EXCEPTION_HANDLER_F*' */
    uint16_t   ce_begin;  /* [<= ce_end] Exception handler protection start address. */
    uint16_t   ce_end;    /* [>= ce_begin] Exception handler protection end address. */
    uint16_t   ce_addr;   /* [< ce_begin && >= ce_end] Exception handler entry point. */
    uint8_t    ce_stack;  /* Stack depth that must be ensured when this handler is executed. */
    Dec_Object ce_mask;   /* Exception handler mask. (NOTE: `DTYPE_NULL' is allowed) */
} Dec_8BitCodeExcept;
typedef struct PACKED {
    uint8_t            ces_len;    /* Amount of exception descriptors. */
    Dec_8BitCodeExcept ces_vec[1]; /* [ces_len] Vector of exception descriptors. */
} Dec_8BitCodeExceptions;

typedef struct PACKED {
    /* All uint8_t[1]-fields in this structure are tightly packed
     * integers decodable using `READ_SLEB()' or `READ_ULEB()'
     * The actual length and member-offsets within this
     * structure are therefor only known at compile-time. */
    uint16_t     rs_flags;   /* Set of `DDI_REGS_F*' */
    uint8_t      rs_uip[1];  /* [DECODE(READ_ULEB)] Initial user instruction. */
    uint8_t      rs_usp[1];  /* [DECODE(READ_ULEB)] Initial stack alignment/depth. */
    uint8_t      rs_path[1]; /* [DECODE(READ_ULEB)] Initial path number. (NOTE: ZERO indicates no path and all other values are used as index-1 in the `cd_paths' vector of the associated `Dec_CodeDDI') */
    uint8_t      rs_file[1]; /* [DECODE(READ_ULEB)] Initial file number. */
    uint8_t      rs_name[1]; /* [DECODE(READ_ULEB)] Initial name offset (points into the string table). */
    uint8_t      rs_col[1];  /* [DECODE(READ_SLEB)] Initial column number within the active line. */
    uint8_t      rs_lno[1];  /* [DECODE(READ_SLEB)] Initial Line number (0-based). */
} Dec_DDIRegStart;

typedef struct PACKED {
    uint16_t  dx_size;    /* Data size (when (uint16_t)-1, the a uint32_t containing the actual size follows immediatly) */
    uint8_t   dx_data[1]; /* [dx_size] DDI extension data. */
} Dec_DDIExdat;

typedef struct PACKED {
    uint32_t        cd_strings; /* Absolute pointer to a `Dec_Strmap' structure describing DDI string. */
    uint32_t        cd_ddixdat; /* Absolute pointer to a `Dec_DDIExdat' structure, or 0. */
    uint32_t        cd_ddiaddr; /* Absolute offset into the file to a block of `cd_ddisize' bytes of text describing DDI code (s.a.: `DDI_*'). */
    uint32_t        cd_ddisize; /* The total size (in bytes) of DDI text for translating instruction pointers to file+line, etc. */
    uint16_t        cd_ddiinit; /* Amount of leading DDI instruction bytes that are used for state initialization */
    Dec_DDIRegStart cd_regs;    /* The initial register state. */
} Dec_CodeDDI;

typedef struct PACKED {
    uint16_t        cd_strings; /* Absolute pointer to a `Dec_Strmap' structure describing DDI string. */
    uint16_t        cd_ddixdat; /* Absolute pointer to a `Dec_DDIExdat' structure, or 0. */
    uint16_t        cd_ddiaddr; /* Absolute offset into the file to a block of `cd_ddisize' bytes of text describing DDI code (s.a.: `DDI_*'). */
    uint16_t        cd_ddisize; /* The total size (in bytes) of DDI text for translating instruction pointers to file+line, etc. */
    uint8_t         cd_ddiinit; /* Amount of leading DDI instruction bytes that are used for state initialization */
    Dec_DDIRegStart cd_regs;    /* The initial register state. */
} Dec_8BitCodeDDI;

typedef struct PACKED {
    uint8_t     ck_len[1]; /* Length of the keyword (excluding any trailing \0 character) */
    uint8_t     ck_off[1]; /* [exists_if(ck_len != 0)] Offset into string table to where the keyword's name is written. */
} Dec_CodeKwd;
typedef struct PACKED {
    Dec_CodeKwd ck_map[1]; /* Vector of code keywords.
                            * NOTE: The length of this vector is `co_argc_max' */
} Dec_CodeKwds;

typedef struct PACKED {
    uint8_t  ck_map[1]; /* Offsets into the string table (`e_stroff') to zero-terminated string.
                         * NOTE: Individual pointers are decoded using `Dec_DecodePointer()'.
                         * NOTE: The length of this vector is `co_argc_max' */
} Dec_8BitCodeKwds;

typedef struct PACKED {
    uint16_t   co_flags;      /* Set of `CODE_F*' optionally or'd with `DEC_CODE_F8BIT'.
                               * NOTE: When set, this data structure must be
                               *       interpreted as an `Dec_8BitCode' object */
    uint16_t   co_localc;     /* Amount of local variables used by code. */
    uint16_t   co_refc;       /* Amount of reference variables used by this code. */
    uint16_t   co_argc_min;   /* Min amount of arguments required to execute this code. */
    uint16_t   co_stackmax;   /* The greatest allowed stack depth of the assembly associated with this code. */
    uint16_t   co_pad;        /* ... */
    uint32_t   co_staticoff;  /* Absolute file offset to a static/constant variable descriptor table (`Dec_Objects').
                               * NOTE: `DTYPE_NULL' is not allowed. */
    uint32_t   co_exceptoff;  /* Absolute file offset to an exception handler descriptor table (`Dec_CodeExceptions'). */
    uint32_t   co_defaultoff; /* Absolute file offset to an argument-default descriptor table (`Dec_Objects').
                               * NOTE: When ZERO(0), there are no default arguments, but if not,
                               *       `co_argc_max' can be calculated from `co_argc_min+os_len'
                               * NOTE: `DTYPE_NULL' is not allowed. */
    uint32_t   co_ddioff;     /* Absolute file offset to optional code debug information (`Dec_CodeDDI').
                               * NOTE: When ZERO(0), there is no DDI information. */
    uint32_t   co_kwdoff;     /* Absolute file offset to optional keyword argument information (`Dec_CodeKwds')
                               * NOTE: When ZERO(0), there is no keyword information. */
    uint32_t   co_textsiz;    /* Absolute size of this code's text section (in bytes) */
    uint32_t   co_textoff;    /* Absolute file offset to the assembly text that will be executed by this code. */
} Dec_Code;

typedef struct PACKED {
    uint16_t   co_flags;      /* Set of `CODE_F*' optionally or'd with `DEC_CODE_F8BIT'.
                               * NOTE: This data structure must only (and always) be used
                               *       when the `DEC_CODE_F8BIT' flag is set. */
    uint8_t    co_localc;     /* Amount of local variables used by code. */
    uint8_t    co_refc;       /* Amount of reference variables used by this code. */
    uint8_t    co_argc_min;   /* Min amount of arguments required to execute this code. */
    uint8_t    co_stackmax;   /* The greatest allowed stack depth of the assembly associated with this code. */
    uint16_t   co_staticoff;  /* Absolute file offset to a static/constant variable descriptor table (`Dec_Objects'). */
    uint16_t   co_exceptoff;  /* Absolute file offset to an exception handler descriptor table (`Dec_8BitCodeExceptions'). */
    uint16_t   co_defaultoff; /* Absolute file offset to an argument-default descriptor table (`Dec_Objects').
                               * NOTE: When ZERO(0), there are no default arguments, but if not,
                               *      `co_argc_max' can be calculated from `co_argc_min+os_len' */
    uint16_t   co_ddioff;     /* Absolute file offset to optional code debug information (`Dec_8BitCodeDDI').
                               * NOTE: When ZERO(0), there is no DDI information. */
    uint16_t   co_kwdoff;     /* Absolute file offset to optional keyword argument information (`Dec_8BitCodeKwds')
                               * NOTE: When ZERO(0), there is no keyword information. */
    uint16_t   co_textsiz;    /* Absolute size of this code's text section (in bytes) */
    uint16_t   co_textoff;    /* Absolute file offset to the assembly text that will be executed by this code. */
} Dec_8BitCode;

/* The following 2 structure are taken from /usr/include/i386-linux-gnu/ieee754.h */
/* Copyright (C) 1992-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
typedef union PACKED {
    double d;
    struct {
#ifdef CONFIG_BIG_ENDIAN
        unsigned int negative:1;
        unsigned int exponent:11;
        unsigned int mantissa0:20;
        unsigned int mantissa1:32;
#elif CONFIG_HOST_FLOAT_ENDIAN == 4321
        unsigned int mantissa0:20;
        unsigned int exponent:11;
        unsigned int negative:1;
        unsigned int mantissa1:32;
#else
        unsigned int mantissa1:32;
        unsigned int mantissa0:20;
        unsigned int exponent:11;
        unsigned int negative:1;
#endif
    } ieee;
    struct {
#ifdef CONFIG_BIG_ENDIAN
        unsigned int negative:1;
        unsigned int exponent:11;
        unsigned int quiet_nan:1;
        unsigned int mantissa0:19;
        unsigned int mantissa1:32;
#elif CONFIG_HOST_FLOAT_ENDIAN == 4321
        unsigned int mantissa0:19;
        unsigned int quiet_nan:1;
        unsigned int exponent:11;
        unsigned int negative:1;
        unsigned int mantissa1:32;
#else
        unsigned int mantissa1:32;
        unsigned int mantissa0:19;
        unsigned int quiet_nan:1;
        unsigned int exponent:11;
        unsigned int negative:1;
#endif
    } ieee_nan;
} Dec_host_ieee754_double;

typedef union PACKED {
    struct {
        unsigned int mantissa1:32;
        unsigned int mantissa0:20;
        unsigned int exponent:11;
        unsigned int negative:1;
    } ieee;
    struct {
        unsigned int mantissa1:32;
        unsigned int mantissa0:19;
        unsigned int quiet_nan:1;
        unsigned int exponent:11;
        unsigned int negative:1;
    } ieee_nan;
} Dec_ieee754_double;


typedef struct PACKED {
    uint16_t           co_name; /* Name of the operator (one of `OPERATOR_*') */
    uint16_t           co_addr; /* [<= :cd_cmemb_size] Index into the class member
                                 * table, to where the operator callback can be bound. */
} Dec_ClassOperator;

typedef struct PACKED {
    uint16_t           ca_addr;          /* Address of the attribute (behavior depends on flags) */
    uint16_t           ca_flags;         /* Attribute flags (Set of `CLASS_ATTRIBUTE_F*') */
    uint8_t            ca_nam[1];        /* Name of the attribute (Pointer to a ZERO-terminated string within the `e_stroff' string table)
                                          * NOTE: Decode using `Dec_DecodePointer()' */
    uint8_t            ca_doclen[1];     /* The length of the documentation string (when ZERO, there is doc-string)
                                          * NOTE: Decode using `Dec_DecodePointer()' */
    uint8_t            ca_doc[1];        /* Only exists when `ca_doclen' evaluates to non-zero:
                                          * The offset into `e_stroff' of the documentation string's start.
                                          * NOTE: Decode using `Dec_DecodePointer()' */
} Dec_ClassAttribute;

typedef struct PACKED {
    uint16_t           cd_flags;         /* Additional flags to set for the resulting type (set of `TP_F*'). */
    uint8_t            cd_nam[1];        /* Name of the class (Pointer to a ZERO-terminated string within the `e_stroff' string table)
                                          * NOTE: Decode using `Dec_DecodePointer()' */
    uint8_t            cd_doclen[1];     /* The length of the documentation string (when ZERO, there is doc-string)
                                          * NOTE: Decode using `Dec_DecodePointer()' */
    uint8_t            cd_doc[1];        /* Only exists when `cd_doclen' evaluates to non-zero:
                                          * The offset into `e_stroff' of the documentation string's start.
                                          * NOTE: Decode using `Dec_DecodePointer()' */
    uint16_t           cd_cmemb_size;    /* The allocation size of the class member table. */
    uint16_t           cd_imemb_size;    /* The allocation size of the instance member table. */
    uint16_t           cd_op_count;      /* Amount of user-defined operator bindings. */
    uint8_t            cd_cattr_count[1];/* Amount of class attributes (Decode using `Dec_DecodePointer()'). */
    uint8_t            cd_iattr_count[1];/* Amount of instance attributes (Decode using `Dec_DecodePointer()'). */
    Dec_ClassOperator  cd_op_list[1];    /* [cd_op_count] List of operator bindings. */
    Dec_ClassAttribute cd_cattr_list[1]; /* [cd_cattr_count] List of class attributes. */
    Dec_ClassAttribute cd_iattr_list[1]; /* [cd_iattr_count] List of instance attributes. */
} Dec_ClassDescriptor;

typedef struct PACKED {
    uint8_t                co_name; /* Name of the operator (one of `OPERATOR_*') */
    uint8_t                co_addr; /* [<= :cd_cmemb_size] Index into the class member
                                     * table, to where the operator callback can be bound. */
} Dec_8BitClassOperator;

typedef struct PACKED {
    uint8_t                ca_addr;          /* Address of the attribute (behavior depends on flags) */
    uint8_t                ca_flags;         /* Attribute flags (Set of `CLASS_ATTRIBUTE_F*') */
    uint8_t                ca_nam[1];        /* Name of the attribute (Pointer to a ZERO-terminated string within the `e_stroff' string table)
                                              * NOTE: Decode using `Dec_DecodePointer()' */
    uint8_t                ca_doclen[1];     /* The length of the documentation string (when ZERO, there is doc-string)
                                              * NOTE: Decode using `Dec_DecodePointer()' */
    uint8_t                ca_doc[1];        /* Only exists when `ca_doclen' evaluates to non-zero:
                                              * The offset into `e_stroff' of the documentation string's start.
                                              * NOTE: Decode using `Dec_DecodePointer()' */
} Dec_8BitClassAttribute;

typedef struct PACKED {
    uint8_t                cd_flags;         /* Additional flags to set for the resulting type (set of `TP_F*'). */
    uint8_t                cd_nam[1];        /* Name of the class (Pointer to a ZERO-terminated string within the `e_stroff' string table)
                                              * NOTE: Decode using `Dec_DecodePointer()' */
    uint8_t                cd_doclen[1];     /* The length of the documentation string (when ZERO, there is doc-string)
                                              * NOTE: Decode using `Dec_DecodePointer()' */
    uint8_t                cd_doc[1];        /* Only exists when `cd_doclen' evaluates to non-zero:
                                              * The offset into `e_stroff' of the documentation string's start.
                                              * NOTE: Decode using `Dec_DecodePointer()' */
    uint8_t                cd_cmemb_size;    /* The allocation size of the class member table. */
    uint8_t                cd_imemb_size;    /* The allocation size of the instance member table. */
    uint8_t                cd_op_count;      /* Amount of user-defined operator bindings. */
    uint8_t                cd_cattr_count;   /* Amount of class attributes. */
    uint8_t                cd_iattr_count;   /* Amount of instance attributes. */
    Dec_8BitClassOperator  cd_op_list[1];    /* [cd_op_count] List of operator bindings. */
    Dec_8BitClassAttribute cd_cattr_list[1]; /* [cd_cattr_count] List of class attributes. */
    Dec_8BitClassAttribute cd_iattr_list[1]; /* [cd_iattr_count] List of instance attributes. */
} Dec_8BitClassDescriptor;


typedef struct PACKED {
    uint8_t       kwe_nam[1];    /* Name of the keyword (Pointer to a ZERO-terminated string within the `e_stroff' string table)
                                  * NOTE: Decode using `Dec_DecodePointer()' */
} Dec_KwdsEntry;
typedef struct PACKED {
    uint8_t       kw_siz;        /* The amount of keyword entries. */
    Dec_KwdsEntry kw_members[1]; /* [kw_siz] One entry for each member.
                                  * NOTE: The keyword index (`struct kwds_entry::ke_index')
                                  *       is the index into this vector. */
} Dec_Kwds;





/* Decode a DEC pointer into an offset (usually into the string table)
 * HINT: The DEC pointer encoding format is
 *       identical to ULEB encoding used by DDI. */
LOCAL uint32_t DCALL
Dec_DecodePointer(uint8_t **__restrict pptr) {
 uint32_t result = 0; uint8_t byte;
 uint8_t *ptr = *pptr;
 uint8_t num_bits = 0;
 do {
  byte      = *ptr++;
  result   |= (byte & 0x7f) << num_bits;
  num_bits += 7;
 } while (byte & 0x80);
 *pptr = ptr;
 return result;
}





/* Object type IDs within a compiled source file (One of DTYPE_*).
 * NOTE: Unless otherwise stated, all operands are encoded in little-endian. */
#define DTYPE_NONE       0x00   /* +0   `Dee_None'       -- Simply the none builtin object (no operand) */
#define DTYPE_IEEE754    0x01   /* +8   `DeeFloat_Type'  -- 64-bit IEEE754 double-precision floating point number (`Dec_ieee754_double')
                                 *                         (byteorder: little-endian, float-word-order: little-endian). */
#define DTYPE_SLEB       0x02   /* +n   `DeeInt_Type'    -- Variable-length, signed immediate integer (s.a. `READ_SLEB()') */
#define DTYPE_ULEB       0x03   /* +n   `DeeInt_Type'    -- Variable-length, unsigned immediate integer (s.a. `READ_ULEB()') */
#define DTYPE_STRING     0x04   /* +n   `DeeString_Type' -- Followed by 1/2 `Dec_DecodePointer()' immediate values, the first being
                                 *                          the length of the string and the second being an offset into `e_stroff'.
                                 *                          Note however that if the first value equals ZERO(0), the second doesn't exist.
                                 *                          NOTE: Unlike debug-string (which are encoded as LATIN-1), text associated
                                 *                                with string objects is encoded as UTF-8 (allowing for full unicode) */
#define DTYPE_TUPLE      0x05   /* +n   `DeeTuple_Type'  -- Followed by `Dec_DecodePointer()' describing the length, then length more `DTYPE_*' codes for each element. */
#define DTYPE_LIST       0x06   /* +n   `DeeList_Type'   -- Followed by `Dec_DecodePointer()' describing the length, then length more `DTYPE_*' codes for each element. */
#define DTYPE_CLASSDESC  0x07   /* +n   `DeeClassDescriptor_Type' -- Followed by an immediate `Dec_8BitClassDescriptor' structure. */
#define DTYPE_FUNCTION   0x08   /* +n   `DeeFunction_Type' -- An immediate `Dec_Code' data structure defining a new code object, packed within a function object. - Following this, referenced objects are stored in-line. */
#define DTYPE_KWDS       0x09   /* +n   `DeeKwds_Type'   -- An immediate `Dec_Kwds' data structure defining a new keywords object. */
/*      DTYPE_           0x0a    */
/*      DTYPE_           0x0b    */
/*      DTYPE_           0x0c    */
#define DTYPE_NULL       0x0d   /* +0   `NULL'           -- A placeholder/NULL object. (Not allowed in all contexts) */
#define DTYPE_CODE       0x0e   /* +n   `DeeCode_Type'   -- An immediate `Dec_Code' data structure defining a new code object. */
#define DTYPE_EXTENDED   0x0f   /* +1+n `?'              -- Followed by an extended DTYPE-code (One of `DTYPE16_*'). */

/* All remaining single-byte type codes refer to individual builtin objects.
 * NOTE: Which set of builtins is made available depends on the
 *      `DBUILTINS_*' that was specified in `e_builtinset'. */
#define DTYPE_ISBUILTIN(x) ((x) > 0x0f)
#define DTYPE_BUILTIN_MIN  0x10 /* Lowest builtin object id. */
#define DTYPE_BUILTIN_MAX  0xff /* Greatest builtin object id. */
#define DTYPE_BUILTIN_NUM  0xf0 /* Max number of single-byte builtin object codes. */

/* 16-bit DEC-type codes. */
#define DTYPE16_NONE        0x0f00 /* Same as `DTYPE_NONE' */
/*      DTYPE16_            0x0f01  */
/*      DTYPE16_            0x0f02  */
/*      DTYPE16_            0x0f03  */
/*      DTYPE16_            0x0f04  */
#define DTYPE16_HASHSET     0x0f05 /* +n     `DeeHashSet_Type'     -- Followed by `Dec_DecodePointer()' describing the length, then length more `DTYPE_*' codes for each element. */
#define DTYPE16_DICT        0x0f06 /* +n     `DeeDict_Type'        -- Followed by `Dec_DecodePointer()' describing the length, then length*2 more `DTYPE_*' codes for each key/item pair (the key appearing first). */
#define DTYPE16_CLASSDESC   0x0f07 /* +n     `DeeClassDescriptor_Type' -- Followed by an immediate `Dec_ClassDescriptor' structure. */
#define DTYPE16_ROSET       0x0f08 /* +n     `DeeRoSet_Type'       -- Encoded the same way as `DTYPE16_HASHSET' */
#define DTYPE16_RODICT      0x0f09 /* +n     `DeeRoDict_Type'      -- Encoded the same way as `DTYPE16_DICT' */
/*      DTYPE16_            0x0f0a  */
/*      DTYPE16_            0x0f0b  */
/*      DTYPE16_            0x0f0c  */
/*      DTYPE16_            0x0f0d  */
/*      DTYPE16_            0x0f0e  */
#define DTYPE16_CELL        0x0f0d /* +n     `DeeCell_Type'  -- Followed either by `DTYPE_NULL' for an empty cell, or another object that is contained within. */
#define DTYPE16_EXTENDED    0x0f0f /* Reserved for future expansion. */
#define DTYPE16_BUILTIN_MIN 0x0f10 /* All extended type codes greater than this refer to a custom builtin SET id-0x10,
                                    * that is then followed by 1 more byte naming the object within that set:
                                    * >> 0x0f1142 // Use builtin object 0x42 from set #0x01  */



/* Builtin object set ids. */
#define DBUILTINS_NORMAL 0 /* The original, normal set of builtin objects is used. */
#define DBUILTINS_MAX    0 /* The greatest (currently) recognized builtin-set ID. */


#ifdef __COMPILER_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif




/* Deemon's implementation header of the DEC specifications. */
#ifdef CONFIG_BUILDING_DEEMON
#ifndef CONFIG_NO_DEC

/* Return the builtin object associated with `id'
 * within `set', return `NULL' when the given `set'
 * is unknown, or `id' is unassigned.
 * NOTE: The caller is responsible for passing an `id' that is located
 *       within the inclusive bounds `DTYPE_BUILTIN_MIN...DTYPE_BUILTIN_MAX'.
 *       This function internally asserts this and crashes if that is not the case. */
INTDEF DeeObject *DCALL Dec_GetBuiltin(uint8_t set, uint8_t id);

/* Return the ID of a given object.
 * If the given object isn't a builtin object, `DEC_BUILTINID_UNKNOWN' is returned.
 * The set and ID of the returned identifier can be extracted through
 * use of the `DEC_BUILTINID_SETOF' and `DEC_BUILTINID_IDOF' macros below.
 * To qualify as a builtin object, the object must be defined as an object
 * exported publicly by the deemon core, such that this includes the builtin
 * error types, as well as pretty much all other types found in deemon headers.
 * However there may be exceptions to this rule as there would be no point to
 * have an object like `DeeObjMethod_Type' or `DeeCMethod_Type' be made available
 * as a builtin object, considering access to those should happen through use of
 * the builtin `deemon' module.
 * The main idea behind builtin ids is to be able to encode the builtin error
 * types, because without a way of encoding them, we couldn't be using them
 * as compile-time exception masks in user-defined exception handlers (because
 * if we still did this, the resulting code object could not be saved to a DEC
 * compiled source files)
 * Note however that the compiler is very much aware of what can be used as a
 * builtin object, and will automatically prevent constant propagation if there
 * is no way of encoding the resulting object as a DEC constant expression. */
INTDEF uint16_t DCALL Dec_BuiltinID(DeeObject *__restrict obj);
#define DEC_BUILTINID_UNKNOWN     0 /* The object is not recognized as a builtin. */
#define DEC_BUILTINID_MAKE(setid,objid) ((setid) << 8 | (objid))
#define DEC_BUILTINID_SETOF(x)          (((x)&0xff00) >> 8)
#define DEC_BUILTINID_IDOF(x)            ((x)&0xff)


#define DECFILE_PADDING 32

struct module_object;
struct string_object;
struct compiler_options;
struct code_object;
struct ddi_object;

typedef struct {
    union {
        uint8_t               *df_data;    /* [0..df_size+DECFILE_PADDING][owned]
                                            *  A full mapping of all data from the input DEC file, followed
                                            *  by a couple of bytes of padding data that is ZERO-initialized. */
        uint8_t               *df_base;    /* [0..df_size] Base address of the DEC image mapped into host memory. */
        Dec_Ehdr              *df_ehdr;    /* [0..1] A pointer to the DEC file header mapped into host memory. */
    };
    size_t                     df_size;    /* Total number of usable bytes of memory
                                            * that can be found within the source file. */
    DREF struct string_object *df_name;    /* [1..1] The filename of the `*.dec' file opened by this descriptor. */
    DREF struct module_object *df_module;  /* [1..1] The module that is being loaded. */
    struct compiler_options   *df_options; /* [0..1] Compilation options. */
    DREF struct string_object *df_strtab;  /* [0..1] Lazily allocated copy of the string table.
                                            *        This string is used by DDI descriptors in
                                            *        order to allow for sharing of string tables. */
} DecFile;

/* Initialize a DEC file, given an input stream, as well as its pathname.
 * @return:  1: The given `input_stream' doesn't describe a valid DEC file.
 * @return:  0: Successfully initialized the DEC file.
 * @return: -1: An error occurred while attempting to read the DEC's data,
 *              or failed to allocate a sufficient buffer for the DEC. */
INTDEF int  DCALL DecFile_Init(DecFile *__restrict self,
                               DeeObject *__restrict input_stream,
                               struct module_object *__restrict module,
                               struct string_object *__restrict dec_pathname,
                               struct compiler_options *__restrict options);
INTDEF void DCALL DecFile_Fini(DecFile *__restrict self);

/* Return a string for the entire strtab of a given DEC-file.
 * Upon error, NULL is returned.
 * NOTE: The return value is _NOT_ a reference! */
INTDEF DeeObject *DCALL DecFile_Strtab(DecFile *__restrict self);

/* Check if a given DEC file is up to date, or if it must not be loaded.
 * because it a dependency has changed since it was created.
 * @return:  0: The file is up-to-date.
 * @return:  1: The file is not up-to-date.
 * @return: -1: An error occurred. */
INTDEF int DCALL DecFile_IsUpToDate(DecFile *__restrict self);

/* Load a given DEC file and fill in the given `module'.
 * @return:  0: Successfully loaded the given DEC file.
 * @return:  1: The DEC file has been corrupted or is out of date.
 * @return: -1: An error occurred. */
INTDEF int DCALL DecFile_Load(DecFile *__restrict self);

/* @return:  0: Successfully loaded the given DEC file.
 * @return:  1: The DEC file was out of date or had been corrupted.
 * @return: -1: An error occurred. */
INTDEF int DCALL
DeeModule_OpenDec(struct module_object *__restrict module,
                  DeeObject *__restrict input_stream,
                  struct compiler_options *__restrict options,
                  struct string_object *__restrict dec_pathname);

/* DEC loader implementation. */

/* @return: * :        A reference to the object that got loaded.
 * @return: NULL:      An error occurred. (NOTE: `DTYPE_NULL' is not allowed and indicates a corrupt file)
 * @return: ITER_DONE: The DEC file has been corrupted. */
INTDEF DREF DeeObject *DCALL
DecFile_LoadObject(DecFile *__restrict self,
                   uint8_t **__restrict preader);
/* @param: allow_dtype_null: When true, individual vector elements are allowed
 *                           to be `NULL' as the result of `DTYPE_NULL'
 * @return: * :              Newly heap-allocated vector of objects (length is stored in `*pcount').
 * @return: NULL:            An error occurred.
 * @return: ITER_DONE:       The DEC file has been corrupted. */
INTDEF DREF DeeObject **DCALL
DecFile_LoadObjectVector(DecFile *__restrict self,
                         uint16_t *__restrict pcount,
                         uint8_t **__restrict preader,
                         bool allow_dtype_null);

/* @return: * :        New reference to a code object.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The DEC file has been corrupted. */
INTDEF DREF struct code_object *DCALL
DecFile_LoadCode(DecFile *__restrict self,
                 uint8_t **__restrict preader);
/* @return: * :        New reference to a ddi object.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The DEC file has been corrupted. */
INTDEF DREF struct ddi_object *DCALL
DecFile_LoadDDI(DecFile *__restrict self,
                uint8_t *__restrict reader,
                bool is_8bit_ddi);


/* Return the last-modified time (in microseconds since 01.01.1970).
 * For this purpose, an internal cache is kept that is consulted
 * before and populated after making an attempt at contacting the
 * host operating system for the required information.
 * @return: * :           Last-modified time (in microseconds since 01.01.1970).
 * @return: 0 :           The given file could not be found.
 * @return: (uint64_t)-1: The lookup failed and an error was thrown. */
INTDEF uint64_t DCALL DecTime_Lookup(DeeObject *__restrict filename);

/* Return the current time (in microseconds since 01.01.1970) (never fails). */
INTDEF uint64_t DCALL DecTime_Now(void);

/* Try to free up memory from the dec time-cache. */
INTDEF size_t DCALL DecTime_ClearCache(size_t max_clear);

#endif /* !CONFIG_NO_DEC */
#endif /* CONFIG_BUILDING_DEEMON */


DECL_END

#endif /* !GUARD_DEEMON_DEC_H */
