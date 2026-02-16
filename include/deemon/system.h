/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
/*!export **/
/*!export -CONFIG_HAVE_**/
/*!export DeeNTSystem_**/
/*!export DeeSystem_**/
/*!export DeeUnixSystem_**/
#ifndef GUARD_DEEMON_SYSTEM_H
#define GUARD_DEEMON_SYSTEM_H 1 /*!export-*/

#include "api.h"

#include <hybrid/typecore.h> /* __BYTE_TYPE__, __ULONG32_TYPE__ */

#include "types.h" /* DREF, DeeObject, DeeTypeObject, Dee_formatprinter_t, Dee_ssize_t */

#include <stdarg.h>  /* va_list */
#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uint64_t, uintptr_t */

/* We only need "memrend" if it is available, so we don't need "system-features.h" */
/*!fixincludes fake_include "system-features.h" // memrend */

#ifdef CONFIG_NO_STRING_H
#undef CONFIG_HAVE_STRING_H
#elif !defined(CONFIG_HAVE_STRING_H) && \
      (defined(__NO_has_include) || __has_include(<string.h>))
#define CONFIG_HAVE_STRING_H
#endif

#ifdef CONFIG_HAVE_STRING_H
#include <string.h>
#endif /* CONFIG_HAVE_STRING_H */

DECL_BEGIN

struct Dee_unicode_printer;

#ifdef CONFIG_HOST_WINDOWS
/* Recognize `STDIN$', `STDOUT$' and `STDERR$' special file names, to
 * go alongside filenames such as `CON', `NUL', `CONIN$', `CONOUT$' */
#undef CONFIG_WANT_WINDOWS_STD_FILES
#ifndef CONFIG_NO_WANT_WINDOWS_STD_FILES
#define CONFIG_WANT_WINDOWS_STD_FILES
#endif /* !CONFIG_NO_WANT_WINDOWS_STD_FILES */
#endif /* CONFIG_HOST_WINDOWS */

#ifdef CONFIG_HOST_WINDOWS
#define DeeSystem_HAVE_FS_ICASE
#define DeeSystem_HAVE_FS_DRIVES
#define DeeSystem_DELIM        ';'
#define DeeSystem_DELIM_S      ";"
#define DeeSystem_SEP          '\\'
#define DeeSystem_SEP_S        "\\"
#define DeeSystem_ALTSEP       '/'
#define DeeSystem_ALTSEP_S     "/"
#define DeeSystem_IsSep(x)     ((x) == '\\' || (x) == '/')
#define DeeSystem_IsAbs(x)     ((x)[0] && ((x)[1] == ':' || ((x)[0] == '\\' && (x)[1] == '\\')))
#define DeeSystem_IsAbsN(x, n) ((n) >= 2 && ((x)[1] == ':' || ((x)[0] == '\\' && (x)[1] == '\\')))
#undef DeeSystem_HAVE_FS_ISABS_CHECKS_LEADING_SLASHES
#elif defined(__CYGWIN__) || defined(__CYGWIN32__)
/* Cygwin paths also accept r'\' as alias for r'/' */
#undef DeeSystem_HAVE_FS_ICASE
#define DeeSystem_DELIM        ':'
#define DeeSystem_DELIM_S      ":"
#define DeeSystem_SEP          '/'
#define DeeSystem_SEP_S        "/"
#define DeeSystem_ALTSEP       '\\'
#define DeeSystem_ALTSEP_S     "\\"
#define DeeSystem_IsSep(x)     ((x) == '/' || (x) == '\\')
#define DeeSystem_IsAbs(x)     ((x)[0] == '/') /* Absolute paths still require a leading '/'! */
#define DeeSystem_IsAbsN(x, n) ((n) >= 1 && (x)[0] == '/')
#define DeeSystem_HAVE_FS_ISABS_CHECKS_LEADING_SLASHES
#else /* ... */
#undef DeeSystem_HAVE_FS_ICASE
#define DeeSystem_DELIM        ':'
#define DeeSystem_DELIM_S      ":"
#define DeeSystem_SEP          '/'
#define DeeSystem_SEP_S        "/"
#define DeeSystem_IsSep(x)     ((x) == '/')
#define DeeSystem_IsAbs(x)     ((x)[0] == '/')
#define DeeSystem_IsAbsN(x, n) ((n) >= 1 && (x)[0] == '/')
#define DeeSystem_HAVE_FS_ISABS_CHECKS_LEADING_SLASHES
#endif /* !... */

