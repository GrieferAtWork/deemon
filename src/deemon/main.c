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
#ifndef GUARD_MAIN_C
#define GUARD_MAIN_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/dec.h>
#include <deemon/compiler/lexer.h>
#include <deemon/error.h>
#include <deemon/thread.h>
#include <deemon/dex.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/gc.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/tuple.h>

#include <string.h>
#include <stdio.h>
#ifndef CONFIG_NO_STDLIB
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#endif

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif

#include "cmdline.h"
#include "runtime/runtime_error.h"

#if !defined(__USE_KOS)
#define strend(s) ((s)+strlen(s))
#endif

DECL_BEGIN

/* TODO: Change the type-hint syntax in doc texts from
 *      `foo(int x) -> bool' to `foo(x: int): bool'
 *       That way, we can have this syntax also be used
 *       by user-function, which can then merge prototype
 *       documentation with actual implementation:
 *    >> function add(x: int, y: int): int -> x+y;
 *       Implicit doc-text:
 *    >> @"(x:int,y:int):int"
 */

#if defined(_MSC_FULL_VER)
/* >> http://stackoverflow.com/questions/70013/how-to-detect-if-im-compiling-code-with-visual-studio-2008 */
#if _MSC_VER <= 1100
#   define DEE_VC_VERSION " 5.0"
#elif _MSC_VER <= 1200
#   define DEE_VC_VERSION " 6.0"
#elif _MSC_VER <= 1300
#   define DEE_VC_VERSION " 7.0"
#elif _MSC_VER <= 1310
#   define DEE_VC_VERSION " 7.1 (VS 2003)"
#elif _MSC_VER <= 1400
#   define DEE_VC_VERSION " 8.0 (VS 2005)"
#elif _MSC_VER <= 1500
#   define DEE_VC_VERSION " 9.0 (VS 2008)"
#elif _MSC_VER <= 1600
#   define DEE_VC_VERSION " 10.0 (VS 2010)"
#elif _MSC_VER <= 1700
#   define DEE_VC_VERSION " 11.0 (VS 2012)"
#elif _MSC_VER <= 1800
#   define DEE_VC_VERSION " 12.0 (VS 2013)"
#elif _MSC_VER <= 1900
#   define DEE_VC_VERSION " 14.0 (VS 2015)"
#else
#   define DEE_VC_VERSION ""
#endif
#ifdef __cplusplus
#   define DEE_COMPILER_CXX "++"
#else
#   define DEE_COMPILER_CXX ""
#endif
#   define DEE_COMPILER "VC" DEE_COMPILER_CXX DEE_VC_VERSION " (" PP_STR(_MSC_FULL_VER) ")"
#elif defined(__clang__)
#   define DEE_COMPILER "clang" DEE_COMPILER_CXX " " PP_STR(__clang__)
#elif defined(__GNUC__) && defined(__cplusplus)
#   define DEE_COMPILER "g++ " PP_STR(__GNUC__) "." PP_STR(__GNUC_MINOR__) "." PP_STR(__GNUC_PATCHLEVEL__)
#elif defined(__GNUC__) && !defined(__cplusplus)
#   define DEE_COMPILER "gcc " PP_STR(__GNUC__) "." PP_STR(__GNUC_MINOR__) "." PP_STR(__GNUC_PATCHLEVEL__)
#elif defined(__TINYC__)
#   define DEE_COMPILER "tcc " PP_STR(__TINYC__)
#else
#   define DEE_COMPILER "Unknown c" DEE_COMPILER_CXX " compiler"
#endif


PRIVATE char const str_usage[] =
"Usage: deemon [options...] <infile> [args...]\n"
"       deemon [options...] -i\n"
;
PRIVATE char const str_minhelp[] =
"See `deemon --help' for more help\n"
;
PRIVATE char const str_version[] =
"["
#ifndef NDEBUG
"DEBUG|"
#endif /* !NDEBUG */
#ifdef __TPP_VERSION__
"TPP " PP_STR(__TPP_VERSION__) "|"
#endif /* __TPP_VERSION__ */
DEE_COMPILER "|"
#ifdef __TIME__
__TIME__ "|"
#endif /* __TIME__ */
#ifdef __DATE__
__DATE__ "|"
#endif /* __DATE__ */
#ifdef CONFIG_HOST_UNIX
"posix|"
#elif defined(CONFIG_HOST_WINDOWS)
"windows|"
#endif
#ifdef CONFIG_HAVE_EXEC_ASM
"asm|"
#endif
#ifdef __x86_64__
"x86_64|"
#elif defined(__i686__)
"i686|"
#elif defined(__i586__)
"i586|"
#elif defined(__i486__)
"i486|"
#elif defined(__i386__)
"i386|"
#elif defined(__arm__)
"arm|"
#endif
#ifdef CONFIG_LITTLE_ENDIAN
"little-endian"
#elif defined(CONFIG_BIG_ENDIAN)
"big-endian"
#else
PP_STR(CONFIG_HOST_ENDIAN) "-endian"
#endif
"]\n"
"deemon    version "
PP_STR(DEE_VERSION_API) "/"
PP_STR(DEE_VERSION_COMPILER) ":"
PP_STR(DEE_VERSION_REVISION)           " -  Deemon Compiler  - "
"Copyright (C) 2016-2018 Griefer@Work\n"
"tpp       version "
PP_STR(TPP_API_VERSION) "/"
PP_STR(TPP_PREPROCESSOR_VERSION) "  "  " - Tiny PreProcessor - "
"Copyright (C) 2015-2018 Griefer@Work\n"
"\tGitub: https://github.com/GrieferAtWork/deemon\n"
"\n\n"
;


/* Deemon operational mode codes. */
#define OPERATION_MODE_RUNSCRIPT   0 /* Default: run a user-script. */
#define OPERATION_MODE_PRINTPP     1 /* Print preprocessor output. */
#define OPERATION_MODE_PRINTASM    2 /* Print deemon assembly for a user-script. */
#define OPERATION_MODE_FORMAT      3 /* Scan for format comment blocks and expand them. */
#define OPERATION_MODE_BUILDONLY   4 /* Only build the source file, but don't execute it. */
#define OPERATION_MODE_INTERACTIVE 5 /* Read, compile, and execute sourcecode interactively from the stdin. */

/* The effective operation mode (One of `OPERATION_MODE_*') */
PRIVATE uint8_t operation_mode = OPERATION_MODE_RUNSCRIPT;


/* Operational flags and state for `OPERATION_MODE_PRINTPP' operations mode. */
#define EMITPP_FNORMAL         0x0000 /* Normal preprocessor flags. */
#define EMITPP_MOUTLINE        0x000f /* Token outline mode mask. */
#define EMITPP_FOUTLINE_NORMAL 0x0000 /* Normal (no) token outlining. */
#define EMITPP_FOUTLINE_TOK    0x0001 /* Outline with [...] */
#define EMITPP_FOUTLINE_ZERO   0x0002 /* Separate with `\0' */
#define EMITPP_FNOLINE         0x0100 /* Do not emit #line directives. */
#define EMITPP_FNOCXXLINE      0x0200 /* Emit #line directives using the STD-C notation of `#line ...', rather than `# ...' */
#define EMITPP_FATLINEFEED     0x1000 /* The preprocessor has last emit a linefeed. */
#define EMITPP_FNODECODETOK    0x2000 /* Don't decode stuff like escape sequences and trigraphs before writing to out. */
#define EMITPP_FMAGICTOKENS    0x4000 /* Enable ~magic~ tokens for small line-shifts to prevent a #line being emit. */
PRIVATE uint16_t emitpp_state = EMITPP_FNORMAL;
PRIVATE DREF DeeObject *emitpp_dpout = NULL; /* Output stream for source dependencies. */



INTDEF struct compiler_options import_options; /* Options used to compile imported libraries. */
INTDEF struct compiler_options script_options; /* Options used to compile the user-script. */

PRIVATE int DCALL compiler_setup(void *arg);
PRIVATE int DCALL error_handler(struct compiler_error_object *__restrict error, int fatality_mode, void *arg);
PRIVATE int DCALL cmd_version(char *arg);
PRIVATE int DCALL cmd_help(char *arg);
PRIVATE int DCALL cmd_o(char *arg) {
 Dee_XDecref(script_options.co_decoutput);
 if (strcmp(arg,"-") == 0)
  /* Special case: output to stdout */
  script_options.co_decoutput = DeeFile_GetStd(DEE_STDOUT);
 else {
  script_options.co_decoutput = DeeFile_OpenString(arg,
                                                   OPEN_FWRONLY|OPEN_FCREAT,
                                                   644);
 }
 if unlikely(!script_options.co_decoutput)
    return -1;
 return 0;
}
PRIVATE int DCALL cmd_O(char *arg) {
 int level = atoi(arg);
 script_options.co_optimizer &= ~(OPTIMIZE_FENABLED|OPTIMIZE_FCSE);
 script_options.co_assembler &= ~(ASM_FSTACKDISP|ASM_FPEEPHOLE|
                                  ASM_FOPTIMIZE|ASM_FREUSELOC|ASM_FNODDI);
 /* Level #4: Disable features that hinder optimization (i.e. debug info) */
 if (level >= 4) script_options.co_assembler |= ASM_FNODDI,
                 script_options.co_optimizer |= OPTIMIZE_FCSE; /* CSE results in somewhat obscured DDI
                                                                * info, so we only enable it at level#4 */
 /* Level #3: Enable the AST-level optimization pass.
  *        -> This is mainly where constant propagation is implemented,
  *           among other, minor optimizations such as double-casts to
  *           known types
  */
 if (level >= 3) script_options.co_optimizer |= OPTIMIZE_FENABLED,
                 script_options.co_assembler |= ASM_FREUSELOC;
 /* Level #2: Enable initialization-is-allocation for __stack variable & peephole optimization.
  *        -> Note that peephole also implements dead-code elimination, as well
  *           as various other optimizations, such as elimination or variable
  *           reads/writes, among other things.
  *           However, peephole is greatly restricted by debug information where the
  *           existence of DDI checkpoints prevents inter-opcode optimizations. */
 if (level >= 2) script_options.co_assembler |= ASM_FSTACKDISP|ASM_FPEEPHOLE;
 /* Level #1: Enable general assembly optimizations (mainly affects automatic
  *           instruction width selection, used to minimize assembly size). */
 if (level >= 1) script_options.co_assembler |= ASM_FOPTIMIZE;
 return 0;
}
PRIVATE int DCALL cmd_i(char *UNUSED(arg)) {
 operation_mode = OPERATION_MODE_INTERACTIVE;
 script_options.co_parser |= PARSE_FLFSTMT;
 return 0;
}
PRIVATE int DCALL cmd_E(char *UNUSED(arg)) { operation_mode = OPERATION_MODE_PRINTPP; return 0; }
PRIVATE int DCALL cmd_S(char *UNUSED(arg)) { operation_mode = OPERATION_MODE_PRINTASM; return 0; }
PRIVATE int DCALL cmd_F(char *UNUSED(arg)) { operation_mode = OPERATION_MODE_FORMAT; return 0; }
PRIVATE int DCALL cmd_P(char *UNUSED(arg)) { emitpp_state |= EMITPP_FNOLINE; return 0; }
PRIVATE int DCALL cmd_c(char *UNUSED(arg)) {
 operation_mode = OPERATION_MODE_BUILDONLY;
 script_options.co_assembler &= ~ASM_FNODEC;
 return 0;
}
PRIVATE int DCALL cmd_ppC(char *UNUSED(arg)) { TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTCOMMENTS; return 0; }
PRIVATE int DCALL cmd_tok(char *UNUSED(arg)) { emitpp_state = (emitpp_state&~EMITPP_MOUTLINE)|EMITPP_FOUTLINE_TOK; return 0; }
PRIVATE int DCALL cmd_cpp(char *UNUSED(arg)) { TPPLexer_Current->l_flags &= ~(TPPLEXER_FLAG_NO_MACROS|TPPLEXER_FLAG_NO_DIRECTIVES|TPPLEXER_FLAG_NO_BUILTIN_MACROS); return 0; }
PRIVATE int DCALL cmd_nocpp(char *UNUSED(arg)) { TPPLexer_Current->l_flags |= (TPPLEXER_FLAG_NO_MACROS|TPPLEXER_FLAG_NO_DIRECTIVES|TPPLEXER_FLAG_NO_BUILTIN_MACROS); return 0; }
PRIVATE int DCALL cmd_undef(char *UNUSED(arg)) { TPPLexer_DisableExtension(EXT_SYSTEM_MACROS); return 0; }
PRIVATE int DCALL cmd_trigraphs(char *UNUSED(arg)) { TPPLexer_EnableExtension(EXT_TRIGRAPHS); return 0; }
PRIVATE int DCALL cmd_traditional(char *UNUSED(arg)) { TPPLexer_EnableExtension(EXT_TRADITIONAL_MACRO); TPPLexer_Current->l_extokens |= TPPLEXER_TOKEN_EQUALBINOP; return 0; }

