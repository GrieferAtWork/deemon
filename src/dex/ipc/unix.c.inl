/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_IPC_UNIX_C_INL
#define GUARD_DEX_IPC_UNIX_C_INL 1
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system.h> /* DeeUnixSystem_GetFD() */
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/rwlock.h>

#include <hybrid/sched/yield.h>

#include <stdarg.h>

#include "_res.h"
#include "libipc.h"

#ifdef SIGKILL
#define TERMINATE_SIGNAL SIGKILL
#elif defined(SIGTERM)
#define TERMINATE_SIGNAL SIGTERM
#elif defined(SIGABRT)
#define TERMINATE_SIGNAL SIGABRT
#elif 1 /* Maybe it's an enum? */
#define TERMINATE_SIGNAL SIGKILL
#else
#error "No termination signal recognized"
#endif


/**/
#include "generic-cmdline.c.inl"

DECL_BEGIN

#ifndef CONFIG_HAVE_strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */

#ifdef _MSC_VER
typedef uintptr_t pid_t;
#endif /* _MSC_VER */

#ifdef CONFIG_HAVE_waitpid
#define joinpid(pid, pexit_status)    waitpid(pid, pexit_status, 0)
#define tryjoinpid(pid, pexit_status) waitpid(pid, pexit_status, WNOHANG)
#elif defined(CONFIG_HAVE_wait4)
#define joinpid(pid, pexit_status)    wait4(pid, pexit_status, 0, NULL)
#define tryjoinpid(pid, pexit_status) wait4(pid, pexit_status, WNOHANG, NULL)
#elif defined(__INTELLISENSE__)
pid_t joinpid(pid_t pid, int *pexit_status);
pid_t tryjoinpid(pid_t pid, int *pexit_status);
#define WEXITSTATUS(status) 0
#define WIFEXITED(status)   1
#else
#error "No known way of joining child processes detected"
#endif


#ifdef __INTELLISENSE__
#ifdef CONFIG_HOST_WINDOWS
pid_t fork(void);
int kill(pid_t pid, int signo);
#endif /* CONFIG_HOST_WINDOWS */
#endif /* __INTELLISENSE__ */

#ifndef CONFIG_HAVE_vfork
#define vfork()  fork()
#endif /* !CONFIG_HAVE_vfork */

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif /* !STDIN_FILENO */

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif /* !STDOUT_FILENO */

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif /* !STDERR_FILENO */

#ifdef CONFIG_HAVE__Exit
#define EXIT_AFTER_FORK _Exit
#else /* CONFIG_HAVE__Exit */
#define EXIT_AFTER_FORK exit
#endif /* !CONFIG_HAVE__Exit */

#ifndef PATH_MAX
#ifdef PATHMAX
#   define PATH_MAX PATHMAX
#elif defined(MAX_PATH)
#   define PATH_MAX MAX_PATH
#elif defined(MAXPATH)
#   define PATH_MAX MAXPATH
#else
#   define PATH_MAX 260
#endif
#endif /* !PATH_MAX */


#define HAVE_UNIX_READLINK 1
#ifdef CONFIG_NO_READLINK
#undef HAVE_UNIX_READLINK
#endif /* CONFIG_NO_READLINK */
#ifdef CONFIG_NO_LSTAT
#undef HAVE_UNIX_READLINK
#endif /* CONFIG_NO_LSTAT */
#ifndef S_ISLNK
#undef HAVE_UNIX_READLINK
#endif /* !S_ISLNK */


#ifdef HAVE_UNIX_READLINK
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
unix_readlink(/*utf-8*/ char const *__restrict path) {
	char *buffer, *new_buffer;
	int error;
	size_t bufsize, new_size;
	dssize_t req_size;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	bufsize = PATH_MAX;
	buffer  = unicode_printer_alloc_utf8(&printer, bufsize);
	if unlikely(!buffer)
		goto err;
	for (;;) {
		struct stat st;
		if (DeeThread_CheckInterrupt())
			goto err_buf;
		DBG_ALIGNMENT_DISABLE();
		req_size = readlink(path, buffer, bufsize + 1);
		if unlikely(req_size < 0) {
handle_error:
			error = DeeSystem_GetErrno();
			DBG_ALIGNMENT_ENABLE();
			DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
			                          "Failed to read symbolic link %q",
			                          path);
			goto err_buf;
		}
		DBG_ALIGNMENT_ENABLE();
		if ((size_t)req_size <= bufsize)
			break;
		DBG_ALIGNMENT_DISABLE();
		if (lstat(path, &st))
			goto handle_error;
		DBG_ALIGNMENT_ENABLE();
		/* Ensure that this is still a symbolic link. */
		if (!S_ISLNK(st.st_mode)) {
			error = EINVAL;
			goto handle_error;
		}
		new_size = (size_t)st.st_size;
		if (new_size <= bufsize)
			break; /* Shouldn't happen, but might due to race conditions? */
		new_buffer = unicode_printer_resize_utf8(&printer, buffer, new_size);
		if unlikely(!new_buffer)
			goto err_buf;
		buffer  = new_buffer;
		bufsize = new_size;
	}
	/* Release unused data. */
	if unlikely(unicode_printer_commit_utf8(&printer, buffer, (size_t)req_size) < 0)
		goto err_buf;
	bufsize = UNICODE_PRINTER_LENGTH(&printer);
	while (bufsize && !DeeSystem_IsSep(UNICODE_PRINTER_GETCHAR(&printer, bufsize - 1)))
		--bufsize;
	while (bufsize && DeeSystem_IsSep(UNICODE_PRINTER_GETCHAR(&printer, bufsize - 1)))
		--bufsize;
	UNICODE_PRINTER_SETCHAR(&printer, bufsize, DeeSystem_SEP);
	unicode_printer_truncate(&printer, bufsize + 1);
	return unicode_printer_pack(&printer);
err_buf:
	unicode_printer_free_utf8(&printer, buffer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
unix_vreadlinkf(/*utf-8*/ char const *format, va_list args) {
	char *utf8_path;
	DREF DeeObject *result = NULL, *path;
	path = DeeString_VNewf(format, args);
	if unlikely(!path)
		goto done;
	utf8_path = DeeString_AsUtf8(path);
	if likely(utf8_path) {
		result = unix_readlink(utf8_path);
	}
	Dee_Decref(path);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *
unix_readlinkf(/*utf-8*/ char const *format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = unix_vreadlinkf(format, args);
	va_end(args);
	return result;
}
#endif /* HAVE_UNIX_READLINK */




typedef struct {
	OBJECT_HEAD
	rwlock_t                   p_lock;    /* Lock for accessing members of this structure. */
	DREF DeeObject            *p_std[3];  /* [lock(p_lock)][0..1] STD streams for when the process will be running (the handle is loaded by calling `.fileno()'). */
	DREF DeeObject            *p_environ; /* [lock(p_lock)][0..1] The environment block used by the process (or `NULL' to refer to that of the calling process).
	                                       * NOTE: When set, this is a sequence type compatible with `fs.environ': `{string: string}' */
	DREF DeeStringObject      *p_pwd;     /* [lock(p_lock)][0..1] The process working directory (or `NULL' to refer to that of the calling process). */
	DREF DeeStringObject      *p_exe;     /* [lock(p_lock)][0..1] The filename to an executable that should be run by the process. */
	DREF DeeObject            *p_argv;    /* [lock(p_lock)][0..1] The argument vector for the executable. */
	pid_t                      p_pid;     /* [valid_if(PROCESS_FSTARTED)] The ID of the process (pid) */
	int                        p_status;  /* [valid_if(PROCESS_FDIDJOIN)] The process's exit status. */
#define PROCESS_FNORMAL        0x0000     /* Normal process flags. */
#define PROCESS_FSTARTED       0x0001     /* The process has been started. (Always set for external processes) */
#define PROCESS_FDETACHED      0x0002     /* The process has been detached (`p_pid' is dangling and `p_handle' is invalid). */
#define PROCESS_FCHILD         0x0004     /* The process is one of deemon's children (Set during process start). */
#define PROCESS_FTERMINATED    0x0008     /* The process has terminated. */
#define PROCESS_FSTARTING      0x1000     /* The process is currently starting. */
#define PROCESS_FDETACHING     0x2000     /* The process is currently being detached. */
#define PROCESS_FDIDJOIN       0x4000     /* The process has been joined. */
	uint16_t                   p_state;   /* The state of the process (Set of `PROCESS_F*') */
} Process;

PRIVATE Process this_process = {
	OBJECT_HEAD_INIT(&DeeProcess_Type),
	/* .p_lock    = */ RWLOCK_INIT,
	/* .p_std     = */ {
		/* [DEE_STDIN]  = */ NULL, /* Always NULL - Access is forwarded to DeeFile_DefaultStd(). */
		/* [DEE_STDOUT] = */ NULL, /* Always NULL - Access is forwarded to DeeFile_DefaultStd(). */
		/* [DEE_STDERR] = */ NULL, /* Always NULL - Access is forwarded to DeeFile_DefaultStd(). */
	},
	/* .p_environ = */ NULL,      /* Always NULL - Access is forwarded to `fs.environ'. */
	/* .p_pwd     = */ NULL,      /* Always NULL - Access is forwarded to `fs.getpwd()' and `fs.chdir()'. */
	/* .p_exe     = */ NULL,      /* Always NULL - Read using `readlink("/proc/self/exe")'. */
	/* .p_argv    = */ NULL,      /* Always NULL - Read using `read("/proc/self/cmdline")'. */
	/* .p_pid     = */ (pid_t)-1, /* Lazily loaded. */
	/* .p_state   = */ PROCESS_FSTARTED
};

#ifndef IPC_UNIMPLEMENTED_DEFINED
#define IPC_UNIMPLEMENTED_DEFINED 1
PRIVATE ATTR_COLD int DCALL ipc_unimplemented(void) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "IPI functionality is not supported");
}
#endif /* !IPC_UNIMPLEMENTED_DEFINED */

