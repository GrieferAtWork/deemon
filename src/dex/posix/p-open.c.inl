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
#ifndef GUARD_DEX_POSIX_P_OPEN_C_INL
#define GUARD_DEX_POSIX_P_OPEN_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"

DECL_BEGIN

/*[[[deemon
import * from deemon;
import * from _dexutils;
MODULE_NAME = "posix";
local orig_stdout = File.stdout;

include("p-open-constants.def");

local allDecls = [];

function fgii(name, doc = none) {
	allDecls.append(name);
	gii(name, doc: doc);
}

function fgii_conf(name, doc = none) {
	allDecls.append(name);
	gii(name, doc: doc, check: "CONFIG_HAVE_" + name);
}

fgii_conf("O_RDONLY");
fgii_conf("O_WRONLY");
fgii_conf("O_RDWR");
fgii_conf("O_APPEND",    doc: "Always append data to the end of the file");
fgii_conf("O_CREAT",     doc: "If missing, create a new file");
fgii_conf("O_TRUNC");
fgii_conf("O_EXCL",      doc: "When used with #O_CREAT, set #EEXIST if the file already exists");
fgii_conf("O_TEXT");
fgii_conf("O_BINARY");
fgii_conf("O_WTEXT");
fgii_conf("O_U16TEXT");
fgii_conf("O_U8TEXT");
fgii_conf("O_CLOEXEC",   doc: "Close the file during ${exec()}");
fgii_conf("O_TEMPORARY");
fgii_conf("O_SHORT_LIVED");
fgii_conf("O_OBTAIN_DIR");
fgii_conf("O_SEQUENTIAL");
fgii_conf("O_RANDOM");
fgii_conf("O_NOCTTY",    doc: "If the calling process does not have a controlling terminal assigned, do not attempt to assign the newly opened file as terminal, even when ${isatty(open(...))} would be true");
fgii_conf("O_NONBLOCK",  doc: "Do not block when trying to read data that hasn't been written, yet");
fgii_conf("O_SYNC");
fgii_conf("O_RSYNC");
fgii_conf("O_DSYNC");
fgii_conf("O_ASYNC");
fgii_conf("O_DIRECT");
fgii_conf("O_LARGEFILE");
fgii_conf("O_DIRECTORY", doc: "Set #ENOTDIR when the final path component of an open() system call turns out to be something other than a directory");
fgii_conf("O_NOFOLLOW",  doc: "Set #ELOOP when the final path component of an open() system call turns out to be a symbolic link");
fgii_conf("O_NOATIME",   doc: "Don't update last-accessed time stamps");
fgii_conf("O_PATH",      doc: "Open a path for *at system calls");
fgii_conf("O_TMPFILE");
fgii_conf("O_CLOFORK",   doc: "Close the handle when the file descriptors are unshared");
fgii_conf("O_SYMLINK",   doc: "Open a symlink itself, rather than dereferencing it");
fgii_conf("O_DOSPATH",   doc: "Interpret $\"\\\" as $\"/\", and ignore casing during path resolution. Additionally, recognize DOS mounting points, and interpret leading slashes as relative to the closest DOS mounting point");
fgii_conf("O_SHLOCK");
fgii_conf("O_EXLOCK");
fgii_conf("O_XATTR");
fgii_conf("O_EXEC");
fgii_conf("O_SEARCH");
fgii_conf("O_TTY_INIT");
fgii_conf("O_NOLINKS");

fgii("AT_FDCWD",            doc: "Special value used to indicate the *at functions should use the current working directory");
fgii("AT_SYMLINK_NOFOLLOW", doc: "Do not follow symbolic links");
fgii("AT_REMOVEDIR",        doc: "Remove directory instead of unlinking file");
fgii("AT_SYMLINK_FOLLOW",   doc: "Follow symbolic links");
fgii("AT_NO_AUTOMOUNT",     doc: "Suppress terminal automount traversal");
fgii("AT_EMPTY_PATH",       doc: "Allow empty relative pathname");
fgii("AT_EACCESS",          doc: "Test access permitted for effective IDs, not real IDs");
fgii("AT_REMOVEREG",        doc: "Explicitly allow removing anything that unlink() removes. (Default; Set in addition to #AT_REMOVEDIR to implement #remove semantics)");
fgii("AT_DOSPATH",          doc: "Interpret $\"\\\" as $\"/\", and ignore casing during path resolution");
fgii("AT_FDROOT",           doc: "Same as #AT_FDCWD but sets the filesystem root (using this, you can #chroot with #dup2)");
fgii("AT_THIS_TASK");
fgii("AT_THIS_MMAN");
fgii("AT_THIS_STACK");

File.stdout = orig_stdout;
print "#define POSIX_OPEN_BASIC_DEFS \\";
for (local x: allDecls) {
	print "\tPOSIX_",;
	print x,;
	print "_DEF \\";
}
print "/" "**" "/";

]]]*/
#include "p-open-constants.def"
#define POSIX_OPEN_BASIC_DEFS \
	POSIX_O_RDONLY_DEF \
	POSIX_O_WRONLY_DEF \
	POSIX_O_RDWR_DEF \
	POSIX_O_APPEND_DEF \
	POSIX_O_CREAT_DEF \
	POSIX_O_TRUNC_DEF \
	POSIX_O_EXCL_DEF \
	POSIX_O_TEXT_DEF \
	POSIX_O_BINARY_DEF \
	POSIX_O_WTEXT_DEF \
	POSIX_O_U16TEXT_DEF \
	POSIX_O_U8TEXT_DEF \
	POSIX_O_CLOEXEC_DEF \
	POSIX_O_TEMPORARY_DEF \
	POSIX_O_SHORT_LIVED_DEF \
	POSIX_O_OBTAIN_DIR_DEF \
	POSIX_O_SEQUENTIAL_DEF \
	POSIX_O_RANDOM_DEF \
	POSIX_O_NOCTTY_DEF \
	POSIX_O_NONBLOCK_DEF \
	POSIX_O_SYNC_DEF \
	POSIX_O_RSYNC_DEF \
	POSIX_O_DSYNC_DEF \
	POSIX_O_ASYNC_DEF \
	POSIX_O_DIRECT_DEF \
	POSIX_O_LARGEFILE_DEF \
	POSIX_O_DIRECTORY_DEF \
	POSIX_O_NOFOLLOW_DEF \
	POSIX_O_NOATIME_DEF \
	POSIX_O_PATH_DEF \
	POSIX_O_TMPFILE_DEF \
	POSIX_O_CLOFORK_DEF \
	POSIX_O_SYMLINK_DEF \
	POSIX_O_DOSPATH_DEF \
	POSIX_O_SHLOCK_DEF \
	POSIX_O_EXLOCK_DEF \
	POSIX_O_XATTR_DEF \
	POSIX_O_EXEC_DEF \
	POSIX_O_SEARCH_DEF \
	POSIX_O_TTY_INIT_DEF \
	POSIX_O_NOLINKS_DEF \
	POSIX_AT_FDCWD_DEF \
	POSIX_AT_SYMLINK_NOFOLLOW_DEF \
	POSIX_AT_REMOVEDIR_DEF \
	POSIX_AT_SYMLINK_FOLLOW_DEF \
	POSIX_AT_NO_AUTOMOUNT_DEF \
	POSIX_AT_EMPTY_PATH_DEF \
	POSIX_AT_EACCESS_DEF \
	POSIX_AT_REMOVEREG_DEF \
	POSIX_AT_DOSPATH_DEF \
	POSIX_AT_FDROOT_DEF \
	POSIX_AT_THIS_TASK_DEF \
	POSIX_AT_THIS_MMAN_DEF \
	POSIX_AT_THIS_STACK_DEF \
