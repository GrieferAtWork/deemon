/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_EXEC_H
#define GUARD_DEEMON_EXEC_H 1

#include "api.h"

#include "object.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_compiler_options compiler_options
#endif /* DEE_SOURCE */


/* Get/Set deemon's home path.
 * The home path is used to locate builtin libraries, as well as extensions.
 * Upon first access, `DeeExec_GetHome()' will pre-initialize the home-path as follows:
 * >> deemon_home = fs.environ["DEEMON_HOME"];
 * >> if (deemon_home !is none) {
 * >>     deemon_home = fs.abspath(deemon_home);
 * >> } else {
 * >>#ifdef CONFIG_DEEMON_HOME
 * >>     deemon_home = CONFIG_DEEMON_HOME;
 * >>#else
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
 * >> fs.env.get("DEEMON_PATH","").split(fs.delim)...;
 * >> fs.join(DeeExec_GetHome(),"lib")
 *
 * This list is also used to locate system-include paths for the preprocessor,
 * in that every entry that is a string is an include-path after appending "/include":
 * >> function get_include_paths() { for (local x: DeeModule_Path) if (x is string) yield x.rstrip("/")+"/include"; }
 */
#ifdef GUARD_DEEMON_EXECUTE_MODPATH_C
DDATDEF DeeListObject DeeModule_Path;
#else /* GUARD_DEEMON_EXECUTE_MODPATH_C */
DDATDEF DeeObject     DeeModule_Path;
#endif /* !GUARD_DEEMON_EXECUTE_MODPATH_C */
DFUNDEF void DCALL DeeModule_InitPath(void);
#define DeeModule_FiniPath() DeeList_Clear(&DeeModule_Path)

/* Initialize the module path sub-system and return its global list of path. */
#define DeeModule_GetPath() (DeeModule_InitPath(), &DeeModule_Path)

/* The timestamp when deemon was compiled, generated as `__DATE__ "|" __TIME__' */
DDATDEF char const DeeExec_Timestamp[];

/* Return the time (in milliseconds since 01.01.1970) when deemon was compiled.
 * This value is also used to initialize the `mo_ctime' value of the builtin
 * `deemon' module, automatically forcing user-code to be recompiled if the
 * associated deemon core has changed, and if they are using the `deemon' module. */
DDATDEF uint64_t DCALL DeeExec_GetTimestamp(void);


/* High-level functionality for registering at-exit hooks.
 * When executed, at-exit callbacks are run in order of being registered.
 * NOTE: This function makes use of libC's `atexit()' function (if available).
 * @param args: A tuple object the is used to invoke `callback'
 * @return:  0: Successfully registered the given callback.
 * @return: -1: An error occurred or atexit() can no longer be used
 *              because `Dee_RunAtExit()' is being, or had been called. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
Dee_AtExit(DeeObject *callback, DeeObject *args);

/* Run callbacks that have been registered using `Dee_AtExit()'
 * @return:  0: Successfully executed all callbacks.
 * @return: -1: An error occurred (never returned when `DEE_RUNATEXIT_FRUNALL' is passed)
 * NOTE: This function is automatically called when `exit()'
 *       from stdlib is used to stop execution of the program. */
DFUNDEF int DCALL Dee_RunAtExit(uint16_t flags);
#define DEE_RUNATEXIT_FNORMAL  0x0000 /* Normal flags. */
#define DEE_RUNATEXIT_FRUNALL  0x0001 /* Always run all callbacks and display all errors
                                       * that may occur during their execution. */
