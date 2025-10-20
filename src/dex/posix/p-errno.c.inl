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
#ifndef GUARD_DEX_POSIX_P_ERRNO_C_INL
#define GUARD_DEX_POSIX_P_ERRNO_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/dex.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/system-features.h>
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

#if !defined(ELIMIT) && defined(EMAX)
#define ELIMIT EMAX
#endif /* !ELIMIT && EMAX */
#if !defined(ELIMIT) && defined(ECOUNT)
#define ELIMIT (ECOUNT - 1)
#endif /* !ELIMIT && ECOUNT */


#ifdef __CYGWIN__
/* Prevent syntax errors because of `_Pragma' deprecation warnings
 * inside definitions, which in turn break use of these within
 * preprocessor expressions. */
#undef ENOSHARE
#undef ECASECLASH
#define ENOSHARE   136
#define ECASECLASH 137
#endif /* __CYGWIN__ */


/*[[[deemon
import * from deemon;
import * from rt.gen.dexutils;
MODULE_NAME = "posix";
local orig_stdout = File.stdout;

include("p-errno-constants.def");

local allDecls = [];
function e(name, doc = none) {
	allDecls.append(name);
	gii(name, doc: doc);
}

e("EOK",             doc: "Success");
e("EPERM",           doc: "Operation not permitted");
e("ENOENT",          doc: "No such file or directory");
e("ESRCH",           doc: "No such process");
e("EINTR",           doc: "Interrupted system call");
e("EIO",             doc: "I/O error");
e("ENXIO",           doc: "No such device or address");
e("E2BIG",           doc: "Argument list too long");
e("ENOEXEC",         doc: "Exec format error");
e("EBADF",           doc: "Bad file number");
e("ECHILD",          doc: "No child processes");
e("EAGAIN",          doc: "Try again");
e("ENOMEM",          doc: "Out of memory");
e("EACCES",          doc: "Permission denied");
e("EFAULT",          doc: "Bad address");
e("ENOTBLK",         doc: "Block device required");
e("EBUSY",           doc: "Device or resource busy");
e("EEXIST",          doc: "File exists");
e("EXDEV",           doc: "Cross-device link");
e("ENODEV",          doc: "No such device");
e("ENOTDIR",         doc: "Not a directory");
e("EISDIR",          doc: "Is a directory");
e("EINVAL",          doc: "Invalid argument");
e("ENFILE",          doc: "File table overflow");
e("EMFILE",          doc: "Too many open files");
e("ENOTTY",          doc: "Not a typewriter");
e("ETXTBSY",         doc: "Text file busy");
e("EFBIG",           doc: "File too large");
e("ENOSPC",          doc: "No space left on device");
e("ESPIPE",          doc: "Illegal seek");
e("EROFS",           doc: "Read-only file system");
e("EMLINK",          doc: "Too many links");
e("EPIPE",           doc: "Broken pipe");
e("EDOM",            doc: "Math argument out of domain of func");
e("ERANGE",          doc: "Math result not representable");
e("EDEADLK",         doc: "Resource deadlock would occur");
e("ENAMETOOLONG",    doc: "File name too long");
e("ENOLCK",          doc: "No record locks available");
e("ENOSYS",          doc: "Function not implemented");
e("ENOTEMPTY",       doc: "Directory not empty");
e("ELOOP",           doc: "Too many symbolic links encountered");
e("EWOULDBLOCK",     doc: "Operation would block");
e("ENOMSG",          doc: "No message of desired type");
e("EIDRM",           doc: "Identifier removed");
e("ECHRNG",          doc: "Channel number out of range");
e("EL2NSYNC",        doc: "Level 2 not synchronized");
e("EL3HLT",          doc: "Level 3 halted");
e("EL3RST",          doc: "Level 3 reset");
e("ELNRNG",          doc: "Link number out of range");
e("EUNATCH",         doc: "Protocol driver not attached");
e("ENOCSI",          doc: "No CSI structure available");
e("EL2HLT",          doc: "Level 2 halted");
e("EBADE",           doc: "Invalid exchange");
e("EBADR",           doc: "Invalid request descriptor");
e("EXFULL",          doc: "Exchange full");
e("ENOANO",          doc: "No anode");
e("EBADRQC",         doc: "Invalid request code");
e("EBADSLT",         doc: "Invalid slot");
e("EDEADLOCK",       doc: "Resource deadlock would occur");
e("EBFONT",          doc: "Bad font file format");
e("ENOSTR",          doc: "Device not a stream");
e("ENODATA",         doc: "No data available");
e("ETIME",           doc: "Timer expired");
e("ENOSR",           doc: "Out of streams resources");
e("ENONET",          doc: "Machine is not on the network");
e("ENOPKG",          doc: "Package not installed");
e("EREMOTE",         doc: "Object is remote");
e("ENOLINK",         doc: "Link has been severed");
e("EADV",            doc: "Advertise error");
e("ESRMNT",          doc: "Srmount error");
e("ECOMM",           doc: "Communication error on send");
e("EPROTO",          doc: "Protocol error");
e("EMULTIHOP",       doc: "Multihop attempted");
e("EDOTDOT",         doc: "RFS specific error");
e("EBADMSG",         doc: "Not a data message");
e("EOVERFLOW",       doc: "Value too large for defined data type");
e("ENOTUNIQ",        doc: "Name not unique on network");
e("EBADFD",          doc: "File descriptor in bad state");
e("EREMCHG",         doc: "Remote address changed");
e("ELIBACC",         doc: "Can not access a needed shared library");
e("ELIBBAD",         doc: "Accessing a corrupted shared library");
e("ELIBSCN",         doc: ".lib section in a.out corrupted");
e("ELIBMAX",         doc: "Attempting to link in too many shared libraries");
e("ELIBEXEC",        doc: "Cannot exec a shared library directly");
e("EILSEQ",          doc: "Illegal byte sequence");
e("ERESTART",        doc: "Interrupted system call should be restarted");
e("ESTRPIPE",        doc: "Streams pipe error");
e("EUSERS",          doc: "Too many users");
e("ENOTSOCK",        doc: "Socket operation on non-socket");
e("EDESTADDRREQ",    doc: "Destination address required");
e("EMSGSIZE",        doc: "Message too long");
e("EPROTOTYPE",      doc: "Protocol wrong type for socket");
e("ENOPROTOOPT",     doc: "Protocol not available");
e("EPROTONOSUPPORT", doc: "Protocol not supported");
e("ESOCKTNOSUPPORT", doc: "Socket type not supported");
e("EOPNOTSUPP",      doc: "Operation not supported on transport endpoint");
e("EPFNOSUPPORT",    doc: "Protocol family not supported");
e("EAFNOSUPPORT",    doc: "Address family not supported by protocol");
e("EADDRINUSE",      doc: "Address already in use");
e("EADDRNOTAVAIL",   doc: "Cannot assign requested address");
e("ENETDOWN",        doc: "Network is down");
e("ENETUNREACH",     doc: "Network is unreachable");
e("ENETRESET",       doc: "Network dropped connection because of reset");
e("ECONNABORTED",    doc: "Software caused connection abort");
e("ECONNRESET",      doc: "Connection reset by peer");
e("ENOBUFS",         doc: "No buffer space available");
e("EISCONN",         doc: "Transport endpoint is already connected");
e("ENOTCONN",        doc: "Transport endpoint is not connected");
e("ESHUTDOWN",       doc: "Cannot send after transport endpoint shutdown");
e("ETOOMANYREFS",    doc: "Too many references: cannot splice");
e("ETIMEDOUT",       doc: "Connection timed out");
e("ECONNREFUSED",    doc: "Connection refused");
e("EHOSTDOWN",       doc: "Host is down");
e("EHOSTUNREACH",    doc: "No route to host");
e("EALREADY",        doc: "Operation already in progress");
e("EINPROGRESS",     doc: "Operation now in progress");
e("ESTALE",          doc: "Stale NFS file handle");
e("EUCLEAN",         doc: "Structure needs cleaning");
e("ENOTNAM",         doc: "Not a XENIX named type file");
e("ENAVAIL",         doc: "No XENIX semaphores available");
e("EISNAM",          doc: "Is a named type file");
e("EREMOTEIO",       doc: "Remote I/O error");
e("EDQUOT",          doc: "Quota exceeded");
e("ENOMEDIUM",       doc: "No medium found");
e("EMEDIUMTYPE",     doc: "Wrong medium type");
e("ECANCELED",       doc: "Operation Canceled");
e("ENOKEY",          doc: "Required key not available");
e("EKEYEXPIRED",     doc: "Key has expired");
e("EKEYREVOKED",     doc: "Key has been revoked");
e("EKEYREJECTED",    doc: "Key was rejected by service");
e("EOWNERDEAD",      doc: "Owner died");
e("ENOTRECOVERABLE", doc: "State not recoverable");
e("ERFKILL",         doc: "Operation not possible due to RF-kill");
e("EHWPOISON",       doc: "Memory page has hardware error");
e("ELBIN",           doc: "Inode is remote (not really error)");
e("EPROCLIM");
e("EFTYPE",          doc: "Inappropriate file type or format");
e("ENMFILE",         doc: "No more files");
e("ENOTSUP",         doc: "Not supported");
e("ENOSHARE",        doc: "No such host or network path");
e("ECASECLASH",      doc: "Filename exists with different case");
e("EOTHER",          doc: "Other");
e("EAUTH");
e("EBADRPC");
e("ELASTERROR");
e("ELOCKUNMAPPED");
e("ENEEDAUTH");
e("ENOATTR");
e("ENOTACTIVE");
e("EPROCUNAVAIL");
e("EPROGMISMATCH");
e("EPROGUNAVAIL");
e("ERPCMISMATCH");
e("STRUNCATE",       doc: "Truncated");


print("#ifndef ELIMIT");
print("#define ELIMIT (-1)");
for (local x: allDecls) {
	print("#if defined(", x, ") && ", x, " > ELIMIT");
	print("#undef ELIMIT");
	print("#define ELIMIT ", x);
	print("#endif /" "* ", x, " && ", x, " > ELIMIT *" "/");
}
print("#if ELIMIT < 0");
print("#undef ELIMIT");
print("#endif /" "* ELIMIT < 0 *" "/");
print("#endif /" "* !ELIMIT *" "/");
print("#if !defined(ECOUNT) && defined(ELIMIT)");
print("#define ECOUNT (ELIMIT + 1)");
print("#endif /" "* !ECOUNT && ELIMIT *" "/");

e("ELIMIT",          doc: "Max possible errno");
e("ECOUNT",          doc: "One plus ?GELIMIT");

print("#ifdef ELIMIT");
print('#define POSIX_EMAX_DEF \\');
print('	{ "EMAX", (DeeObject *)&posix_ELIMIT, MODSYM_FREADONLY | MODSYM_FCONSTEXPR, "Max possible errno" },');
print("#else /" "* ELIMIT *" "/");
print('#define POSIX_EMAX_DEF /' '* nothing *' '/');
print("#endif /" "* !ELIMIT *" "/");
allDecls.append("EMAX");

File.stdout = orig_stdout;
print "#define POSIX_ERRNO_DEFS \\";
for (local x: allDecls)
	print("\tPOSIX_", x, "_DEF \\");
print "/" "**" "/";

]]]*/
#include "p-errno-constants.def"
#define POSIX_ERRNO_DEFS \
	POSIX_EOK_DEF \
	POSIX_EPERM_DEF \
	POSIX_ENOENT_DEF \
	POSIX_ESRCH_DEF \
	POSIX_EINTR_DEF \
	POSIX_EIO_DEF \
	POSIX_ENXIO_DEF \
	POSIX_E2BIG_DEF \
	POSIX_ENOEXEC_DEF \
	POSIX_EBADF_DEF \
	POSIX_ECHILD_DEF \
	POSIX_EAGAIN_DEF \
	POSIX_ENOMEM_DEF \
	POSIX_EACCES_DEF \
	POSIX_EFAULT_DEF \
	POSIX_ENOTBLK_DEF \
	POSIX_EBUSY_DEF \
	POSIX_EEXIST_DEF \
	POSIX_EXDEV_DEF \
	POSIX_ENODEV_DEF \
	POSIX_ENOTDIR_DEF \
	POSIX_EISDIR_DEF \
	POSIX_EINVAL_DEF \
	POSIX_ENFILE_DEF \
	POSIX_EMFILE_DEF \
	POSIX_ENOTTY_DEF \
	POSIX_ETXTBSY_DEF \
	POSIX_EFBIG_DEF \
	POSIX_ENOSPC_DEF \
	POSIX_ESPIPE_DEF \
	POSIX_EROFS_DEF \
	POSIX_EMLINK_DEF \
	POSIX_EPIPE_DEF \
	POSIX_EDOM_DEF \
	POSIX_ERANGE_DEF \
	POSIX_EDEADLK_DEF \
	POSIX_ENAMETOOLONG_DEF \
	POSIX_ENOLCK_DEF \
	POSIX_ENOSYS_DEF \
	POSIX_ENOTEMPTY_DEF \
	POSIX_ELOOP_DEF \
	POSIX_EWOULDBLOCK_DEF \
	POSIX_ENOMSG_DEF \
	POSIX_EIDRM_DEF \
	POSIX_ECHRNG_DEF \
	POSIX_EL2NSYNC_DEF \
	POSIX_EL3HLT_DEF \
	POSIX_EL3RST_DEF \
	POSIX_ELNRNG_DEF \
	POSIX_EUNATCH_DEF \
	POSIX_ENOCSI_DEF \
	POSIX_EL2HLT_DEF \
	POSIX_EBADE_DEF \
	POSIX_EBADR_DEF \
	POSIX_EXFULL_DEF \
	POSIX_ENOANO_DEF \
	POSIX_EBADRQC_DEF \
	POSIX_EBADSLT_DEF \
	POSIX_EDEADLOCK_DEF \
	POSIX_EBFONT_DEF \
	POSIX_ENOSTR_DEF \
	POSIX_ENODATA_DEF \
	POSIX_ETIME_DEF \
	POSIX_ENOSR_DEF \
	POSIX_ENONET_DEF \
	POSIX_ENOPKG_DEF \
	POSIX_EREMOTE_DEF \
	POSIX_ENOLINK_DEF \
	POSIX_EADV_DEF \
	POSIX_ESRMNT_DEF \
	POSIX_ECOMM_DEF \
	POSIX_EPROTO_DEF \
	POSIX_EMULTIHOP_DEF \
	POSIX_EDOTDOT_DEF \
	POSIX_EBADMSG_DEF \
	POSIX_EOVERFLOW_DEF \
	POSIX_ENOTUNIQ_DEF \
	POSIX_EBADFD_DEF \
	POSIX_EREMCHG_DEF \
	POSIX_ELIBACC_DEF \
	POSIX_ELIBBAD_DEF \
	POSIX_ELIBSCN_DEF \
	POSIX_ELIBMAX_DEF \
	POSIX_ELIBEXEC_DEF \
	POSIX_EILSEQ_DEF \
	POSIX_ERESTART_DEF \
	POSIX_ESTRPIPE_DEF \
	POSIX_EUSERS_DEF \
	POSIX_ENOTSOCK_DEF \
	POSIX_EDESTADDRREQ_DEF \
	POSIX_EMSGSIZE_DEF \
	POSIX_EPROTOTYPE_DEF \
	POSIX_ENOPROTOOPT_DEF \
	POSIX_EPROTONOSUPPORT_DEF \
	POSIX_ESOCKTNOSUPPORT_DEF \
	POSIX_EOPNOTSUPP_DEF \
	POSIX_EPFNOSUPPORT_DEF \
	POSIX_EAFNOSUPPORT_DEF \
	POSIX_EADDRINUSE_DEF \
	POSIX_EADDRNOTAVAIL_DEF \
	POSIX_ENETDOWN_DEF \
	POSIX_ENETUNREACH_DEF \
	POSIX_ENETRESET_DEF \
	POSIX_ECONNABORTED_DEF \
	POSIX_ECONNRESET_DEF \
	POSIX_ENOBUFS_DEF \
	POSIX_EISCONN_DEF \
	POSIX_ENOTCONN_DEF \
	POSIX_ESHUTDOWN_DEF \
	POSIX_ETOOMANYREFS_DEF \
	POSIX_ETIMEDOUT_DEF \
	POSIX_ECONNREFUSED_DEF \
	POSIX_EHOSTDOWN_DEF \
	POSIX_EHOSTUNREACH_DEF \
	POSIX_EALREADY_DEF \
	POSIX_EINPROGRESS_DEF \
	POSIX_ESTALE_DEF \
	POSIX_EUCLEAN_DEF \
	POSIX_ENOTNAM_DEF \
	POSIX_ENAVAIL_DEF \
	POSIX_EISNAM_DEF \
	POSIX_EREMOTEIO_DEF \
	POSIX_EDQUOT_DEF \
	POSIX_ENOMEDIUM_DEF \
	POSIX_EMEDIUMTYPE_DEF \
	POSIX_ECANCELED_DEF \
	POSIX_ENOKEY_DEF \
	POSIX_EKEYEXPIRED_DEF \
	POSIX_EKEYREVOKED_DEF \
	POSIX_EKEYREJECTED_DEF \
	POSIX_EOWNERDEAD_DEF \
	POSIX_ENOTRECOVERABLE_DEF \
	POSIX_ERFKILL_DEF \
	POSIX_EHWPOISON_DEF \
	POSIX_ELBIN_DEF \
	POSIX_EPROCLIM_DEF \
	POSIX_EFTYPE_DEF \
	POSIX_ENMFILE_DEF \
	POSIX_ENOTSUP_DEF \
	POSIX_ENOSHARE_DEF \
	POSIX_ECASECLASH_DEF \
	POSIX_EOTHER_DEF \
	POSIX_EAUTH_DEF \
	POSIX_EBADRPC_DEF \
	POSIX_ELASTERROR_DEF \
	POSIX_ELOCKUNMAPPED_DEF \
	POSIX_ENEEDAUTH_DEF \
	POSIX_ENOATTR_DEF \
	POSIX_ENOTACTIVE_DEF \
	POSIX_EPROCUNAVAIL_DEF \
	POSIX_EPROGMISMATCH_DEF \
	POSIX_EPROGUNAVAIL_DEF \
	POSIX_ERPCMISMATCH_DEF \
	POSIX_STRUNCATE_DEF \
	POSIX_ELIMIT_DEF \
	POSIX_ECOUNT_DEF \
	POSIX_EMAX_DEF \
