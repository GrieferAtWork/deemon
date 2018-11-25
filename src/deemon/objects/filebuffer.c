/* Copyright (c) 2018 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_OBJECTS_FILEBUFFER_C
#define GUARD_DEEMON_OBJECTS_FILEBUFFER_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/error.h>
#include <deemon/arg.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/string.h>

#include <string.h>
#ifndef CONFIG_NO_STDLIB
#include <stdlib.h> /* atexit() */
#endif
#if !defined(CONFIG_HOST_WINDOWS) && \
    !defined(CONFIG_HOST_UNIX) && \
    !defined(CONFIG_NO_STDIO)
#include <stdio.h>
#endif

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

DECL_BEGIN

typedef DeeFileBufferObject Buffer;
#define buf_reading(x)       recursive_rwlock_reading(&((Buffer *)(x))->fb_lock)
#define buf_writing(x)       recursive_rwlock_writing(&((Buffer *)(x))->fb_lock)
#define buf_tryread(self)    recursive_rwlock_tryread(&((Buffer *)(self))->fb_lock)
#define buf_trywrite(self)   recursive_rwlock_trywrite(&((Buffer *)(self))->fb_lock)
#define buf_read(self)       recursive_rwlock_read(&((Buffer *)(self))->fb_lock)
#define buf_write(self)      recursive_rwlock_write(&((Buffer *)(self))->fb_lock)
#define buf_tryupgrade(self) recursive_rwlock_tryupgrade(&((Buffer *)(self))->fb_lock)
#define buf_upgrade(self)    recursive_rwlock_upgrade(&((Buffer *)(self))->fb_lock)
#define buf_downgrade(self)  recursive_rwlock_downgrade(&((Buffer *)(self))->fb_lock)
#define buf_endwrite(self)   recursive_rwlock_endwrite(&((Buffer *)(self))->fb_lock)
#define buf_endread(self)    recursive_rwlock_endread(&((Buffer *)(self))->fb_lock)
#define buf_end(self)        recursive_rwlock_end(&((Buffer *)(self))->fb_lock)



/* [0..1][lock(buffer_ttys_lock)] Chain of tty-buffers. */
PRIVATE Buffer *buffer_ttys = NULL;
#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(buffer_ttys_lock);
#define buffer_ttys_lock_enter() rwlock_write(&buffer_ttys_lock)
#define buffer_ttys_lock_leave() rwlock_endwrite(&buffer_ttys_lock)
#else
#define buffer_ttys_lock_enter() (void)0
#define buffer_ttys_lock_leave() (void)0
#endif
#ifndef CONFIG_NO_STDLIB
PRIVATE bool atexit_registered = false;
#endif

/* Add/Remove a given buffer from the TTY-buffer chain. */
PRIVATE void DCALL buffer_addtty(Buffer *__restrict self);
PRIVATE void DCALL buffer_deltty(Buffer *__restrict self);


PRIVATE ATTR_COLD void DCALL err_buffer_closed(void);

PRIVATE dssize_t DCALL buffer_read_nolock(Buffer *__restrict self, uint8_t *__restrict buffer, size_t bufsize, dioflag_t flags);
PRIVATE dssize_t DCALL buffer_write_nolock(Buffer *__restrict self, uint8_t const *__restrict buffer, size_t bufsize, dioflag_t flags);
PRIVATE doff_t DCALL buffer_seek_nolock(Buffer *__restrict self, doff_t off, int whence);
PRIVATE int DCALL buffer_sync_nolock(Buffer *__restrict self, uint16_t mode);
#define BUFFER_SYNC_FNORMAL          0x0000
#define BUFFER_SYNC_FERROR_IF_CLOSED 0x0001 /* Throw an error if the buffer was closed. */
#define BUFFER_SYNC_FNOSYNC_FILE     0x0002 /* Don't synchronize the underlying file, regardless of whether
                                             * or not the `FILE_BUFFER_FSYNC' flag has been set. */
PRIVATE int DCALL buffer_getc_nolock(Buffer *__restrict self, dioflag_t flags);
PRIVATE int DCALL buffer_ungetc_nolock(Buffer *__restrict self, int ch);
PRIVATE int DCALL buffer_trunc_nolock(Buffer *__restrict self, dpos_t new_size);


#ifndef CONFIG_NO_STDLIB
PRIVATE void atexit_flushall(void) {
 for (;;) {
  DREF Buffer *buffer; int error;
  /* XXX: Considering how this function is called during exit(),
   *      can we actually be certain that no other thread was
   *      terminated while still holding this lock?
   *      What does STD-C say about the time of execution of
   *      atexit() functions in relation to other threads and
   *      their potential termination?
   *      I'm guessing it's OK as this'll probably be one of
   *      the first things to get executed during termination,
   *      but can we be certain that it will? */
  buffer_ttys_lock_enter();
  buffer = buffer_ttys;
  while (buffer && !Dee_IncrefIfNotZero(buffer))
         buffer = buffer->fb_ttych.fbl_next;
  if (buffer) {
   ASSERT(buffer->fb_ttych.fbl_pself);
   if ((*buffer->fb_ttych.fbl_pself = buffer->fb_ttych.fbl_next) != NULL)
         buffer->fb_ttych.fbl_next->fb_ttych.fbl_pself = buffer->fb_ttych.fbl_pself;
   buffer->fb_ttych.fbl_pself = NULL;
  }
  buffer_ttys_lock_leave();
  if (!buffer) break;
  /* Synchronize this buffer. */
  buf_write(buffer);
  error = buffer_sync_nolock(buffer,BUFFER_SYNC_FNORMAL);
  buf_endwrite(buffer);
  if unlikely(error) {
   DeeError_Print("Failed to synchronize tty during exit\n",
                  ERROR_PRINT_HANDLEINTR);
  }
  Dee_Decref(buffer);
 }
}
#endif

PRIVATE void DCALL
buffer_addtty(Buffer *__restrict self) {
 buffer_ttys_lock_enter();
#ifndef CONFIG_NO_STDLIB
 if (!atexit_registered) {
  /* NOTE: If atexit() fails, there's nothing we could do about it,
   *       so we won't even bother to handle any errors it may return. */
  atexit(&atexit_flushall);
  atexit_registered = true;
 }
#endif
 if (!self->fb_ttych.fbl_pself) {
  self->fb_ttych.fbl_pself = &buffer_ttys;
  if ((self->fb_ttych.fbl_next = buffer_ttys) != NULL)
       buffer_ttys->fb_ttych.fbl_pself = &self->fb_ttych.fbl_next;
  buffer_ttys = self;
 }
 buffer_ttys_lock_leave();
}
PRIVATE void DCALL
buffer_deltty(Buffer *__restrict self) {
 buffer_ttys_lock_enter();
 if (self->fb_ttych.fbl_pself) {
  if ((*self->fb_ttych.fbl_pself = self->fb_ttych.fbl_next) != NULL)
        self->fb_ttych.fbl_next->fb_ttych.fbl_pself = self->fb_ttych.fbl_pself;
  self->fb_ttych.fbl_pself = NULL;
 }
 buffer_ttys_lock_leave();
}


LOCAL int DCALL
buffer_init(Buffer *__restrict self,
            DeeObject *__restrict file,
            uint16_t mode, size_t size) {
 ASSERT_OBJECT(file);
 /* Validate that the given `mode' is an accepted buffer mode. */
 ASSERT((mode&~(FILE_BUFFER_FREADONLY|FILE_BUFFER_FSYNC|FILE_BUFFER_FCLOFILE)) == FILE_BUFFER_MODE_NONE ||
        (mode&~(FILE_BUFFER_FREADONLY|FILE_BUFFER_FSYNC|FILE_BUFFER_FCLOFILE)) == FILE_BUFFER_MODE_FULL ||
        (mode&~(FILE_BUFFER_FREADONLY|FILE_BUFFER_FSYNC|FILE_BUFFER_FCLOFILE)) == FILE_BUFFER_MODE_LINE ||
        (mode&~(FILE_BUFFER_FREADONLY|FILE_BUFFER_FSYNC|FILE_BUFFER_FCLOFILE)) == FILE_BUFFER_MODE_AUTO);

 if ((self->fb_size = size) != 0) {
  /* Allocate the initial buffer. */
  self->fb_base = (uint8_t *)Dee_Malloc(size);
  if unlikely(!self->fb_base) goto err;
 } else {
  self->fb_base = NULL;
 }

