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
#ifndef GUARD_DEX_POSIX_P_PATH_C_INL
#define GUARD_DEX_POSIX_P_PATH_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/stringutils.h>
#include <deemon/system.h>

#include <hybrid/minmax.h>

/**/
#include "p-pwd.c.inl"
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

#ifndef CONFIG_HAVE_memrchr
#define CONFIG_HAVE_memrchr
#undef memrchr
#define memrchr dee_memrchr
DeeSystem_DEFINE_memrchr(dee_memrchr)
#endif /* !CONFIG_HAVE_memrchr */

/************************************************************************/
/* Path utilities (originally from `fs')                                */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL posix_path_headof_f(DeeStringObject *__restrict path);
PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL posix_path_tailof_f(DeeStringObject *__restrict path);
PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL posix_path_driveof_f(DeeStringObject *__restrict path);
PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL posix_path_inctrail_f(DeeStringObject *__restrict path);
PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL posix_path_exctrail_f(DeeStringObject *__restrict path);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeStringObject *DCALL posix_path_walklink_f(DeeStringObject *link, DeeStringObject *linkname);
PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL posix_path_abspath_f(DeeStringObject *path, DeeStringObject *pwd);
PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL posix_path_relpath_f(DeeStringObject *path, DeeStringObject *pwd);
PRIVATE WUNUSED ATTR_INS(2, 1) DREF DeeStringObject *DCALL posix_path_joinpath_f(size_t pathc, DeeStringObject *const *__restrict pathv);
PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL posix_path_normalpath_f(DeeStringObject *__restrict path);



/************************************************************************/
/* Implementation                                                       */
/************************************************************************/
#define DeeString_IsAbsPath(self) DeeSystem_IsAbs(DeeString_STR(self))

PRIVATE DEFINE_STRING(str_single_dot, ".");
PRIVATE DEFINE_STRING(posix_FS_DELIM, DeeSystem_DELIM_S);
PRIVATE DEFINE_STRING(posix_FS_SEP, DeeSystem_SEP_S);
#ifdef DeeSystem_ALTSEP_S
PRIVATE DEFINE_STRING(posix_FS_ALTSEP, DeeSystem_ALTSEP_S);
#else /* DeeSystem_ALTSEP_S */
#define posix_FS_ALTSEP posix_FS_SEP
#endif /* !DeeSystem_ALTSEP_S */


PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
posix_path_headof_f(DeeStringObject *__restrict path) {
	char const *tailsep, *pathstr;
	ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
	pathstr = DeeString_AsUtf8((DREF DeeObject *)path);
	if unlikely(!pathstr)
		goto err;
	tailsep = DeeSystem_BaseName(pathstr, WSTR_LENGTH(pathstr));
	ASSERT(tailsep >= pathstr);
	if (tailsep <= pathstr)
		return_reference_((DeeStringObject *)Dee_EmptyString);
	/* Note that we *do* include the trailing slash! */
	return (DREF DeeStringObject *)DeeString_NewUtf8(pathstr,
	                                                 (size_t)(tailsep - pathstr),
	                                                 STRING_ERROR_FIGNORE);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
posix_path_tailof_f(DeeStringObject *__restrict path) {
	char const *tailsep, *pathstr;
	ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
	pathstr = DeeString_AsUtf8((DeeObject *)path);
	if unlikely(!pathstr)
		goto err;
	tailsep = DeeSystem_BaseName(pathstr, WSTR_LENGTH(pathstr));
	ASSERT(tailsep >= pathstr);
	if (tailsep <= pathstr)
		return_reference_(path);
	return (DREF DeeStringObject *)DeeString_NewUtf8(tailsep,
	                                                 (size_t)((pathstr + WSTR_LENGTH(pathstr)) - tailsep),
	                                                 STRING_ERROR_FIGNORE);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
posix_path_driveof_f(DeeStringObject *__restrict path) {
#ifdef DEE_SYSTEM_FS_DRIVES
	char const *pathstr, *iter, *end;
	DREF DeeStringObject *result;
	ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
	pathstr = DeeString_AsUtf8((DeeObject *)path);
	if unlikely(!pathstr)
		goto err;
	iter = pathstr;
	end  = pathstr + WSTR_LENGTH(pathstr);
	for (; iter < end; ++iter) {
		char *dst;
		size_t drive_length;
		if (DeeSystem_IsSep(*iter))
			break; /* Stop on the first slash. */
		if (*iter != ':')
			continue; /* Found the drive character. */
		++iter;
		drive_length = (size_t)(iter - pathstr);
		result = (DREF DeeStringObject *)DeeString_NewBuffer(drive_length + 1);
		if unlikely(!result)
			goto err;
		dst = DeeString_GetBuffer(result);
		dst = (char *)mempcpyc(dst, pathstr, drive_length, sizeof(char));
		/* Always follow up with a slash. */
#ifdef DeeSystem_ALTSEP
		*dst = *iter == DeeSystem_ALTSEP ? DeeSystem_ALTSEP : DeeSystem_SEP;
#else /* DeeSystem_ALTSEP */
		*dst = DeeSystem_SEP;
#endif /* !DeeSystem_ALTSEP */
		return (DREF DeeStringObject *)DeeString_SetUtf8((DREF DeeObject *)result, STRING_ERROR_FIGNORE);
	}
	return_reference_((DeeStringObject *)Dee_EmptyString);
err:
	return NULL;
#else /* DEE_SYSTEM_FS_DRIVES */
	ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
	(void)path;
	return_reference_((DeeStringObject *)&posix_FS_SEP);
#endif /* !DEE_SYSTEM_FS_DRIVES */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
posix_path_inctrail_f(DeeStringObject *__restrict path) {
	uint32_t endch;
	size_t wlength;
	ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
	wlength = DeeString_WLEN(path);
	if unlikely(!wlength)
		return_reference_((DeeStringObject *)&posix_FS_SEP);
	endch = DeeString_GetChar(path, wlength - 1);
	if (DeeSystem_IsSep(endch))
		return_reference_(path);

#ifdef DeeSystem_ALTSEP
	/* If the string contains an instances of the alt-sep,
	 * but no instance of the primary sep, then we must
	 * append the alt-sep instead!
	 *
	 * For this, we can always just search DeeString_STR. */
	if (memchr(DeeString_STR(path), DeeSystem_ALTSEP, DeeString_SIZE(path) * sizeof(char)) != NULL &&
	    memchr(DeeString_STR(path), DeeSystem_SEP, DeeString_SIZE(path) * sizeof(char)) == NULL)
		return (DREF DeeStringObject *)DeeObject_Add((DeeObject *)path, (DeeObject *)&posix_FS_ALTSEP);
#endif /* DeeSystem_ALTSEP */
	return (DREF DeeStringObject *)DeeObject_Add((DeeObject *)path, (DeeObject *)&posix_FS_SEP);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
posix_path_exctrail_f(DeeStringObject *__restrict path) {
	int width;
	size_t wlength, new_wlength;
	ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
	wlength = DeeString_WLEN(path);
	new_wlength = wlength;
	while (new_wlength) {
		uint32_t endch;
		endch = DeeString_GetChar(path, new_wlength - 1);
		if (!DeeSystem_IsSep(endch))
			break;
		--new_wlength;
	}
	ASSERT(new_wlength <= wlength);
	if (new_wlength >= wlength)
		return_reference_(path);
	width = DeeString_WIDTH(path);
	return (DREF DeeStringObject *)DeeString_NewWithWidth(DeeString_WSTR(path),
	                                                      new_wlength, width);
}


/* Given 2 text pointer, return a pointer to the start of
 * the latest path segment, or re-return `pth_begin' if only
 * one, or zero segments exist:
 * >> "/foo/bar/foobar/"
 *     ^        ^      ^
 *     pth_begin|      pth_end
 *              |
 *              +- return
 * >> "/foo/bar/foobar/./"
 *     ^        ^        ^
 *     pth_begin|        pth_end
 *              |
 *              +- return
 * >> "/foo/bar/fiz/baz/../"
 *     ^        ^          ^
 *     pth_begin|          pth_end
 *              |
 *              +- return
 */
PRIVATE WUNUSED NONNULL((1, 2)) /*utf-8*/ char const *DCALL
find_last_path_segment(/*utf-8*/ char const *pth_begin,
                       /*utf-8*/ char const *pth_end) {
	char const *next;
	uint32_t ch;
	int name_state;
	size_t count = 0;
again:
	name_state = 0;
	for (;;) {
		if (pth_begin >= pth_end)
			goto done;
		next = pth_end;
		ch   = unicode_readutf8_rev_n(&next, pth_begin);
		if (!DeeSystem_IsSep(ch) && !DeeUni_IsSpace(ch))
			break;
		pth_end = next;
	}

	/* Search for the next DeeSystem_SEP and unroll `pth_end' to point directly after it. */
	for (;;) {
		if (pth_begin >= pth_end)
			goto done;
		next = pth_end;
		/* TODO: special handling for unwinding `.' and `..' segments. */
		ch = unicode_readutf8_rev_n(&next, pth_begin);
		if (DeeSystem_IsSep(ch))
			break;
		if (ch == '.') {
			if (name_state == 0) {
				name_state = 1; /* Self-directory reference. */
			} else if (name_state == 1) {
				name_state = 2; /* Self-directory reference. */
			} else {
				name_state = -1; /* Not a special folder */
			}
		} else if (name_state >= 0) {
			if (!DeeUni_IsSpace(ch)) {
				name_state = -1; /* Not a special folder */
			} else {
				if (name_state == 1) {
					name_state = 3; /* Self-directory reference (hard). */
				} else if (name_state == 2) {
					name_state = 4; /* Parent-directory reference (hard). */
				}
			}
		}
		pth_end = next;
	}
	switch (name_state) {

	case 2:
	case 4:
		/* Parent directory reference. */
		++count;
		goto again;

	case 1:
	case 3:
		/* Self-directory reference. */
		goto again;

	default: break;
	}
	if (count) {
		--count;
		goto again;
	}
done:
	return pth_end;
}



#ifdef CONFIG_HOST_WINDOWS
/* Check if `path_str...+=path_len' is a special NT filename */
PRIVATE WUNUSED NONNULL((1)) bool DCALL
posix_path_is_nt_special(char const *path_str, size_t path_len) {
	switch (path_len) {
#ifdef DEE_SYSTEM_FS_ICASE
#define eqfscase(a, b) ((a) == (b) || (a) == ((b) - ('A' - 'a')))
#else /* DEE_SYSTEM_FS_ICASE */
#define eqfscase(a, b) ((a) == (b))
#endif /* !DEE_SYSTEM_FS_ICASE */

	case 3:
		if (eqfscase(path_str[0], 'N') && eqfscase(path_str[1], 'U') && eqfscase(path_str[2], 'L'))
			goto is_special; /* NUL */
		if (eqfscase(path_str[0], 'C') && eqfscase(path_str[1], 'O') && eqfscase(path_str[2], 'N'))
			goto is_special; /* CON */
		if (eqfscase(path_str[0], 'P') && eqfscase(path_str[1], 'R') && eqfscase(path_str[2], 'N'))
			goto is_special; /* PRN */
		if (eqfscase(path_str[0], 'A') && eqfscase(path_str[1], 'U') && eqfscase(path_str[2], 'X'))
			goto is_special; /* AUX */
		break;

	case 4:
		/* COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9,
		 * LPT1, LPT2, LPT3, LPT4, LPT5, LPT6, LPT7, LPT8, LPT9 */
		if (path_str[3] >= '1' && path_str[3] <= '9' &&
		    ((eqfscase(path_str[0], 'C') && eqfscase(path_str[1], 'O') && eqfscase(path_str[1], 'M')) ||
		     (eqfscase(path_str[0], 'L') && eqfscase(path_str[1], 'P') && eqfscase(path_str[1], 'T'))))
			goto is_special;
		break;

	case 6:
		if (eqfscase(path_str[3], 'I') && eqfscase(path_str[4], 'N') && path_str[5] == '$') {
			if (eqfscase(path_str[0], 'C') && eqfscase(path_str[1], 'O') && eqfscase(path_str[2], 'N'))
				goto is_special; /* CONIN$ */
#ifdef CONFIG_WANT_WINDOWS_STD_FILES
			if (eqfscase(path_str[0], 'S') && eqfscase(path_str[1], 'T') && eqfscase(path_str[2], 'D'))
				goto is_special; /* STDIN$ */
#endif /* CONFIG_WANT_WINDOWS_STD_FILES */
		}
		break;

	case 7:
		if (eqfscase(path_str[3], 'O') && eqfscase(path_str[4], 'U') &&
		    eqfscase(path_str[5], 'T') && path_str[6] == '$') {
			if (eqfscase(path_str[0], 'C') && eqfscase(path_str[1], 'O') && eqfscase(path_str[2], 'N'))
				goto is_special; /* CONOUT$ */
#ifdef CONFIG_WANT_WINDOWS_STD_FILES
			if (eqfscase(path_str[0], 'S') && eqfscase(path_str[1], 'T') && eqfscase(path_str[2], 'D'))
				goto is_special; /* STDOUT$ */
#endif /* CONFIG_WANT_WINDOWS_STD_FILES */
		}
#ifdef CONFIG_WANT_WINDOWS_STD_FILES
		if (eqfscase(path_str[0], 'S') && eqfscase(path_str[1], 'T') && eqfscase(path_str[2], 'D') &&
		    eqfscase(path_str[3], 'E') && eqfscase(path_str[4], 'R') && eqfscase(path_str[5], 'R') &&
		    path_str[6] == '$')
			goto is_special; /* STDERR$ */
#endif /* CONFIG_WANT_WINDOWS_STD_FILES */
		break;

#undef eqfscase
	default:
		break;
	}
	return false;
is_special:
	return true;
}
#endif /* CONFIG_HOST_WINDOWS */


/* Similar to `posix_path_abspath_f()', don't try to get rid of `..' elements,
 * and use `posix_path_headof_f(linkname)' when `link' isn't an absolute path:
 * >> if (isabs(link))
 * >>     return link;
 * >> linkname = headof(linkname);
 * >> while (link.startswith("." + FS_SEP))
 * >>     link = link[#("." + FS_SEP):];
 * >> return headof(linkname) + link; */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeStringObject *DCALL
posix_path_walklink_f(DeeStringObject *link, DeeStringObject *linkname) {
	DREF DeeStringObject *result;
	char const *link_base, *link_start, *link_end;
	char const *name_base, *name_end;

	/* Check for simple case: `link' is absolute */
	if (DeeString_IsAbsPath(link))
		goto return_link;

#ifdef CONFIG_HOST_WINDOWS
	/* Don't modify special filenames such as `CON' or `NUL' */
	if (posix_path_is_nt_special(DeeString_STR(link),
	                             DeeString_SIZE(link)))
		goto return_link;
#endif /* CONFIG_HOST_WINDOWS */

	/* Load `linkname' as UTF-8 and check where the containing directory ends. */
	name_base = DeeString_AsUtf8((DeeObject *)linkname);
	if unlikely(!name_base)
		goto err;
	name_end = DeeSystem_BaseName(name_base, WSTR_LENGTH(name_base));
	ASSERT(name_end >= name_base);
	if (name_end <= name_base) {
		/* The link name is empty, meaning the link text
		 * can just be used as-is and be PWD-relative */
		goto return_link;
	}

	/* Load the string as UTF-8 and strip leading `./' */
	link_base = DeeString_AsUtf8((DeeObject *)link);
	if unlikely(!link_base)
		goto err;
	link_start = link_base;
	link_end   = link_base + WSTR_LENGTH(link_base);
	while (link_start < link_end &&
	       link_end[0] == '.' &&
	       DeeSystem_IsSep(link_end[1]))
		link_start += 2;

	/* Return the string "{[name_base:name_end)}{[link_start:link_end)}" */
	{
		char *dst;
		size_t name_len = (size_t)(name_end - name_base);
		size_t link_len = (size_t)(link_end - link_start);
		result = (DREF DeeStringObject *)DeeString_NewBuffer(name_len + link_len);
		if unlikely(!result)
			goto err;
		dst = DeeString_GetBuffer(result);
		dst = (char *)mempcpyc(dst, name_base, name_len, sizeof(char));
		memcpyc(dst, link_base, link_len, sizeof(char));
	}

	/* Set the resulting string as UTF-8 */
	result = (DREF DeeStringObject *)DeeString_SetUtf8((DREF DeeObject *)result,
	                                                   STRING_ERROR_FIGNORE);
	return result;
return_link:
	return_reference_(link);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
posix_path_abspath_f(DeeStringObject *path, DeeStringObject *pwd) {
	DREF DeeStringObject *result;
	ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT_OPT(pwd, &DeeString_Type);

	/* Quick check: If the given path is already absolute,
	 *              then we've got nothing to do. */
	if (DeeString_IsAbsPath(path)) {
		if (!pwd) {
#ifdef CONFIG_HOST_WINDOWS
return_unmodified:
#endif /* CONFIG_HOST_WINDOWS */
			return_reference_(path);
		}

		/* If a custom PWD is given, then we have to do this double-callback. */
		path = posix_path_relpath_f(path, NULL);
		if unlikely(!path)
			goto err;
		ASSERT(!DeeString_IsAbsPath(path));
		result = posix_path_abspath_f(path, pwd);
		Dee_Decref(path);
		return result;
	}

#ifdef CONFIG_HOST_WINDOWS
	/* Don't modify special filenames such as `CON' or `NUL' */
	if (posix_path_is_nt_special(DeeString_STR(path),
	                             DeeString_SIZE(path)))
		goto return_unmodified;
#endif /* CONFIG_HOST_WINDOWS */

	/* If the given `pwd' isn't absolute, make it using the real PWD. */
	if (pwd && !DeeString_IsAbsPath(pwd)) {
		pwd = posix_path_abspath_f(pwd, NULL);
		if unlikely(!pwd)
			goto err;
	} else if (pwd) {
		Dee_Incref(pwd);
	} else {
		pwd = (DeeStringObject *)posix_getcwd_f_impl();
		if unlikely(!pwd)
			goto err;
	}
	{
		uint32_t ch;
		char const *next;
		char const *pwd_base, *pwd_begin, *pwd_end;
		char const *pth_base, *pth_begin, *pth_end;
		pwd_base = DeeString_AsUtf8((DeeObject *)pwd);
		if unlikely(!pwd_base)
			goto err_pwd;
		pth_base = DeeString_AsUtf8((DeeObject *)path);
		if unlikely(!pth_base)
			goto err_pwd;
		pwd_end = (pwd_begin = pwd_base) + WSTR_LENGTH(pwd_base);
		pth_end = (pth_begin = pth_base) + WSTR_LENGTH(pth_base);

		/* Trim the given PWD and PATH strings. */
		while (pth_end > pth_begin) {
			next = pth_end;
			ch   = unicode_readutf8_rev_n(&next, pth_begin);
			if (!DeeUni_IsSpace(ch) && !DeeSystem_IsSep(ch))
				break;
			pth_end = next;
		}

		/* Special case: drive-relative paths. */
#ifdef DEE_SYSTEM_FS_DRIVES
		if (pth_begin < pth_end && DeeSystem_IsSep(*pth_begin)) {
			char *dst;
			size_t pth_len;
			size_t pwd_drive_len;
			char const *pwd_drive_end = pwd_begin;
			while (pwd_drive_end < pwd_end && !DeeSystem_IsSep(*pwd_drive_end))
				++pwd_drive_end;
			pwd_drive_len = (size_t)(pwd_drive_end - pwd_begin);
			pth_len       = (size_t)(pth_end - pth_begin);
			result = (DREF DeeStringObject *)DeeString_NewBuffer(pwd_drive_len + pth_len);
			if unlikely(!result)
				goto err_pwd;
			dst = DeeString_GetBuffer(result);
			dst = (char *)mempcpyc(dst, pwd_begin, pwd_drive_len, sizeof(char));
			memcpyc(dst, pth_begin, pth_len, sizeof(char));
			result = (DREF DeeStringObject *)DeeString_SetUtf8((DeeObject *)result, STRING_ERROR_FIGNORE);
			goto done_decref_pwd;
		}
#endif /* DEE_SYSTEM_FS_DRIVES */

		while (pth_begin < pth_end) {
			next = pth_begin;
			ch   = unicode_readutf8_n(&next, pth_end);
			if (!DeeUni_IsSpace(ch) && !DeeSystem_IsSep(ch))
				break;
			pth_begin = next;
		}
again_trip_paths:
		while (pwd_end > pwd_begin) {
			next = pwd_end;
			ch   = unicode_readutf8_rev_n(&next, pwd_begin);
			if (!DeeUni_IsSpace(ch) && !DeeSystem_IsSep(ch))
				break;
			pwd_end = next;
		}

		/* Check for leading parent-/current-folder references in `pth_begin' */
		if (*pth_begin == '.') {
			bool is_parent_ref;
			next          = pth_begin + 1;
			is_parent_ref = *next == '.';
			if (is_parent_ref)
				++next;

			/* Check if this segment really only contains 1/2 dots. */
			while (next < pth_end) {
				ch = unicode_readutf8_n(&next, pth_end);
				if (DeeSystem_IsSep(ch))
					break;
				if (!DeeUni_IsSpace(ch))
					goto done_merge_paths;
			}

			/* This is a special-reference segment! (skip all additional slashes/spaces) */
			pth_begin = next;
			while (pth_begin < pth_end) {
				next = pth_begin;
				ch   = unicode_readutf8_n(&next, pth_end);
				if (!DeeSystem_IsSep(ch) && !DeeUni_IsSpace(ch))
					break;
				pth_begin = next;
			}
			if (is_parent_ref) {
				/* Must strip a trailing path segment from `pwd_begin...pwd_end' */
				pwd_end = find_last_path_segment(pwd_begin, pwd_end);
			}
			goto again_trip_paths;
		}

done_merge_paths:
		/* Special optimizations when one part wasn't used at all.
		 * Also: Special handling when one of the 2 paths has gotten empty! */
		if (pwd_begin >= pwd_end) {
			if (pth_begin == pth_base &&
			    pth_end == pth_base + WSTR_LENGTH(pth_base)) {
				result = path;
				Dee_Incref(result);
			} else {
				result = (DREF DeeStringObject *)DeeString_NewUtf8(pth_begin, pth_end - pth_begin,
				                                                   STRING_ERROR_FIGNORE);
			}
		} else if (pth_begin >= pth_end) {
			if (pwd_begin == pwd_base &&
			    pwd_end == pwd_base + WSTR_LENGTH(pwd_base)) {
				result = pwd;
				Dee_Incref(result);
			} else {
				result = (DREF DeeStringObject *)DeeString_NewUtf8(pwd_begin, pwd_end - pwd_begin,
				                                                   STRING_ERROR_FIGNORE);
			}
		} else {
			/* Create the result buffer. */
			char *dst;
			size_t pth_length = (size_t)(pth_end - pth_begin);
			size_t pwd_length = (size_t)(pwd_end - pwd_begin);
			result = (DREF DeeStringObject *)DeeString_NewBuffer(pwd_length + 1 + pth_length);
			if unlikely(!result)
				goto err_pwd;
			dst = DeeString_GetBuffer(result);
			dst = (char *)mempcpyc(dst, pwd_begin, pwd_length, sizeof(char));
			*dst++ = DeeSystem_SEP;
			memcpyc(dst, pth_begin, pth_length, sizeof(char));
			result = (DREF DeeStringObject *)DeeString_SetUtf8((DREF DeeObject *)result, STRING_ERROR_FIGNORE);
		}
	}
#ifdef DEE_SYSTEM_FS_DRIVES
done_decref_pwd:
#endif /* DEE_SYSTEM_FS_DRIVES */
	Dee_Decref(pwd);
	return result;
err_pwd:
	Dee_Decref(pwd);
err:
	return NULL;
}


#define MAX_UPREF_COPY 4
PRIVATE char const aligned_upref_buffer[MAX_UPREF_COPY][3] = {
	{ '.', '.', DeeSystem_SEP },
	{ '.', '.', DeeSystem_SEP },
	{ '.', '.', DeeSystem_SEP },
	{ '.', '.', DeeSystem_SEP }
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
posix_path_relpath_f(DeeStringObject *path, DeeStringObject *pwd) {
	DREF DeeStringObject *result;
	size_t uprefs, pth_length;
	char *dst;
	char const *next;
	char const *pth_begin, *pth_iter, *pth_end;
	char const *pwd_begin, *pwd_iter, *pwd_end;
	uint32_t a, b;
#ifdef DEE_SYSTEM_FS_DRIVES
	char const *pth_base;
#endif /* DEE_SYSTEM_FS_DRIVES */
	bool is_nonempty_segment;
	ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);

	/* Quick check: If the given path isn't absolute,
	 *              then we've got nothing to do. */
	if (!DeeString_IsAbsPath(path)) {
		if (!pwd) {
#ifdef DEE_SYSTEM_FS_DRIVES
			/* If the given path is drive-relative, then we still want to
			 * turn it into a normal (PWD-relative) path:
			 * >> chdir(r"D:\path\to\my\folder");
			 * >> print relpath(r"\foo.txt"); // r"..\..\..\..\foo.txt"
			 *
			 * Also: without this check, we'd have different behavior for:
			 * >> print relpath(r"\foo.txt");      // r"\foo.txt"
			 * >> print relpath(r"\foo.txt", "."); // r"..\..\..\..\foo.txt"
			 */
			if (!DeeSystem_IsSep(DeeString_STR(path)[0]))
#endif /* DEE_SYSTEM_FS_DRIVES */
			{
				return_reference_(path);
			}
		}

		/* If a custom PWD is given, then we have to do this double-callback. */
		path = posix_path_abspath_f(path, NULL);
		if unlikely(!path)
			goto err;
		ASSERT(DeeString_IsAbsPath(path));
		/* Now that the path is absolute, re-invoke the relative path creator. */
		result = posix_path_relpath_f(path, pwd);
		Dee_Decref(path);
		return result;
	}

	/* If the given `pwd' isn't absolute, make it using the real PWD. */
	if (pwd && !DeeString_IsAbsPath(pwd)) {
		pwd = posix_path_abspath_f(pwd, NULL);
		if unlikely(!pwd)
			goto err;
	} else if (pwd) {
		Dee_Incref(pwd);
	} else {
		/* Lookup the real PWD. */
		pwd = (DeeStringObject *)posix_getcwd_f_impl();
		if unlikely(!pwd)
			goto err;
	}
	pth_iter = DeeString_AsUtf8((DeeObject *)path);
	if unlikely(!pth_iter)
		goto err;
	pwd_iter = DeeString_AsUtf8((DeeObject *)pwd);
	if unlikely(!pwd_iter)
		goto err;
	pth_end = pth_iter + WSTR_LENGTH(pth_iter);
	pwd_end = pwd_iter + WSTR_LENGTH(pwd_iter);

	/* Match the drive prefix. */
#ifdef DEE_SYSTEM_FS_DRIVES
	for (;;) {
		a = unicode_readutf8_n(&pth_iter, pth_end);
		b = unicode_readutf8_n(&pwd_iter, pwd_end);
		if ((a != b) && (DeeUni_ToUpper(a) != DeeUni_ToUpper(b))) {
			/* Different drives (return the given path as-is) */
			result = path;
			Dee_Incref(result);
			goto done;
		}

		/* Stop when the path prefix is found. */
		if likely(a == ':')
			break;

		/* This shouldn't really happen, but we've got no guaranty that it can't. */
		if unlikely(!a)
			goto return_single_dot;
	}
	pth_base = pth_iter;
#endif /* DEE_SYSTEM_FS_DRIVES */

	/* Jump to start here, so that we automatically
	 * skip leading space and slashes. */
	goto continue_after_sep;

	for (;;) {
		a = unicode_readutf8_n(&pth_iter, pth_end);
		b = unicode_readutf8_n(&pwd_iter, pwd_end);
#ifdef DEE_SYSTEM_FS_ICASE
		a = DeeUni_ToUpper(a);
		b = DeeUni_ToUpper(b);
#endif /* DEE_SYSTEM_FS_ICASE */
		if (DeeSystem_IsSep(a)) {
			/* Align differing space in `b' */
			while (DeeUni_IsSpace(b)) {
				b = unicode_readutf8_n(&pwd_iter, pwd_end);
			}
			if (!DeeSystem_IsSep(b)) {
				if (!b && pwd_iter >= pwd_end) {
					/* End of the pwd-string when the path-string is at a slash
					 * Setup the non matching path portions:
					 *  - path: everything after the current slash
					 *  - pwd:  empty */
					pwd_begin = pwd_end;
					for (;;) {
						next = pth_iter;
						a    = unicode_readutf8_n(&next, pth_end);
						if (!DeeSystem_IsSep(a) && !DeeUni_IsSpace(a))
							break;
						pth_iter = next;
					}
					pth_begin = pth_iter;
				}
				break;
			}
continue_after_sep:
			while (pth_iter < pth_end) {
				next = pth_iter;
				a    = unicode_readutf8_n(&next, pth_end);
				if (!DeeUni_IsSpace(a) && !DeeSystem_IsSep(a))
					break;
				pth_iter = next;
			}
			while (pwd_iter < pwd_end) {
				next = pwd_iter;
				b    = unicode_readutf8_n(&next, pwd_end);
				if (!DeeUni_IsSpace(b) && !DeeSystem_IsSep(b))
					break;
				pwd_iter = next;
			}
continue_after_sep_sp:
			/* Keep track the last shared folder. */
			pth_begin = pth_iter;
			pwd_begin = pwd_iter;
			continue;
		}
		if (DeeSystem_IsSep(b)) {
			/* Align differing space in `a' */
			while (DeeUni_IsSpace(a))
				a = unicode_readutf8_n(&pth_iter, pth_end);
			if (!DeeSystem_IsSep(a)) {
				if (!a && pth_iter >= pth_end) {
					/* End of the path-string when the pwd-string is at a slash
					 * Setup the non matching path portions:
					 *  - path: empty
					 *  - pwd:  everything after the current slash */
					pth_begin = pth_end;
					for (;;) {
						next = pwd_iter;
						b    = unicode_readutf8_n(&next, pwd_end);
						if (!DeeSystem_IsSep(b) && !DeeUni_IsSpace(b))
							break;
						pwd_iter = next;
					}
					pwd_begin = pwd_iter;
				}
				break;
			}
			goto continue_after_sep;
		}
		if (a != b) {
			if (DeeUni_IsSpace(a) && DeeUni_IsSpace(b)) {
				/* Special handling for differing space characters surrounding slashes. */
				while (pth_iter < pth_end) {
					next = pth_iter;
					a    = unicode_readutf8_n(&next, pth_end);
					if (!DeeUni_IsSpace(a) && !DeeSystem_IsSep(a))
						break;
					pth_iter = next;
				}
				while (pwd_iter < pwd_end) {
					next = pwd_iter;
					b    = unicode_readutf8_n(&next, pwd_end);
					if (!DeeUni_IsSpace(b) && !DeeSystem_IsSep(b))
						break;
					pwd_iter = next;
				}

				/* If a slash follows on both sides, continue normally. */
				if (DeeSystem_IsSep(a) && DeeSystem_IsSep(b))
					goto continue_after_sep_sp;
			}

			/* Special handling for `relpath("foo/bar", "foo/bar/")' */
			if (!a && pth_iter >= pth_end) {
				next = pwd_iter;
				while (next < pwd_end) {
					b = unicode_readutf8_n(&next, pwd_end);
					if (!DeeUni_IsSpace(b) && !DeeSystem_IsSep(b))
						break;
				}
				if (next >= pwd_end)
					goto return_single_dot;
			}

			/* Special handling for `relpath("foo/bar/", "foo/bar")' */
			if (!b && pwd_iter >= pwd_end) {
				next = pth_iter;
				while (next < pth_end) {
					a = unicode_readutf8_n(&next, pth_end);
					if (!DeeUni_IsSpace(a) && !DeeSystem_IsSep(a))
						break;
				}
				if (next >= pth_end)
					goto return_single_dot;
			}
			break;
		}

		/* NOTE: When `a' is NUL, we also know that `b' is NUL
		 *       because `a != b' breaks out of the loop, so we
		 *       wouldn't get here if `a' didn't equal `b'. */
		if (!a && (pth_iter >= pth_end ||
		           pwd_iter >= pwd_end)) {
			/* If both paths are now empty, then they were equal from the get-go. */
			if (pth_iter >= pth_end && pwd_iter >= pwd_end)
				goto return_single_dot;
			break;
		}
	}

	/* Count the amount of folders remaining in 'cwd'
	 * >> Depending on it's about, we have to add
	 *    additional `..DeeSystem_SEP' prefixes to the resulting path. */
	uprefs              = 0;
	is_nonempty_segment = false;
continue_uprefs_normal:
	while (pwd_begin < pwd_end) {
		b = unicode_readutf8_n(&pwd_begin, pwd_end);
		if (!DeeSystem_IsSep(b) || (!b && pwd_begin >= pwd_end)) {
			bool is_parent_ref;

			/* Deal with trailing `/././.'-like and `/../../..'-like paths! */
			if (b != '.') {
				is_nonempty_segment = true;
				continue;
			}
			is_nonempty_segment = false;
			b                   = unicode_readutf8_n(&pwd_begin, pwd_end);
			is_parent_ref       = b == '.';
			if (is_parent_ref)
				b = unicode_readutf8_n(&pwd_begin, pwd_end);
			while (!DeeSystem_IsSep(b)) {
				if (!DeeUni_IsSpace(b))
					goto continue_uprefs_normal;
				b = unicode_readutf8_n(&pwd_begin, pwd_end);
			}

			/* Got a self/parent directory reference. */
			if (is_parent_ref) {
				/* Simple case: undo an upwards reference. */
				if (uprefs) {
					--uprefs;
				} else {
					/* relpath("E:/c/dexmon/deemon", "E:/c/dexmon/../../d/unrelated");
					 * RESULT: "../../c/dexmon/deemon"
					 *                [][-----]
					 * To implement this, we must retroactively search for the last
					 * sep in the given `path' string, and revert `pth_begin' to be
					 * located directly past its position.
					 * The two brackets denote the portions of the input `path' that
					 * had to be retrieved retroactively. */
#ifndef DEE_SYSTEM_FS_DRIVES
					char const *pth_base;
					pth_base = DeeString_AsUtf8((DeeObject *)path);
					if unlikely(!pth_base)
						goto err;
#endif /* !DEE_SYSTEM_FS_DRIVES */

					/* Skip trailing slash/space characters that had been skipped previously. */
					pth_begin = find_last_path_segment(pth_base, pth_begin);
				}
			}

			/* Skip all additional space and DeeSystem_SEP-characters. */
			for (;;) {
				next = pwd_begin;
				b    = unicode_readutf8_n(&next, pwd_end);
				if (!DeeSystem_IsSep(b) && !DeeUni_IsSpace(b))
					break;
				pwd_begin = next;
			}
			continue;
		}
		++uprefs;
		is_nonempty_segment = false;
		do {
			b = unicode_readutf8_n(&pwd_begin, pwd_end);
		} while (DeeSystem_IsSep(b) || DeeUni_IsSpace(b));
	}
	if (is_nonempty_segment)
		++uprefs;

#if 1 /* Small, optional memory-reuse optimization */
	if (pth_begin == DeeString_STR(path) && !uprefs)
		return_reference_(path);
#endif

	/* Strip leading slashes & whitespace from `path' */
	while (pth_begin < pth_end) {
		next = pth_begin;
		a    = unicode_readutf8_n(&next, pth_end);
		if (!DeeSystem_IsSep(a) && !DeeUni_IsSpace(a))
			break;
		pth_begin = next;
	}

	/* Strip trailing whitespace from `path' */
	while (pth_end > pth_begin) {
		next = pth_end;
		a    = unicode_readutf8_rev_n(&next, pth_begin);
		if (!DeeUni_IsSpace(a))
			break;
		pth_end = next;
	}
	if (!uprefs && pth_iter >= pth_end) {
		/* Special case: The 2 given paths match each other exactly. */
return_single_dot:
		result = (DREF DeeStringObject *)&str_single_dot;
		Dee_Incref(result);
		goto done;
	}

	/* Create the string that'll be returned. */
	pth_length = (size_t)(pth_end - pth_begin);
	result = (DREF DeeStringObject *)DeeString_NewBuffer(uprefs * COMPILER_LENOF(aligned_upref_buffer[0]) +
	                                                     pth_length);
	if unlikely(!result)
		goto err_pwd;
	dst = DeeString_GetBuffer(result);
	while (uprefs) {
		size_t part = MIN(uprefs, (size_t)MAX_UPREF_COPY);
		dst = (char *)mempcpyc(dst, (void *)aligned_upref_buffer,
		                       part * COMPILER_LENOF(aligned_upref_buffer[0]),
		                       sizeof(char));
		uprefs -= part;
	}

	/* With upwards references out of the way, copy the remainder of the given path. */
	memcpyc(dst, pth_begin, pth_length, sizeof(char));
	result = (DREF DeeStringObject *)DeeString_SetUtf8((DREF DeeObject *)result,
	                                                   STRING_ERROR_FSTRICT);
done:
	Dee_Decref(pwd);
	return result;
err_pwd:
	Dee_Decref(pwd);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
posix_path_normalpath_f(DeeStringObject *__restrict path) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	/*utf-8*/ char const *iter, *begin, *end, *iter_next;
	/*utf-8*/ char const *flush_start, *flush_end;
	uint32_t ch;
	ASSERT_OBJECT_TYPE_EXACT(path, &DeeString_Type);
	begin = DeeString_AsUtf8((DeeObject *)path);
	if unlikely(!begin)
		goto err;
	end = begin + WSTR_LENGTH(begin);
	ASSERT(*end == '\0');
	flush_start = iter = begin;
next:
	ch = *iter++;
	switch (ch) {
		/* NOTE: The following part has been mirrored in `DeeSystem_MakeAbsolute'
		 * If a bug is found in this code, it should be fixed here, as well as
		 * within the core. */

#if defined(DeeSystem_ALTSEP) && DeeSystem_ALTSEP != DeeSystem_SEP
	case DeeSystem_ALTSEP:
#endif /* DeeSystem_ALTSEP && DeeSystem_ALTSEP != DeeSystem_SEP */
	case DeeSystem_SEP:
	case '\0': {
		char const *sep_loc;
		bool did_print_sep;

		/* Delete slashes and expand paths. */
		sep_loc = flush_end = iter - 1;

		/* Skip multiple slashes and whitespace following a path separator. */
		while (iter < end) {
			iter_next = iter;
			ch        = unicode_readutf8_n(&iter_next, end);
			if (!DeeUni_IsSpace(ch) && !DeeSystem_IsSep(ch))
				break;
			iter = iter_next;
		}
		while (flush_end > flush_start) {
			iter_next = flush_end;
			ch        = unicode_readutf8_rev_n(&iter_next, flush_start);
			if (!DeeUni_IsSpace(ch))
				break;
			flush_end = iter_next;
		}

		/* Analyze the last path portion for being a special name (`.' or `..') */
		if (flush_end[-1] == '.') {
			if (flush_end[-2] == '.' && flush_end - 2 == flush_start) {
				/* Parent-directory-reference. */
				size_t printer_length, new_end;

				/* Delete the last directory that was written. */
				printer_length = UNICODE_PRINTER_LENGTH(&printer);
				if (!printer_length)
					goto do_flush_after_sep;
				if (UNICODE_PRINTER_GETCHAR(&printer, printer_length - 1) == DeeSystem_SEP)
					--printer_length;
				new_end = unicode_printer_memrchr(&printer, DeeSystem_SEP, 0, printer_length);
				if (new_end == (size_t)-1) {
					/* Special handling to trim a path segment at the very start of the given path. */
					if (printer_length == 0)
						goto do_flush_after_sep;
					new_end = 0;
				} else {
					++new_end;
				}

				/* Truncate the valid length of the printer to after the previous slash. */
				unicode_printer_truncate(&printer, new_end);
				goto done_flush;
			} else if (flush_end[-3] == DeeSystem_SEP) {
				/* Parent-directory-reference. */
				char const *new_end;
				ASSERT((flush_end - 3) >= flush_start);
				new_end = (char const *)memrchr(flush_start, DeeSystem_SEP,
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
#ifdef DeeSystem_ALTSEP
		    && (*sep_loc == DeeSystem_SEP || iter == end + 1)
#endif /* DeeSystem_ALTSEP */
		    ) {
			goto done_flush_nostart;
		}

		/* If we can already include a slash in this part, do so. */
		did_print_sep = false;
		if (sep_loc == flush_end
#ifdef DeeSystem_ALTSEP
		    && (*sep_loc == DeeSystem_SEP)
#endif /* DeeSystem_ALTSEP */
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
		if (did_print_sep) {
			/* The slash has already been been printed: `foo/ bar' */
		} else if (sep_loc == iter - 1
#ifdef DeeSystem_ALTSEP
		           && (!*sep_loc || *sep_loc == DeeSystem_SEP)
#endif /* DeeSystem_ALTSEP */
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
		if (UNICODE_PRINTER_ISEMPTY(&printer)) {
			unicode_printer_fini(&printer);
			return_reference_(path);
		}

		/* Actually print the remainder. */
		if (unicode_printer_print(&printer, flush_start,
		                          (size_t)(iter - flush_start)) < 0)
			goto err;
	}

	/* Pack everything together. */
	return (DREF DeeStringObject *)unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED ATTR_INS(2, 1) DREF DeeStringObject *DCALL
posix_path_joinpath_f(size_t pathc, DeeStringObject *const *__restrict pathv) {
	size_t i;
	char nextsep = DeeSystem_SEP;
	struct unicode_printer printer;

	/* Special case: Return `.' when no paths are given. */
	if unlikely(!pathc)
		return_reference_((DeeStringObject *)&str_single_dot);
	unicode_printer_init(&printer);
	for (i = 0; i < pathc; ++i) {
		DeeStringObject *path = pathv[i];
		char const *begin, *end, *next;
		uint32_t ch;

		/* Validate that the path is actually a string. */
		if (DeeObject_AssertTypeExact(path, &DeeString_Type))
			goto err;
		begin = DeeString_AsUtf8((DeeObject *)path);
		if unlikely(!begin)
			goto err;
		end = begin + WSTR_LENGTH(begin);
		while (begin < end) {
			next = begin;
			ch   = unicode_readutf8_n(&next, end);
			if (!DeeUni_IsSpace(ch) && /* Don't skip leading SEPs */
			    (UNICODE_PRINTER_ISEMPTY(&printer) || !DeeSystem_IsSep(ch)))
				break;
			begin = next;
		}
		while (end > begin) {
			next = end;
			ch   = unicode_readutf8_rev_n(&next, begin);
			if (!DeeUni_IsSpace(ch) && !DeeSystem_IsSep(ch))
				break;
			end = next;
		}
		if (!UNICODE_PRINTER_ISEMPTY(&printer)) {
			/* Not the first non-empty path (figure out how, and print a separator). */
			if (DeeSystem_IsSep(begin[-1])) {
				--begin; /* Re-use this separator. */
			} else {
				/* Manually print a separator. */
				if (unicode_printer_putascii(&printer, nextsep))
					goto err;
			}
		}
		if (unicode_printer_print(&printer, begin, (size_t)(end - begin)) < 0)
			goto err;

		/* Set the separator that should be preferred for the next part. */
		nextsep = end[0];
		if (!DeeSystem_IsSep(nextsep))
			nextsep = DeeSystem_SEP;
	}
	return (DREF DeeStringObject *)unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}


/************************************************************************/
/* High-level bindings                                                  */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("headof", "path:?Dstring->?Dstring", libname: "posix", isconst: true);]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_headof_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_headof_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_HEADOF_DEF { "headof", (DeeObject *)&posix_headof, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring") },
#define POSIX_HEADOF_DEF_DOC(doc) { "headof", (DeeObject *)&posix_headof, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_headof, &posix_headof_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
#ifndef DEFINED_kwlist__path
#define DEFINED_kwlist__path
PRIVATE DEFINE_KWLIST(kwlist__path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_headof_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path, "o:headof", &args))
		goto err;
	return posix_headof_f_impl(args.path);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_headof_f_impl(DeeObject *path)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	return (DREF DeeObject *)posix_path_headof_f((DeeStringObject *)path);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("tailof", "path:?Dstring->?Dstring", libname: "posix", isconst: true);]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_tailof_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_tailof_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_TAILOF_DEF { "tailof", (DeeObject *)&posix_tailof, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring") },
#define POSIX_TAILOF_DEF_DOC(doc) { "tailof", (DeeObject *)&posix_tailof, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_tailof, &posix_tailof_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
#ifndef DEFINED_kwlist__path
#define DEFINED_kwlist__path
PRIVATE DEFINE_KWLIST(kwlist__path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_tailof_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path, "o:tailof", &args))
		goto err;
	return posix_tailof_f_impl(args.path);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_tailof_f_impl(DeeObject *path)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	return (DREF DeeObject *)posix_path_tailof_f((DeeStringObject *)path);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("driveof", "path:?Dstring->?Dstring", libname: "posix", isconst: true);]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_driveof_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_driveof_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_DRIVEOF_DEF { "driveof", (DeeObject *)&posix_driveof, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring") },
#define POSIX_DRIVEOF_DEF_DOC(doc) { "driveof", (DeeObject *)&posix_driveof, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_driveof, &posix_driveof_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
#ifndef DEFINED_kwlist__path
#define DEFINED_kwlist__path
PRIVATE DEFINE_KWLIST(kwlist__path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_driveof_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path, "o:driveof", &args))
		goto err;
	return posix_driveof_f_impl(args.path);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_driveof_f_impl(DeeObject *path)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	return (DREF DeeObject *)posix_path_driveof_f((DeeStringObject *)path);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("inctrail", "path:?Dstring->?Dstring", libname: "posix", isconst: true);]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_inctrail_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_inctrail_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_INCTRAIL_DEF { "inctrail", (DeeObject *)&posix_inctrail, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring") },
#define POSIX_INCTRAIL_DEF_DOC(doc) { "inctrail", (DeeObject *)&posix_inctrail, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_inctrail, &posix_inctrail_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
#ifndef DEFINED_kwlist__path
#define DEFINED_kwlist__path
PRIVATE DEFINE_KWLIST(kwlist__path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_inctrail_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path, "o:inctrail", &args))
		goto err;
	return posix_inctrail_f_impl(args.path);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_inctrail_f_impl(DeeObject *path)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	return (DREF DeeObject *)posix_path_inctrail_f((DeeStringObject *)path);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("exctrail", "path:?Dstring->?Dstring", libname: "posix", isconst: true);]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_exctrail_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_exctrail_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_EXCTRAIL_DEF { "exctrail", (DeeObject *)&posix_exctrail, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring") },
#define POSIX_EXCTRAIL_DEF_DOC(doc) { "exctrail", (DeeObject *)&posix_exctrail, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_exctrail, &posix_exctrail_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
#ifndef DEFINED_kwlist__path
#define DEFINED_kwlist__path
PRIVATE DEFINE_KWLIST(kwlist__path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_exctrail_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path, "o:exctrail", &args))
		goto err;
	return posix_exctrail_f_impl(args.path);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_exctrail_f_impl(DeeObject *path)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	return (DREF DeeObject *)posix_path_exctrail_f((DeeStringObject *)path);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("abspath", "path:?Dstring,pwd:?Dstring=NULL->?Dstring", libname: "posix", ispure: true);]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_abspath_f_impl(DeeObject *path, DeeObject *pwd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_abspath_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_ABSPATH_DEF { "abspath", (DeeObject *)&posix_abspath, MODSYM_FREADONLY, DOC("(path:?Dstring,pwd?:?Dstring)->?Dstring") },
#define POSIX_ABSPATH_DEF_DOC(doc) { "abspath", (DeeObject *)&posix_abspath, MODSYM_FREADONLY, DOC("(path:?Dstring,pwd?:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_abspath, &posix_abspath_f, METHOD_FPURECALL);
#ifndef DEFINED_kwlist__path_pwd
#define DEFINED_kwlist__path_pwd
PRIVATE DEFINE_KWLIST(kwlist__path_pwd, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("pwd", 0x33a2a198, 0xc64fc5785f58c2ce), KEND });
#endif /* !DEFINED_kwlist__path_pwd */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_abspath_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
		DeeObject *pwd;
	} args;
	args.pwd = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path_pwd, "o|o:abspath", &args))
		goto err;
	return posix_abspath_f_impl(args.path, args.pwd);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_abspath_f_impl(DeeObject *path, DeeObject *pwd)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	if (pwd && DeeObject_AssertTypeExact(pwd, &DeeString_Type))
		goto err;
	return (DREF DeeObject *)posix_path_abspath_f((DeeStringObject *)path,
	                                              (DeeStringObject *)pwd);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("walklink", "linktext:?Dstring,linkname:?Dstring->?Dstring", libname: "posix", isconst: true);]]]*/
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_walklink_f_impl(DeeObject *linktext, DeeObject *linkname);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_walklink_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_WALKLINK_DEF { "walklink", (DeeObject *)&posix_walklink, MODSYM_FREADONLY, DOC("(linktext:?Dstring,linkname:?Dstring)->?Dstring") },
#define POSIX_WALKLINK_DEF_DOC(doc) { "walklink", (DeeObject *)&posix_walklink, MODSYM_FREADONLY, DOC("(linktext:?Dstring,linkname:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_walklink, &posix_walklink_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
#ifndef DEFINED_kwlist__linktext_linkname
#define DEFINED_kwlist__linktext_linkname
PRIVATE DEFINE_KWLIST(kwlist__linktext_linkname, { KEX("linktext", 0x1a980550, 0xe7f93f503eadfdf7), KEX("linkname", 0x8fc35daf, 0xe202766a17bf6f0a), KEND });
#endif /* !DEFINED_kwlist__linktext_linkname */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_walklink_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *linktext;
		DeeObject *linkname;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__linktext_linkname, "oo:walklink", &args))
		goto err;
	return posix_walklink_f_impl(args.linktext, args.linkname);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1, 2))DREF DeeObject *DCALL posix_walklink_f_impl(DeeObject *linktext, DeeObject *linkname)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(linktext, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(linkname, &DeeString_Type))
		goto err;
	return (DREF DeeObject *)posix_path_walklink_f((DeeStringObject *)linktext,
	                                               (DeeStringObject *)linkname);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("relpath", "path:?Dstring,pwd:?Dstring=NULL->?Dstring", libname: "posix", ispure: true);]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_relpath_f_impl(DeeObject *path, DeeObject *pwd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_relpath_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_RELPATH_DEF { "relpath", (DeeObject *)&posix_relpath, MODSYM_FREADONLY, DOC("(path:?Dstring,pwd?:?Dstring)->?Dstring") },
#define POSIX_RELPATH_DEF_DOC(doc) { "relpath", (DeeObject *)&posix_relpath, MODSYM_FREADONLY, DOC("(path:?Dstring,pwd?:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_relpath, &posix_relpath_f, METHOD_FPURECALL);
#ifndef DEFINED_kwlist__path_pwd
#define DEFINED_kwlist__path_pwd
PRIVATE DEFINE_KWLIST(kwlist__path_pwd, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("pwd", 0x33a2a198, 0xc64fc5785f58c2ce), KEND });
#endif /* !DEFINED_kwlist__path_pwd */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_relpath_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
		DeeObject *pwd;
	} args;
	args.pwd = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path_pwd, "o|o:relpath", &args))
		goto err;
	return posix_relpath_f_impl(args.path, args.pwd);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_relpath_f_impl(DeeObject *path, DeeObject *pwd)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	if (pwd && DeeObject_AssertTypeExact(pwd, &DeeString_Type))
		goto err;
	return (DREF DeeObject *)posix_path_relpath_f((DeeStringObject *)path,
	                                              (DeeStringObject *)pwd);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("normalpath", "path:?Dstring->?Dstring", libname: "posix", isconst: true);]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_normalpath_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_normalpath_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_NORMALPATH_DEF { "normalpath", (DeeObject *)&posix_normalpath, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring") },
#define POSIX_NORMALPATH_DEF_DOC(doc) { "normalpath", (DeeObject *)&posix_normalpath, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_normalpath, &posix_normalpath_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
#ifndef DEFINED_kwlist__path
#define DEFINED_kwlist__path
PRIVATE DEFINE_KWLIST(kwlist__path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_normalpath_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path, "o:normalpath", &args))
		goto err;
	return posix_normalpath_f_impl(args.path);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_normalpath_f_impl(DeeObject *path)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	return (DREF DeeObject *)posix_path_normalpath_f((DeeStringObject *)path);
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("isabs", "path:?Dstring->?Dbool", libname: "posix", isconst: true);]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_isabs_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_isabs_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_ISABS_DEF { "isabs", (DeeObject *)&posix_isabs, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dbool") },
#define POSIX_ISABS_DEF_DOC(doc) { "isabs", (DeeObject *)&posix_isabs, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_isabs, &posix_isabs_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
#ifndef DEFINED_kwlist__path
#define DEFINED_kwlist__path
PRIVATE DEFINE_KWLIST(kwlist__path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_isabs_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path, "o:isabs", &args))
		goto err;
	return posix_isabs_f_impl(args.path);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_isabs_f_impl(DeeObject *path)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	return_bool(DeeString_IsAbsPath(path));
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("isrel", "path:?Dstring->?Dbool", libname: "posix", isconst: true);]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_isrel_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_isrel_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_ISREL_DEF { "isrel", (DeeObject *)&posix_isrel, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dbool") },
#define POSIX_ISREL_DEF_DOC(doc) { "isrel", (DeeObject *)&posix_isrel, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_isrel, &posix_isrel_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
#ifndef DEFINED_kwlist__path
#define DEFINED_kwlist__path
PRIVATE DEFINE_KWLIST(kwlist__path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_isrel_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path, "o:isrel", &args))
		goto err;
	return posix_isrel_f_impl(args.path);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_isrel_f_impl(DeeObject *path)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	return_bool(!DeeString_IsAbsPath(path));