/**/
/*[[[end]]]*/

#ifdef STRUNCATE
#define IS_ERRNONAME_FIRSTCHAR(ch) ((ch) == 'E' || (ch) == 'S')
#else /* STRUNCATE */
#define IS_ERRNONAME_FIRSTCHAR(ch) ((ch) == 'E')
#endif /* !STRUNCATE */
#define SYMBOL_NAME_IS_ERRNO(x) \
	(IS_ERRNONAME_FIRSTCHAR((x)[0]) && (isupper((x)[1]) || isdigit((x)[1]))/* && !strchr(x, '_')*/)



/************************************************************************/
/* errno                                                                */
/************************************************************************/

PRIVATE WUNUSED DREF DeeObject *DCALL
posix_errno_get_f(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("errno.get");]]]*/
	DeeArg_Unpack0(err, argc, argv, "errno.get");
/*[[[end]]]*/
#ifdef CONFIG_HAVE_errno
	return DeeInt_NewInt(DeeSystem_GetErrno());
#else /* CONFIG_HAVE_errno */
#define NEED_posix_err_unsupported
	posix_err_unsupported("errno");
#endif /* !CONFIG_HAVE_errno */
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
posix_errno_del_f(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("errno.del");]]]*/
	DeeArg_Unpack0(err, argc, argv, "errno.del");
