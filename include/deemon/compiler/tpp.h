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
#ifndef GUARD_DEEMON_COMPILER_TPP_H
#define GUARD_DEEMON_COMPILER_TPP_H 1

#include "../api.h"

#ifdef CONFIG_BUILDING_DEEMON
#include "../string.h"

#ifdef GUARD_TPP_H
#error "Don't #include `tpp.h' directly. - Deemon much configure it for itself!"
#endif

DECL_BEGIN


#if 0 /* When defined, implement user-assembly as described
       * by `/lib/LANGUAGE.txt' for distributions lacking
       * inline-assembly support.
       *  - Still allow assembly for creation of artificial
       *    dependencies restricting the ast-based optimizer.
       *  - Cause a compiler error when the assembly text contains
       *    non-whitespace characters.
       */
#undef CONFIG_LANGUAGE_NO_ASM
#define CONFIG_LANGUAGE_NO_ASM 1
#endif



struct ast_loc;
struct ast;

/* Emit a compiler warning/error, given its TPP warning number.
 * The passed var-args are interpreted based on `wnum',
 * which is one of `W_*' defined by the lexer.
 * @return: -1: TPP had already been set to an error-state.
 * @return: -1: A fatal compiler error was thrown and TPP was set to an error-state.
 * @return:  0: The warning is being ignored.
 * @return:  0: The warning was printed, but is not considered dangerous.
 * @return:  0: The warning caused an error to be thrown, but the
 *              max number of compiler errors has yet to be reached. */
INTDEF ATTR_COLD int (parser_warnf)(int wnum, ...);
INTDEF ATTR_COLD int (parser_warnatf)(struct ast_loc *loc, int wnum, ...);
INTDEF ATTR_COLD int (parser_warnastf)(struct ast *__restrict loc_ast, int wnum, ...);

/* Similar to `parser_warnf()', but force the warning
 * to be fatal, regardless of its user-defined state.
 * @return: -1: Always returns -1. */
INTDEF ATTR_COLD int (parser_errf)(int wnum, ...);
INTDEF ATTR_COLD int (parser_erratf)(struct ast_loc *loc, int wnum, ...);
INTDEF ATTR_COLD int (parser_errastf)(struct ast *__restrict loc_ast, int wnum, ...);

DFUNDEF ATTR_COLD int DCALL Dee_BadAlloc(size_t req_bytes);

struct TPPFile;
struct TPPKeyword;

/* In order to prevent data redundancy of the library path (considering
 * that the way standard system include paths are now implemented as
 * `for (x: (module from deemon).paths) yield (joinpath from fs)(x,"include");'),
 * we simply implement the unknown-file hook of TPP, allowing us to search the
 * default library path for a given filename whenever TPP couldn't find the file
 * as part of its own library path. */
INTDEF struct TPPFile *DCALL tpp_unknown_file(int mode, char *__restrict filename, size_t filename_size,
                                              struct TPPKeyword **pkeyword_entry);

/* TPP isn't exported by deemon, so we configure it to only be used internally. */
#define TPPFUN                                  INTDEF
#define TPP(x)                                  x
#define TPPCALL                                 DCALL
#define TPP_USERDEFS                           <deemon/compiler/lexer.def>
#define TPP_CONFIG_ONELEXER                     2 /* Configure for one global lexer to speed things up. */
#define TPP_CONFIG_GCCFUNC                      0 /* Disable builtin GCC preprocessor functions. */
#define TPP_CONFIG_MINMACRO                     1 /* Enable minimal-macro mode, disabling all of those predefined C macros. */
#define TPP_CONFIG_USERSTREAMS                  1 /* Use `DeeFileObject *' as stream type for TPP. */
#define TPP_CONFIG_RAW_STRING_LITERALS          1 /* Enable support for raw string literals. */
#define TPP_USERSTREAM_TYPE                     struct file_object *
#define TPP_USERSTREAM_INVALID                  NULL
#define TPP_CONFIG_SET_API_ERROR                1 /* Get TPP to set errors on bad-alloc. */
#define TPP_CONFIG_SET_API_ERROR_BADALLOC       Dee_BadAlloc /* Get TPP to call this function on bad-alloc. */
#define TPP_CONFIG_NONBLOCKING_IO               1 /* Enable non-blocking I/O support. */
//#define TPP_CONFIG_NO_CALLBACK_PARSE_PRAGMA   1
#define TPP_CONFIG_NO_CALLBACK_PARSE_PRAGMA_GCC 1
#define TPP_CONFIG_NO_CALLBACK_INS_COMMENT      1
#define TPP_CONFIG_NO_CALLBACK_NEW_TEXTFILE     1
#define TPP_CONFIG_CALLBACK_UNKNOWN_FILE        tpp_unknown_file /* Statically link our unknown-file callback. */
#define TPP_CONFIG_CALLBACK_WARNING(...)       (parser_warnf(__VA_ARGS__) == 0)
#define TPP_CONFIG_FASTSTARTUP_KEYWORD_FLAGS    1
#define TPP_CONFIG_USERDEFINED_KWD_DEFAULT      1
#define TPP_CONFIG_USERDEFINED_KWD_ASSERT       1
#define TPP_CONFIG_USERDEFINED_KWD_IMPORT       1
#define TPP_CONFIG_USERDEFINED_KWD_IF           1


