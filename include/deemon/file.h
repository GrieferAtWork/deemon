/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_FILE_H
#define GUARD_DEEMON_FILE_H 1

#include "api.h"

#include "object.h"

#ifndef CONFIG_NO_THREADS
#include "util/rwlock.h"
#endif /* !CONFIG_NO_THREADS */

#include <stdarg.h>  /* va_list */
#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL */

#if (!defined(DEESYSTEM_FILE_USE_WINDOWS) && \
     !defined(DEESYSTEM_FILE_USE_UNIX) &&    \
     !defined(DEESYSTEM_FILE_USE_STDIO) &&   \
     !defined(DEESYSTEM_FILE_USE_STUB))
#ifdef CONFIG_HOST_WINDOWS
#define DEESYSTEM_FILE_USE_WINDOWS 1
#elif defined(CONFIG_HOST_UNIX)
#define DEESYSTEM_FILE_USE_UNIX 1
#else /* ... */
#include "system-features.h"
#if (defined(CONFIG_HAVE_read) || defined(CONFIG_HAVE_write) ||    \
     defined(CONFIG_HAVE_open) || defined(CONFIG_HAVE_open64) ||   \
     defined(CONFIG_HAVE_creat) || defined(CONFIG_HAVE_creat64) || \
     defined(CONFIG_HAVE_close) || defined(CONFIG_HAVE_lseek) ||   \
     defined(CONFIG_HAVE_lseek64))
/* Check if there are any fd-based functions available */
#define DEESYSTEM_FILE_USE_UNIX 1
#elif (defined(CONFIG_HAVE_fopen) || defined(CONFIG_HAVE_fopen64) || \
       defined(CONFIG_HAVE_fread) || defined(CONFIG_HAVE_fwrite) ||  \
       defined(CONFIG_HAVE_fgetc) || defined(CONFIG_HAVE_fputc))
/* Check if there are any FILE-based functions available */
#define DEESYSTEM_FILE_USE_STDIO 1
#else /* FILE-based I/O mechanism */
/* Fallback: Use stub file apis */
#define DEESYSTEM_FILE_USE_STUB 1
#endif /* Unknown I/O mechanism */
#endif /* !... */
#endif /* !... */

#ifndef SEEK_SET
#define SEEK_SET 0
#endif /* !SEEK_SET */

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif /* !SEEK_CUR */

#ifndef SEEK_END
#define SEEK_END 2
#endif /* !SEEK_END */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_file_object        file_object
#define Dee_filetype_object    filetype_object
#define FILE_OBJECT_HEAD       Dee_FILE_OBJECT_HEAD
#define FILE_OBJECT_HEAD_INIT  Dee_FILE_OBJECT_HEAD_INIT
#define LFILE_OBJECT_HEAD      Dee_LFILE_OBJECT_HEAD
#define LFILE_OBJECT_HEAD_INIT Dee_LFILE_OBJECT_HEAD_INIT
#endif /* DEE_SOURCE */


typedef struct Dee_file_object DeeFileObject;
typedef struct Dee_filetype_object DeeFileTypeObject;

#define Dee_FILEIO_FNORMAL      0x0000 /* Normal I/O flags. */
#define Dee_FILEIO_FNONBLOCKING 0x0001 /* Do not block when reading/writing data. */
typedef unsigned int Dee_ioflag_t; /* Set of `Dee_FILEIO_F*' */
#ifdef DEE_SOURCE
typedef Dee_ioflag_t dioflag_t;
#endif /* DEE_SOURCE */