PRIVATE int DCALL cmd_L(char *arg) {
 int result;
 DREF DeeObject *path = DeeString_New(arg);
 if unlikely(!path) return -1;
 result = DeeList_Append(DeeModule_GetPath(),path);
 Dee_Decref(path);
 return result;
}
PRIVATE int DCALL cmd_pp(char *UNUSED(arg)) {
 TPPLexer_Current->l_flags &= ~(TPPLEXER_FLAG_WANTSPACE|TPPLEXER_FLAG_WANTLF);
 emitpp_state = (emitpp_state&~EMITPP_MOUTLINE)|EMITPP_FOUTLINE_ZERO;
 return 0;
}
PRIVATE int DCALL cmd_ftabstop(char *arg) {
 script_options.co_tabwidth = (uint16_t)atoi(arg);
 return 0;
}
PRIVATE int TPPCALL emitpp_reemit_pragma(void);
PRIVATE int DCALL cmd_f(char *arg) {
 bool disable = false;
 if (arg[0] == 'n' && arg[1] == 'o' && arg[2] == '-')
     disable = true,arg += 3;
 /* */if (!strcmp(arg,"spc"))            disable ? (TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTSPACE)
                                                 : (TPPLexer_Current->l_flags |=  TPPLEXER_FLAG_WANTSPACE);
 else if (!strcmp(arg,"lf"))             disable ? (TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF)
                                                 : (TPPLexer_Current->l_flags |=  TPPLEXER_FLAG_WANTLF);
 else if (!strcmp(arg,"comments"))       disable ? (TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTCOMMENTS)
                                                 : (TPPLexer_Current->l_flags |=  TPPLEXER_FLAG_WANTCOMMENTS);
 else if (!strcmp(arg,"magiclf"))        disable ? (emitpp_state &= ~EMITPP_FMAGICTOKENS)
                                                 : (emitpp_state |=  EMITPP_FMAGICTOKENS);
 else if (!strcmp(arg,"decode"))         disable ? (emitpp_state |=  EMITPP_FNODECODETOK)
                                                 : (emitpp_state &= ~EMITPP_FNODECODETOK);
 else if (!strcmp(arg,"longstring"))     disable ? (TPPLexer_Current->l_flags |=  TPPLEXER_FLAG_TERMINATE_STRING_LF)
                                                 : (TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_TERMINATE_STRING_LF);
 else if (!strcmp(arg,"unify-pragma"))   disable ? (TPPLexer_Current->l_callbacks.c_parse_pragma = NULL)
                                                 : (TPPLexer_Current->l_callbacks.c_parse_pragma = &emitpp_reemit_pragma);
 else if (!strcmp(arg,"unknown-pragma")) disable ? (TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_REEMIT_UNKNOWN_PRAGMA)
                                                 : (TPPLexer_Current->l_flags |=  TPPLEXER_FLAG_REEMIT_UNKNOWN_PRAGMA);
 else if (!strcmp(arg,"line"))           disable ? (emitpp_state |=  EMITPP_FNOLINE)
                                                 : (emitpp_state &= ~EMITPP_FNOLINE);
 else if (!strcmp(arg,"cppline"))        disable ? (emitpp_state |=  EMITPP_FNOCXXLINE)
                                                 : (emitpp_state &= ~EMITPP_FNOCXXLINE);
 else if (!TPPLexer_SetExtension(arg,!disable)) {
  DeeError_Throwf(&DeeError_ValueError,
                  "Unknown extension `%#q'",
                  arg);
  return -1;
 }
 return 0;
}
PRIVATE int DCALL cmd_W(char *arg) {
 wstate_t state = WSTATE_ERROR;
 int error;
 if (!strcmp(arg,"error")) TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WERROR;
 else if (!strcmp(arg,"system-headers")) TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WSYSTEMHEADERS;
 else if (!strcmp(arg,"all")) {
  unsigned int i;
  for (i = 0; i < W_COUNT; ++i)
   if (!TPPLexer_SetWarning(i,state))
        return -1;
 } else {
  if (arg[0] == 'n' && arg[1] == 'o' && arg[2] == '-')
      state = WSTATE_DISABLED,arg += 3;
  error = TPPLexer_SetWarnings(arg,state);
  if (!error) return -1;
  if (error == 2) {
   DeeError_Throwf(&DeeError_ValueError,
                   "Unknown warning `%#q'",
                   arg);
   return -1;
  }
 }
 return 0;
}
PRIVATE int DCALL cmd_D(char *arg) {
 char *macro_value;
 size_t name_length;
 ASSERT(arg);
 macro_value = strchr(arg,'=');
 if (macro_value)
      name_length = (size_t)((macro_value++)-arg);
 else macro_value = "1",name_length = strlen(arg);
 if (!TPPLexer_Define(arg,name_length,
                      macro_value,strlen(macro_value),
                      TPPLEXER_DEFINE_FLAG_NONE))
      return -1;
 return 0;
}
PRIVATE int DCALL cmd_U(char *arg) {
 ASSERT(arg);
 TPPLexer_Undef(arg,strlen(arg));
 return 0;
}
PRIVATE int DCALL cmd_I(char *arg) {
 ASSERT(arg);
 if (!TPPLexer_AddIncludePath(arg,strlen(arg)))
      return -1;
 return 0;
}
PRIVATE int DCALL cmd_A(char *arg) {
 bool add = true; char *val;
 if (*arg == '-') ++arg,add = false;
 val = strchr(arg,'=');
 /* */if (val) *val++ = '\0';
 else if (add) {
  DeeError_Throwf(&DeeError_ValueError,
                  "No assertion value given by `%#q'",
                  arg);
  return -1;
 }
 if (add && !TPPLexer_AddAssert(arg,strlen(arg),val,strlen(val)))
     return -1;
 TPPLexer_DelAssert(arg,strlen(arg),
                    val,val ? strlen(val) : 0);
 return 0;
}

PRIVATE int DCALL cmd_message_format(char *arg) {
 /* */if (strcmp(arg,"gcc") == 0)  TPPLexer_Current->l_flags &= ~(TPPLEXER_FLAG_MSVC_MESSAGEFORMAT);
 else if (strcmp(arg,"msvc") == 0) TPPLexer_Current->l_flags |= (TPPLEXER_FLAG_MSVC_MESSAGEFORMAT);
 else {
  DeeError_Throwf(&DeeError_ValueError,
                  "Unsupported message for `%s'",
                  arg);
  return -1;
 }
 return 0;
}
PRIVATE int DCALL cmdpp_name(char *arg) {
 Dee_XDecref(script_options.co_filename);
 script_options.co_filename = (DeeStringObject *)DeeString_New(arg);
 return script_options.co_filename ? 0 : -1;
}
PRIVATE int DCALL cmd_name(char *arg) {
 Dee_XDecref(script_options.co_rootname);
 script_options.co_rootname = (DeeStringObject *)DeeString_New(arg);
 return script_options.co_rootname ? 0 : -1;
}


struct compiler_flag {
    char const name[15]; /* The name of this flag. */
    uint8_t    inv;      /* Meaning of this flag is revered. */
    uint16_t   addr;     /* Offset into a `struct compiler_options' where this flag is located at. */
    uint16_t   flag;     /* The bit associated with this flag. */
};

#define FIELD(x) COMPILER_OFFSETOF(struct compiler_options,x)
PRIVATE struct compiler_flag const compiler_flags[] = {
    { "lfstmt",        0, FIELD(co_parser),    PARSE_FLFSTMT },
    { "ast-optimize",  0, FIELD(co_optimizer), OPTIMIZE_FENABLED },
    { "ast-predict",   1, FIELD(co_optimizer), OPTIMIZE_FNOPREDICT },
    { "ast-compare",   1, FIELD(co_optimizer), OPTIMIZE_FNOCOMPARE },
    { "ast-onepass",   0, FIELD(co_optimizer), OPTIMIZE_FONEPASS },
    { "cse",           0, FIELD(co_optimizer), OPTIMIZE_FCSE },
    { "bigcode",       0, FIELD(co_assembler), ASM_FBIGCODE },
    { "optimize",      0, FIELD(co_assembler), ASM_FOPTIMIZE },
    { "optimize-size", 0, FIELD(co_assembler), ASM_FOPTIMIZE_SIZE },
    { "reuse-locals",  0, FIELD(co_assembler), ASM_FREUSELOC },
    { "peephole",      0, FIELD(co_assembler), ASM_FPEEPHOLE },
    { "stackdisp",     0, FIELD(co_assembler), ASM_FSTACKDISP },
    { "ddi",           1, FIELD(co_assembler), ASM_FNODDI },
    { "assert",        1, FIELD(co_assembler), ASM_FNOASSERT },
    { "gendec",        1, FIELD(co_assembler), ASM_FNODEC },
    { "reuse-consts",  1, FIELD(co_assembler), ASM_FNOREUSECONST },
    { "imp-dec",       1, FIELD(co_decloader), DEC_FDISABLE },
    { "imp-outdated",  0, FIELD(co_decloader), DEC_FLOADOUTDATED },
    { "imp-trusted",   1, FIELD(co_decloader), DEC_FUNTRUSTED },
    { "dec-optsiz",    0, FIELD(co_decwriter), DEC_WRITE_FREUSE_GLOBAL }, /* Optimize-for-size. */
    { "dec-ddi",       1, FIELD(co_decwriter), DEC_WRITE_FNODEBUG },
    { "dec-doc",       1, FIELD(co_decwriter), DEC_WRITE_FNODOC },
    { "dec-bigfile",   0, FIELD(co_decwriter), DEC_WRITE_FBIGFILE },
};
#undef FIELD

PRIVATE int DCALL cmd_C(char *arg) {
 uint8_t disable = 0; unsigned int i;
 if (arg[0] == 'n' && arg[1] == 'o' && arg[2] == '-')
     disable = 1,arg += 3;
 for (i = 0; i < COMPILER_LENOF(compiler_flags); ++i) {
  if (strcmp(compiler_flags[i].name,arg)) continue;
  /* Invert the logical meaning of the flag. */
  disable ^= compiler_flags[i].inv;
  /* Set/Delete the associated flag bit. */
  disable ? (*(uint16_t *)((uintptr_t)&script_options+compiler_flags[i].addr) &= ~compiler_flags[i].flag)
          : (*(uint16_t *)((uintptr_t)&script_options+compiler_flags[i].addr) |=  compiler_flags[i].flag);
  return 0;
 }
 DeeError_Throwf(&DeeError_ValueError,
                 "Unknown compiler flag `%#q'",
                 arg);
 return -1;
}



PRIVATE char const doc_cmdc[]    = "\tOnly build (parse + compile) the given source file. Don't run it afterwards";
PRIVATE char const doc_cmdo[]    = "<name>\tRedirect output to a given file (defaults to STDOUT)";
PRIVATE char const doc_cmdO[]    = "level\tSet the optimization level as an integer `level' between 0 and 3 (default: 1)";
PRIVATE char const doc_cmdS[]    = "Emit deemon module assembly after compiling a user-script";
PRIVATE char const doc_cmdA[]    = "pred=answer\tDefine an assertion `pred' as `answer'\n"
                                   "-pred[=answer]\tDelete `answer' or all assertions previously made about `pred'";
