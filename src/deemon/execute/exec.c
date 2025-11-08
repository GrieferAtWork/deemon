/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_EXECUTE_EXEC_C
#define GUARD_DEEMON_EXECUTE_EXEC_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/gc.h>
#include <deemon/module.h>
#include <deemon/notify.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/sched/yield.h>

/* Pull in some header to form artificial dependencies in order
 * to force the timestamp of the builtin `deemon' module to be
 * incremented if anything in these headers changed. */
#include <deemon/asm.h>
#include "../runtime/builtin.h"
/**/

DECL_BEGIN


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
 * @param: default_symbols: A mapping-like object of type `{string: Object}', that
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
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeExec_RunStream(DeeObject *source_stream,
                  unsigned int mode,
                  size_t argc, DeeObject *const *argv,
                  int start_line, int start_col,
                  struct compiler_options *options,
                  DeeObject *default_symbols,
                  DeeObject *source_pathname,
                  DeeObject *module_name) {
	DREF DeeObject *function;
	function = DeeExec_CompileFunctionStream(source_stream,
	                                         mode,
	                                         start_line,
	                                         start_col,
	                                         options,
	                                         default_symbols,
	                                         source_pathname,
	                                         module_name);
	if unlikely(!function)
		goto err;
	return DeeObject_CallInherited(function, argc, argv);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeExec_RunStreamString(DeeObject *source_stream,
                        unsigned int mode,
                        size_t argc, DeeObject *const *argv,
                        int start_line, int start_col,
                        struct compiler_options *options,
                        DeeObject *default_symbols,
                        /*utf-8*/ char const *source_pathname,
                        size_t source_pathsize,
                        /*utf-8*/ char const *module_name,
                        size_t module_namesize) {
	DREF DeeObject *result;
	DREF DeeObject *source_pathname_ob = NULL;
	DREF DeeObject *module_name_ob     = NULL;
	if (source_pathname) {
		source_pathname_ob = DeeString_NewUtf8(source_pathname,
		                                       source_pathsize,
		                                       STRING_ERROR_FSTRICT);
		if unlikely(!source_pathname_ob)
			goto err;
	}
	if (module_name) {
		module_name_ob = DeeString_NewUtf8(module_name,
		                                   module_namesize,
		                                   STRING_ERROR_FSTRICT);
		if unlikely(!module_name_ob)
			goto err_source_pathname_ob;
	}
	result = DeeExec_RunStream(source_stream,
	                           mode,
	                           argc,
	                           argv,
	                           start_line,
	                           start_col,
	                           options,
	                           default_symbols,
	                           source_pathname_ob,
	                           module_name_ob);
	Dee_XDecref(module_name_ob);
	Dee_XDecref(source_pathname_ob);
	return result;
err_source_pathname_ob:
	Dee_XDecref(source_pathname_ob);
err:
	return NULL;
}


/* Similar to `DeeExec_RunStream()', but rather than directly executing it,
 * return the module or the module's root function used to describe the code
 * that is being executed. */
PUBLIC WUNUSED NONNULL((1)) /*Callable*/ DREF DeeObject *DCALL
DeeExec_CompileFunctionStream(DeeObject *source_stream,
                              unsigned int mode,
                              int start_line, int start_col,
                              struct compiler_options *options,
                              DeeObject *default_symbols,
                              DeeObject *source_pathname,
                              DeeObject *module_name) {
	DREF DeeObject *result;
	DREF DeeObject *module;
	module = DeeExec_CompileModuleStream(source_stream,
	                                     mode,
	                                     start_line,
	                                     start_col,
	                                     options,
	                                     default_symbols,
	                                     source_pathname,
	                                     module_name);
	if unlikely(!module)
		goto err;
	result = DeeModule_GetRoot(module, true);
	Dee_Decref(module);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) /*Module*/ DREF DeeObject *DCALL
