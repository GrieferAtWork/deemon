/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_P_OPEN_C_INL
#define GUARD_DEX_POSIX_P_OPEN_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"

DECL_BEGIN

/*[[[deemon
import * from deemon;
import * from _dexutils;
MODULE_NAME = "posix";
local orig_stdout = File.stdout;

include("p-open-constants.def");

local allDecls = [];

function fgi(name, doc = none) {
	allDecls.append(name);
	gi(name, doc: doc);
}

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

fgi      ("AT_FDCWD",            doc: "Special value used to indicate the *at functions should use the current working directory");
fgi      ("AT_SYMLINK_NOFOLLOW", doc: "Do not follow symbolic links");
fgi      ("AT_REMOVEDIR",        doc: "Remove directory instead of unlinking file");
fgii_conf("AT_SYMLINK_FOLLOW",   doc: "Follow symbolic links");
fgii_conf("AT_NO_AUTOMOUNT",     doc: "Suppress terminal automount traversal");
fgii_conf("AT_EMPTY_PATH",       doc: "Allow empty relative pathname");
fgii_conf("AT_EACCESS",          doc: "Test access permitted for effective IDs, not real IDs");
fgi      ("AT_REMOVEREG",        doc: "Explicitly allow removing anything that unlink() removes. (Default; Set in addition to ?GAT_REMOVEDIR to implement ?Gremove semantics)");
fgii_conf("AT_DOSPATH",          doc: "Interpret $\"\\\" as $\"/\", and ignore casing during path resolution");
fgii_conf("AT_FDROOT",           doc: "Same as ?GAT_FDCWD but sets the filesystem root (using this, you can ?Gchroot with ?Gdup2)");
fgii_conf("AT_THIS_TASK");
fgii_conf("AT_THIS_PROCESS");
fgii_conf("AT_PARENT_PROCESS");
fgii_conf("AT_GROUP_LEADER");
fgii_conf("AT_SESSION_LEADER");
fgii_conf("AT_DOS_DRIVEMIN");
fgii_conf("AT_DOS_DRIVEMAX");

fgii     ("RENAME_NOREPLACE",    doc: "Don't overwrite target");
fgii_conf("RENAME_EXCHANGE",     doc: "Exchange source and dest");
fgii_conf("RENAME_WHITEOUT",     doc: "Whiteout source");

File.stdout = orig_stdout;
print "#define POSIX_OPEN_BASIC_DEFS \\";
for (local x: allDecls)
	print("\tPOSIX_", x, "_DEF \\");
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
	POSIX_AT_THIS_PROCESS_DEF \
	POSIX_AT_PARENT_PROCESS_DEF \
	POSIX_AT_GROUP_LEADER_DEF \
	POSIX_AT_SESSION_LEADER_DEF \
	POSIX_AT_DOS_DRIVEMIN_DEF \
	POSIX_AT_DOS_DRIVEMAX_DEF \
	POSIX_RENAME_NOREPLACE_DEF \
	POSIX_RENAME_EXCHANGE_DEF \
	POSIX_RENAME_WHITEOUT_DEF \
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

/* Flags that are automatically passed by `open()' (but not by `_open()'). */
#undef POSIX_OPEN_DEFAULT_FLAGS
#if defined(CONFIG_HAVE_O_BINARY) && defined(CONFIG_HAVE_O_OBTAIN_DIR)
#define POSIX_OPEN_DEFAULT_FLAGS (O_BINARY | O_OBTAIN_DIR)
#elif defined(CONFIG_HAVE_O_BINARY)
#define POSIX_OPEN_DEFAULT_FLAGS (O_BINARY)
#elif defined(CONFIG_HAVE_O_OBTAIN_DIR)
#define POSIX_OPEN_DEFAULT_FLAGS (O_OBTAIN_DIR)
#endif /* ... */

