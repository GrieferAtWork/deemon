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
#ifndef GUARD_DEEMON_SYSTEM_SYSTEM_C
#define GUARD_DEEMON_SYSTEM_SYSTEM_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/int.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>

#include <hybrid/atomic.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

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
#elif defined(CONFIG_HAVE_wgetcwd) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define DeeSystem_PrintPwd_USE_WGETCWD 1
#elif defined(CONFIG_HAVE_getcwd)
#define DeeSystem_PrintPwd_USE_GETCWD 1
#elif defined(CONFIG_HAVE_wgetcwd)
#define DeeSystem_PrintPwd_USE_WGETCWD 1
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
		return DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
		                               "Permission to read a part of the current "
		                               "working directory's path was denied");
	} else if (DeeNTSystem_IsFileNotFoundError(dwError)) {
		return DeeNTSystem_ThrowErrorf(&DeeError_FileNotFound, dwError,
		                               "The current working directory has been unlinked");
	}
	return DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
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
#ifdef CONFIG_HOST_WINDOWS
	/* Don't modify special filenames such as `CON' or `NUL' */
	switch ((size_t)(end - begin)) {
#define eqnocase(a, b) ((a) == (b) || (a) == ((b) + ('A' - 'a')))

	case 3:
		if (eqnocase(begin[0], 'n') && eqnocase(begin[1], 'u') && eqnocase(begin[2], 'l'))
			goto return_unmodified; /* NUL */
		if (eqnocase(begin[0], 'c') && eqnocase(begin[1], 'o') && eqnocase(begin[2], 'n'))
			goto return_unmodified; /* CON */
		if (eqnocase(begin[0], 'p') && eqnocase(begin[1], 'r') && eqnocase(begin[2], 'n'))
			goto return_unmodified; /* PRN */
		if (eqnocase(begin[0], 'a') && eqnocase(begin[1], 'u') && eqnocase(begin[2], 'x'))
			goto return_unmodified; /* AUX */
		break;

	case 4:
		/* COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9,
		 * LPT1, LPT2, LPT3, LPT4, LPT5, LPT6, LPT7, LPT8, LPT9 */
		if (begin[3] >= '1' && begin[3] <= '9' &&
		    ((eqnocase(begin[0], 'c') && eqnocase(begin[1], 'o') && eqnocase(begin[1], 'm')) ||
		     (eqnocase(begin[0], 'l') && eqnocase(begin[1], 'p') && eqnocase(begin[1], 't'))))
			goto return_unmodified;
		break;

	case 6:
		if (eqnocase(begin[3], 'i') && eqnocase(begin[4], 'n') && begin[5] == '$') {
			if (eqnocase(begin[0], 'c') && eqnocase(begin[1], 'o') && eqnocase(begin[2], 'n'))
				goto return_unmodified; /* CONIN$ */
#ifdef CONFIG_WANT_WINDOWS_STD_FILES
			if (eqnocase(begin[0], 's') && eqnocase(begin[1], 't') && eqnocase(begin[2], 'd'))
				goto return_unmodified; /* STDIN$ */
#endif /* CONFIG_WANT_WINDOWS_STD_FILES */
		}
		break;

	case 7:
		if (eqnocase(begin[3], 'o') && eqnocase(begin[4], 'u') &&
		    eqnocase(begin[5], 't') && begin[6] == '$') {
			if (eqnocase(begin[0], 'c') && eqnocase(begin[1], 'o') && eqnocase(begin[2], 'n'))
				goto return_unmodified; /* CONOUT$ */
#ifdef CONFIG_WANT_WINDOWS_STD_FILES
			if (eqnocase(begin[0], 's') && eqnocase(begin[1], 't') && eqnocase(begin[2], 'd'))
				goto return_unmodified; /* STDOUT$ */
#endif /* CONFIG_WANT_WINDOWS_STD_FILES */
		}
#ifdef CONFIG_WANT_WINDOWS_STD_FILES
		if (eqnocase(begin[0], 's') && eqnocase(begin[1], 't') && eqnocase(begin[2], 'd') &&
		    eqnocase(begin[3], 'e') && eqnocase(begin[4], 'r') && eqnocase(begin[5], 'r') &&
		    begin[6] == '$')
			goto return_unmodified; /* STDERR$ */
#endif /* CONFIG_WANT_WINDOWS_STD_FILES */
		break;

#undef eqnocase
	default:
		break;
	}
#endif /* CONFIG_HOST_WINDOWS */
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
#ifdef CONFIG_HOST_WINDOWS
return_unmodified:
#endif /* CONFIG_HOST_WINDOWS */
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
	DWORD new_bufsize, bufsize = PATH_MAX;
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

