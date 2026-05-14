/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    in a product, an acknowledgement (see the following) in the product     *
 *    documentation is required:                                              *
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
/*!export **/
/*!export DeeBuiltin_**/
/*!export DeeModule**/
/*!export Dee_COMPILER_ERROR_FATALITY_**/
/*!export Dee_MODSYM_F**/
/*!export Dee_MODULE_F**/
/*!export Dee_module_**/
/*!export _Dee_MODULE_**/
#ifndef GUARD_DEEMON_MODULE_H
#define GUARD_DEEMON_MODULE_H 1 /*!export-*/

#include "api.h"

#include <hybrid/typecore.h> /* __*_TYPE__, __SIZEOF_SIZE_T__ */

#include "gc.h"              /* Dee_gc_head */
#include "object.h"          /* DeeObject_NewRef */
#include "string.h"          /* DeeString*, Dee_STRING_ERROR_FIGNORE */
#include "system-features.h" /* bcmp, strlen */
#include "types.h"           /* DREF, DeeObject, DeeObject_InstanceOf, DeeTypeObject, Dee_AsObject, Dee_OBJECT_HEAD, Dee_REQUIRES_OBJECT, Dee_WEAKREF_SUPPORT, Dee_formatprinter_t, Dee_hash_t, Dee_ssize_t, ITER_DONE */
#include "util/hash.h"       /* Dee_HashPtr, Dee_HashStr */
#include "util/lock.h"       /* Dee_ATOMIC_RWLOCK_INIT, Dee_atomic_rwlock_* */

#include <stdarg.h>  /* va_list */
#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uintN_t, uintptr_t */

#ifndef __INTELLISENSE__
#include "alloc.h" /* Dee_Free, Dee_Malloc, Dee_TryMalloc */
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

/*
 * Module names can be imported using import strings using 1 of 4 flavors:
 *
 * #1: Absolute, or relative filesystem names
 *     Used if:
 *        The import string contains a "/" (or "\\") character, or
 *        more specifically: `DeeSystem_SEP' or `DeeSystem_ALTSEP'
 *     Type:
 *        If filename ends with ".so" (or ".dll"): DEX module
 *        Else, deemon source file or directory
 *     Examples:
 *        >> import mod = "E:\\projects\\deemon\\script.dee";
 *        >> import mod = "E:\\projects\\deemon\\file.txt";
 *        >> import mod = "./script.dee";
 *        >> import mod = "./file.txt";
 *
 * #2: Absolute, or relative filesystem names, without extension
 *     Examples:
 *        >> import mod = "E:\\projects\\deemon\\script";
 *        >> import mod = "./script";
 *     Same as case #1, but deemon will automatically check for
 *     relevant extensions ".dee", ".so" (or ".dll"), and then
 *     one last time without any extension. The first time that
 *     is found to actually exist during this part is then used,
 *     with the extension implying the type of module:
 *        1. ".dee"  Deemon source file (including check/creation of ".*.dec" file)
 *        2. ".so"   Native DEX module
 *        3. ""      As-is (can either be a deemon source file, or a directory)
 *
 * #3: Relative "dot"-encoded module names
 *     Used if:
 *        The import string does not contain "/" (or "\\")
 *        The import string starts with a leading "."
 *     Examples:
 *        >> import mod = ".";     // "./{CURRENT_FILE}.dee"
 *        >> import mod = "..";    // "./."        (directory containing current file)
 *        >> import mod = ".foo";  // "./foo.dee"  (sibling file)
 *        >> import mod = "..";    // when called from a file "/script.dee" on linux:      "<Filesystem root module>"
 *        >> import mod = "...";   // when called from a file "C:\\script.dee" on windows: "<Filesystem root module>"
 *     Relative "dot"-encoded module names are used to identify:
 *      1. The calling module itself when "." is used as import string
 *      2. The containing directory (recursive) when consisting of only "." characters:
 *         - 1 dot is handled by case #1
 *         - 2 dots mean "{headof(CURRENT_FILE)}"
 *         - 3 dots mean "{headof(CURRENT_FILE)}/.."
 *         - 4 dots mean "{headof(CURRENT_FILE)}/../.."
 *         - Trying to go beyond the filesystem root is not allowed, but...
 *         - ... on windows, the filesystem root is located one folder above
 *           the root of any any drive
 *      2. Import strings containing more than just "." characters:
 *         - Leading dots are handled the as for case #2 to construct the location
 *           of a "directory" relative to which the remainder of the import string
 *           is then evaluated
 *         - The remainder of the import string takes the form "foo.bar.baz", which
 *           then gets translated to "{BASE_DIR_FROM_LEADING_DOTS}/foo/bar/baz.dee"
 *         - It is an ERROR for this type of import string to:
 *           - End with trailing "." characters:            "...dir.script."
 *           - Contain multiple consecutive "." characters: "...dir..script"
 *     Note that the initial context of a dot-encoded module name always
 *     qualifies a **SIBLING** of the context-module: (examples are from "/home/me/script.dee")
 *     >> import . as me;      // "/home/me/script.dee"
 *     >> import .foo as sib;  // "/home/me/foo.dee"         // Sibling module!
 *     >> local sub = me.bar;  // "/home/me/script/bar.dee"  // Child module!
 *
 * #4: LIBPATH-based module names (with optional "dot"-sub-references)
 *     Used if:
 *        The import string does not contain "/" (or "\\")
 *        The import string does not starts with a leading "."
 *     Examples:
 *        >> import mod = deemon;        // Special case: "DeeModule_Deemon" (always exists, even if LIBPATH is empty)
 *        >> import mod = util;          // "{LIBPATH}/util.dee"
 *        >> import mod = rt.gen.unpack; // "{LIBPATH}/rt/gen/unpack.dee"
 *     This type of import string is resolved alongside a list of path names
 *     stored within `DeeModule_GetLibPath()'.
 *        - By default, this list contains only a single element "{DeeExec_GetHome()}/lib"
 *        - The environ variable $DEEMON_PATH can be used to add more paths
 *        - The environ variable $DEEMON_HOME can be used to override "DeeExec_GetHome()"
 *     When resolving LIBPATH-based module names, the list of strings from
 *     `DeeModule_GetLibPath()' is enumerated in ascending order, and filenames
 *     are constructed using the same mechanism as documented under #3.2, using
 *     the string from `DeeModule_GetLibPath()' as the "directory", and the
 *     given import string as-is as the remainder.
 *
 *
 * Notes on module "children":
 * - Since directories are considered as valid modules (though the following does not
 *   only apply to directory modules, but all modules that do not have a non-standard
 *   extension (e.g. "./script.txt")), the names of modules within a directory matching
 *   the name of the module (or a properly named directory matching the name of a ".dee"
 *   or ".so" module with the extension removed) can be accessed
 *   as attributes of that module:
 *   Files (in /home/me/deemon):
 *     |
 *     +--- main.dee
 *     |
 *     +--- script.dee
 *     |
 *     +--- script
 *     |    |
 *     |    +--- foo.dee
 *     |    |
 *     |    +--- bar.dee
 *     |
 *     +--- somedir
 *          |
 *          +--- baz.dee
 *    $ cat script.dee
 *    >> global foo = 42;
 *
 *    $ cat main.dee
 *    >> import . as me;
 *    >> import .script;
 *    >> import .script.foo;
 *    >> import .somedir;
 *    >> print me;          // <Module /home/me/deemon/main>
 *    >> print script;      // <Module /home/me/deemon/script>
 *    >> print script.foo;  // 42                                  <-- *real* globals always overrule module children
 *    >> print foo;         // <Module /home/me/deemon/script/foo>
 *    >> print script.bar;  // <Module /home/me/deemon/script/bar>
 *    >> print somedir;     // <Module /home/me/deemon/somedir>
 *    >> print somedir.baz; // <Module /home/me/deemon/somedir/baz>
 *    >>
 *    >> import * from .script;   // Works, but only imports "global foo = 42"
 *    >> import bar from .script; // Compile error: "sym from module" cannot be used to import child modules (this may change in the future)
 *
 *
 * Notes on the compiler's "import" keyword:
 * - The "import" keyword can be used in a number of different ways:
 *   Statements:
 *       >> import foo;                           // local foo = import("foo");
 *       >> import .foo;                          // local foo = import(".foo");
 *       >> import . as me;                       // local me = import(".");
 *       >> import me = .;                        // local me = import(".");
 *       >> import d = deemon;                    // local d = import("deemon");
 *       >> import deemon as d;                   // local d = import("deemon");
 *       >> import compare from deemon;           // <alias> local compare = import("deemon").compare;
 *       >> from deemon import compare;           // <alias> local compare = import("deemon").compare;
 *       >> import comp = compare from deemon;    // <alias> local comp = import("deemon").compare;
 *       >> import compare as comp from deemon;   // <alias> local comp = import("deemon").compare;
 *       >> from deemon import comp = compare;    // <alias> local comp = import("deemon").compare;
 *       >> from deemon import compare as comp;   // <alias> local comp = import("deemon").compare;
 *       >> from deemon import *;                 // <alias> <define locals for all symbols from "deemon" that don't already exist>
 *       >> import * from deemon;                 // <alias> <define locals for all symbols from "deemon" that don't already exist>
 *     NOTE: "<alias>" means that writes to the symbol that go to the source, rather than override a local
 *   Expressions:
 *       >> import("{IMPORT_STRING}");            // Evaluate "{IMPORT_STRING}" (using the caller's module as context for relative imports), and return the result
 *       >> import.foo;                           // import("foo");
 *       >> import.foo.bar;                       // import("foo").bar;
 *     Note the absence of whitespace between "import" and "." in the last 2 cases!
 *     If the compiler finds there to be whitespace after "import" (and the piece
 *     of code is found in a context where statements are allowed), then the meaning
 *     will be different:
 *       >> import.foo;               // import("foo");
 *       >> import .foo;              // local foo = import("foo");
 *       >> (import .foo);            // import("foo");
 *       >> local foo = import.foo;   // local foo = import("foo");
 *       >> local foo = import .foo;  // local foo = import("foo");
 *     As such, you can think of "import." being kind-of a different token than "import"
 *     TODO: The "import.xxx" syntax is not yet implemented
 */

struct Dee_string_object;
struct Dee_code_object;
struct Dee_cmethod_object;
struct Dee_thread_object;
struct Dee_tuple_object;