/* Returns the filename-portion of a given `path'
 * >> print DeeSystem_BaseName(r"/foo/bar/file.txt");   // "file.txt"
 * >> print DeeSystem_BaseName(r"file.txt");            // "file.txt"
 * >> print DeeSystem_BaseName(r"E:\path\to\file.txt"); // "file.txt"  (Windows-only)
 * 
 * This function returns a pointer to 1 character past the
 * last `DeeSystem_SEP' or `DeeSystem_ALTSEP' in `path', or
 * just re-returns `path' when no such character exists. */
DFUNDEF ATTR_PURE ATTR_RETNONNULL WUNUSED ATTR_INS(1, 2) char const *
(DFCALL DeeSystem_BaseName)(char const *__restrict path, size_t pathlen);

#ifdef CONFIG_NO_memrend
#undef CONFIG_HAVE_memrend
#elif !defined(CONFIG_HAVE_memrend) && \
      (defined(memrend) || defined(__memrend_defined) || defined(__USE_KOS))
#define CONFIG_HAVE_memrend
#endif

#if !defined(DeeSystem_ALTSEP) && !defined(__OPTIMIZE_SIZE__) && defined(CONFIG_HAVE_memrend)
#define DeeSystem_BaseName(path, pathlen) \
	((char const *)((__BYTE_TYPE__ *)memrend(path, DeeSystem_SEP, (pathlen) * sizeof(char)) + 1))
#endif /* !DeeSystem_ALTSEP && !__OPTIMIZE_SIZE__ && CONFIG_HAVE_memrend */



/* Print the current working directory to the given `printer'
 * @param: include_trailing_sep: A trailing / or \\-character is also printed.
 * @return:  0: Success.
 * @return: -1: An error occurred (s.a. `DeeError_*'). */
DFUNDEF WUNUSED NONNULL((1)) int
(DCALL DeeSystem_PrintPwd)(struct Dee_unicode_printer *__restrict printer,
                           bool include_trailing_sep);

/* Ensure that the given `filename' describes an absolute, normalized path. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeSystem_MakeNormalAndAbsolute(/*String*/ DeeObject *__restrict filename);

/* Check if the given `filename' can be considered an absolute, normalized
 * path, that is: isn't relative, and doesn't contain `.' or `..' segments. */
DFUNDEF WUNUSED NONNULL((1)) bool DCALL
DeeSystem_IsNormalAndAbsolute(/*utf-8*/ char const *filename, size_t filename_len);

/* Return the current UTC realtime in microseconds since 01-01-1970T00:00:00+00:00 */
DFUNDEF WUNUSED uint64_t DCALL DeeSystem_GetWalltime(void);

/* Return the last modified timestamp of `filename'
 * > uses the same format as `DeeSystem_GetWalltime()'
 * @return: (uint64_t)-1: An error was thrown
 * @return: 0 : Failed to query file timestamp (probably ENOENT) */
DFUNDEF WUNUSED NONNULL((1)) uint64_t DCALL
DeeSystem_GetLastModified(/*String*/ DeeObject *__restrict filename);
DFUNDEF WUNUSED NONNULL((1)) uint64_t DCALL
DeeSystem_GetLastModifiedString(/*utf-8*/ char const *__restrict filename);

/* Check if the named file (or directory) exists in some way, shape, or form,
 * and return the type of that file. If `filename' refers to a symbolic link,
 * that link is dereferenced, and information about its target is returned.
 *
 * @return: DeeSystem_GetFileType_T_NONE: `filename' does not exist
 * @return: DeeSystem_GetFileType_T_DIR:  `filename' is a directory
 * @return: DeeSystem_GetFileType_T_REG:  `filename' is a regular file
 * @return: DeeSystem_GetFileType_ERR:    An error was thrown */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeSystem_GetFileType(/*String*/ DeeObject *__restrict filename);
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeSystem_GetFileTypeString(/*utf-8*/ char const *__restrict filename);
#define DeeSystem_GetFileType_T_NONE 0 /* File does not exist */
#define DeeSystem_GetFileType_T_DIR  1 /* File is a directory */
#define DeeSystem_GetFileType_T_REG  2 /* File is a regular file, a device, or something else (but not a symlink; those are dereferenced) */
#define DeeSystem_GetFileType_ERR (-1) /* Error was thrown */