/**/
//[[[end]]]

#ifdef O_CLOEXEC
#define POSIX_OPEN_O_NOINHERIT_ALIAS_DEF \
	{ "O_NOINHERIT", (DeeObject *)&posix_O_CLOEXEC, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, "Alias for #O_CLOEXEC" },
#else /* O_CLOEXEC */
#define POSIX_OPEN_O_NOINHERIT_ALIAS_DEF /* nothing */
#endif /* !O_CLOEXEC */

#ifdef O_NONBLOCK
#define POSIX_OPEN_O_NDELAY_ALIAS_DEF \
	{ "O_NDELAY", (DeeObject *)&posix_O_NONBLOCK, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, "Alias for #O_NONBLOCK" },
#else /* O_NONBLOCK */
#define POSIX_OPEN_O_NDELAY_ALIAS_DEF /* nothing */
#endif /* !O_NONBLOCK */

#define POSIX_OPEN_DEFS              \
	POSIX_OPEN_BASIC_DEFS            \
	POSIX_OPEN_O_NOINHERIT_ALIAS_DEF \
	POSIX_OPEN_O_NDELAY_ALIAS_DEF





/************************************************************************/
/* open()                                                               */
/************************************************************************/

#if defined(CONFIG_HAVE_wopen64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("open", "filename:c:wchar_t[],oflags:u,mode:u=0644->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_open_f_impl(dwchar_t const *filename, unsigned int oflags, unsigned int mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_open_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_OPEN_DEF { "open", (DeeObject *)&posix_open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=0644)->?Dint") },
#define POSIX_OPEN_DEF_DOC(doc) { "open", (DeeObject *)&posix_open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_open, posix_open_f);
#ifndef POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED
#define POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_filename_oflags_mode, { K(filename), K(oflags), K(mode), KEND });
#endif /* !POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_open_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	dwchar_t const *filename_str;
	DeeStringObject *filename;
	unsigned int oflags;
	unsigned int mode = 0644;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_oflags_mode, "ou|u:open", &filename, &oflags, &mode))
	    goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
	    goto err;
	filename_str = (dwchar_t const *)DeeString_AsWide((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return posix_open_f_impl(filename_str,oflags,mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_open_f_impl(dwchar_t const *filename, unsigned int oflags, unsigned int mode)
//[[[end]]]
#endif
#if !defined(CONFIG_HAVE_wopen64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("open", "filename:c:char[],oflags:u,mode:u=0644->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_open_f_impl(/*utf-8*/ char const *filename, unsigned int oflags, unsigned int mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_open_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_OPEN_DEF { "open", (DeeObject *)&posix_open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=0644)->?Dint") },
#define POSIX_OPEN_DEF_DOC(doc) { "open", (DeeObject *)&posix_open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_open, posix_open_f);
#ifndef POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED
#define POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_filename_oflags_mode, { K(filename), K(oflags), K(mode), KEND });
#endif /* !POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_open_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	/*utf-8*/ char const *filename_str;
	DeeStringObject *filename;
	unsigned int oflags;
	unsigned int mode = 0644;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_oflags_mode, "ou|u:open", &filename, &oflags, &mode))
	    goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return posix_open_f_impl(filename_str,oflags,mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_open_f_impl(/*utf-8*/ char const *filename, unsigned int oflags, unsigned int mode)
//[[[end]]]
#endif
{
#if defined(CONFIG_HAVE_wopen64) || defined(CONFIG_HAVE_open64)
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_wopen64
#define OPEN_PRINTF_FILENAME "%ls"
	result = wopen64(filename, (int)oflags, (int)mode);
#else /* CONFIG_HAVE_wopen64 */
#define OPEN_PRINTF_FILENAME "%s"
	result = open64(filename, (int)oflags, (int)mode);
#endif /* !CONFIG_HAVE_wopen64 */
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		result = DeeSystem_GetErrno();
		HANDLE_EINTR(result, again, err)
		HANDLE_ENOENT_ENOTDIR(result, err, "File or directory " OPEN_PRINTF_FILENAME " could not be found", filename)
		HANDLE_EEXIST_IF(result, oflags & OPEN_FEXCL, err, "File " OPEN_PRINTF_FILENAME " already exists", filename)
		HANDLE_EACCES(result, err, "Failed to access " OPEN_PRINTF_FILENAME, filename)
		HANDLE_ENXIO_EISDIR(result, err, "Cannot open directory " OPEN_PRINTF_FILENAME " for writing", filename)
		HANDLE_EROFS_ETXTBSY(result, err, "Read-only file " OPEN_PRINTF_FILENAME, filename)
		HANDLE_ENOSYS(result, err, "open")
		DeeError_SysThrowf(&DeeError_FSError, result,
		                   "Failed to open " OPEN_PRINTF_FILENAME,
		                   filename);
		goto err;
	}
	return DeeInt_NewUInt((unsigned int)result);
err:
#else /* CONFIG_HAVE_wopen64 || CONFIG_HAVE_open64 */
#define NEED_ERR_UNSUPPORTED 1
	(void)filename;
	(void)oflags;
	(void)mode;
	posix_err_unsupported("open");
#endif /* !CONFIG_HAVE_wopen64 && !CONFIG_HAVE_open64 */
	return NULL;
}





/************************************************************************/
/* creat()                                                              */
/************************************************************************/

#if defined(CONFIG_HAVE_wcreat64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("creat", "filename:c:wchar_t[],mode:u=0644->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_creat_f_impl(dwchar_t const *filename, unsigned int mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_creat_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_CREAT_DEF { "creat", (DeeObject *)&posix_creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=0644)->?Dint") },
#define POSIX_CREAT_DEF_DOC(doc) { "creat", (DeeObject *)&posix_creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_creat, posix_creat_f);
#ifndef POSIX_KWDS_FILENAME_MODE_DEFINED
#define POSIX_KWDS_FILENAME_MODE_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_filename_mode, { K(filename), K(mode), KEND });
#endif /* !POSIX_KWDS_FILENAME_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_creat_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	dwchar_t const *filename_str;
	DeeStringObject *filename;
	unsigned int mode = 0644;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_mode, "o|u:creat", &filename, &mode))
	    goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
	    goto err;
	filename_str = (dwchar_t const *)DeeString_AsWide((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return posix_creat_f_impl(filename_str,mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_creat_f_impl(dwchar_t const *filename, unsigned int mode)
//[[[end]]]
#endif
#if !defined(CONFIG_HAVE_wcreat64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("creat", "filename:c:char[],mode:u=0644->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_creat_f_impl(/*utf-8*/ char const *filename, unsigned int mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_creat_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_CREAT_DEF { "creat", (DeeObject *)&posix_creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=0644)->?Dint") },
#define POSIX_CREAT_DEF_DOC(doc) { "creat", (DeeObject *)&posix_creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_creat, posix_creat_f);
#ifndef POSIX_KWDS_FILENAME_MODE_DEFINED
#define POSIX_KWDS_FILENAME_MODE_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_filename_mode, { K(filename), K(mode), KEND });
#endif /* !POSIX_KWDS_FILENAME_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_creat_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	/*utf-8*/ char const *filename_str;
	DeeStringObject *filename;
	unsigned int mode = 0644;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_mode, "o|u:creat", &filename, &mode))
	    goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return posix_creat_f_impl(filename_str,mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_creat_f_impl(/*utf-8*/ char const *filename, unsigned int mode)
//[[[end]]]
#endif
{
#if defined(CONFIG_HAVE_wcreat64) || defined(CONFIG_HAVE_creat64)
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_wcreat64
#define CREAT_PRINTF_FILENAME "%ls"
	result = wcreat64(filename, (int)mode);
#else /* CONFIG_HAVE_wcreat64 */
#define CREAT_PRINTF_FILENAME "%s"
	result = creat64(filename, (int)mode);
#endif /* !CONFIG_HAVE_wcreat64 */
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		result = DeeSystem_GetErrno();
		HANDLE_EINTR(result, again, err)
		HANDLE_ENOENT_ENOTDIR(result, err, "File or directory " CREAT_PRINTF_FILENAME " could not be found", filename)
		HANDLE_EACCES(result, err, "Failed to access " CREAT_PRINTF_FILENAME, filename)
		HANDLE_ENXIO_EISDIR(result, err, "Cannot open directory " CREAT_PRINTF_FILENAME " for writing", filename)
		HANDLE_EROFS_ETXTBSY(result, err, "Read-only file " CREAT_PRINTF_FILENAME, filename)
		HANDLE_ENOSYS(result, err, "creat")
		DeeError_SysThrowf(&DeeError_FSError, result,
		                   "Failed to open " CREAT_PRINTF_FILENAME,
		                   filename);
		goto err;
	}
	return DeeInt_NewUInt((unsigned int)result);
err:
#else /* CONFIG_HAVE_wcreat64 || CONFIG_HAVE_creat64 */
#define NEED_ERR_UNSUPPORTED 1
	(void)filename;
	(void)mode;
	posix_err_unsupported("creat");
#endif /* !CONFIG_HAVE_wcreat64 && !CONFIG_HAVE_creat64 */
	return NULL;
}

DECL_END

#endif /* !GUARD_DEX_POSIX_P_OPEN_C_INL */
