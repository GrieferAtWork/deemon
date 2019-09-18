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
#ifndef GUARD_DEX_FS_NL_INL
#define GUARD_DEX_FS_NL_INL 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1

#include <deemon/api.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#else /* CONFIG_HOST_WINDOWS */
#include <sys/stat.h>

#include <unistd.h>
#endif /* !CONFIG_HOST_WINDOWS */

/**/
#include "libfs.h"
/**/

#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/string.h>

#include <string.h>

#include "../time/libtime.h"

#ifdef _WIN32_WCE
#undef GetProcAddress
#define GetProcAddress GetProcAddressA
#endif /* _WIN32_WCE */

#ifndef __USE_KOS
#define strend(str) ((str) + strlen(str))
#endif /* !__USE_KOS */

DECL_BEGIN

INTERN DREF DeeObject *DCALL
nt_GetEnvironmentVariableA(char const *__restrict name) {
	DREF DeeObject *result, *new_result;
	DWORD bufsize = 256, error;
	result        = DeeString_NewBuffer(bufsize);
	if unlikely(!result)
		goto err_consume;
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		error = GetEnvironmentVariableA(name, DeeString_STR(result), bufsize + 1);
		DBG_ALIGNMENT_ENABLE();
		if (!error) {
			Dee_DecrefDokill(result);
			goto err;
		} /* Error. */
		if (error <= bufsize)
			break;
		/* Resize to fit. */
		new_result = DeeString_ResizeBuffer(result, error);
		if unlikely(!new_result)
			goto err_result;
		result  = new_result;
		bufsize = error;
	}
	new_result = DeeString_TryResizeBuffer(result, error);
	if likely(new_result)
		result = new_result;
	return result;
err_result:
	Dee_DecrefDokill(result);
err_consume:
	DeeError_Handled(ERROR_HANDLED_RESTORE);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
nt_GetTempPath(void) {
	LPWSTR buffer, new_buffer;
	DWORD bufsize = 256, error;
	buffer        = DeeString_NewWideBuffer(bufsize);
	if unlikely(!buffer)
		goto err;
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		error = GetTempPathW(bufsize + 1, buffer);
		DBG_ALIGNMENT_ENABLE();
		if (!error) {
			/* Error. */
			DeeError_SysThrowf(&DeeError_SystemError, GetLastError(),
			                   "Failed to lookup the path for tmp");
			goto err_result;
		}
		if (error <= bufsize)
			break;
		/* Resize to fit. */
		new_buffer = DeeString_ResizeWideBuffer(buffer, error);
		if unlikely(!new_buffer)
			goto err_result;
		buffer  = new_buffer;
		bufsize = error;
	}
	new_buffer = DeeString_TryResizeWideBuffer(buffer, error);
	if likely(new_buffer)
		buffer = new_buffer;
	return DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(buffer);
err:
	return NULL;
}