/* Try to unlink() the given `filename'
 * WARNING: When `filename' is an empty directory, it is system-specific if
 *          that directory will be removed or not (basically, this function
 *          may be implemented using the STD-C `remove()' function)
 * NOTE:    Even upon error, there exists a chance that the file was deleted.
 * @return: 1 : The unlink() operation failed (only returned when `throw_exception_on_error' is `false')
 * @return: 0 : The unlink() operation was successful
 * @return: -1: An error occurred (may still be returned, even when `throw_exception_on_error' is `false') */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeSystem_Unlink(/*String*/ DeeObject *__restrict filename,
                 bool throw_exception_on_error);
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeSystem_UnlinkString(/*utf-8*/ char const *__restrict filename,
                       bool throw_exception_on_error);

#ifdef CONFIG_HOST_WINDOWS
typedef __ULONG32_TYPE__ DeeNT_DWORD;

/* Retrieve the Windows handle associated with a given object.
 * The translation is done by performing the following:
 * >> #ifdef Dee_fd_t_IS_HANDLE
 * >> if (DeeFile_Check(ob))
 * >>     return DeeFile_GetSysFD(ob);
 * >> #endif
 * >> #ifdef Dee_fd_t_IS_int
 * >> if (DeeFile_Check(ob))
 * >>     return get_osfhandle(DeeFile_GetSysFD(ob));
 * >> #endif
 * >> if (DeeNone_Check(ob))
 * >>     return (void *)(HANDLE)NULL;
 * >> if (DeeInt_Check(ob))
 * >>     return get_osfhandle(DeeInt_AsInt(ob));
 * >> try return DeeObject_AsInt(DeeObject_GetAttr(ob, Dee_fd_osfhandle_GETSET)); catch (AttributeError);
 * >> try return get_osfhandle(DeeObject_AsInt(DeeObject_GetAttr(ob, Dee_fd_fileno_GETSET))); catch (AttributeError);
 * >> return get_osfhandle(DeeObject_AsInt(ob));
 * Note that both msvc, as well as cygwin define `get_osfhandle()' as one
 * of the available functions, meaning that in both scenarios we are able
 * to get access to the underlying HANDLE. However, should deemon ever be
 * linked against a windows libc without this function, then only the
 * `Dee_fd_osfhandle_GETSET' variant will be usable.
 * @return: * :                   Success (the actual handle value)
 * @return: INVALID_HANDLE_VALUE: Error (handle translation failed)
 *                                In case the actual handle value stored inside
 *                                of `ob' was `INVALID_HANDLE_VALUE', then an
 *                                `DeeError_FileClosed' error is thrown. */
DFUNDEF WUNUSED NONNULL((1)) /*HANDLE*/ void *DCALL
DeeNTSystem_GetHandle(DeeObject *__restrict ob);

/* Same as `DeeNTSystem_GetHandleEx()', but also writes to `p_fd' (when non-NULL):
 * - `-1': If `get_osfhandle()' wasn't used
 * - `*':  The file descriptor number passed to `get_osfhandle()' */
DFUNDEF WUNUSED NONNULL((1)) /*HANDLE*/ void *DCALL
DeeNTSystem_GetHandleEx(DeeObject *__restrict ob, int *p_fd);

/* Similar to `DeeNTSystem_GetHandle()', but allow `ob' to refer to INVALID_HANDLE_VALUE,
 * instead of unconditionally throwing an `DeeError_FileClosed' error when such a handle
 * value is encountered.
 * @return: 0:  Success (the handle value was stored in `*pHandle', and is allowed to be `INVALID_HANDLE_VALUE')
 * @return: -1: Error (a deemon error was thrown; s.a. `DeeError_Throw()') */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeNTSystem_TryGetHandle(DeeObject *__restrict ob,
                         /*PHANDLE*/ void **pHandle);

