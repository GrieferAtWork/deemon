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
#ifndef GUARD_DEEMON_CXX_FILE_H
#define GUARD_DEEMON_CXX_FILE_H 1

#include "api.h"
/**/

#include "object.h"
#include "sequence.h"
/**/

#include "../format.h"
#include "../file.h"
#include "../system-features.h"
/**/

#include <stdarg.h>

DEE_CXX_BEGIN

#undef read
#undef write
#undef getc
#undef putc
#undef ungetc
#undef printf
#undef vprintf

class File
	: public Sequence<Bytes>
{
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeFile_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeFile_Check(ob);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeFile_CheckExact(ob);
	}
public:
	static Ref<File> open(/*String*/ DeeObject *__restrict filename, int oflags, int mode) {
		return inherit(DeeFile_Open(filename, oflags, mode));
	}
	static Ref<File> open(/*utf-8*/ char const *__restrict filename, int oflags, int mode) {
		return inherit(DeeFile_OpenString(filename, oflags, mode));
	}
	static Ref<File> openfd(Dee_fd_t fd, /*String*/ DeeObject *filename, int oflags, bool inherit_fd) {
		return inherit(DeeFile_OpenFd(fd, filename, oflags, inherit_fd));
	}
	static Ref<File> stdstream(unsigned int id) {
		return inherit(DeeFile_GetStd(id));
	}
	static Ref<File> stdin_() {
		return inherit(DeeFile_GetStd(DEE_STDIN));
	}
	static Ref<File> stdout_() {
		return inherit(DeeFile_GetStd(DEE_STDOUT));
	}
	static Ref<File> stderr_() {
		return inherit(DeeFile_GetStd(DEE_STDERR));
	}
	static Ref<File> stddbg_() {
		return inherit(DeeFile_GetStd(DEE_STDDBG));
	}

