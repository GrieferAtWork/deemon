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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_C_INL 1

#ifdef __INTELLISENSE__
#include "string_functions.c"
#endif

#include <deemon/seq.h>

/* Proxy sequence objects for `string.refindall',
 * `string.relocateall' and `string.resplit' */

DECL_BEGIN

INTDEF DeeTypeObject REFindAllIterator_Type;
INTDEF DeeTypeObject RELocateAllIterator_Type;
INTDEF DeeTypeObject RESplitIterator_Type;
INTDEF DeeTypeObject REFindAll_Type;
INTDEF DeeTypeObject RELocateAll_Type;
INTDEF DeeTypeObject RESplit_Type;

typedef struct {
    OBJECT_HEAD
    DREF String   *re_data;    /* [const][1..1] Data string. */
    DREF String   *re_pattern; /* [const][1..1] Pattern string. */
    struct re_args re_args;    /* [const] Regex arguments. */
} ReSequence;


typedef struct {
    OBJECT_HEAD
    DREF String   *re_data;    /* [const][1..1] Data string. */
    DREF String   *re_pattern; /* [const][1..1] Pattern string. */
    struct re_args re_args;    /* Regex arguments.
                                * NOTE: Only `re_dataptr' and `re_datalen' are mutable! */
#ifndef CONFIG_NO_THREADS
    rwlock_t       re_lock;    /* Lock used during iteration. */
#endif
} ReSequenceIterator;

STATIC_ASSERT(COMPILER_OFFSETOF(ReSequence,re_data) ==
              COMPILER_OFFSETOF(ReSequenceIterator,re_data));
STATIC_ASSERT(COMPILER_OFFSETOF(ReSequence,re_pattern) ==
              COMPILER_OFFSETOF(ReSequenceIterator,re_pattern));
STATIC_ASSERT(COMPILER_OFFSETOF(ReSequence,re_args) ==
              COMPILER_OFFSETOF(ReSequenceIterator,re_args));



PRIVATE int DCALL
refaiter_ctor(ReSequenceIterator *__restrict self) {
 self->re_data    = (DREF String *)Dee_EmptyString;
 self->re_pattern = (DREF String *)Dee_EmptyString;
 Dee_Incref_n(Dee_EmptyString,2);
 self->re_args.re_dataptr    = DeeString_STR(Dee_EmptyString);
 self->re_args.re_patternptr = DeeString_STR(Dee_EmptyString);
 self->re_args.re_datalen    = 0;
 self->re_args.re_patternlen = 0;
 self->re_args.re_offset     = 0;
 self->re_args.re_flags      = DEE_REGEX_FNORMAL;
 rwlock_init(&self->re_lock);
 return 0;
}

PRIVATE int DCALL
refaiter_init(ReSequenceIterator *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 ReSequence *reseq;
 if (DeeArg_Unpack(argc,argv,"o:_refindalliterator",&reseq) ||
     DeeObject_AssertTypeExact((DeeObject *)reseq,&REFindAll_Type))
     goto err;
 memcpy(&self->re_data,&reseq->re_data,
        sizeof(ReSequence)-offsetof(ReSequence,re_data));
 Dee_Incref(self->re_data);
 Dee_Incref(self->re_pattern);
 rwlock_init(&self->re_lock);
 return 0;
err:
 return -1;
}

PRIVATE void DCALL
refaiter_fini(ReSequenceIterator *__restrict self) {
 Dee_Decref(self->re_data);
 Dee_Decref(self->re_pattern);
}

PRIVATE int DCALL
refaiter_bool(ReSequenceIterator *__restrict self) {
 struct regex_range_ptr range;
 char *dataptr; size_t datalen;
 rwlock_read(&self->re_lock);
 dataptr = self->re_args.re_dataptr;
 datalen = self->re_args.re_datalen;
 rwlock_endread(&self->re_lock);
 /* Check if there is at least a single match. */
 return DeeRegex_FindPtr(dataptr,
                         datalen,
                         self->re_args.re_patternptr,
                         self->re_args.re_patternlen,
                        &range,
                         self->re_args.re_flags);
}

/* Assert binary compatibility between the index-variants
 * of `struct regex_range' and `struct regex_range_ex' */
