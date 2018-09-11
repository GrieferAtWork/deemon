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
#ifndef GUARD_DEEMON_API_H
#define GUARD_DEEMON_API_H 1
#define __NO_KOS_SYSTEM_HEADERS__ 1
#ifndef _ATFILE_SOURCE
#define _ATFILE_SOURCE 1
#endif

/* Disable garbage */
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_SECURE_NO_WARNINGS 1
#define _CRT_NONSTDC_NO_WARNINGS 1

#if defined(_MSC_VER) && !defined(NDEBUG)
#define _CRTDBG_MAP_ALLOC 1
#endif

#define DEE_VERSION_API      200
#define DEE_VERSION_COMPILER 200
#define DEE_VERSION_REVISION 0


#include <hybrid/compiler.h>
#include <hybrid/typecore.h>
#include <hybrid/host.h>
#include <hybrid/debug-alignment.h>
#include <hybrid/__byteorder.h>

#ifdef __CC__
#include <stddef.h>
#include <stdarg.h>

#if defined(_CRTDBG_MAP_ALLOC) && \
   !defined(__KOS_SYSTEM_HEADERS__)
#include <crtdbg.h>
#endif
#endif /* __CC__ */

#ifdef _MSC_VER
#pragma warning(disable: 4054) /* Cast from function pointer to `void *' */
#pragma warning(disable: 4152) /* Literally the same thing as `4054', but emit for static initializers. */
#endif

DECL_BEGIN

/* #define CONFIG_NO_THREADS 1 */
/* #define CONFIG_NO_STDIO 1 */

/* CONFIG:  Enable tracing of all incref()s and decref()s that
 *          happen to an object over the course of its lifetime.
 *          When deemon shuts down, dump that reference count
 *          history for all dynamically allocated objects that
 *          still haven't been destroyed.
 *       -> Since this includes every incref and decref operation
 *          ever performed on the object, as well as the reference
 *          counter values at that point in time, it becomes fairly
 *          easy to spot the point when the reference counter become
 *          unsynchronized due to the lack of a required decref.
 * WARNING: Since every object in existence is tracked when this is
 *          enabled, combined with the open-ended-ness of the journal
 *          being kept, this option can become quite expensive to leave
 *          enabled, and should only be enabled to help tracking down
 *          reference leaks.
 * WARNING: Don't leave this option enabled when you don't need it!
 *          Using this option disables various fast-pass code options,
 *          as well as induce a _huge_ overhead caused by practically
 *          any kind of operation with objects, as well as significant
 *          hang-times when used with larger code bases (especially
 *          ones with a lot of code-reuse and cross-dependencies)
 *       -> This is only meant for testing small example applications
 *          that have proven to cause reference leaks, in order to
 *          analyze what exactly is causing them. */
#if 0
#define CONFIG_TRACE_REFCHANGES 1
#else
#define CONFIG_NO_TRACE_REFCHANGES 1 
#endif

#ifdef NDEBUG
#define CONFIG_NO_BADREFCNT_CHECKS 1
#else
#ifndef CONFIG_NO_TRACE_REFCHANGES
#define CONFIG_TRACE_REFCHANGES    1
#endif /* !CONFIG_NO_TRACE_REFCHANGES */
#endif


#ifdef CONFIG_TRACE_REFCHANGES
/* Assembly interpreters do not implement the additional
 * overhead required to properly track reference counts.
 * -> So just disable them! */
#undef CONFIG_HAVE_EXEC_ASM
#else
#define CONFIG_NO_TRACE_REFCHANGES 1
#endif

/* TODO: Update changes in the assembly implementation:
 *   - Add support for the new class system (the new class-instructions)
 *   - Add support for `CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS'
 *   - Add support for the new/refactored `defcmember', `getcmember', `getmember', etc. instructions.
 */
#undef CONFIG_HAVE_EXEC_ASM

#undef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
#undef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
#ifndef __OPTIMIZE_SIZE__
#define CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS 1
#define CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS 1
#endif


#if defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__WINDOWS__) || \
    defined(_WIN16) || defined(WIN16) || defined(_WIN32) || defined(WIN32) || \
    defined(_WIN64) || defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
    defined(_WIN32_WCE) || defined(WIN32_WCE)
#define CONFIG_HOST_WINDOWS 1
#endif
#if defined(__unix__) || defined(__unix) || defined(unix)
#define CONFIG_HOST_UNIX 1
#endif


