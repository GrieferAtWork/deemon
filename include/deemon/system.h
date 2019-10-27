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
#ifndef GUARD_DEEMON_SYSTEM_H
#define GUARD_DEEMON_SYSTEM_H 1

#include "api.h"
#include "error.h" /* Dee_syserrno_t */

#include <stdbool.h>

#include "object.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_unicode_printer unicode_printer
#endif /* DEE_SOURCE */

struct Dee_unicode_printer;


#ifdef CONFIG_HOST_WINDOWS
#define DEE_SYSTEM_NOCASE_FS 1
#define DeeSystem_SEP      '\\'
#define DeeSystem_ALTSEP   '/'
#define DeeSystem_SEP_S    "\\"
#define DeeSystem_ALTSEP_S "/"
#define DeeSystem_IsSep(x) ((x) == '\\' || (x) == '/')
#define DeeSystem_IsAbs(x) ((x)[0] && (x)[1] == ':')
#else /* CONFIG_HOST_WINDOWS */
#undef DEE_SYSTEM_NOCASE_FS
#define DeeSystem_SEP      '/'
#define DeeSystem_SEP_S    "/"
#define DeeSystem_IsSep(x) ((x) == '/')
#define DeeSystem_IsAbs(x) ((x)[0] == '/')
#endif /* !CONFIG_HOST_WINDOWS */

/* Print the current working directory to the given `printer'
 * @param: include_trailing_sep: A trailing / or \\-character is also printed.
 * @return:  0: Success.
 * @return: -1: An error occurred (s.a. `DeeError_*'). */
DFUNDEF int DCALL
DeeSystem_PrintPwd(struct Dee_unicode_printer *__restrict printer,
                   bool include_trailing_sep);

/* Ensure that the given `filename' describes an absolute path. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeSystem_MakeAbsolute(/*String*/ DeeObject *__restrict filename);

/* Return the current UTC realtime in microseconds since 01.01.1970T00:00:00+00:00 */
DFUNDEF WUNUSED uint64_t DCALL DeeSystem_GetWalltime(void);

/* Return the last modified timestamp of `filename'
 * > uses the same format as `DeeSystem_GetWalltime()' */
DFUNDEF WUNUSED uint64_t DCALL
DeeSystem_GetLastModified(/*String*/ DeeObject *__restrict filename);

/* Try to unlink() the given `filename'
 * WARNING: When `filename' is an empty directory, it is system-specific if
 *          that directory will be removed or not (basically, this function
 *          may be implemented using the STD-C `remove()' function)
 * NOTE:    Even upon error, there exists a chance that the file was deleted.
 * @return: 1 : The unlink() operation failed (only returned when `throw_exception_on_error' is `false')
 * @return: 0 : The unlink() operation was successful
 * @return: -1: An error occurred (may still be returned, even when `throw_exception_on_error' is `false') */
DFUNDEF WUNUSED int DCALL
DeeSystem_Unlink(/*String*/ DeeObject *__restrict filename,
                 bool throw_exception_on_error);

#ifdef CONFIG_HOST_WINDOWS
typedef __ULONG32_TYPE__ DeeNT_DWORD;

/* Retrieve the Windows handle associated with a given object.
 * The translation is done by performing the following:
 * >> #ifdef DeeSysFS_IS_HANDLE
 * >> if (DeeFile_Check(ob))
 * >>     return DeeFile_GetSysFD(ob);
 * >> #endif
 * >> #ifdef DeeSysFS_IS_INT
 * >> if (DeeFile_Check(ob))
 * >>     return get_osfhandle(DeeFile_GetSysFD(ob));
 * >> #endif
 * >> if (DeeInt_Check(ob))
 * >>     return get_osfhandle(DeeInt_AsInt(ob));
 * >> try return DeeObject_AsInt(DeeObject_GetAttr(ob, DeeSysFD_HANDLE_GETSET)); catch (AttributeError);
 * >> try return get_osfhandle(DeeObject_AsInt(DeeObject_GetAttr(ob, DeeSysFD_INT_GETSET))); catch (AttributeError);
 * >> return get_osfhandle(DeeObject_AsInt(ob));
 * Note that both msvc, as well as cygwin define `get_osfhandle()' as one
 * of the available functions, meaning that in both scenarios we are able
 * to get access to the underlying HANDLE. However, should deemon ever be
 * linked against a windows libc without this function, then only the
 * `DeeSysFD_HANDLE_GETSET' variant will be usable.
 * @return: * :                   Success (the actual handle value)
 * @return: INVALID_HANDLE_VALUE: Error (handle translation failed)
 *                                In case the actual handle value stored inside
 *                                of `ob' was `INVALID_HANDLE_VALUE', then an
 *                                `DeeError_FileClosed' error is thrown. */