#define Dee_MODSYM_FNORMAL         0x00 /* Normal symbol flags. */
#define Dee_MODSYM_FREADONLY       0x01 /* Don't allow write-access to this symbol.
                                         * NOTE: This flag should have really been the default. When not
                                         *       set, the behavior is as it is for classes/functions defined
                                         *       as "varying" in user-code (or raw non-final "global foo"
                                         *       variables). As such, you should probably set this flag at
                                         *       least for all exported functions/classes (unless you'd mark
                                         *       the class/function as "varying").
                                         *
                                         * When set, attempting to write/delete to this symbol will cause a
                                         * compiler-error (except for the first assignment when the symbol
                                         * is part of the calling module), or attempting to write/delete at
                                         * runtime and a non-NULL value has already been assigned.
                                         *
                                         * When this flag and `Dee_MODSYM_FCONSTEXPR' are both set, then the
                                         * compiler is allowed (but not required) to initialize the module,
                                         * then propagate this symbol's actual value as a compile-time
                                         * constant expression, should that value be one of the following
                                         * (and for a sequence, containing only such):
                                         *   - DeeInt_Type
                                         *   - DeeString_Type
                                         *   - DeeNone_Type
                                         *   - DeeBool_Type
                                         *   - DeeObject_Type
                                         *   - DeeList_Type
                                         *   - DeeTuple_Type
                                         *   - DeeHashSet_Type
                                         *   - DeeDict_Type
                                         *   - DeeRoDict_Type
                                         *   - DeeRoSet_Type
                                         * NOTE: All white-listed types encode _exact_ matches (aka.
                                         *       `DeeObject_InstanceOfExact()', rather than `DeeObject_InstanceOf()')
                                         * NOTE: Some more additions are made for few more special objects
                                         *       that are not documented here, but the idea should be clear:
                                         *       Nothing that may produce side-effects in an obvious fashion,
                                         *       and nothing that is too complex, or wouldn't make sense.
                                         * WARNING: Despite all of these rules, the basic initialization that
                                         *          leads to some specific value being assigned still remains
                                         *          in the hands of user, meaning that it is the job of a
                                         *          module to make sure that exported constants be always
                                         *          assigned the same values.
                                         * WARNING: This flag cannot be enforced when user-code assembly is
                                         *          used to modify the value of an external/global symbol,
                                         *          meaning that you must still always assume that any
                                         *          module member no longer contains the proper value. */
#define Dee_MODSYM_FCONSTEXPR      0x02 /* May be combined with `Dee_MODSYM_FREADONLY' to allow the compiler to
                                         * propagate this symbol as a constant expression at compile-time,
                                         * so-long as its runtime-value matches the criteria detailed above.
                                         * NOTE: Regardless of this flag, the compiler mustn't propagate the
                                         *       assigned value when `Dee_MODSYM_FPROPERTY' it set. */
#define Dee_MODSYM_FALIAS          0x04 /* This symbol is aliasing another.
                                         *
                                         * This flag is handled by `DeeModule_GlobalName()', which will try
                                         * to continue searching for another member with the same index, but
                                         * without this flag set.
                                         *
                                         * Should such a member not exist, return one of the aliases already
                                         * encountered at random. */
#define Dee_MODSYM_FHIDDEN         0x08 /* Don't enumerate this symbol in export listings or documentations.
                                         * The only way to access it is to know that it exists and call it by
                                         * name (or take apart the source binary to learn what may be there).
                                         *
                                         * This flag is used by some hidden (and implementation-specific)
                                         * helper functions found in the `deemon' module which the compiler
                                         * is allowed to generate helper calls to for stuff that doesn't
                                         * deserve its own opcode due to how rarely its used... */
#define Dee_MODSYM_FPROPERTY       0x10 /* The symbol is a property. */
#define Dee_MODSYM_FEXTERN         0x20 /* Refers to an global variable slot for a different module
                                         * (allowing for forwarding/aliasing). Using this, one can
                                         * implement something along the lines of:
                                         * -     `global import foo = bar from baz;'
                                         * - vs. `local import foo = bar from baz;'
                                         * (Default visibility would be `local') */
#define Dee_MODSYM_FMASK           0x3f /* Mask of known Dee_MODSYM_F* flags (those that are allowed by DEC files). */
#define Dee_MODSYM_FNAMEOBJ        0x40 /* The symbol's name is actually a reference to a string object's `s_str' */
#define Dee_MODSYM_FDOCOBJ         0x80 /* The symbol's doc is actually a reference to a string object's `s_str'
                                         * NOTE: When this flag is set, `ss_doc' is always [1..1] */

#define Dee_MODULE_PROPERTY_GET    0 /* Index offset for property get callbacks. */
#define Dee_MODULE_PROPERTY_DEL    1 /* Index offset for property del callbacks. */
#define Dee_MODULE_PROPERTY_SET    2 /* Index offset for property set callbacks. */

struct Dee_module_symbol {
	/* For the sake of DEX modules, `ss_doc' should be allowed to be a `char const *', with
	 * one of the symbol flags being used to indicate if it's actually an object, which must
	 * be cleaned by `Dee_Decref(COMPILER_CONTAINER_OF(ss_doc, DeeStringObject, s_str))' */
	char const              *ss_name;  /* [0..1] Name of this symbol (NULL marks the sentinel) */
	char const              *ss_doc;   /* [0..1] An optional documentation string. */
	Dee_hash_t               ss_hash;  /* [== Dee_HashStr(ss_name)] Hash-value of this symbol. */
	__UINTPTR_QUARTER_TYPE__ ss_flags; /* Set of `Dee_MODSYM_F*'. */
	__UINTPTR_QUARTER_TYPE__ ss_impid; /* [< :mo_importc][valid_if(Dee_MODSYM_FEXTERN)] The index of the referenced module in the import vector. */
	__UINTPTR_HALF_TYPE__    ss_index; /* [< :mo_globalc][valid_if(!Dee_MODSYM_FEXTERN)] The index of this symbol in the `:mo_globalv' vector.
	                                    * [< :mo_importv[ss_impid]->mo_globalc][valid_if(Dee_MODSYM_FEXTERN)] The index of this symbol in the module's `:mo_globalv' vector.
	                                    * NOTE: In the case of a property, either 1 or 3 indices are allocated,
	                                    *       3 if `Dee_MODSYM_FREADONLY' isn't set, and 1 if it is. */
};

#define Dee_module_symbol_getindex(self) ((uint16_t)(self)->ss_index)

#define Dee_MODULE_SYMBOL_EQUALS(x, name, size)              \
	(bcmp((x)->ss_name, name, (size) * sizeof(char)) == 0 && \
	 (x)->ss_name[size] == 0)
#define Dee_MODULE_SYMBOL_EQUALS_STR(x, string)                                                 \
	(((x)->ss_flags & Dee_MODSYM_FNAMEOBJ)                                                      \
	 ? DeeString_EqualsSTR(string, COMPILER_CONTAINER_OF((x)->ss_name, DeeStringObject, s_str)) \
	 : DeeString_EqualsBuf(string, (x)->ss_name, strlen((x)->ss_name)))
#define Dee_MODULE_SYMBOL_GETNAMESTR(x) ((x)->ss_name)
#define Dee_MODULE_SYMBOL_GETNAMELEN(x) (((x)->ss_flags & Dee_MODSYM_FNAMEOBJ) ? DeeString_SIZE(COMPILER_CONTAINER_OF((x)->ss_name, DeeStringObject, s_str)) : strlen((x)->ss_name))
#define Dee_MODULE_SYMBOL_GETDOCSTR(x)  ((x)->ss_doc)
#define Dee_MODULE_SYMBOL_GETDOCLEN(x)  (((x)->ss_flags & Dee_MODSYM_FDOCOBJ) ? DeeString_SIZE(COMPILER_CONTAINER_OF((x)->ss_doc, DeeStringObject, s_str)) : strlen((x)->ss_doc))
#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1)) DREF struct Dee_string_object *DCALL Dee_module_symbol_getnameobj(struct Dee_module_symbol *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF struct Dee_string_object *DCALL Dee_module_symbol_getdocobj(struct Dee_module_symbol *__restrict self);
#else /* CONFIG_BUILDING_DEEMON */
#define Dee_module_symbol_getnameobj(x) ((DeeStringObject *)(((x)->ss_flags & Dee_MODSYM_FNAMEOBJ) ? DeeObject_NewRef(Dee_AsObject(COMPILER_CONTAINER_OF((x)->ss_name, DeeStringObject, s_str))) : DeeString_NewWithHash((x)->ss_name, (x)->ss_hash)))
#define Dee_module_symbol_getdocobj(x)  ((DeeStringObject *)(((x)->ss_flags & Dee_MODSYM_FDOCOBJ) ? DeeObject_NewRef(Dee_AsObject(COMPILER_CONTAINER_OF((x)->ss_doc, DeeStringObject, s_str))) : DeeString_NewUtf8((x)->ss_doc, strlen((x)->ss_doc), Dee_STRING_ERROR_FIGNORE)))
#endif /* !CONFIG_BUILDING_DEEMON */




struct Dee_compiler_error_object;
/* An optional callback that is invoked immediately before a compiler error is thrown.
 * This function's usual purpose is to immediately print the error to stderr,
 * though theoretically, it could also be used to do something different.
 * @param: fatality_mode: One of `COMPILER_ERROR_FATALITY_*'
 * @return: -1: An error occurred during handling.
 * @return:  0: The error was acknowledged and compilation may continue
 *              if `fatality_mode' is `Dee_COMPILER_ERROR_FATALITY_WARNING'
 *              or `Dee_COMPILER_ERROR_FATALITY_ERROR'.
 * @return:  1: Disregard `Dee_COMPILER_ERROR_FATALITY_ERROR' and continue compilation
 *              as though the error model was `Dee_COMPILER_ERROR_FATALITY_WARNING'.
 *              WARNING: This overrules the user-configuration set by `#pragma warning'
 * @return:  2: Disregard both `Dee_COMPILER_ERROR_FATALITY_FATAL' and
 *             `Dee_COMPILER_ERROR_FATALITY_ERROR' and interpret the error
 *              as a warning, only to be included in a multi-compiler-error
 *              if another compiler error is processed fatally at a later point.
 *              WARNING: This overrules the user-configuration set by `#pragma warning'
 * @return:  3: Ignore the error completely, so long as its fatality_mode isn't
 *             `Dee_COMPILER_ERROR_FATALITY_FORCEFATAL'. This means that it won't be
 *              thrown or scheduled as a warning, but simply discarded as though
 *              it never happened in the first place.
 * NOTES:
 *   - When no handler is set, the behavior is the same as though it always returned `0'
 *   - Upon entry, `error->ce_mode == fatality_mode'
 *   - Depending on return value, `error->ce_mode' is re-written before being saved. */
typedef WUNUSED_T NONNULL_T((1)) int
(DCALL *Dee_compiler_error_handler_t)(struct Dee_compiler_error_object *__restrict error,
                                      int fatality_mode, void *arg);
