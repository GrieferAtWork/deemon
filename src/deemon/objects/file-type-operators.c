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
#ifndef GUARD_DEEMON_OBJECTS_FILE_TYPE_OPERATORS_C
#define GUARD_DEEMON_OBJECTS_FILE_TYPE_OPERATORS_C 1

#include <deemon/api.h>

#include <deemon/arg.h>      /* DeeArg_Unpack*, UNPdN */
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/super.h>

#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "../runtime/runtime_error.h"
#include "file-type-operators.h"

#include <stdbool.h> /* false */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* int64_t, uint64_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#define OPNAME(opname) "operator " opname


/************************************************************************/
/* Instance operator hooks                                              */
/************************************************************************/
INTERN WUNUSED NONNULL((1)) ATTR_OUTS(2, 3) size_t DCALL
instance_read(DeeFileObject *self, void *buffer,
              size_t bufsize, Dee_ioflag_t flags) {
	return instance_tread(Dee_TYPE(self), self, buffer, bufsize, flags);
}

INTERN WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL
instance_write(DeeFileObject *self, void const *buffer,
               size_t bufsize, Dee_ioflag_t flags) {
	return instance_twrite(Dee_TYPE(self), self, buffer, bufsize, flags);
}

INTERN WUNUSED NONNULL((1)) Dee_pos_t DCALL
instance_seek(DeeFileObject *__restrict self, Dee_off_t off, int whence) {
	return instance_tseek(Dee_TYPE(self), self, off, whence);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_sync(DeeFileObject *__restrict self) {
	return instance_tsync(Dee_TYPE(self), self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_trunc(DeeFileObject *__restrict self, Dee_pos_t size) {
	return instance_ttrunc(Dee_TYPE(self), self, size);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_close(DeeFileObject *__restrict self) {
	return instance_tclose(Dee_TYPE(self), self);
}

INTERN WUNUSED NONNULL((1)) ATTR_OUTS(2, 3) size_t DCALL
instance_pread(DeeFileObject *self, void *buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags) {
	return instance_tpread(Dee_TYPE(self), self, buffer, bufsize, pos, flags);
}

INTERN WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL
instance_pwrite(DeeFileObject *self, void const *buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags) {
	return instance_tpwrite(Dee_TYPE(self), self, buffer, bufsize, pos, flags);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_getc(DeeFileObject *__restrict self, Dee_ioflag_t flags) {
	return instance_tgetc(Dee_TYPE(self), self, flags);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_ungetc(DeeFileObject *__restrict self, int ch) {
	return instance_tungetc(Dee_TYPE(self), self, ch);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_putc(DeeFileObject *__restrict self, int ch, Dee_ioflag_t flags) {
	return instance_tputc(Dee_TYPE(self), self, ch, flags);
}


INTERN WUNUSED NONNULL((1, 2)) ATTR_OUTS(3, 4) size_t DCALL
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

INTERN WUNUSED NONNULL((1, 2)) ATTR_INS(3, 4) size_t DCALL
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

INTERN WUNUSED NONNULL((1, 2)) Dee_pos_t DCALL
instance_tseek(DeeFileTypeObject *tp_self, DeeFileObject *self,
               Dee_off_t off, int whence) {
	DREF DeeObject *result_ob;
	result_ob = DeeClass_CallOperatorf((DeeTypeObject *)tp_self, Dee_AsObject(self),
	                                   FILE_OPERATOR_SEEK,
	                                   PCKd64 "d", (int64_t)off, whence);
	if unlikely(!result_ob)
		goto err;
	return DeeObject_AsXUIntDirectInherited(Dee_SIZEOF_POS_T, result_ob);
err:
	return (Dee_pos_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_tsync(DeeFileTypeObject *tp_self, DeeFileObject *self) {
	DREF DeeObject *result_ob;
	result_ob = DeeClass_CallOperator((DeeTypeObject *)tp_self, Dee_AsObject(self),
	                                  FILE_OPERATOR_SYNC, 0, NULL);
	if unlikely(!result_ob)
		goto err;
	Dee_Decref(result_ob);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_ttrunc(DeeFileTypeObject *tp_self, DeeFileObject *self, Dee_pos_t size) {
	DREF DeeObject *result_ob;
	result_ob = DeeClass_CallOperatorf((DeeTypeObject *)tp_self, Dee_AsObject(self),
	                                   FILE_OPERATOR_TRUNC, PCKu64, (uint64_t)size);
	if unlikely(!result_ob)
		goto err;
	Dee_Decref(result_ob);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_tclose(DeeFileTypeObject *tp_self, DeeFileObject *self) {
	DREF DeeObject *result_ob;
	result_ob = DeeClass_CallOperator((DeeTypeObject *)tp_self, Dee_AsObject(self),
	                                  FILE_OPERATOR_CLOSE, 0, NULL);
	if unlikely(!result_ob)
		goto err;
	Dee_Decref(result_ob);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) ATTR_OUTS(3, 4) size_t DCALL
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

INTERN WUNUSED NONNULL((1, 2)) ATTR_INS(3, 4) size_t DCALL
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

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_tgetc(DeeFileTypeObject *tp_self, DeeFileObject *self, Dee_ioflag_t flags) {
	int temp;
	DREF DeeObject *result_ob;
	result_ob = DeeClass_CallOperatorf((DeeTypeObject *)tp_self, Dee_AsObject(self),
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
	DeeRT_ErrIntegerOverflowS(temp, GETC_EOF, 0xff);
err_result_ob:
	Dee_Decref(result_ob);
err:
	return GETC_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_tungetc(DeeFileTypeObject *tp_self, DeeFileObject *self, int ch) {
	int temp;
	DREF DeeObject *result_ob;
	result_ob = DeeClass_CallOperatorf((DeeTypeObject *)tp_self, Dee_AsObject(self),
	                                   FILE_OPERATOR_UNGETC,
	                                   PCKuN(1), (byte_t)ch);
	if unlikely(!result_ob)
		goto err;
	if (DeeNone_Check(result_ob)) {
		Dee_DecrefNokill(result_ob);
		return ch;
	}
	temp = DeeObject_BoolInherited(result_ob);
	if unlikely(temp < 0)
		goto err;
	return temp ? ch : GETC_EOF;
err:
	return GETC_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_tputc(DeeFileTypeObject *tp_self, DeeFileObject *self,
               int ch, Dee_ioflag_t flags) {
	int temp;
	DREF DeeObject *result_ob;
	result_ob = DeeClass_CallOperatorf((DeeTypeObject *)tp_self, Dee_AsObject(self),
	                                   FILE_OPERATOR_PUTC,
	                                   PCKuN(1) "u", (byte_t)ch, flags);
	if unlikely(!result_ob)
		goto err;
	if (DeeNone_Check(result_ob)) {
		Dee_DecrefNokill(result_ob);
		return ch;
	}
	temp = DeeObject_BoolInherited(result_ob);
	if unlikely(temp < 0)
		goto err;
	return temp ? ch : GETC_EOF;
err:
	return GETC_ERR;
}





/************************************************************************/
/* Operator inheritance overrides.                                      */
/************************************************************************/
#define DEFINE_FILETYPE_INHERIT_HOOK(filetype_inherit_name, type_inherit_file_name) \
	INTERN NONNULL((1, 2, 3)) void DCALL                                            \
	filetype_inherit_name(DeeFileTypeObject *self, DeeTypeObject *type_type,        \
	                      struct Dee_opinfo const *info) {                          \
		ASSERT(type_type == &DeeFileType_Type);                                     \
		(void)type_type;                                                            \
		(void)info;                                                                 \
		type_inherit_file_name(self);                                               \
	}
DEFINE_FILETYPE_INHERIT_HOOK(filetype_inherit_read, DeeFileType_InheritRead)
DEFINE_FILETYPE_INHERIT_HOOK(filetype_inherit_write, DeeFileType_InheritWrite)
//DEFINE_FILETYPE_INHERIT_HOOK(filetype_inherit_seek, DeeFileType_InheritSeek)   /* Not needed; standard inheritance already does the job. */
//DEFINE_FILETYPE_INHERIT_HOOK(filetype_inherit_sync, DeeFileType_InheritSync)   /* Not needed; standard inheritance already does the job. */
//DEFINE_FILETYPE_INHERIT_HOOK(filetype_inherit_trunc, DeeFileType_InheritTrunc) /* Not needed; standard inheritance already does the job. */
//DEFINE_FILETYPE_INHERIT_HOOK(filetype_inherit_close, DeeFileType_InheritClose) /* Not needed; standard inheritance already does the job. */
DEFINE_FILETYPE_INHERIT_HOOK(filetype_inherit_pread, DeeFileType_InheritPRead)
DEFINE_FILETYPE_INHERIT_HOOK(filetype_inherit_pwrite, DeeFileType_InheritPWrite)
DEFINE_FILETYPE_INHERIT_HOOK(filetype_inherit_getc, DeeFileType_InheritGetc)
DEFINE_FILETYPE_INHERIT_HOOK(filetype_inherit_ungetc, DeeFileType_InheritUngetc)
DEFINE_FILETYPE_INHERIT_HOOK(filetype_inherit_putc, DeeFileType_InheritPutc)
#undef DEFINE_FILETYPE_INHERIT_HOOK





/************************************************************************/
/* Generic operator invocation                                          */
/************************************************************************/
#define DEFINE_OPERATOR_INVOKE(name, instance_name, inherit_name)              \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                      \
	invoke_##name(DeeFileTypeObject *tp_self, DeeFileObject *self,             \
	              /*0..1*/ DREF DeeFileObject **p_self,                        \
	              size_t argc, DeeObject *const *argv, Dee_operator_t opname); \
	PRIVATE struct Dee_operator_invoke tpconst name =                          \
	Dee_OPERATOR_INVOKE_INIT(&invoke_##name, instance_name, inherit_name);     \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                      \
	invoke_##name(DeeFileTypeObject *tp_self, DeeFileObject *self,             \
	              /*0..1*/ DREF DeeFileObject **p_self,                        \
	              size_t argc, DeeObject *const *argv, Dee_operator_t opname)

LOCAL ATTR_RETNONNULL WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL
make_super(DeeSuperObject *__restrict buf, DeeFileTypeObject *tp_self, DeeFileObject *self) {
	ASSERT_OBJECT_TYPE(Dee_AsObject(self), (DeeTypeObject *)tp_self);
	if (Dee_TYPE(self) == tp_self)
		return Dee_AsObject(self);
	buf->ob_refcnt = 1;
	buf->ob_type   = &DeeSuper_Type;
	buf->s_type    = (DeeTypeObject *)tp_self;
	buf->s_self    = Dee_AsObject(self);
	return (DeeObject *)buf;
}

/* >> operator read(buf: <Buffer>): int;
 * >> operator read(buf: <Buffer>, max_size: int): int;
 * >> operator read(buf: <Buffer>, start: int, end: int): int;
 * >> operator read(buf: <Buffer>, start: int, end: int, flags: int): int;
 * >> operator read(maxBytes: int): Bytes; 
 * >> operator read(): Bytes; */
DEFINE_OPERATOR_INVOKE(operator_read, &instance_read, &filetype_inherit_read) {
	DeeBuffer buf;
	size_t buf_start, buf_end;
	size_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)('" OPNAME("read") "', params: "
	DeeObject *data  = NULL;
	DeeObject *start = NULL;
	DeeObject *end   = NULL;
	Dee_ioflag_t flags = Dee_FILEIO_FNORMAL = !0;
");]]]*/
	struct {
		DeeObject *data;
		DeeObject *start;
		DeeObject *end;
		Dee_ioflag_t flags;
	} args;
	args.data = NULL;
	args.start = NULL;
	args.end = NULL;
	args.flags = Dee_FILEIO_FNORMAL;
	if (DeeArg_UnpackStruct(argc, argv, "|ooou:" OPNAME("read"), &args))
		goto err;
/*[[[end]]]*/
	(void)p_self;
	(void)opname;
	ASSERT(tp_self->ft_read);
	if (!args.data) {
		DeeSuperObject super_buf;
		DeeObject *me = make_super(&super_buf, tp_self, self);
		return DeeFile_ReadBytes(me, (size_t)-1, false);
	}
	if (args.end) {
		if (DeeObject_AsSize(args.start, &buf_start))
			goto err;
		if (DeeObject_AsSizeM1(args.end, &buf_end))
			goto err;
	} else if (args.start) {
		if (DeeObject_AsSizeM1(args.start, &buf_end))
			goto err;
		buf_start = 0;
	} else {
		if (DeeInt_Check(args.data)) {
			DeeSuperObject super_buf;
			DeeObject *me;
			size_t max_bytes;
			if (DeeInt_AsSizeM1(args.data, &max_bytes))
				goto err;
			me = make_super(&super_buf, tp_self, self);
			return DeeFile_ReadBytes(me, max_bytes, false);
		}
		buf_start = 0;
		buf_end   = (size_t)-1;
	}
	if (DeeObject_GetBuf(args.data, &buf, Dee_BUFFER_FWRITABLE))
		goto err;
	if (buf_end > buf.bb_size)
		buf_end = buf.bb_size;
	if (buf_start >= buf_end) {
		DeeBuffer_Fini(&buf);
		return DeeInt_NewZero();
	}
	result = DeeFileType_invoke_ft_read(tp_self, tp_self->ft_read, self,
	                                    (byte_t *)buf.bb_base + buf_start,
	                                    buf_end - buf_start, args.flags);
	DeeBuffer_Fini(&buf);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/* >> operator write(buf: <Buffer>): int;
 * >> operator write(buf: <Buffer>, max_size: int): int;
 * >> operator write(buf: <Buffer>, start: int, end: int): int; */
DEFINE_OPERATOR_INVOKE(operator_write, &instance_write, &filetype_inherit_write) {
	DeeBuffer buf;
	size_t buf_start, buf_end;
	size_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)('" OPNAME("write") "', params: "
	DeeObject *data;
	DeeObject *start = NULL;
	DeeObject *end   = NULL;
	Dee_ioflag_t flags = Dee_FILEIO_FNORMAL = !0;
");]]]*/
	struct {
		DeeObject *data;
		DeeObject *start;
		DeeObject *end;
		Dee_ioflag_t flags;
	} args;
	args.start = NULL;
	args.end = NULL;
	args.flags = Dee_FILEIO_FNORMAL;
	if (DeeArg_UnpackStruct(argc, argv, "o|oou:" OPNAME("write"), &args))
		goto err;
/*[[[end]]]*/
	(void)p_self;
	(void)opname;
	ASSERT(tp_self->ft_write);
	if (args.end) {
		if (DeeObject_AsSize(args.start, &buf_start))
			goto err;
		if (DeeObject_AsSizeM1(args.end, &buf_end))
			goto err;
	} else if (args.start) {
		if (DeeObject_AsSizeM1(args.start, &buf_end))
			goto err;
		buf_start = 0;
	} else {
		buf_start = 0;
		buf_end   = (size_t)-1;
	}
	if (DeeObject_GetBuf(args.data, &buf, Dee_BUFFER_FREADONLY))
		goto err;
	if (buf_end > buf.bb_size)
		buf_end = buf.bb_size;
	if (buf_start >= buf_end) {
		DeeBuffer_Fini(&buf);
		return DeeInt_NewZero();
	}
	result = DeeFileType_invoke_ft_write(tp_self, tp_self->ft_write, self,
	                                     (byte_t const *)buf.bb_base + buf_start,
	                                     buf_end - buf_start, args.flags);
	DeeBuffer_Fini(&buf);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/* >> operator seek(off: int): int; */
/* >> operator seek(off: int, whence: int): int; */
DEFINE_OPERATOR_INVOKE(operator_seek, &instance_seek, NULL /*&filetype_inherit_seek*/) {
	Dee_pos_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)('" OPNAME("seek") "', params: "
	Dee_off_t off;
	int whence = SEEK_SET = !A!DFile!PSEEK_SET;
");]]]*/
	struct {
		Dee_off_t off;
		int whence;
	} args;
	args.whence = SEEK_SET;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, OPNAME("seek"), &args, &args.off, UNPdN(Dee_SIZEOF_OFF_T), DeeObject_MapAsXInt(Dee_SIZEOF_OFF_T), &args.whence, "d", DeeObject_AsInt);
/*[[[end]]]*/
	(void)p_self;
	(void)opname;
	ASSERT(tp_self->ft_seek);
	result = DeeFileType_invoke_ft_seek(tp_self, tp_self->ft_seek,
	                                    self, args.off, args.whence);
	if unlikely(result == (Dee_pos_t)-1)
		goto err;
	return DeeInt_NewUInt64(result);
err:
	return NULL;
}

/* >> operator sync(); */
DEFINE_OPERATOR_INVOKE(operator_sync, &instance_sync, NULL /*&filetype_inherit_sync*/) {
	(void)p_self;
	(void)opname;
	ASSERT(tp_self->ft_sync);
	DeeArg_Unpack0(err, argc, argv, OPNAME("sync"));
	if (DeeFileType_invoke_ft_sync(tp_self, tp_self->ft_sync, self))
		goto err;
	return_none;
err:
	return NULL;
}

/* >> operator trunc(length: int);
 * >> operator trunc(): int; */
DEFINE_OPERATOR_INVOKE(operator_trunc, &instance_trunc, NULL /*&filetype_inherit_trunc*/) {
	Dee_pos_t length;
	(void)p_self;
	(void)opname;
	ASSERT(tp_self->ft_trunc);
	if (argc) {
		if unlikely(argc != 1) {
			err_invalid_argc(OPNAME("turnc"), argc, 0, 1);
			goto err;
		}
		if (DeeObject_AsUInt64(argv[0], &length))
			goto err;
		if (DeeFileType_invoke_ft_trunc(tp_self, tp_self->ft_trunc, self, length))
			goto err;
	} else {
		/* TODO: Directly invoke `ft_trunc' */
		if (DeeFile_TTruncHere((DeeTypeObject *)tp_self, Dee_AsObject(self), &length))
			goto err;
	}
	return DeeInt_NewUInt64(length);
err:
	return NULL;
}

/* >> operator close() */
DEFINE_OPERATOR_INVOKE(operator_close, &instance_close, NULL /*&filetype_inherit_close*/) {
	(void)p_self;
	(void)opname;
	ASSERT(tp_self->ft_close);
	DeeArg_Unpack0(err, argc, argv, OPNAME("close"));
	if (DeeFileType_invoke_ft_close(tp_self, tp_self->ft_close, self))
		goto err;
	return_none;
err:
	return NULL;
}

/* >> operator pread(buf: <Buffer>, pos: int): int;
 * >> operator pread(buf: <Buffer>, max_size: int, pos: int): int;
 * >> operator pread(buf: <Buffer>, start: int, end: int, pos: int): int;
 * >> operator pread(buf: <Buffer>, start: int, end: int, pos: int, flags: int): int;
 * >> operator pread(maxBytes: int, pos: int): Bytes; 
 * >> operator pread(pos: int): Bytes; */
DEFINE_OPERATOR_INVOKE(operator_pread, &instance_pread, &filetype_inherit_pread) {
	Dee_pos_t pos;
	size_t start, end;
	size_t result;
	DeeBuffer buf;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)('" OPNAME("pread") "', params: "
	DeeObject *a;
	DeeObject *b = NULL;
	DeeObject *c = NULL;
	DeeObject *d = NULL;
	Dee_ioflag_t flags = Dee_FILEIO_FNORMAL = !0;
");]]]*/
	struct {
		DeeObject *a;
		DeeObject *b;
		DeeObject *c;
		DeeObject *d;
		Dee_ioflag_t flags;
	} args;
	args.b = NULL;
	args.c = NULL;
	args.d = NULL;
	args.flags = Dee_FILEIO_FNORMAL;
	if (DeeArg_UnpackStruct(argc, argv, "o|ooou:" OPNAME("pread"), &args))
		goto err;
/*[[[end]]]*/
	ASSERT(tp_self->ft_pread);
	(void)p_self;
	(void)opname;
	if (args.d) {
		if (DeeObject_AsUInt64(args.d, &pos))
			goto err;
		if (DeeObject_AsSizeM1(args.c, &end))
			goto err;
		if (DeeObject_AsSize(args.b, &start))
			goto err;
	} else if (args.c) {
		if (DeeObject_AsUInt64(args.c, &pos))
			goto err;
		if (DeeObject_AsSizeM1(args.b, &end))
			goto err;
		start = 0;
	} else if (args.b) {
		if (DeeObject_AsUInt64(args.b, &pos))
			goto err;
		if (DeeInt_Check(args.a)) {
			DeeSuperObject super_buf;
			DeeObject *me;
			if (DeeInt_AsSizeM1(args.a, &end))
				goto err;
			me = make_super(&super_buf, tp_self, self);
			return DeeFile_PReadBytes(me, end, pos, false);
		}
		start = 0;
		end   = (size_t)-1;
	} else {
		DeeSuperObject super_buf;
		DeeObject *me;
		if (DeeObject_AsUInt64(args.a, &pos))
			goto err;
		me = make_super(&super_buf, tp_self, self);
		return DeeFile_PReadBytes(me, (size_t)-1, pos, false);
	}
	if (DeeObject_GetBuf(args.a, &buf, Dee_BUFFER_FWRITABLE))
		goto err;
	if (end > buf.bb_size)
		end = buf.bb_size;
	if (start >= end) {
		DeeBuffer_Fini(&buf);
		return DeeInt_NewZero();
	}
	result = DeeFileType_invoke_ft_pread(tp_self, tp_self->ft_pread, self,
	                                     (byte_t *)buf.bb_base + start,
	                                     end - start, pos, args.flags);
	DeeBuffer_Fini(&buf);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/* >> operator pwrite(data: <Buffer>, pos: int): int;
 * >> operator pwrite(data: <Buffer>, max_size: int, pos: int): int;
 * >> operator pwrite(data: <Buffer>, start: int, end: int, pos: int): int;
 * >> operator pwrite(data: <Buffer>, start: int, end: int, pos: int, flags: int): int; */
DEFINE_OPERATOR_INVOKE(operator_pwrite, &instance_pwrite, &filetype_inherit_pwrite) {
	Dee_pos_t pos;
	size_t start, end;
	size_t result;
	DeeBuffer buf;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)('" OPNAME("pwrite") "', params: "
	DeeObject *a;
	DeeObject *b;
	DeeObject *c = NULL;
	DeeObject *d = NULL;
	Dee_ioflag_t flags = Dee_FILEIO_FNORMAL = !0;
");]]]*/
	struct {
		DeeObject *a;
		DeeObject *b;
		DeeObject *c;
		DeeObject *d;
		Dee_ioflag_t flags;
	} args;
	args.c = NULL;
	args.d = NULL;
	args.flags = Dee_FILEIO_FNORMAL;
	if (DeeArg_UnpackStruct(argc, argv, "oo|oou:" OPNAME("pwrite"), &args))
		goto err;
/*[[[end]]]*/
	ASSERT(tp_self->ft_pwrite);
	(void)p_self;
	(void)opname;
	if (args.d) {
		if (DeeObject_AsUInt64(args.d, &pos))
			goto err;
		if (DeeObject_AsSizeM1(args.c, &end))
			goto err;
		if (DeeObject_AsSize(args.b, &start))
			goto err;
	} else if (args.c) {
		if (DeeObject_AsUInt64(args.c, &pos))
			goto err;
		if (DeeObject_AsSizeM1(args.b, &end))
			goto err;
		start = 0;
	} else {
		if (DeeObject_AsUInt64(args.b, &pos))
			goto err;
		start = 0;
		end   = (size_t)-1;
	}
	if (DeeObject_GetBuf(args.a, &buf, Dee_BUFFER_FREADONLY))
		goto err;
	if (end > buf.bb_size)
		end = buf.bb_size;
	if (start >= end) {
		DeeBuffer_Fini(&buf);
		return DeeInt_NewZero();
	}
	result = DeeFileType_invoke_ft_pwrite(tp_self, tp_self->ft_pwrite, self,
	                                      (byte_t *)buf.bb_base + start,
	                                      end - start, pos, args.flags);
	DeeBuffer_Fini(&buf);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

/* >> operator getc(): int;
 * >> operator getc(flags: int): int; */
DEFINE_OPERATOR_INVOKE(operator_getc, &instance_getc, &filetype_inherit_getc) {
	int result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)('" OPNAME("getc") "', params: "
	Dee_ioflag_t flags = Dee_FILEIO_FNORMAL = !0;
");]]]*/
	struct {
		Dee_ioflag_t flags;
	} args;
	args.flags = Dee_FILEIO_FNORMAL;
	DeeArg_Unpack0Or1X(err, argc, argv, OPNAME("getc"), &args.flags, "u", DeeObject_AsUInt);
/*[[[end]]]*/
	(void)p_self;
	(void)opname;
	ASSERT(tp_self->ft_getc);
	result = DeeFileType_invoke_ft_getc(tp_self, tp_self->ft_getc, self, args.flags);
	if unlikely(result == GETC_ERR)
		goto err;
	return DeeInt_NewInt(result);
err:
	return NULL;
}

/* >> operator ungetc(ch: int): bool; */
DEFINE_OPERATOR_INVOKE(operator_ungetc, &instance_ungetc, &filetype_inherit_ungetc) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)('" OPNAME("ungetc") "', params: "
	int ch
");]]]*/
	struct {
		int ch;
	} args;
	DeeArg_Unpack1X(err, argc, argv, OPNAME("ungetc"), &args.ch, "d", DeeObject_AsInt);
/*[[[end]]]*/
	(void)p_self;
	(void)opname;
	ASSERT(tp_self->ft_ungetc);
	args.ch = DeeFileType_invoke_ft_ungetc(tp_self, tp_self->ft_ungetc, self, args.ch);
	if unlikely(args.ch == GETC_ERR)
		goto err;
	return_bool(args.ch != GETC_EOF);
err:
	return NULL;
}

/* >> operator putc(ch: int): bool; */
DEFINE_OPERATOR_INVOKE(operator_putc, &instance_putc, &filetype_inherit_putc) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)('" OPNAME("putc") "', params: "
	int ch;
	Dee_ioflag_t flags = Dee_FILEIO_FNORMAL = !0;
");]]]*/
	struct {
		int ch;
		Dee_ioflag_t flags;
	} args;
	args.flags = Dee_FILEIO_FNORMAL;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, OPNAME("putc"), &args, &args.ch, "d", DeeObject_AsInt, &args.flags, "u", DeeObject_AsUInt);
/*[[[end]]]*/
	(void)p_self;
	(void)opname;
	ASSERT(tp_self->ft_putc);
	args.ch = DeeFileType_invoke_ft_putc(tp_self, tp_self->ft_putc, self, args.ch, args.flags);
	if unlikely(args.ch == GETC_ERR)
		goto err;
	return_bool(args.ch != GETC_EOF);
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
