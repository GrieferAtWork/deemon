/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_IPC_WINDOWS_C_INL
#define GUARD_DEX_IPC_WINDOWS_C_INL 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1

#include "libipc.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/atomic.h>
#include <hybrid/minmax.h>
#include <hybrid/sched/yield.h>

#include <string.h>
#include <tlhelp32.h>
#include <wchar.h>

#include "_res.h"

DECL_BEGIN

INTDEF WUNUSED NONNULL((1, 2)) int DCALL
cmdline_add_arg(struct ascii_printer *__restrict printer,
                DeeStringObject *__restrict arg);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
cmdline_add_args(struct ascii_printer *__restrict printer,
                 DeeObject *__restrict args);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmdline_split(DeeStringObject *__restrict cmdline);



/* @param: procenv: {(string,string)...} */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
print_environ(struct ascii_printer *__restrict printer,
              DeeObject *__restrict procenv) {
	DREF DeeObject *item, *iter;
	DREF DeeObject *name_and_value[2];
	iter = DeeObject_IterSelf(procenv);
	while (ITER_ISOK(item = DeeObject_IterNext(iter))) {
		if (DeeObject_Unpack(item, 2, name_and_value))
			goto err_item;
		if (DeeObject_AssertTypeExact(name_and_value[0], &DeeString_Type) ||
		    DeeObject_AssertTypeExact(name_and_value[1], &DeeString_Type))
			goto err_name_and_value;
		if (ascii_printer_print(printer,
		                        DeeString_STR(name_and_value[0]),
		                        DeeString_SIZE(name_and_value[0])) < 0)
			goto err_name_and_value;
		if (ascii_printer_putc(printer, '='))
			goto err_name_and_value;
		if (ascii_printer_print(printer,
		                        DeeString_STR(name_and_value[1]),
		                        DeeString_SIZE(name_and_value[1])) < 0)
			goto err_name_and_value;
		if (ascii_printer_putc(printer, '\0'))
			goto err_name_and_value;
		Dee_Decref(name_and_value[1]);
		Dee_Decref(name_and_value[0]);
		Dee_Decref(item);
	}
	Dee_Decref(iter);
	if unlikely(!item)
		goto err;
	if (ascii_printer_putc(printer, '\0'))
		goto err;
	return 0;
err_name_and_value:
	Dee_Decref(name_and_value[1]);
	Dee_Decref(name_and_value[0]);
err_item:
	Dee_Decref(item);
	Dee_Decref(iter);
err:
	return -1;
}

/* Pack the given procenv into an environment string. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
pack_environ(DeeObject *__restrict procenv) {
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	if unlikely(print_environ(&printer, procenv))
		goto err;
	return ascii_printer_pack(&printer);
err:
	ascii_printer_fini(&printer);
	return NULL;
}



typedef struct {
	OBJECT_HEAD /* GC Object */
	rwlock_t                   p_lock;    /* Lock for accessing members of this structure. */
	/* NOTE: None of the following may be set to a
	 *       non-NULL value when `PROCESS_FSTARTING' is set. */
	DREF DeeObject            *p_std[3];  /* [lock(p_lock)][0..1] STD streams for when the process will be running (the handle is loaded by calling `.fileno()'). */
	DREF DeeObject            *p_environ; /* [lock(p_lock)][0..1] The environment block used by the process (or `NULL' to refer to that of the calling process).
	                                       * NOTE: When set, this is a sequence type compatible with `fs.environ': `{string: string}' */
	DREF DeeStringObject      *p_pwd;     /* [lock(p_lock)][0..1] The process working directory (or `NULL' to refer to that of the calling process). */
	DREF DeeStringObject      *p_exe;     /* [lock(p_lock)][0..1] The filename to an executable that should be run by the process. */
	DREF DeeStringObject      *p_cmdline; /* [lock(p_lock)][0..1] The commandline of the process (VC/VC++ compatible). */
	HANDLE                     p_handle;  /* [lock(p_lock)][const_if(PROCESS_FSTARTED)][owned]
	                                       * [valid_if(PROCESS_FSTARTED && != NULL)]
	                                       * A handle to the process. */
	DWORD                      p_id;      /* [valid_if(PROCESS_FSTARTED)] The ID of the process (pid) */
#define PROCESS_FNORMAL        0x0000     /* Normal process flags. */
#define PROCESS_FSTARTED       0x0001     /* The process has been started. (Always set for external processes) */
#define PROCESS_FDETACHED      0x0002     /* The process has been detached (`p_id' is dangling and `p_handle' is invalid). */
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
	/* .p_exe     = */ NULL,      /* Always NULL - Read using `GetModuleFileNameW()'. */
	/* .p_cmdline = */ NULL,      /* Always NULL - Read using `GetCommandLineW()'. */
	/* .p_handle  = */ NULL,      /* Lazily loaded. */
	/* .p_id      = */ (DWORD)-1, /* Lazily loaded. */
	/* .p_state   = */ PROCESS_FSTARTED
};



PRIVATE NONNULL((1)) void DCALL
process_fini(Process *__restrict self) {
	if ((self->p_state & PROCESS_FSTARTED) &&
	    self->p_handle != NULL) {
		DBG_ALIGNMENT_DISABLE();
		CloseHandle(self->p_handle);
		DBG_ALIGNMENT_ENABLE();
	}
	Dee_XDecref(self->p_std[0]);
	Dee_XDecref(self->p_std[1]);
	Dee_XDecref(self->p_std[2]);
	Dee_XDecref(self->p_environ);
	Dee_XDecref(self->p_pwd);
	Dee_XDecref(self->p_exe);
	Dee_XDecref(self->p_cmdline);
}

PRIVATE NONNULL((1, 2)) void DCALL
process_visit(Process *__restrict self, dvisit_t proc, void *arg) {
	rwlock_read(&self->p_lock);
	Dee_XVisit(self->p_std[0]);
	Dee_XVisit(self->p_std[1]);
	Dee_XVisit(self->p_std[2]);
	Dee_XVisit(self->p_environ);
	rwlock_endread(&self->p_lock);
}

PRIVATE NONNULL((1)) void DCALL
process_clear(Process *__restrict self) {
	DREF DeeObject *obj[4];
	rwlock_write(&self->p_lock);
	obj[0]          = self->p_std[0];
	obj[1]          = self->p_std[1];
	obj[2]          = self->p_std[2];
	obj[3]          = self->p_environ;
	self->p_std[0]  = NULL;
	self->p_std[1]  = NULL;
	self->p_std[2]  = NULL;
	self->p_environ = NULL;
	rwlock_endwrite(&self->p_lock);
	Dee_XDecref(obj[3]);
	Dee_XDecref(obj[2]);
	Dee_XDecref(obj[1]);
	Dee_XDecref(obj[0]);
}




PRIVATE HANDLE DCALL
get_stdhandle_for_process(DeeObject *procfd, int stdnum) {
	HANDLE result;
	if (procfd) {
		result = (HANDLE)DeeNTSystem_GetHandle(procfd);
	} else {
		result = ((DeeSystemFileObject *)DeeFile_DefaultStd(stdnum))->sf_handle;
		ASSERT(result != INVALID_HANDLE_VALUE); /* XXX: Does this always hold up? */
	}
	if (result != INVALID_HANDLE_VALUE) {
		/* Make the handle inheritable. */
		DBG_ALIGNMENT_DISABLE();
		SetHandleInformation(result,
		                     HANDLE_FLAG_INHERIT,
		                     HANDLE_FLAG_INHERIT);
		DBG_ALIGNMENT_ENABLE();
	}
	return result;
}