err:
	return NULL;
}

/*[[[deemon import("rt.gen.dexutils").gw("issep", "path:?Dstring->?Dbool", libname: "posix", isconst: true);]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_issep_f_impl(DeeObject *path);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_issep_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_ISSEP_DEF { "issep", (DeeObject *)&posix_issep, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dbool") },
#define POSIX_ISSEP_DEF_DOC(doc) { "issep", (DeeObject *)&posix_issep, MODSYM_FREADONLY, DOC("(path:?Dstring)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_issep, &posix_issep_f, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST);
#ifndef DEFINED_kwlist__path
#define DEFINED_kwlist__path
PRIVATE DEFINE_KWLIST(kwlist__path, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEND });
#endif /* !DEFINED_kwlist__path */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_issep_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *path;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path, "o:issep", &args))
		goto err;
	return posix_issep_f_impl(args.path);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_issep_f_impl(DeeObject *path)
/*[[[end]]]*/
{
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	return_bool(DeeString_SIZE(path) == 1 &&
	            DeeSystem_IsSep(DeeString_STR(path)[0]));
err:
	return NULL;
}


#define POSIX_JOINPATH_DEF { "joinpath", (DeeObject *)&posix_joinpath, MODSYM_FNORMAL, DOC("(paths!:?Dstring)->?Dstring") },
#define POSIX_JOINPATH_DEF_DOC(doc) { "joinpath", (DeeObject *)&posix_joinpath, MODSYM_FNORMAL, DOC("(paths!:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_joinpath, &posix_path_joinpath_f, METHOD_FCONSTCALL);