/* Flags that are automatically passed by `creat()' (but not by `_creat()'). */
#undef POSIX_CREAT_DEFAULT_FLAGS
#if defined(CONFIG_HAVE_O_BINARY)
#define POSIX_CREAT_DEFAULT_FLAGS (O_BINARY)
#endif /* CONFIG_HAVE_O_BINARY */





/* Figure out how to implement `open()' */
#undef posix_open_USE_wopen
#undef posix_open_USE_open
#undef posix_open_USE_STUB
#if (defined(CONFIG_HAVE_wopen) || defined(CONFIG_HAVE_wopen64)) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_open_USE_wopen
#elif defined(CONFIG_HAVE_open) || defined(CONFIG_HAVE_open64)
#define posix_open_USE_open
#elif defined(CONFIG_HAVE_wopen) || defined(CONFIG_HAVE_wopen64)
#define posix_open_USE_wopen
#else /* ... */
#define posix_open_USE_STUB
#endif /* !... */

/* Figure out how to implement `creat()' */
#undef posix_creat_USE_wcreat
#undef posix_creat_USE_creat
#undef posix_creat_USE_STUB
#if (defined(CONFIG_HAVE_wcreat) || defined(CONFIG_HAVE_wcreat64)) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_creat_USE_wcreat
#elif defined(CONFIG_HAVE_creat) || defined(CONFIG_HAVE_creat64)
#define posix_creat_USE_creat
#elif defined(CONFIG_HAVE_wcreat) || defined(CONFIG_HAVE_wcreat64)
#define posix_creat_USE_wcreat
#else /* ... */
#define posix_creat_USE_STUB
#endif /* !... */
#if !(defined(O_CREAT) && (defined(O_WRONLY) || defined(O_RDWR)) && defined(O_TRUNC)) || \
      defined(posix_open_USE_STUB)
#undef POSIX_CREAT_DEFAULT_FLAGS
#endif /* ... */




/************************************************************************/
/* open()                                                               */
/************************************************************************/

