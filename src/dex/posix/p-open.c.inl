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
#ifndef GUARD_DEX_POSIX_P_OPEN_C_INL
#define GUARD_DEX_POSIX_P_OPEN_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/api.h>

#include <deemon/arg.h>             /* DEFINE_KWLIST, DeeArg_UnpackStructKw */
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/system-features.h>
#include <deemon/system.h>

#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* intptr_t */

DECL_BEGIN

/*[[[deemon
import * from deemon;
import * from rt.gen.dexutils;
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
fgii     ("O_HIDDEN", doc: "When creating a new file, mark it with the DOS HIDDEN attribute");

fgi      ("AT_FDCWD",            doc: "Special value used to indicate the *at functions should use the current working directory");
fgi      ("AT_SYMLINK_NOFOLLOW", doc: "Do not follow symbolic links");
fgi      ("AT_REMOVEDIR",        doc: "Remove directory instead of unlinking file");
fgii_conf("AT_SYMLINK_FOLLOW",   doc: "Follow symbolic links");
fgii_conf("AT_NO_AUTOMOUNT",     doc: "Suppress terminal automount traversal");
fgi      ("AT_EMPTY_PATH",       doc: "Allow empty relative pathname");
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
	POSIX_O_HIDDEN_DEF \
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
/*[[[end]]]*/

#ifdef CONFIG_HAVE_O_CLOEXEC
#define POSIX_OPEN_O_NOINHERIT_ALIAS_DEF \
	DEX_MEMBER_F("O_NOINHERIT", &posix_O_CLOEXEC, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, "Alias for ?GO_CLOEXEC"),
#else /* CONFIG_HAVE_O_CLOEXEC */
#define POSIX_OPEN_O_NOINHERIT_ALIAS_DEF /* nothing */
#endif /* !CONFIG_HAVE_O_CLOEXEC */

#ifdef CONFIG_HAVE_O_NONBLOCK
#define POSIX_OPEN_O_NDELAY_ALIAS_DEF \
	DEX_MEMBER_F("O_NDELAY", &posix_O_NONBLOCK, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, "Alias for ?GO_NONBLOCK"),
#else /* CONFIG_HAVE_O_NONBLOCK */
#define POSIX_OPEN_O_NDELAY_ALIAS_DEF /* nothing */
#endif /* !CONFIG_HAVE_O_NONBLOCK */

#define POSIX_OPEN_DEFS              \
	POSIX_OPEN_BASIC_DEFS            \
	POSIX_OPEN_O_NOINHERIT_ALIAS_DEF \
	POSIX_OPEN_O_NDELAY_ALIAS_DEF


/* Posix default flags. */
#ifdef CONFIG_HAVE_O_BINARY
#define POSIX_OPT_O_BINARY O_BINARY
#else /* CONFIG_HAVE_O_BINARY */
#define POSIX_OPT_O_BINARY 0
#endif /* !CONFIG_HAVE_O_BINARY */
#ifdef CONFIG_HAVE_O_OBTAIN_DIR
#define POSIX_OPT_O_OBTAIN_DIR O_OBTAIN_DIR
#else /* CONFIG_HAVE_O_OBTAIN_DIR */
#define POSIX_OPT_O_OBTAIN_DIR 0
#endif /* !CONFIG_HAVE_O_OBTAIN_DIR */
#ifdef CONFIG_HAVE_O_LARGEFILE
#define POSIX_OPT_O_LARGEFILE O_LARGEFILE
#else /* CONFIG_HAVE_O_LARGEFILE */
#define POSIX_OPT_O_LARGEFILE 0
#endif /* !CONFIG_HAVE_O_LARGEFILE */

/* Flags that are automatically passed by `open()' (but not by `_open()'). */
#undef POSIX_OPEN_DEFAULT_FLAGS
#if POSIX_OPT_O_BINARY || POSIX_OPT_O_LARGEFILE || POSIX_OPT_O_OBTAIN_DIR
#define POSIX_OPEN_DEFAULT_FLAGS (POSIX_OPT_O_BINARY | POSIX_OPT_O_LARGEFILE | POSIX_OPT_O_OBTAIN_DIR)
#endif /* POSIX_OPT_O_BINARY || POSIX_OPT_O_LARGEFILE || POSIX_OPT_O_OBTAIN_DIR */

