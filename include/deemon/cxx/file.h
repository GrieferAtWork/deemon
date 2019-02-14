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
#ifndef GUARD_DEEMON_CXX_FILE_H
#define GUARD_DEEMON_CXX_FILE_H 1

#include "api.h"
#include "object.h"
#include "../file.h"
#include "../filetypes.h"

#include <stdarg.h>

DEE_CXX_BEGIN

class file: public object {
public:
    static DeeTypeObject *classtype() DEE_CXX_NOTHROW { return (DeeTypeObject *)&DeeFile_Type; }
    static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW { return DeeFile_Check(ob); }
    static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW { return DeeFile_CheckExact(ob); }
public:
    class buffer;
    class writer;
public:
    DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(file,object)
    size_t (read)(void *__restrict buffer, size_t bufsize) const { return throw_if_negative(DeeFile_Read(*this,buffer,bufsize)); }
    size_t (read)(void *__restrict buffer, size_t bufsize, Dee_ioflag_t flags) const { return throw_if_negative(DeeFile_Readf(*this,buffer,bufsize,flags)); }
    size_t (write)(void const *__restrict buffer, size_t bufsize) const { return throw_if_negative(DeeFile_Write(*this,buffer,bufsize)); }
    size_t (write)(void const *__restrict buffer, size_t bufsize, Dee_ioflag_t flags) const { return throw_if_negative(DeeFile_Writef(*this,buffer,bufsize,flags)); }
    size_t (pread)(void *__restrict buffer, size_t bufsize, Dee_pos_t pos) const { return throw_if_negative(DeeFile_PRead(*this,buffer,bufsize,pos)); }
    size_t (pread)(void *__restrict buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags) const { return throw_if_negative(DeeFile_PReadf(*this,buffer,bufsize,pos,flags)); }
    size_t (pwrite)(void const *__restrict buffer, size_t bufsize, Dee_pos_t pos) const { return throw_if_negative(DeeFile_PWrite(*this,buffer,bufsize,pos)); }
    size_t (pwrite)(void const *__restrict buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags) const { return throw_if_negative(DeeFile_PWritef(*this,buffer,bufsize,pos,flags)); }
    size_t (readall)(void *__restrict buffer, size_t bufsize) const { return throw_if_negative(DeeFile_ReadAll(*this,buffer,bufsize)); }
    size_t (writeall)(void const *__restrict buffer, size_t bufsize) const { return throw_if_negative(DeeFile_WriteAll(*this,buffer,bufsize)); }
    size_t (preadall)(void *__restrict buffer, size_t bufsize, Dee_pos_t pos) const { return throw_if_negative(DeeFile_PReadAll(*this,buffer,bufsize,pos)); }
    size_t (pwriteall)(void const *__restrict buffer, size_t bufsize, Dee_pos_t pos) const { return throw_if_negative(DeeFile_PWriteAll(*this,buffer,bufsize,pos)); }
    Dee_pos_t (seek)(Dee_off_t off, int whence = SEEK_SET) const { return throw_if_negative(DeeFile_Seek(*this,off,whence)); }
    Dee_pos_t (tell)() const { return throw_if_negative(DeeFile_Tell(*this)); }
    void (rewind)() const { throw_if_negative(DeeFile_Rewind(*this)); }
    void (sync)() const { throw_if_nonzero(DeeFile_Sync(*this)); }
    void (trunc)(Dee_pos_t size) const { throw_if_nonzero(DeeFile_Trunc(*this,size)); }
    void (trunc)(Dee_pos_t *psize = NULL) const { throw_if_nonzero(DeeFile_TruncHere(*this,psize)); }
    void (close)() const { throw_if_nonzero(DeeFile_Close(*this)); }
    int (getc)() const { int result = DeeFile_Getc(*this); if unlikely(result == GETC_ERR) throw_last_deemon_exception(); return result; }
    int (getc)(Dee_ioflag_t flags) const { int result = DeeFile_Getcf(*this,flags); if unlikely(result == GETC_ERR) throw_last_deemon_exception(); return result; }
    int (ungetc)(int ch) const { int result = DeeFile_Ungetc(*this,ch); if unlikely(result == GETC_ERR) throw_last_deemon_exception(); return result; }
    int (putc)(int ch) const { int result = DeeFile_Putc(*this,ch); if unlikely(result == GETC_ERR) throw_last_deemon_exception(); return result; }
    int (putc)(int ch, Dee_ioflag_t flags) const { int result = DeeFile_Putcf(*this,ch,flags); if unlikely(result == GETC_ERR) throw_last_deemon_exception(); return result; }
    Dee_pos_t (size)() const { return throw_if_minusone(DeeFile_GetSize(*this)); }
    bool (isatty)() const { return throw_if_negative(DeeFile_IsAtty(*this)) != 0; }
    dsysfd_t (fileno)() const { dsysfd_t result = DeeFile_Fileno(*this); if unlikely(result == DSYSFD_INVALID) throw_last_deemon_exception(); return result; }
    string (filename)() const;
    string (readline)(size_t max_length = (size_t)-1, bool keep_lf = true) const;
    string (read)(size_t max_length = (size_t)-1, bool readall = false) const;
    string (pread)(Dee_pos_t pos, size_t max_length = (size_t)-1, bool readall = false) const;
    size_t (printf)(char const *__restrict format, ...) const {
        Dee_ssize_t result;
        va_list args;
        va_start(args,format);
        result = DeeFile_VPrintf(*this,format,args);
        va_end(args);
        return throw_if_negative(result);
    }
    size_t (vprintf)(char const *__restrict format, va_list args) const {
        return throw_if_negative(DeeFile_VPrintf(*this,format,args));
    }
    void (printnl)() const { throw_if_nonzero(DeeFile_PrintNl(*this)); }
    void (printobj)(DeeObject *__restrict obj) const { throw_if_nonzero(DeeFile_PrintObject(*this,obj)); }
    void (printobjsp)(DeeObject *__restrict obj) const { throw_if_nonzero(DeeFile_PrintObjectSp(*this,obj)); }
    void (printobjnl)(DeeObject *__restrict obj) const { throw_if_nonzero(DeeFile_PrintObjectNl(*this,obj)); }
    void (printall)(DeeObject *__restrict seq) const { throw_if_nonzero(DeeFile_PrintAll(*this,seq)); }
    void (printallsp)(DeeObject *__restrict seq) const { throw_if_nonzero(DeeFile_PrintAllSp(*this,seq)); }
    void (printallnl)(DeeObject *__restrict seq) const { throw_if_nonzero(DeeFile_PrintAllNl(*this,seq)); }
    file const &operator << (DeeObject *__restrict right) const { printobj(right); return *this; }
    file const &operator << (object const &right) const { printobj(right); return *this; }
    template<class T> typename std::enable_if<detail::any_convertible<T>::exists,file>::type const &
    operator << (T const &right) const { printobj(object(inherit(detail::any_convertible<T>::convert(right)))); return *this; }