#define DEE_RUNATEXIT_FDONTRUN 0x0002 /* Do not execute callbacks, but discard all of them.
                                       * However, still disallow further calls to `Dee_AtExit()' */

/* Terminate the application the same way `deemon.Error.AppExit.exit()' would,
 * either through use of `exit()' from <stdlib.h>, or by throwing an exception.
 * NOTE: When available, calling stdlib's `exit()' is identical to this.
 * @return: -1: If this function returns at all, it always returns `-1' */
DFUNDEF int DCALL Dee_Exit(int exitcode, bool run_atexit);


/* Get/Set the user-code argument vector that is
 * accessible from module-scope code using `...':
 * >> local argv = [...];
 * >> print repr argv; // ["my_script.dee","these","are","from","the","commandline"]
 * With that in mind, module root-code objects are varargs functions that are
 * invoked using the Argv tuple modifiable using this pair of functions.
 * The deemon launcher should call `Dee_SetArgv()' to set the original argument tuple.
 * NOTE: By default, an empty tuple is set for argv. */
DFUNDEF WUNUSED ATTR_RETNONNULL /*Tuple*/ DREF DeeObject *DCALL Dee_GetArgv(void);
DFUNDEF NONNULL((1)) void DCALL Dee_SetArgv(/*Tuple*/ DeeObject *__restrict argv);




#define DEE_EXEC_RUNMODE_EXPR                  0x0000 /* Parse, compile and execute a basic expression (same as `DEE_EXEC_RUNMODE_EXPR',
                                                       * but the compiler may choose not to allow function or class expressions, thus
                                                       * restricting the set of executable to only basic expressions)
                                                       * When this mode is chosen, and `DEE_EXEC_RUNMODE_FHASPP' isn't set (and specified
                                                       * compiler options don't prevent it), the compiler may choose to use a simpler,
                                                       * smaller compilation driver which doesn't include the overhead associated with
                                                       * the normal (full) compiler.
                                                       * NOTE: As far as comma-expressions go, simply return the last component. */
#define DEE_EXEC_RUNMODE_FULLEXPR              0x0001 /* Parse, compile and execute a single expression (e.g. `10 + 20' or `[]{ print 42; return "foo"; }') and return the result
                                                       * NOTE: As far as comma-expressions go, simply return the last component, but do allow assignments. */
#define DEE_EXEC_RUNMODE_STMT                  0x0002 /* Parse, compile and execute a single statement (e.g. `if (x) y; else z;') and return the result like `return ({ ... });' */
#define DEE_EXEC_RUNMODE_STMTS                 0x0003 /* Parse a whole module source as a sequence of statements. */
#define DEE_EXEC_RUNMODE_MASK                  0x000f /* Mode mask */
#define DEE_EXEC_RUNMODE_FDEFAULTS_ARE_GLOBALS 0x4000 /* FLAG: Default symbols are declared as read/write globals (otherwise, they are read-only constants) */
#define DEE_EXEC_RUNMODE_FHASPP                0x8000 /* FLAG: Enable the preprocessor.
                                                       *  - When not set, preprocessor directives, macros and escaped line-feeds are disabled. */
struct Dee_compiler_options;

/* Execute source code from `source_stream' and return the result of invoking it.
 * @param: source_stream:   The input stream from which to take input arguments.
 * @param: mode:            One of `DEE_EXEC_RUNMODE_*', optionally or'd with a set of `DEE_EXEC_RUNMODE_F*'
 * @param: argv:            Variable arguments passed to user-code 
 * @param: start_line:      The starting line number when compiling code. (zero-based)
 * @param: start_col:       The starting column number when compiling code. (zero-based)
 * @param: options:         A set of compiler options applicable for compiled code.
 *                          Note however that certain options have no effect, such
 *                          as the fact that peephole and other optimizations are
 *                          forced to be disabled, or DEC files are never generated,
 *                          all for reasons that should be quite obvious.
 * @param: default_symbols: A mapping-like object of type `{(string,object)...}', that
 *                          contains a set of pre-defined variables that should be made
 *                          available to the interactive source code by use of global
 *                          variables.
 *                          These are either provided as constants, or as globals,
 *                          depending on `DEE_EXEC_RUNMODE_FDEFAULTS_ARE_GLOBALS'
 * @param: source_pathname: The name for the source file (the path of which is
 *                          then used for relative import()s and #include's)
 * @param: module_name:     The name of the internal module, or NULL to determine automatically.
 *                          Note that the internal module is never registered globally, and
 *                          only exists as an anonymous module. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeExec_RunStream(DeeObject *source_stream, unsigned int mode,
                  size_t argc, DeeObject *const *argv, int start_line, int start_col,
                  struct Dee_compiler_options *options, DeeObject *default_symbols,
                  DeeObject *source_pathname, DeeObject *module_name);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeExec_RunStreamString(DeeObject *source_stream, unsigned int mode,
                        size_t argc, DeeObject *const *argv, int start_line, int start_col,
                        struct Dee_compiler_options *options, DeeObject *default_symbols,
                        /*utf-8*/ char const *source_pathname, size_t source_pathsize,
                        /*utf-8*/ char const *module_name, size_t module_namesize);