INTERN WUNUSED DREF DeeObject *DCALL
nt_GetEnvironmentVariable(LPCWSTR lpName) {
	LPWSTR buffer;
	DWORD bufsize = 256, error;
	LPWSTR new_buffer;
	buffer = DeeString_NewWideBuffer(bufsize);
	if unlikely(!buffer)
		goto err;
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		error = GetEnvironmentVariableW(lpName, buffer, bufsize + 1);
		DBG_ALIGNMENT_ENABLE();
		if (!error) {
			/* Error. */
			DeeString_FreeWideBuffer(buffer);
			return ITER_DONE;
		}
		if (error <= bufsize)
			break;
		/* Resize to fit. */
		new_buffer = DeeString_ResizeWideBuffer(buffer, error - 1);
		if unlikely(!new_buffer)
			goto err_result;
		buffer  = new_buffer;
		bufsize = error - 1;
	}
	buffer = DeeString_TruncateWideBuffer(buffer, error);
	return DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(buffer);
err:
	return NULL;
}



PRIVATE WUNUSED DREF DeeStringObject *DCALL
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
		while (*next && *next != ';')
			++next;
		pathlen = (size_t)(next - iter);
		if (next[-1] != '/' && next[-1] != '\\')
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
		memcpy(bufiter, iter, (size_t)(next - iter) * sizeof(WCHAR));
		bufiter += (size_t)(next - iter);
		if (next[-1] != '/' && next[-1] != '\\')
			*bufiter++ = '\\';
		memcpy(bufiter, lpApplicationName, szApplicationNameLength * sizeof(WCHAR));
		bufiter[szApplicationNameLength] = 0; /* Ensure zero-termination */
		ASSERT(bFixUnc ? bufiter + szApplicationNameLength == buffer + WSTR_LENGTH(buffer)
		               : bufiter + szApplicationNameLength <= buffer + WSTR_LENGTH(buffer));
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
		while (*next && *next == ';')
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

PRIVATE WUNUSED DREF DeeStringObject *DCALL
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
		while (*next && *next != ';')
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
		bufiter = buffer;
		/* Create the path for the filename. */
		memcpy(bufiter, lpApplicationName, szApplicationNameLength * sizeof(WCHAR));
		bufiter += szApplicationNameLength;
		memcpy(bufiter, iter, (size_t)(next - iter) * sizeof(WCHAR));
		bufiter[(size_t)(next - iter)] = 0; /* Ensure zero-termination */
		ASSERT(bufiter + (size_t)(next - iter) <= buffer + WSTR_LENGTH(buffer));
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
		while (*next && *next == ';')
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


PRIVATE WCHAR const wPathStr[] = { 'P', 'A', 'T', 'H', 0 };
PRIVATE WCHAR const wPathExtStr[] = { 'P', 'A', 'T', 'H', 'E', 'X', 'T', 0 };


/* Create a new process by searching $PATH for its application name,
 * before trying again while appending extensions for $PATHEXT.
 * If all that still failed, try one more time while fixing UNC path strings.
 * @return: * :        The application name that was eventually used to start the process.
 * @return: NULL:      An error occurred and was thrown.
 * @return: ITER_DONE: Failed to start the process (see GetLastError()) */