DFUNDEF WUNUSED /*HANDLE*/ void *DCALL
DeeNTSystem_GetHandle(DeeObject *__restrict ob);

/* Fix the given filename and extend it to an absolute UNC path. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeNTSystem_FixUncPath(/*String*/ DeeObject *__restrict filename);

/* Check if a given error code indicates a UNC-path problem that should be
 * addressed by fixing the path using `DeeNTSystem_FixUncPath()', then trying again. */
DFUNDEF WUNUSED bool DCALL DeeNTSystem_IsUncError(/*DWORD*/ DeeNT_DWORD dwError);
DFUNDEF WUNUSED bool DCALL DeeNTSystem_IsFileNotFoundError(/*DWORD*/ DeeNT_DWORD dwError);
DFUNDEF WUNUSED bool DCALL DeeNTSystem_IsAccessDeniedError(/*DWORD*/ DeeNT_DWORD dwError);
DFUNDEF WUNUSED bool DCALL DeeNTSystem_IsBadAllocError(/*DWORD*/ DeeNT_DWORD dwError);
DFUNDEF WUNUSED bool DCALL DeeNTSystem_IsBusy(/*DWORD*/ DeeNT_DWORD dwError);
DFUNDEF WUNUSED bool DCALL DeeNTSystem_IsExists(/*DWORD*/ DeeNT_DWORD dwError);
DFUNDEF WUNUSED bool DCALL DeeNTSystem_IsNotDir(/*DWORD*/ DeeNT_DWORD dwError);
DFUNDEF WUNUSED bool DCALL DeeNTSystem_IsNotEmpty(/*DWORD*/ DeeNT_DWORD dwError);
DFUNDEF WUNUSED bool DCALL DeeNTSystem_IsBadF(/*DWORD*/ DeeNT_DWORD dwError);
DFUNDEF WUNUSED bool DCALL DeeNTSystem_IsXDev(/*DWORD*/ DeeNT_DWORD dwError);
DFUNDEF WUNUSED bool DCALL DeeNTSystem_IsUnsupportedError(/*DWORD*/ DeeNT_DWORD dwError);
DFUNDEF WUNUSED bool DCALL DeeNTSystem_IsIntr(/*DWORD*/ DeeNT_DWORD dwError);
DFUNDEF WUNUSED bool DCALL DeeNTSystem_IsBufferTooSmall(/*DWORD*/ DeeNT_DWORD dwError);

/* Translate a given `dwError' into the appropriate `errno' error code.
 * If the translation failed, return a fallback value.
 * Note that (if possible), the implementation of this function is handled by the
 * linked C library, using MSVC's `_dosmaperr()' (if available). Otherwise, the
 * translation is performed identical to what is known to be done by the linked
 * C library, or a combination of CYGWIN and MSVC if some other libc is hosting
 * deemon within a windows environment.
 * NOTE: This function is also used by `DeeNTSystem_ThrowErrorf()' to translate
 *       the given NT error code into an errno. */
DFUNDEF WUNUSED Dee_syserrno_t DCALL
DeeNTSystem_TranslateErrno(/*DWORD*/ DeeNT_DWORD dwError);

/* Work around a problem with long path names.
 * @return: * :                   The new handle.
 * @return: NULL:                 A deemon callback failed and an error was thrown.
 * @return: INVALID_HANDLE_VALUE: The system call failed (See GetLastError()) */
DFUNDEF WUNUSED NONNULL((1)) /*HANDLE*/ void *DCALL
DeeNTSystem_CreateFile(/*String*/ DeeObject *__restrict lpFileName,
                       /*DWORD*/ DeeNT_DWORD dwDesiredAccess,
                       /*DWORD*/ DeeNT_DWORD dwShareMode,
                       /*LPSECURITY_ATTRIBUTES*/ void *lpSecurityAttributes,
                       /*DWORD*/ DeeNT_DWORD dwCreationDisposition,
                       /*DWORD*/ DeeNT_DWORD dwFlagsAndAttributes,
                       /*HANDLE*/ void *hTemplateFile);

/* Determine the filename from a handle, as returned by `DeeNTSystem_CreateFile()' */
DFUNDEF WUNUSED DREF /*String*/ DeeObject *DCALL
DeeNTSystem_GetFilenameOfHandle(/*HANDLE*/ void *hFile);

/* Same as `DeeNTSystem_GetFilenameOfHandle()', but return `ITER_DONE' rather than
 * throwing a SystemError when `DeeNTSystem_PrintFilenameOfHandle()' returns `1' */
