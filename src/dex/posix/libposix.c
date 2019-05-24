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
#ifndef GUARD_DEX_POSIX_LIBPOSIX_C
#define GUARD_DEX_POSIX_LIBPOSIX_C 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif

#include <deemon/arg.h>
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/exec.h>
#include <deemon/tuple.h>
#include <deemon/error.h>
#include <deemon/module.h>
#include <deemon/thread.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>

#include <hybrid/sched/yield.h>

DECL_BEGIN

/*[[[deemon
import * from _dexutils;
include("constants.def");
gii("SEEK_SET");
gii("SEEK_CUR");
gii("SEEK_END");
gii("SEEK_HOLE");
gii("SEEK_DATA");

gii("EPERM",           doc: "Operation not permitted");
gii("ENOENT",          doc: "No such file or directory");
gii("ESRCH",           doc: "No such process");
gii("EINTR",           doc: "Interrupted system call");
gii("EIO",             doc: "I/O error");
gii("ENXIO",           doc: "No such device or address");
gii("E2BIG",           doc: "Argument list too long");
gii("ENOEXEC",         doc: "Exec format error");
gii("EBADF",           doc: "Bad file number");
gii("ECHILD",          doc: "No child processes");
gii("EAGAIN",          doc: "Try again");
gii("ENOMEM",          doc: "Out of memory");
gii("EACCES",          doc: "Permission denied");
gii("EFAULT",          doc: "Bad address");
gii("ENOTBLK",         doc: "Block device required");
gii("EBUSY",           doc: "Device or resource busy");
gii("EEXIST",          doc: "File exists");
gii("EXDEV",           doc: "Cross-device link");
gii("ENODEV",          doc: "No such device");
gii("ENOTDIR",         doc: "Not a directory");
gii("EISDIR",          doc: "Is a directory");
gii("EINVAL",          doc: "Invalid argument");
gii("ENFILE",          doc: "File table overflow");
gii("EMFILE",          doc: "Too many open files");
gii("ENOTTY",          doc: "Not a typewriter");
gii("ETXTBSY",         doc: "Text file busy");
gii("EFBIG",           doc: "File too large");
gii("ENOSPC",          doc: "No space left on device");
gii("ESPIPE",          doc: "Illegal seek");
gii("EROFS",           doc: "Read-only file system");
gii("EMLINK",          doc: "Too many links");
gii("EPIPE",           doc: "Broken pipe");
gii("EDOM",            doc: "Math argument out of domain of func");
gii("ERANGE",          doc: "Math result not representable");
gii("EDEADLK",         doc: "Resource deadlock would occur");
gii("ENAMETOOLONG",    doc: "File name too long");
gii("ENOLCK",          doc: "No record locks available");
gii("ENOSYS",          doc: "Function not implemented");
gii("ENOTEMPTY",       doc: "Directory not empty");
gii("ELOOP",           doc: "Too many symbolic links encountered");
gii("EWOULDBLOCK",     doc: "Operation would block");
gii("ENOMSG",          doc: "No message of desired type");
gii("EIDRM",           doc: "Identifier removed");
gii("ECHRNG",          doc: "Channel number out of range");
gii("EL2NSYNC",        doc: "Level 2 not synchronized");
gii("EL3HLT",          doc: "Level 3 halted");
gii("EL3RST",          doc: "Level 3 reset");
gii("ELNRNG",          doc: "Link number out of range");
gii("EUNATCH",         doc: "Protocol driver not attached");
gii("ENOCSI",          doc: "No CSI structure available");
gii("EL2HLT",          doc: "Level 2 halted");
gii("EBADE",           doc: "Invalid exchange");
gii("EBADR",           doc: "Invalid request descriptor");
gii("EXFULL",          doc: "Exchange full");
gii("ENOANO",          doc: "No anode");
gii("EBADRQC",         doc: "Invalid request code");
gii("EBADSLT",         doc: "Invalid slot");
gii("EDEADLOCK",       doc: "Resource deadlock would occur");
gii("EBFONT",          doc: "Bad font file format");
gii("ENOSTR",          doc: "Device not a stream");
gii("ENODATA",         doc: "No data available");
gii("ETIME",           doc: "Timer expired");
gii("ENOSR",           doc: "Out of streams resources");
gii("ENONET",          doc: "Machine is not on the network");
gii("ENOPKG",          doc: "Package not installed");
gii("EREMOTE",         doc: "Object is remote");
gii("ENOLINK",         doc: "Link has been severed");
gii("EADV",            doc: "Advertise error");
gii("ESRMNT",          doc: "Srmount error");
gii("ECOMM",           doc: "Communication error on send");
gii("EPROTO",          doc: "Protocol error");
gii("EMULTIHOP",       doc: "Multihop attempted");
gii("EDOTDOT",         doc: "RFS specific error");
gii("EBADMSG",         doc: "Not a data message");
gii("EOVERFLOW",       doc: "Value too large for defined data type");
gii("ENOTUNIQ",        doc: "Name not unique on network");
gii("EBADFD",          doc: "File descriptor in bad state");
gii("EREMCHG",         doc: "Remote address changed");
gii("ELIBACC",         doc: "Can not access a needed shared library");
gii("ELIBBAD",         doc: "Accessing a corrupted shared library");
gii("ELIBSCN",         doc: ".lib section in a.out corrupted");
gii("ELIBMAX",         doc: "Attempting to link in too many shared libraries");
gii("ELIBEXEC",        doc: "Cannot exec a shared library directly");
gii("EILSEQ",          doc: "Illegal byte sequence");
gii("ERESTART",        doc: "Interrupted system call should be restarted");
gii("ESTRPIPE",        doc: "Streams pipe error");
gii("EUSERS",          doc: "Too many users");
gii("ENOTSOCK",        doc: "Socket operation on non-socket");
gii("EDESTADDRREQ",    doc: "Destination address required");
gii("EMSGSIZE",        doc: "Message too long");
gii("EPROTOTYPE",      doc: "Protocol wrong type for socket");
gii("ENOPROTOOPT",     doc: "Protocol not available");
gii("EPROTONOSUPPORT", doc: "Protocol not supported");
gii("ESOCKTNOSUPPORT", doc: "Socket type not supported");
gii("EOPNOTSUPP",      doc: "Operation not supported on transport endpoint");
gii("EPFNOSUPPORT",    doc: "Protocol family not supported");
gii("EAFNOSUPPORT",    doc: "Address family not supported by protocol");
gii("EADDRINUSE",      doc: "Address already in use");
gii("EADDRNOTAVAIL",   doc: "Cannot assign requested address");
gii("ENETDOWN",        doc: "Network is down");
gii("ENETUNREACH",     doc: "Network is unreachable");
gii("ENETRESET",       doc: "Network dropped connection because of reset");
gii("ECONNABORTED",    doc: "Software caused connection abort");
gii("ECONNRESET",      doc: "Connection reset by peer");
gii("ENOBUFS",         doc: "No buffer space available");
gii("EISCONN",         doc: "Transport endpoint is already connected");
gii("ENOTCONN",        doc: "Transport endpoint is not connected");
gii("ESHUTDOWN",       doc: "Cannot send after transport endpoint shutdown");
gii("ETOOMANYREFS",    doc: "Too many references: cannot splice");
gii("ETIMEDOUT",       doc: "Connection timed out");
gii("ECONNREFUSED",    doc: "Connection refused");
gii("EHOSTDOWN",       doc: "Host is down");
gii("EHOSTUNREACH",    doc: "No route to host");
gii("EALREADY",        doc: "Operation already in progress");
gii("EINPROGRESS",     doc: "Operation now in progress");
gii("ESTALE",          doc: "Stale NFS file handle");
gii("EUCLEAN",         doc: "Structure needs cleaning");
gii("ENOTNAM",         doc: "Not a XENIX named type file");
gii("ENAVAIL",         doc: "No XENIX semaphores available");
gii("EISNAM",          doc: "Is a named type file");
gii("EREMOTEIO",       doc: "Remote I/O error");
gii("EDQUOT",          doc: "Quota exceeded");
gii("ENOMEDIUM",       doc: "No medium found");
gii("EMEDIUMTYPE",     doc: "Wrong medium type");
gii("ECANCELED",       doc: "Operation Canceled");
gii("ENOKEY",          doc: "Required key not available");
gii("EKEYEXPIRED",     doc: "Key has expired");
gii("EKEYREVOKED",     doc: "Key has been revoked");
gii("EKEYREJECTED",    doc: "Key was rejected by service");
gii("EOWNERDEAD",      doc: "Owner died");
gii("ENOTRECOVERABLE", doc: "State not recoverable");
gii("ERFKILL",         doc: "Operation not possible due to RF-kill");
gii("EHWPOISON",       doc: "Memory page has hardware error");
gii("ELBIN",           doc: "Inode is remote (not really error)");
gii("EPROCLIM");
gii("EFTYPE",          doc: "Inappropriate file type or format");
gii("ENMFILE",         doc: "No more files");
gii("ENOTSUP",         doc: "Not supported");
gii("ENOSHARE",        doc: "No such host or network path");
gii("ECASECLASH",      doc: "Filename exists with different case");

gii("O_RDONLY");
gii("O_WRONLY");
gii("O_RDWR");
gii("O_APPEND",    doc: "Always append data to the end of the file");
gii("O_CREAT",     doc: "If missing, create a new file");
gii("O_TRUNC");
gii("O_EXCL",      doc: "When used with #O_CREAT, set #EEXIST if the file already exists");
gii("O_TEXT");
gii("O_BINARY");
gii("O_WTEXT");
gii("O_U16TEXT");
gii("O_U8TEXT");
gii("O_CLOEXEC",   doc: "Close the file during ${exec()}");
gii("O_TEMPORARY");
gii("O_SHORT_LIVED");
gii("O_OBTAIN_DIR");
gii("O_SEQUENTIAL");
gii("O_RANDOM");
gii("O_NOCTTY",    doc: "If the calling process does not have a controlling terminal assigned, do not attempt to assign the newly opened file as terminal, even when ${isatty(open(...))} would be true");
gii("O_NONBLOCK",  doc: "Do not block when trying to read data that hasn't been written, yet");
gii("O_SYNC");
gii("O_RSYNC");
gii("O_DSYNC");
gii("O_ASYNC");
gii("O_DIRECT");
gii("O_LARGEFILE");
gii("O_DIRECTORY", doc: "Set #ENOTDIR when the final path component of an open() system call turns out to be something other than a directory");
gii("O_NOFOLLOW",  doc: "Set #ELOOP when the final path component of an open() system call turns out to be a symbolic link");
gii("O_NOATIME",   doc: "Don't update last-accessed time stamps");
gii("O_PATH",      doc: "Open a path for *at system calls");
gii("O_TMPFILE");
gii("O_CLOFORK",   doc: "Close the handle when the file descriptors are unshared");
gii("O_SYMLINK",   doc: "Open a symlink itself, rather than dereferencing it");
gii("O_DOSPATH",   doc: "Interpret $\"\\\" as $\"/\", and ignore casing during path resolution. Additionally, recognize DOS mounting points, and interpret leading slashes as relative to the closest DOS mounting point");
gii("O_SHLOCK");
gii("O_EXLOCK");
gii("O_XATTR");
gii("O_EXEC");
gii("O_SEARCH");
gii("O_TTY_INIT");
gii("O_NOLINKS");

gii("EXIT_SUCCESS");
gii("EXIT_FAILURE");

gii("R_OK", doc: "Test for read permission");
gii("W_OK", doc: "Test for write permission");
gii("X_OK", doc: "Test for execute permission");
gii("F_OK", doc: "Test for existence");


gii("AT_FDCWD",            doc: "Special value used to indicate the *at functions should use the current working directory");
gii("AT_SYMLINK_NOFOLLOW", doc: "Do not follow symbolic links");
gii("AT_REMOVEDIR",        doc: "Remove directory instead of unlinking file");
gii("AT_SYMLINK_FOLLOW",   doc: "Follow symbolic links");
gii("AT_NO_AUTOMOUNT",     doc: "Suppress terminal automount traversal");
gii("AT_EMPTY_PATH",       doc: "Allow empty relative pathname");
gii("AT_EACCESS",          doc: "Test access permitted for effective IDs, not real IDs");
gii("AT_REMOVEREG",        doc: "Explicitly allow removing anything that unlink() removes. (Default; Set in addition to #AT_REMOVEDIR to implement #remove semantics)");
gii("AT_DOSPATH",          doc: "Interpret $\"\\\" as $\"/\", and ignore casing during path resolution");
gii("AT_FDROOT",           doc: "Same as #AT_FDCWD but sets the filesystem root (using this, you can #chroot with #dup2)");
gii("AT_THIS_TASK");
gii("AT_THIS_MMAN");
gii("AT_THIS_STACK");


]]]*/
#include "constants.def"
//[[[end]]]


PRIVATE ATTR_NOINLINE ATTR_UNUSED ATTR_COLD int DCALL
err_unsupported(char const *__restrict name) {
 return DeeError_Throwf(&DeeError_UnsupportedAPI,
                        "Unsupported function `%s'",
                         name);
}

#if defined(EINTR) && !defined(__INTELLISENSE__)
#define EINTR_LABEL(again)        again:
#define HANDLE_EINTR(error,again) if ((error) == EINTR) goto again;
#else
#define EINTR_LABEL(again)        /* nothing */
#define HANDLE_EINTR(error,again) /* nothing */
#endif

#if defined(ENOENT) && defined(ENOTDIR)
#define HANDLE_ENOENT_ENOTDIR(error,err_label,...) \
 if ((error) == ENOENT || (error) == ENOTDIR) { \
     DeeError_SysThrowf(&DeeError_FileNotFound,error,__VA_ARGS__); \
     goto err_label; \
 }
#elif defined(ENOENT)
#define HANDLE_ENOENT_ENOTDIR(error,err_label,...) \
 if ((error) == ENOENT) { \
     DeeError_SysThrowf(&DeeError_FileNotFound,error,__VA_ARGS__); \
     goto err_label; \
 }
#elif defined(ENOTDIR)
#define HANDLE_ENOENT_ENOTDIR(error,err_label,...) \
 if ((error) == ENOTDIR) { \
     DeeError_SysThrowf(&DeeError_FileNotFound,error,__VA_ARGS__); \
     goto err_label; \
 }
#else
#define HANDLE_ENOENT_ENOTDIR(error,err_label,...) /* nothing */
#endif


#if defined(ENXIO) && defined(EISDIR)
#define HANDLE_ENXIO_EISDIR(error,err_label,...) \
 if ((error) == ENXIO || (error) == EISDIR) { \
     DeeError_SysThrowf(&DeeError_ReadOnlyFile,error,__VA_ARGS__); \
     goto err_label; \
 }
#elif defined(ENXIO)
#define HANDLE_ENXIO_EISDIR(error,err_label,...) \
 if ((error) == ENXIO) { \
     DeeError_SysThrowf(&DeeError_ReadOnlyFile,error,__VA_ARGS__); \
     goto err_label; \
 }
#elif defined(EISDIR)
#define HANDLE_ENXIO_EISDIR(error,err_label,...) \
 if ((error) == EISDIR) { \
     DeeError_SysThrowf(&DeeError_ReadOnlyFile,error,__VA_ARGS__); \
     goto err_label; \
 }
#else
#define HANDLE_ENXIO_EISDIR(error,err_label,...) /* nothing */
#endif

#if defined(EROFS) && defined(ETXTBSY)
#define HANDLE_EROFS_ETXTBSY(error,err_label,...) \
 if ((error) == EROFS || (error) == ETXTBSY) { \
     DeeError_SysThrowf(&DeeError_ReadOnlyFile,error,__VA_ARGS__); \
     goto err_label; \
 }
#elif defined(EROFS)
#define HANDLE_EROFS_ETXTBSY(error,err_label,...) \
 if ((error) == EROFS) { \
     DeeError_SysThrowf(&DeeError_ReadOnlyFile,error,__VA_ARGS__); \
     goto err_label; \
 }
#elif defined(ETXTBSY)
#define HANDLE_EROFS_ETXTBSY(error,err_label,...) \
 if ((error) == ETXTBSY) { \
     DeeError_SysThrowf(&DeeError_ReadOnlyFile,error,__VA_ARGS__); \
     goto err_label; \
 }
#else
#define HANDLE_EROFS_ETXTBSY(error,err_label,...) /* nothing */
#endif



#if defined(EACCES)
#define HANDLE_EACCES(error,err_label,...) \
 if ((error) == EACCES) { \
     DeeError_SysThrowf(&DeeError_FileAccessError,error,__VA_ARGS__); \
     goto err_label; \
 }
#else
#define HANDLE_EACCES(error,err_label,...) /* nothing */
#endif

