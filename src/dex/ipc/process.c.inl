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
#ifndef GUARD_DEX_IPC_PROCESS_C_INL
#define GUARD_DEX_IPC_PROCESS_C_INL 1
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/dict.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/notify.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/debug-alignment.h>
#include <hybrid/overflow.h>
#include <hybrid/sched/yield.h>
#include <hybrid/sequence/list.h>
/**/

#include "libipc.h"
/**/

#include <stdbool.h>
#include <stddef.h> /* size_t */
#ifdef CONFIG_HAVE_PATHS_H
#include <paths.h> /* _PATH_SHELLS */
#endif /* CONFIG_HAVE_PATHS_H */
#ifdef CONFIG_HAVE_VFORK_H
#include <vfork.h>
#endif /* CONFIG_HAVE_VFORK_H */

/* Figure out if `vfork()' causes the VM to be shared with a child-process */
#if defined(CONFIG_HAVE_vfork) && !defined(CONFIG_HAVE_VFORK_IS_FORK)
#ifdef __ARCH_HAVE_SHARED_VM_VFORK
#define CONFIG_IPC_HAVE_vfork_HAS_SHARED_VM
#elif defined(__KOS_SYSTEM_HEADERS__)
#ifdef __ARCH_HAVE_SHARED_VM_VFORK
#define CONFIG_IPC_HAVE_vfork_HAS_SHARED_VM
#else /* __ARCH_HAVE_SHARED_VM_VFORK */
#undef CONFIG_IPC_HAVE_vfork_HAS_SHARED_VM
#endif /* !__ARCH_HAVE_SHARED_VM_VFORK */
#elif defined(__linux__) || defined(__KOS__)
#define CONFIG_IPC_HAVE_vfork_HAS_SHARED_VM
#else /* ... */
/* Maybe, but better to be safe than sorry... */
#undef CONFIG_IPC_HAVE_vfork_HAS_SHARED_VM
#endif /* !... */
#endif /* CONFIG_HAVE_vfork */

/* Join the next-terminating child-process */
#undef ipc_joinany
#ifdef CONFIG_HAVE_wait
#define ipc_joinany(p_status) wait(pid, p_status)
#elif defined(CONFIG_HAVE_wait3)
#define ipc_joinany(p_status) wait3(p_status, 0, NULL)
#elif defined(CONFIG_HAVE_waitpid)
#define ipc_joinany(p_status) waitpid(-1, p_status, 0)
#elif defined(CONFIG_HAVE_wait4)
#define ipc_joinany(p_status) wait4(-1, p_status, 0, NULL)
#endif /* ... */

/* Figure out how we can join processes */
#undef ipc_joinpid
#undef ipc_tryjoinpid
#ifdef CONFIG_HAVE_waitpid
#define ipc_joinpid(pid, p_status) waitpid(pid, p_status, 0)
#ifdef CONFIG_HAVE_WNOHANG
#define ipc_tryjoinpid(pid, p_status) waitpid(pid, p_status, WNOHANG)
#endif /* CONFIG_HAVE_WNOHANG */
#elif defined(CONFIG_HAVE_wait4)
#define ipc_joinpid(pid, p_status) wait4(pid, p_status, 0, NULL)
#ifdef CONFIG_HAVE_WNOHANG
#define ipc_tryjoinpid(pid, p_status) wait4(pid, p_status, WNOHANG, NULL)
#endif /* CONFIG_HAVE_WNOHANG */
#elif defined(CONFIG_HAVE_cwait)
#ifndef WAIT_CHILD
#ifdef _WAIT_CHILD
#define WAIT_CHILD _WAIT_CHILD
#else /* _WAIT_CHILD */
#define WAIT_CHILD 0
#endif /* !_WAIT_CHILD */
#endif /* _WAIT_CHILD */
#define ipc_joinpid(pid, p_status) cwait(p_status, pid, WAIT_CHILD)
#elif defined(ipc_joinany)
#define ipc_joinpid_IS_ipc_joinpid_impl
#define ipc_joinpid(pid, p_status) ipc_joinpid_impl(pid, p_status)
#endif /* !... */

#if !defined(P_NOWAIT) && defined(_P_NOWAIT)
#define P_NOWAIT _P_NOWAIT
#endif /* !P_NOWAIT && _P_NOWAIT */

#undef PRFMAXdPID
#ifndef __SIZEOF_PID_T__
#define __SIZEOF_PID_T__ __SIZEOF_INT__
#endif /* !__SIZEOF_PID_T__ */
#if __SIZEOF_PID_T__ <= 1
#define PRFMAXdPID "-128"
#define PRFdPID    PRFd8
#elif __SIZEOF_PID_T__ <= 2
#define PRFMAXdPID "-32768"
#define PRFdPID    PRFd16
#elif __SIZEOF_PID_T__ <= 4
#define PRFMAXdPID "-2147483648"
#define PRFdPID    PRFd32
#elif __SIZEOF_PID_T__ <= 8
#define PRFMAXdPID "-9223372036854775808"
#define PRFdPID    PRFd64
#else /* __SIZEOF_PID_T__ <= ... */
#define PRFMAXdPID "-170141183460469231731687303715884105728"
#define PRFdPID    PRFd128
#endif /* __SIZEOF_PID_T__ > ... */


/* Check if we have the minimum requirements for doing whatever needs to be done after a `fork()' */
#undef CONFIG_IPC_HAVE_exec_requirements
#if (((defined(CONFIG_HAVE_wexecve) && (defined(CONFIG_HAVE_wexecv) || defined(CONFIG_HAVE_wenviron))) || \
      (defined(CONFIG_HAVE_execve) && (defined(CONFIG_HAVE_execv) || defined(CONFIG_HAVE_environ)))) &&   \
     (defined(CONFIG_HAVE_wchdir) || defined(CONFIG_HAVE_chdir)) &&                                       \
      defined(CONFIG_HAVE_dup2) && defined(CONFIG_HAVE__Exit) &&                                          \
      defined(CONFIG_HAVE_errno) && defined(ipc_joinpid))
#define CONFIG_IPC_HAVE_exec_requirements
#endif /* ... */




/************************************************************************/
/* FIGURE OUT HOW TO IMPLEMENT PROCESS SUPPORT                          */
/************************************************************************/
#undef ipc_Process_USE_CreateProcessW
#undef ipc_Process_USE_posix_spawn
#undef ipc_Process_USE_spawnve
/* TODO: Support for linux clone() with `CLONE_PIDFD' (in order to then use said pidfd to facilitate wait() operations) */
#undef ipc_Process_USE_vfork_AND_execve
#undef ipc_Process_USE_fork_AND_execve
#undef ipc_Process_USE_STUB
#if 0
#define ipc_Process_USE_fork_AND_execve
#undef CONFIG_PREFER_WCHAR_FUNCTIONS
#elif defined(CONFIG_HOST_WINDOWS)
#define ipc_Process_USE_CreateProcessW
#elif (defined(CONFIG_HAVE_SPAWN_H) && defined(CONFIG_HAVE_posix_spawn) && \
       defined(CONFIG_HAVE_posix_spawn_file_actions_adddup2) &&            \
       defined(CONFIG_HAVE_posix_spawn_file_actions_addchdir))
#define ipc_Process_USE_posix_spawn
#elif (defined(CONFIG_HAVE_vfork) && defined(CONFIG_IPC_HAVE_vfork_HAS_SHARED_VM) && defined(CONFIG_IPC_HAVE_exec_requirements))
#define ipc_Process_USE_vfork_AND_execve
#elif (defined(CONFIG_HAVE_fork) && defined(CONFIG_IPC_HAVE_exec_requirements) && \
       (defined(CONFIG_HAVE_pipe2) && defined(CONFIG_HAVE_O_CLOEXEC) &&           \
        defined(CONFIG_HAVE_close) && defined(CONFIG_HAVE_read) &&                \
        (defined(CONFIG_HAVE_write) || defined(CONFIG_HAVE_writeall))))
#define ipc_Process_USE_fork_AND_execve
#elif ((defined(CONFIG_HAVE_wspawnve) || defined(CONFIG_HAVE_spawnve)) && \
       ((defined(CONFIG_HAVE_wchdir) && defined(CONFIG_HAVE_wgetcwd)) ||  \
        (defined(CONFIG_HAVE_chdir) && defined(CONFIG_HAVE_getcwd))) &&   \
       defined(P_NOWAIT) && defined(CONFIG_HAVE_dup) &&                   \
       defined(CONFIG_HAVE_close) && defined(CONFIG_HAVE_dup2))
#define ipc_Process_USE_spawnve
#else /* ... */
#define ipc_Process_USE_STUB
#endif /* !... */

#ifndef CONFIG_HAVE_posix_spawnattr_init
#define posix_spawnattr_init(at) (bzero(at, sizeof(posix_spawnattr_t)), 0)
#endif /* !CONFIG_HAVE_posix_spawnattr_init */

#ifndef CONFIG_HAVE_posix_spawnattr_destroy
#define posix_spawnattr_destroy(at) 0
#endif /* !CONFIG_HAVE_posix_spawnattr_destroy */

#ifndef CONFIG_HAVE_posix_spawn_file_actions_init
#define posix_spawn_file_actions_init(fa) (bzero(fa, sizeof(posix_spawn_file_actions_t)), 0)
#endif /* !CONFIG_HAVE_posix_spawn_file_actions_init */

#ifndef CONFIG_HAVE_posix_spawn_file_actions_destroy
#define posix_spawn_file_actions_destroy(fa) 0
#endif /* !CONFIG_HAVE_posix_spawn_file_actions_destroy */

#ifdef ipc_Process_USE_CreateProcessW
#include <Windows.h>
#endif /* ipc_Process_USE_CreateProcessW */

#ifdef ipc_Process_USE_posix_spawn
#include <spawn.h>
#endif /* ipc_Process_USE_posix_spawn */

#ifndef PATH_MAX
#ifdef PATHMAX
#   define PATH_MAX PATHMAX
#elif defined(MAX_PATH)
#   define PATH_MAX MAX_PATH
#elif defined(MAXPATH)
#   define PATH_MAX MAXPATH
#else /* ... */
#   define PATH_MAX 260
#endif /* !... */
#endif /* !PATH_MAX */


/* Check if we want to support `/etc/shells' */
#undef CONFIG_IPC_HAVE_etc_shells
#ifndef CONFIG_HOST_WINDOWS
#define CONFIG_IPC_HAVE_etc_shells
#endif /* !CONFIG_HOST_WINDOWS */


DECL_BEGIN

#if (defined(ipc_Process_USE_posix_spawn) ||      \
     defined(ipc_Process_USE_vfork_AND_execve) || \
     defined(ipc_Process_USE_fork_AND_execve) ||  \
     defined(ipc_Process_USE_spawnve))
#define ipc_Process_USE_UNIX_APIS
#ifdef ipc_Process_USE_posix_spawn
#undef ipc_Process_USE_WCHAR_SPAWN_APIS
#undef ipc_Process_USE_WCHAR_CHDIR_APIS
#elif defined(ipc_Process_USE_vfork_AND_execve) || defined(ipc_Process_USE_fork_AND_execve)
#if defined(CONFIG_HAVE_execve) && defined(CONFIG_HAVE_fexecve)
#undef ipc_Process_USE_WCHAR_SPAWN_APIS /* Using the wide-character API would make it impossible to use `fexecve(2)' */
#elif defined(CONFIG_PREFER_WCHAR_FUNCTIONS) && defined(CONFIG_HAVE_wexecve)
#define ipc_Process_USE_WCHAR_SPAWN_APIS
#elif defined(CONFIG_HAVE_execve)
#undef ipc_Process_USE_WCHAR_SPAWN_APIS
#elif defined(CONFIG_HAVE_wexecve)
#define ipc_Process_USE_WCHAR_SPAWN_APIS
#endif /* ... */
#if defined(CONFIG_PREFER_WCHAR_FUNCTIONS) && defined(CONFIG_HAVE_wchdir)
#define ipc_Process_USE_WCHAR_CHDIR_APIS
#elif defined(CONFIG_HAVE_chdir)
#undef ipc_Process_USE_WCHAR_CHDIR_APIS
#elif defined(CONFIG_HAVE_wchdir)
#define ipc_Process_USE_WCHAR_CHDIR_APIS
#endif /* ... */
#elif defined(ipc_Process_USE_spawnve)
#if defined(CONFIG_PREFER_WCHAR_FUNCTIONS) && defined(CONFIG_HAVE_wspawnve)
#define ipc_Process_USE_WCHAR_SPAWN_APIS
#elif defined(CONFIG_HAVE_spawnve)
#undef ipc_Process_USE_WCHAR_SPAWN_APIS
#elif defined(CONFIG_HAVE_wspawnve)
#define ipc_Process_USE_WCHAR_SPAWN_APIS
#endif /* ... */
#if defined(CONFIG_PREFER_WCHAR_FUNCTIONS) && defined(CONFIG_HAVE_wchdir) && defined(CONFIG_HAVE_wgetcwd)
#define ipc_Process_USE_WCHAR_CHDIR_APIS
#elif defined(CONFIG_HAVE_chdir) && defined(CONFIG_HAVE_getcwd)
#undef ipc_Process_USE_WCHAR_CHDIR_APIS
#elif defined(CONFIG_HAVE_wchdir) && defined(CONFIG_HAVE_wgetcwd)
#define ipc_Process_USE_WCHAR_CHDIR_APIS
#endif /* ... */
#endif /* ... */
#endif /* ... */

#ifdef ipc_Process_USE_CreateProcessW
#define ipc_Process_pid_t         DWORD
#define ipc_Process_pid_t_INVALID 0
#elif defined(ipc_Process_USE_UNIX_APIS)
#ifndef CONFIG_HAVE_pid_t
#define CONFIG_HAVE_pid_t
#define pid_t int
#endif /* !CONFIG_HAVE_pid_t */
#define ipc_Process_pid_t         pid_t
#define ipc_Process_pid_t_INVALID 0
#ifdef ipc_joinpid_IS_ipc_joinpid_impl
#define NEED_ipc_joinpid_impl
#endif /* ipc_joinpid_IS_ipc_joinpid_impl */
#endif /* ... */


/* Figure out if we want the ext2path cache */
#undef ipc_Process_WANT_ipc_exe2path_cache
#ifdef ipc_Process_USE_CreateProcessW
#define ipc_Process_WANT_ipc_exe2path_cache
#endif /* ipc_Process_USE_CreateProcessW */
#if defined(ipc_Process_USE_UNIX_APIS)
#undef ipc_exe2path_USE_access
#undef ipc_exe2path_USE_waccess
#if defined(CONFIG_HAVE_waccess) && defined(X_OK) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define ipc_exe2path_USE_waccess
#elif defined(CONFIG_HAVE_access) && defined(X_OK)
#define ipc_exe2path_USE_access
#elif defined(CONFIG_HAVE_waccess) && defined(X_OK)
#define ipc_exe2path_USE_waccess
#endif
#if (defined(ipc_exe2path_USE_access) || \
     defined(ipc_exe2path_USE_waccess))
#define ipc_Process_WANT_ipc_exe2path_cache
#endif /* ... */
#endif /* ipc_Process_USE_UNIX_APIS */


/* Return the current process's PID */
#ifdef ipc_Process_USE_CreateProcessW
#define ipc_getpid() GetCurrentProcessId()
#elif defined(ipc_Process_USE_UNIX_APIS) && defined(CONFIG_HAVE_getpid)
#define ipc_getpid() getpid()
#elif defined(ipc_Process_USE_UNIX_APIS) && defined(CONFIG_HAVE_syscall) && defined(SYS_getpid)
#define ipc_getpid() syscall(SYS_getpid)
#elif defined(ipc_Process_USE_UNIX_APIS) && defined(CONFIG_HAVE_syscall) && defined(__NR_getpid)
#define ipc_getpid() syscall(__NR_getpid)
#endif /* ipc_Process_USE_CreateProcessW */



/*[[[deemon
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_SHELL", "SHELL");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_PATH", "PATH");
print("#ifdef ipc_Process_USE_CreateProcessW");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_PATHEXT", "PATHEXT");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_DEFAULT_SHELL", "cmd.exe");
print("#else /" "* ipc_Process_USE_CreateProcessW *" "/");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_DEFAULT_SHELL", "/bin/sh");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_DEFAULT_SHELL_C", "-c");
print("#endif /" "* !ipc_Process_USE_CreateProcessW *" "/");
]]]*/
PRIVATE DEFINE_STRING_EX(str_SHELL, "SHELL", 0xbbbeaf62, 0x99d1481fb3782307);
PRIVATE DEFINE_STRING_EX(str_PATH, "PATH", 0x842a043c, 0x9429f792a6880074);
#ifdef ipc_Process_USE_CreateProcessW
PRIVATE DEFINE_STRING_EX(str_PATHEXT, "PATHEXT", 0xe011e8eb, 0x16a0c5286fc39b1b);
PRIVATE DEFINE_STRING_EX(str_DEFAULT_SHELL, "cmd.exe", 0xc92c7d3f, 0xbc7fe7bdbefeaf0);
#else /* ipc_Process_USE_CreateProcessW */
PRIVATE DEFINE_STRING_EX(str_DEFAULT_SHELL, "/bin/sh", 0x472a3c8, 0xe73f0f00a36d316e);
PRIVATE DEFINE_STRING_EX(str_DEFAULT_SHELL_C, "-c", 0x609d4fb4, 0xfea31f2416d3d4bd);
#endif /* !ipc_Process_USE_CreateProcessW */
/*[[[end]]]*/





#ifdef ipc_Process_USE_CreateProcessW
/* Encode the process commandline in a way that is compatible
 * with the decoder found in any program compiled using VC/VC++
 * Additionally, try to escape as few characters as possible, and
 * use quotation marks as sparingly as possible.
 * RULES:
 *   - Only use `"' for escaping (`'' seems to be missing/broken)
 *   - When argv[i] contains any ` ' or `\t' characters,
 *     the argument is to be escaped by surrounding it with `"'
 *      - `foo bar'  --> `"foo bar"'
 *   - When argv[i] contains a `"' character, that character
 *     is prefixed with a `\' character
 *      - `foo"'  --> `foo\"'
 *   - Any number of `\' that are followed by a `"' is replaced
 *     with double that number of `\' characters, 
 *      - `foo\'  --> `foo\'
 *      - `foo\"' --> `foo\\\"'
 * HINT: The algorithm used by VC/VC++ is located
 *       in `crt/src/stdargv.c' of Visual Studio
 */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ipc_nt_cmdline_add_arg(struct unicode_printer *__restrict printer,
                       DeeStringObject *__restrict arg) {
	char const *begin, *iter, *end, *flush_start;
	bool must_quote = false;
	size_t start_length;
	if (!UNICODE_PRINTER_ISEMPTY(printer) && unicode_printer_putc(printer, ' '))
		goto err;
	start_length = UNICODE_PRINTER_LENGTH(printer);
	begin        = DeeString_AsUtf8((DeeObject *)arg);
	if unlikely(!begin)
		goto err;
	iter        = begin;
	flush_start = begin;
	end         = begin + WSTR_LENGTH(begin);
	for (; iter < end; ++iter) {
		if (*iter == '\"') {
			char const *quote_start = iter;
			while (iter > begin && iter[-1] == '\\')
				--iter;
			if (unicode_printer_print(printer, flush_start, (size_t)(iter - flush_start)) < 0)
				goto err;

			/* Escape by writing double the number of slashes. */
			if (quote_start != iter) {
				if (unicode_printer_print(printer, iter, (size_t)(quote_start - iter)) < 0 ||
				    unicode_printer_print(printer, iter, (size_t)(quote_start - iter)) < 0)
					goto err;
			}

			/* Following this, write the escaped quote. */
			if (unicode_printer_printascii(printer, "\\\"", 2) < 0)
				goto err;
			flush_start = iter + 1;
			/* Needed for cygwin:
			 * >> echo  foo\"bar   # Not parsed correctly? (but does work for programs compiled by msvc)
			 * >> echo  "foo\"bar" # This works as expected everywhere */
			must_quote = true;
		} else if (*iter == '\'') {
			/* Needed for cygwin:
			 * While msvc completely ignores '-characters on the commandline,
			 * cygwin does assign them a similar meaning as the "-character.
			 * As such, we must escape them by quoting the entire argument. */
			must_quote = true;
		} else if (*iter == ' ' || *iter == '\t') {
			must_quote = true;
		}
	}

	/* Flush the remainder of the argument. */
	if (unicode_printer_print(printer, flush_start, (size_t)(iter - flush_start)) < 0)
		goto err;

	/* Surround the argument with quotation marks. */
	if (must_quote) {
		size_t length = UNICODE_PRINTER_LENGTH(printer) - start_length;
		if unlikely(unicode_printer_printascii(printer, "\"\"", 2) < 0)
			goto err;

		/* Shift the argument text. */
		unicode_printer_memmove(printer,
		                        start_length + 1,
		                        start_length,
		                        length);

		/* Fill in leading quote. */
		UNICODE_PRINTER_SETCHAR(printer, start_length, '\"');
	}
	return 0;
err:
	return -1;
}

#define ipc_nt_cmdline_add_args(printer, args) \
	DeeObject_Foreach(args, &ipc_nt_cmdline_add_args_foreach_cb, printer)
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ipc_nt_cmdline_add_args_foreach_cb(void *__restrict arg,
                                   DeeObject *__restrict item) {
	struct unicode_printer *printer = (struct unicode_printer *)arg;
	if (DeeObject_AssertTypeExact(item, &DeeString_Type))
		goto err;
	return ipc_nt_cmdline_add_arg(printer, (DeeStringObject *)item);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeStringObject *DCALL
ipc_argv2cmdline(DeeObject *__restrict args) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(ipc_nt_cmdline_add_args(&printer, args))
		goto err;
	return (DREF DeeStringObject *)unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeStringObject *DCALL
ipc_argv2cmdline_cmd_c(DeeStringObject *__restrict cmd_exe,
                       DeeStringObject *__restrict command) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
#ifdef ipc_Process_USE_CreateProcessW
	if (cmd_exe == (DeeStringObject *)&str_DEFAULT_SHELL) {
		if unlikely(unicode_printer_printascii(&printer, "CMD.EXE /C", 10) < 0)
			goto err;
	} else
#endif /* ipc_Process_USE_CreateProcessW */
	{
		if unlikely(ipc_nt_cmdline_add_arg(&printer, cmd_exe))
			goto err;
		if unlikely(unicode_printer_printascii(&printer, " /C", 3) < 0)
			goto err;
	}
	if unlikely(ipc_nt_cmdline_add_arg(&printer, command))
		goto err;
	return (DREF DeeStringObject *)unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
ipc_cmdline2argv(DeeStringObject *__restrict cmdline) {
	char const *iter, *end;
	struct unicode_printer printer;
	DREF DeeObject *arg;
	DREF DeeTupleObject *result;
	size_t result_alloc, result_size;
	iter = DeeString_AsUtf8((DeeObject *)cmdline);
	if unlikely(!iter)
		goto err;
	end = iter + WSTR_LENGTH(iter);

	/* Allocate a tuple buffer for the to-be returned list of string. */
	result_alloc = 4;
	result = DeeTuple_TryNewUninitialized(result_alloc);
	if unlikely(!result) {
		result_alloc = 1;
		result = DeeTuple_TryNewUninitialized(result_alloc);
		if unlikely(!result) {
			result_alloc = 0;
			result     = (DeeTupleObject *)&DeeTuple_Empty;
			Dee_Incref(result);
		}
	}
	result_size = 0;
	while (iter < end) {
		bool is_quoting = false;
		unsigned int num_slashes;
		char const *flush_start;

		/* Skip leading whitespace. */
		while (*iter == ' ' || *iter == '\t')
			++iter;
		if (iter >= end)
			break; /* End of argument list. */
		unicode_printer_init(&printer);
		flush_start = iter;
		while (iter < end &&
		       (is_quoting || (*iter != ' ' && *iter != '\t'))) {
			char const *part_start = iter;
			num_slashes      = 0;
			while (*iter == '\\')
				++iter, ++num_slashes;
			if (*iter == '\"') {
				/* Special handling for escaped quotation marks. */
				/* Print one backslash for every second leading backslash. */
				part_start += num_slashes / 2;
				if (unicode_printer_print(&printer, flush_start, (size_t)(part_start - flush_start)) < 0)
					goto err_r_printer;

				/* Continue flushing after the quotation character. */
				flush_start = iter + 1;

				/* Special extension: When inside a quoted string, double-quotes are
				 * extended into a single quote. Implement this by moving the flush_start
				 * pointer back by one (so-as to print the `\"' that got us here). */
				if (is_quoting && iter[1] == '\"')
					--flush_start;
				is_quoting = !is_quoting;
			} else {
				/* Without a following quotation mark,
				 * backslashes do not act as escaping. */
			}
			++iter;
		}

		/* Flush the remainder of the argument. */
		if (unicode_printer_print(&printer, flush_start, (size_t)(iter - flush_start)) < 0)
			goto err_r_printer;

		/* Make space for the new argument in the to-be returned tuple. */
		if (result_size >= result_alloc) {
			size_t new_alloc = result_alloc * 2;
			DREF DeeTupleObject *new_result;
			ASSERT(new_alloc > result_alloc);
			new_result = DeeTuple_TryResizeUninitialized(result, new_alloc);
			if (!new_result) {
				new_alloc  = result_alloc + 1;
				new_result = DeeTuple_ResizeUninitialized(result, new_alloc);
				if unlikely(!new_result)
					goto err_r_printer;
			}
			result       = new_result;
			result_alloc = new_alloc;
		}

		/* Pack together the argument and append it. */
		arg = unicode_printer_pack(&printer);
		if unlikely(!arg)
			goto err_r;
		DeeTuple_SET(result, result_size, arg); /* Inherit reference */
		++result_size;
	}
	result = DeeTuple_TruncateUninitialized(result, result_size);
	return result;
err_r_printer:
	unicode_printer_fini(&printer);
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}
#endif /* ipc_Process_USE_CreateProcessW */



/* Process flags. */
#define PROCESS_FLAG_NORMAL     0x0000 /* Normal process flags. */
#define PROCESS_FLAG_STARTING   0x0001 /* [lock(SET(p_lock), CLEAR(CALLER_SET_BEFORE))] The process is currently starting. */
#define PROCESS_FLAG_STARTED    0x0002 /* [lock(WRITE_ONCE)] The process has been started. (Always set for external processes) */
#define PROCESS_FLAG_JOINING    0x0004 /* [lock(SET(p_lock), CLEAR(CALLER_SET_BEFORE))] The process is being joined */
#define PROCESS_FLAG_DETACHED   0x0008 /* [lock(WRITE_ONCE)] The process has been detached (`p_id' is dangling and `p_handle' is invalid). */
#define PROCESS_FLAG_TERMINATED 0x0010 /* [lock(WRITE_ONCE)] The process has terminated (implies `PROCESS_FLAG_DETACHED'). */
#define PROCESS_FLAG_EXTERN     0x4000 /* [lock(const)] The process isn't a child of the hosting deemon process (implies `PROCESS_FLAG_STARTING', `PROCESS_FLAG_STARTED', `PROCESS_FLAG_DETACHED') */
#define PROCESS_FLAG_SELF       0x8000 /* [lock(const)] This is the stand-in for the deemon-process itself (implies `PROCESS_FLAG_EXTERN'). */

typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t   p_lock;     /* Lock for accessing members of this structure. */
#endif /* !CONFIG_NO_THREADS */
	/* NOTE: `EXTERN_CONST_IF' here means that only the thread that set `PROCESS_FLAG_STARTING' may modify the field. */
	DREF DeeObject       *p_exe;      /* [lock(p_lock)][0..1][if(!PROCESS_FLAG_EXTERN,[1..1])][EXTERN_CONST_IF(PROCESS_FLAG_STARTING && !PROCESS_FLAG_EXTERN)]
	                                   * The main executable (either as a string, or an object carrying a file descriptor/handle). (lazily loaded when `PROCESS_FLAG_EXTERN' is set) */
#ifdef ipc_Process_USE_CreateProcessW
#define ipc_Process_USE_cmdline
	DREF DeeStringObject *p_cmdline;  /* [lock(p_lock)][0..1][if(!PROCESS_FLAG_EXTERN,[1..1])][EXTERN_CONST_IF(PROCESS_FLAG_STARTING && !PROCESS_FLAG_EXTERN)]
	                                   * The commandline of the process (VC/VC++ compatible). (lazily loaded when `PROCESS_FLAG_EXTERN') */
#else /* ipc_Process_USE_CreateProcessW */
#define ipc_Process_USE_argv
	DREF DeeObject       *p_argv;     /* [lock(p_lock)][0..1][if(!PROCESS_FLAG_EXTERN,[1..1])][EXTERN_CONST_IF(PROCESS_FLAG_STARTING && !PROCESS_FLAG_EXTERN)]
	                                   * Array of arguments passed to the program's main()-function. (lazily loaded when `PROCESS_FLAG_EXTERN') */
#endif /* !ipc_Process_USE_CreateProcessW */
	DREF DeeObject       *p_envp;     /* [lock(p_lock)][0..1][EXTERN_CONST_IF(PROCESS_FLAG_STARTING)] The environment block used by the process (or `NULL' to refer to that of the calling process).
	                                   * NOTE: When set, this is a sequence type compatible with `posix.environ': `{string: string}' (always NULL when `PROCESS_FLAG_EXTERN') */
	DREF DeeObject       *p_pwd;      /* [lock(p_lock)][0..1][EXTERN_CONST_IF(PROCESS_FLAG_STARTING)] The process working directory (or `NULL' to refer to that of the calling process). (always NULL when `PROCESS_FLAG_EXTERN') */
	DREF DeeObject       *p_stdfd[3]; /* [lock(p_lock)][0..1][EXTERN_CONST_IF(PROCESS_FLAG_STARTING)] Std file streams (index is one of DEE_STD*) (always NULL when `PROCESS_FLAG_EXTERN') */

#ifdef ipc_Process_pid_t
	ipc_Process_pid_t     p_pid;      /* [valid_if(p_pid != ipc_Process_pid_t_INVALID)]
	                                   * [lock(p_lock && WRITE_ONCE)] Process ID */
#endif /* ipc_Process_pid_t */
#ifdef ipc_Process_USE_CreateProcessW
	HANDLE                p_handle;   /* [valid_if(p_handle != INVALID_HANDLE_VALUE)]
	                                   * [lock(p_lock)][owned] Process handle. */
#endif /* ipc_Process_USE_CreateProcessW */
	uint16_t              p_state;    /* [lock(READ(ATOMIC), WRITE(p_lock))] The state of the process (Set of `PROCESS_FLAG_*') */
} Process;