/*[[[end]]]*/
#ifdef CONFIG_HAVE_errno
	DeeSystem_SetErrno(EOK);
	return_none;
#else /* CONFIG_HAVE_errno */
#define NEED_posix_err_unsupported
	posix_err_unsupported("errno");
#endif /* !CONFIG_HAVE_errno */
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
posix_errno_set_f(size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("errno.set", params: "int value");]]]*/
	struct {
		int value;
	} args;
	DeeArg_Unpack1X(err, argc, argv, "errno.set", &args.value, "d", DeeObject_AsInt);
/*[[[end]]]*/
#ifdef CONFIG_HAVE_errno
	DeeSystem_SetErrno(args.value);
	return_none;
#else /* CONFIG_HAVE_errno */
#define NEED_posix_err_unsupported
	posix_err_unsupported("errno");
#endif /* !CONFIG_HAVE_errno */
err:
	return NULL;
}

PRIVATE DEFINE_CMETHOD(posix_errno_get, &posix_errno_get_f, METHOD_FPURECALL);
PRIVATE DEFINE_CMETHOD(posix_errno_del, &posix_errno_del_f, METHOD_FNORMAL);
PRIVATE DEFINE_CMETHOD(posix_errno_set, &posix_errno_set_f, METHOD_FNORMAL);