#if defined(EEXIST)
#define HANDLE_EEXIST_IF(error,cond,err_label,...) \
 if ((error) == EEXIST && (cond)) { \
     DeeError_SysThrowf(&DeeError_FileExists,error,__VA_ARGS__); \
     goto err_label; \
 }
#else
#define HANDLE_EEXIST_IF(error,cond,err_label,...) /* nothing */
#endif

#if defined(EINVAL)
#define HANDLE_EINVAL(error,err_label,...) \
 if ((error) == EINVAL) { \
     DeeError_Throwf(&DeeError_ValueError,__VA_ARGS__); \
     goto err_label; \
 }
#else
#define HANDLE_EINVAL(error,err_label,...) /* nothing */
#endif

#if defined(ENOMEM)
#define HANDLE_ENOMEM(error,err_label,...) \
 if ((error) == ENOMEM) { \
     DeeError_Throwf(&DeeError_NoMemory,__VA_ARGS__); \
     goto err_label; \
 }
#else
#define HANDLE_ENOMEM(error,err_label,...) /* nothing */
#endif

#if defined(EBADF)
#define HANDLE_EBADF(error,err_label,...) \
 if ((error) == EBADF) { \
     DeeError_Throwf(&DeeError_FileClosed,__VA_ARGS__); \
     goto err_label; \
 }
#else
#define HANDLE_EBADF(error,err_label,...) /* nothing */
#endif

#if defined(EFBIG) && defined(EINVAL)
#define HANDLE_EFBIG_EINVAL(error,err_label,...) \
 if ((error) == EFBIG || (error) == EINVAL) { \
     DeeError_Throwf(&DeeError_IntegerOverflow,__VA_ARGS__); \
     goto err_label; \
 }
#elif defined(EFBIG)
#define HANDLE_EFBIG_EINVAL(error,err_label,...) \
 if ((error) == EFBIG) { \
     DeeError_Throwf(&DeeError_IntegerOverflow,__VA_ARGS__); \
     goto err_label; \
 }
#elif defined(EINVAL)
#define HANDLE_EFBIG_EINVAL(error,err_label,...) \
 if ((error) == EINVAL) { \
     DeeError_Throwf(&DeeError_IntegerOverflow,__VA_ARGS__); \
     goto err_label; \
 }
#else
#define HANDLE_EFBIG_EINVAL(error,err_label,...) /* nothing */
#endif



#if defined(ENOSYS) && defined(ENOTSUP) && defined(EOPNOTSUPP)
#define HANDLE_ENOSYS(error,err_label,name) \
 if ((error) == ENOSYS || (error) == ENOTSUP || (error) == EOPNOTSUPP) { \
     err_unsupported(name); \
     goto err_label; \
 }
#elif defined(ENOSYS) && defined(ENOTSUP)
#define HANDLE_ENOSYS(error,err_label,name) \
 if ((error) == ENOSYS || (error) == ENOTSUP) { \
     err_unsupported(name); \
     goto err_label; \
 }
#elif defined(ENOSYS) && defined(EOPNOTSUPP)
#define HANDLE_ENOSYS(error,err_label,name) \
 if ((error) == ENOSYS || (error) == EOPNOTSUPP) { \
     err_unsupported(name); \
     goto err_label; \
 }
#elif defined(ENOTSUP) && defined(EOPNOTSUPP)
#define HANDLE_ENOSYS(error,err_label,name) \
 if ((error) == ENOTSUP || (error) == EOPNOTSUPP) { \
     err_unsupported(name); \
     goto err_label; \
 }
#elif defined(EOPNOTSUPP)
#define HANDLE_ENOSYS(error,err_label,name) \
 if ((error) == EOPNOTSUPP) { \
     err_unsupported(name); \
     goto err_label; \
 }
#elif defined(ENOTSUP)
#define HANDLE_ENOSYS(error,err_label,name) \
 if ((error) == ENOTSUP) { \
     err_unsupported(name); \
     goto err_label; \
 }
#elif defined(ENOSYS)
#define HANDLE_ENOSYS(error,err_label,name) \
 if ((error) == ENOSYS) { \
     err_unsupported(name); \
     goto err_label; \
 }
#else
#define HANDLE_ENOSYS(error,err_label,name) /* nothing */
#endif




#if defined(HAVE_WOPEN) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("open","filename:c:wchar_t[],oflags:u,mode:u=0644->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_open_f_impl(dwchar_t const *__restrict filename, unsigned int oflags, unsigned int mode);
PRIVATE DREF DeeObject *DCALL libposix_open_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_OPEN_DEF { "open", (DeeObject *)&libposix_open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=0644)->?Dint") },
#define LIBPOSIX_OPEN_DEF_DOC(doc) { "open", (DeeObject *)&libposix_open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_open,libposix_open_f);
#ifndef LIBPOSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED
#define LIBPOSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_filename_oflags_mode,{ K(filename), K(oflags), K(mode), KEND });
#endif /* !LIBPOSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_open_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	dwchar_t const *filename_str;
	DeeStringObject *filename;
	unsigned int oflags;
	unsigned int mode = 0644;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_filename_oflags_mode,"ou|u:open",&filename,&oflags,&mode))
	    goto err;
	if (DeeObject_AssertTypeExact(filename,&DeeString_Type))
	    goto err;
	filename_str = (dwchar_t const *)DeeString_AsWide((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return libposix_open_f_impl(filename_str,oflags,mode);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_open_f_impl(dwchar_t const *__restrict filename, unsigned int oflags, unsigned int mode)
//[[[end]]]
{
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = wopen(filename,(int)oflags,(int)mode);
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  result = errno;
  HANDLE_EINTR(result,again)
  HANDLE_ENOENT_ENOTDIR(result,err,"File or directory %ls could not be found",filename)
  HANDLE_EEXIST_IF(result,oflags & OPEN_FEXCL,err,"File %ls already exists",filename)
  HANDLE_EACCES(result,err,"Failed to access %ls",filename)
  HANDLE_ENXIO_EISDIR(result,err,"Cannot open directory %ls for writing",filename)
  HANDLE_EROFS_ETXTBSY(result,err,"Read-only file %ls",filename)
  HANDLE_ENOSYS(result,err,"open")
  DeeError_SysThrowf(&DeeError_FSError,result,
                     "Failed to open %ls",filename);
  goto err;
 }
 return DeeInt_NewUInt((unsigned int)result);
err:
 return NULL;
}
#endif

#if !defined(HAVE_WOPEN) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("open","filename:c:char[],oflags:u,mode:u=0644->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_open_f_impl(/*utf-8*/char const *__restrict filename, unsigned int oflags, unsigned int mode);
PRIVATE DREF DeeObject *DCALL libposix_open_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_OPEN_DEF { "open", (DeeObject *)&libposix_open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=0644)->?Dint") },
#define LIBPOSIX_OPEN_DEF_DOC(doc) { "open", (DeeObject *)&libposix_open, MODSYM_FNORMAL, DOC("(filename:?Dstring,oflags:?Dint,mode:?Dint=0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_open,libposix_open_f);
#ifndef LIBPOSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED
#define LIBPOSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_filename_oflags_mode,{ K(filename), K(oflags), K(mode), KEND });
#endif /* !LIBPOSIX_KWDS_FILENAME_OFLAGS_MODE_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_open_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	/*utf-8*/char const *filename_str;
	DeeStringObject *filename;
	unsigned int oflags;
	unsigned int mode = 0644;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_filename_oflags_mode,"ou|u:open",&filename,&oflags,&mode))
	    goto err;
	if (DeeObject_AssertTypeExact(filename,&DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return libposix_open_f_impl(filename_str,oflags,mode);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_open_f_impl(/*utf-8*/char const *__restrict filename, unsigned int oflags, unsigned int mode)
//[[[end]]]
{
#ifdef HAVE_FILEIO
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
#ifdef HAVE_OPEN64
 result = open64(filename,(int)oflags,(int)mode);
#else
 result = open(filename,(int)oflags,(int)mode);
#endif
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  result = errno;
  HANDLE_EINTR(result,again)
  HANDLE_ENOENT_ENOTDIR(result,err,"File or directory %s could not be found",filename)
  HANDLE_EEXIST_IF(result,oflags & OPEN_FEXCL,err,"File %s already exists",filename)
  HANDLE_EACCES(result,err,"Failed to access %s",filename)
  HANDLE_ENXIO_EISDIR(result,err,"Cannot open directory %s for writing",filename)
  HANDLE_EROFS_ETXTBSY(result,err,"Read-only file %s",filename)
  HANDLE_ENOSYS(result,err,"open")
  DeeError_SysThrowf(&DeeError_FSError,result,
                     "Failed to open %s",filename);
  goto err;
 }
 return DeeInt_NewUInt((unsigned int)result);
err:
 return NULL;
#else
 (void)filename;
 (void)oflags;
 (void)mode;
 err_unsupported("open");
 return NULL;
#endif
}
#endif




#if defined(HAVE_WCREAT) || defined(HAVE_WOPEN) || \
    defined(HAVE_WCREAT64) || defined(HAVE_WOPEN64) || \
    defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("creat","filename:c:wchar_t[],mode:u=0644->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_creat_f_impl(dwchar_t const *__restrict filename, unsigned int mode);
PRIVATE DREF DeeObject *DCALL libposix_creat_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_CREAT_DEF { "creat", (DeeObject *)&libposix_creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=0644)->?Dint") },
#define LIBPOSIX_CREAT_DEF_DOC(doc) { "creat", (DeeObject *)&libposix_creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_creat,libposix_creat_f);
#ifndef LIBPOSIX_KWDS_FILENAME_MODE_DEFINED
#define LIBPOSIX_KWDS_FILENAME_MODE_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_filename_mode,{ K(filename), K(mode), KEND });
#endif /* !LIBPOSIX_KWDS_FILENAME_MODE_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_creat_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	dwchar_t const *filename_str;
	DeeStringObject *filename;
	unsigned int mode = 0644;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_filename_mode,"o|u:creat",&filename,&mode))
	    goto err;
	if (DeeObject_AssertTypeExact(filename,&DeeString_Type))
	    goto err;
	filename_str = (dwchar_t const *)DeeString_AsWide((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return libposix_creat_f_impl(filename_str,mode);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_creat_f_impl(dwchar_t const *__restrict filename, unsigned int mode)
//[[[end]]]
{
#if defined(HAVE_FILEIO) && (defined(HAVE_CREAT) || \
   (defined(O_CREAT) && (defined(O_WRONLY) || defined(O_RDWR)) && defined(O_TRUNC)))
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
#ifdef HAVE_WCREAT64
 result = wcreat64(filename,(int)mode);
#elif defined(HAVE_WCREAT)
 result = wcreat(filename,(int)mode);
#elif defined(O_WRONLY) && defined(HAVE_WOPEN64)
 result = wopen64(filename,O_CREAT|O_WRONLY|O_TRUNC,(int)mode);
#elif defined(O_WRONLY)
 result = wopen(filename,O_CREAT|O_WRONLY|O_TRUNC,(int)mode);
#elif defined(HAVE_WOPEN64)
 result = wopen64(filename,O_CREAT|O_RDWR|O_TRUNC,(int)mode);
#else
 result = wopen(filename,O_CREAT|O_RDWR|O_TRUNC,(int)mode);
#endif
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  result = errno;
  HANDLE_EINTR(result,again)
  HANDLE_ENOENT_ENOTDIR(result,err,"File or directory %ls could not be found",filename)
  HANDLE_EACCES(result,err,"Failed to access %ls",filename)
  HANDLE_ENXIO_EISDIR(result,err,"Cannot open directory %ls for writing",filename)
  HANDLE_EROFS_ETXTBSY(result,err,"Read-only file %ls",filename)
  HANDLE_ENOSYS(result,err,"creat")
  DeeError_SysThrowf(&DeeError_FSError,result,
                     "Failed to open %ls",filename);
  goto err;
 }
 return DeeInt_NewUInt((unsigned int)result);
err:
 return NULL;
#else
 (void)filename;
 (void)mode;
 err_unsupported("creat");
 return NULL;
#endif
}
#endif
#if !(defined(HAVE_WCREAT) || defined(HAVE_WOPEN) || \
     defined(HAVE_WCREAT64) || defined(HAVE_WOPEN64)) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("creat","filename:c:char[],mode:u=0644->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_creat_f_impl(/*utf-8*/char const *__restrict filename, unsigned int mode);
PRIVATE DREF DeeObject *DCALL libposix_creat_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_CREAT_DEF { "creat", (DeeObject *)&libposix_creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=0644)->?Dint") },
#define LIBPOSIX_CREAT_DEF_DOC(doc) { "creat", (DeeObject *)&libposix_creat, MODSYM_FNORMAL, DOC("(filename:?Dstring,mode:?Dint=0644)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_creat,libposix_creat_f);
#ifndef LIBPOSIX_KWDS_FILENAME_MODE_DEFINED
#define LIBPOSIX_KWDS_FILENAME_MODE_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_filename_mode,{ K(filename), K(mode), KEND });
#endif /* !LIBPOSIX_KWDS_FILENAME_MODE_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_creat_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	/*utf-8*/char const *filename_str;
	DeeStringObject *filename;
	unsigned int mode = 0644;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_filename_mode,"o|u:creat",&filename,&mode))
	    goto err;
	if (DeeObject_AssertTypeExact(filename,&DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return libposix_creat_f_impl(filename_str,mode);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_creat_f_impl(/*utf-8*/char const *__restrict filename, unsigned int mode)
//[[[end]]]
{
#if defined(HAVE_FILEIO) && (defined(HAVE_CREAT) || \
   (defined(O_CREAT) && (defined(O_WRONLY) || defined(O_RDWR)) && defined(O_TRUNC)))
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
#ifdef HAVE_CREAT64
 result = creat64(filename,(int)mode);
#elif defined(HAVE_CREAT)
 result = creat(filename,(int)mode);
#elif defined(O_WRONLY) && defined(HAVE_OPEN64)
 result = open64(filename,O_CREAT|O_WRONLY|O_TRUNC,(int)mode);
#elif defined(O_WRONLY)
 result = open(filename,O_CREAT|O_WRONLY|O_TRUNC,(int)mode);
#elif defined(HAVE_OPEN64)
 result = open64(filename,O_CREAT|O_RDWR|O_TRUNC,(int)mode);
#else
 result = open(filename,O_CREAT|O_RDWR|O_TRUNC,(int)mode);
#endif
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  result = errno;
  HANDLE_EINTR(result,again)
  HANDLE_ENOENT_ENOTDIR(result,err,"File or directory %s could not be found",filename)
  HANDLE_EACCES(result,err,"Failed to access %s",filename)
  HANDLE_ENXIO_EISDIR(result,err,"Cannot open directory %s for writing",filename)
  HANDLE_EROFS_ETXTBSY(result,err,"Read-only file %s",filename)
  HANDLE_ENOSYS(result,err,"creat")
  DeeError_SysThrowf(&DeeError_FSError,result,
                     "Failed to open %s",filename);
  goto err;
 }
 return DeeInt_NewUInt((unsigned int)result);
err:
 return NULL;
#else
 (void)filename;
 (void)mode;
 err_unsupported("creat");
 return NULL;
#endif
}
#endif