#ifdef CONFIG_HOST_WINDOWS
#ifndef _WIN32_WINNT
/* Limit windows headers to only provide XP stuff. */
#   define _WIN32_WINNT _WIN32_WINNT_WINXP
#endif /* !_WIN32_WINNT */
#endif


#define CONFIG_HOST_ENDIAN  __BYTE_ORDER__
#if CONFIG_HOST_ENDIAN == __ORDER_LITTLE_ENDIAN__
#   define CONFIG_LITTLE_ENDIAN 1
#elif CONFIG_HOST_ENDIAN == __ORDER_BIG_ENDIAN__
#   define CONFIG_BIG_ENDIAN 1
#endif


#ifdef CONFIG_BUILDING_DEEMON
#if (defined(__i386__) && !defined(__x86_64__)) && \
     defined(CONFIG_HOST_WINDOWS)
#if 0
#define ASSEMBLY_NAME(x,s) PP_CAT4(__USER_LABEL_PREFIX__,x,@,s)
#else
#define ASSEMBLY_NAME(x,s) PP_CAT2(__USER_LABEL_PREFIX__,x@s)
#endif
#else
#define ASSEMBLY_NAME(x,s) PP_CAT2(__USER_LABEL_PREFIX__,x)
#endif
#endif /* CONFIG_BUILDING_DEEMON */

#if defined(__i386__) && !defined(__x86_64__)
/* The `va_list' structure is simply a pointer into the argument list,
 * where arguments can be indexed by alignment of at least sizeof(void *).
 * This allows one to do the following:
 * >> void DCALL function_a(size_t argc, void **argv) {
 * >>      size_t i;
 * >>      for (i = 0; i < argc; ++i)
 * >>          printf("argv[%lu] = %p\n",(unsigned long)i,argv[i]);
 * >> }
 * >> #ifndef __NO_DEFINE_ALIAS
 * >> DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(function_b,8),
 * >>                     ASSEMBLY_NAME(function_a,8));
 * >> #else
 * >> void DCALL function_b(size_t argc, va_list args) {
 * >>      function_a(argc,(void **)args);
 * >> }
 * >> #endif
 * >> void function_c(size_t argc, ...) {
 * >>      va_list args;
 * >>      va_start(args,argc);
 * >>      function_b(argc,args);
 * >>      va_end(args,argc);
 * >> }
 * Using this internally, we can greatly optimize calls to functions
 * like `DeeObject_CallPack()' by not needing to pack everything together
 * into a temporary vector that would have to be allocated on the heap.
 */
#define CONFIG_VA_LIST_IS_STACK_POINTER 1
#endif



#ifdef __CC__

#if defined(__INTELLISENSE__) && defined(__cplusplus)
/* Highlight invalid usage of `NULL' in functions returning `int' */
#undef NULL
#define NULL nullptr
#endif

#ifdef _MSC_VER
extern void (__debugbreak)(void);
#pragma intrinsic(__debugbreak)
#define BREAKPOINT()           __debugbreak()
#elif defined(__COMPILER_HAVE_GCC_ASM) && (defined(__i386__) || defined(__x86_64__))
#define BREAKPOINT()  XBLOCK({ __asm__ __volatile__("int {$}3" : ); (void)0; })
#else
#define BREAKPOINT()          (void)0
#endif

#ifdef CONFIG_BUILDING_DEEMON
#   define DFUNDEF __EXPDEF
#   define DDATDEF __EXPDEF
#else
#   define DFUNDEF __IMPDEF
#   define DDATDEF __IMPDEF
#endif

#ifdef __GNUC__
/* Define if the compiler allows labels to be
 * addressed: `foo: printf("foo = %p",&&foo);'
 * ... as well as such addresses to be used
 * by `goto': `void *ip = &&foo; goto *ip;' */
#define CONFIG_COMPILER_HAVE_ADDRESSIBLE_LABELS 1
#endif

#define DCALL    ATTR_STDCALL
#define DREF     /* Annotation for pointer: transfer/storage of a reference.
                  * NOTE: When returned by a function, a return value
                  *       of NULL indicates a newly raised exception. */

#ifdef _MSC_VER
#pragma warning(disable: 4201)
#pragma warning(disable: 4510)
#pragma warning(disable: 4512)
#pragma warning(disable: 4565)
#pragma warning(disable: 4610)
#endif