#if 1 /* Unwind recursive buffers. */
 if (DeeObject_InstanceOf(file,(DeeTypeObject *)&DeeFileBuffer_Type)) {
  DeeObject *base_file;
  buf_write(file);
  base_file = ((Buffer *)file)->fb_file;
  Dee_XIncref(base_file);
  buf_endwrite(file);
  /* Use the original buffer-file if available. */
  if (base_file)
      file = base_file;
 } else
#endif
 {
  Dee_Incref(file);
 }
#ifndef CONFIG_NO_THREADS
 /* rwlock_init(&self->fo_lock); */
 recursive_rwlock_init(&self->fb_lock);
#endif
 self->fb_file            = file;
 self->fb_ptr             = self->fb_base;
 self->fb_cnt             = 0;
 self->fb_chng            = self->fb_base;
 self->fb_chsz            = 0;
 self->fb_ttych.fbl_pself = NULL;
 self->fb_fblk            = 0;
 self->fb_fpos            = 0;
 self->fb_flag            = mode;
 return 0;
err:
 return -1;
}

#ifdef NDEBUG
PRIVATE uint8_t *DCALL
buffer_realloc_nolock(Buffer *__restrict self, size_t new_size) {
 if (self->fb_flag&FILE_BUFFER_FSTATICBUF)
     return (uint8_t *)Dee_Malloc(new_size);
 ASSERT(!(self->fb_flag&FILE_BUFFER_FREADING));
 return (uint8_t *)Dee_Realloc(self->fb_base,new_size);
}
PRIVATE uint8_t *DCALL
buffer_tryrealloc_nolock(Buffer *__restrict self, size_t new_size) {
 if (self->fb_flag&FILE_BUFFER_FSTATICBUF)
     return (uint8_t *)Dee_TryMalloc(new_size);
 ASSERT(!(self->fb_flag&FILE_BUFFER_FREADING));
 return (uint8_t *)Dee_TryRealloc(self->fb_base,new_size);
}
#else
#define buffer_realloc_nolock(self,new_size) \
        buffer_realloc_nolock_d(self,new_size,__FILE__,__LINE__)
PRIVATE uint8_t *DCALL
buffer_realloc_nolock_d(Buffer *__restrict self, size_t new_size,
                        char const *file, int line) {
 if (self->fb_flag&FILE_BUFFER_FSTATICBUF)
     return (uint8_t *)DeeDbg_Malloc(new_size,file,line);
 ASSERT(!(self->fb_flag&FILE_BUFFER_FREADING));
 return (uint8_t *)DeeDbg_Realloc(self->fb_base,new_size,file,line);
}
#define buffer_tryrealloc_nolock(self,new_size) \
        buffer_tryrealloc_nolock_d(self,new_size,__FILE__,__LINE__)
PRIVATE uint8_t *DCALL
buffer_tryrealloc_nolock_d(Buffer *__restrict self, size_t new_size,
                           char const *file, int line) {
 if (self->fb_flag&FILE_BUFFER_FSTATICBUF)
     return (uint8_t *)DeeDbg_TryMalloc(new_size,file,line);
 ASSERT(!(self->fb_flag&FILE_BUFFER_FREADING));
 return (uint8_t *)DeeDbg_TryRealloc(self->fb_base,new_size,file,line);
}
#endif



PRIVATE int DCALL
buffer_determine_isatty(Buffer *__restrict self) {
 int is_a_tty; DREF DeeObject *file;
 uint16_t flags = self->fb_flag;
 if (flags&(FILE_BUFFER_FNOTATTY|
            FILE_BUFFER_FISATTY)) {
set_lfflag:
  if (flags&FILE_BUFFER_FLNIFTTY) {
   self->fb_flag &= ~FILE_BUFFER_FLNIFTTY;
   /* Set the line-buffered flag if it is a TTY. */
   if (flags&FILE_BUFFER_FISATTY)
       self->fb_flag |= FILE_BUFFER_FLNBUF;
  }
  return 0;
 }
 if unlikely(!self->fb_file) { err_buffer_closed(); goto err; }
 file = self->fb_file,Dee_Incref(file);
 COMPILER_BARRIER();
 is_a_tty = DeeFile_IsAtty(file);
 COMPILER_BARRIER();
 Dee_Decref(file);
 if unlikely(is_a_tty < 0) goto err;
 self->fb_flag |= is_a_tty ? FILE_BUFFER_FISATTY : FILE_BUFFER_FNOTATTY;
 flags          = self->fb_flag;
 goto set_lfflag;
err:
 return -1;
}

/* Construct a new file-buffer.
 * @param: file: The file that is meant to be buffered.
 *               NOTE: If this is another file buffer, its pointed-to
 *                     file is unwound, so-long at it hasn't been closed.
 * @param: mode: One of `FILE_BUFFER_MODE_*'
 * @param: size: The size of the buffer, or ZERO(0)
 *               to allow it to change dynamically. */
PUBLIC DREF DeeObject *DCALL
DeeFileBuffer_New(DeeObject *__restrict file,
                  uint16_t mode, size_t size) {
 DREF Buffer *result;
 result = DeeObject_MALLOC(Buffer);
 if unlikely(!result) goto done;
 /* Initialize the buffer. */
 if unlikely(buffer_init(result,file,mode,size)) goto err_r;
 DeeObject_Init(result,&DeeFileBuffer_Type);
done:
 return (DREF DeeObject *)result;
err_r:
 DeeObject_Free(result);
 return NULL;
}

/* Change the operations mode of a given buffer.
 * @param: mode: One of `FILE_BUFFER_MODE_*'
 * @param: size: The size of the buffer, or ZERO(0)
 *               to allow it to change dynamically. */
PUBLIC int DCALL
DeeFileBuffer_SetMode(DeeObject *__restrict self,
                      uint16_t mode, size_t size) {
 int result = 0; uint8_t *new_buffer;
 Buffer *me = (Buffer *)self;
 ASSERT((mode&~(FILE_BUFFER_FSYNC)) == FILE_BUFFER_MODE_NONE ||
        (mode&~(FILE_BUFFER_FSYNC)) == FILE_BUFFER_MODE_FULL ||
        (mode&~(FILE_BUFFER_FSYNC)) == FILE_BUFFER_MODE_LINE ||
        (mode&~(FILE_BUFFER_FSYNC)) == FILE_BUFFER_MODE_AUTO ||
         mode == FILE_BUFFER_MODE_KEEP);
 ASSERT_OBJECT_TYPE(self,(DeeTypeObject *)&DeeFileBuffer_Type);
 buf_write(me);
 result = buffer_sync_nolock(me,BUFFER_SYNC_FERROR_IF_CLOSED);
 if unlikely(result) goto done;

 /* Set the new buffering mode. */
 if (mode != FILE_BUFFER_MODE_KEEP) {
  me->fb_flag &= ~(FILE_BUFFER_FNODYNSCALE|
                   FILE_BUFFER_FLNBUF|
                   FILE_BUFFER_FLNIFTTY);
  me->fb_flag |= mode;
 }
 me->fb_chsz = 0;
 if (!size) {
  if (!(me->fb_flag&FILE_BUFFER_FNODYNSCALE)) {
   if (me->fb_flag&FILE_BUFFER_FREADING)
       goto err_cannot_resize;
   /* Resize-to-zero. */
   if (!(me->fb_flag&FILE_BUFFER_FSTATICBUF))
         Dee_Free(me->fb_base);
   me->fb_ptr  = (uint8_t *)NULL + (me->fb_ptr-me->fb_base);
   me->fb_cnt  = 0;
   me->fb_size = 0;
   me->fb_base = NULL;
  } else {
   /* Dynamically scaled buffer. */
  }
 } else if (size != me->fb_size) {
  size_t bufoff;
  if (me->fb_flag&FILE_BUFFER_FREADING)
      goto err_cannot_resize;
  new_buffer = buffer_realloc_nolock(me,size);
  if unlikely(!new_buffer) goto err;
  /* Figure out where pointers are located at and update them. */
  bufoff = (size_t)(me->fb_ptr-me->fb_base);
  if (bufoff >= size) {
   /* Clear the available-buffer now that
    * it has been truncated to ZERO(0). */
   me->fb_cnt = 0;
  } else {
   size_t minsiz = (size_t)((me->fb_base+size)-me->fb_ptr);
   /* Truncate the available-buffer. */
   if (me->fb_cnt > minsiz)
       me->fb_cnt = minsiz;
  }
  /* Relocate pointers. */
  me->fb_ptr  = new_buffer+(me->fb_ptr-me->fb_base);
  me->fb_chng = new_buffer;
  me->fb_size = size;
 }
 me->fb_chng = me->fb_base;
done:
 buf_endwrite(me);
 return result;
err_cannot_resize:
err: result = -1; goto done;
}