/*[[[deemon import("_dexutils").gw("read","fd:d,buf:obj:buffer->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_read_f_impl(int fd, DeeObject *__restrict buf);
PRIVATE DREF DeeObject *DCALL libposix_read_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_READ_DEF { "read", (DeeObject *)&libposix_read, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes)->?Dint") },
#define LIBPOSIX_READ_DEF_DOC(doc) { "read", (DeeObject *)&libposix_read, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_read,libposix_read_f);
#ifndef LIBPOSIX_KWDS_FD_BUF_DEFINED
#define LIBPOSIX_KWDS_FD_BUF_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd_buf,{ K(fd), K(buf), KEND });
#endif /* !LIBPOSIX_KWDS_FD_BUF_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_read_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	DeeObject *buf;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd_buf,"do:read",&fd,&buf))
	    goto err;
	return libposix_read_f_impl(fd,buf);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_read_f_impl(int fd, DeeObject *__restrict buf)
//[[[end]]]
{
#ifdef HAVE_FILEIO
 DeeBuffer buffer;
 Dee_ssize_t result_value;
 if (DeeObject_GetBuf(buf,&buffer,Dee_BUFFER_FWRITABLE))
     goto err;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result_value = (Dee_ssize_t)read(fd,buffer.bb_base,buffer.bb_size);
 DBG_ALIGNMENT_ENABLE();
 if (result_value < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  DeeObject_PutBuf(buf,&buffer,Dee_BUFFER_FWRITABLE);
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  DeeError_SysThrowf(&DeeError_FSError,error,
                     "Failed to read from %d",fd);
  goto err;
 }
 DeeObject_PutBuf(buf,&buffer,Dee_BUFFER_FWRITABLE);
 return DeeInt_NewSSize(result_value);
err:
#else
 (void)fd;
 (void)buf;
 err_unsupported("read");
#endif
 return NULL;
}

/*[[[deemon import("_dexutils").gw("write","fd:d,buf:obj:buffer->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_write_f_impl(int fd, DeeObject *__restrict buf);
PRIVATE DREF DeeObject *DCALL libposix_write_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_WRITE_DEF { "write", (DeeObject *)&libposix_write, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes)->?Dint") },
#define LIBPOSIX_WRITE_DEF_DOC(doc) { "write", (DeeObject *)&libposix_write, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_write,libposix_write_f);
#ifndef LIBPOSIX_KWDS_FD_BUF_DEFINED
#define LIBPOSIX_KWDS_FD_BUF_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd_buf,{ K(fd), K(buf), KEND });
#endif /* !LIBPOSIX_KWDS_FD_BUF_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_write_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	DeeObject *buf;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd_buf,"do:write",&fd,&buf))
	    goto err;
	return libposix_write_f_impl(fd,buf);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_write_f_impl(int fd, DeeObject *__restrict buf)
//[[[end]]]
{
#ifdef HAVE_FILEIO
 DeeBuffer buffer;
 Dee_ssize_t result_value;
 if (DeeObject_GetBuf(buf,&buffer,Dee_BUFFER_FREADONLY))
     goto err;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result_value = (Dee_ssize_t)write(fd,buffer.bb_base,buffer.bb_size);
 DBG_ALIGNMENT_ENABLE();
 if (result_value < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  DeeObject_PutBuf(buf,&buffer,Dee_BUFFER_FREADONLY);
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  DeeError_SysThrowf(&DeeError_FSError,error,
                     "Failed to write to %d",fd);
  goto err;
 }
 DeeObject_PutBuf(buf,&buffer,Dee_BUFFER_FREADONLY);
 return DeeInt_NewSSize(result_value);
#else
 (void)fd;
 (void)buf;
 err_unsupported("write");
#endif
err:
 return NULL;
}



#if defined(HAVE_PREAD64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("pread","fd:d,buf:obj:buffer,offset:I64d->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_pread_f_impl(int fd, DeeObject *__restrict buf, int64_t offset);
PRIVATE DREF DeeObject *DCALL libposix_pread_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_PREAD_DEF { "pread", (DeeObject *)&libposix_pread, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint") },
#define LIBPOSIX_PREAD_DEF_DOC(doc) { "pread", (DeeObject *)&libposix_pread, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_pread,libposix_pread_f);
#ifndef LIBPOSIX_KWDS_FD_BUF_OFFSET_DEFINED
#define LIBPOSIX_KWDS_FD_BUF_OFFSET_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd_buf_offset,{ K(fd), K(buf), K(offset), KEND });
#endif /* !LIBPOSIX_KWDS_FD_BUF_OFFSET_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_pread_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	DeeObject *buf;
	int64_t offset;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd_buf_offset,"doI64d:pread",&fd,&buf,&offset))
	    goto err;
	return libposix_pread_f_impl(fd,buf,offset);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_pread_f_impl(int fd, DeeObject *__restrict buf, int64_t offset)
//[[[end]]]
{
 DeeBuffer buffer;
 Dee_ssize_t result_value;
 if (DeeObject_GetBuf(buf,&buffer,Dee_BUFFER_FWRITABLE))
     goto err;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result_value = (Dee_ssize_t)pread64(fd,buffer.bb_base,buffer.bb_size,offset);
 DBG_ALIGNMENT_ENABLE();
 if (result_value < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  DeeObject_PutBuf(buf,&buffer,Dee_BUFFER_FWRITABLE);
  HANDLE_ENOSYS(error,err,"pread")
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  DeeError_SysThrowf(&DeeError_FSError,error,
                     "Failed to read from %d",fd);
  goto err;
 }
 DeeObject_PutBuf(buf,&buffer,Dee_BUFFER_FWRITABLE);
 return DeeInt_NewSSize(result_value);
err:
 return NULL;
}
#endif
#if !defined(HAVE_PREAD64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("pread","fd:d,buf:obj:buffer,offset:I32d->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_pread_f_impl(int fd, DeeObject *__restrict buf, int32_t offset);
PRIVATE DREF DeeObject *DCALL libposix_pread_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_PREAD_DEF { "pread", (DeeObject *)&libposix_pread, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint") },
#define LIBPOSIX_PREAD_DEF_DOC(doc) { "pread", (DeeObject *)&libposix_pread, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_pread,libposix_pread_f);
#ifndef LIBPOSIX_KWDS_FD_BUF_OFFSET_DEFINED
#define LIBPOSIX_KWDS_FD_BUF_OFFSET_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd_buf_offset,{ K(fd), K(buf), K(offset), KEND });
#endif /* !LIBPOSIX_KWDS_FD_BUF_OFFSET_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_pread_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	DeeObject *buf;
	int32_t offset;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd_buf_offset,"doI32d:pread",&fd,&buf,&offset))
	    goto err;
	return libposix_pread_f_impl(fd,buf,offset);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_pread_f_impl(int fd, DeeObject *__restrict buf, int32_t offset)
//[[[end]]]
{
#ifdef HAVE_PREAD
 DeeBuffer buffer;
 Dee_ssize_t result_value;
 if (DeeObject_GetBuf(buf,&buffer,Dee_BUFFER_FWRITABLE))
     goto err;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result_value = (Dee_ssize_t)pread(fd,buffer.bb_base,buffer.bb_size,offset);
 DBG_ALIGNMENT_ENABLE();
 if (result_value < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  DeeObject_PutBuf(buf,&buffer,Dee_BUFFER_FWRITABLE);
  HANDLE_ENOSYS(error,err,"pread")
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  DeeError_SysThrowf(&DeeError_FSError,error,
                     "Failed to read from %d",fd);
  goto err;
 }
 DeeObject_PutBuf(buf,&buffer,Dee_BUFFER_FWRITABLE);
 return DeeInt_NewSSize(result_value);
err:
#else
 (void)fd;
 (void)buf;
 (void)offset;
 err_unsupported("pread");
#endif
 return NULL;
}
#endif

#if defined(HAVE_PWRITE64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("pwrite","fd:d,buf:obj:buffer,offset:I64d->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_pwrite_f_impl(int fd, DeeObject *__restrict buf, int64_t offset);
PRIVATE DREF DeeObject *DCALL libposix_pwrite_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_PWRITE_DEF { "pwrite", (DeeObject *)&libposix_pwrite, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint") },
#define LIBPOSIX_PWRITE_DEF_DOC(doc) { "pwrite", (DeeObject *)&libposix_pwrite, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_pwrite,libposix_pwrite_f);
#ifndef LIBPOSIX_KWDS_FD_BUF_OFFSET_DEFINED
#define LIBPOSIX_KWDS_FD_BUF_OFFSET_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd_buf_offset,{ K(fd), K(buf), K(offset), KEND });
#endif /* !LIBPOSIX_KWDS_FD_BUF_OFFSET_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_pwrite_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	DeeObject *buf;
	int64_t offset;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd_buf_offset,"doI64d:pwrite",&fd,&buf,&offset))
	    goto err;
	return libposix_pwrite_f_impl(fd,buf,offset);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_pwrite_f_impl(int fd, DeeObject *__restrict buf, int64_t offset)
//[[[end]]]
{
 DeeBuffer buffer;
 Dee_ssize_t result_value;
 if (DeeObject_GetBuf(buf,&buffer,Dee_BUFFER_FREADONLY))
     goto err;
 EINTR_LABEL(again)
 if (DeeThwrite_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result_value = (Dee_ssize_t)pwrite64(fd,buffer.bb_base,buffer.bb_size,offset);
 DBG_ALIGNMENT_ENABLE();
 if (result_value < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  DeeObject_PutBuf(buf,&buffer,Dee_BUFFER_FREADONLY);
  HANDLE_ENOSYS(error,err,"pwrite")
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  DeeError_SysThrowf(&DeeError_FSError,error,
                     "Failed to write to %d",fd);
  goto err;
 }
 DeeObject_PutBuf(buf,&buffer,Dee_BUFFER_FREADONLY);
 return DeeInt_NewSSize(result_value);
err:
 return NULL;
}
#endif
#if !defined(HAVE_PWRITE64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("pwrite","fd:d,buf:obj:buffer,offset:I32d->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_pwrite_f_impl(int fd, DeeObject *__restrict buf, int32_t offset);
PRIVATE DREF DeeObject *DCALL libposix_pwrite_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_PWRITE_DEF { "pwrite", (DeeObject *)&libposix_pwrite, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint") },
#define LIBPOSIX_PWRITE_DEF_DOC(doc) { "pwrite", (DeeObject *)&libposix_pwrite, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_pwrite,libposix_pwrite_f);
#ifndef LIBPOSIX_KWDS_FD_BUF_OFFSET_DEFINED
#define LIBPOSIX_KWDS_FD_BUF_OFFSET_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd_buf_offset,{ K(fd), K(buf), K(offset), KEND });
#endif /* !LIBPOSIX_KWDS_FD_BUF_OFFSET_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_pwrite_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	DeeObject *buf;
	int32_t offset;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd_buf_offset,"doI32d:pwrite",&fd,&buf,&offset))
	    goto err;
	return libposix_pwrite_f_impl(fd,buf,offset);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_pwrite_f_impl(int fd, DeeObject *__restrict buf, int32_t offset)
//[[[end]]]
{
#ifdef HAVE_PWRITE
 DeeBuffer buffer;
 Dee_ssize_t result_value;
 if (DeeObject_GetBuf(buf,&buffer,Dee_BUFFER_FREADONLY))
     goto err;
 EINTR_LABEL(again)
 if (DeeThwrite_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result_value = (Dee_ssize_t)pwrite(fd,buffer.bb_base,buffer.bb_size,offset);
 DBG_ALIGNMENT_ENABLE();
 if (result_value < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  DeeObject_PutBuf(buf,&buffer,Dee_BUFFER_FREADONLY);
  HANDLE_ENOSYS(error,err,"pwrite")
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  DeeError_SysThrowf(&DeeError_FSError,error,
                     "Failed to write to %d",fd);
  goto err;
 }
 DeeObject_PutBuf(buf,&buffer,Dee_BUFFER_FREADONLY);
 return DeeInt_NewSSize(result_value);
err:
#else
 (void)fd;
 (void)buf;
 (void)offset;
 err_unsupported("pwrite");
#endif
 return NULL;
}
#endif

#if defined(HAVE_LSEEK64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("lseek","fd:d,off:I64d,whence:d->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_lseek_f_impl(int fd, int64_t off, int whence);
PRIVATE DREF DeeObject *DCALL libposix_lseek_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_LSEEK_DEF { "lseek", (DeeObject *)&libposix_lseek, MODSYM_FNORMAL, DOC("(fd:?Dint,off:?Dint,whence:?Dint)->?Dint") },
#define LIBPOSIX_LSEEK_DEF_DOC(doc) { "lseek", (DeeObject *)&libposix_lseek, MODSYM_FNORMAL, DOC("(fd:?Dint,off:?Dint,whence:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_lseek,libposix_lseek_f);
#ifndef LIBPOSIX_KWDS_FD_OFF_WHENCE_DEFINED
#define LIBPOSIX_KWDS_FD_OFF_WHENCE_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd_off_whence,{ K(fd), K(off), K(whence), KEND });
#endif /* !LIBPOSIX_KWDS_FD_OFF_WHENCE_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_lseek_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	int64_t off;
	int whence;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd_off_whence,"dI64dd:lseek",&fd,&off,&whence))
	    goto err;
	return libposix_lseek_f_impl(fd,off,whence);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_lseek_f_impl(int fd, int64_t off, int whence)
//[[[end]]]
{
 int64_t result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = lseek64(fd,off,whence);
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_ENOSYS(error,err,"lseek")
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  DeeError_SysThrowf(&DeeError_FSError,error,
                     "Failed to seek %d",fd);
  goto err;
 }
 return DeeInt_NewS64(result);
err:
 return NULL;
}
#endif

#if !defined(HAVE_LSEEK64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("lseek","fd:d,off:I32d,whence:d->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_lseek_f_impl(int fd, int32_t off, int whence);
PRIVATE DREF DeeObject *DCALL libposix_lseek_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_LSEEK_DEF { "lseek", (DeeObject *)&libposix_lseek, MODSYM_FNORMAL, DOC("(fd:?Dint,off:?Dint,whence:?Dint)->?Dint") },
#define LIBPOSIX_LSEEK_DEF_DOC(doc) { "lseek", (DeeObject *)&libposix_lseek, MODSYM_FNORMAL, DOC("(fd:?Dint,off:?Dint,whence:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_lseek,libposix_lseek_f);
#ifndef LIBPOSIX_KWDS_FD_OFF_WHENCE_DEFINED
#define LIBPOSIX_KWDS_FD_OFF_WHENCE_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd_off_whence,{ K(fd), K(off), K(whence), KEND });
#endif /* !LIBPOSIX_KWDS_FD_OFF_WHENCE_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_lseek_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	int32_t off;
	int whence;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd_off_whence,"dI32dd:lseek",&fd,&off,&whence))
	    goto err;
	return libposix_lseek_f_impl(fd,off,whence);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_lseek_f_impl(int fd, int32_t off, int whence)
//[[[end]]]
{
#ifdef HAVE_FILEIO
 int32_t result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = lseek(fd,off,whence);
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_ENOSYS(error,err,"lseek")
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  DeeError_SysThrowf(&DeeError_FSError,error,
                     "Failed to seek %d",fd);
  goto err;
 }
 return DeeInt_NewS32(result);
err:
#else
 (void)fd;
 (void)off;
 (void)whence;
 err_unsupported("lseek");
#endif
 return NULL;
}
#endif

/*[[[deemon import("_dexutils").gw("close","fd:d"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_close_f_impl(int fd);
PRIVATE DREF DeeObject *DCALL libposix_close_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_CLOSE_DEF { "close", (DeeObject *)&libposix_close, MODSYM_FNORMAL, DOC("(fd:?Dint)") },
#define LIBPOSIX_CLOSE_DEF_DOC(doc) { "close", (DeeObject *)&libposix_close, MODSYM_FNORMAL, DOC("(fd:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_close,libposix_close_f);
#ifndef LIBPOSIX_KWDS_FD_DEFINED
#define LIBPOSIX_KWDS_FD_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd,{ K(fd), KEND });
#endif /* !LIBPOSIX_KWDS_FD_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_close_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd,"d:close",&fd))
	    goto err;
	return libposix_close_f_impl(fd);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_close_f_impl(int fd)
//[[[end]]]
{
 int result;
#ifdef HAVE_FILEIO
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = close(fd);
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_ENOSYS(error,err,"close")
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  DeeError_SysThrowf(&DeeError_FSError,error,
                     "Failed to close %d",fd);
  goto err;
 }
 return_none;
err:
#else
 (void)fd;
 err_unsupported("close");
#endif
 return NULL;
}


/*[[[deemon import("_dexutils").gw("fsync","fd:d"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_fsync_f_impl(int fd);
PRIVATE DREF DeeObject *DCALL libposix_fsync_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_FSYNC_DEF { "fsync", (DeeObject *)&libposix_fsync, MODSYM_FNORMAL, DOC("(fd:?Dint)") },
#define LIBPOSIX_FSYNC_DEF_DOC(doc) { "fsync", (DeeObject *)&libposix_fsync, MODSYM_FNORMAL, DOC("(fd:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_fsync,libposix_fsync_f);
#ifndef LIBPOSIX_KWDS_FD_DEFINED
#define LIBPOSIX_KWDS_FD_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd,{ K(fd), KEND });
#endif /* !LIBPOSIX_KWDS_FD_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_fsync_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd,"d:fsync",&fd))
	    goto err;
	return libposix_fsync_f_impl(fd);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_fsync_f_impl(int fd)
//[[[end]]]
{
 int result;
#ifdef HAVE_FSYNC
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = fsync(fd);
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  HANDLE_ENOSYS(error,err,"fsync")
  DeeError_SysThrowf(&DeeError_FSError,error,
                     "Failed to sync %d",fd);
  goto err;
 }
 return_none;
err:
#else
 (void)fd;
 err_unsupported("fsync");
#endif
 return NULL;
}

/*[[[deemon import("_dexutils").gw("fdatasync","fd:d"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_fdatasync_f_impl(int fd);
PRIVATE DREF DeeObject *DCALL libposix_fdatasync_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_FDATASYNC_DEF { "fdatasync", (DeeObject *)&libposix_fdatasync, MODSYM_FNORMAL, DOC("(fd:?Dint)") },
#define LIBPOSIX_FDATASYNC_DEF_DOC(doc) { "fdatasync", (DeeObject *)&libposix_fdatasync, MODSYM_FNORMAL, DOC("(fd:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_fdatasync,libposix_fdatasync_f);
#ifndef LIBPOSIX_KWDS_FD_DEFINED
#define LIBPOSIX_KWDS_FD_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd,{ K(fd), KEND });
#endif /* !LIBPOSIX_KWDS_FD_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_fdatasync_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd,"d:fdatasync",&fd))
	    goto err;
	return libposix_fdatasync_f_impl(fd);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_fdatasync_f_impl(int fd)
//[[[end]]]
{
#ifdef HAVE_FDATASYNC
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = fdatasync(fd);
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  HANDLE_ENOSYS(error,err,"fdatasync")
  DeeError_SysThrowf(&DeeError_FSError,error,
                     "Failed to sync %d",fd);
  goto err;
 }
 return_none;
err:
#else
 (void)fd;
 err_unsupported("fdatasync");
#endif
 return NULL;
}




/*[[[deemon import("_dexutils").gw("getpid","->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_getpid_f_impl(void);
PRIVATE DREF DeeObject *DCALL libposix_getpid_f(size_t argc, DeeObject **__restrict argv);
#define LIBPOSIX_GETPID_DEF { "getpid", (DeeObject *)&libposix_getpid, MODSYM_FNORMAL, DOC("()->?Dint") },
#define LIBPOSIX_GETPID_DEF_DOC(doc) { "getpid", (DeeObject *)&libposix_getpid, MODSYM_FNORMAL, DOC("()->?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(libposix_getpid,libposix_getpid_f);
PRIVATE DREF DeeObject *DCALL libposix_getpid_f(size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc,argv,":getpid"))
	    goto err;
	return libposix_getpid_f_impl();
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_getpid_f_impl(void)
//[[[end]]]
{
#ifdef HAVE_GETPID
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = getpid();
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  int error = errno;
  if (error != 0) {
   HANDLE_EINTR(error,again)
   HANDLE_ENOSYS(error,err,"getpid")
   DeeError_SysThrowf(&DeeError_SystemError,error,
                      "Failed to get pid");
   goto err;
  }
 }
 return DeeInt_NewInt(result);
err:
 return NULL;
#else
 (void)command;
 err_unsupported("getpid");
 return NULL;
#endif
}



/*[[[deemon import("_dexutils").gw("umask","mask:d->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_umask_f_impl(int mask);
PRIVATE DREF DeeObject *DCALL libposix_umask_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_UMASK_DEF { "umask", (DeeObject *)&libposix_umask, MODSYM_FNORMAL, DOC("(mask:?Dint)->?Dint") },
#define LIBPOSIX_UMASK_DEF_DOC(doc) { "umask", (DeeObject *)&libposix_umask, MODSYM_FNORMAL, DOC("(mask:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_umask,libposix_umask_f);
#ifndef LIBPOSIX_KWDS_MASK_DEFINED
#define LIBPOSIX_KWDS_MASK_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_mask,{ K(mask), KEND });
#endif /* !LIBPOSIX_KWDS_MASK_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_umask_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int mask;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_mask,"d:umask",&mask))
	    goto err;
	return libposix_umask_f_impl(mask);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_umask_f_impl(int mask)
//[[[end]]]
{
#ifdef HAVE_UMASK
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = umask(mask);
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_ENOSYS(error,err,"umask")
  DeeError_SysThrowf(&DeeError_SystemError,error,
                     "Failed set umask");
  goto err;
 }
 return DeeInt_NewInt(result);
err:
#else
 (void)mask;
 err_unsupported("umask");
#endif
 return NULL;
}




/*[[[deemon import("_dexutils").gw("dup","fd:d->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_dup_f_impl(int fd);
PRIVATE DREF DeeObject *DCALL libposix_dup_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_DUP_DEF { "dup", (DeeObject *)&libposix_dup, MODSYM_FNORMAL, DOC("(fd:?Dint)->?Dint") },
#define LIBPOSIX_DUP_DEF_DOC(doc) { "dup", (DeeObject *)&libposix_dup, MODSYM_FNORMAL, DOC("(fd:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_dup,libposix_dup_f);
#ifndef LIBPOSIX_KWDS_FD_DEFINED
#define LIBPOSIX_KWDS_FD_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd,{ K(fd), KEND });
#endif /* !LIBPOSIX_KWDS_FD_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_dup_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd,"d:dup",&fd))
	    goto err;
	return libposix_dup_f_impl(fd);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_dup_f_impl(int fd)
//[[[end]]]
{
#ifdef HAVE_DUP
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = dup(fd);
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_ENOSYS(error,err,"dup")
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  DeeError_SysThrowf(&DeeError_SystemError,error,
                     "Failed to dup %d",fd);
  goto err;
 }
 return DeeInt_NewInt(result);
err:
#else
 (void)fd;
 err_unsupported("dup");
#endif
 return NULL;
}




/*[[[deemon import("_dexutils").gw("dup2","oldfd:d,newfd:d->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_dup2_f_impl(int oldfd, int newfd);
PRIVATE DREF DeeObject *DCALL libposix_dup2_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_DUP2_DEF { "dup2", (DeeObject *)&libposix_dup2, MODSYM_FNORMAL, DOC("(oldfd:?Dint,newfd:?Dint)->?Dint") },
#define LIBPOSIX_DUP2_DEF_DOC(doc) { "dup2", (DeeObject *)&libposix_dup2, MODSYM_FNORMAL, DOC("(oldfd:?Dint,newfd:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_dup2,libposix_dup2_f);
#ifndef LIBPOSIX_KWDS_OLDFD_NEWFD_DEFINED
#define LIBPOSIX_KWDS_OLDFD_NEWFD_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_oldfd_newfd,{ K(oldfd), K(newfd), KEND });
#endif /* !LIBPOSIX_KWDS_OLDFD_NEWFD_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_dup2_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int oldfd;
	int newfd;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_oldfd_newfd,"dd:dup2",&oldfd,&newfd))
	    goto err;
	return libposix_dup2_f_impl(oldfd,newfd);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_dup2_f_impl(int oldfd, int newfd)
//[[[end]]]
{
#ifdef HAVE_DUP2
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = dup2(oldfd,newfd);
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_ENOSYS(error,err,"dup2")
  HANDLE_EBADF(error,err,"Invalid handle %d",oldfd)
  DeeError_SysThrowf(&DeeError_SystemError,error,
                     "Failed to dup %d",oldfd);
  goto err;
 }
 return DeeInt_NewInt(result);
err:
#else
 (void)oldfd;
 (void)newfd;
 err_unsupported("dup2");
#endif
 return NULL;
}




/*[[[deemon import("_dexutils").gw("dup3","oldfd:d,newfd:d,flags:d->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_dup3_f_impl(int oldfd, int newfd, int flags);
PRIVATE DREF DeeObject *DCALL libposix_dup3_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_DUP3_DEF { "dup3", (DeeObject *)&libposix_dup3, MODSYM_FNORMAL, DOC("(oldfd:?Dint,newfd:?Dint,flags:?Dint)->?Dint") },
#define LIBPOSIX_DUP3_DEF_DOC(doc) { "dup3", (DeeObject *)&libposix_dup3, MODSYM_FNORMAL, DOC("(oldfd:?Dint,newfd:?Dint,flags:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_dup3,libposix_dup3_f);
#ifndef LIBPOSIX_KWDS_OLDFD_NEWFD_FLAGS_DEFINED
#define LIBPOSIX_KWDS_OLDFD_NEWFD_FLAGS_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_oldfd_newfd_flags,{ K(oldfd), K(newfd), K(flags), KEND });
#endif /* !LIBPOSIX_KWDS_OLDFD_NEWFD_FLAGS_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_dup3_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int oldfd;
	int newfd;
	int flags;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_oldfd_newfd_flags,"ddd:dup3",&oldfd,&newfd,&flags))
	    goto err;
	return libposix_dup3_f_impl(oldfd,newfd,flags);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_dup3_f_impl(int oldfd, int newfd, int flags)
//[[[end]]]
{
#if defined(HAVE_DUP3) || defined(_MSC_VER)
 int result;
#ifndef HAVE_DUP3
 if (flags & ~O_CLOEXEC) {
  errno = EINVAL;
  DeeError_Throwf(&DeeError_ValueError,
                  "Invalid flags for dup3 %#x",
                   flags);
  goto err;
 }
#endif
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
#ifdef HAVE_DUP3
 result = dup3(oldfd,newfd);
#else
 result = dup2(oldfd,newfd);
 if (result >= 0) {
  SetHandleInformation((HANDLE)(uintptr_t)_get_osfhandle(result),
                        HANDLE_FLAG_INHERIT,
                       (flags & O_CLOEXEC) ? 0 : HANDLE_FLAG_INHERIT);
 }
#endif
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_ENOSYS(error,err,"dup3")
  HANDLE_EBADF(error,err,"Invalid handle %d",oldfd)
  HANDLE_EINVAL(error,err,"Invalid flags for dup3 %#x",flags)
  DeeError_SysThrowf(&DeeError_SystemError,error,
                     "Failed to dup %d",oldfd);
  goto err;
 }
 return DeeInt_NewInt(result);
err:
#else
 (void)oldfd;
 (void)newfd;
 (void)flags;
 err_unsupported("dup3");
#endif
 return NULL;
}




/*[[[deemon import("_dexutils").gw("isatty","fd:d->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_isatty_f_impl(int fd);
PRIVATE DREF DeeObject *DCALL libposix_isatty_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_ISATTY_DEF { "isatty", (DeeObject *)&libposix_isatty, MODSYM_FNORMAL, DOC("(fd:?Dint)->?Dbool") },
#define LIBPOSIX_ISATTY_DEF_DOC(doc) { "isatty", (DeeObject *)&libposix_isatty, MODSYM_FNORMAL, DOC("(fd:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_isatty,libposix_isatty_f);
#ifndef LIBPOSIX_KWDS_FD_DEFINED
#define LIBPOSIX_KWDS_FD_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd,{ K(fd), KEND });
#endif /* !LIBPOSIX_KWDS_FD_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_isatty_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd,"d:isatty",&fd))
	    goto err;
	return libposix_isatty_f_impl(fd);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_isatty_f_impl(int fd)
//[[[end]]]
{
#ifdef HAVE_ISATTY
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = isatty(fd);
 DBG_ALIGNMENT_ENABLE();
 if (!result) {
  int error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_ENOSYS(error,err,"isatty")
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  if (error != EINVAL
#ifdef ENOTTY
      && error != ENOTTY
#endif
      ) {
   DeeError_SysThrowf(&DeeError_SystemError,error,
                      "Failed to check isatty for %d",fd);
  }
  goto err;
 }
 return_bool_(result != 0);
err:
#else
 (void)fd;
 err_unsupported("isatty");
#endif
 return NULL;
}




/*[[[deemon import("_dexutils").gw("system","command:c:char[]->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_system_f_impl(/*utf-8*/char const *__restrict command);
PRIVATE DREF DeeObject *DCALL libposix_system_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_SYSTEM_DEF { "system", (DeeObject *)&libposix_system, MODSYM_FNORMAL, DOC("(command:?Dstring)->?Dint") },
#define LIBPOSIX_SYSTEM_DEF_DOC(doc) { "system", (DeeObject *)&libposix_system, MODSYM_FNORMAL, DOC("(command:?Dstring)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_system,libposix_system_f);
#ifndef LIBPOSIX_KWDS_COMMAND_DEFINED
#define LIBPOSIX_KWDS_COMMAND_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_command,{ K(command), KEND });
#endif /* !LIBPOSIX_KWDS_COMMAND_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_system_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	/*utf-8*/char const *command_str;
	DeeStringObject *command;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_command,"o:system",&command))
	    goto err;
	if (DeeObject_AssertTypeExact(command,&DeeString_Type))
	    goto err;
	command_str = DeeString_AsUtf8((DeeObject *)command);
	if unlikely(!command_str)
	    goto err;
	return libposix_system_f_impl(command_str);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_system_f_impl(/*utf-8*/char const *__restrict command)