/* Figure out how to implement `posix_strerror()' */
#undef posix_strerror_USE_strerrordesc_np
#undef posix_strerror_USE_sys_errlist
#undef posix_strerror_USE_strerror
#undef posix_strerror_USE_STUB
#undef posix_strerror_USE_DOCSTRINGS
#if !defined(CONFIG_HAVE_errno)
#define posix_strerror_USE_STUB
#elif defined(CONFIG_HAVE_strerrordesc_np)
#define posix_strerror_USE_strerrordesc_np
#define posix_strerror_USE_DOCSTRINGS
#elif defined(CONFIG_HAVE_sys_errlist) && defined(CONFIG_HAVE_sys_nerr)
#define posix_strerror_USE_sys_errlist
#define posix_strerror_USE_DOCSTRINGS
#elif defined(CONFIG_HAVE_strerror)
#define posix_strerror_USE_strerror
#define posix_strerror_USE_DOCSTRINGS
#else /* ... */
#define posix_strerror_USE_DOCSTRINGS
#endif /* !... */




/************************************************************************/
/* strerror()                                                           */
/************************************************************************/
/*[[[deemon import("rt.gen.dexutils").gw("strerror", "errnum?:d=DeeSystem_GetErrno()->?X2?Dstring?N", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_strerror_f_impl(int errnum);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_strerror_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_STRERROR_DEF { "strerror", (DeeObject *)&posix_strerror, MODSYM_FREADONLY, DOC("(errnum?:?Dint)->?X2?Dstring?N") },
#define POSIX_STRERROR_DEF_DOC(doc) { "strerror", (DeeObject *)&posix_strerror, MODSYM_FREADONLY, DOC("(errnum?:?Dint)->?X2?Dstring?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_strerror, &posix_strerror_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__errnum
#define DEFINED_kwlist__errnum
PRIVATE DEFINE_KWLIST(kwlist__errnum, { KEX("errnum", 0x60862d21, 0xe0c2e0432c8b5f4a), KEND });
#endif /* !DEFINED_kwlist__errnum */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_strerror_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		int errnum;
	} args;
	args.errnum = DeeSystem_GetErrno();
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__errnum, "|d:strerror", &args))
		goto err;
	return posix_strerror_f_impl(args.errnum);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_strerror_f_impl(int errnum)