#define Dee_COMPILER_ERROR_FATALITY_WARNING    0 /* The error is a mere warning and will not cause the
                                                  * the compiler to fail, and neither will it thrown
                                                  * once the function returns.
                                                  * However, should a future error cause the compiler
                                                  * to fail, this warning will still be included in
                                                  * the resulting `DeeCompileErrorObject' error. */
#define Dee_COMPILER_ERROR_FATALITY_ERROR      1 /* The error will be fatal to prevent the compiler
                                                  * from successfully generating working user-code.
                                                  * However, compilation will continue for the time
                                                  * being, in order to collect further errors/warnings
                                                  * until a certain limit has been reached, or until
                                                  * a fatal/force-fatal error is encountered. */
#define Dee_COMPILER_ERROR_FATALITY_FATAL      2 /* The error is fatal, but handling exist to continue
                                                  * parsing code. Yet the user expects compilation to
                                                  * stop without the parser/assembler continuing to
                                                  * produce further errors. */
#define Dee_COMPILER_ERROR_FATALITY_FORCEFATAL 3 /* The error must be processed as fatal. */


/* General-purpose, optional compiler options that
 * can be specified whenever a module is loaded. */
struct Dee_compiler_options {
	struct Dee_compiler_options  *co_inner;             /* [0..1] Options used for compiling modules imported by this one. */
	char const                   *co_pathname;          /* [0..1] A filename used to resolve #include and relative import directives. */
	struct Dee_string_object     *co_filename;          /* [0..1] The filename that should appear in debug information when referring to `input_file'.
	                                                     *        This is also the filename returned by `__FILE__' and `__BASEFILE__',
	                                                     *        if not otherwise overwritten using `#line' */
	struct Dee_string_object     *co_rootname;          /* [0..1] The name of the root code object (as set in DDI) */
	WUNUSED_T int         (DCALL *co_setup)(void *arg); /* [0..1] Called once the compiler has been enabled.
	                                                     *        This callback can be used to perform additional compiler
	                                                     *        initialization, such as defining macros/assertions, or
	                                                     *        adding addition include paths.
	                                                     *        NOTE: Invocation happens after other compiler options have
	                                                     *              all been set and the initial source file has been
	                                                     *              pushed onto the #include-stack, though the first
	                                                     *              token hasn't been yielded, yet.
	                                                     * @return: >= 0: Setup was successful.
	                                                     * @return: < 0: `DeeModule_LoadSourceStream()' will fail with the same error. */
	void                         *co_setup_arg;         /* [?..?] Argument to `co_setup' */
	Dee_compiler_error_handler_t  co_error_handler;     /* [0..1] Called before compiler errors are processed.
	                                                     * This function's usual purpose is to print the error
	                                                     * during live-compilation mode, but can also be used
	                                                     * to put a twist on how errors are actually processed. */
	void                         *co_error_arg;         /* [?..?] Argument to `co_error_handler' */
	uint16_t                      co_tabwidth;          /* The width of tabulators, or `0' to use the hard-coded default. */
	uint16_t                      co_compiler;          /* Set of `COMPILER_F*' from `<deemon/compiler/compiler.h>'. */
	uint16_t                      co_parser;            /* Set of `PARSE_F*'    from `<deemon/compiler/lexer.h>' */
	uint16_t                      co_optimizer;         /* Set of `OPTIMIZE_F*' from `<deemon/compiler/ast.h>' */
	uint16_t                      co_unwind_limit;      /* Limit control for loop unwinding: The max amount of times that
	                                                     * a constant loop may be unwound. (Set to ZERO(0) to disable) */
	uint16_t                      co_assembler;         /* Set of `ASM_F*'      from `<deemon/compiler/assembler.h>' */
#ifndef CONFIG_EXPERIMENTAL_MMAP_DEC
	uint16_t                      co_decwriter;         /* Set of `DEC_WRITE_F*' from `<deemon/compiler/dec.h>' (unused when deemon was built with `CONFIG_NO_DEC') */
	DeeObject                    *co_decoutput;         /* [0..1] Dec output location (ignored when `ASM_FNODEC' is set)
	                                                     *  - Filename (string) of the generated `.dec' file.
	                                                     *  - Stream (any other object) into which to write the contents of the dec file.
	                                                     *  - When `NULL', the filename is selected such that it will be used to
	                                                     *    quickly load module object when one of the dec-enabled `DeeModule_OpenGlobal*'
	                                                     *    functions is used (aka. `<source_path>/.<source_file>.dec')
	                                                     * Note that when passing a string, that file will be overwritten, should it already exists. */
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
};







/* Opening modules under different paths:
 * >> import("deemon");                   // DeeModule_Type      (mo_absname == NULL; DeeModule_GetDeemon())
 * >> import("util");                     // DeeModuleDee_Type   (mo_absname = "/usr/lib/deemon/lib/util")
 * >> import("_codecs");                  // DeeModuleDir_Type   (mo_absname = "/usr/lib/deemon/lib/_codecs"; because there is no "${LIBPATH}/_codecs.dee" file)
 * >> import("net");                      // DeeModuleDex_Type   (mo_absname = "/usr/lib/deemon/lib/net")
 * >> import("rt.d200");                  // DeeModuleDee_Type   (mo_absname = "/usr/lib/deemon/lib/rt/d200")
 * >> import("rt").d200;                  // DeeModuleDee_Type   (mo_absname = "/usr/lib/deemon/lib/rt/d200")
 * >> import(".foo");                     // DeeModuleDee_Type   (mo_absname = "/path/to/current/file/foo")
 * >> import(".foo.dee");                 // DeeModuleDee_Type   (mo_absname = "/path/to/current/file/foo/dee")
 * >> import("./foo");                    // DeeModuleDee_Type   (mo_absname = "/path/to/current/file/foo")
 * >> import("./foo.dee");                // DeeModuleDee_Type   (mo_absname = "/path/to/current/file/foo")
 * >> import("./foo.txt");                // DeeModuleDee_Type   (mo_absname = "/path/to/current/file/foo.txt"; this module doesn't do directory enumeration, and has "Dee_MODULE_FABSFILE" set)
 * >> import(".");                        // DeeModuleDee_Type   (mo_absname = "/path/to/current/file/CURRENT_FILE"; the calling module itself)
 * >> import(".CURRENT_FILE");            // DeeModuleDee_Type   (mo_absname = "/path/to/current/file/CURRENT_FILE"; the calling module itself)
 * >> import(".").foo;                    // DeeModuleDee_Type   (mo_absname = "/path/to/current/file/CURRENT_FILE/foo")
 * >> import(".").operator . ("foo.bar"); // DeeModuleDee_Type   (mo_absname = "/path/to/current/file/CURRENT_FILE/foo.bar")
 * >> import(".foo.bar");                 // DeeModuleDee_Type   (mo_absname = "/path/to/current/file/CURRENT_FILE/foo/bar")
 * >> import("..");                       // DeeModuleDir_Type   (mo_absname = "/path/to/current/file")
 * >> import("..").CURRENT_FILE;          // DeeModuleDee_Type   (mo_absname = "/path/to/current/file/CURRENT_FILE"; the calling module itself)
 * >> import("...");                      // DeeModuleDir_Type   (mo_absname = "/path/to/current")
 *
 * The actual file where the module was loaded from then depends on the module type:
 * - DeeModule_Type:     File wasn't loaded from anywhere (is anonymous, or the "deemon" module)
 * - DeeModuleDee_Type:  f"{mo_absname}.dee"  (and/or f"{fs.headof(mo_absname)}/.{fs.tailof(mo_absname)}.dec"; or f"{mo_absname}" if )
 * - DeeModuleDex_Type:  f"{mo_absname}.so"   (or f"{mo_absname}.dll")
 * - DeeModuleDir_Type:  mo_absname           (it's a directory)
 */

union Dee_module_buildid {
	size_t   mbi_words[16 / __SIZEOF_SIZE_T__];
	uint64_t mbi_word64[2];
	uint32_t mbi_word32[4];
	uint16_t mbi_word16[8];
	uint8_t  mbi_word8[16];
};

struct Dee_module_libentry {
	DREF struct Dee_string_object *mle_name; /* [0..1][lock(INTERNAL(module_libtree_lock))][valid_if(mo_absname)]
	                                          * LIBPATH-name of module (or "NULL" if not a global module, or not
	                                          * yet accessed using the LIBPATH). Cleared for all modules (except
	                                          * for the built-in "deemon" module) whenever the libpath is changed. */
	union {
		struct Dee_module_object  *mle_mod;  /* [1..1][const] The module that this entry is for (least significant bit is R/B-flag) */
		__UINTPTR_TYPE__           mle_red;  /* Least significant bit is red/black bit */
	}                              mle_dat;  /* [valid_if(mle_name)] Data... */
	struct {
		struct Dee_module_libentry *rb_par;  /* [?..?] Parent node */
		struct Dee_module_libentry *rb_lhs;  /* [?..?] Left node */
		struct Dee_module_libentry *rb_rhs;  /* [?..?] Right node */
	}                              mle_node; /* [lock(INTERNAL(module_libtree_lock))][valid_if(mo_absname && mo_libname)]
	                                          * Node in tree of modules-by-mo_libname */
	struct Dee_module_libentry    *mle_next; /* [0..1][valid_if(mle_name)][lock(INTERNAL(module_libtree_lock))][owned] Another name for this module */
};

#define Dee_module_libentry_getmodule(self) \
	((DeeModuleObject *)((__UINTPTR_TYPE__)(self)->mle_dat.mle_mod & ~1))
#ifdef __INTELLISENSE__
#define Dee_module_libentry_tryalloc() ((struct Dee_module_libentry *)(sizeof(struct Dee_module_libentry)))
#define Dee_module_libentry_alloc()    ((struct Dee_module_libentry *)(sizeof(struct Dee_module_libentry)))
#define Dee_module_libentry_free(self) (void)(self)
#else /* __INTELLISENSE__ */
#define Dee_module_libentry_tryalloc() ((struct Dee_module_libentry *)Dee_TryMalloc(sizeof(struct Dee_module_libentry)))
#define Dee_module_libentry_alloc()    ((struct Dee_module_libentry *)Dee_Malloc(sizeof(struct Dee_module_libentry)))
#define Dee_module_libentry_free(self) Dee_Free(self)
#endif /* !__INTELLISENSE__ */

#ifndef CONFIG_NO_DEX
struct Dee_module_dexdata;
#endif /* !CONFIG_NO_DEX */

struct Dee_module_treenode {
	struct Dee_module_object *rb_par;       /* [?..?] Parent node */
	struct Dee_module_object *rb_lhs;       /* [?..?] Left node */
	struct Dee_module_object *rb_rhs;       /* [?..?] Right node */
};

