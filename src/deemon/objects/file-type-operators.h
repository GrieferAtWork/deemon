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
#ifndef GUARD_DEEMON_OBJECTS_FILE_TYPE_OPERATORS_H
#define GUARD_DEEMON_OBJECTS_FILE_TYPE_OPERATORS_H 1

#include <deemon/api.h>

#include <deemon/file.h>   /* DeeFileObject, DeeFileTypeObject, Dee_ioflag_t, FILE_OPERATOR_COUNT */
#include <deemon/object.h> /* DeeTypeObject, Dee_off_t, Dee_pos_t */
#include <deemon/type.h>   /* Dee_opinfo, type_operator */

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */

DECL_BEGIN

#define LENGTHOF_file_type_operators FILE_OPERATOR_COUNT
INTDEF struct type_operator tpconst file_type_operators[LENGTHOF_file_type_operators];

/* Inherit file operators from bases, and auto-complete missing operators. */
INTDEF NONNULL((1)) bool DCALL DeeFileType_InheritSync(DeeFileTypeObject *__restrict self);
INTDEF NONNULL((1)) bool DCALL DeeFileType_InheritTrunc(DeeFileTypeObject *__restrict self);
INTDEF NONNULL((1)) bool DCALL DeeFileType_InheritClose(DeeFileTypeObject *__restrict self);
INTDEF NONNULL((1)) bool DCALL DeeFileType_InheritSeek(DeeFileTypeObject *__restrict self);
INTDEF NONNULL((1)) bool DCALL DeeFileType_InheritRead(DeeFileTypeObject *__restrict self);  /* ft_read, ft_getc, ft_ungetc, ft_pread [ft_seek] */
INTDEF NONNULL((1)) bool DCALL DeeFileType_InheritWrite(DeeFileTypeObject *__restrict self); /* ft_write, ft_putc, ft_trunc, ft_pwrite [ft_seek] */
INTDEF NONNULL((1)) bool DCALL DeeFileType_InheritPRead(DeeFileTypeObject *__restrict self); /* TODO: Remove this */
INTDEF NONNULL((1)) bool DCALL DeeFileType_InheritPWrite(DeeFileTypeObject *__restrict self); /* TODO: Remove this */
INTDEF NONNULL((1)) bool DCALL DeeFileType_InheritUngetc(DeeFileTypeObject *__restrict self); /* TODO: Remove this */
INTDEF NONNULL((1)) bool DCALL DeeFileType_InheritGetc(DeeFileTypeObject *__restrict self); /* TODO: Remove this */
INTDEF NONNULL((1)) bool DCALL DeeFileType_InheritPutc(DeeFileTypeObject *__restrict self); /* TODO: Remove this */