PRIVATE WUNUSED NONNULL((1, 3)) char **DCALL
process_pack_argv_fast(DeeObject *__restrict seq, size_t fastlen,
                       DREF DeeObject ***__restrict pobj_vector) {
	char **result, *utf8;
	DREF DeeObject *elem, **obj_vector;
	size_t i;
	result = (char **)Dee_Mallocc(fastlen + 2, sizeof(char *));
	if (!result)
		goto done;
	obj_vector = (DREF DeeObject **)Dee_Mallocc(fastlen, sizeof(DREF DeeObject *));
	if (!result)
		goto err_result;
	for (i = 0; i < fastlen; ++i) {
		elem = DeeFastSeq_GetItem(seq, i);
		if unlikely(!elem)
			goto err;
		if (DeeObject_AssertTypeExact(elem, &DeeString_Type))
			goto err_elem;
		utf8 = DeeString_AsUtf8(elem);
		if unlikely(!utf8)
			goto err_elem;
		result[i + 1] = utf8;
		obj_vector[i] = elem; /* Inherit reference. */
	}
	result[fastlen + 1] = NULL;
	*pobj_vector = obj_vector;
done:
	return result;
err_elem:
	Dee_Decref(elem);
err:
	while (i--)
		Dee_Decref(obj_vector[i]);
	Dee_Free(obj_vector);
err_result:
	Dee_Free(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) char **DCALL
process_pack_argv_iter(DeeObject *__restrict iterator,
                       DREF DeeObject ***__restrict pobj_vector,
                       size_t *__restrict pobj_length) {
	char **result, **new_result;
	DREF DeeObject **obj_vector, **new_obj_vector;
	size_t result_len, result_alloc;
	DREF DeeObject *elem;
	result_len   = 0;
	result_alloc = 8;
	result       = (char **)Dee_TryMallocc(result_alloc + 2, sizeof(char *));
	obj_vector   = (DREF DeeObject **)Dee_TryMallocc(result_alloc, sizeof(DREF DeeObject *));
	if (!result || !obj_vector) {
		Dee_Free(result);
		Dee_Free(obj_vector);
		result_alloc = 0;
		result = (char **)Dee_Mallocc(result_alloc + 2, sizeof(char *));
		if unlikely(!result)
			goto done;
		obj_vector = NULL;
	}
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		char *elem_utf8;
		if (DeeObject_AssertTypeExact(elem, &DeeString_Type))
			goto err_elem;
		elem_utf8 = DeeString_AsUtf8(elem);
		if unlikely(!elem_utf8)
			goto err_elem;
		ASSERT(result_len <= result_alloc);
		if (result_len >= result_alloc) {
			size_t new_alloc = result_alloc * 2;
			if unlikely(!new_alloc)
				new_alloc = 1;
			new_obj_vector = (DREF DeeObject **)Dee_TryReallocc(obj_vector, new_alloc,
			                                                    sizeof(DREF DeeObject *));
			if unlikely(!new_obj_vector) {
				new_alloc = result_alloc + 1;
				new_obj_vector = (DREF DeeObject **)Dee_Reallocc(obj_vector, new_alloc,
				                                                 sizeof(DREF DeeObject *));
				if unlikely(!new_obj_vector)
					goto err_elem;
			}
			obj_vector = new_obj_vector;
			new_result = (char **)Dee_TryReallocc(result, new_alloc + 2, sizeof(char *));
			if unlikely(!new_result) {
				new_alloc = result_alloc + 1;
				new_result = (char **)Dee_Reallocc(result, new_alloc + 2, sizeof(char *));
				if unlikely(!new_result)
					goto err_elem;
			}
			result       = new_result;
			result_alloc = new_alloc;
		}
		result[result_len + 1] = elem_utf8;
		obj_vector[result_len] = elem; /* Inherit reference. */
		++result_len;
	}
	if unlikely(!elem)
		goto err;
	ASSERT(result_len <= result_alloc);
	if (result_len < result_alloc) {
		new_result = (char **)Dee_TryReallocc(result, result_len + 2, sizeof(char *));
		if likely(new_result)
			result = new_result;
		new_obj_vector = (DREF DeeObject **)Dee_TryReallocc(obj_vector, result_len,
		                                                    sizeof(DREF DeeObject *));
		if likely(new_obj_vector)
			obj_vector = new_obj_vector;
	}
	*pobj_length = result_len;
	*pobj_vector = obj_vector;
	result[result_len + 1] = NULL;
done:
	return result;
err_elem:
	Dee_Decref(elem);
err:
	while (result_len--)
		Dee_Decref(obj_vector[result_len]);
	Dee_Free(obj_vector);
	Dee_Free(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) char **DCALL
process_pack_argv(DeeObject *__restrict sequence,
                  DREF DeeObject ***__restrict pobj_vector,
                  size_t *__restrict pobj_length) {
	size_t fastlen;
	char **result;
	fastlen = DeeFastSeq_GetSize(sequence);
	if (fastlen != DEE_FASTSEQ_NOTFAST) {
		result = process_pack_argv_fast(sequence, fastlen, pobj_vector);
		*pobj_length = fastlen;
	} else {
		DREF DeeObject *iterator;
		iterator = DeeObject_IterSelf(sequence);
		if unlikely(!iterator)
			goto err;
		result = process_pack_argv_iter(iterator, pobj_vector, pobj_length);
		Dee_Decref(iterator);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) char **DCALL
process_pack_envp_iter(DeeObject *__restrict iterator) {
	char **result, **new_result;
	size_t result_len, result_alloc;
	DREF DeeObject *key_and_value[2];
	DREF DeeObject *elem;
	result_len   = 0;
	result_alloc = 8;
	result       = (char **)Dee_TryMallocc(result_alloc + 1, sizeof(char *));
	if (!result) {
		result_alloc = 0;
		result = (char **)Dee_Mallocc(result_alloc + 1, sizeof(char *));
		if unlikely(!result)
			goto done;
	}
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		/*utf-8*/ char *key_utf8, *value_utf8, *line, *p;
		size_t key_length, value_length, line_length;
		ASSERT(result_len <= result_alloc);
		if (result_len >= result_alloc) {
			size_t new_alloc = result_alloc * 2;
			if unlikely(!new_alloc)
				new_alloc = 1;
			new_result = (char **)Dee_TryReallocc(result, new_alloc + 1, sizeof(char *));
			if unlikely(!new_result) {
				new_alloc = result_alloc + 1;
				new_result = (char **)Dee_Reallocc(result, new_alloc + 1, sizeof(char *));
				if unlikely(!new_result)
					goto err_elem;
			}
			result       = new_result;
			result_alloc = new_alloc;
		}
		if (DeeObject_Unpack(elem, 2, key_and_value))
			goto err_elem;
		Dee_Decref(elem);
		if (DeeObject_AssertTypeExact(key_and_value[0], &DeeString_Type))
			goto err_key_and_value;
		if (DeeObject_AssertTypeExact(key_and_value[1], &DeeString_Type))
			goto err_key_and_value;
		key_utf8 = DeeString_AsUtf8(key_and_value[0]);
		if unlikely(!key_utf8)
			goto err_key_and_value;
		value_utf8 = DeeString_AsUtf8(key_and_value[1]);
		if unlikely(!value_utf8)
			goto err_key_and_value;
		key_length   = WSTR_LENGTH(key_utf8);
		value_length = WSTR_LENGTH(value_utf8);
		line_length  = key_length + 1 + value_length;
		line = (char *)Dee_Mallocc(line_length + 1, sizeof(char));
		if unlikely(!line)
			goto err_key_and_value;
		p    = line;
		p    = (char *)mempcpyc(p, key_utf8, key_length, sizeof(char));
		*p++ = '=';
		p    = (char *)mempcpyc(p, value_utf8, value_length, sizeof(char));
		*p   = '\0';
		Dee_Decref(key_and_value[1]);
		Dee_Decref(key_and_value[0]);
		result[result_len] = line; /* Inherit */
		++result_len;
	}
	if unlikely(!elem)
		goto err;
	ASSERT(result_len <= result_alloc);
	if (result_len < result_alloc) {
		new_result = (char **)Dee_TryReallocc(result, result_len + 1, sizeof(char *));
		if likely(new_result)
			result = new_result;
	}
	result[result_len] = NULL;
done:
	return result;
err_key_and_value:
	Dee_Decref(key_and_value[1]);
	Dee_Decref(key_and_value[0]);
	goto err;
err_elem:
	Dee_Decref(elem);
err:
	while (result_len--)
		Dee_Free(result[result_len]);
	Dee_Free(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) char **DCALL
process_pack_envp(DeeObject *__restrict mapping) {
	char **result;
	DREF DeeObject *iterator;
	iterator = DeeObject_IterSelf(mapping);
	if unlikely(!iterator)
		goto err;
	result = process_pack_envp_iter(iterator);
	Dee_Decref(iterator);
	return result;
err:
	return NULL;
}

PRIVATE void DCALL
process_free_envp(char **envp) {
	if (envp) {
		size_t i;
		for (i = 0; envp[i]; ++i)
			;
		while (i--)
			Dee_Free(envp[i]);
		Dee_Free(envp);
	}
}

PRIVATE ATTR_NOINLINE NONNULL((1)) pid_t DCALL
process_do_spawn(char const *used_exe, char *const *used_argv,
                 char *const *used_envp, char const *used_pwd,
                 int used_stdin, int used_stdout,
                 int used_stderr, bool search_path) {
	pid_t cpid;
#ifndef DEE_NO_DPRINTF
	if (_Dee_dprint_enabled) {
		Dee_DPRINTF("[IPC] spawn(exe:%q,argv:", used_exe);
		if (used_argv) {
			size_t i;
			Dee_DPRINT("[");
			for (i = 0; used_argv[i]; ++i) {
				if (i != 0)
					Dee_DPRINT(",");
				Dee_DPRINTF("%q", used_argv[i]);
			}
			Dee_DPRINT("]");
		} else {
			Dee_DPRINT("NULL");
		}
		Dee_DPRINT(",envp:");
		if (used_envp) {
			size_t i;
			Dee_DPRINT("[");
			for (i = 0; used_envp[i]; ++i) {
				if (i != 0)
					Dee_DPRINT(",");
				Dee_DPRINTF("%q", used_envp[i]);
			}
			Dee_DPRINT("]");
		} else {
			Dee_DPRINT("NULL");
		}
		Dee_DPRINT(",pwd:");
		if (used_pwd) {
			Dee_DPRINTF("%q", used_pwd);
		} else {
			Dee_DPRINT("NULL");
		}
		Dee_DPRINTF(",stdin:%d,stdout:%d,stderr:%d,search_path:%u)\n",
		            used_stdin, used_stdout, used_stderr,
		            search_path ? 1 : 0);
	}
#endif /* !DEE_NO_DPRINTF */

	cpid = vfork();
	if (cpid == 0) {
		/* Apply child process settings. */
		if (used_pwd) {
			if (chdir(used_pwd) != 0)
				goto child_error;
		}
		if (used_stdin != STDIN_FILENO) {
			if (dup2(used_stdin, STDIN_FILENO) < 0)
				goto child_error;
		}
		if (used_stdout != STDOUT_FILENO) {
			if (dup2(used_stdout, STDOUT_FILENO) < 0)
				goto child_error;
		}
		if (used_stderr != STDERR_FILENO) {
			if (dup2(used_stderr, STDERR_FILENO) < 0)
				goto child_error;
		}

		/* Actually execute the new process. */
		if (search_path) {
			if (used_envp) {
				execvpe(used_exe,
				        (EXEC_STRING_VECTOR_TYPE)used_argv,
				        (EXEC_STRING_VECTOR_TYPE)used_envp);
			} else {
				execvp(used_exe,
				       (EXEC_STRING_VECTOR_TYPE)used_argv);
			}
		} else {
			if (used_envp) {
				execve(used_exe,
				       (EXEC_STRING_VECTOR_TYPE)used_argv,
				       (EXEC_STRING_VECTOR_TYPE)used_envp);
			} else {
				execv(used_exe,
				      (EXEC_STRING_VECTOR_TYPE)used_argv);
			}
		}
child_error:
#ifdef CONFIG_HAVE_errno
		EXIT_AFTER_FORK(EXIT_FAILURE + DeeSystem_GetErrno());
#else /* CONFIG_HAVE_errno */
		EXIT_AFTER_FORK(EXIT_FAILURE);
#endif /* !CONFIG_HAVE_errno */
	}
	return cpid;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_start(Process *self, size_t argc, DeeObject *const *argv) {
	DeeObject *result = NULL;
	pid_t cpid;
	DREF DeeStringObject *exe, *pwd;
	DREF DeeObject *procin, *procout, *procerr;
	DREF DeeObject *procenv, *procargv;
	char **used_argv, **used_envp = NULL;
	DREF DeeObject **argv_objvector; size_t argv_objlength;
	char *used_exe, *used_pwd = NULL;
	int used_stdin  = STDIN_FILENO;
	int used_stdout = STDOUT_FILENO;
	int used_stderr = STDERR_FILENO;
	bool search_path;

	if (DeeArg_Unpack(argc, argv, ":" S_Process_function_start_name))
		goto done;
	if (DeeThread_CheckInterrupt())
		goto done;
again:
	rwlock_read(&self->p_lock);
	/* Check if the process had already been started. */
	if unlikely(self->p_state & PROCESS_FSTARTED) {
		rwlock_endread(&self->p_lock);
		return_false;
	}
	/* Set the starting-flag. */
	if (atomic_fetchor(&self->p_state, PROCESS_FSTARTING) & PROCESS_FSTARTING) {
		rwlock_endread(&self->p_lock);
		SCHED_YIELD();
		goto again;
	}
	exe      = self->p_exe;
	procargv = self->p_argv;
	if unlikely(!exe || !procargv) {
		/* Shouldn't happen because processes without these
		 * attributes should have the `PROCESS_FSTARTED'
		 * flag set. */
		rwlock_endread(&self->p_lock);
		DeeError_Throwf(&DeeError_ValueError,
		                "Cannot start process without exe and cmdline");
		goto done;
	}
	pwd     = self->p_pwd;
	procin  = self->p_std[DEE_STDIN];
	procout = self->p_std[DEE_STDOUT];
	procerr = self->p_std[DEE_STDERR];
	procenv = self->p_environ;
	Dee_Incref(exe);
	Dee_Incref(procargv);
	Dee_XIncref(pwd);
	Dee_XIncref(procin);
	Dee_XIncref(procout);
	Dee_XIncref(procerr);
	Dee_XIncref(procenv);
	rwlock_endread(&self->p_lock);

	/* Load the application exe */
	used_exe = DeeString_AsUtf8((DeeObject *)exe);
	if unlikely(!used_exe)
		goto done_procenv;

	/* Load the argument vector */
	used_argv = process_pack_argv(procargv,
	                              &argv_objvector,
	                              &argv_objlength);
	if unlikely(!used_argv)
		goto done_procenv;

	/* Always inject the exe name as the initial argument. */
	used_argv[0] = used_exe;

	/* Load the environment table */
	if (procenv) {
		used_envp = process_pack_envp(procenv);
		if unlikely(!used_envp)
			goto done_procenv_argv;
	}

	/* Load the initial working directory */
	if (pwd) {
		used_pwd = DeeString_AsUtf8((DeeObject *)pwd);
		if unlikely(!used_pwd)
			goto done_procenv_envp;
	}

	/* Load std file handles */
	if (procin) {
		used_stdin = DeeUnixSystem_GetFD(procin);
		if unlikely(used_stdin == -1)
			goto done_procenv_envp;
	}
	if (procout) {
		used_stdout = DeeUnixSystem_GetFD(procout);
		if unlikely(used_stdout == -1)
			goto done_procenv_envp;
	}
	if (procerr) {
		used_stderr = DeeUnixSystem_GetFD(procerr);
		if unlikely(used_stderr == -1)
			goto done_procenv_envp;
	}

	/* Check if $PATH should be searched for the executable.
	 * We do this when the specified executable doesn't contain
	 * any slashes. - Otherwise, search $PATH for it. */
#ifdef DeeSystem_ALTSEP
	search_path = strchr(used_exe, DeeSystem_SEP) == NULL &&
	              strchr(used_exe, DeeSystem_ALTSEP) == NULL;
#else /* DeeSystem_ALTSEP */
	search_path = strchr(used_exe, DeeSystem_SEP) == NULL;
#endif /* !DeeSystem_ALTSEP */

	/* Actually spawn the process */
	cpid = process_do_spawn(used_exe,
	                        used_argv,
	                        used_envp,
	                        used_pwd,
	                        used_stdin,
	                        used_stdout,
	                        used_stderr,
	                        search_path);
	if (cpid < 0) {
		int error;
		DBG_ALIGNMENT_DISABLE();
		error = errno;
		DBG_ALIGNMENT_ENABLE();
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                          "Failed to spawn new process");
	} else {
		rwlock_write(&self->p_lock);
		/* Set the started and child flags now that the processed is running. */
		self->p_state |= (PROCESS_FSTARTED | PROCESS_FCHILD);
		self->p_pid = cpid;
		rwlock_endwrite(&self->p_lock);
		result = Dee_None;
		Dee_Incref(Dee_None);
	}
done_procenv_envp:
	process_free_envp(used_envp);
done_procenv_argv:
	while (argv_objlength--)
		Dee_Decref(argv_objvector[argv_objlength]);
	Dee_Free(argv_objvector);
	Dee_Free(used_argv);
done_procenv:
	rwlock_read(&self->p_lock);
	atomic_and(&self->p_state, ~PROCESS_FSTARTING);
	rwlock_endread(&self->p_lock);
	Dee_XDecref(procenv);
	Dee_XDecref(procerr);
	Dee_XDecref(procout);
	Dee_XDecref(procin);
	Dee_XDecref(pwd);
	Dee_Decref(procargv);
	Dee_Decref(exe);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_detach(Process *self, size_t argc, DeeObject *const *argv) {
	uint16_t state;
	if (DeeArg_Unpack(argc, argv, ":" S_Process_function_detach_name))
		goto err;
	while (atomic_fetchor(&self->p_state, PROCESS_FDETACHING) &
	       PROCESS_FDETACHING)
		SCHED_YIELD();
	if (!(self->p_state & (PROCESS_FSTARTED | PROCESS_FTERMINATED))) {
		/* Process was never started. */
		atomic_and(&self->p_state, ~THREAD_STATE_DETACHING);
		DeeError_Throwf(&DeeError_ValueError,
		                "Cannot detach process %k that hasn't been started",
		                self);
		goto err;
	}
	if (!(self->p_state & PROCESS_FCHILD)) {
		/* Process isn't one of our children. */
		atomic_and(&self->p_state, ~THREAD_STATE_DETACHING);
		DeeError_Throwf(&DeeError_ValueError,
		                "Cannot detach process %k that isn't a child",
		                self);
		goto err;
	}
	if (self->p_state & THREAD_STATE_DETACHED) {
		/* Process was already detached. */
		atomic_and(&self->p_state, ~PROCESS_FDETACHING);
		return_false;
	}
#ifdef CONFIG_HAVE_detach
	detach(self->p_pid);
#endif /* CONFIG_HAVE_detach */
	/* Set the detached-flag and unset the detaching-flag. */
	do {
		state = atomic_read(&self->p_state);
	} while (!atomic_cmpxch_weak_or_write(&self->p_state, state,
	                                      (state & ~PROCESS_FDETACHING) |
	                                      PROCESS_FDETACHED));
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_id(Process *__restrict self) {
	pid_t pid;
	rwlock_read(&self->p_lock);
	if (!(self->p_state & PROCESS_FSTARTED)) {
		rwlock_endread(&self->p_lock);
		DeeError_Throwf(&DeeError_ValueError,
		                "Process %k has no id because it wasn't started",
		                self);
		goto err;
	}
	pid = self->p_pid;
	if (pid == 0 && self == &this_process) {
		/* Lazily load the PID of the current process. */
		DBG_ALIGNMENT_DISABLE();
		pid = getpid();
		DBG_ALIGNMENT_ENABLE();
		rwlock_upgrade(&self->p_lock);
		COMPILER_READ_BARRIER();
		if (self->p_pid == 0) {
			self->p_pid = pid;
		} else {
			pid = self->p_pid;
		}
		rwlock_endwrite(&self->p_lock);
	} else {
		rwlock_endread(&self->p_lock);
	}
	return DeeInt_NewU32((uint32_t)pid);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_terminate(Process *self, size_t argc, DeeObject *const *argv) {
	uint32_t exit_code = 0;
	if (DeeArg_Unpack(argc, argv,
	                  "|" UNPu32 ":" S_Process_function_terminate_name,
	                  &exit_code))
		goto err;
	if (self == &this_process) {
		Dee_Exit((int)exit_code, false);
		return NULL;
	}
	if (DeeThread_CheckInterrupt())
		goto err;
	rwlock_write(&self->p_lock);
	if (!(self->p_state & PROCESS_FSTARTED)) {
		rwlock_endwrite(&self->p_lock);
		DeeError_Throwf(&DeeError_ValueError,
		                "Process %k has not been started",
		                self);
		goto err;
	}
	if (self->p_state & PROCESS_FTERMINATED) {
		rwlock_endwrite(&self->p_lock);
		/* Already terminated. */
		return_false;
	}
	DBG_ALIGNMENT_DISABLE();
	if (kill(self->p_pid, TERMINATE_SIGNAL) != 0) {
		int error;
		DBG_ALIGNMENT_DISABLE();
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                          "Process %k could not be terminated",
		                          self);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	/* Set the did-terminate flag on success. */
	self->p_state |= PROCESS_FTERMINATED;
	rwlock_endwrite(&self->p_lock);
	return_true;
err:
	return NULL;
}

/* @return:  1: Join failed (timeout)
 * @return:  0: Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
wait_for_process(Process *__restrict self,
                 pid_t pid,
                 uint64_t timeout_microseconds) {
	int result;
#ifdef EINTR
again:
#endif /* EINTR */
	/* Handle the simple cases: no timeout/infinite timeout. */
	if (timeout_microseconds == 0) {
		result = tryjoinpid(pid, &self->p_status);
	} else if (timeout_microseconds == (uint64_t)-1) {
		result = joinpid(pid, &self->p_status);
	} else {
#if defined(CONFIG_HAVE_SYS_SIGNALFD_H) && \
    defined(CONFIG_HAVE_sigprocmask)
		/* The complicated: a custom timeout. */
		/* NOTE: wait() w/ timeout can be implemented with `signalfd()':
		 * >> pid_t waitpid_timeout(pid_t pid, struct timespec *tmo, int *status) {
		 * >>     pid_t result;
		 * >>     result = waitpid(pid, status, WNOHANG);
		 * >>     if (result == 0) {
		 * >>         int fd;
		 * >>         struct pollfd pfd[1];
		 * >>         sigset_t ss, old_ss;
		 * >>         sigemptyset(&ss);
		 * >>         sigaddset(&ss, SIGCHLD);
		 * >>         // Create a signalfd to wait for SIGCHLD
		 * >>         // NOTE: Also set the `SFD_NONBLOCK' flag so us reading
		 * >>         //       from the descriptor in order to clear it will
		 * >>         //       not block.
		 * >>         fd = signalfd(-1, &ss, SFD_CLOEXEC | SFD_NONBLOCK);
		 * >>         // Prevent SIGCHLD from triggering a signal handler, and
		 * >>         // ensure that it is always able to handle the signal.
		 * >>         sigprocmask(SIG_BLOCK, &ss, &old_ss);
		 * >>         pfd[0].fd     = fd;
		 * >>         pfd[0].events = POLLIN;
		 * >>         for (;;) {
		 * >>             // With our signalfd connected, try once again
		 * >>             // if the given process has already terminated.
		 * >>             // If it has, we wouldn't receive SIGCHLD
		 * >>             result = waitpid(pid, status, WNOHANG);
		 * >>             if (result != 0)
		 * >>                 break;
		 * >>             // TODO: Starting with the second iteration, read from `signalfd()',
		 * >>             //       since there may be other processes that could be dying
		 * >>             //       while we're waiting for ours.
		 * >>             // Poll (with timeout) the signalfd, which will become
		 * >>             // readable once our process got a SIGCHLD from a dying child
		 * >>             // TODO: Account for lost `tmo' during multiple iterations.
		 * >>             ppoll(pfd, 1, tmo, NULL);
		 * >>         }
		 * >>         // Restore the old signal mask.
		 * >>         sigprocmask(SIG_SETMASK, &old_ss, NULL);
		 * >>     }
		 * >>     return result;
		 * >> }
		 * >>
		 */
		/* TODO */
#else /* Proper */
		/* TODO: while (!DID_TIME_OUT) tryjoin(); */
#endif /* Poll-based */
		DeeError_NOTIMPLEMENTED();
		goto err;
	}
	if (result == 0) {
		atomic_and(&self->p_state, ~PROCESS_FDETACHING);
		return 1; /* Wait failed (timeout) */
	}
	if (result > 0)
		return 0; /* Success */
	result = DeeSystem_GetErrno();
#ifdef EINTR
	if (result == EINTR)
		goto again;
#endif /* EINTR */
	atomic_and(&self->p_state, ~PROCESS_FDETACHING);
	DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, result,
	                          "Failed to join process %k", self);
err:
	return -1;
}


/* @return: 1: Timeout */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
process_dojoin(Process *__restrict self,
               int *__restrict proc_result,
               uint64_t timeout_microseconds) {
	int result = 0;
	if (!(self->p_state & PROCESS_FDIDJOIN)) {
		uint64_t timeout_end = (uint64_t)-1;
		uint16_t state, new_flags;
		/* Always set the did-join and terminated flags in the end. */
		new_flags = (PROCESS_FDIDJOIN | PROCESS_FTERMINATED);
		if (timeout_microseconds != (uint64_t)-1) {
			timeout_end = DeeThread_GetTimeMicroSeconds();
			timeout_end += timeout_microseconds;
		}
		if (timeout_microseconds != 0 &&
		    DeeThread_CheckInterrupt())
			goto err;

		/* Wait for the process to terminate. */
		while (atomic_fetchor(&self->p_state, PROCESS_FDETACHING) &
		       PROCESS_FDETACHING) {
			uint64_t now;

			/* Idly wait for another process also in the process of joining this process. */
			if (timeout_microseconds == 0)
				return 1; /* Timeout */
			if (DeeThread_Sleep(1000))
				goto err; /* Error (interrupt delivery) */
			now = DeeThread_GetTimeMicroSeconds();
			if (now >= timeout_end)
				return 1; /* Timeout */

			/* Update the remaining timeout. */
			timeout_microseconds = timeout_end - now;
		}

		/* Check for case: The process was joined in the mean time. */
		if (self->p_state & PROCESS_FDIDJOIN)
			goto done_join;

		/* Check for case: The process has terminated, but was never ~actually~ detached. */
		if ((self->p_state & (PROCESS_FTERMINATED | PROCESS_FDETACHED)) ==
		    PROCESS_FTERMINATED)
			goto done_join;
		if (!(self->p_state & PROCESS_FSTARTED)) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Process %k has not been started",
			                self);
			goto err;
		}
		if (self->p_state & PROCESS_FDETACHED) {
			/* Special case: The process was detached, but not joined. */
			atomic_and(&self->p_state, ~PROCESS_FDETACHING);
			DeeError_Throwf(&DeeError_ValueError,
			                "Cannot join process %k after being detached",
			                self);
			goto err;
		}

		/* Do the actual wait! */
		result = wait_for_process(self, self->p_pid, timeout_microseconds);
		if (result != 0)
			goto done;
done_join:
		/* Delete the detaching-flag and set the detached-flag. */
		do {
			state = atomic_read(&self->p_state);
		} while (!atomic_cmpxch_weak(&self->p_state, state,
		                             (state & ~PROCESS_FDETACHING) | new_flags));
	}
	*proc_result = self->p_status;
done:
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_join(Process *self, size_t argc, DeeObject *const *argv) {
	int error, result;
	if (DeeArg_Unpack(argc, argv, ":" S_Process_function_join_name))
		goto err;
	do {
		error = process_dojoin(self, &result, (uint64_t)-1);
	} while (error > 0);
	if unlikely(error < 0)
		goto err;
	result = WIFEXITED(result) ? WEXITSTATUS(result) : 1;
	return DeeInt_NewUInt((unsigned int)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_tryjoin(Process *self, size_t argc, DeeObject *const *argv) {
	int error, result;
	if (DeeArg_Unpack(argc, argv, ":" S_Process_function_join_name))
		goto err;
	error = process_dojoin(self, &result, 0);
	if unlikely(error < 0)
		goto err;
	if (error > 0)
		return_none;
	result = WIFEXITED(result) ? WEXITSTATUS(result) : 1;
	return DeeInt_NewUInt((unsigned int)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_timedjoin(Process *self, size_t argc, DeeObject *const *argv) {
	int error, result;
	uint64_t timeout;
	if (DeeArg_Unpack(argc, argv, UNPd64 ":" S_Process_function_timedjoin_name, &timeout))
		goto err;
	error = process_dojoin(self, &result, timeout);
	if unlikely(error < 0)
		goto err;
	if (error > 0)
		return_none;
	result = WIFEXITED(result) ? WEXITSTATUS(result) : 1;
	return DeeInt_NewUInt((unsigned int)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_hasterminated(Process *self) {
	int error;
	uint16_t state;
	if (self->p_state & PROCESS_FTERMINATED)
		goto yes;
	if (!(self->p_state & PROCESS_FSTARTED))
		goto nope;
	if (self->p_state & PROCESS_FDIDJOIN)
		goto yes;
	if (atomic_fetchor(&self->p_state, PROCESS_FDETACHING) & PROCESS_FDETACHING)
		goto nope;
	error = tryjoinpid(self->p_pid, &self->p_status);
	if (error <= 0) {
		atomic_and(&self->p_state, ~PROCESS_FDETACHING);
		goto nope;
	}
	/* Delete the detaching-flag and set the detached-flag. */
	do {
		state = atomic_read(&self->p_state);
	} while (!atomic_cmpxch_weak(&self->p_state, state,
	                             (state & ~PROCESS_FDETACHING) |
	                             (PROCESS_FDIDJOIN | PROCESS_FTERMINATED)));
yes:
	return_true;
nope:
	return_false;
}


PRIVATE struct type_method tpconst process_methods[] = {
	TYPE_METHOD(S_Process_function_start_name, &process_start, S_Process_function_start_doc),
	TYPE_METHOD(S_Process_function_join_name, &process_join, S_Process_function_join_doc),
	TYPE_METHOD(S_Process_function_tryjoin_name, &process_tryjoin, S_Process_function_tryjoin_doc),
	TYPE_METHOD(S_Process_function_timedjoin_name, &process_timedjoin, S_Process_function_timedjoin_doc),
	TYPE_METHOD(S_Process_function_detach_name, &process_detach, S_Process_function_detach_doc),
	TYPE_METHOD(S_Process_function_terminate_name, &process_terminate, S_Process_function_terminate_doc),
	TYPE_METHOD_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_files(Process *__restrict self) {
	/* TODO: opendir("/proc/%d/fd/") */
	(void)self;
	ipc_unimplemented();
	return NULL;
}

#ifndef ERR_UNBOUND_ATTRIBUTE_DEFINED
#define ERR_UNBOUND_ATTRIBUTE_DEFINED 1
INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_unbound_attribute(DeeTypeObject *__restrict tp,
                      char const *__restrict name) {
	ASSERT_OBJECT(tp);
	ASSERT(DeeType_Check(tp));
	return DeeError_Throwf(&DeeError_UnboundAttribute,
	                       "Unbound attribute `%k.%s'",
	                       tp, name);
}
#endif /* !ERR_UNBOUND_ATTRIBUTE_DEFINED */

#ifndef STD_NAMES_DEFINED
#define STD_NAMES_DEFINED 1
PRIVATE char const std_names[][(sizeof(S_Process_getset_stdin_name) > sizeof(S_Process_getset_stdout_name)
                               ? (sizeof(S_Process_getset_stdin_name) > sizeof(S_Process_getset_stderr_name)
                                   ? sizeof(S_Process_getset_stdin_name)
                                   : sizeof(S_Process_getset_stderr_name))
                               : (sizeof(S_Process_getset_stdout_name) > sizeof(S_Process_getset_stderr_name)
                                   ? sizeof(S_Process_getset_stdout_name)
                                   : sizeof(S_Process_getset_stderr_name))) /
                               sizeof(char)] = {
	/* [DEE_STDIN ] = */ S_Process_getset_stdin_name,
	/* [DEE_STDOUT] = */ S_Process_getset_stdout_name,
	/* [DEE_STDERR] = */ S_Process_getset_stderr_name,
};
#endif /* !STD_NAMES_DEFINED */

#ifndef ERR_FILE_NOT_FOUND_DEFINED
#define ERR_FILE_NOT_FOUND_DEFINED 1
PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_file_not_found_ob(DeeObject *__restrict filename) {
	return DeeError_Throwf(&DeeError_FileNotFound,
	                       "File `%k' could not be found",
	                       filename);
}
#endif /* !ERR_FILE_NOT_FOUND_DEFINED */



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_file(Process *__restrict self, int stdno, int fd) {
#ifdef HAVE_UNIX_READLINK
	pid_t pid;
#endif /* HAVE_UNIX_READLINK */
	DREF DeeObject *result;
	if (self == &this_process)
		return DeeFile_GetStd(stdno);
	rwlock_read(&self->p_lock);
	if (!(self->p_state & PROCESS_FSTARTED)) {
		result = self->p_std[stdno];
		Dee_XIncref(result);
		rwlock_endread(&self->p_lock);
		if (!result)
			err_unbound_attribute(&DeeProcess_Type, std_names[stdno]);
		return result;
	}
#ifdef HAVE_UNIX_READLINK
	pid = self->p_pid;
#endif /* HAVE_UNIX_READLINK */
	rwlock_endread(&self->p_lock);
#ifdef HAVE_UNIX_READLINK
	{
		DREF DeeObject *result;
		DREF DeeObject *filename;
		filename = unix_readlinkf("/proc/%d/fd/%d", pid, fd);
		if unlikely(!filename)
			goto err;
		/* Try to open the linked file. */
		result = DeeFile_Open(filename, OPEN_FRDWR, 0);
		if (!result && DeeError_Catch(&DeeError_FSError))
			result = DeeFile_Open(filename, OPEN_FRDONLY, 0);
		if (result == ITER_DONE) {
			err_file_not_found_ob(filename);
			result = NULL;
		}
		Dee_Decref(filename);
		return result;
err:
		return NULL;
	}
#else /* HAVE_UNIX_READLINK */
	ipc_unimplemented();
	return NULL;
#endif /* !HAVE_UNIX_READLINK */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_file(Process *self, int stdno, DeeObject *value) {
	DREF DeeObject *old_stream;
	if (self == &this_process) {
		old_stream = DeeFile_SetStd(stdno, value);
		if (!old_stream) {
#ifdef CONFIG_ERROR_DELETE_UNBOUND
			if (value)
				goto done;
			err_unbound_attribute(&DeeProcess_Type, std_names[stdno]);
			goto err;
#else /* CONFIG_ERROR_DELETE_UNBOUND */
			goto done;
#endif /* !CONFIG_ERROR_DELETE_UNBOUND */
		}
		if (old_stream != ITER_DONE)
			Dee_Decref(old_stream);
		goto done;
	}
	rwlock_write(&self->p_lock);
	if (!(self->p_state & (PROCESS_FSTARTING | PROCESS_FSTARTED))) {
		Dee_XIncref(value);
		old_stream         = self->p_std[stdno];
		self->p_std[stdno] = value;
		rwlock_endwrite(&self->p_lock);
		if (!old_stream) {
			if (value)
				goto done;
			err_unbound_attribute(&DeeProcess_Type, std_names[stdno]);
		}
		Dee_Decref(old_stream);
done:
		return 0;
	}
	rwlock_endwrite(&self->p_lock);
	DeeError_Throwf(&DeeError_ValueError,
	                "Process %k has already been started",
	                self);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
err:
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	return -1;
}

#define DEFINE_PROCESS_STD_FUNCTIONS(stdxxx, DEE_STDXXX, fd) \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL       \
	process_get_##stdxxx(Process *__restrict self) {         \
		if (self == &this_process)                           \
			return DeeFile_GetStd(DEE_STDXXX);               \
		return process_get_file(self, DEE_STDXXX, fd);       \
	}                                                        \
	PRIVATE WUNUSED NONNULL((1)) int DCALL                   \
	process_del_##stdxxx(Process *__restrict self) {         \
		if (self == &this_process) {                         \
			DREF DeeObject *oldval;                          \
			oldval = DeeFile_SetStd(DEE_STDXXX, NULL);       \
			if (ITER_ISOK(oldval))                           \
				Dee_Decref(oldval);                          \
			return 0;                                        \
		}                                                    \
		return process_set_file(self, DEE_STDXXX, NULL);     \
	}                                                        \
	PRIVATE WUNUSED NONNULL((1, 2)) int DCALL                \
	process_set_##stdxxx(Process *self, DeeObject *value) {  \
		if (self == &this_process) {                         \
			DREF DeeObject *oldval;                          \
			oldval = DeeFile_SetStd(DEE_STDXXX, value);      \
			if (ITER_ISOK(oldval))                           \
				Dee_Decref(oldval);                          \
			return 0;                                        \
		}                                                    \
		return process_set_file(self, DEE_STDXXX, value);    \
	}
DEFINE_PROCESS_STD_FUNCTIONS(stdin, DEE_STDIN, STDIN_FILENO)
DEFINE_PROCESS_STD_FUNCTIONS(stdout, DEE_STDOUT, STDOUT_FILENO)
DEFINE_PROCESS_STD_FUNCTIONS(stderr, DEE_STDERR, STDERR_FILENO)
#undef DEFINE_PROCESS_STD_FUNCTIONS

/*[[[deemon
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_posix", "posix");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_getcwd", "getcwd");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_chdir", "chdir");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_environ", "environ");
]]]*/
PRIVATE DEFINE_STRING_EX(str_posix, "posix", 0x8a12ee56, 0xfc8c64936b261e96);
PRIVATE DEFINE_STRING_EX(str_getcwd, "getcwd", 0x2a8d58b5, 0xd71ca646f3668c32);
PRIVATE DEFINE_STRING_EX(str_chdir, "chdir", 0xd4e07810, 0x5d1f1c76d494fde6);
PRIVATE DEFINE_STRING_EX(str_environ, "environ", 0xd8ecb380, 0x8d2a0a9c2432f221);
/*[[[end]]]*/

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_threads(Process *__restrict self) {
	if (self == &this_process)
		return DeeObject_GetAttrString((DeeObject *)&DeeThread_Type, "enumerate");
	ipc_unimplemented();
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_environ_from_nulterm_bytes(char const *data,
                                       size_t datalen) {
	DREF DeeObject *result;
	char const *end = data + datalen;
	result = DeeDict_New();
	while (data < end) {
		int error;
		DREF DeeObject *value;
		size_t len = strnlen(data, (size_t)(end - data));
		char const *valpos = strchr(data, '=');
		size_t keylen, vallen;
		if (valpos) {
			keylen = (size_t)(valpos - data);
			++valpos;
			vallen = len - (keylen + 1);
			value = DeeString_NewUtf8(valpos, vallen, STRING_ERROR_FIGNORE);
			if unlikely(!value)
				goto err;
		} else {
			keylen = len;
			value  = Dee_EmptyString;
			Dee_Incref(value);
		}
		error = DeeDict_SetItemStringLen(result, data, keylen,
		                                 Dee_HashUtf8(data, keylen),
		                                 value);
		Dee_Decref(value);
		if unlikely(error)
			goto err;
		data += len + 1;
		goto err;
	}
	return result;
err:
	Dee_Decref(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_environ(Process *__restrict self) {
	pid_t pid;
	DREF DeeObject *result, *filename, *fp, *content;
	if (self == &this_process) {
		return DeeModule_GetExtern((DeeObject *)&str_posix,
		                           (DeeObject *)&str_environ);
	}
	rwlock_read(&self->p_lock);
	if (!(self->p_state & PROCESS_FSTARTED)) {
		result = self->p_environ;
		Dee_XIncref(result);
		rwlock_endread(&self->p_lock);
		if (!result)
			err_unbound_attribute(&DeeProcess_Type, DeeString_STR(&str_environ));
		return result;
	}
	pid = self->p_pid;
	rwlock_endread(&self->p_lock);
	filename = DeeString_Newf("/proc/%d/environ", pid);
	if unlikely(!filename)
		goto err;
	fp = DeeFile_Open(filename, OPEN_FRDONLY, 0);
	if (!ITER_ISOK(fp)) {
		if (fp == ITER_DONE)
			err_file_not_found_ob(filename);
		Dee_Decref(filename);
		goto err;
	}
	Dee_Decref(filename);
	content = DeeFile_ReadBytes(fp, (size_t)-1, true);
	Dee_Decref(fp);
	if unlikely(!content)
		goto err;
	result = process_get_environ_from_nulterm_bytes((char const *)DeeBytes_DATA(content),
	                                                DeeBytes_SIZE(content));
	Dee_Decref(content);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_environ(Process *self, DeeObject *value) {
	DREF DeeObject *temp;
	if (self == &this_process) {
		int result;
		temp = DeeModule_GetExtern((DeeObject *)&str_posix,
		                           (DeeObject *)&str_environ);
		if unlikely(!temp)
			goto err;
		result = DeeObject_Assign(temp, value ? value : Dee_None);
		Dee_Decref(temp);
		return result;
	}
	rwlock_write(&self->p_lock);
	if (!(self->p_state & PROCESS_FSTARTED)) {
		temp            = self->p_environ;
		self->p_environ = value;
		Dee_XIncref(value);
		rwlock_endwrite(&self->p_lock);
		Dee_XDecref(temp);
		return 0;
	}
	rwlock_endwrite(&self->p_lock);
	DeeError_Throwf(&DeeError_ValueError,
	                "Cannot %s environ of running process %k",
	                value ? "change"
	                      : "delete",
	                self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_memory(Process *__restrict self) {
#if 0
	if (self == &this_process)
		; /* TODO */
#endif
	(void)self;
	/* TODO: /proc/%d/mem */
	ipc_unimplemented();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_environ(Process *__restrict self) {
	return process_set_environ(self, NULL);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_pwd(Process *__restrict self) {
#ifdef HAVE_UNIX_READLINK
	pid_t pid;
#endif /* HAVE_UNIX_READLINK */
	DREF DeeStringObject *result;
	if (self == &this_process) {
		return DeeModule_CallExtern((DeeObject *)&str_posix,
		                            (DeeObject *)&str_getcwd,
		                            0, NULL);
	}
	rwlock_read(&self->p_lock);
	if (!(self->p_state & PROCESS_FSTARTED)) {
		result = self->p_pwd;
		Dee_XIncref(result);
		rwlock_endread(&self->p_lock);
		if (!result)
			err_unbound_attribute(&DeeProcess_Type, S_Process_getset_pwd_name);
		return (DREF DeeObject *)result;
	}
#ifdef HAVE_UNIX_READLINK
	pid = self->p_pid;
#endif /* HAVE_UNIX_READLINK */
	rwlock_endread(&self->p_lock);
#ifdef HAVE_UNIX_READLINK
	return unix_readlinkf("/proc/%d/cwd", pid);
#else /* HAVE_UNIX_READLINK */
	ipc_unimplemented();
	return NULL;
#endif /* !HAVE_UNIX_READLINK */
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_pwd(Process *self, DeeObject *value) {
	DREF DeeStringObject *oldval;
	if (value && DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto err;
	if (self == &this_process) {
		DREF DeeObject *temp;
		if unlikely(!value)
			goto err_running;
		temp = DeeModule_CallExtern((DeeObject *)&str_posix,
		                            (DeeObject *)&str_chdir,
		                            1, &value);
		if unlikely(!temp)
			goto err;
		Dee_Decref(temp);
		return 0;
	}
	rwlock_write(&self->p_lock);
	if (!(self->p_state & (PROCESS_FSTARTING | PROCESS_FSTARTED))) {
		Dee_XIncref(value);
		oldval      = self->p_pwd;
		self->p_pwd = (DREF DeeStringObject *)value;
		rwlock_endwrite(&self->p_lock);
		if (oldval) {
			Dee_Decref(oldval);
		}
#ifdef CONFIG_ERROR_DELETE_UNBOUND
		else if (!value) {
			err_unbound_attribute(&DeeProcess_Type, S_Process_getset_pwd_name);
			goto err;
		}
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
		return 0;
	}
	rwlock_endwrite(&self->p_lock);
err_running:
	DeeError_Throwf(&DeeError_ValueError,
	                "Cannot %s pwd of running process %k",
	                value ? "change"
	                      : "delete",
	                self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_pwd(Process *__restrict self) {
	return process_set_pwd(self, NULL);
}

#ifdef HAVE_UNIX_READLINK
PRIVATE DREF DeeObject *DCALL process_get_self_exe(void) {
	return unix_readlink("/proc/self/exe");
}
#endif /* HAVE_UNIX_READLINK */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_exe(Process *__restrict self) {
	DREF DeeStringObject *result;
#ifdef HAVE_UNIX_READLINK
	pid_t pid;
#endif /* HAVE_UNIX_READLINK */
	if (self == &this_process) {
#ifdef HAVE_UNIX_READLINK
		return process_get_self_exe();
#else /* HAVE_UNIX_READLINK */
		result = DeeModule_GetDeemon()->mo_name;
		return_reference_((DREF DeeObject *)result);
#endif /* !HAVE_UNIX_READLINK */
	}
	rwlock_read(&self->p_lock);
	result = self->p_exe;
	if (result) {
		Dee_Incref(result);
		rwlock_endread(&self->p_lock);
done:
		return (DeeObject *)result;
	}
	if (!(self->p_state & PROCESS_FSTARTED)) {
		rwlock_endread(&self->p_lock);
		err_unbound_attribute(&DeeProcess_Type, S_Process_getset_exe_name);
		goto done;
	}
#ifdef HAVE_UNIX_READLINK
	pid = self->p_pid;
	rwlock_endread(&self->p_lock);
	result = (DREF DeeStringObject *)unix_readlinkf("/proc/%d/exe", pid);
	if unlikely(!result)
		goto done;
	rwlock_write(&self->p_lock);
	if (!self->p_exe) {
		Dee_Incref(result);
		self->p_exe = result;
		rwlock_endwrite(&self->p_lock);
	} else {
		DREF DeeStringObject *new_result;
		new_result = self->p_exe;
		Dee_Incref(new_result);
		rwlock_endwrite(&self->p_lock);
		Dee_Decref(result);
		result = new_result;
	}
	goto done;
#else /* HAVE_UNIX_READLINK */
	rwlock_endread(&self->p_lock);
	ipc_unimplemented();
	return NULL;
#endif /* !HAVE_UNIX_READLINK */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_exe(Process *self, DeeObject *value) {
	DREF DeeStringObject *old_value;
	if (value && DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto err;
	if (self == &this_process) {
err_started:
		DeeError_Throwf(&DeeError_ValueError,
		                "Cannot set exe for running process %k",
		                self);
		goto err;
	}
	rwlock_write(&self->p_lock);
	if (self->p_state & (PROCESS_FSTARTING | PROCESS_FSTARTED)) {
		rwlock_endwrite(&self->p_lock);
		goto err_started;
	}
	Dee_XIncref(value);
	old_value   = self->p_exe;
	self->p_exe = (DREF DeeStringObject *)value;
	rwlock_endwrite(&self->p_lock);
	if (old_value) {
		Dee_Decref(old_value);
	}
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	else if (!value) {
		err_unbound_attribute(&DeeProcess_Type, S_Process_getset_exe_name);
		goto err;
	}
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_exe(Process *__restrict self) {
	return process_set_exe(self, NULL);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
process_get_my_cmdline_from_argv(void) {
	size_t i;
	DREF DeeObject *argv = Dee_GetArgv();
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	DeeStringObject *deemon_str = DeeModule_GetDeemon()->mo_name;
	if (unicode_printer_print(&printer,
	                          DeeString_STR(deemon_str),
	                          DeeString_SIZE(deemon_str)) < 0)
		goto err_printer;
	for (i = 0; i < DeeTuple_SIZE(argv); ++i) {
		char *arg;
		if (unicode_printer_putascii(&printer, ' '))
			goto err_printer;
		ASSERT_OBJECT_TYPE_EXACT(DeeTuple_GET(argv, i), &DeeString_Type);
		arg = DeeString_AsUtf8(DeeTuple_GET(argv, i));
		if unlikely(!arg)
			goto err_printer;
		if (process_cmdline_encode_argument(&unicode_printer_print,
		                                    &printer,
		                                    arg, WSTR_LENGTH(arg)) < 0)
			goto err_printer;
	}
	Dee_Decref(argv);
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	Dee_Decref(argv);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
process_get_cmdline_from_argv_fast(struct unicode_printer *__restrict printer,
                                   DeeObject *__restrict seq, size_t fastlen) {
	size_t i;
	DREF DeeObject *elem;
	for (i = 0; i < fastlen; ++i) {
		char *utf8;
		if (unicode_printer_putascii(printer, ' '))
			goto err;
		elem = DeeFastSeq_GetItem(seq, i);
		if unlikely(!elem)
			goto err;
		if (DeeObject_AssertTypeExact(elem, &DeeString_Type))
			goto err;
		utf8 = DeeString_AsUtf8(elem);
		if unlikely(!utf8)
			goto err_elem;
		if (process_cmdline_encode_argument(&unicode_printer_print, printer,
		                                    utf8, WSTR_LENGTH(utf8)) < 0)
			goto err_elem;
		Dee_Decref(elem);
	}
	return 0;
err_elem:
	Dee_Decref(elem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
process_get_cmdline_from_argv_iter(struct unicode_printer *__restrict printer,
                                   DeeObject *__restrict iterator) {
	DREF DeeObject *elem;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		char *utf8;
		if (unicode_printer_putascii(printer, ' '))
			goto err_elem;
		if (DeeObject_AssertTypeExact(elem, &DeeString_Type))
			goto err;
		utf8 = DeeString_AsUtf8(elem);
		if unlikely(!utf8)
			goto err_elem;
		if (process_cmdline_encode_argument(&unicode_printer_print, printer,
		                                    utf8, WSTR_LENGTH(utf8)) < 0)
			goto err_elem;
		Dee_Decref(elem);
	}
	if unlikely(!elem)
		goto err;
	return 0;
err_elem:
	Dee_Decref(elem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
process_get_cmdline_from_argv(char const *__restrict utf8_exe,
                              DeeObject *__restrict argv) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	size_t fastlen;
	DREF DeeObject *iterator;
	if (process_cmdline_encode_argument(&unicode_printer_print, &printer,
	                                    utf8_exe, WSTR_LENGTH(utf8_exe)) < 0)
		goto err;
	fastlen = DeeFastSeq_GetSize(argv);
	if (fastlen != DEE_FASTSEQ_NOTFAST) {
		if unlikely(process_get_cmdline_from_argv_fast(&printer, argv, fastlen))
			goto err;
	} else {
		iterator = DeeObject_IterSelf(argv);
		if unlikely(!iterator)
			goto err;
		if unlikely(process_get_cmdline_from_argv_iter(&printer, iterator))
			goto err_iter;
		Dee_Decref(iterator);
	}
	return unicode_printer_pack(&printer);
err_iter:
	Dee_Decref(iterator);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
process_get_cmdline_from_nulterm_bytes(char const *data,
                                       size_t datalen) {
	bool is_first;
	char const *end = data + datalen;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	DeeStringObject *deemon_str = DeeModule_GetDeemon()->mo_name;
	if (unicode_printer_print(&printer,
	                          DeeString_STR(deemon_str),
	                          DeeString_SIZE(deemon_str)) < 0)
		goto err_printer;
	is_first = true;
	while (data < end) {
		size_t len;
		if (!is_first) {
			if (unicode_printer_putascii(&printer, ' '))
				goto err_printer;
		}
		is_first = false;
		len = strnlen(data, (size_t)(end - data));
		if (process_cmdline_encode_argument(&unicode_printer_print,
		                                    &printer,
		                                    data, len) < 0)
			goto err_printer;
		data += len + 1;
	}
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_cmdline(Process *__restrict self) {
	DREF DeeObject *result, *fp, *content;
	if (self == &this_process) {
		fp = DeeFile_OpenString("/proc/self/cmdline", OPEN_FRDONLY, 0);
		if (!ITER_ISOK(fp)) {
			if (!fp)
				goto err;
			return process_get_my_cmdline_from_argv();
		}
	} else {
		DREF DeeStringObject *exe;
		DREF DeeObject *filename;
		pid_t pid;
		rwlock_read(&self->p_lock);
		exe     = self->p_exe;
		content = self->p_argv;
		if (exe && content) {
			char *utf8_exe;
			Dee_Incref(exe);
			Dee_Incref(content);
			rwlock_endread(&self->p_lock);
			utf8_exe = DeeString_AsUtf8((DeeObject *)exe);
			if unlikely(!utf8_exe) {
				result = NULL;
			} else {
				result = process_get_cmdline_from_argv(utf8_exe, content);
			}
			Dee_Decref(exe);
			Dee_Decref(content);
			return (DeeObject *)result;
		}
		if (!(self->p_state & PROCESS_FSTARTED)) {
			rwlock_endread(&self->p_lock);
			err_unbound_attribute(&DeeProcess_Type, S_Process_getset_cmdline_name);
			goto err;
		}
		pid = self->p_pid;
		rwlock_endread(&self->p_lock);
		filename = DeeString_Newf("/proc/%d/cmdline", pid);
		if unlikely(!filename)
			goto err;
		fp = DeeFile_Open(filename, OPEN_FRDONLY, 0);
		if (!ITER_ISOK(fp)) {
			if (fp)
				err_file_not_found_ob(filename);
			Dee_Decref(filename);
			goto err;
		}
		Dee_Decref(filename);
	}
	content = DeeFile_ReadBytes(fp, (size_t)-1, true);
	Dee_Decref(fp);
	if unlikely(!content)
		goto err;
	result = process_get_cmdline_from_nulterm_bytes((char const *)DeeBytes_DATA(content),
	                                                DeeBytes_SIZE(content));
	Dee_Decref(content);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
process_set_cmdline(Process *self, DeeObject *value) {
	DREF DeeObject *old_argv, *new_argv;
	DREF DeeStringObject *old_exe, *new_exe;
	/*utf-8*/ char *cmdline_utf8;
	if (DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto err;
	if (self == &this_process) {
err_started:
		DeeError_Throwf(&DeeError_ValueError,
		                "Cannot set cmdline for running process %k",
		                self);
		goto err;
	}
	if (atomic_read(&self->p_state) & (PROCESS_FSTARTING | PROCESS_FSTARTED))
		goto err_started;
	cmdline_utf8 = DeeString_AsUtf8(value);
	if unlikely(!cmdline_utf8)
		goto err;
	new_argv = process_cmdline_decode_full(cmdline_utf8, (DREF DeeObject **)&new_exe);
	if unlikely(!new_argv)
		goto err;
	if (DeeString_IsEmpty(new_exe))
		Dee_Clear(new_exe);
	rwlock_write(&self->p_lock);
	if (self->p_state & (PROCESS_FSTARTING | PROCESS_FSTARTED)) {
		rwlock_endwrite(&self->p_lock);
		Dee_XDecref(new_exe);
		Dee_Decref(new_argv);
		goto err_started;
	}
	old_exe      = self->p_exe;
	old_argv     = self->p_argv;
	self->p_exe  = new_exe;
	self->p_argv = new_argv;
	rwlock_endwrite(&self->p_lock);
	Dee_XDecref(old_exe);
	Dee_XDecref(old_argv);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_cmdline(Process *__restrict self) {
	return process_set_cmdline(self, Dee_EmptyString);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
process_get_my_argv_from_argv(void) {
	size_t i;
	DREF DeeObject *result, *temp;
	DREF DeeObject *argv = Dee_GetArgv();
	result = DeeTuple_NewUninitialized(DeeTuple_SIZE(argv) + 1);
	if unlikely(!result)
		goto err;
	temp = (DREF DeeObject *)DeeModule_GetDeemon()->mo_name;
	Dee_Incref(temp);
	DeeTuple_SET(result, 0, temp); /* Inherit reference */
	for (i = 0; i < DeeTuple_SIZE(argv); ++i) {
		temp = DeeTuple_GET(argv, i);
		Dee_Incref(temp);
		DeeTuple_SET(result, i + 1, temp); /* Inherit reference */
	}
	Dee_Decref(argv);
	return result;
err:
	Dee_Decref(argv);
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
process_get_argv_from_nulterm_bytes(char const *__restrict data,
                                    size_t datalen) {
	bool is_first;
	char const *end = data + datalen;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	DeeStringObject *deemon_str = DeeModule_GetDeemon()->mo_name;
	if (unicode_printer_print(&printer,
	                          DeeString_STR(deemon_str),
	                          DeeString_SIZE(deemon_str)) < 0)
		goto err_printer;
	is_first = true;
	while (data < end) {
		size_t len;
		if (!is_first) {
			if (unicode_printer_putascii(&printer, ' '))
				goto err_printer;
		}
		is_first = false;
		len = strnlen(data, (size_t)(end - data));
		if (process_cmdline_encode_argument(&unicode_printer_print,
		                                    &printer,
		                                    data, len) < 0)
			goto err_printer;
		data += len + 1;
	}
	return unicode_printer_pack(&printer);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_argv(Process *__restrict self) {
	DREF DeeObject *result, *fp, *content;
	if (self == &this_process) {
		fp = DeeFile_OpenString("/proc/self/cmdline", OPEN_FRDONLY, 0);
		if (!ITER_ISOK(fp)) {
			if (!fp)
				goto err;
			return process_get_my_argv_from_argv();
		}
	} else {
		DREF DeeObject *filename;
		pid_t pid;
		rwlock_read(&self->p_lock);
		result = self->p_argv;
		if (result) {
			Dee_Incref(result);
			rwlock_endread(&self->p_lock);
			return (DeeObject *)result;
		}
		if (!(self->p_state & PROCESS_FSTARTED)) {
			rwlock_endread(&self->p_lock);
			err_unbound_attribute(&DeeProcess_Type, S_Process_getset_argv_name);
			goto err;
		}
		pid = self->p_pid;
		rwlock_endread(&self->p_lock);
		filename = DeeString_Newf("/proc/%d/cmdline", pid);
		if unlikely(!filename)
			goto err;
		fp = DeeFile_Open(filename, OPEN_FRDONLY, 0);
		if (!ITER_ISOK(fp)) {
			if (fp)
				err_file_not_found_ob(filename);
			Dee_Decref(filename);
			goto err;
		}
		Dee_Decref(filename);
	}
	content = DeeFile_ReadBytes(fp, (size_t)-1, true);
	Dee_Decref(fp);
	if unlikely(!content)
		goto err;
	result = process_get_argv_from_nulterm_bytes((char const *)DeeBytes_DATA(content),
	                                             DeeBytes_SIZE(content));
	Dee_Decref(content);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_argv(Process *self, DeeObject *value) {
	DREF DeeObject *old_argv;
	if (self == &this_process) {
err_started:
		DeeError_Throwf(&DeeError_ValueError,
		                "Cannot set cmdline for running process %k",
		                self);
		goto err;
	}
	rwlock_write(&self->p_lock);
	if (self->p_state & (PROCESS_FSTARTING | PROCESS_FSTARTED)) {
		rwlock_endwrite(&self->p_lock);
		goto err_started;
	}
	Dee_Incref(value);
	old_argv     = self->p_argv;
	self->p_argv = value;
	rwlock_endwrite(&self->p_lock);
	Dee_XDecref(old_argv);
	return 0;
err:
	return -1;
}


PRIVATE struct type_getset tpconst process_getsets[] = {
	TYPE_GETSET(S_Process_getset_stdin_name, &process_get_stdin, &process_del_stdin, &process_set_stdin, S_Process_getset_stdin_doc),
	TYPE_GETSET(S_Process_getset_stdout_name, &process_get_stdout, &process_del_stdout, &process_set_stdout, S_Process_getset_stdout_doc),
	TYPE_GETSET(S_Process_getset_stderr_name, &process_get_stderr, &process_del_stderr, &process_set_stderr, S_Process_getset_stderr_doc),
	TYPE_GETSET(S_Process_getset_pwd_name, &process_get_pwd, &process_del_pwd, &process_set_pwd, S_Process_getset_pwd_doc),
	TYPE_GETSET(S_Process_getset_exe_name, &process_get_exe, &process_del_exe, &process_set_exe, S_Process_getset_exe_doc),
	TYPE_GETSET(S_Process_getset_cmdline_name, &process_get_cmdline, &process_del_cmdline, &process_set_cmdline, S_Process_getset_cmdline_doc),
	TYPE_GETSET(S_Process_getset_argv_name, &process_get_argv, &process_del_cmdline, &process_set_argv, S_Process_getset_argv_doc),
	TYPE_GETSET(S_Process_getset_cmd_name, &process_get_cmdline, &process_del_cmdline, &process_set_cmdline, S_Process_getset_cmd_doc),
	TYPE_GETSET(S_Process_getset_environ_name, &process_get_environ, &process_del_environ, &process_set_environ, S_Process_getset_environ_doc),
	TYPE_GETTER(S_Process_getset_hasterminated_name, &process_hasterminated, S_Process_getset_hasterminated_doc),
	TYPE_GETTER(S_Process_getset_id_name, &process_id, S_Process_getset_id_doc),
	TYPE_GETTER(S_Process_getset_threads_name, &process_get_threads, S_Process_getset_threads_doc),
	TYPE_GETTER(S_Process_getset_files_name, &process_get_files, S_Process_getset_files_doc),
	TYPE_GETTER(S_Process_getset_memory_name, &process_get_memory, S_Process_getset_memory_doc),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst process_members[] = {
	TYPE_MEMBER_BITFIELD_DOC(S_Process_getset_hasstarted_name,
	                         STRUCT_CONST, Process, p_state, PROCESS_FSTARTED,
	                         S_Process_getset_hasstarted_doc),
	TYPE_MEMBER_BITFIELD_DOC(S_Process_getset_wasdetached_name,
	                         STRUCT_CONST, Process, p_state, PROCESS_FDETACHED,
	                         S_Process_getset_wasdetached_doc),
	TYPE_MEMBER_BITFIELD_DOC(S_Process_getset_isachild_name,
	                         STRUCT_CONST, Process, p_state, PROCESS_FCHILD,
	                         S_Process_getset_isachild_doc),
	TYPE_MEMBER_END
};



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_class_self(DeeObject *UNUSED(self), size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":" S_Process_class_function_self_name))
		goto err;
	return_reference_((DeeObject *)&this_process);
err:
	return NULL;
}

PRIVATE struct type_method tpconst process_class_methods[] = {
	TYPE_METHOD(S_Process_class_function_self_name, &process_class_self, S_Process_class_function_self_doc),
	TYPE_METHOD_END
};

PRIVATE struct type_member tpconst process_class_members[] = {
	TYPE_MEMBER_CONST_DOC(S_Process_member_current_name, &this_process, S_Process_member_current_doc),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
process_init(Process *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DREF DeeStringObject *exe;
	DREF DeeObject *args = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:" S_Process_tp_name, &exe, &args))
		goto err;
	if (DeeString_Check(exe)) {
		if (args) {
			Dee_Incref(exe);
			Dee_Incref(args);
		} else {
			/* exe is a commandline
			 * TODO: This doesn't match the behavior of the windows implementation,
			 *       which uses `exe' as a single commandline argument in this case. */
			char *cmdline;
			cmdline = DeeString_AsUtf8((DeeObject *)exe);
			if unlikely(!cmdline)
				goto err;
			args = process_cmdline_decode_full(cmdline, (DREF DeeObject **)&exe);
			if unlikely(!args)
				goto err;
		}
	} else {
#ifdef HAVE_UNIX_READLINK
		if (DeeInt_Check(exe)) {
			int fd;
			if (DeeObject_AsInt((DeeObject *)exe, &fd))
				goto err;
			exe = (DREF DeeStringObject *)unix_readlinkf("/proc/self/fd/%d", fd);
		} else
#endif /* HAVE_UNIX_READLINK */
		{
			exe = (DREF DeeStringObject *)DeeFile_Filename((DeeObject *)exe);
		}
		if unlikely(!exe)
			goto err;
	}
	rwlock_init(&self->p_lock);
	self->p_std[0]  = NULL;
	self->p_std[1]  = NULL;
	self->p_std[2]  = NULL;
	self->p_environ = NULL;
	self->p_pwd     = NULL;
	self->p_exe     = exe;  /* Inherit reference */
	self->p_argv    = args; /* Inherit reference */
	self->p_pid     = 0;
	self->p_state   = PROCESS_FNORMAL;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
process_fini(Process *__restrict self) {
#ifdef CONFIG_HAVE_detach
	if ((self->p_state & (PROCESS_FSTARTED | PROCESS_FDETACHED)) == PROCESS_FSTARTED)
		detach(self->p_pid);
#endif /* CONFIG_HAVE_detach */
	Dee_XDecref(self->p_std[0]);
	Dee_XDecref(self->p_std[1]);
	Dee_XDecref(self->p_std[2]);
	Dee_XDecref(self->p_environ);
	Dee_XDecref(self->p_pwd);
	Dee_XDecref(self->p_exe);
	Dee_XDecref(self->p_argv);
}

PRIVATE NONNULL((1, 2)) void DCALL
process_visit(Process *__restrict self, dvisit_t proc, void *arg) {
	Dee_XVisit(self->p_std[0]);
	Dee_XVisit(self->p_std[1]);
	Dee_XVisit(self->p_std[2]);
	Dee_XVisit(self->p_environ);
	Dee_XVisit(self->p_pwd);
	Dee_XVisit(self->p_exe);
	Dee_XVisit(self->p_argv);
}

INTERN DeeTypeObject DeeProcess_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_Process_tp_name,
	/* .tp_doc      = */ S_Process_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&process_init,
				TYPE_FIXED_ALLOCATOR(Process)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&process_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&process_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
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



/* TODO: opendir("/proc") */
INTERN DeeTypeObject DeeProcEnumIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_ProcEnumIterator_tp_name,
	/* .tp_doc      = */ S_ProcEnumIterator_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject) /* TODO */
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL, /* TODO */
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE struct type_member tpconst enumproc_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeProcEnumIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeProcEnum_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_ProcEnum_tp_name,
	/* .tp_doc      = */ S_ProcEnum_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL, /* TODO */
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ enumproc_class_members
};

DECL_END

#ifndef __INTELLISENSE__
#include "unix-pipe.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEX_IPC_UNIX_C_INL */
