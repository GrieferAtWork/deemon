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
#ifndef GUARD_DEEMON_OBJECTS_FILE_C
#define GUARD_DEEMON_OBJECTS_FILE_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/module.h>
#include <deemon/bool.h>
#include <deemon/none.h>
#include <deemon/format.h>
#include <deemon/arg.h>
#include <deemon/bytes.h>
#include <deemon/string.h>
#include <deemon/seq.h>
#include <deemon/int.h>
#include <deemon/error.h>
#include <hybrid/minmax.h>

#include <string.h>

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

DECL_BEGIN

/* Use libc functions for case-insensitive UTF-8 string compare when available. */
#if defined(__USE_KOS) && !defined(CONFIG_NO_CTYPE)
#define MEMCASEEQ(a,b,s) (memcasecmp(a,b,s) == 0)
#elif defined(_MSC_VER) && !defined(CONFIG_NO_CTYPE)
#define MEMCASEEQ(a,b,s) (_memicmp(a,b,s) == 0)
#else
#define MEMCASEEQ(a,b,s)  dee_memcaseeq((uint8_t *)(a),(uint8_t *)(b),s)
LOCAL bool dee_memcaseeq(uint8_t const *a, uint8_t const *b, size_t s) {
 while (s--) {
  if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
      return false;
  ++a;
  ++b;
 }
 return true;
}
#endif

#define UNPACK_ARGS_0()      /* nothing */
#define UNPACK_ARGS_1(a)     ,a
#define UNPACK_ARGS_2(a,b)   ,a,b
#define UNPACK_ARGS_3(a,b,c) ,a,b,c
#define UNPACK_ARGS(n,args) UNPACK_ARGS_##n args

/* File operator invocation. */
#define DEFILE_FILE_OPERATOR(Tresult,eof_result,error_result,Read,READ,ft_read,n,args,param) \
PUBLIC Tresult DCALL \
DeeFile_##Read(DeeObject *__restrict self UNPACK_ARGS(n,args)) \
{ \
 DeeTypeObject *tp_self = Dee_TYPE(self); \
 if (tp_self == &DeeSuper_Type) { \
  tp_self = DeeSuper_TYPE(self); \
  self    = DeeSuper_SELF(self); \
 } \
 while (DeeFileType_CheckExact(tp_self)) { \
  if (tp_self->tp_features&TF_HASFILEOPS) { \
   if unlikely(!((DeeFileTypeObject *)tp_self)->ft_read) \
      break; \
   return (*((DeeFileTypeObject *)tp_self)->ft_read)((DeeFileObject *)self UNPACK_ARGS(n,param)); \
  } \
  tp_self = DeeType_Base(tp_self); \
 } \
 if (DeeNone_Check(self)) return eof_result; \
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_##READ); \
 return error_result; \
}
DEFILE_FILE_OPERATOR(dssize_t,0,-1,Readf,READ,ft_read,3,(void *__restrict buffer, size_t bufsize, dioflag_t flags),(buffer,bufsize,flags))
DEFILE_FILE_OPERATOR(dssize_t,0,-1,Writef,WRITE,ft_write,3,(void const *__restrict buffer, size_t bufsize, dioflag_t flags),(buffer,bufsize,flags))
DEFILE_FILE_OPERATOR(doff_t,0,-1,Seek,SEEK,ft_seek,2,(doff_t off, int whence),(off,whence))
DEFILE_FILE_OPERATOR(int,0,-1,Sync,SYNC,ft_sync,0,(),())
DEFILE_FILE_OPERATOR(int,0,-1,Trunc,TRUNC,ft_trunc,1,(dpos_t size),(size))
DEFILE_FILE_OPERATOR(int,0,-1,Close,CLOSE,ft_close,0,(),())
DEFILE_FILE_OPERATOR(int,GETC_EOF,GETC_ERR,Ungetc,UNGETC,ft_ungetc,1,(int ch),(ch))
#undef DEFILE_FILE_OPERATOR
PUBLIC dssize_t DCALL
DeeFile_Read(DeeObject *__restrict self,
             void *__restrict buffer,
             size_t bufsize) {
 DeeTypeObject *tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 while (DeeFileType_CheckExact(tp_self)) {
  if (tp_self->tp_features&TF_HASFILEOPS) {
   if unlikely(!((DeeFileTypeObject *)tp_self)->ft_read)
      break;
   return (*((DeeFileTypeObject *)tp_self)->ft_read)((DeeFileObject *)self,
                                                      buffer,
                                                      bufsize,
                                                      DEE_FILEIO_FNORMAL);
  }
  tp_self = DeeType_Base(tp_self);
 }
 if (DeeNone_Check(self)) return 0;
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_READ);
 return -1;
}
PUBLIC dssize_t DCALL
DeeFile_Write(DeeObject *__restrict self,
              void const *__restrict buffer,
              size_t bufsize) {
 DeeTypeObject *tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 while (DeeFileType_CheckExact(tp_self)) {
  if (tp_self->tp_features&TF_HASFILEOPS) {
   if unlikely(!((DeeFileTypeObject *)tp_self)->ft_write)
      break;
   return (*((DeeFileTypeObject *)tp_self)->ft_write)((DeeFileObject *)self,
                                                       buffer,
                                                       bufsize,
                                                       DEE_FILEIO_FNORMAL);
  }
  tp_self = DeeType_Base(tp_self);
 }
 if (DeeNone_Check(self)) return 0;
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_WRITE);
 return -1;
}

PUBLIC int DCALL
DeeFile_TruncHere(DeeObject *__restrict self, dpos_t *psize) {
 DeeTypeObject *tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 while (DeeFileType_CheckExact(tp_self)) {
  if (tp_self->tp_features&TF_HASFILEOPS) {
   dpos_t trunc_pos; int result;
   if unlikely(!((DeeFileTypeObject *)tp_self)->ft_trunc ||
               !((DeeFileTypeObject *)tp_self)->ft_seek)
      break;
   /* Determine the current position and truncate the file there. */
   trunc_pos = (dpos_t)(*((DeeFileTypeObject *)tp_self)->ft_seek)((DeeFileObject *)self,0,SEEK_CUR);
   if unlikely((doff_t)trunc_pos < 0) result = -1;
   else result = (*((DeeFileTypeObject *)tp_self)->ft_trunc)((DeeFileObject *)self,trunc_pos);
   if (psize) *psize = trunc_pos;
   return result;
  }
  tp_self = DeeType_Base(tp_self);
 }
 if (DeeNone_Check(self)) return 0;
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_TRUNC);
 return -1;
}
PUBLIC int DCALL DeeFile_Getc(DeeObject *__restrict self) {
 int result;
 DeeTypeObject *tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 while (DeeFileType_CheckExact(tp_self)) {
  if (tp_self->tp_features&TF_HASFILEOPS) {
   if (((DeeFileTypeObject *)tp_self)->ft_getc) {
    result = (*((DeeFileTypeObject *)tp_self)->ft_getc)((DeeFileObject *)self,
                                                         DEE_FILEIO_FNORMAL);
   } else if (((DeeFileTypeObject *)tp_self)->ft_read) {
    char value; dssize_t error;
    error = (*((DeeFileTypeObject *)tp_self)->ft_read)((DeeFileObject *)self,
                                                       &value,sizeof(char),
                                                        DEE_FILEIO_FNORMAL);
    /* */if (error < 0) result = GETC_ERR;
    else if ((size_t)error >= sizeof(char)) result = (int)value;
    else result = GETC_EOF;
   } else break;
   return result;
  }
  tp_self = DeeType_Base(tp_self);
 }
 if (DeeNone_Check(self)) return GETC_EOF;
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_READ);
 return GETC_ERR;
}
PUBLIC int DCALL
DeeFile_Getcf(DeeObject *__restrict self, dioflag_t flags) {
 int result;
 DeeTypeObject *tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 while (DeeFileType_CheckExact(tp_self)) {
  if (tp_self->tp_features&TF_HASFILEOPS) {
   if (((DeeFileTypeObject *)tp_self)->ft_getc) {
    result = (*((DeeFileTypeObject *)tp_self)->ft_getc)((DeeFileObject *)self,flags);
   } else if (((DeeFileTypeObject *)tp_self)->ft_read) {
    char value; dssize_t error;
    error = (*((DeeFileTypeObject *)tp_self)->ft_read)((DeeFileObject *)self,
                                                       &value,sizeof(char),
                                                        flags);
    /* */if (error < 0) result = GETC_ERR;
    else if ((size_t)error >= sizeof(char)) result = (int)value;
    else result = GETC_EOF;
   } else break;
   return result;
  }
  tp_self = DeeType_Base(tp_self);
 }
 if (DeeNone_Check(self)) return GETC_EOF;
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_READ);
 return GETC_ERR;
}
PUBLIC int DCALL
DeeFile_Putc(DeeObject *__restrict self, int ch) {
 int result;
 DeeTypeObject *tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 while (DeeFileType_CheckExact(tp_self)) {
  if (tp_self->tp_features&TF_HASFILEOPS) {
   if (((DeeFileTypeObject *)tp_self)->ft_putc) {
    result = (*((DeeFileTypeObject *)tp_self)->ft_putc)((DeeFileObject *)self,ch,
                                                         DEE_FILEIO_FNORMAL);
   } else if (((DeeFileTypeObject *)tp_self)->ft_write) {
    char value = (char)ch; dssize_t error;
    error = (*((DeeFileTypeObject *)tp_self)->ft_write)((DeeFileObject *)self,
                                                        &value,sizeof(char),
                                                         DEE_FILEIO_FNORMAL);
    /* */if (error < 0) result = GETC_ERR;
    else if ((size_t)error >= sizeof(char)) result = (int)value;
    else result = GETC_EOF;
   } else break;
   return result;
  }
  tp_self = DeeType_Base(tp_self);
 }
 if (DeeNone_Check(self)) return GETC_EOF;
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_WRITE);
 return GETC_ERR;
}
PUBLIC int DCALL
DeeFile_Putcf(DeeObject *__restrict self, int ch, dioflag_t flags) {
 int result;
 DeeTypeObject *tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 while (DeeFileType_CheckExact(tp_self)) {
  if (tp_self->tp_features&TF_HASFILEOPS) {
   if (((DeeFileTypeObject *)tp_self)->ft_putc) {
    result = (*((DeeFileTypeObject *)tp_self)->ft_putc)((DeeFileObject *)self,ch,flags);
   } else if (((DeeFileTypeObject *)tp_self)->ft_write) {
    char value = (char)ch; dssize_t error;
    error = (*((DeeFileTypeObject *)tp_self)->ft_write)((DeeFileObject *)self,
                                                        &value,sizeof(char),
                                                         flags);
    /* */if (error < 0) result = GETC_ERR;
    else if ((size_t)error >= sizeof(char)) result = (int)value;
    else result = GETC_EOF;
   } else break;
   return result;
  }
  tp_self = DeeType_Base(tp_self);
 }
 if (DeeNone_Check(self)) return GETC_EOF;
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_WRITE);
 return GETC_ERR;
}