/* Synchronize unwritten data of all interactive TTY devices.
 * NOTE: The first time a TTY device is changed, this function
 *       is added to the `atexit()' chain unless deemon was
 *       built with the CONFIG_NO_STDLIB option enabled. */
PUBLIC int DCALL DeeFileBuffer_SyncTTYs(void) {
 int result = 0;
 for (;;) {
  DREF Buffer *buffer;
  buffer_ttys_lock_enter();
  buffer = buffer_ttys;
  while (buffer && !Dee_IncrefIfNotZero(buffer))
         buffer = buffer->fb_ttych.fbl_next;
  buffer_ttys_lock_leave();
  if (!buffer) break;
  /* Synchronize this buffer. */
  buf_write(buffer);
  COMPILER_BARRIER();
  result = buffer_sync_nolock(buffer,BUFFER_SYNC_FNORMAL);
  COMPILER_BARRIER();
  buf_endwrite(buffer);
  Dee_Decref(buffer);
  if unlikely(result)
     break;
 }
 return result;
}


PRIVATE dssize_t DCALL
buffer_read_nolock(Buffer *__restrict self,
                   uint8_t *__restrict buffer,
                   size_t bufsize, dioflag_t flags) {
 dssize_t read_size,result = 0; size_t bufavail;
 uint16_t old_flags; dpos_t next_data;
 uint8_t *new_buffer; DREF DeeObject *file;
 bool did_read_data = false;
 if unlikely(!self->fb_file) {
  err_buffer_closed();
  goto err;
 }
again:
 bufavail = self->fb_cnt;
 if likely(bufavail) {
read_from_buffer:
  if (bufavail > bufsize)
      bufavail = bufsize;
  memcpy(buffer,self->fb_ptr,bufavail);
  /* Update buffer pointers. */
  self->fb_cnt -= bufavail;
  self->fb_ptr += bufavail;
  result       += bufavail;
  bufsize      -= bufavail;
  if (!bufsize) goto done;
  if (did_read_data) goto done;
  buffer       += bufavail;
 }
 /* The buffer is empty and must be re-filled. */
 /* First off: Flush any changes that had been made. */
 COMPILER_BARRIER();
 if (buffer_sync_nolock(self,true)) goto err;
 if (buffer_determine_isatty(self)) goto err;
 COMPILER_BARRIER();
 self->fb_chng = self->fb_base;
 self->fb_chsz = 0;
 /* Unlikely: But can happen due to recursive callbacks. */
 if unlikely(self->fb_cnt) goto read_from_buffer;
 if unlikely(!self->fb_file) { err_buffer_closed(); goto err; }

 /* If we're a TTY buffer, flush all other TTY buffers before reading. */
 if (self->fb_flag & FILE_BUFFER_FISATTY) {
  COMPILER_BARRIER();
  if (DeeFileBuffer_SyncTTYs()) goto err;
  COMPILER_BARRIER();
  if unlikely(self->fb_cnt) goto read_from_buffer;
  if unlikely(!self->fb_file) { err_buffer_closed(); goto err; }
  self->fb_chng = self->fb_base;
  self->fb_chsz = 0;
 }

 /* Determine where the next block of data is. */
 next_data = self->fb_fblk+(self->fb_ptr-self->fb_base);

 if unlikely(self->fb_flag&FILE_BUFFER_FNODYNSCALE) {
  /* Dynamic scaling is disabled. Must forward the getc() to the underlying file. */
read_through:
  file = self->fb_file;
  Dee_Incref(file);
  if (next_data != self->fb_fpos) {
   /* Seek in the underlying file to get where we need to go. */
   doff_t new_pos;
   COMPILER_BARRIER();
   new_pos = DeeFile_Seek(file,(doff_t)next_data,SEEK_SET);
   COMPILER_BARRIER();
   Dee_Decref(file);
   if unlikely(new_pos < 0) goto err;
   self->fb_fpos = next_data;
   goto again; /* Must start over because of recursive callbacks. */
  }
  COMPILER_BARRIER();
  read_size = DeeFile_Readf(file,buffer,bufsize,flags);
  COMPILER_BARRIER();
  Dee_Decref(file);
  if unlikely(read_size < 0) goto err;
  self->fb_fpos = next_data+bufsize;
  result += read_size;
  goto done;
 }
 /* If no buffer had been allocated, allocate one now. */
 if unlikely(!self->fb_size) {
  /* Start out with the smallest size. */
  size_t initial_bufsize;
  if (bufsize >= FILE_BUFSIZ_MAX)
      goto read_through;
  initial_bufsize = bufsize;
  if (initial_bufsize < FILE_BUFSIZ_MIN)
      initial_bufsize = FILE_BUFSIZ_MIN;
  new_buffer = buffer_tryrealloc_nolock(self,initial_bufsize);
  if unlikely(!new_buffer) {
   if (Dee_CollectMemory(1))
       goto again;
   goto err;
  }
  self->fb_base = new_buffer;
  self->fb_size = initial_bufsize;
 } else if (bufsize >= self->fb_size) {
  /* The caller want's at least as much as this buffer could even handle.
   * Upscale the buffer, or use load data using read-through mode. */
  if (self->fb_flag&(FILE_BUFFER_FNODYNSCALE|FILE_BUFFER_FREADING))
      goto read_through;
  if (bufsize > FILE_BUFSIZ_MAX)
      goto read_through;
  /* Upscale the buffer. */
  new_buffer = buffer_tryrealloc_nolock(self,bufsize);
  /* If the allocation failed, also use read-through mode. */
  if unlikely(!new_buffer)
     goto read_through;
  self->fb_base = new_buffer;
  self->fb_size = bufsize;
 }

 self->fb_ptr  = self->fb_base;
 self->fb_chng = self->fb_base;
 self->fb_fblk = next_data;
 ASSERT(self->fb_cnt  == 0);
 ASSERT(self->fb_chsz == 0);
 ASSERT(self->fb_size != 0);
 ASSERT(self->fb_file);
 file = self->fb_file;
 Dee_Incref(file);
 if (next_data != self->fb_fpos) {
  /* Seek in the underlying file to get where we need to go. */
  doff_t new_pos;
  COMPILER_BARRIER();
  new_pos = DeeFile_Seek(file,(doff_t)next_data,SEEK_SET);
  COMPILER_BARRIER();
  Dee_Decref(file);
  if unlikely(new_pos < 0) goto err;
  self->fb_fpos = next_data;
  goto again; /* Must start over because of recursive callbacks. */
 }

 /* Actually read the data. */
 new_buffer = self->fb_base;
 old_flags  = self->fb_flag;
 self->fb_flag |= FILE_BUFFER_FREADING;
 COMPILER_BARRIER();
 read_size = DeeFile_Readf(file,self->fb_base,self->fb_size,flags);
 COMPILER_BARRIER();
 Dee_Decref(file);
 self->fb_flag &= ~FILE_BUFFER_FREADING;
 self->fb_flag |= old_flags&FILE_BUFFER_FREADING;
 if unlikely(read_size < 0) goto err;
 if unlikely(read_size == 0) goto done;
 self->fb_fpos = next_data+(size_t)read_size;
 self->fb_ptr  = self->fb_base;
 self->fb_cnt  = (size_t)read_size;
 did_read_data = true;
 goto again;

done:
 return result;
err:
 return -1;
}

