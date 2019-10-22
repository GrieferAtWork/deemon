/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEX_IPC_LIBIPC_H
#define GUARD_DEX_IPC_LIBIPC_H 1

#include <deemon/api.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

#include <deemon/dex.h>
#include <deemon/file.h>
#include <deemon/object.h>

#ifdef CONFIG_NO_IO_H
#undef CONFIG_HAVE_IO_H
#elif !defined(CONFIG_HAVE_IO_H) && \
      (defined(_MSC_VER) || __has_include(<io.h>))
#define CONFIG_HAVE_IO_H 1
#endif

#ifdef CONFIG_NO_DIRECT_H
#undef CONFIG_HAVE_DIRECT_H
#elif !defined(CONFIG_HAVE_DIRECT_H) && \
      (defined(_MSC_VER) || __has_include(<direct.h>))
#define CONFIG_HAVE_DIRECT_H 1
#endif

#ifdef CONFIG_NO_PROCESS_H
#undef CONFIG_HAVE_PROCESS_H
#elif !defined(CONFIG_HAVE_PROCESS_H) && \
      (defined(_MSC_VER) || __has_include(<process.h>))
#define CONFIG_HAVE_PROCESS_H 1
#endif

#ifdef CONFIG_NO_UNISTD_H
#undef CONFIG_HAVE_UNISTD_H
#elif !defined(CONFIG_HAVE_UNISTD_H) && \
      (defined(__linux__) || defined(__linux) || defined(linux) || \
       defined(__unix__) || defined(__unix) || defined(unix) || \
       __has_include(<unistd.h>))
#define CONFIG_HAVE_UNISTD_H 1
#endif

#ifdef CONFIG_NO_ERRNO_H
#undef CONFIG_HAVE_ERRNO_H
#elif !defined(CONFIG_HAVE_ERRNO_H) && \
      (defined(_MSC_VER) || \
       defined(__linux__) || defined(__linux) || defined(linux) || \
       defined(__unix__) || defined(__unix) || defined(unix) || \
       __has_include(<errno.h>))
#define CONFIG_HAVE_ERRNO_H 1
#endif

#ifdef CONFIG_NO_SIGNAL_H
#undef CONFIG_HAVE_SIGNAL_H
#elif !defined(CONFIG_HAVE_SIGNAL_H) && \
      (defined(_MSC_VER) || \
       defined(__linux__) || defined(__linux) || defined(linux) || \
       defined(__unix__) || defined(__unix) || defined(unix) || \
       __has_include(<signal.h>))
#define CONFIG_HAVE_SIGNAL_H 1
#endif


#ifdef CONFIG_NO_SCHED_H
#undef CONFIG_HAVE_SCHED_H
#elif !defined(CONFIG_HAVE_SCHED_H) && \
      (defined(__linux__) || defined(__linux) || defined(linux) || \
       defined(__unix__) || defined(__unix) || defined(unix) || \
       __has_include(<sched.h>))
#define CONFIG_HAVE_SCHED_H 1
#endif

#ifdef CONFIG_NO_SYS_STAT_H
#undef CONFIG_HAVE_SYS_STAT_H
#elif !defined(CONFIG_HAVE_SYS_STAT_H) && \
      (defined(_MSC_VER) || \
       defined(__linux__) || defined(__linux) || defined(linux) || \
       defined(__unix__) || defined(__unix) || defined(unix) || \
       __has_include(<sys/stat.h>))
#define CONFIG_HAVE_SYS_STAT_H 1
#endif

#ifdef CONFIG_NO_SYS_WAIT_H
#undef CONFIG_HAVE_SYS_WAIT_H
#elif !defined(CONFIG_HAVE_SYS_WAIT_H) && \
      (defined(__linux__) || defined(__linux) || defined(linux) || \
       defined(__unix__) || defined(__unix) || defined(unix) || \
       __has_include(<sys/wait.h>))
#define CONFIG_HAVE_SYS_WAIT_H 1
#endif

#ifdef CONFIG_NO_WAIT_H
#undef CONFIG_HAVE_WAIT_H
#elif !defined(CONFIG_HAVE_WAIT_H) && \
      (__has_include(<wait.h>))
#define CONFIG_HAVE_WAIT_H 1
#endif

#ifdef CONFIG_NO_SYS_SIGNALFD_H
#undef CONFIG_HAVE_SYS_SIGNALFD_H
#elif !defined(CONFIG_HAVE_SYS_SIGNALFD_H) && \
      (__has_include(<wait.h>))
#define CONFIG_HAVE_SYS_SIGNALFD_H 1
#endif


DECL_BEGIN

