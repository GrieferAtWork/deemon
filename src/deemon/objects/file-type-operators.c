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
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/super.h>

#include <hybrid/typecore.h>
/**/

#include "../runtime/runtime_error.h"
#include "file-type-operators.h"

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#define OPNAME(opname) "operator " opname


INTERN WUNUSED ATTR_INOUT((1)) ATTR_OUTS(2, 3) size_t DCALL
instance_read(DeeFileObject *self, void *buffer,
              size_t bufsize, Dee_ioflag_t flags) {
	return instance_tread(Dee_TYPE(self), self, buffer, bufsize, flags);
}

INTERN WUNUSED ATTR_INOUT((1)) ATTR_INS(2, 3) size_t DCALL
instance_write(DeeFileObject *self, void const *buffer,
               size_t bufsize, Dee_ioflag_t flags) {
	return instance_twrite(Dee_TYPE(self), self, buffer, bufsize, flags);
}

INTERN WUNUSED ATTR_INOUT((1)) Dee_pos_t DCALL
instance_seek(DeeFileObject *__restrict self, Dee_off_t off, int whence) {
	return instance_tseek(Dee_TYPE(self), self, off, whence);
}

INTERN WUNUSED ATTR_INOUT((1)) int DCALL
instance_sync(DeeFileObject *__restrict self) {
	return instance_tsync(Dee_TYPE(self), self);
}

INTERN WUNUSED ATTR_INOUT((1)) int DCALL
instance_trunc(DeeFileObject *__restrict self, Dee_pos_t size) {
	return instance_ttrunc(Dee_TYPE(self), self, size);
}

INTERN WUNUSED ATTR_INOUT((1)) int DCALL
instance_close(DeeFileObject *__restrict self) {
	return instance_tclose(Dee_TYPE(self), self);
}

INTERN WUNUSED ATTR_INOUT((1)) ATTR_OUTS(2, 3) size_t DCALL
instance_pread(DeeFileObject *self, void *buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags) {
	return instance_tpread(Dee_TYPE(self), self, buffer, bufsize, pos, flags);
}

INTERN WUNUSED ATTR_INOUT((1)) ATTR_INS(2, 3) size_t DCALL
instance_pwrite(DeeFileObject *self, void const *buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags) {
	return instance_tpwrite(Dee_TYPE(self), self, buffer, bufsize, pos, flags);
}

INTERN WUNUSED ATTR_INOUT((1)) int DCALL
instance_getc(DeeFileObject *__restrict self, Dee_ioflag_t flags) {
	return instance_tgetc(Dee_TYPE(self), self, flags);
}

INTERN WUNUSED ATTR_INOUT((1)) int DCALL
instance_ungetc(DeeFileObject *__restrict self, int ch) {
	return instance_tungetc(Dee_TYPE(self), self, ch);
}

INTERN WUNUSED ATTR_INOUT((1)) int DCALL
instance_putc(DeeFileObject *__restrict self, int ch, Dee_ioflag_t flags) {
	return instance_tputc(Dee_TYPE(self), self, ch, flags);
}


INTERN WUNUSED ATTR_INOUT((1, 2)) ATTR_OUTS(3, 4) size_t DCALL
instance_tread(DeeFileTypeObject *tp_self, DeeFileObject *self,
               void *buffer, size_t bufsize, Dee_ioflag_t flags) {
	/* TODO: Need a way to construct temporary Bytes objects: `DeeBytes_NewTempView() + DeeBytes_ReleaseTempView()',
	 *       where `DeeBytes_ReleaseTempView()' makes it so the bytes object can't be accessed anymore.
	 *       (and yes: I realize that means adding a lock to `DeeBytesObject') */
	(void)tp_self;
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)flags;
	DeeError_NOTIMPLEMENTED();
	return (size_t)-1;
}

INTERN WUNUSED ATTR_INOUT((1, 2)) ATTR_INS(3, 4) size_t DCALL
instance_twrite(DeeFileTypeObject *tp_self, DeeFileObject *self,
                void const *buffer, size_t bufsize, Dee_ioflag_t flags) {
	/* TODO: Need a way to construct temporary Bytes objects: `DeeBytes_NewTempView() + DeeBytes_ReleaseTempView()',
	 *       where `DeeBytes_ReleaseTempView()' makes it so the bytes object can't be accessed anymore.
	 *       (and yes: I realize that means adding a lock to `DeeBytesObject') */
	(void)tp_self;
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)flags;
	DeeError_NOTIMPLEMENTED();
	return (size_t)-1;
}