#if defined(DeeSystem_PrintPwd_USE_WGETCWD) || \
    defined(DeeSystem_PrintPwd_USE_GETCWD)
#ifdef DeeSystem_PrintPwd_USE_WGETCWD
#define IFELSE_WCHAR(wc, c) wc
#else /* DeeSystem_PrintPwd_USE_WGETCWD */
#define IFELSE_WCHAR(wc, c) c
#endif /* !DeeSystem_PrintPwd_USE_WGETCWD */
	IFELSE_WCHAR(dwchar_t, char) *buffer, *new_buffer;
	size_t bufsize = PATH_MAX, buflen;
	buffer = IFELSE_WCHAR(unicode_printer_alloc_wchar(printer, bufsize),
	                      unicode_printer_alloc_utf8(printer, bufsize));
	if unlikely(!buffer)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	while (!IFELSE_WCHAR(wgetcwd((wchar_t *)buffer, bufsize + 1),
	                     getcwd((char *)buffer, bufsize + 1))) {
		DBG_ALIGNMENT_ENABLE();
		/* Increase the buffer and try again. */
#if defined(CONFIG_HAVE_errno) && defined(ERANGE)
		int error = DeeSystem_GetErrno();
		if (error != ERANGE) {
			DBG_ALIGNMENT_ENABLE();
			DeeSystem_IF_E1(error, EACCES, {
				DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, error,
				                          "Permission to read a part of the current "
				                          "working directory's path was denied");
				goto err_release;
			});
			DeeSystem_IF_E1(error, ENOENT, {
				DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, error,
				                          "The current working directory has been unlinked");
				goto err_release;
			});
			DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
			                          "Failed to retrieve the current working directory");
			goto err_release;
		}
#endif /* CONFIG_HAVE_errno && ERANGE */
		bufsize *= 2;
		new_buffer = IFELSE_WCHAR(unicode_printer_resize_wchar(printer, buffer, bufsize),
		                          unicode_printer_resize_utf8(printer, buffer, bufsize));
		if unlikely(!new_buffer)
			goto err_release;
		DBG_ALIGNMENT_DISABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	buflen = IFELSE_WCHAR(wcslen(buffer),
	                      strlen(buffer));
	if (!include_trailing_sep) {
		while (buflen && DeeSystem_IsSep(buffer[buflen - 1]))
			--buflen;
	}
	if unlikely(!buflen) {
		buflen    = 1;
		buffer[0] = '.';
	}
	if (include_trailing_sep && buflen < bufsize &&
	    !DeeSystem_IsSep(buffer[buflen - 1])) {
		buffer[buflen + 0] = DeeSystem_SEP;
		buffer[buflen + 1] = 0;
		++buflen;
		include_trailing_sep = false;
	}
	if unlikely(IFELSE_WCHAR(unicode_printer_confirm_wchar(printer, buffer, buflen),
	                         unicode_printer_confirm_utf8(printer, buffer, buflen)) < 0)
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
	IFELSE_WCHAR(unicode_printer_free_wchar(printer, buffer),
	             unicode_printer_free_utf8(printer, buffer));
err:
	return -1;
#undef IFELSE_WCHAR
#endif /* DeeSystem_PrintPwd_USE_WGETCWD || DeeSystem_PrintPwd_USE_GETCWD */

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
	if likely(error > 0)
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

#ifdef DeeSystem_DlOpen_USE_DLFCN
#ifndef USED_DLOPEN_SCOPE
#if defined(CONFIG_HAVE_RTLD_LOCAL)
#define USED_DLOPEN_SCOPE RTLD_LOCAL
#elif defined(CONFIG_HAVE_RTLD_GLOBAL)
#define USED_DLOPEN_SCOPE RTLD_GLOBAL
#else
#define USED_DLOPEN_SCOPE 0
#endif
#endif /* !USED_DLOPEN_SCOPE */