PRIVATE char const doc_cmdf[]    = "[no-]<extension>\tEnable/Disable a given `extension' (s.a.: `--help extensions')";
PRIVATE char const doc_cmdW[]    = "[no-]<warning>\tEnable/Disable a given `warning' (s.a.: `--help warnings')";
PRIVATE char const doc_cmdname[] = "<name>\tSet the name of the main module";
PRIVATE char const doc_cmdi[]    = "Read, compile, and execute sourcecode interactively from the stdin";
PRIVATE char const doc_cmdE[]    = "Emit preprocessor output, rather than running a user-script";
PRIVATE char const doc_cmdP[]    = "Disable emission of #line adjustment directives (Default: on)";
PRIVATE char const doc_cmdD[]    = "sym[=val=1]\tDefines `sym' as `val'";
PRIVATE char const doc_cmdU[]    = "sym\tUndefine a previously defined symbol `sym'";
PRIVATE char const doc_cmdL[]    = "<path>\tAdd `path' to the system module search path (s.a.: `(module from deemon).path')";
PRIVATE char const doc_cmdI[]    = "<dir>\tAdd `dir' to the list of #include <...> paths";
PRIVATE char const doc_cmdtok[]  = "Outline all tokens using the [...] notation (Default: off)";
PRIVATE char const doc_cmdpp[]   = "Enable preprocess-mode, which emits all tokens separated by `\\0'-bytes\n"
                                   "Enabling this option also disabled SPACE and LF tokens, though\n"
                                   "they can be re-enabled using the `-fspc' and `-flf' switches";
#ifdef _MSC_VER
PRIVATE char const doc_cmdmessage_format[] = "={msvc|gcc}\tSet the format for error message (Default: msvc)";
#else
PRIVATE char const doc_cmdmessage_format[] = "={msvc|gcc}\tSet the format for error message (Default: gcc)";
#endif
PRIVATE char const doc_cmd_ftabstop[] = "=width\tSet the width of tab characters used by `__COLUMN__' and in warning/error messages (Default: " PP_STR(TPPLEXER_DEFAULT_TABSIZE) ")";
PRIVATE char const doc_cmd_undef[] = "Disable all builtin macros";
PRIVATE char const doc_cmd_trigraphs[] = "Enable recognition of trigraph character sequences";
PRIVATE char const doc_cmd_traditional[] = "Enable recognition of traditional tokens & macros (Default: off)";

PRIVATE struct cmd_option preprocessor_options[] = {
    { CMD_FARG|CMD_FARGIMM|CMD_FARGEQ, "", "name", { &cmdpp_name }, "<name>\tSet the name used for `__FILE__' and debug informations by `INFILE' (Useful when running in interactive mode)" },
    { CMD_FJOINABLE, "E", NULL, { &cmd_E }, doc_cmdE },
    { CMD_FJOINABLE, "P", NULL, { &cmd_P }, doc_cmdP },
    { CMD_FARG|CMD_FARGIMM, "o", NULL, { &cmd_o }, doc_cmdo },
    { CMD_FARG|CMD_FARGIMM|CMD_FARGEQ, "", "out", { &cmd_o }, doc_cmdo },
    { CMD_FARG|CMD_FARGIMM|CMD_FRUNLATER, "D", NULL, { &cmd_D }, doc_cmdD },
    { CMD_FARG|CMD_FARGIMM|CMD_FRUNLATER, "U", NULL, { &cmd_U }, doc_cmdU },
    { CMD_FARG|CMD_FARGIMM|CMD_FRUNLATER, "A", NULL, { &cmd_A }, doc_cmdA },
    { CMD_FARG|CMD_FARGIMM|CMD_FRUNLATER, "I", NULL, { &cmd_I }, doc_cmdI },
    { CMD_FNORMAL/*|CMD_FLONG1DASH*/, "", "tok", { &cmd_tok }, doc_cmdtok },
    { CMD_FNORMAL|CMD_FRUNLATER, "", "pp", { &cmd_pp }, doc_cmdpp },
    { CMD_FNORMAL|CMD_FLONG1DASH|CMD_FRUNLATER, "", "undef", { &cmd_undef }, doc_cmd_undef },
    { CMD_FNORMAL|CMD_FLONG1DASH|CMD_FRUNLATER, "", "cpp", { &cmd_cpp }, "Enable preprocessing" },
    { CMD_FNORMAL|CMD_FLONG1DASH|CMD_FRUNLATER, "", "nocpp", { &cmd_nocpp }, "Disable preprocessing" },
    { CMD_FNORMAL|CMD_FRUNLATER, "C", NULL, { &cmd_ppC }, "Do not discard comments" },
    { CMD_FARG|CMD_FARGIMM|CMD_FARGEQ|CMD_FRUNLATER, "", "message-format", { &cmd_message_format }, doc_cmdmessage_format },
    { CMD_FARG|CMD_FARGIMM|CMD_FARGEQ|CMD_FLONG1DASH, "", "ftabstop", { &cmd_ftabstop }, doc_cmd_ftabstop },
    { CMD_FNORMAL|CMD_FLONG1DASH|CMD_FRUNLATER, "", "trigraphs", { &cmd_trigraphs }, doc_cmd_trigraphs },
    { CMD_FNORMAL|CMD_FLONG1DASH|CMD_FRUNLATER, "", "traditional", { &cmd_traditional }, doc_cmd_traditional },
    { CMD_FNORMAL|CMD_FLONG1DASH|CMD_FRUNLATER, "", "traditional-cpp", { &cmd_traditional }, doc_cmd_traditional },
    { CMD_FARG|CMD_FARGIMM|CMD_FRUNLATER, "f", NULL, { &cmd_f }, doc_cmdf },
    { CMD_FARG|CMD_FARGIMM|CMD_FRUNLATER, "W", NULL, { &cmd_W }, doc_cmdW },
/*TODO:
                INDENT "-M                          Instead of emitting preprocessor output, emit a make-style list of dependencies.\n"
                INDENT "-MM                         Similar to `-M', but don't include system headers.\n"
                INDENT "-MD                         Like `-M', but don't disable preprocessing.\n"
                INDENT "-MMD                        Like `-MM', but don't disable preprocessing.\n"
                INDENT "-MG                         Disable preprocessing, but include missing files as dependencies, assuming they will be generated.\n"
                INDENT "-MP                         Emit dummy targets for every dependency.\n"
                INDENT "-MF <file>                  Enable dependency tracking and emit its output to <file>, but also preprocess regularly.\n"
                INDENT "-MT <target>                Specify the target object name used within the generated make dependency.\n"
                INDENT "-MQ <target>                Same as `-MT', but escape characters special to make, such as `$'.\n"
*/
    CMD_OPTION_SENTINEL
};
PRIVATE struct cmd_option assembler_options[] = {
    { CMD_FARG|CMD_FARGIMM, "O", NULL, { &cmd_O }, doc_cmdO },
    { CMD_FJOINABLE, "S", NULL, { &cmd_S }, doc_cmdS },
    CMD_OPTION_SENTINEL
};
PRIVATE struct cmd_option linker_options[] = {
    { CMD_FARG|CMD_FARGIMM|CMD_FARGEQ, "", "name", { &cmd_name }, doc_cmdname },
    { CMD_FARG|CMD_FARGIMM, "L", NULL, { &cmd_L }, doc_cmdL },
    { CMD_FARG|CMD_FARGIMM|CMD_FARGEQ, "", "library-path", { &cmd_L }, doc_cmdL },
    CMD_OPTION_SENTINEL
};

PRIVATE struct cmd_option cmdline_options[] = {

    /* Basic options. */
    { CMD_FNORMAL, "", "version", { &cmd_version }, "Display version information" },
    { CMD_FARG|CMD_FARGOPT, "", "help", { &cmd_help }, "[subject]\tDisplays this help, or help on the given `subject'" },
    { CMD_FJOINABLE, "F", NULL, { &cmd_F }, "Enable file formatting" },
    { CMD_FARG|CMD_FARGIMM, "C", NULL, { &cmd_C }, "[no-]<opt>\tEnable/disable a given compiler option <opt> (s.a. `--help compiler-options')" },

    /* Sub-option namespaces. */
    { CMD_FGROUP, "Wp", NULL, { preprocessor_options }, "Preprocessor-specific options" },
    { CMD_FGROUP, "Wa", NULL, { assembler_options }, "Assembler-specific options" },
    { CMD_FGROUP, "Wl", NULL, { linker_options }, "Linker-specific options" },

    /* Preprocessor-specific options that are promoted into the root commandline namespace. */
    { CMD_FJOINABLE, "E", NULL, { &cmd_E }, doc_cmdE },
    { CMD_FJOINABLE, "i", NULL, { &cmd_i }, doc_cmdi },
    { CMD_FJOINABLE, "P", NULL, { &cmd_P }, doc_cmdP },
    { CMD_FJOINABLE, "c", NULL, { &cmd_c }, doc_cmdc },
    { CMD_FARG|CMD_FARGIMM, "o", NULL, { &cmd_o }, doc_cmdo },
    { CMD_FARG|CMD_FARGIMM|CMD_FRUNLATER, "D", NULL, { &cmd_D }, doc_cmdD },
    { CMD_FARG|CMD_FARGIMM|CMD_FRUNLATER, "U", NULL, { &cmd_U }, doc_cmdU },
    { CMD_FARG|CMD_FARGIMM|CMD_FRUNLATER, "A", NULL, { &cmd_A }, doc_cmdA },
    { CMD_FARG|CMD_FARGIMM|CMD_FRUNLATER, "I", NULL, { &cmd_I }, doc_cmdI },
    { CMD_FLONG1DASH, "", "tok", { &cmd_tok }, doc_cmdtok },
    { CMD_FRUNLATER, "", "pp", { &cmd_pp }, doc_cmdpp },
    { CMD_FLONG1DASH, "", "undef", { &cmd_undef }, doc_cmd_undef },
    { CMD_FARG|CMD_FARGIMM|CMD_FARGEQ|CMD_FRUNLATER, "", "message-format", { &cmd_message_format }, doc_cmdmessage_format },
    { CMD_FARG|CMD_FARGIMM|CMD_FARGEQ, "", "ftabstop", { &cmd_ftabstop }, doc_cmd_ftabstop },
    { CMD_FLONG1DASH|CMD_FRUNLATER, "", "trigraphs", { &cmd_trigraphs }, doc_cmd_trigraphs },
    { CMD_FLONG1DASH|CMD_FRUNLATER, "", "traditional", { &cmd_traditional }, doc_cmd_traditional },
    { CMD_FLONG1DASH|CMD_FRUNLATER, "", "traditional-cpp", { &cmd_traditional }, doc_cmd_traditional },
    { CMD_FARG|CMD_FARGIMM|CMD_FRUNLATER, "f", NULL, { &cmd_f }, doc_cmdf },
    { CMD_FARG|CMD_FARGIMM|CMD_FRUNLATER, "W", NULL, { &cmd_W }, doc_cmdW },

    /* Assembler-specific options that are promoted into the root commandline namespace. */
    { CMD_FJOINABLE, "S", NULL, { &cmd_S }, doc_cmdS },
    { CMD_FARG|CMD_FARGIMM, "O", NULL, { &cmd_O }, doc_cmdO },

    /* Linker-specific options that are promoted into the root commandline namespace. */
    { CMD_FARG|CMD_FARGIMM|CMD_FARGEQ, "", "name", { &cmd_name }, doc_cmdname },
    { CMD_FARG|CMD_FARGIMM, "L", NULL, { &cmd_L }, doc_cmdL },

    CMD_OPTION_SENTINEL
};