#define Process_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->p_lock)
#define Process_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->p_lock)
#define Process_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->p_lock)
#define Process_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->p_lock)
#define Process_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->p_lock)
#define Process_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->p_lock)
#define Process_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->p_lock)
#define Process_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->p_lock)
#define Process_LockRead(self)       Dee_atomic_rwlock_read(&(self)->p_lock)
#define Process_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->p_lock)
#define Process_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->p_lock)
#define Process_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->p_lock)
#define Process_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->p_lock)
#define Process_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->p_lock)
#define Process_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->p_lock)
#define Process_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->p_lock)


PRIVATE Process this_process = {
	OBJECT_HEAD_INIT(&DeeProcess_Type),
#ifndef CONFIG_NO_THREADS
	/* .p_lock    = */ DEE_ATOMIC_RWLOCK_INIT,
#endif /* !CONFIG_NO_THREADS */
	/* .p_exe     = */ NULL,
#ifdef ipc_Process_USE_cmdline
	/* .p_cmdline = */ NULL,
#endif /* ipc_Process_USE_cmdline */
#ifdef ipc_Process_USE_argv
	/* .p_argv    = */ NULL,
#endif /* !ipc_Process_USE_argv */
	/* .p_envp    = */ NULL,
	/* .p_pwd     = */ NULL,
	/* .p_stdfd   = */ { NULL, NULL, NULL },
#ifdef ipc_Process_pid_t
	/* .p_pid     = */ ipc_Process_pid_t_INVALID,
#endif /* ipc_Process_pid_t */
#ifdef ipc_Process_USE_CreateProcessW
	/* .p_handle  = */ INVALID_HANDLE_VALUE,
#endif /* ipc_Process_USE_CreateProcessW */
	/* .p_state   = */ (PROCESS_FLAG_STARTING | PROCESS_FLAG_STARTED |
	                    PROCESS_FLAG_EXTERN | PROCESS_FLAG_SELF)
};


#ifdef CONFIG_IPC_HAVE_etc_shells

/* [0..1][lock(WRITE_ONCE)] The default /etc/shells shell. */
PRIVATE DREF DeeStringObject *ipc_etc_shells_default_shell = NULL;
#define HAVE_ipc_etc_shells_default_shell_fini
#define ipc_etc_shells_default_shell_fini()     \
	(ITER_ISOK(ipc_etc_shells_default_shell)    \
	 ? Dee_Decref(ipc_etc_shells_default_shell) \
	 : (void)0)

#ifndef _PATH_SHELLS
#define _PATH_SHELLS "/etc/shells"
#endif /* !_PATH_SHELLS */

#ifndef CONFIG_HAVE_strnlen
#define CONFIG_HAVE_strnlen
#undef strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */

#undef is_executable_file_USE_waccess
#undef is_executable_file_USE_access
#undef is_executable_file_USE_DeeFile_Open
#if defined(CONFIG_HAVE_waccess) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define is_executable_file_USE_waccess
#elif defined(CONFIG_HAVE_access)
#define is_executable_file_USE_access
#elif defined(CONFIG_HAVE_waccess)
#define is_executable_file_USE_waccess
#else /* CONFIG_HAVE_waccess */
#define is_executable_file_USE_DeeFile_Open
#endif /* !CONFIG_HAVE_waccess */

#ifdef X_OK
#define is_executable_file_USED_X_OK X_OK
#elif defined(F_OK)
#define is_executable_file_USED_X_OK F_OK
#else /* ... */
#define is_executable_file_USED_X_OK 0
#endif /* !... */

/* @return: 1 : Yes
 * @return: 0 : No
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
is_executable_file(DeeStringObject *__restrict filename) {
#ifdef is_executable_file_USE_waccess
	{
		int error;
		Dee_wchar_t const *filename_wide;
		filename_wide = DeeString_AsWide((DeeObject *)filename);
		if unlikely(!filename_wide)
			goto err;
#define NEED_err
		error = waccess((wchar_t *)filename_wide, is_executable_file_USED_X_OK);
		if (error == 0)
			return 1;
	}
#endif /* is_executable_file_USE_waccess */

#ifdef is_executable_file_USE_access
	{
		int error;
		char const *filename_utf8;
		filename_utf8 = DeeString_AsUtf8((DeeObject *)filename);
		if unlikely(!filename_utf8)
			goto err;
#define NEED_err
		error = access(filename_utf8, is_executable_file_USED_X_OK);
		if (error == 0)
			return 1;
	}
#endif /* is_executable_file_USE_access */

#ifdef is_executable_file_USE_DeeFile_Open
	{
		DREF DeeObject *fp;
		fp = DeeFile_Open((DeeObject *)filename, Dee_OPEN_FRDONLY, 0);
		if (ITER_ISOK(fp)) {
			Dee_Decref_likely(fp);
			return 1;
		}
		if unlikely(!fp)
			goto err;
#define NEED_err
	}
#endif /* is_executable_file_USE_DeeFile_Open */

	return 0;
#ifdef NEED_err
#undef NEED_err
err:
	return -1;
#endif /* NEED_err */
}

PRIVATE WUNUSED DREF DeeStringObject *DCALL
process_do_get_etc_shells_default_shell(void) {
	DREF DeeObject *fp, *line;
	fp = DeeFile_OpenString(_PATH_SHELLS, Dee_OPEN_FRDONLY, 0);
	if (!ITER_ISOK(fp))
		return (DREF DeeStringObject *)fp; /* File-not-found or error. */
	while (ITER_ISOK(line = DeeFile_ReadLine(fp, (size_t)-1, false))) {
		char const *line_data;
		size_t line_size;
		line_data = (char const *)DeeBytes_DATA(line);
		line_size = strnlen(line_data, DeeBytes_SIZE(line));

		/* Strip leading/trailing whitespace. */
		while (line_size && isspace(line_data[0])) {
			++line_data;
			--line_size;
		}
		while (line_size && isspace(line_data[line_size - 1]))
			--line_size;

		/* Ignore anything that isn't an absolute path (including line starting with '#') */
		if (DeeSystem_IsAbsN(line_data, line_size)) {
			DREF DeeStringObject *filename;
			int error;
			filename = (DREF DeeStringObject *)DeeString_NewUtf8(line_data, line_size, STRING_ERROR_FREPLAC);
			if unlikely(!filename) {
err_fp:
				line = NULL;
				break;
			}
			error = is_executable_file(filename);
			if (error > 0)
				return filename;
			Dee_Decref(filename);
			if (error < 0)
				goto err_fp;
		}
		Dee_Decref(line);
	}
	Dee_Decref(fp);
	return (DREF DeeStringObject *)ITER_DONE;
}


/* @return: * :        The default shell (according to `/etc/shells')
 * @return: ITER_DONE: `/etc/shells' does not exist, or does not specify any valid shells.
 * @return: NULL:      Error. */
PRIVATE WUNUSED DREF DeeStringObject *DCALL
process_get_etc_shells_default_shell(void) {
	DREF DeeStringObject *result;
again:
	result = atomic_read(&ipc_etc_shells_default_shell);
	if (result != NULL) {
		if (result != (DREF DeeStringObject *)ITER_DONE)
			Dee_Incref(result);
	} else {
		result = process_do_get_etc_shells_default_shell();
		if unlikely(!atomic_cmpxch(&ipc_etc_shells_default_shell, NULL, result)) {
			Dee_Decref_likely(result);
			goto again;
		}
	}
	return result;
}
#endif /* CONFIG_IPC_HAVE_etc_shells */