PUBLIC dssize_t DCALL
DeeFile_PRead(DeeObject *__restrict self,
              void *__restrict buffer,
              size_t bufsize, dpos_t pos) {
 dssize_t result;
 DeeTypeObject *tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 while (DeeFileType_CheckExact(tp_self)) {
  if (tp_self->tp_features&TF_HASFILEOPS) {
   if (((DeeFileTypeObject *)tp_self)->ft_pread) {
    result = (*((DeeFileTypeObject *)tp_self)->ft_pread)((DeeFileObject *)self,buffer,bufsize,pos,DEE_FILEIO_FNORMAL);
   } else if (((DeeFileTypeObject *)tp_self)->ft_read &&
              ((DeeFileTypeObject *)tp_self)->ft_seek) {
    result = (dssize_t)(*((DeeFileTypeObject *)tp_self)->ft_seek)((DeeFileObject *)self,pos,SEEK_SET);
    if (result >= 0) result = (*((DeeFileTypeObject *)tp_self)->ft_read)((DeeFileObject *)self,buffer,bufsize,DEE_FILEIO_FNORMAL);
   } else break;
   return result;
  }
  tp_self = DeeType_Base(tp_self);
 }
 if (DeeNone_Check(self)) return 0;
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_PREAD);
 return -1;
}
PUBLIC dssize_t DCALL
DeeFile_PReadf(DeeObject *__restrict self,
               void *__restrict buffer,
               size_t bufsize, dpos_t pos, dioflag_t flags) {
 dssize_t result;
 DeeTypeObject *tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 while (DeeFileType_CheckExact(tp_self)) {
  if (tp_self->tp_features&TF_HASFILEOPS) {
   if (((DeeFileTypeObject *)tp_self)->ft_pread) {
    result = (*((DeeFileTypeObject *)tp_self)->ft_pread)((DeeFileObject *)self,buffer,bufsize,pos,flags);
   } else if (((DeeFileTypeObject *)tp_self)->ft_read &&
              ((DeeFileTypeObject *)tp_self)->ft_seek) {
    result = (dssize_t)(*((DeeFileTypeObject *)tp_self)->ft_seek)((DeeFileObject *)self,pos,SEEK_SET);
    if (result >= 0) result = (*((DeeFileTypeObject *)tp_self)->ft_read)((DeeFileObject *)self,buffer,bufsize,flags);
   } else break;
   return result;
  }
  tp_self = DeeType_Base(tp_self);
 }
 if (DeeNone_Check(self)) return 0;
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_PREAD);
 return -1;
}
PUBLIC dssize_t DCALL
DeeFile_PWrite(DeeObject *__restrict self,
               void const *__restrict buffer,
               size_t bufsize, dpos_t pos) {
 dssize_t result;
 DeeTypeObject *tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 while (DeeFileType_CheckExact(tp_self)) {
  if (tp_self->tp_features&TF_HASFILEOPS) {
   if (((DeeFileTypeObject *)tp_self)->ft_pwrite) {
    result = (*((DeeFileTypeObject *)tp_self)->ft_pwrite)((DeeFileObject *)self,buffer,bufsize,pos,DEE_FILEIO_FNORMAL);
   } else if (((DeeFileTypeObject *)tp_self)->ft_write &&
              ((DeeFileTypeObject *)tp_self)->ft_seek) {
    result = (dssize_t)(*((DeeFileTypeObject *)tp_self)->ft_seek)((DeeFileObject *)self,pos,SEEK_SET);
    if (result >= 0) result = (*((DeeFileTypeObject *)tp_self)->ft_write)((DeeFileObject *)self,buffer,bufsize,DEE_FILEIO_FNORMAL);
   } else break;
   return result;
  }
  tp_self = DeeType_Base(tp_self);
 }
 if (DeeNone_Check(self)) return 0;
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_PWRITE);
 return -1;
}
PUBLIC dssize_t DCALL
DeeFile_PWritef(DeeObject *__restrict self,
                void const *__restrict buffer,
                size_t bufsize, dpos_t pos, dioflag_t flags) {
 dssize_t result;
 DeeTypeObject *tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 while (DeeFileType_CheckExact(tp_self)) {
  if (tp_self->tp_features&TF_HASFILEOPS) {
   if (((DeeFileTypeObject *)tp_self)->ft_pwrite) {
    result = (*((DeeFileTypeObject *)tp_self)->ft_pwrite)((DeeFileObject *)self,buffer,bufsize,pos,flags);
   } else if (((DeeFileTypeObject *)tp_self)->ft_write &&
              ((DeeFileTypeObject *)tp_self)->ft_seek) {
    result = (dssize_t)(*((DeeFileTypeObject *)tp_self)->ft_seek)((DeeFileObject *)self,pos,SEEK_SET);
    if (result >= 0) result = (*((DeeFileTypeObject *)tp_self)->ft_write)((DeeFileObject *)self,buffer,bufsize,flags);
   } else break;
   return result;
  }
  tp_self = DeeType_Base(tp_self);
 }
 if (DeeNone_Check(self)) return 0;
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_PWRITE);
 return -1;
}




PUBLIC dssize_t DCALL
DeeFile_ReadAll(DeeObject *__restrict self,
                void *__restrict buffer, size_t bufsize) {
 dssize_t result = 0,temp;
 for (;;) {
  temp = DeeFile_Read(self,buffer,bufsize);
  if (temp < 0) return temp;
  if (!temp) break;
  result += temp;
  if ((size_t)temp >= bufsize) break;
  bufsize -= temp;
  buffer = (void *)((uintptr_t)buffer+temp);
 }
 return result;
}
PUBLIC dssize_t DCALL
DeeFile_WriteAll(DeeObject *__restrict self,
                 void const *__restrict buffer,
                 size_t bufsize) {
 dssize_t result = 0,temp;
 for (;;) {
  temp = DeeFile_Write(self,buffer,bufsize);
  if (temp < 0) return temp;
  if (!temp) break;
  result += temp;
  if ((size_t)temp >= bufsize) break;
  bufsize -= temp;
  buffer = (void *)((uintptr_t)buffer+temp);
 }
 return result;
}
PUBLIC dssize_t DCALL
DeeFile_PReadAll(DeeObject *__restrict self,
                 void *__restrict buffer,
                 size_t bufsize, dpos_t pos) {
 dssize_t result = 0,temp;
 for (;;) {
  temp = DeeFile_PRead(self,buffer,bufsize,pos);
  if (temp < 0) return temp;
  if (!temp) break;
  result += temp;
  if ((size_t)temp >= bufsize) break;
  pos     += temp;
  bufsize -= temp;
  buffer   = (void *)((uintptr_t)buffer+temp);
 }
 return result;
}
PUBLIC dssize_t DCALL
DeeFile_PWriteAll(DeeObject *__restrict self,
                  void const *__restrict buffer,
                  size_t bufsize, dpos_t pos) {
 dssize_t result = 0,temp;
 for (;;) {
  temp = DeeFile_PWrite(self,buffer,bufsize,pos);
  if (temp < 0) return temp;
  if (!temp) break;
  result += temp;
  if ((size_t)temp >= bufsize) break;
  pos     += temp;
  bufsize -= temp;
  buffer   = (void *)((uintptr_t)buffer+temp);
 }
 return result;
}

PUBLIC int DCALL
DeeFile_IsAtty(DeeObject *__restrict self) {
 DREF DeeObject *result_ob; int result;
 /* Very simply: Just call the `isatty()' member function. */
 result_ob = DeeObject_CallAttr(self,&str_isatty,0,NULL);
 if unlikely(!result_ob) goto err_call;
 result = DeeObject_Bool(result_ob);
 Dee_Decref(result_ob);
 return result;
err_call:
 /* Check if we can handle attribute/not-implement errors that
  * could be interpreted as indicative of this not being a tty.
  * Fun fact: The way that isatty() is implemented on linux (and KOS ;) ),
  *           is by invoking an fcntl() that is only allowed for TTY file
  *           descriptors, then checking if errno was set, meaning that
  *           even linux does something similar to this, just on a
  *           different level. */
 if (DeeError_Catch(&DeeError_AttributeError) ||
     DeeError_Catch(&DeeError_NotImplemented))
     return 0;
 return -1;
}
PUBLIC dsysfd_t DCALL
DeeFile_Fileno(DeeObject *__restrict self) {
#ifdef CONFIG_FILENO_DENY_ARBITRARY_INTEGERS
 if (DeeObject_AssertType(self,(DeeTypeObject *)&DeeSystemFile_Type))
     goto err;
 return DeeSystemFile_Fileno(self);
#else
 DREF DeeObject *result_ob; dsysfd_t result;
 /* Special case: If the file is a system-file,  */
 if (DeeObject_InstanceOf(self,(DeeTypeObject *)&DeeSystemFile_Type))
     return DeeSystemFile_Fileno(self);
#if 0 /* Types like `socket' aren't drived from files, but implement fileno() */
 /* Check that it's a file at all. */
 if (DeeObject_AssertType(self,(DeeTypeObject *)&DeeFile_Type))
     goto err;
#endif
 /* Callback: Call a `fileno()' member function. */
 result_ob = DeeObject_CallAttr(self,&str_fileno,0,NULL);
 if unlikely(!result_ob) goto err;
 /* Cast the member function's return value to an integer. */
 if (DeeObject_AsFd(result_ob,&result))
     goto err_result_ob;
 Dee_Decref(result_ob);
 return result;
err_result_ob:
 Dee_Decref(result_ob);
err:
 return DSYSFD_INVALID;
#endif
}
PUBLIC DREF /*String*/DeeObject *DCALL
DeeFile_Filename(DeeObject *__restrict self) {
 DREF DeeObject *result;
 /* Special case: If the file is a system-file,  */
 if (DeeObject_InstanceOf(self,(DeeTypeObject *)&DeeSystemFile_Type))
     return DeeSystemFile_Filename(self);
#if 0 /* There might be non-file types that implement a `filename' member. */
 /* Check that it's a file at all. */
 if (DeeObject_AssertType(self,(DeeTypeObject *)&DeeFile_Type))
     goto err;
#endif
 result = DeeObject_GetAttr(self,&str_filename);
 /* Validate that `filename' is actually a string. */
 if (result && DeeObject_AssertTypeExact(result,&DeeString_Type))
     Dee_Clear(result);
 return result;
/*err:*/
/* return NULL;*/
}

