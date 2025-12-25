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
#ifndef GUARD_DEEMON_EXECUTE_MODPATH_DIRECTORY_C
#define GUARD_DEEMON_EXECUTE_MODPATH_DIRECTORY_C 1

#include <deemon/api.h>
/**/

#include <deemon/alloc.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/debug-alignment.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
DECL_BEGIN

/* Allocate+return the uncached contents of the directory represented by `self'.
 * >> for (local e: opendir(self->mo_absname)) {
 * >>     if (e.d_type == DT_DIR) {
 * >>         yield e.d_name;
 * >>     } else if (e.d_type == DT_REG && e.d_name.endswith(".dee")) {
 * >>         yield e.d_name[:-4];
 * >>     } else if (e.d_type == DT_REG && e.d_name.endswith(DeeSystem_SOEXT)) {
 * >>         yield e.d_name[:-#DeeSystem_SOEXT];
 * >>     }
 * >> }
 *
 * Special case when `mo_absname == ""':
 * - enumerate files in filesystem root "/" on unix
 * - enumerate (lower-case) drive letters on windows
 *
 * Special case when `mo_absname == "C:"':
 * - enumerate files from "C:\" on windows
 *
 * If the directory no longer exists for some reason,
 * return "Dee_EmptyTuple" instead */

#undef DeeModule_GetDirectory_USE_FindFirstFileExW
#undef DeeModule_GetDirectory_USE_GetLogicalDrives
#undef DeeModule_GetDirectory_USE_opendir
#undef DeeModule_GetDirectory_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define DeeModule_GetDirectory_USE_FindFirstFileExW
#define DeeModule_GetDirectory_USE_GetLogicalDrives
/* TODO: Add another option to implement using `_findfirst()' */
#elif defined(CONFIG_HAVE_opendir) && (defined(CONFIG_HAVE_readdir) || defined(CONFIG_HAVE_readdir64))
#define DeeModule_GetDirectory_USE_opendir
#else /* ... */
#define DeeModule_GetDirectory_USE_STUB
#endif /* !... */


#ifdef DeeModule_GetDirectory_USE_FindFirstFileExW
#ifndef CONFIG_HAVE_wcslen
#define CONFIG_HAVE_wcslen
#undef wcslen
#define wcslen dee_wcslen
DeeSystem_DEFINE_wcslen(dee_wcslen)
#endif /* !CONFIG_HAVE_wcslen */
#endif /* DeeModule_GetDirectory_USE_FindFirstFileExW */


#ifdef DeeModule_GetDirectory_USE_FindFirstFileExW
PRIVATE ATTR_PURE WUNUSED bool DCALL
fs_wstr_endswith_str(LPCWSTR wstr, size_t wstr_length,
                     char const *ext, size_t ext_len) {
	if (wstr_length < ext_len)
		return false;
	wstr += wstr_length;
	wstr -= ext_len;
	while (*ext) {
		unsigned char ch = (unsigned char)*ext++;
		WCHAR wch = *wstr++;
		if (wch == (WCHAR)ch)
			continue;
#ifdef DeeSystem_HAVE_FS_ICASE
		if (wch == (WCHAR)(unsigned int)tolower(ch))
			continue;
		if (wch == (WCHAR)(unsigned int)toupper(ch))
			continue;
#endif /* DeeSystem_HAVE_FS_ICASE */
		return false;
	}
	return true;
}
#endif /* DeeModule_GetDirectory_USE_FindFirstFileExW */

#ifdef DeeModule_GetDirectory_USE_opendir
#ifdef CONFIG_HAVE_struct_dirent_d_namlen
#define dirent_namelen(x) (x)->d_namlen
#elif defined(_D_EXACT_NAMLEN)
#define dirent_namelen(x) _D_EXACT_NAMLEN(x)
#else /* ... */
#define dirent_namelen(x) strlen((x)->d_name)
#endif /* !... */
#endif /* DeeModule_GetDirectory_USE_opendir */