DeeExec_CompileModuleStreamString(DeeObject *source_stream,
                                  unsigned int mode,
                                  int start_line, int start_col,
                                  struct compiler_options *options,
                                  DeeObject *default_symbols,
                                  /*utf-8*/ char const *source_pathname,
                                  size_t source_pathsize,
                                  /*utf-8*/ char const *module_name,
                                  size_t module_namesize) {
	DREF DeeObject *result;
	DREF DeeObject *source_pathname_ob = NULL;
	DREF DeeObject *module_name_ob     = NULL;
	if (source_pathname) {
		source_pathname_ob = DeeString_NewUtf8(source_pathname,
		                                       source_pathsize,
		                                       STRING_ERROR_FSTRICT);
		if unlikely(!source_pathname_ob)
			goto err;
	}
	if (module_name) {
		module_name_ob = DeeString_NewUtf8(module_name,
		                                   module_namesize,
		                                   STRING_ERROR_FSTRICT);
		if unlikely(!module_name_ob)
			goto err_source_pathname_ob;
	}
	result = DeeExec_CompileModuleStream(source_stream,
	                                     mode,
	                                     start_line,
	                                     start_col,
	                                     options,
	                                     default_symbols,
	                                     source_pathname_ob,
	                                     module_name_ob);
	Dee_XDecref(module_name_ob);
	Dee_XDecref(source_pathname_ob);
	return result;
err_source_pathname_ob:
	Dee_XDecref(source_pathname_ob);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) /*Callable*/ DREF DeeObject *DCALL
DeeExec_CompileFunctionStreamString(DeeObject *source_stream,
                                    unsigned int mode,
                                    int start_line, int start_col,
                                    struct compiler_options *options,
                                    DeeObject *default_symbols,
                                    /*utf-8*/ char const *source_pathname,
                                    size_t source_pathsize,
                                    /*utf-8*/ char const *module_name,
                                    size_t module_namesize) {
	DREF DeeObject *result;
	DREF DeeObject *module;
	module = DeeExec_CompileModuleStreamString(source_stream,
	                                           mode,
	                                           start_line,
	                                           start_col,
	                                           options,
	                                           default_symbols,
	                                           source_pathname,
	                                           source_pathsize,
	                                           module_name,
	                                           module_namesize);
	if unlikely(!module)
		goto err;
	result = DeeModule_GetRoot(module, true);
	Dee_Decref(module);
	return result;
err:
	return NULL;
}