PUBLIC DREF DeeObject *DCALL
DeeFile_ReadLine(DeeObject *__restrict self,
                 size_t max_length, bool keep_lf) {
 /* TODO: Unicode support? (or maybe change things to return a `bytes' object?) */
 DeeTypeObject *tp_self; uint32_t features; int ch;
 int (DCALL *pgetc)(DeeFileObject *__restrict,dioflag_t);
 int (DCALL *pungetc)(DeeFileObject *__restrict,int);
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 /* Figure out the getc/ungetc callbacks that should be used. */
 while (DeeFileType_CheckExact(tp_self)) {
  features = tp_self->tp_features;
  if (features&TF_HASFILEOPS) {
   pgetc   = ((DeeFileTypeObject *)tp_self)->ft_getc;
   pungetc = ((DeeFileTypeObject *)tp_self)->ft_ungetc;
   if (!pgetc && ((DeeFileTypeObject *)tp_self)->ft_read)
        pgetc = (int(DCALL *)(DeeFileObject *__restrict,dioflag_t))&DeeFile_Getcf;
   if likely(pgetc && pungetc) goto got_read;
   if (pgetc) {
    /* If GETC() is implemented, but UNGETC()
     * isn't, use the correct error description. */
    err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_UNGETC);
    goto err;
   }
   break;
  }
  tp_self = DeeType_Base(tp_self);
 }
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_GETC);
 goto err;
got_read:
 {
  struct bytes_printer printer = BYTES_PRINTER_INIT;
  /* Keep on reading characters until a linefeed is encountered. */
  while (BYTES_PRINTER_SIZE(&printer) < max_length) {
   ch = (*pgetc)((DeeFileObject *)self,DEE_FILEIO_FNORMAL);
   if (ch == '\r') {
    /* If the next character is '\n', then we must consume it as well. */
    ch = (*pgetc)((DeeFileObject *)self,DEE_FILEIO_FNORMAL);
    if (ch >= 0 && ch != '\n')
        ch = (*pungetc)((DeeFileObject *)self,ch);
    if (ch == GETC_ERR) goto err_printer;
    /* Found a \r\n or \r-linefeed. */
    if (keep_lf) {
     if (bytes_printer_putb(&printer,'\r')) goto err_printer;
     if (ch == '\n' && BYTES_PRINTER_SIZE(&printer) < max_length &&
         bytes_printer_putb(&printer,'\n')) goto err_printer;
    }
    goto done;
   }
   if (ch == GETC_ERR) goto err_printer;
   if (ch == '\n') {
    /* Found a \n-linefeed */
    if (keep_lf &&
        bytes_printer_putb(&printer,'\n'))
        goto err_printer;
    goto done;
   }
   if (ch == GETC_EOF) {
    /* Stop on EOF */
    if (!BYTES_PRINTER_SIZE(&printer)) {
     /* Nothing was read -> return ITER_DONE */
     bytes_printer_fini(&printer);
     return ITER_DONE;
    }
    goto done;
   }
   /* Print the character. */
   if (bytes_printer_putb(&printer,(uint8_t)ch))
       goto err_printer;
  }
done:
  return bytes_printer_pack(&printer);
err_printer:
  bytes_printer_fini(&printer);
 }
err:
 return NULL;
}


#define READTEXT_BUFSIZE 1024

PUBLIC DREF DeeObject *DCALL
DeeFile_ReadText(DeeObject *__restrict self,
                 size_t max_length, bool readall) {
 uint32_t features; DeeTypeObject *tp_self;
 dssize_t (DCALL *pread)(DeeFileObject *__restrict,void *__restrict,size_t,dioflag_t);
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 /* Figure out the getc/ungetc callbacks that should be used. */
 while (DeeFileType_CheckExact(tp_self)) {
  features = tp_self->tp_features;
  if (features & TF_HASFILEOPS) {
   pread = ((DeeFileTypeObject *)tp_self)->ft_read;
   if likely(pread) goto got_read;
   break;
  }
  tp_self = DeeType_Base(tp_self);
 }
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_READ);
 goto err;
got_read:
 {
  struct bytes_printer printer = BYTES_PRINTER_INIT;
  for (;;) {
   uint8_t *buffer; dssize_t read_size;
   size_t bufsize = MIN(max_length,READTEXT_BUFSIZE);
   /* Allocate more buffer memory. */
   buffer = bytes_printer_alloc(&printer,bufsize);
   if unlikely(!buffer) goto err_printer;
   /* Read more data. */
   read_size = (*pread)((DeeFileObject *)self,buffer,bufsize,DEE_FILEIO_FNORMAL);
   if unlikely(read_size < 0) goto err_printer;
   bytes_printer_release(&printer,bufsize-(size_t)read_size);
   if (readall ? (size_t)read_size == 0
               : (size_t)read_size != bufsize)
       break; /* EOF */
   max_length -= (size_t)read_size;
  }
 /*done:*/
  return bytes_printer_pack(&printer);
err_printer:
  bytes_printer_fini(&printer);
 }
err:
 return NULL;
}

PUBLIC DREF DeeObject *DCALL
DeeFile_PReadText(DeeObject *__restrict self,
                  size_t max_length, dpos_t pos,
                  bool readall) {
 uint32_t features; DeeTypeObject *tp_self;
 dssize_t (DCALL *ppread)(DeeFileObject *__restrict,void *__restrict,size_t,dpos_t,dioflag_t);
 ASSERT_OBJECT(self);
 tp_self = Dee_TYPE(self);
 if (tp_self == &DeeSuper_Type) {
  tp_self = DeeSuper_TYPE(self);
  self    = DeeSuper_SELF(self);
 }
 /* Figure out the getc/ungetc callbacks that should be used. */
 while (DeeFileType_CheckExact(tp_self)) {
  features = tp_self->tp_features;
  if (features&TF_HASFILEOPS) {
   ppread = ((DeeFileTypeObject *)tp_self)->ft_pread;
   if likely(ppread) goto got_read;
   break;
  }
  tp_self = DeeType_Base(tp_self);
 }
 err_unimplemented_operator(Dee_TYPE(self),FILE_OPERATOR_PREAD);
 goto err;
got_read:
 {
  struct bytes_printer printer = BYTES_PRINTER_INIT;
  for (;;) {
   uint8_t *buffer; dssize_t read_size;
   size_t bufsize = MIN(max_length,READTEXT_BUFSIZE);
   /* Allocate more buffer memory. */
   buffer = bytes_printer_alloc(&printer,bufsize);
   if unlikely(!buffer) goto err_printer;
   /* Read more data. */
   read_size = (*ppread)((DeeFileObject *)self,buffer,bufsize,pos,DEE_FILEIO_FNORMAL);
   if unlikely(read_size < 0) goto err_printer;
   bytes_printer_release(&printer,bufsize-(size_t)read_size);
   if (readall ? (size_t)read_size == 0
               : (size_t)read_size != bufsize)
       break; /* EOF */
   max_length -= (size_t)read_size;
   pos        += (size_t)read_size;
  }
/*done:*/
  return bytes_printer_pack(&printer);
err_printer:
  bytes_printer_fini(&printer);
 }
err:
 return NULL;
}



PRIVATE int DCALL print_sp(DeeObject *__restrict self) {
 dssize_t result = DeeFile_WriteAll(self," ",sizeof(char));
 return result < 0 ? (int)result : 0;
}

PUBLIC int DCALL
DeeFile_PrintLn(DeeObject *__restrict self) {
 dssize_t result = DeeFile_WriteAll(self,"\n",sizeof(char));
 return unlikely(result < 0) ? (int)result : 0;
}

#define print_ob_str(self,ob) \
  (DeeObject_Print(ob,(dformatprinter)&DeeFile_WriteAll,self) < 0)


PUBLIC int DCALL
DeeFile_PrintObject(DeeObject *__restrict self,
                    DeeObject *__restrict ob) {
 return print_ob_str(self,ob);
}
PUBLIC int DCALL
DeeFile_PrintObjectSp(DeeObject *__restrict self,
                      DeeObject *__restrict ob) {
 if unlikely(print_ob_str(self,ob)) return -1;
 return print_sp(self);
}
PUBLIC int DCALL
DeeFile_PrintObjectNl(DeeObject *__restrict self,
                      DeeObject *__restrict ob) {
 if unlikely(print_ob_str(self,ob)) return -1;
 return DeeFile_PrintLn(self);
}
PUBLIC int DCALL
DeeFile_PrintAll(DeeObject *__restrict self,
                 DeeObject *__restrict ob) {
 int result; DREF DeeObject *elem; bool is_first = true;
 size_t fast_size = DeeFastSeq_GetSize(ob);
 /* Optimization for fast-sequence objects. */
 if (fast_size != DEE_FASTSEQ_NOTFAST) {
  size_t i;
  for (i = 0; i < fast_size; ++i) {
   if (i != 0) {
    result = print_sp(self);
    if unlikely(result) goto err;
   }
   elem = DeeFastSeq_GetItem(ob,i);
   if unlikely(!elem) goto err_m1;
   result = DeeFile_PrintObject(self,elem);
   Dee_Decref(elem);
   if unlikely(result) goto err;
  }
  return 0;
 }
 if unlikely((ob = DeeObject_IterSelf(ob)) == NULL) goto err_m1;
 while (ITER_ISOK(elem = DeeObject_IterNext(ob))) {
  if (!is_first) {
   result = print_sp(self);
   if unlikely(result) goto err_elem;
  }
  result = DeeFile_PrintObject(self,elem);
  if unlikely(result) goto err_elem;
  Dee_Decref(elem);
  is_first = false;
  if (DeeThread_CheckInterrupt())
      goto err_ob;
 }
 Dee_Decref(ob);
 if unlikely(!elem) goto err_m1;
 return 0;
err_elem:
 Dee_Decref(elem);
err_ob:
 Dee_Decref(ob);
err:
 return result;
err_m1:
 result = -1;
 goto err;
}
PUBLIC int DCALL
DeeFile_PrintAllSp(DeeObject *__restrict self,
                   DeeObject *__restrict ob) {
 int result; DREF DeeObject *elem;
 size_t fast_size = DeeFastSeq_GetSize(ob);
 /* Optimization for fast-sequence objects. */
 if (fast_size != DEE_FASTSEQ_NOTFAST) {
  size_t i;
  for (i = 0; i < fast_size; ++i) {
   elem = DeeFastSeq_GetItem(ob,i);
   if unlikely(!elem) goto err;
   result = DeeFile_PrintObjectSp(self,elem);
   Dee_Decref(elem);
   if unlikely(result) goto err;
  }
  return 0;
 }
 if unlikely((ob = DeeObject_IterSelf(ob)) == NULL) goto err;
 while (ITER_ISOK(elem = DeeObject_IterNext(ob))) {
  result = DeeFile_PrintObjectSp(self,elem);
  Dee_Decref(elem);
  if unlikely(result) { Dee_Decref(ob); return result; }
  if (DeeThread_CheckInterrupt())
      goto err_ob;
 }
 if unlikely(!elem) goto err_ob;
 Dee_Decref(ob);
 return 0;
err_ob:
 Dee_Decref(ob);
err:
 return -1;
}
PUBLIC int DCALL
DeeFile_PrintAllNl(DeeObject *__restrict self,
                   DeeObject *__restrict ob) {
 if unlikely(DeeFile_PrintAll(self,ob)) return -1;
 return DeeFile_PrintLn(self);
}