#ifndef USED_DLOPEN_BIND
#if defined(CONFIG_HAVE_RTLD_LAZY)
#define USED_DLOPEN_BIND RTLD_LAZY
#elif defined(CONFIG_HAVE_RTLD_NOW)
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
	result = dlopen(filename,
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

/* Try to get a human-readable description on what went wrong during a call
 * to `DeeSystem_DlOpen[String]()' that caused `DEESYSTEM_DLOPEN_FAILED' to
 * be returned.
 * @return: * :        The human-readable error description
 * @return: NULL:      A deemon callback failed and an error was thrown.
 * @return: ITER_DONE: No description is available. */
PUBLIC WUNUSED DREF /*String*/ DeeObject *DCALL DeeSystem_DlError(void) {
#ifdef DeeSystem_DlOpen_USE_LOADLIBRARY
	DREF /*String*/ DeeObject *result;
	DWORD dwError;
	dwError = GetLastError();
	if (dwError == 0)
		return ITER_DONE;
	result = DeeNTSystem_FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
	                                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	                                   NULL);
	return result;
#endif /* DeeSystem_DlOpen_USE_LOADLIBRARY */

#ifdef DeeSystem_DlOpen_USE_DLFCN
#ifdef CONFIG_HAVE_dlerror
	char *message;
	DREF /*String*/ DeeObject *result;
	DBG_ALIGNMENT_DISABLE();
	message = dlerror();
	DBG_ALIGNMENT_ENABLE();
	if (!message)
		return ITER_DONE;
	result = DeeString_NewUtf8(message, strlen(message),
	                           STRING_ERROR_FIGNORE);
	return result;
#else /* CONFIG_HAVE_dlerror */
	return ITER_DONE;
#endif /* !CONFIG_HAVE_dlerror */
#endif /* DeeSystem_DlOpen_USE_DLFCN */

#ifdef DeeSystem_DlOpen_USE_STUB
	return ITER_DONE;
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






#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define FILETIME_GET64(x) (((x) << 32)|((x) >> 32))
#else /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */
#define FILETIME_GET64(x) (x)
#endif /* !__BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ */

/* A couple of helper macros taken from the libtime DEX. */
#define time_yer2day(x)     (((146097 * (x)) / 400) /*-1*/)
#define MICROSECONDS_PER_MILLISECOND UINT64_C(1000)
#define MILLISECONDS_PER_SECOND      UINT64_C(1000)
#define SECONDS_PER_MINUTE           UINT64_C(60)
#define MINUTES_PER_HOUR             UINT64_C(60)
#define HOURS_PER_DAY                UINT64_C(24)
#define MICROSECONDS_PER_SECOND (MICROSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND)
#define MICROSECONDS_PER_MINUTE (MICROSECONDS_PER_SECOND * SECONDS_PER_MINUTE)
#define MICROSECONDS_PER_HOUR   (MICROSECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define MICROSECONDS_PER_DAY    (MICROSECONDS_PER_HOUR * HOURS_PER_DAY)



/* Figure out how to implement `DeeSystem_GetLastModified()' */
#undef DeeSystem_GetLastModified_USE_GETFILEATTRIBUTESEX
#undef DeeSystem_GetLastModified_USE_STAT
#undef DeeSystem_GetLastModified_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define DeeSystem_GetLastModified_USE_GETFILEATTRIBUTESEX 1
#elif defined(CONFIG_HAVE_stat) || defined(CONFIG_HAVE_stat64)
#define DeeSystem_GetLastModified_USE_STAT 1
#else
#define DeeSystem_GetLastModified_USE_STUB 1
#endif


/* Figure out how to implement `DeeSystem_GetWalltime()' */
#undef DeeSystem_GetWalltime_USE_GETSYSTEMTIMEPRECISEASFILETIME
#undef DeeSystem_GetWalltime_USE_TIME
#undef DeeSystem_GetWalltime_USE_STUB
#undef DeeSystem_GetWalltime_USE_GETSYSTEMTIMEPRECISEASFILETIME
#undef DeeSystem_GetWalltime_USE_GETTIMEOFDAY64
#undef DeeSystem_GetWalltime_USE_GETTIMEOFDAY
#undef DeeSystem_GetWalltime_USE_CLOCK_GETTIME64
#undef DeeSystem_GetWalltime_USE_CLOCK_GETTIME
#undef DeeSystem_GetWalltime_USE_TIME64
#undef DeeSystem_GetWalltime_USE_TIME
#undef DeeSystem_GetWalltime_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define DeeSystem_GetWalltime_USE_GETSYSTEMTIMEPRECISEASFILETIME 1
#elif defined(CONFIG_HAVE_gettimeofday64)
#define DeeSystem_GetWalltime_USE_GETTIMEOFDAY64 1
#elif defined(CONFIG_HAVE_gettimeofday)
#define DeeSystem_GetWalltime_USE_GETTIMEOFDAY 1
#elif defined(CONFIG_HAVE_clock_gettime64) && defined(CONFIG_HAVE_CLOCK_REALTIME)
#define DeeSystem_GetWalltime_USE_CLOCK_GETTIME64 1
#elif defined(CONFIG_HAVE_clock_gettime) && defined(CONFIG_HAVE_CLOCK_REALTIME)
#define DeeSystem_GetWalltime_USE_CLOCK_GETTIME 1
#elif defined(CONFIG_HAVE_time64)
#define DeeSystem_GetWalltime_USE_TIME64 1
#elif defined(CONFIG_HAVE_time)
#define DeeSystem_GetWalltime_USE_TIME 1
#else
#define DeeSystem_GetWalltime_USE_STUB 1
#endif




#if (defined(DeeSystem_GetLastModified_USE_GETFILEATTRIBUTESEX) || \
     defined(DeeSystem_GetWalltime_USE_GETSYSTEMTIMEPRECISEASFILETIME))