/* Similar to `DeeExec_RunStream()', but rather than directly executing it,
 * return the module used to describe the code that is being executed, or
 * some unspecified, callable object which (when invoked) executed the given
 * input code in one way or another.
 * It is up to the implementation if an associated module should simply be
 * generated, before that module's root is returned, or if the given user-code
 * is only executed when the function is called, potentially allowing for
 * JIT-like execution of simple expressions such as `10 + 20' */
DFUNDEF WUNUSED NONNULL((1)) /*Module*/ DREF DeeObject *DCALL
DeeExec_CompileModuleStream(DeeObject *source_stream,
                            unsigned int mode, int start_line, int start_col,
                            struct Dee_compiler_options *options, DeeObject *default_symbols,
                            DeeObject *source_pathname, DeeObject *module_name);
DFUNDEF WUNUSED NONNULL((1)) /*Callable*/ DREF DeeObject *DCALL
DeeExec_CompileFunctionStream(DeeObject *source_stream,
                              unsigned int mode, int start_line, int start_col,
                              struct Dee_compiler_options *options, DeeObject *default_symbols,
                              DeeObject *source_pathname, DeeObject *module_name);
DFUNDEF WUNUSED NONNULL((1)) /*Module*/ DREF DeeObject *DCALL
DeeExec_CompileModuleStreamString(DeeObject *source_stream,
                                  unsigned int mode, int start_line, int start_col,
                                  struct Dee_compiler_options *options, DeeObject *default_symbols,
                                  /*utf-8*/ char const *source_pathname, size_t source_pathsize,
                                  /*utf-8*/ char const *module_name, size_t module_namesize);
DFUNDEF WUNUSED NONNULL((1)) /*Callable*/ DREF DeeObject *DCALL
DeeExec_CompileFunctionStreamString(DeeObject *source_stream,
                                    unsigned int mode, int start_line, int start_col,
                                    struct Dee_compiler_options *options, DeeObject *default_symbols,
                                    /*utf-8*/ char const *source_pathname, size_t source_pathsize,
                                    /*utf-8*/ char const *module_name, size_t module_namesize);


/* Same as the functions above, but instead take a raw memory block as input */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeExec_RunMemory(/*utf-8*/ char const *__restrict data, size_t data_size,
                  unsigned int mode, size_t argc, DeeObject *const *argv,
                  int start_line, int start_col,
                  struct Dee_compiler_options *options, DeeObject *default_symbols,
                  DeeObject *source_pathname, DeeObject *module_name);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeExec_RunMemoryString(/*utf-8*/ char const *__restrict data, size_t data_size,
                        unsigned int mode, size_t argc, DeeObject *const *argv,
                        int start_line, int start_col,
                        struct Dee_compiler_options *options, DeeObject *default_symbols,
                        /*utf-8*/ char const *source_pathname, size_t source_pathsize,
                        /*utf-8*/ char const *module_name, size_t module_namesize);
DFUNDEF WUNUSED NONNULL((1)) /*Module*/ DREF DeeObject *DCALL
DeeExec_CompileModuleMemory(/*utf-8*/ char const *__restrict data, size_t data_size,
                            unsigned int mode, int start_line, int start_col,
                            struct Dee_compiler_options *options, DeeObject *default_symbols,
                            DeeObject *source_pathname, DeeObject *module_name);
DFUNDEF WUNUSED NONNULL((1)) /*Callable*/ DREF DeeObject *DCALL
DeeExec_CompileFunctionMemory(/*utf-8*/ char const *__restrict data, size_t data_size,
                              unsigned int mode, int start_line, int start_col,
                              struct Dee_compiler_options *options, DeeObject *default_symbols,
                              DeeObject *source_pathname, DeeObject *module_name);
DFUNDEF WUNUSED NONNULL((1)) /*Module*/ DREF DeeObject *DCALL
DeeExec_CompileModuleMemoryString(/*utf-8*/ char const *__restrict data, size_t data_size,
                                  unsigned int mode, int start_line, int start_col,
                                  struct Dee_compiler_options *options, DeeObject *default_symbols,
                                  /*utf-8*/ char const *source_pathname, size_t source_pathsize,
                                  /*utf-8*/ char const *module_name, size_t module_namesize);
DFUNDEF WUNUSED NONNULL((1)) /*Callable*/ DREF DeeObject *DCALL
DeeExec_CompileFunctionMemoryString(/*utf-8*/ char const *__restrict data, size_t data_size,
                                    unsigned int mode, int start_line, int start_col,
                                    struct Dee_compiler_options *options, DeeObject *default_symbols,
                                    /*utf-8*/ char const *source_pathname, size_t source_pathsize,
                                    /*utf-8*/ char const *module_name, size_t module_namesize);