/* Fix the given filename and extend it to an absolute UNC path. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeNTSystem_FixUncPath(/*String*/ DeeObject *__restrict filename);

/* Check if a given error code indicates a UNC-path problem that should be
 * addressed by fixing the path using `DeeNTSystem_FixUncPath()', then trying again. */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsUncError(/*DWORD*/ DeeNT_DWORD dwError);

/* Check for a number of different NT error classes. */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsFileNotFoundError(/*DWORD*/ DeeNT_DWORD dwError); /* ENOENT */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsAccessDeniedError(/*DWORD*/ DeeNT_DWORD dwError); /* EACCES */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsBadAllocError(/*DWORD*/ DeeNT_DWORD dwError);     /* ENOMEM */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsBusy(/*DWORD*/ DeeNT_DWORD dwError);              /* EBUSY */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsExists(/*DWORD*/ DeeNT_DWORD dwError);            /* EEXIST */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsNotDir(/*DWORD*/ DeeNT_DWORD dwError);            /* ENOTDIR */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsNotEmpty(/*DWORD*/ DeeNT_DWORD dwError);          /* ENOTEMPTY */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsBadF(/*DWORD*/ DeeNT_DWORD dwError);              /* EBADF */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsXDev(/*DWORD*/ DeeNT_DWORD dwError);              /* EXDEV */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsUnsupportedError(/*DWORD*/ DeeNT_DWORD dwError);  /* ENOTSUP */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsIntr(/*DWORD*/ DeeNT_DWORD dwError);              /* EINTR */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsBufferTooSmall(/*DWORD*/ DeeNT_DWORD dwError);    /* ERANGE */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsInvalidArgument(/*DWORD*/ DeeNT_DWORD dwError);   /* EINVAL */
DFUNDEF ATTR_CONST WUNUSED bool DCALL DeeNTSystem_IsNoLink(/*DWORD*/ DeeNT_DWORD dwError);            /* EINVAL (for readlink(2): not a symbolic link) */

/* Try to handle generic errors:
 * - DeeNTSystem_IsBadAllocError -- Dee_ReleaseSystemMemory()
 * - DeeNTSystem_IsIntr          -- DeeThread_CheckInterrupt()
 * @return: _DeeNTSystem_HandleGenericError_AGAIN: System error was handled; try again
 * @return: _DeeNTSystem_HandleGenericError_OTHER: System error could not be handled.
 * @return: _DeeNTSystem_HandleGenericError_ERR:   A deemon error was thrown. */
DFUNDEF WUNUSED unsigned int DCALL _DeeNTSystem_HandleGenericError(/*DWORD*/ DeeNT_DWORD dwError);
#define _DeeNTSystem_HandleGenericError_OTHER 0
#define _DeeNTSystem_HandleGenericError_AGAIN 1
#define _DeeNTSystem_HandleGenericError_ERR   2
#define DeeNTSystem_HandleGenericError(dwError, err, again)     \
	do {                                                        \
		switch (_DeeNTSystem_HandleGenericError(dwError)) {     \
		case _DeeNTSystem_HandleGenericError_OTHER: break;      \
		case _DeeNTSystem_HandleGenericError_AGAIN: goto again; \
		case _DeeNTSystem_HandleGenericError_ERR: goto err;     \
		default: __builtin_unreachable();                       \
		}                                                       \
	}	__WHILE0

/* Translate a given `dwError' into the appropriate `errno' error code.
 * If the translation failed, return a fallback value.
 * Note that (if possible), the implementation of this function is handled by the
 * linked C library, using MSVC's `_dosmaperr()' (if available). Otherwise, the
 * translation is performed identical to what is known to be done by the linked
 * C library, or a combination of CYGWIN and MSVC if some other libc is hosting
 * deemon within a windows environment.
 * NOTE: This function is also used by `DeeNTSystem_ThrowErrorf()' to translate
 *       the given NT error code into an errno. */
DFUNDEF ATTR_CONST WUNUSED /*errno_t*/ int DCALL
DeeNTSystem_TranslateErrno(/*DWORD*/ DeeNT_DWORD dwError);

/* Do the reverse of `DeeNTSystem_TranslateErrno()' */
DFUNDEF ATTR_CONST WUNUSED /*DWORD*/ DeeNT_DWORD DCALL
DeeNTSystem_TranslateNtError(/*errno_t*/ int errno_value);