/* Get the absolute pathname of the shell to use in order to execute shell-commands. */
PRIVATE WUNUSED DREF DeeStringObject *DCALL process_get_shell(void) {
	DREF DeeStringObject *result;
	/* Allow overriding the system shell program with the "$SHELL" environment variable. */
	result = (DREF DeeStringObject *)Dee_GetEnv((DeeObject *)&str_SHELL);
	if (result == (DREF DeeStringObject *)ITER_DONE) {
		/* On unix, try to make use of `/etc/shells':
		 * >> function getDefaultShell(): string {
		 * >>     local result = posix.environ.get("SHELL");
		 * >>     if (result !is none)
		 * >>         return result;
		 * >>     local fp = File.open("/etc/shells", "r");
		 * >>     for (;;) {
		 * >>         local line = fp.readline(false);
		 * >>         if (line is none)
		 * >>             break;
		 * >>         line = line.strip();
		 * >>         if (posix.isabs(line)) {
		 * >>             if (posix.access(line, posix.X_OK))
		 * >>                 return line;
		 * >>         }
		 * >>     }
		 * >>     return "/bin/sh";
		 * >> } */
#ifdef CONFIG_IPC_HAVE_etc_shells
		result = process_get_etc_shells_default_shell();
		if (result == (DREF DeeStringObject *)ITER_DONE)
#endif /* CONFIG_IPC_HAVE_etc_shells */
		{
			result = (DREF DeeStringObject *)&str_DEFAULT_SHELL;
			Dee_Incref(result);
		}
	}
	return result;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
process_init(Process *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeObject *exe_or_cmdline_or_pid, *proc_argv = NULL;
	self->p_envp = NULL;
	DeeArg_Unpack1Or2Or3(err, argc, argv, "Process",
	                     &exe_or_cmdline_or_pid,
	                     &proc_argv, &self->p_envp);
	if (proc_argv) {
#ifdef ipc_Process_USE_CreateProcessW
		self->p_cmdline = ipc_argv2cmdline(proc_argv);
		if unlikely(!self->p_cmdline)
			goto err;
#else /* ipc_Process_USE_CreateProcessW */
		self->p_argv = proc_argv;
		Dee_Incref(proc_argv);
#endif /* !ipc_Process_USE_CreateProcessW */
		self->p_exe = exe_or_cmdline_or_pid;
		Dee_Incref(exe_or_cmdline_or_pid);
		Dee_XIncref(self->p_envp);
	} else if (DeeString_Check(exe_or_cmdline_or_pid)) {
		/* Use the system shell as executable. */
		self->p_exe = (DREF DeeObject *)process_get_shell();
		if unlikely(!self->p_exe)
			goto err;

		/* Now pack the cmdline/argv-tuple */
#ifdef ipc_Process_USE_CreateProcessW
		self->p_cmdline = ipc_argv2cmdline_cmd_c((DeeStringObject *)self->p_exe,
		                                         (DeeStringObject *)exe_or_cmdline_or_pid);
		if unlikely(!self->p_cmdline)
#else /* ipc_Process_USE_CreateProcessW */
		self->p_argv = DeeTuple_Pack(3, self->p_exe, &str_DEFAULT_SHELL_C, exe_or_cmdline_or_pid);
		if unlikely(!self->p_argv)
#endif /* !ipc_Process_USE_CreateProcessW */
		{
			Dee_Decref(self->p_exe);
			goto err;
		}
	} else {
		/* Caller wants to open a process, given its PID. */
#ifdef ipc_Process_pid_t
		if unlikely(DeeObject_AsUIntX(exe_or_cmdline_or_pid, &self->p_pid))
			goto err;
		Dee_atomic_rwlock_init(&self->p_lock);
		self->p_exe = NULL;
#ifdef ipc_Process_USE_cmdline
		self->p_cmdline = NULL;
#endif /* ipc_Process_USE_cmdline */
#ifdef ipc_Process_USE_argv
		self->p_argv = NULL;
#endif /* ipc_Process_USE_argv */
		self->p_envp = NULL;
		self->p_pwd = NULL;
		bzero(self->p_stdfd, sizeof(self->p_stdfd));
		self->p_state = PROCESS_FLAG_NORMAL;
#ifdef ipc_Process_USE_CreateProcessW
		self->p_handle = INVALID_HANDLE_VALUE;
#endif /* ipc_Process_USE_CreateProcessW */
		self->p_state = PROCESS_FLAG_STARTING | PROCESS_FLAG_STARTED |
		                PROCESS_FLAG_DETACHED | PROCESS_FLAG_EXTERN;
		return 0;
#else /* ipc_Process_pid_t */
		unsigned int ignored;
		if unlikely(DeeObject_AsUInt(exe_or_cmdline_or_pid, &ignored))
			goto err;
		ipc_unimplemented();
		goto err;
#endif /* ipc_Process_pid_t */
	}

	/* Default attributes that can't be specified as constructor arguments. */
	Dee_atomic_rwlock_init(&self->p_lock);
	self->p_pwd = NULL;
	bzero(self->p_stdfd, sizeof(self->p_stdfd));
	self->p_state = PROCESS_FLAG_NORMAL;
#ifdef ipc_Process_USE_CreateProcessW
	self->p_handle = INVALID_HANDLE_VALUE;
#endif /* ipc_Process_USE_CreateProcessW */
#ifdef ipc_Process_pid_t
	self->p_pid = ipc_Process_pid_t_INVALID;
#endif /* ipc_Process_pid_t */
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
process_fini(Process *__restrict self) {
#ifdef ipc_Process_USE_CreateProcessW
	if (self->p_handle != INVALID_HANDLE_VALUE) {
		DBG_ALIGNMENT_DISABLE();
		CloseHandle(self->p_handle);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* ipc_Process_USE_CreateProcessW */

#if defined(ipc_Process_USE_pid_t) && defined(CONFIG_HAVE_detach)
	if ((self->p_state & (PROCESS_FLAG_STARTED | PROCESS_FLAG_DETACHED)) == PROCESS_FLAG_STARTED) {
		DBG_ALIGNMENT_DISABLE();
		detach(self->p_pid);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* ipc_Process_USE_pid_t && CONFIG_HAVE_detach */

	Dee_XDecrefv(self->p_stdfd, COMPILER_LENOF(self->p_stdfd));
	Dee_XDecref(self->p_pwd);
	Dee_XDecref(self->p_envp);
#ifdef ipc_Process_USE_cmdline
	Dee_XDecref(self->p_cmdline);
#endif /* ipc_Process_USE_cmdline */
#ifdef ipc_Process_USE_argv
	Dee_XDecref(self->p_argv);
#endif /* ipc_Process_USE_argv */
	Dee_XDecref(self->p_exe);
}

PRIVATE NONNULL((1, 2)) void DCALL
process_visit(Process *__restrict self, Dee_visit_t proc, void *arg) {
	Process_LockRead(self);
	Dee_XVisitv(self->p_stdfd, COMPILER_LENOF(self->p_stdfd));
	Dee_XVisit(self->p_envp);
#ifdef ipc_Process_USE_argv
	Dee_XVisit(self->p_argv);
#endif /* ipc_Process_USE_argv */
	Process_LockEndRead(self);
}

PRIVATE NONNULL((1)) void DCALL
process_clear(Process *__restrict self) {
	DREF DeeObject *objs[COMPILER_LENOF(self->p_stdfd) + 1
#ifdef ipc_Process_USE_argv
	                    + 1
#endif /* ipc_Process_USE_argv */
	];
	Process_LockWrite(self);
	memcpyc(objs, self->p_stdfd, COMPILER_LENOF(self->p_stdfd), sizeof(DREF DeeObject *));
	objs[COMPILER_LENOF(self->p_stdfd) + 0] = self->p_envp;
#ifdef ipc_Process_USE_argv
	objs[COMPILER_LENOF(self->p_stdfd) + 1] = self->p_argv;
#endif /* ipc_Process_USE_argv */
	bzeroc(self->p_stdfd, COMPILER_LENOF(self->p_stdfd), sizeof(DREF DeeObject *));
	self->p_envp = NULL;
#ifdef ipc_Process_USE_argv
	self->p_argv = NULL;
#endif /* ipc_Process_USE_argv */
	Process_LockEndWrite(self);
	Dee_XDecrefv(objs, COMPILER_LENOF(objs));
}

PRIVATE struct type_gc tpconst process_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&process_clear
};


#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
process_print(Process *__restrict self,
              Dee_formatprinter_t printer, void *arg) {
	DREF DeeObject *proc_exe;
#ifdef ipc_Process_USE_cmdline
	DREF DeeStringObject *proc_cmdline;
#endif /* ipc_Process_USE_cmdline */
#ifdef ipc_Process_USE_argv
	DREF DeeObject *proc_argv;
#endif /* ipc_Process_USE_argv */
#ifdef ipc_Process_pid_t
	ipc_Process_pid_t proc_pid;
#endif /* ipc_Process_pid_t */
	uint16_t proc_state;
	Dee_ssize_t temp, result = 0;

	/* Load process properties. */
	Process_LockRead(self);
	proc_exe = self->p_exe;
	Dee_XIncref(proc_exe);
#ifdef ipc_Process_USE_cmdline
	proc_cmdline = self->p_cmdline;
	Dee_XIncref(proc_cmdline);
#endif /* ipc_Process_USE_cmdline */
#ifdef ipc_Process_USE_argv
	proc_argv = self->p_argv;
	Dee_XIncref(proc_argv);
#endif /* ipc_Process_USE_argv */
#ifdef ipc_Process_pid_t
	proc_pid = self->p_pid;
#endif /* ipc_Process_pid_t */
	proc_state = self->p_state;
	Process_LockEndRead(self);

	/* Print a human-readable representation of the process. */
	DO(err, DeeFormat_PRINT(printer, arg, "<Process "));
	if (proc_state & PROCESS_FLAG_SELF) {
		DO(err, DeeFormat_PRINT(printer, arg, "<this process>"));
	} else {
		char const *status;
		bool has_name = false;
#ifdef ipc_Process_USE_cmdline
		if (proc_cmdline) {
			has_name = true;
			DO(err, DeeFormat_PrintObjectRepr(printer, arg, (DeeObject *)proc_cmdline));
		} else
#endif /* ipc_Process_USE_cmdline */
#ifdef ipc_Process_USE_argv
		if (proc_argv) {
			has_name = true;
			DO(err, DeeFormat_PrintObjectRepr(printer, arg, (DeeObject *)proc_argv));
		} else
#endif /* ipc_Process_USE_argv */
		if (proc_exe) {
			has_name = true;
			DO(err, DeeFormat_PrintObjectRepr(printer, arg, (DeeObject *)proc_exe));
		}

#ifdef ipc_Process_pid_t
		if (proc_pid != ipc_Process_pid_t_INVALID) {
			if (has_name)
				DO(err, DeeFormat_PRINT(printer, arg, ", "));
			DO(err, DeeFormat_Printf(printer, arg, "pid: %" PRFdPID, proc_pid));
			has_name = true;
		}
#endif /* ipc_Process_pid_t */

		status = NULL;
		if (proc_state & PROCESS_FLAG_TERMINATED) {
			status = "(terminated)";
		} else if (proc_state & PROCESS_FLAG_EXTERN) {
			status = "(extern)";
		} else if (proc_state & PROCESS_FLAG_DETACHED) {
			status = "(detached)";
		} else if (proc_state & PROCESS_FLAG_STARTED) {
			status = "(started)";
		}
		if (status != NULL) {
			if (has_name)
				DO(err, DeeFormat_PRINT(printer, arg, " "));
			DO(err, DeeFormat_PrintStr(printer, arg, status));
		}
	}
	DO(err, DeeFormat_PRINT(printer, arg, ">"));
done:
	Dee_XDecref(proc_exe);
#ifdef ipc_Process_USE_cmdline
	Dee_XDecref(proc_cmdline);
#endif /* ipc_Process_USE_cmdline */
#ifdef ipc_Process_USE_argv
	Dee_XDecref(proc_argv);
#endif /* ipc_Process_USE_argv */
	return result;
err:
	result = temp;
	goto done;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
process_printrepr(Process *__restrict self,
                  Dee_formatprinter_t printer, void *arg) {
	DREF DeeObject *proc_exe;
	DREF DeeObject *proc_argv;
	DREF DeeObject *proc_environ;
	DREF DeeObject *proc_pwd;
	DREF DeeObject *proc_stdfd[COMPILER_LENOF(self->p_stdfd)];
#ifdef ipc_Process_USE_cmdline
	DREF DeeStringObject *proc_cmdline;
#endif /* ipc_Process_USE_cmdline */
	uint16_t proc_state;
	Dee_ssize_t temp, result = 0;

	/* Load process properties. */
	Process_LockRead(self);

	/* Check for special case: external process reference */
#ifdef ipc_Process_pid_t
	if (self->p_state & PROCESS_FLAG_EXTERN) {
		ipc_Process_pid_t pid = self->p_pid;
		Process_LockEndRead(self);
		return DeeFormat_Printf(printer, arg, "Process(pid: " PRFdPID ")", pid);
	}
#endif /* ipc_Process_pid_t */

	/* Check for special case: current process reference */
	if (self->p_state & PROCESS_FLAG_SELF) {
		Process_LockEndRead(self);
		return DeeFormat_PRINT(printer, arg, "Process.current");
	}

	proc_exe = self->p_exe;
	Dee_XIncref(proc_exe);
#ifdef ipc_Process_USE_cmdline
	proc_cmdline = self->p_cmdline;
	Dee_XIncref(proc_cmdline);
#elif defined(ipc_Process_USE_argv)
	proc_argv = self->p_argv;
	Dee_XIncref(proc_argv);
#endif /* ipc_Process_USE_cmdline */
	proc_environ = self->p_envp;
	Dee_XIncref(proc_environ);
	proc_pwd = self->p_pwd;
	Dee_XIncref(proc_pwd);
	Dee_XMovrefv(proc_stdfd, self->p_stdfd, COMPILER_LENOF(proc_stdfd));
	proc_state = self->p_state;
	Process_LockEndRead(self);

#ifdef ipc_Process_USE_cmdline
	proc_argv = NULL;
	if (proc_cmdline) {
		proc_argv = (DREF DeeObject *)ipc_cmdline2argv(proc_cmdline);
		Dee_Decref(proc_cmdline);
		if unlikely(!proc_argv) {
			temp = -1;
			goto err;
		}
	}
#endif /* ipc_Process_USE_cmdline */

	/* Print a human-readable representation of the process. */
	/* TODO: Add keyword argument support to the real constructor! */
	DO(err, DeeFormat_PRINT(printer, arg, "Process(exe: "));
	if likely(proc_exe != NULL) {
		DO(err, DeeFormat_PrintObjectRepr(printer, arg, proc_exe));
	} else {
		DO(err, DeeFormat_PRINT(printer, arg, "?"));
	}
	DO(err, DeeFormat_PRINT(printer, arg, ", argv: "));
	if likely(proc_argv != NULL) {
		DO(err, DeeFormat_PrintObjectRepr(printer, arg, proc_argv));
	} else {
		DO(err, DeeFormat_PRINT(printer, arg, "?"));
	}
	if (proc_environ)
		DO(err, DeeFormat_Printf(printer, arg, ", environ: %r", proc_environ));
	/* TODO: allow direct specification of all of the following arguments within the constructor */
	if (proc_pwd)
		DO(err, DeeFormat_Printf(printer, arg, ", pwd: %r", proc_pwd));
	if (proc_stdfd[DEE_STDIN])
		DO(err, DeeFormat_Printf(printer, arg, ", stdin: %r", proc_stdfd[DEE_STDIN]));
	if (proc_stdfd[DEE_STDOUT])
		DO(err, DeeFormat_Printf(printer, arg, ", stdout: %r", proc_stdfd[DEE_STDOUT]));
	if (proc_stdfd[DEE_STDERR])
		DO(err, DeeFormat_Printf(printer, arg, ", stderr: %r", proc_stdfd[DEE_STDERR]));
	DO(err, DeeFormat_PRINT(printer, arg, ")"));

	/* Still (somewhat) include the process's status in its representation */
	if (proc_state & PROCESS_FLAG_TERMINATED) {
		DO(err, DeeFormat_PRINT(printer, arg, "<.hasterminated>"));
	} else if (proc_state & PROCESS_FLAG_DETACHED) {
		DO(err, DeeFormat_PRINT(printer, arg, "<.wasdetached>"));
	} else if (proc_state & PROCESS_FLAG_STARTED) {
		DO(err, DeeFormat_PRINT(printer, arg, "<.hasstarted>"));
	}

done:
	Dee_XDecrefv(proc_stdfd, COMPILER_LENOF(proc_stdfd));
	Dee_XDecref(proc_pwd);
	Dee_XDecref(proc_environ);
	Dee_XDecref(proc_argv);
	Dee_XDecref(proc_exe);
	return result;
err:
	result = temp;
	goto done;
}

#undef DO



#ifdef NEED_ipc_joinpid_impl
struct reaped_childproc;
SLIST_HEAD(reaped_childproc_slist, reaped_childproc);
struct reaped_childproc {
	SLIST_ENTRY(reaped_childproc) rc_link; /* [0..1][lock(reaped_childprocs_lock)] Link in list of reaped processes. */
	pid_t                         rc_cpid; /* [const] Reaped child process ID. */
	int                           rc_stat; /* [const] Reaped child process exit status. */
};

/* [0..n][lock(reaped_childprocs_lock)] List of reaped child processes */
PRIVATE struct reaped_childproc_slist reaped_childprocs = SLIST_HEAD_INITIALIZER(reaped_childprocs);
PRIVATE Dee_shared_rwlock_t reaped_childprocs_lock      = DEE_SHARED_RWLOCK_INIT;
#define reaped_childprocs_lock_reading()    Dee_shared_rwlock_reading(&reaped_childprocs_lock)
#define reaped_childprocs_lock_writing()    Dee_shared_rwlock_writing(&reaped_childprocs_lock)
#define reaped_childprocs_lock_tryread()    Dee_shared_rwlock_tryread(&reaped_childprocs_lock)
#define reaped_childprocs_lock_trywrite()   Dee_shared_rwlock_trywrite(&reaped_childprocs_lock)
#define reaped_childprocs_lock_canread()    Dee_shared_rwlock_canread(&reaped_childprocs_lock)
#define reaped_childprocs_lock_canwrite()   Dee_shared_rwlock_canwrite(&reaped_childprocs_lock)
#define reaped_childprocs_lock_waitread()   Dee_shared_rwlock_waitread(&reaped_childprocs_lock)
#define reaped_childprocs_lock_waitwrite()  Dee_shared_rwlock_waitwrite(&reaped_childprocs_lock)
#define reaped_childprocs_lock_read()       Dee_shared_rwlock_read(&reaped_childprocs_lock)
#define reaped_childprocs_lock_write()      Dee_shared_rwlock_write(&reaped_childprocs_lock)
#define reaped_childprocs_lock_tryupgrade() Dee_shared_rwlock_tryupgrade(&reaped_childprocs_lock)
#define reaped_childprocs_lock_upgrade()    Dee_shared_rwlock_upgrade(&reaped_childprocs_lock)
#define reaped_childprocs_lock_downgrade()  Dee_shared_rwlock_downgrade(&reaped_childprocs_lock)
#define reaped_childprocs_lock_endwrite()   Dee_shared_rwlock_endwrite(&reaped_childprocs_lock)
#define reaped_childprocs_lock_endread()    Dee_shared_rwlock_endread(&reaped_childprocs_lock)
#define reaped_childprocs_lock_end()        Dee_shared_rwlock_end(&reaped_childprocs_lock)


#define HAVE_ipc_reaped_childprocs_clear
PRIVATE void DCALL
ipc_reaped_childprocs_clear(void) {
	while (!SLIST_EMPTY(&reaped_childprocs)) {
		struct reaped_childproc *ent;
		ent = SLIST_FIRST(&reaped_childprocs);
		SLIST_REMOVE_HEAD(&reaped_childprocs, rc_link);
		Dee_Free(ent);
	}
	return -1;
}

PRIVATE NONNULL((2)) int DCALL
ipc_tryjoinpid_locked(pid_t pid, int *p_status) {
	struct reaped_childproc *ent, **p_ent;
	SLIST_FOREACH_PREVPTR (ent, p_ent, &reaped_childprocs, rc_link) {
		if (ent->rc_cpid == pid) {
			SLIST_REMOVE_PREVPTR(p_ent, ent, rc_link);
			*p_status = ent->rc_stat;
			Dee_Free(ent);
			return 0;
		}
	}
	return -1;
}

PRIVATE NONNULL((2)) int DCALL
ipc_joinpid_impl(pid_t pid, int *p_status) {
	int result;
	struct reaped_childproc *ent;
again:
	if (reaped_childprocs_lock_read())
		goto err_interrupt;
	result = ipc_tryjoinpid_locked(pid, p_status);
	if (result == 0) {
		reaped_childprocs_lock_endread();
		return result;
	}
	reaped_childprocs_lock_endread();

	ent = (struct reaped_childproc *)Dee_TryMalloc(sizeof(struct reaped_childproc));
	if unlikely(!ent) {
#ifdef ENOMEM
		DeeSystem_SetErrno(ENOMEM);
#endif /* ENOMEM */
		return -1;
	}

	/* Use a write-lock to ensure that only 1 thread is
	 * ever waiting for child-processes at the same time. */
	if (reaped_childprocs_lock_write())
		goto err_interrupt;
	result = ipc_tryjoinpid_locked(pid, p_status);
	if (result == 0) {
		reaped_childprocs_lock_endwrite();
		Dee_Free(ent);
		return result;
	}

	ent->rc_cpid = ipc_joinany(&ent->rc_stat);
	if unlikely(ent->rc_cpid < 0) {
		int error = ent->rc_cpid;
		reaped_childprocs_lock_endwrite();
		Dee_Free(ent);
		return error;
	}

	/* Load the reaped child process descriptor
	 * into the list of reaped processes. */
	SLIST_INSERT(&reaped_childprocs, ent, rc_link);
	reaped_childprocs_lock_endwrite();
	goto again;
err_interrupt:
#ifdef EINTR
	DeeSystem_SetErrno(EINTR);
#endif /* EINTR */
	DeeError_Handled(ERROR_HANDLED_RESTORE);
	return -1;
}
#endif /* NEED_ipc_joinpid_impl */




#ifdef ipc_Process_WANT_ipc_exe2path_cache
/* Cache for mapping from executable names to absolute paths.
 * This cache is cleared when $PATH or $PATHEXT is changed.
 * 
 * The first time that this cache is used, it is hooked into the
 * notify system via `DeeNotify_StartListen()', and once the ipc
 * dex is unloaded, that hook is deleted by `DeeNotify_EndListen()' */
PRIVATE DeeDictObject ipc_exe2path_cache = Dee_DICT_INIT;
PRIVATE int ipc_exe2path_cache_listening = 0;

PRIVATE NONNULL((1, 2)) void DCALL
ipc_exe2path_remember(DeeStringObject *__restrict exe_str,
                      DeeStringObject *__restrict full_exe_str) {
	if (atomic_read(&ipc_exe2path_cache_listening) > 0) {
		int error;
		error = DeeDict_SetItem((DeeObject *)&ipc_exe2path_cache,
		                        (DeeObject *)exe_str,
		                        (DeeObject *)full_exe_str);
		if unlikely(error != 0)
			DeeError_Handled(ERROR_HANDLED_RESTORE);
	}
}

PRIVATE int DCALL
ipc_exe2path_notify(DeeObject *UNUSED(arg)) {
	DeeDict_Clear((DeeObject *)&ipc_exe2path_cache);
	return 0;
}


/* (Try to) start listening for changes to $PATH and $PATHEXT */
PRIVATE bool DCALL
ipc_exe2path_do_start_listen(void) {
	int ok;
	ok = DeeNotify_StartListen(Dee_NOTIFICATION_CLASS_ENVIRON,
	                           (DeeObject *)&str_PATH,
	                           &ipc_exe2path_notify, NULL);
#ifdef ipc_Process_USE_CreateProcessW
	if likely(ok >= 0) {
		ok = DeeNotify_StartListen(Dee_NOTIFICATION_CLASS_ENVIRON,
		                           (DeeObject *)&str_PATHEXT,
		                           &ipc_exe2path_notify, NULL);
		if likely(ok >= 0) {
			/* On windows, also need to be clear the cache when the program's PWD changes! */
			ok = DeeNotify_StartListenClass(Dee_NOTIFICATION_CLASS_PWD,
			                                &ipc_exe2path_notify, NULL);
			if unlikely(ok < 0) {
				DeeNotify_EndListen(Dee_NOTIFICATION_CLASS_ENVIRON,
				                    (DeeObject *)&str_PATHEXT,
				                    &ipc_exe2path_notify, NULL);
			}
		}
		if unlikely(ok < 0) {
			DeeNotify_EndListen(Dee_NOTIFICATION_CLASS_ENVIRON,
			                    (DeeObject *)&str_PATH,
			                    &ipc_exe2path_notify, NULL);
		}
	}
#endif /* ipc_Process_USE_CreateProcessW */
	if unlikely(ok < 0) {
		DeeError_Handled(ERROR_HANDLED_RESTORE);
		return false;
	}
	return true;
}

/* (Try to) start listening for changes to $PATH and $PATHEXT */
PRIVATE void DCALL
ipc_exe2path_start_listen(void) {
	if (atomic_read(&ipc_exe2path_cache_listening) > 0)
		return;
	while (!atomic_cmpxch_weak(&ipc_exe2path_cache_listening, 0, -1)) {
		if (atomic_read(&ipc_exe2path_cache_listening) > 0)
			return;
		SCHED_YIELD();
	}
	if (ipc_exe2path_do_start_listen()) {
		atomic_write(&ipc_exe2path_cache_listening, 1);
	} else {
		atomic_write(&ipc_exe2path_cache_listening, 0);
	}
}


#define HAVE_ipc_exe2path_fini
PRIVATE void DCALL ipc_exe2path_fini(void) {
	(*DeeDict_Type.tp_init.tp_dtor)((DeeObject *)&ipc_exe2path_cache);
	if (atomic_read(&ipc_exe2path_cache_listening) > 0) {
		DeeNotify_EndListen(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)&str_PATH, &ipc_exe2path_notify, NULL);
#ifdef ipc_Process_USE_CreateProcessW
		DeeNotify_EndListen(Dee_NOTIFICATION_CLASS_ENVIRON, (DeeObject *)&str_PATHEXT, &ipc_exe2path_notify, NULL);
		DeeNotify_EndListenClass(Dee_NOTIFICATION_CLASS_PWD, &ipc_exe2path_notify, NULL);
#endif /* ipc_Process_USE_CreateProcessW */
	}
}

#ifdef ipc_Process_USE_UNIX_APIS
#if (defined(ipc_exe2path_USE_access) || \
     defined(ipc_exe2path_USE_waccess))
#ifdef ipc_exe2path_USE_waccess
#ifndef CONFIG_HAVE_wmemchr
#define CONFIG_HAVE_wmemchr
#undef wmemchr
#define wmemchr dee_wmemchr
DeeSystem_DEFINE_wmemchr(dee_wmemchr)
#endif /* !CONFIG_HAVE_wmemchr */
#define ipc_exe2path_char_t                                    wchar_t
#define ipc_exe2path_char_memchr                               wmemchr
#define access_ipc_exe2path_char_t                             waccess
#define DeeString_As_ipc_exe2path_char_t                       DeeString_AsWide
#define DeeString_NewBuffer_ipc_exe2path_char_t(len)           DeeString_NewWideBuffer(len)
#define DeeString_ResizeBuffer_ipc_exe2path_char_t(buf, len)   DeeString_ResizeWideBuffer(buf, len)
#define DeeString_FreeBuffer_ipc_exe2path_char_t(buf)          DeeString_FreeWideBuffer(buf)
#define DeeString_TruncateBuffer_ipc_exe2path_char_t(buf, len) DeeString_TruncateWideBuffer(buf, len)
#define DeeString_PackBuffer_ipc_exe2path_char_t(buf, emo)     DeeString_PackWideBuffer(buf, emo)
#else /* ipc_exe2path_USE_waccess */
#define ipc_exe2path_char_t                                    char
#define ipc_exe2path_char_memchr                               memchr
#define access_ipc_exe2path_char_t                             access
#define DeeString_As_ipc_exe2path_char_t                       DeeString_AsUtf8
#define DeeString_NewBuffer_ipc_exe2path_char_t(len)           (char *)DeeString_New1ByteBuffer(len)
#define DeeString_ResizeBuffer_ipc_exe2path_char_t(buf, len)   (char *)DeeString_Resize1ByteBuffer((uint8_t *)(buf), len)
#define DeeString_FreeBuffer_ipc_exe2path_char_t(buf)          DeeString_Free1ByteBuffer((uint8_t *)(buf))
#define DeeString_TruncateBuffer_ipc_exe2path_char_t(buf, len) (char *)DeeString_Truncate1ByteBuffer((uint8_t *)(buf), len)
#define DeeString_PackBuffer_ipc_exe2path_char_t(buf, emo)     DeeString_Pack1ByteBuffer((uint8_t *)(buf))
#endif /* !ipc_exe2path_USE_waccess */

/* Return the absolute (fully resolved) version of `exe' */
PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeStringObject *DCALL
ipc_exe2path_uncached_in_path(ipc_exe2path_char_t const *exe, size_t exe_len,
                              ipc_exe2path_char_t const *path) {
	ipc_exe2path_char_t const *iter = path, *next;
	ipc_exe2path_char_t *buffer, *new_buffer, *bufiter;
	buffer = DeeString_NewBuffer_ipc_exe2path_char_t(128 + exe_len);
	if unlikely(!buffer)
		goto err;
	for (;;) {
		int error;
		size_t pathlen;
		next = iter;
		while (*next && *next != (ipc_exe2path_char_t)DeeSystem_DELIM)
			++next;
		pathlen = (size_t)(next - iter);
		if (!DeeSystem_IsSep(next[-1]))
			++pathlen;
		pathlen += exe_len;
		if (pathlen > WSTR_LENGTH(buffer)) {
			/* Resize the encoding buffer. */
			new_buffer = DeeString_ResizeBuffer_ipc_exe2path_char_t(buffer, pathlen);
			if unlikely(!new_buffer)
				goto err_buffer;
			buffer = new_buffer;
		}
		bufiter = buffer;

		/* Create the path for the filename. */
		bufiter = (ipc_exe2path_char_t *)mempcpyc(bufiter, iter, (size_t)(next - iter),
		                                          sizeof(ipc_exe2path_char_t));
		if (!DeeSystem_IsSep(next[-1]))
			*bufiter++ = (ipc_exe2path_char_t)DeeSystem_SEP;
		bufiter  = (ipc_exe2path_char_t *)mempcpyc(bufiter, exe, exe_len, sizeof(ipc_exe2path_char_t));
		*bufiter = (ipc_exe2path_char_t)'\0'; /* Ensure zero-termination */
		ASSERT(bufiter <= buffer + WSTR_LENGTH(buffer));

		/* Check the constructed path. */
		DBG_ALIGNMENT_DISABLE();
		error = access_ipc_exe2path_char_t(buffer, X_OK);
		DBG_ALIGNMENT_ENABLE();
		if (error == 0) {
			/* Constructed path can be executed. */
			DREF DeeStringObject *result;
			buffer = DeeString_TruncateBuffer_ipc_exe2path_char_t(buffer, pathlen);
			result = (DREF DeeStringObject *)DeeString_PackBuffer_ipc_exe2path_char_t(buffer, STRING_ERROR_FREPLAC);
			return result;
		}

		while (*next && *next == DeeSystem_DELIM)
			++next;
		if (!*next)
			break;
		iter = next;
	}
	DeeString_FreeBuffer_ipc_exe2path_char_t(buffer);
	return (DREF DeeStringObject *)ITER_DONE;
err_buffer:
	DeeString_FreeBuffer_ipc_exe2path_char_t(buffer);
err:
	return NULL;
}

/* Return the absolute (fully resolved) version of `exe'
 * @return: * :   Fully resolved program path
 * @return: NULL: Error
 * @return: ITER_DONE: No such environment variable `$PATH' */
PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
ipc_exe2path_uncached(ipc_exe2path_char_t const *__restrict exe) {
	DREF DeeStringObject *result;
	DREF DeeStringObject *path;
	ipc_exe2path_char_t *path_str;
	Dee_DPRINTF("ipc_exe2path_uncached: lookup %q\n", exe);
	path = (DREF DeeStringObject *)Dee_GetEnv((DeeObject *)&str_PATH);
	if unlikely(!ITER_ISOK(path)) {
		if (!path)
			goto err;
		return (DREF DeeStringObject *)ITER_DONE;
	}
	path_str = (ipc_exe2path_char_t *)DeeString_As_ipc_exe2path_char_t((DeeObject *)path);
	if unlikely(!path_str)
		goto err_path;
	result = ipc_exe2path_uncached_in_path(exe, WSTR_LENGTH(exe), path_str);
	Dee_Decref(path);
	return result;
err_path:
	Dee_Decref(path);
err:
	return NULL;
}

/* Return the absolute (fully resolved) version of `exe' */
PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
ipc_exe2path(DeeStringObject *__restrict exe_strob) {
	DREF DeeStringObject *result;
	ipc_exe2path_char_t *exe;
	exe = (ipc_exe2path_char_t *)DeeString_As_ipc_exe2path_char_t((DeeObject *)exe_strob);
	if unlikely(!exe)
		goto err;
	if (ipc_exe2path_char_memchr(exe, (ipc_exe2path_char_t)DeeSystem_SEP, WSTR_LENGTH(exe))
#ifdef DeeSystem_ALTSEP
	    || ipc_exe2path_char_memchr(exe, (ipc_exe2path_char_t)DeeSystem_ALTSEP, WSTR_LENGTH(exe))
#endif /* DeeSystem_ALTSEP */
	    ) {
		return_reference_(exe_strob); /* Path is already absolute! */
	}

	/* Try to lookup the given `exe_strob' within the exe2path cache. */
	ipc_exe2path_start_listen();
	result = (DREF DeeStringObject *)DeeDict_TryGetItem((DeeObject *)&ipc_exe2path_cache,
	                                                    (DeeObject *)exe_strob);
	if (result != (DREF DeeStringObject *)ITER_DONE)
		return result;

	/* Try to resolve the path name using $PATH */
	result = ipc_exe2path_uncached(exe);

	/* Upon success, try to remember the cached pathname. */
	if (result != NULL) {
		if (result == (DREF DeeStringObject *)ITER_DONE) {
			/* No environment variable $PATH has been defined (no choice but to re-return `exe_strob') */
			result = exe_strob;
			Dee_Incref(result);
		} else {
			ipc_exe2path_remember(exe_strob, result);
		}
	}
	return result;
err:
	return NULL;
}
#endif /* ... */
#endif /* ipc_Process_USE_UNIX_APIS */
#endif /* ipc_Process_WANT_ipc_exe2path_cache */




#ifdef ipc_Process_USE_CreateProcessW
PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
ipc_nt_print_environ_item(void *arg, DeeObject *key, DeeObject *value) {
	struct unicode_printer *printer = (struct unicode_printer *)arg;
	if unlikely(DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	if unlikely(DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto err;
	if unlikely(unicode_printer_printstring(printer, key) < 0)
		goto err;
	if unlikely(unicode_printer_putascii(printer, '='))
		goto err;
	if unlikely(unicode_printer_printstring(printer, value) < 0)
		goto err;
	if unlikely(unicode_printer_putascii(printer, '\0'))
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ipc_nt_print_environ(struct unicode_printer *__restrict printer,
                     DeeObject *__restrict envp) {
	int result;
	result = (int)DeeObject_ForeachPair(envp, &ipc_nt_print_environ_item, printer);
	if likely(result == 0)
		result = unicode_printer_putascii(printer, '\0');
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
ipc_nt_pack_environ(DeeObject *__restrict envp) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(ipc_nt_print_environ(&printer, envp))
		goto err;
	return (DREF DeeStringObject *)unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3, 4)) int DCALL
ipc_nt_get_handle_for_CreateProcessW(Process *__restrict self, unsigned int std_handle_id,
                                     /*out[0..1]*/ DREF DeeObject **__restrict p_keep_alive_reference,
                                     PHANDLE pHandle) {
	int result;
	DeeObject *file = self->p_stdfd[std_handle_id];
	if (file != NULL) {
		*p_keep_alive_reference = NULL;
	} else {
		file = DeeFile_GetStd(std_handle_id);
		if unlikely(!file)
			return -1;
		*p_keep_alive_reference = file; /* Let the caller inherit the reference. */
	}
	/* Extract the referenced handle from the file. */
	result = DeeNTSystem_TryGetHandle(file, pHandle);
	if unlikely(result != 0) {
		Dee_XClear(*p_keep_alive_reference);
	} else {
		/* Have to ensure that the handle is inheritable. */
		if (*pHandle != INVALID_HANDLE_VALUE) {
			DBG_ALIGNMENT_DISABLE();
			SetHandleInformation(*pHandle,
			                     HANDLE_FLAG_INHERIT,
			                     HANDLE_FLAG_INHERIT);
			DBG_ALIGNMENT_ENABLE();
		}
	}
	return result;
}

#ifndef CONFIG_HAVE_wmemchr
#define CONFIG_HAVE_wmemchr
#undef wmemchr
#define wmemchr dee_wmemchr
DeeSystem_DEFINE_wmemchr(dee_wmemchr)
#endif /* !CONFIG_HAVE_wmemchr */



/* Try to create a process for exe f"{lpPathVariable}/{lpApplicationName...+=szApplicationNameLength}"
 * @param: bFixUnc:    When true, apply UNC path fixes.
 * @return: * :        The application name that was eventually used to start the process.
 * @return: NULL:      An error occurred and was thrown.
 * @return: ITER_DONE: Failed to start the process (see GetLastError()) */
PRIVATE WUNUSED NONNULL((1, 3, 5)) DREF DeeStringObject *DCALL
nt_CreateProcessPathNoExt(LPWSTR lpApplicationName, SIZE_T szApplicationNameLength,
                          LPWSTR lpPathVariable, BOOL bFixUnc, LPWSTR lpCommandLine,
                          LPSECURITY_ATTRIBUTES lpProcessAttributes,
                          LPSECURITY_ATTRIBUTES lpThreadAttributes,
                          BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,
                          LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
                          LPPROCESS_INFORMATION lpProcessInformation) {
	LPWSTR iter = lpPathVariable, next, bufiter;
	LPWSTR buffer, new_buffer;
	buffer = DeeString_NewWideBuffer(128 + szApplicationNameLength);
	if unlikely(!buffer)
		goto err;
	for (;;) {
		DWORD error;
		size_t pathlen;
		next = iter;
		while (*next && *next != DeeSystem_DELIM)
			++next;
		pathlen = (size_t)(next - iter);
		if (!DeeSystem_IsSep(next[-1]))
			++pathlen;
		pathlen += szApplicationNameLength;
		if (pathlen > WSTR_LENGTH(buffer) ||
		    /* In FixUnc mode, the buffer length must match the exact path size. */
		    (bFixUnc && pathlen != WSTR_LENGTH(buffer))) {
			/* Resize the encoding buffer. */
			new_buffer = DeeString_ResizeWideBuffer(buffer, pathlen);
			if unlikely(!new_buffer)
				goto err_buffer;
			buffer = new_buffer;
		}
		bufiter = buffer;

		/* Create the path for the filename. */
		bufiter = (LPWSTR)mempcpyc(bufiter, iter, (size_t)(next - iter), sizeof(WCHAR));
		if (!DeeSystem_IsSep(next[-1]))
			*bufiter++ = (WCHAR)DeeSystem_SEP;
		bufiter  = (LPWSTR)mempcpyc(bufiter, lpApplicationName, szApplicationNameLength, sizeof(WCHAR));
		*bufiter = (WCHAR)'\0'; /* Ensure zero-termination */
		ASSERT(bFixUnc ? bufiter == buffer + WSTR_LENGTH(buffer)
		               : bufiter <= buffer + WSTR_LENGTH(buffer));
		if (bFixUnc) {
			LPWSTR pathname;
			DREF DeeStringObject *result, *fixed_result;
			/* If requested, also fix UNC casing for the buffer-path. */
			result = (DREF DeeStringObject *)DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
			if unlikely(!result)
				goto err;
			fixed_result = (DREF DeeStringObject *)DeeNTSystem_FixUncPath((DeeObject *)result);
			if unlikely(!fixed_result) {
err_result:
				Dee_DecrefDokill(result);
				goto err;
			}
			pathname = (LPWSTR)DeeString_AsWide((DeeObject *)fixed_result);
			if unlikely(!pathname) {
				Dee_Decref(fixed_result);
				goto err_result;
			}

			/* Create the process using a UNC pathname. */
			DBG_ALIGNMENT_DISABLE();
			if (CreateProcessW(pathname, lpCommandLine, lpProcessAttributes,
			                   lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
			                   lpCurrentDirectory, lpStartupInfo, lpProcessInformation)) {
				DBG_ALIGNMENT_ENABLE();
				/* If we managed to create the process using a UNC pathname,
				 * return that path instead of the regular buffer. */
				Dee_DecrefDokill(result);
				return fixed_result;
			}
			error = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			Dee_DecrefNokill(&DeeString_Type);
			if (result->s_data) {
				ASSERT(result->s_data->u_utf16 == buffer);
				result->s_data->u_utf16 = NULL;
				if (result->s_data->u_data[STRING_WIDTH_2BYTE] == (size_t *)buffer)
					result->s_data->u_data[STRING_WIDTH_2BYTE] = NULL;
				Dee_string_utf_fini(result->s_data, result);
				Dee_string_utf_free(result->s_data);
			}
			DeeObject_FreeTracker((DeeObject *)result);
			DeeObject_Free((DeeObject *)result);
		} else {
			/* All right. Let's do this! */
			DBG_ALIGNMENT_DISABLE();
			if (CreateProcessW(buffer, lpCommandLine, lpProcessAttributes,
			                   lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
			                   lpCurrentDirectory, lpStartupInfo, lpProcessInformation)) {
				/* Process was successfully created. */
				DREF DeeStringObject *result;
				DBG_ALIGNMENT_ENABLE();
				buffer = DeeString_TruncateWideBuffer(buffer, pathlen);
				result = (DREF DeeStringObject *)DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
				/* XXX: But the process is already running if `result' is NULL... */
				return result;
			}
			error = GetLastError();
			DBG_ALIGNMENT_ENABLE();
		}

		/* The creation failed. - Check if this is due to a file-
		 * not-found error, or because because of some other error. */
		if (!DeeNTSystem_IsFileNotFoundError(error) &&
		    !DeeNTSystem_IsAccessDeniedError(error)) {
			DeeNTSystem_ThrowErrorf(NULL, error, "Failed to create process");
			goto err_buffer;
		}
		while (*next && *next == DeeSystem_DELIM)
			++next;
		if (!*next)
			break;
		iter = next;
	}
	DeeString_FreeWideBuffer(buffer);
	return (DREF DeeStringObject *)ITER_DONE;
err_buffer:
	DeeString_FreeWideBuffer(buffer);
err:
	return NULL;
}

/* Same as `ipc_nt_CreateProcessPath()', but don't make use of the path-cache
 * @return: * :        The application name that was eventually used to start the process.
 * @return: NULL:      An error occurred and was thrown.
 * @return: ITER_DONE: Failed to start the process (see GetLastError()) */
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeStringObject *DCALL
ipc_nt_CreateProcessPathWithoutCache(LPWSTR lpApplicationName, SIZE_T szApplicationNameLength,
                                     LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                     LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                     BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,
                                     LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
                                     LPPROCESS_INFORMATION lpProcessInformation) {
	DREF DeeStringObject *result = (DREF DeeStringObject *)ITER_DONE;
	DREF DeeStringObject *path, *pathext;
	LPWSTR pathStr;
	BOOL bFixUnc = FALSE;
	path = (DREF DeeStringObject *)Dee_GetEnv((DeeObject *)&str_PATH);
	if unlikely(!ITER_ISOK(path)) {
		if (!path)
			goto err_nopath;
		goto done_nopath;
	}
	pathStr = (LPWSTR)DeeString_AsWide((DeeObject *)path);
	if unlikely(!pathStr)
		goto err;
again:
	result = nt_CreateProcessPathNoExt(lpApplicationName, szApplicationNameLength, pathStr,
	                                   bFixUnc, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
	                                   bInheritHandles, dwCreationFlags, lpEnvironment,
	                                   lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	if (result != (DREF DeeStringObject *)ITER_DONE)
		goto done;
	pathext = (DREF DeeStringObject *)Dee_GetEnv((DeeObject *)&str_PATHEXT);
	if (pathext != (DREF DeeStringObject *)ITER_DONE) {
		LPWSTR appnameBuffer, iter, next, newAppnameBuffer;
		SIZE_T appnameBufferSize = szApplicationNameLength + 4; /* Optimize for 4-character long extensions. */
		if unlikely(!pathext)
			goto err;
		appnameBuffer = (LPWSTR)Dee_Mallocc(appnameBufferSize + 1, sizeof(WCHAR));
		if unlikely(!appnameBuffer)
			goto err_pathext;
		iter = (LPWSTR)DeeString_AsWide((DeeObject *)pathext);
		if unlikely(!iter) {
			Dee_Free(appnameBuffer);
			goto err_pathext;
		}

		/* Copy the appname itself into the buffer. */
		memcpyc(appnameBuffer, lpApplicationName,
		        szApplicationNameLength, sizeof(WCHAR));
		for (;;) {
			SIZE_T appnameLength, extLength;
			next = iter;
			while (*next && *next != DeeSystem_DELIM)
				++next;

			/* Append extensions from $PATHEXT */
			extLength     = (SIZE_T)(next - iter);
			appnameLength = szApplicationNameLength + extLength;
			if unlikely(appnameLength > appnameBufferSize) {
				/* More-than-4-character extension. */
				newAppnameBuffer = (LPWSTR)Dee_Reallocc(appnameBuffer, appnameLength + 1, sizeof(WCHAR));
				if unlikely(!newAppnameBuffer) {
					Dee_Free(appnameBuffer);
					goto err_pathext;
				}
				appnameBuffer     = newAppnameBuffer;
				appnameBufferSize = appnameLength;
			}

			/* Copy the extension. */
			memcpyc(appnameBuffer + szApplicationNameLength,
			        iter, extLength, sizeof(WCHAR));
			appnameBuffer[appnameLength] = 0; /* Ensure ZERO-termination. */

			/* Try to create a process with this application name. */
			result = nt_CreateProcessPathNoExt(appnameBuffer, appnameLength, pathStr,
			                                   bFixUnc, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
			                                   bInheritHandles, dwCreationFlags, lpEnvironment,
			                                   lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
			if (result != (DREF DeeStringObject *)ITER_DONE) {
				/* Error or process successfully created. */
				Dee_Free(appnameBuffer);
				Dee_Decref(pathext);
				goto done;
			}
			if (!*next)
				break;
			while (*next && *next == DeeSystem_DELIM)
				++next;
			iter = next;
		}
		Dee_Free(appnameBuffer);
		Dee_Decref(pathext);
	}

	/* Try again while also fixing UNC paths. */
	if (!bFixUnc) {
		bFixUnc = TRUE;
		goto again;
	}
done:
	Dee_Decref(path);
done_nopath:
	return result;
err_pathext:
	Dee_Decref(pathext);
err:
	result = NULL;
	goto done;
err_nopath:
	result = NULL;
	goto done_nopath;
}

/* Create a new process by searching $PATH for its application name,
 * before trying again while appending extensions for $PATHEXT.
 * If all that still failed, try one more time while fixing UNC path strings.
 * @return: * :        The application name that was eventually used to start the process.
 * @return: NULL:      An error occurred and was thrown.
 * @return: ITER_DONE: Failed to start the process (see GetLastError()) */
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeStringObject *DCALL
ipc_nt_CreateProcessPath(DeeStringObject *exe_str, LPWSTR lpApplicationName,
                         LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
                         LPSECURITY_ATTRIBUTES lpThreadAttributes,
                         BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,
                         LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
                         LPPROCESS_INFORMATION lpProcessInformation) {
	DREF DeeStringObject *cached_pathname;

	/* Check if we already have the correct expansion of `exe_str' in-cache */
	ipc_exe2path_start_listen();
	cached_pathname = (DREF DeeStringObject *)DeeDict_TryGetItem((DeeObject *)&ipc_exe2path_cache,
	                                                             (DeeObject *)exe_str);
	if (cached_pathname != (DREF DeeStringObject *)ITER_DONE) {
		/* Simply launch via the cached application path. */
		if likely(cached_pathname != NULL) {
			BOOL bCreateOk;
			LPWSTR lpwCachedPathName;
			lpwCachedPathName = (LPWSTR)DeeString_AsWide((DeeObject *)cached_pathname);
			if unlikely(!lpwCachedPathName)
				goto err_cached_pathname;
			bCreateOk = CreateProcessW(lpwCachedPathName, lpCommandLine, lpProcessAttributes,
			                           lpThreadAttributes, bInheritHandles, dwCreationFlags,
			                           lpEnvironment, lpCurrentDirectory, lpStartupInfo,
			                           lpProcessInformation);
			if unlikely(!bCreateOk) {
				Dee_Decref(cached_pathname);
				cached_pathname = (DREF DeeStringObject *)ITER_DONE;
			}
		}
		return cached_pathname;
	}

	/* Do the actual hard work of finding the proper exe and launching it. */
	cached_pathname = ipc_nt_CreateProcessPathWithoutCache(lpApplicationName, WSTR_LENGTH(lpApplicationName),
	                                                       lpCommandLine, lpProcessAttributes,
	                                                       lpThreadAttributes, bInheritHandles, dwCreationFlags,
	                                                       lpEnvironment, lpCurrentDirectory, lpStartupInfo,
	                                                       lpProcessInformation);

	/* Remember the correct path mapping. */
	if (ITER_ISOK(cached_pathname))
		ipc_exe2path_remember(exe_str, cached_pathname);
	return cached_pathname;
err_cached_pathname:
	Dee_Decref(cached_pathname);
/*err:*/
	return NULL;
}

/* Try to create a process for exe f"{lpApplicationName...+=szApplicationNameLength}{lpPathExtVariable.split(";")}"
 * @return: * :        The application name that was eventually used to start the process.
 * @return: NULL:      An error occurred and was thrown.
 * @return: ITER_DONE: Failed to start the process (see GetLastError()) */
PRIVATE WUNUSED NONNULL((1, 3, 4)) DREF DeeStringObject *DCALL
nt_CreateProcessPathWithExt(LPWSTR lpApplicationName, SIZE_T szApplicationNameLength,
                            LPWSTR lpPathExtVariable, LPWSTR lpCommandLine,
                            LPSECURITY_ATTRIBUTES lpProcessAttributes,
                            LPSECURITY_ATTRIBUTES lpThreadAttributes,
                            BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,
                            LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
                            LPPROCESS_INFORMATION lpProcessInformation) {
	LPWSTR iter = lpPathExtVariable, next, bufiter;
	LPWSTR buffer, new_buffer;
	buffer = DeeString_NewWideBuffer(128 + szApplicationNameLength);
	if unlikely(!buffer)
		goto err;
	for (;;) {
		DWORD error;
		size_t pathlen;
		next = iter;
		while (*next && *next != DeeSystem_DELIM)
			++next;
		pathlen = (size_t)(next - iter);
		pathlen += szApplicationNameLength;
		if (pathlen > WSTR_LENGTH(buffer)) {
			/* Resize the encoding buffer. */
			new_buffer = DeeString_ResizeWideBuffer(buffer, pathlen);
			if unlikely(!new_buffer)
				goto err_buffer;
			buffer = new_buffer;
		}

		/* Create the path for the filename. */
		bufiter  = buffer;
		bufiter  = (LPWSTR)mempcpyc(bufiter, lpApplicationName, szApplicationNameLength, sizeof(WCHAR));
		bufiter  = (LPWSTR)mempcpyc(bufiter, iter, (size_t)(next - iter), sizeof(WCHAR));
		*bufiter = (WCHAR)'\0'; /* Ensure zero-termination */
		ASSERT(bufiter <= buffer + WSTR_LENGTH(buffer));

		/* All right. Let's do this! */
		DBG_ALIGNMENT_DISABLE();
		if (CreateProcessW(buffer, lpCommandLine, lpProcessAttributes,
		                   lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
		                   lpCurrentDirectory, lpStartupInfo, lpProcessInformation)) {
			DREF DeeStringObject *result;
			DBG_ALIGNMENT_ENABLE();
			/* Process was successfully created. */
			buffer = DeeString_TruncateWideBuffer(buffer, pathlen);
			result = (DREF DeeStringObject *)DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
			/* XXX: But the process is already running if `result' is NULL... */
			return result;
		}
		error = GetLastError();
		DBG_ALIGNMENT_ENABLE();

		/* The creation failed. - Check if this is due to a file-
		 * not-found error, or because because of some other error. */
		if (!DeeNTSystem_IsFileNotFoundError(error) && !DeeNTSystem_IsAccessDeniedError(error)) {
			DeeNTSystem_ThrowErrorf(NULL, error, "Failed to create process");
			goto err_buffer;
		}
		while (*next && *next == DeeSystem_DELIM)
			++next;
		if (!*next)
			break;
		iter = next;
	}
	DeeString_FreeWideBuffer(buffer);
	return (DREF DeeStringObject *)ITER_DONE;
err_buffer:
	DeeString_FreeWideBuffer(buffer);
err:
	return NULL;
}

/* Create a new process by trying to append extensions for $PATHEXT.
 * If all that still failed, try one more time while fixing UNC path strings.
 * @return: * :        The application name that was eventually used to start the process.
 * @return: NULL:      An error occurred and was thrown.
 * @return: ITER_DONE: Failed to start the process (see GetLastError()) */
PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeStringObject *DCALL
ipc_nt_CreateProcessExtWithoutCache(LPWSTR lpApplicationName, SIZE_T szApplicationNameLength,
                                    LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                    LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                    BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,
                                    LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
                                    LPPROCESS_INFORMATION lpProcessInformation) {
	DREF DeeStringObject *result = (DREF DeeStringObject *)ITER_DONE;
	DREF DeeStringObject *pathext_str;
	pathext_str = (DREF DeeStringObject *)Dee_GetEnv((DeeObject *)&str_PATHEXT);
	if (pathext_str != (DREF DeeStringObject *)ITER_DONE) {
		LPWSTR lpPathExt;
		if unlikely(!pathext_str)
			goto err;
		lpPathExt = (LPWSTR)DeeString_AsWide((DeeObject *)pathext_str);
		if unlikely(!lpPathExt) {
			result = NULL;
		} else {
			result = nt_CreateProcessPathWithExt(lpApplicationName,
			                                     szApplicationNameLength,
			                                     lpPathExt,
			                                     lpCommandLine,
			                                     lpProcessAttributes,
			                                     lpThreadAttributes,
			                                     bInheritHandles,
			                                     dwCreationFlags,
			                                     lpEnvironment,
			                                     lpCurrentDirectory,
			                                     lpStartupInfo,
			                                     lpProcessInformation);
		}
		Dee_Decref(pathext_str);
	}
	return result;
err:
	return NULL;
}

#endif /* ipc_Process_USE_CreateProcessW */


#ifdef ipc_Process_USE_UNIX_APIS
#define IS_SINGLE_DECIMAL_DIGIT(x) ((x) >= 0 && (x) <= 9)
#if (IS_SINGLE_DECIMAL_DIGIT(STDIN_FILENO) &&  \
     IS_SINGLE_DECIMAL_DIGIT(STDOUT_FILENO) && \
     IS_SINGLE_DECIMAL_DIGIT(STDERR_FILENO))
#define PRFMAXdSTDFD "9"
#elif __SIZEOF_INT__ <= 1
#define PRFMAXdSTDFD "-128"
#elif __SIZEOF_PID_T__ <= 2
#define PRFMAXdSTDFD "-32768"
#elif __SIZEOF_PID_T__ <= 4
#define PRFMAXdSTDFD "-2147483648"
#elif __SIZEOF_PID_T__ <= 8
#define PRFMAXdSTDFD "-9223372036854775808"
#else /* __SIZEOF_PID_T__ <= ... */
#define PRFMAXdSTDFD "-170141183460469231731687303715884105728"
#endif
#undef IS_SINGLE_DECIMAL_DIGIT
#if STDIN_FILENO == DEE_STDIN && STDOUT_FILENO == DEE_STDOUT && STDERR_FILENO == DEE_STDERR
#define DEE_STDID_TO_FILENO(id) ((int)(id))
#else /* ... */
#define DEE_STDID_TO_FILENO(id) _DEE_STDID_TO_FILENO[id]
PRIVATE int const _DEE_STDID_TO_FILENO[] = { STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO };
#endif /* !... */


#ifdef ipc_Process_USE_WCHAR_CHDIR_APIS
#define ipc_unix_chdir_char_t              wchar_t
#define DeeString_As_ipc_unix_chdir_char_t DeeString_AsWide
#else /* ipc_Process_USE_WCHAR_CHDIR_APIS */
#define ipc_unix_chdir_char_t              char
#define DeeString_As_ipc_unix_chdir_char_t DeeString_AsUtf8
#endif /* !ipc_Process_USE_WCHAR_CHDIR_APIS */

#ifdef ipc_Process_USE_WCHAR_SPAWN_APIS
#define ipc_unix_exec_char_t              wchar_t
#define DeeString_As_ipc_unix_exec_char_t DeeString_AsWide
#else /* ipc_Process_USE_WCHAR_SPAWN_APIS */
#define ipc_unix_exec_char_t              char
#define DeeString_As_ipc_unix_exec_char_t DeeString_AsUtf8
#endif /* !ipc_Process_USE_WCHAR_SPAWN_APIS */

#define ipc_unix_free_string_array(str_vec, objv_vec, item_count) \
	(Dee_Decrefv(objv_vec, item_count), Dee_Free(objv_vec), Dee_Free(str_vec))

/* Unpack a sequence of strings to construct an array of NUL-terminated utf-8 strings. */
PRIVATE WUNUSED NONNULL((1)) ipc_unix_exec_char_t **DCALL
ipc_unix_unpack_string_array(DeeObject *__restrict seq,
                             DREF DeeStringObject ***__restrict p_str_objv,
                             size_t *__restrict p_str_objc) {
	size_t i;
	ipc_unix_exec_char_t **result;
	DREF DeeStringObject **strings;
	strings = (DREF DeeStringObject **)DeeSeq_AsHeapVector(seq, p_str_objc);
	if unlikely(!strings)
		goto err;
	result = (ipc_unix_exec_char_t **)Dee_Mallocc(*p_str_objc + 1,
	                                              sizeof(ipc_unix_exec_char_t *));
	if unlikely(!result)
		goto err_strings;

	/* Load the utf-8 representation of all strings, and
	 * assert that all objects actually *are* strings. */
	for (i = 0; i < *p_str_objc; ++i) {
		ipc_unix_exec_char_t *str;
		DeeStringObject *str_ob;
		str_ob = strings[i];
		if unlikely(DeeObject_AssertTypeExact(str_ob, &DeeString_Type))
			goto err_strings_result;
		str = (ipc_unix_exec_char_t *)DeeString_As_ipc_unix_exec_char_t((DeeObject *)str_ob);
		if unlikely(!str)
			goto err_strings_result;
		result[i] = str;
	}

	/* Give the caller their string-array. */
	result[i]   = NULL;
	*p_str_objv = strings; /* Inherit */
	return result;
err_strings_result:
	Dee_Free(result);
err_strings:
	Dee_Decrefv(strings, *p_str_objc);
	Dee_Free(strings);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
ipc_unix_get_fd_for_exec(Process *__restrict self, unsigned int std_handle_id,
                         /*out[0..1]*/ DREF DeeObject **__restrict p_keep_alive_reference) {
	int result;
	DeeObject *file = self->p_stdfd[std_handle_id];
	if (file != NULL) {
		*p_keep_alive_reference = NULL;
	} else {
		file = DeeFile_GetStd(std_handle_id);
		if unlikely(!file)
			return -1;
		*p_keep_alive_reference = file; /* Let the caller inherit the reference. */
	}

	/* Extract the referenced handle from the file. */
	result = DeeUnixSystem_GetFD(file);
	if unlikely(result == -1)
		Dee_XClear(*p_keep_alive_reference);
	return result;
}


#if (defined(ipc_Process_USE_posix_spawn)   \
     ? (defined(CONFIG_HAVE_posix_fspawn_np)) \
     : (defined(CONFIG_HAVE_fexecve) && !defined(ipc_Process_USE_WCHAR_SPAWN_APIS)))
#define HAVE_ipc_unix_spawn_exe_fd
#endif /* ... */

#undef HAVE_ipc_unix_spawn_pwd_fd
#if (defined(ipc_Process_USE_posix_spawn)                      \
     ? defined(CONFIG_HAVE_posix_spawn_file_actions_addfchdir) \
     : defined(CONFIG_HAVE_fchdir))
#define HAVE_ipc_unix_spawn_pwd_fd
#endif /* ... */


struct unix_spawn_args {
	ipc_unix_exec_char_t      *usa_exe;   /* [1..1][valid_if(usa_exefd != -1)] Executable name (absolute path). */
#ifdef HAVE_ipc_unix_spawn_exe_fd
	int                        usa_exefd; /* Executable file descriptor. Used instead of `exe_str' when `!= -1' */
#endif /* HAVE_ipc_unix_spawn_exe_fd */
	ipc_unix_exec_char_t     **usa_argv;  /* [1..1][1..n] Argument vector (must be non-NULL) */
	ipc_unix_exec_char_t     **usa_envp;  /* [1..1][0..n] Environment strings (when NULL, use `environ' / `wenviron' instead) */
#ifdef ipc_Process_USE_posix_spawn
	posix_spawn_file_actions_t usa_spawn_file_actions; /* Posix spawn file actions */
	posix_spawnattr_t          usa_spawn_attr;         /* Posix spawn attributes */
#endif /* ipc_Process_USE_posix_spawn */

	/* All of the following are only used when compiled with `#ifndef ipc_Process_USE_posix_spawn' */
	int                        usa_stdfds[COMPILER_LENOF(((Process *)0)->p_stdfd)]; /* Overrides for STD file handles. */
	ipc_unix_chdir_char_t     *usa_pwd;   /* [0..1][valid_if(usa_pwdfd != -1)] Process working directory (when NULL, re-use PWD of parent) */
#ifdef HAVE_ipc_unix_spawn_pwd_fd
	int                        usa_pwdfd; /* Process working directory file descriptor. Used instead of `usa_pwd' when `!= -1' */
#endif /* HAVE_ipc_unix_spawn_pwd_fd */
};

#ifdef ipc_Process_USE_posix_spawn
PRIVATE NONNULL((1)) int DCALL
unix_spawn_args_init_posix_spawn(struct unix_spawn_args *__restrict self) {
	int error;
	/* Initialize control structures */
	error = posix_spawn_file_actions_init(&self->usa_spawn_file_actions);
	if unlikely(error != 0)
		goto err;
	error = posix_spawnattr_init(&self->usa_spawn_attr);
	if unlikely(error != 0)
		goto err_spawn_file_actions;

	if (self->usa_stdfds[DEE_STDIN] != STDIN_FILENO) {
		error = posix_spawn_file_actions_adddup2(&self->usa_spawn_file_actions, self->usa_stdfds[DEE_STDIN], STDIN_FILENO);
		if unlikely(error != 0)
			goto err_spawn_file_actions_spawn_attr;
	}
	if (self->usa_stdfds[DEE_STDOUT] != STDOUT_FILENO) {
		error = posix_spawn_file_actions_adddup2(&self->usa_spawn_file_actions, self->usa_stdfds[DEE_STDOUT], STDOUT_FILENO);
		if unlikely(error != 0)
			goto err_spawn_file_actions_spawn_attr;
	}
	if (self->usa_stdfds[DEE_STDERR] != STDERR_FILENO) {
		error = posix_spawn_file_actions_adddup2(&self->usa_spawn_file_actions, self->usa_stdfds[DEE_STDERR], STDERR_FILENO);
		if unlikely(error != 0)
			goto err_spawn_file_actions_spawn_attr;
	}
#ifdef HAVE_ipc_unix_spawn_pwd_fd
	if (self->usa_pwdfd != -1) {
		error = posix_spawn_file_actions_addfchdir(&self->usa_spawn_file_actions, self->usa_pwdfd);
		if unlikely(error != 0)
			goto err_spawn_file_actions_spawn_attr;
	} else
#endif /* HAVE_ipc_unix_spawn_pwd_fd */
	if (self->usa_pwd != NULL) {
		error = posix_spawn_file_actions_addchdir(&self->usa_spawn_file_actions, self->usa_pwd);
		if unlikely(error != 0)
			goto err_spawn_file_actions_spawn_attr;
	}
err:
	return error;
err_spawn_file_actions_spawn_attr:
	posix_spawnattr_destroy(&self->usa_spawn_attr);
err_spawn_file_actions:
	posix_spawn_file_actions_destroy(&self->usa_spawn_file_actions);
	goto err;
}
#endif /* ipc_Process_USE_posix_spawn */


#if (defined(ipc_Process_USE_vfork_AND_execve) || \
     defined(ipc_Process_USE_fork_AND_execve))
/* Do spawn actions within the child process, then execute the child program.
 * Only returns upon error (upon success, the child program is given control) */
PRIVATE void DCALL
ipc_unix_spawn_in_child(struct unix_spawn_args const *__restrict self) {
#if 1
#define DBG_ipc_unix_spawn_in_child_ACTION(...) Dee_DPRINTF("ipc_unix_spawn_in_child: " __VA_ARGS__)
#else
#define DBG_ipc_unix_spawn_in_child_ACTION(...) (void)0
#endif

	/* Load std file descriptor overrides */
	if (self->usa_stdfds[DEE_STDIN] != STDIN_FILENO) {
		DBG_ipc_unix_spawn_in_child_ACTION("dup2(%d, %d)\n", self->usa_stdfds[DEE_STDIN], STDIN_FILENO);
		if unlikely(dup2(self->usa_stdfds[DEE_STDIN], STDIN_FILENO) < 0)
			return;
	}
	if (self->usa_stdfds[DEE_STDOUT] != STDOUT_FILENO) {
		DBG_ipc_unix_spawn_in_child_ACTION("dup2(%d, %d)\n", self->usa_stdfds[DEE_STDOUT], STDOUT_FILENO);
		if unlikely(dup2(self->usa_stdfds[DEE_STDOUT], STDOUT_FILENO) < 0)
			return;
	}
	if (self->usa_stdfds[DEE_STDERR] != STDERR_FILENO) {
		DBG_ipc_unix_spawn_in_child_ACTION("dup2(%d, %d)\n", self->usa_stdfds[DEE_STDERR], STDERR_FILENO);
		if unlikely(dup2(self->usa_stdfds[DEE_STDERR], STDERR_FILENO) < 0)
			return;
	}

	/* Load process-working-directory override */
#ifdef HAVE_ipc_unix_spawn_pwd_fd
	if (self->usa_pwdfd != -1) {
		DBG_ipc_unix_spawn_in_child_ACTION("fchdir(%d)\n", self->usa_pwdfd);
		if unlikely(fchdir(self->usa_pwdfd))
			return;
	} else
#endif /* HAVE_ipc_unix_spawn_pwd_fd */
	if (self->usa_pwd != NULL) {
#ifdef ipc_Process_USE_WCHAR_CHDIR_APIS
		DBG_ipc_unix_spawn_in_child_ACTION("chdir(%lq)\n", self->usa_pwd);
		if unlikely(wchdir(self->usa_pwd))
			return;
#else /* ipc_Process_USE_WCHAR_CHDIR_APIS */
		DBG_ipc_unix_spawn_in_child_ACTION("chdir(%q)\n", self->usa_pwd);
		if unlikely(chdir(self->usa_pwd))
			return;
#endif /* !ipc_Process_USE_WCHAR_CHDIR_APIS */
	}

	/* Finally, it's time to do the actual exec! */
#ifdef HAVE_ipc_unix_spawn_exe_fd
	if (self->usa_exefd != -1) {
		char **envp = self->usa_envp;
		if (envp == NULL)
			envp = environ;
		DBG_ipc_unix_spawn_in_child_ACTION("fexecve(%d, ...)\n", self->usa_exefd);
		fexecve(self->usa_exefd, self->usa_argv, envp);
	} else
#endif /* HAVE_ipc_unix_spawn_exe_fd */
	{
#ifdef ipc_Process_USE_WCHAR_SPAWN_APIS
		if (self->usa_envp == NULL) {
#ifdef CONFIG_HAVE_wexecv
			DBG_ipc_unix_spawn_in_child_ACTION("wexecv(%lq, ...)\n", self->usa_exe);
			wexecv(self->usa_exe, self->usa_argv);
#else /* CONFIG_HAVE_wexecv */
			DBG_ipc_unix_spawn_in_child_ACTION("wexecve(%lq, ...)\n", self->usa_exe);
			wexecve(self->usa_exe, self->usa_argv, wenviron);
#endif /* !CONFIG_HAVE_wexecv */
		} else {
			DBG_ipc_unix_spawn_in_child_ACTION("wexecve(%lq, ...)\n", self->usa_exe);
			wexecve(self->usa_exe, self->usa_argv, self->usa_envp);
		}
#else /* ipc_Process_USE_WCHAR_SPAWN_APIS */
		if (self->usa_envp == NULL) {
#ifdef CONFIG_HAVE_execv
			DBG_ipc_unix_spawn_in_child_ACTION("execv(%q, ...)\n", self->usa_exe);
			execv(self->usa_exe, self->usa_argv);
#else /* CONFIG_HAVE_execv */
			DBG_ipc_unix_spawn_in_child_ACTION("execve(%q, ...)\n", self->usa_exe);
			execve(self->usa_exe, self->usa_argv, environ);
#endif /* !CONFIG_HAVE_execv */
		} else {
			DBG_ipc_unix_spawn_in_child_ACTION("execve(%q, ...)\n", self->usa_exe);
			execve(self->usa_exe, self->usa_argv, self->usa_envp);
		}
#endif /* !ipc_Process_USE_WCHAR_SPAWN_APIS */
	}
#undef DBG_ipc_unix_spawn_in_child_ACTION
}
#endif /* ... */

#ifdef ipc_Process_USE_fork_AND_execve
#ifndef CONFIG_HAVE_writeall
#undef writeall
#define writeall dee_writeall
PRIVATE void DCALL dee_writeall(int fd, void const *buf, size_t num_bytes) {
	for (;;) {
		__SSIZE_TYPE__ temp = write(fd, buf, num_bytes);
		if unlikely(temp <= 0)
			continue;
		if likely((size_t)temp >= num_bytes)
			break;
		num_bytes -= (size_t)temp;
		buf = (__BYTE_TYPE__ const *)buf + (size_t)temp;
	}
}
#endif /* !CONFIG_HAVE_writeall */
#endif /* ipc_Process_USE_fork_AND_execve */

#ifdef ipc_Process_USE_spawnve

#ifdef ipc_Process_USE_WCHAR_CHDIR_APIS
#ifndef CONFIG_HAVE_wcslen
#define CONFIG_HAVE_wcslen
#undef wcslen
#define wcslen dee_wcslen
DeeSystem_DEFINE_wcslen(dee_wcslen)
#endif /* !CONFIG_HAVE_wcslen */
#endif /* ipc_Process_USE_WCHAR_CHDIR_APIS */


PRIVATE WUNUSED ipc_unix_chdir_char_t *DCALL
ipc_unix_chdir_getcwd(void) {
	ipc_unix_chdir_char_t *buffer, *new_buffer;
	size_t bufsize = PATH_MAX, buflen;
	buffer = (ipc_unix_chdir_char_t *)Dee_TryMallocc(bufsize, sizeof(ipc_unix_chdir_char_t));
	if unlikely(!buffer)
		goto err;
#ifdef EINTR
again:
#endif /* EINTR */
	if (DeeThread_CheckInterrupt())
		goto err_buf;
	DBG_ALIGNMENT_DISABLE();
#ifdef ipc_Process_USE_WCHAR_CHDIR_APIS
	while (!wgetcwd(buffer, bufsize + 1))
#else /* ipc_Process_USE_WCHAR_CHDIR_APIS */
	while (!getcwd(buffer, bufsize + 1))
#endif /* !ipc_Process_USE_WCHAR_CHDIR_APIS */
	{
		/* Increase the buffer and try again. */
#if defined(CONFIG_HAVE_errno) && defined(ERANGE)
		int error = DeeSystem_GetErrno();
		if (error != ERANGE) {
			DBG_ALIGNMENT_ENABLE();
#ifdef EINTR
			if (error == EINTR)
				goto again;
#endif /* EINTR */
			goto err_buf;
		}
#endif /* CONFIG_HAVE_errno && ERANGE */
		DBG_ALIGNMENT_ENABLE();
		bufsize *= 2;
		new_buffer = (ipc_unix_chdir_char_t *)Dee_TryReallocc(buffer, bufsize,
		                                                      sizeof(ipc_unix_chdir_char_t));
		if unlikely(!new_buffer)
			goto err_buf;
		DBG_ALIGNMENT_DISABLE();
	}
	DBG_ALIGNMENT_ENABLE();
#ifdef ipc_Process_USE_WCHAR_CHDIR_APIS
	buflen = wcslen(buffer);
#else /* ipc_Process_USE_WCHAR_CHDIR_APIS */
	buflen = strlen(buffer);
#endif /* !ipc_Process_USE_WCHAR_CHDIR_APIS */
	while (buflen && DeeSystem_IsSep(buffer[buflen - 1]))
		--buflen;
	if unlikely(!buflen) {
		buflen    = 1;
		buffer[0] = '.';
	}
	new_buffer = (ipc_unix_chdir_char_t *)Dee_TryReallocc(buffer, buflen + 1, sizeof(ipc_unix_chdir_char_t));
	if likely(new_buffer)
		buffer = new_buffer;
	return buffer;
err_buf:
	Dee_Free(buffer);
err:
	return NULL;
}
#endif /* ipc_Process_USE_spawnve */


/* Spawn a new process and return its PID.
 * @return: >= 0: The PID of the new child process
 * @return: <  0: The negative errno value that caused the spawn to fail. */
PRIVATE WUNUSED pid_t DCALL
ipc_unix_spawn(struct unix_spawn_args const *__restrict self) {

	/* Implementation: posix_spawn */
#ifdef ipc_Process_USE_posix_spawn
	{
		int error;
		pid_t cpid;
		char **envp = self->usa_envp;
		if (envp == NULL)
			envp = environ;
#ifdef HAVE_ipc_unix_spawn_exe_fd
		if (self->usa_exefd != -1) {
			error = posix_fspawn_np(&cpid, self->usa_exefd,
			                        &self->usa_spawn_file_actions,
			                        &self->usa_spawn_attr,
			                        self->usa_argv, envp);
		} else
#endif /* HAVE_ipc_unix_spawn_exe_fd */
		{
			error = posix_spawn(&cpid, self->usa_exe,
			                    &self->usa_spawn_file_actions,
			                    &self->usa_spawn_attr,
			                    self->usa_argv, envp);
		}
		if unlikely(error != 0)
			cpid = -error;
		return cpid;
	}
#endif /* ipc_Process_USE_posix_spawn */

	/* Implementation: vfork() + exec()
	 * NOTE: We only ever configure ourselves to use vfork() when VM sharing is enabled! */
#ifdef ipc_Process_USE_vfork_AND_execve
	{
		pid_t cpid;
		int saved_errno, error;
		saved_errno = errno;
		errno       = 0;
		cpid        = vfork();
		if (cpid == 0) {
			/* We're now within the child process! */
			ipc_unix_spawn_in_child(self);
			_Exit(127);
		}

		/* Check if the vfork() itself failed */
		if (cpid < 0) {
			error = errno;
			if unlikely(error == 0)
				error = 1; /* Shouldn't happen, but mustn't indicate success here */
			return -error;
		}

		/* Check if the child process encountered a problem.
		 *
		 * Because vfork() (in this configuration) allows us to share a VM with our
		 * child process, any error that happened within the child process will have
		 * modified `errno' from the 0-value we wrote prior to the vfork().
		 *
		 * As such, if the child process encountered any kind of problem, we now have
		 * its error-code safely stored in our `errno'. */
		error = errno;
		if (error != 0) {
			int status;
			/* Must still reap `cpid' */
			while (ipc_joinpid(cpid, &status) < 0) {
#ifdef EINTR
				if (errno == EINTR)
					continue;
#endif /* EINTR */
				break;
			}
			return -error;
		}
		errno = saved_errno;
		return cpid;
	}
#endif /* ipc_Process_USE_vfork_AND_execve */

	/* Implementation: fork() + exec() */
#ifdef ipc_Process_USE_fork_AND_execve
	{
		pid_t cpid;
		int pipefd[2];
		int error;
		if unlikely(pipe2(pipefd, O_CLOEXEC) != 0)
			return -errno;

		/* Still try to use vfork() (if it's available), since it might be faster than regular fork(),
		 * and since in this variant we're not relying on vfork()'s (usual) VM-sharing feature, we can
		 * just use it as a drop-in replacement for fork(). */
#ifdef CONFIG_HAVE_vfork
		cpid = vfork();
#else /* CONFIG_HAVE_vfork */
		cpid = fork();
#endif /* !CONFIG_HAVE_vfork */
		if (cpid == 0) {
			/* We're now within the child process! */
			ipc_unix_spawn_in_child(self);

			/* Exec fail -> write the current `errno' value to the pipe. */
			error = errno;
			if unlikely(error == 0)
				error = 1; /* Shouldn't happen, but mustn't indicate success here */
			(void)writeall(pipefd[1], &error, sizeof(error));

			/* Terminating our process will close the pipe, and unblock our parent. */
			_Exit(127);
		}

		/* Check if the fork() itself failed */
		if (cpid < 0) {
			error = errno;
			if unlikely(error == 0)
				error = 1; /* Shouldn't happen, but mustn't indicate success here */
			return -error;
		}

		/* Wait until the child process closes  */
		(void)close(pipefd[1]); /* Close write-end */
		error = 0;
		for (;;) {
			__SSIZE_TYPE__ temp;
			temp = read(pipefd[0], &error, sizeof(error));
			if (temp >= 0)
				break;
#ifdef EINTR
			if (errno == EINTR)
				continue;
#endif /* EINTR */
			error = errno;
			break;
		}
		(void)close(pipefd[0]); /* Close read-end */

		/* Check if the child process encountered an error prior-to/during exec() */
		if (error != 0) {
			int status;
			Dee_DPRINTF("ipc_unix_spawn: fork'd child process answered with error %d\n", error);

			/* Must still reap `cpid' */
			while (ipc_joinpid(cpid, &status) < 0) {
#ifdef EINTR
				if (errno == EINTR)
					continue;
#endif /* EINTR */
				break;
			}
			return -error;
		}
		return cpid;
	}
#endif /* ipc_Process_USE_fork_AND_execve */

	/* Implementation: spawnve() */
#ifdef ipc_Process_USE_spawnve
	/* NOTE: The spawn-implementation is extremely bad racy, since it requires us to
	 *       temporarily re-direct the pwd/stdin/stdout/stderr of our own process! */
	{
		pid_t cpid;
		int error;
		int saved_fds[COMPILER_LENOF(self->usa_stdfds)];
		unsigned int i;
		ipc_unix_chdir_char_t *saved_pwd = NULL;

		/* Save STD stream descriptors where they will end up being overwritten. */
		for (i = 0; i < COMPILER_LENOF(saved_fds); ++i) {
			int i_fd = DEE_STDID_TO_FILENO(i);
			if (self->usa_stdfds[i] == i_fd) {
				saved_fds[i] = i_fd;
			} else {
				saved_fds[i] = dup(i_fd);
				if unlikely(saved_fds[i] == -1)
					goto return_errno;
			}
		}
		if (self->usa_pwd) {
			/* Save current PWD into `saved_pwd' */
			saved_pwd = ipc_unix_chdir_getcwd();
			if unlikely(!saved_pwd)
				goto return_errno_saved_fd;
		}

		/* Override host process content. */
		for (i = 0; i < COMPILER_LENOF(saved_fds); ++i) {
			int i_fd = DEE_STDID_TO_FILENO(i);
			if (saved_fds[i] != i_fd) {
				if unlikely(dup2(saved_fds[i], i_fd) == -1)
					goto return_errno_saved_fd_pwd_restore_fd_i;
			}
		}
		if (self->usa_pwd) {
#ifdef ipc_Process_USE_WCHAR_CHDIR_APIS
			if unlikely(wchdir(self->usa_pwd) != 0)
#else /* ipc_Process_USE_WCHAR_CHDIR_APIS */
			if unlikely(chdir(self->usa_pwd) != 0)
#endif /* !ipc_Process_USE_WCHAR_CHDIR_APIS */
			{
				goto return_errno_saved_fd_pwd_restore_fd;
			}
		}

		/* Spawn new child process */
#ifdef ipc_Process_USE_WCHAR_SPAWN_APIS
		if (self->usa_envp == NULL) {
#ifdef CONFIG_HAVE_wspawnv
			cpid = wspawnv(P_NOWAIT, self->usa_exe, self->usa_argv);
#else /* CONFIG_HAVE_wspawnv */
			cpid = wspawnve(P_NOWAIT, self->usa_exe, self->usa_argv, wenviron);
#endif /* !CONFIG_HAVE_wspawnv */
		} else {
			cpid = wspawnve(P_NOWAIT, self->usa_exe, self->usa_argv, self->usa_envp);
		}
#else /* ipc_Process_USE_WCHAR_SPAWN_APIS */
		if (self->usa_envp == NULL) {
#ifdef CONFIG_HAVE_spawnv
			cpid = spawnv(P_NOWAIT, self->usa_exe, self->usa_argv);
#else /* CONFIG_HAVE_spawnv */
			cpid = spawnve(P_NOWAIT, self->usa_exe, self->usa_argv, environ);
#endif /* !CONFIG_HAVE_spawnv */
		} else {
			cpid = spawnve(P_NOWAIT, self->usa_exe, self->usa_argv, self->usa_envp);
		}
#endif /* !ipc_Process_USE_WCHAR_SPAWN_APIS */
		if (cpid < 0) {
			cpid = -errno;
			if unlikely(cpid == 0)
				cpid = -1; /* Shouldn't happen, but mustn't indicate success here */
		}

		/* Restore host process content. */
		if (saved_pwd) {
#ifdef ipc_Process_USE_WCHAR_CHDIR_APIS
			(void)wchdir(saved_pwd);
#else /* ipc_Process_USE_WCHAR_CHDIR_APIS */
			(void)chdir(saved_pwd);
#endif /* !ipc_Process_USE_WCHAR_CHDIR_APIS */
			Dee_Free(saved_pwd);
		}
		for (i = 0; i < COMPILER_LENOF(saved_fds); ++i) {
			int i_fd = DEE_STDID_TO_FILENO(i);
			if (saved_fds[i] != i_fd) {
				(void)dup2(saved_fds[i], i_fd);
				(void)close(saved_fds[i]);
			}
		}

		/* Return child PID */
		return cpid;

		/* Error handling. */
return_errno_saved_fd_pwd_restore_fd:
		i = COMPILER_LENOF(saved_fds);
return_errno_saved_fd_pwd_restore_fd_i:
		while (i--) {
			int i_fd = DEE_STDID_TO_FILENO(i);
			if (saved_fds[i] != i_fd) {
				(void)dup2(saved_fds[i], i_fd);
				(void)close(saved_fds[i]);
			}
		}
/*return_errno_saved_fd_pwd:*/
		Dee_Free(saved_pwd);
return_errno_saved_fd:
		i = COMPILER_LENOF(saved_fds);
		while (i--) {
			if (saved_fds[i] != DEE_STDID_TO_FILENO(i))
				(void)close(saved_fds[i]);
		}
return_errno:
		error = errno;
		if unlikely(error == 0)
			error = 1; /* Shouldn't happen, but mustn't indicate success here */
		return -error;
	}
#endif /* ipc_Process_USE_spawnve */

}
#endif /* ipc_Process_USE_UNIX_APIS */



/* @return: 1:  Already started
 * @return: 0:  Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
process_start_impl(Process *__restrict self) {
#ifdef ipc_Process_USE_STUB
	(void)self;
	return ipc_unimplemented();
#else /* ipc_Process_USE_STUB */
again:
	Process_LockRead(self);
	/* Check if the process had already been started, or if
	 * another thread is currently in the process of starting
	 * the process. */
	if unlikely(self->p_state & (PROCESS_FLAG_STARTED | PROCESS_FLAG_STARTING)) {
		uint16_t state = self->p_state;
		Process_LockEndRead(self);
		if (state & PROCESS_FLAG_STARTING) {
			SCHED_YIELD();
			goto again;
		}
		return 1;
	}

	/* Upgrade to a write-lock so we can set `PROCESS_FLAG_STARTED' */
	if (!Process_LockUpgrade(self)) {
		if unlikely(self->p_state & PROCESS_FLAG_STARTED) {
			uint16_t state = self->p_state;
			Process_LockEndWrite(self);
			if (state & PROCESS_FLAG_STARTING) {
				SCHED_YIELD();
				goto again;
			}
			return 1;
		}
	}
	self->p_state |= PROCESS_FLAG_STARTING;
	Process_LockEndWrite(self);

	/* Ensure that mandatory attributes have been set. */
	ASSERT(self->p_exe);
#ifdef ipc_Process_USE_cmdline
	ASSERT(self->p_cmdline);
#endif /* ipc_Process_USE_cmdline */
#ifdef ipc_Process_USE_argv
	ASSERT(self->p_argv);
#endif /* ipc_Process_USE_argv */

#ifdef ipc_Process_USE_CreateProcessW
	{
		STARTUPINFOW siStartupInfo;
		PROCESS_INFORMATION piProcessInformation;
		DREF DeeStringObject *full_exe_str;
		DREF DeeStringObject *exe_str;
		DREF DeeStringObject *pwd_str;
		DREF DeeStringObject *env_str;
		DREF DeeObject *stdfd_files[COMPILER_LENOF(self->p_stdfd)];
		LPWSTR lpwExe, lpwCmdLine, lpwCmdLineCopy;
		LPWSTR lpwProcessWorkingDirectory, lpwEnviron;
#define WANT_DeeObject_AsFileSystemPathString
		exe_str = DeeObject_AsFileSystemPathString(self->p_exe);
		if unlikely(!exe_str)
			goto err;
		lpwExe = (LPWSTR)DeeString_AsWide((DeeObject *)exe_str);
		if unlikely(!lpwExe)
			goto err_exe_str;
		lpwCmdLine = (LPWSTR)DeeString_AsWide((DeeObject *)self->p_cmdline);
		if unlikely(!lpwCmdLine)
			goto err_exe_str;

		/* Must create a duplicate of `lpwCmdLine' since the `CreateProcessW'
		 * function is allowed to (and does) modify it. - So if we were to pass
		 * the string's original buffer, we would accidentally inplace-modify
		 * that string. */
		lpwCmdLineCopy = (LPWSTR)Dee_Mallocac(WSTR_LENGTH(lpwCmdLine) + 1, sizeof(WCHAR));
		if unlikely(!lpwCmdLineCopy)
			goto err_exe_str;
		memcpyc(lpwCmdLineCopy, lpwCmdLine, WSTR_LENGTH(lpwCmdLine) + 1, sizeof(WCHAR));

		/* Load the PWD-override (if given) */
		pwd_str                    = NULL;
		lpwProcessWorkingDirectory = NULL;
		if (self->p_pwd != NULL) {
#define WANT_DeeObject_AsFileSystemPathString
			pwd_str = DeeObject_AsFileSystemPathString(self->p_pwd);
			if unlikely(!pwd_str)
				goto err_exe_str_lpwCmdLineCopy;
			lpwProcessWorkingDirectory = (LPWSTR)DeeString_AsWide((DeeObject *)pwd_str);
			if unlikely(!lpwProcessWorkingDirectory)
				goto err_exe_str_lpwCmdLineCopy_pwd_str;
		}

		/* Load environment strings (if given) */
		env_str    = NULL;
		lpwEnviron = NULL;
		if (self->p_envp != NULL) {
			env_str = ipc_nt_pack_environ(self->p_envp);
			if unlikely(!env_str)
				goto err_exe_str_lpwCmdLineCopy_pwd_str;
			lpwEnviron = (LPWSTR)DeeString_AsWide((DeeObject *)env_str);
			if unlikely(!lpwEnviron)
				goto err_exe_str_lpwCmdLineCopy_pwd_str_env_str;
		}

		/* Fill in process startup info. */
		bzero(&siStartupInfo, sizeof(STARTUPINFOW));
		siStartupInfo.cb      = sizeof(STARTUPINFOW);
		siStartupInfo.dwFlags = STARTF_USESTDHANDLES;

		/* Load standard handles for the new process. */
		{
			unsigned int i;
			HANDLE hStdHandles[COMPILER_LENOF(stdfd_files)];
			for (i = 0; i < COMPILER_LENOF(stdfd_files); ++i) {
				int error = ipc_nt_get_handle_for_CreateProcessW(self, i,
				                                                 &stdfd_files[i],
				                                                 &hStdHandles[i]);
				if unlikely(error != 0) {
					Dee_XDecrefv(stdfd_files, i);
					goto err_exe_str_lpwCmdLineCopy_pwd_str_env_str;
				}
			}
			siStartupInfo.hStdInput  = hStdHandles[DEE_STDIN];
			siStartupInfo.hStdOutput = hStdHandles[DEE_STDOUT];
			siStartupInfo.hStdError  = hStdHandles[DEE_STDERR];
		}

		/* Finally, we get to the actual mean to process creation!
		 *
		 * Check if `lpwExe' contains any forward or backward slashes.
		 * If it doesn't, then we must search $PATH and $PATHEXT for the
		 * program. */
		if (wmemchr(lpwExe, '/', WSTR_LENGTH(lpwExe)) == NULL &&
		    wmemchr(lpwExe, '\\', WSTR_LENGTH(lpwExe)) == NULL) {
			full_exe_str = ipc_nt_CreateProcessPath(exe_str, lpwExe, lpwCmdLineCopy, NULL, NULL,
			                                        TRUE, CREATE_UNICODE_ENVIRONMENT, lpwEnviron,
			                                        lpwProcessWorkingDirectory, &siStartupInfo,
			                                        &piProcessInformation);
			if (full_exe_str != (DREF DeeStringObject *)ITER_DONE) {
				DREF DeeObject *old_exe_str;
				if unlikely(full_exe_str == NULL)
					goto err_exe_str_lpwCmdLineCopy_pwd_str_env_str_stdfd_files;

				/* Successfully launched process. (remember the full exe path) */
save_full_exe_str_and_process_created_ok:
				Process_LockWrite(self);
				old_exe_str = self->p_exe;
				self->p_exe = (DREF DeeObject *)full_exe_str; /* Inherit reference */
				Process_LockEndWrite(self);
				Dee_Decref(old_exe_str);
				goto process_created_ok;
			}
			/* Following windows conventions, also search PWD for raw executable names,
			 * which is done by falling through to the absolute-path handler below. */
		}

		/* Create a new process, given its absolute path. */
		DBG_ALIGNMENT_DISABLE();
		if (!CreateProcessW(lpwExe, lpwCmdLineCopy, NULL, NULL, TRUE,
		                    CREATE_UNICODE_ENVIRONMENT, lpwEnviron,
		                    lpwProcessWorkingDirectory, &siStartupInfo,
		                    &piProcessInformation)) {
			DWORD dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsUncError(dwError)) {
				LPWSTR lpwFullExe;
				full_exe_str = (DREF DeeStringObject *)DeeNTSystem_FixUncPath((DeeObject *)exe_str);
				if unlikely(!full_exe_str)
					goto err_exe_str_lpwCmdLineCopy_pwd_str_env_str_stdfd_files;
				lpwFullExe = (LPWSTR)DeeString_AsWide((DeeObject *)full_exe_str);
				if unlikely(!lpwFullExe)
					goto err_exe_str_lpwCmdLineCopy_pwd_str_env_str_stdfd_files_full_exe_str;
				DBG_ALIGNMENT_DISABLE();
				if (CreateProcessW(lpwFullExe, lpwCmdLineCopy, NULL, NULL, TRUE,
				                   CREATE_UNICODE_ENVIRONMENT, lpwEnviron,
				                   lpwProcessWorkingDirectory, &siStartupInfo,
				                   &piProcessInformation)) {
					DBG_ALIGNMENT_ENABLE();
cache_full_exe_str_and_process_created_ok:
					ipc_exe2path_remember(exe_str, full_exe_str);
					goto save_full_exe_str_and_process_created_ok;
				}
				dwError = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				Dee_Decref(full_exe_str);
			}
			if (DeeNTSystem_IsFileNotFoundError(dwError) ||
			    DeeNTSystem_IsAccessDeniedError(dwError)) {
				/* Append %PATHEXT% to the executable name, and try again. */
				full_exe_str = ipc_nt_CreateProcessExtWithoutCache(lpwExe, WSTR_LENGTH(lpwExe), lpwCmdLineCopy, NULL,
				                                                   NULL, TRUE, CREATE_UNICODE_ENVIRONMENT, lpwEnviron,
				                                                   lpwProcessWorkingDirectory, &siStartupInfo,
				                                                   &piProcessInformation);
				if (full_exe_str != (DREF DeeStringObject *)ITER_DONE) {
					if unlikely(!full_exe_str)
						goto err_exe_str_lpwCmdLineCopy_pwd_str_env_str_stdfd_files;
					goto cache_full_exe_str_and_process_created_ok;
				}
				DBG_ALIGNMENT_DISABLE();
				dwError = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				DeeNTSystem_ThrowErrorf(NULL, dwError, "Application %r could not be found", exe_str);
			} else {
				DeeNTSystem_ThrowErrorf(NULL, dwError, "Failed to create process");
			}
			goto err_exe_str_lpwCmdLineCopy_pwd_str_env_str_stdfd_files;
		}
		DBG_ALIGNMENT_ENABLE();

		/* Error handling... */
		__IF0 {
err_exe_str_lpwCmdLineCopy_pwd_str_env_str_stdfd_files_full_exe_str:
			Dee_Decref(full_exe_str);
err_exe_str_lpwCmdLineCopy_pwd_str_env_str_stdfd_files:
			Dee_XDecrefv(stdfd_files, COMPILER_LENOF(stdfd_files));
err_exe_str_lpwCmdLineCopy_pwd_str_env_str:
			Dee_XDecref(env_str);
err_exe_str_lpwCmdLineCopy_pwd_str:
			Dee_XDecref(pwd_str);
err_exe_str_lpwCmdLineCopy:
			Dee_Freea(lpwCmdLineCopy);
err_exe_str:
			Dee_Decref(exe_str);
			goto err;
		}

		/* Cleanup... */
process_created_ok:
		Dee_XDecrefv(stdfd_files, COMPILER_LENOF(stdfd_files));
		Dee_XDecref(env_str);
		Dee_XDecref(pwd_str);
		Dee_Freea(lpwCmdLineCopy);
		Dee_Decref(exe_str);

		/* Close our handle to the process's main thread. */
		DBG_ALIGNMENT_DISABLE();
		CloseHandle(piProcessInformation.hThread);
		DBG_ALIGNMENT_ENABLE();

		/* Remember that the process has now been started. */
		Process_LockWrite(self);
		self->p_handle = piProcessInformation.hProcess;
		self->p_pid    = piProcessInformation.dwProcessId;
		self->p_state |= PROCESS_FLAG_STARTED;
		Process_LockEndWrite(self);
		return 0;
	}
#endif /* ipc_Process_USE_CreateProcessW */

#ifdef ipc_Process_USE_UNIX_APIS
	{
		pid_t cpid;
		struct unix_spawn_args spawn_args;
		size_t argc, envc;
		DREF DeeObject *stdfd_files[COMPILER_LENOF(self->p_stdfd)];
		DREF DeeStringObject **argv_objv;
		DREF DeeStringObject **envp_objv;
		DREF DeeStringObject *pwd_obj;

		/* Load argument and environment strings */
		spawn_args.usa_argv = ipc_unix_unpack_string_array(self->p_argv, &argv_objv, &argc);
		if unlikely(!spawn_args.usa_argv)
			goto err;
		if (self->p_envp) {
			spawn_args.usa_envp = ipc_unix_unpack_string_array(self->p_envp, &envp_objv, &envc);
			if unlikely(!spawn_args.usa_envp)
				goto err_argv;
		} else {
			spawn_args.usa_envp = NULL;
			envp_objv           = NULL;
			envc                = 0;
		}

		/* Load requested standard file handles. */
		{
			unsigned int i;
			for (i = 0; i < COMPILER_LENOF(stdfd_files); ++i) {
				spawn_args.usa_stdfds[i] = ipc_unix_get_fd_for_exec(self, i, &stdfd_files[i]);
				if unlikely(spawn_args.usa_stdfds[i] == -1) {
					Dee_XDecrefv(stdfd_files, i);
					goto err_argv_envp;
				}
			}
		}

		/* Load the child process working directory */
		if (self->p_pwd != NULL) {
#ifdef HAVE_ipc_unix_spawn_pwd_fd
			if (!DeeString_Check(self->p_pwd)) {
				spawn_args.usa_pwdfd = DeeUnixSystem_GetFD(self->p_pwd);
				if unlikely(spawn_args.usa_pwdfd == -1)
					goto err_argv_envp_stdfd_files;
				pwd_obj = NULL;
			} else
#endif /* HAVE_ipc_unix_spawn_pwd_fd */
			{
#define WANT_DeeObject_AsFileSystemPathString
				pwd_obj = DeeObject_AsFileSystemPathString(self->p_pwd);
				if unlikely(!pwd_obj)
					goto err_argv_envp_stdfd_files;
				spawn_args.usa_pwd = (ipc_unix_chdir_char_t *)DeeString_As_ipc_unix_chdir_char_t((DeeObject *)pwd_obj);
				if unlikely(!spawn_args.usa_pwd)
					goto err_argv_envp_stdfd_files_pwd;
#ifdef HAVE_ipc_unix_spawn_pwd_fd
				spawn_args.usa_pwdfd = -1;
#endif /* HAVE_ipc_unix_spawn_pwd_fd */
			}
		} else {
			pwd_obj              = NULL;
			spawn_args.usa_pwd   = NULL;
#ifdef HAVE_ipc_unix_spawn_pwd_fd
			spawn_args.usa_pwdfd = -1;
#endif /* HAVE_ipc_unix_spawn_pwd_fd */
		}

		/* Configure posix_spawn file actions to our liking. */
#ifdef ipc_Process_USE_posix_spawn
		{
			int init_error;
			init_error = unix_spawn_args_init_posix_spawn(&spawn_args);
			if unlikely(init_error != 0) {
				DeeUnixSystem_ThrowErrorf(NULL, init_error, "Failed to initialize posix_spawn structures");
				goto err_argv_envp_stdfd_files_pwd;
			}
		}
#endif /* ipc_Process_USE_posix_spawn */

		/* Check for special case: trying to execute a file descriptor. */
#ifdef HAVE_ipc_unix_spawn_exe_fd
		if (!DeeString_Check(self->p_exe)) {
			/* Special case for `fexecve()' / `posix_fspawn_np()' */
			spawn_args.usa_exefd = DeeUnixSystem_GetFD(self->p_exe);
			if unlikely(spawn_args.usa_exefd == -1)
				goto err_argv_envp_stdfd_files_pwd_posix;
			cpid = ipc_unix_spawn(&spawn_args);
			if (cpid < 0)
				goto handle_cpid_fallback__err_argv_envp_stdfd_files_pwd_posix;
		} else
#endif /* HAVE_ipc_unix_spawn_exe_fd */
		{
			DREF DeeStringObject *exe_str;

			/* Form the name of the executable that should be executed. */
#ifndef HAVE_ipc_unix_spawn_exe_fd
#define WANT_DeeObject_AsFileSystemPathString
			exe_str = DeeObject_AsFileSystemPathString(self->p_exe);
			if unlikely(!exe_str)
				goto err_argv_envp_stdfd_files_pwd_posix;
			{
				DREF DeeStringObject *resolved_exe_str;
				resolved_exe_str = ipc_exe2path(exe_str);
				Dee_Decref(exe_str);
				exe_str = resolved_exe_str;
			}
#else /* HAVE_ipc_unix_spawn_exe_fd */
			spawn_args.usa_exefd = -1; /* We don't have an exec-fd */
			exe_str = ipc_exe2path((DeeStringObject *)self->p_exe);
#endif /* HAVE_ipc_unix_spawn_exe_fd */
			if unlikely(!exe_str)
				goto err_argv_envp_stdfd_files_pwd_posix;
			spawn_args.usa_exe = (ipc_unix_exec_char_t *)DeeString_As_ipc_unix_exec_char_t((DeeObject *)exe_str);
			if unlikely(!spawn_args.usa_exe) {
				Dee_Decref(exe_str);
				goto err_argv_envp_stdfd_files_pwd_posix;
			}

			/* Do the actual spawn! */
			cpid = ipc_unix_spawn(&spawn_args);
			Dee_DPRINTF("ipc_unix_spawn returned: %d\n", cpid);
			Dee_Decref(exe_str);
			if (cpid < 0) {
				DeeSystem_IF_E1(-cpid, ENOENT, {
					DeeUnixSystem_ThrowErrorf(NULL, ENOENT, "Application %r could not be found", exe_str);
					goto err_argv_envp_stdfd_files_pwd_posix;
				});
				goto handle_cpid_fallback__err_argv_envp_stdfd_files_pwd_posix;
			}
		}

		/* Error handling... */
		__IF0 {
handle_cpid_fallback__err_argv_envp_stdfd_files_pwd_posix:
			DeeUnixSystem_ThrowErrorf(NULL, -cpid, "Failed to create process");
err_argv_envp_stdfd_files_pwd_posix:
#ifdef ipc_Process_USE_posix_spawn
			(void)posix_spawnattr_destroy(&spawn_args.usa_spawn_attr);
			(void)posix_spawn_file_actions_destroy(&spawn_args.usa_spawn_file_actions);
#endif /* ipc_Process_USE_posix_spawn */
err_argv_envp_stdfd_files_pwd:
			Dee_XDecref(pwd_obj);
err_argv_envp_stdfd_files:
			Dee_XDecrefv(stdfd_files, COMPILER_LENOF(stdfd_files));
err_argv_envp:
			if (spawn_args.usa_envp)
				ipc_unix_free_string_array(spawn_args.usa_envp, envp_objv, envc);
err_argv:
			ipc_unix_free_string_array(spawn_args.usa_argv, argv_objv, argc);
			goto err;
		}

		/* Cleanup */
#ifdef ipc_Process_USE_posix_spawn
		(void)posix_spawnattr_destroy(&spawn_args.usa_spawn_attr);
		(void)posix_spawn_file_actions_destroy(&spawn_args.usa_spawn_file_actions);
#endif /* ipc_Process_USE_posix_spawn */
		Dee_XDecref(pwd_obj);
		Dee_XDecrefv(stdfd_files, COMPILER_LENOF(stdfd_files));
		if (spawn_args.usa_envp)
			ipc_unix_free_string_array(spawn_args.usa_envp, envp_objv, envc);
		ipc_unix_free_string_array(spawn_args.usa_argv, argv_objv, argc);

		/* Remember that the process has now been started. */
		Process_LockWrite(self);
		self->p_pid = cpid;
		self->p_state |= PROCESS_FLAG_STARTED;
		Process_LockEndWrite(self);
		return 0;
	}
#endif /* ipc_Process_USE_UNIX_APIS */

	__builtin_unreachable();
err:
	/* Clear the starting-flag to indicate that we're no longer trying to start the process. */
	Process_LockWrite(self);
	self->p_state &= ~PROCESS_FLAG_STARTING;
	Process_LockEndWrite(self);
	return -1;
#endif /* !ipc_Process_USE_STUB */
}


PRIVATE ATTR_COLD NONNULL((1)) int DCALL
ipc_throw_process_not_started_error(Process *__restrict self) {
	return DeeError_Throwf(&DeeError_ValueError, "Process %k has not been started", self);
}

#ifdef ipc_Process_USE_CreateProcessW
PRIVATE ATTR_COLD int DCALL
ipc_nt_throw_process_access_error(DWORD dwPid) {
	return DeeNTSystem_ThrowErrorf(NULL, GetLastError(),
	                               "Unable to access process with id %" PRFu32,
	                               dwPid);
}

/* Return the handle of `self', or `INVALID_HANDLE_VALUE' alongside an error */
PRIVATE WUNUSED NONNULL((1)) HANDLE DCALL
process_nt_gethandle_or_unlock(Process *__restrict self) {
	HANDLE hResult = self->p_handle;
	if (hResult == INVALID_HANDLE_VALUE) {
		if (self->p_state & PROCESS_FLAG_SELF) {
			/* Lazily load handle for current process */
			hResult = GetCurrentProcess();
			self->p_handle = hResult;
		} else if (self->p_pid != ipc_Process_pid_t_INVALID) {
			/* Lazily load handle for foreign process.
			 * Try with as many rights as possible first, then go down from there. */
			hResult = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_TERMINATE, FALSE, self->p_pid);
			if (hResult == NULL || hResult == INVALID_HANDLE_VALUE)
				hResult = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, self->p_pid);
			if (hResult == NULL || hResult == INVALID_HANDLE_VALUE)
				hResult = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, self->p_pid);
			if (hResult == NULL || hResult == INVALID_HANDLE_VALUE) {
				DWORD dwPid = self->p_pid;
				Process_LockEndWrite(self);
				ipc_nt_throw_process_access_error(dwPid);
				return INVALID_HANDLE_VALUE;
			}
			self->p_handle = hResult;
		} else {
			Process_LockEndWrite(self);
			ipc_throw_process_not_started_error(self);
			return INVALID_HANDLE_VALUE;
		}
	}
	return hResult;
}