struct Dee_file_object {
	Dee_OBJECT_HEAD_EX(DeeFileTypeObject)
#define Dee_FILE_OBJECT_HEAD             Dee_OBJECT_HEAD_EX(DeeFileTypeObject)
#define Dee_FILE_OBJECT_HEAD_INIT(type)  Dee_OBJECT_HEAD_INIT(type)
#define DeeFileObject_Init(self, type)   (DeeObject_Init(self, type))
#ifdef CONFIG_NO_THREADS
#define Dee_FILE_OBJECT_HEADL            Dee_OBJECT_HEAD_EX(DeeFileTypeObject)
#define Dee_FILE_OBJECT_HEADL_INIT(type) Dee_OBJECT_HEAD_INIT(type)
#define DeeFileObject_InitL(self, type)  (DeeObject_Init(self, type))
#define DeeFile_LockRead(self)           (void)0
#define DeeFile_LockWrite(self)          (void)0
#define DeeFile_LockEndRead(self)        (void)0
#define DeeFile_LockEndWrite(self)       (void)0
#define DeeFile_TryLockRead(self)        1
#define DeeFile_TryLockWrite(self)       1
#else /* CONFIG_NO_THREADS */
#define Dee_LFILE_OBJECT_HEAD            Dee_OBJECT_HEAD_EX(DeeFileTypeObject) Dee_rwlock_t fo_lock;
#define Dee_LFILE_OBJECT_HEAD_INIT(type) Dee_OBJECT_HEAD_INIT(type), RWLOCK_INIT
#define DeeLFileObject_Init(self, type)  (DeeObject_Init(self, type), rwlock_init(&(self)->fo_lock))
#define DeeFile_LockRead(self)           Dee_rwlock_read(&(self)->fo_lock)
#define DeeFile_LockWrite(self)          Dee_rwlock_write(&(self)->fo_lock)
#define DeeFile_LockEndRead(self)        Dee_rwlock_endread(&(self)->fo_lock)
#define DeeFile_LockEndWrite(self)       Dee_rwlock_endwrite(&(self)->fo_lock)
#define DeeFile_TryLockRead(self)        Dee_rwlock_tryread(&(self)->fo_lock)
#define DeeFile_TryLockWrite(self)       Dee_rwlock_trywrite(&(self)->fo_lock)
#endif /* !CONFIG_NO_THREADS */
};

/* The underlying system file descriptor type. */
/* When `CONFIG_HOST_WINDOWS' is defined:
 *     HANDLE
 * else: When at least one of [`read', `write', [<some-other-functions-using-fds>]] exist:
 *     int
 * else: When `FILE' is defined in `<stdio.h>':
 *     `FILE *' (stored as a `void *')
 * else:
 *     int
 */

#undef DeeSysFD_IS_HANDLE /* Window's `HANDLE' */
#undef DeeSysFD_IS_INT    /* Unix's `int' */
#undef DeeSysFD_IS_FILE   /* Stdio's `FILE *' */
#if defined(DEESYSTEM_FILE_USE_WINDOWS)
#define DeeSysFD_IS_HANDLE 1
#elif defined(DEESYSTEM_FILE_USE_UNIX)
#define DeeSysFD_IS_INT 1
#elif defined(DEESYSTEM_FILE_USE_STDIO)
#define DeeSysFD_IS_FILE 1
#else /* ... */
/* Fallback: Just assume FD-based file I/O (even though at this point
 *           it's most likely that no I/O at all is supported, which
 *           will cause the deemon sources to stub out most APIs...) */
#define DeeSysFD_IS_INT 1
#endif /* !... */

#define DeeSysFD_HANDLE_GETSET  "osfhandle_np"
#define DeeSysFD_INT_GETSET     "fileno_np"

#ifdef DeeSysFD_IS_HANDLE
#define DeeSysFD_Close(x) CloseHandle(x)
#undef  DeeSysFD_SIGNED
#define DeeSysFD_SIZE    __SIZEOF_POINTER__
#define DeeSysFD_INVALID ((DeeSysFD)-1l)
#define DeeSysFD_GETSET  DeeSysFD_HANDLE_GETSET
typedef void *DeeSysFD;
#endif /* DeeSysFD_IS_HANDLE */

#ifdef DeeSysFD_IS_INT
#if !defined(GUARD_DEEMON_SYSTEM_FEATURES_H) || defined(CONFIG_HAVE_close)
#define DeeSysFD_Close(x) close(x)
#endif /* !GUARD_DEEMON_SYSTEM_FEATURES_H || CONFIG_HAVE_close */
#define DeeSysFD_SIGNED  1
#define DeeSysFD_SIZE    __SIZEOF_INT__
#define DeeSysFD_INVALID (-1)
#define DeeSysFD_GETSET  DeeSysFD_INT_GETSET
typedef int DeeSysFD;
#endif /* DeeSysFD_IS_INT */

