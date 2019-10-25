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
#ifndef GUARD_DEEMON_SYSTEM_SYSTEM_C
#define GUARD_DEEMON_SYSTEM_SYSTEM_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

#ifdef CONFIG_HAVE_DLFCN_H
#include <dlfcn.h>
#endif /* CONFIG_HAVE_DLFCN_H */

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

#ifndef CONFIG_HAVE_memrchr
#define CONFIG_HAVE_memrchr 1
#define memrchr dee_memrchr
DeeSystem_DEFINE_memrchr(dee_memrchr)
#endif /* !CONFIG_HAVE_memrchr */


/* Figure out how to implement `DeeSystem_PrintPwd()' */
#undef DeeSystem_PrintPwd_USE_WINDOWS
#undef DeeSystem_PrintPwd_USE_WGETCWD
#undef DeeSystem_PrintPwd_USE_GETCWD
#undef DeeSystem_PrintPwd_USE_GETENV
#undef DeeSystem_PrintPwd_USE_DOT
#if defined(CONFIG_HOST_WINDOWS)
#define DeeSystem_PrintPwd_USE_WINDOWS 1
#elif defined(CONFIG_HAVE_wgetcwd)
#define DeeSystem_PrintPwd_USE_WGETCWD 1
#elif defined(CONFIG_HAVE_getcwd)
#define DeeSystem_PrintPwd_USE_GETCWD 1
#elif defined(CONFIG_HAVE_getenv)
#define DeeSystem_PrintPwd_USE_GETENV 1
#elif
#define DeeSystem_PrintPwd_USE_DOT 1
#endif

#ifdef DeeSystem_PrintPwd_USE_WGETCWD
#ifndef CONFIG_HAVE_wcslen
#define wcslen          dee_wcslen
DeeSystem_DEFINE_wcslen(dee_wcslen)
#endif /* !CONFIG_HAVE_wcslen */
#endif /* DeeSystem_PrintPwd_USE_WGETCWD */

#ifdef DeeSystem_PrintPwd_USE_WINDOWS
PRIVATE ATTR_COLD int DCALL nt_err_getcwd(DWORD dwError) {
	if (DeeNTSystem_IsAccessDeniedError(dwError)) {
		return DeeError_SysThrowf(&DeeError_FileAccessError, dwError,
		                          "Permission to read a part of the current "
		                          "working directory's path was denied");
	} else if (DeeNTSystem_IsFileNotFoundError(dwError)) {
		return DeeError_SysThrowf(&DeeError_FileNotFound, dwError,
		                          "The current working directory has been unlinked");
	}
	return DeeError_SysThrowf(&DeeError_FSError, dwError,
	                          "Failed to retrieve the current working directory");
}
#endif /* DeeSystem_PrintPwd_USE_WINDOWS */



/* Ensure that the given `filename' describes an absolute path. */
PUBLIC WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeSystem_MakeAbsolute(/*String*/ DeeObject *__restrict filename) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	char *iter, *begin, *end, *flush_start, *flush_end, ch;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	begin = DeeString_AsUtf8(filename);
	if unlikely(!begin)
		goto err;
	end = begin + WSTR_LENGTH(begin);
	/* Strip leading space. */
	begin = utf8_skipspace(begin, end);
	if (!DeeSystem_IsAbs(begin)) {
		/* Print the current working directory when the given path isn't absolute. */
		if unlikely(DeeSystem_PrintPwd(&printer, true) < 0)
			goto err;
#ifdef CONFIG_HOST_WINDOWS
		/* Handle drive-relative paths. */
		if (DeeSystem_IsSep(begin[0]) && UNICODE_PRINTER_LENGTH(&printer)) {
			size_t index = 0;
			/* This sep must exist because it was printed by `DeeSystem_PrintPwd()' */
			while ((++index, UNICODE_PRINTER_GETCHAR(&printer, index - 1) != DeeSystem_SEP))
				;
			unicode_printer_truncate(&printer, index);
			/* Strip leading slashes. */
			for (;;) {
				begin = utf8_skipspace(begin, end);
				if (begin >= end)
					break;
				if (!DeeSystem_IsSep(*begin))
					break;
				++begin;
			}
		}
#endif /* CONFIG_HOST_WINDOWS */
	}
	iter = flush_start = begin;
	ASSERTF(*end == '\0',
	        "path = %r\n"
	        "end = %p(%q)\n",
	        filename, end, end);
