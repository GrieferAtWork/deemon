/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_FILE_TYPE_OPERATORS_C
#define GUARD_DEEMON_OBJECTS_FILE_TYPE_OPERATORS_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/file.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
/**/

#include "../runtime/runtime_error.h"
#include "file-type-operators.h"

DECL_BEGIN

typedef DeeTypeObject Type;

#define OPNAME(opname) "operator " opname

#define DEFINE_OPERATOR_INVOKE(name, instance_name)                                                                 \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                                                           \
	invoke_##name(DeeTypeObject *tp_self, DeeObject *self, /*0..1*/ DREF DeeObject **p_self,                        \
	              size_t argc, DeeObject *const *argv);                                                             \
	PRIVATE struct Dee_operator_invoke tpconst name = { &invoke_##name, (dfunptr_t)(void const *)(instance_name) }; \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                                                           \
	invoke_##name(DeeTypeObject *tp_self, DeeObject *self, /*0..1*/ DREF DeeObject **p_self,                        \
	              size_t argc, DeeObject *const *argv)

/* >> operator read(buf: <Buffer>): int;
 * >> operator read(buf: <Buffer>, max_size: int): int;
 * >> operator read(buf: <Buffer>, start: int, end: int): int;
 * >> operator read(max_bytes: int): Bytes; 
 * >> operator read(): Bytes; */
DEFINE_OPERATOR_INVOKE(operator_read, NULL) { /* TODO: Allow user-code to override this operator */
	DeeObject *data  = NULL;
	DeeObject *begin = NULL;
	DeeObject *end   = NULL;
	DeeBuffer buf;
	size_t buf_begin, buf_end;
	size_t result;
	(void)tp_self; /* TODO: Must look at this! */
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, "|ooo:" OPNAME("read"), &data, &begin, &end))
		goto err;
	if (!data)
		return DeeFile_ReadBytes(self, (size_t)-1, false);
	if (end) {
		if (DeeObject_AsSSize(begin, (dssize_t *)&buf_begin))
			goto err;
		if (DeeObject_AsSSize(end, (dssize_t *)&buf_end))
			goto err;
	} else if (begin) {
		if (DeeObject_AsSSize(begin, (dssize_t *)&buf_end))
			goto err;
		buf_begin = 0;
	} else {
		if (DeeInt_Check(data)) {
			size_t max_bytes;
			if (DeeInt_AsSize(data, &max_bytes))
				goto err;
			return DeeFile_ReadBytes(self, max_bytes, false);
		}
		buf_begin = 0;
		buf_end   = (size_t)-1;
	}
	if (DeeObject_GetBuf(data, &buf, Dee_BUFFER_FWRITABLE))
		goto err;
	if (buf_end > buf.bb_size)
		buf_end = buf.bb_size;
	if (buf_begin >= buf_end) {
		DeeObject_PutBuf(data, &buf, Dee_BUFFER_FWRITABLE);
		return_reference_(DeeInt_Zero);
	}
	result = DeeFile_Read(self, (uint8_t *)buf.bb_base + buf_begin, buf_end - buf_begin);
	DeeObject_PutBuf(data, &buf, Dee_BUFFER_FWRITABLE);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/* >> operator write(buf: <Buffer>): int;
 * >> operator write(buf: <Buffer>, max_size: int): int;
 * >> operator write(buf: <Buffer>, start: int, end: int): int; */