#if defined(posix_open_USE_wopen) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("_open", "filename:c:wchar_t[],oflags:u,mode:u=0644->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix__open_f_impl(dwchar_t const *filename, unsigned int oflags, unsigned int mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix__open_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX__OPEN_DEF { "_open", (DeeObject *)&posix__open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=!0644)->?Dint") },
#define POSIX__OPEN_DEF_DOC(doc) { "_open", (DeeObject *)&posix__open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=!0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix__open, posix__open_f);
#ifndef POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED
#define POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_filename_oflags_mode, { K(filename), K(oflags), K(mode), KEND });
#endif /* !POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix__open_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	dwchar_t const *filename_str;
	DeeStringObject *filename;
	unsigned int oflags;
	unsigned int mode = 0644;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_oflags_mode, "ou|u:_open", &filename, &oflags, &mode))
		goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
		goto err;
	filename_str = (dwchar_t const *)DeeString_AsWide((DeeObject *)filename);
	if unlikely(!filename_str)
		goto err;
	return posix__open_f_impl(filename_str, oflags, mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix__open_f_impl(dwchar_t const *filename, unsigned int oflags, unsigned int mode)
//[[[end]]]
#endif /* posix_open_USE_wopen */
#if defined(posix_open_USE_open) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("_open", "filename:c:char[],oflags:u,mode:u=0644->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix__open_f_impl(/*utf-8*/ char const *filename, unsigned int oflags, unsigned int mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix__open_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX__OPEN_DEF { "_open", (DeeObject *)&posix__open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=!0644)->?Dint") },
#define POSIX__OPEN_DEF_DOC(doc) { "_open", (DeeObject *)&posix__open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=!0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix__open, posix__open_f);
#ifndef POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED
#define POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_filename_oflags_mode, { K(filename), K(oflags), K(mode), KEND });
#endif /* !POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix__open_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	/*utf-8*/ char const *filename_str;
	DeeStringObject *filename;
	unsigned int oflags;
	unsigned int mode = 0644;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_oflags_mode, "ou|u:_open", &filename, &oflags, &mode))
		goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
		goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
		goto err;
	return posix__open_f_impl(filename_str, oflags, mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix__open_f_impl(/*utf-8*/ char const *filename, unsigned int oflags, unsigned int mode)
//[[[end]]]
#endif /* posix_open_USE_open */
{
#if defined(posix_open_USE_wopen) || defined(posix_open_USE_open)
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_open_USE_wopen
#define OPEN_PRINTF_FILENAME "%lq"
#ifdef CONFIG_HAVE_wopen64
	result = wopen64(filename, (int)oflags, (int)mode);
#else /* CONFIG_HAVE_wopen64 */
	result = wopen(filename, (int)oflags, (int)mode);
#endif /* !CONFIG_HAVE_wopen64 */
#endif /* posix_open_USE_wopen */

#ifdef posix_open_USE_open
#define OPEN_PRINTF_FILENAME "%q"
#ifdef CONFIG_HAVE_open64
	result = open64(filename, (int)oflags, (int)mode);
#else /* CONFIG_HAVE_open64 */
	result = open(filename, (int)oflags, (int)mode);
#endif /* !CONFIG_HAVE_open64 */
#endif /* posix_open_USE_open */

	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		result = DeeSystem_GetErrno();
		EINTR_HANDLE(result, again, err)
		HANDLE_ENOENT_ENOTDIR(result, err, "File or directory " OPEN_PRINTF_FILENAME " could not be found", filename)
		HANDLE_EEXIST_IF(result, oflags & OPEN_FEXCL, err, "File " OPEN_PRINTF_FILENAME " already exists", filename)
		HANDLE_EACCES(result, err, "Failed to access " OPEN_PRINTF_FILENAME, filename)
		HANDLE_ENXIO_EISDIR(result, err, "Cannot open directory " OPEN_PRINTF_FILENAME " for writing", filename)
		HANDLE_EROFS_ETXTBSY(result, err, "Read-only file " OPEN_PRINTF_FILENAME, filename)
		HANDLE_ENOSYS(result, err, "open")
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, result,
		                          "Failed to open " OPEN_PRINTF_FILENAME,
		                          filename);
		goto err;
	}
	return DeeInt_NewUInt((unsigned int)result);
err:
	return NULL;
#undef OPEN_PRINTF_FILENAME
#endif /* posix_open_USE_wopen || posix_open_USE_open */

#ifdef posix_open_USE_STUB
#define NEED_posix_err_unsupported
	(void)filename;
	(void)oflags;
	(void)mode;
	posix_err_unsupported("open");
	return NULL;
#endif /* posix_open_USE_STUB */
}