next:
	ch = *iter++;
	switch (ch) {

	/* NOTE: The following part has been mirrored in `fs_pathexpand'
	 *       that is apart of the `fs' DEX implementation file: `fs/path.c'
	 *       If a bug is found in this code, it should be fixed here, as
	 *       well as within the DEX source file. */
#ifdef DeeSystem_ALTSEP
	case DeeSystem_ALTSEP:
#endif /* DeeSystem_ALTSEP */
	case DeeSystem_SEP:
	case '\0': {
		char const *sep_loc;
		bool did_print_sep;
		sep_loc = flush_end = iter - 1;
		/* Skip multiple slashes and whitespace following a path separator. */
		for (;;) {
			iter = utf8_skipspace(iter, end);
			if (iter >= end)
				break;
			if (!DeeSystem_IsSep(*iter))
				break;
			++iter;
		}
		flush_end = utf8_skipspace_rev(flush_end, flush_start);
		/* Analyze the last path portion for being a special name (`.' or `..') */
		if (flush_end[-1] == '.') {
			if (flush_end[-2] == '.' && flush_end - 2 == flush_start) {
				dssize_t new_end;
				size_t printer_length;
				/* Parent-directory-reference. */
				/* Delete the last directory that was written. */
				if (!printer.up_buffer)
					goto do_flush_after_sep;
				printer_length = printer.up_length;
				if (!printer_length)
					goto do_flush_after_sep;
				if (UNICODE_PRINTER_GETCHAR(&printer, printer_length - 1) == DeeSystem_SEP)
					--printer_length;
				new_end = unicode_printer_memrchr(&printer, DeeSystem_SEP, 0, printer_length);
				if (new_end < 0)
					goto do_flush_after_sep;
				++new_end;
				/* Truncate the valid length of the printer to after the previous slash. */
				printer.up_length = (size_t)new_end;
				unicode_printer_truncate(&printer, (size_t)new_end);
				goto done_flush;
			} else if (flush_end[-3] == DeeSystem_SEP && flush_end - 3 >= flush_start) {
				/* Parent-directory-reference. */
				char *new_end;
				new_end = (char *)memrchr(flush_start, DeeSystem_SEP,
				                          (size_t)((flush_end - 3) - flush_start));
				if (!new_end)
					goto done_flush;
				flush_end = new_end + 1; /* Include the previous sep in this flush. */
				if (unicode_printer_print(&printer, flush_start,
				                          (size_t)(flush_end - flush_start)) < 0)
					goto err;
				goto done_flush;
			} else if (flush_end - 1 == flush_start) {
				/* Self-directory-reference. */
done_flush:
				flush_start = iter;
				goto done_flush_nostart;
			} else if (flush_end[-2] == DeeSystem_SEP &&
			           flush_end - 2 >= flush_start) {
				/* Self-directory-reference. */
				flush_end -= 2;
			}
		}
do_flush_after_sep:
		/* Check if we need to fix anything */
		if (flush_end == iter - 1
#ifdef CONFIG_HOST_WINDOWS
		    && (*sep_loc == DeeSystem_SEP || iter == end + 1)
#endif /* CONFIG_HOST_WINDOWS */
		    ) {
			goto done_flush_nostart;
		}
		/* If we can already include a slash in this part, do so. */
		did_print_sep = false;
		if (sep_loc == flush_end
#ifdef CONFIG_HOST_WINDOWS
		    && (*sep_loc == DeeSystem_SEP)
#endif /* !CONFIG_HOST_WINDOWS */
		    ) {
			++flush_end;
			did_print_sep = true;
		}
		/* Flush everything prior to the path. */
		ASSERT(flush_end >= flush_start);
		if (unicode_printer_print(&printer, flush_start,
		                          (size_t)(flush_end - flush_start)) < 0)
			goto err;
		flush_start = iter;
		if (did_print_sep)
			; /* The slash has already been been printed: `foo/ bar' */
		else if (sep_loc == iter - 1
#ifdef CONFIG_HOST_WINDOWS
		         && (!*sep_loc || *sep_loc == DeeSystem_SEP)
#endif /* !CONFIG_HOST_WINDOWS */
		         ) {
			--flush_start; /* The slash will be printed as part of the next flush: `foo /bar' */
		} else {
			/* The slash must be printed explicitly: `foo / bar' */
			if (unicode_printer_putascii(&printer, DeeSystem_SEP) < 0)
				goto err;
		}
done_flush_nostart:
		if (iter == end + 1)
			goto done;
		goto next;
	}
	default: goto next;
	}