PRIVATE dssize_t DCALL
buffer_write_nolock(Buffer *__restrict self,
                    uint8_t const *__restrict buffer,
                    size_t bufsize, dioflag_t flags) {
 dssize_t result = 0; size_t new_bufsize;
 size_t bufavail; uint8_t *new_buffer;
 DREF DeeObject *file;
 if (buffer_determine_isatty(self))
     goto err;
 if (self->fb_flag&FILE_BUFFER_FREADONLY) {
  DeeError_Throwf(&DeeError_NotImplemented,
                  "The buffer hasn't been opened for writing");
  goto err;
 }
again_checkfile:
 if unlikely(!self->fb_file) {
err_closed:
  err_buffer_closed();
  goto err;
 }
again:
 /* Fill available buffer. */
 bufavail = (self->fb_base+self->fb_size)-self->fb_ptr;
 if likely(bufavail) {
  if (bufavail > bufsize)
      bufavail = bufsize;
  if unlikely(!bufavail) goto done;
  memcpy(self->fb_ptr,buffer,bufavail);
  result += bufavail;
  /* Update the changed file-area. */
  if (!self->fb_chsz) {
   self->fb_chng = self->fb_ptr;
   self->fb_chsz = bufavail;
  } else {
   if (self->fb_chng > self->fb_ptr) {
    self->fb_chsz += (size_t)(self->fb_chng-self->fb_ptr);
    self->fb_chng  = self->fb_ptr;
   }
   if (self->fb_chng+self->fb_chsz < self->fb_ptr+bufavail)
       self->fb_chsz = (size_t)((self->fb_ptr+bufavail)-self->fb_chng);
  }
  /* If this is a TTY device, add it to the chain of changed ones. */
  if (self->fb_flag&FILE_BUFFER_FISATTY)
      buffer_addtty(self);

  /* Update the file pointer. */
  self->fb_ptr += bufavail;
  if (self->fb_cnt >= bufavail)
      self->fb_cnt -= bufavail;
  else self->fb_cnt = 0;
  /* When operating in line-buffered mode, check
   * if there was a linefeed in what we just wrote. */
  if ((self->fb_flag&FILE_BUFFER_FLNBUF) &&
      (memchr(buffer,'\n',bufsize) ||
       memchr(buffer,'\r',bufsize))) {
   if ((self->fb_flag & FILE_BUFFER_FISATTY) &&
        DeeFileBuffer_SyncTTYs())
        goto err;
   /* Flush this file. */
   COMPILER_BARRIER();
   if (buffer_sync_nolock(self,BUFFER_SYNC_FERROR_IF_CLOSED))
       goto err;
   COMPILER_BARRIER();
   bufsize -= bufavail;
   if (!bufsize) goto done;
   buffer += bufavail;
   goto again_checkfile;
  }

  bufsize -= bufavail;
  if (!bufsize) goto done;
  buffer += bufavail;
 }
 /* No more buffer available.
  * Either we must flush the buffer, or we must extend it. */
 if (self->fb_size >= FILE_BUFSIZ_MAX ||
    (self->fb_flag&(FILE_BUFFER_FNODYNSCALE|
                    FILE_BUFFER_FREADING))) {
  /* Buffer is too large or cannot be relocated.
   * >> Therefor, we must flush it, then try again. */
  if ((self->fb_flag & FILE_BUFFER_FISATTY) &&
       DeeFileBuffer_SyncTTYs())
       goto err;
  COMPILER_BARRIER();
  if (buffer_sync_nolock(self,BUFFER_SYNC_FERROR_IF_CLOSED))
      goto err;
  COMPILER_BARRIER();
  if unlikely(!self->fb_file) goto err_closed;
  self->fb_chng = self->fb_base;
  self->fb_chsz = 0;
  /* Check for special case: If the buffer is fixed to zero-length,
   *                         we must act as a write-through buffer. */
  if (!self->fb_size) {
   dssize_t temp;
do_writethrough:
   file = self->fb_file;
   Dee_Incref(file);
   COMPILER_BARRIER();
   temp = DeeFile_Writef(file,buffer,bufsize,flags);
   COMPILER_BARRIER();
   Dee_Decref(file);
   if unlikely(temp < 0) goto err;
   result += temp;
   goto done;
  }
  /* If there is no more available buffer to-be read,
   * reset the file pointer and change to the next chunk. */
  if (!self->fb_cnt) {
   self->fb_fblk += (size_t)(self->fb_ptr-self->fb_base);
   self->fb_ptr   = self->fb_base;
  }
  goto again;
 }

 /* Extend the buffer */
 new_bufsize = self->fb_size*2;
 if (new_bufsize < FILE_BUFSIZ_MIN)
     new_bufsize = FILE_BUFSIZ_MIN;
 new_buffer = buffer_tryrealloc_nolock(self,new_bufsize);
 if unlikely(!new_buffer) {
  /* Buffer relocation failed. - sync() + operate in write-through mode as fallback. */
  if ((self->fb_flag & FILE_BUFFER_FISATTY) &&
       DeeFileBuffer_SyncTTYs())
       goto err;
  COMPILER_BARRIER();
  if (buffer_sync_nolock(self,BUFFER_SYNC_FERROR_IF_CLOSED))
      goto err;
  COMPILER_BARRIER();
  if unlikely(!self->fb_file) goto err_closed;
  self->fb_chng = self->fb_base;
  self->fb_chsz = 0;
  goto do_writethrough;
 }
 /* Install the new buffer. */
 self->fb_ptr  = new_buffer+(self->fb_ptr-self->fb_base);
 self->fb_chng = new_buffer+(self->fb_chng-self->fb_base);
 self->fb_size = new_bufsize;
 self->fb_base = new_buffer;
 /* Go back and use the new buffer. */
 goto again;

done:
 return result;
err:
 return -1;
}

PRIVATE doff_t DCALL
buffer_seek_nolock(Buffer *__restrict self,
                   doff_t off, int whence) {
 doff_t result;
 DREF DeeObject *file;
 if (whence == SEEK_SET ||
     whence == SEEK_CUR) {
  dpos_t old_abspos;
  dpos_t new_abspos;
  uint8_t *new_pos;
  if unlikely(!self->fb_file) { err_buffer_closed(); goto err; }
  /* For these modes, we can calculate the new position,
   * allowing for in-buffer seek, as well as delayed seek. */
  old_abspos = self->fb_fblk;
  old_abspos += (self->fb_ptr-self->fb_base);
  if (whence == SEEK_SET)
   new_abspos = (dpos_t)off;
  else {
   new_abspos = old_abspos+off;
  }
  if unlikely(new_abspos >= INT64_MAX) {
   DeeError_Throwf(&DeeError_ValueError,
                   "Invalid seek offset");
   goto err;
  }
  if (new_abspos < old_abspos)
      goto full_seek;
#if __SIZEOF_POINTER__ < 8
  if ((new_abspos-old_abspos) >= SIZE_MAX)
      goto full_seek;
#endif
  /* Seek-ahead-in-buffer. */
  new_pos = self->fb_base+(size_t)(new_abspos-self->fb_fblk);
#if __SIZEOF_POINTER__ < 8
  if (new_pos < self->fb_base) goto full_seek;
#endif
  /* Truncate the read buffer */
  if (new_pos < self->fb_ptr) {
   /* Seek below the current pointer (but we don't
    * remember how much was actually read there, so
    * we simply truncate the buffer fully) */
   self->fb_cnt = 0;
  } else {
   size_t skipsz = (size_t)(new_pos-self->fb_ptr);
   if (self->fb_cnt >= skipsz)
       self->fb_cnt -= skipsz;
   else self->fb_cnt = 0;
  }
  self->fb_ptr = new_pos;
  return (doff_t)new_abspos;
 }
full_seek:
 if (buffer_determine_isatty(self))
     goto err;
 if ((self->fb_flag & FILE_BUFFER_FISATTY) &&
      DeeFileBuffer_SyncTTYs())
      goto err;
 /* Synchronize the buffer. */
 COMPILER_BARRIER();
 if (buffer_sync_nolock(self,BUFFER_SYNC_FERROR_IF_CLOSED))
     goto err;
 COMPILER_BARRIER();
 if unlikely(!self->fb_file) { err_buffer_closed(); goto err; }
 self->fb_chng = self->fb_base;
 self->fb_chsz = 0;
 file = self->fb_file;
 Dee_Incref(file);
 COMPILER_BARRIER();

 /* Do a full seek using the underlying file. */
 result = DeeFile_Seek(file,off,whence);
 Dee_Decref(file);
 if unlikely(result < 0) goto err;
 COMPILER_BARRIER();

 /* Clear the buffer and set the new file pointer. */
 self->fb_fblk = (dpos_t)result;
 self->fb_fpos = (dpos_t)result;
 self->fb_cnt  = 0;
 self->fb_ptr  = self->fb_base;
 self->fb_chng = self->fb_base;
 self->fb_chsz = 0;

 return result;
err:
 return -1;
}

