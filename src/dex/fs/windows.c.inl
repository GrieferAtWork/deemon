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
#ifndef GUARD_DEX_FS_WINDOWS_C_INL
#define GUARD_DEX_FS_WINDOWS_C_INL 1
#define DEE_SOURCE 1

#include "libfs.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* strend() */
#include <deemon/system.h>
#include <deemon/tuple.h>

#include "_res.h"

#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif /* !CONFIG_NO_THREADS */

#include <hybrid/atomic.h>
#include <hybrid/unaligned.h>

#include <Windows.h>

#include "../time/libtime.h"

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


DECL_BEGIN

/* Use libc functions for case-insensitive UTF-8 string compare when available. */
#ifdef CONFIG_HAVE_memcasecmp
#define MEMCASEEQ(a, b, s) (memcasecmp(a, b, s) == 0)
#else /* CONFIG_HAVE_memcasecmp */
#define MEMCASEEQ(a, b, s) dee_memcaseeq((uint8_t *)(a), (uint8_t *)(b), s)
LOCAL bool dee_memcaseeq(uint8_t const *a, uint8_t const *b, size_t s) {
	while (s--) {
		if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
			return false;
		++a;
		++b;
	}
	return true;
}
#endif /* !CONFIG_HAVE_memcasecmp */


INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL nt_GetEnvironmentVariableA(char const *__restrict name);
INTDEF WUNUSED DREF DeeObject *DCALL nt_GetTempPath(void);

/* Work around a problem with long path names.
 * @return:  0: Successfully changed working directories.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1)) int DCALL nt_SetCurrentDirectory(DeeObject *__restrict lpPathName);

/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_GetFileAttributesEx(DeeObject *__restrict lpFileName,
                       GET_FILEEX_INFO_LEVELS fInfoLevelId,
                       LPVOID lpFileInformation);

/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
nt_GetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD *__restrict presult);

/* Work around a problem with long path names.
 * @return:  0: Successfully set attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_SetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD dwFileAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the new directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_CreateDirectory(DeeObject *__restrict lpPathName,
                   LPSECURITY_ATTRIBUTES lpSecurityAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_RemoveDirectory(DeeObject *__restrict lpPathName);

/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given file.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_DeleteFile(DeeObject *__restrict lpFileName);

/* Work around a problem with long path names.
 * @return:  0: Successfully moved the given file.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
nt_MoveFile(DeeObject *__restrict lpExistingFileName,
            DeeObject *__restrict lpNewFileName);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the hardlink.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
nt_CreateHardLink(DeeObject *__restrict lpFileName,
                  DeeObject *__restrict lpExistingFileName,
                  LPSECURITY_ATTRIBUTES lpSecurityAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the symlink.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF int DCALL
nt_CreateSymbolicLink(DeeObject *__restrict lpSymlinkFileName,
                      DeeObject *__restrict lpTargetFileName,
                      DWORD dwFlags);


typedef struct {
	OBJECT_HEAD
	LPWCH e_strings; /* [1..1][const] Environment strings, as retrieved by `GetEnvironmentStringsW()' */
	LPWCH e_iter;    /* [1..1] Next environment string (Pointed to an empty string when done). */
} Env;

PRIVATE WUNUSED NONNULL((1)) int DCALL
env_init(Env *__restrict self) {
again:
	DBG_ALIGNMENT_DISABLE();
	self->e_strings = GetEnvironmentStringsW();
	DBG_ALIGNMENT_ENABLE();
	if unlikely(!self->e_strings) {
		if (Dee_CollectMemory(42)) /* ??? */
			goto again;
		return -1;
	}
	self->e_iter = self->e_strings;
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
env_fini(Env *__restrict self) {
	DBG_ALIGNMENT_DISABLE();
	FreeEnvironmentStringsW(self->e_strings);
	DBG_ALIGNMENT_ENABLE();
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
env_bool(Env *__restrict self) {
	return self->e_iter[0] != 0;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
env_next(Env *__restrict self) {
	LPWCH result_string, next_string;
	DREF DeeObject *name, *value, *result;
#ifdef CONFIG_NO_THREADS
	result_string = ATOMIC_READ(self->e_iter);
	if (!*result_string)
		return ITER_DONE;
	next_string = result_string;
	while (*next_string++)
		;
	self->e_iter = next_string;
#else /* CONFIG_NO_THREADS */
	do {
		result_string = ATOMIC_READ(self->e_iter);
		if (!*result_string)
			return ITER_DONE;
		next_string = result_string;
		while (*next_string++)
			;
	} while (!ATOMIC_CMPXCH(self->e_iter, result_string, next_string));
#endif /* !CONFIG_NO_THREADS */
	/* Split the line to extract the name and value. */
	next_string = result_string + 1;
	/* XXX: This code assumes the double NUL-termination guarantied by windows. */
	while (*next_string++ && next_string[-1] != '=')
		;
	name = DeeString_NewWide(result_string,
	                         (size_t)(next_string - result_string) - 1,
	                         STRING_ERROR_FREPLAC);
	if unlikely(!name)
		goto err;
	value = DeeString_NewWide(next_string,
	                          wcslen(next_string),
	                          STRING_ERROR_FREPLAC);
	if unlikely(!value)
		goto err_name;
	result = DeeTuple_PackSymbolic(2, name, value); /* Inherit: name, value */
	if unlikely(!result)
		goto err_value;
	return result;
err_value:
	Dee_Decref(value);
err_name:
	Dee_Decref(name);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
enviterator_next_key(DeeObject *__restrict self) {
	LPWCH result_string, next_string;
	Env *me = (Env *)self;
#ifdef CONFIG_NO_THREADS
	result_string = ATOMIC_READ(me->e_iter);
	if (!*result_string)
		return ITER_DONE;
	next_string = result_string;
	while (*next_string++)
		;
	me->e_iter = next_string;
#else /* CONFIG_NO_THREADS */
	do {
		result_string = ATOMIC_READ(me->e_iter);
		if (!*result_string)
			return ITER_DONE;
		next_string = result_string;
		while (*next_string++)
			;
	} while (!ATOMIC_CMPXCH(me->e_iter, result_string, next_string));
#endif /* !CONFIG_NO_THREADS */
	/* Split the line to extract the name and value. */
	next_string = result_string + 1;
	/* XXX: This code assumes the double NUL-termination guarantied by windows. */
	while (*next_string++ && next_string[-1] != '=')
		;
	return DeeString_NewWide(result_string,
	                         (size_t)(next_string - result_string) - 1,
	                         STRING_ERROR_FREPLAC);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
enviterator_next_value(DeeObject *__restrict self) {
	LPWCH result_string, next_string;
	Env *me = (Env *)self;
#ifdef CONFIG_NO_THREADS
	result_string = ATOMIC_READ(me->e_iter);
	if (!*result_string)
		return ITER_DONE;
	next_string = result_string;
	while (*next_string++)
		;
	me->e_iter = next_string;
#else /* CONFIG_NO_THREADS */
	do {
		result_string = ATOMIC_READ(me->e_iter);
		if (!*result_string)
			return ITER_DONE;
		next_string = result_string;
		while (*next_string++)
			;
	} while (!ATOMIC_CMPXCH(me->e_iter, result_string, next_string));
#endif /* !CONFIG_NO_THREADS */
	/* Split the line to extract the name and value. */
	next_string = result_string + 1;
	/* XXX: This code assumes the double NUL-termination guarantied by windows. */
	while (*next_string++ && next_string[-1] != '=')
		;
	return DeeString_NewWide(next_string,
	                         wcslen(next_string),
	                         STRING_ERROR_FREPLAC);
}


PRIVATE struct type_member env_members[] = {
	TYPE_MEMBER_CONST("seq", &DeeEnv_Singleton),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeEnvIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_EnvIterator_tp_name,
	/* .tp_doc      = */ S_EnvIterator_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&env_init,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(Env)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&env_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&env_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&env_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ env_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



PRIVATE NONNULL((1)) void DCALL
err_unknown_env_var(DeeObject *__restrict name) {
	DeeError_Throwf(&DeeError_KeyError,
	                "Unknown environment variable `%k'",
	                name);
}

INTERN WUNUSED NONNULL((1)) bool DCALL
fs_hasenv(/*String*/ DeeObject *__restrict name) {
	LPWSTR wname;
	bool result;
	wname = (LPWSTR)DeeString_AsWide(name);
	if unlikely(!wname) {
		DeeError_Handled(ERROR_HANDLED_RESTORE);
		return false;
	}
	DBG_ALIGNMENT_DISABLE();
	result = GetEnvironmentVariableW(wname, NULL, 0) != 0;
	DBG_ALIGNMENT_ENABLE();
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fs_getenv(DeeObject *__restrict name, bool try_get) {
	LPWSTR buffer, new_buffer;
	DREF DeeObject *result;
	LPWSTR wname;
	DWORD bufsize = 256, error;
	wname         = (LPWSTR)DeeString_AsWide(name);
	if unlikely(!wname)
		goto err_consume;
	buffer = DeeString_NewWideBuffer(bufsize);
	if unlikely(!buffer)
		goto err_consume;
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		error = GetEnvironmentVariableW(wname, buffer, bufsize + 1);
		DBG_ALIGNMENT_ENABLE();
		if (!error) {
			/* Error. */
			if (!try_get)
				err_unknown_env_var(name);
			DeeString_FreeWideBuffer(buffer);
			goto err;
		}
		if (error <= bufsize)
			break;
		/* Resize to fit. */
		new_buffer = DeeString_ResizeWideBuffer(buffer, error);
		if unlikely(!new_buffer)
			goto err_result;
		buffer  = new_buffer;
		bufsize = error - 1;
	}
	buffer = DeeString_TruncateWideBuffer(buffer, error);
	result = DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
	if unlikely(!result)
		goto err_consume;
	return result;
err_result:
	DeeString_FreeWideBuffer(buffer);
err_consume:
	if (try_get)
		DeeError_Handled(ERROR_HANDLED_RESTORE);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_wprintenv(uint16_t const *__restrict name,
             struct unicode_printer *__restrict printer,
             bool try_get) {
	LPWSTR buffer;
	DWORD new_bufsize, bufsize = 256;
	buffer = unicode_printer_alloc_wchar(printer, bufsize);
	if unlikely(!buffer)
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	new_bufsize = GetEnvironmentVariableW((LPCWSTR)name, buffer, bufsize + 1);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(!new_bufsize) {
		if (!try_get) {
			DeeError_Throwf(&DeeError_KeyError,
			                "Unknown environment variable `%I16s'",
			                name);
			goto err_release;
		}
		unicode_printer_free_wchar(printer, buffer);
		return 1; /* Not found. */
	}
	if (new_bufsize > bufsize) {
		LPWSTR new_buffer;
		/* Increase the buffer and try again. */
		new_buffer = unicode_printer_resize_wchar(printer, buffer, new_bufsize);
		if unlikely(!new_buffer)
			goto err_release;
		buffer  = new_buffer;
		bufsize = new_bufsize;
		goto again;
	}
	if (unicode_printer_confirm_wchar(printer, buffer, new_bufsize) < 0)
		goto err;
	return 0;
err_release:
	unicode_printer_free_wchar(printer, buffer);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_printenv(/*utf-8*/ char const *__restrict name,
            struct unicode_printer *__restrict printer,
            bool try_get) {
	LPWSTR buffer, wname;
	DWORD new_bufsize, bufsize = 256;
	DREF DeeObject *wide_name;
	wide_name = DeeString_NewUtf8(name, strlen(name), STRING_ERROR_FSTRICT);
	if unlikely(!wide_name)
		goto err;
	wname = DeeString_AsWide(wide_name);
	if unlikely(!wname)
		goto err_wide_name;
	buffer = unicode_printer_alloc_wchar(printer, bufsize);
	if unlikely(!buffer)
		goto err_wide_name;
again:
	DBG_ALIGNMENT_DISABLE();
	new_bufsize = GetEnvironmentVariableW(wname, buffer, bufsize + 1);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(!new_bufsize) {
		if (!try_get) {
			DeeError_Throwf(&DeeError_KeyError,
			                "Unknown environment variable `%s'",
			                name);
			goto err_release;
		}
		unicode_printer_free_wchar(printer, buffer);
		Dee_Decref(wide_name);
		return 1; /* Not found. */
	}
	if (new_bufsize > bufsize) {
		LPWSTR new_buffer;
		/* Increase the buffer and try again. */
		new_buffer = unicode_printer_resize_wchar(printer, buffer, new_bufsize);
		if unlikely(!new_buffer)
			goto err_release;
		buffer  = new_buffer;
		bufsize = new_bufsize;
		goto again;
	}
	if (unicode_printer_confirm_wchar(printer, buffer, new_bufsize) < 0)
		goto err;
	Dee_Decref(wide_name);
	return 0;
err_release:
	unicode_printer_free_wchar(printer, buffer);
err_wide_name:
	Dee_Decref(wide_name);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fs_delenv(DeeObject *__restrict name) {
	LPWSTR wname;
	wname = (LPWSTR)DeeString_AsWide(name);
	if unlikely(!wname)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	if (!SetEnvironmentVariableW(wname, NULL)) {
		DBG_ALIGNMENT_ENABLE();
		err_unknown_env_var(name);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, name);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_setenv(DeeObject *__restrict name,
          DeeObject *__restrict value) {
	LPWSTR wname, wvalue;
	wname = (LPWSTR)DeeString_AsWide(name);
	if unlikely(!wname)
		goto err;
	wvalue = (LPWSTR)DeeString_AsWide(value);
	if unlikely(!wvalue)
		goto err;
again_setenv:
	DBG_ALIGNMENT_DISABLE();
	if (!SetEnvironmentVariableW(wname, wvalue)) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_CollectMemory(1))
				goto again_setenv;
		} else {
			DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
			                        "Failed to set environment variable `%k' to `%k'",
			                        name, value);
		}
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	/* Broadcast an environ-changed notification. */
	return DeeNotify_Broadcast(Dee_NOTIFICATION_CLASS_ENVIRON, name);
err:
	return -1;
}

INTERN WUNUSED DREF /*String*/ DeeObject *DCALL fs_gethostname(void) {
	DWORD bufsize = MAX_COMPUTERNAME_LENGTH + 1;
	LPWSTR buffer, new_buffer;
	if (DeeThread_CheckInterrupt())
		goto err;
	buffer = DeeString_NewWideBuffer(bufsize - 1);
	if unlikely(!buffer)
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	if (!GetComputerNameW(buffer, &bufsize)) {
		DWORD error = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (error == ERROR_BUFFER_OVERFLOW && bufsize &&
		    bufsize - 1 > WSTR_LENGTH(buffer)) {
			new_buffer = DeeString_ResizeWideBuffer(buffer, bufsize - 1);
			if unlikely(!new_buffer)
				goto err_result;
			buffer = new_buffer;
			goto again;
		}
		if (DeeNTSystem_IsBadAllocError(error)) {
			if (Dee_CollectMemory(1))
				goto again;
			goto err_result;
		}
		DeeError_Throwf(&DeeError_SystemError,
		                "Failed to retrieve the name of the hosting machine");
		goto err_result;
	}
	DBG_ALIGNMENT_ENABLE();
	/* Truncate the buffer and return it. */
	buffer = DeeString_TruncateWideBuffer(buffer, bufsize);
	return DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
err_result:
	DeeString_FreeWideBuffer(buffer);
err:
	return NULL;
}

PRIVATE DEFINE_STRING(tmpdir_0, "TMPDIR");
PRIVATE DEFINE_STRING(tmpdir_1, "TMP");
PRIVATE DEFINE_STRING(tmpdir_2, "TEMP");
PRIVATE DEFINE_STRING(tmpdir_3, "TEMPDIR");
PRIVATE DeeObject *tmpdir_vars[] = {
	(DeeObject *)&tmpdir_0,
	(DeeObject *)&tmpdir_1,
	(DeeObject *)&tmpdir_2,
	(DeeObject *)&tmpdir_3
};

INTERN WUNUSED DREF DeeObject *DCALL fs_gettmp(void) {
	DREF DeeObject *result;
	size_t i;
	if (DeeThread_CheckInterrupt())
		goto err;
	for (i = 0; i < COMPILER_STRLEN(tmpdir_vars); ++i)
		if ((result = fs_getenv(tmpdir_vars[i], true)) != NULL)
			goto done;
	/* Fallback: Lookup using windows. */
	result = nt_GetTempPath();
done:
	return result;
err:
	return NULL;
}


PRIVATE ATTR_COLD int DCALL
err_path_no_dir(DWORD error, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_NoDirectory, error,
	                               "Some part of the path %r is not a directory",
	                               path);
}

PRIVATE ATTR_COLD int DCALL
err_path_not_found(DWORD error, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileNotFound, error,
	                               "Path %r could not be found",
	                               path);
}

PRIVATE ATTR_COLD int DCALL
err_path_no_access(DWORD error, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, error,
	                               "Search permissions are not granted for path %r",
	                               path);
}

PRIVATE ATTR_COLD int DCALL
err_chattr_no_access(DWORD error, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, error,
	                               "Changes to the attributes of %r are not allowed",
	                               path);
}

PRIVATE ATTR_COLD int DCALL
err_handle_closed(DWORD error, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FileClosed, error,
	                               "The given handle %r has been closed",
	                               path);
}



typedef BOOL(WINAPI *LPGETUSERPROFILEDIRECTORYW)(HANDLE hToken, LPWSTR lpProfileDir, LPDWORD lpcchSize);
typedef BOOL(WINAPI *LPGETPROFILESDIRECTORYW)(LPWSTR lpProfileDir, LPDWORD lpcchSize);
typedef BOOL(WINAPI *LPGETDEFAULTUSERPROFILEDIRECTORYW)(LPWSTR lpProfileDir, LPDWORD lpcchSize);
typedef BOOL(WINAPI *LPGETALLUSERSPROFILEDIRECTORYW)(LPWSTR lpProfileDir, LPDWORD lpcchSize);
PRIVATE LPGETUSERPROFILEDIRECTORYW p_GetUserProfileDirectoryW               = NULL;
PRIVATE LPGETPROFILESDIRECTORYW p_GetProfilesDirectoryW                     = NULL;
PRIVATE LPGETDEFAULTUSERPROFILEDIRECTORYW p_GetDefaultUserProfileDirectoryW = NULL;
PRIVATE LPGETALLUSERSPROFILEDIRECTORYW p_GetAllUsersProfileDirectoryW       = NULL;
PRIVATE char const s_GetUserProfileDirectoryW[]                             = "GetUserProfileDirectoryW";
PRIVATE char const s_GetProfilesDirectoryW[]                                = "GetProfilesDirectoryW";
PRIVATE char const s_GetDefaultUserProfileDirectoryW[]                      = "GetDefaultUserProfileDirectoryW";
PRIVATE char const s_GetAllUsersProfileDirectoryW[]                         = "GetAllUsersProfileDirectoryW";
PRIVATE WCHAR s_USERENV[]                                                   = { 'U', 'S', 'E', 'R', 'E', 'N', 'V', 0 };

PRIVATE WCHAR s_userenv_dll[] = { 'u', 's', 'e', 'r', 'e', 'n', 'v', '.', 'd', 'l', 'l', 0 };

PRIVATE HMODULE DCALL get_userenv_module(void) {
	HMODULE result = GetModuleHandleW(s_USERENV);
	if (!result)
		result = LoadLibraryW(s_userenv_dll);
	return result;
}

INTERN int DCALL
nt_printhome_token(struct unicode_printer *__restrict printer, void *hToken, bool bTryGet) {
	LPGETUSERPROFILEDIRECTORYW my_GetUserProfileDirectoryW;
	LPWSTR wBuffer;
	DWORD dwBufsize;
	my_GetUserProfileDirectoryW = p_GetUserProfileDirectoryW;
	if (*(void **)&my_GetUserProfileDirectoryW == (void *)NULL) {
		/* Load the module */
		HMODULE hUserEnv;
		DBG_ALIGNMENT_DISABLE();
		hUserEnv = get_userenv_module();
		if (hUserEnv)
			my_GetUserProfileDirectoryW = (LPGETUSERPROFILEDIRECTORYW)GetProcAddress(hUserEnv, "GetUserProfileDirectoryW");
		DBG_ALIGNMENT_ENABLE();
		if (!my_GetUserProfileDirectoryW)
			*(void **)&my_GetUserProfileDirectoryW = (void *)ITER_DONE;
		p_GetUserProfileDirectoryW = my_GetUserProfileDirectoryW;
	}
	if (*(void **)&my_GetUserProfileDirectoryW == (void *)ITER_DONE) {
		if (bTryGet)
			return 1;
		return DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, NO_ERROR,
		                               "Cannot determine home of token %p (`GetUserProfileDirectoryW' not found)",
		                               hToken);
	}
	dwBufsize = PATH_MAX;
	wBuffer   = unicode_printer_alloc_wchar(printer, dwBufsize);
	if unlikely(!wBuffer)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	while (!(*my_GetUserProfileDirectoryW)((HANDLE)hToken, wBuffer, &dwBufsize)) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBufferTooSmall(dwError)) {
			LPWSTR wNewBuffer;
			wNewBuffer = unicode_printer_resize_wchar(printer, wBuffer, dwBufsize - 1);
			if unlikely(!wNewBuffer)
				goto err_release;
			wBuffer = wNewBuffer;
		} else {
			if (bTryGet) {
				unicode_printer_free_wchar(printer, wBuffer);
				return 1;
			}
			DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
			                        "Failed to determine home of token %p",
			                        hToken);
			goto err_release;
		}
		DBG_ALIGNMENT_DISABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	if (unicode_printer_confirm_wchar(printer, wBuffer, dwBufsize - 1) < 0)
		goto err;
	return 0;