/* Flags that are automatically passed by `creat()' (but not by `_creat()'). */
#undef POSIX_CREAT_DEFAULT_FLAGS
#if POSIX_OPT_O_BINARY || POSIX_OPT_O_LARGEFILE
#define POSIX_CREAT_DEFAULT_FLAGS (POSIX_OPT_O_BINARY | POSIX_OPT_O_LARGEFILE)
#endif /* POSIX_OPT_O_BINARY || POSIX_OPT_O_LARGEFILE */

#undef POSIX_CREAT_OFLAGS
#if defined(CONFIG_HAVE_O_CREAT) && defined(CONFIG_HAVE_O_TRUNC) && defined(CONFIG_HAVE_O_RDWR)
#define POSIX_CREAT_OFLAGS (O_CREAT | O_TRUNC | O_RDWR)
#elif defined(CONFIG_HAVE_O_CREAT) && defined(CONFIG_HAVE_O_TRUNC) && defined(CONFIG_HAVE_O_WRONLY)
#define POSIX_CREAT_OFLAGS (O_CREAT | O_TRUNC | O_WRONLY)
#endif /* ... */


/* Re-link system functions to try and use 64-bit variants (if available) */
/*[[[deemon
local functions = {
	"open",
	"openat",
	"creat",
	"wopen",
	"wopenat",
	"wcreat",
};
for (local f: functions) {
	print("#ifdef CONFIG_HAVE_", f, "64");
	print("#undef CONFIG_HAVE_", f);
	print("#define CONFIG_HAVE_", f);
	print("#undef ", f);
	print("#define ", f, " ", f, "64");
	print("#endif /" "* CONFIG_HAVE_", f, "64 *" "/");
}
]]]*/
#ifdef CONFIG_HAVE_open64
#undef CONFIG_HAVE_open
#define CONFIG_HAVE_open
#undef open
#define open open64
#endif /* CONFIG_HAVE_open64 */
#ifdef CONFIG_HAVE_openat64
#undef CONFIG_HAVE_openat
#define CONFIG_HAVE_openat
#undef openat
#define openat openat64
#endif /* CONFIG_HAVE_openat64 */
#ifdef CONFIG_HAVE_creat64
#undef CONFIG_HAVE_creat
#define CONFIG_HAVE_creat
#undef creat
#define creat creat64
#endif /* CONFIG_HAVE_creat64 */
#ifdef CONFIG_HAVE_wopen64
#undef CONFIG_HAVE_wopen
#define CONFIG_HAVE_wopen
#undef wopen
#define wopen wopen64
#endif /* CONFIG_HAVE_wopen64 */
#ifdef CONFIG_HAVE_wopenat64
#undef CONFIG_HAVE_wopenat
#define CONFIG_HAVE_wopenat
#undef wopenat
#define wopenat wopenat64
#endif /* CONFIG_HAVE_wopenat64 */
#ifdef CONFIG_HAVE_wcreat64
#undef CONFIG_HAVE_wcreat
#define CONFIG_HAVE_wcreat
#undef wcreat
#define wcreat wcreat64
#endif /* CONFIG_HAVE_wcreat64 */
/*[[[end]]]*/





/* Figure out how to implement `open()' */
#undef posix_open_USE_wopen
#undef posix_open_USE_open
#undef posix_open_USE_STUB
#ifndef posix_open_USE_open_osfhandle__AND__CreateFile
#if defined(CONFIG_HAVE_wopen) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_open_USE_wopen
#elif defined(CONFIG_HAVE_open)
#define posix_open_USE_open
#elif defined(CONFIG_HAVE_wopen)
#define posix_open_USE_wopen
#else /* ... */
#define posix_open_USE_STUB
#endif /* !... */
#endif /* !posix_open_USE_open_osfhandle__AND__CreateFile */