#ifdef DeeSysFD_IS_FILE
#if !defined(GUARD_DEEMON_SYSTEM_FEATURES_H) || defined(CONFIG_HAVE_fclose)
#define DeeSysFD_Close(x) fclose(x)
#endif /* !GUARD_DEEMON_SYSTEM_FEATURES_H || CONFIG_HAVE_fclose */
#undef  DeeSysFD_SIGNED
#define DeeSysFD_SIZE    __SIZEOF_POINTER__
#define DeeSysFD_INVALID NULL
typedef void *DeeSysFD; /* FILE * */
#endif /* DeeSysFD_IS_FILE */


#ifndef DeeSysFD_Close
#define DeeSysFD_Close(x) (void)0
#endif /* !DeeSysFD_Close */


#ifndef CONFIG_FILENO_DENY_ARBITRARY_INTEGERS
#ifdef DeeSysFD_SIGNED
#define DeeObject_AsFd(self, result) DeeObject_AsXUInt(DeeSysFD_SIZE, self, result)
#else /* DeeSysFD_SIGNED */
#define DeeObject_AsFd(self, result) DeeObject_AsXInt(DeeSysFD_SIZE, self, result)
#endif /* !DeeSysFD_SIGNED */
#endif /* !CONFIG_FILENO_DENY_ARBITRARY_INTEGERS */


#ifdef DEE_SOURCE
#define FILE_OPERATOR_READ   OPERATOR_EXTENDED(0x0000) /* `operator read(buf: Bytes): int'  */
#define FILE_OPERATOR_WRITE  OPERATOR_EXTENDED(0x0001) /* `operator write(buf: Bytes): int' */
#define FILE_OPERATOR_SEEK   OPERATOR_EXTENDED(0x0002) /* `operator seek(off: int, whence: int): int' */
#define FILE_OPERATOR_SYNC   OPERATOR_EXTENDED(0x0003) /* `operator sync()' */
#define FILE_OPERATOR_TRUNC  OPERATOR_EXTENDED(0x0004) /* `operator trunc()'  or  `operator trunc(size: int)' */
#define FILE_OPERATOR_CLOSE  OPERATOR_EXTENDED(0x0005) /* `operator close()' */
#define FILE_OPERATOR_PREAD  OPERATOR_EXTENDED(0x0006) /* `operator pread(buf: Bytes, pos: int): int' */
#define FILE_OPERATOR_PWRITE OPERATOR_EXTENDED(0x0007) /* `operator pwrite(buf: Bytes, pos: int): int' */
#define FILE_OPERATOR_GETC   OPERATOR_EXTENDED(0x0008) /* `operator getc(): int' */
#define FILE_OPERATOR_UNGETC OPERATOR_EXTENDED(0x0009) /* `operator ungetc(ch: int): int' */
#define FILE_OPERATOR_PUTC   OPERATOR_EXTENDED(0x000a) /* `operator putc(ch: int): int' */
#define FILE_OPERATOR_COUNT                    0x000b
#endif /* DEE_SOURCE */