/*[[[end]]]*/
{
#ifdef posix_strerror_USE_strerrordesc_np
	{
		char const *text;
		text = strerrordesc_np(errnum);
		if (text)
			return DeeString_NewUtf8(text, strlen(text), STRING_ERROR_FIGNORE);
	}
#endif /* posix_strerror_USE_strerrordesc_np */

#ifdef posix_strerror_USE_sys_errlist
	if (errnum >= 0 && errnum < sys_nerr) {
		char const *text;
		text = (char const *)sys_errlist[errnum];
		if (text)
			return DeeString_NewUtf8(text, strlen(text), STRING_ERROR_FIGNORE);
	}
#endif /* posix_strerror_USE_sys_errlist */

#ifdef posix_strerror_USE_DOCSTRINGS
	/* Search our own symbol table for error code.
	 * Since we use the errno message as doc string, we can simply return the doc here. */
	{
		struct dex_symbol *iter;
		for (iter = DEX.d_symbols;
		     iter->ds_name && SYMBOL_NAME_IS_ERRNO(iter->ds_name); ++iter) {
			int eval;
			if (!DeeInt_Check(iter->ds_obj))
				break;
			if (!DeeInt_TryAsInt(iter->ds_obj, &eval))
				break;
			if (eval == errnum) {
				char const *docstring;
				/* Found it! Now to simply return the doc string */
				docstring = iter->ds_doc;
				if (!docstring)
					break;
				return DeeString_NewUtf8(docstring,
				                         strlen(docstring),
				                         STRING_ERROR_FIGNORE);
			}
		}
	}
#endif /* !posix_strerror_USE_DOCSTRINGS */

#ifdef posix_strerror_USE_strerror
	{
		char const *text;
		text = strerror(errnum);
		if (text)
			return DeeString_NewUtf8(text, strlen(text), STRING_ERROR_FIGNORE);
	}
#endif /* posix_strerror_USE_strerror */

	return_none;
}