PRIVATE WUNUSED NONNULL((1)) HANDLE DCALL
process_nt_gethandle_upgrade_or_unlock(Process *__restrict self, DWORD dwDesiredAccess) {
	HANDLE hResult;
	if (self->p_handle == INVALID_HANDLE_VALUE) {
		/* Special case: calling process -> the symbolic handle always has all rights. */
		if (self->p_state & PROCESS_FLAG_SELF) {
			hResult = GetCurrentProcess();
			ASSERT(hResult != INVALID_HANDLE_VALUE);
			self->p_handle = hResult;
			return hResult;
		}
		/* Special case: not-yet-started child process. */
		if (!(self->p_state & PROCESS_FLAG_EXTERN)) {
			Process_LockEndWrite(self);
			ipc_throw_process_not_started_error(self);
			return INVALID_HANDLE_VALUE;
		}
	}
	hResult = OpenProcess(dwDesiredAccess | SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_TERMINATE, FALSE, self->p_pid);
	if (hResult == NULL || hResult == INVALID_HANDLE_VALUE)
		hResult = OpenProcess(dwDesiredAccess | SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, self->p_pid);
	if (hResult == NULL || hResult == INVALID_HANDLE_VALUE)
		hResult = OpenProcess(dwDesiredAccess | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, self->p_pid);
	if (hResult == NULL || hResult == INVALID_HANDLE_VALUE)
		hResult = OpenProcess(dwDesiredAccess | SYNCHRONIZE, FALSE, self->p_pid);
	if (hResult == NULL || hResult == INVALID_HANDLE_VALUE)
		hResult = OpenProcess(dwDesiredAccess, FALSE, self->p_pid);
	if (hResult == NULL || hResult == INVALID_HANDLE_VALUE) {
		DWORD dwPid = self->p_pid;
		Process_LockEndWrite(self);
		ipc_nt_throw_process_access_error(dwPid);
		return INVALID_HANDLE_VALUE;
	}
	if (self->p_handle != INVALID_HANDLE_VALUE)
		CloseHandle(self->p_handle);
	self->p_handle = hResult;
	return hResult;
}
#endif /* ipc_Process_USE_CreateProcessW */