/* Initialize the deemon runtime.
 * This does very little, as most components are designed for lazy initialization,
 * or are simply initialized statically (i.e. already come pre-initialized).
 * However, some components do require some pre-initialization, the most notable
 * here being `DeeThread_Init()', as well as allocation of the data block used by
 * the slab allocator. */
DFUNDEF void DCALL Dee_Initialize(void);

/* Keep clearing global hooks while invoking the GC to
 * finalize all user-objects that may still be loaded.
 * This function can be called any number of times, but
 * is intended to be called once before deemon gets unloaded.
 * @return: * : The total number of GC object that were collected. */
DFUNDEF size_t DCALL Dee_Shutdown(void);

#ifdef CONFIG_BUILDING_DEEMON
/* Used internally to prevent any further execution of user-code
 * and is called during `Dee_Shutdown()' when it becomes obvious
 * that deemon cannot be shutdown through conventional means.
 * To oversimplify such a situation, imagine user-code like this:
 * >> global global_instance;
 * >> 
 * >> class SelfRevivingDestructor {
 * >>     ~this() {
 * >>         // Allowed: The destructor revives itself by generating
 * >>         //          a new reference to itself within some global,
 * >>         //          or otherwise outside variable.
 * >>         global_instance = this;
 * >>     }
 * >> }
 * >> // Kick-start the evil...
 * >> SelfRevivingDestructor();
 * >> 
 * Now code like this might seem evil, however is 100% allowed, and even
 * when something like this is done, deemon will continue to try shutting
 * down normally, and without breaking any rules (just slightly altering
 * them in a way that remains consistent without introducing undefined
 * behavior).
 * How is this done? Well. If the name of this function hasn't
 * answered that yet, let me make it plain obvious:
 *   >> By killing™ all existing code objects.
 * This is quite the simple task as a matter of fact:
 *   - We use `tp_visit' to recursively visit all GC-objects,
 *     where for every `code' object that we encounter, we
 *     simply do an ATOMIC_WRITE of the first instruction byte
 *    (unless the code is empty?), to set it to `ASM_RET_NONE'
 *   - `code' objects are also GC objects, meaning that we can
 *     be sure that every existing piece of user-code can be
 *     reached by simply iterating all GC objects and filtering
 *     instances of `DeeCode_Type'.
 *     >> import deemon;
 *     >> for (local x: deemon.gc) {
 *     >>     if (x !is deemon.code)
 *     >>         continue;
 *     >>     ...
 *     >> }
 *   - That might seem dangerous, but consider the implications:
 *      - Assuming that the caller will continue to use `DeeThread_JoinAll()'
 *        to join any new threads that may appear, we can also assume
 *        that any running user-code function will eventually return
 *        to its caller. However: any further calls to other user-code
 *        functions will immediately return `none' (or throw StopIteration),
 *        meaning that while existing usercode can finish execution
 *        (unless it's assembly text describes a loop that jumps back
 *        to code address +0000, in which case the loop will terminate
 *        the next time it tries to wrap around), no new functions can
 *        be started (unless user-code _really_ _really_ tries super-hard
 *        to counteract this by somehow managing to re-compile itself?)
 *      - The caller (`Dee_Shutdown()') will have joined all existing
 *        threads, meaning that no user-code can still be running, also
 *        meaning that we are safe to modify any existing piece of user-text
 *        that might exist. (Otherwise, we'd have to do some complicated
 *        trickery by taking `DeeThread_SuspendAll()' into account)
 *   - Anyways. This way (assuming that all C code is free of reference leaks),
 *     it should be possible to stop intentional reference loops (such as
 *     the one caused by the persistent instance of `SelfRevivingDestructor'
 *     in the example above), still allowing ~actual~ destructors to do
 *     their work prior to this function being used to give the user a slap
 *     on the wrists, while all C-level cleanup functions can continue to
 *     operate normally and do all the cleanup that we can trust (or at
 *     least hope to be able to trust) not to cause something like this.
 * @return: * : The amount of code objects that were affected. */
INTDEF size_t DCALL DeeExec_KillUserCode(void);
#endif /* !CONFIG_BUILDING_DEEMON */


DECL_END

#endif /* !GUARD_DEEMON_EXEC_H */