/* TODO: This function is good and all, but won't be usable to implement `enumattr(import)'!
 *       In order to implement that one, there needs to be a function to iteratively produce
 *       viable module items from deemon lib paths one-at-a-time (if this function was used,
 *       then the modules of each lib path would have to be loaded all-at-once, rather than
 *       loading them lazily as the enumattr-iterator is enumerated)
 */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_GetDirectory_uncached_impl(struct Dee_tuple_builder *__restrict builder,
                                     char const *__restrict absname) {
#ifdef DeeModule_GetDirectory_USE_GetLogicalDrives
	if (!*absname) {
		DWORD dwDrives = GetLogicalDrives();
		uint8_t letter = 'a';
		while (dwDrives) {
			if (dwDrives & 1) {
				DREF DeeObject *drive = DeeString_Chr(letter);
				if unlikely(!drive)
					goto err;
				if unlikely(Dee_tuple_builder_append_inherited(builder, drive) < 0)
					goto err;
#define NEED_err
			}
			dwDrives >>= 1;
			++letter;
		}
		return 0;
	}
#endif /* DeeModule_GetDirectory_USE_GetLogicalDrives */

#ifdef DeeModule_GetDirectory_USE_FindFirstFileExW
	{
		HANDLE hFind;
		WIN32_FIND_DATAW fdData;
		LPWSTR wildcard, dst;
		size_t absname_length = strlen(absname);
		uint32_t ch;
		ASSERT(!absname_length || absname[absname_length - 1] != DeeSystem_SEP);
		/* Enough space for: f"\\\\.\\{absname}\\*\0" */
		wildcard = (LPWSTR)Dee_Mallocac(4 + absname_length + 3, sizeof(WCHAR));
		if unlikely(!wildcard)
			goto err;
#define NEED_err
		dst = wildcard;
		*dst++ = (WCHAR)'\\';
		*dst++ = (WCHAR)'\\';
		*dst++ = (WCHAR)'.';
		*dst++ = (WCHAR)'\\';
		while ((ch = unicode_readutf8(&absname)) != 0) {
			/* TODO: encode utf-32 character as utf-16 */
			*dst++ = (WCHAR)ch;
		}
		*dst++ = (WCHAR)'\\';
		*dst++ = (WCHAR)'*';
		*dst = (WCHAR)'\0';

		DBG_ALIGNMENT_DISABLE();
		hFind = FindFirstFileExW(wildcard, FindExInfoBasic, &fdData,
		                         FindExSearchNameMatch, NULL, 0);
		if likely(hFind != INVALID_HANDLE_VALUE) {
			do {
				size_t i, szFileName = wcslen(fdData.cFileName);
				DBG_ALIGNMENT_ENABLE();
				if (fdData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					/* Allow directories... */
				} else if (fdData.dwFileAttributes & (/*FILE_ATTRIBUTE_REPARSE_POINT | */FILE_ATTRIBUTE_DEVICE)) {
					/* Do not allow device files (but do allow symlinks) */
					goto next_file;
				} else if (fs_wstr_endswith_str(fdData.cFileName, szFileName, ".dee", 4)) {
					/* Deemon script -> allow */
					szFileName -= 4;
#ifndef CONFIG_NO_DEX
				} else if (fs_wstr_endswith_str(fdData.cFileName, szFileName,
				                                DeeSystem_SOEXT,
				                                COMPILER_STRLEN(DeeSystem_SOEXT))) {
					/* Potential dex module -> allow */
					szFileName -= COMPILER_STRLEN(DeeSystem_SOEXT);
#endif /* !CONFIG_NO_DEX */
				} else {
					goto next_file;
				}

				/* Assert that "fdData.cFileName" does not contain any "." characters */
				for (i = 0; i < szFileName; ++i) {
					if (fdData.cFileName[i] == (WCHAR)'.')
						goto next_file;
				}

				/* Append file to builder. */
				{
					DREF DeeObject *string;
					string = DeeString_NewWide((Dee_wchar_t const *)fdData.cFileName,
					                           szFileName, STRING_ERROR_FIGNORE);
					if unlikely(!string)
						goto err_find;
					if unlikely(Dee_tuple_builder_append_inherited(builder, string) < 0)
						goto err;
				}

next_file:
				DBG_ALIGNMENT_DISABLE();
			} while (FindNextFileW(hFind, &fdData));
			(void)FindClose(hFind);
		}
		DBG_ALIGNMENT_ENABLE();
		Dee_Freea(wildcard);
		return 0;
err_find:
		DBG_ALIGNMENT_DISABLE();
		(void)FindClose(hFind);
		DBG_ALIGNMENT_ENABLE();
		Dee_Freea(wildcard);
		return -1;
	}
#endif /* DeeModule_GetDirectory_USE_FindFirstFileExW */

#ifdef DeeModule_GetDirectory_USE_opendir
	/* TODO */
	(void)builder;
	(void)absname;
	return 0;
#endif /* DeeModule_GetDirectory_USE_opendir */

#ifdef DeeModule_GetDirectory_USE_STUB
	(void)builder;
	(void)absname;
	return 0;
#endif /* DeeModule_GetDirectory_USE_STUB */

#ifdef NEED_err
#undef NEED_err
err:
	return -1;
#endif /* NEED_err */
}

#ifndef CONFIG_HAVE_qsort
#define CONFIG_HAVE_qsort
#define qsort   dee_qsort
DeeSystem_DEFINE_qsort(dee_qsort)
#endif /* !CONFIG_HAVE_qsort */

#ifndef __LIBCCALL
#define __LIBCCALL /* nothing */
#endif /* !__LIBCCALL */