/* Figure out how to implement `creat()' */
#undef posix_creat_USE_wcreat
#undef posix_creat_USE_creat
#undef posix_creat_USE_open
#undef posix_creat_USE_STUB
#ifdef posix_open_USE_open_osfhandle__AND__CreateFile
#define posix_creat_USE_open
#elif defined(CONFIG_HAVE_wcreat) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_creat_USE_wcreat
#elif defined(CONFIG_HAVE_creat)
#define posix_creat_USE_creat
#elif defined(CONFIG_HAVE_wcreat)
#define posix_creat_USE_wcreat
#elif !defined(posix_open_USE_STUB) && defined(POSIX_CREAT_OFLAGS)
#define posix_creat_USE_open
#else /* ... */
#define posix_creat_USE_STUB
#endif /* !... */


/* Figure out how to implement `openat()' */
#undef posix_openat_USE_wopenat
#undef posix_openat_USE_openat
#undef posix_openat_USE_posix_open
#undef posix_openat_USE_STUB
#ifdef posix_open_USE_open_osfhandle__AND__CreateFile
#define posix_openat_USE_posix_open
#elif defined(CONFIG_HAVE_wopenat) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_openat_USE_wopenat
#define posix_openat_USE_posix_open
#elif defined(CONFIG_HAVE_openat)
#define posix_openat_USE_openat
#define posix_openat_USE_posix_open
#elif defined(CONFIG_HAVE_wopenat)
#define posix_openat_USE_wopenat
#define posix_openat_USE_posix_open
#elif !defined(posix_open_USE_STUB)
#define posix_openat_USE_posix_open
#else /* ... */
#define posix_openat_USE_STUB
#endif /* !... */




/************************************************************************/
/* open()                                                               */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("_open", "filename:?Dstring,oflags:u,mode:u=0644->?Dint", libname: "posix"); ]]]*/
#define POSIX__OPEN_DEF          DEX_MEMBER_F("_open", &posix__open, DEXSYM_READONLY, "(filename:?Dstring,oflags:?Dint,mode=!0644)->?Dint"),
#define POSIX__OPEN_DEF_DOC(doc) DEX_MEMBER_F("_open", &posix__open, DEXSYM_READONLY, "(filename:?Dstring,oflags:?Dint,mode=!0644)->?Dint\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix__open_f_impl(DeeObject *filename, unsigned int oflags, unsigned int mode);
#ifndef DEFINED_kwlist__filename_oflags_mode
#define DEFINED_kwlist__filename_oflags_mode
PRIVATE DEFINE_KWLIST(kwlist__filename_oflags_mode, { KEX("filename", 0x199d68d3, 0x4a5d0431e1a3caed), KEX("oflags", 0xbe92b5be, 0x4f84e498f7c9d171), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEND });
#endif /* !DEFINED_kwlist__filename_oflags_mode */
PRIVATE WUNUSED DREF DeeObject *DCALL posix__open_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *filename;
		unsigned int oflags;
		unsigned int mode;
	} args;
	args.mode = 0644;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__filename_oflags_mode, "ou|u:_open", &args))
		goto err;
	return posix__open_f_impl(args.filename, args.oflags, args.mode);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix__open, &posix__open_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix__open_f_impl(DeeObject *filename, unsigned int oflags, unsigned int mode)