PUBLIC dssize_t DCALL
DeeFile_VPrintf(DeeObject *__restrict self,
                char const *__restrict format, va_list args) {
 return Dee_VFormatPrintf((dformatprinter)&DeeFile_WriteAll,self,format,args);
}
PUBLIC dssize_t
DeeFile_Printf(DeeObject *__restrict self,
               char const *__restrict format, ...) {
 va_list args; dssize_t result;
 va_start(args,format);
 result = DeeFile_VPrintf(self,format,args);
 va_end(args);
 return result;
}

PUBLIC int DCALL
DeeFile_ProcessMode(char const *__restrict text) {
 int result;
 ASSERT(text);
 for (;;) {
  char ch = *text++;
  /* */if (ch == 'w') result = OPEN_FWRONLY|OPEN_FCREAT|OPEN_FTRUNC;
  else if (ch == 'r') result = OPEN_FRDONLY;
  else if (ch == '+') result = (result&~(OPEN_FACCMODE|OPEN_FTRUNC))|OPEN_FRDWR;
  else if (!ch) break;
  else { /* TODO: Error Invalid mode string. */ }
 }
 return result;
}


INTDEF int DCALL
type_ctor(DeeTypeObject *__restrict self);

PRIVATE int DCALL
filetype_ctor(DeeFileTypeObject *__restrict self) {
 self->ft_read   = NULL;
 self->ft_write  = NULL;
 self->ft_seek   = NULL;
 self->ft_sync   = NULL;
 self->ft_trunc  = NULL;
 self->ft_close  = NULL;
 self->ft_pread  = NULL;
 self->ft_pwrite = NULL;
 self->ft_getc   = NULL;
 self->ft_ungetc = NULL;
 self->ft_putc   = NULL;
 return type_ctor(&self->ft_base);
}

PUBLIC DeeTypeObject DeeFileType_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"filetype",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FGC,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeType_Type,
    /* .tp_init = */{
        {
            /* .tp_var = */{
                /* .tp_ctor      = */&filetype_ctor,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                /* .tp_typesize  = */{ sizeof(DeeFileTypeObject) }
                
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};






PRIVATE int DCALL
file_init(DeeFileObject *__restrict UNUSED(self)) {
 return 0;
}


struct open_option {
    char     name[11]; /* Name. */
#define OPEN_EXFLAG_FNORMAL 0x00
#define OPEN_EXFLAG_FTEXT   0x01 /* Wrap the file in a text-file wrapper that
                                  * automatically converts its encoding to UTF-8. */
#define OPEN_EXFLAG_FNOBUF  0x02 /* Open the file without wrapping it inside a buffer. */
    uint8_t  exflg;   /* Extended flags (Set of `OPEN_EXFLAG_F*'). */
    int      mask;    /* Mask of flags which, when already set, causes the format to become invalid. */
    int      flag;    /* Flags. (or-ed with the flags after `mask' is checked) */
};

/* Open options are parsed from a comma-separated
 * string passed as second argument to file.open:
 * >> file.open("foo.txt","w+");                    // STD-C mode name.
 * >> file.open("foo.txt","text,RW,T,C");           // Extended form.
 * >> file.open("foo.txt","text,rdwr,trunc,creat"); // Long form.
 */
#define BASEMODE_MASK (OPEN_FACCMODE|OPEN_FCREAT|OPEN_FEXCL|OPEN_FTRUNC|OPEN_FAPPEND)
PRIVATE struct open_option const open_options[] = {
#if 0
    /* STD-C compatible open modes. */
    { "r",    OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FRDONLY },
    { "r+",   OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FRDWR },
    { "a",    OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FWRONLY|OPEN_FAPPEND|OPEN_FCREAT },
    { "a+",   OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FRDWR|OPEN_FAPPEND|OPEN_FCREAT },
    { "rb",   OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDONLY },
    { "rb+",  OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR },
    { "r+b",  OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR },
    { "ab",   OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FWRONLY|OPEN_FAPPEND|OPEN_FCREAT },
    { "ab+",  OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR|OPEN_FAPPEND|OPEN_FCREAT },
    { "a+b",  OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR|OPEN_FAPPEND|OPEN_FCREAT },
    { "w",    OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FWRONLY|OPEN_FTRUNC|OPEN_FCREAT },
    { "w+",   OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FRDWR|OPEN_FTRUNC|OPEN_FCREAT },
    { "wb",   OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FWRONLY|OPEN_FTRUNC|OPEN_FCREAT },
    { "wb+",  OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR|OPEN_FTRUNC|OPEN_FCREAT },
    { "w+b",  OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR|OPEN_FTRUNC|OPEN_FCREAT },
    { "wx",   OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FWRONLY|OPEN_FTRUNC|OPEN_FCREAT|OPEN_FEXCL },
    { "w+x",  OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FRDWR|OPEN_FTRUNC|OPEN_FCREAT|OPEN_FEXCL },
    { "wbx",  OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FWRONLY|OPEN_FTRUNC|OPEN_FCREAT|OPEN_FEXCL },
    { "wb+x", OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR|OPEN_FTRUNC|OPEN_FCREAT|OPEN_FEXCL },
    { "w+bx", OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR|OPEN_FTRUNC|OPEN_FCREAT|OPEN_FEXCL },
#endif
    /* Short flag names. */
    { "R", OPEN_EXFLAG_FNORMAL, OPEN_FACCMODE, OPEN_FRDONLY },
    { "W", OPEN_EXFLAG_FNORMAL, OPEN_FACCMODE, OPEN_FWRONLY },
    { "RW", OPEN_EXFLAG_FNORMAL, OPEN_FACCMODE, OPEN_FRDWR },
    { "C", OPEN_EXFLAG_FNORMAL, OPEN_FCREAT, OPEN_FCREAT },
    { "X", OPEN_EXFLAG_FNORMAL, OPEN_FEXCL, OPEN_FEXCL },
    { "T", OPEN_EXFLAG_FNORMAL, OPEN_FTRUNC, OPEN_FTRUNC },
    { "A", OPEN_EXFLAG_FNORMAL, OPEN_FAPPEND, OPEN_FAPPEND },
    { "NB", OPEN_EXFLAG_FNORMAL, OPEN_FNONBLOCK, OPEN_FNONBLOCK },
    { "S", OPEN_EXFLAG_FNORMAL, OPEN_FSYNC, OPEN_FSYNC },
    { "D", OPEN_EXFLAG_FNOBUF, OPEN_FDIRECT, OPEN_FDIRECT },
    { "NF", OPEN_EXFLAG_FNORMAL, OPEN_FNOFOLLOW, OPEN_FNOFOLLOW },
    { "NA", OPEN_EXFLAG_FNORMAL, OPEN_FNOATIME, OPEN_FNOATIME },
    { "CE", OPEN_EXFLAG_FNORMAL, OPEN_FCLOEXEC, OPEN_FCLOEXEC },
    { "XR", OPEN_EXFLAG_FNORMAL, OPEN_FXREAD, OPEN_FXREAD },
    { "XW", OPEN_EXFLAG_FNORMAL, OPEN_FXWRITE, OPEN_FXWRITE },
    { "H", OPEN_EXFLAG_FNORMAL, OPEN_FHIDDEN, OPEN_FHIDDEN },
    /* Flags by name. */
    { "rdonly", OPEN_EXFLAG_FNORMAL, OPEN_FACCMODE, OPEN_FRDONLY },
    { "wronly", OPEN_EXFLAG_FNORMAL, OPEN_FACCMODE, OPEN_FWRONLY },
    { "rdwr", OPEN_EXFLAG_FNORMAL, OPEN_FACCMODE, OPEN_FRDWR },
    { "creat", OPEN_EXFLAG_FNORMAL, OPEN_FCREAT, OPEN_FCREAT },
    { "excl", OPEN_EXFLAG_FNORMAL, OPEN_FEXCL, OPEN_FEXCL },
    { "trunc", OPEN_EXFLAG_FNORMAL, OPEN_FTRUNC, OPEN_FTRUNC },
    { "append", OPEN_EXFLAG_FNORMAL, OPEN_FAPPEND, OPEN_FAPPEND },
    { "nonblock", OPEN_EXFLAG_FNORMAL, OPEN_FNONBLOCK, OPEN_FNONBLOCK },
    { "sync", OPEN_EXFLAG_FNORMAL, OPEN_FSYNC, OPEN_FSYNC },
    { "direct", OPEN_EXFLAG_FNOBUF, OPEN_FDIRECT, OPEN_FDIRECT },
    { "nofollow", OPEN_EXFLAG_FNORMAL, OPEN_FNOFOLLOW, OPEN_FNOFOLLOW },
    { "noatime", OPEN_EXFLAG_FNORMAL, OPEN_FNOATIME, OPEN_FNOATIME },
    { "cloexec", OPEN_EXFLAG_FNORMAL, OPEN_FCLOEXEC, OPEN_FCLOEXEC },
    { "xread", OPEN_EXFLAG_FNORMAL, OPEN_FXREAD, OPEN_FXREAD },
    { "xwrite", OPEN_EXFLAG_FNORMAL, OPEN_FXWRITE, OPEN_FXWRITE },
    { "hidden", OPEN_EXFLAG_FNORMAL, OPEN_FHIDDEN, OPEN_FHIDDEN },
    /* Extended flag names. */
    { "binary", OPEN_EXFLAG_FNORMAL, 0, 0 },
    { "text", OPEN_EXFLAG_FTEXT, 0, 0 },
    { "nobuf", OPEN_EXFLAG_FNOBUF, 0, 0 },
};

PRIVATE DREF DeeObject *DCALL
file_class_open(DeeObject *__restrict UNUSED(self),
                size_t argc, DeeObject **__restrict argv) {
 DeeObject *path; DREF DeeObject *result,*new_result;
 uint8_t flags; int oflags,mode = 0644;
 DeeObject *oflags_ob = NULL;
 if (DeeArg_Unpack(argc,argv,"o|od",&path,&oflags_ob,&mode) ||
     DeeObject_AssertTypeExact(path,&DeeString_Type))
     goto err;
 if (!oflags_ob) {
  /* Default to `r' */
  flags  = OPEN_EXFLAG_FTEXT;
  oflags = OPEN_FRDONLY;
 } else if (!DeeString_Check(oflags_ob)) {
  flags = OPEN_EXFLAG_FNORMAL;
  if (DeeObject_AsInt(oflags_ob,&oflags))
      goto err;
 } else {
  char *iter;
  iter   = DeeString_STR(oflags_ob);
  flags  = OPEN_EXFLAG_FNORMAL;
  oflags = 0;
  for (;;) {
   bool open_binary;
   unsigned int i; size_t optlen;
   char *next = strchr(iter,',');
   if (!next) next = iter+strlen(iter);
   optlen = (size_t)(next-iter);
   if (optlen < COMPILER_LENOF(open_options[0].name)) {
    for (i = 0; i < COMPILER_LENOF(open_options); ++i) {
     if (open_options[i].name[optlen]) continue;
     if (memcmp(open_options[i].name,iter,optlen*sizeof(char)) != 0) continue;
     if (oflags&open_options[i].mask) goto err_invalid_oflags; /* Check illegal old flags. */
     /* Apply new flags. */
     flags  |= open_options[i].exflg;
     oflags |= open_options[i].flag;
     goto found_option;
    }
   }
   /* Check for an STD-C conforming open mode. */
   if (!optlen) goto err_invalid_oflags;
   if (oflags&BASEMODE_MASK) goto err_invalid_oflags;
   open_binary = false;
   if (*iter == 'r') {
    oflags |= OPEN_FRDONLY;
   } else if (*iter == 'w') {
    oflags |= OPEN_FWRONLY|OPEN_FTRUNC|OPEN_FCREAT;
   } else if (*iter == 'a') {
    oflags |= OPEN_FWRONLY|OPEN_FAPPEND|OPEN_FCREAT;
   } else goto err_invalid_oflags;
   if (*++iter == 'b') ++iter,open_binary = true;
   if (*iter == '+') ++iter,oflags &= ~OPEN_FACCMODE,oflags |= OPEN_FRDWR;
   if (*iter == 'b' && !open_binary) ++iter,open_binary = true;
   if (*iter == 'x' && (oflags&(OPEN_FTRUNC|OPEN_FCREAT)) == (OPEN_FTRUNC|OPEN_FCREAT))
      ++iter,oflags |= OPEN_FEXCL;
   if (*iter == 't' && !open_binary) ++iter; /* Accept a trailing `t', as suggested by STD-C */
   if (iter != next) goto err_invalid_oflags;
   if (!open_binary) flags |= OPEN_EXFLAG_FTEXT;
found_option:
   if (!*next) break;
   iter = next+1;
  }
 }
 /* Actually open the file. */
 result = DeeFile_Open(path,oflags,mode);
 if unlikely(!ITER_ISOK(result)) {
  if unlikely(!result) goto err;
  /* Handle file-not-found / file-already-exists errors. */
  if ((oflags&(OPEN_FCREAT|OPEN_FEXCL)) == (OPEN_FCREAT|OPEN_FEXCL)) {
   DeeError_Throwf(&DeeError_FileExists,
                   "File %r already exists",path);
  } else {
   DeeError_Throwf(&DeeError_FileNotFound,
                   "File %r could not be found",
                   path);
  }
  goto err;
 }
 if (flags&OPEN_EXFLAG_FTEXT) {
  /* TODO: Wrap the file in a text-decoder. */
 }
 if (!(flags&OPEN_EXFLAG_FNOBUF)) {
  /* Wrap the file in a buffer. */
  new_result = DeeFileBuffer_New(result,(oflags&OPEN_FACCMODE) == OPEN_FRDONLY
                                 ? (FILE_BUFFER_MODE_AUTO|FILE_BUFFER_FREADONLY)
                                 : (FILE_BUFFER_MODE_AUTO),0);
  Dee_Decref(result);
  if unlikely(!new_result) goto err;
  result = new_result;
 }
 return result;
err_invalid_oflags:
 DeeError_Throwf(&DeeError_ValueError,
                 "Invalid open mode %r",
                 oflags_ob);
err:
 return NULL;
}


PRIVATE struct type_method file_class_methods[] = {
    { "open", &file_class_open,
      DOC("(string path,string mode=\"r\")->file\n"
          "(string path,int mode)->file\n"
          "@interrupt\n"
          "@throw FileExists The passed mode contains both ${\"creat\"} and ${\"excl\"}, but the given @path already existed\n"
          "@throw FileNotFound The given @path could not be found\n"
          "@throw AccessError The current user does not have permissions to access the given @path in the requested manner\n"
          "@throw ReadOnly Write-access, or create-file was requested, but the filesystem hosting @path is mounted as read-only\n"
          "@throw UnsupportedAPI Filesystem access has been disabled, or ${\"creat\"} was passed and the filesystem hosting @path does not support the creation of new files\n"
          "@throw FSError Failed to open the given @path for some reason\n"
          "Consults the filesystem to open or create a given @path, using the given @mode\n"
          "Mode is implemented as a comma-separated list of open options that include "
          "those described by the C standard, allowing the basic ${\"r\"}, ${\"w\"}, ${\"a\"} "
          "as well as each followed by an additional ${\"+\"}, also supporting the "
          "${\"b\"} and ${\"x\"} modifiers, as well as the suggested ${\"t\"} flag\n"
          "In addition to this, any of the comma-separated options can be one of the following "
          "strings to better fine-tune the exact open-behavior, if supported by the host:\n"
          "%{table Flag String|Description\n"
          "-${\"rdonly\"}, ${\"R\"}|Open for read-only (default)\n"
          "-${\"wronly\"}, ${\"W\"}|Open for write access only\n"
          "-${\"rdwr\"}, ${\"RW\"}|Open for both reading and writing\n"
          "-${\"creat\"}, ${\"C\"}|Create the file if it doesn't exist already\n"
          "-${\"excl\"}, ${\"X\"}|When used with ${\"creat\"}, fail if the file already exists\n"
          "-${\"trunc\"}, ${\"T\"}|Truncate an existing file to a length of $0 before opening it\n"
          "-${\"append\"}, ${\"A\"}|Write operations always append data to the end of the file}\n"
          "Additionally, the following flags are accepted, but ignored if the host doesn't support them:\n"
          "%{table Flag String|Description\n"
          "-${\"nonblock\"}, ${\"NB\"}|Don't block when attempting to read/write\n"
          "-${\"sync\"}, ${\"S\"}|Write operations block until all data has been written to disk\n"
          "-${\"direct\"}, ${\"D\"}|Bypass system buffers and directly pass data to the kernel when writing\n"
          "-${\"nofollow\"}, ${\"NF\"}|Do not follow symbolic links\n"
          "-${\"noatime\"}, ${\"NA\"}|Do not update access times\n"
          "-${\"cloexec\"}, ${\"CE\"}|Do not inherit the file in child processes\n"
          "-${\"xread\"}, ${\"XR\"}|Request exclusive read access\n"
          "-${\"xwrite\"}, ${\"XW\"}|Request exclusive write access\n"
          "-${\"hidden\"}, ${\"H\"}|Set a host-specific hidden-file flag when creating a new file (if the host uses a flag to track this attribute)}\n"
          "The following flags may be passed to modify buffering behavior:\n"
          "%{table Flag String|Description\n"
          "-${\"binary\"}|Open the file in binary mode (default, unless an STD-C modifier is used, in which case it that must contain the ${\"b\"} flag)\n"
          "-${\"text\"}|Open the file in text mode (default if an STD-C modifier was used)\n"
          "-${\"nobuf\"}|Do not wrap the returned file in a buffer (Also implied when ${\"direct\"} is passed)}\n"
          "Not that unlike in many other places, case is NOT ignored for these options\n"
          "In addition to the string-based options for @mode, an integer bit-set may be "
          "passed consisting of `OPEN_F*' flags that can be found in deemon's system headers") },
    { NULL }
};


#define DEE_STDCNT 3
PRIVATE DREF DeeObject *dee_std[DEE_STDCNT] =
    { ITER_DONE, ITER_DONE, ITER_DONE };
#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(dee_std_lock);
#endif

PRIVATE uint16_t const std_buffer_modes[DEE_STDCNT] = {
    /* [DEE_STDIN ] = */FILE_BUFFER_MODE_AUTO|FILE_BUFFER_FREADONLY,
    /* [DEE_STDOUT] = */FILE_BUFFER_MODE_AUTO,
    /* [DEE_STDERR] = */FILE_BUFFER_MODE_AUTO
};

PRIVATE DREF DeeObject *DCALL
create_std_buffer(unsigned int id) {
 DREF DeeObject *result,*new_result;
 ASSERT(id < DEE_STDCNT);
 /* Create a buffer for the standard stream. */
#ifdef CONFIG_NATIVE_STD_FILES_ARE_BUFFERED
 /* If the native STD files are already buffered, there'd
  * be no point in us adding our own buffer into the mix. */
 result = DeeFile_DefaultStd(id);
 Dee_Incref(result);
#else
 result = DeeFileBuffer_New(DeeFile_DefaultStd(id),
                            std_buffer_modes[id],0);
 if unlikely(!result) goto done;
#endif
#ifndef CONFIG_NO_THREADS
 rwlock_write(&dee_std_lock);
#endif
 /* Save the newly created buffer in the standard stream vector. */
 new_result = dee_std[id];
 if unlikely(new_result != ITER_DONE) {
  Dee_XIncref(new_result);
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&dee_std_lock);
#endif
  Dee_Decref(result);
  result = new_result;
  if (!result) {
   DeeError_Throwf(&DeeError_UnboundAttribute,
                   "Unbound standard stream");
  }
  goto done;
 }
 Dee_Incref(result);
 dee_std[id] = result;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&dee_std_lock);
#endif
done:
 return result;
}


