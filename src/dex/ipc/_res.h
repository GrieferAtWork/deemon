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
#ifndef GUARD_DEX_IPC_STRINGS_H
#define GUARD_DEX_IPC_STRINGS_H 1

#include <deemon/api.h>
#include <deemon/dex.h>

DECL_BEGIN

/* Process */
#define S_Process_tp_name "Process"
#define S_Process_tp_doc                                                                      \
	DOC("(exe:?X3?Dint?DFile?Dstring,args?:?S?Dstring)\n"                                     \
	    "@param args A sequence of :{string}s that should be passed as secondary arguments. " \
	    "When provided, @exe is prepended first, however the full argument list "             \
	    "can be overwritten using the ?#argv property\n"                                      \
	    "Construct a new, non-started process for application @exe\n"                         \
	    "If provided, additional arguments @args are passed "                                 \
	    "to the process's C-level ${main()} function\n"                                       \
	    "To start a process, invoke the ?#start function, prior to which "                    \
	    "you are able to override/redirect the full commandline (?#cmdline), "                \
	    "the full argument list (?#argv, including the implicit initial "                     \
	    "argument that normally defaults to @exe), or the standard file "                     \
	    "streams (?#stdin, ?#stdout, ?#stderr) using ?GPipe readers/writers.")
#define S_Process_function_start_name "start"
#define S_Process_function_start_doc                                                       \
	DOC("->?Dbool\n"                                                                       \
	    "@interrupt\n"                                                                     \
	    "@throw FileNotFound The specified executable could not be found\n"                \
	    "@throw FileAccessError The current user does not have permissions to access the " \
	    "executable, or the executable is lacking execute permissions\n"                   \
	    "@throw SystemError Failed to start the process for some reason\n"                 \
	    "Begin execution of the process")
#define S_Process_function_detach_name "detach"
#define S_Process_function_detach_doc                                             \
	DOC("->?Dbool\n"                                                              \
	    "@throw ValueError @this process was never started\n"                     \
	    "@throw ValueError @this process is not a child of the calling process\n" \
	    "@throw SystemError Failed to detach @this process for some reason\n"     \
	    "@return true: The ?GProcess has been detached\n"                         \
	    "@return false: The ?GProcess was already detached\n"                     \
	    "Detaches @this process")
#define S_Process_function_terminate_name "terminate"
#define S_Process_function_terminate_doc                                         \
	DOC("(exitcode=!0)->?Dbool\n"                                                \
	    "@throw ValueError @this process was never started\n"                    \
	    "@throw SystemError Failed to terminate @this process for some reason\n" \
	    "@return true: The ?Gprocess has been terminated\n"                      \
	    "@return false: The ?Gprocess was already terminated\n"                  \
	    "Terminate @this process with the given @exitcode")
#define S_Process_function_join_name "join"
#define S_Process_function_join_doc                                         \
	DOC("->?Dint\n"                                                         \
	    "@interrupt\n"                                                      \
	    "@throw ValueError @this process was never started\n"               \
	    "@throw SystemError Failed to join @this process for some reason\n" \
	    "@return The exit code of the process\n"                            \
	    "Joins @this process and returns the return value of its main function")
#define S_Process_function_tryjoin_name "tryjoin"
#define S_Process_function_tryjoin_doc                                      \
	DOC("->?X2?Dint?N\n"                                                    \
	    "@throw ValueError @this process was never started\n"               \
	    "@throw SystemError Failed to join @this process for some reason\n" \
	    "Same as ?#join, but don't check for interrupts and fail immediately")
#define S_Process_function_timedjoin_name "timedjoin"
#define S_Process_function_timedjoin_doc                                    \
	DOC("(timeout_in_microseconds:?Dint)->?X2?Dint?N\n"                     \
	    "@throw ValueError @this process was never started\n"               \
	    "@throw SystemError Failed to join @this process for some reason\n" \
	    "Same as ?#join, but only attempt to join for a given @timeout_in_microseconds")
#define S_Process_getset_hasstarted_name "hasstarted"
#define S_Process_getset_hasstarted_doc \
	DOC("->?Dbool\n"                    \
	    "Returns ?t if @this process was started")
#define S_Process_getset_wasdetached_name "wasdetached"
#define S_Process_getset_wasdetached_doc \
	DOC("->?Dbool\n"                     \
	    "Returns ?t if @this process has been detached")
#define S_Process_getset_hasterminated_name "hasterminated"
#define S_Process_getset_hasterminated_doc \
	DOC("->?Dbool\n"                       \
	    "Returns ?t if @this process has terminated")
