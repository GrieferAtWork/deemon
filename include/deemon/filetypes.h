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
#ifndef GUARD_DEEMON_FILETYPES_H
#define GUARD_DEEMON_FILETYPES_H 1

#include "api.h"

#include "file.h"
#include "object.h"
#include "string.h"

#ifndef CONFIG_NO_THREADS
#include "util/lock.h"
#include "util/recursive-rwlock.h"
#endif /* !CONFIG_NO_THREADS */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_system_file_object         system_file_object
#define Dee_file_buffer_object         file_buffer_object
#define Dee_file_buffer_link           file_buffer_link
#define Dee_memory_file_object         memory_file_object
#define Dee_file_reader_object         file_reader_object
#define Dee_file_writer_object         file_writer_object
#define FILE_BUFFER_FNORMAL            Dee_FILE_BUFFER_FNORMAL
#define FILE_BUFFER_FREADONLY          Dee_FILE_BUFFER_FREADONLY
#define FILE_BUFFER_FNODYNSCALE        Dee_FILE_BUFFER_FNODYNSCALE
#define FILE_BUFFER_FLNBUF             Dee_FILE_BUFFER_FLNBUF
#define FILE_BUFFER_FSYNC              Dee_FILE_BUFFER_FSYNC
#define FILE_BUFFER_FCLOFILE           Dee_FILE_BUFFER_FCLOFILE
#define FILE_BUFFER_FREADING           Dee_FILE_BUFFER_FREADING
#define FILE_BUFFER_FNOTATTY           Dee_FILE_BUFFER_FNOTATTY
#define FILE_BUFFER_FISATTY            Dee_FILE_BUFFER_FISATTY
#define FILE_BUFFER_FSTATICBUF         Dee_FILE_BUFFER_FSTATICBUF
#define FILE_BUFFER_FLNIFTTY           Dee_FILE_BUFFER_FLNIFTTY
#define FILE_BUFSIZ_MAX                Dee_FILE_BUFSIZ_MAX
#define FILE_BUFSIZ_MIN                Dee_FILE_BUFSIZ_MIN
#define FILE_BUFSIZ_RELOCATE_THRESHOLD Dee_FILE_BUFSIZ_RELOCATE_THRESHOLD
#define FILE_BUFFER_MODE_NONE          Dee_FILE_BUFFER_MODE_NONE
#define FILE_BUFFER_MODE_FULL          Dee_FILE_BUFFER_MODE_FULL
#define FILE_BUFFER_MODE_LINE          Dee_FILE_BUFFER_MODE_LINE
#define FILE_BUFFER_MODE_AUTO          Dee_FILE_BUFFER_MODE_AUTO
#define FILE_BUFFER_MODE_KEEP          Dee_FILE_BUFFER_MODE_KEEP
#endif /* DEE_SOURCE */

typedef struct Dee_system_file_object DeeSystemFileObject;
typedef struct Dee_file_buffer_object DeeFileBufferObject;
typedef struct Dee_memory_file_object DeeMemoryFileObject;
typedef struct Dee_file_reader_object DeeFileReaderObject;
typedef struct Dee_file_writer_object DeeFileWriterObject;


