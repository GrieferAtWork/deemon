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
#ifndef GUARD_DEEMON_OBJECTS_FILEBUFFER_C
#define GUARD_DEEMON_OBJECTS_FILEBUFFER_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* atexit(), memcpy(), ... */
#include <deemon/util/atomic.h>

#include <hybrid/sequence/list.h>
#include <hybrid/typecore.h>

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#ifdef DEESYSTEM_FILE_USE_STDIO
#include <stdio.h>
#endif /* DEESYSTEM_FILE_USE_STDIO */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

LIST_HEAD(file_buffer_object_list, file_buffer_object);
typedef DeeFileBufferObject Buffer;

/* [0..1][lock(buffer_ttys_lock)] Chain of tty-buffers. */
PRIVATE struct file_buffer_object_list buffer_ttys = LIST_HEAD_INITIALIZER(buffer_ttys);
#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_lock_t buffer_ttys_lock = DEE_ATOMIC_LOCK_INIT;
#endif /* CONFIG_NO_THREADS */
#define buffer_ttys_lock_available()  Dee_atomic_lock_available(&buffer_ttys_lock)
#define buffer_ttys_lock_acquired()   Dee_atomic_lock_acquired(&buffer_ttys_lock)
#define buffer_ttys_lock_tryacquire() Dee_atomic_lock_tryacquire(&buffer_ttys_lock)
#define buffer_ttys_lock_acquire()    Dee_atomic_lock_acquire(&buffer_ttys_lock)
#define buffer_ttys_lock_waitfor()    Dee_atomic_lock_waitfor(&buffer_ttys_lock)
#define buffer_ttys_lock_release()    Dee_atomic_lock_release(&buffer_ttys_lock)


#ifdef CONFIG_HAVE_atexit
PRIVATE bool atexit_registered = false;
#endif /* CONFIG_HAVE_atexit */

/* Add/Remove a given buffer from the TTY-buffer chain. */
PRIVATE NONNULL((1)) void DCALL buffer_addtty(Buffer *__restrict self);
PRIVATE NONNULL((1)) void DCALL buffer_deltty(Buffer *__restrict self);


PRIVATE ATTR_COLD int DCALL err_buffer_closed(void);

/* Special return values for `buffer_read_or_unlock' and `buffer_write_or_unlock' */
#define BUFFER_IO_EOF        (0)  /* End-of-file */
#define BUFFER_IO_ERROR      (-1) /* Error (an exception was thrown) */
#define BUFFER_IO_DID_UNLOCK (-2) /* The buffer had to be unlocked (try again) */
#define GETC_DID_UNLOCK      (-3) /* The buffer had to be unlocked (try again) (for `buffer_getc_or_unlock') */
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL buffer_read_or_unlock(Buffer *__restrict self, byte_t *__restrict buffer, size_t bufsize, dioflag_t flags);
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL buffer_write_or_unlock(Buffer *__restrict self, byte_t const *__restrict buffer, size_t bufsize, dioflag_t flags);
PRIVATE WUNUSED NONNULL((1)) Dee_pos_t DCALL buffer_seek_or_unlock(Buffer *__restrict self, Dee_off_t off, int whence);
PRIVATE WUNUSED NONNULL((1)) int DCALL buffer_getc_or_unlock(Buffer *__restrict self, dioflag_t flags);

PRIVATE WUNUSED NONNULL((1)) int DCALL buffer_ungetc_nolock(Buffer *__restrict self, int ch);
PRIVATE WUNUSED NONNULL((1)) int DCALL buffer_trunc_nolock(Buffer *__restrict self, Dee_pos_t new_size);
PRIVATE WUNUSED NONNULL((1)) int DCALL buffer_sync_nolock(Buffer *__restrict self, uint16_t mode);
#define BUFFER_SYNC_FNORMAL          0x0000
#define BUFFER_SYNC_FERROR_IF_CLOSED 0x0001 /* Throw an error if the buffer was closed. */
#define BUFFER_SYNC_FNOSYNC_FILE     0x0002 /* Don't synchronize the underlying file, regardless of whether
                                             * or not the `FILE_BUFFER_FSYNC' flag has been set. */


#ifdef CONFIG_HAVE_atexit
PRIVATE void atexit_flushall(void) {
	for (;;) {
		DREF Buffer *buffer;
		int error;
		/* XXX: Considering how this function is called during exit(),
		 *      can we actually be certain that no other thread was
		 *      terminated while still holding this lock?
		 *      What does STD-C say about the time of execution of
		 *      atexit() functions in relation to other threads and
		 *      their potential termination?
		 *      I'm guessing it's OK as this'll probably be one of
		 *      the first things to get executed during termination,
		 *      but can we be certain that it will? */
		buffer_ttys_lock_acquire();
		LIST_FOREACH (buffer, &buffer_ttys, fb_ttych) {
			if (Dee_IncrefIfNotZero(buffer))
				break;
		}
		if (buffer) {
			ASSERT(LIST_ISBOUND(buffer, fb_ttych));
			LIST_UNBIND(buffer, fb_ttych);
		}
		buffer_ttys_lock_release();
		if (!buffer)
			break;

		/* Synchronize this buffer. */
		DeeFileBuffer_LockWriteNoInt(buffer);
		error = buffer_sync_nolock(buffer, BUFFER_SYNC_FNORMAL);
		DeeFileBuffer_LockEndWrite(buffer);
		if unlikely(error) {
			DeeError_Print("Failed to synchronize tty during exit\n",
			               ERROR_PRINT_HANDLEINTR);
		}
		Dee_Decref(buffer);
	}
}
#endif /* CONFIG_HAVE_atexit */

PRIVATE NONNULL((1)) void DCALL
buffer_addtty(Buffer *__restrict self) {
	buffer_ttys_lock_acquire();
#ifdef CONFIG_HAVE_atexit
	if (!atexit_registered) {
		/* NOTE: If atexit() fails, there's nothing we could do about it,
		 *       so we won't even bother to handle any errors it may return. */
		atexit(&atexit_flushall);
		atexit_registered = true;
	}
#endif /* CONFIG_HAVE_atexit */
	if (!LIST_ISBOUND(self, fb_ttych))
		LIST_INSERT_HEAD(&buffer_ttys, self, fb_ttych);
	buffer_ttys_lock_release();
}

PRIVATE NONNULL((1)) void DCALL
buffer_deltty(Buffer *__restrict self) {
	buffer_ttys_lock_acquire();
	if (LIST_ISBOUND(self, fb_ttych))
		LIST_UNBIND(self, fb_ttych);
	buffer_ttys_lock_release();
}


LOCAL WUNUSED NONNULL((1, 2)) int DCALL
buffer_init(Buffer *__restrict self,
            DeeObject *__restrict file,
            uint16_t mode, size_t size) {
	ASSERT_OBJECT(file);
	/* Validate that the given `mode' is an accepted buffer mode. */
	ASSERT((mode & ~(FILE_BUFFER_FREADONLY | FILE_BUFFER_FSYNC | FILE_BUFFER_FCLOFILE)) == FILE_BUFFER_MODE_NONE ||
	       (mode & ~(FILE_BUFFER_FREADONLY | FILE_BUFFER_FSYNC | FILE_BUFFER_FCLOFILE)) == FILE_BUFFER_MODE_FULL ||
	       (mode & ~(FILE_BUFFER_FREADONLY | FILE_BUFFER_FSYNC | FILE_BUFFER_FCLOFILE)) == FILE_BUFFER_MODE_LINE ||
	       (mode & ~(FILE_BUFFER_FREADONLY | FILE_BUFFER_FSYNC | FILE_BUFFER_FCLOFILE)) == FILE_BUFFER_MODE_AUTO);

	if ((self->fb_size = size) != 0) {
		/* Allocate the initial buffer. */
		self->fb_base = (byte_t *)Dee_Malloc(size);
		if unlikely(!self->fb_base)
			goto err;
	} else {
		self->fb_base = NULL;
	}

#if 1 /* Unwind recursive buffers. */
	if (DeeObject_InstanceOf(file, (DeeTypeObject *)&DeeFileBuffer_Type)) {
		DeeObject *base_file;
		if (DeeFileBuffer_LockWrite((Buffer *)file)) {
			Dee_Free(self->fb_base);
			goto err;
		}
		base_file = ((Buffer *)file)->fb_file;
		Dee_XIncref(base_file);
		DeeFileBuffer_LockEndWrite((Buffer *)file);
		/* Use the original buffer-file if available. */
		if (base_file)
			file = base_file;
	} else
#endif
	{
		Dee_Incref(file);
	}
	Dee_rshared_rwlock_init(&self->fb_lock);
	self->fb_file = file;
	self->fb_ptr  = self->fb_base;
	self->fb_cnt  = 0;
	self->fb_chng = self->fb_base;
	self->fb_chsz = 0;
	LIST_ENTRY_UNBOUND_INIT(&self->fb_ttych);
	self->fb_fblk = 0;
	self->fb_fpos = 0;
	self->fb_flag = mode;
	return 0;
err:
	return -1;
}

#ifdef NDEBUG
PRIVATE WUNUSED NONNULL((1)) byte_t *DCALL
buffer_realloc_nolock(Buffer *__restrict self, size_t new_size) {
	if (self->fb_flag & FILE_BUFFER_FSTATICBUF)
		return (byte_t *)Dee_Malloc(new_size);
	ASSERT(!(self->fb_flag & FILE_BUFFER_FREADING));
	return (byte_t *)Dee_Realloc(self->fb_base, new_size);
}