/* Work around a problem with long path names. (Note: also handles interrupts)
 * @return: * :                   The new handle.
 * @return: NULL:                 A deemon callback failed and an error was thrown.
 * @return: INVALID_HANDLE_VALUE: The system call failed (s.a. `GetLastError()') */
DFUNDEF WUNUSED NONNULL((1)) /*HANDLE*/ void *DCALL
DeeNTSystem_CreateFile(/*String*/ DeeObject *__restrict lpFileName,
                       /*DWORD*/ DeeNT_DWORD dwDesiredAccess,
                       /*DWORD*/ DeeNT_DWORD dwShareMode,
                       /*LPSECURITY_ATTRIBUTES*/ void *lpSecurityAttributes,
                       /*DWORD*/ DeeNT_DWORD dwCreationDisposition,
                       /*DWORD*/ DeeNT_DWORD dwFlagsAndAttributes,
                       /*HANDLE*/ void *hTemplateFile);

/* Same as `DeeNTSystem_CreateFile()', but try not to modify the file's last-accessed timestamp
 * @return: * :                   The new handle.
 * @return: NULL:                 A deemon callback failed and an error was thrown.
 * @return: INVALID_HANDLE_VALUE: The system call failed (s.a. `GetLastError()') */
DFUNDEF WUNUSED NONNULL((1)) /*HANDLE*/ void *DCALL
DeeNTSystem_CreateFileNoATime(/*String*/ DeeObject *__restrict lpFileName,
                              /*DWORD*/ DeeNT_DWORD dwDesiredAccess,
                              /*DWORD*/ DeeNT_DWORD dwShareMode,
                              /*LPSECURITY_ATTRIBUTES*/ void *lpSecurityAttributes,
                              /*DWORD*/ DeeNT_DWORD dwCreationDisposition,
                              /*DWORD*/ DeeNT_DWORD dwFlagsAndAttributes,
                              /*HANDLE*/ void *hTemplateFile);

/* Wrapper around `DeeNTSystem_CreateFile()' and `DeeNTSystem_CreateFileNoATime()'
 * that is used to implement `posix.open()' and `File.open()' by taking unix-like
 * oflags and mode.
 * @param: oflags: Set of `Dee_OPEN_F*'
 * @param: mode:   When no bits from `0444' are set, use `FILE_ATTRIBUTE_READONLY'
 * @return: * :    The new handle.
 * @return: NULL:  A deemon callback failed and an error was thrown.
 * @return: INVALID_HANDLE_VALUE: File not found (`!Dee_OPEN_FCREAT') or already
 *                                exists (`Dee_OPEN_FCREAT | Dee_OPEN_FEXCL') */
DFUNDEF WUNUSED NONNULL((1)) /*HANDLE*/ void *DCALL
DeeNTSystem_OpenFile(/*String*/ DeeObject *__restrict filename, int oflags, int mode);

/* Determine the filename from a handle, as returned by `DeeNTSystem_CreateFile()' */
DFUNDEF WUNUSED DREF /*String*/ DeeObject *DCALL
DeeNTSystem_GetFilenameOfHandle(/*HANDLE*/ void *hFile);

/* Same as `DeeNTSystem_GetFilenameOfHandle()', but return `ITER_DONE' rather than
 * throwing a SystemError when `DeeNTSystem_PrintFilenameOfHandle()' returns `1' */
DFUNDEF WUNUSED DREF /*String*/ DeeObject *DCALL
DeeNTSystem_TryGetFilenameOfHandle(/*HANDLE*/ void *hFile);

/* @return: 1:  The system call failed (s.a. `GetLastError()').
 * @return: 0:  Success.
 * @return: -1: A deemon callback failed and an error was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeNTSystem_PrintFilenameOfHandle(struct Dee_unicode_printer *__restrict printer,
                                  /*HANDLE*/ void *hFile);