struct Dee_system_file_object {
	Dee_FILE_OBJECT_HEAD
#ifdef DEESYSTEM_FILE_USE_WINDOWS
#define DEESYSTEM_FILE_HAVE_sf_filename
	DREF DeeObject  *sf_filename;   /* [0..1][lock(WRITE_ONCE)] The filename of this systemfile. */
	/*HANDLE*/ void *sf_handle;     /* [0..1][lock(CLEAR_ONCE)] Underlying file handle (or `INVALID_HANDLE_VALUE') */
	/*HANDLE*/ void *sf_ownhandle;  /* [0..1][lock(CLEAR_ONCE)] The owned file handle (or `INVALID_HANDLE_VALUE') */
	uint32_t         sf_filetype;   /* One of `FILE_TYPE_*' or `FILE_TYPE_UNKNOWN' when not loaded. */
	unsigned char    sf_pendingc;   /* Number of write-pending characters (for UTF-8 console output). */
	unsigned char    sf_pending[7]; /* Write-pending characters (for UTF-8 console output). */
#define DeeSystemFile_GetHandle(self) ((DeeSystemFileObject *)Dee_REQUIRES_OBJECT(self))->sf_handle
#endif /* DEESYSTEM_FILE_USE_WINDOWS */
#ifdef DEESYSTEM_FILE_USE_UNIX
#define DEESYSTEM_FILE_HAVE_sf_filename
	DREF DeeObject  *sf_filename;   /* [0..1][lock(WRITE_ONCE)] The filename, or NULL if not known. */
	int              sf_handle;     /* [0..1][lock(CLEAR_ONCE)] Underlying system file (or `-1') */
	int              sf_ownhandle;  /* [0..1][lock(CLEAR_ONCE)] The owned underlying system file (or `-1') */
#define DeeSystemFile_GetHandle(self) ((DeeSystemFileObject *)Dee_REQUIRES_OBJECT(self))->sf_handle
#endif /* DEESYSTEM_FILE_USE_UNIX */
#ifdef DEESYSTEM_FILE_USE_STDIO
#define DEESYSTEM_FILE_HAVE_sf_filename
	DREF DeeObject  *sf_filename;   /* [0..1][const] The filename, or NULL if not known. */
	/*FILE*/ void   *sf_handle;     /* [0..1][lock(CLEAR_ONCE)] Underlying system file (or `NULL') */
	/*FILE*/ void   *sf_ownhandle;  /* [0..1][lock(CLEAR_ONCE)] The owned underlying system file (or `NULL') */
#define DeeSystemFile_GetHandle(self) ((DeeSystemFileObject *)Dee_REQUIRES_OBJECT(self))->sf_handle
#endif /* DEESYSTEM_FILE_USE_STDIO */
};


struct Dee_file_buffer_link {
	DeeFileBufferObject **fbl_pself; /* [1..1, == self][0..1][lock(INTERN(:buffer_ttys_lock))] Self-pointer. */
	DeeFileBufferObject  *fbl_next;  /* [0..1][valid_if(fbl_pself != NULL)][lock(INTERN(:buffer_ttys_lock))] Next-pointer. */
};

struct Dee_file_buffer_object {
	Dee_FILE_OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	/* TODO: This lock right here should be shared (i.e.: preemptive) */
	Dee_recursive_rwlock_t      fb_lock;  /* Lock for synchronizing access to the buffer. */
#endif /* !CONFIG_NO_THREADS */
	DREF DeeObject             *fb_file;  /* [0..1][lock(fb_lock)] The file referenced by this buffer.
	                                       * NOTE: Set to `NULL' when the buffer is closed. */
	uint8_t                    *fb_ptr;   /* [>= fb_base][+fb_cnt <= fb_base+fb_size][lock(fb_lock)]
	                                       * Pointer to the next character to-be read/written.
	                                       * The absolute in-file position is then `fb_fblk+(fb_ptr-fb_base)' */
	size_t                      fb_cnt;   /* [lock(fb_lock)] The amount of unread, buffered bytes located at `fb_ptr'. */
	uint8_t                    *fb_chng;  /* [>= fb_base][+fb_chsz <= fb_base+fb_size]
	                                       * [valid_if(fb_chsz != 0)][lock(fb_lock)]
	                                       * Pointer to the first character that was
	                                       * changed since the buffer had been loaded. */
	size_t                      fb_chsz;  /* [lock(fb_lock)] Amount of bytes that were changed. */
	uint8_t                    *fb_base;  /* [0..fb_size][owned_if(!FILE_BUFFER_FSTATICBUF)][lock(fb_lock)] Allocated buffer.
	                                       * NOTE: This pointer must not be modified when `FILE_BUFFER_FREADING' is set. */
	size_t                      fb_size;  /* [lock(fb_lock)] Total allocated / available buffer size.
	                                       * NOTE: This pointer must not be modified when `FILE_BUFFER_FREADING' is set. */
	struct Dee_file_buffer_link fb_ttych; /* Chain of changed TTY file buffers (buffers that are used with an interactive file).
	                                       * Any buffer that is connected to an interactive device is flushed before
	                                       * data is read from any other interactive device.
	                                       * This chain is weakly linked in that buffer objects remove themself
	                                       * before destruction, also meaning that any buffer contained in this
	                                       * chain may have a reference counter to ZERO(0). */
	Dee_pos_t                   fb_fblk;  /* The starting address of the data block currently stored in `fb_base'. */
	Dee_pos_t                   fb_fpos;  /* The current (assumed) position within `fb_file'. */
	uint16_t                    fb_flag;  /* [lock(fb_lock)] The current state of the buffer (Set of `FILE_BUFFER_F*'). */
};