PRIVATE int DCALL exit_ok(void) {
 DREF DeeObject *err;
 err = DeeObject_NewDefault(&DeeError_AppExit);
 if likely(err) {
  DeeError_Throw(err);
  Dee_Decref(err);
 }
 return -1;
}
PRIVATE int DCALL cmd_version(char *UNUSED(arg)) {
 DREF DeeObject *fp;
 fp = DeeFile_GetStd(DEE_STDERR);
 if unlikely(!fp) goto err_nofp;
 if (DeeFile_WriteAll(fp,str_version,COMPILER_STRLEN(str_version)) < 0)
     goto err;
 Dee_Decref(fp);
 return exit_ok();
err:
 Dee_Decref(fp);
err_nofp:
 return -1;
}
PRIVATE int DCALL cmd_help(char *arg) {
 struct cmd_option *help_option;
 DREF DeeObject *fp;
 fp = DeeFile_GetStd(DEE_STDERR);
 if unlikely(!fp) goto err_nofp;
 help_option = cmdline_options;
 (void)arg; /* TODO: Display help for a specific option. */
 if (DeeFile_WriteAll(fp,str_usage,COMPILER_STRLEN(str_usage)) < 0)
     goto err;
 (void)help_option;
 /* TODO: Display help on all options from `cmdline_options' */

 Dee_Decref(fp);
 return exit_ok();
err:
 Dee_Decref(fp);
err_nofp:
 return -1;
}


PRIVATE int DCALL compiler_setup(void *UNUSED(arg)) {
 /* Define a macro `__MAIN__' in order to indicate to script/module
  * hybrid source files that they are being executed as a script.
  * Using macros for this case is OK because when executed as a
  * script, the source file isn't normally cached as a dec-file,
  * meaning that it's OK if it does something different when used
  * in this manner. */
 if (!TPPLexer_Define("__MAIN__",8,"1",1,TPPLEXER_DEFINE_FLAG_NONE))
      return -1;

 /* - Add additional #include paths passed through `-I' */
 /* - Add pre-defined macros passed through `-D' */
 /* - Add pre-defined assertions passed through `-A' */
 /* - Set misc. lexer context/flags based on the commandline. */
 return cmd_runlate();
}
PRIVATE int DCALL
error_handler(struct compiler_error_object *__restrict error,
              int UNUSED(fatality_mode), void *UNUSED(arg)) {
 return DeeFile_PrintObjectNl(DeeFile_DefaultStddbg,(DeeObject *)error);
}
PRIVATE int DCALL operation_mode_printpp(int argc, char **argv);
PRIVATE int DCALL operation_mode_format(int argc, char **argv);




/* ==================================================================== *
 * --- MAIN()                                                           *
 * ==================================================================== */
int main(int argc, char *argv[]) {
#ifdef EXIT_SUCCESS
 int result = EXIT_SUCCESS;
#else
 int result = 0;
#endif
#ifdef __CYGWIN__
 /* Cygwin writes some garbage before passing control to main()
  * Since it fails to print a terminating line-feed, this is the
  * best we can do to at least prevent our first line from being
  * appended to its. */
 DEE_DPRINT("\n");
#endif
#ifdef CONFIG_HOST_WINDOWS
 /* Reset the default console-output code page to fix UTF-8 output.
  * See the explanation in `/src/deemon/system/win-file.c.inl:sysfile_write'
  * for why this needs to be done in order to get proper UTF-8 support. */
 SetConsoleOutputCP(GetOEMCP());
#endif

 /*_CrtSetBreakAlloc(280169);*/

 /* Literally the only deemon component that actually needs to
  * be initialized (and isn't already initialized statically):
  *  - The TLS variable that is used by `DeeThread_Self()'
  * To bad there's no cross-platform way to do this statically.
  * Else, this'd be so much simpler. */
 DeeThread_Init();

#ifndef CONFIG_NO_KEYBOARD_INTERRUPT
 /* Install the keyboard interrupt handler. */
 DeeError_InstallKeyboardInterrupt();
#endif /* !CONFIG_NO_KEYBOARD_INTERRUPT */

 /* TODO: Generate DDI information for argument, ref, and static symbol names. */

 /* Skip the first argument (the deemon executable name) */
 if (argc) --argc,++argv;

 if (!argc) {
  DREF DeeObject *fp; dssize_t temp;
  /* When no arguments were passed, print a short help-message and exit. */
  fp = DeeFile_GetStd(DEE_STDERR);
  if unlikely(!fp) goto err;
  temp = DeeFile_WriteAll(fp,str_usage,COMPILER_STRLEN(str_usage));
  if (temp >= 0)
      temp = DeeFile_WriteAll(fp,str_minhelp,COMPILER_STRLEN(str_minhelp));
  Dee_Decref(fp);
  if unlikely(temp < 0) goto err;
  goto done;
 }

 /* Parse the commandline. */
 if (cmd_parse(&argc,&argv,cmdline_options,false))
     goto err;

 if (operation_mode == OPERATION_MODE_PRINTPP) {
  /* Print preprocessor output. */
  if unlikely(!argc) goto err_no_input;
  result = operation_mode_printpp(argc,argv);
  if unlikely(result) goto err;
 } else if (operation_mode == OPERATION_MODE_FORMAT) {
  /* Format [[[deemon]]] tags. */
  if unlikely(!argc) goto err_no_input;
  result = operation_mode_format(argc,argv);
  if unlikely(result) goto err;
 } else {
  DREF DeeModuleObject *user_module;
  DREF DeeTupleObject *sys_argv; unsigned int i;

  /* Set interpreter arguments based on everything following the exec-script name. */
  sys_argv = (DREF DeeTupleObject *)DeeTuple_NewUninitialized((size_t)argc);
  if unlikely(!sys_argv) goto err;
  for (i = 0; i < (unsigned int)argc; ++i) {
   DREF DeeObject *arg = DeeString_New(argv[i]);
   if unlikely(!arg) {
    while (i--) Dee_Decref(DeeTuple_GET(sys_argv,i));
    DeeTuple_FreeUninitialized((DeeObject *)sys_argv);
    goto err;
   }
   DeeTuple_SET(sys_argv,i,arg); /* Inherit */
  }
  /* Set the system argument vector. */
  Dee_SetArgv((DeeObject *)sys_argv);
  if (operation_mode == OPERATION_MODE_INTERACTIVE) {
   DREF DeeObject *interactive_input;
   DREF DeeObject *interactive_module;
   DREF DeeObject *interactive_iterator;
   DREF DeeObject *interactive_output;
   DREF DeeObject *value;
   interactive_input = DeeFile_GetStd(DEE_STDIN);
   if unlikely(!interactive_input) goto err;
   interactive_output = script_options.co_decoutput;
   script_options.co_decoutput = NULL;
   /* Default to using `stderr' as output for interactive modules. */
   if (!interactive_output &&
       (interactive_output = DeeFile_GetStd(DEE_STDERR)) == NULL)
        goto err;
   interactive_module = DeeModule_OpenInteractiveString("STDIN",
                                                        NULL,
                                                        interactive_input,
                                                        0,
                                                        0,
                                                       &script_options,
                                                        MODULE_INTERACTIVE_MODE_FONLYBASEFILE|
                                                        MODULE_INTERACTIVE_MODE_FYIELDROOTEXPR,
                                                       (DeeObject *)sys_argv,
                                                        NULL);
   Dee_Decref(interactive_input);
   Dee_Decref(sys_argv);
   if unlikely(!interactive_module) {
    Dee_Decref(interactive_output);
    goto err;
   }
   /* Construct an iterator for the interactive module. */
   interactive_iterator = DeeObject_IterSelf(interactive_module);
   if unlikely(!interactive_iterator) {
    Dee_Decref(interactive_module);
    Dee_Decref(interactive_output);
    goto err;
   }
   for (;;) {
    int error = 0;
    /* Pull items from the interactive module. */
    value = DeeObject_IterNext(interactive_iterator);
    if (!ITER_ISOK(value)) break;
    if (DeeObject_PrintRepr(value,(dformatprinter)&DeeFile_WriteAll,interactive_output) < 0 ||
        DeeFile_WriteAll(interactive_output,"\n",1*sizeof(char)) < 0)
        error = -1;
    Dee_Decref(value);
    if unlikely(error) { value = NULL; break; }
   }
   Dee_Decref(interactive_iterator);
   Dee_Decref(interactive_module);
   Dee_Decref(interactive_output);
   if (!value) goto err;
   goto done;
  }
  Dee_Decref(sys_argv);
  if unlikely(!argc) goto err_no_input;

  /* Run the module passed through argv[0] */
  user_module = (DREF DeeModuleObject *)DeeModule_OpenFileString(argv[0],NULL,
                                                                 MODULE_FILECLASS_SOURCE|
                                                                 MODULE_FILECLASS_THROWERROR,
                                                                &script_options);
  if unlikely(!user_module) goto err;

  if (operation_mode == OPERATION_MODE_PRINTASM) {
   /* Print a full disassembly of the user-module. */
   dssize_t temp;
   if (!script_options.co_decoutput &&
       (script_options.co_decoutput = DeeFile_GetStd(DEE_STDOUT)) == NULL)
    temp = -1;
   else {
    PRIVATE DEFINE_STRING(str_disassembler,"disassembler");
    DREF DeeObject *disassembler_module;
    disassembler_module = DeeModule_Open((DeeObject *)&str_disassembler,NULL,true);
    if unlikely(!disassembler_module) temp = -1;
    else {
     DREF DeeObject *disasm_error;
     disasm_error = DeeObject_CallAttrStringPack(disassembler_module,
                                                 "printcode",
                                                 2,
                                                 user_module->mo_root,
                                                 script_options.co_decoutput);
     Dee_Decref(disassembler_module);
     Dee_XDecref(disasm_error);
     temp = disasm_error ? 0 : -1;
    }
   }
   Dee_Decref(user_module);
   if unlikely(temp < 0) goto err;
  } else if (operation_mode == OPERATION_MODE_BUILDONLY) {
   /* ... */
   Dee_Decref(user_module);
  } else {
   DREF DeeObject *user_module_main;
   DREF DeeObject *user_module_result;
   DREF DeeObject *user_module_args;
   int temp;
   /* The user's module has been loaded. - Now load dependencies and open it's root. */
   user_module_main = DeeModule_GetRoot((DeeObject *)user_module);
   Dee_Decref(user_module);
   if unlikely(!user_module_main) goto err;

   /* With the root now open, invoke it using the system argument vector. */
   user_module_args = Dee_GetArgv();
   ASSERT(user_module_args);
   user_module_result = DeeObject_Call(user_module_main,
                                       DeeTuple_SIZE(user_module_args),
                                       DeeTuple_ELEM(user_module_args));
   Dee_Decref(user_module_args);
   Dee_Decref(user_module_main);
   if unlikely(!user_module_result) goto err;
#if EXIT_SUCCESS != 0
   if (DeeNone_Check(user_module_result)) {
    /* Special case: A return value of `none' indicates a portable EXIT_SUCCESS */
    result = EXIT_SUCCESS;
    Dee_DecrefNokill(user_module_result);
   } else
#endif
   {
    /* Interpret the module-result as an integer and use that as exit-code. */
    temp = DeeObject_AsInt(user_module_result,&result);
    Dee_Decref(user_module_result);
    if unlikely(temp) goto err;
   }
  }
 }

done:

 /* Clear out object-level compiler options. */
 Dee_XDecref(emitpp_dpout);
 Dee_XDecref(script_options.co_decoutput);
 Dee_XDecref(import_options.co_decoutput);
 Dee_XDecref(script_options.co_pathname);
 Dee_XDecref(import_options.co_pathname);
 Dee_XDecref(script_options.co_filename);
 Dee_XDecref(import_options.co_filename);
 Dee_XDecref(script_options.co_rootname);
 Dee_XDecref(import_options.co_rootname);

 DEE_CHECKMEMORY();

 /* Run functions registered for atexit(). */
 Dee_RunAtExit(DEE_RUNATEXIT_FRUNALL);

 /* Reset the argument vector tuple. */
 Dee_SetArgv(Dee_EmptyTuple);

 /* Shutdown the deemon core.
  * This function does pretty much everything for us:
  *  - Interrupt + join all threads but the calling one.
  *  - Clear all remaining GC objects.
  *  - Unload all loaded DEX modules.
  *  - ...
  */
 Dee_Shutdown();

 /* Free up cached objects. They're not actually memory leaks, mkay?
  * Any malloc()-ed heap-block at application exit is a pretty broad
  * definition of memory leaks. While the set of data encompassed by
  * this definition does include any kind of memory leak caused by
  * malloc(), it also includes other stuff that should't be considered
  * a leak, which is mainly qualified by this stuff right here:
  *   - Lazily allocated global data structures:
  *     It's not a memory leak because there can only ever be
  *     one of them allocated a time. And if we fail to release
  *     it at allocation exit, the kernel will just do that for
  *     us.
  *      - DeeExec_SetHome
  *      - DeeThread_Fini
  *   - Preallocated object caches:
  *     These are just another abstraction layer between the
  *     universal heap, and a very specific kind of allocation,
  *     which are meant to speed by allocating objects of known
  *     size by re-using previously ~freed~ objects instead of
  *     having to go through the regular heap.
  *     They're not leaks because they're meant to represent
  *     data that has been free()-ed, and will be cleaned up
  *     by the kernel once the application has terminated.
  *     As a matter of fact: I bet you that the underlying
  *     malloc()-free() functions won't munmap() the entire heap
  *     once nothing is being used anymore, but will instead
  *     let the kernel do that.
  *      - DeeMem_ClearCaches
  */
#if !defined(NDEBUG) || defined(CONFIG_TRACE_REFCHANGES)
 DeeExec_SetHome(NULL);
 DEE_CHECKMEMORY();
 DeeThread_Fini();
 DEE_CHECKMEMORY();
#endif
#ifndef NDEBUG
 DeeMem_ClearCaches((size_t)-1);
 DEE_CHECKMEMORY();
#endif
 /* Dump information on all objects that are still alive.
  * Anything that still exists at this point really is a
  * reference leak. */
#ifdef CONFIG_TRACE_REFCHANGES
 Dee_DumpReferenceLeaks();
#endif
#ifndef NDEBUG
#if defined(_MSC_VER) || defined(__CRT_DOS)
 {
#if !defined(_MSC_VER) || defined(_DLL)
  extern ATTR_DLLIMPORT int ATTR_CDECL _CrtDumpMemoryLeaks(void);
#else
  extern int ATTR_CDECL _CrtDumpMemoryLeaks(void);
#endif
  _CrtDumpMemoryLeaks();
 }
#endif
#endif
 return result;

err_no_input:
 DeeError_Throwf(&DeeError_RuntimeError,
                 "No input files");
err:
#ifdef EXIT_FAILURE
 result = EXIT_FAILURE;
#else
 result = 1;
#endif
handle_errors:
 do {
  DeeObject *cur = DeeError_Current();
  /* Special handling for AppExit errors. */
  if (cur && DeeAppExit_Check(cur)) {
   result = DeeAppExit_Exitcode(cur);
   DeeError_Handled(ERROR_HANDLED_INTERRUPT);
   goto handle_errors;
  }
 } while (DeeError_Print(NULL,ERROR_PRINT_HANDLEINTR));
 goto done;
}