#define FILETIME_PER_SECONDS 10000000 /* 100 nanoseconds / 0.1 microseconds. */
PRIVATE uint64_t DCALL
nt_getunixfiletime(uint64_t filetime) {
	uint64_t result;
	SYSTEMTIME systime;
	/* System-time only has millisecond-precision, so we copy over that part. */
	result = (FILETIME_GET64(filetime) / (FILETIME_PER_SECONDS / MICROSECONDS_PER_SECOND)) %
	         MICROSECONDS_PER_MILLISECOND;
	DBG_ALIGNMENT_DISABLE();
	FileTimeToSystemTime((LPFILETIME)&filetime, &systime);
	SystemTimeToTzSpecificLocalTime(NULL, &systime, &systime);
	SystemTimeToFileTime(&systime, (LPFILETIME)&filetime);
	DBG_ALIGNMENT_ENABLE();
	/* Copy over millisecond information and everything above. */
	result += (FILETIME_GET64(filetime) / (FILETIME_PER_SECONDS / MICROSECONDS_PER_SECOND));
	/* Window's filetime started counting on 01.01.1601. */
	return result - (time_yer2day(1970) - time_yer2day(1601)) * MICROSECONDS_PER_DAY;
}
#endif /* DeeSystem_GetLastModified_USE_GETFILEATTRIBUTESEX || DeeSystem_GetWalltime_USE_GETSYSTEMTIMEPRECISEASFILETIME */


/* Return the last modified timestamp of `filename'
 * > uses the same format as `DeeSystem_GetWalltime()' */
PUBLIC WUNUSED uint64_t DCALL
DeeSystem_GetLastModified(/*String*/ DeeObject *__restrict filename) {
#ifdef DeeSystem_GetLastModified_USE_GETFILEATTRIBUTESEX
	WIN32_FILE_ATTRIBUTE_DATA attrib;
	LPWSTR wname;
	wname = (LPWSTR)DeeString_AsWide(filename);
	if unlikely(!wname)
		return (uint64_t)-1;
	DBG_ALIGNMENT_DISABLE();
	if (!GetFileAttributesExW(wname, GetFileExInfoStandard, &attrib)) {
		DBG_ALIGNMENT_ENABLE();
		return 0;
	}
	DBG_ALIGNMENT_ENABLE();
	return nt_getunixfiletime((uint64_t)attrib.ftLastWriteTime.dwLowDateTime |
	                          (uint64_t)attrib.ftLastWriteTime.dwHighDateTime << 32);
#endif /* DeeSystem_GetLastModified_USE_GETFILEATTRIBUTESEX */

#ifdef DeeSystem_GetLastModified_USE_STAT
	uint64_t result;
#ifdef CONFIG_HAVE_stat64
	struct stat64 st;
#else /* CONFIG_HAVE_stat64 */
	struct stat st;
#endif /* !CONFIG_HAVE_stat64 */
	char const *utf8_name;
	utf8_name = DeeString_AsUtf8(filename);
	if unlikely(!utf8_name)
		return (uint64_t)-1;
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_stat64
	if (stat64(utf8_name, &st))
#else /* CONFIG_HAVE_stat64 */
	if (stat(utf8_name, &st))
#endif /* !CONFIG_HAVE_stat64 */
	{
		DBG_ALIGNMENT_ENABLE();
		return 0;
	}
	DBG_ALIGNMENT_ENABLE();
	result = (uint64_t)st.st_mtime * MICROSECONDS_PER_SECOND;
	/* Try to get more precision out of this */
#ifdef CONFIG_HAVE_STAT_ST_NSEC
	result += st.st_mtimensec / 1000;
#elif defined(CONFIG_HAVE_STAT_ST_TIM)
	result += st.st_mtim.tv_nsec / 1000;
#elif defined(CONFIG_HAVE_STAT_ST_TIMESPEC)
	result += st.st_mtimespec.tv_nsec / 1000;
#endif /* ... */

	return result;
#endif /* DeeSystem_GetLastModified_USE_STAT */

#ifdef DeeSystem_GetLastModified_USE_STUB
	(void)filename;
	return 0;
#endif /* DeeSystem_GetLastModified_USE_STUB */
}