err_release:
	unicode_printer_free_wchar(printer, wBuffer);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
nt_print_GetProfilesDirectory(struct unicode_printer *__restrict printer, bool bTryGet) {
	LPGETPROFILESDIRECTORYW my_GetProfilesDirectoryW;
	LPWSTR wBuffer;
	DWORD dwBufsize;
	my_GetProfilesDirectoryW = p_GetProfilesDirectoryW;
	if (*(void **)&my_GetProfilesDirectoryW == (void *)NULL) {
		/* Load the module */
		HMODULE hUserEnv;
		DBG_ALIGNMENT_DISABLE();
		hUserEnv = get_userenv_module();
		if (hUserEnv)
			my_GetProfilesDirectoryW = (LPGETPROFILESDIRECTORYW)GetProcAddress(hUserEnv, "GetProfilesDirectoryW");
		DBG_ALIGNMENT_ENABLE();
		if (!my_GetProfilesDirectoryW)
			*(void **)&my_GetProfilesDirectoryW = (void *)ITER_DONE;
		p_GetProfilesDirectoryW = my_GetProfilesDirectoryW;
	}
	if (*(void **)&my_GetProfilesDirectoryW == (void *)ITER_DONE) {
		if (bTryGet)
			return 1;
		return DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, NO_ERROR,
		                               "Cannot determine profiles directory (`GetProfilesDirectoryW' not found)");
	}
	dwBufsize = PATH_MAX;
	wBuffer   = unicode_printer_alloc_wchar(printer, dwBufsize);
	if unlikely(!wBuffer)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	while (!(*my_GetProfilesDirectoryW)(wBuffer, &dwBufsize)) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBufferTooSmall(dwError)) {
			LPWSTR wNewBuffer;
			wNewBuffer = unicode_printer_resize_wchar(printer, wBuffer, dwBufsize - 1);
			if unlikely(!wNewBuffer)
				goto err_release;
			wBuffer = wNewBuffer;
		} else {
			if (bTryGet) {
				unicode_printer_free_wchar(printer, wBuffer);
				return 1;
			}
			DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
			                        "Failed to determine profiles directory");
			goto err_release;
		}
		DBG_ALIGNMENT_DISABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	if (unicode_printer_confirm_wchar(printer, wBuffer, dwBufsize - 1) < 0)
		goto err;
	return 0;
err_release:
	unicode_printer_free_wchar(printer, wBuffer);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