INTERN struct compiler_options import_options = {
    /* .co_inner         = */&import_options,
    /* .co_pathname      = */NULL,
    /* .co_filename      = */NULL,
    /* .co_rootname      = */NULL,
    /* .co_setup         = */NULL,
    /* .co_setup_arg     = */NULL,
    /* .co_error_handler = */&error_handler,
    /* .co_error_arg     = */NULL,
    /* .co_tabwidth      = */0,
    /* .co_compiler      = */COMPILER_FNORMAL,
    /* .co_parser        = */PARSE_FNORMAL,
    /* .co_optimizer     = */OPTIMIZE_FENABLED,
    /* .co_unwind_limit  = */4,
    /* .co_assembler     = */ASM_FOPTIMIZE|ASM_FPEEPHOLE|ASM_FREUSELOC|ASM_FSTACKDISP,
    /* .co_decloader     = */DEC_FNORMAL,
#ifdef DEC_WRITE_FNORMAL
    /* .co_decwriter     = */DEC_WRITE_FNORMAL,
#else
    /* .co_decwriter     = */0,
#endif
    /* .co_decoutput     = */NULL
};

INTERN struct compiler_options script_options = {
    /* .co_inner         = */&import_options,
    /* .co_pathname      = */NULL,
    /* .co_filename      = */NULL,
    /* .co_rootname      = */NULL,
    /* .co_setup         = */&compiler_setup,
    /* .co_setup_arg     = */NULL,
    /* .co_error_handler = */&error_handler,
    /* .co_error_arg     = */NULL,
    /* .co_tabwidth      = */0,
    /* .co_compiler      = */COMPILER_FNORMAL,
    /* .co_parser        = */PARSE_FNORMAL,
    /* .co_optimizer     = */OPTIMIZE_FDISABLED,
    /* .co_unwind_limit  = */0,
    /* .co_assembler     = */ASM_FNORMAL|ASM_FNODEC|ASM_FOPTIMIZE,
    /* .co_decloader     = */DEC_FDISABLE,
#ifdef DEC_WRITE_FNORMAL
    /* .co_decwriter     = */DEC_WRITE_FNORMAL,
#else
    /* .co_decwriter     = */0,
#endif
    /* .co_decoutput     = */NULL
};

#if defined(__GNUC__) && !defined(NDEBUG)
INTERN uintptr_t __stack_chk_guard = 0x1246Ab1f;
INTERN ATTR_NORETURN void __stack_chk_fail(void) { ASSERT(0); _Exit(1); }
#endif


PRIVATE void DCALL
emitpp_writeout(void const *__restrict p, size_t s) {
 if (DeeFile_WriteAll(script_options.co_decoutput,p,s) < 0)
     DeeError_Handled(ERROR_HANDLED_RESTORE);
}
PRIVATE int TPPCALL
emitpp_printout(char const *buf, size_t bufsize,
                void *UNUSED(closure)) {
 emitpp_writeout(buf,bufsize*sizeof(char));
 return 0;
}


/* Emitpp contextual variables. */
PRIVATE line_t          emitpp_curr_line;
PRIVATE struct TPPFile *emitpp_lasttoken_file;
PRIVATE char           *emitpp_lasttoken_end;
PRIVATE size_t          emitpp_lasttoken_fpoff;
PRIVATE char const     *emitpp_lastfilename;
PRIVATE uint32_t        emitpp_orig_flags;

LOCAL void DCALL emitpp_putline(void) {
 size_t      filename_size;
 char const *filename_text;
 struct TPPFile *f;
 line_t line; char buffer[16];
 if (emitpp_state&EMITPP_FNOLINE) return;
 f = TPPLexer_Textfile();
 if (TPPLexer_Current->l_token.t_file == f) {
  /* Try to use the start of the current token.
   * NOTE: Something like `f->f_oldpos' would be more
   *       appropriate to use, but we don't track that... */
  line = TPPFile_LineAt(f,TPPLexer_Current->l_token.t_begin);
 } else {
  /* Fallback: Use the current position within the file. */
  line = TPPFile_LineAt(f,f->f_pos);
 }
 filename_text = TPPFile_Filename(f,&filename_size);
 if (emitpp_curr_line == line && emitpp_lastfilename == filename_text) return;
 if ((emitpp_state&EMITPP_MOUTLINE) != EMITPP_FOUTLINE_ZERO &&
     (emitpp_state&EMITPP_FMAGICTOKENS) &&
     emitpp_lastfilename == filename_text && emitpp_curr_line <= line-1 &&
     emitpp_curr_line >= line-2) {
  /* Optimization: For smaller line-offsets of less than 2, it is usually
   *               easier to simply emit the linefeeds individually.
   * WARNING: We can't do this in ZERO-mode though, as in this mode linefeeds
   *          must only be emit when they actually exist
   */
  size_t offset = line-emitpp_curr_line;
  emitpp_curr_line = line;
  if ((emitpp_state&EMITPP_MOUTLINE) == EMITPP_FOUTLINE_TOK) {
   emitpp_writeout("[\n][\n]",(offset*3)*sizeof(char));
  } else {
   emitpp_writeout("\n\n",offset*sizeof(char));
  }
  return;
 }
 if (!(emitpp_state&EMITPP_FATLINEFEED))
       emitpp_writeout("\n",sizeof(char));
 emitpp_curr_line = line;
 if (emitpp_state&EMITPP_FNOCXXLINE) {
  emitpp_writeout("#line ",6*sizeof(char));
 } else {
  emitpp_writeout("# ",2*sizeof(char));
 }
 emitpp_writeout(buffer,(TPP_Itos(buffer,(TPP(int_t))(line+1))-buffer)*sizeof(char));
 if (emitpp_lastfilename != filename_text) {
  char *quote_buffer;
  size_t quote_size;
  emitpp_lastfilename = filename_text;
  emitpp_writeout(" \"",2);
  quote_size = TPP_SizeofEscape(filename_text,filename_size);
  quote_buffer = (char *)malloc(quote_size*sizeof(char));
  if (quote_buffer) {
   TPP_Escape(quote_buffer,filename_text,filename_size);
   emitpp_writeout(quote_buffer,quote_size*sizeof(char));
   free(quote_buffer);
  }
  emitpp_writeout((emitpp_state&EMITPP_MOUTLINE) == EMITPP_FOUTLINE_ZERO
               ? "\"\0" : "\"\n",2*sizeof(char));
 } else {
  emitpp_writeout((emitpp_state&EMITPP_MOUTLINE) == EMITPP_FOUTLINE_ZERO
               ? "\0" : "\n",sizeof(char));
 }
 emitpp_state |= EMITPP_FATLINEFEED;
}

LOCAL size_t DCALL get_file_offset(char *p) {
 struct TPPFile *f = TPPLexer_Current->l_token.t_file;
 size_t result = p-f->f_begin;
 if (f->f_kind == TPPFILE_KIND_TEXT) {
  result += (f->f_textfile.f_rdata-f->f_text->s_size);
 }
 return result;
}
LOCAL int DCALL
count_linefeeds(char const *iter, char const *end) {
 int result = 0;
 while (iter != end) {
  if (*iter == '\r') {
   if (iter != end-1 &&
       iter[1] == '\n') ++iter;
   ++result;
  } else if (*iter == '\n') ++result;
  ++iter;
 }
 return result;
}
PRIVATE void DCALL pp_print(char const *buf, size_t bufsize) {
 if ((emitpp_state&EMITPP_MOUTLINE) == EMITPP_FOUTLINE_TOK)
     emitpp_writeout("[",sizeof(char));
 emitpp_writeout(buf,bufsize*sizeof(char));
 switch (emitpp_state&EMITPP_MOUTLINE) {
 case EMITPP_FOUTLINE_ZERO:
  emitpp_writeout("\0",sizeof(char));
  emitpp_state |= EMITPP_FATLINEFEED;
  break;
 case EMITPP_FOUTLINE_TOK:
  emitpp_writeout("]",sizeof(char));
  ATTR_FALLTHROUGH
 default:
  emitpp_state &= ~EMITPP_FATLINEFEED;
  break;
 }
}
PRIVATE void DCALL emitpp_emitraw(void) {
 emitpp_lasttoken_file  = TPPLexer_Current->l_token.t_file;
 emitpp_lasttoken_end   = TPPLexer_Current->l_token.t_end;
 emitpp_lasttoken_fpoff = get_file_offset(emitpp_lasttoken_end);
 if ((emitpp_state&EMITPP_MOUTLINE) == EMITPP_FOUTLINE_TOK)
     emitpp_writeout("[",sizeof(char));
 if (!(emitpp_state&EMITPP_FNODECODETOK)) {
  TPP_PrintToken(&emitpp_printout,NULL);
  if (TPPLexer_Current->l_token.t_id == '\n')
      ++emitpp_curr_line;
 } else {
  emitpp_writeout(TPPLexer_Current->l_token.t_begin,
      (size_t)(TPPLexer_Current->l_token.t_end-
               TPPLexer_Current->l_token.t_begin)*
       sizeof(char));
  /* Track what we expect the current line number to be,
   * which is them compared to the actual line number. */
  emitpp_curr_line += count_linefeeds(TPPLexer_Current->l_token.t_begin,
                                  TPPLexer_Current->l_token.t_end);
 }
 switch (emitpp_state&EMITPP_MOUTLINE) {
 case EMITPP_FOUTLINE_ZERO:
  emitpp_writeout("\0",sizeof(char));
  emitpp_state |= EMITPP_FATLINEFEED;
  break;
 case EMITPP_FOUTLINE_TOK:
  emitpp_writeout("]",sizeof(char));
  emitpp_state &= ~EMITPP_FATLINEFEED;
  break;
 default: 
  if (emitpp_lasttoken_end[-1] == '\n' ||
      emitpp_lasttoken_end[-1] == '\r') {
   emitpp_state |=  EMITPP_FATLINEFEED;
  } else {
   emitpp_state &= ~EMITPP_FATLINEFEED;
  }
  break;
 }
}
PRIVATE int TPPCALL emitpp_reemit_pragma(void) {
#define PRAGMA_COPYMASK  (TPPLEXER_FLAG_WANTCOMMENTS| \
                          TPPLEXER_FLAG_WANTSPACE| \
                          TPPLEXER_FLAG_WANTLF)
 if (!(emitpp_state&EMITPP_FATLINEFEED) &&
      (emitpp_state&EMITPP_MOUTLINE) != EMITPP_FOUTLINE_NORMAL)
       pp_print("\n",1),++emitpp_curr_line;
 pp_print("#",1);
 pp_print("pragma",6);
 if (emitpp_orig_flags&TPPLEXER_FLAG_WANTSPACE)
     pp_print(" ",1);
 TPPLexer_Current->l_flags &= ~PRAGMA_COPYMASK;
 TPPLexer_Current->l_flags |= (emitpp_orig_flags&PRAGMA_COPYMASK);
 do emitpp_emitraw(); while (TPPLexer_Yield() > 0);
 pp_print("\n",1);
 ++emitpp_curr_line;
 emitpp_state         |= EMITPP_FATLINEFEED;
 emitpp_lasttoken_file = NULL;
 emitpp_lasttoken_end  = NULL;
 return 1;
}
PRIVATE void DCALL emitpp_emit(void) {
 if (emitpp_lasttoken_file  != TPPLexer_Current->l_token.t_file ||
    (emitpp_lasttoken_end   != TPPLexer_Current->l_token.t_begin &&
     emitpp_lasttoken_fpoff != get_file_offset(TPPLexer_Current->l_token.t_begin))) {
  /* The file changed, or there is a difference in the in-file position
   * between the end of the last token and the start of this one.
   * >> In any case, we must update the #line offset. */
  emitpp_putline();
 }
 emitpp_emitraw();
}