STATIC_ASSERT((COMPILER_OFFSETOF(struct regex_range,rr_end) -
               COMPILER_OFFSETOF(struct regex_range,rr_start)) ==
              (COMPILER_OFFSETOF(struct regex_range_ex,rr_end) -
               COMPILER_OFFSETOF(struct regex_range_ex,rr_start)));

PRIVATE DREF DeeObject *DCALL
refaiter_next(ReSequenceIterator *__restrict self) {
 struct regex_range_ex range;
 char *dataptr; size_t datalen;
 int error; size_t offset;
again:
 rwlock_read(&self->re_lock);
 dataptr = self->re_args.re_dataptr;
 datalen = self->re_args.re_datalen;
 rwlock_endread(&self->re_lock);
 if (!datalen) return ITER_DONE;
 error = DeeRegex_FindEx(dataptr,datalen,
                         self->re_args.re_patternptr,
                         self->re_args.re_patternlen,
                        &range,
                         self->re_args.re_flags);
 if unlikely(error < 0) return NULL;
 if (!error) {
  rwlock_write(&self->re_lock);
  if (dataptr != self->re_args.re_dataptr ||
      datalen != self->re_args.re_datalen) {
   rwlock_endwrite(&self->re_lock);
   goto again;
  }
  self->re_args.re_datalen = 0;
  rwlock_endwrite(&self->re_lock);
  return ITER_DONE;
 }
 ASSERT(range.rr_start < range.rr_end);
 rwlock_write(&self->re_lock);
 if (dataptr != self->re_args.re_dataptr ||
     datalen != self->re_args.re_datalen) {
  rwlock_endwrite(&self->re_lock);
  goto again;
 }
 offset = self->re_args.re_offset;
 self->re_args.re_datalen -= (size_t)(range.rr_end_ptr - self->re_args.re_dataptr);
 self->re_args.re_dataptr  = range.rr_end_ptr;
 self->re_args.re_offset  += range.rr_end;
 rwlock_endwrite(&self->re_lock);
 return regex_pack_range(offset,
                        (struct regex_range *)&range.rr_start);
}

PRIVATE DREF DeeObject *DCALL
refaiter_getseq(ReSequenceIterator *__restrict self) {
 struct re_args args_copy;
 rwlock_read(&self->re_lock);
 memcpy(&args_copy,&self->re_args,sizeof(struct re_args));
 rwlock_endread(&self->re_lock);
 return string_re_findall(self->re_data,
                          self->re_pattern,
                         &args_copy);
}

#define REITER_GETDATAPTR(x) ATOMIC_READ((x)->re_args.re_dataptr)

#define DEFINE_REFA_COMPARE(name,op) \
PRIVATE DREF DeeObject *DCALL \
name(ReSequenceIterator *__restrict self, \
     ReSequenceIterator *__restrict other) { \
 if (DeeObject_AssertTypeExact((DeeObject *)other,Dee_TYPE(self))) \
     return NULL; \
 return_bool(REITER_GETDATAPTR(self) op REITER_GETDATAPTR(other)); \
}
DEFINE_REFA_COMPARE(refa_eq,==)
DEFINE_REFA_COMPARE(refa_ne,!=)
DEFINE_REFA_COMPARE(refa_lo,<)
DEFINE_REFA_COMPARE(refa_le,<=)
DEFINE_REFA_COMPARE(refa_gr,>)
DEFINE_REFA_COMPARE(refa_ge,>=)
#undef DEFINE_REFA_COMPARE


PRIVATE struct type_cmp refaiter_cmp = {
    /* .tp_hash = */NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&refa_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&refa_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&refa_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&refa_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&refa_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&refa_ge
};


PRIVATE struct type_getset refaiter_getsets[] = {
    { "seq", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&refaiter_getseq },
    { NULL }
};

PRIVATE struct type_member refaiter_members[] = {
    TYPE_MEMBER_FIELD("__data__",STRUCT_OBJECT,offsetof(ReSequenceIterator,re_data)),
    TYPE_MEMBER_FIELD("__pattern__",STRUCT_OBJECT,offsetof(ReSequenceIterator,re_pattern)),
    TYPE_MEMBER_END
};