//[[[end]]]
{
#ifdef HAVE_SYSTEM
 int result;
#ifdef EINTR
again:
#endif
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = system(command);
 DBG_ALIGNMENT_ENABLE();
#ifdef EINTR
 if (result < 0 && errno == EINTR)
     goto again;
#endif /* EINTR */
 return DeeInt_NewInt(result);
err:
 return NULL;
#else
 (void)command;
 err_unsupported("system");
 return NULL;
#endif
}



/*[[[deemon import("_dexutils").gw("sched_yield","->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_sched_yield_f_impl(void);
PRIVATE DREF DeeObject *DCALL libposix_sched_yield_f(size_t argc, DeeObject **__restrict argv);
#define LIBPOSIX_SCHED_YIELD_DEF { "sched_yield", (DeeObject *)&libposix_sched_yield, MODSYM_FNORMAL, DOC("()->?Dint") },
#define LIBPOSIX_SCHED_YIELD_DEF_DOC(doc) { "sched_yield", (DeeObject *)&libposix_sched_yield, MODSYM_FNORMAL, DOC("()->?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(libposix_sched_yield,libposix_sched_yield_f);
PRIVATE DREF DeeObject *DCALL libposix_sched_yield_f(size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc,argv,":sched_yield"))
	    goto err;
	return libposix_sched_yield_f_impl();
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_sched_yield_f_impl(void)
//[[[end]]]
{
 SCHED_YIELD();
 return_reference_(&DeeInt_Zero);
}



PRIVATE DREF DeeObject *DCALL
libposix_errno_get_f(size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":errno.getter"))
     goto err;
#ifdef CONFIG_HAVE_ERRNO_H
 return DeeInt_NewInt(errno);
#else
 err_unsupported("errno");
#endif
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
libposix_errno_set_f(size_t argc, DeeObject **__restrict argv) {
 int value;
 if (DeeArg_Unpack(argc,argv,"d:errno.setter",&value))
     goto err;
#ifdef CONFIG_HAVE_ERRNO_H
 errno = value;
 return_none;
#else
 err_unsupported("errno");
#endif
err:
 return NULL;
}

PRIVATE DEFINE_CMETHOD(libposix_errno_get,&libposix_errno_get_f);
PRIVATE DEFINE_CMETHOD(libposix_errno_set,&libposix_errno_set_f);


/*[[[deemon import("_dexutils").gw("atexit","callback:?DCallable,args:?DTuple=Dee_EmptyTuple"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_atexit_f_impl(DeeObject *__restrict callback, DeeObject *__restrict args);
PRIVATE DREF DeeObject *DCALL libposix_atexit_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_ATEXIT_DEF { "atexit", (DeeObject *)&libposix_atexit, MODSYM_FNORMAL, DOC("(callback:?DCallable,args:?DTuple=!T0)") },
#define LIBPOSIX_ATEXIT_DEF_DOC(doc) { "atexit", (DeeObject *)&libposix_atexit, MODSYM_FNORMAL, DOC("(callback:?DCallable,args:?DTuple=!T0)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_atexit,libposix_atexit_f);
#ifndef LIBPOSIX_KWDS_CALLBACK_ARGS_DEFINED
#define LIBPOSIX_KWDS_CALLBACK_ARGS_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_callback_args,{ K(callback), K(args), KEND });
#endif /* !LIBPOSIX_KWDS_CALLBACK_ARGS_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_atexit_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *callback;
	DeeObject *args = Dee_EmptyTuple;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_callback_args,"o|o:atexit",&callback,&args))
	    goto err;
	return libposix_atexit_f_impl(callback,args);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_atexit_f_impl(DeeObject *__restrict callback, DeeObject *__restrict args)
//[[[end]]]
{
 if (DeeObject_AssertTypeExact(args,&DeeTuple_Type))
     goto err;
 if (Dee_AtExit(callback,args))
     goto err;
 return_none;
err:
 return NULL;
}


/*[[[deemon import("_dexutils").gw("exit","exitcode:d"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_exit_f_impl(int exitcode);
PRIVATE DREF DeeObject *DCALL libposix_exit_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_EXIT_DEF { "exit", (DeeObject *)&libposix_exit, MODSYM_FNORMAL, DOC("(exitcode:?Dint)") },
#define LIBPOSIX_EXIT_DEF_DOC(doc) { "exit", (DeeObject *)&libposix_exit, MODSYM_FNORMAL, DOC("(exitcode:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_exit,libposix_exit_f);
#ifndef LIBPOSIX_KWDS_EXITCODE_DEFINED
#define LIBPOSIX_KWDS_EXITCODE_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_exitcode,{ K(exitcode), KEND });
#endif /* !LIBPOSIX_KWDS_EXITCODE_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_exit_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int exitcode;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_exitcode,"d:exit",&exitcode))
	    goto err;
	return libposix_exit_f_impl(exitcode);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_exit_f_impl(int exitcode)
//[[[end]]]
{
 Dee_Exit(exitcode,true);
 return NULL;
}


/*[[[deemon import("_dexutils").gw("_exit","exitcode:d"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix__exit_f_impl(int exitcode);
PRIVATE DREF DeeObject *DCALL libposix__exit_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX__EXIT_DEF { "_exit", (DeeObject *)&libposix__exit, MODSYM_FNORMAL, DOC("(exitcode:?Dint)") },
#define LIBPOSIX__EXIT_DEF_DOC(doc) { "_exit", (DeeObject *)&libposix__exit, MODSYM_FNORMAL, DOC("(exitcode:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix__exit,libposix__exit_f);
#ifndef LIBPOSIX_KWDS_EXITCODE_DEFINED
#define LIBPOSIX_KWDS_EXITCODE_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_exitcode,{ K(exitcode), KEND });
#endif /* !LIBPOSIX_KWDS_EXITCODE_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix__exit_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int exitcode;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_exitcode,"d:_exit",&exitcode))
	    goto err;
	return libposix__exit_f_impl(exitcode);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix__exit_f_impl(int exitcode)
//[[[end]]]
{
 Dee_Exit(exitcode,false);
 return NULL;
}


/*[[[deemon import("_dexutils").gw("abort",""); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_abort_f_impl(void);
PRIVATE DREF DeeObject *DCALL libposix_abort_f(size_t argc, DeeObject **__restrict argv);
#define LIBPOSIX_ABORT_DEF { "abort", (DeeObject *)&libposix_abort, MODSYM_FNORMAL, DOC("()") },
#define LIBPOSIX_ABORT_DEF_DOC(doc) { "abort", (DeeObject *)&libposix_abort, MODSYM_FNORMAL, DOC("()\n" doc) },
PRIVATE DEFINE_CMETHOD(libposix_abort,libposix_abort_f);
PRIVATE DREF DeeObject *DCALL libposix_abort_f(size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc,argv,":abort"))
	    goto err;
	return libposix_abort_f_impl();
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_abort_f_impl(void)
//[[[end]]]
{
 Dee_Exit(EXIT_FAILURE,false);
 return NULL;
}



#if (defined(HAVE_FILEIO) && defined(HAVE_FTRUNCATE64)) || \
     defined(HAVE_WTRUNCATE64) || defined(HAVE_TRUNCATE64) || defined(__DEEMON__)
#if defined(HAVE_WTRUNCATE64) || defined(HAVE_WOPEN) || defined(HAVE_WOPEN64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("truncate","filename:c:wchar_t[],len:I64d"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_truncate_f_impl(dwchar_t const *__restrict filename, int64_t len);
PRIVATE DREF DeeObject *DCALL libposix_truncate_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_TRUNCATE_DEF { "truncate", (DeeObject *)&libposix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)") },
#define LIBPOSIX_TRUNCATE_DEF_DOC(doc) { "truncate", (DeeObject *)&libposix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_truncate,libposix_truncate_f);
#ifndef LIBPOSIX_KWDS_FILENAME_LEN_DEFINED
#define LIBPOSIX_KWDS_FILENAME_LEN_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_filename_len,{ K(filename), K(len), KEND });
#endif /* !LIBPOSIX_KWDS_FILENAME_LEN_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_truncate_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	dwchar_t const *filename_str;
	DeeStringObject *filename;
	int64_t len;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_filename_len,"oI64d:truncate",&filename,&len))
	    goto err;
	if (DeeObject_AssertTypeExact(filename,&DeeString_Type))
	    goto err;
	filename_str = (dwchar_t const *)DeeString_AsWide((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return libposix_truncate_f_impl(filename_str,len);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_truncate_f_impl(dwchar_t const *__restrict filename, int64_t len)
//[[[end]]]
{
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
#ifdef HAVE_WTRUNCATE64
 result = wtruncate64(filename,len);
#else
 {
#ifdef HAVE_WOPEN64
  int fd = result = wopen64(filename,O_RDWR);
#else
  int fd = result = wopen(filename,O_RDWR);
#endif
  if (fd >= 0) {
   result = ftruncate64(fd,len);
   close(fd);
  }
 }
#endif
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  result = errno;
  HANDLE_EINTR(result,again)
  HANDLE_ENOENT_ENOTDIR(result,err,"File or directory %ls could not be found",filename)
  HANDLE_ENOSYS(result,err,"truncate")
  HANDLE_EACCES(result,err,"Failed to access %ls",filename)
  HANDLE_EFBIG_EINVAL(result,err,"Cannot truncate %ls: Invalid size %I64d",filename,len)
  HANDLE_ENXIO_EISDIR(result,err,"Cannot truncate directory %ls",filename)
  HANDLE_EROFS_ETXTBSY(result,err,"Read-only file %ls",filename)
  DeeError_SysThrowf(&DeeError_SystemError,result,
                     "Failed to truncate %ls",filename);
  goto err;
 }
 return_none;
err:
 return NULL;
}
#endif
#if !(defined(HAVE_WTRUNCATE64) || defined(HAVE_WOPEN) || defined(HAVE_WOPEN64)) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("truncate","filename:c:char[],len:I64d"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_truncate_f_impl(/*utf-8*/char const *__restrict filename, int64_t len);
PRIVATE DREF DeeObject *DCALL libposix_truncate_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_TRUNCATE_DEF { "truncate", (DeeObject *)&libposix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)") },
#define LIBPOSIX_TRUNCATE_DEF_DOC(doc) { "truncate", (DeeObject *)&libposix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_truncate,libposix_truncate_f);
#ifndef LIBPOSIX_KWDS_FILENAME_LEN_DEFINED
#define LIBPOSIX_KWDS_FILENAME_LEN_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_filename_len,{ K(filename), K(len), KEND });
#endif /* !LIBPOSIX_KWDS_FILENAME_LEN_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_truncate_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	/*utf-8*/char const *filename_str;
	DeeStringObject *filename;
	int64_t len;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_filename_len,"oI64d:truncate",&filename,&len))
	    goto err;
	if (DeeObject_AssertTypeExact(filename,&DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return libposix_truncate_f_impl(filename_str,len);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_truncate_f_impl(/*utf-8*/char const *__restrict filename, int64_t len)
//[[[end]]]
{
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
#ifdef HAVE_TRUNCATE64
 result = truncate64(filename,len);
#else
 {
#ifdef HAVE_OPEN64
  int fd = result = open64(filename,O_RDWR);
#else
  int fd = result = open(filename,O_RDWR);
#endif
  if (fd >= 0) {
   result = ftruncate64(fd,len);
   close(fd);
  }
 }
#endif
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  result = errno;
  HANDLE_EINTR(result,again)
  HANDLE_ENOENT_ENOTDIR(result,err,"File or directory %s could not be found",filename)
  HANDLE_ENOSYS(result,err,"truncate")
  HANDLE_EACCES(result,err,"Failed to access %s",filename)
  HANDLE_EFBIG_EINVAL(result,err,"Cannot truncate %s: Invalid size %I64d",filename,len)
  HANDLE_ENXIO_EISDIR(result,err,"Cannot truncate directory %s",filename)
  HANDLE_EROFS_ETXTBSY(result,err,"Read-only file %s",filename)
  DeeError_SysThrowf(&DeeError_SystemError,result,"Failed to truncate %s",filename);
  goto err;
 }
 return_none;
err:
 return NULL;
}
#endif
#endif

#if !((defined(HAVE_FILEIO) && defined(HAVE_FTRUNCATE64)) || \
       defined(HAVE_WTRUNCATE64) || defined(HAVE_TRUNCATE64)) || defined(__DEEMON__)