PUBLIC DREF DeeObject *DCALL
DeeFile_GetStd(unsigned int id) {
 DREF DeeObject *result;
 ASSERT(id < DEE_STDCNT);
#ifndef CONFIG_NO_THREADS
 rwlock_read(&dee_std_lock);
#endif
 result = dee_std[id];
 if unlikely(!ITER_ISOK(result)) {
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&dee_std_lock);
#endif
  /* When the stream is `ITER_DONE', lazily create the STD stream. */
  if (result == ITER_DONE)
      return create_std_buffer(id);
  DeeError_Throwf(&DeeError_UnboundAttribute,
                  "Unbound standard stream");
  goto done;
 }
 Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&dee_std_lock);
#endif
done:
 return result;
}

PUBLIC DREF DeeObject *DCALL
DeeFile_TryGetStd(unsigned int id) {
 DREF DeeObject *result;
 ASSERT(id < DEE_STDCNT);
#ifndef CONFIG_NO_THREADS
 rwlock_read(&dee_std_lock);
#endif
 result = dee_std[id];
 if unlikely(!ITER_ISOK(result)) {
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&dee_std_lock);
#endif
  /* When the stream is `ITER_DONE', lazily create the STD stream. */
  if (result == ITER_DONE) {
   result = create_std_buffer(id);
   if unlikely(!result)
      DeeError_Handled(ERROR_HANDLED_RESTORE);
  }
  goto done;
 }
 Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&dee_std_lock);
#endif
done:
 return result;
}

PUBLIC DREF DeeObject *DCALL
DeeFile_SetStd(unsigned int id, DeeObject *file) {
 DREF DeeObject *old_stream;
 ASSERT(id < DEE_STDCNT);
 if (ITER_ISOK(file)) {
  ASSERT(DeeObject_DoCheck(file));
  Dee_Incref(file);
 }
#ifndef CONFIG_NO_THREADS
 rwlock_write(&dee_std_lock);
#endif
 /* Set the given stream. */
 old_stream = dee_std[id];
 dee_std[id] = file;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&dee_std_lock);