struct Dee_filetype_object {
	DeeTypeObject       ft_base; /* Underlying type. */
	/* File operators. (Ignored unless the `TF_HASFILEOPS' feature bit is set) */
	WUNUSED_T NONNULL_T((1, 2)) size_t    (DCALL *ft_read)(DeeFileObject *__restrict self, void *__restrict buffer, size_t bufsize, Dee_ioflag_t flags);
	WUNUSED_T NONNULL_T((1, 2)) size_t    (DCALL *ft_write)(DeeFileObject *__restrict self, void const *__restrict buffer, size_t bufsize, Dee_ioflag_t flags);
	/* @param: whence: One of `SEEK_*' from `<stdio.h>' */
	WUNUSED_T NONNULL_T((1))    Dee_pos_t (DCALL *ft_seek)(DeeFileObject *__restrict self, Dee_off_t off, int whence);
	WUNUSED_T NONNULL_T((1))    int       (DCALL *ft_sync)(DeeFileObject *__restrict self);
	WUNUSED_T NONNULL_T((1))    int       (DCALL *ft_trunc)(DeeFileObject *__restrict self, Dee_pos_t size);
	WUNUSED_T NONNULL_T((1))    int       (DCALL *ft_close)(DeeFileObject *__restrict self);
	WUNUSED_T NONNULL_T((1, 2)) size_t    (DCALL *ft_pread)(DeeFileObject *__restrict self, void *__restrict buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
	WUNUSED_T NONNULL_T((1, 2)) size_t    (DCALL *ft_pwrite)(DeeFileObject *__restrict self, void const *__restrict buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
#define GETC_EOF (-1)
#define GETC_ERR (-2)
	/* Read and return one byte, or `GETC_EOF' for EOF and `GETC_ERR' if an error occurred. */
	WUNUSED_T NONNULL_T((1))    int       (DCALL *ft_getc)(DeeFileObject *__restrict self, Dee_ioflag_t flags);
	/* Return a previous read byte. (Required for implementing scanf())
	 * NOTE: The return value of this function is `GETC_EOF' for EOF, `GETC_ERR' for errors, and `0' for success. */
	WUNUSED_T NONNULL_T((1))    int       (DCALL *ft_ungetc)(DeeFileObject *__restrict self, int ch);
	/* Write a single byte to the file.
	 * NOTE: The return value of this function is `GETC_EOF' for EOF, `GETC_ERR' for errors, and `0' for success. */
	WUNUSED_T NONNULL_T((1))    int       (DCALL *ft_putc)(DeeFileObject *__restrict self, int ch, Dee_ioflag_t flags);
};

/* The type object for file types.
 * >> Because file types are extensions upon regular
 *    types, they also need _their_ own type.
 * In user-code, this type can be address as follows:
 * >> import file
 * >> ...
 * >> print file;           // DeeFile_Type
 * >> print type file;      // DeeFileType_Type
 * >> print type type file; // DeeType_Type */
DDATDEF DeeTypeObject DeeFileType_Type;
#define DeeFileType_Check(ob)      DeeObject_InstanceOf(ob, &DeeFileType_Type)
#define DeeFileType_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeFileType_Type)

/* Base class for all file types. */
DDATDEF DeeFileTypeObject DeeFile_Type;
#define DeeFile_Check(ob)      DeeObject_InstanceOf(ob, (DeeTypeObject *)&DeeFile_Type)
#define DeeFile_CheckExact(ob) DeeObject_InstanceOfExact(ob, (DeeTypeObject *)&DeeFile_Type)

/* Builtin system file sub-classes.
 * NOTE: When not implemented by the host, attempting to use these
 *       types will cause an `Error.RuntimeError.NotImplemented'. */
DDATDEF DeeFileTypeObject DeeSystemFile_Type; /* A system file. (Usually contains a generic descriptor, such as `int', `HANDLE' or `FILE *') */
DDATDEF DeeFileTypeObject     DeeFSFile_Type; /* A file-system file. (Created using `File.open(...)') */
#define DeeSystemFile_Check(ob) DeeObject_InstanceOf(ob, (DeeTypeObject *)&DeeSystemFile_Type)

/*  A buffering file that is basically what stdio's `FILE' is to a HANDLE/fd.
 * (Providing getc/ungetc and fully/line-buffered I/O operations)
 *  HINT: Any custom attribute of the underlying file is automatically forwarded.
 *  NOTE: Buffered files can be created using the type constructor:
 *        >> buffer(file base_stream, string mode = "write,sync,auto", size_t bufsize = 0);
 *           @param: mode:    One of "auto", "full", "line" or "none", optionally prefixed with:
 *                            - `w', `w-', `w,', `write,' or suffixed with `+' to make the buffer writable.
 *                            - `s', `s-', `s,', `sync,' to cause the underlying file to be synchronized whenever the buffer is.
 *                            When not given, default to `write,full'
 *           @param: bufsize: Buffer size hint, or ZERO(0) to determine automatically.
 *        Buffering behavior can later be adjusted using `setvbuf()':
 *        >> setbuf(string mode, size_t bufsize = 0) -> none;
 *           The argument are the same as for the constructor. */
DDATDEF DeeFileTypeObject DeeFileBuffer_Type;


/* HINT: All operator invocation functions below correctly handle `self' not being a file at all. */

/* File operator invocation.
 * @return: (size_t)-1: Error */
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeFile_Read(DeeObject *__restrict self, void *__restrict buffer, size_t bufsize);
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeFile_Readf(DeeObject *__restrict self, void *__restrict buffer, size_t bufsize, Dee_ioflag_t flags);
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeFile_Write(DeeObject *__restrict self, void const *__restrict buffer, size_t bufsize);
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeFile_Writef(DeeObject *__restrict self, void const *__restrict buffer, size_t bufsize, Dee_ioflag_t flags);
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeFile_PRead(DeeObject *__restrict self, void *__restrict buffer, size_t bufsize, Dee_pos_t pos);
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeFile_PReadf(DeeObject *__restrict self, void *__restrict buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeFile_PWrite(DeeObject *__restrict self, void const *__restrict buffer, size_t bufsize, Dee_pos_t pos);
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeFile_PWritef(DeeObject *__restrict self, void const *__restrict buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
/* Similar to functions above, but re-attempt to read/write until an error, or EOF.
 * >> Useful for unbuffered input/output streams that can only process data at a specific rate. */
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeFile_ReadAll(DeeObject *__restrict self, void *__restrict buffer, size_t bufsize);
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DPRINTER_CC DeeFile_WriteAll(DeeObject *__restrict self, void const *__restrict buffer, size_t bufsize);
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeFile_PReadAll(DeeObject *__restrict self, void *__restrict buffer, size_t bufsize, Dee_pos_t pos);
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeFile_PWriteAll(DeeObject *__restrict self, void const *__restrict buffer, size_t bufsize, Dee_pos_t pos);
/* @throw NotImplemented: The file does not support seeking.
 * @return: (Dee_pos_t)-1: Error */
DFUNDEF WUNUSED NONNULL((1)) Dee_pos_t DCALL DeeFile_Seek(DeeObject *__restrict self, Dee_off_t off, int whence);
#define DeeFile_Tell(self)        DeeFile_Seek(self, 0, SEEK_CUR)
#define DeeFile_Rewind(self)      DeeFile_Seek(self, 0, SEEK_SET)
#define DeeFile_SetPos(self, pos) DeeFile_Seek(self, (Dee_pos_t)(pos), SEEK_SET)
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeFile_Sync(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeFile_Trunc(DeeObject *__restrict self, Dee_pos_t size);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeFile_TruncHere(DeeObject *__restrict self, Dee_pos_t *psize);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeFile_Close(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeFile_Getc(DeeObject *__restrict self); /* @return: GETC_ERR: Error */
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeFile_Getcf(DeeObject *__restrict self, Dee_ioflag_t flags); /* @return: GETC_ERR: Error */
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeFile_Ungetc(DeeObject *__restrict self, int ch); /* @return: GETC_ERR: Error */
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeFile_Putc(DeeObject *__restrict self, int ch); /* @return: GETC_ERR: Error */
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeFile_Putcf(DeeObject *__restrict self, int ch, Dee_ioflag_t flags); /* @return: GETC_ERR: Error */

/* Returns the total size of a given file stream.
 * If the file doesn't support retrieval of its
 * size, a NotImplemented error is thrown.
 * NOTE: This function is equivalent to calling a member function `size()',
 *       which file objects default-implement by temporarily seeking to the
 *       end of the file and determining where that position is located at.
 * @return: * : The size of the given file `self' in bytes.
 * @return: (Dee_pos_t)-1: An error occurred. */
DFUNDEF WUNUSED NONNULL((1)) Dee_pos_t DCALL DeeFile_GetSize(DeeObject *__restrict self);

/* Check if the given file is an interactive device.
 * HINT: In actuality, this function checks for a sub-class of `DeeFileType_Type' and
 *       invokes `self.isatty()' without arguments, casting the return value to bool.
 *       This function is used to implement the auto-buffering mode of `File.Buffer'
 * @return: > 0 : The file is a TTY
 * @return:   0 : The file isn't a TTY
 * @return: < 0 : An error occurred. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeFile_IsAtty(DeeObject *__restrict self);

/* Return the system file descriptor of the given file, or throw
 * an error and return `DeeSysFD_INVALID' if the file was closed,
 * or doesn't refer to a file carrying a descriptor.
 * Note that this function queries the `DeeSysFD_GETSET' attribute
 * of the given object, and always fails if `DeeSysFD_GETSET' isn't
 * defined for the configuration used when deemon was built.
 * NOTE: This function doesn't require that `self' actually be
 *       derived from a `deemon.File'!
 * @return: * :               The used system fD. (either a `HANDLE', `fd_t' or `FILE *')
 * @return: DeeSysFD_INVALID: An error occurred. */
DFUNDEF WUNUSED NONNULL((1)) DeeSysFD DCALL DeeFile_GetSysFD(DeeObject *__restrict self);

/* Retrieve and return the filename used to open the given file.
 * NOTE: This function automatically asserts that `self'
 *       is a `File', throwing a TypeError if it isn't.
 * For this purpose, `DeeSystemFile_Filename()' is invoked if `self'
 * is a system file, however if it isn't, `self.filename' will be
 * retrieved (using `operator getattr()') and after asserting the
 * result to be a string object, its value will be returned instead.
 * This function should be used by library functions that wish to
 * operate on a path, thus allowing them to accept file objects just
 * as well as strings for operations:
 * >> if (!DeeString_Check(arg)) {
 * >>     arg = DeeFile_Filename(arg);
 * >>     if unlikely(!arg) goto err;
 * >> } else {
 * >>     Dee_Incref(arg);
 * >> }
 * >> ... // Operate on a filename string `arg'
 * >> Dee_Decref(arg); */
DFUNDEF WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeFile_Filename(DeeObject *__restrict self);

/* Read text from a file, a line or block at a time.
 * @param: readall: When true, keep trying to read data until `DeeFile_Read()'
 *                  actually returns ZERO(0), rather than stopping one it returns
 *                  something other than the then effective read buffer size.
 * @return: ITER_DONE: [DeeFile_ReadLine] The file has ended. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*Bytes*/ DeeObject *DCALL
DeeFile_ReadLine(DeeObject *__restrict self, size_t max_length, bool keep_lf);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Bytes*/ DeeObject *DCALL
DeeFile_ReadBytes(DeeObject *__restrict self, size_t max_length, bool readall);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Bytes*/ DeeObject *DCALL
DeeFile_PReadBytes(DeeObject *__restrict self, size_t max_length, Dee_pos_t pos, bool readall);


/* HINT: `DeeFile_Printf' is literally implemented as `DeeFormat_Printf(&DeeFile_WriteAll, self, format, ...)'
 * @return: -1: Error */
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t
DeeFile_Printf(DeeObject *__restrict self, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeFile_VPrintf(DeeObject *__restrict self, char const *__restrict format, va_list args);


/* Print text to a given file, implementing behavior of the print statement. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeFile_PrintNl(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeFile_PrintObject(DeeObject *self, DeeObject *ob);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeFile_PrintObjectSp(DeeObject *self, DeeObject *ob);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeFile_PrintObjectNl(DeeObject *self, DeeObject *ob);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeFile_PrintAll(DeeObject *self, DeeObject *ob);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeFile_PrintAllSp(DeeObject *self, DeeObject *ob);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeFile_PrintAllNl(DeeObject *self, DeeObject *ob);


/* Open a new system file.
 * @param: oflags: Set of `OPEN_F*'
 * @param: mode:   Permissions that should be applied when a new file is created. (Ignored when not supported by the host)
 * NOTE: If the host doesn't not support a filesystem or the requested open-mode
 *      (like that's ever going to happen... - deemon being used somewhere without
 *       a filesystem, that is), an `Error.SystemError.UnsupportedAPI' is thrown.
 * @return: * :        A new reference to the file in question.
 * @return: NULL:      An error (other than file-not-found) has occurred.
 * @return: ITER_DONE: The specified file does not exist.
 * @return: ITER_DONE: `OPEN_FCREAT' has not been given and file could not be found (no error was thrown)
 * @return: ITER_DONE: `OPEN_FEXCL' has been given and the file already exists (no error was thrown) */
DFUNDEF WUNUSED NONNULL((1)) DREF /*File*/ DeeObject *DCALL
DeeFile_Open(/*String*/ DeeObject *__restrict filename, int oflags, int mode);
DFUNDEF WUNUSED NONNULL((1)) DREF /*File*/ DeeObject *DCALL
DeeFile_OpenString(/*utf-8*/ char const *__restrict filename, int oflags, int mode);

#define Dee_OPEN_FRDONLY   0x00000000 /* Open only for reading. */
#define Dee_OPEN_FWRONLY   0x00000001 /* Open only for writing. */
#define Dee_OPEN_FRDWR     0x00000002 /* Open for reading + writing. */
#define Dee_OPEN_FACCMODE  0x00000003 /* Mask for read/write access. */
#define Dee_OPEN_FCREAT    0x00000040 /* Create the file if it was missing. */
#define Dee_OPEN_FEXCL     0x00000080 /* Used with `OPEN_FCREAT': Fail if file already exists. */
#define Dee_OPEN_FTRUNC    0x00000200 /* Truncate existing files. */
#define Dee_OPEN_FAPPEND   0x00000400 /* Append to the end of files. */
#define Dee_OPEN_FNONBLOCK 0x00000800 /* Don't block when attempting to read/write (Ignored if the host doesn't support this) */
#define Dee_OPEN_FSYNC     0x00001000 /* Write operations block until all data has been written to disk (Ignored if the host doesn't support this) */
#define Dee_OPEN_FDIRECT   0x00004000 /* Bypass system buffers and directly pass data to the kernel when writing (Ignored if the host doesn't support this) */
#define Dee_OPEN_FNOFOLLOW 0x00020000 /* Do not follow symbolic links (Ignored if the host doesn't support this) */
#define Dee_OPEN_FNOATIME  0x00040000 /* Do not update access times (Ignored if the host doesn't support this) */
#define Dee_OPEN_FCLOEXEC  0x00080000 /* Do not inherit the file in child processes (Ignored if the host doesn't support this) */
#define Dee_OPEN_FXREAD    0x10000000 /* Request exclusive read access (Ignored if the host doesn't support this) */
#define Dee_OPEN_FXWRITE   0x20000000 /* Request exclusive write access (Ignored if the host doesn't support this)
                                       * HINT: This flag is used for opening files for `deemon -F' to prevent
                                       *       other processes from modifying the file at the same time. */
#define Dee_OPEN_FHIDDEN   0x80000000 /* If the host's file system implements a hidden-file attribute, set it when creating a new file. */

#ifdef DEE_SOURCE
#define OPEN_FRDONLY   Dee_OPEN_FRDONLY
#define OPEN_FWRONLY   Dee_OPEN_FWRONLY
#define OPEN_FRDWR     Dee_OPEN_FRDWR
#define OPEN_FACCMODE  Dee_OPEN_FACCMODE
#define OPEN_FCREAT    Dee_OPEN_FCREAT
#define OPEN_FEXCL     Dee_OPEN_FEXCL
#define OPEN_FTRUNC    Dee_OPEN_FTRUNC
#define OPEN_FAPPEND   Dee_OPEN_FAPPEND
#define OPEN_FNONBLOCK Dee_OPEN_FNONBLOCK
#define OPEN_FSYNC     Dee_OPEN_FSYNC
#define OPEN_FDIRECT   Dee_OPEN_FDIRECT
#define OPEN_FNOFOLLOW Dee_OPEN_FNOFOLLOW
#define OPEN_FNOATIME  Dee_OPEN_FNOATIME
#define OPEN_FCLOEXEC  Dee_OPEN_FCLOEXEC
#define OPEN_FXREAD    Dee_OPEN_FXREAD
#define OPEN_FXWRITE   Dee_OPEN_FXWRITE
#define OPEN_FHIDDEN   Dee_OPEN_FHIDDEN
#endif /* DEE_SOURCE */



#define DEE_STDIN  0 /* Standard input */
#define DEE_STDOUT 1 /* Standard output */
#define DEE_STDERR 2 /* Standard error */

/* Return a file stream for a standard file number `id'.
 * @param: id:   One of `DEE_STD*' (Except `DEE_STDDBG')
 * @param: file: The file to use, or `NULL' to unbind that stream.
 * `DeeFile_GetStd()' will throw an `UnboundLocal' error if the stream isn't assigned. */
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeFile_GetStd(unsigned int id);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeFile_TryGetStd(unsigned int id);
/* Returns the old stream, `NULL' when none was assigned,
 * or `ITER_DONE' when it hadn't been allocated yet */
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeFile_SetStd(unsigned int id, DeeObject *file);

/* Reset all standard stream (called during the cleanup phase prior to shutdown)
 * @return: true:  A non-default file had been assigned to at
 *                 least one of the known standard streams.
 * @return: false: All streams had already been reset. */
DFUNDEF bool DCALL DeeFile_ResetStd(void);

/* Return the the default stream for a given STD number. */
DFUNDEF ATTR_RETNONNULL DeeObject *DCALL DeeFile_DefaultStd(unsigned int id);


/* DEE_STDDBG -- Same as `DEE_STDERR', but on windows, also print to `OutputDebugString()'
 * On other platforms, similar system APIs intended for printing debug strings may
 * be linked as well, but in all cases, text will always be printed to stderr, the
 * same way a non-redirected `DeeFile_DefaultStderr' would do. */
#ifdef CONFIG_HOST_WINDOWS
#define DEE_STDDBG_IS_UNIQUE 1
#define DEE_STDDBG 3
#else /* CONFIG_HOST_WINDOWS */
#define DEE_STDDBG_IS_STDERR 1
#define DEE_STDDBG DEE_STDERR
#endif /* !CONFIG_HOST_WINDOWS */

#ifdef __INTELLISENSE__
extern DREF DeeObject *const DeeFile_DefaultStdin;
extern DREF DeeObject *const DeeFile_DefaultStdout;
extern DREF DeeObject *const DeeFile_DefaultStderr;
extern DREF DeeObject *const DeeFile_DefaultStddbg;
#else /* __INTELLISENSE__ */
#define DeeFile_DefaultStdin   DeeFile_DefaultStd(DEE_STDIN)
#define DeeFile_DefaultStdout  DeeFile_DefaultStd(DEE_STDOUT)
#define DeeFile_DefaultStderr  DeeFile_DefaultStd(DEE_STDERR)
#define DeeFile_DefaultStddbg  DeeFile_DefaultStd(DEE_STDDBG)
#endif /* !__INTELLISENSE__ */



#ifdef CONFIG_BUILDING_DEEMON
/* Return the underlying file descriptor for `self', that must be
 * an instance of `DeeSystemFile_Type' (or one of its sub-classes)
 * WARNING: The caller is required not to pass objects nothing matching `DeeSystemFile_Check' */
INTDEF WUNUSED NONNULL((1)) DeeSysFD DCALL DeeSystemFile_Fileno(/*SystemFile*/ DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSystemFile_Filename(/*SystemFile*/ DeeObject *__restrict self);
#endif /* CONFIG_BUILDING_DEEMON */

/* Open a new filesystem file using the given system file-descriptor.
 * Depending on the underlying technology used, `filename' may be
 * ignored during this process, or used as a pre-defined cache.
 * NOTE: Even when the filename argument is ignored, passing
 *       a value other than `NULL' may still be interpreted
 *       as the descriptor's filename, meaning that in this
 *       case the given `filename' should be an absolute path.
 * HINT: On platforms that ignore the argument, the filename
 *       of an open file descriptor can be determined through
 *       some sort of trick/api.
 *         - readlink("/proc/self/fd/%d" % fd);
 *         - GetFinalPathNameByHandle(fd);
 *         - GetMappedFileName(MapViewOfFile(CreateFileMapping(fd)));
 * WARNING: Once this function succeeds, it is unwise to still be accessing
 *          the raw file descriptor using platform-specific APIs, in favor
 *          of deemon's own APIs, because deemon may be doing its own
 *          caching of data prior to said data being passed to the actual
 *          descriptor.
 *          Some very real example of where this may otherwise lead to
 *          problems is the sad fact that `CreateFileMapping()' doesn't
 *          work when used on empty files, in which case deemon will
 *          re-open the file for writing without sharing said write-access,
 *          before writing a single byte of data to the stream and creating
 *          the mapping again. Later it will then delete that one byte before
 *          closing the secondary descriptor, synchronizing the whole thing
 *          by keeping a lock to `fo_lock' and assuming that nothing else
 *          could potentially be writing to the file as the same time.
 *    HINT: If such caching cannot be done (such as when the passed
 *          descriptor refers to a TTY or some other interactive device),
 *          you may specify `OPEN_FAPPEND' or `OPEN_FDIRECT' in `oflags'
 *          to always suppress such caching being done, although any such
 *          cache implementation will automatically invoke `isatty()' or
 *          the equivalent in order to disable caching at runtime.
 * NOTE: When `inherit_fd' is false, the given `fd' will not be closed,
 *       when either `close()' is invoked, or when the file is destroyed.
 *       Otherwise, the function will inherit the given `fd' upon success.  */
DFUNDEF WUNUSED DREF /*File*/ DeeObject *DCALL
DeeFile_OpenFd(DeeSysFD fd, /*String*/ DeeObject *filename,
               int oflags, bool inherit_fd);


DECL_END

#endif /* !GUARD_DEEMON_FILE_H */