#if defined(HAVE_WTRUNCATE) || defined(HAVE_WOPEN) || defined(HAVE_WOPEN64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("truncate","filename:c:wchar_t[],len:I32d"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_truncate_f_impl(dwchar_t const *__restrict filename, int32_t len);
PRIVATE DREF DeeObject *DCALL libposix_truncate_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_TRUNCATE_DEF { "truncate", (DeeObject *)&libposix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)") },
#define LIBPOSIX_TRUNCATE_DEF_DOC(doc) { "truncate", (DeeObject *)&libposix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_truncate,libposix_truncate_f);
#ifndef LIBPOSIX_KWDS_FILENAME_LEN_DEFINED
#define LIBPOSIX_KWDS_FILENAME_LEN_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_filename_len,{ K(filename), K(len), KEND });
#endif /* !LIBPOSIX_KWDS_FILENAME_LEN_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_truncate_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	dwchar_t const *filename_str;
	DeeStringObject *filename;
	int32_t len;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_filename_len,"oI32d:truncate",&filename,&len))
	    goto err;
	if (DeeObject_AssertTypeExact(filename,&DeeString_Type))
	    goto err;
	filename_str = (dwchar_t const *)DeeString_AsWide((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return libposix_truncate_f_impl(filename_str,len);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_truncate_f_impl(dwchar_t const *__restrict filename, int32_t len)
//[[[end]]]
{
#ifdef HAVE_FILEIO
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
#ifdef HAVE_WTRUNCATE
 result = wtruncate(filename,len);
#else
 {
  int fd = result = wopen(filename,O_RDWR);
  if (fd >= 0) {
   result = ftruncate(fd,len);
   close(fd);
  }
 }
#endif
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_ENOENT_ENOTDIR(result,err,"File or directory %ls could not be found",filename)
  HANDLE_ENOSYS(result,err,"truncate")
  HANDLE_EACCES(result,err,"Failed to access %ls",filename)
  HANDLE_EFBIG_EINVAL(result,err,"Cannot truncate %ls: Invalid size %I32d",filename,len)
  HANDLE_ENXIO_EISDIR(result,err,"Cannot truncate directory %ls",filename)
  HANDLE_EROFS_ETXTBSY(result,err,"Read-only file %ls",filename)
  DeeError_SysThrowf(&DeeError_SystemError,error,"Failed to truncate %ls",filename);
  goto err;
 }
 return_none;
err:
#else
 (void)fd;
 (void)len;
 err_unsupported("truncate");
#endif
 return NULL;
}
#endif
#if !(defined(HAVE_WTRUNCATE) || defined(HAVE_WOPEN) || defined(HAVE_WOPEN64)) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("truncate","filename:c:char[],len:I32d"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_truncate_f_impl(/*utf-8*/char const *__restrict filename, int32_t len);
PRIVATE DREF DeeObject *DCALL libposix_truncate_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_TRUNCATE_DEF { "truncate", (DeeObject *)&libposix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)") },
#define LIBPOSIX_TRUNCATE_DEF_DOC(doc) { "truncate", (DeeObject *)&libposix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_truncate,libposix_truncate_f);
#ifndef LIBPOSIX_KWDS_FILENAME_LEN_DEFINED
#define LIBPOSIX_KWDS_FILENAME_LEN_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_filename_len,{ K(filename), K(len), KEND });
#endif /* !LIBPOSIX_KWDS_FILENAME_LEN_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_truncate_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	/*utf-8*/char const *filename_str;
	DeeStringObject *filename;
	int32_t len;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_filename_len,"oI32d:truncate",&filename,&len))
	    goto err;
	if (DeeObject_AssertTypeExact(filename,&DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return libposix_truncate_f_impl(filename_str,len);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_truncate_f_impl(/*utf-8*/char const *__restrict filename, int32_t len)
//[[[end]]]
{
#ifdef HAVE_FILEIO
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
#ifdef HAVE_TRUNCATE
 result = truncate(filename,len);
#else
 {
  int fd = result = open(filename,O_RDWR);
  if (fd >= 0) {
   result = ftruncate(fd,len);
   close(fd);
  }
 }
#endif
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_ENOENT_ENOTDIR(result,err,"File or directory %s could not be found",filename)
  HANDLE_ENOSYS(result,err,"truncate")
  HANDLE_EACCES(result,err,"Failed to access %s",filename)
  HANDLE_EFBIG_EINVAL(result,err,"Cannot truncate %s: Invalid size %I32d",filename,len)
  HANDLE_ENXIO_EISDIR(result,err,"Cannot truncate directory %s",filename)
  HANDLE_EROFS_ETXTBSY(result,err,"Read-only file %s",filename)
  DeeError_SysThrowf(&DeeError_SystemError,error,"Failed to truncate %s",filename);
  goto err;
 }
 return_none;
err:
#else
 (void)fd;
 (void)len;
 err_unsupported("truncate");
#endif
 return NULL;
}
#endif
#endif

#if defined(HAVE_FTRUNCATE64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("ftruncate","fd:d,len:I64d"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_ftruncate_f_impl(int fd, int64_t len);
PRIVATE DREF DeeObject *DCALL libposix_ftruncate_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_FTRUNCATE_DEF { "ftruncate", (DeeObject *)&libposix_ftruncate, MODSYM_FNORMAL, DOC("(fd:?Dint,len:?Dint)") },
#define LIBPOSIX_FTRUNCATE_DEF_DOC(doc) { "ftruncate", (DeeObject *)&libposix_ftruncate, MODSYM_FNORMAL, DOC("(fd:?Dint,len:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_ftruncate,libposix_ftruncate_f);
#ifndef LIBPOSIX_KWDS_FD_LEN_DEFINED
#define LIBPOSIX_KWDS_FD_LEN_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd_len,{ K(fd), K(len), KEND });
#endif /* !LIBPOSIX_KWDS_FD_LEN_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_ftruncate_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	int64_t len;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd_len,"dI64d:ftruncate",&fd,&len))
	    goto err;
	return libposix_ftruncate_f_impl(fd,len);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_ftruncate_f_impl(int fd, int64_t len)
//[[[end]]]
{
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = ftruncate64(fd,len);
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_ENOSYS(result,err,"ftruncate")
  HANDLE_EFBIG_EINVAL(result,err,"Cannot truncate %d: Invalid size %I64d",fd,len)
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  DeeError_SysThrowf(&DeeError_SystemError,error,"Failed to truncate %d",fd);
  goto err;
 }
 return_none;
err:
 return NULL;
}
#endif

#if !defined(HAVE_FTRUNCATE64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("ftruncate","fd:d,len:I32d"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_ftruncate_f_impl(int fd, int32_t len);
PRIVATE DREF DeeObject *DCALL libposix_ftruncate_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_FTRUNCATE_DEF { "ftruncate", (DeeObject *)&libposix_ftruncate, MODSYM_FNORMAL, DOC("(fd:?Dint,len:?Dint)") },
#define LIBPOSIX_FTRUNCATE_DEF_DOC(doc) { "ftruncate", (DeeObject *)&libposix_ftruncate, MODSYM_FNORMAL, DOC("(fd:?Dint,len:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_ftruncate,libposix_ftruncate_f);
#ifndef LIBPOSIX_KWDS_FD_LEN_DEFINED
#define LIBPOSIX_KWDS_FD_LEN_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_fd_len,{ K(fd), K(len), KEND });
#endif /* !LIBPOSIX_KWDS_FD_LEN_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_ftruncate_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int fd;
	int32_t len;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_fd_len,"dI32d:ftruncate",&fd,&len))
	    goto err;
	return libposix_ftruncate_f_impl(fd,len);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_ftruncate_f_impl(int fd, int32_t len)
//[[[end]]]
{
#ifdef HAVE_FILEIO
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = ftruncate(fd,len);
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  int error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_ENOSYS(result,err,"ftruncate")
  HANDLE_EFBIG_EINVAL(result,err,"Cannot truncate %d: Invalid size %I32d",fd,len)
  HANDLE_EBADF(error,err,"Invalid handle %d",fd)
  DeeError_SysThrowf(&DeeError_SystemError,error,"Failed to truncate %d",fd);
  goto err;
 }
 return_none;
err:
#else
 (void)fd;
 (void)len;
 err_unsupported("ftruncate");
#endif
 return NULL;
}
#endif