INTERN DeeTypeObject REFindAllIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_refindalliterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&refaiter_ctor,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&refaiter_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(ReSequenceIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&refaiter_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&refaiter_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /* No visit, because it only ever references strings. */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&refaiter_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&refaiter_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */refaiter_getsets,
    /* .tp_members       = */refaiter_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


#define relaiter_ctor    refaiter_ctor
#define relaiter_fini    refaiter_fini
#define relaiter_bool    refaiter_bool
#define relaiter_cmp     refaiter_cmp
#define relaiter_members refaiter_members

PRIVATE int DCALL
relaiter_init(ReSequenceIterator *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 ReSequence *reseq;
 if (DeeArg_Unpack(argc,argv,"o:_relocatealliterator",&reseq) ||
     DeeObject_AssertTypeExact((DeeObject *)reseq,&RELocateAll_Type))
     goto err;
 memcpy(&self->re_data,&reseq->re_data,
        sizeof(ReSequence)-offsetof(ReSequence,re_data));
 Dee_Incref(self->re_data);
 Dee_Incref(self->re_pattern);
 rwlock_init(&self->re_lock);
 return 0;
err:
 return -1;
}


PRIVATE DREF DeeObject *DCALL
relaiter_getseq(ReSequenceIterator *__restrict self) {
 struct re_args args_copy;
 rwlock_read(&self->re_lock);
 memcpy(&args_copy,&self->re_args,sizeof(struct re_args));
 rwlock_endread(&self->re_lock);
 return string_re_locateall(self->re_data,
                            self->re_pattern,
                           &args_copy);
}

PRIVATE struct type_getset relaiter_getsets[] = {
    { "seq", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&relaiter_getseq },
    { NULL }
};

PRIVATE DREF DeeObject *DCALL
relaiter_next(ReSequenceIterator *__restrict self) {
 struct regex_range_ptr range;
 char *dataptr; size_t datalen; int error;
again:
 rwlock_read(&self->re_lock);
 dataptr = self->re_args.re_dataptr;
 datalen = self->re_args.re_datalen;
 rwlock_endread(&self->re_lock);
 if (!datalen) return ITER_DONE;
 error = DeeRegex_FindPtr(dataptr,datalen,
                          self->re_args.re_patternptr,
                          self->re_args.re_patternlen,
                         &range,
                          self->re_args.re_flags);
 if unlikely(error < 0) return NULL;
 if (!error) {
  rwlock_write(&self->re_lock);
  if (dataptr != self->re_args.re_dataptr ||
      datalen != self->re_args.re_datalen) {
   rwlock_endwrite(&self->re_lock);
   goto again;
  }
  self->re_args.re_datalen = 0;
  rwlock_endwrite(&self->re_lock);
  return ITER_DONE;
 }
 ASSERT(range.rr_start < range.rr_end);
 rwlock_write(&self->re_lock);
 if (dataptr != self->re_args.re_dataptr ||
     datalen != self->re_args.re_datalen) {
  rwlock_endwrite(&self->re_lock);
  goto again;
 }
 self->re_args.re_datalen -= (size_t)(range.rr_end - self->re_args.re_dataptr);
 self->re_args.re_dataptr  = range.rr_end;
 rwlock_endwrite(&self->re_lock);
 /* Return the sub-string matched by the range. */
 return DeeString_NewUtf8(range.rr_start,
                         (size_t)(range.rr_end - range.rr_start),
                          STRING_ERROR_FIGNORE);
}

INTERN DeeTypeObject RELocateAllIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_relocatealliterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&relaiter_ctor,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&relaiter_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(ReSequenceIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&relaiter_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&relaiter_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /* No visit, because it only ever references strings. */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&relaiter_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&relaiter_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */relaiter_getsets,
    /* .tp_members       = */relaiter_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


#define respiter_ctor    refaiter_ctor
#define respiter_fini    refaiter_fini
#define respiter_cmp     refaiter_cmp
#define respiter_members refaiter_members

PRIVATE int DCALL
respiter_init(ReSequenceIterator *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 ReSequence *reseq;
 if (DeeArg_Unpack(argc,argv,"o:_resplititerator",&reseq) ||
     DeeObject_AssertTypeExact((DeeObject *)reseq,&RESplit_Type))
     goto err;
 memcpy(&self->re_data,&reseq->re_data,
         sizeof(ReSequence)-offsetof(ReSequence,re_data));
 if (!self->re_args.re_datalen)
      self->re_args.re_dataptr = NULL;
 Dee_Incref(self->re_data);
 Dee_Incref(self->re_pattern);
 rwlock_init(&self->re_lock);
 return 0;
err:
 return -1;
}

PRIVATE DREF DeeObject *DCALL
respiter_getseq(ReSequenceIterator *__restrict self) {
 struct re_args args_copy;
 rwlock_read(&self->re_lock);
 memcpy(&args_copy,self->re_data,sizeof(struct re_args));
 rwlock_endread(&self->re_lock);
 return string_re_split(self->re_data,
                        self->re_pattern,
                       &args_copy);
}

PRIVATE struct type_getset respiter_getsets[] = {
    { "seq", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&respiter_getseq },
    { NULL }
};

PRIVATE DREF DeeObject *DCALL
respiter_next(ReSequenceIterator *__restrict self) {
 struct regex_range_ptr range;
 char *dataptr; size_t datalen; int error;
again:
 rwlock_read(&self->re_lock);
 dataptr = self->re_args.re_dataptr;
 datalen = self->re_args.re_datalen;
 rwlock_endread(&self->re_lock);
 if (!dataptr) return ITER_DONE;
 error = DeeRegex_FindPtr(dataptr,datalen,
                          self->re_args.re_patternptr,
                          self->re_args.re_patternlen,
                         &range,
                          self->re_args.re_flags);
 if unlikely(error < 0) return NULL;
 if (!error) {
  rwlock_write(&self->re_lock);
  if (dataptr != self->re_args.re_dataptr ||
      datalen != self->re_args.re_datalen) {
   rwlock_endwrite(&self->re_lock);
   goto again;
  }
  self->re_args.re_dataptr = NULL;
  rwlock_endwrite(&self->re_lock);
  /* The last sub-string (aka. all the trailing data) */
  return DeeString_NewUtf8(dataptr,datalen,STRING_ERROR_FIGNORE);
 }
 ASSERT(range.rr_start < range.rr_end);
 rwlock_write(&self->re_lock);
 if (dataptr != self->re_args.re_dataptr ||
     datalen != self->re_args.re_datalen) {
  rwlock_endwrite(&self->re_lock);
  goto again;
 }
 self->re_args.re_datalen -= (size_t)(range.rr_end - self->re_args.re_dataptr);
 self->re_args.re_dataptr  = range.rr_end;
 rwlock_endwrite(&self->re_lock);
 /* Return the sub-string found between this match and the previous one. */
 return DeeString_NewUtf8(dataptr,
                         (size_t)(range.rr_start - dataptr),
                          STRING_ERROR_FIGNORE);
}

PRIVATE int DCALL
respiter_bool(ReSequenceIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
 return self->re_args.re_dataptr != NULL;
#else
 return ATOMIC_READ(self->re_args.re_dataptr) != NULL;
#endif
}


INTERN DeeTypeObject RESplitIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_resplititerator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&respiter_ctor,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&respiter_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(ReSequenceIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&respiter_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&respiter_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /* No visit, because it only ever references strings. */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&respiter_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&respiter_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */respiter_getsets,
    /* .tp_members       = */respiter_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


PRIVATE int DCALL
refa_ctor(ReSequence *__restrict self) {
 self->re_data    = (DREF String *)Dee_EmptyString;
 self->re_pattern = (DREF String *)Dee_EmptyString;
 Dee_Incref_n(Dee_EmptyString,2);
 self->re_args.re_dataptr    = DeeString_STR(Dee_EmptyString);
 self->re_args.re_patternptr = DeeString_STR(Dee_EmptyString);
 self->re_args.re_datalen    = 0;
 self->re_args.re_patternlen = 0;
 self->re_args.re_offset     = 0;
 self->re_args.re_flags      = DEE_REGEX_FNORMAL;
 return 0;
}

PRIVATE int DCALL
refa_init(ReSequence *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 String *rules = NULL;
 if (DeeArg_Unpack(argc,argv,"oo|o" /*":_refindall"*/,
                   &self->re_data,&self->re_pattern,&rules) ||
     DeeObject_AssertTypeExact((DeeObject *)self->re_data,&DeeString_Type) ||
     DeeObject_AssertTypeExact((DeeObject *)self->re_pattern,&DeeString_Type))
     goto err;
 self->re_args.re_flags = DEE_REGEX_FNORMAL;
 if (rules) {
  if (DeeObject_AssertTypeExact((DeeObject *)rules,&DeeString_Type))
      goto err;
  if (regex_get_rules(DeeString_STR(rules),&self->re_args.re_flags))
      goto err;
 }
 self->re_args.re_dataptr = DeeString_AsUtf8((DeeObject *)self->re_data);
 if unlikely(!self->re_args.re_dataptr) goto err;
 self->re_args.re_patternptr = DeeString_AsUtf8((DeeObject *)self->re_pattern);
 if unlikely(!self->re_args.re_patternptr) goto err;
 self->re_args.re_datalen    = WSTR_LENGTH(self->re_args.re_dataptr);
 self->re_args.re_patternlen = WSTR_LENGTH(self->re_args.re_patternptr);
 Dee_Incref(self->re_data);
 Dee_Incref(self->re_pattern);
 self->re_args.re_offset = 0;
 return 0;
err:
 return -1;
}

#define refa_fini refaiter_fini

PRIVATE int DCALL
refa_bool(ReSequence *__restrict self) {
 struct regex_range_ptr range;
 /* Check if there is at least a single match. */
 return DeeRegex_FindPtr(self->re_args.re_dataptr,
                         self->re_args.re_datalen,
                         self->re_args.re_patternptr,
                         self->re_args.re_patternlen,
                        &range,
                         self->re_args.re_flags);
}

PRIVATE DREF ReSequenceIterator *DCALL
refa_iter(ReSequence *__restrict self) {
 DREF ReSequenceIterator *result;
 result = DeeObject_MALLOC(ReSequenceIterator);
 if unlikely(!result) goto done;
 memcpy(&result->re_data,&self->re_data,
        sizeof(ReSequence)-offsetof(ReSequence,re_data));
 Dee_Incref(result->re_data);
 Dee_Incref(result->re_pattern);
 DeeObject_Init(result,&REFindAllIterator_Type);
 rwlock_init(&result->re_lock);
 return result;
done:
 return NULL;
}

PRIVATE size_t DCALL
refa_nsi_getsize(ReSequence *__restrict self) {
 int error; size_t result;
 struct regex_range_ptr range;
 char *dataptr; size_t datalen;
 result = 0;
 dataptr = self->re_args.re_dataptr;
 datalen = self->re_args.re_datalen;
 for (;;) {
  error = DeeRegex_FindPtr(dataptr,
                           datalen,
                           self->re_args.re_patternptr,
                           self->re_args.re_patternlen,
                          &range,
                           self->re_args.re_flags);
  if unlikely(error < 0) goto err;
  if (!error) break;
  ++result;
  datalen -= (size_t)(range.rr_end - dataptr);
  dataptr  = range.rr_end;
 }
 return result;
err:
 return (size_t)-1;
}

PRIVATE DREF DeeObject *DCALL
refa_size(ReSequence *__restrict self) {
 size_t result = refa_nsi_getsize(self);
 if unlikely(result == (size_t)-1) return NULL;
 return DeeInt_NewSize(result);
}


PRIVATE struct type_nsi refa_nsi = {
    /* .nsi_class   = */TYPE_SEQX_CLASS_SEQ,
    /* .nsi_flags   = */TYPE_SEQX_FNORMAL,
    {
        /* .nsi_seqlike = */{
            /* .nsi_getsize      = */(void *)&refa_nsi_getsize,
            /* .nsi_getsize_fast = */(void *)NULL,
            /* .nsi_getitem      = */(void *)NULL,
            /* .nsi_delitem      = */(void *)NULL,
            /* .nsi_setitem      = */(void *)NULL,
            /* .nsi_getitem_fast = */(void *)NULL,
            /* .nsi_getrange     = */(void *)NULL,
            /* .nsi_getrange_n   = */(void *)NULL,
            /* .nsi_setrange     = */(void *)NULL,
            /* .nsi_setrange_n   = */(void *)NULL,
            /* .nsi_find         = */(void *)NULL,
            /* .nsi_rfind        = */(void *)NULL,
            /* .nsi_xch          = */(void *)NULL,
            /* .nsi_insert       = */(void *)NULL,
            /* .nsi_insertall    = */(void *)NULL,
            /* .nsi_insertvec    = */(void *)NULL,
            /* .nsi_pop          = */(void *)NULL,
            /* .nsi_erase        = */(void *)NULL,
            /* .nsi_remove       = */(void *)NULL,
            /* .nsi_rremove      = */(void *)NULL,
            /* .nsi_removeall    = */(void *)NULL
        }
    }
};

PRIVATE struct type_seq refa_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&refa_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&refa_size,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */NULL,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&refa_nsi
};

PRIVATE struct type_member refa_members[] = {
    TYPE_MEMBER_FIELD("__data__",STRUCT_OBJECT,offsetof(ReSequence,re_data)),
    TYPE_MEMBER_FIELD("__pattern__",STRUCT_OBJECT,offsetof(ReSequence,re_pattern)),
    TYPE_MEMBER_END
};

PRIVATE struct type_member refa_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&REFindAllIterator_Type),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject REFindAll_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_refindall",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&refa_ctor,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&refa_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(ReSequence)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&refa_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&refa_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /* No visit, because it only ever references strings. */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&refa_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */refa_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */refa_class_members
};