DFUNDEF WUNUSED DREF /*String*/ DeeObject *DCALL
DeeNTSystem_TryGetFilenameOfHandle(/*HANDLE*/ void *hFile);

/* @return: 1:  The system call failed (See GetLastError()).
 * @return: 0:  Success.
 * @return: -1: A deemon callback failed and an error was thrown. */
DFUNDEF WUNUSED int DCALL
DeeNTSystem_PrintFilenameOfHandle(struct Dee_unicode_printer *__restrict printer,
                                  /*HANDLE*/ void *hFile);

/* Wrapper for the `GetFinalPathNameByHandle()' system call.
 * @return: 2:  Unsupported.
 * @return: 1:  The system call failed (See GetLastError()).
 * @return: 0:  Success.
 * @return: -1: A deemon callback failed and an error was thrown. */
DFUNDEF WUNUSED int DCALL
DeeNTSystem_PrintFinalPathNameByHandle(struct Dee_unicode_printer *__restrict printer,
                                       /*HANDLE*/ void *hFile,
                                       /*DWORD*/ DeeNT_DWORD dwFlags);

/* Wrapper for the `GetMappedFileName()' system call.
 * @return: 2:  Unsupported.
 * @return: 1:  The system call failed (See GetLastError()).
 * @return: 0:  Success.
 * @return: -1: A deemon callback failed and an error was thrown. */
DFUNDEF WUNUSED int DCALL
DeeNTSystem_PrintMappedFileName(struct Dee_unicode_printer *__restrict printer,
                                /*HANDLE*/ void *hProcess,
                                /*LPVOID*/ void *lpv);


/* Wrapper for the `FormatMessageW()' system call.
 * @return: * :        The formatted message.
 * @return: NULL:      A deemon callback failed and an error was thrown.
 * @return: ITER_DONE: The system call failed (See GetLastError()). */
DFUNDEF WUNUSED DREF /*String*/ DeeObject *DCALL
DeeNTSystem_FormatMessage(DeeNT_DWORD dwFlags, void const *lpSource,
                          DeeNT_DWORD dwMessageId, DeeNT_DWORD dwLanguageId,
                          /* va_list * */ void *Arguments);

/* @return: 1:  The system call failed (See GetLastError())
 * @return: 0:  Successfully printed the message.
 * @return: -1: A deemon callback failed and an error was thrown. */
DFUNDEF WUNUSED int DCALL
DeeNTSystem_PrintFormatMessage(struct Dee_unicode_printer *__restrict printer,
                               DeeNT_DWORD dwFlags, void const *lpSource,
                               DeeNT_DWORD dwMessageId, DeeNT_DWORD dwLanguageId,
                               /* va_list * */ void *Arguments);

/* Convenience wrapper around `DeeNTSystem_FormatMessage()' for getting error message.
 * When no error message exists, return an empty string.
 * @return: * :   The error message. (or an empty string)
 * @return: NULL: A deemon callback failed and an error was thrown. */
DFUNDEF WUNUSED DREF /*String*/ DeeObject *DCALL
DeeNTSystem_FormatErrorMessage(DeeNT_DWORD dwError);


/* Throw NT system errors, given an error code as returned by `GetLastError()'
 * When no error code is given, `GetLastError()' is called internally.
 * When `tp' is `NULL', the proper error type is automatically determined using the `DeeNTSystem_Is*' functions.
 * @return: -1: These functions always return -1 */
DFUNDEF NONNULL((3)) int DeeNTSystem_ThrowErrorf(DeeTypeObject *tp, DeeNT_DWORD dwError, char const *__restrict format, ...);
DFUNDEF NONNULL((2)) int DeeNTSystem_ThrowLastErrorf(DeeTypeObject *tp, char const *__restrict format, ...);
DFUNDEF NONNULL((3)) int DCALL DeeNTSystem_VThrowErrorf(DeeTypeObject *tp, DeeNT_DWORD dwError, char const *__restrict format, va_list args);
DFUNDEF NONNULL((2)) int DCALL DeeNTSystem_VThrowLastErrorf(DeeTypeObject *tp, char const *__restrict format, va_list args);

/* Throw a given `dwError' or use the return value of `GetLastError()'
 * as the proper Error type derived from SystemError.
 * TODO: Remove these!
 * @return: -1: Always returned. */
DFUNDEF int DCALL nt_ThrowLastError(void);
DFUNDEF int DCALL nt_ThrowError(__ULONG32_TYPE__ dwError);
#endif /* CONFIG_HOST_WINDOWS */