done:
	--iter;
	/* Print the remainder. */
	if (iter > flush_start) {

		/* Check for special case: The printer was never used.
		 * If this is the case, we can simply re-return the given path. */
		if (!UNICODE_PRINTER_LENGTH(&printer)) {
			unicode_printer_fini(&printer);
			return_reference_(filename);
		}
		/* Actually print the remainder. */
		if (unicode_printer_print(&printer, flush_start,
		                          (size_t)(iter - flush_start)) < 0)
			goto err;
	}
	/* Pack everything together. */
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}


/* Print the current working directory to the given `printer'
 * @param: include_trailing_sep: A trailing / or \\-character is also printed.
 * @return:  0: Success.
 * @return: -1: An error occurred (s.a. `DeeError_*'). */
PUBLIC int
(DCALL DeeSystem_PrintPwd)(struct Dee_unicode_printer *__restrict printer,
                           bool include_trailing_sep) {
#ifdef DeeSystem_PrintPwd_USE_WINDOWS
	LPWSTR buffer;
	DWORD new_bufsize, bufsize = 256;
	buffer = unicode_printer_alloc_wchar(printer, bufsize);
	if unlikely(!buffer)
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	new_bufsize = GetCurrentDirectoryW(bufsize + 1, buffer);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(!new_bufsize) {
DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_CollectMemory(1))
				goto again;
		} else {
			nt_err_getcwd(dwError);
		}
		goto err_release;
	}
	if (new_bufsize > bufsize) {
		LPWSTR new_buffer;
		/* Increase the buffer and try again. */
		new_buffer = unicode_printer_resize_wchar(printer, buffer, new_bufsize);
		if unlikely(!new_buffer)
			goto err_release;
		bufsize = new_bufsize;
		goto again;
	}
	if (!include_trailing_sep) {
		while (new_bufsize && DeeSystem_IsSep(buffer[new_bufsize - 1]))
			--new_bufsize;
	}
	if unlikely(!new_bufsize) {
		new_bufsize = 1;
		buffer[0] = '.';
	}
	if (include_trailing_sep && new_bufsize < bufsize &&
	    !DeeSystem_IsSep(buffer[new_bufsize - 1])) {
		buffer[new_bufsize + 0] = DeeSystem_SEP;
		buffer[new_bufsize + 1] = 0;
		++new_bufsize;
		include_trailing_sep = false;
	}
	if unlikely(unicode_printer_confirm_wchar(printer, buffer, new_bufsize) < 0)
		goto err;
	ASSERT(printer->up_length != 0);
	if (include_trailing_sep) {
		uint32_t ch;
		ch = UNICODE_PRINTER_GETCHAR(printer, printer->up_length - 1);
		if (!DeeSystem_IsSep(ch)) {
			if (unicode_printer_putascii(printer, DeeSystem_SEP))
				goto err;
		}
	}
	return 0;