/* Wrapper for the `GetFinalPathNameByHandle()' system call.
 * @return: 2:  Unsupported.
 * @return: 1:  The system call failed (s.a. `GetLastError()').
 * @return: 0:  Success.
 * @return: -1: A deemon callback failed and an error was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeNTSystem_PrintFinalPathNameByHandle(struct Dee_unicode_printer *__restrict printer,
                                       /*HANDLE*/ void *hFile,
                                       /*DWORD*/ DeeNT_DWORD dwFlags);

/* Wrapper for the `GetMappedFileName()' system call.
 * @return: 2:  Unsupported.
 * @return: 1:  The system call failed (s.a. `GetLastError()').
 * @return: 0:  Success.
 * @return: -1: A deemon callback failed and an error was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeNTSystem_PrintMappedFileName(struct Dee_unicode_printer *__restrict printer,
                                /*HANDLE*/ void *hProcess,
                                /*LPVOID*/ void *lpv);


/* Wrapper for the `FormatMessageW()' system call.
 * @return: * :        The formatted message.
 * @return: NULL:      A deemon callback failed and an error was thrown.
 * @return: ITER_DONE: The system call failed (s.a. `GetLastError()'). */
DFUNDEF WUNUSED DREF /*String*/ DeeObject *DCALL
DeeNTSystem_FormatMessage(DeeNT_DWORD dwFlags, void const *lpSource,
                          DeeNT_DWORD dwMessageId, DeeNT_DWORD dwLanguageId,
                          /* va_list * */ void *Arguments);

/* @return: 1:  The system call failed (nothing was printed; s.a. `GetLastError()')
 * @return: 0:  Successfully printed the message.
 * @return: -1: A deemon callback failed and an error was thrown. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeNTSystem_UPrintFormatMessage(struct Dee_unicode_printer *__restrict printer,
                                DeeNT_DWORD dwFlags, void const *lpSource,
                                DeeNT_DWORD dwMessageId, DeeNT_DWORD dwLanguageId,
                                /* va_list * */ void *Arguments);

/* @param: p_success: Set to `false' if the system call failed and nothing was printed (s.a. `GetLastError()')
 * @return: >= 0: Success.
 * @return: < 0:  A deemon callback failed and an error was thrown (`*p_success' is undefined). */
DFUNDEF WUNUSED NONNULL((1, 8)) Dee_ssize_t DCALL
DeeNTSystem_PrintFormatMessage(Dee_formatprinter_t printer, void *arg,
                               DeeNT_DWORD dwFlags, void const *lpSource,
                               DeeNT_DWORD dwMessageId, DeeNT_DWORD dwLanguageId,
                               /* va_list * */ void *Arguments,
                               bool *__restrict p_success);

/* Convenience wrapper around `DeeNTSystem_FormatMessage()' for getting error messages.
 * When no error message exists, return an empty string.
 * @return: * :   The error message. (or an empty string)
 * @return: NULL: A deemon callback failed and an error was thrown. */
DFUNDEF WUNUSED DREF /*String*/ DeeObject *DCALL
DeeNTSystem_FormatErrorMessage(DeeNT_DWORD dwError);


/* Throw NT system errors, given an error code as returned by `GetLastError()'
 * When no error code is given, `GetLastError()' is called internally.
 * When `tp' is `NULL', the proper error type is automatically determined using the `DeeNTSystem_Is*' functions.
 * @return: -1: These functions always return -1 */
DFUNDEF ATTR_COLD NONNULL((3)) int (DeeNTSystem_ThrowErrorf)(DeeTypeObject *tp, DeeNT_DWORD dwError, char const *__restrict format, ...);
DFUNDEF ATTR_COLD NONNULL((2)) int (DeeNTSystem_ThrowLastErrorf)(DeeTypeObject *tp, char const *__restrict format, ...);
DFUNDEF ATTR_COLD NONNULL((3)) int (DCALL DeeNTSystem_VThrowErrorf)(DeeTypeObject *tp, DeeNT_DWORD dwError, char const *__restrict format, va_list args);
DFUNDEF ATTR_COLD NONNULL((2)) int (DCALL DeeNTSystem_VThrowLastErrorf)(DeeTypeObject *tp, char const *__restrict format, va_list args);