/*[[[deemon
import PRIVATE_DEFINE_STRING from rt.gen.string;
print("#ifdef CONFIG_HOST_WINDOWS");
PRIVATE_DEFINE_STRING("posix_DEV_NULL", "NUL");
PRIVATE_DEFINE_STRING("posix_DEV_TTY", "CON");
print("#ifdef CONFIG_WANT_WINDOWS_STD_FILES");
PRIVATE_DEFINE_STRING("posix_DEV_STDIN", "STDIN$");
PRIVATE_DEFINE_STRING("posix_DEV_STDOUT", "STDOUT$");
PRIVATE_DEFINE_STRING("posix_DEV_STDERR", "STDERR$");
print("#else /" "* CONFIG_WANT_WINDOWS_STD_FILES *" "/");
print("/" "* Not ~really~ the same, but (might be) good enough... *" "/");
PRIVATE_DEFINE_STRING("posix_DEV_STDIN", "CONIN$");
PRIVATE_DEFINE_STRING("posix_DEV_STDOUT", "CONOUT$");
print("#define posix_DEV_STDERR posix_DEV_STDOUT");
print("#endif /" "* !CONFIG_WANT_WINDOWS_STD_FILES *" "/");
print("#else /" "* CONFIG_HOST_WINDOWS *" "/");
PRIVATE_DEFINE_STRING("posix_DEV_NULL", "/dev/null");
PRIVATE_DEFINE_STRING("posix_DEV_TTY", "/dev/tty");
PRIVATE_DEFINE_STRING("posix_DEV_STDIN", "/dev/stdin");
PRIVATE_DEFINE_STRING("posix_DEV_STDOUT", "/dev/stdout");
PRIVATE_DEFINE_STRING("posix_DEV_STDERR", "/dev/stderr");
print("#endif /" "* !CONFIG_HOST_WINDOWS *" "/");
]]]*/
#ifdef CONFIG_HOST_WINDOWS
PRIVATE DEFINE_STRING_EX(posix_DEV_NULL, "NUL", 0x297be5d1, 0x3d8f7ae6aa67df1c);
PRIVATE DEFINE_STRING_EX(posix_DEV_TTY, "CON", 0xb110e83f, 0xa73f81bd9988039c);
#ifdef CONFIG_WANT_WINDOWS_STD_FILES
PRIVATE DEFINE_STRING_EX(posix_DEV_STDIN, "STDIN$", 0xb0ec2c08, 0xbeb042cc01f0e59b);
PRIVATE DEFINE_STRING_EX(posix_DEV_STDOUT, "STDOUT$", 0x601e7682, 0x8c5aa4b421050b);
PRIVATE DEFINE_STRING_EX(posix_DEV_STDERR, "STDERR$", 0xa538f46f, 0xc334b8fd0ebabcc0);
#else /* CONFIG_WANT_WINDOWS_STD_FILES */
/* Not ~really~ the same, but (might be) good enough... */
PRIVATE DEFINE_STRING_EX(posix_DEV_STDIN, "CONIN$", 0xd8214f01, 0x1ecaafc7dbbd22f4);
PRIVATE DEFINE_STRING_EX(posix_DEV_STDOUT, "CONOUT$", 0x72267aa6, 0xa91f9d83a4b324db);
#define posix_DEV_STDERR posix_DEV_STDOUT
#endif /* !CONFIG_WANT_WINDOWS_STD_FILES */
#else /* CONFIG_HOST_WINDOWS */
PRIVATE DEFINE_STRING_EX(posix_DEV_NULL, "/dev/null", 0xd132f8f7, 0x6d8429be54d7865c);
PRIVATE DEFINE_STRING_EX(posix_DEV_TTY, "/dev/tty", 0x8e50730a, 0xc9b684e28c6b31f8);
PRIVATE DEFINE_STRING_EX(posix_DEV_STDIN, "/dev/stdin", 0x61dde285, 0xa2ebb4f0610cae9c);
PRIVATE DEFINE_STRING_EX(posix_DEV_STDOUT, "/dev/stdout", 0xe4a13278, 0x86a4a6cab4dca74);
PRIVATE DEFINE_STRING_EX(posix_DEV_STDERR, "/dev/stderr", 0xd46ef3cd, 0x8848ed6b2b366081);
#endif /* !CONFIG_HOST_WINDOWS */
/*[[[end]]]*/



DECL_END

#endif /* !GUARD_DEX_POSIX_P_PATH_C_INL */
