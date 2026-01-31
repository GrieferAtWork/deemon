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
#ifndef GUARD_DEEMON_EXECUTE_MODPATH_DIRECTORY_C
#define GUARD_DEEMON_EXECUTE_MODPATH_DIRECTORY_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_Free, Dee_Mallocc, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S */
#include <deemon/arg.h>             /* DeeArg_UnpackStruct2, DeeArg_UnpackStructKw */
#include <deemon/callable.h>        /* DeeCallable_Type */
#include <deemon/module.h>          /* DeeModule*, Dee_MODULE_FABSFILE */
#include <deemon/object.h>
#include <deemon/string.h>          /* DeeString*, Dee_wchar_t, STRING_ERROR_FIGNORE */
#include <deemon/stringutils.h>     /* Dee_unicode_readutf8 */
#include <deemon/system-features.h> /* CONFIG_HAVE_*, DeeSystem_DEFINE_*, memmovedownc, strlen, to(lower|upper) */
#include <deemon/system.h>          /* DeeSystem_* */
#include <deemon/tuple.h>           /* DeeTupleObject, DeeTuple_NewEmpty, Dee_tuple_builder* */
#include <deemon/util/atomic.h>     /* atomic_cmpxch, atomic_read */

#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */

#include "../runtime/kwlist.h"
#include "../runtime/strings.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint8_t, uint16_t, uint32_t */

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

#define UTF16_LOW_SURROGATE_MIN  0xdc00
#define UTF16_HIGH_SURROGATE_MIN 0xd800
#define UTF16_SURROGATE_SHIFT    0x10000