#ifdef DeeSystem_HAVE_FS_ICASE
#ifndef CONFIG_HAVE_strcasecmp
#define CONFIG_HAVE_strcasecmp
#undef strcasecmp
#define strcasecmp dee_strcasecmp
DeeSystem_DEFINE_strcasecmp(dee_strcasecmp)
#endif /* !CONFIG_HAVE_strcasecmp */
#define fs_strcmp(a, b) strcasecmp(a, b)
#else /* DeeSystem_HAVE_FS_ICASE */
#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */
#define fs_strcmp(a, b) strcmp(a, b)
#endif /* !DeeSystem_HAVE_FS_ICASE */


PRIVATE int __LIBCCALL
compare_strings_by_utf8(void const *lhs_ptr, void const *rhs_ptr) {
	DeeStringObject *lhs = *(DeeStringObject **)lhs_ptr;
	DeeStringObject *rhs = *(DeeStringObject **)rhs_ptr;
	char const *lhs_utf8 = DeeString_AsUtf8((DeeObject *)lhs);
	char const *rhs_utf8 = DeeString_AsUtf8((DeeObject *)rhs);
	ASSERT(lhs_utf8);
	ASSERT(rhs_utf8);
	return fs_strcmp(lhs_utf8, rhs_utf8);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_tuple_builder_sort_and_make_unique(struct Dee_tuple_builder *__restrict self) {
	size_t i;
	DeeTupleObject *tuple = self->tb_tuple;
	if unlikely(!self->tb_size)
		goto done;

	/* Ensure that the UTF-8 reprs of all strings have been cached. */
	for (i = 0; i < self->tb_size; ++i) {
		char const *utf8;
		DeeStringObject *item;
		item = (DeeStringObject *)tuple->t_elem[i];
		utf8 = DeeString_AsUtf8((DeeObject *)item);
		if unlikely(!utf8)
			goto err;
	}

	/* Sort items (needed because modules expect their child directories to be sorted
	 * in order to allow use of binary search for checking if a specific child exists) */
	qsort(tuple->t_elem, self->tb_size, sizeof(DREF DeeObject *), &compare_strings_by_utf8);

	/* Remove duplicate items... */
	for (i = 0; (i + 1) < self->tb_size;) {
		DeeStringObject *a = (DeeStringObject *)tuple->t_elem[i + 0];
		DeeStringObject *b = (DeeStringObject *)tuple->t_elem[i + 1];
		if (DeeString_EqualsSTR(a, b)) {
			--self->tb_size;
			memmovedownc(&tuple->t_elem[i + 1], &tuple->t_elem[i + 2],
			             self->tb_size - (i + 1), sizeof(DREF DeeObject *));
			Dee_Decref(b);
		} else {
			++i;
		}
	}

done:
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
DeeModule_GetDirectory_uncached(DeeModuleObject *__restrict self) {
	struct Dee_tuple_builder builder;

	/* Special handling for anonymous- and file-based modules (e.g. `import("./readme.txt")') */
	if (self->mo_absname == NULL)
		goto return_empty;
	if (self->mo_flags & Dee_MODULE_FABSFILE) {
		if (Dee_TYPE(self) != &DeeModuleDir_Type)
			goto return_empty;
	}

	/* Use builder-pattern */
	Dee_tuple_builder_init(&builder);
	if unlikely(DeeModule_GetDirectory_uncached_impl(&builder, self->mo_absname))
		goto err_builder;
	if unlikely(Dee_tuple_builder_sort_and_make_unique(&builder))
		goto err_builder;
	return (DREF DeeTupleObject *)Dee_tuple_builder_pack(&builder);
err_builder:
	Dee_tuple_builder_fini(&builder);
	return NULL;
return_empty:
	return (DREF DeeTupleObject *)DeeTuple_NewEmpty();
}

/* Return the directory view of a given module, that is: the (lexicographically
 * sorted) list of ["*.dee", "*.so", "*.dll", DT_DIR]-files within a directory
 * (that don't contain any '.' characters) and matching the module's filename
 * with *its* trailing "*.dee"/"*.so"/"*.dll" removed (or just the directory
 * itself if that exists, but no associated source file / DEX module does).
 * If no such directory exists, "Dee_EmptyTuple" is returned.
 *
 * @return: * : The module's directory file list.
 * @return: NULL: An error was thrown */
PUBLIC WUNUSED NONNULL((1)) /*Tuple*/ DeeObject *DCALL
DeeModule_GetDirectory(DeeModuleObject *__restrict self) {
	/*Tuple*/ DeeTupleObject *result;
again:
	result = atomic_read(&self->mo_dir);
	if (result == NULL) {
		result = DeeModule_GetDirectory_uncached(self);
		if unlikely(!atomic_cmpxch(&self->mo_dir, NULL, result)) {
			Dee_Decref_likely(result);
			goto again;
		}
	}
	return (DeeObject *)result;
}

DECL_END
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

#endif /* !GUARD_DEEMON_EXECUTE_MODPATH_DIRECTORY_C */