err_release:
	unicode_printer_free_wchar(printer, buffer);
err:
	return -1;
#endif /* DeeSystem_PrintPwd_USE_WINDOWS */
#ifdef DeeSystem_PrintPwd_USE_WGETCWD
	dwchar_t *buffer, *new_buffer;
	size_t bufsize = 256;
	buffer = unicode_printer_alloc_wchar(printer, bufsize);
	if unlikely(!buffer)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	while (!wgetcwd((wchar_t *)buffer, bufsize + 1)) {
		DBG_ALIGNMENT_ENABLE();
		/* Increase the buffer and try again. */
#if defined(CONFIG_HAVE_errno) && defined(ERANGE)
		int error = DeeSystem_GetErrno();
		if (error != ERANGE) {
			DBG_ALIGNMENT_ENABLE();
			DeeError_SysThrowf(&DeeError_SystemError, error,
			                   "Failed to determine the current working directory");
			goto err_release;
		}
#endif /* CONFIG_HAVE_errno && ERANGE */
		bufsize *= 2;
		new_buffer = unicode_printer_resize_wchar(printer, buffer, bufsize);
		if unlikely(!new_buffer)
			goto err_release;
		DBG_ALIGNMENT_DISABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	bufsize = wcslen(buffer);
	if (!include_trailing_sep) {
		while (bufsize && DeeSystem_IsSep(buffer[bufsize - 1]))
			--bufsize;
	}
	if unlikely(!bufsize) {
		bufsize   = 1;
		buffer[0] = '.';
	}
	if (include_trailing_sep && bufsize < bufsize &&
	    !DeeSystem_IsSep(buffer[bufsize - 1])) {
		buffer[bufsize + 0] = DeeSystem_SEP;
		buffer[bufsize + 1] = 0;
		++bufsize;
		include_trailing_sep = false;
	}
	if unlikely(unicode_printer_confirm_wchar(printer, buffer, bufsize) < 0)
		goto err;
	ASSERT(printer->up_length != 0);
	if (include_trailing_sep) {
		uint32_t ch;
		ch = UNICODE_PRINTER_GETCHAR(printer, printer->up_length - 1);
		if (!DeeSystem_IsSep(ch)) {
			if (unicode_printer_putascii(printer, DeeSystem_SEP))
				goto err;
		}
	}
	return 0;
err_release:
	unicode_printer_free_wchar(printer, buffer);
err:
	return -1;
#endif /* DeeSystem_PrintPwd_USE_WGETCWD */
#ifdef DeeSystem_PrintPwd_USE_GETCWD
	char *buffer, *new_buffer;
	size_t bufsize = 256;
	buffer = unicode_printer_alloc_utf8(printer, bufsize);
	if unlikely(!buffer)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	while (!getcwd(buffer, bufsize + 1)) {
		/* Increase the buffer and try again. */
#if defined(CONFIG_HAVE_errno) && defined(ERANGE)
		int error = DeeSystem_GetErrno();
		if (error != ERANGE) {
			DBG_ALIGNMENT_ENABLE();
			DeeError_SysThrowf(&DeeError_SystemError, error,
			                   "Failed to determine the current working directory");
			goto err_release;
		}
#endif /* CONFIG_HAVE_errno && ERANGE */
		bufsize *= 2;
		new_buffer = unicode_printer_resize_utf8(printer, buffer, bufsize);
		if unlikely(!new_buffer)
			goto err_release;
		DBG_ALIGNMENT_DISABLE();
	}
	bufsize = strlen(buffer);
	DBG_ALIGNMENT_ENABLE();
	if (!include_trailing_sep) {
		while (bufsize && DeeSystem_IsSep(buffer[bufsize - 1]))
			--bufsize;
	}
	if unlikely(!bufsize) {
		bufsize   = 1;
		buffer[0] = '.';
	}
	if (include_trailing_sep && bufsize < bufsize &&
	    !DeeSystem_IsSep(buffer[bufsize - 1])) {
		buffer[bufsize + 0] = DeeSystem_SEP;
		buffer[bufsize + 1] = 0;
		++bufsize;
		include_trailing_sep = false;
	}
	if unlikely(unicode_printer_confirm_utf8(printer, buffer, bufsize) < 0)
		goto err;
	ASSERT(printer->up_length != 0);
	if (include_trailing_sep) {
		uint32_t ch;
		ch = UNICODE_PRINTER_GETCHAR(printer, printer->up_length - 1);
		if (!DeeSystem_IsSep(ch)) {
			if (unicode_printer_putascii(printer, DeeSystem_SEP))
				goto err;
		}
	}
	return 0;
err_release:
	unicode_printer_free_utf8(printer, buffer);
err:
	return -1;
#endif /* DeeSystem_PrintPwd_USE_GETCWD */
#ifdef DeeSystem_PrintPwd_USE_GETENV
	size_t pwdlen;
	char const *pwd;
	DBG_ALIGNMENT_DISABLE();
	pwd = getenv("PWD");
	DBG_ALIGNMENT_ENABLE();
	if (!pwd)
		pwd = "";
	DBG_ALIGNMENT_DISABLE();
	pwdlen = strlen(pwd);
	DBG_ALIGNMENT_ENABLE();
	while (pwdlen && DeeSystem_IsSep(pwd[pwdlen - 1]))
		--pwdlen;
	if unlikely(!pwdlen) {
		pwd = "." DeeSystem_SEP_S;
		pwdlen = 2;
		if (!include_trailing_sep)
			pwdlen = 1;
	}
	if (unicode_printer_printutf8(printer, pwd, pwdlen) < 0)
		goto err;
	ASSERT(printer->up_length != 0);
	if (include_trailing_sep) {
		uint32_t ch;
		ch = UNICODE_PRINTER_GETCHAR(printer, printer->up_length - 1);
		if (!DeeSystem_IsSep(ch)) {
			if (unicode_printer_putascii(printer, DeeSystem_SEP))
				goto err;
		}
	}
	return 0;
err:
	return -1;
#endif /* DeeSystem_PrintPwd_USE_GETENV */
#ifdef DeeSystem_PrintPwd_USE_DOT
	dssize_t error;
	error = unicode_printer_printutf8(printer,
	                                  "." DeeSystem_SEP_S,
	                                  include_trailing_sep ? 2 : 1);
	if likely(error >= 0)
		error = 0;
	return (int)error;
#endif /* DeeSystem_PrintPwd_USE_DOT */
}