INTDEF DeeFileTypeObject DeePipe_Type; /* Extends `_SystemFile' */
INTDEF DeeFileTypeObject DeePipeReader_Type;
INTDEF DeeFileTypeObject DeePipeWriter_Type;

/* The main process type.
 *
 * Constructor:
 * >> process(string exe, sequence argv = []);
 * >> process(file exefp, sequence argv = []);
 * >> process(int exefd, sequence argv = []);
 * Construct a new process, using `[exe,argv...]' as arguments.
 * NOTE: Before start(), the arguments can modified again
 *       through use of the `argv' and `cmdline' properties.
 *
 * Thread enumeration functions:
 * >> property threads -> {thread...};
 *
 * File enumeration functions:
 * >> property files -> {_SystemFile...};
 *
 * Process environment control:
 * >> property stdin -> file;
 * >> property stdout -> file;
 * >> property stderr -> file;
 * >> property pwd -> string;
 * >> property exe -> string;
 * >> property cmdline -> string; // Simply an alias for `" ".join(argv)'
 * >> property argv -> {string...};
 * >> property environ -> {(string,string)...};
 * >> property memory -> file; // A file allowing read/write access to the process's VM
 *
 * Execution state functions:
 * >> function started(): bool;
 * >> function detached(): bool;
 * >> function terminated(): bool;
 * >> function isachild(): bool;
 *
 * Execution control functions:
 * >> function start() -> none;
 * >> function id() -> int;
 *    Return the process's pid
 * >> function join() -> int;
 * >> function tryjoin() -> (bool, int);
 * >> function timedjoin(timeout_in_microseconds:?Dint) -> (bool, int);
 * >> function detach(): bool;
 *    Returns false if the process already was detached.
 * >> function terminate(int exit_code = 0): bool;
 *    Returns false if the process already was terminated.
 *
 * Non-portable control functions:
 * >> function kill_np(int signo) -> none; // Unix: Send a signal.
 *
 */
INTDEF DeeTypeObject DeeProcess_Type;

/* Process enumeration types -> [process...] */
INTDEF DeeTypeObject DeeProcEnumIterator_Type;
INTDEF DeeTypeObject DeeProcEnum_Type;


#ifdef CONFIG_HOST_WINDOWS

#define PROCATTR_STANDARDINPUT         0 /* DREF DeeSystemFileObject * */
#define PROCATTR_STANDARDOUTPUT        1 /* DREF DeeSystemFileObject * */
#define PROCATTR_STANDARDERROR         2 /* DREF DeeSystemFileObject * */
#define PROCATTR_CURRENTDIRECTORY      3 /* DREF DeeStringObject * */
#define PROCATTR_DLLPATH               4 /* DREF DeeStringObject * */
#define PROCATTR_IMAGEPATHNAME         5 /* DREF DeeStringObject * */
#define PROCATTR_COMMANDLINE           6 /* DREF DeeStringObject * */
#define PROCATTR_ENVIRONMENT           7 /* DREF DeeSequenceObject * -- {(string,string)...} */

/* Read the attribute `dwAttributeId' (One of `PROCATTR_*')
 * for the given process `*lphProcess' with id `dwProcessId'.
 * @param: lphProcess:     [in|out] A handle to the process (may be replaced by a different handle, though the old handle is not closed)
 * @param: dwProcessId:     The ID of the process in question.
 * @param: dwAttributeType: The attribute that should be accessed (One of `PROCATTR_*').
 * @return: * :             An object encapsulating the value referred to by `dwAttributeId'
 * @return: NULL:           An error occurred. */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
nt_GetProcessAttribute(HANDLE *__restrict lphProcess,
                       DWORD dwProcessId,
                       DWORD dwAttributeId);

INTDEF WUNUSED DREF DeeObject *DCALL
nt_GetModuleFileName(HMODULE hModule);

/* @return: * :              The full process image name.
 * @return: Dee_EmptyString: The process is a zombie.
 * @return: NULL:            An error occurred.
 * @return: ITER_DONE:       The host does not implement this functionality. Try
 *                           `nt_GetProcessAttribute()' with `PROCATTR_IMAGEPATHNAME' instead. */
INTDEF WUNUSED DREF DeeObject *DCALL
nt_QueryFullProcessImageName(HANDLE hProcess, DWORD dwFlags);

/* Work around problems with windows permissions. */
INTDEF BOOL DCALL nt_GetExitCodeProcess(DWORD dwProcessId, HANDLE hProcess, LPDWORD lpExitCode);

#endif /* CONFIG_HOST_WINDOWS */

DECL_END

#endif /* !GUARD_DEX_IPC_LIBIPC_H */