PRIVATE int DCALL
buffer_sync_nolock(Buffer *__restrict self, uint16_t mode) {
 dpos_t abs_chngpos; size_t changed_size;
 dssize_t temp; DREF DeeObject *file;
again:
 if unlikely(!self->fb_file) {
  /* The buffer has been closed. */
  if (!(mode&BUFFER_SYNC_FERROR_IF_CLOSED))
        goto done;
  err_buffer_closed();
  goto err;
 }
 changed_size = self->fb_chsz;
 if (!changed_size) goto done;
 file = self->fb_file;
 Dee_Incref(file);
 abs_chngpos  = self->fb_fblk;
 abs_chngpos += (self->fb_chng-self->fb_base);
 if (abs_chngpos != self->fb_fpos) {
  doff_t new_pos;
  /* Seek to the position where we need to change stuff. */
  COMPILER_BARRIER();
  new_pos = DeeFile_Seek(file,(doff_t)abs_chngpos,SEEK_SET);
  COMPILER_BARRIER();
  Dee_Decref(file);
  if unlikely(new_pos < 0) goto err;
  self->fb_fpos = (dpos_t)new_pos;
  /* Since the buffer may have changed, we need to start over. */
  goto again;
 }
 /* Write all changed data. */
 COMPILER_BARRIER();
 temp = DeeFile_WriteAll(file,
                         self->fb_chng,
                         changed_size);
 COMPILER_BARRIER();
 Dee_Decref(file);
 if unlikely(temp < 0) goto err;
 if (changed_size == self->fb_chsz && self->fb_file &&
     abs_chngpos  == self->fb_fblk+(self->fb_chng-self->fb_base)) {
  /* Data was synchronized. */
  self->fb_chsz  = 0;
  self->fb_fpos += changed_size;
  self->fb_ptr   = self->fb_chng+changed_size;

  /* Remove the buffer from the chain of changed, line-buffered TTYs. */
  buffer_deltty(self);

  /* Also synchronize the underlying file. */
  if ((self->fb_flag&FILE_BUFFER_FSYNC) &&
     !(mode&BUFFER_SYNC_FNOSYNC_FILE)) {
   int error;
   file = self->fb_file;
   Dee_Incref(file);
   COMPILER_BARRIER();
   error = DeeFile_Sync(file);
   COMPILER_BARRIER();
   Dee_Decref(file);
   if (error) goto err;
  }
 }
done:
 return 0;
err:
 return -1;
}

PRIVATE int DCALL
buffer_getc_nolock(Buffer *__restrict self, dioflag_t flags) {
 uint8_t *new_buffer; dssize_t read_size; DREF DeeObject *file;
 uint16_t old_flags; int result; dpos_t next_data;
again:
 if (self->fb_cnt) {
read_from_buffer:
  /* Simple case: we can read from the active buffer. */
  result = (int)(char)*self->fb_ptr++;
  --self->fb_cnt;
  goto done;
 }
 /* The buffer is empty and must be re-filled. */
 /* First off: Flush any changes that had been made. */
 COMPILER_BARRIER();
 if (buffer_sync_nolock(self,BUFFER_SYNC_FERROR_IF_CLOSED)) goto err;
 if (buffer_determine_isatty(self)) goto err;
 COMPILER_BARRIER();
 self->fb_chng = self->fb_base;
 self->fb_chsz = 0;
 /* Unlikely: But can happen due to recursive callbacks. */
 if unlikely(self->fb_cnt) goto read_from_buffer;
 if unlikely(!self->fb_file) { err_buffer_closed(); goto err; }

 /* If we're a TTY buffer, flush all other TTY buffers before reading. */
 if (buffer_determine_isatty(self))
     goto err;
 if (self->fb_flag & FILE_BUFFER_FISATTY) {
  COMPILER_BARRIER();
  if (DeeFileBuffer_SyncTTYs()) goto err;
  COMPILER_BARRIER();
  if unlikely(self->fb_cnt) goto read_from_buffer;
  if unlikely(!self->fb_file) { err_buffer_closed(); goto err; }
  self->fb_chng = self->fb_base;
  self->fb_chsz = 0;
 }

 /* Determine where the next block of data is. */
 next_data = self->fb_fblk+(self->fb_ptr-self->fb_base);

 if unlikely(self->fb_flag&FILE_BUFFER_FNODYNSCALE) {
  /* Dynamic scaling is disabled. Must forward the getc() to the underlying file. */
read_through:
  file = self->fb_file;
  Dee_Incref(file);
  if (next_data != self->fb_fpos) {
   /* Seek in the underlying file to get where we need to go. */
   doff_t new_pos;
   COMPILER_BARRIER();
   new_pos = DeeFile_Seek(file,(doff_t)next_data,SEEK_SET);
   COMPILER_BARRIER();
   Dee_Decref(file);
   if unlikely(new_pos < 0) goto err;
   self->fb_fpos = next_data;
   goto again; /* Must start over because of recursive callbacks. */
  }
  COMPILER_BARRIER();
  result = DeeFile_Getcf(file,flags);
  COMPILER_BARRIER();
  Dee_Decref(file);
  if (result >= 0) {
   /* Set the file and block address. */
   self->fb_fpos = next_data+1;
   self->fb_fblk = next_data+1;
  }
  goto done;
 }

 /* If no buffer had been allocated, allocate one now. */
 if unlikely(!self->fb_size) {
  /* Start out with the smallest size. */
  new_buffer = buffer_tryrealloc_nolock(self,FILE_BUFSIZ_MIN);
  if unlikely(!new_buffer) {
   if (Dee_CollectMemory(1))
       goto again;
   goto err;
  }
  self->fb_base = new_buffer;
  self->fb_size = FILE_BUFSIZ_MIN;
 } else {
  if (self->fb_size < FILE_BUFSIZ_MIN &&
    !(self->fb_flag&(FILE_BUFFER_FNODYNSCALE|FILE_BUFFER_FREADING))) {
   /* Upscale the buffer. */
   new_buffer = buffer_tryrealloc_nolock(self,FILE_BUFSIZ_MIN);
   if unlikely(!new_buffer) goto read_through;
   self->fb_base = new_buffer;
   self->fb_size = FILE_BUFSIZ_MIN;
  }
 }

 self->fb_ptr  = self->fb_base;
 self->fb_chng = self->fb_base;
 self->fb_fblk = next_data;
 ASSERT(self->fb_cnt  == 0);
 ASSERT(self->fb_chsz == 0);
 ASSERT(self->fb_size != 0);
 ASSERT(self->fb_file);
 file = self->fb_file;
 Dee_Incref(file);
 if (next_data != self->fb_fpos) {
  /* Seek in the underlying file to get where we need to go. */
  doff_t new_pos;
  COMPILER_BARRIER();
  new_pos = DeeFile_Seek(file,(doff_t)next_data,SEEK_SET);
  COMPILER_BARRIER();
  Dee_Decref(file);
  if unlikely(new_pos < 0) goto err;
  self->fb_fpos = next_data;
  goto again; /* Must start over because of recursive callbacks. */
 }

 /* Actually read the data. */
 new_buffer = self->fb_base;
 old_flags  = self->fb_flag;
 self->fb_flag |= FILE_BUFFER_FREADING;
 COMPILER_BARRIER();
 read_size = DeeFile_Readf(file,self->fb_base,self->fb_size,flags);
 COMPILER_BARRIER();
 Dee_Decref(file);
 self->fb_flag &= ~FILE_BUFFER_FREADING;
 self->fb_flag |= old_flags&FILE_BUFFER_FREADING;
 if unlikely(read_size < 0) goto err;
 if unlikely(self->fb_cnt) goto read_from_buffer;
 if unlikely(self->fb_chsz) goto again;
 self->fb_fpos = next_data+(size_t)read_size;
 /* Check for special case: EOF reached. */
 if (!read_size)
  result = GETC_EOF;
 else {
  self->fb_cnt = read_size-1;
  self->fb_ptr = self->fb_base+1;
  result = (int)(char)*self->fb_base;
 }
done:
 return result;
err: result = GETC_ERR; goto done;
}