/* Figure out how to implement `posix_strerrorname()' */
#undef posix_strerrorname_USE_STUB
#undef posix_strerrorname_USE_strerrorname_np
#undef posix_strerrorname_USE_SYMBOLNAMES
#if !defined(CONFIG_HAVE_errno)
#define posix_strerrorname_USE_STUB
#elif defined(CONFIG_HAVE_strerrorname_np)
#define posix_strerrorname_USE_strerrorname_np
#define posix_strerrorname_USE_SYMBOLNAMES
#else /* ... */
#define posix_strerrorname_USE_SYMBOLNAMES
#endif /* !... */



/************************************************************************/
/* strerrorname()                                                       */
/************************************************************************/
/*[[[deemon import("rt.gen.dexutils").gw("strerrorname", "errnum?:d=DeeSystem_GetErrno()->?X2?Dstring?N", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_strerrorname_f_impl(int errnum);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_strerrorname_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_STRERRORNAME_DEF { "strerrorname", (DeeObject *)&posix_strerrorname, MODSYM_FREADONLY, DOC("(errnum?:?Dint)->?X2?Dstring?N") },
#define POSIX_STRERRORNAME_DEF_DOC(doc) { "strerrorname", (DeeObject *)&posix_strerrorname, MODSYM_FREADONLY, DOC("(errnum?:?Dint)->?X2?Dstring?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_strerrorname, &posix_strerrorname_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__errnum
#define DEFINED_kwlist__errnum
PRIVATE DEFINE_KWLIST(kwlist__errnum, { KEX("errnum", 0x60862d21, 0xe0c2e0432c8b5f4a), KEND });
#endif /* !DEFINED_kwlist__errnum */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_strerrorname_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		int errnum;
	} args;
	args.errnum = DeeSystem_GetErrno();
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__errnum, "|d:strerrorname", &args))
		goto err;
	return posix_strerrorname_f_impl(args.errnum);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_strerrorname_f_impl(int errnum)