#define Dee_FILE_BUFFER_FNORMAL     0x0000 /* Normal buffer flags. */
#define Dee_FILE_BUFFER_FREADONLY   0x0001 /* The buffer can only be used for reading. */
#define Dee_FILE_BUFFER_FNODYNSCALE 0x0002 /* The buffer is not allowed to dynamically change its buffer size. */
#define Dee_FILE_BUFFER_FLNBUF      0x0004 /* The buffer is line-buffered, meaning that it will
                                            * flush its data whenever a line-feed is printed.
                                            * Additionally if the `FILE_BUFFER_FISATTY' flag is set,
                                            * attempting to read from a line-buffered file will cause
                                            * all other existing line-buffered files to be synchronized
                                            * first. This is done to ensure that interactive files are
                                            * always up-to-date before data is read from one of them. */
#define Dee_FILE_BUFFER_FSYNC       0x0008 /* Also synchronize the underlying file after flushing the buffer. */
#define Dee_FILE_BUFFER_FCLOFILE    0x0010 /* When the buffer is closed through use of `operator close',
                                            * also invoke `operator close' on the associated file.
                                            * However, when `close()' is never invoked on the buffer, its
                                            * destructor will _NOT_ invoke close on the underlying file. */
#define Dee_FILE_BUFFER_FREADING    0x0800 /* The buffer is currently being read into and must not be changed or resized. */
#define Dee_FILE_BUFFER_FNOTATTY    0x1000 /* This buffer does not refer to a TTY device. */
#define Dee_FILE_BUFFER_FISATTY     0x2000 /* This buffer refers to a TTY device. */
#define Dee_FILE_BUFFER_FSTATICBUF  0x4000 /* Must be used with `FILE_BUFFER_FNODYNSCALE': When set,
                                            * the buffer doesn't actually own its buffer and must not
                                            * attempt to free() it during destruction.
                                            * The `FILE_BUFFER_FNODYNSCALE' must be set to prevent the
                                            * buffer from attempting to resize (realloc) it dynamically. */
#define Dee_FILE_BUFFER_FLNIFTTY    0x8000 /* Automatically set/delete the `FILE_BUFFER_FLNBUF' and
                                            * `FILE_BUFFER_FISATTY' flags, and add/remove the file from
                                            * `fb_ttys' the next time this comes into question. To determine
                                            * this, the pointed-to file is tested for being a TTY device
                                            * using `DeeFile_IsAtty(fb_file)'.
                                            * HINT: This flag is set for all newly created buffers by default. */



/* Automatic scaling configuration when `FILE_BUFFER_FNODYNSCALE' is disabled. */
#define Dee_FILE_BUFSIZ_MAX                8192 /* The max size to which the buffer may grow. */
#define Dee_FILE_BUFSIZ_MIN                512  /* The default size when no dynamic buffer was allocated before. */
#define Dee_FILE_BUFSIZ_RELOCATE_THRESHOLD 2048 /* When >= this amount of bytes are unused in the buffer, shrink the buffer. */



/* File buffer mode flags. */
#define Dee_FILE_BUFFER_MODE_NONE (Dee_FILE_BUFFER_FNODYNSCALE) /* Do not perform any buffering (causes a zero-length buffer to be used internally)
                                                                 * NOTE: When set, `ZERO(0)' must be passed for `size' */
#define Dee_FILE_BUFFER_MODE_FULL (Dee_FILE_BUFFER_FNORMAL)     /* Do full buffering. (Data is only synced when the buffer becomes full, or when sync() is called) */
#define Dee_FILE_BUFFER_MODE_LINE (Dee_FILE_BUFFER_FLNBUF)      /* Do line-buffering. (Same as `FILE_BUFFER_MODE_FULL', but also flush whenever data was written that contained a line-feed) */
#define Dee_FILE_BUFFER_MODE_AUTO (Dee_FILE_BUFFER_FLNIFTTY)    /* Automatically determine the buffer mode based on calling `isatty()' on the
                                                                 * underlying file. When true, use line-buffering. Otherwise, use full buffering. */
#define Dee_FILE_BUFFER_MODE_KEEP (0xffff) /* Only accepted by `DeeFileBuffer_SetMode()': Keep on using the previous buffering configuration. */