union Dee_module_moddata {
#ifndef CONFIG_NO_DEX
	struct Dee_module_dexdata *mo_dexdata;  /* [1..1][valid_if(DeeModuleDex_Type)][const] Dex data of module */
#define Dee_MODULE_MODDATA_INIT_CODE(c) { (struct Dee_module_dexdata *)Dee_REQUIRES_TYPE(struct Dee_code_object *, c) }
#else /* !CONFIG_NO_DEX */
#define Dee_MODULE_MODDATA_INIT_CODE(c) { c }
#endif /* CONFIG_NO_DEX */
	/* NOTE: The whole of idea of "mo_rootcode" not forming a reference loop with the module itself
	 *       is flawed! This reference loop is actually **NECESSARY** to ensure that modules remain
	 *       loaded even if no-longer used, because importing a module invokes user-code, which may
	 *       have side effects that shouldn't be repeated if the module is imported multiple times.
	 *
	 * Instead, "DeeCodeObject" should be changed such that "co_module" becomes [0..1] (with NULL
	 * being allowed if the code running inside doesn't make use of the module; though the module
	 * can still be determined using `DeeModule_OfObject()' after CONFIG_EXPERIMENTAL_MMAP_DEC),
	 * and `mo_rootcode' of a user-code module may be set to `DeeCode_Empty' if all initialization
	 * can be done statically (iow: the module's global function doesn't have any side-effects).
	 *
	 * Once all that is done, we're automatically at the point where modules that *can* be unloaded
	 * as soon as they aren't used anymore, *will* automatically unload once that happens, too! */
	DREF struct Dee_code_object *mo_rootcode; /* [1..1][valid_if(DeeModuleDee_Type)][lock(:mo_lock)] Root code object
	                                           * If the module is discovered to be unreachable (part of a GC reference
	                                           * loop), then (alongside clearing of globals), this code is replaced with
	                                           * `DeeCode_Empty' in order to break reference loops via `co_module'. */
};

typedef struct Dee_module_object {
	/* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec.gas-386.S' */
	Dee_OBJECT_HEAD /* GC Object. */
	/*utf-8*/ char                *mo_absname;  /* [0..1][owned][const] Absolute, system-specific path to
	                                             * directory containing the module, followed by `DeeSystem_SEP',
	                                             * then followed by the raw name of the module. This string can
	                                             * also be interpreted as a system filename, which (if it exists)
	                                             * points at a directory whose `.dee' and `.so/.dll' files, as
	                                             * well as other sub-directories (except "." and "..") can be
	                                             * enumerated as additional attributes of this module.
	                                             *
	                                             * This string can be used as the host-global, unique identifier
	                                             * for this module, but is set to `NULL' if the module is:
	                                             * - Anonymous (explicitly created by user-code)
	                                             * - The special built-in "deemon" module
	                                             *
	                                             * When executing a file as deemon code that doesn't end with
	                                             * `.dee' (e.g. "/opt/foo/somefile.txt"), then this string is
	                                             * set to that path as-is (with the extension still in-tact),
	                                             * but the module is also initialized to do no directory-scanning */
	struct Dee_module_treenode     mo_absnode;  /* [lock(INTERNAL(module_abstree_lock))][valid_if(mo_absname)]
	                                             * Node in tree of modules-by-mo_absname */
	struct Dee_module_libentry     mo_libname;  /* [valid_if(mo_absname != NULL)] Primary lib entry for this module (unused if "mle_name" is "NULL") */
	DREF struct Dee_tuple_object  *mo_dir;      /* [0..1][lock(WRITE_ONCE)] Tuple of strings (names of possible sub-modules of this module)
	                                             * Results of interpreting `mo_absname' as a directory and scanning
	                                             * that directory for "*.dee" files, and more directories. */
#define Dee_MODULE_INIT_UNINITIALIZED ((struct Dee_thread_object *)NULL)
#define Dee_MODULE_INIT_INITIALIZED   ((struct Dee_thread_object *)ITER_DONE)
	struct Dee_thread_object      *mo_init;     /* [0..1][lock(ATOMIC)] Module initialization state (one of `Dee_MODULE_INIT_*', or the thread doing the init)
	                                             * NOTE: For module types other than `DeeModuleDee_Type' and `DeeModuleDex_Type',
	                                             *       this must always be `Dee_MODULE_INIT_INITIALIZED' */
	union Dee_module_buildid       mo_buildid;  /* [valid_if(Dee_MODULE_FHASBUILDID)][lock(WRITE_ONCE)] Module build ID ({0,0} for DeeModuleDir_Type-modules) */
#define Dee_MODULE_FNORMAL         0x0000       /* Normal module flags. */
#define Dee_MODULE_FABSFILE        0x0001       /* [DeeModuleDee_Type][const] `mo_absname' is the actual, absolute filename of this module (which doesn't end with `.dee') */
#define Dee_MODULE_FHASBUILDID     0x0002       /* [lock(WRITE_ONCE)] Field `mo_buildid' has been initialized */
#define Dee_MODULE_FWAITINIT       0x0004       /* [lock(ATOMIC)] When `mo_init' is set to `Dee_MODULE_INIT_UNINITIALIZED' or `Dee_MODULE_INIT_INITIALIZED', must `DeeFutex_WakeAll(&mo_init)' */
#define Dee_MODULE_FNOSERIAL       0x0008       /* [const] Indicates that this module could not be serialized during compilation, meaning that anything that depends on it can't be serialized, either. */
#define Dee_MODULE_FABSRED         0x0100       /* [lock(ATOMIC)] is-red-bit for `mo_absnode' */
#define Dee_MODULE_FADRRED         0x0200       /* [lock(ATOMIC)] is-red-bit for `mo_adrnode' */
#define _Dee_MODULE_FNOADDR        0x2000       /* [const] Used for dex modules: "mo_minaddr" and "mo_maxaddr" may not be right. Internally, `dex_byaddr_tree' is used instead of `module_byaddr_tree' */
#define _Dee_MODULE_FLIBALL        0x4000       /* [lock(ATOMIC)] Used internally by `DeeModule_GetLibName()' */
#define _Dee_MODULE_FCLEARED       0x8000       /* [lock(ATOMIC)] Used internally by `DeeModule_ClearDexModuleCaches()' */
	uint16_t                       mo_flags;    /* Module flags (Set of `Dee_MODULE_F*') */
	uint16_t                       mo_importc;  /* [const] The total number of other modules imported by this one.
	                                             * (there may be more than these, but these are the ones accessible
	                                             * to deemon assembly within this module; s.a. `Dec_Dhdr') */
	uint16_t                       mo_globalc;  /* [const] The total number of globals allocated by this module. */
	/* Tables for symbols defined by this module. */
	uint16_t                       mo_bucketm;  /* [const] Mask that should be applied to hash values before indexing `mo_bucketv'. */
	struct Dee_module_symbol      *mo_bucketv;  /* [1..mo_bucketm+1][owned_if(!= empty_module_buckets)][const]
	                                             * Hash-vector for translating a string into a `uint16_t' index for `mo_globalv'.
	                                             * This is where module symbol names are stored and also used to
	                                             * implement symbol access by name at runtime. */
#define Dee_MODULE_HASHST(self, hash)  ((hash) & Dee_REQUIRES_OBJECT(DeeModuleObject, self)->mo_bucketm)
#define Dee_MODULE_HASHNX(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define Dee_MODULE_HASHIT(self, i)     (Dee_REQUIRES_OBJECT(DeeModuleObject, self)->mo_bucketv + ((i) & ((DeeModuleObject *)(self))->mo_bucketm))
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t            mo_lock;     /* Lock for accessing `mo_globalv'. */
#endif /* !CONFIG_NO_THREADS */
	Dee_WEAKREF_SUPPORT
	/* End of common data (the following fields don't exist for DeeModuleDir_Type-type modules) */
	union Dee_module_moddata                  mo_moddata;  /* Module-type-specific data */
	DREF struct Dee_module_object     *const *mo_importv;  /* [1..1][const][0..mo_importc][const][owned] Vector of other modules imported by this one. */
	__BYTE_TYPE__ const                      *mo_minaddr;  /* [const] Min address of memory mapped by this module */
	__BYTE_TYPE__ const                      *mo_maxaddr;  /* [const] Max address of memory mapped by this module */
	struct Dee_module_treenode                mo_adrnode;  /* [lock(INTERNAL(module_byaddr_lock))] Node in by-address tree */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, mo_globalv); /* [0..1][lock(mo_globalv)][mo_globalc] Globals of this module */
} DeeModuleObject;

#ifndef CONFIG_NO_THREADS
#define _Dee_MODULE_STRUCT_mo_lock Dee_atomic_rwlock_t mo_lock;
#define _Dee_MODULE_INIT_mo_lock   Dee_ATOMIC_RWLOCK_INIT,
#else /* !CONFIG_NO_THREADS */
#define _Dee_MODULE_STRUCT_mo_lock /* nothing */
#define _Dee_MODULE_INIT_mo_lock   /* nothing */
#endif /* CONFIG_NO_THREADS */

#define Dee_MODULE_STRUCT_EX(name, mo_globalv_def) \
	struct name {                                  \
		Dee_OBJECT_HEAD                            \
		/*utf-8*/ char                *mo_absname; \
		struct Dee_module_treenode     mo_absnode; \
		struct Dee_module_libentry     mo_libname; \
		struct Dee_tuple_object       *mo_dir;     \
		struct Dee_thread_object      *mo_init;    \
		union Dee_module_buildid       mo_buildid; \
		uint16_t                       mo_flags;   \
		uint16_t                       mo_importc; \
		uint16_t                       mo_globalc; \
		uint16_t                       mo_bucketm; \
		struct Dee_module_symbol      *mo_bucketv; \
		_Dee_MODULE_STRUCT_mo_lock                 \
		Dee_WEAKREF_SUPPORT                        \
		union Dee_module_moddata       mo_moddata; \
		DREF DeeModuleObject   *const *mo_importv; \
		__BYTE_TYPE__ const           *mo_minaddr; \
		__BYTE_TYPE__ const           *mo_maxaddr; \
		struct Dee_module_treenode     mo_adrnode; \
		mo_globalv_def                             \
	}
#define Dee_MODULE_STRUCT(name, mo_globalc_) \
	Dee_MODULE_STRUCT_EX(name, DREF DeeObject *mo_globalv[mo_globalc_];)