PRIVATE WUNUSED uint16_t *DCALL 
Dee_unicode_writeutf16(/*utf-16*/ uint16_t *__restrict dst, uint32_t ch) {
	if likely(ch <= 0xffff && (ch < 0xd800 || ch > 0xdfff)) {
		*dst++ = (uint16_t)ch;
	} else {
		ch -= UTF16_SURROGATE_SHIFT;
		*dst++ = UTF16_HIGH_SURROGATE_MIN + (uint16_t)(ch >> 10);
		*dst++ = UTF16_LOW_SURROGATE_MIN + (uint16_t)(ch & 0x3ff);
	}
	return dst;
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


struct module_dir_iterator {
	char const *mdi_absname; /* [1..1][const] Absolute pathname that is being iterated (s.a. `DeeModuleObject::mo_absname') */
	union {
#ifdef DeeModule_GetDirectory_USE_GetLogicalDrives
		struct {
			DWORD dwDrives/* = GetLogicalDrives()*/;
			uint8_t letter/* = 'a'*/;
		} mdid_fsroot; /* [valid_if(*mdi_absname == '\0')] */
#endif /* DeeModule_GetDirectory_USE_GetLogicalDrives */

#ifdef DeeModule_GetDirectory_USE_FindFirstFileExW
		struct {
			HANDLE hFind;
			WIN32_FIND_DATAW fdData;
			LPWSTR wildcard;
		} mdid_find; /* [valid_if(*mdi_absname != '\0')] */
#endif /* DeeModule_GetDirectory_USE_FindFirstFileExW */

	} mdi_data; /* Iterator-specific data */
};

/* @return: 1 : Failure (no error)
 * @return: 0 : Success
 * @return: -1: Error was thrown */
PRIVATE WUNUSED NONNULL((1)) int DCALL
module_dir_iterator_init(struct module_dir_iterator *__restrict self) {
	char const *absname = self->mdi_absname;
	ASSERT(absname);
#ifdef DeeModule_GetDirectory_USE_GetLogicalDrives
	if (!*absname) {
		self->mdi_data.mdid_fsroot.dwDrives = GetLogicalDrives();
		self->mdi_data.mdid_fsroot.letter   = 'a';
		return 0;
	}
#endif /* DeeModule_GetDirectory_USE_GetLogicalDrives */

#ifdef DeeModule_GetDirectory_USE_FindFirstFileExW
	{
		LPWSTR dst;
		size_t absname_length = strlen(absname);
		uint32_t ch;
		ASSERT(!absname_length || absname[absname_length - 1] != DeeSystem_SEP);
		/* Enough space for: f"\\\\.\\{absname}\\*\0" */
		self->mdi_data.mdid_find.wildcard = (LPWSTR)Dee_Mallocc(4 + absname_length + 3, sizeof(WCHAR));
		if unlikely(!self->mdi_data.mdid_find.wildcard)
			return -1;
		dst = self->mdi_data.mdid_find.wildcard;
		*dst++ = (WCHAR)'\\';
		*dst++ = (WCHAR)'\\';
		*dst++ = (WCHAR)'.';
		*dst++ = (WCHAR)'\\';
		while ((ch = Dee_unicode_readutf8(&absname)) != 0) {
			/* encode utf-32 character as utf-16 */
			dst = (LPWSTR)Dee_unicode_writeutf16((uint16_t *)dst, ch);
		}
		*dst++ = (WCHAR)'\\';
		*dst++ = (WCHAR)'*';
		*dst = (WCHAR)'\0';

		DBG_ALIGNMENT_DISABLE();
		self->mdi_data.mdid_find.hFind = FindFirstFileExW(self->mdi_data.mdid_find.wildcard,
		                                                  FindExInfoBasic,
		                                                  &self->mdi_data.mdid_find.fdData,
		                                                  FindExSearchNameMatch, NULL, 0);
		DBG_ALIGNMENT_ENABLE();
		if likely(self->mdi_data.mdid_find.hFind == INVALID_HANDLE_VALUE) {
			Dee_Free(self->mdi_data.mdid_find.wildcard);
			return 1;
		}
		return 0;
	}
#endif /* DeeModule_GetDirectory_USE_FindFirstFileExW */

#ifdef DeeModule_GetDirectory_USE_opendir
	/* TODO */
	(void)self;
	(void)absname;
	return 1;
#endif /* DeeModule_GetDirectory_USE_opendir */

#ifdef DeeModule_GetDirectory_USE_STUB
	(void)self;
	(void)absname;
	return 1;
#endif /* DeeModule_GetDirectory_USE_STUB */
}

PRIVATE NONNULL((1)) void DCALL
module_dir_iterator_fini(struct module_dir_iterator *__restrict self) {
#ifdef DeeModule_GetDirectory_USE_GetLogicalDrives
	if (!*self->mdi_absname)
		return;
#endif /* DeeModule_GetDirectory_USE_GetLogicalDrives */

#ifdef DeeModule_GetDirectory_USE_FindFirstFileExW
	if likely(self->mdi_data.mdid_find.hFind != INVALID_HANDLE_VALUE) {
		DBG_ALIGNMENT_DISABLE();
		FindClose(self->mdi_data.mdid_find.hFind);
		DBG_ALIGNMENT_ENABLE();
	}
	Dee_Free(self->mdi_data.mdid_find.wildcard);
#endif /* DeeModule_GetDirectory_USE_FindFirstFileExW */

#ifdef DeeModule_GetDirectory_USE_opendir
	/* TODO */
	(void)self;
#endif /* DeeModule_GetDirectory_USE_opendir */

#ifdef DeeModule_GetDirectory_USE_STUB
	(void)self;
#endif /* DeeModule_GetDirectory_USE_STUB */
}

/* Yield next module directory item
 * @return: * :        Next item
 * @return: ITER_DONE: End-of-directory reached
 * @return: NULL:      Error was thrown */
PRIVATE WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
module_dir_iterator_next(struct module_dir_iterator *__restrict self) {
#ifdef DeeModule_GetDirectory_USE_GetLogicalDrives
	if (!*self->mdi_absname) {
		while (self->mdi_data.mdid_fsroot.dwDrives) {
			if (self->mdi_data.mdid_fsroot.dwDrives & 1) {
				DREF DeeObject *drive = DeeString_Chr(self->mdi_data.mdid_fsroot.letter);
				if unlikely(!drive)
					goto err;
#define NEED_err
				self->mdi_data.mdid_fsroot.dwDrives >>= 1;
				++self->mdi_data.mdid_fsroot.letter;
				return drive;
			}
			self->mdi_data.mdid_fsroot.dwDrives >>= 1;
			++self->mdi_data.mdid_fsroot.letter;
		}
		return ITER_DONE;
	}
#endif /* DeeModule_GetDirectory_USE_GetLogicalDrives */

#ifdef DeeModule_GetDirectory_USE_FindFirstFileExW
	if likely(self->mdi_data.mdid_find.hFind != INVALID_HANDLE_VALUE) {
		do {
			size_t i, szFileName = wcslen(self->mdi_data.mdid_find.fdData.cFileName);
			DBG_ALIGNMENT_ENABLE();
			if (self->mdi_data.mdid_find.fdData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				/* Allow directories... */
			} else if (self->mdi_data.mdid_find.fdData.dwFileAttributes & (/*FILE_ATTRIBUTE_REPARSE_POINT | */FILE_ATTRIBUTE_DEVICE)) {
				/* Do not allow device files (but do allow symlinks) */
				goto next_file;
			} else if (fs_wstr_endswith_str(self->mdi_data.mdid_find.fdData.cFileName,
			                                szFileName, ".dee", 4)) {
				/* Deemon script -> allow */
				szFileName -= 4;
#ifndef CONFIG_NO_DEX
			} else if (fs_wstr_endswith_str(self->mdi_data.mdid_find.fdData.cFileName,
			                                szFileName, DeeSystem_SOEXT,
			                                COMPILER_STRLEN(DeeSystem_SOEXT))) {
				/* Potential dex module -> allow */
				szFileName -= COMPILER_STRLEN(DeeSystem_SOEXT);
#endif /* !CONFIG_NO_DEX */
			} else {
				goto next_file;
			}

			/* Assert that "self->mdi_data.mdid_find.fdData.cFileName"
			 * does not contain any "." characters */
			for (i = 0; i < szFileName; ++i) {
				if (self->mdi_data.mdid_find.fdData.cFileName[i] == (WCHAR)'.')
					goto next_file;
			}

			/* Append file to builder. */
			{
				DREF DeeObject *string;
				string = DeeString_NewWide((Dee_wchar_t const *)self->mdi_data.mdid_find.fdData.cFileName,
				                           szFileName, STRING_ERROR_FIGNORE);
				if unlikely(!string)
					goto err;
#define NEED_err
				DBG_ALIGNMENT_DISABLE();
				if (!FindNextFileW(self->mdi_data.mdid_find.hFind,
				                   &self->mdi_data.mdid_find.fdData)) {
					(void)FindClose(self->mdi_data.mdid_find.hFind);
					self->mdi_data.mdid_find.hFind = INVALID_HANDLE_VALUE;
				}
				DBG_ALIGNMENT_ENABLE();
				return string;
			}

next_file:
			DBG_ALIGNMENT_DISABLE();
		} while (FindNextFileW(self->mdi_data.mdid_find.hFind, &self->mdi_data.mdid_find.fdData));
		(void)FindClose(self->mdi_data.mdid_find.hFind);
		self->mdi_data.mdid_find.hFind = INVALID_HANDLE_VALUE;
		DBG_ALIGNMENT_ENABLE();
	}
	return ITER_DONE;
#endif /* DeeModule_GetDirectory_USE_FindFirstFileExW */

#ifdef DeeModule_GetDirectory_USE_opendir
	/* TODO */
	(void)self;
	return ITER_DONE;
#endif /* DeeModule_GetDirectory_USE_opendir */

#ifdef DeeModule_GetDirectory_USE_STUB
	(void)self;
	return ITER_DONE;
#endif /* DeeModule_GetDirectory_USE_STUB */

#ifdef NEED_err
#undef NEED_err
err:
	return NULL;
#endif /* NEED_err */
}




PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
DeeModule_GetDirectory_uncached_impl(struct Dee_tuple_builder *__restrict builder,
                                     char const *__restrict absname) {
	int init_status;
	DREF DeeObject *item;
	struct module_dir_iterator iter;

	/* Initialize iterator */
	iter.mdi_absname = absname;
	init_status = module_dir_iterator_init(&iter);
	if unlikely(init_status != 0) {
		if unlikely(init_status < 0)
			goto err;
		return 0;
	}

	/* Enumerate all items and append them to "builder" */
	while (ITER_ISOK(item = module_dir_iterator_next(&iter))) {
		if unlikely(Dee_tuple_builder_append_inherited(builder, item) < 0)
			goto err_iter;
	}
	if unlikely(!item)
		goto err_iter;

	/* Cleanup */
	module_dir_iterator_fini(&iter);
	return 0;
err_iter:
	module_dir_iterator_fini(&iter);
err:
	return -1;
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


/* TODO: Functions to iteratively enumerate the modules
 *       from elements of "DeeModule_GetLibPath()" */


INTDEF WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
import_getattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
import_getattr_string_hash(DeeObject *self, char const *attr, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeModuleObject *DCALL
import_getattr_string_len_hash(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL
import_boundattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
import_boundattr_string_hash(DeeObject *self, char const *attr, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
import_boundattr_string_len_hash(DeeObject *self, char const *attr, size_t attrlen, Dee_hash_t hash);

#define import_hasattr                 import_boundattr
#define import_hasattr_string_hash     import_boundattr_string_hash
#define import_hasattr_string_len_hash import_boundattr_string_len_hash

PRIVATE struct type_attr import_attr = {
	/* .tp_getattr                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&import_getattr,
	/* .tp_delattr                   = */ NULL,
	/* .tp_setattr                   = */ NULL,
	/* .tp_iterattr                  = */ NULL, // TODO: &import_iterattr,
	/* .tp_findattr                  = */ NULL, // TODO: &import_findattr,
	/* .tp_hasattr                   = */ &import_hasattr,
	/* .tp_boundattr                 = */ &import_boundattr,
	/* .tp_callattr                  = */ NULL,
	/* .tp_callattr_kw               = */ NULL,
	/* .tp_vcallattrf                = */ NULL,
	/* .tp_getattr_string_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&import_getattr_string_hash,
	/* .tp_delattr_string_hash       = */ NULL,
	/* .tp_setattr_string_hash       = */ NULL,
	/* .tp_hasattr_string_hash       = */ &import_hasattr_string_hash,
	/* .tp_boundattr_string_hash     = */ &import_boundattr_string_hash,
	/* .tp_callattr_string_hash      = */ NULL,
	/* .tp_callattr_string_hash_kw   = */ NULL,
	/* .tp_vcallattr_string_hashf    = */ NULL,
	/* .tp_getattr_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&import_getattr_string_len_hash,
	/* .tp_delattr_string_len_hash   = */ NULL,
	/* .tp_setattr_string_len_hash   = */ NULL,
	/* .tp_hasattr_string_len_hash   = */ &import_hasattr_string_len_hash,
	/* .tp_boundattr_string_len_hash = */ &import_boundattr_string_len_hash,
};


#define import_repr import_str
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
import_str(DeeObject *__restrict UNUSED(self)) {
	return_reference(&str_import);
}


PRIVATE WUNUSED ATTR_INS(3, 2) NONNULL((1)) DREF DeeModuleObject *DCALL
import_call(DeeObject *UNUSED(self), size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("import", params: """
	DeeObject *base:?X4?DModule?Dstring?DType?N;
	DeeStringObject *name;
""");]]]*/
	struct {
		DeeObject *base;
		DeeStringObject *name;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "import", &args, &args.base, &args.name);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.name, &DeeString_Type))
		goto err;
	return DeeModule_Import(Dee_AsObject(args.name), args.base,
	                        DeeModule_IMPORT_F_NORMAL);
err:
	return NULL;
}