INTERN WUNUSED ATTR_INOUT((1, 2)) Dee_pos_t DCALL
instance_tseek(DeeFileTypeObject *tp_self, DeeFileObject *self,
               Dee_off_t off, int whence) {
	Dee_pos_t result;
	DREF DeeObject *result_ob;
	result_ob = DeeClass_CallOperatorf((DeeTypeObject *)tp_self, (DeeObject *)self,
	                                   FILE_OPERATOR_SEEK,
	                                   PCKd64 "d", (int64_t)off, whence);
	if unlikely(!result_ob)
		goto err;
	if unlikely(DeeObject_AsUIntX(result_ob, &result))
		goto err_result_ob;
	if unlikely(result == (Dee_pos_t)-1)
		goto err_result_ob_overflow;
	Dee_Decref(result_ob);
	return result;
err_result_ob_overflow:
	err_integer_overflow(result_ob, sizeof(Dee_pos_t) * 8, true);
err_result_ob:
	Dee_Decref(result_ob);
err:
	return (Dee_pos_t)-1;
}

INTERN WUNUSED ATTR_INOUT((1, 2)) int DCALL
instance_tsync(DeeFileTypeObject *tp_self, DeeFileObject *self) {
	DREF DeeObject *result_ob;
	result_ob = DeeClass_CallOperator((DeeTypeObject *)tp_self, (DeeObject *)self,
	                                  FILE_OPERATOR_SYNC, 0, NULL);
	if unlikely(!result_ob)
		goto err;
	Dee_Decref(result_ob);
	return 0;
err:
	return -1;
}

INTERN WUNUSED ATTR_INOUT((1, 2)) int DCALL
instance_ttrunc(DeeFileTypeObject *tp_self, DeeFileObject *self, Dee_pos_t size) {
	DREF DeeObject *result_ob;
	result_ob = DeeClass_CallOperatorf((DeeTypeObject *)tp_self, (DeeObject *)self,
	                                   FILE_OPERATOR_TRUNC, PCKu64, (uint64_t)size);
	if unlikely(!result_ob)
		goto err;
	Dee_Decref(result_ob);
	return 0;
err:
	return -1;
}

INTERN WUNUSED ATTR_INOUT((1, 2)) int DCALL
instance_tclose(DeeFileTypeObject *tp_self, DeeFileObject *self) {
	DREF DeeObject *result_ob;
	result_ob = DeeClass_CallOperator((DeeTypeObject *)tp_self, (DeeObject *)self,
	                                  FILE_OPERATOR_CLOSE, 0, NULL);
	if unlikely(!result_ob)
		goto err;
	Dee_Decref(result_ob);
	return 0;
err:
	return -1;
}

INTERN WUNUSED ATTR_INOUT((1, 2)) ATTR_OUTS(3, 4) size_t DCALL
instance_tpread(DeeFileTypeObject *tp_self, DeeFileObject *self,
                void *buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags) {
	/* TODO: Need a way to construct temporary Bytes objects: `DeeBytes_NewTempView() + DeeBytes_ReleaseTempView()',
	 *       where `DeeBytes_ReleaseTempView()' makes it so the bytes object can't be accessed anymore.
	 *       (and yes: I realize that means adding a lock to `DeeBytesObject') */
	(void)tp_self;
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)pos;
	(void)flags;
	DeeError_NOTIMPLEMENTED();
	return (size_t)-1;
}

INTERN WUNUSED ATTR_INOUT((1, 2)) ATTR_INS(3, 4) size_t DCALL
instance_tpwrite(DeeFileTypeObject *tp_self, DeeFileObject *self,
                 void const *buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags) {
	/* TODO: Need a way to construct temporary Bytes objects: `DeeBytes_NewTempView() + DeeBytes_ReleaseTempView()',
	 *       where `DeeBytes_ReleaseTempView()' makes it so the bytes object can't be accessed anymore.
	 *       (and yes: I realize that means adding a lock to `DeeBytesObject') */
	(void)tp_self;
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)pos;
	(void)flags;
	DeeError_NOTIMPLEMENTED();
	return (size_t)-1;
}

INTERN WUNUSED ATTR_INOUT((1, 2)) int DCALL
instance_tgetc(DeeFileTypeObject *tp_self, DeeFileObject *self, Dee_ioflag_t flags) {
	int temp;
	DREF DeeObject *result_ob;
	result_ob = DeeClass_CallOperatorf((DeeTypeObject *)tp_self, (DeeObject *)self,
	                                   FILE_OPERATOR_GETC, "u", flags);
	if unlikely(!result_ob)
		goto err;
	if (DeeNone_Check(result_ob)) {
		Dee_DecrefNokill(result_ob);
		return GETC_EOF;
	}
	if unlikely(DeeObject_AsInt(result_ob, &temp))
		goto err_result_ob;
	if likely(temp == GETC_EOF || (temp >= 0 && temp <= 0xff)) {
		Dee_Decref(result_ob);
		return temp;
	}
	err_integer_overflow(result_ob, 8, temp >= 0);
err_result_ob:
	Dee_Decref(result_ob);
err:
	return GETC_ERR;
}