/*[[[deemon import("_dexutils").gw("access","filename:c:char[],how:d->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_access_f_impl(/*utf-8*/char const *__restrict filename, int how);
PRIVATE DREF DeeObject *DCALL libposix_access_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_ACCESS_DEF { "access", (DeeObject *)&libposix_access, MODSYM_FNORMAL, DOC("(filename:?Dstring,how:?Dint)->?Dbool") },
#define LIBPOSIX_ACCESS_DEF_DOC(doc) { "access", (DeeObject *)&libposix_access, MODSYM_FNORMAL, DOC("(filename:?Dstring,how:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_access,libposix_access_f);
#ifndef LIBPOSIX_KWDS_FILENAME_HOW_DEFINED
#define LIBPOSIX_KWDS_FILENAME_HOW_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_filename_how,{ K(filename), K(how), KEND });
#endif /* !LIBPOSIX_KWDS_FILENAME_HOW_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_access_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	/*utf-8*/char const *filename_str;
	DeeStringObject *filename;
	int how;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_filename_how,"od:access",&filename,&how))
	    goto err;
	if (DeeObject_AssertTypeExact(filename,&DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return libposix_access_f_impl(filename_str,how);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_access_f_impl(/*utf-8*/char const *__restrict filename, int how)
//[[[end]]]
{
#ifdef HAVE_ACCESS
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = access(filename,how);
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  result = errno;
  HANDLE_EINTR(result,again)
#ifdef EACCES
  if (result == EACCES)
      return_false;
  if (result == EINVAL)
      return_false;
  HANDLE_ENOSYS(result,err,"access")
  HANDLE_EINVAL(result,err,"Invalid access mode %d",how)
  HANDLE_ENOMEM(result,err,"Insufficient kernel memory to check access to %s",filename)
  HANDLE_ENOENT_ENOTDIR(result,err,"File or directory %s could not be found",filename)
  HANDLE_EROFS_ETXTBSY(result,err,"Read-only file %s",filename)
  DeeError_SysThrowf(&DeeError_SystemError,result,
                     "Failed to check access to %s",
                      filename);
  goto err;
#else
  return_false;
#endif
 }
 return_true;
err:
#else
 (void)filename;
 (void)how;
 err_unsupported("access");
#endif
 return NULL;
}



/*[[[deemon import("_dexutils").gw("euidaccess","filename:c:char[],how:d->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_euidaccess_f_impl(/*utf-8*/char const *__restrict filename, int how);
PRIVATE DREF DeeObject *DCALL libposix_euidaccess_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_EUIDACCESS_DEF { "euidaccess", (DeeObject *)&libposix_euidaccess, MODSYM_FNORMAL, DOC("(filename:?Dstring,how:?Dint)->?Dbool") },
#define LIBPOSIX_EUIDACCESS_DEF_DOC(doc) { "euidaccess", (DeeObject *)&libposix_euidaccess, MODSYM_FNORMAL, DOC("(filename:?Dstring,how:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_euidaccess,libposix_euidaccess_f);
#ifndef LIBPOSIX_KWDS_FILENAME_HOW_DEFINED
#define LIBPOSIX_KWDS_FILENAME_HOW_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_filename_how,{ K(filename), K(how), KEND });
#endif /* !LIBPOSIX_KWDS_FILENAME_HOW_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_euidaccess_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	/*utf-8*/char const *filename_str;
	DeeStringObject *filename;
	int how;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_filename_how,"od:euidaccess",&filename,&how))
	    goto err;
	if (DeeObject_AssertTypeExact(filename,&DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return libposix_euidaccess_f_impl(filename_str,how);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_euidaccess_f_impl(/*utf-8*/char const *__restrict filename, int how)
//[[[end]]]
{
#if defined(HAVE_EUIDACCESS) || defined(HAVE_EACCESS)
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
#ifdef HAVE_EUIDACCESS
 result = euidaccess(filename,how);
#else
 result = eaccess(filename,how);
#endif
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  result = errno;
  HANDLE_EINTR(result,again)
#ifdef EACCES
  if (result == EACCES)
      return_false;
  if (result == EINVAL)
      return_false;
  HANDLE_ENOSYS(result,err,"euidaccess")
  HANDLE_EINVAL(result,err,"Invalid access mode %d",how)
  HANDLE_ENOMEM(result,err,"Insufficient kernel memory to check access to %s",filename)
  HANDLE_ENOENT_ENOTDIR(result,err,"File or directory %s could not be found",filename)
  HANDLE_EROFS_ETXTBSY(result,err,"Read-only file %s",filename)
  DeeError_SysThrowf(&DeeError_SystemError,result,"Failed to check access to %s",filename);
  goto err;
#else
  return_false;
#endif
 }
 return_true;
err:
#else
 (void)filename;
 (void)how;
 err_unsupported("euidaccess");
#endif
 return NULL;
}


/*[[[deemon import("_dexutils").gw("faccessat","dfd:d,filename:c:char[],how:d,atflags:d->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_faccessat_f_impl(int dfd, /*utf-8*/char const *__restrict filename, int how, int atflags);
PRIVATE DREF DeeObject *DCALL libposix_faccessat_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_FACCESSAT_DEF { "faccessat", (DeeObject *)&libposix_faccessat, MODSYM_FNORMAL, DOC("(dfd:?Dint,filename:?Dstring,how:?Dint,atflags:?Dint)->?Dbool") },
#define LIBPOSIX_FACCESSAT_DEF_DOC(doc) { "faccessat", (DeeObject *)&libposix_faccessat, MODSYM_FNORMAL, DOC("(dfd:?Dint,filename:?Dstring,how:?Dint,atflags:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_faccessat,libposix_faccessat_f);
#ifndef LIBPOSIX_KWDS_DFD_FILENAME_HOW_ATFLAGS_DEFINED
#define LIBPOSIX_KWDS_DFD_FILENAME_HOW_ATFLAGS_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_dfd_filename_how_atflags,{ K(dfd), K(filename), K(how), K(atflags), KEND });
#endif /* !LIBPOSIX_KWDS_DFD_FILENAME_HOW_ATFLAGS_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_faccessat_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int dfd;
	/*utf-8*/char const *filename_str;
	DeeStringObject *filename;
	int how;
	int atflags;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_dfd_filename_how_atflags,"dodd:faccessat",&dfd,&filename,&how,&atflags))
	    goto err;
	if (DeeObject_AssertTypeExact(filename,&DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return libposix_faccessat_f_impl(dfd,filename_str,how,atflags);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_faccessat_f_impl(int dfd, /*utf-8*/char const *__restrict filename, int how, int atflags)
//[[[end]]]
{
#ifdef HAVE_FACCESSAT
 int result;
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = faccessat(dfd,filename,how,atflags);
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  result = errno;
  HANDLE_EINTR(result,again)
#ifdef EACCES
  if (result == EACCES)
      return_false;
  if (result == EINVAL)
      return_false;
  HANDLE_ENOSYS(result,err,"faccessat")
  HANDLE_EINVAL(result,err,"Invalid access-mode (%#x) or at-flags (%#x)",how,atflags)
  HANDLE_ENOMEM(result,err,"Insufficient kernel memory to check access to %d:%s",dfd,filename)
  HANDLE_ENOENT_ENOTDIR(result,err,"File or directory %d:%s could not be found",dfd,filename)
  HANDLE_EROFS_ETXTBSY(result,err,"Read-only file %d:%s",dfd,filename)
  HANDLE_EBADF(error,err,"Invalid handle %d",dfd)
  DeeError_SysThrowf(&DeeError_SystemError,result,"Failed to check access to %d:%s",dfd,filename);
  goto err;
#else
  return_false;
#endif
 }
 return_true;
err:
#else
 (void)dfd;
 (void)filename;
 (void)how;
 (void)atflags;
 err_unsupported("faccessat");
#endif
 return NULL;
}



/*[[[deemon import("_dexutils").gw("pipe","->?T2?Dint?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_pipe_f_impl(void);
PRIVATE DREF DeeObject *DCALL libposix_pipe_f(size_t argc, DeeObject **__restrict argv);
#define LIBPOSIX_PIPE_DEF { "pipe", (DeeObject *)&libposix_pipe, MODSYM_FNORMAL, DOC("()->?T2?Dint?Dint") },
#define LIBPOSIX_PIPE_DEF_DOC(doc) { "pipe", (DeeObject *)&libposix_pipe, MODSYM_FNORMAL, DOC("()->?T2?Dint?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(libposix_pipe,libposix_pipe_f);
PRIVATE DREF DeeObject *DCALL libposix_pipe_f(size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc,argv,":pipe"))
	    goto err;
	return libposix_pipe_f_impl();
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_pipe_f_impl(void)
//[[[end]]]
{
#ifdef HAVE_PIPE
 DREF DeeObject *result;
 int error;
 int fds[2];
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 error = pipe(fds);
 DBG_ALIGNMENT_ENABLE();
 if (error < 0) {
  error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_ENOSYS(result,err,"pipe")
  /* TODO: Other errors */
  DeeError_SysThrowf(&DeeError_SystemError,error,
                     "Failed to create pipe");
  goto err;
 }
 result = DeeTuple_Newf("dd",fds[0],fds[1]);
 if unlikely(!result)
    goto err_fds;
 return result;
err_fds:
 DBG_ALIGNMENT_DISABLE();
 close(fds[1]);
 close(fds[0]);
 DBG_ALIGNMENT_ENABLE();
err:
#elif defined(_MSC_VER)
 DREF DeeObject *result;
 HANDLE hRead,hWrite;
 int fds[2];
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 if (!CreatePipe(&hRead,&hWrite,NULL,0)) {
  DWORD dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (dwError == ERROR_OPERATION_ABORTED)
      goto again;
  DeeError_SysThrowf(&DeeError_SystemError,dwError,
                     "Failed to create pipe");
  goto err;
 }
 /* On unix, pipe handles are inheritable by default */
 if (!SetHandleInformation(hRead,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT))
     goto err_hWritehRead_nterror;
 if (!SetHandleInformation(hWrite,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT))
     goto err_hWritehRead_nterror;
 fds[0] = _open_osfhandle((intptr_t)(uintptr_t)hRead,O_RDONLY);
 if unlikely(fds[0] < 0)
    goto err_hWritehRead_errno;
 fds[1] = _open_osfhandle((intptr_t)(uintptr_t)hWrite,O_WRONLY);
 DBG_ALIGNMENT_ENABLE();
 if unlikely(fds[1] < 0)
    goto err_hWritefds0_errno;
 result = DeeTuple_Newf("dd",fds[0],fds[1]);
 if unlikely(!result)
    goto err_fds;
 return result;
err_fds:
 DBG_ALIGNMENT_DISABLE();
 close(fds[1]);
 close(fds[0]);
 goto err;
err_hWritefds0_errno:
 DeeError_SysThrowf(&DeeError_SystemError,errno,
                    "Failed to create pipe");
 close(fds[0]);
 goto err_hWrite;
err_hWritehRead_errno:
 DeeError_SysThrowf(&DeeError_SystemError,errno,
                    "Failed to create pipe");
 goto err_hWritehRead;
err_hWritehRead_nterror:
 DeeError_SysThrowf(&DeeError_SystemError,GetLastError(),
                    "Failed to create pipe");
err_hWritehRead:
 CloseHandle(hRead);
err_hWrite:
 CloseHandle(hWrite);
 DBG_ALIGNMENT_ENABLE();
err:
#else
 err_unsupported("pipe");
#endif
 return NULL;
}


/*[[[deemon import("_dexutils").gw("pipe2","oflags:d->?T2?Dint?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_pipe2_f_impl(int oflags);
PRIVATE DREF DeeObject *DCALL libposix_pipe2_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_PIPE2_DEF { "pipe2", (DeeObject *)&libposix_pipe2, MODSYM_FNORMAL, DOC("(oflags:?Dint)->?T2?Dint?Dint") },
#define LIBPOSIX_PIPE2_DEF_DOC(doc) { "pipe2", (DeeObject *)&libposix_pipe2, MODSYM_FNORMAL, DOC("(oflags:?Dint)->?T2?Dint?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_pipe2,libposix_pipe2_f);
#ifndef LIBPOSIX_KWDS_OFLAGS_DEFINED
#define LIBPOSIX_KWDS_OFLAGS_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_oflags,{ K(oflags), KEND });
#endif /* !LIBPOSIX_KWDS_OFLAGS_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_pipe2_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int oflags;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_oflags,"d:pipe2",&oflags))
	    goto err;
	return libposix_pipe2_f_impl(oflags);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_pipe2_f_impl(int oflags)
//[[[end]]]
{
#if defined(HAVE_PIPE2) || \
   (defined(HAVE_PIPE) && defined(HAVE_FCNTL) && defined(F_SETFD) && \
    defined(FD_CLOEXEC) && defined(O_CLOEXEC) && (!defined(O_NONBLOCK) || defined(F_SETFL)))
 DREF DeeObject *result;
 int error;
 int fds[2];
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
#ifdef HAVE_PIPE2
 error = pipe2(fds,oflags);
#else /* HAVE_PIPE2 */
#ifdef O_NONBLOCK
 if (oflags & ~(O_CLOEXEC|O_NONBLOCK))
#else
 if (oflags & ~(O_CLOEXEC))
#endif
 {
  errno = EINVAL;
  DBG_ALIGNMENT_ENABLE();
  return_none;
 }
 error = pipe(fds);
 if (error >= 0) {
  if (oflags & O_CLOEXEC) {
   error = fcntl(fds[0],F_SETFD,FD_CLOEXEC);
   if unlikely(error < 0)
      goto err_fds_errno;
   error = fcntl(fds[1],F_SETFD,FD_CLOEXEC);
   if unlikely(error < 0)
      goto err_fds_errno;
  }
#ifdef O_NONBLOCK
  if (oflags & O_NONBLOCK) {
   error = fcntl(fds[0],F_SETFL,O_NONBLOCK);
   if unlikely(error < 0)
      goto err_fds_errno;
   error = fcntl(fds[1],F_SETFL,O_NONBLOCK);
   if unlikely(error < 0)
      goto err_fds_errno;
  }
#endif
 }
#endif /* !HAVE_PIPE2 */
 DBG_ALIGNMENT_ENABLE();
 if (error < 0) {
  error = errno;
  HANDLE_EINTR(error,again)
  HANDLE_ENOSYS(result,err,"pipe")
  /* TODO: Other errors */
  DeeError_SysThrowf(&DeeError_SystemError,error,
                     "Failed to create pipe");
  goto err;
 }
 result = DeeTuple_Newf("dd",fds[0],fds[1]);
 if unlikely(!result)
    goto err_fds;
 return result;
#ifndef HAVE_PIPE2
err_fds_errno:
#ifdef EINTR
 if (errno == EINTR) {
  DBG_ALIGNMENT_DISABLE();
  close(fds[1]);
  close(fds[0]);
  DBG_ALIGNMENT_ENABLE();
  goto again;
 }
#endif /* EINTR */
#endif /* !HAVE_PIPE2 */
err_fds:
 DBG_ALIGNMENT_DISABLE();
 close(fds[1]);
 close(fds[0]);
 DBG_ALIGNMENT_ENABLE();
err:
#elif defined(_MSC_VER)
 DREF DeeObject *result;
 HANDLE hRead,hWrite;
 int fds[2];
 if (oflags & ~(O_CLOEXEC)) {
  errno = EINVAL;
  return_none;
 }
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 if (!CreatePipe(&hRead,&hWrite,NULL,0)) {
  DWORD dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (dwError == ERROR_OPERATION_ABORTED)
      goto again;
  DeeError_SysThrowf(&DeeError_SystemError,dwError,
                     "Failed to create pipe");
  goto err;
 }
 if (!(oflags & O_CLOEXEC)) {
  if (!SetHandleInformation(hRead,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT))
      goto err_hWritehRead_nterror;
  if (!SetHandleInformation(hWrite,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT))
      goto err_hWritehRead_nterror;
 }
 fds[0] = _open_osfhandle((intptr_t)(uintptr_t)hRead,O_RDONLY);
 if unlikely(fds[0] < 0)
    goto err_hWritehRead_errno;
 fds[1] = _open_osfhandle((intptr_t)(uintptr_t)hWrite,O_WRONLY);
 DBG_ALIGNMENT_ENABLE();
 if unlikely(fds[1] < 0)
    goto err_hWritefds0_errno;
 result = DeeTuple_Newf("dd",fds[0],fds[1]);
 if unlikely(!result)
    goto err_fds;
 return result;
err_fds:
 DBG_ALIGNMENT_DISABLE();
 close(fds[1]);
 close(fds[0]);
 goto err;
err_hWritefds0_errno:
 DeeError_SysThrowf(&DeeError_SystemError,errno,
                    "Failed to create pipe");
 close(fds[0]);
 goto err_hWrite;
err_hWritehRead_errno:
 DeeError_SysThrowf(&DeeError_SystemError,errno,
                    "Failed to create pipe");
 goto err_hWritehRead;
err_hWritehRead_nterror:
 DeeError_SysThrowf(&DeeError_SystemError,GetLastError(),
                    "Failed to create pipe");
err_hWritehRead:
 CloseHandle(hRead);
err_hWrite:
 CloseHandle(hWrite);
 DBG_ALIGNMENT_ENABLE();
err:
#else
 (void)oflags;
 err_unsupported("pipe2");
#endif
 return NULL;
}