#define S_Process_getset_isachild_name "isachild"
#define S_Process_getset_isachild_doc \
	DOC("->?Dbool\n"                  \
	    "Returns ?t if @this process is a child of the calling process")
#define S_Process_getset_id_name "id"
#define S_Process_getset_id_doc                                                       \
	DOC("->?Dint\n"                                                                   \
	    "@throw ValueError The process hasn't been started yet\n"                     \
	    "@throw SystemError The system does not provide a way to query process ids\n" \
	    "Returns an operating-system specific id of @this process")
#define S_Process_getset_stdin_name "stdin"
#define S_Process_getset_stdin_doc \
	DOC("->?DFile\n"               \
	    "Get or set the standard input stream used by the process")
#define S_Process_getset_stdout_name "stdout"
#define S_Process_getset_stdout_doc \
	DOC("->?DFile\n"                \
	    "Get or set the standard output stream used by the process")
#define S_Process_getset_stderr_name "stderr"
#define S_Process_getset_stderr_doc \
	DOC("->?DFile\n"                \
	    "Get or set the standard error stream used by the process")
#define S_Process_getset_pwd_name "pwd"
#define S_Process_getset_pwd_doc \
	DOC("->?DFile\n"             \
	    "Get or set the process working directory used by the process")
#define S_Process_getset_exe_name "exe"
#define S_Process_getset_exe_doc \
	DOC("->?Dstring\n"           \
	    "The filename of the image being executed by the process")
#define S_Process_getset_cmdline_name "cmdline"
#define S_Process_getset_cmdline_doc \
	DOC("->?Dstring\n"               \
	    "The commandline used to executed the process")
#define S_Process_getset_argv_name "argv"
#define S_Process_getset_argv_doc \
	DOC("->?S?Dstring\n"          \
	    "The argument vector passed to the programms C-main() method")
#define S_Process_getset_cmd_name "cmd"
#define S_Process_getset_cmd_doc \
	DOC("->?Dstring\n"           \
	    "Deprecated alias for #cmdline")
#define S_Process_getset_environ_name "environ"
#define S_Process_getset_environ_doc \
	DOC("->?S?T2?Dstring?Dstring\n"  \
	    "The state of environment variables in the given process")
#define S_Process_getset_memory_name "memory"
#define S_Process_getset_memory_doc                                         \
	DOC("->?DFile\n"                                                        \
	    "File-based access to the process's VM\n"                           \
	    "Warning: Modifying memory without caution leads to instability.\n" \
	    "Writing to ${Process.current.memory} will most likely crash deemon")
#define S_Process_getset_threads_name "threads"
#define S_Process_getset_threads_doc \
	DOC("->?S?DThread\n"             \
	    "Enumerate all the threads of @this process")
#define S_Process_getset_files_name "files"
#define S_Process_getset_files_doc \
	DOC("->?S?DFile"               \
	    "Enumerate all the open files of @this process")
#define S_Process_class_function_self_name "self"
#define S_Process_class_function_self_doc \
	DOC("->?.\n"                          \
	    "Deprecated alias for ?#current")
#define S_Process_member_current_name "current"
#define S_Process_member_current_doc NULL

/* ProcessThreads (sequence returned by Process.threads) */
#define S_ProcessThreads_tp_name "ProcessThreads"
#define S_ProcessThreads_tp_doc NULL
#define S_ProcessThreadsIterator_tp_name "ProcessThreadsIterator"
#define S_ProcessThreadsIterator_tp_doc NULL


/* ProcEnum */
#define S_ProcEnum_tp_name "_ProcEnum"
#define S_ProcEnum_tp_doc NULL
#define S_ProcEnumIterator_tp_name "_ProcEnumIterator"
#define S_ProcEnumIterator_tp_doc NULL



/* Pipe */
#define S_Pipe_tp_name "Pipe"
#define S_Pipe_tp_doc NULL
#define S_Pipe_function_new_name "new"
#define S_Pipe_function_new_doc                 \
	DOC("(size_hint=!0)->?T2?#Reader?#Writer\n" \
	    "Creates a new pair of linked pipe files")
#define S_Pipe_member_Reader_name "Reader"
#define S_Pipe_member_Reader_doc NULL
#define S_Pipe_member_Writer_name "Writer"
#define S_Pipe_member_Writer_doc NULL

/* PipeReader */
#define S_PipeReader_tp_name "PipeReader"
#define S_PipeReader_tp_doc NULL

/* PipeWriter */
#define S_PipeWriter_tp_name "PipeWriter"
#define S_PipeWriter_tp_doc NULL


DECL_END

#endif /* !GUARD_DEX_IPC_STRINGS_H */