/*[[[end]]]*/
{

#ifdef posix_open_USE_open_osfhandle__AND__CreateFile
	DREF DeeObject *result;
	int resfd;
	HANDLE hFile;
	hFile = DeeNTSystem_OpenFile(filename, oflags, mode);
	if unlikely(!hFile)
		goto err;
	if (hFile == INVALID_HANDLE_VALUE) {
		if ((oflags & (OPEN_FCREAT | OPEN_FEXCL)) == (OPEN_FCREAT | OPEN_FEXCL)) {
			DeeError_Throwf(&DeeError_FileExists,
			                "File %r already exists",
			                filename);
		} else {
			DeeError_Throwf(&DeeError_FileNotFound,
			                "File %r could not be found",
			                filename);
		}
		goto err;
	}

	/* Wrap the handle into a file descriptor. */
ENOMEM_LABEL(again_wrap)
	{
		int fd_flags = 0;
#ifdef _O_RDONLY
		if ((oflags & Dee_OPEN_FACCMODE) == Dee_OPEN_FRDONLY)
			fd_flags |= _O_RDONLY;
#endif /* _O_RDONLY */
#ifdef _O_APPEND
		if (oflags & Dee_OPEN_FAPPEND)
			fd_flags |= _O_APPEND;
#endif /* _O_APPEND */
		resfd = open_osfhandle((intptr_t)hFile, fd_flags);
	}
	if unlikely(resfd == -1) {
		resfd = DeeSystem_GetErrno();
		ENOMEM_HANDLE(resfd, again_wrap, err_fp)
	}

	result = DeeInt_NewUInt((unsigned int)resfd);
	if unlikely(!result)
		(void)OPT_close(resfd);
	return result;
err_fp:
	CloseHandle(hFile);
err:
	return NULL;
#endif /* posix_open_USE_open_osfhandle__AND__CreateFile */

#if defined(posix_open_USE_wopen) || defined(posix_open_USE_open)
	DREF DeeObject *result;
	int resfd;
EINTR_LABEL(again)

#ifdef posix_open_USE_wopen
	{
		Dee_wchar_t const *wide_filename;
		wide_filename = DeeString_AsWide(filename);
		if unlikely(!wide_filename)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		resfd = wopen((wchar_t *)wide_filename, (int)oflags, (int)mode);
	}
#endif /* posix_open_USE_wopen */

#ifdef posix_open_USE_open
	{
		char const *utf8_filename;
		utf8_filename = DeeString_AsUtf8(filename);
		if unlikely(!utf8_filename)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		resfd = open((char *)utf8_filename, (int)oflags, (int)mode);
	}
#endif /* posix_open_USE_open */

	DBG_ALIGNMENT_ENABLE();
	if (resfd < 0) {
		resfd = DeeSystem_GetErrno();
		EINTR_HANDLE(resfd, again, err)
		HANDLE_ENOENT_ENOTDIR(resfd, err, "File or directory %r could not be found", filename)
		HANDLE_EEXIST_IF(resfd, oflags & OPEN_FEXCL, err, "File %r already exists", filename)
		HANDLE_EACCES(resfd, err, "Failed to access %r", filename)
		HANDLE_ENXIO_EISDIR(resfd, err, "Cannot open directory %r for writing", filename)
		HANDLE_EROFS_ETXTBSY(resfd, err, "Read-only file %r", filename)
		HANDLE_ENOSYS(resfd, err, "open")
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, resfd, "Failed to open %r", filename);
		goto err;
	}
	result = DeeInt_NewUInt((unsigned int)resfd);
	if unlikely(!result)
		(void)OPT_close(resfd);
	return result;
err:
	return NULL;
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

/*[[[deemon import("rt.gen.dexutils").gw("open", "filename:?Dstring,oflags:u,mode:u=0644->?Dint", libname: "posix"); ]]]*/
#define POSIX_OPEN_DEF          DEX_MEMBER_F("open", &posix_open, DEXSYM_READONLY, "(filename:?Dstring,oflags:?Dint,mode=!0644)->?Dint"),
#define POSIX_OPEN_DEF_DOC(doc) DEX_MEMBER_F("open", &posix_open, DEXSYM_READONLY, "(filename:?Dstring,oflags:?Dint,mode=!0644)->?Dint\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_open_f_impl(DeeObject *filename, unsigned int oflags, unsigned int mode);
#ifndef DEFINED_kwlist__filename_oflags_mode
#define DEFINED_kwlist__filename_oflags_mode
PRIVATE DEFINE_KWLIST(kwlist__filename_oflags_mode, { KEX("filename", 0x199d68d3, 0x4a5d0431e1a3caed), KEX("oflags", 0xbe92b5be, 0x4f84e498f7c9d171), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEND });
#endif /* !DEFINED_kwlist__filename_oflags_mode */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_open_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *filename;
		unsigned int oflags;
		unsigned int mode;
	} args;
	args.mode = 0644;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__filename_oflags_mode, "ou|u:open", &args))
		goto err;
	return posix_open_f_impl(args.filename, args.oflags, args.mode);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_open, &posix_open_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_open_f_impl(DeeObject *filename, unsigned int oflags, unsigned int mode)