#define rela_ctor    refa_ctor
#define rela_init    refa_init
#define rela_fini    refa_fini
#define rela_bool    refa_bool
#define rela_members refa_members
#define rela_size    refa_size
#define rela_nsi     refa_nsi

PRIVATE DREF ReSequenceIterator *DCALL
rela_iter(ReSequence *__restrict self) {
 DREF ReSequenceIterator *result;
 result = DeeObject_MALLOC(ReSequenceIterator);
 if unlikely(!result) goto done;
 memcpy(&result->re_data,&self->re_data,
        sizeof(ReSequence)-offsetof(ReSequence,re_data));
 Dee_Incref(result->re_data);
 Dee_Incref(result->re_pattern);
 DeeObject_Init(result,&RELocateAllIterator_Type);
 rwlock_init(&result->re_lock);
 return result;
done:
 return NULL;
}

PRIVATE struct type_seq rela_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rela_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rela_size,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */NULL,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&rela_nsi
};

PRIVATE struct type_member rela_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&RELocateAllIterator_Type),
    TYPE_MEMBER_END
};


INTERN DeeTypeObject RELocateAll_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_relocateall",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&rela_ctor,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&rela_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(ReSequence)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&rela_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&rela_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /* No visit, because it only ever references strings. */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&rela_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */rela_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */rela_class_members
};