PRIVATE WUNUSED ATTR_INS(3, 2) NONNULL((1)) DREF DeeModuleObject *DCALL
import_call_kw(DeeObject *UNUSED(self), size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("import", params: """
	DeeObject *base:?X4?DModule?Dstring?DType?N;
	DeeStringObject *name;
""", docStringPrefix: "import");]]]*/
#define import_import_params "base:?X4?DModule?Dstring?DType?N,name:?Dstring"
	struct {
		DeeObject *base;
		DeeStringObject *name;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__base_name, "oo:import", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.name, &DeeString_Type))
		goto err;
	return DeeModule_Import(Dee_AsObject(args.name), args.base,
	                        DeeModule_IMPORT_F_NORMAL);
err:
	return NULL;
}

PRIVATE struct type_callable import_callable = {
	/* .tp_call_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&import_call_kw,
};

PUBLIC DeeTypeObject DeeBuiltin_ImportType = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ STR_import,
	/* .tp_doc      = */ DOC("Runtime part of the magic compiler builtin $import keyword. "
	                         /**/ "This object's ?#{op:call} implements ${import(\"foo\")}, "
	                         /**/ "and its ?#{op:getattr} implements ${import.foo}\n"
	                         "\n"
	                         "getattr->?DModule\n"
	                         "\n"
	                         "call(" import_import_params ")->?DModule"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FFINAL | TP_FNAMEOBJECT | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* Static singleton, so no serial needed */
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ &import_str,
		/* .tp_repr      = */ &import_repr,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ NULL,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &DeeObject_GenericCmpByAddr,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ &import_attr,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&import_call,
	/* .tp_callable      = */ &import_callable,
};

PUBLIC DeeObject DeeBuiltin_Import = {
	OBJECT_HEAD_INIT(&DeeBuiltin_ImportType)
};

DECL_END
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

#endif /* !GUARD_DEEMON_EXECUTE_MODPATH_DIRECTORY_C */