    static file (stdin_)() { return inherit(DeeFile_GetStd(DEE_STDIN)); }
    static file (stdout_)() { return inherit(DeeFile_GetStd(DEE_STDOUT)); }
    static file (stderr_)() { return inherit(DeeFile_GetStd(DEE_STDERR)); }
    static file (stddbg_)() { return inherit(DeeFile_GetStd(DEE_STDDBG)); }
    static file (default_stdin)() DEE_CXX_NOTHROW { return nonnull(DeeFile_DefaultStd(DEE_STDIN)); }
    static file (default_stdout)() DEE_CXX_NOTHROW { return nonnull(DeeFile_DefaultStd(DEE_STDOUT)); }
    static file (default_stderr)() DEE_CXX_NOTHROW { return nonnull(DeeFile_DefaultStd(DEE_STDERR)); }
    static file (default_stddbg)() DEE_CXX_NOTHROW { return nonnull(DeeFile_DefaultStd(DEE_STDDBG)); }
    static file (setstdin)(DeeObject *__restrict stream) { return inherit(DeeFile_SetStd(DEE_STDIN,stream)); }
    static file (setstdout)(DeeObject *__restrict stream) { return inherit(DeeFile_SetStd(DEE_STDOUT,stream)); }
    static file (setstderr)(DeeObject *__restrict stream) { return inherit(DeeFile_SetStd(DEE_STDERR,stream)); }
    static file (setstddbg)(DeeObject *__restrict stream) { return inherit(DeeFile_SetStd(DEE_STDDBG,stream)); }
    /* NOTE: These functions will return a NULL-object when the file could not be found,
     *       for any of the reasons that a file may not be locatable when searching though
     *       a path-list. */
    static file (open)(DeeObject *__restrict filename, int oflags = OPEN_FRDONLY, int mode = 0644) {
        DREF DeeObject *result;
        result = DeeFile_Open(filename,oflags,mode);
        if (result == ITER_DONE)
            return inherit(maybenull((DeeObject *)NULL));
        return inherit(result);
    }
    static file (open)(char const *__restrict filename, int oflags = OPEN_FRDONLY, int mode = 0644) {
        DREF DeeObject *result;
        result = DeeFile_OpenString(filename,oflags,mode);
        if (result == ITER_DONE)
            return inherit(maybenull((DeeObject *)NULL));
        return inherit(result);
    }
    static file (openfd)(dsysfd_t fd, /*String*/DeeObject *filename = NULL, int oflags = OPEN_FRDONLY, bool inherit_fd = true) {
        return inherit(DeeFile_OpenFd(fd,filename,oflags,inherit_fd));
    }
    static file (open_object)(DeeObject *__restrict data_owner, void const *data, size_t data_size) {
        return inherit(DeeFile_OpenObjectMemory(data_owner,data,data_size));
    }
    static file (open_object)(DeeObject *__restrict data_owner, Dee_ssize_t begin, Dee_ssize_t end) {
        return inherit(DeeFile_OpenObjectBuffer(data_owner,begin,end));
    }
};