/* Construct a new file-buffer.
 * @param: file: The file that is meant to be buffered.
 *               NOTE: If this is another file buffer, its pointed-to
 *                     file is unwound, so-long at it hasn't been closed.
 * @param: mode: One of `FILE_BUFFER_MODE_*', optionally or'd with
 *                      `FILE_BUFFER_FREADONLY', `FILE_BUFFER_FSYNC' and
 *                      `FILE_BUFFER_FCLOFILE'
 * @param: size: The size of the buffer, or ZERO(0) to allow it to change dynamically. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFileBuffer_New(DeeObject *__restrict file,
                  uint16_t mode, size_t size);

/* Change the operations mode of a given buffer.
 * @param: mode: One of `FILE_BUFFER_MODE_*', optionally or'd with `FILE_BUFFER_FSYNC'
 * @param: size: The size of the buffer, or ZERO(0) to allow it to change dynamically. */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeFileBuffer_SetMode(DeeObject *__restrict self,
                      uint16_t mode, size_t size);

/* Synchronize unwritten data of all interactive TTY devices.
 * NOTE: The first time a buffered TTY device is written to, this
 *       function is added to the `atexit()' chain unless deemon
 *       was built with the `CONFIG_NO_STDLIB' option enabled.
 * NOTE: This function can be called as `(File from deemon).buffer.sync()' */
DFUNDEF WUNUSED int DCALL DeeFileBuffer_SyncTTYs(void);



struct Dee_memory_file_object {
	Dee_FILE_OBJECT_HEAD
	char               *mf_begin; /* [0..1][<= mf_end][lock(mf_lock)] The effective start position. */
	char               *mf_ptr;   /* [0..1][>= mf_begin][lock(mf_lock)] The current string position (May be above `r_end', in which case no more data may be read) */
	char               *mf_end;   /* [0..1][>= mf_begin][lock(mf_lock)] The effective end position. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t mf_lock;  /* Lock for this memory-file object. */
#endif /* !CONFIG_NO_THREADS */
};

#define DeeMemoryFile_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->mf_lock)
#define DeeMemoryFile_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->mf_lock)
#define DeeMemoryFile_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->mf_lock)
#define DeeMemoryFile_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->mf_lock)
#define DeeMemoryFile_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->mf_lock)
#define DeeMemoryFile_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->mf_lock)
#define DeeMemoryFile_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->mf_lock)
#define DeeMemoryFile_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->mf_lock)
#define DeeMemoryFile_LockRead(self)       Dee_atomic_rwlock_read(&(self)->mf_lock)
#define DeeMemoryFile_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->mf_lock)
#define DeeMemoryFile_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->mf_lock)
#define DeeMemoryFile_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->mf_lock)
#define DeeMemoryFile_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->mf_lock)
#define DeeMemoryFile_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->mf_lock)
#define DeeMemoryFile_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->mf_lock)
#define DeeMemoryFile_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->mf_lock)

DDATDEF DeeFileTypeObject DeeMemoryFile_Type;


/* Open a read-only view for raw memory contained within the given data-block.
 * The returned file can be used to access said data in a read-only fashion,
 * however since the data isn't copied, before that data gets freed, you must
 * call `DeeFile_ReleaseMemory()' to inform the view of this happened, while
 * simultaneously decrementing its reference counter by ONE.
 * `DeeFile_ReleaseMemory()' will automatically determine the proper course
 * of action, dependent on whether the file is being shared with some other
 * part of deemon. If it is, it will replace the view's data with a heap-allocated
 * copy of that data, and if that isn't possible, modify the view to represent
 * an empty data set.
 * The main use of this functionality is to allow the use of `DeeModule_LoadSourceStream()'
 * with a stream backed by source code located in memory. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*File*/ DeeObject *DCALL
DeeFile_OpenRoMemory(void const *data, size_t data_size);
DFUNDEF NONNULL((1)) void DCALL
DeeFile_ReleaseMemory(DREF /*File*/ DeeObject *__restrict self);