#ifndef Dee_ASSUMED_VALUE_IS_NOOP
#define DeeNTSystem_ThrowErrorf(tp, dwError, ...)           Dee_ASSUMED_VALUE(DeeNTSystem_ThrowErrorf(tp, dwError, __VA_ARGS__), -1)
#define DeeNTSystem_ThrowLastErrorf(tp, ...)                Dee_ASSUMED_VALUE(DeeNTSystem_ThrowLastErrorf(tp, __VA_ARGS__), -1)
#define DeeNTSystem_VThrowErrorf(tp, dwError, format, args) Dee_ASSUMED_VALUE(DeeNTSystem_VThrowErrorf(tp, dwError, format, args), -1)
#define DeeNTSystem_VThrowLastErrorf(tp, format, args)      Dee_ASSUMED_VALUE(DeeNTSystem_VThrowLastErrorf(tp, format, args), -1)
#endif /* !Dee_ASSUMED_VALUE_IS_NOOP */

#endif /* CONFIG_HOST_WINDOWS */


/* Required file extension for shared libraries. */
#ifdef CONFIG_SHLIB_EXTENSION
#define DeeSystem_SOEXT CONFIG_SHLIB_EXTENSION
#elif defined(CONFIG_HOST_WINDOWS)
#define DeeSystem_SOEXT ".dll"
#elif defined(CONFIG_HOST_UNIX)
#define DeeSystem_SOEXT ".so"
#else /* ... */
#define DeeSystem_SOEXT ".so" /* ??? */
#endif /* !... */

/* Open a shared library
 * @return: * :                      A handle for the shared library.
 * @return: NULL:                    A deemon callback failed and an error was thrown.
 * @return: DeeSystem_DlOpen_FAILED: Failed to open the shared library. */
DFUNDEF WUNUSED NONNULL((1)) void *DCALL DeeSystem_DlOpen(/*String*/ DeeObject *__restrict filename);
DFUNDEF WUNUSED NONNULL((1)) void *DCALL DeeSystem_DlOpenString(/*utf-8*/ char const *filename);
#define DeeSystem_DlOpen_FAILED ((void *)(uintptr_t)-1)

/* Lookup a symbol within a given shared library
 * Returns `NULL' if the symbol could not be found */
DFUNDEF WUNUSED NONNULL((2)) void *DCALL DeeSystem_DlSym(void *handle, char const *symbol_name);

/* Try to get a human-readable description on what went wrong during a call
 * to `DeeSystem_DlOpen[String]()' that caused `DeeSystem_DlOpen_FAILED' to
 * be returned, or `DeeSystem_DlSym()' to have caused `NULL' to be returned.
 * @return: * :        The human-readable error description
 * @return: NULL:      A deemon callback failed and an error was thrown.
 * @return: ITER_DONE: No description is available. */
DFUNDEF WUNUSED DREF /*String*/ DeeObject *DCALL DeeSystem_DlError(void);

/* Close a given shared library */
DFUNDEF void DCALL DeeSystem_DlClose(void *handle);


/* Determine the filename from a file descriptor, as returned by `open()'
 * If the host doesn't support FD-based file descriptors, throw an error. */
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeSystem_GetFilenameOfFD(int fd);

/* @return: 0:  Success.
 * @return: -1: Error. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeSystem_PrintFilenameOfFD(struct Dee_unicode_printer *__restrict printer, int fd);

/* Read the contexts of the given link.
 * If the host doesn't support symbolic links, throw an error.
 * @return: 0 / * :        Success.
 * @return: -1 / NULL:     Error.
 * @return: 1 / ITER_DONE: The specified file isn't a symbolic link. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeUnixSystem_PrintLink(struct Dee_unicode_printer *__restrict printer, /*String*/ DeeObject *__restrict filename);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeUnixSystem_PrintLinkString(struct Dee_unicode_printer *__restrict printer, /*utf-8*/ char const *filename);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeUnixSystem_ReadLink(/*String*/ DeeObject *__restrict filename);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeUnixSystem_ReadLinkString(/*utf-8*/ char const *filename);