#endif
 return old_stream;
}

/* [0..1][lock(WRITE_ONCE)] The `files' module. */
PRIVATE DREF DeeObject *files_module = NULL;
#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(files_module_lock);
#endif

PRIVATE DREF DeeObject *DCALL
get_files_object(DeeObject *__restrict name) {
 DREF DeeObject *result,*mod;
again:
#ifndef CONFIG_NO_THREADS
 rwlock_read(&files_module_lock);
#endif
 mod = files_module;
 if unlikely(!mod) {
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&files_module_lock);
#endif
  mod = DeeModule_Open(&str_files,NULL,true);
  if unlikely(!mod) return NULL;
  if unlikely(DeeModule_RunInit(mod) < 0) {
   Dee_Decref(mod);
   return NULL;
  }
#ifndef CONFIG_NO_THREADS
  rwlock_write(&files_module_lock);
#endif
  if unlikely(files_module) {
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&files_module_lock);
#endif
   Dee_Decref(mod);
   goto again;
  }
  Dee_Incref(mod);
  files_module = mod;
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&files_module_lock);
#endif
 } else {
  Dee_Incref(mod);
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&files_module_lock);
#endif
 }
 result = DeeObject_GetAttr(mod,name);
 Dee_Decref(mod);
 return result;
}

PRIVATE bool DCALL clear_files_module(void) {
 DREF DeeObject *mod;
#ifndef CONFIG_NO_THREADS
 rwlock_write(&files_module_lock);
#endif
 mod = files_module;
 files_module = NULL;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&files_module_lock);
#endif
 Dee_XDecref(mod);
 return mod != NULL;
}

PUBLIC bool DCALL DeeFile_ResetStd(void) {
 bool result = clear_files_module();
 unsigned int id = 0;
 /* Set the default stream for all standard streams. */
 do {
  DREF DeeObject *old_stream,*default_stream;
  default_stream = DeeFile_DefaultStd(id);
  old_stream = DeeFile_SetStd(id,default_stream);
  if (ITER_ISOK(old_stream))
      Dee_Decref(old_stream);
  if (old_stream != default_stream)
      result = true;
 } while (++id != DEE_STDCNT);
 return result;
}

#define DEFINE_FILE_CLASS_STD_FUNCTIONS(stdxxx,DEE_STDXXX) \
PRIVATE DREF DeeObject *DCALL \
file_class_get_##stdxxx(DeeObject *__restrict UNUSED(self)) \
{ \
    return DeeFile_GetStd(DEE_STDXXX); \
} \
PRIVATE int DCALL \
file_class_del_##stdxxx(DeeObject *__restrict self) \
{ \
    DREF DeeObject *old_stream; \
    old_stream = DeeFile_SetStd(DEE_STDXXX,NULL); \
    if unlikely(!old_stream) { \
        err_unbound_attribute(Dee_TYPE(self),#stdxxx); \
        return -1; \
    } \
    return 0; \
} \
PRIVATE int DCALL \
file_class_set_##stdxxx(DeeObject *__restrict UNUSED(self), \
                        DeeObject *__restrict value) \
{ \
    DREF DeeObject *old_stream; \
    old_stream = DeeFile_SetStd(DEE_STDXXX,value); \
    if (old_stream && old_stream != ITER_DONE) \
        Dee_Decref(old_stream); \
    return 0; \
}
DEFINE_FILE_CLASS_STD_FUNCTIONS(stdin,DEE_STDIN)
DEFINE_FILE_CLASS_STD_FUNCTIONS(stdout,DEE_STDOUT)
DEFINE_FILE_CLASS_STD_FUNCTIONS(stderr,DEE_STDERR)
#undef DEFINE_FILE_CLASS_STD_FUNCTIONS
PRIVATE DREF DeeObject *DCALL
file_class_stddbg(DeeObject *__restrict UNUSED(self)) {
 return_reference(DeeFile_DefaultStd(DEE_STDDBG));
}

PRIVATE DREF DeeObject *DCALL
file_class_default_stdin(DeeObject *__restrict UNUSED(self)) {
 return_reference(DeeFile_DefaultStd(DEE_STDIN));
}
PRIVATE DREF DeeObject *DCALL
file_class_default_stdout(DeeObject *__restrict UNUSED(self)) {
 return_reference(DeeFile_DefaultStd(DEE_STDOUT));
}
PRIVATE DREF DeeObject *DCALL
file_class_default_stderr(DeeObject *__restrict UNUSED(self)) {
 return_reference(DeeFile_DefaultStd(DEE_STDERR));
}

PRIVATE DREF DeeObject *DCALL
file_class_getjoined(DeeObject *__restrict UNUSED(self)) {
 return get_files_object(&str_joined);
}

PRIVATE struct type_getset file_class_getsets[] = {
    { "stdin", &file_class_get_stdin, &file_class_del_stdin, &file_class_set_stdin,
      DOC("->file\nThe standard input stream") },
    { "stdout", &file_class_get_stdout, &file_class_del_stdout, &file_class_set_stdout,
      DOC("->file\nThe standard output stream\n"
          "This is also what $print statements will write to when used "
          "without an explicit file target:\n"
          ">print \"foo\";\n"
          ">// Same as:\n"
          ">import file from deemon;\n"
          ">print file.stdout: \"foo\";\n"
          ) },
    { "stderr", &file_class_get_stderr, &file_class_del_stderr, &file_class_set_stderr,
      DOC("->file\nThe standard error stream") },
    { "default_stdin", &file_class_default_stdin, NULL, NULL,
      DOC("->file\nThe default standard input stream") },
    { "default_stdout", &file_class_default_stdout, NULL, NULL,
      DOC("->file\nThe default standard output stream") },
    { "default_stderr", &file_class_default_stderr, NULL, NULL,
      DOC("->file\nThe default standard error stream") },
    { "stddbg", &file_class_stddbg, NULL, NULL,
      DOC("->file\n"
          "A standard stream that usually simply aliases the "
          "default #stderr, but should be used for debug-output\n"
          "Note that unlike the other streams, this one can't be redirected") },
    { "joined", &file_class_getjoined, NULL, NULL,
      DOC("->type\nDeprecated alias for :files:joined") },
    { NULL }
};

PRIVATE struct type_member file_class_members[] = {
    TYPE_MEMBER_CONST("iterator",(DeeObject *)&DeeFile_Type),
    TYPE_MEMBER_CONST("reader",(DeeObject *)&DeeFileReader_Type),
    TYPE_MEMBER_CONST("writer",(DeeObject *)&DeeFileWriter_Type),
    TYPE_MEMBER_CONST("buffer",(DeeObject *)&DeeFileBuffer_Type),
    TYPE_MEMBER_CONST("system",(DeeObject *)&DeeSystemFile_Type),
    TYPE_MEMBER_CONST_DOC("io",(DeeObject *)&DeeFile_Type,
                          "Deprecated alias for backwards-compatible access to "
                          "std-streams that used to be located in ${file.io.stdxxx}\n"
                          "Starting with deemon v200, these streams can now be found "
                          "under ${file.stdxxx} and the ${file.io} type has been "
                          "renamed to #system\n"
                          "With that in mind, this field is now simply an alias for :file"),
    TYPE_MEMBER_END
};

PUBLIC doff_t DCALL
DeeFile_GetSize(DeeObject *__restrict self) {
 DREF DeeObject *result;
 result = DeeObject_CallAttr(self,&str_size,0,NULL);
 if likely(result) {
  dpos_t resval; int error;
  error = DeeObject_AsUInt64(result,&resval);
  Dee_Decref(result);
  if unlikely(error) goto err;
  /* Ensure that the file isn't too large. */
  if unlikely((doff_t)resval < 0) {
   DeeError_Throwf(&DeeError_ValueError,
                   "Failed %k is too large (%I64u is bigger than 2^63 bytes)",
                   self,resval);
   goto err;
  }
  return (doff_t)resval;
 }
 /* Failed to call the size() member function. */
 if (DeeFileType_CheckExact(Dee_TYPE(self)) &&
     DeeError_Catch(&DeeError_AttributeError)) {
  /* Translate missing size() attributes to doesnt-implement-seek errors. */
  err_unimplemented_operator(Dee_TYPE(self),
                             FILE_OPERATOR_SEEK);
 }
err:
 return -1;
}