/* Default substitution of file operators. */
INTDEF WUNUSED NONNULL((1)) ATTR_OUTS(2, 3) size_t DCALL DeeFile_DefaultReadWithGetc(DeeFileObject *self, void *buffer, size_t bufsize, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL DeeFile_DefaultWriteWithPutc(DeeFileObject *self, void const *buffer, size_t bufsize, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeFile_DefaultGetcWithRead(DeeFileObject *__restrict self, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeFile_DefaultPutcWithWrite(DeeFileObject *__restrict self, int ch, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1)) ATTR_OUTS(2, 3) size_t DCALL DeeFile_DefaultPreadWithSeekAndRead(DeeFileObject *self, void *buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL DeeFile_DefaultPwriteWithSeekAndWrite(DeeFileObject *self, void const *buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeFile_DefaultUngetcWithSeek(DeeFileObject *__restrict self, int ch);


/* File operator callbacks for user-defined class types. */
INTDEF WUNUSED NONNULL((1)) ATTR_OUTS(2, 3) size_t DCALL instance_read(DeeFileObject *self, void *buffer, size_t bufsize, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL instance_write(DeeFileObject *self, void const *buffer, size_t bufsize, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1)) Dee_pos_t DCALL instance_seek(DeeFileObject *__restrict self, Dee_off_t off, int whence);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_sync(DeeFileObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_trunc(DeeFileObject *__restrict self, Dee_pos_t size);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_close(DeeFileObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) ATTR_OUTS(2, 3) size_t DCALL instance_pread(DeeFileObject *self, void *buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL instance_pwrite(DeeFileObject *self, void const *buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_getc(DeeFileObject *__restrict self, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_ungetc(DeeFileObject *__restrict self, int ch);
INTDEF WUNUSED NONNULL((1)) int DCALL instance_putc(DeeFileObject *__restrict self, int ch, Dee_ioflag_t flags);

INTDEF WUNUSED NONNULL((1, 2)) ATTR_OUTS(3, 4) size_t DCALL instance_tread(DeeFileTypeObject *tp_self, DeeFileObject *self, void *buffer, size_t bufsize, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1, 2)) ATTR_INS(3, 4) size_t DCALL instance_twrite(DeeFileTypeObject *tp_self, DeeFileObject *self, void const *buffer, size_t bufsize, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1, 2)) Dee_pos_t DCALL instance_tseek(DeeFileTypeObject *tp_self, DeeFileObject *self, Dee_off_t off, int whence);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_tsync(DeeFileTypeObject *tp_self, DeeFileObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_ttrunc(DeeFileTypeObject *tp_self, DeeFileObject *self, Dee_pos_t size);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_tclose(DeeFileTypeObject *tp_self, DeeFileObject *self);
INTDEF WUNUSED NONNULL((1, 2)) ATTR_OUTS(3, 4) size_t DCALL instance_tpread(DeeFileTypeObject *tp_self, DeeFileObject *self, void *buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1, 2)) ATTR_INS(3, 4) size_t DCALL instance_tpwrite(DeeFileTypeObject *tp_self, DeeFileObject *self, void const *buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_tgetc(DeeFileTypeObject *tp_self, DeeFileObject *self, Dee_ioflag_t flags);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_tungetc(DeeFileTypeObject *tp_self, DeeFileObject *self, int ch);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL instance_tputc(DeeFileTypeObject *tp_self, DeeFileObject *self, int ch, Dee_ioflag_t flags);

/* Callbacks for "struct Dee_operator_invoke::opi_inherit" in file operators. */
INTDEF NONNULL((1, 2, 3)) void DCALL filetype_inherit_read(DeeFileTypeObject *self, DeeTypeObject *type_type, struct Dee_opinfo const *info);
INTDEF NONNULL((1, 2, 3)) void DCALL filetype_inherit_write(DeeFileTypeObject *self, DeeTypeObject *type_type, struct Dee_opinfo const *info);
//INTDEF NONNULL((1, 2, 3)) void DCALL filetype_inherit_seek(DeeFileTypeObject *self, DeeTypeObject *type_type, struct Dee_opinfo const *info);  /* Not needed; standard inheritance already does the job. */
//INTDEF NONNULL((1, 2, 3)) void DCALL filetype_inherit_sync(DeeFileTypeObject *self, DeeTypeObject *type_type, struct Dee_opinfo const *info);  /* Not needed; standard inheritance already does the job. */
//INTDEF NONNULL((1, 2, 3)) void DCALL filetype_inherit_trunc(DeeFileTypeObject *self, DeeTypeObject *type_type, struct Dee_opinfo const *info); /* Not needed; standard inheritance already does the job. */
//INTDEF NONNULL((1, 2, 3)) void DCALL filetype_inherit_close(DeeFileTypeObject *self, DeeTypeObject *type_type, struct Dee_opinfo const *info); /* Not needed; standard inheritance already does the job. */
INTDEF NONNULL((1, 2, 3)) void DCALL filetype_inherit_pread(DeeFileTypeObject *self, DeeTypeObject *type_type, struct Dee_opinfo const *info);
INTDEF NONNULL((1, 2, 3)) void DCALL filetype_inherit_pwrite(DeeFileTypeObject *self, DeeTypeObject *type_type, struct Dee_opinfo const *info);
INTDEF NONNULL((1, 2, 3)) void DCALL filetype_inherit_getc(DeeFileTypeObject *self, DeeTypeObject *type_type, struct Dee_opinfo const *info);
INTDEF NONNULL((1, 2, 3)) void DCALL filetype_inherit_ungetc(DeeFileTypeObject *self, DeeTypeObject *type_type, struct Dee_opinfo const *info);
INTDEF NONNULL((1, 2, 3)) void DCALL filetype_inherit_putc(DeeFileTypeObject *self, DeeTypeObject *type_type, struct Dee_opinfo const *info);



/* Generic file operator invocation wrappers. */
#define DeeFileType_invoke_ft_read(tp_self, ft_read, self, buffer, bufsize, flags)                     \
	((ft_read) == &instance_read                                                                       \
	 ? instance_tread((DeeFileTypeObject *)(tp_self), (DeeFileObject *)(self), buffer, bufsize, flags) \
	 : (*(ft_read))((DeeFileObject *)(self), buffer, bufsize, flags))
#define DeeFileType_invoke_ft_write(tp_self, ft_write, self, buffer, bufsize, flags)                    \
	((ft_write) == &instance_write                                                                      \
	 ? instance_twrite((DeeFileTypeObject *)(tp_self), (DeeFileObject *)(self), buffer, bufsize, flags) \
	 : (*(ft_write))((DeeFileObject *)(self), buffer, bufsize, flags))
#define DeeFileType_invoke_ft_seek(tp_self, ft_seek, self, off, whence)                     \
	((ft_seek) == &instance_seek                                                            \
	 ? instance_tseek((DeeFileTypeObject *)(tp_self), (DeeFileObject *)(self), off, whence) \
	 : (*(ft_seek))((DeeFileObject *)(self), off, whence))
#define DeeFileType_invoke_ft_sync(tp_self, ft_sync, self)                     \
	((ft_sync) == &instance_sync                                               \
	 ? instance_tsync((DeeFileTypeObject *)(tp_self), (DeeFileObject *)(self)) \
	 : (*(ft_sync))((DeeFileObject *)(self)))
#define DeeFileType_invoke_ft_trunc(tp_self, ft_trunc, self, size)                    \
	((ft_trunc) == &instance_trunc                                                    \
	 ? instance_ttrunc((DeeFileTypeObject *)(tp_self), (DeeFileObject *)(self), size) \
	 : (*(ft_trunc))((DeeFileObject *)(self), size))
#define DeeFileType_invoke_ft_close(tp_self, ft_close, self)                    \
	((ft_close) == &instance_close                                              \
	 ? instance_tclose((DeeFileTypeObject *)(tp_self), (DeeFileObject *)(self)) \
	 : (*(ft_close))((DeeFileObject *)(self)))
#define DeeFileType_invoke_ft_pread(tp_self, ft_pread, self, buffer, bufsize, pos, flags)                    \
	((ft_pread) == &instance_pread                                                                           \
	 ? instance_tpread((DeeFileTypeObject *)(tp_self), (DeeFileObject *)(self), buffer, bufsize, pos, flags) \
	 : (*(ft_pread))((DeeFileObject *)(self), buffer, bufsize, pos, flags))
#define DeeFileType_invoke_ft_pwrite(tp_self, ft_pwrite, self, buffer, bufsize, pos, flags)                   \
	((ft_pwrite) == &instance_pwrite                                                                          \
	 ? instance_tpwrite((DeeFileTypeObject *)(tp_self), (DeeFileObject *)(self), buffer, bufsize, pos, flags) \
	 : (*(ft_pwrite))((DeeFileObject *)(self), buffer, bufsize, pos, flags))
#define DeeFileType_invoke_ft_getc(tp_self, ft_getc, self, flags)                     \
	((ft_getc) == &instance_getc                                                      \
	 ? instance_tgetc((DeeFileTypeObject *)(tp_self), (DeeFileObject *)(self), flags) \
	 : (*(ft_getc))((DeeFileObject *)(self), flags))
#define DeeFileType_invoke_ft_ungetc(tp_self, ft_ungetc, self, ch)                   \
	((ft_ungetc) == &instance_ungetc                                                 \
	 ? instance_tungetc((DeeFileTypeObject *)(tp_self), (DeeFileObject *)(self), ch) \
	 : (*(ft_ungetc))((DeeFileObject *)(self), ch))
#define DeeFileType_invoke_ft_putc(tp_self, ft_putc, self, ch, flags)                     \
	((ft_putc) == &instance_putc                                                          \
	 ? instance_tputc((DeeFileTypeObject *)(tp_self), (DeeFileObject *)(self), ch, flags) \
	 : (*(ft_putc))((DeeFileObject *)(self), ch, flags))

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_FILE_TYPE_OPERATORS_H */