/* Retrieve the unix FD associated with a given object.
 * The translation is done by performing the following:
 * >> #ifdef Dee_fd_t_IS_int
 * >> if (DeeFile_Check(ob))
 * >>     return DeeFile_GetSysFD(ob);
 * >> #endif
 * >> if (DeeInt_Check(ob))
 * >>     return DeeInt_AsInt(ob);
 * >> try return DeeObject_AsInt(DeeObject_GetAttr(ob, Dee_fd_fileno_GETSET)); catch (AttributeError);
 * >> return DeeObject_AsInt(ob);
 * @return: * : Success (the actual handle value)
 * @return: -1: Error (handle translation failed)
 *              In case the actual handle value stored inside of `ob'
 *              was `-1', then an `DeeError_FileClosed' error is thrown. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeUnixSystem_GetFD(DeeObject *__restrict ob);

/* Try to handle generic errors:
 * - ENOMEM -- Dee_ReleaseSystemMemory()
 * - EINTR  -- DeeThread_CheckInterrupt()
 * @return: _DeeUnixSystem_HandleGenericError_AGAIN: System error was handled; try again
 * @return: _DeeUnixSystem_HandleGenericError_OTHER: System error could not be handled.
 * @return: _DeeUnixSystem_HandleGenericError_ERR:   A deemon error was thrown. */
DFUNDEF WUNUSED unsigned int DCALL _DeeUnixSystem_HandleGenericError(int errno_value);
#define _DeeUnixSystem_HandleGenericError_OTHER 0
#define _DeeUnixSystem_HandleGenericError_AGAIN 1
#define _DeeUnixSystem_HandleGenericError_ERR   2
#define DeeUnixSystem_HandleGenericError(errno_value, err, again) \
	do {                                                          \
		switch (_DeeUnixSystem_HandleGenericError(errno_value)) { \
		case _DeeUnixSystem_HandleGenericError_OTHER: break;      \
		case _DeeUnixSystem_HandleGenericError_AGAIN: goto again; \
		case _DeeUnixSystem_HandleGenericError_ERR: goto err;     \
		default: __builtin_unreachable();                         \
		}                                                         \
	}	__WHILE0

/* Throw an exception alongside an errno error-code `error'
 * When `tp' is `NULL', automatically select an appropriate
 * error type based on the value of `error' */
DFUNDEF ATTR_COLD NONNULL((3)) int
(DeeUnixSystem_ThrowErrorf)(DeeTypeObject *tp, /*errno_t*/ int errno_value,
                            char const *__restrict format, ...);
DFUNDEF ATTR_COLD NONNULL((3)) int
(DCALL DeeUnixSystem_VThrowErrorf)(DeeTypeObject *tp, /*errno_t*/ int errno_value,
                                   char const *__restrict format, va_list args);

#ifndef Dee_ASSUMED_VALUE_IS_NOOP
#define DeeUnixSystem_ThrowErrorf(tp, errno_value, ...)           Dee_ASSUMED_VALUE(DeeUnixSystem_ThrowErrorf(tp, errno_value, __VA_ARGS__), -1)
#define DeeUnixSystem_VThrowErrorf(tp, errno_value, format, args) Dee_ASSUMED_VALUE(DeeUnixSystem_VThrowErrorf(tp, errno_value, format, args), -1)
#endif /* !Dee_ASSUMED_VALUE_IS_NOOP */


/* Value set for `errno_t' when the error is not known. */
#ifndef Dee_SYSTEM_ERROR_UNKNOWN
#define Dee_SYSTEM_ERROR_UNKNOWN 0
#endif /* !Dee_SYSTEM_ERROR_UNKNOWN */


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeSystem_PrintPwd(printer, include_trailing_sep) \
	__builtin_expect(DeeSystem_PrintPwd(printer, include_trailing_sep), 0)
#define DeeNTSystem_TryGetHandle(ob, pHandle) \
	__builtin_expect(DeeNTSystem_TryGetHandle(ob, pHandle), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

/* Returns "true" if "ptr" points into statically allocated memory.
 * If "ptr" is heap-allocated, or it being statically allocated can't
 * be determined for some other reason, return "false".
 *
 * If the given "ptr" is "NULL" or otherwise not at least readable,
 * behavior is undefined (meaning this function may SEGFAULT is given
 * a faulty pointer) */
DFUNDEF ATTR_CONST WUNUSED NONNULL((1)) bool DCALL
DeeSystem_IsStaticPointer(void const *ptr);

DECL_END

#endif /* !GUARD_DEEMON_SYSTEM_H */