DECL_END


#ifdef DeeSystem_DlOpen_USE_LOADLIBRARY
#ifdef _WIN32_WCE
#undef GetProcAddress
#define GetProcAddress GetProcAddressA
#endif /* _WIN32_WCE */
#endif /* DeeSystem_DlOpen_USE_LOADLIBRARY */

#if defined(DeeSystem_DlOpen_USE_DLFCN) && defined(CONFIG_HAVE_DLFCN_H)
#include <dlfcn.h>
#endif /* DeeSystem_DlOpen_USE_DLFCN && CONFIG_HAVE_DLFCN_H */

#ifdef DeeSystem_DlOpen_USE_DLFCN
#ifndef USED_DLOPEN_SCOPE
#if !defined(CONFIG_NO_RTLD_LOCAL) && \
    (defined(RTLD_LOCAL) || defined(CONFIG_HAVE_RTLD_LOCAL))
#define USED_DLOPEN_SCOPE RTLD_LOCAL
#elif !defined(CONFIG_NO_RTLD_GLOBAL) && \
      (defined(RTLD_GLOBAL) || defined(CONFIG_HAVE_RTLD_GLOBAL))
#define USED_DLOPEN_SCOPE RTLD_GLOBAL
#else
#define USED_DLOPEN_SCOPE 0
#endif
#endif /* !USED_DLOPEN_SCOPE */