/*[[[end]]]*/
{
#ifdef posix_strerrorname_USE_strerrorname_np
	{
		char const *text;
		text = strerrorname_np(errnum);
		if (text)
			return DeeString_NewUtf8(text, strlen(text), STRING_ERROR_FIGNORE);
	}
#endif /* posix_strerrorname_USE_strerrorname_np */

#ifdef posix_strerrorname_USE_SYMBOLNAMES
	/* Search our own symbol table for error code.
	 * Since we use the errno message as doc string, we can simply return the doc here. */
	{
		struct dex_symbol *iter;
		for (iter = DEX.d_symbols;
		     iter->ds_name && SYMBOL_NAME_IS_ERRNO(iter->ds_name); ++iter) {
			int eval;
			if (!DeeInt_Check(iter->ds_obj))
				break;
			if (!DeeInt_TryAsInt(iter->ds_obj, &eval))
				break;
			if (eval == errnum) {
				/* Found it! Now to simply return the symbol's name */
				return DeeString_New(iter->ds_name);
			}
		}
	}
#endif /* !posix_strerrorname_USE_SYMBOLNAMES */

	return_none;
}

#undef IS_ERRNONAME_FIRSTCHAR

DECL_END

#endif /* !GUARD_DEX_POSIX_P_ERRNO_C_INL */