#define DeeModule_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->mo_lock)
#define DeeModule_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->mo_lock)
#define DeeModule_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->mo_lock)
#define DeeModule_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->mo_lock)
#define DeeModule_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->mo_lock)
#define DeeModule_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->mo_lock)
#define DeeModule_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->mo_lock)
#define DeeModule_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->mo_lock)
#define DeeModule_LockRead(self)       Dee_atomic_rwlock_read(&(self)->mo_lock)
#define DeeModule_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->mo_lock)
#define DeeModule_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->mo_lock)
#define DeeModule_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->mo_lock)
#define DeeModule_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->mo_lock)
#define DeeModule_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->mo_lock)
#define DeeModule_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->mo_lock)
#define DeeModule_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->mo_lock)

DDATDEF DeeTypeObject DeeModule_Type;
#define DeeModule_Check(ob)      DeeObject_InstanceOf(ob, &DeeModule_Type)
#if 0 /* Wouldn't make sense -- "DeeModule_Type" is more of an abstract base class for the types below. */
#define DeeModule_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeModule_Type)
#endif

DDATDEF DeeTypeObject DeeModuleDir_Type; /* ./folder   (directory-only module) */
DDATDEF DeeTypeObject DeeModuleDee_Type; /* ./foo.dee  (or ".foo.dec"; user-code module) */
#ifndef CONFIG_NO_DEX
DDATDEF DeeTypeObject DeeModuleDex_Type; /* ./net.so   (".so/.dll"; native module) */
#endif /* !CONFIG_NO_DEX */


struct Dee_static_module_struct {
	/* Even though never tracked, static modules still need the GC header for visiting. */
	struct Dee_gc_head       m_head;
	struct Dee_module_object m_module;
};

/* The built-in `deemon' module. */
#ifndef GUARD_DEEMON_RUNTIME_BUILTIN_C
#ifdef __INTELLISENSE__
DDATDEF DeeModuleObject DeeModule_Deemon;
#else /* __INTELLISENSE__ */
#undef DeeModule_Deemon
DDATDEF struct Dee_static_module_struct DeeModule_Deemon;
#define DeeModule_Deemon DeeModule_Deemon.m_module
#endif /* !__INTELLISENSE__ */
/* The module of builtin objects accessible by opening `deemon'. */
#define DeeModule_GetDeemon() (&DeeModule_Deemon)
#endif /* !GUARD_DEEMON_RUNTIME_BUILTIN_C */

#ifdef CONFIG_BUILDING_DEEMON
#ifndef GUARD_DEEMON_EXECUTE_MODULE_C
/* A stub module-object named `' (empty string), and pointing to `DeeCode_Empty'. */
#ifdef __INTELLISENSE__
INTDEF DeeModuleObject DeeModule_Empty;
#else /* __INTELLISENSE__ */
#undef DeeModule_Empty
INTDEF struct Dee_static_module_struct DeeModule_Empty;
#define DeeModule_Empty DeeModule_Empty.m_module
#endif /* !__INTELLISENSE__ */
#endif /* !GUARD_DEEMON_EXECUTE_MODULE_C */
#endif /* CONFIG_BUILDING_DEEMON */


/* Possible values for `DeeModule_ImportEx()' and `DeeModule_OpenEx()' */
#define DeeModule_IMPORT_F_NORMAL 0x0000 /* Normal import flags */
#define DeeModule_IMPORT_F_ENOENT 0x0001 /* Handle file-not-found errors by returning ITER_DONE instead of throwing an error */
#define DeeModule_IMPORT_F_FILNAM 0x0002 /* The given "import_str" is a system filename that is then loaded as a ".dee" file or directory */
#define DeeModule_IMPORT_F_CTXDIR 0x0004 /* `context_absname' is the path of the directory to use for relative imports, rather than a file within that directory. */
#define DeeModule_IMPORT_F_ANONYM 0x0008 /* Don't look at, or write into the global module tree -- always load anew as an anonymous module (unless it's a dex module). */
#define DeeModule_IMPORT_F_ERECUR 0x0010 /* Enable return of `DeeModule_IMPORT_ERECUR' when the module in question is currently being compiled by the calling thread */
#ifndef CONFIG_NO_DEX
#define DeeModule_IMPORT_F_NOLDEX 0x0020 /* Do not attempt to load DEX modules */
#endif /* !CONFIG_NO_DEX */
#ifndef CONFIG_NO_DEC
#define DeeModule_IMPORT_F_NOLDEC 0x0040 /* Do not attempt to load ".dec" files */
#define DeeModule_IMPORT_F_NOGDEC 0x0080 /* Do not attempt to generate ".dec" files */
#endif /* !CONFIG_NO_DEC */

/* Possible return values for `DeeModule_Open()' and `DeeModule_Import()' */
#define DeeModule_IMPORT_ERROR   ((DREF DeeModuleObject *)NULL)      /* An error was thrown */
#define DeeModule_IMPORT_ENOENT  ((DREF DeeModuleObject *)ITER_DONE) /* DeeModule_IMPORT_F_ENOENT: No such file, or file cannot be opened */
#define DeeModule_IMPORT_ERECUR  ((DREF DeeModuleObject *)-2L)       /* DeeModule_IMPORT_F_ERECUR: Module is already being compiled */
#define DeeModule_IMPORT_ISOK(x) (((uintptr_t)(x) - 2) < (uintptr_t)-3l)  /* `x != NULL && x != DeeModule_IMPORT_ENOENT && x != DeeModule_IMPORT_ERECUR' */

/* Import (DeeModule_Open() + DeeModule_Initialize()) a specific module */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_Import(/*String*/ DeeObject *__restrict import_str,
                 /*Module|String|Type|None*/ DeeObject *context_absname,
                 unsigned int flags);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_ImportString(/*utf-8*/ char const *__restrict import_str, size_t import_str_size,
                       /*Module|String|Type|None*/ DeeObject *context_absname, unsigned int flags);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_ImportEx(/*utf-8*/ char const *__restrict import_str, size_t import_str_size,
                   /*utf-8*/ char const *context_absname, size_t context_absname_size,
                   unsigned int flags, struct Dee_compiler_options *options);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
DeeModule_ImportChild(DeeModuleObject *self,
                      /*String*/ DeeObject *name,
                      unsigned int flags);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
DeeModule_ImportChildEx(DeeModuleObject *self,
                        /*utf-8*/ char const *__restrict name, size_t name_size,
                        unsigned int flags, struct Dee_compiler_options *options);


/* Open a module, given an import string, and another module/path used
 * to resolve relative paths. The given "import_str" can take any of the
 * following forms (assuming that `DeeSystem_SEP' is '/'):
 * - [m] "deemon"                    (DeeModule_GetDeemon())
 * - [m] "net.ftp"                   ("${LIBPATH}/net/ftp.dee")
 * - [m] "net"                       ("${LIBPATH}/net.so")
 * - [m] ".some_module"              (fs.headof(context_absname) + "/some_module.dee")
 * - [m] ".subdir.that_module"       (fs.headof(context_absname) + "/subdir/that_module.dee")
 * - [m] ".subdir"                   (fs.headof(context_absname) + "/subdir")                  DeeModuleDir_Type
 * - [m] "..pardir.subdir"           (fs.headof(context_absname) + "/../pardir/subdir")        DeeModuleDir_Type
 * - [m] "."                         (DeeModule_Open(fs.abspath(context_absname), NULL))       Only allowed if "context_absname" doesn't end with a trailing "/"
 * - [m] "./subdir/that_module"      (fs.headof(context_absname) + "/subdir/that_module.dee")
 * - [f] "./subdir/that_module.dee"  (fs.headof(context_absname) + "/subdir/that_module.dee")
 * - [f] "/opt/deemon/file.dee"      ("/opt/deemon/file.dee")
 * - [m] "/opt/deemon/file"          ("/opt/deemon/file.dee")
 * - [f] "/opt/deemon"               ("/opt/deemon")                                           DeeModuleDir_Type
 *
 * NOTE: When `DeeModule_IMPORT_F_FILNAM' is given, **ONLY** examples
 *       marked as [f] can be loaded (that is: "import_str" is treated
 *       as a native filename (**WITH** extension), rather than the
 *       usual combination of module-name/filename).
 *
 * The given "context_absname" should be the mo_absname-style name of
 * the calling file, or (at the very least) be a string ending with a
 * trailing `DeeSystem_SEP' (in this case, import_str="." will throw
 * an error). When this string isn't actually absolute, it will be
 * made absolute using `DeeSystem_MakeNormalAndAbsolute()'. When it is NULL or
 * an empty string, `DeeSystem_PrintPwd()' is used instead.
 *
 * @return: * :                      The newly opened module
 * @return: DeeModule_IMPORT_ERROR:  An error was thrown
 * @return: DeeModule_IMPORT_ENOENT: `DeeModule_IMPORT_F_ENOENT' was set, and no such file exists
 * @return: DeeModule_IMPORT_ERECUR: `DeeModule_IMPORT_F_ERECUR' was set, and module is already being imported */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_Open(/*String*/ DeeObject *__restrict import_str,
               /*Module|String|Type|None*/ DeeObject *context_absname,
               unsigned int flags);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenString(/*utf-8*/ char const *__restrict import_str, size_t import_str_size,
                     /*Module|String|Type|None*/ DeeObject *context_absname, unsigned int flags);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OpenEx(/*utf-8*/ char const *__restrict import_str, size_t import_str_size,
                 /*utf-8*/ char const *context_absname, size_t context_absname_size,
                 unsigned int flags, struct Dee_compiler_options *options);

/* Open a child of a specific module:
 * >> rt      = DeeModule_Open("rt", NULL, DeeModule_IMPORT_F_NORMAL);
 * >> rt_hash = DeeModule_OpenChild(rt, "hash", DeeModule_IMPORT_F_NORMAL);
 * Same as:
 * >> rt_hash = DeeModule_Open("rt.hash", NULL, DeeModule_IMPORT_F_NORMAL);
 *
 * NOTE: These functions ignore the `DeeModule_IMPORT_F_CTXDIR' flag! */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
DeeModule_OpenChild(DeeModuleObject *self,
                    /*String*/ DeeObject *name,
                    unsigned int flags);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
DeeModule_OpenChildEx(DeeModuleObject *self,
                      /*utf-8*/ char const *__restrict name, size_t name_size,
                      unsigned int flags, struct Dee_compiler_options *options);




/* Ensure that the initializer (aka. "mo_rootcode" code) of "self" has been run.
 * @return: 1 : The calling thread is already in the process of initializing "self".
 * @return: 0 : Success, or initializer was already executed.
 * @return: -1: An error was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeModule_Initialize(DeeModuleObject *__restrict self);

/* Ensure that imports of "self" have been initialized.
 * @return: 0 : Success.
 * @return: -1: An error was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeModule_InitializeImports(DeeModuleObject *__restrict self);

/* Check if the given module's current stat is `Dee_MODULE_INIT_UNINITIALIZED',
 * and if so: change it to `Dee_MODULE_INIT_INITIALIZED' (even if the module
 * may not have already been initialized)
 * @return: * : One of `DeeModule_SetInitialized_*' */