INTERN WUNUSED ATTR_INOUT((1, 2)) int DCALL
instance_tungetc(DeeFileTypeObject *tp_self, DeeFileObject *self, int ch) {
	int temp;
	DREF DeeObject *result_ob;
	result_ob = DeeClass_CallOperatorf((DeeTypeObject *)tp_self, (DeeObject *)self,
	                                   FILE_OPERATOR_UNGETC,
	                                   PCKuN(1), (byte_t)ch);
	if unlikely(!result_ob)
		goto err;
	if (DeeNone_Check(result_ob)) {
		Dee_DecrefNokill(result_ob);
		return ch;
	}
	temp = DeeObject_Bool(result_ob);
	Dee_Decref(result_ob);
	if unlikely(temp < 0)
		goto err;
	return temp ? ch : GETC_EOF;
err:
	return GETC_ERR;
}

INTERN WUNUSED ATTR_INOUT((1, 2)) int DCALL
instance_tputc(DeeFileTypeObject *tp_self, DeeFileObject *self,
               int ch, Dee_ioflag_t flags) {
	int temp;
	DREF DeeObject *result_ob;
	result_ob = DeeClass_CallOperatorf((DeeTypeObject *)tp_self, (DeeObject *)self,
	                                   FILE_OPERATOR_PUTC,
	                                   PCKuN(1) "u", (byte_t)ch, flags);
	if unlikely(!result_ob)
		goto err;
	if (DeeNone_Check(result_ob)) {
		Dee_DecrefNokill(result_ob);
		return ch;
	}
	temp = DeeObject_Bool(result_ob);
	Dee_Decref(result_ob);
	if unlikely(temp < 0)
		goto err;
	return temp ? ch : GETC_EOF;
err:
	return GETC_ERR;
}



#define DEFINE_OPERATOR_INVOKE(name, instance_name)                \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL          \
	invoke_##name(DeeFileTypeObject *tp_self, DeeFileObject *self, \
	              /*0..1*/ DREF DeeObject **p_self,                \
	              size_t argc, DeeObject *const *argv);            \
	PRIVATE struct Dee_operator_invoke tpconst name = {            \
		(Dee_operator_invoke_cb_t)&invoke_##name,                  \
		(dfunptr_t)(void const *)(instance_name)                   \
	};                                                             \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL          \
	invoke_##name(DeeFileTypeObject *tp_self, DeeFileObject *self, \
	              /*0..1*/ DREF DeeObject **p_self,                \
	              size_t argc, DeeObject *const *argv)

LOCAL ATTR_RETNONNULL WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL
make_super(DeeSuperObject *__restrict buf, DeeFileTypeObject *tp_self, DeeFileObject *self) {
	ASSERT_OBJECT_TYPE((DeeObject *)self, (DeeTypeObject *)tp_self);
	if (Dee_TYPE(self) == tp_self)
		return (DeeObject *)self;
	buf->ob_refcnt = 1;
	buf->ob_type   = &DeeSuper_Type;
	buf->s_type    = (DeeTypeObject *)tp_self;
	buf->s_self    = (DeeObject *)self;
	return (DeeObject *)buf;
}

/* >> operator read(buf: <Buffer>): int;
 * >> operator read(buf: <Buffer>, max_size: int): int;
 * >> operator read(buf: <Buffer>, start: int, end: int): int;
 * >> operator read(max_bytes: int): Bytes; 
 * >> operator read(): Bytes; */