#ifndef USED_DLOPEN_BIND
#if !defined(CONFIG_NO_RTLD_LAZY) && \
    (defined(RTLD_LAZY) || defined(CONFIG_HAVE_RTLD_LAZY))
#define USED_DLOPEN_BIND RTLD_LAZY
#elif !defined(CONFIG_NO_RTLD_NOW) && \
      (defined(RTLD_NOW) || defined(CONFIG_HAVE_RTLD_NOW))
#define USED_DLOPEN_BIND RTLD_NOW
#else
#define USED_DLOPEN_BIND 0
#endif
#endif /* !USED_DLOPEN_BIND */
#endif /* DeeSystem_DlOpen_USE_DLFCN */



DECL_BEGIN

/* Open a shared library
 * @return: * :                      A handle for the shared library.
 * @return: NULL:                    A deemon callback failed and an error was thrown.
 * @return: DEESYSTEM_DLOPEN_FAILED: Failed to open the shared library. */
PUBLIC WUNUSED void *DCALL
DeeSystem_DlOpen(/*String*/ DeeObject *__restrict filename) {
#ifdef DeeSystem_DlOpen_USE_LOADLIBRARY
	HMODULE hResult;
	LPCWSTR lpFilename;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
again:
	lpFilename = DeeString_AsWide(filename);
	if unlikely(!lpFilename)
		goto err;
again_loadlib:
	DBG_ALIGNMENT_DISABLE();
	hResult = LoadLibraryW(lpFilename);
	DBG_ALIGNMENT_ENABLE();
	if (!hResult) {
		DWORD dwError;
		hResult = (HMODULE)DEESYSTEM_DLOPEN_FAILED;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again_loadlib;
		}
		if (DeeNTSystem_IsUncError(dwError)) {
			DREF DeeObject *unc_filename;
			/* Try to fix-up the filename. */
			unc_filename = DeeNTSystem_FixUncPath(filename);
			if unlikely(!unc_filename)
				goto err;
			lpFilename = DeeString_AsWide(unc_filename);
			if unlikely(!lpFilename) {
				Dee_Decref(unc_filename);
				goto err;
			}
			/* Try to load the library once again. */
			DBG_ALIGNMENT_DISABLE();
			hResult = LoadLibraryW(lpFilename);
			DBG_ALIGNMENT_ENABLE();
			Dee_Decref(unc_filename);
			if (!hResult) {
				DBG_ALIGNMENT_DISABLE();
				dwError = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				if (DeeNTSystem_IsIntr(dwError)) {
					if (DeeThread_CheckInterrupt())
						goto err;
					goto again;
				}
				hResult = (HMODULE)DEESYSTEM_DLOPEN_FAILED;
			}
		}
	}
	return (void *)hResult;
err:
	return NULL;
#endif /* DeeSystem_DlOpen_USE_LOADLIBRARY */

#if defined(DeeSystem_DlOpen_USE_DLFCN)
	void *result;
	char *utf8_filename;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	utf8_filename = DeeString_AsUtf8(filename);
	if unlikely(!utf8_filename)
		goto err;
	result = DeeSystem_DlOpenString(utf8_filename);
	return result;
err:
	return NULL;
#endif /* DeeSystem_DlOpen_USE_DLFCN */

#ifdef DeeSystem_DlOpen_USE_STUB
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	(void)filename;
	return DEESYSTEM_DLOPEN_FAILED;
#endif /* DeeSystem_DlOpen_USE_STUB */
}