#ifdef ipc_Process_pid_t
/* Return the ID of `self', or `ipc_Process_pid_t_INVALID' alongside unlocking `self' and an error */
INTDEF WUNUSED NONNULL((1)) ipc_Process_pid_t DCALL
process_getpid_or_unlock(Process *__restrict self);

/* Return the ID of `self', or `ipc_Process_pid_t_INVALID' alongside an error */
PRIVATE WUNUSED NONNULL((1)) ipc_Process_pid_t DCALL
process_getpid(Process *__restrict self) {
	ipc_Process_pid_t result;
	Process_LockRead(self);
	result = self->p_pid;
	if (result == ipc_Process_pid_t_INVALID) {
#ifdef ipc_getpid
		if (self->p_state & PROCESS_FLAG_SELF) {
			/* Lazily load ID of current process */
			result = ipc_getpid();
			self->p_pid = result;
		} else
#endif /* ipc_getpid */
#ifdef ipc_Process_USE_CreateProcessW
		if (self->p_handle != INVALID_HANDLE_VALUE) {
			/* Lazily load handle for foreign process */
			if (!Process_LockUpgrade(self)) {
				result = self->p_pid;
				if unlikely(result != ipc_Process_pid_t_INVALID) {
					Process_LockEndWrite(self);
					return result;
				}
				if unlikely(self->p_handle == INVALID_HANDLE_VALUE) {
					Process_LockEndWrite(self);
#define NEED_err_no_handle
					goto err_no_handle;
				}
			}
			self->p_pid = GetProcessId(self->p_handle);
			if (self->p_pid == 0) {
				HANDLE hProcess = self->p_handle;
				DWORD dwError   = GetLastError();
				Process_LockEndWrite(self);
				DeeNTSystem_ThrowErrorf(NULL, dwError,
				                        "Unable to get PID of process with handle %p",
				                        hProcess);
				return ipc_Process_pid_t_INVALID;
			}
			Process_LockEndWrite(self);
		} else
#endif /* ipc_Process_USE_CreateProcessW */
		{
			Process_LockEndRead(self);
#ifdef NEED_err_no_handle
#undef NEED_err_no_handle
err_no_handle:
#endif /* NEED_err_no_handle */
			ipc_throw_process_not_started_error(self);
			return ipc_Process_pid_t_INVALID;
		}
	}
	Process_LockEndRead(self);
	return result;
}
#endif /* ipc_Process_pid_t */