/*[[[end]]]*/
{
#ifdef POSIX_OPEN_DEFAULT_FLAGS
	oflags |= POSIX_OPEN_DEFAULT_FLAGS;
#endif /* POSIX_OPEN_DEFAULT_FLAGS */
	return posix__open_f_impl(filename, oflags, mode);
}




/************************************************************************/
/* creat()                                                              */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("_creat", "filename:?Dstring,mode:u=0644->?Dint", libname: "posix"); ]]]*/
#define POSIX__CREAT_DEF          DEX_MEMBER_F("_creat", &posix__creat, DEXSYM_READONLY, "(filename:?Dstring,mode=!0644)->?Dint"),
#define POSIX__CREAT_DEF_DOC(doc) DEX_MEMBER_F("_creat", &posix__creat, DEXSYM_READONLY, "(filename:?Dstring,mode=!0644)->?Dint\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix__creat_f_impl(DeeObject *filename, unsigned int mode);
#ifndef DEFINED_kwlist__filename_mode
#define DEFINED_kwlist__filename_mode
PRIVATE DEFINE_KWLIST(kwlist__filename_mode, { KEX("filename", 0x199d68d3, 0x4a5d0431e1a3caed), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEND });
#endif /* !DEFINED_kwlist__filename_mode */
PRIVATE WUNUSED DREF DeeObject *DCALL posix__creat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *filename;
		unsigned int mode;
	} args;
	args.mode = 0644;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__filename_mode, "o|u:_creat", &args))
		goto err;
	return posix__creat_f_impl(args.filename, args.mode);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix__creat, &posix__creat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix__creat_f_impl(DeeObject *filename, unsigned int mode)
/*[[[end]]]*/
{
#if defined(posix_creat_USE_wcreat) || defined(posix_creat_USE_creat)
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();

#ifdef posix_creat_USE_wcreat
	{
		Dee_wchar_t const *wide_filename;
		wide_filename = DeeString_AsWide(filename);
		if unlikely(!wide_filename)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		result = wcreat((wchar_t *)wide_filename, (int)mode);
	}
#endif /* posix_creat_USE_wcreat */

#ifdef posix_creat_USE_creat
	{
		char const *utf8_filename;
		utf8_filename = DeeString_AsUtf8(filename);
		if unlikely(!utf8_filename)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		result = creat((char *)utf8_filename, (int)mode);
	}
#endif /* posix_creat_USE_creat */

	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		result = DeeSystem_GetErrno();
		EINTR_HANDLE(result, again, err)
		HANDLE_ENOENT_ENOTDIR(result, err, "File or directory %r could not be found", filename)
		HANDLE_EACCES(result, err, "Failed to access %r", filename)
		HANDLE_ENXIO_EISDIR(result, err, "Cannot open directory %r for writing", filename)
		HANDLE_EROFS_ETXTBSY(result, err, "Read-only file %r", filename)
		HANDLE_ENOSYS(result, err, "creat")
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, result, "Failed to open %r", filename);
		goto err;
	}
	return DeeInt_NewUInt((unsigned int)result);
err:
	return NULL;
#endif /* posix_creat_USE_wcreat || posix_creat_USE_creat */

#ifdef posix_creat_USE_open
	return posix__open_f_impl(filename, POSIX_CREAT_OFLAGS, mode);
#endif /* posix_creat_USE_open */

#ifdef posix_creat_USE_STUB
#define NEED_posix_err_unsupported
	(void)filename;
	(void)mode;
	posix_err_unsupported("creat");
	return NULL;
#endif /* posix_creat_USE_STUB */
}