/*[[[deemon import("_dexutils").gw("fchownat","dfd:d,filename:c:char[],owner:?X3?Efs:user?Dstring?Dint,group:?X3?Efs:group?Dstring?Dint,atflags:d->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libposix_fchownat_f_impl(int dfd, /*utf-8*/char const *__restrict filename, DeeObject *__restrict owner, DeeObject *__restrict group, int atflags);
PRIVATE DREF DeeObject *DCALL libposix_fchownat_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBPOSIX_FCHOWNAT_DEF { "fchownat", (DeeObject *)&libposix_fchownat, MODSYM_FNORMAL, DOC("(dfd:?Dint,filename:?Dstring,owner:?X3?Efs:user?Dstring?Dint,group:?X3?Efs:group?Dstring?Dint,atflags:?Dint)->?Dint") },
#define LIBPOSIX_FCHOWNAT_DEF_DOC(doc) { "fchownat", (DeeObject *)&libposix_fchownat, MODSYM_FNORMAL, DOC("(dfd:?Dint,filename:?Dstring,owner:?X3?Efs:user?Dstring?Dint,group:?X3?Efs:group?Dstring?Dint,atflags:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libposix_fchownat,libposix_fchownat_f);
#ifndef LIBPOSIX_KWDS_DFD_FILENAME_OWNER_GROUP_ATFLAGS_DEFINED
#define LIBPOSIX_KWDS_DFD_FILENAME_OWNER_GROUP_ATFLAGS_DEFINED 1
PRIVATE DEFINE_KWLIST(libposix_kwds_dfd_filename_owner_group_atflags,{ K(dfd), K(filename), K(owner), K(group), K(atflags), KEND });
#endif /* !LIBPOSIX_KWDS_DFD_FILENAME_OWNER_GROUP_ATFLAGS_DEFINED */
PRIVATE DREF DeeObject *DCALL libposix_fchownat_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	int dfd;
	/*utf-8*/char const *filename_str;
	DeeStringObject *filename;
	DeeObject *owner;
	DeeObject *group;
	int atflags;
	if (DeeArg_UnpackKw(argc,argv,kw,libposix_kwds_dfd_filename_owner_group_atflags,"doood:fchownat",&dfd,&filename,&owner,&group,&atflags))
	    goto err;
	if (DeeObject_AssertTypeExact(filename,&DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return libposix_fchownat_f_impl(dfd,filename_str,owner,group,atflags);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libposix_fchownat_f_impl(int dfd, /*utf-8*/char const *__restrict filename, DeeObject *__restrict owner, DeeObject *__restrict group, int atflags)
//[[[end]]]
{
#ifdef HAVE_FCHOWNAT
 uid_t owner_uid;
 gid_t group_gid;
 int result;
 if (DeeInt_Check(owner)) {
  if (DeeObject_AsUINT(owner,&owner_uid))
      goto err;
 } else {
  owner = DeeObject_CallAttrString(FS_MODULE,"User",1,&owner);
  if unlikely(!owner)
     goto err;
  result = DeeObject_AsUINT(owner,&owner_uid);
  Dee_Decref(owner);
  if unlikely(result)
     goto err;
 }
 if (DeeInt_Check(group)) {
  if (DeeObject_AsUINT(group,&group_gid))
      goto err;
 } else {
  group = DeeObject_CallAttrString(FS_MODULE,"Group",1,&group);
  if unlikely(!group)
     goto err;
  result = DeeObject_AsUINT(group,&group_gid);
  Dee_Decref(group);
  if unlikely(result)
     goto err;
 }
 EINTR_LABEL(again)
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 result = fchownat(dfd,filename,owner_uid,group_gid,atflags);
 DBG_ALIGNMENT_ENABLE();
 if (result < 0) {
  result = errno;
  HANDLE_EINTR(result,again)
  HANDLE_ENOSYS(result,err,"fchownat")
  HANDLE_EINVAL(result,err,"Invalid at-flags")
  HANDLE_ENOMEM(result,err,"Insufficient kernel memory to change ownership of %d:%s",dfd,filename)
  HANDLE_ENOENT_ENOTDIR(result,err,"File or directory %d:%s could not be found",dfd,filename)
  HANDLE_EROFS_ETXTBSY(result,err,"Read-only file %d:%s",dfd,filename)
  HANDLE_EBADF(error,err,"Invalid handle %d",dfd)
  DeeError_SysThrowf(&DeeError_SystemError,result,"Failed to change ownership of %d:%s",dfd,filename);
  goto err;
 }
 return DeeInt_NewInt(result);
err:
#else
 (void)dfd;
 (void)filename;
 (void)owner;
 (void)group;
 (void)atflags;
 err_unsupported("fchownat");
#endif
 return NULL;
}



#undef environ

#undef stat
#undef lstat
#undef getcwd
#undef gethostname
#undef chdir
#undef chmod
#undef lchmod
#undef chown
#undef lchown
#undef mkdir
#undef rmdir
#undef unlink
#undef remove
#undef rename
#undef link
#undef symlink
#undef readlink

#undef S_IFMT
#undef S_IFDIR
#undef S_IFCHR
#undef S_IFBLK
#undef S_IFREG
#undef S_IFIFO
#undef S_IFLNK
#undef S_IFSOCK
#undef S_ISUID
#undef S_ISGID
#undef S_ISVTX
#undef S_IRUSR
#undef S_IWUSR
#undef S_IXUSR
#undef S_IRGRP
#undef S_IWGRP
#undef S_IXGRP
#undef S_IROTH
#undef S_IWOTH
#undef S_IXOTH

#undef S_ISDIR
#undef S_ISCHR
#undef S_ISBLK
#undef S_ISREG
#undef S_ISFIFO
#undef S_ISLNK
#undef S_ISSOCK

#define DEFINE_LIBFS_FORWARD_WRAPPER(name,symbol_name) \
PRIVATE DEFINE_STRING(libposix_libfs_name_##name,symbol_name); \
PRIVATE DREF DeeObject *DCALL \
libposix_getfs_##name##_f(size_t UNUSED(argc), DeeObject **__restrict UNUSED(argv)) \
{ \
 return DeeObject_GetAttr(FS_MODULE,(DeeObject *)&libposix_libfs_name_##name); \
} \
PRIVATE DEFINE_CMETHOD(libposix_getfs_##name,&libposix_getfs_##name##_f);
#define DEFINE_LIBFS_FORWARD_WRAPPER_S(name) \
        DEFINE_LIBFS_FORWARD_WRAPPER(name,#name)
DEFINE_LIBFS_FORWARD_WRAPPER(opendir,"dir")
DEFINE_LIBFS_FORWARD_WRAPPER_S(environ)
DEFINE_LIBFS_FORWARD_WRAPPER_S(stat)
DEFINE_LIBFS_FORWARD_WRAPPER_S(lstat)
DEFINE_LIBFS_FORWARD_WRAPPER_S(getcwd)
DEFINE_LIBFS_FORWARD_WRAPPER_S(gethostname)
DEFINE_LIBFS_FORWARD_WRAPPER_S(chdir)
DEFINE_LIBFS_FORWARD_WRAPPER_S(chmod)
DEFINE_LIBFS_FORWARD_WRAPPER_S(lchmod)
DEFINE_LIBFS_FORWARD_WRAPPER_S(chown)
DEFINE_LIBFS_FORWARD_WRAPPER_S(lchown)
DEFINE_LIBFS_FORWARD_WRAPPER_S(mkdir)
DEFINE_LIBFS_FORWARD_WRAPPER_S(rmdir)
DEFINE_LIBFS_FORWARD_WRAPPER_S(unlink)
DEFINE_LIBFS_FORWARD_WRAPPER_S(remove)
DEFINE_LIBFS_FORWARD_WRAPPER_S(rename)
DEFINE_LIBFS_FORWARD_WRAPPER_S(link)
DEFINE_LIBFS_FORWARD_WRAPPER_S(symlink)
DEFINE_LIBFS_FORWARD_WRAPPER_S(readlink)

DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFMT)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFDIR)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFCHR)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFBLK)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFREG)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFIFO)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFLNK)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFSOCK)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISUID)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISGID)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISVTX)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IRUSR)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IWUSR)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IXUSR)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IRGRP)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IWGRP)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IXGRP)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IROTH)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IWOTH)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IXOTH)

DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISDIR)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISCHR)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISBLK)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISREG)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISFIFO)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISLNK)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISSOCK)

#undef DEFINE_LIBFS_FORWARD_WRAPPER_S
#undef DEFINE_LIBFS_FORWARD_WRAPPER

PRIVATE char const *import_table[] = {
    /* NOTE: Indices in this table must match those used by `*_MODULE' macros! */
    "fs",  /* #define FS_MODULE   DEX.d_imports[0] */
    NULL
};