#ifdef DeeSystem_GetWalltime_USE_GETSYSTEMTIMEPRECISEASFILETIME
typedef void (WINAPI *LPGETSYSTEMTIMEPRECISEASFILETIME)(LPFILETIME lpSystemTimeAsFileTime);
static LPGETSYSTEMTIMEPRECISEASFILETIME pdyn_GetSystemTimePreciseAsFileTime = NULL;
#define GetSystemTimePreciseAsFileTime (*pdyn_GetSystemTimePreciseAsFileTime)

#ifndef DEFINED_GET_KERNEL32_HANDLE
#define DEFINED_GET_KERNEL32_HANDLE 1
PRIVATE WCHAR const wKernel32[]    = { 'K', 'E', 'R', 'N', 'E', 'L', '3', '2', 0 };
PRIVATE WCHAR const wKernel32Dll[] = { 'K', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', 0 };
PRIVATE HMODULE DCALL GetKernel32Handle(void) {
	HMODULE hKernel32;
	hKernel32 = GetModuleHandleW(wKernel32);
	if (!hKernel32)
		hKernel32 = LoadLibraryW(wKernel32Dll);
	return hKernel32;
}
#endif /* !DEFINED_GET_KERNEL32_HANDLE */
#endif /* DeeSystem_GetWalltime_USE_GETSYSTEMTIMEPRECISEASFILETIME */


/* Return the current UTC realtime in microseconds since 01.01.1970T00:00:00+00:00 */
PUBLIC WUNUSED uint64_t DCALL DeeSystem_GetWalltime(void) {
#ifdef DeeSystem_GetWalltime_USE_GETSYSTEMTIMEPRECISEASFILETIME
	uint64_t filetime;
	DBG_ALIGNMENT_DISABLE();
	if (pdyn_GetSystemTimePreciseAsFileTime == NULL) {
		HMODULE hKernel32 = GetKernel32Handle();
		if (!hKernel32)
			ATOMIC_WRITE(*(void **)&pdyn_GetSystemTimePreciseAsFileTime, (void *)(uintptr_t)-1);
		else {
			LPGETSYSTEMTIMEPRECISEASFILETIME func;
			func = (LPGETSYSTEMTIMEPRECISEASFILETIME)GetProcAddress(hKernel32, "GetSystemTimePreciseAsFileTime");
			if (!func)
				*(void **)&func = (void *)(uintptr_t)-1;
			ATOMIC_WRITE(pdyn_GetSystemTimePreciseAsFileTime, func);
		}
	}
	if (pdyn_GetSystemTimePreciseAsFileTime != (LPGETSYSTEMTIMEPRECISEASFILETIME)-1) {
		GetSystemTimePreciseAsFileTime((LPFILETIME)&filetime);
	} else {
		GetSystemTimeAsFileTime((LPFILETIME)&filetime);
	}
	DBG_ALIGNMENT_ENABLE();
	return nt_getunixfiletime(filetime);
#endif /* DeeSystem_GetWalltime_USE_GETSYSTEMTIMEPRECISEASFILETIME */

	/* TODO: clock_gettime() */

#ifdef DeeSystem_GetWalltime_USE_GETTIMEOFDAY64
	struct timeval64 tv;
	DBG_ALIGNMENT_DISABLE();
	if (gettimeofday64(&tv, NULL)) {
		DBG_ALIGNMENT_ENABLE();
		return 0;
	}
	DBG_ALIGNMENT_ENABLE();
	return (uint64_t)tv.tv_sec * MICROSECONDS_PER_SECOND + tv.tv_usec;
#endif /* DeeSystem_GetWalltime_USE_GETTIMEOFDAY64 */

#ifdef DeeSystem_GetWalltime_USE_GETTIMEOFDAY
	struct timeval tv;
	DBG_ALIGNMENT_DISABLE();
	if (gettimeofday(&tv, NULL)) {
		DBG_ALIGNMENT_ENABLE();
		return 0;
	}
	DBG_ALIGNMENT_ENABLE();
	return (uint64_t)tv.tv_sec * MICROSECONDS_PER_SECOND + tv.tv_usec;
#endif /* DeeSystem_GetWalltime_USE_GETTIMEOFDAY */

#ifdef DeeSystem_GetWalltime_USE_CLOCK_GETTIME64
	struct timespec64 ts;
	DBG_ALIGNMENT_DISABLE();
	if (clock_gettime64(CLOCK_REALTIME, &ts)) {
		DBG_ALIGNMENT_ENABLE();
		return 0;
	}
	DBG_ALIGNMENT_ENABLE();
	return (uint64_t)tv.tv_sec * MICROSECONDS_PER_SECOND + (tv.tv_nsec / 1000);
#endif /* DeeSystem_GetWalltime_USE_CLOCK_GETTIME64 */

#ifdef DeeSystem_GetWalltime_USE_CLOCK_GETTIME
	struct timespec ts;
	DBG_ALIGNMENT_DISABLE();
	if (clock_gettime(CLOCK_REALTIME, &ts)) {
		DBG_ALIGNMENT_ENABLE();
		return 0;
	}
	DBG_ALIGNMENT_ENABLE();
	return (uint64_t)tv.tv_sec * MICROSECONDS_PER_SECOND + (tv.tv_nsec / 1000);
#endif /* DeeSystem_GetWalltime_USE_CLOCK_GETTIME */

#ifdef DeeSystem_GetWalltime_USE_TIME64
	time64_t now;
	DBG_ALIGNMENT_DISABLE();
	now = time64(NULL);
	DBG_ALIGNMENT_ENABLE();
	return (uint64_t)now * MICROSECONDS_PER_SECOND;
#endif /* DeeSystem_GetWalltime_USE_TIME64 */

#ifdef DeeSystem_GetWalltime_USE_TIME
	time_t now;
	DBG_ALIGNMENT_DISABLE();
	now = time(NULL);
	DBG_ALIGNMENT_ENABLE();
	return (uint64_t)now * MICROSECONDS_PER_SECOND;
#endif /* DeeSystem_GetWalltime_USE_TIME */

#ifdef DeeSystem_GetWalltime_USE_STUB
	return 0;
#endif /* DeeSystem_GetWalltime_USE_STUB */
}




/* Figure out how to implement `DeeSystem_Unlink()' */
#undef DeeSystem_Unlink_USE_DELETEFILE
#undef DeeSystem_Unlink_WUNLINK
#undef DeeSystem_Unlink_UNLINK
#undef DeeSystem_Unlink_WREMOVE
#undef DeeSystem_Unlink_REMOVE
#undef DeeSystem_Unlink_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define DeeSystem_Unlink_USE_DELETEFILE 1
#elif defined(CONFIG_HAVE_wunlink) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define DeeSystem_Unlink_WUNLINK 1
#elif defined(CONFIG_HAVE_unlink)
#define DeeSystem_Unlink_UNLINK 1
#elif defined(CONFIG_HAVE_wunlink)
#define DeeSystem_Unlink_WUNLINK 1
#elif defined(CONFIG_HAVE_wremove) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define DeeSystem_Unlink_WREMOVE 1
#elif defined(CONFIG_HAVE_remove)
#define DeeSystem_Unlink_REMOVE 1
#elif defined(CONFIG_HAVE_wremove)
#define DeeSystem_Unlink_WREMOVE 1
#else
#define DeeSystem_Unlink_STUB 1
#endif



/* Try to unlink() the given `filename'
 * @return: 1 : The unlink() operation failed (only returned when `throw_exception_on_error' is `false')
 * @return: 0 : The unlink() operation was successful
 * @return: -1: An error occurred (may still be returned, even when `throw_exception_on_error' is `false') */
PUBLIC WUNUSED int DCALL
DeeSystem_Unlink(/*String*/ DeeObject *__restrict filename,
                 bool throw_exception_on_error) {
#ifdef DeeSystem_Unlink_USE_DELETEFILE
	DWORD dwError;
	LPWSTR wname;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	wname = (LPWSTR)DeeString_AsWide(filename);
	if unlikely(!wname) {
		/* Since unlink() is a cleanup operation,
		 * try hard to comply, even after an error! */
		if (DeleteFileA(DeeString_STR(filename))) {
			if (DeeError_Handled(ERROR_HANDLED_NORMAL))
				return 0;
		}
		goto err;
	}
again_deletefile:
	if (DeleteFileW(wname))
		return 0;
	dwError = GetLastError();
	if (DeeNTSystem_IsIntr(dwError)) {
		if (DeeThread_CheckInterrupt()) {
			/* Try hard to delete the file... (even after an interrupt) */
			DeleteFileW(wname);
			goto err;
		}
		goto again_deletefile;
	}
	if (DeeNTSystem_IsUncError(dwError)) {
		DREF DeeStringObject *unc_filename;
		/* Try again with a UNC filename. */
		unc_filename = (DREF DeeStringObject *)DeeNTSystem_FixUncPath(filename);
		if unlikely(!unc_filename)
			goto err;
		wname = (LPWSTR)DeeString_AsWide(filename);
		if unlikely(!wname) {
			/* No point in trying to use DeleteFileA() here.
			 * It doesn't work for UNC paths to begin with... */
			Dee_Decref(unc_filename);
			goto err;
		}
again_deletefile2:
		if (DeleteFileW(wname)) {
			Dee_Decref(unc_filename);
			return 0; /* Success! */
		}
		dwError = GetLastError();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt()) {
				/* Try hard to delete the file... (even after an interrupt) */
				DeleteFileW(wname);
				Dee_Decref(unc_filename);
				goto err;
			}
			goto again_deletefile2;
		}
		Dee_Decref(unc_filename);
	}
	/* Can't delete the file, no matter what I try...  :( */
	if (!throw_exception_on_error)
		return 1;
	DeeNTSystem_ThrowErrorf(NULL, dwError,
	                        "Failed to delete file %r",
	                        filename);
