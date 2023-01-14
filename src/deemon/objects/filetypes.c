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
#ifndef GUARD_DEEMON_OBJECTS_FILETYPES_C
#define GUARD_DEEMON_OBJECTS_FILETYPES_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/int.h>
#include <deemon/mapfile.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* memcpy(), ... */

#include <hybrid/atomic.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "gc_inspect.h"

DECL_BEGIN

typedef DeeMemoryFileObject MemoryFile;
typedef DeeFileReaderObject Reader;
typedef DeeFileWriterObject Writer;

PRIVATE WUNUSED NONNULL((1)) int DCALL
mf_init(MemoryFile *__restrict self) {
	self->mf_begin = NULL;
	self->mf_ptr   = NULL;
	self->mf_end   = NULL;
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
mf_fini(MemoryFile *__restrict self) {
	/* We only get here if `DeeFile_ReleaseMemory()'
	 * was used to duplicate the memory block! */
	Dee_Free((void *)self->mf_begin);
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
mf_read(MemoryFile *__restrict self, void *__restrict buffer,
        size_t bufsize, dioflag_t UNUSED(flags)) {
	size_t result;
	DeeFile_LockWrite(self);
	ASSERT(self->mf_ptr >= self->mf_begin);
	if (self->mf_ptr >= self->mf_end) {
		result = 0;
	} else {
		result = (size_t)(self->mf_end - self->mf_ptr) * sizeof(char);
		if (result > bufsize)
			result = bufsize;
		/* Copy data into the given buffer. */
		memcpy(buffer, self->mf_ptr, result);
		self->mf_ptr = (char *)((uint8_t *)self->mf_ptr + result);
	}
	DeeFile_LockEndWrite(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
mf_pread(MemoryFile *__restrict self, void *__restrict buffer,
         size_t bufsize, dpos_t pos, dioflag_t UNUSED(flags)) {
	size_t result;
	DeeFile_LockRead(self);
	ASSERT(self->mf_ptr >= self->mf_begin);
	result = (size_t)(self->mf_end - self->mf_begin) * sizeof(char);
	if unlikely(pos >= (dpos_t)result) {
		result = 0;
	} else {
		result = result - (size_t)pos;
		if (result > bufsize)
			result = bufsize;
		/* Copy data into the given buffer. */
		memcpy(buffer, self->mf_begin + (size_t)pos, result);
	}
	DeeFile_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) dpos_t DCALL
mf_seek(MemoryFile *__restrict self, doff_t off, int whence) {
	dpos_t result;
	char *new_pointer;
	DeeFile_LockWrite(self);
	ASSERT(self->mf_ptr >= self->mf_begin);
	switch (whence) {

	case SEEK_SET:
		if unlikely(off < 0)
			goto err_invalid;
		if unlikely(self->mf_begin + (size_t)off < self->mf_begin)
			goto err_overflow;
		self->mf_ptr = self->mf_begin + (size_t)off;
		result       = (dpos_t)off;
		break;

	case SEEK_CUR:
		result = (size_t)(self->mf_ptr - self->mf_begin);
		result += (dssize_t)off;
		if unlikely((doff_t)result < 0)
			goto err_invalid;
		new_pointer = self->mf_ptr + (dssize_t)off;
		if unlikely(off > 0 && new_pointer < self->mf_ptr)
			goto err_overflow;
		self->mf_ptr = new_pointer;
		break;

	case SEEK_END:
		result = (size_t)(self->mf_end - self->mf_begin);
		result += (dssize_t)off;
		if unlikely((doff_t)result < 0)
			goto err_invalid;
		new_pointer = self->mf_begin + result;
		if unlikely(new_pointer < self->mf_begin)
			goto err_overflow;
		self->mf_ptr = new_pointer;
		break;

	default:
		DeeFile_LockEndWrite(self);
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid seek mode %d",
		                whence);
		goto err;
	}
	DeeFile_LockEndWrite(self);
	return result;
err_invalid:
	DeeFile_LockEndWrite(self);
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid seek pointer");
	goto err;
err_overflow:
	DeeFile_LockEndWrite(self);
	DeeError_Throwf(&DeeError_IntegerOverflow,
	                "Seek pointer is overflowing");
err:
	return (dpos_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mf_close(MemoryFile *__restrict self) {
	DeeFile_LockWrite(self);
	self->mf_end = self->mf_begin;
	DeeFile_LockEndWrite(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mf_getc(MemoryFile *__restrict self, dioflag_t UNUSED(flags)) {
	int result;
	DeeFile_LockWrite(self);
	ASSERT(self->mf_ptr >= self->mf_begin);
	if unlikely(self->mf_ptr >= self->mf_end) {
		result = GETC_EOF;
	} else {
		result = *self->mf_ptr++;
	}
	DeeFile_LockEndWrite(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mf_ungetc(MemoryFile *__restrict self, int ch) {
	int result;
	DeeFile_LockWrite(self);
	ASSERT(self->mf_ptr >= self->mf_begin);
	if unlikely(self->mf_ptr == self->mf_begin) {
		result = GETC_EOF;
	} else {
#if 0
		if (self->mf_ptr[-1] != (char)ch) {
			DeeFile_LockEndWrite(self);
			DeeError_Throwf(&DeeError_ValueError,
			                "Incorrect character for ungetc()");
			return GETC_ERR;
		}
#endif
		(void)ch;
		/* Rewind the file pointer. */
		--self->mf_ptr;
		result = 0;
	}
	DeeFile_LockEndWrite(self);
	return result;
}


PUBLIC DeeFileTypeObject DeeMemoryFile_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_MemoryFile",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_HASFILEOPS,
		/* .tp_base     = */ (DeeTypeObject *)&DeeFile_Type,
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor      = */ (dfunptr_t)&mf_init,
					/* .tp_copy_ctor = */ (dfunptr_t)NULL,
					/* .tp_deep_ctor = */ (dfunptr_t)NULL,
					/* .tp_any_ctor  = */ (dfunptr_t)NULL,
					TYPE_FIXED_ALLOCATOR(MemoryFile)
				}
			},
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mf_fini,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str  = */ NULL,
			/* .tp_repr = */ NULL,
			/* .tp_bool = */ NULL
		},
		/* .tp_call          = */ NULL,
		/* .tp_visit         = */ NULL,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */ NULL,
		/* .tp_getsets       = */ NULL,
		/* .tp_members       = */ NULL,
		/* .tp_class_methods = */ NULL,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ NULL
	},
	/* .ft_read   = */ (size_t (DCALL *)(DeeFileObject *__restrict, void *__restrict, size_t, dioflag_t))&mf_read,
	/* .ft_write  = */ NULL,
	/* .ft_seek   = */ (dpos_t (DCALL *)(DeeFileObject *__restrict, doff_t, int))&mf_seek,
	/* .ft_sync   = */ NULL,
	/* .ft_trunc  = */ NULL,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *__restrict))&mf_close,
	/* .ft_pread  = */ (size_t (DCALL *)(DeeFileObject *__restrict, void *__restrict, size_t, dpos_t, dioflag_t))&mf_pread,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ (int (DCALL *)(DeeFileObject *__restrict, dioflag_t))&mf_getc,
	/* .ft_ungetc = */ (int (DCALL *)(DeeFileObject *__restrict, int))&mf_ungetc,
	/* .ft_putc   = */ NULL
};

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
PUBLIC WUNUSED NONNULL((1)) DREF /*File*/ DeeObject *DCALL
DeeFile_OpenRoMemory(void const *data, size_t data_size) {
	DREF MemoryFile *result;
	result = DeeObject_MALLOC(MemoryFile);
	if unlikely(!result)
		goto done;
	result->mf_begin = (char *)data;
	result->mf_end   = (char *)data + data_size;
	result->mf_ptr   = result->mf_begin;
	DeeLFileObject_Init(result, &DeeMemoryFile_Type);
done:
	return (DREF DeeObject *)result;
}

PUBLIC NONNULL((1)) void DCALL
DeeFile_ReleaseMemory(DREF /*File*/ DeeObject *__restrict self) {
	MemoryFile *me = (MemoryFile *)self;
	ASSERT_OBJECT_TYPE_EXACT(self, (DeeTypeObject *)&DeeMemoryFile_Type);
	if (!DeeObject_IsShared(me)) {
		/* The file also went away, so we can simply not free its data! */
		Dee_DecrefNokill((DeeObject *)&DeeMemoryFile_Type);
		DeeObject_FreeTracker(me);
		DeeObject_Free(me);
	} else {
		/* Try to copy the data of the file in question. */
		void *data_copy;
		size_t size;
		DeeFile_LockRead(me);
		size = (size_t)(me->mf_end - me->mf_begin);
		DeeFile_LockEndRead(me);
		data_copy = size ? Dee_TryMalloc(size) : NULL;
		COMPILER_READ_BARRIER();
		DeeFile_LockWrite(me);
		if likely(data_copy || !size) {
			/* Copy the stream's data */
			void *data_copy_end;
			data_copy_end = mempcpy(data_copy, me->mf_begin, (size_t)(me->mf_end - me->mf_begin));
			me->mf_end    = (char *)data_copy_end;
			me->mf_ptr    = (char *)data_copy_end;
			me->mf_begin  = (char *)data_copy;
		} else {
			/* Failed to copy data. - Fallback by deleting the
			 * stream's data so it becomes weak undefined behavior... */
			me->mf_begin = NULL;
			me->mf_end   = NULL;
			me->mf_ptr   = NULL;
		}
		DeeFile_LockEndWrite(me);
	}
}





PRIVATE ATTR_COLD int DCALL err_file_closed(void) {
	return DeeError_Throwf(&DeeError_FileClosed,
	                       "File was closed");
}


PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
reader_read(Reader *__restrict self, void *__restrict buffer,
            size_t bufsize, dioflag_t UNUSED(flags)) {
	size_t result;
	DeeFile_LockWrite(self);
	ASSERT(self->r_ptr >= self->r_begin);
	if unlikely(!self->r_owner) {
		DeeFile_LockEndWrite(self);
		return (size_t)err_file_closed();
	}
	if (self->r_ptr >= self->r_end) {
		result = 0;
	} else {
		result = (size_t)(self->r_end - self->r_ptr) * sizeof(char);
		if (result > bufsize)
			result = bufsize;
		/* Copy data into the given buffer. */
		memcpy(buffer, self->r_ptr, result);
		self->r_ptr = (char *)((uint8_t *)self->r_ptr + result);
	}
	DeeFile_LockEndWrite(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
reader_pread(Reader *__restrict self, void *__restrict buffer,
             size_t bufsize, dpos_t pos, dioflag_t UNUSED(flags)) {
	size_t result;
	DeeFile_LockRead(self);
	ASSERT(self->r_ptr >= self->r_begin);
	if unlikely(!self->r_owner) {
		DeeFile_LockEndRead(self);
		return (size_t)err_file_closed();
	}
	result = (size_t)(self->r_end - self->r_begin) * sizeof(char);
	if unlikely(pos >= (dpos_t)result) {
		result = 0;
	} else {
		result = result - (size_t)pos;
		if (result > bufsize)
			result = bufsize;
		/* Copy data into the given buffer. */
		memcpy(buffer, self->r_begin + (size_t)pos, result);
	}
	DeeFile_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) dpos_t DCALL
reader_seek(Reader *__restrict self,
            doff_t off, int whence) {
	dpos_t result;
	char *new_pointer;
	DeeFile_LockWrite(self);
	ASSERT(self->r_ptr >= self->r_begin);
	if unlikely(!self->r_owner) {
		DeeFile_LockEndWrite(self);
		return (dpos_t)err_file_closed();
	}
	switch (whence) {

	case SEEK_SET:
		if unlikely(off < 0)
			goto err_invalid;
		if unlikely(self->r_begin + (size_t)off < self->r_begin)
			goto err_overflow;
		self->r_ptr = self->r_begin + (size_t)off;
		result      = (dpos_t)off;
		break;

	case SEEK_CUR:
		result = (size_t)(self->r_ptr - self->r_begin);
		result += (dssize_t)off;
		if unlikely((doff_t)result < 0)
			goto err_invalid;
		new_pointer = self->r_ptr + (dssize_t)off;
		if unlikely(off > 0 && new_pointer < self->r_ptr)
			goto err_overflow;
		self->r_ptr = new_pointer;
		break;

	case SEEK_END:
		result = (size_t)(self->r_end - self->r_begin);
		result += (dssize_t)off;
		if unlikely((doff_t)result < 0)
			goto err_invalid;
		new_pointer = self->r_begin + result;
		if unlikely(new_pointer < self->r_begin)
			goto err_overflow;
		self->r_ptr = new_pointer;
		break;

	default:
		DeeFile_LockEndWrite(self);
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid seek mode %d",
		                whence);
		goto err;
	}
	DeeFile_LockEndWrite(self);
	return result;
err_invalid:
	DeeFile_LockEndWrite(self);
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid seek pointer");
	goto err;
err_overflow:
	DeeFile_LockEndWrite(self);
	DeeError_Throwf(&DeeError_IntegerOverflow,
	                "Seek pointer is overflowing");
err:
	return (dpos_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
reader_sync(Reader *__restrict self) {
#ifdef CONFIG_NO_THREADS
	if unlikely(!self->r_owner)
#else /* CONFIG_NO_THREADS */
	if unlikely(!ATOMIC_READ(self->r_owner))
#endif /* !CONFIG_NO_THREADS */
	{
		return err_file_closed();
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
reader_close(Reader *__restrict self) {
	DREF DeeObject *owner;
	DeeFile_LockWrite(self);
	ASSERT(self->r_ptr >= self->r_begin);
	/* Extract the string from which data was being read. */
	owner = self->r_owner;
	/* Clear all fields to NULL. */
	self->r_owner = NULL;
	self->r_begin = NULL;
	self->r_ptr   = NULL;
	self->r_end   = NULL;
	DeeFile_LockEndWrite(self);
	/* If the string was already deleted, throw an error. */
	if unlikely(!owner)
		return err_file_closed();
	/* Decref() the string. */
	DeeObject_PutBuf(owner, &self->r_buffer, Dee_BUFFER_FREADONLY);
	Dee_Decref(owner);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
reader_getowner(Reader *__restrict self) {
	DREF DeeObject *result;
	DeeFile_LockRead(self);
	result = self->r_owner;
	Dee_XIncref(result);
	DeeFile_LockEndRead(self);
	if (!result)
		err_file_closed();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
reader_setowner(Reader *__restrict self,
                DeeObject *__restrict value) {
	DeeObject *old_value;
	DeeBuffer new_buffer, old_buffer;
	if (DeeGC_IsReachable((DeeObject *)self, value))
		return err_reference_loop((DeeObject *)self, value);
	if (DeeObject_GetBuf(value, &new_buffer, Dee_BUFFER_FREADONLY))
		goto err;
	Dee_Incref(value);
	DeeFile_LockRead(self);
	old_value     = self->r_owner;
	self->r_owner = value;
	/* Setup pointers to read from the entire string. */
	self->r_begin = (char *)new_buffer.bb_base;
	self->r_ptr   = (char *)new_buffer.bb_base;
	self->r_end   = (char *)new_buffer.bb_base + new_buffer.bb_size;
	memcpy(&old_buffer, &self->r_buffer, sizeof(DeeBuffer));
	memcpy(&self->r_buffer, &new_buffer, sizeof(DeeBuffer));
	DeeFile_LockEndRead(self);
	if (old_value) {
		DeeObject_PutBuf(old_value, &old_buffer, Dee_BUFFER_FREADONLY);
		Dee_Decref(old_value);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
reader_getc(Reader *__restrict self, dioflag_t UNUSED(flags)) {
	int result;
	DeeFile_LockWrite(self);
	ASSERT(self->r_ptr >= self->r_begin);
	if unlikely(!self->r_owner) {
		DeeFile_LockEndWrite(self);
		err_file_closed();
		return GETC_ERR;
	}
	if unlikely(self->r_ptr >= self->r_end) {
		result = GETC_EOF;
	} else {
		result = *self->r_ptr++;
	}
	DeeFile_LockEndWrite(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
reader_ungetc(Reader *__restrict self,
              int ch) {
	int result;
	DeeFile_LockWrite(self);
	ASSERT(self->r_ptr >= self->r_begin);
	if unlikely(!self->r_owner) {
		DeeFile_LockEndWrite(self);
		err_file_closed();
		return GETC_ERR;
	}
	if unlikely(self->r_ptr == self->r_begin) {
		result = GETC_EOF;
	} else {
#if 0
		if (self->r_ptr[-1] != (char)ch) {
			DeeFile_LockEndWrite(self);
			DeeError_Throwf(&DeeError_ValueError,
			                "Incorrect character for ungetc()");
			return GETC_ERR;
		}
#endif
		(void)ch;
		/* Rewind the file pointer. */
		--self->r_ptr;
		result = 0;
	}
	DeeFile_LockEndWrite(self);
	return result;
}


PRIVATE NONNULL((1)) void DCALL
reader_fini(Reader *__restrict self) {
	if (self->r_owner) {
		DeeObject_PutBuf(self->r_owner,
		                 &self->r_buffer,
		                 Dee_BUFFER_FREADONLY);
		Dee_Decref(self->r_owner);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
reader_visit(Reader *__restrict self, dvisit_t proc, void *arg) {
	Dee_XVisit(self->r_owner);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
reader_ctor(Reader *__restrict self) {
	rwlock_init(&self->fo_lock);
	self->r_owner = NULL;
	self->r_begin = NULL;
	self->r_ptr   = NULL;
	self->r_end   = NULL;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
reader_init(Reader *__restrict self,
            size_t argc, DeeObject *const *argv) {
	size_t begin = 0, end = (size_t)-1;
	if (DeeArg_Unpack(argc, argv, "o|" UNPdSIZ UNPdSIZ ":_FileReader", &self->r_owner, &begin, &end))
		goto err;
	if (DeeObject_GetBuf(self->r_owner, &self->r_buffer, Dee_BUFFER_FREADONLY))
		goto err;
	/* Truncate the end-pointer. */
	if (end > self->r_buffer.bb_size)
		end = self->r_buffer.bb_size;
	/* Handle empty read. */
	if unlikely(begin >= end)
		begin = end = 0;
	/* Fill in members. */
	Dee_Incref(self->r_owner);
	rwlock_init(&self->fo_lock);
	self->r_begin = (char *)self->r_buffer.bb_base + begin;
	self->r_end   = (char *)self->r_buffer.bb_base + end;
	self->r_ptr   = self->r_begin;
	return 0;
err:
	return -1;
}

PRIVATE struct type_getset tpconst reader_getsets[] = {
	TYPE_GETSET("owner", &reader_getowner, &reader_close, &reader_setowner,
	            "Assign the object from which data is being read"),
	TYPE_GETSET_END
};


PUBLIC DeeFileTypeObject DeeFileReader_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_FileReader",
		/* .tp_doc      = */ DOC("()\n"
		                         "(s:?DBytes,start=!0,end=!-1)\n"
		                         "Create a file stream for reading data of the given @s as a buffer, "
		                         /**/ "starting at its byte-offset @start and ending at @end\n"
		                         "Note that the given indices @start and @end refer to byte "
		                         /**/ "offsets, and not character offsets, not its preferred encoding"),
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_HASFILEOPS | TF_NONLOOPING,
		/* .tp_base     = */ (DeeTypeObject *)&DeeFile_Type,
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor      = */ (dfunptr_t)&reader_ctor,
					/* .tp_copy_ctor = */ (dfunptr_t)NULL,
					/* .tp_deep_ctor = */ (dfunptr_t)NULL,
					/* .tp_any_ctor  = */ (dfunptr_t)&reader_init,
					TYPE_FIXED_ALLOCATOR(Reader)
				}
			},
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&reader_fini,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str  = */ NULL,
			/* .tp_repr = */ NULL,
			/* .tp_bool = */ NULL
		},
		/* .tp_call          = */ NULL,
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&reader_visit,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */ NULL,
		/* .tp_getsets       = */ reader_getsets,
		/* .tp_members       = */ NULL,
		/* .tp_class_methods = */ NULL,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ NULL
	},
	/* .ft_read   = */ (size_t (DCALL *)(DeeFileObject *__restrict, void *__restrict, size_t, dioflag_t))&reader_read,
	/* .ft_write  = */ NULL,
	/* .ft_seek   = */ (dpos_t (DCALL *)(DeeFileObject *__restrict, doff_t, int))&reader_seek,
	/* .ft_sync   = */ (int (DCALL *)(DeeFileObject *__restrict))&reader_sync,
	/* .ft_trunc  = */ NULL,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *__restrict))&reader_close,
	/* .ft_pread  = */ (size_t (DCALL *)(DeeFileObject *__restrict, void *__restrict, size_t, dpos_t, dioflag_t))&reader_pread,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ (int (DCALL *)(DeeFileObject *__restrict, dioflag_t))&reader_getc,
	/* .ft_ungetc = */ (int (DCALL *)(DeeFileObject *__restrict, int))&reader_ungetc,
	/* .ft_putc   = */ NULL
};

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
PUBLIC WUNUSED NONNULL((1, 2)) DREF /*File*/ DeeObject *DCALL
DeeFile_OpenObjectMemory(DeeObject *__restrict data_owner,
                         void const *data, size_t data_size) {
	DREF Reader *result;
	result = DeeObject_MALLOC(Reader);
	if unlikely(!result)
		goto done;
	Dee_Incref(data_owner);
	result->r_owner = data_owner;
	result->r_begin = (char *)data;
	result->r_end   = (char *)data + data_size;
	result->r_ptr   = (char *)data;
#ifndef __INTELLISENSE__
	result->r_buffer.bb_put = NULL; /* Hide the buffer interface component. */
#endif /* !__INTELLISENSE__ */
	DeeLFileObject_Init(result, &DeeFileReader_Type);
done:
	return (DREF DeeObject *)result;
}

/* Similar to `DeeFile_OpenObjectMemory()', but used
 * to open a generic object using the buffer-interface. */
PUBLIC WUNUSED NONNULL((1)) DREF /*File*/ DeeObject *DCALL
DeeFile_OpenObjectBuffer(DeeObject *__restrict data,
                         size_t begin, size_t end) {
	DREF Reader *result;
	result = DeeObject_MALLOC(Reader);
	if unlikely(!result)
		goto done;
	if (DeeObject_GetBuf(data, &result->r_buffer, Dee_BUFFER_FREADONLY))
		goto err_r;
	/* Truncate the end-pointer. */
	if (end > result->r_buffer.bb_size)
		end = result->r_buffer.bb_size;
	/* Handle empty read. */
	if unlikely(begin > end)
		begin = end;
	/* Fill in members. */
	Dee_Incref(data);
	result->r_owner = data;
	result->r_begin = (char *)result->r_buffer.bb_base + begin;
	result->r_end   = (char *)result->r_buffer.bb_base + end;
	result->r_ptr   = result->r_begin;
	DeeLFileObject_Init(result, &DeeFileReader_Type);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}






PRIVATE /*WUNUSED*/ NONNULL((1)) int DCALL
writer_ctor(Writer *__restrict self) {
	rwlock_init(&self->fo_lock);
	unicode_printer_init(&self->w_printer);
	self->w_string = NULL;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
writer_init(Writer *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeStringObject *init_string;
	if (DeeArg_Unpack(argc, argv, "o:_FileWriter", &init_string))
		goto err;
	if (DeeObject_AssertTypeExact(init_string, &DeeString_Type))
		goto err;
	rwlock_init(&self->fo_lock);
	self->w_printer.up_flags = (uint8_t)DeeString_WIDTH(init_string);
	if (self->w_printer.up_flags == STRING_WIDTH_1BYTE) {
		self->w_string            = NULL;
		self->w_printer.up_buffer = DeeString_STR(init_string);
		self->w_printer.up_length = DeeString_SIZE(init_string);
	} else {
		self->w_string            = init_string;
		self->w_printer.up_buffer = DeeString_WSTR(init_string);
		self->w_printer.up_length = WSTR_LENGTH(self->w_printer.up_buffer);
	}
	Dee_Incref(init_string);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
writer_fini(Writer *__restrict self) {
	if (self->w_string) {
		Dee_Decref(self->w_string);
	} else if (!self->w_printer.up_buffer) {
	} else if ((self->w_printer.up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		Dee_Decref(COMPILER_CONTAINER_OF(self->w_printer.up_buffer,
		                                 DeeStringObject,
		                                 s_str));
	} else {
		unicode_printer_fini(&self->w_printer);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
writer_visit(Writer *__restrict self, dvisit_t proc, void *arg) {
	DeeFile_LockRead(self);
	if (self->w_string) {
		Dee_Visit(self->w_string);
	} else if (!self->w_printer.up_buffer) {
	} else if ((self->w_printer.up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		Dee_Visit(COMPILER_CONTAINER_OF(self->w_printer.up_buffer,
		                                DeeStringObject,
		                                s_str));
	} else {
	}
	DeeFile_LockEndRead(self);
}

/* Returns the current string written by the writer. */
PUBLIC WUNUSED NONNULL((1)) /*string*/ DREF DeeObject *DCALL
DeeFileWriter_GetString(DeeObject *__restrict self) {
	Writer *me = (Writer *)self;
	DREF DeeStringObject *result;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeFileWriter_Type);
again:
	DeeFile_LockRead(me);
	if ((me->w_printer.up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		/* Special case for 1-byte strings. */
		ASSERT(!me->w_string);
		if (!me->w_printer.up_buffer) {
			DeeFile_LockEndRead(me);
			return_empty_string;
		}
		result = COMPILER_CONTAINER_OF(me->w_printer.up_buffer,
		                               DeeStringObject,
		                               s_str);
		if (!DeeObject_IsShared(result)) {
			if (me->w_printer.up_length != result->s_len) {
				/* Flush the string buffer and deallocated unused memory. */
				if (result->s_data) {
					Dee_string_utf_fini(result->s_data, result);
					Dee_string_utf_free(result->s_data);
					result->s_data = NULL;
				}
				result->s_hash = DEE_STRING_HASH_UNSET;
				result->s_len  = me->w_printer.up_length;
				result = (DREF DeeStringObject *)DeeObject_TryRealloc(result,
				                                                      offsetof(DeeStringObject, s_str) +
				                                                      (me->w_printer.up_length + 1) * sizeof(char));
				if unlikely(!result) {
					result = COMPILER_CONTAINER_OF(me->w_printer.up_buffer,
					                               DeeStringObject,
					                               s_str);
				}
				me->w_printer.up_buffer = result->s_str;
			}
			result->s_str[result->s_len] = 0;
		} else {
			/* The string is already being shared, meaning that the must have already been flushed. */
			ASSERT(me->w_printer.up_length == result->s_len);
		}
	} else if (me->w_string) {
		/* A cached multi-byte string has already been set. */
		result = me->w_string;
	} else {
#ifndef CONFIG_NO_THREADS
		if (!atomic_rwlock_upgrade(&me->fo_lock) &&
		    (result = me->w_string) != NULL)
			;
		else
#endif /* !CONFIG_NO_THREADS */
		{
			/* Must pack together a multi-byte string. */
			result = (DREF DeeStringObject *)unicode_printer_trypack(&me->w_printer);
			if unlikely(!result)
				goto err_collect;
			me->w_string           = result;
			me->w_printer.up_flags = (uint8_t)DeeString_WIDTH(result);
			if (me->w_printer.up_flags == STRING_WIDTH_1BYTE) {
				me->w_string            = NULL;
				me->w_printer.up_buffer = DeeString_STR(result);
				me->w_printer.up_length = DeeString_SIZE(result);
			} else {
				me->w_string            = result;
				me->w_printer.up_buffer = DeeString_WSTR(result);
				me->w_printer.up_length = WSTR_LENGTH(me->w_printer.up_buffer);
			}
			atomic_rwlock_downgrade(&me->fo_lock);
		}
	}
	Dee_Incref(result);
	DeeFile_LockEndRead(me);
	ASSERT(DeeString_STR(result)[DeeString_SIZE(result)] == 0);

	return (DREF DeeObject *)result;
err_collect:
	DeeFile_LockEndRead(me);
	if (Dee_CollectMemory((size_t)-1))
		goto again;
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
writer_delstring(Writer *__restrict self) {
	DeeFile_LockWrite(self);
	if (self->w_string) {
		DeeStringObject *old_string;
		old_string     = self->w_string;
		self->w_string = NULL;
		unicode_printer_init(&self->w_printer);
		DeeFile_LockEndWrite(self);
		Dee_Decref(old_string);
	} else if (!self->w_printer.up_buffer) {
		ASSERT(self->w_printer.up_length == 0);
		ASSERT((self->w_printer.up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
		DeeFile_LockEndWrite(self);
	} else if ((self->w_printer.up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		Dee_Decref(COMPILER_CONTAINER_OF(self->w_printer.up_buffer,
		                                 DeeStringObject,
		                                 s_str));
		unicode_printer_init(&self->w_printer);
		DeeFile_LockEndWrite(self);
	} else {
		unicode_printer_fini(&self->w_printer);
		unicode_printer_init(&self->w_printer);
		DeeFile_LockEndWrite(self);
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
writer_setstring(Writer *__restrict self,
                 DeeStringObject *__restrict value) {
	DREF DeeStringObject *old_string;
	struct unicode_printer old_printer;
	if (DeeNone_Check(value))
		goto do_del_string;
	if (DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto err;
	if (DeeString_IsEmpty(value))
		goto do_del_string;
	Dee_Incref(value);
	DeeFile_LockWrite(self);
	memcpy(&old_printer, &self->w_printer, sizeof(struct unicode_printer));
	old_string               = self->w_string;
	self->w_printer.up_flags = (uint8_t)DeeString_WIDTH(value);
	if (self->w_printer.up_flags == STRING_WIDTH_1BYTE) {
		self->w_string            = NULL;
		self->w_printer.up_buffer = DeeString_STR(value);
		self->w_printer.up_length = DeeString_SIZE(value);
	} else {
		self->w_string            = value;
		self->w_printer.up_buffer = DeeString_WSTR(value);
		self->w_printer.up_length = WSTR_LENGTH(self->w_printer.up_buffer);
	}
	DeeFile_LockEndWrite(self);
	if (old_string) {
		Dee_Decref(old_string);
	} else if (!old_printer.up_buffer) {
		ASSERT(old_printer.up_length == 0);
		ASSERT((old_printer.up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
	} else if ((old_printer.up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		Dee_Decref(COMPILER_CONTAINER_OF(old_printer.up_buffer,
		                                 DeeStringObject,
		                                 s_str));
	} else {
		unicode_printer_fini(&old_printer);
	}
	return 0;
do_del_string:
	return writer_delstring(self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
writer_sizeof(Writer *self) {
	size_t result;
	DeeFile_LockRead(self);
	result = sizeof(Writer) + (self->w_printer.up_buffer
	                           ? ((WSTR_LENGTH(self->w_printer.up_buffer) + 1) *
	                              STRING_SIZEOF_WIDTH(self->w_printer.up_flags & UNICODE_PRINTER_FWIDTH))
	                           : 0);
	DeeFile_LockEndRead(self);
	return DeeInt_NewSize(result);
}

PRIVATE struct type_getset tpconst writer_getsets[] = {
	TYPE_GETSET(STR_string, &DeeFileWriter_GetString, &writer_delstring, &writer_setstring,
	            "->?Dstring\n"
	            "Get/set the currently written text string"),
	TYPE_GETTER(STR___sizeof__, &writer_sizeof, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
writer_get(Writer *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":get"))
		goto err;
	return (DREF DeeStringObject *)DeeFileWriter_GetString((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
writer_size(Writer *self, size_t argc, DeeObject *const *argv) {
	size_t result;
	if (DeeArg_Unpack(argc, argv, ":size"))
		goto err;
#ifdef CONFIG_NO_THREADS
	result = self->w_printer.up_length;
#else /* CONFIG_NO_THREADS */
	result = ATOMIC_READ(self->w_printer.up_length);
#endif /* !CONFIG_NO_THREADS */
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
writer_allocated(Writer *self, size_t argc, DeeObject *const *argv) {
	size_t result;
	if (DeeArg_Unpack(argc, argv, ":allocated"))
		goto err;
	DeeFile_LockRead(self);
	result = self->w_printer.up_buffer
	         ? WSTR_LENGTH(self->w_printer.up_buffer)
	         : 0;
	DeeFile_LockEndRead(self);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE struct type_method tpconst writer_methods[] = {
	TYPE_METHOD(STR_get, &writer_get,
	            "->?Dstring\n"
	            "Synchronize and retrieve all data that has already been written"),
	TYPE_METHOD("pack", &writer_get,
	            "->?Dstring\n"
	            "Deprecated alias for reading from ?#string"),
	TYPE_METHOD(STR_size, &writer_size,
	            "->?Dint\n"
	            "Return the total amount of written bytes"),
	TYPE_METHOD("allocated", &writer_allocated,
	            "->?Dint\n"
	            "Returns the currently allocated buffer size (in bytes)"),
	TYPE_METHOD_END
};

#define UNICODE_PRINTER_INITIAL_BUFSIZE 64

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
writer_tryappend8(Writer *__restrict self,
                  uint8_t const *__restrict buffer,
                  size_t bufsize) {
	size_t i, written, avail;
	if (!self->w_printer.up_buffer) {
		/* Allocate the initial buffer. */
		size_t init_size = bufsize;
		DeeStringObject *init_buffer;
		if (init_size < UNICODE_PRINTER_INITIAL_BUFSIZE)
			init_size = UNICODE_PRINTER_INITIAL_BUFSIZE;
		init_buffer = (DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject, s_str) +
		                                                     (init_size + 1) * sizeof(char));
		if unlikely(!init_buffer) {
			init_size   = bufsize;
			init_buffer = (DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject, s_str) +
			                                                     (init_size + 1) * sizeof(char));
			if unlikely(!init_buffer)
				goto err;
		}
		DeeObject_Init(init_buffer, &DeeString_Type);
		init_buffer->s_data = NULL;
		init_buffer->s_hash = DEE_STRING_HASH_UNSET;
		init_buffer->s_len  = init_size;
		memcpy(init_buffer->s_str, buffer, bufsize);
		self->w_printer.up_buffer = init_buffer->s_str;
		self->w_printer.up_length = bufsize;
		goto ok;
	}
	avail   = WSTR_LENGTH(self->w_printer.up_buffer);
	written = self->w_printer.up_length;
	ASSERT(avail >= written);
	SWITCH_SIZEOF_WIDTH(self->w_printer.up_flags & UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE:
		if (written + bufsize > avail) {
			/* Must allocate more memory. */
			DeeStringObject *new_buffer;
			size_t new_size = avail;
			do {
				new_size *= 2;
			} while (new_size < written + bufsize);
			new_buffer = (DeeStringObject *)DeeObject_TryRealloc(COMPILER_CONTAINER_OF(self->w_printer.up_buffer,
			                                                                           DeeStringObject,
			                                                                           s_str),
			                                                     offsetof(DeeStringObject, s_str) +
			                                                     (new_size + 1) * sizeof(char));
			if unlikely(!new_buffer) {
				new_size   = written + bufsize;
				new_buffer = (DeeStringObject *)DeeObject_TryRealloc(COMPILER_CONTAINER_OF(self->w_printer.up_buffer,
				                                                                           DeeStringObject,
				                                                                           s_str),
				                                                     offsetof(DeeStringObject, s_str) +
				                                                     (new_size + 1) * sizeof(char));
				if unlikely(!new_buffer)
					goto err;
			}
			new_buffer->s_len         = new_size;
			self->w_printer.up_buffer = new_buffer->s_str;
		}
		memcpy((uint8_t *)self->w_printer.up_buffer +
		       self->w_printer.up_length,
		       buffer, bufsize);
		self->w_printer.up_length += bufsize;
		break;

	CASE_WIDTH_2BYTE: {
		uint16_t *dst;
		if (written + bufsize > avail) {
			/* Must allocate more memory. */
			uint16_t *new_buffer;
			size_t new_size = avail;
			do {
				new_size *= 2;
			} while (new_size < written + bufsize);
			new_buffer = DeeString_TryResize2ByteBuffer((uint16_t *)self->w_printer.up_buffer, new_size);
			if unlikely(!new_buffer) {
				new_buffer = DeeString_TryResize2ByteBuffer((uint16_t *)self->w_printer.up_buffer, written + bufsize);
				if unlikely(!new_buffer)
					goto err;
			}
			self->w_printer.up_buffer = new_buffer;
		}
		dst = (uint16_t *)self->w_printer.up_buffer;
		dst += self->w_printer.up_length;
		self->w_printer.up_length += bufsize;
		for (i = 0; i < bufsize; ++i)
			*dst++ = buffer[i];
	}	break;

	CASE_WIDTH_4BYTE: {
		uint32_t *dst;
		if (written + bufsize > avail) {
			/* Must allocate more memory. */
			uint32_t *new_buffer;
			size_t new_size = avail;
			do {
				new_size *= 2;
			} while (new_size < written + bufsize);
			new_buffer = DeeString_TryResize4ByteBuffer((uint32_t *)self->w_printer.up_buffer, new_size);
			if unlikely(!new_buffer) {
				new_buffer = DeeString_TryResize4ByteBuffer((uint32_t *)self->w_printer.up_buffer, written + bufsize);
				if unlikely(!new_buffer)
					goto err;
			}
			self->w_printer.up_buffer = new_buffer;
		}
		dst = (uint32_t *)self->w_printer.up_buffer;
		dst += self->w_printer.up_length;
		self->w_printer.up_length += bufsize;
		for (i = 0; i < bufsize; ++i)
			*dst++ = buffer[i];
	}	break;
	}
ok:
	return true;
err:
	return false;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
writer_tryappendch(Writer *__restrict self, uint32_t ch) {
	size_t written, avail;
	if (!self->w_printer.up_buffer) {
		/* Allocate the initial buffer. */
		if (ch <= 0xff) {
			size_t init_size = UNICODE_PRINTER_INITIAL_BUFSIZE;
			DeeStringObject *init_buffer;
			init_buffer = (DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject, s_str) +
			                                                     (init_size + 1) * sizeof(char));
			if unlikely(!init_buffer) {
				init_size   = 1;
				init_buffer = (DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject, s_str) +
				                                                     (init_size + 1) * sizeof(char));
				if unlikely(!init_buffer)
					goto err;
			}
			DeeObject_Init(init_buffer, &DeeString_Type);
			init_buffer->s_data   = NULL;
			init_buffer->s_hash   = DEE_STRING_HASH_UNSET;
			init_buffer->s_len    = init_size;
			init_buffer->s_str[0] = (char)(uint8_t)ch;
			ASSERT((self->w_printer.up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
			self->w_printer.up_buffer = init_buffer->s_str;
			self->w_printer.up_length = 1;
		} else if (ch <= 0xffff) {
			uint16_t *init_buffer;
			init_buffer = DeeString_TryNew2ByteBuffer(UNICODE_PRINTER_INITIAL_BUFSIZE);
			if unlikely(!init_buffer) {
				init_buffer = DeeString_TryNew2ByteBuffer(1);
				if unlikely(!init_buffer)
					goto err;
			}
			init_buffer[0]            = (uint16_t)ch;
			self->w_printer.up_buffer = init_buffer;
			self->w_printer.up_length = 1;
			ASSERT((self->w_printer.up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
			self->w_printer.up_flags |= STRING_WIDTH_2BYTE;
		} else {
			uint32_t *init_buffer;
			init_buffer = DeeString_TryNew4ByteBuffer(UNICODE_PRINTER_INITIAL_BUFSIZE);
			if unlikely(!init_buffer) {
				init_buffer = DeeString_TryNew4ByteBuffer(1);
				if unlikely(!init_buffer)
					goto err;
			}
			init_buffer[0]            = ch;
			self->w_printer.up_buffer = init_buffer;
			self->w_printer.up_length = 1;
			ASSERT((self->w_printer.up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
			self->w_printer.up_flags |= STRING_WIDTH_4BYTE;
		}
		goto ok;
	}
	/* Append to an existing buffer, possibly up-casting that buffer to a greater magnitude. */
	avail   = WSTR_LENGTH(self->w_printer.up_buffer);
	written = self->w_printer.up_length;
	ASSERT(avail >= written);
	SWITCH_SIZEOF_WIDTH(self->w_printer.up_flags & UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE:
		if (ch <= 0xff) {
			if (written >= avail) {
				/* Must allocate more memory. */
				DeeStringObject *new_buffer;
				size_t new_size = avail;
				do {
					new_size *= 2;
				} while (new_size <= written);
				new_buffer = (DeeStringObject *)DeeObject_TryRealloc(COMPILER_CONTAINER_OF(self->w_printer.up_buffer,
				                                                                           DeeStringObject,
				                                                                           s_str),
				                                                     offsetof(DeeStringObject, s_str) +
				                                                     (new_size + 1) * sizeof(char));
				if unlikely(!new_buffer) {
					new_size   = written + 1;
					new_buffer = (DeeStringObject *)DeeObject_TryRealloc(COMPILER_CONTAINER_OF(self->w_printer.up_buffer,
					                                                                           DeeStringObject,
					                                                                           s_str),
					                                                     offsetof(DeeStringObject, s_str) +
					                                                     (new_size + 1) * sizeof(char));
					if unlikely(!new_buffer)
						goto err;
				}
				new_buffer->s_len         = new_size;
				self->w_printer.up_buffer = new_buffer->s_str;
			}
			((uint8_t *)self->w_printer.up_buffer)[self->w_printer.up_length] = (uint8_t)ch;
			++self->w_printer.up_length;
			break;
		}
		if (ch <= 0xffff) {
			/* Must up-cast to 16-bit */
			uint16_t *new_buffer;
			size_t i, length;
			length     = self->w_printer.up_length;
			new_buffer = DeeString_TryNew2ByteBuffer(length + 1);
			if unlikely(!new_buffer)
				goto err;
			for (i = 0; i < length; ++i)
				new_buffer[i] = ((uint8_t *)self->w_printer.up_buffer)[i];
			new_buffer[length] = (uint16_t)ch;
			DeeObject_Free(COMPILER_CONTAINER_OF(self->w_printer.up_buffer,
			                                     DeeStringObject,
			                                     s_str));
			Dee_DecrefNokill(&DeeString_Type);
			self->w_printer.up_buffer = new_buffer;
#if STRING_WIDTH_1BYTE != 0
			self->w_printer.up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
			self->w_printer.up_flags |= STRING_WIDTH_2BYTE;
			++self->w_printer.up_length;
		} else {
			/* Must up-cast to 32-bit */
			uint32_t *new_buffer;
			size_t i, length;
			length     = self->w_printer.up_length;
			new_buffer = DeeString_TryNew4ByteBuffer(length + 1);
			if unlikely(!new_buffer)
				goto err;
			for (i = 0; i < length; ++i)
				new_buffer[i] = ((uint8_t *)self->w_printer.up_buffer)[i];
			new_buffer[length] = (uint32_t)ch;
			DeeObject_Free(COMPILER_CONTAINER_OF(self->w_printer.up_buffer,
			                                     DeeStringObject,
			                                     s_str));
			Dee_DecrefNokill(&DeeString_Type);
			self->w_printer.up_buffer = new_buffer;
#if STRING_WIDTH_1BYTE != 0
			self->w_printer.up_flags &= ~UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
			self->w_printer.up_flags |= STRING_WIDTH_4BYTE;
			++self->w_printer.up_length;
		}
		break;

	CASE_WIDTH_2BYTE:
		if (ch > 0xffff) {
			/* Must up-cast to 32-bit. */
			uint32_t *new_buffer;
			size_t i, length;
			uint16_t *src;
			length     = self->w_printer.up_length;
			new_buffer = DeeString_TryNew4ByteBuffer(length + 1);
			if unlikely(!new_buffer)
				goto err;
			src = (uint16_t *)self->w_printer.up_buffer;
			for (i = 0; i < length; ++i)
				new_buffer[i] = src[i];
			new_buffer[length]        = ch;
			self->w_printer.up_buffer = new_buffer;
			self->w_printer.up_flags &= ~UNICODE_PRINTER_FWIDTH;
			self->w_printer.up_flags |= STRING_WIDTH_4BYTE;
			++self->w_printer.up_length;
			Dee_Free((size_t *)src - 1);
			break;
		}
		if (written >= avail) {
			/* Must allocate more memory. */
			uint16_t *new_buffer;
			size_t new_size = avail;
			do {
				new_size *= 2;
			} while (new_size <= written);
			new_buffer = DeeString_TryResize2ByteBuffer((uint16_t *)self->w_printer.up_buffer, new_size);
			if unlikely(!new_buffer) {
				new_buffer = DeeString_TryResize2ByteBuffer((uint16_t *)self->w_printer.up_buffer, written + 1);
				if unlikely(!new_buffer)
					goto err;
			}
			self->w_printer.up_buffer = new_buffer;
		}
		((uint16_t *)self->w_printer.up_buffer)[self->w_printer.up_length] = (uint16_t)ch;
		++self->w_printer.up_length;
		break;

	CASE_WIDTH_4BYTE:
		if (written >= avail) {
			/* Must allocate more memory. */
			uint32_t *new_buffer;
			size_t new_size = avail;
			do {
				new_size *= 2;
			} while (new_size <= written);
			new_buffer = DeeString_TryResize4ByteBuffer((uint32_t *)self->w_printer.up_buffer, new_size);
			if unlikely(!new_buffer) {
				new_buffer = DeeString_TryResize4ByteBuffer((uint32_t *)self->w_printer.up_buffer, written + 1);
				if unlikely(!new_buffer)
					goto err;
			}
			self->w_printer.up_buffer = new_buffer;
		}
		((uint32_t *)self->w_printer.up_buffer)[self->w_printer.up_length] = ch;
		++self->w_printer.up_length;
		break;
	}
ok:
	return true;
err:
	return false;
}

INTDEF ATTR_PURE WUNUSED NONNULL((1)) uint32_t DCALL
utf8_getchar(uint8_t const *__restrict base, uint8_t seqlen);

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
writer_write(Writer *__restrict self,
             uint8_t const *__restrict buffer,
             size_t bufsize, dioflag_t UNUSED(flags)) {
	size_t result = bufsize;
again:
	DeeFile_LockWrite(self);
	if (self->w_string) {
		DeeStringObject *wstr = self->w_string;
		unsigned int width    = self->w_printer.up_flags & UNICODE_PRINTER_FWIDTH;
		ASSERT(DeeString_WIDTH(wstr) != STRING_WIDTH_1BYTE);
		ASSERT(DeeString_WIDTH(wstr) == width);
		ASSERT(self->w_printer.up_buffer == DeeString_WSTR(wstr));
		ASSERT(self->w_printer.up_length == DeeString_WLEN(wstr));
		if (DeeObject_IsShared(wstr)) {
			/* Unshare the pre-written multi-byte buffer. */
			if (width == STRING_WIDTH_2BYTE) {
				uint16_t *buffer_copy;
				size_t length;
				length      = self->w_printer.up_length;
				buffer_copy = DeeString_TryNew2ByteBuffer(length + bufsize);
				if unlikely(!buffer_copy) {
					DeeFile_LockEndWrite(self);
					if (Dee_CollectMemory(sizeof(size_t) + (length + bufsize + 1) * 2))
						goto again;
					goto err;
				}
				memcpyw(buffer_copy, self->w_printer.up_buffer, length);
				self->w_printer.up_buffer = buffer_copy; /* Inherit data */
			} else {
				uint32_t *buffer_copy;
				size_t length;
				length      = self->w_printer.up_length;
				buffer_copy = DeeString_TryNew4ByteBuffer(length + bufsize);
				if unlikely(!buffer_copy) {
					DeeFile_LockEndWrite(self);
					if (Dee_CollectMemory(sizeof(size_t) + (length + bufsize + 1) * 4))
						goto again;
					goto err;
				}
				memcpyl(buffer_copy, self->w_printer.up_buffer, length);
				self->w_printer.up_buffer = buffer_copy; /* Inherit data */
			}
			/* Drop our reference to the pre-packed string. */
			self->w_string = NULL;
			DeeFile_LockEndWrite(self);
			Dee_Decref_unlikely(wstr);
			goto again;
		} else {
			/* The string isn't being shared, so we can just discard all unused data,
			 * and keep on appending to the pre-generated multi-byte buffer. */
			struct string_utf *utf = wstr->s_data;
			ASSERT(utf != NULL);
			/* WARNING: Just string UTF finalizer that doesn't free width data for `width' */
			if (width == STRING_WIDTH_2BYTE && utf->u_data[STRING_WIDTH_4BYTE]) {
				Dee_Free((size_t *)utf->u_data[STRING_WIDTH_4BYTE] - 1);
			} else if (width == STRING_WIDTH_4BYTE && utf->u_data[STRING_WIDTH_2BYTE]) {
				Dee_Free((size_t *)utf->u_data[STRING_WIDTH_2BYTE] - 1);
				if (utf->u_utf16 &&
				    (uint16_t *)utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE])
					Dee_Free((size_t *)utf->u_utf16 - 1);
			}
			if (utf->u_utf8 && utf->u_utf8 != wstr->s_str)
				Dee_Free((size_t *)utf->u_utf8 - 1);
			Dee_string_utf_free(utf);
			DeeObject_Free(wstr);
			Dee_DecrefNokill(&DeeString_Type);
		}
		self->w_string = NULL;
	} else if (self->w_printer.up_buffer &&
	           (self->w_printer.up_flags & UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		DeeStringObject *written_buffer;
		written_buffer = COMPILER_CONTAINER_OF(self->w_printer.up_buffer,
		                                       DeeStringObject,
		                                       s_str);
		ASSERT(DeeString_WIDTH(written_buffer) == STRING_WIDTH_1BYTE);
		if unlikely(DeeObject_IsShared(written_buffer)) {
			/* Unshare the already written portion of the buffer. */
			DeeStringObject *buffer_copy;
			size_t buffer_length = self->w_printer.up_length;
			ASSERT(buffer_length == DeeString_SIZE(written_buffer));
			buffer_copy = (DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject, s_str) +
			                                                     (buffer_length + bufsize + 1) * sizeof(char));
			if unlikely(!buffer_copy) {
				DeeFile_LockEndWrite(self);
				if (Dee_CollectMemory(offsetof(DeeStringObject, s_str) +
				                      (buffer_length + bufsize + 1) * sizeof(char)))
					goto again;
				goto err;
			}
			DeeObject_Init(buffer_copy, &DeeString_Type);
			buffer_copy->s_len  = buffer_length;
			buffer_copy->s_data = NULL;
			buffer_copy->s_hash = DEE_STRING_HASH_UNSET;
			self->w_printer.up_buffer = (char *)memcpyc(buffer_copy->s_str, written_buffer->s_str,
			                                            self->w_printer.up_length, sizeof(char));
			DeeFile_LockEndWrite(self);
			Dee_Decref_unlikely(written_buffer);
			goto again;
		}
	}
	/* At this point, we know that the buffer has been locked, and that
	 * the pre-written string has been unshared. - Now we can actually
	 * get to work and start appending the new content! */
	if (self->w_printer.up_flags & UNICODE_PRINTER_FPENDING) {
		/* Complete a UTF-8 sequence. */
		uint8_t seqlen = utf8_sequence_len[self->w_printer.up_pend[0]];
		uint8_t gotlen = (self->w_printer.up_flags & UNICODE_PRINTER_FPENDING) >> UNICODE_PRINTER_FPENDING_SHFT;
		uint8_t missing, full_sequence[UTF8_CUR_MBLEN], *tempptr;
		ASSERT(gotlen < seqlen);
		missing = seqlen - gotlen;
		if (missing > bufsize) {
			/* Append what we got, but that won't be all of it... */
			memcpy(self->w_printer.up_pend + gotlen, buffer, bufsize);
			self->w_printer.up_flags += (uint8_t)bufsize << UNICODE_PRINTER_FPENDING_SHFT;
			goto done_unlock;
		}
		/* Complete the sequence, and append the character. */
		tempptr = (uint8_t *)mempcpy(full_sequence, self->w_printer.up_pend, gotlen);
		memcpy(tempptr, buffer, missing);
		if (!writer_tryappendch(self, utf8_getchar(full_sequence, seqlen))) {
			DeeFile_LockEndWrite(self);
			if (Dee_CollectMemory(4))
				goto again;
		}
		self->w_printer.up_flags &= ~UNICODE_PRINTER_FPENDING;
		buffer += missing;
		bufsize -= missing;
	}
	{
		uint8_t *flush_start, *iter, *end;
		end = (flush_start = iter = (uint8_t *)buffer) + bufsize;
		while (iter < end) {
			uint8_t seqlen;
			/* Search for UTF-8 byte sequences */
			if (*iter < 0xc0) {
				++iter;
				continue;
			}
			if (flush_start < iter &&
			    !writer_tryappend8(self, flush_start, (size_t)(iter - flush_start))) {
				DeeFile_LockEndWrite(self);
				buffer  = flush_start;
				bufsize = (size_t)(end - flush_start);
				if (Dee_CollectMemory((size_t)(iter - flush_start)))
					goto again;
				goto err;
			}
			/* Goto a multi-byte sequence. */
			seqlen = utf8_sequence_len[*iter];
			if (seqlen > (size_t)(end - iter)) {
				/* Incomplete sequence (remember the portion already given) */
				seqlen = (uint8_t)(end - iter);
				self->w_printer.up_flags |= seqlen << UNICODE_PRINTER_FPENDING_SHFT;
				memcpy(self->w_printer.up_pend, iter, seqlen);
				goto done_unlock;
			}
			/* The full sequence has been given! */
			if (!writer_tryappendch(self, utf8_getchar(iter, seqlen))) {
				DeeFile_LockEndWrite(self);
				if (!Dee_CollectMemory(4))
					goto err;
				buffer  = iter;
				bufsize = (size_t)(end - iter);
				goto again;
			}
			iter += seqlen;
			flush_start = iter;
		}
		/* Flush the remainder. */
		if (flush_start < end &&
		    !writer_tryappend8(self, flush_start, (size_t)(end - flush_start))) {
			DeeFile_LockEndWrite(self);
			buffer  = flush_start;
			bufsize = (size_t)(end - flush_start);
			if (Dee_CollectMemory(bufsize))
				goto again;
			goto err;
		}
	}
done_unlock:
	DeeFile_LockEndWrite(self);
	return result;
err:
	return (size_t)-1;
}


PUBLIC DeeFileTypeObject DeeFileWriter_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_FileWriter",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_HASFILEOPS | TF_NONLOOPING,
		/* .tp_base     = */ (DeeTypeObject *)&DeeFile_Type,
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor      = */ (dfunptr_t)&writer_ctor,
					/* .tp_copy_ctor = */ (dfunptr_t)NULL,
					/* .tp_deep_ctor = */ (dfunptr_t)NULL,
					/* .tp_any_ctor  = */ (dfunptr_t)&writer_init,
					TYPE_FIXED_ALLOCATOR(Writer)
				}
			},
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&writer_fini,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str  = */ NULL,
			/* .tp_repr = */ NULL,
			/* .tp_bool = */ NULL
		},
		/* .tp_call          = */ NULL,
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&writer_visit,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */ writer_methods,
		/* .tp_getsets       = */ writer_getsets,
		/* .tp_members       = */ NULL,
		/* .tp_class_methods = */ NULL,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ NULL
	},
	/* .ft_read   = */ NULL,
	/* .ft_write  = */ (size_t (DCALL *)(DeeFileObject *__restrict, void const *__restrict, size_t, dioflag_t))&writer_write,
	/* .ft_seek   = */ NULL,
	/* .ft_sync   = */ NULL,
	/* .ft_trunc  = */ NULL,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *__restrict))&writer_delstring,
	/* .ft_pread  = */ NULL,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ NULL,
	/* .ft_ungetc = */ NULL,
	/* .ft_putc   = */ NULL
};

/* Open a new file stream that writes all written data into a string. */
PUBLIC WUNUSED DREF /*File*/ DeeObject *DCALL DeeFile_OpenWriter(void) {
	DREF Writer *result;
	result = DeeObject_MALLOC(Writer);
	if unlikely(!result)
		goto done;
	writer_ctor(result);
	DeeLFileObject_Init(result, &DeeFileWriter_Type);
done:
	return (DREF DeeObject *)result;
}



/************************************************************************/
/* MMAP API                                                             */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1)) int DCALL
mapfile_init_kw(DeeMapFileObject *__restrict self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DeeObject *fd;
	size_t minbytes = (size_t)0;
	size_t maxbytes = (size_t)-1;
	dpos_t offset   = (dpos_t)-1;
	size_t nulbytes = 0;
	bool readall    = false;
	bool mustmmap   = false;
	bool mapshared  = false;
	unsigned int mapflags;
	PRIVATE DEFINE_KWLIST(kwlist, { K(fd), K(minbytes), K(maxbytes), K(offset), K(nulbytes), K(readall), K(mustmmap), K(mapshared), KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist,
	                    "o|" UNPdSIZ UNPdSIZ UNPuN(DEE_SIZEOF_DEE_POS_T) UNPdSIZ "bbb" ":mapfile",
	                    &fd, &minbytes, &maxbytes, &offset, &nulbytes, &readall, &mustmmap, &mapshared))
		goto err;
	mapflags = 0;
	/* Package flags */
	if (readall)
		mapflags |= DEE_MAPFILE_F_READALL;
	if (mustmmap)
		mapflags |= DEE_MAPFILE_F_MUSTMMAP;
	if (mapshared) {
		mapflags |= DEE_MAPFILE_F_MUSTMMAP | DEE_MAPFILE_F_MAPSHARED;
		if (nulbytes != 0) {
			return DeeError_Throwf(&DeeError_ValueError,
			                       "Cannot use `mapshared = true' with non-zero `nulbytes = %Iu'",
			                       nulbytes);
		}
	}

	/* Construct the mapfile. */
	/* TODO: Use DeeNTSystem_GetHandle() + DeeMapFile_InitSysFd(); */
	/* TODO: Use DeeUnixSystem_GetFD() + DeeMapFile_InitSysFd(); */
	if unlikely(DeeMapFile_InitFile(&self->mf_map, fd,
	                                offset, minbytes, maxbytes,
	                                nulbytes, mapflags))
		goto err;
	self->mf_rsize = DeeMapFile_GetSize(&self->mf_map) + nulbytes;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
mapfile_fini(DeeMapFileObject *__restrict self) {
	DeeMapFile_Fini(&self->mf_map);
}

PRIVATE int DCALL
mapfile_getbuf(DeeMapFileObject *__restrict self,
               DeeBuffer *__restrict info,
               unsigned int UNUSED(flags)) {
	info->bb_base = (void *)DeeMapFile_GetBase(&self->mf_map);
	info->bb_size = self->mf_rsize;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapfile_bytes(DeeMapFileObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":bytes"))
		goto err;
	return DeeBytes_NewView((DeeObject *)self,
	                        (void *)DeeMapFile_GetBase(&self->mf_map),
	                        self->mf_rsize, Dee_BUFFER_FWRITABLE);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapfile_get_ismmap(DeeMapFileObject *__restrict self) {
#ifdef DeeMapFile_UsesMmap_IS_ALWAYS_ZERO
	(void)self;
	return_false;
#else /* DeeMapFile_UsesMmap_IS_ALWAYS_ZERO */
	return_bool(DeeMapFile_UsesMmap(&self->mf_map));
#endif /* !DeeMapFile_UsesMmap_IS_ALWAYS_ZERO */
}


PRIVATE struct type_buffer mapfile_buffer = {
	/* .tp_getbuf       = */ (int (DCALL *)(DeeObject *__restrict, DeeBuffer *__restrict, unsigned int))&mapfile_getbuf,
	/* .tp_putbuf       = */ NULL,
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FNORMAL
};

PRIVATE struct type_method tpconst mapfile_methods[] = {
	TYPE_METHOD(STR_bytes, &mapfile_bytes,
	            "->?DBytes\n"
	            "Same as ${Bytes(this)}"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst mapfile_getsets[] = {
	TYPE_GETTER("ismmap", &mapfile_get_ismmap,
	            "->?Dbool\n"
	            "Returns !t if @this file map uses mmap to implement its buffer"),
	TYPE_GETSET_END
};


PUBLIC DeeTypeObject DeeMapFile_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapFile",
	/* .tp_doc      = */ DOC("(fd:?X2?Dint?DFile,maxbytes=!-1,offset=!-1,nulbytes=!0,readall=!f,mustmmap=!f,mapshared=!f)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeMapFileObject),
				/* .tp_any_ctor_kw = */ (dfunptr_t)&mapfile_init_kw
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mapfile_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ &mapfile_buffer,
	/* .tp_methods       = */ mapfile_methods,
	/* .tp_getsets       = */ mapfile_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_FILETYPES_C */