PRIVATE int DCALL
operation_mode_printpp(int argc, char **argv) {
 if (!TPP_INITIALIZE()) goto err_nofin;
 TPPLexer_Current->l_flags |= (TPPLEXER_FLAG_WANTSPACE|
                               TPPLEXER_FLAG_WANTLF|
                               TPPLEXER_FLAG_REEMIT_UNKNOWN_PRAGMA|
#ifdef _WIN32
                               TPPLEXER_FLAG_MSVC_MESSAGEFORMAT|
#endif
                               TPPLEXER_FLAG_TERMINATE_STRING_LF);

 if (!script_options.co_decoutput &&
     (script_options.co_decoutput = DeeFile_GetStd(DEE_STDOUT)) == NULL)
      goto err;

 /* Setup the #include-stack. */
 while (argc--) {
  struct TPPFile *file;
  file = TPPFile_Open(*argv++);
  if unlikely(!file) {
   if (!(TPPLexer_Current->l_flags & TPPLEXER_FLAG_ERROR))
         err_file_not_found(argv[-1]);
   goto err;
  }
  TPPLexer_PushFileInherited(file);
 }
 /* Re-emit all pragmas using the #pragma notation. */
 TPPLexer_Current->l_callbacks.c_parse_pragma = &emitpp_reemit_pragma;
 if (script_options.co_setup &&
   (*script_options.co_setup)(script_options.co_setup_arg))
     goto err;

 /* Initial values to simulate the last token
  * ending where the first file starts. */
 emitpp_orig_flags      = TPPLexer_Current->l_flags;
 emitpp_lasttoken_file  = NULL; /*infile;*/ /* Force a line directive at the first token. */
 emitpp_lasttoken_end   = TPPLexer_Current->l_token.t_file->f_begin;
 emitpp_lasttoken_fpoff = 0;
 emitpp_curr_line       = 0;
 emitpp_state          |= EMITPP_FATLINEFEED;
 emitpp_lastfilename    = NULL;
 parser_start();
 while (TPPLexer_Yield() > 0) emitpp_emit();
 if (parser_rethrow((TPPLexer_Current->l_flags & TPPLEXER_FLAG_ERROR) != 0))
     goto err;
 if ((emitpp_state&EMITPP_MOUTLINE) == EMITPP_FOUTLINE_ZERO)
      emitpp_writeout("\0",sizeof(char));
 TPP_FINALIZE();
 return 0;
err:       TPP_FINALIZE();
err_nofin: return -1;
}


PRIVATE char const dformat_code_head[] = "[[[deemon";
PRIVATE char const dformat_code_tail[] = "]]]";
PRIVATE char const dformat_stop[] = "[[[end]]]";

PRIVATE char *DCALL
compare_escaped(char *lf_escaped_text,
                char const *other_text) {
 char ch;
 while ((ch = *other_text++) != 0) {
  char escape_ch = *lf_escaped_text++;
  if (escape_ch == ch) continue;
  /* Check for escaped line-feeds. */
  if (escape_ch != '\\') return NULL;
  escape_ch = *lf_escaped_text++;
  if (escape_ch == '\r') {
   if (*lf_escaped_text == '\n')
      ++lf_escaped_text;
  } else if (escape_ch == '\n') {
  } else {
   return NULL;
  }
  --other_text;
 }
 return lf_escaped_text;
}

PRIVATE char *DCALL
compare_escaped_rev(char *lf_escaped_text_end,
                    char const *other_text_end,
                    char const *other_text_start) {
 char ch,escape_ch;
 while (other_text_end > other_text_start) {
  ch = *--other_text_end;
  escape_ch = *--lf_escaped_text_end;
  if (escape_ch == ch) continue;
  /* Check for escaped line-feeds. */
  if (escape_ch == '\n') {
   if (lf_escaped_text_end[-1] == '\r')
       --lf_escaped_text_end;
  } else if (escape_ch == '\r') {
  } else {
   return NULL;
  }
  if (lf_escaped_text_end[-1] != '\\')
      return NULL;
  --lf_escaped_text_end;
  ++other_text_end;
 }
 return lf_escaped_text_end;
}


#define COMMENT_TYPE_OTHER       0 /* Everything else. */
#define COMMENT_TYPE_BLOCK_START 1 /* `[[[deemon*' */
#define COMMENT_TYPE_BLOCK_END   2 /* `[[[end]]]' */
PRIVATE int DCALL get_comment_type(void) {
 char *comment_start;
 char *comment_end;
 if (tok != TOK_COMMENT) goto is_other;
 comment_start = token.t_begin+1;
 comment_end   = token.t_end;
 while (SKIP_WRAPLF(comment_start,comment_end));
 if (*token.t_begin == '/') {
  ++comment_start;
  if (comment_start[-1] == '*') {
   while (SKIP_WRAPLF(comment_start,comment_end));
   if (compare_escaped(comment_start,dformat_code_head))
       return COMMENT_TYPE_BLOCK_START;
   comment_start = compare_escaped(comment_start,dformat_stop);
   if (comment_start) {
    /* Strictly check for slash-start end-comments. - No whitespace allowed! */
    while (SKIP_WRAPLF_REV(comment_end,comment_start));
    --comment_end; /* /*[[[end]]]* */
    while (SKIP_WRAPLF_REV(comment_end,comment_start));
    --comment_end; /* /*[[[end]]] */
    while (SKIP_WRAPLF_REV(comment_end,comment_start));
    if (comment_start == comment_end)
        return COMMENT_TYPE_BLOCK_END;
   }
  } else {
   ASSERT(comment_start[-1] == '/');
   while (SKIP_WRAPLF(comment_start,comment_end));
   goto check_single_line;
  }
 } else {
  ASSERT(*token.t_begin == '#');
check_single_line:
  if (compare_escaped(comment_start,dformat_code_head))
      return COMMENT_TYPE_BLOCK_START;
  if (compare_escaped(comment_start,dformat_stop))
      return COMMENT_TYPE_BLOCK_END;
 }
is_other:
 return COMMENT_TYPE_OTHER;
}



PRIVATE void DCALL clear_inner_tpp_state(void) {
 struct TPPFile *fileiter,*filenext;
 /* Emit warnings about all unclosed #ifdef-blocks. */
 if (!(TPPLexer_Current->l_flags&TPPLEXER_FLAG_ERROR))
       TPPLexer_ClearIfdefStack();
 free(TPPLexer_Current->l_ifdef.is_slotv);
 /* Clear the remainder of the #include-stack. */
 fileiter = TPPLexer_Current->l_token.t_file;
 do {
  ASSERT(fileiter);
  filenext = fileiter->f_prev;
  TPPFile_Decref(fileiter);
 } while ((fileiter = filenext) != NULL);
}


/* Just as the name says: execute a module's root-function
 * and capture stdout output, which is then returned. */
PRIVATE DREF DeeStringObject *DCALL
exec_module_and_capture_stdout(DeeModuleObject *__restrict module) {
 DREF DeeStringObject *result = NULL;
 DREF DeeFunctionObject *module_root;
 DREF DeeObject *old_stdout;
 DREF DeeObject *new_stdout;
 DREF DeeObject *temp;
 /* Open the root of the module. */
 module_root = (DeeFunctionObject *)DeeModule_GetRoot((DeeObject *)module);
 if unlikely(!module_root) goto out;
 /* Create a new writer and set it as target for STDOUT */
 new_stdout = DeeFile_OpenWriter();
 if unlikely(!new_stdout) goto out_root;
 old_stdout = DeeFile_SetStd(DEE_STDOUT,new_stdout);

 /* Execute the module root. */
 /* XXX: Maybe pass something more interesting through arguments? */
 temp = DeeObject_Call((DeeObject *)module_root,0,NULL);
 if (temp) {
  /* Pack together the printed string. */
  result = (DREF DeeStringObject *)DeeFileWriter_GetString(new_stdout);
  Dee_Decref(temp);
 }

 Dee_Decref(new_stdout);

 /* Restore the old STD streams. */
 new_stdout = DeeFile_SetStd(DEE_STDOUT,old_stdout);
 if (ITER_ISOK(old_stdout)) Dee_Decref(old_stdout);
 if (ITER_ISOK(new_stdout)) Dee_Decref(new_stdout);
out_root:
 Dee_Decref(module_root);
out:
 return result;
}




/* With the current token set to the last token apart of the initial format-code comment,
 * search for the format end-comment and execute format code if it could be found.
 * Upon return, the current token is set to the [[[end]]] comment, and if code was
 * executed and the source file was updated, the TPP file cache will have been reset
 * in order to re-sync it with the updated source file. */