#if !defined(NDEBUG) && !defined(CONFIG_NO_CHECKMEMORY) && 1
#ifdef CONFIG_HOST_WINDOWS
#define CONFIG_OUTPUTDEBUGSTRINGA_DEFINED 1
extern ATTR_DLLIMPORT void ATTR_STDCALL OutputDebugStringA(char const *lpOutputString);
extern ATTR_DLLIMPORT int ATTR_STDCALL IsDebuggerPresent(void);
#define DEE_DPRINT(str)    (DBG_ALIGNMENT_DISABLE(),IsDebuggerPresent() ? OutputDebugStringA(str) : (void)0,DBG_ALIGNMENT_ENABLE())
#ifdef _MSC_VER
#define DEE_CHECKMEMORY()  (DBG_ALIGNMENT_DISABLE(),_CrtCheckMemory(),DBG_ALIGNMENT_ENABLE())
#if !defined(_MSC_VER) || defined(_DLL)
extern ATTR_DLLIMPORT int ATTR_CDECL _CrtCheckMemory(void);
#else
extern int ATTR_CDECL _CrtCheckMemory(void);
#endif
#endif /* _MSC_VER */
#endif
#endif /* !NDEBUG */

#ifdef __INTELLISENSE__
extern "C++" template<class T> T ____INTELLISENSE_req_type(T x);
#define REQUIRES_TYPE(T,x)  ____INTELLISENSE_req_type< T >(x)
#else
#define REQUIRES_TYPE(T,x) (x)
#endif

#ifndef NDEBUG
#define DEE_DPRINTF  Dee_dprintf
#define DEE_VDPRINTF Dee_vdprintf
DFUNDEF void (Dee_dprintf)(char const *__restrict format, ...);
DFUNDEF void (DCALL Dee_vdprintf)(char const *__restrict format, va_list args);
#endif


#ifndef NO_ASSERT
#ifndef NDEBUG
DFUNDEF void (DCALL DeeAssert_Fail)(char const *expr, char const *file, int line);
DFUNDEF void (DeeAssert_Failf)(char const *expr, char const *file, int line, char const *format, ...);
#define ASSERT(expr)      ((expr) || (DeeAssert_Fail(#expr,__FILE__,__LINE__),BREAKPOINT(),0))
#define ASSERTF(expr,...) ((expr) || (DeeAssert_Failf(#expr,__FILE__,__LINE__,__VA_ARGS__),BREAKPOINT(),0))
#elif !defined(__NO_builtin_assume)
#define ASSERT(expr)       __builtin_assume(expr)
#define ASSERTF(expr,...)  __builtin_assume(expr)
#else
#define ASSERT(expr)      (void)0
#define ASSERTF(expr,...) (void)0
#endif
#endif /* !NO_ASSERT */




#ifndef DEE_DPRINT
#define DEE_DPRINT(str) DEE_DPRINTF(str)
#endif
#ifndef DEE_DPRINTF
#define DEE_NO_DPRINTF             1
#define DEE_DPRINTF(...)          (void)0
#define DEE_VDPRINTF(format,args) (void)0
#endif

#ifndef DEE_CHECKMEMORY
#define DEE_NO_CHECKMEMORY         1
#define DEE_CHECKMEMORY()         (void)0
#endif

#endif /* __CC__ */