/* Same as the functions above, but instead take a raw memory block as input */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeExec_RunMemory(/*utf-8*/ char const *__restrict data, size_t data_size,
                  unsigned int mode, size_t argc, DeeObject *const *argv,
                  int start_line, int start_col,
                  struct compiler_options *options,
                  DeeObject *default_symbols,
                  DeeObject *source_pathname,
                  DeeObject *module_name) {
	DREF DeeObject *result;
	DREF DeeObject *stream;
	stream = DeeFile_OpenRoMemory(data, data_size);
	if unlikely(!stream)
		goto err;
	result = DeeExec_RunStream(stream,
	                           mode,
	                           argc,
	                           argv,
	                           start_line,
	                           start_col,
	                           options,
	                           default_symbols,
	                           source_pathname,
	                           module_name);
	DeeFile_ReleaseMemory(stream);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeExec_RunMemoryString(/*utf-8*/ char const *__restrict data, size_t data_size,
                        unsigned int mode, size_t argc, DeeObject *const *argv,
                        int start_line, int start_col,
                        struct compiler_options *options,
                        DeeObject *default_symbols,
                        /*utf-8*/ char const *source_pathname,
                        size_t source_pathsize,
                        /*utf-8*/ char const *module_name,
                        size_t module_namesize) {
	DREF DeeObject *result;
	DREF DeeObject *stream;
	stream = DeeFile_OpenRoMemory(data, data_size);
	if unlikely(!stream)
		goto err;
	result = DeeExec_RunStreamString(stream,
	                                 mode,
	                                 argc,
	                                 argv,
	                                 start_line,
	                                 start_col,
	                                 options,
	                                 default_symbols,
	                                 source_pathname,
	                                 source_pathsize,
	                                 module_name,
	                                 module_namesize);
	DeeFile_ReleaseMemory(stream);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) /*Module*/ DREF DeeObject *DCALL
DeeExec_CompileModuleMemory(/*utf-8*/ char const *__restrict data, size_t data_size,
                            unsigned int mode, int start_line, int start_col,
                            struct compiler_options *options,
                            DeeObject *default_symbols,
                            DeeObject *source_pathname,
                            DeeObject *module_name) {
	DREF DeeObject *result;
	DREF DeeObject *stream;
	stream = DeeFile_OpenRoMemory(data, data_size);
	if unlikely(!stream)
		goto err;
	result = DeeExec_CompileModuleStream(stream,
	                                     mode,
	                                     start_line,
	                                     start_col,
	                                     options,
	                                     default_symbols,
	                                     source_pathname,
	                                     module_name);
	DeeFile_ReleaseMemory(stream);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) /*Callable*/ DREF DeeObject *DCALL
DeeExec_CompileFunctionMemory(/*utf-8*/ char const *__restrict data, size_t data_size,
                              unsigned int mode, int start_line, int start_col,
                              struct compiler_options *options,
                              DeeObject *default_symbols,
                              DeeObject *source_pathname,
                              DeeObject *module_name) {
	DREF DeeObject *result;
	DREF DeeObject *stream;
	stream = DeeFile_OpenRoMemory(data, data_size);
	if unlikely(!stream)
		goto err;
	result = DeeExec_CompileFunctionStream(stream,
	                                       mode,
	                                       start_line,
	                                       start_col,
	                                       options,
	                                       default_symbols,
	                                       source_pathname,
	                                       module_name);
	DeeFile_ReleaseMemory(stream);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) /*Module*/ DREF DeeObject *DCALL
DeeExec_CompileModuleMemoryString(/*utf-8*/ char const *__restrict data, size_t data_size,
                                  unsigned int mode, int start_line, int start_col,
                                  struct compiler_options *options,
                                  DeeObject *default_symbols,
                                  /*utf-8*/ char const *source_pathname,
                                  size_t source_pathsize,
                                  /*utf-8*/ char const *module_name,
                                  size_t module_namesize) {
	DREF DeeObject *result;
	DREF DeeObject *stream;
	stream = DeeFile_OpenRoMemory(data, data_size);
	if unlikely(!stream)
		goto err;
	result = DeeExec_CompileModuleStreamString(stream,
	                                           mode,
	                                           start_line,
	                                           start_col,
	                                           options,
	                                           default_symbols,
	                                           source_pathname,
	                                           source_pathsize,
	                                           module_name,
	                                           module_namesize);
	DeeFile_ReleaseMemory(stream);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) /*Callable*/ DREF DeeObject *DCALL
DeeExec_CompileFunctionMemoryString(/*utf-8*/ char const *__restrict data, size_t data_size,
                                    unsigned int mode, int start_line, int start_col,
                                    struct compiler_options *options,
                                    DeeObject *default_symbols,
                                    /*utf-8*/ char const *source_pathname,
                                    size_t source_pathsize,
                                    /*utf-8*/ char const *module_name,
                                    size_t module_namesize) {
	DREF DeeObject *result;
	DREF DeeObject *stream;
	stream = DeeFile_OpenRoMemory(data, data_size);
	if unlikely(!stream)
		goto err;
	result = DeeExec_CompileFunctionStreamString(stream,
	                                             mode,
	                                             start_line,
	                                             start_col,
	                                             options,
	                                             default_symbols,
	                                             source_pathname,
	                                             source_pathsize,
	                                             module_name,
	                                             module_namesize);
	DeeFile_ReleaseMemory(stream);
	return result;
err:
	return NULL;
}




#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_lock_t atexit_lock = DEE_ATOMIC_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define atexit_lock_available()  Dee_atomic_lock_available(&atexit_lock)
#define atexit_lock_acquired()   Dee_atomic_lock_acquired(&atexit_lock)
#define atexit_lock_tryacquire() Dee_atomic_lock_tryacquire(&atexit_lock)
#define atexit_lock_acquire()    Dee_atomic_lock_acquire(&atexit_lock)
#define atexit_lock_waitfor()    Dee_atomic_lock_waitfor(&atexit_lock)
#define atexit_lock_release()    Dee_atomic_lock_release(&atexit_lock)

struct atexit_entry {
	DREF DeeObject      *ae_func; /* [1..1] The function that would be invoked. */
	DREF DeeTupleObject *ae_args; /* [1..1] Arguments passed to the function. */
};

/* [lock(atexit_lock)] The atexit list used by deemon. */
PRIVATE size_t atexit_size = 0;
PRIVATE struct atexit_entry *atexit_list = NULL;

#define ATEXIT_FNORMAL 0x0000 /* Normal atexit flags. */
#define ATEXIT_FDIDRUN 0x0001 /* `atexit_callback' has been executed. */
#ifdef CONFIG_HAVE_atexit
#define ATEXIT_FDIDREG 0x0002 /* `atexit_callback' was registered using `atexit()' */
#endif /* CONFIG_HAVE_atexit */
/* [lock(atexit_lock)] Set of `ATEXIT_F*' */
PRIVATE uint16_t atexit_mode = ATEXIT_FNORMAL;


/* Run callbacks that have been registered using `Dee_AtExit()'
 * @return:  0: Successfully executed all callbacks.
 * @return: -1: An error occurred (never returned when `DEE_RUNATEXIT_FRUNALL' is passed)
 * NOTE: This function is automatically called when `exit()'
 *       from stdlib is used to stop execution of the program. */
PUBLIC int DCALL
Dee_RunAtExit(uint16_t flags) {
	struct atexit_entry *list;
	size_t size;
	atexit_lock_acquire();
	/* Only execute atexit() once! */
	if ((atexit_mode & ATEXIT_FDIDRUN) && !atexit_size) {
		atexit_lock_release();
		goto done;
	}
	atexit_mode |= ATEXIT_FDIDRUN;
	list        = atexit_list;
	size        = atexit_size;
	atexit_list = NULL;
	atexit_size = 0;
	atexit_lock_release();
	while (size--) {
		if (!(flags & DEE_RUNATEXIT_FDONTRUN)) {
			DREF DeeObject *temp;
			/* Invoke the callback. */
			temp = DeeObject_Call(list[size].ae_func,
			                      DeeTuple_SIZE(list[size].ae_args),
			                      DeeTuple_ELEM(list[size].ae_args));
			if unlikely(!temp) {
				/* An error occurred. */
				if (flags & DEE_RUNATEXIT_FRUNALL) {
					/* Just dump the error (including interrupts). */
					DeeError_Print("Unhandled error in atexit() callback",
					               ERROR_PRINT_HANDLEINTR);
				} else {
					/* Restore the list (Since we've already set `ATEXIT_FDIDRUN'
					 * flag, we can be sure that there is no way anything was
					 * able to register additional callbacks, meaning that the
					 * list must still be empty) */
					++size; /* Restore the one entry we've just failed to execute. */
					atexit_lock_acquire();
					ASSERT(atexit_list == NULL);
					ASSERT(atexit_size == 0);
					atexit_list = list;
					atexit_size = size;
					atexit_lock_release();
					return -1;
				}
			} else {
				Dee_Decref(temp);
			}
		}
		Dee_Decref(list[size].ae_args);
		Dee_Decref(list[size].ae_func);
	}
	Dee_Free(list);
done:
	return 0;
}

#ifndef __LIBCCALL
#define __LIBCCALL /* nothing */
#endif /* !__LIBCCALL */

PRIVATE void __LIBCCALL atexit_callback(void) {
	/* Simply run all registered atexit() functions. */
	Dee_RunAtExit(DEE_RUNATEXIT_FRUNALL);
}

/* High-level functionality for registering at-exit hooks.
 * When executed, at-exit callbacks are run in order of being registered.
 * NOTE: This function makes use of libc's `atexit()' function (if available).
 * @param args: A tuple object the is used to invoke `callback'
 * @return:  0: Successfully registered the given callback.
 * @return: -1: An error occurred or atexit() can no longer be used
 *              because `Dee_RunAtExit()' is being, or had been called. */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
Dee_AtExit(DeeObject *callback, DeeObject *args) {
	struct atexit_entry *new_list;
	ASSERT_OBJECT(callback);
	ASSERT_OBJECT_TYPE_EXACT(args, &DeeTuple_Type);
again:
	atexit_lock_acquire();
	/* Check if we're still allowed to register new entries. */
	if (atexit_mode & ATEXIT_FDIDRUN) {
		atexit_lock_release();
		DeeError_Throwf(&DeeError_RuntimeError,
		                "atexit() cannot be called after "
		                "callbacks had already been executed");
		goto err;
	}
	/* Allocate more entries. */
	new_list = (struct atexit_entry *)Dee_TryReallocc(atexit_list,
	                                                  atexit_size + 1,
	                                                  sizeof(struct atexit_entry));
	if unlikely(!new_list) {
		size_t old_size = atexit_size;
		atexit_lock_release();
		if (Dee_CollectMemoryc(old_size + 1, sizeof(struct atexit_entry)))
			goto again;
		goto err;
	}
	atexit_list = new_list;
	new_list += atexit_size++;
#ifdef CONFIG_HAVE_atexit
	/* If the atexit-callback hasn't been registered yet, do that now. */
	if (!(atexit_mode & ATEXIT_FDIDREG)) {
		/* Don't bother handling errors returned by `atexit()'... */
		atexit(&atexit_callback);
		atexit_mode |= ATEXIT_FDIDREG;
	}
#endif /* CONFIG_HAVE_atexit */

	/* Initialize the new callback entry. */
	Dee_Incref(callback);
	Dee_Incref(args);
	new_list->ae_func = callback;
	new_list->ae_args = (DREF DeeTupleObject *)args;
	atexit_lock_release();
	return 0;
err:
	return -1;
}



#ifdef CONFIG_NO_DEC
PRIVATE uint64_t exec_timestamp = (uint64_t)-1;
#define HAS_EXEC_TIMESTAMP    (exec_timestamp != (uint64_t)-1)
#define SET_EXEC_TIMESTAMP(x) (exec_timestamp = (x))
#else /* CONFIG_NO_DEC */
#define exec_timestamp        DeeModule_Deemon.mo_ctime
#define HAS_EXEC_TIMESTAMP    (DeeModule_Deemon.mo_flags & MODULE_FHASCTIME)
#define SET_EXEC_TIMESTAMP(x) (exec_timestamp = (x), atomic_or(&DeeModule_Deemon.mo_flags, MODULE_FHASCTIME))
#endif /* !CONFIG_NO_DEC */


/* A couple of helper macros taken from the libtime DEX. */
#define time_year2day(value) ((146097 * (value)) / 400) /* TODO: REMOVE ME */
#define MICROSECONDS_PER_MILLISECOND UINT64_C(1000)
#define MILLISECONDS_PER_SECOND      UINT64_C(1000)
#define SECONDS_PER_MINUTE           UINT64_C(60)
#define MINUTES_PER_HOUR             UINT64_C(60)
#define HOURS_PER_DAY                UINT64_C(24)
#define MICROSECONDS_PER_SECOND      (MICROSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND)
#define MICROSECONDS_PER_MINUTE      (MICROSECONDS_PER_SECOND * SECONDS_PER_MINUTE)
#define MICROSECONDS_PER_HOUR        (MICROSECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define MICROSECONDS_PER_DAY         (MICROSECONDS_PER_HOUR * HOURS_PER_DAY)


#ifdef BUILD_TIMESTAMP
#define WRAP_UINT64_C(x) UINT64_C(x)
#define parse_timestamp() (WRAP_UINT64_C(BUILD_TIMESTAMP) * MICROSECONDS_PER_SECOND)
#elif defined(_MSC_VER) && !defined(__clang__) && defined(__PE__)
extern /*IMAGE_DOS_HEADER*/ uint8_t const __ImageBase[];
LOCAL uint64_t parse_timestamp(void) {
	uint32_t e_lfanew      = *(uint32_t const *)(__ImageBase + 60);
	uint32_t TimeDateStamp = *(uint32_t const *)(__ImageBase + e_lfanew + 8); /* Problem: this is 32-bit... */
	return (uint64_t)TimeDateStamp * MICROSECONDS_PER_SECOND;
}
#else /* ... */
/* The timestamp when deemon was compiled, generated as `__DATE__ "|" __TIME__'
 * CAUTION (and why we try not to use this variant):
 *    this timestamp string is LOCALTIME! (but we'd need it to be UTC)
 * That's why we only use this variant as the last-case fallback! */
PRIVATE char const exec_timestamp_str[] = __DATE__ "|" __TIME__;

#define TIMESTAMP_MON  (exec_timestamp_str)
#define TIMESTAMP_MDAY (exec_timestamp_str + 4)
#define TIMESTAMP_YEAR (exec_timestamp_str + 7)
#define TIMESTAMP_HOUR (exec_timestamp_str + 12)
#define TIMESTAMP_MIN  (exec_timestamp_str + 15)
#define TIMESTAMP_SEC  (exec_timestamp_str + 18)


LOCAL uint64_t parse_timestamp(void) {
	unsigned int monthday;
	unsigned int monthstart;
	unsigned int year;
	unsigned int is_leap_year;
	unsigned int hour;
	unsigned int minute;
	unsigned int second;

#define MONTHSTART_JAN is_leap_year ? 0 : 0
#define MONTHSTART_FEB is_leap_year ? 31 : 31
#define MONTHSTART_MAR is_leap_year ? 60 : 59
#define MONTHSTART_APR is_leap_year ? 91 : 90
#define MONTHSTART_MAY is_leap_year ? 121 : 120
#define MONTHSTART_JUN is_leap_year ? 152 : 151
#define MONTHSTART_JUL is_leap_year ? 182 : 181
#define MONTHSTART_AUG is_leap_year ? 213 : 212
#define MONTHSTART_SEP is_leap_year ? 244 : 243
#define MONTHSTART_OCT is_leap_year ? 274 : 273
#define MONTHSTART_NOV is_leap_year ? 305 : 304
#define MONTHSTART_DEC is_leap_year ? 335 : 334
	/* With any luck, any decent optimizer will get rid of most of this stuff...
	 * NOTE: That's also the reason why everything is written without
	 *       loops, or the use of api functions such as sscanf().
	 *       Additionally, every variable is assigned to once (potentially
	 *       in different branches of the same if-statement who's condition
	 *       should already be known at compile-time), meaning that constant
	 *       propagation should be fairly easy to achieve. */
	monthday = (((TIMESTAMP_MDAY[0] - '0') * 10) + (TIMESTAMP_MDAY[1] - '0')) - 1;
	year = (((TIMESTAMP_YEAR[0] - '0') * 1000) +
	        ((TIMESTAMP_YEAR[1] - '0') * 100) +
	        ((TIMESTAMP_YEAR[2] - '0') * 10) +
	        (TIMESTAMP_YEAR[3] - '0'));
	is_leap_year = !(year % 400) || ((year % 100) && !(year % 4));
	if (TIMESTAMP_MON[0] == 'J') {
		if (TIMESTAMP_MON[1] == 'a') {
			monthstart = MONTHSTART_JAN;
		} else if (TIMESTAMP_MON[2] == 'n') {
			monthstart = MONTHSTART_JUN;
		} else {
			monthstart = MONTHSTART_JUL;
		}
	} else if (TIMESTAMP_MON[0] == 'F') {
		monthstart = MONTHSTART_FEB;
	} else if (TIMESTAMP_MON[0] == 'M') {
		if (TIMESTAMP_MON[2] == 'r') {
			monthstart = MONTHSTART_MAR;
		} else {
			monthstart = MONTHSTART_MAY;
		}
	} else if (TIMESTAMP_MON[0] == 'A') {
		if (TIMESTAMP_MON[1] == 'p') {
			monthstart = MONTHSTART_APR;
		} else {
			monthstart = MONTHSTART_AUG;
		}
	} else if (TIMESTAMP_MON[0] == 'S') {
		monthstart = MONTHSTART_SEP;
	} else if (TIMESTAMP_MON[0] == 'O') {
		monthstart = MONTHSTART_OCT;
	} else if (TIMESTAMP_MON[0] == 'N') {
		monthstart = MONTHSTART_NOV;
	} else {
		monthstart = MONTHSTART_DEC;
	}

	/* Figure out the time. */
	hour   = ((TIMESTAMP_HOUR[0] - '0') * 10 + (TIMESTAMP_HOUR[1] - '0'));
	minute = ((TIMESTAMP_MIN[0] - '0') * 10 + (TIMESTAMP_MIN[1] - '0'));
	second = ((TIMESTAMP_SEC[0] - '0') * 10 + (TIMESTAMP_SEC[1] - '0'));

	/* Pack everything together and return the result. */
	return (((uint64_t)((time_year2day(year) + monthstart + monthday) -
	                    /* Subtract the year 1970 from the result. */
	                    time_year2day(1970)) *
	         MICROSECONDS_PER_DAY) +
	        /* Add the hour, minute and second to the calculation. */
	        ((uint64_t)hour * MICROSECONDS_PER_HOUR) +
	        ((uint64_t)minute * MICROSECONDS_PER_MINUTE) +
	        ((uint64_t)second * MICROSECONDS_PER_SECOND));
}
#endif /* !... */


/* Return the time (in UTC milliseconds since 01-01-1970) when deemon was compiled.
 * This value is also used to initialize the `mo_ctime' value of the builtin
 * `deemon' module, automatically forcing user-code to be recompiled if the
 * associated deemon core has changed, and if they are using the `deemon' module. */
PUBLIC uint64_t DCALL DeeExec_GetTimestamp(void) {
	uint64_t result = exec_timestamp;
	if (!HAS_EXEC_TIMESTAMP) {
		result = parse_timestamp();
		SET_EXEC_TIMESTAMP(result);
	}
	return result;
}




/* A reference to the user-code ARGV tuple. */
PRIVATE DREF DeeTupleObject *usercode_argv = (DREF DeeTupleObject *)Dee_EmptyTuple;

/* Get/Set the user-code argument vector that is
 * accessible from module-scope code using `...':
 * >> local argv = [...];
 * >> print repr argv; // ["my_script.dee", "these", "are", "from", "the", "commandline"]
 * With that in mind, module root-code objects are varargs functions that are
 * invoked using the Argv tuple modifiable using this pair of functions.
 * The deemon launcher should call `Dee_SetArgv()' to set the original argument tuple.
 * NOTE: By default, an empty tuple is set for argv. */
PUBLIC WUNUSED ATTR_RETNONNULL /*Tuple*/ DREF DeeObject *DCALL Dee_GetArgv(void) {
	DREF DeeTupleObject *result;
	for (;;) {
		result = atomic_xch(&usercode_argv, NULL);
		if (result)
			break;
		SCHED_YIELD();
	}
	Dee_Incref(result);
	atomic_write(&usercode_argv, result);
	ASSERT_OBJECT_TYPE_EXACT(result, &DeeTuple_Type);
	return (DREF DeeObject *)result;
}

PUBLIC NONNULL((1)) void DCALL
Dee_SetArgv(/*Tuple*/ DeeObject *__restrict argv) {
	DREF DeeTupleObject *old_argv;
	ASSERT_OBJECT_TYPE_EXACT(argv, &DeeTuple_Type);
	Dee_Incref(argv);
	do {
		for (;;) {
			old_argv = atomic_read(&usercode_argv);
			if (old_argv)
				break;
			SCHED_YIELD();
		}
	} while (!atomic_cmpxch_weak_or_write(&usercode_argv, old_argv,
	                                      (DeeTupleObject *)argv));
	Dee_Decref(old_argv);
}

INTDEF bool DCALL libcodecs_shutdown(void);
INTDEF bool DCALL clear_strexec_cache(void);

PRIVATE bool DCALL shutdown_globals(void) {
	bool result;
	result = DeeModule_FiniPath();
	result |= libcodecs_shutdown();
	result |= clear_strexec_cache();
	result |= DeeFile_ResetStd();
	result |= DeeThread_ClearTls();
#ifndef CONFIG_NO_THREADS
	result |= DeeThread_InterruptAndJoinAll();
#endif /* !CONFIG_NO_THREADS */
#ifndef CONFIG_NO_DEX
	result |= DeeDex_Cleanup();
#endif /* !CONFIG_NO_DEX */
#ifndef CONFIG_NO_NOTIFICATIONS
	result |= DeeNotify_Shutdown();
#endif /* !CONFIG_NO_NOTIFICATIONS */
	return result;
}

/* The max number of GC iterations yielding
 * no results before user-code is killed. */
#define MAX_EMPTY_GC_ITERATIONS_BEFORE_KILL    16
/* The total max number of GC iterations before user-code is killed. */
#define MAX_NONEMPTY_GC_ITERATIONS_BEFORE_KILL 128

#ifndef NDEBUG
INTDEF void DCALL gc_dump_all(void);
#endif /* !NDEBUG */

#ifndef CONFIG_NO_OBJECT_SLABS
INTDEF void DCALL DeeSlab_Initialize(void);
INTDEF void DCALL DeeSlab_Finalize(void);
#endif /* !CONFIG_NO_OBJECT_SLABS */


/* Initialize the deemon runtime.
 * This does very little, as most components are designed for lazy initialization,
 * or are simply initialized statically (i.e. already come pre-initialized).
 * However, some components do require some pre-initialization, the most notable
 * here being `DeeThread_SubSystemInit()', as well as allocation of the memory
 * block used by the slab allocator. */
PUBLIC void DCALL Dee_Initialize(void) {

	/* Initialize the thread sub-system */
	DeeThread_SubSystemInit();

	/* Reserve system memory for slab allocators. */
#ifndef CONFIG_NO_OBJECT_SLABS
	DeeSlab_Initialize();
#endif /* !CONFIG_NO_OBJECT_SLABS */

	/* Install the keyboard interrupt handler. */
#ifndef CONFIG_NO_KEYBOARD_INTERRUPT
	DeeError_InstallKeyboardInterrupt();
#endif /* !CONFIG_NO_KEYBOARD_INTERRUPT */
}


#ifdef CONFIG_HOST_WINDOWS
#ifndef CONFIG_OUTPUTDEBUGSTRINGA_DEFINED
#define CONFIG_OUTPUTDEBUGSTRINGA_DEFINED
extern ATTR_DLLIMPORT void ATTR_STDCALL OutputDebugStringA(char const *lpOutputString);
extern ATTR_DLLIMPORT int ATTR_STDCALL IsDebuggerPresent(void);
#endif /* !CONFIG_OUTPUTDEBUGSTRINGA_DEFINED */
#endif /* CONFIG_HOST_WINDOWS */


/* Keep clearing global hooks while invoking the GC to
 * finalize all user-objects that may still be loaded.
 * This function can be called any number of times, but
 * is intended to be called once before deemon gets unloaded.
 * @return: * : The total number of GC object that were collected. */
PUBLIC size_t DCALL Dee_Shutdown(void) {
	size_t result = 0, temp;
	size_t num_gc = 0, num_empty_gc = 0;
	for (;;) {
		bool must_continue = false;

		/* Track how often we've already invoked the GC. */
		if (++num_gc == MAX_NONEMPTY_GC_ITERATIONS_BEFORE_KILL) {
do_kill_user:
			num_gc       = 0;
			num_empty_gc = 0;
#ifndef CONFIG_NO_THREADS
			/* Make sure that no secondary threads could enter an
			 * undefined state by us tinkering with their code. */
			must_continue |= DeeThread_InterruptAndJoinAll();
#endif /* !CONFIG_NO_THREADS */

			/* Tell the user about what's happening (stddbg is forwarded to stderr) */
			if (DeeFile_Printf(DeeFile_DefaultStddbg,
			                   "Stop executing user-code to fix unresolvable reference loop\n") < 0)
				DeeError_Print(NULL, Dee_ERROR_PRINT_HANDLEINTR);
			if (!DeeExec_KillUserCode()) {
				/* Well... shit!
				 * If we've gotten here, that probably means that there is some sort of
				 * resource leak caused by deemon itself, meaning it's probably my fault... */
#ifndef NDEBUG
#ifdef CONFIG_HOST_WINDOWS
				if (!IsDebuggerPresent())
					break;
#endif /* CONFIG_HOST_WINDOWS */
				/* Log all GC objects still alive */
				gc_dump_all();
				
#ifdef CONFIG_TRACE_REFCHANGES
				Dee_DPRINT("\n\n\nReference Leaks:\n");
				Dee_DumpReferenceLeaks();
#endif /* CONFIG_TRACE_REFCHANGES */
				Dee_BREAKPOINT();
#else /* !NDEBUG */
				/* Silently hide the shame when not built for debug mode... */
#endif /* NDEBUG */
				break;
			}
		}

		/* Do an initial shutdown (clear) on all global variables. */
		must_continue |= shutdown_globals();

		/* Collect as many GC objects as possible. */
		temp = DeeGC_Collect((size_t)-1);
		if (temp) {
			result += temp;
			num_empty_gc = 0; /* Reset the empty-gc counter. */
			continue;
		}

		/* Make sure that nothing is hiding in globals,
		 * now that the garbage has been collected. */
		must_continue |= shutdown_globals();

		/* If we already know that we must continue, don't even
		 * bother looking at the GC chain as it stands right now. */
		if (must_continue)
			continue;

		/* Check if the GC truly is empty, and also
		 * ensure that globals haven't been trying
		 * to hide in here. */
		if (!DeeGC_IsEmptyWithoutDex()) {
			++num_empty_gc;
			if (num_empty_gc == MAX_EMPTY_GC_ITERATIONS_BEFORE_KILL)
				goto do_kill_user;
			continue;
		}

		/* If nothing's left to-be done, _then_ we can stop. */
		if (!must_continue)
			break;
	}

	/* Uninstall the keyboard interrupt handler. */
#ifndef CONFIG_NO_KEYBOARD_INTERRUPT
	DeeError_UninstallKeyboardInterrupt();
#endif /* !CONFIG_NO_KEYBOARD_INTERRUPT */

	/* Shutdown all loaded DEX extensions. */
#ifndef CONFIG_NO_DEX
	Dee_CHECKMEMORY();
	DeeDex_Finalize();
	Dee_CHECKMEMORY();
#endif /* !CONFIG_NO_DEX */

	/* Free up cached objects. They're not actually memory leaks, mkay?
	 * Any malloc()-ed heap-block at application exit is a pretty broad
	 * definition of memory leaks. While the set of data encompassed by
	 * this definition does include any kind of memory leak caused by
	 * malloc(), it also includes other stuff that should't be considered
	 * a leak, which is mainly qualified by this stuff right here:
	 *   - Lazily allocated global data structures:
	 *     It's not a memory leak because there can only ever be
	 *     one of them allocated a time. And if we fail to release
	 *     it at application exit, the kernel will just do that for
	 *     us.
	 *      - DeeExec_SetHome
	 *      - DeeThread_SubSystemFini
	 *   - Preallocated object caches:
	 *     These are just another abstraction layer between the
	 *     universal heap, and a very specific kind of allocation,
	 *     which are meant to speed up allocating objects of known
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
	Dee_CHECKMEMORY();
	DeeThread_SubSystemFini();
	Dee_CHECKMEMORY();
#endif /* !NDEBUG || CONFIG_TRACE_REFCHANGES */
#ifndef NDEBUG
	DeeMem_ClearCaches((size_t)-1);
	Dee_CHECKMEMORY();
#endif /* !NDEBUG */

	/* Deallocate slab caches. */
#ifndef CONFIG_NO_OBJECT_SLABS
	DeeSlab_Finalize();
#endif /* !CONFIG_NO_OBJECT_SLABS */
	return result;
}


DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_EXEC_C */