/* Configure non-variable TPP options. */
#if 1
/* #define TPP_CONFIG_FEATURE_TRIGRAPHS           0 */
/* #define TPP_CONFIG_FEATURE_DIGRAPHS            0 */
#define TPP_CONFIG_EXTENSION_GCC_VA_ARGS       1
#define TPP_CONFIG_EXTENSION_GCC_VA_COMMA      1
#define TPP_CONFIG_EXTENSION_GCC_IFELSE        1
#define TPP_CONFIG_EXTENSION_VA_COMMA          1
#define TPP_CONFIG_EXTENSION_VA_NARGS          1
#define TPP_CONFIG_EXTENSION_VA_ARGS           1
#define TPP_CONFIG_EXTENSION_STR_E             1
#define TPP_CONFIG_EXTENSION_ALTMAC            1
/* #define TPP_CONFIG_EXTENSION_RECMAC            0 */
#define TPP_CONFIG_EXTENSION_BININTEGRAL       1
/* #define TPP_CONFIG_EXTENSION_MSVC_PRAGMA       0 */
#define TPP_CONFIG_EXTENSION_STRINGOPS         1
#define TPP_CONFIG_EXTENSION_HASH_AT           1
#define TPP_CONFIG_EXTENSION_HASH_XCLAIM       1
#define TPP_CONFIG_EXTENSION_WARNING           1
#define TPP_CONFIG_EXTENSION_SHEBANG           1
#define TPP_CONFIG_EXTENSION_INCLUDE_NEXT      1
#define TPP_CONFIG_EXTENSION_IMPORT            1
#define TPP_CONFIG_EXTENSION_IDENT_SCCS        0
#define TPP_CONFIG_EXTENSION_BASEFILE          1
#define TPP_CONFIG_EXTENSION_INCLUDE_LEVEL     1
#define TPP_CONFIG_EXTENSION_COUNTER           1
#define TPP_CONFIG_EXTENSION_CLANG_FEATURES    1
#define TPP_CONFIG_EXTENSION_HAS_INCLUDE       1
#define TPP_CONFIG_EXTENSION_LXOR              0
#define TPP_CONFIG_EXTENSION_MULTICHAR_CONST   1
#define TPP_CONFIG_EXTENSION_DATEUTILS         1
#define TPP_CONFIG_EXTENSION_TIMEUTILS         1
#define TPP_CONFIG_EXTENSION_TIMESTAMP         1
#define TPP_CONFIG_EXTENSION_COLUMN            1
#define TPP_CONFIG_EXTENSION_TPP_EVAL          1
#define TPP_CONFIG_EXTENSION_TPP_UNIQUE        1
#define TPP_CONFIG_EXTENSION_TPP_LOAD_FILE     1
#define TPP_CONFIG_EXTENSION_TPP_COUNTER       1
#define TPP_CONFIG_EXTENSION_TPP_RANDOM        1
#define TPP_CONFIG_EXTENSION_TPP_STR_DECOMPILE 1
#define TPP_CONFIG_EXTENSION_TPP_STR_SUBSTR    1
#define TPP_CONFIG_EXTENSION_TPP_STR_SIZE      1
#define TPP_CONFIG_EXTENSION_TPP_STR_PACK      1
#define TPP_CONFIG_EXTENSION_TPP_COUNT_TOKENS  1
/* #define TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA   0 */
#define TPP_CONFIG_EXTENSION_ASSERTIONS        1
/* #define TPP_CONFIG_EXTENSION_CANONICAL_HEADERS 0 */
/* #define TPP_CONFIG_EXTENSION_EXT_ARE_FEATURES  0 */
#define TPP_CONFIG_EXTENSION_MSVC_FIXED_INT_DEFAULT 0 /* Default to disabled. */
/* #define TPP_CONFIG_EXTENSION_NO_EXPAND_DEFINED 0 */
#define TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR    1
/* #define TPP_CONFIG_EXTENSION_EXTENDED_IDENTS   0 */
/* #define TPP_CONFIG_EXTENSION_TRADITIONAL_MACRO 0 */
#else
#define TPP_CONFIG_FEATURE_TRIGRAPHS           0
#define TPP_CONFIG_FEATURE_DIGRAPHS            0
#define TPP_CONFIG_EXTENSION_GCC_VA_ARGS       0
#define TPP_CONFIG_EXTENSION_GCC_VA_COMMA      0
#define TPP_CONFIG_EXTENSION_GCC_IFELSE        0
#define TPP_CONFIG_EXTENSION_VA_COMMA          0
#define TPP_CONFIG_EXTENSION_VA_NARGS          0
#define TPP_CONFIG_EXTENSION_VA_ARGS           0
#define TPP_CONFIG_EXTENSION_STR_E             0
#define TPP_CONFIG_EXTENSION_ALTMAC            0
#define TPP_CONFIG_EXTENSION_RECMAC            0
#define TPP_CONFIG_EXTENSION_BININTEGRAL       0
#define TPP_CONFIG_EXTENSION_MSVC_PRAGMA       0
#define TPP_CONFIG_EXTENSION_STRINGOPS         0
#define TPP_CONFIG_EXTENSION_HASH_AT           0
#define TPP_CONFIG_EXTENSION_HASH_XCLAIM       0
#define TPP_CONFIG_EXTENSION_WARNING           0
#define TPP_CONFIG_EXTENSION_SHEBANG           0
#define TPP_CONFIG_EXTENSION_INCLUDE_NEXT      0
#define TPP_CONFIG_EXTENSION_IMPORT            0
#define TPP_CONFIG_EXTENSION_IDENT_SCCS        0
#define TPP_CONFIG_EXTENSION_BASEFILE          0
#define TPP_CONFIG_EXTENSION_INCLUDE_LEVEL     0
#define TPP_CONFIG_EXTENSION_COUNTER           0
#define TPP_CONFIG_EXTENSION_CLANG_FEATURES    0
#define TPP_CONFIG_EXTENSION_HAS_INCLUDE       0
#define TPP_CONFIG_EXTENSION_LXOR              0
#define TPP_CONFIG_EXTENSION_MULTICHAR_CONST   0
#define TPP_CONFIG_EXTENSION_DATEUTILS         0
#define TPP_CONFIG_EXTENSION_TIMEUTILS         0
#define TPP_CONFIG_EXTENSION_TIMESTAMP         0
#define TPP_CONFIG_EXTENSION_COLUMN            0
#define TPP_CONFIG_EXTENSION_TPP_EVAL          0
#define TPP_CONFIG_EXTENSION_TPP_UNIQUE        0
#define TPP_CONFIG_EXTENSION_TPP_LOAD_FILE     0
#define TPP_CONFIG_EXTENSION_TPP_COUNTER       0
#define TPP_CONFIG_EXTENSION_TPP_RANDOM        0
#define TPP_CONFIG_EXTENSION_TPP_STR_DECOMPILE 0
#define TPP_CONFIG_EXTENSION_TPP_STR_SUBSTR    0
#define TPP_CONFIG_EXTENSION_TPP_STR_SIZE      0
#define TPP_CONFIG_EXTENSION_TPP_STR_PACK      0
#define TPP_CONFIG_EXTENSION_TPP_COUNT_TOKENS  0
#define TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA   0
#define TPP_CONFIG_EXTENSION_ASSERTIONS        0
#define TPP_CONFIG_EXTENSION_CANONICAL_HEADERS 0
#define TPP_CONFIG_EXTENSION_EXT_ARE_FEATURES  0
#define TPP_CONFIG_EXTENSION_MSVC_FIXED_INT    0
#define TPP_CONFIG_EXTENSION_NO_EXPAND_DEFINED 0
#define TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR    0
#define TPP_CONFIG_EXTENSION_EXTENDED_IDENTS   0
#define TPP_CONFIG_EXTENSION_TRADITIONAL_MACRO 0
#endif