DFUNDEF NONNULL((1)) unsigned int DCALL
DeeModule_SetInitialized(DeeModuleObject *__restrict self);
#define DeeModule_SetInitialized_SUCCESS 0 /* Module was marked as `Dee_MODULE_INIT_INITIALIZED' */
#define DeeModule_SetInitialized_ALREADY 1 /* Module was already marked as `Dee_MODULE_INIT_INITIALIZED' */
#define DeeModule_SetInitialized_INPRGRS 2 /* Module is currently being initialized and can't have its status changed */

/* Return the root code object of a given module.
 * The caller must ensure that `self' is an instance of "DeeModuleDee_Type" */
DFUNDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF struct Dee_code_object *DCALL
DeeModule_GetRootCode(DeeModuleObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Function*/ DeeObject *DCALL
DeeModule_GetRootFunction(DeeModuleObject *__restrict self);

/* Return the "Build ID" of this module, which is an opaque
 * identifier that can be treated as a hash for the specific version that is
 * loaded into the module "self". When the module unloads and is then re-loaded,
 * this hash might change, in which case dependents of "self" should also be
 * re-build (potentially causing their build IDs to change also).
 *
 * @return: * :   The module's build ID
 * @return: NULL: An error was thrown */
DFUNDEF WUNUSED NONNULL((1)) union Dee_module_buildid const *DCALL
DeeModule_GetBuildId(DeeModuleObject *__restrict self);

/* Return the directory view of a given module, that is: the (lexicographically
 * sorted) list of ["*.dee", "*.so", "*.dll", DT_DIR]-files within a directory
 * (that don't contain any '.' characters) and matching the module's filename
 * with *its* trailing "*.dee"/"*.so"/"*.dll" removed (or just the directory
 * itself if that exists, but no associated source file / DEX module does).
 * If no such directory exists, "Dee_EmptyTuple" is returned.
 *
 * @return: * : The module's directory file list.
 * @return: NULL: An error was thrown */
DFUNDEF WUNUSED NONNULL((1)) /*Tuple*/ DeeObject *DCALL
DeeModule_GetDirectory(DeeModuleObject *__restrict self);

/* Return the unique, absolute name used to identify "self" within the filesystem.
 * - This string is "NULL" if the module was loaded with `DeeModule_IMPORT_F_ANONYM'
 *   or was returned by `DeeExec_CompileModuleStream()' (or a related function).
 * - This string (if non-NULL) can be used as-is with `DeeModule_Open' to open
 *   the module by-name.
 * - This is the absolute, normalized path to the module's directory, followed by
 *   the module's filename with a trailing `.dee' (or `.so' / `.dll') removed. As
 *   such, it is also **ALWAYS** the path that is searched for sub-modules for the
 *   directory returned by `DeeModule_GetDirectory()'
 * - When opening a file that does not end with '.dee', the string returned here
 *   will include that trailing extension.
 * Examples:
 * - E:\projects\deemon\lib\rt        (after opening 'E:\projects\deemon\lib\rt.dll')
 * - /opt/deemon/lib/net              (after opening '/opt/deemon/lib/net.so')
 * - /home/me/projects/deemon/script  (after opening '/home/me/projects/deemon/script.dee')
 * - /home/me/projects/readme.txt     (after opening '/home/me/projects/readme.txt' with `DeeModule_IMPORT_F_FILNAM')
 *
 * @return: * :   The module's absolute name
 * @return: NULL: Anonymous module */
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED NONNULL((1)) char const *DCALL
DeeModule_GetAbsName(DeeModuleObject *__restrict self);
#else /* __INTELLISENSE__ */
#define DeeModule_GetAbsName(self) \
	Dee_REQUIRES_TYPE(DeeModuleObject const *, self)->mo_absname
#endif /* !__INTELLISENSE__ */

/* Return the module's human-readable "short" name, that is everything after
 * the last '/' (or '\') within `DeeModule_GetAbsName()', or the string
 * "<anonymous module>" if `DeeModule_GetAbsName() == NULL' */
DFUNDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) char const *DCALL
DeeModule_GetShortName(DeeModuleObject *__restrict self);

/* Return the absolute, normalized filename that the module was loaded from,
 * `ITER_DONE' if `DeeModule_GetAbsName() == NULL'. This function combines
 * the module's 'Dee_MODULE_FABSFILE' flag together with its typing in order
 * to reconstruct the original filename that the module was loaded from.
 *
 * Examples:
 * - E:\projects\deemon\lib\rt.dll       (after opening 'E:\projects\deemon\lib\rt.dll')
 * - /opt/deemon/lib/net.so              (after opening '/opt/deemon/lib/net.so')
 * - /home/me/projects/deemon/script.dee (after opening '/home/me/projects/deemon/script.dee')
 * - /home/me/projects/readme.txt        (after opening '/home/me/projects/readme.txt' with `DeeModule_IMPORT_F_FILNAM')
 *
 * @return: * : The module's original, absolute, normalized source filename.
 * @return: ITER_DONE: Module is anonymous and doesn't have a source filename.
 * @return: NULL:      An error was thrown. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeModule_GetFileName(DeeModuleObject *__restrict self);

/* Return the relative import name of `self' when accessed from a file or module
 * `context_absname'. For more information, see `DeeModule_GetRelNameEx()'.
 *
 * @param: flags: Set of `DeeModule_RELNAME_F_*'
 * @return: * :        The module's name, written relative to `context_absname'
 * @return: ITER_DONE: The given module is anonymous or has its `Dee_MODULE_FABSFILE' flag set
 * @return: NULL:      An error was thrown. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeModule_GetRelName(DeeModuleObject *__restrict self,
                     /*Module|String|Type|None*/ DeeObject *context_absname,
                     unsigned int flags);

/* Same as `DeeModule_GetRelName()', but allows you to specify the context
 * path in the same manner as can be specified by `DeeModule_OpenEx()':
 * - When `DeeModule_RELNAME_F_CTXDIR' is not given, "context_absname"
 *   should be the mo_absname-style name of the calling file, or (at
 *   the very least) be a string ending with a trailing `DeeSystem_SEP'.
 * - When `DeeModule_RELNAME_F_CTXDIR' is given, "context_absname" is
 *   treated as the directory relative to which the returned path will
 *   be printed.
 * - When `self' is anonymous or cannot be opened without the use of
 *   the `DeeModule_IMPORT_F_FILNAM' flag, then `ITER_DONE' is returned.
 * - When the last part of the module name (after the last '/') contains
 *   a '.' (e.g. '/home/me/projects/foo/script.v1.dee'), then `ITER_DONE'
 *   is also returned, since no relative module name can be formed. The
 *   same also happens when any part of the path that would appear within
 *   the relative module path contains a '.'.
 * - When this string isn't actually absolute, it will be made absolute
 *   using `DeeSystem_MakeNormalAndAbsolute()'. When it is NULL or an
 *   empty string, `DeeSystem_PrintPwd()' is used instead.
 *
 * Examples:
 * - .rt           (self='E:\projects\deemon\lib\rt.dll', context_absname='E:\projects\deemon\lib\doc.dee' + DeeModule_RELNAME_F_NORMAL)
 * - .rt           (self='E:\projects\deemon\lib\rt.dll', context_absname='E:\projects\deemon\lib' + DeeModule_RELNAME_F_CTXDIR)
 * - rt            (self='E:\projects\deemon\lib\rt.dll', context_absname='<ignored>' + DeeModule_RELNAME_F_LIBNAM)
 * - ..foo.script  (self='/home/me/projects/foo/script.dee', context_absname='/home/me/projects/bar/script.dee' + DeeModule_RELNAME_F_NORMAL)
 * - ITER_DONE     (self='/home/me/projects/readme.txt', context_absname='<ignored>' + <ignored>))
 *
 * @param: flags: Set of `DeeModule_RELNAME_F_*'
 * @return: * :        The module's name, written relative to `context_absname'
 * @return: ITER_DONE: The given module is anonymous or has its `Dee_MODULE_FABSFILE' flag set
 * @return: NULL:      An error was thrown. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeModule_GetRelNameEx(DeeModuleObject *__restrict self,
                       /*utf-8*/ char const *context_absname,
                       size_t context_absname_size, unsigned int flags);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeModule_PrintRelNameEx(DeeModuleObject *__restrict self,
                         Dee_formatprinter_t printer, void *arg,
                         /*utf-8*/ char const *context_absname,
                         size_t context_absname_size, unsigned int flags);
#define DeeModule_RELNAME_F_NORMAL DeeModule_IMPORT_F_NORMAL /* Normal name flags */
#define DeeModule_RELNAME_F_CTXDIR DeeModule_IMPORT_F_CTXDIR /* `context_absname' is the path of the directory to use for relative imports, rather than a file within that directory. */
#define DeeModule_RELNAME_F_LIBNAM 0x8000 /* If the module has at least 1 LibName (s.a. `DeeModule_GetLibName()'), return the first of those instead. */


/* Ensure that all possible lib (global) names for `self' have been
 * determined (using paths from `DeeModule_SetLibPath()'), then return
 * the `index'th (0-based) one of them.
 * - A special case is made for the builtin `DeeModule_Deemon',
 *   which always has exactly `1' lib name "deemon".
 * - When the same module may be accessible from multiple lib paths,
 *   then the order in which its possible absolute names are listed
 *   is undefined.
 * - When `DEEMON_PATH' is set-up such that multiple modules might
 *   hold the same lib name, only one of them will (and this function
 *   will also list them for only that one module), though it is
 *   undefined which of those modules that will be.
 *
 * Examples:
 * - rt.gen.unpack        (self='/opt/deemon/lib/rt/gen/unpack.dee', DEEMON_PATH="/opt/deemon/lib", index=0)
 * - ITER_DONE            (self='/home/me/projects/foo/script.dee', DEEMON_PATH="/opt/deemon/lib", index=<ignored>)
 * - lib.rt.gen.unpack    (self='/opt/deemon/lib/rt/gen/unpack.dee', DEEMON_PATH="/opt/deemon:/opt/deemon/lib", index=0)
 * - rt.gen.unpack        (self='/opt/deemon/lib/rt/gen/unpack.dee', DEEMON_PATH="/opt/deemon:/opt/deemon/lib", index=1)
 *
 * @return: * :        The module's index'th lib name, written relative to `context_absname'
 * @return: ITER_DONE: The given module is anonymous or has its `Dee_MODULE_FABSFILE'
 *                     flag set, or isn't located in a sub-directory of `DEEMON_PATH',
 *                     or `index' is greater than the module's # of lib names.
 * @return: NULL:      An error was thrown. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeModule_GetLibName(DeeModuleObject *__restrict self, size_t index);

/* Return 1+ the greatest index that may be passed to `DeeModule_GetLibName()' for the
 * purpose of querying module lib names. Note that calls to `DeeModule_SetLibPath()'
 * (even those made from different threads) may cause the return value of this function
 * to fall out-of-date the second this function does return, so be always be prepared
 * for `DeeModule_GetLibName()' to return `ITER_DONE' even before this limit is reached.
 *
 * @return: 0 : The given module is anonymous or has its `Dee_MODULE_FABSFILE'
 *              flag set, or isn't located in a sub-directory of `DEEMON_PATH'.
 * @return: * : The # of lib names that `self' had at the time of this call.
 * @return: (size_t)-1: An error was thrown. */