DEFINE_OPERATOR_INVOKE(operator_write, NULL) { /* TODO: Allow user-code to override this operator */
	DeeObject *data  = NULL;
	DeeObject *begin = NULL;
	DeeObject *end   = NULL;
	DeeBuffer buf;
	size_t buf_begin, buf_end;
	size_t result;
	(void)tp_self; /* TODO: Must look at this! */
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, "o|oo:" OPNAME("write"), &data, &begin, &end))
		goto err;
	if (end) {
		if (DeeObject_AsSSize(begin, (dssize_t *)&buf_begin))
			goto err;
		if (DeeObject_AsSSize(end, (dssize_t *)&buf_end))
			goto err;
	} else if (begin) {
		if (DeeObject_AsSSize(begin, (dssize_t *)&buf_end))
			goto err;
		buf_begin = 0;
	} else {
		buf_begin = 0;
		buf_end   = (size_t)-1;
	}
	if (DeeObject_GetBuf(data, &buf, Dee_BUFFER_FREADONLY))
		goto err;
	if (buf_end > buf.bb_size)
		buf_end = buf.bb_size;
	if (buf_begin >= buf_end) {
		DeeObject_PutBuf(data, &buf, Dee_BUFFER_FREADONLY);
		return_reference_(DeeInt_Zero);
	}
	result = DeeFile_Write(self, (uint8_t *)buf.bb_base + buf_begin, buf_end - buf_begin);
	DeeObject_PutBuf(data, &buf, Dee_BUFFER_FREADONLY);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/* >> operator seek(off: int, whence: int): int; */
DEFINE_OPERATOR_INVOKE(operator_seek, NULL) { /* TODO: Allow user-code to override this operator */
	doff_t off;
	dpos_t result;
	int whence = SEEK_SET;
	(void)tp_self; /* TODO: Must look at this! */
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, UNPdN(DEE_SIZEOF_DEE_POS_T) "|d:" OPNAME("seek"), &off, &whence))
		goto err;
	result = DeeFile_Seek(self, off, whence);
	if unlikely(result == (dpos_t)-1)
		goto err;
	return DeeInt_NewUInt64(result);
err:
	return NULL;
}

/* >> operator sync(); */
DEFINE_OPERATOR_INVOKE(operator_sync, NULL) { /* TODO: Allow user-code to override this operator */
	(void)tp_self; /* TODO: Must look at this! */
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("sync")))
		goto err;
	if (DeeFile_Sync(self))
		goto err;
	return_none;
err:
	return NULL;
}

/* >> operator trunc(length: int);
 * >> operator trunc(): int; */
DEFINE_OPERATOR_INVOKE(operator_trunc, NULL) { /* TODO: Allow user-code to override this operator */
	dpos_t length;
	(void)tp_self; /* TODO: Must look at this! */
	(void)p_self;
	if (argc) {
		if unlikely(argc != 1) {
			err_invalid_argc(OPNAME("turnc"), argc, 0, 1);
			goto err;
		}
		if (DeeObject_AsUInt64(argv[0], &length))
			goto err;
		if (DeeFile_Trunc(self, length))
			goto err;
	} else {
		if (DeeFile_TruncHere(self, &length))
			goto err;
	}
	return DeeInt_NewUInt64(length);
err:
	return NULL;
}

/* >> operator close() */
DEFINE_OPERATOR_INVOKE(operator_close, NULL) { /* TODO: Allow user-code to override this operator */
	(void)tp_self; /* TODO: Must look at this! */
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("close")))
		goto err;
	if (DeeFile_Close(self))
		goto err;
	return_none;
err:
	return NULL;
}

/* >> operator pread(buf: <Buffer>, pos: int): int;
 * >> operator pread(buf: <Buffer>, max_size: int, pos: int): int;
 * >> operator pread(buf: <Buffer>, start: int, end: int, pos: int): int;
 * >> operator pread(max_bytes: int, pos: int): Bytes; 
 * >> operator pread(pos: int): Bytes; */