#define resp_fini    refa_fini
#define resp_members refa_members

PRIVATE int DCALL
resp_ctor(ReSequence *__restrict self) {
 int result = refa_ctor(self);
 self->re_args.re_dataptr = NULL;
 return result;
}

PRIVATE int DCALL
resp_init(ReSequence *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
 int result = refa_init(self,argc,argv);
 if (!self->re_args.re_datalen)
      self->re_args.re_dataptr = NULL;
 return result;
}

PRIVATE int DCALL
resp_bool(ReSequence *__restrict self) {
 return self->re_args.re_dataptr != NULL;
}

PRIVATE DREF ReSequenceIterator *DCALL
resp_iter(ReSequence *__restrict self) {
 DREF ReSequenceIterator *result;
 result = DeeObject_MALLOC(ReSequenceIterator);
 if unlikely(!result) goto done;
 memcpy(&result->re_data,&self->re_data,
        sizeof(ReSequence)-offsetof(ReSequence,re_data));
 ASSERT(result->re_args.re_datalen ? 1 : 
        result->re_args.re_dataptr == NULL);
 Dee_Incref(result->re_data);
 Dee_Incref(result->re_pattern);
 DeeObject_Init(result,&RESplitIterator_Type);
 rwlock_init(&result->re_lock);
 return result;
done:
 return NULL;
}