DFUNDEF WUNUSED NONNULL((1)) size_t DCALL
DeeModule_GetLibNameCount(DeeModuleObject *__restrict self);


/* Special functions exported by `DeeModule_Deemon'.
 * These are here so that dex modules (like _hostasm) can detect calls to these functions. */
DDATDEF struct Dee_cmethod_object DeeBuiltin_HasAttr;
DDATDEF struct Dee_cmethod_object DeeBuiltin_HasItem;
DDATDEF struct Dee_cmethod_object DeeBuiltin_BoundAttr;
DDATDEF struct Dee_cmethod_object DeeBuiltin_BoundItem;
DDATDEF struct Dee_cmethod_object DeeBuiltin_Compare;
DDATDEF struct Dee_cmethod_object DeeBuiltin_Equals;
DDATDEF struct Dee_cmethod_object DeeBuiltin_Hash;
DDATDEF struct Dee_cmethod_object DeeBuiltin_Exec;

/* The object used to implement the magic compiler "import" builtin.
 *
 * This object behaves in 1 of 2 ways:
 *
 * - It provides an `operator ()' that can be invoked by passing a string
 *   argument that is then interpreted as either an absolute (LIBPATH),
 *   or relative (to the calling module) import string:
 *   >> local relativeModule = import(".sibling"); // Loads "./sibling.dee"
 *   >> local libModule      = import("deemon");   // Returns "DeeModule_Deemon"
 * - Note that for relative imports to correctly work, when the compiler
 *   identifies a call to "import", it will inject an additional, leading
 *   argument that is equal to the caller's own module, and resolve the
 *   call as a whole against "deemon.__import__":
 *   >> local libModule = import(".sibling");
 *   Same as:
 *   >> import . as me, deemon;
 *   >> local libModule = deemon.__import__(me, ".sibling");
 * - Note that "deemon.__import__" simply resolves to `DeeBuiltin_Import',
 *   meaning that the built-in `import' keyword is pretty much just there
 *   as (admittedly very necessary, since `deemon.__import__' is actually
 *   an implementation-specific symbol) syntax sugar.
 *
 * - It provides an `operator .' and `operator enumattr' that can be used
 *   to enumerate modules of- and import modules from the LIBPATH (as set
 *   by `DeeModule_SetLibPath()'):
 *   >> local libModule1 = import.deemon;   // import("deemon")
 *   >> local libModule2 = import.rt;       // import("rt")
 *   >> local libModule3 = import.rt.hash;  // import("rt.hash")
 *
 * - In order to enumerate all modules found on the LIBPATH, you can simply
 *   treat "import" as an object whose attributes can be enumerated to get
 *   the names and mappings of all top-level LIBPATH modules:
 *   >> local allLibModules = Mapping.fromattr(import);
 *   >> for (local k, v: allLibModules)
 *   >>     print repr k, repr v;
 *   Output will probably look like this:
 *   >> [...]
 *   >> "deemon" import.deemon
 *   >> [...]
 *   >> "annotations" import.annotations
 *   >> "collections" import.collections
 *   >> "ctypes" import.ctypes
 *   >> "disassembler" import.disassembler
 *   >> "doc" import.doc
 *   >> "doctext" import.doctext
 *   >> "errors" import.errors
 *   >> "files" import.files
 *   >> "fs" import.fs
 *   >> [...]
 *
 * - Thanks to this new attribute-based syntax, as well as the fact that
 *   regular directories can be treated as modules (even when a properly
 *   named ".dee" file also exists), this means that anything defined by
 *   LIBPATH modules can be named without the need of "(symbol from module)":
 *   >> import.deemon.string.find; // this...
 *   >> (string from deemon).find; // ... is the same as this, but easier to read
 */
DDATDEF DeeObject DeeBuiltin_Import;
DDATDEF DeeTypeObject DeeBuiltin_ImportType; /* Dee_TYPE(&DeeBuiltin_Import) */


/* Return the export address of a native symbol exported from a dex `self'.
 * When `self' isn't a dex, but a regular module, or if the symbol wasn't found, return `NULL'.
 * NOTE: Because native symbols cannot appear in user-defined modules,
 *       in the interest of keeping native functionality to its bare
 *       minimum, any code making using of this function should contain
 *       a fallback that calls a global symbol of the module, rather
 *       than a native symbol:
 * >> static int (*p_add)(int x, int y) = NULL;
 * >> if (!p_add)
 * >>     *(void **)&p_add = DeeModule_GetNativeSymbol(IMPORTED_MODULE, "add");
 * >> // Fallback: Invoke a member attribute `add' if the native symbol doesn't exist.
 * >> if (!p_add)
 * >>     return DeeObject_CallAttrStringf(IMPORTED_MODULE, "add", "dd", x, y);
 * >> // Invoke the native symbol.
 * >> return DeeInt_NewInt((*p_add)(x, y)); */
DFUNDEF WUNUSED NONNULL((1, 2)) void *DCALL
DeeModule_GetNativeSymbol(DeeModuleObject *__restrict self,
                          char const *__restrict name);

/* Given a pointer `ptr' that is either for some statically allocated variable/symbol
 * (as in: a pointer to some statically allocated structure), or is part of some user
 * module's statically allocated memory blob (e.g. the address of a 'DeeStringObject'
 * that is a constant in user-code), try to return a reference for the module that
 * contains this pointer (only when CONFIG_EXPERIMENTAL_MMAP_DEC).
 *
 * @return: * :   A pointer to the module that 'ptr' belongs to.
 * @return: NULL: Given `ptr' is either invalid, heap-allocated, or simply not part
 *                of the deemon core, some dex module, or a some user-code module. */
DFUNDEF WUNUSED DREF DeeModuleObject *DCALL
DeeModule_OfPointer(void const *ptr);

/* Extension to `DeeModule_OfPointer()' that checks if `ob' is statically allocated
 * within some specific module. But if it isn't, then it looks at the type of `ob'
 * and tries to return the associated module via type-specific means:
 * - DeeType_Type: DeeTypeObject::tp_module
 * - DeeCode_Type: DeeCodeObject::co_module */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
DeeModule_OfObject(DeeObject *__restrict ob);

/* Check if `DeeModule_OfPointer(ptr) == self' (but is a bit faster than that).
 * Use this function instead of looking at `mo_minaddr' / `mo_maxaddr', because
 * this function does some necessarily extra handling for certain types of DEX
 * modules that are loaded in multiple segments (in which case it would not be
 * defined if `mo_minaddr' / `mo_maxaddr' is union of all segments, or only some
 * (sub-)set of segments)
 *
 * NOTE: Unlike many other functions, this one can actually still be used while `self'
 *       is being finalized (e.g. while inside of `Dee_module_dexdata::mdx_fini'). It
 *       also guaranties that no user-code will ever be executed (hence the "PURE")
 *
 * @return: true:  Yes, "ptr" is part of "self"
 * @return: false: No, "ptr" is not part of "self" */
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
DeeModule_ContainsPointer(DeeModuleObject *__restrict self, void const *ptr);


/* Callback prototypes for `DeeModule_Enumerate*' functions below.
 * All of these behave in a Dee_formatprinter_t-compatible manner:
 * @return: >= 0: Success (sum of return values is accumulated and returned by caller)
 * @return: < 0:  Error (abort enumeration immediately and propagate return value) */
typedef NONNULL_T((2)) Dee_ssize_t
(DCALL *Dee_module_enumerate_cb_t)(void *arg, DeeModuleObject *__restrict mod);
typedef NONNULL_T((2, 3)) Dee_ssize_t
(DCALL *Dee_module_enumerate_lib_cb_t)(void *arg, DeeModuleObject *__restrict mod,
                                       /*String*/ DeeObject *libname);

/* Enumerate loaded modules using various different means.
 *
 * DeeModule_EnumerateAbsTree:
 *     Enumerate all non-anonymous modules (i.e. ones with `mo_absname != NULL').
 *
 * DeeModule_EnumerateLibTree:
 *     Enumerate modules via their "lib" names (e.g. "deemon", "rt", etc.)
 *     Note that this only includes modules whose lib-names are loaded **right now**.
 *     If any changes are mading to the module LIBPATH (e.g. `DeeModule_AddLibPath()'
 *     or `DeeModule_RemoveLibPath()' is called), the lib-names of already-loaded
 *     modules will **NOT** be calculated immediatly, but lazily. And a call to
 *     `DeeModule_EnumerateLibTree()' will **NOT** do this lazy calculation.
 *
 * DeeModule_EnumerateAdrTree:
 *     Enumerate modules that reside within the address space (i.e.: have an
 *     address range as per `mo_minaddr' / `mo_maxaddr'). This essentially means
 *     that all `DeeModuleDee_Type' and `DeeModuleDex_Type' modules (including
 *     the core `DeeModule_Deemon' module) will be enumerated.
 *
 * NOTES:
 * - The order in which modules are enumerated is undefined but will not change
 *   for already-enumerated modules (including modules enumerated during a prior
 *   call to these functions).
 * - Every qualifying module loaded at the time the `DeeModule_Enumerate*' call
 *   started, and still-loaded when this call returns has been passed to `*cb'
 *   exactly once. (Modules that are unloaded and then quickly re-loaded may be
 *   enumerated multiple times however)
 * - None of the `DeeModule_Enumerate*' functions can throw errors on their own.
 *   The only way that some negative value can be returned, is from `cb' returning
 *   that same negative value.
 * - The "opt_type_filter" argument can either be "NULL", or one of:
 *   - DeeModuleDee_Type
 *   - DeeModuleDir_Type
 *   - DeeModuleDex_Type
 *   ... to only enumerate modules with that specific typing.
 * - The `*cb' callback is allowed to do anything it wants, including invoking any
 *   user-code, as well as load additional modules. It is however undefined if modules
 *   that were loaded after the `DeeModule_Enumerate*' call started will also be
 *   enumerated.
 *
 * @param: cb:              The callback that should be invoked
 * @param: arg:             Cookie argument that should be passed to
 * @param: start_after:     Start enumeration with whatever module comes after `start_after'.
 *                          When `NULL', start enumeration at the very beginning.
 * @param: opt_type_filter: Only enumerate modules of this type (set to "NULL" to not filter).
 *
 * @return: * : Sum of return values of `*cb'
 * @return: 0 : Either `*cb' always returned `0', or it was never invoked
 * @return: <0: A call to `*cb' returned this same negative value. */