nt_print_GetDefaultUserProfileDirectory(struct unicode_printer *__restrict printer, bool bTryGet) {
	LPGETDEFAULTUSERPROFILEDIRECTORYW my_GetDefaultUserProfileDirectoryW;
	LPWSTR wBuffer;
	DWORD dwBufsize;
	my_GetDefaultUserProfileDirectoryW = p_GetDefaultUserProfileDirectoryW;
	if (*(void **)&my_GetDefaultUserProfileDirectoryW == (void *)NULL) {
		/* Load the module */
		HMODULE hUserEnv;
		DBG_ALIGNMENT_DISABLE();
		hUserEnv = get_userenv_module();
		if (hUserEnv)
			my_GetDefaultUserProfileDirectoryW = (LPGETDEFAULTUSERPROFILEDIRECTORYW)GetProcAddress(hUserEnv, "GetDefaultUserProfileDirectoryW");
		DBG_ALIGNMENT_ENABLE();
		if (!my_GetDefaultUserProfileDirectoryW)
			*(void **)&my_GetDefaultUserProfileDirectoryW = (void *)ITER_DONE;
		p_GetDefaultUserProfileDirectoryW = my_GetDefaultUserProfileDirectoryW;
	}
	if (*(void **)&my_GetDefaultUserProfileDirectoryW == (void *)ITER_DONE) {
		if (bTryGet)
			return 1;
		return DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, NO_ERROR,
		                               "Cannot determine profiles directory (`GetDefaultUserProfileDirectoryW' not found)");
	}
	dwBufsize = PATH_MAX;
	wBuffer   = unicode_printer_alloc_wchar(printer, dwBufsize);
	if unlikely(!wBuffer)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	while (!(*my_GetDefaultUserProfileDirectoryW)(wBuffer, &dwBufsize)) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBufferTooSmall(dwError)) {
			LPWSTR wNewBuffer;
			wNewBuffer = unicode_printer_resize_wchar(printer, wBuffer, dwBufsize - 1);
			if unlikely(!wNewBuffer)
				goto err_release;
			wBuffer = wNewBuffer;
		} else {
			if (bTryGet) {
				unicode_printer_free_wchar(printer, wBuffer);
				return 1;
			}
			DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
			                        "Failed to determine profiles directory");
			goto err_release;
		}
		DBG_ALIGNMENT_DISABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	if (unicode_printer_confirm_wchar(printer, wBuffer, dwBufsize - 1) < 0)
		goto err;
	return 0;
err_release:
	unicode_printer_free_wchar(printer, wBuffer);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
nt_print_GetAllUsersProfileDirectory(struct unicode_printer *__restrict printer, bool bTryGet) {
	LPGETALLUSERSPROFILEDIRECTORYW my_GetAllUsersProfileDirectoryW;
	LPWSTR wBuffer;
	DWORD dwBufsize;
	my_GetAllUsersProfileDirectoryW = p_GetAllUsersProfileDirectoryW;
	if (*(void **)&my_GetAllUsersProfileDirectoryW == (void *)NULL) {
		/* Load the module */
		HMODULE hUserEnv;
		DBG_ALIGNMENT_DISABLE();
		hUserEnv = get_userenv_module();
		if (hUserEnv)
			my_GetAllUsersProfileDirectoryW = (LPGETALLUSERSPROFILEDIRECTORYW)GetProcAddress(hUserEnv, "GetAllUsersProfileDirectoryW");
		DBG_ALIGNMENT_ENABLE();
		if (!my_GetAllUsersProfileDirectoryW)
			*(void **)&my_GetAllUsersProfileDirectoryW = (void *)ITER_DONE;
		p_GetAllUsersProfileDirectoryW = my_GetAllUsersProfileDirectoryW;
	}
	if (*(void **)&my_GetAllUsersProfileDirectoryW == (void *)ITER_DONE) {
		if (bTryGet)
			return 1;
		return DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, NO_ERROR,
		                               "Cannot determine profiles directory (`GetAllUsersProfileDirectoryW' not found)");
	}
	dwBufsize = PATH_MAX;
	wBuffer   = unicode_printer_alloc_wchar(printer, dwBufsize);
	if unlikely(!wBuffer)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	while (!(*my_GetAllUsersProfileDirectoryW)(wBuffer, &dwBufsize)) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBufferTooSmall(dwError)) {
			LPWSTR wNewBuffer;
			wNewBuffer = unicode_printer_resize_wchar(printer, wBuffer, dwBufsize - 1);
			if unlikely(!wNewBuffer)
				goto err_release;
			wBuffer = wNewBuffer;
		} else {
			if (bTryGet) {
				unicode_printer_free_wchar(printer, wBuffer);
				return 1;
			}
			DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
			                        "Failed to determine profiles directory");
			goto err_release;
		}
		DBG_ALIGNMENT_DISABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	if (unicode_printer_confirm_wchar(printer, wBuffer, dwBufsize - 1) < 0)
		goto err;
	return 0;
err_release:
	unicode_printer_free_wchar(printer, wBuffer);
err:
	return -1;
}


INTERN int DCALL
nt_printhome_process(struct unicode_printer *__restrict printer, void *hProcess, bool bTryGet) {
	int result;
	HANDLE hProcessToken;
	DBG_ALIGNMENT_DISABLE();
	if unlikely(!OpenProcessToken((HANDLE)hProcess, TOKEN_QUERY, &hProcessToken)) {
		DWORD dwError;
		DBG_ALIGNMENT_ENABLE();
		if (bTryGet)
			return 1;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		return DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
		                               "Failed to determine home of process %p",
		                               hProcess);
	}
	result = nt_printhome_token(printer, (void *)hProcessToken, bTryGet);
	CloseHandle(hProcessToken);
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fs_printhome(struct unicode_printer *__restrict printer, bool try_get) {
	PRIVATE uint16_t const var_HOME[]        = { 'H', 'O', 'M', 'E', 0 };
	PRIVATE uint16_t const var_USERPROFILE[] = { 'U', 'S', 'E', 'R', 'P', 'R', 'O', 'F', 'I', 'L', 'E', 0 };
	PRIVATE uint16_t const var_HOMEDRIVE[]   = { 'H', 'O', 'M', 'E', 'D', 'R', 'I', 'V', 'E', 0 };
	PRIVATE uint16_t const var_HOMEPATH[]    = { 'H', 'O', 'M', 'E', 'P', 'A', 'T', 'H', 0 };
	int error;
	size_t old_length;
	if ((error = fs_wprintenv(var_HOME, printer, true)) <= 0)
		goto done_error;
	if ((error = fs_wprintenv(var_USERPROFILE, printer, true)) <= 0)
		goto done_error;
	old_length = UNICODE_PRINTER_LENGTH(printer);
	if ((error = fs_wprintenv(var_HOMEDRIVE, printer, true)) <= 0) {
		size_t before_homepath_index;
		if unlikely(error < 0)
			goto done_error;
		before_homepath_index = UNICODE_PRINTER_LENGTH(printer);
		while (before_homepath_index) {
			uint32_t ch;
			ch = UNICODE_PRINTER_GETCHAR(printer, before_homepath_index - 1);
			if (ch != '/' && ch != '\\' && !DeeUni_IsSpace(ch))
				break;
			--before_homepath_index;
		}
		unicode_printer_truncate(printer, before_homepath_index);
		if (unicode_printer_putascii(printer, '\\'))
			goto err;
		++before_homepath_index;
		error = fs_wprintenv(var_HOMEPATH, printer, true);
		if (error <= 0) {
			size_t remove_count;
			if unlikely(error < 0)
				goto done_error;
			/* Remove any additional leading slashes/space characters from $HOMEPATH */
			remove_count = 0;
			while (before_homepath_index + remove_count < UNICODE_PRINTER_LENGTH(printer)) {
				uint32_t ch;
				ch = UNICODE_PRINTER_GETCHAR(printer,
				                             before_homepath_index +
				                             remove_count);
				if (ch != '/' && ch != '\\' && !DeeUni_IsSpace(ch))
					break;
				++remove_count;
			}
			if (remove_count) {
				unicode_printer_memmove(printer,
				                        before_homepath_index,
				                        before_homepath_index + remove_count,
				                        UNICODE_PRINTER_LENGTH(printer) -
				                        (before_homepath_index + remove_count));
				unicode_printer_truncate(printer, UNICODE_PRINTER_LENGTH(printer) - remove_count);
			}
			goto done_error;
		}
		unicode_printer_truncate(printer, old_length);
	}
	/* Environment variables aren't holding the key...
	 * -> Let's try something else. */
	return nt_printhome_process(printer, GetCurrentProcess(), try_get);
done_error:
	return error;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fs_printuser(struct unicode_printer *__restrict printer, bool try_get) {
	DWORD dwBufsize = 64 + 1;
	LPWSTR wBuffer  = unicode_printer_alloc_wchar(printer, dwBufsize - 1);
	if unlikely(!wBuffer)
		goto err;
	while (!GetUserNameW(wBuffer, &dwBufsize)) {
		DWORD dwError;
		dwError = GetLastError();
		if (DeeNTSystem_IsBufferTooSmall(dwError)) {
			LPWSTR wNewBuffer;
			wNewBuffer = unicode_printer_resize_wchar(printer, wBuffer, dwBufsize - 1);
			if unlikely(!wNewBuffer)
				goto err_release;
		} else {
			if (try_get) {
				unicode_printer_free_wchar(printer, wBuffer);
				return 1;
			}
			DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
			                        "Failed to determine name of the current user");
			goto err_release;
		}
	}
	if (unicode_printer_confirm_wchar(printer, wBuffer, dwBufsize - 1) < 0)
		goto err;
	return 0;
err_release:
	unicode_printer_free_wchar(printer, wBuffer);
err:
	return -1;
}



INTERN WUNUSED DREF DeeObject *DCALL
fs_gethome(bool try_get) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (fs_printhome(&printer, try_get))
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
fs_getuser(bool try_get) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (fs_printuser(&printer, try_get))
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

#include <aclapi.h>

/* User (SSID) implementation. */
struct user_object {
	OBJECT_HEAD
	/* TODO: SSID */
	PSECURITY_DESCRIPTOR u_sd;  /* [0..1][const][owned(LocalFree)] The security descriptor buffer, or `NULL' if the current user is being referred to. */
	PSID                 u_sid; /* [0..1][valid_if(u_sd != NULL)][const] The SID descriptor of this user-object */
};