PRIVATE size_t DCALL
resp_nsi_getsize(ReSequence *__restrict self) {
 size_t result;
 if (!self->re_args.re_dataptr)
      return 0;
 result = refa_nsi_getsize(self);
 if (result != (size_t)-1) ++result;
 return result;
}

PRIVATE DREF DeeObject *DCALL
resp_size(ReSequence *__restrict self) {
 size_t result = resp_nsi_getsize(self);
 if unlikely(result == (size_t)-1) return NULL;
 return DeeInt_NewSize(result);
}


PRIVATE struct type_nsi resp_nsi = {
    /* .nsi_class   = */TYPE_SEQX_CLASS_SEQ,
    /* .nsi_flags   = */TYPE_SEQX_FNORMAL,
    {
        /* .nsi_seqlike = */{
            /* .nsi_getsize      = */(void *)&resp_nsi_getsize,
            /* .nsi_getsize_fast = */(void *)NULL,
            /* .nsi_getitem      = */(void *)NULL,
            /* .nsi_delitem      = */(void *)NULL,
            /* .nsi_setitem      = */(void *)NULL,
            /* .nsi_getitem_fast = */(void *)NULL,
            /* .nsi_getrange     = */(void *)NULL,
            /* .nsi_getrange_n   = */(void *)NULL,
            /* .nsi_setrange     = */(void *)NULL,
            /* .nsi_setrange_n   = */(void *)NULL,
            /* .nsi_find         = */(void *)NULL,
            /* .nsi_rfind        = */(void *)NULL,
            /* .nsi_xch          = */(void *)NULL,
            /* .nsi_insert       = */(void *)NULL,
            /* .nsi_insertall    = */(void *)NULL,
            /* .nsi_insertvec    = */(void *)NULL,
            /* .nsi_pop          = */(void *)NULL,
            /* .nsi_erase        = */(void *)NULL,
            /* .nsi_remove       = */(void *)NULL,
            /* .nsi_rremove      = */(void *)NULL,
            /* .nsi_removeall    = */(void *)NULL
        }
    }
};