DFUNDEF NONNULL((1)) Dee_ssize_t DCALL
DeeModule_EnumerateAbsTree(Dee_module_enumerate_cb_t cb, void *arg,
                           DeeModuleObject *start_after,
                           DeeTypeObject *opt_type_filter);
DFUNDEF NONNULL((1)) Dee_ssize_t DCALL
DeeModule_EnumerateAdrTree(Dee_module_enumerate_cb_t cb, void *arg,
                           DeeModuleObject *start_after,
                           DeeTypeObject *opt_type_filter);
DFUNDEF NONNULL((1)) Dee_ssize_t DCALL
DeeModule_EnumerateLibTree(Dee_module_enumerate_lib_cb_t cb, void *arg,
                           DeeModuleObject *start_after_mod,
                           /*String*/ DeeObject *start_after_name,
                           DeeTypeObject *opt_type_filter);
#ifdef CONFIG_NO_DEX
#define DeeModule_EnumerateDexModules(cb, arg, start_after) \
	((start_after) ? 0 : (*(cb))(arg, &DeeModule_Deemon))
#else /* CONFIG_NO_DEX */
#define DeeModule_EnumerateDexModules(cb, arg, start_after) \
	DeeModule_EnumerateAdrTree(cb, arg, start_after, &DeeModuleDex_Type)
#endif /* !CONFIG_NO_DEX */

/* Convenience wrappers around `DeeModule_Enumerate*' that return whatever
 * module comes after "prev" (if such a module exists), or "NULL" if no such
 * module exists. When "prev" is "NULL", return the first module of that tree. */
DFUNDEF WUNUSED DREF DeeModuleObject *DCALL
DeeModule_NextAbsTree(DeeModuleObject *prev, DeeTypeObject *opt_type_filter);
DFUNDEF WUNUSED DREF DeeModuleObject *DCALL
DeeModule_NextAdrTree(DeeModuleObject *prev, DeeTypeObject *opt_type_filter);
DFUNDEF WUNUSED NONNULL((4)) DREF DeeModuleObject *DCALL
DeeModule_NextLibTree(DeeModuleObject *prev, /*String*/ DeeObject *prev_libname,
                      DeeTypeObject *opt_type_filter,
                      DREF /*String*/ DeeObject **__restrict p_libname);


/* Lookup an external symbol.
 * Convenience function (same as `DeeObject_GetAttr(DeeModule_Import(...), ...)') */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetExtern(/*String*/ DeeObject *module_name,
                    /*String*/ DeeObject *global_name);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_GetExternString(/*utf-8*/ char const *module_name,
                          /*utf-8*/ char const *global_name);

/* Helper wrapper for `DeeObject_Call(DeeModule_GetExternString(...), ...)',
 * that returns the return value of the call operation. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_CallExtern(/*String*/ DeeObject *module_name,
                     /*String*/ DeeObject *global_name,
                     size_t argc, DeeObject *const *argv);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_CallExternString(/*utf-8*/ char const *module_name,
                           /*utf-8*/ char const *global_name,
                           size_t argc, DeeObject *const *argv);

/* Helper wrapper for `DeeObject_Callf(DeeModule_GetExternString(...), ...)',
 * that returns the return value of the call operation. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeModule_CallExternf(/*String*/ DeeObject *module_name,
                      /*String*/ DeeObject *global_name,
                      char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_VCallExternf(/*String*/ DeeObject *module_name,
                       /*String*/ DeeObject *global_name,
                       char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeModule_CallExternStringf(/*utf-8*/ char const *module_name,
                            /*utf-8*/ char const *global_name,
                            char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeModule_VCallExternStringf(/*utf-8*/ char const *module_name,
                             /*utf-8*/ char const *global_name,
                             char const *__restrict format, va_list args);


#ifdef CONFIG_BUILDING_DEEMON
struct Dee_attribute_info;
struct Dee_attribute_lookup_rules;
struct Dee_attrinfo;

/* Access global variables of a given module by their name described by a C-string.
 * These functions act and behave just as once would expect, raising errors when
 * appropriate and returning NULL/false/-1 upon error or not knowing the given name. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeModule_GetAttr(DeeModuleObject *self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_HasAttr(DeeModuleObject *self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_BoundAttr(DeeModuleObject *self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_DelAttr(DeeModuleObject *self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeModule_SetAttr(DeeModuleObject *self, /*String*/ DeeObject *attr, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeModule_GetAttrStringHash(DeeModuleObject *__restrict self, char const *__restrict attr, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeModule_GetAttrStringLenHash(DeeModuleObject *__restrict self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_HasAttrStringHash(DeeModuleObject *__restrict self, char const *__restrict attr, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_HasAttrStringLenHash(DeeModuleObject *__restrict self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_BoundAttrStringHash(DeeModuleObject *__restrict self, char const *__restrict attr, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_BoundAttrStringLenHash(DeeModuleObject *__restrict self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_DelAttrStringHash(DeeModuleObject *__restrict self, char const *__restrict attr, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_DelAttrStringLenHash(DeeModuleObject *__restrict self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeModule_SetAttrStringHash(DeeModuleObject *self, char const *__restrict attr, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeModule_SetAttrStringLenHash(DeeModuleObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) bool DCALL DeeModule_FindAttrInfoStringLenHash(DeeModuleObject *self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash, struct Dee_attrinfo *__restrict retinfo);

#define DeeModule_GetAttrHash(self, attr, hash)                DeeModule_GetAttrStringHash(self, DeeString_STR(attr), hash)
#define DeeModule_GetAttrString(self, attr)                    DeeModule_GetAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeModule_GetAttrStringLen(self, attr, attrlen)        DeeModule_GetAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeModule_HasAttrHash(self, attr, hash)                DeeModule_HasAttrStringHash(self, DeeString_STR(attr), hash)
#define DeeModule_HasAttrString(self, attr)                    DeeModule_HasAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeModule_HasAttrStringLen(self, attr, attrlen)        DeeModule_HasAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeModule_BoundAttrHash(self, attr, hash)              DeeModule_BoundAttrStringHash(self, DeeString_STR(attr), hash)
#define DeeModule_BoundAttrString(self, attr)                  DeeModule_BoundAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeModule_BoundAttrStringLen(self, attr, attrlen)      DeeModule_BoundAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeModule_DelAttrHash(self, attr, hash)                DeeModule_DelAttrStringHash(self, DeeString_STR(attr), hash)
#define DeeModule_DelAttrString(self, attr)                    DeeModule_DelAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeModule_DelAttrStringLen(self, attr, attrlen)        DeeModule_DelAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeModule_SetAttrHash(self, attr, hash, value)         DeeModule_SetAttrStringHash(self, DeeString_STR(attr), hash, value)
#define DeeModule_SetAttrString(self, attr, value)             DeeModule_SetAttrStringHash(self, attr, Dee_HashStr(attr), value)
#define DeeModule_SetAttrStringLen(self, attr, attrlen, value) DeeModule_SetAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen), value)
#define DeeModule_FindAttrInfoStringHash(self, attr, hash, retinfo) DeeModule_FindAttrInfoStringLenHash(self, attr, strlen(attr), hash, retinfo)
#endif /* CONFIG_BUILDING_DEEMON */

/* Lookup the module symbol associated with a given its name or GID.
 * If the symbol could not be found, return `NULL', but _DONT_ throw an error.
 * WARNING: When `self' could potentially be an interactive module, you
 *          must surround a call to any of these functions with a lock that
 *          can be acquired / released using `DeeModule_LockSymbols()' /
 *         `DeeModule_UnlockSymbols()'
 *          Additionally, you must be extremely careful, as an interactive
 *          module may arbitrarily modify its global object table! */
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_module_symbol *DCALL DeeModule_GetSymbol(DeeModuleObject const *__restrict self, /*String*/ DeeObject *__restrict name);
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_module_symbol *DCALL DeeModule_GetSymbolStringHash(DeeModuleObject const *__restrict self, char const *__restrict attr, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1, 2)) struct Dee_module_symbol *DCALL DeeModule_GetSymbolStringLenHash(DeeModuleObject const *__restrict self, char const *__restrict attr, size_t attrlen, Dee_hash_t hash);
DFUNDEF WUNUSED NONNULL((1)) struct Dee_module_symbol *DCALL DeeModule_GetSymbolID(DeeModuleObject const *__restrict self, uint16_t gid);
#define DeeModule_GetSymbolHash(self, attr, hash)         DeeModule_GetSymbolStringHash(self, DeeString_STR(attr), hash)
#define DeeModule_GetSymbolString(self, attr)             DeeModule_GetSymbolStringHash(self, attr, Dee_HashStr(attr))
#define DeeModule_GetSymbolStringLen(self, attr, attrlen) DeeModule_GetSymbolStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeModule_GetAttrSymbol(DeeModuleObject *__restrict self, struct Dee_module_symbol const *__restrict sym);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_BoundAttrSymbol(DeeModuleObject *__restrict self, struct Dee_module_symbol const *__restrict sym);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeModule_DelAttrSymbol(DeeModuleObject *__restrict self, struct Dee_module_symbol const *__restrict sym);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeModule_SetAttrSymbol(DeeModuleObject *__restrict self, struct Dee_module_symbol const *__restrict sym, DeeObject *__restrict value);

/* Return the name of a global variable in the given module.
 * @return: NULL: The given `gid' is not recognized, or the module hasn't finished/started loading yet.
 * @return: * :   The name of the global associated with `gid'.
 *                Note that in the case of aliases existing for `gid', this function prefers not to
 *                return the name of an alias, but that of the original symbol itself, so long as that
 *                symbol actually exist, which if it doesn't, it will return the name of a random alias. */
DFUNDEF WUNUSED NONNULL((1)) char const *DCALL
DeeModule_GlobalName(DeeModuleObject *__restrict self, uint16_t gid);

DECL_END

#endif /* !GUARD_DEEMON_MODULE_H */