DEFINE_OPERATOR_INVOKE(operator_read, &instance_read) {
	DeeObject *data  = NULL;
	DeeObject *begin = NULL;
	DeeObject *end   = NULL;
	DeeBuffer buf;
	size_t buf_begin, buf_end;
	size_t result;
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, "|ooo:" OPNAME("read"), &data, &begin, &end))
		goto err;
	if (!data) {
		DeeSuperObject super_buf;
		DeeObject *me = make_super(&super_buf, tp_self, self);
		return DeeFile_ReadBytes(me, (size_t)-1, false);
	}
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
			DeeSuperObject super_buf;
			DeeObject *me;
			size_t max_bytes;
			if (DeeInt_AsSize(data, &max_bytes))
				goto err;
			me = make_super(&super_buf, tp_self, self);
			return DeeFile_ReadBytes(me, max_bytes, false);
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
	result = DeeFile_TRead((DeeTypeObject *)tp_self, (DeeObject *)self,
	                       (byte_t *)buf.bb_base + buf_begin,
	                       buf_end - buf_begin);
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
DEFINE_OPERATOR_INVOKE(operator_write, &instance_write) {
	DeeObject *data  = NULL;
	DeeObject *begin = NULL;
	DeeObject *end   = NULL;
	DeeBuffer buf;
	size_t buf_begin, buf_end;
	size_t result;
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
	result = DeeFile_TWrite((DeeTypeObject *)tp_self, (DeeObject *)self,
	                        (byte_t const *)buf.bb_base + buf_begin,
	                        buf_end - buf_begin);
	DeeObject_PutBuf(data, &buf, Dee_BUFFER_FREADONLY);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/* >> operator seek(off: int, whence: int): int; */
DEFINE_OPERATOR_INVOKE(operator_seek, &instance_seek) {
	doff_t off;
	dpos_t result;
	int whence = SEEK_SET;
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, UNPdN(DEE_SIZEOF_DEE_POS_T) "|d:" OPNAME("seek"), &off, &whence))
		goto err;
	result = DeeFile_TSeek((DeeTypeObject *)tp_self, (DeeObject *)self, off, whence);
	if unlikely(result == (dpos_t)-1)
		goto err;
	return DeeInt_NewUInt64(result);
err:
	return NULL;
}

/* >> operator sync(); */
DEFINE_OPERATOR_INVOKE(operator_sync, &instance_sync) {
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("sync")))
		goto err;
	if (DeeFile_TSync((DeeTypeObject *)tp_self, (DeeObject *)self))
		goto err;
	return_none;
err:
	return NULL;
}

/* >> operator trunc(length: int);
 * >> operator trunc(): int; */
DEFINE_OPERATOR_INVOKE(operator_trunc, &instance_trunc) {
	dpos_t length;
	(void)p_self;
	if (argc) {
		if unlikely(argc != 1) {
			err_invalid_argc(OPNAME("turnc"), argc, 0, 1);
			goto err;
		}
		if (DeeObject_AsUInt64(argv[0], &length))
			goto err;
		if (DeeFile_TTrunc((DeeTypeObject *)tp_self, (DeeObject *)self, length))
			goto err;
	} else {
		if (DeeFile_TTruncHere((DeeTypeObject *)tp_self, (DeeObject *)self, &length))
			goto err;
	}
	return DeeInt_NewUInt64(length);
err:
	return NULL;
}

/* >> operator close() */
DEFINE_OPERATOR_INVOKE(operator_close, &instance_close) {
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("close")))
		goto err;
	if (DeeFile_TClose((DeeTypeObject *)tp_self, (DeeObject *)self))
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
DEFINE_OPERATOR_INVOKE(operator_pread, &instance_pread) {
	DeeObject *a;
	DeeObject *b = NULL;
	DeeObject *c = NULL;
	DeeObject *d = NULL;
	dpos_t pos;
	size_t start, end;
	size_t result;
	DeeBuffer buf;
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
			DeeSuperObject super_buf;
			DeeObject *me;
			if (DeeObject_AsSSize(a, (dssize_t *)&end))
				goto err;
			me = make_super(&super_buf, tp_self, self);
			return DeeFile_PReadBytes(me, end, pos, false);
		}
		start = 0;
		end   = (size_t)-1;
	} else {
		DeeSuperObject super_buf;
		DeeObject *me;
		if (DeeObject_AsUInt64(a, &pos))
			goto err;
		me = make_super(&super_buf, tp_self, self);
		return DeeFile_PReadBytes(me, (size_t)-1, pos, false);
	}
	if (DeeObject_GetBuf(a, &buf, Dee_BUFFER_FWRITABLE))
		goto err;
	if (end > buf.bb_size)
		end = buf.bb_size;
	if (start >= end) {
		DeeObject_PutBuf(a, &buf, Dee_BUFFER_FWRITABLE);
		return_reference_(DeeInt_Zero);
	}
	result = DeeFile_TPRead((DeeTypeObject *)tp_self, (DeeObject *)self,
	                        (byte_t *)buf.bb_base + start, end - start, pos);
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
DEFINE_OPERATOR_INVOKE(operator_pwrite, &instance_pwrite) {
	DeeObject *a;
	DeeObject *b;
	DeeObject *c = NULL;
	DeeObject *d = NULL;
	dpos_t pos;
	size_t start, end;
	size_t result;
	DeeBuffer buf;
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
	result = DeeFile_TPWrite((DeeTypeObject *)tp_self, (DeeObject *)self,
	                         (byte_t *)buf.bb_base + start, end - start, pos);
	DeeObject_PutBuf(a, &buf, Dee_BUFFER_FREADONLY);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/* >> operator getc(): int; */
DEFINE_OPERATOR_INVOKE(operator_getc, &instance_getc) {
	int result;
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, ":" OPNAME("getc")))
		goto err;
	result = DeeFile_TGetc((DeeTypeObject *)tp_self, (DeeObject *)self);
	if unlikely(result == GETC_ERR)
		goto err;
	return DeeInt_NewInt(result);