err:
	return -1;
#endif /* DeeSystem_Unlink_USE_DELETEFILE */

#if (defined(DeeSystem_Unlink_WUNLINK) || defined(DeeSystem_Unlink_WREMOVE) || \
     defined(DeeSystem_Unlink_UNLINK) || defined(DeeSystem_Unlink_REMOVE))
#if defined(DeeSystem_Unlink_WUNLINK) || defined(DeeSystem_Unlink_WREMOVE)
	dwchar_t *wname;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	wname = DeeString_AsWide(filename);
	if unlikely(!wname) {
#if defined(CONFIG_HAVE_unlink) || defined(CONFIG_HAVE_remove)
		/* Since unlink() is a cleanup operation,
		 * try hard to comply, even after an error! */
#ifdef CONFIG_HAVE_unlink
		if (unlink(DeeString_STR(filename)))
#else /* CONFIG_HAVE_unlink */
		if (remove(DeeString_STR(filename)))
#endif /* !CONFIG_HAVE_unlink */
		{
			if (DeeError_Handled(ERROR_HANDLED_NORMAL))
				return 0;
		}
#endif /* CONFIG_HAVE_unlink || CONFIG_HAVE_remove */
		return -1;
	}
#if defined(CONFIG_HAVE_errno) && defined(EINTR)
again_deletefile:
#endif /* CONFIG_HAVE_errno && EINTR */
#ifdef DeeSystem_Unlink_WUNLINK
	if (wunlink(wname))
		return 0;