public:
	WUNUSED size_t read(void *__restrict buffer, size_t bufsize) {
		return throw_if_minusone(DeeFile_Read(this, buffer, bufsize));
	}
	WUNUSED size_t read(void *__restrict buffer, size_t bufsize, Dee_ioflag_t flags) {
		return throw_if_minusone(DeeFile_Readf(this, buffer, bufsize, flags));
	}
	size_t write(void const *__restrict buffer, size_t bufsize) {
		return throw_if_minusone(DeeFile_Write(this, buffer, bufsize));
	}
	size_t write(void const *__restrict buffer, size_t bufsize, Dee_ioflag_t flags) {
		return throw_if_minusone(DeeFile_Writef(this, buffer, bufsize, flags));
	}
	WUNUSED size_t pread(void *__restrict buffer, size_t bufsize, Dee_pos_t pos) {
		return throw_if_minusone(DeeFile_PRead(this, buffer, bufsize, pos));
	}
	WUNUSED size_t pread(void *__restrict buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags) {
		return throw_if_minusone(DeeFile_PReadf(this, buffer, bufsize, pos, flags));
	}
	size_t pwrite(void const *__restrict buffer, size_t bufsize, Dee_pos_t pos) {
		return throw_if_minusone(DeeFile_PWrite(this, buffer, bufsize, pos));
	}
	size_t pwrite(void const *__restrict buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags) {
		return throw_if_minusone(DeeFile_PWritef(this, buffer, bufsize, pos, flags));
	}

	size_t readall(void *__restrict buffer, size_t bufsize) {
		return throw_if_minusone(DeeFile_ReadAll(this, buffer, bufsize));
	}
	size_t writeall(void const *__restrict buffer, size_t bufsize) {
		return throw_if_minusone(DeeFile_WriteAll(this, buffer, bufsize));
	}
	size_t preadall(void *__restrict buffer, size_t bufsize, Dee_pos_t pos) {
		return throw_if_minusone(DeeFile_PReadAll(this, buffer, bufsize, pos));
	}
	size_t pwriteall(void const *__restrict buffer, size_t bufsize, Dee_pos_t pos) {
		return throw_if_minusone(DeeFile_PWriteAll(this, buffer, bufsize, pos));
	}

	Dee_pos_t seek(Dee_off_t off, int whence) {
		return throw_if_minusone(DeeFile_Seek(this, off, whence));
	}

	WUNUSED Dee_pos_t tell() {
		return throw_if_minusone(DeeFile_Tell(this));
	}
	void rewind() {
		throw_if_minusone(DeeFile_Rewind(this));
	}
	void setpos(Dee_pos_t pos) {
		throw_if_minusone(DeeFile_SetPos(this, pos));
	}

	void sync() {
		throw_if_nonzero(DeeFile_Sync(this));
	}
	void trunc(Dee_pos_t size) {
		throw_if_nonzero(DeeFile_Trunc(this, size));
	}
	void trunc(Dee_pos_t *p_size) {
		throw_if_nonzero(DeeFile_TruncHere(this, p_size));
	}
	void close() {
		throw_if_nonzero(DeeFile_Close(this));
	}
	WUNUSED int getc() {
		int result = DeeFile_Getc(this);
		if (result == GETC_ERR)
			throw_last_deemon_exception();
		return result;
	}
	WUNUSED int getc(Dee_ioflag_t flags) {
		int result = DeeFile_Getcf(this, flags);
		if (result == GETC_ERR)
			throw_last_deemon_exception();
		return result;
	}
	int ungetc(int ch) {
		int result = DeeFile_Ungetc(this, ch);
		if (result == GETC_ERR)
			throw_last_deemon_exception();
		return result;
	}
	int putc(int ch) {
		int result = DeeFile_Putc(this, ch);
		if (result == GETC_ERR)
			throw_last_deemon_exception();
		return result;
	}
	int putc(int ch, Dee_ioflag_t flags) {
		int result = DeeFile_Putcf(this, ch, flags);
		if (result == GETC_ERR)
			throw_last_deemon_exception();
		return result;
	}

	Dee_pos_t size() {
		return throw_if_minusone(DeeFile_GetSize(this));
	}

	bool isatty() {
		return throw_if_negative(DeeFile_IsAtty(this)) != 0;
	}

	Dee_fd_t fileno() {
		Dee_fd_t result = DeeFile_GetSysFD(this);
		if (result == Dee_fd_INVALID)
			throw_last_deemon_exception();
		return result;
	}

	Ref<string> filename() {
		return inherit(DeeFile_Filename(this));
	}

	Ref<Bytes> readline(bool keep_lf) {
		return inherit(DeeFile_ReadLine(this, (size_t)-1, keep_lf));
	}
	Ref<Bytes> readline(size_t max_length = (size_t)-1, bool keep_lf = true) {
		return inherit(DeeFile_ReadLine(this, max_length, keep_lf));
	}
	Ref<Bytes> readbytes(size_t max_length = (size_t)-1, bool readall = false) {
		return inherit(DeeFile_ReadBytes(this, max_length, readall));
	}
	Ref<Bytes> preadbytes(Dee_pos_t pos) {
		return inherit(DeeFile_PReadBytes(this, (size_t)-1, pos, false));
	}
	Ref<Bytes> preadbytes(size_t max_length, Dee_pos_t pos, bool readall = false) {
		return inherit(DeeFile_PReadBytes(this, max_length, pos, readall));
	}

	size_t printf(char const *format, ...) {
		size_t result;
		va_list args;
		va_start(args, format);
		result = DeeFile_VPrintf(this, format, args);
		va_end(args);
		return throw_if_minusone(result);
	}
	size_t vprintf(char const *format, va_list args) {
		return throw_if_minusone(DeeFile_VPrintf(this, format, args));
	}

	void print_nl() {
		throw_if_nonzero(DeeFile_PrintNl(this));
	}
	void print_object(DeeObject *ob) {
		throw_if_nonzero(DeeFile_PrintObject(this, ob));
	}
	void print_object_sp(DeeObject *ob) {
		throw_if_nonzero(DeeFile_PrintObjectSp(this, ob));
	}
	void print_object_nl(DeeObject *ob) {
		throw_if_nonzero(DeeFile_PrintObjectNl(this, ob));
	}
	void print_all(DeeObject *ob) {
		throw_if_nonzero(DeeFile_PrintAll(this, ob));
	}
	void print_all_sp(DeeObject *ob) {
		throw_if_nonzero(DeeFile_PrintAllSp(this, ob));
	}
	void print_all_nl(DeeObject *ob) {
		throw_if_nonzero(DeeFile_PrintAllNl(this, ob));
	}

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(File from deemon).printCxxApi(exclude: {
	"getc", "read", "pread", "readall", "preadall",
	"putc", "write", "pwrite", "writeall", "pwriteall",
	"ungetc", "seek", "trunc", "readline", "tell", "size",
	"close", "sync", "rewind", "setpos",
	"mmap", // Way too many overloads (over 1000)
});]]]*/
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (readinto)(DeeObject *dst) {
		DeeObject *args[1];
		args[0] = dst;
		return inherit(DeeObject_CallAttrStringHash(this, "readinto", _Dee_HashSelectC(0x3beface0, 0x14b1a7a62217aa57), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (readinto)(DeeObject *dst, DeeObject *readall) {
		DeeObject *args[2];
		args[0] = dst;
		args[1] = readall;
		return inherit(DeeObject_CallAttrStringHash(this, "readinto", _Dee_HashSelectC(0x3beface0, 0x14b1a7a62217aa57), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (readinto)(DeeObject *dst, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readinto", _Dee_HashSelectC(0x3beface0, 0x14b1a7a62217aa57), "ob", dst, readall));
	}
	WUNUSED Ref<deemon::int_> (readinto)(char const *dst) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readinto", _Dee_HashSelectC(0x3beface0, 0x14b1a7a62217aa57), "s", dst));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::int_> (readinto)(char const *dst, DeeObject *readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readinto", _Dee_HashSelectC(0x3beface0, 0x14b1a7a62217aa57), "so", dst, readall));
	}
	WUNUSED Ref<deemon::int_> (readinto)(char const *dst, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readinto", _Dee_HashSelectC(0x3beface0, 0x14b1a7a62217aa57), "sb", dst, readall));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Bytes> (preadinto)(DeeObject *dst, DeeObject *pos) {
		DeeObject *args[2];
		args[0] = dst;
		args[1] = pos;
		return inherit(DeeObject_CallAttrStringHash(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<Bytes> (preadinto)(DeeObject *dst, DeeObject *pos, DeeObject *readall) {
		DeeObject *args[3];
		args[0] = dst;
		args[1] = pos;
		args[2] = readall;
		return inherit(DeeObject_CallAttrStringHash(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Bytes> (preadinto)(DeeObject *dst, DeeObject *pos, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "oob", dst, pos, readall));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (preadinto)(DeeObject *dst, Dee_ssize_t pos) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "o" DEE_PCKdSIZ, dst, pos));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Bytes> (preadinto)(DeeObject *dst, Dee_ssize_t pos, DeeObject *readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "o" DEE_PCKdSIZ "o", dst, pos, readall));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (preadinto)(DeeObject *dst, Dee_ssize_t pos, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "o" DEE_PCKdSIZ "b", dst, pos, readall));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (preadinto)(DeeObject *dst, size_t pos) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "o" DEE_PCKuSIZ, dst, pos));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Bytes> (preadinto)(DeeObject *dst, size_t pos, DeeObject *readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "o" DEE_PCKuSIZ "o", dst, pos, readall));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (preadinto)(DeeObject *dst, size_t pos, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "o" DEE_PCKuSIZ "b", dst, pos, readall));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (preadinto)(char const *dst, DeeObject *pos) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "so", dst, pos));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Bytes> (preadinto)(char const *dst, DeeObject *pos, DeeObject *readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "soo", dst, pos, readall));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (preadinto)(char const *dst, DeeObject *pos, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "sob", dst, pos, readall));
	}
	WUNUSED Ref<Bytes> (preadinto)(char const *dst, Dee_ssize_t pos) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "s" DEE_PCKdSIZ, dst, pos));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Bytes> (preadinto)(char const *dst, Dee_ssize_t pos, DeeObject *readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "s" DEE_PCKdSIZ "o", dst, pos, readall));
	}
	WUNUSED Ref<Bytes> (preadinto)(char const *dst, Dee_ssize_t pos, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "s" DEE_PCKdSIZ "b", dst, pos, readall));
	}
	WUNUSED Ref<Bytes> (preadinto)(char const *dst, size_t pos) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "s" DEE_PCKuSIZ, dst, pos));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Bytes> (preadinto)(char const *dst, size_t pos, DeeObject *readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "s" DEE_PCKuSIZ "o", dst, pos, readall));
	}
	WUNUSED Ref<Bytes> (preadinto)(char const *dst, size_t pos, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "preadinto", _Dee_HashSelectC(0x739ea18e, 0x69dbe6959249e490), "s" DEE_PCKuSIZ "b", dst, pos, readall));
	}
	WUNUSED Ref<string> (getutf8)() {
		return inherit(DeeObject_CallAttrStringHash(this, "getutf8", _Dee_HashSelectC(0x793968d0, 0x250a8130a90938ba), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (ungetutf8)(DeeObject *ch) {
		DeeObject *args[1];
		args[0] = ch;
		return inherit(DeeObject_CallAttrStringHash(this, "ungetutf8", _Dee_HashSelectC(0xa3df015d, 0x478925f9320a9ed8), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (ungetutf8)(Dee_ssize_t ch) {
		return inherit(DeeObject_CallAttrStringHashf(this, "ungetutf8", _Dee_HashSelectC(0xa3df015d, 0x478925f9320a9ed8),  DEE_PCKdSIZ, ch));
	}
	WUNUSED Ref<deemon::bool_> (ungetutf8)(size_t ch) {
		return inherit(DeeObject_CallAttrStringHashf(this, "ungetutf8", _Dee_HashSelectC(0xa3df015d, 0x478925f9320a9ed8),  DEE_PCKuSIZ, ch));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (pututf8)(DeeObject *data) {
		DeeObject *args[1];
		args[0] = data;
		return inherit(DeeObject_CallAttrStringHash(this, "pututf8", _Dee_HashSelectC(0xb8bfcee8, 0xbad5f3b7beb8cb05), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (pututf8)(char const *data) {
		return inherit(DeeObject_CallAttrStringHashf(this, "pututf8", _Dee_HashSelectC(0xb8bfcee8, 0xbad5f3b7beb8cb05), "s", data));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (readat)(DeeObject *pos) {
		DeeObject *args[1];
		args[0] = pos;
		return inherit(DeeObject_CallAttrStringHash(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Bytes> (readat)(DeeObject *pos, DeeObject *maxbytes) {
		DeeObject *args[2];
		args[0] = pos;
		args[1] = maxbytes;
		return inherit(DeeObject_CallAttrStringHash(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<Bytes> (readat)(DeeObject *pos, DeeObject *maxbytes, DeeObject *readall) {
		DeeObject *args[3];
		args[0] = pos;
		args[1] = maxbytes;
		args[2] = readall;
		return inherit(DeeObject_CallAttrStringHash(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Bytes> (readat)(DeeObject *pos, DeeObject *maxbytes, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e), "oob", pos, maxbytes, readall));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (readat)(DeeObject *pos, Dee_ssize_t maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e), "o" DEE_PCKdSIZ, pos, maxbytes));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Bytes> (readat)(DeeObject *pos, Dee_ssize_t maxbytes, DeeObject *readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e), "o" DEE_PCKdSIZ "o", pos, maxbytes, readall));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (readat)(DeeObject *pos, Dee_ssize_t maxbytes, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e), "o" DEE_PCKdSIZ "b", pos, maxbytes, readall));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (readat)(DeeObject *pos, size_t maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e), "o" DEE_PCKuSIZ, pos, maxbytes));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Bytes> (readat)(DeeObject *pos, size_t maxbytes, DeeObject *readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e), "o" DEE_PCKuSIZ "o", pos, maxbytes, readall));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (readat)(DeeObject *pos, size_t maxbytes, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e), "o" DEE_PCKuSIZ "b", pos, maxbytes, readall));
	}
	WUNUSED Ref<Bytes> (readat)(Dee_ssize_t pos) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKdSIZ, pos));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (readat)(Dee_ssize_t pos, DeeObject *maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKdSIZ "o", pos, maxbytes));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Bytes> (readat)(Dee_ssize_t pos, DeeObject *maxbytes, DeeObject *readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKdSIZ "oo", pos, maxbytes, readall));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (readat)(Dee_ssize_t pos, DeeObject *maxbytes, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKdSIZ "ob", pos, maxbytes, readall));
	}
	WUNUSED Ref<Bytes> (readat)(Dee_ssize_t pos, Dee_ssize_t maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKdSIZ DEE_PCKdSIZ, pos, maxbytes));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Bytes> (readat)(Dee_ssize_t pos, Dee_ssize_t maxbytes, DeeObject *readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKdSIZ DEE_PCKdSIZ "o", pos, maxbytes, readall));
	}
	WUNUSED Ref<Bytes> (readat)(Dee_ssize_t pos, Dee_ssize_t maxbytes, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKdSIZ DEE_PCKdSIZ "b", pos, maxbytes, readall));
	}
	WUNUSED Ref<Bytes> (readat)(Dee_ssize_t pos, size_t maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKdSIZ DEE_PCKuSIZ, pos, maxbytes));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Bytes> (readat)(Dee_ssize_t pos, size_t maxbytes, DeeObject *readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKdSIZ DEE_PCKuSIZ "o", pos, maxbytes, readall));
	}
	WUNUSED Ref<Bytes> (readat)(Dee_ssize_t pos, size_t maxbytes, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKdSIZ DEE_PCKuSIZ "b", pos, maxbytes, readall));
	}
	WUNUSED Ref<Bytes> (readat)(size_t pos) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKuSIZ, pos));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (readat)(size_t pos, DeeObject *maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKuSIZ "o", pos, maxbytes));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Bytes> (readat)(size_t pos, DeeObject *maxbytes, DeeObject *readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKuSIZ "oo", pos, maxbytes, readall));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (readat)(size_t pos, DeeObject *maxbytes, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKuSIZ "ob", pos, maxbytes, readall));
	}
	WUNUSED Ref<Bytes> (readat)(size_t pos, Dee_ssize_t maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKuSIZ DEE_PCKdSIZ, pos, maxbytes));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Bytes> (readat)(size_t pos, Dee_ssize_t maxbytes, DeeObject *readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKuSIZ DEE_PCKdSIZ "o", pos, maxbytes, readall));
	}
	WUNUSED Ref<Bytes> (readat)(size_t pos, Dee_ssize_t maxbytes, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKuSIZ DEE_PCKdSIZ "b", pos, maxbytes, readall));
	}
	WUNUSED Ref<Bytes> (readat)(size_t pos, size_t maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKuSIZ DEE_PCKuSIZ, pos, maxbytes));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Bytes> (readat)(size_t pos, size_t maxbytes, DeeObject *readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKuSIZ DEE_PCKuSIZ "o", pos, maxbytes, readall));
	}
	WUNUSED Ref<Bytes> (readat)(size_t pos, size_t maxbytes, bool readall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readat", _Dee_HashSelectC(0xba87cc58, 0xb190e1a01928006e),  DEE_PCKuSIZ DEE_PCKuSIZ "b", pos, maxbytes, readall));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (writeat)(DeeObject *data, DeeObject *pos) {
		DeeObject *args[2];
		args[0] = data;
		args[1] = pos;
		return inherit(DeeObject_CallAttrStringHash(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (writeat)(DeeObject *data, DeeObject *pos, DeeObject *writeall) {
		DeeObject *args[3];
		args[0] = data;
		args[1] = pos;
		args[2] = writeall;
		return inherit(DeeObject_CallAttrStringHash(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (writeat)(DeeObject *data, DeeObject *pos, bool writeall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "oob", data, pos, writeall));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (writeat)(DeeObject *data, Dee_ssize_t pos) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "o" DEE_PCKdSIZ, data, pos));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (writeat)(DeeObject *data, Dee_ssize_t pos, DeeObject *writeall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "o" DEE_PCKdSIZ "o", data, pos, writeall));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (writeat)(DeeObject *data, Dee_ssize_t pos, bool writeall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "o" DEE_PCKdSIZ "b", data, pos, writeall));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (writeat)(DeeObject *data, size_t pos) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "o" DEE_PCKuSIZ, data, pos));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (writeat)(DeeObject *data, size_t pos, DeeObject *writeall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "o" DEE_PCKuSIZ "o", data, pos, writeall));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (writeat)(DeeObject *data, size_t pos, bool writeall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "o" DEE_PCKuSIZ "b", data, pos, writeall));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::int_> (writeat)(char const *data, DeeObject *pos) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "so", data, pos));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<deemon::int_> (writeat)(char const *data, DeeObject *pos, DeeObject *writeall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "soo", data, pos, writeall));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::int_> (writeat)(char const *data, DeeObject *pos, bool writeall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "sob", data, pos, writeall));
	}
	WUNUSED Ref<deemon::int_> (writeat)(char const *data, Dee_ssize_t pos) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "s" DEE_PCKdSIZ, data, pos));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::int_> (writeat)(char const *data, Dee_ssize_t pos, DeeObject *writeall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "s" DEE_PCKdSIZ "o", data, pos, writeall));
	}
	WUNUSED Ref<deemon::int_> (writeat)(char const *data, Dee_ssize_t pos, bool writeall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "s" DEE_PCKdSIZ "b", data, pos, writeall));
	}
	WUNUSED Ref<deemon::int_> (writeat)(char const *data, size_t pos) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "s" DEE_PCKuSIZ, data, pos));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::int_> (writeat)(char const *data, size_t pos, DeeObject *writeall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "s" DEE_PCKuSIZ "o", data, pos, writeall));
	}
	WUNUSED Ref<deemon::int_> (writeat)(char const *data, size_t pos, bool writeall) {
		return inherit(DeeObject_CallAttrStringHashf(this, "writeat", _Dee_HashSelectC(0x5735b114, 0x2d7a153f48778210), "s" DEE_PCKuSIZ "b", data, pos, writeall));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (readallat)(DeeObject *pos) {
		DeeObject *args[1];
		args[0] = pos;
		return inherit(DeeObject_CallAttrStringHash(this, "readallat", _Dee_HashSelectC(0x4a3b37ea, 0x92eb943eeeb4889d), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Bytes> (readallat)(DeeObject *pos, DeeObject *maxbytes) {
		DeeObject *args[2];
		args[0] = pos;
		args[1] = maxbytes;
		return inherit(DeeObject_CallAttrStringHash(this, "readallat", _Dee_HashSelectC(0x4a3b37ea, 0x92eb943eeeb4889d), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (readallat)(DeeObject *pos, Dee_ssize_t maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readallat", _Dee_HashSelectC(0x4a3b37ea, 0x92eb943eeeb4889d), "o" DEE_PCKdSIZ, pos, maxbytes));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Bytes> (readallat)(DeeObject *pos, size_t maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readallat", _Dee_HashSelectC(0x4a3b37ea, 0x92eb943eeeb4889d), "o" DEE_PCKuSIZ, pos, maxbytes));
	}
	WUNUSED Ref<Bytes> (readallat)(Dee_ssize_t pos) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readallat", _Dee_HashSelectC(0x4a3b37ea, 0x92eb943eeeb4889d),  DEE_PCKdSIZ, pos));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (readallat)(Dee_ssize_t pos, DeeObject *maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readallat", _Dee_HashSelectC(0x4a3b37ea, 0x92eb943eeeb4889d),  DEE_PCKdSIZ "o", pos, maxbytes));
	}
	WUNUSED Ref<Bytes> (readallat)(Dee_ssize_t pos, Dee_ssize_t maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readallat", _Dee_HashSelectC(0x4a3b37ea, 0x92eb943eeeb4889d),  DEE_PCKdSIZ DEE_PCKdSIZ, pos, maxbytes));
	}
	WUNUSED Ref<Bytes> (readallat)(Dee_ssize_t pos, size_t maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readallat", _Dee_HashSelectC(0x4a3b37ea, 0x92eb943eeeb4889d),  DEE_PCKdSIZ DEE_PCKuSIZ, pos, maxbytes));
	}
	WUNUSED Ref<Bytes> (readallat)(size_t pos) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readallat", _Dee_HashSelectC(0x4a3b37ea, 0x92eb943eeeb4889d),  DEE_PCKuSIZ, pos));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Bytes> (readallat)(size_t pos, DeeObject *maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readallat", _Dee_HashSelectC(0x4a3b37ea, 0x92eb943eeeb4889d),  DEE_PCKuSIZ "o", pos, maxbytes));
	}
	WUNUSED Ref<Bytes> (readallat)(size_t pos, Dee_ssize_t maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readallat", _Dee_HashSelectC(0x4a3b37ea, 0x92eb943eeeb4889d),  DEE_PCKuSIZ DEE_PCKdSIZ, pos, maxbytes));
	}
	WUNUSED Ref<Bytes> (readallat)(size_t pos, size_t maxbytes) {
		return inherit(DeeObject_CallAttrStringHashf(this, "readallat", _Dee_HashSelectC(0x4a3b37ea, 0x92eb943eeeb4889d),  DEE_PCKuSIZ DEE_PCKuSIZ, pos, maxbytes));
	}
	void (flush)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "flush", _Dee_HashSelectC(0xc55ede0a, 0x8a3df9f6a93e2205), 0, NULL)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (puts)(DeeObject *data) {
		DeeObject *args[1];
		args[0] = data;
		return inherit(DeeObject_CallAttrStringHash(this, "puts", _Dee_HashSelectC(0x7cfecd38, 0x2f5a3dd681edead9), 1, args));
	}
	WUNUSED Ref<deemon::int_> (puts)(char const *data) {
		return inherit(DeeObject_CallAttrStringHashf(this, "puts", _Dee_HashSelectC(0x7cfecd38, 0x2f5a3dd681edead9), "s", data));
	}
	class _Wrap_pos
		: public deemon::detail::ConstGetRefProxy<_Wrap_pos, deemon::int_>
		, public deemon::detail::ConstSetRefProxy<_Wrap_pos, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_pos, deemon::int_>::operator =;
		_Wrap_pos(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "pos", _Dee_HashSelectC(0xb1aecbb4, 0x277b6d36f75741ae));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "pos", _Dee_HashSelectC(0xb1aecbb4, 0x277b6d36f75741ae)));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "pos", _Dee_HashSelectC(0xb1aecbb4, 0x277b6d36f75741ae)));
		}
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "pos", _Dee_HashSelectC(0xb1aecbb4, 0x277b6d36f75741ae), value);
		}
	};
	WUNUSED _Wrap_pos (pos)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_seq
		: public deemon::detail::ConstGetRefProxy<_Wrap_seq, File> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_seq(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "seq", _Dee_HashSelectC(0x232af2b7, 0x80a0b0950a5a5251));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "seq", _Dee_HashSelectC(0x232af2b7, 0x80a0b0950a5a5251)));
		}
	};
	WUNUSED _Wrap_seq (seq)() DEE_CXX_NOTHROW {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_FILE_H */