DECL_END

/* TODO: Use `DeeStringObject *' for `struct TPPString' */
/* TODO: Use `DeeObject *' (String/Int) for `struct TPPConst' */
#include "../../../src/tpp/tpp.h"

DECL_BEGIN


#ifdef __INTELLISENSE__
struct TPPToken token;
tok_t tok;
tok_t yield(void);
tok_t yieldnb(void);
tok_t yieldnbif(bool allow);
#else
#define token            TPPLexer_Global.l_token
#define tok              TPPLexer_Global.l_token.t_id
#define yield()          TPPLexer_Yield()
#define yieldnb()        TPPLexer_YieldNB()
#define yieldnbif(allow) ((allow) ? TPPLexer_YieldNB() : TPPLexer_Yield())
#endif
#define HAS(ext)         TPPLexer_HasExtension(ext)
#define WARN(...)        parser_warnf(__VA_ARGS__)
#define WARNAT(loc,...)  parser_warnatf(loc,__VA_ARGS__)
#define WARNAST(ast,...) parser_warnastf(ast,__VA_ARGS__)
#define PERR(...)        parser_errf(__VA_ARGS__)
#define PERRAT(loc,...)  parser_erratf(loc,__VA_ARGS__)
#define PERRAST(ast,...) parser_errastf(ast,__VA_ARGS__)
#define TPP_PUSHF()      do{uint32_t _old_flags = TPPLexer_Current->l_flags
#define TPP_BREAKF()     TPPLexer_Current->l_flags = _old_flags
#define TPP_POPF()       TPPLexer_Current->l_flags = _old_flags;}__WHILE0


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define parser_warnf(...)            __builtin_expect(parser_warnf(__VA_ARGS__),0)
#define parser_warnatf(loc,...)      __builtin_expect(parser_warnatf(loc,__VA_ARGS__),0)
#define parser_warnastf(loc_ast,...) __builtin_expect(parser_warnastf(loc_ast,__VA_ARGS__),0)
#endif
#endif