#else /* DeeSystem_Unlink_WUNLINK */
	if (wremove(wname))
		return 0;
#endif /* !DeeSystem_Unlink_WUNLINK */
#if defined(CONFIG_HAVE_errno) && defined(EINTR)
	if (DeeSystem_GetErrno() == EINTR) {
		if (DeeThread_CheckInterrupt()) {
			/* Try hard to delete the file... (even after an interrupt) */
#ifdef DeeSystem_Unlink_WUNLINK
			wunlink(wname);
#else /* DeeSystem_Unlink_WUNLINK */
			wremove(wname);
#endif /* !DeeSystem_Unlink_WUNLINK */
			return -1;
		}
		goto again_deletefile;
	}
#endif /* CONFIG_HAVE_errno && EINTR */
#endif /* DeeSystem_Unlink_WUNLINK || DeeSystem_Unlink_WREMOVE */

#if defined(DeeSystem_Unlink_UNLINK) || defined(DeeSystem_Unlink_REMOVE)
	char const *utf8_name;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	utf8_name = DeeString_AsUtf8(filename);
	if unlikely(!utf8_name) {
		/* Since unlink() is a cleanup operation,
		 * try hard to comply, even after an error! */
#ifdef DeeSystem_Unlink_UNLINK
		if (unlink(DeeString_STR(filename)))
#else /* CONFIG_HAVE_unlink */
		if (remove(DeeString_STR(filename)))
#endif /* !CONFIG_HAVE_unlink */
		{
			if (DeeError_Handled(ERROR_HANDLED_NORMAL))
				return 0;
		}
		return -1;
	}
#if defined(CONFIG_HAVE_errno) && defined(EINTR)
again_deletefile:
#endif /* CONFIG_HAVE_errno && EINTR */
#ifdef DeeSystem_Unlink_UNLINK
	if (unlink(utf8_name))
		return 0;
#else /* DeeSystem_Unlink_UNLINK */
	if (remove(utf8_name))
		return 0;