PRIVATE int DCALL
buffer_ungetc_nolock(Buffer *__restrict self, int ch) {
 uint8_t *new_buffer;
 size_t new_bufsize,inc_size;
again:
 /* Simple case: unget() the character. */
 if (self->fb_ptr != self->fb_base)
     goto unget_in_buffer;
 /* The buffer is already full. - Try to resize it, then insert at the front. */
 if (self->fb_flag&FILE_BUFFER_FREADING)
     goto eof;
 if (self->fb_flag&FILE_BUFFER_FNODYNSCALE) {
  /* Check for special case: Even when dynscale is disabled,
   * still allow for an unget buffer of at least a single byte. */
  if (self->fb_size != 0) goto eof;
 }
 /* If the current block cannot be further extended, that's an EOF. */
 if (!self->fb_fblk) goto eof;
 inc_size = self->fb_size;
 /* Determine the minimum buffer size. */
 if unlikely(!inc_size)
    inc_size = (self->fb_flag&FILE_BUFFER_FNODYNSCALE) ? 1 : FILE_BUFSIZ_MIN;
 if (inc_size > self->fb_fblk)
     inc_size = (size_t)self->fb_fblk;
 new_bufsize = self->fb_size+inc_size;
 new_buffer  = buffer_tryrealloc_nolock(self,new_bufsize);
 if unlikely(!new_buffer) {
  inc_size    = 1;
  new_bufsize = self->fb_size+1;
  new_buffer  = buffer_tryrealloc_nolock(self,new_bufsize);
  if unlikely(!new_buffer) {
   if (Dee_CollectMemory(1))
       goto again;
   goto err;
  }
 }
 ASSERT(new_bufsize > self->fb_size);
 /* Install the new buffer. */
 self->fb_ptr   = new_buffer+(self->fb_ptr-self->fb_base)+inc_size;
 if (self->fb_chsz) self->fb_chng = new_buffer+(self->fb_chng-self->fb_base)+inc_size;
 self->fb_fblk -= inc_size;
 self->fb_base  = new_buffer;
 self->fb_size  = new_bufsize;
 ASSERT(self->fb_ptr != self->fb_base);
 /* Finally, insert the character into the buffer. */
unget_in_buffer:
 *--self->fb_ptr = (uint8_t)(char)ch;
 ++self->fb_cnt;
 return 0;
eof: return GETC_EOF;
err: return GETC_ERR;
}

PRIVATE int DCALL
buffer_trunc_nolock(Buffer *__restrict self, dpos_t new_size) {
 dpos_t abs_pos,abs_end; int result;
 DREF DeeObject *file;
 /* Synchronize the buffer. */
 if unlikely(buffer_sync_nolock(self,BUFFER_SYNC_FERROR_IF_CLOSED))
    goto err;
 if unlikely(!self->fb_file) { err_buffer_closed(); goto err; }
 self->fb_chng = self->fb_base;
 self->fb_chsz = 0;
 abs_pos = self->fb_fblk+(self->fb_ptr-self->fb_base);
 abs_end = abs_pos+self->fb_cnt;
 if (new_size < abs_pos) {
  /* Truncate to get rid of the current buffer. */
  self->fb_cnt = 0;
 } else if (new_size < abs_end) {
  /* Truncate the current buffer. */
  self->fb_cnt = (size_t)(new_size-abs_pos);
 }
 /* With data flushed and the loaded buffer
  * truncated, truncate the underlying file. */
 file = self->fb_file;
 Dee_Incref(file);
 COMPILER_BARRIER();
 result = DeeFile_Trunc(file,new_size);
 COMPILER_BARRIER();
 Dee_Decref(file);
 return result;
err:
 return -1;
}


PRIVATE dssize_t DCALL
buffer_read(Buffer *__restrict self,
            void *__restrict buffer,
            size_t bufsize, dioflag_t flags) {
 dssize_t result;
 buf_write(self);
 result = buffer_read_nolock(self,(uint8_t *)buffer,bufsize,flags);
 buf_endwrite(self);
 return result;
}
PRIVATE dssize_t DCALL
buffer_write(Buffer *__restrict self,
             void const *__restrict buffer,
             size_t bufsize, dioflag_t flags) {
 dssize_t result;
 buf_write(self);
 result = buffer_write_nolock(self,(uint8_t const *)buffer,bufsize,flags);
 buf_endwrite(self);
 return result;
}
PRIVATE int DCALL
buffer_sync(Buffer *__restrict self) {
 int result;
 buf_write(self);
 result = buffer_sync_nolock(self,BUFFER_SYNC_FERROR_IF_CLOSED);
 buf_endwrite(self);
 return result;
}
PRIVATE int DCALL
buffer_getc(Buffer *__restrict self, dioflag_t flags) {
 int result;
 buf_write(self);
 result = buffer_getc_nolock(self,flags);
 buf_endwrite(self);
 return result;
}
PRIVATE int DCALL
buffer_ungetc(Buffer *__restrict self, int ch) {
 int result;
 buf_write(self);
 result = buffer_ungetc_nolock(self,ch);
 buf_endwrite(self);
 return result;
}
PRIVATE int DCALL
buffer_trunc(Buffer *__restrict self, dpos_t new_size) {
 int result;
 buf_write(self);
 result = buffer_trunc_nolock(self,new_size);
 buf_endwrite(self);
 return result;
}
PRIVATE doff_t DCALL
buffer_seek(Buffer *__restrict self,
            doff_t off, int whence) {
 doff_t result;
 buf_write(self);
 result = buffer_seek_nolock(self,off,whence);
 buf_endwrite(self);
 return result;
}

PRIVATE int DCALL
buffer_close(Buffer *__restrict self) {
 DREF DeeObject *file; uint8_t *buffer;
 uint16_t flags;
 buf_write(self);
 if unlikely(buffer_sync_nolock(self,BUFFER_SYNC_FERROR_IF_CLOSED))
    goto err;
 file = self->fb_file;
 buffer = self->fb_base;
 flags = self->fb_flag;

 /* Close the underlying file when that flag is set. */
 if ((flags&FILE_BUFFER_FCLOFILE) &&
      DeeFile_Close(file))
      goto err;

 self->fb_file = NULL;

 /* Clear out buffer pointers. */
 self->fb_ptr  = NULL;
 self->fb_cnt  = 0;
 if (flags&FILE_BUFFER_FREADING)
  buffer = NULL;
 else {
  self->fb_base = NULL;
  self->fb_size = 0;
 }
 self->fb_chng = NULL;
 self->fb_chsz = 0;
 if (flags&FILE_BUFFER_FSTATICBUF)
     buffer = NULL;
 self->fb_flag = FILE_BUFFER_FNORMAL|(flags&FILE_BUFFER_FREADING);
 self->fb_fpos = 0;
 buffer_deltty(self);
 buf_endwrite(self);

 Dee_XDecref(file); /* May already be NULL due to recursive callbacks. */
 Dee_Free(buffer);
 return 0;
err:
 buf_endwrite(self);
 return -1;
}


struct mode_name {
    char     name[4]; /* Mode name. */
    uint16_t flag;    /* Mode flags. */
};

PRIVATE struct mode_name const mode_names[] = {
    { { 'k', 'e', 'e', 'p' }, FILE_BUFFER_MODE_KEEP }, /* Must be kept first. */
    { { 'n', 'o', 'n', 'e' }, FILE_BUFFER_MODE_NONE },
    { { 'f', 'u', 'l', 'l' }, FILE_BUFFER_MODE_FULL },
    { { 'l', 'i', 'n', 'e' }, FILE_BUFFER_MODE_LINE },
    { { 'a', 'u', 't', 'o' }, FILE_BUFFER_MODE_AUTO }
};

/* CASEEQ(x,'w') --> x == 'w' || x == 'W' */
#define CASEEQ(x,ch) ((x) == (ch) || (x) == (ch)-('a'-'A'))