PRIVATE int DCALL
try_exec_format_impl(DeeObject *__restrict stream,
                     char *filename, char *ddi_filename,
                     char *__restrict format_code_start,
                     char *__restrict format_code_end,
                     line_t format_code_start_line,
                     col_t format_code_start_col) {
 struct TPPFile *file = token.t_file;
 bool is_file_relative_code; int error;
 dpos_t override_start_pos;
 char *override_start_ptr;
 char *override_end_ptr;
 bool has_leading_linefeed;
 DREF DeeStringObject *script_result;
 ASSERT(tok == TOK_COMMENT);
 override_start_ptr = token.t_end;
 is_file_relative_code = (format_code_start >= file->f_begin &&
                          format_code_start <  file->f_end);
 if (is_file_relative_code) {
  *(uintptr_t *)&format_code_start -= (uintptr_t)file->f_begin;
  *(uintptr_t *)&format_code_end   -= (uintptr_t)file->f_begin;
 }
 *(uintptr_t *)&token.t_begin      -= (uintptr_t)file->f_begin;
 *(uintptr_t *)&token.t_end        -= (uintptr_t)file->f_begin;
 *(uintptr_t *)&override_start_ptr -= (uintptr_t)file->f_begin;
 /* Load the remainder of the current file. */
 do error = TPPFile_NextChunk(file,TPPFILE_NEXTCHUNK_FLAG_EXTEND);
 while (error > 0);
 if (is_file_relative_code) {
  *(uintptr_t *)&format_code_start += (uintptr_t)file->f_begin;
  *(uintptr_t *)&format_code_end   += (uintptr_t)file->f_begin;
 }
 *(uintptr_t *)&token.t_begin      += (uintptr_t)file->f_begin;
 *(uintptr_t *)&token.t_end        += (uintptr_t)file->f_begin;
 *(uintptr_t *)&override_start_ptr += (uintptr_t)file->f_begin;
 if (error < 0) goto err;
 /* Setup the lexer to not escape the current file. */
 TPPLexer_Current->l_eob_file = file;

 /* Search for the block-end-token. */
 for (;;) {
  tok_t next = yield();
  if (next < 0) goto err;
  if (next == TOK_EOF)
      goto done; /* Block-end not found prior to end-of-file.
                  * TODO: Emit a warning, telling the user that the end is missing. */
  if (token.t_file == file) {
   int type = get_comment_type();
   if (type == COMMENT_TYPE_BLOCK_START)
       goto done; /* Another block-start found prior to the end of the current.
                   * TODO: Emit a warning, telling the user that the inner one
                   *       will get executed, rather than the outer one. */
   if (type == COMMENT_TYPE_BLOCK_END)
       break; /* The end of the current block was found! */
  }
 }
 override_end_ptr = token.t_begin;
 /* Do some special checking to skip a single leading line-feed
  * within the data block which is going to get overwritten.
  * -> That way, the user can easily insert a persistent LF
  *    just before the output block, but still be able to
  *    generate inline-output data if they don't do this:
  * >> /-*[[[deemon print "Hello";]]]*-/Hello/-*[[[end]]]*-/
  * >> /-*[[[deemon print "Hello";]]]*-/
  * >> Hello
  * >> /-*[[[end]]]*-/
  * Additionally (as can already be seen), this will also cause a
  * trailing line-feed to be re-appended to generated text, after it
  * has been tripped of all trailing whitespace (including linefeeds):
  * >> local output_text = get_output_text();
  * >> output_text = output_text.rstrip();
  * >> if (has_leading_linefeed)
  * >>     output_text += "\n";
  * NOTE: The old deemon didn't used to do this rstrip(), however in
  *       all the time that I've already been using `deemon -F', trailing
  *       whitespace is something I often have to be real careful to prevent,
  *       or adjust in a way that won't cause your eyes to start bleeding...
  *    -> Having deemon itself auto-adjust trailing whitespace will solve
  *       all those problems, as all I really want it to do, is to mirror
  *       the spacing behavior already seen at the start of the output-block. */
 has_leading_linefeed = false;
 if (*override_start_ptr == '\n')
  has_leading_linefeed = true,
  ++override_start_ptr;
 else if (*override_start_ptr == '\r') {
  if (*++override_start_ptr == '\n')
       ++override_start_ptr;
  has_leading_linefeed = true;
 }

 /* Figure out the absolute in-source file position where overriding will start. */
 override_start_pos = (dpos_t)DeeFile_Tell(stream);
 if unlikely((doff_t)override_start_pos < 0) goto err;
 override_start_pos -= (size_t)(file->f_end-override_start_ptr);

 /* Skip leading line-feeds in the format-code.
  * This way, `DeeModule_OpenMemoryString()' won't have to adjust for column-offsets. */
 while (format_code_start < format_code_end &&
       (*format_code_start == '\n' ||
        *format_code_start == '\r')) {
  ++format_code_start;
  if (format_code_start[-1] == '\r' &&
      format_code_start[0]  == '\n')
    ++format_code_start;
  ++format_code_start_line;
  format_code_start_col = 0;
 }

 /* Let's recap:
  *  - We're supposed to execute text from `format_code_start...format_code_end'
  *    as deemon user-code with file.stdout redirected such that all data written
  *    gets appended to an internal string-stream.
  *  - Once that is done (and only if doing so didn't fail with some kind of error),
  *    take all that output and replace all source-data from `override_start_pos'
  *    up to `override_start_pos + (override_end_ptr - override_start_ptr)' with
  *    that new data, after copying back-up of the source file into $TMP (as
  *    determined using the `fs' module's `gettmp()' function)
  *  - Once we're finished with that, update TPP file caches, such that it
  *    can safely continue parsing the source file for more format-blocks.
  *    NOTE: The way that I ended up implementing it, this is a given...
  */
 {
  struct compiler_options opt;
  DREF DeeModuleObject *script_module;
  struct TPPToken old_token;
  struct TPPIfdefStack old_ifdef;
  tok_t old_l_noerror;
  size_t old_l_eof_paren;
  uint32_t old_l_flags;
  uint32_t old_l_extokens;
  struct TPPFile *old_l_eob_file;
  struct TPPFile *old_l_eof_file;

  /* Setup compiler options for the inner script. */
  memcpy(&opt,&script_options,sizeof(struct compiler_options));
  opt.co_compiler  |= (COMPILER_FKEEPLEXER|COMPILER_FKEEPERROR); /* Keep using the same lexer and errors! */
  opt.co_assembler |= ASM_FNODEC; /* Disable DEC file creation */

  if (ddi_filename && ddi_filename != filename) {
   opt.co_filename = (struct string_object *)DeeString_New(ddi_filename);
   if unlikely(!opt.co_filename) goto err;
  }

  /* Save the current token, include-stack, as well as some other TPP options.
   * We don't just use a new lexer (i.e. we set the `COMPILER_FKEEPLEXER' flag),
   * because we want to make the same macros visible to the script, as were
   * already visible by the surrounding code. That way, format-scripts are able
   * to make use of macros defined by the surrounding code, allowing them to
   * generate code in compliance to macros defined by something along the lines
   * of a config-file. */
  memcpy(&old_token,&TPPLexer_Current->l_token,sizeof(struct TPPToken));
  TPPFile_Incref(&TPPFile_Empty);
  TPPLexer_Current->l_token.t_id    = TOK_EOF;
  TPPLexer_Current->l_token.t_num   = 0;
  TPPLexer_Current->l_token.t_file  = &TPPFile_Empty;
  TPPLexer_Current->l_token.t_begin = TPPFile_Empty.f_begin;
  TPPLexer_Current->l_token.t_end   = TPPFile_Empty.f_end;
  memcpy(&old_ifdef,&TPPLexer_Current->l_ifdef,sizeof(struct TPPIfdefStack));
  memset(&TPPLexer_Current->l_ifdef,0,sizeof(struct TPPIfdefStack));
  old_l_noerror = TPPLexer_Current->l_noerror;
  TPPLexer_Current->l_noerror = TOK_EOF;
  old_l_eof_paren = TPPLexer_Current->l_eof_paren;
  TPPLexer_Current->l_eof_paren = 0;
  old_l_flags = TPPLexer_Current->l_flags;
  TPPLexer_Current->l_flags = (TPPLEXER_FLAG_DEFAULT
#ifdef _MSC_VER
                               |
                               TPPLEXER_FLAG_MSVC_MESSAGEFORMAT
#endif
                               );
  old_l_extokens = TPPLexer_Current->l_extokens;
  TPPLexer_Current->l_extokens = TPPLEXER_TOKEN_LANG_DEEMON;
  old_l_eob_file = TPPLexer_Current->l_eob_file;
  TPPLexer_Current->l_eob_file = NULL;
  old_l_eof_file = TPPLexer_Current->l_eof_file;
  TPPLexer_Current->l_eof_file = NULL;

  /* Reset the `#pragma once' lexer state. (required for backwards compatibility)
   * This is the same trick that deemon 101+ used to re-enable including of headers
   * within script code, when those headers had already been included once before. */
  TPPLexer_Reset(TPPLexer_Current,TPPLEXER_RESET_FONCE);

  /* During Execution of script code, define another macro `__FORMAT_SCRIPT__'
   * that the execution-context of format scripts to be detected in user-code. */
  if (!TPPLexer_Define("__FORMAT_SCRIPT__",17,"1",1,TPPLEXER_DEFINE_FLAG_NONE))
       script_module = NULL;
  else {
   /* Compile the format-script into a module. */
   script_module = (DREF DeeModuleObject *)DeeModule_OpenMemoryString(filename,
                                                                      NULL,
                                                                      format_code_start,
                                                                     (size_t)(format_code_end - format_code_start),
                                                                      format_code_start_line,
                                                                      format_code_start_col,
                                                                     &opt);
   /* Remove the format-script macro again. */
   TPPLexer_Undef("__FORMAT_SCRIPT__",17);
  }

  Dee_XDecref(opt.co_filename);

  /* Clear any lexer component not properly restored by the script. */
  clear_inner_tpp_state();


  /* Restore the old TPP context. */
  TPPLexer_Current->l_eof_file  = old_l_eof_file;
  TPPLexer_Current->l_eob_file  = old_l_eob_file;
  TPPLexer_Current->l_extokens  = old_l_extokens;
  TPPLexer_Current->l_flags     = old_l_flags;
  TPPLexer_Current->l_eof_paren = old_l_eof_paren;
  TPPLexer_Current->l_noerror   = old_l_noerror;
  memcpy(&TPPLexer_Current->l_ifdef,&old_ifdef,sizeof(struct TPPIfdefStack));
  memcpy(&TPPLexer_Current->l_token,&old_token,sizeof(struct TPPToken));

  /* Check if we managed to parse the script properly. */
  if unlikely(!script_module) goto err;

  script_result = exec_module_and_capture_stdout(script_module);
  Dee_Decref(script_module);
 }

 if unlikely(!script_result)
    goto err;

 /* TODO: Copy a backup of the file into
  *       some temporary-file-folder... */

 {
  char *result_start,*result_end;
  size_t new_text_size;
  result_start = DeeString_STR(script_result);
  result_end   = result_start+DeeString_SIZE(script_result);
  /* Strip trailing whitespace. */
  while (result_end > result_start &&
         DeeUni_IsSpace(result_end[-1]))
       --result_end;
  /* Now comes the part that it's been all about:
   * This is where we override the original source file's contents! */
  if (DeeFile_Seek(stream,(doff_t)override_start_pos,SEEK_SET) < 0)
      goto err_script_result;
  /* Write data to the stream. */
  new_text_size = (size_t)(result_end-result_start);
  if (DeeFile_WriteAll(stream,
                       result_start,
                       new_text_size) < 0)
      goto err_script_result_restore;

  if (has_leading_linefeed) {
   /* Append a trailing linefeed at the end. */
   if (DeeFile_Putc(stream,'\n') < 0)
       goto err_script_result_restore;
   ++new_text_size;
  }

  /* Write all data we weren't supposed to override. */
  if (DeeFile_WriteAll(stream,
                       override_end_ptr,
                      (size_t)(file->f_end-override_end_ptr)) < 0)
      goto err_script_result_restore;
  if (new_text_size < (size_t)(override_end_ptr-override_start_ptr)) {
   /* Must truncate the file to its new size. */
   if (DeeFile_TruncHere(stream,NULL))
       goto err_script_result_restore;
  }
 }

 /* XXX: Delete the backup? */

 Dee_Decref(script_result);
done:
 return 0;
err_script_result_restore:
 /* TODO: Restore the backup */
err_script_result:
 Dee_Decref(script_result);
err:
 return -1;
}

PRIVATE int DCALL
try_exec_format(DeeObject *__restrict stream,
                char *filename, char *ddi_filename,
                char *__restrict format_code_start,
                char *__restrict format_code_end,
                line_t format_code_start_line,
                col_t format_code_start_col) {
 int result;
 struct TPPFile *old_eob;
 uint32_t old_flags;
 /* Preserve some options that get overwritten by the implementation above. */
 old_eob = TPPLexer_Current->l_eob_file;
 old_flags = TPPLexer_Current->l_flags;
 result = try_exec_format_impl(stream,
                               filename,
                               ddi_filename,
                               format_code_start,
                               format_code_end,
                               format_code_start_line,
                               format_code_start_col);
 /* Restore options saved above. */
 TPPLexer_Current->l_flags &= TPPLEXER_FLAG_MERGEMASK;
 TPPLexer_Current->l_flags |= old_flags;
 TPPLexer_Current->l_eob_file = old_eob;
 return result;
}