#endif /* !DeeSystem_Unlink_UNLINK */
#if defined(CONFIG_HAVE_errno) && defined(EINTR)
	if (DeeSystem_GetErrno() == EINTR) {
		if (DeeThread_CheckInterrupt()) {
			/* Try hard to delete the file... (even after an interrupt) */
#ifdef DeeSystem_Unlink_UNLINK
			unlink(utf8_name);
#else /* DeeSystem_Unlink_UNLINK */
			remove(utf8_name);
#endif /* !DeeSystem_Unlink_UNLINK */
			return -1;
		}
		goto again_deletefile;
	}
#endif /* CONFIG_HAVE_errno && EINTR */
#endif /* DeeSystem_Unlink_WUNLINK || DeeSystem_Unlink_WREMOVE */
	/* Can't delete the file, no matter what I try...  :( */
	if (!throw_exception_on_error)
		return 1;
	{
		int error = DeeSystem_GetErrno();
		DeeSystem_IF_E4(error, ENOENT, ENOTDIR, ENAMETOOLONG, ELOOP, {
			return DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, error,
			                                 "Cannot delete missing file %r",
			                                 filename);
		});
		DeeSystem_IF_E2(error, EPERM, EACCES, {
			return DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, error,
			                                 "Not allowed to delete file %r",
			                                 filename);
		});
		DeeSystem_IF_E1(error, EROFS, {
			return DeeUnixSystem_ThrowErrorf(&DeeError_ReadOnlyFile, error,
			                                 "Cannot delete file %r on read-only filesystem",
			                                 filename);
		});
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                          "Failed to delete file %r",
		                          filename);
	}
	return -1;
#endif /* DeeSystem_Unlink_WUNLINK || DeeSystem_Unlink_WREMOVE || \
          DeeSystem_Unlink_UNLINK || DeeSystem_Unlink_REMOVE */

#ifdef DeeSystem_Unlink_STUB
	if (!throw_exception_on_error)
		return 1;
	DeeUnixSystem_ThrowErrorf(&DeeError_SystemError,
	                          DeeSystem_GetErrno(),
	                          "Failed to delete file %r",
	                          filename);
	return -1;
#endif /* DeeSystem_Unlink_STUB */

}




#ifndef GETATTR_fileno
#if defined(DeeSysFD_GETSET) && defined(DeeSysFS_IS_FILE)
#define GETATTR_fileno(ob) DeeObject_GetAttr(ob, &str_getsysfd)
#else /* DeeSysFD_GETSET && DeeSysFS_IS_FILE */
#define GETATTR_fileno(ob) DeeObject_GetAttrString(ob, DeeSysFD_INT_GETSET)
#endif /* !DeeSysFD_GETSET || !DeeSysFS_IS_FILE */
#endif /* !GETATTR_fileno */


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
PUBLIC WUNUSED int DCALL
DeeUnixSystem_GetFD(DeeObject *__restrict ob) {
	int error, result;
	DREF DeeObject *attr;
#ifdef DeeSysFS_IS_INT
	STATIC_ASSERT(DeeSysFD_INVALID == -1);
	if (DeeFile_Check(ob))
		return (int)DeeFile_GetSysFD(ob);
#endif /* DeeSysFS_IS_INT */
	if (DeeInt_Check(ob)) {
		error = DeeInt_AsInt(ob, &result);
		if unlikely(error)
			goto err;
		if unlikely(result == -1)
			goto err_fd_minus_one;
		return result;
	}
	attr = GETATTR_fileno(ob);
	if unlikely(!attr)
		goto err;
	if (attr) {
		error = DeeObject_AsInt(attr, &result);
		Dee_Decref(attr);
	} else {
		if (!DeeError_Catch(&DeeError_AttributeError) &&
		    !DeeError_Catch(&DeeError_NotImplemented))
			goto err;
		/* Fallback: Convert an `int'-object into a unix file descriptor. */
		error = DeeObject_AsInt(ob, &result);
	}
	if unlikely(error)
		goto err;
	if unlikely(result == -1)
		goto err_fd_minus_one;
	return result;
err_fd_minus_one:
	DeeError_Throwf(&DeeError_FileClosed,
	                "Invalid file descriptor -1");
err:
	return -1;
}




DECL_END

#ifndef __INTELLISENSE__
#ifdef CONFIG_HOST_WINDOWS
#include "system-nt.c.inl"
#endif /* CONFIG_HOST_WINDOWS */

#include "system-unix.c.inl"
#endif /* !__INTELLISENSE__ */



#endif /* !GUARD_DEEMON_SYSTEM_SYSTEM_C */