PRIVATE WUNUSED DREF DeeStringObject *DCALL
nt_CreateProcessPath(LPWSTR lpApplicationName, SIZE_T szApplicationNameLength,
                     LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
                     LPSECURITY_ATTRIBUTES lpThreadAttributes,
                     BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,
                     LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
                     LPPROCESS_INFORMATION lpProcessInformation) {
	DREF DeeStringObject *result = (DREF DeeStringObject *)ITER_DONE;
	DREF DeeStringObject *path, *pathext;
	LPWSTR pathStr;
	BOOL bFixUnc = FALSE;
	path         = (DREF DeeStringObject *)nt_GetEnvironmentVariable(wPathStr);
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
	pathext = (DREF DeeStringObject *)nt_GetEnvironmentVariable(wPathExtStr);
#if 0 /* ??? */
	if (ITER_ISOK(pathext)) {
		Dee_Decref(pathext);
		pathext = (DREF DeeStringObject *)nt_GetEnvironmentVariable(wPathExtStr);
	}
#endif
	if (pathext != (DREF DeeStringObject *)ITER_DONE) {
		LPWSTR appnameBuffer, iter, next, newAppnameBuffer;
		SIZE_T appnameBufferSize = szApplicationNameLength + 4; /* Optimize for 4-character long extensions. */
		if unlikely(!pathext)
			goto err;
		appnameBuffer = (LPWSTR)Dee_Malloc((appnameBufferSize + 1) * sizeof(WCHAR));
		if unlikely(!appnameBuffer)
			goto err_pathext;
		iter = (LPWSTR)DeeString_AsWide((DeeObject *)pathext);
		if unlikely(!iter) {
			Dee_Free(appnameBuffer);
			goto err_pathext;
		}
		/* Copy the appname itself into the buffer. */
		memcpy(appnameBuffer, lpApplicationName, szApplicationNameLength * sizeof(WCHAR));
		for (;;) {
			SIZE_T appnameLength, extLength;
			next = iter;
			while (*next && *next != ';')
				++next;
			/* Append extensions from $PATHEXT */
			extLength     = (SIZE_T)(next - iter);
			appnameLength = szApplicationNameLength + extLength;
			if unlikely(appnameLength > appnameBufferSize) {
				/* More-than-4-character extension. */
				newAppnameBuffer = (LPWSTR)Dee_Realloc(appnameBuffer, (appnameLength + 1) * sizeof(WCHAR));
				if unlikely(!newAppnameBuffer) {
					Dee_Free(appnameBuffer);
					goto err_pathext;
				}
				appnameBuffer     = newAppnameBuffer;
				appnameBufferSize = appnameLength;
			}
			/* Copy the extension. */
			memcpy(appnameBuffer + szApplicationNameLength,
			       iter, extLength * sizeof(WCHAR));
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
			while (*next && *next == ';')
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




PRIVATE WUNUSED DREF DeeStringObject *DCALL
nt_CreateProcessExt(LPWSTR lpApplicationName, SIZE_T szApplicationNameLength,
                    LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
                    LPSECURITY_ATTRIBUTES lpThreadAttributes,
                    BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,
                    LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
                    LPPROCESS_INFORMATION lpProcessInformation) {
	DREF DeeStringObject *result = (DREF DeeStringObject *)ITER_DONE;
	DREF DeeStringObject *pathext;
	pathext = (DREF DeeStringObject *)nt_GetEnvironmentVariable(wPathExtStr);
	if (pathext != (DREF DeeStringObject *)ITER_DONE) {
		LPWSTR lpPathExt;
		if unlikely(!pathext)
			goto err;
		lpPathExt = (LPWSTR)DeeString_AsWide((DeeObject *)pathext);
		if unlikely(!lpPathExt)
			result = NULL;
		else {
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
		Dee_Decref(pathext);
	}
	return result;
err:
	return NULL;
}






PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_start(Process *self, size_t argc, DeeObject *const *argv) {
	LPWSTR wExe, wExeStart, wCmdLine, wCmdLineCopy, wEnviron, wPwd;
	DeeObject *result = NULL;
	size_t wCmdLineSize;
	DREF DeeStringObject *exe, *cmdline, *pwd;
	DREF DeeObject *procin, *procout, *procerr;
	DREF DeeObject *procenv, *temp;
	DREF DeeStringObject *final_exe;
	STARTUPINFOW startupInfo;
	PROCESS_INFORMATION procInfo;
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
	if (ATOMIC_FETCHOR(self->p_state, PROCESS_FSTARTING) &
	    PROCESS_FSTARTING) {
		rwlock_endread(&self->p_lock);
		SCHED_YIELD();
		goto again;
	}
	exe     = self->p_exe;
	cmdline = self->p_cmdline;
	if unlikely(!exe || !cmdline) {
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
	Dee_Incref(cmdline);
	Dee_XIncref(pwd);
	Dee_XIncref(procin);
	Dee_XIncref(procout);
	Dee_XIncref(procerr);
	Dee_XIncref(procenv);
	rwlock_endread(&self->p_lock);

	/* Load the widestring representations of the exe and cmdline. */
	wExe = (LPWSTR)DeeString_AsWide((DeeObject *)exe);
	if unlikely(!wExe)
		goto done_procenv;
	wCmdLine = (LPWSTR)DeeString_AsWide((DeeObject *)cmdline);
	if unlikely(!wCmdLine)
		goto done_procenv;

	/* Since `CreateProcessW()' is allowed to modify the commandline, we must copy it beforehand. */
	wCmdLineSize = (WSTR_LENGTH(wCmdLine) + 1) * sizeof(WCHAR);
	wCmdLineCopy = (LPWSTR)Dee_Malloc(wCmdLineSize);
	if unlikely(!wCmdLineCopy)
		goto done_procenv;
	memcpy(wCmdLineCopy, wCmdLine, wCmdLineSize);

	/* Pack together environment strings. */
	wEnviron = NULL;
	if (procenv) {
		temp = pack_environ(procenv);
		if unlikely(!temp)
			goto done_cmdline_copy;
		Dee_Decref(procenv);
		procenv = temp;
		/* Load the widestring representation of the environment table. */
		wEnviron = (LPWSTR)DeeString_AsWide(procenv);
		if unlikely(!wEnviron)
			goto done_cmdline_copy;
	}

	wPwd = NULL;
	if (pwd) {
		/* Load the wide-character version of the process working directory. */
		wPwd = (LPWSTR)DeeString_AsWide((DeeObject *)pwd);
		if unlikely(!wEnviron)
			goto done_cmdline_copy;
	}

	memset(&startupInfo, 0, sizeof(STARTUPINFOW));
	startupInfo.cb      = sizeof(STARTUPINFOA);
	startupInfo.dwFlags = STARTF_USESTDHANDLES;

	/* Load standard handles for the new process. */
	startupInfo.hStdInput = get_stdhandle_for_process(procin, DEE_STDIN);
	if unlikely(startupInfo.hStdInput == INVALID_HANDLE_VALUE)
		goto done_cmdline_copy;
	startupInfo.hStdOutput = get_stdhandle_for_process(procout, DEE_STDOUT);
	if unlikely(startupInfo.hStdOutput == INVALID_HANDLE_VALUE)
		goto done_cmdline_copy;
	startupInfo.hStdError = get_stdhandle_for_process(procerr, DEE_STDERR);
	if unlikely(startupInfo.hStdError == INVALID_HANDLE_VALUE)
		goto done_cmdline_copy;

	/* If `wExe' isn't an absolute path and isn't relative to the current directory,
	 * search $PATH for it, before trying again while appending extensions for $PATHEXT.
	 * If all that still failed, try one more time while fixing UNC path strings. */
	wExeStart = wExe;
	while (DeeUni_IsSpace(wExe[0]))
		++wExe;
	if ((wExe[0] != '.') &&                    /* PWD relative */
	    (wExe[0] != '\\' && wExe[0] != '/') && /* DRIVE relative */
	    (wExe[1] != ':')) {                    /* ABSOLUTE path. */
		LPWSTR wExeEnd;
		wExeEnd = wExeStart + WSTR_LENGTH(wExeStart);
		while (wExeEnd != wExe && DeeUni_IsSpace(wExeEnd[-1]))
			--wExeEnd;
		/* TODO: Keep a cache dependent on $PATH and $PATHEXT for mapping
		 *       application names to their absolute file names.
		 *       The cache must be invalidated when either variable is
		 *       modified, meaning that for situations like this, we must
		 *       add a sub-system for broadcasting environment-changes
		 *       throughout deemon:
		 *    >> typedef int (DCALL *env_changed_t)(void *arg);
		 *    >> int DeeEnv_AddListen(DeeObject *__restrict name, env_changed_t callback, void *arg);
		 *    >> int DeeEnv_DelListen(DeeObject *__restrict name, env_changed_t callback, void *arg);
		 *    >> int DeeEnv_NotifyChanged(DeeObject *__restrict name);
		 */
		/* Start the process by searching $PATH */
		final_exe = nt_CreateProcessPath(wExe, (SIZE_T)(wExeEnd - wExe),
		                                 wCmdLineCopy, NULL, NULL, TRUE,
		                                 CREATE_UNICODE_ENVIRONMENT, wEnviron,
		                                 wPwd, &startupInfo, &procInfo);
		if unlikely(!final_exe)
			goto done_cmdline_copy;
		if (final_exe != (DREF DeeStringObject *)ITER_DONE) {
			/* Saved the new (fixed) executable name. */
save_final_exe:
			DBG_ALIGNMENT_ENABLE();
			rwlock_write(&self->p_lock);
			ASSERT(self->p_exe == exe); /* Inherit reference (from `self->p_exe' into `exe') */
			Dee_Incref(final_exe);
			self->p_exe = final_exe; /* Inherit reference */
			rwlock_endwrite(&self->p_lock);
			Dee_DecrefNokill(exe); /* The reference inherited from `self->p_exe' */
			Dee_Decref(exe);       /* The reference created above. */
			exe = final_exe;       /* Inherit reference */
			goto create_ok;
		}
		/* Following windows conventions, also search PWD for raw executable names,
		 * which is done by falling through to the absolute-path handler below. */
	}

	/* TODO: Keep a cache dependent only on $PATHEXT for mapping
	 *       application names to their absolute file names. */

	/* Create a new process, given its absolute path. */
	DBG_ALIGNMENT_DISABLE();
	if (!CreateProcessW(wExe, wCmdLineCopy, NULL, NULL, TRUE,
	                    CREATE_UNICODE_ENVIRONMENT, wEnviron,
	                    wPwd, &startupInfo, &procInfo)) {
		DWORD error                     = GetLastError();
		DREF DeeStringObject *fixed_exe = NULL;
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsUncError(error)) {
			final_exe = (DREF DeeStringObject *)DeeNTSystem_FixUncPath((DeeObject *)exe);
			if unlikely(!final_exe)
				goto done_cmdline_copy;
			wExe = (LPWSTR)DeeString_AsWide((DeeObject *)final_exe);
			if unlikely(!wExe) {
				Dee_Decref(final_exe);
				goto done_cmdline_copy;
			}
			/* Try again after fixing UNC. */
			DBG_ALIGNMENT_DISABLE();
			if (CreateProcessW(wExe, wCmdLineCopy, NULL, NULL, TRUE,
			                   CREATE_UNICODE_ENVIRONMENT, wEnviron,
			                   wPwd, &startupInfo, &procInfo))
				goto save_final_exe;
			error = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			fixed_exe = final_exe;
		}
		if (DeeNTSystem_IsFileNotFoundError(error) ||
		    DeeNTSystem_IsAccessDeniedError(error)) {
			/* Append %PATHEXT% to the executable name, and try again. */
			final_exe = nt_CreateProcessExt(wExe, WSTR_LENGTH(wExe),
			                                wCmdLineCopy, NULL, NULL, TRUE,
			                                CREATE_UNICODE_ENVIRONMENT, wEnviron,
			                                wPwd, &startupInfo, &procInfo);
			Dee_XDecref(fixed_exe);
			if (final_exe != (DREF DeeStringObject *)ITER_DONE) {
				if (!final_exe)
					goto done_cmdline_copy;
				goto save_final_exe;
			}
			DBG_ALIGNMENT_DISABLE();
			error = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			DeeNTSystem_ThrowErrorf(&DeeError_FileNotFound, error,
			                        "Application %r could not be found",
			                        exe);
		} else {
			Dee_XDecref(fixed_exe);
			DeeNTSystem_ThrowErrorf(NULL, error, "Failed to create process");
		}
		goto done_cmdline_copy;
	}
	DBG_ALIGNMENT_ENABLE();
create_ok:

	rwlock_write(&self->p_lock);

	/* Set the started and child flags now that the processed is running. */
	self->p_state |= (PROCESS_FSTARTED | PROCESS_FCHILD);

	/* Copy process id and handle. */
	self->p_id     = procInfo.dwProcessId;
	self->p_handle = procInfo.hProcess;

	rwlock_endwrite(&self->p_lock);

	/* Close our handle to the process's main thread. */
	DBG_ALIGNMENT_DISABLE();
	CloseHandle(procInfo.hThread);
	DBG_ALIGNMENT_ENABLE();

	/* Return true on success. */
	result = Dee_True;
	Dee_Incref(Dee_True);

done_cmdline_copy:
	Dee_Free(wCmdLineCopy);
done_procenv:
	rwlock_read(&self->p_lock);
	ATOMIC_FETCHAND(self->p_state, ~PROCESS_FSTARTING);
	rwlock_endread(&self->p_lock);
	Dee_XDecref(procenv);
	Dee_XDecref(procerr);
	Dee_XDecref(procout);
	Dee_XDecref(procin);
	Dee_XDecref(pwd);
	Dee_Decref(cmdline);
	Dee_Decref(exe);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_ensure_handle(Process *__restrict self, DWORD dwDesiredAccess) {
	if (self->p_handle == NULL) {
		HANDLE hProcess;
		DBG_ALIGNMENT_DISABLE();
		if (self == &this_process) {
			hProcess = GetCurrentProcess();
		} else {
			hProcess = OpenProcess(dwDesiredAccess, TRUE, self->p_id);
		}
		DBG_ALIGNMENT_ENABLE();
		if (!hProcess)
			goto err;
		rwlock_write(&self->p_lock);
		if (self->p_handle == NULL) {
			self->p_handle = hProcess;
			rwlock_endwrite(&self->p_lock);
		} else {
			rwlock_endwrite(&self->p_lock);
			if (self != &this_process) {
				DBG_ALIGNMENT_DISABLE();
				CloseHandle(hProcess);
				DBG_ALIGNMENT_ENABLE();
			}
		}
	}
	return 0;
err:
	{
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		return DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
	                                   "Failed to access process %k", self);
	}
}


/* @return: 1: Timeout */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
process_dojoin(Process *__restrict self,
               DWORD *__restrict proc_result,
               uint64_t timeout_microseconds) {
	ASSERT(proc_result);
again:
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
		while (ATOMIC_FETCHOR(self->p_state, PROCESS_FDETACHING) &
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
			ATOMIC_FETCHAND(self->p_state, ~PROCESS_FDETACHING);
			DeeError_Throwf(&DeeError_ValueError,
			                "Cannot join process %k after being detached",
			                self);
			goto err;
		}
		if (process_ensure_handle(self, SYNCHRONIZE))
			goto err;
		{
			/* Wait for the process to complete. */
			DWORD wait_state;
			DBG_ALIGNMENT_DISABLE();
			wait_state = WaitForMultipleObjectsEx(1, (HANDLE *)&self->p_handle, FALSE,
			                                      timeout_microseconds == (uint64_t)-1
			                                      ? INFINITE
			                                      : (DWORD)(timeout_microseconds / 1000),
			                                      TRUE);
			DBG_ALIGNMENT_ENABLE();
			switch (wait_state) {

			case WAIT_IO_COMPLETION:
				/* Interrupt. */
				ATOMIC_FETCHAND(self->p_state, ~PROCESS_FDETACHING);
				goto again;

			case WAIT_TIMEOUT:
				ATOMIC_FETCHAND(self->p_state, ~PROCESS_FDETACHING);
				return 1; /* Timeout */

			case WAIT_FAILED:
				/* Error */
				ATOMIC_FETCHAND(self->p_state, ~PROCESS_FDETACHING);
				{
					DWORD dwError;
					DBG_ALIGNMENT_DISABLE();
					dwError = GetLastError();
					DBG_ALIGNMENT_ENABLE();
					DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
					                        "Failed to join process %k", self);
				}
				goto err;

#if 0
			case WAIT_ABANDONED_0:
			case WAIT_OBJECT_0:
				break;
#endif

			default: break;
			}
		}

done_join:
		/* Delete the detaching-flag and set the detached-flag. */
		do {
			state = ATOMIC_READ(self->p_state);
		} while (!ATOMIC_CMPXCH_WEAK(self->p_state, state,
		                             (state & ~PROCESS_FDETACHING) | new_flags));
	}
	if (process_ensure_handle(self, PROCESS_QUERY_LIMITED_INFORMATION))
		goto err;
	/* Retrieve the process's exit code. */
	if (!nt_GetExitCodeProcess(self->p_id, self->p_handle, proc_result))
		*proc_result = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_detach(Process *self, size_t argc, DeeObject *const *argv) {
	uint16_t state;
	if (DeeArg_Unpack(argc, argv, ":" S_Process_function_detach_name))
		goto err;
	while (ATOMIC_FETCHOR(self->p_state, PROCESS_FDETACHING) &
	       PROCESS_FDETACHING)
		SCHED_YIELD();
	if (!(self->p_state & (PROCESS_FSTARTED | PROCESS_FTERMINATED))) {
		/* Process was never started. */
		ATOMIC_FETCHAND(self->p_state, ~THREAD_STATE_DETACHING);
		DeeError_Throwf(&DeeError_ValueError,
		                "Cannot detach process %k that hasn't been started",
		                self);
		goto err;
	}
	if (!(self->p_state & PROCESS_FCHILD)) {
		/* Process isn't one of our children. */
		ATOMIC_FETCHAND(self->p_state, ~THREAD_STATE_DETACHING);
		DeeError_Throwf(&DeeError_ValueError,
		                "Cannot detach process %k that isn't a child",
		                self);
		goto err;
	}
	if (self->p_state & THREAD_STATE_DETACHED) {
		/* Process was already detached. */
		ATOMIC_FETCHAND(self->p_state, ~PROCESS_FDETACHING);
		return_false;
	}

	/* Set the detached-flag and unset the detaching-flag. */
	do {
		state = ATOMIC_READ(self->p_state);
	} while (!ATOMIC_CMPXCH_WEAK(self->p_state, state,
	                             (state & ~PROCESS_FDETACHING) | PROCESS_FDETACHED));
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_id(Process *__restrict self) {
	DWORD pid;
	rwlock_read(&self->p_lock);
	if (!(self->p_state & PROCESS_FSTARTED)) {
		rwlock_endread(&self->p_lock);
		DeeError_Throwf(&DeeError_ValueError,
		                "Process %k has no id because it wasn't started",
		                self);
		goto err;
	}
	pid = self->p_id;
	if (pid == (DWORD)-1 && self == &this_process) {
		/* Lazily load the PID of the current process. */
		DBG_ALIGNMENT_DISABLE();
		pid = GetCurrentProcessId();
		DBG_ALIGNMENT_ENABLE();
		rwlock_upgrade(&self->p_lock);
		COMPILER_READ_BARRIER();
		if (self->p_id == (DWORD)-1)
			self->p_id = pid;
		else {
			pid = self->p_id;
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
	UINT exit_code = 0;
	if (DeeArg_Unpack(argc, argv, "|I32u:" S_Process_function_terminate_name, &exit_code))
		goto err;
	if (DeeThread_CheckInterrupt())
		goto err;
	rwlock_write(&self->p_lock);
	if (!(self->p_state & PROCESS_FSTARTED)) {
		rwlock_endwrite(&self->p_lock);
		/* Already terminated. */
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
	if (process_ensure_handle(self, PROCESS_TERMINATE))
		goto err;
	DBG_ALIGNMENT_DISABLE();
	if (!TerminateProcess(self->p_handle, exit_code)) {
		DWORD error, pid;
		pid = self->p_id;
		rwlock_endwrite(&self->p_lock);
		error = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (error == ERROR_ACCESS_DENIED) {
			HANDLE hProcess;
			/* Try to acquire the terminate-permission. */
			DBG_ALIGNMENT_DISABLE();
			hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
			if (!hProcess)
				error = GetLastError();
			else {
				if (TerminateProcess(hProcess, exit_code)) {
					CloseHandle(hProcess);
					goto term_ok;
				}
				error = GetLastError();
				/* Check for case: The process had already terminated on its own. */
				if (error == ERROR_ACCESS_DENIED &&
				    nt_GetExitCodeProcess(self->p_id, hProcess, (LPDWORD)&exit_code) &&
				    exit_code != STILL_ACTIVE) {
					CloseHandle(hProcess);
					goto term_ok;
				}
				CloseHandle(hProcess);
			}
			DBG_ALIGNMENT_ENABLE();
		}
		DeeNTSystem_ThrowErrorf(NULL, error,
		                        "Failed to terminate process");
		goto err;
	}
term_ok:
	DBG_ALIGNMENT_ENABLE();
	/* Set the did-terminate flag on success. */
	self->p_state |= PROCESS_FTERMINATED;
	rwlock_endwrite(&self->p_lock);
	return_true;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_join(Process *self, size_t argc, DeeObject *const *argv) {
	int error;
	DWORD result;
	if (DeeArg_Unpack(argc, argv, ":" S_Process_function_join_name))
		goto err;
	error = process_dojoin(self, &result, (uint64_t)-1);
	if unlikely(error < 0)
		goto err;
	return DeeInt_NewU32((uint32_t)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_tryjoin(Process *self, size_t argc, DeeObject *const *argv) {
	int error;
	DWORD result;
	if (DeeArg_Unpack(argc, argv, ":" S_Process_function_tryjoin_name))
		goto err;
	error = process_dojoin(self, &result, 0);
	if unlikely(error < 0)
		goto err;
	if (error == 0)
		return DeeInt_NewU32(result);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_timedjoin(Process *self, size_t argc, DeeObject *const *argv) {
	int error;
	DWORD result;
	uint64_t timeout;
	if (DeeArg_Unpack(argc, argv, "I64d:" S_Process_function_timedjoin_name, &timeout))
		goto err;
	error = process_dojoin(self, &result, timeout);
	if unlikely(error < 0)
		goto err;
	if (error == 0)
		return DeeInt_NewU32(result);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_hasterminated(Process *__restrict self) {
	DWORD exitcode;
	if (self->p_state & PROCESS_FTERMINATED)
		return_true;
	rwlock_read(&self->p_lock);
	if (!(self->p_state & PROCESS_FSTARTED)) {
		rwlock_endread(&self->p_lock);
		return_false;
	}
	if (process_ensure_handle(self, PROCESS_QUERY_LIMITED_INFORMATION))
		goto err;
	if (nt_GetExitCodeProcess(self->p_id, self->p_handle, &exitcode) &&
	    exitcode != STILL_ACTIVE) {
		rwlock_upgrade(&self->p_lock);
		self->p_state |= PROCESS_FTERMINATED;
		rwlock_endwrite(&self->p_lock);
		return_true;
	}
	rwlock_endread(&self->p_lock);
	return_false;
err:
	return NULL;
}


PRIVATE struct type_method process_methods[] = {
	{ S_Process_function_start_name,
	  (DREF DeeObject * (DCALL *)(DeeObject *, size_t, DeeObject *const *))&process_start,
	  DOC(S_Process_function_start_doc) },
	{ S_Process_function_join_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&process_join,
	  DOC(S_Process_function_join_doc) },
	{ S_Process_function_tryjoin_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&process_tryjoin,
	  DOC(S_Process_function_tryjoin_doc) },
	{ S_Process_function_timedjoin_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&process_timedjoin,
	  DOC(S_Process_function_timedjoin_doc) },
	{ S_Process_function_detach_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&process_detach,
	  DOC(S_Process_function_detach_doc) },
	{ S_Process_function_terminate_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&process_terminate,
	  DOC(S_Process_function_terminate_doc) },
	{ NULL }
};

#ifndef CONFIG_NO_THREADS
typedef struct {
	OBJECT_HEAD
	DWORD pt_id; /* [const] The ID of the process to look out for, or 0 if all are good. */
} ProcessThreads;

typedef struct {
	OBJECT_HEAD
	HANDLE pti_handle;       /* [1..1][owned][const] The handle used to enumerate threads. */
	THREADENTRY32 pti_entry; /* [lock(pti_lock)] The next entry (`dwSize' is ZERO when iteration is complete) */
	rwlock_t pti_lock;       /* Lock for `pti_entry' */
	DWORD pti_id;            /* [const] The ID of the process to look out for, or 0 if all are good. */
} ProcessThreadsIterator;

INTDEF DeeTypeObject ProcessThreadsIterator_Type;
INTDEF DeeTypeObject ProcessThreads_Type;

PRIVATE WUNUSED DREF ProcessThreads *DCALL pt_new(DWORD pid) {
	DREF ProcessThreads *result;
	result = DeeObject_MALLOC(ProcessThreads);
	if unlikely(!result)
		goto done;
	result->pt_id = pid;
	DeeObject_Init(result, &ProcessThreads_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF ProcessThreadsIterator *DCALL
pt_iter(ProcessThreads *__restrict self) {
	DREF ProcessThreadsIterator *result;
	result = DeeObject_MALLOC(ProcessThreadsIterator);
	if unlikely(!result)
		goto done;
	DBG_ALIGNMENT_DISABLE();
	result->pti_handle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, self->pt_id);
	DBG_ALIGNMENT_ENABLE();
	if (!result->pti_handle ||
	    result->pti_handle == INVALID_HANDLE_VALUE) {
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		DeeNTSystem_ThrowErrorf(NULL, dwError,
		                        "Failed to create a snapshot of the thread of process %I32u",
		                        self->pt_id);
		goto err_r;
	}
	result->pti_entry.dwSize = sizeof(result->pti_entry);
	DBG_ALIGNMENT_DISABLE();
	if (!Thread32First(result->pti_handle, &result->pti_entry))
		result->pti_entry.dwSize = 0;
	else {
		while (!result->pti_entry.dwSize) {
			if (!Thread32Next(result->pti_handle, &result->pti_entry)) {
				result->pti_entry.dwSize = 0;
				break;
			}
		}
	}
	DBG_ALIGNMENT_ENABLE();
	result->pti_id = self->pt_id;
	rwlock_init(&result->pti_lock);
	DeeObject_Init(result, &ProcessThreadsIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}


PRIVATE struct type_seq pt_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&pt_iter
};


PRIVATE struct type_member pt_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &ProcessThreadsIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject ProcessThreads_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_ProcessThreads_tp_name,
	/* .tp_doc      = */ DOC(S_ProcessThreads_tp_doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(ProcessThreads)
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
	/* .tp_seq           = */ &pt_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ pt_class_members
};

PRIVATE NONNULL((1)) void DCALL
pti_fini(ProcessThreadsIterator *__restrict self) {
	DBG_ALIGNMENT_DISABLE();
	CloseHandle(self->pti_handle);
	DBG_ALIGNMENT_ENABLE();
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
pti_next(ProcessThreadsIterator *__restrict self) {
	DWORD /*pid,*/ tid;
	HANDLE thread_handle;
	DREF DeeObject *result;
again:
	rwlock_write(&self->pti_lock);
	if (!self->pti_entry.dwSize) {
done:
		DBG_ALIGNMENT_ENABLE();
		rwlock_endwrite(&self->pti_lock);
		return ITER_DONE;
	}
	for (;;) {
		if (self->pti_entry.dwSize >= COMPILER_OFFSETAFTER(THREADENTRY32, th32OwnerProcessID) &&
		    (self->pti_id == 0 || self->pti_entry.th32OwnerProcessID == self->pti_id))
			break;
		do {
			self->pti_entry.dwSize = sizeof(self->pti_entry);
			DBG_ALIGNMENT_DISABLE();
			if (!Thread32Next(self->pti_handle, &self->pti_entry))
				goto done;
			DBG_ALIGNMENT_ENABLE();
		} while (!self->pti_entry.dwSize);
	}
	/*pid = self->pti_entry.th32OwnerProcessID;*/
	tid = self->pti_entry.th32ThreadID;
	do {
		self->pti_entry.dwSize = sizeof(self->pti_entry);
		DBG_ALIGNMENT_DISABLE();
		if (!Thread32Next(self->pti_handle, &self->pti_entry)) {
			DBG_ALIGNMENT_ENABLE();
			self->pti_entry.dwSize = 0;
			break;
		}
		DBG_ALIGNMENT_ENABLE();
	} while (!self->pti_entry.dwSize);
	rwlock_endwrite(&self->pti_lock);
	DBG_ALIGNMENT_DISABLE();
	thread_handle = OpenThread(SYNCHRONIZE, FALSE, tid);
	DBG_ALIGNMENT_ENABLE();
	if (thread_handle == NULL)
		goto again;
#ifndef CONFIG_NO_THREADID
	result = DeeThread_NewExternal(thread_handle, tid);
#else /* !CONFIG_NO_THREADID */
	result = DeeThread_NewExternal(thread_handle);
#endif /* CONFIG_NO_THREADID */
	if unlikely(!result)
		CloseHandle(thread_handle);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF ProcessThreads *DCALL
pti_getseq(ProcessThreadsIterator *__restrict self) {
	return pt_new(self->pti_id);
}

PRIVATE struct type_getset pti_getsets[] = {
	{ "seq", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&pti_getseq },
	{ NULL }
};

INTERN DeeTypeObject ProcessThreadsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_ProcessThreadsIterator_tp_name,
	/* .tp_doc      = */ DOC(S_ProcessThreadsIterator_tp_doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(ProcessThreadsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&pti_fini,
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
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&pti_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ pti_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_threads(Process *__restrict self) {
	if (!(self->p_state & PROCESS_FSTARTED) ||
	    (self->p_state & PROCESS_FTERMINATED))
		return_reference_(Dee_EmptySeq);
	if (self == &this_process)
		return DeeObject_GetAttrString((DeeObject *)&DeeThread_Type, "enumerate");
	return (DREF DeeObject *)pt_new(self->p_id);
}
#else /* !CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_threads(Process *__restrict self) {
	if (self == &this_process)
		return DeeObject_GetAttrString((DeeObject *)&DeeThread_Type, "enumerate");
	return_reference_(Dee_EmptySeq);
}

#endif /* CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_files(Process *__restrict self) {
	(void)self;
	DERROR_NOTIMPLEMENTED(); /* TODO */
	return NULL;
}



#ifndef ERR_UNBOUND_ATTRIBUTE_DEFINED
#define ERR_UNBOUND_ATTRIBUTE_DEFINED 1
INTERN ATTR_COLD NONNULL((1, 2)) int DCALL
err_unbound_attribute(DeeTypeObject *__restrict tp,
                      char const *__restrict name) {
	ASSERT_OBJECT(tp);
	ASSERT(name);
	ASSERT(DeeType_Check(tp));
	return DeeError_Throwf(&DeeError_UnboundAttribute,
	                       "Unbound attribute `%k.%s'",
	                       tp, name);
}
#endif /* !ERR_UNBOUND_ATTRIBUTE_DEFINED */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_attribute_and_unlock(Process *__restrict self,
                                 DWORD dwAttributeId) {
	DREF DeeObject *result;
	HANDLE orig_handle, final_handle;
	ASSERT(rwlock_reading(&self->p_lock));
	orig_handle = self->p_handle;
	rwlock_endread(&self->p_lock);
	final_handle = orig_handle;
	result       = nt_GetProcessAttribute(&final_handle, self->p_id, dwAttributeId);
	/* Close the new handle if it changed. */
	if (final_handle != orig_handle) {
		DBG_ALIGNMENT_DISABLE();
		CloseHandle(final_handle);
		DBG_ALIGNMENT_ENABLE();
	}
	return result;
}

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

STATIC_ASSERT(PROCATTR_STANDARDINPUT == DEE_STDIN);
STATIC_ASSERT(PROCATTR_STANDARDOUTPUT == DEE_STDOUT);
STATIC_ASSERT(PROCATTR_STANDARDERROR == DEE_STDERR);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_std(Process *__restrict self, int stdno) {
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
	return process_get_attribute_and_unlock(self, stdno);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_std(Process *self, int stdno, DeeObject *value) {
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

#define DEFINE_PROCESS_STD_FUNCTIONS(stdxxx, DEE_STDXXX)    \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL      \
	process_get_##stdxxx(Process *__restrict self) {        \
		return process_get_std(self, DEE_STDXXX);           \
	}                                                       \
	PRIVATE WUNUSED NONNULL((1)) int DCALL                  \
	process_del_##stdxxx(Process *__restrict self) {        \
		return process_set_std(self, DEE_STDXXX, NULL);     \
	}                                                       \
	PRIVATE WUNUSED NONNULL((1, 2)) int DCALL               \
	process_set_##stdxxx(Process *self, DeeObject *value) { \
		return process_set_std(self, DEE_STDXXX, value);    \
	}
DEFINE_PROCESS_STD_FUNCTIONS(stdin, DEE_STDIN)
DEFINE_PROCESS_STD_FUNCTIONS(stdout, DEE_STDOUT)
DEFINE_PROCESS_STD_FUNCTIONS(stderr, DEE_STDERR)
#undef DEFINE_PROCESS_STD_FUNCTIONS

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
call_extern(DeeObject *module_name,
            DeeObject *global_name,
            size_t argc, DeeObject *const *argv) {
	DREF DeeObject *module, *result;
	module = DeeModule_OpenGlobal(module_name, NULL, true);
	if unlikely(!module)
		goto err;
	result = DeeObject_CallAttr(module, global_name, argc, argv);
	Dee_Decref(module);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
get_extern(DeeObject *module_name, DeeObject *global_name) {
	DREF DeeObject *module, *result;
	module = DeeModule_OpenGlobal(module_name, NULL, true);
	if unlikely(!module)
		goto err;
	result = DeeObject_GetAttr(module, global_name);
	Dee_Decref(module);
	return result;
err:
	return NULL;
}

PRIVATE DEFINE_STRING(str_fs, "fs");
PRIVATE DEFINE_STRING(str_getcwd, "getcwd");
PRIVATE DEFINE_STRING(str_chdir, "chdir");
PRIVATE DEFINE_STRING(str_environ, "environ");

#ifndef IPC_UNIMPLEMENTED_DEFINED
#define IPC_UNIMPLEMENTED_DEFINED 1
PRIVATE ATTR_COLD int DCALL ipc_unimplemented(void) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "IPI functionality is not supported");
}
#endif /* !IPC_UNIMPLEMENTED_DEFINED */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_memory(Process *__restrict self) {
	(void)self;
	ipc_unimplemented();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_environ(Process *__restrict self) {
	DREF DeeObject *result;
	if (self == &this_process)
		return get_extern((DeeObject *)&str_fs, (DeeObject *)&str_environ);
	rwlock_read(&self->p_lock);
	if (!(self->p_state & PROCESS_FSTARTED)) {
		result = self->p_environ;
		Dee_XIncref(result);
		rwlock_endread(&self->p_lock);
		if (!result)
			err_unbound_attribute(&DeeProcess_Type, DeeString_STR(&str_environ));
		return result;
	}
	return process_get_attribute_and_unlock(self, PROCATTR_ENVIRONMENT);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_environ(Process *self, DeeObject *value) {
	DREF DeeObject *temp;
	if (self == &this_process) {
		int result;
		temp = get_extern((DeeObject *)&str_fs, (DeeObject *)&str_environ);
		if unlikely(!temp)
			return -1;
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
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Cannot %s environ of running process %k",
	                       value ? "change"
	                             : "delete",
	                       self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_environ(Process *__restrict self) {
	return process_set_environ(self, NULL);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_pwd(Process *__restrict self) {
	DREF DeeStringObject *result;
	if (self == &this_process)
		return call_extern((DeeObject *)&str_fs, (DeeObject *)&str_getcwd, 0, NULL);
	rwlock_read(&self->p_lock);
	if (!(self->p_state & PROCESS_FSTARTED)) {
		result = self->p_pwd;
		Dee_XIncref(result);
		rwlock_endread(&self->p_lock);
		if (!result)
			err_unbound_attribute(&DeeProcess_Type, S_Process_getset_pwd_name);
		return (DREF DeeObject *)result;
	}
	return process_get_attribute_and_unlock(self, PROCATTR_CURRENTDIRECTORY);
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
		temp = call_extern((DeeObject *)&str_fs,
		                   (DeeObject *)&str_chdir,
		                   0, NULL);
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_exe(Process *__restrict self) {
	DREF DeeStringObject *result;
	HANDLE orig_handle;
	if (self == &this_process)
		return nt_GetModuleFileName(NULL);
	rwlock_read(&self->p_lock);
	result = self->p_exe;
	if (result) {
		Dee_Incref(result);
		rwlock_endread(&self->p_lock);
		goto done;
	}
	if (!(self->p_state & PROCESS_FSTARTED)) {
		rwlock_endread(&self->p_lock);
		err_unbound_attribute(&DeeProcess_Type, S_Process_getset_exe_name);
		goto done;
	}
	orig_handle = self->p_handle;
	rwlock_endread(&self->p_lock);
	result = (DREF DeeStringObject *)nt_QueryFullProcessImageName(orig_handle, 0);
	if (result != (DREF DeeStringObject *)ITER_DONE) {
		if unlikely(!result)
			goto done;
		goto done_setbuf;
	}
	rwlock_read(&self->p_lock);
	result = (DREF DeeStringObject *)process_get_attribute_and_unlock(self, PROCATTR_IMAGEPATHNAME);
	if (!result)
		goto done;
done_setbuf:
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
done:
	return (DeeObject *)result;
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_cmdline(Process *__restrict self) {
	DREF DeeStringObject *result;
	if (self == &this_process) {
		LPWSTR cmdline = GetCommandLineW();
		return DeeString_NewWide(cmdline,
		                         wcslen(cmdline),
		                         STRING_ERROR_FREPLAC);
	}
	rwlock_read(&self->p_lock);
	result = self->p_cmdline;
	if (result) {
		Dee_Incref(result);
		rwlock_endread(&self->p_lock);
		goto done;
	}
	if (!(self->p_state & PROCESS_FSTARTED)) {
		rwlock_endread(&self->p_lock);
		err_unbound_attribute(&DeeProcess_Type, S_Process_getset_cmdline_name);
		goto done;
	}
	result = (DREF DeeStringObject *)process_get_attribute_and_unlock(self, PROCATTR_COMMANDLINE);
	if (!result)
		goto done;
	rwlock_write(&self->p_lock);
	if (!self->p_cmdline) {
		Dee_Incref(result);
		self->p_cmdline = result;
		rwlock_endwrite(&self->p_lock);
	} else {
		DREF DeeStringObject *new_result;
		new_result = self->p_cmdline;
		Dee_Incref(new_result);
		rwlock_endwrite(&self->p_lock);
		Dee_Decref(result);
		result = new_result;
	}
done:
	return (DeeObject *)result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_cmdline(Process *self, DeeObject *value) {
	DREF DeeStringObject *old_value;
	if (value && DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto err;
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
	Dee_XIncref(value);
	old_value       = self->p_cmdline;
	self->p_cmdline = (DREF DeeStringObject *)value;
	rwlock_endwrite(&self->p_lock);
	if (old_value) {
		Dee_Decref(old_value);
	}
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	else if (!value) {
		err_unbound_attribute(&DeeProcess_Type, S_Process_getset_cmdline_name);
		goto err;
	}
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_cmdline(Process *__restrict self) {
	return process_set_cmdline(self, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_argv(Process *__restrict self) {
	DREF DeeObject *result;
	DREF DeeStringObject *cmdline;
	cmdline = (DREF DeeStringObject *)process_get_cmdline(self);
	if unlikely(!cmdline)
		goto err;
	/* Split the commandline. */
	result = cmdline_split(cmdline);
	Dee_Decref(cmdline);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
process_set_argv(Process *self, DeeObject *value) {
	DREF DeeStringObject *cmdline;
	int result;
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	/* Pack together a commandline. */
	if unlikely(cmdline_add_args(&printer, value))
		goto err;
	cmdline = (DREF DeeStringObject *)ascii_printer_pack(&printer);
	if unlikely(!cmdline)
		goto err_noprinter;
	/* Set the commandline of the process. */
	result = process_set_cmdline(self, (DeeObject *)cmdline);
	Dee_Decref(cmdline);
	return result;
err:
	ascii_printer_fini(&printer);
err_noprinter:
	return -1;
}


PRIVATE struct type_getset process_getsets[] = {
	{ S_Process_getset_hasterminated_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *))&process_hasterminated, NULL, NULL,
	  DOC(S_Process_getset_hasterminated_doc) },
	{ S_Process_getset_id_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *))&process_id, NULL, NULL,
	  DOC(S_Process_getset_id_doc) },
	{ S_Process_getset_threads_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_threads, NULL, NULL,
	  DOC(S_Process_getset_threads_doc) },
	{ S_Process_getset_files_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_files, NULL, NULL,
	  DOC(S_Process_getset_files_doc) },
	{ S_Process_getset_stdin_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_stdin,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_stdin,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_stdin,
	  DOC(S_Process_getset_stdin_doc) },
	{ S_Process_getset_stdout_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_stdout,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_stdout,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_stdout,
	  DOC(S_Process_getset_stdout_doc) },
	{ S_Process_getset_stderr_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_stderr,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_stderr,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_stderr,
	  DOC(S_Process_getset_stderr_doc) },
	{ S_Process_getset_pwd_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_pwd,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_pwd,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_pwd,
	  DOC(S_Process_getset_pwd_doc) },
	{ S_Process_getset_exe_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_exe,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_exe,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_exe,
	  DOC(S_Process_getset_exe_doc) },
	{ S_Process_getset_cmdline_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_cmdline,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_cmdline,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_cmdline,
	  DOC(S_Process_getset_cmdline_doc) },
	{ S_Process_getset_argv_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_argv,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_cmdline, /* Same thing! */
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_argv,
	  DOC(S_Process_getset_argv_doc) },
	{ S_Process_getset_cmd_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_cmdline,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_cmdline,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_cmdline,
	  DOC(S_Process_getset_cmd_doc) },
	{ S_Process_getset_environ_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_environ,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_environ,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_environ,
	  DOC(S_Process_getset_environ_doc) },
	{ S_Process_getset_memory_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_memory, NULL, NULL,
	  DOC(S_Process_getset_memory_doc) },
	{ NULL }
};

PRIVATE struct type_member process_members[] = {
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
process_class_self(DeeObject *UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":" S_Process_class_function_self_name))
		return NULL;
	return_reference_((DeeObject *)&this_process);
}

PRIVATE struct type_method process_class_methods[] = {
	{ S_Process_class_function_self_name, &process_class_self,
	  DOC(S_Process_class_function_self_doc) },
	{ NULL }
};

PRIVATE struct type_member process_class_members[] = {
	TYPE_MEMBER_CONST_DOC(S_Process_member_current_name,
	                      &this_process,
	                      S_Process_member_current_doc),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
process_init(Process *__restrict self,
             size_t argc, DeeObject *const *argv) {
	int temp;
	struct ascii_printer cmdline = ASCII_PRINTER_INIT;
	DREF DeeStringObject *exe;
	DeeObject *args = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:" S_Process_tp_name, &exe, &args))
		goto err;
	/* Add the initial exe-argument. */
	if (DeeString_Check(exe)) {
		if unlikely(cmdline_add_arg(&cmdline, exe))
			goto err;
		Dee_Incref(exe);
	} else {
		if (DeeInt_Check(exe)) {
			HANDLE fd;
			if (DeeObject_AsUIntptr((DeeObject *)exe, (uintptr_t *)&fd))
				goto err;
			exe = (DREF DeeStringObject *)DeeNTSystem_GetFilenameOfHandle(fd);
		} else {
			exe = (DREF DeeStringObject *)DeeFile_Filename((DeeObject *)exe);
		}
		if unlikely(!exe)
			goto err;
		temp = cmdline_add_arg(&cmdline, exe);
		if unlikely(temp)
			goto err_exe;
	}
	/* Add additional arguments. */
	if (args && cmdline_add_args(&cmdline, args))
		goto err_exe;

	/* Pack together the commandline. */
	self->p_cmdline = (DREF DeeStringObject *)ascii_printer_pack(&cmdline);
	if unlikely(!self->p_cmdline)
		goto err_exe_noprinter;
	self->p_exe = (DREF DeeStringObject *)exe; /* Inherit */

	/* Initialize all the other fields. */
	rwlock_init(&self->p_lock);
#ifndef NDEBUG
	memset(&self->p_id, 0xcc, sizeof(DWORD));
	memset(&self->p_handle, 0xcc, sizeof(HANDLE));
#endif /* !NDEBUG */
	self->p_state   = PROCESS_FNORMAL;
	self->p_std[0]  = NULL;
	self->p_std[1]  = NULL;
	self->p_std[2]  = NULL;
	self->p_environ = NULL;
	self->p_pwd     = NULL;
	return 0;
err_exe:
	Dee_Decref(exe);
err:
	ascii_printer_fini(&cmdline);
err_noprinter:
	return -1;
err_exe_noprinter:
	Dee_Decref(exe);
	goto err_noprinter;
}

PRIVATE struct type_gc process_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&process_clear
};

INTERN DeeTypeObject DeeProcess_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_Process_tp_name,
	/* .tp_doc      = */ DOC(S_Process_tp_doc),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (void *)&process_init,
				TYPE_FIXED_ALLOCATOR_GC(Process)
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
	/* .tp_gc            = */ &process_gc,
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



INTERN DeeTypeObject DeeProcEnumIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_ProcEnumIterator_tp_name,
	/* .tp_doc      = */ DOC(S_ProcEnumIterator_tp_doc),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
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

PRIVATE struct type_member enumproc_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeProcEnumIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeProcEnum_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_ProcEnum_tp_name,
	/* .tp_doc      = */ DOC(S_ProcEnum_tp_doc),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
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
#include "nt.c.inl"
#include "windows-pipe.c.inl"
#include "windows-cmdline.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEX_IPC_WINDOWS_C_INL */