#define SKIP_WRAPLF(iter,end) \
 (*(iter) == '\\' && (iter) != (end)-1\
  ? ((iter)[1] == '\n' ? ((iter) += 2,1) :\
     (iter)[1] == '\r' ? ((iter) += \
    ((iter) != (end)-2 && (iter)[2] == '\n') ? 3 : 2,1)\
  : 0) : 0)
#define SKIP_WRAPLF_REV(iter,begin) \
 ((iter)[-1] == '\n' && (iter) != (begin)+1 \
  ? ((iter)[-2] == '\\' ? ((iter) -= 2,1) : \
    ((iter)[-2] == '\r' && (iter) != (begin)+2 && (iter)[-3] == '\\') ? ((iter) -= 3,1) \
  : 0) : (((iter)[-1] == '\r' && (iter) != (begin)+1 && (iter)[-2] == '\\') \
  ? ((iter) -= 2,1) : 0))

INTDEF struct TPPKeyword TPPKeyword_Empty;
INTDEF struct TPPKeyword *DCALL tok_without_underscores(void);
INTDEF char *DCALL peek_next_token(struct TPPFile **tok_file);
INTDEF char *DCALL peek_next_advance(char *p, struct TPPFile *__restrict *tok_file);
INTDEF struct TPPKeyword *DCALL peek_keyword(struct TPPFile *__restrict tok_file, char *__restrict tok_begin, int create_missing);
INTDEF struct TPPKeyword *DCALL peek_next_keyword(int create_missing);
INTDEF char *DCALL advance_wraplf(char *__restrict p);

DECL_END
#else /* CONFIG_BUILDING_DEEMON */

DECL_BEGIN
struct TPPKeyword;
DECL_END

#endif /* !CONFIG_BUILDING_DEEMON */

#endif /* !GUARD_DEEMON_COMPILER_TPP_H */
