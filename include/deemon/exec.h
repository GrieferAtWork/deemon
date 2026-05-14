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
/*!export Dee_RUNATEXIT_**/
/*!export DeeExec_**/
/*!export DeeExec_RUNMODE_**/
/*!export DeeModule_**/
/*!export DeeModule_*LibPath**/
/*!export Dee_SHUTDOWN_F_**/
#ifndef GUARD_DEEMON_EXEC_H
#define GUARD_DEEMON_EXEC_H 1 /**/

#include "api.h"

#include "types.h" /* DREF, DeeObject */

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uint16_t */

DECL_BEGIN

/* Get/Set deemon's home path.
 * The home path is used to locate builtin libraries, as well as extensions.
 * Upon first access, `DeeExec_GetHome()' will pre-initialize the home-path as follows:
 * >> deemon_home = fs.environ.get("DEEMON_HOME");
 * >> if (deemon_home !is none) {
 * >>     deemon_home = fs.abspath(deemon_home);
 * >> } else {
 * >>#ifdef CONFIG_DEEMON_HOME
 * >>     deemon_home = CONFIG_DEEMON_HOME;
 * >>#else
 * >>     // Some other os-specific shenanigans here...
 * >>     deemon_home = fs.headof(fs.readlink("/proc/self/exe"));
 * >>#endif
 * >> }
 * >> deemon_home = fs.inctrail(deemon_home);
 * That is: Try to lookup an environment variable `DEEMON_HOME', which
 *          if found is then converted into an absolute filename.
 *          When this variable doesn't exist, behavior depends on how deemon was built.
 *          If it was built with the `CONFIG_DEEMON_HOME' option enabled, that
 *          option is interpreted as a string which is then used as the effective
 *          home path, but if that option was disabled, the folder of deemon's
 *          executable is used as home folder instead.
 * @return: NULL: Failed to determine the home folder (an error was set).
 * NOTE: The home path _MUST_ include a trailing slash! */
DFUNDEF WUNUSED DREF /*String*/ DeeObject *DCALL DeeExec_GetHome(void);

/* Set the new home folder, overwriting whatever was set before.
 * HINT: You may pass `NULL' to cause the default home path to be re-created. */
DFUNDEF void DCALL DeeExec_SetHome(/*String*/ DeeObject *new_home);




/* List of strings that should be used as base paths when searching for global modules.
 * Access to this list should go through `DeeModule_InitPath()', which will
 * automatically initialize the list to the following default contents upon access:
 *
 * >> Commandline: `-L...' where every occurrance is pre-pended before the home-path.
 *    NOTE: These paths are not added by `DeeModule_InitPath()', but instead
 *          the first encouter of a -L option will call `DeeModule_GetPath()'
 *          before pre-pending the following string at the front of the list,
 *          following other -L paths prepended before then.
 * >> posix.environ.get("DEEMON_PATH", "").split(posix.FS_DELIM)...;
 * >> posix.joinpath(DeeExec_GetHome(), "lib")
 *
 * This list is also used to locate system-include paths for the preprocessor,
 * in that every entry that is a string is an include-path after appending "/include":
 * >> function get_include_paths(): {string...} {
 * >>     for (local x: DeeModule_Path)
 * >>         if (x is string)
 * >>             yield f"{x.rstrip("/")}/include";
 * >> } */

/* Return the current module path, which is a tuple of absolute,
 * normalized directory names describing where deemon system
 * modules can be found. (Paths have **NO** trailing '/'!)
 * @return: * :   The module path
 * @return: NULL: Error */
DFUNDEF WUNUSED DREF /*Tuple*/ DeeObject *DCALL DeeModule_GetLibPath(void);

/* Set (or reset when "new_libpath == NULL") the module path.
 * - Assumes that "new_libpath" is a tuple
 * - Throws an error if any element of "new_libpath" isn't a string
 * - Normalizes given paths using `DeeSystem_MakeNormalAndAbsolute()'
 * - Removes duplicate paths (but retains order of distinct paths)
 * @return: 0 : Success (always returned when "new_libpath == NULL")
 * @return: -1: Error */
DFUNDEF WUNUSED int DCALL DeeModule_SetLibPath(/*Tuple*/ DeeObject *new_libpath);

/* Add or remove a new module path
 * @return: 1 : Given path was added (or removed)
 * @return: 0 : Nothing happened (path was already added, or removed)
 * @return: -1: Error */
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeModule_AddLibPath(/*String*/ DeeObject *__restrict path);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeModule_AddLibPathString(/*utf-8*/ char const *__restrict path);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeModule_AddLibPathStringLen(/*utf-8*/ char const *__restrict path, size_t path_len);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeModule_RemoveLibPath(/*String*/ DeeObject *__restrict path);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeModule_RemoveLibPathString(/*utf-8*/ char const *__restrict path);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeModule_RemoveLibPathStringLen(/*utf-8*/ char const *__restrict path, size_t path_len);
#ifdef CONFIG_BUILDING_DEEMON
INTDEF bool DCALL DeeModule_ClearLibPath(void);
#endif /* CONFIG_BUILDING_DEEMON */