/* Doc string formatting:
 *
 *    - \   Used as escape character to prevent the next character from being
 *          recognized as a special documentation token.
 *          This character is deleted from the generated text, but is not
 *          recognized as an escape character within an specific doc option.
 *          e.g.: "Therefor\\:this is an example"
 *
 *    - :ident
 *    - :{ident}
 *        - `ident' may only contain `UNICODE_FSYMSTRT->UNICODE_FSYMCONT'
 *          characters, as well as `:' and `.' characters.
 *          If this could lead to ambiguity (e.g.: a reference at the end of a
 *          sentence, meaning that the next text-character is a `.'), `ident'
 *          may be wrapped by {...}, as shown above.
 *        - Reference to `ident' (a symbol name), searched for in the following
 *          places, in order. If the `:' character is not followed by a `UNICODE_FSYMSTRT'
 *          character, no reference should be detected and the `:' should be included
 *          in the documentation text.
 *          e.g.: "Because it inherits from :sequence a lot of predefined functions are available"
 *        - If `ident' contains another `:' character, it separates the identifier into 2 parts:
 *             `:{module:item}'  --> `module' and `item'
 *          The left part is interpreted as the module name as also accepted by the `import()' expression.
 *          It may however also be a relative module name (starting with a `.') which then refers
 *          to another module that is relative to the one containing the doc string.
 *        - If no `:' is used to explicitly state a module, the object referenced is search for as follows:
 *            #1: Search the module of the type referring to the documentation string.
 *            #2: Search all modules imported by that module, but don't do so recursively.
 *            #3: Search the builtin `deemon' module if it wasn't already searched.
 *            #4: Recursively search all builtin errors reachable from `Error from deemon' and `Signal from deemon'
 *            #5: Try to open a module with the specified name.
 *        - The `item' part, or `ident' itself when no `module' was specified may contain
 *          additional `.' characters to specify a specific attribute which should be
 *          referenced instead.
 *          e.g.: "This implementation differs from :deemon:sequence.find in that it accepts @start and @end arguments."
 *        - When an attribute is known for both the type and instance, the type's is preferred.
 *          e.g.: "This refers to the type and not the method: :dict.keys"
 *
 *    - @ident
 *    - @{ident}
 *        - `ident' may only contain `UNICODE_FSYMSTRT->UNICODE_FSYMCONT' characters.
 *        - Used to refer to an argument while inside the documentation text of a function.
 *          `ident' should be the name of the argument in this case.
 *          Additionally, `ident' may be `this' to refer to the self-argument in
 *          a member function that is implemented as a this-call.
 *
 *    - #ident
 *    - #{ident}
 *        - `ident' may only contain `UNICODE_FSYMSTRT->UNICODE_FSYMCONT' characters.
 *        - Used to refer to another attribute `ident' within the
 *          same type (member-doc) / module (module-symbol-doc)
 *
 *    - $code
 *    - ${code}
 *    - [\n[optional_whitespace]$code\n]...
 *    - [\n[optional_whitespace]>code\n]...
 *        - The last form will automatically remove whitespace before `$' and
 *          any amount of whitespace following that is shared by all lines before
 *          appending everything together.
 *          Note that the form of `${code}' counts the number of `{' and `}'
 *          characters, allowing code printing of code line `${ { local x = foo; } }'
 *          Also: Any leading/trailing whitespace of code is automatically removed.
 *          e.g.: "(string other) -> int\n"
 *                " Find @other within @this\n"
 *                " $print \"foo\".find("o"); // 1\n"
 *                " $print \"foo\".find("r"); // -1\n"
 *          WARNING: Code blocks are still subject to processing rules of any other
 *                   text, meaning that they may be terminated prematurely by
 *                   incorrect documentation strings.
 *          Code portions of documentation strings are still subject to documentation
 *          interpretation, meaning that if the documentation parser expects a line-feed,
 *          the code block may be terminated prematurely.
 *
 *    - %ident
 *    - %{ident}
 *        Documentation format command:
 *        - %{table ['|' ~~ COLUMN_NAMES...] \n
 *            \n ~~ ['|' ~~ COLUMN_TEXT...]...}
 *          Both `COLUMN_NAMES' and `COLUMN_TEXT' are processed as regular text.
 *          The only special thing is that `|' is used to switch to the next column,
 *          and line-feeds (if not escaped) are used to go to the next row.
 *          Whitespace surrounding '|' is removed
 *        - %{html text...}
 *          Emit the whitespace-stripped text from `text...' as HTML
 *        - %{href link text...}
 *          Emit a hyperlink to `link', with `text' being the link's text.
 *
 *
 *    - \n[optional_whitespace]@ident[:][...]\n
 *        - An optional `:' character may be part of `ident' must is simply discarded.
 *        - Special parsing of tags found at the start of a line.
 *          Note that if any text found in following lines with
 *          a leading indentation that reaches at least until the end of the
 *          tag name is considered part of the line and re-formatted as follows:
 *          >> "@return: -1: The item could not be found\n"
 *             "        The reason for this is that it doesn't exist\n"
 *             "       This line is no longer part of the return-doc text\n"
 *          Same as:
 *          >> "@return: -1: The item could not be found. The reason for this is that it doesn't exist\n"
 *             "      This line is no longer part of the return-doc text\n"
 *          NOTE: If the first non-whitespace character of the following line
 *                is written in uppercase, a missing '.' is appended to the
 *                previous line before the two are added together.
 *                Additionally, any trailing whitespace is removed from the first
 *                line, while any leading whitespace is removed from the second,
 *                before a single space-character is inserted in-between.
 *          NOTE: Tab characters count as 4 space characters for this calculation.
 *        - Note that this kind of recognition supersedes references
 *          to arguments in function documentation strings.
 *        - Based on `ident', the leading portion of the text
 *          associated with the tag may be parsed differently.
 *          For this reason, the following tags are recognized specifically:
 *          - @throw [type :] text
 *          - @throws [type :] text
 *            Process `type' as an object type that may be thrown by the documented
 *            object, processing it as though it was written as `:{type}'
 *            Note that `type' may not contain any whitespace characters unless
 *            it is followed by another ':', in which case everything but the
 *            following `:' and optional whitespace immediate before are part of the type.
 *            When no ':' is found after at most 4 individual groups of whitespace characters
 *            have been encountered, the tag is assumed not to contain a type reference
 *            and the entirety of the text following the @throw[s][:] tag is interpreted
 *            as a human-readable string describing behavior when any kind of object is
 *            thrown by the function.
 *            NOTE: Because `:' may also appear in `type' naturally because it
 *                  may be referring to an object in another module, `:' should
 *                  be followed by whitespace, should it be used to name a
 *                  specific type that is thrown:
 *               OK:    >> @throws deemon:Error: An error
 *               OK:    >> @throws Error: An error
 *               Also OK (because only a single ':' is parsed as part of `type'):
 *                      >> @throws deemon:Error:An error
 *               WRONG: >> @throws Error:An error
 *          - @return [expr :] text
 *          - @returns [expr :] text
 *            Parsing of `expr' and `text' follows the same rules as the parsing
 *            or `type' and `text' in the @throw tag, except that the resulting
 *            `expr' is instead processed as though it was written as `${expr}'
 *            NOTE: When `expr' after being stripped of whitespace is equal to
 *                  a single '*' character, the tag behaves the same as though
 *                 `expr' wasn't given (aka. documenting a general-purpose return value)
 *          - @arg[:] ident [:] text
 *          - @param[:] ident [:] text
 *            Document a specific argument taken by a function.
 *            `ident' may not chain any whitespace and is
 *            interpreted as though it was written as `@{ident}'.
 *          - @interrupt
 *            Same as `@throw Signal.Interrupt: The calling :thread was interrupted'
 *
 *
 *
 * ----------------------  Documentation parsing:
 *
 * // How to read (I know that a doc needing a doc is bad, but it's really not that complicated...):
 * //  x       ::= y; - Define rule x as y
 * //  @x      ::= y; - Define a special rule x as y, where its meaning can be deduced by the human-readable name `x'
 * //  x{args} ::= y; - Define rule x as y, alongside a comma-separated list of names which are replaced within `y' each time `x' is invoked.
 * //  x              - Reference to another rule (imagine it being replaced with the other rule's content)
 * //  x{args}        - Replace with another rule `x{args}', using comma-separated specs from `args' to fill in occurences in `y' of operands named in the definition.
 * //  <x>            - x should be interpreted as human-readable text describing some special behavior.
 * //  'x'            - x is a character / character sequence
 * //  (x)            - Parenthesis to prevent ambiguity
 * //  [x]            - x is optional
 * //  a b            - Characters or tokens a and b are whitespace separated
 * //  a ## b         - a and b follow each other directly (not whitespace separated)
 * //  x ~~ y...      - Only starting at the second occurrence of y, y must always be preceded by x
 * //  x ~~ y##...    - Only starting at the second occurrence of y, y must always be preceded by x. Instances are not whitespace separated
 * //  x...           - x can be repeated an unlimited about of times (at least once)
 * //  x##...         - x can be repeated an unlimited about of times (at least once). Instances are not whitespace separated
 * //  a ## b...      - same as "a ## ('' ~~ b...)"
 * //  a... ## b...   - same as "('' ~~ a...) ## ('' ~~ b...)"
 * //  a... ## b      - same as "('' ~~ a...) ## b"
 * //  'a'...'b'      - Range of characters between a and b (including a and b) (same as a|a+1|a+2|a+3|...|b)
 * //  x | y          - Either x or y
 *
 * // NOTES:
 * //   - Any number of whitespace/linefeed characters must be appended to the
 * //     end of a documentation string if doing so should prevent a parser error.
 * //     Additionally, any number of superfluous whitespace/linefeed at the end
 * //     should be ignored.
 * //   - Leading whitespace-only or empty lines are removed
 * //     from, and should be ignored in doc strings.
 *
 *
 * get_function_name() -> <The name of the attribute/operator/type/module documented by this string>
 *
 * text                ::= <Any sequence of characters not matching the following token.
 *                          Additionally, `\' may be used to escape the following character
 *                          to prevent it being detected as matching the next token.
 *                          Furthermore, text is subjected to the formatting described above.
 *                          The interpreter may choose to append a '.' character to terminate
 *                          a sentence if it deems doing so appropriate>
 *                     ;
 *
 * type                ::= text; // A type string.
 *                               // Should be interpreted as text reformatted as `:{text}'. (aka. as a reference)
 *                               // NOTE: If this string starts with `:', `$' or `@', it is interpreted as-is (and not reformatted)
 *
 * rule_prototype      ::= [ident = get_function_name()]
 *                         ['(' [ ',' ~~ ([type ' ']... text ['=' text]) ')')...]
 *                         //              ^            ^         ^
 *                         //              |            |         +- Default argument value (Interpreted as `')
 *                         //              |            +----------- argument name (The last string before `,' or `)'
 *                         //              |                         following a list of other white-space separated strings)
 *                         //              +------------------------ argument type (When not given, default to `object')
 *                         ['->' type] // Return type. When omit, this may also be deduced from its context.
 *                                     //              e.g.: `operator str() -> string', `class my_class { this() -> my_class }', etc.
 *                                     //              Otherwise, it defaults to `-> object'
 *                                     // NOTE: When the non-reformatted `type' starts with `:', `$' or `@', an implicit
 *                                     //       documentation line is added to the associated text, so long that that text
 *                                     //       does not already contain a `@return' / `@returns' tag:
 *                                     //       "@return * : Always returns type" -- where `type' is replaced with `type' found after `->'
 *                                     // e.g.: "() -> ${42}" -- formatted to "() -> ${42}\n@return * : Always returns ${42}"
 *                         '\n'
 *                     ;
 *
 * rule_overload_group{prefix}
 *                     ::= ([prefix] rule_prototype)...
 *                         [(text '\n')...] // Documentation text for the prototypes listed before.
 *                                          // Lines are appended to each other and each line must
 *                                          // start with at least a single whitespace character.
 *                                          // The amount of space removed is `(for (local x: lines) #x-#x.lstrip()) < ...'
 *                                          // or in other words: the least amount of whitespace found at the start of any line.
 *                         '\n'             // Terminate with a line-feed, meaning that prototype-groups are separated by an empty line
 *                     ;
 *
 * // Documentation string for `DeeTypeObject::tp_doc'
 * @rule_type_doc      ::= [(text '\n')...]                           // Generic text describing this type
 *                         [(rule_overload_group { 'class' | 'this' } // Documentation for constructor overloads
 *                                                                    // NOTE: Also invoked when the name of the class is used as prefix.
 *                         | rule_overload_group { 'operator' }       // Documentation for operators implemented
 *                                                                    // NOTE: These overloads require `get_function_name()'
 *                                                                    //       to be used, which should be interpreted as
 *                                                                    //       either the `oi_uname' or `oi_sname' as obtainable
 *                                                                    //       using the `Dee_OperatorInfo()' API function.
 *                           )...]
 *                     ;
 *
 * // Documentation string for methods (e.g.: `struct type_method::m_doc')
 * @rule_method_doc    ::= rule_overload_group { 'function' };
 *
 * // Documentation string for getsets (e.g.: `struct type_getset::gs_doc')
 * @rule_getset_doc    ::= ['->' type '\n'] // Type of object referred to by the getset
 *                         [(text '\n')...] // Documentation text.
 *                                          // Any amount of leading whitespace shared by all lines is yet again removed.
 *                     ;
 *
 * // Documentation string for members (e.g.: `struct type_member::m_doc')
 * @rule_member_doc    ::= ['->' type '\n'] // The type that a human may reasonably expect this member to refer to
 *                                          // Note that the types of constant/structure members don't necessarily
 *                                          // need to specify this information, as their typing can already be
 *                                          // determined from other factors, unless their type is `STRUCT_OBJECT'.
 *                         [(text '\n')...] // Documentation text.
 *                                          // Any amount of leading whitespace shared by all lines is yet again removed.
 *                     ;
 *
 *
 * ---------------------------------------------------------------------------------
 *
 * // User-code may add documentation strings then subject to the
 * // processing rules described above in the following places,
 * // using one of two syntax options:
 * @doc_attribute ::= ( ('@' string)
 *                    | ('@' 'doc' '(' string ')')
 *                    | ('@' 'doc' '(' 'auto' ')') // Same as `@doc("")', adding an empty line, but causing the doc-attribute to
 *                                                 // not be empty, and therefor the compiler to try and auto-complete the string.
 *                    );
 * // NOTE: No documentation string are ever generated when
 * //       the effective documentation string is empty.
 * // REMINDER: `@doc("")' will actually add "\n" to the doc-string, so that changes it to non-empty
 * // REMINDER: `@doc(auto)' is the same as `@doc("")' and will cause the compiler to try and auto-complete the string.
 *
 * // Defining multiple doc attributes will cause strings to be
 * // appended to each other, using a line-feed as separator.
 * // >> @"Line 1"
 * // >> @"Line 2"
 * // Is the same as
 * // >> @"Line 1\nLine 2"
 *
 * >>
 * >> // Document a global symbol (a function in this case).
 * >> @"(int a, int b) -> int"
 * >> @"@return: * : The sum of @a and @b"
 * >> @"Adds together 2 integers @a and @b and return the result"
 * >> function add(a,b) {
 * >>     return a+b;
 * >> }
 * >>
 * >> @"This is my class"
 * >> class my_class {
 * >>
 * >>     // This doc string is internally appended to that of the class itself
 * >>     // in such a way that 2 empty lines of intermediate text are produced,
 * >>     // thus ensuring that documentation strings for different operators
 * >>     // are separated from each other.
 * >>     @"this()"
 * >>     @"Creates a new instance of :my_class"
 * >>     this(args...) {
 * >>     }
 * >>
 * >>     // Special case: The compiler will automatically notice that this
 * >>     //               documentation string referrs to the __str__ operator
 * >>     //               and will therefor prepend `str' before lines starting
 * >>     //               with a `(' character, thus ensuring that the string,
 * >>     //               when later added to the class's main documentation
 * >>     //               string, remains consistent.
 * >>     @"() -> string"
 * >>     @"Returns a string representation of my_class"
 * >>     operator str {
 * >>        return "Instance of my_class";
 * >>     }
 * >>
 * >>     // Special case: The compiler notices that no prototype is included
 * >>     //               in an operator documentation string and will automatically
 * >>     //               add it, changing this doc-string to "bool()\nAlways returns true"
 * >>     @"Always returns true"
 * >>     operator bool {
 * >>        return true;
 * >>     }
 * >>
 * >>     // Documentation string for a class member.
 * >>     // Special case: The compiler notice that no type information is included
 * >>     //               with the documentation string, but is still able to predict
 * >>     //               the exact type used by this member initializer.
 * >>     //               It therefor chances the documentation string to
 * >>     //               "->list\nContains a list of items"
 * >>     @"Contains a list of items"
 * >>     public items = list();
 * >>
 * >>
 * >>     // Being able to see the implementation of the function, as well
 * >>     // as the fact that no prototype information is contained in the
 * >>     // documentation string, the compiler changes to the string to
 * >>     // "(a,b)->none\ndoesn't do anything"
 * >>     @"doesn't do anything"
 * >>     foo(a,b) {
 * >>     }
 * >>
 * >> };
 */

/* NOTE: This config option only affects internal documentation strings. */
#ifndef CONFIG_NO_DOC
#   define DOC(x)             x
#   define DOC_DEF(name,x)    PRIVATE char const name[] = x
#   define DOC_GET(name)      name
#else
#   define DOC(x)           ((char *)NULL)
#   define DOC_DEF(name,x)    /* nothing */
#   define DOC_GET(name)    ((char *)NULL)
#endif

DECL_END

#endif /* !GUARD_DEEMON_API_H */