#ifdef CONFIG_BUILDING_DEEMON
/* Figure out how to implement the shared library system API */
#undef DeeSystem_DlOpen_USE_LOADLIBRARY
#undef DeeSystem_DlOpen_USE_DLFCN
#undef DeeSystem_DlOpen_USE_STUB
#if (defined(__CYGWIN__) || defined(__CYGWIN32__)) && \
    (defined(CONFIG_HAVE_dlopen) && defined(CONFIG_HAVE_dlsym))
#define DeeSystem_DlOpen_USE_DLFCN 1
#elif defined(CONFIG_HOST_WINDOWS)
#define DeeSystem_DlOpen_USE_LOADLIBRARY 1
#elif defined(CONFIG_HAVE_dlopen) && defined(CONFIG_HAVE_dlsym)
#define DeeSystem_DlOpen_USE_DLFCN 1
#else
#define DeeSystem_DlOpen_USE_STUB 1
#endif
#endif /* CONFIG_BUILDING_DEEMON */


/* Required file extension for shared libraries. */
#ifdef CONFIG_SHLIB_EXTENSION
#define DeeSystem_SHEXT  CONFIG_SHLIB_EXTENSION
#elif defined(CONFIG_HOST_WINDOWS)
#define DeeSystem_SHEXT  ".dll"
#elif defined(CONFIG_HOST_UNIX)
#define DeeSystem_SHEXT  ".so"
#else
#define DeeSystem_SHEXT  ".so" /* ??? */
#endif

/* Open a shared library
 * @return: * :                      A handle for the shared library.
 * @return: NULL:                    A deemon callback failed and an error was thrown.
 * @return: DEESYSTEM_DLOPEN_FAILED: Failed to open the shared library. */
DFUNDEF WUNUSED void *DCALL DeeSystem_DlOpen(/*String*/ DeeObject *__restrict filename);
DFUNDEF WUNUSED void *DCALL DeeSystem_DlOpenString(/*utf-8*/ char const *filename);
#define DEESYSTEM_DLOPEN_FAILED ((void *)-1)

/* Lookup a symbol within a given shared library
 * Returns `NULL' if the symbol could not be found */
DFUNDEF WUNUSED void *DCALL DeeSystem_DlSym(void *handle, char const *symbol_name);

/* Try to get a human-readable description on what went wrong during a call
 * to `DeeSystem_DlOpen[String]()' that caused `DEESYSTEM_DLOPEN_FAILED' to
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
DFUNDEF WUNUSED int DCALL
DeeSystem_PrintFilenameOfFD(struct Dee_unicode_printer *__restrict printer, int fd);

/* Read the contexts of the given link.
 * If the host doesn't support symbolic links, throw an error.
 * @return: 0 / * :        Success.
 * @return: -1 / NULL:     Error.
 * @return: 1 / ITER_DONE: The specified file isn't a symbolic link. */
DFUNDEF WUNUSED int DCALL DeeUnixSystem_Printlink(struct Dee_unicode_printer *__restrict printer, /*String*/ DeeObject *__restrict filename);
DFUNDEF WUNUSED int DCALL DeeUnixSystem_PrintlinkString(struct Dee_unicode_printer *__restrict printer, /*utf-8*/ char const *filename);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeUnixSystem_Readlink(/*String*/ DeeObject *__restrict filename);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeUnixSystem_ReadlinkString(/*utf-8*/ char const *filename);

/* Retrieve the unix FD associated with a given object.
 * The translation is done by performing the following:
 * >> #ifdef DeeSysFS_IS_INT
 * >> if (DeeFile_Check(ob))
 * >>     return DeeFile_GetSysFD(ob);
 * >> #endif
 * >> if (DeeInt_Check(ob))
 * >>     return DeeInt_AsInt(ob);
 * >> try return DeeObject_AsInt(DeeObject_GetAttr(ob, DeeSysFD_INT_GETSET)); catch (AttributeError);
 * >> return DeeObject_AsInt(ob);
 * Note that both msvc, as well as cygwin define `get_osfhandle()' as one
 * of the available functions, meaning that in both scenarios we are able
 * to get access to the underlying HANDLE. However, should deemon ever be
 * linked against a windows libc without this function, then only the
 * `DeeSysFD_HANDLE_GETSET' variant will be usable.
 * @return: * : Success (the actual handle value)
 * @return: -1: Error (handle translation failed)
 *              In case the actual handle value stored inside of `ob'
 *              was `-1', then an `DeeError_FileClosed' error is thrown. */
DFUNDEF WUNUSED int DCALL
DeeUnixSystem_GetFD(DeeObject *__restrict ob);



#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeSystem_PrintPwd(printer, include_trailing_sep) \
	__builtin_expect(DeeSystem_PrintPwd(printer, include_trailing_sep), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */


DECL_END

#endif /* !GUARD_DEEMON_SYSTEM_H */