/* High-level functionality for registering at-exit hooks.
 * When executed, at-exit callbacks are run in order of being registered.
 * NOTE: This function makes use of libc's `atexit()' function (if available).
 * @param args: A tuple object the is used to invoke `callback'
 * @return:  0: Successfully registered the given callback.
 * @return: -1: An error occurred or atexit() can no longer be used
 *              because `Dee_RunAtExit()' is being, or had been called. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_AtExit(DeeObject *callback, DeeObject *args);

/* Run callbacks that have been registered using `Dee_AtExit()'
 * @return:  0: Successfully executed all callbacks.
 * @return: -1: An error occurred (never returned when `Dee_RUNATEXIT_FRUNALL' is passed)
 * NOTE: This function is automatically called when `exit()'
 *       from stdlib is used to stop execution of the program. */
DFUNDEF int DCALL Dee_RunAtExit(uint16_t flags);
#define Dee_RUNATEXIT_FNORMAL  0x0000 /* Normal flags. */
#define Dee_RUNATEXIT_FRUNALL  0x0001 /* Always run all callbacks and display all errors
                                       * that may occur during their execution. */
#define Dee_RUNATEXIT_FDONTRUN 0x0002 /* Do not execute callbacks, but discard all of them.
                                       * However, still disallow further calls to `Dee_AtExit()' */

/* Terminate the application the same way `deemon.Error.AppExit.exit()' would,
 * either through use of `exit()' from <stdlib.h>, or by throwing an exception.
 * NOTE: When available, calling stdlib's `exit()' is identical to this.
 * @return: -1: If this function returns at all, it always returns `-1' */
DFUNDEF int DCALL Dee_Exit(int exitcode, bool run_atexit);


/* Get/Set the user-code argument vector that is
 * accessible from module-scope code using `...':
 * >> local argv = [...];
 * >> print repr argv; // ["my_script.dee", "these", "are", "from", "the", "commandline"]
 * With that in mind, module root-code objects are varargs functions that are
 * invoked using the Argv tuple modifiable using this pair of functions.
 * The deemon launcher should call `Dee_SetArgv()' to set the original argument tuple.
 * NOTE: By default, an empty tuple is set for argv. */
DFUNDEF WUNUSED ATTR_RETNONNULL DREF /*Tuple*/ DeeObject *DCALL Dee_GetArgv(void);
DFUNDEF NONNULL((1)) void DCALL Dee_SetArgv(/*Tuple*/ DeeObject *__restrict argv);




#define DeeExec_RUNMODE_DEFAULT               0x0000 /* Default run mode: just run as-is */
#define DeeExec_RUNMODE_STMTS                 0x0000 /* Parse a whole module source as a sequence of statements. */
#define DeeExec_RUNMODE_STMT                  0x0001 /* Parse, compile and execute a single statement (e.g. `if (x) y; else z;') and return the result like `return ({ ... });' */
#define DeeExec_RUNMODE_FULLEXPR              0x0002 /* Parse, compile and execute a single expression (e.g. `10 + 20' or `[]{ print 42; return "foo"; }') and return the result
                                                      * NOTE: As far as comma-expressions go, simply return the last component, but do allow assignments. */
#define DeeExec_RUNMODE_EXPR                  0x0003 /* Parse, compile and execute a basic expression
                                                      * NOTE: As far as comma-expressions go, simply return the last component. */
#define DeeExec_RUNMODE_MASK                  0x000f /* Mode mask */
#define DeeExec_RUNMODE_FDEFAULTS_ARE_GLOBALS 0x4000 /* FLAG: Default symbols are declared as read/write globals (otherwise, they are read-only constants) */
struct Dee_compiler_options;

/* Execute source code from `source_stream' and return the result of invoking it.
 * @param: source_stream:   The input stream from which to take input arguments.
 * @param: mode:            One of `DeeExec_RUNMODE_*', optionally or'd with a set of `DeeExec_RUNMODE_F*'
 * @param: argv:            Variable arguments passed to user-code
 * @param: start_line:      The starting line number when compiling code. (zero-based)
 * @param: start_col:       The starting column number when compiling code. (zero-based)
 * @param: options:         A set of compiler options applicable for compiled code.
 *                          Note however that certain options have no effect, such
 *                          as the fact that peephole and other optimizations are
 *                          forced to be disabled, or DEC files are never generated,
 *                          all for reasons that should be quite obvious.
 * @param: default_symbols: A mapping-like object of type `{string: Object}', that
 *                          contains a set of pre-defined variables that should be made
 *                          available to the interactive source code by use of global
 *                          variables.
 *                          These are either provided as constants, or as globals,
 *                          depending on `DeeExec_RUNMODE_FDEFAULTS_ARE_GLOBALS' */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeExec_RunStream(DeeObject *source_stream, size_t argc, DeeObject *const *argv,
                  int start_line, int start_col, unsigned int mode,
                  struct Dee_compiler_options *options, DeeObject *default_symbols);


