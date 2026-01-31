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
#ifndef GUARD_DEEMON_OBJECTS_FILETYPES_C
#define GUARD_DEEMON_OBJECTS_FILETYPES_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_*, Dee_CollectMemory, Dee_CollectMemoryoc, Dee_Free, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TryMalloc */
#include <deemon/arg.h>                /* DeeArg_Unpack*, UNP* */
#include <deemon/bool.h>               /* return_bool, return_false */
#include <deemon/bytes.h>              /* DeeBytes* */
#include <deemon/computed-operators.h>
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/file.h>               /* DeeFileObject, DeeFileObject_Init, DeeFileTypeObject, DeeFileType_AsType, DeeFileType_Type, DeeFile_Check, DeeFile_Type, Dee_SEEK_*, Dee_fd_t_IS_HANDLE, Dee_fd_t_IS_int, Dee_ioflag_t, GETC_EOF, GETC_ERR */
#include <deemon/filetypes.h>          /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES, DeeFilePrinterObject, DeeFilePrinter_*, DeeFileReaderObject, DeeFileReader_*, DeeFileWriterObject, DeeFileWriter_*, DeeMemoryFileObject, DeeMemoryFile_*, Dee_FILE_WRITER_HINT_BYTES */
#include <deemon/format.h>             /* DeeFormat_PRINT, DeeFormat_Printf, PRFuSIZ */
#include <deemon/int.h>                /* DeeInt_NewSize */
#include <deemon/mapfile.h>            /* DeeMapFile* */
#include <deemon/none.h>               /* DeeNone_Check, DeeNone_Type, Dee_None */
#include <deemon/object.h>
#include <deemon/serial.h>             /* DeeSerial*, Dee_SERADDR_ISOK, Dee_seraddr_t */
#include <deemon/string.h>             /* CASE_WIDTH_nBYTE, DeeString*, Dee_CASE_WIDTH_nBYTE, Dee_EmptyString, Dee_STRING_*, Dee_UNICODE_PRINTER_*, Dee_charptr, Dee_string_utf*, Dee_unicode_printer*, STRING_ERROR_FSTRICT, STRING_WIDTH_nBYTE, SWITCH_SIZEOF_WIDTH, WSTR_LENGTH */
#include <deemon/stringutils.h>        /* Dee_UNICODE_UTF8_MAXLEN, Dee_unicode_readutf8_chk, Dee_unicode_utf8seqlen */
#include <deemon/system-features.h>    /* bzero, memcmp, memcpy*, mempcpy, strcmp */
#include <deemon/system.h>             /* DeeNTSystem_GetHandle, DeeUnixSystem_GetFD */
#include <deemon/util/atomic.h>        /* atomic_add, atomic_read */
#include <deemon/util/lock.h>          /* Dee_atomic_read_with_atomic_rwlock, Dee_atomic_rwlock_init, Dee_shared_rwlock_init */

#include <hybrid/overflow.h> /* OVERFLOW_UADD */
#include <hybrid/typecore.h> /* __BYTE_TYPE__, __SHIFT_TYPE__, __SIZEOF_SIZE_T__ */

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "gc_inspect.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, ptrdiff_t, size_t */
#include <stdint.h>  /* uint8_t, uint16_t, uint32_t, uintptr_t */

#undef shift_t
#define shift_t __SHIFT_TYPE__

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