/* @return: 1:  Not yet terminated
 * @return: 0:  Has already terminated
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
process_hasterminated_impl(Process *__restrict self) {
	Process_LockWrite(self);
	if (self->p_state & PROCESS_FLAG_SELF) {
		Process_LockEndWrite(self);
		return 1;
	}
	if (self->p_state & PROCESS_FLAG_TERMINATED) {
		Process_LockEndWrite(self);
		/* Already terminated. */
		return 0;
	}

#ifdef ipc_Process_USE_CreateProcessW
	{
		DWORD dwExitCode;
		HANDLE hProcess;
		hProcess = process_nt_gethandle_or_unlock(self);
		if unlikely(hProcess == INVALID_HANDLE_VALUE)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		if (!GetExitCodeProcess(hProcess, &dwExitCode)) {
			DBG_ALIGNMENT_ENABLE();
			hProcess = process_nt_gethandle_upgrade_or_unlock(self, PROCESS_QUERY_LIMITED_INFORMATION);
			if unlikely(hProcess == INVALID_HANDLE_VALUE)
				goto err;
			DBG_ALIGNMENT_DISABLE();
			if (!GetExitCodeProcess(hProcess, &dwExitCode)) {
				DWORD dwPid;
				DBG_ALIGNMENT_ENABLE();
				dwPid = self->p_pid;
				Process_LockEndWrite(self);
				ipc_nt_throw_process_access_error(dwPid);
				goto err;
			}
			DBG_ALIGNMENT_ENABLE();
		}
		DBG_ALIGNMENT_ENABLE();
		Process_LockEndWrite(self);
		return dwExitCode == STILL_ACTIVE ? 1 : 0;
	}
err:
	return -1;
#elif defined(ipc_Process_USE_UNIX_APIS) && defined(CONFIG_HAVE_kill)
	{
#define WANT_process_getpid_or_unlock
		pid_t pid = process_getpid_or_unlock(self);
		if unlikely(pid == ipc_Process_pid_t_INVALID)
			goto err;
		if (kill(pid, 0) != 0) {
			int error = errno;
#ifdef ESRCH
			if (error == ESRCH)
				return 0;
#endif /* ESRCH */
			Process_LockEndWrite(self);
			DeeUnixSystem_ThrowErrorf(NULL, error, "Unable to access process with id %d", pid);
			goto err;
		}
	}
	Process_LockEndWrite(self);
	return 1;
err:
	return -1;
#else /* ... */
	Process_LockEndWrite(self);
	return ipc_unimplemented();
#endif /* !... */
}

/* @return: 1:  Already killed
 * @return: 0:  Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
process_kill_impl(Process *__restrict self, int exit_code, int signo) {
	Process_LockWrite(self);
	if (self->p_state & PROCESS_FLAG_SELF) {
		Process_LockEndWrite(self);
		return Dee_Exit(exit_code, false);
	}
	if (self->p_state & PROCESS_FLAG_TERMINATED) {
		Process_LockEndWrite(self);
		/* Already terminated. */
		return 1;
	}

#ifdef ipc_Process_USE_CreateProcessW
	{
		HANDLE hProcess;
		(void)signo;
		hProcess = process_nt_gethandle_or_unlock(self);
		if unlikely(hProcess == INVALID_HANDLE_VALUE)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		if (TerminateProcess(hProcess, exit_code)) {
			DBG_ALIGNMENT_ENABLE();
		} else {
			DWORD dwExitCode;
			DWORD dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (dwError == ERROR_ACCESS_DENIED) {
				/* Quick check if the process has already exited on its own. */
				DBG_ALIGNMENT_DISABLE();
				if (GetExitCodeProcess(hProcess, &dwExitCode) && dwExitCode != STILL_ACTIVE) {
					DBG_ALIGNMENT_ENABLE();
					goto ok;
				}
				DBG_ALIGNMENT_ENABLE();
	
				/* Try again, but this time with `PROCESS_TERMINATE' access. */
				hProcess = process_nt_gethandle_upgrade_or_unlock(self, PROCESS_TERMINATE);
				if unlikely(hProcess == INVALID_HANDLE_VALUE)
					goto err;
				DBG_ALIGNMENT_DISABLE();
				if (TerminateProcess(hProcess, exit_code)) {
					DBG_ALIGNMENT_ENABLE();
					goto ok;
				}
				dwError = GetLastError();
				DBG_ALIGNMENT_ENABLE();
	
				/* Check for case: The process may have already terminated on its own. */
				if (dwError == ERROR_ACCESS_DENIED) {
					DBG_ALIGNMENT_DISABLE();
					if (!GetExitCodeProcess(hProcess, &dwExitCode)) {
						DBG_ALIGNMENT_ENABLE();
						hProcess = process_nt_gethandle_upgrade_or_unlock(self, PROCESS_QUERY_LIMITED_INFORMATION);
						if unlikely(hProcess == INVALID_HANDLE_VALUE)
							goto err;
						DBG_ALIGNMENT_DISABLE();
						if (!GetExitCodeProcess(hProcess, &dwExitCode)) {
							DWORD dwPid;
							DBG_ALIGNMENT_ENABLE();
							dwPid = self->p_pid;
							Process_LockEndWrite(self);
							ipc_nt_throw_process_access_error(dwPid);
							goto err;
						}
					}
					DBG_ALIGNMENT_ENABLE();
					if (exit_code != STILL_ACTIVE)
						goto ok;
				}
			}
			Process_LockEndWrite(self);
			DeeNTSystem_ThrowErrorf(NULL, dwError, "Failed to terminate process %k", self);
			goto err;
		}
	}
ok:
	/* Set the did-terminate flag on success. */
	self->p_state |= PROCESS_FLAG_TERMINATED;
	Process_LockEndWrite(self);
	return 0;
err:
	return -1;
#elif defined(ipc_Process_USE_UNIX_APIS) && defined(CONFIG_HAVE_kill)
	{
#define WANT_process_getpid_or_unlock
		pid_t pid = process_getpid_or_unlock(self);
		if unlikely(pid == ipc_Process_pid_t_INVALID)
			goto err;
		(void)exit_code;
		if (kill(pid, signo) != 0) {
			int error = errno;
#ifdef ESRCH
			if (error != ESRCH)
#endif /* ESRCH */
			{
				Process_LockEndWrite(self);
				DeeUnixSystem_ThrowErrorf(NULL, error, "Failed to terminate process %k", self);
				goto err;
			}
		}
	}

	/* Set the did-terminate flag on success. */
	self->p_state |= PROCESS_FLAG_TERMINATED;
	Process_LockEndWrite(self);
err:
	return -1;
#else /* ... */
	(void)exit_code;
	(void)signo;
	Process_LockEndWrite(self);
	return ipc_unimplemented();
#endif /* !... */
}




/* @return: 2:  Timeout expired or interrupted
 * @return: 1:  Already joined (*p_status is left undefined)
 * @return: 0:  Process joined (*p_status is filled with the exit status)
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
process_join_impl2(Process *__restrict self,
                   uint64_t timeout_nanoseconds,
                   int *__restrict p_status) {
	Process_LockWrite(self);
	if (self->p_state & PROCESS_FLAG_DETACHED) {
		Process_LockEndWrite(self);
		/* Already joined or detached. */
		return 1;
	}
	if (self->p_state & PROCESS_FLAG_JOINING) {
		Process_LockEndWrite(self);
		/* Wait for another process to finish joining */
		if (timeout_nanoseconds != 0) {
			uint64_t timeout_micro;
			timeout_micro = timeout_nanoseconds;
			timeout_micro /= 1000;
			timeout_micro /= 2;
			if (timeout_micro > 1000)
				timeout_micro = 1000;
			if (DeeThread_Sleep(1000))
				goto err; /* Error (interrupt delivery) */
		}
		return 2;
	}

	/* Set the joining flag. */
	self->p_state |= PROCESS_FLAG_JOINING;

#ifdef ipc_Process_USE_CreateProcessW
	{
		DWORD dwWaitState, dwTimeout, dwExitCode;
		HANDLE hProcess;
		hProcess = process_nt_gethandle_or_unlock(self);
		if unlikely(hProcess == INVALID_HANDLE_VALUE)
			goto err_stopjoin;
		Process_LockEndWrite(self);
		if (timeout_nanoseconds == 0) {
			dwTimeout = 0;
		} else if (timeout_nanoseconds == (uint64_t)-1) {
			dwTimeout = INFINITE;
		} else {
			uint64_t timeout_microseconds = timeout_nanoseconds / 1000;
			if (OVERFLOW_UCAST(timeout_microseconds, &dwTimeout))
				dwTimeout = INFINITE - 1;
		}
		DBG_ALIGNMENT_DISABLE();
		dwWaitState = WaitForMultipleObjectsEx(1, &hProcess, FALSE, dwTimeout, TRUE);
		DBG_ALIGNMENT_ENABLE();
		if (dwWaitState == WAIT_FAILED) {
			Process_LockWrite(self);
			hProcess = process_nt_gethandle_upgrade_or_unlock(self, SYNCHRONIZE);
			if unlikely(hProcess == INVALID_HANDLE_VALUE)
				goto err_stopjoin;
			Process_LockEndWrite(self);
			DBG_ALIGNMENT_DISABLE();
			dwWaitState = WaitForMultipleObjectsEx(1, &hProcess, FALSE, dwTimeout, TRUE);
			DBG_ALIGNMENT_ENABLE();
		}
		switch (dwWaitState) {

		case WAIT_IO_COMPLETION:
		case WAIT_TIMEOUT:
			/* Interrupt or timeout */
			goto interrupted_stopjoin;

		case WAIT_FAILED: {
			DWORD dwError; /* Error */
			DBG_ALIGNMENT_DISABLE();
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			DeeNTSystem_ThrowErrorf(NULL, dwError, "Failed to join process %k", self);
			goto err_stopjoin;
		}	break;

		default: break;
		}
		if (!GetExitCodeProcess(hProcess, &dwExitCode)) {
			dwExitCode = 0;
			hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, self->p_pid);
			if (hProcess != NULL && hProcess != INVALID_HANDLE_VALUE) {
				if (!GetExitCodeProcess(hProcess, &dwExitCode))
					dwExitCode = 0;
				CloseHandle(hProcess);
			}
		}
		if unlikely(dwExitCode == STILL_ACTIVE)
			goto interrupted_stopjoin; /* Shouldn't happen */
		*p_status = (int)dwExitCode;
	}

	/* Set the did-terminate flag on success. */
	Process_LockWrite(self);
	self->p_state |= PROCESS_FLAG_TERMINATED | PROCESS_FLAG_DETACHED;
	Process_LockEndWrite(self);
	return 0;
interrupted_stopjoin:
	Process_LockWrite(self);
	self->p_state &= ~PROCESS_FLAG_JOINING;
	Process_LockEndWrite(self);
	if (DeeThread_CheckInterrupt())
		goto err;
	return 2;