/* Work around a problem with long path names.
 * @return:  0: Successfully changed working directories.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN int DCALL
nt_SetCurrentDirectory(DeeObject *__restrict lpPathName) {
	LPWSTR wname;
	BOOL result;
	DWORD dwError;
	wname = (LPWSTR)DeeString_AsWide(lpPathName);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	result = SetCurrentDirectoryW(wname);
	if (!result && nt_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpPathName = nt_FixUncPath(lpPathName);
		if unlikely(!lpPathName)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(lpPathName);
		if unlikely(!wname) {
			Dee_Decref(lpPathName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		result  = SetCurrentDirectoryW(wname);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpPathName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !result;
err:
	return -1;
}


/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN int DCALL
nt_GetFileAttributesEx(DeeObject *__restrict lpFileName,
                       GET_FILEEX_INFO_LEVELS fInfoLevelId,
                       LPVOID lpFileInformation) {
	LPWSTR wname;
	BOOL result;
	DWORD dwError;
	wname = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	result = GetFileAttributesExW(wname, fInfoLevelId, lpFileInformation);
	if (!result && nt_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = nt_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!wname) {
			Dee_Decref(lpFileName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		result  = GetFileAttributesExW(wname, fInfoLevelId, lpFileInformation);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !result;
err:
	return -1;
}

/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN int DCALL
nt_GetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD *__restrict presult) {
	LPWSTR wname;
	DWORD dwError;
	wname = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	*presult = GetFileAttributesW(wname);
	if ((*presult == INVALID_FILE_ATTRIBUTES) && nt_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = nt_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!wname) {
			Dee_Decref(lpFileName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		*presult = GetFileAttributesW(wname);
		dwError  = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return *presult == INVALID_FILE_ATTRIBUTES;
err:
	return -1;
}


/* Work around a problem with long path names.
 * @return:  0: Successfully set attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN int DCALL
nt_SetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD dwFileAttributes) {
	LPWSTR wname;
	BOOL error;
	DWORD dwError;
	wname = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = SetFileAttributesW(wname, dwFileAttributes);
	if (!error && nt_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = nt_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!wname) {
			Dee_Decref(lpFileName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		error   = SetFileAttributesW(wname, dwFileAttributes);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !error;
err:
	return -1;
}

/* Work around a problem with long path names.
 * @return:  0: Successfully created the new directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN int DCALL
nt_CreateDirectory(DeeObject *__restrict lpPathName,
                   LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
	LPWSTR wname;
	BOOL error;
	DWORD dwError;
	wname = (LPWSTR)DeeString_AsWide(lpPathName);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = CreateDirectoryW(wname, lpSecurityAttributes);
	if (!error && nt_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpPathName = nt_FixUncPath(lpPathName);
		if unlikely(!lpPathName)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(lpPathName);
		if unlikely(!wname) {
			Dee_Decref(lpPathName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		error   = CreateDirectoryW(wname, lpSecurityAttributes);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpPathName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !error;
err:
	return -1;
}

/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF int DCALL
nt_RemoveDirectory(DeeObject *__restrict lpPathName) {
	LPWSTR wname;
	BOOL error;
	DWORD dwError;
	wname = (LPWSTR)DeeString_AsWide(lpPathName);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = RemoveDirectoryW(wname);
	if (!error && nt_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpPathName = nt_FixUncPath(lpPathName);
		if unlikely(!lpPathName)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(lpPathName);
		if unlikely(!wname) {
			Dee_Decref(lpPathName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		error   = RemoveDirectoryW(wname);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpPathName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !error;
err:
	return -1;
}

/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF int DCALL
nt_DeleteFile(DeeObject *__restrict lpFileName) {
	LPWSTR wname;
	BOOL error;
	DWORD dwError;
	wname = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = DeleteFileW(wname);
	if (!error && nt_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = nt_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!wname) {
			Dee_Decref(lpFileName);
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
		error   = DeleteFileW(wname);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !error;
err:
	return -1;
}

/* Work around a problem with long path names.
 * @return:  0: Successfully moved the given file.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTERN int DCALL
nt_MoveFile(DeeObject *__restrict lpExistingFileName,
            DeeObject *__restrict lpNewFileName) {
	LPWSTR wExistingFileName, wNewFileName;
	BOOL error;
	DWORD dwError;
	wExistingFileName = (LPWSTR)DeeString_AsWide(lpExistingFileName);
	if unlikely(!wExistingFileName)
		goto err;
	wNewFileName = (LPWSTR)DeeString_AsWide(lpNewFileName);
	if unlikely(!wNewFileName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = MoveFileW(wExistingFileName, wNewFileName);
	if (!error && nt_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpExistingFileName = nt_FixUncPath(lpExistingFileName);
		if unlikely(!lpExistingFileName)
			goto err;
		lpNewFileName = nt_FixUncPath(lpNewFileName);
		if unlikely(!lpNewFileName)
			goto err_existing;
		wExistingFileName = (LPWSTR)DeeString_AsWide(lpExistingFileName);
		if unlikely(!wExistingFileName)
			goto err_new;
		wNewFileName = (LPWSTR)DeeString_AsWide(lpNewFileName);
		if unlikely(!wNewFileName)
			goto err_new;
		/* Invoke the system call once again. */
		DBG_ALIGNMENT_DISABLE();
		error   = MoveFileW(wExistingFileName, wNewFileName);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpNewFileName);
		Dee_Decref(lpExistingFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !error;
err_new:
	Dee_Decref(lpNewFileName);
err_existing:
	Dee_Decref(lpExistingFileName);
err:
	return -1;
}