PRIVATE struct type_seq resp_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&resp_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&resp_size,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */NULL,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&resp_nsi
};

PRIVATE struct type_member resp_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&RESplitIterator_Type),
    TYPE_MEMBER_END
};


INTERN DeeTypeObject RESplit_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_resplit",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&resp_ctor,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&resp_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(ReSequence)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&resp_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&resp_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /* No visit, because it only ever references strings. */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&resp_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */resp_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */resp_class_members
};




INTERN DREF DeeObject *DCALL
string_re_findall(String *__restrict self,
                  String *__restrict pattern,
                  struct re_args const *__restrict args) {
 DREF ReSequence *result;
 result = DeeObject_MALLOC(ReSequence);
 if unlikely(!result) goto done;
 result->re_data    = self;
 result->re_pattern = pattern;
 memcpy(&result->re_args,args,sizeof(struct re_args));
 Dee_Incref(self);
 Dee_Incref(pattern);
 DeeObject_Init(result,&REFindAll_Type);
done:
 return (DREF DeeObject *)result;
}

INTERN DREF DeeObject *DCALL
string_re_locateall(String *__restrict self,
                    String *__restrict pattern,
                    struct re_args const *__restrict args) {
 DREF ReSequence *result;
 result = DeeObject_MALLOC(ReSequence);
 if unlikely(!result) goto done;
 result->re_data    = self;
 result->re_pattern = pattern;
 memcpy(&result->re_args,args,sizeof(struct re_args));
 Dee_Incref(self);
 Dee_Incref(pattern);
 DeeObject_Init(result,&RELocateAll_Type);
done:
 return (DREF DeeObject *)result;
}

INTERN DREF DeeObject *DCALL
string_re_split(String *__restrict self,
                String *__restrict pattern,
                struct re_args const *__restrict args) {
 DREF ReSequence *result;
 result = DeeObject_MALLOC(ReSequence);
 if unlikely(!result) goto done;
 result->re_data    = self;
 result->re_pattern = pattern;
 memcpy(&result->re_args,args,sizeof(struct re_args));
 if (!result->re_args.re_datalen)
      result->re_args.re_dataptr = NULL;
 Dee_Incref(self);
 Dee_Incref(pattern);
 DeeObject_Init(result,&RESplit_Type);
done:
 return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_REPROXY_C_INL */