#elif defined(ipc_Process_USE_UNIX_APIS) && defined(ipc_joinpid)
	{
		int error;
#define WANT_process_getpid_or_unlock
		pid_t pid = process_getpid_or_unlock(self);
		if unlikely(pid == ipc_Process_pid_t_INVALID)
			goto err;
		Process_LockEndWrite(self);

		/* TODO: Alternative implementation that polls for has-exited
		 *       by testing with `kill(pid, 0) != -1 || errno != ESRCH' */
		(void)timeout_nanoseconds;
#ifdef ipc_tryjoinpid
		if (timeout_nanoseconds != (uint64_t)-1) {
			error = ipc_tryjoinpid(self->p_pid, p_status);
		} else
#endif /* ipc_tryjoinpid */
		{
			error = ipc_joinpid(self->p_pid, p_status);
		}
	
		/* From `man 2 waitid':
		 * """
		 * waitid(): returns 0 on success or if WNOHANG was specified
		 * and no child(ren) specified by id has yet changed state.
		 * """ */
#ifdef ipc_tryjoinpid
		if (error == 0) {
#ifdef EAGAIN
do_handle_EAGAIN:
#endif /* EAGAIN */
			/* WNOHANG -> process hasn't terminated, yet. */
			Process_LockWrite(self);
			self->p_state &= ~PROCESS_FLAG_JOINING;
			Process_LockEndWrite(self);
			if (timeout_nanoseconds != 0) {
				uint64_t timeout_micro;
				timeout_micro = timeout_nanoseconds;
				timeout_micro /= 1000;
				timeout_micro /= 2;
				if (timeout_micro > 1000)
					timeout_micro = 1000;
				if (DeeThread_Sleep(1000))
					goto err; /* Error (interrupt delivery) */
			}
			return 2;
		}
#endif /* ipc_tryjoinpid */

		/* Check for error. */
		if (error < 0) {
			error = errno;
#if defined(ipc_tryjoinpid) && defined(EAGAIN)
			if (error == EAGAIN)
				goto do_handle_EAGAIN;
#endif /* ipc_tryjoinpid && EAGAIN */
#ifdef EINTR
			if (error == EINTR) {
				Process_LockWrite(self);
				self->p_state &= ~PROCESS_FLAG_JOINING;
				Process_LockEndWrite(self);
				if (DeeThread_CheckInterrupt())
					goto err;
				return 2;
			}
#endif /* EINTR */
			DeeUnixSystem_ThrowErrorf(NULL, error, "Failed to join process %k", self);
			goto err_stopjoin;
		}
	}

	/* Set the did-terminate flag on success. */
	Process_LockWrite(self);
	self->p_state |= PROCESS_FLAG_TERMINATED | PROCESS_FLAG_DETACHED;
	Process_LockEndWrite(self);
	return 0;
#else /* ... */

	/* Not supported. */
	Process_LockEndWrite(self);
	(void)self;
	(void)timeout_nanoseconds;
	(void)p_status;
	ipc_unimplemented();
	goto err_stopjoin;
#endif /* !... */

err_stopjoin:
	Process_LockWrite(self);
	self->p_state &= ~PROCESS_FLAG_JOINING;
	Process_LockEndWrite(self);
err:
	return -1;
}


/* @return: 2:  Timeout expired
 * @return: 1:  Already joined (*p_status is left undefined)
 * @return: 0:  Process joined (*p_status is filled with the exit status)
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
process_join_impl(Process *__restrict self,
                  uint64_t timeout_nanoseconds,
                  int *__restrict p_status) {
	int result;
	if (timeout_nanoseconds == 0) {
		result = process_join_impl2(self, timeout_nanoseconds, p_status);
	} else if (timeout_nanoseconds == (uint64_t)-1) {
do_infinite_timeout:
		do {
			result = process_join_impl2(self, timeout_nanoseconds, p_status);
		} while (result == 2);
	} else {
		uint64_t now_microseconds, then_microseconds;
		now_microseconds = DeeThread_GetTimeMicroSeconds();
		if (OVERFLOW_UADD(now_microseconds, timeout_nanoseconds / 1000, &then_microseconds))
			goto do_infinite_timeout;
		for (;;) {
			result = process_join_impl2(self, timeout_nanoseconds, p_status);
			if (result != 2)
				break;
			now_microseconds = DeeThread_GetTimeMicroSeconds();
			if (OVERFLOW_USUB(then_microseconds, now_microseconds, &timeout_nanoseconds))
				return 2; /* Timeout */
			timeout_nanoseconds *= 1000;
		}
	}
	return result;
}


/* @return: 1:  Already detached
 * @return: 0:  Process was detached
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
process_detach_impl(Process *__restrict self) {
again:
	Process_LockWrite(self);
	if (self->p_state & PROCESS_FLAG_DETACHED) {
		Process_LockEndWrite(self);
		/* Already joined or detached. */
		return 1;
	}
	if (self->p_state & PROCESS_FLAG_JOINING) {
		Process_LockEndWrite(self);
		/* Wait for another process to finish joining */
		if (DeeThread_Sleep(1000))
			goto err; /* Error (interrupt delivery) */
		goto again;
	}

	/* Set flags to indicate a detached process. */
	self->p_state |= PROCESS_FLAG_JOINING | PROCESS_FLAG_DETACHED;
	Process_LockEndWrite(self);
	return 0;
err:
	return -1;
}


#ifdef WANT_process_getpid_or_unlock
#undef WANT_process_getpid_or_unlock
INTERN WUNUSED NONNULL((1)) ipc_Process_pid_t DCALL
process_getpid_or_unlock(Process *__restrict self) {
	ipc_Process_pid_t result;
	result = self->p_pid;
	if (result == ipc_Process_pid_t_INVALID) {
#ifdef ipc_getpid
		if (self->p_state & PROCESS_FLAG_SELF) {
			/* Lazily load handle for current process */
			result = ipc_getpid();
			self->p_pid = result;
		} else
#endif /* ipc_getpid */
#ifdef ipc_Process_USE_CreateProcessW
		if (self->p_handle != INVALID_HANDLE_VALUE) {
			/* Lazily load handle for foreign process */
			self->p_pid = GetProcessId(self->p_handle);
			if (self->p_pid == 0) {
				HANDLE hProcess = self->p_handle;
				DWORD dwError   = GetLastError();
				Process_LockEndWrite(self);
				DeeNTSystem_ThrowErrorf(NULL, dwError,
				                        "Unable to get PID of process with handle %p",
				                        hProcess);
				return ipc_Process_pid_t_INVALID;
			}
			Process_LockEndWrite(self);
		} else
#endif /* ipc_Process_USE_CreateProcessW */
		{
			Process_LockEndWrite(self);
			ipc_throw_process_not_started_error(self);
			return ipc_Process_pid_t_INVALID;
		}
	}
	return result;
}
#endif /* WANT_process_getpid_or_unlock */






PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_start(Process *self, size_t argc, DeeObject *const *argv) {
	int error;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("start");]]]*/
	DeeArg_Unpack0(err, argc, argv, "start");
/*[[[end]]]*/
	if (DeeThread_CheckInterrupt())
		goto err;
	error = process_start_impl(self);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_kill(Process *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int error;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("kill", params: """
	int exitcode = 0;
	int signo = 9;
""", defineKwList: true);]]]*/
	static DEFINE_KWLIST(kill_kwlist, { KEX("exitcode", 0x79c9d863, 0x3fcba2655ec7fa85), KEX("signo", 0xa7bcef88, 0xe5690ca4216c1f3b), KEND });
	struct {
		int exitcode;
		int signo;
	} args;
	args.exitcode = 0;
	args.signo = 9;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kill_kwlist, "|dd:kill", &args))
		goto err;
/*[[[end]]]*/
	if (DeeThread_CheckInterrupt())
		goto err;
	error = process_kill_impl(self, args.exitcode, args.signo);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
ipc_throw_process_already_joined(Process *__restrict self) {
	return DeeError_Throwf(&DeeError_ValueError, "Process %k has already been joined", self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_join(Process *self, size_t argc, DeeObject *const *argv) {
	int error, status;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("join");]]]*/
	DeeArg_Unpack0(err, argc, argv, "join");
/*[[[end]]]*/
	if (DeeThread_CheckInterrupt())
		goto err;
	error = process_join_impl(self, (uint64_t)-1, &status);
	if unlikely(error < 0)
		goto err;
	if unlikely(error != 0)
		goto err_already;
	return DeeInt_NewInt(status);
err_already:
	ipc_throw_process_already_joined(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_tryjoin(Process *self, size_t argc, DeeObject *const *argv) {
	int error, status;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("tryjoin");]]]*/
	DeeArg_Unpack0(err, argc, argv, "tryjoin");
/*[[[end]]]*/
	if (DeeThread_CheckInterrupt())
		goto err;
	error = process_join_impl(self, 0, &status);
	if unlikely(error < 0)
		goto err;
	if (error != 0) {
		if (error == 2)
			return_none; /* Timeout */
		goto err_already;
	}
	return DeeInt_NewInt(status);
err_already:
	ipc_throw_process_already_joined(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_timedjoin(Process *self, size_t argc, DeeObject *const *argv) {
	int error, status;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("timedjoin", params: """
	uint64_t timeout_nanoseconds
""");]]]*/
	struct {
		uint64_t timeout_nanoseconds;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "timedjoin", &args.timeout_nanoseconds, UNPu64, DeeObject_AsUInt64);
/*[[[end]]]*/
	if (DeeThread_CheckInterrupt())
		goto err;
	error = process_join_impl(self, args.timeout_nanoseconds, &status);
	if unlikely(error < 0)
		goto err;
	if (error != 0) {
		if (error == 2)
			return_none; /* Timeout */
		goto err_already;
	}
	return DeeInt_NewInt(status);
err_already:
	ipc_throw_process_already_joined(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_detach(Process *self, size_t argc, DeeObject *const *argv) {
	int error;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("detach");]]]*/
	DeeArg_Unpack0(err, argc, argv, "detach");
/*[[[end]]]*/
	if (DeeThread_CheckInterrupt())
		goto err;
	error = process_detach_impl(self);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_hasterminated(Process *__restrict self) {
	int error = process_hasterminated_impl(self);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_id(Process *__restrict self) {
#ifdef ipc_Process_pid_t
	ipc_Process_pid_t pid = process_getpid(self);
	if unlikely(pid == ipc_Process_pid_t_INVALID)
		goto err;
	return DeeInt_NEWU(pid);
err:
	return NULL;
#else /* ipc_Process_pid_t */
	(void)self;
#define WANT_ipc_unimplemented
	ipc_unimplemented();
	return NULL;
#endif /* !ipc_Process_pid_t */
}

#ifdef ipc_Process_USE_CreateProcessW
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_osfhandle_np(Process *__restrict self) {
	HANDLE hProcess;
	Process_LockWrite(self);
	hProcess = process_nt_gethandle_or_unlock(self);
	if unlikely(hProcess == INVALID_HANDLE_VALUE)
		goto err;
	Process_LockEndWrite(self);
	return DeeInt_NewUIntptr((uintptr_t)hProcess);
err:
	return NULL;
}
#endif /* ipc_Process_USE_CreateProcessW */



/* Misc. NT helper functions. */
#ifdef ipc_Process_USE_CreateProcessW
PRIVATE WUNUSED DREF DeeObject *DCALL
ipc_nt_GetModuleFileName(HANDLE hModule) {
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize = PATH_MAX, dwError;
	lpBuffer        = DeeString_NewWideBuffer(dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
again:
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = GetModuleFileNameW((HMODULE)hModule, lpBuffer, dwBufSize + 1);
		if (!dwError) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_buffer;
				goto again;
			}
			if (DeeNTSystem_IsBufferTooSmall(dwError))
				goto do_increase_buffer;
			DeeString_FreeWideBuffer(lpBuffer);
			DeeNTSystem_ThrowErrorf(NULL, dwError,
			                        "Failed to determine name of module %p",
			                        hModule);
			goto err;
		}
		DBG_ALIGNMENT_ENABLE();
		if (dwError <= dwBufSize) {
			if (dwError < dwBufSize)
				break;
			DBG_ALIGNMENT_DISABLE();
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (!DeeNTSystem_IsBufferTooSmall(dwError))
				break;
		}
		/* Increase buffer size. */
do_increase_buffer:
		dwBufSize *= 2;
		lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer, dwBufSize);
		if unlikely(!lpNewBuffer)
			goto err_buffer;
		lpBuffer = lpNewBuffer;
	}
	lpBuffer = DeeString_TruncateWideBuffer(lpBuffer, dwError);
	return DeeString_PackWideBuffer(lpBuffer, STRING_ERROR_FREPLAC);
err_buffer:
	DeeString_FreeWideBuffer(lpBuffer);
err:
	return NULL;
}

typedef BOOL (WINAPI *LPQUERYFULLPROCESSIMAGENAMEW)(HANDLE hProcess, DWORD dwFlags, LPWSTR lpExeName, PDWORD lpdwSize);
PRIVATE LPQUERYFULLPROCESSIMAGENAMEW pQueryFullProcessImageNameW = NULL;
PRIVATE WCHAR const str_KERNEL32[] = { 'K', 'E', 'R', 'N', 'E', 'L', '3', '2', 0 };

#ifdef _WIN32_WCE
#undef GetProcAddress
#define GetProcAddress GetProcAddressA
#endif /* _WIN32_WCE */

/* @return: * : Success
 * @return: NULL: Error
 * @return: ITER_DONE: Not supported
 * @return: ITER_DONE: Not supported */
PRIVATE WUNUSED DREF DeeObject *DCALL
ipc_nt_QueryFullProcessImageName(HANDLE hProcess, DWORD dwFlags) {
	LPWSTR buffer, new_buffer;
	DWORD bufsize = 256;
	LPQUERYFULLPROCESSIMAGENAMEW func;
	func = pQueryFullProcessImageNameW;
	if (!func) {
		DBG_ALIGNMENT_DISABLE();
		*(FARPROC *)&func = GetProcAddress(GetModuleHandleW(str_KERNEL32),
		                                   "QueryFullProcessImageNameW");
		DBG_ALIGNMENT_ENABLE();
		if (!func)
			*(void **)&func = (void *)(uintptr_t)-1;
		pQueryFullProcessImageNameW = func;
		COMPILER_WRITE_BARRIER();
	}

	/* If the function is not supported, return ITER_DONE. */
	if (*(void **)&func == (void *)(uintptr_t)-1)
		return ITER_DONE;

	buffer = DeeString_NewWideBuffer(bufsize);
	if unlikely(!buffer)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	while (!(*func)(hProcess, dwFlags, buffer, &bufsize)) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		switch (dwError) {

		case ERROR_GEN_FAILURE:
			/* Generated for Zombie Processes */
			DeeString_FreeWideBuffer(buffer);
			return DeeString_NewEmpty();

		case ERROR_BUFFER_OVERFLOW:
		case ERROR_MORE_DATA:
		case ERROR_INSUFFICIENT_BUFFER:
			/* Increase the buffer size. */
			bufsize    = (DWORD)(WSTR_LENGTH(buffer) * 2);
			new_buffer = DeeString_ResizeWideBuffer(buffer, bufsize);
			if unlikely(!new_buffer)
				goto err_r;
			buffer = new_buffer;
			break;

		default:
			DeeNTSystem_ThrowErrorf(NULL, dwError,
			                        "Call to QueryFullProcessImageNameW(%p, %#" PRFx32 ") failed",
			                        hProcess, dwFlags);
			goto err_r;
		}
		DBG_ALIGNMENT_DISABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	buffer = DeeString_TruncateWideBuffer(buffer, bufsize);
	return DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
err_r:
	DeeString_FreeWideBuffer(buffer);
err:
	return NULL;
}
#endif /* ipc_Process_USE_CreateProcessW */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_exe(Process *__restrict self) {
	DREF DeeObject *result;
	Process_LockRead(self);

	/* Check if we've already cached the correct result. */
	result = self->p_exe;
	if (result) {
		Dee_Incref(result);
		Process_LockEndRead(self);
		goto done;
	}

	/* Make sure that the process has already started. */
	if (!(self->p_state & PROCESS_FLAG_STARTED)) {
		Process_LockEndRead(self);
		DeeRT_ErrTUnboundAttrCStr(&DeeProcess_Type, self, "exe");
		goto err;
	}

	/* Get a write-lock. */
	if (!Process_LockUpgrade(self)) {
		result = self->p_exe;
		if (result) {
			Dee_Incref(result);
			Process_LockEndWrite(self);
			goto done;
		}
	}

	/* OS-specific part... */
#ifdef ipc_Process_USE_CreateProcessW
	if (self->p_state & PROCESS_FLAG_SELF) {
		Process_LockEndWrite(self);
		result = ipc_nt_GetModuleFileName(NULL);
		if unlikely(!result)
			goto err;
		Dee_UntrackAlloc(result); /* Not a leak! */
	} else {
		HANDLE hProcess;
		hProcess = process_nt_gethandle_or_unlock(self);
		if unlikely(hProcess == INVALID_HANDLE_VALUE)
			goto err;
		Process_LockEndWrite(self);
		result = ipc_nt_QueryFullProcessImageName(hProcess, 0);
		if unlikely(!result)
			goto err;
		if (result == ITER_DONE) {
			/* TODO: Try to read from `RTL_USER_PROCESS_PARAMETERS' */
#define WANT_ipc_unimplemented
			ipc_unimplemented();
			goto err;
		}
	}
#elif defined(ipc_Process_USE_UNIX_APIS)
	{
		char exepath[sizeof("/proc/" PRFMAXdPID "/exe")];
#define WANT_process_getpid_or_unlock
		pid_t pid = process_getpid_or_unlock(self);
		if unlikely(pid == ipc_Process_pid_t_INVALID)
			goto err;
		Process_LockEndWrite(self);
		Dee_sprintf(exepath, "/proc/%" PRFdPID "/exe", pid);
		result = DeeUnixSystem_ReadLinkString(exepath);
		if unlikely(result == ITER_DONE) {
#define WANT_ipc_unimplemented
			ipc_unimplemented();
			goto err;
		}
#ifndef NDEBUG
		if (atomic_read(&self->p_state) & PROCESS_FLAG_SELF)
			Dee_UntrackAlloc(result); /* Not a leak! */
#endif /* !NDEBUG */
	}
#else /* ... */
	if (self->p_state & PROCESS_FLAG_SELF) {
		Process_LockEndWrite(self);
		result = (DREF DeeObject *)DeeModule_GetDeemon()->mo_name;
		Dee_Incref(result);
		/*Dee_UntrackAlloc(result);*/
	} else {
		Process_LockEndWrite(self);
		ipc_unimplemented();
		goto err;
	}
#endif /* !... */
	Process_LockWrite(self);
	if likely(!self->p_exe) {
		Dee_Incref(result);
		self->p_exe = result;
		Process_LockEndWrite(self);
	} else {
		DREF DeeObject *old_result;
		old_result = self->p_exe;
		Dee_Incref(old_result);
		Process_LockEndWrite(self);
		Dee_Decref_likely(result);
		result = old_result;
	}
done:
	return result;
err:
	return NULL;
}