INTERN int DCALL
nt_CreateHardLink(DeeObject *__restrict lpFileName,
                  DeeObject *__restrict lpExistingFileName,
                  LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
	LPWSTR wFileName, wExistingFileName;
	BOOL error;
	DWORD dwError;
	wFileName = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!wFileName)
		goto err;
	wExistingFileName = (LPWSTR)DeeString_AsWide(lpExistingFileName);
	if unlikely(!wExistingFileName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = CreateHardLinkW(wFileName, wExistingFileName, lpSecurityAttributes);
	if (!error && nt_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpFileName = nt_FixUncPath(lpFileName);
		if unlikely(!lpFileName)
			goto err;
		lpExistingFileName = nt_FixUncPath(lpExistingFileName);
		if unlikely(!lpExistingFileName)
			goto err_filename;
		wFileName = (LPWSTR)DeeString_AsWide(lpFileName);
		if unlikely(!wFileName)
			goto err_existing;
		wExistingFileName = (LPWSTR)DeeString_AsWide(lpExistingFileName);
		if unlikely(!wExistingFileName)
			goto err_existing;
		/* Invoke the system call once again. */
		DBG_ALIGNMENT_DISABLE();
		error   = CreateHardLinkW(wFileName, wExistingFileName, lpSecurityAttributes);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpExistingFileName);
		Dee_Decref(lpFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !error;
err_existing:
	Dee_Decref(lpExistingFileName);
err_filename:
	Dee_Decref(lpFileName);
err:
	return -1;
}

typedef BOOLEAN(APIENTRY *LPCREATESYMBOLICLINKW)(LPCWSTR lpSymlinkFileName,
                                                 LPCWSTR lpTargetFileName,
                                                 DWORD dwFlags);
PRIVATE LPCREATESYMBOLICLINKW pCreateSymbolicLinkW = NULL;
PRIVATE WCHAR const wKernel32[]                    = { 'K', 'E', 'R', 'N', 'E', 'L', '3', '2', 0 };


INTERN int APIENTRY
nt_CreateSymbolicLink(DeeObject *__restrict lpSymlinkFileName,
                      DeeObject *__restrict lpTargetFileName,
                      DWORD dwFlags) {
	LPWSTR wSymlinkFileName, wTargetFileName;
	BOOLEAN error;
	DWORD dwError;
	LPCREATESYMBOLICLINKW callback = pCreateSymbolicLinkW;
	if (!callback) {
		DBG_ALIGNMENT_DISABLE();
		*(FARPROC *)&callback = GetProcAddress(GetModuleHandleW(wKernel32),
		                                       "CreateSymbolicLinkW");
		DBG_ALIGNMENT_ENABLE();
		if (!callback)
			*(void **)&callback = (void *)-1;
		*(void **)&pCreateSymbolicLinkW = *(void **)&callback;
	}
	if (*(void **)&callback == (void *)-1) {
		DeeError_Throwf(&DeeError_UnsupportedAPI,
		                "The host operating system does "
		                "not implement symbolic links");
		goto err;
	}
	wSymlinkFileName = (LPWSTR)DeeString_AsWide(lpSymlinkFileName);
	if unlikely(!wSymlinkFileName)
		goto err;
	wTargetFileName = (LPWSTR)DeeString_AsWide(lpTargetFileName);
	if unlikely(!wTargetFileName)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = (*callback)(wSymlinkFileName, wTargetFileName, dwFlags);
	if (!error && nt_IsUncError(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		lpSymlinkFileName = nt_FixUncPath(lpSymlinkFileName);
		if unlikely(!lpSymlinkFileName)
			goto err;
		lpTargetFileName = nt_FixUncPath(lpTargetFileName);
		if unlikely(!lpTargetFileName)
			goto err_filename;
		wSymlinkFileName = (LPWSTR)DeeString_AsWide(lpSymlinkFileName);
		if unlikely(!wSymlinkFileName)
			goto err_existing;
		wTargetFileName = (LPWSTR)DeeString_AsWide(lpTargetFileName);
		if unlikely(!wTargetFileName)
			goto err_existing;
		/* Invoke the system call once again. */
		DBG_ALIGNMENT_DISABLE();
		error   = (*callback)(wSymlinkFileName, wTargetFileName, dwFlags);
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		Dee_Decref(lpTargetFileName);
		Dee_Decref(lpSymlinkFileName);
		DBG_ALIGNMENT_DISABLE();
		SetLastError(dwError);
		DBG_ALIGNMENT_ENABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	return !error;
err_existing:
	Dee_Decref(lpTargetFileName);
err_filename:
	Dee_Decref(lpSymlinkFileName);
err:
	return -1;
}

DECL_END


#endif /* !GUARD_DEX_FS_NL_INL */