/*[[[deemon import("rt.gen.dexutils").gw("creat", "filename:?Dstring,mode:u=0644->?Dint", libname: "posix"); ]]]*/
#define POSIX_CREAT_DEF          DEX_MEMBER_F("creat", &posix_creat, DEXSYM_READONLY, "(filename:?Dstring,mode=!0644)->?Dint"),
#define POSIX_CREAT_DEF_DOC(doc) DEX_MEMBER_F("creat", &posix_creat, DEXSYM_READONLY, "(filename:?Dstring,mode=!0644)->?Dint\n" doc),
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_creat_f_impl(DeeObject *filename, unsigned int mode);
#ifndef DEFINED_kwlist__filename_mode
#define DEFINED_kwlist__filename_mode
PRIVATE DEFINE_KWLIST(kwlist__filename_mode, { KEX("filename", 0x199d68d3, 0x4a5d0431e1a3caed), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEND });
#endif /* !DEFINED_kwlist__filename_mode */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_creat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *filename;
		unsigned int mode;
	} args;
	args.mode = 0644;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__filename_mode, "o|u:creat", &args))
		goto err;
	return posix_creat_f_impl(args.filename, args.mode);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_creat, &posix_creat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_creat_f_impl(DeeObject *filename, unsigned int mode)
/*[[[end]]]*/
{
#if defined(POSIX_CREAT_OFLAGS) && defined(POSIX_CREAT_DEFAULT_FLAGS)
	return posix__open_f_impl(filename,
	                          POSIX_CREAT_OFLAGS |
	                          POSIX_CREAT_DEFAULT_FLAGS,
	                          mode);
#else /* POSIX_CREAT_OFLAGS && POSIX_CREAT_DEFAULT_FLAGS */
	return posix__creat_f_impl(filename, mode);
#endif /* !POSIX_CREAT_OFLAGS || !POSIX_CREAT_DEFAULT_FLAGS */
}






/************************************************************************/
/* openat()                                                             */
/************************************************************************/

/*[[[deemon import("rt.gen.dexutils").gw("_openat", "dfd:?X3?DFile?Dint?Dstring,filename:?Dstring,oflags:u,mode:u=0644->?Dint", libname: "posix"); ]]]*/
#define POSIX__OPENAT_DEF          DEX_MEMBER_F("_openat", &posix__openat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,filename:?Dstring,oflags:?Dint,mode=!0644)->?Dint"),
#define POSIX__OPENAT_DEF_DOC(doc) DEX_MEMBER_F("_openat", &posix__openat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,filename:?Dstring,oflags:?Dint,mode=!0644)->?Dint\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix__openat_f_impl(DeeObject *dfd, DeeObject *filename, unsigned int oflags, unsigned int mode);
#ifndef DEFINED_kwlist__dfd_filename_oflags_mode
#define DEFINED_kwlist__dfd_filename_oflags_mode
PRIVATE DEFINE_KWLIST(kwlist__dfd_filename_oflags_mode, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("filename", 0x199d68d3, 0x4a5d0431e1a3caed), KEX("oflags", 0xbe92b5be, 0x4f84e498f7c9d171), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEND });
#endif /* !DEFINED_kwlist__dfd_filename_oflags_mode */
PRIVATE WUNUSED DREF DeeObject *DCALL posix__openat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *dfd;
		DeeObject *filename;
		unsigned int oflags;
		unsigned int mode;
	} args;
	args.mode = 0644;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dfd_filename_oflags_mode, "oou|u:_openat", &args))
		goto err;
	return posix__openat_f_impl(args.dfd, args.filename, args.oflags, args.mode);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix__openat, &posix__openat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix__openat_f_impl(DeeObject *dfd, DeeObject *filename, unsigned int oflags, unsigned int mode)