PRIVATE int DCALL
buffer_ctor(Buffer *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 uint16_t mode = (FILE_BUFFER_MODE_AUTO);
 DeeObject *file; char const *mode_str = NULL; size_t size = 0;
 if (DeeArg_Unpack(argc,argv,"o|sd:buffer",&file,&mode_str,&size))
     goto err;
 if (mode_str) {
  char const *mode_iter = mode_str;
  unsigned int i;
  union{
   char chrs[4];
   uint32_t id;
  } buf;
  mode = 0;
  /* Interpret the given mode string. */
  for (;;) {
   if (CASEEQ(*mode_iter,'r')) {
    ++mode_iter;
    if (CASEEQ(*mode_iter,'o')) {
     ++mode_iter;
     if (*mode_iter == '-' || *mode_iter == ',')
        ++mode_iter;
    }
    else if (CASEEQ(mode_iter[0],'e') &&
             CASEEQ(mode_iter[1],'a') &&
             CASEEQ(mode_iter[2],'d') &&
             CASEEQ(mode_iter[4],'o') &&
             CASEEQ(mode_iter[5],'n') &&
             CASEEQ(mode_iter[6],'l') &&
             CASEEQ(mode_iter[7],'y') &&
             mode_iter[8] == ',') {
     mode_iter += 9;
    } else {
     goto err_invalid_mode;
    }
    if (mode&FILE_BUFFER_FREADONLY)
        goto err_invalid_mode;
    mode |= FILE_BUFFER_FREADONLY;
    continue;
   }
   if (CASEEQ(*mode_iter,'c')) {
    ++mode_iter;
    if (*mode_iter == '-' || *mode_iter == ',')
        ++mode_iter;
    else if (CASEEQ(mode_iter[0],'l') &&
             CASEEQ(mode_iter[1],'o') &&
             CASEEQ(mode_iter[2],'s') &&
             CASEEQ(mode_iter[3],'e') &&
             mode_iter[4] == ',') {
     mode_iter += 5;
    }
    if (mode&FILE_BUFFER_FCLOFILE)
        goto err_invalid_mode;
    mode |= FILE_BUFFER_FCLOFILE;
    continue;
   }
   if (CASEEQ(*mode_iter,'s')) {
    ++mode_iter;
    if (*mode_iter == '-' || *mode_iter == ',')
        ++mode_iter;
    else if (CASEEQ(mode_iter[0],'y') &&
             CASEEQ(mode_iter[1],'n') &&
             CASEEQ(mode_iter[2],'c') &&
             mode_iter[3] == ',') {
     mode_iter += 4;
    }
    if (mode&FILE_BUFFER_FSYNC)
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
 return buffer_init(self,file,mode,size);
err_invalid_mode:
 DeeError_Throwf(&DeeError_ValueError,
                 "Unrecognized buffer mode `%s'",
                 mode_str);
err:
 return -1;
}

PRIVATE void DCALL
buffer_fini(Buffer *__restrict self) {
 if (self->fb_ttych.fbl_pself) {
#ifndef CONFIG_NO_THREADS
  COMPILER_READ_BARRIER();
  buffer_ttys_lock_enter();
  if (self->fb_ttych.fbl_pself)
#endif
  {
   if ((*self->fb_ttych.fbl_pself = self->fb_ttych.fbl_next) != NULL)
         self->fb_ttych.fbl_next->fb_ttych.fbl_pself = self->fb_ttych.fbl_pself;
  }
  buffer_ttys_lock_leave();
 }
 /* Synchronize the buffer one last time. */
 if unlikely(buffer_sync_nolock(self,BUFFER_SYNC_FNORMAL)) {
  DeeError_Print("Discarding error in buffer synchronization in finalization\n",
                 ERROR_PRINT_DOHANDLE);
 }
 Dee_XDecref(self->fb_file);
 if (!(self->fb_flag&FILE_BUFFER_FSTATICBUF))
       Dee_Free(self->fb_base);
}

PRIVATE void DCALL
buffer_visit(Buffer *__restrict self, dvisit_t proc, void *arg) {
 buf_write(self);
 Dee_XVisit(self->fb_file);
 buf_endwrite(self);
}


PRIVATE DREF DeeObject *DCALL
buffer_size(Buffer *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *file,*result;
 buf_write(self);
 file = self->fb_file;
 if unlikely(!file){
  buf_endwrite(self);
  err_buffer_closed();
  goto err;
 }
 Dee_Incref(file);
 buf_endwrite(self);
 /* Forward the to contained file. */
 result = DeeObject_CallAttr(file,&str_size,argc,argv);
 Dee_Decref(file);
 return result;
err:
 return NULL;
}

#ifndef CONFIG_FILENO_DENY_ARBITRARY_INTEGERS
PRIVATE DREF DeeObject *DCALL
buffer_fileno(Buffer *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *file,*result;
 buf_write(self);
 file = self->fb_file;
 if unlikely(!file){
  buf_endwrite(self);
  err_buffer_closed();
  goto err;
 }
 Dee_Incref(file);
 buf_endwrite(self);
 /* Forward the to contained file. */
 result = DeeObject_CallAttr(file,&str_fileno,argc,argv);
 Dee_Decref(file);
 return result;
err:
 return NULL;
}
#endif

PRIVATE DREF DeeObject *DCALL
buffer_isatty(Buffer *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 int error;
 if (DeeArg_Unpack(argc,argv,":isatty"))
     goto err;
 buf_write(self);
 /* Determine if the buffer points to a TTY. */
 error = buffer_determine_isatty(self);
 buf_endwrite(self);
 if unlikely(error) goto err;
 return_bool(self->fb_flag&FILE_BUFFER_FISATTY);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
buffer_flush(Buffer *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 int error;
 if (DeeArg_Unpack(argc,argv,":flush"))
     goto err;
 buf_write(self);
 /* Synchronize the buffer, but don't synchronize its file. */
 error = buffer_sync_nolock(self,
                            BUFFER_SYNC_FERROR_IF_CLOSED|
                            BUFFER_SYNC_FNOSYNC_FILE);
 buf_endwrite(self);
 if unlikely(error) goto err;
 return_bool(self->fb_flag&FILE_BUFFER_FISATTY);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
buffer_setbuf(Buffer *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 uint16_t mode; char const *mode_iter;
 char const *mode_str; size_t size = 0;
 unsigned int i; union{ char chrs[4]; uint32_t id; } buf;
 if (DeeArg_Unpack(argc,argv,"s|d:setbuf",&mode_str,&size))
     goto err;
 mode_iter = mode_str,mode = 0;
 /* Interpret the given mode string. */
 for (;;) {
  if (CASEEQ(*mode_iter,'s')) {
   ++mode_iter;
   if (*mode_iter == '-' || *mode_iter == ',')
    ++mode_iter;
   else if (CASEEQ(mode_iter[0],'y') &&
            CASEEQ(mode_iter[1],'n') &&
            CASEEQ(mode_iter[2],'c') &&
            mode_iter[3] == ',') {
    mode_iter += 4;
   }
   if (mode&FILE_BUFFER_FSYNC)
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
 if (mode_iter[4]) goto err_invalid_mode;

 /* Parse the main mode name. */
 for (i = 0;; ++i) {
  if (i == COMPILER_LENOF(mode_names))
      goto err_invalid_mode;
  if (*(uint32_t *)mode_names[i].name != buf.id)
      continue;
  mode |= mode_names[i].flag; /* Found it! */
  break;
 }
 if (DeeFileBuffer_SetMode((DeeObject *)self,mode,size))
     goto err;
 return_none;
err_invalid_mode:
 DeeError_Throwf(&DeeError_ValueError,
                 "Unrecognized buffer mode `%s'",
                 mode_str);
err:
 return NULL;
}

PRIVATE struct type_method buffer_methods[] = {
    { "size", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&buffer_size,
      DOC("->?Dint\n"
          "Forward to the $size function of the file being buffered") },
#ifndef CONFIG_FILENO_DENY_ARBITRARY_INTEGERS
    { "fileno", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&buffer_fileno,
      DOC("->?Dint\n"
          "@throw AttributeError The file being buffered does not implement a member function $fileno\n"
          "Forward to the $fileno function of the file being buffered") },
#endif
    { "isatty", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&buffer_isatty,
      DOC("->?Dbool\n"
          "Forward to the $isatty function of the file being buffered\n"
          "Note that in order to implement auto-buffering, file buffers are allowed to "
          "cache the return value of ${this.file.isatty()}, furthermore allowing this "
          "function to simply return that cached value, in other words meaning that "
          "any side-effects caused by the underlying $isatty may not come into effect "
          "following repeated calls") },
    { "flush", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&buffer_flush,
      DOC("()\n"
          "Similar to #sync, but never synchronize the underlying file, regardless "
          "of whether or not $\"nosync\" was passed to the constructor, or #setbuf") },
    { "setbuf", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&buffer_setbuf,
      DOC("(mode:?Dstring,size=!0)\n"
          "@throw ValueError The given @mode is malformed, or not recognized\n"
          "Set the buffering mode of @this buffer to @mode, with a buffer size of @size\n"
          "When non-zero, @size dictates the initial buffer size, or a fixed buffer size when "
          "the buffering mode is set to $\"none\", otherwise a buffer size of $0 allows "
          "the buffer to keep its buffer size implementation-defined, as well as allow it to "
          "dynamically change said size if doing so is deemed useful\n"
          "The given @mode must case-insensitively equal to one of the following:\n"
          "%{table Mode|Description\n"
          "$\"auto\"|Automatically determine buffering, using line-buffering for TTY files, and full buffering for all others\n"
          "$\"full\"|Use full buffering, only synchronizing the buffer when it becomes full\n"
          "$\"line\"|Use line-buffering, automatically synchronizing the buffer whenever a linefeed is written\n"
          "$\"none\"|Use no buffering at all when @size is ${0}, or use a fixed-length buffer of @size bytes. "
                    "Note that even when the no buffer is allocated, the runtime guaranties "
                    "that #ungetc can be used to return and re-read at least 1 byte\n"
          "$\"keep\"|Keep using the old buffering model, but allow the buffer size to be changed}\n"
          "Additionally, @mode may be prefixed with one or more of the following case-insensitive strings:\n"
          "%{table Prefix|Description\n"
          "$\"s\", $\"s-\", $\"s,\", $\"sync,\"|Also invoke #sync on @this buffer's #file whenever the buffer is synced}") },
    { NULL }
};

PRIVATE DREF DeeObject *DCALL
buffer_filename(Buffer *__restrict self) {
 DREF DeeObject *file,*result;
 buf_write(self);
 file = self->fb_file;
 if unlikely(!file){
  buf_endwrite(self);
  err_buffer_closed();
  goto err;
 }
 Dee_Incref(file);
 buf_endwrite(self);
 /* Forward the to contained file. */
 result = DeeObject_GetAttr(file,&str_filename);
 Dee_Decref(file);
 return result;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
buffer_getfile(Buffer *__restrict self) {
 DREF DeeObject *result;
 buf_write(self);
 result = self->fb_file;
 if unlikely(!result){
  buf_endwrite(self);
  err_buffer_closed();
  goto err;
 }
 Dee_Incref(result);
 buf_endwrite(self);
 return result;
err:
 return NULL;
}


PRIVATE struct type_getset buffer_getsets[] = {
    { "file", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&buffer_getfile, NULL, NULL,
       DOC("->?Dfile\nReturns the file that is being buffered") },
    { "filename", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&buffer_filename, NULL, NULL,
       DOC("->?Dstring\nForward the filename attribute of the file being buffered") },
    { NULL }
};


PRIVATE DREF DeeObject *DCALL
buffer_class_sync(DeeObject *__restrict UNUSED(self),
                  size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":sync") ||
     DeeFileBuffer_SyncTTYs())
     return NULL;
#if !defined(CONFIG_HOST_WINDOWS) && \
    !defined(CONFIG_HOST_UNIX) && \
    !defined(CONFIG_NO_STDIO)
 /* When the STDIO filesystem is used, also flush its buffers.
  * Since it doesn't use ours, we need to make sure that its
  * buffer implementation is flushed when the user requests
  * all buffers to be flushed. */
 fflush(NULL);
#endif
 return_none;
}

PRIVATE struct type_method buffer_class_methods[] = {
    { "sync", &buffer_class_sync,
      DOC("()\n"
          "Flush all buffers that are connected to TTY file descriptors\n"
          "This function is automatically called at the same time as :AppExit.atexit, "
          "as well as prior to any read operation on any other TTY buffer that might block") },
    { NULL }
};

PUBLIC DeeFileTypeObject DeeFileBuffer_Type = {
    /* .ft_base = */{
        OBJECT_HEAD_INIT(&DeeFileType_Type),
        /* .tp_name     = */"buffer",
        /* .tp_doc      = */DOC("General-purpose input/output file buffering\n"
                                "\n"
                                "(fp:?Dfile,mode=!Pauto,size=!0)\n"
                                "@throw ValueError The given @mode is malformed, or not recognized\n"
                                "Construct a new buffer for @fp using the given @mode and @size\n"
                                "When non-zero, @size dictates the initial buffer size, or a fixed buffer size when "
                                "the buffering mode is set to $\"none\". Otherwise a buffer size of $0 causes "
                                "the buffer to use an implementation-defined buffer size, as well as allow it to "
                                "dynamically change said size if doing so is deemed useful\n"
                                "The given @mode must case-insensitively equal to one of the following:\n"
                                "%{table Mode|Description\n"
                                "$\"auto\"|Automatically determine buffering, using line-buffering for TTY files, and full buffering for all others\n"
                                "$\"full\"|Use full buffering, only synchronizing the buffer when it becomes full\n"
                                "$\"line\"|Use line-buffering, automatically synchronizing the buffer whenever a linefeed is written\n"
                                "$\"none\"|Use no buffering at all when @size is $0, or use a fixed-length buffer of @size bytes. "
                                          "Note that even when the no buffer is allocated, the runtime guaranties "
                                          "that #ungetc can be used to return and re-read at least 1 byte}\n"
                                "Additionally, @mode may be prefixed with one or more of the following case-insensitive strings:\n"
                                "%{table Prefix|Description\n"
                                "$\"s\", $\"s-\", $\"s,\", $\"sync,\"|Also invoke #sync on the buffer's "
                                                                     "#file whenever the buffer is synced\n"
                                "$\"c\", $\"c-\", $\"c,\", $\"close,\"|When the buffer is closed using ${operator close}, "
                                                                      "also invoke that same operator on the contained file\n"
                                "$\"ro\", $\"ro-\", $\"ro,\", $\"readonly,\"|Disable write-access to the underlying @fp}\n"
                                ">import file from deemon;\n"
                                ">local base_fp = file.open(\"foo.txt\",\"rdonly,nobuf\"); /* Open without buffering */\n"
                                ">local fp = file.buffer(base_fp,\"sync,full\"); /* Create a buffer wrapper */\n"
                                ">fp.write(\"foo\"); /* Write data to buffer */\n"
                                ">fp.sync(); /* Commit data to disk */\n"
                                "The most notably useful feature made possible through use of buffers is "
                                "the possibility of adding #getc and #ungetc support to file types that "
                                "normally wouldn't support the later\n"
                                "During initialization, deemon will initialize the standard "
                                "streams :file.stdin, :file.stdout and :file.stderr as follows:\n"
                                ">import file from deemon;\n"
                                ">file.stdin = file.buffer(file.default_stdin,\"ro,auto\");\n"
                                ">file.stdout = file.buffer(file.default_stdout,\"auto\");\n"
                                ">file.stderr = file.buffer(file.default_stderr,\"auto\");"),
        /* .tp_flags    = */TP_FNORMAL,
        /* .tp_weakrefs = */0,
        /* .tp_features = */TF_HASFILEOPS,
        /* .tp_base     = */(DeeTypeObject *)&DeeFile_Type,
        /* .tp_init = */{
            {
                /* .tp_alloc = */{
                    /* .tp_ctor      = */NULL,
                    /* .tp_copy_ctor = */NULL,
                    /* .tp_deep_ctor = */NULL,
                    /* .tp_any_ctor  = */&buffer_ctor,
                    TYPE_FIXED_ALLOCATOR(Buffer)
                }
            },
            /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&buffer_fini,
            /* .tp_assign      = */NULL,
            /* .tp_move_assign = */NULL
        },
        /* .tp_cast = */{
            /* .tp_str  = */NULL,
            /* .tp_repr = */NULL,
            /* .tp_bool = */NULL
        },
        /* .tp_call          = */NULL,
        /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&buffer_visit,
        /* .tp_gc            = */NULL,
        /* .tp_math          = */NULL,
        /* .tp_cmp           = */NULL,
        /* .tp_seq           = */NULL,
        /* .tp_iter_next     = */NULL,
        /* .tp_attr          = */NULL,
        /* .tp_with          = */NULL,
        /* .tp_buffer        = */NULL,
        /* .tp_methods       = */buffer_methods,
        /* .tp_getsets       = */buffer_getsets,
        /* .tp_members       = */NULL,
        /* .tp_class_methods = */buffer_class_methods,
        /* .tp_class_getsets = */NULL,
        /* .tp_class_members = */NULL
    },
    /* .ft_read   = */(dssize_t(DCALL *)(DeeFileObject *__restrict,void *__restrict,size_t,dioflag_t))&buffer_read,
    /* .ft_write  = */(dssize_t(DCALL *)(DeeFileObject *__restrict,void const *__restrict,size_t,dioflag_t))&buffer_write,
    /* .ft_seek   = */(doff_t(DCALL *)(DeeFileObject *__restrict,doff_t,int))&buffer_seek,
    /* .ft_sync   = */(int(DCALL *)(DeeFileObject *__restrict))&buffer_sync,
    /* .ft_trunc  = */(int(DCALL *)(DeeFileObject *__restrict,dpos_t))&buffer_trunc,
    /* .ft_close  = */(int(DCALL *)(DeeFileObject *__restrict))&buffer_close,
    /* .ft_pread  = */NULL,
    /* .ft_pwrite = */NULL,
    /* .ft_getc   = */(int(DCALL *)(DeeFileObject *__restrict,dioflag_t))&buffer_getc,
    /* .ft_ungetc = */(int(DCALL *)(DeeFileObject *__restrict,int))&buffer_ungetc,
    /* .ft_putc   = */NULL
};


PRIVATE ATTR_COLD void DCALL err_buffer_closed(void) {
 DeeError_Throwf(&DeeError_HandleClosed,
                 "Buffer has been closed");
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_FILEBUFFER_C */