struct Dee_file_reader_object {
	Dee_FILE_OBJECT_HEAD
	char               *r_begin;  /* [0..1][in(r_string->s_str)][<= r_end][lock(r_lock)] The effective start position within `r_string'. */
	char               *r_ptr;    /* [0..1][>= r_begin][lock(r_lock)] The current string position (May be above `r_end', in which case no more data may be read) */
	char               *r_end;    /* [0..1][in(r_string->s_str)][>= r_begin][lock(r_lock)] The effective end position within `r_string'. */
	DREF DeeObject     *r_owner;  /* [0..1][lock(r_lock)] The owner for the data. NOTE: Set to NULL when the file is closed. */
	DeeBuffer           r_buffer; /* [valid_if(r_owner)][lock(r_lock)] The data buffer view for `r_owner' (using `Dee_BUFFER_FREADONLY') */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t r_lock;   /* Lock for this file reader object. */
#endif /* !CONFIG_NO_THREADS */
};

#define DeeFileReader_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->r_lock)
#define DeeFileReader_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->r_lock)
#define DeeFileReader_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->r_lock)
#define DeeFileReader_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->r_lock)
#define DeeFileReader_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->r_lock)
#define DeeFileReader_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->r_lock)
#define DeeFileReader_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->r_lock)
#define DeeFileReader_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->r_lock)
#define DeeFileReader_LockRead(self)       Dee_atomic_rwlock_read(&(self)->r_lock)
#define DeeFileReader_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->r_lock)
#define DeeFileReader_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->r_lock)
#define DeeFileReader_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->r_lock)
#define DeeFileReader_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->r_lock)
#define DeeFileReader_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->r_lock)
#define DeeFileReader_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->r_lock)
#define DeeFileReader_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->r_lock)

DDATDEF DeeFileTypeObject DeeFileReader_Type; /* File.Reader */

/* Open a new file stream for reading memory from `data...+=data_size'
 * This stream assumes that data is immutable, and owned by `data_owner'.
 * The best example for a type that fits these requirements is `string'
 * This function greatly differs from `DeeFile_OpenRoMemory()', in that
 * the referenced data is shared with an explicit object, rather that
 * being held using a ticket-system, where the caller must manually
 * inform the memory stream when data is supposed to get released.
 * However, the end result of both mechanisms is the same, in that
 * the stream indirectly referenced a given data-block, rather than
 * having to keep its own copy of some potentially humongous memory block. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF /*File*/ DeeObject *DCALL
DeeFile_OpenObjectMemory(DeeObject *__restrict data_owner,
                         void const *data, size_t data_size);

/* Similar to `DeeFile_OpenObjectMemory()', but used
 * to open a generic object using the buffer-interface. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*File*/ DeeObject *DCALL
DeeFile_OpenObjectBuffer(DeeObject *__restrict data,
                         size_t begin, size_t end);


struct Dee_file_writer_object {
	Dee_FILE_OBJECT_HEAD
	struct Dee_unicode_printer w_printer; /* [lock(w_lock)][owned_if(!w_string)] The printer used to generate the string. */
	DREF DeeStringObject      *w_string;  /* [lock(w_lock)][0..1][valid_if((w_printer.up_flags & UNICODE_PRINTER_FWIDTH) != STRING_WIDTH_1BYTE)]
	                                       * Cached variant of the last-accessed, generated multi-byte string. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t        w_lock;    /* Lock for this file writer object. */
#endif /* !CONFIG_NO_THREADS */
};

#define DeeFileWriter_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->w_lock)
#define DeeFileWriter_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->w_lock)
#define DeeFileWriter_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->w_lock)
#define DeeFileWriter_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->w_lock)
#define DeeFileWriter_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->w_lock)
#define DeeFileWriter_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->w_lock)
#define DeeFileWriter_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->w_lock)
#define DeeFileWriter_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->w_lock)
#define DeeFileWriter_LockRead(self)       Dee_atomic_rwlock_read(&(self)->w_lock)
#define DeeFileWriter_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->w_lock)
#define DeeFileWriter_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->w_lock)
#define DeeFileWriter_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->w_lock)
#define DeeFileWriter_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->w_lock)
#define DeeFileWriter_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->w_lock)
#define DeeFileWriter_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->w_lock)
#define DeeFileWriter_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->w_lock)

DDATDEF DeeFileTypeObject DeeFileWriter_Type; /* File.Writer */

/* Open a new file stream that writes all written data into a string. */
DFUNDEF WUNUSED DREF /*File*/ DeeObject *DCALL DeeFile_OpenWriter(void);

/* Returns the current string written by the writer. */
DFUNDEF WUNUSED NONNULL((1)) DREF /*string*/ DeeObject *DCALL
DeeFileWriter_GetString(DeeObject *__restrict self);


DECL_END

#endif /* !GUARD_DEEMON_FILETYPES_H */