PRIVATE int DCALL
user_init(struct user_object *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DREF DeeObject *name_or_id = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:user", &name_or_id))
		goto err;
	if (!name_or_id) {
		self->u_sd  = NULL;
		self->u_sid = NULL;
	} else {
		/* TODO */
		DERROR_NOTIMPLEMENTED();
		goto err;
	}
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_user_get_home(DeeObject *__restrict UNUSED(self)) {
	return fs_gethome(false);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_user_get_domain(DeeObject *__restrict UNUSED(self)) {
	return fs_gethostname();
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_user_get_name(DeeObject *__restrict UNUSED(self)) {
	return fs_getuser(false);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
user_get_profiles_dir(DeeObject *__restrict UNUSED(self)) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (nt_print_GetProfilesDirectory(&printer, false))
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
user_get_defaulthome(DeeObject *__restrict UNUSED(self)) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (nt_print_GetDefaultUserProfileDirectory(&printer, false))
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
user_get_allusershome(DeeObject *__restrict UNUSED(self)) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (nt_print_GetAllUsersProfileDirectory(&printer, false))
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE struct type_getset user_class_getsets[] = {
	{ S_User_class_getset_name_name, &default_user_get_name, NULL, NULL, S_User_class_getset_name_doc },
	{ S_User_class_getset_home_name, &default_user_get_home, NULL, NULL, S_User_class_getset_home_doc },
	{ "domain_np", &default_user_get_domain, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Alias for :gethostname") },
	{ "profilesdir_np", &user_get_profiles_dir, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Returns the windows profiles directory containing all user profiles (Usually ${r\"C:\\Users\"})") },
	{ "defaultuserhome_np", &user_get_defaulthome, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Returns the home folder of the default profile (Usually ${r\"C:\\Users\\Default\"})") },
	{ "allusershome_np", &user_get_allusershome, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Returns the home folder of all profiles (Usually ${r\"C:\\ProgramData\"})") },
	{ NULL }
};


PRIVATE void DCALL
user_fini(struct user_object *__restrict self) {
	LocalFree(self->u_sd);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
user_get_name_and_domain(struct user_object *__restrict self,
                         LPWSTR *__restrict pname,
                         LPWSTR *__restrict pdomain,
                         PSID_NAME_USE peUse) {
	LPWSTR wNameBuffer;
	DWORD wNameBufSize = 64 + 1;
	LPWSTR wDomainBuffer;
	DWORD wDomainBufSize = 64 + 1;
	LPWSTR wNewBuffer;
	wNameBuffer = DeeString_NewWideBuffer(wNameBufSize - 1);
	if unlikely(!wNameBuffer)
		goto err;
	wDomainBuffer = DeeString_NewWideBuffer(wDomainBufSize - 1);
	if unlikely(!wDomainBuffer)
		goto err_name_buffer;
	DBG_ALIGNMENT_DISABLE();
	while (!LookupAccountSidW(NULL,
	                          self->u_sid,
	                          wNameBuffer, &wNameBufSize,
	                          wDomainBuffer, &wDomainBufSize,
	                          peUse)) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBufferTooSmall(dwError)) {
			wNewBuffer = DeeString_ResizeWideBuffer(wNameBuffer, wNameBufSize - 1);
			if unlikely(!wNewBuffer)
				goto err_domain_buffer;
			wNameBuffer = wNewBuffer;
			wNewBuffer  = DeeString_ResizeWideBuffer(wDomainBuffer, wDomainBufSize - 1);
			if unlikely(!wNewBuffer)
				goto err_domain_buffer;
			wDomainBuffer = wNewBuffer;
		} else {

			goto err_domain_buffer;
		}
		DBG_ALIGNMENT_DISABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	if (wNameBuffer[wNameBufSize])
		wNameBufSize = (DWORD)wcsnlen(wNameBuffer, wNameBufSize);
	if (wDomainBuffer[wDomainBufSize])
		wDomainBufSize = (DWORD)wcsnlen(wDomainBuffer, wDomainBufSize);
	wNameBuffer   = DeeString_TruncateWideBuffer(wNameBuffer, wNameBufSize);
	wDomainBuffer = DeeString_TruncateWideBuffer(wDomainBuffer, wDomainBufSize);
	*pname        = wNameBuffer;
	*pdomain      = wDomainBuffer;
	return 0;
err_domain_buffer:
	DeeString_FreeWideBuffer(wDomainBuffer);
err_name_buffer:
	DeeString_FreeWideBuffer(wNameBuffer);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
user_get_name(struct user_object *__restrict self) {
	LPWSTR name, domain;
	SID_NAME_USE use;
	if (!self->u_sd)
		return fs_getuser(false);
	if (user_get_name_and_domain(self, &name, &domain, &use))
		return NULL;
	DeeString_FreeWideBuffer(domain);
	return DeeString_PackWideBuffer(name, STRING_ERROR_FIGNORE);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
user_get_domain(struct user_object *__restrict self) {
	LPWSTR name, domain;
	SID_NAME_USE use;
	if (!self->u_sd)
		return fs_gethostname();
	if (user_get_name_and_domain(self, &name, &domain, &use))
		return NULL;
	DeeString_FreeWideBuffer(name);
	return DeeString_PackWideBuffer(domain, STRING_ERROR_FIGNORE);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
user_get_home(struct user_object *__restrict self) {
	LPWSTR name, domain;
	SID_NAME_USE use;
	if (!self->u_sd)
		return fs_gethome(false);
	{
		dssize_t error;
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		if (nt_print_GetProfilesDirectory(&printer, false))
			goto err;
		if (UNICODE_PRINTER_LENGTH(&printer) &&
		    UNICODE_PRINTER_GETCHAR(&printer, UNICODE_PRINTER_LENGTH(&printer) - 1) != '\\' &&
		    unicode_printer_putascii(&printer, '\\'))
			goto err;
		if (user_get_name_and_domain(self, &name, &domain, &use))
			goto err;
		DeeString_FreeWideBuffer(domain);
		error = unicode_printer_printwide(&printer, name, WSTR_LENGTH(name));
		DeeString_FreeWideBuffer(name);
		if unlikely(error < 0)
			goto err;
		return unicode_printer_pack(&printer);
err:
		unicode_printer_fini(&printer);
		return NULL;
	}
}

PRIVATE struct type_getset user_getsets[] = {
	{ S_User_getset_name_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&user_get_name, NULL, NULL, S_User_getset_name_doc },
	{ S_User_getset_home_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&user_get_home, NULL, NULL, S_User_getset_home_doc },
	{ "domain_np", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&user_get_domain, NULL, NULL,
	  DOC("->?Dstring\n"
	      "@throw SystemError Failed to retrieve the domain\n"
	      "Returns the windows-specific name of the domain associated with @this user") },
	{ NULL }
};

INTERN DeeTypeObject DeeUser_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "User",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (void *)&user_init,
				TYPE_FIXED_ALLOCATOR(DeeUserObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&user_fini,
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
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ user_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ user_class_getsets,
	/* .tp_class_members = */ NULL
};

PRIVATE WUNUSED DREF DeeObject *DCALL
nt_NewUserDescriptor(/*inherit(on_success)*/ PSID pSid,
                     /*inherit(on_success)*/ PSECURITY_DESCRIPTOR pSD) {
	DREF struct user_object *result;
	result = DeeObject_MALLOC(struct user_object);
	if unlikely(!result)
		goto done;
	result->u_sd  = pSD;
	result->u_sid = pSid;
	DeeObject_Init(result, &DeeUser_Type);
done:
	return (DREF DeeObject *)result;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
nt_NewUserDescriptorFromHandleOwner(HANDLE hHandle, SE_OBJECT_TYPE ObjectType) {
	PSID pSidOwner;
	DWORD dwError;
	PSECURITY_DESCRIPTOR pSD;
	DREF DeeObject *result;
	DBG_ALIGNMENT_DISABLE();
	dwError = GetSecurityInfo(hHandle, ObjectType,
	                          OWNER_SECURITY_INFORMATION,
	                          &pSidOwner, NULL, NULL, NULL, &pSD);
	if (dwError != ERROR_SUCCESS) {
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
		                        "Failed to query owner SID of %lu-typed handle %p",
		                        (unsigned long)ObjectType, (void *)hHandle);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	result = nt_NewUserDescriptor(pSidOwner, pSD);
	if likely(result)
		return result;
	LocalFree(pSD);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
nt_NewUserDescriptorFromHandleGroup(HANDLE hHandle, SE_OBJECT_TYPE ObjectType) {
	PSID pSidGroup;
	DWORD dwError;
	PSECURITY_DESCRIPTOR pSD;
	DREF DeeObject *result;
	DBG_ALIGNMENT_DISABLE();
	dwError = GetSecurityInfo(hHandle, ObjectType,
	                          GROUP_SECURITY_INFORMATION,
	                          NULL, &pSidGroup, NULL, NULL, &pSD);
	if (dwError != ERROR_SUCCESS) {
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		DeeNTSystem_ThrowErrorf(&DeeError_SystemError, dwError,
		                        "Failed to query group SID of %lu-typed handle %p",
		                        (unsigned long)ObjectType, (void *)hHandle);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	result = nt_NewUserDescriptor(pSidGroup, pSD);
	if likely(result)
		return result;
	LocalFree(pSD);
err:
	return NULL;
}







typedef struct {
	BY_HANDLE_FILE_INFORMATION s_info;   /* Windows-specific stat information. */
	DWORD                      s_ftype;  /* One of `FILE_TYPE_*' or `FILE_TYPE_UNKNOWN' when not determined. */
#define STAT_FNORMAL           0x0000    /* Normal information. */
#define STAT_FNOTIME           0x0001    /* Time stamps are unknown. */
#define STAT_FNOVOLSERIAL      0x0002    /* `dwVolumeSerialNumber' is unknown. */
#define STAT_FNOSIZE           0x0004    /* `nFileSize' is unknown. */
#define STAT_FNONLINK          0x0008    /* `nNumberOfLinks' is unknown. */
#define STAT_FNOFILEID         0x0010    /* `nFileIndex' is unknown. */
#define STAT_FNONTTYPE         0x0020    /* `s_ftype' is unknown. */
	uint16_t                   s_valid;  /* Set of `STAT_F*' */
	uint16_t                   s_pad[1]; /* ... */
	HANDLE                     s_hand;   /* [0..1|NULL(INVALID_HANDLE_VALUE)]
	                                      * Optional handle that may be used to load
	                                      * additional information upon request. */
} Stat;
#define Stat_Fini(x) ((x)->s_hand == INVALID_HANDLE_VALUE || (CloseHandle((x)->s_hand), 0))


/* STAT implementation. */
struct stat_object {
	OBJECT_HEAD
	Stat   st_stat;
};


/* Missing stat information errors. */
PRIVATE ATTR_NOINLINE ATTR_COLD int
DCALL
err_no_info(char const *__restrict level) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "The stat object does not contain any %s information",
	                       level);
}

PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_dev_info(void) {
	return err_no_info("device");
}

PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_ino_info(void) {
	return err_no_info("inode");
}

PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_link_info(void) {
	return err_no_info("nlink");
}

PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_uid_info(void) {
	return err_no_info("uid");
}

PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_gid_info(void) {
	return err_no_info("gid");
}

PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_size_info(void) {
	return err_no_info("size");
}

PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_time_info(void) {
	return err_no_info("time");
}

PRIVATE ATTR_NOINLINE ATTR_COLD int DCALL err_no_nttype_info(void) {
	return err_no_info("NT Type");
}

#define DOSTAT_FNORMAL   0x00
#define DOSTAT_FTRY      0x01
#define DOSTAT_FLSTAT    0x02
#define DOSTAT_FNOEXINFO 0x04

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
form_path_from_dfd_and_filename(DeeObject *dfd,
                                DeeObject *filename) {
	(void)dfd;
	(void)filename;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

/* @return:  1: `try_stat' was true and the given `path' could not be found.
 * @return:  0: Successfully did a stat() in the given `path'.
 * @return: -1: The state failed and an error was thrown.
 * @param: flags: Set of `DOSTAT_F*' */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Stat_Init(Stat *__restrict self,
          DeeObject *path,
          DeeObject *arg2,
          uint16_t flags) {
	HANDLE fd;
	int error;
again:
	if (DeeThread_CheckInterrupt())
		goto err;
	self->s_hand  = INVALID_HANDLE_VALUE; /* If inherited, set later. */
	self->s_ftype = FILE_TYPE_UNKNOWN;    /* Lazily initialized. */
	if (arg2) {
		/* `path = dfd; arg2 = dfd_relative_path;' */
		path = form_path_from_dfd_and_filename(path, arg2);
		if unlikely(!path)
			goto err;
		error = Stat_Init(self, path, NULL, flags);
		Dee_Decref(path);
		return error;
	}
	if (DeeString_Check(path)) {
		WIN32_FILE_ATTRIBUTE_DATA attrib;
		fd = DeeNTSystem_CreateFile(path, FILE_READ_ATTRIBUTES | READ_CONTROL,
		                            FILE_SHARE_READ | FILE_SHARE_WRITE |
		                            FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
		                            /* In lstat()-mode, open a reparse point.
		                             * NOTE: If the file isn't a reparse
		                             *       point, the flag is ignored ;) */
		                            (flags & DOSTAT_FLSTAT ? (FILE_ATTRIBUTE_NORMAL |
		                                                      FILE_FLAG_BACKUP_SEMANTICS |
		                                                      FILE_FLAG_OPEN_REPARSE_POINT)
		                                                   : (FILE_ATTRIBUTE_NORMAL |
		                                                      FILE_FLAG_BACKUP_SEMANTICS)),
		                            NULL);
		if (fd == INVALID_HANDLE_VALUE) {
			/* Try again, but leave out READ_CONTROL access permissions.
			 * NOTE: `READ_CONTROL' is normally required by the `st_uid' / `st_gid' fields. */
			fd = DeeNTSystem_CreateFile(path, FILE_READ_ATTRIBUTES,
			                            FILE_SHARE_READ | FILE_SHARE_WRITE |
			                            FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
			                            /* In lstat()-mode, open a reparse point.
			                             * NOTE: If the file isn't a reparse
			                             *       point, the flag is ignored ;) */
			                            (flags & DOSTAT_FLSTAT ? (FILE_ATTRIBUTE_NORMAL |
			                                                      FILE_FLAG_BACKUP_SEMANTICS |
			                                                      FILE_FLAG_OPEN_REPARSE_POINT)
			                                                   : (FILE_ATTRIBUTE_NORMAL |
			                                                      FILE_FLAG_BACKUP_SEMANTICS)),
			                            NULL);
		}
		if (fd == NULL)
			goto err;
		if (fd != INVALID_HANDLE_VALUE) {
			BOOL bOk;
			DBG_ALIGNMENT_DISABLE();
			bOk = GetFileInformationByHandle(fd, &self->s_info);
			if (!bOk) {
				CloseHandle(fd);
				DBG_ALIGNMENT_ENABLE();
				goto err_nt;
			}
			DBG_ALIGNMENT_ENABLE();
			self->s_valid = STAT_FNORMAL;
			self->s_hand  = fd; /* Inherit */
done:
			return 0;
		}
		/* Failed to open the file as a reparse point.
		 * All the fallback code here only works for
		 * regular stat, so we can't use it... */
#if 0
		if (flags & DOSTAT_FLSTAT)
			goto err_nt;
#endif

		/* CreateFile() failed. - Try a more direct approach. */
		bzero(&self->s_info, sizeof(BY_HANDLE_FILE_INFORMATION));
		DBG_ALIGNMENT_DISABLE();
		error = nt_GetFileAttributesEx(path, GetFileExInfoStandard, &attrib);
		DBG_ALIGNMENT_ENABLE();
		if unlikely(error < 0)
			goto err;
		if (!error) {
			/* It worked! */
			self->s_info.dwFileAttributes = attrib.dwFileAttributes;
			self->s_info.ftCreationTime   = attrib.ftCreationTime;
			self->s_info.ftLastAccessTime = attrib.ftLastAccessTime;
			self->s_info.ftLastWriteTime  = attrib.ftLastWriteTime;
			self->s_info.nFileSizeHigh    = attrib.nFileSizeHigh;
			self->s_info.nFileSizeLow     = attrib.nFileSizeLow;
			self->s_valid                 = (STAT_FNOVOLSERIAL | STAT_FNONLINK | STAT_FNOFILEID);
			goto done;
		}
		/* Nope. Still nothing...
		 * Try this one last thing. */
		error = nt_GetFileAttributes(path, &self->s_info.dwFileAttributes);
		if unlikely(error < 0)
			goto err;
		if unlikely(error)
			goto err_nt;
		self->s_valid = (STAT_FNOTIME | STAT_FNOVOLSERIAL | STAT_FNOSIZE |
		                 STAT_FNONLINK | STAT_FNOFILEID);
		goto done;
	}
	fd = DeeNTSystem_GetHandle(path);
	if (fd == INVALID_HANDLE_VALUE) {
		if (!DeeError_Catch(&DeeError_AttributeError) &&
		    !DeeError_Catch(&DeeError_NotImplemented) &&
		    !DeeError_Catch(&DeeError_FileClosed))
			goto err;
		/* Try to use the filename of the given object. */
		path = DeeFile_Filename(path);
		if unlikely(!path)
			goto err;
		error = Stat_Init(self, path, NULL, flags);
		Dee_Decref(path);
		return error;
	}
	/* Retrieve information by handle. */
	DBG_ALIGNMENT_DISABLE();
	if (GetFileInformationByHandle(fd, &self->s_info)) {
		DBG_ALIGNMENT_ENABLE();
		/* Immediately load the file type if the descriptor was given by the user. */
		if (flags & DOSTAT_FNOEXINFO) {
			self->s_valid |= STAT_FNONTTYPE;
		} else {
			HANDLE hCurrentProcess;
			DBG_ALIGNMENT_DISABLE();
			hCurrentProcess = GetCurrentProcess();
			DBG_ALIGNMENT_ENABLE();
			/* Duplicate the user-given handle, so we can use it to lookup additional information later. */
			if (!DuplicateHandle(hCurrentProcess, fd,
			                     hCurrentProcess, &self->s_hand,
			                     0, TRUE, DUPLICATE_SAME_ACCESS)) {
				self->s_hand = INVALID_HANDLE_VALUE;
				DBG_ALIGNMENT_DISABLE();
				self->s_ftype = GetFileType(fd);
				DBG_ALIGNMENT_ENABLE();
				self->s_valid = STAT_FNORMAL;
				if unlikely(self->s_ftype == FILE_TYPE_UNKNOWN)
					self->s_valid |= STAT_FNONTTYPE;
			}
		}
		goto done;
	}
err_nt:
	DBG_ALIGNMENT_DISABLE();
	error = (int)GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if ((flags & DOSTAT_FTRY) && DeeNTSystem_IsFileNotFoundError((DWORD)error))
		return 1; /* File not found. */
	if (DeeNTSystem_IsBadAllocError((DWORD)error)) {
		if (Dee_CollectMemory(1))
			goto again;
	} else if (DeeNTSystem_IsNotDir((DWORD)error)) {
		if (flags & DOSTAT_FTRY)
			return 1;
		err_path_no_dir((DWORD)error, path);
	} else if (DeeNTSystem_IsAccessDeniedError((DWORD)error)) {
		err_path_no_access((DWORD)error, path);
	} else if (DeeNTSystem_IsFileNotFoundError((DWORD)error)) {
		if (flags & DOSTAT_FTRY)
			return 1;
		err_path_not_found((DWORD)error, path);
	} else if (DeeNTSystem_IsBadF((DWORD)error)) {
		err_handle_closed((DWORD)error, path);
	} else {
		DeeNTSystem_ThrowErrorf(&DeeError_FSError, (DWORD)error,
		                        "Failed to open file %r",
		                        path);
	}
err:
	return -1;
}

/* Returns one of `FILE_TIME_*' describing the type of the given file.
 * When the type cannot be determined, return FILE_TIME_UNKNOWN when
 * `try_get' is true, or throw an error when it is false. */
PRIVATE DWORD DCALL
stat_get_nttype(Stat *__restrict self, bool try_get) {
	DWORD new_type, result = self->s_ftype;
	if (result == FILE_TYPE_UNKNOWN) {
		if (self->s_valid & STAT_FNONTTYPE)
			goto err_noinfo;
		if (self->s_hand == INVALID_HANDLE_VALUE) {
			ATOMIC_FETCHOR(self->s_valid, STAT_FNONTTYPE);
			goto err_noinfo;
		}
		if (DeeThread_CheckInterrupt())
			goto err;
		DBG_ALIGNMENT_DISABLE();
		result = GetFileType(self->s_hand);
		DBG_ALIGNMENT_ENABLE();
		if unlikely(result == FILE_TYPE_UNKNOWN)
			goto err_noinfo;
		new_type = ATOMIC_CMPXCH_VAL(self->s_ftype, FILE_TYPE_UNKNOWN, result);
		if (new_type != FILE_TYPE_UNKNOWN)
			result = new_type;
	}
	return result;
err_noinfo:
	if (!try_get)
		err_no_nttype_info();
err:
	return FILE_TYPE_UNKNOWN;
}




PRIVATE WUNUSED NONNULL((1)) int DCALL
stat_ctor(DeeStatObject *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeObject *path, *arg2 = NULL;
	if (DeeArg_Unpack(argc, argv, "o|on:" S_Stat_tp_name, &path, &arg2))
		return -1;
	return Stat_Init(&self->st_stat, path, arg2, DOSTAT_FNORMAL);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lstat_ctor(DeeStatObject *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeObject *path, *arg2 = NULL;
	if (DeeArg_Unpack(argc, argv, "o|on:" S_LStat_tp_name, &path, &arg2))
		return -1;
	return Stat_Init(&self->st_stat, path, arg2, DOSTAT_FLSTAT);
}

PRIVATE NONNULL((1)) void DCALL
stat_fini(DeeStatObject *__restrict self) {
	Stat_Fini(&self->st_stat);
}

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define FILETIME_GET64(x) (((x) << 32) | ((x) >> 32))
#else /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */
#define FILETIME_GET64(x)   (x)
#endif /* __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ */
#define FILETIME_PER_SECONDS 10000000 /* 100 nanoseconds / 0.1 microseconds. */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeTime_NewFiletime(FILETIME const *__restrict val) {
	uint64_t result;
	result = (FILETIME_GET64(*(uint64_t *)val) /
	          (FILETIME_PER_SECONDS / MICROSECONDS_PER_SECOND));
	result += time_yer2day(1601) * MICROSECONDS_PER_DAY;
	return DeeTime_New(result);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_dev(DeeStatObject *__restrict self) {
	if unlikely(self->st_stat.s_valid & STAT_FNOVOLSERIAL) {
		err_no_dev_info();
		return NULL;
	}
	return DeeInt_NewU32((uint32_t)self->st_stat.s_info.dwVolumeSerialNumber);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_ino(DeeStatObject *__restrict self) {
	if unlikely(self->st_stat.s_valid & STAT_FNOFILEID) {
		err_no_ino_info();
		return NULL;
	}
	return DeeInt_NewU64(((uint64_t)self->st_stat.s_info.nFileIndexHigh << 32) |
	                     ((uint64_t)self->st_stat.s_info.nFileIndexLow));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_mode(DeeStatObject *__restrict self) {
	uint32_t result = 0222 | 0111; /* XXX: executable should depend on extension. */
	if (self->st_stat.s_info.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
		result |= 0444;
	switch (stat_get_nttype(&self->st_stat, true)) {

	case FILE_TYPE_CHAR:
		result |= STAT_IFCHR;
		break;

	case FILE_TYPE_PIPE:
		result |= STAT_IFIFO;
		break;

	case FILE_TYPE_REMOTE:
		result |= STAT_IFSOCK;
		break;

	case FILE_TYPE_DISK:
		/* Actually means a file on-disk when the device flag isn't set. */
		if (self->st_stat.s_info.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) {
			result |= STAT_IFBLK;
			break;
		}
		ATTR_FALLTHROUGH
	default:
		if (self->st_stat.s_info.dwFileAttributes &
		    FILE_ATTRIBUTE_DIRECTORY)
			result |= STAT_IFDIR;
		else if (self->st_stat.s_info.dwFileAttributes &
		         FILE_ATTRIBUTE_REPARSE_POINT)
			result |= STAT_IFLNK;
		else {
			result |= STAT_IFREG;
		}
		break;
	}
	return DeeInt_NewU32(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_nlink(DeeStatObject *__restrict self) {
	if unlikely(self->st_stat.s_valid & STAT_FNONLINK)
		goto err_nolink;
	return DeeInt_NewU32(self->st_stat.s_info.nNumberOfLinks);
err_nolink:
	err_no_link_info();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_uid(DeeStatObject *__restrict self) {
	if unlikely(self->st_stat.s_hand == INVALID_HANDLE_VALUE)
		goto err_nouid;
	return nt_NewUserDescriptorFromHandleOwner(self->st_stat.s_hand, SE_FILE_OBJECT);
err_nouid:
	err_no_uid_info();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_gid(DeeStatObject *__restrict self) {
	if unlikely(self->st_stat.s_hand == INVALID_HANDLE_VALUE)
		goto err_nogid;
	return nt_NewUserDescriptorFromHandleGroup(self->st_stat.s_hand, SE_FILE_OBJECT);
err_nogid:
	err_no_gid_info();
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
stat_get_rdev(DeeStatObject *__restrict UNUSED(self)) {
	err_no_info("rdev");
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_size(DeeStatObject *__restrict self) {
	if unlikely(self->st_stat.s_valid & STAT_FNOSIZE)
		goto err_nosize;
	return DeeInt_NewU64(((uint64_t)self->st_stat.s_info.nFileSizeHigh << 32) |
	                     ((uint64_t)self->st_stat.s_info.nFileSizeLow));
err_nosize:
	err_no_size_info();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_atime(DeeStatObject *__restrict self) {
	if unlikely(self->st_stat.s_valid & STAT_FNOTIME)
		goto err_notime;
	return DeeTime_NewFiletime(&self->st_stat.s_info.ftLastAccessTime);
err_notime:
	err_no_time_info();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_mtime(DeeStatObject *__restrict self) {
	if unlikely(self->st_stat.s_valid & STAT_FNOTIME)
		goto err_notime;
	return DeeTime_NewFiletime(&self->st_stat.s_info.ftLastWriteTime);
err_notime:
	err_no_time_info();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_ctime(DeeStatObject *__restrict self) {
	if unlikely(self->st_stat.s_valid & STAT_FNOTIME)
		goto err_notime;
	return DeeTime_NewFiletime(&self->st_stat.s_info.ftCreationTime);
err_notime:
	err_no_time_info();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_isdir(DeeStatObject *__restrict self) {
	if (self->st_stat.s_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return_true;
	return_false;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_ischr(DeeStatObject *__restrict self) {
	if (!(self->st_stat.s_info.dwFileAttributes & FILE_ATTRIBUTE_DEVICE))
		return_false;
	return_bool(stat_get_nttype(&self->st_stat, true) == FILE_TYPE_CHAR);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_isblk(DeeStatObject *__restrict self) {
	if (!(self->st_stat.s_info.dwFileAttributes & FILE_ATTRIBUTE_DEVICE))
		return_false;
	return_bool(stat_get_nttype(&self->st_stat, true) == FILE_TYPE_DISK);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_isreg(DeeStatObject *__restrict self) {
	return_bool(!(self->st_stat.s_info.dwFileAttributes &
	              (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_DIRECTORY |
	               FILE_ATTRIBUTE_REPARSE_POINT)));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_isfifo(DeeStatObject *__restrict self) {
	return_bool(stat_get_nttype(&self->st_stat, true) == FILE_TYPE_PIPE);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_islnk(DeeStatObject *__restrict self) {
	return_bool(stat_get_nttype(&self->st_stat, true) == FILE_TYPE_PIPE);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_issock(DeeStatObject *__restrict self) {
	return_bool(stat_get_nttype(&self->st_stat, true) == FILE_TYPE_REMOTE);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_getntattr_np(DeeStatObject *__restrict self) {
	return DeeInt_NewU32(self->st_stat.s_info.dwFileAttributes);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_getnttype_np(DeeStatObject *__restrict self) {
	DWORD result = stat_get_nttype(&self->st_stat, false);
	if unlikely(result == FILE_TYPE_UNKNOWN)
		return NULL;
	return DeeInt_NewU32(result);
}

PRIVATE struct type_getset stat_getsets[] = {
	{ S_Stat_getset_st_dev_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_dev, NULL, NULL, S_Stat_getset_st_dev_doc },
	{ S_Stat_getset_st_ino_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_ino, NULL, NULL, S_Stat_getset_st_ino_doc },
	{ S_Stat_getset_st_mode_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_mode, NULL, NULL, S_Stat_getset_st_mode_doc },
	{ S_Stat_getset_st_nlink_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_nlink, NULL, NULL, S_Stat_getset_st_nlink_doc },
	{ S_Stat_getset_st_uid_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_uid, NULL, NULL, S_Stat_getset_st_uid_doc },
	{ S_Stat_getset_st_gid_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_gid, NULL, NULL, S_Stat_getset_st_gid_doc },
	{ S_Stat_getset_st_rdev_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_rdev, NULL, NULL, S_Stat_getset_st_rdev_doc },
	{ S_Stat_getset_st_size_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_size, NULL, NULL, S_Stat_getset_st_size_doc },
	{ S_Stat_getset_st_atime_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_atime, NULL, NULL, S_Stat_getset_st_atime_doc },
	{ S_Stat_getset_st_mtime_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_mtime, NULL, NULL, S_Stat_getset_st_mtime_doc },
	{ S_Stat_getset_st_ctime_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_ctime, NULL, NULL, S_Stat_getset_st_ctime_doc },
	{ S_Stat_getset_isdir_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_isdir, NULL, NULL, S_Stat_getset_isdir_doc },
	{ S_Stat_getset_ischr_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_ischr, NULL, NULL, S_Stat_getset_ischr_doc },
	{ S_Stat_getset_isblk_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_isblk, NULL, NULL, S_Stat_getset_isblk_doc },
	{ S_Stat_getset_isreg_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_isreg, NULL, NULL, S_Stat_getset_isreg_doc },
	{ S_Stat_getset_isfifo_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_isfifo, NULL, NULL, S_Stat_getset_isfifo_doc },
	{ S_Stat_getset_islnk_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_islnk, NULL, NULL, S_Stat_getset_islnk_doc },
	{ S_Stat_getset_issock_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_issock, NULL, NULL, S_Stat_getset_issock_doc },

	/* Non-portable NT extensions. */
	{ "ntattr_np", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_getntattr_np, NULL, NULL,
	  DOC("->?Dint\n"
	      "Non-portable windows extension for retrieving the NT attributes of the stat-file, those "
	      "attributes being a set of the `FILE_ATTRIBUTE_*' constants found in windows system headers") },
	{ "nttype_np", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_getnttype_np, NULL, NULL,
	  DOC("->?Dint\n"
	      "@throw ValueError @this stat-file does not contain valid NT-type information\n"
	      "Non-portable windows extension for retrieving the NT type of this stat-file, that "
	      "type being one of the `FILE_TYPE_*' constants found in windows system headers") },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_exists(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *path, *arg2 = NULL;
	int error;
	Stat buf;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeArg_Unpack(argc, argv, "o|on:" S_Stat_class_function_exists_name, &path, &arg2))
		goto err;
	if (DeeString_Check(path) && !arg2) {
		DWORD attr; /* Do a quick attribute query. */
		error = nt_GetFileAttributes(path, &attr);
		if unlikely(error < 0)
			goto err;
		return_bool_(!error);
	}
	error = Stat_Init(&buf, path, arg2,
	                  self == (DeeObject *)&DeeLStat_Type
	                  ? DOSTAT_FTRY | DOSTAT_FNOEXINFO | DOSTAT_FLSTAT
	                  : DOSTAT_FTRY | DOSTAT_FNOEXINFO);
	if unlikely(error < 0)
		goto err;
	if (error > 0)
		return_false;
	Stat_Fini(&buf);
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isdir(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *path, *arg2 = NULL;
	int error;
	Stat buf;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeArg_Unpack(argc, argv, "o|on:" S_Stat_class_function_isdir_name, &path, &arg2))
		goto err;
	if (DeeString_Check(path) && !arg2) {
		DWORD attr; /* Do a quick attribute query. */
		error = nt_GetFileAttributes(path, &attr);
		if unlikely(error < 0)
			goto err;
		if (!error)
			return_bool(attr & FILE_ATTRIBUTE_DIRECTORY);
	}
	error = Stat_Init(&buf, path, arg2,
	                  self == (DeeObject *)&DeeLStat_Type
	                  ? DOSTAT_FTRY | DOSTAT_FNOEXINFO | DOSTAT_FLSTAT
	                  : DOSTAT_FTRY | DOSTAT_FNOEXINFO);
	if unlikely(error < 0)
		goto err;
	if (error > 0)
		return_false;
	error = (int)(buf.s_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	Stat_Fini(&buf);
	return_bool_(error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_ischr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *path, *arg2 = NULL;
	int error;
	Stat buf;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeArg_Unpack(argc, argv, "o|on:" S_Stat_class_function_ischr_name, &path, &arg2))
		goto err;
	error = Stat_Init(&buf, path, arg2,
	                  self == (DeeObject *)&DeeLStat_Type
	                  ? DOSTAT_FTRY | DOSTAT_FLSTAT
	                  : DOSTAT_FTRY);
	if unlikely(error < 0)
		goto err;
	if (error > 0)
		return_false;
	if (!(buf.s_info.dwFileAttributes & FILE_ATTRIBUTE_DEVICE))
		error = 0;
	else {
		error = stat_get_nttype(&buf, true) == FILE_TYPE_CHAR;
	}
	Stat_Fini(&buf);
	return_bool_(error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isblk(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *path, *arg2 = NULL;
	int error;
	Stat buf;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeArg_Unpack(argc, argv, "o|on:" S_Stat_class_function_isblk_name, &path, &arg2))
		goto err;
	error = Stat_Init(&buf, path, arg2,
	                  self == (DeeObject *)&DeeLStat_Type
	                  ? DOSTAT_FTRY | DOSTAT_FLSTAT
	                  : DOSTAT_FTRY);
	if unlikely(error < 0)
		goto err;
	if (error > 0)
		return_false;
	if (!(buf.s_info.dwFileAttributes & FILE_ATTRIBUTE_DEVICE))
		error = 0;
	else {
		error = stat_get_nttype(&buf, true) == FILE_TYPE_DISK;
	}
	Stat_Fini(&buf);
	return_bool_(error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isreg(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *path, *arg2 = NULL;
	int error;
	Stat buf;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeArg_Unpack(argc, argv, "o|on:" S_Stat_class_function_isreg_name, &path, &arg2))
		goto err;
	if (DeeString_Check(path) && !arg2) {
		DWORD attr; /* Do a quick attribute query. */
		error = nt_GetFileAttributes(path, &attr);
		if unlikely(error < 0)
			goto err;
		if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) &&
		    self != (DeeObject *)&DeeLStat_Type)
			goto do_normal_stat;
		if (!error)
			return_bool(!(attr & (FILE_ATTRIBUTE_DEVICE |
			                      FILE_ATTRIBUTE_DIRECTORY |
			                      FILE_ATTRIBUTE_REPARSE_POINT)));
	}
do_normal_stat:
	error = Stat_Init(&buf, path, arg2,
	                  self == (DeeObject *)&DeeLStat_Type
	                  ? DOSTAT_FTRY | DOSTAT_FNOEXINFO | DOSTAT_FLSTAT
	                  : DOSTAT_FTRY | DOSTAT_FNOEXINFO);
	if unlikely(error < 0)
		goto err;
	if (error > 0)
		return_false;
	error = (int)(buf.s_info.dwFileAttributes &
	              (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_DIRECTORY |
	               FILE_ATTRIBUTE_REPARSE_POINT));
	Stat_Fini(&buf);
	return_bool_(!error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isfifo(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *path, *arg2 = NULL;
	int error;
	Stat buf;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeArg_Unpack(argc, argv, "o|on:" S_Stat_class_function_isfifo_name, &path, &arg2))
		goto err;
	error = Stat_Init(&buf, path, arg2,
	                  self == (DeeObject *)&DeeLStat_Type
	                  ? DOSTAT_FTRY | DOSTAT_FLSTAT
	                  : DOSTAT_FTRY);
	if unlikely(error < 0)
		goto err;
	if (error > 0)
		return_false;
	error = stat_get_nttype(&buf, true) == FILE_TYPE_PIPE;
	Stat_Fini(&buf);
	return_bool_(error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_islnk(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
	DeeObject *path, *arg2 = NULL;
	int error;
	Stat buf;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeArg_Unpack(argc, argv, "o|on:" S_Stat_class_function_islnk_name, &path, &arg2))
		goto err;
	if (DeeString_Check(path) && !arg2) {
		DWORD attr; /* Do a quick attribute query. */
		error = nt_GetFileAttributes(path, &attr);
		if unlikely(error < 0)
			goto err;
		if (!error)
			return_bool(attr & FILE_ATTRIBUTE_REPARSE_POINT);
	}
	error = Stat_Init(&buf, path, arg2,
	                  DOSTAT_FTRY |
	                  DOSTAT_FNOEXINFO |
	                  DOSTAT_FLSTAT);
	if unlikely(error < 0)
		goto err;
	if (error > 0)
		return_false;
	error = (int)(buf.s_info.dwFileAttributes &
	              FILE_ATTRIBUTE_REPARSE_POINT);
	Stat_Fini(&buf);
	return_bool_(error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_issock(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *path, *arg2 = NULL;
	int error;
	Stat buf;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeArg_Unpack(argc, argv, "o|on:" S_Stat_class_function_issock_name, &path, &arg2))
		goto err;
	error = Stat_Init(&buf, path, arg2,
	                  self == (DeeObject *)&DeeLStat_Type
	                  ? DOSTAT_FTRY | DOSTAT_FLSTAT
	                  : DOSTAT_FTRY);
	if unlikely(error < 0)
		goto err;
	if (error > 0)
		return_false;
	error = stat_get_nttype(&buf, true) == FILE_TYPE_REMOTE;
	Stat_Fini(&buf);
	return_bool_(error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_ishidden(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *path, *arg2 = NULL;
	int error;
	Stat buf;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeArg_Unpack(argc, argv, "o:" S_Stat_class_function_ishidden_name, &path, &arg2))
		goto err;
	if (DeeString_Check(path) && !arg2) {
		DWORD attr; /* Do a quick attribute query. */
		error = nt_GetFileAttributes(path, &attr);
		if unlikely(error < 0)
			goto err;
		if ((attr & FILE_ATTRIBUTE_REPARSE_POINT) &&
		    self != (DeeObject *)&DeeLStat_Type)
			goto do_normal_stat;
		if (!error)
			return_bool(attr & FILE_ATTRIBUTE_HIDDEN);
	}
do_normal_stat:
	error = Stat_Init(&buf, path, arg2,
	                  self == (DeeObject *)&DeeLStat_Type
	                  ? (DOSTAT_FTRY | DOSTAT_FNOEXINFO | DOSTAT_FLSTAT)
	                  : (DOSTAT_FTRY | DOSTAT_FNOEXINFO));
	if unlikely(error < 0)
		goto err;
	if (error > 0)
		return_false;
	error = (int)(buf.s_info.dwFileAttributes &
	              FILE_ATTRIBUTE_HIDDEN);
	Stat_Fini(&buf);
	return_bool_(error);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) bool DCALL
is_exe_filename(DeeObject *__restrict path) {
	DREF DeeObject *pathext_ob;
	bool result;
	char *ext_begin, *ext_end, *pathext;
	size_t ext_size;
	ext_begin = ext_end = DeeString_END(path);
	for (;;) {
		if (ext_begin == DeeString_STR(path))
			return false;
		if (ext_begin[-1] == '.')
			break;
		--ext_begin;
		if (*ext_begin == '/' ||
		    *ext_begin == '\\')
			return false;
	}
	ext_size = (size_t)(ext_end - ext_begin);
	/* Got the file path. */
	pathext_ob = nt_GetEnvironmentVariableA("PATHEXT");
	if likely(pathext_ob)
		pathext = DeeString_STR(pathext_ob);
	else {
		pathext = (char *)".COM;.EXE;.BAT;.CMD";
	}
	result = false;
	while (*pathext) {
		char *next = strchr(pathext, ';');
		if (!next)
			next = strend(pathext);
		/* Check if this is the extension we've been looking for. */
		if (ext_size == (size_t)(next - pathext) &&
		    MEMCASEEQ(pathext, ext_begin, ext_size * sizeof(char))) {
			result = true;
			break;
		}
		pathext = next;
		if (*pathext)
			++pathext; /* Skip `;' */
	}
	Dee_XDecref(pathext_ob);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isexe(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
	DeeObject *path, *arg2 = NULL;
	bool result;
	if (DeeArg_Unpack(argc, argv, "o|on:" S_Stat_class_function_isexe_name, &path, &arg2))
		goto err;
	if (DeeThread_CheckInterrupt())
		goto err;
	if unlikely(arg2) {
		path = form_path_from_dfd_and_filename(path, arg2);
		if unlikely(!path)
			goto err;
		result = is_exe_filename(path);
		Dee_Decref(path);
	} else if (DeeString_Check(path)) {
		result = is_exe_filename(path);
	} else {
		if (DeeInt_Check(path)) {
			/* TODO: Solve the confusion of HANDLE vs. int-fd on windows! */
			HANDLE fd; /* Support for descriptor-based isexe() */
			if (DeeObject_AsUIntptr(path, (uintptr_t *)&fd))
				goto err;
			path = DeeNTSystem_GetFilenameOfHandle(fd);
		} else {
			path = DeeFile_Filename(path);
		}
		if unlikely(!path)
			goto err;
		result = is_exe_filename(path);
		Dee_Decref(path);
	}
	return_bool_(result);
err:
	return NULL;
}

PRIVATE struct type_method stat_class_methods[] = {
	{ S_Stat_class_function_exists_name, &stat_class_exists, S_Stat_class_function_exists_doc },
	{ S_Stat_class_function_isdir_name, &stat_class_isdir, S_Stat_class_function_isdir_doc },
	{ S_Stat_class_function_ischr_name, &stat_class_ischr, S_Stat_class_function_ischr_doc },
	{ S_Stat_class_function_isblk_name, &stat_class_isblk, S_Stat_class_function_isblk_doc },
	{ S_Stat_class_function_isreg_name, &stat_class_isreg, S_Stat_class_function_isreg_doc },
	{ S_Stat_class_function_isfifo_name, &stat_class_isfifo, S_Stat_class_function_isfifo_doc },
	{ S_Stat_class_function_islnk_name, &stat_class_islnk, S_Stat_class_function_islnk_doc },
	{ S_Stat_class_function_issock_name, &stat_class_issock, S_Stat_class_function_issock_doc },
	{ S_Stat_class_function_ishidden_name, &stat_class_ishidden, S_Stat_class_function_ishidden_doc },
	{ S_Stat_class_function_isexe_name, &stat_class_isexe, S_Stat_class_function_isexe_doc },
	{ NULL }
};

INTERN DeeTypeObject DeeStat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_Stat_tp_name,
	/* .tp_doc      = */ S_Stat_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (void *)&stat_ctor,
				TYPE_FIXED_ALLOCATOR(DeeStatObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&stat_fini,
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
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ stat_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ stat_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DeeLStat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_LStat_tp_name,
	/* .tp_doc      = */ S_LStat_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeStat_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (void *)&lstat_ctor,
				TYPE_FIXED_ALLOCATOR(DeeStatObject)
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
	/* .tp_iter_next     = */ NULL,
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




INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_chattr_np(DeeObject *__restrict path,
             DeeObject *__restrict new_attr) {
	DWORD attr;
	int error;
	if (!DeeString_Check(path)) {
		if (DeeInt_Check(path)) {
			HANDLE fd; /* Support for descriptor-based chmod() */
			if (DeeObject_AsUIntptr(path, (uintptr_t *)&fd))
				goto err;
			path = DeeNTSystem_GetFilenameOfHandle(fd);
		} else {
			path = DeeFile_Filename(path);
		}
		if unlikely(!path)
			goto err;
		error = fs_chattr_np(path, new_attr);
		Dee_Decref(path);
		return error;
	}
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AsUInt32(new_attr, (uint32_t *)&attr))
		goto err;
again:
	error = nt_SetFileAttributes(path, attr);
	if (error <= 0)
		return error;
	DBG_ALIGNMENT_DISABLE();
	error = (int)GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsBadAllocError((DWORD)error)) {
		if (Dee_CollectMemory(1))
			goto again;
	} else if (DeeNTSystem_IsAccessDeniedError((DWORD)error)) {
		err_chattr_no_access((DWORD)error, path);
	} else if (DeeNTSystem_IsUnsupportedError((DWORD)error)) {
		DeeNTSystem_ThrowErrorf(&DeeError_UnsupportedAPI, (DWORD)error,
		                        "The filesystem hosting the path %r does "
		                        "not support the changing of NT attributes",
		                        path);
	} else {
		DeeNTSystem_ThrowErrorf(&DeeError_FSError, (DWORD)error,
		                        "Failed to change attributes of %r",
		                        path);
	}
err:
	return -1;
}




typedef struct {
	OBJECT_HEAD
	DREF DeeStringObject *d_path; /* [1..1] The path describing this directory. */
} Dir;

typedef struct {
	OBJECT_HEAD
	DREF Dir        *d_dir;  /* [1..1][const] The associated directory. */
	HANDLE           d_hnd;  /* [0..1|NULL(INVALID_HANDLE_VALUE)][lock(d_lock)]
	                          * The iteration handle or INVALID_HANDLE_VALUE when exhausted. */
	WIN32_FIND_DATAW d_data; /* [lock(d_lock)] The file data for the next matching entry. */
#ifndef CONFIG_NO_THREADS
	rwlock_t         d_lock;
#endif /* !CONFIG_NO_THREADS */
} DirIterator;

PRIVATE NONNULL((1)) void DCALL
diriter_fini(DirIterator *__restrict self) {
	if (self->d_hnd != INVALID_HANDLE_VALUE) {
		DBG_ALIGNMENT_DISABLE();
		FindClose(self->d_hnd);
		DBG_ALIGNMENT_ENABLE();
	}
	Dee_Decref(self->d_dir);
}

PRIVATE NONNULL((1, 2)) void DCALL
diriter_visit(DirIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->d_dir);
}

#if 0 /* Find-handles aren't real handles! */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
diriter_copy(DirIterator *__restrict self,
             DirIterator *__restrict other) {
	HANDLE prochnd = GetCurrentProcess();
	rwlock_read(&other->d_lock);
	if (other->d_hnd == INVALID_HANDLE_VALUE) {
		/* The other directory has been exhausted. */
		self->d_hnd = INVALID_HANDLE_VALUE;
	} else {
		DBG_ALIGNMENT_DISABLE();
		if (!DuplicateHandle(prochnd, other->d_hnd,
		                     prochnd, &self->d_hnd,
		                     0, TRUE, DUPLICATE_SAME_ACCESS)) {
			DBG_ALIGNMENT_ENABLE();
			rwlock_endread(&other->d_lock);
			return DeeNTSystem_ThrowErrorf(NULL, GetLastError(),
			                               "Failed to duplicate Find-handle");
		}
		DBG_ALIGNMENT_ENABLE();
		memcpy(&self->d_data, &other->d_data, sizeof(WIN32_FIND_DATAW));
	}
	self->d_dir = other->d_dir;
	Dee_Incref(self->d_dir);
	rwlock_endread(&other->d_lock);
	return 0;
}
#endif

PRIVATE ATTR_COLD int DCALL
err_handle_findnextfile(DWORD dwError, DeeObject *__restrict path) {
	return DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
	                               "Failed to read entires from directory %r",
	                               path);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
diriter_next(DirIterator *__restrict self) {
	WCHAR *result_string, *begin;
	size_t length;
	DWORD dwError;
again:
	if (DeeThread_CheckInterrupt())
		goto err;
	rwlock_write(&self->d_lock);
	/* Quick check: Has the iterator been exhausted. */
	if (self->d_hnd == INVALID_HANDLE_VALUE) {
		rwlock_endwrite(&self->d_lock);
iter_done:
		return (DREF DeeStringObject *)ITER_DONE;
	}
read_filename:
	begin = self->d_data.cFileName, length = 0;
	for (; length < COMPILER_LENOF(self->d_data.cFileName) && begin[length]; ++length)
		;
	if (length <= 2 && begin[0] == '.' &&
	    (length == 1 || begin[1] == '.')) {
		/* Skip self/parent directories. */
		DBG_ALIGNMENT_DISABLE();
		if (!FindNextFileW(self->d_hnd, &self->d_data)) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (dwError == ERROR_NO_MORE_FILES) {
				HANDLE hnd  = self->d_hnd;
				self->d_hnd = INVALID_HANDLE_VALUE;
				rwlock_endwrite(&self->d_lock);
				DBG_ALIGNMENT_DISABLE();
				FindClose(hnd);
				DBG_ALIGNMENT_ENABLE();
				goto iter_done;
			}
			rwlock_endwrite(&self->d_lock);
			if (DeeNTSystem_IsBadAllocError(dwError)) {
				if (Dee_CollectMemory(1))
					goto again;
				goto err;
			}
			goto err_read_failed;
		}
		DBG_ALIGNMENT_ENABLE();
		goto read_filename;
	}
	result_string = (WCHAR *)Dee_TryMalloc(sizeof(size_t) + 4 + length * 2);
	if unlikely(!result_string) {
		rwlock_endwrite(&self->d_lock);
		if (Dee_CollectMemory(sizeof(size_t) + 4 + length * 2))
			goto again;
		goto err;
	}
	/* Create an encoding string. */
	*(*(size_t **)&result_string)++ = length;
	memcpyw(result_string, self->d_data.cFileName, length);
	result_string[length] = 0;
	/* Advance the directory by one. */
	DBG_ALIGNMENT_DISABLE();
	if (!FindNextFileW(self->d_hnd, &self->d_data)) {
		HANDLE hnd;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if unlikely(dwError != ERROR_NO_MORE_FILES) {
			rwlock_endwrite(&self->d_lock);
			if (DeeNTSystem_IsBadAllocError(dwError)) {
				if (Dee_CollectMemory(1))
					goto again;
				goto err;
			}
			goto err_read_failed;
		}
		hnd         = self->d_hnd;
		self->d_hnd = INVALID_HANDLE_VALUE;
		rwlock_endwrite(&self->d_lock);
		DBG_ALIGNMENT_DISABLE();
		FindClose(hnd);
		DBG_ALIGNMENT_ENABLE();
	} else {
		DBG_ALIGNMENT_ENABLE();
		rwlock_endwrite(&self->d_lock);
	}
	/* Manually construct a string object and fill
	 * it with data read from the directory entry. */
	return (DREF DeeStringObject *)DeeString_PackWideBuffer(result_string, STRING_ERROR_FREPLAC);
err_read_failed:
	err_handle_findnextfile(dwError, (DeeObject *)self->d_dir->d_path);
err:
	return NULL;
}

PRIVATE ATTR_COLD int DCALL
err_handle_opendir(DWORD error, DeeObject *__restrict path) {
	if (DeeNTSystem_IsFileNotFoundError(error))
		return err_path_not_found(error, path);
	if (DeeNTSystem_IsNotDir(error))
		return err_path_no_dir(error, path);
	if (DeeNTSystem_IsAccessDeniedError(error))
		return err_path_no_access(error, path);
	return DeeNTSystem_ThrowErrorf(&DeeError_FSError, error,
	                               "Failed to open directory %r",
	                               path);
}


PRIVATE WUNUSED NONNULL((1)) DREF DirIterator *DCALL
dir_iter(Dir *__restrict self) {
	DREF DirIterator *result;
	LPWSTR wname, wpattern;
	size_t i, wname_length;
	if (DeeThread_CheckInterrupt())
		goto err;
	result = DeeObject_MALLOC(DirIterator);
	if unlikely(!result)
		goto err;
	wname = (LPWSTR)DeeString_AsWide((DeeObject *)self->d_path);
	if unlikely(!wname)
		goto err_r;
	/* Append the `\\*' to the given path and fix forward-slashes. */
	wname_length = WSTR_LENGTH(wname);
	wpattern     = (LPWSTR)Dee_AMalloc(8 + wname_length * 2);
	if unlikely(!wpattern)
		goto err_r;
	for (i = 0; i < wname_length; ++i) {
		WCHAR ch = wname[i];
		/* FindFirstFile() actually fails when handed forward-slashes.
		 * That's something I didn't notice in the old deemon, which
		 * caused fs.dir() to (seemingly) fail at random. */
		if (ch == '/')
			ch = '\\';
		wpattern[i] = ch;
	}
	/* Use the current directory if the given name is empty. */
	if (!wname_length)
		wpattern[wname_length++] = '.';
	/* Append a trailing backslash if there isn't one already. */
	if (wpattern[wname_length - 1] != '\\')
		wpattern[wname_length++] = '\\';
	/* Append a match-all wildcard. */
	wpattern[wname_length++] = '*';
	wpattern[wname_length]   = 0;
	DBG_ALIGNMENT_DISABLE();
	result->d_hnd = FindFirstFileExW(wpattern, FindExInfoBasic, &result->d_data,
	                                 FindExSearchNameMatch, NULL, 0);
	DBG_ALIGNMENT_ENABLE();
	Dee_AFree(wpattern);
	if unlikely(result->d_hnd == INVALID_HANDLE_VALUE) {
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (dwError != ERROR_NO_MORE_FILES) {
			err_handle_opendir(dwError, (DeeObject *)self->d_path);
			goto err_r;
		}
		/* Empty directory? ok... */
	}
	/* Finish initializing misc. members. */
	Dee_Incref(self);
	result->d_dir = self;
	rwlock_init(&result->d_lock);
	DeeObject_Init(result, &DeeDirIterator_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE struct type_member diriter_members[] = {
	TYPE_MEMBER_FIELD("seq", STRUCT_OBJECT, offsetof(DirIterator, d_dir)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeDirIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_DirIterator_tp_name,
	/* .tp_doc      = */ S_DirIterator_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL, /* (void *)&diriter_copy, */
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DirIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&diriter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&diriter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&diriter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ diriter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE NONNULL((1)) void DCALL
dir_fini(Dir *__restrict self) {
	Dee_Decref(self->d_path);
}

PRIVATE NONNULL((1, 2)) void DCALL
dir_visit(Dir *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->d_path);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dir_copy(Dir *__restrict self,
         Dir *__restrict other) {
	self->d_path = other->d_path;
	Dee_Incref(self->d_path);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dir_ctor(Dir *__restrict self,
         size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:" S_Dir_tp_name, &self->d_path))
		goto err;
	if (DeeString_Check(self->d_path)) {
		Dee_Incref(self->d_path);
	} else if (DeeInt_Check(self->d_path)) {
		HANDLE fd; /* Support for descriptor-based dir() */
		if (DeeObject_AsUIntptr((DeeObject *)self->d_path, (uintptr_t *)&fd))
			goto err;
		self->d_path = (DREF DeeStringObject *)DeeNTSystem_GetFilenameOfHandle(fd);
		if unlikely(!self->d_path)
			goto err;
	} else {
		self->d_path = (DREF DeeStringObject *)DeeFile_Filename((DeeObject *)self->d_path);
		if unlikely(!self->d_path)
			goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE struct type_seq dir_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dir_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_range_del = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_range_set = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))NULL
};

PRIVATE struct type_member dir_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeDirIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeDir_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_Dir_tp_name,
	/* .tp_doc      = */ S_Dir_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ (void *)&dir_copy,
				/* .tp_deep_ctor = */ (void *)&dir_copy,
				/* .tp_any_ctor  = */ (void *)&dir_ctor,
				TYPE_FIXED_ALLOCATOR(Dir)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dir_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dir_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dir_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dir_class_members
};


/* To a wildcard-enabled string comparison on `string' using `pattern'
 * Taken from one of my other projects: `KOS' - `/libs/libc/string.c:libc_wildstrcmp' */
PRIVATE int DCALL
wild_match(LPWSTR string, LPWSTR pattern) {
	WCHAR card_post;
	for (;;) {
		if (!*string) {
			/* End of string (if the patter is empty, or only contains '*', we have a match) */
			while (*pattern == '*')
				++pattern;
			return -(int)*pattern;
		}
		if (!*pattern)
			return (int)*string; /* Pattern end doesn't match */
		if (*pattern == '*') {
			/* Skip starts */
			do {
				++pattern;
			} while (*pattern == '*');
			if ((card_post = *pattern++) == '\0')
				return 0; /* Pattern ends with '*' (matches everything) */
			if (card_post == '?')
				goto next; /* Match any --> already found */
			card_post = (WCHAR)DeeUni_ToLower(card_post);
			for (;;) {
				WCHAR ch = *string++;
				if ((WCHAR)DeeUni_ToLower(ch) == card_post) {
					/* Recursively check if the rest of the string and pattern match */
					if (!wild_match(string, pattern))
						return 0;
				} else if (!ch) {
					return -(int)card_post; /* Wildcard suffix not found */
				}
			}
		}
		if (DeeUni_ToLower(*pattern) == DeeUni_ToLower(*string) ||
		    *pattern == '?') {
next:
			++string, ++pattern;
			continue; /* single character match */
		}
		break; /* mismatch */
	}
	return *string - *pattern;
}


/* query() types. */
typedef struct {
	DirIterator q_iter; /* The underlying iterator. */
	LPWSTR      q_wild; /* The wildcard pattern string with which to match filenames. */
} QueryIterator;

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
queryiter_next(QueryIterator *__restrict self) {
	WCHAR *result_string, *begin;
	size_t length;
	DWORD dwError;
again:
	if (DeeThread_CheckInterrupt())
		goto err;
	rwlock_write(&self->q_iter.d_lock);
	/* Quick check: Has the iterator been exhausted. */
	if (self->q_iter.d_hnd == INVALID_HANDLE_VALUE) {
		rwlock_endwrite(&self->q_iter.d_lock);
iter_done:
		return (DREF DeeStringObject *)ITER_DONE;
	}
read_filename:
	begin = self->q_iter.d_data.cFileName, length = 0;
	for (; length < COMPILER_LENOF(self->q_iter.d_data.cFileName) - 1 &&
	       begin[length];
	     ++length)
		;
	(COMPILER_ENDOF(self->q_iter.d_data.cFileName)[-1]) = 0; /* Ensure validity. */
	if ((length <= 2 && begin[0] == '.' &&
	     (length == 1 || begin[1] == '.')) ||
	    wild_match(begin, self->q_wild) != 0) {
		/* Skip self/parent directories. */
		DBG_ALIGNMENT_DISABLE();
		if (!FindNextFileW(self->q_iter.d_hnd, &self->q_iter.d_data)) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (dwError == ERROR_NO_MORE_FILES) {
				HANDLE hnd         = self->q_iter.d_hnd;
				self->q_iter.d_hnd = INVALID_HANDLE_VALUE;
				rwlock_endwrite(&self->q_iter.d_lock);
				FindClose(hnd);
				goto iter_done;
			}
			rwlock_endwrite(&self->q_iter.d_lock);
			if (DeeNTSystem_IsBadAllocError(dwError)) {
				if (Dee_CollectMemory(1))
					goto again;
				goto err;
			}
			goto err_read_failed;
		}
		DBG_ALIGNMENT_ENABLE();
		goto read_filename;
	}
	result_string = (WCHAR *)Dee_TryMalloc(sizeof(size_t) + 4 + length * 2);
	if unlikely(!result_string) {
		rwlock_endwrite(&self->q_iter.d_lock);
		if (Dee_CollectMemory(sizeof(size_t) + 4 + length * 2))
			goto again;
		goto err;
	}
	/* Create an encoding string. */
	*(*(size_t **)&result_string)++ = length;
	memcpyw(result_string, self->q_iter.d_data.cFileName, length);
	result_string[length] = 0;
	/* Advance the directory by one. */
	DBG_ALIGNMENT_DISABLE();
	if (!FindNextFileW(self->q_iter.d_hnd, &self->q_iter.d_data)) {
		HANDLE hnd;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if unlikely(dwError != ERROR_NO_MORE_FILES) {
			rwlock_endwrite(&self->q_iter.d_lock);
			if (DeeNTSystem_IsBadAllocError(dwError)) {
				if (Dee_CollectMemory(1))
					goto again;
				goto err;
			}
			goto err_read_failed;
		}
		hnd                = self->q_iter.d_hnd;
		self->q_iter.d_hnd = INVALID_HANDLE_VALUE;
		rwlock_endwrite(&self->q_iter.d_lock);
		DBG_ALIGNMENT_DISABLE();
		FindClose(hnd);
		DBG_ALIGNMENT_ENABLE();
	} else {
		DBG_ALIGNMENT_ENABLE();
		rwlock_endwrite(&self->q_iter.d_lock);
	}
	/* Manually construct a string object and fill
	 * it with data read from the directory entry. */
	return (DREF DeeStringObject *)DeeString_PackWideBuffer(result_string, STRING_ERROR_FREPLAC);
err_read_failed:
	err_handle_findnextfile(dwError, (DeeObject *)self->q_iter.d_dir->d_path);
err:
	return NULL;
}


INTERN DeeTypeObject DeeQueryIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_QueryIterator_tp_name,
	/* .tp_doc      = */ S_QueryIterator_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeDirIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL, /* (void *)&diriter_copy, */
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(QueryIterator)
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
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&queryiter_next,
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

PRIVATE struct type_member query_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeQueryIterator_Type),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) DREF QueryIterator *DCALL
query_iter(Dir *__restrict self) {
	DREF QueryIterator *result;
	LPWSTR wname, wpattern;
	size_t i, wname_length;
	if (DeeThread_CheckInterrupt())
		goto err;
	result = DeeObject_MALLOC(QueryIterator);
	if unlikely(!result)
		goto err;
	wname = (LPWSTR)DeeString_AsWide((DeeObject *)self->d_path);
	if unlikely(!wname)
		goto err_r;
again_wname:
	/* Append the `\\*' to the given path and fix forward-slashes. */
	wname_length = WSTR_LENGTH(wname);
	/* Locate the previous directory. */
	while (wname_length &&
	       (wname[wname_length - 1] != '/' &&
	        wname[wname_length - 1] != '\\'))
		--wname_length;
	/* Set up the wildcard patter string pointer
	 * of the resulting query iterator. */
	result->q_wild = wname + wname_length;
	wpattern       = (LPWSTR)Dee_AMalloc(8 + wname_length * 2);
	if unlikely(!wpattern)
		goto err_r;
	for (i = 0; i < wname_length; ++i) {
		WCHAR ch = wname[i];
		/* FindFirstFile() actually fails when handed forward-slashes.
		 * That's something I didn't notice in the old deemon, which
		 * caused fs.dir() to (seemingly) fail at random. */
		if (ch == '/')
			ch = '\\';
		wpattern[i] = ch;
	}
	/* Use the current directory if the given name is empty. */
	if (!wname_length)
		wpattern[wname_length++] = '.';
	/* Append a trailing backslash. */
	wpattern[wname_length++] = '\\';
	/* Append a match-all wildcard. */
	wpattern[wname_length++] = '*';
	wpattern[wname_length]   = 0;
	DBG_ALIGNMENT_DISABLE();
	result->q_iter.d_hnd = FindFirstFileExW(wpattern, FindExInfoBasic,
	                                        &result->q_iter.d_data,
	                                        FindExSearchNameMatch, NULL, 0);
	DBG_ALIGNMENT_ENABLE();
	Dee_AFree(wpattern);
	if unlikely(result->q_iter.d_hnd == INVALID_HANDLE_VALUE) {
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (dwError != ERROR_NO_MORE_FILES) {
			DREF DeeObject *query_path;
			if (DeeNTSystem_IsBadAllocError(dwError)) {
				if (Dee_CollectMemory(1))
					goto again_wname;
				goto err;
			}
			query_path = DeeString_NewWide(wname, wname_length,
			                               STRING_ERROR_FIGNORE);
			if unlikely(!query_path)
				goto err_r;
			err_handle_opendir(dwError, query_path);
			Dee_Decref(query_path);
			goto err_r;
		}
		/* Empty directory? ok... */
	}
	/* Finish initializing misc. members. */
	Dee_Incref(self);
	result->q_iter.d_dir = self;
	rwlock_init(&result->q_iter.d_lock);
	DeeObject_Init(&result->q_iter, &DeeQueryIterator_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE struct type_seq query_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&query_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_range_del = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_range_set = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))NULL
};

INTERN DeeTypeObject DeeQuery_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_Query_tp_name,
	/* .tp_doc      = */ S_Query_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL | TP_FINHERITCTOR,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeDir_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(Dir)
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
	/* .tp_seq           = */ &query_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ query_class_members
};


DECL_END

#ifndef __INTELLISENSE__
#include "nt.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEX_FS_WINDOWS_C_INL */