PRIVATE int const format_disabled_warnings[] = {
    W_REDEFINING_BUILTIN_KEYWORD,
    W_UNKNOWN_PREPROCESSOR_DIRECTIVE,
    W_STARSLASH_OUTSIDE_OF_COMMENT,
    W_FILE_NOT_FOUND,
    W_CANT_UNDEF_BUILTIN_MACRO,
    W_INVALID_INTEGER,
    W_UNKNOWN_TOKEN_IN_EXPR_IS_ZERO,
    W_INVALID_WARNING,
    W_DIVIDE_BY_ZERO, /* As the result of `W_UNKNOWN_TOKEN_IN_EXPR_IS_ZERO' */
    W_SLASHSTAR_INSIDE_OF_COMMENT,
    W_LINE_COMMENT_CONTINUED,
    W_CHARACTER_TOO_LONG,
    W_MULTICHAR_NOT_ALLOWED,
    W_ENCOUNTERED_TRIGRAPH,
    W_INTEGRAL_OVERFLOW,
    W_INTEGRAL_CLAMPED,
    W_INVALID_FLOAT_SUFFIX,
};


PRIVATE int DCALL
dformat_source_files(char *filename,
                     char *ddi_filename) {
 struct TPPFile *file;
 DREF DeeFileObject *filestream;
 if (!TPP_INITIALIZE()) goto err_nofin;
 /* Configure the lexer for what we have in mind. */
 TPPLexer_Current->l_flags |= (TPPLEXER_FLAG_WANTCOMMENTS|
#ifdef _WIN32
                               TPPLEXER_FLAG_MSVC_MESSAGEFORMAT|
#endif
                               TPPLEXER_FLAG_ASM_COMMENTS|
                               TPPLEXER_FLAG_TERMINATE_STRING_LF);

 /* Define a macro `__FORMAT__' that can be used to
  * identify a format-related preprocessor context. */
 if (!TPPLexer_Define("__FORMAT__",10,"1",1,TPPLEXER_DEFINE_FLAG_NONE))
      goto err;

 /* Execute late commandline parameters. */
 if (script_options.co_setup &&
   (*script_options.co_setup)(script_options.co_setup_arg))
     goto err;

 /* Disable some warnings that could get ~real~ annoying here... */
 {
  unsigned int i;
  for (i = 0; i < COMPILER_LENOF(format_disabled_warnings); ++i) {
   if (!TPPLexer_SetWarning(format_disabled_warnings[i],WSTATE_DISABLED))
        goto err;
  }
 }

 /* Open the file that's supposed to get formatted. */
 filestream = (DREF DeeFileObject *)DeeFile_OpenString(filename,
                                                       OPEN_FRDWR|OPEN_FXWRITE,
                                                       0);
 if unlikely(filestream == (DREF DeeFileObject *)ITER_DONE) {
  /* File-not-found. */
  DeeError_Throwf(&DeeError_FileNotFound,
                  "File %q could not be found",
                   filename);
  filestream = NULL;
 }

 if unlikely(!filestream) goto err;
 file = TPPFile_OpenStream(filestream,filename);
 if unlikely(!file) goto err_stream;
 if (ddi_filename && ddi_filename != filename) {
  struct TPPString *used_name;
  size_t ddi_length = strlen(ddi_filename);
do_set_ddi_name:
  used_name = TPPString_New(ddi_filename,ddi_length);
  if unlikely(!used_name) {
   if (Dee_CollectMemory(offsetof(struct TPPString,s_text)+
                        (ddi_length+1)*sizeof(char)))
       goto do_set_ddi_name;
   TPPFile_Decref(file);
   goto err_stream;
  }
  ASSERT(!file->f_textfile.f_usedname);
  file->f_textfile.f_usedname = used_name; /* Inherit */
 }

 TPPLexer_PushFileInherited(file);
 /* Scan for comment tokens. */
 parser_start();
 for (;;) {
  char *comment_start;
  char *comment_end;
  struct TPPLCInfo lc;
  if (tok != TOK_COMMENT) goto next_token;   /* Not a comment. */
  if (token.t_file != file) goto next_token; /* Located in a different file. */
  /* Found a token that may be what we're looking for. */
  comment_start = token.t_begin+1;
  comment_end   = token.t_end;
  while (SKIP_WRAPLF(comment_start,comment_end));
  if (token.t_begin[0] == '/') {
   if (*comment_start == '*') {
    /* Multi-line comment. */
    ++comment_start;
    while (SKIP_WRAPLF(comment_start,comment_end));
    comment_start = compare_escaped(comment_start,dformat_code_head);
    if (!comment_start) goto next_token;
    while (SKIP_WRAPLF_REV(comment_end,comment_start));
    --comment_end; /* /*foo* */
    while (SKIP_WRAPLF_REV(comment_end,comment_start));
    --comment_end; /* /*foo */
    while (SKIP_WRAPLF_REV(comment_end,comment_start));
    comment_end = compare_escaped_rev(comment_end,
                                      COMPILER_STREND(dformat_code_tail),
                                      dformat_code_tail);
    if (!comment_end) goto next_token;
    TPPFile_LCAt(token.t_file,&lc,comment_start);
    if (try_exec_format((DeeObject *)filestream,
                         filename,
                         ddi_filename,
                         comment_start,
                         comment_end,
                         lc.lc_line,
                         lc.lc_col))
        goto err_stream;
   } else {
    /* Single-line comment (allow continuation in the next line) */
    ASSERT(*comment_start == '/');
    ++comment_start;
    while (SKIP_WRAPLF(comment_start,comment_end));
    comment_start = compare_escaped(comment_start,dformat_code_head);
    if (!comment_start) goto next_token;
    comment_end = compare_escaped_rev(comment_end,
                                      COMPILER_STREND(dformat_code_tail),
                                      dformat_code_tail);
    if (!comment_end) {
     /* TODO: Scan immediately following comment tokens for the end of the format-block. */
     DERROR_NOTIMPLEMENTED();
     goto err_stream;
    }
   }
  } else {
   /* Assembly-style comment. */
   ASSERT(token.t_begin[0] == '#');
   comment_start = compare_escaped(comment_start,dformat_code_head);
   if (!comment_start) goto next_token;
   comment_end = compare_escaped_rev(comment_end,
                                     COMPILER_STREND(dformat_code_tail),
                                     dformat_code_tail);
   if (!comment_end) {
    /* TODO: Scan immediately following comment tokens for the end of the format-block. */
    DERROR_NOTIMPLEMENTED();
    goto err_stream;
   }
  }

next_token:
  if (yield() <= 0)
      break;
 }
 Dee_Decref(filestream);
 if (!(TPPLexer_Current->l_flags & TPPLEXER_FLAG_ERROR))
       TPPLexer_ClearIfdefStack();
 if (parser_rethrow((TPPLexer_Current->l_flags & TPPLEXER_FLAG_ERROR) != 0))
     goto err;
 TPP_FINALIZE();
 parser_errors_fini(&current_parser_errors);
 return 0;
err_stream:
 Dee_Decref(filestream);
 parser_rethrow(true);
err:
 parser_errors_fini(&current_parser_errors);
 TPP_FINALIZE();
err_nofin:
 return -1;
}


#ifdef CONFIG_HOST_WINDOWS
PRIVATE bool DCALL os_trychdir(char *__restrict path) {
 DREF DeeObject *pathob; LPWSTR wpath;
 DWORD dwError;
 if (SetCurrentDirectoryA(path))
     return true;
 /* Try the wide-character version. */
 pathob = DeeString_New(path);
 if unlikely(!pathob) goto err;
 wpath = (LPWSTR)DeeString_AsWide(pathob);
 if unlikely(!wpath) goto err_pathob;
 if (SetCurrentDirectoryW(wpath)) {
  Dee_Decref(pathob);
  return true;
 }
 dwError = GetLastError();
 if (nt_IsUncError(dwError)) {
  /* Try one last time after fixing the path to be UNC-compliant. */
  DREF DeeObject *new_path;
  new_path = nt_FixUncPath(pathob);
  Dee_Decref(pathob);
  if unlikely(!new_path) goto err;
  pathob = new_path;
  wpath = (LPWSTR)DeeString_AsWide(pathob);
  if unlikely(!wpath) goto err_pathob;
  if (SetCurrentDirectoryW(wpath)) {
   Dee_Decref(pathob);
   return true;
  }
 }
 Dee_Decref(pathob);
 return false;
err_pathob:
 Dee_Decref(pathob);
err:
 DeeError_Handled(ERROR_HANDLED_RESTORE);
 return false;
}
#define IS_SEP(x) ((x) == '\\' || (x) == '/')
#else
#define IS_SEP(x) ((x) == '/')
#define os_trychdir(path) (chdir(path) == 0)
#endif


PRIVATE int DCALL
dchdir_and_format_source_files(char *__restrict filename) {
 int result;
 /* Try to chdir() into the folder of the module.
  * This is something the old deemon didn't do,
  * leading to a lot of format-scripts starting with:
  * >> #include <fs>
  * >> fs.chdir(fs.path.head(__FILE__));
  * The idea here is that format-scripts usually do
  * something like this:
  * >> import * from deemon;
  * >> for (local line: file.open("../../defs/mydef.def")) {
  * >>     line = line.strip();
  * >>     if (!line || line.startswith("#")) continue;
  * >>     local x,y = line.scanf("%d = %d")...;
  * >>     print "DEFINE("+x+","+y+")";
  * >> }
  */
 char *path_end;
 path_end = strend(filename);
 while (path_end > filename) {
  if (IS_SEP(*path_end)) {
   char backup = *path_end,*iter;
   bool chdir_ok; unsigned int num_uprefs;
   char *buffer; size_t req_bufsize;
   *path_end = '\0';
   chdir_ok = os_trychdir(filename);
   *path_end = backup;
   if (!chdir_ok) break;
   /* Format the script as though it was located in the current folder. */
   result = dformat_source_files(path_end+1,filename);
   /* Change back to the directory that we originated from. */
   num_uprefs = 1;
   iter = filename;
   while (iter < path_end) {
    if (IS_SEP(*iter)) ++num_uprefs;
    ++iter;
   }
   req_bufsize = (num_uprefs*3)-1;
   if (req_bufsize <= strlen(filename)) {
    /* Re-use the given filename as buffer. */
    buffer = filename;
   } else {
    buffer = (char *)Dee_AMalloc((req_bufsize+1)*sizeof(char));
    if unlikely(!buffer) return -1;
   }
   iter = buffer;
   for (;;) {
    *iter++ = '.';
    *iter++ = '.';
    if (!--num_uprefs) break;
#ifdef CONFIG_HOST_WINDOWS
    *iter++ = '\\';
#else
    *iter++ = '/';
#endif
   }
   /* Back back to the original folder. */
   *iter = '\0';
   os_trychdir(buffer);

   if (buffer != filename)
       Dee_AFree(buffer);
   return result;
  }
  --path_end;
 }

 /* Without any slashes, or if the chdir() failed, don't change directory! */
 result = dformat_source_files(filename,NULL);
 return result;
}


PRIVATE int DCALL
operation_mode_format(int argc, char **argv) {
 int i;
#ifndef CONFIG_NO_THREADS
 /* Acquire a lock to the compiler sub-system to prevent
  * scripts from spawning new threads and having those threads
  * tinker with the compiler while the main thread is trying to
  * format the original source file. */
 recursive_rwlock_write(&DeeCompiler_Lock);
#endif
 /* Go over all input files and format them individually. */
 for (i = 0; i < argc; ++i)
    if (dchdir_and_format_source_files(argv[i]))
        goto err;
#ifndef CONFIG_NO_THREADS
 recursive_rwlock_endwrite(&DeeCompiler_Lock);
#endif
 return 0;
err:
#ifndef CONFIG_NO_THREADS
 recursive_rwlock_endwrite(&DeeCompiler_Lock);
#endif
 return -1;
}


DECL_END

#endif /* !GUARD_MAIN_C */