err:
	return NULL;
}

/* >> operator ungetc(ch: int): bool; */
DEFINE_OPERATOR_INVOKE(operator_ungetc, &instance_ungetc) {
	int ch;
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, "d:" OPNAME("ungetc"), &ch))
		goto err;
	ch = DeeFile_TUngetc((DeeTypeObject *)tp_self, (DeeObject *)self, ch);
	if unlikely(ch == GETC_ERR)
		goto err;
	return_bool_(ch != GETC_EOF);
err:
	return NULL;
}

/* >> operator putc(ch: int): bool; */
DEFINE_OPERATOR_INVOKE(operator_putc, &instance_putc) {
	int ch;
	(void)p_self;
	if (DeeArg_Unpack(argc, argv, "d:" OPNAME("putc"), &ch))
		goto err;
	ch = DeeFile_TPutc((DeeTypeObject *)tp_self, (DeeObject *)self, ch);
	if unlikely(ch == GETC_ERR)
		goto err;
	return_bool_(ch != GETC_EOF);
err:
	return NULL;
}


#undef DEFINE_OPERATOR_INVOKE

INTERN_CONST struct type_operator tpconst file_type_operators[LENGTHOF_file_type_operators] = {
	/* IMPORTANT: This stuff needs to be kept sorted by operator ID! */
	TYPE_OPERATOR_DECL(OPERATOR_FILE_0000_READ, /*  */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_read), /*  */ OPCC_SPECIAL, "read", /*  */ "read", /*  */ "ft_read", &operator_read),
	TYPE_OPERATOR_DECL(OPERATOR_FILE_0001_WRITE, /* */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_write), /* */ OPCC_SPECIAL, "write", /* */ "write", /* */ "ft_write", &operator_write),
	TYPE_OPERATOR_DECL(OPERATOR_FILE_0002_SEEK, /*  */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_seek), /*  */ OPCC_SPECIAL, "seek", /*  */ "seek", /*  */ "ft_seek", &operator_seek),
	TYPE_OPERATOR_DECL(OPERATOR_FILE_0003_SYNC, /*  */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_sync), /*  */ OPCC_SPECIAL, "sync", /*  */ "sync", /*  */ "ft_sync", &operator_sync),
	TYPE_OPERATOR_DECL(OPERATOR_FILE_0004_TRUNC, /* */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_trunc), /* */ OPCC_SPECIAL, "trunc", /* */ "trunc", /* */ "ft_trunc", &operator_trunc),
	TYPE_OPERATOR_DECL(OPERATOR_FILE_0005_CLOSE, /* */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_close), /* */ OPCC_SPECIAL, "close", /* */ "close", /* */ "ft_close", &operator_close),
	TYPE_OPERATOR_DECL(OPERATOR_FILE_0006_PREAD, /* */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_pread), /* */ OPCC_SPECIAL, "pread", /* */ "pread", /* */ "ft_pread", &operator_pread),
	TYPE_OPERATOR_DECL(OPERATOR_FILE_0007_PWRITE, /**/ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_pwrite), /**/ OPCC_SPECIAL, "pwrite", /**/ "pwrite", /**/ "ft_pwrite", &operator_pwrite),
	TYPE_OPERATOR_DECL(OPERATOR_FILE_0008_GETC, /*  */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_getc), /*  */ OPCC_SPECIAL, "getc", /*  */ "getc", /*  */ "ft_getc", &operator_getc),
	TYPE_OPERATOR_DECL(OPERATOR_FILE_0009_UNGETC, /**/ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_ungetc), /**/ OPCC_SPECIAL, "ungetc", /**/ "ungetc", /**/ "ft_ungetc", &operator_ungetc),
	TYPE_OPERATOR_DECL(OPERATOR_FILE_000A_PUTC, /*  */ OPCLASS_TYPE, offsetof(DeeFileTypeObject, ft_putc), /*  */ OPCC_SPECIAL, "putc", /*  */ "putc", /*  */ "ft_putc", &operator_putc)
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_FILE_TYPE_OPERATORS_C */