PRIVATE DREF DeeObject *DCALL
file_read(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 size_t max_length = (size_t)-1; bool readall = false;
 if (DeeArg_Unpack(argc,argv,"|Idb:read",&max_length,&readall))
     return NULL;
 return DeeFile_ReadText(self,max_length,readall);
}
PRIVATE DREF DeeObject *DCALL
file_readinto(DeeObject *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DeeBuffer buffer; dssize_t result;
 DeeObject *data; bool readall = false;
 if (DeeArg_Unpack(argc,argv,"o|b:readinfo",&data,&readall) ||
     DeeObject_GetBuf(data,&buffer,DEE_BUFFER_FWRITABLE))
     goto err;
 result = readall ? DeeFile_ReadAll(self,buffer.bb_base,buffer.bb_size)
                  : DeeFile_Read(self,buffer.bb_base,buffer.bb_size);
 DeeObject_PutBuf(data,&buffer,DEE_BUFFER_FWRITABLE);
 if unlikely(result < 0) goto err;
 return DeeInt_NewSize((size_t)result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
file_write(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 DeeBuffer buffer; DeeObject *data;
 bool writeall = true; dssize_t result;
 if (DeeArg_Unpack(argc,argv,"o|b:write",&data,&writeall) ||
     DeeObject_GetBuf(data,&buffer,DEE_BUFFER_FREADONLY))
     goto err;
 result = writeall ? DeeFile_WriteAll(self,buffer.bb_base,buffer.bb_size)
                   : DeeFile_Write(self,buffer.bb_base,buffer.bb_size);
 DeeObject_PutBuf(data,&buffer,DEE_BUFFER_FREADONLY);
 if unlikely(result < 0) goto err;
 return DeeInt_NewSize((size_t)result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
file_pread(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 dpos_t file_pos; size_t max_length = (size_t)-1; bool readall = false;
 if (DeeArg_Unpack(argc,argv,"I64u|Idb:pread",&file_pos,&max_length,&readall))
     return NULL;
 return DeeFile_PReadText(self,max_length,file_pos,readall);
}
PRIVATE DREF DeeObject *DCALL
file_preadinto(DeeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 DeeBuffer buffer; dssize_t result;
 DeeObject *data; dpos_t file_pos; bool readall = false;
 if (DeeArg_Unpack(argc,argv,"oI64u|b:readinfo",&data,&file_pos,&readall) ||
     DeeObject_GetBuf(data,&buffer,DEE_BUFFER_FWRITABLE))
     goto err;
 result = readall ? DeeFile_PReadAll(self,buffer.bb_base,buffer.bb_size,file_pos)
                  : DeeFile_PRead(self,buffer.bb_base,buffer.bb_size,file_pos);
 DeeObject_PutBuf(data,&buffer,DEE_BUFFER_FWRITABLE);
 if unlikely(result < 0) goto err;
 return DeeInt_NewSize((size_t)result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
file_pwrite(DeeObject *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 DeeBuffer buffer; DeeObject *data;
 dpos_t file_pos; bool writeall = true; dssize_t result;
 if (DeeArg_Unpack(argc,argv,"oI64u|b:pwrite",&data,&file_pos,&writeall) ||
     DeeObject_GetBuf(data,&buffer,DEE_BUFFER_FREADONLY))
     goto err;
 result = writeall ? DeeFile_PWriteAll(self,buffer.bb_base,buffer.bb_size,file_pos)
                   : DeeFile_PWrite(self,buffer.bb_base,buffer.bb_size,file_pos);
 DeeObject_PutBuf(data,&buffer,DEE_BUFFER_FREADONLY);
 if unlikely(result < 0) goto err;
 return DeeInt_NewSize((size_t)result);
err:
 return NULL;
}

struct whence_name {
    char name[4];
    int  id;
};

PRIVATE struct whence_name whence_names[] = {
    { "set", SEEK_SET },
    { "cur", SEEK_CUR },
    { "end", SEEK_END },
};


PRIVATE DREF DeeObject *DCALL
file_seek(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 doff_t seek_off; int whence = SEEK_SET;
 DeeObject *whence_ob = NULL;
 if (DeeArg_Unpack(argc,argv,"I64d|o:seek",&seek_off,&whence_ob))
     goto err;
 if (whence_ob) {
  if (DeeString_Check(whence_ob)) {
   char const *name = DeeString_STR(whence_ob);
   size_t length    = DeeString_SIZE(whence_ob);
   if (length >= 5 && MEMCASEEQ(name,"SEEK_",5*sizeof(char)))
       name += 5,length -= 5;
   if (length == 3) {
    char buf[4];
    /* Convert the given mode name to lower-case. */
    buf[0] = (char)DeeUni_ToLower(name[0]);
    buf[1] = (char)DeeUni_ToLower(name[1]);
    buf[2] = (char)DeeUni_ToLower(name[2]);
    buf[3] = '\0';
    for (whence = 0; (unsigned int)whence < COMPILER_LENOF(whence_names); ++whence) {
     if (*(uint32_t *)whence_names[(unsigned int)whence].name != *(uint32_t *)buf)
         continue;
     whence = whence_names[(unsigned int)whence].id;
     goto got_whence;
    }
   }
   DeeError_Throwf(&DeeError_ValueError,
                   "Unknown whence mode %r for seek",
                   whence_ob);
   goto err;
  }
  /* Fallback: Conver the whence-object to an integer. */
  if (DeeObject_AsInt(whence_ob,&whence))
      goto err;
 }
got_whence:
 seek_off = DeeFile_Seek(self,seek_off,whence);
 if unlikely(seek_off < 0) goto err;
 return DeeInt_NewU64((dpos_t)seek_off);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
file_tell(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 doff_t result;
 if (DeeArg_Unpack(argc,argv,":tell"))
     goto err;
 result = DeeFile_Tell(self);
 if unlikely(result < 0) goto err;
 return DeeInt_NewU64((dpos_t)result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
file_rewind(DeeObject *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":rewind"))
     goto err;
 if (DeeFile_Rewind(self) < 0) goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
file_trunc(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 dpos_t trunc_pos;
 if (argc == 0) {
  /* Truncate at the current position. */
  if (DeeFile_TruncHere(self,&trunc_pos)) goto err;
 } else {
  /* Truncate at the current position. */
  if (DeeArg_Unpack(argc,argv,"|I64u:trunc",&trunc_pos))
      goto err;
  if (DeeFile_Trunc(self,trunc_pos)) goto err;
 }
 /* Return the position where we've truncated the file. */
 return DeeInt_NewU64(trunc_pos);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
file_sync(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":sync") ||
     DeeFile_Sync(self))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
file_close(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":close") ||
     DeeFile_Close(self))
     goto err;
 return_none;
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
file_getc(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 int result;
 if (DeeArg_Unpack(argc,argv,":getc"))
     goto err;
 result = DeeFile_Getc(self);
 if unlikely(result == GETC_ERR) goto err;
#if GETC_EOF != -1
 if (result == GETC_EOF)
     result = -1;
#endif
 return DeeInt_NewInt(result);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
file_ungetc(DeeObject *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 int result;
 if (DeeArg_Unpack(argc,argv,"d:ungetc",&result))
     goto err;
 result = DeeFile_Ungetc(self,result);
 if unlikely(result == GETC_ERR) goto err;
 return_bool_(result != GETC_EOF);
err:
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
file_putc(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 int result; uint8_t byte;
 if (DeeArg_Unpack(argc,argv,"I8u:putc",&byte))
     goto err;
 result = DeeFile_Putc(self,(int)byte);
 if unlikely(result == GETC_ERR) goto err;
 return_bool_(result != GETC_EOF);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
file_size(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 DeeTypeObject *tp_self = Dee_TYPE(self);
 if (DeeArg_Unpack(argc,argv,":size"))
     goto err;
 while (DeeFileType_CheckExact(tp_self)) {
  doff_t(DCALL *pseek)(DeeFileObject *__restrict self,doff_t off,int whence);
  if (tp_self->tp_features&TF_HASFILEOPS) {
   pseek = ((DeeFileTypeObject *)tp_self)->ft_seek;
   if (pseek) {
    dpos_t old_pos,filesize;
    old_pos = (dpos_t)(*pseek)((DeeFileObject *)self,0,SEEK_CUR);
    if unlikely((doff_t)old_pos < 0) goto err;
    /* Seek to the end to figure out how large the file is. */
    filesize = (dpos_t)(*pseek)((DeeFileObject *)self,0,SEEK_END);
    /* Return to the previous file position. */
    if ((doff_t)filesize >= 0 &&
        (*pseek)((DeeFileObject *)self,(doff_t)old_pos,SEEK_SET) < 0)
        filesize = (dpos_t)-1;
    if unlikely(filesize < 0) goto err;
    /* Return the size of the file. */
    return DeeInt_NewU64((uint64_t)filesize);
   }
  }
  tp_self = DeeType_Base(tp_self);
 }
 err_unimplemented_operator(Dee_TYPE(self),
                            FILE_OPERATOR_SEEK);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
file_readline(DeeObject *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result;
 size_t max_length = (size_t)-1; bool keeplf = true;
 if (argc == 1 && DeeBool_Check(argv[0])) {
  keeplf = DeeBool_IsTrue(argv[0]);
 } else {
  if (DeeArg_Unpack(argc,argv,"|Idb:readline",&max_length,&keeplf))
      return NULL;
 }
 result = DeeFile_ReadLine(self,max_length,keeplf);
 if (result == ITER_DONE)
     result = Dee_EmptyString,
     Dee_Incref(result);
 return result;
}


PRIVATE DREF DeeObject *DCALL
file_readall(DeeObject *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 size_t max_length = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"|Id:readall",&max_length))
     return NULL;
 return DeeFile_ReadText(self,max_length,true);
}
PRIVATE DREF DeeObject *DCALL
file_readallat(DeeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 dpos_t file_pos; size_t max_length = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"I64u|Id:readallat",&file_pos,&max_length))
     return NULL;
 return DeeFile_PReadText(self,max_length,file_pos,true);
}


PRIVATE struct type_method file_methods[] = {
    { "read", &file_read,
      DOC("(int max_bytes=-1,bool readall=false)->bytes\n"
          "Read and return at most @max_bytes of data from the file stream. "
          "When @readall is :true, keep on reading data until the buffer is full, or the "
          "read-callback returns ${0}, rather than until it returns something other than the "
          "internal buffer size used when reading data.") },
    { "readinto", &file_readinto,
      DOC("(buffer dst,bool readall=false)->int\n"
          "Read data into the given buffer @dst and return the number of bytes read. "
          "When @readall is :true, keep on reading data until the buffer is full, or the "
          "read-callback returns ${0}, rather than until it returns something other than the "
          "requested read size.") },
    { "write", &file_write,
      DOC("(buffer data,bool writeall=true)->int\n"
          "Write @data to the file stream and return the actual number of bytes written. "
          "When @writeall is :true, keep writing data until the write-callback "
          "returns $0 or until all data has been written, rather than invoke "
          "the write-callback only a single time.") },
    { "pread", &file_pread,
      DOC("(int pos,int max_bytes=-1,bool readall=false)->bytes\n"
          "Similar to #read, but read data from a given file-offset "
          "@pos, rather than from the current file position") },
    { "preadinto", &file_preadinto,
      DOC("(buffer dst,int pos,bool readall=false)->bytes\n"
          "Similar to #readinto, but read data from a given file-offset "
          "@pos, rather than from the current file position") },
    { "pwrite", &file_pwrite,
      DOC("(buffer data,int pos,bool writeall=true)->int\n"
          "Similar to #write, but write data to a given file-offset "
          "@pos, rather than at the current file position") },
    { "seek", &file_seek,
      DOC("(int off,string whence=\"SET\")->int\n"
          "(int off,int whence)->int\n"
          "@throw ValueError The given string passed as seek mode @whence was not recognized\n"
          "Change the current file pointer according to @off and @whence "
          "before returning its absolute offset within the file.\n"
          "When a string is given for @whence, it may be one of the following "
          "case-insensitive values, optionally prefixed with ${\"SEEK_\"}:\n"
          "-${\"SET\"}: Set the file pointer to an absolute in-file position\n"
          "-${\"CUR\"}: Adjust the file pointer relative to its previous position\n"
          "-${\"END\"}: Set the file pointer relative to the end of the stream") },
    { "tell", &file_tell,
      DOC("()->int\n"
          "Same as calling #seek as ${this.seek(0,\"CUR\")}") },
    { "rewind", &file_rewind,
      DOC("()->none\n"
          "Same as calling #seek as ${this.seek(0,\"SET\")}") },
    { "trunc", &file_trunc,
      DOC("(int new_size)->int\n"
          "()->int\n"
          "Truncate the file to a new length of @new_size bytes. "
          "When no argument is given, the file's length is truncated "
          "to its current position, rather than the one given") },
    { "sync", &file_sync,
      DOC("()->none\n"
          "Flush buffers and synchronize disk activity of the file") },
    { "close", &file_close,
      DOC("()->none\n"
          "Close the file") },
    { "getc", &file_getc,
      DOC("()->int\n"
          "Read and return a single character (byte) from then file, "
          "or return ${-1} if the file's end has been reached") },
    { "ungetc", &file_ungetc,
      DOC("(int ch)->bool\n"
          "Unget a given character @ch to be re-read the next time #getc or #read is called. "
          "If the file's start has already been reached, :false is returned and the character "
          "will not be re-read from this file") },
    { "putc", &file_putc,
      DOC("(int byte)->bool\n"
          "Append a single @byte at the end of @this file, returning :true on "
          "success, or :false if the file has entered an end-of-file state") },
    { DeeString_STR(&str_size), &file_size,
      DOC("()->int\nReturns the size (in bytes) of the file stream") },
    { "readline", &file_readline,
      DOC("(bool keeplf)->bytes\n"
          "(int max_bytes=-1,bool keeplf=true)->bytes\n"
          "Read one line from the file stream, but read at most @max_bytes bytes.\n"
          "When @keeplf is :false, strip the trailing linefeed from the returned bytes object") },

    /* Deprecated functions. */
    { "readall", &file_readall,
      DOC("(int max_bytes=-1)->bytes\n"
          "Deprecated alias for ${this.read(max_bytes,true)}") },
    { "readat", &file_pread,
      DOC("(int pos,int max_bytes=-1,bool readall=false)->bytes\n"
          "Deprecated alias for #pread") },
    { "writeat", &file_pwrite,
      DOC("(buffer data,int pos,bool writeall=true)->int\n"
          "Deprecated alias for #pwrite") },
    { "readallat", &file_readallat,
      DOC("(int pos,int max_bytes=-1,bool readall=false)->bytes\n"
          "Deprecated alias for ${this.pread(pos,max_bytes,true)}") },
    { "setpos", &file_seek,
      DOC("(int pos)->int\n"
          "Deprecated alias for #seek") },
    { "flush", &file_sync,
      DOC("->none\n"
          "Deprecated alias for #sync") },
    { "puts", &file_write,
      DOC("(buffer data)->int\n"
          "Deprecated alias for #write") },

    { NULL }
};

PRIVATE struct type_with file_with = {
    /* Implement with-control for files to close the file upon exit. */
    /* .tp_enter = */NULL,
    /* .tp_leave = */&DeeFile_Close
};

PRIVATE DREF DeeObject *DCALL
file_next(DeeFileObject *__restrict self) {
 return DeeFile_ReadLine((DeeObject *)self,(size_t)-1,true);
}

PRIVATE DREF DeeFileObject *DCALL
file_return_self(DeeFileObject *__restrict self) {
 /* A file is its own iterator */
 return_reference_(self);
}

PRIVATE struct type_getset file_getsets[] = {
    /* Maintain at least a tiny bit of compatibility to the iterator interface... */
    { "seq", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_return_self },
    { NULL }
};


PRIVATE struct type_seq file_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_return_self
};


INTERN DREF DeeObject *DCALL
file_shl(DeeObject *__restrict self,
         DeeObject *__restrict some_object) {
 if (DeeFile_PrintObject(self,some_object))
     return NULL;
 return_reference_(self);
}
PRIVATE DREF DeeObject *DCALL
file_shr(DeeObject *__restrict self,
         DeeObject *__restrict some_object) {
 DeeBuffer buffer; dssize_t result;
 if (DeeObject_GetBuf(some_object,&buffer,DEE_BUFFER_FWRITABLE))
     goto err;
 result = DeeFile_ReadAll(self,buffer.bb_base,buffer.bb_size);
 DeeObject_PutBuf(some_object,&buffer,DEE_BUFFER_FWRITABLE);
 if unlikely(result < 0) goto err;
 if unlikely((size_t)result < buffer.bb_size) {
  DeeError_Throwf(&DeeError_FSError,
                  "Failed to fill the entire buffer of %Iu bytes when only %Iu were read",
                  buffer.bb_size,(size_t)result);
  goto err;
 }
 return_reference_(self);
err:
 return NULL;
}

PRIVATE struct type_math file_math = {
    /* .tp_int32  = */NULL,
    /* .tp_int64  = */NULL,
    /* .tp_double = */NULL,
    /* .tp_int    = */NULL,
    /* .tp_inv    = */NULL,
    /* .tp_pos    = */NULL,
    /* .tp_neg    = */NULL,
    /* .tp_add    = */NULL,
    /* .tp_sub    = */NULL,
    /* .tp_mul    = */NULL,
    /* .tp_div    = */NULL,
    /* .tp_mod    = */NULL,
    /* .tp_shl    = */&file_shl,
    /* .tp_shr    = */&file_shr,
    /* .tp_and    = */NULL,
    /* .tp_or     = */NULL,
    /* .tp_xor    = */NULL,
    /* .tp_pow    = */NULL
};

PUBLIC DeeFileTypeObject DeeFile_Type = {
    /* .ft_base = */{
        OBJECT_HEAD_INIT(&DeeFileType_Type),
        /* .tp_name     = */DeeString_STR(&str_file),
        /* .tp_doc      = */DOC("The base class for all file types\n"
                                "Types derived from this are able to implement a variety of "
                                "new operators related to file operations, which are then "
                                "used by the member functions and other operators provided "
                                "by this type, or can also be used directly:\n"
                                "%{table Operator prototype|Description\n"
                                "-${operator read(int size) -> buffer}|Reads up to size bytes and returns a buffer (usually a :string) containing read data\n"
                                "-${operator write(buffer buf) -> int}|Writes data from buf into the file and returns the number of bytes written\n"
                                "-${operator seek(int off, int whence) -> int}|Moves the file pointer relative to whence, "
                                 "which is one of #SEEK_SET, #SEEK_CUR or #SEEK_END. The return value of this operator is "
                                 "the new, absolute file position within the stream\n"
                                "-${operator sync() -> none}|Synchronize unwritten data with lower-level components\n"
                                "-${operator trunc(int newsize) -> none}|Truncate, or pre-allocate file memory to match a length of newsize\n"
                                "-${operator close() -> none}|Close the file. This operator is invoked during destruction, but can "
                                 "also be invoked before then using the #close member function\n"
                                "-${operator pread(int size, int pos) -> buffer}|Similar to ${operator read}, but data is read from the given absolute file position pos\n"
                                "-${operator pwrite(buffer buf, int pos) -> int}|Similar to ${operator write}, but data is written to the given absolute file position pos\n"
                                "-${operator getc() -> int}|Reads, and returns a single byte from the stream (Usually the same as ${operator read(1)}). "
                                 "If EOF has been reached, a negative value is returned\n"
                                "-${operator ungetc(int ch) -> bool}|Returns a previously read character to the stream, allowing it to be read once again. "
                                 "This functionality isn't provided by many file types, but #buffer supports it, thereby "
                                 "allowing you to wrap practically any file into a buffer to enable support for ungetc, "
                                 "which is a dependency for #{readline}. "
                                 "If the character could be returned, :true is returned. Otherwise :false is\n"
                                "-${operator putc(int ch) -> bool}|Write a single byte to the stream (Usually the same as ${operator write((uint8_t from ctypes)ch)}). "
                                 "Returns :true if the byte was successfully written, or :false if EOF was reached}\n"
                                "\n"
                                "()\n"
                                "Default-construct the file base-class\n"
                                "\n"
                                "iter()\n"
                                "Returns an iterator that allows for line-wise processing of "
                                "file data, making use of the the #readline member function\n"
                                "The returned lines have their trailing line-feeds stripped\n"
                                "Note that because a file cannot be iterated multiple times "
                                "without additional work being done, as well as the fact that "
                                "this type of iteration isn't thread-save, :file isn't derived "
                                "from :sequence, meaning that abstract sequence functions are "
                                "not implicitly provided, but would have to be invoked like "
                                "${sequence.find(file.open(\"foo.txt\"),\"find this line\")}\n"
                                "Note that because isn't derived from :sequence, the returned "
                                "iterator also isn't required to be derived from :iterator\n"
                                "\n"
                                "<<(object ob)\n"
                                "@return Always re-returns @this file\n"
                                "Same as ${print this: ob,;}\n"
                                "\n"
                                ">>(buffer buf)\n"
                                "@throw FSError Failed to fill the entirety of @buf\n"
                                "@return Always re-returns @this file\n"
                                "Same as ${this.readinto(buf,true)}\n"
                                "\n"
                                "leave()\n"
                                "Invokes ${this.operator close()}\n"
                                "Note that due to this operators presence, an "
                                "implicit enter-operator exists, which is a no-op"),
        /* .tp_flags    = */TP_FNORMAL|TP_FNAMEOBJECT,
        /* .tp_weakrefs = */0,
        /* .tp_features = */TF_NONE,
#if 0 /* Even though `file' is technically a sequence, don't spam its
       * members with all the additional functions provided by `DeeSeq_Type'.
       * Especially considering that `file' does its own handling for
       * operations such as `a|b' (creating a multi-targeted file), whereas
       * `sequence' already implements that operator as a union (in the case
       * of file: of all lines in either file).
       * Besides: A lot of sequence functions expect to be able to re-run
       *          iterators multiple times, yet file iterators expect the
       *          user to rewind the file before they can be iterated again,
       *          meaning that in that respect, `file' isn't 100% compliant
       *          to the `sequence' interface, meaning we're kind-of not
       *          even allowed to consider ourselves a sequence.
       *          But as already said: that's a good thing, because
       *          we don't even want to be considered one. */
        /* .tp_base     = */&DeeSeq_Type,
#else
        /* .tp_base     = */&DeeObject_Type,
#endif
        /* .tp_init = */{
            {
                /* .tp_alloc = */{
                    /* .tp_ctor      = */&file_init,
                    /* .tp_copy_ctor = */NULL,
                    /* .tp_deep_ctor = */NULL,
                    /* .tp_any_ctor  = */NULL,
                    /* .tp_free      = */NULL,
                    {
                        /* .tp_instance_size = */sizeof(DeeFileObject)
                    }
                }
            },
            /* .tp_dtor        = */NULL,
            /* .tp_assign      = */NULL,
            /* .tp_move_assign = */NULL
        },
        /* .tp_cast = */{
            /* .tp_str  = */NULL,
            /* .tp_repr = */NULL,
            /* .tp_bool = */NULL
        },
        /* .tp_call          = */NULL,
        /* .tp_visit         = */NULL,
        /* .tp_gc            = */NULL,
        /* .tp_math          = */&file_math,
        /* .tp_cmp           = */NULL,
        /* .tp_seq           = */&file_seq,
        /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_next,
        /* .tp_attr          = */NULL,
        /* .tp_with          = */&file_with,
        /* .tp_buffer        = */NULL,
        /* .tp_methods       = */file_methods,
        /* .tp_getsets       = */file_getsets,
        /* .tp_members       = */NULL,
        /* .tp_class_methods = */file_class_methods,
        /* .tp_class_getsets = */file_class_getsets,
        /* .tp_class_members = */file_class_members
    },
    /* .ft_read   = */NULL,
    /* .ft_write  = */NULL,
    /* .ft_seek   = */NULL,
    /* .ft_sync   = */NULL,
    /* .ft_trunc  = */NULL,
    /* .ft_close  = */NULL,
    /* .ft_pread  = */NULL,
    /* .ft_pwrite = */NULL,
    /* .ft_getc   = */NULL,
    /* .ft_ungetc = */NULL,
    /* .ft_putc   = */NULL
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_FILE_C */