/*[[[end]]]*/
{
#ifdef posix_openat_USE_posix_open
	DREF DeeObject *abspath, *result;

#if defined(posix_openat_USE_wopenat) || defined(posix_openat_USE_openat)
	if (!DeeString_Check(dfd)) {
		int result;
		int os_dfd;
#ifdef posix_openat_USE_wopenat
		Dee_wchar_t const *wide_filename;
#endif /* posix_openat_USE_wopenat */
#ifdef posix_openat_USE_openat
		char const *utf8_filename;
#endif /* posix_openat_USE_openat */
		os_dfd = DeeUnixSystem_GetFD(dfd);
		if unlikely(os_dfd == -1)
			goto err;
		if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
			goto err;
#ifdef posix_openat_USE_wopenat
		wide_filename = DeeString_AsWide(filename);
		if unlikely(!wide_filename)
			goto err;
#endif /* posix_openat_USE_wopenat */
#ifdef posix_openat_USE_openat
		utf8_filename = DeeString_AsUtf8(filename);
		if unlikely(!utf8_filename)
			goto err;
#endif /* posix_openat_USE_openat */

EINTR_ENOMEM_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
#ifdef posix_openat_USE_wopenat
		result = wopenat(os_dfd, (wchar_t *)wide_filename, (int)oflags, (int)mode);
#endif /* posix_openat_USE_wopenat */
#ifdef posix_openat_USE_openat
		result = openat(os_dfd, (char *)utf8_filename, (int)oflags, (int)mode);
#endif /* posix_openat_USE_openat */
		if (result >= 0) {
			DBG_ALIGNMENT_ENABLE();
			return DeeInt_NewUInt((unsigned int)result);
		}
		result = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		EINTR_HANDLE(result, again, err);
		ENOMEM_HANDLE(result, again, err);
		/* Fallthru to fallback path below */
	}
#endif /* posix_openat_USE_wopenat || posix_openat_USE_openat */
	abspath = posix_dfd_makepath(dfd, filename, POSIX_DFD_MAKEPATH_ATFLAGS_FROM_OFLAGS(oflags));
#define NEED_posix_dfd_makepath
	if unlikely(!abspath)
		goto err;
	result = posix__open_f_impl(abspath, oflags, mode);
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_openat_USE_posix_open */

#ifdef posix_openat_USE_STUB
#define NEED_posix_err_unsupported
	(void)dfd;
	(void)filename;
	(void)oflags;
	(void)mode;
	posix_err_unsupported("openat");
	return NULL;
#endif /* posix_openat_USE_STUB */
}

/*[[[deemon import("rt.gen.dexutils").gw("openat", "dfd:?X3?DFile?Dint?Dstring,filename:?Dstring,oflags:u,mode:u=0644->?Dint", libname: "posix"); ]]]*/
#define POSIX_OPENAT_DEF          DEX_MEMBER_F("openat", &posix_openat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,filename:?Dstring,oflags:?Dint,mode=!0644)->?Dint"),
#define POSIX_OPENAT_DEF_DOC(doc) DEX_MEMBER_F("openat", &posix_openat, DEXSYM_READONLY, "(dfd:?X3?DFile?Dint?Dstring,filename:?Dstring,oflags:?Dint,mode=!0644)->?Dint\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_openat_f_impl(DeeObject *dfd, DeeObject *filename, unsigned int oflags, unsigned int mode);
#ifndef DEFINED_kwlist__dfd_filename_oflags_mode
#define DEFINED_kwlist__dfd_filename_oflags_mode
PRIVATE DEFINE_KWLIST(kwlist__dfd_filename_oflags_mode, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("filename", 0x199d68d3, 0x4a5d0431e1a3caed), KEX("oflags", 0xbe92b5be, 0x4f84e498f7c9d171), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEND });
#endif /* !DEFINED_kwlist__dfd_filename_oflags_mode */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_openat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *dfd;
		DeeObject *filename;
		unsigned int oflags;
		unsigned int mode;
	} args;
	args.mode = 0644;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dfd_filename_oflags_mode, "oou|u:openat", &args))
		goto err;
	return posix_openat_f_impl(args.dfd, args.filename, args.oflags, args.mode);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_openat, &posix_openat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_openat_f_impl(DeeObject *dfd, DeeObject *filename, unsigned int oflags, unsigned int mode)
/*[[[end]]]*/
{
#ifdef POSIX_OPEN_DEFAULT_FLAGS
	oflags |= POSIX_OPEN_DEFAULT_FLAGS;
#endif /* POSIX_OPEN_DEFAULT_FLAGS */
	return posix__openat_f_impl(dfd, filename, oflags, mode);
}



DECL_END

#endif /* !GUARD_DEX_POSIX_P_OPEN_C_INL */