class file::buffer: public file {
public:
    static DeeTypeObject *classtype() DEE_CXX_NOTHROW { return (DeeTypeObject *)&DeeFileBuffer_Type; }
    static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW { return DeeObject_InstanceOf(ob,(DeeTypeObject *)&DeeFileBuffer_Type); }
    static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW { return DeeObject_InstanceOfExact(ob,(DeeTypeObject *)&DeeFileBuffer_Type); }
public:
    DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(buffer,deemon::file)
    buffer(DeeObject *__restrict file, uint16_t mode, size_t size): deemon::file(inherit(DeeFileBuffer_New(file,mode,size))) {}
    void setmode(uint16_t mode, size_t size) { throw_if_nonzero(DeeFileBuffer_SetMode(*this,mode,size)); }
    static void sync_ttys() { throw_if_nonzero(DeeFileBuffer_SyncTTYs()); }
};

class file::writer: public file {
public:
    static DeeTypeObject *classtype() DEE_CXX_NOTHROW { return (DeeTypeObject *)&DeeFileWriter_Type; }
    static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW { return DeeObject_InstanceOf(ob,(DeeTypeObject *)&DeeFileWriter_Type); }
    static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW { return DeeObject_InstanceOfExact(ob,(DeeTypeObject *)&DeeFileWriter_Type); }
public:
    writer(): file(inherit(DeeFile_OpenWriter())) {}
    DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(writer,file)
    deemon::string (string)() const;
    writer const &operator << (DeeObject *__restrict right) const { printobj(right); return *this; }
    writer const &operator << (object const &right) const { printobj(right); return *this; }
    template<class T> typename std::enable_if<detail::any_convertible<T>::exists,writer>::type const &
    operator << (T const &right) const { printobj(object(inherit(detail::any_convertible<T>::convert(right)))); return *this; }
};


namespace detail {
class file_printer {
private:
    DREF DeeObject *fp_file; /* [1..1] Destination file */
public:
    file_printer(): fp_file(throw_if_null(DeeFile_GetStd(DEE_STDOUT))) {}
    file_printer(file_printer const &right) DEE_CXX_NOTHROW: fp_file(incref(right.fp_file)) { }
    file_printer(DeeObject *obj): fp_file(incref(throw_if_null(obj))) { }
    file_printer(obj_nonnull obj) DEE_CXX_NOTHROW: fp_file(incref(obj)) { }
    file_printer(obj_inherited obj): fp_file(throw_if_null(obj)) { }
    file_printer(obj_nonnull_inherited obj) DEE_CXX_NOTHROW: fp_file(obj) { }
#if 1 /* Don't throw errors from destructors... */
    ~file_printer() { if unlikely(DeeFile_PrintNl(fp_file)) DeeError_Handled(ERROR_HANDLED_RESTORE); Dee_Decref(fp_file); }
#else
    ~file_printer() { int error = DeeFile_PrintNl(fp_file); Dee_Decref(fp_file); throw_if_nonzero(error); }
#endif
    file_printer const &operator << (DeeObject *obj) const { throw_if_nonzero(DeeFile_PrintObject(fp_file,obj)); return *this; }
    file_printer const &operator << (object const &obj) const { return this->operator << (obj.ptr()); }
    file_printer const &operator , (DeeObject *obj) const { throw_if_negative(DeeFile_WriteAll(fp_file," ",1*sizeof(char))); throw_if_nonzero(DeeFile_PrintObject(fp_file,obj)); return *this; }
    file_printer const &operator , (object const &obj) const { return this->operator , (obj.ptr()); }
};
}

/* Define a keyword `deemon_print' that is syntactically
 * extremely close to what deemon's native `print' statement
 * does. */
#define deemon_printto(fp) (::deemon::detail::file_printer(fp))<<
#define deemon_print       (::deemon::detail::file_printer())<<

#ifndef NO_PRINT
#define printto(fp)  deemon_printto(fp)
#define print        deemon_print
#endif



#ifdef GUARD_DEEMON_CXX_STRING_H
inline string (file::filename)() const { return inherit(DeeFile_Filename(*this)); }
inline string (file::readline)(size_t max_length, bool keep_lf) const { return inherit(DeeFile_ReadLine(*this,max_length,keep_lf)); }
inline string (file::read)(size_t max_length, bool readall) const { return inherit(DeeFile_ReadText(*this,max_length,readall)); }
inline string (file::pread)(Dee_pos_t pos, size_t max_length, bool readall) const { return inherit(DeeFile_PReadText(*this,max_length,pos,readall)); }
inline deemon::string (file::writer::string)() const { return inherit(DeeObject_InstanceOfExact(this->ptr(),(DeeTypeObject *)&DeeFileWriter_Type) ? DeeFileWriter_GetString(*this) : DeeObject_GetAttrString(*this,"string")); }
#endif /* GUARD_DEEMON_CXX_STRING_H */

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_FILE_H */