#if defined(posix_open_USE_wopen) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("open", "filename:c:wchar_t[],oflags:u,mode:u=0644->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_open_f_impl(dwchar_t const *filename, unsigned int oflags, unsigned int mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_open_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_OPEN_DEF { "open", (DeeObject *)&posix_open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=!0644)->?Dint") },
#define POSIX_OPEN_DEF_DOC(doc) { "open", (DeeObject *)&posix_open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=!0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_open, posix_open_f);
#ifndef POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED
#define POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_filename_oflags_mode, { K(filename), K(oflags), K(mode), KEND });
#endif /* !POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_open_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
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
	return posix_open_f_impl(filename_str, oflags, mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_open_f_impl(dwchar_t const *filename, unsigned int oflags, unsigned int mode)
//[[[end]]]
#endif /* posix_open_USE_wopen */
#if defined(posix_open_USE_open) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("open", "filename:c:char[],oflags:u,mode:u=0644->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_open_f_impl(/*utf-8*/ char const *filename, unsigned int oflags, unsigned int mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_open_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_OPEN_DEF { "open", (DeeObject *)&posix_open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=!0644)->?Dint") },
#define POSIX_OPEN_DEF_DOC(doc) { "open", (DeeObject *)&posix_open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=!0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_open, posix_open_f);
#ifndef POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED
#define POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_filename_oflags_mode, { K(filename), K(oflags), K(mode), KEND });
#endif /* !POSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_open_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
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
	return posix_open_f_impl(filename_str, oflags, mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_open_f_impl(/*utf-8*/ char const *filename, unsigned int oflags, unsigned int mode)
//[[[end]]]
#endif /* posix_open_USE_open */
{
#ifdef POSIX_OPEN_DEFAULT_FLAGS
	oflags |= POSIX_OPEN_DEFAULT_FLAGS;
#endif /* POSIX_OPEN_DEFAULT_FLAGS */
	return posix__open_f_impl(filename, oflags, mode);
}




/************************************************************************/
/* creat()                                                              */
/************************************************************************/

#if defined(posix_creat_USE_wcreat) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("_creat", "filename:c:wchar_t[],mode:u=0644->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix__creat_f_impl(dwchar_t const *filename, unsigned int mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix__creat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX__CREAT_DEF { "_creat", (DeeObject *)&posix__creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=!0644)->?Dint") },
#define POSIX__CREAT_DEF_DOC(doc) { "_creat", (DeeObject *)&posix__creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=!0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix__creat, posix__creat_f);
#ifndef POSIX_KWDS_FILENAME_MODE_DEFINED
#define POSIX_KWDS_FILENAME_MODE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_filename_mode, { K(filename), K(mode), KEND });
#endif /* !POSIX_KWDS_FILENAME_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix__creat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	dwchar_t const *filename_str;
	DeeStringObject *filename;
	unsigned int mode = 0644;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_mode, "o|u:_creat", &filename, &mode))
		goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
		goto err;
	filename_str = (dwchar_t const *)DeeString_AsWide((DeeObject *)filename);
	if unlikely(!filename_str)
		goto err;
	return posix__creat_f_impl(filename_str, mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix__creat_f_impl(dwchar_t const *filename, unsigned int mode)
//[[[end]]]
#endif /* posix_creat_USE_wcreat */
#if defined(posix_creat_USE_creat) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("_creat", "filename:c:char[],mode:u=0644->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix__creat_f_impl(/*utf-8*/ char const *filename, unsigned int mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix__creat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX__CREAT_DEF { "_creat", (DeeObject *)&posix__creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=!0644)->?Dint") },
#define POSIX__CREAT_DEF_DOC(doc) { "_creat", (DeeObject *)&posix__creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=!0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix__creat, posix__creat_f);
#ifndef POSIX_KWDS_FILENAME_MODE_DEFINED
#define POSIX_KWDS_FILENAME_MODE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_filename_mode, { K(filename), K(mode), KEND });
#endif /* !POSIX_KWDS_FILENAME_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix__creat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	/*utf-8*/ char const *filename_str;
	DeeStringObject *filename;
	unsigned int mode = 0644;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_mode, "o|u:_creat", &filename, &mode))
		goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
		goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
		goto err;
	return posix__creat_f_impl(filename_str, mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix__creat_f_impl(/*utf-8*/ char const *filename, unsigned int mode)
//[[[end]]]
#endif /* posix_creat_USE_creat */
{
#if defined(posix_creat_USE_wcreat) || defined(posix_creat_USE_creat)
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();

#ifdef posix_creat_USE_wcreat
#define CREAT_PRINTF_FILENAME "%lq"
#ifdef CONFIG_HAVE_wcreat64
	result = wcreat64(filename, (int)mode);
#else /* CONFIG_HAVE_wcreat64 */
	result = wcreat(filename, (int)mode);
#endif /* !CONFIG_HAVE_wcreat64 */
#endif /* posix_creat_USE_wcreat */

#ifdef posix_creat_USE_creat
#define CREAT_PRINTF_FILENAME "%q"
#ifdef CONFIG_HAVE_creat64
	result = creat64(filename, (int)mode);
#else /* CONFIG_HAVE_creat64 */
	result = creat(filename, (int)mode);
#endif /* !CONFIG_HAVE_creat64 */
#endif /* posix_creat_USE_creat */

	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		result = DeeSystem_GetErrno();
		EINTR_HANDLE(result, again, err)
		HANDLE_ENOENT_ENOTDIR(result, err, "File or directory " CREAT_PRINTF_FILENAME " could not be found", filename)
		HANDLE_EACCES(result, err, "Failed to access " CREAT_PRINTF_FILENAME, filename)
		HANDLE_ENXIO_EISDIR(result, err, "Cannot open directory " CREAT_PRINTF_FILENAME " for writing", filename)
		HANDLE_EROFS_ETXTBSY(result, err, "Read-only file " CREAT_PRINTF_FILENAME, filename)
		HANDLE_ENOSYS(result, err, "creat")
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, result,
		                          "Failed to open " CREAT_PRINTF_FILENAME,
		                          filename);
		goto err;
	}
	return DeeInt_NewUInt((unsigned int)result);