PRIVATE ATTR_COLD NONNULL((1, 2)) int DCALL
err_cannot_set_attribute_for_running_process(Process *__restrict self,
                                             char const *__restrict attr) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Cannot set '%s' for running process %k",
	                       attr, self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_exe(Process *self, DeeObject *value) {
	DREF DeeObject *old_value;
	Process_LockWrite(self);
	if (self->p_state & (PROCESS_FLAG_STARTING | PROCESS_FLAG_STARTED)) {
		Process_LockEndWrite(self);
		goto err_running;
	}
	Dee_XIncref(value);
	old_value   = self->p_exe;
	self->p_exe = value;
	Process_LockEndWrite(self);
	if (old_value) {
		Dee_Decref(old_value);
	}
	return 0;
err_running:
	err_cannot_set_attribute_for_running_process(self, "exe");
#ifdef WANT_err
#undef WANT_err
err:
#endif /* WANT_err */
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_exe(Process *__restrict self) {
	return process_set_exe(self, NULL);
}


#ifdef ipc_Process_USE_UNIX_APIS
PRIVATE WUNUSED DREF DeeTupleObject *DCALL
ipc_unix_strings_from_nulterm_bytes(char const *__restrict data,
                                    size_t datalen) {
	DREF DeeTupleObject *result;
	char const *end = data + datalen;
	char const *iter;
	size_t i, num_strings;
	if unlikely(end[-1] != '\0') {
		while (end > data && end[-1] != '\0')
			--end;
	}
	if unlikely(data >= end) {
		result = (DREF DeeTupleObject *)DeeTuple_NewEmpty();
		goto done;
	}

	/* Count the # of NUL-characters that are embedded in `data'.
	 * This is the number of strings that we will need to return. */
	for (num_strings = 0, iter = data; iter < end; ++num_strings)
		iter = strend(iter) + 1;

	/* Package all of the strings together. */
	result = DeeTuple_NewUninitialized(num_strings);
	if unlikely(!result)
		goto err;
	for (i = 0, iter = data; i < num_strings; ++i) {
		DREF DeeObject *arg;
		size_t arg_len = strlen(iter);
		arg = DeeString_NewUtf8(iter, arg_len, STRING_ERROR_FIGNORE);
		if unlikely(!arg)
			goto err_result_i;
		DeeTuple_SET(result, i, arg);
		iter += arg_len + 1;
	}
done:
	return result;
err_result_i:
	Dee_Decrefv(DeeTuple_ELEM(result), i);
/*err_result:*/
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}
#endif /* ipc_Process_USE_UNIX_APIS */


#if defined(ipc_Process_USE_cmdline) && defined(ipc_Process_USE_CreateProcessW)
#ifndef CONFIG_HAVE_wcslen
#define CONFIG_HAVE_wcslen
#undef wcslen
#define wcslen dee_wcslen
DeeSystem_DEFINE_wcslen(dee_wcslen)
#endif /* !CONFIG_HAVE_wcslen */
#endif /* ipc_Process_USE_cmdline && ipc_Process_USE_CreateProcessW */ 


#ifdef ipc_Process_USE_cmdline
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_cmdline(Process *__restrict self)
#else /* ipc_Process_USE_cmdline */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_argv(Process *__restrict self)
#endif /* !ipc_Process_USE_cmdline */
{
	DREF DeeObject *result;
	Process_LockRead(self);

	/* Check if we've already cached the correct result. */
#ifdef ipc_Process_USE_cmdline
	result = (DREF DeeObject *)self->p_cmdline;
#else /* ipc_Process_USE_cmdline */
	result = self->p_argv;
#endif /* !ipc_Process_USE_cmdline */
	if (result) {
		Dee_Incref(result);
		Process_LockEndRead(self);
		goto done;
	}

	/* Make sure that the process has already started. */
	if (!(self->p_state & PROCESS_FLAG_STARTED)) {
		Process_LockEndRead(self);
		DeeRT_ErrTUnboundAttrCStr(&DeeProcess_Type, self, "argv");
		goto err;
	}

	/* Get a write-lock. */
	if (!Process_LockUpgrade(self)) {
#ifdef ipc_Process_USE_cmdline
		result = (DREF DeeObject *)self->p_cmdline;
#else /* ipc_Process_USE_cmdline */
		result = self->p_argv;
#endif /* !ipc_Process_USE_cmdline */
		if (result) {
			Dee_Incref(result);
			Process_LockEndWrite(self);
			goto done;
		}
	}

	/* OS-specific part... */
#ifdef ipc_Process_USE_CreateProcessW
#ifndef ipc_Process_USE_cmdline
#error "Unsupported configuration"
#endif /* !ipc_Process_USE_cmdline */
	if (self->p_state & PROCESS_FLAG_SELF) {
		LPWSTR lpwCommandLine;
		Process_LockEndWrite(self);
		lpwCommandLine = GetCommandLineW();
		if unlikely(!lpwCommandLine) {
			DeeNTSystem_ThrowErrorf(NULL, GetLastError(),
			                        "Failed to load process commandline");
			goto err;
		}
		result = DeeString_NewWide(lpwCommandLine,
		                           wcslen(lpwCommandLine),
		                           STRING_ERROR_FIGNORE);
		if unlikely(!result)
			goto err;
		Dee_UntrackAlloc(result); /* Not a leak! */
	} else {
		HANDLE hProcess;
		hProcess = process_nt_gethandle_or_unlock(self);
		if unlikely(hProcess == INVALID_HANDLE_VALUE)
			goto err;
		Process_LockEndWrite(self);
		/* TODO: Try to read from `RTL_USER_PROCESS_PARAMETERS' */
#define WANT_ipc_unimplemented
		ipc_unimplemented();
		goto err;
	}
#elif defined(ipc_Process_USE_UNIX_APIS)
#ifdef ipc_Process_USE_cmdline
#error "Unsupported configuration"
#endif /* ipc_Process_USE_cmdline */
	{
		DREF DeeObject *cmdline_file, *cmdline_content;
		char cmdline_path[sizeof("/proc/" PRFMAXdPID "/cmdline")];
#define WANT_process_getpid_or_unlock
		pid_t pid = process_getpid_or_unlock(self);
		if unlikely(pid == ipc_Process_pid_t_INVALID)
			goto err;
		Process_LockEndWrite(self);
		Dee_sprintf(cmdline_path, "/proc/%" PRFdPID "/cmdline", pid);
		cmdline_file = DeeFile_OpenString(cmdline_path, OPEN_FRDONLY, 0);
		if (!ITER_ISOK(cmdline_file)) {
#define WANT_err_file_not_found
			if (cmdline_file != NULL)
				err_file_not_found_string(cmdline_path);
			goto err;
		}
		cmdline_content = DeeFile_ReadBytes(cmdline_file, (size_t)-1, true);
		Dee_Decref(cmdline_file);
		if unlikely(!cmdline_content)
			goto err;
		result = (DREF DeeObject *)ipc_unix_strings_from_nulterm_bytes((char const *)DeeBytes_DATA(cmdline_content),
		                                                               DeeBytes_SIZE(cmdline_content));
		if unlikely(!result)
			goto err;
#ifndef NDEBUG
		if (atomic_read(&self->p_state) & PROCESS_FLAG_SELF)
			Dee_UntrackAlloc(result); /* Not a leak! */
#endif /* !NDEBUG */
	}
#else /* ... */
#ifdef ipc_Process_USE_cmdline
#error "Unsupported configuration"
#endif /* ipc_Process_USE_cmdline */
	Process_LockEndWrite(self);
	if (self->p_state & PROCESS_FLAG_SELF) {
		DeeObject *argv = Dee_GetArgv(), *name;
		result = DeeTuple_NewUninitialized(1 + DeeTuple_SIZE(argv));
		if unlikely(!result)
			goto err;
		name = (DREF DeeObject *)DeeModule_GetDeemon()->mo_name;
		Dee_Incref(name);
		DeeTuple_SET(result, 0, name);
		Dee_Movrefv(DeeTuple_ELEM(result) + 1, DeeTuple_ELEM(argv), DeeTuple_SIZE(argv));
#ifndef NDEBUG
		Dee_UntrackAlloc(result); /* Not a leak! */
#endif /* !NDEBUG */
	} else {
		ipc_unimplemented();
		goto err;
	}
#endif /* !... */
	Process_LockWrite(self);
#ifdef ipc_Process_USE_cmdline
	if likely(!self->p_cmdline)
#else /* ipc_Process_USE_cmdline */
	if likely(!self->p_argv)
#endif /* !ipc_Process_USE_cmdline */
	{
		Dee_Incref(result);
#ifdef ipc_Process_USE_cmdline
		self->p_cmdline = (DREF DeeStringObject *)result;
#else /* ipc_Process_USE_cmdline */
		self->p_argv = result;
#endif /* !ipc_Process_USE_cmdline */
		Process_LockEndWrite(self);
	} else {
		DREF DeeObject *old_result;
#ifdef ipc_Process_USE_cmdline
		old_result = (DREF DeeObject *)self->p_cmdline;
#else /* ipc_Process_USE_cmdline */
		old_result = self->p_argv;
#endif /* !ipc_Process_USE_cmdline */
		Dee_Incref(old_result);
		Process_LockEndWrite(self);
		Dee_Decref_likely(result);
		result = old_result;
	}
done:
	return result;
err:
	return NULL;
}

#ifdef ipc_Process_USE_cmdline
PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
process_get_argv(Process *__restrict self) {
	DREF DeeTupleObject *result;
	DREF DeeStringObject *cmdline;
	cmdline = (DREF DeeStringObject *)process_get_cmdline(self);
	if unlikely(!cmdline)
		goto err;
	result = ipc_cmdline2argv(cmdline);
	Dee_Decref(cmdline);
	return result;
err:
	return NULL;
}
#endif /* !ipc_Process_USE_cmdline */


#ifdef ipc_Process_USE_cmdline
PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_cmdline(Process *self, DeeObject *value)
#else /* ipc_Process_USE_cmdline */
PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_argv(Process *self, DeeObject *value)
#endif /* !ipc_Process_USE_cmdline */
{
	DREF DeeObject *old_value;
#ifdef ipc_Process_USE_cmdline
	if (value && DeeObject_AssertTypeExact(value, &DeeString_Type)) {
#define WANT_err
		goto err;
	}
#endif /* ipc_Process_USE_cmdline */
	Process_LockWrite(self);
	if (self->p_state & (PROCESS_FLAG_STARTING | PROCESS_FLAG_STARTED)) {
		Process_LockEndWrite(self);
		goto err_running;
	}
	Dee_XIncref(value);
#ifdef ipc_Process_USE_cmdline
	old_value       = (DREF DeeObject *)self->p_cmdline;
	self->p_cmdline = (DREF DeeStringObject *)value;
#else /* ipc_Process_USE_cmdline */
	old_value    = self->p_argv;
	self->p_argv = value;
#endif /* !ipc_Process_USE_cmdline */
	Process_LockEndWrite(self);
	Dee_XDecref(old_value);
	return 0;
err_running:
	err_cannot_set_attribute_for_running_process(self, "argv");
#ifdef WANT_err
#undef WANT_err
err:
#endif /* WANT_err */
	return -1;
}

#ifdef ipc_Process_USE_cmdline
PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_cmdline(Process *__restrict self) {
	return process_set_cmdline(self, NULL);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
process_set_argv(Process *self, DeeObject *value) {
	int result;
	DREF DeeStringObject *cmdline;
	cmdline = ipc_argv2cmdline(value);
	if unlikely(!cmdline)
		goto err;
	result = process_set_cmdline(self, (DeeObject *)cmdline);
	Dee_Decref(cmdline);
	return result;
err:
	return -1;
}

#define process_del_argv process_del_cmdline
#else /* !pc_Process_USE_cmdline */
PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_argv(Process *__restrict self) {
	return process_set_argv(self, NULL);
}
#endif /* !ipc_Process_USE_cmdline */



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_environ(Process *__restrict self) {
	DREF DeeObject *result;
	Process_LockRead(self);

	/* Check if we've already cached the correct result. */
	result = self->p_envp;
	if (result) {
		Dee_Incref(result);
		Process_LockEndRead(self);
		goto done;
	}

	/* Make sure that the process has already started. */
	if (!(self->p_state & PROCESS_FLAG_STARTED)) {
		Process_LockEndRead(self);
		DeeRT_ErrTUnboundAttrCStr(&DeeProcess_Type, self, "environ");
		goto err;
	}

	/* Get a write-lock. */
	if (!Process_LockUpgrade(self)) {
		result = self->p_envp;
		if (result) {
			Dee_Incref(result);
			Process_LockEndWrite(self);
			goto done;
		}
	}

	/* Special case when doing `Process.current.environ' (which just aliases `posix.environ') */
	if (self->p_state & PROCESS_FLAG_SELF) {
		Process_LockEndWrite(self);
		return DeeModule_GetExternString("posix", "environ");
	}

	/* OS-specific part... */
#ifdef ipc_Process_USE_CreateProcessW
	{
		HANDLE hProcess;
		hProcess = process_nt_gethandle_or_unlock(self);
		if unlikely(hProcess == INVALID_HANDLE_VALUE)
			goto err;
		Process_LockEndWrite(self);
		/* TODO: Try to read from `RTL_USER_PROCESS_PARAMETERS' */
#define WANT_ipc_unimplemented
		ipc_unimplemented();
		goto err;
	}
#elif defined(ipc_Process_USE_UNIX_APIS)
	{
		DREF DeeObject *environ_file, *environ_content;
		char environ_path[sizeof("/proc/" PRFMAXdPID "/environ")];
#define WANT_process_getpid_or_unlock
		pid_t pid = process_getpid_or_unlock(self);
		if unlikely(pid == ipc_Process_pid_t_INVALID)
			goto err;
		Process_LockEndWrite(self);
		Dee_sprintf(environ_path, "/proc/%" PRFdPID "/environ", pid);
		environ_file = DeeFile_OpenString(environ_path, OPEN_FRDONLY, 0);
		if (!ITER_ISOK(environ_file)) {
#define WANT_err_file_not_found
			if (environ_file != NULL)
				err_file_not_found_string(environ_path);
			goto err;
		}
		environ_content = DeeFile_ReadBytes(environ_file, (size_t)-1, true);
		Dee_Decref(environ_file);
		if unlikely(!environ_content)
			goto err;
		result = (DREF DeeObject *)ipc_unix_strings_from_nulterm_bytes((char const *)DeeBytes_DATA(environ_content),
		                                                               DeeBytes_SIZE(environ_content));
		if unlikely(!result)
			goto err;
	}
#else /* ... */
	Process_LockEndWrite(self);
	{
		ipc_unimplemented();
		goto err;
	}
#endif /* !... */

#if 0 /* Don't cache the environment blocks of external processes */
	Process_LockWrite(self);
	if likely(!self->p_envp) {
		Dee_Incref(result);
		self->p_envp = result;
		Process_LockEndWrite(self);
	} else {
		DREF DeeObject *old_result;
		old_result = self->p_envp;
		Dee_Incref(old_result);
		Process_LockEndWrite(self);
		Dee_Decref_likely(result);
		result = old_result;
	}
#endif
done:
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_environ(Process *self, DeeObject *value) {
	DREF DeeObject *old_value;
	Process_LockWrite(self);
	if (self->p_state & (PROCESS_FLAG_STARTING | PROCESS_FLAG_STARTED)) {
		if (self->p_state & PROCESS_FLAG_SELF) {
			int result;
			DREF DeeObject *posix_environ;
			Process_LockEndWrite(self);
			/* Special case: forward the request on-to `posix.environ' */
			if (value == NULL)
				value = Dee_EmptySeq; /* Clear environ */
			posix_environ = DeeModule_GetExternString("posix", "environ");
			if unlikely(!posix_environ)
				goto err;
			result = DeeObject_Assign(posix_environ, value);
			Dee_Decref(posix_environ);
			return result;
		}
		Process_LockEndWrite(self);
		goto err_running;
	}
	Dee_XIncref(value);
	old_value    = self->p_envp;
	self->p_envp = value;
	Process_LockEndWrite(self);
	Dee_XDecref(old_value);
	return 0;
err_running:
	err_cannot_set_attribute_for_running_process(self, "environ");
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_environ(Process *__restrict self) {
	return process_set_environ(self, NULL);
}




PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_pwd(Process *__restrict self) {
	DREF DeeObject *result;
	Process_LockRead(self);

	/* Check if we've already cached the correct result. */
	result = self->p_pwd;
	if (result) {
		Dee_Incref(result);
		Process_LockEndRead(self);
		goto done;
	}

	/* Make sure that the process has already started. */
	if (!(self->p_state & PROCESS_FLAG_STARTED)) {
		Process_LockEndRead(self);
		DeeRT_ErrTUnboundAttrCStr(&DeeProcess_Type, self, "pwd");
		goto err;
	}

	/* Get a write-lock. */
	if (!Process_LockUpgrade(self)) {
		result = self->p_pwd;
		if (result) {
			Dee_Incref(result);
			Process_LockEndWrite(self);
			goto done;
		}
	}

	/* Special case: current process */
	if (self->p_state & PROCESS_FLAG_SELF) {
		Process_LockEndWrite(self);
		return DeeModule_CallExternString("posix", "getcwd", 0, NULL);
	}

	/* OS-specific part... */
#ifdef ipc_Process_USE_CreateProcessW
	{
		HANDLE hProcess;
		hProcess = process_nt_gethandle_or_unlock(self);
		if unlikely(hProcess == INVALID_HANDLE_VALUE)
			goto err;
		Process_LockEndWrite(self);
		/* TODO: Try to read from `RTL_USER_PROCESS_PARAMETERS' */
#define WANT_ipc_unimplemented
		ipc_unimplemented();
		goto err;
	}
#elif defined(ipc_Process_USE_UNIX_APIS)
	{
		char cwdpath[sizeof("/proc/" PRFMAXdPID "/cwd")];
#define WANT_process_getpid_or_unlock
		pid_t pid = process_getpid_or_unlock(self);
		if unlikely(pid == ipc_Process_pid_t_INVALID)
			goto err;
		Process_LockEndWrite(self);
		Dee_sprintf(cwdpath, "/proc/%" PRFdPID "/cwd", pid);
		result = DeeUnixSystem_ReadLinkString(cwdpath);
		if unlikely(result == ITER_DONE) {
#define WANT_ipc_unimplemented
			ipc_unimplemented();
			goto err;
		}
	}
#else /* ... */
	{
		Process_LockEndWrite(self);
		ipc_unimplemented();
		goto err;
	}
#endif /* !... */

#if 0 /* Don't cache the working directory of external processes */
	Process_LockWrite(self);
	if likely(!self->p_pwd) {
		Dee_Incref(result);
		self->p_pwd = result;
		Process_LockEndWrite(self);
	} else {
		DREF DeeObject *old_result;
		old_result = self->p_pwd;
		Dee_Incref(old_result);
		Process_LockEndWrite(self);
		Dee_Decref_likely(result);
		result = old_result;
	}
#endif
done:
	return result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_pwd(Process *self, DeeObject *value) {
	DREF DeeObject *old_value;
	Process_LockWrite(self);
	if (self->p_state & (PROCESS_FLAG_STARTING | PROCESS_FLAG_STARTED)) {
		if ((self->p_state & PROCESS_FLAG_SELF) && value != NULL) {
			DREF DeeObject *response;
			Process_LockEndWrite(self);
			/* Special case: change working directory of our own process. */
			response = DeeModule_CallExternStringf("posix", "chdir", "o", value);
			if unlikely(!response)
				goto err;
			Dee_Decref(response);
			return 0;
		}
		Process_LockEndWrite(self);
		goto err_running;
	}
	Dee_XIncref(value);
	old_value   = self->p_pwd;
	self->p_pwd = value;
	Process_LockEndWrite(self);
	Dee_XDecref(old_value);
	return 0;
err_running:
	err_cannot_set_attribute_for_running_process(self, "pwd");
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_pwd(Process *__restrict self) {
	return process_set_pwd(self, NULL);
}


STATIC_ASSERT(DEE_STDIN == 0);
STATIC_ASSERT(DEE_STDOUT == 1);
STATIC_ASSERT(DEE_STDERR == 2);
PRIVATE char const std_handle_names[3][8] = {
	/*[DEE_STDIN]  =*/ "stdin",
	/*[DEE_STDOUT] =*/ "stdout",
	/*[DEE_STDERR] =*/ "stderr",
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_stdfd(Process *__restrict self, unsigned int std_handle_id) {
	DREF DeeObject *result;
	Process_LockRead(self);

	/* Check if we've already cached the correct result. */
	result = self->p_stdfd[std_handle_id];
	if (result) {
		Dee_Incref(result);
		Process_LockEndRead(self);
		goto done;
	}

	/* Make sure that the process has already started. */
	if (!(self->p_state & PROCESS_FLAG_STARTED)) {
		Process_LockEndRead(self);
		DeeRT_ErrTUnboundAttrCStr(&DeeProcess_Type, self, std_handle_names[std_handle_id]);
		goto err;
	}

	/* Get a write-lock. */
	if (!Process_LockUpgrade(self)) {
		result = self->p_stdfd[std_handle_id];
		if (result) {
			Dee_Incref(result);
			Process_LockEndWrite(self);
			goto done;
		}
	}

	/* Special case: current process */
	if (self->p_state & PROCESS_FLAG_SELF) {
		Process_LockEndWrite(self);
		return DeeFile_GetStd(std_handle_id);
	}

	/* OS-specific part... */
#ifdef ipc_Process_USE_CreateProcessW
	{
		HANDLE hProcess;
		hProcess = process_nt_gethandle_or_unlock(self);
		if unlikely(hProcess == INVALID_HANDLE_VALUE)
			goto err;
		Process_LockEndWrite(self);
		/* TODO: Try to read from `RTL_USER_PROCESS_PARAMETERS' */
#define WANT_ipc_unimplemented
		ipc_unimplemented();
		goto err;
	}
#elif defined(ipc_Process_USE_UNIX_APIS)
	{
		char stdfd_path[sizeof("/proc/" PRFMAXdPID "/fd/" PRFMAXdSTDFD)];
#define WANT_process_getpid_or_unlock
		pid_t pid = process_getpid_or_unlock(self);
		if unlikely(pid == ipc_Process_pid_t_INVALID)
			goto err;
		Process_LockEndWrite(self);
		Dee_sprintf(stdfd_path, "/proc/%" PRFdPID "/fd/%d", pid,
		            DEE_STDID_TO_FILENO(std_handle_id));
		result = DeeFile_OpenString(stdfd_path, OPEN_FRDWR, 0);
		if (result == NULL && DeeError_Catch(&DeeError_FileAccessError))
			result = DeeFile_OpenString(stdfd_path, OPEN_FRDONLY, 0);
		if unlikely(result == ITER_DONE) {
#define WANT_ipc_unimplemented
			ipc_unimplemented();
			goto err;
		}
	}
#else /* ... */
	{
		Process_LockEndWrite(self);
		ipc_unimplemented();
		goto err;
	}
#endif /* !... */

#if 0 /* Don't cache STD files of external processes */
	Process_LockWrite(self);
	if likely(!self->p_stdfd) {
		Dee_Incref(result);
		self->p_stdfd = result;
		Process_LockEndWrite(self);
	} else {
		DREF DeeObject *old_result;
		old_result = self->p_stdfd;
		Dee_Incref(old_result);
		Process_LockEndWrite(self);
		Dee_Decref_likely(result);
		result = old_result;
	}
#endif
done:
	return result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_stdfd(Process *self, unsigned int std_handle_id, DeeObject *value) {
	DREF DeeObject *old_value;
	Process_LockWrite(self);
	if (self->p_state & (PROCESS_FLAG_STARTING | PROCESS_FLAG_STARTED)) {
		/* Special case: change std handle of current process. */
		if (self->p_state & PROCESS_FLAG_SELF) {
			DREF DeeObject *response;
			Process_LockEndWrite(self);
			if (value == NULL)
				value = DeeFile_DefaultStd(std_handle_id);
			response = DeeFile_SetStd(std_handle_id, value);
			if (ITER_ISOK(response))
				Dee_Decref(response);
			return 0;
		}
		Process_LockEndWrite(self);
		goto err_running;
	}
	Dee_XIncref(value);
	old_value = self->p_stdfd[std_handle_id];
	self->p_stdfd[std_handle_id] = value;
	Process_LockEndWrite(self);
	Dee_XDecref(old_value);
	return 0;
err_running:
	err_cannot_set_attribute_for_running_process(self, std_handle_names[std_handle_id]);
#ifdef WANT_err
#undef WANT_err
err:
#endif /* WANT_err */
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_stdfd(Process *__restrict self, unsigned int std_handle_id) {
	return process_set_stdfd(self, std_handle_id, NULL);
}



/* Process std handle accessor wrappers. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_stdin(Process *__restrict self) {
	return process_get_stdfd(self, DEE_STDIN);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_stdin(Process *__restrict self) {
	return process_del_stdfd(self, DEE_STDIN);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
process_set_stdin(Process *self, DeeObject *value) {
	return process_set_stdfd(self, DEE_STDIN, value);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_stdout(Process *__restrict self) {
	return process_get_stdfd(self, DEE_STDOUT);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_stdout(Process *__restrict self) {
	return process_del_stdfd(self, DEE_STDOUT);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
process_set_stdout(Process *self, DeeObject *value) {
	return process_set_stdfd(self, DEE_STDOUT, value);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_stderr(Process *__restrict self) {
	return process_get_stdfd(self, DEE_STDERR);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_stderr(Process *__restrict self) {
	return process_del_stdfd(self, DEE_STDERR);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
process_set_stderr(Process *self, DeeObject *value) {
	return process_set_stdfd(self, DEE_STDERR, value);
}




PRIVATE struct type_method tpconst process_methods[] = {
	TYPE_METHOD_F("start", &process_start, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "#t{:Interrupt}"
	              "#tFileNotFound{The specified executable could not be found}"
	              "#tFileAccessError{The current user does not have permissions to access the "
	              /*                  */ "executable, or the executable is lacking execute permissions}"
	              "#tSystemError{Failed to start the process for some reason}"
	              "Begin execution of the process"),
	TYPE_METHOD_F("join", &process_join, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#t{:Interrupt}"
	              "#tValueError{@this process was never started}"
	              "#tValueError{@this process as already detached or joined}"
	              "#tSystemError{Failed to join @this process for some reason}"
	              "#r{The exit code of the process}"
	              "Joins @this process and returns the return value of its main function"),
	TYPE_METHOD_F("tryjoin", &process_tryjoin, METHOD_FNOREFESCAPE,
	              "->?X2?Dint?N\n"
	              "#tValueError{@this process was never started}"
	              "#tValueError{@this process as already detached or joined}"
	              "#tSystemError{Failed to join @this process for some reason}"
	              "Same as ?#join, but don't check for interrupts and fail immediately"),
	TYPE_METHOD_F("timedjoin", &process_timedjoin, METHOD_FNOREFESCAPE,
	              "(timeout_in_nanoseconds:?Dint)->?X2?Dint?N\n"
	              "#tValueError{@this process was never started}"
	              "#tValueError{@this process as already detached or joined}"
	              "#tSystemError{Failed to join @this process for some reason}"
	              "Same as ?#join, but only attempt to join for a given @timeout_in_nanoseconds"),
	TYPE_METHOD_F("detach", &process_detach, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "#tValueError{@this process was never started}"
	              "#tSystemError{Failed to detach @this process for some reason}"
	              "#r{true The ?. has been detached}"
	              "#r{false The ?. had already been detached or joined}"
	              "Detaches @this process"),
	TYPE_KWMETHOD_F("kill", &process_kill, METHOD_FNOREFESCAPE,
	                "(exitcode=!0,signal=!9)->?Dbool\n"
	                "#tValueError{@this process was never started}"
	                "#tSystemError{Failed to terminate @this process for some reason}"
	                "#r{true The ?. has been terminated}"
	                "#r{false The ?. was already terminated}"
	                "Terminate @this process with the given @exitcode or @signal"),
	TYPE_KWMETHOD_F("terminate", &process_kill, METHOD_FNOREFESCAPE,
	                "(exitcode=!0,signal=!9)->?Dbool\n"
	                "Deprecated alias for ?#kill"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst process_getsets[] = {
	TYPE_GETSET_F("exe", &process_get_exe, &process_del_exe, &process_set_exe, METHOD_FNOREFESCAPE,
	              "->?X3?Dint?DFile?Dstring\n"
	              "The filename of the image being executed by the ?.\n"
	              "When constructing a new ?., this can also be a file descriptor or ?DFile"),
	TYPE_GETSET_F("argv", &process_get_argv, &process_del_argv, &process_set_argv, METHOD_FNOREFESCAPE,
	              "->?S?Dstring\n"
	              "The argument vector passed to the program's #C{main()} method"),
	TYPE_GETSET_F("environ", &process_get_environ, &process_del_environ, &process_set_environ, METHOD_FNOREFESCAPE,
	              "->?M?Dstring?Dstring\n"
	              "The state of environment variables in the given ?.\n"
	              "When constructing a new ?., or leaving this unset, the contents of "
	              /**/ "?Eposix:environ at the time of ?#start being called is used instead"),
	TYPE_GETSET_F("pwd", &process_get_pwd, &process_del_pwd, &process_set_pwd, METHOD_FNOREFESCAPE,
	              "->?X3?Dint?DFile?Dstring\n"
	              "Get or set the path to the ?. working directory used by the ?.\n"
	              "When constructing a new ?., this can also be a file descriptor or ?DFile"),
	TYPE_GETSET_F("stdin", &process_get_stdin, &process_del_stdin, &process_set_stdin, METHOD_FNOREFESCAPE,
	              "->?X2?Dint?DFile\n"
	              "Get or set the standard input stream used by the ?.\n"
	              "When constructing a new ?., this can also be a file descriptor"),
	TYPE_GETSET_F("stdout", &process_get_stdout, &process_del_stdout, &process_set_stdout, METHOD_FNOREFESCAPE,
	              "->?X2?Dint?DFile\n"
	              "Get or set the standard output stream used by the ?.\n"
	              "When constructing a new ?., this can also be a file descriptor"),
	TYPE_GETSET_F("stderr", &process_get_stderr, &process_del_stderr, &process_set_stderr, METHOD_FNOREFESCAPE,
	              "->?X2?Dint?DFile\n"
	              "Get or set the standard error stream used by the ?.\n"
	              "When constructing a new ?., this can also be a file descriptor"),
	TYPE_GETTER_F("hasterminated", &process_hasterminated, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this ?. has terminated"),
	TYPE_GETTER_F("id", &process_id, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{The ?. hasn't been started yet}"
	              "Returns an operating-system specific id of @this ?."),

#ifdef ipc_Process_USE_cmdline
	TYPE_GETSET_F("cmdline_np", &process_get_cmdline, &process_del_cmdline, &process_set_cmdline, METHOD_FNOREFESCAPE,
	              "->?Dstring\n"
	              "The commandline used to package ?#argv"),
#endif /* ipc_Process_USE_cmdline */
#ifdef ipc_Process_USE_CreateProcessW
	TYPE_GETTER_F(Dee_fd_osfhandle_GETSET, &process_osfhandle_np, METHOD_FNOREFESCAPE, "->?Dint"),
#endif /* ipc_Process_USE_CreateProcessW */

	/* TODO: vvv re-implement these at a later point in time. */
	// TODO: TYPE_GETTER("threads", &process_get_threads, S_Process_getset_threads_doc),
	// TODO: TYPE_GETTER("files", &process_get_files, S_Process_getset_files_doc),
	// TODO: TYPE_GETTER("memory", &process_get_memory, S_Process_getset_memory_doc),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst process_members[] = {
	TYPE_MEMBER_BITFIELD_DOC("hasstarted", STRUCT_CONST, Process, p_state, PROCESS_FLAG_STARTED,
	                         "->?Dbool\n"
	                         "Returns ?t if @this process was started"),
	TYPE_MEMBER_BITFIELD_DOC("wasdetached", STRUCT_CONST, Process, p_state, PROCESS_FLAG_DETACHED,
	                         "->?Dbool\n"
	                         "Returns ?t if @this process has been detached or joined"),
	TYPE_MEMBER_BITFIELD_DOC("isextern", STRUCT_CONST, Process, p_state, PROCESS_FLAG_EXTERN,
	                         "->?Dbool\n"
	                         "Returns ?t if @this process is not controlled by deemon"),
	TYPE_MEMBER_END
};




PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_class_self(DeeObject *UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("self");]]]*/
	DeeArg_Unpack0(err, argc, argv, "self");
/*[[[end]]]*/
	Dee_Incref(&this_process);
	return (DeeObject *)&this_process;
err:
	return NULL;
}

PRIVATE struct type_method tpconst process_class_methods[] = {
	TYPE_METHOD("self", &process_class_self,
	            "->?.\n"
	            "Deprecated alias for ?#current"),
	TYPE_METHOD_END
};

PRIVATE struct type_member tpconst process_class_members[] = {
	TYPE_MEMBER_CONST_DOC("current", &this_process,
	                      "A process descriptor for the hosting instance of deemon itself"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeProcess_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Process",
	/* .tp_doc      = */ DOC("(pid:?Dint)\n"
	                         "Construct a descriptor for a process with the given @pid\n"
	                         "\n"
	                         "(cmdline:?Dstring)\n"
	                         "(exe:?X3?Dint?DFile?Dstring,argv:?S?Dstring,environ?:?S?Dstring)\n"
	                         "Construct a new process that can be started with ?#start, and joined with ?#join"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&process_init,
				TYPE_FIXED_ALLOCATOR_GC(Process)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&process_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&process_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&process_printrepr
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&process_visit,
	/* .tp_gc            = */ &process_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ process_methods,
	/* .tp_getsets       = */ process_getsets,
	/* .tp_members       = */ process_members,
	/* .tp_class_methods = */ process_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ process_class_members
};


#if (defined(HAVE_ipc_reaped_childprocs_clear) || \
     defined(HAVE_ipc_exe2path_fini) ||           \
     defined(HAVE_ipc_etc_shells_default_shell_fini))
#ifdef HAVE_libipc_fini
#error "Multiple definitions of `libipc_fini()'"
#endif /* HAVE_libipc_fini */
#define HAVE_libipc_fini
PRIVATE NONNULL((1)) void DCALL
libipc_fini(DeeDexObject *__restrict UNUSED(self)) {
#ifdef HAVE_ipc_reaped_childprocs_clear
	ipc_reaped_childprocs_clear();
#endif /* HAVE_ipc_reaped_childprocs_clear */
#ifdef HAVE_ipc_exe2path_fini
	ipc_exe2path_fini();
#endif /* HAVE_ipc_exe2path_fini */
#ifdef HAVE_ipc_etc_shells_default_shell_fini
	ipc_etc_shells_default_shell_fini();
#endif /* HAVE_ipc_etc_shells_default_shell_fini */
}
#endif /* ... */


DECL_END

#endif /* !GUARD_DEX_IPC_PROCESS_C_INL */