PRIVATE WUNUSED NONNULL((1)) int DCALL
mf_init(DeeMemoryFileObject *__restrict self) {
	Dee_atomic_rwlock_init(&self->mf_lock);
	self->mf_begin = NULL;
	self->mf_ptr   = NULL;
	self->mf_end   = NULL;
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
mf_fini(DeeMemoryFileObject *__restrict self) {
	/* We only get here if `DeeMemoryFile_Close()'
	 * was used to duplicate the memory block! */
	Dee_Free((void *)self->mf_begin);
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
mf_read(DeeMemoryFileObject *__restrict self, void *__restrict buffer,
        size_t bufsize, Dee_ioflag_t UNUSED(flags)) {
	size_t result;
	byte_t const *srcptr;
	DeeMemoryFile_LockRead(self);
again_locked:
	srcptr = self->mf_ptr;
	ASSERT(srcptr >= self->mf_begin);
	if (srcptr >= self->mf_end) {
		result = 0;
		DeeMemoryFile_LockEndRead(self);
	} else {
		result = (size_t)(self->mf_end - srcptr);
		if (result > bufsize)
			result = bufsize;

		/* Copy data into the given buffer. */
		memcpy(buffer, srcptr, result);

		if (!DeeMemoryFile_LockUpgrade(self)) {
			if (self->mf_ptr != srcptr) {
				DeeMemoryFile_LockDowngrade(self);
				goto again_locked;
			}
		}
		self->mf_ptr = srcptr + result;
		DeeMemoryFile_LockEndWrite(self);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
mf_pread(DeeMemoryFileObject *__restrict self, void *__restrict buffer,
         size_t bufsize, Dee_pos_t pos, Dee_ioflag_t UNUSED(flags)) {
	size_t result;
	DeeMemoryFile_LockRead(self);
	ASSERT(self->mf_ptr >= self->mf_begin);
	result = (size_t)(self->mf_end - self->mf_begin);
	if unlikely(pos >= (Dee_pos_t)result) {
		result = 0;
	} else {
		result = result - (size_t)pos;
		if (result > bufsize)
			result = bufsize;
		/* Copy data into the given buffer. */
		memcpy(buffer, self->mf_begin + (size_t)pos, result);
	}
	DeeMemoryFile_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_pos_t DCALL
mf_seek(DeeMemoryFileObject *__restrict self, Dee_off_t off, int whence) {
	Dee_pos_t result;
	byte_t const *old_pointer, *new_pointer;
	DeeMemoryFile_LockRead(self);
again_locked:
	old_pointer = self->mf_ptr;
	ASSERT(old_pointer >= self->mf_begin);
	switch (whence) {

	case Dee_SEEK_SET:
		if unlikely(off < 0)
			goto err_invalid;
		if unlikely(self->mf_begin + (size_t)off < self->mf_begin)
			goto err_overflow;
		new_pointer = self->mf_begin + (size_t)off;
		result      = (Dee_pos_t)off;
		break;

	case Dee_SEEK_CUR:
		result = (size_t)(old_pointer - self->mf_begin);
		result += (Dee_ssize_t)off;
		if unlikely((Dee_off_t)result < 0)
			goto err_invalid;
		new_pointer = old_pointer + (Dee_ssize_t)off;
		if unlikely(off > 0 && new_pointer < old_pointer)
			goto err_overflow;
		break;

	case Dee_SEEK_END:
		result = (size_t)(self->mf_end - self->mf_begin);
		result += (Dee_ssize_t)off;
		if unlikely((Dee_off_t)result < 0)
			goto err_invalid;
		new_pointer = self->mf_begin + result;
		if unlikely(new_pointer < self->mf_begin)
			goto err_overflow;
		break;

	default:
		DeeMemoryFile_LockEndRead(self);
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid seek mode %d",
		                whence);
		goto err;
	}
	if (!DeeMemoryFile_LockUpgrade(self)) {
		if (self->mf_ptr != old_pointer) {
			DeeMemoryFile_LockDowngrade(self);
			goto again_locked;
		}
	}
	self->mf_ptr = new_pointer;
	DeeMemoryFile_LockEndWrite(self);
	return result;
err_invalid:
	DeeMemoryFile_LockEndRead(self);
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid seek pointer");
	goto err;
err_overflow:
	DeeMemoryFile_LockEndRead(self);
	DeeError_Throwf(&DeeError_IntegerOverflow,
	                "Seek pointer is overflowing");
err:
	return (Dee_pos_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mf_close(DeeMemoryFileObject *__restrict self) {
	DeeMemoryFile_LockWrite(self);
	self->mf_end = self->mf_begin;
	DeeMemoryFile_LockEndWrite(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mf_getc(DeeMemoryFileObject *__restrict self, Dee_ioflag_t UNUSED(flags)) {
	int result;
	DeeMemoryFile_LockWrite(self);
	ASSERT(self->mf_ptr >= self->mf_begin);
	if unlikely(self->mf_ptr >= self->mf_end) {
		result = GETC_EOF;
	} else {
		result = *self->mf_ptr++;
	}
	DeeMemoryFile_LockEndWrite(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mf_ungetc(DeeMemoryFileObject *__restrict self, int ch) {
	int result;
	DeeMemoryFile_LockWrite(self);
	ASSERT(self->mf_ptr >= self->mf_begin);
	if unlikely(self->mf_ptr == self->mf_begin) {
		result = GETC_EOF;
	} else {
#if 0
		if (self->mf_ptr[-1] != (byte_t)ch) {
			DeeMemoryFile_LockEndWrite(self);
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
	DeeMemoryFile_LockEndWrite(self);
	return result;
}


PUBLIC DeeFileTypeObject DeeMemoryFile_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_MemoryFile",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ &DeeFile_Type.ft_base,
		/* .tp_init = */ {
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
				/* T:              */ DeeMemoryFileObject,
				/* tp_ctor:        */ &mf_init,
				/* tp_copy_ctor:   */ NULL,
				/* tp_deep_ctor:   */ NULL,
				/* tp_any_ctor:    */ NULL,
				/* tp_any_ctor_kw: */ NULL,
				/* tp_serialize:   */ NULL /* Not serializable */
			),
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mf_fini,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str  = */ NULL,
			/* .tp_repr = */ NULL,
			/* .tp_bool = */ NULL
		},
		/* .tp_visit         = */ NULL,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_iterator      = */ NULL,
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
	/* .ft_read   = */ (size_t (DCALL *)(DeeFileObject *__restrict, void *__restrict, size_t, Dee_ioflag_t))&mf_read,
	/* .ft_write  = */ NULL,
	/* .ft_seek   = */ (Dee_pos_t (DCALL *)(DeeFileObject *__restrict, Dee_off_t, int))&mf_seek,
	/* .ft_sync   = */ NULL,
	/* .ft_trunc  = */ NULL,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *__restrict))&mf_close,
	/* .ft_pread  = */ (size_t (DCALL *)(DeeFileObject *__restrict, void *__restrict, size_t, Dee_pos_t, Dee_ioflag_t))&mf_pread,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ (int (DCALL *)(DeeFileObject *__restrict, Dee_ioflag_t))&mf_getc,
	/* .ft_ungetc = */ (int (DCALL *)(DeeFileObject *__restrict, int))&mf_ungetc,
	/* .ft_putc   = */ NULL
};

/* Open a read-only view for raw memory contained within the given data-block.
 * The returned file can be used to access said data in a read-only fashion,
 * however since the data isn't copied, before that data gets freed, you must
 * call `DeeMemoryFile_Close()' to inform the view of this happened, while
 * simultaneously decrementing its reference counter by ONE.
 * `DeeMemoryFile_Close()' will automatically determine the proper course
 * of action, dependent on whether the file is being shared with some other
 * part of deemon. If it is, it will replace the view's data with a heap-allocated
 * copy of that data, and if that isn't possible, modify the view to represent
 * an empty data set. */
PUBLIC WUNUSED NONNULL((1)) DREF /*File*/ DeeObject *DCALL
DeeMemoryFile_New(void const *data, size_t data_size) {
	DREF DeeMemoryFileObject *result;
	result = DeeObject_MALLOC(DeeMemoryFileObject);
	if unlikely(!result)
		goto done;
	result->mf_begin = (byte_t const *)data;
	result->mf_end   = (byte_t const *)data + data_size;
	result->mf_ptr   = result->mf_begin;
	Dee_atomic_rwlock_init(&result->mf_lock);
	DeeFileObject_Init(result, &DeeMemoryFile_Type);
done:
	return Dee_AsObject(result);
}

PUBLIC NONNULL((1)) void DCALL
DeeMemoryFile_Close(DREF /*File*/ DeeObject *__restrict self) {
	DeeMemoryFileObject *me = (DeeMemoryFileObject *)self;
	ASSERT_OBJECT_TYPE_EXACT(self, (DeeTypeObject *)&DeeMemoryFile_Type);
	if (!DeeObject_IsShared(me)) {
		/* The file also went away, so we can simply not free its data! */
		Dee_DecrefNokill(DeeFileType_AsType(&DeeMemoryFile_Type));
		DeeObject_FreeTracker(me);
		DeeObject_Free(me);
	} else {
		/* Try to copy the data of the file in question. */
		void *data_copy;
		size_t size;
		DeeMemoryFile_LockRead(me);
		size = (size_t)(me->mf_end - me->mf_begin);
		DeeMemoryFile_LockEndRead(me);
		data_copy = size ? Dee_TryMalloc(size) : NULL;
		COMPILER_READ_BARRIER();
		DeeMemoryFile_LockWrite(me);
		if likely(data_copy && size == (size_t)(me->mf_end - me->mf_begin)) {
			/* Copy the stream's data */
			me->mf_ptr   = (byte_t const *)data_copy + (me->mf_ptr - me->mf_begin);
			me->mf_begin = (byte_t const *)data_copy;
			me->mf_end   = (byte_t const *)mempcpy(data_copy, me->mf_begin, size);
			data_copy    = NULL;
		} else {
			/* Failed to copy data. - Fallback by deleting the
			 * stream's data so it becomes weak undefined behavior... */
			me->mf_ptr   = (byte_t const *)NULL + (me->mf_ptr - me->mf_begin);
			me->mf_begin = NULL;
			me->mf_end   = NULL;
		}
		DeeMemoryFile_LockEndWrite(me);
		Dee_Free(data_copy);
	}
}





PRIVATE ATTR_COLD int DCALL err_file_closed(void) {
	return DeeError_Throwf(&DeeError_FileClosed,
	                       "File was closed");
}


PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
reader_read(DeeFileReaderObject *__restrict self, void *__restrict buffer,
            size_t bufsize, Dee_ioflag_t UNUSED(flags)) {
	size_t result;
	byte_t const *srcptr;
	DeeFileReader_LockRead(self);
again_locked:
	srcptr = self->r_ptr;
	ASSERT(srcptr >= self->r_begin);
	if unlikely(!self->r_owner) {
		DeeFileReader_LockEndRead(self);
		return (size_t)err_file_closed();
	}
	if (srcptr >= self->r_end) {
		result = 0;
		DeeFileReader_LockEndRead(self);
	} else {
		result = (size_t)(self->r_end - srcptr);
		if (result > bufsize)
			result = bufsize;

		/* Copy data into the given buffer. */
		memcpy(buffer, srcptr, result);
		if (!DeeFileReader_LockUpgrade(self)) {
			if (self->r_ptr != srcptr) {
				DeeFileReader_LockDowngrade(self);
				goto again_locked;
			}
		}
		self->r_ptr = srcptr + result;
		DeeFileReader_LockEndWrite(self);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
reader_pread(DeeFileReaderObject *__restrict self, void *__restrict buffer,
             size_t bufsize, Dee_pos_t pos, Dee_ioflag_t UNUSED(flags)) {
	size_t result;
	DeeFileReader_LockRead(self);
	ASSERT(self->r_ptr >= self->r_begin);
	if unlikely(!self->r_owner) {
		DeeFileReader_LockEndRead(self);
		return (size_t)err_file_closed();
	}
	result = (size_t)(self->r_end - self->r_begin);
	if unlikely(pos >= (Dee_pos_t)result) {
		result = 0;
	} else {
		result = result - (size_t)pos;
		if (result > bufsize)
			result = bufsize;

		/* Copy data into the given buffer. */
		memcpy(buffer, self->r_begin + (size_t)pos, result);
	}
	DeeFileReader_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_pos_t DCALL
reader_seek(DeeFileReaderObject *__restrict self,
            Dee_off_t off, int whence) {
	Dee_pos_t result;
	byte_t const *old_pointer, *new_pointer;
	DeeFileReader_LockRead(self);
again_locked:
	old_pointer = self->r_ptr;
	ASSERT(self->r_ptr >= self->r_begin);
	if unlikely(!self->r_owner) {
		DeeFileReader_LockEndRead(self);
		return (Dee_pos_t)err_file_closed();
	}
	switch (whence) {

	case Dee_SEEK_SET:
		if unlikely(off < 0)
			goto err_invalid;
		if unlikely(self->r_begin + (size_t)off < self->r_begin)
			goto err_overflow;
		new_pointer = self->r_begin + (size_t)off;
		result      = (Dee_pos_t)off;
		break;

	case Dee_SEEK_CUR:
		result = (size_t)(old_pointer - self->r_begin);
		result += (Dee_ssize_t)off;
		if unlikely((Dee_off_t)result < 0)
			goto err_invalid;
		new_pointer = old_pointer + (Dee_ssize_t)off;
		if unlikely(off > 0 && new_pointer < old_pointer)
			goto err_overflow;
		break;

	case Dee_SEEK_END:
		result = (size_t)(self->r_end - self->r_begin);
		result += (Dee_ssize_t)off;
		if unlikely((Dee_off_t)result < 0)
			goto err_invalid;
		new_pointer = self->r_begin + result;
		if unlikely(new_pointer < self->r_begin)
			goto err_overflow;
		break;

	default:
		DeeFileReader_LockEndRead(self);
		DeeError_Throwf(&DeeError_ValueError,
		                "Invalid seek mode %d",
		                whence);
		goto err;
	}
	if (!DeeFileReader_LockUpgrade(self)) {
		if (self->r_ptr != old_pointer) {
			DeeFileReader_LockDowngrade(self);
			goto again_locked;
		}
	}
	self->r_ptr = new_pointer;
	DeeFileReader_LockEndWrite(self);
	return result;
err_invalid:
	DeeFileReader_LockEndRead(self);
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid seek pointer");
	goto err;
err_overflow:
	DeeFileReader_LockEndRead(self);
	DeeError_Throwf(&DeeError_IntegerOverflow,
	                "Seek pointer is overflowing");
err:
	return (Dee_pos_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
reader_sync(DeeFileReaderObject *__restrict self) {
	if unlikely(!Dee_atomic_read_with_atomic_rwlock(&self->r_owner, &self->r_lock)) {
		return err_file_closed();
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
reader_close(DeeFileReaderObject *__restrict self) {
	DREF DeeObject *owner;
	DeeFileReader_LockWrite(self);
	ASSERT(self->r_ptr >= self->r_begin);

	/* Extract the string from which data was being read. */
	owner = self->r_owner;

	/* Clear all fields to NULL. */
	self->r_owner = NULL;
	self->r_begin = NULL;
	self->r_ptr   = NULL;
	self->r_end   = NULL;
	DeeFileReader_LockEndWrite(self);

	/* If the string was already deleted, throw an error. */
	if unlikely(!owner)
		return err_file_closed();

	/* Decref() the string. */
	DeeBuffer_Fini(&self->r_buffer);
	Dee_Decref(owner);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
reader_getowner(DeeFileReaderObject *__restrict self) {
	DREF DeeObject *result;
	DeeFileReader_LockRead(self);
	result = self->r_owner;
	Dee_XIncref(result);
	DeeFileReader_LockEndRead(self);
	if (!result)
		err_file_closed();
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
reader_setowner(DeeFileReaderObject *__restrict self,
                DeeObject *__restrict value) {
	DeeObject *old_value;
	DeeBuffer new_buffer, old_buffer;
	if (DeeGC_IsReachable(Dee_AsObject(self), value))
		return err_reference_loop(Dee_AsObject(self), value);
	if (DeeObject_GetBuf(value, &new_buffer, Dee_BUFFER_FREADONLY))
		goto err;
	Dee_Incref(value);
	DeeFileReader_LockWrite(self);
	old_value     = self->r_owner;
	self->r_owner = value;

	/* Setup pointers to read from the entire string. */
	self->r_begin = (byte_t const *)new_buffer.bb_base;
	self->r_ptr   = (byte_t const *)new_buffer.bb_base;
	self->r_end   = (byte_t const *)new_buffer.bb_base + new_buffer.bb_size;
	memcpy(&old_buffer, &self->r_buffer, sizeof(DeeBuffer));
	memcpy(&self->r_buffer, &new_buffer, sizeof(DeeBuffer));
	DeeFileReader_LockEndWrite(self);
	if (old_value) {
		DeeBuffer_Fini(&old_buffer);
		Dee_Decref(old_value);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
reader_getc(DeeFileReaderObject *__restrict self, Dee_ioflag_t UNUSED(flags)) {
	int result;
	DeeFileReader_LockWrite(self);
	ASSERT(self->r_ptr >= self->r_begin);
	if unlikely(!self->r_owner) {
		DeeFileReader_LockEndWrite(self);
		err_file_closed();
		return GETC_ERR;
	}
	if unlikely(self->r_ptr >= self->r_end) {
		result = GETC_EOF;
	} else {
		result = *self->r_ptr++;
	}
	DeeFileReader_LockEndWrite(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
reader_ungetc(DeeFileReaderObject *__restrict self,
              int ch) {
	int result;
	DeeFileReader_LockWrite(self);
	ASSERT(self->r_ptr >= self->r_begin);
	if unlikely(!self->r_owner) {
		DeeFileReader_LockEndWrite(self);
		err_file_closed();
		return GETC_ERR;
	}
	if unlikely(self->r_ptr == self->r_begin) {
		result = GETC_EOF;
	} else {
#if 0
		if (self->r_ptr[-1] != (byte_t)ch) {
			DeeFileReader_LockEndWrite(self);
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
	DeeFileReader_LockEndWrite(self);
	return result;
}


PRIVATE NONNULL((1)) void DCALL
reader_fini(DeeFileReaderObject *__restrict self) {
	if (self->r_owner) {
		DeeBuffer_Fini(&self->r_buffer);
		Dee_Decref(self->r_owner);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
reader_visit(DeeFileReaderObject *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_XVisit(self->r_owner);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
reader_ctor(DeeFileReaderObject *__restrict self) {
	Dee_atomic_rwlock_init(&self->r_lock);
	self->r_owner = NULL;
	self->r_begin = NULL;
	self->r_ptr   = NULL;
	self->r_end   = NULL;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
reader_init_kw(DeeFileReaderObject *__restrict self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("_FileReader", params: "
	data:?DBytes;
	size_t start = 0;
	size_t end = (size_t)-1;
	size_t pos = 0;
", docStringPrefix: "reader");]]]*/
#define reader__FileReader_params "data:?DBytes,start=!0,end=!-1,pos=!0"
	struct {
		DeeObject *data;
		size_t start;
		size_t end;
		size_t pos;
	} args;
	args.start = 0;
	args.end = (size_t)-1;
	args.pos = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__data_start_end_pos, "o|" UNPuSIZ UNPxSIZ UNPuSIZ ":_FileReader", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_GetBuf(args.data, &self->r_buffer, Dee_BUFFER_FREADONLY))
		goto err;

	/* Truncate the end-pointer. */
	if (args.end > self->r_buffer.bb_size)
		args.end = self->r_buffer.bb_size;

	/* Handle empty read. */
	if unlikely(args.start >= args.end)
		args.start = args.end = 0;

	/* Fill in members. */
	Dee_Incref(args.data);
	self->r_owner = args.data;
	Dee_atomic_rwlock_init(&self->r_lock);
	self->r_begin = (byte_t const *)self->r_buffer.bb_base + args.start;
	self->r_end   = (byte_t const *)self->r_buffer.bb_base + args.end;
	if (OVERFLOW_UADD((uintptr_t)self->r_begin, args.pos, (uintptr_t *)&self->r_ptr))
		self->r_ptr = (byte_t const *)(uintptr_t)-1l;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
reader_serialize(DeeFileReaderObject *__restrict self,
                 DeeSerial *__restrict writer,
                 Dee_seraddr_t addr) {
	DeeFileReaderObject *out;
	byte_t const *self__r_begin;
	byte_t const *self__r_ptr;
	byte_t const *self__r_end;
	DREF DeeObject *self__r_owner;
	DeeBuffer self__r_buffer;
	DeeFileReader_LockRead(self);
	self__r_owner = self->r_owner;
	if (!self__r_owner) {
		DeeFileReader_LockEndRead(self);
		out = DeeSerial_Addr2Mem(writer, addr, DeeFileReaderObject);
		out->r_begin = NULL;
		out->r_ptr   = NULL;
		out->r_end   = NULL;
		out->r_owner = NULL;
		out->r_buffer.bb_base = NULL;
		out->r_buffer.bb_size = 0;
	} else {
		Dee_Incref(self__r_owner);
		self__r_begin  = self->r_begin;
		self__r_ptr    = self->r_ptr;
		self__r_end    = self->r_end;
		self__r_buffer = self->r_buffer;
		DeeFileReader_LockEndRead(self);
#define ADDROF(field) (addr + offsetof(DeeFileReaderObject, field))
		if (DeeSerial_PutObjectInherited(writer, ADDROF(r_owner), self__r_owner))
			goto err;
		if (DeeSerial_PutPointer(writer, ADDROF(r_begin), self__r_begin))
			goto err;
		if (DeeSerial_PutPointer(writer, ADDROF(r_ptr), self__r_ptr))
			goto err;
		if (DeeSerial_PutPointer(writer, ADDROF(r_end), self__r_end))
			goto err;
		if (DeeSerial_PutPointer(writer, ADDROF(r_buffer.bb_base), self__r_buffer.bb_base))
			goto err;
#undef ADDROF
		out = DeeSerial_Addr2Mem(writer, addr, DeeFileReaderObject);
		out->r_buffer.bb_size = self__r_buffer.bb_size;
	}
	Dee_atomic_rwlock_init(&out->r_lock);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) Dee_ssize_t DCALL
reader_printrepr(DeeFileReaderObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result, temp;
	size_t start = (size_t)(self->r_begin - (byte_t const *)self->r_buffer.bb_base);
	size_t end   = (size_t)(self->r_end - (byte_t const *)self->r_buffer.bb_base);
	size_t pos   = (size_t)(self->r_ptr - self->r_begin);
	ASSERT(start <= end);
	ASSERT(end <= self->r_buffer.bb_size);
	result = DeeFormat_Printf(printer, arg, "File.Reader(%r", self->r_owner);
	if likely(result < 0)
		goto done;
	if (start > 0)
		DO(err, DeeFormat_Printf(printer, arg, ", start: %" PRFuSIZ, start));
	if (end < self->r_buffer.bb_size)
		DO(err, DeeFormat_Printf(printer, arg, ", end: %" PRFuSIZ, end));
	if (pos > 0)
		DO(err, DeeFormat_Printf(printer, arg, ", pos: %" PRFuSIZ, pos));
	DO(err, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err:
	return temp;
}

PRIVATE struct type_getset tpconst reader_getsets[] = {
	TYPE_GETSET_F("owner", &reader_getowner, &reader_close, &reader_setowner, METHOD_FNOREFESCAPE,
	              "Assign the object from which data is being read"),
	TYPE_GETSET_END
};


PUBLIC DeeFileTypeObject DeeFileReader_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_FileReader",
		/* .tp_doc      = */ DOC("()\n"
		                         "(" reader__FileReader_params ")\n"
		                         "Create a file stream for reading data of the given @data as a buffer, "
		                         /**/ "starting at its byte-offset @start and ending at @end\n"
		                         "Note that the given indices @start and @end refer to byte "
		                         /**/ "offsets (which may not necessarily equal character offsets)"),
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONLOOPING,
		/* .tp_base     = */ &DeeFile_Type.ft_base,
		/* .tp_init = */ {
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
				/* T:              */ DeeFileReaderObject,
				/* tp_ctor:        */ &reader_ctor,
				/* tp_copy_ctor:   */ NULL,
				/* tp_deep_ctor:   */ NULL,
				/* tp_any_ctor:    */ NULL,
				/* tp_any_ctor_kw: */ &reader_init_kw,
				/* tp_serialize:   */ &reader_serialize
			),
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&reader_fini,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str       = */ NULL,
			/* .tp_repr      = */ NULL,
			/* .tp_bool      = */ NULL,
			/* .tp_print     = */ NULL,
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&reader_printrepr,
		},
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&reader_visit,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_iterator      = */ NULL,
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
	/* .ft_read   = */ (size_t (DCALL *)(DeeFileObject *__restrict, void *__restrict, size_t, Dee_ioflag_t))&reader_read,
	/* .ft_write  = */ NULL,
	/* .ft_seek   = */ (Dee_pos_t (DCALL *)(DeeFileObject *__restrict, Dee_off_t, int))&reader_seek,
	/* .ft_sync   = */ (int (DCALL *)(DeeFileObject *__restrict))&reader_sync,
	/* .ft_trunc  = */ NULL,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *__restrict))&reader_close,
	/* .ft_pread  = */ (size_t (DCALL *)(DeeFileObject *__restrict, void *__restrict, size_t, Dee_pos_t, Dee_ioflag_t))&reader_pread,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ (int (DCALL *)(DeeFileObject *__restrict, Dee_ioflag_t))&reader_getc,
	/* .ft_ungetc = */ (int (DCALL *)(DeeFileObject *__restrict, int))&reader_ungetc,
	/* .ft_putc   = */ NULL
};

/* Open a new file stream for reading memory from `data...+=data_size'
 * This stream assumes that data is immutable, and owned by `data_owner'.
 *
 * The best example for a type that fits these requirements is `string'
 * This function greatly differs from `DeeMemoryFile_New()', in that
 * the referenced data is shared with an explicit object, rather that
 * being held using a ticket-system, where the caller must manually
 * inform the memory stream when data is supposed to get released.
 *
 * However, the end result of both mechanisms is the same, in that the
 * stream indirectly references a given data-block, rather than having
 * to keep its own copy of some potentially humongous memory block. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF /*File*/ DeeObject *DCALL
DeeFileReader_NewMemory(DeeObject *__restrict data_owner,
                              void const *data, size_t data_size) {
	DREF DeeFileReaderObject *result;
	result = DeeObject_MALLOC(DeeFileReaderObject);
	if unlikely(!result)
		goto done;
	Dee_Incref(data_owner);
	result->r_owner = data_owner;
	result->r_begin = (byte_t const *)data;
	result->r_end   = (byte_t const *)data + data_size;
	result->r_ptr   = (byte_t const *)data;
	Dee_atomic_rwlock_init(&result->r_lock);
	DeeFileObject_Init(result, &DeeFileReader_Type);
done:
	return Dee_AsObject(result);
}

/* Similar to `DeeFileReader_NewMemory()', but used
 * to open a generic object using the buffer-interface. */
PUBLIC WUNUSED NONNULL((1)) DREF /*File*/ DeeObject *DCALL
DeeFileReader_NewObjectBuffer(DeeObject *__restrict data,
                              size_t start, size_t end) {
	DREF DeeFileReaderObject *result;
	result = DeeObject_MALLOC(DeeFileReaderObject);
	if unlikely(!result)
		goto done;
	if (DeeObject_GetBuf(data, &result->r_buffer, Dee_BUFFER_FREADONLY))
		goto err_r;

	/* Truncate the end-pointer. */
	if (end > result->r_buffer.bb_size)
		end = result->r_buffer.bb_size;

	/* Handle empty read. */
	if unlikely(start > end)
		start = end;

	/* Fill in members. */
	Dee_Incref(data);
	result->r_owner = data;
	result->r_begin = (byte_t const *)result->r_buffer.bb_base + start;
	result->r_end   = (byte_t const *)result->r_buffer.bb_base + end;
	result->r_ptr   = result->r_begin;
	Dee_atomic_rwlock_init(&result->r_lock);
	DeeFileObject_Init(result, &DeeFileReader_Type);
done:
	return Dee_AsObject(result);
err_r:
	DeeObject_FREE(result);
	return NULL;
}






PRIVATE NONNULL((1)) void DCALL
writer_init_empty_string(DeeFileWriterObject *__restrict self) {
	Dee_unicode_printer_init(&self->w_printer.wp_uni);
	self->w_string = NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
writer_ctor(DeeFileWriterObject *__restrict self) {
	Dee_atomic_rwlock_init(&self->w_lock);
	writer_init_empty_string(self);
	return 0;
}

#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
PRIVATE NONNULL((1, 2)) void DCALL
writer_init_bytes(DeeFileWriterObject *__restrict self,
                  DeeBytesObject *__restrict bytes) {
	self->w_printer.wp_byt.bp_length = DeeBytes_SIZE(bytes);
	self->w_printer.wp_byt.bp_bytes  = bytes;
	self->w_string = (DREF DeeStringObject *)ITER_DONE;
	if (DeeBytes_IsEmpty(bytes)) {
		self->w_printer.wp_byt.bp_bytes = NULL;
	} else {
		Dee_Incref(bytes);
	}
}

PRIVATE NONNULL((1)) void DCALL
writer_init_empty_bytes(DeeFileWriterObject *__restrict self) {
	self->w_printer.wp_byt.bp_bytes  = NULL;
	self->w_printer.wp_byt.bp_length = 0;
	self->w_string = (DREF DeeStringObject *)ITER_DONE;
}
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
writer_init_empty_with_hint(DeeFileWriterObject *__restrict self,
                            char const *__restrict hint) {
	if (strcmp(hint, "string") == 0) {
		writer_init_empty_string(self);
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	} else if (strcmp(hint, "bytes") == 0) {
		writer_init_empty_bytes(self);
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	} else if (strcmp(hint, "data") == 0) {
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		/* Prefer bytes-mode when it comes to "data" */
		writer_init_empty_bytes(self);
#else /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		writer_init_empty_string(self);
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	} else {
		return DeeError_Throwf(&DeeError_ValueError,
		                       "Unsupported file writer hint %q",
		                       hint);
	}
	return 0;
}



PRIVATE NONNULL((1, 2)) void DCALL
writer_init_string(DeeFileWriterObject *__restrict self,
                   DeeStringObject *__restrict string) {
	self->w_printer.wp_uni.up_flags = (uint8_t)DeeString_WIDTH(string);
	if (self->w_printer.wp_uni.up_flags == STRING_WIDTH_1BYTE) {
		self->w_string = NULL;
		self->w_printer.wp_uni.up_buffer = (void *)DeeString_STR(string);
		self->w_printer.wp_uni.up_length = DeeString_SIZE(string);
	} else {
		self->w_string = string;
		self->w_printer.wp_uni.up_buffer = (void *)DeeString_WSTR(string);
		self->w_printer.wp_uni.up_length = WSTR_LENGTH(self->w_printer.wp_uni.up_buffer);
	}
	Dee_Incref(string);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
writer_copy(DeeFileWriterObject *__restrict self,
            DeeFileWriterObject *__restrict other) {
	DREF DeeObject *data = DeeFileWriter_GetData((DeeObject *)other);
	if unlikely(!data)
		goto err;
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	if (DeeBytes_Check(data)) {
		writer_init_bytes(self, (DeeBytesObject *)data);
	} else
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	{
		ASSERT_OBJECT_TYPE_EXACT(data, &DeeString_Type);
		writer_init_string(self, (DeeStringObject *)data);
	}
	Dee_Decref_unlikely(data);
	Dee_atomic_rwlock_init(&self->w_lock);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
writer_init(DeeFileWriterObject *__restrict self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("_FileWriter", params: "
	DeeObject *data:?X3?Dstring?DBytes?N=Dee_EmptyString,
	DeeObject *hint:?Dstring=!P{string}=NULL
");]]]*/
	struct {
		DeeObject *data;
		DeeObject *hint;
	} args;
	args.data = Dee_EmptyString;
	args.hint = NULL;
	DeeArg_UnpackStruct0Or1Or2(err, argc, argv, "_FileWriter", &args, &args.data, &args.hint);
/*[[[end]]]*/
	if (args.hint && DeeObject_AssertTypeExact(args.hint, &DeeString_Type))
		goto err;
	Dee_atomic_rwlock_init(&self->w_lock);
	if (DeeNone_Check(args.data)) {
		if (args.hint)
			return writer_init_empty_with_hint(self, DeeString_STR(args.hint));
		writer_init_empty_string(self);
	} else {
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		if (DeeBytes_Check(args.data)) {
			writer_init_bytes(self, (DeeBytesObject *)args.data);
		} else if (DeeString_Check(args.data)) {
			writer_init_string(self, (DeeStringObject *)args.data);
		} else {
			DeeObject_TypeAssertFailed3(args.data, &DeeBytes_Type, &DeeString_Type, &DeeNone_Type);
			goto err;
		}
#else /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		if (DeeObject_AssertTypeExact(args.data, &DeeString_Type))
			goto err;
		writer_init_string(self, (DeeStringObject *)args.data);
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
writer_init_kw(DeeFileWriterObject *__restrict self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("_FileWriter", params: "
	DeeObject *data?:?X3?Dstring?DBytes?N=!N,
	char const *hint:?Dstring=!P{string}=\"string\"
", docStringPrefix: "writer");]]]*/
#define writer__FileWriter_params "data:?X3?Dstring?DBytes?N=!N,hint=!P{string}"
	struct {
		DeeObject *data;
		char const *hint;
	} args;
	args.data = Dee_None;
	args.hint = "string";
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__data_hint, "|os:_FileWriter", &args))
		goto err;
/*[[[end]]]*/
	Dee_atomic_rwlock_init(&self->w_lock);
	if (DeeNone_Check(args.data))
		return writer_init_empty_with_hint(self, args.hint);
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	if (DeeBytes_Check(args.data)) {
		writer_init_bytes(self, (DeeBytesObject *)args.data);
	} else if (DeeString_Check(args.data)) {
		writer_init_string(self, (DeeStringObject *)args.data);
	} else {
		DeeObject_TypeAssertFailed3(args.data, &DeeBytes_Type, &DeeString_Type, &DeeNone_Type);
		goto err;
	}
#else /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
#undef writer__FileWriter_params
#define writer__FileWriter_params "data:?X2?Dstring?N=!N,hint=!P{string}"
	if (DeeObject_AssertTypeExact(args.data, &DeeString_Type))
		goto err;
	writer_init_string(self, (DeeStringObject *)args.data);
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
writer_serialize(DeeFileWriterObject *__restrict self,
                 DeeSerial *__restrict writer,
                 Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DeeFileWriterObject, field))
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	STATIC_ASSERT(offsetof(DeeFileWriterObject, w_printer.wp_uni.up_length) ==
	              offsetof(DeeFileWriterObject, w_printer.wp_byt.bp_length));
	STATIC_ASSERT(offsetof(DeeFileWriterObject, w_printer.wp_uni.up_buffer) ==
	              offsetof(DeeFileWriterObject, w_printer.wp_byt.bp_bytes));
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	DeeFileWriterObject *out;
	size_t self__up_length;
	void *self__up_buffer;
	unsigned char self__up_flags;
	unsigned char self__up_pend[COMPILER_LENOF(self->w_printer.wp_uni.up_pend)];
again:
	DeeFileWriter_LockRead(self);
	self__up_length = self->w_printer.wp_uni.up_length;
	self__up_buffer = self->w_printer.wp_uni.up_buffer;
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	if (self->w_string == (DREF DeeStringObject *)ITER_DONE) {
		size_t bytes_size;
		DREF DeeBytesObject *bytes = (DREF DeeBytesObject *)self__up_buffer;
		out = DeeSerial_Addr2Mem(writer, addr, DeeFileWriterObject);
		Dee_atomic_rwlock_init(&out->w_lock);
		out->w_string = (DREF DeeStringObject *)ITER_DONE;
		out->w_printer.wp_byt.bp_length = self__up_length;
		if unlikely(!bytes) {
			out->w_printer.wp_byt.bp_bytes = NULL;
			ASSERT(self__up_length == 0);
			DeeFileWriter_LockEndRead(self);
			return 0;
		}
		bytes_size = bytes->b_size;
		ASSERT(self__up_length <= bytes_size);
		if (DeeObject_IsShared(bytes)) {
			ASSERT(self__up_length == bytes_size);
		} else if (self__up_length < bytes_size) {
			bytes = DeeBytes_TruncateBuffer(bytes, self__up_length);
			self->w_printer.wp_byt.bp_bytes = bytes;
		}
		Dee_Incref(bytes);
		DeeFileWriter_LockEndRead(self);
		return DeeSerial_PutObjectInherited(writer, ADDROF(w_printer.wp_byt.bp_bytes), bytes);
	}
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	self__up_flags = self->w_printer.wp_uni.up_flags;
	memcpy(self__up_pend, self->w_printer.wp_uni.up_pend, sizeof(self__up_pend));
	if (self__up_buffer) {
		unsigned int width = self__up_flags & Dee_UNICODE_PRINTER_FWIDTH;
		void *copy_base;
		size_t copy_size;
		ptrdiff_t copy_offset;
		Dee_seraddr_t copy_outaddr;
		void *copy_out;
		SWITCH_SIZEOF_WIDTH(width) {
		Dee_CASE_WIDTH_1BYTE: {
			copy_base   = COMPILER_CONTAINER_OF((char *)self__up_buffer, DeeStringObject, s_str);
			copy_size   = offsetof(DeeStringObject, s_str) + ((self__up_length + 1) * sizeof(char));
			copy_offset = offsetof(DeeStringObject, s_str);
		}	break;

		Dee_CASE_WIDTH_2BYTE: {
			copy_base   = (size_t *)self__up_buffer - 1;
			copy_size   = sizeof(size_t) + ((self__up_length + 1) * 2);
			copy_offset = sizeof(size_t);
		}	break;

		Dee_CASE_WIDTH_4BYTE: {
			copy_base   = (size_t *)self__up_buffer - 1;
			copy_size   = sizeof(size_t) + ((self__up_length + 1) * 4);
			copy_offset = sizeof(size_t);
		}	break;
		}
		copy_outaddr = DeeSerial_TryMalloc(writer, copy_size, NULL);
		if (!Dee_SERADDR_ISOK(copy_outaddr)) {
			DeeFileWriter_LockEndRead(self);
			copy_outaddr = DeeSerial_Malloc(writer, copy_size, NULL);
			if (!Dee_SERADDR_ISOK(copy_outaddr))
				goto err;
			DeeFileWriter_LockRead(self);
			if unlikely(self__up_length != self->w_printer.wp_uni.up_length) {
free_copy_outaddr_and_again:
				DeeFileWriter_LockEndRead(self);
				DeeSerial_Free(writer, copy_outaddr, NULL);
				goto again;
			}
			if unlikely(self__up_buffer != self->w_printer.wp_uni.up_buffer)
				goto free_copy_outaddr_and_again;
			if unlikely(self__up_flags != self->w_printer.wp_uni.up_flags)
				goto free_copy_outaddr_and_again;
			if unlikely(memcmp(self__up_pend, self->w_printer.wp_uni.up_pend, sizeof(self__up_pend)) != 0)
				goto free_copy_outaddr_and_again;
		}
		copy_out = DeeSerial_Addr2Mem(writer, copy_outaddr, void *);
		memcpy(copy_out, copy_base, copy_size);
		((size_t *)((byte_t *)copy_out + copy_offset))[-1] = self__up_length;
		DeeFileWriter_LockEndRead(self);
		if (DeeSerial_PutAddr(writer, ADDROF(w_printer.wp_uni.up_buffer), copy_outaddr + copy_offset))
			goto err;
		out = DeeSerial_Addr2Mem(writer, addr, DeeFileWriterObject);
	} else {
		DeeFileWriter_LockEndRead(self);
		out = DeeSerial_Addr2Mem(writer, addr, DeeFileWriterObject);
		out->w_printer.wp_uni.up_buffer = NULL;
	}
	Dee_atomic_rwlock_init(&out->w_lock);
	out->w_string = NULL;
	out->w_printer.wp_uni.up_length = self__up_length;
	out->w_printer.wp_uni.up_flags  = self__up_flags;
	memcpy(out->w_printer.wp_uni.up_pend, self__up_pend, sizeof(self__up_pend));
	return 0;
err:
	return -1;
#undef ADDROF
}

PRIVATE NONNULL((1)) void DCALL
writer_fini(DeeFileWriterObject *__restrict self) {
	if (self->w_string) {
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		if (self->w_string == (DREF DeeStringObject *)ITER_DONE) {
			Dee_XDecref_likely(self->w_printer.wp_byt.bp_bytes);
		} else
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		{
			Dee_Decref(self->w_string);
		}
	} else if (!self->w_printer.wp_uni.up_buffer) {
		/* ... */
	} else if ((self->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		Dee_Decref(COMPILER_CONTAINER_OF(self->w_printer.wp_uni.up_buffer,
		                                 DeeStringObject,
		                                 s_str));
	} else {
		Dee_unicode_printer_fini(&self->w_printer.wp_uni);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
writer_visit(DeeFileWriterObject *__restrict self, Dee_visit_t proc, void *arg) {
	DeeFileWriter_LockRead(self);
	if (self->w_string) {
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		if (self->w_string == (DREF DeeStringObject *)ITER_DONE) {
			DeeBytesObject *bytes = self->w_printer.wp_byt.bp_bytes;
			if (bytes) {
				ASSERT(bytes->b_orig);
				if (bytes->b_orig != Dee_AsObject(bytes))
					Dee_Visit(bytes->b_orig);
			}
		} else
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		{
			Dee_Visit(self->w_string);
		}
	} else if (!self->w_printer.wp_uni.up_buffer) {
		/* ... */
	} else if ((self->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		Dee_Visit(COMPILER_CONTAINER_OF(self->w_printer.wp_uni.up_buffer,
		                                DeeStringObject,
		                                s_str));
	} else {
		/* ... */
	}
	DeeFileWriter_LockEndRead(self);
}

PRIVATE NONNULL((1, 2)) Dee_ssize_t DCALL
writer_printrepr(DeeFileWriterObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	DREF DeeObject *content;
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	content = DeeFileWriter_GetData(Dee_AsObject(self));
#else /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	content = DeeFileWriter_GetString(Dee_AsObject(self));
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	if unlikely(!content)
		goto err;
	return DeeFormat_Printf(printer, arg, "File.Writer(%R)", content);
err:
	return -1;
}

/* @return: ITER_DONE: Try again */
PRIVATE WUNUSED NONNULL((1)) DREF /*string*/ DeeStringObject *DCALL
DeeFileWriter_PackStringOrUnlock(DeeFileWriterObject *__restrict me) {
	DREF DeeStringObject *result;
	if unlikely(me->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FPENDING) {
		DeeFileWriter_LockEndWrite(me);
		DeeError_Throwf(&DeeError_UnicodeEncodeError,
		                "Incomplete utf-8 sequence");
		return NULL;
	}
	if ((me->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		/* Special case for 1-byte strings. */
		ASSERT(!me->w_string);
		if (!me->w_printer.wp_uni.up_buffer) {
			DeeFileWriter_LockEndWrite(me);
			return (DREF DeeStringObject *)DeeString_NewEmpty();
		}
		result = COMPILER_CONTAINER_OF(me->w_printer.wp_uni.up_buffer,
		                               DeeStringObject,
		                               s_str);
		if (!DeeObject_IsShared(result)) {
			if (me->w_printer.wp_uni.up_length != result->s_len) {
				/* Flush the string buffer and deallocated unused memory. */
				if (result->s_data) {
					Dee_string_utf_fini(result->s_data, result);
					Dee_string_utf_free(result->s_data);
					result->s_data = NULL;
				}
				result->s_hash = Dee_STRING_HASH_UNSET;
				result->s_len  = me->w_printer.wp_uni.up_length;
				result = (DREF DeeStringObject *)DeeObject_TryReallocc(result, offsetof(DeeStringObject, s_str),
				                                                       me->w_printer.wp_uni.up_length + 1, sizeof(char));
				if unlikely(!result) {
					result = COMPILER_CONTAINER_OF(me->w_printer.wp_uni.up_buffer,
					                               DeeStringObject,
					                               s_str);
				}
				me->w_printer.wp_uni.up_buffer = result->s_str;
			}
			result->s_str[result->s_len] = 0;
		} else {
			/* The string is already being shared, meaning that it must have already been flushed. */
			ASSERT(me->w_printer.wp_uni.up_length == result->s_len);
		}
	} else if (me->w_string) {
		/* A cached multi-byte string has already been set. */
		result = me->w_string;
	} else {
		/* Must pack together a multi-byte string. */
		result = (DREF DeeStringObject *)Dee_unicode_printer_trypack(&me->w_printer.wp_uni);
		if unlikely(!result)
			goto err_collect;
		me->w_string = result;
		me->w_printer.wp_uni.up_flags = (uint8_t)DeeString_WIDTH(result);
		if (me->w_printer.wp_uni.up_flags == STRING_WIDTH_1BYTE) {
			me->w_string = NULL;
			me->w_printer.wp_uni.up_buffer = (void *)DeeString_STR(result);
			me->w_printer.wp_uni.up_length = DeeString_SIZE(result);
		} else {
			me->w_string = result;
			me->w_printer.wp_uni.up_buffer = (void *)DeeString_WSTR(result);
			me->w_printer.wp_uni.up_length = WSTR_LENGTH(me->w_printer.wp_uni.up_buffer);
		}
	}
	Dee_Incref(result);
	DeeFileWriter_LockEndWrite(me);
	ASSERT(DeeString_STR(result)[DeeString_SIZE(result)] == 0);
	return result;
err_collect:
	DeeFileWriter_LockEndWrite(me);
	if (Dee_CollectMemory(512))
		return (DREF DeeStringObject *)ITER_DONE;
	return NULL;
}

#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
PRIVATE WUNUSED NONNULL((1)) DREF /*Bytes*/ DeeObject *DCALL
DeeFileWriter_PackBytesAndUnlock(DeeFileWriterObject *__restrict me) {
	size_t bytes_used, bytes_size;
	DREF DeeBytesObject *bytes = me->w_printer.wp_byt.bp_bytes;
	if unlikely(!bytes) {
		ASSERT(me->w_printer.wp_byt.bp_length == 0);
		DeeFileWriter_LockEndWrite(me);
		return DeeBytes_NewEmpty();
	}
	bytes_used = me->w_printer.wp_byt.bp_length;
	bytes_size = bytes->b_size;
	ASSERT(bytes_used <= bytes_size);
	if (DeeObject_IsShared(bytes)) {
		ASSERT(bytes_used == bytes_size);
		Dee_Incref(bytes);
		DeeFileWriter_LockEndWrite(me);
		return Dee_AsObject(bytes);
	}
	if (bytes_used < bytes_size) {
		bytes = DeeBytes_TruncateBuffer(bytes, bytes_used);
		me->w_printer.wp_byt.bp_bytes = bytes;
	}
	Dee_Incref(bytes);
	DeeFileWriter_LockEndWrite(me);
	return Dee_AsObject(bytes);
}

PUBLIC WUNUSED NONNULL((1)) DREF /*string|Bytes*/ DeeObject *DCALL
DeeFileWriter_GetData(DeeObject *__restrict self) {
	DREF DeeStringObject *string;
	DeeFileWriterObject *me = (DeeFileWriterObject *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeFileWriter_Type);
again:
	DeeFileWriter_LockWrite(me);
	if (DeeFileWriter_IsBytes(me))
		return DeeFileWriter_PackBytesAndUnlock(me);
	string = DeeFileWriter_PackStringOrUnlock(me);
	if (string != (DREF DeeStringObject *)ITER_DONE)
		return Dee_AsObject(string);
	goto again;
}




/* (Theoretical) utf-8 unicode sequence ranges:
 *  - 1-byte    -- 7               = 7 bits
 *  - 2-byte    -- 5+6             = 11 bits
 *  - 3-byte    -- 4+6+6           = 16 bits
 *  - 4-byte    -- 3+6+6+6         = 21 bits
 *  - 5-byte    -- 2+6+6+6+6       = 26 bits (Not valid unicode characters)
 *  - 6-byte    -- 1+6+6+6+6+6     = 31 bits (Not valid unicode characters)
 *  - 7-byte    --   6+6+6+6+6+6   = 36 bits (Not valid unicode characters)
 *  - 8-byte    --   6+6+6+6+6+6+6 = 42 bits (Not valid unicode characters)
 */
#define UTF8_1BYTE_MAX    (((uint32_t)1 << 7) - 1)
#define UTF8_2BYTE_MAX    (((uint32_t)1 << 11) - 1)
#define UTF8_3BYTE_MAX    (((uint32_t)1 << 16) - 1)
#define UTF8_4BYTE_MAX    (((uint32_t)1 << 21) - 1)
#define UTF8_5BYTE_MAX    (((uint32_t)1 << 26) - 1)
#define UTF8_6BYTE_MAX    (((uint32_t)1 << 31) - 1)


/* HINT: usc1 ->> U+0000 - U+00FF  (aka. latin-1) */
PRIVATE ATTR_PURE WUNUSED size_t DCALL
ucs1_as_utf8_length(uint8_t const *in, size_t in_chars) {
	size_t result = in_chars;
	while (in_chars--) {
		uint8_t b = *in++;
		if (b <= UTF8_1BYTE_MAX) {
			/* ... */
		} else {
			++result;
		}
	}
	return result;
}

PRIVATE byte_t *DCALL
ucs1_to_utf8_convert(byte_t *out, uint8_t const *in, size_t in_chars) {
	while (in_chars--) {
		uint8_t b = *in++;
		if (b <= UTF8_1BYTE_MAX) {
			*out++ = b;
		} else {
			*out++ = 0xc0 | (byte_t)((b >> 6) /* & 0x1f*/);
			*out++ = 0x80 | (byte_t)((b)&0x3f);
		}
	}
	return out;
}

/* HINT: usc2 ->> U+0000 - U+FFFF */
PRIVATE ATTR_PURE WUNUSED size_t DCALL
ucs2_as_utf8_length(uint16_t const *in, size_t in_chars) {
	size_t result = in_chars;
	while (in_chars--) {
		uint16_t b = *in++;
		if (b <= UTF8_1BYTE_MAX) {
			/* ... */
		} else if (b <= UTF8_2BYTE_MAX) {
			result += 1;
		} else {
			result += 2;
		}
	}
	return result;
}

PRIVATE byte_t *DCALL
ucs2_to_utf8_convert(byte_t *out, uint16_t const *in, size_t in_chars) {
	while (in_chars--) {
		uint16_t b = *in++;
		if (b <= UTF8_1BYTE_MAX) {
			*out++ = (byte_t)b;
		} else if (b <= UTF8_2BYTE_MAX) {
			*out++ = 0xc0 | (byte_t)((b >> 6) /* & 0x1f*/);
			*out++ = 0x80 | (byte_t)((b)&0x3f);
		} else {
			*out++ = 0xe0 | (byte_t)((b >> 12) /* & 0x0f*/);
			*out++ = 0x80 | (byte_t)((b >> 6) & 0x3f);
			*out++ = 0x80 | (byte_t)((b)&0x3f);
		}
	}
	return out;
}

/* HINT: usc4 ->> U+00000000 - U+FFFFFFFF  (aka. utf-32) */
PRIVATE ATTR_PURE WUNUSED size_t DCALL
ucs4_as_utf8_length(uint32_t const *in, size_t in_chars) {
	size_t result = in_chars;
	while (in_chars--) {
		uint32_t b = *in++;
		if (b <= UTF8_1BYTE_MAX) {
			/* ... */
		} else if (b <= UTF8_2BYTE_MAX) {
			result += 1;
		} else if (b <= UTF8_3BYTE_MAX) {
			result += 2;
		} else if (b <= UTF8_4BYTE_MAX) {
			result += 3;
		} else if (b <= UTF8_5BYTE_MAX) {
			result += 4;
		} else if (b <= UTF8_6BYTE_MAX) {
			result += 5;
		} else {
			result += 6;
		}
	}
	return result;
}

PRIVATE byte_t *DCALL
ucs4_to_utf8_convert(byte_t *out, uint32_t const *in, size_t in_chars) {
	while (in_chars--) {
		uint32_t b = *in++;
		if (b <= UTF8_1BYTE_MAX) {
			*out++ = (byte_t)b;
		} else if (b <= UTF8_2BYTE_MAX) {
			*out++ = 0xc0 | (byte_t)((b >> 6) /* & 0x1f*/);
			*out++ = 0x80 | (byte_t)((b)&0x3f);
		} else if (b <= UTF8_3BYTE_MAX) {
			*out++ = 0xe0 | (byte_t)((b >> 12) /* & 0x0f*/);
			*out++ = 0x80 | (byte_t)((b >> 6) & 0x3f);
			*out++ = 0x80 | (byte_t)((b)&0x3f);
		} else if (b <= UTF8_4BYTE_MAX) {
			*out++ = 0xf0 | (byte_t)((b >> 18) /* & 0x07*/);
			*out++ = 0x80 | (byte_t)((b >> 12) & 0x3f);
			*out++ = 0x80 | (byte_t)((b >> 6) & 0x3f);
			*out++ = 0x80 | (byte_t)((b)&0x3f);
		} else if (b <= UTF8_5BYTE_MAX) {
			*out++ = 0xf8 | (byte_t)((b >> 24) /* & 0x03*/);
			*out++ = 0x80 | (byte_t)((b >> 18) & 0x3f);
			*out++ = 0x80 | (byte_t)((b >> 12) & 0x3f);
			*out++ = 0x80 | (byte_t)((b >> 6) & 0x3f);
			*out++ = 0x80 | (byte_t)((b)&0x3f);
		} else if (b <= UTF8_6BYTE_MAX) {
			*out++ = 0xfc | (byte_t)((b >> 30) /* & 0x01*/);
			*out++ = 0x80 | (byte_t)((b >> 24) & 0x3f);
			*out++ = 0x80 | (byte_t)((b >> 18) & 0x3f);
			*out++ = 0x80 | (byte_t)((b >> 12) & 0x3f);
			*out++ = 0x80 | (byte_t)((b >> 6) & 0x3f);
			*out++ = 0x80 | (byte_t)((b)&0x3f);
		} else {
			*out++ = 0xfe;
			*out++ = 0x80 | (byte_t)((b >> 30) & 0x03 /* & 0x3f*/);
			*out++ = 0x80 | (byte_t)((b >> 24) & 0x3f);
			*out++ = 0x80 | (byte_t)((b >> 18) & 0x3f);
			*out++ = 0x80 | (byte_t)((b >> 12) & 0x3f);
			*out++ = 0x80 | (byte_t)((b >> 6) & 0x3f);
			*out++ = 0x80 | (byte_t)((b)&0x3f);
		}
	}
	return out;
}

/* @return: 1 : Try again (unlocked)
 * @return: 0 : Success (lock still held)
 * @return: -1: Error (unlocked) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeFileWriter_String2BytesOrUnlock(DeeFileWriterObject *__restrict me) {
	/* Must do the equivalent of: `me.bytes = me.string.encode("utf-8")',
	 * but also append "Dee_UNICODE_PRINTER_FPENDING" if that exists. */
	size_t ucs_length, num_pending, num_bytes, ucs_chars;
	DREF DeeBytesObject *bytes;
	DREF DeeStringObject *string;
	union Dee_charptr wstr;
	shift_t width;
	byte_t *out_iter;
	ASSERT(DeeFileWriter_LockWriting(me));
	ASSERT(DeeFileWriter_IsString(me));

	/* Figure out */
	num_pending = (size_t)((me->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FPENDING) >>
	                       Dee_UNICODE_PRINTER_FPENDING_SHFT);

	/* Special case for 1-byte strings. */
	width = me->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH;
	if (width == STRING_WIDTH_1BYTE) {
		ASSERT(!me->w_string);
		if (!me->w_printer.wp_uni.up_buffer) {
			if (!num_pending) {
				me->w_printer.wp_byt.bp_bytes  = NULL;
				me->w_printer.wp_byt.bp_length = 0;
				me->w_string = (DREF DeeStringObject *)ITER_DONE;
				return 0;
			}
			bytes = DeeBytes_TryNewBufferData(me->w_printer.wp_uni.up_pend, num_pending);
			if unlikely(!bytes) {
				DeeFileWriter_LockEndWrite(me);
				bytes = DeeBytes_NewBufferUninitialized(num_pending);
				if unlikely(!bytes)
					goto err;
				DeeFileWriter_LockWrite(me);
				if (DeeFileWriter_IsString(me) &&
				    (me->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE &&
				    (num_pending == (size_t)((me->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FPENDING) >>
				                             Dee_UNICODE_PRINTER_FPENDING_SHFT)) &&
				    !me->w_printer.wp_uni.up_buffer) {
					/* continue... */
					me->w_printer.wp_byt.bp_bytes  = bytes;
					me->w_printer.wp_byt.bp_length = num_pending;
					me->w_string = (DREF DeeStringObject *)ITER_DONE;
					DeeFileWriter_LockEndWrite(me);
				} else {
					DeeFileWriter_LockEndWrite(me);
					DeeBytes_Destroy(bytes);
				}
				return 1;
			}
			me->w_printer.wp_byt.bp_bytes  = bytes;
			me->w_printer.wp_byt.bp_length = num_pending;
			me->w_string = (DREF DeeStringObject *)ITER_DONE;
			return 0;
		}

		/* Load string that was written until this point */
		string = COMPILER_CONTAINER_OF(me->w_printer.wp_uni.up_buffer,
		                               DeeStringObject,
		                               s_str);

		/* Determine length of utf-8 sequence contained within
		 * string (since that's what had been decoded thus far) */
		ucs_chars  = me->w_printer.wp_uni.up_length;
		ucs_length = ucs1_as_utf8_length((uint8_t const *)string->s_str, ucs_chars);

		/* Allocate bytes equivalent of string */
		num_bytes = ucs_length + num_pending;
		bytes = DeeBytes_TryNewBufferUninitialized(num_bytes);
		if unlikely(!bytes)
			goto unlock_and_collect_memory;

		/* Convert ucs-1 (aka. latin-1) string to utf-8 bytes */
		out_iter = bytes->b_buffer;
		if (ucs_chars == ucs_length) {
			out_iter = (byte_t *)mempcpy(out_iter, string->s_str, ucs_length);
		} else {
			out_iter = ucs1_to_utf8_convert(out_iter, (uint8_t const *)string->s_str, ucs_chars);
		}
		ASSERT(out_iter == bytes->b_buffer + ucs_length);

		/* Append trailing, pending utf-8 characters */
		memcpy(out_iter, me->w_printer.wp_uni.up_pend, num_pending);

		/* Assign "bytes" */
		me->w_printer.wp_byt.bp_bytes  = bytes;
		me->w_printer.wp_byt.bp_length = num_bytes;
		me->w_string = (DREF DeeStringObject *)ITER_DONE;

		/* Destroy old string */
		if (!string->s_data || !(string->s_data->u_flags & Dee_STRING_UTF_FFINIHOOK)) {
			Dee_Decref_likely(string);
			return 0;
		}
		if (Dee_DecrefIfNotOne(string))
			return 0;

		/* String is probably being destroyed, and has
		 * a fini-hook -> must destroy without locks! */
		DeeFileWriter_LockEndWrite(me);
		Dee_Decref_likely(string);
		return 1;
	}

	/* Deal with case where bytes until now were decoded into a multi-byte string. */
	ASSERT(width == STRING_WIDTH_2BYTE ||
	       width == STRING_WIDTH_4BYTE);
	string   = me->w_string;
	wstr.ptr = me->w_printer.wp_uni.up_buffer;
	ASSERT(!string || (string->s_data));
	ASSERT(!string || (DeeString_WIDTH(string) != STRING_WIDTH_1BYTE));
	ASSERT(!string || (DeeString_WIDTH(string) == width));
	ASSERT(!string || (me->w_printer.wp_uni.up_buffer == DeeString_WSTR(string)));
	ASSERT(!string || (me->w_printer.wp_uni.up_length == DeeString_WLEN(string)));

	/* Must pack together a multi-byte string. */
	ucs_chars = me->w_printer.wp_uni.up_length;
	if (width == STRING_WIDTH_2BYTE) {
		ucs_length = ucs2_as_utf8_length(wstr.cp16, ucs_chars);
	} else {
		ucs_length = ucs4_as_utf8_length(wstr.cp32, ucs_chars);
	}

	/* Allocate bytes equivalent of string */
	num_bytes = ucs_length + num_pending;
	bytes = DeeBytes_TryNewBufferUninitialized(num_bytes);
	if unlikely(!bytes)
		goto unlock_and_collect_memory;

	/* Convert ucs-2/4 string to utf-8 bytes */
	out_iter = bytes->b_buffer;
	if (width == STRING_WIDTH_2BYTE) {
		out_iter = ucs2_to_utf8_convert(out_iter, wstr.cp16, ucs_chars);
	} else {
		out_iter = ucs4_to_utf8_convert(out_iter, wstr.cp32, ucs_chars);
	}
	ASSERT(out_iter == bytes->b_buffer + ucs_length);

	/* Append trailing, pending utf-8 characters */
	memcpy(out_iter, me->w_printer.wp_uni.up_pend, num_pending);

	/* Assign "bytes" */
	me->w_printer.wp_byt.bp_bytes  = bytes;
	me->w_printer.wp_byt.bp_length = num_bytes;
	me->w_string = (DREF DeeStringObject *)ITER_DONE;

	/* Cleanup the old string/unicode-printer */
	if (!string) {
		DeeString_FreeWidthBuffer(wstr.ptr, width); /* Equivalent to 'Dee_unicode_printer_fini(&me->w_printer.wp_uni)' */
		return 0;
	}
	ASSERT(string->s_data);
	if (!(string->s_data->u_flags & Dee_STRING_UTF_FFINIHOOK)) {
		Dee_Decref_likely(string);
		return 0;
	}
	if (Dee_DecrefIfNotOne(string))
		return 0;
	DeeFileWriter_LockEndWrite(me);
	return 1;
unlock_and_collect_memory:
	DeeFileWriter_LockEndWrite(me);
	if (Dee_CollectMemory(offsetof(DeeBytesObject, b_buffer) + num_bytes))
		return 1;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) DREF /*Bytes*/ DeeObject *DCALL
DeeFileWriter_GetBytes(DeeObject *__restrict self) {
	DeeFileWriterObject *me = (DeeFileWriterObject *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeFileWriter_Type);
again:
	DeeFileWriter_LockWrite(me);
	if (!DeeFileWriter_IsBytes(me)) {
		int status = DeeFileWriter_String2BytesOrUnlock(me);
		if (status != 0) {
			if unlikely(status < 0)
				goto err;
			goto again;
		}
	}
	return DeeFileWriter_PackBytesAndUnlock(me);
err:
	return NULL;
}
#else /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
PUBLIC WUNUSED NONNULL((1)) DREF /*Bytes*/ DeeObject *DCALL
DeeFileWriter_GetBytes(DeeObject *__restrict self) {
	char const *utf8;
	DREF DeeObject *result;
	DREF DeeObject *string = DeeFileWriter_GetString(self);
	if unlikely(!string)
		goto err;
	utf8 = DeeString_AsUtf8(string);
	if unlikely(!utf8)
		goto err_string;
	result = DeeBytes_NewViewRo(string, utf8, WSTR_LENGTH(utf8));
	Dee_Decref_unlikely(string);
	return result;
err_string:
	Dee_Decref_unlikely(string);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF /*string|Bytes*/ DeeObject *DCALL
DeeFileWriter_GetData(DeeObject *__restrict self) {
	return DeeFileWriter_GetString(self);
}
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */

/* Returns the current string written by the writer. */
PUBLIC WUNUSED NONNULL((1)) /*string*/ DREF DeeObject *DCALL
DeeFileWriter_GetString(DeeObject *__restrict self) {
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	DREF DeeObject *result;
	DREF DeeObject *data = DeeFileWriter_GetData(self);
	if likely(DeeString_Check(data))
		return data;
	ASSERT_OBJECT_TYPE_EXACT(data, &DeeBytes_Type);
	result = DeeString_NewUtf8((char const *)DeeBytes_DATA(data),
	                           DeeBytes_SIZE(data),
	                           STRING_ERROR_FSTRICT);
	Dee_Decref_unlikely(data);
	return result;
#else /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	DeeFileWriterObject *me = (DeeFileWriterObject *)self;
	DREF DeeStringObject *result;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeFileWriter_Type);
again:
	DeeFileWriter_LockWrite(me);
	result = DeeFileWriter_PackStringOrUnlock(me);
	if (result != (DREF DeeStringObject *)ITER_DONE)
		return Dee_AsObject(result);
	goto again;
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
}

#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
#define writer_delstring_or_bytes__keep       0
#define writer_delstring_or_bytes__set_bytes  1
#define writer_delstring_or_bytes__set_string 2
PRIVATE WUNUSED NONNULL((1)) int DCALL
writer_delstring_or_bytes(DeeFileWriterObject *__restrict self, unsigned int how)
#else /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
PRIVATE WUNUSED NONNULL((1)) int DCALL
writer_delstring(DeeFileWriterObject *__restrict self)
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
{
	DeeFileWriter_LockWrite(self);
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	if (DeeFileWriter_IsBytes(self)) {
		DREF DeeBytesObject *bytes;
		bytes = self->w_printer.wp_byt.bp_bytes;
		self->w_printer.wp_byt.bp_bytes  = NULL;
		self->w_printer.wp_byt.bp_length = 0;
		ASSERT(self->w_string == (DREF DeeStringObject *)ITER_DONE);
		if (how == writer_delstring_or_bytes__set_string)
			self->w_string = NULL;
		DeeFileWriter_LockEndWrite(self);
		Dee_XDecref_likely(bytes);
		return 0;
	}
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	if (self->w_string) {
		DREF DeeStringObject *old_string;
		old_string = self->w_string;
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		if (how == writer_delstring_or_bytes__set_bytes) {
			self->w_string = (DREF DeeStringObject *)ITER_DONE;
			self->w_printer.wp_byt.bp_bytes  = NULL;
			self->w_printer.wp_byt.bp_length = 0;
		} else
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		{
			self->w_string = NULL;
			Dee_unicode_printer_init(&self->w_printer.wp_uni);
		}
		DeeFileWriter_LockEndWrite(self);
		Dee_Decref(old_string);
	} else if (!self->w_printer.wp_uni.up_buffer) {
		ASSERT(self->w_printer.wp_uni.up_length == 0);
		ASSERT((self->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		if (how == writer_delstring_or_bytes__set_bytes) {
			STATIC_ASSERT(offsetof(DeeFileWriterObject, w_printer.wp_uni.up_length) ==
			              offsetof(DeeFileWriterObject, w_printer.wp_byt.bp_length));
			STATIC_ASSERT(offsetof(DeeFileWriterObject, w_printer.wp_uni.up_buffer) ==
			              offsetof(DeeFileWriterObject, w_printer.wp_byt.bp_bytes));
			self->w_string = (DREF DeeStringObject *)ITER_DONE;
		} else
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		{
			Dee_unicode_printer_init(&self->w_printer.wp_uni);
		}
		DeeFileWriter_LockEndWrite(self);
	} else if ((self->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		Dee_Decref(COMPILER_CONTAINER_OF(self->w_printer.wp_uni.up_buffer,
		                                 DeeStringObject,
		                                 s_str));
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		if (how == writer_delstring_or_bytes__set_bytes) {
			self->w_string = (DREF DeeStringObject *)ITER_DONE;
			self->w_printer.wp_byt.bp_bytes  = NULL;
			self->w_printer.wp_byt.bp_length = 0;
		} else
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		{
			Dee_unicode_printer_init(&self->w_printer.wp_uni);
		}
		DeeFileWriter_LockEndWrite(self);
	} else {
		struct Dee_unicode_printer old_printer;
		memcpy(&old_printer, &self->w_printer.wp_uni, sizeof(old_printer));
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		if (how == writer_delstring_or_bytes__set_bytes) {
			self->w_string = (DREF DeeStringObject *)ITER_DONE;
			self->w_printer.wp_byt.bp_bytes  = NULL;
			self->w_printer.wp_byt.bp_length = 0;
		} else
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		{
			Dee_unicode_printer_init(&self->w_printer.wp_uni);
		}
		DeeFileWriter_LockEndWrite(self);
		Dee_unicode_printer_fini(&old_printer);
	}
	return 0;
}

#define writer_getstring DeeFileWriter_GetString

#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
PRIVATE WUNUSED NONNULL((1)) int DCALL
writer_delstring(DeeFileWriterObject *__restrict self) {
	return writer_delstring_or_bytes(self, writer_delstring_or_bytes__set_string);
}
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
writer_setstring(DeeFileWriterObject *__restrict self,
                 DeeStringObject *__restrict value) {
	DREF DeeStringObject *old_string;
	struct Dee_unicode_printer old_printer;
	if (DeeNone_Check(value))
		goto do_del_string;
	if (DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto err;
	if (DeeString_IsEmpty(value))
		goto do_del_string;
	Dee_Incref(value);
	DeeFileWriter_LockWrite(self);
	memcpy(&old_printer, &self->w_printer.wp_uni, sizeof(struct Dee_unicode_printer));
	old_string = self->w_string;
	self->w_printer.wp_uni.up_flags = (uint8_t)DeeString_WIDTH(value);
	if (self->w_printer.wp_uni.up_flags == STRING_WIDTH_1BYTE) {
		self->w_string = NULL;
		self->w_printer.wp_uni.up_buffer = (void *)DeeString_STR(value);
		self->w_printer.wp_uni.up_length = DeeString_SIZE(value);
	} else {
		self->w_string = value;
		self->w_printer.wp_uni.up_buffer = (void *)DeeString_WSTR(value);
		self->w_printer.wp_uni.up_length = WSTR_LENGTH(self->w_printer.wp_uni.up_buffer);
	}
	DeeFileWriter_LockEndWrite(self);
	if (old_string) {
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		if (old_string == (DREF DeeStringObject *)ITER_DONE) {
			DREF DeeBytesObject *old_bytes;
			STATIC_ASSERT(offsetof(DeeFileWriterObject, w_printer.wp_uni.up_buffer) ==
			              offsetof(DeeFileWriterObject, w_printer.wp_byt.bp_bytes));
			old_bytes = (DREF DeeBytesObject *)old_printer.up_buffer;
			Dee_XDecref_likely(old_bytes);
		} else
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		{
			Dee_Decref(old_string);
		}
	} else if (!old_printer.up_buffer) {
		ASSERT(old_printer.up_length == 0);
		ASSERT((old_printer.up_flags & Dee_UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
	} else if ((old_printer.up_flags & Dee_UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		Dee_Decref(COMPILER_CONTAINER_OF(old_printer.up_buffer,
		                                 DeeStringObject,
		                                 s_str));
	} else {
		Dee_unicode_printer_fini(&old_printer);
	}
	return 0;
do_del_string:
	return writer_delstring(self);
err:
	return -1;
}


#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
#define writer_getbytes DeeFileWriter_GetBytes

PRIVATE WUNUSED NONNULL((1)) int DCALL
writer_delbytes(DeeFileWriterObject *__restrict self) {
	return writer_delstring_or_bytes(self, writer_delstring_or_bytes__set_bytes);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
writer_setbytes(DeeFileWriterObject *__restrict self,
                DeeBytesObject *__restrict value) {
	DREF DeeStringObject *old_string;
	struct Dee_unicode_printer old_printer;
	if (DeeNone_Check(value))
		goto do_del_bytes;
	if (DeeObject_AssertTypeExact(value, &DeeBytes_Type))
		goto err;
	if (DeeBytes_IsEmpty(value))
		goto do_del_bytes;
	Dee_Incref(value);
	DeeFileWriter_LockWrite(self);
	memcpy(&old_printer, &self->w_printer.wp_uni, sizeof(struct Dee_unicode_printer));
	old_string = self->w_string;
	self->w_printer.wp_byt.bp_bytes = value;
	self->w_printer.wp_byt.bp_length = DeeBytes_SIZE(value);
	DeeFileWriter_LockEndWrite(self);
	if (old_string) {
		if (old_string == (DREF DeeStringObject *)ITER_DONE) {
			DREF DeeBytesObject *old_bytes;
			STATIC_ASSERT(offsetof(DeeFileWriterObject, w_printer.wp_uni.up_buffer) ==
			              offsetof(DeeFileWriterObject, w_printer.wp_byt.bp_bytes));
			old_bytes = (DREF DeeBytesObject *)old_printer.up_buffer;
			Dee_XDecref_likely(old_bytes);
		} else {
			Dee_Decref(old_string);
		}
	} else if (!old_printer.up_buffer) {
		ASSERT(old_printer.up_length == 0);
		ASSERT((old_printer.up_flags & Dee_UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
	} else if ((old_printer.up_flags & Dee_UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		Dee_Decref(COMPILER_CONTAINER_OF(old_printer.up_buffer,
		                                 DeeStringObject,
		                                 s_str));
	} else {
		Dee_unicode_printer_fini(&old_printer);
	}
	return 0;
do_del_bytes:
	return writer_delbytes(self);
err:
	return -1;
}
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */

#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
#define writer_getdata DeeFileWriter_GetData

PRIVATE WUNUSED NONNULL((1)) int DCALL
writer_deldata(DeeFileWriterObject *__restrict self) {
	return writer_delstring_or_bytes(self, writer_delstring_or_bytes__keep);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
writer_setdata(DeeFileWriterObject *__restrict self, DeeObject *data) {
	if (DeeString_Check(data))
		return writer_setstring(self, (DeeStringObject *)data);
	if (DeeBytes_Check(data))
		return writer_setbytes(self, (DeeBytesObject *)data);
	if (DeeNone_Check(data))
		return writer_deldata(self);
	return DeeObject_TypeAssertFailed3(data, &DeeString_Type, &DeeBytes_Type, &DeeNone_Type);
}
#else /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
#define writer_getdata writer_getstring
#define writer_deldata writer_delstring
#define writer_setdata writer_setstring
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
writer_sizeof(DeeFileWriterObject *self) {
	size_t result;
	DeeFileWriter_LockRead(self);
	result = sizeof(DeeFileWriterObject);
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	if (DeeFileWriter_IsBytes(self)) {
		if (!self->w_printer.wp_byt.bp_bytes) {
			ASSERT(self->w_printer.wp_byt.bp_length == 0);
		} else if (DeeObject_IsShared(self->w_printer.wp_byt.bp_bytes)) {
			/* Not exclusively our bytes! */
		} else {
			result += offsetof(DeeBytesObject, b_buffer);
			result += self->w_printer.wp_byt.bp_length;
		}
	} else
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	if (self->w_printer.wp_uni.up_buffer) {
		shift_t width = self->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH;
		size_t wsiz = WSTR_LENGTH(self->w_printer.wp_uni.up_buffer) + 1;
		result += Dee_STRING_MUL_SIZEOF_WIDTH(wsiz, width);
	}
	DeeFileWriter_LockEndRead(self);
	return DeeInt_NewSize(result);
}

PRIVATE struct type_getset tpconst writer_getsets[] = {
	TYPE_GETSET_AB_F(STR_string,
	                 &writer_getstring,
	                 &writer_delstring,
	                 &writer_setstring,
	                 METHOD_FNOREFESCAPE,
	                 "->?Dstring\n"
	                 "Get/set the currently written text string (in utf-8)"),
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	TYPE_GETSET_AB_F("bytes",
	                 &writer_getbytes,
	                 &writer_delbytes,
	                 &writer_setbytes,
	                 METHOD_FNOREFESCAPE,
	                 "->?DBytes\n"
	                 "Get/set the currently written data bytes"),
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	TYPE_GETSET_AB_F("data",
	                 &writer_getdata,
	                 &writer_deldata,
	                 &writer_setdata,
	                 METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?DBytes\n"
	                 "Get/set the currently written file contents as either a ?Dstring, or as "
	                 /**/ "?DBytes, depending on which format was hinted to the writer, and which "
	                 /**/ "format can still be used to represent the file's contents. For this "
	                 /**/ "purpose, note that ?Dstring can (and is) only used as long as ?#bytes "
	                 /**/ "would be equal to the ${this.string.encode(<Default Encoding>)}, where "
	                 /**/ "#C{<Default Encoding>} is the encoding used by the $print statement "
	                 /**/ "when writing unicode strings to a file (this encoding will probably "
	                 /**/ "always be $\"utf-8\")"),

	/* TODO: hint->?Dstring  (one of "string", "bytes" or "data", the last never
	 *       actually being returned and when set, will be treated as a no-op) */

	TYPE_GETTER_AB_F("__sizeof__", &writer_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
writer_get(DeeFileWriterObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("get", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "get");
/*[[[end]]]*/
	return (DREF DeeStringObject *)DeeFileWriter_GetString(Dee_AsObject(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
writer_size(DeeFileWriterObject *self, size_t argc, DeeObject *const *argv) {
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	STATIC_ASSERT(offsetof(DeeFileWriterObject, w_printer.wp_uni.up_length) ==
	              offsetof(DeeFileWriterObject, w_printer.wp_byt.bp_length));
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	size_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("size", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "size");
/*[[[end]]]*/
	result = Dee_atomic_read_with_atomic_rwlock(&self->w_printer.wp_uni.up_length,
	                                            &self->w_lock);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
writer_allocated(DeeFileWriterObject *self, size_t argc, DeeObject *const *argv) {
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	STATIC_ASSERT(offsetof(DeeFileWriterObject, w_printer.wp_uni.up_buffer) ==
	              offsetof(DeeFileWriterObject, w_printer.wp_byt.bp_bytes));
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	size_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("allocated", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "allocated");
/*[[[end]]]*/
	DeeFileWriter_LockRead(self);
	if (self->w_printer.wp_uni.up_buffer) {
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		if (DeeFileWriter_IsBytes(self)) {
			result = DeeBytes_SIZE(self->w_printer.wp_byt.bp_bytes);
		} else
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		{
			result = WSTR_LENGTH(self->w_printer.wp_uni.up_buffer);
		}
	} else {
		result = 0;
	}
	DeeFileWriter_LockEndRead(self);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE struct type_method tpconst writer_methods[] = {
	TYPE_METHOD_F(STR_get, &writer_get, METHOD_FNOREFESCAPE,
	              "->?Dstring\n"
	              "Deprecated alias for reading from ?#string"),
	TYPE_METHOD_F("pack", &writer_get, METHOD_FNOREFESCAPE,
	              "->?Dstring\n"
	              "Deprecated alias for reading from ?#string"),
	TYPE_METHOD_F(STR_size, &writer_size, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Return the total amount of written bytes"),
	TYPE_METHOD_F("allocated", &writer_allocated, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Returns the currently allocated buffer size (in bytes)"),
	/* TODO: write()    -- same as File.write(), but automatically set `hint = "bytes"' */
	/* TODO: writeall() -- same as File.writeall(), but automatically set `hint = "bytes"' */
	TYPE_METHOD_END
};

#define UNICODE_PRINTER_INITIAL_BUFSIZE 64

#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
#define BYTES_PRINTER_INITIAL_BUFSIZE UNICODE_PRINTER_INITIAL_BUFSIZE
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
writer_tryappend8_string_locked(DeeFileWriterObject *__restrict self,
                                uint8_t const *__restrict buffer,
                                size_t bufsize) {
	size_t i, written, avail;
	if (!self->w_printer.wp_uni.up_buffer) {
		/* Allocate the initial buffer. */
		size_t init_size = bufsize;
		DeeStringObject *init_buffer;
		if (init_size < UNICODE_PRINTER_INITIAL_BUFSIZE)
			init_size = UNICODE_PRINTER_INITIAL_BUFSIZE;
		init_buffer = (DeeStringObject *)DeeObject_TryMallocc(offsetof(DeeStringObject, s_str),
		                                                      init_size + 1, sizeof(char));
		if unlikely(!init_buffer) {
			init_size   = bufsize;
			init_buffer = (DeeStringObject *)DeeObject_TryMallocc(offsetof(DeeStringObject, s_str),
			                                                      init_size + 1, sizeof(char));
			if unlikely(!init_buffer)
				goto err;
		}
		DeeObject_Init(init_buffer, &DeeString_Type);
		init_buffer->s_data = NULL;
		init_buffer->s_hash = Dee_STRING_HASH_UNSET;
		init_buffer->s_len  = init_size;
		memcpy(init_buffer->s_str, buffer, bufsize);
		self->w_printer.wp_uni.up_buffer = init_buffer->s_str;
		self->w_printer.wp_uni.up_length = bufsize;
		goto ok;
	}
	avail   = WSTR_LENGTH(self->w_printer.wp_uni.up_buffer);
	written = self->w_printer.wp_uni.up_length;
	ASSERT(avail >= written);
	SWITCH_SIZEOF_WIDTH(self->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE:
		if (written + bufsize > avail) {
			/* Must allocate more memory. */
			DeeStringObject *new_buffer;
			size_t new_size = avail;
			do {
				new_size *= 2;
			} while (new_size < written + bufsize);
			new_buffer = (DeeStringObject *)DeeObject_TryReallocc(COMPILER_CONTAINER_OF(self->w_printer.wp_uni.up_buffer, DeeStringObject, s_str),
			                                                      offsetof(DeeStringObject, s_str), new_size + 1, sizeof(char));
			if unlikely(!new_buffer) {
				new_size   = written + bufsize;
				new_buffer = (DeeStringObject *)DeeObject_TryReallocc(COMPILER_CONTAINER_OF(self->w_printer.wp_uni.up_buffer, DeeStringObject, s_str),
				                                                      offsetof(DeeStringObject, s_str), new_size + 1, sizeof(char));
				if unlikely(!new_buffer)
					goto err;
			}
			new_buffer->s_len = new_size;
			self->w_printer.wp_uni.up_buffer = new_buffer->s_str;
		}
		memcpy((uint8_t *)self->w_printer.wp_uni.up_buffer +
		       self->w_printer.wp_uni.up_length,
		       buffer, bufsize);
		self->w_printer.wp_uni.up_length += bufsize;
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
			new_buffer = DeeString_TryResize2ByteBuffer((uint16_t *)self->w_printer.wp_uni.up_buffer, new_size);
			if unlikely(!new_buffer) {
				new_buffer = DeeString_TryResize2ByteBuffer((uint16_t *)self->w_printer.wp_uni.up_buffer, written + bufsize);
				if unlikely(!new_buffer)
					goto err;
			}
			self->w_printer.wp_uni.up_buffer = new_buffer;
		}
		dst = (uint16_t *)self->w_printer.wp_uni.up_buffer;
		dst += self->w_printer.wp_uni.up_length;
		self->w_printer.wp_uni.up_length += bufsize;
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
			new_buffer = DeeString_TryResize4ByteBuffer((uint32_t *)self->w_printer.wp_uni.up_buffer, new_size);
			if unlikely(!new_buffer) {
				new_buffer = DeeString_TryResize4ByteBuffer((uint32_t *)self->w_printer.wp_uni.up_buffer, written + bufsize);
				if unlikely(!new_buffer)
					goto err;
			}
			self->w_printer.wp_uni.up_buffer = new_buffer;
		}
		dst = (uint32_t *)self->w_printer.wp_uni.up_buffer;
		dst += self->w_printer.wp_uni.up_length;
		self->w_printer.wp_uni.up_length += bufsize;
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
writer_tryappendch_string_locked(DeeFileWriterObject *__restrict self, uint32_t ch) {
	size_t written, avail;
	if (!self->w_printer.wp_uni.up_buffer) {
		/* Allocate the initial buffer. */
		if (ch <= 0xff) {
			size_t init_size = UNICODE_PRINTER_INITIAL_BUFSIZE;
			DeeStringObject *init_buffer;
			init_buffer = (DeeStringObject *)DeeObject_TryMallocc(offsetof(DeeStringObject, s_str),
			                                                      init_size + 1, sizeof(char));
			if unlikely(!init_buffer) {
				init_size   = 1;
				init_buffer = (DeeStringObject *)DeeObject_TryMallocc(offsetof(DeeStringObject, s_str),
				                                                      init_size + 1, sizeof(char));
				if unlikely(!init_buffer)
					goto err;
			}
			DeeObject_Init(init_buffer, &DeeString_Type);
			init_buffer->s_data   = NULL;
			init_buffer->s_hash   = Dee_STRING_HASH_UNSET;
			init_buffer->s_len    = init_size;
			init_buffer->s_str[0] = (char)(uint8_t)ch;
			ASSERT((self->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
			self->w_printer.wp_uni.up_buffer = init_buffer->s_str;
			self->w_printer.wp_uni.up_length = 1;
		} else if (ch <= 0xffff) {
			uint16_t *init_buffer;
			init_buffer = DeeString_TryNew2ByteBuffer(UNICODE_PRINTER_INITIAL_BUFSIZE);
			if unlikely(!init_buffer) {
				init_buffer = DeeString_TryNew2ByteBuffer(1);
				if unlikely(!init_buffer)
					goto err;
			}
			init_buffer[0] = (uint16_t)ch;
			self->w_printer.wp_uni.up_buffer = init_buffer;
			self->w_printer.wp_uni.up_length = 1;
			ASSERT((self->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
			self->w_printer.wp_uni.up_flags |= STRING_WIDTH_2BYTE;
		} else {
			uint32_t *init_buffer;
			init_buffer = DeeString_TryNew4ByteBuffer(UNICODE_PRINTER_INITIAL_BUFSIZE);
			if unlikely(!init_buffer) {
				init_buffer = DeeString_TryNew4ByteBuffer(1);
				if unlikely(!init_buffer)
					goto err;
			}
			init_buffer[0] = ch;
			self->w_printer.wp_uni.up_buffer = init_buffer;
			self->w_printer.wp_uni.up_length = 1;
			ASSERT((self->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE);
			self->w_printer.wp_uni.up_flags |= STRING_WIDTH_4BYTE;
		}
		goto ok;
	}

	/* Append to an existing buffer, possibly up-casting that buffer to a greater magnitude. */
	avail   = WSTR_LENGTH(self->w_printer.wp_uni.up_buffer);
	written = self->w_printer.wp_uni.up_length;
	ASSERT(avail >= written);
	SWITCH_SIZEOF_WIDTH(self->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE:
		if (ch <= 0xff) {
			if (written >= avail) {
				/* Must allocate more memory. */
				DeeStringObject *new_buffer;
				size_t new_size = avail;
				do {
					new_size *= 2;
				} while (new_size <= written);
				new_buffer = (DeeStringObject *)DeeObject_TryReallocc(COMPILER_CONTAINER_OF(self->w_printer.wp_uni.up_buffer, DeeStringObject, s_str),
				                                                      offsetof(DeeStringObject, s_str), new_size + 1, sizeof(char));
				if unlikely(!new_buffer) {
					new_size   = written + 1;
					new_buffer = (DeeStringObject *)DeeObject_TryReallocc(COMPILER_CONTAINER_OF(self->w_printer.wp_uni.up_buffer, DeeStringObject, s_str),
					                                                      offsetof(DeeStringObject, s_str), new_size + 1, sizeof(char));
					if unlikely(!new_buffer)
						goto err;
				}
				new_buffer->s_len = new_size;
				self->w_printer.wp_uni.up_buffer = new_buffer->s_str;
			}
			((uint8_t *)self->w_printer.wp_uni.up_buffer)[self->w_printer.wp_uni.up_length] = (uint8_t)ch;
			++self->w_printer.wp_uni.up_length;
			break;
		}
		if (ch <= 0xffff) {
			/* Must up-cast to 16-bit */
			uint16_t *new_buffer;
			size_t i, length;
			length     = self->w_printer.wp_uni.up_length;
			new_buffer = DeeString_TryNew2ByteBuffer(length + 1);
			if unlikely(!new_buffer)
				goto err;
			for (i = 0; i < length; ++i)
				new_buffer[i] = ((uint8_t *)self->w_printer.wp_uni.up_buffer)[i];
			new_buffer[length] = (uint16_t)ch;
			DeeObject_Free(COMPILER_CONTAINER_OF(self->w_printer.wp_uni.up_buffer,
			                                     DeeStringObject,
			                                     s_str));
			Dee_DecrefNokill(&DeeString_Type);
			self->w_printer.wp_uni.up_buffer = new_buffer;
#if STRING_WIDTH_1BYTE != 0
			self->w_printer.wp_uni.up_flags &= ~Dee_UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
			self->w_printer.wp_uni.up_flags |= STRING_WIDTH_2BYTE;
			++self->w_printer.wp_uni.up_length;
		} else {
			/* Must up-cast to 32-bit */
			uint32_t *new_buffer;
			size_t i, length;
			length     = self->w_printer.wp_uni.up_length;
			new_buffer = DeeString_TryNew4ByteBuffer(length + 1);
			if unlikely(!new_buffer)
				goto err;
			for (i = 0; i < length; ++i)
				new_buffer[i] = ((uint8_t *)self->w_printer.wp_uni.up_buffer)[i];
			new_buffer[length] = (uint32_t)ch;
			DeeObject_Free(COMPILER_CONTAINER_OF(self->w_printer.wp_uni.up_buffer,
			                                     DeeStringObject,
			                                     s_str));
			Dee_DecrefNokill(&DeeString_Type);
			self->w_printer.wp_uni.up_buffer = new_buffer;
#if STRING_WIDTH_1BYTE != 0
			self->w_printer.wp_uni.up_flags &= ~Dee_UNICODE_PRINTER_FWIDTH;
#endif /* STRING_WIDTH_1BYTE != 0 */
			self->w_printer.wp_uni.up_flags |= STRING_WIDTH_4BYTE;
			++self->w_printer.wp_uni.up_length;
		}
		break;

	CASE_WIDTH_2BYTE:
		if (ch > 0xffff) {
			/* Must up-cast to 32-bit. */
			uint32_t *new_buffer;
			size_t i, length;
			uint16_t *src;
			length     = self->w_printer.wp_uni.up_length;
			new_buffer = DeeString_TryNew4ByteBuffer(length + 1);
			if unlikely(!new_buffer)
				goto err;
			src = (uint16_t *)self->w_printer.wp_uni.up_buffer;
			for (i = 0; i < length; ++i)
				new_buffer[i] = src[i];
			new_buffer[length]        = ch;
			self->w_printer.wp_uni.up_buffer = new_buffer;
			self->w_printer.wp_uni.up_flags &= ~Dee_UNICODE_PRINTER_FWIDTH;
			self->w_printer.wp_uni.up_flags |= STRING_WIDTH_4BYTE;
			++self->w_printer.wp_uni.up_length;
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
			new_buffer = DeeString_TryResize2ByteBuffer((uint16_t *)self->w_printer.wp_uni.up_buffer, new_size);
			if unlikely(!new_buffer) {
				new_buffer = DeeString_TryResize2ByteBuffer((uint16_t *)self->w_printer.wp_uni.up_buffer, written + 1);
				if unlikely(!new_buffer)
					goto err;
			}
			self->w_printer.wp_uni.up_buffer = new_buffer;
		}
		((uint16_t *)self->w_printer.wp_uni.up_buffer)[self->w_printer.wp_uni.up_length] = (uint16_t)ch;
		++self->w_printer.wp_uni.up_length;
		break;

	CASE_WIDTH_4BYTE:
		if (written >= avail) {
			/* Must allocate more memory. */
			uint32_t *new_buffer;
			size_t new_size = avail;
			do {
				new_size *= 2;
			} while (new_size <= written);
			new_buffer = DeeString_TryResize4ByteBuffer((uint32_t *)self->w_printer.wp_uni.up_buffer, new_size);
			if unlikely(!new_buffer) {
				new_buffer = DeeString_TryResize4ByteBuffer((uint32_t *)self->w_printer.wp_uni.up_buffer, written + 1);
				if unlikely(!new_buffer)
					goto err;
			}
			self->w_printer.wp_uni.up_buffer = new_buffer;
		}
		((uint32_t *)self->w_printer.wp_uni.up_buffer)[self->w_printer.wp_uni.up_length] = ch;
		++self->w_printer.wp_uni.up_length;
		break;
	}
ok:
	return true;
err:
	return false;
}

#ifndef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
INTDEF ATTR_PURE WUNUSED NONNULL((1)) uint32_t DCALL
utf8_getchar(uint8_t const *__restrict base, uint8_t seqlen);
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */

#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
/* @return: 1 : Try again (lock was released)
 * @return: 0 : Success (lock is kept)
 * @return: -1: Error (lock was released) */
PRIVATE WUNUSED NONNULL((1)) int DCALL
writer_bytes_set_allocated_or_unlock(DeeFileWriterObject *__restrict self,
                                     size_t allocated) {
	ASSERT(DeeFileWriter_LockWriting(self));
	ASSERT(self->w_string == (DeeStringObject *)ITER_DONE);
	DREF DeeBytesObject *bytes;
	DREF DeeBytesObject *new_bytes;
	size_t bytes_used, bytes_alloc;
	bytes = self->w_printer.wp_byt.bp_bytes;
	if unlikely(!bytes) {
		ASSERT(self->w_printer.wp_byt.bp_length == 0);
		bytes = DeeBytes_TryNewBufferUninitialized(allocated);
		if likely(bytes) {
			self->w_printer.wp_byt.bp_bytes = bytes;
			return 0;
		}
		DeeFileWriter_LockEndWrite(self);
		bytes = DeeBytes_NewBufferUninitialized(allocated);
		if unlikely(!bytes)
			goto err;
		DeeFileWriter_LockWrite(self);
		if likely(self->w_string == (DeeStringObject *)ITER_DONE &&
		          self->w_printer.wp_byt.bp_bytes == NULL) {
			self->w_printer.wp_byt.bp_bytes = bytes;
			DeeFileWriter_LockEndWrite(self);
		} else {
			DeeFileWriter_LockEndWrite(self);
			DeeBytes_Destroy(bytes);
		}
		return 1;
	}

	/* Check if we need to unshare/copy "bytes" */
	bytes_used = self->w_printer.wp_byt.bp_length;
	if (bytes_used >= allocated)
		return 0;
	if (DeeObject_IsShared(bytes) || (bytes->b_orig != Dee_AsObject(bytes))) {
		new_bytes = DeeBytes_TryNewBufferUninitialized(allocated);
		if unlikely(!new_bytes) {
			DeeFileWriter_LockEndWrite(self);
			new_bytes = DeeBytes_NewBufferUninitialized(allocated);
			if unlikely(!new_bytes)
				goto err;
			DeeFileWriter_LockWrite(self);
			if unlikely(self->w_string != (DeeStringObject *)ITER_DONE) {
unlock_and_destroy_new_bytes_and_try_again:
				DeeFileWriter_LockEndWrite(self);
				DeeBytes_Destroy(new_bytes);
				return 1;
			}
			if unlikely(self->w_printer.wp_byt.bp_bytes != bytes)
				goto unlock_and_destroy_new_bytes_and_try_again;
			if unlikely(self->w_printer.wp_byt.bp_length != bytes_used)
				goto unlock_and_destroy_new_bytes_and_try_again;
		}
		memcpy(new_bytes->b_buffer, DeeBytes_DATA(bytes), bytes_used);
		ASSERT(self->w_printer.wp_byt.bp_bytes == bytes);
		self->w_printer.wp_byt.bp_bytes = new_bytes;
		if likely(Dee_DecrefIfNotOne(bytes))
			return 0;
		DeeFileWriter_LockEndWrite(self);
		Dee_Decref(bytes);
		return 1;
	}

	/* Check how much space is available in "bytes" right now */
	bytes_alloc = DeeBytes_SIZE(bytes);
	ASSERT(bytes_used <= bytes_alloc);
	if (bytes_alloc >= allocated)
		return 0; /* Already enough memory allocated! */

	/* Resize to get more memory */
	new_bytes = DeeBytes_TryResizeBuffer(bytes, allocated);
	if unlikely(!new_bytes) {
		DeeFileWriter_LockEndWrite(self);
		new_bytes = DeeBytes_NewBufferUninitialized(allocated);
		if unlikely(!new_bytes)
			goto err;
		DeeFileWriter_LockWrite(self);
		if unlikely(self->w_string != (DeeStringObject *)ITER_DONE)
			goto unlock_and_destroy_new_bytes_and_try_again;
		if unlikely(self->w_printer.wp_byt.bp_bytes != bytes)
			goto unlock_and_destroy_new_bytes_and_try_again;
		if unlikely(self->w_printer.wp_byt.bp_length != bytes_used)
			goto unlock_and_destroy_new_bytes_and_try_again;
		memcpy(new_bytes->b_buffer, bytes->b_buffer, bytes_used);
		DeeBytes_Destroy(bytes);
	}
	self->w_printer.wp_byt.bp_bytes = new_bytes;
	bytes = new_bytes;
	ASSERT(DeeBytes_SIZE(bytes) >= bytes_used);
	ASSERT(DeeBytes_SIZE(bytes) >= allocated);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
writer_write(DeeFileWriterObject *__restrict self,
             uint8_t const *__restrict buffer,
             size_t bufsize, Dee_ioflag_t UNUSED(flags)) {
	size_t result = bufsize;
	uint8_t *flush_start, *iter, *end;
again:
	DeeFileWriter_LockWrite(self);
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
again_locked:
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	if (self->w_string) {
		DeeStringObject *wstr = self->w_string;
		unsigned int width;
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		if (wstr == (DeeStringObject *)ITER_DONE) {
			DREF DeeBytesObject *bytes;
			DREF DeeBytesObject *new_bytes;
			size_t bytes_used, bytes_alloc, bytes_avail;
			bytes = self->w_printer.wp_byt.bp_bytes;
			if unlikely(!bytes) {
				size_t avail = bufsize * 2;
				if (avail < bufsize)
					avail = bufsize;
				if (avail < BYTES_PRINTER_INITIAL_BUFSIZE)
					avail = BYTES_PRINTER_INITIAL_BUFSIZE;
				ASSERT(self->w_printer.wp_byt.bp_length == 0);
				bytes = DeeBytes_TryNewBufferUninitialized(avail);
				if likely(bytes) {
					memcpy(bytes->b_buffer, buffer, bufsize);
set_initial_bytes_and_unlock:
					self->w_printer.wp_byt.bp_bytes  = bytes;
					self->w_printer.wp_byt.bp_length = bufsize;
					goto done_unlock;
				}
				bytes = DeeBytes_TryNewBufferData(buffer, bufsize);
				if likely(bytes)
					goto set_initial_bytes_and_unlock;
				DeeFileWriter_LockEndWrite(self);
				bytes = DeeBytes_NewBufferData(buffer, bufsize);
				if unlikely(!bytes)
					goto err;
				DeeFileWriter_LockWrite(self);
				if likely(self->w_string == (DeeStringObject *)ITER_DONE &&
				          self->w_printer.wp_byt.bp_bytes == NULL)
					goto set_initial_bytes_and_unlock;
				DeeFileWriter_LockEndWrite(self);
				DeeBytes_Destroy(bytes);
				goto again;
			}

			/* Check if we need to unshare/copy "bytes" */
			bytes_used = self->w_printer.wp_byt.bp_length;
			if (DeeObject_IsShared(bytes) || (bytes->b_orig != Dee_AsObject(bytes))) {
				size_t min_size, new_size;
				min_size = bytes_used + bufsize;
				new_size = bytes_used * 2;
				if (new_size < min_size)
					new_size = min_size;
				if (new_size < BYTES_PRINTER_INITIAL_BUFSIZE)
					new_size = BYTES_PRINTER_INITIAL_BUFSIZE;
				new_bytes = DeeBytes_TryNewBufferUninitialized(new_size);
				if unlikely(!new_bytes) {
					new_size = min_size;
					new_bytes = DeeBytes_TryNewBufferUninitialized(new_size);
					if unlikely(!new_bytes) {
						DeeFileWriter_LockEndWrite(self);
						new_bytes = DeeBytes_NewBufferUninitialized(new_size);
						if unlikely(!new_bytes)
							goto err;
						DeeFileWriter_LockWrite(self);
						if unlikely(self->w_string != (DeeStringObject *)ITER_DONE) {
unlock_and_destroy_new_bytes_and_try_again:
							DeeFileWriter_LockEndWrite(self);
							DeeBytes_Destroy(new_bytes);
							goto again;
						}
						if unlikely(self->w_printer.wp_byt.bp_bytes != bytes)
							goto unlock_and_destroy_new_bytes_and_try_again;
						if unlikely(self->w_printer.wp_byt.bp_length != bytes_used)
							goto unlock_and_destroy_new_bytes_and_try_again;
					}
				}
				memcpy(mempcpy(new_bytes->b_buffer, DeeBytes_DATA(bytes), bytes_used),
				       buffer, bufsize);
				ASSERT(self->w_printer.wp_byt.bp_bytes == bytes);
				self->w_printer.wp_byt.bp_bytes  = new_bytes;
				self->w_printer.wp_byt.bp_length = min_size;
				DeeFileWriter_LockEndWrite(self);
				Dee_Decref(bytes);
				goto done;
			}

			/* Check how much space is available in "bytes" right now */
			bytes_alloc = DeeBytes_SIZE(bytes);
			ASSERT(bytes_used <= bytes_alloc);
			bytes_avail = bytes_alloc - bytes_used;
			if (bytes_avail < bufsize) {
				size_t new_alloc = bytes_alloc * 2;
				size_t min_alloc = bytes_used + bufsize;
				if (new_alloc < min_alloc)
					new_alloc = min_alloc;
				if (new_alloc < BYTES_PRINTER_INITIAL_BUFSIZE)
					new_alloc = BYTES_PRINTER_INITIAL_BUFSIZE;
				new_bytes = DeeBytes_TryResizeBuffer(bytes, new_alloc);
				if unlikely(!new_bytes) {
					new_alloc = min_alloc;
					new_bytes = DeeBytes_TryResizeBuffer(bytes, new_alloc);
					if unlikely(!new_bytes) {
						DeeFileWriter_LockEndWrite(self);
						new_bytes = DeeBytes_NewBufferUninitialized(new_alloc);
						if unlikely(!new_bytes)
							goto err;
						DeeFileWriter_LockWrite(self);
						if unlikely(self->w_string != (DeeStringObject *)ITER_DONE)
							goto unlock_and_destroy_new_bytes_and_try_again;
						if unlikely(self->w_printer.wp_byt.bp_bytes != bytes)
							goto unlock_and_destroy_new_bytes_and_try_again;
						if unlikely(self->w_printer.wp_byt.bp_length != bytes_used)
							goto unlock_and_destroy_new_bytes_and_try_again;
						memcpy(new_bytes->b_buffer, bytes->b_buffer, bytes_used);
						DeeBytes_Destroy(bytes);
					}
				}
				self->w_printer.wp_byt.bp_bytes = new_bytes;
				bytes = new_bytes;
			}
			ASSERT(DeeBytes_SIZE(bytes) >= bytes_used);
			ASSERT((DeeBytes_SIZE(bytes) - bytes_used) >= bufsize);

			/* Append data to bytes buffer */
			memcpy(bytes->b_buffer + bytes_used, buffer, bufsize);
			self->w_printer.wp_byt.bp_length += bufsize;
			goto done_unlock;
		}
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		width = self->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH;
		ASSERT(DeeString_WIDTH(wstr) != STRING_WIDTH_1BYTE);
		ASSERT(DeeString_WIDTH(wstr) == width);
		ASSERT(self->w_printer.wp_uni.up_buffer == DeeString_WSTR(wstr));
		ASSERT(self->w_printer.wp_uni.up_length == DeeString_WLEN(wstr));
		if (DeeObject_IsShared(wstr)) {
			/* Unshare the pre-written multi-byte buffer. */
			if (width == STRING_WIDTH_2BYTE) {
				size_t length = self->w_printer.wp_uni.up_length;
				uint16_t *buffer_copy = DeeString_TryNew2ByteBuffer(length + bufsize);
				if unlikely(!buffer_copy) {
					DeeFileWriter_LockEndWrite(self);
					if (Dee_CollectMemoryoc(sizeof(size_t), length + bufsize + 1, 2))
						goto again;
					goto err;
				}
				memcpyw(buffer_copy, self->w_printer.wp_uni.up_buffer, length);
				self->w_printer.wp_uni.up_buffer = buffer_copy; /* Inherit data */
			} else {
				size_t length = self->w_printer.wp_uni.up_length;
				uint32_t *buffer_copy = DeeString_TryNew4ByteBuffer(length + bufsize);
				if unlikely(!buffer_copy) {
					DeeFileWriter_LockEndWrite(self);
					if (Dee_CollectMemoryoc(sizeof(size_t), length + bufsize + 1, 4))
						goto again;
					goto err;
				}
				memcpyl(buffer_copy, self->w_printer.wp_uni.up_buffer, length);
				self->w_printer.wp_uni.up_buffer = buffer_copy; /* Inherit data */
			}

			/* Drop our reference to the pre-packed string. */
			self->w_string = NULL;
			DeeFileWriter_LockEndWrite(self);
			Dee_Decref_unlikely(wstr);
			goto again;
		} else {
			/* The string isn't being shared, so we can just discard all unused data,
			 * and keep on appending to the pre-generated multi-byte buffer. */
			struct Dee_string_utf *utf = wstr->s_data;
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
	} else if (self->w_printer.wp_uni.up_buffer &&
	           (self->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FWIDTH) == STRING_WIDTH_1BYTE) {
		DeeStringObject *written_buffer;
		written_buffer = COMPILER_CONTAINER_OF(self->w_printer.wp_uni.up_buffer,
		                                       DeeStringObject,
		                                       s_str);
		ASSERT(DeeString_WIDTH(written_buffer) == STRING_WIDTH_1BYTE);
		if unlikely(DeeObject_IsShared(written_buffer)) {
			/* Unshare the already written portion of the buffer. */
			DeeStringObject *buffer_copy;
			size_t buffer_length = self->w_printer.wp_uni.up_length;
			ASSERT(buffer_length == DeeString_SIZE(written_buffer));
			buffer_copy = (DeeStringObject *)DeeObject_TryMallocc(offsetof(DeeStringObject, s_str),
			                                                      buffer_length + bufsize + 1,
			                                                      sizeof(char));
			if unlikely(!buffer_copy) {
				DeeFileWriter_LockEndWrite(self);
				if (Dee_CollectMemoryoc(offsetof(DeeStringObject, s_str),
				                        buffer_length + bufsize + 1,
				                        sizeof(char)))
					goto again;
				goto err;
			}
			DeeObject_Init(buffer_copy, &DeeString_Type);
			buffer_copy->s_len  = buffer_length;
			buffer_copy->s_data = NULL;
			buffer_copy->s_hash = Dee_STRING_HASH_UNSET;
			self->w_printer.wp_uni.up_buffer = (char *)memcpyc(buffer_copy->s_str, written_buffer->s_str,
			                                                   self->w_printer.wp_uni.up_length, sizeof(char));
			DeeFileWriter_LockEndWrite(self);
			Dee_Decref_unlikely(written_buffer);
			goto again;
		}
	}

	/* At this point, we know that the buffer has been locked, and that
	 * the pre-written string has been unshared. - Now we can actually
	 * get to work and start appending the new content! */
	if (self->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FPENDING) {
		/* Complete a UTF-8 sequence. */
		uint8_t seqlen = Dee_unicode_utf8seqlen[self->w_printer.wp_uni.up_pend[0]];
		uint8_t gotlen = (self->w_printer.wp_uni.up_flags & Dee_UNICODE_PRINTER_FPENDING) >> Dee_UNICODE_PRINTER_FPENDING_SHFT;
		uint8_t missing, full_sequence[Dee_UNICODE_UTF8_MAXLEN], *tempptr;
		uint32_t ch32;
		ASSERT(gotlen < seqlen);
		missing = seqlen - gotlen;
		if (missing > bufsize) {
			/* Append what we got, but that won't be all of it... */
			memcpy(self->w_printer.wp_uni.up_pend + gotlen, buffer, bufsize);
			self->w_printer.wp_uni.up_flags += (uint8_t)bufsize << Dee_UNICODE_PRINTER_FPENDING_SHFT;
			goto done_unlock;
		}

		/* Complete the sequence, and append the character. */
		tempptr = (uint8_t *)mempcpy(full_sequence, self->w_printer.wp_uni.up_pend, gotlen);
		memcpy(tempptr, buffer, missing);
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		tempptr = full_sequence;
		ch32 = Dee_unicode_readutf8_chk(&tempptr);
		if unlikely(!ch32)
			goto convert_to_bytes;
#else /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		ch32 = utf8_getchar(full_sequence, seqlen);
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		if (!writer_tryappendch_string_locked(self, ch32)) {
			DeeFileWriter_LockEndWrite(self);
			if (Dee_CollectMemory(4))
				goto again;
		}
		self->w_printer.wp_uni.up_flags &= ~Dee_UNICODE_PRINTER_FPENDING;
		buffer += missing;
		bufsize -= missing;
	}

	flush_start = iter = (uint8_t *)buffer;
	end = iter + bufsize;
	while (iter < end) {
		uint8_t seqlen;
		uint32_t ch32;

		/* Search for UTF-8 byte sequences */
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		if (*iter < 0x80)
#else /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		if (*iter < 0xc0)
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		{
			++iter;
			continue;
		}
		if (flush_start < iter &&
		    !writer_tryappend8_string_locked(self, flush_start, (size_t)(iter - flush_start))) {
			DeeFileWriter_LockEndWrite(self);
			buffer  = flush_start;
			bufsize = (size_t)(end - flush_start);
			if (Dee_CollectMemory((size_t)(iter - flush_start)))
				goto again;
			goto err;
		}

		/* Goto a multi-byte sequence. */
		seqlen = Dee_unicode_utf8seqlen[*iter];
#ifndef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		ASSERT(seqlen);
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		if (seqlen > (size_t)(end - iter)) {
			/* Incomplete sequence (remember the portion already given) */
			seqlen = (uint8_t)(end - iter);
			self->w_printer.wp_uni.up_flags |= seqlen << Dee_UNICODE_PRINTER_FPENDING_SHFT;
			memcpy(self->w_printer.wp_uni.up_pend, iter, seqlen);
			goto done_unlock;
		}

		/* The full sequence has been given! */
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
		if unlikely(!seqlen) {
convert_to_bytes_at_iter:
			buffer  = iter;
			bufsize = (size_t)(end - iter);
			goto convert_to_bytes;
		}
		ch32 = Dee_unicode_readutf8_chk(&iter);
		if unlikely(!ch32) {
			--iter;
			goto convert_to_bytes_at_iter;
		}
		if (!writer_tryappendch_string_locked(self, ch32)) {
			DeeFileWriter_LockEndWrite(self);
			if (!Dee_CollectMemory(4))
				goto err;
			iter -= seqlen;
			buffer  = iter;
			bufsize = (size_t)(end - iter);
			goto again;
		}
		flush_start = iter;
#else /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
		ch32 = utf8_getchar(iter, seqlen);
		if (!writer_tryappendch_string_locked(self, ch32)) {
			DeeFileWriter_LockEndWrite(self);
			if (!Dee_CollectMemory(4))
				goto err;
			buffer  = iter;
			bufsize = (size_t)(end - iter);
			goto again;
		}
		iter += seqlen;
		flush_start = iter;
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	}

	/* Flush the remainder. */
	if (flush_start < end &&
	    !writer_tryappend8_string_locked(self, flush_start, (size_t)(end - flush_start))) {
		DeeFileWriter_LockEndWrite(self);
		buffer  = flush_start;
		bufsize = (size_t)(end - flush_start);
		if (Dee_CollectMemory(bufsize))
			goto again;
		goto err;
	}
done_unlock:
	DeeFileWriter_LockEndWrite(self);
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
done:
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	return result;
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	{
		int status;
convert_to_bytes:
		status = DeeFileWriter_String2BytesOrUnlock(self);
		if likely(status >= 0) {
			if (status == 0)
				goto again_locked;
			goto again;
		}
	}
#endif /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
err:
	return (size_t)-1;
}


PRIVATE WUNUSED NONNULL((1)) ATTR_OUTS(2, 3) size_t DCALL
writer_pread(DeeFileWriterObject *self, void *buffer,
             size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags) {
	size_t result = 0;
	DREF DeeBytesObject *bytes;
	(void)flags;
	bytes = (DREF DeeBytesObject *)DeeFileWriter_GetBytes((DeeObject *)self);
	if unlikely(!bytes)
		goto err;
	if (pos < (Dee_pos_t)DeeBytes_SIZE(bytes)) {
		result = (size_t)((Dee_pos_t)DeeBytes_SIZE(bytes) - pos);
		if (result > bufsize)
			result = bufsize;
		memcpy(buffer, DeeBytes_DATA(bytes) + (size_t)pos, result);
	}
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL
writer_pwrite(DeeFileWriterObject *self, void const *buffer,
              size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags) {
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	int status;
	size_t old_length;
	void *base;
	(void)flags;

#if Dee_SIZEOF_POS_T > __SIZEOF_SIZE_T__
	if (pos >= (Dee_pos_t)(size_t)-1)
		return 0;
#endif /* Dee_SIZEOF_POS_T > __SIZEOF_SIZE_T__ */
	if (pos != 0) {
		size_t max_write = (size_t)0 - (size_t)pos;
		if (bufsize > max_write)
			bufsize = max_write;
	}
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeFileWriter_Type);
again:
	DeeFileWriter_LockWrite(self);
	if (!DeeFileWriter_IsBytes(self)) {
		status = DeeFileWriter_String2BytesOrUnlock(self);
		if (status != 0) {
			if unlikely(status < 0)
				goto err;
			goto again;
		}
	}

	/* Ensure that bytes have been allocated far enough along for us to be
	 * able to perform the write!
	 * If the caller's `pos' is something unreasonable, this will just OOM. */
	status = writer_bytes_set_allocated_or_unlock(self, (size_t)pos + bufsize);
	if (status != 0) {
		if unlikely(status < 0)
			goto err;
		goto again;
	}

	/* bzero-initialize memory between the current (old) EOF and the caller-given `pos' */
	old_length = self->w_printer.wp_byt.bp_length;
	base = DeeBytes_DATA(self->w_printer.wp_byt.bp_bytes);
	ASSERT(DeeBytes_SIZE(self->w_printer.wp_byt.bp_bytes) >= ((size_t)pos + bufsize));
	if (old_length < (size_t)pos) {
		size_t num_bzero = (size_t)pos - old_length;
		bzero((byte_t *)base + old_length, num_bzero);
	}

	/* Copy the caller's intended data into the file. */
	memcpy((byte_t *)base + (size_t)pos, buffer, bufsize);

	DeeFileWriter_LockEndWrite(self);
	return bufsize;
err:
	return (size_t)-1;
#else /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)pos;
	(void)flags;
	return (size_t)DeeError_NOTIMPLEMENTED();
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
}

#ifndef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
#define writer_deldata writer_delstring
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */

#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
#define IF_WRITER_BYTES(x) x
#else /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
#define IF_WRITER_BYTES(x) /* nothing */
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */

PUBLIC DeeFileTypeObject DeeFileWriter_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_FileWriter",
		/* .tp_doc      = */ DOC("()\n"
		                         "(" writer__FileWriter_params ")"
		                         "#pdata{The initial contents of the file}"
		                         "#phint{Hints at how written data will eventually be retrieved. "
		                         /*  */ "One of $\"string\" (for ?#string)"
		                         /*  */ IF_WRITER_BYTES(", $\"bytes\" (for ?#bytes)")
		                         /*  */ IF_WRITER_BYTES(" or $\"data\" (for ?#data)")
		                         /*  */ ". Note that this is merely a hint, and that all methods "
		                         /*  */ "available for retrieving the contents of the file will "
		                         /*  */ "always work, no matter what hint was specified}"),
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONLOOPING,
		/* .tp_base     = */ &DeeFile_Type.ft_base,
		/* .tp_init = */ {
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
				/* T:              */ DeeFileWriterObject,
				/* tp_ctor:        */ &writer_ctor,
				/* tp_copy_ctor:   */ &writer_copy,
				/* tp_deep_ctor:   */ NULL,
				/* tp_any_ctor:    */ &writer_init,
				/* tp_any_ctor_kw: */ &writer_init_kw,
				/* tp_serialize:   */ &writer_serialize
			),
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&writer_fini,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str       = */ NULL,
			/* .tp_repr      = */ NULL,
			/* .tp_bool      = */ NULL,
			/* .tp_print     = */ NULL,
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&writer_printrepr,
		},
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&writer_visit,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_iterator      = */ NULL,
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
	/* .ft_write  = */ (size_t (DCALL *)(DeeFileObject *__restrict, void const *__restrict, size_t, Dee_ioflag_t))&writer_write,
	/* .ft_seek   = */ NULL,
	/* .ft_sync   = */ NULL,
	/* .ft_trunc  = */ NULL,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *__restrict))&writer_deldata,
	/* .ft_pread  = */ (size_t (DCALL *)(DeeFileObject *__restrict, void *, size_t, Dee_pos_t, Dee_ioflag_t))&writer_pread,
	/* .ft_pwrite = */ (size_t (DCALL *)(DeeFileObject *__restrict, void const *, size_t, Dee_pos_t, Dee_ioflag_t))&writer_pwrite,
	/* .ft_getc   = */ NULL,
	/* .ft_ungetc = */ NULL,
	/* .ft_putc   = */ NULL
};

/* Open a new file stream that writes all written data into a string.
 * @param: hint: One of `Dee_FILE_WRITER_HINT_*' */
PUBLIC WUNUSED DREF /*File*/ DeeObject *DCALL
DeeFileWriter_New(unsigned int hint) {
	DREF DeeFileWriterObject *result;
	result = DeeObject_MALLOC(DeeFileWriterObject);
	if unlikely(!result)
		goto done;
	Dee_atomic_rwlock_init(&result->w_lock);
	Dee_unicode_printer_init(&result->w_printer.wp_uni);
	result->w_string = NULL;
#ifdef CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES
	if (hint == Dee_FILE_WRITER_HINT_BYTES)
		result->w_string = (DREF DeeStringObject *)ITER_DONE;
#else /* CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	(void)hint;
#endif /* !CONFIG_EXPERIMENTAL_FILE_WRITER_BYTES */
	DeeFileObject_Init(result, &DeeFileWriter_Type);
done:
	return Dee_AsObject(result);
}






/************************************************************************/
/* FILE PRINTER                                                         */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
printer_write(DeeFilePrinterObject *__restrict self,
              uint8_t const *__restrict buffer,
              size_t bufsize, Dee_ioflag_t UNUSED(flags)) {
	Dee_ssize_t status;
	if (DeeFilePrinter_LockRead(self))
		goto err;
	if unlikely(!self->fp_printer) {
		/* Printer was already closed */
		DeeFilePrinter_LockEndRead(self);
		goto err_closed;
	}
	/* Write to the underlying printer. */
	status = (*self->fp_printer)(self->fp_arg, (char const *)buffer, bufsize);
	if likely(status > 0)
		atomic_add(&self->fp_result, (size_t)status);
	DeeFilePrinter_LockEndRead(self);
	if unlikely(status < 0)
		goto err;
	return bufsize;
err_closed:
	err_file_closed();
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
printer_close(DeeFilePrinterObject *__restrict self) {
	if (DeeFilePrinter_LockWrite(self))
		goto err;
	if unlikely(self->fp_printer == NULL) {
		DeeFilePrinter_LockEndWrite(self);
		goto err_closed;
	}
	self->fp_printer = NULL;
	DeeFilePrinter_LockEndWrite(self);
	return 0;
err_closed:
	err_file_closed();
err:
	return -1;
}

PUBLIC DeeFileTypeObject DeeFilePrinter_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_FilePrinter",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ &DeeFile_Type.ft_base,
		/* .tp_init = */ {
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
				/* T:              */ DeeFilePrinterObject,
				/* tp_ctor:        */ NULL,
				/* tp_copy_ctor:   */ NULL,
				/* tp_deep_ctor:   */ NULL,
				/* tp_any_ctor:    */ NULL,
				/* tp_any_ctor_kw: */ NULL,
				/* tp_serialize:   */ NULL /* Not serializable (wouldn't work with `DeeFilePrinter_Close()') */
			),
			/* .tp_dtor        = */ NULL,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str  = */ NULL,
			/* .tp_repr = */ NULL,
			/* .tp_bool = */ NULL
		},
		/* .tp_visit         = */ NULL,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_iterator      = */ NULL,
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
	/* .ft_read   = */ NULL,
	/* .ft_write  = */ (size_t (DCALL *)(DeeFileObject *__restrict, void const *__restrict, size_t, Dee_ioflag_t))&printer_write,
	/* .ft_seek   = */ NULL,
	/* .ft_sync   = */ NULL,
	/* .ft_trunc  = */ NULL,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *__restrict))&printer_close,
	/* .ft_pread  = */ NULL,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ NULL,
	/* .ft_ungetc = */ NULL,
	/* .ft_putc   = */ NULL
};

/* Construct a new printer-wrapper for `printer' and `arg' */
PUBLIC WUNUSED NONNULL((1)) DREF /*FilePrinter*/ DeeObject *DCALL
DeeFilePrinter_New(Dee_formatprinter_t printer, void *arg) {
	DREF DeeFilePrinterObject *result;
	result = DeeObject_MALLOC(DeeFilePrinterObject);
	if unlikely(!result)
		goto done;
	result->fp_printer = printer;
	result->fp_arg     = arg;
	result->fp_result  = 0;
	Dee_shared_rwlock_init(&result->fp_lock);
	DeeFileObject_Init(result, &DeeFilePrinter_Type);
done:
	return Dee_AsObject(result);
}

/* Drop the primary reference from `self'.
 *
 * This function tries to destroy `self', but if that fails (because
 * the object is still being shared), it will acquire a write-lock to
 * `self' (without serving interrupts), and then proceed to delete
 * the linked printer.
 * 
 * @return: * : The total sum of return values of the underlying printer. */
PUBLIC NONNULL((1)) size_t DCALL
DeeFilePrinter_Close(/*inherit(always)*/ DREF /*FilePrinter*/ DeeObject *__restrict self) {
	DREF DeeFilePrinterObject *me = (DREF DeeFilePrinterObject *)self;
	size_t result = atomic_read(&me->fp_result); /* TODO: _with_shared_rwlock */
	ASSERT_OBJECT_TYPE_EXACT((DeeObject *)me, (DeeTypeObject *)&DeeFilePrinter_Type);
	if (!Dee_DecrefIfOne(me)) {
		DeeFilePrinter_LockWriteNoInt(me);
		me->fp_printer = NULL;
		result = me->fp_result;
		DeeFilePrinter_LockEndWrite(me);
		Dee_Decref_unlikely(me);
	}
	return result;
}








/************************************************************************/
/* MMAP API                                                             */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1)) int DCALL
mapfile_init_kw(DeeMapFileObject *__restrict self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	unsigned int mapflags;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("mapfile", params: "
	DeeObject *fd: ?\" FDTYP_mapfile_init_kw \";
	size_t minbytes  = (size_t)0;
	size_t maxbytes  = (size_t)-1;
	Dee_pos_t offset = (Dee_pos_t)-1;
	size_t nulbytes  = 0;
	bool readall     = false;
	bool mustmmap    = false;
	bool mapshared   = false;
", docStringPrefix: "mapfile");]]]*/
#define mapfile_mapfile_params "fd:?" FDTYP_mapfile_init_kw ",minbytes=!0,maxbytes=!-1,offset=!-1,nulbytes=!0,readall=!f,mustmmap=!f,mapshared=!f"
	struct {
		DeeObject *fd;
		size_t minbytes;
		size_t maxbytes;
		Dee_pos_t offset;
		size_t nulbytes;
		bool readall;
		bool mustmmap;
		bool mapshared;
	} args;
	args.minbytes = (size_t)0;
	args.maxbytes = (size_t)-1;
	args.offset = (Dee_pos_t)-1;
	args.nulbytes = 0;
	args.readall = false;
	args.mustmmap = false;
	args.mapshared = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__fd_minbytes_maxbytes_offset_nulbytes_readall_mustmmap_mapshared, "o|" UNPuSIZ UNPxSIZ UNPxN(Dee_SIZEOF_POS_T) UNPuSIZ "bbb:mapfile", &args))
		goto err;
/*[[[end]]]*/
	mapflags = 0;
	/* Package flags */
	if (args.readall)
		mapflags |= DeeMapFile_F_READALL;
	if (args.mustmmap)
		mapflags |= DeeMapFile_F_MUSTMMAP;
	if (args.mapshared) {
		mapflags |= DeeMapFile_F_MUSTMMAP | DeeMapFile_F_MAPSHARED;
		if (args.nulbytes != 0) {
			return DeeError_Throwf(&DeeError_ValueError,
			                       "Cannot use `mapshared = true' with non-zero `nulbytes = %" PRFuSIZ "'",
			                       args.nulbytes);
		}
	}

	/* Construct the mapfile. */
	{
		int error;
#if defined(Dee_fd_t_IS_HANDLE) && defined(CONFIG_HOST_WINDOWS)
#define FDTYP_mapfile_init_kw "X3?DFile?Dint?Ewin32:HANDLE"
		if (!DeeFile_Check(args.fd)) {
			void *sysfd = DeeNTSystem_GetHandle(args.fd);
			if unlikely(sysfd == (void *)-1)
				goto err;
			error = DeeMapFile_InitSysFd(&self->mf_map, sysfd,
			                             args.offset, args.minbytes,
			                             args.maxbytes, args.nulbytes,
			                             mapflags);
		} else
#elif defined(Dee_fd_t_IS_int)
#define FDTYP_mapfile_init_kw "X2?DFile?Dint"
		if (!DeeFile_Check(args.fd)) {
			int sysfd = DeeUnixSystem_GetFD(args.fd);
			if unlikely(sysfd == -1)
				goto err;
			error = DeeMapFile_InitSysFd(&self->mf_map, sysfd,
			                             args.offset, args.minbytes,
			                             args.maxbytes, args.nulbytes,
			                             mapflags);
		} else
#endif /* ... */
		{
#ifndef FDTYP_mapfile_init_kw
#define FDTYP_mapfile_init_kw "DFile"
#endif /* !FDTYP_mapfile_init_kw */
			error = DeeMapFile_InitFile(&self->mf_map, args.fd,
			                            args.offset, args.minbytes,
			                            args.maxbytes, args.nulbytes,
			                            mapflags);
		}
		if unlikely(error)
			goto err;
	}
	self->mf_rsize = DeeMapFile_GetSize(&self->mf_map) + args.nulbytes;
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
	info->bb_base = DeeMapFile_GetAddr(&self->mf_map);
	info->bb_size = self->mf_rsize;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapfile_bytes(DeeMapFileObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("bytes", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "bytes");
/*[[[end]]]*/
	return DeeBytes_NewView(Dee_AsObject(self),
	                        DeeMapFile_GetAddr(&self->mf_map),
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
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FNORMAL
};

PRIVATE struct type_method tpconst mapfile_methods[] = {
	TYPE_METHOD("bytes", &mapfile_bytes,
	            "->?DBytes\n"
	            "Same as ${Bytes(this)}"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst mapfile_getsets[] = {
	TYPE_GETTER_AB_F("ismmap", &mapfile_get_ismmap, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this file map uses mmap to implement its buffer"),
	TYPE_GETSET_END
};


PUBLIC DeeTypeObject DeeMapFile_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapFile",
	/* .tp_doc      = */ DOC("(" mapfile_mapfile_params ")"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeMapFileObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ &mapfile_init_kw,
			/* tp_serialize:   */ NULL /* Not serializable since "struct DeeMapFile" isn't (always) serializable */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mapfile_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&object_repr),
		/* .tp_bool = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default__printrepr__with__repr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ DEFIMPL_UNSUPPORTED(&default__tp_cmp__8F384E6A64571883),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ &mapfile_buffer,
	/* .tp_methods       = */ mapfile_methods,
	/* .tp_getsets       = */ mapfile_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_FILETYPES_C */