/* Similar to `DeeExec_RunStream()', but rather than directly executing it,
 * return the module used to describe the code that is being executed, or
 * some unspecified, callable object which (when invoked) executes the given
 * input code in one way or another.
 * It is up to the implementation if an associated module should simply be
 * generated, before that module's root is returned, or if the given user-code
 * is only executed when the function is called, potentially allowing for
 * JIT-like execution of simple expressions such as `10 + 20' */
struct Dee_module_object;
#ifdef CONFIG_BUILDING_DEEMON
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
struct Dee_serial;
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeExec_CompileModuleStream_impl(struct Dee_serial *__restrict writer, DeeObject *source_stream,
                                 int start_line, int start_col, unsigned int mode,
                                 struct Dee_compiler_options *options, DeeObject *default_symbols);
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
INTDEF WUNUSED NONNULL((1)) DREF /*untracked*/ struct Dee_module_object *DCALL
DeeExec_CompileModuleStream_impl(DeeObject *source_stream,
                                 int start_line, int start_col, unsigned int mode,
                                 struct Dee_compiler_options *options, DeeObject *default_symbols);
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */
#endif /* CONFIG_BUILDING_DEEMON */
DFUNDEF WUNUSED NONNULL((1)) DREF struct Dee_module_object *DCALL
DeeExec_CompileModuleStream(DeeObject *source_stream,
                            int start_line, int start_col, unsigned int mode,
                            struct Dee_compiler_options *options, DeeObject *default_symbols);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Callable*/ DeeObject *DCALL
DeeExec_CompileFunctionStream(DeeObject *source_stream,
                              int start_line, int start_col, unsigned int mode,
                              struct Dee_compiler_options *options, DeeObject *default_symbols);


/* Same as the functions above, but instead take a raw memory block as input */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeExec_RunMemory(/*utf-8*/ char const *__restrict data, size_t data_size,
                  size_t argc, DeeObject *const *argv,
                  int start_line, int start_col, unsigned int mode,
                  struct Dee_compiler_options *options, DeeObject *default_symbols);
DFUNDEF WUNUSED NONNULL((1)) DREF struct Dee_module_object *DCALL
DeeExec_CompileModuleMemory(/*utf-8*/ char const *__restrict data, size_t data_size,
                            int start_line, int start_col, unsigned int mode,
                            struct Dee_compiler_options *options, DeeObject *default_symbols);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Callable*/ DeeObject *DCALL
DeeExec_CompileFunctionMemory(/*utf-8*/ char const *__restrict data, size_t data_size,
                              int start_line, int start_col, unsigned int mode,
                              struct Dee_compiler_options *options, DeeObject *default_symbols);


/* Initialize the deemon runtime.
 * This does very little, as most components are designed for lazy initialization,
 * or are simply initialized statically (i.e. already come pre-initialized).
 * However, some components do require some pre-initialization, the most notable
 * here being `DeeThread_SubSystemInit()', as well as allocation of the memory
 * block used by the slab allocator. */
DFUNDEF void DCALL Dee_Initialize(void);

/* Keep clearing global hooks while invoking the GC to
 * finalize all user-objects that may still be loaded.
 * This function can be called any number of times, but
 * is intended to be called once before deemon gets unloaded.
 *
 * After this function was called, `Dee_Initialize()' must
 * be called before deemon APIs are once again safe to use.
 *
 * @param: Set of `Dee_SHUTDOWN_F_*' */
DFUNDEF void DCALL Dee_Shutdown(unsigned int flags);
#define Dee_SHUTDOWN_F_NORMAL 0x0000 /* Normal (full) shutdown */
#define Dee_SHUTDOWN_F_FAST   0x0001 /* Perform a "fast" shutdown that only guaranties that objects whose
                                      * destructors may have side-effects (including nested side-effects
                                      * such as destroying a tuple containing an object whose destructor
                                      * has a side-effect) are destroyed.
                                      * When this flag is set, deemon **WILL** leak **LOTS** of memory,
                                      * and it is **NOT** safe to call `Dee_Initialize()' again.
                                      * Only use this option if you:
                                      * - are about to exit(2) or return from your main()
                                      * - Fully unload deemon (i.e.: dlclose() the deemon core), or
                                      *   are not going to interact with deemon anymore, and don't
                                      *   care about the memory leaks that this flag causes. */

DECL_END

#endif /* !GUARD_DEEMON_EXEC_H */