PUBLIC WUNUSED void *DCALL
DeeSystem_DlOpenString(/*utf-8*/ char const *filename) {
#ifdef DeeSystem_DlOpen_USE_LOADLIBRARY
	HMODULE hResult;
	DBG_ALIGNMENT_DISABLE();
	hResult = LoadLibraryA(filename);
	DBG_ALIGNMENT_ENABLE();
	if (!hResult) {
		/* Try to convert `filename' into its wide-character form,
		 * then use `DeeSystem_DlOpen()' to load the library. */
		DREF DeeStringObject *filename_ob;
		filename_ob = (DREF DeeStringObject *)DeeString_NewUtf8(filename, strlen(filename),
		                                                        STRING_ERROR_FSTRICT);
		if unlikely(!filename_ob)
			goto done;
		hResult = (HMODULE)DeeSystem_DlOpen((DeeObject *)filename_ob);
		Dee_Decref(filename_ob);
	}
done:
	return (void *)hResult;
#endif /* DeeSystem_DlOpen_USE_LOADLIBRARY */

#ifdef DeeSystem_DlOpen_USE_DLFCN
	void *result;
	DBG_ALIGNMENT_DISABLE();
	result = dlopen(filenames,
	                USED_DLOPEN_SCOPE |
	                USED_DLOPEN_BIND);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(!result)
		result = DEESYSTEM_DLOPEN_FAILED;
	return result;
#endif /* DeeSystem_DlOpen_USE_DLFCN */

#ifdef DeeSystem_DlOpen_USE_STUB
	(void)filename;
	return DEESYSTEM_DLOPEN_FAILED;
#endif /* DeeSystem_DlOpen_USE_STUB */
}


/* Lookup a symbol within a given shared library
 * Returns `NULL' if the symbol could not be found */
PUBLIC WUNUSED void *DCALL
DeeSystem_DlSym(void *handle, char const *symbol_name) {
#ifdef DeeSystem_DlOpen_USE_LOADLIBRARY
	FARPROC result;
	DBG_ALIGNMENT_DISABLE();
	result = GetProcAddress((HMODULE)handle, symbol_name);
	DBG_ALIGNMENT_ENABLE();
	return *(void **)&result;
#endif /* DeeSystem_DlOpen_USE_LOADLIBRARY */
#ifdef DeeSystem_DlOpen_USE_DLFCN
	void *result;
	DBG_ALIGNMENT_DISABLE();
	result = dlsym(handle, symbol_name);
	DBG_ALIGNMENT_ENABLE();
	return result;
#endif /* DeeSystem_DlOpen_USE_DLFCN */
#ifdef DeeSystem_DlOpen_USE_STUB
	(void)handle;
	(void)symbol_name;
	return NULL;
#endif /* DeeSystem_DlOpen_USE_STUB */
}

/* Close a given shared library */
PUBLIC void DCALL DeeSystem_DlClose(void *handle) {
#ifdef DeeSystem_DlOpen_USE_LOADLIBRARY
	DBG_ALIGNMENT_DISABLE();
	FreeLibrary((HMODULE)handle);
	DBG_ALIGNMENT_ENABLE();
#endif /* DeeSystem_DlOpen_USE_LOADLIBRARY */
#ifdef DeeSystem_DlOpen_USE_DLFCN
#ifdef CONFIG_HAVE_dlclose
	DBG_ALIGNMENT_DISABLE();
	dlclose(handle);
	DBG_ALIGNMENT_ENABLE();
#endif /* CONFIG_HAVE_dlclose */
#endif /* DeeSystem_DlOpen_USE_DLFCN */
}


DECL_END

#ifndef __INTELLISENSE__
#ifdef CONFIG_HOST_WINDOWS
#include "system-nt.c.inl"
#endif /* CONFIG_HOST_WINDOWS */

#include "system-unix.c.inl"
#endif /* !__INTELLISENSE__ */



#endif /* !GUARD_DEEMON_SYSTEM_SYSTEM_C */