DEFINE_OPERATOR_INVOKE(operator_pread, NULL) { /* TODO: Allow user-code to override this operator */
	DeeObject *a;
	DeeObject *b = NULL;
	DeeObject *c = NULL;
	DeeObject *d = NULL;
	dpos_t pos;
	size_t start, end;
	size_t result;
	DeeBuffer buf;
	(void)tp_self; /* TODO: Must look at this! */
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, "o|ooo:" OPNAME("pread"), &a, &b, &c, &d))
		goto err;
	if (d) {
		if (DeeObject_AsUInt64(d, &pos))
			goto err;
		if (DeeObject_AsSSize(c, (dssize_t *)&end))
			goto err;
		if (DeeObject_AsSSize(b, (dssize_t *)&start))
			goto err;
	} else if (c) {
		if (DeeObject_AsUInt64(c, &pos))
			goto err;
		if (DeeObject_AsSSize(b, (dssize_t *)&end))
			goto err;
		start = 0;
	} else if (b) {
		if (DeeObject_AsUInt64(b, &pos))
			goto err;
		if (DeeInt_Check(a)) {
			if (DeeObject_AsSSize(a, (dssize_t *)&end))
				goto err;
			return DeeFile_PReadBytes(self, end, pos, false);
		}
		start = 0;
		end   = (size_t)-1;
	} else {
		if (DeeObject_AsUInt64(a, &pos))
			goto err;
		return DeeFile_PReadBytes(self, (size_t)-1, pos, false);
	}
	if (DeeObject_GetBuf(a, &buf, Dee_BUFFER_FWRITABLE))
		goto err;
	if (end > buf.bb_size)
		end = buf.bb_size;
	if (start >= end) {
		DeeObject_PutBuf(a, &buf, Dee_BUFFER_FWRITABLE);
		return_reference_(DeeInt_Zero);
	}
	result = DeeFile_PRead(self, (uint8_t *)buf.bb_base + start, end - start, pos);
	DeeObject_PutBuf(a, &buf, Dee_BUFFER_FWRITABLE);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/* >> operator pwrite(data: <Buffer>, pos: int): int;
 * >> operator pwrite(data: <Buffer>, max_size: int, pos: int): int;
 * >> operator pwrite(data: <Buffer>, start: int, end: int, pos: int): int; */
DEFINE_OPERATOR_INVOKE(operator_pwrite, NULL) { /* TODO: Allow user-code to override this operator */
	DeeObject *a;
	DeeObject *b;
	DeeObject *c = NULL;
	DeeObject *d = NULL;
	dpos_t pos;
	size_t start, end;
	size_t result;
	DeeBuffer buf;
	(void)tp_self; /* TODO: Must look at this! */
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, "oo|oo:" OPNAME("pwrite"), &a, &b, &c, &d))
		goto err;
	if (d) {
		if (DeeObject_AsUInt64(d, &pos))
			goto err;
		if (DeeObject_AsSSize(c, (dssize_t *)&end))
			goto err;
		if (DeeObject_AsSSize(b, (dssize_t *)&start))
			goto err;
	} else if (c) {
		if (DeeObject_AsUInt64(c, &pos))
			goto err;
		if (DeeObject_AsSSize(b, (dssize_t *)&end))
			goto err;
		start = 0;
	} else {
		if (DeeObject_AsUInt64(b, &pos))
			goto err;
		start = 0;
		end   = (size_t)-1;
	}
	if (DeeObject_GetBuf(a, &buf, Dee_BUFFER_FREADONLY))
		goto err;
	if (end > buf.bb_size)
		end = buf.bb_size;
	if (start >= end) {
		DeeObject_PutBuf(a, &buf, Dee_BUFFER_FREADONLY);
		return_reference_(DeeInt_Zero);
	}
	result = DeeFile_PWrite(self, (uint8_t *)buf.bb_base + start, end - start, pos);
	DeeObject_PutBuf(a, &buf, Dee_BUFFER_FREADONLY);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/* >> operator getc(): int; */
DEFINE_OPERATOR_INVOKE(operator_getc, NULL) { /* TODO: Allow user-code to override this operator */
	int result;
	(void)tp_self; /* TODO: Must look at this! */
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("getc")))
		goto err;
	result = DeeFile_Getc(self);
	if unlikely(result == GETC_ERR)
		goto err;
	return DeeInt_NewInt(result);
err:
	return NULL;
}