err:
	return NULL;
#undef CREAT_PRINTF_FILENAME
#endif /* posix_creat_USE_wcreat || posix_creat_USE_creat */

#ifdef posix_creat_USE_STUB
#define NEED_posix_err_unsupported
	(void)filename;
	(void)mode;
	posix_err_unsupported("creat");
	return NULL;
#endif /* posix_creat_USE_STUB */
}

#if defined(posix_creat_USE_wcreat) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("creat", "filename:c:wchar_t[],mode:u=0644->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_creat_f_impl(dwchar_t const *filename, unsigned int mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_creat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_CREAT_DEF { "creat", (DeeObject *)&posix_creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=!0644)->?Dint") },
#define POSIX_CREAT_DEF_DOC(doc) { "creat", (DeeObject *)&posix_creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=!0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_creat, posix_creat_f);
#ifndef POSIX_KWDS_FILENAME_MODE_DEFINED
#define POSIX_KWDS_FILENAME_MODE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_filename_mode, { K(filename), K(mode), KEND });
#endif /* !POSIX_KWDS_FILENAME_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_creat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
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
	return posix_creat_f_impl(filename_str, mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_creat_f_impl(dwchar_t const *filename, unsigned int mode)
//[[[end]]]
#endif /* posix_creat_USE_wcreat */
#if defined(posix_creat_USE_creat) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("creat", "filename:c:char[],mode:u=0644->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_creat_f_impl(/*utf-8*/ char const *filename, unsigned int mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_creat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_CREAT_DEF { "creat", (DeeObject *)&posix_creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=!0644)->?Dint") },
#define POSIX_CREAT_DEF_DOC(doc) { "creat", (DeeObject *)&posix_creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=!0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_creat, posix_creat_f);
#ifndef POSIX_KWDS_FILENAME_MODE_DEFINED
#define POSIX_KWDS_FILENAME_MODE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_filename_mode, { K(filename), K(mode), KEND });
#endif /* !POSIX_KWDS_FILENAME_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_creat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
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
	return posix_creat_f_impl(filename_str, mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_creat_f_impl(/*utf-8*/ char const *filename, unsigned int mode)
//[[[end]]]
#endif /* posix_creat_USE_creat */
{
#ifdef POSIX_CREAT_DEFAULT_FLAGS
	return posix__open_f_impl(filename,
#ifdef O_WRONLY
	                          POSIX_CREAT_DEFAULT_FLAGS | O_CREAT | O_WRONLY | O_TRUNC
#else  /* O_WRONLY */
	                          POSIX_CREAT_DEFAULT_FLAGS | O_CREAT | O_RDWR | O_TRUNC
#endif /* !O_WRONLY */
	                          ,
	                          mode);
#else /* POSIX_CREAT_DEFAULT_FLAGS */
	return posix__creat_f_impl(filename, mode);
#endif /* !POSIX_CREAT_DEFAULT_FLAGS */
}

DECL_END

#endif /* !GUARD_DEX_POSIX_P_OPEN_C_INL */
