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
#ifndef GUARD_DEX_IPC_LIBIPC_H
#define GUARD_DEX_IPC_LIBIPC_H 1

#include <deemon/api.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif

#include <deemon/dex.h>
#include <deemon/object.h>
#include <deemon/file.h>

DECL_BEGIN

INTDEF DeeFileTypeObject DeePipe_Type; /* Extends `system_file' */
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
 * >> property files -> {system_file...};
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
 * >> function started() -> bool;
 * >> function detached() -> bool;
 * >> function terminated() -> bool;
 * >> function isachild() -> bool;
 *
 * Execution control functions:
 * >> function start() -> none;
 * >> function id() -> int;
 *    Return the process's pid
 * >> function join() -> int;
 * >> function tryjoin() -> (bool,int);
 * >> function timedjoin(timeout_in_microseconds:?Dint) -> (bool,int);
 * >> function detach() -> bool;
 *    Returns false if the process already was detached.
 * >> function terminate(int exit_code = 0) -> bool;
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
INTDEF DREF DeeObject *DCALL
nt_GetProcessAttribute(HANDLE *__restrict lphProcess,
                       DWORD dwProcessId,
                       DWORD dwAttributeId);

INTDEF DREF DeeObject *DCALL
nt_GetModuleFileName(HMODULE hModule);

/* @return: * :              The full process image name.
 * @return: Dee_EmptyString: The process is a zombie.
 * @return: NULL:            An error occurred.
 * @return: ITER_DONE:       The host does not implement this functionality. Try
 *                           `nt_GetProcessAttribute()' with `PROCATTR_IMAGEPATHNAME' instead. */
INTDEF DREF DeeObject *DCALL
nt_QueryFullProcessImageName(HANDLE hProcess, DWORD dwFlags);

/* Work around problems with windows permissions. */
INTDEF BOOL DCALL nt_GetExitCodeProcess(DWORD dwProcessId, HANDLE hProcess, LPDWORD lpExitCode);

#endif

DECL_END

#endif /* !GUARD_DEX_IPC_LIBIPC_H */