PRIVATE struct dex_symbol symbols[] = {
    /* File control */
    LIBPOSIX_OPEN_DEF
    LIBPOSIX_CREAT_DEF
    LIBPOSIX_READ_DEF
    LIBPOSIX_WRITE_DEF
    LIBPOSIX_LSEEK_DEF
    LIBPOSIX_FSYNC_DEF
    LIBPOSIX_FDATASYNC_DEF
    LIBPOSIX_CLOSE_DEF
    LIBPOSIX_UMASK_DEF
    LIBPOSIX_DUP_DEF
    LIBPOSIX_DUP2_DEF
    LIBPOSIX_DUP3_DEF
    /* TODO: closerange() */
    /* TODO: lockf() */
    LIBPOSIX_PREAD_DEF
    /* TODO: readv() */
    /* TODO: preadv() */
    LIBPOSIX_PWRITE_DEF
    /* TODO: writev() */
    /* TODO: pwritev() */
    LIBPOSIX_ISATTY_DEF
    LIBPOSIX_PIPE_DEF
    LIBPOSIX_PIPE2_DEF
    /* TODO: fcntl() */
    /* TODO: ioctl() */
    /* TODO: posix_fallocate() */
    /* TODO: posix_fadvise() */

    /* Filesystem control */
    LIBPOSIX_TRUNCATE_DEF
    LIBPOSIX_FTRUNCATE_DEF
    LIBPOSIX_ACCESS_DEF
    LIBPOSIX_EUIDACCESS_DEF
    LIBPOSIX_FACCESSAT_DEF
    LIBPOSIX_FCHOWNAT_DEF
    /* TODO: fchmodat() */
    /* TODO: chflags() */
    /* TODO: lchflags() */
    /* TODO: chroot() */
    /* TODO: mkfifo() */
    /* TODO: mknod() */
    /* TODO: major() */
    /* TODO: minor() */
    /* TODO: mkdev() */
    /* TODO: sync() */
    /* TODO: utime() */
    /* TODO: pathconf() */
    /* TODO: fpathconf() */
    /* TODO: statvfs() */
    /* TODO: fstatvfs() */
    /* TODO: getxattr() */
    /* TODO: lgetxattr() */
    /* TODO: fgetxattr() */
    /* TODO: setxattr() */
    /* TODO: lsetxattr() */
    /* TODO: fsetxattr() */
    /* TODO: removexattr() */
    /* TODO: lremovexattr() */
    /* TODO: flremovexattr() */
    /* TODO: listxattr() */
    /* TODO: llistxattr() */
    /* TODO: flistxattr() */

    /* System information */
    /* TODO: uname() */
    /* TODO: confstr() */
    /* TODO: sysconf() */
    /* TODO: times() */
    /* TODO: getloadavg() */

    /* Process control */
    LIBPOSIX_GETPID_DEF
    LIBPOSIX_SYSTEM_DEF
    /* TODO: execv() */
    /* TODO: execve() */
    /* TODO: execvp() */
    /* TODO: execvpe() */
    /* TODO: cwait() */
    /* TODO: spawn() */
    /* TODO: spawne() */
    /* TODO: spawnp() */
    /* TODO: spawnpe() */

    /* Terminal control */
    /* TODO: ttyname() */
    /* TODO: ctermid() */
    /* TODO: openpty() */
    /* TODO: forkpty() */
    /* TODO: getlogin() */
    /* TODO: tcgetpgrp() */
    /* TODO: tcsetpgrp() */

    /* Scheduling control */
    LIBPOSIX_SCHED_YIELD_DEF
    /* TODO: sched_get_priority_min() */
    /* TODO: sched_get_priority_max() */
    /* TODO: sched_getparam() */
    /* TODO: sched_setparam() */
    /* TODO: sched_getscheduler() */
    /* TODO: sched_setscheduler() */
    /* TODO: sched_rr_get_interval() */
    /* TODO: sched_getaffinity() */
    /* TODO: sched_getaffinity() */
    /* TODO: nice() */
    /* TODO: getpriority() */
    /* TODO: setpriority() */
    /* TODO: fork() */
    /* TODO: wait() */
    /* TODO: wait3() */
    /* TODO: wait4() */
    /* TODO: waitid() */
    /* TODO: waitpid() */
    /* TODO: kill() */
    /* TODO: killpg() */
    /* TODO: getppid() */
    /* TODO: getpgrp() */
    /* TODO: setpgrp() */
    /* TODO: getpgid() */
    /* TODO: setpgid() */
    /* TODO: getsid() */
    /* TODO: setsid() */
    /* TODO: WCOREDUMP() */
    /* TODO: WIFCONTINUED() */
    /* TODO: WIFSTOPPED() */
    /* TODO: WIFSIGNALED() */
    /* TODO: WIFEXITED() */
    /* TODO: WEXITSTATUS() */
    /* TODO: WTERMSIG() */
    /* TODO: WSTOPSIG() */

    /* User/Permission control */
    /* TODO: geteuid() */
    /* TODO: seteuid() */
    /* TODO: getegid() */
    /* TODO: setegid() */
    /* TODO: getgid() */
    /* TODO: setgid() */
    /* TODO: getuid() */
    /* TODO: setuid() */
    /* TODO: setreuid() */
    /* TODO: setregid() */
    /* TODO: getgrouplist() */
    /* TODO: getgroups() */
    /* TODO: setgroups() */
    /* TODO: initgroups() */
    /* TODO: setresuid() */
    /* TODO: setresgid() */
    /* TODO: getresuid() */
    /* TODO: getresgid() */

    /* Random number generation */
    /* TODO: urandom() */
    /* TODO: getrandom() */

    /* Environ control */
    /* TODO: getenv() */
    /* TODO: setenv() */
    /* TODO: putenv() */
    /* TODO: unputenv() */
    /* TODO: clearenv() */

    /* Python-like helper functions */
    /* TODO: cpu_count() */
    /* TODO: get_inheritable() */
    /* TODO: set_inheritable() */

    /* Higher-level wrapper functions */
    /* TODO: popen() */
    /* TODO: fdopen() (Basically just a wrapper around `DeeFile_OpenFd') */

    /* Forward-aliases to `libfs' */
#define DEFINE_LIBFS_ALIAS_ALT(altname,name,libfs_name,proto) \
    { altname, (DeeObject *)&libposix_getfs_##name, MODSYM_FPROPERTY|MODSYM_FREADONLY, \
      DOC(proto "Alias for :fs." libfs_name) },
#define DEFINE_LIBFS_ALIAS_S_ALT(altname,name,proto) \
    { altname, \
     (DeeObject *)&libposix_getfs_##name, MODSYM_FPROPERTY|MODSYM_FREADONLY, \
      DOC(proto "Alias for :fs." #name) },
#define DEFINE_LIBFS_ALIAS(name,libfs_name,proto) \
        DEFINE_LIBFS_ALIAS_ALT(#name,name,libfs_name,proto)
#define DEFINE_LIBFS_ALIAS_S(name,proto) \
        DEFINE_LIBFS_ALIAS_S_ALT(DeeString_STR(&libposix_libfs_name_##name),name,proto)
    DEFINE_LIBFS_ALIAS(opendir,"dir","(path:?Dstring)\n")
    DEFINE_LIBFS_ALIAS_S(environ,"->?S?T2?Dstring?Dstring\n")
    DEFINE_LIBFS_ALIAS_S(stat,"(path:?Dstring)\n")
    DEFINE_LIBFS_ALIAS_S(lstat,"(path:?Dstring)\n")
    DEFINE_LIBFS_ALIAS_S(getcwd,"->?Dstring\n")
    DEFINE_LIBFS_ALIAS_S(gethostname,"->?Dstring\n")
    DEFINE_LIBFS_ALIAS_S(chdir,"(path:?Dstring)\n")
    DEFINE_LIBFS_ALIAS_S(chmod,"(path:?Dstring,mode:?X2?Dstring?Dint)\n")
    DEFINE_LIBFS_ALIAS_S(lchmod,"(path:?Dstring,mode:?X2?Dstring?Dint)\n")
    DEFINE_LIBFS_ALIAS_S(chown,"(path:?Dstring,user:?X3?Efs:user?Dstring?Dint,group:?X3?Efs:group?Dstring?Dint)\n")
    DEFINE_LIBFS_ALIAS_S(lchown,"(path:?Dstring,user:?X3?Efs:user?Dstring?Dint,group:?X3?Efs:group?Dstring?Dint)\n")
    DEFINE_LIBFS_ALIAS_S(mkdir,"(path:?Dstring,permissions:?X2?Dstring?Dint=!N)\n")
    DEFINE_LIBFS_ALIAS_S(rmdir,"(path:?Dstring)\n")
    DEFINE_LIBFS_ALIAS_S(unlink,"(path:?Dstring)\n")
    DEFINE_LIBFS_ALIAS_S(remove,"(path:?Dstring)\n")
    DEFINE_LIBFS_ALIAS_S(rename,"(existing_path:?Dstring,new_path:?Dstring)\n")
    DEFINE_LIBFS_ALIAS_S(link,"(existing_path:?X3?Dstring?DFile?Dint,new_path:?Dstring)\n")
    DEFINE_LIBFS_ALIAS_S(symlink,"(target_text:?Dstring,link_path:?Dstring,format_target=!t)\n")
    DEFINE_LIBFS_ALIAS_S(readlink,"(path:?Dstring)->?Dstring\n(fp:?DFile)->?Dstring\n(fd:?Dint)->?Dstring\n")
    DEFINE_LIBFS_ALIAS_ALT("fopendir",opendir,"dir","(fp:?DFile)\n(fd:?Dint)\n")
    DEFINE_LIBFS_ALIAS_S_ALT("fstat",stat,"(fp:?DFile)\n(fd:?Dint)\n")
    DEFINE_LIBFS_ALIAS_S_ALT("fchdir",chdir,"(fp:?DFile)\n(fd:?Dint)\n")
    DEFINE_LIBFS_ALIAS_S_ALT("fchmod",chmod,"(fp:?DFile,mode:?X2?Dstring?Dint)\n"
                                            "(fd:?Dint,mode:?X2?Dstring?Dint)\n")
    DEFINE_LIBFS_ALIAS_S_ALT("fchown",chown,"(fp:?DFile,user:?X3?Efs:user?Dstring?Dint,group:?X3?Efs:group?Dstring?Dint)\n"
                                            "(fd:?Dint,user:?X3?Efs:user?Dstring?Dint,group:?X3?Efs:group?Dstring?Dint)\n")

    /* Application exit control */
    LIBPOSIX_ATEXIT_DEF_DOC("Register a callback to-be invoked before #exit (Same as :deemon:Error.AppExit.atexit)")
    LIBPOSIX_EXIT_DEF_DOC("Terminate execution of deemon after invoking #atexit callbacks\n"
                          "Termination is done using the C $exit or $_exit functions, if available. However if these "
                          "functions are not provided by the host, an :AppExit error is thrown instead\n"
                          "When no @exitcode is given, the host's default default value of #EXIT_FAILURE, or $1 is used\n"
                          "This function never returns normally")
    LIBPOSIX__EXIT_DEF_DOC("Terminate execution of deemon without invoking #atexit callbacks (s.a. #exit)")
    LIBPOSIX_ABORT_DEF_DOC("Same as #_exit when passing #EXIT_FAILURE")

    /* stat.st_mode bits. */
    DEFINE_LIBFS_ALIAS_S(S_IFMT,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IFDIR,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IFCHR,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IFBLK,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IFREG,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IFIFO,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IFLNK,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IFSOCK,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_ISUID,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_ISGID,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_ISVTX,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IRUSR,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IWUSR,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IXUSR,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IRGRP,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IWGRP,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IXGRP,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IROTH,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IWOTH,"->?Dint\n")
    DEFINE_LIBFS_ALIAS_S(S_IXOTH,"->?Dint\n")

    /* stat.st_mode helper functions. */
    DEFINE_LIBFS_ALIAS_S(S_ISDIR,"(mode:?Dint)->?Dbool\n")
    DEFINE_LIBFS_ALIAS_S(S_ISCHR,"(mode:?Dint)->?Dbool\n")
    DEFINE_LIBFS_ALIAS_S(S_ISBLK,"(mode:?Dint)->?Dbool\n")
    DEFINE_LIBFS_ALIAS_S(S_ISREG,"(mode:?Dint)->?Dbool\n")
    DEFINE_LIBFS_ALIAS_S(S_ISFIFO,"(mode:?Dint)->?Dbool\n")
    DEFINE_LIBFS_ALIAS_S(S_ISLNK,"(mode:?Dint)->?Dbool\n")
    DEFINE_LIBFS_ALIAS_S(S_ISSOCK,"(mode:?Dint)->?Dbool\n")

#undef DEFINE_LIBFS_ALIAS_S
#undef DEFINE_LIBFS_ALIAS
#undef DEFINE_LIBFS_ALIAS_S_ALT
#undef DEFINE_LIBFS_ALIAS_ALT


    /* O_* values */
    LIBPOSIX_O_RDONLY_DEF
    LIBPOSIX_O_WRONLY_DEF
    LIBPOSIX_O_RDWR_DEF
    LIBPOSIX_O_APPEND_DEF
    LIBPOSIX_O_CREAT_DEF
    LIBPOSIX_O_TRUNC_DEF
    LIBPOSIX_O_EXCL_DEF
    LIBPOSIX_O_TEXT_DEF
    LIBPOSIX_O_BINARY_DEF
    LIBPOSIX_O_WTEXT_DEF
    LIBPOSIX_O_U16TEXT_DEF
    LIBPOSIX_O_U8TEXT_DEF
    LIBPOSIX_O_CLOEXEC_DEF
#ifdef O_CLOEXEC /* Alias */
    { "O_NOINHERIT", (DeeObject *)&libposix_O_CLOEXEC, MODSYM_FREADONLY|MODSYM_FCONSTEXPR, "Alias for #O_CLOEXEC" },
#endif
    LIBPOSIX_O_TEMPORARY_DEF
    LIBPOSIX_O_SHORT_LIVED_DEF
    LIBPOSIX_O_OBTAIN_DIR_DEF
    LIBPOSIX_O_SEQUENTIAL_DEF
    LIBPOSIX_O_RANDOM_DEF
    LIBPOSIX_O_NOCTTY_DEF
    LIBPOSIX_O_NONBLOCK_DEF
#ifdef O_NONBLOCK /* Alias */
    { "O_NDELAY", (DeeObject *)&libposix_O_NONBLOCK, MODSYM_FREADONLY|MODSYM_FCONSTEXPR, "Alias for #O_NDELAY" },
#endif
    LIBPOSIX_O_SYNC_DEF
    LIBPOSIX_O_RSYNC_DEF
    LIBPOSIX_O_DSYNC_DEF
    LIBPOSIX_O_ASYNC_DEF
    LIBPOSIX_O_DIRECT_DEF
    LIBPOSIX_O_LARGEFILE_DEF
    LIBPOSIX_O_DIRECTORY_DEF
    LIBPOSIX_O_NOFOLLOW_DEF
    LIBPOSIX_O_NOATIME_DEF
    LIBPOSIX_O_PATH_DEF
    LIBPOSIX_O_TMPFILE_DEF
    LIBPOSIX_O_CLOFORK_DEF
    LIBPOSIX_O_SYMLINK_DEF
    LIBPOSIX_O_DOSPATH_DEF
    LIBPOSIX_O_SHLOCK_DEF
    LIBPOSIX_O_EXLOCK_DEF
    LIBPOSIX_O_XATTR_DEF
    LIBPOSIX_O_EXEC_DEF
    LIBPOSIX_O_SEARCH_DEF
    LIBPOSIX_O_TTY_INIT_DEF
    LIBPOSIX_O_NOLINKS_DEF

    /* SEEK_* values */
    LIBPOSIX_SEEK_SET_DEF
    LIBPOSIX_SEEK_CUR_DEF
    LIBPOSIX_SEEK_END_DEF
    LIBPOSIX_SEEK_HOLE_DEF
    LIBPOSIX_SEEK_DATA_DEF

    /* Errno codes */
    LIBPOSIX_EPERM_DEF
    LIBPOSIX_ENOENT_DEF
    LIBPOSIX_ESRCH_DEF
    LIBPOSIX_EINTR_DEF
    LIBPOSIX_EIO_DEF
    LIBPOSIX_ENXIO_DEF
    LIBPOSIX_E2BIG_DEF
    LIBPOSIX_ENOEXEC_DEF
    LIBPOSIX_EBADF_DEF
    LIBPOSIX_ECHILD_DEF
    LIBPOSIX_EAGAIN_DEF
    LIBPOSIX_ENOMEM_DEF
    LIBPOSIX_EACCES_DEF
    LIBPOSIX_EFAULT_DEF
    LIBPOSIX_ENOTBLK_DEF
    LIBPOSIX_EBUSY_DEF
    LIBPOSIX_EEXIST_DEF
    LIBPOSIX_EXDEV_DEF
    LIBPOSIX_ENODEV_DEF
    LIBPOSIX_ENOTDIR_DEF
    LIBPOSIX_EISDIR_DEF
    LIBPOSIX_EINVAL_DEF
    LIBPOSIX_ENFILE_DEF
    LIBPOSIX_EMFILE_DEF
    LIBPOSIX_ENOTTY_DEF
    LIBPOSIX_ETXTBSY_DEF
    LIBPOSIX_EFBIG_DEF
    LIBPOSIX_ENOSPC_DEF
    LIBPOSIX_ESPIPE_DEF
    LIBPOSIX_EROFS_DEF
    LIBPOSIX_EMLINK_DEF
    LIBPOSIX_EPIPE_DEF
    LIBPOSIX_EDOM_DEF
    LIBPOSIX_ERANGE_DEF
    LIBPOSIX_EDEADLK_DEF
    LIBPOSIX_ENAMETOOLONG_DEF
    LIBPOSIX_ENOLCK_DEF
    LIBPOSIX_ENOSYS_DEF
    LIBPOSIX_ENOTEMPTY_DEF
    LIBPOSIX_ELOOP_DEF
    LIBPOSIX_EWOULDBLOCK_DEF
    LIBPOSIX_ENOMSG_DEF
    LIBPOSIX_EIDRM_DEF
    LIBPOSIX_ECHRNG_DEF
    LIBPOSIX_EL2NSYNC_DEF
    LIBPOSIX_EL3HLT_DEF
    LIBPOSIX_EL3RST_DEF
    LIBPOSIX_ELNRNG_DEF
    LIBPOSIX_EUNATCH_DEF
    LIBPOSIX_ENOCSI_DEF
    LIBPOSIX_EL2HLT_DEF
    LIBPOSIX_EBADE_DEF
    LIBPOSIX_EBADR_DEF
    LIBPOSIX_EXFULL_DEF
    LIBPOSIX_ENOANO_DEF
    LIBPOSIX_EBADRQC_DEF
    LIBPOSIX_EBADSLT_DEF
    LIBPOSIX_EDEADLOCK_DEF
    LIBPOSIX_EBFONT_DEF
    LIBPOSIX_ENOSTR_DEF
    LIBPOSIX_ENODATA_DEF
    LIBPOSIX_ETIME_DEF
    LIBPOSIX_ENOSR_DEF
    LIBPOSIX_ENONET_DEF
    LIBPOSIX_ENOPKG_DEF
    LIBPOSIX_EREMOTE_DEF
    LIBPOSIX_ENOLINK_DEF
    LIBPOSIX_EADV_DEF
    LIBPOSIX_ESRMNT_DEF
    LIBPOSIX_ECOMM_DEF
    LIBPOSIX_EPROTO_DEF
    LIBPOSIX_EMULTIHOP_DEF
    LIBPOSIX_EDOTDOT_DEF
    LIBPOSIX_EBADMSG_DEF
    LIBPOSIX_EOVERFLOW_DEF
    LIBPOSIX_ENOTUNIQ_DEF
    LIBPOSIX_EBADFD_DEF
    LIBPOSIX_EREMCHG_DEF
    LIBPOSIX_ELIBACC_DEF
    LIBPOSIX_ELIBBAD_DEF
    LIBPOSIX_ELIBSCN_DEF
    LIBPOSIX_ELIBMAX_DEF
    LIBPOSIX_ELIBEXEC_DEF
    LIBPOSIX_EILSEQ_DEF
    LIBPOSIX_ERESTART_DEF
    LIBPOSIX_ESTRPIPE_DEF
    LIBPOSIX_EUSERS_DEF
    LIBPOSIX_ENOTSOCK_DEF
    LIBPOSIX_EDESTADDRREQ_DEF
    LIBPOSIX_EMSGSIZE_DEF
    LIBPOSIX_EPROTOTYPE_DEF
    LIBPOSIX_ENOPROTOOPT_DEF
    LIBPOSIX_EPROTONOSUPPORT_DEF
    LIBPOSIX_ESOCKTNOSUPPORT_DEF
    LIBPOSIX_EOPNOTSUPP_DEF
    LIBPOSIX_EPFNOSUPPORT_DEF
    LIBPOSIX_EAFNOSUPPORT_DEF
    LIBPOSIX_EADDRINUSE_DEF
    LIBPOSIX_EADDRNOTAVAIL_DEF
    LIBPOSIX_ENETDOWN_DEF
    LIBPOSIX_ENETUNREACH_DEF
    LIBPOSIX_ENETRESET_DEF
    LIBPOSIX_ECONNABORTED_DEF
    LIBPOSIX_ECONNRESET_DEF
    LIBPOSIX_ENOBUFS_DEF
    LIBPOSIX_EISCONN_DEF
    LIBPOSIX_ENOTCONN_DEF
    LIBPOSIX_ESHUTDOWN_DEF
    LIBPOSIX_ETOOMANYREFS_DEF
    LIBPOSIX_ETIMEDOUT_DEF
    LIBPOSIX_ECONNREFUSED_DEF
    LIBPOSIX_EHOSTDOWN_DEF
    LIBPOSIX_EHOSTUNREACH_DEF
    LIBPOSIX_EALREADY_DEF
    LIBPOSIX_EINPROGRESS_DEF
    LIBPOSIX_ESTALE_DEF
    LIBPOSIX_EUCLEAN_DEF
    LIBPOSIX_ENOTNAM_DEF
    LIBPOSIX_ENAVAIL_DEF
    LIBPOSIX_EISNAM_DEF
    LIBPOSIX_EREMOTEIO_DEF
    LIBPOSIX_EDQUOT_DEF
    LIBPOSIX_ENOMEDIUM_DEF
    LIBPOSIX_EMEDIUMTYPE_DEF
    LIBPOSIX_ECANCELED_DEF
    LIBPOSIX_ENOKEY_DEF
    LIBPOSIX_EKEYEXPIRED_DEF
    LIBPOSIX_EKEYREVOKED_DEF
    LIBPOSIX_EKEYREJECTED_DEF
    LIBPOSIX_EOWNERDEAD_DEF
    LIBPOSIX_ENOTRECOVERABLE_DEF
    LIBPOSIX_ERFKILL_DEF
    LIBPOSIX_EHWPOISON_DEF
    LIBPOSIX_ELBIN_DEF
    LIBPOSIX_EPROCLIM_DEF
    LIBPOSIX_EFTYPE_DEF
    LIBPOSIX_ENMFILE_DEF
    LIBPOSIX_ENOTSUP_DEF
    LIBPOSIX_ENOSHARE_DEF
    LIBPOSIX_ECASECLASH_DEF

    LIBPOSIX_EXIT_SUCCESS_DEF
    LIBPOSIX_EXIT_FAILURE_DEF

    LIBPOSIX_R_OK_DEF
    LIBPOSIX_W_OK_DEF
    LIBPOSIX_X_OK_DEF
    LIBPOSIX_F_OK_DEF

    LIBPOSIX_AT_FDCWD_DEF
    LIBPOSIX_AT_SYMLINK_NOFOLLOW_DEF
    LIBPOSIX_AT_REMOVEDIR_DEF
    LIBPOSIX_AT_SYMLINK_FOLLOW_DEF
    LIBPOSIX_AT_NO_AUTOMOUNT_DEF
    LIBPOSIX_AT_EMPTY_PATH_DEF
    LIBPOSIX_AT_EACCESS_DEF
    LIBPOSIX_AT_REMOVEREG_DEF
    LIBPOSIX_AT_DOSPATH_DEF
    LIBPOSIX_AT_FDROOT_DEF
    LIBPOSIX_AT_THIS_TASK_DEF
    LIBPOSIX_AT_THIS_MMAN_DEF
    LIBPOSIX_AT_THIS_STACK_DEF

    { "errno", (DeeObject *)&libposix_errno_get, MODSYM_FPROPERTY,
      DOC("->?Dint\n"
          "Read/write the C errno thread-local variable") },
    { NULL, NULL, MODSYM_FNORMAL },
    { NULL, (DeeObject *)&libposix_errno_set, MODSYM_FNORMAL },
    /* TODO: strerror() */

    { NULL }
};

PUBLIC struct dex DEX = {
    /* .d_symbols      = */symbols,
    /* .d_init         = */NULL,
    /* .d_fini         = */NULL,
    /* .d_import_names = */{ import_table }
};

DECL_END

#endif /* !GUARD_DEX_WIN32_LIBWIN32_C */
