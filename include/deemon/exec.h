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
#ifndef GUARD_DEEMON_EXEC_H
#define GUARD_DEEMON_EXEC_H 1

#include "api.h"
#include "object.h"

DECL_BEGIN


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
DFUNDEF DREF /*String*/DeeObject *DCALL DeeExec_GetHome(void);
/* Set the new home folder, overwriting whatever was set before.
 * HINT: You may pass `NULL' to cause the default home path to be re-created. */
DFUNDEF void DCALL DeeExec_SetHome(/*String*/DeeObject *new_home);




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
#else
DDATDEF DeeObject     DeeModule_Path;
#endif
DFUNDEF void DCALL DeeModule_InitPath(void);
#define DeeModule_FiniPath()  DeeList_Clear(&DeeModule_Path)

/* Initialize the module path sub-system and return its global list of path. */
#define DeeModule_GetPath()  (DeeModule_InitPath(),&DeeModule_Path)

/* The timestamp when deemon was compiled, generated as `__DATE__ "|" __TIME__' */
DDATDEF char const DeeExec_Timestamp[];

/* Return the time (in milliseconds since 01.01.1970) when deemon was compiled.
 * This value is also used to initialize the `mo_ctime' value of the builtin
 * `deemon' module, automatically forcing user-code to be recompiled if the
 * associated deemon core has changed, and if they are using the `deemon' module. */
DDATDEF uint64_t DCALL DeeExec_GetTimestamp(void);


/* High-level functionality for registering at-exit hooks.
 * When executed, at-exit callbacks are run in order of execution.
 * NOTE: This function makes use of libC's `atexit()' function.
 *       When deemon was built with `CONFIG_NO_STDLIB', this
 *       function always throws an error and returns -1.
 * @param args: A tuple object the is used to invoke `callback'
 * @throw NotImplemented: Deemon was built with `CONFIG_NO_STDLIB'
 * @throw RuntimeError: `Dee_RunAtExit()' had already been invoked.
 * @return:  0: Successfully registered the given callback.
 * @return: -1: An error occurred or atexit() can no longer be used
 *              because `Dee_RunAtExit()' is being, or had been called. */
DFUNDEF int DCALL Dee_AtExit(DeeObject *__restrict callback,
                             DeeObject *__restrict args);

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



/* Get/Set the user-code argument vector that is
 * accessible from module-scope code using `...':
 * >> local argv = [...];
 * >> print repr argv; // ["my_script.dee","these","are","from","the","commandline"]
 * With that in mind, module root-code objects are varargs functions that are
 * invoked using the Argv tuple modifiable using this pair of functions.
 * The deemon launcher should call `Dee_SetArgv()' to set the original argument tuple.
 * NOTE: By default, an empty tuple is set for argv. */
DFUNDEF ATTR_RETNONNULL /*Tuple*/DREF DeeObject *DCALL Dee_GetArgv(void);
DFUNDEF NONNULL((1)) void DCALL Dee_SetArgv(/*Tuple*/DeeObject *__restrict argv);




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