/* >> operator ungetc(ch: int): bool; */
DEFINE_OPERATOR_INVOKE(operator_ungetc, NULL) { /* TODO: Allow user-code to override this operator */
	int ch;
	(void)tp_self; /* TODO: Must look at this! */
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, "d:" OPNAME("ungetc"), &ch))
		goto err;
	ch = DeeFile_Ungetc(self, ch);
	if unlikely(ch == GETC_ERR)
		goto err;
	return_bool_(ch != GETC_EOF);
err:
	return NULL;
}

/* >> operator putc(ch: int): bool; */
DEFINE_OPERATOR_INVOKE(operator_putc, NULL) { /* TODO: Allow user-code to override this operator */
	int ch;
	(void)tp_self; /* TODO: Must look at this! */
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, "d:" OPNAME("putc"), &ch))
		goto err;
	ch = DeeFile_Putc(self, ch);
	if unlikely(ch == GETC_ERR)
		goto err;
	return_bool_(ch != GETC_EOF);
err:
	return NULL;
}


#undef DEFINE_OPERATOR_INVOKE

INTERN_CONST struct type_operator tpconst file_type_operators[LENGTHOF_file_type_operators] = {
	/* IMPORTANT: This stuff needs to be kept sorted by operator ID! */
	TYPE_OPERATOR_DECL(FILE_OPERATOR_0000_READ, /*  */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_read), /*  */ OPCC_SPECIAL, "read", /*  */ "read", /*  */ "ft_read", &operator_read),
	TYPE_OPERATOR_DECL(FILE_OPERATOR_0001_WRITE, /* */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_write), /* */ OPCC_SPECIAL, "write", /* */ "write", /* */ "ft_write", &operator_write),
	TYPE_OPERATOR_DECL(FILE_OPERATOR_0002_SEEK, /*  */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_seek), /*  */ OPCC_SPECIAL, "seek", /*  */ "seek", /*  */ "ft_seek", &operator_seek),
	TYPE_OPERATOR_DECL(FILE_OPERATOR_0003_SYNC, /*  */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_sync), /*  */ OPCC_SPECIAL, "sync", /*  */ "sync", /*  */ "ft_sync", &operator_sync),
	TYPE_OPERATOR_DECL(FILE_OPERATOR_0004_TRUNC, /* */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_trunc), /* */ OPCC_SPECIAL, "trunc", /* */ "trunc", /* */ "ft_trunc", &operator_trunc),
	TYPE_OPERATOR_DECL(FILE_OPERATOR_0005_CLOSE, /* */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_close), /* */ OPCC_SPECIAL, "close", /* */ "close", /* */ "ft_close", &operator_close),
	TYPE_OPERATOR_DECL(FILE_OPERATOR_0006_PREAD, /* */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_pread), /* */ OPCC_SPECIAL, "pread", /* */ "pread", /* */ "ft_pread", &operator_pread),
	TYPE_OPERATOR_DECL(FILE_OPERATOR_0007_PWRITE, /**/ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_pwrite), /**/ OPCC_SPECIAL, "pwrite", /**/ "pwrite", /**/ "ft_pwrite", &operator_pwrite),
	TYPE_OPERATOR_DECL(FILE_OPERATOR_0008_GETC, /*  */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_getc), /*  */ OPCC_SPECIAL, "getc", /*  */ "getc", /*  */ "ft_getc", &operator_getc),
	TYPE_OPERATOR_DECL(FILE_OPERATOR_0009_UNGETC, /**/ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_ungetc), /**/ OPCC_SPECIAL, "ungetc", /**/ "ungetc", /**/ "ft_ungetc", &operator_ungetc),
	TYPE_OPERATOR_DECL(FILE_OPERATOR_000A_PUTC, /*  */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_putc), /*  */ OPCC_SPECIAL, "putc", /*  */ "putc", /*  */ "ft_putc", &operator_putc)
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_FILE_TYPE_OPERATORS_C */