PRIVATE WUNUSED NONNULL((1)) byte_t *DCALL
buffer_tryrealloc_nolock(Buffer *__restrict self, size_t new_size) {
	if (self->fb_flag & FILE_BUFFER_FSTATICBUF)
		return (byte_t *)Dee_TryMalloc(new_size);
	ASSERT(!(self->fb_flag & FILE_BUFFER_FREADING));
	return (byte_t *)Dee_TryRealloc(self->fb_base, new_size);
}
#else /* NDEBUG */
#define buffer_realloc_nolock(self, new_size) \
	buffer_realloc_nolock_d(self, new_size, __FILE__, __LINE__)
PRIVATE WUNUSED NONNULL((1)) byte_t *DCALL
buffer_realloc_nolock_d(Buffer *__restrict self, size_t new_size,
                        char const *file, int line) {
	if (self->fb_flag & FILE_BUFFER_FSTATICBUF)
		return (byte_t *)DeeDbg_Malloc(new_size, file, line);
	ASSERT(!(self->fb_flag & FILE_BUFFER_FREADING));
	return (byte_t *)DeeDbg_Realloc(self->fb_base, new_size, file, line);
}
#define buffer_tryrealloc_nolock(self, new_size) \
	buffer_tryrealloc_nolock_d(self, new_size, __FILE__, __LINE__)
PRIVATE WUNUSED NONNULL((1)) byte_t *DCALL
buffer_tryrealloc_nolock_d(Buffer *__restrict self, size_t new_size,
                           char const *file, int line) {
	if (self->fb_flag & FILE_BUFFER_FSTATICBUF)
		return (byte_t *)DeeDbg_TryMalloc(new_size, file, line);
	ASSERT(!(self->fb_flag & FILE_BUFFER_FREADING));
	return (byte_t *)DeeDbg_TryRealloc(self->fb_base, new_size, file, line);
}
#endif /* !NDEBUG */



PRIVATE WUNUSED NONNULL((1)) int DCALL
buffer_determine_isatty(Buffer *__restrict self) {
	int is_a_tty;
	DREF DeeObject *file;
	uint16_t flags = self->fb_flag;
	if (flags & (FILE_BUFFER_FNOTATTY |
	             FILE_BUFFER_FISATTY)) {
set_lfflag:
		if (flags & FILE_BUFFER_FLNIFTTY) {
			self->fb_flag &= ~FILE_BUFFER_FLNIFTTY;
			/* Set the line-buffered flag if it is a TTY. */
			if (flags & FILE_BUFFER_FISATTY)
				self->fb_flag |= FILE_BUFFER_FLNBUF;
		}
		return 0;
	}
	if unlikely(!self->fb_file)
		goto err_closed;
	file = self->fb_file, Dee_Incref(file);
	is_a_tty = DeeFile_IsAtty(file);
	Dee_Decref(file);
	if unlikely(is_a_tty < 0)
		goto err;
	self->fb_flag |= is_a_tty ? FILE_BUFFER_FISATTY : FILE_BUFFER_FNOTATTY;
	flags = self->fb_flag;
	goto set_lfflag;
err_closed:
	err_buffer_closed();
err:
	return -1;
}

/* Construct a new file-buffer.
 * @param: file: The file that is meant to be buffered.
 *               NOTE: If this is another file buffer, its pointed-to
 *                     file is unwound, so-long at it hasn't been closed.
 * @param: mode: One of `FILE_BUFFER_MODE_*', optionally or'd with
 *                      `FILE_BUFFER_FREADONLY', `FILE_BUFFER_FSYNC' and
 *                      `FILE_BUFFER_FCLOFILE'
 * @param: size: The size of the buffer, or ZERO(0) to allow it to change dynamically. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFileBuffer_New(DeeObject *__restrict file,
                  uint16_t mode, size_t size) {
	DREF Buffer *result;
	result = DeeObject_MALLOC(Buffer);
	if unlikely(!result)
		goto done;
	/* Initialize the buffer. */
	if unlikely(buffer_init(result, file, mode, size))
		goto err_r;
	DeeFileObject_Init(result, &DeeFileBuffer_Type);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

/* Change the operations mode of a given buffer.
 * @param: mode: One of `FILE_BUFFER_MODE_*', optionally or'd with `FILE_BUFFER_FSYNC'
 * @param: size: The size of the buffer, or ZERO(0) to allow it to change dynamically. */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeFileBuffer_SetMode(DeeObject *__restrict self,
                      uint16_t mode, size_t size) {
	int result = 0;
	byte_t *new_buffer;
	Buffer *me = (Buffer *)self;
	ASSERT((mode & ~(FILE_BUFFER_FSYNC)) == FILE_BUFFER_MODE_NONE ||
	       (mode & ~(FILE_BUFFER_FSYNC)) == FILE_BUFFER_MODE_FULL ||
	       (mode & ~(FILE_BUFFER_FSYNC)) == FILE_BUFFER_MODE_LINE ||
	       (mode & ~(FILE_BUFFER_FSYNC)) == FILE_BUFFER_MODE_AUTO ||
	       mode == FILE_BUFFER_MODE_KEEP);
	ASSERT_OBJECT_TYPE(self, (DeeTypeObject *)&DeeFileBuffer_Type);
	if (DeeFileBuffer_LockWrite(me))
		return -1;
	result = buffer_sync_nolock(me, BUFFER_SYNC_FERROR_IF_CLOSED);
	if unlikely(result)
		goto done;

	/* Set the new buffering mode. */
	if (mode != FILE_BUFFER_MODE_KEEP) {
		me->fb_flag &= ~(FILE_BUFFER_FNODYNSCALE |
		                 FILE_BUFFER_FLNBUF |
		                 FILE_BUFFER_FLNIFTTY);
		me->fb_flag |= mode;
	}
	me->fb_chsz = 0;
	if (!size) {
		if (!(me->fb_flag & FILE_BUFFER_FNODYNSCALE)) {
			if (me->fb_flag & FILE_BUFFER_FREADING)
				goto err_cannot_resize;
			/* Resize-to-zero. */
			if (!(me->fb_flag & FILE_BUFFER_FSTATICBUF))
				Dee_Free(me->fb_base);
			me->fb_ptr  = (byte_t *)NULL + (me->fb_ptr - me->fb_base);
			me->fb_cnt  = 0;
			me->fb_size = 0;
			me->fb_base = NULL;
		} else {
			/* Dynamically scaled buffer. */
		}
	} else if (size != me->fb_size) {
		size_t bufoff;
		if (me->fb_flag & FILE_BUFFER_FREADING)
			goto err_cannot_resize;
		new_buffer = buffer_realloc_nolock(me, size);
		if unlikely(!new_buffer)
			goto err;
		/* Figure out where pointers are located at and update them. */
		bufoff = (size_t)(me->fb_ptr - me->fb_base);
		if (bufoff >= size) {
			/* Clear the available-buffer now that
			 * it has been truncated to ZERO(0). */
			me->fb_cnt = 0;
		} else {
			size_t minsiz = (size_t)((me->fb_base + size) - me->fb_ptr);
			/* Truncate the available-buffer. */
			if (me->fb_cnt > minsiz)
				me->fb_cnt = minsiz;
		}
		/* Relocate pointers. */
		me->fb_ptr  = new_buffer + (me->fb_ptr - me->fb_base);
		me->fb_base = new_buffer;
		me->fb_size = size;
	}
	me->fb_chng = me->fb_base;
done:
	DeeFileBuffer_LockEndWrite(me);
	return result;
err_cannot_resize:
err:
	result = -1;
	goto done;
}


/* Synchronize unwritten data of all interactive TTY devices.
 * NOTE: The first time a buffered TTY device is written to,
 *       this function is registered with `Dee_AtExit()'.
 * NOTE: This function can be called as `(File from deemon).buffer.sync()'
 * @return: 1 : `or_unlock_me' was non-NULL, and had to be unlocked
 * @return: 0 : Success
 * @return: -1: Error */
PUBLIC WUNUSED int DCALL
DeeFileBuffer_SyncTTYs(DeeFileBufferObject *or_unlock_me) {
	int result = 0;
	for (;;) {
		DREF Buffer *buffer;
		buffer_ttys_lock_acquire();
		LIST_FOREACH (buffer, &buffer_ttys, fb_ttych) {
			if (Dee_IncrefIfNotZero(buffer))
				break;
		}
		buffer_ttys_lock_release();
		if (!buffer)
			break;
		/* Synchronize this buffer. */

		/* Important check: if there's nothing to sync, don't even try to lock the file!
		 * This is required to prevent a soft-lock when many threads are locking & un-
		 * locking the same set of buffers all at once, using the `BUFFER_IO_DID_UNLOCK'
		 * path to prevent dead-locks. In this case, we want to prevent any sort of file
		 * locking in the case where we don't actually have to sync anything. */
		if (atomic_read(&buffer->fb_chsz) != 0) {
			/* Special handling is needed here because:
			 * - A normal write-lock would dead-lock with `buffer_write()', because we're
			 *   trying to acquire the same object-semantic lock twice here. If 2 threads
			 *   try to do this at the same time, 2 buffers locks will already be held,
			 *   and neither thread will be able to acquire the other thread's lock here! */
			if (!DeeFileBuffer_LockTryWrite(buffer)) {
				if (or_unlock_me) {
					/* Unlock the caller-given `or_unlock_me' */
					DeeFileBuffer_LockEndWrite(or_unlock_me);
					Dee_Decref(buffer);
	
					/* While not holding any locks, force a sync of all TTYs */
					result = DeeFileBuffer_SyncTTYs(NULL);
					if (result == 0) {
						result = 1; /* Tell the caller that we lost their lock... */
					} else {
						ASSERT(result == -1);
						/* XXX: Tell the caller that the lock was lost, *and* an error happened. */
						DeeFileBuffer_LockWriteNoInt(or_unlock_me);
					}
					return result;
				} else {
					if (DeeFileBuffer_LockWrite(buffer)) {
						Dee_Decref(buffer);
						return -1;
					}
				}
			}
			result = buffer_sync_nolock(buffer, BUFFER_SYNC_FNORMAL);
			DeeFileBuffer_LockEndWrite(buffer);
		}

		/* Drop the buffer and check for error */
		Dee_Decref(buffer);
		if unlikely(result != 0)
			break;
	}
	return result;
}


PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
buffer_read_or_unlock(Buffer *__restrict self,
                      byte_t *__restrict buffer,
                      size_t bufsize, dioflag_t flags) {
	size_t read_size, result = 0;
	size_t bufavail;
	uint16_t old_flags;
	Dee_pos_t next_data;
	byte_t *new_buffer;
	DREF DeeObject *file;
	bool did_read_data = false;
	if unlikely(!self->fb_file)
		goto err_closed;
again:
	bufavail = self->fb_cnt;
	if likely(bufavail) {
read_from_buffer:
		if (bufavail > bufsize)
			bufavail = bufsize;
		buffer = (byte_t *)mempcpy(buffer, self->fb_ptr, bufavail);
		/* Update buffer pointers. */
		self->fb_cnt -= bufavail;
		self->fb_ptr += bufavail;
		result += bufavail;
		bufsize -= bufavail;
		if (!bufsize)
			goto done;
		/* Make sure that we only invoke read() on the underlying
		 * file once, so-as to ensure that we don't start blocking
		 * because the underlying file is pipe-like and now empty,
		 * before the caller got a chance to process the data that
		 * we _did_ manage to extract! */
		if (did_read_data)
			goto done;
	}

	/* The buffer is empty and must be re-filled. */
	/* First off: Flush any changes that had been made. */
	if (buffer_sync_nolock(self, BUFFER_SYNC_FERROR_IF_CLOSED))
		goto err;
	if (buffer_determine_isatty(self))
		goto err;
	self->fb_chng = self->fb_base;
	self->fb_chsz = 0;

	/* Unlikely: But can happen due to recursive callbacks. */
	if unlikely(self->fb_cnt)
		goto read_from_buffer;
	if unlikely(!self->fb_file)
		goto err_closed;

	/* If we're a TTY buffer, flush all other TTY buffers before reading. */
	if (self->fb_flag & FILE_BUFFER_FISATTY) {
		int error;
		error = DeeFileBuffer_SyncTTYs(self);
		if unlikely(error != 0) {
			if (error < 0)
				goto err;
			goto did_unlock;
		}
		if unlikely(self->fb_cnt)
			goto read_from_buffer;
		if unlikely(!self->fb_file)
			goto err_closed;
		self->fb_chng = self->fb_base;
		self->fb_chsz = 0;
	}

	/* Determine where the next block of data is. */
	next_data = self->fb_fblk + (self->fb_ptr - self->fb_base);

	/* If no buffer had been allocated, allocate one now. */
	if unlikely(!self->fb_size) {
		size_t initial_bufsize;
		ASSERT(self->fb_ptr == self->fb_base);
		if unlikely(self->fb_flag & FILE_BUFFER_FNODYNSCALE) {
			/* Dynamic scaling is disabled. Must forward the getc() to the underlying file. */
read_through:
			self->fb_ptr = self->fb_base;
			file         = self->fb_file;
			Dee_Incref(file);
			if (next_data != self->fb_fpos) {
				/* Seek in the underlying file to get where we need to go. */
				Dee_pos_t new_pos;
				new_pos = DeeFile_SetPos(file, next_data);
				Dee_Decref(file);
				if unlikely(new_pos == (Dee_pos_t)-1)
					goto err;
				self->fb_fpos = next_data;
				goto again; /* Must start over because of recursive callbacks. */
			}
			read_size = DeeFile_Readf(file, buffer, bufsize, flags);
			Dee_Decref(file);
			if unlikely(read_size == (size_t)-1)
				goto err;
			next_data += read_size;
			self->fb_fpos = next_data;
			self->fb_fblk = next_data;
			result += read_size;
			goto done;
		}
		if (bufsize >= FILE_BUFSIZ_MAX)
			goto read_through;
		initial_bufsize = bufsize * 2;
		if (initial_bufsize > FILE_BUFSIZ_MAX)
			initial_bufsize = FILE_BUFSIZ_MAX;
		if (initial_bufsize < FILE_BUFSIZ_MIN)
			initial_bufsize = FILE_BUFSIZ_MIN;
		new_buffer = buffer_tryrealloc_nolock(self, initial_bufsize);
		if unlikely(!new_buffer) {
			if (Dee_CollectMemory(1))
				goto again;
			goto err;
		}
		self->fb_base = new_buffer;
		self->fb_size = initial_bufsize;
	} else if (bufsize >= self->fb_size) {
		size_t new_bufsize;
		/* The caller wants at least as much as this buffer could even handle.
		 * Upscale the buffer, or use load data using read-through mode. */
		if (self->fb_flag & (FILE_BUFFER_FNODYNSCALE | FILE_BUFFER_FREADING))
			goto read_through;
		if (bufsize > FILE_BUFSIZ_MAX)
			goto read_through;
		new_bufsize = bufsize * 2;
		if (new_bufsize > FILE_BUFSIZ_MAX)
			new_bufsize = FILE_BUFSIZ_MAX;

		/* Upscale the buffer. */
		new_buffer = buffer_tryrealloc_nolock(self, new_bufsize);

		/* If the allocation failed, also use read-through mode. */
		if unlikely(!new_buffer)
			goto read_through;
		self->fb_base = new_buffer;
		self->fb_size = new_bufsize;
	}

	self->fb_ptr  = self->fb_base;
	self->fb_chng = self->fb_base;
	self->fb_fblk = next_data;
	ASSERT(self->fb_cnt == 0);
	ASSERT(self->fb_chsz == 0);
	ASSERT(self->fb_size != 0);
	ASSERT(self->fb_file);
	file = self->fb_file;
	Dee_Incref(file);
	if (next_data != self->fb_fpos) {
		/* Seek in the underlying file to get where we need to go. */
		Dee_pos_t new_pos;
		new_pos = DeeFile_SetPos(file, next_data);
		Dee_Decref(file);
		if unlikely(new_pos == (Dee_pos_t)-1)
			goto err;
		self->fb_fpos = next_data;
		goto again; /* Must start over because of recursive callbacks. */
	}

	/* Actually read the data. */
	new_buffer = self->fb_base;
	old_flags  = self->fb_flag;
	self->fb_flag |= FILE_BUFFER_FREADING;
	read_size = DeeFile_Readf(file, self->fb_base, self->fb_size, flags);
	Dee_Decref(file);
	self->fb_flag &= ~FILE_BUFFER_FREADING;
	self->fb_flag |= old_flags & FILE_BUFFER_FREADING;
	if unlikely(read_size == (size_t)-1)
		goto err;
	if unlikely(read_size == 0)
		goto done;
	self->fb_fpos = next_data + read_size;
	self->fb_ptr  = self->fb_base;
	self->fb_cnt  = read_size;
	did_read_data = true;
	goto again;

done:
	return result;
err_closed:
	err_buffer_closed();
err:
	return (size_t)BUFFER_IO_ERROR;
did_unlock:
	return (size_t)BUFFER_IO_DID_UNLOCK;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
buffer_write_or_unlock(Buffer *__restrict self,
                       byte_t const *__restrict buffer,
                       size_t bufsize, dioflag_t flags) {
	size_t result = 0;
	size_t new_bufsize;
	size_t bufavail;
	byte_t *new_buffer;
	DREF DeeObject *file;
	if (buffer_determine_isatty(self))
		goto err;
	if (self->fb_flag & FILE_BUFFER_FREADONLY) {
		DeeError_Throwf(&DeeError_NotImplemented,
		                "The buffer hasn't been opened for writing");
		goto err;
	}
again_checkfile:
	if unlikely(!self->fb_file)
		goto err_closed;
again:
	/* Fill available buffer. */
	bufavail = (self->fb_base + self->fb_size) - self->fb_ptr;
	if likely(bufavail) {
		if (bufavail > bufsize)
			bufavail = bufsize;
		if unlikely(!bufavail)
			goto done;
		memcpy(self->fb_ptr, buffer, bufavail);
		result += bufavail;

		/* Update the changed file-area. */
		if (!self->fb_chsz) {
			self->fb_chng = self->fb_ptr;
			self->fb_chsz = bufavail;
		} else {
			if (self->fb_chng > self->fb_ptr) {
				self->fb_chsz += (size_t)(self->fb_chng - self->fb_ptr);
				self->fb_chng = self->fb_ptr;
			}
			if (self->fb_chng + self->fb_chsz < self->fb_ptr + bufavail)
				self->fb_chsz = (size_t)((self->fb_ptr + bufavail) - self->fb_chng);
		}

		/* If this is a TTY device, add it to the chain of changed ones. */
		if (self->fb_flag & FILE_BUFFER_FISATTY)
			buffer_addtty(self);

		/* Update the file pointer. */
		self->fb_ptr += bufavail;
		if (self->fb_cnt >= bufavail) {
			self->fb_cnt -= bufavail;
		} else {
			self->fb_cnt = 0;
		}

		/* When operating in line-buffered mode, check
		 * if there was a linefeed in what we just wrote. */
		if ((self->fb_flag & FILE_BUFFER_FLNBUF) &&
		    (memchr(buffer, '\n', bufsize) ||
		     memchr(buffer, '\r', bufsize))) {
			if (self->fb_flag & FILE_BUFFER_FISATTY) {
				int error = DeeFileBuffer_SyncTTYs(self);
				if unlikely(error) {
					if unlikely(error < 0)
						goto err;
					goto did_unlock;
				}
			}

			/* Flush this file. */
			if (buffer_sync_nolock(self, BUFFER_SYNC_FERROR_IF_CLOSED))
				goto err;
			bufsize -= bufavail;
			if (!bufsize)
				goto done;
			buffer += bufavail;
			goto again_checkfile;
		}

		bufsize -= bufavail;
		if (!bufsize)
			goto done;
		buffer += bufavail;
	}

	/* No more buffer available.
	 * Either we must flush the buffer, or we must extend it. */
	if (self->fb_size >= FILE_BUFSIZ_MAX ||
	    (self->fb_flag & (FILE_BUFFER_FNODYNSCALE |
	                      FILE_BUFFER_FREADING))) {
		/* Buffer is too large or cannot be relocated.
		 * >> Therefor, we must flush it, then try again. */
		if (self->fb_flag & FILE_BUFFER_FISATTY) {
			int error = DeeFileBuffer_SyncTTYs(self);
			if unlikely(error) {
				if unlikely(error < 0)
					goto err;
				goto did_unlock;
			}
		}
		if (buffer_sync_nolock(self, BUFFER_SYNC_FERROR_IF_CLOSED))
			goto err;
		if unlikely(!self->fb_file)
			goto err_closed;
		self->fb_chng = self->fb_base;
		self->fb_chsz = 0;
		/* Check for special case: If the buffer is fixed to zero-length,
		 *                         we must act as a write-through buffer. */
		if (!self->fb_size) {
			size_t temp;
do_writethrough:
			file = self->fb_file;
			Dee_Incref(file);
			temp = DeeFile_Writef(file, buffer, bufsize, flags);
			Dee_Decref(file);
			if unlikely(temp == (size_t)-1)
				goto err;
			result += temp;
			goto done;
		}
		/* If there is no more available buffer to-be read,
		 * reset the file pointer and change to the next chunk. */
		if (!self->fb_cnt) {
			self->fb_fblk += (size_t)(self->fb_ptr - self->fb_base);
			self->fb_ptr = self->fb_base;
		}
		goto again;
	}

	/* Extend the buffer */
	new_bufsize = self->fb_size * 2;
	if (new_bufsize < FILE_BUFSIZ_MIN)
		new_bufsize = FILE_BUFSIZ_MIN;
	if (new_bufsize > FILE_BUFSIZ_MAX)
		new_bufsize = FILE_BUFSIZ_MAX;
	new_buffer = buffer_tryrealloc_nolock(self, new_bufsize);
	if unlikely(!new_buffer) {
		/* Buffer relocation failed. - sync() + operate in write-through mode as fallback. */
		if (self->fb_flag & FILE_BUFFER_FISATTY) {
			int error = DeeFileBuffer_SyncTTYs(self);
			if unlikely(error) {
				if unlikely(error < 0)
					goto err;
				goto did_unlock;
			}
		}
		if (buffer_sync_nolock(self, BUFFER_SYNC_FERROR_IF_CLOSED))
			goto err;
		if unlikely(!self->fb_file)
			goto err_closed;
		self->fb_chng = self->fb_base;
		self->fb_chsz = 0;
		goto do_writethrough;
	}
	/* Install the new buffer. */
	self->fb_ptr  = new_buffer + (self->fb_ptr - self->fb_base);
	self->fb_chng = new_buffer + (self->fb_chng - self->fb_base);
	self->fb_size = new_bufsize;
	self->fb_base = new_buffer;
	/* Go back and use the new buffer. */
	goto again;

done:
	return result;
err_closed:
	err_buffer_closed();
err:
	return (size_t)BUFFER_IO_ERROR;
did_unlock:
	return (size_t)BUFFER_IO_DID_UNLOCK;
}

PRIVATE Dee_pos_t DCALL
buffer_seek_or_unlock(Buffer *__restrict self,
                      Dee_off_t off, int whence) {
	Dee_pos_t result;
	DREF DeeObject *file;
	if (whence == SEEK_SET || whence == SEEK_CUR) {
		Dee_pos_t old_abspos;
		Dee_pos_t new_abspos;
		byte_t *new_pos;
		if unlikely(!self->fb_file)
			goto err_closed;

		/* For these modes, we can calculate the new position,
		 * allowing for in-buffer seek, as well as delayed seek. */
		old_abspos = self->fb_fblk;
		old_abspos += (self->fb_ptr - self->fb_base);
		if (whence == SEEK_SET) {
			new_abspos = (Dee_pos_t)off;
		} else {
			/* Special case: position-query */
			if (off == 0)
				return old_abspos;
			new_abspos = old_abspos + off;
			off = new_abspos - self->fb_fpos; /* In case we need to do a full seek. */
		}
		if unlikely(new_abspos >= INT64_MAX) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Invalid seek offset");
			goto err;
		}
		if (new_abspos < old_abspos)
			goto full_seek;
#if __SIZEOF_SIZE_T__ < Dee_SIZEOF_POS_T
		if ((new_abspos - old_abspos) >= SIZE_MAX)
			goto full_seek;
#endif /* ZEOF_POINTER__ < Dee_SIZEOF_POS_T */

		/* Seek-ahead-in-buffer. */
		new_pos = self->fb_base + (size_t)(new_abspos - self->fb_fblk);
#if __SIZEOF_SIZE_T__ < Dee_SIZEOF_POS_T
		if (new_pos < self->fb_base)
			goto full_seek;
#endif /* __SIZEOF_SIZE_T__ < Dee_SIZEOF_POS_T */

		/* Truncate the read buffer */
		if (new_pos < self->fb_ptr) {
			/* Seek below the current pointer (but we don't
			 * remember how much was actually read there, so
			 * we simply truncate the buffer fully) */
			self->fb_cnt = 0;
		} else {
			size_t skipsz = (size_t)(new_pos - self->fb_ptr);
			if (self->fb_cnt >= skipsz) {
				self->fb_cnt -= skipsz;
			} else {
				/* New position lies beyond the current buffer end.
				 * -> In this case, we must do a full seek. */
				goto full_seek;
			}
		}
		self->fb_ptr = new_pos;
		return new_abspos;
	}
full_seek:
	if (buffer_determine_isatty(self))
		goto err;
	if (self->fb_flag & FILE_BUFFER_FISATTY) {
		int error = DeeFileBuffer_SyncTTYs(self);
		if unlikely(error) {
			if unlikely(error < 0)
				goto err;
			goto did_unlock;
		}
	}
	/* Synchronize the buffer. */
	if (buffer_sync_nolock(self, BUFFER_SYNC_FERROR_IF_CLOSED))
		goto err;
	if unlikely(!self->fb_file)
		goto err_closed;
	self->fb_chng = self->fb_base;
	self->fb_chsz = 0;
	file          = self->fb_file;
	Dee_Incref(file);

	/* Do a full seek using the underlying file. */
	result = DeeFile_Seek(file, off, whence);
	Dee_Decref(file);
	if unlikely(result == (Dee_pos_t)-1)
		goto err;

	/* Clear the buffer and set the new file pointer. */
	self->fb_fblk = result;
	self->fb_fpos = result;
	self->fb_cnt  = 0;
	self->fb_ptr  = self->fb_base;
	self->fb_chng = self->fb_base;
	self->fb_chsz = 0;
	return result;
err_closed:
	err_buffer_closed();
err:
	return (Dee_pos_t)BUFFER_IO_ERROR;
did_unlock:
	return (Dee_pos_t)BUFFER_IO_DID_UNLOCK;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
buffer_sync_nolock(Buffer *__restrict self, uint16_t mode) {
	Dee_pos_t abs_chngpos;
	size_t changed_size;
	size_t temp;
	uint16_t old_flags;
	DREF DeeObject *file;
again:
	if unlikely(!self->fb_file) {
		/* The buffer has been closed. */
		if (!(mode & BUFFER_SYNC_FERROR_IF_CLOSED))
			goto done;
		goto err_closed;
	}
	changed_size = self->fb_chsz;
	if (!changed_size)
		goto done;
	file = self->fb_file;
	Dee_Incref(file);
	abs_chngpos = self->fb_fblk;
	abs_chngpos += (self->fb_chng - self->fb_base);
	if (abs_chngpos != self->fb_fpos) {
		Dee_pos_t new_pos;

		/* Seek to the position where we need to change stuff. */
		new_pos = DeeFile_SetPos(file, abs_chngpos);
		Dee_Decref(file);
		if unlikely(new_pos == (Dee_pos_t)-1)
			goto err;
		self->fb_fpos = new_pos;

		/* Since the buffer may have changed, we need to start over. */
		goto again;
	}

	/* Write all changed data. */
	old_flags = self->fb_flag;
	self->fb_flag |= FILE_BUFFER_FREADING;
	temp = DeeFile_WriteAll(file,
	                        self->fb_chng,
	                        changed_size);
	self->fb_flag &= ~FILE_BUFFER_FREADING;
	self->fb_flag |= old_flags & FILE_BUFFER_FREADING;
	Dee_Decref(file);
	if unlikely(temp == (size_t)-1)
		goto err;
	if (changed_size == self->fb_chsz && self->fb_file &&
	    abs_chngpos == self->fb_fblk + (self->fb_chng - self->fb_base)) {
		/* Data was synchronized. */
		self->fb_chsz = 0;
		self->fb_fpos += changed_size;
		self->fb_ptr = self->fb_chng + changed_size;

		/* Remove the buffer from the chain of changed, line-buffered TTYs. */
		buffer_deltty(self);

		/* Also synchronize the underlying file. */
		if ((self->fb_flag & FILE_BUFFER_FSYNC) &&
		    !(mode & BUFFER_SYNC_FNOSYNC_FILE)) {
			int error;
			file = self->fb_file;
			Dee_Incref(file);
			error = DeeFile_Sync(file);
			Dee_Decref(file);
			if (error)
				goto err;
		}
	}
done:
	return 0;
err_closed:
	err_buffer_closed();
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
buffer_getc_or_unlock(Buffer *__restrict self, dioflag_t flags) {
	byte_t *new_buffer;
	size_t read_size;
	DREF DeeObject *file;
	uint16_t old_flags;
	int result;
	Dee_pos_t next_data;
again:
	if (self->fb_cnt) {
read_from_buffer:
		/* Simple case: we can read from the active buffer. */
		result = (int)(unsigned int)(unsigned char)*self->fb_ptr++;
		--self->fb_cnt;
		goto done;
	}

	/* The buffer is empty and must be re-filled. */
	/* First off: Flush any changes that had been made. */
	if (buffer_sync_nolock(self, BUFFER_SYNC_FERROR_IF_CLOSED))
		goto err;
	self->fb_chng = self->fb_base;
	self->fb_chsz = 0;

	/* Unlikely: But can happen due to recursive callbacks. */
	if unlikely(self->fb_cnt)
		goto read_from_buffer;
	if unlikely(!self->fb_file)
		goto err_closed;

	/* If we're a TTY buffer, flush all other TTY buffers before reading. */
	if (buffer_determine_isatty(self))
		goto err;
	if (self->fb_flag & FILE_BUFFER_FISATTY) {
		int error;
		error = DeeFileBuffer_SyncTTYs(self);
		if unlikely(error != 0) {
			if unlikely(error < 0)
				goto err;
			goto did_unlock;
		}
		if unlikely(self->fb_cnt)
			goto read_from_buffer;
		if unlikely(!self->fb_file)
			goto err_closed;
		self->fb_chng = self->fb_base;
		self->fb_chsz = 0;
	}

	/* Determine where the next block of data is. */
	next_data = self->fb_fblk + (self->fb_ptr - self->fb_base);

	/* If no buffer had been allocated, allocate one now. */
	if unlikely(!self->fb_size) {
		if unlikely(self->fb_flag & FILE_BUFFER_FNODYNSCALE) {
			/* Dynamic scaling is disabled. Must forward the getc() to the underlying file. */
read_through:
			file = self->fb_file;
			Dee_Incref(file);
			if (next_data != self->fb_fpos) {
				/* Seek in the underlying file to get where we need to go. */
				Dee_pos_t new_pos;
				new_pos = DeeFile_SetPos(file, next_data);
				Dee_Decref(file);
				if unlikely(new_pos == (Dee_pos_t)-1)
					goto err;
				self->fb_fpos = next_data;
				goto again; /* Must start over because of recursive callbacks. */
			}
			result = DeeFile_Getcf(file, flags);
			Dee_Decref(file);
			if (result >= 0) {
				/* Set the file and block address. */
				self->fb_fpos = next_data + 1;
				self->fb_fblk = next_data + 1;
			}
			goto done;
		}
		/* Start out with the smallest size. */
		new_buffer = buffer_tryrealloc_nolock(self, FILE_BUFSIZ_MIN);
		if unlikely(!new_buffer) {
			if (Dee_CollectMemory(1))
				goto again;
			goto err;
		}
		self->fb_base = new_buffer;
		self->fb_size = FILE_BUFSIZ_MIN;
	} else {
		if (self->fb_size < FILE_BUFSIZ_MIN &&
		    !(self->fb_flag & (FILE_BUFFER_FNODYNSCALE | FILE_BUFFER_FREADING))) {
			/* Upscale the buffer. */
			new_buffer = buffer_tryrealloc_nolock(self, FILE_BUFSIZ_MIN);
			if unlikely(!new_buffer)
				goto read_through;
			self->fb_base = new_buffer;
			self->fb_size = FILE_BUFSIZ_MIN;
		}
	}

	self->fb_ptr  = self->fb_base;
	self->fb_chng = self->fb_base;
	self->fb_fblk = next_data;
	ASSERT(self->fb_cnt == 0);
	ASSERT(self->fb_chsz == 0);
	ASSERT(self->fb_size != 0);
	ASSERT(self->fb_file);
	file = self->fb_file;
	Dee_Incref(file);
	if (next_data != self->fb_fpos) {
		/* Seek in the underlying file to get where we need to go. */
		Dee_pos_t new_pos;
		new_pos = DeeFile_SetPos(file, next_data);
		Dee_Decref(file);
		if unlikely(new_pos == (Dee_pos_t)-1)
			goto err;
		self->fb_fpos = next_data;
		goto again; /* Must start over because of recursive callbacks. */
	}

	/* Actually read the data. */
	new_buffer = self->fb_base;
	old_flags  = self->fb_flag;
	self->fb_flag |= FILE_BUFFER_FREADING;
	read_size = DeeFile_Readf(file, self->fb_base, self->fb_size, flags);
	Dee_Decref(file);
	self->fb_flag &= ~FILE_BUFFER_FREADING;
	self->fb_flag |= old_flags & FILE_BUFFER_FREADING;
	if unlikely(read_size == (size_t)-1)
		goto err;
	if unlikely(self->fb_cnt)
		goto read_from_buffer;
	if unlikely(self->fb_chsz)
		goto again;
	self->fb_fpos = next_data + read_size;

	/* Check for special case: EOF reached. */
	if (!read_size) {
		result = GETC_EOF;
	} else {
		self->fb_cnt = read_size - 1;
		self->fb_ptr = self->fb_base + 1;
		result       = (int)(unsigned int)(unsigned char)*self->fb_base;
	}
done:
	return result;
err_closed:
	err_buffer_closed();
err:
	result = GETC_ERR;
	goto done;
did_unlock:
	result = GETC_DID_UNLOCK;
	goto done;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
buffer_ungetc_nolock(Buffer *__restrict self, int ch) {
	byte_t *new_buffer;
	size_t new_bufsize, inc_size;
again:
	/* Simple case: unget() the character. */
	if (self->fb_ptr != self->fb_base)
		goto unget_in_buffer;
	/* The buffer is already full. - Try to resize it, then insert at the front. */
	if (self->fb_flag & FILE_BUFFER_FREADING)
		goto eof;
	if (self->fb_flag & FILE_BUFFER_FNODYNSCALE) {
		/* Check for special case: Even when dynscale is disabled,
		 * still allow for an unget buffer of at least a single byte. */
		if (self->fb_size != 0)
			goto eof;
	}
	/* If the current block cannot be further extended, that's an EOF. */
	if (!self->fb_fblk)
		goto eof;
	inc_size = self->fb_size;
	/* Determine the minimum buffer size. */
	if unlikely(!inc_size)
		inc_size = (self->fb_flag & FILE_BUFFER_FNODYNSCALE) ? 1 : FILE_BUFSIZ_MIN;
	if (inc_size > self->fb_fblk)
		inc_size = (size_t)self->fb_fblk;
	new_bufsize = self->fb_size + inc_size;
	new_buffer  = buffer_tryrealloc_nolock(self, new_bufsize);
	if unlikely(!new_buffer) {
		inc_size    = 1;
		new_bufsize = self->fb_size + 1;
		new_buffer  = buffer_tryrealloc_nolock(self, new_bufsize);
		if unlikely(!new_buffer) {
			if (Dee_CollectMemory(1))
				goto again;
			goto err;
		}
	}
	ASSERT(new_bufsize > self->fb_size);
	/* Install the new buffer. */
	self->fb_ptr = new_buffer + (self->fb_ptr - self->fb_base) + inc_size;
	if (self->fb_chsz)
		self->fb_chng = new_buffer + (self->fb_chng - self->fb_base) + inc_size;
	self->fb_fblk -= inc_size;
	self->fb_base = new_buffer;
	self->fb_size = new_bufsize;
	ASSERT(self->fb_ptr != self->fb_base);
	/* Finally, insert the character into the buffer. */
unget_in_buffer:
	*--self->fb_ptr = (byte_t)(unsigned int)ch;
	++self->fb_cnt;
	return 0;
eof:
	return GETC_EOF;
err:
	return GETC_ERR;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
buffer_trunc_nolock(Buffer *__restrict self, Dee_pos_t new_size) {
	Dee_pos_t abs_pos, abs_end;
	int result;
	DREF DeeObject *file;
	/* Synchronize the buffer. */
	if unlikely(buffer_sync_nolock(self, BUFFER_SYNC_FERROR_IF_CLOSED))
		goto err;
	if unlikely(!self->fb_file)
		goto err_closed;
	self->fb_chng = self->fb_base;
	self->fb_chsz = 0;
	abs_pos       = self->fb_fblk + (self->fb_ptr - self->fb_base);
	abs_end       = abs_pos + self->fb_cnt;
	if (new_size < abs_pos) {
		/* Truncate to get rid of the current buffer. */
		self->fb_cnt = 0;
	} else if (new_size < abs_end) {
		/* Truncate the current buffer. */
		self->fb_cnt = (size_t)(new_size - abs_pos);
	}
	/* With data flushed and the loaded buffer
	 * truncated, truncate the underlying file. */
	file = self->fb_file;
	Dee_Incref(file);
	result = DeeFile_Trunc(file, new_size);
	Dee_Decref(file);
	return result;
err_closed:
	err_buffer_closed();
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
buffer_read(Buffer *__restrict self,
            void *__restrict buffer,
            size_t bufsize, dioflag_t flags) {
	size_t result;
	do {
		if (DeeFileBuffer_LockWrite(self))
			goto err;
		result = buffer_read_or_unlock(self, (byte_t *)buffer, bufsize, flags);
	} while unlikely(result == (size_t)BUFFER_IO_DID_UNLOCK);
	DeeFileBuffer_LockEndWrite(self);
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
buffer_write(Buffer *__restrict self,
             void const *__restrict buffer,
             size_t bufsize, dioflag_t flags) {
	size_t result;
	do {
		if (DeeFileBuffer_LockWrite(self))
			goto err;
		result = buffer_write_or_unlock(self, (byte_t const *)buffer, bufsize, flags);
	} while unlikely(result == (size_t)BUFFER_IO_DID_UNLOCK);
	DeeFileBuffer_LockEndWrite(self);
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
buffer_sync(Buffer *__restrict self) {
	int result;
	/* Important check: if there's nothing to sync, don't even try to lock the file! */
	if (atomic_read(&self->fb_chsz) == 0) {
		result = 0;
	} else {
		if (DeeFileBuffer_LockWrite(self))
			goto err;
		result = buffer_sync_nolock(self, BUFFER_SYNC_FERROR_IF_CLOSED);
		DeeFileBuffer_LockEndWrite(self);
	}
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
buffer_getc(Buffer *__restrict self, dioflag_t flags) {
	int result;
	do {
		if (DeeFileBuffer_LockWrite(self))
			goto err;
		result = buffer_getc_or_unlock(self, flags);
	} while unlikely(result == GETC_DID_UNLOCK);
	DeeFileBuffer_LockEndWrite(self);
	return result;
err:
	return GETC_ERR;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
buffer_ungetc(Buffer *__restrict self, int ch) {
	int result;
	if (DeeFileBuffer_LockWrite(self))
		goto err;
	result = buffer_ungetc_nolock(self, ch);
	DeeFileBuffer_LockEndWrite(self);
	return result;
err:
	return GETC_ERR;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
buffer_trunc(Buffer *__restrict self, Dee_pos_t new_size) {
	int result;
	if (DeeFileBuffer_LockWrite(self))
		goto err;
	result = buffer_trunc_nolock(self, new_size);
	DeeFileBuffer_LockEndWrite(self);
	return result;
err:
	return -1;
}

PRIVATE Dee_pos_t DCALL
buffer_seek(Buffer *__restrict self,
            Dee_off_t off, int whence) {
	Dee_pos_t result;
	do {
		if (DeeFileBuffer_LockWrite(self))
			goto err;
		result = buffer_seek_or_unlock(self, off, whence);
	} while unlikely(result == (Dee_pos_t)BUFFER_IO_DID_UNLOCK);
	DeeFileBuffer_LockEndWrite(self);
	return result;
err:
	return (Dee_pos_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
buffer_close(Buffer *__restrict self) {
	DREF DeeObject *file;
	byte_t *buffer;
	uint16_t flags;
	if (DeeFileBuffer_LockWrite(self))
		goto err;
	if unlikely(buffer_sync_nolock(self, BUFFER_SYNC_FERROR_IF_CLOSED))
		goto err_unlock;
	file   = self->fb_file;
	buffer = self->fb_base;
	flags  = self->fb_flag;

	/* Close the underlying file when that flag is set. */
	if ((flags & FILE_BUFFER_FCLOFILE) &&
	    DeeFile_Close(file))
		goto err_unlock;

	self->fb_file = NULL;

	/* Clear out buffer pointers. */
	self->fb_ptr = NULL;
	self->fb_cnt = 0;
	if (flags & FILE_BUFFER_FREADING) {
		buffer = NULL;
	} else {
		self->fb_base = NULL;
		self->fb_size = 0;
	}
	self->fb_chng = NULL;
	self->fb_chsz = 0;
	if (flags & FILE_BUFFER_FSTATICBUF)
		buffer = NULL;
	self->fb_flag = FILE_BUFFER_FNORMAL | (flags & FILE_BUFFER_FREADING);
	self->fb_fpos = 0;
	buffer_deltty(self);
	DeeFileBuffer_LockEndWrite(self);

	Dee_XDecref(file); /* May already be NULL due to recursive callbacks. */
	Dee_Free(buffer);
	return 0;
err_unlock:
	DeeFileBuffer_LockEndWrite(self);
err:
	return -1;
}


struct mode_name {
	char name[4];  /* Mode name. */
	uint16_t flag; /* Mode flags. */
};

PRIVATE struct mode_name const mode_names[] = {
	{ { 'k', 'e', 'e', 'p' }, FILE_BUFFER_MODE_KEEP }, /* Must be kept first. */
	{ { 'n', 'o', 'n', 'e' }, FILE_BUFFER_MODE_NONE },
	{ { 'f', 'u', 'l', 'l' }, FILE_BUFFER_MODE_FULL },
	{ { 'l', 'i', 'n', 'e' }, FILE_BUFFER_MODE_LINE },
	{ { 'a', 'u', 't', 'o' }, FILE_BUFFER_MODE_AUTO }
};

/* CASEEQ(x, 'w') --> x == 'w' || x == 'W' */
#define CASEEQ(x, ch) ((x) == (ch) || (x) == (ch) - ('a' - 'A'))

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
buffer_print(Buffer *__restrict self, Dee_formatprinter_t printer, void * arg) {
	Dee_ssize_t result;
	char const *mode;
	DREF DeeObject *inner_file;
	uint16_t buffer_flags;
	if (DeeFileBuffer_LockRead(self))
		goto err;
	inner_file = self->fb_file;
	if (inner_file == NULL) {
		DeeFileBuffer_LockEndRead(self);
		return DeeFormat_PRINT(printer, arg, "<Buffer (closed)>");
	}
	buffer_flags = self->fb_flag;
	Dee_Incref(inner_file);
	DeeFileBuffer_LockEndRead(self);
	if (buffer_flags & FILE_BUFFER_FLNBUF) {
		mode = "Line";
	} else if (buffer_flags & FILE_BUFFER_FNODYNSCALE) {
		mode = "NoOp";
	} else if (buffer_flags & FILE_BUFFER_FLNIFTTY) {
		mode = "Auto";
	} else {
		mode = "Full";
	}
	result = DeeFormat_Printf(printer, arg,
	                          "<%s-Buffer for %k>",
	                          mode, inner_file);
	Dee_Decref(inner_file);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
buffer_printrepr(Buffer *__restrict self, Dee_formatprinter_t printer, void * arg) {
	char mode[sizeof("sync,close,readonly,auto")], *mode_iter;
	Dee_ssize_t result;
	DREF DeeObject *inner_file;
	uint16_t buffer_flags;
	size_t buffer_size;
	if (DeeFileBuffer_LockRead(self))
		goto err;
	inner_file = self->fb_file;
	if (inner_file == NULL) {
		DeeFileBuffer_LockEndRead(self);
		return DeeFormat_PRINT(printer, arg, "File.Buffer()<.closed>");
	}
	buffer_flags = self->fb_flag;
	buffer_size  = self->fb_size;
	Dee_Incref(inner_file);
	DeeFileBuffer_LockEndRead(self);

	/* Re-construct the buffer mode string. */
	mode_iter = mode;
	if (buffer_flags & Dee_FILE_BUFFER_FSYNC)
		mode_iter = stpcpy(mode_iter, "sync,");
	if (buffer_flags & Dee_FILE_BUFFER_FCLOFILE)
		mode_iter = stpcpy(mode_iter, "close,");
	if (buffer_flags & Dee_FILE_BUFFER_FREADONLY)
		mode_iter = stpcpy(mode_iter, "readonly,");
	if (buffer_flags & FILE_BUFFER_FLNBUF) {
		mode_iter = stpcpy(mode_iter, "line");
	} else if (buffer_flags & FILE_BUFFER_FNODYNSCALE) {
		mode_iter = stpcpy(mode_iter, "none");
	} else if (buffer_flags & FILE_BUFFER_FLNIFTTY) {
		mode_iter = stpcpy(mode_iter, "auto");
	} else {
		mode_iter = stpcpy(mode_iter, "full");
	}

	/* Render the buffer's representation. */
	result = DeeFormat_Printf(printer, arg,
	                          "File.Buffer(%r, %q, %" PRFuSIZ ")",
	                          inner_file, mode_iter, buffer_size);
	Dee_Decref(inner_file);
	return result;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
buffer_init_operator(Buffer *__restrict self,
                     size_t argc, DeeObject *const *argv) {
	uint16_t mode = (FILE_BUFFER_MODE_AUTO);
	struct {
		DeeObject *file;
		char const *mode_str;
		size_t size;
	} args;
	args.mode_str = NULL;
	args.size     = 0;
	if (DeeArg_UnpackStruct(argc, argv, "o|sd:_FileBuffer", &args))
		goto err;
	if (args.mode_str) {
		char const *mode_iter = args.mode_str;
		unsigned int i;
		union {
			char chrs[4];
			uint32_t id;
		} buf;
		mode = 0;
		/* Interpret the given mode string. */
		for (;;) {
			if (CASEEQ(*mode_iter, 'r')) {
				++mode_iter;
				if (CASEEQ(*mode_iter, 'o')) {
					++mode_iter;
					if (*mode_iter == '-' || *mode_iter == ',')
						++mode_iter;
				} else if (CASEEQ(mode_iter[0], 'e') &&
				           CASEEQ(mode_iter[1], 'a') &&
				           CASEEQ(mode_iter[2], 'd') &&
				           CASEEQ(mode_iter[4], 'o') &&
				           CASEEQ(mode_iter[5], 'n') &&
				           CASEEQ(mode_iter[6], 'l') &&
				           CASEEQ(mode_iter[7], 'y') &&
				           mode_iter[8] == ',') {
					mode_iter += 9;
				} else {
					goto err_invalid_mode;
				}
				if (mode & FILE_BUFFER_FREADONLY)
					goto err_invalid_mode;
				mode |= FILE_BUFFER_FREADONLY;
				continue;
			}
			if (CASEEQ(*mode_iter, 'c')) {
				++mode_iter;
				if (*mode_iter == '-' || *mode_iter == ',') {
					++mode_iter;
				} else if (CASEEQ(mode_iter[0], 'l') &&
				           CASEEQ(mode_iter[1], 'o') &&
				           CASEEQ(mode_iter[2], 's') &&
				           CASEEQ(mode_iter[3], 'e') &&
				           mode_iter[4] == ',') {
					mode_iter += 5;
				}
				if (mode & FILE_BUFFER_FCLOFILE)
					goto err_invalid_mode;
				mode |= FILE_BUFFER_FCLOFILE;
				continue;
			}
			if (CASEEQ(*mode_iter, 's')) {
				++mode_iter;
				if (*mode_iter == '-' || *mode_iter == ',') {
					++mode_iter;
				} else if (CASEEQ(mode_iter[0], 'y') &&
				           CASEEQ(mode_iter[1], 'n') &&
				           CASEEQ(mode_iter[2], 'c') &&
				           mode_iter[3] == ',') {
					mode_iter += 4;
				}
				if (mode & FILE_BUFFER_FSYNC)
					goto err_invalid_mode;
				mode |= FILE_BUFFER_FSYNC;
				continue;
			}
			break;
		}
		/* Ensure that the string is at least 4 characters long. */
		if ((buf.chrs[0] = (char)DeeUni_ToLower(mode_iter[0])) == '\0' ||
		    (buf.chrs[1] = (char)DeeUni_ToLower(mode_iter[1])) == '\0' ||
		    (buf.chrs[2] = (char)DeeUni_ToLower(mode_iter[2])) == '\0' ||
		    (buf.chrs[3] = (char)DeeUni_ToLower(mode_iter[3])) == '\0')
			goto err_invalid_mode;

		/* Parse the main mode name. */
		for (i = 1;; ++i) {
			if (i == COMPILER_LENOF(mode_names))
				goto err_invalid_mode;
			if (*(uint32_t *)mode_names[i].name != buf.id)
				continue;
			mode |= mode_names[i].flag; /* Found it! */
			break;
		}
	}
	return buffer_init(self, args.file, mode, args.size);
err_invalid_mode:
	DeeError_Throwf(&DeeError_ValueError,
	                "Unrecognized buffer mode `%s'",
	                args.mode_str);
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
buffer_fini(Buffer *__restrict self) {
	/* Synchronize the buffer one last time. */
	if unlikely(buffer_sync_nolock(self, BUFFER_SYNC_FNORMAL)) {
		DeeError_Print("Discarding error in buffer synchronization in finalization\n",
		               ERROR_PRINT_DOHANDLE);
	}

	/* Unlink from TTY buffers (must happen *AFTER* the final sync) */
	if (LIST_ISBOUND(self, fb_ttych)) {
#ifndef CONFIG_NO_THREADS
		COMPILER_READ_BARRIER();
		buffer_ttys_lock_acquire();
		if (LIST_ISBOUND(self, fb_ttych))
#endif /* !CONFIG_NO_THREADS */
		{
			LIST_REMOVE(self, fb_ttych);
		}
		buffer_ttys_lock_release();
	}

	Dee_XDecref(self->fb_file);
	if (!(self->fb_flag & FILE_BUFFER_FSTATICBUF))
		Dee_Free(self->fb_base);
}

PRIVATE NONNULL((1, 2)) void DCALL
buffer_visit(Buffer *__restrict self, dvisit_t proc, void *arg) {
	DeeFileBuffer_LockWriteNoInt(self);
	Dee_XVisit(self->fb_file);
	DeeFileBuffer_LockEndWrite(self);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
buffer_size(Buffer *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *file, *result;
	if (DeeFileBuffer_LockWrite(self))
		goto err;
	file = self->fb_file;
	if unlikely(!file)
		goto err_closed_unlock;
	Dee_Incref(file);
	DeeFileBuffer_LockEndWrite(self);
	/* Forward the to contained file. */
	result = DeeObject_CallAttr(file, (DeeObject *)&str_size, argc, argv);
	Dee_Decref(file);
	return result;
err_closed_unlock:
	DeeFileBuffer_LockEndWrite(self);
	err_buffer_closed();
err:
	return NULL;
}

#ifdef Dee_fd_GETSET
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
buffer_getsysfd(Buffer *__restrict self) {
	DREF DeeObject *file, *result;
	if (DeeFileBuffer_LockRead(self))
		goto err;
	file = self->fb_file;
	if unlikely(!file)
		goto err_closed_unlock;
	Dee_Incref(file);
	DeeFileBuffer_LockEndRead(self);
	/* Forward the to contained file. */
	result = DeeObject_GetAttr(file, (DeeObject *)&str_getsysfd);
	Dee_Decref(file);
	return result;
err_closed_unlock:
	DeeFileBuffer_LockEndRead(self);
	err_buffer_closed();
err:
	return NULL;
}
#endif /* Dee_fd_GETSET */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
buffer_isatty(Buffer *__restrict self) {
	int error;
	if (DeeFileBuffer_LockWrite(self))
		goto err;
	/* Determine if the buffer points to a TTY. */
	error = buffer_determine_isatty(self);
	DeeFileBuffer_LockEndWrite(self);
	if unlikely(error)
		goto err;
	return_bool(self->fb_flag & FILE_BUFFER_FISATTY);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
buffer_flush(Buffer *self, size_t argc, DeeObject *const *argv) {
	int error;
	_DeeArg_Unpack0(err, argc, argv, "flush");
	if (DeeFileBuffer_LockWrite(self))
		goto err;
	/* Synchronize the buffer, but don't synchronize its file. */
	error = buffer_sync_nolock(self,
	                           BUFFER_SYNC_FERROR_IF_CLOSED |
	                           BUFFER_SYNC_FNOSYNC_FILE);
	DeeFileBuffer_LockEndWrite(self);
	if unlikely(error)
		goto err;
	return_bool(self->fb_flag & FILE_BUFFER_FISATTY);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
buffer_setbuf(Buffer *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	uint16_t mode;
	char const *mode_iter;
	char const *mode_str;
	size_t size = 0;
	unsigned int i;
	union {
		char chrs[4];
		uint32_t id;
	} buf;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__mode_size,
	                    "s|d:setbuf", &mode_str, &size))
		goto err;
	mode_iter = mode_str, mode = 0;
	/* Interpret the given mode string. */
	for (;;) {
		if (CASEEQ(*mode_iter, 's')) {
			++mode_iter;
			if (*mode_iter == '-' || *mode_iter == ',') {
				++mode_iter;
			} else if (CASEEQ(mode_iter[0], 'y') &&
			           CASEEQ(mode_iter[1], 'n') &&
			           CASEEQ(mode_iter[2], 'c') &&
			           mode_iter[3] == ',') {
				mode_iter += 4;
			}
			if (mode & FILE_BUFFER_FSYNC)
				goto err_invalid_mode;
			mode |= FILE_BUFFER_FSYNC;
			continue;
		}
		break;
	}
	/* Ensure that the string is at least 4 characters long. */
	if ((buf.chrs[0] = (char)DeeUni_ToLower(mode_iter[0])) == '\0' ||
	    (buf.chrs[1] = (char)DeeUni_ToLower(mode_iter[1])) == '\0' ||
	    (buf.chrs[2] = (char)DeeUni_ToLower(mode_iter[2])) == '\0' ||
	    (buf.chrs[3] = (char)DeeUni_ToLower(mode_iter[3])) == '\0')
		goto err_invalid_mode;
	if (mode_iter[4])
		goto err_invalid_mode;

	/* Parse the main mode name. */
	for (i = 0;; ++i) {
		if (i == COMPILER_LENOF(mode_names))
			goto err_invalid_mode;
		if (*(uint32_t *)mode_names[i].name != buf.id)
			continue;
		mode |= mode_names[i].flag; /* Found it! */
		break;
	}
	if (DeeFileBuffer_SetMode((DeeObject *)self, mode, size))
		goto err;
	return_none;
err_invalid_mode:
	DeeError_Throwf(&DeeError_ValueError,
	                "Unrecognized buffer mode `%s'",
	                mode_str);
err:
	return NULL;
}

PRIVATE struct type_method tpconst buffer_methods[] = {
	TYPE_METHOD_F(STR_size, &buffer_size, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Forward to the $size function of the file being buffered"),
	TYPE_METHOD_F("flush", &buffer_flush, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Similar to ?#sync, but never synchronize the underlying file, regardless "
	              /**/ "of whether or not $\"nosync\" was passed to the constructor, or ?#setbuf"),
	TYPE_KWMETHOD_F("setbuf", &buffer_setbuf, METHOD_FNOREFESCAPE,
	                "(mode:?Dstring,size=!0)\n"
	                "#tValueError{The given @mode is malformed, or not recognized}"
	                "Set the buffering mode of @this buffer to @mode, with a buffer size of @size\n"
	                "When non-zero, @size dictates the initial buffer size, or a fixed buffer size when "
	                /**/ "the buffering mode is set to $\"none\", otherwise a buffer size of $0 allows "
	                /**/ "the buffer to keep its buffer size implementation-defined, as well as allow it to "
	                /**/ "dynamically change said size if doing so is deemed useful\n"
	                "The given @mode must case-insensitively equal to one of the following:\n"
	                "#T{Mode|Description~"
	                "$\"auto\"|Automatically determine buffering, using line-buffering for TTY files, and full buffering for all others&"
	                "$\"full\"|Use full buffering, only synchronizing the buffer when it becomes full&"
	                "$\"line\"|Use line-buffering, automatically synchronizing the buffer whenever a linefeed is written&"
	                "$\"none\"|Use no buffering at all when @size is ${0}, or use a fixed-length buffer of @size bytes. "
	                /*     */ "Note that even when the no buffer is allocated, the runtime guaranties "
	                /*     */ "that ?#ungetc can be used to return and re-read at least 1 byte&"
	                "$\"keep\"|Keep using the old buffering model, but allow the buffer size to be changed}\n"
	                "Additionally, @mode may be prefixed with one or more of the following case-insensitive strings:\n"
	                "#T{Prefix|Description~"
	                "$\"s\", $\"s-\", $\"s,\", $\"sync,\"|Also invoke ?#sync on @this buffer's ?#file whenever the buffer is synced}"),
	TYPE_METHOD_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
buffer_filename(Buffer *__restrict self) {
	DREF DeeObject *file, *result;
	if (DeeFileBuffer_LockWrite(self))
		goto err;
	file = self->fb_file;
	if unlikely(!file)
		goto err_closed_unlock;
	Dee_Incref(file);
	DeeFileBuffer_LockEndWrite(self);
	/* Forward the to contained file. */
	result = DeeObject_GetAttr(file, (DeeObject *)&str_filename);
	Dee_Decref(file);
	return result;
err_closed_unlock:
	DeeFileBuffer_LockEndWrite(self);
	err_buffer_closed();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
buffer_getfile(Buffer *__restrict self) {
	DREF DeeObject *result;
	if (DeeFileBuffer_LockWrite(self))
		goto err;
	result = self->fb_file;
	if unlikely(!result)
		goto err_closed_unlock;
	Dee_Incref(result);
	DeeFileBuffer_LockEndWrite(self);
	return result;
err_closed_unlock:
	DeeFileBuffer_LockEndWrite(self);
	err_buffer_closed();
err:
	return NULL;
}


PRIVATE struct type_getset tpconst buffer_getsets[] = {
#ifdef Dee_fd_GETSET
	TYPE_GETTER_F(STR_getsysfd, &buffer_getsysfd, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tAttributeError{The file being buffered does not implement a member function $" Dee_fd_GETSET "}"
	              "Forward to the $" Dee_fd_GETSET " getset of the file being buffered"),
#endif /* Dee_fd_GETSET */
	TYPE_GETTER_F("file", &buffer_getfile, METHOD_FNOREFESCAPE,
	              "->?DFile\n"
	              "Returns the file that is being buffered"),
	TYPE_GETTER_F(STR_isatty, &buffer_isatty, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Forward to the $isatty property of the file being buffered\n"
	              "Note that in order to implement auto-buffering, file buffers are allowed to "
	              /**/ "cache the return value of ${this.file.isatty}, furthermore allowing this "
	              /**/ "property to simply return that cached value, in other words meaning that "
	              /**/ "any side-effects caused by the underlying $isatty may not come into effect "
	              /**/ "following repeated calls"),
	TYPE_GETTER_F(STR_filename, &buffer_filename, METHOD_FNOREFESCAPE,
	              "->?Dstring\n"
	              "Forward the filename attribute of the file being buffered"),
	TYPE_GETSET_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
buffer_class_sync(DeeObject *UNUSED(self),
                  size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "sync");
	if (DeeFileBuffer_SyncTTYs(NULL) < 0)
		goto err;
#ifdef DEESYSTEM_FILE_USE_STDIO
	/* When the STDIO filesystem is used, also flush its buffers.
	 * Since it doesn't use ours, we need to make sure that its
	 * buffer implementation is flushed when the user requests
	 * all buffers to be flushed. */
	fflush(NULL);
#endif /* DEESYSTEM_FILE_USE_STDIO */
	return_none;
err:
	return NULL;
}

PRIVATE struct type_method tpconst buffer_class_methods[] = {
	TYPE_METHOD("sync", &buffer_class_sync,
	            "()\n"
	            "Flush all buffers that are connected to TTY file descriptors\n"
	            "This function is automatically called at the same time as :AppExit.atexit, as "
	            /**/ "well as prior to any read operation on any other TTY buffer that might block"),
	TYPE_METHOD_END
};

PUBLIC DeeFileTypeObject DeeFileBuffer_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_FileBuffer",
		/* .tp_doc      = */ DOC("General-purpose input/output file buffering\n"
		                         "\n"
		                         "(fp:?DFile,mode=!Pauto,size=!0)\n"
		                         "#tValueError{The given @mode is malformed, or not recognized}"
		                         "Construct a new buffer for @fp using the given @mode and @size\n"
		                         "When non-zero, @size dictates the initial buffer size, or a fixed buffer size when "
		                         /**/ "the buffering mode is set to $\"none\". Otherwise a buffer size of $0 causes "
		                         /**/ "the buffer to use an implementation-defined buffer size, as well as allow it to "
		                         /**/ "dynamically change said size if doing so is deemed useful\n"
		                         "The given @mode must case-insensitively equal to one of the following:\n"
		                         "#T{Mode|Description~"
		                         /**/ "$\"auto\"|Automatically determine buffering, using line-buffering for TTY files, and full buffering for all others&"
		                         /**/ "$\"full\"|Use full buffering, only synchronizing the buffer when it becomes full&"
		                         /**/ "$\"line\"|Use line-buffering, automatically synchronizing the buffer whenever a linefeed is written&"
		                         /**/ "$\"none\"|Use no buffering at all when @size is $0, or use a fixed-length buffer of @size bytes. "
		                         /**/ /*     */ "Note that even when the no buffer is allocated, the runtime guaranties "
		                         /**/ /*     */ "that ?#ungetc can be used to return and re-read at least 1 byte"
		                         "}\n"
		                         "Additionally, @mode may be prefixed with one or more of the following case-insensitive strings:\n"
		                         "#T{Prefix|Description~"
		                         /**/ "$\"s\", $\"s-\", $\"s,\", $\"sync,\"|Also invoke ?#sync on the buffer's "
		                         /**/ /*                                */ "?#file whenever the buffer is synced&"
		                         /**/ "$\"c\", $\"c-\", $\"c,\", $\"close,\"|When the buffer is closed using ${operator close}, "
		                         /**/ /*                                */  "also invoke that same operator on the contained file&"
		                         /**/ "$\"ro\", $\"ro-\", $\"ro,\", $\"readonly,\"|Disable write-access to the underlying @fp"
		                         "}\n"
		                         "${"
		                         /**/ "import File from deemon;\n"
		                         /**/ "local base_fp = File.open(\"foo.txt\", \"rdonly,nobuf\"); /* Open without buffering */\n"
		                         /**/ "local fp = File.Buffer(base_fp, \"sync,full\"); /* Create a buffer wrapper */\n"
		                         /**/ "fp.write(\"foo\"); /* Write data to buffer */\n"
		                         /**/ "fp.sync(); /* Commit data to disk */"
		                         "}\n"
		                         "The most notably useful feature made possible through use of buffers is "
		                         /**/ "the possibility of adding ?#getc and ?#ungetc support to file types that "
		                         /**/ "normally wouldn't support the later\n"
		                         "During initialization, deemon will assign the standard "
		                         /**/ "streams ?Astdin?DFile, ?Astdout?DFile and ?Astderr?DFile as follows:\n"
		                         "${"
		                         /**/ "import File from deemon;\n"
		                         /**/ "File.stdin = File.Buffer(File.default_stdin, \"ro,auto\");\n"
		                         /**/ "File.stdout = File.Buffer(File.default_stdout, \"auto\");\n"
		                         /**/ "File.stderr = File.Buffer(File.default_stderr, \"auto\");"
		                         "}"),
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ (DeeTypeObject *)&DeeFile_Type,
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor      = */ (dfunptr_t)NULL,
					/* .tp_copy_ctor = */ (dfunptr_t)NULL,
					/* .tp_deep_ctor = */ (dfunptr_t)NULL,
					/* .tp_any_ctor  = */ (dfunptr_t)&buffer_init_operator,
					TYPE_FIXED_ALLOCATOR(Buffer)
				}
			},
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&buffer_fini,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str       = */ NULL,
			/* .tp_repr      = */ NULL,
			/* .tp_bool      = */ NULL,
			/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&buffer_print,
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&buffer_printrepr
		},
			/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&buffer_visit,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_iterator      = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */ buffer_methods,
		/* .tp_getsets       = */ buffer_getsets,
		/* .tp_members       = */ NULL,
		/* .tp_class_methods = */ buffer_class_methods,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ NULL
	},
	/* .ft_read   = */ (size_t (DCALL *)(DeeFileObject *__restrict, void *__restrict, size_t, dioflag_t))&buffer_read,
	/* .ft_write  = */ (size_t (DCALL *)(DeeFileObject *__restrict, void const *__restrict, size_t, dioflag_t))&buffer_write,
	/* .ft_seek   = */ (Dee_pos_t (DCALL *)(DeeFileObject *__restrict, Dee_off_t, int))&buffer_seek,
	/* .ft_sync   = */ (int (DCALL *)(DeeFileObject *__restrict))&buffer_sync,
	/* .ft_trunc  = */ (int (DCALL *)(DeeFileObject *__restrict, Dee_pos_t))&buffer_trunc,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *__restrict))&buffer_close,
	/* .ft_pread  = */ NULL,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ (int (DCALL *)(DeeFileObject *__restrict, dioflag_t))&buffer_getc,
	/* .ft_ungetc = */ (int (DCALL *)(DeeFileObject *__restrict, int))&buffer_ungetc,
	/* .ft_putc   = */ NULL
};


PRIVATE ATTR_COLD int DCALL err_buffer_closed(void) {
	return DeeError_Throwf(&DeeError_FileClosed,
	                       "Buffer has been closed");
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_FILEBUFFER_C */
